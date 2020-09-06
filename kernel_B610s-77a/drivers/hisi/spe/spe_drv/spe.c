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


#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/ipv6.h>
#include <linux/dmapool.h>

#include <bsp_cpufreq.h>
#include <net/ipv6.h>

#include "spe.h"
#include "spe_entry.h"
#include "spe_desc.h"
#include "spe_dbg.h"


struct spe spe_balong = {
    .msg_level = SPE_MSG_ERR,
    .dbg_level = SPE_DBG_FWUDP|SPE_DBG_RD_RESULT|SPE_DBG_TD_RESULT,
    .evt_mask = {SPE_EVENT_COMP_ONLY,SPE_EVENT_EMPTY_FULL}
};

#define SPE_SKB_CB(__skb)	((struct spe_skb_cb *)&((__skb)->dma))


dma_addr_t spe_get_skb_dma(struct sk_buff *skb)
{
	return 	SPE_SKB_CB(skb)->dma;
}

void spe_set_skb_dma(struct sk_buff *skb, dma_addr_t dma)
{
	SPE_SKB_CB(skb)->dma = dma;
}

static void spe_pack_timeout_set(struct spe *spe)
{
    spe_writel(spe->regs, HI_SPE_PACK_MAX_TIME_OFFSET, spe->ncmwrap_timeout);
}

/* TODO:just open td done/rd done/rd empty. refill rd while rd empty */
void spe_intr_enable(struct spe *spe)
{
   uint32_t i;

   for (i = 0; i < spe->num_event_buffers; i++) {
        /* set intr timeout */
        spe_writel(spe->regs, SPE_INT_INTERVAL_OFFSET(i), spe->intr_timeout);
        spe_writel(spe->regs, SPE_EVENT_BUF_MASK_OFFSET(i), spe->evt_mask[i]);
   }
}

void spe_intr_disable(struct spe *spe)
{
    uint32_t i;

    for (i = 0; i < spe->num_event_buffers; i++) {
        spe_writel(spe->regs, SPE_EVENT_BUF_MASK_OFFSET(i), SPE_EVENT_DISABLE);
    }
}

void spe_aging_timer_set(struct spe *spe)
{
    HI_SPE_AGING_TIME_T aging_timer;

    aging_timer.bits.spe_ip_aging_time = spe->ipfw_timeout;
    aging_timer.bits.spe_mac_aging_time = spe->macfw_timeout;

    spe_writel(spe->regs, HI_SPE_AGING_TIME_OFFSET, aging_timer.u32);
}



int spe_enable(struct spe *spe)
{
    HI_SPE_EN_T value;


    value.u32 = spe_readl(spe->regs, HI_SPE_EN_OFFSET);
    value.bits.spe_en = 1;
    spe_writel(spe->regs, HI_SPE_EN_OFFSET, value.u32);

    spe_intr_enable(spe);

    spe->flags |= SPE_FLAG_ENABLE;

    return 0;
}

int spe_disable(struct spe *spe)
{
    HI_SPE_EN_T value;
    int32_t ret = 0;

    /* disable spe hw */
    value.u32 = spe_readl(spe->regs, HI_SPE_EN_OFFSET);
    value.bits.spe_en = 0;
    spe_writel(spe->regs, HI_SPE_EN_OFFSET, value.u32);

    spe_intr_disable(spe);

    spe->flags &= (~SPE_FLAG_ENABLE);

    return ret;
}

void spe_dbgen_enable(void){
	struct spe *spe = &spe_balong;
    HI_SPE_DBGEN_T spe_dbg;
	
	spe_dbg.u32 = spe_readl(spe->regs, HI_SPE_DBGEN_OFFSET);
	spe_dbg.bits.spe_dbgen = 1;
	spe_writel(spe->regs, HI_SPE_DBGEN_OFFSET, spe_dbg.u32);
}

void spe_dbgen_disable(void){
	struct spe *spe = &spe_balong;
    HI_SPE_DBGEN_T spe_dbg;
	
	spe_dbg.u32 = spe_readl(spe->regs, HI_SPE_DBGEN_OFFSET);
	spe_dbg.bits.spe_dbgen = 0;
	spe_writel(spe->regs, HI_SPE_DBGEN_OFFSET, spe_dbg.u32);
}


