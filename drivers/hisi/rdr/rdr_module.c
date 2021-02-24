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
#include <osl_list.h>
#include <osl_sem.h>
#include <osl_malloc.h>

#include <linux/hisi/rdr_pub.h>
#include "bsp_slice.h"
#include "bsp_nvim.h"
#include "rdr_exc.h"
#include "rdr_debug.h"
#include "rdr_area.h"
#include "rdr_comm.h"
#include "rdr_module.h"

struct rdr_module_info_s g_st_module_table[MNTN_MAX] = {
    {
        .initState      = false,
        .coreid         = RDR_AP,
        .core           = "AP",
        .modeid_start   = HISI_MNTN_EXC_AP_START,
        .modeid_end     = HISI_MNTN_EXC_AP_END,
    },
    {
        .initState      = false,
        .coreid         = RDR_CP,
        .core           = "CP",
        .modeid_start   = HISI_MNTN_EXC_CP_START,
        .modeid_end     = HISI_MNTN_EXC_CP_START,
    },
    {
        .initState      = false,
        .coreid         = RDR_MDMAP,
        .core           = "MDMAP",
        .modeid_start   = HISI_MNTN_EXC_MDMAP_START,
        .modeid_end     = HISI_MNTN_EXC_MDMAP_END,
    },
    {
        .initState      = false,
        .coreid         = RDR_TEEOS,
        .core           = "TEEOS",
        .modeid_start   = HISI_MNTN_EXC_TEEOS_START,
        .modeid_end     = HISI_MNTN_EXC_TEEOS_END,
    },
    {
        .initState      = false,
        .coreid         = RDR_LPM3,
        .core           = "LPM3",
        .modeid_start   = HISI_MNTN_EXC_LPM3_START,
        .modeid_end     = HISI_MNTN_EXC_LPM3_END,
    },
};

static inline struct rdr_module_info_s* rdr_get_module_info_bycoreid(u64 coreid)
{
    s32 i = 0;

    for(i = 0;i< MNTN_MAX; i++)
    {
        if((coreid == g_st_module_table[i].coreid)&&(g_st_module_table[i].initState))
            return &g_st_module_table[i];
    }
    rdr_err("invalid parameter or module info not initial! 0x%llx\n",coreid);
    show_stack(current, NULL);
    return NULL;
}

static inline struct rdr_module_info_s* rdr_get_module_info_bymodid(u32 modeid)
{
    s32 i = 0;
    for(i = 0;i< MNTN_MAX; i++)
    {
        if(((modeid <= g_st_module_table[i].modeid_end)\
            &&(modeid >= g_st_module_table[i].modeid_start))\
            &&(g_st_module_table[i].initState))
            return &g_st_module_table[i];
    }
    return &g_st_module_table[MNTN_CP];/*不在其他子系统范围内的mode id统一划归Modem 子系统*/
}

char* rdr_get_module_name(u32 modeid)
{
    struct rdr_module_info_s*  module_info = NULL;

    module_info = rdr_get_module_info_bymodid(modeid);
    if(NULL == module_info)
    {
        return NULL;
    }
    return module_info->core;
}
EXPORT_SYMBOL(rdr_get_module_name);

u64 rdr_get_coreid_bymodeid(u32 modeid)
{
    struct rdr_module_info_s*  module_info = NULL;

    module_info = rdr_get_module_info_bymodid(modeid);
    if(NULL == module_info)
    {
        return RDR_MAX;
    }
    return module_info->coreid;
}
EXPORT_SYMBOL(rdr_get_coreid_bymodeid);

u32 rdr_get_module_areainfo(u64 coreid,struct rdr_register_module_result * areainfo)
{
    struct rdr_module_info_s*  module_info;

    if(NULL == areainfo){
        rdr_err("invalid parameter!\n");
        return RDR_ERROR;
    }

    module_info = rdr_get_module_info_bycoreid(coreid);
    if(NULL == module_info)
    {
        return RDR_ERROR;
    }

    areainfo->log_addr = (uintptr_t)module_info->addr;
    areainfo->log_len  = module_info->len;
    areainfo->nve      = module_info->nve;

    return RDR_OK;
}
EXPORT_SYMBOL(rdr_get_module_areainfo);

u32 rdr_register_module_callback_ops(u64 coreid,struct rdr_module_ops_pub* ops)
{
    struct rdr_module_info_s*  module_info;

    if(NULL == ops->ops_dump || NULL == ops->ops_reset){
        rdr_err("invalid parameter!\n");
        return RDR_ERROR;
    }

    module_info = rdr_get_module_info_bycoreid(coreid);
    if(NULL == module_info)
    {
        return RDR_ERROR;
    }

    if(RDR_DEBUG_SW(RDR_DEBUG_OPS_REG_SW))
        return RDR_OK;

    module_info->dump_callback = ops->ops_dump;
    module_info->reset_callback = ops->ops_reset;
    return RDR_OK;
}
EXPORT_SYMBOL(rdr_register_module_callback_ops);

