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


#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <asm/mach/irq.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <drv_comm.h>
#include <asm/uaccess.h>
#include <osl_spinlock.h>
#include <drv_nv_id.h>
#include <drv_nv_def.h>
#include <bsp_nvim.h>
#include "bsp_coul.h"
#include "pmic_coul.h"
#include "pmic_inner.h"
#ifdef CONFIG_OF
#include <linux/of_address.h>
#include <linux/of.h>
#endif

#ifdef CONFIG_COUL
#include "hi_pmu.h"
#include "bsp_pmu.h"
#endif


struct coul_data g_coul_st;

#define COUL_BEFORE_TIMES_MAX    10
#define R_COUL_MOHM 10/*参考电阻:10 mohm*/
/*COUL校准配置nv值*/
COUL_CALI_NV_TYPE    coul_cali_config={1000,0,1000,0};/*默认值，无校准*/

//u32 g_pmu_base_addr = 0;

/* 写整个寄存器 */
void coul_reg_write(u16 reg, u8 val)
{
	pmic_reg_write((u32)reg, (u32)val);
}
/* 读寄存器 */
u8 coul_reg_read(u16 reg)
{
    u32 val = 0;
	pmic_reg_read((u32)reg, &val);
    return (u8)val;
}
/* 写寄存器的某些位 */
void coul_reg_mask_write(u16 reg,u8 val,u8 lowbit, u8 highbit)
{
    u8 mask;
    u32 reg_tmp;
    mask   = (1 << (highbit - lowbit + 1) -1) << lowbit;
	pmic_reg_read(reg, &reg_tmp);
    reg_tmp &= ~mask;
    reg_tmp |= val << lowbit;
	coul_reg_write(reg, reg_tmp);
}
#if 0
/*****************************************************************************
 函 数 名  : coul_battery_voltage
 功能描述  : 寄存器值转换成电压
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
/**
 * convert_regval2uv
 * 1 bit = 225.1196 uv = 4.8/21322 V = 4.8/21322 * 1000 * 1000 uV = 24 * 1000 * 100/ 10661 uV
 * convert regval to uv
 */
u64 coul_convert_regval2uv(unsigned short reg_val)
{
    u64 temp;

    if (reg_val & 0x8000)
    {
        return -1;
    }

    temp = (u64)((u64)(reg_val)  * (u64)(24 * 1000 * 100));

    temp = div_u64(temp, 10661);

#if 1 /* for debug */
    temp = (u64) coul_cali_config.v_offset_a *temp;
/*    temp = div_u64(temp, 1000000);*/
    temp += coul_cali_config.v_offset_b;
#endif

    return temp;
}
/*****************************************************************************
 函 数 名  : coul_convert_regval2ua
 功能描述  : 电流转换函数
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
/**
 * convert_regval2ua
 * 10 mohm resistance: 1 bit = 5/10661 A = 5*1000*1000 / 10661 uA
 * 20 mohm resistance: 1 bit = 10 mohm / 2
 * 30 mohm resistance: 1 bit = 10 mohm / 3
 * ...
 * high bit = 0 is in, 1 is out
 * convert regval to ua
 */
s64 coul_convert_regval2ua(short reg_val)
{
    int ret;
    s64 temp;

    ret = reg_val;
    temp = (s64)(ret) * (s64)(1000 * 1000 * 5);
    temp = div_s64(temp, 10661);

    ret = temp / (R_COUL_MOHM/10);

    ret = -ret;/*何意?*/

#if 1 /* for debug */
    temp = (s64) coul_cali_config.c_offset_a *ret;
 /*   ret = div_s64(temp, 1000000);*/
    ret += coul_cali_config.c_offset_b;
#endif

    return ret;
}
/*****************************************************************************
 函 数 名  : coul_battery_voltage
 功能描述  : 电量转换函数(单位:uC)
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
/**
 * convert_regval2uc
 * 1 bit = 2.345*110*10^(-6) C = 2.5795*10^(-5) C = 25795 / 1000 uC
 * convert regval to uv
 */
u64 coul_convert_regval2uc(unsigned short reg_val)
{
    int ret;
    s64 temp;

    ret = reg_val;
    temp = (s64)(ret) * (s64)(25795);
    temp = div_s64(temp, 1000);

    return temp;
}
#endif
/*****************************************************************************
 函 数 名  : coul_convert_regval2mv
 功能描述  : 寄存器值转换成电压
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
/**
 * convert_regval2uv
 * 1 bit = 225.1196 uv = 4.8/21322 V = 4.8/21322 * 1000 * 1000 uV = 24 * 1000 * 100/ 10661 uV
 * convert regval to mv
 */
unsigned int coul_convert_regval2mv(unsigned short reg_val)
{
    unsigned int temp;

    if (reg_val & 0x8000)/*电压不会是负值*/
    {
        return BSP_COUL_UNVALID;
    }

    temp = (unsigned int)(reg_val*225);

    temp = temp / 1000;/*单位转化为mV*/

    return temp;
}
/*****************************************************************************
 函 数 名  : coul_convert_regval2ua
 功能描述  : 电流转换函数
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
/**
 * convert_regval2ua
 * 10 mohm resistance: 1 bit = 5/10661 A = 5*1000*1000 / 10661 uA = 469uA
 * 20 mohm resistance: 1 bit = 10 mohm / 2
 * 30 mohm resistance: 1 bit = 10 mohm / 3
 * ...
 * high bit = 0 is in, 1 is out
 * convert regval to ma
 */
