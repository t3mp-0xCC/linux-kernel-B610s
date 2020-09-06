/*******************************************************************************
  DWMAC DMA Header file.

  Copyright (C) 2007-2009  STMicroelectronics Ltd

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

#ifndef __DWMAC_DMA_H__
#define __DWMAC_DMA_H__

/* DMA CRS Control and Status Register Mapping */
#define DMA_BUS_MODE		0x00001000	/* DMA Mode */
#define DMA_SYSBUS_MODE		0x00001004	/* SysBus Mode */
#define DMA_INT_STAT		0x00001008	/* DMA interrupt status */
#define	DMA_DBG_STAT(n)		(0x0000100c + (n)*4)	/* 0=<n<=2 */
#define	AXI4_TX_AR_ACE_CTL	0x00001020
#define	AXI4_RX_AW_ACE_CTL	0x00001024
#define	AXI4_TXRX_AWAR_ACE_CTL	0x00001028
//#define	BIT(n)  (1UL << (n))

#define DMA_CAP_NUM		3

/*DMA Channel registers, n depends channel configuration */
#define DMA_CHN_CTL(n)			(0x00001100 + (n)*0x80)
#define	DMA_CHN_TX_CTL(n)		(DMA_CHN_CTL(n) + 0x4)
#define	DMA_CHN_RX_CTL(n)		(DMA_CHN_CTL(n) + 0x8)
#define	DMA_CHN_TD_LHADDR(n)	(DMA_CHN_CTL(n) + 0x10)
#define	DMA_CHN_TD_LADDR(n)		(DMA_CHN_CTL(n) + 0x14)
#define	DMA_CHN_RD_LHADDR(n)	(DMA_CHN_CTL(n) + 0x18)
#define	DMA_CHN_RD_LADDR(n)		(DMA_CHN_CTL(n) + 0x1c)
#define	DMA_CHN_TD_TAIL_PTR(n)	(DMA_CHN_CTL(n) + 0x20)
#define	DMA_CHN_RD_TAIL_PTR(n)	(DMA_CHN_CTL(n) + 0x28)
#define	DMA_CHN_TD_RING_LEN(n)	(DMA_CHN_CTL(n) + 0x2c)
#define	DMA_CHN_RD_RING_LEN(n)	(DMA_CHN_CTL(n) + 0x30)
#define	DMA_CHN_INT_EN(n)		(DMA_CHN_CTL(n) + 0x34)
#define	DMA_CHN_RIWT(n)			(DMA_CHN_CTL(n) + 0x38)
#define	DMA_CHN_SFCS(n)			(DMA_CHN_CTL(n) + 0x3c)
#define	DMA_CHN_CUR_APP_TD(n)	(DMA_CHN_CTL(n) + 0x44)
#define	DMA_CHN_CUR_APP_RD(n)	(DMA_CHN_CTL(n) + 0x4c)
#define	DMA_CHN_CUR_APP_TBH(n)	(DMA_CHN_CTL(n) + 0x50)
#define	DMA_CHN_CUR_APP_TB(n)	(DMA_CHN_CTL(n) + 0x54)
#define	DMA_CHN_CUR_APP_RBH(n)	(DMA_CHN_CTL(n) + 0x58)
#define	DMA_CHN_CUR_APP_RB(n)	(DMA_CHN_CTL(n) + 0x5c)
#define	DMA_CHN_STAT(n)			(DMA_CHN_CTL(n) + 0x60)
#define	DMA_CHN_MFRAME_CNT(n)	(DMA_CHN_CTL(n) + 0x6c)

/* DMA Normal interrupt */
#define DMA_INTR_ENA_NIE 0x00008000	/* Normal Summary */
#define DMA_INTR_ENA_TIE 0x00000001	/* Transmit Interrupt */
#define DMA_INTR_ENA_TUE 0x00000004	/* Transmit Buffer Unavailable */
#define DMA_INTR_ENA_RIE 0x00000040	/* Receive Interrupt */
#define DMA_INTR_ENA_ERE 0x00000800	/* Early Receive */

#define DMA_INTR_NORMAL	(DMA_INTR_ENA_NIE | DMA_INTR_ENA_RIE | \
			DMA_INTR_ENA_TIE)

/* DMA Abnormal interrupt */
#define DMA_INTR_ENA_AIE 0x00004000	/* Abnormal Summary */
#define DMA_INTR_ENA_FBE 0x00001000	/* Fatal Bus Error */
#define DMA_INTR_ENA_ETE 0x00000400	/* Early Transmit */
#define DMA_INTR_ENA_RWE 0x00000200	/* Receive Watchdog */
#define DMA_INTR_ENA_RSE 0x00000100	/* Receive Stopped */
#define DMA_INTR_ENA_RUE 0x00000080	/* Receive Buffer Unavailable */
#define DMA_INTR_ENA_OVE 0x00000010	/* Receive Overflow */
#define DMA_INTR_ENA_TJE 0x00000008	/* Transmit Jabber */
#define DMA_INTR_ENA_TSE 0x00000002	/* Transmit Stopped */

