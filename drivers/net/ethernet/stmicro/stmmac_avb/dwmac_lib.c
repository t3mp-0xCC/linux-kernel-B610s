/*******************************************************************************
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

#include <linux/io.h>
#include "stmmac.h"
#include "dwmac_dma.h"
#include "stmmac_debug.h"

#define GMAC_HI_REG_AE		0x80000000

/* CSR1 enables the transmit DMA to check for new descriptor */
void dwmac_enable_dma_transmission(int chn, unsigned int index)
{
	struct stmmac_priv *priv = gmac_priv;
	void __iomem *ioaddr = priv->ioaddr;
	dma_addr_t dma_addr;
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, chn);

	if ( unlikely((index + 1) >= tx_queue->dma_tx_size)) {
		index = 0;
	} else {
		index++;
	}

	dma_addr = tx_queue->dma_tx_phy + (index * sizeof(union dma_desc));
	writel(dma_addr, ioaddr + DMA_CHN_TD_TAIL_PTR(chn));	
}

void dwmac_enable_dma_irq(void __iomem *ioaddr, int chn)
{
	writel(DMA_INTR_DEFAULT_MASK, ioaddr + DMA_CHN_INT_EN(chn));
}

void dwmac_disable_dma_irq(void __iomem *ioaddr, int chn)
{
	writel(0, ioaddr + DMA_CHN_INT_EN(chn));
}

void dwmac_dma_start_tx(void __iomem *ioaddr, int chn)
{
	unsigned int value;

	value = readl(ioaddr + DMA_CHN_TX_CTL(chn));
	value |= DMA_TX_START;
	writel(value, ioaddr + DMA_CHN_TX_CTL(chn));
}

void dwmac_dma_stop_tx(void __iomem *ioaddr, int chn)
{
	unsigned int value;

	value = readl(ioaddr + DMA_CHN_TX_CTL(chn));
	value &= ~DMA_TX_START;
	writel(value, ioaddr + DMA_CHN_TX_CTL(chn));
}

void dwmac_dma_start_rx(void __iomem *ioaddr, int chn)
{
	unsigned int value;
	
	value = readl(ioaddr + DMA_CHN_RX_CTL(chn));
	value |= DMA_RX_START;
	writel(value, ioaddr + DMA_CHN_RX_CTL(chn));
}

void dwmac_dma_stop_rx(void __iomem *ioaddr, int chn)
{
	unsigned int value;
	
	value = readl(ioaddr + DMA_CHN_RX_CTL(chn));
	value &= ~DMA_RX_START;
	writel(value, ioaddr + DMA_CHN_RX_CTL(chn));
}

#ifdef DWMAC_DMA_DEBUG
static void show_tx_process_state(int chn, unsigned int status)
{
	unsigned int state;
	
	state = (status & DMA_STATUS_TS_MASK(chn)) >> DMA_STATUS_TS_SHIFT(chn);
	switch (state) {
	case 0:
		pr_info("- TX ch%d(Stopped): Reset or Stop command\n", chn);
		break;
	case 1:
		pr_info("- TX ch%d(Running): Fetching the Tx desc\n", chn);
		break;
	case 2:
		pr_info("- TX ch%d(Running): Waiting for status\n", chn);
		break;
	case 3:
		pr_info("- TX ch%d(Running): Reading Data from system memory "
		       "buffer and queuing it to the Tx buffer (Tx FIFO)\n", chn);
		break;
	case 4:
		pr_info("- TX ch%d(Running): Timestamp write state\n", chn);
		break;
	case 6:
		pr_info("- TX ch%d(Suspended): Tx Descriptor Unavailable "
		       "or Tx Buffer Underflow\n", chn);
		break;
	case 7:
		pr_info("- TX ch%d(Running): Closing Tx descriptor\n", chn);
		break;
	default:
		break;
	}
}

