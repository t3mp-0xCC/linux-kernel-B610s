/*******************************************************************************
  This is the driver for the GMAC on-chip Ethernet controller for ST SoCs.
  DWC Ether MAC 10/100/1000 Universal version 3.41a  has been used for
  developing this code.

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
#include <linux/bitops.h>
#include <linux/types.h>
#include "dwmac1000.h"
#include "dwmac_dma.h"
#include "stmmac.h"

static dma_addr_t dwmac1000_get_rx_tail_ptr(unsigned int index, int chn)
{
	dma_addr_t dma_addr;
	struct stmmac_priv *priv = gmac_priv;
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, chn);
		
	dma_addr = rx_queue->dma_rx_phy + (index * sizeof(union dma_desc));
	return dma_addr;
}

static void dwmac1000_set_rx_tail_ptr(unsigned int index, int chn)
{
	dma_addr_t dma_addr;
	struct stmmac_priv *priv = gmac_priv;

	dma_addr = priv->rx_queue[chn].dma_rx_phy + (index * sizeof(union dma_desc));
	writel(dma_addr, priv->ioaddr + DMA_CHN_RD_TAIL_PTR(chn));
}

static int dwmac1000_get_tx_slotnum(void __iomem *ioaddr, int chn)
{
	int slotnum;

	slotnum = readl(ioaddr + DMA_CHN_SFCS(chn));
	slotnum = (slotnum >> DMA_SFCS_RSN_POS) & SLOTNUM_MASK ;

	return slotnum;
}

static int dwmac1000_dma_init(int fb, int mb,int burst_len)
{
	void *ioaddr;
	u32 value;
	int limit;
	int chn;
	unsigned int index;
	dma_addr_t rx_tail;
	struct stmmac_priv *priv = gmac_priv;
	struct gmac_tx_queue *tx_queue = NULL;
	struct gmac_rx_queue *rx_queue = NULL;

	ioaddr = priv->ioaddr;
	value =  readl(ioaddr + DMA_BUS_MODE);
	
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

	/* Transmit Arbitration Algorithm is fixed priority, so we using default value:0.
	*   Interrupt mode:INTM=1
	*   DA=1, fixed priority arbitration scheme.
	*/
	value = readl(ioaddr + DMA_BUS_MODE);
	value |= DMA_BUS_MODE_INTM | DMA_BUS_MODE_DA |DMA_BUS_MODE_TAA;
	writel(value, ioaddr + DMA_BUS_MODE);
	
	/* AAL=0, using BLEN16, don't reubild burst
	 *  wr_osr_lmt = rd_osr_lmt = 3
	*/
	value = readl(ioaddr + DMA_SYSBUS_MODE);	
	value |= DMA_SYSBUS_MODE_AAL | DMA_SYSBUS_MODE_BLEN16 |DMA_SYSBUS_MODE_RB |
			DMA_SYSBUS_MODE_WR_OSR_LMT | DMA_SYSBUS_MODE_RD_OSR_LMT;

	if (fb)
		value |= DMA_SYSBUS_MODE_FB;
	if (mb) {
		value |= DMA_SYSBUS_MODE_MB;
	}	
	writel(value, ioaddr + DMA_SYSBUS_MODE);

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {		
		/* Configure the DMA control registers. */
		value = readl(ioaddr + DMA_CHN_CTL(chn));
		value |= DMA_CTL_DSL | DMA_CTL_PBLX8;
		writel(value, ioaddr + DMA_CHN_CTL(chn));	
		writel(DMA_INTR_DEFAULT_MASK, ioaddr + DMA_CHN_INT_EN(chn));
	}

	/* Initialize the descriptor related registers */
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);
		
		writel(tx_queue->dma_tx_size - 1, ioaddr + DMA_CHN_TD_RING_LEN(chn));
		writel(tx_queue->dma_tx_phy, ioaddr + DMA_CHN_TD_LADDR(chn));
		writel(tx_queue->dma_tx_tail, ioaddr + DMA_CHN_TD_TAIL_PTR(chn));

		/* configure TX contrl parameters */
		value = readl(ioaddr + DMA_CHN_TX_CTL(chn));
		value |= DMA_CHN_OSP | DMA_CHN_TXPBL |DMA_CHN_TCW;
		writel(value, ioaddr + DMA_CHN_TX_CTL(chn));

		/* Start tx */
		//value = readl(ioaddr + DMA_CHN_TX_CTL(chn));
		//writel(value | DMA_TX_START, ioaddr + DMA_CHN_TX_CTL(chn));
	}

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		rx_queue = GET_RX_QUEUE(priv, chn);
		index =  rx_queue->dma_rx_size - 1; //256-1=255
		
		rx_tail = dwmac1000_get_rx_tail_ptr(index, chn);		
		writel(rx_queue->dma_rx_size - 1, ioaddr + DMA_CHN_RD_RING_LEN(chn));
		writel(rx_queue->dma_rx_phy, ioaddr + DMA_CHN_RD_LADDR(chn));
		writel(rx_tail, ioaddr + DMA_CHN_RD_TAIL_PTR(chn));

		/* configure RX control parameters */
		value = readl(ioaddr + DMA_CHN_RX_CTL(chn));
		value |= DMA_RX_RPF | DMA_RX_PBL;
		writel(value, ioaddr + DMA_CHN_RX_CTL(chn));

		/* start Rx */
		//value = readl(ioaddr + DMA_CHN_RX_CTL(chn));
		//writel(value | DMA_RX_START, ioaddr + DMA_CHN_RX_CTL(chn));
	}
	
	return 0;
}

