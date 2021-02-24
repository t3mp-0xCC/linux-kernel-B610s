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

/*ϵͳ��ӡ�ӿ�*/
#define SYS_WLAN_INFO(fmt, ...) (void)pr_warning("Wi-Fi <INFO> [%s:%d]: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define SYS_WLAN_ERR(fmt, ...)  (void)pr_err("Wi-Fi <ERRO> [%s:%d]: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

/*�ַ�����ʽ���ӿ�*/
#define OSA_SNPRINTF(str, sz, fmt, ...)  snprintf(str, sz, fmt, ##__VA_ARGS__)

/* sizeof��ʽ�ƶ����ȵ��ַ�����ʽ�� */
#define SIZEOF_SNPRINTF(str, fmt, ...)   (void)OSA_SNPRINTF(str, sizeof(str), fmt, ##__VA_ARGS__)

/* �����Сȡֵ */
#ifdef ARRAY_SIZE
    #undef ARRAY_SIZE
#endif
#define ARRAY_SIZE(x)                 (sizeof(x) / sizeof(x[0]))

/*��ָ����Լ��*/
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

/* ��������ֵ�б� */
typedef enum
{
    WLAN_DRV_NO_LOADED     =  -10, /*����δloaded*/
    WLAN_MM_NO_ENOUGH      =  -9,  /*�ڴ治��*/
    WLAN_STATUS_ERROR      =  -8,  /*״̬����*/
    WLAN_NO_RECORD         =  -7,  /*û�м�¼����*/
    WLAN_POINTER_NULL      =  -6,  /*ָ��Ϊ NULL*/
    WLAN_GETRECORD_FAIL    =  -5,  /*��ȡ��ǰ��¼ʧ��*/
    WLAN_NO_SUPPORT        =  -4,  /*��֧�ֵĲ���*/
    WLAN_TIMEOUT           =  -3,  /*���ó�ʱ*/
    WLAN_EFFECT_FAILURE    =  -2,  /*����ʧЧ*/
    WLAN_PARA_ERR          =  -1,  /*��������*/
    WLAN_SUCCESS           =   0   /*�ɹ�*/
}WLAN_RETURN_TYPE;

/* оƬ���Ͷ��壬����: chipstub��ʾδʹ���κ�оƬ */
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
#define   WLAN_CHIP_MAX (13) /* ��ǰ֧�ֵ�WiFiоƬ��������chipstub */

/*****************************************************************************
��������  : wlan_chip_type2name
��������  : ��ȡоƬ����
�������  : chip_type: оƬ����
�������  : NA
�� �� ֵ  : оƬ�����ַ���
*****************************************************************************/
const char *wlan_chip_type2name(int chip_type);

/*****************************************************************************
��������  : wlan_chip_name2type
��������  : ��ȡоƬ����
�������  : chip_name: оƬ����
�������  : NA
�� �� ֵ  : оƬ����
*****************************************************************************/
int wlan_chip_name2type(const char *chip_name);

/*****************************************************************************
 ��������  : WLAN_RETURN_TYPE wlan_run_cmd(const char *pcmd)
 ��������  : ��WiFiоƬ�·���������
 �������  : pcmd:ִ�е�����
 �������  : NA
 �� �� ֵ  : WLAN_RETURN_TYPE
*****************************************************************************/
WLAN_RETURN_TYPE wlan_run_cmd(const char *pcmd);

/*****************************************************************************
 ��������  : WLAN_RETURN_TYPE wlan_run_shell(const char *pcmd)
 ��������  : ��WiFiоƬ�·�shell��������
 �������  : pcmd:ִ�е�����
 �������  : NA
 �� �� ֵ  : WLAN_RETURN_TYPE
*****************************************************************************/
WLAN_RETURN_TYPE wlan_run_shell(const char *pshell);

/*****************************************************************************
 ��������  : int wlan_read_file(const char *filename, char *buf, int32 len)
 ��������  : ��ȡһ������
 �������  : filp:����ָ�룬len:�����С
 �������  : buffer:���ݻ���
 �� �� ֵ  : int
*****************************************************************************/
int wlan_read_file(const char *filename, char *buf, int len);

/* �������� */
typedef enum
{
    WLAN_BASE10      =  10,  /* ʮ���� */
    WLAN_BASE16      =  16,  /* ʮ������ */
}WLAN_BASE_TYPE;

/*****************************************************************************
 ��������  : int wlan_strtoi(const char *nptr, char **endptr, WLAN_BASE_TYPE base)
 ��������  : �ַ���ת������
 �������  : nptr:�ַ���ָ�룬base:ת�����õĽ���
 �������  : endptr:����λָ��
 �� �� ֵ  : int:ת������ֵ
*****************************************************************************/
int wlan_strtoi(const char *nptr, const char **endptr, WLAN_BASE_TYPE base);

