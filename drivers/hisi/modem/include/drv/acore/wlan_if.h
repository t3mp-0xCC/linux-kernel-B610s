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

#define PKTID_MAP_HANDLE_MEMALLOC_SIZE (131104)/*4356оƬpktmap handleԤ�����ڴ��С*/

/* �ж��Ƿ����SDIO�ӿڵ�WiFiоƬ */
#define   IS_SDIO_CHIP(chip) ((chip == bcm43362) \
                            || (chip == bcm4354)   \
                            || (chip == rtl8189)   \
                            || (chip == rtl8192))  
#define  CONFIG_WIFI_SDIO (IS_SDIO_CHIP(MBB_WIFI_CHIP1) || IS_SDIO_CHIP(MBB_WIFI_CHIP2)) 

/* signal report */
typedef enum
{
    WIFI_SIGNAL_UPDATE_STA_LIST    = SIGIO,   /* WiFi STA�б���� */
    WIFI_SIGNAL_AUTO_SHUTDOWN      = SIGUSR2,   /* WiFi�Զ��ر��ź� */
}WLAN_SIGNAL_EVENT_ENUM;

typedef enum _WLAN_EVENT_REPORT
{ 
   WLAN_WOW_EVENT  = 0,/*wow ֪ͨ����sta�б�*/
   WLAN_FW_ABNORMAL_EVENT = 1, /*FW �����쳣����£��ָ��Ļ���*/
   WLAN_BCM_WPS_OVERLAP_EVENT = 4,/*WPS �Ự�ص��¼������ϲ� WLAN_WPS_OVERLAP_EVENT��ʵ������һ������Դ��ͬ*/
   WLAN_EVENT_MAX  /* ����¼�ֵ��С�ڴ�ֵ����Ϊ�Ƿ��¼� */  
}WLAN_EVENT_REPORT;/*�˴�Ҫ��֤��Ӧ�ö���һ��*/

/* event report */
typedef enum _WLAN_EVENT_TYPE
{
    USER_WIFI_TIMEOUT_EVENT         = 1,         /* WiFi�Զ��ر���Ϣ */
    USER_WIFI_UPDATE_STA_LIST       = 2,         /* STA������������¼� */
    USER_WIFI_DATA_DEAD_EVENT       = 32,        /* ���Ƶ���FW�쳣�ϱ� */
    USER_WIFI_CTRL_DEAD_EVENT       = 33,        /* ��������FW�쳣�ϱ� */
    USER_WIFI_NULL_EVENT = ((unsigned int)-1),   /* ���¼� */
}WLAN_EVENT_TYPE;

typedef struct _wlan_user_event
{
    WLAN_EVENT_TYPE eventId;
    unsigned int eventVal;
} WLAN_USER_EVENT;

 /* WiFiоƬ״̬���ṹ��,DHD�л�����ͬ�ṹ��Ķ���*/
typedef struct
{
    unsigned int rxerror;
    unsigned int txerror;
    unsigned int cmderror;
}WLAN_STATUS_STU;


 /*===========================================================================
 
                         ������������
 
 ===========================================================================*/

/*****************************************************************************
 ��������  : WLAN_RETURN_TYPE wlan_signal_report(WiFi_SIGNAL_EVENT_ENUM signal)
 ��������  : ��Ӧ�ò㷢���ź�
 �������  : NA
 �������  : NA
 �� �� ֵ  : ��ǰ����WiFi����ֵ
*****************************************************************************/
WLAN_RETURN_TYPE wlan_signal_report(WLAN_SIGNAL_EVENT_ENUM signal);

/*****************************************************************************
 ��������  : WLAN_RETURN_TYPE wlan_event_report(WLAN_USER_EVENT *event)
 ��������  : ��Ӧ�ò㷢���¼�
 �������  : NA
 �������  : NA
 �� �� ֵ  : ��ǰ����WiFi����ֵ
*****************************************************************************/
WLAN_RETURN_TYPE wlan_event_report(WLAN_USER_EVENT *event);

/*****************************************************************************
 ��������  : wlan_nv_read_wiinfo
 ��������  : ��ȡװ��AT^WiInfo��NV����
 �������  : type: wiinfo������
 �������  : plen: ָ�򻺴泤�ȵ�ָ��
 �� �� ֵ  : NV��������
*****************************************************************************/
void * wlan_nv_read_wiinfo(int type, unsigned *plen);
/* ����V7ƽ̨֧��ͨ��NV��ʽ����AT^WiInfo */
#define WLAN_NV_READ_WIINFO(info_type, plen) wlan_nv_read_wiinfo((int)info_type, (unsigned *)plen)

/*****************************************************************************
 ��������  : wlan_request_wakeup_irq
 ��������  : WiFi����BB �ж�ע��ӿ�
 �������  : handler: �жϻص�������devname: �豸���ƣ�dev:�ж���Ӧ����
 �������  : NA
 �� �� ֵ  : int
*****************************************************************************/
unsigned int wlan_request_wakeup_irq(irq_handler_t handler, const char *devname, void *dev);

/*****************************************************************************
 ��������  : wlan_free_irq
 ��������  : WiFi����BB �ж��ͷŽӿ�
 �������  : irq: �ж�id
 �������  : NA
 �� �� ֵ  : int
*****************************************************************************/
void wlan_free_irq(unsigned int irq, void *dev);

/*****************************************************************************
 ��������  : wlan_set_driver_lock
 ��������  : WiFi drv ͶƱ�ӿ�
 �������  : locked: �Ƿ�����ϵͳ������������
 �������  : NA
 �� �� ֵ  : locked
*****************************************************************************/
void wlan_set_driver_lock(int locked);
 /***********************************************************************************
 Function:          wifi_tcmd
 Description:      ��Ҫ����װ�����ԣ������ṩwl����Ľӿ�
 Calls:
 Input:              pCtrl: control block
 			   Data: date buffer
 			   len: data len
 Output:            NA
 Return:            NULL or node
                  
 ************************************************************************************/
int wifi_tcmd(char *tcmd_str);
#endif


