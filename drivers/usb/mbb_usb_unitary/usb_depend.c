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

#include "usb_config.h"
#include "usb_debug.h"
#include "usb_platform_comm.h"
#include "usb_event.h"
#include "usb_smem.h"

#define USB_EVENT_AVAILABLE 1
#define USB_EVENT_NOT_AVAILABLE 0

static const char* usbid_string[USB_MAX_EVENT] =
{
    [USB_ATTACH_EVENT]  = __nametostring(USB_ATTACH_EVENT),
    [USB_REMOVE_EVENT]  = __nametostring(USB_REMOVE_EVENT),
    [USB_ENABLE_EVENT]  = __nametostring(USB_ENABLE_EVENT),
    [USB_DISABLE_EVENT] = __nametostring(USB_DISABLE_EVENT),
    [USB_INIT_EVENT]    = __nametostring(USB_INIT_EVENT),
    [USB_HALT_EVENT] = __nametostring(USB_HALT_EVENT),
    [USB_RESET_EVENT] = __nametostring(USB_RESET_EVENT),
    [USB_SUSPEND_EVENT] = __nametostring(USB_SUSPEND_EVENT),
    [USB_RESUME_EVENT] = __nametostring(USB_RESUME_EVENT),
#ifdef MBB_USB_UNITARY_Q
    [USB_CRADLE_UP_EVENT] = __nametostring(USB_CRADLE_UP_EVENT),
    [USB_CRADLE_DOWN_EVENT] = __nametostring(USB_CRADLE_DOWN_EVENT),
    [USB_CRADLE_ATTACH_EVENT] = __nametostring(USB_CRADLE_ATTACH_EVENT),
    [USB_CRADLE_REMOVE_EVENT] = __nametostring(USB_CRADLE_REMOVE_EVENT),
    [USB_CRADLE_PLUGIN_EVENT] = __nametostring(USB_CRADLE_PLUGIN_EVENT),
    [USB_CRADLE_UNPLUG_EVENT] = __nametostring(USB_CRADLE_UNPLUG_EVENT),
#else
    [USB_SAMBA_PRINT_ATTACH_EVENT] = __nametostring(USB_SAMBA_PRINT_ATTACH_EVENT),
    [USB_SAMBA_PRINT_DETACH_EVENT] = __nametostring(USB_SAMBA_PRINT_DETACH_EVENT),
#endif
};

static const char* cradleid_string[CRADLE_MAX_EVENT] =
{
    [CRADLE_INSERT_EVENT]  = __nametostring(CRADLE_INSERT_EVENT),
    [CRADLE_REMOVE_EVENT]  = __nametostring(CRADLE_REMOVE_EVENT),
    [CRADLE_PPPOE_UP_EVENT]  = __nametostring(CRADLE_PPPOE_UP_EVENT),
    [CRADLE_DYNAMICIP_UP_EVENT] = __nametostring(CRADLE_DYNAMICIP_UP_EVENT),
    [CRADLE_STATICIP_UP_EVENT]    = __nametostring(CRADLE_STATICIP_UP_EVENT),
    [CRADLE_PPPOE_DYNAMICIP_FAIL_EVENT] = __nametostring(CRADLE_PPPOE_DYNAMICIP_FAIL_EVENT),
    [CRADLE_LAN_UP_EVENT] = __nametostring(CRADLE_LAN_UP_EVENT),
    [CRADLE_MUTI_CAST_EVENT] = __nametostring(CRADLE_MUTI_CAST_EVENT),
};

static const char* chargerid_string[CHARGER_MAX_EVENT] =
{
    [CHARGER_ATTATCH_EVENT]  = __nametostring(CHARGER_ATTATCH_EVENT),
    [CHARGER_REMOVE_EVENT]  = __nametostring(CHARGER_REMOVE_EVENT),
};
static const char* sdid_string[SD_EVENT_MAX_EVENT] =
{
    [SD_ATTATCH_EVENT]  = __nametostring(SD_ATTATCH_EVENT),
    [SD_REMOVE_EVENT]  = __nametostring(SD_REMOVE_EVENT),
    [U_DISK_ATTATCH_EVENT]  = __nametostring(U_DISK_ATTATCH_EVENT),
    [U_DISK_REMOVE_EVENT]  = __nametostring(U_DISK_REMOVE_EVENT),
};
/*============================================================================*/
/*USB事件功能模块*/
/*============================================================================*/

/*****************************************************************
Parameters    :  eventcode
Return        :    
Description   :  usb事件有效性判断
*****************************************************************/
static USB_INT usb_event_usbid_available(USB_INT eventcode)
{
    /*usb event 从USB_ATTACH_EVENT(0)开始*/
    if (eventcode >= USB_MAX_EVENT || eventcode < USB_ATTACH_EVENT)
    {
        return USB_EVENT_NOT_AVAILABLE;
    }
    else
    {
        DBG_I(MBB_EVENT, "usbid event not available !!\n");
        return USB_EVENT_AVAILABLE;
    }
}

