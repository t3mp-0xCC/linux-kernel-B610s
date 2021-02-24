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


#ifndef CHG_CONFIG_H
#define CHG_CONFIG_H
/*----------------------------------------------*
 * �����ⲿͷ�ļ�                                   *
 *----------------------------------------------*/
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
#include "mdrv_chg.h"
#include "bsp_onoff.h"
#include "mdrv_sysboot.h"
#include "power_com.h"
#include "bsp_version.h"
#endif

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#ifndef FEATURE_ON
#define FEATURE_ON  1
#endif/*FEATURE_ON*/

#ifndef FEATURE_OFF
#define FEATURE_OFF 0
#endif/*FEATURE_OFF*/
/*************************BEGIN:��Ʒ�������Ժ궨��***********************/
/*��ŵ繦���ܿ��ƺ�,������ƺ꣬�ⲿ�����������˴����ٶ���*/
//#define    MBB_CHARGE
/*����LINUXƽ̨�򿪴�ƽ̨���Ժ�*/
#ifndef   MBB_CHG_PLATFORM_BALONG
#define   MBB_CHG_PLATFORM_BALONG          FEATURE_OFF
#endif/*HUAWEI_CHG_PLATFORM_BALONG*/

/*��ͨlinuxƽ̨�򿪴�ƽ̨���Ժ�*/
#ifndef   MBB_CHG_PLATFORM_QUALCOMM
#define   MBB_CHG_PLATFORM_QUALCOMM        FEATURE_OFF
#endif/*HUAWEI_CHG_PLATFORM_QUALCOMM*/

/*LCD��Ʒ���Ժ�*/
#ifndef   MBB_CHG_LCD
#define   MBB_CHG_LCD                      FEATURE_OFF
#endif/*MBB_CHG_LCD*/

/*LED��Ʒ���Ժ�*/
#ifndef   MBB_CHG_LED
#define   MBB_CHG_LED                      FEATURE_OFF
#endif/*MBB_CHG_LED*/

#ifndef   MBB_CHG_OLED
#define   MBB_CHG_OLED                     FEATURE_OFF
#endif/*MBB_CHG_OLED*/

/*���ÿ��ؼ�BQ27510���Ժ�*/
#ifndef   MBB_CHG_BQ27510
#define   MBB_CHG_BQ27510                  FEATURE_OFF
#endif/*MBB_CHG_BQ27510*/

/*BQ24196/BQ24192/BQ24296���оƬ���Ժ�*/
#ifndef   MBB_CHG_BQ24196
#define   MBB_CHG_BQ24196                  FEATURE_OFF
#endif/*MBB_CHG_BQ24196*/

/*BQ25892���оƬ���Ժ�*/
#ifndef   MBB_CHG_BQ25892
#define   MBB_CHG_BQ25892                  FEATURE_OFF
#endif/*MBB_CHG_BQ25892*/

/*SMB1351���оƬ���Ժ�*/
#ifndef   MBB_CHG_SMB1351
#define   MBB_CHG_SMB1351                FEATURE_OFF
#endif/*MBB_CHG_SMB1351*/

/*��ѹ������Ժ�*/
#ifndef   MBB_CHG_HVDCP_CHARGE
#define   MBB_CHG_HVDCP_CHARGE             FEATURE_OFF
#endif/*MBB_CHG_HVDCP_CHARGE*/

/*���������Ժ�*/
#ifndef   MBB_CHG_EXTCHG
#define   MBB_CHG_EXTCHG                   FEATURE_OFF
#endif/*MBB_CHG_EXTCHG*/

/*���߳�����Ժ�*/
#ifndef   MBB_CHG_WIRELESS
#define   MBB_CHG_WIRELESS                 FEATURE_OFF
#endif/*MBB_CHG_WIRELESS*/

/*�����������Ժ�*/
#ifndef   MBB_CHG_COMPENSATE
#define   MBB_CHG_COMPENSATE               FEATURE_OFF
#endif/*MBB_CHG_COMPENSATE*/

