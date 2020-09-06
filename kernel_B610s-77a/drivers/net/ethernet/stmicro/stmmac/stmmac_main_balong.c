 /*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/if_ether.h>
#include <linux/crc32.h>
#include <linux/mii.h>
#include <linux/if.h>
#include <linux/if_vlan.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/prefetch.h>
#ifdef CONFIG_STMMAC_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif /* CONFIG_STMMAC_DEBUG_FS */
#include <linux/net_tstamp.h>
#include "stmmac_ptp.h"
#include "stmmac.h"
#include "stmmac_debug.h"
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
#include <linux/spe/spe_interface.h>
#endif

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
static inline void stmmac_rx_refill(struct stmmac_priv *priv);
static int stmmac_rx(struct stmmac_priv *priv, int limit);

static int
stmmac_finish_rd(int portno, int src_portno, struct sk_buff *skb, dma_addr_t dma, unsigned int flags);
static int
stmmac_finish_td(int portno, struct sk_buff *skb, unsigned int flags);
static netdev_tx_t stmmac_spe_xmit(struct sk_buff *skb, struct net_device *dev);
#endif

#if defined(CONFIG_STMMAC_IPF_SIM)
#define RNIC_ETHER_ADDR_LEN             (6)
#endif

#undef STMMAC_DEBUG
/*#define STMMAC_DEBUG*/
#ifdef STMMAC_DEBUG
#define DBG(nlevel, klevel, fmt, args...) \
		((void)(netif_msg_##nlevel(priv) && \
		printk(KERN_##klevel fmt, ## args)))
#else
#define DBG(nlevel, klevel, fmt, args...) do { } while (0)
#endif

#undef STMMAC_RX_DEBUG
/*#define STMMAC_RX_DEBUG*/
#ifdef STMMAC_RX_DEBUG
#define RX_DBG(fmt, args...)  printk(fmt, ## args)
#else
#define RX_DBG(fmt, args...)  do { } while (0)
#endif

#undef STMMAC_XMIT_DEBUG
/*#define STMMAC_XMIT_DEBUG*/
#ifdef STMMAC_XMIT_DEBUG
#define TX_DBG(fmt, args...)  printk(fmt, ## args)
#else
#define TX_DBG(fmt, args...)  do { } while (0)
#endif
#if (FEATURE_ON == MBB_FEATURE_ETH)
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
#define   NET_DEVICE_NAME   "et0"
int g_mmitest_enable = 0; 
#define LAN_WAN_PORT_NAME   "eth0"
#define LAN_WAN_VLAN_TAG    5
#else
#define   NET_DEVICE_NAME   "eth0"
#endif

extern int mbb_check_net_upgrade(struct sk_buff *skb);
extern int mbb_mac_clone_rx_restore(struct sk_buff *skb);
extern int mbb_mac_clone_tx_save(struct sk_buff *skb);
extern int mbb_get_eth_macAddr(char *eth_macAddr);
#if (FEATURE_ON == MBB_CTF_COMMON)
extern void mbb_ctf_init(struct net_device *net);
extern int mbb_ctf_forward(struct sk_buff *skb,  struct net_device *dev);
extern void mbb_ctf_exit(struct net_device *net);
#endif
#if ((FEATURE_ON == MBB_FEATURE_FASTIP) && (FEATURE_ON == MBB_ETH_STMMAC_TX_THREAD))
extern void mbb_fastip_init(struct net_device *net);
extern int mbb_fastip_forward(struct sk_buff *skb,  struct net_device *dev);
extern void mbb_fastip_exit(struct net_device *net);
#endif
#if (FEATURE_ON == MBB_ETH_STMMAC_TX_THREAD)
static int stmmac_vendor_tx_thread_init(struct stmmac_priv *stmmac);
static int stmmac_vendor_tx_thread_uninit(struct stmmac_priv *stmmac);
int stmmac_vendor_post_skb(struct sk_buff *skb,   struct net_device *dev);
#endif
#endif
#define STMMAC_ALIGN(x)	L1_CACHE_ALIGN(x)
#define JUMBO_LEN	9000

struct stmmac_extra_stats *gmac_status;

/* Module parameters */
#if (FEATURE_ON == MBB_ETH_STMMAC_TX_THREAD)
#define TX_TIMEO 500000 
#else
#define TX_TIMEO 5000 /* default 5 seconds */
#endif
static int watchdog = TX_TIMEO;
module_param(watchdog, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(watchdog, "Transmit timeout in milliseconds (default 5s)");

static int debug = -1;
module_param(debug, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Message Level (-1: default, 0: no output, 16: all)");
#if (FEATURE_ON == MBB_FEATURE_ETH)
/* phy芯片 8367，Pin82若上拉, 则表示PHYID=29。Pin82若下拉, 则表示PHYID=0 */
/* B612 使用的 8367，硬件设计 Pin82 下拉，所以这里要把 phyid改成 0 */
#if defined(BSP_CONFIG_BOARD_CPE_B612) || defined(BSP_CONFIG_BOARD_CPE_B715)
int phyaddr = 0;
#else
int phyaddr = 29;
#endif
#else
int phyaddr = 1;
#endif
module_param(phyaddr, int, S_IRUGO);
MODULE_PARM_DESC(phyaddr, "Physical device address");

#define DMA_TX_SIZE 256
static int dma_txsize = DMA_TX_SIZE;
module_param(dma_txsize, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dma_txsize, "Number of descriptors in the TX list");

#define DMA_RX_SIZE 256
static int dma_rxsize = DMA_RX_SIZE;
module_param(dma_rxsize, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dma_rxsize, "Number of descriptors in the RX list");

static int flow_ctrl = FLOW_OFF;
module_param(flow_ctrl, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(flow_ctrl, "Flow control ability [on/off]");

static int pause = PAUSE_TIME;
module_param(pause, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(pause, "Flow Control Pause Time");

#define TC_DEFAULT 64
static int tc = TC_DEFAULT;
module_param(tc, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(tc, "DMA threshold control value");
#if (FEATURE_ON == MBB_FEATURE_ETH)
#define DMA_BUFFER_SIZE    BUF_SIZE_4KiB
#define MTU_AND_HEAD    1518
#else
#define DMA_BUFFER_SIZE	BUF_SIZE_2KiB
#endif

static int buf_sz = DMA_BUFFER_SIZE;
module_param(buf_sz, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(buf_sz, "DMA buffer size");

static const u32 default_msg_level = (NETIF_MSG_DRV | NETIF_MSG_PROBE |
				      NETIF_MSG_LINK | NETIF_MSG_IFUP |
				      NETIF_MSG_IFDOWN | NETIF_MSG_TIMER);

#define STMMAC_DEFAULT_LPI_TIMER	1000
static int eee_timer = STMMAC_DEFAULT_LPI_TIMER;
module_param(eee_timer, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(eee_timer, "LPI tx expiration time in msec");
#define STMMAC_LPI_T(x) (jiffies + msecs_to_jiffies(x))

/* By default the driver will use the ring mode to manage tx and rx descriptors
 * but passing this value so user can force to use the chain instead of the ring
 */
static unsigned int chain_mode;
module_param(chain_mode, int, S_IRUGO);
MODULE_PARM_DESC(chain_mode, "To use chain instead of ring mode");

static irqreturn_t stmmac_interrupt(int irq, void *dev_id);

#ifdef CONFIG_STMMAC_DEBUG_FS
static int stmmac_init_fs(struct net_device *dev);
static void stmmac_exit_fs(void);
#endif

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))   
static netdev_tx_t stmmac_xmit(struct sk_buff *skb, struct net_device *dev);
#endif

#define STMMAC_COAL_TIMER(x) (jiffies + usecs_to_jiffies(x))
#if (FEATURE_ON == MBB_FEATURE_ETH)
unsigned int gmac_msg_level = GMAC_LEVEL_ERR;
EXPORT_SYMBOL(gmac_msg_level);
#else
#ifdef CONFIG_GMAC_DEBUG
unsigned int gmac_msg_level = GMAC_LEVEL_ERR;
#endif
#endif/*MBB_FEATURE_ETH*/
#if (FEATURE_ON == MBB_FEATURE_ETH)
int riwt_value = 1;
#else
int riwt_value = MIN_DMA_RIWT;
#endif
int rx_irq_flag = 1;


void set_riwt_value(unsigned int cnt)
{
	riwt_value = cnt;
}

void set_rx_irq_flag(unsigned int cnt)
{
	rx_irq_flag = cnt;
}
void set_gmac_msg(unsigned int level)
{
	gmac_msg_level = level;
}
EXPORT_SYMBOL(set_gmac_msg);

unsigned int get_gmac_msg(void)
{
	printk("level : %x\n",gmac_msg_level);
    return gmac_msg_level;
}
EXPORT_SYMBOL(get_gmac_msg);
#if (FEATURE_ON == MBB_ETH_STMMAC_TX_THREAD)
uint stmmac_pkt_txthread_flag = true;
uint stmmac_get_txthread_flag(void)
{
    printk("stmmac_pkt_txthread_flag:%d\n", stmmac_pkt_txthread_flag);
    return 0;
}
uint stmmac_set_txthread_flag(uint flag)
{
    stmmac_pkt_txthread_flag = flag;
    return 0;
}
#endif

/**
 * stmmac_verify_args - verify the driver parameters.
 * Description: it verifies if some wrong parameter is passed to the driver.
 * Note that wrong parameters are replaced with the default values.
 */
static void stmmac_verify_args(void)
{
	if (unlikely(watchdog < 0))
		watchdog = TX_TIMEO;
	if (unlikely(dma_rxsize < 0))
		dma_rxsize = DMA_RX_SIZE;
	if (unlikely(dma_txsize < 0))
		dma_txsize = DMA_TX_SIZE;
	if (unlikely((buf_sz < DMA_BUFFER_SIZE) || (buf_sz > BUF_SIZE_16KiB)))
		buf_sz = DMA_BUFFER_SIZE;
	if (unlikely(flow_ctrl > 1))
		flow_ctrl = FLOW_AUTO;
	else if (likely(flow_ctrl < 0))
		flow_ctrl = FLOW_OFF;
	if (unlikely((pause < 0) || (pause > 0xffff)))
		pause = PAUSE_TIME;
	if (eee_timer < 0)
		eee_timer = STMMAC_DEFAULT_LPI_TIMER;
}

/**
 * stmmac_clk_csr_set - dynamically set the MDC clock
 * @priv: driver private structure
 * Description: this is to dynamically set the MDC clock according to the csr
 * clock input.
 * Note:
 *	If a specific clk_csr value is passed from the platform
 *	this means that the CSR Clock Range selection cannot be
 *	changed at run-time and it is fixed (as reported in the driver
 *	documentation). Viceversa the driver will try to set the MDC
 *	clock dynamically according to the actual clock input.
 */


#if defined(STMMAC_XMIT_DEBUG) || defined(STMMAC_RX_DEBUG)
static void print_pkt(unsigned char *buf, int len)
{
	int j;
	pr_info("len = %d byte, buf addr: 0x%p", len, buf);
	for (j = 0; j < len; j++) {
		if ((j % 16) == 0)
			pr_info("\n %03x:", j);
		pr_info(" %02x", buf[j]);
	}
	pr_info("\n");
}
#endif

/* minimum number of free TX descriptors required to wake up TX process */
#define STMMAC_TX_THRESH(x)	(x->dma_tx_size/4)

static inline u32 stmmac_tx_avail(struct stmmac_priv *priv)
{
	return priv->dirty_tx + priv->dma_tx_size - priv->cur_tx - 1;
}

/**
 * stmmac_hw_fix_mac_speed: callback for speed selection
 * @priv: driver private structure
 * Description: on some platforms (e.g. ST), some HW system configuraton
 * registers have to be set according to the link speed negotiated.
 */
static inline void stmmac_hw_fix_mac_speed(struct stmmac_priv *priv)
{
	struct phy_device *phydev = priv->phydev;

	if (likely(priv->plat->fix_mac_speed))
		priv->plat->fix_mac_speed(priv->plat->bsp_priv, phydev->speed);
}

/**
 * stmmac_enable_eee_mode: Check and enter in LPI mode
 * @priv: driver private structure
 * Description: this function is to verify and enter in LPI mode for EEE.
 */
static void stmmac_enable_eee_mode(struct stmmac_priv *priv)
{
	/* Check and enter in LPI mode */
	if ((priv->dirty_tx == priv->cur_tx) &&
	    (priv->tx_path_in_lpi_mode == false))
		priv->hw->mac->set_eee_mode(priv->ioaddr);
}

/**
 * stmmac_disable_eee_mode: disable/exit from EEE
 * @priv: driver private structure
 * Description: this function is to exit and disable EEE in case of
 * LPI state is true. This is called by the xmit.
 */
void stmmac_disable_eee_mode(struct stmmac_priv *priv)
{
	priv->hw->mac->reset_eee_mode(priv->ioaddr);
	del_timer_sync(&priv->eee_ctrl_timer);
	priv->tx_path_in_lpi_mode = false;
}

/**
 * stmmac_eee_ctrl_timer: EEE TX SW timer.
 * @arg : data hook
 * Description:
 *  if there is no data transfer and if we are not in LPI state,
 *  then MAC Transmitter can be moved to LPI state.
 */
static void stmmac_eee_ctrl_timer(unsigned long arg)
{
	struct stmmac_priv *priv = (struct stmmac_priv *)arg;

	stmmac_enable_eee_mode(priv);
	mod_timer(&priv->eee_ctrl_timer, STMMAC_LPI_T(eee_timer));
}

/**
 * stmmac_eee_init: init EEE
 * @priv: driver private structure
 * Description:
 *  If the EEE support has been enabled while configuring the driver,
 *  if the GMAC actually supports the EEE (from the HW cap reg) and the
 *  phy can also manage EEE, so enable the LPI state and start the timer
 *  to verify if the tx path can enter in LPI state.
 */
bool stmmac_eee_init(struct stmmac_priv *priv)
{
	bool ret = false;

	/* Using PCS we cannot dial with the phy registers at this stage
	 * so we do not support extra feature like EEE.
	 */
	if ((priv->pcs == STMMAC_PCS_RGMII) || (priv->pcs == STMMAC_PCS_TBI) ||
	    (priv->pcs == STMMAC_PCS_RTBI))
		goto out;

	/* MAC core supports the EEE feature. */
	if (priv->dma_cap.eee) {
		if(!priv->phydev){
			pr_err("%s: phydev null\n",__func__);
			goto out;
		}
		/* Check if the PHY supports EEE */
		if (phy_init_eee(priv->phydev, 1))
			goto out;

		if (!priv->eee_active) {
			priv->eee_active = 1;
			init_timer(&priv->eee_ctrl_timer);
			priv->eee_ctrl_timer.function = stmmac_eee_ctrl_timer;
			priv->eee_ctrl_timer.data = (unsigned long)priv;
			priv->eee_ctrl_timer.expires = STMMAC_LPI_T(eee_timer);
			add_timer(&priv->eee_ctrl_timer);

			priv->hw->mac->set_eee_timer(priv->ioaddr,
						     STMMAC_DEFAULT_LIT_LS,
						     priv->tx_lpi_timer);
		} else
			/* Set HW EEE according to the speed */
			priv->hw->mac->set_eee_pls(priv->ioaddr,
						   priv->phydev->link);

		pr_info("stmmac: Energy-Efficient Ethernet initialized\n");

		ret = true;
	}
out:
	return ret;
}

/* stmmac_get_tx_hwtstamp: get HW TX timestamps
 * @priv: driver private structure
 * @entry : descriptor index to be used.
 * @skb : the socket buffer
 * Description :
 * This function will read timestamp from the descriptor & pass it to stack.
 * and also perform some sanity checks.
 */
static void stmmac_get_tx_hwtstamp(struct stmmac_priv *priv,
				   unsigned int entry, struct sk_buff *skb)
{
	struct skb_shared_hwtstamps shhwtstamp;
	u64 ns;
	void *desc = NULL;

	if (!priv->hwts_tx_en)
		return;

	/* exit if skb doesn't support hw tstamp */
	if (likely(!(skb_shinfo(skb)->tx_flags & SKBTX_IN_PROGRESS)))
		return;

	if (priv->adv_ts)
		desc = (priv->dma_etx + entry);
	else
		desc = (priv->dma_tx + entry);

	/* check tx tstamp status */
	if (!priv->hw->desc->get_tx_timestamp_status((struct dma_desc *)desc))
		return;

	/* get the valid tstamp */
	ns = priv->hw->desc->get_timestamp(desc, priv->adv_ts);

	memset(&shhwtstamp, 0, sizeof(struct skb_shared_hwtstamps));
	shhwtstamp.hwtstamp = ns_to_ktime(ns);
	/* pass tstamp to stack */
	skb_tstamp_tx(skb, &shhwtstamp);

	return;
}

/* stmmac_get_rx_hwtstamp: get HW RX timestamps
 * @priv: driver private structure
 * @entry : descriptor index to be used.
 * @skb : the socket buffer
 * Description :
 * This function will read received packet's timestamp from the descriptor
 * and pass it to stack. It also perform some sanity checks.
 */
static void stmmac_get_rx_hwtstamp(struct stmmac_priv *priv,
				   unsigned int entry, struct sk_buff *skb)
{
	struct skb_shared_hwtstamps *shhwtstamp = NULL;
	u64 ns;
	void *desc = NULL;

	if (!priv->hwts_rx_en)
		return;

	if (priv->adv_ts)
		desc = (priv->dma_erx + entry);
	else
		desc = (priv->dma_rx + entry);

	/* exit if rx tstamp is not valid */
	if (!priv->hw->desc->get_rx_timestamp_status(desc, priv->adv_ts))
		return;

	/* get valid tstamp */
	ns = priv->hw->desc->get_timestamp(desc, priv->adv_ts);
	shhwtstamp = skb_hwtstamps(skb);
	memset(shhwtstamp, 0, sizeof(struct skb_shared_hwtstamps));
	shhwtstamp->hwtstamp = ns_to_ktime(ns);
}

/**
 *  stmmac_hwtstamp_ioctl - control hardware timestamping.
 *  @dev: device pointer.
 *  @ifr: An IOCTL specefic structure, that can contain a pointer to
 *  a proprietary structure used to pass information to the driver.
 *  Description:
 *  This function configures the MAC to enable/disable both outgoing(TX)
 *  and incoming(RX) packets time stamping based on user input.
 *  Return Value:
 *  0 on success and an appropriate -ve integer on failure.
 */
static int stmmac_hwtstamp_ioctl(struct net_device *dev, struct ifreq *ifr)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	struct hwtstamp_config config;
	struct timespec now;
	u64 temp = 0;
	u32 ptp_v2 = 0;
	u32 tstamp_all = 0;
	u32 ptp_over_ipv4_udp = 0;
	u32 ptp_over_ipv6_udp = 0;
	u32 ptp_over_ethernet = 0;
	u32 snap_type_sel = 0;
	u32 ts_master_en = 0;
	u32 ts_event_en = 0;
	u32 value = 0;

	if (!(priv->dma_cap.time_stamp || priv->adv_ts)) {
		netdev_alert(priv->dev, "No support for HW time stamping\n");
		priv->hwts_tx_en = 0;
		priv->hwts_rx_en = 0;

		return -EOPNOTSUPP;
	}

	if (copy_from_user(&config, ifr->ifr_data,
			   sizeof(struct hwtstamp_config)))
		return -EFAULT;

	GMAC_DBG(("%s config flags:0x%x, tx_type:0x%x, rx_filter:0x%x\n",
		 __func__, config.flags, config.tx_type, config.rx_filter));

	/* reserved for future extensions */
	if (config.flags)
		return -EINVAL;

	switch (config.tx_type) {
	case HWTSTAMP_TX_OFF:
		priv->hwts_tx_en = 0;
		break;
	case HWTSTAMP_TX_ON:
		priv->hwts_tx_en = 1;
		break;
	default:
		return -ERANGE;
	}

	if (priv->adv_ts) {
		switch (config.rx_filter) {
		case HWTSTAMP_FILTER_NONE:
			/* time stamp no incoming packet at all */
			config.rx_filter = HWTSTAMP_FILTER_NONE;
			break;

		case HWTSTAMP_FILTER_PTP_V1_L4_EVENT:
			/* PTP v1, UDP, any kind of event packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_EVENT;
			/* take time stamp for all event messages */
			snap_type_sel = PTP_TCR_SNAPTYPSEL_1;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V1_L4_SYNC:
			/* PTP v1, UDP, Sync packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_SYNC;
			/* take time stamp for SYNC messages only */
			ts_event_en = PTP_TCR_TSEVNTENA;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ:
			/* PTP v1, UDP, Delay_req packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ;
			/* take time stamp for Delay_Req messages only */
			ts_master_en = PTP_TCR_TSMSTRENA;
			ts_event_en = PTP_TCR_TSEVNTENA;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_L4_EVENT:
			/* PTP v2, UDP, any kind of event packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_L4_EVENT;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for all event messages */
			snap_type_sel = PTP_TCR_SNAPTYPSEL_1;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_L4_SYNC:
			/* PTP v2, UDP, Sync packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_L4_SYNC;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for SYNC messages only */
			ts_event_en = PTP_TCR_TSEVNTENA;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ:
			/* PTP v2, UDP, Delay_req packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for Delay_Req messages only */
			ts_master_en = PTP_TCR_TSMSTRENA;
			ts_event_en = PTP_TCR_TSEVNTENA;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_EVENT:
			/* PTP v2/802.AS1 any layer, any kind of event packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_EVENT;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for all event messages */
			snap_type_sel = PTP_TCR_SNAPTYPSEL_1;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			ptp_over_ethernet = PTP_TCR_TSIPENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_SYNC:
			/* PTP v2/802.AS1, any layer, Sync packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_SYNC;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for SYNC messages only */
			ts_event_en = PTP_TCR_TSEVNTENA;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			ptp_over_ethernet = PTP_TCR_TSIPENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_DELAY_REQ:
			/* PTP v2/802.AS1, any layer, Delay_req packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_DELAY_REQ;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for Delay_Req messages only */
			ts_master_en = PTP_TCR_TSMSTRENA;
			ts_event_en = PTP_TCR_TSEVNTENA;

			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			ptp_over_ethernet = PTP_TCR_TSIPENA;
			break;

		case HWTSTAMP_FILTER_ALL:
			/* time stamp any incoming packet */
			config.rx_filter = HWTSTAMP_FILTER_ALL;
			tstamp_all = PTP_TCR_TSENALL;
			break;

		default:
			return -ERANGE;
		}
	} else {
		switch (config.rx_filter) {
		case HWTSTAMP_FILTER_NONE:
			config.rx_filter = HWTSTAMP_FILTER_NONE;
			break;
		default:
			/* PTP v1, UDP, any kind of event packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_EVENT;
			break;
		}
	}
	priv->hwts_rx_en = ((config.rx_filter == HWTSTAMP_FILTER_NONE) ? 0 : 1);

	if (!priv->hwts_tx_en && !priv->hwts_rx_en)
		priv->hw->ptp->config_hw_tstamping(priv->ioaddr, 0);
	else {
		value = (PTP_TCR_TSENA | PTP_TCR_TSCFUPDT | PTP_TCR_TSCTRLSSR |
			 tstamp_all | ptp_v2 | ptp_over_ethernet |
			 ptp_over_ipv6_udp | ptp_over_ipv4_udp | ts_event_en |
			 ts_master_en | snap_type_sel);

		priv->hw->ptp->config_hw_tstamping(priv->ioaddr, value);

		/* program Sub Second Increment reg */
		priv->hw->ptp->config_sub_second_increment(priv->ioaddr);

		/* calculate default added value:
		 * formula is :
		 * addend = (2^32)/freq_div_ratio;
		 * where, freq_div_ratio = STMMAC_SYSCLOCK/50MHz
		 * hence, addend = ((2^32) * 50MHz)/STMMAC_SYSCLOCK;
		 * NOTE: STMMAC_SYSCLOCK should be >= 50MHz to
		 *       achive 20ns accuracy.
		 *
		 * 2^x * y == (y << x), hence
		 * 2^32 * 50000000 ==> (50000000 << 32)
		 */
		temp = (u64) (50000000ULL << 32);
		priv->default_addend = div_u64(temp, STMMAC_SYSCLOCK);
		priv->hw->ptp->config_addend(priv->ioaddr,
					     priv->default_addend);

		/* initialize system time */
		getnstimeofday(&now);
		priv->hw->ptp->init_systime(priv->ioaddr, now.tv_sec,
					    now.tv_nsec);
	}

	return copy_to_user(ifr->ifr_data, &config,
			    sizeof(struct hwtstamp_config)) ? -EFAULT : 0;
}

/**
 * stmmac_init_ptp: init PTP
 * @priv: driver private structure
 * Description: this is to verify if the HW supports the PTPv1 or v2.
 * This is done by looking at the HW cap. register.
 * Also it registers the ptp driver.
 */
static int stmmac_init_ptp(struct stmmac_priv *priv)
{
	if (!(priv->dma_cap.time_stamp || priv->dma_cap.atime_stamp))
		return -EOPNOTSUPP;

	if (netif_msg_hw(priv)) {
		if (priv->dma_cap.time_stamp) {
			GMAC_DBG(("IEEE 1588-2002 Time Stamp supported\n"));
			priv->adv_ts = 0;
		}
		if (priv->dma_cap.atime_stamp && priv->extend_desc) {
			GMAC_DBG(
			    ("IEEE 1588-2008 Advanced Time Stamp supported\n"));
			priv->adv_ts = 1;
		}
	}

	priv->hw->ptp = &stmmac_ptp;
	priv->hwts_tx_en = 0;
	priv->hwts_rx_en = 0;

	return stmmac_ptp_register(priv);
}

static void stmmac_release_ptp(struct stmmac_priv *priv)
{
	stmmac_ptp_unregister(priv);
}

/**
 * stmmac_adjust_link
 * @dev: net device structure
 * Description: it adjusts the link parameters.
 */
static void stmmac_adjust_link(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	struct phy_device *phydev = priv->phydev;
	unsigned long flags;
	int new_state = 0;
	unsigned int fc = priv->flow_ctrl, pause_time = priv->pause;

	if (phydev == NULL)
		return;

	spin_lock_irqsave(&priv->lock, flags);

	if (phydev->link) {
		u32 ctrl = readl(priv->ioaddr + MAC_CTRL_REG);

		/* Now we make sure that we can be in full duplex mode.
		 * If not, we operate in half-duplex mode. */
		if (phydev->duplex != priv->oldduplex) {
			new_state = 1;
			if (!(phydev->duplex))
				ctrl &= ~priv->hw->link.duplex;
			else
				ctrl |= priv->hw->link.duplex;
			priv->oldduplex = phydev->duplex;
		}
		/* Flow Control operation */
		if (phydev->pause)
			priv->hw->mac->flow_ctrl(priv->ioaddr, phydev->duplex,
						 fc, pause_time);

		if (phydev->speed != priv->speed) {
			new_state = 1;
			switch (phydev->speed) {
			case 1000:
				if (likely(priv->plat->has_gmac))
					ctrl &= ~priv->hw->link.port;
				stmmac_hw_fix_mac_speed(priv);
				break;
			case 100:
			case 10:
				if (priv->plat->has_gmac) {
					ctrl |= priv->hw->link.port;
					if (phydev->speed == SPEED_100) {
						ctrl |= priv->hw->link.speed;
					} else {
						ctrl &= ~(priv->hw->link.speed);
					}
				} else {
					ctrl &= ~priv->hw->link.port;
				}
				stmmac_hw_fix_mac_speed(priv);
				break;
			default:
				if (netif_msg_link(priv))
					GMAC_WARNING(("%s: Speed (%d) not 10/100\n",
						dev->name, phydev->speed));
				break;
			}

			priv->speed = phydev->speed;
		}

		writel(ctrl, priv->ioaddr + MAC_CTRL_REG);

		if (!priv->oldlink) {
			new_state = 1;
			priv->oldlink = 1;
		}
	} else if (priv->oldlink) {
		new_state = 1;
		priv->oldlink = 0;
		priv->speed = 0;
		priv->oldduplex = -1;
	}

	if (new_state && netif_msg_link(priv))
		phy_print_status(phydev);

	/* At this stage, it could be needed to setup the EEE or adjust some
	 * MAC related HW registers.
	 */
	priv->eee_enabled = stmmac_eee_init(priv);

	spin_unlock_irqrestore(&priv->lock, flags);

	//GMAC_DBG(( "stmmac_adjust_link: exiting\n"));
}

/**
 * stmmac_check_pcs_mode: verify if RGMII/SGMII is supported
 * @priv: driver private structure
 * Description: this is to verify if the HW supports the PCS.
 * Physical Coding Sublayer (PCS) interface that can be used when the MAC is
 * configured for the TBI, RTBI, or SGMII PHY interface.
 */
static void stmmac_check_pcs_mode(struct stmmac_priv *priv)
{
	int interface = priv->plat->interface;

	if (priv->dma_cap.pcs) {
		if ((interface & PHY_INTERFACE_MODE_RGMII) ||
		    (interface & PHY_INTERFACE_MODE_RGMII_ID) ||
		    (interface & PHY_INTERFACE_MODE_RGMII_RXID) ||
		    (interface & PHY_INTERFACE_MODE_RGMII_TXID)) {
			GMAC_DBG(("STMMAC: PCS RGMII support enable\n"));
			priv->pcs = STMMAC_PCS_RGMII;
		} else if (interface & PHY_INTERFACE_MODE_SGMII) {
			GMAC_DBG(("STMMAC: PCS SGMII support enable\n"));
			priv->pcs = STMMAC_PCS_SGMII;
		}
	}
}

/**
 * stmmac_init_phy - PHY initialization
 * @dev: net device structure
 * Description: it initializes the driver's PHY state, and attaches the PHY
 * to the mac driver.
 *  Return value:
 *  0 on success
 */
static int stmmac_init_phy(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	struct phy_device *phydev;
	char phy_id_fmt[MII_BUS_ID_SIZE + 3];
	char bus_id[MII_BUS_ID_SIZE];
	int interface = priv->plat->interface;
	priv->oldlink = 0;
	priv->speed = 0;
	priv->oldduplex = -1;

	if (priv->plat->phy_bus_name)
		snprintf(bus_id, MII_BUS_ID_SIZE, "%s-%x",
			 priv->plat->phy_bus_name, priv->plat->bus_id);
	else
		snprintf(bus_id, MII_BUS_ID_SIZE, "stmmac-%x",
			 priv->plat->bus_id);

	snprintf(phy_id_fmt, MII_BUS_ID_SIZE + 3, PHY_ID_FMT, bus_id,
		 priv->plat->phy_addr);
	GMAC_DBG(("stmmac_init_phy:  trying to attach to %s\n", phy_id_fmt));

	phydev = phy_connect(dev, phy_id_fmt, &stmmac_adjust_link, interface);

	if (IS_ERR(phydev)) {
		GMAC_ERR(("%s: Could not attach to PHY\n", dev->name));
		return PTR_ERR(phydev);
	}

	/* Stop Advertising 1000BASE Capability if interface is not GMII */
	if ((interface == PHY_INTERFACE_MODE_MII) ||
	    (interface == PHY_INTERFACE_MODE_RMII))
		phydev->advertising &= ~(SUPPORTED_1000baseT_Half |
					 SUPPORTED_1000baseT_Full);

	/*
	 * Broken HW is sometimes missing the pull-up resistor on the
	 * MDIO line, which results in reads to non-existent devices returning
	 * 0 rather than 0xffff. Catch this here and treat 0 as a non-existent
	 * device as well.
	 * Note: phydev->phy_id is the result of reading the UID PHY registers.
	 */
	if (phydev->phy_id == 0) {
		phy_disconnect(phydev);
		return -ENODEV;
	}
	GMAC_DBG(("stmmac_init_phy:  %s: attached to PHY (UID 0x%x)"
		 " Link = %d\n", dev->name, phydev->phy_id, phydev->link));

	priv->phydev = phydev;

	return 0;
}

/**
 * stmmac_display_ring: display ring
 * @head: pointer to the head of the ring passed.
 * @size: size of the ring.
 * @extend_desc: to verify if extended descriptors are used.
 * Description: display the control/status and buffer descriptors.
 */
static void stmmac_display_ring(void *head, int size, int extend_desc)
{
	int i;
	struct dma_extended_desc *ep = (struct dma_extended_desc *)head;
	struct dma_desc *p = (struct dma_desc *)head;

	for (i = 0; i < size; i++) {
		u64 x;
		if (extend_desc) {
			x = *(u64 *) ep;
			GMAC_DBG(("%d [0x%x]: 0x%x 0x%x 0x%x 0x%x\n",
				i, (unsigned int)virt_to_phys(ep),
				(unsigned int)x, (unsigned int)(x >> 32),
				ep->basic.des2, ep->basic.des3));
			ep++;
		} else {
			x = *(u64 *) p;
			GMAC_DBG(("%d [0x%x]: 0x%x 0x%x 0x%x 0x%x",
				i, (unsigned int)virt_to_phys(p),
				(unsigned int)x, (unsigned int)(x >> 32),
				p->des2, p->des3));
			p++;
		}
		GMAC_DBG(("\n"));
	}
}

static void stmmac_display_rings(struct stmmac_priv *priv)
{
	unsigned int txsize = priv->dma_tx_size;
	unsigned int rxsize = priv->dma_rx_size;

	if (priv->extend_desc) {
		GMAC_INFO(("Extended RX descriptor ring:\n"));
		stmmac_display_ring((void *)priv->dma_erx, rxsize, 1);
		GMAC_INFO(("Extended TX descriptor ring:\n"));
		stmmac_display_ring((void *)priv->dma_etx, txsize, 1);
	} else {
		GMAC_INFO(("RX descriptor ring:\n"));
		stmmac_display_ring((void *)priv->dma_rx, rxsize, 0);
		GMAC_INFO(("TX descriptor ring:\n"));
		stmmac_display_ring((void *)priv->dma_tx, txsize, 0);
	}
}

/**
 * stmmac_clear_descriptors: clear descriptors
 * @priv: driver private structure
 * Description: this function is called to clear the tx and rx descriptors
 * in case of both basic and extended descriptors are used.
 */
static void stmmac_clear_descriptors(struct stmmac_priv *priv)
{
	int i;
	unsigned int txsize = priv->dma_tx_size;
	unsigned int rxsize = priv->dma_rx_size;

	/* Clear the Rx/Tx descriptors */
	for (i = 0; i < rxsize; i++)
		if (priv->extend_desc)
			priv->hw->desc->init_rx_desc(&priv->dma_erx[i].basic,
						     priv->use_riwt, priv->mode,
						     (i == rxsize - 1));
		else
			priv->hw->desc->init_rx_desc(&priv->dma_rx[i],
						     priv->use_riwt, priv->mode,
						     (i == rxsize - 1));
	for (i = 0; i < txsize; i++)
		if (priv->extend_desc)
			priv->hw->desc->init_tx_desc(&priv->dma_etx[i].basic,
						     priv->mode,
						     (i == txsize - 1));
		else
			priv->hw->desc->init_tx_desc(&priv->dma_tx[i],
						     priv->mode,
						     (i == txsize - 1));
}

static int stmmac_init_rx_buffers(struct stmmac_priv *priv, struct dma_desc *p,
				  int i)
{
	struct sk_buff *skb;

	if(!priv->rx_skbuff || !priv->rx_skbuff_dma){
		GMAC_ERR(("%s: Rx init fails;priv is incomplete\n", __func__));
		return -1;
	}
	
	skb = __netdev_alloc_skb(priv->dev, priv->dma_buf_sz + NET_IP_ALIGN,
				 GFP_KERNEL);
	if (unlikely(skb == NULL)) {
		GMAC_ERR(("%s: Rx init fails; skb is NULL\n", __func__));
		return 1;
	}
	skb_reserve(skb, NET_IP_ALIGN);
	priv->rx_skbuff[i] = skb;
	priv->rx_skbuff_dma[i] = dma_map_single(priv->device, skb->data,
						priv->dma_buf_sz,
						DMA_FROM_DEVICE);
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
        if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
            spe_hook.set_skb_dma(skb, priv->rx_skbuff_dma[i]);
        }
#endif
	p->des2 = priv->rx_skbuff_dma[i];

	if ((priv->mode == STMMAC_RING_MODE) &&
	    (priv->dma_buf_sz == BUF_SIZE_16KiB))
		priv->hw->ring->init_desc3(p);

	return 0;
}

/**
 * init_dma_desc_rings - init the RX/TX descriptor rings
 * @dev: net device structure
 * Description:  this function initializes the DMA RX/TX descriptors
 * and allocates the socket buffers. It suppors the chained and ring
 * modes.
 */
static void init_dma_desc_rings(struct net_device *dev)
{
	int i;
	struct stmmac_priv *priv = netdev_priv(dev);
	unsigned int txsize = priv->dma_tx_size;
	unsigned int rxsize = priv->dma_rx_size;
	unsigned int bfsize = 0;

	/* Set the max buffer size according to the DESC mode
	 * and the MTU. Note that RING mode allows 16KiB bsize.
	 */
	if (priv->mode == STMMAC_RING_MODE)
		bfsize = priv->hw->ring->set_16kib_bfsize(dev->mtu);

	if (bfsize < BUF_SIZE_16KiB)
        bfsize = BUF_SIZE_1_8kiB;


	GMAC_TRACE(("stmmac: txsize %d, rxsize %d, bfsize %d\n",
	    txsize, rxsize, bfsize));

	if (priv->extend_desc) {
		priv->dma_erx = dma_alloc_coherent(priv->device, rxsize *
						   sizeof(struct
							  dma_extended_desc),
						   &priv->dma_rx_phy,
						   GFP_KERNEL);
		priv->dma_etx = dma_alloc_coherent(priv->device, txsize *
						   sizeof(struct
							  dma_extended_desc),
						   &priv->dma_tx_phy,
						   GFP_KERNEL);
		if ((!priv->dma_erx) || (!priv->dma_etx))
			return;
	} else {
		priv->dma_rx = dma_alloc_coherent(priv->device, rxsize *
						  sizeof(struct dma_desc),
						  &priv->dma_rx_phy,
						  GFP_KERNEL);
		priv->dma_tx = dma_alloc_coherent(priv->device, txsize *
						  sizeof(struct dma_desc),
						  &priv->dma_tx_phy,
						  GFP_KERNEL);
		if ((!priv->dma_rx) || (!priv->dma_tx))
			return;
	}

	priv->rx_skbuff_dma = kmalloc_array(rxsize, sizeof(dma_addr_t),
					    GFP_KERNEL);
	priv->rx_skbuff = kmalloc_array(rxsize, sizeof(struct sk_buff *),
					GFP_KERNEL);
	priv->tx_skbuff_dma = kmalloc_array(txsize, sizeof(dma_addr_t),
					    GFP_KERNEL);
	priv->tx_skbuff = kmalloc_array(txsize, sizeof(struct sk_buff *),
					GFP_KERNEL);
	if (netif_msg_drv(priv))
		GMAC_DBG(("(%s) dma_rx_phy=0x%08x dma_tx_phy=0x%08x\n", __func__,
			 (u32) priv->dma_rx_phy, (u32) priv->dma_tx_phy));

	/* RX INITIALIZATION */
	GMAC_TRACE(("stmmac: SKB addresses:\nskb\t\tskb data\tdma data\n"));

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
    if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
        skb_queue_head_init(&priv->free_q);
    }
#endif

	for (i = 0; i < rxsize; i++) {
		struct dma_desc *p;
		if (priv->extend_desc)
			p = &((priv->dma_erx + i)->basic);
		else
			p = priv->dma_rx + i;

		if (stmmac_init_rx_buffers(priv, p, i))
			break;

		GMAC_TRACE(("[%p]\t[%p]\t[%x]\n", priv->rx_skbuff[i],
		    priv->rx_skbuff[i]->data, priv->rx_skbuff_dma[i]));
	}
	priv->cur_rx = 0;
	priv->dirty_rx = (unsigned int)(i - rxsize);
	priv->dma_buf_sz = bfsize;
	buf_sz = bfsize;

	/* Setup the chained descriptor addresses */
	if (priv->mode == STMMAC_CHAIN_MODE) {
		if (priv->extend_desc) {
			priv->hw->chain->init(priv->dma_erx, priv->dma_rx_phy,
					      rxsize, 1);
			priv->hw->chain->init(priv->dma_etx, priv->dma_tx_phy,
					      txsize, 1);
		} else {
			priv->hw->chain->init(priv->dma_rx, priv->dma_rx_phy,
					      rxsize, 0);
			priv->hw->chain->init(priv->dma_tx, priv->dma_tx_phy,
					      txsize, 0);
		}
	}

	/* TX INITIALIZATION */

	if(!priv->tx_skbuff || !priv->tx_skbuff_dma){
		pr_err("%s: tx_skbuff or tx_skbuff_dma is null\n",__func__);
		return;
	}
	
	for (i = 0; i < txsize; i++) {
		struct dma_desc *p;
		if (priv->extend_desc)
			p = &((priv->dma_etx + i)->basic);
		else
			p = priv->dma_tx + i;
		p->des2 = 0;
		priv->tx_skbuff_dma[i] = 0;
		priv->tx_skbuff[i] = NULL;
	}

	priv->dirty_tx = 0;
	priv->cur_tx = 0;

	stmmac_clear_descriptors(priv);

	if (netif_msg_hw(priv))
		stmmac_display_rings(priv);
}

static void dma_free_rx_skbufs(struct stmmac_priv *priv)
{
	int i;

	for (i = 0; i < priv->dma_rx_size; i++) {
		if (priv->rx_skbuff[i]) {
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
            if (!priv->rx_skbuff[i]->spe_own)
#endif
			{
			dma_unmap_single(priv->device, priv->rx_skbuff_dma[i],
					 priv->dma_buf_sz, DMA_FROM_DEVICE);
			dev_kfree_skb_any(priv->rx_skbuff[i]);
			}
		}
		priv->rx_skbuff[i] = NULL;
	}
}

static void dma_free_tx_skbufs(struct stmmac_priv *priv)
{
	int i;

	for (i = 0; i < priv->dma_tx_size; i++) {
		if (priv->tx_skbuff[i] != NULL) {
			struct dma_desc *p;
			if (priv->extend_desc)
				p = &((priv->dma_etx + i)->basic);
			else
				p = priv->dma_tx + i;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
            if (priv->tx_skbuff[i]->spe_own) {
                if (priv->tx_skbuff_dma[i]) {
                    spe_hook.rd_config(priv->tx_skbuff[i]->spe_own, 
                        priv->tx_skbuff[i], priv->tx_skbuff_dma[i]);
                }
                   
            } else
#endif
			{
				if (priv->tx_skbuff_dma[i])
						dma_unmap_single(priv->device,
						priv->tx_skbuff_dma[i],
						priv->hw->desc->get_tx_len(p),
						DMA_TO_DEVICE);
				dev_kfree_skb_any(priv->tx_skbuff[i]);
			}
			priv->tx_skbuff[i] = NULL;
			priv->tx_skbuff_dma[i] = 0;
		}
	}
}

static void free_dma_desc_resources(struct stmmac_priv *priv)
{
	/* Release the DMA TX/RX socket buffers */
	dma_free_rx_skbufs(priv);
	dma_free_tx_skbufs(priv);

	/* Free DMA regions of consistent memory previously allocated */
	if (!priv->extend_desc) {
		dma_free_coherent(priv->device,
				  priv->dma_tx_size * sizeof(struct dma_desc),
				  priv->dma_tx, priv->dma_tx_phy);
		dma_free_coherent(priv->device,
				  priv->dma_rx_size * sizeof(struct dma_desc),
				  priv->dma_rx, priv->dma_rx_phy);
	} else {
		dma_free_coherent(priv->device, priv->dma_tx_size *
				  sizeof(struct dma_extended_desc),
				  priv->dma_etx, priv->dma_tx_phy);
		dma_free_coherent(priv->device, priv->dma_rx_size *
				  sizeof(struct dma_extended_desc),
				  priv->dma_erx, priv->dma_rx_phy);
	}
	kfree(priv->rx_skbuff_dma);
	kfree(priv->rx_skbuff);
	kfree(priv->tx_skbuff_dma);
	kfree(priv->tx_skbuff);
}

/**
 *  stmmac_dma_operation_mode - HW DMA operation mode
 *  @priv: driver private structure
 *  Description: it sets the DMA operation mode: tx/rx DMA thresholds
 *  or Store-And-Forward capability.
 */
static void stmmac_dma_operation_mode(struct stmmac_priv *priv)
{
	if (likely(priv->plat->force_sf_dma_mode ||
		   ((priv->plat->tx_coe) && (!priv->no_csum_insertion)))) {
		/*
		 * In case of GMAC, SF mode can be enabled
		 * to perform the TX COE in HW. This depends on:
		 * 1) TX COE if actually supported
		 * 2) There is no bugged Jumbo frame support
		 *    that needs to not insert csum in the TDES.
		 */
		priv->hw->dma->dma_mode(priv->ioaddr, SF_DMA_MODE, SF_DMA_MODE);
		tc = SF_DMA_MODE;
	} else
		priv->hw->dma->dma_mode(priv->ioaddr, tc, SF_DMA_MODE);
}

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
static int
stmmac_finish_rd(int portno, int src_portno, struct sk_buff *skb, dma_addr_t dma, unsigned int flags)
{
    struct net_device *dev;

	BUG_ON(!spe_hook.port_netdev);
	dev = spe_hook.port_netdev(portno);

    if (!spe_hook.port_is_enable(portno)) {
        dev->stats.tx_dropped++;
        return NETDEV_TX_BUSY;
            
    }
    return stmmac_spe_xmit(skb, dev);
}

static void stmmac_reset_rx_skb_spe(struct sk_buff *skb)
{
    /* be care, rx skb is reserved NET_IP_ALIGN and NET_SKB_PAD */
    //skb->data = skb->head;
    //skb_reserve(skb, NET_IP_ALIGN + NET_SKB_PAD);
	skb_reset_tail_pointer(skb);
    skb->len = 0;
    skb->cloned = 0;
}

static int
stmmac_finish_td(int portno, struct sk_buff *skb, unsigned int flags)
{
    int ret = 0;
    struct net_device *dev;
    struct stmmac_priv *priv;

	BUG_ON(!spe_hook.port_netdev);
	dev = spe_hook.port_netdev(portno);
    priv = netdev_priv(dev);
#ifndef CONFIG_GMAC_TEST
	stmmac_reset_rx_skb_spe(skb);

	skb_queue_tail(&priv->free_q, skb);

	stmmac_rx_refill(priv);
#endif

    return ret;
}
#endif

/**
 * stmmac_tx_clean:
 * @priv: driver private structure
 * Description: it reclaims resources after transmission completes.
 */
static void stmmac_tx_clean(struct stmmac_priv *priv)
{
	unsigned int txsize = priv->dma_tx_size;
	unsigned long flags;



	spin_lock_irqsave(&priv->tx_lock, flags);

	priv->xstats.tx_clean++;

	while (priv->dirty_tx != priv->cur_tx) {
		int last;
		unsigned int entry = priv->dirty_tx % txsize;
		struct sk_buff *skb = priv->tx_skbuff[entry];
		struct dma_desc *p;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
        dma_addr_t dma;
#endif

		if (priv->extend_desc)
			p = (struct dma_desc *)(priv->dma_etx + entry);
		else
			p = priv->dma_tx + entry;

		/* Check if the descriptor is owned by the DMA. */
		if (priv->hw->desc->get_tx_owner(p))
			break;

		/* Verify tx error by looking at the last segment. */
		last = priv->hw->desc->get_tx_ls(p);
		if (likely(last)) {
			int tx_error =
			    priv->hw->desc->tx_status(&priv->dev->stats,
						      &priv->xstats, p,
						      priv->ioaddr);
			if (likely(tx_error == 0)) {
				priv->dev->stats.tx_packets++;
				priv->xstats.tx_pkt_n++;
			} else
				priv->dev->stats.tx_errors++;

			stmmac_get_tx_hwtstamp(priv, entry, skb);
		}
		GMAC_DBG(("%s: curr %d, dirty %d\n", __func__,
		       priv->cur_tx, priv->dirty_tx));

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
		if (skb->spe_own) {
            priv->tx_skbuff_dma[entry] = 0;
		}else
#endif
		{
			if (likely(priv->tx_skbuff_dma[entry])) {
				dma_unmap_single(priv->device,
						 priv->tx_skbuff_dma[entry],
						 priv->hw->desc->get_tx_len(p),
						 DMA_TO_DEVICE);
				priv->tx_skbuff_dma[entry] = 0;
			}
		}
		priv->hw->ring->clean_desc3(priv, p);

		if (likely(skb != NULL)) {
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
            if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
                if (skb->spe_own) {
                    //stmmac_reset_tx_skb_spe(skb, priv->dma_tx);
                    dma = spe_hook.get_skb_dma(skb);
                    spe_hook.rd_config(skb->spe_own, skb, dma);
                }
                else {
        	        dev_kfree_skb_any(skb);
                }
            }else
#endif
			{
				dev_kfree_skb_any(skb);
			}
			priv->tx_skbuff[entry] = NULL;
		}

		priv->hw->desc->release_tx_desc(p, priv->mode);

		priv->dirty_tx++;
	}
	if (unlikely(netif_queue_stopped(priv->dev) &&
		     stmmac_tx_avail(priv) > STMMAC_TX_THRESH(priv))) {
		netif_tx_lock(priv->dev);
		if (netif_queue_stopped(priv->dev) &&
		    stmmac_tx_avail(priv) > STMMAC_TX_THRESH(priv)) {
			GMAC_DBG(("%s: restart transmit\n", __func__));
			netif_wake_queue(priv->dev);
		}
		netif_tx_unlock(priv->dev);
	}

	if ((priv->eee_enabled) && (!priv->tx_path_in_lpi_mode)) {
		stmmac_enable_eee_mode(priv);
		mod_timer(&priv->eee_ctrl_timer, STMMAC_LPI_T(eee_timer));
	}
	spin_unlock_irqrestore(&priv->tx_lock, flags);
}

static inline void stmmac_enable_dma_irq(struct stmmac_priv *priv)
{
	priv->hw->dma->enable_dma_irq(priv->ioaddr);
}

static inline void stmmac_disable_dma_irq(struct stmmac_priv *priv)
{
	priv->hw->dma->disable_dma_irq(priv->ioaddr);
}

/**
 * stmmac_tx_err: irq tx error mng function
 * @priv: driver private structure
 * Description: it cleans the descriptors and restarts the transmission
 * in case of errors.
 */
static void stmmac_tx_err(struct stmmac_priv *priv)
{

#if 0
	int i;
	int txsize = priv->dma_tx_size;


	netif_stop_queue(priv->dev);

	priv->hw->dma->stop_tx(priv->ioaddr);
	dma_free_tx_skbufs(priv);
	for (i = 0; i < txsize; i++)
		if (priv->extend_desc)
			priv->hw->desc->init_tx_desc(&priv->dma_etx[i].basic,
						     priv->mode,
						     (i == txsize - 1));
		else
			priv->hw->desc->init_tx_desc(&priv->dma_tx[i],
						     priv->mode,
						     (i == txsize - 1));
	priv->dirty_tx = 0;
	priv->cur_tx = 0;
	priv->hw->dma->start_tx(priv->ioaddr);
#endif

	priv->dev->stats.tx_errors++;
	netif_wake_queue(priv->dev);
}

/**
 * stmmac_dma_interrupt: DMA ISR
 * @priv: driver private structure
 * Description: this is the DMA ISR. It is called by the main ISR.
 * It calls the dwmac dma routine to understand which type of interrupt
 * happened. In case of there is a Normal interrupt and either TX or RX
 * interrupt happened so the NAPI is scheduled.
 */
static void stmmac_dma_interrupt(struct stmmac_priv *priv)
{
	int status;

	status = priv->hw->dma->dma_interrupt(priv->ioaddr, &priv->xstats);
	if (likely((status & handle_rx)) || (status & handle_tx)) {
		if (likely(napi_schedule_prep(&priv->napi))) {
			stmmac_disable_dma_irq(priv);
			__napi_schedule(&priv->napi);
		}
	}
	if (unlikely(status & tx_hard_error_bump_tc)) {
		/* Try to bump up the dma threshold on this failure */
		if (unlikely(tc != SF_DMA_MODE) && (tc <= 256)) {
			tc += 64;
			priv->hw->dma->dma_mode(priv->ioaddr, tc, SF_DMA_MODE);
			priv->xstats.threshold = tc;
		}
	} else if (unlikely(status == tx_hard_error))
		stmmac_tx_err(priv);
}

/**
 * stmmac_mmc_setup: setup the Mac Management Counters (MMC)
 * @priv: driver private structure
 * Description: this masks the MMC irq, in fact, the counters are managed in SW.
 */
static void stmmac_mmc_setup(struct stmmac_priv *priv)
{
	unsigned int mode = /*MMC_CNTRL_RESET_ON_READ | */MMC_CNTRL_COUNTER_RESET |
			    MMC_CNTRL_PRESET | MMC_CNTRL_FULL_HALF_PRESET;

	dwmac_mmc_intr_all_mask(priv->ioaddr);

	if (priv->dma_cap.rmon) {
		dwmac_mmc_ctrl(priv->ioaddr, mode);
		memset(&priv->mmc, 0, sizeof(struct stmmac_counters));
	} else
		GMAC_INFO(("No MAC Management Counters available\n"));
}

static u32 stmmac_get_synopsys_id(struct stmmac_priv *priv)
{
	u32 hwid = priv->hw->synopsys_uid;

	/* Check Synopsys Id (not available on old chips) */
	if (likely(hwid)) {
		u32 uid = ((hwid & 0x0000ff00) >> 8);
		u32 synid = (hwid & 0x000000ff);

		GMAC_INFO(("stmmac - user ID: 0x%x, Synopsys ID: 0x%x\n",
			uid, synid));

		return synid;
	}
	return 0;
}

/**
 * stmmac_selec_desc_mode: to select among: normal/alternate/extend descriptors
 * @priv: driver private structure
 * Description: select the Enhanced/Alternate or Normal descriptors.
 * In case of Enhanced/Alternate, it looks at the extended descriptors are
 * supported by the HW cap. register.
 */
static void stmmac_selec_desc_mode(struct stmmac_priv *priv)
{
	if (priv->plat->enh_desc) {
		pr_info(" Enhanced/Alternate descriptors\n");

		/* GMAC older than 3.50 has no extended descriptors */
		if (priv->synopsys_id >= DWMAC_CORE_3_50) {
			GMAC_INFO(("\tEnabled extended descriptors\n"));
			priv->extend_desc = priv->plat->enh_desc_atds;
		} else
			GMAC_INFO(("Extended descriptors not supported\n"));

		priv->hw->desc = &enh_desc_ops;
	} else {
		pr_info(" Normal descriptors\n");
		priv->hw->desc = &ndesc_ops;
	}
}

/**
 * stmmac_get_hw_features: get MAC capabilities from the HW cap. register.
 * @priv: driver private structure
 * Description:
 *  new GMAC chip generations have a new register to indicate the
 *  presence of the optional feature/functions.
 *  This can be also used to override the value passed through the
 *  platform and necessary for old MAC10/100 and GMAC chips.
 */
static int stmmac_get_hw_features(struct stmmac_priv *priv)
{
	u32 hw_cap = 0;

	if (priv->hw->dma->get_hw_feature) {
		hw_cap = priv->hw->dma->get_hw_feature(priv->ioaddr);

		priv->dma_cap.mbps_10_100 = (hw_cap & DMA_HW_FEAT_MIISEL);
		priv->dma_cap.mbps_1000 = (hw_cap & DMA_HW_FEAT_GMIISEL) >> 1;
		priv->dma_cap.half_duplex = (hw_cap & DMA_HW_FEAT_HDSEL) >> 2;
		priv->dma_cap.hash_filter = (hw_cap & DMA_HW_FEAT_HASHSEL) >> 4;
		priv->dma_cap.multi_addr = (hw_cap & DMA_HW_FEAT_ADDMAC) >> 5;
		priv->dma_cap.pcs = (hw_cap & DMA_HW_FEAT_PCSSEL) >> 6;
		priv->dma_cap.sma_mdio = (hw_cap & DMA_HW_FEAT_SMASEL) >> 8;
		priv->dma_cap.pmt_remote_wake_up =
		    (hw_cap & DMA_HW_FEAT_RWKSEL) >> 9;
		priv->dma_cap.pmt_magic_frame =
		    (hw_cap & DMA_HW_FEAT_MGKSEL) >> 10;
		/* MMC */
		priv->dma_cap.rmon = (hw_cap & DMA_HW_FEAT_MMCSEL) >> 11;
		/* IEEE 1588-2002 */
		priv->dma_cap.time_stamp =
		    (hw_cap & DMA_HW_FEAT_TSVER1SEL) >> 12;
		/* IEEE 1588-2008 */
		priv->dma_cap.atime_stamp =
		    (hw_cap & DMA_HW_FEAT_TSVER2SEL) >> 13;
		/* 802.3az - Energy-Efficient Ethernet (EEE) */
		priv->dma_cap.eee = (hw_cap & DMA_HW_FEAT_EEESEL) >> 14;
		priv->dma_cap.av = (hw_cap & DMA_HW_FEAT_AVSEL) >> 15;
		/* TX and RX csum */
		priv->dma_cap.tx_coe = (hw_cap & DMA_HW_FEAT_TXCOESEL) >> 16;
		priv->dma_cap.rx_coe_type1 =
		    (hw_cap & DMA_HW_FEAT_RXTYP1COE) >> 17;
		priv->dma_cap.rx_coe_type2 =
		    (hw_cap & DMA_HW_FEAT_RXTYP2COE) >> 18;
		priv->dma_cap.rxfifo_over_2048 =
		    (hw_cap & DMA_HW_FEAT_RXFIFOSIZE) >> 19;
		/* TX and RX number of channels */
		priv->dma_cap.number_rx_channel =
		    (hw_cap & DMA_HW_FEAT_RXCHCNT) >> 20;
		priv->dma_cap.number_tx_channel =
		    (hw_cap & DMA_HW_FEAT_TXCHCNT) >> 22;
		/* Alternate (enhanced) DESC mode */
		priv->dma_cap.enh_desc = (hw_cap & DMA_HW_FEAT_ENHDESSEL) >> 24;
	}

	return hw_cap;
}

/**
 * stmmac_check_ether_addr: check if the MAC addr is valid
 * @priv: driver private structure
 * Description:
 * it is to verify if the MAC address is valid, in case of failures it
 * generates a random MAC address
 */
static void stmmac_check_ether_addr(struct stmmac_priv *priv)
{
	if (!is_valid_ether_addr(priv->dev->dev_addr)) {
		priv->hw->mac->get_umac_addr((void __iomem *)
					     priv->dev->base_addr,
					     priv->dev->dev_addr, 0);
		if (!is_valid_ether_addr(priv->dev->dev_addr))
			eth_hw_addr_random(priv->dev);
	}
	GMAC_WARNING(("%s: device MAC address %pM\n", priv->dev->name,
		priv->dev->dev_addr));
}

/**
 * stmmac_init_dma_engine: DMA init.
 * @priv: driver private structure
 * Description:
 * It inits the DMA invoking the specific MAC/GMAC callback.
 * Some DMA parameters can be passed from the platform;
 * in case of these are not passed a default is kept for the MAC or GMAC.
 */
static int stmmac_init_dma_engine(struct stmmac_priv *priv)
{
	int pbl = DEFAULT_DMA_PBL, fixed_burst = 0, burst_len = 0;
	int mixed_burst = 0;
	int atds = 0;

	if (priv->plat->dma_cfg) {
		pbl = priv->plat->dma_cfg->pbl;
		fixed_burst = priv->plat->dma_cfg->fixed_burst;
		mixed_burst = priv->plat->dma_cfg->mixed_burst;
		burst_len = priv->plat->dma_cfg->burst_len;
	}

	if (priv->extend_desc && (priv->mode == STMMAC_RING_MODE))
		atds = 1;

	return priv->hw->dma->init(priv->ioaddr, pbl, fixed_burst, mixed_burst,
				   burst_len, priv->dma_tx_phy,
				   priv->dma_rx_phy, atds);
}

/**
 * stmmac_tx_timer: mitigation sw timer for tx.
 * @data: data pointer
 * Description:
 * This is the timer handler to directly invoke the stmmac_tx_clean.
 */
static void stmmac_tx_timer(unsigned long data)
{
	struct stmmac_priv *priv = (struct stmmac_priv *)data;

	stmmac_tx_clean(priv);
}

/**
 * stmmac_init_tx_coalesce: init tx mitigation options.
 * @priv: driver private structure
 * Description:
 * This inits the transmit coalesce parameters: i.e. timer rate,
 * timer handler and default threshold used for enabling the
 * interrupt on completion bit.
 */
static void stmmac_init_tx_coalesce(struct stmmac_priv *priv)
{
	priv->tx_coal_frames = STMMAC_TX_FRAMES;
	priv->tx_coal_timer = STMMAC_COAL_TX_TIMER;
	init_timer(&priv->txtimer);
	priv->txtimer.expires = STMMAC_COAL_TIMER(priv->tx_coal_timer);
	priv->txtimer.data = (unsigned long)priv;
	priv->txtimer.function = stmmac_tx_timer;
	add_timer(&priv->txtimer);
}
#if (FEATURE_ON == MBB_FEATURE_ETH)
static void stmmac_link_init(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	struct phy_device *phydev = priv->phydev;
	unsigned int fc = priv->flow_ctrl, pause_time = priv->pause;

	if (NULL == phydev)
		return;

	if (phydev->link) 
    {
		u32 ctrl = readl(priv->ioaddr + MAC_CTRL_REG);

        if (!(phydev->duplex))
			ctrl &= ~priv->hw->link.duplex;
		else
			ctrl |= priv->hw->link.duplex;
		priv->oldduplex = phydev->duplex;
		
		/* Flow Control operation */
		if (phydev->pause)
			priv->hw->mac->flow_ctrl(priv->ioaddr, phydev->duplex,
						 fc, pause_time);

    	switch (phydev->speed) {
		case 1000:
			if (likely(priv->plat->has_gmac))
				ctrl &= ~priv->hw->link.port;
				stmmac_hw_fix_mac_speed(priv);
			break;
		case 100:
		case 10:
			if (priv->plat->has_gmac) {
				ctrl |= priv->hw->link.port;
				if (phydev->speed == SPEED_100) {
					ctrl |= priv->hw->link.speed;
				} else {
					ctrl &= ~(priv->hw->link.speed);
				}
			} else {
				ctrl &= ~priv->hw->link.port;
			}
			stmmac_hw_fix_mac_speed(priv);
			break;
		default:
			break;
		}

		priv->speed = phydev->speed;
		
		writel(ctrl, priv->ioaddr + MAC_CTRL_REG);

		priv->oldlink = 1;
		
	} 
}
#endif
/**
 *  stmmac_open - open entry point of the driver
 *  @dev : pointer to the device structure.
 *  Description:
 *  This function is the open entry point of the driver.
 *  Return value:
 *  0 on success and an appropriate (-)ve integer as defined in errno.h
 *  file on failure.
 */
static int stmmac_open(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	int ret;

    stmmac_clk_enable(priv->device);

	stmmac_check_ether_addr(priv);

	if (priv->pcs != STMMAC_PCS_RGMII && priv->pcs != STMMAC_PCS_TBI &&
	    priv->pcs != STMMAC_PCS_RTBI) {
		ret = stmmac_init_phy(dev);
		if (ret) {
			GMAC_ERR(("%s: Cannot attach to PHY (error: %d)\n",
			       __func__, ret));
			goto open_error;
		}
	}
	
#if (FEATURE_ON == MBB_CTF_COMMON)
    mbb_ctf_init(dev);
#endif
#if (FEATURE_ON == MBB_ETH_STMMAC_TX_THREAD)
    stmmac_vendor_tx_thread_init(priv);
#endif
#if ((FEATURE_ON == MBB_FEATURE_FASTIP) && (FEATURE_ON == MBB_ETH_STMMAC_TX_THREAD))
    mbb_fastip_init(dev);
#endif
	/* Create and initialize the TX/RX descriptors chains. */
	priv->dma_tx_size = STMMAC_ALIGN(dma_txsize);
	priv->dma_rx_size = STMMAC_ALIGN(dma_rxsize);
	priv->dma_buf_sz = STMMAC_ALIGN(buf_sz);
	init_dma_desc_rings(dev);

	/* DMA initialization and SW reset */
	ret = stmmac_init_dma_engine(priv);
	if (ret < 0) {
		GMAC_ERR(("%s: DMA initialization failed\n", __func__));
		goto open_error;
	}

	/* Copy the MAC addr into the HW  */
	priv->hw->mac->set_umac_addr(priv->ioaddr, dev->dev_addr, 0);

	/* If required, perform hw setup of the bus. */
	if (priv->plat->bus_setup)
		priv->plat->bus_setup(priv->ioaddr);

	/* Initialize the MAC Core */
	priv->hw->mac->core_init(priv->ioaddr);


	/* Alloc spe port and enable it */
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		(void)spe_hook.port_enable(priv->portno);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
        (void)spe_hook.port_enable(priv->portno_wan);
#endif

	}
#endif

	/* Request the IRQ lines */
	ret = request_irq(dev->irq, stmmac_interrupt,
			  IRQF_SHARED, dev->name, dev);
	if (unlikely(ret < 0)) {
		GMAC_ERR(("%s: ERROR: allocating the IRQ %d (error: %d)\n",
		       __func__, dev->irq, ret));
		goto open_error;
	}

	/* Request the Wake IRQ in case of another line is used for WoL */
	if (priv->wol_irq != dev->irq) {
		ret = request_irq(priv->wol_irq, stmmac_interrupt,
				  IRQF_SHARED, dev->name, dev);
		if (unlikely(ret < 0)) {
			GMAC_ERR(("%s: ERROR: allocating the WoL IRQ %d (%d)\n",
			       __func__, priv->wol_irq, ret));
			goto open_error_wolirq;
		}
	}

	/* Request the IRQ lines */
	if (priv->lpi_irq != -ENXIO) {
		ret = request_irq(priv->lpi_irq, stmmac_interrupt, IRQF_SHARED,
				  dev->name, dev);
		if (unlikely(ret < 0)) {
			GMAC_ERR(("%s: ERROR: allocating the LPI IRQ %d (%d)\n",
			       __func__, priv->lpi_irq, ret));
			goto open_error_lpiirq;
		}
	}

	/* Enable the MAC Rx/Tx */
	stmmac_set_mac(priv->ioaddr, true);

	/* Set the HW DMA mode and the COE */
	stmmac_dma_operation_mode(priv);

	/* Extra statistics */
	memset(&priv->xstats, 0, sizeof(struct stmmac_extra_stats));
	priv->xstats.threshold = tc;
	gmac_status = &priv->xstats;

	stmmac_mmc_setup(priv);

	ret = stmmac_init_ptp(priv);
	if (ret)
		GMAC_WARNING(("%s: failed PTP initialisation\n", __func__));

#ifdef CONFIG_STMMAC_DEBUG_FS
	ret = stmmac_init_fs(dev);
	if (ret < 0)
		GMAC_WARNING(("%s: failed debugFS registration\n", __func__));
#endif
	/* Start the ball rolling... */
	GMAC_DBG(( "%s: DMA RX/TX processes started...\n", dev->name));
	priv->hw->dma->start_tx(priv->ioaddr);
	priv->hw->dma->start_rx(priv->ioaddr);

	priv->hw->dma->rx_watchdog(priv->ioaddr, riwt_value);

	/* Dump DMA/MAC registers */
	if (netif_msg_hw(priv)) {
		priv->hw->mac->dump_regs(priv->ioaddr);
		priv->hw->dma->dump_regs(priv->ioaddr);
	}
#if (FEATURE_ON == MBB_FEATURE_ETH)
    stmmac_link_init(dev);
#endif

	if (priv->phydev)
		phy_start(priv->phydev);

	priv->tx_lpi_timer = STMMAC_DEFAULT_TWT_LS;

	priv->eee_enabled = stmmac_eee_init(priv);


	if ((priv->use_riwt) && (priv->hw->dma->rx_watchdog)) {
		priv->rx_riwt = MIN_DMA_RIWT;
		priv->hw->dma->rx_watchdog(priv->ioaddr, MIN_DMA_RIWT);
	}

	if (priv->pcs && priv->hw->mac->ctrl_ane)
		priv->hw->mac->ctrl_ane(priv->ioaddr, 0);

	napi_enable(&priv->napi);
	netif_start_queue(dev);

	return 0;

open_error_lpiirq:
	if (priv->wol_irq != dev->irq)
		free_irq(priv->wol_irq, dev);

open_error_wolirq:
	free_irq(dev->irq, dev);

open_error:
	if (priv->phydev)
		phy_disconnect(priv->phydev);

	return ret;
}


/**
 *  stmmac_release - close entry point of the driver
 *  @dev : device pointer.
 *  Description:
 *  This is the stop entry point of the driver.
 */
static int stmmac_release(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	int ret=0;

	if (priv->eee_enabled)
		del_timer_sync(&priv->eee_ctrl_timer);


#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		(void)spe_hook.port_disable(priv->portno);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
        (void)spe_hook.port_disable(priv->portno_wan);
#endif

	}
#endif
	/* Stop and disconnect the PHY */
	if (priv->phydev) {
		phy_stop(priv->phydev);
		phy_disconnect(priv->phydev);
		priv->phydev = NULL;
	}

	netif_stop_queue(dev);
	priv->xstats.gmac_release++;

	napi_disable(&priv->napi);


	ret = del_timer_sync(&priv->txtimer);	


	/* Free the IRQ lines */
	free_irq(dev->irq, dev);
	if (priv->wol_irq != dev->irq)
		free_irq(priv->wol_irq, dev);
	if (priv->lpi_irq != -ENXIO)
		free_irq(priv->lpi_irq, dev);

	/* Stop TX/RX DMA and clear the descriptors */
	priv->hw->dma->stop_tx(priv->ioaddr);
	priv->hw->dma->stop_rx(priv->ioaddr);

	/* Release and free the Rx/Tx resources */
	free_dma_desc_resources(priv);

	/* Disable the MAC Rx/Tx */
	stmmac_set_mac(priv->ioaddr, false);

	netif_carrier_off(dev);

#ifdef CONFIG_STMMAC_DEBUG_FS
	stmmac_exit_fs();
#endif
#if (FEATURE_ON == MBB_CTF_COMMON)
    mbb_ctf_exit(dev);
#endif
#if ((FEATURE_ON == MBB_FEATURE_FASTIP) && (FEATURE_ON == MBB_ETH_STMMAC_TX_THREAD))
    mbb_fastip_exit(dev);
#endif
#if (FEATURE_ON == MBB_ETH_STMMAC_TX_THREAD)
    stmmac_vendor_tx_thread_uninit(priv);
#endif
	stmmac_release_ptp(priv);

    stmmac_clk_disable(priv->device);

	return 0;
}

/**
 *  stmmac_xmit: Tx entry point of the driver
 *  @skb : the socket buffer
 *  @dev : device pointer
 *  Description : this is the tx entry point of the driver.
 *  It programs the chain or the ring and supports oversized frames
 *  and SG feature.
 */
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
static netdev_tx_t stmmac_spe_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	unsigned int txsize = priv->dma_tx_size;
	unsigned int entry;
	int csum_insertion = 0;
	int nfrags = skb_shinfo(skb)->nr_frags;
	struct dma_desc *desc, *first;
	unsigned int nopaged_len = skb_headlen(skb);
	unsigned long flags;

	priv->xstats.enter_xmit++;

	if (unlikely(stmmac_tx_avail(priv) < nfrags + 1)) {
		priv->xstats.tx_desc_full++;
		return NETDEV_TX_BUSY;
	}

	spin_lock_irqsave(&priv->tx_lock, flags);

	if (priv->tx_path_in_lpi_mode)
		stmmac_disable_eee_mode(priv);

	entry = priv->cur_tx % txsize;

	csum_insertion = (skb->ip_summed == CHECKSUM_PARTIAL);

	if (priv->extend_desc)
		desc = (struct dma_desc *)(priv->dma_etx + entry);
	else
		desc = priv->dma_tx + entry;

	first = desc;

	priv->tx_skbuff[entry] = skb;
	
	if(skb->spe_own){
		desc->des2 = spe_hook.get_skb_dma(skb);
	}
	else{
		desc->des2 = dma_map_single(priv->device, skb->data,
				nopaged_len, DMA_TO_DEVICE);
	}
	
	priv->tx_skbuff_dma[entry] = desc->des2;
	priv->hw->desc->prepare_tx_desc(desc, 1, nopaged_len,
					csum_insertion, priv->mode);

	priv->hw->desc->close_tx_desc(desc, 1);
	/* According to the coalesce parameter the IC bit for the latest
	 * segment could be reset and the timer re-started to invoke the
	 * stmmac_tx function. This approach takes care about the fragments.
	 */
	priv->tx_count_frames += nfrags + 1;
	if (priv->tx_coal_frames > priv->tx_count_frames) {
		priv->hw->desc->clear_tx_ic(desc);
		priv->xstats.tx_reset_ic_bit++;
		GMAC_DBG(("\t[entry %d]: tx_count_frames %d\n", entry,
		       priv->tx_count_frames));
		mod_timer(&priv->txtimer,
			  STMMAC_COAL_TIMER(priv->tx_coal_timer));
	} else
		priv->tx_count_frames = 0;

	/* To avoid raise condition */
	priv->hw->desc->set_tx_owner(first);
	wmb();

	priv->cur_tx++;

	dev->stats.tx_bytes += skb->len;

	if (unlikely((skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP) &&
		     priv->hwts_tx_en)) {
		/* declare that device is doing timestamping */
		skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
		priv->hw->desc->enable_tx_timestamp(first);
	}

	if (!priv->hwts_tx_en)
		skb_tx_timestamp(skb);

	priv->hw->dma->enable_dma_transmission(priv->ioaddr);

	spin_unlock_irqrestore(&priv->tx_lock, flags);

	return NETDEV_TX_OK;
}
#endif

static netdev_tx_t stmmac_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	unsigned int txsize = priv->dma_tx_size;
	unsigned int entry;
	int i, csum_insertion = 0, is_jumbo = 0;
	int nfrags = skb_shinfo(skb)->nr_frags;
	struct dma_desc *desc, *first;
	unsigned int nopaged_len = skb_headlen(skb);
	unsigned long flags;

	priv->xstats.enter_xmit++;

	if (unlikely(stmmac_tx_avail(priv) < nfrags + 1)) {
		if (!netif_queue_stopped(dev)) {
			netif_stop_queue(dev);
			/* This is a hard error, log it. */
			GMAC_DBG(("%s: Tx Ring full when queue awake\n", __func__));
		}
		priv->xstats.tx_desc_full++;
		return NETDEV_TX_BUSY;
	}

	spin_lock_irqsave(&priv->tx_lock, flags);

#if (FEATURE_ON == MBB_FEATURE_ETH)
    mbb_mac_clone_tx_save(skb);
#endif
	if (priv->tx_path_in_lpi_mode)
		stmmac_disable_eee_mode(priv);

	entry = priv->cur_tx % txsize;

	if (gmac_msg_level & GMAC_LEVEL_TX_DBG) {
		if ((skb->len > ETH_FRAME_LEN) || nfrags)
			GMAC_DBG(("%s: [entry %d]: skb addr %p len: %d nopagedlen: %d\n"
				 "\tn_frags: %d - ip_summed: %d - %s gso\n"
				 "\ttx_count_frames %d\n", __func__, entry,
				 skb, skb->len, nopaged_len, nfrags, skb->ip_summed,
				 !skb_is_gso(skb) ? "isn't" : "is",
				 priv->tx_count_frames));
	}

	csum_insertion = (skb->ip_summed == CHECKSUM_PARTIAL);

	if (priv->extend_desc)
		desc = (struct dma_desc *)(priv->dma_etx + entry);
	else
		desc = priv->dma_tx + entry;

	first = desc;

	if (gmac_msg_level & GMAC_LEVEL_TX_DBG) {
		if ((nfrags > 0) || (skb->len > ETH_FRAME_LEN))
			GMAC_DBG(("\tskb len: %d, nopaged_len: %d,\n"
				 "\t\tn_frags: %d, ip_summed: %d\n",
				 skb->len, nopaged_len, nfrags, skb->ip_summed));
	}
	priv->tx_skbuff[entry] = skb;

	/* To program the descriptors according to the size of the frame */
	if (priv->mode == STMMAC_RING_MODE) {
		is_jumbo = priv->hw->ring->is_jumbo_frm(skb->len,
							priv->plat->enh_desc);
		if (unlikely(is_jumbo))
			entry = priv->hw->ring->jumbo_frm(priv, skb,
							  csum_insertion);
	} else {
		is_jumbo = priv->hw->chain->is_jumbo_frm(skb->len,
							 priv->plat->enh_desc);
		if (unlikely(is_jumbo))
			entry = priv->hw->chain->jumbo_frm(priv, skb,
							   csum_insertion);
	}
	if (likely(!is_jumbo)) {
		desc->des2 = dma_map_single(priv->device, skb->data,
					    nopaged_len, DMA_TO_DEVICE);

		priv->tx_skbuff_dma[entry] = desc->des2;
		priv->hw->desc->prepare_tx_desc(desc, 1, nopaged_len,
						csum_insertion, priv->mode);
	} else
		desc = first;

	for (i = 0; i < nfrags; i++) {
		const skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
		int len = skb_frag_size(frag);

		entry = (++priv->cur_tx) % txsize;
		if (priv->extend_desc)
			desc = (struct dma_desc *)(priv->dma_etx + entry);
		else
			desc = priv->dma_tx + entry;

		GMAC_DBG(("\t[entry %d] segment len: %d\n", entry, len));
		desc->des2 = skb_frag_dma_map(priv->device, frag, 0, len,
					      DMA_TO_DEVICE);
		priv->tx_skbuff_dma[entry] = desc->des2;
		priv->tx_skbuff[entry] = NULL;
		priv->hw->desc->prepare_tx_desc(desc, 0, len, csum_insertion,
						priv->mode);
		wmb();
		priv->hw->desc->set_tx_owner(desc);
		wmb();
	}

	priv->hw->desc->close_tx_desc(desc, 1);
	/* According to the coalesce parameter the IC bit for the latest
	 * segment could be reset and the timer re-started to invoke the
	 * stmmac_tx function. This approach takes care about the fragments.
	 */
	priv->tx_count_frames += nfrags + 1;
	if (priv->tx_coal_frames > priv->tx_count_frames) {
		priv->hw->desc->clear_tx_ic(desc);
		priv->xstats.tx_reset_ic_bit++;
		GMAC_DBG(("\t[entry %d]: tx_count_frames %d\n", entry,
		       priv->tx_count_frames));
		mod_timer(&priv->txtimer,
			  STMMAC_COAL_TIMER(priv->tx_coal_timer));
	} else
		priv->tx_count_frames = 0;

	/* To avoid raise condition */
	priv->hw->desc->set_tx_owner(first);
	wmb();

	priv->cur_tx++;

	if(gmac_msg_level & GMAC_LEVEL_TX_DBG){
		GMAC_TX_DBG(("%s: curr %d dirty=%d entry=%d, first=%p, nfrags=%d\n",
			__func__, (priv->cur_tx % txsize),
			(priv->dirty_tx % txsize), entry, first, nfrags));

		if (priv->extend_desc)
			stmmac_display_ring((void *)priv->dma_etx, txsize, 1);
		else
			stmmac_display_ring((void *)priv->dma_tx, txsize, 0);

		GMAC_INFO((">>> frame to be transmitted: "));
		print_pkt(skb->data, skb->len);

	}

	if (unlikely(stmmac_tx_avail(priv) <= (MAX_SKB_FRAGS + 1))) {
		GMAC_DBG(("%s: stop transmitted packets\n", __func__));
		netif_stop_queue(dev);
		priv->xstats.xmit_td_full++;
		priv->xstats.tx_avail = stmmac_tx_avail(priv);
	}

	dev->stats.tx_bytes += skb->len;

	if (unlikely((skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP) &&
		     priv->hwts_tx_en)) {
		/* declare that device is doing timestamping */
		skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
		priv->hw->desc->enable_tx_timestamp(first);
	}

	if (!priv->hwts_tx_en)
		skb_tx_timestamp(skb);

	priv->hw->dma->enable_dma_transmission(priv->ioaddr);

	spin_unlock_irqrestore(&priv->tx_lock, flags);

	return NETDEV_TX_OK;
}

/**
 * stmmac_rx_refill: refill used skb preallocated buffers
 * @priv: driver private structure
 * Description : this is to reallocate the skb for the reception process
 * that is based on zero-copy.
 */
static inline void stmmac_rx_refill(struct stmmac_priv *priv)
{
	unsigned int rxsize = priv->dma_rx_size;
	int bfsize = priv->dma_buf_sz;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
    dma_addr_t dma;
#endif
    unsigned long flags;
    spin_lock_irqsave(&priv->rx_lock, flags);
	for (; priv->cur_rx - priv->dirty_rx > 0; priv->dirty_rx++) {
		unsigned int entry = priv->dirty_rx % rxsize;
		struct dma_desc *p;

		if (priv->extend_desc)
			p = (struct dma_desc *)(priv->dma_erx + entry);
		else
			p = priv->dma_rx + entry;

		if (likely(priv->rx_skbuff[entry] == NULL)) {
			struct sk_buff *skb;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
            if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
                skb = skb_dequeue(&priv->free_q);
                if (unlikely(skb == NULL))
            				break;
                priv->rx_skbuff[entry] = skb;
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
                dma = dma_map_single(priv->device, skb->data, 32, DMA_FROM_DEVICE);
#endif
                dma = spe_hook.get_skb_dma(skb);
                p->des2 = dma;
            }else
#endif
			{
			skb = netdev_alloc_skb_ip_align(priv->dev, bfsize);

			if (unlikely(skb == NULL))
				break;

			priv->rx_skbuff[entry] = skb;
			priv->rx_skbuff_dma[entry] =
			    dma_map_single(priv->device, skb->data, bfsize,
					   DMA_FROM_DEVICE);

			p->des2 = priv->rx_skbuff_dma[entry];
			}

			priv->hw->ring->refill_desc3(priv, p);

			RX_DBG(KERN_INFO "\trefill entry #%d\n", entry);
		}
		wmb();
		priv->hw->desc->set_rx_owner(p);
		priv->hw->desc->dis_rx_intr_comp(p,rx_irq_flag);
		wmb();
	}
    
    spin_unlock_irqrestore(&priv->rx_lock, flags);
}

