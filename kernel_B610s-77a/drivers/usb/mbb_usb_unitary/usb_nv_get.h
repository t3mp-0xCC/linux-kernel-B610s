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

#ifndef __USB_NV_GET_H__
#define __USB_NV_GET_H__
#include "usb_platform_comm.h"
#include "usb_debug.h"
#include "hw_pnp.h"

/*|==========================================================================|*/
/*|申请新的NV说明                                                            |*/
/*|新申请的NV请添加一个类型为unsigned long int 的头字段:字节对齐             |*/
/*|USB_UINT32 nv_status;                                                     |*/
/*|请参考  NV50049                                                           |*/
/*|balong 平台NV必须是4字节整数倍，申请NV时请注意                            |*/
/*|==========================================================================|*/

/*需要和申请的NV结构体数组下标一致16*/
#define MODE_NUM   16

/*------------------------------------------------------------------------------
 HS USB Huawei Device Interface Table Type Definition
------------------------------------------------------------------------------*/
/*模块配置端口通过下面的protocol。需要转译*/
typedef enum
{
    HSU_CONF_HWDEV_INVALID_IFACE      = 0x00,

    HSU_CONF_HWDEV_MODEM_IFACE        = 0x01,  /*modem interface*/
    HSU_CONF_HWDEV_PCUI_IFACE         = 0x02,  /* pcui interface*/
    HSU_CONF_HWDEV_DIAG_IFACE         = 0x03,  /* diag interface*/
    HSU_CONF_HWDEV_PCSC_IFACE         = 0x04,  /*pcsc interface*/
    HSU_CONF_HWDEV_GPSNMEA_IFACE      = 0x05,  /*gps interface*/

    HSU_CONF_HWDEV_RMNET_IFACE        = 0x07,  /*remote network interface*/
    HSU_CONF_HWDEV_ECM_DATA_IFACE     = 0x08,  /*ECM data interface*/

    HSU_CONF_HWDEV_MBIM_IFACE         = 0x20,  /*PC voice interface*/


    HSU_CONF_HWDEV_AUTORUN_DISABLED   = 0xFF,  /*Disable the autorun function*/
} hsu_conf_hwdev_iface_enum_type;


typedef struct huawei_interface_protocol_translation_st
{
    hsu_conf_hwdev_iface_enum_type  old_standard;
    USB_PID_UNIFY_IF_PROT_T   new_standard;
} protocol_translation_info;


typedef enum 
{                                       /*|==================================|*/
#ifdef MBB_USB_UNITARY_Q                /*|                                  |*/
    USB_NV_SERIAL_NUM_ID      = 50049,  /*|高通平台控制SN固定与否            |*/
#else                                   /*|                                  |*/
    USB_NV_SERIAL_NUM         = 6,      /*|balong 产品sn                     |*/
    USB_NV_SERIAL_NUM_ID      = 26,     /*|balong平台控制SN固定与否          |*/
    USB_NV_ITEM_AT_SHELL_OPEN = 33,     /*|balong平台 shell 端口控制         |*/
    USB_NV_FACTORY_MODE_I     = 36,     /*|balong平台 产线升级模式           |*/
    USB_NV_SD_WORKMODE        = 51,     /*|balong平台 记录SD卡工作模式       |*/
#endif/*MBB_USB_UNITARY_Q*/             /*|                                  |*/
    USB_NV_PID_UNIFICATION_ID = 50071,  /*|PID可定制项                       |*/
    USB_NV_PORT_INFO_ID       = 50091,  /*|端口可配置                        |*/
    USB_NV_MASS_DYNAMIC_NAME  = 50108,  /*|光盘名称可定制                    |*/
    USB_NV_DYNAMIC_INFO_NAME  = 50109,  /*|设备名称可定制                    |*/
    USB_NV_PROT_OFFSET_ID     = 50110,  /*|VDF定制端口                       |*/
                                        /*|重定义了VDF端口协议字段(废弃50110)|*/
    USB_NV_FEATURE_WWAN       = 50151,  /*|高通平台WWAN功能可定制，balong:d123|*/
    USB_NV_HILINK_MODEM_MODE  = 50367,  /*|HILINK_MODEM功能可定制            |*/
    USB_NV_HUAWEI_DYNAMIC_VID  = 50394, /*| VID可定制|*/
    USB_NV_HUAWEI_DYNAMIC_BOOT_PID  = 50395,  /*|模块升级PId可定制      |*/
    USB_NV_HUAWEI_DYNAMIC_NORMAL_PID  = 50396,/*|模块正常模式PID可定制  |*/
    USB_NV_HUAWEI_DYNAMIC_DEBUG_PID  = 50397, /*|模块debug模式PID可定制 |*/
    USB_NV_WINBLUE_PRF_ID     = 50424,     /*|MBIM功能可定制                 |*/
    USB_NV_NET_SPEED_ID       = 50456,     /*|单板上报给PC的网卡速率         |*/
    USB_NV_MULTI_CONFIG_PORT_INFO = 50457, /*|模块产品端口形态配置项         |*/
    USB_NV_USB_PRIVATE_INFO   = 50498,     /*|USB log可控制                  |*/
    USB_NV_USB_TETHERING_ID   =  50524,     /*|USB网络共享定制 |*/
    USB_NV_USB_DEBUG_MODE_FLAGE   = 50538, /*|调试模式配置                   |*/
    USB_NV_SECURITY_FLAG          = 50577    /*|USB调试模式端口使能配置       |*/
} usb_nv_item;                             /*|===============================|*/


