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

#include <linux/notifier.h>
#ifdef MBB_USB_UNITARY_Q
#else
#include <drv_fastOnOff.h>

#include <mdrv_chg.h>
#include <bsp_onoff.h>
#include <mdrv_sysboot.h>
#include <power_com.h>
#endif
#include "usb_config.h"
#include "hw_pnp_api.h"
#include "usb_fast_on_off.h"
#include "usb_otg_dev_detect.h"
#include "usb_debug.h"

#ifdef USB_FAST_ON_OFF
static USB_INT fast_off_status = MBB_USB_FALSE;
/*****************************************************************
Parameters    : fastMode :假关机模式
Return        :    无
Description   :  假关机模式设置
*****************************************************************/
static USB_INT usb_fast_on_off_mode_set( FASTONOFF_MODE fastMode)
{   
    usb_pnp_api_hanlder *pnp_api_handler = pnp_adp_get_api_handler();
#ifdef USB_OTG_DEV_DETECT
    usb_otg_hanlder_t * otg_hanlder = usb_get_otg_ctx();
#endif
    DBG_T(MBB_CHARGER, "Entery mode %d!\n", fastMode);
    switch (fastMode)
    {
        /*退出假关机状态*/
        case FASTONOFF_MODE_CLR:            
            fast_off_status = MBB_USB_FALSE;
#ifdef USB_OTG_DEV_DETECT
            if(otg_hanlder->otg_usb_notify_event_cb)
            {
                otg_hanlder->otg_usb_notify_event_cb(USB_OTG_FAST_ON,NULL);
            }
            else
            {
                DBG_T(MBB_CHARGER, "otg_usb_notify_event_cb do not exist!\n");
            }
#endif
            break;
        /*进入假关机状态*/
        case FASTONOFF_MODE_SET:
            fast_off_status = MBB_USB_TRUE;
#ifdef USB_OTG_DEV_DETECT
            if(otg_hanlder->otg_usb_notify_event_cb)
            {
                otg_hanlder->otg_usb_notify_event_cb(USB_OTG_FAST_OFF,NULL);
            }
            else
            {
                DBG_T(MBB_CHARGER, "otg_usb_notify_event_cb do not exist!\n");
            }
#endif
            break;
        default:
            DBG_E(MBB_CHARGER, "defalut fast power off :fastMode:%d\n",fastMode);
            break;
    }

    if(pnp_api_handler->pnp_switch_autorun_port_cb)
    {
        pnp_api_handler->pnp_switch_autorun_port_cb();
    }
    else
    {
        DBG_I(MBB_PNP, " pnp is not insmod!\n");
    }

    return 0;
}

/*****************************************************************
Parameters    : 
Return        :    
Description   :  
*****************************************************************/
static USB_INT usb_monitor_charger_event(struct notifier_block *nb,
                                            USB_ULONG val, USB_PVOID data)
{
    USB_INT ret = 0;
    
    DBG_I(MBB_CHARGER, "%s +++usb_monitor_charger_event+++ %ld \n", __func__, val);
    ret = usb_fast_on_off_mode_set(val);
    return ret;
}

static struct notifier_block usb_monitor_charger_block =
{
    .notifier_call = usb_monitor_charger_event
};

/*****************************************************************
Parameters    :  无
Return        :  1:假关机  0: 非假关机
Description   :  获取假关机状态
*****************************************************************/
USB_INT usb_fast_on_off_stat(USB_VOID)
{
    if (MBB_USB_YES == fast_off_status)
    {
        DBG_I(MBB_PNP, " fast_on_off stat!\n");
    }
    return fast_off_status ;
}

/*****************************************************************
Parameters    :  无
Return        :    无
Description   :  USB  归一化假关机模块初始化
*****************************************************************/
USB_VOID usb_fast_on_off_init(USB_VOID)
{
    blocking_notifier_chain_register(&g_fast_on_off_notifier_list, 
                                    &usb_monitor_charger_block);
}
/*****************************************************************
Parameters    :  无
Return        :    无
Description   :  USB  归一化假关机模块退出
*****************************************************************/
USB_VOID usb_fast_on_off_exit(USB_VOID)
{
    blocking_notifier_chain_unregister(&g_fast_on_off_notifier_list, 
                                        &usb_monitor_charger_block);
}

#else
USB_INT usb_fast_on_off_stat(USB_VOID)
{
    return MBB_USB_FALSE;
}

USB_VOID usb_fast_on_off_init(USB_VOID)
{
    return ;
}

USB_VOID usb_fast_on_off_exit(USB_VOID)
{
    return ;
}
#endif
EXPORT_SYMBOL(usb_fast_on_off_stat);

