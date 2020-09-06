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


#define MAX_HDLEN                   20
#define MAX_INQUIRY_STRING          29 //8+16+4+1
#define SC_READ_DISC_INFORMATION        0x51


static USB_UINT8  pnp_get_configration_respond[] =
{
    0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x03, 0x04, 0x00, 0x08, 0x01, 0x00,
    0x00, 0x01, 0x07, 0x08, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x07, 0x04,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x04, 0x40, 0x00, 0x00, 0x00, 0x00, 0x10, 0x01, 0x08,
    0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1E, 0x09, 0x04, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x03, 0x00, 0x01, 0x05, 0x07, 0x04
};
static inline USB_INT __fsg_is_set(struct fsg_common* common,
                                   USB_CHAR const* func, USB_UINT line)
{
    struct mass_storage_function_config_data* configdata = common->private_data;
    struct fsg_state_count* fsg_state_for_common = &configdata->fsg_state;

    if (common->fsg)
    { return 1; }

    DBG_T(MBB_MASS, "common->fsg is NULL\n");
    fsg_state_for_common->fsg_free++;
    return 0;
}
#define fsg_is_set(common) likely(__fsg_is_set(common, __func__, __LINE__))
static USB_INT fsg_set_halt(struct fsg_dev* fsg, struct usb_ep* ep)
{
    USB_CHAR* name;
    struct mass_storage_function_config_data* configdata = fsg->common->private_data;
    struct fsg_state_count* fsg_state_for_common = &configdata->fsg_state;
    if (ep == fsg->bulk_in)
    {
        name = "bulk-in";
        fsg_state_for_common->bulk_in_set_halt++;
    }
    else if (ep == fsg->bulk_out)
    {
        name = "bulk-out";
        fsg_state_for_common->bulk_out_set_halt++;
    }
    else
    { name = (USB_CHAR*)ep->name; }

    DBG_I(MBB_MASS, "%s set halt\n", name);

    return usb_ep_set_halt(ep);
}

static USB_INT halt_bulk_in_endpoint(struct fsg_dev* fsg)
{
    USB_INT    rc;

    rc = fsg_set_halt(fsg, fsg->bulk_in);

    if (rc == -EAGAIN)
    { DBG_T(MBB_MASS, "delayed bulk-in endpoint halt\n"); }

    while (rc != 0)
    {
        if (rc != -EAGAIN)
        {
            DBG_W(MBB_MASS, "usb_ep_set_halt -> %d\n", rc);
            rc = 0;
            break;
        }

        /* Wait for a short time and then try again */
        if (msleep_interruptible(100) != 0)
        { return -EINTR; }

        rc = usb_ep_set_halt(fsg->bulk_in);
    }

    return rc;
}
USB_VOID set_inquiry_unit_serial_number_page(USB_PUINT8 serialnumberpage)
{
#ifdef  MBB_USB_RESERVED
    USB_PUINT8 serial_number = NULL;

    //serial_number = get_serial_number();
    if ( serial_number )
    {
        memcpy(serialnumberpage, serial_number, INQUIRY_PAGE_LENGTH);
    }
    else
    {
        memset(serialnumberpage, 0x20, INQUIRY_PAGE_LENGTH);
    }
#else
    memset(serialnumberpage, 0x20, INQUIRY_PAGE_LENGTH);
#endif

}

static USB_INT sleep_thread(struct fsg_common* common)
{
    USB_INT    rc = 0;

    /* Wait until a signal arrives or we are woken up */
    for (;;)
    {
        try_to_freeze();
        set_current_state(TASK_INTERRUPTIBLE);

        if (signal_pending(current))
        {
            rc = -EINTR;
            break;
        }

        if (common->thread_wakeup_needed)
        { break; }

        schedule();
    }

    __set_current_state(TASK_RUNNING);
    common->thread_wakeup_needed = 0;

    return rc;
}

/* Caller must hold fsg->lock */
static USB_VOID wakeup_thread(struct fsg_common* common)
{
    /* Tell the main thread that something has happened */
    common->thread_wakeup_needed = 1;

    if (common->thread_task)
    { wake_up_process(common->thread_task); }
}

USB_VOID  fsg_thread_exit(struct fsg_common* common)
{
    spin_lock(&common->lock);
    common->file_op = FSG_EXIT;
    wakeup_thread(common);
    spin_unlock(&common->lock);
}


/*****************************************************************************
 Prototype    : check_command
 Description  :
 Input        : struct fsg_common* common
                USB_INT cmnd_size
                enum data_direction data_dir
                USB_UINT mask
                USB_INT needs_medium
                USB_PCHAR name
 Output       : None
 Return Value : static USB_INT
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/21
    Author       :
    Modification : Created function

*****************************************************************************/
static USB_INT check_command(struct fsg_common* common, USB_INT cmnd_size,
                             enum data_direction data_dir, USB_INT needs_medium)
{
    USB_INT            lun = common->cmnd[1] >> 5;
    struct fsg_lun*        curlun;
    struct mass_storage_function_config_data* configdata = common->private_data;
    struct fsg_state_count* fsg_state_for_common = &configdata->fsg_state;
    /*
     * We can't reply at all until we know the correct data direction
     * and size.
     */
    if (common->data_size_from_cmnd == 0)
    { data_dir = DATA_DIR_NONE; }

    if (common->data_size < common->data_size_from_cmnd)
    {
        /*
         * Host data size < Device data size is a phase error.
         * Carry out the command, but only transfer as much as
         * we are allowed.
         */
        common->data_size_from_cmnd = common->data_size;
        common->phase_error = 1;
    }

    common->residue = common->data_size;
    common->usb_amount_left = common->data_size;

    /* Conflicting data directions is a phase error */
    if (common->data_dir != data_dir && common->data_size_from_cmnd > 0)
    {
        common->phase_error = 1;
        DBG_W(MBB_MASS, "Conflicting data directions is a phase error \n");
        return -EINVAL;
    }

    /* Verify the length of the command itself */
    if (cmnd_size != common->cmnd_size)
    {

        /*
         * Special case workaround: There are plenty of buggy SCSI
         * implementations. Many have issues with cbw->Length
         * field passing a wrong command size. For those cases we
         * always try to work around the problem by using the length
         * sent by the host side provided it is at least as large
         * as the correct command length.
         * Examples of such cases would be MS-Windows, which issues
         * REQUEST SENSE with cbw->Length == 12 where it should
         * be 6, and xbox360 issuing INQUIRY, TEST UNIT READY and
         * REQUEST SENSE with cbw->Length == 10 where it should
         * be 6 as well.
         */
        if (cmnd_size <= common->cmnd_size)
        {
            /*
            DBG(common, "%s is buggy! Expected length %d "
                "but we got %d\n", name,
                cmnd_size, common->cmnd_size);
            */
            cmnd_size = common->cmnd_size;
        }
        else
        {
            common->phase_error = 1;
        DBG_W(MBB_MASS, "cmnd_size > common->cmnd_size \n");
            return -EINVAL;
        }
    }

    /* Check that the LUN values are consistent */
    if (common->lun != lun)
    {
        /*
        DBG(common, "using LUN %d from CBW, not LUN %d from CDB\n",
            common->lun, lun);
        */
        fsg_state_for_common->lun_values_not_consistent++;
    }

    /* Check the LUN */
    curlun = common->curlun;

    if (curlun)
    {
        if (common->cmnd[0] != REQUEST_SENSE)
        {
            curlun->sense_data = SS_NO_SENSE;
            curlun->sense_data_info = 0;
            curlun->info_valid = 0;
        }
    }
    else
    {
        common->bad_lun_okay = 0;
        /*
         * INQUIRY and REQUEST SENSE commands are explicitly allowed
         * to use unsupported LUNs; all others may not.
         */
        if (common->cmnd[0] != INQUIRY &&
            common->cmnd[0] != REQUEST_SENSE)
        {
            DBG_T(MBB_MASS, "unsupported LUN %d\n", common->lun);
            return -EINVAL;
        }
    }

    /*
     * If a unit attention condition exists, only INQUIRY and
     * REQUEST SENSE commands are allowed; anything else must fail.
     */
    if (curlun && curlun->unit_attention_data != SS_NO_SENSE &&
        common->cmnd[0] != INQUIRY &&
        common->cmnd[0] != REQUEST_SENSE)
    {
        curlun->sense_data = curlun->unit_attention_data;
        curlun->unit_attention_data = SS_NO_SENSE;
     DBG_W(MBB_MASS, " only INQUIRY and* REQUEST SENSE commands are allowed \n");    
        return -EINVAL;
    }

    /* Check that only command bytes listed in the mask are non-zero */
    common->cmnd[1] &= 0x1f;            /* Mask away the LUN */

    /* If the medium isn't mounted and the command needs to access
     * it, return an error. */
    if (curlun && !fsg_lun_is_open(curlun) && needs_medium)
    {
        curlun->sense_data = SS_MEDIUM_NOT_PRESENT;
     DBG_W(MBB_MASS, " needs_medium \n");
        return -EINVAL;
    }

    return 0;
}

static USB_INT check_command_size_in_blocks(struct fsg_common* common,
        USB_INT cmnd_size, enum data_direction data_dir, USB_INT needs_medium)
{
    if (common->curlun)
    {
        common->data_size_from_cmnd <<= common->curlun->blkbits;
    }
    return check_command(common, cmnd_size, data_dir, needs_medium);
}

