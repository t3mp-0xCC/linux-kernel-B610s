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
 * spe_sim_reg.c -- spe simulatore register process
 *
 */
//#include <stdlib.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include "spe_sim.h"
#include "spe_sim_priv.h"
#include "spe_sim_priv.h"
#include "spe_sim_reg.h"



#define	SPE_SIM_OVERALL_SIZE       0x1000
#define	SPE_SIM_SUCCESS	          	0
#define SPE_SIM_FAIL                1


#define MASK_EVENT_BUFFER_COUNT		0x00ff
#define MASK_EVENT_BUFFER_SIZE		0x00ff
#define	MASK_EVENT_BUFFER_EN		0x8000
#define	MASK_PORT_TYPE		        0x6000
#define PORT_TYPE_RIGHT_SHIFT       25
#define SPE_SIM_REG_WRITE           0

typedef struct tag_spe_sim_reg_func_table {
    unsigned int (*spe_sim_func_regs)(spe_sim_ctx_t* ctx, int port_num);
    unsigned int offset;
}spe_sim_reg_func_table_t;


static char* base_addr;
static spe_sim_ctx_t* the_sim_ctx;



#if 0
/* init some useful regs */
static void spe_sim_init_regs(void)
{
    HI_SPE_RDY_T ready;
    unsigned int* tmp;

    tmp = (unsigned int*)(base_addr + SPE_SIM_INIT_DONE);
    ready.u32 = *tmp;
    ready.bits.spe_rdy = 1;
    *tmp = ready.u32;
    return;
}
#endif

/* static */
void spe_sim_reg_init(spe_sim_ctx_t* ctx)
{
    HI_SPE_INT_STATE_T reg;
    HI_SPE_IDLE_T idle;
    unsigned int *tmp;

    SPE_SIM_TRACE("%s entry \n",__func__);
    base_addr = (unsigned char *)kzalloc(SPE_SIM_OVERALL_SIZE, GFP_KERNEL);
    printk("reg base addr is 0x%x\n", base_addr);
    if (NULL == base_addr) {
        printk(KERN_EMERG"spe_sim_reg_init alloc reg mem fail\n");
        return;
    }
    //spe_sim_init_regs();
    tmp = (unsigned int*)(base_addr + HI_SPE_INT_STATE_OFFSET);

    reg.u32 = *tmp;
    reg.bits.spe_event_buff0_int_state = 1;
    *tmp = reg.u32;

    /* idle status is always 1 */
    tmp = (unsigned int*)(base_addr + HI_SPE_IDLE_OFFSET);
    idle.u32 = *tmp;
    idle.bits.spe_idle = 1;
    *tmp = idle.u32;

    the_sim_ctx = ctx;
    return;
}

void spe_sim_reg_exit(void)
{
    kfree(base_addr);
}

unsigned int spe_sim_reg_reserved(spe_sim_ctx_t* ctx, int port_num)
{
    return SPE_SIM_SUCCESS;
}

void spe_sim_reg_set_status(spe_sim_ctx_t* ctx, int status)
{
    HI_SPE_IDLE_T reg;
    unsigned int *tmp = (unsigned int*)(base_addr + SPE_SIM_GLB_STAT);

    reg.u32 = *tmp;
    reg.bits.spe_idle = status;
    *tmp = reg.u32;

    return;
}

static unsigned int spe_sim_reg_global_enable(spe_sim_ctx_t* ctx, int port_num)
{
    HI_SPE_EN_T reg;
	static int init = 0;

    reg.u32 = *(unsigned int*)(base_addr + SPE_SIM_GLB_ENABLE);
    if (reg.bits.spe_en && !init) {
        printk("spe sim enabled\n");
		init = 1;
    }

    if (init == 1 && reg.bits.spe_en == 0) {
        //spe_sim_intr_re_init();
        printk("spe sim disabled\n");
    }

    init = reg.bits.spe_en;
    return 0;
}

static unsigned int spe_sim_reg_get_age_time(spe_sim_ctx_t* ctx, int port_num)
{
    HI_SPE_AGING_TIME_T    timer;
    timer= *(HI_SPE_AGING_TIME_T *)(base_addr + SPE_SIM_AG_TMOUT);
    spe_sim_set_mac_age_time((unsigned int)timer.bits.spe_mac_aging_time);
    spe_sim_set_ip_age_time((unsigned int)timer.bits.spe_ip_aging_time);

    return 0;
}

