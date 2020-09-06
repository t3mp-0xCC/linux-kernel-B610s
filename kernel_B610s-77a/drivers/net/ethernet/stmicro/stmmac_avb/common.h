/*******************************************************************************
  STMMAC Common Header File

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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/phy.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#define STMMAC_VLAN_TAG_USED
#include <linux/if_vlan.h>
#endif

#include "descs.h"
#include "mmc.h"
#include "dwmac1000.h"
#include "stmmac_ptp.h"

#define CHIP_DEBUG_PRINT
/* Turn-on extra printk debug for MAC core, dma and descriptors */
/* #define CHIP_DEBUG_PRINT */

#ifdef CHIP_DEBUG_PRINT
#define CHIP_DBG(fmt, args...)  printk(fmt, ## args)
#else
#define CHIP_DBG(fmt, args...)  do { } while (0)
#endif

/* Synopsys Core versions */
#define	DWMAC_CORE_3_40	0x34
#define	DWMAC_CORE_3_50	0x35
#define	DWMAC_CORE_4_10	0x41

/* tx and rx channel number */
#define TX_CHN_NR		3		/* Tx channel number, it should bigger than Rx */
#define RX_CHN_NR		1		/* Rx channel number */
#define TX_CFG_CHN_NR	2		/* Only 2 channels can be configured */
#define TX_CHN_NET		0
#define TX_CHN_CLASS_A	2
#define	TX_CHN_CLASS_B	1
#define RX_CHN			0

#define	EVENT_NUM_MASK		0xe	/* mask bit0 */
#define SLOTNUM_MASK		0xf	/* bit[3:0] */

/* Get the right bit field.
* For example: data=0b0000 1010 0100 1011, lbit = 3, hbit = 5
* the result is 0b001.
*/
#define GET_VALUE(data, lbit, hbit) ((data >>lbit) & (~(~0<<(hbit-lbit+1))))
#define GET_RX_QUEUE(x, index) ((x)->rx_queue + (index))
#define GET_TX_QUEUE(x, index) ((x)->tx_queue + (index))
#define GET_RX_STAT(x, index) ((x)->rx_stat + (index))

#undef FRAME_FILTER_DEBUG
/* #define FRAME_FILTER_DEBUG */

struct stmmac_extra_stats {
	/* Transmit errors */
	unsigned long tx_underflow ____cacheline_aligned;
	unsigned long tx_carrier;
	unsigned long tx_losscarrier;
	unsigned long vlan_tag;
	unsigned long tx_deferred;
	unsigned long tx_vlan;
	unsigned long tx_jabber;
	unsigned long tx_frame_flushed;
	unsigned long tx_payload_error;
	unsigned long tx_ip_header_error;
	
	/*  gmac test    */
	unsigned long tx_desc_full[TX_CHN_NR];
	unsigned long enter_xmit[TX_CHN_NR];
	unsigned long xmit_td_full[TX_CHN_NR];
	unsigned long gmac_suspend;
	unsigned long gmac_release;
	unsigned long tx_avail[TX_CHN_NR];
	/* skb psh flag */
	unsigned long rx_psh_count;
	unsigned long rx_skb_count;

	/* packet related */
	unsigned long tx_pkt_cnt[TX_CHN_NR];

	/* avb related */
	unsigned long dma_chn_int[TX_CHN_NR];
	unsigned long mac_int;
	