/*���³�����Ժ�*/
#ifndef   MBB_CHG_WARM_CHARGE
#define   MBB_CHG_WARM_CHARGE              FEATURE_OFF
#endif/*MBB_CHG_WARM_CHARGE*/

/*power supply���Ժ�*/
#ifndef   MBB_CHG_POWER_SUPPLY
#define   MBB_CHG_POWER_SUPPLY             FEATURE_OFF
#endif/*MBB_CHG_POWER_SUPPLY*/

/*���ؼ����Ժ�*/
#ifndef   MBB_CHG_COULOMETER
#define   MBB_CHG_COULOMETER              FEATURE_OFF
#endif/*MBB_CHG_COULOMETER*/

/*��ѹ������Ժ�*/
#ifndef   MBB_CHG_HIGH_VOLT_BATT
#define   MBB_CHG_HIGH_VOLT_BATT          FEATURE_OFF
#endif/*MBB_CHG_HIGH_VOLT_BATT*/

/*�ɲ�ж������Ժ꣬����֧�ֲ���AT��ѯ���ɲ�ж��ش򿪴˺꣬�ǿɲ�ж��عرմ˺�*/
#ifndef   MBB_CHG_BAT_KNOCK_DOWN
#define   MBB_CHG_BAT_KNOCK_DOWN          FEATURE_OFF
#endif/*MBB_CHG_BAT_KNOCK_DOWN*/

/*�������ƴ�USB/�����ȡ�����Ժ�*/
#ifndef   MBB_CHG_CURRENT_SUPPLY_LIMIT
#define   MBB_CHG_CURRENT_SUPPLY_LIMIT     FEATURE_OFF
#endif/*MBB_CHG_CURRENT_SUPPLY_LIMIT*/

/*�����س�������궨�� 20%�궨���Ժ�*/
#ifndef   MBB_CHG_CHARGE_CURRENT_LIMIT
#define   MBB_CHG_CHARGE_CURRENT_LIMIT     FEATURE_OFF
#endif/*MBB_CHG_CURRENT_SUPPLY_LIMIT*/

/*��һ��������ֲ�ⲿ�����ӿڴ�׮���Ժ�,�ӿ�������ɺ��*/
//#define CHG_STUB

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
#if (MBB_FAST_ON_OFF == FEATURE_ON)
#define CONFIG_MBB_FAST_ON_OFF
#endif/*MBB_FAST_ON_OFF == FEATURE_ON*/
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/


/*************************END:��Ʒ�������Ժ궨��****************/

/*NV��궨��*/
#define  NV_OVER_TEMP_SHUTOFF_PROTECT     (50016)
#define  NV_LOW_TEMP_SHUTOFF_PROTECT      (52005)
#define  NV_BATT_TEMP_PROTECT_I           (50385)
#define  NV_BATT_VOLT_PROTECT_I           (50386)
#define  NV_POWERUP_MODE                  (50364)
#define  NV_FACTORY_MODE_I                (36)


/*----------------------------------------------*
* ���������Ͷ���˵��                         *
*----------------------------------------------*/

typedef unsigned long           ulong64_t;
typedef unsigned int            uint32_t;
typedef signed int              int32_t;
typedef unsigned short          uint16_t;
typedef signed short            int16_t;
typedef unsigned char           uint8_t;
typedef signed char             int8_t;
typedef uint8_t                 boolean;

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
#ifndef YES
#define YES  1
#endif

#ifndef NO
#define NO   0
#endif

#ifndef TRUE
#define TRUE                  1
#endif
#ifndef FALSE
#define FALSE                 0
#endif

#define CHG_OK                 (0 )
#define CHG_ERROR              (-1)

/*=====================ERROR CODE===================*/
#define CHG_BUS_ERR            (-2)
#define CHG_IO_ERR             (-3)
#define CHG_NO_MEM             (-4)
#define CHG_ADC_ERR            (-5)
/**********Charge Module Return Code Macros**********/