/**
 * stmmac_rx_refill: refill used skb preallocated buffers
 * @priv: driver private structure
 * @limit: napi bugget.
 * Description :  this the function called by the napi poll method.
 * It gets all the frames inside the ring.
 */
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
static int stmmac_spe_rx(struct stmmac_priv *priv, int limit)
{
	unsigned int rxsize = priv->dma_rx_size;
	unsigned int entry;
	unsigned int next_entry;
	unsigned int count = 0;
	int coe = priv->plat->rx_coe;
    unsigned long flags;
	struct sk_buff *skb;
	int frame_len;
    dma_addr_t dma;
    int ret;
	int push = 0;
	int status;
	struct dma_desc *p;

#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
        u16 vlan_tag = 0;
        struct net_device *lan_wan_dev = NULL;
    
        lan_wan_dev = dev_get_by_name(&init_net, LAN_WAN_PORT_NAME);
#endif

	priv->xstats.rx_enter++;

    spin_lock_irqsave(&priv->rx_lock,flags);
    entry = priv->cur_rx % rxsize;
	
	while (count < limit) {

		if (priv->extend_desc)
			p = (struct dma_desc *)(priv->dma_erx + entry);
		else
			p = priv->dma_rx + entry;

		if (priv->hw->desc->get_rx_owner(p))
			break;

		count++;

		next_entry = (++priv->cur_rx) % rxsize;
		if (priv->extend_desc)
			prefetch(priv->dma_erx + next_entry);
		else
			prefetch(priv->dma_rx + next_entry);

		/* read the status of the incoming frame */
		status = priv->hw->desc->rx_status(&priv->dev->stats,
						   &priv->xstats, p);
		if ((priv->extend_desc) && (priv->hw->desc->rx_extended_status))
			priv->hw->desc->rx_extended_status(&priv->dev->stats,
							   &priv->xstats,
							   priv->dma_erx +
							   entry);
		if (unlikely(status == discard_frame)) {
			priv->dev->stats.rx_errors++;
			if (priv->hwts_rx_en && !priv->extend_desc) {
				/* DESC2 & DESC3 will be overwitten by device
				 * with timestamp value, hence reinitialize
				 * them in stmmac_rx_refill() function so that
				 * device can reuse it.
				 */
                skb = priv->rx_skbuff[entry];
                priv->rx_skbuff[entry] = NULL;
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
                ret = vlan_get_tag(skb, &vlan_tag);
                if ((0 == ret && LAN_WAN_VLAN_TAG == vlan_tag)
                    && (NULL != lan_wan_dev && !(lan_wan_dev->priv_flags & IFF_BRIDGE_PORT))
                    ) {
                    stmmac_finish_td(priv->portno_wan, skb, 0);
                }else{
                    stmmac_finish_td(priv->portno, skb, 0);
                }
#else
                stmmac_finish_td(priv->portno, skb, 0);
#endif
			}
		} else {
			frame_len = priv->hw->desc->get_rx_frame_len(p, coe);

			/* ACS is set; GMAC core strips PAD/FCS for IEEE 802.3
			 * Type frames (LLC/LLC-SNAP)
			 */
			if (unlikely(status != llc_snap))
				frame_len -= ETH_FCS_LEN;

			skb = priv->rx_skbuff[entry];
			if (unlikely(!skb)) {
				GMAC_ERR(("%s: Inconsistent Rx descriptor chain\n",
				       priv->dev->name));
				priv->dev->stats.rx_dropped++;
				break;
			}
			prefetch(skb->data - NET_IP_ALIGN);
			priv->rx_skbuff[entry] = NULL;

			stmmac_get_rx_hwtstamp(priv, entry, skb);

#if (FEATURE_ON == MBB_FEATURE_ETH)
            if(DMA_BUFFER_SIZE >= frame_len)
            {
                skb_put(skb, frame_len);
            }
            else
            {
                skb_put(skb, DMA_BUFFER_SIZE);
            }
            mbb_mac_clone_rx_restore(skb);
            mbb_check_net_upgrade(skb);
#else
            skb_put(skb, frame_len);
#endif

			if(gmac_msg_level & GMAC_LEVEL_RX_DBG) {
				GMAC_INFO(("%s: frame received (%dbytes)",__func__, frame_len));
				print_pkt(skb->data, frame_len);
			}
			
			if (priv->extend_desc)
				p = (struct dma_desc *)(priv->dma_erx + next_entry);
			else
				p = priv->dma_rx + next_entry;
		
			if (priv->hw->desc->get_rx_owner(p) || (count == limit))
				push = 1;
			/**/
#if (FEATURE_ON == MBB_FEATURE_ETH)
               if(MTU_AND_HEAD < frame_len)
               {
                    struct sk_buff* large_skb = NULL;
                    dma_unmap_single(priv->device, priv->rx_skbuff_dma[entry], priv->dma_buf_sz, DMA_FROM_DEVICE);
                    if(frame_len <= DMA_BUFFER_SIZE)
                    {
                        large_skb = skb_copy(skb, GFP_ATOMIC);
                    }

                    dma_map_single(priv->device, skb->data, frame_len, DMA_FROM_DEVICE);
                    stmmac_reset_rx_skb_spe(skb);
                    skb_queue_tail(&priv->free_q, skb);
                    spin_unlock_irqrestore(&priv->rx_lock, flags);
                    if (large_skb)
                    {
                        large_skb->protocol = eth_type_trans(large_skb,priv->dev);
                        napi_gro_receive(&priv->napi, large_skb);
                    }
                    stmmac_rx_refill(priv);
                    spin_lock_irqsave(&priv->rx_lock, flags);
            }
            else
            {
#endif
			dma = spe_hook.get_skb_dma(skb);
            
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
            dma_unmap_single(priv->device,priv->rx_skbuff_dma[entry],32, DMA_FROM_DEVICE);
            ret = vlan_get_tag(skb, &vlan_tag);
            if ((0 == ret && LAN_WAN_VLAN_TAG == vlan_tag)
                && (NULL != lan_wan_dev && !(lan_wan_dev->priv_flags & IFF_BRIDGE_PORT))
                ){
                ret = spe_hook.td_config(priv->portno_wan, skb, dma, spe_l3_bottom, push);
            }else{
                ret = spe_hook.td_config(priv->portno, skb, dma, spe_l3_bottom, push);
            }
#else
            ret = spe_hook.td_config(priv->portno, skb, dma, spe_l3_bottom, push);
#endif

			if (SPE_ERR_TDFULL == ret || SPE_ERR_PORT_DISABLED == ret) {
				GMAC_DBG(("%s: td full\n",__func__));
				priv->xstats.rx_collision++;
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
                ret = vlan_get_tag(skb, &vlan_tag);
                if ((0 == ret && LAN_WAN_VLAN_TAG == vlan_tag)
                    && (NULL != lan_wan_dev && !(lan_wan_dev->priv_flags & IFF_BRIDGE_PORT))
                    )
                {
                    stmmac_finish_td(priv->portno_wan, skb, 0);
                }else{
                    stmmac_finish_td(priv->portno, skb, 0);
                }
#else
                stmmac_finish_td(priv->portno, skb, 0);
#endif
			}
#if (FEATURE_ON == MBB_FEATURE_ETH)
            }
#endif
            gmac_status->rx_skb_count++;

			priv->dev->stats.rx_packets++;
			priv->dev->stats.rx_bytes += frame_len;
		}
		entry = next_entry;
		
	}
    spin_unlock_irqrestore(&priv->rx_lock,flags);

	priv->xstats.rx_pkt_n += count;

	return count;
}
#endif

