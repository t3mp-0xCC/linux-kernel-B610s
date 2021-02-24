 /*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

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

#include <linux/leds.h>
#include <linux/mutex.h>
#include <hi_dr.h>
#include <drv_nv_def.h>
#include <drv_nv_id.h>
#include <mdrv_leds.h>
#include <bsp_om.h>
#include <bsp_dr.h>
#include <bsp_regulator.h>

/* LED id */
#define LED1_ID     0
#define LED2_ID     1
#define LED3_ID     2
#define LED4_ID     3
#define LED5_ID     4
#define LED_ID_MIN  LED1_ID
#define LED_ID_MAX  LED5_ID

/* LED name, for balong_ledX, X must not be less than LED_ID_MIN and bigger than LED_ID_MAX */
#define LED1_NAME    H6551_DR1_NM   /* 这里的名称必须与regulator的name一致，应使用宏定义 */
#define LED2_NAME    H6551_DR2_NM
#define LED3_NAME    H6551_DR3_NM
#define LED4_NAME    H6551_DR4_NM
#define LED5_NAME    H6551_DR5_NM

/* trigger name */
#define LED_TRIGGER_TIMER       "timer"     /* defined in timer_led_trigger, ledtrig-timer.c */
#define LED_TRIGGER_BREATH      "breath"    /* defined in breath_led_trigger, ledtrig-breath.c */

/* 用于sysfs系统显示呼吸模式参数时，最大输出字符串的长度 */
#define LED_SYSFS_MAX_STR_LENTH   8

/* some important structure */
struct balong_led_platdata {
    char *name;
    unsigned long full_on, full_off, fade_on, fade_off;
    int	(*led_breath_set)(struct led_classdev *led_cdev, unsigned long *full_on, unsigned long *full_off, 
        unsigned long *fade_on, unsigned long *fade_off);
    struct regulator *pregulator;
	struct mutex		mlock;
};

struct balong_led_device{
     struct led_classdev cdev;
     struct balong_led_platdata *pdata;
#if ( FEATURE_ON == MBB_LED_DR )
    struct work_struct work;     /* 亮灭操作需要使用工作队列完成 */
    int    brightness;           /* 需要被设置的亮度 */
    unsigned dr;
    unsigned default_brightness; /* 默认的亮度 */
#endif
};

/* 平台中LED_RED对应的DR1管脚、LED_GREEN对应DR2管脚*/
#if(FEATURE_ON == MBB_FEATURE_M2M_LED)
enum LED_COLOR_WHOLE
{
    LED_NULL = 0x0,
    LED_RED = 0x1,                      /*DR1,对应产品板LED1管脚*/
    LED_GREEN = 0x2,                    /*DR2,对应产品板LED2管脚*/
    LED_BLUE = 0x4,                     /*DR3,产品板暂无LED3管脚*/
    LED_YELLOW = 0x3,                   /*对应DR1、DR2管脚*/
    LED_PURPLE = 0x5,                   /*对应DR1、DR3管脚*/
    LED_CYAN = 0x6,                     /*对应DR2、DR3管脚*/
    LED_WHITE = 0x7,                    /*对应DR1、DR2、DR3管脚*/
};
#else

/******************************************************************************************/
/* 三色灯相关的宏 */

/* 颜色值,0位表示蓝,1位表示绿,2位表示红,其他位不用
青为蓝+绿,黄为红+绿,紫为红+蓝*/

enum LED_COLOR
{
    LED_NULL    = 0x0,
    LED_BLUE    = 0x1,
    LED_GREEN   = 0x2,
    LED_CYAN    = 0x3,
    LED_RED     = 0x4,
    LED_PURPLE  = 0x5,
    LED_YELLOW  = 0x6,
    LED_WHITE   = 0x7
};
#endif

#if(FEATURE_ON == MBB_FEATURE_M2M_LED)
#define  LED_NV_VALID                   0
#define  LED_NV_INVALID                 1


enum DRV_M2M_LED_MODE_ENUM
{
    DRV_M2M_LED_MODE_CLOSED  = 0,                                                /* 禁用闪灯 */
    DRV_M2M_LED_MODE_DEFAULT = 1,                                                /* 华为默认闪灯方案 */
    DRV_M2M_LED_MODE_USER    = 2,                                                /* 用户自定义闪灯方案 */
    DRV_M2M_LED_MODE_BUTT
};

