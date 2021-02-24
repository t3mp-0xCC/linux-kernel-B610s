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
/*
该文件包含了所有OTG 设备检测所需的平台差异接口

MBB_USB_UNITARY_Q/MBB_USB_UNITARY_B

后续如果有平台差异修改，请添加到这个文件

usb_otg_dev_adp.c 不要出现平台差异判断

*/


#include <linux/irq.h>

#include "usb_config.h"
#include "usb_platform_comm.h"
#include "usb_debug.h"
#include "usb_otg_dev_detect.h"
#include "usb_hotplug.h"
#include "usb_event.h"

#ifdef MBB_USB_UNITARY_Q
#else/*MBB_USB_UNITARY_Q*/
#include <linux/usb/usb_interface_external.h>
//#include "usb_otg_extern_charge.h"
#include "usb_vendor.h"
#include "bsp_version.h"
#include "mdrv_version.h"
#include "mbb_usb_adp.h"
#include "usb_notify.h"
#include <mbb_config.h>
#endif/*MBB_USB_UNITARY_Q*/

#ifdef MBB_USB_UNITARY_Q
extern void qc_otg_host_on_off(int id);
extern int qc_otg_get_vbus_state(void);
#endif

#define CHECK_ID_VOLT_LOW       (150)
#define CHECK_ID_VOLT_HIG       (250)


USB_VOID otg_dev_request_source(USB_VOID)
{
#ifndef MBB_USB_UNITARY_Q
    USB_INT ret = -1;
    if (false == gpio_is_valid(GPIO_OTG_ID_SET) || false == gpio_is_valid(GPIO_DMDP_CONNECT))
    {
        DBG_E(MBB_OTG_CHARGER, "GPIO_OTG_ID_SET or GPIO_OTG_ID_SET is invalid\n");
        return;
    }
    ret = gpio_request(GPIO_DMDP_CONNECT, "OTG_DP_DM_CONNECT_GPIO");
    if (ret < 0)
    {
        DBG_E(MBB_OTG_CHARGER, "gpio request failed for OTG_DP_DM_CONNECT_GPIO\n");
        gpio_free(GPIO_DMDP_CONNECT);
    }
    ret = gpio_request(GPIO_OTG_ID_SET, "GPIO_OTG_ID_SET");
    if (ret < 0)
    {
        DBG_E(MBB_OTG_CHARGER, "gpio request failed for GPIO_OTG_ID_SET\n");
        gpio_free(GPIO_OTG_ID_SET);
    }
    gpio_direction_output(GPIO_DMDP_CONNECT, 0);
    gpio_direction_output(GPIO_OTG_ID_SET, 1);
#else
    /*qualcomm*/
#endif
}
EXPORT_SYMBOL(otg_dev_request_source);

USB_VOID otg_dev_free_source(USB_VOID)
{
#ifndef MBB_USB_UNITARY_Q

    if (gpio_is_valid(GPIO_DMDP_CONNECT))
    {
        gpio_free(GPIO_DMDP_CONNECT);
    }
    if (gpio_is_valid(GPIO_OTG_ID_SET))
    {
        gpio_free(GPIO_OTG_ID_SET);
    }
#else
    /*qualcomm*/
#endif

}
EXPORT_SYMBOL(otg_dev_free_source);


USB_VOID otg_dev_set_platform(struct otg_dev_det* otg)
{
#ifdef MBB_USB_UNITARY_Q
    otg->platform = PLATFORM_QUALCOMM;
#else
    otg->platform = PLATFORM_BALONG;
#endif
}
EXPORT_SYMBOL(otg_dev_set_platform);


/*编译问题，该函数用宏报，
如果后续，高通用，可去掉宏*/
USB_VOID  otg_gpio_clear_set(USB_INT gpio)
{
#ifndef MBB_USB_UNITARY_Q
   if(gpio_is_valid(gpio))
   	{
    gpio_int_unmask_set(gpio);
    gpio_int_state_clear(gpio);
   	}
#else
    /*qualcomm operation*/
#endif
}
EXPORT_SYMBOL(otg_gpio_clear_set);

static USB_VOID  otg_gpio_set(USB_INT gpio)
{
#ifndef MBB_USB_UNITARY_Q
    if (gpio_is_valid(gpio))
    {
        gpio_int_mask_set(gpio);
        gpio_int_state_clear(gpio);
    }
#else
    /*qualcomm operation*/
#endif
}
EXPORT_SYMBOL(otg_gpio_set);
USB_VOID otg_id_gpio_irq_set(USB_INT id_gpio)
{
#ifndef MBB_USB_UNITARY_Q
    if (gpio_is_valid(id_gpio))
    {
        gpio_int_mask_set(id_gpio);
        gpio_int_state_clear(id_gpio);
        gpio_set_function(id_gpio, GPIO_INTERRUPT);
    }
#endif
}
EXPORT_SYMBOL(otg_id_gpio_irq_set);


USB_VOID otg_id_trigger_set(struct otg_dev_det* otg)
{
    return;
}
EXPORT_SYMBOL(otg_id_trigger_set);