static int stmmac_rx(struct stmmac_priv *priv, int limit)
{
	unsigned int rxsize = priv->dma_rx_size;
	unsigned int entry;
	unsigned int next_entry;
	unsigned int count = 0;
	int coe = priv->plat->rx_coe;
	struct sk_buff *last_skb = NULL;

#ifdef STMMAC_RX_DEBUG
	if (netif_msg_hw(priv)) {
		GMAC_DBG((">>> stmmac_rx: descriptor ring:\n"));
		if (priv->extend_desc)
			stmmac_display_ring((void *)priv->dma_erx, rxsize, 1);
		else
			stmmac_display_ring((void *)priv->dma_rx, rxsize, 0);
	}
#endif
    entry = priv->cur_rx % rxsize;

	while (count < limit) {
		int status;
		struct dma_desc *p;

		if (priv->extend_desc)
			p = (struct dma_desc *)(priv->dma_erx + entry);
		else
			p = priv->dma_rx + entry;

		if (priv->hw->desc->get_rx_owner(p))
			break;

		count++;

		next_entry = (++priv->cur_rx) % rxsize;
		if (priv->extend_desc)
			prefetch(priv->dma_erx + next_entry);
		else
			prefetch(priv->dma_rx + next_entry);

		/* read the status of the incoming frame */
		status = priv->hw->desc->rx_status(&priv->dev->stats,
						   &priv->xstats, p);
		if ((priv->extend_desc) && (priv->hw->desc->rx_extended_status))
			priv->hw->desc->rx_extended_status(&priv->dev->stats,
							   &priv->xstats,
							   priv->dma_erx +
							   entry);
		if (unlikely(status == discard_frame)) {
			priv->dev->stats.rx_errors++;
			if (priv->hwts_rx_en && !priv->extend_desc) {
				/* DESC2 & DESC3 will be overwitten by device
				 * with timestamp value, hence reinitialize
				 * them in stmmac_rx_refill() function so that
				 * device can reuse it.
				 */
				priv->rx_skbuff[entry] = NULL;
				dma_unmap_single(priv->device,
						 priv->rx_skbuff_dma[entry],
						 priv->dma_buf_sz,
						 DMA_FROM_DEVICE);
			}
		} else {
			struct sk_buff *skb;
			int frame_len;

			frame_len = priv->hw->desc->get_rx_frame_len(p, coe);

			/* ACS is set; GMAC core strips PAD/FCS for IEEE 802.3
			 * Type frames (LLC/LLC-SNAP)
			 */
			if (unlikely(status != llc_snap))
				frame_len -= ETH_FCS_LEN;
#ifdef STMMAC_RX_DEBUG
			if (frame_len > ETH_FRAME_LEN)
				GMAC_DBG(("\tRX frame size %d, COE status: %d\n",
					 frame_len, status));

			if (netif_msg_hw(priv))
				GMAC_DBG(("\tdesc: %p [entry %d] buff=0x%x\n",
					 p, entry, p->des2));
#endif
			skb = priv->rx_skbuff[entry];
			if (unlikely(!skb)) {
				GMAC_ERR(("%s: Inconsistent Rx descriptor chain\n",
				       priv->dev->name));
				priv->dev->stats.rx_dropped++;
				break;
			}
			prefetch(skb->data - NET_IP_ALIGN);
			priv->rx_skbuff[entry] = NULL;

			stmmac_get_rx_hwtstamp(priv, entry, skb);

#if (FEATURE_ON == MBB_FEATURE_ETH)
            if(frame_len <= DMA_BUFFER_SIZE)
            {
                skb_put(skb, frame_len);
            }
            else
            {
                skb_put(skb, DMA_BUFFER_SIZE);
            }
#else
            skb_put(skb, frame_len);
#endif

			dma_unmap_single(priv->device,
					 priv->rx_skbuff_dma[entry],
					 priv->dma_buf_sz, DMA_FROM_DEVICE);


			if(gmac_msg_level & GMAC_LEVEL_RX_DBG) {
				GMAC_INFO(("%s: frame received (%dbytes)",__func__, frame_len));
				print_pkt(skb->data, frame_len);
			}
#if (FEATURE_ON == MBB_FEATURE_ETH)
            mbb_mac_clone_rx_restore(skb);
            mbb_check_net_upgrade(skb);
#endif

			skb->protocol = eth_type_trans(skb, priv->dev);

			if(last_skb){
            	if (unlikely(!coe)) {
                    /* No RX COE for old mac10/100 devices */
                    skb_checksum_none_assert(last_skb);
                } else {
                    last_skb->ip_summed = CHECKSUM_UNNECESSARY;
             	}

				napi_gro_receive(&priv->napi, last_skb);

			}
#ifdef CONFIG_BALONG_SKB_MEMBER
			skb->psh = 0;
#endif
            last_skb = skb;
            
            gmac_status->rx_skb_count++;

			priv->dev->stats.rx_packets++;
			priv->dev->stats.rx_bytes += frame_len;
		}
		entry = next_entry;
	}

    if(last_skb){
#ifdef CONFIG_BALONG_SKB_MEMBER
        last_skb->psh = 1;
#endif
        gmac_status->rx_psh_count++;

        if (unlikely(!coe)) {
            /* No RX COE for old mac10/100 devices */
            skb_checksum_none_assert(last_skb);
        } else {
            last_skb->ip_summed = CHECKSUM_UNNECESSARY;
        }

#ifdef CONFIG_GMAC_TEST
		local_receive_skb(count-1, last_skb);
#else
		napi_gro_receive(&priv->napi, last_skb);
#endif
    } 

	/* refill is moved to stmmac_finish_td in spe version */
	stmmac_rx_refill(priv);

	priv->xstats.rx_pkt_n += count;

	return count;
}

