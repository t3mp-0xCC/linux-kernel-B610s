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
#include <linux/hisi/rdr_pub.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include "rdr_debug.h"


struct rdr_debug_ctrl_info_s g_rdr_debug_ctrl;

struct rdr_debug_sw_info_s g_rdr_sw_info[]=
{
    RDR_DEBUG_TYPE(RDR_DEBUG_PRINT_SW),
    RDR_DEBUG_TYPE(RDR_DEBUG_OPS_REG_SW),
    RDR_DEBUG_TYPE(RDR_DEBUG_EXCEPTION_REG_SW),
    RDR_DEBUG_TYPE(RDR_DEBUG_EXCEPTION_PROCESS_SW),
    RDR_DEBUG_TYPE(RDR_DEBUG_SAVE_RDR_SW),
    RDR_DEBUG_TYPE(RDR_DEBUG_SAVE_HISTORY_SW),
    RDR_DEBUG_TYPE(RDR_DEBUG_CREATE_DIR_SW),
    RDR_DEBUG_TYPE(RDR_DEBUG_RESET_SW),
    RDR_DEBUG_TYPE(RDR_DEBUG_DUMP_SW),
};


void rdr_debug_set_sw(u32 ops,u32 flag)
{
    if(flag){
        g_rdr_debug_ctrl.ulSw |= (1 << ops);
    }
    else{
        g_rdr_debug_ctrl.ulSw &= ~(1 << ops);
    }
}
EXPORT_SYMBOL(rdr_debug_set_sw);

void rdr_debug_sw_info(void)
{
    int i;
    int count;

    count = sizeof(g_rdr_sw_info)/sizeof(g_rdr_sw_info[0]);
    for(i=0;i<count;i++)
    {
        printk(KERN_ERR"use 'ecall rdr_debug_set_sw %d 1' to open %s \n",\
            g_rdr_sw_info[i].sw_type,g_rdr_sw_info[i].name);
    }
}
EXPORT_SYMBOL(rdr_debug_sw_info);

void rdr_debug_register_debug_info(char* desc,void (*help)(u32 arg),u32 arg)
{
    struct rdr_debug_help_info_s* debug_help = NULL;
    if(rdr_debug_init())
    {
        return;
    }

    if(NULL == desc || NULL == help)
    {
        rdr_err("Invalid parameter!\n");
        return;
    }
    debug_help = (struct rdr_debug_help_info_s*)kmalloc(sizeof(struct rdr_debug_help_info_s),GFP_KERNEL);
    if(NULL == debug_help)
    {
        rdr_err("kmalloc failed\n");
        return;
    }
    memset(debug_help,0,sizeof(struct rdr_debug_help_info_s));
    debug_help->help = help;
    memcpy(debug_help->desc,desc, sizeof(debug_help->desc)>strlen(desc) ? strlen(desc) : sizeof(debug_help->desc));
    debug_help->arg  = arg;

    list_add_tail(&debug_help->help_list,&g_rdr_debug_ctrl.help);
}
EXPORT_SYMBOL(rdr_debug_register_debug_info);

void rdr_help(u32 arg)
{
    struct list_head *n,*p;
    struct rdr_debug_help_info_s * cur;

    printk(KERN_ERR"================================================\n");
    printk(KERN_ERR">>>>>>>>>>>>>>>>>>>>rdr help<<<<<<<<<<<<<<<<<<<<\n");
    printk(KERN_ERR"================================================\n");
    list_for_each_safe(p, n, &g_rdr_debug_ctrl.help)
    {
        cur = (struct rdr_debug_help_info_s*)list_entry(p, struct rdr_debug_help_info_s, help_list);
        printk(KERN_ERR"\n [Function]:'%pS' \n [Argument]:'0x%x' \n [Discription]:'%s' \n\n",cur->help,cur->arg,cur->desc);
        if(true == arg)
            cur->help(cur->arg);
        printk(KERN_ERR"\n");
    }
    printk(KERN_ERR"=================rdr debug info=================\n");
    rdr_debug_sw_info();
    printk(KERN_ERR"================================================\n");
    printk(KERN_ERR"================================================\n");
    printk(KERN_ERR"================================================\n");
}
EXPORT_SYMBOL(rdr_help);

u32 rdr_debug_init(void)
{
    if(g_rdr_debug_ctrl.initState)
    {
        return RDR_OK;
    }

    INIT_LIST_HEAD(&g_rdr_debug_ctrl.help);
    g_rdr_debug_ctrl.initState = true;
    rdr_err("ok!\n");
    return RDR_OK;
}



