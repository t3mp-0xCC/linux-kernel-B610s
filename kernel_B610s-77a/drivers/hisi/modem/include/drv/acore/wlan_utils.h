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
 
#ifndef _wlan_utils_h_
#define _wlan_utils_h_

#include <linux/kernel.h>

#ifdef __cplusplus
    #if __cplusplus
    extern "C" {
    #endif
#endif

/*系统打印接口*/
#define SYS_WLAN_INFO(fmt, ...) (void)pr_warning("Wi-Fi <INFO> [%s:%d]: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define SYS_WLAN_ERR(fmt, ...)  (void)pr_err("Wi-Fi <ERRO> [%s:%d]: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

/*字符串格式化接口*/
#define OSA_SNPRINTF(str, sz, fmt, ...)  snprintf(str, sz, fmt, ##__VA_ARGS__)

/* sizeof方式制定长度的字符串格式化 */
#define SIZEOF_SNPRINTF(str, fmt, ...)   (void)OSA_SNPRINTF(str, sizeof(str), fmt, ##__VA_ARGS__)

/* 数组大小取值 */
#ifdef ARRAY_SIZE
    #undef ARRAY_SIZE
#endif
#define ARRAY_SIZE(x)                 (sizeof(x) / sizeof(x[0]))

/*空指针断言检测*/
#define ASSERT_NULL_POINTER(p, ret)  do { \
    if (NULL == (p)) \
    { \
        PLAT_WLAN_ERR("Exit ret = %d, on NULL Pointer %s", (int)ret, #p); \
        return ret; \
    } \
}while(0)

#ifndef STATIC
#define STATIC static
#endif

#ifndef _TYPEDEFS_H_
    #if !defined(_UINT32_DEFINED) && !defined(TYPEDEF_UINT32)
        typedef  unsigned long int  uint32;      /* Unsigned 32 bit value */
        #define _UINT32_DEFINED
        #define TYPEDEF_UINT32
    #endif

    #if !defined(_UINT16_DEFINED) && !defined(TYPEDEF_UINT16)
        typedef  unsigned short     uint16;      /* Unsigned 16 bit value */
        #define _UINT16_DEFINED
        #define TYPEDEF_UINT16
    #endif

    #if !defined(_UINT8_DEFINED) && !defined(TYPEDEF_UINT8)
        typedef  unsigned char      uint8;       /* Unsigned 8  bit value */
        #define _UINT8_DEFINED
        #define TYPEDEF_UINT8
    #endif

    #if !defined(_INT32_DEFINED) && !defined(TYPEDEF_INT32)
        typedef  signed long int    int32;       /* Signed 32 bit value */
        #define _INT32_DEFINED
        #define TYPEDEF_INT32
    #endif

    #if !defined(_INT16_DEFINED) && !defined(TYPEDEF_INT16)
        typedef  signed short       int16;       /* Signed 16 bit value */
        #define _INT16_DEFINED
        #define TYPEDEF_INT16
    #endif

    #if !defined(_INT8_DEFINED) && !defined(TYPEDEF_INT8)
        typedef  char        int8;               /* Signed 8  bit value */
        #define _INT8_DEFINED
        #define TYPEDEF_INT8
    #endif
#endif /*_TYPEDEFS_H_*/

/* 函数返回值列表 */
typedef enum
{
    WLAN_DRV_NO_LOADED     =  -10, /*驱动未loaded*/
    WLAN_MM_NO_ENOUGH      =  -9,  /*内存不足*/
    WLAN_STATUS_ERROR      =  -8,  /*状态错误*/
    WLAN_NO_RECORD         =  -7,  /*没有记录返回*/
    WLAN_POINTER_NULL      =  -6,  /*指针为 NULL*/
    WLAN_GETRECORD_FAIL    =  -5,  /*获取当前记录失败*/
    WLAN_NO_SUPPORT        =  -4,  /*不支持的参数*/
    WLAN_TIMEOUT           =  -3,  /*配置超时*/
    WLAN_EFFECT_FAILURE    =  -2,  /*配置失效*/
    WLAN_PARA_ERR          =  -1,  /*参数错误*/
    WLAN_SUCCESS           =   0   /*成功*/
}WLAN_RETURN_TYPE;

/* 芯片类型定义，其中: chipstub表示未使用任何芯片 */
#define   chipstub      (0)
#define   bcm43362      (1)
#define   bcm43239      (2)
#define   bcm43241      (3)
#define   bcm43236      (4)
#define   bcm43217      (5)
#define   bcm4354       (6)
#define   bcm4352       (7)
#define   rtl8189       (8)
#define   rtl8192       (9)
#define   bcm4356       (10)
#define   bcm4359       (11)
#define   bcm4360       (12) 
#define   WLAN_CHIP_MAX (13) /* 当前支持的WiFi芯片总数，含chipstub */

/*****************************************************************************
函数名称  : wlan_chip_type2name
功能描述  : 获取芯片名称
输入参数  : chip_type: 芯片类型
输出参数  : NA
返 回 值  : 芯片名称字符串
*****************************************************************************/
const char *wlan_chip_type2name(int chip_type);

/*****************************************************************************
函数名称  : wlan_chip_name2type
功能描述  : 获取芯片类型
输入参数  : chip_name: 芯片名称
输出参数  : NA
返 回 值  : 芯片类型
*****************************************************************************/
int wlan_chip_name2type(const char *chip_name);

/*****************************************************************************
 函数名称  : WLAN_RETURN_TYPE wlan_run_cmd(const char *pcmd)
 功能描述  : 向WiFi芯片下发配置命令
 输入参数  : pcmd:执行的命令
 输出参数  : NA
 返 回 值  : WLAN_RETURN_TYPE
*****************************************************************************/
WLAN_RETURN_TYPE wlan_run_cmd(const char *pcmd);

/*****************************************************************************
 函数名称  : WLAN_RETURN_TYPE wlan_run_shell(const char *pcmd)
 功能描述  : 向WiFi芯片下发shell配置命令
 输入参数  : pcmd:执行的命令
 输出参数  : NA
 返 回 值  : WLAN_RETURN_TYPE
*****************************************************************************/
WLAN_RETURN_TYPE wlan_run_shell(const char *pshell);

/*****************************************************************************
 函数名称  : int wlan_read_file(const char *filename, char *buf, int32 len)
 功能描述  : 读取一行数据
 输入参数  : filp:数据指针，len:缓存大小
 输出参数  : buffer:数据缓存
 返 回 值  : int
*****************************************************************************/
int wlan_read_file(const char *filename, char *buf, int len);

/* 进制类型 */
typedef enum
{
    WLAN_BASE10      =  10,  /* 十进制 */
    WLAN_BASE16      =  16,  /* 十六进制 */
}WLAN_BASE_TYPE;

/*****************************************************************************
 函数名称  : int wlan_strtoi(const char *nptr, char **endptr, WLAN_BASE_TYPE base)
 功能描述  : 字符串转整形数
 输入参数  : nptr:字符串指针，base:转换采用的进制
 输出参数  : endptr:结束位指针
 返 回 值  : int:转换后数值
*****************************************************************************/
int wlan_strtoi(const char *nptr, const char **endptr, WLAN_BASE_TYPE base);

/* 从字符串中解析出整型值 */
#define WLAN_STR_TO_iVALE(str, key, val, type) do { \
    val = (type)strlen(key); \
    key = strstr(str, key); \
    ASSERT_NULL_POINTER(key, WLAN_POINTER_NULL); \
    val = (type)wlan_strtoi((key + (int)val), NULL, WLAN_BASE10); \
} while(0)

