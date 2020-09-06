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

/*lint --e{537,958}*/
#include <of.h>
#include <product_config.h>
#include <osl_bio.h>
#include <osl_thread.h>
#include <osl_sem.h>
#include <osl_spinlock.h>
#include <osl_malloc.h>
#include <soc_memmap.h>
#include <bsp_shared_ddr.h>
#include <bsp_pmu.h>
#include <drv_nv_id.h>
#include <drv_nv_def.h>
#include <bsp_version.h>
#include <bsp_nvim.h>
#include <bsp_ipc.h>
#include <bsp_regulator.h>
#include <bsp_pm_om.h>

#include <pmu_balong.h>
#include <pmic_inner.h>
#include "pmic_dr.h"

#define DR_INVAILD_ADDR 0xffff/*地址等无效值，有效地址最大为0x1AE*/
#define DR_INVAILD_DATA 0xffff/*数据等无效值*/
#define DR_INVAILD_OFFSET 0xffff/*偏移等无效值，最大偏移为8*/

struct pmic_dr_info g_pmic_drinfo;

/*****************************************************************************
* 函 数 名  : pmic_dr_para_check
*
* 功能描述  : 检查电压源是否合法
*
* 输入参数  : @dr_id：待检查的电压源
*
* 输出参数  : 无
*
* 返 回 值  : BSP_PMU_OK:合法;   BSP_PMU_ERROR:不合法
*****************************************************************************/
static __inline__ int pmic_dr_para_check(int dr_id)
{
    /* 有效性检查 */
    if ((dr_id < 0) || (dr_id >= g_pmic_drinfo.dr_num))
    {
        pmu_print_error("doesn't support dr %d!\n",dr_id);
        pmu_print_error("the dr you can use is from 0 to %d,please check!\n", g_pmic_drinfo.dr_num - 1);
        return BSP_PMU_ERROR;
    }

    return BSP_PMU_OK;
}

/*****************************************************************************
 函 数 名  : pmic_dr_set_mode
 功能描述  : pmic DR模式设置
            (呼吸闪烁)
 输入参数  : dr_id:电流源编号；mode:模式
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 备注说明  :
*****************************************************************************/
int pmic_dr_set_mode(int dr_id, dr_mode_e mode)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
	unsigned long dr_flag = 0;

    /* 参数有效性检查 */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    /*lint --e{746,718}*/
    spin_lock_irqsave(&g_pmic_drinfo.dr_lock, dr_flag);

    switch(mode){
    case PMU_DRS_MODE_FLA_FLASH:

        pmu_print_info("dr_id[%d] will be set fla-flash mode\n", dr_id);
        /*先设置为闪烁模式*/
        pmic_reg_read(dr_ctrl->dr_reg.mode_sel_reg, &regval);
        regval |= ((u32)0x1 << dr_ctrl->dr_reg.mode_sel_bit);
        pmic_reg_write(dr_ctrl->dr_reg.mode_sel_reg, regval);

        /*开启闪亮模式*/
        pmic_reg_read(dr_ctrl->dr_fla.mode_reg, &regval);
        regval |= ((u32)0x1 << dr_ctrl->dr_fla.mode_bit);
        pmic_reg_write(dr_ctrl->dr_fla.mode_reg, regval);
        break;
    case PMU_DRS_MODE_FLA_LIGHT:
        pmic_print_info("dr_id[%d] will be set fla-light mode\n", dr_id);

        /*先设置为闪烁模式*/
        pmic_reg_read(dr_ctrl->dr_reg.mode_sel_reg, &regval);
        regval |= ((u32)0x1 << dr_ctrl->dr_reg.mode_sel_bit);
        pmic_reg_write(dr_ctrl->dr_reg.mode_sel_reg, regval);

        /*关闭闪亮模式*/
        pmic_reg_read(dr_ctrl->dr_fla.mode_reg, &regval);
        regval &= ~((u32)0x1 << dr_ctrl->dr_fla.mode_bit);
        pmic_reg_write(dr_ctrl->dr_fla.mode_reg, regval);
        break;
    case PMU_DRS_MODE_BRE:
        pmic_print_info("dr_id[%d] will be set normal bre mode\n", dr_id);

        pmic_reg_read(dr_ctrl->dr_reg.mode_sel_reg, &regval);
        regval &= ~((u32)0x1 << dr_ctrl->dr_reg.mode_sel_bit);
        pmic_reg_write(dr_ctrl->dr_reg.mode_sel_reg, regval);
        break;
    case PMU_DRS_MODE_BRE_FLASH:
        pmic_print_info("dr_id[%d] will be set bre-flash mode\n", dr_id);
        /*先设置为呼吸模式*/
        pmic_reg_read(dr_ctrl->dr_reg.mode_sel_reg, &regval);
        regval &= ~((u32)0x1 << dr_ctrl->dr_reg.mode_sel_bit);
        pmic_reg_write(dr_ctrl->dr_reg.mode_sel_reg, regval);
        /*再设置为呼吸闪烁模式*/
        if(DR_INVAILD_ADDR == dr_ctrl->dr_bre.mode_reg)/*dr3/4/5没有该模式*/
        {
            pmu_print_error("dr_id[%d] cann't support bre-flash mode,will be set normal bre mode \n", dr_id);
            iret = BSP_PMU_ERROR;
        }
        else
        {
            pmic_reg_read(dr_ctrl->dr_bre.mode_reg, &regval);
            regval |= ((u32)0x1 << dr_ctrl->dr_bre.mode_bit);
            pmic_reg_write(dr_ctrl->dr_bre.mode_reg, regval);
        }
        break;

    case PMU_DRS_MODE_BRE_LIGHT:
        pmic_print_info("dr_id[%d] will be set bre-light mode\n", dr_id);
        /*先设置为呼吸模式*/
        pmic_reg_read(dr_ctrl->dr_reg.mode_sel_reg, &regval);
        regval &= ~((u32)0x1 << dr_ctrl->dr_reg.mode_sel_bit);
        pmic_reg_write(dr_ctrl->dr_reg.mode_sel_reg, regval);
        /*再设置为呼吸长亮模式*/
        if(DR_INVAILD_ADDR == dr_ctrl->dr_bre.mode_reg)/*dr3/4/5没有该模式*/
        {
            pmic_print_error("dr_id[%d] cann't support breath-light mode,will be set normal bre mode \n", dr_id);
            iret = BSP_PMU_ERROR;
        }
        else
        {
            pmic_reg_read(dr_ctrl->dr_bre.mode_reg, &regval);
            regval &= ~((u32)0x1 << dr_ctrl->dr_bre.mode_bit);
            pmic_reg_write(dr_ctrl->dr_bre.mode_reg, regval);
        }
        break;

    default:
        pmic_print_error("cann't support the mode\n");
        iret = BSP_PMU_ERROR;
        break;
    }

    spin_unlock_irqrestore(&g_pmic_drinfo.dr_lock, dr_flag);

    return iret;
}

