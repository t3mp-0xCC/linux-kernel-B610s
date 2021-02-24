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
#ifndef _wlan_drv_if_h_
#define _wlan_drv_if_h_


#define  BRCM_WL_TOOL  "/system/bin/wifi_brcm/exe/wl"

typedef enum 
{
    WIFI_CHIP_NAME_43362    = 43362, /* BCM 43362 */
    WIFI_CHIP_NAME_43239    = 43239, /* BCM 43239 */
    WIFI_CHIP_NAME_43241    = 43241, /* BCM 43241 */
    WIFI_CHIP_NAME_43236    = 43236, /* BCM 43236 */
    WIFI_CHIP_NAME_43217    = 43217, /* BCM 43217 */
    WIFI_CHIP_NAME_4354     = 4354,  /* BCM 4354  */  
    WIFI_CHIP_NAME_8189     = 8189,  /* RTL 8189  */
    WIFI_CHIP_NAME_8192     = 8192,  /* RTL 8192  */ 
}WIFI_CHIP_NAME;


/* WiFi ģ���Լ���netlink�¼��ţ���Ҫ��ϵͳ�ĳ�ͻ */
#define NETLINK_USER_WIFI_EVENT   31



/* ��v7r1 ����һ�£�����Ӧ�ò㴦�� */
#define  WIFI_AP_SHUTDOWN_TIMEOUT_SET_STR          "9 timeout:%u"           /* wifi �Զ��ر�ʱ�� */
#define  WIFI_SHUTDOWN_SET_STR                     "10 shutdown %u:%s"      /* wifi оƬ�µ� */
#define  WIFI_STARTUP_SET_STR                      "11 powerup %u:%s"       /* wifi оƬ�ϵ� */
#define  WIFI_STARTUP_VSDB_SET_STR                 "12 powerup %u:%s"
#define  WIFI_TIMER_SET_CMD_STR                    "13 timeout:%u"
#define  WIFI_SET_KERNEL_FW_TYPE                   "14 fw_type:%u"          /* оƬ����ģʽ */
#define  WIFI_MONITOR_PID_SET_STR                  "15 msg_pid:%u"          /* msg_monitor pid */
#define  WIFI_LOG_LEVEL_SET_STR                    "16 setlog:%d"           /* wifi log ��ӡ���� */


#define  WIFI_AP_SHUTDOWN_TIMEOUT_SET       9
#define  WIFI_SHUTDOWN_SET                  10
#define  WIFI_STARTUP_SET                   11
#define  WIFI_STARTUP_VSDB_SET              12
#define  WIFI_STA_SCAN_TIMEOUT_SET          13
#define  WIFI_KERNEL_FW_TYPE_SET            14
#define  WIFI_MONITOR_PID_SET               15
#define  WIFI_LOG_LEVEL_SET                 16


#endif


