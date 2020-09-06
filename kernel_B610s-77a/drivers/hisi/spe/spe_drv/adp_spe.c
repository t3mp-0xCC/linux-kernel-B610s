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

#include <linux/spe/spe_interface.h>
#include <linux/usb/bsp_usb.h>

#include "mdrv_spe_wport.h"
#include "spe_dbg.h"
//#include "spe_entry.h"

#define SPE_WPORT_TD_NUM                (1024)
#define SPE_WPORT_RD_NUM                (512)
#define SPE_WPORT_RD_SKB_SIZE           (1800)
#define SPE_WPORT_JUST_ONE_MAIN()       (1)
#define SPE_WPORT_CACHE_HDR_SIZE        (sizeof(struct ethhdr))


typedef struct spe_wport_id {
    int is_open;
    spe_wport_attr_t attr;
    spe_wport_rx_cb_t rx_cb;

    /* statistics for wport */
    int stat_xmit_ipv4;
    int stat_xmit_ipv6;
    int stat_xmit_err_type;
    int stat_xmit_td_err;
    int stat_open_cnt;
    int stat_close_cnt;
    int stat_config_td;
    int stat_finish_td;
    int stat_config_rd;
    int stat_config_rd_err;
    int stat_finish_rd_norm;
    int stat_finish_rd_bypass;
    int stat_rx_cb_null;
    int stat_rx_cb_norm;
    int stat_rx_cb_bypass;
}spe_wport_id_t;

/* we can support SPE_PORT_NUM ports for wan spe port adp */
typedef struct tag_spe_wport_ctx {
    int bypass_id;
    int main_id;                            /* opened port_no */
    struct net_device * dummy_net_dev;      /* used for stick mode*/
    spe_wport_id_t ports[SPE_PORT_NUM];
    spe_wport_recycle_cb_t recycle;
    /* statistics for ctx */
    struct ethhdr eth_for_ipf;
    int stat_skb_kfree_direct;
    int stat_rx_port_err;
    int stat_port_alloc_err;
    int stat_port_enable_err;
    int stat_net_dev_err;
    int stat_net_modem_diff_err;
}spe_wport_ctx_t;

static spe_wport_ctx_t spe_wport_ctx;
static unsigned int spe_wport_dft_td_num = SPE_WPORT_TD_NUM;
static unsigned int spe_wport_dft_rd_num = SPE_WPORT_RD_NUM;
static unsigned int spe_wport_dft_skb_sz = SPE_WPORT_RD_SKB_SIZE;

static int spe_wport_finish_rd(int portno, int src_portno,
    struct sk_buff *skb, dma_addr_t dma, unsigned int flags)
{
    spe_wport_id_t* port_ptr = &spe_wport_ctx.ports[portno];
    spe_wport_rx_cb_t rx_cb = port_ptr->rx_cb;

    unsigned int len = (skb->len < SPE_WPORT_CACHE_HDR_SIZE) ? skb->len : SPE_WPORT_CACHE_HDR_SIZE;
    dma_unmap_single(NULL,skb->dma, len,DMA_FROM_DEVICE);

    if (rx_cb) {
        port_ptr->stat_finish_rd_norm++;
        rx_cb(portno, skb);
    } else {
		mdrv_spe_wport_recycle(skb);
    	port_ptr->stat_rx_cb_null++;
	}
    port_ptr->stat_rx_cb_norm++;
    return 0;
}