signed int coul_convert_regval2ma(signed short reg_val)
{
    signed curr;
    s32 temp;

    temp = (s32)reg_val * 469;/*uA*/

    curr = temp / (R_COUL_MOHM/10);/*uA*/

    curr = curr / 1000;/*转化为mA*/

    return curr;
}
/*****************************************************************************
 函 数 名  : coul_battery_voltage
 功能描述  : 电量转换函数(单位:uC)
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned long long
 调用函数  :
 被调函数  :
*****************************************************************************/
/**
 * convert_regval2uc
 * 1 bit = 469*110*10^(-9) C = 5.159*10^(-5) C = 51.59  uC
 * convert regval to uv
 */
unsigned long long coul_convert_regval2uc(unsigned int reg_val)
{
    unsigned long long temp;

    temp = (unsigned long long)reg_val * 52;/*uc*/
 /*   temp = (temp / 1000);*//*mC**/

    return temp;
}
/*****************************************************************************
 函 数 名  : coul_cali_voltage
 功能描述  : 获取校准后的电压
 输入参数  : voltage:未经校准的电压
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned int coul_cali_voltage(unsigned voltage)
{
    int calied_volt;

    calied_volt = (int)(coul_cali_config.v_offset_a *voltage);
    calied_volt = calied_volt / 1000;/*消除由于校准斜率计算产生的数量级误差*/
    calied_volt += coul_cali_config.v_offset_b;

    return (unsigned int)calied_volt;
}
/*****************************************************************************
 函 数 名  : coul_cali_current
 功能描述  : 获取校准后的电流
 输入参数  : cur:未经校准的电流
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
signed int coul_cali_current(signed cur)
{
    signed int calied_cur;

    calied_cur = (signed int)coul_cali_config.c_offset_a * cur;
    calied_cur = calied_cur / 1000;/*消除由于校准斜率计算产生的数量级误差*/
    calied_cur += coul_cali_config.c_offset_b;

    return calied_cur;
}
/*****************************************************************************
 函 数 名  : coul_convert_regval2mv_calied
 功能描述  : 由寄存器值获取校准后的电压
 输入参数  : voltage:未经校准的电压
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned int coul_convert_regval2mv_calied(unsigned short reg_val)
{
    unsigned int uncali_volt;
    unsigned int calied_volt;

    uncali_volt = coul_convert_regval2mv(reg_val);
    calied_volt = coul_cali_voltage(uncali_volt);

    return calied_volt;
}
/*****************************************************************************
 函 数 名  : coul_convert_regval2calivolt
 功能描述  : 由寄存器值获取校准后的电流
 输入参数  : cur:未经校准的电流
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
signed int coul_convert_regval2ma_calied(signed short reg_val)
{
    signed int uncali_cur;
    signed int calied_cur;

    uncali_cur = coul_convert_regval2ma(reg_val);
    calied_cur = coul_cali_current(uncali_cur);

    return calied_cur;
}
/*****************************************************************************
 函 数 名  : coul_voltage_regval
 功能描述  : 当前电压寄存器获取
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned short
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned short coul_voltage_regval(void)
{
    unsigned char val1;
    unsigned char val2;
    unsigned short val = 0;

    val1 = coul_reg_read(g_coul_st.coul_reg.v_out_pre0.v_out0_pre0);
    val2 = coul_reg_read(g_coul_st.coul_reg.v_out_pre0.v_out1_pre0);

    val = (unsigned short)(val1 | ((u32)(val2 & 0x7f)<< 8));/*电压为正值，最高位为符号位，不关注*/
    return val;
}
/*****************************************************************************
 函 数 名  : coul_voltage_before_regval
 功能描述  : 获取当前电压前times次电压寄存器值，单位:uv
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned short
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned short coul_voltage_before_regval(unsigned int times)
{
    unsigned char val1;
    unsigned char val2;
    unsigned short val = 0;

    val1 = coul_reg_read((u16)(g_coul_st.coul_reg.v_out_pre0.v_out0_pre0 + 2 * times));
    val2 = coul_reg_read((u16)(g_coul_st.coul_reg.v_out_pre0.v_out1_pre0 + 2 * times ));

    val = (unsigned short)(val1 | ((u32)(val2 & 0xef)<< 8));/*电压为正值，最高位为符号位，不关注*/
    return val;
}
/*****************************************************************************
 函 数 名  : coul_battery_current_regval
 功能描述  : 当前电流寄存器值获取
 输入参数  : void
 输出参数  : 无
 返 回 值  : signed short
 调用函数  :
 被调函数  :
*****************************************************************************/
signed short coul_current_regval(void)
{
    unsigned char val1;
    unsigned char val2;
    signed short val = 0;

    val1 = coul_reg_read(g_coul_st.coul_reg.cur_pre0.cur0_pre0);
    val2 = coul_reg_read(g_coul_st.coul_reg.cur_pre0.cur1_pre0);

    val = (signed short)(val1 | ((u32)val2 << 8));

    return val;
}
/*****************************************************************************
 函 数 名  : coul_current_before_regval
 功能描述  : 获取当前电压前times次电流寄存器值
 输入参数  : times:要获取的前几次的电流值([范围[1,20])
 输出参数  : 无
 返 回 值  : signed short
 调用函数  :
 被调函数  :
*****************************************************************************/
signed short coul_current_before_regval(unsigned int times)
{
    unsigned char val1;
    unsigned char val2;
    signed short val = 0;

    val1 = coul_reg_read((u16)(g_coul_st.coul_reg.cur_pre0.cur0_pre0 + 2 * times));
    val2 = coul_reg_read((u16)(g_coul_st.coul_reg.cur_pre0.cur1_pre0 + 2 * times));/*最高位代表符号位*/

    val = (signed short)(val1 | ((u32)val2 << 8));
    return val;
}
/*****************************************************************************
 函 数 名  : coul_in_capacity_regval
 功能描述  : 流入电量寄存器值获取
 输入参数  : void
 输出参数  : 无
 返 回 值  : signed int
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned int coul_in_capacity_regval(void)
{
    unsigned char val1;
    unsigned char val2;
    unsigned char val3;
    unsigned char val4;
    unsigned int in;

    val1 = coul_reg_read(g_coul_st.coul_reg.cl_in.cl_in0);
    val2 = coul_reg_read(g_coul_st.coul_reg.cl_in.cl_in1);
    val3 = coul_reg_read(g_coul_st.coul_reg.cl_in.cl_in2);
    val4 = coul_reg_read(g_coul_st.coul_reg.cl_in.cl_in3);
    in = (unsigned int)(val1 | ((u32)val2 << 8) | ((u32)val3 << 16) | ((u32)val4 << 24));

    return in;
}
/*****************************************************************************
 函 数 名  : coul_out_capacity_regval
 功能描述  : 流出电量寄存器值获取
 输入参数  : void
 输出参数  : 无
 返 回 值  : signed int
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned int coul_out_capacity_regval(void)
{
    unsigned char val1;
    unsigned char val2;
    unsigned char val3;
    unsigned char val4;
    unsigned int out;

    val1 = coul_reg_read(g_coul_st.coul_reg.cl_out.cl_out0);
    val2 = coul_reg_read(g_coul_st.coul_reg.cl_out.cl_out1);
    val3 = coul_reg_read(g_coul_st.coul_reg.cl_out.cl_out2);
    val4 = coul_reg_read(g_coul_st.coul_reg.cl_out.cl_out3);

    out = (unsigned int)(val1 | ((u32)val2 << 8) | ((u32)val3 << 16) | ((u32)val4 << 24));

    return out;
}
/*****************************************************************************
 函 数 名  : coul_ocv_data_regval
 功能描述  : 电池电压开路电压数值寄存器值获取
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned short
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned short coul_ocv_data_regval(void)
{
    unsigned char val1;
    unsigned char val2;
    unsigned short val = 0;

    val1 = coul_reg_read(g_coul_st.coul_reg.v_ocv_data.v_ocv_data0);
    val2 = coul_reg_read(g_coul_st.coul_reg.v_ocv_data.v_ocv_data1);

    val = (unsigned short)(val1 | ((u32)val2 << 8));
    return val;
}
/*****************************************************************************
 函 数 名  : coul_ocv_offset_regval
 功能描述  : 电池电压开路电压误差寄存器值获取
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned short
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned short coul_ocv_offset_regval(void)
{
    unsigned char val1;
    unsigned char val2;
    unsigned short val = 0;

    val1 = coul_reg_read(g_coul_st.coul_reg.off_vol.off_vol0);
    val2 = coul_reg_read(g_coul_st.coul_reg.off_vol.off_vol1);

    val = (unsigned short)(val1 | ((u32)val2 << 8));
    return val;
}
/*****************************************************************************
 函 数 名  : coul_battery_charge_time_regval
 功能描述  : 库仑计充电时间计数寄存器值获取
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned int coul_charge_time_regval(void)
{
    unsigned char val1;
    unsigned char val2;
    unsigned char val3;
    unsigned char val4;

    val1 = coul_reg_read(g_coul_st.coul_reg.chg_timer.chg_timer0);
    val2 = coul_reg_read(g_coul_st.coul_reg.chg_timer.chg_timer1);
    val3 = coul_reg_read(g_coul_st.coul_reg.chg_timer.chg_timer2);
    val4 = coul_reg_read(g_coul_st.coul_reg.chg_timer.chg_timer3);

    return (unsigned int)val1 | ((u32)val2 << 8) | ((u32)val3 << 16) | ((u32)val4 << 24);
}

/*****************************************************************************
 函 数 名  : coul_battery_discharge_time_regval
 功能描述  : 放电时间寄存器值获取
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned int coul_discharge_time_regval(void)
{
    unsigned char val1;
    unsigned char val2;
    unsigned char val3;
    unsigned char val4;

    val1 = coul_reg_read(g_coul_st.coul_reg.load_timer.load_timer0);
    val2 = coul_reg_read(g_coul_st.coul_reg.load_timer.load_timer1);
    val3 = coul_reg_read(g_coul_st.coul_reg.load_timer.load_timer2);
    val4 = coul_reg_read(g_coul_st.coul_reg.load_timer.load_timer3);

    return (unsigned int)val1 | ((u32)val2 << 8) | ((u32)val3 << 16) | ((u32)val4 << 24);
}

/*****************************************************************************
 函 数 名  : coul_set_vbat_value
 功能描述  : 设置电池低电值(需要校准转化后的数据)
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :
*****************************************************************************/
void coul_voltage_low_regval_set(unsigned short value)
{
    unsigned char val1 = (value & 0xff);
    unsigned char val2 = (value >> 8) & 0xff;

    coul_reg_write(g_coul_st.coul_reg.v_int.v_int0, val1);
    coul_reg_write(g_coul_st.coul_reg.v_int.v_int1, val2);
}