/*****************************************************************************
 函 数 名  : bsp_hi6551_dr_get_mode
 功能描述  : PMIC HI6551电源模式查询
            (支持闪烁和呼吸)
            (DR1/2支持普通闪烁，呼吸长亮，呼吸闪烁( 高精度闪烁 )
             DR3/4/5只支持闪烁和呼吸)
 输入参数  : dr_id:要查询的电流源编号
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 备注说明  : Acore提供该函数实现
*****************************************************************************/
dr_mode_e pmic_dr_get_mode(int dr_id)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
	unsigned long dr_flag = 0;

    /* 参数有效性检查 */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
        return PMU_DRS_MODE_BUTTOM;

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    /*lint --e{746,718}*/
    spin_lock_irqsave(&g_pmic_drinfo.dr_lock, dr_flag);
    /*先判断是闪烁模式还是呼吸模式*/
    pmic_reg_read(dr_ctrl->dr_reg.mode_sel_reg, &regval);
    spin_unlock_irqrestore(&g_pmic_drinfo.dr_lock, dr_flag);
    if(regval & ((u32)0x1 << dr_ctrl->dr_reg.mode_sel_bit))
    {
        /*判断是闪烁长亮还是闪烁闪亮*/
        spin_lock_irqsave(&g_pmic_drinfo.dr_lock, dr_flag);
        pmic_reg_read(dr_ctrl->dr_fla.mode_reg, &regval);
        spin_unlock_irqrestore(&g_pmic_drinfo.dr_lock, dr_flag);
        if(regval & ((u32)0x1 << dr_ctrl->dr_fla.mode_bit))
        {
            pmic_print_info("dr_id[%d] is in fla-flash mode\n", dr_id);
            return PMU_DRS_MODE_FLA_FLASH;/*闪烁闪动低精度模式*/
        }
        else
        {
            pmic_print_info("dr_id[%d] is in fla-light mode\n", dr_id);
            return PMU_DRS_MODE_FLA_LIGHT;/*闪烁长亮持续输出模式*/
        }
    }
    else
    {
        /*if(PMIC_HI6551_DR03 <= dr_id),DR3,4,5不区分呼吸长亮和呼吸闪烁*/
        if(DR_INVAILD_ADDR == dr_ctrl->dr_bre.mode_reg)
        {
           pmic_print_info("dr_id[%d] is in normal bre mode\n", dr_id);
           return PMU_DRS_MODE_BRE;/*呼吸模式*/
        }
        else
        {
            /*判断是呼吸长亮还是呼吸闪烁*/
            spin_lock_irqsave(&g_pmic_drinfo.dr_lock, dr_flag);
            pmic_reg_read(dr_ctrl->dr_bre.mode_reg, &regval);
            spin_unlock_irqrestore(&g_pmic_drinfo.dr_lock, dr_flag);
            if(regval & ((u32)0x1 << dr_ctrl->dr_bre.mode_bit))
            {
                pmic_print_info("dr_id[%d] is in breath-flash mode\n", dr_id);
                return PMU_DRS_MODE_BRE_FLASH;/*呼吸闪烁高精度模式*/
            }
            else
            {
                pmic_print_info("dr_id[%d] is in breath-light mode\n", dr_id);
                return PMU_DRS_MODE_BRE_LIGHT;/*呼吸长亮模式*/
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : bsp_pmic_dr_enable
 功能描述  : 使能电源dr_id
 输入参数  : @dr_id 电源id
 输出参数  : 无
 返 回 值  : BSP_PMU_OK: 成功； else:失败
*****************************************************************************/
int pmic_dr_enable(int dr_id)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
	unsigned long dr_flag = 0;

    /* 参数有效性检查 */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    if(PMU_DRS_MODE_FLA_FLASH == pmic_dr_get_mode(dr_id) || PMU_DRS_MODE_FLA_LIGHT == pmic_dr_get_mode(dr_id))/*FLA MODE*/
    {
        spin_lock_irqsave(&g_pmic_drinfo.dr_lock, dr_flag);
        pmic_reg_read(dr_ctrl->dr_fla.onoff_reg, &regval);
        regval |= ((u32)0x1 << dr_ctrl->dr_fla.onoff_bit);
        pmic_reg_write(dr_ctrl->dr_fla.onoff_reg, regval);
        spin_unlock_irqrestore(&g_pmic_drinfo.dr_lock, dr_flag);
    }
    else/*BRE MODE*/
    {
        spin_lock_irqsave(&g_pmic_drinfo.dr_lock, dr_flag);
        pmic_reg_read(dr_ctrl->dr_bre.onoff_reg, &regval);
        regval |= ((u32)0x1 << dr_ctrl->dr_bre.onoff_bit);
        pmic_reg_write(dr_ctrl->dr_bre.onoff_reg, regval);
        spin_unlock_irqrestore(&g_pmic_drinfo.dr_lock, dr_flag);
    }

    return iret;
}

/*****************************************************************************
 函 数 名  : bsp_pmic_dr_disable
 功能描述  : 禁止电源dr_id
 输入参数  : @dr_id 电源id
 输出参数  : 无
 返 回 值  : BSP_PMU_OK: 成功； else:失败
*****************************************************************************/
int pmic_dr_disable(int dr_id)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
	unsigned long dr_flag = 0;

    /* 参数有效性检查 */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    if(PMU_DRS_MODE_FLA_FLASH == pmic_dr_get_mode(dr_id) || PMU_DRS_MODE_FLA_LIGHT == pmic_dr_get_mode(dr_id))/*FLA MODE*/
    {
        spin_lock_irqsave(&g_pmic_drinfo.dr_lock, dr_flag);
        pmic_reg_read(dr_ctrl->dr_fla.onoff_reg, &regval);
        regval &= ~((u32)0x1 << dr_ctrl->dr_fla.onoff_bit);
        pmic_reg_write(dr_ctrl->dr_fla.onoff_reg, regval);
        spin_unlock_irqrestore(&g_pmic_drinfo.dr_lock, dr_flag);
    }
    else/*BRE MODE*/
    {
        spin_lock_irqsave(&g_pmic_drinfo.dr_lock, dr_flag);
        pmic_reg_read(dr_ctrl->dr_bre.onoff_reg, &regval);
        regval &= ~((u32)0x1 << dr_ctrl->dr_bre.onoff_bit);
        pmic_reg_write(dr_ctrl->dr_bre.onoff_reg, regval);
        spin_unlock_irqrestore(&g_pmic_drinfo.dr_lock, dr_flag);
    }

    return iret;
}

/*****************************************************************************
 函 数 名  : bsp_pmic_dr_is_enabled
 功能描述  : 查询某路电压源是否开启。
 输入参数  : dr_id:电压源id号
 输出参数  : 无
 返 回 值  : 0:未开启；else:开启
*****************************************************************************/
int pmic_dr_is_enabled(int dr_id)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
	unsigned long dr_flag = 0;

    /* 参数有效性检查 */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    if(PMU_DRS_MODE_FLA_FLASH == pmic_dr_get_mode(dr_id) || PMU_DRS_MODE_FLA_LIGHT == pmic_dr_get_mode(dr_id))/*FLA MODE*/
    {
        spin_lock_irqsave(&g_pmic_drinfo.dr_lock, dr_flag);
        pmic_reg_read(dr_ctrl->dr_fla.onoff_reg, &regval);
        spin_unlock_irqrestore(&g_pmic_drinfo.dr_lock, dr_flag);

        return (regval & ((u32)0x1 << dr_ctrl->dr_fla.onoff_bit));
    }
    else/*BRE MODE*/
    {
        spin_lock_irqsave(&g_pmic_drinfo.dr_lock, dr_flag);
        pmic_reg_read(dr_ctrl->dr_bre.onoff_reg, &regval);
        spin_unlock_irqrestore(&g_pmic_drinfo.dr_lock, dr_flag);

        return (regval & ((u32)0x1 << dr_ctrl->dr_bre.onoff_bit));
    }

    return iret;
}

