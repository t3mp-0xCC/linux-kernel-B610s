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
 

#include <net/arp.h>
#include <net/ndisc.h>
#include "spe_entry.h"
#include "spe_dbg.h"
#include <linux/netfilter/nf_nat.h>
#include <net/netfilter/nf_conntrack_l3proto.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <linux/ipv6.h>
/*
 * debug code for spe_ip_fw_add, the IP_FW_ADD_ERR_REC_MAX is the max code line
 * of function: spe_ip_fw_add
 */
#define IP_FW_ADD_ERR_REC_MAX   300
#define IP_FW_ADD_REC_MASK      (IP_FW_ADD_ERR_REC_MAX -1)
#define IP_FW_ADD_ERR_REC_INIT() \
do {\
    /* idx 0 is record the base line */\
    if (unlikely(NULL == sg_ip_fw_err_line)) {\
        sg_ip_fw_err_line = kzalloc(sg_ip_fw_dft_max*sizeof(int), GFP_ATOMIC);\
        if (sg_ip_fw_err_line) \
            sg_ip_fw_err_line[0] = __LINE__;\
    }\
}while(0)

#define IP_FW_ADD_ERR_REC_START() \
do {\
    int idx;\
    /* the room of "err line rec" is ok */\
    if (likely(sg_ip_fw_err_line)) {\
        idx = __LINE__ - sg_ip_fw_err_line[0];\
        /* the room of "err line rec" is not enough */\
        if (unlikely(idx >= sg_ip_fw_dft_max)) {\
            /* expand the max and remalloc */\
            int* rec_alloc;\
            sg_ip_fw_dft_max = idx + 100;\
            rec_alloc = kzalloc(sg_ip_fw_dft_max*sizeof(int), GFP_ATOMIC);\
            if (rec_alloc) {\
                /* kfree the old one and use the new room */\
                kfree((void*)sg_ip_fw_err_line);\
                sg_ip_fw_err_line = rec_alloc;\
                sg_ip_fw_err_line[idx]++;\
            }\
        }\
        /* ok, record it */\
        else {\
            sg_ip_fw_err_line[idx]++;\
        }\
    }\
}while(0)

static int* sg_ip_fw_err_line = NULL;
static u32 sg_ip_fw_dft_max = IP_FW_ADD_ERR_REC_MAX;


extern u_int32_t spe_hash_conntrack(const struct nf_conntrack_tuple *tuple,
				  u16 zone, unsigned int size);
extern void ip6_route_input(struct sk_buff *skb);

static int spe_ipv6_skip_exthdr(const struct sk_buff *skb, int start,
				  uint8_t *nexthdrp, int len)
{
	uint8_t nexthdr = *nexthdrp;

	while (ipv6_ext_hdr(nexthdr)) {
		struct ipv6_opt_hdr hdr;
		int hdrlen;

		if (len < (int)sizeof(struct ipv6_opt_hdr))
			return -1;
		if (nexthdr == NEXTHDR_NONE)
			break;
		if (nexthdr == NEXTHDR_FRAGMENT)
			break;
		if (skb_copy_bits(skb, start, &hdr, sizeof(hdr)))
			BUG();
		if (nexthdr == NEXTHDR_AUTH)
			hdrlen = (hdr.hdrlen+2)<<2;
		else
			hdrlen = ipv6_optlen(&hdr);

		nexthdr = hdr.nexthdr;
		len -= hdrlen;
		start += hdrlen;
	}

	*nexthdrp = nexthdr;
	return start;
}

int spe_ipv6_get_l4proto(const struct sk_buff *skb, unsigned int nhoff,
			    unsigned int *dataoff, u_int8_t *protonum)
{
	unsigned int extoff = nhoff + sizeof(struct ipv6hdr);
	unsigned char pnum;
	int protoff;

	if (skb_copy_bits(skb, nhoff + offsetof(struct ipv6hdr, nexthdr),
			  &pnum, sizeof(pnum)) != 0) {
		SPE_ERR("spe_ipv6_get_l4proto: can't get nexthdr\n");
		return -EIO;
	}
	protoff = spe_ipv6_skip_exthdr(skb, extoff, &pnum, skb->len - extoff);
	/*
	 * (protoff == skb->len) mean that the packet doesn't have no data
	 * except of IPv6 & ext headers. but it's tracked anyway. - YK
	 */
	if ((protoff < 0) || (protoff > skb->len)) {
		SPE_ERR("spe_ipv6_get_l4proto: can't find proto in pkt\n");
		return -EIO;
	}

	*dataoff = protoff;
	*protonum = pnum;
	return 0;
}

static inline unsigned compare_ipall_addr(const u32 *a, const u32 *b)
{	return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2]) | (a[3] ^ b[3])) == 0;
}

/*black_write = 0 : write ; black_write = 1 : black*/
void spe_entry_set_black_white(enum spe_tab_list_type list_type, unsigned int black_white)
{
    struct spe *spe = &spe_balong;
    HI_SPE_BLACK_WHITE_T black_white_reg;
	black_white_reg.u32 = spe_readl(spe->regs, HI_SPE_BLACK_WHITE_OFFSET);

    switch(list_type){
        case spe_tab_list_mac_filt:
            black_white_reg.bits.spe_mac_black_white = !!black_white;
            spe_writel(spe->regs, HI_SPE_BLACK_WHITE_OFFSET, black_white_reg.u32);
            break;

        case spe_tab_list_ipv4_filt:
        case spe_tab_list_ipv6_filt:
            black_white_reg.bits.spe_ip_black_white = !!black_white;
            spe_writel(spe->regs, HI_SPE_BLACK_WHITE_OFFSET, black_white_reg.u32);
            break;

        default:
            SPE_ERR("invalid list_type(%d)\n",list_type);
            break;
    }
}

