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
 * spe_sim_td.c -- spe simulator TD process
 *
 */

#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/list.h>

#include <linux/kernel.h>
#include <linux/list.h>

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>
#include <net/ip.h>
#include <linux/netfilter/nf_conntrack_tuple_common.h>


#include "spe_sim.h"
#include "spe_sim_priv.h"

#define IPV4_VERSION               (4)                                     /* IP头部中IP V4版本号 */
#define IPV6_VERSION               (6)                                     /* IP头部中IP V6版本号 */
#define ETH_HEADER_LEN          (14)
#define IP_HEAD_VERSION_OFFSET_MASK (0x0F)                                 /* IP头部协议版本偏移量掩码 */

typedef struct _IPHDR
{
    unsigned char    ucIpHdrLen:4;                 /* version */
    unsigned char    ucIpVer:4;
    unsigned char    ucServiceType;     /* type of service */
    unsigned short   usTotalLen;           /* total length */
    unsigned short   usIdentification;   /* identification */
    unsigned short   usOffset;               /* fragment offset field */
    unsigned char    ucTTL;                    /* time to live*/
    unsigned char    ucProtocol;            /* protocol */
    unsigned short   usCheckSum;        /* checksum */
    unsigned int   ulSrcAddr;
    unsigned int   ulDestAddr;          /* source and dest address */
}ETH_TEST_IPHDR_T;


#define SPE_SIM_TD_IS_EMPTY(w, r) ((w) == (r))
#define SPE_SIM_GET_CUR_TD(base, pos, usr_num) \
    ((tran_desc_t*)((base) + (pos) * (sizeof(tran_desc_t) + ((usr_num)*4))))


static inline int __spe_sim_udp_protect(spe_sim_port_t* port)
{
    return 0;
}

static inline int __spe_sim_rx_limit(spe_sim_port_t* port)
{
    return 0;
}



static int __spe_sim_add_ipf_mac(spe_sim_port_t* port, char* buf, unsigned int len)
{
    char *data = buf;
    char *ip_header = buf + ETH_HEADER_LEN;
    char ucIpType;
    struct ethhdr* eth = (struct ethhdr*)buf;
    ETH_TEST_IPHDR_T *ip_head = ip_header;
		
    spe_sim_ctx_t *ctx = container_of(port, spe_sim_ctx_t, ports[port->port_num]);
    memcpy(eth->h_dest, (void *)ctx->ipf_dst_mac, MAC_ADDR_LEN);
    memcpy(eth->h_source, (void *)ctx->ipf_src_mac, MAC_ADDR_LEN);
					
    if(IPV4_VERSION == ip_head->ucIpVer){
        eth->h_proto = htons(ETH_P_IP);
        SPE_SIM_TRACE("RNIC Port set eth head to ipv4. \n");
    }
    else if(IPV6_VERSION == ip_head->ucIpVer){
        eth->h_proto = htons(ETH_P_IPV6);
        SPE_SIM_TRACE("RNIC Port set eth head to ipv6. \n");
    }
    else{
        SPE_SIM_TRACE("RNIC Port set no eth head type. \n");
    }
    port->src_ptr = (char*)buf;
    port->src_len = (unsigned int)len;
#if 0
		printk("td:dump pkt\n");
		print_hex_dump(KERN_ERR, "process: ", DUMP_PREFIX_ADDRESS, \
			16, 1, buf, min(64, len), false);
		printk("***************\n");
		printk("rnic portmac isn \n");
		spe_sim_print_mac_addr(port->port_mac);
#endif	
    return 0;
}


/*
 * unwrap the ncm/rndis packet, to tmp buffer: port->src_ptr
 * return 1, need coninue
 * return 0, unpack to end
 */
