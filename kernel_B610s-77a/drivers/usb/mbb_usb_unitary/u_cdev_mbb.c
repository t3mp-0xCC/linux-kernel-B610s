/*
* Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
*
* mobile@huawei.com
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

static struct u_cdev_driver* gs_acm_cdev_alloc_driver(USB_INT lines);
static USB_INT gs_acm_cdev_port_alloc(USB_UINT port_num,
                                      struct usb_cdc_line_coding* coding);
static USB_INT gs_acm_cdev_register_driver(struct u_cdev_driver* driver);
static struct device* gs_acm_cdev_register_device(struct u_cdev_driver* driver,
        USB_UINT index, struct device* device);
static USB_INT gs_acm_usb_notifier_cb(struct notifier_block* nb,
                                      USB_ULONG event, USB_PVOID priv);
static USB_VOID gs_acm_cdev_unregister_device(struct u_cdev_driver* driver,
        USB_UINT index);
static USB_VOID gs_acm_cdev_unregister_driver(struct u_cdev_driver* driver);
static int gs_acm_cdev_write_base(struct gs_acm_cdev_port* port, char* buf,
                                  char* phy, unsigned int len, bool is_sync);
static USB_VOID acm_cdev_dump_ep_info(struct gs_acm_cdev_port* port);

USB_INT gacm_cdev_setup(struct usb_gadget* g, USB_UINT count)
{
    USB_UINT			i;
    struct usb_cdc_line_coding	coding;
    USB_INT				status;

    if (count == 0 || count > ACM_CDEV_COUNT)
    { return -EINVAL; }

    gs_cdev_driver = gs_acm_cdev_alloc_driver(count);
    if (!gs_cdev_driver)
    { return -ENOMEM; }

    gs_cdev_driver->owner = THIS_MODULE;
    gs_cdev_driver->driver_name = ACM_CDEV_DRV_NAME;
    gs_cdev_driver->name = ACM_CDEV_PREFIX;

    gs_acm_evt_init(&gs_acm_write_evt_manage, "write_evt");
    gs_acm_evt_init(&gs_acm_read_evt_manage, "read_evt");
    gs_acm_evt_init(&gs_acm_sig_stat_evt_manage, "sig_stat_evt");

    coding.dwDTERate = cpu_to_le32(115200);
    coding.bCharFormat = 8;
    coding.bParityType = USB_CDC_NO_PARITY;
    coding.bDataBits = USB_CDC_1_STOP_BITS;

    /* alloc and init each port */
    for (i = 0; i < count; i++)
    {
        mutex_init(&gs_acm_cdev_ports[i].open_close_lock);
        status = gs_acm_cdev_port_alloc(i, &coding);
        if (status)
        {
            count = i;
            goto setup_fail;
        }
    }
    gs_acm_cdev_n_ports = count;

    /* register the driver ... */
    status = gs_acm_cdev_register_driver(gs_cdev_driver);
    if (status)
    {
        DBG_E(MBB_ACM, " cannot register, err %d\n", status);

        goto setup_fail;
    }

    /* register devices ... */
    for (i = 0; i < count; i++)
    {
        struct device*	cdev;

        cdev = gs_acm_cdev_register_device(gs_cdev_driver, i, &g->dev);
        if (IS_ERR(cdev))
        {
            DBG_W(MBB_ACM, " no classdev for port %d, err %ld\n",
                  i, PTR_ERR(cdev));
            goto setup_fail;
        }
        gs_acm_cdev_ports[i].cdev = cdev;
    }

    gs_cdev_driver->acm_work_queue = create_singlethread_workqueue("acm_cdev");
    if (!gs_cdev_driver->acm_work_queue)
    {
        status = -ENOMEM;
        goto setup_fail;
    }

    /* we just regist once, and don't unregist any more */
    if (!gs_acm_nb_ptr)
    {
        gs_acm_nb_ptr = &gs_acm_nb;
        gs_acm_nb.priority = 0;
        gs_acm_nb.notifier_call = gs_acm_usb_notifier_cb;
        mbb_usb_register_notify(gs_acm_nb_ptr);
    }

    return status;

setup_fail:
    if (gs_cdev_driver->acm_work_queue)
    { destroy_workqueue(gs_cdev_driver->acm_work_queue); }

    if (count >= ACM_CDEV_COUNT)
    {
        DBG_E(MBB_ACM, "count is too big %d\n", count);
        count = ACM_CDEV_COUNT - 1;
    }

    while (count)
    {
        if (gs_acm_cdev_ports[count].port)
        {
            kfree(gs_acm_cdev_ports[count].port);
            gs_acm_cdev_ports[count].port = NULL;
        }

        if (gs_acm_cdev_ports[count].cdev)
        {
            gs_acm_cdev_unregister_device(gs_cdev_driver, count);
        }

        count--;
    }

    gs_acm_cdev_unregister_driver(gs_cdev_driver);

    if (gs_cdev_driver)
    {
        kfree(gs_cdev_driver);
        gs_cdev_driver = NULL;
    }
    return status;
}