/*****************************************************************************
 函 数 名  : coul_power_ctrl
 功能描述  :
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :
*****************************************************************************/
void coul_power_ctrl(COUL_POWER_CTRL ctrl)
{
    unsigned char val;
    coul_irqflags_t coul_flags = 0;

    spin_lock_irqsave(&g_coul_st.lock,coul_flags);
    val = coul_reg_read(g_coul_st.coul_reg.coul_ctrl_onoff_reg.offset);
    if(COUL_POWER_ON == ctrl){
        val |= 0x1 << g_coul_st.coul_reg.coul_ctrl_onoff_reg.lowbit;
    }else{
        val &= ~(0x1 << g_coul_st.coul_reg.coul_ctrl_onoff_reg.lowbit);
    }
    coul_reg_write(g_coul_st.coul_reg.coul_ctrl_onoff_reg.offset, val);
    spin_unlock_irqrestore(&g_coul_st.lock,coul_flags);
}

/*****************************************************************************
 函 数 名  : coul_cali_ctrl
 功能描述  : 是否强制进入校准状态
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :
*****************************************************************************/
void coul_cali_ctrl(COUL_CALI_CTRL ctrl)
{
    unsigned char val;
    coul_irqflags_t coul_flags = 0;

    spin_lock_irqsave(&g_coul_st.lock,coul_flags);
    val = coul_reg_read(g_coul_st.coul_reg.calibration_ctrl.offset);
    if(COUL_CALI_ON == ctrl){
        val |= 0x1 << g_coul_st.coul_reg.calibration_ctrl.lowbit;
    }else{
        val &= ~(0x1 << g_coul_st.coul_reg.calibration_ctrl.lowbit);
    }
    coul_reg_write(g_coul_st.coul_reg.calibration_ctrl.offset, val);
    spin_unlock_irqrestore(&g_coul_st.lock,coul_flags);
}

