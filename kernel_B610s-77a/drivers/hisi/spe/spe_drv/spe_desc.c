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


#include "spe.h"
#include "spe_desc.h"
#include "spe_dbg.h"


void spe_skb_refresh(struct sk_buff *skb)
{

	skb_reset_tail_pointer(skb);
    skb->len = 0;
    skb->cloned = 0;
	skb->priority = 0;
}

int spe_td_config(int portno, struct sk_buff *skb, dma_addr_t dma, spe_l3_type_t l3_type, u32 push)
{
    struct spe *spe = &spe_balong;
    struct spe_port_ctrl *ctrl = NULL;
    struct spe_port_stat *stat = NULL;
    struct spe_td_desc *desc = NULL;
    union spe_desc_queue_ptr td_ptr;
    unsigned long flags;

    SPE_TRACE("enter\n");

    SPE_BUG(("td skb(%#x) or dma(%#x) addr null!\n", (unsigned int)skb, dma),!skb || !dma);

	if(!skb || !dma){
		return -EFAULT;
	}

    if(!spe_port_exist(portno)){
        SPE_ERR("port %d not exist!\n",portno);
        return SPE_ERR_NOCHAN;
    }

    SPE_SKB_DUMP(skb);

    ctrl = &spe->ports[portno].ctrl;
    stat = &spe->ports[portno].stat;

    spin_lock_irqsave(&ctrl->lock, flags);

    if(!(ctrl->flags & SPE_PORT_ENABLED)){
        spin_unlock_irqrestore(&ctrl->lock, flags);

        stat->td_port_disabled++;
        SPE_DBG("port %d still disabled! \n", portno);
        return SPE_ERR_PORT_DISABLED;
    }

    spin_unlock_irqrestore(&ctrl->lock, flags);

    spin_lock_irqsave(&ctrl->td_lock, flags);

    if(spe_td_full(portno)){
        spin_unlock_irqrestore(&ctrl->td_lock, flags);
        stat->td_full++;
        return SPE_ERR_TDFULL;
    }

    desc = (struct spe_td_desc *)
        (ctrl->td_addr + sizeof(struct spe_td_desc)*ctrl->td_free);
//    SPE_BUG(("desc is null!\n"), NULL==desc);

    if(portno == spe->ipfport.portno){
        if(spe_l3_ipv4 == l3_type){
            desc->trb.eth_type= 0x0008;
        }
        else if(spe_l3_ipv6 == l3_type){
            desc->trb.eth_type= 0xDD86;
        }
        else{
            SPE_ERR("ipf port set know l3 type! \n");
        }
}

    desc->trb.buf_addr = dma;
    desc->trb.skb_addr = (uint32_t)skb;
    desc->trb.push_en = push;
    if(0 != push){
        stat->td_push++;
    }

    desc->trb.irq_en = ctrl->td_evt_gap?((ctrl->td_free%ctrl->td_evt_gap)?0:1):1;

	if(spe->dbg_level & SPE_DBG_RD_TIMESTAMP){
	    desc->trb.timestamp_en = 1;
	    stat->td_ts_en++;
	}else{
    	desc->trb.timestamp_en = 0;
	}

    desc->trb.pkt_len = skb->len;
    desc->trb.result  = 0;

    ctrl->td_free = (ctrl->td_free + 1) % ctrl->td_depth;

    td_ptr.raw = spe_readl(spe->regs, SPE_TDQ_PTR_OFFSET(portno));
    td_ptr.bits.wptr = ctrl->td_free;
    spe_writel(spe->regs, SPE_TDQ_PTR_OFFSET(portno), td_ptr.raw);

    spe->stat.evt_td++;
    stat->td_config++;
    stat->td_config_bytes += skb->len;

    spin_unlock_irqrestore(&ctrl->td_lock, flags);

    SPE_TRACE("leave\n");

    return 0;
}

