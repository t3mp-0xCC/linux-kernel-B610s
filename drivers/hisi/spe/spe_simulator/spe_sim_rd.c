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
 * spe_sim_rd.c -- spe simulator RD process
 *
 */

#include "spe_sim.h"
#include "spe_sim_priv.h"
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include "spe_sim.h"
#include "spe_sim_priv.h"
#include <linux/unaligned/le_struct.h>
#include <linux/if_ether.h>

#include <linux/device.h>
//#include <linux/etherdevice.h>
#include <linux/crc32.h>
#include <linux/kthread.h>
#include <linux/usb/cdc.h>
#include <net/ip.h>
#include "spe_sim_reg.h"


#define SPE_SIM_GET_CUR_RD(base, pos, usr_num) \
    ((reveived_desc_t*)((base) + (pos) * (sizeof(reveived_desc_t) + ((usr_num)*4))))

void dump_rd_info(unsigned int port_num)
{
	spe_sim_port_t* dst = NULL;
	spe_sim_rd_ctx_t* rd_ctx;
	int i = 0;
	int rd_num;
	reveived_desc_t* cur;
	spe_sim_ctx_t* ctx;

	spe_sim_get_port_info(port_num, &dst);
	rd_num = dst->rd_ctx.rd_num;
       rd_ctx = &dst->rd_ctx;
	ctx = (spe_sim_ctx_t*)dst->ctx;
	
	for(i=0; i<rd_num; i++){
		cur = SPE_SIM_GET_CUR_RD(rd_ctx->base, i, ctx->usr_field_num);
		printk("buf_addr_low 0x%x, skb_addr 0x%x, pkt_len %d \n",cur->buf_addr_low, cur->skb_addr, cur->pkt_len);
	}
}


void spe_sim_proto_csum_replace4(__sum16 *sum,
			      __be32 from, __be32 to, int pseudohdr)
{
	__be32 diff[] = { ~from, to };

	*sum = csum_fold(csum_partial(diff, sizeof(diff),
			~csum_unfold(*sum)));

}

static inline void
spe_sim_src_ip_manip_pkt(struct iphdr* iphdr, spe_sim_ip_fwd_modify_t* modify)
{
    csum_replace4(&iphdr->check, iphdr->saddr, modify->ip);
    //iphdr->saddr = modify->ip;
}

static inline void
spe_sim_dst_ip_manip_pkt(struct iphdr* iphdr, spe_sim_ip_fwd_modify_t* modify)
{
    csum_replace4(&iphdr->check, iphdr->daddr, modify->ip);
    //iphdr->daddr = modify->ip;
}

static void
spe_sim_udp_manip_pkt(struct iphdr* iphdr, struct udphdr *udphdr,
	      spe_sim_ip_fwd_modify_t* modify)
{
	__be32 oldip, newip;
	__be16 *portptr, newport;

    if (!(modify->modify_mask & SPE_SIM_MODIFY_IP_PORT)) {
        return;
    }

	if (modify->modify_mask & SPE_SIM_MODIFY_SRC) {
        spe_sim_src_ip_manip_pkt(iphdr, modify);

		/* Get rid of src ip and src pt */
		oldip = iphdr->saddr;
		newip = modify->ip;
        iphdr->saddr = modify->ip;
		newport = modify->port;
		portptr = &udphdr->source;
	} else {
	    spe_sim_dst_ip_manip_pkt(iphdr, modify);

		/* Get rid of dst ip and dst pt */
		oldip = iphdr->daddr;
		newip = modify->ip;
        iphdr->daddr = modify->ip;
		newport = modify->port;
		portptr = &udphdr->dest;
	}

	spe_sim_proto_csum_replace4(&udphdr->check, oldip, newip, 1);
	spe_sim_proto_csum_replace4(&udphdr->check,
                                (__be32)*portptr, (__be32)newport, 0);

	if (!udphdr->check)
		udphdr->check = CSUM_MANGLED_0;

	*portptr = newport;
	return;
}

