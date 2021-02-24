/*******************************************************************************
  This contains the functions to handle the normal descriptors.

  Copyright (C) 2007-2009  STMicroelectronics Ltd

 * 2016-2-18 - Modifed code to adapt Synopsys DesignWare Cores Ethernet 
 * Quality-of-Service (DWC_ether_qos) core, 4.10a.
 * liufangyuan <liufangyuan2@huawei.com>
 * Copyright (C) Huawei Technologies Co., Ltd.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
*******************************************************************************/

#include <linux/stmmac_avb.h>
#include "common.h"
#include "descs_com.h"
#include "stmmac.h"

static int ndesc_get_tx_status(void *data, struct stmmac_extra_stats *x,
			       union dma_desc *p, void __iomem *ioaddr, int chn)
{
	int ret = 0;
	struct net_device_stats *stats = (struct net_device_stats *)data;

	if (unlikely(p->tx_desc.nwb.err_sum)) {
		if (unlikely(p->tx_desc.nwb.underflow_err)) {
			x->tx_underflow++;
			stats->tx_fifo_errors++;
		}
		if (unlikely(p->tx_desc.nwb.no_carrier)) {
			x->tx_carrier++;
			stats->tx_carrier_errors++;
		}
		if (unlikely(p->tx_desc.nwb.loc)) {
			x->tx_losscarrier++;
			stats->tx_carrier_errors++;
		}
		if (unlikely((p->tx_desc.nwb.excessive_deferral) ||
			     (p->tx_desc.nwb.excessive_collision) ||
			     (p->tx_desc.nwb.late_collision)))
			stats->collisions += p->tx_desc.nwb.collision_count;
		ret = -EINVAL;
	}
	
#if 0
	if (p->tx_desc.nwb.vlan_frame) {
		CHIP_DBG(KERN_INFO "GMAC TX status: VLAN frame\n");
		x->tx_vlan[chn]++;
	}
#endif

	if (unlikely(p->tx_desc.nwb.deferred))
		x->tx_deferred++;

	return ret;
}

static int ndesc_get_tx_len(union dma_desc *p)
{
	return p->tx_desc.nrd.buf1_len;
}

/* This function verifies if each incoming frame has some errors
 * and, if required, updates the multicast statistics.
 * In case of success, it returns good_frame because the GMAC device
 * is supposed to be able to compute the csum in HW. */
static int ndesc_get_rx_status(void *data, struct stmmac_extra_stats *x,
			       union dma_desc *p)
{
	int ret = good_frame;
	struct net_device_stats *stats = (struct net_device_stats *)data;

	if (unlikely(p->rx_desc.nwb.sa_filter_fail)) {
		CHIP_DBG(KERN_ERR "GMAC RX : Source Address filter fail\n");
		x->sa_rx_filter_fail++;
		ret = discard_frame;
	}
	if (unlikely(p->rx_desc.nwb.da_filter_fail)) {
		CHIP_DBG(KERN_ERR "GMAC RX : Dest Address filter fail\n");
		x->da_rx_filter_fail++;
		ret = discard_frame;
	}
	
	if (unlikely(p->rx_desc.nwb.last_desc == 0)) {	// stub it, remember to change this!
		pr_warn("%s: Oversized frame spanned multiple buffers\n",
			__func__);
		stats->rx_length_errors++;
		return discard_frame;
	}

	if (unlikely(p->rx_desc.nwb.err_sum)) {
		if (unlikely(p->rx_desc.nwb.dribble_err))
			x->dribbling_bit++;
		if (unlikely(p->rx_desc.nwb.receive_err))
			x->rx_err++;
		if (unlikely(p->rx_desc.nwb.overflow_err)) 
			x->overflow_error++;
		if (unlikely(p->rx_desc.nwb.rw_timeout))
			x->rx_watchdog++;
		if (unlikely(p->rx_desc.nwb.giant_pkt))
			x->giant_pkt++;
		if (unlikely(p->rx_desc.nwb.crc_err)) {
			x->rx_crc++;
			stats->rx_crc_errors++;
		}
		ret = discard_frame;
	}
	
#ifdef STMMAC_VLAN_TAG_USED
	//if (p->des01.rx.vlan_tag)
		//x->vlan_tag++;
#endif 
	return ret;
}

static void ndesc_init_rx_desc(union dma_desc *p, int ioc_en)
{
	/* using buffer 1, no buffer 2 */
	p->rx_desc.nrd.own = 1;
	p->rx_desc.nrd.buf1_addr_valid = 1;
	p->rx_desc.nrd.buf2_addr_valid = 0;
	
	p->rx_desc.nrd.ioc = !(!ioc_en);
}

static void ndesc_init_tx_desc(union dma_desc *p)
{
	p->tx_desc.nrd.own = 0;
	p->tx_desc.nrd.ctx_type = 0;
}

static int ndesc_get_tx_owner(union dma_desc *p)
{
	return p->tx_desc.nrd.own;
}

static int ndesc_get_rx_owner(union dma_desc *p)
{
	return p->rx_desc.nrd.own;
}

static void ndesc_set_tx_owner(union dma_desc *p)
{
	p->tx_desc.nrd.own = 1;
}

static void ndesc_set_rx_owner(union dma_desc *p)
{
	p->rx_desc.nrd.own = 1;
}

static int ndesc_get_tx_ls(union dma_desc *p)
{
	return p->tx_desc.nrd.last_desc;
}