/**
 * gacm_cdev_cleanup - remove cdev-over-USB driver and devices
 * Context: may sleep
 *
 * This is called to free all resources allocated by @gserial_setup().
 * Accordingly, it may need to wait until some open /dev/ files have
 * closed.
 *
 * The caller must have issued @gserial_disconnect() for any gs_acm_cdev_ports
 * that had previously been connected, so that there is never any
 * I/O pending when it's called.
 */
USB_VOID gacm_cdev_cleanup(USB_VOID)
{
    USB_UINT i;
    struct gs_acm_cdev_port*	port;

    if (!gs_cdev_driver)
    { return; }

    /* start sysfs and /dev/ttyGS* node removal */
    for (i = 0; i < gs_acm_cdev_n_ports; i++)
    { gs_acm_cdev_unregister_device(gs_cdev_driver, i); }

    for (i = 0; i < gs_acm_cdev_n_ports; i++)
    {
        /* prevent new opens */
        mutex_lock(&gs_acm_cdev_ports[i].open_close_lock);
        port = gs_acm_cdev_ports[i].port;

        mutex_unlock(&gs_acm_cdev_ports[i].open_close_lock);

        /* wait for old opens to finish */
        //wait_event(port->close_wait, gs_acm_cdev_closed(port));
        wait_event_interruptible(port->close_wait, gs_acm_cdev_closed(port));
        WARN_ON(port->port_usb != NULL);
        mutex_destroy(&port->write_lock);
        mutex_destroy(&port->read_lock);
        gs_acm_cdev_ports[i].port = NULL;

        cancel_delayed_work_sync(&port->rw_work);
        kfree(port);
    }
    gs_acm_cdev_n_ports = 0;

    gs_acm_cdev_unregister_driver(gs_cdev_driver);

    if (gs_cdev_driver->acm_work_queue)
    {
        flush_workqueue(gs_cdev_driver->acm_work_queue);
        destroy_workqueue(gs_cdev_driver->acm_work_queue);
        gs_cdev_driver->acm_work_queue = NULL;
    }

    if (gs_cdev_driver)
    {
        kfree(gs_cdev_driver);
        gs_cdev_driver = NULL;
    }
}


/*****************************************************************************
 函 数 名  : gs_acm_cdev_set_port_open_close
 功能描述  : 打开关闭GPS功能CTRL命令底层回调
*****************************************************************************/
USB_VOID gs_acm_cdev_set_port_open_close(struct usb_ep* ep,
        struct usb_request* req)
{
    struct f_acm*  acm = ep->driver_data;
    struct gs_acm_cdev_port* port = gs_acm_cdev_ports[acm->port_num].port;

    if (req->status != 0)
    {
        DBG_I(MBB_ACM, "acm ttyGS%d completion, err %d\n",
              acm->port_num, req->status);
        return;
    }

    /* normal completion */
    if (req->actual != sizeof(USB_UINT8))
    {
        DBG_I(MBB_ACM, "acm ttyGS%d short resp, len %d\n",
              acm->port_num, req->actual);
        usb_ep_set_halt(ep);
    }
    else
    {
        USB_UINT8* value = (USB_UINT8*)req->buf;

        if (USB_IF_PROTOCOL_GPS == acm->protocal)
        {
            if (ACM_CDEV_PORT_CLOSE == *value)
            {
                if (port->port_close_cb)
                {
                    port->port_close_cb();
                }
            }
            else if (ACM_CDEV_PORT_OPEN == *value)
            {
                if (port->port_open_cb)
                {
                    port->port_open_cb();
                }
            }
            else
            {
                DBG_I(MBB_ACM, "acm ttyGS%d short resp, value %u\n",
                      acm->port_num, *value);
                usb_ep_set_halt(ep);
            }
        }
    }
}