/*�����ٷֱȶ���*/
#define    CHG_EVENT_NONEED_CARE     255

#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
/*SB03�����ͨ�ò�ͬ���ػ�������1%��*/
#if (defined(BSP_CONFIG_BOARD_E5_SB03))
#define    BATT_CAPACITY_SHUTOFF     1
#else
#define    BATT_CAPACITY_SHUTOFF     2
#endif
#define    BATT_CAPACITY_LEVELLOW    10
#define    BATT_CAPACITY_LEVEL1      11
#define    BATT_CAPACITY_LEVEL2      20
#define    BATT_CAPACITY_LEVEL3      30
#define    BATT_CAPACITY_LEVEL4      60
#define    BATT_CAPACITY_RECHG       95
#define    BATT_CAPACITY_FULL        100
#else
#define    BATT_CAPACITY_SHUTOFF     0
#define    BATT_CAPACITY_LEVELLOW    3
#define    BATT_CAPACITY_LEVEL1      10
#define    BATT_CAPACITY_LEVEL2      30
#define    BATT_CAPACITY_LEVEL3      50
#define    BATT_CAPACITY_LEVEL4      80
#define    BATT_CAPACITY_RECHG       95
#define    BATT_CAPACITY_FULL        100
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/

#if((FEATURE_ON == MBB_FACTORY)\
     && (FEATURE_ON == MBB_CHG_COULOMETER)\
     && (FEATURE_ON == MBB_CHG_EXTCHG ))
#define   COUL_CHECK_VBUS_TIME_MAX         (3)
#define   COUL_CHECK_CURRENT_TIME_MAX      (10)
#define   COUL_CHECK_VBUS_DELAY_MS         (50)
#define   COUL_CHECK_CURRENT_DELAY_MS      (500)
#define   COUL_CHECK_CURRENT_WAIT_MS       (2000)
#endif

/*----------------------------------------------*
 * ö�ٶ���                                       *
 *----------------------------------------------*/
typedef enum
{
    CHG_MSG_ERR = 0,
    CHG_MSG_INFO,
    CHG_MSG_DEBUG,
    CHG_MSG_MAX
}CHG_MSG_TYPE;

typedef enum
{
    /*CHG_STM_INIT_ST: NOT real state, just for initialize.*/
    CHG_STM_INIT_ST = -1,
    CHG_STM_TRANSIT_ST = 0,
    CHG_STM_FAST_CHARGE_ST,
    CHG_STM_MAINT_ST,
    CHG_STM_INVALID_CHG_TEMP_ST,
    CHG_STM_BATTERY_ONLY,
    CHG_STM_WARMCHG_ST,
    CHG_STM_HVDCP_CHARGE_ST,
    CHG_STM_MAX_ST,
}chg_stm_state_type;

typedef enum
{
    CHG_MODE_INIT = -1,
    CHG_WIRED_MODE   = 0,
    CHG_WIRELESS_MODE,
    CHG_EXTCHG_MODE,
    CHG_BAT_ONLY_MODE,
    CHG_SUPPLY_MODE,
    CHG_MODE_UNKNOW
}CHG_MODE_ENUM;


/****Note:����ö�ٱ����ڲ�ͬ��ƽ̨���ڲ�ͬ���ļ����ж��壬���ƽ̨�����ļ����ж��屾�ļ��е����¶������ע��******/

