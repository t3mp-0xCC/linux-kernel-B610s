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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <drv_comm.h>
#include <asm/uaccess.h>
#include <osl_bio.h>
#include <bsp_ipc.h>
#include <bsp_memmap.h>
#include <bsp_dump.h>
#include <drv_nv_id.h>
#include <drv_nv_def.h>
#include <bsp_nvim.h>
#include <bsp_icc.h>
#include <bsp_pmu.h>
#include <bsp_shared_ddr.h>
#include <osl_thread.h>
#include <osl_sem.h>
#include <osl_spinlock.h>

#include <pmic_volt.h>
#include <pmic_irq.h>
#include "pmic_inner.h"

extern struct pmic_irq *g_pmic_irqinfo;/*中断全局结构体*/

/*****************************************************************************
 函 数 名  : pmic_irq_state
 功能描述  : get irq state information
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 备注说明  : 无
*****************************************************************************/
void pmic_irq_state(void)
{
    unsigned irq_nums = g_pmic_irqinfo->irq_nums;/*num of irq*/
    struct pmic_irq_handle *irq_handler;
    int index = 0;

    pmic_print_dbg("************pmic irq info*******************");

    pmic_print_dbg("%-20s%-20s\n","irq_num","irq_cnts");
    pmic_print_dbg("%-20d%-20d\n",g_pmic_irqinfo->irq,g_pmic_irqinfo->cnts);

    pmic_print_dbg("%-20s%-20s\n","sub_irq_num","sub_irq_cnts");

    for(index = 0; index < irq_nums; index++)
    {
        irq_handler = &(g_pmic_irqinfo->irq_handler[index]);

        pmic_print_dbg("%-20d%-20d\n",irq_handler->irq_num,irq_handler->cnt);
    }
}

/*****************************************************************************
 函 数 名  : pmic_exc_record_info
 功能描述  : 查看record从dts中获取的配置
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 备注说明  : 将record的值在fastboot中已经读到共享内存中
*****************************************************************************/
void pmic_irq_reg(void)
{
    struct pmic_irq_reg *irq_reg = g_pmic_irqinfo->irq_reg;
    struct pmic_irq_special *irq_special= g_pmic_irqinfo->irq_special;

    pmic_print_dbg("*****pmic irq reg************");
    pmic_print_dbg("irqarrays:%d\n",irq_reg->irqarrays);
    pmic_print_dbg("mask_reg base:0x%x\n",irq_reg->mask_base);
    pmic_print_dbg("irq_reg base:0x%x\n",irq_reg->irq_base);

    pmic_print_dbg("*****pmic irq_special************");
    pmic_print_dbg("sim_arry:%d\n",irq_special->sim_arry);
    pmic_print_dbg("sim0_bit:0x%x\n",irq_special->sim0_bit);
    pmic_print_dbg("sim0_ldo base:0x%x\n",irq_special->sim0_ldo);
    pmic_print_dbg("sim1_bit base:0x%x\n",irq_special->sim1_bit);
    pmic_print_dbg("sim1_ldo base:0x%x\n",irq_special->sim1_ldo);

    pmic_print_dbg("key_arry:0x%x\n",irq_special->key_arry);
    pmic_print_dbg("key_pending:0x%x\n",irq_special->key_pending);
    pmic_print_dbg("keyup_irq base:0x%x\n",irq_special->keyup_irq);
    pmic_print_dbg("keydown_irq base:0x%x\n",irq_special->keydown_irq);
}

