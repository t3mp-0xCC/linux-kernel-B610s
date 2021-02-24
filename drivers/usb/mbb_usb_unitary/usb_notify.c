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


#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/notifier.h>
#include <linux/usb.h>
#include <linux/mutex.h>
#include "usb_notify.h"
#include "usb_debug.h"
#include "usb_config.h"

static BLOCKING_NOTIFIER_HEAD(usb_otg_event_list);
/**
 * usb_register_otg_notify - register a notifier callback whenever a usb otg 
change happens
 * @nb: pointer to the notifier block for the callback events.
 *
 * These changes are either USB ID have been connected to GND or disconnect.
 */
USB_VOID usb_register_otg_notify(struct notifier_block *nb)
{
    //printk("---------------> usb_register_otg_notify \n");
    DBG_I(MBB_OTG_CHARGER,"---------------> usb_register_otg_notify \n");
    blocking_notifier_chain_register(&usb_otg_event_list, nb);
}
EXPORT_SYMBOL_GPL(usb_register_otg_notify);

/**
 * usb_unregister_otg_notify - unregister a notifier callback
 * @nb: pointer to the notifier block for the callback events.
 *
 * usb_register_notify() must have been previously called for this function
 * to work properly.
 */
USB_VOID usb_unregister_otg_notify(struct notifier_block *nb)
{
    blocking_notifier_chain_unregister(&usb_otg_event_list, nb);
}
EXPORT_SYMBOL_GPL(usb_unregister_otg_notify);

USB_VOID usb_notify_event(USB_ULONG val, USB_PVOID v)
{
    DBG_I(MBB_EVENT,"---------------> usb_notify_event: val = %lu \n",val);
    blocking_notifier_call_chain(&usb_otg_event_list, val, v);
}
EXPORT_SYMBOL_GPL(usb_notify_event);