void spe_entry_get_black_white(void)
{
    struct spe *spe = &spe_balong;
    HI_SPE_BLACK_WHITE_T black_white_reg;
	black_white_reg.u32 = spe_readl(spe->regs, HI_SPE_BLACK_WHITE_OFFSET);

    printk("mac:%s  ip:%s\n", black_white_reg.bits.spe_mac_black_white?"black":"white", black_white_reg.bits.spe_ip_black_white?"black":"white");
}

static inline int spe_entry_config(enum spe_tab_ctrl_type type,
    u32 *buf, u32 size)
{
    struct spe *spe = &spe_balong;
    u32 i;
    u32 timeout = 500;
    HI_SPE_TAB_ACT_RESULT_T act_result;

    for(i=0; i<size; i++){
        spe_writel(spe->regs, SPE_TAB_WORD_OFFSET(i), buf[i]);
    }

    spe_writel(spe->regs, HI_SPE_TAB_CTRL_OFFSET, type);

    do{
        act_result.u32 = spe_readl(spe->regs, HI_SPE_TAB_ACT_RESULT_OFFSET);
        if(act_result.bits.spe_tab_done){
            /* spe_tab_done bit is w1c,so we must clear it */
            spe_writel(spe->regs, HI_SPE_TAB_ACT_RESULT_OFFSET, act_result.u32);
            return act_result.bits.spe_tab_success?0:SPE_ERR_ENTRYOP;
        }

		timeout--;
		if (!timeout){
            SPE_ERR("spe entry config timeout!type:%d,buf:%x,size:%x\n",type,buf,size);
            return -ETIMEDOUT;
        }

		udelay(1);
    }while(1);
}

int spe_mac_filt_add(struct spe_mac_filt_entry *ent)
{
    if(!ent){
        return -EINVAL;
    }
    return spe_entry_config(spe_tab_ctrl_add_mac_filt,
                            (u32 *)ent, sizeof(*ent)/sizeof(u32));
}

int spe_mac_filt_del(struct spe_mac_filt_entry *ent)
{
    if(!ent){
        return -EINVAL;
    }
    return spe_entry_config(spe_tab_ctrl_del_mac_filt,
                            (u32 *)ent, sizeof(*ent)/sizeof(u32));
}

int spe_ipv4_filt_add(struct spe_ipv4_filt_entry *ent)
{
    if(!ent){
        return -EINVAL;
    }
    return spe_entry_config(spe_tab_ctrl_add_ipv4_filt,
                            (u32 *)ent, sizeof(*ent)/sizeof(u32));
}

int spe_ipv4_filt_del(struct spe_ipv4_filt_entry *ent)
{
    if(!ent){
        return -EINVAL;
    }
    return spe_entry_config(spe_tab_ctrl_del_ipv4_filt,
                            (u32 *)ent, sizeof(*ent)/sizeof(u32));
}

int spe_ipv6_filt_add(struct spe_ipv6_filt_entry *ent)
{
    if(!ent){
        return -EINVAL;
    }
    return spe_entry_config(spe_tab_ctrl_add_ipv6_filt,
                            (u32 *)ent, sizeof(*ent)/sizeof(u32));
}

int spe_ipv6_filt_del(struct spe_ipv6_filt_entry *ent)
{
    if(!ent){
        return -EINVAL;
    }
    return spe_entry_config(spe_tab_ctrl_del_ipv6_filt,
                            (u32 *)ent, sizeof(*ent)/sizeof(u32));
}

static void spe_list_del(struct spe *spe, struct spe_mac_fw_entry_ext* ent_ext, 
            struct list_head * d_list)
{
	struct spe_mac_fw_entry_ext *pos;
    struct spe_mac_fw_entry_ext *n;

	list_for_each_entry_safe(pos, n, d_list, list){
        if(ent_ext->ent.mac_hi == pos->ent.mac_hi
            && ent_ext->ent.mac_lo == pos->ent.mac_lo){
            list_del_init((struct list_head *)&pos->list);
            kmem_cache_free(spe->macfw.slab, (void*)pos);
        }
    }
}

/* add an entry to SPE HW*/
static int spe_entry_add_sync(struct spe *spe,struct spe_mac_fw_entry_ext* new_entry){
	int ret = 0;
	
	if(!spe->mask_flags){
		/* to avoid duplication, delete first */
		spe_entry_config(spe_tab_ctrl_del_mac_fw,
                            (u32 *)&new_entry->ent, sizeof(new_entry->ent)/sizeof(u32));
			/* if no duplication, add it */
		ret = spe_entry_config(spe_tab_ctrl_add_mac_fw,
                            (u32 *)&new_entry->ent, sizeof(new_entry->ent)/sizeof(u32));
	}

	if(!ret)
	{
		spe_list_del(spe, new_entry, &spe->macfw.backups);
		list_add(&new_entry->list, &spe->macfw.backups);
	}

	return ret;
}

static int spe_entry_del_sync(struct spe *spe,struct spe_mac_fw_entry_ext* ent_ext){
	int ret=0;
	
    ret = spe_entry_config(spe_tab_ctrl_del_mac_fw,
                            (u32 *)&ent_ext->ent, sizeof(ent_ext->ent)/sizeof(u32));
	spe_list_del(spe, ent_ext, &spe->macfw.backups);

	return ret;
}