static USB_VOID send_status(struct fsg_common* common)
{
    struct fsg_lun*        curlun = common->curlun;
    struct bulk_cs_wrap*    csw;
    USB_UINT8            status = US_BULK_STAT_OK;
    USB_UINT32            sd;
    unsigned long flags;
    USB_INT ret = 0;
    struct usb_request* req;
    struct mass_storage_function_config_data* configdata = common->private_data;
    struct fsg_state_count* fsg_state_for_common = &configdata->fsg_state;
    if (curlun)
    {
        sd = curlun->sense_data;
    }
    else if (common->bad_lun_okay)
    {
        sd = SS_NO_SENSE;
    }
    else
    {
        sd = SS_LOGICAL_UNIT_NOT_SUPPORTED;
    }

    if (common->phase_error)
    {
        status = US_BULK_STAT_PHASE;
        fsg_state_for_common->phase_error++;
    }
    else if (sd != SS_NO_SENSE)
    {
        status = US_BULK_STAT_FAIL;
        fsg_state_for_common->command_failure++;
    }

    /* Store and send the Bulk-only CSW */
    memset(common->csw_buf , 0 , US_BULK_CS_WRAP_LEN );
    csw = common->csw_buf;
    csw->Signature = cpu_to_le32(US_BULK_CS_SIGN);
    csw->Tag = common->tag;
    csw->Residue = cpu_to_le32(common->residue);
    csw->Status = status;
    spin_lock_irqsave(&common->tx_lock,flags);

    if (list_empty(&common->tx_req_pool))
    {
        spin_unlock_irqrestore(&common->tx_lock,flags);
        return;
    }    
    
    req = list_entry(common->tx_req_pool.next, struct usb_request, list);
    list_del(&req->list);
    req->length = US_BULK_CS_WRAP_LEN;
    req->buf = common->csw_buf;
    req->zero = 0;
    spin_unlock_irqrestore(&common->tx_lock,flags);
    ret = usb_ep_queue(common->fsg->bulk_in, req, GFP_ATOMIC);
    if (ret)
    {
        DBG_T(MBB_MASS, "tx failed\n");
        spin_lock_irqsave(&common->tx_lock,flags);
        memset(req->buf , 0 , req->actual);
        req->actual = 0;
        list_add_tail(&req->list, &common->tx_req_pool);
        spin_unlock_irqrestore(&common->tx_lock,flags);
    }
    fsg_state_for_common->csw++;

    return ;
}

static USB_VOID bulk_out_to_throw_complete(struct usb_ep* ep, struct usb_request* req)
{
    /*throw_away_data*/
    memset(req->buf,0,FSG_BUFLEN);
}

static int throw_away_data(struct fsg_common* common)
{
    struct usb_request* req;
    USB_INT ret = -1;
    
    req = common->throw_data_req;

    ret = usb_ep_queue(common->fsg->bulk_out, req, GFP_ATOMIC);
    if (ret)
    {
        DBG_E(MBB_MASS, "usb_ep_queue err\n");
        return ret;
    }
    return 0;
}

static USB_VOID fsg_send_data_to_host(struct fsg_common* common , USB_INT len)
{
    USB_INT length = 0;

    length = min((USB_UINT)len, common->data_size_from_cmnd);
    common->residue -= length;
    DBG_I(MBB_MASS, " residue %d length %d\n", common->residue ,length);
    if (0 == common->data_size )
    {
        /* Nothing to send */
        DBG_I(MBB_MASS, "Nothing to send \n");
    }
    else if(!common->residue)
    {
         fsg_start_tx(common ,length , 0);
    }
    else
    {
        fsg_start_tx(common ,length , 0);
        if (common->can_stall && ! length)
        { 
            DBG_I(MBB_MASS, "send stall to host \n");
            if(halt_bulk_in_endpoint(common->fsg))
            {
                DBG_E(MBB_MASS, "send stall failed \n");
            }
        }
    }
}
/*************************************************************************************
SCSI command   begin
**************************************************************************************/
static USB_INT do_unknow_cmd(struct fsg_common* common)
{
    struct mass_storage_function_config_data* configdata = common->private_data;
    struct fsg_state_count* fsg_state_for_common = &configdata->fsg_state;

    DBG_I(MBB_MASS, "enter\n");
    common->data_size_from_cmnd = 0;
    if (0 == check_command(common, common->cmnd_size, DATA_DIR_UNKNOWN, 0))
    {
        common->curlun->sense_data = SS_INVALID_COMMAND;
    }
    fsg_send_data_to_host(common , 0);
    fsg_state_for_common->unknown_cbw++;
    return 0;
}

static USB_INT do_inquiry(struct fsg_common* common)
{
    struct fsg_lun* curlun = common->curlun;
    USB_INT result = 0;
    USB_PUINT8 buf = (USB_PUINT8) common->buf;
    DBG_I(MBB_MASS, "\n");
    if (common->cmnd[4] > ((sizeof(common->inquiry_string) - 1) + 8))
    {
        common->cmnd[4] = (sizeof(common->inquiry_string) - 1) + 8;
    }

    /*bit 7 6 5 means lun*/
    common->cmnd[1] &= 0xe0;
    /*Page code*/
    common->cmnd[2] = 0;
    /*Reserved*/
    common->cmnd[3] = 0;

    common->data_size = common->cmnd[4];

    common->data_size_from_cmnd = common->cmnd[4];

    if ( check_command(common, 6, DATA_DIR_TO_HOST,  0))
    {
        DBG_T(MBB_MASS, "check_command err\n");
        goto send_data;
    }

    if (!curlun)          /* Unsupported LUNs are okay */
    {
        common->bad_lun_okay = 1;
        memset(buf, 0, 36);
        buf[0] = 0x7f;        /* Unsupported, no device-type */
        buf[4] = 31;        /* Additional length */
        result = 36;
        goto send_data;
    }

    /*Vital product data parameters return*/
    if ( ENABLE_VITAL_PRODUCT_DATA == common->cmnd[1] && UNIT_SERIAL_NUMBER_PAGE == common->cmnd[2])
    {
        /*converity 错误CURRENTLUN_CONNECT_OR_UNABLE_TO_DETERMINE 恒为0*/
        //buf[0] = (CURRENTLUN_CONNECT_OR_UNABLE_TO_DETERMINE) && (curlun->cdrom ? TYPE_ROM : TYPE_DISK);
        buf[0]= (curlun->cdrom ? TYPE_ROM : TYPE_DISK);
        buf[1] = UNIT_SERIAL_NUMBER_PAGE;
        buf[2] = RESERVED;
        buf[3] = INQUIRY_PAGE_LENGTH;
        set_inquiry_unit_serial_number_page(&buf[4]);
        result = UNIT_SERIAL_NUMBER_PAGE_LENGTH;
        goto send_data;
    }

    buf[0] = curlun->cdrom ? TYPE_ROM : TYPE_DISK;// 05 CDROM
    buf[1] = curlun->removable ? 0x80 : 0; // 80可移除，0不可移除
    buf[2] = 2; /* ANSI SCSI level 2 */
    buf[3] = 2; /* SCSI-2 INQUIRY data format */
    buf[4] = 31;/* Additional length */
    buf[5] = 0; /* No special options */
    buf[6] = 0;
    buf[7] = 0;
    memcpy(buf + USB_NUM_8, curlun->inquiry_string, sizeof curlun->inquiry_string);  //28+8=36
    result = 36;
send_data:    
    fsg_send_data_to_host(common , result);
    return 0;
}



static USB_INT do_mode_select(struct fsg_common* common)
{
    struct fsg_lun*    curlun = common->curlun;
    USB_INT    reply = -EINVAL;
    USB_INT         scsi_id = 0;
    USB_INT throw_data  = -1;
    scsi_id = common->cmnd[0];
    DBG_I(MBB_MASS, "\n");
    if ( MODE_SELECT == scsi_id)
    {
        common->data_size_from_cmnd = common->cmnd[4];
        reply = check_command(common, 6, DATA_DIR_FROM_HOST, 0);
    }
    else if (MODE_SELECT_10 == scsi_id)
    {
        common->data_size_from_cmnd = get_unaligned_be16(&common->cmnd[7]);
        reply = check_command(common, 10, DATA_DIR_FROM_HOST, 0);
    }
    throw_data = throw_away_data(common);
    if (throw_data)
    {
        DBG_E(MBB_MASS, "throw_data %d \n",throw_data);
    }
    if (reply)
    {
           return 0;
    }

    /* We don't support MODE SELECT */
    if (curlun)
    {
        curlun->sense_data = SS_INVALID_COMMAND;
    }

    return 0;

}


