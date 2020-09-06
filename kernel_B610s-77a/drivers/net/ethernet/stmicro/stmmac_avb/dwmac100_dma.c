/*******************************************************************************
  This is the driver for the MAC 10/100 on-chip Ethernet controller
  currently tested on all the ST boards based on STb7109 and stx7200 SoCs.

  DWC Ether MAC 10/100 Universal version 4.0 has been used for developing
  this code.

  This contains the functions to handle the dma.

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

#include <asm/io.h>
#include "dwmac100.h"
#include "dwmac_dma.h"

static int dwmac100_dma_init(void __iomem *ioaddr, int pbl, int fb, int mb,
			     int burst_len, u32 dma_tx, u32 dma_rx, int atds)
{
	u32 value = readl(ioaddr + DMA_BUS_MODE);
	int limit;

	/* DMA SW reset */
	value |= DMA_BUS_MODE_SFT_RESET;
	writel(value, ioaddr + DMA_BUS_MODE);
	
	limit = 100;
	while (limit--) {
		if (!(readl(ioaddr + DMA_BUS_MODE) & DMA_BUS_MODE_SFT_RESET))
			break;
		mdelay(10);
	}
	if (limit < 0)
		return -EBUSY;

	/* Enable Application Access by writing to DMA CSR0 */
	writel(DMA_BUS_MODE_DEFAULT | (pbl << DMA_BUS_MODE_PBL_SHIFT),
	       ioaddr + DMA_BUS_MODE);

	/* Mask interrupts by writing to CSR7 */
	writel(DMA_INTR_DEFAULT_MASK, ioaddr + DMA_CHN_INT_EN(0));

	/* RX/TX descriptor base addr lists must be written into
	 * DMA CSR3 and CSR4, respectively
	 */
	writel(dma_tx, ioaddr + DMA_CHN_TD_LADDR(0));
	writel(dma_rx, ioaddr + DMA_CHN_RD_LADDR(0));

	return 0;
}

/* Store and Forward capability is not used at all.
 *
 * The transmit threshold can be programmed by setting the TTC bits in the DMA
 * control register.
 */
static void dwmac100_dma_operation_mode(void __iomem *ioaddr, int txmode,
					int rxmode)
{
	u32 csr6 = readl(ioaddr + DMA_CHN_CTL(0));

	if (txmode <= 32)
		csr6 |= DMA_CONTROL_TTC_32;
	else if (txmode <= 64)
		csr6 |= DMA_CONTROL_TTC_64;
	else
		csr6 |= DMA_CONTROL_TTC_128;

	writel(csr6, ioaddr + DMA_CHN_CTL(0));
}

static void dwmac100_dump_dma_regs(void __iomem *ioaddr)
{
	int i;

	CHIP_DBG(KERN_DEBUG "DWMAC 100 DMA CSR\n");
	for (i = 0; i < 9; i++)
		pr_debug("\t CSR%d (offset 0x%x): 0x%08x\n", i,
			 (DMA_BUS_MODE + i * 4),
			 readl(ioaddr + DMA_BUS_MODE + i * 4));
	CHIP_DBG(KERN_DEBUG "\t CSR20 (offset 0x%x): 0x%08x\n",
		 DMA_CUR_TX_BUF_ADDR, readl(ioaddr + DMA_CHN_CUR_APP_TB(0)));
	CHIP_DBG(KERN_DEBUG "\t CSR21 (offset 0x%x): 0x%08x\n",
		 DMA_CUR_RX_BUF_ADDR, readl(ioaddr + DMA_CHN_CUR_APP_RB(0)));
}

/* DMA controller has two counters to track the number of the missed frames. */
static void dwmac100_dma_diagnostic_fr(void *data, struct stmmac_extra_stats *x,
				       void __iomem *ioaddr)
{
	struct net_device_stats *stats = (struct net_device_stats *)data;
	u32 csr8 = readl(ioaddr + DMA_CHN_MFRAME_CNT(0));

	if (unlikely(csr8)) {
		if (csr8 & DMA_MISSED_FRAME_OVE) {
			stats->rx_over_errors += 0x800;
			x->rx_overflow_cntr += 0x800;
		} else {
			unsigned int ove_cntr;
			ove_cntr = ((csr8 & DMA_MISSED_FRAME_OVE_CNTR) >> 17);
			stats->rx_over_errors += ove_cntr;
			x->rx_overflow_cntr += ove_cntr;
		}

		if (csr8 & DMA_MISSED_FRAME_OVE_M) {
			stats->rx_missed_errors += 0xffff;
			x->rx_missed_cntr += 0xffff;
		} else {
			unsigned int miss_f = (csr8 & DMA_MISSED_FRAME_M_CNTR);
			stats->rx_missed_errors += miss_f;
			x->rx_missed_cntr += miss_f;
		}
	}
}

const struct stmmac_dma_ops dwmac100_dma_ops = {
	.init = dwmac100_dma_init,
	.dump_regs = dwmac100_dump_dma_regs,
	.dma_mode = dwmac100_dma_operation_mode,
	.dma_diagnostic_fr = dwmac100_dma_diagnostic_fr,
	.enable_dma_transmission = dwmac_enable_dma_transmission,
	.enable_dma_irq = dwmac_enable_dma_irq,
	.disable_dma_irq = dwmac_disable_dma_irq,
	.start_tx = dwmac_dma_start_tx,
	.stop_tx = dwmac_dma_stop_tx,
	.start_rx = dwmac_dma_start_rx,
	.stop_rx = dwmac_dma_stop_rx,
	.dma_interrupt = dwmac_dma_interrupt,
};