static unsigned int spe_sim_reg_set_time(spe_sim_ctx_t* ctx, int port_num)
{
    *(unsigned int *)(base_addr + SPE_SIM_REG_TIMER) = (unsigned int)jiffies;
    return 0;
}

static unsigned int spe_sim_reg_get_globle_ctl(spe_sim_ctx_t* ctx, int port_num)
{
    HI_SPE_USRFIELD_CTRL_T global_ctl;
    global_ctl.u32 = *(unsigned int *)(base_addr + SPE_SIM_GLB_CFG_REG);
    ctx->usr_field_num = global_ctl.bits.spe_usrfield_ctrl;
    return 0;
}

/* get enc type from register */
static unsigned int spe_sim_reg_get_property(spe_sim_ctx_t* ctx, int port_num)
{
    HI_SPE_PORTX_PROPERTY_0_T ch_property;
    int old_enable = ctx->ports[port_num].is_enable;

    SPE_SIM_TRACE("reg: spe_sim_reg_get_property:entry\n");
    SPE_SIM_INFO("reg:spe_sim_reg_get_property port num is %u\n", port_num);

    /* process enc_type */
    ch_property.u32 = *(unsigned int *)(base_addr + SPE_SIM_PORT_DEF(port_num));
    SPE_SIM_INFO("reg:spe_sim_reg_get_property ch_property.u32 value 0x%x\n",ch_property.u32);

    ctx->ports[port_num].enc_type =
        (spe_sim_enc_type_t)ch_property.bits.spe_port_enc_type;
    SPE_SIM_INFO("reg:spe_sim_reg_get_property enc_type value 0x%x\n", ctx->ports[port_num].enc_type);

    /* set cpu port ptr */
    if (spe_sim_enc_cpu == ctx->ports[port_num].enc_type) {
        ctx->cpu_port = &ctx->ports[port_num];
        spe_sim_reg_set_cpu_port(ctx->cpu_port);
    }

    /* process ch enable */
    if (ch_property.bits.spe_port_en) {
        ctx->ports[port_num].is_enable = 1;
    }
    else {
        ctx->ports[port_num].is_enable = 0;
        if (1 == old_enable) {
            spe_sim_recycle_all_td(&ctx->ports[port_num]);
        }
    }
    spe_sim_rescan_enable_port();

    /* process port type */
    ctx->ports[port_num].port_type = ch_property.bits.spe_port_attach_brg;
    printk("new_reg_sim: port %u type is %u\n", port_num,ctx->ports[port_num].port_type);
    ctx->ports[port_num].ctx =ctx;

    return 0;
}

/* get pri from register */
static unsigned int spe_sim_reg_get_pri(spe_sim_ctx_t* ctx, int port_num)
{
	HI_SPE_TDQX_PRI_0_T pri;
	SPE_SIM_INFO("%s reg: port num is %u\n", __func__, port_num);
	
	pri.u32 = *(unsigned int *)(base_addr + SPE_SIM_TD_PRI(port_num));
	SPE_SIM_INFO("%s reg: port pri is %u\n", __func__, pri.u32);

	ctx->ports[port_num].tocken_set = (int)pri.bits.spe_tdq_pri;

	return 0;
}

static unsigned int spe_sim_reg_get_port_num(spe_sim_ctx_t* ctx, int port_num)
{
    HI_SPE_CHX_PORTNUM_0_T ch_num;
	
    ch_num.u32 = *(unsigned int*)(base_addr + SPE_SIM_PORT_NUM(port_num));
    ctx->ports_idx[ch_num.bits.spe_ch_portnum] = &ctx->ports[port_num];
    SPE_SIM_TRACE("reg:%s phy port : %u, loc port : %u ports_idx 0x%x\n",
		__func__,port_num,ch_num.bits.spe_ch_portnum,ctx->ports_idx[ch_num.bits.spe_ch_portnum]);
	ctx->ports[port_num].port_num = port_num;
//    spe_sim_rescan_port_by_num(ctx, ch_num.bits.spe_tdq_ch_num);
    return 0;
}


/* set td/rd info to ctx port info */
static unsigned int spe_sim_reg_get_td_addr(spe_sim_ctx_t* ctx, int port_num)
{
    unsigned int tmp_base;

    tmp_base = (*(unsigned int*)(base_addr + SPE_SIM_TD_ADDR(port_num)));
    /* becare, we must trans the phy address to virt */
	ctx->ports[port_num].td_ctx.base =
	    (char*)spe_sim_phy_to_virt(tmp_base, SPE_SIM_DESC_ADDR_TYPE);
    return 0;
}

