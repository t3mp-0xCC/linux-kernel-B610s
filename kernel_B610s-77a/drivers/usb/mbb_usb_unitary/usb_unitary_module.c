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

#include <linux/module.h>
#include "usb_config.h"
#include "usb_debug.h"
#include "usb_platform_comm.h"

/*hotplug 功能*/
#include "usb_hotplug.h"
/*otg_dev 检测功能*/
#ifdef USB_OTG_DEV_DETECT
#include "usb_otg_dev_detect.h"
#endif
/*hw pnp功能*/
#include "hw_pnp_api.h"
/*mass功能*/
#include "f_mass_storage_api.h"

//#include "hw_pnp.c"
//#include "usb_hotplug.c"
#ifdef USB_OTG_DEV_DETECT
#include "usb_otg_dev_detect.c"
#endif
//#include "f_mass_storage.c"

USB_INT __init usb_unitary_module_init(USB_VOID)
{
    /*初始化注意优先级*/

    usb_mass_storage_init();
    
    usb_pnp_init();
#ifdef USB_OTG_DEV_DETECT
    usb_otg_device_detect_init();
#endif
    usb_hotplug_init();

    return 0;
}
module_init(usb_unitary_module_init);

USB_VOID __exit usb_unitary_module_exit(USB_VOID)
{
    usb_hotplug_exit();
#ifdef USB_OTG_DEV_DETECT
    usb_otg_device_detect_exit();
#endif

}
module_exit(usb_unitary_module_exit);

MODULE_DESCRIPTION("HUAWEI usb unitary");
MODULE_AUTHOR("HUAWEI usb");
MODULE_LICENSE("GPL");