typedef enum POWER_ON_STATUS_tag
{
    POWER_ON_STATUS_INVALID         = 0x00000000, /* */
    POWER_ON_STATUS_BOOTING         = 0x00000000, /* */
    POWER_ON_STATUS_CHARGING        = 0x504F5343, /* */
    POWER_ON_STATUS_CAPACITY_LOW    = 0x504F434C, /* */
    POWER_ON_STATUS_BATTERY_ERROR   = 0x504F4245, /* */
    POWER_ON_STATUS_FINISH_NORMAL   = 0x6f656d00, /* */
    POWER_ON_STATUS_FINISH_CHARGE   = 0x59455353, /* */
    POWER_ON_STATUS_BUTT            = 0xFFFFFFFF  /* */
}POWER_ON_STATUS_ENUM;

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
typedef DRV_START_MODE_E DRV_START_MODE_ENUM;
typedef BATT_LEVEL_E BATT_LEVEL_ENUM;
#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
typedef enum DRV_START_MODE_tag
{
     DRV_START_MODE_EXCEPTION,  /* �쳣����ģʽ����ʱ�󲿷�ҵ�񶼲�����, ����ʾ��Ϣ */
     DRV_START_MODE_NORMAL,   /*��������*/
     DRV_START_MODE_CHARGING,   /* ��翪��ģʽ, ps ��ҵ������  */
     DRV_START_MODE_NOAPP,      /*SD�������ڶ�������������APP*/
     DRV_START_MODE_MODEM,
     DRV_START_MODE_BUTT
}DRV_START_MODE_ENUM;
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/

 /*chgģ���õ���ADC�ɼ�ͨ������ö��*/
typedef enum CHG_PARAMETER_ENUM_tag
{
    CHG_PARAMETER__BATTERY_VOLT,        /* ��ص�ѹ */
    CHG_PARAMETER__BATT_THERM_DEGC,     /* ���NTC�¶ȶ�Ӧ��ѹ */
    CHG_PARAMETER__VBUS_VOLT,           /* ���������ڼ��VBUS��ѹ*/
    CHG_PARAMETER__BATT_BOT_THERM_DEGC, /* �弶�¶ȶ�Ӧ��ص�ѹ*/
    CHG_PARAMETER__BATTERY_ID_VOLT,     /* ���ID��ѹ���ڵ���ͺ�ʶ��*/
    CHG_PARAMETER__VPH_PWR_VOLT,        /* VPH_PWR��ѹ*/
    CHG_PARAMETER__SURFACE_THERM_DEGC,  /* DCM�ȱ��������л�ȡ����¶�*/
    /*USB�±�*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    CHG_PARAMETER__USB_PORT_TEMP_DEGC,  /* USB ADC�¶ȶ�Ӧ��ѹ*/
#endif
    CHG_PARAMETER__INVALID
} CHG_PARAMETER_ENUM;

/*���LCD��ʾ*/
typedef enum
{
    CHG_DISP_OK,
    CHG_DISP_FAIL,
    CHG_DISP_BATTERY_LOWER,  /* LOW BATTERY   */
    CHG_DISP_BATTERY_BAD,  /*BATTERY ERROR*/
    CHG_DISP_OVER_HEATED,/* Over-heated */
    CHG_DISP_TEMP_LOW, /* Temp. Too Low */
    CHG_DISP_MAX
}CHG_BATT_DISPLAY_TYPE;

#if (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
typedef enum DRV_SHUTDOWN_REASON_tag_s
{
    DRV_SHUTDOWN_LOW_BATTERY,           /* ��ص�����                 */
    DRV_SHUTDOWN_BATTERY_ERROR,         /* ����쳣                   */
    DRV_SHUTDOWN_POWER_KEY,             /* ���� Power ���ػ�          */
    DRV_SHUTDOWN_TEMPERATURE_PROTECT,   /* ���±����ػ�               */
    DRV_SHUTDOWN_LOW_TEMP_PROTECT,
    DRV_SHUTDOWN_RESET,                 /* ϵͳ��λ                 */
    DRV_SHUTDOWN_CHARGE_REMOVE,         /* �ػ����ģʽ�£��γ������ */
    DRV_SHUTDOWN_UPDATE,                /* �ػ�����������ģʽ         */
    DRV_SHUTDOWN_BUTT
}DRV_SHUTDOWN_REASON_ENUM;