void rd_kevent(struct spe *spe, int portno, int flags)
{
    struct spe_port_stat *stat = &spe->ports[spe->cpuport.portno].stat;

	if (test_and_set_bit(flags + portno, &spe->todo))
		return;

	if (!schedule_delayed_work(&spe->work, spe->work_delay)){
        stat->cpu_kevent_cancel++;
    }else{
        stat->cpu_kevent_ok++;
    }
}

static void spe_rd_result_record(u32 result,struct spe_port_stat *stat)
{
	struct spe_rd_result_s *rp = (struct spe_rd_result_s *)&result;

    if( spe_balong.dbg_level & SPE_DBG_RD_RESULT){
		stat->rd_result[rp->result]++;
        stat->rd_reason[rp->drop_rsn]++;
		stat->rd_fw[ffz(rp->path)-1]++;
		stat->rd_indicate[rp->indicat]++;
		stat->rd_wrap[rp->done_rsn]++;
    }
}

void spe_rd_rewind(struct spe *spe, u32 portno)
{
    struct spe_port_ctrl *ctrl = &spe->ports[portno].ctrl;
    unsigned long flags;
    union spe_desc_queue_ptr rd_ptr;
    u32 rd_depth = ctrl->rd_depth;
    struct sk_buff *skb;
    struct spe_rd_desc *rd_base = (struct spe_rd_desc *)ctrl->rd_addr;
    u32 idx;

    spin_lock_irqsave(&ctrl->rd_lock, flags);
    /* get rdq ptr, include rd and wr ptr */
    rd_ptr.raw = spe_readl(spe->regs, SPE_RDQ_PTR_OFFSET(portno));
	if(rd_ptr.bits.wptr == rd_ptr.bits.rptr){/* no more rd, don't need rewind */
	    spin_unlock_irqrestore(&ctrl->rd_lock, flags);
		return;
	}

    /* rewind the wptr to rptr, and recycle the skb to free_q */
    for(idx = rd_ptr.bits.wptr; idx != rd_ptr.bits.rptr;){
		idx = (idx - 1) % rd_depth;
        skb = (struct sk_buff *)rd_base[idx].trb.skb_addr;
        skb_queue_tail(ctrl->rd_free_q, skb);
    }

	rd_ptr.bits.wptr = rd_ptr.bits.rptr;
	spe_writel(spe->regs,SPE_RDQ_PTR_OFFSET(portno),rd_ptr.raw);

    spin_unlock_irqrestore(&ctrl->rd_lock, flags);
    return;
}

