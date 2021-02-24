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
#ifndef  _USB_EVENT_H
#define  _USB_EVENT_H
#include "usb_platform_comm.h"
#include "usb_config.h"

#ifdef USB_EVENT_NOTIFY
#ifdef MBB_USB_UNITARY_Q 
#include <linux/huawei_netlink.h>
/*USB事件*/
#define    USB_ATTACH_EVENT        GPIO_USB_ATTACH
#define    USB_REMOVE_EVENT        GPIO_USB_REMOVE
#define    USB_ENABLE_EVENT        GPIO_USB_ENABLE
#define    USB_DISABLE_EVENT       GPIO_USB_DISABLE
#define    USB_INIT_EVENT          GPIO_USB_INIT
#define    USB_HALT_EVENT          GPIO_USB_HALT
#define    USB_RESET_EVENT         GPIO_USB_RESET
#define    USB_SUSPEND_EVENT       GPIO_USB_SUSPEND
#define    USB_RESUME_EVENT        GPIO_USB_RESUME
#define    USB_CRADLE_UP_EVENT     GPIO_USB_CRADLE_UP
#define    USB_CRADLE_DOWN_EVENT   GPIO_USB_CRADLE_DOWN
#define    USB_CRADLE_ATTACH_EVENT GPIO_USB_CRADLE_ATTACH
#define    USB_CRADLE_REMOVE_EVENT GPIO_USB_CRADLE_REMOVE
#define    USB_CRADLE_PLUGIN_EVENT GPIO_USB_CRADLE_PLUGIN
#define    USB_CRADLE_UNPLUG_EVENT GPIO_USB_CRADLE_UNPLUG
#define    USB_MAX_EVENT           GPIO_USB_EVENT_MAX

/*CRADLE事件*/
#define    CRADLE_INVAILD_EVENT      CRADLE_INVAILD
#define    CRADLE_INSERT_EVENT       CRADLE_INSERT
#define    CRADLE_REMOVE_EVENT       CRADLE_REMOVE
#define    CRADLE_PPPOE_UP_EVENT     CRADLE_PPPOE_UP
#define    CRADLE_DYNAMICIP_UP_EVENT CRADLE_DYNAMICIP_UP
#define    CRADLE_STATICIP_UP_EVENT  CRADLE_STATICIP_UP
#define    CRADLE_PPPOE_DYNAMICIP_FAIL_EVENT   CRADLE_PPPOE_DYNAMICIP_FAIL
#define    CRADLE_LAN_UP_EVENT       CRADLE_LAN_UP
#define    CRADLE_MUTI_CAST_EVENT    CRADLE_MUTI_CAST
#define    CRADLE_MAX_EVENT          CRADLE_MAX

/*CHARGER事件*/
#define    CHARGER_ATTATCH_EVENT GPIO_CHARGER_ATTATCH
#define    CHARGER_REMOVE_EVENT  GPIO_CHARGER_REMOVE
#define    CHARGER_MAX_EVENT     GPIO_CHARGER_EVENT_MAX
/*产生事件的设备*/
#define    EVENT_DEVICE_USB      DEVICE_ID_USB
#define    EVENT_DEVICE_CHARGER  DEVICE_ID_CHARGER
#define    EVENT_DEVICE_CRADLE   DEVICE_ID_CRADLE

#define    EVENT_DEVICE_SD       5
#else/*MBB_USB_UNITARY_Q*/

/*balong平台*/
#include "mdrv_mbb_channel.h"
/*USB事件*/
#define    USB_ATTACH_EVENT    USB_ATTACH
#define    USB_REMOVE_EVENT    USB_REMOVE
#define    USB_ENABLE_EVENT    USB_ENABLE
#define    USB_DISABLE_EVENT   USB_DISABLE
#define    USB_INIT_EVENT      USB_INIT
#define    USB_HALT_EVENT      USB_HALT
#define    USB_RESET_EVENT     USB_RESET
#define    USB_SUSPEND_EVENT   USB_SUSPEND
#define    USB_RESUME_EVENT    USB_RESUME
#define    USB_SAMBA_PRINT_ATTACH_EVENT    USB_SAMBA_PRINT_ATTACH
#define    USB_SAMBA_PRINT_DETACH_EVENT    USB_SAMBA_PRINT_DETACH
#define    USB_CRADLE_UP_EVENT       USB_EVENT_MAX
#define    USB_CRADLE_DOWN_EVENT     USB_EVENT_MAX
#define    USB_CRADLE_ATTACH_EVENT   USB_EVENT_MAX
#define    USB_CRADLE_REMOVE_EVENT   USB_EVENT_MAX
#define    USB_CRADLE_PLUGIN_EVENT   USB_EVENT_MAX
#define    USB_CRADLE_UNPLUG_EVENT   USB_EVENT_MAX
#define    USB_MAX_EVENT             USB_EVENT_MAX