static unsigned int spe_sim_reg_get_td_size(spe_sim_ctx_t* ctx, int port_num)
{
    unsigned int td_num;
    td_num = *(unsigned int*)(base_addr + SPE_SIM_TD_DEPT(port_num));
    ctx->ports[port_num].td_ctx.td_num = td_num;
    return 0;
}

static unsigned int spe_sim_reg_get_rd_addr(spe_sim_ctx_t* ctx, int port_num)
{
    unsigned int tmp_base;
	
    tmp_base = (*(unsigned int*)(base_addr + SPE_SIM_RD_ADDR(port_num)));
    ctx->ports[port_num].rd_ctx.base =
        (char*)spe_sim_phy_to_virt(tmp_base, SPE_SIM_DESC_ADDR_TYPE);
    return 0;
}

static unsigned int spe_sim_reg_get_rd_size(spe_sim_ctx_t* ctx, int port_num)
{
    unsigned int rd_num;
    rd_num = *(unsigned int *)(base_addr + SPE_SIM_RD_DEPT(port_num));
    ctx->ports[port_num].rd_ctx.rd_num = rd_num;
    return 0;
}

static unsigned int spe_sim_reg_get_td_wptr(spe_sim_ctx_t* ctx, int port_num)
{
    HI_SPE_TDQX_PTR_0_T td_ptr;
 //   spe_sim_port_t* port;
	
    td_ptr.u32 = *(unsigned int*)(base_addr + SPE_SIM_TD_WR_PTR(port_num));
    ctx->ports[port_num].td_ctx.w_pos = (int)td_ptr.bits.spe_tdq_wptr;
//    spe_sim_get_port_info(port_num, &port);
//    spe_sim_reg_set_td_rptr(port);

    spe_sim_schedule_port(NULL);

    return 0;
}

static unsigned int spe_sim_reg_get_rd_wptr(spe_sim_ctx_t* ctx, int port_num)
{
    HI_SPE_RDQX_PTR_0_T rd_ptr;
//    spe_sim_port_t* port;

//    spe_sim_get_port_info(port_num, &port);
//    spe_sim_reg_set_rd_rptr(port);
    rd_ptr.u32 = *(unsigned int*)(base_addr + SPE_SIM_RD_WR_PTR(port_num));
    SPE_SIM_INFO("reg: port %d \n" ,port_num);
    SPE_SIM_INFO("reg: rd reg addr is 0x%x \n" ,base_addr + SPE_SIM_RD_WR_PTR(port_num));	

    SPE_SIM_INFO("reg: rd wprt  %d  \n" ,(int)rd_ptr.bits.spe_rdq_wptr,rd_ptr.u32);
    SPE_SIM_INFO("reg: rd_ptr.u32 0x%x\n" ,rd_ptr.u32);
    ctx->ports[port_num].rd_ctx.w_pos = (int)rd_ptr.bits.spe_rdq_wptr;

    return 0;
}


static unsigned int spe_sim_reg_get_ipf_mac(spe_sim_ctx_t* ctx, int port_num)
{
    struct spe_sim_mac_fw_entry spe_sim_entry;
    struct spe_mac_addr_heads* mac_addr;
    spe_msg_level =0x3;
    printk("reg: base_addr 0x%x ctx 0x%x \n", base_addr, ctx);
    mac_addr = (struct spe_mac_addr_heads *)(base_addr + HI_SPE_SIM_GLB_SMAC_ADDR_L_OFFSET);
    ctx->ipf_src_mac[0] = mac_addr->mac[3];
    ctx->ipf_src_mac[1] = mac_addr->mac[2];
    ctx->ipf_src_mac[2] = mac_addr->mac[1];
    ctx->ipf_src_mac[3] = mac_addr->mac[0];
		
    mac_addr = (struct spe_mac_addr_heads *)(base_addr + HI_SPE_SIM_GLB_SMAC_ADDR_H_OFFSET);
    ctx->ipf_src_mac[4] = mac_addr->mac[1];
    ctx->ipf_src_mac[5] = mac_addr->mac[0];
	
    SPE_SIM_INFO("reg: ipf port %u src mac is  ",port_num);
    spe_sim_print_mac_addr(&ctx->ipf_src_mac);

    mac_addr = (struct spe_mac_addr_heads *)(base_addr + HI_SPE_SIM_GLB_DMAC_ADDR_L_OFFSET);
    ctx->ipf_dst_mac[0] = mac_addr->mac[3];
    ctx->ipf_dst_mac[1] = mac_addr->mac[2];
    ctx->ipf_dst_mac[2] = mac_addr->mac[1];
    ctx->ipf_dst_mac[3] = mac_addr->mac[0];
		
    mac_addr = (struct spe_mac_addr_heads *)(base_addr + HI_SPE_SIM_GLB_DMAC_ADDR_H_OFFSET);
    ctx->ipf_dst_mac[4] = mac_addr->mac[1];
    ctx->ipf_dst_mac[5] = mac_addr->mac[0];
	
    SPE_SIM_INFO("reg: ipf port %u src mac is  ",port_num);
    spe_sim_print_mac_addr(&ctx->ipf_dst_mac);
    spe_msg_level =0x1;
    return 0;
}