/*****************************************************************
Parameters    :  eventcode
Return        :    
Description   :  cradle事件有效性判断
*****************************************************************/
static USB_INT usb_event_cradleid_available(USB_INT eventcode)
{
    /*cradle event 从CRADLE_INSERT_EVENT(1)开始*/
    if (eventcode >= CRADLE_MAX_EVENT || eventcode < CRADLE_INVAILD_EVENT)
    {
        return USB_EVENT_NOT_AVAILABLE;
    }
    else
    {
        DBG_I(MBB_EVENT, "cradleid event not available !!\n");
        return USB_EVENT_AVAILABLE;
    }
}

/*****************************************************************
Parameters    :  eventcode
Return        :    
Description   :  charger事件有效性判断
*****************************************************************/
static USB_INT usb_event_chargerid_available(USB_INT eventcode)
{
    /*charger event 从CHARGER_ATTATCH_EVENT(0)开始*/
    if (eventcode >= CHARGER_MAX_EVENT || eventcode < 0)
    {
        return USB_EVENT_NOT_AVAILABLE;
    }
    else
    {
        DBG_I(MBB_EVENT, "chargerid event not available !!\n");
        return USB_EVENT_AVAILABLE;
    }
}

/*****************************************************************
Parameters    :  eventcode
Return        :    
Description   :  sd事件有效性判断
*****************************************************************/
static USB_INT usb_event_sdid_available(USB_INT eventcode)
{
    /*sd event 从SD_ATTATCH_EVENT(0)开始*/
    if (eventcode >= SD_EVENT_MAX_EVENT || eventcode < SD_ATTATCH_EVENT)
    {
        return USB_EVENT_NOT_AVAILABLE;
    }
    else
    {
        DBG_I(MBB_EVENT, "sdid event not available !!\n");
        return USB_EVENT_AVAILABLE;
    }
}

/*****************************************************************
Parameters    :  eventcode
Return        :    
Description   :  usb事件名称获取
*****************************************************************/
static const char* usb_event_usbid_string(USB_INT eventcode)
{
    USB_INT ret = USB_EVENT_NOT_AVAILABLE;

    ret = usb_event_usbid_available(eventcode);
    if ( USB_EVENT_AVAILABLE == ret)
    { return usbid_string[eventcode]; }
    else
    { return "usbid INVAILD eventcode"; }
}

/*****************************************************************
Parameters    :  eventcode
Return        :    
Description   :  cradle事件名称获取
*****************************************************************/
static const char* usb_event_cradleid_string(USB_INT eventcode)
{
    USB_INT ret = USB_EVENT_NOT_AVAILABLE;

    ret = usb_event_cradleid_available(eventcode);
    if ( USB_EVENT_AVAILABLE == ret)
    { return cradleid_string[eventcode]; }
    else
    { return "cradleid INVAILD eventcode"; }
}

/*****************************************************************
Parameters    :  eventcode
Return        :    
Description   :  charger事件名称获取
*****************************************************************/
static const char* usb_event_chargerid_string(USB_INT eventcode)
{
    USB_INT ret = USB_EVENT_NOT_AVAILABLE;

    ret = usb_event_chargerid_available(eventcode);
    if ( USB_EVENT_AVAILABLE == ret)
    { return chargerid_string[eventcode]; }
    else
    { return "chargerid INVAILD eventcode"; }
}

/*****************************************************************
Parameters    :  eventcode
Return        :    
Description   :  sd事件名称获取
*****************************************************************/
static const char* usb_event_sdid_string(USB_INT eventcode)
{
    USB_INT ret = USB_EVENT_NOT_AVAILABLE;

    ret = usb_event_sdid_available(eventcode);
    if ( USB_EVENT_AVAILABLE == ret)
    { return sdid_string[eventcode]; }
    else
    { return "sdid INVAILD eventcode"; }
}

/*****************************************************************
Parameters    :  deviceid 
                 eventcode
Return        :    
Description   :  事件名称获取
*****************************************************************/
static const char* usb_event_deviceid_string(USB_INT deviceid, USB_INT eventcode)
{
    switch (deviceid)
    {
        case EVENT_DEVICE_USB:
            return usb_event_usbid_string(eventcode);
        case EVENT_DEVICE_CHARGER:
            return usb_event_chargerid_string(eventcode);
        case EVENT_DEVICE_CRADLE:
            return usb_event_cradleid_string(eventcode);
        case EVENT_DEVICE_SD:
            return usb_event_sdid_string(eventcode);
        default:
            return "INVAILD DEVICE";
    }
}

