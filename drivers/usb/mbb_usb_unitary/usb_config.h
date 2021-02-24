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
 


#ifndef __USB_CONFIG_H__
#define __USB_CONFIG_H__

#include <product_config.h>
/*===================ƽ̨����� BEGIN==============*/

//#define MBB_USB_UNITARY_Q     /*�����ͨƽ̨�ĺ�*/
#define MBB_USB_UNITARY_B     /*�������ƽ̨�ĺ�*/
/*==================ƽ̨����� END==============*/

/*===================���ܺ�BEGIN==============*/

// #define USB_DEBUG

#if (FEATURE_ON == MBB_USB_MULTICONFIG)
#define USB_MULTICONFIG
#endif

#if (FEATURE_ON == MBB_USB_SECURITY)
#define USB_SECURITY
#endif

#if (FEATURE_ON == MBB_USB_SD)
#define USB_SD
#endif

#if (FEATURE_ON == MBB_USB_PORT_AUTH)
#define USB_PORT_AUTH
#endif

#if (FEATURE_ON == MBB_USB_CPE)
#define USB_CPE
#endif

#if (FEATURE_ON == MBB_USB_E5)
#define USB_E5
#endif

#if (FEATURE_ON == MBB_USB_HILINK)
#define USB_HILINK
#endif

#if (FEATURE_ON == MBB_USB_STICK)
#define USB_STICK
#endif

#if (FEATURE_ON == MBB_USB_WINGLE)
#define USB_WINGLE
#endif

#if (FEATURE_ON == MBB_USB_SOLUTION)
#define USB_SOLUTION
#endif

#ifdef USB_SOLUTION
#define USB_PORT_NAME_UNITARY
#endif

#if (FEATURE_ON == MBB_USB_CAPABILITY_THREE)
#define USB_SUPER_SPEED
#endif

#if (FEATURE_ON == MBB_CHARGE)
#define USB_CHARGE
#if (FEATURE_ON == MBB_CHG_EXTCHG)
#define USB_CHARGE_EXT
#define USB_OTG_DEV_DETECT
#endif
#endif

#if (FEATURE_ON == MBB_FAST_ON_OFF)
#define USB_FAST_ON_OFF
#endif

#ifdef CONFIG_BALONG_RNDIS
#define USB_RNDIS
#endif

#if (FEATURE_ON == MBB_DLOAD)
#define USB_DLOAD
#endif/*USB_DLOAD*/

#if (FEATURE_ON == MBB_BUILD_DEBUG)
/*debug �汾�д�USB adb����*/
#define USB_ADB
/*debug �汾�д�USB ͬ������SN����*/
#define USB_UNIQUE_SERIAL_NUM
/*��debug�汾��usb_dfx*/
#define USB_DFX
/*debug �汾�رռ�Ȩ���ܣ����ж˿ڶ�����ע���pnp*/
#else
/*Ĭ�ϴ�USB�˿ڼ�Ȩ���ܣ�ֻ�а�ȫ�Ķ˿ڲ���ͨ�������л�����*/
#define USB_PORT_AUTHENTICATION 
#endif/*USB_ADB*/

#if (FEATURE_ON == MBB_USB_TYPEC)
#ifdef CONFIG_I2C
#define USB_TYPEC_CTRL
#endif
#endif
/* ˫оƬCPE��Ʒ,��ʹ�õ��Ķ����ڼ��മ��ģʽ */
/* #define USB_DUAL_CORE_CPE */      

/* ����������
#define MBB_CHG_EXTCHG
#define CONFIG_USB_OTG
#define CONFIG_GADGET_SUPPORT_REMOTE_WAKEUP
#define CONFIG_NCM_MBIM_SUPPORT
#define MBB_CHG_WIRELESS
#define MBB_DLOAD
#define NCM_ENCAP_CMD_CID_ENABLED
#define CONFIG_USB_GADGET_SUPERSPEED
#define GNET_SINGLE_INTERFACE
#define GNET_CLASS_TYPE_HUAWEI
#define CONFIG_BUS_PCI
#define CONFIG_GNET_PREALLOC_RX_MEM
#define CONFIG_GNET_PREALLOC_TX_MEM
*/
/*----------------------------------------------*/
/*����ƽ̨�����ڴ����                         */
/*�����ڴ���Ʋ�����ʱ�رմ˺�   */
/*-----------------------------------------------*/
#define USB_SMEM

/*----------------------------------------------*/
/*����ƽ̨��Ϣ����*/
/*��Ϣ���Ʋ�����ʱ�رմ˺�   */
/*-----------------------------------------------*/
#define USB_EVENT_NOTIFY

/*----------------------------------------------*/
/*����ƽ̨mlog����*/
/*mlog���Ʋ�����ʱ�رմ˺�   */
/*-----------------------------------------------*/
#if (FEATURE_ON == MBB_MLOG)
#define USB_MLOG
#endif/*MBB_MLOG*/
/*----------------------------------------------*/
/*����ƽ̨NV����*/
/*NV���Ʋ�����ʱ�رմ˺�   */
/*-----------------------------------------------*/
#define USB_NV

/*===================���ܺ� END==============*/
#endif
