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
#include <osl_types.h>
#include <osl_bio.h>
#include <osl_malloc.h>

#include <linux/hisi/rdr_pub.h>
#include <bsp_shared_ddr.h>
#include <bsp_dump.h>
#include "rdr_area.h"
#include "rdr_debug.h"
#include "rdr_comm.h"


struct rdr_area_ctrl_info_s g_rdr_area_ctrl;
EXPORT_SYMBOL(g_rdr_area_ctrl);

void * rdr_vmap(phys_addr_t paddr, size_t size)
{
    return ioremap_wc(paddr,size);
}
EXPORT_SYMBOL(rdr_vmap);

void rdr_umap(void* vaddr)
{
    iounmap(vaddr);
}

u32 rdr_get_area_info(u64 coreid,struct rdr_area_mntn_addr_info_s* area_info)
{
    u32 core = 0;

    if(!g_rdr_area_ctrl.initState)
    {
        return RDR_ERROR;
    }

    if(NULL == area_info)
    {
        rdr_err("invalid parameter!\n");
        return RDR_ERROR;
    }
    switch(coreid)
    {
        case RDR_AP:
            core = MNTN_AP;
            break;
        case RDR_CP:
            core = MNTN_CP;
            break;
        case RDR_LPM3:
            core = MNTN_LPM3;
            break;
        case RDR_TEEOS:
            core = MNTN_TEEOS;
            break;
        case RDR_MDMAP:
            core = MNTN_MDMAP;
            break;
        default:
            rdr_err("invalid core id! 0x%llx\n",coreid);
            return RDR_ERROR;
    }

    area_info->vaddr    = (void*)g_rdr_area_ctrl.virt_base_addr+g_rdr_area_ctrl.virt_base_addr->area_info[core].offset;
    area_info->paddr    = (void*)(g_rdr_area_ctrl.phy_base_addr+g_rdr_area_ctrl.virt_base_addr->area_info[core].offset);
    area_info->len      = g_rdr_area_ctrl.virt_base_addr->area_info[core].length;

    return RDR_OK;

}
EXPORT_SYMBOL(rdr_get_area_info);

void rdr_get_mntn_addr_info(struct rdr_area_mntn_addr_info_s* area_info)
{
    if(!g_rdr_area_ctrl.initState)
    {
        return;
    }
    area_info->paddr = g_rdr_area_ctrl.phy_base_addr;
    area_info->vaddr = g_rdr_area_ctrl.virt_base_addr;
    area_info->len   = g_rdr_area_ctrl.base_len;
    return;
}
EXPORT_SYMBOL(rdr_get_mntn_addr_info);

void rdr_get_backup_mntn_addr_info(struct rdr_area_mntn_addr_info_s* area_info)
{
    if(!g_rdr_area_ctrl.initState)
    {
        return;
    }
    area_info->paddr = g_rdr_area_ctrl.phy_backup_addr;
    area_info->vaddr = g_rdr_area_ctrl.virt_backup_addr;
    area_info->len   = g_rdr_area_ctrl.backup_len;
    return;
}
EXPORT_SYMBOL(rdr_get_backup_mntn_addr_info);

void rdr_fill_syserr_para(u32 modid,u32 arg1,u32 arg2)
{
    if(!g_rdr_area_ctrl.initState)
    {
        return;
    }
    g_rdr_area_ctrl.virt_base_addr->base_info.modid = modid;
    g_rdr_area_ctrl.virt_base_addr->base_info.arg1  = arg1;
    g_rdr_area_ctrl.virt_base_addr->base_info.arg2  = arg2;
    return ;
}
EXPORT_SYMBOL(rdr_fill_syserr_para);

void rdr_fill_exc_info(struct rdr_exception_info_s* exc_info,char* date)
{
    if(!g_rdr_area_ctrl.initState)
    {
        return;
    }
    if((NULL == exc_info)||(NULL ==date))
    {
        return;
    }
	g_rdr_area_ctrl.virt_base_addr->base_info.e_core = exc_info->e_from_core;
	g_rdr_area_ctrl.virt_base_addr->base_info.e_type = exc_info->e_exce_type;
	memcpy(g_rdr_area_ctrl.virt_base_addr->base_info.datetime, date, sizeof(g_rdr_area_ctrl.virt_base_addr->base_info.datetime));
	memcpy(g_rdr_area_ctrl.virt_base_addr->base_info.e_module, exc_info->e_from_module, sizeof(g_rdr_area_ctrl.virt_base_addr->base_info.e_module));
	memcpy(g_rdr_area_ctrl.virt_base_addr->base_info.e_desc, exc_info->e_desc, STR_EXCEPTIONDESC_MAXLEN);

    g_rdr_area_ctrl.virt_base_addr->base_info.start_flag         = RDR_START_EXCH;
    g_rdr_area_ctrl.virt_base_addr->base_info.savefile_flag      = RDR_SAVE_FILE_NEED;

    return ;
}
EXPORT_SYMBOL(rdr_fill_exc_info);

void rdr_fill_save_done(void)
{
    if(!g_rdr_area_ctrl.initState)
    {
        return;
    }
    writel(RDR_SAVE_FILE_END, &g_rdr_area_ctrl.virt_base_addr->base_info.savefile_flag);
}
EXPORT_SYMBOL(rdr_fill_save_done);

void rdr_fill_reboot_done(void)
{
    if(!g_rdr_area_ctrl.initState)
    {
        return;
    }
    if(RDR_START_EXCH != g_rdr_area_ctrl.virt_base_addr->base_info.start_flag)
    {
        writel(SOFT_REBOOT, &g_rdr_area_ctrl.virt_base_addr->base_info.e_type);
    }
    writel(RDR_START_REBOOT,&g_rdr_area_ctrl.virt_base_addr->base_info.reboot_flag);
}
EXPORT_SYMBOL(rdr_fill_reboot_done);

