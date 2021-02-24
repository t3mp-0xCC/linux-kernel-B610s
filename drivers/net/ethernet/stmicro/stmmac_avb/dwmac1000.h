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
#ifndef __DWMAC1000_H__
#define __DWMAC1000_H__

#include <linux/phy.h>
#include "common.h"

#define MAC_ADDR_NUM	1

/* MAC registers */
#define GMAC_CONTROL		0x00000000	/* Configuration */
#define GMAC_EXT_CFG		0x00000004	/* Extended operation mode */
#define GMAC_PKT_FILTER		0x00000008	/* The MAC packet filter register */
#define GMAC_WDG_TIMEO		0x0000000c	/* The mac watchdog timeout */
#define GMAC_HASH_TABLE(n)	(0x10 + (n) * 4)	/* 0 =< n <= 7*/
#define GMAC_VLAN_TAG		0x00000050	/* MAC VLAN tag */
#define GMAC_VLAN_HTABLE	0x00000058
#define	GMAC_VLAN_INCL		0x00000060
#define	GMAC_IVLAN_INCL		0x00000064
#define GMAC_TXQ_FCTL(n)	(0x00000070 + (n) * 4)	/* 0=<n<= 7*/
#define GMAC_RX_FLOW_CTL	0x00000090
#define	GMAC_TXQ_PRTY_MAP0	0x00000098
#define	GMAC_TXQ_PRTY_MAP1	0x0000009C
#define	GMAC_RXQ_CTL(n)		(0x000000A0 + (n) * 4)	/*0=<n<=3*/
#define GMAC_INT_STAT		0x000000b0	/* MAC interrupt status */
#define GMAC_INT_EN			0x000000b4	/* MAC interrupt enable */
#define	GMAC_RX_TX_STAT		0x000000b8	
#define	GMAC_PMT_CTL_STAT	0x000000c0
#define	GMAC_RWK_PKT_FLT	0x000000c4
#define	GMAC_LPI_CTL_STAT	0x000000d0
#define	GMAC_LPI_TIMERS_CTL	0x000000d4
#define	GMAC_LPI_ETIMER		0x000000d8
#define	GMAC_1US_TIC_CNTR	0x000000dc

/* PCS registers (AN/TBI/SGMII/RGMII) offset */
#define GMAC_AN_CTRL		0x000000e0	/* AN control */
#define GMAC_AN_STATUS		0x000000e4	/* AN status */
#define GMAC_ANE_ADV		0x000000e8	/* Auto-Neg. Advertisement */
#define GMAC_ANE_LPA		0x000000ec	/* Auto-Neg. link partener ability */
#define GMAC_ANE_EXP		0x000000f0	/* ANE expansion */
#define GMAC_TBI_EXT_STAT	0x000000f4	/* TBI extend status */
#define GMAC_PHYIF_CTL_STAT	0x000000f8	/* SGMII RGMII status */

#define GMAC_VERSION		0x00000110	/* GMAC CORE Version */
#define	GMAC_DEBUG			0x00000114
#define GMAC_HW_FEATURE0	0x0000011C	/* HW Feature0 Register */
#define GMAC_HW_FEATURE1	0x00000120	/* HW Feature1 Register */
#define GMAC_HW_FEATURE2	0x00000124	/* HW Feature2 Register */
#define GMAC_MDIO_ADDR		0x00000200	/* MAC MDIO address */
#define	GMAC_MDIO_DATA		0x00000204	/* MAC MDIO Data */
#define	GMAC_GPIO_CTL		0x00000208
#define	GMAC_GPIO_STATUS	0x0000020c
#define	GMAC_ARP_ADDR		0x00000210

/* GMAC HW ADDR regs */
#define GMAC_ADDR_HIGH(n)	(0x00000300 + (n) * 8)	/*0=<reg<=127*/
#define GMAC_ADDR_LOW(n)	(0x00000304 + (n) * 8)	/*0=<reg<=127*/