static unsigned int spe_sim_reg_get_port_mac(spe_sim_ctx_t* ctx, int port_num)
{
    struct spe_sim_mac_fw_entry spe_sim_entry;
    struct spe_mac_addr_heads* mac_addr;
		
    mac_addr = (struct spe_mac_addr_heads *)(base_addr + SPE_SIM_PORT_MAC_ADR_L(port_num));
#if 0
    printk("reg mac_addr is 0x%x\n", mac_addr);
    printk("reg base addr is 0x%x\n", base_addr);
    printk("reg ext addr is 0x%x\n", SPE_SIM_PORT_MAC_ADR_H(port_num));
    printk("port num 0x%x\n", port_num);
    printk("ctx addr 0x%x\n", ctx);
    printk("port info addr 0x%x\n", &ctx->ports[port_num]);
    printk("port mac addr 0x%x\n", &ctx->ports[port_num].port_mac[0]);
#endif
	
    ctx->ports[port_num].port_mac[0] = mac_addr->mac[3];
    ctx->ports[port_num].port_mac[1] = mac_addr->mac[2];
    ctx->ports[port_num].port_mac[2] = mac_addr->mac[1];
    ctx->ports[port_num].port_mac[3] = mac_addr->mac[0];
	
    printk("reg ext addr is 0x%x\n", SPE_SIM_PORT_MAC_ADR_L(port_num));
	
    mac_addr = (unsigned int*)(base_addr + SPE_SIM_PORT_MAC_ADR_H(port_num));
    ctx->ports[port_num].port_mac[4] = mac_addr->mac[1];
    ctx->ports[port_num].port_mac[5] = mac_addr->mac[0];
	
    SPE_SIM_INFO("reg: port %u mac is  ",port_num);
    spe_sim_print_mac_addr(&ctx->ports[port_num].port_mac);
    return 0;
}

static unsigned int spe_sim_reg_get_ncm_param(spe_sim_ctx_t* ctx, int port_num)
{
    HI_SPE_PACK_CTRL_T ncm_pack;
    ncm_pack.u32 = *(unsigned int*)(base_addr + SPE_SIM_NCM_SP_CTR);

    /* set ncm param */
    ctx->pack_fixed_in_len = ncm_pack.bits.spe_pack_max_len;
    ctx->pack_max_num = ncm_pack.bits.spe_pack_max_pkt_cnt;
    ctx->is_ntb32 = 1;
    return 0;
}

