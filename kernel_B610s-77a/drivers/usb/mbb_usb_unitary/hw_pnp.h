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


#ifndef __HW_PNP_H__
#define __HW_PNP_H__

#include "usb_debug.h"
#include "usb_workqueue.h"
/*-------------------------------------------------------------------------*/

/* Default vendor and product IDs*/
#define VENDOR_ID    0x12D1    /* NetChip */

#define SERIAL_NUM_FLAG_UNIQUE 1
/*UNIQUE_SERIAL_NUM_FLAG_PATH 非0时认为是USB唯一SN*/
/*为0或者不存在时，认为保持原来逻辑*/
#define UNIQUE_SERIAL_NUM_FLAG_PATH "/app/usb_unique_sn"
#define ONLY_CHARGE_PATH  "/data/userdata/usb_config/only_charge_mode"
#define ONLY_CHARGE_MODE  '1'
#define USER_MODE  '0'
#define USB_DRIVER_NAME		"android_usb"
#define DIAL_MODE_MODEM         0
#define DIAL_MODE_NDIS          1
#define DIAL_MODE_MODEM_N_NDIS  2
#define DIAL_MODE_NONE          3

#define CDC_SPEC_NONE           0
#define CDC_SPEC_MODEM          1
#define CDC_SPEC_NDIS           2
#define CDC_SPEC_MODEM_N_NDIS   3

/* string IDs are assigned dynamically */
#define STRING_MANUFACTURER_IDX 0
#define STRING_PRODUCT_IDX      1
#define STRING_SERIAL_IDX       2
#define STRING_MSOS_IDX         3

#define ANDROID_DEVICE_NODE_NAME_LENGTH 11

#define SETMODE_UNKNOWN_STATE  -1

#define USB_LOW_MASK    0x00FF
#define USB_HIGH_MASK   0xFF00

#define MULTI_CONFIG_ONE      1
#define MULTI_CONFIG_TWO      2
#define MULTI_CONFIG_THR      3

#define USB_CONFIG_MBIM       MULTI_CONFIG_TWO

#define CTL_APP_START      0
#define CTL_APP_DISABLE    1
#define CTL_APP_WAIT       9

#define CTL_APP_START_FALG      '0'
#define CTL_APP_DISABLE_FALG    '1'
#define CTL_APP_WAIT_FALG       '9'

#define GATEWAT_MODEM_MODE    0
#define GATEWAY_NDIS_MODE     1
#define GATEWAY_MODE_INVALID    0xff

#define APP_MODE_FLAG_LEN     2
#define APP_WRITE_DATA_SITE   0

#define MAX_SHOW_LEN          4096
#define MAX_STRING_DESC_NUM   256

#define TCM_FOR_ISO_LOAD_FLAG_VAL          0x87654321

#define HUAWEI_CONFIG_DYNAMIC_PID_CONTINUE 0
#define HUAWEI_CONFIG_DYNAMIC_PID_COMPLETE 1

#define DYNAMIC_PID_MAX_PORT_NUM     17
#define MAX_SUPPORT_PORT_NAME_STRING 255
#define MAX_TOTAL_PORT_NUM           255

#define CONFIG_TWO_MAX_PORT_NUM      4 //JUST FOR WIN8,do not need driver

#define DYNAMIC_CD_NAME_CHAR_NUM     28
#define DYNAMIC_INFO_NAME_CHAR_NUM   40

#define MAX_REWIND_PARAM_NUM    15

#define PNP_SUPPORT_FUNC_MAX    50

#define MAX_INTERFACE_VECTOR_NUM    100
#define MAX_PORT_MODE_NUM    20

#define MANUFACTURER_BUFFER_SIZE 256
#define MAX_SPEED_NAME_LEN       16

#define WINDOWS_OS_FLAG    0x00
#define MAC_OS_FLAG        0x10
#define LINUX_OS_FLAG      0x20
#define GATEWAY_OS_FLAG    0x30
#define OS_FLAG_INVALID    0xff

