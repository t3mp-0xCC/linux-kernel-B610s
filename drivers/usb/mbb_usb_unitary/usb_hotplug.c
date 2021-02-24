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
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/wakelock.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <asm/mach/irq.h>
#include <linux/wait.h>

#include "usb_config.h"
#include "usb_debug.h"
#include "usb_platform_comm.h"
#include "hw_pnp_api.h"
#include "hw_pnp.h"
/*增加usb typec特性的支持*/
#ifdef USB_TYPEC_CTRL
#include "usb_typec_driver.h"
#endif

#ifdef MBB_USB_UNITARY_Q
#else
#include "usb_vendor.h"
#include "mbb_config.h"
#include "product_config.h"
#endif

#include "usb_workqueue.h"
#include "usb_charger_manager.h"
#include "usb_hotplug.h"

#define USB_DETECT_DELAY        200/*USB 中断防抖中断检测延时*/

/*拔出解锁控制，拔出时判断是否插入状态，选择是否解锁*/
static struct timer_list g_wake_unlock_timer ;

/*wake unlock锁定时器，默认3s*/
static USB_INT g_wake_unlockt_loop = 3000;

static usb_hotplug_datamodel_t  g_usb_notifier_h =
{
    .usb_status = MBB_USB_DEVICE_REMOVE,
    .usb_old_status = MBB_USB_DEVICE_REMOVE,
};

/**注册外部接口函数**/
#ifdef MBB_USB_UNITARY_Q
USB_INT hw_usb_entry(USB_VOID)
{
    pnp_probe();
    return 0;
}
USB_VOID hw_usb_exit(USB_VOID)
{
    pnp_remove();
}

static usb_hotplug_hanlder_t g_usb_hotplug_ctx =
{
    .usb_poweroff_cb =  0x00,
    .usb_poweron_cb = 0x00,
    .usb_entry_cb =       hw_usb_entry,
    .usb_exit_cb =          hw_usb_exit,
    .usbid_proc_cb = 0x00,
    .usb_wait_enumdone_cb = 0x00,
    .usb_check_wireless_chg_cb = 0x00,
    .usb_wireless_chg_remove_cb = 0x00,
    .usb_notify_wq_extern = 0x00,/*在init函数赋值不，在此初始化不符合C标准，编译错误*/
};

#else
static usb_hotplug_hanlder_t g_usb_hotplug_ctx =
{
    .usb_poweron_cb =                          power_on_dwc3_usb,
    .usb_poweroff_cb =                          power_off_dwc3_usb,
    .usb_entry_cb =                                mbb_usb_init,
    .usb_exit_cb =                                   mbb_usb_exit,
    .usbid_proc_cb =                                NULL,
    .usb_wait_enumdone_cb =               bsp_usb_wait_cdev_created,
    .usb_clear_enumdone_cb  =              bsp_usb_clear_last_cdev_name,
#if (FEATURE_ON == MBB_USB_CHARGE)
    .usb_check_wireless_chg_cb =          usb_chg_wireless_detect,
    .usb_wireless_chg_remove_cb =       usb_chg_wireless_remove,
#else
    .usb_check_wireless_chg_cb =          0x00,
    .usb_wireless_chg_remove_cb =       0x00,
#endif
    .usb_notify_wq_extern = 0x00,
};
#endif

/*****************************************************************
Parameters    :  USB_INT loop 定时器时间
Return        :    无
Description   :  设置wake unlock的定时器时间
*****************************************************************/
USB_VOID usb_set_wake_unlockt_loop( USB_INT loop)
{
    DBG_I(MBB_HOTPLUG, "set wake_unlockt_loop to : %d\n", loop);
    g_wake_unlockt_loop = loop;
}