void mdrv_spe_wport_recycle(struct sk_buff *skb)
{
    dma_addr_t dma;
    unsigned int len = 0;

    if (skb == NULL){
        return;
    }

    if (likely(skb->spe_own)) {
        /*ads td done*/
        if (skb->spe_own > SPE_PORT_NUM) {
            if (spe_wport_ctx.recycle) {
                if (spe_wport_ctx.recycle(skb)) {
                    goto skb_kfree_direct;
                }
                return;
            }
        }
        /* check whether spe_own is correct port id */
        if (unlikely(SPE_WPORT_JUST_ONE_MAIN()
            && spe_wport_ctx.main_id != skb->spe_own)) {
            SPE_ERR("may be err port id: <%d>-<%d>\n",
                spe_wport_ctx.main_id, skb->spe_own);
            spe_wport_ctx.stat_rx_port_err++;
            goto skb_kfree_direct;
        } else {
            /* spe_own is port_no*/
            unsigned int port_no = skb->spe_own;
            if(port_no >= SPE_PORT_NUM){
                goto skb_kfree_direct;
            }
            
            dma = spe_get_skb_dma(skb);
            skb->data = phys_to_virt(dma);

            len = (skb->len < SPE_WPORT_CACHE_HDR_SIZE) ? skb->len : SPE_WPORT_CACHE_HDR_SIZE;
            dma_map_single(NULL,skb->data,len,DMA_FROM_DEVICE);

            if (spe_rd_config(port_no, skb, dma)) {
                spe_wport_ctx.ports[port_no].stat_config_rd_err++;
                goto skb_kfree_direct;
            }
            spe_wport_ctx.ports[port_no].stat_config_rd++;
        }
    } else {
		goto skb_kfree_direct;
	}
    return;

skb_kfree_direct:
    dev_kfree_skb_any(skb);
    spe_wport_ctx.stat_skb_kfree_direct++;
    return;
}

/* only bypass mode need this function */
static int spe_wport_finish_rd_bypass(int portno, int src_portno,
    struct sk_buff *skb)
{
    int bypass_id = spe_wport_ctx.bypass_id;
    spe_wport_rx_cb_t rx_cb = spe_wport_ctx.ports[bypass_id].rx_cb;

    /* ndis need wan port handle, so we use bypass_id */
    if (rx_cb) {
        spe_wport_ctx.ports[bypass_id].stat_rx_cb_bypass++;
        rx_cb(bypass_id, skb);
    } else {
	 	mdrv_spe_wport_recycle(skb);
    	spe_wport_ctx.ports[bypass_id].stat_rx_cb_null++;
	}
    spe_wport_ctx.ports[bypass_id].stat_finish_rd_bypass++;
    return 0;
}

static int spe_wport_finish_td(int portno, struct sk_buff *skb,
    unsigned int flags)
{
    dma_addr_t dma;

    if (skb->spe_own > SPE_PORT_NUM) {
        if (spe_wport_ctx.recycle) {
            if (spe_wport_ctx.recycle(skb)) {
                dev_kfree_skb_any(skb);
            }
        }
    } else {
        dma = spe_get_skb_dma(skb);
        dma_unmap_single(spe_balong.dev, dma, skb->len, DMA_TO_DEVICE);
        dev_kfree_skb_any(skb);
    }
    spe_wport_ctx.ports[portno].stat_finish_td++;
    return 0;
}

static void spe_wport_set_ipf_mac(struct ethhdr* add_hdr)
{
    memcpy(&spe_wport_ctx.eth_for_ipf, add_hdr, sizeof(struct ethhdr));
    spe_set_ipf_mac(spe_mac_dst, (struct ether_addr *)add_hdr->h_dest);
    spe_set_ipf_mac(spe_mac_src, (struct ether_addr *)add_hdr->h_source);
}

static void spe_wport_set_rx_cb(int port, spe_wport_rx_cb_t rx_cb)
{
    spe_wport_ctx.ports[port].rx_cb = rx_cb;
}

static void spe_wport_set_push_cb(int port, spe_wport_push_cb_t push_cb)
{
    spe_balong.ports[port].ctrl.wport_push = push_cb;
}