#define SC_REWIND_11       0x11
#define SC_REWIND_11_LEN   31

#define VENDOR_ID_HUAWEI      0x12D1
#define VENDOR_PRO            0xFF

typedef enum
{
    LOW_LINUX_PID          = 0x1001,  /* 适配网关的PID gateway ndis    */
    DOWNLOAD_SCENE_PID_E5  = 0x1c05,  /* E5 stick 升级PID/烧片版本     */
    POWER_OFF_CDROM_PID    = 0x1C20,  /* power off                     */
    RNDIS_DEBUG            = 0x1441,  /* RNDIS形态产品 高通平台调试模式*/
    DOWNLOAD_SCENE_PID     = 0x1442,  /* RNDIS形态产品 升级PID         */
    RNDIS_WORKMODE         = 0x14db,  /* RNDIS形态产品 工作模式        */
    RNDIS_WORKMODE_SD      = 0x14DC,  /* RNDIS形态产品带SD工作模式     */
	GATEWAY_NDIS_PID       = 0x155E,  /* 适配网关的PID gateway ndis    */
	MBIM_DOWNLOAD_PID      = 0x1568,  /* 支持mbim产品升级PID           */
    RNDIS_DEBUG_PID        = 0x1566,  /* RNDIS形态产品 balong平台调试  */
    DYNAMIC_PID            = 0x1506,  /* 归一化PID方案,数据卡类产品PID */
    MBIM_HILINK_PID        = 0x157D,  /* RNDIS形态产品支持mbim时使用   */
    MBIM_MAINLINE_PID      = 0x15CD,  /* 非RNDIS形态产品支持mbim时使用 */
    CDROM_HILINK_PID       = 0x1F01,  /* RNDIS形态产品 光盘模式PID     */
    CDROM_MAINLINE_PID     = 0x15CA,  /* 非RNDIS形态产品 光盘模式PID   */
    MODULE_BALONG_PID      = 0x15C1,  /* 归一化PID方案模块产品PID      */
    MODULE_BALONG_DEBUG_PID= 0x1F1A,  /* 归一化PID方案模块产品DEBUG PID*/
    MODULE_BALONG_HP_PID   = 0x931D   /* 归一化PID方案模块产品HP PID   */
}mbb_usb_pid;


#ifdef USB_RNDIS
#ifdef USB_SD
#define PID_USER   RNDIS_WORKMODE_SD /* RNDIS+SD*/
#else/*USB_SD*/
#define PID_USER   RNDIS_WORKMODE    /* RNDIS   */
#endif/*USB_SD*/

#else/*USB_RNDIS*/
#define PID_USER   DYNAMIC_PID
#endif/*USB_RNDIS*/

#ifdef USB_RNDIS
#define PRODUCT_ID_CONFIG2 MBIM_HILINK_PID
#define CDROM_SCENE_PID    CDROM_HILINK_PID

#else/*USB_RNDIS*/
#define PRODUCT_ID_CONFIG2 MBIM_MAINLINE_PID
#define CDROM_SCENE_PID    CDROM_MAINLINE_PID
#endif/*USB_RNDIS*/


#ifdef USB_SOLUTION
#define DYNAMIC_DEBUG_PID  MODULE_BALONG_DEBUG_PID
#else
#define DYNAMIC_DEBUG_PID  DYNAMIC_PID
#endif

#define PRODUCT_ID_P711SOLUTION MODULE_BALONG_PID //P711s 模块新分配 pid
#define DYNAMIC_HP_PID     MODULE_BALONG_HP_PID

/* 如果是多配置则通知PC取config 2，如果是单配置则通知PC取config 1 */
#define ALTRCFG_3 0x33/*'3'*/
#define ALTRCFG_2 0x32/*'2'*/
#define ALTRCFG_1 0x31/*'1'*/

#define EXISTS     1
#define NON_EXISTS 0

#define TIME_OFF_MS 50

