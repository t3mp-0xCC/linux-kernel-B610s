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

#include <linux/string.h>
#include "usb_nv_get.h"
#include "usb_debug.h"
#include "f_mbb_storage.h"
#include "hw_pnp_api.h"
#include "hw_pnp_adapt.h"

#ifdef USB_SOLUTION
#include "mdrv_version.h"
#endif
static mbb_usb_nv_info_st g_usb_nv_info;/*全局变量禁止直接使用，请使用usb_nv_get_ctx()*/
static nv_protocol_base_type gstDevProtOfsDefault = { 0x00, 0x00 };

static nv_cust_pid_type gstDevPidDefault = { 0x00, CDROM_SCENE_PID, PID_USER };

#ifdef USB_RNDIS
static uint8_t gRewindPortDefault[DYNAMIC_PID_MAX_PORT_NUM] =
{
    USB_IF_PROTOCOL_RNDIS,
#ifdef USB_SD
    USB_IF_PROTOCOL_SDRAM,
#endif
};
#else/*USB_RNDIS*/

static uint8_t gRewindPortDefault[DYNAMIC_PID_MAX_PORT_NUM] =
{
    USB_IF_PROTOCOL_PCUI,      //PCUI
};
#endif/*USB_RNDIS*/

/*将模块的protocol转译为，通用的protocol*/
protocol_translation_info translation_table[] =
{
    {HSU_CONF_HWDEV_MODEM_IFACE, USB_IF_PROTOCOL_MODEM },    //MDM
    {HSU_CONF_HWDEV_PCUI_IFACE, USB_IF_PROTOCOL_PCUI },      //PCUI
    {HSU_CONF_HWDEV_DIAG_IFACE, USB_IF_PROTOCOL_DIAG },      //DIAG
    {HSU_CONF_HWDEV_GPSNMEA_IFACE, USB_IF_PROTOCOL_GPS },    //GPS
    {HSU_CONF_HWDEV_RMNET_IFACE, USB_IF_PROTOCOL_NDIS },     //NDIS
    {HSU_CONF_HWDEV_ECM_DATA_IFACE, USB_IF_PROTOCOL_RNDIS }, //ECM
    /*PCSC need to be updated*/
    /*MBIM translation*/
    {HSU_CONF_HWDEV_MBIM_IFACE, USB_IF_PROTOCOL_MBIM },     //MBIM
};

static USB_UINT8 gFirstConfigPortDefault[DYNAMIC_PID_MAX_PORT_NUM] =
{
    USB_IF_PROTOCOL_MODEM,    //MODEM
    USB_IF_PROTOCOL_DIAG,     //DIAG
    USB_IF_PROTOCOL_PCUI,     //PCUI
#ifdef MBB_USB_UNITARY_Q
    USB_IF_PROTOCOL_NDIS,
#else
    USB_IF_PROTOCOL_NCM,      //NCM
#endif/*MBB_USB_UNITARY_Q*/
    USB_IF_PROTOCOL_GPS,      //GPS
#ifdef MBB_USB_UNITARY_Q
    USB_IF_PROTOCOL_ADB,
#else
    USB_IF_PROTOCOL_COMM_B,
#endif/*MBB_USB_UNITARY_Q*/
};

static USB_UINT8 gSecondConfigPortDefault[DYNAMIC_PID_MAX_PORT_NUM] =
{
    /*ECM 放在第一位，linux系统自动选择该config*/
    USB_IF_PROTOCOL_RNDIS,   //linux 使用时pnp自动更新为ECM
    USB_IF_PROTOCOL_MODEM,   //MODEM
    USB_IF_PROTOCOL_DIAG,    //DIAG
    USB_IF_PROTOCOL_PCUI,    //PCUI
    USB_IF_PROTOCOL_GPS,     //GPS

    USB_IF_PROTOCOL_COMM_B,
};

static USB_UINT8 gThirdConfigPortDefault[DYNAMIC_PID_MAX_PORT_NUM] =
{
    USB_IF_PROTOCOL_MBIM,    //MBIM
    USB_IF_PROTOCOL_GPS,     //GPS
};

static USB_UINT8 g_FirstConfigPortDebug[DYNAMIC_PID_MAX_PORT_NUM] =
{
    USB_IF_PROTOCOL_MODEM,   //MODEM
    USB_IF_PROTOCOL_DIAG,    //DIAG
    USB_IF_PROTOCOL_PCUI,    //PCUI
    USB_IF_PROTOCOL_NCM,     //NCM
    USB_IF_PROTOCOL_GPS,     //GPS
#ifdef MBB_USB_UNITARY_Q
    USB_IF_PROTOCOL_ADB,     //ADB
#else
    USB_IF_PROTOCOL_COMM_B,
    USB_IF_PROTOCOL_COMM_C,
    USB_IF_PROTOCOL_SHEEL_A,
    USB_IF_PROTOCOL_SHEEL_B,
#endif/*MBB_USB_UNITARY_Q*/
};
/*QC Debug mode is same as Default*/
static USB_UINT8 g_SecondConfigPortDebug[DYNAMIC_PID_MAX_PORT_NUM] =
{
    /*ECM 放在第一位，linux系统自动选择该config*/
    USB_IF_PROTOCOL_RNDIS,   //linux 使用时pnp自动更新为ECM
    USB_IF_PROTOCOL_MODEM,   //MODEM
    USB_IF_PROTOCOL_DIAG,    //DIAG
    USB_IF_PROTOCOL_PCUI,    //PCUI
    USB_IF_PROTOCOL_GPS,     //GPS
    USB_IF_PROTOCOL_COMM_B,
};
static USB_UINT8 g_ThirdConfigPortDebug[DYNAMIC_PID_MAX_PORT_NUM] =
{
    USB_IF_PROTOCOL_MBIM,    //MBIM
    USB_IF_PROTOCOL_GPS,     //GPS
    USB_IF_PROTOCOL_PCUI,    //PCUI
    USB_IF_PROTOCOL_DIAG,    //DIAG
#ifdef MBB_USB_UNITARY_Q
    USB_IF_PROTOCOL_ADB,     //ADB
#else
    USB_IF_PROTOCOL_COMM_B,
#endif/*MBB_USB_UNITARY_Q*/
};

static huawei_port_string cust_device_name[] =
{
    {
        PRODUCT_ID_P711SOLUTION,
        {
            "HUAWEI Mobile",
            "Huawei Mobile Connect - Modem",
            "Huawei Mobile Connect - Application",
            "Huawei Mobile Connect - Pcui",
            "Huawei Mobile Connect - Gps",
            "Huawei Mobile Connect - Ctrl",
            "Huawei Mobile Connect - Serial B",
            "HUAWEI Mobile Connect - Network Card"
        }
    },
    {
        DYNAMIC_HP_PID,
        {
            "HP Mobile",
            "HP Mobile Connect - Modem",
            "HP Mobile Connect - Application",
            "HP Mobile Connect - Pcui",
            "HP Mobile Connect - Gps",
            "HP Mobile Connect - Ctrl",
            "HP Mobile Connect - Serial B",
            "HP Mobile Connect - Network Card"
        }
    },
    {0, {""}}
};