static int spe_wport_diag_upload(struct spe_flow_stat_s* sfp)
{
	struct spe *spe = &spe_balong;
    	union spe_desc_queue_ptr rd_ptr;
    	union spe_desc_queue_ptr td_ptr;
	int portno, i,j;
	unsigned int *spe_tab;

	if(!sfp)
        return -EINVAL;

	for(portno=0;portno<SPE_PORT_NUM;portno++)
	{
		sfp->port_enable[portno] = 
        		(spe->ports[portno].ctrl.flags & SPE_PORT_ENABLED);

        if (sfp->port_enable[portno]) {
			td_ptr.raw = spe_readl(spe->regs, SPE_TDQ_PTR_OFFSET(portno));
			rd_ptr.raw = spe_readl(spe->regs, SPE_RDQ_PTR_OFFSET(portno));
            
	    	sfp->td_ptr_a[portno] = td_ptr.raw;
	    	sfp->rd_ptr_a[portno] = rd_ptr.raw;
	    	sfp->td_ful_h[portno] =spe->stat.evt_td_full[portno];
	    	sfp->td_empty_h[portno] =spe->stat.evt_td_empty[portno];
	    	sfp->rd_ful_h[portno] =spe->stat.evt_rd_full[portno];
	    	sfp->rd_empty_h[portno] =spe->stat.evt_rd_empty[portno];
	    	sfp->td_ful_s[portno] =spe->ports[portno].stat.td_full;
	    	sfp->rd_ful_s[portno] =spe->ports[portno].stat.rd_full;

	        sfp->td_input_bps[portno] = (spe->ports[portno].stat.td_config_bytes << 3); /*B/s -> b/s*/
			spe->ports[portno].stat.td_config_bytes = 0;

			sfp->rd_output_bps[portno] = (spe->ports[portno].stat.rd_finished_bytes << 3); /*B/s -> b/s*/
	        spe->ports[portno].stat.rd_finished_bytes = 0;

	        sfp->td_using_num[portno] = (td_ptr.bits.wptr >= td_ptr.bits.rptr) ?
	        	(td_ptr.bits.wptr - td_ptr.bits.rptr) : 
	            (spe->ports[portno].ctrl.td_depth -td_ptr.bits.rptr + td_ptr.bits.wptr);

			sfp->rd_using_num[portno] = (rd_ptr.bits.wptr >= rd_ptr.bits.rptr) ?
	            (rd_ptr.bits.wptr - rd_ptr.bits.rptr) : 
	            (spe->ports[portno].ctrl.rd_depth -rd_ptr.bits.rptr + rd_ptr.bits.wptr);

			for(i = 0; i<SPE_PORT_NUM; i++){
				memcpy(&sfp->desc_result[i], &spe->ports[portno].stat.rd_sport[0], sizeof(struct desc_result_s));
			}
    	}
	}

	sfp->ipfw_head = spe->ipfw.hbucket;
	sfp->ipfw_lock = &spe->ipfw.lock;
	spe_dbgen_enable();
	
	for (i = 0; i < SPE_MAC_TAB_NUMBER; i++){
		spe_tab = (unsigned int*)&sfp->mcfw[i];
		for (j = 0; j<SPE_MAC_TAB_WORD_NO; j++){
			spe_tab[j]=spe_readl(spe->regs, SPE_MAC_TAB_OFFSET(i*SPE_MAC_TAB_WORD_NO+j));
		}	
	}
	
	spe_dbgen_disable();

	return 0;
}

struct spe_flow_stat_s sfp_s;

void spe_macfw_entry_dump0(void)
{
	int* spe_tab;
	struct spe_mac_entry_in_sram* mac_entry;
	int i;

	for (i = 0; i < SPE_MACFW_DEPTH; i++){
		spe_tab = (int*)&sfp_s.mcfw[i];
		if (spe_tab[3] & IS_MACFW_ENTRY){
			mac_entry = (struct spe_mac_entry_in_sram*)spe_tab;
		}else{
			continue;
		}	
		printk("mac entry (%d) : %pM	bid:%d	vid:%d	port:%d	timestamp:0x%x	static:%d\n",
			i, mac_entry->mac,	mac_entry->bid & 0xf, mac_entry->vid  & 0xfff 
			, mac_entry->port  & 0xf , mac_entry->timestamp  & 0xffff, mac_entry->is_static  & 0x1);
	}

}

static inline void iptable_dump_entry0(int idx, struct spe_ip_fw_entry *pos)
{
    struct nf_conntrack_tuple *tuple = &pos->tuple;
	
    if (AF_INET6 == tuple->src.l3num) {
        printk(KERN_EMERG"l3num(%d) protonum(%d) %pI6:(%u) --> \n %pI6:(%u)\n",\
		tuple->src.l3num, tuple->dst.protonum, \
		&(tuple->src.u3.ip6), ntohs(tuple->src.u.all), \
		&(tuple->dst.u3.ip6), ntohs(tuple->dst.u.all));
    }
    else {
        printk(KERN_EMERG"l3num(%d) protonum(%d) %pI4:(%u) --> %pI4:(%u)\n",\
		tuple->src.l3num, tuple->dst.protonum, \
		&(tuple->src.u3.ip), ntohs(tuple->src.u.all), \
		&(tuple->dst.u3.ip), ntohs(tuple->dst.u.all));
    }
    printk(KERN_EMERG"entry[%d]: valid(%d)  vir(0x%p) dma: (0x%x) nextdma: (0x%x)\n",
        idx ,pos->valid ,pos, pos->dma, (unsigned int)pos->next);
    printk(KERN_EMERG"entry->portno %d\n", pos->portno);
    printk(KERN_EMERG"entry->action %d\n", pos->action);
    printk(KERN_EMERG"entry->vid %d\n", ntohs(pos->vid));
    printk(KERN_EMERG"entry->shost %pM\n", pos->shost.octet);
    printk(KERN_EMERG"entry->dhost %pM\n", pos->dhost.octet);
    printk(KERN_EMERG"manip_ip: %pI4 manip_port %u\n",\
        &(pos->nat.ip), ntohs(pos->nat.port));

}

