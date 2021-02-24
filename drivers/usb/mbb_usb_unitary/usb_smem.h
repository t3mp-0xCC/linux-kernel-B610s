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


#ifndef __USB_SMEM__
#define __USB_SMEM__
#include "usb_platform_comm.h"
#include "usb_config.h"

#ifdef USB_SMEM

#ifdef MBB_USB_UNITARY_Q
#include <mach/huawei_smem.h>
#else/*MBB_USB_UNITARY_Q*/
#include "bsp_sram.h"
#endif/*MBB_USB_UNITARY_Q*/
#else/*USB_SMEM*/
#define HUAWEI_NV_MAC_SIZE      24
typedef struct
{
    unsigned char lanmac_num;        /* LAN_MAC����*/
    unsigned char wifimac_num;       /* WIFI_MAC ����*/
    unsigned char btmac_num;        /* BLUETOOTH_MAC����*/
    unsigned char usbmac_num;       /* USB_MAC����*/
    unsigned char reserve[4];          /*Ԥ���ֶ�*/
}huawei_nv_mac_num;

/*��Ϊsmem�ṹ�壬����ֻ��¼USBʹ�õ�smem���Դ�׮*/
typedef  struct
{
    USB_UINT32 smem_dload_flag;                  /* ����Ƿ��������ģʽ */
    USB_UINT32 smem_huawei_usb_port_security;    /* �����Ƿ��ϱ�Diag��ADB�˿� */
    USB_UINT32 smem_switch_pcui_flag;            /* ��ʶNV�Զ��ָ��׶Σ�����ʱUSB�˿���̬����*/
    USB_UINT32 smem_huawei_poweroff_chg;         /* ��¼�����¼���USB�������ǰ����������Լ��Ƿ��������µĹػ��¼� */
    USB_UINT32 smem_hw_support_mode;          /*��¼modem���ȡ�ĵ������ݿ�����*/
} static_smem_vendor0;

#define USB_SMEM_SIZE 1024
/*��Ϊsmem NV �ṹ�壬����ֻ��¼USBʹ�õ�smem���Դ�׮*/
typedef struct
{
    unsigned char  smem_base_mac[HUAWEI_NV_MAC_SIZE];/*���߹�һ��MAC��ַ����*/
    huawei_nv_mac_num   smem_nv_mac_num;/*���߹�һ��MAC��ַ����*/
    USB_UINT8 usb_smem_data[USB_SMEM_SIZE];
} dynamic_smem_vendor1;

#endif/*USB_SMEM*/


#ifdef MBB_USB_UNITARY_Q
/*AT getportmode use */
typedef enum
{
    SUPPORT_NONE                   = -1,
    SUPPORT_GSM                    = 0,
    SUPPORT_CDMA                   = 1,
    SUPPORT_WCDMA                  = 2,
    SUPPORT_TDSCDMA                = 3,
    SUPPORT_LTE                    = 5,
    SUPPORT_MAX            
}dsat_support_mode_e_type;

#define SYS_BM( val )              (1<<(int)(val))
#define SUPPORT_GSM_ACT             SYS_BM(SUPPORT_GSM)
#define SUPPORT_CDMA_ACT            SYS_BM(SUPPORT_CDMA)
#define SUPPORT_WCDMA_ACT           SYS_BM(SUPPORT_WCDMA)
#define SUPPORT_TDSCDMA_ACT         SYS_BM(SUPPORT_TDSCDMA)
#define SUPPORT_LTE_ACT             SYS_BM(SUPPORT_LTE)
#define LTE_MODE_STRING_LENGTH   4
#define WCDMA_MODE_STRING_LENGTH 6
#define CDMA_MODE_STRING_LENGTH  5
#define GSM_MODE_STRING_LENGTH   4
#define CHIP_MANUFACTURER_NAME  "Qualcomm"

#else
#define CHIP_MANUFACTURER_NAME  "balong"
#endif/*MBB_USB_UNITARY_Q*/


/*DLOAD MAGIC FLAG*/
#define SMEM_DLOAD_FLAG_NUM         0X444C464E  /*DLFN*/
/*NV BACKUP MAGIC FLAG*/
#define SMEM_SWITCH_PUCI_FLAG_NUM   0X444E5350
/*POWER_OFF MAGIC FLAG*/
#define POWER_OFF_CHARGER           0x59455353  /*�ػ�ħ����*/
/*USB_SECURITY MAGIC FLAG*/
#define USB_REPORT_MAGIC            0x55414442

#ifdef MBB_USB_UNITARY_Q
typedef static_smem_vendor0  USB_SMEM_COMMOM;
#else
typedef huawei_smem_info  USB_SMEM_COMMOM;
#endif

USB_SMEM_COMMOM* usb_get_smem_info(USB_VOID);
#ifdef MBB_USB_UNITARY_Q
dynamic_smem_vendor1* usb_get_smem_info_vendor1(USB_VOID);
#endif
#endif/*__USB_SMEM__ file end */