static void
spe_sim_tcp_manip_pkt(struct iphdr* iphdr, struct tcphdr *tcphdr,
	      spe_sim_ip_fwd_modify_t* modify)
{
	__be32 oldip, newip;
	__be16 *portptr, newport, oldport;
    unsigned int modify_src  = modify->modify_mask & SPE_SIM_MODIFY_SRC;

    if (!(modify->modify_mask & SPE_SIM_MODIFY_IP_PORT)) {
        return;
    }

	if (modify->modify_mask & SPE_SIM_MODIFY_SRC) {
        spe_sim_src_ip_manip_pkt(iphdr, modify);

		/* Get rid of src ip and src pt */
		oldip = iphdr->saddr;
		newip = modify->ip;
        iphdr->saddr = modify->ip;
		newport = modify->port;
		portptr = &tcphdr->source;
	} else {
	    spe_sim_dst_ip_manip_pkt(iphdr, modify);

		/* Get rid of dst ip and dst pt */
		oldip = iphdr->daddr;
		newip = modify->ip;
        iphdr->daddr = modify->ip;
		newport = modify->port;
		portptr = &tcphdr->dest;
	}

	oldport = *portptr;
	*portptr = newport;

	spe_sim_proto_csum_replace4(&tcphdr->check, oldip, newip, 1);
	spe_sim_proto_csum_replace4(&tcphdr->check,
                                (__be32)oldport, (__be32)newport, 0);
	return;
}

void print_cur_rd(reveived_desc_t* cur)
{
	//printk("attribute is % \n", cur->attribute);
	char *virt_buff;
	virt_buff = spe_sim_phy_to_virt(cur->buf_addr_low, SPE_SIM_RD_ADDR_TYPE);

	SPE_SIM_INFO("rd:rd buffer addr is 0x%x \n", virt_buff);
	SPE_SIM_INFO("rd:rd port is 0x%x \n", cur->dport_num);
	SPE_SIM_INFO("rd:rd buffer len is %u \n", cur->pkt_len);
/*	print_hex_dump(KERN_ERR, "sim_rd_complete:data", DUMP_PREFIX_ADDRESS, \
                       16, 1, virt_buff, cur->pkt_len, true);*/

}


/* static */
void __spe_sim_rd_complete(spe_sim_port_t* port, int result)
{
    spe_sim_ctx_t* ctx = (spe_sim_ctx_t*)port->ctx;
    spe_sim_rd_ctx_t* rd_ctx = &port->rd_ctx;
    struct received_descriptor* cur;
    spe_sim_port_t* src_port;
    char *virt_buff;
    unsigned int r_pos,w_pos;
    unsigned int irq_en;
    SPE_SIM_TRACE("rd:__spe_sim_rd_complete called \n");
    r_pos = rd_ctx->r_pos;
    w_pos = rd_ctx->w_pos;

    port->debug_info.stat_rd_complete++;
    cur = SPE_SIM_GET_CUR_RD(rd_ctx->base, rd_ctx->r_pos, ctx->usr_field_num);

    virt_buff = spe_sim_phy_to_virt(cur->buf_addr_low, SPE_SIM_RD_ADDR_TYPE);

    SPE_SIM_INFO("rd:spe_sim_rd_complete entering warp addr 0x%x len %u \n",virt_buff,cur->pkt_len);
    /* wrap end */
    if (spe_sim_enc_ncm == port->enc_type) {
        spe_sim_ncm_end_wrap(port);
    }
    else if (spe_sim_enc_rndis == port->enc_type) {
        spe_sim_rndis_end_wrap(port);
    }

    rd_ctx->wrapping = 0;

    /* write result to rd attr */
    cur->result = (unsigned int)result;

    if (0 == result) {
        cur->attribute.update_only = (unsigned int)rd_ctx->update_only;
        src_port = (spe_sim_port_t*)rd_ctx->src;
        cur->attribute.source_port_num = src_port->port_num;
        cur->pkt_len = port->dst_len;
    }
    else {
        cur->attribute.source_port_num = 0;
        cur->pkt_len = 0;
    }

    irq_en = cur->attribute.irq_enable;


    r_pos = (r_pos + 1) % rd_ctx->rd_num;
    rd_ctx->r_pos = r_pos;
    /* write r_pos to reg memory */
    spe_sim_reg_set_rd_rptr(port);

    /* flush the output buffer */
    spe_sim_flush_cache((void*)port->dst_ptr, port->dst_len);

    /*start rd complete event */
    if (irq_en){
        SPE_SIM_INFO("rd:start intr port: %u \n",port->port_num);
        port->debug_info.stat_rd_intr++;
        spe_sim_start_intr(port, SPE_RD_COMPLETE);
    }
}