int spe_mac_fw_add(const char *mac, uint16_t vid, u32 br_speno, u32 src_speno, u32 is_static)
{
    struct spe *spe = &spe_balong;
    struct spe_mac_fw_entry_ext* ent_ext;
	unsigned long flags;
    int ret = 0;

    if(!mac){
        return -EINVAL;
    }

    ent_ext = (struct spe_mac_fw_entry_ext*)
                kmem_cache_alloc(spe->macfw.slab , GFP_ATOMIC);
    if (!ent_ext) {
        return -ENOMEM;
    }

    memset((void*)ent_ext, 0, sizeof(struct spe_mac_fw_entry_ext));
    /* mac field in big endian */
    ent_ext->ent.mac_hi = (mac[0]) | (mac[1] << 8) | (mac[2] << 16) | (mac[3] << 24);
    ent_ext->ent.mac_lo = (mac[4]) | (mac[5]<<8);
    ent_ext->ent.port_br_id = br_speno & 0xf;
    ent_ext->ent.port_no = src_speno & 0xf;
    ent_ext->ent.vid = vid & 0xfff;
    ent_ext->ent.is_static = is_static;

	spin_lock_irqsave(&spe->macfw.lock, flags);
	
	ret = spe_entry_add_sync(spe,ent_ext);
	if(ret){
	        /* if entry op fail, add it to pending queue */
		spe_list_del(spe, ent_ext, &spe->macfw.pending);
		list_add(&ent_ext->list, &spe->macfw.pending);
	}
   
	spin_unlock_irqrestore(&spe->macfw.lock, flags);
    
    return ret;
}

int spe_mac_fw_del(char *mac, uint16_t vid, u32 br_speno, u32 src_speno)
{
    struct spe_mac_fw_entry_ext ent_ext = {{0}};
    struct spe *spe = &spe_balong;
    int ret;
	unsigned long flags;
        struct spe_mac_fw_entry_ext *pos;
        struct spe_mac_fw_entry_ext *n;
    if(!mac){
        return -EINVAL;
    }

    /* mac field in big endian */
    ent_ext.ent.mac_hi = (mac[0]) | (mac[1] << 8) | (mac[2] << 16) | (mac[3] << 24);
    ent_ext.ent.mac_lo = (mac[4]) | (mac[5]<<8);
    ent_ext.ent.port_br_id = br_speno & 0xf;
    ent_ext.ent.port_no = src_speno & 0xf;
    ent_ext.ent.vid = vid & 0xfff;

	spin_lock_irqsave(&spe->macfw.lock, flags);
	
    /* delete the entry both in HW and pending queue */	
    ret = spe_entry_del_sync(spe, &ent_ext);
    if(!ret){
        list_for_each_entry_safe(pos, n, &spe->macfw.pending, list){
			list_del_init((struct list_head *)&pos->list);
            if(!spe_entry_add_sync(spe,pos)){
				break;		
            }
			else{
				list_add(&pos->list, &spe->macfw.pending);
			}
        }
        
    }else{
		spe_list_del(spe, &ent_ext,&spe->macfw.pending);
	}

	spin_unlock_irqrestore(&spe->macfw.lock, flags);

	return ret;
}

static void spe_mac_fw_checkout(struct spe *spe)
{
    struct spe_mac_fw_entry_ext *pos;
    struct spe_mac_fw_entry_ext *n;

    list_for_each_entry_safe(pos, n, &spe->macfw.backups, list){
        spe_entry_config(spe_tab_ctrl_del_mac_fw,
                            (u32 *)&pos->ent, sizeof(pos->ent)/sizeof(u32));
		spe_entry_config(spe_tab_ctrl_add_mac_fw,
                            (u32 *)&pos->ent, sizeof(pos->ent)/sizeof(u32));
    }

}
void spe_print_macfw_backup(void)
{
    struct spe_mac_fw_entry_ext *pos;
    struct spe_mac_fw_entry_ext *n;
	struct spe *spe = &spe_balong;
	list_for_each_entry_safe(pos, n, &spe->macfw.backups, list){
        printk("MAC: %x%x\n",pos->ent.mac_hi,pos->ent.mac_lo);
    }
}

void spe_macfw_entry_dump(void)
{
	int spe_tab[SPE_MAC_TAB_WORD_NO];
	struct spe *spe = &spe_balong;
	struct spe_mac_entry_in_sram* mac_entry;
	int i;
	int j;

	spe_dbgen_enable();
	
	for (i = 0; i < SPE_MAC_TAB_NUMBER; i++){
		for (j = 0; j<SPE_MAC_TAB_WORD_NO; j++){
			spe_tab[j]=spe_readl(spe->regs, SPE_MAC_TAB_OFFSET(i*SPE_MAC_TAB_WORD_NO+j));
		}
		if (spe_tab[3] & IS_MAC_FW_ENTRY){
			mac_entry = (struct spe_mac_entry_in_sram*)spe_tab;
		}else{
			continue;
		}	
		printk("mac entry (%d) : %pM	bid:%d	vid:%d	port:%d	timestamp:0x%x	static:%d\n",
			i, mac_entry->mac,	mac_entry->bid & 0xf, mac_entry->vid  & 0xfff 
			, mac_entry->port  & 0xf , mac_entry->timestamp  & 0xffff, mac_entry->is_static  & 0x1);
	}
	spe_dbgen_disable();
}

void spe_macfl_entry_dump(void)
{
	int spe_tab[SPE_MAC_TAB_WORD_NO];
	struct spe *spe = &spe_balong;
	struct spe_mac_entry_in_sram* mac_entry;
	int i;
	int j;

	spe_dbgen_enable();

	for (i = 0; i < SPE_MAC_TAB_NUMBER; i++){
		for (j = 0; j<SPE_MAC_TAB_WORD_NO; j++){
			spe_tab[j]=spe_readl(spe->regs, SPE_MAC_TAB_OFFSET(i*SPE_MAC_TAB_WORD_NO+j));
		}
		if (spe_tab[3] & IS_MAC_FL_ENTRY){
			mac_entry = (struct spe_mac_entry_in_sram*)spe_tab;
		}else{
			continue;
		}
		printk("[mac entry (%d)] %pM  action:%d  redir_port:%d\n",
		 i, mac_entry->mac,
		 mac_entry->action & 0x1 , mac_entry->rd_port & 0xf);
	}
	spe_dbgen_disable();
    printk("==================================================================\n");
}