/**
 *  stmmac_poll - stmmac poll method (NAPI)
 *  @napi : pointer to the napi structure.
 *  @budget : maximum number of packets that the current CPU can receive from
 *	      all interfaces.
 *  Description :
 *  To look at the incoming frames and clear the tx resources.
 */
static int stmmac_poll(struct napi_struct *napi, int budget)
{
	struct stmmac_priv *priv = container_of(napi, struct stmmac_priv, napi);
	int work_done = 0;

	priv->xstats.napi_poll++;
	stmmac_tx_clean(priv);

#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
    if(g_mmitest_enable)
    {
        return work_done;
    }
#endif

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		work_done = stmmac_spe_rx(priv, budget);
	}
	else
	{
		work_done = stmmac_rx(priv, budget);
	}
#else
	work_done = stmmac_rx(priv, budget);
#endif
	if (work_done < budget) {
		napi_complete(napi);
		stmmac_enable_dma_irq(priv);
	}
	return work_done;
}

/**
 *  stmmac_tx_timeout
 *  @dev : Pointer to net device structure
 *  Description: this function is called when a packet transmission fails to
 *   complete within a reasonable time. The driver will mark the error in the
 *   netdev structure and arrange for the device to be reset to a sane state
 *   in order to transmit a new packet.
 */
