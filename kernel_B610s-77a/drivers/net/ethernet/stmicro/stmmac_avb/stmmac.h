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

#ifndef __STMMAC_H__
#define __STMMAC_H__

#define STMMAC_RESOURCE_NAME   "stmmaceth"
#define DRV_MODULE_VERSION	"March_2013"

#include <linux/clk.h>
#include <linux/stmmac_avb.h>
#include <linux/phy.h>
#include <linux/pci.h>
#include "common.h"
#include <linux/ptp_clock_kernel.h>

struct gmac_rx_stat {
	unsigned int rx_bytes;
	unsigned int rx_cnt;	
	unsigned int rx_last_bytes;
};

struct gmac_rx_queue {
	union dma_desc *dma_rx	;
	struct sk_buff **rx_skbuff;
	unsigned int cur_rx;
	unsigned int dirty_rx;
	unsigned int dma_rx_size;
	unsigned int rx_riwt;
	unsigned int hwts_rx_en;
	dma_addr_t *rx_skbuff_dma;
	dma_addr_t dma_rx_phy;
	dma_addr_t dma_rx_tail;
	unsigned int use_riwt;

	unsigned int mtl_fifo_size;
	unsigned int rx_bytes;
	unsigned int rx_pkt_cnt;	
};

struct gmac_tx_queue {
	union dma_desc *dma_tx;			//dma desc queue
	struct sk_buff **tx_skbuff;		//tx skb buffer pointer queue
	unsigned int cur_tx;			//application write pointer
	unsigned int dirty_tx;			//dma read and write back pointer
	unsigned int dma_tx_size;		//dma tx desc queue size
	unsigned int tx_count_frames;			//tx counter for the IC bit
	unsigned int tx_coal_frames;				//the coalesce parameter for the IC bit
	unsigned int tx_coal_timer;
	dma_addr_t *tx_skbuff_dma;		//tx  skb buffer's dma addresses
	dma_addr_t dma_tx_phy;			//dma tx desc phy addresses
	dma_addr_t dma_tx_tail;			//dma tx desc tail pointer
	//int tx_coalesce;
	unsigned int hwts_tx_en;					//hardware timestamp tx enable
	spinlock_t tx_lock;				
	bool tx_path_in_lpi_mode;
	struct timer_list txtimer;		//tx coalesce timer.

	unsigned int mtl_fifo_size;
	unsigned int tx_bytes;
	unsigned int tx_pkt_cnt;
};

struct stmmac_priv {
	struct gmac_tx_queue *tx_queue;
	unsigned int tx_queue_cnt;
	
	struct gmac_rx_queue *rx_queue;
	unsigned int rx_queue_cnt;
	unsigned int rx_queue_id;		//It indicates which rx channel is used.

	struct gmac_rx_stat *rx_stat;	//It indicates received packets of various channel.
	unsigned int enable_rate;
	unsigned int time;
	unsigned int avb_support;
	spinlock_t rx_lock;
	
	unsigned int dma_buf_sz;
	struct napi_struct napi ____cacheline_aligned_in_smp;

	int hw_cap_support;
	void __iomem *ioaddr;
	struct net_device *dev;
	struct device *device;
	struct mac_device_info *hw;
	int no_csum_insertion;
	spinlock_t lock;

	struct phy_device *phydev ____cacheline_aligned_in_smp;
	int oldlink;
	int speed;
	int oldduplex;
	unsigned int flow_ctrl;
	unsigned int pause;
	struct mii_bus *mii;
	int mii_irq[PHY_MAX_ADDR];

	struct stmmac_extra_stats xstats ;//____cacheline_aligned_in_smp;
	struct plat_stmmacenet_data *plat;
	struct dma_features dma_cap;
	struct stmmac_counters mmc;
	int synopsys_id;
	unsigned int msg_enable;
	int wolopts;
	int wol_irq;
	int ch0_txirq;
	int ch1_txirq;
	int ch2_txirq;
	int rx_irq;
	struct clk *stmmac_clk;
	int clk_csr;
	struct timer_list eee_ctrl_timer;
	int lpi_irq;
	int eee_enabled;
	int eee_active;
	int tx_lpi_timer;
	int pcs;