void spe_ipv4_filter_dump(void)
{
	int spe_tab[SPE_IPV4_TAB_WORD_NO];
	struct spe *spe = &spe_balong;
    struct spe_ipv4_filter_entry_in_sram* ipv4_filter_entry;

	int i;
	int j;

	spe_dbgen_enable();

	for (i = 0; i < SPE_IPV4_TAB_NUMBER; i++){
		for (j = 0; j<SPE_IPV4_TAB_WORD_NO; j++){
			spe_tab[j]=spe_readl(spe->regs, SPE_IPV4_TAB_OFFSET(i*SPE_IPV4_TAB_WORD_NO+j));
		};
		if (spe_tab[1] & IS_IP_FL_ENTRY){
            ipv4_filter_entry = (struct spe_ipv4_filter_entry_in_sram*)spe_tab;
		}else{
			continue;
		}
		printk("[ipv4 entry (%d)] \nsip_mask%d  dip_mask:%d  protocol:%d  tos:%d  filter_mask:0x%x  "
            "sip:%pI4  dip:%pI4   sport_min:%d  sport_max:%d  dport_min:%d  dport_max:%d\n",
		 i, ipv4_filter_entry->sip_mask & 0xff, ipv4_filter_entry->dip_mask & 0xff, 
		 ipv4_filter_entry->protocol & 0xff, ipv4_filter_entry->tos & 0xff, ipv4_filter_entry->filter_mask & 0xff,
		 ipv4_filter_entry->sip, ipv4_filter_entry->dip, be16_to_cpu(ipv4_filter_entry->sport_min & 0xffff),
		 be16_to_cpu(ipv4_filter_entry->sport_max & 0xffff), be16_to_cpu(ipv4_filter_entry->dport_min & 0xffff),
		 be16_to_cpu(ipv4_filter_entry->dport_max & 0xffff));
	}
	spe_dbgen_disable();
    printk("================================================================\n");
}

void spe_ipv6_filter_dump(void)
{
	int spe_tab[SPE_IPV6_TAB_WORD_NO];
	struct spe *spe = &spe_balong;
	struct spe_ipv6_filter_entry_in_sram *ipv6_filter_entry;
	int i;
	int j;

	spe_dbgen_enable();

	for (i = 0; i < SPE_IPV6_TAB_NUMBER; i++){
		for (j = 0; j<SPE_IPV6_TAB_WORD_NO; j++){
			spe_tab[j]=spe_readl(spe->regs, SPE_IPV6_TAB_OFFSET(i*SPE_IPV6_TAB_WORD_NO+j));
		}
		if (spe_tab[1] & IS_IP_FL_ENTRY){
			ipv6_filter_entry = (struct spe_ipv6_filter_entry_in_sram*)spe_tab;
		}else{
			continue;
		}
		printk("[ipv6 entry (%d)] \nsip_mask:%d  dip_mask:%d  next_hdr:0x%x  tc:%d  filter_mask:0x%x  "
            "flow_label:0x%x  sip:%pI6  dip:%pI6  sport_min:%d  sport_max:%d  dport_min:%d  dport_max:%d\n",
		 i, ipv6_filter_entry->sip_mask & 0xff, ipv6_filter_entry->dip_mask & 0xff, 
		 ipv6_filter_entry->next_hdr & 0xff, ipv6_filter_entry->tc & 0xff, ipv6_filter_entry->filter_mask & 0xff,
		 ipv6_filter_entry->flow_label & 0xff, ipv6_filter_entry->sip, ipv6_filter_entry->dip, 
		 be16_to_cpu(ipv6_filter_entry->sport_min & 0xffff), be16_to_cpu(ipv6_filter_entry->sport_max & 0xffff),
		 be16_to_cpu(ipv6_filter_entry->dport_min & 0xffff), be16_to_cpu(ipv6_filter_entry->dport_max & 0xffff));
	}
	spe_dbgen_disable();
    printk("==============================================================\n");
}


void spe_transfer_pause(void){
    struct spe *spe = &spe_balong;
    HI_SPE_TAB_CLR_T tab_clr;
	unsigned long flags;
	
	spin_lock_irqsave(&spe->macfw.lock, flags);
	
    spe_writel(spe->regs, HI_SPE_HASH_BADDR_OFFSET, spe->ipfw.hbucket_dma_empty);
	
    tab_clr.u32 = spe_readl(spe->regs, HI_SPE_TAB_CLR_OFFSET);
    tab_clr.bits.spe_tab_clr |= BIT(1); 
    spe_writel(spe->regs, HI_SPE_TAB_CLR_OFFSET, tab_clr.u32);
	
	spe->mask_flags = 1;

	spin_unlock_irqrestore(&spe->macfw.lock, flags);
}

void spe_transfer_restart(void){
    struct spe *spe = &spe_balong;
    HI_SPE_TAB_CLR_T tab_clr;
	unsigned long flags;
	
	spin_lock_irqsave(&spe->macfw.lock, flags);
    spe_writel(spe->regs, HI_SPE_HASH_BADDR_OFFSET, spe->ipfw.hbucket_dma);
	
    tab_clr.u32 = spe_readl(spe->regs, HI_SPE_TAB_CLR_OFFSET);
    tab_clr.bits.spe_tab_clr &= ~(BIT(1)); 
    spe_writel(spe->regs, HI_SPE_TAB_CLR_OFFSET, tab_clr.u32);

	spe->mask_flags = 0;
	spe_mac_fw_checkout(spe);

	spin_unlock_irqrestore(&spe->macfw.lock, flags);
}