static void ndesc_release_tx_desc(union dma_desc *p)
{
	//int clean_size;
	//int ter = p->tx_desc.nwb.last_desc;

	//clean_size = 8;
	memset(p, 0, sizeof(union dma_desc));
	//ndesc_end_tx_desc_on_ring(p, ter);
}

static void ndesc_prepare_tx_desc(union dma_desc *p, int is_fs, int len,
				  int csum_flag)
{
	p->tx_desc.nrd.first_desc = is_fs;
	norm_set_tx_desc_len_on_ring(p, len);
	p->tx_desc.nrd.pkt_len = len;	//assume nfrags = 0
	if (likely(csum_flag))
		p->tx_desc.nrd.csum_ins_ctl = cic_full;
}

void ndesc_config_desc_slotnum(int chn, union dma_desc *desc)
{
	int slotnum;
	struct stmmac_priv *priv = gmac_priv;
	
	if (chn == TX_CHN_CLASS_A) {

			/* For audio channel, the DMA fetch descripters which slot number 
			 * is ahead of the reference slot number(RSN) by 1 slot, and its 
			 * send interval is 125us. So one descrpter's slotnum should ahead 
			 * of RSN by 2 slots and interval should be 1.
			*/
			slotnum = priv->hw->dma->get_tx_slotnum(priv->ioaddr, chn);
			slotnum =  (slotnum + 2) & SLOTNUM_MASK;
		} else if (chn == TX_CHN_CLASS_B) {

			/* For audio channel, the DMA fetch descripters which slot number
			 * is ahead of the reference slot number(RSN) by 1 slot, and its 
			 * send interval is 250us(represents 2 slots). So one descrpter's 
			 *slotnum should ahead of RSN by 3 slots and interval should be 2.
			*/
			slotnum = priv->hw->dma->get_tx_slotnum(priv->ioaddr, chn);
			slotnum = ((slotnum & EVENT_NUM_MASK) + 3) & SLOTNUM_MASK;	
		} else {
			return;
		}

		desc->tx_desc.nrd.slot_num = slotnum;	
}

static void ndesc_clear_tx_ic(union dma_desc *p)
{
	p->tx_desc.nrd.ioc = 0;
}

static void ndesc_close_tx_desc(union dma_desc *p)
{
	p->tx_desc.nrd.last_desc = 1;
	p->tx_desc.nrd.ioc = 1;
}

static int ndesc_get_rx_frame_len(union dma_desc *p )
{
	/* The type-1 checksum offload engines append the checksum at
	 * the end of frame and the two bytes of checksum are added in
	 * the length. Adjust for that in the framelen for type-1 checksum offload
	 * engines. */
	/*if (rx_coe_type == STMMAC_RX_COE_TYPE1)
		return p->des01.rx.frame_length - 2;
	else */
	
	return p->rx_desc.nwb.pkt_len;
}

static void ndesc_enable_tx_timestamp(union dma_desc *p)
{
	p->tx_desc.nrd.time_stamp_enable = 1;
}

static int ndesc_get_tx_timestamp_status(union dma_desc *p)
{
	return p->tx_desc.nwb.time_stamp_status;
}

static void ndesc_get_timestamp(void *desc, struct stmmac_time_spec *time)
{
	union dma_desc *p = (union dma_desc *)desc;

	time->nsec = p->tx_desc.nwb.tx_pkt_ts_low;
	time->sec = p->tx_desc.nwb.tx_pkt_ts_high;

}

static void ndesc_set_rx_ioc(union dma_desc *p, int ioc_en)
{
    if(ioc_en)
        p->rx_desc.nrd.ioc = 1;
}
static int ndesc_get_rx_timestamp_status(void *desc, u32 ats)
{
	//union dma_desc *p = (union dma_desc *)desc;

	//if ((p->des2 == 0xffffffff) && (p->des3 == 0xffffffff))
		/* timestamp is corrupted, hence don't store it */
		//return 0;
	//else
		return 1;
}

struct stmmac_desc_ops ndesc_ops = {
	.tx_status = ndesc_get_tx_status,
	.rx_status = ndesc_get_rx_status,
	.get_tx_len = ndesc_get_tx_len,
	.init_rx_desc = ndesc_init_rx_desc,
	.init_tx_desc = ndesc_init_tx_desc,
	.get_tx_owner = ndesc_get_tx_owner,
	.get_rx_owner = ndesc_get_rx_owner,
	.release_tx_desc = ndesc_release_tx_desc,
	.prepare_tx_desc = ndesc_prepare_tx_desc,
	.clear_tx_ic = ndesc_clear_tx_ic,
	.close_tx_desc = ndesc_close_tx_desc,
	.get_tx_ls = ndesc_get_tx_ls,
	.set_tx_owner = ndesc_set_tx_owner,
	.set_rx_owner = ndesc_set_rx_owner,
	.get_rx_frame_len = ndesc_get_rx_frame_len,
    .set_rx_ioc = ndesc_set_rx_ioc,
	.enable_tx_timestamp = ndesc_enable_tx_timestamp,
	.get_tx_timestamp_status = ndesc_get_tx_timestamp_status,
	.get_timestamp = ndesc_get_timestamp,
	.get_rx_timestamp_status = ndesc_get_rx_timestamp_status,
	.config_tx_slotnum = ndesc_config_desc_slotnum,
};