static void stmmac_tx_timeout(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);

	/* Clear Tx resources and restart transmitting again */
	stmmac_tx_err(priv);
}

/* Configuration changes (passed on by ifconfig) */
static int stmmac_config(struct net_device *dev, struct ifmap *map)
{
	if (dev->flags & IFF_UP)	/* can't act on a running interface */
		return -EBUSY;

	/* Don't allow changing the I/O address */
	if (map->base_addr != dev->base_addr) {
		GMAC_WARNING(("%s: can't change I/O address\n", dev->name));
		return -EOPNOTSUPP;
	}

	/* Don't allow changing the IRQ */
	if (map->irq != dev->irq) {
		GMAC_WARNING(("%s: not change IRQ number %d\n", dev->name, dev->irq));
		return -EOPNOTSUPP;
	}

	return 0;
}

/**
 *  stmmac_set_rx_mode - entry point for multicast addressing
 *  @dev : pointer to the device structure
 *  Description:
 *  This function is a driver entry point which gets called by the kernel
 *  whenever multicast addresses must be enabled/disabled.
 *  Return value:
 *  void.
 */
static void stmmac_set_rx_mode(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);

	spin_lock(&priv->lock);
	priv->hw->mac->set_filter(dev, priv->synopsys_id);
	spin_unlock(&priv->lock);
}