/*****************************************************************
Parameters    :  deviceid 
                 eventcode
Return        :    
Description   :  事件有效性判断
*****************************************************************/
static USB_INT usb_event_available(USB_INT deviceid, USB_INT eventcode)
{
    USB_INT ret = USB_EVENT_NOT_AVAILABLE;
    switch (deviceid)
    {
        case EVENT_DEVICE_USB:
            return usb_event_usbid_available(eventcode);
        case EVENT_DEVICE_CHARGER:
            return usb_event_cradleid_available(eventcode);
        case EVENT_DEVICE_CRADLE:
            return usb_event_chargerid_available(eventcode);
        case EVENT_DEVICE_SD:
            return usb_event_sdid_available(eventcode);
        default:
            DBG_I(MBB_EVENT, "event deviceid not available !!\n");
            return ret;
    }
}

#ifdef USB_EVENT_NOTIFY
/*****************************************************************
Parameters    :  deviceid  : 设备ID    eventcode :  事件
Return        :    无
Description   :  向应用层上报  USB   事件
*****************************************************************/
USB_VOID usb_notify_syswatch(USB_INT deviceid, USB_INT eventcode)
{
    USB_INT ret = USB_EVENT_NOT_AVAILABLE;
    /*事件有效性判断*/
    ret = usb_event_available(deviceid, eventcode);
    if ( USB_EVENT_NOT_AVAILABLE == ret)
    {
        DBG_T(MBB_EVENT, "not available event <%d,%d>\n", deviceid, eventcode);
        return;
    }
    DBG_T(MBB_EVENT, "<%d,%d><%s>\n", deviceid, eventcode,
          usb_event_deviceid_string(deviceid, eventcode));
#ifdef MBB_USB_UNITARY_Q
    /*高通平台发送事件*/
    nl_dev_type nl_dev;
    nl_dev.device_id = deviceid;
    nl_dev.value = eventcode;
    nl_dev.desc = NULL;
    nl_dev.desc_len = 0;
    (USB_VOID)syswatch_nl_send(nl_dev);
#else/*MBB_USB_UNITARY_Q*/
    /*balong平台发送事件*/
    DEVICE_EVENT stusbEvent = {0};
    stusbEvent.device_id = deviceid;
    stusbEvent.event_code = eventcode;
    stusbEvent.len = 0;
    (USB_VOID)device_event_report(&stusbEvent, sizeof(DEVICE_EVENT));
#endif/*MBB_USB_UNITARY_Q*/
}

#else/*USB_EVENT_NOTIFY*//*平台notify 不可用时，打桩*/
USB_VOID usb_notify_syswatch(USB_INT deviceid, USB_INT eventcode)
{
    DBG_T(MBB_EVENT, "usb_notify not enable !!\n");
    DBG_I(MBB_EVENT, "<%d,%d><%s>\n", deviceid, eventcode,
          usb_event_deviceid_string(deviceid, eventcode));
}
#endif/*USB_EVENT_NOTIFY*/
EXPORT_SYMBOL_GPL(usb_notify_syswatch);

/*============================================================================*/
/*USB事件功能模块*/
/*============================================================================*/



/*============================================================================*/
/*USB 共享内存模块*/
/*============================================================================*/

#ifdef USB_SMEM
USB_SMEM_COMMOM* usb_get_smem_info(USB_VOID)
{
#ifdef MBB_USB_UNITARY_Q
    return (USB_SMEM_COMMOM*)SMEM_HUAWEI_ALLOC(SMEM_ID_VENDOR0,
            sizeof(USB_SMEM_COMMOM));
#else/*MBB_USB_UNITARY_Q*/
    return (USB_SMEM_COMMOM*)SRAM_DLOAD_ADDR;
#endif/*MBB_USB_UNITARY_Q*/
}

#ifdef MBB_USB_UNITARY_Q
dynamic_smem_vendor1* usb_get_smem_info_vendor1(USB_VOID)
{
    return (dynamic_smem_vendor1*)SMEM_HUAWEI_ALLOC(SMEM_ID_VENDOR1,
            sizeof(dynamic_smem_vendor1));
}
#endif/*MBB_USB_UNITARY_Q*/

#else/*USB_SMEM*//*平台smem 不可用时，打桩*/
USB_SMEM_COMMOM* usb_get_smem_info(USB_VOID)
{
    return NULL;
}

dynamic_smem_vendor1* usb_get_smem_info_vendor1(USB_VOID)
{
    return NULL;
}
#endif/*USB_SMEM*/
/*============================================================================*/
/*USB 共享内存模块*/
/*============================================================================*/

