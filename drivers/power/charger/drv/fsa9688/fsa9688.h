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
/**********************问题单修改记录******************************************
日    期              修改人         问题单号           修改内容

******************************************************************************/

#ifndef __FSA9688_H__
#define __FSA9688_H__

 /*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <product_config.h>
#include "../../chg_config.h"
#define SWITCH_OK    (0)
#define SWITCH_ERR    (-1)
#define DEALY_50MS    (50)
/******************************************************************************
* Register addresses
******************************************************************************/
#define FSA9688_REG_MIN    (0x01)
#define FSA9688_REG_MAX    (0x5C)
#define FSA9688_SLAVE_REG_MIN    (0x00)
#define FSA9688_SLAVE_REG_MAX    (0x32)

#define FSA9688_REG_DEVICE_ID    (0x01)
/*Memory Location: 01, Reset State: 0001 0000*/
#define FSA9688_VERSION_ID_BITPOS     (0x3)
#define FSA9688_VERSION_ID   (0x2 << FSA9688_VERSION_ID_BITPOS)
#define FSA9688_VERSION_ID_MASK   (0x1F << FSA9688_VERSION_ID_BITPOS)

#define FSA9688_VENDOR_ID_BITPOS     (0x0)
#define FSA9688_VENDOR_ID   (0x0 << FSA9688_VENDOR_ID_BITPOS)
#define FSA9688_VENDOR_ID_MASK   (0x7 << FSA9688_VENDOR_ID_BITPOS)


#define FSA9688_REG_CONTROL1    (0x02)
/*Memory Location: 02, Reset State: XXX1 1111*/
#define FSA9688_CONTROL1_RESERVE_BITPOS    (0x5)
#define FSA9688_CONTROL1_RESERVE    (0x0 << FSA9688_CONTROL1_RESERVE_BITPOS)
#define FSA9688_CONTROL1_RESERVE_MASK    (0x7 << FSA9688_CONTROL1_RESERVE_BITPOS)

#define FSA9688_SWITCH_OPEN_BITPOS    (0x4)
#define FSA9688_SWITCH_OPEN_ALL    (0x0 << FSA9688_SWITCH_OPEN_BITPOS)
#define FSA9688_SWITCH_OPEN_AUTO    (0x1 << FSA9688_SWITCH_OPEN_BITPOS)
#define FSA9688_SWITCH_OPEN_MASK    (0x1 << FSA9688_SWITCH_OPEN_BITPOS)

#define FSA9688_RAW_DATA_BITPOS    (0x3)
#define FSA9688_REPORT_STATUS_CHANGES    (0x0 << FSA9688_RAW_DATA_BITPOS)
#define FSA9688_NOT_REPORT_STATUS_CHANGES    (0x1 << FSA9688_RAW_DATA_BITPOS)
#define FSA9688_RAW_DATA_MASK    (0x1 << FSA9688_RAW_DATA_BITPOS)

#define FSA9688_MANUAL_SW_BITPOS    (0x2)
#define FSA9688_MANUAL_SW    (0x0 << FSA9688_MANUAL_SW_BITPOS)
#define FSA9688_AUTO_SW    (0x1 << FSA9688_MANUAL_SW_BITPOS)
#define FSA9688_MANUAL_SW_MASK    (0x1 << FSA9688_MANUAL_SW_BITPOS)

#define FSA9688_WAIT_BITPOS    (0x1)
#define FSA9688_OPEN_NO_WAIT    (0x0 << FSA9688_WAIT_BITPOS)
#define FSA9688_OPEN_WAIT    (0x1 << FSA9688_WAIT_BITPOS)
#define FSA9688_WAIT_MASK    (0x1 << FSA9688_WAIT_BITPOS)

#define FSA9688_INI_MASK_BITPOS    (0x0)
#define FSA9688_UNMASK_INT    (0x0 << FSA9688_INI_MASK_BITPOS)
#define FSA9688_MASK_INT    (0x1 << FSA9688_INI_MASK_BITPOS)
#define FSA9688_INT_MASK    (0x1 << FSA9688_INI_MASK_BITPOS)


#define FSA9688_REG_INTERRUPT    (0x03)
/*Memory Location: 02, Reset State: 0000 xx00*/
#define FSA9688_ADC_INI_BITPOS    (0x7)
#define FSA9688_ADC_INI    (0x1 << FSA9688_ADC_INI_BITPOS)
#define FSA9688_NO_ADC_INI    (0x0 << FSA9688_ADC_INI_BITPOS)
#define FSA9688_ADC_INI_MASK    (0x1 << FSA9688_ADC_INI_BITPOS)

#define FSA9688_RESERVED_ATTACH_INI_BITPOS    (0x6)
#define FSA9688_RESERVED_ATTACH_INI    (0x1 << FSA9688_RESERVED_ATTACH_INI_BITPOS)
#define FSA9688_NO_RESERVED_ATTACH_INI    (0x0 << FSA9688_RESERVED_ATTACH_INI_BITPOS)
#define FSA9688_RESERVED_ATTACH_INI_MASK    (0x1 << FSA9688_RESERVED_ATTACH_INI_BITPOS)

#define FSA9688_VBUS_INI_BITPOS    (0x5)
#define FSA9688_VBUS_INI    (0x1 << FSA9688_VBUS_INI_BITPOS)
#define FSA9688_NO_VBUS_INI    (0x0 << FSA9688_VBUS_INI_BITPOS)
#define FSA9688_VBUS_INI_MASK    (0x1 << FSA9688_VBUS_INI_BITPOS)