/* MMC registers */
#define	MMC_CTRL			0x00000700
#define	MMC_RX_INT			0x00000704
#define	MMC_TX_INT 			0x00000706
#define	MMC_RX_INT_MASK		0x0000070c
#define	MMC_TX_INT_MASK		0x00000710
#define	TX_OCTET_CNT_GB		0x00000714
#define	TX_PKT_CNT_GB		0x00000718
#define	TX_BRDCT_PKT_GB		0x0000071c
#define	TX_MTCT_PKT_GB		0x00000720
#define	TX_64OT_PKT_GB		0x00000724
#define	TX_65TO127OT_PGB	0x00000728
#define	TX_128TO255OT_PGB	0x0000072c
#define	TX_256TO511OT_PGB	0x00000730
#define	TX_512TO1023OT_PGB	0x00000734
#define	TX_1024TOMAXOT_PGB	0x00000738
/* wait for coninue, up to 0x000008fc */

/* L3-L4 Reigsters:0x00000900~0x00000a6c*/

/* MAC timestamp related registers */
/* see stmmac_ptp.h
#define GMAC_TIMESTAMP_CONTROL					0x0B00 
#define GMAC_SUB_SECOND_INCREMENT               0x0B04 
#define GMAC_SYSTEM_TIME_SECONDS                0x0B08 
#define GMAC_SYSTEM_TIME_NANOSECONDS            0x0B0C 
#define GMAC_SYSTEM_TIME_SECONDS_UPDATE         0x0B10 
#define GMAC_SYSTEM_TIME_NANOSECONDS_UPDATE     0x0B14 
#define GMAC_TIMESTAMP_ADDEND                   0x0B18 
#define GMAC_SYSTEM_TIME_HIGHER_WORD_SECONDS    0x0B1C 
#define GMAC_TIMESTAMP_STATUS                   0x0B20 
#define GMAC_TXTIMESTAMP_STATUS_NANOSECONDS     0x0B30 
#define GMAC_TXTIMESTAMP_STATUS_SECONDS         0x0B34 
#define GMAC_AUXILIARY_CONTROL                  0x0B40 
#define GMAC_AUXILIARY_TIMESTAMP_NANOSECONDS    0x0B48 
#define GMAC_AUXILIARY_TIMESTAMP_SECONDS        0x0B4C 
#define GMAC_TIMESTAMP_INGRESS_ASYM_CORR        0x0B50 
#define GMAC_TIMESTAMP_EGRESS_ASYM_CORR         0x0B54 
#define GMAC_TIMESTAMP_INGRESS_CORR_NANOSECOND  0x0B58 
#define GMAC_TIMESTAMP_EGRESS_CORR_NANOSECOND   0x0B5C 
#define GMAC_PPS_CONTROL                        0x0B70 
#define GMAC_PPS0_TARGET_TIME_SECONDS           0x0B80 
#define GMAC_PPS0_TARGET_TIME_NANOSECONDS       0x0B84 
#define GMAC_PPS0_INTERVAL                      0x0B88 
#define GMAC_PPS0_WIDTH                         0x0B8C 
#define GMAC_PPS1_TARGET_TIME_SECONDS           0x0B90 
#define GMAC_PPS1_TARGET_TIME_NANOSECONDS       0x0B94 
#define GMAC_PPS1_INTERVAL                      0x0B98 
#define GMAC_PPS1_WIDTH                         0x0B9C 
#define GMAC_PPS2_TARGET_TIME_SECONDS           0x0BA0 
#define GMAC_PPS2_TARGET_TIME_NANOSECONDS       0x0BA4 
#define GMAC_PPS2_INTERVAL                      0x0BA8 
#define GMAC_PPS2_WIDTH                         0x0BAC 
#define GMAC_PPS3_TARGET_TIME_SECONDS           0x0BB0 
#define GMAC_PPS3_TARGET_TIME_NANOSECONDS       0x0BB4 
#define GMAC_PPS3_INTERVAL                      0x0BB8 
#define GMAC_PPS3_WIDTH                         0x0BBC 
#define GMAC_PTO_CONTROL                        0x0BC0 
#define GMAC_SOURCE_PORT_IDENTITY0              0x0BC4 
#define GMAC_SOURCE_PORT_IDENTITY1              0x0BC8 
#define GMAC_SOURCE_PORT_IDENTITY2              0x0BCC 
#define GMAC_LOG_MESSAGE_INTERVAL               0x0BD0 
*/