static USB_UINT8 gUsbModeLogLever[MODE_NUM] =
{
    U_ERROR,
};

/*****************************************************************
Parameters    :  无
Return        :    无
Description   :  获取 shell_lock 值
*****************************************************************/
#ifdef MBB_USB_UNITARY_Q
#else
USB_INT get_shell_lock(USB_VOID)
{
    return g_usb_nv_info.shell_lock;
}
#endif
/*****************************************************************
Parameters    :  无
Return        :    无
Description   :  初始化USB 各个模块的log级别
*****************************************************************/
USB_VOID usb_mode_log_lever_init(USB_VOID)
{
    USB_INT i = 0;
    for ( ; i < MODE_NUM ; i++)
    {
        gUsbModeLogLever[i] = U_ERROR;
    }
}

/*****************************************************************
Parameters    :  index
Return        :  huawei_port_string* 
Description   :  获取index对应的device_name
*****************************************************************/
huawei_port_string* usb_device_desc_string_get(USB_INT index)
{
    if((index >= 0)&& (index < (sizeof(cust_device_name) / sizeof(huawei_port_string))))
    {
        return &(cust_device_name[index]);
    }
    else
    {
        return NULL;
    }
}
/*****************************************************************
Parameters    : USB_VOID
Return        : 查询到返回 PID 对应的index，否则返回-1
Description   : 取PID 对应的index值
*****************************************************************/
USB_INT device_desc_get_idx(USB_VOID)
{
    USB_INT index = 0;
    for(index = 0; cust_device_name[index].cust_pid; index++)
    {
        if (g_usb_nv_info.pid_info.nv_status)
        {
            if(cust_device_name[index].cust_pid == g_usb_nv_info.pid_info.cust_first_pid)
            {
                return index;
            }
        }
        if (g_usb_nv_info.dynamic_pid_info.nv_status)
        {
            if (cust_device_name[index].cust_pid == g_usb_nv_info.dynamic_pid_info.dynamic_normal_pid)
            {
                return index;
            }
        }
    }
    return -1;
}
#ifdef USB_SOLUTION
/*****************************************************************
Parameters    : 取CE和M2M分别对应的端口形态
Return        :    无
Description   : 取CE和M2M分别对应的端口形态
*****************************************************************/
USB_VOID usb_get_multiconfig_port()
{
    USB_INT index = 0;
    if (MDRV_VER_TYPE_M2M == mdrv_ver_get_solution_type())
    {
       for (index = 0; index < DYNAMIC_PID_MAX_PORT_NUM; index++)
       {
            if (USB_IF_PROTOCOL_GPS == g_ThirdConfigPortDebug[index])
            {
                g_ThirdConfigPortDebug[index] = USB_IF_PROTOCOL_CTRL;
            }
            if (USB_IF_PROTOCOL_GPS == g_SecondConfigPortDebug[index])
            {
                g_SecondConfigPortDebug[index] = USB_IF_PROTOCOL_CTRL;
            }
            if (USB_IF_PROTOCOL_GPS == g_FirstConfigPortDebug[index])
            {
                g_FirstConfigPortDebug[index] = USB_IF_PROTOCOL_CTRL;
            }
       }
    }
    return;
}
#endif

/*模块功能使用的转译函数*/
USB_VOID usb_nv_translation_protocol (USB_UINT8* protocol_info)
{
    USB_INT i;
    USB_UINT8 old_protocol = USB_IF_PROTOCOL_VOID;
    
    if( NULL == protocol_info)
    {
        DBG_I(MBB_USB_NV , "protocol_info NULL!\n");
        return;
    }
    
    for (i = 0; i < (sizeof(translation_table) / sizeof(protocol_translation_info)); i++)
    {
        old_protocol = translation_table[i].old_standard;

        if ( old_protocol == protocol_info[i] )
        {
            protocol_info[i] = translation_table[i].new_standard;
        }
    }
}


/*****************************************************************
Parameters    :  无
Return        :    0  MBB_USB_OK
Description   :  读取   所有 USB NV 项
*****************************************************************/
USB_INT  usb_nv_get(USB_VOID)
{
    USB_INT ret= MBB_USB_ERROR;
    nv_huawei_dynamic_name_realy nv_tmp ;
    char *p_tmp = NULL; /*临时指针*/
    USB_INT index = 0;
    int i = 0;
    DBG_I(MBB_USB_NV , "begin!\n");
#ifdef MBB_USB_UNITARY_Q
    /*USB_NV_SERIAL_NUM_ID  NV 50049 使用时不用判断nv_status*/
    memset(&(g_usb_nv_info.stUSBNvSnSupp), 0, sizeof(g_usb_nv_info.stUSBNvSnSupp));
    ret = usb_read_nv(USB_NV_SERIAL_NUM_ID, (USB_PVOID) & (g_usb_nv_info.stUSBNvSnSupp),
                      sizeof(g_usb_nv_info.stUSBNvSnSupp));
    if ((MBB_USB_OK != ret) 
        || (MBB_NV_DISABLED == g_usb_nv_info.stUSBNvSnSupp.nv_status) )
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%d \r\n", USB_NV_SERIAL_NUM_ID, ret,
              g_usb_nv_info.stUSBNvSnSupp.nv_status);
        g_usb_nv_info.stUSBNvSnSupp.sn_enable = MBB_USB_FALSE;
    }
