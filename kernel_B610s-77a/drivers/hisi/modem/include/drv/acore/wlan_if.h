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
 
 
#ifndef _wlan_if_h_
#define _wlan_if_h_

#include <linux/signal.h>
#include "wlan_utils.h"
#include <linux/interrupt.h>
#ifndef WLAN_TRACE_INFO
#define    WLAN_TRACE_INFO(fmt, ...)    printk(fmt, ##__VA_ARGS__)
#define    WLAN_TRACE_ERROR(fmt, ...)    printk(fmt, ##__VA_ARGS__)
#endif

#define PKTID_MAP_HANDLE_MEMALLOC_SIZE (131104)/*4356芯片pktmap handle预申请内存大小*/

/* 判断是否采用SDIO接口的WiFi芯片 */
#define   IS_SDIO_CHIP(chip) ((chip == bcm43362) \
                            || (chip == bcm4354)   \
                            || (chip == rtl8189)   \
                            || (chip == rtl8192))  
#define  CONFIG_WIFI_SDIO (IS_SDIO_CHIP(MBB_WIFI_CHIP1) || IS_SDIO_CHIP(MBB_WIFI_CHIP2)) 

/* signal report */
typedef enum
{
    WIFI_SIGNAL_UPDATE_STA_LIST    = SIGIO,   /* WiFi STA列表更新 */
    WIFI_SIGNAL_AUTO_SHUTDOWN      = SIGUSR2,   /* WiFi自动关闭信号 */
}WLAN_SIGNAL_EVENT_ENUM;

typedef enum _WLAN_EVENT_REPORT
{ 
   WLAN_WOW_EVENT  = 0,/*wow 通知更新sta列表*/
   WLAN_FW_ABNORMAL_EVENT = 1, /*FW 各种异常情况下，恢复的机制*/
   WLAN_BCM_WPS_OVERLAP_EVENT = 4,/*WPS 会话重叠事件区别上层 WLAN_WPS_OVERLAP_EVENT，实际意义一样，来源不同*/
   WLAN_EVENT_MAX  /* 如果事件值不小于此值，则为非法事件 */  
}WLAN_EVENT_REPORT;/*此处要保证与应用定义一致*/

/* event report */
typedef enum _WLAN_EVENT_TYPE
{
    USER_WIFI_TIMEOUT_EVENT         = 1,         /* WiFi自动关闭消息 */
    USER_WIFI_UPDATE_STA_LIST       = 2,         /* STA接入个数更新事件 */
    USER_WIFI_DATA_DEAD_EVENT       = 32,        /* 控制导致FW异常上报 */
    USER_WIFI_CTRL_DEAD_EVENT       = 33,        /* 数传导致FW异常上报 */
    USER_WIFI_NULL_EVENT = ((unsigned int)-1),   /* 空事件 */
}WLAN_EVENT_TYPE;

typedef struct _wlan_user_event
{
    WLAN_EVENT_TYPE eventId;
    unsigned int eventVal;
} WLAN_USER_EVENT;

 /* WiFi芯片状态检测结构体,DHD中还有相同结构体的定义*/
typedef struct
{
    unsigned int rxerror;
    unsigned int txerror;
    unsigned int cmderror;
}WLAN_STATUS_STU;


 /*===========================================================================
 
                         函数声明部分
 
 ===========================================================================*/

/*****************************************************************************
 函数名称  : WLAN_RETURN_TYPE wlan_signal_report(WiFi_SIGNAL_EVENT_ENUM signal)
 功能描述  : 向应用层发送信号
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 当前加载WiFi类型值
*****************************************************************************/
WLAN_RETURN_TYPE wlan_signal_report(WLAN_SIGNAL_EVENT_ENUM signal);

/*****************************************************************************
 函数名称  : WLAN_RETURN_TYPE wlan_event_report(WLAN_USER_EVENT *event)
 功能描述  : 向应用层发送事件
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 当前加载WiFi类型值
*****************************************************************************/
WLAN_RETURN_TYPE wlan_event_report(WLAN_USER_EVENT *event);

/*****************************************************************************
 函数名称  : wlan_nv_read_wiinfo
 功能描述  : 获取装备AT^WiInfo的NV配置
 输入参数  : type: wiinfo的类型
 输出参数  : plen: 指向缓存长度的指针
 返 回 值  : NV缓存数据
*****************************************************************************/
void * wlan_nv_read_wiinfo(int type, unsigned *plen);
/* 巴龙V7平台支持通过NV方式配置AT^WiInfo */
#define WLAN_NV_READ_WIINFO(info_type, plen) wlan_nv_read_wiinfo((int)info_type, (unsigned *)plen)

/*****************************************************************************
 函数名称  : wlan_request_wakeup_irq
 功能描述  : WiFi唤醒BB 中断注册接口
 输入参数  : handler: 中断回调函数，devname: 设备名称，dev:中断相应参数
 输出参数  : NA
 返 回 值  : int
*****************************************************************************/
unsigned int wlan_request_wakeup_irq(irq_handler_t handler, const char *devname, void *dev);

/*****************************************************************************
 函数名称  : wlan_free_irq
 功能描述  : WiFi唤醒BB 中断释放接口
 输入参数  : irq: 中断id
 输出参数  : NA
 返 回 值  : int
*****************************************************************************/
void wlan_free_irq(unsigned int irq, void *dev);

/*****************************************************************************
 函数名称  : wlan_set_driver_lock
 功能描述  : WiFi drv 投票接口
 输入参数  : locked: 是否锁定系统，不允许休眠
 输出参数  : NA
 返 回 值  : locked
*****************************************************************************/
void wlan_set_driver_lock(int locked);
 /***********************************************************************************
 Function:          wifi_tcmd
 Description:      主要用于装备测试，对外提供wl命令的接口
 Calls:
 Input:              pCtrl: control block
 			   Data: date buffer
 			   len: data len
 Output:            NA
 Return:            NULL or node
                  
 ************************************************************************************/
int wifi_tcmd(char *tcmd_str);
#endif