/* end of MAC registers */

/* MTL registers*/
#define MTL_OPERT_MODE		0x00000c00	/* MTL operation mode register */
#define MTL_DEBUG_CTL		0x00000c08
#define MTL_DEBUG_STAT		0x00000c0c
#define MTL_FIFO_DBG_DATA	0x00000c10
#define MTL_INT_STAT		0x00000c20	/* MTL interrupt status  */
#define MTL_RXQ_DMA_MAP(n)	(0x00000c30 + (n)*4) /* 0=<n<=2 */

/* MTL queue n registers */
#define MTL_TXQ_OPRT_MODE(n)	(0x00000d00 + (n)*0x40) 		/* 0=<n<=7*/
#define MTL_TXQ_UNDER_FLOW(n)	(MTL_TXQ_OPRT_MODE(n) + 0x4)	/* 0=<n<=7*/
#define MTL_TXQ_DBG(n)			(MTL_TXQ_OPRT_MODE(n) + 0x8) 	/* 0=<n<=7*/
#define MTL_TXQ_ETS_CTL(n)		(MTL_TXQ_OPRT_MODE(n) + 0x10) 	/* 1=<n<=7*/
#define MTL_TXQ_ETS_STAT(n)		(MTL_TXQ_OPRT_MODE(n) + 0x14)	/* 0=<n<=7*/
#define MTL_TXQ_IDLE_SLOPE_CREDIT(n)	(MTL_TXQ_OPRT_MODE(n) + 0x18)	/* 1=<n<=7*/
#define MTL_TXQ_SEND_SLOPE_CREDIT(n)	(MTL_TXQ_OPRT_MODE(n) + 0x1c)	/* 1=<n<=7*/
#define MTL_TXQ_HI_CREDIT(n)		(MTL_TXQ_OPRT_MODE(n) + 0x20)	/* 1=<n<=7*/
#define MTL_TXQ_LO_CREDIT(n)		(MTL_TXQ_OPRT_MODE(n) + 0x24)	/* 1=<n<=7*/
#define MTL_QINT_CTL_STAT(n)	(MTL_TXQ_OPRT_MODE(n) + 0x2c)	/* 0=<n<=7*/
#define MTL_RXQ_OPRT_MODE(n)	(MTL_TXQ_OPRT_MODE(n) + 0x30)	/* 0=<n<=7*/
#define MTL_RXQ_MPKT_OFLW(n)	(MTL_TXQ_OPRT_MODE(n) + 0x34)	/* 0=<n<=7*/
#define MTL_RXQ_DBG(n)			(MTL_TXQ_OPRT_MODE(n) + 0x38)	/* 0=<n<=7*/
#define MTL_RXQ_CTL(n)			(MTL_TXQ_OPRT_MODE(n) + 0x3c)	/* 0=<n<=7*/

/* GMAC HW ADDR defines */
#define MTL_MULTI_ADDR_HI	0x0000ffff	/* multicast mac address high*/
#define MTL_MULTI_ADDR_LO	0xffffffff	/* multicast mac address low */

/* MTL operation mode defines */
#define MTL_OPT_DTXSTS	0x00000000	/* Drop Transmit Status: 0 */
#define MTL_OPT_SCHALG  0x00000060	/* Tx Scheduling Algorithm:Strick Alg */
#define MTL_OPT_RAA		0x00000000	/* Strict priority */

/* MTL Tx queue 0 operation mode defines */
#define MTL_TX_TTC		0x00000000	/* TTC is 000:32 */
//#define MTL_TXQ_SIZE	0x002F0000	/* (47 + 1) * 256 bytes:3 queues, each have 256 descs*/
#define MTL_TXQ_SIZE_OFFSET	16
#define	MTL_TXQ_EN		0x00000008	/* Transmit queue enable */
#define	MTL_TXQ_AV_EN	0x00000004	/* Transmit queue AV feature enable */
#define MTL_TX_TSF		0x00000002	/* Transmit Store and Forward */