/****Note:����ö�ٱ����ڲ�ͬ��ƽ̨���ڲ�ͬ���ļ����ж��壬���ƽ̨�����ļ����ж��屾�ļ��е����¶������ע��******/
/*��س��״̬--��ѯ��״̬�Ե���Ϊ׼*/
typedef enum CHARGING_STATE_ENUM_tag
{
    CHARGING_INIT =  -1,
    NO_CHARGING_UP = 0,         /* ����δ���*/
    CHARGING_UP,               /*�������ڳ�� */
    NO_CHARGING_DOWN,          /* �ػ�δ��� */
    CHARGING_DOWN             /* �ػ����  */
}CHARGING_STATE_ENUM;

/* ��ص���*/
typedef enum BATT_LEVEL_ENUM_tag
{
    BATT_INIT =  -2,       /* */
    BATT_LOW_POWER = -1 ,  /*��ص͵磬*/
    BATT_LEVEL_0,                 /*0���ص���**/
    BATT_LEVEL_1 ,                /*1���ص���*/
    BATT_LEVEL_2 ,                 /*2���ص���*/
    BATT_LEVEL_3 ,               /*3���ص���*/
    BATT_LEVEL_4  ,              /*4���ص���*/
    BATT_LEVEL_MAX
}BATT_LEVEL_ENUM;

/****Note:����ö�ٱ����ڲ�ͬ��ƽ̨���ڲ�ͬ���ļ����ж��壬���ƽ̨�����ļ����ж��屾�ļ��е����¶������ע��******/

/*��ظ����ϱ��¼�*/
typedef enum _TEMP_EVENT
{
   TEMP_BATT_LOW,    /* ��ص����¼� */
   TEMP_BATT_HIGH,   /* ��ظ����¼� */
   TEMP_BATT_NORMAL, /* ��ظ��½���¼� */
   TEMP_BATT_MAX     /* ����¼�ֵ��С�ڴ�ֵ����Ϊ�Ƿ��¼�*/
}TEMP_EVENT;
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/

/*USB�±�*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
typedef enum _USB_TEMP_EVENT
{
    USB_TEMP_NORMAL,   /*USB���½��*/
    USB_TEMP_HIGH,     /*USB�����¼�*/
    USB_TEMP_MAX
}USB_TEMP_EVENT;
#endif

/*BATTERY(���)�����ϱ����¼�*/
typedef enum _BAT_EVENT
{
    BAT_CHARGING_ON_START = 0,  /* �ڿ���ģʽ����ʼ��� */
    BAT_CHARGING_OFF_START,     /* �ڹػ�ģʽ����ʼ��� */
    BAT_LOW_POWER,              /* ��ص�����ʱ���ϱ����¼� */
    BAT_CHARGING_UP_STOP,       /* ����ֹͣ���ʱ���ϱ����¼� */
    BAT_CHARGING_DOWN_STOP,     /* �ػ�ֹͣ���ʱ���ϱ����¼� */
    BAT_LOW_POWEROFF,           /* ��ص����͵��µĹػ����ϱ����¼�*/
    BAT_EVENT_MAX               /* ����¼�ֵ��С�ڴ�ֵ����Ϊ�Ƿ��¼� */
}BATTERY_EVENT;