#else

    /*USB_NV_SERIAL_NUM_ID  NV 6 无nv_status*/
    memset(g_usb_nv_info.serial_num.aucSerialNumber, 0, sizeof(nv_huawei_device_serial_num));
    /*只读取16位SN，防止NV中16位以后存的不是字符串结束符*/
    ret = usb_read_nv(USB_NV_SERIAL_NUM, g_usb_nv_info.serial_num.aucSerialNumber,
                      USB_NUM_16);
    if ( MBB_USB_OK != ret )
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d \r\n", 
            USB_NV_SERIAL_NUM, ret);
        memcpy((USB_PVOID)(g_usb_nv_info.serial_num.aucSerialNumber), 
            (USB_PVOID)(SERIAL_NUM_DEFAULT), sizeof(SERIAL_NUM_DEFAULT));
    }

    /*USB_NV_SERIAL_NUM_ID  NV 26 使用时需要判断nv_status*/
    memset(&(g_usb_nv_info.stUSBNvSnSupp), 0, sizeof(g_usb_nv_info.stUSBNvSnSupp));
    ret = usb_read_nv(USB_NV_SERIAL_NUM_ID, (USB_PVOID) & (g_usb_nv_info.stUSBNvSnSupp),
                      sizeof(g_usb_nv_info.stUSBNvSnSupp));
    if ((MBB_USB_OK != ret) 
        || (MBB_NV_DISABLED == g_usb_nv_info.stUSBNvSnSupp.usbSnNvStatus) )
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%d \r\n", 
            USB_NV_SERIAL_NUM_ID, ret,g_usb_nv_info.stUSBNvSnSupp.usbSnNvStatus);
        g_usb_nv_info.stUSBNvSnSupp.usbSnNvStatus = MBB_NV_DISABLED;
        g_usb_nv_info.stUSBNvSnSupp.usbSnNvSucFlag = 0;
    }
    /*USB_NV_ITEM_AT_SHELL_OPEN NV 33   无nv_status */
    memset(&(g_usb_nv_info.shell_lock), 0, sizeof(g_usb_nv_info.shell_lock));
    ret = usb_read_nv(USB_NV_ITEM_AT_SHELL_OPEN, (USB_PVOID) & (g_usb_nv_info.shell_lock),
                      sizeof(USB_UINT32));
    if (MBB_USB_OK != ret)
    {
        DBG_T(MBB_USB_NV, "NV %d use default!\r\n", USB_NV_ITEM_AT_SHELL_OPEN);
        g_usb_nv_info.shell_lock = 0;
    }

    /*USB_NV_FACTORY_MODE_I  NV 36 无nv_status */
    memset(&(g_usb_nv_info.factory_mode), 0, sizeof(g_usb_nv_info.factory_mode));
    ret = usb_read_nv(USB_NV_FACTORY_MODE_I, (USB_PVOID) & (g_usb_nv_info.factory_mode),
                      sizeof(USB_UINT32));
    if (MBB_USB_OK != ret)
    {
        DBG_T(MBB_USB_NV , "NV %d use default!\r\n", USB_NV_FACTORY_MODE_I);
        g_usb_nv_info.factory_mode = 1;
    }

    /*USB_NV_SD_WORKMODE NV 51 无nv_status */
    memset(&(g_usb_nv_info.ulSDWorkMode), 0, sizeof(g_usb_nv_info.ulSDWorkMode));
    ret = usb_read_nv(USB_NV_SD_WORKMODE, 
    &(g_usb_nv_info.ulSDWorkMode), sizeof(g_usb_nv_info.ulSDWorkMode));
    if (MBB_USB_OK != ret)
    {
        DBG_T(MBB_USB_NV , "NV %d use default!\r\n", USB_NV_SD_WORKMODE);
        g_usb_nv_info.ulSDWorkMode = SD_WORKMODE_MASS;
    }
#endif
    /*USB_NV_PID_UNIFICATION_ID NV 50071 使用时不用判断nv_status*/
    memset(&(g_usb_nv_info.pid_info), 0, sizeof(nv_cust_pid_type));
    ret = usb_read_nv(USB_NV_PID_UNIFICATION_ID, 
        (USB_PVOID) & (g_usb_nv_info.pid_info),sizeof(nv_cust_pid_type));
    if ((MBB_USB_OK != ret) 
        || (MBB_NV_DISABLED == g_usb_nv_info.pid_info.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%d\r\n", 
            USB_NV_PID_UNIFICATION_ID, ret,g_usb_nv_info.pid_info.nv_status);
        memcpy((USB_PVOID)(&(g_usb_nv_info.pid_info)), 
            (USB_PVOID)(&gstDevPidDefault), sizeof(nv_cust_pid_type));
    }

    /*USB_NV_PORT_INFO_ID NV 50091 使用时不用判断nv_status*/
    memset(&(g_usb_nv_info.dev_profile_info), 0, sizeof(nv_huawei_dynamic_pid_type));
    ret = usb_read_nv(USB_NV_PORT_INFO_ID, (USB_PVOID) & (g_usb_nv_info.dev_profile_info),
                      sizeof(nv_huawei_dynamic_pid_type));
    if ((MBB_USB_OK != ret) 
        ||(MBB_NV_DISABLED == g_usb_nv_info.dev_profile_info.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%d\r\n", 
            USB_NV_PORT_INFO_ID, ret,g_usb_nv_info.dev_profile_info.nv_status);
        g_usb_nv_info.dev_profile_info.first_port_style[0] = USB_IF_PROTOCOL_CDROM;
        memcpy(g_usb_nv_info.dev_profile_info.rewind_port_style, gRewindPortDefault,
               DYNAMIC_PID_MAX_PORT_NUM);
    }
    /*进行防呆处理，避免写NV时未写入切换后的端口形态导致单板处于无端口的尴尬局面*/
    else
    {
        if( USB_IF_PROTOCOL_VOID == g_usb_nv_info.dev_profile_info.rewind_port_style[0])
        {
            DBG_E(MBB_USB_NV , "please enter the NV value! use default\n");
            memcpy(g_usb_nv_info.dev_profile_info.rewind_port_style, gRewindPortDefault,
                   DYNAMIC_PID_MAX_PORT_NUM);
        }
    }
    /*老化测试等场景下不能读NV，并且测试需要直接上报全端口，所以在此处直接修改*/
#ifdef USB_DEBUG_PORT
    g_usb_nv_info.dev_profile_info.first_port_style[0] = USB_IF_PROTOCOL_NOPNP;
    DBG_T(MBB_USB_NV , "USB_DEBUG_PORT port mode\n");