static USB_INT do_mode_sense(struct fsg_common* common)
{
    struct fsg_lun*    curlun = common->curlun;
    USB_INT               mscmnd = common->cmnd[0];
    USB_PUINT8         buf = (USB_PUINT8) common->buf;
    USB_PUINT8         buf0 = buf;
    USB_INT              pc, page_code;
    USB_INT              changeable_values, all_pages;
    USB_INT              valid_page = 0;
    USB_INT              len = 0;
    USB_INT              limit = 0;

    USB_INT    reply = -EINVAL;
    DBG_I(MBB_MASS, "\n");
    mscmnd = common->cmnd[0];

    if ( MODE_SENSE == mscmnd)
    {
        common->data_size_from_cmnd = common->cmnd[4];
        reply = check_command(common, 6, DATA_DIR_TO_HOST, 0);
    }
    else if (MODE_SENSE_10 == mscmnd)
    {
        common->data_size_from_cmnd = get_unaligned_be16(&common->cmnd[7]);
        reply = check_command(common, 10, DATA_DIR_TO_HOST,0);
    }

    if ( reply)
    {
        DBG_T(MBB_MASS, "check_cmd failed \n");
        len = 0;
        goto send_data;
    }

    if ((common->cmnd[1] & ~0x08) != 0)      /* Mask away DBD */
    {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        DBG_T(MBB_MASS, "SS_INVALID_FIELD_IN_CDB\n");
        len = 0;
        goto send_data;
    }

    pc = common->cmnd[2] >> 6;
    page_code = common->cmnd[2] & 0x3f;

    if (pc == 3)
    {
        curlun->sense_data = SS_SAVING_PARAMETERS_NOT_SUPPORTED;
        DBG_T(MBB_MASS, "SS_SAVING_PARAMETERS_NOT_SUPPORTED\n");
        len = 0;
        goto send_data;
    }

    changeable_values = (pc == 1);
    all_pages = (page_code == 0x3f);

    /*
     * Write the mode parameter header.  Fixed values are: default
     * medium type, no cache control (DPOFUA), and no block descriptors.
     * The only variable value is the WriteProtect bit.  We will fill in
     * the mode data length later.
     */
    memset(buf, 0, 8);

    if (mscmnd == MODE_SENSE)
    {
        buf[2] = (curlun->ro ? 0x80 : 0x00);        /* WP, DPOFUA */
        buf += 4;
        limit = 255;
    }
    else                                                      /*MODE_SENSE_10 */
    {
        buf[3] = (curlun->ro ? 0x80 : 0x00);        /* WP, DPOFUA */
        buf += 8;
        limit = 65535;                                        /* Should really be FSG_BUFLEN */
    }

    /* No block descriptors */

    /*
     * The mode pages, in numerical order.  The only page we support
     * is the Caching page.
     */
    if (page_code == 0x08 || all_pages)
    {
        valid_page = 1;
        buf[0] = 0x08;                                  /* Page code */
        buf[1] = 10;                                  /* Page length */
        memset(buf + 2, 0, 10);                       /* None of the fields are changeable */

        if (!changeable_values)
        {
            buf[2] = 0x04;    /* Write cache enable, */
            /* Read cache not disabled */
            /* No cache retention priorities */
            put_unaligned_be16(0xffff, &buf[4]);
            /* Don't disable prefetch */
            /* Minimum prefetch = 0 */
            put_unaligned_be16(0xffff, &buf[8]);
            /* Maximum prefetch */
            put_unaligned_be16(0xffff, &buf[10]);
            /* Maximum prefetch ceiling */
        }

        buf += 12;
    }

    /*
     * Check that a valid page was requested and the mode data length
     * isn't too long.
     */
    len = buf - buf0;

    if (!valid_page || len > limit)
    {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        DBG_W(MBB_MASS, "!valid_page/%d || len/%d > limit/%d \n",
              valid_page, len, limit);
        len = 0;
        goto send_data;
    }

    /*  Store the mode data length */
    if (mscmnd == MODE_SENSE)
    {
        buf0[0] = len - 1;
    }
    else
    {
        put_unaligned_be16(len - 2, buf0);
    }
send_data:
    fsg_send_data_to_host(common , len);
    return 0;
}


static USB_INT do_prevent_allow(struct fsg_common* common)
{
    struct fsg_lun*    curlun = common->curlun;
    USB_INT        prevent = 0;
    DBG_I(MBB_MASS, "enter !\n");
    common->data_size_from_cmnd = 0;
    if (check_command(common, 6, DATA_DIR_NONE, 0))
    {
        DBG_E(MBB_MASS, "check cmd failed !\n");
        return 0;
    }

    if (!common->curlun)
    {
        DBG_E(MBB_MASS, "curlun err  !\n");
        return 0;
    }
    else if (!common->curlun->removable)
    {
        common->curlun->sense_data = SS_INVALID_COMMAND;
        DBG_E(MBB_MASS, "SS_INVALID_COMMAND  !\n");
        return 0;
    }

    prevent = common->cmnd[4] & 0x01;

    if (prevent)
    {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        DBG_E(MBB_MASS, "prevent = 1  !\n");
        return 0;
    }

    if ((common->cmnd[4] & ~0x01) != 0)      /* Mask away Prevent */
    {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        DBG_E(MBB_MASS, "err  !\n");
        return 0;
    }

    if (curlun->prevent_medium_removal && !prevent)
    {
        fsg_lun_fsync_sub(curlun);
    }

    curlun->prevent_medium_removal = prevent;
    return 0;
}

static USB_INT do_read_capacity(struct fsg_common* common)
{
    struct fsg_lun*    curlun = common->curlun;
    USB_UINT32           lba = get_unaligned_be32(&common->cmnd[2]);
    USB_INT              pmi = common->cmnd[8];
    USB_PUINT8         buf = (USB_PUINT8)common->buf;
    USB_INT              len = 0;
    DBG_I(MBB_MASS, "enter  !\n");
    common->data_size_from_cmnd = 8;
    if (  check_command(common, 10, DATA_DIR_TO_HOST,  1))
    {
        DBG_W(MBB_MASS, "check cmd failed  !\n");
        len = 0;
        goto send_data;
    }
    /* Check the PMI and LBA fields */
    if (pmi > 1 || (pmi == 0 && lba != 0))
    {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        DBG_W(MBB_MASS, "SS_INVALID_FIELD_IN_CDB  !\n");
        len = 0;
        goto send_data;
    }

    put_unaligned_be32(curlun->num_sectors - 1, &buf[0]);
    /* Max logical block */
    put_unaligned_be32(curlun->blksize, &buf[4]);/* Block length */
    len = 8;
send_data:
    fsg_send_data_to_host(common , len);
    return 0;
}


static USB_INT do_read_header(struct fsg_common* common)
{
    struct fsg_lun*    curlun = common->curlun;
    USB_INT               msf = common->cmnd[1] & 0x02;
    USB_UINT32           lba = get_unaligned_be32(&common->cmnd[2]);
    USB_PUINT8          buf = (USB_PUINT8)common->buf;
    USB_INT               len = 0;
    DBG_I(MBB_MASS, "enter  !\n");
    if (!common->curlun || !common->curlun->cdrom)
    {
        DBG_E(MBB_MASS, "do_unknow_cmd  !\n");
        return do_unknow_cmd(common);
    }
    common->data_size_from_cmnd = get_unaligned_be16(&common->cmnd[7]);
    if ( check_command(common, 10, DATA_DIR_TO_HOST,1))
    {
        DBG_E(MBB_MASS, "check cmd err  !\n");
        len = 0;
        goto send_data;
    }

    if (common->cmnd[1] & ~0x02)          /* Mask away MSF */
    {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        DBG_E(MBB_MASS, "SS_INVALID_FIELD_IN_CDB  !\n");
        len = 0;
        goto send_data;
    }

    if (lba >= curlun->num_sectors)
    {
        curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
        DBG_E(MBB_MASS, "lba >= curlun->num_sectors  !\n");
        len = 0;
        goto send_data;
    }

    memset(buf, 0, 8);
    buf[0] = 0x01;        /* 2048 bytes of app data, rest is EC */
    store_cdrom_address(&buf[4], msf, lba);
    len = 8;
send_data:
    fsg_send_data_to_host(common , len);
    return 0;
}


/*------------------------------------------------------------
  名称     : void mmc_set_addr(mmc_addr_t *addr, u32 lba, int is_msf)
  描述     : 计算msf参数
  输入     : addr 数据存放地址  lab 逻辑地址 is_msf 编码格式
  输出     : 无
  返回值   : 无
-------------------------------------------------------------*/
static void mmc_set_addr(mmc_addr_t* addr, u32 lba, int is_msf)
{
    if (!is_msf)
    {
        put_unaligned_be32( lba, &addr->lba );
        return;
    }

    /* Calculate MSF */
    addr->msf.reserved = 0;
    addr->msf.frame = lba % 75;
    lba /= 75;  /* Got the frames. Now convert to seconds */
    lba += 2;   /* LBA 0 is always mapped to MSF 0/2/0 */
    addr->msf.min = lba / 60;
    addr->msf.sec = lba % 60;
}