#define FSA9688_DEVICE_INI_BITPOS    (0x4)
#define FSA9688_DEVICE_INI    (0x1 << FSA9688_DEVICE_INI_BITPOS)
#define FSA9688_NO_DEVICE_INI    (0x0 << FSA9688_DEVICE_INI_BITPOS)
#define FSA9688_DEVICE_INI_MASK    (0x1 << FSA9688_DEVICE_INI_BITPOS)

#define FSA9688_RESERVE_INI_BITPOS    (0x2)
#define FSA9688_RESERVE_INI    (0x0 << FSA9688_RESERVE_INI_BITPOS)
#define FSA9688_RESERVE_INI_MASK    (0x3 << FSA9688_RESERVE_INI_BITPOS)

#define FSA9688_DETACH_INI_BITPOS    (0x1)
#define FSA9688_DETACH_INI    (0x1 << FSA9688_DETACH_INI_BITPOS)
#define FSA9688_NO_DETACH_INI    (0x0 << FSA9688_DETACH_INI_BITPOS)
#define FSA9688_DETACH_INI_MASK    (0x1 << FSA9688_DETACH_INI_BITPOS)

#define FSA9688_ATTACH_INI_BITPOS    (0x0)
#define FSA9688_ATTACH_INI    (0x1 << FSA9688_ATTACH_INI_BITPOS)
#define FSA9688_NO_ATTACH_INI    (0x0 << FSA9688_ATTACH_INI_BITPOS)
#define FSA9688_ATTACH_INI_MASK    (0x1 << FSA9688_ATTACH_INI_BITPOS)

#define FSA9688_REG_INTERRUPT_MASK    (0x04)
/*Memory Location: 04, Reset State: 0010 xx00*/
#define FSA9688_MASK_ADC_INI_BITPOS    (0x7)
#define FSA9688_MASK_ADC_INI    (0x1 << FSA9688_MASK_ADC_INI_BITPOS)
#define FSA9688_UNMASK_ADC_INI    (0x0 << FSA9688_MASK_ADC_INI_BITPOS)
#define FSA9688_MASK_ADC_INI_MASK    (0x1 << FSA9688_MASK_ADC_INI_BITPOS)

#define FSA9688_MASK_RESERVED_ATTACH_INI_BITPOS    (0x6)
#define FSA9688_MASK_RESERVED_ATTACH_INI    (0x1 << FSA9688_MASK_RESERVED_ATTACH_INI_BITPOS)
#define FSA9688_UNMASK_RESERVED_ATTACH_INI    (0x0 << FSA9688_MASK_RESERVED_ATTACH_INI_BITPOS)
#define FSA9688_MASK_RESERVED_ATTACH_INI_MASK    (0x1 << FSA9688_MASK_RESERVED_ATTACH_INI_BITPOS)

#define FSA9688_MASK_VBUS_INI_BITPOS    (0x5)
#define FSA9688_MASK_VBUS_INI    (0x1 << FSA9688_MASK_VBUS_INI_BITPOS)
#define FSA9688_UNMASK_VBUS_INI    (0x0 << FSA9688_MASK_VBUS_INI_BITPOS)
#define FSA9688_MASK_VBUS_INI_MASK    (0x1 << FSA9688_MASK_VBUS_INI_BITPOS)

#define FSA9688_MASK_DEVICE_INI_BITPOS    (0x4)
#define FSA9688_MASK_DEVICE_INI    (0x1 << FSA9688_MASK_DEVICE_INI_BITPOS)
#define FSA9688_UNMASK_DEVICE_INI    (0x0 << FSA9688_MASK_DEVICE_INI_BITPOS)
#define FSA9688_MASK_DEVICE_INI_MASK    (0x1 << FSA9688_MASK_DEVICE_INI_BITPOS)

#define FSA9688_MASK_RESERVE_INI_BITPOS    (0x2)
#define FSA9688_MASK_RESERVE_INI    (0x0 << FSA9688_RESERVE_INI_BITPOS)
#define FSA9688_MASK_RESERVE_INI_MASK    (0x3 << FSA9688_RESERVE_INI_BITPOS)

#define FSA9688_MASK_DETACH_INI_BITPOS    (0x1)
#define FSA9688_MASK_DETACH_INI    (0x1 << FSA9688_MASK_DETACH_INI_BITPOS)
#define FSA9688_UNMASK_DETACH_INI    (0x0 << FSA9688_MASK_DETACH_INI_BITPOS)
#define FSA9688_MASK_DETACH_INI_MASK    (0x1 << FSA9688_MASK_DETACH_INI_BITPOS)

#define FSA9688_MASK_ATTACH_INI_BITPOS    (0x0)
#define FSA9688_MASK_ATTACH_INI    (0x1 << FSA9688_MASK_ATTACH_INI_BITPOS)
#define FSA9688_UNMASK_ATTACH_INI    (0x0 << FSA9688_MASK_ATTACH_INI_BITPOS)
#define FSA9688_MASK_ATTACH_INI_MASK    (0x1 << FSA9688_MASK_ATTACH_INI_BITPOS)

#define FSA9688_REG_ADC    (0x05)
/*Memory Location: 05, Reset State: XXX1 1111*/
#define FSA9688_ADC_RESERVE_BITPOS    (0x5)
#define FSA9688_ADC_RESERVE    (0x0 << FSA9688_ADC_RESERVE_BITPOS)
#define FSA9688_ADC_RESERVE_MASK    (0x7 << FSA9688_ADC_RESERVE_BITPOS)