/**
 *  stmmac_change_mtu - entry point to change MTU size for the device.
 *  @dev : device pointer.
 *  @new_mtu : the new MTU size for the device.
 *  Description: the Maximum Transfer Unit (MTU) is used by the network layer
 *  to drive packet transmission. Ethernet has an MTU of 1500 octets
 *  (ETH_DATA_LEN). This value can be changed with ifconfig.
 *  Return value:
 *  0 on success and an appropriate (-)ve integer as defined in errno.h
 *  file on failure.
 */
static int stmmac_change_mtu(struct net_device *dev, int new_mtu)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	int max_mtu;

	if (netif_running(dev)) {
		GMAC_ERR(("%s: must be stopped to change its MTU\n", dev->name));
		return -EBUSY;
	}

	if (priv->plat->enh_desc)
		max_mtu = JUMBO_LEN;
	else
		max_mtu = SKB_MAX_HEAD(NET_SKB_PAD + NET_IP_ALIGN);

	if ((new_mtu < 46) || (new_mtu > max_mtu)) {
		GMAC_ERR(("%s: invalid MTU, max MTU is: %d\n", dev->name, max_mtu));
		return -EINVAL;
	}

	dev->mtu = new_mtu;
	netdev_update_features(dev);

	return 0;
}

static netdev_features_t stmmac_fix_features(struct net_device *dev,
					     netdev_features_t features)
{
	struct stmmac_priv *priv = netdev_priv(dev);

	if (priv->plat->rx_coe == STMMAC_RX_COE_NONE)
		features &= ~NETIF_F_RXCSUM;
	else if (priv->plat->rx_coe == STMMAC_RX_COE_TYPE1)
		features &= ~NETIF_F_IPV6_CSUM;
	if (!priv->plat->tx_coe)
		features &= ~NETIF_F_ALL_CSUM;

	/* Some GMAC devices have a bugged Jumbo frame support that
	 * needs to have the Tx COE disabled for oversized frames
	 * (due to limited buffer sizes). In this case we disable
	 * the TX csum insertionin the TDES and not use SF.
	 */
	if (priv->plat->bugged_jumbo && (dev->mtu > ETH_DATA_LEN))
		features &= ~NETIF_F_ALL_CSUM;

	return features;
}

/**
 *  stmmac_interrupt - main ISR
 *  @irq: interrupt number.
 *  @dev_id: to pass the net device pointer.
 *  Description: this is the main driver interrupt service routine.
 *  It calls the DMA ISR and also the core ISR to manage PMT, MMC, LPI
 *  interrupts.
 */
static irqreturn_t stmmac_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *)dev_id;
	struct stmmac_priv *priv = netdev_priv(dev);

	if (unlikely(!dev)) {
		GMAC_ERR(("%s: invalid dev pointer\n", __func__));
		return IRQ_NONE;
	}

	/* To handle GMAC own interrupts */
	if (priv->plat->has_gmac) {
		int status = priv->hw->mac->host_irq_status((void __iomem *)
							    dev->base_addr,
							    &priv->xstats);
		if (unlikely(status)) {
			/* For LPI we need to save the tx status */
			if (status & CORE_IRQ_TX_PATH_IN_LPI_MODE)
				priv->tx_path_in_lpi_mode = true;
			if (status & CORE_IRQ_TX_PATH_EXIT_LPI_MODE)
				priv->tx_path_in_lpi_mode = false;
		}
	}

	/* To handle DMA interrupts */
	stmmac_dma_interrupt(priv);

	return IRQ_HANDLED;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
/* Polling receive - used by NETCONSOLE and other diagnostic tools
 * to allow network I/O with interrupts disabled.
 */
static void stmmac_poll_controller(struct net_device *dev)
{
	disable_irq(dev->irq);
	stmmac_interrupt(dev->irq, dev);
	enable_irq(dev->irq);
}
#endif

int	stmmac_set_mac_address(struct net_device *dev, void *addr)
{
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	struct stmmac_priv *priv = netdev_priv(dev); 
#endif
	int ret=0;
	
	ret = eth_mac_addr(dev, addr);
	if(ret)
	{
		printk("eth_mac_addr return %d \n",ret);
		return ret;
	}

	priv->hw->mac->set_umac_addr(priv->ioaddr, dev->dev_addr, 0);	
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		spe_hook.port_ioctl(priv->portno,spe_port_ioctl_set_mac,(int)dev->dev_addr);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
        spe_hook.port_ioctl(priv->portno_wan,spe_port_ioctl_set_mac,(int)dev->dev_addr);
#endif
	}
#endif
	return 0;
}

/**
 *  stmmac_ioctl - Entry point for the Ioctl
 *  @dev: Device pointer.
 *  @rq: An IOCTL specefic structure, that can contain a pointer to
 *  a proprietary structure used to pass information to the driver.
 *  @cmd: IOCTL command
 *  Description:
 *  Currently it supports the phy_mii_ioctl(...) and HW time stamping.
 */
#define SIOCLINKENABLE 	0x89FA   // 禁用/启用eth端口
#define SIOCLINKSTATE   0x89F8   //系统获取多网口状态
static int stmmac_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	int ret = -EOPNOTSUPP;

	if (!netif_running(dev))
		return -EINVAL;

	switch (cmd) {
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
    case SIOCLINKENABLE:
    case SIOCLINKSTATE:		
        if (!priv->phydev)
			return -EINVAL;
		ret = phy_mii_ioctl(priv->phydev, rq, cmd);
		break;
	case SIOCSHWTSTAMP:
		ret = stmmac_hwtstamp_ioctl(dev, rq);
		break;
	default:
		break;
	}

	return ret;
}

#ifdef CONFIG_STMMAC_DEBUG_FS
static struct dentry *stmmac_fs_dir;
static struct dentry *stmmac_rings_status;
static struct dentry *stmmac_dma_cap;

static void sysfs_display_ring(void *head, int size, int extend_desc,
			       struct seq_file *seq)
{
	int i;
	struct dma_extended_desc *ep = (struct dma_extended_desc *)head;
	struct dma_desc *p = (struct dma_desc *)head;

	for (i = 0; i < size; i++) {
		u64 x;
		if (extend_desc) {
			x = *(u64 *) ep;
			seq_printf(seq, "%d [0x%x]: 0x%x 0x%x 0x%x 0x%x\n",
				   i, (unsigned int)virt_to_phys(ep),
				   (unsigned int)x, (unsigned int)(x >> 32),
				   ep->basic.des2, ep->basic.des3);
			ep++;
		} else {
			x = *(u64 *) p;
			seq_printf(seq, "%d [0x%x]: 0x%x 0x%x 0x%x 0x%x\n",
				   i, (unsigned int)virt_to_phys(ep),
				   (unsigned int)x, (unsigned int)(x >> 32),
				   p->des2, p->des3);
			p++;
		}
		seq_printf(seq, "\n");
	}
}