typedef struct COMMON_NV_TYPE
{
    USB_UINT nv_status;
    USB_UINT8 reserved;
} nv_huawei_common_type;

/*NV 6*/
#define SERIAL_NUM_DEFAULT "0123456789ABCDEF"
#define SERIAL_NUM_MAX 20
typedef struct
{
    USB_UINT8   aucSerialNumber[SERIAL_NUM_MAX];
}nv_huawei_device_serial_num;

/*NV 26*/
typedef struct tagUSB_NV_SERIAL_NUM_T
{
    USB_UINT usbSnNvStatus;
    USB_UINT usbSnNvSucFlag;
} USB_NV_SERIAL_NUM_T;


/* NV 50049 SN enable */
typedef struct NV_HUAWEI_SN_REPORT_ENABLE_I
{
    USB_UINT    nv_status;
    USB_UINT8   sn_enable;
} nv_huawei_sn_report_enable;

/* 自定义 PID使能 */
typedef struct PID_ENABLE
{
    USB_UINT pid_enabled;
} nv_pid_enable_type;

/* NV 50067 SD enable */
typedef struct NV_HUAWEI_FIRST_REPORT_PORT_I
{
    USB_UINT    nv_status;
    USB_UINT8   sd_enable;
} nv_huawei_first_report_port;

/* 50071 PID */
typedef struct PRODUCT_PID
{
    USB_UINT nv_status;
    USB_UINT16 cust_first_pid;
    USB_UINT16 cust_rewind_pid;
} nv_cust_pid_type;

/* NV 50080 CD enable */
typedef struct NV_HUAWEI_DISABLE_CDROM_I
{
    USB_UINT nv_status;
    USB_UINT8     cd_enable;
} nv_huawei_disable_cdrom;

/* 50091 端口形态 */
typedef struct PACKED_POST
{
    USB_UINT nv_status;
    USB_UINT8 first_port_style[DYNAMIC_PID_MAX_PORT_NUM];
    USB_UINT8 rewind_port_style[DYNAMIC_PID_MAX_PORT_NUM];
    USB_UINT8 reserved[USB_NUM_22];
} nv_huawei_dynamic_pid_type;

/*  比nv 50108 结构体空间大2 byte，用来存储字串结束符*/
typedef struct NV_HUAWEI_DYNAMIC_NAME_I
{
    USB_UINT nv_status;
    USB_UINT8 huawei_cdrom_dynamic_name[DYNAMIC_CD_NAME_CHAR_NUM + 1];
    USB_UINT8 huawei_sd_dynamic_name[DYNAMIC_CD_NAME_CHAR_NUM + 1];
    USB_UINT8 reserved[USB_NUM_32];
} nv_huawei_dynamic_name;

/* NV 50108 CD/SD name  与nv结构体一致*/
typedef struct NV_HUAWEI_DYNAMIC_NAME_I_REALY
{
    USB_UINT nv_status;
    USB_UINT8 huawei_cdrom_dynamic_name[DYNAMIC_CD_NAME_CHAR_NUM];
    USB_UINT8 huawei_sd_dynamic_name[DYNAMIC_CD_NAME_CHAR_NUM];
    USB_UINT8 reserved[USB_NUM_32];
} nv_huawei_dynamic_name_realy;