static int __spe_sim_unwrap(spe_sim_port_t* port)
{
    spe_sim_ctx_t* ctx = (spe_sim_ctx_t*)port->ctx;
    tran_desc_t* cur;
    spe_sim_td_ctx_t* td_ctx = &port->td_ctx;
    int ret;
    char *virt_buff;

    if (unlikely(port->redo_td)) {
        ret = port->unwrap_result;
        port->redo_td = 0;
        SPE_SIM_TRACE("td:port->redo_td set return \n");
        return ret;
    }

    cur = SPE_SIM_GET_CUR_TD(td_ctx->base, td_ctx->r_pos, ctx->usr_field_num);
    virt_buff = spe_sim_phy_to_virt(cur->buf_addr_low, SPE_SIM_TD_ADDR_TYPE);

    SPE_SIM_INFO("td:%s usr field len is %u\n", __func__, ctx->usr_field_num);	
    SPE_SIM_INFO("td:%s current td addr is 0x%x\n", __func__, cur);
    SPE_SIM_INFO("td: %s phy addr is 0x%x \n", __func__, cur->buf_addr_low);
    SPE_SIM_INFO("td: %s virt is 0x%x \n", __func__, virt_buff);
	
    if (spe_sim_enc_ncm == port->enc_type) {
        SPE_SIM_TRACE("td:enter __spe_sim_unwrap ncm \n");
        ret = spe_sim_ncm_unwrap(port, (char*)virt_buff, cur->pkt_len);
        if(ret<0){
            port->debug_info.ncm_unwarp_fail_drop++;
        }
    }
    else if (spe_sim_enc_rndis == port->enc_type) {
        SPE_SIM_TRACE("td:enter __spe_sim_unwrap rndis \n");
        ret = spe_sim_rndis_unwrap(port, (char*)virt_buff, cur->pkt_len);
        if(ret<0){
            port->debug_info.rndis_unwarp_fail_drop++;
        }
    }
    else if (spe_sim_enc_ipf == port->enc_type) {
        SPE_SIM_TRACE("td:enter __spe_sim_unwrap ipf \n");
        /*if data from ipf port, add ether header*/
        ret = __spe_sim_add_ipf_mac(port, (char*)virt_buff, cur->pkt_len);
        if(ret<0){
            port->debug_info.rndis_unwarp_fail_drop++;
        }
    }
    else {
        port->src_ptr = (char*)virt_buff;
        port->src_len = (unsigned int)cur->pkt_len;
        ret = 0;
        SPE_SIM_TRACE("td:enter no unwarp proc \n");
    }

    if (cur->attribute.push_en) {
        port->last_packet= 1;
    }

    return ret;
}