/*CRADLE事件*/
#define    CRADLE_INVAILD_EVENT      CRADLE_INVAILD
#define    CRADLE_INSERT_EVENT       CRADLE_INSERT
#define    CRADLE_REMOVE_EVENT       CRADLE_REMOVE
#define    CRADLE_PPPOE_UP_EVENT     CRADLE_PPPOE_UP
#define    CRADLE_DYNAMICIP_UP_EVENT CRADLE_DYNAMICIP_UP
#define    CRADLE_STATICIP_UP_EVENT  CRADLE_STATICIP_UP
#define    CRADLE_PPPOE_DYNAMICIP_FAIL_EVENT   CRADLE_PPPOE_DYNAMICIP_FAIL
#define    CRADLE_LAN_UP_EVENT       CRADLE_LAN_UP
#define    CRADLE_MUTI_CAST_EVENT    CRADLE_MUTI_CAST
#define    CRADLE_MAX_EVENT          CRADLE_MAX

/*SD事件*/
#define    SD_ATTATCH_EVENT       SD_ATTATCH
#define    SD_REMOVE_EVENT        SD_REMOVE
#define    U_DISK_ATTATCH_EVENT   U_DISK_ATTATCH
#define    U_DISK_REMOVE_EVENT    U_DISK_REMOVE
#define    SD_EVENT_MAX_EVENT     SD_EVENT_MAX

/*CHARGER事件*/
#define    CHARGER_ATTATCH_EVENT 0
#define    CHARGER_REMOVE_EVENT  1
#define    CHARGER_MAX_EVENT     2

/*产生事件的设备*/
#define    EVENT_DEVICE_USB     DEVICE_ID_USB
#define    EVENT_DEVICE_CHARGER DEVICE_ID_CHARGER
#if (FEATURE_ON == MBB_FEATURE_ETH)
#define    EVENT_DEVICE_CRADLE  DEVICE_ID_CRADLE
#else
#define    EVENT_DEVICE_CRADLE   16
#endif

#define    EVENT_DEVICE_SD      DEVICE_ID_SD
#endif/*MBB_USB_UNITARY_Q*/

#else/*USB_EVENT_NOTIFY*//*平台notify 不可用时，时间ID定义打桩*/
/*USB事件*/
#define    USB_ATTACH_EVENT    0
#define    USB_REMOVE_EVENT    1
#define    USB_ENABLE_EVENT    2
#define    USB_DISABLE_EVENT   3
#define    USB_INIT_EVENT      4
#define    USB_HALT_EVENT      5
#define    USB_RESET_EVENT     6
#define    USB_SUSPEND_EVENT   7
#define    USB_RESUME_EVENT    8
#define    USB_CRADLE_UP_EVENT 9
#define    USB_CRADLE_DOWN_EVENT        10
#define    USB_SAMBA_PRINT_ATTACH_EVENT 11
#define    USB_SAMBA_PRINT_DETACH_EVENT 12
#define    USB_CRADLE_ATTACH_EVENT      13
#define    USB_CRADLE_REMOVE_EVENT      14
#define    USB_CRADLE_PLUGIN_EVENT      15
#define    USB_CRADLE_UNPLUG_EVENT      16
#define    USB_MAX_EVENT                17

/*SD事件*/
#define    SD_ATTATCH_EVENT     0
#define    SD_REMOVE_EVENT      1
#define    U_DISK_ATTATCH_EVENT 2
#define    U_DISK_REMOVE_EVENT  3
#define    SD_EVENT_MAX_EVENT   4

/*CRADLE事件*/
#define    CRADLE_INVAILD_EVENT      0
#define    CRADLE_INSERT_EVENT       1
#define    CRADLE_REMOVE_EVENT       2
#define    CRADLE_PPPOE_UP_EVENT     3
#define    CRADLE_DYNAMICIP_UP_EVENT 4
#define    CRADLE_STATICIP_UP_EVENT  5
#define    CRADLE_PPPOE_DYNAMICIP_FAIL_EVENT   6
#define    CRADLE_LAN_UP_EVENT       7
#define    CRADLE_MUTI_CAST_EVENT    8
#define    CRADLE_MAX_EVENT          9

/*CHARGER事件*/
#define    CHARGER_ATTATCH_EVENT   0
#define    CHARGER_REMOVE_EVENT    1
#define    CHARGER_MAX_EVENT       2

/*产生事件的设备*/
#define    EVENT_DEVICE_USB     1
#define    EVENT_DEVICE_CHARGER 2
#define    EVENT_DEVICE_CRADLE  3
#define    EVENT_DEVICE_SD      5
#endif/*USB_EVENT_NOTIFY*/

USB_VOID usb_notify_syswatch(USB_INT deviceid, USB_INT eventcode);

#endif/*_USB_EVENT_H*/