/*KEY�����ϱ����¼�*/
typedef enum _GPIO_KEY_EVENT
{
    GPIO_KEY_WAKEUP = 0,           /* �̰����ⰴ�����ϱ����¼������ڵ�����Ļ */
    GPIO_KEY_WLAN,                 /* ���ڴ�/�ر�WLAN�豸����ͬ�İ汾ʹ�õİ�����ͬ */
    GPIO_KEY_WPS,                  /* ����WPS��������5�룩ʱ���ϱ����¼������ڼ���WPS���� */
    GPIO_KEY_FACTORY_RESTORE,      /* ������λ��ʱ���ϱ����¼������ڻָ��������� */
    GPIO_KEY_POWER_OFF,            /* ����POWER��ʱ���ϱ����¼������ڹػ� */
    GPIO_KEY_UPDATE,               /* ��ϵͳ��Ҫ����ʱ���ϱ����¼�����������ϵͳ */
    GPIO_KEY_SSID,                 /* �̰�WPS����С��5�룩ʱ��������ʾSSID��ֻ�в��ְ汾��Ҫ */
    GPIO_KEY_WPS_PROMPT,           /* �̰�WPS��������1�룩ʱ��������ʾWPS��ʾ�ֻ�в��ְ汾��Ҫ */
    GPIO_KEY_POWER_SHORT,          /* �̰�Power������500������Ҫ�ɿ����� */
    GPIO_KEY_WPS_SHORT,            /* �̰�WPS������500������Ҫ�ɿ����� */
    GPIO_KEY_RESET_SHORT,          /* �̰�Reset������500 ������Ҫ�ɿ����� */
    GPIO_KEY_WPS_POWER,            /* WPS��POWER ��һ����2�� */
    GPIO_KEY_WPS_POWER_UP,         /* WPS��POWER ��һ����2�����5��ǰ̧�𣬻���5���δ��500������̧����WPS */
    GPIO_KEY_POWER_ONE_SECOND,     /* Power����1�� */
    GPIO_KEY_POWER_ONE_SECOND_UP,  /* Power����1�����2��ǰ̧�� */
    GPIO_KEY_WPS_ONE_SECOND,       /*WPS �̰� 1���¼�����Ҫ���ڿ��� WiFi Offload*/
    GPIO_KEY_WPS_DIAL = 16,         /*WPS �̰��ɿ��ϱ� WPS �����¼�����Ҫ���ڰ�������*/
#ifdef FEATURE_FUNC_FAST_BOOT
    /*ƽ̨�ṩ5����ٹػ��¼�*/
    GPIO_KEY_SHORT_POWER_OFF = 17,
    /*ƽ̨�ṩ5����ٿ����¼�*/
    GPIO_KEY_SHORT_POWER_ON = 18,
 #endif
    GPIO_KEY_EVENT_MAX             /* ����¼�ֵ��С�ڴ�ֵ����Ϊ�Ƿ��¼� */
}GPIO_KEY_EVENT;

typedef enum
{
    DEVICE_ID_NULL_ID = 0, /*��id�����ڳ�ʼ��id*/
    DEVICE_ID_USB = 1,     /*USB�豸id*/
    DEVICE_ID_KEY = 2,     /*KEY�����豸id*/
    DEVICE_ID_BATTERY = 3,/*����¼��ϱ�ʹ�ô�ID*/
    DEVICE_ID_CHARGER = 4, /*������¼��ϱ�ʹ�ô�ID*/
    DEVICE_ID_SD_CARD = 5, /* */
    DEVICE_ID_GPIO_INTER = 6, /*�ǰ�����gpio�ж��ϱ���ʹ�ô�ID*/
    DEVICE_ID_SCREEN = 7, /* */
    DEVICE_ID_WLAN = 8, /* */
    DEVICE_ID_OM = 9, /* OM�ϱ���Ϣ */
    DEVICE_ID_TEMP = 10,/*�¶ȱ���ID*/
    DEVICE_ID_WIRELESS = 254, /*���߳���¼�*/
    DEVICE_ID_EXTCHG = 255,   /*�������¼�*/
    DEVICE_ID_MAX_ID   /*�����߽籣��*/
}DEVICE_ID_E;