/* NV 50109 ports name*/
typedef struct NV_HUAWEI_DYNAMIC_INFO_NAME_I
{
    USB_UINT nv_status;
    USB_UINT8 huawei_product_dynamic_name[DYNAMIC_INFO_NAME_CHAR_NUM];
    USB_UINT8 huawei_manufacturer_dynamic_name[DYNAMIC_INFO_NAME_CHAR_NUM];
    USB_UINT8 huawei_configuration_dynamic_name[DYNAMIC_INFO_NAME_CHAR_NUM];
    USB_UINT8 reserved[USB_NUM_8];
} nv_huawei_dynamic_info_name;
#ifndef MBB_USB_UNITARY_Q
/* 50110 接口协议基数 */
typedef struct PROTOCOL_BASE
{
    USB_UINT nv_status;
    USB_UINT protocol_base;
} nv_protocol_base_type;
#else
typedef struct PROTOCOL_BASE
{
    USB_UINT nv_status;
    USB_UINT8 protocol_base;
    USB_UINT8 reserved[USB_NUM_20];
} nv_protocol_base_type;

#endif
/* 50151 WIN7 wwan特性 */
typedef struct NV_HUAWEI_WWAN_CUSTOMIZATION
{
    USB_UINT  nv_status;
    USB_UINT8 wwan_work_flag;
} nv_feature_wwan;

/* 50367 hilink modem 互相切换。模式保存 */
typedef struct NV_HILINK_MODEM_MODE
{
    USB_UINT  nv_status;
    USB_UINT8 mode_flag;
}nv_huawei_hilink_modem_mode;

/*50394 VID*/
typedef struct VENDER_ID
{
    USB_UINT16 nv_status;
    USB_UINT16 dynamic_vid;
} nv_huawei_dynamic_vid_type;

/*50395 boot PID*/
typedef struct
{
    USB_UINT16 nv_status;
    USB_UINT16 dynamic_boot_pid;
    
} nv_huawei_dynamic_boot_pid_type;

/*50396 normal MODE PID*/
typedef struct
{
    USB_UINT16 nv_status;
    USB_UINT16 dynamic_normal_pid;
    
}nv_huawei_dynamic_normal_pid_type;

/*50397 debug MODE PID*/
typedef struct
{
    USB_UINT16 nv_status;
    USB_UINT16 dynamic_debug_pid;
    
}nv_huawei_dynamic_debug_pid_type;

/*50424*/
typedef struct DRV_WINBLUE_PROFILE_TYPE
{
    USB_UINT8  InterfaceName[32];
    USB_UINT8  MBIMEnable;
    USB_UINT8  CdRom;
    USB_UINT8  TCard;
    USB_UINT8  MaxPDPSession;
    USB_UINT16 IPV4MTU;
    USB_UINT16 IPV6MTU;
    USB_UINT32 Reserved1;
    USB_UINT32 Reserved2;
} nv_winblue_profile_type;

/*50456*/
typedef struct USB_CDC_NET_SPEED_TYPE
{
    USB_UINT32  nv_status;
    USB_UINT32  net_speed;  //单位为bps(例如300Mbps=300*1000*1000 bps),注意单位不是1024
    USB_UINT32  reserve1;
    USB_UINT32  reserve2;
    USB_UINT32  reserve3;
} usb_cdc_net_speed_type;

/* 50457 端口形态 ，*/
/* 模块产品必须支持该NV，并且在xml文件中写入默认值*/
typedef struct MULTI_PORT_INFO
{
    USB_UINT32 nv_status;
    USB_UINT8 first_config_port[DYNAMIC_PID_MAX_PORT_NUM];
    USB_UINT8 second_config_port[DYNAMIC_PID_MAX_PORT_NUM];
    USB_UINT8 third_config_port[DYNAMIC_PID_MAX_PORT_NUM];
    USB_UINT8 reserved_1[DYNAMIC_PID_MAX_PORT_NUM];
    USB_UINT8 reserved_2[DYNAMIC_PID_MAX_PORT_NUM];
    USB_UINT8 reserved_3[USB_NUM_19];
} nv_huawei_multi_config_usb_port_type;

/*50498*/
typedef struct USB_PRIVATE_NV_INFO
{
    USB_UINT   nv_status;
    USB_UINT8  debug_mode[MODE_NUM];
    USB_UINT32 reserve[10];
} usb_private_nv_info;

/*50538 setmode*/
typedef struct DEBUG_MODE_FLAG
{
    USB_UINT32 nv_status;
    USB_UINT32 setmode_flag;
} nv_huawei_debug_mode_flag;


typedef struct
{
    USB_UINT32 nv_status;    /*该nv是否激活*/
    USB_UINT16 ucActiveFlag;/*是否开启usb tethering功能*/
    USB_UINT16 ucReserved1; /*保留字段1*/
    USB_UINT16 ucReserved2; /*保留字段2*/
    USB_UINT16 ucReserved3; /*保留字段3*/
} HUAWEI_NV_USB_TETHERING;