	/* Receive errors */
	unsigned long rx_desc;
	unsigned long sa_filter_fail;
	unsigned long overflow_error;
	unsigned long ipc_csum_error;
	unsigned long rx_collision;
	unsigned long rx_crc;
	unsigned long dribbling_bit;
	unsigned long rx_length;
	unsigned long rx_err;
	unsigned long rx_multicast;
	unsigned long rx_gmac_overflow;
	unsigned long rx_watchdog;
	unsigned long da_rx_filter_fail;
	unsigned long sa_rx_filter_fail;
	unsigned long rx_missed_cntr;
	unsigned long rx_overflow_cntr;
	unsigned long rx_vlan;
	unsigned long giant_pkt;
	/* Tx/Rx IRQ error info */
	unsigned long tx_stopped[TX_CHN_NR];
	unsigned long tx_undeflow_irq;	 /* deleted */
	unsigned long tx_jabber_irq;
	unsigned long rx_overflow_irq;
	unsigned long rx_buf_unav_irq[RX_CHN_NR];
	unsigned long rx_process_stopped_irq[RX_CHN_NR];
	unsigned long rx_watchdog_irq[RX_CHN_NR];
	unsigned long tx_early_irq[TX_CHN_NR];
	unsigned long fatal_bus_error_irq[TX_CHN_NR];
	unsigned long rx_watchdog_timeout;
	unsigned long tx_jabber_timeout;
	unsigned long tx_ctx_desc_err[TX_CHN_NR];
	unsigned long rx_dma_err[RX_CHN_NR];
	unsigned long tx_dma_err[TX_CHN_NR];
	unsigned long tx_buf_unavail[TX_CHN_NR];
	unsigned long tx_clean_own[TX_CHN_NR];
	unsigned long tx_chn_err;
	unsigned long tx_mac_header_err;
	/* Tx/Rx IRQ Events */
	unsigned long rx_early_irq[RX_CHN_NR];
	unsigned long threshold;
	unsigned long tx_pkt_n[TX_CHN_NR];
	unsigned long rx_pkt_n;
	unsigned long normal_irq_n[TX_CHN_NR];
	unsigned long rx_normal_irq_n[RX_CHN_NR];
	unsigned long napi_poll;
	unsigned long tx_normal_irq_n[TX_CHN_NR];
	unsigned long tx_clean[TX_CHN_NR];
	unsigned long tx_reset_ic_bit[TX_CHN_NR];
	unsigned long irq_receive_pmt_irq_n;
	unsigned long tx_status_int;
	unsigned long phy_int;
	/* MMC info */
	unsigned long mmc_tx_irq_n;
	unsigned long mmc_rx_irq_n;
	unsigned long mmc_rx_csum_offload_irq_n;
	/* EEE */
	unsigned long irq_tx_path_in_lpi_mode_n;
	unsigned long irq_tx_path_exit_lpi_mode_n;
	unsigned long irq_rx_path_in_lpi_mode_n;
	unsigned long irq_rx_path_exit_lpi_mode_n;
	unsigned long phy_eee_wakeup_error_n;
	/* Extended RDES status */
	unsigned long ip_hdr_err;
	unsigned long ip_payload_err;
	unsigned long ip_csum_bypassed;
	unsigned long ipv4_pkt_rcvd;
	unsigned long ipv6_pkt_rcvd;
	unsigned long rx_msg_type_ext_no_ptp;
	unsigned long rx_msg_type_sync;
	unsigned long rx_msg_type_follow_up;
	unsigned long rx_msg_type_delay_req;
	unsigned long rx_msg_type_delay_resp;
	unsigned long rx_msg_type_pdelay_req;
	unsigned long rx_msg_type_pdelay_resp;
	unsigned long rx_msg_type_pdelay_follow_up;
	unsigned long ptp_frame_type;
	unsigned long ptp_ver;
	unsigned long timestamp_dropped;
	unsigned long av_pkt_rcvd;
	unsigned long av_tagged_pkt_rcvd;
	unsigned long vlan_tag_priority_val;
	unsigned long l3_filter_match;
	unsigned long l4_filter_match;
	unsigned long l3_l4_filter_no_match;
	/* PCS */
	unsigned long irq_pcs_ane_n;
	unsigned long irq_pcs_link_n;
	unsigned long irq_rgmii_n;
	unsigned long pcs_link;
	unsigned long pcs_duplex;
	unsigned long pcs_speed;

	/*MTL interrupt */
	unsigned long mtl_int[TX_CHN_NR];

	/* timestamp related */
	unsigned long timestamp_irq;

	unsigned long rgmii_link_changed;
	
};