/* 模块通用规格的闪灯服务状态 */
enum M2M_LED_STATUS
{
    DRV_M2M_LED_FLIGHT_MODE             = 0,            /* 飞行模式 */
    DRV_M2M_LED_SYSTEM_INITIAL          = 1,            /* 上电初始化 */
    DRV_M2M_LED_OFFLINE_UNREGISTERED    = 2,            /* 注册失败，无服务 */
    DRV_M2M_LED_OFFLINE_NO_SERVICE      = 3,            /* 掉网，无服务 */
    DRV_M2M_LED_GSM_REGISTERED          = 4,            /* GSM注册成功 */
    DRV_M2M_LED_GSM_BEAR_SETUP          = 5,            /* GSM下无线承载建立成功 */
    DRV_M2M_LED_GSM_TRANSMITING         = 6,            /* GSM下数传 */
    DRV_M2M_LED_WCDMA_REGISTERED        = 7,            /* WCDMA注册成功 */
    DRV_M2M_LED_WCDMA_BEAR_SETUP        = 8,            /* WCDMA下无线承载建立成功 */
    DRV_M2M_LED_WCDMA_TRANSMITING       = 9,            /* WCDMA下数传 */
    DRV_M2M_LED_HSPA_BEAR_SETUP         = 10,           /* HSPA下无线承载建立成功 */
    DRV_M2M_LED_HSPA_TRANSMITING        = 11,           /* HSPA下数传 */
    DRV_M2M_LED_LTE_REGISTERED          = 12,            /* WCDMA注册成功 */
    DRV_M2M_LED_LTE_BEAR_SETUP          = 13,            /* WCDMA下无线承载建立成功 */
    DRV_M2M_LED_LTE_TRANSMITING         = 14,            /* WCDMA下数传 */

    DRV_M2M_LED_NORMAL_UPDATING,                        /* 升级状态，新定义服务状态加到升级服务状态前 */
    
    DRV_M2M_LED_STATE_MAX = 32,
}; /*the state of M2M LED */

#define NV_LED_FLICKER_DEFAULT_TIME                            (1)
#define NV_LED_FLICKER_MAX_TIME                                (100)
#define NV_LED_FLICKER_MAX_NUM                                 (2)
#define NV_LED_SERVICE_STATE_NUM                               (15)
/* 模块定制闪灯服务状态 */
enum CE_LED_STATUS
{
    DRV_CE_LED_UNRF            = 0,            /* 关闭RF*/
    DRV_CE_LED_RF         = 1,            /* 开启RF*/
  
    DRV_CE_LED_STATE_MAX = 32,
}; /*the state of CELED */
#endif

#if (FEATURE_ON == MBB_WPG_LED_FLASH)
enum
{
    DEF = 0,                /*0:默认闪灯枚举*/
    HONGKONG,               /*1:香港PCCW闪灯枚举*/
    RUSSIA,                 /*2:俄罗斯MTS闪灯枚举*/
    VODAFONE,               /*3:vodafone闪灯枚举*/   
    CHINAUNION,             /*4:中国联通闪灯枚举*/
                            /*5:预留值*/
    SFR = 6,                /*6:法国SFR闪灯枚举*/
    BSNL,                   /*7:印度BSNL闪灯枚举*/
    KPN,                    /*8:荷兰KPN闪灯枚举*/
    SOFTBANK,               /*9:日本软银闪灯枚举*/
    EMOBILE                /*10:日本Emobile闪灯枚举*/
};
#endif

/* 三色灯默认状态序列*/
#define LED_GREEN_LIGHTING_DOUBLE               {{LED_GREEN, 1},{LED_NULL,1},{LED_GREEN, 1},{LED_NULL, 17},{0,0}}/*绿灯双闪*/
#define LED_GREEN_LIGHTING_SIGNAL               {{LED_GREEN, 1},{LED_NULL, 19},{0,0}}/*绿灯单闪*/
#define LED_GREEN_LIGHTING_FREQUENTLY           {{LED_GREEN, 1},{LED_NULL,1},{0,0}}/*绿灯频闪*/
#define LED_GREEN_LIGHTING_ALWAYS               {{LED_GREEN, 1},{0,0}}/*绿灯常亮*/

#define LED_WHITE_LIGHTING_FREQUENTLY           {{LED_WHITE, 1},{LED_NULL,1},{0,0}}/*白灯频闪*/