/*MTL Rx queue 0 operation mode defines */
#define MTL_RX_RTC		0x00000000	/* RTC is 00:64 */
#define MTL_RX_EHFC		0x00000000	/* The bit is disabled */
#define MTL_RX_RSF		0x00000020	/* Receive Queue Store and Forward disabled */
#define MTL_RX_FUP		0x00000000	/* forward undersized good packets */
//#define MTL_RXQ_SIZE	0x00F00000	/* receive queue size:16 * 256 */
#define	MTL_RXQ_SIZE_OFFSET	20
#define MTL_RXQ_DIS_TCP_EF	0x00000040	/* Don't drop TCP/IP checksum err packets */

/* MAC Packet filter defines */
#define VLAN_TAG_FLT_EN	0x00010000	/* Enable vlan tag filter*/

/* MAC Tx  queue flow control defines */
#define FLOW_CTL_BUSY	0x00000001	/* flow control busy */
#define TX_FLOW_CTL_EN	0x00000002	/* transmit flow control enable */

/* MAC Rx queue control defines */
#define GMAC_RXQ0_EN	0x00000001	/* Rx Queue 0 enable for AV */
#define GMAC_RXQ1_AVPTPQ	0x0000	/* The Rx queue 0 routed the PTP packets */
#define	GMAC_RXQ2_PSQ0	0x00000031	/* bit 0\4\5 for internet data, audio data, video data*/

/* MAC interrupt defines 
 * Enabled RXSTSIE\TXSTSIE\PHYIE, Remember to add TSIE in AVB!!!
*/
#define	GMAC_GPIIS		0x00008000
#define GMAC_RXSTSIS	0x00004000
#define GMAC_TXSTSIS	0x00002000
#define	GMAC_TSIS		0x00001000
#define	GMAC_MMCRXIPIS	0x00000800
#define	GMAC_MMCTXIS	0x00000400
#define	GMAC_MMCRXIS	0x00000200
#define	GMAC_MMCIS		0x00000100
#define	GMAC_LPIIS		0x00000020
#define	GMAC_PMTIS		0x00000010
#define	GMAC_PHYIS		0x00000008
#define	GMAC_PCSANCIS	0x00000004
#define	GMAC_PCSLCHGIS	0x00000002
#define	GMAC_RGSMIIIS	0x00000001
#define MAC_INT_EN	(GMAC_RXSTSIS | GMAC_TXSTSIS | GMAC_TSIS)// | GMAC_PHYIS)

#define	MAC_ISR_GPIIS_HPOS		15
#define	MAC_ISR_GPIIS_LPOS		15

#define MAC_ISR_RXSTSIS_HPOS	14
#define MAC_ISR_RXSTSIS_LPOS	14

#define MAC_ISR_TXSTSIS_HPOS	13
#define MAC_ISR_TXSTSIS_LPOS	13

#define	MAC_ISR_TSIS_HPOS		12
#define	MAC_ISR_TSIS_LPOS		12

#define	MAC_ISR_MMCRXIPIS_HPOS	11
#define	MAC_ISR_MMCRXIPIS_LPOS	11

#define	MAC_ISR_MMCTXIS_HPOS	10
#define	MAC_ISR_MMCTXIS_LPOS	10

#define	MAC_ISR_MMCRXIS_HPOS	9
#define	MAC_ISR_MMCRXIS_LPOS	9

#define	MAC_ISR_MMCIS_HPOS		8
#define	MAC_ISR_MMCIS_LPOS		8

#define	MAC_ISR_LPIIS_HPOS		5
#define	MAC_ISR_LPIIS_LPOS		5

#define	MAC_ISR_PMTIS_HPOS		4
#define	MAC_ISR_PMTIS_LPOS		4

#define	MAC_ISR_PHYIS_HPOS		3
#define	MAC_ISR_PHYIS_LPOS		3

#define	MAC_ISR_PCSANCIS_HPOS	2
#define	MAC_ISR_PCSANCIS_LPOS	2

#define	MAC_ISR_PCSLCHGIS_HPOS	1
#define	MAC_ISR_PCSLCHGIS_LPOS	1

#define	MAC_ISR_RGSMIIIS_HPOS	0
#define	MAC_ISR_RGSMIIIS_LPOS	0


/* GMAC_RX_TX_STAT defines */
#define GMAC_RTX_RWT	0x00000100
#define GMAC_RTX_TJT	0x00000001