/*****************************************************************
Parameters    :
Return        :    无
Description   :  USB插拔入口
*****************************************************************/
USB_VOID mbb_usb_status_change(USB_INT status)
{
    USB_ULONG timeout = 0;

    DBG_I(MBB_HOTPLUG, "status %d\n",   status);
    switch (status)
    {
        case MBB_USB_DEVICE_INSERT:
            timeout = USB_DETECT_DELAY;
            usb_wake_lock();
            g_usb_notifier_h.stat_usb_insert++;
            g_usb_notifier_h.stat_usb_insert_timestamp = jiffies;
            break;
        case MBB_USB_DEVICE_REMOVE :
            mod_timer(&g_wake_unlock_timer, jiffies + msecs_to_jiffies(g_wake_unlockt_loop));
            timeout = USB_DETECT_DELAY;
            g_usb_notifier_h.stat_usb_remove++;
            g_usb_notifier_h.stat_usb_remove_timestamp = jiffies;
            break;
        case MBB_USB_DEVICE_DISABLE:
            g_usb_notifier_h.stat_usb_disable++;
            break;
        case MBB_USB_ENUM_DONE :
            g_usb_notifier_h.stat_usb_enum_done++;
            break;
        
        default:
            DBG_T(MBB_HOTPLUG, "error status:%d\n",   status);
    }

    /*防抖处理*/
    /*pmu在电池满时有150ms 产生异常插拔的bug 200ms防抖作为限制*/
    /*作为设备时*/
    usb_stop_work(&g_usb_notifier_h.usb_notify_wk);
    usb_start_work(&g_usb_notifier_h.usb_notify_wk, timeout);
    g_usb_notifier_h.usb_status = status;

}

/*****************************************************************
Parameters    :
Return        :    无
Description   :  获取USB vbus 是否在位状态
*****************************************************************/
USB_INT usb_get_hotplug_status(USB_VOID)
{
    return g_usb_notifier_h.usb_status;
}
/*****************************************************************
Parameters    :
Return        :    无
Description   :  获取上一次USB动作
*****************************************************************/
USB_INT usb_get_hotplug_old_status(USB_VOID)
{
    return g_usb_notifier_h.usb_host_status;
}
/*****************************************************************
Parameters    :
Return        :    无
Description   :  获取插入时间戳
*****************************************************************/
USB_ULONG usb_get_insert_timestamp(USB_VOID)
{
    return g_usb_notifier_h.stat_usb_insert_timestamp;
}
/*****************************************************************
Parameters    :
Return        :    无
Description   :  获取拔出时间戳
*****************************************************************/
USB_ULONG usb_get_remove_timestamp(USB_VOID)
{
    return g_usb_notifier_h.stat_usb_remove_timestamp;
}

/*****************************************************************
Parameters    :
Return        :    无
Description   :  USB插入处理
*****************************************************************/
static USB_VOID mbb_usb_insert_process(USB_VOID)
{
    USB_ULONG flags = 0;

    mutex_lock(&g_usb_notifier_h.mutex);

    /*maybe changed the HW status so need save the irq */
    local_irq_save(flags);
    g_usb_notifier_h.stat_usb_insert_proc++;
    mlog_print("USB", USB_MLOG_INFO, "stat_usb_insert_proc:%d\n", g_usb_notifier_h.stat_usb_insert_proc);
    usb_broadcast_event(MBB_USB_DEVICE_INSERT);
    /*USB上电*/
    if (NULL != g_usb_hotplug_ctx.usb_poweron_cb)
    {
        if (g_usb_hotplug_ctx.usb_poweron_cb())
        {
            DBG_E(MBB_HOTPLUG, "power on dwc3 usb failed!\n");
            g_usb_notifier_h.stat_usb_poweron_fail++;
            return;
        }

    }
    /*GPIO控制制ID*/
    if (NULL != g_usb_hotplug_ctx.usbid_proc_cb)
    {
        g_usb_hotplug_ctx.usbid_proc_cb(1);
    }
    local_irq_restore(flags);


    /* init the usb driver */
    if (NULL != g_usb_hotplug_ctx.usb_entry_cb)
    {
        (void)g_usb_hotplug_ctx.usb_entry_cb();
    }

    /*通知充电类型*/
    usb_broadcast_event(MBB_USB_CHARGER_IDEN);

    g_usb_notifier_h.stat_usb_insert_proc_end++;
    mlog_print("USB", USB_MLOG_INFO, "stat_usb_insert_proc_end:%d\n", g_usb_notifier_h.stat_usb_insert_proc_end);
    mutex_unlock(&g_usb_notifier_h.mutex);
    return;
}
/*****************************************************************
Parameters    :
Return        :    无
Description   :  U盘插入处理
*****************************************************************/
static USB_VOID mbb_usb_perip_insert_process(USB_VOID)
{
    USB_INT status = -1;

    mutex_lock(&g_usb_notifier_h.mutex);

    g_usb_notifier_h.stat_usb_perip_insert_proc++;
    mlog_print("USB", USB_MLOG_INFO, "stat_usb_perip_insert_proc:%d\n", g_usb_notifier_h.stat_usb_perip_insert_proc);
    /* set usb notifier status in advance to evade unexpect pmu insert intr. */
    status = g_usb_notifier_h.usb_host_old_status;
    g_usb_notifier_h.usb_host_old_status = MBB_USB_PERIP_INSERT;

    /*usb 上电*/
    if (NULL !=  g_usb_hotplug_ctx.usb_poweron_cb)
    {
        if (g_usb_hotplug_ctx.usb_poweron_cb())
        {
            DBG_E(MBB_HOTPLUG, "power on dwc3 usb failed!\n");
            /* restore usb notifier status set in advance */
            g_usb_notifier_h.usb_host_old_status = status;
            g_usb_notifier_h.stat_usb_poweron_fail++;
            return;
        }
    }

    if (NULL !=  g_usb_hotplug_ctx.usbid_proc_cb)
    {
        (void)g_usb_hotplug_ctx.usbid_proc_cb(0);
    }

    /* init the usb driver */
    (void)g_usb_hotplug_ctx.usb_entry_cb();

    g_usb_notifier_h.stat_usb_perip_insert_proc_end++;
    mlog_print("USB", USB_MLOG_INFO, "stat_usb_perip_insert_proc_end:%d\n"
               , g_usb_notifier_h.stat_usb_perip_insert_proc_end);
    mutex_unlock(&g_usb_notifier_h.mutex);
    return;
}