static inline int ip_fw_node_recyclable(struct spe *spe,
                                struct spe_ip_fw_entry *ent)
{
    return time_after(jiffies, (unsigned long)ent->dead_timestamp + (unsigned long)spe->ipfw.deadtime);
}

struct spe_ip_fw_entry *ip_fw_node_get(struct spe *spe)
{
    struct spe_ip_fw_entry *pos = NULL;
    dma_addr_t dma = 0;
    unsigned long flags;

    spin_lock_irqsave(&spe->ipfw.free_lock, flags);

    if(!list_empty(&spe->ipfw.free_list)){
        pos = list_first_entry(&spe->ipfw.free_list, struct spe_ip_fw_entry, list);
        if(ip_fw_node_recyclable(spe, pos)){
            list_del_init(&pos->list);
            spe->ipfw.free_cnt--;
            spin_unlock_irqrestore(&spe->ipfw.free_lock, flags);
            return pos;
        }
    }

    spin_unlock_irqrestore(&spe->ipfw.free_lock, flags);

    pos = (struct spe_ip_fw_entry *)dma_pool_alloc(spe->ipfw.hslab, GFP_ATOMIC, &dma);
	if (!pos){
		return NULL;
	}

    pos->dma = dma;
	pos->prev = NULL;
	pos->next = NULL;
    pos->dead_timestamp = INITIAL_JIFFIES;
	INIT_LIST_HEAD(&pos->list);

    return pos;
}

void ip_fw_node_put(struct spe *spe, struct spe_ip_fw_entry *ent, bool to_pool)
{
    unsigned long flags;

    if(to_pool){
        dma_pool_free(spe->ipfw.hslab, ent, ent->dma);
    }else{
        spin_lock_irqsave(&spe->ipfw.free_lock, flags);

        list_add_tail(&ent->list, &spe->ipfw.free_list);

        spe->ipfw.free_cnt++;

        if(spe->ipfw.free_cnt > spe->ipfw.free_threhold){
            struct spe_ip_fw_entry *pos, *n;

            list_for_each_entry_safe(pos, n, &spe->ipfw.free_list, list){
                if(ip_fw_node_recyclable(spe, pos)){
                    list_del_init(&pos->list);

                    dma_pool_free(spe->ipfw.hslab, pos, pos->dma);
                }else{
                    break;
                }
            }
        }

        spin_unlock_irqrestore(&spe->ipfw.free_lock, flags);
    }
}

bool ip_fw_tuple_compare(struct nf_conntrack_tuple *a,
                            struct nf_conntrack_tuple *b)
{
    if(compare_ipall_addr(a->src.u3.all,b->src.u3.all)
        && compare_ipall_addr(a->dst.u3.all,b->dst.u3.all)
        && (a->src.u.all == b->src.u.all)
        && (a->src.l3num == b->src.l3num)
        && (a->dst.u.all == b->dst.u.all)
        && (a->dst.protonum == b->dst.protonum)){
        return true;
    }
    return false;
}

static void ipv4_tuple_show(struct nf_conntrack_tuple *tuple)
{
	printk(KERN_EMERG"l3num(%d) protonum(%d) %pI4:(%u) --> %pI4:(%u)\n",\
		tuple->src.l3num, tuple->dst.protonum, \
		&(tuple->src.u3.ip), ntohs(tuple->src.u.all), \
		&(tuple->dst.u3.ip), ntohs(tuple->dst.u.all));
    return;
}

static void ipv6_tuple_show(struct nf_conntrack_tuple *tuple)
{
	printk(KERN_EMERG"l3num(%d) protonum(%d) %pI6:(%u) --> \n %pI6:(%u)\n",\
		tuple->src.l3num, tuple->dst.protonum, \
		&(tuple->src.u3.ip6), ntohs(tuple->src.u.all), \
		&(tuple->dst.u3.ip6), ntohs(tuple->dst.u.all));
	return;
}

static void tuple_show(struct nf_conntrack_tuple *tuple)
{
    if (AF_INET6 == tuple->src.l3num) {
        ipv6_tuple_show(tuple);
    }
    else {
        ipv4_tuple_show(tuple);
    }
}

/* move to global, so we can modify them by debuger */
static char usb0_pc_ip[4] = {2,4,10,1};
static char usb0_soc_ip[4] = {2,4,10,150};
static char eth0_pc_ip[4] = {192,168,1,10};

static char eth0_soc_mac[ETHER_ADDR_LEN] = {0x00,0x13,0x3b,0x0d,0xcf,0x09};
static char eth0_pc_mac[ETHER_ADDR_LEN] = {0x3c,0x97,0x0e,0x50,0xe7,0x77};
static char usb0_soc_mac[ETHER_ADDR_LEN] = {0x58,0x02,0x03,0x04,0x06,0x06};
static char usb0_pc_mac[ETHER_ADDR_LEN] = {0x00,0x11,0x09,0x64,0x1,0x1};