#endif

    /*USB_NV_MASS_DYNAMIC_NAME NV 50108 使用时不用判断nv_status*/
    memset(&nv_tmp,0,sizeof(nv_huawei_dynamic_name_realy));
    memset(&(g_usb_nv_info.mass_dynamic_name), 0, sizeof(nv_huawei_dynamic_name));
    /*nv_tmp用来读取真正的nv值，读取成功后转存到大空间结构体防止%-4s越界*/
    ret = usb_read_nv(USB_NV_MASS_DYNAMIC_NAME, (USB_PVOID)(&nv_tmp),
                      sizeof(nv_huawei_dynamic_name_realy));
    if ((MBB_USB_OK != ret) 
        || (MBB_NV_DISABLED == nv_tmp.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%d\r\n", 
            USB_NV_MASS_DYNAMIC_NAME, ret,g_usb_nv_info.mass_dynamic_name.nv_status);
        snprintf(g_usb_nv_info.mass_dynamic_name.huawei_cdrom_dynamic_name, DYNAMIC_CD_NAME_CHAR_NUM + 1,
                 "%-8s%-16s%-4s", "HUAWEI", "Mass Storage", "2.31");
        snprintf(g_usb_nv_info.mass_dynamic_name.huawei_sd_dynamic_name, DYNAMIC_CD_NAME_CHAR_NUM + 1,
                 "%-8s%-16s%-4s", "HUAWEI", "TF CARD Storage", "2.31");
    }
    else
    {

        g_usb_nv_info.mass_dynamic_name.nv_status = nv_tmp.nv_status;
        p_tmp = g_usb_nv_info.mass_dynamic_name.huawei_cdrom_dynamic_name;
        memcpy(p_tmp, nv_tmp.huawei_cdrom_dynamic_name, DYNAMIC_CD_NAME_CHAR_NUM);
        p_tmp = g_usb_nv_info.mass_dynamic_name.huawei_sd_dynamic_name;
        memcpy(p_tmp, nv_tmp.huawei_sd_dynamic_name, DYNAMIC_CD_NAME_CHAR_NUM);
    }
    /*USB_NV_DYNAMIC_INFO_NAME NV 50109 使用时不用判断nv_status*/
    memset(&(g_usb_nv_info.port_dynamic_name), 0, sizeof(nv_huawei_dynamic_info_name));
    ret = usb_read_nv(USB_NV_DYNAMIC_INFO_NAME, 
        (USB_PVOID) & (g_usb_nv_info.port_dynamic_name),
        sizeof(nv_huawei_dynamic_info_name));
    if ((MBB_USB_OK != ret) 
        ||( MBB_NV_DISABLED == g_usb_nv_info.port_dynamic_name.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%d\r\n", 
            USB_NV_DYNAMIC_INFO_NAME, ret,g_usb_nv_info.port_dynamic_name.nv_status);
        strncpy(g_usb_nv_info.port_dynamic_name.huawei_manufacturer_dynamic_name, 
            "HUAWEI_MOBILE",(DYNAMIC_INFO_NAME_CHAR_NUM - 1));
        strncpy(g_usb_nv_info.port_dynamic_name.huawei_product_dynamic_name, 
            "HUAWEI_MOBILE",(DYNAMIC_INFO_NAME_CHAR_NUM - 1));
    }
    /*USB_NV_PROT_OFFSET_ID  NV 50110 废弃*/
    memset( &(g_usb_nv_info.stDevProtOfsInfo), 0, sizeof(nv_protocol_base_type));
    ret = usb_read_nv(USB_NV_PROT_OFFSET_ID, 
        (USB_PVOID) & (g_usb_nv_info.stDevProtOfsInfo),sizeof(nv_protocol_base_type));
    if ((MBB_USB_OK != ret)
        || (MBB_NV_DISABLED == g_usb_nv_info.stDevProtOfsInfo.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%d\r\n",
            USB_NV_PROT_OFFSET_ID, ret,g_usb_nv_info.stDevProtOfsInfo.nv_status);
        memcpy((USB_PVOID) & (g_usb_nv_info.stDevProtOfsInfo), 
            (USB_PVOID) & (gstDevProtOfsDefault), sizeof(nv_protocol_base_type));
    }
    else if (g_usb_nv_info.stDevProtOfsInfo.protocol_base != 0)
    {
        DBG_T(MBB_USB_NV , "Now not support  NV %d! \r\n", 
            USB_NV_PROT_OFFSET_ID);
        DBG_T(MBB_USB_NV , "NV_%d invalid !protocol_base = %d!!\r\n", 
            USB_NV_PROT_OFFSET_ID,g_usb_nv_info.stDevProtOfsInfo.protocol_base);
    }
#ifdef MBB_USB_UNITARY_Q
    /*USB_NV_FEATURE_WWAN NV 50151 使用时不用判断nv_status*/
    memset( &(g_usb_nv_info.feature_wwan), 0, sizeof(g_usb_nv_info.feature_wwan));
    ret = usb_read_nv(USB_NV_FEATURE_WWAN, &(g_usb_nv_info.feature_wwan),
                      sizeof(g_usb_nv_info.feature_wwan));
    if ((MBB_USB_OK != ret)
        || (MBB_NV_DISABLED == g_usb_nv_info.feature_wwan.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%d\r\n",
              USB_NV_FEATURE_WWAN, ret, g_usb_nv_info.feature_wwan.nv_status);
#if defined(USB_E5) || defined(USB_RNDIS)
        g_usb_nv_info.feature_wwan.wwan_work_flag = MBB_USB_FALSE;
#else
        g_usb_nv_info.feature_wwan.wwan_work_flag = MBB_USB_TRUE;
#endif
    }

    /*USB_NV_HILINK_MODEM_MODE 50367 使用时不用判断nv_status*/
    memset( &(g_usb_nv_info.hilink_mode_flag), 0, sizeof(nv_huawei_hilink_modem_mode));
    ret = usb_read_nv(USB_NV_HILINK_MODEM_MODE,
                      &(g_usb_nv_info.hilink_mode_flag), sizeof(nv_huawei_hilink_modem_mode));
    if ((MBB_USB_OK != ret) 
        || (MBB_NV_DISABLED == g_usb_nv_info.hilink_mode_flag.nv_status))
    {
        DBG_T(MBB_USB_NV, "NV %d use default, ret=%d nv_status=%d\r\n", 
            USB_NV_HILINK_MODEM_MODE, ret,g_usb_nv_info.hilink_mode_flag.nv_status);
        g_usb_nv_info.hilink_mode_flag.mode_flag = HILINK_RNDIS_MODE;
    }