#define FSA9688_ADC_VALUE_BITPOS    (0x0)
#define FSA9688_ADC_VALUE_MASK    (0x1F << FSA9688_ADC_VALUE_BITPOS)

#define FSA9688_REG_TIMING_SET_1    (0x06)
/*Memory Location: 06, Reset State: 0000 0000*/
#define FSA9688_SWITCHING_WAIT_BITPOS    (0x4)
#define FSA9688_SWITCHING_WAIT_MASK    (0xF << FSA9688_SWITCHING_WAIT_BITPOS)

#define FSA9688_ADC_DETECTION_BITPOS    (0x0)
#define FSA9688_ADC_DETECTION_MASK    (0xF << FSA9688_ADC_DETECTION_BITPOS)

#define FSA9688_REG_DETACH_CONTROL    (0x07)
/*Memory Location: 07, Reset State: XXXX XXX0*/
#define FSA9688_DETACH_RESERVE_BITPOS    (0x1)
#define FSA9688_DETACH_RESERVE    (0x0 << FSA9688_DETACH_RESERVE_BITPOS)
#define FSA9688_DETACH_RESERVE_MASK    (0xF << FSA9688_DETACH_RESERVE_BITPOS)

#define FSA9688_DETACH_BITPOS    (0x0)
#define FSA9688_DETACHED    (0x1 << FSA9688_DETACH_BITPOS)
#define FSA9688_NO_DETACHED    (0x0 << FSA9688_DETACH_BITPOS)
#define FSA9688_DETACH_MASK    (0xF << FSA9688_DETACH_BITPOS)

#define FSA9688_REG_DEVICE_TYPE_1    (0x08)
/*Memory Location: 08, Reset State: 0000 0000*/
#define FSA9688_USB_OTG_BITPOS    (0x7)
#define FSA9688_USB_OTG_DETECTED    (0x1 << FSA9688_USB_OTG_BITPOS)
#define FSA9688_NO_USB_OTG_DETECTED    (0x0 << FSA9688_USB_OTG_BITPOS)
#define FSA9688_USB_OTG_MASK    (0x1 << FSA9688_USB_OTG_BITPOS)

#define FSA9688_DCP_BITPOS    (0x6)
#define FSA9688_DCP_DETECTED    (0x1 << FSA9688_DCP_BITPOS)
#define FSA9688_NO_DCP_DETECTED    (0x0 << FSA9688_DCP_BITPOS)
#define FSA9688_DCP_MASK    (0x1 << FSA9688_DCP_BITPOS)

/*USB CHARGER*/
#define FSA9688_CDP_BITPOS    (0x5)
#define FSA9688_CDP_DETECTED    (0x1 << FSA9688_CDP_BITPOS)
#define FSA9688_NO_CDP_DETECTED    (0x0 << FSA9688_CDP_BITPOS)
#define FSA9688_CDP_MASK    (0x1 << FSA9688_CDP_BITPOS)

#define FSA9688_MHL_BITPOS    (0x4)
#define FSA9688_MHL_DETECTED    (0x1 << FSA9688_MHL_BITPOS)
#define FSA9688_NO_MHL_DETECTED    (0x0 << FSA9688_MHL_BITPOS)
#define FSA9688_MHL_MASK    (0x1 << FSA9688_MHL_BITPOS)

#define FSA9688_UART_BITPOS    (0x3)
#define FSA9688_UART_DETECTED    (0x1 << FSA9688_UART_BITPOS)
#define FSA9688_NO_UART_DETECTED    (0x0 << FSA9688_UART_BITPOS)
#define FSA9688_UART_MASK    (0x1 << FSA9688_UART_BITPOS)

/*USB standard downstream port*/
#define FSA9688_SDP_BITPOS    (0x2)
#define FSA9688_SDP_DETECTED    (0x1 << FSA9688_SDP_BITPOS)
#define FSA9688_NO_SDP_DETECTED    (0x0 << FSA9688_SDP_BITPOS)
#define FSA9688_SDP_MASK    (0x1 << FSA9688_SDP_BITPOS)

/*Factory mode cable RF Calibration Mode using CP detected*/
#define FSA9688_CP_BITPOS    (0x1)
#define FSA9688_CP_DETECTED    (0x1 << FSA9688_CP_BITPOS)
#define FSA9688_NO_CP_DETECTED    (0x0 << FSA9688_CP_BITPOS)
#define FSA9688_CP_MASK    (0x1 << FSA9688_CP_BITPOS)

/*Factory mode cable USB path with BOOT ON using CP detected */
#define FSA9688_BOOT_ON_CP_BITPOS    (0x0)
#define FSA9688_BOOT_ON_CP_DETECTED    (0x1 << FSA9688_BOOT_ON_CP_BITPOS)
#define FSA9688_NO_BOOT_ON_CP_DETECTED    (0x0 << FSA9688_BOOT_ON_CP_BITPOS)
#define FSA9688_BOOT_ON_CP_MASK    (0x1 << FSA9688_BOOT_ON_CP_BITPOS)

#define FSA9688_REG_DEVICE_TYPE_2    (0x09)
/*Memory Location: 09, Reset State: 0000 0000*/

#define FSA9688_REG_DEVICE_TYPE_3    (0x0A)
/*Memory Location: 0A, Reset State: 0000 0000*/