uint32_t spe_timestamp_get(void)
{
    struct spe *spe = &spe_balong;

    return spe_readl(spe->regs, HI_SPE_TIME_OFFSET);
}

static struct spe_event_buffer *
spe_alloc_one_event_buffer(struct spe *spe, unsigned int buffer_no)
{
	struct spe_event_buffer	*evt;

	if (spe->event_buf_left <= 0) {
	    SPE_ERR("no event buffer left for event buffer:%d\n", buffer_no);
	    return ERR_PTR(-ENOMEM);
	}

	evt = kzalloc(sizeof(*evt), GFP_KERNEL);
	if (!evt) {
		SPE_ERR("alloc event failed\n");
		return ERR_PTR(-ENOMEM);

	}

	evt->spe	= spe;
	evt->length	= SPE_EVENT_BUFFER_DFT_NUM;
	spe->event_buf_left -= evt->length;

	return evt;
}

static int spe_alloc_event_buffers(struct spe *spe)
{
	//int	num;
	int	i;

	spe->num_event_buffers = SPE_EVENT_BUFFERS_USED;
	spe->event_buf_left = SPE_EVENT_BUFFER_TOTAL_NUM;
	//num = SPE_EVENT_BUFFERS_MAX;

	spe->ev_buffs = kzalloc(sizeof(*spe->ev_buffs) * SPE_EVENT_BUFFERS_MAX, GFP_KERNEL);
	if (!spe->ev_buffs) {
		SPE_ERR("can't allocate event buffers array\n");
		return -ENOMEM;
	}

	for (i = 0; i < SPE_EVENT_BUFFERS_USED; i++) {

        struct spe_event_buffer *evt;
		evt = spe_alloc_one_event_buffer(spe, i);
		if (IS_ERR(evt)) {
			SPE_ERR("can't allocate event buffer\n");
			return -ENOMEM;
		}
		spe->ev_buffs[i] = evt;
	}

	return 0;
}

static void spe_event_buffers_setup(struct spe *spe)
{
	struct spe_event_buffer	*evt;
	int				n;
	HI_SPE_EVENT_BUFF_LEN_T evt_len;
    HI_SPE_EVENT_BUFF0_PTR_T evt_ptr;

	for (n = 0; n < spe->num_event_buffers; n++) {

        /*
         * bit[0-10]:event0, bit[16-26]:event1,
         */
        evt = spe->ev_buffs[n];
        evt_len.u32 = spe_readl(spe->regs, SPE_EVENT_BUFF_LEN_OFFSET);
        evt_len.u32 &= (~(SPE_EVENT_BUFFER_NUM_BITS << (n* SPE_EVENT_BUFFER_NUM_BIT_CNT)));
        evt_len.u32 |= ((evt->length & SPE_EVENT_BUFFER_NUM_MASK) << (n*SPE_EVENT_BUFFER_NUM_BIT_CNT));
        spe_writel(spe->regs, SPE_EVENT_BUFF_LEN_OFFSET, evt_len.u32);

		spe_writel(spe->regs, SPE_EVENTBUF_PRO_CNT_OFFSET(n), 0);

    	evt->buf = spe->regs + spe_readl(spe->regs, SPE_EVENT_BUF_ADDR_OFFSET(n));
    	printk("new_reg: event buffer(%d) addr 0x%x\n", n, (unsigned int)evt->buf);

        /* be care, restore the event pos from hardware */
        evt_ptr.u32 = spe_readl(spe->regs, SPE_EVENT_BUF_PTR_OFFSET(n));
        evt->lpos = evt_ptr.bits.spe_event_buff0_rptr;
        SPE_ERR("%s, restore event lpos:%d\n", __func__, evt->lpos);
	}

	return ;
}

static void spe_free_one_event_buffer(struct spe *spe,
		struct spe_event_buffer *evt)
{
	//dma_free_coherent(spe->dev, evt->length, evt->buf, evt->dma);
	kfree(evt);
}

