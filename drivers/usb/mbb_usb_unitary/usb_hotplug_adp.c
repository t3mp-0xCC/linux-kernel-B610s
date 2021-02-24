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


#include <linux/fs.h>
#include <linux/platform_device.h>

#include "usb_config.h"
#include "usb_debug.h"

#include "usb_hotplug.h"

BLOCKING_NOTIFIER_HEAD(usb_notifier_list);/*lint !e34 !e110 !e156 !e651 !e43*/

static hw_usb_hotplug_driver  usb_hotplug_driver =
{
    .hotplug_enable = MBB_USB_ENABLE,
    .hotplug_usb_status = MBB_USB_DEVICE_REMOVE,
};

static ssize_t hotplug_enable_show(struct device* pdev, struct device_attribute* attr,
                                   char* buf)
{
    return snprintf(buf, PAGE_SIZE, "%1d\n", usb_hotplug_driver.hotplug_enable);
}

ssize_t hotplug_enable_store(struct device* pdev, struct device_attribute* attr,
                             const char* buff, size_t size)
{
    usb_hotplug_status_handle_t* usb_status_ctx = NULL;
    /*字符串转int*/
    if (sscanf(buff, "%d", &usb_hotplug_driver.hotplug_enable) > 0)
    {
        DBG_I(MBB_HOTPLUG, "hotplug_enable %d.\n", usb_hotplug_driver.hotplug_enable);

        if (usb_hotplug_driver.hotplug_enable)
        {
            usb_status_ctx = usb_get_usb_status_ctx();
            DBG_I(MBB_HOTPLUG, "usb_status  %d.\n", usb_status_ctx->cur_status);
            usb_adp_set_usb_status(usb_status_ctx->cur_status);
        }
		
        return size;
    }
    else
    {
        return 0;
    }
}

static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR,
    hotplug_enable_show, hotplug_enable_store);

static ssize_t hotplug_usb_status_show(struct device* pdev, struct device_attribute* attr,
                                   char* buf)
{
    usb_hotplug_status_handle_t* usb_status_ctx = NULL;
    usb_status_ctx = usb_get_usb_status_ctx();
    return snprintf(buf, PAGE_SIZE, "%1d\n", usb_status_ctx->cur_status);
}

ssize_t hotplug_usb_status_store(struct device* pdev, struct device_attribute* attr,
                             const char* buff, size_t size)
{
    usb_hotplug_status_handle_t* usb_status_ctx = NULL;
    /*字符串转int*/
    sscanf(buff, "%d", &usb_hotplug_driver.hotplug_usb_status);
    DBG_I(MBB_HOTPLUG, "usb_status %d.\n", usb_hotplug_driver.hotplug_usb_status);
    usb_status_ctx = usb_get_usb_status_ctx();
    /*如果设置的的状态和当前设备状态不同，设置生效*/
    if(NULL != usb_status_ctx)
    {
        if (usb_hotplug_driver.hotplug_usb_status != usb_status_ctx->cur_status)
        {
            DBG_I(MBB_HOTPLUG, "usb_status change to %d.\n", usb_hotplug_driver.hotplug_usb_status);
            usb_adp_set_usb_status(usb_hotplug_driver.hotplug_usb_status);
        }
    }

    return size;
}

/*在hotplug层模拟vbus中断的功能*/
static DEVICE_ATTR(usb_status, S_IRUGO | S_IWUSR,
    hotplug_usb_status_show, hotplug_usb_status_store);

static struct device_attribute* usb_hotplug_attributes[] =
{
    &dev_attr_enable,
    &dev_attr_usb_status,
    NULL
};

/*****************************************************************
Parameters    :  USB_INT action 插拔动作
Return        :    无
Description   :  通知USB插拔事件
*****************************************************************/
USB_VOID usb_broadcast_event(USB_INT action)
{
    usb_hotplug_status_handle_t* usb_status_ctx = usb_get_usb_status_ctx();
    blocking_notifier_call_chain(&usb_notifier_list, action, (USB_VOID*)&usb_status_ctx->cur_charger_type);
}
EXPORT_SYMBOL(usb_broadcast_event);

/**
 * usb_register_notify - register a notifier callback whenever a usb change happens
 * @nb: pointer to the notifier block for the callback events.
 *
 * These changes are either USB devices or busses being added or removed.
 */
USB_VOID mbb_usb_register_notify(struct notifier_block* nb)
{
    blocking_notifier_chain_register(&usb_notifier_list, nb);
}
EXPORT_SYMBOL(mbb_usb_register_notify);

/**
 * usb_unregister_notify - unregister a notifier callback
 * @nb: pointer to the notifier block for the callback events.
 *
 * usb_register_notify() must have been previously called for this function
 * to work properly.
 */