#define DMA_INTR_ABNORMAL	(DMA_INTR_ENA_AIE | DMA_INTR_ENA_FBE)

/* DMA default interrupt mask */
#define DMA_INTR_DEFAULT_MASK	(DMA_INTR_NORMAL | DMA_INTR_ABNORMAL)

/* DMA Status register defines */
#define DMA_STATUS_GLPII	0x40000000	/* GMAC LPI interrupt */

#define DMA_STATUS_EB_MASK	0x00380000	/* Error Bits Mask */
#define DMA_STATUS_EB_TX_ABORT	0x00080000	/* Error Bits - TX Abort */
#define DMA_STATUS_EB_RX_ABORT	0x00100000	/* Error Bits - RX Abort */
#define DMA_STATUS_FBI	0x00002000	/* Fatal Bus Error Interrupt */
#define DMA_STATUS_OVF	0x00000010	/* Receive Overflow */
#define DMA_STATUS_TJT	0x00000008	/* Transmit Jabber Timeout */
#define DMA_STATUS_TU	0x00000004	/* Transmit Buffer Unavailable */
#define DMA_CONTROL_FTF		0x00100000	/* Flush transmit FIFO */

#define DMA_STATUS_TI	BIT(0)		/* Transmit Interrupt */
#define	DMA_STATUS_TPS	BIT(1)		/* Transmit Process Stopped */
#define	DMA_STATUS_TBU	BIT(2)		/* Transmit Buffer Unavailable */
#define DMA_STATUS_RI	BIT(6)		/* Receive Interrupt */
#define DMA_STATUS_RBU	BIT(7)		/* Receive Buffer Unavailable */
#define DMA_STATUS_RPS	BIT(8)		/* Receive Process Stopped */
#define DMA_STATUS_RWT	BIT(9)		/* Receive Watchdog Timeout*/
#define DMA_STATUS_ETI	BIT(10)		/* Early Transmit Interrupt */
#define DMA_STATUS_ERI	BIT(11)		/* Early Receive Interrupt */
#define DMA_STATUS_FBE	BIT(12)		/* Fatal Bus Error */
#define DMA_STATUS_CDE	BIT(13)		/* Context Descriptor Error */
#define DMA_STATUS_AIS	BIT(14)		/* Abnormal Interrupt Summary */
#define DMA_STATUS_NIS	BIT(15)		/* Normal Interrupt Summary */
#define	DMA_STATUS_REB	0x00380000	/* Rx DMA error bits */
#define	DMA_STATUS_TEB	0x00070000	/* Tx DMA error bits */

/* DMA debug  */
#define DMA_STATUS_TS_MASK(n)	((0x0000f000) << (8 * n))	/* Transmit channel 0 Process State */
#define DMA_STATUS_TS_SHIFT(n)	((8 * (n)) + 12)	/*0~2*/
#define DMA_STATUS_RS_MASK(n)	((0x00000f00) << (8 * n))	/* Receive channel 0 Process State */
#define DMA_STATUS_RS_SHIFT(n)	(8 * ((n) + 1))		/*0~2*/

/* DMA_INT_STAT defines */
#define DMA_ISR_DCIS_LPOS(n) (n)	/*0 =< n <= 7*/
#define DMA_ISR_DCIS_HPOS(n) (n)	/*0 =< n <= 7*/

#define DMA_ISR_MTL_LPOS	16
#define DMA_ISR_MTL_HPOS	16

#define DMA_ISR_MAC_LPOS	17
#define DMA_ISR_MAC_HPOS	17

/* DMA_CHN_SFCS defines */
#define DMA_SFCS_RSN_POS	16

extern void dwmac_enable_dma_transmission(int chn, unsigned int index);
extern void dwmac_enable_dma_irq(void __iomem *ioaddr, int chn);
extern void dwmac_disable_dma_irq(void __iomem *ioaddr, int chn);
extern void dwmac_dma_start_tx(void __iomem *ioaddr, int chn);
extern void dwmac_dma_stop_tx(void __iomem *ioaddr, int chn);
extern void dwmac_dma_start_rx(void __iomem *ioaddr, int chn);
extern void dwmac_dma_stop_rx(void __iomem *ioaddr, int chn);
extern int dwmac_dma_interrupt(void __iomem *ioaddr, 
							struct stmmac_extra_stats *x, int chn);

#endif /* __DWMAC_DMA_H__ */
