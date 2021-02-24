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

#include <osl_types.h>
#include "mdrv_chg.h"

/*lint --e{715,818}*/

/*************************CHG模块START********************************/

/*****************************************************************************
 函 数 名  : chg_getCbcState
 功能描述  : 返回电池状态和电量
 输入参数  :pusBcs 0:电池在供电 1:与电池连接电池未供电 2:没有与电池连接
                          pucBcl  0:电量不足或没有与电池连接
 输出参数  : pusBcs 0:电池在供电 1:与电池连接电池未供电 2:没有与电池连接
                          pucBcl  0:电量不足或没有与电池连接
 返回值：   0 操作成功
                         -1操作失败

*****************************************************************************/
int chg_getCbcState(unsigned char *pusBcs,unsigned char *pucBcl)
{
    return -1;
}
/*****************************************************************************
 函 数 名  : mdrv_misc_get_charge_state
 功能描述  :查询
 输入参数  :
 输出参数  :无
 返回值：    无
*****************************************************************************/
int mdrv_misc_get_charge_state(void)
{
    return -1;
}

/**********************************************************************
函 数 名  : chg_tbat_read
功能描述  : AT读接口
输入参数  : 操作方式:读(校准前/后)电池电量还是读电压校准值
输出参数  : 电池电量/电池电压校准值
返 回 值  : 是否操作成功
注意事项  : 
***********************************************************************/
int chg_tbat_read(ENUM_CHG_ATCM_TYPE atID, void *pItem)
{
    return -1;
}

/**********************************************************************
函 数 名  : chg_tbat_write
功能描述  : AT写接口:设置电池电量校准值
输入参数  : 4.2V和3.4V对应的ADC采样值
输出参数  : 无
返 回 值  : 是否操作成功
注意事项  : 无
***********************************************************************/
int chg_tbat_write(ENUM_CHG_ATCM_TYPE atID, void *pItem)
{
    return -1;
}
/**********************************************************************
函 数 名  : chg_tbat_charge_mode_set
功能描述  : 设置电池充电或者放电
输入参数  : 命令参数
输出参数  : none
返 回 值  : CHG_OK:success,CHG_ERROR:fail
注意事项  : 
***********************************************************************/
int chg_tbat_charge_mode_set(ENUM_CHG_MODE_TYPE cmd, unsigned int arg)
{
    return -1;
}
/**********************************************************************
函 数 名  :  chg_tbat_status_get
功能描述  :  TBAT AT^TCHRENABLE?是否需要补电
输入参数  : 无
输出参数  : 无
返 回 值      : 1:需要补电
			    0:不需要补电
注意事项  : 无
***********************************************************************/
int chg_tbat_is_batt_status_ok(void)
{
    return -1;
}
/**********************************************************************
函 数 名  : chg_tbat_get_charging_status
功能描述  : 查询当前充电状态
输入参数  : none
输出参数  : none
返 回 值  : 1:charging,0:no
注意事项  : 
***********************************************************************/
int chg_tbat_get_charging_status(void)
{
    return -1;
}
/**********************************************************************
函 数 名  : chg_tbat_get_charging_status
功能描述  : 查询当前充电模式
输入参数  : none
输出参数  : none
返 回 值  : 0:非充电；1：涓充；2：快充
注意事项  : 
***********************************************************************/
int chg_tbat_get_charging_mode(void)
{
    return -1;
}
/**********************************************************************
函 数 名  : chg_tbat_get_discharging_status
功能描述  : 查询当前是否是suspend模式
输入参数  : none
输出参数  : none
返 回 值  : 1:yes,0:no
注意事项  : 
***********************************************************************/
int chg_tbat_get_discharging_status(void)
{
    return -1;
}
/**********************************************************************
函 数 名  : chg_tbat_sfeature_inquiry_extchg
功能描述  : 查询是否支持对外充电
输入参数  : none
输出参数  : none
返 回 值  : 1:yes,0:no
注意事项  : 
***********************************************************************/
int chg_tbat_sfeature_inquiry_extchg(void)
{
    return -1;
}
/**********************************************************************
函 数 名  : chg_tbat_sfeature_inquiry_wireless
功能描述  : 查询是否支持无线充电
输入参数  : none
输出参数  : none
返 回 值  : 1:yes,0:no
注意事项  : 
***********************************************************************/
int chg_tbat_sfeature_inquiry_wireless(void)
{
    return -1;
}
/**********************************************************************
函 数 名  : chg_tbat_sfeature_inquiry_battery
功能描述  : 查询电池电压典型值
输入参数  : none
输出参数  : none
返 回 值  : 
注意事项  : 
***********************************************************************/
int chg_tbat_sfeature_inquiry_battery(void)
{
    return -1;
}
/**********************************************************************
函 数 名  : chg_MNTN_get_batt_state
功能描述  : AT读接口
输入参数  : 查询是否是可拆卸电池
输出参数  : none
返 回 值  : 1:可拆卸，2：不可拆卸
注意事项  : 
***********************************************************************/
int chg_MNTN_get_batt_state(void)
{
    return -1;
}
/**********************************************************************
函 数 名  : chg_wireless_mmi_test
功能描述  : AT读接口，无线充电电路检测
输入参数  : none
输出参数  : none
返 回 值  : 
注意事项  : 
***********************************************************************/
int chg_wireless_mmi_test(void)
{
    return -1;
}
/**********************************************************************
函 数 名  : chg_extchg_mmi_test
功能描述  : AT读接口，对外充电电路检测
输入参数  : none
输出参数  : none
返 回 值  : 
注意事项  : 
***********************************************************************/
int chg_extchg_mmi_test(void)
{
    return -1;
}

/**********************************************************************
函 数 名  : chg_extchg_mmi_test
功能描述  : AT读接口，获取AT^TCHRENABLE设置的结果
输入参数  : none
输出参数  : none
返 回 值  : 
注意事项  : 
***********************************************************************/
int chg_tbat_get_tchrenable_status(CHG_TCHRENABLE_TYPE *tchrenable_state)
{
    return -1;
}

/*****************************************************************************
 函 数 名  : mdrv_misc_get_battery_state
 功能描述  :获取底层电池状态信息
 输入参数  :battery_state 电量信息
 输出参数  :battery_state 电量信息
 返回值：   0 操作成功
                         -1操作失败

*****************************************************************************/
int app_get_battery_state(BATT_STATE_S *battery_state)
{
    return -1;
}

/*****************************************************************************
 函 数 名  : mdrv_misc_set_charge_state
 功能描述  :使能或者禁止充电
 输入参数  :ulState      0:禁止充电
 						1:使能充电
 输出参数  :无
 返回值：    无
*****************************************************************************/
void mdrv_misc_set_charge_state(unsigned long ulState)
{

}

/*****************************************************************************
 函 数 名  : BSP_CHG_Sply
 功能描述  :查询
 输入参数  :
 输出参数  :无
 返回值：    无
*****************************************************************************/
int mdrv_misc_sply_battery(void)
{
    return -1;
}
/*************************CHG模块 END*********************************/