static USB_INT do_read_toc(struct fsg_common* common)
{
    struct fsg_lun*  curlun = common->curlun;
    int msf = 0;
    u8 mmc_format = 0;
    u8 sff8020_format = 0;
    u16 data_len = 4;
    signed int  i = 0;
    scsi_toc_t* toc = (scsi_toc_t*)(common->buf);
    track_param_t* track = NULL;
    USB_INT len = 0;
    DBG_I(MBB_MASS, "enter  !\n");
    
    if (!common->curlun || !common->curlun->cdrom)
    {
        DBG_E(MBB_MASS, "do_unknow_cmd  !\n");
        return do_unknow_cmd(common);;
    }

    common->data_size_from_cmnd = get_unaligned_be16(&common->cmnd[7]);
    /* 根据SCSI协议，Read Toc 命令的有效字段是1、6、7、8、9，
    mask掩码的值应该是1111000010 */
    /* Code Checker结果说明，此处修改的mask掩码的值是通过十六
    进制数据进行移位获取的，并且为了和原代码格式保持一致
    ，Code Checker警告不处理 */
    if (  check_command(common, 10, DATA_DIR_TO_HOST, 1))
    {
        goto send_data;
    }

    if (!curlun)
    {
        DBG_E(MBB_MASS, "curlun  = 0!\n");
        return -1;
    }

    memset(common->buf, 0, ((common->data_size_from_cmnd == 0xFFFE) ?
                        512 : common->data_size_from_cmnd));

    /* need to be verified in further */
    /* modifieid end by mabinjie at 2010-12-06 */

    mmc_format = common->cmnd[2] & 0xf;
    msf = (common->cmnd[1] & 0x2) ? 1 : 0;

    /* SFF8020, version 1.2 specification define READ_TOC in a different way,
     * and it is relevant only when MMC format is zero */
    if (!mmc_format)
    {
        sff8020_format = (common->cmnd[9] & 0xC0) >> 6;
    }

    if (mmc_format > 2 || sff8020_format > 2)
    {
        DBG_E(MBB_MASS, "unsupported format [%u/%u]\n",mmc_format, sff8020_format);
        return -1 ;
    }

    /* First and last session / track number - relevant for all response
     * formats */
    toc->first_track = toc->last_track = 1;

    if (sff8020_format == 0x2 || mmc_format == 0x2)
    {
        session_param_t* session = (session_param_t*)toc->params;

        for (i = 0; i < 4; i++)
        {
            session[i].session_num = 1;
            session[i].adr_control = (0x1 << 4) | 0x4;
        }

        session[0].point = 0xA0;
        session[0].addr.msf.min = 1;

        session[1].point = 0xA1;
        session[1].addr.msf.min = 1;

        session[2].point = 0xA2;
        mmc_set_addr(&session[2].addr, curlun->num_sectors, 1);

        session[3].point = 0x01;

        data_len += 4 * SESSION_PARAM_SIZE;
        goto Done;
    }

    /* Fill the response format as specified in MMC6-r00, Table 482 */
    track = (track_param_t*)toc->params;

    track[0].adr_control = (0x1 << 4) | 0x4; /* Mode-1, Copy permitted */
    track[0].track_number = 1;
    mmc_set_addr(&track[0].addr,  0 , msf );

    data_len += sizeof(track_param_t);

    if (!sff8020_format || !mmc_format)
    {
        track[1].adr_control = (0x1 << 4) | 0x4; /* Mode-1, Copy permitted */
        track[1].track_number = 0xAA;               /* 0xAA == Lead out Area */
        mmc_set_addr(&track[1].addr, curlun->num_sectors, msf);

        data_len += sizeof(track_param_t);
    }

Done:
    /* MMC-r10a states that the data_len field size should be excluded */
    put_unaligned_be16(data_len - sizeof(toc->data_len), &toc->data_len);

    if ( data_len < common->data_size_from_cmnd )
    {
        common->data_size_from_cmnd = data_len;
    }
    common->data_size = common->data_size_from_cmnd;
    common->residue = common->data_size;
    common->usb_amount_left = common->data_size;

    len = common->data_size_from_cmnd;
send_data:
    fsg_send_data_to_host(common , len);
    return 0;
}


static USB_INT do_read_format_capacities(struct fsg_common* common)
{
    struct fsg_lun*    curlun = common->curlun;
    USB_PUINT8 buf = (USB_PUINT8) common->buf;
    USB_INT len = 0;
    DBG_I(MBB_MASS, "enter  !\n");
    common->data_size_from_cmnd = get_unaligned_be16(&common->cmnd[7]);
    if (check_command(common, 10, DATA_DIR_TO_HOST, 1))
    {
        DBG_T(MBB_MASS, "check cmd err  !\n");
        goto send_data;
    }
    buf[0] = buf[1] = buf[2] = 0;
    buf[3] = 8;                                                             /* Only the Current/Maximum Capacity Descriptor */
    buf += 4;

    put_unaligned_be32(curlun->num_sectors, &buf[0]);
    /* Number of blocks */
    put_unaligned_be32(curlun->blksize, &buf[4]);          /* Block length */
    buf[4] = 0x02;                                               /* Current capacity */
    len = 12;
send_data:
    fsg_send_data_to_host(common , len);
    return 0;
}


static USB_INT do_request_sense(struct fsg_common* common)
{
    struct fsg_lun*    curlun = common->curlun;
    USB_PUINT8 buf = (USB_PUINT8) common->buf;
    USB_UINT32        sd, sdinfo;
    USB_INT        valid;
    USB_INT        len = 0;

    DBG_I(MBB_MASS, "enter!\n");
    common->data_size_from_cmnd = common->cmnd[4];
    if ( check_command(common, 6, DATA_DIR_TO_HOST, 0))
    {
        DBG_E(MBB_MASS, "check cmd err  !\n");
        goto send_data;
    }

    if (!curlun)          /* Unsupported LUNs are okay */
    {
        common->bad_lun_okay = 1;
        sd = SS_LOGICAL_UNIT_NOT_SUPPORTED;
        sdinfo = 0;
        valid = 0;
    }
    else
    {
        sd = curlun->sense_data;
        sdinfo = curlun->sense_data_info;
        valid = curlun->info_valid << 7;
        curlun->sense_data = SS_NO_SENSE;
        curlun->sense_data_info = 0;
        curlun->info_valid = 0;
    }

    memset(buf, 0, 18);
    buf[0] = valid | 0x70;            /* Valid, current error */
    buf[2] = SK(sd);
    put_unaligned_be32(sdinfo, &buf[3]);    /* Sense information */
    buf[7] = 18 - 8;            /* Additional sense length */
    buf[12] = ASC(sd);
    buf[13] = ASCQ(sd);
    len = 18;
send_data:
    fsg_send_data_to_host(common , len);
    return 0;
}


static USB_INT do_start_stop(struct fsg_common* common)
{
    struct fsg_lun*    curlun = common->curlun;
    USB_INT        loej, start;
    common->data_size_from_cmnd = 0;

    DBG_I(MBB_MASS, "enter!\n");
    if ( check_command(common, 6, DATA_DIR_NONE,  0))
    {
        return -EINVAL;
    }

    if (!curlun)
    {
        return -EINVAL;
    }
    else if (!curlun->removable)
    {
        curlun->sense_data = SS_INVALID_COMMAND;
        return -EINVAL;
    }
    else if ((common->cmnd[1] & ~0x01) != 0 ||   /* Mask away Immed */
             (common->cmnd[4] & ~0x03) != 0)   /* Mask LoEj, Start */
    {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return -EINVAL;
    }

    loej  = common->cmnd[4] & 0x02;
    start = common->cmnd[4] & 0x01;

    /*
     * Our emulation doesn't support mounting; the medium is
     * available for use as soon as it is loaded.
     */
    if (start)
    {
        if (!fsg_lun_is_open(curlun))
        {
            curlun->sense_data = SS_MEDIUM_NOT_PRESENT;
            return -EINVAL;
        }

        return 0;
    }

    /* Are we allowed to unload the media? */
    if (curlun->prevent_medium_removal)
    {
        DBG_E(MBB_MASS, "unload attempt prevented\n");
        curlun->sense_data = SS_MEDIUM_REMOVAL_PREVENTED;
        return -EINVAL;
    }

    if (!loej)
    { return 0; }

    /* Simulate an unload/eject */
    if (common->ops && common->ops->pre_eject)
    {
        USB_INT r = common->ops->pre_eject(common, curlun,
                                           curlun - common->luns);

        if (unlikely(r < 0))
        { return r; }
        else if (r)
        { return 0; }
    }

    up_read(&common->filesem);
    down_write(&common->filesem);
    fsg_lun_close(curlun);
    up_write(&common->filesem);
    down_read(&common->filesem);

    return common->ops && common->ops->post_eject
           ? min(0, common->ops->post_eject(common, curlun,
                                            curlun - common->luns))
           : 0;
}

static USB_INT do_synchronize_cache(struct fsg_common* common)
{
    struct fsg_lun*    curlun = common->curlun;
    USB_INT        rc;
    common->data_size_from_cmnd = 0;
    DBG_I(MBB_MASS, "enter!\n");
    if ( check_command(common, 10, DATA_DIR_NONE, 1))
    {
        return -EINVAL;
    }
    /* We ignore the requested LBA and write out all file's
     * dirty data buffers. */
    rc = fsg_lun_fsync_sub(curlun);

    if (rc)
    { curlun->sense_data = SS_WRITE_ERROR; }

    return 0;
}

static USB_INT do_test_unit(struct fsg_common* common)
{
    common->data_size_from_cmnd = 0;
    if ( 0 != check_command(common, 6, DATA_DIR_NONE, 1))
    {
        DBG_W(MBB_MASS, "check cmd err  !\n");
        goto send_data;
    }
    common->residue = 0x00000000;//for mbb and pc interface 2.3 pnp
send_data:
    fsg_send_data_to_host(common , 0);
    return 0;
}

static USB_VOID invalidate_sub(struct fsg_lun* curlun)
{
    struct file*    filp = curlun->filp;
    struct inode*    inode = filp->f_path.dentry->d_inode;
    USB_UINT32    rc;

    rc = invalidate_mapping_pages(inode->i_mapping, 0, -1);
    DBG_I(MBB_MASS, "invalidate_mapping_pages -> %ld\n", rc);
}


