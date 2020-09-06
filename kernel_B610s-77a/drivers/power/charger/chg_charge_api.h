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
 
#ifndef CHG_CHARGE_API_H
#define CHG_CHARGE_API_H

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
#if (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
 /*查询充电状态和模式*/
typedef struct
{
    int chg_state;
    int chg_mode;
}CHG_TCHRENABLE_TYPE;

typedef enum
{
    NO_BATT_MODE            = 0,        /*无电池*/
    KNOCK_DOWN_BATT_MODE    = 1,        /*可拆卸电池*/
    FIX_BATT_MODE           = 2,        /*一体化电池*/
    INVALID_BATT_MODE
}BATT_FIX_MODE;
#endif/*(MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)*/

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)

int chg_tbat_read(ENUM_CHG_ATCM_TYPE atID, void *pItem);


int chg_tbat_write(ENUM_CHG_ATCM_TYPE atID, void *pItem);


int chg_tbat_charge_mode_set(ENUM_CHG_MODE_TYPE cmd, unsigned int arg);


int chg_tbat_is_batt_status_ok(void);


int chg_tbat_get_charging_status(void);


int chg_tbat_get_charging_mode(void);


int chg_tbat_get_discharging_status(void);


int chg_MNTN_get_batt_state(void);


int chg_wireless_mmi_test(void);


int chg_extchg_mmi_test(void);


int chg_tbat_get_tchrenable_status(CHG_TCHRENABLE_TYPE *tchrenable_state);


int chg_getCbcState(unsigned char  *pucBcs ,unsigned char   *pucBcl);

#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/


int chg_tbat_sfeature_inquiry_extchg(void);


int chg_tbat_sfeature_inquiry_battery(void);


int chg_get_coulometer_support(void);




extern int32_t chg_usb_state_callback(int32_t STA, int32_t Msg_Error);

#endif

