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

/*
 * spe_sim_sch.c -- spe simulator schedule process
 *
 */

#include <linux/list.h>
#include "spe_sim.h"
#include "spe_sim_priv.h"
#include "spe_sim_reg.h"
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/unaligned/le_struct.h>
#include <linux/if_ether.h>

#include <linux/device.h>
//#include <linux/etherdevice.h>
#include <linux/crc32.h>
#include <linux/kthread.h>
#include <linux/usb/cdc.h>
#include <net/ip.h>



typedef struct tag_spe_sim_action_node {
    struct list_head node;
    char* entry;
    spe_sim_entry_type_t type;
    spe_sim_entry_action_t action;
}spe_sim_action_node_t;

typedef struct tag_spe_sim_sch_ctx {
    spe_sim_ctx_t* global_ctx;
    struct list_head action_list;
    spinlock_t list_lock;
    struct tasklet_struct sch_tasklet;
}spe_sim_sch_ctx_t;

static spe_sim_ctx_t spe_sim_ctx;
static spe_sim_sch_ctx_t spe_sim_sch_ctx = {0};

int spe_sim_reg_if_port_under_bridge(spe_sim_port_t *port)
{
    if (spe_sim_port_in_br == port->port_type) {
        return 1;
    }
    return 0;
}


int spe_sim_get_port_info(unsigned int port_num, spe_sim_port_t** port)
{
    if (unlikely(port_num >= SPE_SIM_MAX_PORTS)) {
        printk(KERN_EMERG"%s, error port num:%d\n", __func__, port_num);
        return -ENOENT;
    }
    *port = spe_sim_ctx.ports_idx[port_num];
    return 0;
}


static void __spe_sim_proc_entry_action(spe_sim_ctx_t* ctx)
{
    spe_sim_sch_ctx_t* sch_ctx = &spe_sim_sch_ctx;
    struct list_head* head = &sch_ctx->action_list;
    spe_sim_action_node_t* act_node;
    unsigned long flags;
    /* get action node from list in loop */
    for(;;) {

        spin_lock_irqsave(&sch_ctx->list_lock, flags);
        if (list_empty(head)) {
            spin_unlock_irqrestore(&sch_ctx->list_lock, flags);
            break;
        }

        /* get the node from list */
        act_node = list_entry(head->next, spe_sim_action_node_t, node);
		list_del_init(&act_node->node);
        spin_unlock_irqrestore(&sch_ctx->list_lock, flags);

        /* process the action */
        spe_sim_entry_operate(ctx, act_node->action,
                              act_node->type, act_node->entry);

        /* free the node */
        kfree(act_node);
    }
}

static void __spe_sim_process_ports(spe_sim_ctx_t* ctx)
{
    int enable_num;
    int idx = 0;
    int do_again = 0;
    spe_sim_port_t* port;

    enable_num = ctx->ports_en_num;

    /* find available port from the enabled ports */
    while(idx < enable_num) {
        port = ctx->ports_en[idx];
        port->proc_stat = spe_sim_proc_idle;
        port->last_err = 0;
        /* restore the tocken */
        port->tocken = (port->tocken_set == 0)? 1 : port->tocken_set;

        while(spe_sim_can_process_port(port) && port->tocken > 0) {
            /* process one port if the tocken is available */
            spe_sim_td_process(port);
            port->tocken--;

            if (port->last_err < 0)
                break;
        }

        /* current port has works to do, but tocken is not enough */
        if (port->last_err >= 0 &&
            spe_sim_can_process_port(port)) {
            do_again = 1;
        }

        idx++;

        /* if we need do again, reset the idx, restart the loop */
        if (idx >= enable_num && do_again) {
            do_again = 0;
            idx = 0;
        }
    }
}

static void __spe_sim_schedule(unsigned long param)
{
    spe_sim_ctx_t* ctx = (spe_sim_ctx_t*)param;

    /* set busy */
    spe_sim_reg_set_status(ctx, 0);

    /* 1. entry action process */
    __spe_sim_proc_entry_action(ctx);

    /* 2. td process */
    __spe_sim_process_ports(ctx);

    /* set idle */
    spe_sim_reg_set_status(ctx, 1);
}

/* write register to trigger spe schedule */
void spe_sim_schedule_port(spe_sim_port_t* port)
{
    spe_sim_sch_ctx_t* sch_ctx = &spe_sim_sch_ctx;
    /* schedule the tasklet */
    tasklet_schedule(&sch_ctx->sch_tasklet);
}