	/* ptp related */
	struct ptp_clock *ptp_clock;
	struct ptp_clock_info ptp_clock_ops;
	unsigned int default_addend;
	unsigned int ptp_cnt;
	unsigned int ptp_bytes;
	u32 adv_ts;
	spinlock_t ptp_lock;

	/* cbs related */
	unsigned int pcp_class_a;	// PCP's Class A vlaue
	unsigned int pcp_class_b;	// PCP's Class B value
	unsigned int line_speed;	//1000Mbps or 100Mbps

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
    struct sk_buff_head free_q;
    unsigned int portno;
#endif
};

extern int phyaddr;

extern int stmmac_mdio_unregister(struct net_device *ndev);
extern int stmmac_mdio_register(struct net_device *ndev);
extern void stmmac_set_ethtool_ops(struct net_device *netdev);
extern const struct stmmac_desc_ops enh_desc_ops;
extern struct stmmac_desc_ops ndesc_ops;
extern const struct stmmac_hwtimestamp stmmac_ptp;
extern const struct stmmac_slot_ops stmmac_slot;
extern const struct stmmac_cbs_ops stmmac_cbs;
extern int stmmac_ptp_register(struct stmmac_priv *priv);
extern void stmmac_ptp_unregister(struct stmmac_priv *priv);
int stmmac_freeze(struct net_device *ndev);
int stmmac_restore(struct net_device *ndev);
int stmmac_resume(struct net_device *ndev);
int stmmac_suspend(struct net_device *ndev);
int stmmac_dvr_remove(struct net_device *ndev);
struct stmmac_priv *stmmac_dvr_probe(struct device *device,
				     struct plat_stmmacenet_data *plat_dat,
				     void __iomem *addr);
void stmmac_disable_eee_mode(struct stmmac_priv *priv);
bool stmmac_eee_init(struct stmmac_priv *priv);
int stmmac_clk_enable(struct device *dev);
void stmmac_clk_disable(struct device *dev);
#ifdef CONFIG_BALONG_AVB
int stmmac_phy_read(int phyaddr, int phyreg);
int stmmac_phy_write(int phyaddr, int phyreg, u16 phydata);
#endif

#ifdef CONFIG_STMMAC_PLATFORM
extern struct platform_driver stmmac_pltfr_driver;
static inline int stmmac_register_platform(void)
{
	int err;

	err = platform_driver_register(&stmmac_pltfr_driver);
	if (err)
		pr_err("stmmac: failed to register the platform driver\n");

	return err;
}

static inline void stmmac_unregister_platform(void)
{
	platform_driver_unregister(&stmmac_pltfr_driver);
}
#else
static inline int stmmac_register_platform(void)
{
	pr_debug("stmmac: do not register the platf driver\n");

	return 0;
}

static inline void stmmac_unregister_platform(void)
{
}
#endif /* CONFIG_STMMAC_PLATFORM */

#ifdef CONFIG_STMMAC_PCI
extern struct pci_driver stmmac_pci_driver;
static inline int stmmac_register_pci(void)
{
	int err;

	err = pci_register_driver(&stmmac_pci_driver);
	if (err)
		pr_err("stmmac: failed to register the PCI driver\n");

	return err;
}

static inline void stmmac_unregister_pci(void)
{
	pci_unregister_driver(&stmmac_pci_driver);
}
#else
static inline int stmmac_register_pci(void)
{
	pr_debug("stmmac: do not register the PCI driver\n");

	return 0;
}

static inline void stmmac_unregister_pci(void)
{
}
#endif /* CONFIG_STMMAC_PCI */

#endif /* __STMMAC_H__ */