int spe_rd_recycle(struct spe *spe, u32 portno)
{
    struct spe_port_ctrl *ctrl = &spe->ports[portno].ctrl;
    struct spe_port_stat *stat = &spe->ports[portno].stat;
    struct spe_rd_desc *rd_base = (struct spe_rd_desc *)ctrl->rd_addr;
    struct sk_buff *skb = NULL;
    union spe_desc_queue_ptr rd_ptr;
    u32 rd_depth = ctrl->rd_depth;
    u32 idx;
    u32 pkt_len;
    u32 port_enable;
    int ret = 0;
    unsigned long flags;
    struct spe_rd_trb* cur_trb;

    SPE_TRACE("enter\n");
    SPE_TRACE("rd port %d \n", portno);

    spin_lock_irqsave(&ctrl->rd_lock, flags);

    do{
        /* get rdq ptr, include rd and wr ptr */
	    rd_ptr.raw = spe_readl(spe->regs, SPE_RDQ_PTR_OFFSET(portno));
		if(ctrl->rd_busy == rd_ptr.bits.rptr){/* no more rd completed */
			break;
		}

        /* recycle rd from rd_busy to rdq rptr */
        for(idx = ctrl->rd_busy; idx != rd_ptr.bits.rptr;){
            cur_trb = &rd_base[idx].trb;

            skb = (struct sk_buff *)cur_trb->skb_addr;
            if (unlikely(!skb)) {
                stat->rd_skb_null++;
                goto finish_rd;
            }
            SPE_BUG(("skb->spe_own not set while portno(%d) != cpuport(%d)\n",
                portno,spe->cpuport.portno),
                !skb->spe_own && (portno != spe->cpuport.portno));

            pkt_len = (u32)cur_trb->pkt_len;

			spe->rd_success_byte += pkt_len;
            skb_put(skb, pkt_len);
            /*record spe rd timestemp debug info*/
            if(spe->dbg_level & SPE_DBG_RD_TIMESTAMP){
                spe_set_rd_timestamp((void *)(rd_base+idx));
            }
            idx = (idx + 1) % rd_depth;
            ctrl->rd_busy = idx;

            port_enable = (u32)(ctrl->flags & SPE_PORT_ENABLED);
            if (port_enable && ctrl->desc_ops.finish_rd) {
                /*just finsh spe trans, but not send to nic*/
                ret = ctrl->desc_ops.finish_rd(portno, cur_trb->sport_num,
                    skb, cur_trb->buf_addr, cur_trb->updata_only);
            }

            if (unlikely(!port_enable && ret)) {
                stat->rd_drop++;
                (void)spe_rd_config(portno, skb, cur_trb->buf_addr);
            }
            else {
                stat->rd_finished_bytes += skb->len;
            }

finish_rd:
            if(cur_trb->sport_num < SPE_PORT_NUM){
                stat->rd_sport[cur_trb->sport_num]++;
            }

            if(cur_trb->updata_only){
                cur_trb->updata_only = 0;
                stat->rd_upd_only++;
            }

            spe_rd_result_record(cur_trb->result, stat);

            stat->rd_finished++;

            cur_trb->buf_addr = 0;
            cur_trb->pkt_len = 0;
        }

    }while(!ret);

	if(ctrl->wport_push){
		ctrl->wport_push();
	}
	
    spin_unlock_irqrestore(&ctrl->rd_lock, flags);

    SPE_TRACE("leave\n");

    return ret;
}

int spe_rd_config(int portno, struct sk_buff *skb, dma_addr_t dma)
{
    struct spe *spe = &spe_balong;
    struct spe_port_ctrl *ctrl = &spe->ports[portno].ctrl;
    struct spe_port_stat *stat = &spe->ports[portno].stat;
    struct spe_rd_desc *desc;
    union spe_desc_queue_ptr rd_ptr;
    unsigned long flags;

    SPE_TRACE("enter\n");


    SPE_BUG(("skb(%#x) or dma(%#x) is null!",(unsigned int)skb,dma),(!skb)||(!dma));
	if(!skb){
		return -EFAULT;
	}

    spe_skb_refresh(skb);

    if(!spe_port_exist(portno)){
        SPE_ERR("config rd while port(%d) not exist\n",portno);
        return SPE_ERR_NOCHAN;
    }

    SPE_TRACE("port %d skb 0x%x \n",  portno, (unsigned int)skb);

    spin_lock_irqsave(&ctrl->rd_lock, flags);

    if(unlikely(spe_rd_full(portno))){
        spin_unlock_irqrestore(&ctrl->rd_lock, flags);
        stat->rd_full++;
        SPE_TRACE("out rd full \n");
        return SPE_ERR_RDFULL;
    }

    desc = (struct spe_rd_desc *)
        (ctrl->rd_addr + sizeof(struct spe_rd_desc)*ctrl->rd_free);

    desc->trb.buf_addr = dma;
    desc->trb.pkt_len = ctrl->rd_skb_size;
    desc->trb.skb_addr = (uint32_t)skb;
    desc->trb.irq_en = ctrl->rd_evt_gap?((ctrl->rd_free%ctrl->rd_evt_gap)?0:1):1;

    if(1 == desc->trb.irq_en){
        stat->rd_irq_en++;
    }

    ctrl->rd_free = (ctrl->rd_free + 1) % ctrl->rd_depth;

    rd_ptr.raw = spe_readl(spe->regs, SPE_RDQ_PTR_OFFSET(portno));
    rd_ptr.bits.wptr = ctrl->rd_free;
    spe_writel(spe->regs, SPE_RDQ_PTR_OFFSET(portno), rd_ptr.raw);


    stat->rd_config++;
    spe->stat.evt_rd++;

    spin_unlock_irqrestore(&ctrl->rd_lock, flags);

    SPE_TRACE("leave\n");

    return 0;
}
EXPORT_SYMBOL(spe_rd_config);

