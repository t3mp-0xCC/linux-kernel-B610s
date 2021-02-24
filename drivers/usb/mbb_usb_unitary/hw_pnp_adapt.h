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

#ifndef __HE_PNP_ADAPT_H__
#define __HE_PNP_ADAPT_H__

#include "usb_config.h"
#include "usb_platform_comm.h"
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include <asm/unaligned.h>

#ifdef MBB_USB_UNITARY_Q

#include <soc/qcom/smem.h>
#include <linux/qcom/diag_dload.h>


#define USB_BCDDEVICE_QC 0x0000

#define USB_VENDOR_BCDDEVICE USB_BCDDEVICE_QC

#define USB_MBIM_FUNC_NAME  "usb_mbim"
#define USB_SUBCLASS_CODE   USB_SUBCLASS_QC

#define DYNAMIC_MODULE_PID     0x1573

#define MBIM_DEBUG_MODE_PID   0X1572

/* 高通平台模块使用的interface subclass 为04 */
/* 高通平台E5 stick 使用的interface subclass 为01 */
#define USB_DYNAMIC_INTERFACE_SUBCLASS     0x01


#else/*NOT MBB_USB_UNITARY_Q */
#include <mdrv_mbb_channel.h>
#include "usb_vendor.h"
#include "bsp_sram.h"
#include "bsp_pmu.h"
#include <power_com.h>
#include <bsp_onoff.h>

#define USB_BCDDEVICE_BALONG  0x0102

#define USB_VENDOR_BCDDEVICE USB_BCDDEVICE_BALONG
/* balong 平台interface subclass 为03 */
/* balong 平台模块interface sublcass 为05 */

#define DYNAMIC_MODULE_PID     0x15C1
#define MBIM_DEBUG_MODE_PID   0X1567

#ifdef USB_SOLUTION
#define USB_SUBCLASS_CODE   USB_SUBCLASS_BALONGSOLUTION
#else
#define USB_SUBCLASS_CODE   USB_SUBCLASS_BALONG
#endif /*#ifdef USB_SOLUTION*/
#define USB_MBIM_FUNC_NAME  "ncm"
#endif/*NOT MBB_USB_UNITARY_Q*/

#endif/*__HE_PNP_ADAPT_H__*/