static USB_INT do_verify(struct fsg_common* common)
{
    struct fsg_lun*        curlun = common->curlun;
    USB_UINT32        lba;
    USB_UINT32        verification_length;
    loff_t                   file_offset;
    USB_UINT            amount_left;
    
    DBG_T(MBB_MASS, "\n");
    common->data_size_from_cmnd = 0;
    if ( check_command(common, 10, DATA_DIR_NONE, 1))
    {
        DBG_E(MBB_MASS, "check cmd err\n");
        return 0;
    }
    /*
     * Get the starting Logical Block Address and check that it's
     * not too big.
     */
    lba = get_unaligned_be32(&common->cmnd[2]);

    if (lba >= curlun->num_sectors)
    {
        curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
        return 0;
    }

    /*
     * We allow DPO (Disable Page Out = don't save data in the
     * cache) but we don't implement it.
     */
    if (common->cmnd[1] & ~0x10)
    {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return 0;
    }

    verification_length = get_unaligned_be16(&common->cmnd[7]);

    if (unlikely(verification_length == 0))
    { return -EIO; }        /* No default reply */

    /* Prepare to carry out the file verify */
    amount_left = verification_length << curlun->blkbits;
    file_offset = ((loff_t) lba) << curlun->blkbits;

    /* Write out all the dirty buffers before invalidating them */
    fsg_lun_fsync_sub(curlun);

    invalidate_sub(curlun);

    if (file_offset + amount_left > curlun->file_length)
    {
        DBG_E(MBB_MASS, "verfiy err bad read parametes\n");
        return -EINTR;
    }
    else
    {
        return 0;
    }

    /* Just try to read the requested blocks */
    return 0;
}

static USB_INT pad_with_zeros(struct fsg_common* common)
{
    unsigned long flags;
    u32            nsend;
    int            ret = 0;
    struct usb_request* req;
    DBG_I(MBB_MASS, "enter \n");

    common->usb_amount_left =  common->residue;
    nsend = min(common->usb_amount_left, FSG_BUFLEN);
    while (common->usb_amount_left > 0)
    {
        spin_lock_irqsave(&common->tx_lock,flags);
        if (!list_empty(&common->tx_req_pool))
        {
            req = list_entry(common->tx_req_pool.next, struct usb_request, list);
            list_del(&req->list);
            req->buf = common->buf;
            memset(common->buf, 0, nsend);
            req->length = nsend;
            req->zero = 0;
            spin_unlock_irqrestore(&common->tx_lock,flags);
            ret = usb_ep_queue(common->fsg->bulk_in, req, GFP_ATOMIC);
            spin_lock_irqsave(&common->tx_lock,flags);
            if (ret)
            {
                DBG_T(MBB_MASS, "tx failed\n");
                list_add_tail(&req->list, &common->tx_req_pool);
                spin_unlock_irqrestore(&common->tx_lock,flags);
                break;
            }
            common->usb_amount_left -= nsend;
        }
        spin_unlock_irqrestore(&common->tx_lock,flags);
    }
    return 0;
}

static int do_XPWrite(struct fsg_common* common)
{
    USB_INT len = 0;
    USB_INT length = 0;
    struct fsg_lun*    curlun = common->curlun;
    DBG_I(MBB_MASS, "enter  ! \n");
    common->data_size_from_cmnd = 0;
    if ( 0 != check_command(common, 10, DATA_DIR_FROM_HOST, 0))
    {
        goto send_data;
    }
    common->bad_lun_okay = 1;
    curlun->sense_data = SS_NO_SENSE;
    common->phase_error = 0;

    len = common->data_size_from_cmnd;
    
    length = min((USB_UINT)len, common->data_size_from_cmnd);
    common->residue -= length;    
send_data:    
    return pad_with_zeros(common);
}


static USB_INT do_get_configuration(struct fsg_common* common)
{
    USB_PUINT8    buf = (USB_PUINT8) common->buf;
    usbsdms_get_configuration_cmd_type* get_configuration_cmd = NULL;
    USB_INT def_configuration_len  = sizeof(pnp_get_configration_respond);
    USB_INT response_length = 0;
    DBG_I(MBB_MASS, "\n");
    
    common->data_size_from_cmnd = get_unaligned_be16(&common->cmnd[USB_NUM_7]);
    
    get_configuration_cmd = (usbsdms_get_configuration_cmd_type*) & (common->cmnd[0]);

    if (get_configuration_cmd->allocation_length >= def_configuration_len)
    {
            response_length = def_configuration_len;
    }
    else
    {
            response_length = get_configuration_cmd->allocation_length;
    }

    memcpy(buf, pnp_get_configration_respond, response_length);

    if (response_length < common->data_size_from_cmnd)
    {
            common->data_size_from_cmnd = response_length;
    }

    common->data_size = common->data_size_from_cmnd;
    common->residue =  common->data_size;
    common->usb_amount_left = common->data_size;
    DBG_I(MBB_MASS, "response_length %d\n" , response_length);
    fsg_send_data_to_host(common, response_length);
    return 0;

}


static USB_INT do_rewind(struct fsg_common* common)
{
    struct fsg_lun* curlun = NULL;
    usb_pnp_api_hanlder* pnp_api_handler = pnp_adp_get_api_handler();
    USB_INT is_rewind_before = MBB_USB_FALSE;
    USB_INT power_off   = MBB_USB_NO;
    USB_INT fast_off   = MBB_USB_NO;
    power_off   = usb_power_off_chg_stat();
    fast_off    = usb_fast_on_off_stat();
    if (NULL == common)
    {
        DBG_E(MBB_MASS, "common null can't rewind\n");
        return 0;
    }

    curlun = common->curlun;

    if (NULL == curlun)
    {
        DBG_E(MBB_MASS, "NULL == curlun\n");
        return 0;
    }

    if (pnp_api_handler->pnp_is_rewind_before_mode_cb)
    {
        is_rewind_before = pnp_api_handler->pnp_is_rewind_before_mode_cb();
    }

    if ( !is_rewind_before || MBB_USB_YES == power_off || MBB_USB_YES == fast_off )
    {
        curlun->sense_data = SS_NO_SENSE;
        DBG_W(MBB_MASS, "can't rewind\n");
        return 0;
    }

    DBG_T(MBB_MASS, "do_rewind command");

    print_hex_dump(KERN_ERR, "", DUMP_PREFIX_OFFSET, MAX_COMMAND_SIZE, 1,
                   common->cmnd, MAX_COMMAND_SIZE, 0);

    if(pnp_api_handler->pnp_set_rewind_param_cb 
        && pnp_api_handler->pnp_switch_rewind_after_mode_cb)
    {
        pnp_api_handler->pnp_set_rewind_param_cb(&(common->cmnd[1]));
        pnp_api_handler->pnp_switch_rewind_after_mode_cb();
    }

    return 0;
}

USB_INT fsg_file_operation(struct fsg_common* common)
{
    USB_INT            reply = -EINVAL;
    USB_INT            i = 0;
    USB_INT               cmd_id = 0;
    USB_INT               err_stall = 0;
    cmd_id = common->cmnd[0];
    DBG_I(MBB_MASS, "cmd is %x!\n", cmd_id);
    switch(cmd_id)
    {
        case READ_6:
            err_stall = 1;
            i = common->cmnd[4];
            common->data_size_from_cmnd = (i == 0) ? 256 : i;

            reply = check_command_size_in_blocks(common, 6,DATA_DIR_TO_HOST,1);
            if (reply == 0)
            { 
                common->file_op= FSG_READ_EVENT;
            }
            break;
        case READ_10:
            err_stall = 1;
            common->data_size_from_cmnd = get_unaligned_be16(&common->cmnd[7]);
            reply = check_command_size_in_blocks(common, 10,DATA_DIR_TO_HOST,1);
            if (reply == 0)
            { 
                common->file_op= FSG_READ_EVENT;
            }
            break;
        case READ_12:
            err_stall = 1;
            common->data_size_from_cmnd =
            get_unaligned_be32(&common->cmnd[6]);
            reply = check_command_size_in_blocks(common, 12,DATA_DIR_TO_HOST,1);
            if (reply == 0)
            { 
                common->file_op= FSG_READ_EVENT;
            }
            break;
        case WRITE_6:
            i = common->cmnd[4];
            common->data_size_from_cmnd = (i == 0) ? 256 : i;
            reply = check_command_size_in_blocks(common, 6,DATA_DIR_FROM_HOST,1);
            if (reply == 0)
            { 
                common->file_op= FSG_WRITE_EVENT;
            }
            break;
        case WRITE_10:
            common->data_size_from_cmnd =
            get_unaligned_be16(&common->cmnd[7]);
            reply = check_command_size_in_blocks(common, 10,DATA_DIR_FROM_HOST,1);
            if (reply == 0)
            { 
                common->file_op= FSG_WRITE_EVENT;
            }
            break;
        case WRITE_12:
            common->data_size_from_cmnd =
            get_unaligned_be32(&common->cmnd[6]);
            reply = check_command_size_in_blocks(common, 12,DATA_DIR_FROM_HOST,1);
            if (reply == 0)
            { 
                common->file_op= FSG_WRITE_EVENT;
            }
            break;
        case START_STOP:
            common->data_size_from_cmnd = 0;

            reply = check_command(common, 6, DATA_DIR_NONE,0);
            if (reply == 0)
            { 
                common->file_op= FSG_START_STOP;
            }
            break;
        case SYNCHRONIZE_CACHE:
            common->data_size_from_cmnd = 0;
            reply = check_command(common, 10, DATA_DIR_NONE,1);

            if (reply == 0)
            { 
                common->file_op= FSG_SYNCHRONIZE_CACHE;
            }
            break;
        case VERIFY:
            common->data_size_from_cmnd = 0;
            reply = check_command(common, 10, DATA_DIR_NONE,1);

            if (reply == 0)
            { 
                common->file_op= FSG_VERIFY;
            }
            break;
        default:
            DBG_E(MBB_MASS, "SCSI CMD ERR!\n");
            break;
    }
    if(reply)
    {
        DBG_E(MBB_MASS, "check cmd err!\n");
        common->file_op = FSG_ERROR;
        if(err_stall)
        {
            fsg_send_data_to_host(common, 0 ); //需要回复stall的命令
        }
    }
    DBG_I(MBB_MASS, "common->file_op %d\n",common->file_op);
    spin_lock(&common->lock);
    wakeup_thread(common);
       spin_unlock(&common->lock);
    return 1;
}