/*50577*/
typedef struct
{
unsigned char nv_status;    /*该nv是否激活*/
unsigned char diag_enable;  /*diag端口是否使能*/
unsigned char shell_enable; /*shell端口是否使能*/
unsigned char adb_enable;   /*adb端口是否使能*/
unsigned char cbt_enable;   /*cbt端口是否使能*/
unsigned char ucReserved_0; /*保留字段*/
unsigned char ucReserved_1; /*保留字段*/
unsigned char ucReserved_2; /*保留字段*/
}nv_huawei_usb_security_flag;

/*define struct ,list all correlative usb nv*/
typedef struct mbb_usb_nv_info
{
    nv_huawei_device_serial_num    serial_num;
    nv_cust_pid_type               pid_info ;              //USB_NV_PID_UNIFICATION_ID
    nv_huawei_dynamic_pid_type     dev_profile_info ; //USB_NV_PORT_INFO_ID
#ifdef MBB_USB_UNITARY_Q
    nv_huawei_sn_report_enable     stUSBNvSnSupp ; //USB_NV_SERIAL_NUM_ID
#else
    USB_NV_SERIAL_NUM_T              stUSBNvSnSupp ; //USB_NV_SERIAL_NUM_ID
    USB_UINT                       factory_mode ;    //USB_NV_FACTORY_MODE_I
    USB_UINT                       shell_lock;       //USB_NV_ITEM_AT_SHELL_OPEN
#endif
    USB_UINT                       ulSDWorkMode;     //USB_NV_SD_WORKMODE
    nv_feature_wwan                feature_wwan;  //USB_NV_FEATURE_WWAN
    nv_huawei_dynamic_name         mass_dynamic_name; //USB_NV_MASS_DYNAMIC_NAME
    nv_huawei_dynamic_info_name    port_dynamic_name;   //USB_NV_DYNAMIC_INFO_NAME
    nv_protocol_base_type          stDevProtOfsInfo;  //USB_NV_PROT_OFFSET_ID
    nv_huawei_hilink_modem_mode    hilink_mode_flag;
    nv_winblue_profile_type        winblue_profile;
    usb_cdc_net_speed_type         net_speed_info;
    nv_huawei_multi_config_usb_port_type multi_config_port_info;
    usb_private_nv_info            usb_log_ctl;
    nv_huawei_debug_mode_flag      debug_mode;
    nv_huawei_dynamic_vid_type                 dynamic_vid_info;
    nv_huawei_dynamic_boot_pid_type            boot_pid_info;
    nv_huawei_dynamic_normal_pid_type          dynamic_pid_info;
    nv_huawei_dynamic_debug_pid_type           debug_pid_info;
    HUAWEI_NV_USB_TETHERING        usb_tethering_flag;    //USB_NV_USB_TETHERING_ID
    nv_huawei_usb_security_flag    usb_security_flag;   //USB_NV_SECURITY_FLAG
} mbb_usb_nv_info_st;

/*==========================================================
 *===============================API声明=====================
 *===========================================================*/
/****************************************************************
 函 数 名  : usb_nv_init
 功能描述  : usb NV 初始化。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID    usb_nv_init(USB_VOID);
/****************************************************************
 函 数 名  : usb_nv_get
 功能描述  : USB 读取NV接口。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : MBB_USB_OK
*****************************************************************/
/*读取NV文件接口*/
USB_INT     usb_nv_get(USB_VOID);
/****************************************************************
 函 数 名  : get_shell_lock
 功能描述  : 获取shell lock标识。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : shell lock标识。
*****************************************************************/
USB_INT     get_shell_lock(USB_VOID);
/****************************************************************
 函 数 名  : usb_nv_get_ctx
 功能描述  : 获取USB 相关NV 的结构体集。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : USB 相关NV 的结构体集。
*****************************************************************/
/*各模块获取NV值接口*/
mbb_usb_nv_info_st*  usb_nv_get_ctx(USB_VOID);

/*****************************************************************
Parameters    : PID 对应的index值
Return        :    无
Description   : 取PID 对应的字符串描述符地址
*****************************************************************/
huawei_port_string* usb_device_desc_string_get(USB_INT index);
/*****************************************************************
Parameters    : PID 对应的index值
Return        :    无
Description   : 取PID 对应的index值
*****************************************************************/
USB_INT device_desc_get_idx(USB_VOID);
/****************************************************************
 函 数 名  : usb_nv_dump
 功能描述  : us NV 的dump信息。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID usb_nv_dump(USB_VOID);
USB_VOID usb_log_level_set(USB_INT module, USB_INT level);
USB_VOID usb_log_level_get(USB_VOID);

USB_UINT usb_get_mbim_config(USB_VOID);
USB_UINT usb_is_m2m_multi_config(USB_VOID);

#endif