static void show_rx_process_state(int chn, unsigned int status)
{
	unsigned int state;
	state = (status & DMA_STATUS_RS_MASK(chn)) >> DMA_STATUS_RS_SHIFT(chn);

	switch (state) {
	case 0:
		pr_info("- RX ch%d(Stopped): Reset or Stop command\n", chn);
		break;
	case 1:
		pr_info("- RX ch%d(Running): Fetching the Rx desc\n", chn);
		break;
	case 3:
		pr_info("- RX ch%d(Running): Waiting for Rx pkt\n", chn);
		break;
	case 4:
		pr_info("- RX ch%d(Suspended): Rx Descriptor Unavailable\n", chn);
		break;
	case 5:
		pr_info("- RX ch%d(Running): Closing Rx descriptor\n", chn);
		break;
	case 6:
		pr_info("- RX ch%d(Running): Timestamp write state\n", chn);
		break;
	case 7:
		pr_info("- RX ch%d(Running): Transferring the received packet data"
		       "  from the Rx buffer to the system memory\n", chn);
		break;
	default:
		break;
	}
}
#endif
static int dwmac_handle_chn_irq(unsigned int chn, unsigned int stat,
						void __iomem *ioaddr, struct stmmac_extra_stats *x)
{
	int ret = 0;
	
	/* TX/RX NORMAL interrupts */
	if (likely(stat & DMA_STATUS_RI)) {
		u32 value = readl(ioaddr + DMA_CHN_INT_EN(chn));
		/* to schedule NAPI on real RIE event. */
		if (likely(value & DMA_INTR_ENA_RIE)) {
			x->rx_normal_irq_n[chn]++;
			ret |= handle_rx;
		}
	}
	if (likely(stat & DMA_STATUS_TI)) {
		x->tx_normal_irq_n[chn]++;
		ret |= handle_tx;
	}
	if (unlikely(stat & DMA_STATUS_ERI))
		x->rx_early_irq[chn]++;

	if (stat & DMA_STATUS_TBU) {
		x->tx_buf_unavail[chn]++;
	}

	/* ABNORMAL interrupts */
	if (unlikely(stat & DMA_STATUS_TPS)) {
		DWMAC_LIB_DBG(("Transmit Process Stopped\n"));
		x->tx_stopped[chn]++;
		ret = tx_hard_error;
	}
	if (unlikely(stat & DMA_STATUS_RBU)) {
		DWMAC_LIB_DBG(("Receive Buffer Unavailable\n"));
		x->rx_buf_unav_irq[chn]++;
		ret |= rx_buf_uavi;
	}
	if (unlikely(stat & DMA_STATUS_RPS)) {
		DWMAC_LIB_DBG(("Receive Process Stopped \n"));
		x->rx_process_stopped_irq[chn]++;
	}
	if (unlikely(stat & DMA_STATUS_RWT)) {
		DWMAC_LIB_DBG(("Receive Watchdog Timeout\n"));
		x->rx_watchdog_irq[chn]++;
	}
	if (unlikely(stat & DMA_STATUS_ETI)) {
		DWMAC_LIB_DBG(("Early Transmit Interrupt\n"));
		x->tx_early_irq[chn]++;
	}
	if (unlikely(stat & DMA_STATUS_FBE)) {
		DWMAC_LIB_DBG(("Fatal Bus Error\n"));
		x->fatal_bus_error_irq[chn]++;
		ret |= tx_hard_error;		}
	if (unlikely(stat & DMA_STATUS_CDE)) {
		DWMAC_LIB_DBG(("Context Descriptor Error\n"));
		x->tx_ctx_desc_err[chn]++;
	}
	
	if (unlikely(stat & DMA_STATUS_REB)) {
		DWMAC_LIB_DBG(("%s: rx dma error: %08x\n", __func__, (stat >> 19)));
		x->rx_dma_err[chn]++;
	}
	
	if (unlikely(stat & DMA_STATUS_TEB)) {
		DWMAC_LIB_DBG(("%s: tx dma error: %08x\n", __func__, (stat & DMA_STATUS_TEB) >> 16));
		x->tx_dma_err[chn]++;
	}

	/* Clear the interrupt by writing a logic 1 to the register*/
	writel(stat, ioaddr + DMA_CHN_STAT(chn));
	DWMAC_LIB_DBG(("\n\n"));

	return ret;
}

int dwmac_dma_interrupt(void __iomem *ioaddr, 
							struct stmmac_extra_stats *x, int chn)
{
	int ret;
	u32 int_status;		//tx_chn_nr should bigger than rx_chn_nr
	unsigned int debug;
	
	/* Debug channel 0/1/2*/
	debug = readl(ioaddr + DMA_DBG_STAT(0));
	
#ifdef DWMAC_DMA_DEBUG
	if (chn < TX_CHN_NR) { 
		show_tx_process_state(chn, debug);
	}

	if (chn < RX_CHN_NR) {	
		show_rx_process_state(chn, debug);
	}
#endif
		int_status = readl(ioaddr + DMA_CHN_STAT(chn));
		ret = dwmac_handle_chn_irq(chn, int_status, ioaddr, x);
	
	return ret;
}

void dwmac_dma_flush_tx_fifo(void __iomem *ioaddr)
{
	u32 csr6 = readl(ioaddr + DMA_CHN_CTL(0));
	writel((csr6 | DMA_CONTROL_FTF), ioaddr + DMA_CHN_CTL(0));

	do {} while ((readl(ioaddr + DMA_CHN_CTL(0)) & DMA_CONTROL_FTF));
}

void stmmac_set_mac_addr(void __iomem *ioaddr, u8 addr[6],
			 unsigned int high, unsigned int low)
{
	unsigned long data;

	data = (addr[5] << 8) | addr[4];
	/* For MAC Addr registers se have to set the Address Enable (AE)
	 * bit that has no effect on the High Reg 0 where the bit 31 (MO)
	 * is RO.
	 */
	writel(data | GMAC_HI_REG_AE, ioaddr + high);
	data = (addr[3] << 24) | (addr[2] << 16) | (addr[1] << 8) | addr[0];
	writel(data, ioaddr + low);
}

/* Enable disable MAC RX/TX */
void stmmac_set_mac(void __iomem *ioaddr, bool enable)
{
	u32 value = readl(ioaddr + GMAC_CONTROL);

	if (enable)
		value |= GMAC_CONTROL_RE | GMAC_CONTROL_TE;
	else
		value &= ~(GMAC_CONTROL_RE | GMAC_CONTROL_TE);

	writel(value, ioaddr + GMAC_CONTROL);
}

void stmmac_get_mac_addr(void __iomem *ioaddr, unsigned char *addr,
			 unsigned int high, unsigned int low)
{
	unsigned int hi_addr, lo_addr;

	/* Read the MAC address from the hardware */
	hi_addr = readl(ioaddr + high);
	lo_addr = readl(ioaddr + low);

	/* Extract the MAC address from the high and low words */
	addr[0] = lo_addr & 0xff;
	addr[1] = (lo_addr >> 8) & 0xff;
	addr[2] = (lo_addr >> 16) & 0xff;
	addr[3] = (lo_addr >> 24) & 0xff;
	addr[4] = hi_addr & 0xff;
	addr[5] = (hi_addr >> 8) & 0xff;
}