/*****************************************************************************
 函 数 名  : pmic_dr_get_current
 功能描述  : 获取电压源dr_id的电压值
 输入参数  : dr_id:电压源id号
 输出参数  : 无
 返 回 值  : 电压值
*****************************************************************************/
int pmic_dr_get_current(int dr_id)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
    u32 vsel = 0;
    int current_uA = 0;
	unsigned long dr_flag = 0;

    /* 参数有效性检查 */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = &g_pmic_drinfo.dr_ctrl[dr_id];

    spin_lock_irqsave(&g_pmic_drinfo.dr_lock, dr_flag);
    pmic_reg_read(dr_ctrl->dr_reg.current_reg, &regval);
    spin_unlock_irqrestore(&g_pmic_drinfo.dr_lock, dr_flag);
    /* 获取电流档位值 */
    vsel = (regval & dr_ctrl->dr_reg.current_mask);
    current_uA = dr_ctrl->current_table[vsel];

    pmu_print_info("dr_id %d's current is  %d uV!\n", dr_id, current_uA);

    return current_uA;
}

/*****************************************************************************
 函 数 名  : bsp_pmic_dr_set_current
 功能描述  : 设置电压源dr_id的电压值，配置为[min_uV, max_uV]区间的值即可，如果区间内没有合法值，失败返回
 输入参数  : dr_id: 要设置的电源编号
             min_uV: 最小合法电压值
             max_uV: 最大合法电压值
 输出参数  : @selector: 实际设置电压值的档位
 返 回 值  : BSP_PMU_OK: 成功； else:失败
*****************************************************************************/
int pmic_dr_set_current(int dr_id, int min_uA, int max_uA,unsigned *selector)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 vsel = 0;
    s32 current_uA = 0;
	unsigned long dr_flag = 0;

    /* 参数有效性检查 */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = &g_pmic_drinfo.dr_ctrl[dr_id];

    for(vsel = 0; vsel < dr_ctrl->current_nums; vsel++)
    {
        if (((u32)min_uA <= dr_ctrl->current_table[vsel]) && ((u32)max_uA >= dr_ctrl->current_table[vsel]))
        {
            break;
        }
    }
    if((vsel == dr_ctrl->current_nums))
    {
        pmu_print_error("dr_id %d cann't support current between %d and %d uA!\n",dr_id,min_uA,max_uA);
        iret = BSP_PMU_ERROR;
        goto out;
    }
    *selector = vsel;
    current_uA = dr_ctrl->current_table[vsel];

    spin_lock_irqsave(&g_pmic_drinfo.dr_lock, dr_flag);
    pmic_reg_write_mask(dr_ctrl->dr_reg.current_reg, vsel, dr_ctrl->dr_reg.current_mask);
    spin_unlock_irqrestore(&g_pmic_drinfo.dr_lock, dr_flag);

    pmu_print_info("current is set at %d uA!\n", current_uA);

