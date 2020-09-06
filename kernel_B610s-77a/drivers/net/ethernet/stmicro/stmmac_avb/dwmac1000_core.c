/*******************************************************************************
  This is the driver for the GMAC on-chip Ethernet controller for ST SoCs.
  DWC Ether MAC 10/100/1000 Universal version 3.41a  has been used for
  developing this code.

  This only implements the mac core functions for this chip.

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

#include <linux/crc32.h>
#include <linux/slab.h>
#include <linux/ethtool.h>
#include <asm/io.h>
#include "stmmac.h"
#include "dwmac1000.h"

static int dwmac1000_core_init(void __iomem *ioaddr)
{
	unsigned int value;
	int i;	

	for (i = 0; i < MAC_ADDR_NUM; i++) {
		value = readl(ioaddr + GMAC_ADDR_HIGH(i));
		value |= MTL_MULTI_ADDR_HI;
		writel(value, ioaddr + GMAC_ADDR_HIGH(i));

		value = readl(ioaddr + GMAC_ADDR_LOW(i));
		value |= MTL_MULTI_ADDR_LO;
		writel(value, ioaddr + GMAC_ADDR_HIGH(i));		
	}

	/* We don't using mac filter */

	/* MAC Tx queue flow control, we disabled tx flow control. */
	for (i = 0; i < TX_CHN_NR; i++) {
		value = readl(ioaddr + GMAC_TXQ_FCTL(i));

		if (!(value & FLOW_CTL_BUSY)){
			writel(value & (~TX_FLOW_CTL_EN), ioaddr + GMAC_TXQ_FCTL(i));
		} else {
			pr_info("[%s]:Flow control busy!\n", __func__);
			return -EBUSY;
		}
	}

	/* Rx queue enable */
	value = readl(ioaddr + GMAC_RXQ_CTL(0));
	value |= GMAC_RXQ0_EN;
	writel(value, ioaddr + GMAC_RXQ_CTL(0));

	/* Configure Rx Queue 0 to route AV PTP packets */
	value = readl(ioaddr + GMAC_RXQ_CTL(1));
	value |= GMAC_RXQ1_AVPTPQ;
	writel(value, ioaddr + GMAC_RXQ_CTL(1));

	/* Configure Rx priorities */
	value = readl(ioaddr + GMAC_RXQ_CTL(2));
	value |= GMAC_RXQ2_PSQ0;
	writel(value, ioaddr + GMAC_RXQ_CTL(2));
	
	/* MAC interrupt enable */
	writel(MAC_INT_EN, ioaddr + GMAC_INT_EN);

	/* Program MAC configuration, we enable it later */
	value = readl(ioaddr + GMAC_CONTROL);
	value |= GMAC_CORE_INIT;
	writel(value, ioaddr + GMAC_CONTROL);

	return 0;
}

static int dwmac1000_rx_ipc_enable(void __iomem *ioaddr)
{
	u32 value = readl(ioaddr + GMAC_CONTROL);

	value |= GMAC_CONTROL_IPC;
	writel(value, ioaddr + GMAC_CONTROL);

	value = readl(ioaddr + GMAC_CONTROL);

	return !!(value & GMAC_CONTROL_IPC);
}

static void dwmac1000_dump_regs(void __iomem *ioaddr)
{
	int i;
	pr_info("\tDWMAC1000 regs (base addr = 0x%p)\n", ioaddr);

	for (i = 0; i < 55; i++) {
		int offset = i * 4;
		pr_info("\tReg No. %d (offset 0x%x): 0x%08x\n", i,
			offset, readl(ioaddr + offset));
	}
}

static void dwmac1000_set_umac_addr(void __iomem *ioaddr, unsigned char *addr,
				    unsigned int reg_n)
{
	stmmac_set_mac_addr(ioaddr, addr, GMAC_ADDR_HIGH(reg_n),
			    GMAC_ADDR_LOW(reg_n));
}

static void dwmac1000_get_umac_addr(void __iomem *ioaddr, unsigned char *addr,
				    unsigned int reg_n)
{
	stmmac_get_mac_addr(ioaddr, addr, GMAC_ADDR_HIGH(reg_n),
			    GMAC_ADDR_LOW(reg_n));
}

