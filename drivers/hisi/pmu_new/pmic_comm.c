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

#include <linux/string.h>
#include <osl_malloc.h>
#include <osl_spinlock.h>
#include <of.h>
#include <soc_memmap.h>
#include <drv_comm.h>
#include <hi_pmu.h>
#include "pmic_inner.h"
#include "pmic_comm.h"
#include "pmu_balong.h"
#include "pmu_stub.h"
#include "pmic_dr.h"
#include "pmic_irq.h"
#include "pmic_exc.h"
#include "pmic_ioshare.h"

struct pmic_comm g_pmic_comminfo;

int pmic_32k_check_para(int clk_id)
{
    if(clk_id < 0 || clk_id >= g_pmic_comminfo.comm_32k.num_32k)
    {
        pmic_print_error("clk_id is invalied, please check. clk_id:%d\n",clk_id);
        return BSP_PMU_ERROR;
    }
    return BSP_PMU_OK;
}

/*****************************************************************************
 函 数 名  : pmic_32k_clk_enable
 功能描述  : 开启对应路32k时钟
 输入参数  : pmu 32k 时钟枚举值
 输出参数  : 无
 返 回 值  : 操作成功或失败
 调用函数  :
 被调函数  : pmu adp文件调用
*****************************************************************************/
int pmic_32k_clk_enable(int clk_id)
{
    struct comm_32k* comm_32k = &(g_pmic_comminfo.comm_32k);
    struct ctrl_32k* ctrl_32k = NULL;
	unsigned long flag_32k = 0;


    /*check para*/
    if(pmic_32k_check_para(clk_id))
    {
        return BSP_PMU_ERROR;
    }
    ctrl_32k = &(comm_32k->ctrl_32k[clk_id]);
    spin_lock_irqsave(&(comm_32k->lock_32k), flag_32k);
    pmic_reg_write_mask(ctrl_32k->addr,(0x1 << ctrl_32k->bit),(0x1 << ctrl_32k->bit));
    spin_unlock_irqrestore(&(comm_32k->lock_32k), flag_32k);

    return BSP_PMU_OK;

}
/*****************************************************************************
 函 数 名  : pmic_32k_clk_disable
 功能描述  : 关闭对应路32k时钟
 输入参数  : pmu 32k 时钟枚举值
 输出参数  : 无
 返 回 值  : 操作成功或失败
 调用函数  :
 被调函数  : pmu adp文件调用
*****************************************************************************/
int pmic_32k_clk_disable(int clk_id)
{
    struct comm_32k* comm_32k = &(g_pmic_comminfo.comm_32k);
    struct ctrl_32k* ctrl_32k = NULL;
	unsigned long flag_32k = 0;

    /*check para*/
    if(pmic_32k_check_para(clk_id))
    {
        return BSP_PMU_ERROR;
    }
    ctrl_32k = &(comm_32k->ctrl_32k[clk_id]);
    spin_lock_irqsave(&(comm_32k->lock_32k), flag_32k);
    pmic_reg_write_mask(ctrl_32k->addr,(0x0 << ctrl_32k->bit),(0x1 << ctrl_32k->bit));
    spin_unlock_irqrestore(&(comm_32k->lock_32k), flag_32k);

    return BSP_PMU_OK;
}
/*****************************************************************************
 函 数 名  : pmic_32k_clk_disable
 功能描述  : 关闭对应路32k时钟
 输入参数  : pmu 32k 时钟枚举值
 输出参数  : 无
 返 回 值  : 操作成功或失败
 调用函数  :
 被调函数  : pmu adp文件调用
*****************************************************************************/
int pmic_32k_clk_is_enabled(int clk_id)
{
    struct comm_32k* comm_32k = &(g_pmic_comminfo.comm_32k);
    struct ctrl_32k* ctrl_32k = NULL;
    u32 reg_value = 0;
	unsigned long flag_32k = 0;

    /*check para*/

    ctrl_32k = &(comm_32k->ctrl_32k[clk_id]);
    spin_lock_irqsave(&(comm_32k->lock_32k), flag_32k);
    pmic_reg_read(ctrl_32k->addr,&reg_value);
    spin_unlock_irqrestore(&(comm_32k->lock_32k), flag_32k);

    return !!(reg_value & (0x1 << ctrl_32k->bit));
}

int pmic_xo_check_para(int clk_id)
{
    if(clk_id < 0 || clk_id >= g_pmic_comminfo.comm_xo.num_xo)
    {
        pmic_print_error("clk_id is invalied, please check. clk_id:%d\n",clk_id);
        return BSP_PMU_ERROR;
    }
    return BSP_PMU_OK;
}