USB_VOID mbb_usb_unregister_notify(struct notifier_block* nb)
{
    blocking_notifier_chain_unregister(&usb_notifier_list, nb);
}
EXPORT_SYMBOL(mbb_usb_unregister_notify);


/*****************************************************************
Parameters    :
Return        :    无
Description   :  获取usb通知链句柄
*****************************************************************/
USB_VOID* usb_get_notifier_handle(USB_VOID)
{
    return &usb_notifier_list;
}
EXPORT_SYMBOL(usb_get_notifier_handle);

/**/
static usb_hotplug_api_t g_usb_hotplug_api =
{
    NULL,
};

usb_hotplug_api_t* usb_get_usb_hotplug_api(USB_VOID)
{
    return &g_usb_hotplug_api;
}
EXPORT_SYMBOL(usb_get_usb_hotplug_api);

USB_INT usb_adp_get_charger_type(USB_VOID)
{
    usb_hotplug_api_t* usb_hotplug_api = usb_get_usb_hotplug_api();
    usb_hotplug_status_handle_t* usb_status_ctx = usb_get_usb_status_ctx();

    if (usb_hotplug_api->usb_get_charger_type_cb)
    {
        return usb_hotplug_api->usb_get_charger_type_cb();
    }
    else
    {
        return usb_status_ctx->cur_charger_type;
    }
}

USB_INT usb_adp_get_hotplug_old_status(USB_VOID)
{
    usb_hotplug_api_t* usb_hotplug_api = usb_get_usb_hotplug_api();
    if (usb_hotplug_api->get_hotplug_old_status_cb)
    {
        return usb_hotplug_api->get_hotplug_old_status_cb();
    }
    else
    {
        return MBB_USB_DEVICE_REMOVE;
    }
}

/*状态管理类API*/
static usb_hotplug_status_handle_t g_usb_status_ctx =
{
    .status_change_cb = NULL,
    .cur_status = MBB_USB_DEVICE_REMOVE,
    .perip_status_change_cb = NULL,
    .usb_set_charger_type_cb = NULL,
    .cur_charger_type = USB_CHARGER_TYPE_INVALID,
};

usb_hotplug_status_handle_t* usb_get_usb_status_ctx(USB_VOID)
{
    return &g_usb_status_ctx;
}
EXPORT_SYMBOL(usb_get_usb_status_ctx);

/*USB 状态变化*/
USB_VOID usb_adp_set_usb_status(USB_INT status)
{
    usb_hotplug_status_handle_t* usb_status_ctx = usb_get_usb_status_ctx();

    if ( MBB_USB_ENABLE != usb_hotplug_driver.hotplug_enable)
    {
        DBG_T(MBB_HOTPLUG, "usb hotplug is not enabled.\n");
    }
    usb_status_ctx->cur_status = status;

    if (MBB_USB_PERIP_INSERT == status || MBB_USB_PERIP_REMOVE == status)
    {
        if (usb_status_ctx->perip_status_change_cb)
        {
            usb_status_ctx->perip_status_change_cb(status);
        }
        else
        {
            DBG_T(MBB_HOTPLUG, "perip_status_change_cb not exit.\n");
        }
    }
    else
    {
        if (usb_status_ctx->status_change_cb)
        {
            usb_status_ctx->status_change_cb(status);
        }
        else
        {
            DBG_T(MBB_HOTPLUG, "status_change_cb not exit.\n");
        }
    }
}

USB_VOID usb_adp_set_charger_type(USB_INT chg_type)
{
    usb_hotplug_status_handle_t* usb_status_ctx = usb_get_usb_status_ctx();
    usb_status_ctx->cur_charger_type = chg_type;
    if (usb_status_ctx->usb_set_charger_type_cb)
    {
        usb_status_ctx->usb_set_charger_type_cb(chg_type);
    }
}

USB_VOID usb_hotplug_adp_init(USB_VOID)
{
    struct device_attribute** attrs = usb_hotplug_attributes;
    struct device_attribute* attr;
    USB_INT err = -1;
    USB_INT i = 0;
    usb_hotplug_driver.hotplug_class = class_create(THIS_MODULE, HOTPLUG_CLASS_NAME);
    usb_hotplug_driver.hotplug_dev = device_create(usb_hotplug_driver.hotplug_class, NULL,
                                     MKDEV(0, 0), NULL, HOTPLUG_DEV_NAME);

    while ((attr = *attrs++))
    {
        i++;
        err = device_create_file(usb_hotplug_driver.hotplug_dev, attr);
        if (err)
        {
            DBG_E(MBB_HOTPLUG, "create_file err %d,i=%d.\n",err,i);
            continue;
        }
    }
}