static void dwmac1000_set_filter(struct net_device *dev, int id)
{
	void __iomem *ioaddr = (void __iomem *)dev->base_addr;
	unsigned int value = 0;
	struct stmmac_priv *priv = netdev_priv(dev);
	unsigned int perfect_addr_number;

	CHIP_DBG(KERN_INFO "%s: # mcasts %d, # unicast %d\n",
		 __func__, netdev_mc_count(dev), netdev_uc_count(dev));

	if (dev->flags & IFF_PROMISC)
		value = GMAC_FRAME_FILTER_PR;
	else if ((netdev_mc_count(dev) > HASH_TABLE_SIZE)
		 || (dev->flags & IFF_ALLMULTI)) {
		value = GMAC_FRAME_FILTER_PM;	/* pass all multi */

		/* hash table size is 64 bits */
		writel(0xffffffff, ioaddr + GMAC_HASH_TABLE(0));
		writel(0xffffffff, ioaddr + GMAC_HASH_TABLE(1));
	} else if (!netdev_mc_empty(dev)) {
		u32 mc_filter[2];
		struct netdev_hw_addr *ha;

		/* Hash filter for multicast */
		value = GMAC_FRAME_FILTER_HMC;

		memset(mc_filter, 0, sizeof(mc_filter));
		netdev_for_each_mc_addr(ha, dev) {
			/* The upper 6 bits of the calculated CRC are used to
			 * index the contens of the hash table
			 */
			int bit_nr = bitrev32(~crc32_le(~0, ha->addr, 6)) >> 26;
			/* The most significant bit determines the register to
			 * use (H/L) while the other 5 bits determine the bit
			 * within the register.
			 */
			mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
		}
		
		writel(mc_filter[0], ioaddr + GMAC_HASH_TABLE(0));
		writel(mc_filter[1], ioaddr + GMAC_HASH_TABLE(1));

	}

	perfect_addr_number = priv->dma_cap.multi_addr;

	/* Handle multiple unicast addresses (perfect filtering) */
	if (netdev_uc_count(dev) > perfect_addr_number)
		/* Switch to promiscuous mode if more than 16 addrs
		 * are required
		 */
		value |= GMAC_FRAME_FILTER_PR;
	else {
		int reg = 1;
		struct netdev_hw_addr *ha;

		netdev_for_each_uc_addr(ha, dev) {
			dwmac1000_set_umac_addr(ioaddr, ha->addr, reg);
			reg++;
		}
	}

#ifdef FRAME_FILTER_DEBUG
	/* Enable Receive all mode (to debug filtering_fail errors) */
	value |= GMAC_FRAME_FILTER_RA;
#endif
	writel(value, ioaddr + GMAC_PKT_FILTER);

	CHIP_DBG(KERN_INFO "\tFilter: 0x%08x\n\tHash0:0x%08x, Hash1:0x%08x\n",
		 readl(ioaddr + GMAC_PKT_FILTER),
		 readl(ioaddr + GMAC_HASH_TABLE(0)), readl(ioaddr + GMAC_HASH_TABLE(1)));
}

static void dwmac1000_flow_ctrl(void __iomem *ioaddr, unsigned int duplex,
				unsigned int fc, unsigned int pause_time, int chn)
{
	unsigned int flow = 0;

	CHIP_DBG(KERN_DEBUG "GMAC Flow-Control:\n");
	if (fc & FLOW_RX) {
		CHIP_DBG(KERN_DEBUG "\tReceive Flow-Control ON\n");
		flow |= GMAC_FLOW_CTRL_RFE;
	}
	if (fc & FLOW_TX) {
		CHIP_DBG(KERN_DEBUG "\tTransmit Flow-Control ON\n");
		flow |= GMAC_FLOW_CTRL_TFE;
	}

	if (duplex) {
		CHIP_DBG(KERN_DEBUG "\tduplex mode: PAUSE %d\n", pause_time);
		flow |= (pause_time << GMAC_FLOW_CTRL_PT_SHIFT);
	}

	writel(flow, ioaddr + GMAC_TXQ_FCTL(chn));
}

static void dwmac1000_pmt(void __iomem *ioaddr, unsigned long mode)
{
	unsigned int pmt = 0;

	if (mode & WAKE_MAGIC) {
		CHIP_DBG(KERN_DEBUG "GMAC: WOL Magic frame\n");
		pmt |= power_down | magic_pkt_en;
	}
	if (mode & WAKE_UCAST) {
		CHIP_DBG(KERN_DEBUG "GMAC: WOL on global unicast\n");
		pmt |= global_unicast;
	}

	writel(pmt, ioaddr + GMAC_PMT_CTL_STAT);
}

static int dwmac1000_handle_mac_timestamp_irq(void __iomem *ioaddr, int irq)
{
#ifdef GMAC_STUB
	CHIP_DBG(KERN_DEBUG "Stubed! Add timestamp handler here!:%s\n", __func__);
#endif
	return 0;
}
static int dwmac1000_handle_irq(void __iomem *ioaddr,
				struct stmmac_extra_stats *x)
{
	int rtx_val;
	unsigned int mac_isr;