#define USB_SUBCLASS_BALONG              0x03  //BALONG 平台huawei usb interface subclass 
#define USB_SUBCLASS_JUNGO               0x02  //huawei usb in linux os subclass 
#define USB_SUBCLASS_LINUX               0x02  //linux下必须报0x02 否则后台不能识别
#define USB_SUBCLASS_GAOTONG             0x01  //高通平台   huawei usb interface subclass 

#define USB_SUBCLASS_BALONGSOLUTION           0x06  //BALONG平台模块产品huawei usb interface subclass 

#define USB_REQ_TYPE_MSOS_IN             0xC0   //vendor customize request type
#define USB_REQ_TYPE_MSOS_OUT            0x40   //vendor customize request type
#define USB_REQ_GET_MSOS_DEF             0xFE   //windows not install service
#define USB_REQ_GET_MSOS_CUSTOM          0xEE   //windows  installed service
#define USB_REQ_GET_MSOS_CUSTOM_MAC      0xA1  //mac  installed service

#define OS_TYPE_MASK       0xf0 //OS高位
#define OS_TYPE_WINDOWS    0x00 //OS windos
#define OS_TYPE_MAC        0x10 //mac
#define OS_TYPE_LINUX      0x20 //linux 
#define OS_TYPE_GATEWAY    0x30 //网关类包含android chrome
#define WORK_MODE_EXT      0x01
#define DOWNLOAD_MODE_EXT  0x00

#define USB_TYPE_HUAWEI    0xc0
#define USB_REQ_HUAWEI_EXT 0x9a
#define USB_REQ_HILINK_MODEM_SWITCH 0x20

/* MS OS String descriptor */
#define MSOS_STRING_INDEX      0xEE
#define MS_EXTEND_COMPAT_ID    0x04
#define MS_HUAWEI_VENDOR_CODE  0xDE
#define MS_EXTEND_CONTAINER_ID 0x06

#define PNP_CHECK_DEVICE_STATUS_COMPLETE 1
#define PNP_CHECK_DEVICE_STATUS_CONTINUE 0

#define PNP_CHECK_MODE_COMPLETE 1
#define PNP_CHECK_MODE_CONTINUE 0
#define PNP_CHECK_MODE_ERR -1

#define SET_ADD_NONE  0
#define IS_SET_ADD    1

/*MSOS命令 begin*/
/* Microsoft Extended Configuration Descriptor Header Section */
typedef struct _ms_extend_compact_id_header
{
    USB_UINT    dwLength;
    USB_UINT16  bcdVersion;
    USB_UINT16  wIndex;
    USB_UINT8   bCount;
    USB_UINT8   Reserved[USB_NUM_7];
} ms_extend_compact_id_header;

typedef struct _ms_extend_compact_id_function
{
    USB_UINT8 bFirstInterfaceNumber;
    USB_UINT8 bInterfaceCount;
    USB_UINT8 compatibleID[USB_NUM_8];
    USB_UINT8 subCompatibleID[USB_NUM_8];
    USB_UINT8 Reserved[USB_NUM_6];
} ms_extend_compact_id_function;

typedef struct _ms_extend_container_id
{
    USB_UINT    dwLength;
    USB_UINT16  bcdVersion;
    USB_UINT16  wIndex;
    USB_UINT8   containerID[16];
} ms_extend_container_id;

/*MSOS命令 end*/

typedef enum _ncm_mbim_debug_mode
{
    NCM_MBIM_MODE_NORMAL = 0,
    NCM_MBIM_MODE_DEBUG  = 1,
    ECM_LINUX_MODE_DEBUG = 2,
    ACM_PCUI_MODE_DEBUG  = 3,
    NCM_MBIM_MODE_MAX = 0xff,
} ncm_mbim_debug_mode;

enum android_device_state
{
    USB_DISCONNECTED,
    USB_CONNECTED,
    USB_CONFIGURED,
    USB_SUSPENDED,
    USB_RESUMED
};