/* CSR Frequency Access Defines*/
#define CSR_F_35M	35000000
#define CSR_F_60M	60000000
#define CSR_F_100M	100000000
#define CSR_F_150M	150000000
#define CSR_F_250M	250000000
#define CSR_F_300M	300000000

#define	MAC_CSR_H_FRQ_MASK	0x20

#define HASH_TABLE_SIZE 64
#define PAUSE_TIME 0x200

/* Flow Control defines */
#define FLOW_OFF	0
#define FLOW_RX		1
#define FLOW_TX		2
#define FLOW_AUTO	(FLOW_TX | FLOW_RX)

/* PCS defines */
#define STMMAC_PCS_RGMII	(1 << 0)
#define STMMAC_PCS_SGMII	(1 << 1)
#define STMMAC_PCS_TBI		(1 << 2)
#define STMMAC_PCS_RTBI		(1 << 3)

#define SF_DMA_MODE 1		/* DMA STORE-AND-FORWARD Operation Mode */

/* DMA HW feature0 register fields */
#define DMA_HW_FEAT_MIISEL	0x00000001	/* 10/100 Mbps Support */
#define DMA_HW_FEAT_GMIISEL	0x00000002	/* 1000 Mbps Support */
#define DMA_HW_FEAT_HDSEL	0x00000004	/* Half-Duplex Support */
#define DMA_HW_FEAT_PCSSEL	0x00000008	/* PCS Registers (TBI, SGMII, or RTBI PHY interface) */
#define DMA_HW_FEAT_VLHASH	0x00000010	/* VLAN Hash Filter */
#define DMA_HW_FEAT_SMASEL	0x00000020	/* SMA (MDIO) Interface */
#define DMA_HW_FEAT_RWKSEL	0x00000040	/* PMT Remote Wake-up Packet */
#define DMA_HW_FEAT_MGKSEL	0x00000080	/* PMT Magic Packet */
#define DMA_HW_FEAT_MMCSEL	0x00000100	/* RMON Module */
#define DMA_HW_FEAT_ARPOFFSEL	0x00000200	/* ARP Offload */
#define DMA_HW_FEAT_TSSEL	0x00001000	/* IEEE 1588-2008 Timestamp */
#define DMA_HW_FEAT_EEESEL	0x00002000	/* Energy Efficient Ethernet */
#define DMA_HW_FEAT_TXCOESEL	0x00004000	/* Transmit Checksum Offload */
#define DMA_HW_FEAT_RXCOESEL	0x00010000	/*Receive Checksum Offload */
#define DMA_HW_FEAT_ADDMACADRSEL	0x007c0000	/* MAC Addresses 1-31 */
#define DMA_HW_FEAT_MACADR32SEL	0x00800000	/* MAC Addresses 32-63 */
#define DMA_HW_FEAT_MACADR64SEL	0x01000000	/* MAC Addresses 64-127 */
#define DMA_HW_FEAT_TSSTSSEL	0x06000000	/* Timestamp System Time Source */
#define DMA_HW_FEAT_SAVLANINS	0x08000000	/* Source Address or VLAN Insertion Enable */
#define DMA_HW_FEAT_ACTPHYSEL	0x70000000	/* Active PHY Selected */

/* DMA HW feature1 register fields */
#define DMA_HW_FEAT_RXFIFOSIZE	0x0000001F	/* MTL Receive FIFO Size */
#define DMA_HW_FEAT_TXFIFOSIZE	0x000007C0	/* MTL Transmit FIFO Size */
#define DMA_HW_FEAT_OSTEN		0x00000800	/* One-Step Timestamping */
#define DMA_HW_FEAT_PTOEN		0x00001000	/* PTP Offload Enable */
#define DMA_HW_FEAT_ADVTHWORD	0x00002000	/* IEEE 1588 High Word Register */
#define DMA_HW_FEAT_ADDR64		0x0000C000	/* Address Width */
#define DMA_HW_FEAT_DCBEN 		0x00010000	/* DCB Feature Enable */
#define DMA_HW_FEAT_SPHEN		0x00020000	/* Split Header Feature Enable */
#define DMA_HW_FEAT_TSOEN		0x00040000	/* TCP Segmentation Offload Enable */
#define DMA_HW_FEAT_DBGMEMA 	0x00080000	/* Debug Memory Interface Enabled */
#define DMA_HW_FEAT_AVSEL		0x00100000	/* AV Feature Enabled */
#define DMA_HW_FEAT_HASHTBLSZ	0x03000000	/* Hash Table Size */
#define DMA_HW_FEAT_L3L4FNUM	0x38000000	/* Total number of L3 or L4 Filters */

