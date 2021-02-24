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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/platform_device.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include "gadget_chips.h"
#include <linux/proc_fs.h>
#include <linux/netlink.h>
#include <linux/notifier.h>

#include "usb_platform_comm.h"
#include "hw_pnp.h"
#include "u_ether.h"
#include "adapt/hw_net_dev.h"
#include "usb_nv_get.h"
//#include "mbb_fast_on_off.h" //mahialong deleted
#include "u_serial.h"
#include "f_acm.h"
#include "u_modem.h"
#include "u_cdev.h"
#include "f_mbb_storage.h"

#ifdef USB_CHARGE
#include <mdrv_chg.h>
#endif
#include "usb_debug.h"
#include "usb_platform_comm.h"
#include "mbb_usb_adp.h"
#ifdef USB_OTG_DEV_DETECT
#include "usb_otg_dev_detect.h"
#endif

#include "usb_fast_on_off.h"

#ifdef MBB_Q_9x_PLATFORM
//#include "xxx.h"

#endif /*MBB_Q_PLATFORM*/

/*
 * Kbuild is not very cooperative with respect to linking separately
 * compiled library objects into one module.  So for now we won't use
 * separate compilation ... ensuring init/exit sections work to shrink
 * the runtime footprint, and giving us at least some parts of what
 * a "gcc --combine ... part1.c part2.c part3.c ... " build would.
 */
#include "hw_pnp_adapt.c"
#include "usbstring.c"
#include "config.c"
#include "epautoconf.c"
#include "composite.c"
//#include "f_sd.c"

#ifdef USB_ADB
#include "f_adb.c"
#endif

#include "f_ncm_mbb.h"
#include "f_ecm_mbb.h"
#include "rndis.h"

static struct class* android_class  = NULL;

static struct android_dev* _android_dev  = NULL;

static struct usb_composite_driver* android_usb_driver = NULL;


MODULE_AUTHOR("Mike Lockwood");
MODULE_DESCRIPTION("Android Composite USB Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");


/* 高通平台初始化相关操作*/


/* 巴龙平台初始化相关操作*/

static USB_INT g_is_usb_mbb_init = 0;


extern int usb_dwc3_balong_drv_init(void);
extern void usb_dwc3_balong_drv_exit(void);
extern int usb_dwc3_platform_drv_init(void);
extern void usb_dwc3_platform_drv_exit(void);
extern int dwc3_usb2phy_init(void);
extern void dwc3_usb2phy_exit(void);
extern int dwc3_usb3phy_init(void);
extern void dwc3_usb3phy_exit(void);



#ifdef CONFIG_USB_OTG
extern int dwc_otg_init(void);
extern void dwc_otg_exit(void);
#endif

struct class*  android_get_android_class(void)
    {
        return android_class;
    }
EXPORT_SYMBOL_GPL(android_get_android_class);

struct android_dev*  android_get_android_dev(void)
{
    return _android_dev;
}
EXPORT_SYMBOL_GPL(android_get_android_dev);

USB_VOID android_registe_usb_driver(struct usb_composite_driver* usb_driver)
{
    android_usb_driver = usb_driver;
}
EXPORT_SYMBOL_GPL(android_registe_usb_driver);


void usb_android_driver_init(void)
{
    struct android_dev* dev = android_get_android_dev();
    USB_INT vbus_status = 0;
    usb_hotplug_api_t* usb_hotplug_api = usb_get_usb_hotplug_api();
    usb_pnp_api_hanlder* pnp_api_handler = pnp_adp_get_api_handler();
    int err = 0;

    DBG_T(MBB_PNP, "\n");

    if ( NULL == dev)
    {
        DBG_I(MBB_PNP, "android_get_android_dev NULL!\n");
        return ;
    }

    if ( MBB_USB_YES == dev->android_driver_init)
    {
        DBG_I(MBB_PNP, "android_driver_already init \n");
        return;
    }
    dev->android_driver_init = MBB_USB_YES;

    if (pnp_api_handler->android_create_device_cb)
    {
        err = pnp_api_handler->android_create_device_cb(dev, 0);
    }
    else
    {
        pr_err("%s(): android_create_device_cb NULL\n", __func__);
        dev->android_driver_init = MBB_USB_NO;
    }

    if (err)
    {
        pr_err("%s(): android_create_device failed\n", __func__);
        dev->android_driver_init = MBB_USB_NO;
    }

    err = usb_composite_probe(android_usb_driver);
    if (err)
    {
        pr_err("%s(): Failed to register android composite driver\n",
               __func__);
        dev->android_driver_init = MBB_USB_FALSE;
    }

    /*pnp start*/
    if (NULL != usb_hotplug_api->get_hotplug_status_cb)
    {
        vbus_status = usb_hotplug_api->get_hotplug_status_cb();
        if (vbus_status)
        {
            if (pnp_api_handler->pnp_probe_cb)
            {
                pnp_api_handler->pnp_probe_cb();
            }
            else
            {
                pr_err("%s(): pnp_probe_cb NULL\n", __func__);
            }
        }
    }
    else
    {
        DBG_T(MBB_PNP, "usb_hotplug_api get_hotplug_status_cb NOT EXIT !\n");
    }
}
EXPORT_SYMBOL_GPL(usb_android_driver_init);