static USB_INT wedge_bulk_in_endpoint(struct fsg_dev* fsg)
{
    USB_INT    rc;
    struct mass_storage_function_config_data* configdata = fsg->common->private_data;
    struct fsg_state_count* fsg_state_for_common = &configdata->fsg_state;
    DBG_I(MBB_MASS, "bulk-in set wedge\n");
    rc = usb_ep_set_wedge(fsg->bulk_in);

    if (rc == -EAGAIN)
    { DBG_E(MBB_MASS, "delayed bulk-in endpoint wedge\n"); }

    while (rc != 0)
    {
        if (rc != -EAGAIN)
        {
            DBG_W(MBB_MASS, "usb_ep_set_wedge -> %d\n", rc);
            rc = 0;
            break;
        }

        /* Wait for a short time and then try again */
        if (msleep_interruptible(100) != 0)
        { return -EINTR; }

        rc = usb_ep_set_wedge(fsg->bulk_in);
    }
    fsg_state_for_common->bulk_in_set_wedge++;
    return rc;
}

static USB_VOID read_tx(USB_INT length,struct usb_request *req)
{
    USB_INT ret = 0 ;
    struct fsg_common* common = req->context;
    unsigned long flags;
    struct fsg_dev    *fsg = NULL;
    if(0 == length)
    {
        list_add_tail(&req->list, &common->tx_pool);
        DBG_W(MBB_MASS," length = 0 \n"); 
        return ;
    }
    req->length = length;
    req->zero = 0;

    if (NULL == common)
    {
        return ;
    }
    fsg = common->fsg;
    if (NULL != fsg)
    {
        ret = usb_ep_queue(fsg->bulk_in, req, GFP_KERNEL);
        spin_lock_irqsave(&common->tx_lock, flags);
        if (ret)
        {
            DBG_T(MBB_MASS, "tx failed\n");
            list_add_tail(&req->list, &common->tx_pool);
        }
        spin_unlock_irqrestore(&common->tx_lock, flags);
    }
    return ;
}

static USB_VOID write_rx(struct fsg_common* common ,USB_INT length)
{
    USB_INT ret = 0 ;
    struct usb_request *req;
    unsigned long flags;
    struct fsg_dev    *fsg = NULL;
    if(0 == length)
    {
        DBG_T(MBB_MASS," length = 0 \n"); 
        return ;
    }
    spin_lock_irqsave(&common->rx_lock,flags);
    if(!list_empty(&common->rx_pool))
    {
        req = list_entry(common->rx_pool.next, struct usb_request, list);
        list_del(&req->list);
        req->length = length;
        req->zero = 0;
        spin_unlock_irqrestore(&common->rx_lock,flags);
        fsg = common->fsg;
        if (NULL != fsg)
        {
            ret = usb_ep_queue(fsg->bulk_out, req, GFP_KERNEL);
        }
        spin_lock_irqsave(&common->rx_lock, flags);
        if (ret)
        {
            DBG_T(MBB_MASS, "tx failed\n");
            list_add_tail(&req->list, &common->rx_pool);
        }
    }
    spin_unlock_irqrestore(&common->rx_lock,flags);
    return ;
}

static USB_VOID do_read_tx_complete(struct usb_ep* ep, struct usb_request* req)
{
    struct fsg_common*    common = ep->driver_data;
    if (req->status || req->actual != req->length)
    {
        DBG_E(MBB_MASS, "%d, %u/%u\n", req->status, req->actual, req->length);
    }

    if (req->status == -ESHUTDOWN)        /* disable was cancelled */
    {
        memset(req->buf , 0 , req->actual);
        list_add_tail(&req->list, &common->tx_pool);
        return;
    }
    if (req->status == -ECONNRESET)        /* Request was cancelled */
    {
        DBG_I(MBB_MASS, "req->status == -ECONNRESET\n");
        usb_ep_fifo_flush(ep);
    }
    smp_wmb();
    spin_lock(&common->tx_lock);
    memset(req->buf , 0 ,req->actual);
    req->actual = 0;
    list_add_tail(&req->list, &common->tx_pool);
    spin_unlock(&common->tx_lock);
    if(common->thread_need_wakeup )
    {
        DBG_W(MBB_MASS, "need_wakeup\n");
        common->thread_need_wakeup = 0;
        wakeup_thread(common);
    }
}

static USB_VOID do_write_rx_complete(struct usb_ep* ep, struct usb_request* req)
{
    struct fsg_common*    common = ep->driver_data;

    if (req->status || req->actual != req->length)
    {
        DBG_E(MBB_MASS, "%d, %u/%u\n", req->status, req->actual, req->length);
    }

    if (req->status == -ESHUTDOWN)        /* disable was cancelled */
    {
        DBG_T(MBB_MASS, " req->status == -ESHUTDOWN \n");
        memset(req->buf , 0 , req->actual);
        req->actual = 0;
        list_add_tail(&req->list, &common->rx_pool);
        return;
    }

    if (req->status == -ECONNRESET)        /* Request was cancelled */
    {
        DBG_T(MBB_MASS, "req->status == -ECONNRESET\n");
        usb_ep_fifo_flush(ep);
        memset(req->buf , 0 , req->actual);
        req->actual = 0;
        spin_lock(&common->rx_lock);
        list_add_tail(&req->list, &common->rx_pool);
        spin_unlock(&common->rx_lock);
        return;
    }
    smp_wmb();
    spin_lock(&common->rx_lock);
    list_add_tail(&req->list, &common->rx_queue);
    spin_unlock(&common->rx_lock);
    if (common->thread_need_wakeup )
    {
        DBG_I(MBB_MASS, "need_wakeup\n");
        common->thread_need_wakeup = 0;
        wakeup_thread(common);
    }
}

static struct usb_request * alloc_rw_req(struct usb_ep * ep ,unsigned size)
{
    struct usb_request *req;
    req = usb_ep_alloc_request(ep, GFP_ATOMIC);
    if(!req)
    {
        DBG_E(MBB_MASS, "fsg alloc req failed\n");
        return NULL;/*for coverity*/
    }
    req->length = size;
    req->buf = kmalloc(size, GFP_ATOMIC);
    if(!req->buf)
    {
        DBG_T(MBB_MASS, "fsg alloc req->buf failed\n");
        usb_ep_free_request( ep,  req);
        return NULL;
    }
    return req;
}