static void spe_free_event_buffers(struct spe *spe)
{
	struct spe_event_buffer	*evt;
	int i;

    /* event buffers already free, do nothing */
    if (!spe->ev_buffs) {
        return;
    }
	for (i = 0; i < spe->num_event_buffers; i++) {
		evt = spe->ev_buffs[i];
		if (evt)
			spe_free_one_event_buffer(spe, evt);
	}

	kfree(spe->ev_buffs);
	spe->ev_buffs = NULL;
}

static void spe_td_result_record(u32 result, struct spe_port_stat *stat)
{
	struct spe_td_result_s *tp = (struct spe_td_result_s *)&result;

    if (spe_balong.dbg_level & SPE_DBG_TD_RESULT) {

		stat->td_result[tp->result]++;
		stat->td_reason[tp->reason]++;
		stat->td_fw[ffz(tp->path)-1]++;
		stat->td_indicate[tp->indicat]++;
		stat->td_ncm[tp->drop]++;
	 	stat->td_wrap[tp->error]++;
    }
}

int spe_td_complete(struct spe *spe, union spe_event *event)
{
    uint32_t portno = (uint32_t)event->bits.portno;
    struct spe_port_ctrl *ctrl = &spe->ports[portno].ctrl;
    struct spe_port_stat *stat = &spe->ports[portno].stat;
    struct spe_td_desc *td_base = (struct spe_td_desc *)ctrl->td_addr;
    union spe_desc_queue_ptr td_ptr;
    uint32_t td_depth = ctrl->td_depth;
    uint32_t idx;

    SPE_TRACE("enter\n");
	SPE_TRACE("event port: %d\n", portno);

    spin_lock(&ctrl->td_lock);

	do{
        /* get tdq ptr, include rd and wr ptr */
	    td_ptr.raw = spe_readl(spe->regs, SPE_TDQ_PTR_OFFSET(portno));

		SPE_TRACE("td_busy: %d rptr: 0x%x\n", ctrl->td_busy, td_ptr.bits.rptr);

		if(ctrl->td_busy == td_ptr.bits.rptr)/* no more td completed */
		{
			break;
		}

        /* recycle td from td_busy to tdq rptr */
        for(idx = ctrl->td_busy; idx != td_ptr.bits.rptr; idx = (idx + 1) % td_depth){
			ctrl->desc_ops.finish_td(portno, (struct sk_buff *)td_base[idx].trb.skb_addr ,0);
            spe_td_result_record(td_base[idx].trb.result,stat);
            stat->td_finsh_intr_complete++;
            td_base[idx].trb.buf_addr = 0;
            td_base[idx].trb.pkt_len = 0;
        }

        ctrl->td_busy = idx;
	}while(1);

    spin_unlock(&ctrl->td_lock);

	SPE_TRACE("leave\n");

    return 0;
}

int spe_rd_complete(struct spe *spe, union spe_event *event)
{
    uint32_t portno = event->bits.portno;

    return spe_rd_recycle(spe, portno);
}

static void spe_process_event_entry(struct spe *spe, union spe_event *event)
{
	SPE_TRACE("enter\n");

    spe->stat.evt_stat[event->bits.etype]++;

	switch(event->bits.etype)
	{
		case EVENT_TD_COMPLETE:
			spe->stat.evt_td_complt[event->bits.portno]++;
			if(unlikely(!SPE_NORM_PORT(event->bits.portno))){
				spe->stat.evt_td_errport++;
			}

			(void)spe_td_complete(spe, event);
			break;

		case EVENT_TD_EMPTY:
			spe->stat.evt_td_empty[event->bits.portno]++;
			if(unlikely(!SPE_NORM_PORT(event->bits.portno))){
				spe->stat.evt_td_errport++;
			}
			break;

		case EVENT_TD_FULL:
			spe->stat.evt_td_full[event->bits.portno]++;
			if(unlikely(!SPE_NORM_PORT(event->bits.portno))){
				spe->stat.evt_td_errport++;
			}
			break;


		case EVENT_RD_COMPLETE:
			spe->stat.evt_rd_complt[event->bits.portno]++;
			if(unlikely(!SPE_NORM_PORT(event->bits.portno))){
				spe->stat.evt_rd_errport++;
			}

			(void)spe_rd_complete(spe, event);
			break;


		case EVENT_RD_EMPTY:
			spe->stat.evt_rd_empty[event->bits.portno]++;
			if(unlikely(!SPE_NORM_PORT(event->bits.portno))){
				spe->stat.evt_rd_errport++;
			}

			break;

		case EVENT_RD_FULL:
			spe->stat.evt_rd_full[event->bits.portno]++;
			if(unlikely(!SPE_NORM_PORT(event->bits.portno))){
				spe->stat.evt_rd_errport++;
			}

			break;

		default:
			spe->stat.evt_unknown++;
			SPE_ERR("unknow etype:%d current event:%#x\n",
                event->bits.etype, event->raw);
			break;
	}

	SPE_TRACE("leave\n");

    return ;
}

