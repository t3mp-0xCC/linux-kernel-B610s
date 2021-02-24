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
#define LED1_NAME    H6551_DR1_NM   /* ��������Ʊ�����regulator��nameһ�£�Ӧʹ�ú궨�� */
#define LED2_NAME    H6551_DR2_NM
#define LED3_NAME    H6551_DR3_NM
#define LED4_NAME    H6551_DR4_NM
#define LED5_NAME    H6551_DR5_NM

/* trigger name */
#define LED_TRIGGER_TIMER       "timer"     /* defined in timer_led_trigger, ledtrig-timer.c */
#define LED_TRIGGER_BREATH      "breath"    /* defined in breath_led_trigger, ledtrig-breath.c */

/* ����sysfsϵͳ��ʾ����ģʽ����ʱ���������ַ����ĳ��� */
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
    struct work_struct work;     /* ���������Ҫʹ�ù���������� */
    int    brightness;           /* ��Ҫ�����õ����� */
    unsigned dr;
    unsigned default_brightness; /* Ĭ�ϵ����� */
#endif
};

/* ƽ̨��LED_RED��Ӧ��DR1�ܽš�LED_GREEN��ӦDR2�ܽ�*/
#if(FEATURE_ON == MBB_FEATURE_M2M_LED)
enum LED_COLOR_WHOLE
{
    LED_NULL = 0x0,
    LED_RED = 0x1,                      /*DR1,��Ӧ��Ʒ��LED1�ܽ�*/
    LED_GREEN = 0x2,                    /*DR2,��Ӧ��Ʒ��LED2�ܽ�*/
    LED_BLUE = 0x4,                     /*DR3,��Ʒ������LED3�ܽ�*/
    LED_YELLOW = 0x3,                   /*��ӦDR1��DR2�ܽ�*/
    LED_PURPLE = 0x5,                   /*��ӦDR1��DR3�ܽ�*/
    LED_CYAN = 0x6,                     /*��ӦDR2��DR3�ܽ�*/
    LED_WHITE = 0x7,                    /*��ӦDR1��DR2��DR3�ܽ�*/
};
#else

/******************************************************************************************/
/* ��ɫ����صĺ� */

/* ��ɫֵ,0λ��ʾ��,1λ��ʾ��,2λ��ʾ��,����λ����
��Ϊ��+��,��Ϊ��+��,��Ϊ��+��*/

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
    DRV_M2M_LED_MODE_CLOSED  = 0,                                                /* �������� */
    DRV_M2M_LED_MODE_DEFAULT = 1,                                                /* ��ΪĬ�����Ʒ��� */
    DRV_M2M_LED_MODE_USER    = 2,                                                /* �û��Զ������Ʒ��� */
    DRV_M2M_LED_MODE_BUTT
};

/* ģ��ͨ�ù������Ʒ���״̬ */
enum M2M_LED_STATUS
{
    DRV_M2M_LED_FLIGHT_MODE             = 0,            /* ����ģʽ */
    DRV_M2M_LED_SYSTEM_INITIAL          = 1,            /* �ϵ��ʼ�� */
    DRV_M2M_LED_OFFLINE_UNREGISTERED    = 2,            /* ע��ʧ�ܣ��޷��� */
    DRV_M2M_LED_OFFLINE_NO_SERVICE      = 3,            /* �������޷��� */
    DRV_M2M_LED_GSM_REGISTERED          = 4,            /* GSMע��ɹ� */
    DRV_M2M_LED_GSM_BEAR_SETUP          = 5,            /* GSM�����߳��ؽ����ɹ� */
    DRV_M2M_LED_GSM_TRANSMITING         = 6,            /* GSM������ */
    DRV_M2M_LED_WCDMA_REGISTERED        = 7,            /* WCDMAע��ɹ� */
    DRV_M2M_LED_WCDMA_BEAR_SETUP        = 8,            /* WCDMA�����߳��ؽ����ɹ� */
    DRV_M2M_LED_WCDMA_TRANSMITING       = 9,            /* WCDMA������ */
    DRV_M2M_LED_HSPA_BEAR_SETUP         = 10,           /* HSPA�����߳��ؽ����ɹ� */
    DRV_M2M_LED_HSPA_TRANSMITING        = 11,           /* HSPA������ */
    DRV_M2M_LED_LTE_REGISTERED          = 12,            /* WCDMAע��ɹ� */
    DRV_M2M_LED_LTE_BEAR_SETUP          = 13,            /* WCDMA�����߳��ؽ����ɹ� */
    DRV_M2M_LED_LTE_TRANSMITING         = 14,            /* WCDMA������ */