/* MTL Tx Debug defines */
#define MTL_TX_TXQSTS	0x00000010	/* MTL Tx Queue Not Empty Status */
#define MTL_TX_TRCSTS	0x00000006	/* MTL Tx Queue Read Controller Status */

/* MTL Rx Debug defines */
#define MTL_RX_PRXQ		0x3fff0000	/* Number of Packets in Receive Queue */
#define MTL_RX_RXQSTS	0x00000030	/* MTL Rx Queue Fill-Level Status */

/* MTL interrupt status defines */
#define MTL_QIS(n)		(1UL << (n))	/* Queue n Interrupt status */

/* GMAC MDIO defines */
#define GMAC_MDIO_DATA_RA	0xffff0000	/* Register Address */
#define GMAC_MDIO_DATA_GD	0x0000ffff	/* GMII Data */

/* GMAC Vlan Tag defines */
#define VLAN_TAG_ETV	0x00000000	/* 16-bit VLAN Tag comparison */
#define VLAN_TAG_VL_PCP	0x00008000	/* PCP=4 */
#define VLAN_TAG_VL_CFI	0x00000000	/* CFI=0 */
#define	VLAN_TAG_VL_VID	0x00000064	/* VID=0x64 */

/* MTL ETS Control defines */
#define	CREDIT_CONTROL	0x00000008	/* enable credit control */
#define AV_ALGORITHM	0x00000004	/* using credit-based algorithm */
#define SLOT_COUNT		0x00000040	/* 16 slots */
#define ALG_MASK		0x00000004	/* AV algorithm mask */
#define ALG_OFFSET		2			/* AV algorithm offset */
//#define SLC_OFFSET		4			/* slot count offset */
//#define SLC_MASK		0x00000070	/* slot count mask */
#define ABS_MASK		0x00ffffff	/* slot count mask */

/* MTL CBS Defines */
#define IDLE_SLOPE		8111		/* 990M bandwidth, 0.99 *8 * 1024 */
#define SEND_SLOPE		81			/* 990M bandwidth */
#define HI_CREDIT		0x08000000	/* 16384 * 8 * 1024 */
#define LO_CREDIT		0x18000000	/* complement of 16384*8*1024 in bit[28:0] */

/* MTL_TXQ_IDLE_SLOPE_CREDIT defines */
#define MTL_TX_ISLC_MASK	0x001fffff	/* bit[20:0]*/
/* MTL_TXQ_SEND_SLOPE_CREDIT defines */
#define MTL_TX_SSLC_MASK	0x00003fff	/* bit[13:0]*/

enum dwmac1000_irq_status {
	dma_ch_int 	= 0x0001,
	mtl_int		= 0x10000,
	mac_int		= 0x20000,
	lpiis_irq = 0x400,
	time_stamp_irq = 0x0200,
	mmc_rx_csum_offload_irq = 0x0080,	
	mmc_rx_irq = 0x0020,
	mmc_irq = 0x0010,
	pmt_irq = 0x0008,
	pcs_ane_irq = 0x0004,
	pcs_link_irq = 0x0002,
	rgmii_irq = 0x0001,
};

enum power_event {
	pointer_reset = 0x80000000,
	global_unicast = 0x00000200,
	wake_up_rx_frame = 0x00000040,
	magic_frame = 0x00000020,
	wake_up_frame_en = 0x00000004,
	magic_pkt_en = 0x00000002,
	power_down = 0x00000001,
};

/* LPI control and status defines */
#define LPI_CTRL_STATUS_LPITXA	0x00080000	/* Enable LPI TX Automate */
#define LPI_CTRL_STATUS_PLSEN	0x00040000	/* Enable PHY Link Status */
#define LPI_CTRL_STATUS_PLS		0x00020000	/* PHY Link Status */
#define LPI_CTRL_STATUS_LPIEN	0x00010000	/* LPI Enable */
#define LPI_CTRL_STATUS_RLPIST	0x00000200	/* Receive LPI state */
#define LPI_CTRL_STATUS_TLPIST	0x00000100	/* Transmit LPI state */
#define LPI_CTRL_STATUS_RLPIEX	0x00000008	/* Receive LPI Exit */
#define LPI_CTRL_STATUS_RLPIEN	0x00000004	/* Receive LPI Entry */
#define LPI_CTRL_STATUS_TLPIEX	0x00000002	/* Transmit LPI Exit */
#define LPI_CTRL_STATUS_TLPIEN	0x00000001	/* Transmit LPI Entry */