static int alloc_rw_requests(struct usb_ep *ep ,struct list_head *head,int num, int size, 
            void (*cb) (struct usb_ep *ep , struct usb_request * req) , void *context)
{
    int i = 0;
    struct usb_request *req;

    for (i = 0; i < num; i++) 
    {
        req = alloc_rw_req(ep, size);
        if (!req) 
        {
            pr_debug("req allocated:%d\n", i);
            return list_empty(head) ? -ENOMEM : 0;
        }
        req->context = context;
        req->complete = cb;
        list_add(&req->list, head);
    }

    return 0;
}
static void free_rw_requests(struct usb_ep *ep, struct list_head *head)
{
    struct usb_request    *req;

    while (!list_empty(head)) 
    {
        req = list_entry(head->next, struct usb_request, list);
        list_del(&req->list);
        kfree(req->buf);
        req->context = NULL;
        usb_ep_free_request(ep, req);
    }
}
static USB_INT do_read(struct fsg_common* common)
{
    struct fsg_lun*        curlun = common->curlun;
    USB_UINT32            lba;
    USB_UINT        amount_left;
    loff_t            file_offset, file_offset_tmp;
    USB_UINT        amount;
    ssize_t            nread;
    USB_INT              len = 0;
    USB_INT              rc = 0;
    unsigned long flags;
    struct usb_request *req;
    DBG_I(MBB_MASS, "\n");
    
    /*
     * Get the starting Logical Block Address and check that it's
     * not too big.
     */
     common->thread_need_wakeup = 0;

    
    if (common->cmnd[0] == READ_6)
    { 
        lba = get_unaligned_be24(&common->cmnd[1]); 
    }
    else
    {
        lba = get_unaligned_be32(&common->cmnd[2]);

        /*
         * We allow DPO (Disable Page Out = don't save data in the
         * cache) and FUA (Force Unit Access = don't read from the
         * cache), but we don't implement them.
         */
        if ((common->cmnd[1] & ~0x18) != 0)
        {
            curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
            DBG_W(MBB_MASS, "common->cmnd[1] & ~0x18) != 0\n");
            return 0;
        }
    }

    if (lba >= curlun->num_sectors)
    {
        curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
        DBG_W(MBB_MASS, "lba >= curlun->num_sectors\n");
        return 0;
    }

    file_offset = ((loff_t) lba) << curlun->blkbits;

    /* Carry out the file reads */
    amount_left = common->data_size_from_cmnd;
    DBG_I(MBB_MASS, "amount_left %d  data_size_from_cmnd %d\n" , amount_left, common->data_size_from_cmnd);
    if (unlikely(amount_left == 0))
    {
        DBG_W(MBB_MASS, "data_size_from_cmnd = 0\n");
        return 0;
    }

    for (;;)
        {
            /*
             * Figure out how much we need to read:
             * Try to read the remaining amount.
             * But don't read more than the buffer size.
             * And don't try to read past the end of the file.
             */
        amount = min(amount_left, FSG_BUFLEN);
        amount = min((loff_t)amount,curlun->file_length - file_offset);

        while(list_empty(&common->tx_pool)) 
        {
            common->thread_need_wakeup = 1;
            rc = sleep_thread(common);
            if (rc)
            {
                    curlun->sense_data = SS_COMMUNICATION_FAILURE;
                return 0;
            }
        }
        /*
        * If we were asked to read past the end of file,
        * end with an empty buffer.
        */
        if (amount == 0)
        {
            curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
            curlun->sense_data_info = file_offset >> curlun->blkbits;
            curlun->info_valid = 1;
            len = 0;
            DBG_W(MBB_MASS, "amount =0\n");
            break;
        }

        /* Perform the read */
        file_offset_tmp = file_offset;

        /*file removed randomly*/
        if ( NULL == curlun->filp )
        {
            DBG_E(MBB_MASS, "curlun->filp null stop to read!\n");
            curlun->sense_data = SS_UNRECOVERED_READ_ERROR;
            curlun->sense_data_info = file_offset >> curlun->blkbits;
            curlun->info_valid = 1;
            len = 0;
            break;
        }
        spin_lock_irqsave(&common->rx_lock , flags);
        req = list_entry(common->tx_pool.next, struct usb_request, list);
        list_del(&req->list);
        spin_unlock_irqrestore(&common->rx_lock,flags);
        nread = vfs_read(curlun->filp,(char __user*)req->buf,amount, &file_offset_tmp);
#ifdef USB_SD

        /*SD_CARD removed randomly*/
        if (!curlun->cdrom)
        {
            if ( SD_CARD_NOT_PRESENT == g_sd_card_info.sd_card_status
                 || SD_WORKMODE_MASS != g_sd_card_info.sd_card_workmode)
        {
            DBG_E(MBB_MASS, "SD_CARD_NOT_PRESENT! stop to read!\n");
            curlun->sense_data = SS_UNRECOVERED_READ_ERROR;
            curlun->sense_data_info = file_offset >> curlun->blkbits;
            curlun->info_valid = 1;
            len = 0;
            mass_storage_close_usb_sd();
            break;
        }
        }

#endif

        if (nread < 0)
        {
            DBG_W(MBB_MASS, "error in file read: %d\n", (USB_INT)nread);
            nread = 0;
        }
        else if (nread < amount)
        {
            DBG_E(MBB_MASS, "partial file read: %d/%u\n", (USB_INT)nread, amount);
            nread = round_down(nread, curlun->blksize);
        }

        file_offset  += nread;
        amount_left  -= nread;
        common->residue -= nread;

        len= nread;
        /* If an error occurred, report it and its position */
        if (nread < amount)
        {
            curlun->sense_data = SS_UNRECOVERED_READ_ERROR;
            curlun->sense_data_info = file_offset >> curlun->blkbits;
            curlun->info_valid = 1;
            DBG_E(MBB_MASS, "nread %d \n" , (USB_INT)nread);
            break;
        }
        DBG_I(MBB_MASS, "nread %d amount_left %d \n" , (USB_INT)nread, amount_left);
        read_tx( len , req);
        if (amount_left == 0)
        { 
            DBG_I(MBB_MASS, "all the date read compliete \n" );
            break; 
        }

    }

    return 0;        /* No default reply */
}


static USB_INT do_write(struct fsg_common* common)
{
    struct fsg_lun*        curlun = common->curlun;
    USB_UINT32            lba;
    USB_UINT            amount_left_to_write = 0;
    loff_t            usb_offset, file_offset, file_offset_tmp;
    USB_INT                 get_some_more = 0;
    USB_UINT        amount = 0;
    ssize_t            nwritten;
    USB_INT            rc = 0;
    USB_UINT               amount_left_to_req = 0;
    USB_INT               len = 0;
    struct usb_request *req;
    unsigned long flags;
    struct mass_storage_function_config_data* configdata = common->private_data;
    struct fsg_state_count* fsg_state_for_common = &configdata->fsg_state;
    if (NULL == curlun->filp)
    {
        curlun->sense_data = SS_MEDIUM_NOT_PRESENT;
        return 0;
    }
    if (curlun->ro)
    {
        curlun->sense_data = SS_WRITE_PROTECTED;
        return 0;
    }

    spin_lock(&curlun->filp->f_lock);
    curlun->filp->f_flags &= ~O_SYNC;    /* Default is not to wait */
    spin_unlock(&curlun->filp->f_lock);

    if (common->cmnd[0] == WRITE_6)
    { lba = get_unaligned_be24(&common->cmnd[1]); }
    else
    {
        lba = get_unaligned_be32(&common->cmnd[2]);

        /*
         * We allow DPO (Disable Page Out = don't save data in the
         * cache) and FUA (Force Unit Access = write directly to the
         * medium).  We don't implement DPO; we implement FUA by
         * performing synchronous output.
         */
        if (common->cmnd[1] & ~0x18)
        {
            curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
            DBG_T(MBB_MASS, "common->cmnd[1] & ~0x18\n");
            return -EINVAL;
        }

        if (!curlun->nofua && (common->cmnd[1] & 0x08))   /* FUA */
        {
            spin_lock(&curlun->filp->f_lock);
            curlun->filp->f_flags |= O_SYNC;
            spin_unlock(&curlun->filp->f_lock);
        }
    }

    if (lba >= curlun->num_sectors)
    {
        curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
        DBG_T(MBB_MASS, "lba >= curlun->num_sectors\n");
        return 0;
    }


    /* Carry out the file writes */
    get_some_more = 1;
    file_offset = usb_offset = ((loff_t) lba) << curlun->blkbits;
    amount_left_to_req = common->data_size_from_cmnd;
    amount_left_to_write = common->data_size_from_cmnd;
    DBG_I(MBB_MASS, "amount_left_to_req %d!\n",amount_left_to_req);
    if(amount_left_to_write > 0)
    {
        DBG_I(MBB_MASS, "first rx \n");
        len = min(amount_left_to_req , FSG_BUFLEN);
        write_rx(common ,len );
    }
    else
    {
        DBG_E(MBB_MASS, "empty write!! \n");
        return 0;
    }
    while (amount_left_to_write > 0)
    {
        if (list_empty(&common->rx_queue) )
        { 
              common->thread_need_wakeup = 1;
            DBG_I(MBB_MASS, " sleep_thread\n");
            rc = sleep_thread(common);
            if (rc)
            {
                curlun->sense_data = SS_COMMUNICATION_FAILURE;
                return 0;
            }
        }
        
        while (!list_empty(&common->rx_queue))
        {
            smp_rmb();
            spin_lock_irqsave(&common->rx_lock,flags);
            if(!list_empty(&common->rx_queue))
            {
                req = list_entry(common->rx_queue.next, struct usb_request, list);
                list_del(&req->list);
            }
            else
            {
                spin_unlock_irqrestore(&common->rx_lock , flags);
                break;
            }
            spin_unlock_irqrestore(&common->rx_lock , flags);
            /* Did something go wrong with the transfer? */
            if (req->status != 0)
            {
                curlun->sense_data = SS_COMMUNICATION_FAILURE;
                curlun->sense_data_info = file_offset >> curlun->blkbits;
                curlun->info_valid = 1;
                return 0;
            }
            amount = req->actual;
            usb_offset += amount;
            common->usb_amount_left -= amount;
            amount_left_to_req -= amount;
            if (usb_offset >= curlun->file_length)
            {
                curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
                curlun->sense_data_info = usb_offset >> curlun->blkbits;
                curlun->info_valid = 1;
                return 0;
            }
            DBG_I(MBB_MASS, "amount == %d!amount_left_to_req = %d\n",req->actual,amount_left_to_req);
            if(0 == amount_left_to_req)
            {
                get_some_more = 0;
            }
            else
            {
                len = min(amount_left_to_req , FSG_BUFLEN);
                write_rx(common ,len );
            }
            if (curlun->file_length - file_offset < amount)
            {
                DBG_E(MBB_MASS,"write %u @ %llu beyond end %llu\n",
                      amount, (USB_UINT64)file_offset, (USB_UINT64)curlun->file_length);
                amount = curlun->file_length - file_offset;
            }
            /* Don't write a partial block */
            amount = round_down(amount, curlun->blksize);

            if (amount == 0)
            { 
                fsg_state_for_common->short_packet_received++;
                DBG_E(MBB_MASS, "amount == 0!\n");
                spin_lock_irqsave(&common->rx_lock ,flags);
                memset(req->buf, 0 , req->actual);
                req->actual = 0;
                list_add_tail(&req->list, &common->rx_pool);
                spin_unlock_irqrestore(&common->rx_lock,flags);
                return 0;
            }

            /* Perform the write */
            file_offset_tmp = file_offset;

            /*file removed randomly*/
            if ( NULL == curlun->filp )
            {
                DBG_T(MBB_MASS, "curlun->filp null stop to write!\n");
                curlun->sense_data = SS_WRITE_ERROR;
                curlun->sense_data_info =
                file_offset >> curlun->blkbits;
                curlun->info_valid = 1;
                return 0;
            }

            nwritten = vfs_write(curlun->filp, (char __user*)req->buf,amount, &file_offset_tmp);
            DBG_I(MBB_MASS, "nwritten: %d\n",(USB_INT)nwritten);
#ifdef USB_SD

            /*SD_CARD removed randomly*/
            if (!curlun->cdrom)
            {
                if ( SD_CARD_NOT_PRESENT == g_sd_card_info.sd_card_status
                     || SD_WORKMODE_MASS != g_sd_card_info.sd_card_workmode)
                {
                    DBG_E(MBB_MASS, "SD_CARD_NOT_PRESENT! stop to write!\n");
                    curlun->sense_data = SS_WRITE_ERROR;
                    curlun->sense_data_info = file_offset >> curlun->blkbits;
                    curlun->info_valid = 1;
                    mass_storage_close_usb_sd();
                    return 0;
                }
            }

#endif
                DBG_I(MBB_MASS, "file write %u @ %llu -> %d\n", amount, (USB_UINT64)file_offset, (USB_INT)nwritten);

                if (nwritten < 0)
                {
                    DBG_E(MBB_MASS, "error in file write: %d\n",(USB_INT)nwritten);
                    nwritten = 0;
                }
                else if (nwritten < amount)
                {
                    DBG_E(MBB_MASS, "partial file write: %d/%u\n",(USB_INT)nwritten, amount);
                    nwritten = round_down(nwritten, curlun->blksize);
                }

                file_offset += nwritten;
                amount_left_to_write -= nwritten;
                common->residue -= nwritten;

                /* If an error occurred, report it and its position */
                if (nwritten < amount)
                {
                    curlun->sense_data = SS_WRITE_ERROR;
                    curlun->sense_data_info = file_offset >> curlun->blkbits;
                    curlun->info_valid = 1;
                    return 0;
                }

            spin_lock_irqsave(&common->rx_lock , flags);
            memset(req->buf, 0 , req->actual);
            req->actual = 0;
            list_add_tail(&req->list, &common->rx_pool);
            spin_unlock_irqrestore(&common->rx_lock , flags);
            
            if(0 == get_some_more)
            {
                DBG_I(MBB_MASS, "all of the data write compliete\n");
                break;
            }
        }

    }

    DBG_I(MBB_MASS, "exit\n");
    return 0;
}