USB_INT mbb_usb_init(USB_VOID)
{
    struct android_dev* android_dev;
    USB_INT ret = 0;
    USB_INT err;

    DBG_T(MBB_PNP, "enter\n");

    if (1 == get_dload_flag())
    {
        DBG_E(MBB_PNP, " is_in_dload is %d.\n", is_in_dload);
        return -EPERM;
    }

    while (0 == g_is_usb_mbb_init)
    {
        msleep(100); //PMU中断有时较早，需要同步等待USB初始化完毕
    }
    /*资源创建时，先创建底层资源再创建上层资源*/

    if ( usb_balong_init() < 0)
    {
        DBG_E(MBB_PNP, "usb_balong_init error.\n");
        return -EPERM;
    }
    if (!android_class)
    {
        android_class = class_create(THIS_MODULE, "android_usb");

        if (IS_ERR(android_class))
        { return PTR_ERR(android_class); }
    }

    android_dev = kzalloc(sizeof(*android_dev), GFP_KERNEL);
    if (!android_dev)
    {
        DBG_E(MBB_PNP, "Failed to alloc memory for android_dev\n");
        ret = -ENOMEM;
        goto ret_dev_exit;
    }

    android_dev->name = "android_usb";
    android_dev->disable_depth = 1;
    android_dev->functions = supported_functions;
    android_dev->configs_num = 0;
    android_dev->irq_ctl_port = MBB_USB_YES;
    android_dev->android_driver_init = MBB_USB_NO;
    INIT_LIST_HEAD(&android_dev->android_configs);
    INIT_WORK(&android_dev->work, android_work);
    mutex_init(&android_dev->mutex);

    _android_dev = android_dev;

    usb_android_driver_init();


    mod_timer(&g_soft_reconnect_timer, jiffies + msecs_to_jiffies(g_soft_reconnect_loop));

    DBG_I(MBB_PNP, "ok\n");

    return 0;

ret_dev_exit:
    usb_balong_exit(MBB_USB_FALSE);
	return -1;

}

USB_VOID mbb_usb_exit(USB_VOID)
{
    struct android_dev* dev = android_get_android_dev();
    usb_pnp_api_hanlder* pnp_api_handler = pnp_adp_get_api_handler();

    DBG_T(MBB_PNP, "enter");

    if (NULL == dev)
    {
        DBG_E(MBB_PNP, "mbb_usb_exit: init failed.\n");
        return;
    }

    if (1 == get_dload_flag())
    {
        DBG_E(MBB_PNP, "is_in_dload is %d.\n",  is_in_dload);
        return;
    }
    /*拔出时清除定时器*/
    del_timer_sync(&g_soft_reconnect_timer);

    usb_notify_syswatch(EVENT_DEVICE_USB, USB_REMOVE_EVENT);

    /*资源释放时，先释放上层再释放底层*/
    if (pnp_api_handler->pnp_remove_direct_cb)
    {
        pnp_api_handler->pnp_remove_direct_cb();
    }
    /*释放资源与pnp_switch_func 存在竞争关系，需要同步*/
    mutex_lock(&dev->mutex);
    usb_composite_unregister(android_usb_driver);
    _android_dev = NULL;
    if (pnp_api_handler->android_destroy_device_cb)
    {
        pnp_api_handler->android_destroy_device_cb(dev);
    }
    mutex_unlock(&dev->mutex);

    class_destroy(android_class);
    android_class = NULL;
    kfree(dev);
    huawei_set_usb_enum_state(USB_ENUM_NONE);
    usb_balong_exit(MBB_USB_FALSE);
    DBG_I(MBB_PNP, "ok\n");
    return;
}

static USB_INT __init init(USB_VOID)
{
    //USB 日志管理初始化
    usb_debug_init();
    usb_hotplug_adp_init();
    usb_fast_on_off_init();
    usb_workqueue_init();
    //初始化NV
    usb_nv_init();
    //usb_nv_get();

    bsp_usb_adapter_init();
    usb_mass_storage_init();
    usb_serial_init();

    usb_rndis_init();
    usb_ncm_init();
    usb_ecm_init();
#ifdef USB_ADB
    usb_adb_init();
#endif
    usb_acm_init();
    usb_cdev_init();
    usb_modem_init();
#ifdef USB_CHARGE
    usb_charger_init();
#endif

    g_is_usb_mbb_init = 1;

    return 0;
}
module_init(init);

static USB_VOID __exit cleanup(USB_VOID)
{
#ifdef USB_CHARGE
    //模块移除的收尾工作
    usb_charger_exit();
#endif
    usb_fast_on_off_exit();
#ifdef USB_OTG_DEV_DETECT
    /*移除对外充事件电接口*/
    usb_otg_device_detect_exit();
#endif
    g_is_usb_mbb_init = 0;
}
module_exit(cleanup);