static unsigned int spe_sim_reg_get_hash_param(spe_sim_ctx_t* ctx, int port_num)
{
    unsigned int hash_base;
    unsigned int hash_zone;
    unsigned int hash_rand;
    HI_SPE_HASH_WIDTH_T hash_width;
    HI_SPE_HASH_DEPTH_T hash_depth;
	
    hash_base = *(unsigned int*)(base_addr + SPE_SIM_HASH_BASE_ADDR);
    hash_zone = *(unsigned int*)(base_addr + SPE_SIM_HASH_ZONE);
    hash_rand = *(unsigned int*)(base_addr + SPE_SIM_HASH_RAND);
    hash_width.u32 = *(unsigned int*)(base_addr + SPE_SIM_HASH_SIZE);
    hash_depth.u32 = *(unsigned int*)(base_addr + SPE_SIM_HASH_DEPTH);
	

    SPE_SIM_INFO("reg:%s hash info: hase_base 0x%x hash_zone 0x%x hash_rand 0x%x hash_width 0x%x \n",
		__func__, hash_base, hash_zone, hash_rand, hash_width.u32);
    hash_base =
        (unsigned int)spe_sim_phy_to_virt(hash_base, SPE_SIM_HASH_ADDR_TYPE);
    SPE_SIM_INFO("reg:%s hash info: hase_base 0x%x hash_zone 0x%x hash_rand 0x%x hash_width 0x%x \n",
		__func__, hash_base, hash_zone, hash_rand, hash_width.u32);
    sep_sim_entry_set_hash_params(hash_base, hash_rand,
        (unsigned short)hash_zone, hash_width.bits.spe_hash_width*4,/* be care the width is word */
        hash_depth.bits.spe_hash_depth);/*the last 2 need to change after redo reg table*/
    return 0;
}



/* get event buffer info from register */
static unsigned int spe_sim_reg_get_intr_timeout(spe_sim_ctx_t* ctx, int port_num)
{
    HI_SPE_INT0_INTERVAL_T intr_timeout;

    intr_timeout.u32 = *(unsigned int*)(base_addr + SPE_SIM_INT_MDRT_TMOUT);
    spe_sim_intr_set_event_timeout(intr_timeout.bits.spe_int0_interval);
    return 0;
}

/*CDE need fix*/
unsigned int spe_sim_reg_set_event_addr(unsigned int buffer_addr)
{
    unsigned int *tmp_addr;
    tmp_addr = (unsigned int *)(base_addr + SPE_SIM_EVENT_ADDR);
    *tmp_addr = buffer_addr;
    return 0;
}

unsigned int spe_sim_reg_get_event_size(spe_sim_ctx_t* ctx)
{
	HI_SPE_EVENTBUF0_PRO_CNT_T evnet_ctl;

	evnet_ctl.u32 = *(unsigned int *)(base_addr + SPE_SIM_EVENT_BUF_LEN);

	return (unsigned int)evnet_ctl.bits.spe_event_buf0_pro_cnt;
}

//CDE sim get event number processed from reg with set buffer
static unsigned int spe_sim_reg_get_event_cnt(spe_sim_ctx_t* ctx, int port_num)
{
	HI_SPE_EVENTBUF0_RPT_CNT_T event_depth;

	event_depth.u32 = *(unsigned int *)(base_addr + SPE_SIM_EVENT_PROCESSED);
	spe_dec_current_event_num(event_depth.bits.spe_event_buf0_rpt_cnt);
	return event_depth.u32;
}

//CDE sim set event number processed to reg couple with set buffer
unsigned int spe_sim_reg_set_event_depth(unsigned int event_number)
{
	unsigned int * tmp;
	HI_SPE_EVENTBUF0_RPT_CNT_T event_depth;
	
	tmp = (unsigned int *)(base_addr + SPE_SIM_EVENT_COUNT);
	event_depth.u32 = *tmp;
	event_depth.bits.spe_event_buf0_rpt_cnt = event_number;
	*tmp = event_depth.u32;

	return 0;
}
/*CDE change int event mask*/
unsigned int spe_sim_reg_get_event_en(char event)
{
#if 0
	int input_event =  event;
	int event0_mask = *(unsigned int *)(base_addr + SPE_SIM_EVENT_MASK);
	return (event0_mask&input_event);
#else
	return 1;
#endif
}

unsigned int spe_sim_reg_get_entry_dump(spe_sim_ctx_t* ctx)
{

}

unsigned int spe_sim_reg_get_ncm_align_parametor(spe_sim_ctx_t* ctx)
{
	HI_SPE_PACK_ADDR_CTRL_T ncm_addr;
    ncm_addr.u32 = *(unsigned int*)(base_addr + SPE_PACK_ADDR_CTRL);

	ctx->ncm_div = ncm_addr.bits.spe_pack_divisor;
	ctx->ncm_rem = ncm_addr.bits.spe_pack_remainder;
	return 0;
}

unsigned int spe_sim_reg_get_ncm_max_size(spe_sim_ctx_t* ctx)
{
	HI_SPE_UNPACK_CTRL_T ncm_max_size;
    ncm_max_size.u32 = *(unsigned int*)(base_addr + SPE_UNPACK_CTRL);

	ctx->ncm_max_size = ncm_max_size.bits.spe_unpack_ctrl;

	return 0;
}