int spe_ip_fw_show_test(void)
{
//    struct spe *spe = &spe_balong;
    struct spe_ip_fw_entry *head = &sfp_s.ipfw_head[0];
    struct spe_ip_fw_entry *pos = NULL;
    unsigned long flags = 0;
	int i = 1;
    int k = 0;
	
    printk(KERN_EMERG"dump spe_ip_fw_add retrun count end\n\n");

    for (k = 0; k<128; k++)
    {
        head = &sfp_s.ipfw_head[k];
        i = 1;

        spin_lock_irqsave(sfp_s.ipfw_lock, flags);

        /* check if ip_fw exist */
        if (head->valid || NULL != head->next) {
            printk(KERN_EMERG"\nhbucket[%d], head->valid:%d, next:%p\n\n",
                k, head->valid, head->next);

        	/* if head is valid dump the tuple */
        	if (head->valid) {
        	    iptable_dump_entry0(0, head);
        	}

            list_for_each_entry(pos, &head->list, list){
                iptable_dump_entry0(i, pos);
        		i++;
            }
        }

        spin_unlock_irqrestore(sfp_s.ipfw_lock, flags);
    }

	return 0;

}

void sfp_test(int portno)
{
	spe_wport_diag_upload(&sfp_s);

	spe_ip_fw_show_test();

	spe_macfw_entry_dump0();	
}

