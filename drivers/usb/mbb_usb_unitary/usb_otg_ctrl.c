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
#include "usb_debug.h"
#include "hw_pnp_api.h"
#include "usb_config.h"

#define USB_ID_DEVICE_MODE 1  /*DEVICE mode*/
#define USB_ID_HOST_MODE   0  /*host mode*/

void usb_mode_ctrl(unsigned int mode)
{
    /*mode �������0��1����ת��Ϊ0/1*/
    mode = !!mode;
    DBG_I(MBB_USB_OTG, "set to mode %s\n", mode ? "DEVICE_MODE" : "HOST_MODE" );
    sysctrl_set_usbid(mode);
}

#ifdef USB_CPE
void usb3_otg_cpe_init_mode(void)
{
    static int inited = 0;
    int ret = 0;
    int id_status = USB_ID_HOST_MODE;

    if (inited)
    {
        return;
    }

    /*��ID���ƿ���*/
    syssc_usb_iddig_en(1);

#if (FEATURE_ON == MBB_DT_USBLOADER)
    /*USBLOADER ʱ����deviceģʽ*/
    DBG_I(MBB_USB_OTG, "MBB_DT_USBLOADER mode\n");
    usb_mode_ctrl(USB_ID_DEVICE_MODE);    /* Device mode */
#else
    /*NV ���ݻָ�����deviceģʽ*/
    if (PORT_NV_RES == pnp_get_dload_flag())
    {
        id_status = USB_ID_DEVICE_MODE;
        DBG_I(MBB_USB_OTG, "PORT_NV_RES mode\n");
    }
    /*����ʱ����deviceģʽ*/
    else if (PORT_DLOAD == pnp_get_dload_flag())
    {
        id_status = USB_ID_DEVICE_MODE;
        DBG_I(MBB_USB_OTG, "PORT_DLOAD mode\n");
    }
#if	(FEATURE_ON == MBB_FACTORY)
	id_status = USB_ID_DEVICE_MODE;
#endif
    sysctrl_set_usbid(id_status);      /* Host mode or dload mode */
#endif

    inited = 1;
}
#else/*USB_CPE*/
void usb3_otg_cpe_init_mode(void)
{
    DBG_T(MBB_USB_OTG, "with no define USB_CPE\n");
}
#endif/*USB_CPE*/