void spe_print_packet_stream(void)
{
    struct spe_port_ctrl *ctrl;
    struct spe_port_stat *stat;
	int i;

	for (i = 0; i < SPE_PORT_NUM; i++) {
		ctrl = &spe_balong.ports[i].ctrl;
		stat = &spe_balong.ports[i].stat;
	    printk("port[i]:%d ", ctrl->portno);
	}
	printk("\ntd: \n");

	for (i = 0; i < SPE_PORT_NUM; i++) {
		stat = &spe_balong.ports[i].stat;
	    printk("%d ", stat->td_config);
	}
	printk("\nrd: \n");

	for (i = 0; i < SPE_PORT_NUM; i++) {
		stat = &spe_balong.ports[i].stat;
	    printk("%d ", stat->rd_config);
	}
	printk("\ntd complete: \n");

	for (i = 0; i < SPE_PORT_NUM; i++) {
		stat = &spe_balong.ports[i].stat;
	    printk("%d ", stat->td_finsh_intr_complete);
	}
	printk("\nrd complete: \n");

	for (i = 0; i < SPE_PORT_NUM; i++) {
		stat = &spe_balong.ports[i].stat;
	    printk("%d ", stat->rd_finished);
	}
	printk("\n");

	return;
}
void spe_print_port_info(unsigned int portno)
{

	unsigned int i = portno;
    struct spe_port_ctrl *ctrl = &spe_balong.ports[i].ctrl;
    struct spe_port_stat *stat = &spe_balong.ports[i].stat;

    printk("port_num %u \n",    ctrl->portno);
	if(ctrl->net)
		printk("device name %s \n",    ctrl->net->name);

    printk("rd_addr 0x%x \n",	ctrl->rd_addr);
    printk("rd_depth 0x%x \n",	ctrl->rd_depth);
    printk("rd_evt_gap 0x%x \n",	ctrl->rd_evt_gap);
    printk("rd_dma 0x%x \n",	ctrl->rd_dma);
    printk("rd_skb_num 0x%x \n",	ctrl->rd_skb_num);
    printk("rd_skb_size 0x%x \n",	ctrl->rd_skb_size);
    printk("rd_skb_used 0x%x \n",	ctrl->rd_skb_used);
    printk("rd_skb_align 0x%x \n",	ctrl->rd_skb_align);
    printk("rd_busy 0x%x \n",	ctrl->rd_busy);
    printk("rd_free 0x%x \n",	ctrl->rd_free);
    printk("rd_send_success 0x%x \n",	stat->rd_send_success);
    printk("rd_net_null 0x%x \n",	stat->rd_net_null);
    printk("config_rd 0x%x \n",	stat->rd_config);
    printk("rd_full 0x%x \n",	stat->rd_full);
    printk("finsh rd intr complete 0x%x \n",	stat->rd_finsh_intr_complete);
    printk("rd_drop 0x%x \n",	stat->rd_drop);
    printk("config td 0x%x \n",	stat->td_config);
    printk("config td(port disabled) 0x%x \n",	stat->td_port_disabled);
    printk("td full 0x%x \n",	stat->td_full);
    printk("finsh td intr complete 0x%x \n",	stat->td_finsh_intr_complete);
    printk("td_addr 0x%x \n",	ctrl->td_addr);
    printk("td_dma_null 0x%x \n",	stat->td_dma_null);

    printk("td_depth 0x%x \n",	ctrl->td_depth);
    printk("td_evt_gap 0x%x \n",	ctrl->td_evt_gap);
    printk("td_dma 0x%x \n",	ctrl->td_dma);
    printk("td_busy 0x%x \n",	ctrl->td_busy);
    printk("td_free 0x%x \n",	ctrl->td_free);
}

MODULE_LICENSE("GPL");