int iptable_add(u16 sport, u16 dport, u16 l3num, u8 protonum, int ntor)
{
    struct spe *spe = &spe_balong;
    struct spe_ip_fw_entry *new = NULL;
    struct spe_ip_fw_entry *head = &spe_balong.ipfw.hbucket[0];
	char* sip;
	char* dip;
	char* dnat;
	char* smac;
	char* dmac;
 	unsigned long flags = 0;
 	dma_addr_t handle;

    new = (struct spe_ip_fw_entry *)dma_pool_alloc(spe->ipfw.hslab, GFP_KERNEL, &handle);
	if (!new) {
		return -ENOMEM;
	}
	head->next = (struct spe_ip_fw_entry *)handle;
	INIT_LIST_HEAD(&new->list);
	if (ntor) {
		sip = usb0_pc_ip;
		dip = usb0_soc_ip;
		dnat = eth0_pc_ip;
		smac = eth0_soc_mac;
		dmac = eth0_pc_mac;
		new->portno = 2;
		new->action = 1;/*dnat*/
	} else {
		sip = eth0_pc_ip;
		dip = usb0_pc_ip;
		dnat = usb0_soc_ip;
		smac = usb0_soc_mac;
		dmac = usb0_pc_mac;
		new->portno = 1;
		new->action = 0;/*snat*/
	}
	memcpy(&new->tuple.src.u3.ip, sip, 4);
	memcpy(&new->tuple.dst.u3.ip, dip, 4);
	memcpy(&new->shost.octet, smac, 6);
	memcpy(&new->dhost.octet, dmac, 6);

	new->tuple.src.u.all = ntohs(sport);
	new->tuple.src.l3num = l3num;
	new->tuple.dst.u.all = ntohs(dport);
	new->tuple.dst.protonum = protonum;
	new->dma = handle;
	new->is_static = 1;
	memcpy(&new->nat.ip, dnat, 4);
	new->nat.port = ntohs(sport);
	/* just has one node, next must be NULL */
	new->next = NULL;

	spin_lock_irqsave(&spe->ipfw.lock, flags);
	list_add(&new->list, &head->list);
	spin_unlock_irqrestore(&spe->ipfw.lock, flags);

	new->valid = 1;

	return 0;
}

int iptable_delete(void)
{
    struct spe *spe = &spe_balong;
    struct spe_ip_fw_entry *pos = NULL;
    struct spe_ip_fw_entry *n = NULL;
    struct spe_ip_fw_entry *head = &spe_balong.ipfw.hbucket[0];
    unsigned long flags = 0;

    spin_lock_irqsave(&spe->ipfw.lock, flags);

	list_for_each_entry_safe(pos, n, &head->list, list) {
	    pos->valid = 0;
		list_del(&pos->list);
        dma_pool_free(spe->ipfw.hslab, pos, pos->dma);
	}

	spin_unlock_irqrestore(&spe->ipfw.lock, flags);

	return 0;
}

static inline void iptable_dump_entry(int idx, struct spe_ip_fw_entry *pos)
{

    tuple_show(&pos->tuple);
    printk(KERN_EMERG"entry[%d]: valid(%d)  vir(0x%p) dma: (0x%x) nextdma: (0x%x)\n",
        idx ,pos->valid ,pos, pos->dma, (unsigned int)pos->next);
    printk(KERN_EMERG"entry->portno %d\n", pos->portno);
    printk(KERN_EMERG"entry->action %d\n", pos->action);
    printk(KERN_EMERG"entry->vid %d\n", pos->vid);
    printk(KERN_EMERG"entry->shost %pM\n", pos->shost.octet);
    printk(KERN_EMERG"entry->dhost %pM\n", pos->dhost.octet);
    printk(KERN_EMERG"manip_ip: %pI4 manip_port %u\n",\
        &(pos->nat.ip), ntohs(pos->nat.port));

}

int spe_ip_fw_show(void)
{
    struct spe *spe = &spe_balong;
    struct spe_ip_fw_entry *head = &spe->ipfw.hbucket[0];
    struct spe_ip_fw_entry *pos = NULL;
    unsigned long flags = 0;
	int i = 1;
    int k = 0;
    
    IP_FW_ADD_ERR_REC_INIT();
	if (sg_ip_fw_err_line) {
    	printk(KERN_EMERG"dump spe_ip_fw_add retrun count, max(%d), base(%d)\n",
        	sg_ip_fw_dft_max, sg_ip_fw_err_line[0]);
        for (k = 1; k < sg_ip_fw_dft_max; k++) {
            if (sg_ip_fw_err_line[k]) {
                printk(KERN_EMERG"spe_ip_fw_add err line:%d, return count:%d\n",
                    k + sg_ip_fw_err_line[0], sg_ip_fw_err_line[k]);
            }
        }
    }
    printk(KERN_EMERG"dump spe_ip_fw_add retrun count end\n\n");

    for (k = 0; k<SPE_IPFW_HTABLE_SIZE; k++)
    {
        head = &spe->ipfw.hbucket[k];
        i = 1;

        spin_lock_irqsave(&spe->ipfw.lock, flags);

        /* check if ip_fw exist */
        if (head->valid || NULL != head->next) {
            printk(KERN_EMERG"\nhbucket[%d], head->valid:%d, next:%p\n\n",
                k, head->valid, head->next);

        	/* if head is valid dump the tuple */
        	if (head->valid) {
        	    iptable_dump_entry(0, head);
        	}

            list_for_each_entry(pos, &head->list, list){
                iptable_dump_entry(i, pos);
        		i++;
            }
        }

        spin_unlock_irqrestore(&spe->ipfw.lock, flags);
    }

	return 0;
}

void spe_add(u16 sport, u16 dport, int udp, int ntor)
{
	if (udp) {
		iptable_add(sport, dport, 2, 17, ntor);
	} else {
		iptable_add(sport, dport, 2, 6, ntor);
	}
}

struct spe_ip_fw_entry *ip_fw_entry_exist(struct spe *spe,
                                struct nf_conntrack_tuple *tuple, u32 hkey)
{
    struct list_head *head = &spe->ipfw.hbucket[hkey].list;
    struct spe_ip_fw_entry *first = &spe->ipfw.hbucket[hkey];
    struct spe_ip_fw_entry *pos = NULL;

    /* check if first header exist */
    if(first->valid && ip_fw_tuple_compare(&first->tuple, tuple)){
        return first;
    }