#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
/*���ģ���ṩ��coul���¼�����*/
typedef enum
{
    VCHRG_START_USB_CHARGING_EVENT = 0,
    VCHRG_START_AC_CHARGING_EVENT,
    VCHRG_START_CHARGING_EVENT,
    VCHRG_STOP_CHARGING_EVENT,
    VCHRG_CHARGE_DONE_EVENT,
    VCHRG_NOT_CHARGING_EVENT,
    VCHRG_POWER_SUPPLY_OVERVOLTAGE,
    VCHRG_POWER_SUPPLY_WEAKSOURCE,
    VCHRG_MAX
}CHG_COUL_EVENT_TYPE;
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/

/*----------------------------------------------*
 * �ṹ����                                       *
 *----------------------------------------------*/
 /***************Note:ƽ̨��ش��룬����ƽ̨����Ҫ��ӣ��е�ƽ̨��V7R1��Ҫ
                  ��ֲ��Ա������Ҫ����ӻ����Ƴ��±ߺ�������***************************/
/*�������йؼ���Ϣ�ṹ��*/
typedef struct
{
    int32_t            battery_one_temp;        /* ��ص��βɼ��¶�**/
    int32_t            battery_temp;            /* ���ʵ���¶�**/
    int32_t            battery_one_volt;        /* ��ص��βɼ���ѹ**/
    int32_t            battery_volt;            /* ���ʵ�ʵ�ѹ**/
    int32_t            bat_capacity;            /* ��ص����ٷֱ�*/
    int32_t            bat_time_to_full;        /* ��ص�����������ж೤ʱ��*/
    BATT_LEVEL_ENUM    bat_volt_lvl;            /* ��ص�������,����ظ���**/
    int32_t            volt_revise_value;       /* ��ص�ѹ����������ֵ*/
    int32_t            battery_id_volt;         /* ���ID��ѹ*/
    int32_t            vph_pwr_one_volt;        /* VPH_PWR���βɼ���ѹ*/
    int32_t            vph_pwr_avg_volt;        /* VPH_PWRƽ��ֵ��ѹ*/
}CHG_PROCESS_INFO;

#if (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
typedef struct
{
    int32_t            usb_cur_temp;                         /*USB��ǰ�¶�*/
    boolean            usb_temp_protect_cur_stat;            /*USB�±���ǰ״̬��TRUEΪUSB�±���FALSEΪ����*/
    boolean            usb_temp_protect_pre_stat;            /*USB�±���һ״̬��TRUEΪUSB�±���FALSEΪ����*/
}USB_TEMP_PROTECT_INFO;
#endif

/****Note:���½ṹ���ڲ�ͬ��ƽ̨���ڲ�ͬ���ļ����ж��壬���ƽ̨�����ļ����ж��屾�ļ��е����¶������ע��******/
#if (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
typedef struct BATT_STATE_tag
{
    CHARGING_STATE_ENUM  charging_state;
    BATT_LEVEL_ENUM      battery_level;
    TEMP_EVENT           batt_temp_state;
}BATT_STATE_T;
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/
/****Note:����ö�ٱ����ڲ�ͬ��ƽ̨���ڲ�ͬ���ļ����ж��壬���ƽ̨�����ļ����ж��屾�ļ��е����¶������ע��******/
/*=================================================================
 CHG_CHGR_UNKNOWN: Chgr type has not been check completely from USB module.
 CHG_WALL_CHGR   : Wall standard charger, which D+/D- was short.
 CHG_USB_HOST_PC : USB HOST PC or laptop or pad, etc.
 CHG_NONSTD_CHGR : D+/D- wasn't short and USB enumeration failed.
 CHG_CHGR_INVALID: External Charger invalid or absent.
 ==================================================================*/
typedef enum
{
    /*δ֪����*/
    CHG_CHGR_UNKNOWN        = 0 ,
    /*��׼�����*/
    CHG_WALL_CHGR           = 1 ,
    /*USB*/
    CHG_USB_HOST_PC         = 2 ,
    /*�Ǳ�׼�����*/
    CHG_NONSTD_CHGR         = 3 ,
    /*���߳����*/
    CHG_WIRELESS_CHGR       = 4 ,
    /*��������*/
    CHG_EXGCHG_CHGR         = 5 ,
    /*����*/
    CHG_500MA_WALL_CHGR     = 6 ,
    /*cradle*/
    CHG_USB_OTG_CRADLE      = 7 ,
    /*��ѹ�����*/
    CHG_HVDCP_CHGR          = 8 ,
    /*�����������*/
    CHG_CHGR_INVALID,
}chg_chgr_type_t;



 /*ͨ�����Ͷ�Ӧͨ���Žṹ*/
