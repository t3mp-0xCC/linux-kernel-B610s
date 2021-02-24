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

#ifndef __HUAWEI_FLIGHT_MODE_H__
#define __HUAWEI_FLIGHT_MODE_H__


#ifdef __cplusplus
extern "C"
{
#endif
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <bsp_nvim.h>
#include <bsp_icc.h>
#include "product_nv_id.h"
#include "product_nv_def.h"
#include "mdrv_version.h"
#include "bsp_softtimer.h"
#include "mbb_flight_mode.h"

#if (FEATURE_ON == MBB_DLOAD)
#include "bsp_sram.h"
#endif /*MBB_DLOAD*/

#define WDIS_DRIVER_NAME "W-DISABLE"
#define RF_STATUS_DRIVER_NAME  "RF_STATUS"
#define W_DISABLE_PIN GPIO_0_5
#define RF_STATUS_PIN GPIO_2_6
#define WDIS_N_TIMER_LENGTH  (200)

typedef struct 
{
    unsigned int         hw_state;
    unsigned int         sw_state;
    struct softtimer_list irq_timer;
}RFSWITCH_T_CTRL_S;
typedef enum
{
    RFSWITCH_RF_INIT = 0,
    RFSWITCH_RF_ON,
    RFSWITCH_RF_OFF,
    RFSWITCH_RF_NONE
}rfswitch_ccore_op_type;

#if  (FEATURE_ON == MBB_MODULE_THERMAL_PROTECT)
extern void 
RfSwitch_State_Info_Therm(VOS_UINT32 rf_state);
#endif
extern void 
RfSwitch_change(unsigned int rf_state);
extern void 
RfSwitch_State_Report
(
unsigned int rfswstate,
unsigned int rfhwstate
);

void 
RfSwitch_Swswtich_op_end(unsigned int sw_op_mode);
void 
RfSwitch_Hwswtich_op_end(unsigned int hw_op_mode);
void 
RfSwitch_Init_op_end(unsigned int init_op_mode);
#ifdef __cplusplus
}
#endif
#endif