out:
    return iret;
}

/*****************************************************************************
 函 数 名  : bsp_pmic_dr_list_current
 功能描述  : 获取电压源dr_id档位为selector的电压值
 输入参数  : dr_id: 要设置的电源编号
             selector: 电压档位
 返 回 值  : BSP_PMU_OK: 成功； else:失败
*****************************************************************************/
int pmic_dr_list_current(int dr_id, unsigned selector)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    int current_uA = 0;

    /* 参数有效性检查 */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = &g_pmic_drinfo.dr_ctrl[dr_id];

    /* 检查档位有效性 */
    if (selector >= dr_ctrl->current_nums)
    {
        pmu_print_error("selector is %d,not exist,-EINVAL,please input new\n", selector);
        return BSP_PMU_ERROR;
    }

    current_uA = dr_ctrl->current_table[selector];

    pmu_print_info("dr %d selector %d is %d uV!!\n",dr_id,selector,current_uA);

    return current_uA;
}

struct regulator_id_ops pmic_dr_ops = {
        /* for lcd */
        .list_voltage = pmic_dr_list_current,
        .set_voltage = pmic_dr_set_current,
        .get_voltage = pmic_dr_get_current,
        /* for led */
        .set_current = pmic_dr_set_current,
        .get_current_limit = pmic_dr_get_current,
        .is_enabled = pmic_dr_is_enabled,
        .enable = pmic_dr_enable,
        .disable = pmic_dr_disable,
};
void pmic_dr_register_ops(void)
{
    int ret = 0;
    ret = regulator_pmic_ops_register(&pmic_dr_ops, "pmic_dr");
    if (ret)
    {
        pmu_print_error("register pmic_dr ops failed,err id: %d!\n",ret);
    }
}
/*****************************************************************************
 函 数 名  : hi6551_dr_fla_time_get
 功能描述  : 获取dr的闪烁周期时间和点亮时间
 输入参数  : dr_fla_time_st:闪烁时间参数结构体;单位:us
 输出参数  : 无
 返 回 值  : 设置成功或者失败
 调用函数  :
 备注说明  : 对应寄存器只有设置时间的功能，不需要互斥锁
             闪烁周期:reg_value*31.25ms;点亮时间:reg_value*7.8125ms
*****************************************************************************/
int pmic_dr_fla_time_get(int dr_id, DR_FLA_TIME* dr_fla_time_st)
{
    u32 period = 0;/*闪烁周期寄存器配置值*/
    u32 on_time = 0;/*点亮时间设置值*/
    unsigned period_time_us = 0;
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;

    /* 参数有效性检查 */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    if(NULL == dr_fla_time_st)
    {
        pmic_print_error("dr_fla_time_st is NULL!\n");
        return BSP_PMU_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    /*feriod时间获取*/
    pmic_reg_read(dr_ctrl->dr_fla.period_reg,&period);
    period_time_us = period_time_us * dr_ctrl->dr_fla.period_step;
    pmic_print_info("dr fla feriod is %d us!!\n",period_time_us);

    /*闪烁点亮时间获取*/
    pmic_reg_read(dr_ctrl->dr_fla.light_reg,&on_time);
    dr_fla_time_st->fla_on_us = on_time * dr_ctrl->dr_fla.light_step;
    pmic_print_info("dr fla on_time is %d us!!\n",dr_fla_time_st->fla_on_us);

    /*闪烁灯灭时间获取*/
    dr_fla_time_st->fla_off_us = period_time_us - dr_fla_time_st->fla_on_us;
    pmic_print_info("dr fla off_time is %d us!!\n",dr_fla_time_st->fla_off_us);

    return BSP_DR_OK;
}
/*****************************************************************************
 函 数 名  : hi6551_dr_fla_time_set
 功能描述  : 设置dr的闪烁周期时间和点亮时间
 输入参数  : dr_fla_time_st:闪烁时间参数结构体;单位:us
 输出参数  : 无
 返 回 值  : 设置成功或者失败
 调用函数  :
 备注说明  : 对应寄存器只有设置时间的功能，不需要互斥锁
             闪烁周期:reg_value*31.25ms;点亮时间:reg_value*7.8125ms
*****************************************************************************/
int pmic_dr_fla_time_set(int dr_id, DR_FLA_TIME *dr_fla_time_st)
{
    u32 feriod = 0;/*闪烁周期寄存器配置值*/
    u32 on_time = 0;/*点亮时间设置值*/
    DR_FLA_TIME dr_fla_time_current={0,0};
    unsigned int fla_on_time,fla_off_time,feriod_time;
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;

    if(!dr_fla_time_st)
    {
        return BSP_DR_ERROR;
    }

    /* 参数有效性检查 */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    /*获取当前闪烁参数配置值*/
    pmic_dr_fla_time_get(dr_id, &dr_fla_time_current);

    if(DR_VALUE_INVALIED != dr_fla_time_st->fla_on_us)
        fla_on_time = dr_fla_time_st->fla_on_us;
    else
        fla_on_time = dr_fla_time_current.fla_on_us;

    if(DR_VALUE_INVALIED != dr_fla_time_st->fla_off_us)
        fla_off_time = dr_fla_time_st->fla_off_us;
    else
        fla_off_time = dr_fla_time_current.fla_off_us;

    feriod_time = fla_on_time + fla_off_time;

    /*feriod时间配置*/
    feriod = (feriod_time / dr_ctrl->dr_fla.period_step);
    pmic_reg_write(dr_ctrl->dr_fla.period_reg,feriod);

    /*闪烁点亮时间配置*/
    on_time = (fla_on_time / dr_ctrl->dr_fla.light_step);
    pmic_reg_write(dr_ctrl->dr_fla.light_reg,on_time);

    return BSP_DR_OK;
}
/*****************************************************************************
 函 数 名  : pmic_dr_bre_time_set
 功能描述  : 设置dr的呼吸时间(包括长亮，长暗、渐亮、渐暗时间)
 输入参数  : dr_id:要设置的电流源编号;dr_bre_time_st:呼吸时间参数结构体;
            没有找到设置的时间，就设置为长亮
 输出参数  : 无
 返 回 值  : 设置成功或者失败
 调用函数  :
 备注说明  : 对应寄存器只有设置时间的功能，不需要互斥锁
*****************************************************************************/
int pmic_dr_bre_time_set(int dr_id, DR_BRE_TIME *dr_bre_time_st)
{
    unsigned int bre_on_time,bre_off_time,bre_rise_time,bre_fall_time = 0;
    u32 bre_on,bre_off,bre_rise,bre_fall = 0;/*设置的寄存器对应值*/
    u8 i = 0;
    bool valid = 0;
    int iret = BSP_PMU_OK;
    struct pmic_dr_ctrl* dr_ctrl = NULL;

    if(!dr_bre_time_st)
    {
        return BSP_DR_ERROR;
    }

    /* 参数有效性检查 */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    bre_on_time = dr_bre_time_st->bre_on_ms;
    bre_off_time = dr_bre_time_st->bre_off_ms;
    bre_rise_time = dr_bre_time_st->bre_rise_ms;
    bre_fall_time = dr_bre_time_st->bre_fall_ms;

    /*计算出设置的值*/
    if(bre_on_time != DR_VALUE_INVALIED)
    {
        for(i = 0; i < dr_ctrl->dr_bre.onofftime_num; i++ )
        {
            if(bre_on_time == dr_ctrl->onofftime_table[i])
            {
                valid = 1;
                break;
            }
        }
        if(valid)
        {
            valid = 0;

            if(i == dr_ctrl->dr_bre.onofftime_num - 1)
                bre_on = ALWAYS_ON_OFF;/*长亮*/
            else
                bre_on = i;

            pmic_reg_write_mask(dr_ctrl->dr_bre.onofftime_reg, (bre_on << dr_ctrl->dr_bre.ontime_offset), dr_ctrl->dr_bre.ontime_mask << dr_ctrl->dr_bre.ontime_offset);
        }
        else
        {
            pmic_print_error("can not support bre_on_time you want! \n");
            iret = BSP_DR_ERROR;
        }
    }

    if(bre_off_time != DR_VALUE_INVALIED)
    {
        for(i = 0; i < dr_ctrl->dr_bre.onofftime_num; i++ )
        {
            if(bre_off_time == dr_ctrl->onofftime_table[i])
            {
                valid = 1;
                break;
            }
        }
        if(valid)
        {
            valid = 0;

            if(i == dr_ctrl->dr_bre.onofftime_num - 1)
                bre_off = ALWAYS_ON_OFF;/*长暗*/
            else
                bre_off = i;

            pmic_reg_write_mask(dr_ctrl->dr_bre.onofftime_reg, (bre_off << dr_ctrl->dr_bre.offtime_offset), dr_ctrl->dr_bre.offtime_mask << dr_ctrl->dr_bre.offtime_offset);
        }
        else
        {
            pmic_print_info("can not support bre_off_time you want! \n");
            iret = BSP_DR_ERROR;
        }
    }

    if(bre_rise_time != DR_VALUE_INVALIED)
    {
        for(i = 0; i < dr_ctrl->dr_bre.risefalltime_num; i++ )
        {
            if(bre_rise_time == dr_ctrl->risefalltime_table[i])
            {
                valid = 1;
                break;
            }
        }
        if(valid)
        {
            valid = 0;
            bre_rise = i;
            pmic_reg_write_mask(dr_ctrl->dr_bre.risefalltime_reg, (bre_rise << dr_ctrl->dr_bre.risetime_offset), dr_ctrl->dr_bre.risetime_mask << dr_ctrl->dr_bre.risetime_offset);
        }
        else
        {
            pmic_print_info("can not support bre_rise_time you want! \n");
            iret = BSP_DR_ERROR;
        }
    }

    if(bre_fall_time != DR_VALUE_INVALIED)
    {
        for(i = 0; i < dr_ctrl->dr_bre.risefalltime_num; i++ )
        {
            if(bre_fall_time == dr_ctrl->risefalltime_table[i])
            {
                valid = 1;
                break;
            }
        }
        if(valid)
        {
            bre_fall = i;
            pmic_reg_write_mask(dr_ctrl->dr_bre.risefalltime_reg, (bre_fall << dr_ctrl->dr_bre.falltime_offset) , dr_ctrl->dr_bre.falltime_mask << dr_ctrl->dr_bre.falltime_offset);
        }
        else
        {
            pmic_print_info("can not support bre_fall_time you want! \n");
            iret = BSP_DR_ERROR;
        }
    }

    return iret;
}
/*****************************************************************************
 函 数 名  : hi6551_dr_bre_time_list
 功能描述  : 陈列dr的呼吸时间(包括长亮，长暗、渐亮、渐暗时间)各档位的时间值
 输入参数  : dr_id:要设置的电流源编号;bre_time_enum:所要查询的时间类型;
             selector，时间档位
 输出参数  : 无
 返 回 值  : 档位时间
 调用函数  :
 备注说明  :
*****************************************************************************/
unsigned pmic_dr_bre_time_list(int dr_id, dr_bre_time_e bre_time_enum ,unsigned selector)
{
    unsigned dr_bre_time = 0;
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;

    /* 参数有效性检查 */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    switch(bre_time_enum)
    {
        case PMU_DRS_BRE_ON_MS:
        case PMU_DRS_BRE_OFF_MS:
            if(selector > dr_ctrl->dr_bre.onofftime_num)
            {
                pmic_print_error("dr_id[%d] bre_onoff_time don't support selector %d!\n",dr_id,selector);
                pmic_print_error("max selector is %d!\n",dr_ctrl->dr_bre.onofftime_num);
                return BSP_PMU_PARA_ERROR;
            }
            else
                dr_bre_time = dr_ctrl->onofftime_table[selector];
            break;
        case PMU_DRS_BRE_RISE_MS:
        case PMU_DRS_BRE_FALL_MS:
            if(selector > dr_ctrl->dr_bre.risefalltime_num)
            {
                pmic_print_error("dr_id[%d] bre_risefall_time don't support selector %d!\n",dr_id,selector);
                pmic_print_error("max selector is %d!\n",dr_ctrl->dr_bre.risefalltime_num);
                return BSP_PMU_PARA_ERROR;
            }
            else
                dr_bre_time = dr_ctrl->risefalltime_table[selector];
            break;
        default:
            pmu_print_info("don't support this bre time!\n");
            return BSP_PMU_PARA_ERROR;
    }
    pmu_print_info("dr_id[%d],enum %d 's selector %d time is %dms!\n",dr_id,bre_time_enum,selector,dr_bre_time);

    return dr_bre_time;
}

/*****************************************************************************
 函 数 名  : pmic_dr_bre_time_selector_get
 功能描述  : 获取dr 呼吸时间总共档位值
 输入参数  : dr_id:要设置的电流源编号;bre_time_enum:所要查询的时间类型;
 输出参数  : 无
 返 回 值  : 档位值
 调用函数  :
 备注说明  :
*****************************************************************************/
unsigned pmic_dr_bre_time_selectors_get(int dr_id, dr_bre_time_e bre_time_enum)
{
    int iret = BSP_PMU_OK;
    unsigned selectors = 0;
    struct pmic_dr_ctrl* dr_ctrl = NULL;

    /*参数有效性检查*/
    iret = pmic_dr_para_check((int)dr_id);
    if(BSP_PMU_OK != iret)
        return BSP_PMU_PARA_ERROR;

    dr_ctrl = &g_pmic_drinfo.dr_ctrl[dr_id];

    switch(bre_time_enum)
    {
        case PMU_DRS_BRE_ON_MS:
        case PMU_DRS_BRE_OFF_MS:
            selectors = dr_ctrl->dr_bre.onofftime_num;
            break;
        case PMU_DRS_BRE_RISE_MS:
        case PMU_DRS_BRE_FALL_MS:
            selectors = dr_ctrl->dr_bre.risefalltime_num;
            break;
        default:
            pmic_print_error("don't support this bre time!\n");
            return BSP_PMU_PARA_ERROR;
    }
    pmic_print_info("dr_id[%d],enum %d 's selector %d !\n",dr_id,bre_time_enum,selectors);
    return selectors;
}

/*****************************************************************************
 函 数 名  : pmic_dr_start_delay_set
 功能描述  : 设置dr的启动延时时间
 输入参数  : dr_id:要设置的电流源编号;delay_ms:启动延时的时间，单位:ms
             设置范围:[0 , 32768]ms
 输出参数  : 无
 返 回 值  : 设置成功或失败
 调用函数  :
 备注说明  : 对应寄存器只有设置时间的功能，不需要互斥锁，只有DR3/4/5有此功能
*****************************************************************************/
int pmic_dr_start_delay_set(int dr_id, unsigned delay_ms)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
	unsigned long dr_flag = 0;

    /* 参数有效性检查 */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);
    /* 有效性检查*/
    if (DR_INVAILD_DATA == dr_ctrl->dr_del.del_step)
    {
        pmic_print_error("This DR %d can not set start delay time!\n",dr_id);
        return BSP_PMU_PARA_ERROR;
    }
    if ((delay_ms < dr_ctrl->dr_del.del_minms)||(delay_ms > dr_ctrl->dr_del.del_maxms))
    {
        pmic_print_error("This dr start delay time can be set form %d to %d ms!\n",dr_ctrl->dr_del.del_minms,dr_ctrl->dr_del.del_maxms);
        return BSP_PMU_PARA_ERROR;
    }
    spin_lock_irqsave(&g_pmic_drinfo.dr_lock, dr_flag);
    pmic_reg_write(dr_ctrl->dr_del.del_reg,(delay_ms/dr_ctrl->dr_del.del_step));
    spin_unlock_irqrestore(&g_pmic_drinfo.dr_lock, dr_flag);
    return BSP_DR_OK;
}
void pmic_dr_dts_init(void)
{
    struct device_node *dev_node = NULL;
    struct device_node *child_node = NULL;
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    u32 dr_id = 0;
    u32 table_size = 0;
    u32 ctrl_size = 0;
    int ret = 0;

    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,pmic_dr_app");
    if(NULL == dev_node)
    {
        pmic_print_error("pmu_dr device node not found\n");
        return;
    }
    /*获取dr总数*/
    g_pmic_drinfo.dr_num= of_get_child_count(dev_node);
    ctrl_size = sizeof(struct pmic_dr_ctrl)*(g_pmic_drinfo.dr_num);
    g_pmic_drinfo.dr_ctrl= (struct pmic_dr_ctrl *)osl_malloc(ctrl_size);
    if(IS_ERR(g_pmic_drinfo.dr_ctrl))
    {
        pmic_print_error("malloc dr_ctrl failed\n");
        return;
    }
    (void)memset((void *)g_pmic_drinfo.dr_ctrl,0,ctrl_size);

    for_each_child_of_node(dev_node, child_node)
    {
        ret = of_property_read_u32_index(child_node, "dr_id", 0,&dr_id);
        if(ret)
        {
            pmic_print_error("pmu_dr get dr_id for dts failed,err_id %d!\n",ret);
            goto out_loop;
        }

        dr_ctrl = &g_pmic_drinfo.dr_ctrl[dr_id];
        dr_ctrl->id = dr_id;

        (void)of_property_read_string_index(child_node,"dr_name",0,&dr_ctrl->name);

        of_property_read_u32_index(child_node, "current_nums",0,(u32*)&(dr_ctrl->current_nums));
        table_size = sizeof(u32)*(dr_ctrl->current_nums);
        dr_ctrl->current_table = osl_malloc(table_size);
        if(IS_ERR(dr_ctrl->current_table))
        {
            pmic_print_error("volt_id %d malloc current table failed!\n",dr_id);
            goto out_loop;
        }
        (void)memset((void *)dr_ctrl->current_table,0,table_size);

        ret = of_property_read_u32_array(child_node, "current_table",
                        (u32 *)(dr_ctrl->current_table), dr_ctrl->current_nums);
        if (ret) {
            pmic_print_error("dr_id %d get current table info from dts failed,errid %d!\n",dr_id,ret);
            goto out_loop;
        };
        ret = of_property_read_u32_array(child_node, "reg_ctrl",
                        (u32 *)&(dr_ctrl->dr_reg), sizeof(struct dr_reg)/sizeof(u32));
        ret = of_property_read_u32_array(child_node, "fla_ctrl",
                        (u32 *)&(dr_ctrl->dr_fla), sizeof(struct dr_fla)/sizeof(u32));
        ret = of_property_read_u32_array(child_node, "bre_ctrl",
                        (u32 *)&(dr_ctrl->dr_bre), sizeof(struct dr_bre)/sizeof(u32));
        ret = of_property_read_u32(child_node, "bre_onofftime_num", &dr_ctrl->dr_bre.onofftime_num);
        table_size = sizeof(u32)*(dr_ctrl->dr_bre.onofftime_num);
        dr_ctrl->onofftime_table = osl_malloc(table_size);
        if(IS_ERR(dr_ctrl->onofftime_table))
        {
            pmic_print_error("volt_id %d malloc onofftime_table failed!\n",dr_id);
            goto out_loop;
        }
        ret = of_property_read_u32_array(child_node, "bre_onofftime_table",
                        (u32 *)(dr_ctrl->onofftime_table), dr_ctrl->dr_bre.onofftime_num);
        if (ret) {
            pmic_print_error("dr_id %d get bre_onofftime table info from dts failed,errid %d!\n",dr_id,ret);
            goto out_loop;
        };        
        ret = of_property_read_u32(child_node, "bre_risefalltime_num", &dr_ctrl->dr_bre.risefalltime_num);
        table_size = sizeof(u32)*(dr_ctrl->dr_bre.risefalltime_num);
        dr_ctrl->risefalltime_table = osl_malloc(table_size);
        if(IS_ERR(dr_ctrl->risefalltime_table))
        {
            pmic_print_error("volt_id %d malloc risefalltime_table_table failed!\n",dr_id);
            goto out_loop;
        }
        ret = of_property_read_u32_array(child_node, "bre_risefalltime_table",
                        (u32 *)(dr_ctrl->risefalltime_table), dr_ctrl->dr_bre.risefalltime_num);
        if (ret) {
            pmic_print_error("dr_id %d get bre_risefalltime table info from dts failed,errid %d!\n",dr_id,ret);
            goto out_loop;
        }
        ret = of_property_read_u32_array(child_node, "start_del_ctrl",
                        (u32 *)&(dr_ctrl->dr_del), sizeof(struct dr_del)/sizeof(u32));
    }
    spin_lock_init(&(g_pmic_drinfo.dr_lock));
    pmic_print_error("init ok!\n");
    return;
    
out_loop:
    for_each_child_of_node(dev_node, child_node)
    {
        if(dr_ctrl->current_table)
            osl_free(dr_ctrl->current_table);
        if(dr_ctrl->onofftime_table)
            osl_free(dr_ctrl->onofftime_table);
        if(dr_ctrl->risefalltime_table)
            osl_free(dr_ctrl->risefalltime_table);
    }
    osl_free(g_pmic_drinfo.dr_ctrl);
}
/*****************************************************************************
 函 数 名  : bsp_pmu_pmic_init
 功能描述  : PMIC pmic PMU模块初始化
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
void pmic_dr_init(void)
{
    /*read dts*/
    pmic_dr_dts_init();

    pmic_dr_register_ops();

    pmu_print_error("pmic dr init ok!\n");
}