	mac_isr = readl(ioaddr + GMAC_INT_STAT);	
	rtx_val = readl(ioaddr + GMAC_RX_TX_STAT);
	if (GET_VALUE(mac_isr, MAC_ISR_RXSTSIS_LPOS, MAC_ISR_RXSTSIS_LPOS)) {
		CHIP_DBG(KERN_DEBUG "receive watchdog\n");
		x->rx_watchdog_timeout++;
		rtx_val &= ~GMAC_RTX_RWT;
		mac_isr &= ~GMAC_RXSTSIS;
	}
	
	if (GET_VALUE(mac_isr, MAC_ISR_TXSTSIS_LPOS, MAC_ISR_TXSTSIS_HPOS)) {
		x->tx_status_int++;
		if (rtx_val & GMAC_RTX_TJT) {
			rtx_val &= ~GMAC_RTX_TJT;
			CHIP_DBG(KERN_DEBUG "transmit jabber\n");
			x->tx_jabber_timeout++;
		}		
		mac_isr &= ~GMAC_TXSTSIS;
	}

	if (GET_VALUE(mac_isr, MAC_ISR_TSIS_LPOS, MAC_ISR_TSIS_HPOS)) {
		int mac_timstamp_status = readl(ioaddr + GMAC_TIMESTAMP_STATUS);
		x->timestamp_irq++;

		/* stub it */
		dwmac1000_handle_mac_timestamp_irq(ioaddr, mac_timstamp_status);
	}
	
	if (mac_isr & GMAC_PHYIS) {
		x->phy_int++;
		mac_isr &= ~GMAC_PHYIS;
	}

	if (mac_isr & GMAC_RGSMIIIS) {
		readl(ioaddr + GMAC_PHYIF_CTL_STAT);	//clear this bit
		mac_isr &= ~GMAC_RGSMIIIS;
		x->rgmii_link_changed++;
	}

	if (mac_isr & GMAC_LPIIS) {
		readl(ioaddr + GMAC_LPI_CTL_STAT);
		mac_isr &= ~GMAC_LPIIS;
	}

	if (mac_isr & GMAC_PMTIS) {
		readl(ioaddr + GMAC_PMT_CTL_STAT);
		mac_isr &= ~GMAC_PMTIS;
	}

	if (mac_isr & (GMAC_PCSANCIS | GMAC_PCSLCHGIS)) {
		readl(ioaddr + GMAC_AN_STATUS);
		mac_isr &= ~(GMAC_PCSANCIS | GMAC_PCSLCHGIS);
	}

	return mac_isr;
}

static void dwmac1000_set_eee_mode(void __iomem *ioaddr)
{
	u32 value;

	/* Enable the link status receive on RGMII, SGMII ore SMII
	 * receive path and instruct the transmit to enter in LPI
	 * state.
	 */
	value = readl(ioaddr + GMAC_LPI_CTL_STAT);
	value |= LPI_CTRL_STATUS_LPIEN | LPI_CTRL_STATUS_LPITXA;
	writel(value, ioaddr + GMAC_LPI_CTL_STAT);
}

static void dwmac1000_reset_eee_mode(void __iomem *ioaddr)
{
	u32 value;

	value = readl(ioaddr + GMAC_LPI_CTL_STAT);
	value &= ~(LPI_CTRL_STATUS_LPIEN | LPI_CTRL_STATUS_LPITXA);
	writel(value, ioaddr + GMAC_LPI_CTL_STAT);
}

static void dwmac1000_set_eee_pls(void __iomem *ioaddr, int link)
{
	u32 value;

	value = readl(ioaddr + GMAC_LPI_CTL_STAT);

	if (link)
		value |= LPI_CTRL_STATUS_PLS;
	else
		value &= ~LPI_CTRL_STATUS_PLS;

	writel(value, ioaddr + GMAC_LPI_CTL_STAT);
}

static void dwmac1000_set_eee_timer(void __iomem *ioaddr, int ls, int tw)
{
	int value = ((tw & 0xffff)) | ((ls & 0x7ff) << 16);

	/* Program the timers in the LPI timer control register:
	 * LS: minimum time (ms) for which the link
	 *  status from PHY should be ok before transmitting
	 *  the LPI pattern.
	 * TW: minimum time (us) for which the core waits
	 *  after it has stopped transmitting the LPI pattern.
	 */
	writel(value, ioaddr + GMAC_LPI_TIMERS_CTL);
}

static void dwmac1000_ctrl_ane(void __iomem *ioaddr, bool restart)
{
	u32 value;

	value = readl(ioaddr + GMAC_AN_CTRL);
	/* auto negotiation enable and External Loopback enable */
	value = GMAC_AN_CTRL_ANE | GMAC_AN_CTRL_ELE;

	if (restart)
		value |= GMAC_AN_CTRL_RAN;

	writel(value, ioaddr + GMAC_AN_CTRL);
}