#define FSA9688_REG_MANUAL_SW_1    (0x0B)
/*Memory Location: 0B, Reset State: xxxx x011*/

#define FSA9688_REG_MANUAL_SW_2    (0x0C)
/*Memory Location: 0C, Reset State: 0000 0000*/
#define FSA9688_MANUAL_SW_2_RESERVE_BITPOS    (0x3)
#define FSA9688_MANUAL_SW_2_RESERVE    (0x0 << FSA9688_MANUAL_SW_2_RESERVE_BITPOS)
#define FSA9688_MANUAL_SW_2_RESERVE_MASK    (0x1F << FSA9688_MANUAL_SW_2_RESERVE_BITPOS)

#define FSA9688_CHG_DET_BITPOS    (0x2)
#define FSA9688_CHG_DET_EN    (0x1 << FSA9688_CHG_DET_BITPOS)
#define FSA9688_CHG_DET_DIS    (0x0 << FSA9688_CHG_DET_BITPOS)
#define FSA9688_CHG_DET_MASK    (0x1F << FSA9688_CHG_DET_BITPOS)

#define FSA9688_BOOT_SW_BITPOS    (0x1)
#define FSA9688_BOOT_SW_EN    (0x1 << FSA9688_BOOT_SW_BITPOS)
#define FSA9688_BOOT_SW_DIS    (0x0 << FSA9688_BOOT_SW_BITPOS)
#define FSA9688_BOOT_SW_MASK    (0x1F << FSA9688_BOOT_SW_BITPOS)

#define FSA9688_JIG_ON_BITPOS    (0x0)
#define FSA9688_JIG_ON_EN    (0x1 << FSA9688_JIG_ON_BITPOS)
#define FSA9688_JIG_ON_DIS    (0x0 << FSA9688_JIG_ON_BITPOS)
#define FSA9688_JIG_ON_MASK    (0x1F << FSA9688_JIG_ON_BITPOS)

#define FSA9688_REG_TIMING_SET_2    (0x0D)
/*Memory Location: 0D, Reset State: XXXX X000*/
#define FSA9688_TIMING_SET_2_RESERVE_BITPOS    (0x3)
#define FSA9688_TIMING_SET_2_RESERVE    (0x1 << FSA9688_TIMING_SET_2_RESERVE_BITPOS)
#define FSA9688_TIMING_SET_2_RESERVE_MASK    (0x1F << FSA9688_TIMING_SET_2_RESERVE_BITPOS)

#define FSA9688_PHONEOFF_WAIT_TIME_BITPOS    (0x3)
#define FSA9688_PHONEOFF_WAIT_TIME_MASK    (0x7 << FSA9688_PHONEOFF_WAIT_TIME_BITPOS)

#define FSA9688_REG_CONTROL2    (0x0E)
/*Memory Location: 0E, Reset State: 000X XX00*/
#define FSA9688_ACCP_BLOCK_EN_BITPOS    (0x7)
#define FSA9688_ACCP_BLOCK_EN    (0x1 << FSA9688_ACCP_BLOCK_EN_BITPOS)
#define FSA9688_ACCP_BLOCK_DIS    (0x0 << FSA9688_ACCP_BLOCK_EN_BITPOS)
#define FSA9688_ACCP_BLOCK_EN_MASK    (0x1 << FSA9688_ACCP_BLOCK_EN_BITPOS)

#define FSA9688_ACCP_AUTO_PROTL_EN_BITPOS    (0x6)
#define FSA9688_ACCP_AUTO_PROTL_EN    (0x1 << FSA9688_ACCP_AUTO_PROTL_EN_BITPOS)
#define FSA9688_ACCP_AUTO_PROTL_DIS    (0x0 << FSA9688_ACCP_AUTO_PROTL_EN_BITPOS)
#define FSA9688_ACCP_AUTO_PROTL_EN_MASK    (0x1 << FSA9688_ACCP_AUTO_PROTL_EN_BITPOS)

#define FSA9688_ACCP_OSC_EN_BITPOS    (0x5)
#define FSA9688_ACCP_OSC_EN    (0x1 << FSA9688_ACCP_OSC_EN_BITPOS)
#define FSA9688_ACCP_OSC_DIS    (0x0 << FSA9688_ACCP_OSC_EN_BITPOS)
#define FSA9688_ACCP_OSC_EN_MASK    (0x1 << FSA9688_ACCP_OSC_EN_BITPOS)

#define FSA9688_CONTROL_2_RESERVE_BITPOS    (0x2)
#define FSA9688_CONTROL_2_RESERVE    (0x0 << FSA9688_CONTROL_2_RESERVE_BITPOS)
#define FSA9688_CONTROL_2_RESERVE_MASK    (0x7 << FSA9688_CONTROL_2_RESERVE_BITPOS)

#define FSA9688_FTM_EN_BITPOS    (0x1)
#define FSA9688_FTM_EN    (0x1 << FSA9688_FTM_EN_BITPOS)
#define FSA9688_FTM_DIS    (0x0 << FSA9688_FTM_EN_BITPOS)
#define FSA9688_FTM_EN_MASK    (0x1 << FSA9688_FTM_EN_BITPOS)