void rdr_init_baseinfo(void)
{
    if(!g_rdr_area_ctrl.initState)
    {
        return;
    }
    memset(&(g_rdr_area_ctrl.virt_base_addr->base_info),0,sizeof(g_rdr_area_ctrl.virt_base_addr->base_info));
}
EXPORT_SYMBOL(rdr_init_baseinfo);


u32 rdr_area_init(void)
{
    dump_load_info_t * dump_load;
    if(g_rdr_area_ctrl.initState)
    {
        return RDR_OK;
    }
    g_rdr_area_ctrl.phy_base_addr   = (void*)MNTN_BASE_ADDR;
    g_rdr_area_ctrl.base_len        = MNTN_BASE_SIZE;
    g_rdr_area_ctrl.virt_base_addr  = (struct rdr_struct_s*)rdr_vmap(MNTN_BASE_ADDR,MNTN_BASE_SIZE);

    if(NULL == g_rdr_area_ctrl.virt_base_addr)
    {
        rdr_err("map mntn addr error!\n");
        return RDR_ERROR;
    }

    /*判断rdr头部标志是否存在，如果不存在则需要重新初始化*/
    if(g_rdr_area_ctrl.virt_base_addr->top_head.magic != RDR_TOP_HEAD_MAGIC)
    {
        memset(g_rdr_area_ctrl.virt_base_addr,0,sizeof(struct rdr_struct_s));

        g_rdr_area_ctrl.virt_base_addr->top_head.area_number     = MNTN_MAX;
        g_rdr_area_ctrl.virt_base_addr->top_head.magic           = RDR_TOP_HEAD_MAGIC;
        strcpy(g_rdr_area_ctrl.virt_base_addr->top_head.build_time,__DATE__);
        strcat(g_rdr_area_ctrl.virt_base_addr->top_head.build_time,",");
        strcat(g_rdr_area_ctrl.virt_base_addr->top_head.build_time,__TIME__);
        strcpy(g_rdr_area_ctrl.virt_base_addr->top_head.product_name,PRODUCT_NAME);
        strcpy(g_rdr_area_ctrl.virt_base_addr->top_head.product_version,PRODUCT_FULL_VERSION_STR);

        g_rdr_area_ctrl.virt_base_addr->area_info[MNTN_AP].offset     = RDR_AREA_AP_ADDR - RDR_BASEINFO_ADDR;
        g_rdr_area_ctrl.virt_base_addr->area_info[MNTN_AP].length     = RDR_AREA_AP_SIZE;
        g_rdr_area_ctrl.virt_base_addr->area_info[MNTN_CP].offset     = RDR_AREA_CP_ADDR - RDR_BASEINFO_ADDR;
        g_rdr_area_ctrl.virt_base_addr->area_info[MNTN_CP].length     = RDR_AREA_CP_SIZE;
        g_rdr_area_ctrl.virt_base_addr->area_info[MNTN_MDMAP].offset  = RDR_AREA_MDMAP_ADDR - RDR_BASEINFO_ADDR;
        g_rdr_area_ctrl.virt_base_addr->area_info[MNTN_MDMAP].length  = RDR_AREA_MDMAP_SIZE;
        g_rdr_area_ctrl.virt_base_addr->area_info[MNTN_LPM3].offset   = RDR_AREA_LPM3_ADDR - RDR_BASEINFO_ADDR;
        g_rdr_area_ctrl.virt_base_addr->area_info[MNTN_LPM3].length   = RDR_AREA_LPM3_SIZE;
        g_rdr_area_ctrl.virt_base_addr->area_info[MNTN_TEEOS].offset  = RDR_AREA_TEEOS_ADDR - RDR_BASEINFO_ADDR;
        g_rdr_area_ctrl.virt_base_addr->area_info[MNTN_TEEOS].length  = RDR_AREA_TEEOS_SIZE;
    }

    dump_load = (dump_load_info_t *)((u8*)g_rdr_area_ctrl.virt_base_addr+(g_rdr_area_ctrl.base_len-MNTN_AREA_RESERVE_SIZE));
    dump_load->magic_num    = DUMP_LOAD_MAGIC;
    dump_load->ap_ddr  = 0xC0000000;
    dump_load->ap_share= (uintptr_t)SHM_BASE_ADDR;
    dump_load->ap_dump = (u32)g_rdr_area_ctrl.virt_base_addr;
    dump_load->ap_sram = (u32)(uintptr_t)HI_SRAM_MEM_ADDR_VIRT;
    dump_load->ap_dts  = HI_IO_ADDRESS(DDR_ACORE_DTS_ADDR);

    g_rdr_area_ctrl.backup_len          = DDR_SOCP_SIZE;
    g_rdr_area_ctrl.phy_backup_addr     = (void*)DDR_SOCP_ADDR;
    if(g_rdr_area_ctrl.backup_len)
    {
        g_rdr_area_ctrl.virt_backup_addr    = (struct rdr_struct_s*)rdr_vmap((phys_addr_t)g_rdr_area_ctrl.phy_backup_addr,g_rdr_area_ctrl.backup_len);
        if(NULL == g_rdr_area_ctrl.virt_backup_addr)
        {
            rdr_err("map mntn backup addr error!\n");
        }
    }

    g_rdr_area_ctrl.initState = true;

    rdr_err("ok!\n");

    return RDR_OK;
}