void spe_sim_td_process(spe_sim_port_t* port)
{
    int r_pos;
    int w_pos;
    spe_sim_td_ctx_t* td_ctx;
    tran_desc_t* cur_td;
    char* virt_buff;
    unsigned int len;
    spe_sim_ctx_t* ctx = (spe_sim_ctx_t*)port->ctx;
    int do_next;


    SPE_SIM_INFO("td: %s src port num is %u \n",__func__,port->port_num);
    td_ctx = &port->td_ctx;
    if (__spe_sim_udp_protect(port))
        return;

    if (__spe_sim_rx_limit(port))
        return;
    SPE_SIM_TRACE("td:start set wptr rptr \n");
    r_pos = td_ctx->r_pos;
    w_pos = td_ctx->w_pos;
	
    SPE_SIM_TRACE("td:start set proc_stat \n");
    port->proc_stat = spe_sim_proc_td;
    port->debug_info.stat_td_proc++;

    do {
        if (w_pos == r_pos) {
            port->proc_stat = spe_sim_proc_idle;
            return;
        }
        cur_td = SPE_SIM_GET_CUR_TD(td_ctx->base, td_ctx->r_pos, ctx->usr_field_num);
        virt_buff = spe_sim_phy_to_virt(cur_td->buf_addr_low, SPE_SIM_TD_ADDR_TYPE);
        if(NULL == cur_td->buf_addr_low){
            SPE_SIM_ERR("td:spe_sim_invalid_cache \n");

        }
		
        /* invalid the cache before the entry process */
        SPE_SIM_TRACE("td:spe_sim_invalid_cache \n");
        spe_sim_invalid_cache((void*)virt_buff, cur_td->pkt_len);				
        if (spe_msg_level & SPE_SIM_MSG_DUMP) {
				
            len = cur_td->pkt_len;
            printk("td:dump pkt virt addr 0x%x len %u\n",virt_buff,len);
            print_hex_dump(KERN_ERR, "process: ", DUMP_PREFIX_ADDRESS, \
                16, 1, virt_buff, min(64, len), false);
            printk("***************\n");
		
            spe_sim_print_mac_addr(port->port_mac);
        }

        /* if the td's packet need continue unwrapping, do_next return true */
        do_next = __spe_sim_unwrap(port);
        port->unwrap_result = do_next;
        if(do_next < 0){
            spe_sim_td_complete(port, 0x2, NULL);
            port->debug_info.unwarp_fail_drop++;
            SPE_SIM_ERR("%s: Return EFAULT, uwarp error!\n", __func__);
            return;
        }
				
        spe_sim_entry_process(port, port->src_ptr, port->src_len);

        /* if there are error in following process,
         * exp. no rd to process, stop loop
         */
        if (spe_sim_proc_no_rd == port->last_err) {
            port->redo_td = 1;
            break;
        }
    }while(do_next);

}
void print_cur_td(tran_desc_t* cur)
{
	//printk("attribute is % \n", cur->attribute);
	char* virt_buff;
	virt_buff = spe_sim_phy_to_virt(cur->buf_addr_low, SPE_SIM_TD_ADDR_TYPE);

	SPE_SIM_INFO("td:td buffer addr is 0x%x \n", virt_buff);
	SPE_SIM_INFO("td:td buffer len is %u \n", cur->pkt_len);
/*	print_hex_dump(KERN_ERR, "sim_rd_complete:data", DUMP_PREFIX_ADDRESS, \
                       16, 1, virt_buff, cur->pkt_len, true);*/
}

void spe_sim_td_complete(spe_sim_port_t* port, int result, char* usr_field)
{
    spe_sim_ctx_t* ctx = (spe_sim_ctx_t*)port->ctx;
    spe_sim_td_ctx_t* td_ctx = &port->td_ctx;
    tran_desc_t* cur;
    int r_pos;
    int w_pos;
    int irq_en;

    port->debug_info.stat_td_complete++;
    r_pos = td_ctx->r_pos;
    w_pos = td_ctx->w_pos;
    SPE_SIM_TRACE("td:%s entry \n",__func__);
    cur = SPE_SIM_GET_CUR_TD(td_ctx->base, r_pos, ctx->usr_field_num);

    /* write result to td */
    cur->result = result;
    irq_en = cur->attribute.irq_enable;

    /* write td usr field to rd usr field */
    if (usr_field) {
        char* td_usr = (char*)cur;
        td_usr += sizeof(tran_desc_t);
        memcpy(usr_field, td_usr, ctx->usr_field_num);
    }

    /* move td read pos */
    r_pos = (r_pos + 1) % td_ctx->td_num;
    td_ctx->r_pos = r_pos;

    /* write r_pos to reg memory */
    spe_sim_reg_set_td_rptr(port);
    print_cur_td(cur);
	
    /* start td complete event */
    if (irq_en) {
        port->debug_info.stat_td_intr++;
        spe_sim_start_intr(port, SPE_TD_COMPLETE);
    }
}

void spe_sim_recycle_all_td(spe_sim_port_t* port)
{
    spe_sim_td_ctx_t* td_ctx = &port->td_ctx;

    printk("spe_sim_td_complete recycle, r:%d, w:%d\n", td_ctx->r_pos, td_ctx->w_pos);
    while(td_ctx->r_pos != td_ctx->w_pos) {
        printk("spe_sim_td_complete recycle, r:%d, w:%d\n", td_ctx->r_pos, td_ctx->w_pos);
        spe_sim_td_complete(port, -1, NULL);
    }
}

int spe_sim_can_process_port(spe_sim_port_t* port)
{
    if (port->td_ctx.r_pos == port->td_ctx.w_pos) {
		return 0;
    }
    return 1;
}

void spe_sim_td_init(spe_sim_ctx_t* ctx)
{
    return;
}

