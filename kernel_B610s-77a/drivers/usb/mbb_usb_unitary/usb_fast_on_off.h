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


#ifndef _USB_FAST_ON_OFF_H
#define _USB_FAST_ON_OFF_H
#include "usb_platform_comm.h"
#include "usb_config.h"


USB_INT usb_fast_on_off_stat(USB_VOID);
USB_VOID usb_fast_on_off_init(USB_VOID);
USB_VOID usb_fast_on_off_exit(USB_VOID);

#endif