/* read / add / delete a table entry */
int spe_sim_schedule_entry(spe_sim_ctx_t* ctx, spe_sim_entry_action_t action,
                           spe_sim_entry_type_t type, char* entry)
{
    spe_sim_action_node_t* act_node;
    unsigned long flags;
    spe_sim_sch_ctx_t* sch_ctx = &spe_sim_sch_ctx;

    /* alloc a action node */
    act_node = (spe_sim_action_node_t*)
            kmalloc(sizeof(spe_sim_action_node_t), GFP_ATOMIC);
    if (!act_node) {
        return -ENOMEM;
    }

    /* fill the node */
    INIT_LIST_HEAD(&act_node->node);
    act_node->action = action;
    act_node->type = type;
    act_node->entry = entry;

    /* add action node to list */
    spin_lock_irqsave(&sch_ctx->list_lock, flags);
    list_add_tail(&act_node->node, &sch_ctx->action_list);
    spin_unlock_irqrestore(&sch_ctx->list_lock, flags);

    /* schedule the tasklet */
    tasklet_schedule(&sch_ctx->sch_tasklet);
    return 0;
}

/* init the spe simulator schedule process module */
void spe_sim_sch_init(spe_sim_ctx_t* ctx)
{
    spe_sim_sch_ctx_t* sch_ctx = &spe_sim_sch_ctx;

    sch_ctx->global_ctx = ctx;
    tasklet_init(&sch_ctx->sch_tasklet, __spe_sim_schedule, (unsigned long)ctx);
    INIT_LIST_HEAD(&sch_ctx->action_list);
    spin_lock_init(&sch_ctx->list_lock);
}

void spe_sim_rescan_enable_port(void)
{
    int port_idx;
    int en_idx = 0;
    spe_sim_ctx_t* ctx = &spe_sim_ctx;
    spe_sim_port_t* port;
    unsigned long flags;
    spe_sim_sch_ctx_t* sch_ctx = &spe_sim_sch_ctx;

    spin_lock_irqsave(&sch_ctx->list_lock, flags);

    /* rescan the port table to build the enabled port table */
    for (port_idx = 0; port_idx < SPE_SIM_MAX_PORTS; port_idx++) {
        port = &ctx->ports[port_idx];
        if (port->is_enable) {
            ctx->ports_en[en_idx] = port;
            en_idx++;
        }
    }
    ctx->ports_en_num = en_idx;
    spin_unlock_irqrestore(&sch_ctx->list_lock, flags);
}

/* init the spe simulator */
int __init spe_sim_init(void)
{
    spe_sim_ctx_t* ctx = &spe_sim_ctx;

    memset(ctx, 0, sizeof(spe_sim_ctx_t));
    spe_sim_sch_init(ctx);
    spe_sim_reg_init(ctx);
    spe_sim_rd_init(ctx);
    spe_sim_intr_init();
    printk("\n SPE_Simlator init done!\n");
    return 0;
}
subsys_initcall(spe_sim_init);

void spe_sim_port_show(int port_no)
{
    spe_sim_port_t *port;

    if (port_no > SPE_SIM_MAX_PORTS) {
        printk("invalid port:%d\n", port_no);
    }
    port = &spe_sim_ctx.ports[port_no];

    printk("dump port:%d:\n", port_no);
    printk("enc_type:               %d:\n", port->enc_type);
    printk("stat_td_config:         %d\n", port->debug_info.stat_td_proc);
    printk("stat_td_complete:       %d\n", port->debug_info.stat_td_complete);
    printk("stat_td_intr:           %d\n", port->debug_info.stat_td_intr);
    printk("td_num:                 %d\n", port->td_ctx.td_num);
    printk("td r_pos:               %d\n", port->td_ctx.r_pos);
    printk("td w_pos:               %d\n", port->td_ctx.w_pos);
    printk("\n");
    printk("stat_rd_proc:           %d\n", port->debug_info.stat_rd_proc);
    printk("stat_rd_empty:          %d\n", port->debug_info.stat_rd_empty);
    printk("stat_rd_complete:       %d\n", port->debug_info.stat_rd_complete);
    printk("stat_rd_intr:           %d\n", port->debug_info.stat_rd_intr);
    printk("rd_num:                 %d\n", port->rd_ctx.rd_num);
    printk("rd r_pos:               %d\n", port->rd_ctx.r_pos);
    printk("rd w_pos:               %d\n", port->rd_ctx.w_pos);
    printk("\n");
    printk("stat_entry_proc:        %d\n", port->debug_info.stat_entry_proc);
    printk("stat_mac_entry_proc:    %d\n", port->debug_info.stat_mac_entry_proc);
    printk("stat_ip_entry_proc:     %d\n", port->debug_info.stat_ip_entry_proc);
    printk("stat_to_cpu_proc:       %d\n", port->debug_info.stat_to_cpu_proc);
    printk("stat_do_nat:            %d\n", port->debug_info.stat_do_nat);

}