/*****************************************************************************
 函 数 名  : coul_reflash_ctrl
 功能描述  : 刷新控制
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :
*****************************************************************************/
void coul_reflash_ctrl(COUL_REFLASH_CTRL ctrl)
{
    unsigned char val;
    coul_irqflags_t coul_flags = 0;

    spin_lock_irqsave(&g_coul_st.lock,coul_flags);
    val = coul_reg_read(g_coul_st.coul_reg.reflash_value_ctrl.offset);
    
    if(COUL_REFLASH_ECO == ctrl){
        val |= 0x1 << g_coul_st.coul_reg.reflash_value_ctrl.lowbit;
    }else{
        val &= ~(0x1 << g_coul_st.coul_reg.reflash_value_ctrl.lowbit);
    }
    coul_reg_write(g_coul_st.coul_reg.reflash_value_ctrl.offset, val);
    spin_unlock_irqrestore(&g_coul_st.lock,coul_flags);
}

/*****************************************************************************
 函 数 名  : coul_eco_ctrl
 功能描述  :是否强制进入eco模式
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :
*****************************************************************************/
void coul_eco_ctrl(COUL_ECO_CTRL ctrl)
{
    unsigned char val;
    coul_irqflags_t coul_flags = 0;

    spin_lock_irqsave(&g_coul_st.lock,coul_flags);
    val = coul_reg_read(g_coul_st.coul_reg.eco_ctrl.offset);
    if(COUL_ECO_FORCE == ctrl){
        val |= COUL_CTRL_ECO_MASK;
    }else{
        val &= ~COUL_CTRL_ECO_MASK;
    }
    coul_reg_write(g_coul_st.coul_reg.eco_ctrl.offset, val);
    spin_unlock_irqrestore(&g_coul_st.lock,coul_flags);
}