typedef USB_INT (* SCSI_CMD)(struct fsg_common* common);
typedef struct
{
    USB_INT scsi_id;
    SCSI_CMD scsi_cmd_handle;
} SCSI_CMD_TYPE;

/*SCSI CMD SUPPORT LIST*/

const  SCSI_CMD_TYPE scsi_cmd_list[] =
{
    {INQUIRY , do_inquiry},
    {MODE_SELECT , do_mode_select},
    {MODE_SELECT_10 , do_mode_select},
    {MODE_SENSE , do_mode_sense},
    {MODE_SENSE_10 , do_mode_sense},
    {ALLOW_MEDIUM_REMOVAL , do_prevent_allow},
    {READ_6  , fsg_file_operation},
    {READ_10 , fsg_file_operation},
    {READ_12 , fsg_file_operation},
    {READ_CAPACITY , do_read_capacity},
    {READ_HEADER , do_read_header},
    {READ_TOC , do_read_toc},
    {READ_FORMAT_CAPACITIES , do_read_format_capacities},
    {REQUEST_SENSE , do_request_sense},
    {START_STOP , fsg_file_operation},
    {SYNCHRONIZE_CACHE , fsg_file_operation},
    {TEST_UNIT_READY , do_test_unit},
    {VERIFY , fsg_file_operation},
    {WRITE_6 , fsg_file_operation},
    {WRITE_10 , fsg_file_operation},
    {WRITE_12 , fsg_file_operation},
    {SC_READ_DISC_INFORMATION , do_XPWrite},
    {GET_CONFIGURATION_SCSI , do_get_configuration},
    {SC_REWIND_11 , do_rewind},
    {FORMAT_UNIT , NULL},
    {RELEASE , NULL},
    {RESERVE , NULL},
    {SEND_DIAGNOSTIC , NULL},
};



static USB_INT do_scsi_command(struct fsg_common* common)
{
    USB_INT            ret = 0;
    USB_INT            i = 0;
    USB_INT               cmd_id = 0;
    USB_INT               unknow_scsi = 0;
    struct mass_storage_function_config_data* configdata = common->private_data;
    struct fsg_state_count* fsg_state_for_common = &configdata->fsg_state;
    
    common->phase_error = 0;
    common->short_packet_received = 0;
    g_scsi_stat = 1;
    cmd_id = common->cmnd[0];
    DBG_I(MBB_MASS, "cmd_id :[%x]\n", cmd_id);
    
    for (i = 0 ; i < sizeof(scsi_cmd_list) / sizeof(SCSI_CMD_TYPE) ; i++)
    {
        if (scsi_cmd_list[i].scsi_id == cmd_id)
        {
            if ( NULL != scsi_cmd_list[i].scsi_cmd_handle)
            {
            ret = scsi_cmd_list[i].scsi_cmd_handle(common);
            fsg_state_for_common->cbw++;
            fsg_state_for_common->data++;
            }
            else
            {
                ret = do_unknow_cmd(common);
            }
          unknow_scsi = 1; //scsi 列表中存在这条命令
        }
    }
    if(0 == unknow_scsi)
    {
        ret = do_unknow_cmd(common);
    }
    return ret;
}



USB_VOID fsg_cmd_handle(struct fsg_common* common)
{
    USB_INT ret = 0;
#ifdef USB_SD

    if ( SD_CARD_NOT_PRESENT == g_sd_card_info.sd_card_status
         || SD_WORKMODE_MASS != g_sd_card_info.sd_card_workmode)
    {
        mass_storage_close_usb_sd();
    }

#endif
    
    ret = do_scsi_command(common);
    if(!ret)
    {
        send_status(common);
    }
    else if(1 == ret)
    {
        return;
    }
    fsg_start_rx(common);

}



static USB_INT fsg_rw_thread(USB_PVOID common_)
{
    struct fsg_common*    common = common_;

    USB_INT     reply = -EINVAL;

    allow_signal(SIGINT);
    allow_signal(SIGTERM);
    allow_signal(SIGKILL);
    allow_signal(SIGUSR1);

    /* Allow the thread to be frozen */
    set_freezable();

    set_fs(get_ds());

    while (common->file_op!= FSG_EXIT)
    {
        DBG_W(MBB_MASS, "wait for next rw cmd ! \n");
        (USB_VOID)sleep_thread(common);
        if (!common->running)
        {
            continue;
        }
        DBG_I(MBB_MASS, "file_op  event %d\n",common->file_op);
        down_read(&common->filesem);
        switch(common->file_op)
        {
    
            case FSG_READ_EVENT:
                reply = do_read(common);
                break;
            case FSG_WRITE_EVENT:
                reply = do_write(common);
                break;
            case FSG_VERIFY:
                reply = do_verify(common);
                break;
            case FSG_START_STOP:
                reply = do_start_stop(common);
                break;
            case FSG_SYNCHRONIZE_CACHE:
                reply = do_synchronize_cache(common);
                break;
            case FSG_ERROR:
                reply = 0;
                break;
            default:
                common->file_op = FSG_IO_IDEL;
                DBG_W(MBB_MASS, "wait rw cmd\n");
                continue;
        }
        up_read(&common->filesem);
        if(!reply )
        {
            send_status(common);
        }
        common->file_op = FSG_IO_IDEL;
        fsg_start_rx(common);
        DBG_I(MBB_MASS, "rw csw send success\n");

    }
    spin_lock_irq(&common->lock);
    common->thread_task = NULL;
    spin_unlock_irq(&common->lock);
    if (!common->ops || !common->ops->thread_exits || common->ops->thread_exits(common) < 0)
    {
        struct fsg_lun* curlun = common->luns;
        USB_UINT i = common->nluns;

        down_write(&common->filesem);

        for (; i--; ++curlun)
        {
            if (!fsg_lun_is_open(curlun))
            {
                continue;
            }

            fsg_lun_close(curlun);
            curlun->unit_attention_data = SS_MEDIUM_NOT_PRESENT;
        }
        up_write(&common->filesem);
    }

    /* Let fsg_unbind() know the thread has exited */
    complete_and_exit(&common->thread_notifier, 0);
    DBG_T(MBB_MASS, "thread ready exit\n");
    return 0;

}