/*****************************************************************
Parameters    :
Return        :    无
Description   :  枚举完成处理
*****************************************************************/
static USB_VOID mbb_usb_enum_done_process(USB_VOID)
{

    DBG_I(MBB_HOTPLUG, "balong usb enum done,it works well.\n");
    g_usb_notifier_h.stat_usb_enum_done_proc++;

    /*
     * wait for usb enum_done
     */
    if (NULL != g_usb_hotplug_ctx.usb_wait_enumdone_cb)
    {
        g_usb_hotplug_ctx.usb_wait_enumdone_cb();
    }

    /* notify kernel notifier */
    usb_broadcast_event(MBB_USB_ENUM_DONE);
    g_usb_notifier_h.stat_usb_enum_done_proc_end++;
}

/*****************************************************************
Parameters    :
Return        :    无
Description   :  USB设备拔出处理
*****************************************************************/
static USB_VOID mbb_usb_remove_device_process(USB_VOID)
{

    DBG_I(MBB_HOTPLUG, "balong usb remove.\n");

    mutex_lock(&g_usb_notifier_h.mutex);

    /*清空枚举信息*/
    if (NULL != g_usb_hotplug_ctx.usb_clear_enumdone_cb)
    {
        g_usb_hotplug_ctx.usb_clear_enumdone_cb();
    }
    g_usb_notifier_h.stat_usb_remove_proc++;
    mlog_print("USB", USB_MLOG_INFO, "stat_usb_remove_proc:%d\n", g_usb_notifier_h.stat_usb_remove_proc);
    /* notify kernel notifier,
     * we must call notifier list before disable callback,
     * there are something need to do before user
     */
    usb_broadcast_event(MBB_USB_DEVICE_REMOVE);
    g_usb_notifier_h.charger_type = USB_CHARGER_TYPE_INVALID;

    /* exit the usb driver */
    if (NULL != g_usb_hotplug_ctx.usb_exit_cb)
    {
        (void)g_usb_hotplug_ctx.usb_exit_cb();
    }
    mlog_print("USB", USB_MLOG_INFO, "stat_usb_remove_proc_end:%d\n", g_usb_notifier_h.stat_usb_remove_proc_end);
    if (NULL != g_usb_hotplug_ctx.usb_poweroff_cb)
    {
        if (g_usb_hotplug_ctx.usb_poweroff_cb())
        {
            DBG_E(MBB_HOTPLUG, KERN_ERR "fail to disable the dwc3 usb regulator\n");
            g_usb_notifier_h.stat_usb_poweroff_fail++;
        }
    }

    usb_wake_unlock();
    g_usb_notifier_h.stat_usb_remove_proc_end++;

    mutex_unlock(&g_usb_notifier_h.mutex);

    return;
}
/*****************************************************************
Parameters    :
Return        :    无
Description   :  U盘等设备拔出处理
*****************************************************************/
static USB_VOID mbb_usb_remove_perip_process(USB_VOID)
{

    DBG_I(MBB_HOTPLUG, "balong usb peripheral remove.\n");

    mutex_lock(&g_usb_notifier_h.mutex);

    g_usb_notifier_h.stat_usb_perip_remove_proc++;
    mlog_print("USB", USB_MLOG_INFO, "stat_usb_perip_remove_proc:%d\n", g_usb_notifier_h.stat_usb_perip_remove_proc);
    usb_broadcast_event(MBB_USB_PERIP_REMOVE);

    /* exit the usb driver */
    if (NULL != g_usb_hotplug_ctx.usb_exit_cb)
    {
        (void)g_usb_hotplug_ctx.usb_exit_cb();
    }

    /* power-off usb hardware */
    if (NULL != g_usb_hotplug_ctx.usb_poweroff_cb)
    {
        if (g_usb_hotplug_ctx.usb_poweroff_cb())
        {
            DBG_E(MBB_HOTPLUG, KERN_ERR "fail to disable the dwc3 usb regulator\n");
            g_usb_notifier_h.stat_usb_poweroff_fail++;
        }
    }

    //usb_wake_unlock();
    g_usb_notifier_h.stat_usb_perip_remove_proc_end++;
    g_usb_notifier_h.delta_time = 0;
    mlog_print("USB", USB_MLOG_INFO, "stat_usb_perip_remove_proc_end:%d\n"
               , g_usb_notifier_h.stat_usb_perip_remove_proc_end);
    mutex_unlock(&g_usb_notifier_h.mutex);

    return;
}
/*****************************************************************
Parameters    :
Return        :    无
Description   :  设备禁用处理
*****************************************************************/
static USB_VOID mbb_usb_disable_device_process(USB_VOID)
{

    DBG_I(MBB_HOTPLUG, "balong usb disable.\n");
    g_usb_notifier_h.stat_usb_disable_proc++;

    /*清空枚举信息*/
    if (NULL != g_usb_hotplug_ctx.usb_clear_enumdone_cb)
    {
        g_usb_hotplug_ctx.usb_clear_enumdone_cb();
    }

    /* notify kernel notifier */
    usb_broadcast_event( MBB_USB_DEVICE_DISABLE);
    g_usb_notifier_h.charger_type = USB_CHARGER_TYPE_INVALID;

    g_usb_notifier_h.stat_usb_disable_proc_end++;


    return;
}

