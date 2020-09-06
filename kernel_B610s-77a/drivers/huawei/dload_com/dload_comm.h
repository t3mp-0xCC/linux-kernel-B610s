
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


#ifndef _DLOAD_COMM_H_
#define _DLOAD_COMM_H_
#ifdef  __cplusplus
    extern "C"{
#endif


/****************************************************************************
  2 变量类型重定义
*****************************************************************************/

typedef int                int32;
typedef short              int16;
typedef char               int8;
typedef unsigned long long uint64;
typedef long long          int64;
typedef unsigned long      uint32;
typedef unsigned short     uint16;
typedef unsigned char      uint8;
typedef unsigned char      boolean;

/****************************************************************************
 3 枚举类型，结构体，宏定义
 结构体类型加 :_s
 枚举类型加   :_e
 联合体类型加 :_u
 
****************************************************************************/
#define EXTERN   extern
#define STATIC   static
#define CONST    const

#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define DLOAD_ERR        -1
#define DLOAD_OK          0

#define DLOAD_NAME_MAX_LEN 256

/****************************************************************************
 3 枚举类型，结构体，宏定义
 结构体类型加 :_s
 枚举类型加   :_e
 联合体类型加 :_u
****************************************************************************/
typedef enum
{
    DLOAD_BALONG,  /* 巴龙平台 */
    DLOAD_QCOMM,   /* 高通平台 */
}platform_type_t;

typedef enum
{
    DLOAD_STICK,    /* 数据卡形态 */
    DLOAD_HILINK,   /* 闪电卡形态 */
    DLOAD_WINGLE,   /* wingle形态 */
    DLOAD_E5,       /* 通用E5形态 */
    DLOAD_CPE,      /* CPE形态 */
    DLOAD_MOD_M2M,      /* 模块m2m形态 */
    DLOAD_MOD_CE,      /* 模块ce形态 */
    DLOAD_MOD_CAR,      /* 模块车载形态 */
    DLOAD_UDP,      /* UDP形态 */
}product_type_t;

/* 升级状态类型 */
typedef enum
{
    DLOAD_STATE_NORMAL_IDLE,       /*空闲状态*/
    DLOAD_STATE_NORMAL_RUNNING,    /*正在升级中*/
    DLOAD_STATE_AUTOUG_RUNNING,    /*在线升级中*/
    DLOAD_STATE_NORMAL_LOW_POWER,  /*SD卡升级电池低电*/
    DLOAD_STATE_NORMAL_SUCCESS,    /*SD卡升级成功*/
    DLOAD_STATE_NORMAL_FAIL,       /*SD卡升级，在先升级失败时，LCD提示*/
    DLOAD_STATE_FAIL,              /*升级尝试最大次数后显示失败 或 usb升级解析命令或数据失败，E5 在此两种状态下lcd 显示idle*/
    DLOAD_STATE_INVALID,
    /* 最多支持10个状态 */
}dload_state_t;

typedef struct _PRODUCT_INFO
{
    platform_type_t  platfm_id_e; /* 平台标识 */
    product_type_t   prodct_id_e; /* 产品形态 */
    uint32           hw_major_id; /* 产品硬件主ID */
    uint32           hw_minor_id; /* 产品硬件次ID */
    uint8            platfm_name[DLOAD_NAME_MAX_LEN]; /* 平台名称 */
    uint8            prodct_name[DLOAD_NAME_MAX_LEN]; /* 对外产品名称 */
    uint8            upgrade_id[DLOAD_NAME_MAX_LEN];  /* 产品升级ID */
    uint8            feature_name[DLOAD_NAME_MAX_LEN];/* 产品形态对应的名称 */
}product_info_t;

/* 平台信息结构体 */
typedef struct _PLATFORM_INFO
{
    product_info_t  product_info_st; /* 平台产品信息 */
    struct _PLATFORM_FEATURE
    {
        boolean  led_feature;           /* LED特性 TRUE:支持，FALSE: 不支持 */
        boolean  lcd_feature;           /* LCD特性 TRUE:支持，FALSE: 不支持 */
        boolean  reserved4;             /* 预留 */
        boolean  reserved3;             /* 预留 */
        boolean  reserved2;             /* 预留 */
        boolean  reserved1;             /* 预留 */
    }platform_feature;
}platform_info_t;

/*------Fota升级相关属性定义begin--------*/
/*Fota升级fs文件序号枚举结构,仅支持16种文件格式*/
typedef enum
{
    FSFILE_SYSTEM_CPIO = 0,  /*system.cpio文件序号*/
    FSFILE_APP_CPIO,  /*app.cpio文件序号*/
    FSFILE_WEBUI_CPIO,  /*webui.cpio文件序号*/
    FSFILE_MODEM_CPIO,  /*modem.cpio文件序号*/
    /*其他文件在此扩展*/

    FSFILE_MAX = 16
}dload_fota_fsfile_t;
/*Fota升级fs文件系统类型枚举结构,最大支持4种文件格式*/
typedef enum
{
    FSTYPE_YAFFS2 = 0,  /*yaffs2文件系统类型*/
    FSTYPE_JFFS2 = 1,  /*jffs2文件系统类型*/
    /*其他文件系统类型在此扩展*/

    FSTYPE_MAX = 4
}dload_fota_fstype_def_t;
/*Fota属性定义预留扩展空间*/
#define FOTA_FEATURE_RESERV_NUM  6
/*取值运算宏定义,更多场景可以用下面的宏定义在使用时组合*/
#define FOTA_FSLIST(fsfile)  ((1 << fsfile) & 0xFFFF)
#define FOTA_FSTYPE(fsfile, fstype)  ((fstype << (2 * fsfile)) & 0xFFFFFFFF)
#define FOTA_FSTYPE_MASK(fsfile)  ((0x3 << fsfile) & 0xFFFFFFFF)
#define FOTA_FSLIST_BALONG_COMMON  \
    (FOTA_FSLIST(FSFILE_SYSTEM_CPIO) | FOTA_FSLIST(FSFILE_APP_CPIO) | FOTA_FSLIST(FSFILE_WEBUI_CPIO))
#define FOTA_FSTYPE_BALONG_COMMON  \
    (FOTA_FSTYPE(FSFILE_SYSTEM_CPIO, FSTYPE_YAFFS2) | FOTA_FSTYPE(FSFILE_APP_CPIO, FSTYPE_YAFFS2) \
    | FOTA_FSTYPE(FSFILE_WEBUI_CPIO, FSTYPE_YAFFS2))
#define FOTA_FSLIST_QC_COMMON  \
    (FOTA_FSLIST(FSFILE_SYSTEM_CPIO) | FOTA_FSLIST(FSFILE_APP_CPIO) | FOTA_FSLIST(FSFILE_WEBUI_CPIO) | FOTA_FSLIST(FSFILE_MODEM_CPIO))
#define FOTA_FSTYPE_QC_COMMON  \
    (FOTA_FSTYPE(FSFILE_SYSTEM_CPIO, FSTYPE_YAFFS2) | FOTA_FSTYPE(FSFILE_APP_CPIO, FSTYPE_YAFFS2) \
    | FOTA_FSTYPE(FSFILE_WEBUI_CPIO, FSTYPE_YAFFS2) | FOTA_FSTYPE(FSFILE_MODEM_CPIO, FSTYPE_YAFFS2))
/*Fota升级fs文件list*/
typedef union _DLOAD_FOTA_FSLIST
{
    uint16 fslist_value;
    struct
    {
        uint16 fslist_system_cpio  :1;  /*bit[0] -- system.cpio*/
        uint16 fslist_app_cpio  :1;  /*bit[1] -- app.cpio*/
        uint16 fslist_webui_cpio  :1;  /*bit[2] -- webui.cpio*/
        uint16 fslist_modem_cpio  :1;  /*bit[3] -- modem.cpio*/
        uint16 fslist_reserved  :12;  /*bit[4]-bit[15] -- reserved*/
    }fsfile_list;
}dload_fota_fslist_t;
/*Fota升级fs文件类型,4byte数据,每2bit对应上面一个文件*/
/*每2bit数据存在如下对应关系:
   00 -- yaffs2文件系统
   01 -- jiffs文件系统
   10/11 -- 预留*/
typedef union _DLOAD_FOTA_FSTYPE
{
    uint32 fstype_value;
    struct
    {
        uint32 fstype_system_cpio  :2;  /*bit[0]-bit[1] -- system.cpio*/
        uint32 fstype_app_cpio  :2;  /*bit[2]-bit[3] -- app.cpio*/
        uint32 fstype_webui_cpio  :2;  /*bit[4]-bit[5] -- webui.cpio*/
        uint32 fstype_modem_cpio  :2;  /*bit[6]-bit[7] -- modem.cpio*/
        uint32 fstype_reserved :24;  /*bit[8]-bit[31] -- reserved*/
    }fsfile_type;
}dload_fota_fstype_t;
/*Fota升级属性信息结构体*/
typedef struct _DLOAD_FOTA_FEATURE
{
    dload_fota_fslist_t fota_fslist;  /*fota升级中的fs文件列表*/
    dload_fota_fstype_t fota_fstype;  /*fota升级中的文件类型*/
    uint32 fota_memsize;  /*fota升级需要的内存大小*/
    boolean reserved[FOTA_FEATURE_RESERV_NUM];  /*fota升级预留6个字节以后扩展*/
}dload_fota_feature_t;
/*------Fota升级相关属性定义end--------*/

/* 升级特性配置结构体 */
typedef struct _SAFETY1_3_DLOAD_FEATURE
{
    boolean safe_support;    /* 安全升级v1.3方案支持, TRUE: 支持， FALSE: 不支持 */
    boolean datalock_q;      /* datalock支持，TRUE: 支持跨C版本号升级， FALSE: 不支持  */
    boolean server_sign;     /* 服务器签名，FALSE: 本地签名  TRUE: 服务器签名 */
    boolean en_dver_check;   /* d 版本校验开关 */
}dload_safety1_3_t;
#define  FEATURE_RESERV_NUM   50 - sizeof(dload_fota_feature_t) - sizeof(dload_safety1_3_t)     /* 升级预留特性个数 */
typedef struct _DLOAD_FEATURE
{
    uint32           hw_major_id;        /* 产品硬件主ID */
    uint32           hw_minor_id;        /* 产品硬件次ID */
    boolean          usb_dload;          /* 升级: USB升级 */
    boolean          scard_dload;        /* 升级: SD升级 */
    boolean          online_dload;       /* 升级: 在线升级 */
    boolean          safety_dload;       /* 升级: 安全升级 */
    boolean          hota_dload;         /* 升级: HOTA升级 */
    boolean          fota_dload;         /* 升级: FOTA升级 */
    dload_fota_feature_t  fota_feature;  /*Fota升级相关属性定义*/
	dload_safety1_3_t  safety1_3_dload;    /* 升级: 安全升级1.3升级 */
    boolean          reserved[FEATURE_RESERV_NUM]; /* 预留 */
} dload_feature_t;


/* 共享内存访问枚举定义 */
typedef enum
{
    SET_SMEM_DLOAD_MODE_MAGIC,             /* 设置进入下载模式标志 */
    SET_SMEM_DLOAD_MODE_ZERO,              /* 设置进入下载模式标志清零 */
    GET_SMEM_IS_DLOAD_MODE,                /* 获取是否进入下载模式 */
    SET_SMEM_SCARD_DLOAD_MAGIC,            /* 设置按键进入SD升级标志 */
    SET_SMEM_SCARD_DLOAD_ZERO,             /* 设置按键进入SD升级标志清零 */  
    GET_SMEM_IS_SCARD_DLOAD,               /* 获取是否需要SD升级 */  
    SET_SMEM_SWITCH_PORT_MAGIC,            /* 设置NV自动恢复阶段USB端口形态标志 */
    SET_SMEM_SWITCH_PORT_ZERO,             /* 设置NV自动恢复阶段USB端口形态标志清零 */ 
    GET_SMEM_IS_SWITCH_PORT,               /* 获取是否NV自动恢复阶段USB端口形态已切换 */  
    SET_SMEM_ONL_RESTORE_REBOOT_MAGIC,    /* 设置在线升级NV自动恢复完成后单板需要重启 */
    SET_SMEM_ONL_RESTORE_REBOOT_ZERO,     /* 设置在线升级NV自动恢复完成后单板需要重启清零 */
    GET_SMEM_ONL_IS_RESTORE_REBOOT,       /* 获取在线升级NV自动恢复完成后单板是否需要重启 */
    SET_SMEM_SD_RESTORE_REBOOT_MAGIC,     /* 设置SD升级NV自动恢复完成后单板重启标志 */  
    SET_SMEM_SD_RESTORE_REBOOT_ZERO,      /* 设置SD升级NV自动恢复完成后单板重启标志清零 */  
    GET_SMEM_SD_IS_RESTORE_REBOOT,        /* 获取SD升级NV自动恢复完成后单板是否需要重启 */  
    SET_SMEM_FORCE_MODE_MAGIC,             /* 设置进入逃生模式标志 */
    SET_SMEM_FORCE_MODE_ZERO,              /* 设置进入逃生模式标志清零 */  
    SET_SMEM_FORCE_SUCCESS_MAGIC,          /* 设置逃生模式升级成功标志 */  
    GET_SMEM_IS_FORCE_MODE,                 /* 获取是否进入逃生模式 */  
    SET_SMEM_EXCEP_REBOOT_INTO_ONL_MAGIC, /* 设置异常重启多次进入在线升级标志 */
    SET_SMEM_EXCEP_REBOOT_INTO_ONL_ZERO,  /* 设置异常重启多次进入在线升级标志清零 */  
    GET_SMEM_IS_EXCEP_REBOOT_INTO_ONL,    /* 获取异常重启多次是否进入在线升级 */  
    SET_SMEM_UT_DLOAD_MAGIC,                /* 设置多台加载升级标志 */  
    SET_SMEM_UT_DLOAD_ZERO,                 /* 设置多台加载升级标志清零 */  
    GET_SMEM_IS_UT_DLOAD,                   /* 获取是否是多台加载升级 */ 
    SET_SMEM_MULTIUPG_DLOAD_MAGIC,         /* 设置组播升级标志 */
    GET_SMEM_DATALOCK_STATUS_MAGIC,         /* 获取datalock是否解锁标记 */
    SET_SMEM_DATALOCK_MODE_ZERO,            /* 设置datalock状态标记 */
}smem_visit_t;

/*************************************************显示模块公共信息************************************************/
#define STR_MAX_LEN            (64)
#define PATH_MAX_LEN           (256)

/* 状态动作指示 */
typedef enum
{
    LED_READ,
    LED_WRITE,
    LCD_READ,
    LCD_WRITE,
}oper_mode_t;

/* 路径标识码映射结构 */
typedef struct
{
    uint32  path_index;
    char    path_content[PATH_MAX_LEN];
}path_info_t;

/* 升级 lcd/led 运行状态数据结构 */
typedef struct
{
    uint32        up_state;
    oper_mode_t   oper_mode;
    union
    {
        uint32    path_index;
        int32     num_value;
        char      str_value[STR_MAX_LEN];
    };
    uint32        sleep_len;
}upstate_info_t;

#ifdef  __cplusplus
    }
#endif

#endif /* _DLOAD_COMM_H_ */