#endif
    /*USB_NV_HUAWEI_DYNAMIC_VID NV 50394 使用时不用判断nv_status*/
    memset( &(g_usb_nv_info.dynamic_vid_info), 0, sizeof(nv_huawei_dynamic_vid_type));
    ret = usb_read_nv(USB_NV_HUAWEI_DYNAMIC_VID, &(g_usb_nv_info.dynamic_vid_info),
                      sizeof(nv_huawei_dynamic_vid_type));
    if ((MBB_USB_OK != ret)
        || (MBB_NV_DISABLED == g_usb_nv_info.dynamic_vid_info.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%d\r\n", 
            USB_NV_HUAWEI_DYNAMIC_VID,ret, g_usb_nv_info.dynamic_vid_info.nv_status);
        g_usb_nv_info.dynamic_vid_info.dynamic_vid = VENDOR_ID_HUAWEI;
    }

    /*USB_NV_HUAWEI_DYNAMIC_BOOT_PID NV 50395  使用时需要判断nv_status*/
    memset( &(g_usb_nv_info.boot_pid_info), 0, sizeof(nv_huawei_dynamic_boot_pid_type));
    ret = usb_read_nv(USB_NV_HUAWEI_DYNAMIC_BOOT_PID, 
    &(g_usb_nv_info.boot_pid_info),sizeof(nv_huawei_dynamic_boot_pid_type));
    if ((MBB_USB_OK != ret)
        || (MBB_NV_DISABLED == g_usb_nv_info.boot_pid_info.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%d\r\n", 
            USB_NV_HUAWEI_DYNAMIC_BOOT_PID,ret, g_usb_nv_info.boot_pid_info.nv_status);
        g_usb_nv_info.boot_pid_info.dynamic_boot_pid = MBIM_DOWNLOAD_PID;
    }

    /*USB_NV_HUAWEI_DYNAMIC_NORMAL_PID NV 50396  使用时需要判断nv_status*/
    memset( &(g_usb_nv_info.dynamic_pid_info), 0, sizeof(nv_huawei_dynamic_normal_pid_type));
    ret = usb_read_nv(USB_NV_HUAWEI_DYNAMIC_NORMAL_PID, 
    &(g_usb_nv_info.dynamic_pid_info),sizeof(nv_huawei_dynamic_normal_pid_type));
    if ((MBB_USB_OK != ret)
        || (MBB_NV_DISABLED == g_usb_nv_info.dynamic_pid_info.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%d\r\n", 
            USB_NV_HUAWEI_DYNAMIC_NORMAL_PID,
            ret, g_usb_nv_info.dynamic_pid_info.nv_status);
        g_usb_nv_info.dynamic_pid_info.dynamic_normal_pid = DYNAMIC_MODULE_PID;
    }

    /*USB_NV_HUAWEI_DYNAMIC_DEBUG_PID NV 50397  使用时需要判断nv_status*/
    memset( &(g_usb_nv_info.debug_pid_info), 0, sizeof(nv_huawei_dynamic_debug_pid_type));
    ret = usb_read_nv(USB_NV_HUAWEI_DYNAMIC_DEBUG_PID, 
    &(g_usb_nv_info.debug_pid_info),sizeof(nv_huawei_dynamic_debug_pid_type));
    if ((MBB_USB_OK != ret)
        || (MBB_NV_DISABLED == g_usb_nv_info.debug_pid_info.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%d\r\n", 
            USB_NV_HUAWEI_DYNAMIC_DEBUG_PID,ret, g_usb_nv_info.debug_pid_info.nv_status);
        g_usb_nv_info.debug_pid_info.dynamic_debug_pid = DYNAMIC_DEBUG_PID;
    }

    /*NV_WINBLUE_PRF NV50424  使用时需要判断MBIMEnable*/
    memset(&(g_usb_nv_info.winblue_profile), 0, sizeof(nv_winblue_profile_type));
    ret = usb_read_nv(USB_NV_WINBLUE_PRF_ID,
                      &(g_usb_nv_info.winblue_profile), sizeof(nv_winblue_profile_type));
    if (MBB_USB_OK != ret)
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d \r\n",
              USB_NV_WINBLUE_PRF_ID, ret);
        g_usb_nv_info.winblue_profile.MBIMEnable = 0;
    }
    else
    {
        DBG_T(MBB_USB_NV, "Winblue profile nv val: %s, %u, %u, %u, %u, %u, %u\n",
              g_usb_nv_info.winblue_profile.InterfaceName,
              g_usb_nv_info.winblue_profile.MBIMEnable,
              g_usb_nv_info.winblue_profile.CdRom,
              g_usb_nv_info.winblue_profile.TCard,
              g_usb_nv_info.winblue_profile.MaxPDPSession,
              g_usb_nv_info.winblue_profile.IPV4MTU,
              g_usb_nv_info.winblue_profile.IPV6MTU);
    }

    /*net speed NV 50456 使用时不用判断nv_status*/
    memset(&(g_usb_nv_info.net_speed_info), 0,
           sizeof(usb_cdc_net_speed_type));
    ret = usb_read_nv(USB_NV_NET_SPEED_ID,
                      &(g_usb_nv_info.net_speed_info), sizeof(usb_cdc_net_speed_type));
    if ((MBB_USB_OK != ret)
        || (MBB_NV_DISABLED == g_usb_nv_info.net_speed_info.nv_status))
    {
        DBG_T(MBB_USB_NV, "NV %d use default, ret=%d nv_status=%ld\r\n", 
            USB_NV_NET_SPEED_ID, ret,g_usb_nv_info.net_speed_info.nv_status);
        g_usb_nv_info.net_speed_info.net_speed = 0;
    }
    /*USB_NV_USB_TETHERING_ID*/
    memset(&(g_usb_nv_info.usb_tethering_flag),0,sizeof(HUAWEI_NV_USB_TETHERING));
    ret = usb_read_nv(USB_NV_USB_TETHERING_ID,(USB_PVOID) &(g_usb_nv_info.usb_tethering_flag), 
     sizeof(HUAWEI_NV_USB_TETHERING));
    if ((MBB_USB_OK != ret) || (MBB_USB_FALSE == g_usb_nv_info.usb_tethering_flag.nv_status))
    {
        DBG_E(MBB_USB_NV , "%s:  NV = %d, ret=%d nv_status=%d\r\n", __func__, USB_NV_USB_TETHERING_ID,ret,
            g_usb_nv_info.usb_tethering_flag.nv_status);
    }
    else
    {
        if(0 == g_usb_nv_info.usb_tethering_flag.ucActiveFlag)
        {
            int i = 0;
            int j = 0;
            int find = 0;
            for(i;i < DYNAMIC_PID_MAX_PORT_NUM;i++)
            {
                if((USB_IF_PROTOCOL_NCM == g_usb_nv_info.dev_profile_info.rewind_port_style[i])
                    || (VDF_USB_IF_PROTOCOL_NCM == g_usb_nv_info.dev_profile_info.rewind_port_style[i])
                    || (NO_HW_USB_IF_PROTOCOL_NCM == g_usb_nv_info.dev_profile_info.rewind_port_style[i]))
                {
                    find = 1;
                    j = i;
                    break;
                }
            }
            if(1 == find)
            {
                for(j;j < DYNAMIC_PID_MAX_PORT_NUM;j++)
                {
                    if((DYNAMIC_PID_MAX_PORT_NUM - 1) == j)
                    {
                        g_usb_nv_info.dev_profile_info.rewind_port_style[j] = 0;
                    }
                    else
                    {
                        g_usb_nv_info.dev_profile_info.rewind_port_style[j] = g_usb_nv_info.dev_profile_info.rewind_port_style[j + 1];
                    }
                }
            }
        }
    }

    /*USB_NV_PORT_INFO_ID NV 50457  使用时不用判断nv_status*/
    /* nv_status 用作判断是否模块产品*/
    /* 模块产品必须支持该NV，并且在xml文件中写入默认值*/
    memset(&(g_usb_nv_info.multi_config_port_info), 0, 
    sizeof(nv_huawei_multi_config_usb_port_type));
    ret = usb_read_nv(USB_NV_MULTI_CONFIG_PORT_INFO, 
        (USB_PVOID) & (g_usb_nv_info.multi_config_port_info),
        sizeof(nv_huawei_multi_config_usb_port_type));
    if ((MBB_USB_OK != ret)
        || (MBB_NV_DISABLED == g_usb_nv_info.multi_config_port_info.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%ld\r\n", 
            USB_NV_MULTI_CONFIG_PORT_INFO, ret,
            g_usb_nv_info.multi_config_port_info.nv_status);

        g_usb_nv_info.multi_config_port_info.nv_status = MBB_NV_DISABLED;
        memcpy(g_usb_nv_info.multi_config_port_info.first_config_port, 
            gFirstConfigPortDefault,DYNAMIC_PID_MAX_PORT_NUM);
        memcpy(g_usb_nv_info.multi_config_port_info.second_config_port, 
            gSecondConfigPortDefault,DYNAMIC_PID_MAX_PORT_NUM);
        memcpy(g_usb_nv_info.multi_config_port_info.third_config_port, 
            gThirdConfigPortDefault,DYNAMIC_PID_MAX_PORT_NUM);
    }
    else/*NV get success*/
    {
        usb_nv_translation_protocol(g_usb_nv_info.multi_config_port_info.first_config_port);
        usb_nv_translation_protocol(g_usb_nv_info.multi_config_port_info.second_config_port);
        usb_nv_translation_protocol(g_usb_nv_info.multi_config_port_info.third_config_port);
    }

    /*USB_NV_USB_PRIVATE_INFO NV 50498  使用时不用判断nv_status*/
    memset(&(g_usb_nv_info.usb_log_ctl), 0, 
    sizeof(usb_private_nv_info));
    ret = usb_read_nv(USB_NV_USB_PRIVATE_INFO, 
    &(g_usb_nv_info.usb_log_ctl), sizeof(usb_private_nv_info));
    if ((MBB_USB_OK != ret)
        || (MBB_NV_DISABLED == g_usb_nv_info.usb_log_ctl.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%d\r\n",
              USB_NV_USB_PRIVATE_INFO, ret, g_usb_nv_info.usb_log_ctl.nv_status);
        memcpy(&g_usb_nv_info.usb_log_ctl.debug_mode, 
            gUsbModeLogLever, sizeof(gUsbModeLogLever));
    }
    else
    {
        /*g_usb_nv_info.usb_log_ctl.debug_mode[i] 下标大于debug_bank[]下标，*/
        /*for循环按照debug_bank[]下标计算*/
        for ( i = 2; i < MBB_LAST; i++)
        {
            DBG_SET_LEVEL(i, g_usb_nv_info.usb_log_ctl.debug_mode[i]);
        }
    }
    /*USB_NV_USB_DEBUG_MODE_FLAGE NV 50538  使用时需要判断nv_status*/
    memset( &(g_usb_nv_info.debug_mode), 0, sizeof(nv_huawei_debug_mode_flag));
    ret = usb_read_nv(USB_NV_USB_DEBUG_MODE_FLAGE, &(g_usb_nv_info.debug_mode),
                      sizeof(nv_huawei_debug_mode_flag));
    if ((MBB_USB_OK != ret)
        ||( MBB_NV_DISABLED == g_usb_nv_info.debug_mode.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%ld\r\n", USB_NV_USB_DEBUG_MODE_FLAGE, ret,
              g_usb_nv_info.debug_mode.nv_status);
        g_usb_nv_info.debug_mode.nv_status = MBB_NV_DISABLED;
        g_usb_nv_info.debug_mode.setmode_flag = NCM_MBIM_MODE_NORMAL;
    }