/*****************************************************************
Parameters    :
Return        :    无
Description   :  设置充电类型
*****************************************************************/
USB_VOID mbb_usb_set_charger_type(USB_INT type)
{
    g_usb_notifier_h.charger_type = type;
    return;
}
/*****************************************************************
Parameters    :
Return        :    无
Description   :  获取充电类型
*****************************************************************/
int mbb_usb_get_charger_type(USB_VOID)
{
    return g_usb_notifier_h.charger_type;
}
/*****************************************************************
Parameters    :
Return        :    无
Description   :  可维可测拔插设备信息
*****************************************************************/
void usb_hotplug_dump(USB_VOID)
{
    if (USB_CHARGER_TYPE_HUAWEI == g_usb_notifier_h.charger_type)
    {
        DBG_T(MBB_HOTPLUG, "balong usb CHARGER_TYPE: HuaWei     :) \n");
    }
    else if (USB_CHARGER_TYPE_NOT_HUAWEI  == g_usb_notifier_h.charger_type)
    {
        DBG_T(MBB_HOTPLUG, "balong usb CHARGER_TYPE: Not HuaWei :( \n");
    }
    else
    {
        DBG_T(MBB_HOTPLUG, "balong usb CHARGER_TYPE: Invalid    +_+ \n");
    }
    DBG_T(MBB_HOTPLUG, "stat_usb_insert:            %d\n", g_usb_notifier_h.stat_usb_insert);
    DBG_T(MBB_HOTPLUG, "stat_usb_insert_proc:       %d\n", g_usb_notifier_h.stat_usb_insert_proc);
    DBG_T(MBB_HOTPLUG, "stat_usb_insert_proc_end:   %d\n", g_usb_notifier_h.stat_usb_insert_proc_end);
    DBG_T(MBB_HOTPLUG, "stat_usb_enum_done:         %d\n", g_usb_notifier_h.stat_usb_enum_done);
    DBG_T(MBB_HOTPLUG, "stat_usb_enum_done_proc:    %d\n", g_usb_notifier_h.stat_usb_enum_done_proc);
    DBG_T(MBB_HOTPLUG, "stat_usb_enum_done_proc_end:%d\n", g_usb_notifier_h.stat_usb_enum_done_proc_end);
    DBG_T(MBB_HOTPLUG, "stat_usb_remove:            %d\n", g_usb_notifier_h.stat_usb_remove);
    DBG_T(MBB_HOTPLUG, "stat_usb_remove_proc:       %d\n", g_usb_notifier_h.stat_usb_remove_proc);
    DBG_T(MBB_HOTPLUG, "stat_usb_remove_proc_end:   %d\n", g_usb_notifier_h.stat_usb_remove_proc_end);
    DBG_T(MBB_HOTPLUG, "stat_usb_disable:           %d\n", g_usb_notifier_h.stat_usb_disable);
    DBG_T(MBB_HOTPLUG, "stat_usb_disable_proc:      %d\n", g_usb_notifier_h.stat_usb_disable_proc);
    DBG_T(MBB_HOTPLUG, "stat_usb_disable_proc_end:  %d\n", g_usb_notifier_h.stat_usb_disable_proc_end);
    DBG_T(MBB_HOTPLUG, "usb_status:                 %d\n", g_usb_notifier_h.usb_status);
    DBG_T(MBB_HOTPLUG, "usb_old_status:             %d\n", g_usb_notifier_h.usb_old_status);
    DBG_T(MBB_HOTPLUG, "usb_hotplub_state:          %d\n", g_usb_notifier_h.usb_hotplub_state);
    DBG_T(MBB_HOTPLUG, "stat_usb_no_need_notify:    %d\n", g_usb_notifier_h.stat_usb_no_need_notify);
    DBG_T(MBB_HOTPLUG, "stat_usb_perip_insert:            %d\n", g_usb_notifier_h.stat_usb_perip_insert);
    DBG_T(MBB_HOTPLUG, "stat_usb_perip_insert_proc:       %d\n", g_usb_notifier_h.stat_usb_perip_insert_proc);
    DBG_T(MBB_HOTPLUG, "stat_usb_perip_insert_proc_end:   %d\n", g_usb_notifier_h.stat_usb_perip_insert_proc_end);
    DBG_T(MBB_HOTPLUG, "stat_usb_perip_remove:            %d\n", g_usb_notifier_h.stat_usb_perip_remove);
    DBG_T(MBB_HOTPLUG, "stat_usb_perip_remove_proc:       %d\n", g_usb_notifier_h.stat_usb_perip_remove_proc);
    DBG_T(MBB_HOTPLUG, "stat_usb_perip_remove_proc_end:   %d\n", g_usb_notifier_h.stat_usb_perip_remove_proc_end);
    DBG_T(MBB_HOTPLUG, "stat_usb_poweron_fail:    %d\n", g_usb_notifier_h.stat_usb_poweron_fail);
    DBG_T(MBB_HOTPLUG, "stat_usb_poweroff_fail:    %d\n", g_usb_notifier_h.stat_usb_poweroff_fail);
}