enum dload_state
{
    PORT_NORMAL = 0,/* 正常流程,非任何升级 */
    PORT_NO_PCUI,  /* 升级模式但是,不报pcui口 */
    PORT_DLOAD,    /* 一键升级模式报口 */
    PORT_NV_RES,   /* 一键升级nv恢复模式报口 */
};

typedef enum
{
    NET_STATE_NOT_READY = 0 ,
    NET_STATE_ALREADY
} net_state;

#define HILINK_MODEM_DEBUG_MODE_FALG    '1'
#define HILINK_MODEM_MODE_FALG          '2'
#define HILINK_RNDIS_MODE_FALG          '3'

typedef enum hilink_modem_mode
{
    HILINK_MODEM_DEBUG_MODE = 1,
    HILINK_MODEM_MODE,
    HILINK_RNDIS_MODE,
}hilink_work_mode;

typedef enum
{
  NV_HILINK_TYPE ,
  NV_MODEM_TYPE ,
  NV_MODEM_DEBUG_TYPE = 2

} transf_hilink_modem_type;

typedef enum _usb_enum_state
{
    USB_ENUM_NONE = 0,
    USB_ENUM_START,
    USB_ENUM_DONE
} usb_enum_state;

typedef enum _usb_device_node_switch
{
    APP_SETMODE_NORMAL_WORK = 0,
    APP_SETMODE_NORMAL1,/*1*/
    APP_SETMODE_NORMAL2,/*2*/
    APP_SETMODE_NORMAL3,/*3*/
    APP_SETMODE_RNDIS_DEBUG,/*4*/
    APP_SETMODE_RNDIS_DLOAD,/*5*/
    APP_SETMODE_MBIM_DEBUG,  /*6*/
    APP_SETMODE_CHARGE_ONLY,  /*7*/
    APP_SETMODE_NDIS_SWITCH  /*8*/
} usb_device_node_switch;

