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
#ifndef HUAWEI_CHARGER_H
#define HUAWEI_CHARGER_H



/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <linux/power_supply.h>
#include <linux/notifier.h>
#include "product_config.h"/*balongƽ̨ר��ͷ�ļ�*/
#include <linux/rtc.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>

/*----------------------------------------------*
 * �ṹ�嶨��                                    *
 *----------------------------------------------*/
struct chargeIC_chip {
    struct i2c_client   *client;
    struct power_supply bat;
    struct power_supply usb;
    struct power_supply ac;
    struct power_supply extchg;
    struct power_supply wireless;

    int usb_online;                    /*USB�Ƿ���λ��1��ʾ��λ��0��ʾ����λ*/
    /*USB�±�*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    int usb_health;                    /*USB�±�״̬*/
#endif
    int ac_online;                     /*AC�Ƿ���λ��1��ʾ��λ��0��ʾ����λ*/
    int wireless_online;               /*���߳���Ƿ���λ��1��ʾ��λ��0��ʾ����λ*/

    int extchg_online;                 /*USB ID���Ƿ���λ��1��ʾ��λ��0��ʾ����λ*/
    int extchg_status;                 /*������״̬�����ڶ����绹��ͣ���*/
#if ((FEATURE_ON == MBB_FACTORY) \
    && (FEATURE_ON == MBB_CHG_EXTCHG ) \
    && (FEATURE_ON == MBB_CHG_COULOMETER))
    int extchg_start;                 /*�����ŵ�״̬*/
#endif
    int bat_present;                   /*����Ƿ���λ��1��ʾ��λ��0��ʾ����λ*/
    int bat_stat;                      /*��ص�ѹ״̬���Ƿ��ڳ�磬�Ƿ���Ҫ�����*/
    int bat_health;                    /*����¶�״̬���Ƿ���µ�*/
    int bat_technology;                /*������ͣ�﮵�ص�*/
    int bat_avg_voltage;               /*���ƽ����ѹ*/
    int bat_avg_temp;                  /*���ƽ���¶�*/
    int bat_capacity;                  /*��ص����ٷֱ�*/
    int bat_time_to_full;              /*��س�����Ҫʱ��*/
    struct blocking_notifier_head notifier_list_bat;
    struct notifier_block nb;
    /*extchg monitor*/
    struct delayed_work	extchg_monitor_work;
    /*������δʹ��Ϊ����������չ����*/
    int charge_current_limit_ua;
    int input_current_limit_ua;
    int term_current_ua;
    bool charging_enabled;
    bool otg_mode_enabled;
    bool charging_allowed;
    bool usb_suspend_enabled;
    struct rtc_device *rtc;
    int frequency;
    struct work_struct alarm_work;
    struct wake_lock alarm_wake_lock;/* ����ϱ��¼�ʱ����ʱ�� */
    struct wake_lock chg_wake_lock;    /* ��ֹ����ִ���������������� */
};
/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define CHARGEIC_PROC_FILE      "driver/charge_ic"
#define DLOAD_CHG_PROC_FILE     "dload_chg"
#define POWERON_CHG_PROC_FILE   "power_on"

#define ATOI_CONVERT_NUM            (10)
#define ATOI_MAX_LENGTH             (256)

#define ONLINE     1
#define OFFLINE    0

#define PRESENT    1
#define UNPRESENT  0


#endif