USB_VOID set_id_delta_time(int mstime)
{
    g_usb_notifier_h.delta_time = mstime;
}
/*
 * usb charger adapter implement
 */
USB_VOID mbb_usb_status_change_ex(int status)
{
    DBG_I(MBB_HOTPLUG, "status %d.\n", status);
    if (MBB_USB_PERIP_INSERT == status)
    {
        g_usb_notifier_h.stat_usb_perip_insert++;
    }
    else if (MBB_USB_PERIP_REMOVE == status)
    {
        g_usb_notifier_h.stat_usb_perip_remove++;
    }
    else
    {
        DBG_T(MBB_HOTPLUG, "error status:%d\n", status);
    }
    g_usb_notifier_h.usb_host_status = status;
    if (MBB_USB_PERIP_INSERT == status || MBB_USB_PERIP_REMOVE == status)
    {
        cancel_delayed_work(&g_usb_notifier_h.usb_notify_host_wk);
        queue_delayed_work(g_usb_notifier_h.usb_notify_wq,
                           &g_usb_notifier_h.usb_notify_host_wk, msecs_to_jiffies(USB_DETECT_DELAY));
    }
}

USB_VOID bsp_usb_status_update_ex(int status)
{
    g_usb_notifier_h.usb_host_status = status;
}
/*****************************************************************
Parameters    :
Return        :    无
Description   :  usb插拔工作队列
*****************************************************************/
static USB_VOID usb_notify_handler(struct work_struct* work)
{
    USB_INT cur_status = g_usb_notifier_h.usb_old_status;
    USB_INT status = 0;
    USB_ULONG flags = 0 ;
    DBG_I(MBB_HOTPLUG, "old_status %d,status%d\n",
          g_usb_notifier_h.usb_old_status, g_usb_notifier_h.usb_status);
recheck:
    status =  g_usb_notifier_h.usb_status;
    if (g_usb_notifier_h.usb_old_status == status)
    {
        g_usb_notifier_h.stat_usb_no_need_notify++;
    }
    DBG_I(MBB_HOTPLUG, "task of cur_status %d status:%d:start.\n",   cur_status, status);

    if (status != cur_status)
    {
        local_irq_save(flags);
        cur_status = status;
        g_usb_notifier_h.usb_old_status = status;
        local_irq_restore( flags);
        switch (status)
        {
            case MBB_USB_DEVICE_INSERT:

                /*判断是否无线充电功能*/
                if (NULL  != g_usb_hotplug_ctx.usb_check_wireless_chg_cb)
                {
                    if (1 == g_usb_hotplug_ctx.usb_check_wireless_chg_cb())
                    {
                        DBG_I(MBB_HOTPLUG, "wireless charger detected\n");
                        break;
                    }
                }
                if (MBB_USB_HOTPLUG_INSERT
                    == g_usb_notifier_h.usb_hotplub_state)
                {
                    DBG_I(MBB_HOTPLUG, "task of %d cancel.\n",   cur_status);
                    return ;
                }
                g_usb_notifier_h.usb_hotplub_state = MBB_USB_HOTPLUG_INSERT;
#ifdef USB_TYPEC_CTRL
                /*增加usb typec支持*/
                usbc_i2c_hotplug(status);
#endif
                mbb_usb_insert_process();


                break;
            case MBB_USB_PERIP_INSERT:
                local_irq_save(flags);
                mbb_usb_perip_insert_process();
                local_irq_restore(flags);
                break;
            case MBB_USB_ENUM_DONE:
                mbb_usb_enum_done_process();
                break;
            case MBB_USB_DEVICE_REMOVE:
                /*判断是否无线充电功能*/
                if (NULL  != g_usb_hotplug_ctx.usb_wireless_chg_remove_cb)
                {
                    if (1 == g_usb_hotplug_ctx.usb_wireless_chg_remove_cb())
                    {
                        DBG_I(MBB_HOTPLUG, "wireless charger remove\n");
                        break;
                    }
                }

                if (MBB_USB_HOTPLUG_REMOVE
                    == g_usb_notifier_h.usb_hotplub_state)
                {
                    DBG_I(MBB_HOTPLUG, "task of %d cancel.\n",   cur_status);
                    return ;
                }
                g_usb_notifier_h.usb_hotplub_state = MBB_USB_HOTPLUG_REMOVE;
#ifdef USB_TYPEC_CTRL
                /*增加usb typec支持*/
                usbc_i2c_hotplug(status);
#endif
                mbb_usb_remove_device_process();


                break;
            case MBB_USB_PERIP_REMOVE:
                local_irq_save(flags);
                mbb_usb_remove_perip_process();
                local_irq_restore(flags);
                break;
            case MBB_USB_DEVICE_DISABLE:
                mbb_usb_disable_device_process();
                break;
            default:
                DBG_I(MBB_HOTPLUG, "invalid status:%d\n",
                      cur_status);
                return;
        }
        DBG_I(MBB_HOTPLUG, "recheck\n");
        goto recheck;
    }

    DBG_I(MBB_HOTPLUG, "task of %d end.\n",   cur_status);

    g_usb_notifier_h.usb_old_status = cur_status;
    return;
}