typedef enum tagUSB_PID_UNIFY_IF_PROT_T
{
    /*通用版Protocol*/
    USB_IF_PROTOCOL_VOID         = 0x00,/*0x00*/
    USB_IF_PROTOCOL_3G_MODEM     = 0x01,/*0x01*/
    USB_IF_PROTOCOL_3G_PCUI      = 0x02,/*0x02*/
    USB_IF_PROTOCOL_3G_DIAG      = 0x03,/*0x03*/
    USB_IF_PROTOCOL_PCSC         = 0x04,/*0x04*/
    USB_IF_PROTOCOL_3G_GPS       = 0x05,/*0x05*/
    USB_IF_PROTOCOL_CTRL         = 0x06,/*0x06*/
    USB_IF_PROTOCOL_3G_NDIS      = 0x07,/*0x07*/
    USB_IF_PROTOCOL_NDISDATA     = 0x08,/*0x08*/
    USB_IF_PROTOCOL_NDISCTRL     = 0x09,/*0x09*/
    USB_IF_PROTOCOL_BLUETOOTH    = 0x0A,/*0x0A*/
    USB_IF_PROTOCOL_FINGERPRINT  = 0x0B,/*0x0B*/
    USB_IF_PROTOCOL_ACMCTRL      = 0x0C,/*0x0C*/
    USB_IF_PROTOCOL_MMS          = 0x0D,/*0x0D*/
    USB_IF_PROTOCOL_3G_PCVOICE   = 0x0E,/*0x0E*/
    USB_IF_PROTOCOL_DVB          = 0x0F,/*0x0F*/
    USB_IF_PROTOCOL_MODEM        = 0x10,/*0x10*/
    USB_IF_PROTOCOL_NDIS         = 0x11,/*0x11*/
    USB_IF_PROTOCOL_PCUI         = 0x12,/*0x12*/
    USB_IF_PROTOCOL_DIAG         = 0x13,/*0x13*/
    USB_IF_PROTOCOL_GPS          = 0x14,/*0x14*/
    USB_IF_PROTOCOL_PCVOICE      = 0x15,/*0x15*/
    USB_IF_PROTOCOL_NCM          = 0x16,/*0x16*/
    USB_IF_PROTOCOL_MODEM_ORANGE = 0x17,/*0x17*/
    USB_IF_PROTOCOL_SHEEL_A      = 0x18,/*0x18*/
    USB_IF_PROTOCOL_SHEEL_B      = 0x19,/*0x19*/
    USB_IF_PROTOCOL_MBIM         = 0x20,/*0x20*/
    USB_IF_PROTOCOL_COMM_A       = 0x1A,/*0x1A*/
    USB_IF_PROTOCOL_COMM_B       = 0x1B,/*0x1B*/
    USB_IF_PROTOCOL_COMM_C       = 0x1C,/*0x1C*/
    USB_IF_PROTOCOL_COMM_GNSS    = 0x1D,/*0x1D*/
    USB_IF_PROTOCOL_CDROM        = 0xA1,/*0xA1*/
    USB_IF_PROTOCOL_SDRAM        = 0xA2,/*0xA2*/
    USB_IF_PROTOCOL_RNDIS        = 0xA3,/*0xA3*/
    USB_IF_PROTOCOL_ADB          = 0xA4,/*0xA4*/
    /*VDF定制版Protocol*/
    VDF_USB_IF_PROTOCOL_MODEM          = 0x31,/*0x31*/
    VDF_USB_IF_PROTOCOL_PCUI           = 0x32,/*0x32*/
    VDF_USB_IF_PROTOCOL_DIAG           = 0x33,/*0x33*/
    VDF_USB_IF_PROTOCOL_PCSC           = 0x34,/*0x34*/
    VDF_USB_IF_PROTOCOL_GPS            = 0x35,/*0x35*/
    VDF_USB_IF_PROTOCOL_CTRL           = 0x36,/*0x36*/
    VDF_USB_IF_PROTOCOL_NDIS           = 0x37,/*0x37*/
    VDF_USB_IF_PROTOCOL_NDISDATA       = 0x38,/*0x38*/
    VDF_USB_IF_PROTOCOL_NDISCTRL       = 0x39,/*0x39*/
    VDF_USB_IF_PROTOCOL_BLUETOOTH      = 0x3A,/*0x3A*/
    VDF_USB_IF_PROTOCOL_FINGERPRINT    = 0x3B,/*0x3B*/
    VDF_USB_IF_PROTOCOL_ACMCTRL        = 0x3C,/*0x3C*/
    VDF_USB_IF_PROTOCOL_MMS            = 0x3D,/*0x3D*/
    VDF_USB_IF_PROTOCOL_PCVOICE        = 0x3E,/*0x3E*/
    VDF_USB_IF_PROTOCOL_DVB            = 0x3F,/*0x3F*/
    VDF_USB_IF_PROTOCOL_NCM            = 0x46,/*0x46*/
    VDF_USB_IF_PROTOCOL_SHEEL_A        = 0x48,/*0x48*/
    VDF_USB_IF_PROTOCOL_SHEEL_B        = 0x49,/*0x49*/
    VDF_USB_IF_PROTOCOL_COMM_A         = 0x4A,/*0x4A*/
    VDF_USB_IF_PROTOCOL_COMM_B         = 0x4B,/*0x4B*/
    VDF_USB_IF_PROTOCOL_COMM_C         = 0x4C,/*0x4C*/
    VDF_USB_IF_PROTOCOL_COMM_GNSS      = 0x4D,/*0x4D*/
    /*无Huawei 字样*/
    NO_HW_USB_IF_PROTOCOL_MODEM           = 0x61,/*0x61*/
    NO_HW_USB_IF_PROTOCOL_PCUI            = 0x62,/*0x62*/
    NO_HW_USB_IF_PROTOCOL_DIAG            = 0x63,/*0x63*/
    NO_HW_USB_IF_PROTOCOL_PCSC            = 0x64,/*0x64*/
    NO_HW_USB_IF_PROTOCOL_GPS             = 0x65,/*0x65*/
    NO_HW_USB_IF_PROTOCOL_CTRL            = 0x66,/*0x66*/
    NO_HW_USB_IF_PROTOCOL_NDIS            = 0x67,/*0x67*/
    NO_HW_USB_IF_PROTOCOL_NDISDATA        = 0x68,/*0x68*/
    NO_HW_USB_IF_PROTOCOL_NDISCTRL        = 0x69,/*0x69*/
    NO_HW_USB_IF_PROTOCOL_BLUETOOTH       = 0x6A,/*0x6A*/
    NO_HW_USB_IF_PROTOCOL_FINGERPRINT     = 0x6B,/*0x6B*/
    NO_HW_USB_IF_PROTOCOL_ACMCTRL         = 0x6C,/*0x6C*/
    NO_HW_USB_IF_PROTOCOL_MMS             = 0x6D,/*0x6D*/
    NO_HW_USB_IF_PROTOCOL_PCVOICE         = 0x6E,/*0x6E*/
    NO_HW_USB_IF_PROTOCOL_DVB             = 0x6F,/*0x6F*/
    /*驱动端口名称带3G  无且华为字样*/
    NO_HW_USB_IF_PROTOCOL_3G_PCUI           = 0x72,/*0x72*/
    NO_HW_USB_IF_PROTOCOL_3G_DIAG           = 0x73,/*0x73*/
    NO_HW_USB_IF_PROTOCOL_3G_GPS            = 0x74,/*0x74*/
    NO_HW_USB_IF_PROTOCOL_3G_PCVOICE        = 0x75,/*0x75*/
    NO_HW_USB_IF_PROTOCOL_NCM               = 0x76,/*0x76*/
    NO_HW_USB_IF_PROTOCOL_SHEEL_A           = 0x78,/*0x78*/
    NO_HW_USB_IF_PROTOCOL_SHEEL_B           = 0x79,/*0x79*/
    NO_HW_USB_IF_PROTOCOL_COMM_A            = 0x7A,/*0x7A*/
    NO_HW_USB_IF_PROTOCOL_COMM_B            = 0x7B,/*0x7B*/
    NO_HW_USB_IF_PROTOCOL_COMM_C            = 0x7C,/*0x7C*/
    NO_HW_USB_IF_PROTOCOL_COMM_GNSS         = 0x7D,/*0x7D*/

    USB_IF_PROTOCOL_NOPNP            = 0xFF/*0xFF*/
} USB_PID_UNIFY_IF_PROT_T;