u32 rdr_get_module_ops(u64 coreid,struct rdr_module_ops_pub* ops)
{
    struct rdr_module_info_s*  module_info;

    module_info = rdr_get_module_info_bycoreid(coreid);
    if(NULL == module_info)
    {
        return RDR_ERROR;
    }

    ops->ops_dump = module_info->dump_callback ;
    ops->ops_reset= module_info->reset_callback;
    return RDR_OK;
}
EXPORT_SYMBOL(rdr_get_module_ops);

static inline u32 rdr_add_module_exception(struct rdr_module_info_s*  module_info,struct rdr_exception_info_s* e)
{
    struct rdr_exception_info_s* exc_info = NULL;
    unsigned long exc_flags;
    struct rdr_exception_info_s* cur = NULL;
    struct list_head *p,*n;


    exc_info = (struct rdr_exception_info_s*)kmalloc(sizeof(struct rdr_exception_info_s),GFP_ATOMIC);
    if(NULL == exc_info){
        rdr_err("kmalloc failed!\n");
        return RDR_ERROR;
    }
    memcpy(exc_info,e,sizeof(struct rdr_exception_info_s));

    spin_lock_irqsave(&(module_info->spinlock), exc_flags);
    list_for_each_safe(p, n, &(module_info->excList))
    {
        cur = (struct rdr_exception_info_s*)list_entry(p, struct rdr_exception_info_s, e_list);
        if((exc_info->e_modid_end >= cur->e_modid && exc_info->e_modid_end <= cur->e_modid_end)
           ||(exc_info->e_modid >= cur->e_modid && exc_info->e_modid <= cur->e_modid_end)){
            spin_unlock_irqrestore(&(module_info->spinlock), exc_flags);
            kfree(exc_info);
            rdr_err("do not allow register twice or register the same modeid [%s]!\n",rdr_get_exception_type(exc_info->e_exce_type));
            return RDR_ERROR;
        }
    }
    list_add_tail(&(exc_info->e_list),&(module_info->excList));
    spin_unlock_irqrestore(&(module_info->spinlock), exc_flags);
    return RDR_OK;
}

u32 rdr_register_module_exception(u64 coreid,struct rdr_exception_info_s* e)
{
    struct rdr_module_info_s*  module_info = NULL;

    if((NULL == e)||(coreid >= RDR_MAX)){
        rdr_err("invalid parameter!\n");
        return RDR_ERROR;
    }

    module_info = rdr_get_module_info_bymodid(e->e_modid);
    if(NULL == module_info){
        return RDR_ERROR;
    }

    if(RDR_DEBUG_SW(RDR_DEBUG_EXCEPTION_REG_SW))
        return RDR_OK;

    if(rdr_add_module_exception(module_info,e)){
        return RDR_ERROR;
    }
    return RDR_OK;
}
EXPORT_SYMBOL(rdr_register_module_exception);

struct rdr_exception_info_s* rdr_get_exception_info(u32 modeid)
{
    struct rdr_module_info_s*  module_info = NULL;
    struct rdr_exception_info_s* exc_info = NULL;
    struct list_head *p,*n;
    unsigned long exc_flags;

    module_info = rdr_get_module_info_bymodid(modeid);
    if(NULL == module_info)
    {
        return NULL;
    }
    spin_lock_irqsave(&(module_info->spinlock), exc_flags);
    list_for_each_safe(p, n, &(module_info->excList))
    {
        exc_info = (struct rdr_exception_info_s*)list_entry(p, struct rdr_exception_info_s, e_list);
        if((exc_info->e_modid <= modeid)&&(exc_info->e_modid_end >= modeid))
        {
            spin_unlock_irqrestore(&(module_info->spinlock), exc_flags);
            return exc_info;
        }
    }
    spin_unlock_irqrestore(&(module_info->spinlock), exc_flags);
    return NULL;
}
EXPORT_SYMBOL(rdr_get_exception_info);

/*
 * func name: rdr_callback
 * func args:
 *  rdr_exception_info_s *p_exce_info;
 *	u32 mod_id
 *	char *logpath
 */
void rdr_callback(struct rdr_exception_info_s *exc_info, u32 modeid,char *path)
{
    int i;
    struct rdr_exception_info_s* cur;
    struct list_head *p,*n;
    unsigned long exc_flags;

    if((NULL == exc_info)||(NULL == path))
    {
        rdr_err("Invalid parameters!\n");
        return;
    }

    if(exc_info->e_callback)
    {
        exc_info->e_callback(modeid,path);
    }

    for(i = 0; i<MNTN_MAX; i++)
    {
        if((exc_info->e_from_core == g_st_module_table[i].coreid)||(!g_st_module_table[i].initState))/*同一个核的不再处理*/
            continue;

        spin_lock_irqsave(&g_st_module_table[i].spinlock, exc_flags);
        list_for_each_safe(p, n, &g_st_module_table[i].excList)
        {
            cur = (struct rdr_exception_info_s*)list_entry(p, struct rdr_exception_info_s, e_list);
            if((exc_info->e_from_core & cur->e_notify_core_mask)&&(cur->e_callback)&&(cur->e_exce_type != exc_info->e_exce_type))
            {
                spin_unlock_irqrestore(&g_st_module_table[i].spinlock, exc_flags);
                cur->e_callback(modeid,path);
                spin_lock_irqsave(&g_st_module_table[i].spinlock, exc_flags);
            }
        }
        spin_unlock_irqrestore(&g_st_module_table[i].spinlock, exc_flags);
    }
    return;
}