    DRV_M2M_LED_NORMAL_UPDATING,                        /* ����״̬���¶������״̬�ӵ���������״̬ǰ */
    
    DRV_M2M_LED_STATE_MAX = 32,
}; /*the state of M2M LED */

#define NV_LED_FLICKER_DEFAULT_TIME                            (1)
#define NV_LED_FLICKER_MAX_TIME                                (100)
#define NV_LED_FLICKER_MAX_NUM                                 (2)
#define NV_LED_SERVICE_STATE_NUM                               (15)
/* ģ�鶨�����Ʒ���״̬ */
enum CE_LED_STATUS
{
    DRV_CE_LED_UNRF            = 0,            /* �ر�RF*/
    DRV_CE_LED_RF         = 1,            /* ����RF*/
  
    DRV_CE_LED_STATE_MAX = 32,
}; /*the state of CELED */
#endif

#if (FEATURE_ON == MBB_WPG_LED_FLASH)
enum
{
    DEF = 0,                /*0:Ĭ������ö��*/
    HONGKONG,               /*1:���PCCW����ö��*/
    RUSSIA,                 /*2:����˹MTS����ö��*/
    VODAFONE,               /*3:vodafone����ö��*/   
    CHINAUNION,             /*4:�й���ͨ����ö��*/
                            /*5:Ԥ��ֵ*/
    SFR = 6,                /*6:����SFR����ö��*/
    BSNL,                   /*7:ӡ��BSNL����ö��*/
    KPN,                    /*8:����KPN����ö��*/
    SOFTBANK,               /*9:�ձ���������ö��*/
    EMOBILE                /*10:�ձ�Emobile����ö��*/
};
#endif

/* ��ɫ��Ĭ��״̬����*/
#define LED_GREEN_LIGHTING_DOUBLE               {{LED_GREEN, 1},{LED_NULL,1},{LED_GREEN, 1},{LED_NULL, 17},{0,0}}/*�̵�˫��*/
#define LED_GREEN_LIGHTING_SIGNAL               {{LED_GREEN, 1},{LED_NULL, 19},{0,0}}/*�̵Ƶ���*/
#define LED_GREEN_LIGHTING_FREQUENTLY           {{LED_GREEN, 1},{LED_NULL,1},{0,0}}/*�̵�Ƶ��*/
#define LED_GREEN_LIGHTING_ALWAYS               {{LED_GREEN, 1},{0,0}}/*�̵Ƴ���*/

#define LED_WHITE_LIGHTING_FREQUENTLY           {{LED_WHITE, 1},{LED_NULL,1},{0,0}}/*�׵�Ƶ��*/

#define LED_BULE_LIGHTING_SIGNAL                {{LED_BLUE, 1},{LED_NULL, 19},{0,0}}/*���Ƶ���*/
#define LED_BLUE_LIGHTING_FREQUENTLY            {{LED_BLUE, 1},{LED_NULL,1},{0,0}}/*����Ƶ��*/
#define LED_BLUE_LIGHTING_ALWAYS                {{LED_BLUE, 1},{0,0}}/*���Ƴ���*/

#define LED_CYAN_LIGHTING_SIGNAL                {{LED_CYAN, 1},{LED_NULL, 19},{0,0}}/*��Ƶ���*/
#define LED_CYAN_LIGHTING_ALWAYS                {{LED_CYAN, 1},{0,0}}/*��Ƴ���*/

#define LED_BLUE_GREEN_ALTERNATE                {{LED_BLUE, 1},{LED_NULL,1},{LED_GREEN, 1},{LED_NULL, 17},{0,0}}/*���̽���˫��*/  

