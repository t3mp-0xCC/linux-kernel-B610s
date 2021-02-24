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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/ioctl.h>
#include <asm/irq.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include "rndis_webapp_fd.h"
#include "usb_debug.h"
#include "hw_pnp_api.h"

#define UART_NR    8    /* Number of UARTs this driver can handle */

#define FIFO_SIZE    PAGE_SIZE  
#define WAKEUP_CHARS    256
#define RNDIS_DYNAMIC_MINOR 203




long rndis_webapp_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{

    /*����ж�*/
    if (NULL == file)
    {
        DBG_I(MBB_RNDIS, "inode error or file error\n");
        return RNDIS_RETURN_FAIL;
    }

    DBG_I(MBB_RNDIS, "rndis_webapp_ioctl come\n");

    switch (cmd)
    {
        case ENUM_FORCE_SWITCH_PROJECT_MODE:
            DBG_I(MBB_RNDIS, "\nrndis go to project_mode \n");
            pnp_switch_rndis_project_mode();
            //�л��ɹ���ģʽ
            break;
        case ENUM_FORCE_SWITCH_DEBUG_MODE:
            DBG_I(MBB_RNDIS, "\nrndis go to debug_mode \n");
            pnp_switch_rndis_debug_mode();
            //�л��ɵ���
            break;
        case ENUM_USB_RNDIS_SET_RATE:
            rndis_set_rate(arg);
            //��������
            break;
        case ENUM_APP_DHCP_TASK_START:
            rndis_send_connect_event_to_host();
            //֪ͨ��������
            break;
        default:
            break;
    }

    return RNDIS_RET_SUCCESS;

}


static const struct file_operations rndis_webapp_ops =
{
    THIS_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    rndis_webapp_ioctl,
    NULL,
    NULL,
    NULL,

};


static struct miscdevice rndis_app_fd =
{
    RNDIS_DYNAMIC_MINOR,
    "rndisapp",
    &rndis_webapp_ops
};

/*****************************************************************
Parameters    :  void
Return        :    
Description   :  ��ʼ��
*****************************************************************/
int  rndis_webapp_init(void)
{
    int ret = 0;

    ret = misc_register(&rndis_app_fd);

    if (ret < 0)
    {
        DBG_I(MBB_RNDIS, "rndis_webapp_init failed,ret = %x\n", ret);
    }

    else
    {
        DBG_I(MBB_RNDIS, "rndis_webapp_init success,ret = %x\n", ret);
    }

    return ret;
}


module_init(rndis_webapp_init);