static void usb_notify_handler_ex(struct work_struct* work)
{
    int cur_status = g_usb_notifier_h.usb_host_status;
    DBG_I(MBB_HOTPLUG, "old_status %d,cur_status%d\n"
          , g_usb_notifier_h.usb_old_status, cur_status);
    if (g_usb_notifier_h.usb_host_old_status == cur_status)
    {
        g_usb_notifier_h.stat_usb_no_need_notify++;
        return;
    }
    DBG_I(MBB_HOTPLUG, "task of %d start.\n", cur_status);
    switch (cur_status)
    {
        case MBB_USB_PERIP_REMOVE:
            mbb_usb_remove_perip_process();
            break;
        case MBB_USB_PERIP_INSERT:
            mbb_usb_perip_insert_process();
            break;
        default:
            DBG_T(MBB_HOTPLUG, "invalid status:%d\n", cur_status);
            return;
    }
    DBG_I(MBB_HOTPLUG, "task of %d end.\n", cur_status);
    g_usb_notifier_h.usb_host_old_status = cur_status;
    return;
}

/*****************************************************************
Parameters    :
Return        :    无
Description   :  USB hotplug解wakelock锁
*****************************************************************/
USB_VOID wake_unlock_handler( USB_ULONG data )
{
    USB_ULONG flags = 0;

    local_irq_save(flags);
    if ( MBB_USB_DEVICE_REMOVE == g_usb_notifier_h.usb_status)
    {
        DBG_I(MBB_HOTPLUG, "usb_status : %d.\n", g_usb_notifier_h.usb_status);
        usb_wake_unlock();
    }
    local_irq_restore(flags);
}