static void dwmac1000_dma_operation_mode(void __iomem *ioaddr, int txmode,
					 int rxmode)
{
	u32 csr6 = readl(ioaddr + DMA_CHN_CTL(0));

	if (txmode == SF_DMA_MODE) {
		CHIP_DBG(KERN_DEBUG "GMAC: enable TX store and forward mode\n");
		/* Transmit COE type 2 cannot be done in cut-through mode. */
		csr6 |= DMA_CONTROL_TSF;
		/* Operating on second frame increase the performance
		 * especially when transmit store-and-forward is used.
		 */
		csr6 |= DMA_CONTROL_OSF;
	} else {
		CHIP_DBG(KERN_DEBUG "GMAC: disabling TX SF (threshold %d)\n",
			 txmode);
		csr6 &= ~DMA_CONTROL_TSF;
		csr6 &= DMA_CONTROL_TC_TX_MASK;
		/* Set the transmit threshold */
		if (txmode <= 32)
			csr6 |= DMA_CONTROL_TTC_32;
		else if (txmode <= 64)
			csr6 |= DMA_CONTROL_TTC_64;
		else if (txmode <= 128)
			csr6 |= DMA_CONTROL_TTC_128;
		else if (txmode <= 192)
			csr6 |= DMA_CONTROL_TTC_192;
		else
			csr6 |= DMA_CONTROL_TTC_256;
	}

	if (rxmode == SF_DMA_MODE) {
		CHIP_DBG(KERN_DEBUG "GMAC: enable RX store and forward mode\n");
		csr6 |= DMA_CONTROL_RSF;
	} else {
		CHIP_DBG(KERN_DEBUG "GMAC: disable RX SF mode (threshold %d)\n",
			 rxmode);
		csr6 &= ~DMA_CONTROL_RSF;
		csr6 &= DMA_CONTROL_TC_RX_MASK;
		if (rxmode <= 32)
			csr6 |= DMA_CONTROL_RTC_32;
		else if (rxmode <= 64)
			csr6 |= DMA_CONTROL_RTC_64;
		else if (rxmode <= 96)
			csr6 |= DMA_CONTROL_RTC_96;
		else
			csr6 |= DMA_CONTROL_RTC_128;
	}

	writel(csr6, ioaddr + DMA_CHN_CTL(0));
}

static void dwmac1000_dump_dma_regs(void __iomem *ioaddr)
{
	int i;
	pr_info(" DMA registers\n");
	for (i = 0; i < 22; i++) {
		if ((i < 9) || (i > 17)) {
			int offset = i * 4;
			pr_err("\t Reg No. %d (offset 0x%x): 0x%08x\n", i,
			       (DMA_BUS_MODE + offset),
			       readl(ioaddr + DMA_BUS_MODE + offset));
		}
	}
}

static unsigned int dwmac1000_get_hw_feature(void __iomem *ioaddr, 
	unsigned int *hw_feature1, unsigned int *hw_feature2)
{
	unsigned int hw_feature0;

	hw_feature0 = readl(ioaddr + GMAC_HW_FEATURE0);
	*hw_feature1 = readl(ioaddr + GMAC_HW_FEATURE1);
	*hw_feature2 = readl(ioaddr + GMAC_HW_FEATURE2);
	
	return hw_feature0;
}

static void dwmac1000_rx_watchdog(void __iomem *ioaddr, 
											unsigned int riwt, int chn)
{
	writel(riwt, ioaddr + DMA_CHN_RIWT(chn));
}

static int dwmac_get_dma_isr(void __iomem *ioaddr)
{
	int intr_status;
	intr_status = readl(ioaddr + DMA_INT_STAT);

	return intr_status;
}

const struct stmmac_dma_ops dwmac1000_dma_ops = {
	.init = dwmac1000_dma_init,
	.dump_regs = dwmac1000_dump_dma_regs,
	.dma_mode = dwmac1000_dma_operation_mode,
	.enable_dma_transmission = dwmac_enable_dma_transmission,
	.enable_dma_irq = dwmac_enable_dma_irq,
	.disable_dma_irq = dwmac_disable_dma_irq,
	.start_tx = dwmac_dma_start_tx,
	.stop_tx = dwmac_dma_stop_tx,
	.start_rx = dwmac_dma_start_rx,
	.stop_rx = dwmac_dma_stop_rx,
	.dma_interrupt = dwmac_dma_interrupt,
	.get_hw_feature = dwmac1000_get_hw_feature,
	.rx_watchdog = dwmac1000_rx_watchdog,
	.get_dma_isr = dwmac_get_dma_isr,
	.get_rx_tail_ptr = dwmac1000_get_rx_tail_ptr,
	.set_rx_tail_ptr = dwmac1000_set_rx_tail_ptr,
	.get_tx_slotnum = dwmac1000_get_tx_slotnum,
};
