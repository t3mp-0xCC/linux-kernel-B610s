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


#ifndef __USB_HOTPLUG_H__
#define __USB_HOTPLUG_H__

#ifdef __cplusplus
extern "C" { /* allow C++ to use these headers */
#endif /* __cplusplus */

#include <linux/notifier.h>
#include <linux/workqueue.h>
#include "usb_platform_comm.h"

#define HOTPLUG_CLASS_NAME "hw_usb_hotplug"
#define HOTPLUG_DEV_NAME "usb_hotplug"
/*
 * usd to support usb event notify
 * connect pc:
 * insert(1) -> chargertype iden (2) -> enum_done(3) -> remove(0)/disable(-1) +
 *   ^___________________________________________________________|
 *
 * connect charger (must pmu support):
 * insert(1) -> remove(0) +
 *   ^____________________|
 *
 * connect peripheral(such as udisk):
 * perip_insert(4) -> perip_remove(5)
 *   ^_______________________^
 */
#define MBB_USB_DEVICE_INSERT    1
#define MBB_USB_CHARGER_IDEN     2
#define MBB_USB_ENUM_DONE        3
#define MBB_USB_PERIP_INSERT     4
#define MBB_USB_PERIP_REMOVE     5
#define MBB_USB_DEVICE_REMOVE    0
/* if the version is not support pmu detect
 * and all the device is disable, we assume that the usb is remove,
 */
#define MBB_USB_DEVICE_DISABLE   (0xF1)

#define MBB_USB_HOTPLUG_IDLE     0
#define MBB_USB_HOTPLUG_INSERT   1
#define MBB_USB_HOTPLUG_REMOVE   2

/*
 * charger type define
 */
#define USB_CHARGER_TYPE_HUAWEI         1
#define USB_CHARGER_TYPE_NOT_HUAWEI     2
#define USB_CHARGER_TYPE_INVALID        0

#define USB_CDEV_NAME_MAX               64

typedef struct _hw_usb_hotplug_driver  
{
    struct class *hotplug_class;
    struct device *hotplug_dev;
    USB_INT hotplug_enable;
    USB_INT hotplug_usb_status;
}hw_usb_hotplug_driver;


/*
 * usb adapter for charger
 */
typedef struct usb_hotplug_datamodel
{
    USB_INT charger_type;                                          /*USB充电类型*/
    USB_INT usb_status;                                              /*USB当前状态*/
    USB_INT usb_old_status;                                       /*USB上次状态*/
    USB_INT usb_host_status;
    USB_INT usb_host_old_status;
    USB_INT usb_hotplub_state;                                  /*记录USB插拔状态只有插拔动作才会更新此状态*/
    USB_UINT stat_usb_insert;                            /*防抖前插入记数*/
    USB_UINT stat_usb_insert_proc;                   /*防抖后 处理前插入记数*/
    USB_UINT stat_usb_insert_proc_end;          /*防抖后 处完成后插入记数*/
    USB_ULONG stat_usb_insert_timestamp;                   /*插入时间戳*/
    USB_UINT stat_usb_enum_done;                  /*防抖前枚举记数*/
    USB_UINT stat_usb_enum_done_proc;         /*防抖后处理前枚举记数*/
    USB_UINT stat_usb_enum_done_proc_end; /*防抖后处理后枚举记数*/
    USB_UINT stat_usb_remove;                         /*防抖前拔出记数*/
    USB_UINT stat_usb_remove_proc;               /*防抖后处理前拔出记数*/
    USB_UINT stat_usb_remove_proc_end;        /*防抖后处理后拔出记数*/
    USB_ULONG stat_usb_remove_timestamp;      /*拔出时间戳*/
    USB_UINT stat_usb_disable;
    USB_UINT stat_usb_disable_proc;
    USB_UINT stat_usb_disable_proc_end;
    USB_UINT stat_usb_no_need_notify;             /*抖动计数*/
    USB_UINT stat_usb_perip_insert;                   /*HOST防抖前插入记数*/
    USB_UINT stat_usb_perip_insert_proc;           /*HOST防抖前处理前插入记数*/
    USB_UINT stat_usb_perip_insert_proc_end;  /*HOST防抖前处理后插入记数*/
    USB_UINT stat_usb_perip_remove;                 /*HOST防抖前拔出记数*/
    USB_UINT stat_usb_perip_remove_proc;        /*HOST防抖后处理前拔出记数*/
    USB_UINT stat_usb_perip_remove_proc_end; /*HOST防抖后处理后拔出记数*/
    USB_UINT stat_usb_poweroff_fail;                   /*上电失败计数*/
    USB_UINT stat_usb_poweron_fail;                   /*下电失败计数*/
    USB_UINT delta_time;
    //unsigned stat_wait_cdev_created;
    struct workqueue_struct* usb_notify_wq;       /*USB work queue*/
    struct delayed_work usb_notify_wk;               /*USB work */
    struct delayed_work usb_notify_host_wk;      /*USB work for 作为主机时插拔U盘处理 */
    void* private;                                                    /*USB私有数据指针*/
    struct mutex mutex;
} usb_hotplug_datamodel_t;


typedef USB_INT (*usb_notifier_call_chain_cb_t)(unsigned long val, unsigned long v);
typedef  USB_INT (*usb_poweroff)(USB_VOID);
typedef  USB_INT (*usb_poweron)(USB_VOID);
typedef  USB_INT (*usb_entry)(USB_VOID);
typedef  USB_VOID (*usb_exit)(USB_VOID);
typedef USB_VOID (*usbid_proc)(USB_INT v);
typedef USB_VOID (*usb_wait_enumdone)(USB_VOID);
typedef USB_VOID (*usb_clear_enumdone)(USB_VOID);
typedef USB_INT (*usb_check_wireless_chg)(USB_VOID);
typedef USB_INT (*usb_wireless_chg_remove)(USB_VOID);
/*USB插拔需要适配的接口*/
typedef struct _usb_hotplug_hanlder
{
    usb_poweroff usb_poweroff_cb;                   /*USB下电 不支持 usb下电赋空*/
    usb_poweron usb_poweron_cb;                     /*USB上 电 不支持 usb上 电赋空*/
    usb_entry       usb_entry_cb;                   /*USB插入回调*/
    usb_exit         usb_exit_cb;                   /*USB拔出回调*/
    usbid_proc     usbid_proc_cb;                   /*USB ID是否通过GPIO控制*/
    usb_wait_enumdone  usb_wait_enumdone_cb;        /*USB 等待  是否枚举完成*/
    usb_clear_enumdone usb_clear_enumdone_cb;       /*USB 清空 是否枚举完成*/
    usb_check_wireless_chg  usb_check_wireless_chg_cb; /*判断是否无线充电*/
    usb_wireless_chg_remove  usb_wireless_chg_remove_cb; /*通知无线充电拔出*/
    struct workqueue_struct* usb_notify_wq_extern;      /*外部已存在usb的workqueue*/
    /*we can add other callbacks here*/
} usb_hotplug_hanlder_t;



typedef USB_VOID (*register_usb_notify)(struct notifier_block* nb);
typedef USB_VOID (*unregister_usb_notify)(struct notifier_block* nb);
typedef USB_VOID* (*get_notifier_handle)(USB_VOID);
typedef USB_INT (*get_vbus_status)(USB_VOID);
typedef USB_INT (*set_vbus_status)(USB_INT value);
typedef USB_INT (*get_hotplug_status)(USB_VOID);
typedef USB_INT (*get_hotplug_old_status)(USB_VOID);
typedef USB_ULONG (*get_insert_timestamp)(USB_VOID);
typedef USB_ULONG (*get_remove_timestamp)(USB_VOID);
typedef USB_INT (*usb_get_charger_type)(USB_VOID);

/*KO的api，当KO加载的时候，注册API。
 *卸载的时候删除相应的注册。
 */
typedef struct _usb_hotplug_api
{
    get_hotplug_status    get_hotplug_status_cb;
    get_hotplug_old_status   get_hotplug_old_status_cb;
    get_insert_timestamp    get_insert_timestamp_cb;
    get_remove_timestamp    get_remove_timestamp_cb;
    usb_get_charger_type    usb_get_charger_type_cb;
    
}usb_hotplug_api_t;


typedef void (*usb_status_change)(USB_INT);
typedef void (*usb_perip_status_change)(USB_INT);
typedef USB_VOID (*usb_set_charger_type)(USB_INT type);

/*KO需要外部触发的流程，如果注册了回调就记录状态，
 *触发回调,如果没有回调，就记录状态，供KO加载的时候
 *查询状态，触发相应的流程。
 */
 /*外部模块请勿使用该结构体，请定义接口进行状态传递*/
typedef struct _usb_hotplug_status_handle
{
    usb_status_change status_change_cb;
    USB_INT  cur_status;
    usb_perip_status_change perip_status_change_cb;
    usb_set_charger_type    usb_set_charger_type_cb;
    USB_INT  cur_charger_type;
} usb_hotplug_status_handle_t;

/* notify interface */
void mbb_usb_register_notify(struct notifier_block* nb);
void mbb_usb_unregister_notify(struct notifier_block* nb);

USB_VOID mbb_usb_set_charger_type(USB_INT type);

USB_VOID usb_wake_lock(USB_VOID);
USB_VOID usb_wake_unlock(USB_VOID);

USB_ULONG usb_get_insert_timestamp(USB_VOID);
USB_ULONG usb_get_remove_timestamp(USB_VOID);

USB_INT usb_get_hotplug_status(USB_VOID);

USB_INT usb_get_hotplug_old_status(USB_VOID);

USB_VOID * usb_get_notifier_handle(USB_VOID);

USB_INT usb_get_vbus_status(USB_VOID);

USB_VOID usb_adp_set_usb_status(USB_INT status);

usb_hotplug_hanlder_t *usb_get_hotplug_ctx(USB_VOID);

void usb_broadcast_event(USB_INT action);

USB_VOID usb_registe_usb_status_cb(usb_status_change cb);

usb_hotplug_api_t *usb_get_usb_hotplug_api(USB_VOID);

usb_hotplug_status_handle_t *usb_get_usb_status_ctx(USB_VOID);

USB_VOID usb_adp_set_usb_status(USB_INT status);
USB_VOID usb_adp_set_charger_type(USB_INT chg_type);

USB_VOID  usb_hotplug_init(USB_VOID);
USB_VOID  usb_hotplug_exit(USB_VOID);
USB_INT usb_adp_get_hotplug_old_status(USB_VOID);
USB_INT usb_adp_get_charger_type(USB_VOID);
USB_VOID usb_hotplug_adp_init(USB_VOID);

#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif /* __cplusplus */

#endif    /* End of __USB_HOTPLUG_H__ */

