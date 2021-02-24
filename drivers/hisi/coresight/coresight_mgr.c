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

#include "coresight.h"
#include "bsp_dump.h"
#include "bsp_coresight.h"
#include "bsp_nvim.h"
#include "osl_common.h"
#include "drv_nv_def.h"
#include "drv_nv_id.h"
#include "bsp_ap_dump.h"

/* tmc memory address in dump */   
static u8* g_tmc_dump_addr[CAPT_CTRL_BUTT] = {0};

/* tmc memory length in dump */
static u32 g_tmc_dump_len[CAPT_CTRL_BUTT]  = {0};

/* coresight enable flag */
u32 g_cs_enable = false;

/* coresight suspend flag */
u32 g_cs_suspend =  false;

/* coresight-tmc data buffer address */
extern u32 g_tmc_data_addr;
/* coresight-tmc data buffer length */
extern u32 g_tmc_data_len;

extern void etm_cpu_enable(unsigned int cpu);
extern void etm_cpu_disable(unsigned int cpu);

/* coresight enable */
void bsp_coresight_enable(void)
{
    etm_cpu_enable(AP_CORE_SET);
}

/* coresight disable */
void bsp_coresight_disable(void)
{   
    etm_cpu_disable(AP_CORE_SET);
}

void coresight_save_data(u32 core_set)
{
    if(!g_tmc_data_addr || !g_tmc_data_len)
    {
        printk("%s(core %u): no data in tmc\n", __FUNCTION__, core_set);
        return;
    }
   
    if(g_tmc_data_len > g_tmc_dump_len[core_set])
    {
        printk("%s(core %u): dump buffer 0x%x too small, expect 0x%x\n", __FUNCTION__, core_set, g_tmc_dump_len[core_set], g_tmc_data_len);
        return;
    }

    if(*((u32 *)g_tmc_dump_addr[core_set]) != CORESIGHT_MAGIC_NUM)
    {
        /* the first four bytes for magic number */
        *((u32 *)g_tmc_dump_addr[core_set]) = (u32)CORESIGHT_MAGIC_NUM;
        /* tht second four bytes for tmc data length */
        *((u32 *)g_tmc_dump_addr[core_set] + 1) = g_tmc_data_len; 
    
        memcpy((u32*)g_tmc_dump_addr[core_set]+2, (void*)g_tmc_data_addr, g_tmc_data_len);
        printk("[coresight]:save %s TMC data to dump OK\n", (core_set==AP_CORE_SET)?"AP":"CP");
    }
    else
    {
        printk("[coresight]:%s TMC data has been saved\n", (core_set==AP_CORE_SET)?"AP":"CP");
    }
}

void bsp_coresight_save_cp_data(void)
{
    NV_DUMP_STRU   dump_nv;

    /* read dump NV */
    if(BSP_OK != bsp_nvm_read(NVID_DUMP, (u8 *)&dump_nv, sizeof(NV_DUMP_STRU)))
    {
        printk("%s: read nv %d fail\n", __FUNCTION__, NVID_DUMP);
        return;
    }

    if(dump_nv.traceCoreSet & (1 << CP_CORE_SET))
    {
        g_tmc_dump_addr[CP_CORE_SET] = (u8*)bsp_dump_get_field_addr(DUMP_CP_UTRACE);
        if(BSP_NULL == g_tmc_dump_addr[CP_CORE_SET])
        {
            printk("%s: get cp dump buffer fail\n", __FUNCTION__);
            return;
        }
        g_tmc_dump_len[CP_CORE_SET] = DUMP_CP_UTRACE_SIZE;

        etm_cpu_disable(CP_CORE_SET);
        coresight_save_data(CP_CORE_SET);

        printk("%s: save cp data ok\n", __FUNCTION__);
    }
    else
    {
        printk("%s: cp trace is not enable\n", __FUNCTION__);
    }
}

/* coresight callback function for dump */
void bsp_coresight_dump_callback(void)
{
    bsp_coresight_disable();
    coresight_save_data(AP_CORE_SET);
}

/* coresight intialization */
int bsp_coresight_init(void)
{
    dump_handle handle;
    NV_DUMP_STRU   dump_nv;

    /* read dump NV */
    if(BSP_OK != bsp_nvm_read(NVID_DUMP, (u8 *)&dump_nv, sizeof(NV_DUMP_STRU)))
    {
        printk("%s:  read nv %d fail\n", __FUNCTION__, NVID_DUMP);
        return BSP_ERROR;
    }

    /* judge coresight enable set */
    if(dump_nv.traceOnstartFlag != 1)
    {
        printk("%s: trace not enable\n", __FUNCTION__);
        return BSP_OK;
    }
    
    /* register field in dump  */
    if(dump_nv.traceCoreSet & (1 << AP_CORE_SET))
    {
        g_tmc_dump_addr[AP_CORE_SET] = (u8*)bsp_ap_dump_register_field(DUMP_KERNEL_UTRACE, "AP_CORESIGHT", 0, 0, DUMP_AP_UTRACE_SIZE, 0);
        if(BSP_NULL == g_tmc_dump_addr[AP_CORE_SET])
        {
            printk("%s:  get ap dump buffer fail\n", __FUNCTION__);
            return BSP_ERROR;
        }
        g_tmc_dump_len[AP_CORE_SET] = DUMP_AP_UTRACE_SIZE;
        memset(g_tmc_dump_addr[AP_CORE_SET], 0, g_tmc_dump_len[AP_CORE_SET]);
    }
    else
    {
        printk("%s: AP trace is not set\n", __FUNCTION__);
        return BSP_OK;
    }

    /* register dump callback */
    handle = bsp_ap_dump_register_hook("AP_CORESIGHT", (dump_hook)bsp_coresight_dump_callback);
    if((handle == BSP_ERROR) ||(handle==(dump_handle)NULL))
    {
        printk("%s:  dump register fail\n", __FUNCTION__);
        return BSP_ERROR;
    }
    
    /* enable etm and coresight */
    bsp_coresight_enable();

    g_cs_enable = true;

    printk("%s: init ok\n", __FUNCTION__);
    return BSP_OK;
}


void bsp_coresight_suspend(void)
{
    if(g_cs_enable == false)
    {
        return;
    }
    g_cs_suspend = true;

    bsp_coresight_disable();
}

void bsp_coresight_resume(void)
{
    if(g_cs_enable == false)
    {
        return;
    }
    g_cs_suspend = false;
    bsp_coresight_enable();
}

static int __init __bsp_coresight_init(void)
{
	return (int)bsp_coresight_init();
}

late_initcall(__bsp_coresight_init);


