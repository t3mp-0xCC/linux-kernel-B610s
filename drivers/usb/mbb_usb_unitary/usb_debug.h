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

#ifndef __USB_DEBUG_H__
#define __USB_DEBUG_H__

#include "usb_config.h"
#include <linux/types.h>
#include <linux/kernel.h>
#include "usb_platform_comm.h"
#include "usb_mlog.h"

/*codecheck bug*/
#define UNUSED_VAR(x) (x) = (x)

enum DEBUG_LEVELS
{
    U_TRACE = 0,
    U_ERROR, //���󼶱�
    U_WARN, //���漶��
    U_INFO,  //������Ϣ����
    U_ALL,   //���м���
};

enum DEBUG_MODULES
{
    MBB_ALL       =       0,
    MBB_DEBUG,
    MBB_ACM,
    MBB_NET,
    MBB_RNDIS,
    MBB_ECM,
    MBB_NCM,
    MBB_CHARGER,
    MBB_USB_NV,
    MBB_PNP,
    MBB_EVENT,
    MBB_MASS,
    MBB_OTG_CHARGER,
    MBB_USB_OTG,
    MBB_HOTPLUG,
    MBB_TYPECREG,
    MBB_LAST,
} ;

typedef struct DEBUG_MODE_INFO
{
    enum DEBUG_LEVELS lev;
    const USB_CHAR* module_name;
} debug_mode_info;

#ifdef USB_UT_DEBUG 
#define mbbprintk(module_name, fmt, lev_name,args...) \
    printf( "%s() line:%d : " fmt ,  __FUNCTION__,__LINE__, ## args);

#define mbbdumpprintk(fmt, args...) \
    printf( "%s() line:%d "fmt ,__FUNCTION__,__LINE__, ## args);
#else
#define mbbprintk(module_name, fmt, lev_name,args...) \
    printk( KERN_ERR "%s(%s) %s() line:%d : " fmt , module_name, lev_name, __FUNCTION__,__LINE__, ## args);

#define mbbdumpprintk(fmt, args...) \
    printk( KERN_ERR ""fmt , ## args);
#endif
/*
 *����log��ӡ����
 */
 
/*��ӡ����ʹ�õ�log�����������壬ʹ�ô�log��ӡ������ӡ��log��ʼ�ջᱻ��ӡ*/
//#define DBG_T(type, fmt, args...) PRINT_DBG((type), U_TRACE,fmt, ## args)
#define DBG_T(type, fmt, args...) mbbprintk(debug_bank[(int)(type)].module_name, fmt ,lev_name[U_TRACE], ## args);

/*��ӡerr�����log��ÿ��ģ��Ĭ�ϵ�log����*/
#define DBG_E(type, fmt, args...) PRINT_DBG((type), U_ERROR,fmt, ## args)
/*��ӡwarning�����log*/
#define DBG_W(type, fmt, args...) PRINT_DBG((type), U_WARN,fmt, ## args)
/*��ӡinfo�����log*/
#define DBG_I(type, fmt, args...) PRINT_DBG((type), U_INFO,fmt, ## args)

#ifdef USB_DEBUG
#define PRINT_DBG(type, level, fmt, args...) \
    /*lint -e40 -e58 */ \
    do { \
        if ((MBB_DEBUG) == (type))\
        {\
            mbbdumpprintk(fmt , ## args);\
        }\
        else \
        { \
            mbbprintk(debug_bank[(int)(type)].module_name, fmt ,lev_name[level] ,## args);\
        } \
    } while (0)
#else
#define PRINT_DBG(type, level, fmt, args...) \
    /*lint -e40 -e58 */ \
    do { \
        if ((MBB_DEBUG) == (type))\
        {\
            mbbdumpprintk(fmt , ## args);\
        }\
        else if ((level) <= debug_bank[(int)(type)].lev)\
        { \
            mbbprintk(debug_bank[(int)(type)].module_name, fmt ,lev_name[level], ## args);\
        } \
    } while (0)
/*lint +e40 +e58 */
#endif
void valid_type(enum DEBUG_MODULES type);
#define DBG_SET_LEVEL(type, level) \
    do { \
        valid_type(type); \
        debug_bank[(int)(type)].lev = (level); \
    } while (0)

#define DBG_SET_GLOBAL_LEVEL(level) \
    do { \
        int i; \
        for (i = 0; i < MBB_LAST; i++) \
        {\
            debug_bank[i].lev = (level); \
        }\
    } while(0)

#define DBG_GET_LEVEL(type) debug_bank[(USB_INT)(type)].lev

extern debug_mode_info debug_bank[MBB_LAST];

extern USB_CHAR *lev_name[U_ALL+1];

/*==============================================================================
 *===============================API����=================================
 *============================================================================*/
/****************************************************************
 �� �� ��  : debug_init
 ��������  : debug��ʼ����
 �������  : ��
 �������  : ��
 �� �� ֵ  : NA
*****************************************************************/
USB_VOID usb_debug_init(USB_VOID);
/****************************************************************
 �� �� ��  : debug_init
 ��������  : debug��ʼ����
 �������  : ��
 �������  : ��
 �� �� ֵ  : NA
*****************************************************************/
USB_VOID dbg_lev_set(USB_INT module, USB_INT lev);
/****************************************************************
 �� �� ��  : debug_init
 ��������  : debug��ʼ����
 �������  : ��
 �������  : ��
 �� �� ֵ  : NA
*****************************************************************/
USB_VOID usb_all_module_logs(USB_INT all_or_none);

USB_VOID usb_debug_show_module_lev(USB_VOID);

USB_VOID usb_MemHexDump(const USB_PUINT8 buf, USB_UINT size);

#endif