#define FSA9688_DCD_TIMEOUT_EN_BITPOS    (0x0)
#define FSA9688_DCD_TIMEOUT_EN    (0x1 << FSA9688_DCD_TIMEOUT_EN_BITPOS)
#define FSA9688_DCD_TIMEOUT_DIS    (0x0 << FSA9688_DCD_TIMEOUT_EN_BITPOS)
#define FSA9688_DCD_TIMEOUT_EN_MASK    (0x1 << FSA9688_DCD_TIMEOUT_EN_BITPOS)

#define FSA9688_REG_DEVICE_TYPE_4    (0x0F)
/*Memory Location: 0F, Reset State: 00XX XXX0*/
#define FSA9688_ACCP_ADP_BITPOS    (0x7)
#define FSA9688_ACCP_ADP_DETECTED   (0x1 << FSA9688_ACCP_ADP_BITPOS)
#define FSA9688_ACCP_ADP_NOT_DETECTED    (0x0 << FSA9688_ACCP_ADP_BITPOS)
#define FSA9688_ACCP_ADP_MASK    (0x1 << FSA9688_ACCP_ADP_BITPOS)

#define FSA9688_NON_ACCP_ADP_BITPOS    (0x6)
#define FSA9688_NON_ACCP_ADP_DETECTED   (0x1 << FSA9688_NON_ACCP_ADP_BITPOS)
#define FSA9688_NON_ACCP_ADP_NOT_DETECTED    (0x0 << FSA9688_NON_ACCP_ADP_BITPOS)
#define FSA9688_NON_ACCP_ADP_MASK    (0x1 << FSA9688_NON_ACCP_ADP_BITPOS)

#define FSA9688_DEVICE_TYPE_4_RESERVE_BITPOS    (0x1)
#define FSA9688_DEVICE_TYPE_4_RESERVE   (0x0 << FSA9688_DEVICE_TYPE_4_RESERVE_BITPOS)
#define FSA9688_DEVICE_TYPE_4_RESERVE__MASK    (0x1F << FSA9688_DEVICE_TYPE_4_RESERVE_BITPOS)

#define FSA9688_UNKNOW_ADP_BITPOS    (0x0)
#define FSA9688_UNKNOW_ADP_DETECTED   (0x1 << FSA9688_UNKNOW_ADP_BITPOS)
#define FSA9688_UNKNOW_ADP_NOT_DETECTED    (0x0 << FSA9688_UNKNOW_ADP_BITPOS)
#define FSA9688_UNKNOW_ADP_MASK    (0x1 << FSA9688_UNKNOW_ADP_BITPOS)

#define FSA9688_REG_RESET    (0x19)
/*Memory Location: 19, Reset State: XXXX XXX0*/
#define FSA9688_RESET_RESERVE_BITPOS    (0x7)
#define FSA9688_RESET_RESERVE   (0x0 << FSA9688_RESET_RESERVE_BITPOS)
#define FSA9688_RESET_RESERVE_MASK    (0x7F << FSA9688_RESET_RESERVE_BITPOS)

#define FSA9688_RESET_BITPOS    (0x0)
#define FSA9688_RESET   (0x1 << FSA9688_RESET_BITPOS)
#define FSA9688_RESET_MASK    (0x1 << FSA9688_RESET_BITPOS)

#define FSA9688_REG_ACCP_STATUS    (0x40)
/*Memory Location: 40, Reset State: 00XX XXX0*/
#define FSA9688_ACCP_DVC_BITPOS    (0x6)
#define FSA9688_ACCP_STARTED    (0x0 << FSA9688_ACCP_DVC_BITPOS)
#define FSA9688_ACCP_INVALID_DEV_DETECTED   (0x1 << FSA9688_ACCP_DVC_BITPOS)
#define FSA9688_ACCP_DVC_RESERVE   (0x2 << FSA9688_ACCP_DVC_BITPOS)
#define FSA9688_ACCP_SLAVE_DETECTED   (0x3 << FSA9688_ACCP_DVC_BITPOS)
#define FSA9688_ACCP_DVC_MASK    (0x3 << FSA9688_ACCP_DVC_BITPOS)

#define FSA9688_ACCP_STATUS_RESERVE_BITPOS    (0x1)
#define FSA9688_ACCP_STATUS_RESERVE   (0x0 << FSA9688_ACCP_STATUS_RESERVE_BITPOS)
#define FSA9688_ACCP_STATUS_RESERVE_MASK    (0x1F << FSA9688_ACCP_STATUS_RESERVE_BITPOS)

#define FSA9688_DSTREAM_ATTACH_BITPOS    (0x0)
#define FSA9688_DSTREAM_ATTACHED    (0x1 << FSA9688_DSTREAM_ATTACH_BITPOS)
#define FSA9688_NO_DSTREAM_ATTACHED   (0x0 << FSA9688_DSTREAM_ATTACH_BITPOS)
#define FSA9688_DSTREAM_ATTANCH_MASK    (0x1 << FSA9688_DSTREAM_ATTACH_BITPOS)

#define FSA9688_ACCP_STATUS_SLAVE_GOOD    (FSA9688_ACCP_SLAVE_DETECTED | FSA9688_NO_DSTREAM_ATTACHED)


#define FSA9688_REG_ACCP_CNTL    (0x41)
/*Memory Location: 41, Reset State: XXXX 00X0*/
#define FSA9688_ACCP_CNTL_RESERVE0_BITPOS    (0x4)
#define FSA9688_ACCP_CNTL_RESERVE0    (0x0 << FSA9688_ACCP_CNTL_RESERVE0_BITPOS)
#define FSA9688_ACCP_CNTL_RESERVE0_MASK    (0x1 << FSA9688_ACCP_CNTL_RESERVE0_BITPOS)