struct pmu_dr_adp_ops g_dr_adpops = {
    .dr_list_current = pmic_dr_list_current,
    .dr_set_mode = pmic_dr_set_mode,
    .dr_get_mode = pmic_dr_get_mode,
    .dr_fla_time_set = pmic_dr_fla_time_set,
    .dr_fla_time_get = pmic_dr_fla_time_get,
    .dr_bre_time_set = pmic_dr_bre_time_set,
    .dr_bre_time_list = pmic_dr_bre_time_list,
    .dr_bre_time_selectors_get = pmic_dr_bre_time_selectors_get,
    .dr_start_delay_set = pmic_dr_start_delay_set,
};

/*****************************************************************************
* 函 数 名  : bsp_dr_list_current
* 功能描述  :查询某路电流源指定档位电流。
* 输入参数  :dr_id:电流源id号；selector：要查询的档位；
* 输出参数  :无
* 返回值：   所查询的电流源指定档位的电流值（单位：uA）
*
*****************************************************************************/
int bsp_dr_list_current(int dr_id, unsigned selector)
{
    return  g_dr_adpops.dr_list_current(dr_id,selector);
}
/*****************************************************************************
* 函 数 名  : bsp_pmu_dr_set_mode
* 功能描述  :设置某路电流源模式。
* 输入参数  :dr_id:电流源id号；mode：设置的模式；
* 输出参数  :无
* 返回值：   成功或失败
*
*****************************************************************************/
int bsp_dr_set_mode(int dr_id, dr_mode_e mode)
{
    return  g_dr_adpops.dr_set_mode(dr_id,mode);
}
/*****************************************************************************
* 函 数 名  : bsp_hi6551_dr_get_mode
* 功能描述  :获取某路电流源模式。
* 输入参数  :dr_id:电流源id号；
* 输出参数  :无
* 返回值：   电流源当前模式
*
*****************************************************************************/
dr_mode_e bsp_dr_get_mode(int dr_id)
{
    return  g_dr_adpops.dr_get_mode(dr_id);
}
/*****************************************************************************
 函 数 名  : bsp_dr_fla_time_set
 功能描述  : 设置dr的闪烁点亮时间 和 暗灭时间
 输入参数  : dr_fla_time_st:闪烁时间参数结构体;单位:us
 输出参数  : 无
 返 回 值  : 设置成功或者失败
 调用函数  :
*****************************************************************************/
/*呼吸时间配置dr1/2*/
int bsp_dr_fla_time_set(int dr_id, DR_FLA_TIME *dr_fla_time_st)
{
    return  g_dr_adpops.dr_fla_time_set(dr_id, dr_fla_time_st);
}
/*****************************************************************************
 函 数 名  : bsp_dr_fla_time_set
 功能描述  : 获取dr的闪烁点亮时间 和 暗灭时间
 输入参数  : dr_fla_time_st:闪烁时间参数结构体;单位:us
 输出参数  : 无
 返 回 值  : 设置成功或者失败
 调用函数  :
*****************************************************************************/
/*呼吸时间配置dr1/2*/
int bsp_dr_fla_time_get(int dr_id, DR_FLA_TIME *dr_fla_time_st)
{
    return  g_dr_adpops.dr_fla_time_get(dr_id, dr_fla_time_st);
}
/*****************************************************************************
 函 数 名  : bsp_dr_bre_time_set
 功能描述  : 设置dr的呼吸时间(包括长亮，长暗、渐亮、渐暗时间)
 输入参数  : dr_id:要设置的电流源编号;dr_bre_time_st:呼吸时间参数结构体;
            没有找到设置的时间，就设置为长亮
 输出参数  : 无
 返 回 值  : 设置成功或者失败
 调用函数  :
 备注说明  : 对应寄存器只有设置时间的功能，不需要互斥锁
*****************************************************************************/
int bsp_dr_bre_time_set(int dr_id, DR_BRE_TIME *dr_bre_time_st)
{
    return  g_dr_adpops.dr_bre_time_set(dr_id,dr_bre_time_st);
}

