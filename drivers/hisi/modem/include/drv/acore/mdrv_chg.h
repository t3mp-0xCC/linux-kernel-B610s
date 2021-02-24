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
 


#ifndef __MDRV_ACORE_CHG_H__
#define __MDRV_ACORE_CHG_H__

#include "mdrv_public.h"
#include "product_config.h"
#ifdef __cplusplus
extern "C"
{
#endif


typedef enum CHARGING_STATE_E_tag
{
        CHARGING_INIT = -1,
        NO_CHARGING_UP = 0,  /*开机未充电*/
        CHARGING_UP ,              /*开机正充电*/
        NO_CHARGING_DOWN ,  /*关机未充电*/
        CHARGING_DOWN         /*关机未充电*/
}CHARGING_STATE_E;

typedef enum BATT_LEVEL_E_tag
{
        BATT_INIT = -2,
        BATT_LOW_POWER =-1,    /*电池低电*/
        BATT_LEVEL_0,          /*0格电池电量*/
        BATT_LEVEL_1,          /*1格电池电量*/
        BATT_LEVEL_2,          /*2格电池电量*/
        BATT_LEVEL_3,          /*3格电池电量*/
        BATT_LEVEL_4,          /*4格电池电量*/
        BATT_LEVEL_MAX
}BATT_LEVEL_E;

/*电池高温上报事件*/
typedef enum _TEMP_EVENT
{
   TEMP_BATT_LOW,    /* 电池低温事件 */
   TEMP_BATT_HIGH,   /* 电池高温事件 */
   TEMP_BATT_NORMAL, /* 电池高温解除事件 */
   TEMP_BATT_MAX     /* 如果事件值不小于此值，则为非法事件*/
}TEMP_EVENT;

typedef struct BATT_STATE_tag
{
    CHARGING_STATE_E  charging_state;
    BATT_LEVEL_E      battery_level;
    TEMP_EVENT      batt_temp_state;
}BATT_STATE_S;

#if (MBB_COMMON == FEATURE_ON)

typedef BATT_STATE_S BATT_STATE_T;
#define CHG_OK                 0
#define CHG_ERROR              (-1)

extern BATT_LEVEL_E chg_get_batt_level(void);
unsigned char chg_is_powdown_charging (void);

#define  NV_BATT_VOLT_CALI_I           (58639)
#define  TBAT_CHECK_INVALID            (0xFFFF)

/*电池校准参数数据结构*/
typedef struct
{
    unsigned short min_value;
    unsigned short max_value;
}VBAT_CALIBRATION_TYPE;
/*AT命令读取电池电压枚举类型*/
typedef enum
{
    CHG_AT_BATTERY_LEVEL,        /* Battery Voltage after Calibration*/
    CHG_AT_BATTERY_ADC,          /* Battery Voltage before Calibration*/
    CHG_AT_BATTERY_CAPACITY,     /* Battery Capacity*/
    CHG_AT_BATTERY_CHECK,        /* Battery Calibration*/
    CHG_AT_BATTERY_INVALID
} ENUM_CHG_ATCM_TYPE;
typedef enum
{
    CHG_AT_CHARGE_DISALBE,        /* disable charge*/
    CHG_AT_CHARGE_ENABLE,         /* enable charge*/
    CHG_AT_DISCHARGE_DISALBE,     /* disable discharge*/
    CHG_AT_DISCHARGE_ENABLE,      /* enable discharge*/
    CHG_AT_START_CHARGE_SPLY,     /* start charge supply*/
    CHG_AT_CHARGE_MODE_INVALID
} ENUM_CHG_MODE_TYPE;
typedef enum 
{
    NO_BATT_MODE = 0,          /*无电池*/
    KNOCK_DOWN_BATT_MODE = 1,  /*可拆卸电池*/
    FIX_BATT_MODE = 2,         /*一体化电池*/  
    INVALID_BATT_MODE  
}BATT_FIX_MODE;
/*查询充电状态和模式*/
typedef struct
{
    int chg_state;
    int chg_mode;
}CHG_TCHRENABLE_TYPE;
#endif

#if (MBB_COMMON == FEATURE_ON)

int app_get_battery_state(BATT_STATE_S *battery_state);
#define mdrv_misc_get_battery_state app_get_battery_state
/**********************************************************************
函 数 名  : chg_tbat_read
功能描述  : AT读接口
输入参数  : 操作方式:读(校准前/后)电池电量还是读电压校准值
输出参数  : 电池电量/电池电压校准值
返 回 值  : 是否操作成功
注意事项  : 
***********************************************************************/
int chg_tbat_read(ENUM_CHG_ATCM_TYPE atID, void *pItem);
/**********************************************************************
函 数 名  : chg_tbat_write
功能描述  : AT写接口:设置电池电量校准值
输入参数  : 4.2V和3.4V对应的ADC采样值
输出参数  : 无
返 回 值  : 是否操作成功
注意事项  : 无
***********************************************************************/
int chg_tbat_write(ENUM_CHG_ATCM_TYPE atID, void *pItem);
/**********************************************************************
函 数 名  : chg_tbat_charge_mode_set
功能描述  : 设置电池充电或者放电
输入参数  : 命令参数
输出参数  : none
返 回 值  : CHG_OK:success,CHG_ERROR:fail
注意事项  : 
***********************************************************************/
int chg_tbat_charge_mode_set(ENUM_CHG_MODE_TYPE cmd, unsigned int arg);
/**********************************************************************
函 数 名  :  chg_tbat_status_get
功能描述  :  TBAT AT^TCHRENABLE?是否需要补电
输入参数  : 无
输出参数  : 无
返 回 值      : 1:需要补电
			    0:不需要补电
注意事项  : 无
***********************************************************************/
int chg_tbat_is_batt_status_ok(void);
/**********************************************************************
函 数 名  : chg_tbat_get_charging_status
功能描述  : 查询当前充电状态
输入参数  : none
输出参数  : none
返 回 值  : 1:charging,0:no
注意事项  : 
***********************************************************************/
int chg_tbat_get_charging_status(void);
#define mdrv_misc_get_charging_status chg_tbat_get_charging_status
/**********************************************************************
函 数 名  : chg_tbat_get_charging_status
功能描述  : 查询当前充电模式
输入参数  : none
输出参数  : none
返 回 值  : 0:非充电；1：涓充；2：快充
注意事项  : 
***********************************************************************/
int chg_tbat_get_charging_mode(void);
/**********************************************************************
函 数 名  : chg_tbat_get_discharging_status
功能描述  : 查询当前是否是suspend模式
输入参数  : none
输出参数  : none
返 回 值  : 1:yes,0:no
注意事项  : 
***********************************************************************/
int chg_tbat_get_discharging_status(void);
/**********************************************************************
函 数 名  : chg_tbat_sfeature_inquiry_extchg
功能描述  : 查询是否支持对外充电
输入参数  : none
输出参数  : none
返 回 值  : 1:yes,0:no
注意事项  : 
***********************************************************************/
int chg_tbat_sfeature_inquiry_extchg(void);
/**********************************************************************
函 数 名  : chg_tbat_sfeature_inquiry_wireless
功能描述  : 查询是否支持无线充电
输入参数  : none
输出参数  : none
返 回 值  : 1:yes,0:no
注意事项  : 
***********************************************************************/
int chg_tbat_sfeature_inquiry_wireless(void);
/**********************************************************************
函 数 名  : chg_tbat_sfeature_inquiry_battery
功能描述  : 查询电池电压典型值
输入参数  : none
输出参数  : none
返 回 值  : 
注意事项  : 
***********************************************************************/
int chg_tbat_sfeature_inquiry_battery(void);
/**********************************************************************
函 数 名  : chg_MNTN_get_batt_state
功能描述  : AT读接口
输入参数  : 查询是否是可拆卸电池
输出参数  : none
返 回 值  : 1:可拆卸，2：不可拆卸
注意事项  : 
***********************************************************************/
int chg_MNTN_get_batt_state(void);
/**********************************************************************
函 数 名  : chg_wireless_mmi_test
功能描述  : AT读接口，无线充电电路检测
输入参数  : none
输出参数  : none
返 回 值  : 
注意事项  : 
***********************************************************************/
int chg_wireless_mmi_test(void);
/**********************************************************************
函 数 名  : chg_extchg_mmi_test
功能描述  : AT读接口，对外充电电路检测
输入参数  : none
输出参数  : none
返 回 值  : 
注意事项  : 
***********************************************************************/
int chg_extchg_mmi_test(void);
/**********************************************************************
函 数 名  : chg_extchg_mmi_test
功能描述  : AT读接口，获取AT^TCHRENABLE设置的结果
输入参数  : none
输出参数  : none
返 回 值  : 
注意事项  : 
***********************************************************************/
int chg_tbat_get_tchrenable_status(CHG_TCHRENABLE_TYPE *tchrenable_state);
/************************************************************************  
函 数 名  : chg_getCbcState
功能描述  : 在线升级AT^CBC
         pucBcs
            0: 电池在供电
            1: 与电池连接电池未供电
            2: 没有与电池连接
         pucBcl
            0:电量不足或没有与电池连接
             100: 剩余电量百分比
输入参数  : none
输出参数  : pucBcs，pucBcl
返 回 值  : 
注意事项  : 0 Success, -1 failed  
 ***********************************************************************/
int chg_getCbcState(unsigned char  *pucBcs ,unsigned char   *pucBcl);
#define mdrv_misc_get_cbc_state chg_getCbcState

/*****************************************************************************
 函 数 名  : mdrv_misc_set_charge_state
 功能描述  :使能或者禁止充电
 输入参数  :ulState      0:禁止充电
 						1:使能充电
 输出参数  :无
 返回值：    无
*****************************************************************************/
void mdrv_misc_set_charge_state(unsigned long ulState);

/*****************************************************************************
 函 数 名  : mdrv_misc_get_charge_state
 功能描述  :查询
 输入参数  :
 输出参数  :无
 返回值：    无
*****************************************************************************/
int mdrv_misc_get_charge_state(void);

/*****************************************************************************
 函 数 名  : mdrv_misc_sply_battery
 功能描述  :查询
 输入参数  :
 输出参数  :无
 返回值：    无
*****************************************************************************/
int mdrv_misc_sply_battery(void);

#endif /*MBB_COMMON*/


#ifdef __cplusplus
}
#endif
#endif