#ifdef USB_SOLUTION
    usb_get_multiconfig_port();
#endif
    if (NCM_MBIM_MODE_DEBUG == g_usb_nv_info.debug_mode.setmode_flag)
    {
        memcpy(g_usb_nv_info.multi_config_port_info.third_config_port, g_ThirdConfigPortDebug,
               DYNAMIC_PID_MAX_PORT_NUM);
    }
    else if (ECM_LINUX_MODE_DEBUG == g_usb_nv_info.debug_mode.setmode_flag)
    {
        memcpy(g_usb_nv_info.multi_config_port_info.second_config_port, g_SecondConfigPortDebug,
               DYNAMIC_PID_MAX_PORT_NUM);
    }
    else if (ACM_PCUI_MODE_DEBUG == g_usb_nv_info.debug_mode.setmode_flag)
    {
        memcpy(g_usb_nv_info.multi_config_port_info.first_config_port, g_FirstConfigPortDebug,
               DYNAMIC_PID_MAX_PORT_NUM);
    }
    index = device_desc_get_idx();

    if ( index >= 0 )
    {
        strncpy(g_usb_nv_info.port_dynamic_name.huawei_manufacturer_dynamic_name,
                cust_device_name[index].port_name[0], (DYNAMIC_INFO_NAME_CHAR_NUM - 1));
        strncpy(g_usb_nv_info.port_dynamic_name.huawei_product_dynamic_name,
                cust_device_name[index].port_name[0], (DYNAMIC_INFO_NAME_CHAR_NUM - 1));
    }

    /*USB_NV_SECURITY_FLAG 50577 使用时需要判断nv_status*/
    memset( &(g_usb_nv_info.usb_security_flag), 0, sizeof(nv_huawei_usb_security_flag));
    ret = usb_read_nv(USB_NV_SECURITY_FLAG, &(g_usb_nv_info.usb_security_flag),
                      sizeof(nv_huawei_usb_security_flag));
    if ((MBB_USB_OK != ret)
        ||( MBB_NV_DISABLED == g_usb_nv_info.usb_security_flag.nv_status))
    {
        DBG_T(MBB_USB_NV , "NV %d use default, ret=%d nv_status=%ld\r\n", USB_NV_SECURITY_FLAG, ret,
              g_usb_nv_info.usb_security_flag.nv_status);
        g_usb_nv_info.usb_security_flag.nv_status = 0;
        g_usb_nv_info.usb_security_flag.diag_enable = 0;
        g_usb_nv_info.usb_security_flag.shell_enable = 0;
        g_usb_nv_info.usb_security_flag.adb_enable = 0;
        g_usb_nv_info.usb_security_flag.cbt_enable = 0;
    }

    return MBB_USB_OK;
}
EXPORT_SYMBOL_GPL(usb_nv_get);


/*****************************************************************
Parameters    :  usb_nv 结构体指针
Return        :    无
Description   :  对外输出 USB NV  相关内容
*****************************************************************/

mbb_usb_nv_info_st*  usb_nv_get_ctx(USB_VOID)
{
    return &g_usb_nv_info;
}
EXPORT_SYMBOL_GPL(usb_nv_get_ctx);

USB_UINT usb_is_m2m_multi_config(USB_VOID)
{
    mbb_usb_nv_info_st* nv_ctx = NULL;

    nv_ctx = usb_nv_get_ctx();

    return (nv_ctx->multi_config_port_info.nv_status);

}

USB_UINT usb_get_mbim_config(USB_VOID)
{
    mbb_usb_nv_info_st* nv_ctx = NULL;

    nv_ctx = usb_nv_get_ctx();

    return (nv_ctx->multi_config_port_info.nv_status ? USB_NUM_3 : USB_NUM_2);
}

/*****************************************************************
Parameters    :  无
Return        :    无
Description   :  USB NV 初始化为  NV  读取失败的默认值
*****************************************************************/
USB_VOID usb_nv_init(USB_VOID)
{
    /* NV 50091对应内容的 默认值*/
    g_usb_nv_info.dev_profile_info.nv_status = MBB_NV_ENABLE;
    g_usb_nv_info.dev_profile_info.first_port_style[0] = USB_IF_PROTOCOL_CDROM;
    g_usb_nv_info.dev_profile_info.first_port_style[1] = USB_IF_PROTOCOL_SDRAM;
    memcpy(g_usb_nv_info.dev_profile_info.rewind_port_style, gRewindPortDefault,
           DYNAMIC_PID_MAX_PORT_NUM);
#ifdef MBB_USB_UNITARY_Q
    g_usb_nv_info.stUSBNvSnSupp.nv_status = MBB_NV_DISABLED;
    g_usb_nv_info.stUSBNvSnSupp.sn_enable = MBB_USB_FALSE;
#else
    /* NV 26对应内容的 默认值*/
    g_usb_nv_info.stUSBNvSnSupp.usbSnNvStatus = 0;
    g_usb_nv_info.stUSBNvSnSupp.usbSnNvSucFlag = 0;
    g_usb_nv_info.shell_lock = 0;
#endif
    memcpy((USB_PVOID) & (g_usb_nv_info.stDevProtOfsInfo), (USB_PVOID) & (gstDevProtOfsDefault), sizeof(nv_protocol_base_type));
    g_usb_nv_info.mass_dynamic_name.nv_status = MBB_NV_ENABLE;
    g_usb_nv_info.ulSDWorkMode = SD_WORKMODE_MASS;
    g_usb_nv_info.usb_tethering_flag.ucActiveFlag = 1;
    usb_mode_log_lever_init();

    usb_nv_get();
    
}