USB_INT32 acm_cdev_write_test(USB_UINT32 port_num, USB_UINT32 buf_size)
{
    struct sk_buff* skb_tx = NULL;
    struct gs_acm_cdev_port* port = NULL;
    USB_INT ret = 0;
    USB_PCHAR buf = NULL;

    if (port_num >= gs_acm_cdev_n_ports)
    {
        DBG_E(MBB_ACM, "invalid port num %u\n", port_num);
        return -1;
    }

    port = gs_acm_cdev_ports[port_num].port;
    if ((NULL == port) || (USB_TEST_PKT_LEN_MIN > buf_size) || (USB_TEST_PKT_LEN_MAX < buf_size))
    {
        DBG_E(MBB_ACM, "wrong param\n");
        return -1;
    }

    buf = kmalloc(buf_size, GFP_KERNEL);
    if (NULL == buf)
    {
        DBG_E(MBB_ACM, "alloc buf failed\n");
        return -1;
    }
    memset(buf, USB_NUM_BB, buf_size);

    ret = gs_acm_cdev_write_base(port, buf, NULL, buf_size, true);

    kfree(buf);

    return ret;
}

USB_VOID usb_cdev_init(USB_VOID)
{
    DBG_I(MBB_ACM, " %s:entry!\n", __func__);
    acm_ctx = acm_get_ctx();
    return;
}