/*****************************************************************************
 函 数 名  : pmic_xo_clk_enable
 功能描述  : 开启对应路32k时钟
 输入参数  : pmu xo 时钟枚举值
 输出参数  : 无
 返 回 值  : 操作成功或失败
 调用函数  :
 被调函数  : pmu adp文件调用
*****************************************************************************/
int pmic_xo_clk_enable(int clk_id)
{
    struct comm_xo* comm_xo = &(g_pmic_comminfo.comm_xo);
    struct ctrl_xo* ctrl_xo = NULL;
	unsigned long xo_flag = 0;


    /*check para*/
    if(pmic_xo_check_para(clk_id))
    {
        return BSP_PMU_ERROR;
    }
    ctrl_xo = &(comm_xo->ctrl_xo[clk_id]);
    spin_lock_irqsave(&(comm_xo->lock_xo), xo_flag);
    (void)bsp_ipc_spin_lock(IPC_SEM_PMU);
    pmic_reg_write_mask(ctrl_xo->addr,(0x1 << ctrl_xo->bit),(0x1 << ctrl_xo->bit));
    (void)bsp_ipc_spin_unlock(IPC_SEM_PMU);
    spin_unlock_irqrestore(&(comm_xo->lock_xo), xo_flag);

    return BSP_PMU_OK;

}
/*****************************************************************************
 函 数 名  : pmic_xo_clk_disable
 功能描述  : 关闭对应路32k时钟
 输入参数  : pmu xo 时钟枚举值
 输出参数  : 无
 返 回 值  : 操作成功或失败
 调用函数  :
 被调函数  : pmu adp文件调用
*****************************************************************************/
int pmic_xo_clk_disable(int clk_id)
{
    struct comm_xo* comm_xo = &(g_pmic_comminfo.comm_xo);
    struct ctrl_xo* ctrl_xo = NULL;
	unsigned long xo_flag = 0;

    /*check para*/
    if(pmic_xo_check_para(clk_id))
    {
        return BSP_PMU_ERROR;
    }
    ctrl_xo = &(comm_xo->ctrl_xo[clk_id]);
    spin_lock_irqsave(&(comm_xo->lock_xo), xo_flag);
    (void)bsp_ipc_spin_lock(IPC_SEM_PMU);
    pmic_reg_write_mask(ctrl_xo->addr,(0x0 << ctrl_xo->bit),(0x1 << ctrl_xo->bit));
    (void)bsp_ipc_spin_unlock(IPC_SEM_PMU);
    spin_unlock_irqrestore(&(comm_xo->lock_xo), xo_flag);

    return BSP_PMU_OK;
}
/*****************************************************************************
 函 数 名  : pmic_xo_clk_disable
 功能描述  : 关闭对应路32k时钟
 输入参数  : pmu xo 时钟枚举值
 输出参数  : 无
 返 回 值  : 操作成功或失败
 调用函数  :
 被调函数  : pmu adp文件调用
*****************************************************************************/
int pmic_xo_clk_is_enabled(int clk_id)
{
    struct comm_xo* comm_xo = &(g_pmic_comminfo.comm_xo);
    struct ctrl_xo* ctrl_xo = NULL;
    u32 reg_value = 0;
	unsigned long xo_flag = 0;

    /*check para*/

    ctrl_xo = &(comm_xo->ctrl_xo[clk_id]);
    spin_lock_irqsave(&(comm_xo->lock_xo), xo_flag);
    (void)bsp_ipc_spin_lock(IPC_SEM_PMU);
    pmic_reg_read(ctrl_xo->addr,&reg_value);
    (void)bsp_ipc_spin_unlock(IPC_SEM_PMU);
    spin_unlock_irqrestore(&(comm_xo->lock_xo), xo_flag);

    return !!(reg_value & (0x1 << ctrl_xo->bit));
}

/*****************************************************************************
 函 数 名  : pmic_power_key_state_get
 功能描述  : 获取usb是否插拔状态
 输入参数  : void
 输出参数  : 无
 返 回 值  : usb插入或拔出(1:插入；0:拔出)
 调用函数  :
 被调函数  : 开关机模块
*****************************************************************************/
bool pmic_power_key_state_get(void)
{
    u32 regval = 0;

    pmic_reg_read(PMIC_POWERKEY_STATE_REG, &regval);
    return  (regval & PMIC_POWERKEY_STATE_MASK) ? 1 : 0;
}
/*****************************************************************************
 函 数 名  : pmic_usb_state_get
 功能描述  : 获取usb是否插拔状态
 输入参数  : void
 输出参数  : 无
 返 回 值  : usb插入或拔出(1:插入；0:拔出)
 调用函数  :
 被调函数  : 开关机模块
*****************************************************************************/
bool pmic_usb_state_get(void)
{
    u32 regval = 0;

    pmic_reg_read(PMIC_USB_STATE_REG, &regval);
    return  (regval & PMIC_USB_STATE_MASK) ? 1 : 0;
}
/*****************************************************************************
 函 数 名  : pmic_version_get
 功能描述  : 获取pmu的版本号
 输入参数  : void
 输出参数  : 无
 返 回 值  : pmu版本号
 调用函数  :
 被调函数  : pmu adp文件调用
*****************************************************************************/
char* pmic_version_get(void)
{
    u32 val = 0;
    u32 i = 0;
    
    static bool b_geted=false;
    static char version[5];

    if(!b_geted)
    {
        (void)memset((void*)version, 0, 5);
        
        for(i = 0; i < 4; i++)
        {
            pmic_reg_read(PMIC_VERSION_REG0+i, &val);
            version[i] = (u8)val;
        }
        version[4] = '\0';
        b_geted=true;      
    }
    pmu_print_error("version:%s\n", version);
    return  version;

}