/* DMA HW feature2 register fields */
#define DMA_HW_FEAT_RXQCNT 		0x0000000F	/* Number of MTL Receive Queues */
#define DMA_HW_FEAT_TXQCNT 		0x000003C0	/* Number of MTL Transmit Queues */
#define DMA_HW_FEAT_RXCHCNT 	0x0000F000	/* Number of DMA Receive Channels */
#define DMA_HW_FEAT_TXCHCNT 	0x003C0000	/* Number of DMA Transmit Channels */
#define DMA_HW_FEAT_PPSOUTNUM 	0x07000000	/* Number of PPS Outputs */
#define DMA_HW_FEAT_AUXSNAPNUM 	0x70000000	/* Number of Auxiliary Snapshot Inputs */ 	

/* Timestamping with Internal System Time */
#define DMA_HW_FEAT_INTTSEN	0x02000000
#define DMA_HW_FEAT_FLEXIPPSEN	0x04000000	/* Flexible PPS Output */
#define DMA_HW_FEAT_ACTPHYIF	0x70000000	/* Active/selected PHY iface */

/* Max/Min RI Watchdog Timer count value */
#define MAX_DMA_RIWT		0xff
#define MIN_DMA_RIWT		0x3
/* Tx coalesce parameters */
#define STMMAC_COAL_TX_TIMER	40000
#define STMMAC_MAX_COAL_TX_TICK	100000
#define STMMAC_TX_MAX_FRAMES	256
#define STMMAC_TX_FRAMES	64

/* Rx IPC status */
enum rx_frame_status {
	good_frame = 0,
	discard_frame = 1,
	csum_none = 2,
	llc_snap = 4,
};

enum dma_irq_status {
	tx_hard_error = 0x1,
	handle_rx = 0x2,
	handle_tx = 0x4,
	rx_buf_uavi = 0x8,	
};

#define	CORE_IRQ_TX_PATH_IN_LPI_MODE	(1 << 1)
#define	CORE_IRQ_TX_PATH_EXIT_LPI_MODE	(1 << 2)
#define	CORE_IRQ_RX_PATH_IN_LPI_MODE	(1 << 3)
#define	CORE_IRQ_RX_PATH_EXIT_LPI_MODE	(1 << 4)

#define	CORE_PCS_ANE_COMPLETE		(1 << 5)
#define	CORE_PCS_LINK_STATUS		(1 << 6)
#define	CORE_RGMII_IRQ			(1 << 7)

struct rgmii_adv {
	unsigned int pause;
	unsigned int duplex;
	unsigned int lp_pause;
	unsigned int lp_duplex;
};

#define STMMAC_PCS_PAUSE	1
#define STMMAC_PCS_ASYM_PAUSE	2

/* DMA HW capabilities */
struct dma_features {
	unsigned int mbps_10_100;
	unsigned int mbps_1000;
	unsigned int half_duplex;
	unsigned int pcs;
	unsigned int vlan_hash_filter;
	unsigned int sma_mdio;
	unsigned int pmt_remote_wake_up;
	unsigned int pmt_magic_frame;
	unsigned int rmon;
	unsigned int arp_offload;

	/* IEEE 1588-2008 */
	unsigned int time_stamp;
	
	/* 802.3az - Energy-Efficient Ethernet (EEE) */
	unsigned int eee;
	
	/* TX and RX csum */
	unsigned int tx_coe;
	unsigned int rx_coe;

	unsigned int multi_addr;
	unsigned int multi_addr32;
	unsigned int multi_addr64;
	
	/*Timestamp System Time Source*/
	unsigned int systime_source;
	
