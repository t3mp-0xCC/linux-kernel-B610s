
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
 
 
#ifndef CHG_CHIP_PLATFORM_H
#define CHG_CHIP_PLATFORM_H
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

/********Note:以下头文件跟平台强相关，移植人员根据平台需要添加*******/
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/gpio.h>
#include <asm/div64.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <asm/unaligned.h>
#include <linux/spinlock.h>

#ifndef CHG_STUB
#include <linux/mlog_lib.h>
#else
#define  mlog_print(format, ...)
#endif/*CHG_STUB*/

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
/*I2C操作头文件*/
#include "bsp_i2c.h"
/*NV操作头文件*/
#include "bsp_nvim.h"
/*定时器操作头文件*/
#include "bsp_softtimer.h"
/*库仑计头文件*/
#include "bsp_coul.h"
#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
#include <linux/qpnp/qpnp-adc.h>
#include <linux/huawei_feature.h>
#ifndef CHG_STUB
#include <mach/huawei_smem.h>
#include "../../../huawei/adc_driver/adc_driver.h"
#endif/*CHG_STUB*/
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/

/********Note:以下头文件跟平台强相关，移植人员根据平台需要添加*******/
#include <mdrv_sysboot.h>
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
extern void kernel_power_off(void);
typedef DRV_SHUTDOWN_REASON_E DRV_SHUTDOWN_REASON_ENUM;
/*----------------------------------------------*
 * 宏定义说明                                   *
 *----------------------------------------------*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)
/*根据评审意见将所有GPIO号使用宏定义代替*/
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
//#define EXTCHG_CONTROL_GPIO_EN1       GPIO_0_11 /* V7R2平台只控制EN1，EN2由硬件强制拉低*/
#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
#define EXTCHG_CONTROL_GPIO_EN1       (59)
#define EXTCHG_CONTROL_GPIO_EN2       (60)
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/

#if (MBB_CHG_WIRELESS == FEATURE_ON)
/*根据评审意见将所有GPIO号使用宏定义代替*/
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
#define WIRELESS_CONTROL_GPIO_EN1     GPIO_0_11 /* V7R2平台只控制EN1，EN2由硬件强制拉低*/
#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
#define WIRELESS_CONTROL_GPIO_EN1     (59)
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/
#define WIRELESS_CONTROL_GPIO_EN2     (60)
#define USB_WIRELESS_CHGR_DET         (0x0006)
#endif/*MBB_CHG_WIRELESS*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
/**********************************************************************
FUNCTION:    chg_send_stat_to_app
DESCRIPTION: Charge module sends charge state to application layer.
INPUT:       uint32_t chg_device_id,
             uint32_t chg_event_id
OUTPUT:      None.
RETURN:      None.
NOTE:        None
***********************************************************************/
void chg_send_stat_to_app(uint32_t chg_device_id, uint32_t chg_event_id);
/**********************************************************************
FUNCTION:    CHG_SET_POWER_OFF
DESCRIPTION: The poweroff func of CHG module, all the power-off operation
             except at boot phase MUST be performed via calling this.
INPUT:       The shutdown reason which triggered system poweroff.
             All VALID REASON:
             DRV_SHUTDOWN_BATTERY_ERROR --BATTERY ERROR;
             DRV_SHUTDOWN_TEMPERATURE_PROTECT --EXTREAM HIGH TEMPERATURE.
             DRV_SHUTDOWN_LOW_TEMP_PROTECT --EXTREAM LOW TEMPERATURE
             DRV_SHUTDOWN_CHARGE_REMOVE --CHGR REMOVAL WHILE POWEROFF CHG
             DRV_SHUTDOWN_LOW_BATTERY --LOW BATTERY
OUTPUT:      None.
RETURN:      None.
NOTE:        When this function get called to power system off, it record
             the shutdown reason, then simulate POWER_KEY event to APP to
             perform the real system shutdown process.
             THUS, THIS FUNCTION DOESN'T TAKE AFFECT IF APP DIDN'T STARTUP.
***********************************************************************/
void chg_set_power_off(DRV_SHUTDOWN_REASON_ENUM real_reason);


extern int32_t chg_get_volt_from_adc(CHG_PARAMETER_ENUM param_type);


extern int32_t chg_get_batt_volt_value(void);


extern int32_t chg_get_vph_pwr_volt_value(void);


extern int32_t chg_get_temp_value(CHG_PARAMETER_ENUM param_type);


extern boolean chg_is_powdown_charging (void);

/*****************************************************************************
 函 数 名  : get_chgr_type_from_usb
 功能描述  : Get real charger type from USB module.
             This function called the platform interfaces to obtain the real
             charger type from USB module.
             !!Platform dependent, currently we implemented as Balong V700R001
             design.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : Charger type of state machine needed.
 CALL-WHOM : Platform interfaces.
 WHO-CALLED: chg_check_and_update_hw_param_per_chgr_type
             chg_transit_state_period_func
 NOTICE    : 1. Platform chgr_type enum may be different from chg_stm design,
             we need convert or remap them before return.
             2. Balong V3R2/V7R1 platform didn't support CHG_USB_HOST_PC,
             CHG_NONSTD_CHGR and CHG_USB_HOST_PC would all be treated as
             CHG_NONSTD_CHGR.
             3. Export Required.
*****************************************************************************/
extern chg_chgr_type_t get_chgr_type_from_usb(ulong64_t plug,int32_t chg_type);


void chg_display_interface(CHG_BATT_DISPLAY_TYPE disp_type);


/***************Note:平台相关代码，根据平台按需要添加，有的平台如9x25需要
              移植人员根据需要，添加或者移除下边函数调用***************************/
void chg_batt_volt_calib_init(void);

int32_t chg_get_volt_from_adc(CHG_PARAMETER_ENUM param_type);

#if (MBB_CHG_EXTCHG == FEATURE_ON)

extern int extchg_gpio_init(void);


extern void extchg_gpio_control(uint32_t gpio,int32_t level);

#if((FEATURE_ON == MBB_FACTORY)\
     && (FEATURE_ON == MBB_CHG_COULOMETER))

void mmi_extchg_shutdown(void);
#endif/*(FEATURE_ON == MBB_FACTORY) && (FEATURE_ON == MBB_CHG_COULOMETER)*/
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/


#if (MBB_CHG_WIRELESS == FEATURE_ON)

extern int wireless_gpio_init(void);

extern void wireless_gpio_control(uint32_t gpio,int32_t level);
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/


extern boolean is_in_update_mode(void);

/**********************************************************************
函 数 名  : int32_t chg_config_para_read(int32_t num)
功能描述  : 读充电可配置参数
输入参数  : none
输出参数  : 无。
返 回 值  : 无。
注意事项  : 可根据平台不同修改实现方法，比如NV等。
***********************************************************************/
int32_t chg_config_para_read(uint16_t nvID, void *pItem, uint32_t ulLength);


boolean chg_pt_mmi_test_proc(void);

#if (MBB_CHG_COMPENSATE == FEATURE_ON)

void chg_set_fact_release_mode(boolean on);
#endif/*MBB_CHG_COMPENSATE == FEATURE_ON*/


void chg_get_hw_version_id(void);



#endif/*CHG_CHIP_PLATFORM_H*/