#define GMAC_MAX_PERFECT_ADDRESSES	32

/* AN Configuration defines */
#define GMAC_AN_CTRL_RAN	0x00000200	/* Restart Auto-Negotiation */
#define GMAC_AN_CTRL_ANE	0x00001000	/* Auto-Negotiation Enable */
#define GMAC_AN_CTRL_ELE	0x00004000	/* External Loopback Enable */
#define GMAC_AN_CTRL_ECD	0x00010000	/* Enable Comma Detect */
#define GMAC_AN_CTRL_LR		0x00020000	/* Lock to Reference */
#define GMAC_AN_CTRL_SGMRAL	0x00040000	/* SGMII RAL Control */

/* AN Status defines */
#define GMAC_AN_STATUS_LS	0x00000004	/* Link Status 0:down 1:up */
#define GMAC_AN_STATUS_ANA	0x00000008	/* Auto-Negotiation Ability */
#define GMAC_AN_STATUS_ANC	0x00000020	/* Auto-Negotiation Complete */
#define GMAC_AN_STATUS_ES	0x00000100	/* Extended Status */

/* Register 54 (SGMII/RGMII status register) */
#define GMAC_S_R_GMII_LINK		0x8
#define GMAC_S_R_GMII_SPEED		0x5
#define GMAC_S_R_GMII_SPEED_SHIFT	0x1
#define GMAC_S_R_GMII_MODE		0x1
#define GMAC_S_R_GMII_SPEED_125		2
#define GMAC_S_R_GMII_SPEED_25		1

/* Common ADV and LPA defines */
#define GMAC_ANE_FD		(1 << 5)
#define GMAC_ANE_HD		(1 << 6)
#define GMAC_ANE_PSE		(3 << 7)
#define GMAC_ANE_PSE_SHIFT	7

/* GMAC Configuration defines */
#define GMAC_CONTROL_TC	0x01000000	/* Transmit Conf. in RGMII/SGMII */
#define GMAC_CONTROL_WD	0x00800000	/* Disable Watchdog on receive */
#define GMAC_CONTROL_JD	0x00020000	/* Jabber disable */
#define GMAC_CONTROL_BE	0x00200000	/* Frame Burst Enable */
#define GMAC_CONTROL_JE	0x00010000	/* Jumbo frame */
#define GMAC_CONTROL_IPG	0x07000000	/* Inter-Packet Gap */
#define GMAC_CONTROL_DCRS	0x00010000	/* Disable carrier sense */
#define GMAC_CONTROL_PS		0x00008000	/* Port Select 0:GMI 1:MII */
#define GMAC_CONTROL_FES	0x00004000	/* Speed 0:10 1:100 */
#define GMAC_CONTROL_DO		0x00000800	/* Disable Rx Own */
#define GMAC_CONTROL_LM		0x00001000	/* Loop-back mode */
#define GMAC_CONTROL_DM		0x00002000	/* Duplex Mode */
#define GMAC_CONTROL_IPC	0x08000000	/* Checksum Offload */
#define GMAC_CONTROL_DR		0x00000200	/* Disable Retry */
#define GMAC_CONTROL_LUD	0x00000100	/* Link up/down */
#define GMAC_CONTROL_ACS	0x00100000	/* Auto Pad/FCS Stripping */
#define GMAC_CONTROL_DC		0x00000010	/* Deferral Check */
#define GMAC_CONTROL_PRELEN	0x0000000C	/* Preamble Length for Transmit packets */
#define GMAC_CONTROL_TE		0x00000002	/* Transmitter Enable */
#define GMAC_CONTROL_RE		0x00000001	/* Receiver Enable */

#define GMAC_CTL_PS_SHIFT		0xf
#define GMAC_CTL_FES_SHIFT		0xe
#define GMAC_LINE_SPEED_MASK	0x3
#define GMAC_CONTROL_IPG_OFFSET	24	
#define GMAC_CONTROL_PRELEN_OFFSET	2	

