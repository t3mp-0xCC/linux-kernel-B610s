/*******************************************************************************
  Header File to describe Normal/enhanced descriptor functions used for RING
  and CHAINED modes.

  Copyright(C) 2011  STMicroelectronics Ltd

 * 2016-2-18 - Modifed code to adapt Synopsys DesignWare Cores Ethernet 
 * Quality-of-Service (DWC_ether_qos) core, 4.10a. 
 * liufangyuan <liufangyuan2@huawei.com>
 * Copyright (C) Huawei Technologies Co., Ltd.
 
  It defines all the functions used to handle the normal/enhanced
  descriptors in case of the DMA is configured to work in chained or
  in ring mode.

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

#ifndef __DESC_COM_H__
#define __DESC_COM_H__

/* Specific functions used for Ring mode */

/* Normal descriptors */
static inline void ndesc_end_tx_desc_on_ring(union dma_desc *p, int ter)
{
	p->tx_desc.nwb.last_desc = ter;
}

static inline void norm_set_tx_desc_len_on_ring(union dma_desc *p, int len)
{
	if (unlikely(len > BUF_SIZE_2KiB)) {
		p->tx_desc.nrd.buf1_len = BUF_SIZE_2KiB - 1;
		p->tx_desc.nrd.buf2_len = len - p->tx_desc.nrd.buf1_len;
	} else
		p->tx_desc.nrd.buf1_len = len;
}

#endif /* __DESC_COM_H__ */