	/*Source Address or VLAN Insertion*/
	unsigned int sa_vlan_ins;

	/*active PHY*/
	unsigned int phy_mode;

	/*HW feature1*/
	unsigned int rx_fifo_size;
	unsigned int tx_fifo_size;
	unsigned int one_step_ts;
	unsigned int ptp_offload;
	unsigned int high_word_reg;
	unsigned int addr_width;
	unsigned int dcb_enable;
	unsigned int split_hdr;
	unsigned int tso_en;
	unsigned int debug_mem_if;
	unsigned int av_en;
	unsigned int hash_table_size;
	unsigned int l3l4_total_num;

	/*HW feature2*/
	unsigned int num_rx_queue;
	unsigned int num_tx_queue;
	/* TX and RX number of channels */
	unsigned int num_rx_channel;
	unsigned int num_tx_channel;
	unsigned int num_pps_output;
	unsigned int num_aux_snap;
};

struct stmmac_time_spec {
	unsigned int sec;
	unsigned int nsec;
};

/* GMAC TX FIFO is 8K, Rx FIFO is 16K */
#define BUF_SIZE_16KiB 16384
#define BUF_SIZE_8KiB 8192
#define BUF_SIZE_4KiB 4096
#define BUF_SIZE_2KiB 2048
#define BUF_SIZE_1_8kiB 1800

/* Power Down and WOL */
#define PMT_NOT_SUPPORTED 0
#define PMT_SUPPORTED 1

/* Default LPI timers */
#define STMMAC_DEFAULT_LIT_LS	0x3E8
#define STMMAC_DEFAULT_TWT_LS	0x0

struct stmmac_desc_ops {
	/* DMA RX descriptor ring initialization */
	void (*init_rx_desc) (union dma_desc *p, int disable_rx_ic);
	/* DMA TX descriptor ring initialization */
	void (*init_tx_desc) (union dma_desc *p);

	/* Invoked by the xmit function to prepare the tx descriptor */
	void (*prepare_tx_desc) (union dma_desc *p, int is_fs, int len,
				 int csum_flag);
	/* Set/get the owner of the descriptor */
	void (*set_tx_owner) (union dma_desc *p);
	int (*get_tx_owner) (union dma_desc *p);
	/*Disable Rx interruption on completion*/
	void (*set_rx_ioc)(union dma_desc *p, int ioc_en);
	/* Invoked by the xmit function to close the tx descriptor */
	void (*close_tx_desc) (union dma_desc *p);
	/* Clean the tx descriptor as soon as the tx irq is received */
	void (*release_tx_desc) (union dma_desc *p);
	/* Clear interrupt on tx frame completion. When this bit is
	 * set an interrupt happens as soon as the frame is transmitted */
	void (*clear_tx_ic) (union dma_desc *p);
	/* Last tx segment reports the transmit status */
	int (*get_tx_ls) (union dma_desc *p);
	/* Return the transmit status looking at the TDES1 */
	int (*tx_status) (void *data, struct stmmac_extra_stats *x,
			  union dma_desc *p, void __iomem *ioaddr, int chn);
	/* Get the buffer size from the descriptor */
	int (*get_tx_len) (union dma_desc *p);
	/* Handle extra events on specific interrupts hw dependent */
	int (*get_rx_owner) (union dma_desc *p);
	void (*set_rx_owner) (union dma_desc *p);
	/* Get the receive frame size */
	int (*get_rx_frame_len) (union dma_desc *p);
	/* Return the reception status looking at the RDES1 */
	int (*rx_status) (void *data, struct stmmac_extra_stats *x,
			  union dma_desc *p);
	//void (*rx_extended_status) (void *data, struct stmmac_extra_stats *x,
				    //struct dma_extended_desc *p);
	/* Set tx timestamp enable bit */
	void (*enable_tx_timestamp) (union dma_desc *p);
	/* get tx timestamp status */
	int (*get_tx_timestamp_status) (union dma_desc *p);
	/* get timestamp value */
	void (*get_timestamp) (void *desc, struct stmmac_time_spec *time);
	/* get rx timestamp status */
	int (*get_rx_timestamp_status) (void *desc, u32 ats);