    /* check if ip_fw exist */
    list_for_each_entry(pos, head, list){
        if(pos->valid && ip_fw_tuple_compare(&pos->tuple, tuple)){
            return pos;
        }
    }

    return NULL;
}

int ip_fw_list_del(struct spe *spe,
                    struct nf_conntrack_tuple *tuple,
                    u32 hkey)
{
    struct spe_ip_fw_entry *pos = NULL;
    unsigned long flags;

    spin_lock_irqsave(&spe->ipfw.lock, flags);

    if((pos = ip_fw_entry_exist(spe, tuple, hkey))){
        pos->next = 0;
        pos->valid = 0;
        pos->dead_timestamp = jiffies;
        list_entry(pos->list.prev, struct spe_ip_fw_entry, list)->next = (struct spe_ip_fw_entry *)
                (list_is_last(&pos->list, &spe->ipfw.hbucket[hkey].list)?
                0:list_entry(pos->list.next, struct spe_ip_fw_entry, list)->dma);

        list_del_init(&pos->list);
        spin_unlock_irqrestore(&spe->ipfw.lock, flags);
        ip_fw_node_put(spe, pos, false);
		spe->stat.ipfw_del++;
        return 0;
    }else{
        spin_unlock_irqrestore(&spe->ipfw.lock, flags);
        return -ENOENT;
    }
}

void ip_fw_list_add(struct spe *spe, struct spe_ip_fw_entry *new, u32 hkey)
{
    struct spe_ip_fw_entry *head = &spe->ipfw.hbucket[hkey];
    unsigned long flags;

    spin_lock_irqsave(&spe->ipfw.lock, flags);

    list_add(&new->list, &head->list);  /* add entry after first node */
    new->next = (struct spe_ip_fw_entry *)(list_is_last(&new->list, &head->list)?
        0:list_entry(new->list.next,struct spe_ip_fw_entry , list)->dma);
    new->valid = 1;
    head->next = (struct spe_ip_fw_entry *)new->dma;   /* real add to list */

	spe->stat.ipfw_add++;

    spin_unlock_irqrestore(&spe->ipfw.lock, flags);

    return ;
}

int spe_ip_fw_add_tuple(struct spe_ip_fw_entry *entry,
    struct nf_conn *ct, int dir)
{
       
    memcpy(&entry->tuple, &ct->tuplehash[dir].tuple,
        sizeof(struct nf_conntrack_tuple));

    if (__nf_ct_l3proto_find(nf_ct_l3num(ct))->l3proto == AF_INET) {
        entry->iptype = 0;
    } else {
       entry->iptype = 1;
    }
    
	if (ct->status & IPS_SRC_NAT) {
        if (dir == IP_CT_DIR_ORIGINAL) {
            entry->nat.ip = ct->tuplehash[!dir].tuple.dst.u3.ip;
    		entry->nat.port = ct->tuplehash[!dir].tuple.dst.u.tcp.port;
    		entry->action = NF_NAT_MANIP_SRC;
        } else {
            entry->nat.ip = ct->tuplehash[!dir].tuple.src.u3.ip;
            entry->nat.port = ct->tuplehash[!dir].tuple.src.u.tcp.port;
    		entry->action = NF_NAT_MANIP_DST;
        }
	} else if (ct->status & IPS_DST_NAT) {
        if (dir == IP_CT_DIR_ORIGINAL) {
            entry->nat.ip = ct->tuplehash[!dir].tuple.src.u3.ip;
    		entry->nat.port = ct->tuplehash[!dir].tuple.src.u.tcp.port;
    		entry->action = NF_NAT_MANIP_DST;
        } else {
            entry->nat.ip = ct->tuplehash[!dir].tuple.dst.u3.ip;
            entry->nat.port = ct->tuplehash[!dir].tuple.dst.u.tcp.port;
    		entry->action = NF_NAT_MANIP_SRC;
        }
	} else {
        entry->action = 0x3;
        entry->nat.ip = 0;
        entry->nat.port = 0;
    }
        
    return 0;
}