#define LED_BULE_LIGHTING_SIGNAL                {{LED_BLUE, 1},{LED_NULL, 19},{0,0}}/*蓝灯单闪*/
#define LED_BLUE_LIGHTING_FREQUENTLY            {{LED_BLUE, 1},{LED_NULL,1},{0,0}}/*蓝灯频闪*/
#define LED_BLUE_LIGHTING_ALWAYS                {{LED_BLUE, 1},{0,0}}/*蓝灯常亮*/

#define LED_CYAN_LIGHTING_SIGNAL                {{LED_CYAN, 1},{LED_NULL, 19},{0,0}}/*青灯单闪*/
#define LED_CYAN_LIGHTING_ALWAYS                {{LED_CYAN, 1},{0,0}}/*青灯常亮*/

#define LED_BLUE_GREEN_ALTERNATE                {{LED_BLUE, 1},{LED_NULL,1},{LED_GREEN, 1},{LED_NULL, 17},{0,0}}/*蓝绿交替双闪*/  

#define LED_RED_LIGHTING_DOUBLE                 {{LED_RED, 1},{LED_NULL,1},{LED_RED, 1},{LED_NULL, 17},{0,0}}/*红灯双闪*/
#define LED_RED_LIGHTING_SIGNAL                 {{LED_RED, 1},{LED_NULL, 19},{0,0}}/*红灯单闪*/
#define LED_RED_LIGHTING_ALWAYS                 {{LED_RED, 1},{0,0}}/*红灯常亮*/

#define LED_SHUTDOWN_DEFINE                     {{LED_NULL, 1},{0,0}}/*灯灭*/

#if(FEATURE_ON == MBB_FEATURE_M2M_LED)
#define LED_DR1_LIGHTING_SIGNAL       {{LED_RED, 1},{LED_NULL,19},{0,0}}    /*LED1单闪,对应DR1管脚*/
#define LED_DR1_LIGHTING_DOUBLE       {{LED_RED, 1},{LED_NULL, 1},{LED_RED, 1},{LED_NULL, 17},{0,0}}    /*LED1双闪，对应DR1管脚*/
#define LED_DR1_LIGHTING_ALWAYS       {{LED_RED, 1},{0,0}}                  /*LED1常亮，对应DR1管脚*/
#define LED_DR1_LIGHTING_FREQUENTLY   {{LED_RED, 1},{LED_NULL, 1},{0,0}}  /*LED1频闪，对应DR1管脚*/
#endif

#if (FEATURE_ON == MBB_WPG_LED_FLASH)
#define LED_PURPLE_LIGHTING_SIGNAL                {{LED_PURPLE, 1},{LED_NULL, 19},{0,0}}/*紫灯单闪*/
#define LED_PURPLE_LIGHTING_ALWAYS                {{LED_PURPLE, 1},{0,0}}/*紫灯常亮*/
#endif

/* some macro */
#define LED_NV_ALREADY_READ                     (0x0F0F0F0F)  /* nv already read flag */
#define MNTN_LED_STATUS_FLUSH                   (0x1)
#define MNTN_LED_TIMER_OCCURE                   (0x2)

#define RED_ON                                  led_on(LED_COLOR_RED)
#define RED_OFF                                 led_off(LED_COLOR_RED)
#define GREEN_ON                                led_on(LED_COLOR_GREEN)
#define GREEN_OFF                               led_off(LED_COLOR_GREEN)
#define BLUE_ON                                 led_on(LED_COLOR_BLUE)
#define BLUE_OFF                                led_off(LED_COLOR_BLUE)
#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
#define M2M_LED_ON                              led_on(LED1_ID - 1)
#define M2M_LED_OFF                             led_off(LED1_ID - 1)
#endif /*FEATURE_ON == MBB_FEATURE_M2M_LED*/

 /* 三色灯闪灯状态序列个数*/
#define LED_CONFIG_MAX_LTE 10

#if (FEATURE_ON == MBB_WPG_LED_FLASH)
#define LED_OM_CONFIG {LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_SIGNAL,\
                                LED_BULE_LIGHTING_SIGNAL,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_WHITE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_BLUE_GREEN_ALTERNATE,\
                                LED_BLUE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_CYAN_LIGHTING_SIGNAL,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_GREEN_LIGHTING_DOUBLE, \
                                LED_GREEN_LIGHTING_DOUBLE}

#define LED_HONGKONG_CONFIG {LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_SIGNAL,\
                                LED_BULE_LIGHTING_SIGNAL,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_WHITE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_BLUE_GREEN_ALTERNATE,\
                                LED_BLUE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_CYAN_LIGHTING_SIGNAL,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_DOUBLE}