static irqreturn_t spe_process_event_buf(struct spe *spe, uint32_t buf)
{
	struct spe_event_buffer *evt;
	int left;
	uint32_t count;
	irqreturn_t ret = IRQ_NONE;

	SPE_TRACE("enter\n");

	while(1)
	{
		count = spe_readl(spe->regs, SPE_EVENTBUF_RPT_CNT_OFFSET(buf));
		count &= SPE_GEVNTCOUNT_MASK;
		SPE_TRACE("count: %d\n", count);
		if (!count)
			break;
		evt = spe->ev_buffs[buf];
		left = count;
		while (left > 0) {
			union spe_event event;
			event.raw = spe_readl(evt->buf, evt->lpos*4);
			SPE_TRACE("event.raw:%d\n", event.raw);

			spe_process_event_entry(spe, &event);

			evt->lpos = (evt->lpos + 1) % (evt->length);
			left -= 1;
		}

		spe_writel(spe->regs, SPE_EVENTBUF_PRO_CNT_OFFSET(buf), count);

		ret = IRQ_HANDLED;
	}

	if (spe_readl(spe->regs, SPE_EVENT_BUFF_RD_ERR_OFFSET(buf))){
		SPE_BUG(("event buf rd err!\n"),1);
		spe->stat.evt_buf_rd_err++;
		/* SPE_EVENT_BUFF_RD_ERR_OFFSET is w1c */
		spe_writel(spe->regs, SPE_EVENT_BUFF_RD_ERR_OFFSET(buf), 0x1);
	}

	SPE_TRACE("leave\n");

	return ret;
}

static irqreturn_t spe_interrupt(int irq, void *dev_id)
{
	struct spe *spe = (struct spe *)dev_id;
	irqreturn_t ret = IRQ_NONE;
	irqreturn_t status;
    HI_SPE_INT_STATE_T event_src;

	SPE_TRACE("enter\n");

	event_src.u32 = spe_readl(spe->regs, HI_SPE_INT_STATE_OFFSET);

	if(event_src.bits.spe_event_buff0_int_state){
		spe->stat.evt_0_stat++;
		status = spe_process_event_buf(spe, BUFFER_0_EVENT);
		if (status == IRQ_HANDLED)
			ret = status;
	}

	if(event_src.bits.spe_event_buff1_int_state){
		spe->stat.evt_1_stat++;
		status = spe_process_event_buf(spe, BUFFER_1_EVENT);
		if (status == IRQ_HANDLED)
			ret = status;
	}

	SPE_TRACE("leave\n");

	return ret;
}

int spe_set_globle_usr_field(int no)
{
    struct spe *spe = &spe_balong;
    HI_SPE_USRFIELD_CTRL_T field;

    field.u32 = spe_readl(spe->regs, HI_SPE_USRFIELD_CTRL_OFFSET);
    field.bits.spe_usrfield_ctrl = no;
    spe_writel(spe->regs, HI_SPE_USRFIELD_CTRL_OFFSET, field.u32);

    return 0;
}
int spe_is_enable(void)
{
    struct spe *spe = &spe_balong;

    return (spe->flags & SPE_FLAG_ENABLE);
}

bool spe_skb_is_updateonly(struct sk_buff *skb)
{
    return skb->update_only;
}

