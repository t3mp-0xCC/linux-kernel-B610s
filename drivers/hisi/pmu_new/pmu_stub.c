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

#include <bsp_pmu.h>
#include "pmu_balong.h"
#include "pmu_stub.h"


/*****************************************************************************
 函 数 名  : dummy_32k_clk_enable
 功能描述  : 开启、关闭、查询pmu中32k时钟打桩函数
 输入参数  : clk_id:32k时钟编号
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
int dummy_32k_clk_handler(int clk_id)
{
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}
/*****************************************************************************
 函 数 名  : dummy_irq_handler
 功能描述  : mask,unmask pmu中包含的模块的irq打桩函数
 输入参数  : clk_id:32k时钟编号
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
void dummy_irq_mask_handler(unsigned int irq)
{
    pmu_print_error("no pmic macro defined,use dummy!");
}
/*****************************************************************************
 函 数 名  : dummy_irq_handler
 功能描述  : 查询pmu中包含模块中断是否屏蔽打桩函数
 输入参数  : clk_id:32k时钟编号
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
int dummy_irq_state_handler(unsigned int irq)
{
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}
/*****************************************************************************
 函 数 名  : bsp_pmu_irq_callback_register
 功能描述  : 注册中断处理回调函数默认打桩函数
 输入参数  : irq
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  : 包含在pmu内部的中断模块
*****************************************************************************/
int dummy_irq_callback_register(unsigned int irq,pmufuncptr routine,void *data)
{
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}
/*****************************************************************************
 函 数 名  : dummy_get_boot_state
 功能描述  :系统启动时检查pmu寄存器状态，默认打桩函数
 输入参数  : void
 输出参数  : reset.log
 返 回 值  : pmu问题或ok
 调用函数  :
 被调函数  :系统可维可测
*****************************************************************************/
int dummy_get_boot_state(void)
{
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}
/*****************************************************************************
 函 数 名  : dummy_get_part_state
 功能描述  :系统启动时检查pmu内usb\热启动等状态，默认打桩函数
 输入参数  : void
 输出参数  : reset.log
 返 回 值  : pmu问题或ok
 调用函数  :
 被调函数  :充放电、开关机
*****************************************************************************/
bool dummy_get_part_state(void)
{
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}
/*****************************************************************************
 函 数 名  : dummy_sim_debtime_set
 功能描述  : 设置SIM卡中断去抖时间的打桩函数
 输入参数  : para:参数
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
int dummy_sim_debtime_set(u32 para)
{
    pmu_print_info("para is %d",para);
    pmu_print_error("no pmic macro defined,use dummy!");
    return  BSP_PMU_NO_PMIC;
}

void dummy_no_para_handler(void)
{
    pmu_print_info("no pmic macro defined,use dummy!");
}

char* dummy_version_get(void)
{
    pmu_print_info("no pmic macro defined,use dummy!\n");
    return NULL;
}

/*****************************************************************************
 函 数 名  : dummy_info_get_handler
 功能描述  : 查询PMU版本号的打桩函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
u32 dummy_info_get_handler(void)
{
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}

int dummy_sim_upres_disable(u32 para)
{
    pmu_print_info("para is %d",para);
    pmu_print_error("no pmic macro defined,use dummy!");
    return  BSP_PMU_NO_PMIC;
}

int dummy_ocp_register(int volt_id,PMU_OCP_FUNCPTR func)
{
    pmu_print_info("no pmic macro defined,use dummy!\n");
    return 0;
}

int dummy_one_para_handler(u32 para)
{
    pmu_print_info("para is %d",para);
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}