int spe_ip_fw_add_entry(struct neighbour *n, struct net_device *indev, struct nf_conn *ct, struct sk_buff *skb, int dir)
{
	struct spe *spe = &spe_balong;
	struct spe_ip_fw_entry *entry;
	u32 hkey;
	unsigned int seq;
	
	entry = ip_fw_node_get(spe);
    if (!entry) {
        return -ENOMEM;
    }
	if(IP_CT_DIR_ORIGINAL == dir){
		entry->portno = n->dev->spe_portno;
        entry->vid = n->dev->spe_vid;
	    if (n->hh.hh_len) {
	    	do {
	    		seq = read_seqbegin(&n->hh.hh_lock);
	        	memcpy(entry->shost.octet, (char*)n->hh.hh_data
	                + HH_DATA_OFF(sizeof(struct ethhdr)) + ETH_ALEN, ETH_ALEN);
	        	memcpy(entry->dhost.octet, (char*)n->hh.hh_data
	                + HH_DATA_OFF(sizeof(struct ethhdr)), ETH_ALEN);
	    	} while (read_seqretry(&n->hh.hh_lock, seq));
	    } else {
	        memcpy(entry->shost.octet, n->dev->dev_addr, ETH_ALEN);
	        memcpy(entry->dhost.octet, n->ha, ETH_ALEN);
	    }
	}else{
		entry->portno = indev->spe_portno;
        entry->vid = indev->spe_vid;
		memcpy(entry->shost.octet, skb->mac_header, ETH_ALEN);
		memcpy(entry->dhost.octet, skb->mac_header+ETH_ALEN, ETH_ALEN);
	}

	/* explicit initialize unused field */

	entry->dst_net = 0;

	spe_ip_fw_add_tuple(entry, ct, dir);
	
	 /* get timestamp */
    entry->timestamp = spe_timestamp_get();

    /* get hash */
    hkey = spe_hash_conntrack(&entry->tuple, spe->ipfw.hzone, spe->ipfw.hsize);

    /* add node to list */
    ip_fw_list_add(spe, entry, hkey);

	return 0;
}
/*	
* the entry must adds in orginial dir, otherwise causes error. 
* we choose adding spe entry in dst_neigh_output, but it is 
* to be late. the dev is replaced by dst->dev, the outdev.
* so we must find out indev.
****/
int spe_ip_fw_add(struct neighbour *n, struct sk_buff *skb)
{
    struct spe *spe = &spe_balong;
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	struct nf_conn_help *help;
	struct net_device	*indev = NULL;
	u8 protocol;

    IP_FW_ADD_ERR_REC_INIT();
	spe->stat.ipfw_add_enter++;
	ct = nf_ct_get(skb, &ctinfo);

    if (!ct) {
	    IP_FW_ADD_ERR_REC_START();
		goto err;
    }

	if((ct->status&IPS_DST_NAT)&&(ct->status&IPS_SRC_NAT)){
	    IP_FW_ADD_ERR_REC_START();
		goto out;
    }
	
	 /*only for reachable skb*/
	if (!(n->nud_state & NUD_CONNECTED)) {
		IP_FW_ADD_ERR_REC_START();
		goto out;
	}

	 /*only for tcp udp*/
	if(AF_INET == nf_ct_l3num(ct)) {
		protocol = ip_hdr(skb)->protocol;
		if(!((IPPROTO_TCP == protocol
			&& (ctinfo == IP_CT_ESTABLISHED))
			|| IPPROTO_UDP == protocol)){
				IP_FW_ADD_ERR_REC_START();
				goto out;
			}
	}else if(AF_INET6 ==nf_ct_l3num(ct)) {
		protocol = ipv6_hdr(skb)->nexthdr;
		if(!((IPPROTO_TCP == protocol
			&& (ctinfo == IP_CT_ESTABLISHED))
			|| IPPROTO_UDP == protocol)){
			IP_FW_ADD_ERR_REC_START();
			goto out;
		}
		
	}else{
			IP_FW_ADD_ERR_REC_START();
			goto out;
	}
	
	/*do not add master conntrack for spe,
    *for ppp ftp sip it will make error
    */
	help = nfct_help(ct);
	if (help && help->helper) {

		IP_FW_ADD_ERR_REC_START();
		goto out;
    }
    
    if (!n->dev) {
        IP_FW_ADD_ERR_REC_START();
        goto out;
    }
    
	 /*find indev*/
	if (skb->dev && skb->skb_iif)
	    indev = dev_get_by_index_rcu(dev_net(skb->dev), skb->skb_iif);
    if (!indev){
        IP_FW_ADD_ERR_REC_START();
        goto out;
    }
        
    /*
    *only recv forward packet
    *confirm once enough
    */
    if (!(ct->status & IPS_SPE_FORWARD) 
        || ct->status & IPS_SPE_CONFIRMED) {
        IP_FW_ADD_ERR_REC_START();
		goto out;
    }

	if(CTINFO2DIR(ctinfo) != IP_CT_DIR_ORIGINAL){
        IP_FW_ADD_ERR_REC_START();
		goto out;
	}

	spe_ip_fw_add_entry(n, indev, ct, skb, IP_CT_DIR_ORIGINAL);
	spe_ip_fw_add_entry(n, indev, ct, skb, IP_CT_DIR_REPLY);
        
    ct->status |= IPS_SPE_CONFIRMED;
    dsb();

out:
    ct->status &= ~IPS_SPE_FORWARD;

err:
	spe->stat.ipfw_add_leave++;

    return 0;
}

void spe_ip_fw_del(struct nf_conn *ct)
{
    struct spe *spe = &spe_balong;
	struct nf_conntrack_tuple *orig, *repl;
    u32 hkey;

	spe->stat.ipfw_del_enter++;

	orig = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
	repl = &ct->tuplehash[IP_CT_DIR_REPLY].tuple;

	/* If there are no packets over this connection for timeout period
	 * delete the entries.
	 */
    /* get hash */
    hkey = spe_hash_conntrack(orig, spe->ipfw.hzone, spe->ipfw.hsize);

    (void)ip_fw_list_del(spe, orig, hkey);

    hkey = spe_hash_conntrack(repl, spe->ipfw.hzone, spe->ipfw.hsize);

    (void)ip_fw_list_del(spe, repl, hkey);

	spe->stat.ipfw_del_leave++;

	dsb();

    return ;
}

void spe_ip_fw_htab_set(struct spe *spe)
{
    int size = sizeof(struct spe_ip_fw_entry) / 4;
    HI_SPE_HASH_L3_PROTO_T l3proto = {{0}};

    spe_writel(spe->regs, HI_SPE_HASH_BADDR_OFFSET, spe->ipfw.hbucket_dma);
    spe_writel(spe->regs, HI_SPE_HASH_WIDTH_OFFSET, size);
    spe_writel(spe->regs, HI_SPE_HASH_DEPTH_OFFSET, spe->ipfw.hsize);
    spe_writel(spe->regs, HI_SPE_HASH_ZONE_OFFSET, spe->ipfw.hzone);
    spe_writel(spe->regs, HI_SPE_HASH_RAND_OFFSET, spe->ipfw.hrand);
    l3proto.bits.spe_ipv4_hash_l3_proto = SPE_IPFW_L3PROTO_IPV4;
    l3proto.bits.spe_ipv6_hash_l3_proto = SPE_IPFW_L3PROTO_IPV6;
    spe_writel(spe->regs, HI_SPE_HASH_L3_PROTO_OFFSET, l3proto.u32);
}
MODULE_LICENSE("GPL");