int spe_is_ipf_port(unsigned int port_num)
{
    struct spe *spe = &spe_balong;

    return (spe_enc_ipf == spe->ports[port_num].ctrl.property.bits.spe_port_enc_type);
}

int spe_mode(void)
{
    struct spe *spe = &spe_balong;

    return spe->spe_mode;
}
void spe_hook_register(void)
{
    spe_hook.is_enable = spe_is_enable;
	spe_hook.ipfw_entry_add = spe_ip_fw_add;
    spe_hook.ipfw_entry_del = spe_ip_fw_del;
	spe_hook.macfw_entry_add = spe_mac_fw_add;
    spe_hook.macfw_entry_del = spe_mac_fw_del;
    spe_hook.port_alloc = spe_port_alloc;
    spe_hook.port_free = spe_port_free;
    spe_hook.port_ioctl = spe_port_ioctl;
    spe_hook.port_is_enable = spe_port_is_enable;
    spe_hook.port_enable = spe_port_enable;
    spe_hook.port_disable = spe_port_disable;
    spe_hook.port_netdev = spe_port_netdev;
    spe_hook.is_updateonly = spe_skb_is_updateonly;
    spe_hook.get_skb_dma = spe_get_skb_dma;
    spe_hook.set_skb_dma = spe_set_skb_dma;
    spe_hook.td_config = spe_td_config;
    spe_hook.rd_config = spe_rd_config;
    spe_hook.set_ipf_mac = spe_set_ipf_mac;
    spe_hook.set_port_property = spe_port_set_property;
    spe_hook.mode = spe_mode;
    //spe_hook.modify_cpuport_ops = spe_set_bypass_ops;
}

void spe_hook_unregister(void)
{
    spe_hook.is_enable = NULL;
	spe_hook.ipfw_entry_add = NULL;
    spe_hook.ipfw_entry_del = NULL;
	spe_hook.macfw_entry_add = NULL;
    spe_hook.macfw_entry_del = NULL;
    spe_hook.port_alloc = NULL;
    spe_hook.port_free = NULL;
    spe_hook.port_ioctl = NULL;
    spe_hook.port_enable = NULL;
    spe_hook.port_disable = NULL;
    spe_hook.port_netdev = NULL;
    spe_hook.is_updateonly = NULL;
    spe_hook.get_skb_dma = NULL;
    spe_hook.set_skb_dma = NULL;
    spe_hook.td_config = NULL;
    spe_hook.rd_config = NULL;
    spe_hook.set_ipf_mac = NULL;
    spe_hook.set_port_property = NULL;
    spe_hook.mode = NULL;
    //spe_hook.modify_cpuport_ops = NULL;
}