/*****************************************************************************
 函 数 名  : bsp_coul_init
 功能描述  : 库仑计模块初始化
 输入参数  : void
 输出参数  : 无
 返 回 值  : 初始化成功或失败
 调用函数  :
 被调函数  :
*****************************************************************************/
int bsp_coul_dts_init()
{
    struct device_node *dev_node = NULL;
    char node_name[32] = "";
    int ret = 0;
    u32 reg_addr[4];
    int i = 0;
    
    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,coul_app");
    if(!dev_node)
    {
        coul_err("can't get dev_node:hisilicon,coul_app!\n");
        return -1;
    }

    ret = of_property_read_u32_array(dev_node, "coul_int", \
		&g_coul_st.coul_int, (unsigned int)(sizeof(struct coul_int)/sizeof(u32)));

    ret |= of_property_read_u32_array(dev_node, "soft_rst_n", \
		&g_coul_st.coul_reg.soft_rst_n, (unsigned int)(sizeof(struct reg_info)/sizeof(u32)));

    ret |= of_property_read_u32_array(dev_node, "coul_ctrl_onoff_reg", \
		&g_coul_st.coul_reg.coul_ctrl_onoff_reg, sizeof(struct reg_info)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "calibration_ctrl", \
		&g_coul_st.coul_reg.calibration_ctrl, sizeof(struct reg_info)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "eco_filter_time", \
		&g_coul_st.coul_reg.eco_filter_time, sizeof(struct reg_info)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "reflash_value_ctrl", \
		&g_coul_st.coul_reg.reflash_value_ctrl, sizeof(struct reg_info)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "eco_ctrl", \
		&g_coul_st.coul_reg.eco_ctrl, sizeof(struct reg_info)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "eco_reflash_time", \
		&g_coul_st.coul_reg.eco_reflash_time, sizeof(struct reg_info)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "cl_in", \
		&g_coul_st.coul_reg.cl_in, sizeof(struct cl_in)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "cl_out", \
		&g_coul_st.coul_reg.cl_out, sizeof(struct cl_out)/sizeof(u32));
    
    ret |= of_property_read_u32_array(dev_node, "chg_timer", \
		&g_coul_st.coul_reg.chg_timer, sizeof(struct chg_timer)/sizeof(u32));
        
    ret |= of_property_read_u32_array(dev_node, "load_timer", \
		&g_coul_st.coul_reg.load_timer, sizeof(struct load_timer)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "cl_int", \
		&g_coul_st.coul_reg.cl_int, sizeof(struct cl_int)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "v_int", \
		&g_coul_st.coul_reg.v_int, sizeof(struct v_int)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "offset_current", \
		&g_coul_st.coul_reg.off_cur, sizeof(struct off_cur)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "offset_voltage", \
		&g_coul_st.coul_reg.off_vol, sizeof(struct off_vol)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "v_ocv_data", \
		&g_coul_st.coul_reg.v_ocv_data, sizeof(struct v_ocv_data)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "i_ocv_data", \
		&g_coul_st.coul_reg.i_ocv_data, sizeof(struct i_ocv_data)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "v_out_pre0", \
		&g_coul_st.coul_reg.v_out_pre0, sizeof(struct v_out_pre0)/sizeof(u32));

    ret |= of_property_read_u32_array(dev_node, "current_pre0", \
		&g_coul_st.coul_reg.cur_pre0, sizeof(struct cur_pre0)/sizeof(u32));

    if(ret)
    {
        coul_err("fail to get dts\n");
    }    
    return ret;
    
}


/*********************以下接口思考是否有接口需要使用**************************************/
/*****************************************************************************
 函 数 名  : bsp_coul_enable
 功能描述  : 开启库仑计
 输入参数  : void
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :
*****************************************************************************/
void bsp_coul_enable(void)
{
    coul_power_ctrl(COUL_POWER_ON);
}
/*****************************************************************************
 函 数 名  : bsp_coul_disable
 功能描述  : 关闭库仑计
 输入参数  : void
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :
*****************************************************************************/
void bsp_coul_disable(void)
{
    coul_power_ctrl(COUL_POWER_OFF);
}

/*****************************************************************************
 函 数 名  : bsp_coul_cail_on
 功能描述  : 库仑计强制校准
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  :
*****************************************************************************/
#if (FEATURE_ON == MBB_CHG_COULOMETER)
void bsp_coul_cail_on(void)
{
    coul_cali_ctrl(COUL_CALI_ON);
}
#endif