/*USB_NV 可维可测函数*/
USB_VOID usb_nv_dump(USB_VOID)
{
    USB_INT i = 0;
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
#ifdef MBB_USB_UNITARY_Q
    /*50049*/
    DBG_T(MBB_DEBUG, "|NV_50049: \n");
    DBG_T(MBB_DEBUG, "|nv_status = %d\n" , g_usb_nv_info.stUSBNvSnSupp.nv_status);
    DBG_T(MBB_DEBUG, "|u32USBSerialNumSupp = %d\n",
          g_usb_nv_info.stUSBNvSnSupp.sn_enable);
#else
    /*6*/
    DBG_T(MBB_DEBUG, "|NV_6: \n");
    DBG_T(MBB_DEBUG, "|serial_num = %20s\n",g_usb_nv_info.serial_num.aucSerialNumber);
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*26*/
    DBG_T(MBB_DEBUG, "|NV_26: \n");
    DBG_T(MBB_DEBUG , "|nv_status = %d\n" , g_usb_nv_info.stUSBNvSnSupp.usbSnNvStatus);
    DBG_T(MBB_DEBUG, "|u32USBSerialNumSupp = %d\n",
          g_usb_nv_info.stUSBNvSnSupp.usbSnNvSucFlag);
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*33*/
    DBG_T(MBB_DEBUG, "|NV_33:\n");
    DBG_T(MBB_DEBUG, "|shell_lock = %d \n", g_usb_nv_info.shell_lock );
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*36*/
    DBG_T(MBB_DEBUG, "|NV_36:\n");
    DBG_T(MBB_DEBUG, "|factory_mode = %d\n", g_usb_nv_info.factory_mode);

    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*51*/
    DBG_T(MBB_DEBUG, "|NV_51:\n");
    DBG_T(MBB_DEBUG, "|ulSDWorkMode =%d \n", g_usb_nv_info.ulSDWorkMode);
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
#endif
    /*50071*/
    DBG_T(MBB_DEBUG, "|NV_50071:\n");
    DBG_T(MBB_DEBUG, "|nv_status = %d \n", g_usb_nv_info.pid_info.nv_status);
    DBG_T(MBB_DEBUG, "|cust_before_pid = 0x%X \n", g_usb_nv_info.pid_info.cust_first_pid );
    DBG_T(MBB_DEBUG, "|cust_after_pid  = 0x%X \n", g_usb_nv_info.pid_info.cust_rewind_pid );
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*50091*/
    DBG_T(MBB_DEBUG, "|NV_50091:\n");
    for (i = 0; i < DYNAMIC_PID_MAX_PORT_NUM; i++)
    {
        if (g_usb_nv_info.dev_profile_info.first_port_style[i])
        {
            DBG_T(MBB_DEBUG, "| first_port_style[%2d] =0x%X\n",
                  i, g_usb_nv_info.dev_profile_info.first_port_style[i]);
        }
        else
        {
            break;
        }
    }
    for (i = 0; i < DYNAMIC_PID_MAX_PORT_NUM; i++)
    {
        if (g_usb_nv_info.dev_profile_info.rewind_port_style[i])
        {
            DBG_T(MBB_DEBUG , "|rewind_port_style[%2d] =0x%X\n",
                  i, g_usb_nv_info.dev_profile_info.rewind_port_style[i]);
        }
        else
        {
            break;
        }

    }
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*50108*/
    DBG_T(MBB_DEBUG, "|NV_50108: \n");
    DBG_T(MBB_DEBUG, "|nv_status = %d\n", g_usb_nv_info.mass_dynamic_name.nv_status);
    DBG_T(MBB_DEBUG, "|huawei_cdrom_dynamic_name: %28.28s \n"
          , g_usb_nv_info.mass_dynamic_name.huawei_cdrom_dynamic_name);
    DBG_T(MBB_DEBUG, "|huawei_sd_dynamic_name: %s \n"
          , g_usb_nv_info.mass_dynamic_name.huawei_sd_dynamic_name);
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*50109*/
    DBG_T(MBB_DEBUG, "|NV_50109:\n");
    DBG_T(MBB_DEBUG, "|nv_status = %d \n"
          , g_usb_nv_info.port_dynamic_name.nv_status);
    DBG_T(MBB_DEBUG, "|huawei_product_dynamic_name: %s \n"
          , g_usb_nv_info.port_dynamic_name.huawei_product_dynamic_name);
    DBG_T(MBB_DEBUG, "|huawei_manufacturer_dynamic_name: %s\n"
          , g_usb_nv_info.port_dynamic_name.huawei_manufacturer_dynamic_name);
    DBG_T(MBB_DEBUG, "|huawei_configuration_dynamic_name: %s \n"
          , g_usb_nv_info.port_dynamic_name.huawei_configuration_dynamic_name);
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*50110*/
    DBG_T(MBB_DEBUG, "|NV_50110: \n");
    DBG_T(MBB_DEBUG, "|nv_status = %d\n"
          , g_usb_nv_info.stDevProtOfsInfo.nv_status);
    DBG_T(MBB_DEBUG , "|protocol_base = 0x%X\n",
          g_usb_nv_info.stDevProtOfsInfo.protocol_base);
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
#ifdef MBB_USB_UNITARY_Q
    /*50151*/
    DBG_T(MBB_DEBUG, "|NV_50151: \n");
    DBG_T(MBB_DEBUG, "|nv_status = %d\n"
          , g_usb_nv_info.feature_wwan.nv_status);
    DBG_T(MBB_DEBUG , "|wwan_work_flag = %u\n",
          g_usb_nv_info.feature_wwan.wwan_work_flag);
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*50367*/
    DBG_T(MBB_DEBUG, "|NV_50367: \n");
    DBG_T(MBB_DEBUG, "|nv_status = %d\n"
          , g_usb_nv_info.hilink_mode_flag.nv_status);
    DBG_T(MBB_DEBUG , "|wwan_work_flag = %u\n",
          g_usb_nv_info.hilink_mode_flag.mode_flag);
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
#endif
    /*50394*/
    DBG_T(MBB_DEBUG, "|NV_50394: \n");
    DBG_T(MBB_DEBUG, "|nv_status = %d\n"
          , (int)(g_usb_nv_info.dynamic_vid_info.nv_status));
    DBG_T(MBB_DEBUG, "|dynamic_vid = %d\n"
          , (int)(g_usb_nv_info.dynamic_vid_info.dynamic_vid));
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*50395*/
    DBG_T(MBB_DEBUG, "|NV_50395: \n");
    DBG_T(MBB_DEBUG, "|nv_status = %d\n"
          , (int)(g_usb_nv_info.boot_pid_info.nv_status));
    DBG_T(MBB_DEBUG, "|boot_pid = %d\n"
          , (int)(g_usb_nv_info.boot_pid_info.dynamic_boot_pid));
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*50396*/
    DBG_T(MBB_DEBUG, "|NV_50396: \n");
    DBG_T(MBB_DEBUG, "|nv_status = %d\n"
          , (int)(g_usb_nv_info.dynamic_pid_info.nv_status));
    DBG_T(MBB_DEBUG, "|dynamic_normal_pid = %d\n"
          , (int)(g_usb_nv_info.dynamic_pid_info.dynamic_normal_pid));
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*50397*/
    DBG_T(MBB_DEBUG, "|NV_50397: \n");
    DBG_T(MBB_DEBUG, "|nv_status = %d\n"
          , (int)(g_usb_nv_info.debug_pid_info.nv_status));
    DBG_T(MBB_DEBUG, "|dynamic_debug_pid = %d\n"
          , (int)(g_usb_nv_info.debug_pid_info.dynamic_debug_pid));
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*50456*/
    DBG_T(MBB_DEBUG, "|NV_50456: \n");
    DBG_T(MBB_DEBUG, "|nv_status = %d\n"
          , (int)(g_usb_nv_info.net_speed_info.nv_status));
    DBG_T(MBB_DEBUG, "|net_speed = %d\n"
          , (int)(g_usb_nv_info.net_speed_info.net_speed));
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*50457*/
    /* 模块产品必须支持该NV，并且在xml文件中写入默认值*/
    DBG_T(MBB_DEBUG, "|NV_50457:\n");
    DBG_T(MBB_DEBUG, "|nv_status = %ld\n"
          , g_usb_nv_info.multi_config_port_info.nv_status);
    for (i = 0; i < DYNAMIC_PID_MAX_PORT_NUM; i++)
    {
        if (g_usb_nv_info.multi_config_port_info.first_config_port[i])
        {
            DBG_T(MBB_DEBUG, "| first_config_port[%2d] =0x%X\n",
                  i, g_usb_nv_info.multi_config_port_info.first_config_port[i]);
        }
        else
        {
            break;
        }
    }
    for (i = 0; i < DYNAMIC_PID_MAX_PORT_NUM; i++)
    {
        if (g_usb_nv_info.multi_config_port_info.second_config_port[i])
        {
            DBG_T(MBB_DEBUG , "|second_config_port[%2d] =0x%X\n",
                  i, g_usb_nv_info.multi_config_port_info.second_config_port[i]);
        }
        else
        {
            break;
        }

    }
    for (i = 0; i < DYNAMIC_PID_MAX_PORT_NUM; i++)
    {
        if (g_usb_nv_info.multi_config_port_info.third_config_port[i])
        {
            DBG_T(MBB_DEBUG , "|third_config_port[%2d] =0x%X\n",
                  i, g_usb_nv_info.multi_config_port_info.third_config_port[i]);
        }
        else
        {
            break;
        }

    }
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*50498*/
    DBG_T(MBB_DEBUG, "|NV_50498: \n");
    DBG_T(MBB_DEBUG, "|nv_status = %d\n"
          , g_usb_nv_info.usb_log_ctl.nv_status);
    for ( i = 0; i < MODE_NUM; i++ )
    {
        DBG_T(MBB_DEBUG , "|debug_mode[%2d] =%u\n",
              i, g_usb_nv_info.usb_log_ctl.debug_mode[i]);
    }
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*50538*/
    DBG_T(MBB_DEBUG, "|NV_50538: \n");
    DBG_T(MBB_DEBUG, "|nv_status = %d\n"
          , (int)(g_usb_nv_info.debug_mode.nv_status));
    DBG_T(MBB_DEBUG, "|setmode_flag = %d\n"
          , (int)(g_usb_nv_info.debug_mode.setmode_flag));
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
     /*50524*/
    DBG_E(MBB_DEBUG,"U_NV:NV_50524:\n");
    DBG_E(MBB_DEBUG ,"U_NV:  nv_status =%d \n",g_usb_nv_info.usb_tethering_flag.nv_status);
    DBG_E(MBB_DEBUG ,"U_NV:  ucActiveFlag =%d \n",g_usb_nv_info.usb_tethering_flag.ucActiveFlag);

    /*50577*/
    DBG_T(MBB_DEBUG, "|NV_50577: \n");
    DBG_T(MBB_DEBUG, "|nv_status = %d\n"
          , g_usb_nv_info.usb_security_flag.nv_status);
    DBG_T(MBB_DEBUG, "|U_NV: diag_enable = %d\n",g_usb_nv_info.usb_security_flag.diag_enable);
    DBG_T(MBB_DEBUG, "|U_NV: shell_enable = %d\n",g_usb_nv_info.usb_security_flag.shell_enable);
    DBG_T(MBB_DEBUG, "|U_NV: adb_enable = %d\n",g_usb_nv_info.usb_security_flag.adb_enable);
    DBG_T(MBB_DEBUG, "|U_NV: cbt_enable = %d\n",g_usb_nv_info.usb_security_flag.cbt_enable);
}
EXPORT_SYMBOL(usb_nv_dump);