USB_VOID usb_hotplug_registe_cb(USB_VOID)
{
    usb_hotplug_status_handle_t* usb_status = usb_get_usb_status_ctx();
    usb_hotplug_api_t*   usb_hotplug_api = usb_get_usb_hotplug_api();
    DBG_I(MBB_HOTPLUG, "\n");

    /*register cb */
    usb_status->status_change_cb = mbb_usb_status_change;
    usb_status->perip_status_change_cb = mbb_usb_status_change_ex;
    usb_status->usb_set_charger_type_cb = mbb_usb_set_charger_type;
    usb_hotplug_api->get_hotplug_status_cb     = usb_get_hotplug_status;
    usb_hotplug_api->get_hotplug_old_status_cb = usb_get_hotplug_old_status;
    usb_hotplug_api->get_insert_timestamp_cb    = usb_get_insert_timestamp;
    usb_hotplug_api->get_remove_timestamp_cb   = usb_get_remove_timestamp;
    usb_hotplug_api->usb_get_charger_type_cb    = mbb_usb_get_charger_type;
}

USB_VOID usb_hotplug_unregiste_cb(USB_VOID)
{
    usb_hotplug_api_t*   usb_hotplug_api = usb_get_usb_hotplug_api();
    usb_hotplug_status_handle_t* usb_status = usb_get_usb_status_ctx();

    usb_status->status_change_cb = NULL;

    memset(usb_hotplug_api, 0, sizeof(usb_hotplug_api_t));
}

/*****************************************************************
Parameters    :
Return        :    无
Description   :  USB hotplug模块初始化
*****************************************************************/

USB_VOID usb_hotplug_init(USB_VOID)
{
    usb_hotplug_status_handle_t* usb_status = usb_get_usb_status_ctx();

    mutex_init(&g_usb_notifier_h.mutex);

    g_usb_notifier_h.usb_notify_wq = create_singlethread_workqueue("usb_hotplug");
    if (NULL == g_usb_notifier_h.usb_notify_wq)
    {
        DBG_E(MBB_HOTPLUG, "usb_hotplug create_singlethread_workqueue fail\n");
        return;
    }
    INIT_DELAYED_WORK(&g_usb_notifier_h.usb_notify_wk, (void*)usb_notify_handler);
    INIT_DELAYED_WORK(&g_usb_notifier_h.usb_notify_host_wk, (void*)usb_notify_handler_ex);

    setup_timer(&g_wake_unlock_timer , wake_unlock_handler, (USB_ULONG)0);

    usb_hotplug_registe_cb();

    mbb_usb_set_charger_type(usb_status->cur_charger_type);
    mbb_usb_status_change(usb_status->cur_status);
    mbb_usb_status_change_ex(usb_status->cur_status);
}

USB_VOID  usb_hotplug_exit(USB_VOID)
{
    usb_hotplug_unregiste_cb();
    usb_stop_work(&(g_usb_notifier_h.usb_notify_wk));

    usb_stop_work(&(g_usb_notifier_h.usb_notify_host_wk));

    del_timer_sync(&g_wake_unlock_timer);

    //bsp_usb_status_change(USB_BALONG_DEVICE_INSERT);
    //usb_exit_status();
    usb_wake_unlock();/*remove完成需要unlock*/
}