#define LED_RUSSIA_CONFIG {LED_RED_LIGHTING_DOUBLE,\
                                LED_RED_LIGHTING_DOUBLE,\
                                LED_RED_LIGHTING_SIGNAL,\
                                LED_BULE_LIGHTING_SIGNAL,\
                                LED_RED_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_RED_LIGHTING_DOUBLE,\
                                LED_WHITE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_BLUE_GREEN_ALTERNATE,\
                                LED_BLUE_LIGHTING_FREQUENTLY,\
                                LED_RED_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_CYAN_LIGHTING_SIGNAL,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_RED_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_DOUBLE}

#define LED_VODAFONE_CONFIG {LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_SIGNAL,\
                                LED_BULE_LIGHTING_SIGNAL,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_WHITE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_BLUE_GREEN_ALTERNATE,\
                                LED_BLUE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_PURPLE_LIGHTING_SIGNAL,\
                                LED_PURPLE_LIGHTING_ALWAYS,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_DOUBLE}

#define LED_CHINAUNION_CONFIG {LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_SIGNAL,\
                                LED_BULE_LIGHTING_SIGNAL,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_WHITE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_BLUE_GREEN_ALTERNATE,\
                                LED_BLUE_LIGHTING_FREQUENTLY,\
                                LED_RED_LIGHTING_ALWAYS,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_CYAN_LIGHTING_SIGNAL,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_RED_LIGHTING_ALWAYS,\
                                LED_GREEN_LIGHTING_DOUBLE}

#define LED_FRANCE_CONFIG {LED_RED_LIGHTING_ALWAYS,\
                                LED_RED_LIGHTING_ALWAYS,\
                                LED_GREEN_LIGHTING_SIGNAL,\
                                LED_BULE_LIGHTING_SIGNAL,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_RED_LIGHTING_ALWAYS,\
                                LED_WHITE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_BLUE_GREEN_ALTERNATE,\
                                LED_BLUE_LIGHTING_FREQUENTLY,\
                                LED_RED_LIGHTING_ALWAYS,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_RED_LIGHTING_SIGNAL,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_CYAN_LIGHTING_SIGNAL,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_RED_LIGHTING_ALWAYS,\
                                LED_GREEN_LIGHTING_DOUBLE}

#define LED_INDIA_CONFIG {LED_RED_LIGHTING_DOUBLE,\
                                LED_RED_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_SIGNAL,\
                                LED_BULE_LIGHTING_SIGNAL,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_RED_LIGHTING_DOUBLE,\
                                LED_WHITE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_BLUE_GREEN_ALTERNATE,\
                                LED_BLUE_LIGHTING_FREQUENTLY,\
                                LED_RED_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_CYAN_LIGHTING_SIGNAL,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_RED_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_DOUBLE}

#define LED_HOLAND_CONFIG {LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_SIGNAL,\
                                LED_CYAN_LIGHTING_SIGNAL,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_WHITE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_BLUE_GREEN_ALTERNATE,\
                                LED_BLUE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_BULE_LIGHTING_SIGNAL,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_DOUBLE}

#define LED_JAPAN_SOFTBANK_CONFIG
#define LED_JAPAN_EMOBILE_CONFIG

#else
#define LED_OM_CONFIG {LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_GREEN_LIGHTING_SIGNAL,\
                                LED_BULE_LIGHTING_SIGNAL,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_CYAN_LIGHTING_ALWAYS,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_WHITE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_ALWAYS,\
                                LED_BLUE_LIGHTING_ALWAYS,\
                                LED_BLUE_GREEN_ALTERNATE,\
                                LED_BLUE_LIGHTING_FREQUENTLY,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_GREEN_LIGHTING_DOUBLE,\
                                LED_SHUTDOWN_DEFINE,\
                                LED_CYAN_LIGHTING_SIGNAL,\
                                LED_GREEN_LIGHTING_DOUBLE}
#endif
#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
#define M2M_LED_DEFAULT_CONFIG {LED_SHUTDOWN_DEFINE,\
                                LED_DR1_LIGHTING_DOUBLE,\
                                LED_DR1_LIGHTING_DOUBLE,\
                                LED_DR1_LIGHTING_DOUBLE,\
                                LED_DR1_LIGHTING_SIGNAL,\
                                LED_DR1_LIGHTING_ALWAYS,\
                                LED_DR1_LIGHTING_ALWAYS,\
                                LED_DR1_LIGHTING_SIGNAL,\
                                LED_DR1_LIGHTING_ALWAYS,\
                                LED_DR1_LIGHTING_ALWAYS,\
                                LED_DR1_LIGHTING_ALWAYS,\
                                LED_DR1_LIGHTING_ALWAYS,\
                                LED_DR1_LIGHTING_SIGNAL,\
                                LED_DR1_LIGHTING_ALWAYS,\
                                LED_DR1_LIGHTING_ALWAYS,\
                                LED_DR1_LIGHTING_FREQUENTLY}

