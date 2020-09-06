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
#include <osl_malloc.h>

#include <pmic_volt.h>
#include <pmic_irq.h>
#include "pmic_inner.h"
#include "pmic_exc.h"

extern struct pmic_exc *g_pmic_excinfo;/*中断全局结构体*/
extern struct pmic_volt_info g_pmic_voltinfo;
/*****************************************************************************
 函 数 名  : pmic_om_wk_handler
 功能描述  : 将非下电状态记录寄存器写入log文件，并判断此次启动是否由PMU引起的重启
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 备注说明  : 无
*****************************************************************************/
void pmic_record_state(void)
{
    int i,bit = 0;
    struct pmic_record* pmic_record = &(g_pmic_excinfo->record);
    struct record_info* record_info = NULL;
    u32 width = g_pmic_excinfo->data_width;
    unsigned long precord_data = 0;

    /*save and explain record*/
    for(i = 0; i < pmic_record->record_num; i++)
    {
        record_info = &(pmic_record->record_info[i]);
        pmic_print_dbg("reg:0x%x,value:0x%x!\n",record_info->record_addr,record_info->record_data);
        precord_data = (unsigned long)(record_info->record_data);
        for_each_set_bit(bit, &precord_data, width)
        {
            if((record_info->record_data)&(0x1<<bit))
            {
                if((record_info->inacceptable)&(0x1<<bit))
                    pmic_print_dbg("event:%s(abnormal)!\n",record_info->record_name[bit]);
                else
                    pmic_print_dbg("event:%s(normal)!\n",record_info->record_name[bit]);
            }
        }
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
void pmic_record_info(void)
{
    int i,bit = 0;
    struct pmic_record* pmic_record = &(g_pmic_excinfo->record);
    struct record_info* record_info = NULL;
    u32 width = g_pmic_excinfo->data_width;
    unsigned long precord_data = 0;

    /*save and explain record*/
    for(i = 0; i < pmic_record->record_num; i++)
    {
        record_info = &(pmic_record->record_info[i]);
        pmic_print_dbg("reg:0x%x!\n",record_info->record_addr);
        precord_data = (unsigned long)(record_info->record_data);
        for_each_set_bit(bit, &precord_data, width)
        {
            if((record_info->inacceptable)&(0x1<<bit))
                pmic_print_dbg("event:%s(abnormal)!\n",record_info->record_name[bit]);
            else
                pmic_print_dbg("event:%s(normal)!\n",record_info->record_name[bit]);
        }
    }
}

/*****************************************************************************
 函 数 名  : pmic_otp_threshold_set
 功能描述  : 系统启动时设置过温警告阈值
 输入参数  : @threshold 要设置的温度(摄氏度)
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void pmic_otp_info(void)
{
    pmic_excflags_t exc_flags = 0;
    struct pmic_otp *pmic_otp = &g_pmic_excinfo->otp;
    u32 otp_value = 0;

    spin_lock_irqsave(&g_pmic_excinfo->lock,exc_flags);
    pmic_reg_read(pmic_otp->otp_base, &otp_value);
    spin_unlock_irqrestore(&g_pmic_excinfo->lock,exc_flags);

    otp_value = otp_value&(pmic_otp->otp_mask);

    pmic_print_dbg("otp_threshold_reg:\n",otp_value);
    pmic_print_dbg("otp_threshold_value:\n",otp_value);

    return;
}

/*****************************************************************************
 函 数 名  : bsp_hi6559_uvp_threshold_set
 功能描述  : 系统启动时设置欠压警告阈值
 输入参数  : @threshold 要设置的电压(mV)
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void pmic_uvp_info(void)
{
    pmic_excflags_t exc_flags = 0;
    struct pmic_uvp *pmic_uvp = &g_pmic_excinfo->uvp;
    u32 uvp_value = 0;

    spin_lock_irqsave(&g_pmic_excinfo->lock,exc_flags);
    pmic_reg_read(pmic_uvp->uvp_base, &uvp_value);
    spin_unlock_irqrestore(&g_pmic_excinfo->lock,exc_flags);

    uvp_value = uvp_value&(pmic_uvp->uvp_mask);

    pmic_print_dbg("uvp_threshold_reg:\n",uvp_value);
    pmic_print_dbg("uvp_threshold_value:\n",uvp_value);

    return;
}

/*****************************************************************************
 函 数 名  : pmic_exc_ocp_scp_handle
 功能描述  : 过流处理函数
 输入参数  : @para 参数指针
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void pmic_ocp_info(void* para)
{
    struct pmic_ocp *pmic_ocp = &g_pmic_excinfo->ocp;
    struct ocp_info *ocp_info = NULL;
    u32 width = g_pmic_excinfo->data_width;
    int i,bit = 0;
    unsigned long pocp_data = 0;

    pmic_print_dbg("ocp irq:\n",pmic_ocp->ocp_irq);

    /*save and explain record*/
    for(i = 0; i < pmic_ocp->ocp_num; i++)
    {
        ocp_info = &(pmic_ocp->ocp_info[i]);
        pmic_print_dbg("ocp reg:0x%x!\n",ocp_info->ocp_addr);

        pmic_print_dbg("ocp_name:");
        pocp_data = (unsigned long)(ocp_info->ocp_data);
        for_each_set_bit(bit, &pocp_data, width)
        {
            pmic_print_dbg("%s ",ocp_info->ocp_name[bit]);
        }
        pmic_print_dbg("\n");
    }
}

void pmic_ocp_handler(int volt_id)
{
    pmic_print_dbg("ocp %d error!\n", volt_id);
    //pmic_print_dbg("ocp %d error, system will be restarted!\n", volt_id);
    //system_error(DRV_ERRNO_PMU_OVER_CUR, volt_id, 0, NULL, 0);
}

void pmic_ocp_test(void)
{
    u32 index;
    for(index = g_pmic_voltinfo.volt_min; index <= g_pmic_voltinfo.volt_max; index++)
    {
        bsp_pmu_ocp_register(index, pmic_ocp_handler);
    }   
}

void pmic_pro_show(void)
{
    struct pmic_pro *pmic_pro = &g_pmic_excinfo->pro;
    struct pmic_volt_ctrl *volt_ctrl = NULL;
    int index = 0;
    int volt_id = 0;

    pmic_print_dbg("ocp_rst(when over current,system will be reboot):%d\n",pmic_pro->ocp_rst);
    pmic_print_dbg("otp_rst(when over temperature,system will be reboot):%d\n",pmic_pro->otp_rst);
    pmic_print_dbg("otp_off(when over temperature,the num of volt will be close):%d\n",pmic_pro->otp_off_num);

    if(pmic_pro->otp_off_num)
        pmic_print_dbg("otp_off_volt:(when over temperature,this volt will be close)\n");

    for(index = 0; index < pmic_pro->otp_off_num; index++)
    {
        volt_id = pmic_pro->otp_off_arry[index];
        volt_ctrl = &(g_pmic_voltinfo.volt_ctrl[volt_id]);
        pmic_print_dbg("volt_id:%d ,volt_name:%s. \n",volt_id,volt_ctrl->name);
    }
}

