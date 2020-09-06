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

#include <product_config.h>
#include <linux/string.h>
#include <osl_bio.h>
#include <osl_spinlock.h>
#include <osl_thread.h>
#include <osl_list.h>
#include <of.h>
#include <soc_memmap.h>
#include <bsp_om.h>
#include <bsp_regulator.h>
#include <bsp_pmu.h>
#include <bsp_shared_ddr.h>
#include <pmic_inner.h>
#include <pmic_volt.h>
#include <pmic_dr.h>
#include <hi_pmu.h>

extern struct pmic_volt_info g_pmic_voltinfo;
extern struct pmic_dr_info g_pmic_drinfo;
extern struct list_head regulator_list;

#define pmu_print_dbg(fmt, ...) (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_PMU, fmt,##__VA_ARGS__))

/*****************************************************************************
* 函 数 : pmic_log_level_set
* 功 能 : pmic 相关打印等级设置
* 输 入 : void
* 输 出 : void
* 返 回 :
* 说 明 :
*****************************************************************************/
void pmic_log_level_set_debug(u32 level)
{
    (void)bsp_mod_level_set(BSP_MODU_PMU, level);
    pmic_print_info("bbp log id=%d, set to level=%d)\n",BSP_MODU_PMU,level);
}
/*****************************************************************************
* 函 数 : pmic_log_level_get
* 功 能 : 获取bbp 相关打印等级
* 输 入 : void
* 输 出 : void
* 返 回 :
* 说 明 :
*****************************************************************************/
u32 pmic_log_level_get_debug(void)
{
    u32 log_level = 0;

    log_level = bsp_log_module_cfg_get(BSP_MODU_PMU);
    pmic_print_dbg("bbp log id=%d, level=%d\n",BSP_MODU_PMU,log_level);
    return log_level;
}

void pmic_addr_debug(void)
{
    int volt_id = 0;
    struct pmic_volt_ctrl *volt_ctrl = NULL;
    struct pmic_volt_reg *volt_reg = NULL;

    pmu_print_dbg("%-10s%-10s%-10s%-10s%-10s%-10s", "id", "name", "en_reg","en_bit","dis_reg","dis_bit");
    pmu_print_dbg("%-10s%-10s%-10s%-10s\n", "state_reg","state_bit","volt_reg", "volt_mask");

    for(volt_id = g_pmic_voltinfo.volt_min; volt_id <= g_pmic_voltinfo.volt_max; volt_id++)
    {
        volt_ctrl = &g_pmic_voltinfo.volt_ctrl[volt_id];
        volt_reg = &g_pmic_voltinfo.volt_ctrl[volt_id].volt_reg;
        pmu_print_dbg("%-10d%-10s0x%-8x%-10d0x%-8x%-10d",volt_ctrl->id,volt_ctrl->name,\
            volt_reg->en_reg,volt_reg->en_bit,volt_reg->dis_reg,volt_reg->dis_bit);
        pmu_print_dbg("0x%-8x%-10d0x%-8x0x%-8x\n",volt_reg->state_reg,volt_reg->state_bit,\
            volt_reg->voltage_reg,volt_reg->voltage_mask);
    }
}
void pmic_voltage_table_debug(void)
{
    struct pmic_volt_ctrl *volt_ctrl = NULL;
    int volt_id = 0;
    u32 selector,voltage = 0;

    for(volt_id = g_pmic_voltinfo.volt_min; volt_id <= g_pmic_voltinfo.volt_max; volt_id++)
    {
        volt_ctrl = &g_pmic_voltinfo.volt_ctrl[volt_id];

        pmu_print_dbg("%-10d%-10s num:%d table(uV):",volt_id,volt_ctrl->name,volt_ctrl->voltage_nums);
        for(selector = 0;selector < volt_ctrl->voltage_nums;selector++)
        {
            voltage = pmic_volt_list_voltage(volt_id,selector);
            pmu_print_dbg(" %d",voltage);
        }
        pmu_print_dbg(".\n");
    }
}
void pmic_volt_id_debug(void)
{
    struct pmic_volt_ctrl *volt_ctrl = NULL;
    int volt_id = 0;

    pmu_print_dbg("%-10s%-10s\n","id", "name");
    for(volt_id = g_pmic_voltinfo.volt_min; volt_id <= g_pmic_voltinfo.volt_max; volt_id++)
    {
        volt_ctrl = &g_pmic_voltinfo.volt_ctrl[volt_id];
        pmu_print_dbg("%-10d%-10s\n",volt_ctrl->id,volt_ctrl->name);
    }
}

void pmic_volt_state_debug(void)
{
    struct pmic_volt_ctrl *volt_ctrl = NULL;
    int volt_id = 0;
    int onoff = 0;
    int voltage = 0;

    pmu_print_dbg("%-10s%-10s%-20s%-20s\n","id", "name","is_enabled","voltage(uV)");
    for(volt_id = g_pmic_voltinfo.volt_min; volt_id <= g_pmic_voltinfo.volt_max; volt_id++)
    {
        onoff = !!pmic_volt_is_enabled(volt_id);
        voltage = pmic_volt_get_voltage(volt_id);
        volt_ctrl = &g_pmic_voltinfo.volt_ctrl[volt_id];
        pmu_print_dbg("%-10d%-10s%-20d%-20d\n",volt_ctrl->id,volt_ctrl->name,onoff,voltage);
    }
}

void pmic_volt_set_debug(int volt_id, int voltage)
{
    unsigned selector = 0;
    
    (void)pmic_volt_set_voltage(volt_id, voltage, voltage,&selector);
    pmu_print_dbg("set %d to %d, selector = %d\n", volt_id, voltage, selector);
}


void pmic_get_shm_mem_debug(void)
{
	pmu_print_dbg("shm_mem_pmu_npreg_addr:0x%x\n", SHM_MEM_PMU_NPREG_ADDR);
}

void pmu_help(void)
{
    pmu_print_dbg("pmic_volt_enable(volt_id)                               :enable volt\n");
    pmu_print_dbg("pmic_volt_disable(volt_id)                              :disable volt\n");
    pmu_print_dbg("pmic_volt_is_enabled(volt_id)                           :show volt state\n");
    pmu_print_dbg("pmic_volt_get_voltage(volt_id)                          :get volt voltage\n");
    pmu_print_dbg("pmic_volt_list_show(volt_id)                      :show volt all voltage\n");
    pmu_print_dbg("pmic_addr_debug          :show addr info\n");
    pmu_print_dbg("pmic_voltage_table_debug :show voltage table\n");
    pmu_print_dbg("pmic_volt_id_debug       :show id name\n");
    pmu_print_dbg("pmic_volt_state_debug    :show onoff voltage info\n");
    pmu_print_dbg("pmu_regulator_debug      :show power all relationship\n");
	pmu_print_dbg("pmic_get_shm_mem_debug	:show shm mem addr");
}

EXPORT_SYMBOL_GPL(pmic_log_level_set_debug);
EXPORT_SYMBOL_GPL(pmic_log_level_get_debug);
EXPORT_SYMBOL_GPL(pmic_addr_debug);
EXPORT_SYMBOL_GPL(pmic_voltage_table_debug);
EXPORT_SYMBOL_GPL(pmic_volt_id_debug);
EXPORT_SYMBOL_GPL(pmic_volt_state_debug);
EXPORT_SYMBOL_GPL(pmic_volt_set_debug);
EXPORT_SYMBOL_GPL(pmic_get_shm_mem_debug);
EXPORT_SYMBOL_GPL(pmu_help);

