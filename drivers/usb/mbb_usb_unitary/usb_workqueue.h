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

#ifndef __USB_WORKQUEUE_H__
#define __USB_WORKQUEUE_H__
#include "usb_platform_comm.h"

USB_INT adp_usb_queue_delay_work(struct delayed_work* dwork, USB_ULONG delay);
USB_INT usb_workqueue_init(USB_VOID);

USB_VOID usb_wake_unlock(USB_VOID);
USB_VOID usb_wake_lock(USB_VOID);

USB_VOID usb_stop_work(struct delayed_work* wk );
USB_VOID  usb_start_work(struct delayed_work* wk, USB_INT timeout);

#endif