#define GMAC_CORE_INIT (GMAC_CONTROL_JD  | GMAC_CONTROL_ACS | \
			GMAC_CONTROL_JE)
			
/* GMAC Frame Filter defines */
#define GMAC_FRAME_FILTER_PR	0x00000001	/* Promiscuous Mode */
#define GMAC_FRAME_FILTER_HUC	0x00000002	/* Hash Unicast */
#define GMAC_FRAME_FILTER_HMC	0x00000004	/* Hash Multicast */
#define GMAC_FRAME_FILTER_DAIF	0x00000008	/* DA Inverse Filtering */
#define GMAC_FRAME_FILTER_PM	0x00000010	/* Pass all multicast */
#define GMAC_FRAME_FILTER_DBF	0x00000020	/* Disable Broadcast frames */
#define GMAC_FRAME_FILTER_SAIF	0x00000100	/* Inverse Filtering */
#define GMAC_FRAME_FILTER_SAF	0x00000200	/* Source Address Filter */
#define GMAC_FRAME_FILTER_HPF	0x00000400	/* Hash or perfect Filter */
#define GMAC_FRAME_FILTER_RA	0x80000000	/* Receive all mode */
/* GMII ADDR  defines */
#define GMAC_MII_ADDR_WRITE	0x00000002	/* MII Write */
#define GMAC_MII_ADDR_BUSY	0x00000001	/* MII Busy */
/* GMAC FLOW CTRL defines */
#define GMAC_FLOW_CTRL_PT_MASK	0xffff0000	/* Pause Time Mask */
#define GMAC_FLOW_CTRL_PT_SHIFT	16
#define GMAC_FLOW_CTRL_RFE	0x00000001	/* Rx Flow Control Enable */
#define GMAC_FLOW_CTRL_TFE	0x00000002	/* Tx Flow Control Enable */
#define GMAC_FLOW_CTRL_FCB	0x00000001	/* Flow Control Busy ... */

/* DMA Bus Mode register defines */
#define DMA_BUS_MODE_SFT_RESET	0x00000001	/* Software Reset */
#define DMA_BUS_MODE_DA			0x00000002	/* Fixed Priority */
#define DMA_BUS_MODE_TAA		0x00000000	/* Fixed priority */
#define DMA_BUS_MODE_INTM		0x00010000	/* INTM = 1 */
#define DMA_BUS_MODE_DSL_MASK	0x0000007c	/* Descriptor Skip Length */
#define DMA_BUS_MODE_DSL_SHIFT	2		/*   (in DWORDS)      */

/* DMA Ctrol register defines*/
#define DMA_CTL_PBLX8		0x00010000	/* 8xPBL mode: */
#define DMA_CTL_DSL			0x0			/* Descriptor skip length = 0 */

/* DMA  CH0 tx control register defines*/
#define DMA_CHN_TXPBL		0x00100000	/* TXPBL = 16 */
#define	DMA_CHN_OSP			0x00000000	/* No second packet processing */
#define	DMA_CHN_TCW			0x00000000	/* TCW = 0 */
#define DMA_TX_START	0x00000001

/* DMA CH0 Rx control register defines*/
#define DMA_RX_BUF_SIZE	0x00001000
#define DMA_RX_PBL		0x00100000		/* RXPBL = 16 */
#define DMA_RX_START	0x00000001
#define	DMA_RX_RPF		0x00000000		/* No Rx Packet flush */

/* Programmable burst length (passed thorugh platform)*/
#define DMA_BUS_MODE_PBL_MASK	0x00003f00	/* Programmable Burst Len */
#define DMA_BUS_MODE_PBL_SHIFT	8
#define DMA_BUS_MODE_ATDS	0x00000080	/* Alternate Descriptor Size */

enum rx_tx_priority_ratio {
	double_ratio = 0x00004000,	/* 2:1 */
	triple_ratio = 0x00008000,	/* 3:1 */
	quadruple_ratio = 0x0000c000,	/* 4:1 */
};