/*===========================================================================
                           WiFi log打印，可维可测
    echo "16 setlog:4" > /sys/devices/platform/wifi_at_dev/wifi_at_dev
===========================================================================*/
/* log打印级别 */
typedef enum _wlan_log_leve_type
{
    WLAN_LOG_NULL = 0,              /*  NO_LOG，屏蔽所有的WiFi内核态log打印 */
    WLAN_LOG_KERNEL = 1,            /*  内核适配层WLAN_IF的log打印 */
    WLAN_LOG_DRV_LOW = 2,           /*  WiFi驱动层常规的log打印 */
    WLAN_LOG_DRV_MID = 4,           /*  WiFi驱动层中级的log打印，主要是接入过程的关键log */
    WLAN_LOG_DRV_HI = 8,            /*  WiFi驱动层高级的log打印，主要是接入、数传过程中的详细log */
}WLAN_LOG_LEVE_TYPE;

/* log打印方向 */
typedef enum _wlan_log_sign_type
{
    WLAN_LOG_STDOUT = 0x0001,      /*  输出到标准输入输出设备 */
    WLAN_LOG_HISI_SDT = 0x0002,    /*  输出到海思巴龙的SDT工具 */
    WLAN_LOG_QCN_QXDM = 0x0004,    /*  输出到高通平台的QXDM工具 */
    WLAN_LOG_FILE = 0x1000,        /*  输出到文件，主要用于烧片版本log记录 */
}WLAN_LOG_SIGN_TYPE;