/* 固定端口形态索引*/
typedef enum
{
    CDROM_MODE = 1,                  /*1*/
    PCUI_DIAG_MODE,                  /*2*//*有用*/
    DIAG_PCUI_MODE_EX,               /*3*/  /*升级时用的端口形态*//*有用*/
    MBIM_DOWNLOAD_MODE,              /*4*/  /*有用*/
    PCUI_DIAG_MODE_FACTORY,          /*5*/  /*有用*/
    MBIM_SWITCH_DEBUG_MODE,          /*6*/  /*有用*/
    MBIM_SWITCH_NORMAL_MODE,         /*7*/  /*有用*/
    NCM_PCUI_GPS_4GDIAG_3GDIAG_CPE,  /*8*/  /*有用*/
    RNDIS_DEBUG_MODE,                /*9*/ /*有用*/
    RNDIS_DEBUG_MODE_SD,             /*10*/ /*有用*/
    ECM_DEBUG_MODE,                  /*11*/ /*有用*/
    ECM_DEBUG_MODE_SD,               /*12*/ /*有用*/
    MODEM_DIAG_PCUI_GATEWAY,         /*13*/ /*有用*/
    MODEM_DIAG_PCUI_NCM_GATEWAY,     /*14*/ /*有用*/
    POWER_OFF_MODE,                  /*15*/ /*有用*/
    /**/
    DYNAMIC_REWIND_BEFORE_MODE,      /*16*/ 
    DYNAMIC_REWIND_AFTER_MODE,       /*17*/ 
/*DYNAMIC_DIRECT_PORT_MODE直接上报端口，无切换过程的产品使用*/ 
    DYNAMIC_DIRECT_PORT_MODE,              /*18*/
    DYNAMIC_DEBUG_MODE,              /*28*/
    HILINK_PORT_MODEM_MODE,          /*29*/
    HILINK_PORT_MODEM_DEBUG_MODE,    /*30*/
    INVALID_MODE,                    /*31*/
} ports_enum_mode;