#define DMA_SYSBUS_MODE_FB		0x00000001	/* Fixed burst enable */
#define DMA_SYSBUS_MODE_MB		0x00004000	/* Mixed burst */
#define DMA_BUS_MODE_RPBL_MASK	0x003e0000	/* Rx-Programmable Burst Len */
#define DMA_BUS_MODE_RPBL_SHIFT	17
#define DMA_BUS_MODE_USP	0x00800000
#define DMA_BUS_MODE_PBL	0x01000000
#define DMA_SYSBUS_MODE_AAL	0x00000000		/* No address-aligned Burst transfers */
#define DMA_SYSBUS_MODE_BLEN16	0x00000008
#define	DMA_SYSBUS_MODE_WR_OSR_LMT 0x03000000	/* WR_OSR_LMT=3 */
#define	DMA_SYSBUS_MODE_RD_OSR_LMT 0x00030000	/* RD_OSR_LMT=3 */
#define	DMA_SYSBUS_MODE_RB 	0x00000000			/* no rebuild burst */


/* DMA CRS Control and Status Register Mapping */
#define DMA_HOST_TX_DESC	  0x00001048	/* Current Host Tx descriptor */
#define DMA_HOST_RX_DESC	  0x0000104c	/* Current Host Rx descriptor */
/*  DMA Bus Mode register defines */
#define DMA_BUS_PR_RATIO_MASK	  0x0000c000	/* Rx/Tx priority ratio */
#define DMA_BUS_PR_RATIO_SHIFT	  14
#define DMA_BUS_FB	  	  0x00010000	/* Fixed Burst */

/* DMA operation mode defines (start/stop tx/rx are placed in common header)*/
/* Disable Drop TCP/IP csum error */
#define DMA_CONTROL_DT		0x04000000
#define DMA_CONTROL_RSF		0x02000000	/* Receive Store and Forward */
#define DMA_CONTROL_DFF		0x01000000	/* Disaable flushing */
/* Threshold for Activating the FC */
enum rfa {
	act_full_minus_1 = 0x00800000,
	act_full_minus_2 = 0x00800200,
	act_full_minus_3 = 0x00800400,
	act_full_minus_4 = 0x00800600,
};
/* Threshold for Deactivating the FC */
enum rfd {
	deac_full_minus_1 = 0x00400000,
	deac_full_minus_2 = 0x00400800,
	deac_full_minus_3 = 0x00401000,
	deac_full_minus_4 = 0x00401800,
};
#define DMA_CONTROL_TSF	0x00200000	/* Transmit  Store and Forward */

enum ttc_control {
	DMA_CONTROL_TTC_64 = 0x00000000,
	DMA_CONTROL_TTC_128 = 0x00004000,
	DMA_CONTROL_TTC_192 = 0x00008000,
	DMA_CONTROL_TTC_256 = 0x0000c000,
	DMA_CONTROL_TTC_40 = 0x00010000,
	DMA_CONTROL_TTC_32 = 0x00014000,
	DMA_CONTROL_TTC_24 = 0x00018000,
	DMA_CONTROL_TTC_16 = 0x0001c000,
};
#define DMA_CONTROL_TC_TX_MASK	0xfffe3fff

#define DMA_CONTROL_EFC		0x00000100
#define DMA_CONTROL_FEF		0x00000080
#define DMA_CONTROL_FUF		0x00000040

enum rtc_control {
	DMA_CONTROL_RTC_64 = 0x00000000,
	DMA_CONTROL_RTC_32 = 0x00000008,
	DMA_CONTROL_RTC_96 = 0x00000010,
	DMA_CONTROL_RTC_128 = 0x00000018,
};
#define DMA_CONTROL_TC_RX_MASK	0xffffffe7

#define DMA_CONTROL_OSF	0x00000004	/* Operate on second frame */

/* MMC registers offset */
#define GMAC_MMC_CTRL      0x100
#define GMAC_MMC_RX_INTR   0x104
#define GMAC_MMC_TX_INTR   0x108
#define GMAC_MMC_RX_CSUM_OFFLOAD   0x208

extern const struct stmmac_dma_ops dwmac1000_dma_ops;
extern const struct stmmac_mtl_ops dwmac1000_mtl_ops;

#endif /* __DWMAC1000_H__ */