#define FSA9688_ACCP_PROTL_STATUS_BITPOS    (0x3)
#define FSA9688_ACCP_PROTL_ENANBLED    (0x1 << FSA9688_ACCP_PROTL_STATUS_BITPOS)
#define FSA9688_ACCP_PROTL_DISANBLED    (0x0 << FSA9688_ACCP_PROTL_STATUS_BITPOS)
#define FSA9688_ACCP_PROTL_STATUS_MASK    (0x1 << FSA9688_ACCP_PROTL_STATUS_BITPOS)

#define FSA9688_MSTR_RST_BITPOS    (0x2)
#define FSA9688_MSTR_RST_EN    (0x1 << FSA9688_MSTR_RST_BITPOS)
#define FSA9688_MSTR_RST_DIS    (0x0 << FSA9688_MSTR_RST_BITPOS)
#define FSA9688_MSTR_RST_MASK    (0x1 << FSA9688_MSTR_RST_BITPOS)

#define FSA9688_ACCP_CONTROL_RESERVE1_BITPOS    (0x1)
#define FSA9688_ACCP_CONTROL_RESERVE1    (0x0 << FSA9688_ACCP_CONTROL_RESERVE1_BITPOS)
#define FSA9688_ACCP_CONTROL_RESERVE1_MASK    (0x1 << FSA9688_ACCP_CONTROL_RESERVE1_BITPOS)

#define FSA9688_SNDCMD_BITPOS    (0x0)
#define FSA9688_SNDCMD_EN    (0x1 << FSA9688_SNDCMD_BITPOS)
#define FSA9688_SNDCMD_DIS    (0x0 << FSA9688_SNDCMD_BITPOS)
#define FSA9688_SNDCMD_MASK    (0x1 << FSA9688_SNDCMD_BITPOS)

#define FSA9688_REG_ACCP_CMD    (0x44)
/*Memory Location: 44, Reset State: 0000 0000*/
#define FSA9688_ACCP_CMD    (0x0 << FSA9688_ACCP_CMD_BITPOS)
#define FSA9688_ACCP_CMD_MASK    (0xFF << FSA9688_ACCP_CMD_BITPOS)

#define FSA9688_REG_ACCP_ADDR    (0x47)
/*Memory Location: 47, Reset State: 0000 0000*/
#define FSA9688_ACCP_ADDR    (0x0 << FSA9688_ACCP_ADDR_BITPOS)
#define FSA9688_ACCP_ADDR_MASK    (0xFF << FSA9688_ACCP_ADDR_BITPOS)

#define FSA9688_REG_ACCP_DATA    (0x48)
/*Memory Location: 48, Reset State: 0000 0000*/
#define FSA9688_ACCP_DATA    (0x0 << FSA9688_ACCP_DATA_BITPOS)
#define FSA9688_ACCP_DATA_MASK    (0xFF << FSA9688_ACCP_DATA_BITPOS)

#define FSA9688_REG_ACCP_INT1    (0x59)
/*Memory Location: 59, Reset State: 00X0 0XXX*/
#define FSA9688_ACCP_CMDCPL_BITPOS    (0x7)
#define FSA9688_CMDCPL_INT    (0x1 << FSA9688_ACCP_CMDCPL_BITPOS)
#define FSA9688_NO_CMDCPL_INT    (0x0 << FSA9688_ACCP_CMDCPL_BITPOS)
#define FSA9688_ACCP_CMDCPL_MASK    (0x1 << FSA9688_ACCP_CMDCPL_BITPOS)

#define FSA9688_ACCP_ACK_BITPOS    (0x6)
#define FSA9688_ACK_INT    (0x1 << FSA9688_ACCP_ACK_BITPOS)
#define FSA9688_NO_ACK_INT    (0x0 << FSA9688_ACCP_ACK_BITPOS)
#define FSA9688_ACCP_ACK_MASK    (0x1 << FSA9688_ACCP_ACK_BITPOS)

#define FSA9688_ACCP_INT_RESERVE0_BITPOS    (0x5)
#define FSA9688_ACK_INT_RESERVE0    (0x0 << FSA9688_ACCP_INT_RESERVE0_BITPOS)
#define FSA9688_ACK_INT_RESERVE0_MASK    (0x1 << FSA9688_ACCP_INT_RESERVE0_BITPOS)

#define FSA9688_ACCP_NACK_BITPOS    (0x4)
#define FSA9688_NACK_INT    (0x1 << FSA9688_ACCP_NACK_BITPOS)
#define FSA9688_NO_NACK_INT    (0x0 << FSA9688_ACCP_NACK_BITPOS)
#define FSA9688_ACCP_NACK_MASK    (0x1 << FSA9688_ACCP_NACK_BITPOS)

#define FSA9688_ACCP_CRCPAR_BITPOS    (0x3)
#define FSA9688_CRCPAR_INT    (0x1 << FSA9688_ACCP_CRCPAR_BITPOS)
#define FSA9688_NO_CRCPAR_INT    (0x0 << FSA9688_ACCP_CRCPAR_BITPOS)
#define FSA9688_ACCP_CRCPAR_MASK    (0x1 << FSA9688_ACCP_CRCPAR_BITPOS)