/*********************以下接口提供给产品线使用**************************************/
/*采样校准参数时调用*/
/*****************************************************************************
 函 数 名  : bsp_coul_voltage_uncali
 功能描述  : 获取未经校准当前电压，单位:mV
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned int bsp_coul_voltage_uncali(void)
{
    unsigned short  val = 0;
    unsigned int voltage = 0;

    val = coul_voltage_regval();

    voltage = coul_convert_regval2mv(val);

#if (FEATURE_ON == MBB_CHG_COULOMETER)
    coul_dbg("uncali:coul_battery_voltage is %d mV\n",voltage);
#else
    coul_err("uncali:coul_battery_voltage is %d mV\n",voltage);
#endif

    return voltage;
}
/*****************************************************************************
 函 数 名  : bsp_coul_current_uncali
 功能描述  : 获取未经校准当前电流,单位:mA
 输入参数  : void
 输出参数  : 无
 返 回 值  : signed long long
 调用函数  :
 被调函数  :
*****************************************************************************/
signed int  bsp_coul_current_uncali(void)
{
    signed short  val = 0;
    signed int bat_current = 0;

    val = coul_current_regval();

    bat_current = coul_convert_regval2ma(val);

#if (FEATURE_ON == MBB_CHG_COULOMETER)
    coul_dbg("uncali:coul_battery_current is %d mA\n",bat_current);
#else
    coul_err("uncali:coul_battery_current is %d mA\n",bat_current);
#endif

    return bat_current;
}
/*****************************************************************************
 函 数 名  : bsp_coul_voltage
 功能描述  : 当前电压获取，单位:mV
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned int bsp_coul_voltage(void)
{
    unsigned short  val = 0;
    unsigned int voltage = 0;

    val = coul_voltage_regval();

    voltage = coul_convert_regval2mv_calied(val);

#if (FEATURE_ON == MBB_CHG_COULOMETER)
    coul_dbg("calied:coul_battery_voltage is %d mV\n",voltage);
#else
    coul_err("calied:coul_battery_voltage is %d mV\n",voltage);
#endif

    return voltage;
}
/*****************************************************************************
 函 数 名  : bsp_coul_voltage_before
 功能描述  : 获取当前电压第times次电压值，单位:mV
 输入参数  : times:要获取最近的第n次的电压值([范围[1,20])
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned int  bsp_coul_voltage_before(unsigned int times)
{
    unsigned short  val = 0;
    unsigned int  voltage = 0;

#if (FEATURE_ON == MBB_CHG_COULOMETER)
    if(COUL_BEFORE_TIMES_MAX <= times)
#else
    if(COUL_BEFORE_TIMES_MAX < times)
#endif
    {
        coul_err("coul can only support 19 times before!");
        return BSP_COUL_UNVALID;/*怎样区分错误码和电压值?*/
    }

    val = coul_voltage_before_regval(times);

    voltage = coul_convert_regval2mv_calied(val);

#if (FEATURE_ON == MBB_CHG_COULOMETER)
    coul_dbg("calied:before %d times voltage is %d mV\n",times,voltage);
#else
    coul_err("calied:before %d times voltage is %d mV\n",times,voltage);
#endif

    return voltage;
}
/*****************************************************************************
 函 数 名  : bsp_coul_current
 功能描述  : 当前电流获取,单位:mA
 输入参数  : void
 输出参数  : 无
 返 回 值  : signed long long
 调用函数  :
 被调函数  :
*****************************************************************************/
signed int  bsp_coul_current(void)
{
    signed short  val = 0;
    signed int bat_current = 0;

    val = coul_current_regval();

    bat_current = coul_convert_regval2ma_calied(val);

#if (FEATURE_ON == MBB_CHG_COULOMETER)
    coul_dbg("calied:coul_battery_current is %d mA\n",bat_current);
#else
    coul_err("calied:coul_battery_current is %d mA\n",bat_current);
#endif

    return bat_current;
}
/*****************************************************************************
 函 数 名  : bsp_coul_current_before
 功能描述  : 获取当前电流前times次电流值，单位:mA
 输入参数  : times:要获取的前几次的电流值([范围[1,20])
 输出参数  : 无
 返 回 值  : unsigned long long
 调用函数  :
 被调函数  :
*****************************************************************************/
signed int bsp_coul_current_before(unsigned int times)
{
    signed short  val = 0;
    signed int  bat_current = 0;

#if (FEATURE_ON == MBB_CHG_COULOMETER)
    if(COUL_BEFORE_TIMES_MAX <= times)
#else
    if(COUL_BEFORE_TIMES_MAX < times)
#endif
    {
        coul_err("coul can only support 19 times before!");
        return BSP_COUL_ERR;/*怎样区分错误码和电压值?*/
    }

    val = coul_current_before_regval(times);

    bat_current = coul_convert_regval2ma_calied(val);

#if (FEATURE_ON == MBB_CHG_COULOMETER)
    coul_dbg("calied:before %d times current is %d mA\n",times,bat_current);
#else
    coul_err("calied:before %d times current is %d mA\n",times,bat_current);
#endif

    return bat_current;
}
/*****************************************************************************
 函 数 名  : bsp_coul_in_capacity
 功能描述  : 电池流入电量获取,单位:uC
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned long long
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned long long bsp_coul_in_capacity(void)
{
    unsigned int  val = 0;
    unsigned long long capacity = 0;

    val = coul_in_capacity_regval();

    capacity = coul_convert_regval2uc(val);

#if (FEATURE_ON == MBB_CHG_COULOMETER)
    coul_dbg("in capacity is %lld uC\n",capacity);
#else
    coul_err("in capacity is %lld uC\n",capacity);
#endif

    return capacity;
}
/*****************************************************************************
 函 数 名  : bsp_coul_out_capacity
 功能描述  : 电池流出电量获取,单位:mC
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned long long
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned long long bsp_coul_out_capacity(void)
{
    unsigned int  val = 0;
    unsigned long long capacity = 0;

    val = coul_out_capacity_regval();

    capacity = coul_convert_regval2uc(val);

#if (FEATURE_ON == MBB_CHG_COULOMETER)
    coul_dbg("out capacity is %lld uC\n",capacity);
#else
    coul_err("out capacity is %lld uC\n",capacity);
#endif

    return capacity;
}
/*****************************************************************************
 函 数 名  : bsp_coul_charge_time
 功能描述  : 充电时间获取,单位:s
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned int bsp_coul_charge_time(void)
{
    unsigned int  time = 0;

    time = coul_charge_time_regval();

#if (FEATURE_ON == MBB_CHG_COULOMETER)
    coul_dbg("charge_time is %d s\n",time);
#else
    coul_err("charge_time is %d s\n",time);
#endif

    return time;
}
/*****************************************************************************
 函 数 名  : bsp_coul_discharge_time
 功能描述  : 充电时间获取,单位:s
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned int bsp_coul_discharge_time(void)
{
    unsigned int  time = 0;

    time = coul_discharge_time_regval();

#if (FEATURE_ON == MBB_CHG_COULOMETER)
    coul_dbg("discharge_time is %d s\n",time);
#else
    coul_err("discharge_time is %d s\n",time);
#endif

    return time;
}
/*****************************************************************************
 函 数 名  : bsp_coul_eco_filter_time
 功能描述  : 设置库仑计eco滤波时间
 输入参数  : void
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :
*****************************************************************************/
void bsp_coul_eco_filter_time(COUL_FILETR_TIME filter_time)
{
    coul_irqflags_t coul_flags = 0;

    spin_lock_irqsave(&g_coul_st.lock,coul_flags);
    coul_reg_mask_write(g_coul_st.coul_reg.eco_filter_time.offset, filter_time, \
        g_coul_st.coul_reg.eco_filter_time.lowbit, g_coul_st.coul_reg.eco_filter_time.highbit);
    spin_unlock_irqrestore(&g_coul_st.lock,coul_flags);
}
/*****************************************************************************
 函 数 名  : bsp_coul_int_register
 功能描述  : 注册库仑计中断回调函数
 输入参数  : int:中断号；func:中断处理回调函数，data:回调函数参数指针
 输出参数  : 无
 返 回 值  : 注册成功或失败
 调用函数  :
 被调函数  :
*****************************************************************************/
int bsp_coul_int_register(COUL_INT_TYPE irq,COUL_INT_FUNC func,void *data)
{
    /*向coul 进行中断注册*/
	if (irq < 0 || irq >= g_coul_st.coul_int.int_num) {
        coul_err("coul irq is error,please check irq=%d\n", (int)irq);
        return BSP_COUL_ERR;
    }
	
	g_coul_st.irq_handler[irq].irq_num = irq;
	g_coul_st.irq_handler[irq].routine = func;
	g_coul_st.irq_handler[irq].data = data;
   
    return BSP_COUL_OK;
}