static void dwmac1000_get_adv(void __iomem *ioaddr, struct rgmii_adv *adv)
{
	u32 value = readl(ioaddr + GMAC_ANE_ADV);

	if (value & GMAC_ANE_FD)
		adv->duplex = DUPLEX_FULL;
	if (value & GMAC_ANE_HD)
		adv->duplex |= DUPLEX_HALF;

	adv->pause = (value & GMAC_ANE_PSE) >> GMAC_ANE_PSE_SHIFT;

	value = readl(ioaddr + GMAC_ANE_LPA);

	if (value & GMAC_ANE_FD)
		adv->lp_duplex = DUPLEX_FULL;
	if (value & GMAC_ANE_HD)
		adv->lp_duplex = DUPLEX_HALF;

	adv->lp_pause = (value & GMAC_ANE_PSE) >> GMAC_ANE_PSE_SHIFT;
}

static int dwmac1000_get_speed(void __iomem *ioaddr)
{
	unsigned int value;
	int line_speed = -1;

	value = readl(ioaddr + GMAC_CONTROL);
	line_speed = (value >> GMAC_CTL_FES_SHIFT) & GMAC_LINE_SPEED_MASK;

	return line_speed;
}

static int dwmac1000_get_mac_ipg(void __iomem *ioaddr)
{
	unsigned int value;
	int ipg;
	
	value = readl(ioaddr + GMAC_CONTROL);
	value =  (value & GMAC_CONTROL_IPG) >> GMAC_CONTROL_IPG_OFFSET ;

	ipg = 12 - value ;

	return ipg;
}

static int dwmac1000_get_mac_preamble(void __iomem *ioaddr)
{
	unsigned int value;
	int preamble;
	
	value = readl(ioaddr + GMAC_CONTROL);
	value =  (value & GMAC_CONTROL_PRELEN) >> GMAC_CONTROL_PRELEN_OFFSET ;

	preamble = 7 - value * 2 ;

	return preamble;
}

static const struct stmmac_ops dwmac1000_ops = {
	.core_init = dwmac1000_core_init,
	.rx_ipc = dwmac1000_rx_ipc_enable,
	.dump_regs = dwmac1000_dump_regs,
	.handle_irq = dwmac1000_handle_irq,
	.set_filter = dwmac1000_set_filter,
	.flow_ctrl = dwmac1000_flow_ctrl,
	.pmt = dwmac1000_pmt,
	.set_umac_addr = dwmac1000_set_umac_addr,
	.get_umac_addr = dwmac1000_get_umac_addr,
	.set_eee_mode = dwmac1000_set_eee_mode,
	.reset_eee_mode = dwmac1000_reset_eee_mode,
	.set_eee_timer = dwmac1000_set_eee_timer,
	.set_eee_pls = dwmac1000_set_eee_pls,
	.ctrl_ane = dwmac1000_ctrl_ane,
	.get_adv = dwmac1000_get_adv,
	.get_line_spped = dwmac1000_get_speed,
	.get_mac_ipg = dwmac1000_get_mac_ipg,
	.get_mac_preamble = dwmac1000_get_mac_preamble,
};

struct mac_device_info *dwmac1000_setup(void __iomem *ioaddr)
{
	struct mac_device_info *mac;
	u32 hwid = readl(ioaddr + GMAC_VERSION);

	mac = kzalloc(sizeof(const struct mac_device_info), GFP_KERNEL);
	if (!mac)
		return NULL;

	mac->mac = &dwmac1000_ops;
	mac->dma = &dwmac1000_dma_ops;
	mac->mtl = &dwmac1000_mtl_ops;
	mac->desc = &ndesc_ops;
	mac->ring = &ring_mode_ops;
	mac->cbs = &stmmac_cbs;

	/*port seclect, 0 for 1000Mbps, 1 for 10/100Mbps.
	 * In adjust_link,  if using GMAC, it will be reverse to write into 
	 * GMAC_CONTROL register.
	 */
	mac->link.port = GMAC_CONTROL_PS;	
	mac->link.duplex = GMAC_CONTROL_DM;	// select duplex mode.

	/* selects the speed in the 10/100 Mbps mode:
	* 0: 10 Mbps
	* 1: 100 Mbps
	*/
	mac->link.speed = GMAC_CONTROL_FES;	
	mac->mii.addr = GMAC_MDIO_ADDR;
	mac->mii.data = GMAC_MDIO_DATA;
	mac->synopsys_uid = hwid;

	return mac;
}