#define FSA9688_ACCP_INT_RESERVE1_BITPOS    (0x0)
#define FSA9688_ACK_INT_RESERVE1    (0x0 << FSA9688_ACCP_INT_RESERVE1_BITPOS)
#define FSA9688_ACK_INT_RESERVE1_MASK    (0x7 << FSA9688_ACCP_INT_RESERVE1_BITPOS)

#define FSA9688_REG_ACCP_INT2    (0x5A)
/*Memory Location: 59, Reset State: XXX0 0X00*/
#define FSA9688_ACCP_INT2_RESERVE0_BITPOS    (0x5)
#define FSA9688_ACCP_INT2_RESERVE0    (0x0 << FSA9688_ACCP_INT2_RESERVE0_BITPOS)
#define FSA9688_ACCP_INT2_RESERVE0_MASK    (0x7 << FSA9688_ACCP_INT2_RESERVE0_BITPOS)

#define FSA9688_ACCP_CRCRX_BITPOS    (0x4)
#define FSA9688_CRCRX_INT    (0x1 << FSA9688_ACCP_CRCRX_BITPOS)
#define FSA9688_NO_CRCRX_INT    (0x0 << FSA9688_ACCP_CRCRX_BITPOS)
#define FSA9688_ACCP_CRCRX_MASK    (0x1 << FSA9688_ACCP_CRCRX_BITPOS)

#define FSA9688_ACCP_PARRX_BITPOS    (0x3)
#define FSA9688_PARRX_INT    (0x1 << FSA9688_ACCP_PARRX_BITPOS)
#define FSA9688_NO_PARRX_INT    (0x0 << FSA9688_ACCP_PARRX_BITPOS)
#define FSA9688_ACCP_PARRX_MASK    (0x1 << FSA9688_ACCP_PARRX_BITPOS)

#define FSA9688_ACCP_INT2_RESERVE1_BITPOS    (0x2)
#define FSA9688_ACCP_INT2_RESERVE1    (0x0 << FSA9688_ACCP_INT2_RESERVE1_BITPOS)
#define FSA9688_ACCP_INT2_RESERVE1_MASK    (0x7 << FSA9688_ACCP_INT2_RESERVE1_BITPOS)

#define FSA9688_ACCP_PROSTAT_BITPOS    (0x1)
#define FSA9688_PROSTAT_INT    (0x1 << FSA9688_ACCP_PROSTAT_BITPOS)
#define FSA9688_NO_PROSTAT_INT    (0x0 << FSA9688_ACCP_PROSTAT_BITPOS)
#define FSA9688_ACCP_PROSTAT_MASK    (0x1 << FSA9688_ACCP_PROSTAT_BITPOS)

#define FSA9688_ACCP_DSDVCSTAT_BITPOS    (0x0)
#define FSA9688_DSDVCSTAT_INT    (0x1 << FSA9688_ACCP_DSDVCSTAT_BITPOS)
#define FSA9688_NO_DSDVCSTAT_INT    (0x0 << FSA9688_ACCP_DSDVCSTAT_BITPOS)
#define FSA9688_ACCP_DSDVCSTAT_MASK    (0x1 << FSA9688_ACCP_DSDVCSTAT_BITPOS)

#define FSA9688_REG_ACCP_INT1_MASK    (0x5B)
#define FSA9688_REG_ACCP_INT2_MASK    (0x5C)
#define FSA9688_MASK_ACCP_INT    (0XFF)

#define SLAVE_REG_DVCTYPE    (0x00)
#define SLAVE_REG_SPEC_VER    (0x01)
#define SLAVE_REG_SCNTL    (0x02)
#define SLAVE_REG_SSTAT    (0x03)
#define SLAVE_REG_ID_OUT0    (0x04)
#define SLAVE_REG_CAPABILITIES    (0x20)
#define SLAVE_REG_DISCRETE_CAPABILITIES    (0x21)
#define SLAVE_REG_MAX_PWR    (0x22)
#define SLAVE_REG_ADAPTER_STATUS    (0x28)
#define SLAVE_REG_VOUT_STATUS    (0x29)
#define SLAVE_REG_OUTPUT_CONTROL    (0x2b)
#define SLAVE_REG_VOUT_CONFIG    (0x2c)
#define SLAVE_REG_DISCRETE_VOUT_0    (0x30)
#define SLAVE_REG_DISCRETE_VOUT_1    (0x31)
#define SLAVE_REG_DISCRETE_VOUT_2    (0x32)

/*Memory Location: 28, Reset State: 0000 0000*/
#define SLAVE_ADAPTER_OVLT_BITPOS    (0x2)
#define SLAVE_ADAPTER_OVLT    (0x1 << SLAVE_ADAPTER_OVLT_BITPOS)
#define SLAVE_ADAPTER_NOT_OVLT    (0x0 << SLAVE_ADAPTER_OVLT_BITPOS)
#define SLAVE_ADAPTER_OVLT_MASK    (0x7 << SLAVE_ADAPTER_OVLT_BITPOS)

#define SLAVE_ADAPTER_OCURRENT_BITPOS    (0x1)
#define SLAVE_ADAPTER_OCURRENT    (0x1 << SLAVE_ADAPTER_OCURRENT_BITPOS)
#define SLAVE_ADAPTER_NOT_OCURRENT    (0x0 << SLAVE_ADAPTER_OCURRENT_BITPOS)
#define SLAVE_ADAPTER_OCURRENT_MASK    (0x7 << SLAVE_ADAPTER_OCURRENT_BITPOS)