USB_VOID otg_host_on_off(struct otg_dev_det* otg)
{
#ifdef MBB_USB_UNITARY_Q
#ifdef USB_OTG_DEV_DETECT
    qc_otg_host_on_off(otg->phy_id);
#endif
#else
    if (!otg->phy_id)
    {
        usb_adp_set_usb_status(MBB_USB_PERIP_INSERT);
    }
    else
    {
        usb_adp_set_usb_status(MBB_USB_PERIP_REMOVE);
    }

    /*断开 D+, D- , 拉低 ID */
    if (gpio_is_valid(GPIO_DMDP_CONNECT))
    {
        gpio_set_value(GPIO_DMDP_CONNECT, GPIO_LOW);
    }
    if (gpio_is_valid(GPIO_OTG_ID_SET))
    {
        gpio_set_value(GPIO_OTG_ID_SET, otg->phy_id);
    }
#endif

    otg->host = otg->phy_id ? MBB_USB_FALSE : MBB_USB_TRUE;
}
EXPORT_SYMBOL(otg_host_on_off);


USB_INT otg_get_vbus_state(struct otg_dev_det* otg)
{
#ifdef MBB_USB_UNITARY_Q
#ifdef USB_OTG_DEV_DETECT
    return qc_otg_get_vbus_state();
#else
    return 0;
#endif
#else
    return usb_get_vbus_status();
#endif
}
EXPORT_SYMBOL(otg_get_vbus_state);

USB_VOID otg_id_irq_flags_set(struct otg_dev_det* otg)
{
#ifndef MBB_USB_UNITARY_Q
    otg->id_irqflags = IRQF_NO_SUSPEND | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
    otg->id_irq_shared = MBB_USB_TRUE;
#else/*MBB_USB_UNITARY_Q*/
    otg->id_irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
    otg->id_irq_shared = MBB_USB_FALSE;
#endif
}
EXPORT_SYMBOL(otg_id_irq_flags_set);


USB_INT otg_id_irq_share_protect(struct otg_dev_det* otg)
{
    if (gpio_is_valid((unsigned)otg->id_det_gpio))
    {
        return gpio_int_state_get((unsigned)otg->id_det_gpio);
    }
    else
    {
        return 1;
    }
}
EXPORT_SYMBOL(otg_id_irq_share_protect);


USB_VOID otg_first_report_charge_type(struct otg_dev_det* otg)
{
    //static USB_INT first_report = MBB_USB_TRUE;
#ifndef MBB_USB_UNITARY_Q
    static USB_INT first_report = MBB_USB_TRUE;
    usb_adp_ctx_t* ctx = otg->contex;
    if (ctx->stm_set_chg_type_cb && first_report)
    {
        DBG_T(MBB_OTG_CHARGER, "-----first notify charger event----%d\n",otg->charge_type);
        ctx->stm_set_chg_type_cb(otg->charge_type);
        first_report = MBB_USB_FALSE;
    }
#else
    /*qualcomm*/
#endif
}
EXPORT_SYMBOL(otg_first_report_charge_type);

USB_VOID otg_dev_source_set(USB_VOID)
{
#ifndef  MBB_USB_UNITARY_Q
    if (gpio_is_valid(GPIO_DMDP_CONNECT))
    {
        otg_gpio_clear_set(GPIO_DMDP_CONNECT);
    }

    if (gpio_is_valid(GPIO_OTG_ID_SET))
    {
        otg_gpio_clear_set(GPIO_OTG_ID_SET);
    }

#else
    /*qualcommm operation*/
#endif
}
EXPORT_SYMBOL(otg_dev_source_set);

USB_VOID otg_exchg_connect_dpdm(struct otg_dev_det* otg)
{
    /*短接 D+ D- , 进行 1 A 充电*/
    otg->debug.stat_usb_dpdm_connect++;
#ifndef MBB_USB_UNITARY_Q
    if (gpio_is_valid(GPIO_DMDP_CONNECT))
    {
        gpio_set_value(GPIO_DMDP_CONNECT, GPIO_HIGH);
    }
#else
    /*qualcomm op*/
#endif
}
EXPORT_SYMBOL(otg_exchg_connect_dpdm);
USB_VOID otg_exchg_disconnect_dpdm(struct otg_dev_det* otg)
{
    otg->debug.stat_usb_dpdm_disconnect++;
#ifndef MBB_USB_UNITARY_Q
    if (gpio_is_valid(GPIO_DMDP_CONNECT))
    {
        gpio_set_value(GPIO_DMDP_CONNECT, GPIO_LOW);
    }
#else
    /*qualcomm op */
#endif
}
EXPORT_SYMBOL(otg_exchg_disconnect_dpdm);
USB_VOID otg_exchg_disconnect_dpdm_to_host(struct otg_dev_det* otg)
{
    otg->debug.stat_usb_dpdm_disconnect++;
#ifndef MBB_USB_UNITARY_Q
    if (gpio_is_valid(GPIO_DMDP_CONNECT))
    {
    gpio_set_value(GPIO_DMDP_CONNECT, GPIO_LOW);
    	}
#else
    /*qualcomm op */
#endif
    otg->phy_id = HOST_ON;
    otg_host_on_off(otg);
}
EXPORT_SYMBOL(otg_exchg_disconnect_dpdm_to_host);

usb_otg_hanlder_t g_otg_api = 
{
    NULL,
};

usb_otg_hanlder_t *usb_get_otg_ctx(USB_VOID)
{
    return &g_otg_api;
}
EXPORT_SYMBOL(usb_get_otg_ctx);