void bsp_dr_test(void)
{
	/*coverity[var_decl] */
    DR_BRE_TIME bre_time_in, bre_time_out;
	/*coverity[var_decl] */
    DR_FLA_TIME fla_time_in, fla_time_out;

    fla_time_in.fla_off_us = 5;
    fla_time_in.fla_on_us = 5;
    bsp_dr_fla_time_set(0, &fla_time_in);
    bsp_dr_fla_time_get(0, &fla_time_out);

	/*coverity[uninit_use_in_call] */
    pmu_print_error("fla_time:on:%d, off:%d\n", fla_time_out.fla_on_us, fla_time_out.fla_off_us);

    bre_time_in.bre_on_ms = 5;
    bre_time_in.bre_off_ms = 5;
    bre_time_in.bre_fall_ms = 5;
    bre_time_in.bre_rise_ms = 5;
    bsp_dr_bre_time_set(3, &bre_time_in);
	/*coverity[uninit_use_in_call] */
    pmu_print_error("bre_time:on:%d,off:%d,fall:%d,rise:%d\n", \
        bre_time_out.bre_on_ms, bre_time_out.bre_off_ms, bre_time_out.bre_fall_ms, bre_time_out.bre_rise_ms);

}

/*****************************************************************************
 函 数 名  : bsp_dr_bre_time_list
 功能描述  : 陈列dr的呼吸时间(包括长亮，长暗、渐亮、渐暗时间)各档位的时间值
 输入参数  : dr_id:要查询的电流源编号;bre_time_enum:所要查询的时间类型;
             selector，时间档位
 输出参数  : 无
 返 回 值  : 档位时间
 调用函数  :
 备注说明  :
*****************************************************************************/
unsigned bsp_dr_bre_time_list(int dr_id, dr_bre_time_e bre_time_enum ,unsigned selector)
{
    return  g_dr_adpops.dr_bre_time_list(dr_id,bre_time_enum,selector);
}