void mbb_acm_cdev_dump(int port_num)
{
    struct gs_acm_cdev_port*	port;

    if (!gs_cdev_driver || port_num >= (int)gs_acm_cdev_n_ports)
    {
        DBG_E(MBB_ACM, "gacm_dump fail drv:%p, port_num:%d, n_ports:%d\n",
              gs_cdev_driver, port_num, gs_acm_cdev_n_ports);
        return;
    }

    port = gs_acm_cdev_ports[port_num].port;

    DBG_E(MBB_ACM, "=== dump stat dev ctx info ===\n");
    DBG_E(MBB_ACM, "build version:            %s\n", __VERSION__);
    DBG_E(MBB_ACM, "build date:               %s\n", __DATE__);
    DBG_E(MBB_ACM, "build time:               %s\n", __TIME__);
    DBG_E(MBB_ACM, "dev name                  %s\n",
          acm_ctx->cdev_name_type_var[port_num].name);
    DBG_E(MBB_ACM, "gs_stat_drv_invalid       %d\n", gs_stat_drv_invalid);
    DBG_E(MBB_ACM, "gs_stat_port_num_err      %d\n", gs_stat_port_num_err);
    if (NULL == port )
    {
        DBG_E(MBB_ACM, "port is NULL \n");
        return;
    }
    DBG_E(MBB_ACM, "build version:            %s\n", __VERSION__);
    DBG_E(MBB_ACM, "build date:               %s\n", __DATE__);
    DBG_E(MBB_ACM, "build time:               %s\n", __TIME__);
    DBG_E(MBB_ACM, "dev name                  %s\n", ACM_CDEV_GET_NAME(port_num));
    DBG_E(MBB_ACM, "gs_stat_drv_invalid       %d\n", gs_stat_drv_invalid);
    DBG_E(MBB_ACM, "gs_stat_port_num_err      %d\n", gs_stat_port_num_err);
    DBG_E(MBB_ACM, "open_count                %d\n", port->open_count);
    DBG_E(MBB_ACM, "openclose                 %d\n", port->openclose);
    DBG_E(MBB_ACM, "read_cb_null              %d\n", port->read_cb_null);
    DBG_E(MBB_ACM, "is_do_copy                %d\n", port->is_do_copy);
    DBG_E(MBB_ACM, "port_num                  %d\n", port->port_num);
    DBG_E(MBB_ACM, "line_state_on             %d\n", port->line_state_on);
    DBG_E(MBB_ACM, "line_state_change         %d\n", port->line_state_change);
    DBG_E(MBB_ACM, "sr_change                 %d\n", port->sr_change);
    DBG_E(MBB_ACM, "is_suspend                %d\n", port->is_suspend);
    DBG_E(MBB_ACM, "port_usb                  0x%x\n", (unsigned int)port->port_usb);
    DBG_E(MBB_ACM, "stat_port_is_connect      %d\n", port->stat_port_is_connect);
    DBG_E(MBB_ACM, "stat_discon_wakeup_block  %d\n", port->stat_discon_wakeup_block);
    DBG_E(MBB_ACM, "read_done_cb              %pS\n", port->read_done_cb);
    DBG_E(MBB_ACM, "write_done_cb             %pS\n", port->write_done_cb);
    acm_cdev_dump_ep_info(port);

    mdelay(10);
    DBG_E(MBB_ACM, "\n=== dump stat read info ===\n");
    DBG_E(MBB_ACM, "read_started              %d\n", port->read_started);
    DBG_E(MBB_ACM, "read_allocated            %d\n", port->read_allocated);
    DBG_E(MBB_ACM, "read_req_enqueued         %d\n", port->read_req_enqueued);
    DBG_E(MBB_ACM, "read_req_num              %d\n", port->read_req_num);
    DBG_E(MBB_ACM, "read_buf_size             %d\n", port->read_buf_size);
    DBG_E(MBB_ACM, "read_completed            %d\n", port->read_completed);
    DBG_E(MBB_ACM, "reading_pos               %d\n", port->reading_pos);

    DBG_E(MBB_ACM, "\n=== dump rx status info ===\n");
    DBG_E(MBB_ACM, "stat_read_call            %d\n", port->stat_read_call);
    DBG_E(MBB_ACM, "stat_get_buf_call         %d\n", port->stat_get_buf_call);
    DBG_E(MBB_ACM, "stat_ret_buf_call         %d\n", port->stat_ret_buf_call);
    DBG_E(MBB_ACM, "stat_read_param_err       %d\n", port->stat_read_param_err);
    DBG_E(MBB_ACM, "read_blocked              %d\n", port->read_blocked);
    DBG_E(MBB_ACM, "stat_sync_rx_submit       %d\n", port->stat_sync_rx_submit);
    DBG_E(MBB_ACM, "stat_sync_rx_done         %d\n", port->stat_sync_rx_done);
    DBG_E(MBB_ACM, "stat_sync_rx_done_fail    %d\n", port->stat_sync_rx_done_fail);
    DBG_E(MBB_ACM, "stat_sync_rx_done_bytes   %d\n", port->stat_sync_rx_done_bytes);
    DBG_E(MBB_ACM, "stat_sync_rx_copy_fail    %d\n", port->stat_sync_rx_copy_fail);
    DBG_E(MBB_ACM, "stat_sync_rx_disconnect   %d\n", port->stat_sync_rx_disconnect);
    DBG_E(MBB_ACM, "stat_sync_rx_wait_fail    %d\n", port->stat_sync_rx_wait_fail);
    DBG_E(MBB_ACM, "stat_rx_submit            %d\n", port->stat_rx_submit);
    DBG_E(MBB_ACM, "stat_rx_submit_fail       %d\n", port->stat_rx_submit_fail);
    DBG_E(MBB_ACM, "stat_rx_disconnect        %d\n", port->stat_rx_disconnect);
    DBG_E(MBB_ACM, "stat_rx_no_req            %d\n", port->stat_rx_no_req);
    DBG_E(MBB_ACM, "stat_rx_done              %d\n", port->stat_rx_done);
    DBG_E(MBB_ACM, "stat_rx_done_fail         %d\n", port->stat_rx_done_fail);
    DBG_E(MBB_ACM, "stat_rx_done_bytes        %d\n", port->stat_rx_done_bytes);
    DBG_E(MBB_ACM, "stat_rx_done_disconnect   %d\n", port->stat_rx_done_disconnect);
    DBG_E(MBB_ACM, "stat_read_cb_proc         %d\n", port->stat_read_cb_proc);
    DBG_E(MBB_ACM, "stat_rx_done_schdule      %d\n", port->stat_rx_done_schdule);
    DBG_E(MBB_ACM, "stat_rx_wakeup_block      %d\n", port->stat_rx_wakeup_block);
    DBG_E(MBB_ACM, "stat_rx_wakeup_realloc    %d\n", port->stat_rx_wakeup_realloc);
    DBG_E(MBB_ACM, "stat_rx_callback          %d\n", port->stat_rx_callback);
    DBG_E(MBB_ACM, "stat_rx_no_callback       %d\n", port->stat_rx_no_callback);
    DBG_E(MBB_ACM, "stat_rx_no_cb_pos         %d\n", port->stat_rx_no_cb_pos);
    DBG_E(MBB_ACM, "stat_rx_trig_callback     %d\n", port->stat_rx_trig_callback);
    DBG_E(MBB_ACM, "stat_rx_cb_not_start      %d\n", port->stat_rx_cb_not_start);
    DBG_E(MBB_ACM, "stat_rx_dequeue           %d\n", port->stat_rx_dequeue);
    DBG_E(MBB_ACM, "stat_rx_done_list_num     %d\n", port->stat_rx_done_list_num);

    mdelay(10);
    DBG_E(MBB_ACM, "\n=== dump stat write info ===\n");
    DBG_E(MBB_ACM, "write_req_num             %d\n", port->write_req_num);
    DBG_E(MBB_ACM, "write_started             %d\n", port->write_started);
    DBG_E(MBB_ACM, "write_completed           %d\n", port->write_completed);
    DBG_E(MBB_ACM, "write_allocated           %d\n", port->write_allocated);
    DBG_E(MBB_ACM, "write_blocked             %d\n", port->write_blocked);
    DBG_E(MBB_ACM, "write_block_status        %d\n", port->write_block_status);

    DBG_E(MBB_ACM, "\n=== dump tx status info ===\n");
    DBG_E(MBB_ACM, "stat_write_async_call     %d\n", port->stat_write_async_call);
    DBG_E(MBB_ACM, "stat_write_param_err      %d\n", port->stat_write_param_err);
    DBG_E(MBB_ACM, "stat_sync_tx_submit       %d\n", port->stat_sync_tx_submit);
    DBG_E(MBB_ACM, "stat_sync_tx_done         %d\n", port->stat_sync_tx_done);
    DBG_E(MBB_ACM, "stat_sync_tx_fail         %d\n", port->stat_sync_tx_fail);
    DBG_E(MBB_ACM, "stat_sync_tx_wait_fail    %d\n", port->stat_sync_tx_wait_fail);
    DBG_E(MBB_ACM, "stat_tx_submit            %d\n", port->stat_tx_submit);
    DBG_E(MBB_ACM, "stat_tx_submit_fail       %d\n", port->stat_tx_submit_fail);
    DBG_E(MBB_ACM, "stat_tx_submit_bytes      %d\n", port->stat_tx_submit_bytes);
    DBG_E(MBB_ACM, "stat_tx_done              %d\n", port->stat_tx_done);
    DBG_E(MBB_ACM, "stat_tx_by_virt           %d\n", port->stat_tx_by_virt);
    DBG_E(MBB_ACM, "stat_tx_by_phy            %d\n", port->stat_tx_by_phy);
    DBG_E(MBB_ACM, "stat_tx_done_fail         %d\n", port->stat_tx_done_fail);
    DBG_E(MBB_ACM, "stat_tx_done_bytes        %d\n", port->stat_tx_done_bytes);
    DBG_E(MBB_ACM, "stat_tx_done_schdule      %d\n", port->stat_tx_done_schdule);
    DBG_E(MBB_ACM, "stat_tx_done_disconnect   %d\n", port->stat_tx_done_disconnect);
    DBG_E(MBB_ACM, "stat_tx_wakeup_block      %d\n", port->stat_tx_wakeup_block);
    DBG_E(MBB_ACM, "stat_tx_callback          %d\n", port->stat_tx_callback);
    DBG_E(MBB_ACM, "stat_tx_no_callback       %d\n", port->stat_tx_no_callback);
    DBG_E(MBB_ACM, "stat_tx_no_cb_pos         %d\n", port->stat_tx_no_cb_pos);
    DBG_E(MBB_ACM, "stat_tx_no_req            %d\n", port->stat_tx_no_req);
    DBG_E(MBB_ACM, "stat_tx_copy_fail         %d\n", port->stat_tx_copy_fail);
    DBG_E(MBB_ACM, "stat_tx_alloc_fail        %d\n", port->stat_tx_alloc_fail);
    DBG_E(MBB_ACM, "stat_tx_suspend           %d\n", port->stat_tx_suspend);
    DBG_E(MBB_ACM, "stat_tx_disconnect        %d\n", port->stat_tx_disconnect);
    DBG_E(MBB_ACM, "stat_suspend        %d\n", port->stat_suspend);
    DBG_E(MBB_ACM, "stat_resume        %d\n", port->stat_resume);

    mdelay(10);
    DBG_E(MBB_ACM, "\n=== dump port status info ===\n");
    DBG_E(MBB_ACM, "stat_port_connect         %d\n", port->stat_port_connect);
    DBG_E(MBB_ACM, "stat_port_disconnect      %d\n", port->stat_port_disconnect);
    DBG_E(MBB_ACM, "stat_enable_in_fail       %d\n", port->stat_enable_in_fail);
    DBG_E(MBB_ACM, "stat_enable_out_fail      %d\n", port->stat_enable_out_fail);
    DBG_E(MBB_ACM, "stat_notify_sched         %d\n", port->stat_notify_sched);
    DBG_E(MBB_ACM, "stat_notify_on_cnt        %d\n", port->stat_notify_on_cnt);
    DBG_E(MBB_ACM, "stat_notify_off_cnt       %d\n", port->stat_notify_off_cnt);
    DBG_E(MBB_ACM, "stat_wait_port_close      %d\n", port->stat_wait_port_close);
    DBG_E(MBB_ACM, "stat_port_closed          %d\n", port->stat_port_closed);
}