/*****************************************************************
Parameters    :
Return        :
Description   :  设置各个模块的log级别
*****************************************************************/
USB_VOID usb_log_level_set(USB_INT module, USB_INT level)
{
    USB_INT i = 0;
    mbb_usb_nv_info_st*   usb_nv_info = NULL;
    usb_nv_info = usb_nv_get_ctx();
    for ( i = 0 ; i < MBB_LAST ; i++)
    {
        if ( i == module)
        {
            usb_nv_info->usb_log_ctl.debug_mode[i] = level;
        }
    }
    if (!usb_write_nv( USB_NV_USB_PRIVATE_INFO , usb_nv_info->usb_log_ctl.debug_mode , MBB_LAST ))
    {
        DBG_T(MBB_DEBUG, "USB_DEBUG_MODE SET SUCCESSFUL!\n");
    }
    else
    {
        DBG_T(MBB_DEBUG, "USB_DEBUG_MODE SET FAILED!\n");
    }
}
EXPORT_SYMBOL_GPL(usb_log_level_set);

USB_VOID usb_log_level_get(USB_VOID)
{
    USB_INT i = 0;
    mbb_usb_nv_info_st*   usb_nv_info = NULL;
    usb_nv_info = usb_nv_get_ctx();
    DBG_T(MBB_DEBUG, "|U_TRACE = 0,U_ERROR = 1,U_WARN = 2,U_INFO = 3,U_ALL =4|\n");
    for ( i = 2; i < MBB_LAST; i++)
    {
        DBG_T(MBB_DEBUG, "|%15.15s(%-2d)---- lev: %-10.10s |\n",
              debug_bank[i].module_name, i, lev_name[usb_nv_info->usb_log_ctl.debug_mode[i]]);
    }
}
EXPORT_SYMBOL_GPL(usb_log_level_get);

