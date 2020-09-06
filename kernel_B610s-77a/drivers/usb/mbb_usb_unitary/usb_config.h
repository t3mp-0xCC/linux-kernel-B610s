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
/*===================平台适配宏 BEGIN==============*/

//#define MBB_USB_UNITARY_Q     /*适配高通平台的宏*/
#define MBB_USB_UNITARY_B     /*适配巴龙平台的宏*/
/*==================平台适配宏 END==============*/

/*===================功能宏BEGIN==============*/

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
/*debug 版本中打开USB adb功能*/
#define USB_ADB
/*debug 版本中打开USB 同步单板SN功能*/
#define USB_UNIQUE_SERIAL_NUM
/*在debug版本打开usb_dfx*/
#define USB_DFX
/*debug 版本关闭鉴权功能，所有端口都可以注册给pnp*/
#else
/*默认打开USB端口鉴权功能，只有安全的端口才能通过配置切换出来*/
#define USB_PORT_AUTHENTICATION 
#endif/*USB_ADB*/

#if (FEATURE_ON == MBB_USB_TYPEC)
#ifdef CONFIG_I2C
#define USB_TYPEC_CTRL
#endif
#endif
/* 双芯片CPE产品,会使用到的多网口及多串口模式 */
/* #define USB_DUAL_CORE_CPE */      

/* 后续待处理
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
/*依赖平台共享内存机制                         */
/*共享内存机制不可用时关闭此宏   */
/*-----------------------------------------------*/
#define USB_SMEM

/*----------------------------------------------*/
/*依赖平台消息机制*/
/*消息机制不可用时关闭此宏   */
/*-----------------------------------------------*/
#define USB_EVENT_NOTIFY

/*----------------------------------------------*/
/*依赖平台mlog机制*/
/*mlog机制不可用时关闭此宏   */
/*-----------------------------------------------*/
#if (FEATURE_ON == MBB_MLOG)
#define USB_MLOG
#endif/*MBB_MLOG*/
/*----------------------------------------------*/
/*依赖平台NV机制*/
/*NV机制不可用时关闭此宏   */
/*-----------------------------------------------*/
#define USB_NV

/*===================功能宏 END==============*/
#endif