static int stmmac_sysfs_ring_read(struct seq_file *seq, void *v)
{
	struct net_device *dev = seq->private;
	struct stmmac_priv *priv = netdev_priv(dev);
	unsigned int txsize = priv->dma_tx_size;
	unsigned int rxsize = priv->dma_rx_size;

	if (priv->extend_desc) {
		seq_printf(seq, "Extended RX descriptor ring:\n");
		sysfs_display_ring((void *)priv->dma_erx, rxsize, 1, seq);
		seq_printf(seq, "Extended TX descriptor ring:\n");
		sysfs_display_ring((void *)priv->dma_etx, txsize, 1, seq);
	} else {
		seq_printf(seq, "RX descriptor ring:\n");
		sysfs_display_ring((void *)priv->dma_rx, rxsize, 0, seq);
		seq_printf(seq, "TX descriptor ring:\n");
		sysfs_display_ring((void *)priv->dma_tx, txsize, 0, seq);
	}

	return 0;
}

static int stmmac_sysfs_ring_open(struct inode *inode, struct file *file)
{
	return single_open(file, stmmac_sysfs_ring_read, inode->i_private);
}

static const struct file_operations stmmac_rings_status_fops = {
	.owner = THIS_MODULE,
	.open = stmmac_sysfs_ring_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int stmmac_sysfs_dma_cap_read(struct seq_file *seq, void *v)
{
	struct net_device *dev = seq->private;
	struct stmmac_priv *priv = netdev_priv(dev);

	if (!priv->hw_cap_support) {
		seq_printf(seq, "DMA HW features not supported\n");
		return 0;
	}

	seq_printf(seq, "==============================\n");
	seq_printf(seq, "\tDMA HW features\n");
	seq_printf(seq, "==============================\n");

	seq_printf(seq, "\t10/100 Mbps %s\n",
		   (priv->dma_cap.mbps_10_100) ? "Y" : "N");
	seq_printf(seq, "\t1000 Mbps %s\n",
		   (priv->dma_cap.mbps_1000) ? "Y" : "N");
	seq_printf(seq, "\tHalf duple %s\n",
		   (priv->dma_cap.half_duplex) ? "Y" : "N");
	seq_printf(seq, "\tHash Filter: %s\n",
		   (priv->dma_cap.hash_filter) ? "Y" : "N");
	seq_printf(seq, "\tMultiple MAC address registers: %s\n",
		   (priv->dma_cap.multi_addr) ? "Y" : "N");
	seq_printf(seq, "\tPCS (TBI/SGMII/RTBI PHY interfatces): %s\n",
		   (priv->dma_cap.pcs) ? "Y" : "N");
	seq_printf(seq, "\tSMA (MDIO) Interface: %s\n",
		   (priv->dma_cap.sma_mdio) ? "Y" : "N");
	seq_printf(seq, "\tPMT Remote wake up: %s\n",
		   (priv->dma_cap.pmt_remote_wake_up) ? "Y" : "N");
	seq_printf(seq, "\tPMT Magic Frame: %s\n",
		   (priv->dma_cap.pmt_magic_frame) ? "Y" : "N");
	seq_printf(seq, "\tRMON module: %s\n",
		   (priv->dma_cap.rmon) ? "Y" : "N");
	seq_printf(seq, "\tIEEE 1588-2002 Time Stamp: %s\n",
		   (priv->dma_cap.time_stamp) ? "Y" : "N");
	seq_printf(seq, "\tIEEE 1588-2008 Advanced Time Stamp:%s\n",
		   (priv->dma_cap.atime_stamp) ? "Y" : "N");
	seq_printf(seq, "\t802.3az - Energy-Efficient Ethernet (EEE) %s\n",
		   (priv->dma_cap.eee) ? "Y" : "N");
	seq_printf(seq, "\tAV features: %s\n", (priv->dma_cap.av) ? "Y" : "N");
	seq_printf(seq, "\tChecksum Offload in TX: %s\n",
		   (priv->dma_cap.tx_coe) ? "Y" : "N");
	seq_printf(seq, "\tIP Checksum Offload (type1) in RX: %s\n",
		   (priv->dma_cap.rx_coe_type1) ? "Y" : "N");
	seq_printf(seq, "\tIP Checksum Offload (type2) in RX: %s\n",
		   (priv->dma_cap.rx_coe_type2) ? "Y" : "N");
	seq_printf(seq, "\tRXFIFO > 2048bytes: %s\n",
		   (priv->dma_cap.rxfifo_over_2048) ? "Y" : "N");
	seq_printf(seq, "\tNumber of Additional RX channel: %d\n",
		   priv->dma_cap.number_rx_channel);
	seq_printf(seq, "\tNumber of Additional TX channel: %d\n",
		   priv->dma_cap.number_tx_channel);
	seq_printf(seq, "\tEnhanced descriptors: %s\n",
		   (priv->dma_cap.enh_desc) ? "Y" : "N");

	return 0;
}

static int stmmac_sysfs_dma_cap_open(struct inode *inode, struct file *file)
{
	return single_open(file, stmmac_sysfs_dma_cap_read, inode->i_private);
}

static const struct file_operations stmmac_dma_cap_fops = {
	.owner = THIS_MODULE,
	.open = stmmac_sysfs_dma_cap_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int stmmac_init_fs(struct net_device *dev)
{
	/* Create debugfs entries */
	stmmac_fs_dir = debugfs_create_dir(STMMAC_RESOURCE_NAME, NULL);

	if (!stmmac_fs_dir || IS_ERR(stmmac_fs_dir)) {
		GMAC_ERR(("ERROR %s, debugfs create directory failed\n",
		       STMMAC_RESOURCE_NAME));

		return -ENOMEM;
	}

	/* Entry to report DMA RX/TX rings */
	stmmac_rings_status = debugfs_create_file("descriptors_status",
						  S_IRUGO, stmmac_fs_dir, dev,
						  &stmmac_rings_status_fops);

	if (!stmmac_rings_status || IS_ERR(stmmac_rings_status)) {
		pr_info("ERROR creating stmmac ring debugfs file\n");
		debugfs_remove(stmmac_fs_dir);

		return -ENOMEM;
	}

	/* Entry to report the DMA HW features */
	stmmac_dma_cap = debugfs_create_file("dma_cap", S_IRUGO, stmmac_fs_dir,
					     dev, &stmmac_dma_cap_fops);

	if (!stmmac_dma_cap || IS_ERR(stmmac_dma_cap)) {
		pr_info("ERROR creating stmmac MMC debugfs file\n");
		debugfs_remove(stmmac_rings_status);
		debugfs_remove(stmmac_fs_dir);

		return -ENOMEM;
	}

	return 0;
}

static void stmmac_exit_fs(void)
{
	debugfs_remove(stmmac_rings_status);
	debugfs_remove(stmmac_dma_cap);
	debugfs_remove(stmmac_fs_dir);
}
#endif /* CONFIG_STMMAC_DEBUG_FS */

static const struct net_device_ops stmmac_netdev_ops = {
	.ndo_open = stmmac_open,
#if (FEATURE_ON == MBB_ETH_STMMAC_TX_THREAD)
    .ndo_start_xmit = stmmac_vendor_post_skb, 
#else
    .ndo_start_xmit = stmmac_xmit,
#endif
	.ndo_stop = stmmac_release,
	.ndo_change_mtu = stmmac_change_mtu,
	.ndo_fix_features = stmmac_fix_features,
	.ndo_set_rx_mode = stmmac_set_rx_mode,
	.ndo_tx_timeout = stmmac_tx_timeout,
	.ndo_do_ioctl = stmmac_ioctl,
	.ndo_set_config = stmmac_config,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller = stmmac_poll_controller,
#endif
	.ndo_set_mac_address = stmmac_set_mac_address,
};

/**
 *  stmmac_hw_init - Init the MAC device
 *  @priv: driver private structure
 *  Description: this function detects which MAC device
 *  (GMAC/MAC10-100) has to attached, checks the HW capability
 *  (if supported) and sets the driver's features (for example
 *  to use the ring or chaine mode or support the normal/enh
 *  descriptor structure).
 */
static int stmmac_hw_init(struct stmmac_priv *priv)
{
	int ret;
	struct mac_device_info *mac;

	/* Identify the MAC HW device */
	if (priv->plat->has_gmac) {
		priv->dev->priv_flags |= IFF_UNICAST_FLT;
		mac = dwmac1000_setup(priv->ioaddr);
	} else {
		mac = dwmac100_setup(priv->ioaddr);
	}
	if (!mac)
		return -ENOMEM;

	priv->hw = mac;

	/* Get and dump the chip ID */
	priv->synopsys_id = stmmac_get_synopsys_id(priv);

	/* To use alternate (extended) or normal descriptor structures */
	stmmac_selec_desc_mode(priv);

	/* To use the chained or ring mode */
	if (chain_mode) {
		priv->hw->chain = &chain_mode_ops;
		GMAC_INFO((" Chain mode enabled\n"));
		priv->mode = STMMAC_CHAIN_MODE;
	} else {
		priv->hw->ring = &ring_mode_ops;
		GMAC_INFO((" Ring mode enabled\n"));
		priv->mode = STMMAC_RING_MODE;
	}

	/* Get the HW capability (new GMAC newer than 3.50a) */
	priv->hw_cap_support = stmmac_get_hw_features(priv);
	if (priv->hw_cap_support) {
		GMAC_INFO((" DMA HW capability register supported"));

		/* We can override some gmac/dma configuration fields: e.g.
		 * enh_desc, tx_coe (e.g. that are passed through the
		 * platform) with the values from the HW capability
		 * register (if supported).
		 */
		//priv->plat->enh_desc = priv->dma_cap.enh_desc;
		priv->plat->pmt = priv->dma_cap.pmt_remote_wake_up;

		priv->plat->tx_coe = priv->dma_cap.tx_coe;

		if (priv->dma_cap.rx_coe_type2)
			priv->plat->rx_coe = STMMAC_RX_COE_TYPE2;
		else if (priv->dma_cap.rx_coe_type1)
			priv->plat->rx_coe = STMMAC_RX_COE_TYPE1;

	} else
		GMAC_INFO((" No HW DMA feature register supported"));

	ret = priv->hw->mac->rx_ipc(priv->ioaddr);
	if (!ret) {
		GMAC_WARNING((" RX IPC Checksum Offload not configured.\n"));
		priv->plat->rx_coe = STMMAC_RX_COE_NONE;
	}

	if (priv->plat->rx_coe)
		GMAC_INFO((" RX Checksum Offload Engine supported (type %d)\n",
			priv->plat->rx_coe));
	if (priv->plat->tx_coe)
		GMAC_INFO((" TX Checksum insertion supported\n"));

	if (priv->plat->pmt) {
		GMAC_INFO((" Wake-Up On Lan supported\n"));
		device_set_wakeup_capable(priv->device, 1);
	}
	return 0;
}
static int get_gmac_addr(const char *str, u8 *dev_addr)
{
	if (str) {
		unsigned	i;

		for (i = 0; i < 6; i++) {
			unsigned char num;

			if ((*str == '.') || (*str == ':'))
				str++;
			num = hex_to_bin(*str++) << 4;
			num |= hex_to_bin(*str++);
			dev_addr [i] = num;
		}
		if (is_valid_ether_addr(dev_addr))
			return 0;
	}
	random_ether_addr(dev_addr);
	return 1;
}

static char *stmmac_mac = {"00:18:82:0C:0D:66"};
module_param(stmmac_mac, charp, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(stmmac_mac, "stmmac ethernet address ");

/**
 * stmmac_dvr_probe
 * @device: device pointer
 * @plat_dat: platform data pointer
 * @addr: iobase memory address
 * Description: this is the main probe function used to
 * call the alloc_etherdev, allocate the priv structure.
 */
struct stmmac_priv *stmmac_dvr_probe(struct device *device,
				     struct plat_stmmacenet_data *plat_dat,
				     void __iomem *addr)
{
	int ret = 0;
	struct net_device *ndev = NULL;
	struct stmmac_priv *priv;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
    int portno;
    struct spe_port_attr stmmac_attr = {0};
#endif
	ndev = alloc_etherdev(sizeof(struct stmmac_priv));
	if (!ndev)
		return NULL;

	SET_NETDEV_DEV(ndev, device);

	priv = netdev_priv(ndev);
	priv->device = device;
	priv->dev = ndev;

	ether_setup(ndev);

	stmmac_set_ethtool_ops(ndev);
	priv->pause = pause;
	priv->plat = plat_dat;
	priv->ioaddr = addr;
	priv->dev->base_addr = (unsigned long)addr;
	ndev->priv_flags |= IFF_LIVE_ADDR_CHANGE;

	/* Verify driver arguments */
	stmmac_verify_args();

	/* Override with kernel parameters if supplied XXX CRS XXX
	 * this needs to have multiple instances
	 */
	if ((phyaddr >= 0) && (phyaddr <= 31))
		priv->plat->phy_addr = phyaddr;

	/* Init MAC and get the capabilities */
	ret = stmmac_hw_init(priv);
	if (ret)
		goto error_free_netdev;
#if (FEATURE_OFF == MBB_FEATURE_ETH)
    /* Network inteface name--add by wangweichao */
	snprintf(ndev->name, sizeof(ndev->name), "%s%%d", "gmac");
#else	
        snprintf(ndev->name, sizeof(ndev->name), "%s", NET_DEVICE_NAME);
#endif

#if (FEATURE_ON == MBB_FEATURE_ETH)
    if (0 != mbb_get_eth_macAddr(priv->dev->dev_addr))
    {
#endif

    if (get_gmac_addr(stmmac_mac, priv->dev->dev_addr)) {
            GMAC_INFO(("%s:using random ethernet address\n", __func__));
    }
#if (FEATURE_ON == MBB_FEATURE_ETH)
    }
#endif
	ndev->netdev_ops = &stmmac_netdev_ops;
	ndev->flags |= IFF_ALLMULTI;

	//ndev->hw_features = NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM |
	//		    NETIF_F_RXCSUM;
	ndev->features |= ndev->hw_features | NETIF_F_HIGHDMA;
	ndev->watchdog_timeo = msecs_to_jiffies(watchdog);
#ifdef STMMAC_VLAN_TAG_USED
	/* Both mac100 and gmac support receive VLAN tag detection */
	ndev->features |= NETIF_F_HW_VLAN_CTAG_RX;
#endif
	priv->msg_enable = netif_msg_init(debug, default_msg_level);

	if (flow_ctrl)
		priv->flow_ctrl = FLOW_AUTO;	/* RX/TX pause on */

	/* Rx Watchdog is available in the COREs newer than the 3.40.
	 * In some case, for example on bugged HW this feature
	 * has to be disable and this can be done by passing the
	 * riwt_off field from the platform.
	 */
	if ((priv->synopsys_id >= DWMAC_CORE_3_50) && (!priv->plat->riwt_off)) {
		priv->use_riwt = 1;
		pr_info(" Enable RX Mitigation via HW Watchdog Timer\n");
	}

	netif_napi_add(ndev, &priv->napi, stmmac_poll, 64);

	spin_lock_init(&priv->lock);
	spin_lock_init(&priv->tx_lock);

	ret = register_netdev(ndev);
	if (ret) {
		GMAC_ERR(("%s: ERROR %i registering the device\n", __func__, ret));
		goto error_netdev_register;
	}

	/* If a specific clk_csr value is passed from the platform
	 * this means that the CSR Clock Range selection cannot be
	 * changed at run-time and it is fixed. Viceversa the driver'll try to
	 * set the MDC clock dynamically according to the csr actual
	 * clock input.
	 */
	if (priv->plat->clk_csr)
		priv->clk_csr = priv->plat->clk_csr;

	stmmac_check_pcs_mode(priv);

	if (priv->pcs != STMMAC_PCS_RGMII && priv->pcs != STMMAC_PCS_TBI &&
	    priv->pcs != STMMAC_PCS_RTBI) {
		/* MDIO bus Registration */
		ret = stmmac_mdio_register(ndev);
		if (ret < 0) {
			GMAC_DBG(("%s: MDIO bus (id: %d) registration failed",
				 __func__, priv->plat->bus_id));
			goto error_mdio_register;
		}
	}

	stmmac_init_tx_coalesce(priv);

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
    if(spe_hook.is_enable && spe_hook.is_enable()){
#if defined(CONFIG_STMMAC_IPF_SIM)
		uint8_t src_mac[ETHER_ADDR_LEN] = {0x36,0x74,0x9a,0xec,0xa3,0x67}; //mac of gmac pc
		uint8_t dst_mac[ETHER_ADDR_LEN] = {0x0c,0x82,0x18,0x00,0x66,0x0d}; //mac of gmac
        stmmac_attr.enc_type = spe_enc_ipf;
		stmmac_attr.stick_mode = 0;
		memcpy(stmmac_attr.ipf_src_mac.octet, src_mac, RNIC_ETHER_ADDR_LEN);
		memcpy(stmmac_attr.ipf_dst_mac.octet, dst_mac, RNIC_ETHER_ADDR_LEN);
#else
		stmmac_attr.enc_type = spe_enc_none;
#endif
        stmmac_attr.attach_brg = spe_attach_brg_normal;
        stmmac_attr.rd_depth = 256;
        stmmac_attr.td_depth = 256;/*same as rx */
        stmmac_attr.rd_skb_size = 1800;
        stmmac_attr.rd_skb_num = 256;
        stmmac_attr.desc_ops.finish_rd = stmmac_finish_rd;
        stmmac_attr.desc_ops.finish_td = stmmac_finish_td;
        stmmac_attr.net = ndev;
        portno = spe_hook.port_alloc(&stmmac_attr);
        priv->portno = portno;
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
        priv->portno_wan = spe_hook.port_alloc(&stmmac_attr);
#endif
    }
#endif

	return priv;

error_mdio_register:
	unregister_netdev(ndev);
error_netdev_register:
	netif_napi_del(&priv->napi);
error_free_netdev:
	free_netdev(ndev);

	return NULL;
}

/**
 * stmmac_dvr_remove
 * @ndev: net device pointer
 * Description: this function resets the TX/RX processes, disables the MAC RX/TX
 * changes the link status, releases the DMA descriptor rings.
 */
int stmmac_dvr_remove(struct net_device *ndev)
{
	struct stmmac_priv *priv = netdev_priv(ndev);

	pr_info("%s:\n\tremoving driver", __func__);

	priv->hw->dma->stop_rx(priv->ioaddr);
	priv->hw->dma->stop_tx(priv->ioaddr);
	
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		(void)spe_hook.port_free(priv->portno);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
        (void)spe_hook.port_free(priv->portno_wan);
#endif
	}
#endif

	stmmac_set_mac(priv->ioaddr, false);
	if (priv->pcs != STMMAC_PCS_RGMII && priv->pcs != STMMAC_PCS_TBI &&
	    priv->pcs != STMMAC_PCS_RTBI)
		stmmac_mdio_unregister(ndev);
	netif_carrier_off(ndev);
	unregister_netdev(ndev);
	free_netdev(ndev);

	return 0;
}

void show_gmac_xstats(void)
{
    printk("stmmac_xmit tx_desc_full:%d\n", (int)gmac_status->tx_desc_full);
    printk("stmmac_xmit xmit_td_full:%d\n", (int)gmac_status->xmit_td_full);
    printk("stmmac_release net stopped:%d\n", (int)gmac_status->gmac_release);
    printk("stmmac_suspend net stopped:%d\n", (int)gmac_status->gmac_suspend);
    printk("enter xmit:%d\n", (int)gmac_status->enter_xmit);
    printk("tx avail:%d\n", (int)gmac_status->tx_avail);
    printk("rx psh count:%d\n",(int)gmac_status->rx_psh_count);
    printk("rx skb count:%d\n",(int)gmac_status->rx_skb_count);
}

#define SHOWITEM(x)	printk(#x" : %d\n",(int)gmac_status->x)
void show_gmac_xstats_all(void)
{
	SHOWITEM(tx_carrier);
	SHOWITEM(tx_losscarrier);
	SHOWITEM(vlan_tag);
	SHOWITEM(tx_deferred);
	SHOWITEM(tx_vlan);
	SHOWITEM(tx_jabber);
	SHOWITEM(tx_frame_flushed);
	SHOWITEM(tx_payload_error);
	SHOWITEM(tx_ip_header_error);
#ifdef CONFIG_BALONG_GMAC
	/*  gmac test    */
	SHOWITEM(tx_desc_full);
	SHOWITEM(enter_xmit);
	SHOWITEM(xmit_td_full);
	SHOWITEM(gmac_suspend);
	SHOWITEM(gmac_release);
	SHOWITEM(tx_avail);
	/* skb psh flag */
	SHOWITEM(rx_psh_count);
	SHOWITEM(rx_skb_count);
#endif
	/* Receive errors */
	SHOWITEM(rx_desc);
	SHOWITEM(sa_filter_fail);
	SHOWITEM(overflow_error);
	SHOWITEM(ipc_csum_error);
	SHOWITEM(rx_collision);
	SHOWITEM(rx_crc);
	SHOWITEM(dribbling_bit);
	SHOWITEM(rx_length);
	SHOWITEM(rx_mii);
	SHOWITEM(rx_multicast);
	SHOWITEM(rx_gmac_overflow);
	SHOWITEM(rx_watchdog);
	SHOWITEM(da_rx_filter_fail);
	SHOWITEM(sa_rx_filter_fail);
	SHOWITEM(rx_missed_cntr);
	SHOWITEM(rx_overflow_cntr);
	SHOWITEM(rx_vlan);
	SHOWITEM(rx_enter);
	/* Tx/Rx IRQ error info */
	SHOWITEM(tx_undeflow_irq);
	SHOWITEM(tx_process_stopped_irq);
	SHOWITEM(tx_jabber_irq);
	SHOWITEM(rx_overflow_irq);
	SHOWITEM(rx_buf_unav_irq);
	SHOWITEM(rx_process_stopped_irq);
	SHOWITEM(rx_watchdog_irq);
	SHOWITEM(tx_early_irq);
	SHOWITEM(fatal_bus_error_irq);
	/* Tx/Rx IRQ Events */
	SHOWITEM(rx_early_irq);
	SHOWITEM(threshold);
	SHOWITEM(tx_pkt_n);
	SHOWITEM(rx_pkt_n);
	SHOWITEM(normal_irq_n);
	SHOWITEM(rx_normal_irq_n);
	SHOWITEM(napi_poll);
	SHOWITEM(tx_normal_irq_n);
	SHOWITEM(tx_clean);
	SHOWITEM(tx_reset_ic_bit);
	SHOWITEM(irq_receive_pmt_irq_n);
	/* MMC info */
	SHOWITEM(mmc_tx_irq_n);
	SHOWITEM(mmc_rx_irq_n);
	SHOWITEM(mmc_rx_csum_offload_irq_n);
	/* EEE */
	SHOWITEM(irq_tx_path_in_lpi_mode_n);
	SHOWITEM(irq_tx_path_exit_lpi_mode_n);
	SHOWITEM(irq_rx_path_in_lpi_mode_n);
	SHOWITEM(irq_rx_path_exit_lpi_mode_n);
	SHOWITEM(phy_eee_wakeup_error_n);
	/* Extended RDES status */
	SHOWITEM(ip_hdr_err);
	SHOWITEM(ip_payload_err);
	SHOWITEM(ip_csum_bypassed);
	SHOWITEM(ipv4_pkt_rcvd);
	SHOWITEM(ipv6_pkt_rcvd);
	SHOWITEM(rx_msg_type_ext_no_ptp);
	SHOWITEM(rx_msg_type_sync);
	SHOWITEM(rx_msg_type_follow_up);
	SHOWITEM(rx_msg_type_delay_req);
	SHOWITEM(rx_msg_type_delay_resp);
	SHOWITEM(rx_msg_type_pdelay_req);
	SHOWITEM(rx_msg_type_pdelay_resp);
	SHOWITEM(rx_msg_type_pdelay_follow_up);
	SHOWITEM(ptp_frame_type);
	SHOWITEM(ptp_ver);
	SHOWITEM(timestamp_dropped);
	SHOWITEM(av_pkt_rcvd);
	SHOWITEM(av_tagged_pkt_rcvd);
	SHOWITEM(vlan_tag_priority_val);
	SHOWITEM(l3_filter_match);
	SHOWITEM(l4_filter_match);
	SHOWITEM(l3_l4_filter_no_match);
	/* PCS */
	SHOWITEM(irq_pcs_ane_n);
	SHOWITEM(irq_pcs_link_n);
	SHOWITEM(irq_rgmii_n);
	SHOWITEM(pcs_link);
	SHOWITEM(pcs_duplex);
	SHOWITEM(pcs_speed);
}

#ifdef CONFIG_PM
int stmmac_suspend(struct net_device *ndev)
{
	struct stmmac_priv *priv = netdev_priv(ndev);
	unsigned long flags;

	if (!ndev || !netif_running(ndev))
		return 0;

	if (priv->phydev)
		phy_stop(priv->phydev);

	spin_lock_irqsave(&priv->lock, flags);

	netif_device_detach(ndev);
	netif_stop_queue(ndev);
	priv->xstats.gmac_suspend++;

	napi_disable(&priv->napi);

	/* Stop TX/RX DMA */
	priv->hw->dma->stop_tx(priv->ioaddr);
	priv->hw->dma->stop_rx(priv->ioaddr);

	stmmac_clear_descriptors(priv);

	/* Enable Power down mode by programming the PMT regs */
	if (device_may_wakeup(priv->device))
		priv->hw->mac->pmt(priv->ioaddr, priv->wolopts);
	else {
		stmmac_set_mac(priv->ioaddr, false);
	}
	spin_unlock_irqrestore(&priv->lock, flags);
	return 0;
}

int stmmac_resume(struct net_device *ndev)
{
#if (FEATURE_ON == MBB_ETH_PHY_LOWPOWER)
    //stmmac_resume改为在phy上电后调用，见ar8035.c中调用stmmac_restore()
    return 0;
#endif
	struct stmmac_priv *priv = netdev_priv(ndev);
	unsigned long flags;

	if (!netif_running(ndev))
		return 0;

	spin_lock_irqsave(&priv->lock, flags);

	/* Power Down bit, into the PM register, is cleared
	 * automatically as soon as a magic packet or a Wake-up frame
	 * is received. Anyway, it's better to manually clear
	 * this bit because it can generate problems while resuming
	 * from another devices (e.g. serial console).
	 */
	if (device_may_wakeup(priv->device))
		priv->hw->mac->pmt(priv->ioaddr, 0);

	netif_device_attach(ndev);

	/* Enable the MAC and DMA */
	stmmac_set_mac(priv->ioaddr, true);
	priv->hw->dma->start_tx(priv->ioaddr);
	priv->hw->dma->start_rx(priv->ioaddr);

	napi_enable(&priv->napi);

	netif_start_queue(ndev);

	spin_unlock_irqrestore(&priv->lock, flags);

	if (priv->phydev)
		phy_start(priv->phydev);

	return 0;
}

int stmmac_freeze(struct net_device *ndev)
{
	if (!ndev || !netif_running(ndev))
		return 0;

	return stmmac_release(ndev);
}

int stmmac_restore(struct net_device *ndev)
{
	if (!ndev || !netif_running(ndev))
		return 0;

	return stmmac_open(ndev);
}
#endif /* CONFIG_PM */

#ifndef MODULE
static int __init stmmac_cmdline_opt(char *str)
{
	char *opt;

	if (!str || !*str)
		return -EINVAL;
	while ((opt = strsep(&str, ",")) != NULL) {
		if (!strncmp(opt, "debug:", 6)) {
			if (kstrtoint(opt + 6, 0, &debug))
				goto err;
		} else if (!strncmp(opt, "phyaddr:", 8)) {
			if (kstrtoint(opt + 8, 0, &phyaddr))
				goto err;
		} else if (!strncmp(opt, "dma_txsize:", 11)) {
			if (kstrtoint(opt + 11, 0, &dma_txsize))
				goto err;
		} else if (!strncmp(opt, "dma_rxsize:", 11)) {
			if (kstrtoint(opt + 11, 0, &dma_rxsize))
				goto err;
		} else if (!strncmp(opt, "buf_sz:", 7)) {
			if (kstrtoint(opt + 7, 0, &buf_sz))
				goto err;
		} else if (!strncmp(opt, "tc:", 3)) {
			if (kstrtoint(opt + 3, 0, &tc))
				goto err;
		} else if (!strncmp(opt, "watchdog:", 9)) {
			if (kstrtoint(opt + 9, 0, &watchdog))
				goto err;
		} else if (!strncmp(opt, "flow_ctrl:", 10)) {
			if (kstrtoint(opt + 10, 0, &flow_ctrl))
				goto err;
		} else if (!strncmp(opt, "pause:", 6)) {
			if (kstrtoint(opt + 6, 0, &pause))
				goto err;
		} else if (!strncmp(opt, "eee_timer:", 10)) {
			if (kstrtoint(opt + 10, 0, &eee_timer))
				goto err;
		} else if (!strncmp(opt, "chain_mode:", 11)) {
			if (kstrtoint(opt + 11, 0, &chain_mode))
				goto err;
		}
	}
	return 0;

err:
	GMAC_ERR(("%s: ERROR broken module parameter conversion", __func__));
	return -EINVAL;
}

__setup("stmmaceth=", stmmac_cmdline_opt);
#endif /* MODULE */

MODULE_DESCRIPTION("STMMAC 10/100/1000 Ethernet device driver");
MODULE_AUTHOR("Giuseppe Cavallaro <peppe.cavallaro@st.com>");
MODULE_LICENSE("GPL");

#if (FEATURE_ON == MBB_FEATURE_ETH)
extern struct net init_net;
void stmmac_debug()
{
    struct net_device *dev = __dev_get_by_name(&init_net, NET_DEVICE_NAME);
    struct stmmac_priv *priv = netdev_priv(dev);
    printk("PHY INFO:\r\n");
    printk("PHY addr:%d, state:%d, autoneg:%d\n", priv->phydev->addr, priv->phydev->state, priv->phydev->autoneg); 
    printk("PHY link:%d, speed:%d, duplex: %d\n", priv->phydev->link, priv->phydev->speed, priv->phydev->duplex);
    printk("MAC INFO:\r\n");
    printk("ioaddr:     0x%x\n", readl(priv->ioaddr));
    printk("cur_rx:     %d\n",   priv->cur_rx);
    printk("dirty_rx:   %d\n",   priv->dirty_rx);
    printk("cur_tx:     %d\n",   priv->cur_tx);
    printk("dirty_tx:   %d\n",   priv->dirty_tx);
}
EXPORT_SYMBOL(stmmac_debug);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)

typedef int (*check_recv_packet)(struct sk_buff *skb, int frame_len);
void mbb_mmitest_check_recv(struct net_device *dev, check_recv_packet check_fun)
{
    struct stmmac_priv *priv = netdev_priv(dev);
    unsigned int rxsize = priv->dma_rx_size;
    unsigned int entry = priv->cur_rx % rxsize;
    unsigned int next_entry;
    struct dma_desc *p = priv->dma_rx + entry;
    struct dma_desc *p_next;    
    int bfsize = priv->dma_buf_sz;
    int coe = priv->plat->rx_coe;

    stmmac_tx_clean(priv);
       
    while (!priv->hw->desc->get_rx_owner(p)) 
    {
        int status;

        next_entry = (++priv->cur_rx) % rxsize;
        p_next = priv->dma_rx + next_entry;
        prefetch(p_next);

        /* read the status of the incoming frame */
        status = priv->hw->desc->rx_status(&priv->dev->stats, &priv->xstats, p);
        
        if (unlikely(status == discard_frame))
        {
            priv->dev->stats.rx_errors++;
        }    
        else
        {
            struct sk_buff *skb;
            int frame_len;            

            frame_len = priv->hw->desc->get_rx_frame_len(p,coe);
            /* ACS is set; GMAC core strips PAD/FCS for IEEE 802.3
             * Type frames (LLC/LLC-SNAP) */
            if (unlikely(status != llc_snap))
            {   
                frame_len -= ETH_FCS_LEN;
            }

            skb = priv->rx_skbuff[entry];
            if (unlikely(!skb)) 
            {
                GMAC_ERR(("%s: Inconsistent Rx descriptor chain\n",
                    priv->dev->name));
                priv->dev->stats.rx_dropped++;
                break;
            }
            prefetch(skb->data - NET_IP_ALIGN);
            priv->rx_skbuff[entry] = NULL;

            skb_put(skb, frame_len);
            dma_unmap_single(priv->device,
                priv->rx_skbuff_dma[entry],
                priv->dma_buf_sz, DMA_FROM_DEVICE);

            check_fun(skb, frame_len);

            dev_kfree_skb(skb);
        } 

        entry = next_entry;
        p = p_next; /*use prefetched values*/
    }

    stmmac_rx_refill(priv);
}

void mbb_mmitest_xmit(struct sk_buff *skb, struct net_device *dev)
{
    struct stmmac_priv *priv = netdev_priv(dev);

    unsigned int txsize = priv->dma_tx_size;
    unsigned int entry;
    static unsigned int pkt_cnt = 0;
    int i, csum_insertion = 0;
    struct dma_desc *desc, *first;
    unsigned int nopaged_len = skb_headlen(skb);

    spin_lock(&priv->tx_lock);

    entry = priv->cur_tx % txsize;

    csum_insertion = (skb->ip_summed == CHECKSUM_PARTIAL);

    desc = priv->dma_tx + entry;
    first = desc;

    priv->tx_skbuff[entry] = skb;

    if (priv->hw->ring->is_jumbo_frm(skb->len, priv->plat->enh_desc))
    {
        entry = priv->hw->ring->jumbo_frm(priv, skb, csum_insertion);
        desc = priv->dma_tx + entry;
    }
    else 
    {
        desc->des2 = dma_map_single(priv->device, skb->data,
            nopaged_len, DMA_TO_DEVICE);
	priv->hw->desc->prepare_tx_desc(desc, 1, nopaged_len,
						csum_insertion, priv->mode);
    }

    /* Interrupt on completition only for the latest segment */
    pkt_cnt++;
    if(pkt_cnt < 32)
    {
        priv->hw->desc->close_tx_desc(desc, 0);
    }
    else
    {
        pkt_cnt = 0;
        priv->hw->desc->close_tx_desc(desc, 1);
    }

    wmb();

    /* To avoid raise condition */
    priv->hw->desc->set_tx_owner(first);

    priv->cur_tx++;

    priv->hw->dma->enable_dma_transmission(priv->ioaddr);

    spin_unlock(&priv->tx_lock);
}

int mbb_get_spe_wan_portno(void)
{
    struct stmmac_priv *priv = NULL;
    struct net_device *ndev = NULL;
    ndev = dev_get_by_name(&init_net, NET_DEVICE_NAME);
    if (NULL != ndev)
    {
        priv = netdev_priv(ndev);
    }

    if (NULL != priv)
    {
        return priv->portno_wan;
    }
    else
    {
        return -1;
    }
}

#endif
#endif
