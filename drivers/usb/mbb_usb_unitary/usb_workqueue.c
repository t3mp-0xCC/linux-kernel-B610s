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

#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/export.h>
#include <linux/delay.h>
#include <linux/fcntl.h>
#include <linux/namei.h>
#include <linux/syscalls.h>
#include <linux/wait.h>
#include <linux/wakelock.h>

#include "usb_workqueue.h"
#include "usb_config.h"
#include "usb_debug.h"

struct workqueue_struct* usb_notify_wq;       /*USB work queue*/

static struct wake_lock g_dwc_wakelock;

/*****************************************************************
Parameters    :
Return        :    ��
Description   :  ����ӿڣ�USBע��work.
*****************************************************************/
USB_INT adp_usb_queue_delay_work(struct delayed_work* dwork, USB_ULONG delay)
{
    USB_INT ret = 0;
    
    if(usb_notify_wq)
    {
        ret = queue_delayed_work(usb_notify_wq,dwork, msecs_to_jiffies(delay));
    }
    else
    {
        DBG_E(MBB_HOTPLUG, "workqueue err\n");
    }

    return ret;

}
EXPORT_SYMBOL(adp_usb_queue_delay_work);

/*****************************************************************
Parameters    :
Return        :    ��
Description   :  ֹͣUSB�Ĺ�������
*****************************************************************/
 /*�ӿ�*/
USB_VOID usb_stop_work(struct delayed_work* wk )
{
    /*cancel_work_sync��ȡ����Ӧ��work������������
    work�Ѿ���������ôcancel_work_sync��������ֱ��
    work��ɲ�ȡ����Ӧ��work  */
    if (NULL != wk && delayed_work_pending(wk))
    {
        cancel_delayed_work(wk);
    }
}
EXPORT_SYMBOL(usb_stop_work);

/*****************************************************************
Parameters    :
Return        :    ��
Description   :  ����USB�Ĺ�������
*****************************************************************/
 /*�ӿ�*/
 
USB_VOID  usb_start_work(struct delayed_work* wk, USB_INT timeout)
{
    adp_usb_queue_delay_work(wk,timeout);
}
EXPORT_SYMBOL(usb_start_work);

/*****************************************************************
Parameters    :
Return        :    ��
Description   :  USBϵͳ����
*****************************************************************/
 /*�ӿ�*/
 
USB_VOID usb_wake_unlock(USB_VOID)
{
#ifdef USB_E5
    USB_ULONG flags = 0;
    local_irq_save(flags);
    wake_unlock(&g_dwc_wakelock);
    local_irq_restore(flags);
#endif
}
EXPORT_SYMBOL(usb_wake_unlock);
/*****************************************************************
Parameters    :
Return        :    ��
Description   :  USB��ϵͳ��
*****************************************************************/
 /*�ӿ�*/

USB_VOID usb_wake_lock(USB_VOID)
{
    /*��e5�಻�ü���*/
#ifdef  USB_E5
    USB_ULONG flags = 0;
    local_irq_save(flags);
    wake_lock(&g_dwc_wakelock);
    local_irq_restore(flags);
#endif
}
EXPORT_SYMBOL(usb_wake_lock);
/*****************************************************************
Parameters    :
Return        :    ��
Description   :  USB hotplugģ���ʼ��
*****************************************************************/
USB_INT usb_workqueue_init(USB_VOID)
{
    USB_INT ret = 0;

    usb_notify_wq = create_singlethread_workqueue("usb_notify");
    if (!usb_notify_wq)
    {
        DBG_E(MBB_HOTPLUG, "create_singlethread_workqueue fail\n");
        ret = -1;
    }
    
    /*����usb ��wakelock��*/
    wake_lock_init(&g_dwc_wakelock, WAKE_LOCK_SUSPEND, "dwc3-wakelock");
    
    return ret;
}