#define CE_LED_DEFAULT_CONFIG {LED_SHUTDOWN_DEFINE,\
                                LED_RED_LIGHTING_ALWAYS}
#endif

/* 三色灯当前状态参数*/
struct led_param
{
	unsigned char led_state_id;     /* 三色灯状态，对应LED_STATUS */
	unsigned char led_config_id;    /* 闪灯状态序列值 */
	unsigned char led_color_id;     /* 三色灯颜色，对应LED_COLOR的值 */
	unsigned char led_time;         /* 该配置持续的时间长度，单位100ms */
};

/*****************************************************************************
 结构名    : LED_CONTROL_NV_STRU
 结构说明  : LED_CONTROL_NV结构 ID=7
*****************************************************************************/
typedef struct
{
    unsigned char   ucLedColor;      /*三色灯颜色，对应LED_COLOR的值*/
    unsigned char   ucTimeLength;    /*该配置持续的时间长度，单位100ms*/
}LED_CONTROL_NV_STRU;

#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
#define NV_GU_LED_SERVICE_STATE_NUM                            (12)
#define NV_LTE_LED_SERVICE_STATE_NUM                           (3)
#define NV_LED_SERVICE_STATE_NUM                               (15)


typedef struct
{
    LED_CONTROL_NV_STRU   stLED[4];                                                 /* 一个闪灯周期内，至多两次闪烁 */
}M2M_LED_CONTROL_STRU;


typedef struct
{
    BSP_U8                              ucLedMode;                                  /* 闪灯模式 */
    BSP_U8                              unresolved[3];                              /* 保留 */
    M2M_LED_CONTROL_STRU                stLedStr[NV_GU_LED_SERVICE_STATE_NUM];      /* 闪灯配置 */
}NV_LED_SET_PARA_STRU;


typedef struct
{
    M2M_LED_CONTROL_STRU                stLedStr[NV_LTE_LED_SERVICE_STATE_NUM];     /* 闪灯配置 */
}NV_LED_SET_PARA_STRU_EXPAND;


typedef struct
{
    BSP_U8                              ucLedMode;                                  /* 闪灯模式 */
    BSP_U8                              unresolved[3];                              /* 保留 */
    M2M_LED_CONTROL_STRU                stLedStr[NV_LED_SERVICE_STATE_NUM];         /* 闪灯配置 */
}NV_LED_SET_PARA_STRU_COMBINED;
#endif

/* led nv结构体，保存nv和已读标记 */
struct nv_led{
    LED_CONTROL_NV_STRU g_led_state_str_om[LED_LIGHT_STATE_MAX][LED_CONFIG_MAX_LTE];
    int g_already_read;
};

struct led_tled_arg
{
    unsigned char ctl;
    unsigned char new_state;
};
#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
BSP_VOID BSP_M2M_LedInit(void);
#endif

#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
BSP_VOID BSP_CombineLedNVCtrlPara(
    NV_LED_SET_PARA_STRU               *pstLedNvStru,
    NV_LED_SET_PARA_STRU_EXPAND        *pstLedNvExStru,
    NV_LED_SET_PARA_STRU_COMBINED      *pstLedNvCombinedStru
);

BSP_VOID BSP_PartLedNVCtrlPara(
    NV_LED_SET_PARA_STRU               *pstLedNvStru,
    NV_LED_SET_PARA_STRU_EXPAND        *pstLedNvExStru,
    NV_LED_SET_PARA_STRU_COMBINED      *pstLedNvCombinedStru
);

BSP_VOID BSP_M2M_LedUpdate(BSP_VOID *pstLedNvStru);
void do_led_status(void);
#endif

void balong_led_brightness_set(struct led_classdev *led_cdev, enum led_brightness brightness);
int led_threecolor_flush(u32 channel_id , u32 len, void* context);
int do_led_threecolor_flush(void);
int balong_led_breath_set(struct led_classdev *led_cdev, unsigned long *full_on, unsigned long *full_off,
                                        unsigned long *fade_on, unsigned long *fade_off);