void bsp_coul_int_mask(unsigned int irq)
{
    u32 reg_addr,reg_bit,reg_temp = 0;
	unsigned long coul_flag = 0;

    if (irq >= g_coul_st.coul_int.int_num) {
        coul_err("coul irq is error,please check irq=%d\n", (int)irq);
        return;
    }

    reg_addr = g_coul_st.coul_int.int_mask_reg;
    reg_bit = irq & 0x7;

    spin_lock_irqsave(&g_coul_st.lock, coul_flag);
    pmic_reg_read(reg_addr,&reg_temp);
    reg_temp |= ((u32)1 << reg_bit);
    pmic_reg_write(reg_addr,reg_temp);
    spin_unlock_irqrestore(&g_coul_st.lock, coul_flag);
}
void bsp_coul_int_unmask(unsigned int irq)
{
    u32 reg_addr,reg_bit,reg_temp = 0;
	unsigned long coul_flag = 0;

    if (irq >= g_coul_st.coul_int.int_num) {
        coul_err("coul irq is error,please check irq=%d\n", (int)irq);
        return;
    }

    reg_addr = g_coul_st.coul_int.int_mask_reg;
    reg_bit = irq & 0x7;

    spin_lock_irqsave(&g_coul_st.lock, coul_flag);
    pmic_reg_read(reg_addr,&reg_temp);
    reg_temp &= ~((u32)1 << reg_bit);
    pmic_reg_write(reg_addr,reg_temp);
    spin_unlock_irqrestore(&g_coul_st.lock, coul_flag);
}
int bsp_coul_int_is_masked(unsigned int irq)
{
    u32 reg_addr,reg_bit,reg_temp = 0;
	unsigned long coul_flag = 0;

    if (irq >= g_coul_st.coul_int.int_num) {
        coul_err("coul irq is error,please check irq=%d\n", (int)irq);
        return;
    }

    reg_addr = g_coul_st.coul_int.int_mask_reg;
    reg_bit = irq & 0x7;

    spin_lock_irqsave(&g_coul_st.lock, coul_flag);
    pmic_reg_read(reg_addr,&reg_temp);
    spin_unlock_irqrestore(&g_coul_st.lock, coul_flag);

    return  !!(reg_temp & ((u32)1 << reg_bit));
}

int coul_int_register(COUL_INT_TYPE irq,COUL_INT_FUNC func,void *data)
{
    /*注册给pmu中断处理*/
    if(bsp_pmu_irq_callback_register(irq,func,data))
        return BSP_COUL_ERR;
    else
        return BSP_COUL_OK;
}