int mdrv_spe_wport_open(spe_wport_attr_t *attr_set)
{
    int ret = 0;
    int port_num;
    struct spe_port_attr  attr;
    struct spe_port_ops bypass_ops = {0};

    if (unlikely(SPE_WPORT_JUST_ONE_MAIN() &&
        spe_wport_ctx.ports[spe_wport_ctx.main_id].is_open)) {
        SPE_ERR("port <%d> is already opended\n", spe_wport_ctx.main_id);
        return -EINVAL;;
    }

    if(bsp_usb_netmode() != attr_set->is_bypass_mode){
        spe_wport_ctx.stat_net_modem_diff_err++;
        SPE_ERR("proto drv set stick mode diff\n");
    }

    memset(&attr, 0, sizeof (struct spe_port_attr));

    attr.desc_ops.finish_rd =  spe_wport_finish_rd;
    attr.desc_ops.finish_td =  spe_wport_finish_td;
	
    if(attr_set->rd_depth)
    {
        attr.rd_depth = attr_set->rd_depth>spe_wport_dft_rd_num?spe_wport_dft_rd_num:attr_set->rd_depth;
    }
    else
    {
        attr.rd_depth = spe_wport_dft_rd_num;
    }
	
    if(attr_set->td_depth)
    {
        attr.td_depth = attr_set->td_depth>spe_wport_dft_td_num?spe_wport_dft_td_num:attr_set->td_depth;
    }
    else
    {
        attr.td_depth = spe_wport_dft_td_num;
    }
	
    attr.rd_skb_size = spe_wport_dft_skb_sz;
    attr.rd_skb_num = attr.rd_depth;

    if(attr_set->is_bypass_mode) {
        attr.stick_mode = 1;
        attr.net = (struct net_device*)
                       kzalloc(sizeof(struct net_device), GFP_KERNEL);
        spe_wport_ctx.dummy_net_dev = attr.net;
    }
    else{
        attr.net = attr_set->net_dev;
    }

    if(attr_set->is_ipf_port){
        attr.enc_type = spe_enc_ipf;
    }
    else{
        attr.enc_type = spe_enc_none;
    }

    if(NULL == attr.net){
        SPE_ERR("attr.net is error\n");
        spe_wport_ctx.stat_net_dev_err++;
		return -EINVAL;
	}

    attr.attach_brg = spe_attach_brg_normal;

    

    port_num = spe_port_alloc(&attr);
    if(port_num <= 0 || port_num >= SPE_PORT_NUM){
        SPE_ERR("spe wan port alloc failed: %d\n", port_num);
        spe_wport_ctx.stat_port_alloc_err++;
        goto wport_alloc_err;
    }


    if (SPE_WPORT_JUST_ONE_MAIN()) {
        spe_wport_ctx.main_id = port_num;
    }


    if (attr_set->is_bypass_mode) {
	    bypass_ops.finish_rd_bypass = spe_wport_finish_rd_bypass;

	    ret = spe_set_bypass_ops(&bypass_ops);
	    if (ret) {
	        SPE_ERR("spe wan port set ops failed: %d, ret:%d\n", port_num, ret);
	        goto wport_alloc_err;
	    }

        spe_wport_ctx.bypass_id = port_num;
    }
    else {
        spe_wport_ctx.bypass_id = 0;
    }
    memcpy(&spe_wport_ctx.ports[port_num].attr,
        attr_set, sizeof(spe_wport_attr_t));

    spe_wport_ctx.ports[port_num].is_open = 1;
    spe_wport_ctx.ports[port_num].stat_open_cnt++;

    ret = spe_port_enable(port_num);/*check ret*/
    if (ret) {
        SPE_ERR("spe wan port enable failed: %d, ret:%d\n", port_num, ret);
        spe_wport_ctx.stat_port_enable_err++;
        goto wport_enable_err;
    }

    return port_num;



wport_enable_err:
    spe_port_free(port_num);

wport_alloc_err:
    if (spe_wport_ctx.dummy_net_dev) {
        kfree((void*)spe_wport_ctx.dummy_net_dev);
        spe_wport_ctx.dummy_net_dev = NULL;
    }

    return (-1);
}
int mdrv_spe_get_bypass_id(void)
{
   return spe_wport_ctx.bypass_id;
}
void mdrv_spe_wport_close(int port)
{
    spe_wport_id_t *port_ptr;

    if (unlikely(port < 0 || port >= SPE_PORT_NUM)) {
        SPE_ERR("incorrect port:%d\n", port);
        return;
    }
    if (unlikely(SPE_WPORT_JUST_ONE_MAIN() &&
        spe_wport_ctx.main_id != port)) {
        SPE_ERR("port may err:<%d>-<%d>\n", spe_wport_ctx.main_id, port);
        return;
    }

    /* here, port is correct */
    port_ptr = &spe_wport_ctx.ports[port];
    if (!port_ptr->is_open) {
        SPE_ERR("port:%d is closed already\n", port);
        return;
    }
    (void)spe_port_disable(port);
	(void)spe_port_free(port);
	spe_wport_ctx.ports[port].is_open = 0;
	port_ptr->stat_close_cnt++;
	return;
}

int mdrv_spe_wport_xmit(int port, struct sk_buff *skb)
{
    dma_addr_t dma;
    int ret = 0;
    spe_l3_type_t l3_type = spe_l3_bottom;
    spe_wport_id_t *port_ptr;

    if (unlikely(port < 0 || port >= SPE_PORT_NUM)) {
        SPE_ERR("incorrect port:%d\n", port);
        return -EINVAL;
    }
    if (unlikely(SPE_WPORT_JUST_ONE_MAIN() &&
        spe_wport_ctx.main_id != port)) {
        SPE_ERR("port <%d> may be not correct\n", port);
        return -EINVAL;
    }

    port_ptr = &spe_wport_ctx.ports[port];
    if (!port_ptr->is_open) {
        SPE_ERR("port:%d is not open\n", port);
        return -EINVAL;
    }

    if ((skb)->protocol == htons(ETH_P_IP)) {
        l3_type = spe_l3_ipv4;
        port_ptr->stat_xmit_ipv4++;
    }
    else if ((skb)->protocol == htons(ETH_P_IPV6)) {
        l3_type = spe_l3_ipv6;
        port_ptr->stat_xmit_ipv6++;
    }
    else {
        SPE_ERR("port <%d> l3_type:%d error\n", port, (skb)->protocol);
        port_ptr->stat_xmit_err_type++;
        return -EINVAL;
    }

    if (skb->spe_own > SPE_PORT_NUM) {
        dma = spe_get_skb_dma(skb);
    } else {
        dma = dma_map_single(spe_balong.dev, skb->data, skb->len, DMA_TO_DEVICE);
        spe_set_skb_dma(skb, dma);
    }

	ret = spe_td_config(port, skb, dma, l3_type, skb->psh);
	if (ret) {
	    port_ptr->stat_xmit_td_err++;
	}
	port_ptr->stat_config_td++;
	return ret;
}