#define SLAVE_ADAPTER_OTEMP_BITPOS    (0x1)
#define SLAVE_ADAPTER_OTEMP    (0x1 << SLAVE_ADAPTER_OTEMP_BITPOS)
#define SLAVE_ADAPTER_NOT_OTEMP    (0x0 << SLAVE_ADAPTER_OTEMP_BITPOS)
#define SLAVE_ADAPTER_OTEMP_MASK    (0x7 << SLAVE_ADAPTER_OTEMP_BITPOS)

#define FSA9688_ADC_DETECTION_BITPOS    (0x0)
#define FSA9688_ADC_DETECTION_MASK    (0xF << FSA9688_ADC_DETECTION_BITPOS)
#define FSA9688_REG_INTERRUPT    (0x03)
#define FSA9688_REG_INTERRUPT_MASK    (0x04)
#define FSA9688_REG_ADC    (0x05)
#define FSA9688_REG_TIMING_SET_1    (0x06)
#define FSA9688_REG_DETACH_CONTROL    (0x07)
#define FSA9688_REG_DEVICE_TYPE_1    (0x08)
#define FSA9688_REG_DEVICE_TYPE_2    (0x09)
#define FSA9688_REG_DEVICE_TYPE_3    (0x0A)
#define FSA9688_REG_MANUAL_SW_1    (0x0B)
#define FSA9688_REG_MANUAL_SW_2    (0x0C)
#define FSA9688_REG_TIMING_SET_2    (0x0D)
#define FSA9688_REG_VBUS_STATUS    (0x1B)
#define FSA9688_REG_DCD                      0x1f
#define FCP_CMD_SBRRD      0x0c
#define FCP_CMD_SBRWR      0x0b

#define SLAVE_REG_DISCRETE_OUT_V(n)     (0x30 + (n))

/* Register FCP_SLAVE_OUTPUT_CONTROL (0x2b)*/
#define HVDCP_SLAVE_SET_VOUT   (1 << 0)

#define HVDCP_VOL_SETP         (10)
#define HVDCP_OUTPUT_VOL_5V     (5)
#define HVDCP_OUTPUT_VOL_9V     (9)
#define HVDCP_OUTPUT_VOL_12V   (12)

#define HVDCP_RETRY_MAX_TIMES     (3)  /* fcp retry max times */

/******************************************************************************
* fcp definitions  end
******************************************************************************/
enum err_oprt_irq_num
{
    ERR_REQUEST_THREADED_IRQ ,
    ERR_GPIO_DIRECTION_INPUT,
    ERR_GPIO_REQUEST,
    ERR_GPIO_TO_IRQ,
    ERR_OF_GET_NAME_GPIO,
    ERR_SWITCH_USB_DEV_REGISTER,
    ERR_NO_DEV,
};

/*函数声明*/
/******************************************************************************
  Function      fsa9688_dump_all_reg
  Description   读取fsa9688 所有寄存器
  Input         void
  Output        NA
  Return        0     :函数执行成功
                -1    :函数执行失败
  Others        N/A
******************************************************************************/
int32_t fsa9688_dump_all_reg(void);

/******************************************************************************
  Function      fsa9688_dump_all_slave_reg
  Description   读取adapter所有寄存器
  Input         void
  Output        NA
  Return        0     :函数执行成功
                -1    :函数执行失败
  Others        N/A
******************************************************************************/
int32_t fsa9688_dump_all_slave_reg(void);

/******************************************************************************
  Function      fsa9688_manual_detach
  Description   检测是否已识别到FCP adapter
  Input         void
  Output        NA
  Return        0     :fcp adapter
                -1    :not fcp adapter
  Others        N/A
******************************************************************************/
int32_t fsa9688_manual_detach(void);

/******************************************************************************
  Function      hvdcp_get_charger_type
  Description   获取充电器类型
  Input         void
  Output        NA
  Return        charger typer
  Others        N/A
******************************************************************************/
chg_chgr_type_t  fsa9688_get_charger_type(void);

/****************************************************************************
  Function:     chg_get_hvdcp_adapter_max_power
  Description:  获取HVDCP adapter最大输出功率
  Input:        pow_val:功率值指针
  Output:       pow_val:功率值
  Return:       执行结果0:成功-1:失败
***************************************************************************/
int32_t fsa9688_get_hvdcp_adapter_max_power(uint8_t *pow_val);

/****************************************************************************
  Function:     chg_get_hvdcp_adapter_status
  Description:  获取DVDCP充电器状态
  Input:        adp_status,状态值指针
  Output:       adp_status
  Return:       执行结果0:成功-1:失败
***************************************************************************/
int32_t fsa9688_get_hvdcp_adpter_status (uint16_t *adp_status);

/****************************************************************************
  Function:     fcp_set_adapter_output_vol
  Description:  set fcp adapter output vol
  Input:        NA
  Output:       NA
  Return:        0: success
                -1: fail
***************************************************************************/
extern boolean chg_set_hvdcp_adpter_vol(chg_hvdcp_type_value vol_set);

/******************************************************************************
  Function      chg_get_hvdcp_type
  Description   获取高压充电器类型
  Input         void
  Output        NA
  Return        CHG_HVDCP_5V :5V高压充电器
                CHG_HVDCP_9V :9V高压充电器
                CHG_HVDCP_12V:12V高压充电器
                CHG_HVDCP_INVALID:非高压充电器
  Others        N/A
******************************************************************************/
extern chg_hvdcp_type_value chg_get_hvdcp_type(void);
#endif /* __FSA9688_H__ */