enum port_style
{
    pnp_port_style     = 0,
    rewind_port_style,
    download_port_style,
    factory_port_style,
    charge_port_style,
    direct_port_style,
};

/*  RNDIS work mode */
enum rewind_hilink_mode
{
    product_port_mode = 0,  /*  升级模式  */
    work_port_mode,         /*  工作模式  */
    debug_port_mode,        /*  调试模式  */
};

//光盘SD 支持情况
typedef enum
{
    CDROM_ONLY = 1,
    SD_ONLY,
    CDROM_SD,
    CDROM_SD_INVALID,
} cdrom_sd_style;

/*string描述符index*/
typedef enum 
{
    ACM_MODEM_STRING_IDX = 1,
    ACM_DIAG_STRING_IDX,
    ACM_PCUI_STRING_IDX,
    ACM_GPS_STRING_IDX,
    ACM_CTRL_STRING_IDX ,
    ACM_SERIAL_B_STRING_IDX,
    NCM_STRING_IDX,
    STRING_IAD_MAX,
} huawei_port_string_idx;
/*根据protocol索引string描述符*/
struct acm_string_idx_protocol {
    USB_INT type;//接口协议类型
    huawei_port_string_idx port_string_idx;
};
typedef struct  hw_usb_mode_st
{
    USB_UCHAR   port_index;  /*端口形态索引*/
    USB_CHAR*   mode_buf[DYNAMIC_PID_MAX_PORT_NUM];
    USB_INT     PID;
    USB_INT     bDeviceClass;
    USB_UINT    cdrom;
    USB_UINT    sd;
} hw_usb_mode;

typedef struct  hw_usb_dynamic_mode_st
{
    USB_UCHAR   port_index;  /*端口形态索引*/
    USB_CHAR*   mode_buf[MAX_TOTAL_PORT_NUM];
    USB_UINT8   port_protocol[MAX_TOTAL_PORT_NUM];/*串口*/
    USB_INT     PID;
} pnp_dynamic_mode;

/*rewind 切换命令*/
struct rewind_cmd_param
{
    USB_UINT8 bCmdReserved;
    USB_UINT8 bPcType;
    USB_UINT8 bTimeOut;
    USB_UINT8 bPID;
    USB_UINT8 bNewPID;
    USB_UINT8 bSupportCD;
    USB_UINT8 bProFile;
    USB_UINT8 bGreenMode;
    USB_UINT8 reserved[USB_NUM_7];
};

/*PNP 状态机定义*/
typedef struct huawei_dynamic_info
{
    USB_INT     portModeIndex;
    USB_INT     is_daynamic_sn;
    USB_INT     current_port_style;
    USB_INT     is_service_switch;
    USB_INT     app_start_mode;     // app禁止启动标志
    USB_INT     is_mbim_enable;     // 配置信息中是否包含MBIM端口
    hilink_work_mode     hilink_mode;     // hilink modem模式切换
    struct rewind_cmd_param rewind_param;
} huawei_dynamic_info_st;

typedef struct huawei_interface_info_st
{
    USB_UINT    huawei_interface_protocol_num;
    USB_CHAR*   huawei_interface_name;
} huawei_interface_info;