#define LED_RED_LIGHTING_DOUBLE                 {{LED_RED, 1},{LED_NULL,1},{LED_RED, 1},{LED_NULL, 17},{0,0}}/*���˫��*/
#define LED_RED_LIGHTING_SIGNAL                 {{LED_RED, 1},{LED_NULL, 19},{0,0}}/*��Ƶ���*/
#define LED_RED_LIGHTING_ALWAYS                 {{LED_RED, 1},{0,0}}/*��Ƴ���*/

#define LED_SHUTDOWN_DEFINE                     {{LED_NULL, 1},{0,0}}/*����*/

#if(FEATURE_ON == MBB_FEATURE_M2M_LED)
#define LED_DR1_LIGHTING_SIGNAL       {{LED_RED, 1},{LED_NULL,19},{0,0}}    /*LED1����,��ӦDR1�ܽ�*/
#define LED_DR1_LIGHTING_DOUBLE       {{LED_RED, 1},{LED_NULL, 1},{LED_RED, 1},{LED_NULL, 17},{0,0}}    /*LED1˫������ӦDR1�ܽ�*/
#define LED_DR1_LIGHTING_ALWAYS       {{LED_RED, 1},{0,0}}                  /*LED1��������ӦDR1�ܽ�*/
#define LED_DR1_LIGHTING_FREQUENTLY   {{LED_RED, 1},{LED_NULL, 1},{0,0}}  /*LED1Ƶ������ӦDR1�ܽ�*/
#endif

#if (FEATURE_ON == MBB_WPG_LED_FLASH)
#define LED_PURPLE_LIGHTING_SIGNAL                {{LED_PURPLE, 1},{LED_NULL, 19},{0,0}}/*�ϵƵ���*/
#define LED_PURPLE_LIGHTING_ALWAYS                {{LED_PURPLE, 1},{0,0}}/*�ϵƳ���*/
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

 /* ��ɫ������״̬���и���*/
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

/* ��ɫ�Ƶ�ǰ״̬����*/
struct led_param
{
	unsigned char led_state_id;     /* ��ɫ��״̬����ӦLED_STATUS */
	unsigned char led_config_id;    /* ����״̬����ֵ */
	unsigned char led_color_id;     /* ��ɫ����ɫ����ӦLED_COLOR��ֵ */
	unsigned char led_time;         /* �����ó�����ʱ�䳤�ȣ���λ100ms */
};

/*****************************************************************************
 �ṹ��    : LED_CONTROL_NV_STRU
 �ṹ˵��  : LED_CONTROL_NV�ṹ ID=7
*****************************************************************************/
typedef struct
{
    unsigned char   ucLedColor;      /*��ɫ����ɫ����ӦLED_COLOR��ֵ*/
    unsigned char   ucTimeLength;    /*�����ó�����ʱ�䳤�ȣ���λ100ms*/
}LED_CONTROL_NV_STRU;

#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
#define NV_GU_LED_SERVICE_STATE_NUM                            (12)
#define NV_LTE_LED_SERVICE_STATE_NUM                           (3)
#define NV_LED_SERVICE_STATE_NUM                               (15)


typedef struct
{
    LED_CONTROL_NV_STRU   stLED[4];                                                 /* һ�����������ڣ�����������˸ */
}M2M_LED_CONTROL_STRU;


typedef struct
{
    BSP_U8                              ucLedMode;                                  /* ����ģʽ */
    BSP_U8                              unresolved[3];                              /* ���� */
    M2M_LED_CONTROL_STRU                stLedStr[NV_GU_LED_SERVICE_STATE_NUM];      /* �������� */
}NV_LED_SET_PARA_STRU;


typedef struct
{
    M2M_LED_CONTROL_STRU                stLedStr[NV_LTE_LED_SERVICE_STATE_NUM];     /* �������� */
}NV_LED_SET_PARA_STRU_EXPAND;


typedef struct
{
    BSP_U8                              ucLedMode;                                  /* ����ģʽ */
    BSP_U8                              unresolved[3];                              /* ���� */
    M2M_LED_CONTROL_STRU                stLedStr[NV_LED_SERVICE_STATE_NUM];         /* �������� */
}NV_LED_SET_PARA_STRU_COMBINED;
#endif

/* led nv�ṹ�壬����nv���Ѷ���� */
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

