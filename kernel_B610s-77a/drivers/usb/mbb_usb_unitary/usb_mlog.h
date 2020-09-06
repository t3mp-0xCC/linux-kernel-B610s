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


#ifndef __USB_MLOG__
#define __USB_MLOG__

#include "usb_config.h"
#ifdef USB_MLOG
#include <linux/mlog_lib.h>

#define  USB_MLOG_NONE  mlog_lv_none
#define  USB_MLOG_FATAL  mlog_lv_fatal
#define  USB_MLOG_ERROR  mlog_lv_error
#define  USB_MLOG_WARN  mlog_lv_warn
#define  USB_MLOG_INFO  mlog_lv_info

#else
#define  mlog_print(format, ...)
#define  mlog_set_flash_statistics(format, ...)
#define  mlog_set_statis_info(format, ...)

#define  USB_MLOG_NONE  0
#define  USB_MLOG_FATAL  1
#define  USB_MLOG_ERROR  2
#define  USB_MLOG_WARN  3
#define  USB_MLOG_INFO  4
#endif/*USB_MLOG*/

#endif/*__USB_MLOG__*/