u32 rdr_module_init(void)
{
    s32 i = 0;
    struct rdr_area_mntn_addr_info_s  area_info;
    char desc[64];

    for(i = 0;i< MNTN_MAX;i++)
    {
        if(g_st_module_table[i].initState)
            continue;

        INIT_LIST_HEAD(&g_st_module_table[i].excList);
        spin_lock_init(&g_st_module_table[i].spinlock);

        memset(&area_info,0,sizeof(area_info));

        if(rdr_get_area_info(g_st_module_table[i].coreid, &area_info))
        {
            rdr_err("get area ddr info err! 0x%llx\n",g_st_module_table[i].coreid);
            return RDR_ERROR;
        }
        g_st_module_table[i].addr = area_info.paddr;
        g_st_module_table[i].len  = area_info.len;

        memset(desc,0,sizeof(desc));
        snprintf(desc,sizeof(desc),"Show module [%s] Exception info!",g_st_module_table[i].core);
        rdr_debug_register_debug_info(desc,rdr_show_exception_info,(u32)g_st_module_table[i].coreid);
        g_st_module_table[i].initState = true;
    }

    rdr_debug_register_debug_info("Show all module global info", rdr_show_module_info,0);
    rdr_err("ok!\n");

    return RDR_OK;
}
/*
 * func name: rdr_print_one_exc
 * return	NULL
 */
void rdr_print_one_exc(struct rdr_exception_info_s *e)
{
	printk(KERN_ERR" modid:          [0x%x]\n", e->e_modid);
	printk(KERN_ERR" modid_end:      [0x%x]\n", e->e_modid_end);
	printk(KERN_ERR" process_pri:    [0x%x]\n", e->e_process_priority);
	printk(KERN_ERR" reboot_pri:     [0x%x]\n", e->e_reboot_priority);
	printk(KERN_ERR" notify_core_mk: [0x%llx]\n", e->e_notify_core_mask);
	printk(KERN_ERR" reset_core_mk:  [0x%llx]\n", e->e_reset_core_mask);
	printk(KERN_ERR" reentrant:      [%s]\n", (e->e_reentrant == RDR_REENTRANT_ALLOW)?"allow":"disallow");
	printk(KERN_ERR" exce_type:      [%s]\n", rdr_get_exception_type(e->e_exce_type));
	printk(KERN_ERR" from_core:      [%s]\n", rdr_get_core_name(e->e_from_core));
	printk(KERN_ERR" from_module:    [%s]\n", e->e_from_module);
	printk(KERN_ERR" desc:           [%s]\n", e->e_desc);
	printk(KERN_ERR" callback:       [%pS]\n", e->e_callback);
	printk(KERN_ERR" reserve_u32:    [0x%x]\n", e->e_reserve_u32);
	printk(KERN_ERR" reserve_p:      [0x%p]\n", e->e_reserve_p);
}

void rdr_show_module_info(u32 arg)
{
    int i = 0;

    UNUSED(arg);

    for(i=0;i<MNTN_MAX;i++)
    {
        printk(KERN_ERR"======================[%s]======================\n",g_st_module_table[i].core);
        printk(KERN_ERR"Init State: [%s] \n",g_st_module_table[i].initState?"OK":"NO");
        printk(KERN_ERR"Mode id Arrange:[0x%x]~[0x%x]\n",g_st_module_table[i].modeid_start,g_st_module_table[i].modeid_end);
        printk(KERN_ERR"Mntn Phy Addr:[0x%p],Mntn ddr len:[0x%x]\n",g_st_module_table[i].addr,g_st_module_table[i].len);
        printk(KERN_ERR"Ops Dump Callback: %pS\n",g_st_module_table[i].dump_callback);
        printk(KERN_ERR"Ops Reset Callback: %pS\n",g_st_module_table[i].reset_callback);
        printk(KERN_ERR"<INFO>:<ecall rdr_show_exception_info 0x%x> to show this module Exceptions !\n",RDR_CORE(i));
    }
}

void rdr_show_exception_info(u32 core)
{
    struct rdr_module_info_s*  module_info = NULL;
    struct rdr_exception_info_s* cur;
    struct list_head *n,*p;
    u64     coreid = 0;

    coreid = core;

    module_info = rdr_get_module_info_bycoreid(coreid);
    if(NULL == module_info)
    {
        printk(KERN_ERR"Invalid core :0x%llx\n",coreid);
        return;
    }
    printk(KERN_ERR"==============[%s start]================\n",module_info->core);
    list_for_each_safe(p,n,&(module_info->excList))
    {
        cur = (struct rdr_exception_info_s*)list_entry(p, struct rdr_exception_info_s, e_list);
        rdr_print_one_exc(cur);
        printk(KERN_ERR"\n");
    }
    printk(KERN_ERR"==============[%s  end ]================\n",module_info->core);
}