void coul_int_handler(void)
{
	u32 pending = 0;
	u8 offset = 0;
	
	pmic_reg_read(g_coul_st.coul_int.int_reg,&pending);
	/* clear int */
	pmic_reg_write(g_coul_st.coul_int.int_reg, pending);
	if (pending != 0)
    {
        coul_info("pending=0x%x\n", pending);
		for_each_set_bit(offset, &pending, g_coul_st.coul_int.int_num)
        {
			g_coul_st.irq_handler[offset].cnt++;
        	if(g_coul_st.irq_handler[offset].routine)
            	g_coul_st.irq_handler[offset].routine(g_coul_st.irq_handler[offset].data);
        }
    }
}

/*****************************************************************************
 函 数 名  : coul_ocv_get
 功能描述  : 电池开路电压采样值获取
 输入参数  : void
 输出参数  : 无
 返 回 值  : unsigned int
 调用函数  :
 被调函数  :
*****************************************************************************/
unsigned int bsp_coul_ocv_get(void)
{
    unsigned short val = 0;
    unsigned short data_val = 0;
    unsigned short offset_val = 0;
    unsigned int voltage = 0;

    data_val = coul_ocv_data_regval();
    offset_val = coul_ocv_offset_regval();
    val = data_val - offset_val;
    voltage = coul_convert_regval2mv_calied(val);

#if (FEATURE_ON == MBB_CHG_COULOMETER)
    coul_dbg("calied:coul_ocv_voltage is %d mv!\n",voltage);
#else
    coul_err("calied:coul_ocv_voltage is %d mv!\n",voltage);
#endif

    return voltage;
}

void bsp_coul_cali_value_show(void)
{
    coul_err("current_a,b:%d,%d;voltage_a,b:%d,%d.\n", \ 
        coul_cali_config.c_offset_a, coul_cali_config.c_offset_b, coul_cali_config.v_offset_a, coul_cali_config.v_offset_b);
}
int bsp_coul_init(void)
{
    u32 iret = BSP_COUL_OK;

    /*默认在fastboot中已配置*/
#if 0
    unsigned int low_batt_thres = 3400;

    /* 库仑计默认已开启,其他功能后续提供*/

    coul_power_ctrl(COUL_POWER_ON);

    coul_cali_ctrl(COUL_CALI_ON);

    coul_reflash_ctrl(COUL_REFLASH_ECO);

    coul_eco_ctrl(COUL_REFLASH_ECO);

    coul_set_vbat_value(low_batt_thres);
#endif
    bsp_coul_dts_init();
    //g_pmu_base_addr = ioremap(PMU_BASE_ADDR, 0x1000);
    spin_lock_init(&g_coul_st.lock);/*中断只在Acore实现，多core互斥锁*/

	g_coul_st.irq_handler = kzalloc(g_coul_st.coul_int.int_num*sizeof(struct coul_irq_handle), GFP_KERNEL);
    if(g_coul_st.irq_handler == NULL)
    {
        coul_err("coul irq handler kzalloc is failed,please check!\n");
        return -ENOMEM;
    }
	/*注册给pmu中断处理*/
	coul_int_register(g_coul_st.coul_int.pmu_int_num, coul_int_handler, NULL);
    /*读取nv值，获取电压电流计算参数*/
    /*read nv,get the exc protect setting*/
    iret = bsp_nvm_read(NV_ID_DRV_COUL_CALI,(unsigned char *)&coul_cali_config,sizeof(COUL_CALI_NV_TYPE));

    if(NV_OK != iret)
    {
        coul_err("coul cali read nv error,not set,use the default config!\n");
    }
    else
#if (FEATURE_ON == MBB_CHG_COULOMETER)
    {
        coul_info("coul init ok!\n");
    }
#else
        coul_err("coul init ok!\n");
#endif

    return BSP_COUL_OK;
}

/*后续提供*/
#if 0
/*****************************************************************************
 函 数 名  : bsp_coul_vbat_set
 功能描述  : 设置电池低电值,中断的阈值
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :
*****************************************************************************/
void bsp_coul_vbat_set(unsigned short value)
{
    unsigned char val1 = (value & 0xff);
    unsigned char val2 = (value >> 8) & 0xff;

    coul_reg_write(HI6551_V_INT0_OFFSET, val1);
    coul_reg_write(HI6551_V_INT1_OFFSET, val2);
}
#endif
static void __exit bsp_coul_exit(void)
{
}
module_exit(bsp_coul_exit);
module_init(bsp_coul_init);
MODULE_LICENSE("GPL");

EXPORT_SYMBOL_GPL(bsp_coul_voltage);
EXPORT_SYMBOL_GPL(bsp_coul_current);
EXPORT_SYMBOL_GPL(bsp_coul_in_capacity);
EXPORT_SYMBOL_GPL(bsp_coul_out_capacity);
EXPORT_SYMBOL_GPL(bsp_coul_charge_time);
EXPORT_SYMBOL_GPL(bsp_coul_discharge_time);
EXPORT_SYMBOL_GPL(bsp_coul_int_register);
EXPORT_SYMBOL_GPL(bsp_coul_ocv_get);