typedef struct
{
    USB_UINT16 cust_pid;
    USB_CHAR port_name[STRING_IAD_MAX][DYNAMIC_INFO_NAME_CHAR_NUM];
}huawei_port_string;

/*****************************************************************************
KO API 管理begin
*****************************************************************************/
typedef struct android_dev pnp_android_dev;
typedef USB_INT (*adp_pnp_if_cdrom_can_open)(USB_VOID);
typedef USB_VOID (*adp_pnp_set_rewind_param)(USB_UINT8* cmnd);
typedef USB_INT (*adp_pnp_is_service_switch)(USB_VOID);
typedef USB_INT (*adp_pnp_is_rewind_before_mode)(USB_VOID);
typedef USB_INT (*adp_pnp_get_gateway_mode)(USB_VOID);
typedef USB_INT (*adp_pnp_system_type_get)(USB_VOID);
typedef USB_INT (*adp_pnp_if_static_port_mode)(USB_VOID);
typedef USB_INT (*adp_pnp_port_style_stat)(USB_VOID);
typedef USB_VOID*  (*adp_pnp_get_pnp_info)(USB_VOID);
typedef USB_VOID  (*adp_pnp_probe)(USB_VOID);
typedef USB_VOID  (*adp_pnp_remove)(USB_VOID);
typedef USB_VOID  (*adp_pnp_switch_mbim_mode)(USB_INT mode);
typedef USB_VOID  (*adp_pnp_switch_autorun_port)(USB_VOID);
typedef USB_VOID  (*adp_android_disable)(struct android_dev* dev);
typedef USB_INT  (*adp_android_enable)(struct android_dev* dev);
typedef USB_VOID  (*adp_pnp_switch_rewind_after_mode)(USB_VOID);
typedef USB_INT  (*adp_android_create_device)(struct android_dev* dev, u8 usb_core_id);
typedef USB_VOID  (*adp_android_destroy_device)(struct android_dev* dev);
typedef USB_INT  (*adp_pnp_is_multi_lun_mode)( USB_VOID );
typedef USB_VOID (*adp_pnp_set_ctl_app_flag)(USB_INT flag);
typedef USB_INT (*adp_pnp_get_ctl_app_flag)(USB_VOID);

typedef struct _usb_pnp_api
{
    adp_pnp_if_cdrom_can_open          pnp_if_cdrom_can_open_cb;
    adp_pnp_set_rewind_param            pnp_set_rewind_param_cb;
    adp_pnp_switch_rewind_after_mode pnp_switch_rewind_after_mode_cb;
    adp_pnp_is_service_switch             pnp_is_service_switch_cb;
    adp_pnp_is_rewind_before_mode    pnp_is_rewind_before_mode_cb;
    adp_pnp_get_gateway_mode          pnp_get_gateway_mode_cb;
    adp_pnp_system_type_get             pnp_system_type_get_cb;
    adp_pnp_if_static_port_mode         pnp_if_static_port_mode_cb;
    adp_pnp_port_style_stat                pnp_port_style_stat_cb;
    adp_pnp_get_pnp_info                  pnp_get_pnp_info_cb;
    adp_pnp_probe                            pnp_probe_cb;
    adp_pnp_remove                         pnp_remove_cb;
    adp_pnp_remove                         pnp_remove_direct_cb;
    adp_pnp_switch_mbim_mode         pnp_switch_mbim_mode_cb;
    adp_pnp_switch_autorun_port        pnp_switch_autorun_port_cb;
    adp_android_disable                     android_disable_cb;
    adp_android_create_device           android_create_device_cb;
    adp_android_destroy_device         android_destroy_device_cb;
    adp_pnp_is_multi_lun_mode         pnp_is_multi_lun_mode_cb;
    adp_pnp_set_ctl_app_flag           pnp_set_ctl_app_flag_cb;
    adp_pnp_get_ctl_app_flag            pnp_get_ctl_app_flag_cb;
}usb_pnp_api_hanlder;

/*****************************************************************************
KO API 管理end
*****************************************************************************/
#endif

