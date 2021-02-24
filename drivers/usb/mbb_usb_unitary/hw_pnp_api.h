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



#ifndef __HW_PNP_API_H__
#define __HW_PNP_API_H__
#include "usb_debug.h"
#include "android.h"
#include "hw_pnp.h"


USB_VOID pnp_register_usb_support_function(struct android_usb_function* usb_func);

/****************************************************************
 函 数 名  : pnp_probe
 功能描述  : pnp 功能入口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID pnp_probe(USB_VOID);
/****************************************************************
 函 数 名  : pnp_remove
 功能描述  : pnp移除接口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID pnp_remove(USB_VOID);

/****************************************************************
 函 数 名  : huawei_set_usb_enum_state
 功能描述  : 设置枚举状态
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID huawei_set_usb_enum_state(usb_enum_state state);

/****************************************************************
 函 数 名  : usb_power_off_chg_stat
 功能描述  : 查询关机状态
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_INT usb_power_off_chg_stat(USB_VOID);
USB_VOID pnp_mod_soft_reconnect_timer(USB_VOID);


usb_pnp_api_hanlder *pnp_adp_get_api_handler(USB_VOID);
void android_cleanup_functions(struct android_usb_function** functions);
USB_INT Query_USB_PortType(USB_UINT* pucDialmode, USB_UINT* pucCdcSpec);
USB_VOID pnp_usb_init_enum_stat(USB_VOID);
USB_VOID pnp_accessory_disconnect(USB_VOID);

USB_INT pnp_status_check_factory(USB_VOID);
USB_INT pnp_status_check_charge_only(USB_VOID);
ssize_t enable_store(struct device* pdev, struct device_attribute* attr,const char* buff, size_t size);
USB_VOID pnp_init_device_descriptor(struct usb_composite_dev* cdev);
USB_VOID pnp_check_daynamic_sn_flag(USB_VOID);
USB_VOID usb_pnp_adp_init(USB_VOID);
USB_VOID pnp_adp_dump(USB_VOID);
USB_INT pnp_get_soft_delay(USB_VOID);
void get_support_mode_list_str(char * mode_str);
USB_VOID pnp_switch_rndis_project_mode(USB_VOID);
USB_VOID pnp_switch_rndis_debug_mode(USB_VOID);



int usb_port_enable(char *name);

USB_VOID usb_pnp_init(USB_VOID);
void usb_android_driver_init(void);
void usb_android_driver_cleanup(void);
struct class*  android_get_android_class(void);
struct android_dev*  android_get_android_dev(void);
USB_VOID android_registe_usb_driver(struct usb_composite_driver* usb_driver);

USB_VOID pnp_switch_mbim_mode(USB_INT mode);
USB_VOID ncm_set_mbim_mode(USB_INT switch_idex);
USB_UINT ncm_get_mbim_mode(USB_VOID);

USB_VOID huawei_set_adress_flag(USB_INT state);

USB_INT huawei_get_adress_flag(USB_VOID);
USB_INT mbb_usb_init(USB_VOID);
USB_VOID mbb_usb_exit(USB_VOID);
hw_usb_mode *pnp_get_huawei_usb_mode(USB_VOID);
huawei_interface_info *pnp_get_huawei_interface_vector(USB_VOID);
USB_INT pnp_usb_state_get(USB_VOID);
USB_VOID pnp_set_net_drv_state(USB_INT mode);
USB_INT pnp_get_net_drv_state(USB_VOID);
USB_INT pnp_get_dload_flag(USB_VOID);
USB_INT pnp_check_factory(USB_VOID);
void usb_mode_ctrl(unsigned int mode);

#endif