unsigned int spe_sim_reg_set_event_ptr(spe_sim_ctx_t* ctx)
{
	HI_SPE_EVENT_BUFF0_PTR_T evt0_ptr;
	unsigned int rptr;
	unsigned int wptr;

    evt0_ptr.u32 = *(unsigned int*)(base_addr + SPE_EVENT_BUFF0_PTR);
    spe_sim_intr_get_event_ptr(&rptr, &wptr);
    evt0_ptr.bits.spe_event_buff0_wptr = wptr;
    evt0_ptr.bits.spe_event_buff0_rptr = rptr;
	*(unsigned int*)(base_addr + SPE_EVENT_BUFF0_PTR) = evt0_ptr.u32;

	return 0;
}


static unsigned int spe_sim_reg_opt_entry_action(spe_sim_ctx_t* ctx, int port_num)
{
    HI_SPE_TAB_CTRL_T table_ctrl;
    spe_sim_entry_action_t action;
    spe_sim_entry_type_t entry_type;
    char* entry;

    table_ctrl.u32 = *(unsigned int*)(base_addr + SPE_SIM_ENTRY_OPRT);
    entry = (char*)(base_addr + SPE_SIM_ENTRY_TAB_CONTENT);

    /* set ncm param */
    switch(table_ctrl.bits.spe_tab_ctrl) {
    case spe_sim_add_mac_fwd:
        action = spe_sim_add_entry;
        entry_type = spe_sim_entry_mac_forward;
        break;
    case spe_sim_del_mac_fwd:
        action = spe_sim_del_entry;
        entry_type = spe_sim_entry_mac_forward;
        break;
    default:
        return -1;
    }

    spe_sim_schedule_entry(ctx, action, entry_type, entry);
    return 0;
}


void spe_sim_reg_set_entry_result(int result)
{
    unsigned int * tmp;
    HI_SPE_TAB_ACT_RESULT_T act_result;

    tmp = (unsigned int *)(base_addr + SPE_SIM_ENTRY_RESULT);
    act_result.u32 = *tmp;
    act_result.bits.spe_tab_success = !result;
    *tmp = act_result.u32;

    return;
}

/*
int spe_sim_reg_if_port_under_bridge(spe_sim_port_t *port)
{
    unsigned int* tmp;
    HI_SPE_CH_PROPERTY_0_T property;

    tmp = (unsigned int*)(base_addr + SPE_SIM_PORT_DEF(port->port_num));
    property.u32 = *tmp;

    if (spe_sim_port_in_br == property.bits.spe_ch_attach_brg) {
        return 1;
    }
    return 0;
}
*/