#define WLAN_LOG(sign, leve) (((0xFFFF & sign) << 16) | (0xFFFF & leve)) /* log flag组合 */
#define WLAN_LOG_LEVE (wlan_get_log_flag() & 0xFFFF)           /* log打印级别 */
#define WLAN_LOG_SIGN ((wlan_get_log_flag() >> 16) & 0xFFFF)   /* log输出状态 */

/*****************************************************************************
 函数名称  : wlan_get_log_flag
 功能描述  : 获取log打印标记位
 输入参数  : NA
 输出参数  : NA
 返 回 值  : log打印状态标记
*****************************************************************************/
int wlan_get_log_flag(void);

/*****************************************************************************
 函数名称  : wlan_set_log_flag
 功能描述  : 配置log打印
 输入参数  : NA
 输出参数  : NA
 返 回 值  : log打印参数
*****************************************************************************/
int wlan_set_log_flag(int flag);

/*****************************************************************************
 函数名称  : wlan_mfg_log_fprintf
 功能描述  : 产线WT/MT内核log打印至文件
 输入参数  : filename:文件名
 返 回 值  : int
*****************************************************************************/
void wlan_log_fprintf(const char *fmt, ...);

/* 平台打印接口 */
#define PLAT_PRINTT(leve, fmt, ...) do { \
    /* 打印级别判断 */ \
    if (WLAN_LOG_LEVE < leve) \
    { \
        break; \
    } \
    /* 默认标准输出 */ \
    if ((0 == WLAN_LOG_SIGN) || (WLAN_LOG_STDOUT & WLAN_LOG_SIGN)) \
    { \
        pr_err(fmt, ##__VA_ARGS__); \
    } \
    /* 输出到文件 */ \
    if (WLAN_LOG_FILE & WLAN_LOG_SIGN) \
    { \
        wlan_log_fprintf(fmt, ##__VA_ARGS__); \
    } \
}while(0)
#define PLAT_WLAN_INFO(fmt, ...) \
    PLAT_PRINTT(WLAN_LOG_KERNEL, "Wi-Fi<INFO>[%s:%d]: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define PLAT_WLAN_ERR(fmt, ...)  \
    PLAT_PRINTT(WLAN_LOG_KERNEL, "Wi-Fi<ERRO>[%s:%d]: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

/* 带打印级别控制的打印接口 */
#define PLAT_WLAN_DETAIL(leve, fmt, ...) \
    PLAT_PRINTT(leve, "Wi-Fi<DETAIL%d>[%s:%d]: "fmt"\n", leve, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/* 装备打印接口 */
#ifdef WLAN_TRACE_INFO
    #undef WLAN_TRACE_INFO
#endif
#define WLAN_TRACE_INFO(fmt, ...) PLAT_WLAN_INFO(fmt, ##__VA_ARGS__)
#ifdef WLAN_TRACE_ERROR
    #undef WLAN_TRACE_ERROR
#endif
#define WLAN_TRACE_ERROR(fmt, ...)    PLAT_WLAN_ERR(fmt, ##__VA_ARGS__)

/* lint报错处理 */
#if defined(_lint)
#ifdef memset
    #undef memset
    extern void *memset(void *, int, unsigned);
#endif
#endif

#ifdef __cplusplus
    #if __cplusplus
    }
    #endif
#endif   
#endif /* _wlan_utils_h_ */