typedef struct
{
    CHG_PARAMETER_ENUM  param_type;
    int32_t             adc_channel;
}CHG_ADC_PARAM_TO_CHANNEL_TYPE;

/*�¶ȶ�Ӧ��ѹ�ṹ*/
typedef struct
{
    int32_t   temperature;
    int32_t   voltage;
}CHG_TEMP_ADC_TYPE;

typedef struct
{
    chg_stm_state_type cur_stm_state;     /*���״̬����ǰ״̬*/
    chg_chgr_type_t    cur_chgr_type;     /*��ǰ���������*/
    uint32_t          charging_lasted_in_sconds; /*������ʱ��*/
    chg_stm_state_type pre_stm_state;     /*���״̬��ǰһ��״̬*/
    int32_t            bat_stat_type;     /*��س��״̬��Ϣ��power supply STATUS���Զ�Ӧ*/
    int32_t            bat_heath_type;    /*����¶�״̬��Ϣ��power supply HEALTH���Զ�Ӧ*/
    int32_t            extchg_status;     /*������״̬��Ϣ��power supply EXTCHG_STA���Զ�Ӧ*/
    CHG_MODE_ENUM      cur_chg_mode;      /*Ŀǰ�����ĳ��״̬������/����/������״̬*/
    boolean            extchg_online_st;  /*��������λ״̬*/
    boolean            wireless_online_st;/*���߳����λ״̬*/
    boolean            usb_online_st;  /*��������λ״̬*/
    boolean            ac_online_st;/*���߳����λ״̬*/
    /*USB�±�*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    int32_t            usb_heath_type;    /*USB�¶�״̬��Ϣ��power supply usb_health��Ӧ*/
#endif
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    uint32_t          charger_lasted_without_charging_in_seconds;  /*ֹͣ����ҳ������λ�ĳ���ʱ��*/
#endif
}chg_stm_state_info_t;

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
#include "product_config.h"
#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
#include <linux/huawei_feature.h>
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/

#include "chg_charge_api.h"
#include "chg_charge_stm.h"
#include "chg_charge_task.h"
#include "huawei-charger.h"

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
#include "mdrv_chg.h"
//#include "mdrv_onoff.h"
#include "hardware_data/chg_hardware_data.h"
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/

#include "platform/hisi/chg_chip_platform.h"
#include "os_adapter/linux/chg_os_adapter.h"

#if (MBB_CHG_BQ24196 == FEATURE_ON)
#include "drv/bq24196/chg_charge_drv.h"
#elif (MBB_CHG_BQ25892 == FEATURE_ON)
#include "drv/bq25892/chg_charge_drv.h"
#elif (MBB_CHG_SMB1351 == FEATURE_ON)
#include "drv/smb1351/chg_charge_drv.h"
#endif/*MBB_CHG_SMB1351 == FEATURE_ON*/

#if (MBB_CHG_COULOMETER == FEATURE_ON)
#include "drv/coul/drivers/hisi_smartstar_coul.h"
#include "drv/coul/drivers/hisi_coul_drv.h"
#endif

#if (MBB_CHG_BQ27510 == FEATURE_ON)
#include "drv/bq27510/bq27510_battery.h"
#include "drv/bq27510/hisi_coul_drv.h"
#include "drv/bq27510/bq27510_battery_monitor.h"
#endif/*MBB_CHG_BQ27510 == FEATURE_ON*/

#endif/*CHG_CONFIG_H*/