spe_sim_reg_func_table_t spe_sim_func_array[] = {

    /* function                                     offset */
    //0	
    {spe_sim_reg_reserved,                          0 },/* reserved,not in use */
    {spe_sim_reg_get_globle_ctl,                    SPE_SIM_GLB_CFG_REG & MASK_OFFSET},
    {spe_sim_reg_global_enable,                     SPE_SIM_GLB_ENABLE  & MASK_OFFSET},

    //3
    {spe_sim_reg_get_ncm_param,                     SPE_SIM_NCM_SP_CTR & MASK_OFFSET},
    {spe_sim_reg_get_hash_param,                    SPE_SIM_HASH_BASE_ADDR & MASK_OFFSET},
    {spe_sim_reg_get_hash_param,                    SPE_SIM_HASH_ZONE & MASK_OFFSET},
    {spe_sim_reg_get_hash_param,                    SPE_SIM_HASH_RAND & MASK_OFFSET},
    {spe_sim_reg_get_hash_param,                    SPE_SIM_HASH_SIZE & MASK_OFFSET},

    //8
    {spe_sim_reg_opt_entry_action,                  SPE_SIM_ENTRY_OPRT & MASK_OFFSET},

    //9
    {spe_sim_reg_get_intr_timeout,                  SPE_SIM_INT_MDRT_TMOUT & MASK_OFFSET},
    {spe_sim_reg_get_event_cnt,                     SPE_SIM_EVENT_PROCESSED & MASK_OFFSET},
    {spe_sim_reg_get_age_time,                      SPE_SIM_AG_TMOUT & MASK_OFFSET},
    {spe_sim_reg_get_hash_param,                    SPE_SIM_HASH_DEPTH & MASK_OFFSET},//CDE change 12 to hash depth
    {spe_sim_reg_set_time,                          SPE_SIM_REG_TIMER & MASK_OFFSET},


    //14
    {spe_sim_reg_get_property,                      SPE_SIM_PORT_DEF(0) & MASK_OFFSET},//HI_SPE_CH_PROPERTY_0_OFFSET
    {spe_sim_reg_get_port_num,                      SPE_SIM_PORT_NUM(0) & MASK_OFFSET},//HI_SPE_TDQ_CH_NUM_0_OFFSET
    {spe_sim_reg_get_port_mac,                      SPE_SIM_PORT_MAC_ADR_L(0) & MASK_OFFSET},//HI_SPE_CH_MAC_ADDR_H_0_OFFSET
    {spe_sim_reg_get_port_mac,                      SPE_SIM_PORT_MAC_ADR_H(0) & MASK_OFFSET},//HI_SPE_CH_MAC_ADDR_L_0_OFFSET

    //18
    {spe_sim_reg_get_td_addr,                       SPE_SIM_TD_ADDR(0) & MASK_OFFSET},
    {spe_sim_reg_get_td_size,                       SPE_SIM_TD_DEPT(0) & MASK_OFFSET},
    {spe_sim_reg_get_td_wptr,                       SPE_SIM_TD_WR_PTR(0) & MASK_OFFSET},
    {spe_sim_reg_get_pri,                           SPE_SIM_TD_PRI(0) & MASK_OFFSET},

    //22
    {spe_sim_reg_get_rd_addr,                       SPE_SIM_RD_ADDR(0) & MASK_OFFSET},
    {spe_sim_reg_get_rd_size,                       SPE_SIM_RD_DEPT(0) & MASK_OFFSET},
    {spe_sim_reg_get_rd_wptr,                       SPE_SIM_RD_WR_PTR(0) & MASK_OFFSET},

    //25
    {spe_sim_reg_get_ipf_mac,                      HI_SPE_SIM_GLB_SMAC_ADDR_L_OFFSET & MASK_OFFSET},//HI_SPE_GLB_SMAC_ADDR_L_OFFSET
    {spe_sim_reg_get_ipf_mac,                     HI_SPE_SIM_GLB_SMAC_ADDR_H_OFFSET & MASK_OFFSET},
    {spe_sim_reg_get_ipf_mac,                      HI_SPE_SIM_GLB_DMAC_ADDR_L_OFFSET & MASK_OFFSET},
    {spe_sim_reg_get_ipf_mac,                      HI_SPE_SIM_GLB_DMAC_ADDR_H_OFFSET & MASK_OFFSET},

    //29
	{spe_sim_reg_get_ncm_align_parametor,          HI_SPE_PACK_ADDR_CTRL_OFFSET & MASK_OFFSET},
	{spe_sim_reg_get_ncm_max_size,				   HI_SPE_UNPACK_CTRL_OFFSET & MASK_OFFSET},

	//31
	{spe_sim_reg_set_event_ptr,				       HI_SPE_EVENT_BUFF0_PTR_OFFSET & MASK_OFFSET}

};


void spe_reg_write(unsigned int offset, unsigned int value)
{
	unsigned int reg_attribute;
	unsigned int *reg_addr;
	unsigned int port_reg_interval;
	
//	SPE_SIM_INFO("%s: base addr 0x%x offset 0x%x value: 0x%x\n", __func__, base_addr, offset, value);	
	reg_attribute = offset>>SPE_SIM_REG_OFFSET_RIGHT_SHIFT;
	offset = offset&MASK_OFFSET;
	reg_addr = (unsigned int *)(base_addr + offset);
	*reg_addr = value;
//	SPE_SIM_INFO("reg: offset 0x%x attribute 0x%x\n", offset, reg_attribute);
	if (reg_attribute & SPE_SIM_REG_WRITE_EX) {
		reg_attribute &= SPE_SIM_REG_WR_MASK;
	if (reg_attribute == 0xa && value == 0) {
		return;
	}
	if((14<=reg_attribute)&&(reg_attribute<=17)&&(reg_attribute != 15)){
		//SPE_SIM_INFO("new_reg reg: reg_attribute 0x%x\n",__func__, reg_attribute);
		port_reg_interval = SPE_SIM_PORT_CTRL_OFFSET_SIZE;
	}else{
		port_reg_interval = SPE_SIM_TD_RD_OFFSET_SIZE;
	}


	spe_sim_func_array[reg_attribute].spe_sim_func_regs(the_sim_ctx,
			(offset - spe_sim_func_array[reg_attribute].offset) / port_reg_interval);
	if((14<=reg_attribute)&&(reg_attribute<=17)&&(reg_attribute != 15)){
		//SPE_SIM_INFO("new_reg reg:process func called, offset 0x%x head offset 0x%x port 0x%x\n", offset, 
		//	spe_sim_func_array[reg_attribute].offset, (offset - spe_sim_func_array[reg_attribute].offset) / port_reg_interval);
	}
    }

    return;
}
EXPORT_SYMBOL_GPL(spe_reg_write);