void spe_sim_recycle_all_rd(spe_sim_port_t* port)
{
    spe_sim_rd_ctx_t* rd_ctx = &port->rd_ctx;

    while(rd_ctx->r_pos != rd_ctx->w_pos) {
        printk("spe_sim_rd_complete recycle, r:%d, w:%d\n",rd_ctx->r_pos, rd_ctx->w_pos);
        __spe_sim_rd_complete(port, -1);
    }
}


static void __spe_sim_wrap_start_timer(spe_sim_port_t* port)
{
#if 0
    spe_sim_rd_ctx_t* rd_ctx = &port->rd_ctx;

    if (0 == rd_ctx->wrapping) {

        /* start timer */
        add_timer(&rd_ctx->timer);

        /* mark the wrap flag */
        rd_ctx->wrapping = 1;
    }
#endif
}

static void __spe_sim_wrap_timeout_handle(unsigned long data)
{
    printk("__spe_sim_wrap_timeout_handle called \n");
    __spe_sim_rd_complete((spe_sim_port_t*)data, 0);
}

/* copy src packet to dst, according to modify info */
void spe_sim_do_nat(char* src, spe_sim_ip_fwd_modify_t* modify)
{
    unsigned int modify_option;
    unsigned int modify_dest_mac;
    unsigned int modify_src_mac;
    struct ethhdr* spe_sim_rd_eth_hdr;
    struct iphdr* spe_sim_rd_iphdr;
    char* l4hdr;

    spe_sim_rd_eth_hdr  = (struct ethhdr*)src;
    spe_sim_rd_iphdr    = (struct iphdr*)(src + ETH_HLEN);
    l4hdr = (char*)spe_sim_rd_iphdr + (spe_sim_rd_iphdr->ihl << 2);

    modify_option   = modify->modify_mask;
    modify_dest_mac = modify_option&SPE_SIM_MODIFY_DMAC;
    modify_src_mac  = modify_option&SPE_SIM_MODIFY_SMAC;


	SPE_SIM_TRACE("enter\n");

    if (modify_dest_mac) {
		SPE_SIM_TRACE("modify_dest_mac\n");
		memcpy(spe_sim_rd_eth_hdr->h_dest, &modify->dest, ETH_ALEN);
    }

    if (modify_src_mac) {
		SPE_SIM_TRACE("modify_src_mac\n");
        memcpy(spe_sim_rd_eth_hdr->h_source, &modify->source, ETH_ALEN);
    }
    if (modify->l4_proto == IPPROTO_TCP) {
        struct tcphdr* tcphdr = (struct tcphdr*)l4hdr;
        spe_sim_tcp_manip_pkt(spe_sim_rd_iphdr, tcphdr, modify);
    }
    else if (modify->l4_proto == IPPROTO_UDP) {
        struct udphdr* udphdr = (struct udphdr*)l4hdr;
        spe_sim_udp_manip_pkt(spe_sim_rd_iphdr, udphdr, modify);
    }
	SPE_SIM_TRACE("out\n");
}