int mdrv_spe_wport_ioctl(int port, unsigned int cmd, void* param)
{
    int ret = 0;

    if (unlikely(port < 0 || port >= SPE_PORT_NUM
        || !spe_wport_ctx.ports[port].is_open)) {
        SPE_ERR("incorrect port:%d\n", port);
        return -EINVAL;
    }
    if (unlikely(SPE_WPORT_JUST_ONE_MAIN() &&
        spe_wport_ctx.main_id != port)) {
        SPE_ERR("port <%d> may be not correct\n", port);
        return -EINVAL;
    }

    switch(cmd) {
    case SPE_WPORT_IOCTL_SET_MAC:
        spe_wport_set_ipf_mac((struct ethhdr*)param);
        break;
    case SPE_WPORT_IOCTL_SET_RX_CB:
        spe_wport_set_rx_cb(port, (spe_wport_rx_cb_t)param);
        break;
    case SPE_WPORT_IOCTL_SET_PUSH_CB:
        spe_wport_set_push_cb(port, (spe_wport_push_cb_t)param);
        break;
    case SPE_WPORT_IOCTL_SET_RECYCLE_CB:
        spe_wport_ctx.recycle = (spe_wport_recycle_cb_t)param;
        break;
    case SPE_WPORT_IOCTL_DIAG_UPLOAD:
        spe_wport_diag_upload((struct spe_flow_stat_s*) param);
        break;
    default:
        return -ENOENT;
    }
    return ret;
}

int mdrv_spe_usb_eth_is_bypass(int eth_id)
{
    return bsp_usb_netmode();
}

int spe_wport_adp_config_param(unsigned int td_num,
                               unsigned int rd_num, unsigned int skb_sz)
{
    spe_wport_dft_td_num = td_num;
    spe_wport_dft_rd_num = rd_num;
    spe_wport_dft_skb_sz = skb_sz;
    return 0;
}