void spe_filter_init_config(struct spe *spe)
{
	HI_SPE_BLACK_WHITE_T list;
	list.bits.spe_mac_black_white = 1;
	list.bits.spe_ip_black_white = 1;
	spe_writel(spe->regs, HI_SPE_BLACK_WHITE_OFFSET, list.u32);

}
int spe_core_init(struct spe *spe)
{
	int ret;
    int i;

    SPE_TRACE("enter\n");
	snprintf(spe->name, 32, dev_name(spe->dev));
	snprintf(spe->compile, 64, "compiled at " __DATE__" "__TIME__);

	if(!spe->ipfw.hsize){
        //BUILD_BUG_ON_NOT_POWER_OF_2(SPE_IPFW_HTABLE_SIZE);  /* should be 2^n */
        spe->ipfw.hsize = SPE_IPFW_HTABLE_SIZE;
    }

    spe->ipfw.hrand = SPE_IPFW_HTABLE_RAND;
    spe->ipfw.hzone = SPE_IPFW_HTABLE_ZONE;

    /* alloc spe hash bucket */
    spe->ipfw.hbucket = dma_alloc_coherent(spe->dev,
        sizeof(struct spe_ip_fw_entry)*spe->ipfw.hsize,
        &spe->ipfw.hbucket_dma, GFP_KERNEL);
    if(!spe->ipfw.hbucket){
        SPE_ERR("alloc ip fw hbucket of %d entrys failed\n",
            spe->ipfw.hsize);
        ret = -ENOMEM;
        goto err_alloc_hbucket;
    }

    for(i=0; i < spe->ipfw.hsize; i++){
		SPE_INFO("hbucket[%d]: %p\n", i, &spe->ipfw.hbucket[i]);
        INIT_LIST_HEAD(&spe->ipfw.hbucket[i].list);
        spe->ipfw.hbucket[i].dead_timestamp = jiffies;
        spe->ipfw.hbucket[i].dma = spe->ipfw.hbucket_dma +
            i*sizeof(struct spe_ip_fw_entry);
    }

    /* alloc dma pool for ip fw entry */
    spe->ipfw.hslab = (struct dma_pool *)dma_pool_create(dev_name(spe->dev),
        spe->dev,sizeof(struct spe_ip_fw_entry), 4, PAGE_SIZE);
    if(!spe->ipfw.hslab){
        SPE_ERR("alloc ip fw hslab failed\n");
        ret = -ENOMEM;
        goto err_alloc_hslab;
    }

    spe->min_pkt_len = SPE_MIN_PKT_SIZE;

    INIT_LIST_HEAD(&spe->ipfw.free_list);
    spin_lock_init(&spe->ipfw.lock);
    spin_lock_init(&spe->ipfw.free_lock);
    spin_lock_init(&spe->port_alloc_lock);

    spe->ipfw.deadtime = 2; /* jiffies */

    /* alloc pool for mac fw entry */
    spe->macfw.slab = (struct kmem_cache *)kmem_cache_create(dev_name(spe->dev),
        sizeof(struct spe_mac_fw_entry_ext), 0, SLAB_HWCACHE_ALIGN, NULL);
    if(!spe->macfw.slab){
        SPE_ERR("alloc mac fw slab failed\n");
        ret = -ENOMEM;
        goto err_alloc_macfwslab;
    }

    INIT_LIST_HEAD(&spe->macfw.pending);
    INIT_LIST_HEAD(&spe->macfw.backups);
    spin_lock_init(&spe->macfw.lock);

	spe->mask_flags = 0;
	
	ret = spe_alloc_event_buffers(spe);
	if (ret) {
		SPE_ERR("failed to allocate event buffers\n");
		ret = -ENOMEM;
		goto err_alloc_eventbuf;
	}

    spe_event_buffers_setup(spe);

    spe_intr_disable(spe);

    ret = request_irq(spe->irq, spe_interrupt, IRQF_SHARED,
			spe->name, spe);
    if (ret) {
		SPE_ERR("failed to request irq #%d --> %d\n",
				spe->irq, ret);
		goto err_request_irq;
	}

    spe->ipfw_usr_field =
        (sizeof(struct spe_ip_fw_entry) - offsetof(struct spe_ip_fw_entry,dma))/4;
    spe->desc_usr_field =
        (sizeof(struct spe_rd_desc) - offsetof(struct spe_rd_desc,usr_field1))/4;
    spe->ipfw_timeout = SPE_IP_FW_TIMEOUT;
    spe->macfw_timeout = SPE_MAC_FW_TIMEOUT;
    spe->intr_timeout = SPE_INTR_TIMEOUT;
    spe->ncmwrap_timeout = SPE_NCM_WRAP_TIMEOUT;
    spe->stick_ctrl.src_port = 0;
    spe->stick_ctrl.src_status = 0;
    spe->stick_ctrl.dst_port = 0;
    spe->stick_ctrl.dst_status = 0;

    /* set user field */


    /* set timeout */
    spe_pack_timeout_set(spe);

    /* set ip fw hash bucket base and depth ,width unit is word*/
    spe_ip_fw_htab_set(spe);

    /* set ip fw mac fw agint timer*/
    spe_aging_timer_set(spe);

    /* int workqueue */
	spe->work_delay = SPE_WORK_DEF_DELAY;

	spe_set_globle_usr_field(spe->desc_usr_field);
    spe->flags = 0;

	spe_writel(spe->regs, HI_SPE_ETH_MINLEN_OFFSET,spe->min_pkt_len);
    /* spe hw need each spe channel bind unique portno before spe enable. */
    for(i=0; i<SPE_PORT_NUM; i++){
        spe_writel(spe->regs, SPE_PORT_NUM_OFFSET(i), i);

		/*set spe limit rate to the max*/
		spe_writel(spe->regs, SPE_CH_UDP_LIMIT_CNT_OFFSET(i), SPE_PORT_UDP_LIM_MAX);
		spe_writel(spe->regs, SPE_CH_RATE_LIMIT_BYTE_OFFSET(i), SPE_PORT_BYTE_LIM_MAX);		
    }

    spe_filter_init_config(spe);
	

    SPE_TRACE("leave\n");

	return 0;

err_request_irq:
    spe_free_event_buffers(spe);

err_alloc_eventbuf:
    if(spe->macfw.slab){
        kmem_cache_destroy(spe->macfw.slab);
        spe->macfw.slab = NULL;
    }

err_alloc_macfwslab:
    if(spe->ipfw.hslab){
        dma_pool_destroy(spe->ipfw.hslab);
        spe->ipfw.hslab = NULL;
    }

err_alloc_hslab:
    if(spe->ipfw.hbucket){
        dma_free_coherent(spe->dev,
            sizeof(struct spe_ip_fw_entry)*spe->ipfw.hsize,
            spe->ipfw.hbucket,spe->ipfw.hbucket_dma);
        spe->ipfw.hbucket = NULL;
    }

err_alloc_hbucket:
	return ret;
}