int __spe_sim_wrap(spe_sim_port_t* src, spe_sim_port_t* dst,
                          reveived_desc_t* cur, spe_sim_ip_fwd_modify_t* modify)
{
    int ret;
    char *virt_buff;
    spe_sim_rd_ctx_t* rd_ctx = &dst->rd_ctx;
    virt_buff = spe_sim_phy_to_virt(cur->buf_addr_low, SPE_SIM_RD_ADDR_TYPE);
    SPE_SIM_TRACE("rd warp:rd __spe_sim_wrap entry \n");
    dst->dst_ptr = virt_buff;

	spe_sim_do_nat(src->src_ptr, modify);
	
    if (spe_sim_enc_ncm == dst->enc_type) {
		
        __spe_sim_wrap_start_timer(dst);
        ret = spe_sim_ncm_wrap(dst, src->src_ptr, src->src_len);
        src->last_packet = !ret;
		
    }
    else if (spe_sim_enc_rndis == dst->enc_type) {
        __spe_sim_wrap_start_timer(dst);
        ret = spe_sim_rndis_wrap(dst, src->src_ptr, src->src_len);
        src->last_packet = !ret;
    }
    else {

	    memcpy(dst->dst_ptr, src->src_ptr, src->src_len);

		dst->dst_len = src->src_len;
        /* single packet need assemble? */
        src->last_packet = 1;
        rd_ctx->wrapping = 0;
        ret = 0;
    }

    return ret;
}


void spe_sim_rd_process(spe_sim_port_t* src, spe_sim_port_t* dst,
                        spe_sim_ip_fwd_modify_t* modify, int update_only)
{
    spe_sim_rd_ctx_t* rd_ctx = &dst->rd_ctx;
    spe_sim_ctx_t* ctx = (spe_sim_ctx_t*)src->ctx;
    reveived_desc_t* cur;
    int ret;
    int next_r_pos = (rd_ctx->r_pos+1)%rd_ctx->rd_num;
    char *virt_buff;

    SPE_SIM_INFO("rd:rd src port : %u \n", src->port_num);
    SPE_SIM_INFO("rd:rd dst port : %u \n", dst->port_num);

    dst->debug_info.stat_rd_proc++;
    /* rd is empty */
    if (rd_ctx->r_pos == rd_ctx->w_pos) {
        /* refer to td proc, if no_rd errno set, redo_td will be true */
        src->last_err = spe_sim_proc_no_rd;
        dst->debug_info.stat_rd_empty++;
        return;
    }
    cur = SPE_SIM_GET_CUR_RD(rd_ctx->base, rd_ctx->r_pos, ctx->usr_field_num);
    if(NULL == cur->buf_addr_low){
		dump_rd_info(dst->port_num);
    }
    SPE_SIM_INFO("rd:spe_sim_rd_process rd wprt %d rd rptr %d\n",rd_ctx->w_pos,rd_ctx->r_pos);
	
    virt_buff = spe_sim_phy_to_virt(cur->buf_addr_low, SPE_SIM_RD_ADDR_TYPE);

    SPE_SIM_INFO("rd:spe_sim_rd_process rd addr 0x%x rd phy addr 0x%x len %u \n",virt_buff,cur->buf_addr_low,cur->pkt_len);

    rd_ctx->update_only = update_only;
    rd_ctx->src = src;

    /* wrap or copy src packet to dst */
    ret = __spe_sim_wrap(src, dst, cur, modify);

    /* src td unwrap complete? */
    if ((0 == src->unwrap_result)&&(0 == update_only)) {
        /* here, src packet copy complete */
        spe_sim_td_complete(src, 0, (char*)(cur) + sizeof(reveived_desc_t));
    }
	
    if (src->last_packet || !rd_ctx->wrapping) {

        if (dst->enc_type == spe_sim_enc_ncm ||
            dst->enc_type == spe_sim_enc_rndis) {
            /* del the timer */
#if 0
            del_timer(&rd_ctx->timer);
#endif
        }
        /* complete the rd descriptor */
        __spe_sim_rd_complete(dst, 0);
    }

    src->last_err = 0;
}


void spe_sim_rd_init(spe_sim_ctx_t* ctx)
{
    int idx;
    spe_sim_port_t* port;

    for (idx = 0; idx < SPE_SIM_MAX_PORTS; idx++) {
        port = &ctx->ports[idx];
        port->rd_ctx.timer.function = __spe_sim_wrap_timeout_handle;
        port->rd_ctx.timer.data = (unsigned long)port;
        port->rd_ctx.timer.expires = SPE_SIM_DFT_WARP_TIMEOUT;
        init_timer(&port->rd_ctx.timer);
    }
}