int spe_wport_adp_dump(void)
{
    int portno;
    spe_wport_id_t* port_ptr;

    printk(KERN_EMERG"====== spe wport adp dump ======\n");
    printk(KERN_EMERG"spe_wport_dft_td_num:     %d\n", spe_wport_dft_td_num);
    printk(KERN_EMERG"spe_wport_dft_rd_num:     %d\n", spe_wport_dft_rd_num);
    printk(KERN_EMERG"spe_wport_dft_skb_sz:     %d\n", spe_wport_dft_skb_sz);
    printk(KERN_EMERG"bypass_id:                %d\n", spe_wport_ctx.bypass_id);
    printk(KERN_EMERG"main_id:                  %d\n", spe_wport_ctx.main_id);
    printk(KERN_EMERG"dummy_net_dev:            %p\n", spe_wport_ctx.dummy_net_dev);
    printk(KERN_EMERG"source ipf mac:           %02x:%02x:%02x:%02x:%02x:%02x\n",\
        spe_wport_ctx.eth_for_ipf.h_source[0], spe_wport_ctx.eth_for_ipf.h_source[1],\
        spe_wport_ctx.eth_for_ipf.h_source[2], spe_wport_ctx.eth_for_ipf.h_source[3],\
        spe_wport_ctx.eth_for_ipf.h_source[4], spe_wport_ctx.eth_for_ipf.h_source[5]);
    printk(KERN_EMERG"dest ipf mac:             %02x:%02x:%02x:%02x:%02x:%02x\n",\
        spe_wport_ctx.eth_for_ipf.h_dest[0], spe_wport_ctx.eth_for_ipf.h_dest[1],\
        spe_wport_ctx.eth_for_ipf.h_dest[2], spe_wport_ctx.eth_for_ipf.h_dest[3],\
        spe_wport_ctx.eth_for_ipf.h_dest[4], spe_wport_ctx.eth_for_ipf.h_dest[5]);
    printk(KERN_EMERG"stat_skb_kfree_direct:    %d\n", spe_wport_ctx.stat_skb_kfree_direct);
    printk(KERN_EMERG"stat_rx_port_err:         %d\n", spe_wport_ctx.stat_rx_port_err);
    printk(KERN_EMERG"stat_port_alloc_err:      %d\n", spe_wport_ctx.stat_port_alloc_err);
    printk(KERN_EMERG"stat_port_enable_err:     %d\n", spe_wport_ctx.stat_port_enable_err);
    printk(KERN_EMERG"stat_port_alloc_err:      %d\n", spe_wport_ctx.stat_port_alloc_err);
    printk(KERN_EMERG"stat_net_dev_err:         %d\n", spe_wport_ctx.stat_net_dev_err);
    printk(KERN_EMERG"stat_net_modem_diff_err:  %d\n", spe_wport_ctx.stat_net_modem_diff_err);

    for (portno = 0; portno < SPE_PORT_NUM; portno++) {
        port_ptr = &spe_wport_ctx.ports[portno];
        if (port_ptr->stat_open_cnt) {
            printk(KERN_EMERG"==========  dump port[%d] ========\n", portno);
            printk(KERN_EMERG"is_open:                  %d\n", port_ptr->is_open);
            printk(KERN_EMERG"stat_open_cnt:            %d\n", port_ptr->stat_open_cnt);
            printk(KERN_EMERG"stat_close_cnt:           %d\n", port_ptr->stat_close_cnt);
            printk(KERN_EMERG"attr.is_bypass_mode:      %d\n", port_ptr->attr.is_bypass_mode);
            printk(KERN_EMERG"attr.is_ipf_port:         %d\n", port_ptr->attr.is_ipf_port);
            printk(KERN_EMERG"attr.net_dev:             %p\n", port_ptr->attr.net_dev);
            printk(KERN_EMERG"rx_cb:                    %pS\n", port_ptr->rx_cb);
            printk(KERN_EMERG"stat_xmit_ipv4:           %d\n", port_ptr->stat_xmit_ipv4);
            printk(KERN_EMERG"stat_xmit_ipv6:           %d\n", port_ptr->stat_xmit_ipv6);
            printk(KERN_EMERG"stat_xmit_err_type:       %d\n", port_ptr->stat_xmit_err_type);
            printk(KERN_EMERG"stat_xmit_td_err:         %d\n", port_ptr->stat_xmit_td_err);
            printk(KERN_EMERG"stat_config_td:           %d\n", port_ptr->stat_config_td);
            printk(KERN_EMERG"stat_finish_td:           %d\n", port_ptr->stat_finish_td);
            printk(KERN_EMERG"stat_config_rd:           %d\n", port_ptr->stat_config_rd);
            printk(KERN_EMERG"stat_config_rd_err:       %d\n", port_ptr->stat_config_rd_err);
            printk(KERN_EMERG"stat_finish_rd_norm:      %d\n", port_ptr->stat_finish_rd_norm);
            printk(KERN_EMERG"stat_finish_rd_bypass:    %d\n", port_ptr->stat_finish_rd_bypass);
            printk(KERN_EMERG"stat_rx_cb_null:          %d\n", port_ptr->stat_rx_cb_null);
            printk(KERN_EMERG"stat_rx_cb_norm:          %d\n", port_ptr->stat_rx_cb_norm);
            printk(KERN_EMERG"stat_rx_cb_bypass:        %d\n", port_ptr->stat_rx_cb_bypass);
        }
    }
    return 0;
}

void mdrv_spe_get_spe_balong(struct spe *spe_copy)
{
    if (NULL == spe_copy)
    {
        return;
    }
    
    memcpy((void *)spe_copy, (void *)&spe_balong, sizeof(spe_balong));
}
EXPORT_SYMBOL(mdrv_spe_wport_ioctl);
EXPORT_SYMBOL(mdrv_spe_wport_recycle);
EXPORT_SYMBOL(mdrv_spe_get_bypass_id);
EXPORT_SYMBOL(mdrv_spe_get_spe_balong);