unsigned int spe_reg_read(unsigned int offset)
{
    unsigned int reg_attribute;
    unsigned int * read_addr;
    unsigned int port_reg_interval;

    reg_attribute = offset>>SPE_SIM_REG_OFFSET_RIGHT_SHIFT;
    if (reg_attribute & SPE_SIM_REG_READ_EX) {
        reg_attribute &= SPE_SIM_REG_WR_MASK;
        SPE_SIM_INFO("%s: reg_attribute 0x%x\n",__func__, reg_attribute);

        if((14<=reg_attribute)&(reg_attribute<=17)){
            port_reg_interval = SPE_SIM_PORT_CTRL_OFFSET_SIZE;
        }else{
		port_reg_interval = SPE_SIM_TD_RD_OFFSET_SIZE;
        }
        spe_sim_func_array[reg_attribute].spe_sim_func_regs(the_sim_ctx,
            (offset - spe_sim_func_array[reg_attribute].offset) / reg_attribute);
    }
    read_addr = (unsigned int *)(base_addr + (offset & MASK_OFFSET));
    return *read_addr;
}
EXPORT_SYMBOL_GPL(spe_reg_read);

void spe_sim_reg_set_td_rptr(spe_sim_port_t* port)
{
    //spe_sim_ctx_t* ctx = (spe_sim_ctx_t*)port->ctx;
    HI_SPE_TDQX_PTR_0_T td_ptr;
    unsigned int* tmp = (unsigned int*)(base_addr + SPE_SIM_TD_WR_PTR(port->port_num));

    td_ptr.u32 = *tmp;
    td_ptr.bits.spe_tdq_rptr = port->td_ctx.r_pos;
    *tmp = td_ptr.u32;
    return;
}

void spe_sim_reg_set_rd_rptr(spe_sim_port_t* port)
{
    //spe_sim_ctx_t* ctx = port->ctx;
    HI_SPE_RDQX_PTR_0_T rd_ptr;
    unsigned int* tmp = (unsigned int*)(base_addr + SPE_SIM_RD_WR_PTR(port->port_num));

    rd_ptr.u32 = *tmp;
    rd_ptr.bits.spe_rdq_rptr = port->rd_ctx.r_pos;
    *tmp = rd_ptr.u32;
    return;
}

void spe_sim_the_sim_ctx_show()
{
	SPE_SIM_ERR("the_sim_ctx.ports_en_num			%u\n", the_sim_ctx->ports_en_num);
	SPE_SIM_ERR("the_sim_ctx.reg_status			0x%x\n", (unsigned int)the_sim_ctx->reg_status);
	SPE_SIM_ERR("the_sim_ctx.wrap_timeout			%u\n", the_sim_ctx->wrap_timeout);
	//SPE_SIM_INFO("the_sim_ctx.cpu_port				0x%x\n", (unsigned int)the_sim_ctx->cpu_port);
	SPE_SIM_ERR("the_sim_ctx.is_ntb32				%u\n", the_sim_ctx->is_ntb32);
	SPE_SIM_ERR("the_sim_ctx.pack_fixed_in_len		%u\n", the_sim_ctx->pack_fixed_in_len);
	SPE_SIM_ERR("the_sim_ctx.pack_max_num			%u\n", the_sim_ctx->pack_max_num);
	SPE_SIM_ERR("the_sim_ctx.usr_field_num			%u\n", the_sim_ctx->usr_field_num);
	
	
	SPE_SIM_ERR("base_addr			0x%x\n", (unsigned int)base_addr);
}

MODULE_LICENSE("GPL");