/*****************************************************************************
 函 数 名  : bsp_dr_bre_time_selectors_get
 功能描述  : 获取dr 呼吸时间总共档位值
 输入参数  : dr_id:要设置的电流源编号;bre_time_enum:所要查询的时间类型;
 输出参数  : 无
 返 回 值  : 档位时间
 调用函数  :
 备注说明  :
*****************************************************************************/
unsigned bsp_dr_bre_time_selectors_get(int dr_id, dr_bre_time_e bre_time_enum)
{
    return  g_dr_adpops.dr_bre_time_selectors_get(dr_id,bre_time_enum);
}
/*****************************************************************************
 函 数 名  : bsp_dr_start_delay_set
 功能描述  : 设置dr的启动延时时间
 输入参数  : dr_id:要设置的电流源编号;delay_ms:启动延时的时间，单位:ms
             设置范围:[0 , 32768]ms
 输出参数  : 无
 返 回 值  : 设置成功或失败
 调用函数  :
 备注说明  : 对应寄存器只有设置时间的功能，不需要互斥锁，只有DR3/4/5有此功能
*****************************************************************************/
int bsp_dr_start_delay_set(int dr_id, unsigned delay_ms)
{
    return  g_dr_adpops.dr_start_delay_set(dr_id,delay_ms);
}