int spe_core_exit(struct spe *spe)
{
    unsigned long flags;

    SPE_TRACE("enter\n");

    if(spe->irq){
        free_irq(spe->irq, spe);
        spe->irq = 0;
    }

    flush_delayed_work(&spe->work);

    spe_free_event_buffers(spe);

    /* free node in free queue */
    {
        struct spe_ip_fw_entry *pos, *n;
        int i;

        spin_lock_irqsave(&spe->ipfw.lock, flags);

        for(i=0; i < spe->ipfw.hsize; i++){
            list_for_each_entry_safe(pos, n, &spe->ipfw.hbucket[i].list, list){
                list_del_init(&pos->list);
                dma_pool_free(spe->ipfw.hslab, pos, pos->dma);
            }
        }

        spin_unlock_irqrestore(&spe->ipfw.lock, flags);

        spin_lock_irqsave(&spe->ipfw.free_lock, flags);

        list_for_each_entry_safe(pos, n, &spe->ipfw.free_list, list){
            dma_pool_free(spe->ipfw.hslab, pos, pos->dma);
        }

        spin_unlock_irqrestore(&spe->ipfw.free_lock, flags);
    }

    if(spe->ipfw.hslab){
        dma_pool_destroy(spe->ipfw.hslab);
        spe->ipfw.hslab = NULL;
    }

    if(spe->ipfw.hbucket){
        dma_free_coherent(spe->dev,
            sizeof(struct spe_ip_fw_entry)*spe->ipfw.hsize,
            spe->ipfw.hbucket,spe->ipfw.hbucket_dma);
        spe->ipfw.hbucket = NULL;
    }

	if(spe->ipfw.hbucket_empty){
		kfree(spe->ipfw.hbucket_empty);
	}

    /* free node in pending queue */
    {
        struct spe_mac_fw_entry_ext *pos;
        struct spe_mac_fw_entry_ext *n;

        spin_lock_irqsave(&spe->macfw.lock, flags);
        list_for_each_entry_safe(pos, n, &spe->macfw.pending, list){
            list_del_init(&pos->list);
            kmem_cache_free(spe->macfw.slab, (void*)pos);
        }
        list_for_each_entry_safe(pos, n, &spe->macfw.backups, list){
            list_del_init(&pos->list);
            kmem_cache_free(spe->macfw.slab, (void*)pos);
        }
        spin_unlock_irqrestore(&spe->macfw.lock, flags);
    }

    if(spe->macfw.slab){
        kmem_cache_destroy(spe->macfw.slab);
        spe->macfw.slab = NULL;
    }

    SPE_TRACE("exit\n");

    return 0;
}

MODULE_ALIAS("hisilicon network hardware accelerator driver");
MODULE_AUTHOR("hisi bsp4 network");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("smart packet engine(spe) driver");