	void (*config_tx_slotnum) (int chn, union dma_desc *desc);
};

struct stmmac_dma_ops {
	/* DMA core initialization */
	int (*init) (int fb, int mb,int burst_len);
	/* Dump DMA registers */
	void (*dump_regs) (void __iomem *ioaddr);
	/* Set tx/rx threshold in the csr6 register
	 * An invalid value enables the store-and-forward mode */
	void (*dma_mode) (void __iomem *ioaddr, int txmode, int rxmode);
	/* To track extra statistic (if supported) */
	void (*dma_diagnostic_fr) (void *data, struct stmmac_extra_stats *x,
				   void __iomem *ioaddr);
	void (*enable_dma_transmission)(int chn, unsigned int index);
	void (*enable_dma_irq) (void __iomem *ioaddr, int chn);
	void (*disable_dma_irq) (void __iomem *ioaddr, int chn);
	void (*start_tx) (void __iomem *ioaddr, int chn);
	void (*stop_tx) (void __iomem *ioaddr, int chn);
	void (*start_rx) (void __iomem *ioaddr, int chn);
	void (*stop_rx) (void __iomem *ioaddr, int chn);
	int (*dma_interrupt) (void __iomem *ioaddr,struct stmmac_extra_stats *x, 
		int chn);
	/* If supported then get the optional core features */
	unsigned int (*get_hw_feature) (void __iomem *ioaddr, 
					unsigned int *hw_feature1, unsigned int *hw_feature2);
	/* Program the HW RX Watchdog */
	void (*rx_watchdog) (void __iomem *ioaddr, unsigned int riwt, int chn);

	/* Get GMAC interrupt status */
	int (*get_dma_isr) (void __iomem *ioaddr);

	/* Get rx tail pointer */
	dma_addr_t (*get_rx_tail_ptr) (unsigned int index, int chn);

	/* Set rx tail pointer */
	void (*set_rx_tail_ptr) (unsigned int index, int chn);

	/* Get tx reference slot number */
	int (*get_tx_slotnum) (void __iomem *ioaddr, int chn);
};

struct stmmac_ops {
	/* MAC core initialization */
	int (*core_init) (void __iomem *ioaddr);
	/* Enable and verify that the IPC module is supported */
	int (*rx_ipc) (void __iomem *ioaddr);
	/* Dump MAC registers */
	void (*dump_regs) (void __iomem *ioaddr);
	/* Handle extra events on specific interrupts hw dependent */
	int (*handle_irq) (void __iomem *ioaddr,
				struct stmmac_extra_stats *x);
	/* Multicast filter setting */
	void (*set_filter) (struct net_device *dev, int id);
	/* Flow control setting */
	void (*flow_ctrl) (void __iomem *ioaddr, unsigned int duplex,
			   unsigned int fc, unsigned int pause_time, int chn);
	/* Set power management mode (e.g. magic frame) */
	void (*pmt) (void __iomem *ioaddr, unsigned long mode);
	/* Set/Get Unicast MAC addresses */
	void (*set_umac_addr) (void __iomem *ioaddr, unsigned char *addr,
			       unsigned int reg_n);
	void (*get_umac_addr) (void __iomem *ioaddr, unsigned char *addr,
			       unsigned int reg_n);
	void (*set_eee_mode) (void __iomem *ioaddr);
	void (*reset_eee_mode) (void __iomem *ioaddr);
	void (*set_eee_timer) (void __iomem *ioaddr, int ls, int tw);
	void (*set_eee_pls) (void __iomem *ioaddr, int link);
	void (*ctrl_ane) (void __iomem *ioaddr, bool restart);
	void (*get_adv) (void __iomem *ioaddr, struct rgmii_adv *adv);

	/* Get Ethernet line speed */
	int (*get_line_spped)(void __iomem *ioaddr);
	int (*get_mac_ipg)(void __iomem *ioaddr);
	int (*get_mac_preamble)(void __iomem *ioaddr);
};