u32 pmic_subversion_get(void)
{
    u32 regval = 0;
    u32 val1 = 0;
    u32 val2 = 0;

    pmic_reg_read(PMIC_VERSION_REG4, &val1);
    pmic_reg_read(PMIC_VERSION_REG5, &val2);

    regval = ((val1 << 8) | val2);
    return  regval;
}

/*****************************************************************************
 函 数 名  : pmic_usbphy_ldo_set
 功能描述  : 设置usbphy所在电源域不受状态机控制(usb lpm唤醒特性需求)
 输入参数  : void
 输出参数  : 无
 返 回 值  : pmu版本号
 调用函数  :
 被调函数  : pmu adp文件调用
*****************************************************************************/

struct pmu_adp_ops pmic_ops = {
    .clk_32k_enable = pmic_32k_clk_enable,
    .clk_32k_disable = pmic_32k_clk_disable,
    .clk_32k_is_enabled = pmic_32k_clk_is_enabled,
    .clk_xo_enable = pmic_xo_clk_enable,
    .clk_xo_disable = pmic_xo_clk_disable,
    .clk_xo_is_enabled = pmic_xo_clk_is_enabled,
    .usb_state_get = pmic_usb_state_get,
    .key_state_get = pmic_power_key_state_get,
    .irq_mask = pmic_irq_mask,
    .irq_unmask = pmic_irq_unmask,
    .irq_is_masked = pmic_irq_is_masked,
    .irq_callback_register = pmic_irq_callback_register,
    .sim_debtime_set = dummy_sim_debtime_set,
    .ldo22_res_enable = dummy_no_para_handler,
    .ldo22_res_disable = dummy_no_para_handler,
    .version_get = pmic_version_get,
    .sim_upres_disable = dummy_sim_upres_disable,
    .ocp_register = pmic_ocp_register,
    .ioshare_status_get = pmic_ioshare_status_get,
};

void pmic_comm_dts_init(void)
{
    struct device_node *dev_node = NULL;
    struct device_node *child_node = NULL;
    int ret = 0;
    u32 id_32k = 0, id_xo;
    u32 ctrl_size = 0;
    struct comm_32k* comm_32k = &g_pmic_comminfo.comm_32k;
    struct comm_xo* comm_xo = &g_pmic_comminfo.comm_xo;

    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,pmic_comm_32k");
    if (!dev_node)
    {
        pmic_print_error("can not find node hisilicon,pmic_comm_32k\n");
        return;
    }

    comm_32k->num_32k = of_get_child_count(dev_node);
    ctrl_size = sizeof(struct ctrl_32k)*(comm_32k->num_32k);
    comm_32k->ctrl_32k = (struct ctrl_32k *)osl_malloc(ctrl_size);

    for_each_child_of_node(dev_node, child_node)
    {
        ret = of_property_read_u32(child_node, "id",&id_32k);
        if(ret)
        {
            pmic_print_error("pmic comm get 32k id from dts failed,err_id %d!\n",ret);
            return;
        }

        ret = of_property_read_u32_array(child_node, "32k_ctrl",&(comm_32k->ctrl_32k[id_32k].addr),sizeof(struct ctrl_32k)/sizeof(u32));

        if (ret) {
            pmic_print_error("pmic comm get 32k ctrl from dts failed,err_id %d!\n",ret);
            return;
        }

    }

    dev_node = NULL;
    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,pmic_comm_xo");
    if (!dev_node)
    {
        pmic_print_error("can not find node hisilicon,pmic_comm_xo\n");
        return;
    }
    comm_xo->num_xo = of_get_child_count(dev_node);
    ctrl_size = sizeof(struct ctrl_xo)*(comm_xo->num_xo);
    comm_xo->ctrl_xo = (struct ctrl_xo *)osl_malloc(ctrl_size);

    for_each_child_of_node(dev_node, child_node)
    {
        ret = of_property_read_u32(child_node, "id",&id_xo);
        if(ret)
        {
            pmic_print_error("pmic comm get xo id from dts failed,err_id %d!\n",ret);
            return;
        }

        ret = of_property_read_u32_array(child_node, "xo_ctrl",&(comm_xo->ctrl_xo[id_xo].addr),sizeof(struct ctrl_xo)/sizeof(u32));

        if (ret) {
            pmic_print_error("pmic comm get xo ctrl from dts failed,err_id %d!\n",ret);
            return;
        }

    }
    spin_lock_init(&(comm_32k->lock_32k));
    spin_lock_init(&(comm_xo->lock_xo));
}

void pmic_comm_init(void)
{
    /*read dts*/
    pmic_comm_dts_init();
    spin_lock_init(&(g_pmic_comminfo.lock));
    pmu_adpops_register(&pmic_ops);
}