/* ���ַ����н���������ֵ */
#define WLAN_STR_TO_iVALE(str, key, val, type) do { \
    val = (type)strlen(key); \
    key = strstr(str, key); \
    ASSERT_NULL_POINTER(key, WLAN_POINTER_NULL); \
    val = (type)wlan_strtoi((key + (int)val), NULL, WLAN_BASE10); \
} while(0)

/*===========================================================================
                           WiFi log��ӡ����ά�ɲ�
    echo "16 setlog:4" > /sys/devices/platform/wifi_at_dev/wifi_at_dev
===========================================================================*/
/* log��ӡ���� */
typedef enum _wlan_log_leve_type
{
    WLAN_LOG_NULL = 0,              /*  NO_LOG���������е�WiFi�ں�̬log��ӡ */
    WLAN_LOG_KERNEL = 1,            /*  �ں������WLAN_IF��log��ӡ */
    WLAN_LOG_DRV_LOW = 2,           /*  WiFi�����㳣���log��ӡ */
    WLAN_LOG_DRV_MID = 4,           /*  WiFi�������м���log��ӡ����Ҫ�ǽ�����̵Ĺؼ�log */
    WLAN_LOG_DRV_HI = 8,            /*  WiFi������߼���log��ӡ����Ҫ�ǽ��롢���������е���ϸlog */
}WLAN_LOG_LEVE_TYPE;

/* log��ӡ���� */
typedef enum _wlan_log_sign_type
{
    WLAN_LOG_STDOUT = 0x0001,      /*  �������׼��������豸 */
    WLAN_LOG_HISI_SDT = 0x0002,    /*  �������˼������SDT���� */
    WLAN_LOG_QCN_QXDM = 0x0004,    /*  �������ͨƽ̨��QXDM���� */
    WLAN_LOG_FILE = 0x1000,        /*  ������ļ�����Ҫ������Ƭ�汾log��¼ */
}WLAN_LOG_SIGN_TYPE;

#define WLAN_LOG(sign, leve) (((0xFFFF & sign) << 16) | (0xFFFF & leve)) /* log flag��� */
#define WLAN_LOG_LEVE (wlan_get_log_flag() & 0xFFFF)           /* log��ӡ���� */
#define WLAN_LOG_SIGN ((wlan_get_log_flag() >> 16) & 0xFFFF)   /* log���״̬ */

/*****************************************************************************
 ��������  : wlan_get_log_flag
 ��������  : ��ȡlog��ӡ���λ
 �������  : NA
 �������  : NA
 �� �� ֵ  : log��ӡ״̬���
*****************************************************************************/
int wlan_get_log_flag(void);

/*****************************************************************************
 ��������  : wlan_set_log_flag
 ��������  : ����log��ӡ
 �������  : NA
 �������  : NA
 �� �� ֵ  : log��ӡ����
*****************************************************************************/
int wlan_set_log_flag(int flag);

/*****************************************************************************
 ��������  : wlan_mfg_log_fprintf
 ��������  : ����WT/MT�ں�log��ӡ���ļ�
 �������  : filename:�ļ���
 �� �� ֵ  : int
*****************************************************************************/
void wlan_log_fprintf(const char *fmt, ...);

/* ƽ̨��ӡ�ӿ� */
#define PLAT_PRINTT(leve, fmt, ...) do { \
    /* ��ӡ�����ж� */ \
    if (WLAN_LOG_LEVE < leve) \
    { \
        break; \
    } \
    /* Ĭ�ϱ�׼��� */ \
    if ((0 == WLAN_LOG_SIGN) || (WLAN_LOG_STDOUT & WLAN_LOG_SIGN)) \
    { \
        pr_err(fmt, ##__VA_ARGS__); \
    } \
    /* ������ļ� */ \
    if (WLAN_LOG_FILE & WLAN_LOG_SIGN) \
    { \
        wlan_log_fprintf(fmt, ##__VA_ARGS__); \
    } \
}while(0)
#define PLAT_WLAN_INFO(fmt, ...) \
    PLAT_PRINTT(WLAN_LOG_KERNEL, "Wi-Fi<INFO>[%s:%d]: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define PLAT_WLAN_ERR(fmt, ...)  \
    PLAT_PRINTT(WLAN_LOG_KERNEL, "Wi-Fi<ERRO>[%s:%d]: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

/* ����ӡ������ƵĴ�ӡ�ӿ� */
#define PLAT_WLAN_DETAIL(leve, fmt, ...) \
    PLAT_PRINTT(leve, "Wi-Fi<DETAIL%d>[%s:%d]: "fmt"\n", leve, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/* װ����ӡ�ӿ� */
#ifdef WLAN_TRACE_INFO
    #undef WLAN_TRACE_INFO
#endif
#define WLAN_TRACE_INFO(fmt, ...) PLAT_WLAN_INFO(fmt, ##__VA_ARGS__)
#ifdef WLAN_TRACE_ERROR
    #undef WLAN_TRACE_ERROR
#endif
#define WLAN_TRACE_ERROR(fmt, ...)    PLAT_WLAN_ERR(fmt, ##__VA_ARGS__)

/* lint������ */
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