struct stmmac_mtl_ops {
	/* MTL Initialization */
	void (*mtl_init)(void);
};

struct stmmac_hwtimestamp {
	int (*init)(void __iomem *ioaddr);
	void (*config_hw_tstamping) (void __iomem *ioaddr, u32 data);
	void (*config_sub_second_increment) (void __iomem *ioaddr);
	int (*init_systime) (void __iomem *ioaddr, u32 sec, u32 nsec);
	int (*config_addend) (void __iomem *ioaddr, u32 addend);
	int (*adjust_systime) (void __iomem *ioaddr, u32 sec, u32 nsec, int add_sub);
	u64(*get_systime) (void __iomem *ioaddr);
	void (*config_updt_method)(void __iomem *ioaddr, unsigned int method);	
	int (*ptp_ioctl)(struct net_device *ndev, struct ifreq *ifr, int cmd);
};

struct mac_link {
	int port;
	int duplex;
	int speed;
};

struct mii_regs {
	unsigned int addr;	/* MII Address */
	unsigned int data;	/* MII Data */
};

struct stmmac_ring_mode_ops {
	unsigned int (*is_jumbo_frm) (int len);
	unsigned int (*jumbo_frm) (struct sk_buff *skb, int csum, int chn);
	void (*init_desc3) (union dma_desc *p);
	int (*set_16kib_bfsize) (int mtu);
};

struct stmmac_slot_ops {
	void (*config_slot_compare) (void __iomem *ioaddr, int chn,
									unsigned int compare);
	void (*config_slot_advance_check) (void __iomem *ioaddr, int chn, 
										unsigned int check);
};

/* Credit-based Shaper */
struct stmmac_cbs_ops {
	void (*config_idle_slope) (int chn, void __iomem *ioaddr, unsigned int idle_slope);
	void (*config_send_slope) (int chn, void __iomem *ioaddr, unsigned int send_slope);
	void (*config_high_credit) (int chn, void __iomem *ioaddr, unsigned int high_credit);
	void (*config_low_credit) (int chn, void __iomem *ioaddr, unsigned int low_credit);
	void (*set_avb_algorithm) (int chn, void __iomem *ioaddr, unsigned int algorithm);
	int (*get_locredit)(struct net_device *ndev, unsigned int class_id);
	int (*get_hicredit)(struct net_device *ndev, unsigned int class_id);
	int (*get_avb_algorithm)(struct net_device *ndev, unsigned int class_id);
	int (*get_preamble_ipg)(struct net_device *ndev);
	int (*set_class_pcp)(struct net_device *ndev, unsigned int class_id, unsigned int pcp);
	int (*get_class_pcp)(struct net_device *ndev, unsigned int class_id, unsigned int *pcp);
};

struct mac_device_info {
	const struct stmmac_ops *mac;
	struct stmmac_desc_ops *desc;
	const struct stmmac_dma_ops *dma;
	const struct stmmac_ring_mode_ops *ring;
	const struct stmmac_hwtimestamp *ptp;
	const struct stmmac_mtl_ops *mtl;
	const struct stmmac_slot_ops *slot;
	const struct stmmac_cbs_ops *cbs;
	struct mii_regs mii;	/* MII register Addresses */
	struct mac_link link;
	unsigned int synopsys_uid;
};

struct mac_device_info *dwmac1000_setup(void __iomem *ioaddr);
struct mac_device_info *dwmac100_setup(void __iomem *ioaddr);

extern void stmmac_set_mac_addr(void __iomem *ioaddr, u8 addr[6],
				unsigned int high, unsigned int low);
extern void stmmac_get_mac_addr(void __iomem *ioaddr, unsigned char *addr,
				unsigned int high, unsigned int low);

extern void stmmac_set_mac(void __iomem *ioaddr, bool enable);

extern void dwmac_dma_flush_tx_fifo(void __iomem *ioaddr);
extern const struct stmmac_ring_mode_ops ring_mode_ops;
extern struct stmmac_priv *gmac_priv;
#endif /* __COMMON_H__ */
