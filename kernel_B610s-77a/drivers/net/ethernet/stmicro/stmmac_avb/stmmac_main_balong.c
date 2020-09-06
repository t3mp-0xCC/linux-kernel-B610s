/*******************************************************************************
  This is the driver for the ST MAC 10/100/1000 on-chip Ethernet controllers.
  ST Ethernet IPs are built around a Synopsys IP Core.

	Copyright(C) 2007-2011 STMicroelectronics Ltd

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

  Documentation available at:
	http://www.stlinux.com
  Support available at:
	https://bugzilla.stlinux.com/
*******************************************************************************/

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
#include <linux/bitops.h>
#include <linux/in.h>
#ifdef CONFIG_NEW_STMMAC_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif /* CONFIG_NEW_STMMAC_DEBUG_FS */
#include <linux/net_tstamp.h>
#include <bsp_hardtimer.h>
#include "stmmac_ptp.h"
#include "stmmac.h"
#include "stmmac_debug.h"
#include "dwmac_dma.h"
#include "stmmac_cbs.h"
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
#include <linux/spe/spe_interface.h>
#endif
//#include "gmac_pkt_gen.h"

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
static inline void stmmac_rx_refill(struct stmmac_priv *priv);
static int stmmac_rx(struct stmmac_priv *priv, int limit);

static int
stmmac_finish_rd(int portno, int src_portno, struct sk_buff *skb, dma_addr_t dma, unsigned int flags);
static int
stmmac_finish_td(int portno, struct sk_buff *skb, unsigned int flags);
#endif

/* Module parameters */
#define TX_TIMEO	5000
#define JUMBO_LEN	9000

#define	GMAC_TIMER_RATIO	(32768)				/*one second*/
#define	GMAC_TMOUT	(2 * GMAC_TIMER_RATIO)   	/*must less than 16 second*/
#define	GMAC_LEN_RATIO	(1024*1024)

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

struct stmmac_extra_stats *gmac_status;
struct stmmac_priv *gmac_priv;
struct gmac_tx_queue *tx_queue_class_a;
struct gmac_tx_queue *tx_queue_class_b;

#define STMMAC_ALIGN(x)	L1_CACHE_ALIGN(x)
static int watchdog = TX_TIMEO;

module_param(watchdog, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(watchdog, "Transmit timeout in milliseconds (default 5s)");

static int debug = -1;
module_param(debug, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Message Level (-1: default, 0: no output, 16: all)");

int phyaddr = -1;
module_param(phyaddr, int, S_IRUGO);
MODULE_PARM_DESC(phyaddr, "Physical device address");

#define DMA_TX_SIZE 512
static int dma_txsize = DMA_TX_SIZE;
module_param(dma_txsize, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dma_txsize, "Number of descriptors in the TX list");

#define DMA_RX_SIZE 512
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

#define DMA_BUFFER_SIZE	BUF_SIZE_2KiB
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
static irqreturn_t stmmac_interrupt(int irq, void *dev_id);

#ifdef CONFIG_NEW_STMMAC_DEBUG_FS
static int stmmac_init_fs(struct net_device *dev);
static void stmmac_exit_fs(void);
#endif

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))   
//static 
netdev_tx_t stmmac_xmit(struct sk_buff *skb, struct net_device *dev);
#endif

#define STMMAC_COAL_TIMER(x) (jiffies + usecs_to_jiffies(x))
#ifdef CONFIG_GMAC_DEBUG
unsigned int gmac_msg_level = GMAC_LEVEL_ERR;
#endif
int riwt_value = MIN_DMA_RIWT;
int rx_irq_flag = 0;

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

void set_gmac_avb(unsigned int status)
{
	gmac_priv->avb_support = !!(status);
}
EXPORT_SYMBOL(set_gmac_avb);

unsigned long get_gmac_avb(void)
{
	return gmac_priv->avb_support;
}
EXPORT_SYMBOL(get_gmac_avb);

unsigned int get_gmac_msg(void)
{
	printk("level : %x\n",gmac_msg_level);
    return gmac_msg_level;
}
EXPORT_SYMBOL(get_gmac_msg);

void gmac_enable_print_rate(unsigned int enable)
{
	gmac_priv->enable_rate = !!(enable);
}
EXPORT_SYMBOL(gmac_enable_print_rate);

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
 
/* minimum number of free TX descriptors required to wake up TX process */
#define STMMAC_TX_THRESH(x, n)	(x->tx_queue[n].dma_tx_size/8)

static inline u32 stmmac_tx_avail(struct stmmac_priv *priv, int chn)
{
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, chn);
	return tx_queue->dirty_tx + tx_queue->dma_tx_size - tx_queue->cur_tx - 1;
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
static void stmmac_enable_eee_mode(struct stmmac_priv *priv, int chn)
{
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, chn);
	
	/* Check and enter in LPI mode */
	if ((tx_queue->dirty_tx == tx_queue->cur_tx) &&
	    (tx_queue->tx_path_in_lpi_mode == false))
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
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, 0);
	
	priv->hw->mac->reset_eee_mode(priv->ioaddr);
	del_timer_sync(&priv->eee_ctrl_timer);
	tx_queue->tx_path_in_lpi_mode = false;
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

	stmmac_enable_eee_mode(priv, 0);
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
#if 0
/* stmmac_get_tx_hwtstamp: get HW TX timestamps
 * @priv: driver private structure
 * @entry : descriptor index to be used.
 * @skb : the socket buffer
 * Description :
 * This function will read timestamp from the descriptor & pass it to stack.
 * and also perform some sanity checks.
 */
static void stmmac_get_tx_hwtstamp(struct stmmac_priv *priv,
				   unsigned int entry, struct sk_buff *skb, int chn)
{
	struct skb_shared_hwtstamps shhwtstamp;
	u64 ns;
	void *desc = NULL;
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, chn);

	if (!tx_queue->hwts_tx_en)
		return;

	/* exit if skb doesn't support hw tstamp */
	if (likely(!(skb_shinfo(skb)->tx_flags & SKBTX_IN_PROGRESS)))
		return;
	
	desc = (tx_queue->dma_tx + entry);

	/* check tx tstamp status */
	if (!priv->hw->desc->get_tx_timestamp_status((union dma_desc *)desc))
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
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, 0);

	if (!rx_queue->hwts_rx_en)
		return;

	desc = (rx_queue->dma_rx + entry);

	/* exit if rx tstamp is not valid */
	if (!priv->hw->desc->get_rx_timestamp_status(desc, priv->adv_ts))
		return;

	/* get valid tstamp */
	ns = priv->hw->desc->get_timestamp(desc, priv->adv_ts);
	shhwtstamp = skb_hwtstamps(skb);
	memset(shhwtstamp, 0, sizeof(struct skb_shared_hwtstamps));
	shhwtstamp->hwtstamp = ns_to_ktime(ns);
}
#endif
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
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, 0);
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

	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, 0);

	if (!(priv->dma_cap.time_stamp)) {
		netdev_alert(priv->dev, "No support for HW time stamping\n");
		tx_queue->hwts_tx_en = 0;
		rx_queue->hwts_rx_en = 0;

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
		tx_queue->hwts_tx_en = 0;
		break;
	case HWTSTAMP_TX_ON:
		tx_queue->hwts_tx_en = 1;
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
	rx_queue->hwts_rx_en = ((config.rx_filter == HWTSTAMP_FILTER_NONE) ? 0 : 1);

	if (!tx_queue->hwts_tx_en && !rx_queue->hwts_rx_en)
		priv->hw->ptp->config_hw_tstamping(priv->ioaddr, 0);
	else {
		value = (GMAC_TCR_TSENA | GMAC_TCR_TSCFUPDT | GMAC_TCR_TSCTRLSSR |
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
	int ret;
	
	if (!(priv->dma_cap.time_stamp))
		return -EOPNOTSUPP;

	if (netif_msg_hw(priv)) {
		if (priv->dma_cap.time_stamp) {
			GMAC_DBG(("IEEE 1588-2002 Time Stamp supported\n"));
		}
	}

	priv->hw->ptp = &stmmac_ptp;
	priv->hw->slot = &stmmac_slot;
	priv->rx_queue[RX_CHN].hwts_rx_en = 1;
	ret = priv->hw->ptp->init(priv->ioaddr);
	if (ret) {
		GMAC_ERR(("[%s]Init PTP failed!(error no:%d)\n", __func__, ret));
		return ret;
	}
	
	priv->tx_queue[TX_CHN_CLASS_A].hwts_tx_en = 1;
	priv->hw->slot->config_slot_compare(priv->ioaddr, TX_CHN_CLASS_A, 1);
	
	priv->tx_queue[TX_CHN_CLASS_B].hwts_tx_en = 1;
	priv->hw->slot->config_slot_compare(priv->ioaddr, TX_CHN_CLASS_B, 1);
	//priv->hw->slot->config_slot_advance_check(priv->ioaddr, TX_CHN_CLASS_B, 1);

	return stmmac_ptp_register(priv);
}

static void stmmac_release_ptp(struct stmmac_priv *priv)
{
	stmmac_ptp_unregister(priv);
}

static void stmmac_init_vlan(struct stmmac_priv *priv)
{
	int value;

	/* VLAN Tag Hash Table Match\ Double VLAN Processing Enable */
	//value = readl(priv->ioaddr + GMAC_VLAN_TAG);
	value = VLAN_TAG_VL_VID | VLAN_TAG_VL_CFI | VLAN_TAG_VL_PCP| VLAN_TAG_ETV;
	writel(value, priv->ioaddr + GMAC_VLAN_TAG);
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
	int chn;
	unsigned int fc = priv->flow_ctrl, pause_time = priv->pause;

	if (phydev == NULL)
		return;
	
	spin_lock_irqsave(&priv->lock, flags);

	if (phydev->link) {
		u32 ctrl = readl(priv->ioaddr + GMAC_CONTROL);

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
		if (phydev->pause) {
			for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
				priv->hw->mac->flow_ctrl(priv->ioaddr, phydev->duplex,
						 fc, pause_time, chn);
			}
		}

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

		writel(ctrl, priv->ioaddr + GMAC_CONTROL);

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
		snprintf(bus_id, MII_BUS_ID_SIZE, "stmmac-%x", priv->plat->bus_id);

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
 * Description: display the control/status and buffer descriptors.
 */
static void stmmac_display_ring(void *head, int size, int control)
{
	union dma_desc *p = (union dma_desc *)head;
	
	if (gmac_msg_level & control) {
		print_hex_dump(KERN_DEBUG, "gmac desc:", DUMP_PREFIX_NONE,
		       16, 1, (void *)p, size, false);
	}
}

static void stmmac_display_rings(struct stmmac_priv *priv)
{
	int chn;
	unsigned int txsize;
	unsigned int rxsize;
	struct gmac_rx_queue *rx_queue;
	struct gmac_tx_queue *tx_queue;

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		rx_queue = GET_RX_QUEUE(priv, chn);
		rxsize = rx_queue->dma_rx_size;
		GMAC_RING_DBG(("RX chn[%d] descriptor ring:\n", chn));
		stmmac_display_ring((void *)rx_queue->dma_rx, rxsize, GMAC_LEVEL_RING_DBG);
	}
	
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);
		txsize = tx_queue->dma_tx_size;
		GMAC_RING_DBG(("TX chn[%d] descriptor ring:\n", chn));
		stmmac_display_ring((void *)(tx_queue->dma_tx), txsize, GMAC_LEVEL_RING_DBG);
	}
}

/**
 * stmmac_clear_descriptors: clear descriptors
 * @priv: driver private structure
 * Description: this function is called to clear the tx and rx descriptors
 */
static void stmmac_clear_descriptors(struct stmmac_priv *priv)
{
	int i;
	int chn;
	struct gmac_tx_queue *tx_queue;
	struct gmac_rx_queue *rx_queue;

	/* Clear the Rx/Tx descriptors */
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);
		for (i = 0; i < tx_queue->dma_tx_size; i++) {
			priv->hw->desc->init_tx_desc(tx_queue->dma_tx + i);	//ndesc_init_tx_desc
		}
	}

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		rx_queue = GET_RX_QUEUE(priv, chn);
		for (i = 0; i < rx_queue->dma_rx_size; i++) {
			priv->hw->desc->init_rx_desc((rx_queue->dma_rx + i), !(rx_queue->use_riwt));	//ndesc_init_rx_desc
		}
	}
}

static int stmmac_init_rx_buffers(struct stmmac_priv *priv, 
	union dma_desc *p, int i, int chn)
{
	struct sk_buff *skb;
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, chn);
	
	skb = __netdev_alloc_skb(priv->dev, priv->dma_buf_sz + NET_IP_ALIGN,
				 GFP_KERNEL);
		if (unlikely(skb == NULL)) {
		pr_err("%s: Rx init fails; skb is NULL\n", __func__);
		return -ENOMEM;
	}
	
	skb_reserve(skb, NET_IP_ALIGN);
	rx_queue->rx_skbuff[i] = skb;
	rx_queue->rx_skbuff_dma[i] = dma_map_single(priv->device, skb->data,
						priv->dma_buf_sz,
						DMA_FROM_DEVICE);
	
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		spe_hook.set_skb_dma(skb, rx_queue->rx_skbuff_dma[i]);
	}
#endif

	/* Using buffer 1, no buffer 2*/
	p->rx_desc.nrd.buf1_phy_addr = rx_queue->rx_skbuff_dma[i];
	p->rx_desc.nrd.buf1_addr_valid = 1;
	
	return 0;
}

void stmmac_free_rx_buffers(struct stmmac_priv *priv, int chn)
{
	int i;
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, chn);

	for (i = 0; i < rx_queue->dma_rx_size; i++) {
		if (rx_queue->rx_skbuff_dma[i]) {
			dma_unmap_single(priv->device, rx_queue->rx_skbuff_dma[i],
						priv->dma_buf_sz,
						DMA_FROM_DEVICE);
		}

		if (rx_queue->rx_skbuff[i]) {
			dev_kfree_skb_any(rx_queue->rx_skbuff[i]);
			rx_queue->rx_skbuff[i] = NULL;
		}
	}
}


int gmac_alloc_queue_struct(struct stmmac_priv *priv)
{
	int ret = 0;
	priv->tx_queue_cnt = priv->dma_cap.num_tx_queue + 1;
	priv->rx_queue_cnt = priv->dma_cap.num_rx_queue + 1;

	GMAC_DBG(("[%s]tx_queue_cnt:%d,rx_queue_cnt:%d\n", __func__,
						priv->tx_queue_cnt,	priv->rx_queue_cnt));

	priv->tx_queue =
		kzalloc(sizeof(struct gmac_tx_queue) * (priv->tx_queue_cnt),
		GFP_KERNEL);	
	if (priv->tx_queue == NULL) {
		GMAC_ERR(("ERROR: Unable to allocate Tx queue structure\n"));
		ret = -ENOMEM;
	}

	priv->rx_queue =
		kzalloc(sizeof(struct gmac_rx_queue) * (priv->rx_queue_cnt),
		GFP_KERNEL);
	if (priv->rx_queue == NULL) {
		GMAC_ERR(("ERROR: Unable to allocate Rx queue structure\n"));
		ret = -ENOMEM;
		goto free_res;
	}

	GMAC_DBG(("[%s]\n", __func__));

	return 0;

free_res:
	kfree(priv->tx_queue);
	return ret;
}

void gmac_free_queue_struct(struct stmmac_priv *priv)
{
	kfree(priv->tx_queue);
	kfree(priv->rx_queue);	
}

void gmac_free_mac(struct stmmac_priv *priv)
{
	kfree(priv->hw);
}

int gmac_alloc_rx_stat(struct stmmac_priv *priv)
{
	int ret = 0;
	
	priv->rx_stat =
		kzalloc(sizeof(struct gmac_rx_stat) * (priv->tx_queue_cnt),
		GFP_KERNEL);	
	if (priv->rx_stat == NULL) {
		GMAC_ERR(("ERROR: Unable to allocate Rx stat\n"));
		ret = -ENOMEM;
	}
	return ret;
}

void gmac_free_rx_stat(struct stmmac_priv *priv)
{
	if (priv->rx_stat) {
		kfree(priv->rx_stat);
	}
}

void init_mtl_para(struct stmmac_priv *priv)
{
	int tx_size;
	int rx_size;

	tx_size = priv->dma_cap.tx_fifo_size;
	rx_size = priv->dma_cap.rx_fifo_size;

	/* 2:1:1
	 * all TX fifo size :BIT(tx_size) * 128 bytes = BIT(tx_size) << 7 
	 * 2/4 size: (BIT(tx_size) << 7) >> 1
	 * 0 for  256 bytes so :((BIT(tx_size) << 7) >> 1) >> 8 - 1
	 * so it is :(BIT(tx_size) >> 2) - 1
	 * now, rx fifo size is 8K, tx fifo size is 8K, tx_size = 6, rx_size = 6
	 * tx0 fifo size:15(16*256bytes,4K),tx1 fifo size is 7(8*256,2k) tx2 is 2K
	 * rx fifo size:31(32 * 256 bytes, 8k)
	*/
	priv->tx_queue[TX_CHN_NET].mtl_fifo_size = (BIT(tx_size) >> 2) - 1;			
	priv->tx_queue[TX_CHN_CLASS_B].mtl_fifo_size = (BIT(tx_size) >> 3) - 1;		
	priv->tx_queue[TX_CHN_CLASS_A].mtl_fifo_size = (BIT(tx_size) >> 3) - 1;		
	priv->rx_queue[RX_CHN].mtl_fifo_size = (BIT(tx_size) >> 1) - 1;	;
}

/**
 * init_dma_desc_rings - init the RX/TX descriptor rings
 * @dev: net device structure
 * Description:  this function initializes the DMA RX/TX descriptors
 * and allocates the socket buffers. It suppors the chained and ring
 * modes.
 */
static int init_dma_desc_rings(struct net_device *dev)
{
	int i;
	int chn;
	int ret;
	struct stmmac_priv *priv = netdev_priv(dev);
	struct gmac_rx_queue *rx_queue;
	struct gmac_tx_queue *tx_queue;
	unsigned int txsize;
	unsigned int rxsize;
	unsigned int bfsize = 0;
	unsigned int td_width, rd_width;

	/* Set the max buffer size according to the DESC mode
	 * and the MTU. Note that RING mode allows 16KiB bsize.
	 */
	bfsize = priv->hw->ring->set_16kib_bfsize(dev->mtu);
	td_width = sizeof(union dma_desc); //all tx desc have the same size:16 bytes
	rd_width = sizeof(union dma_desc); //all rx desc have the same size:16 bytes

	if (bfsize < BUF_SIZE_16KiB)
        bfsize = BUF_SIZE_1_8kiB;

	/* All of channels using the same buffer size. */
	priv->dma_buf_sz = bfsize;
	buf_sz = bfsize;
	GMAC_TRACE(("stmmac: bfsize %d\n", bfsize));
	
	/* Create and initialize the TX/RX descriptors chains. */
	priv->dma_buf_sz = STMMAC_ALIGN(buf_sz);

	/* Multi-channel Compatible */
	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {

		rx_queue = GET_RX_QUEUE(priv, chn);
		/* fixed me!:the channel should have its dma_rxsize */		
		rx_queue->dma_rx_size = STMMAC_ALIGN(dma_rxsize);
		rxsize = rx_queue->dma_rx_size;
		
		rx_queue->dma_rx = dma_alloc_coherent(priv->device, rxsize *
					  rd_width, &(rx_queue->dma_rx_phy), GFP_KERNEL);
		if (!(rx_queue->dma_rx)) {
			ret = -ENOMEM;
			goto rx_queue_free;
		}
		
		/* Initialising tail pointer the same as tx_phy */
		//rx_queue->dma_rx_tail = rx_queue->dma_rx_phy;

		
		/* Using one rx buffer in a descriptor. */
		rx_queue->rx_skbuff_dma = kmalloc_array(rxsize, sizeof(dma_addr_t),
					    GFP_KERNEL);
		if (!(rx_queue->rx_skbuff_dma)) {
			ret = -ENOMEM;
			goto rx_queue_free;
		}
		
		rx_queue->rx_skbuff = kmalloc_array(rxsize, sizeof(struct sk_buff *),
						GFP_KERNEL);
		if (!(rx_queue->rx_skbuff)) {
			ret = -ENOMEM;
			goto rx_queue_free;
		}

		if (netif_msg_drv(priv)) {
			GMAC_DBG(("(%s) dma_rx_phy[%d]=0x%08x\n", __func__, chn, 
			 (unsigned int)(rx_queue->dma_rx_phy)));
		}

		for (i = 0; i < rxsize; i++) {
			union dma_desc *p;
			p = rx_queue->dma_rx + i;
			memset((void *)p, 0, sizeof(union dma_desc));

			ret = stmmac_init_rx_buffers(priv, p, i, chn);
			if (ret) {
				goto rx_queue_free;
			}

			/* Clear the Rx descriptors */
			priv->hw->desc->init_rx_desc((rx_queue->dma_rx + i), !(rx_queue->use_riwt));
		}

		rx_queue->cur_rx = 0;
		rx_queue->dirty_rx = (unsigned int)(i - rxsize);
	}

	/* Multi-channel compatible and using one tx buffer in a descriptor. */
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);		
		
		/* the channel should have its dma_txsize */
		tx_queue->dma_tx_size = STMMAC_ALIGN(dma_txsize);
		txsize = tx_queue->dma_tx_size;
		
		tx_queue->dma_tx = dma_alloc_coherent(priv->device, txsize *
					  td_width, &(tx_queue->dma_tx_phy), GFP_KERNEL);
		if (!(tx_queue->dma_tx)) {
			ret = -ENOMEM;
			goto tx_queue_free;
		}

		/* Initialising taril pointer the same as tx_phy */
		tx_queue->dma_tx_tail = tx_queue->dma_tx_phy;
		tx_queue->tx_skbuff_dma = kzalloc((txsize * sizeof(dma_addr_t)), GFP_KERNEL);
		if (!(tx_queue->tx_skbuff_dma)) {
			ret = -ENOMEM;
			goto tx_queue_free;
		}
		
		tx_queue->tx_skbuff = kzalloc((txsize * sizeof(struct sk_buff *)), GFP_KERNEL);
		if (!(tx_queue->tx_skbuff)) {
			ret = -ENOMEM;
			goto tx_queue_free;
		}
		memset((void *)tx_queue->dma_tx, 0, (txsize * td_width));

		if (netif_msg_drv(priv)) {
			GMAC_DBG(("(%s) dma_tx_phy[%d]=0x%08x\n", __func__, chn, 
			 (unsigned int)(tx_queue->dma_tx_phy)));
		}

		for (i = 0; i < txsize; i++) {	
			priv->hw->desc->init_tx_desc(tx_queue->dma_tx + i);
		}

		tx_queue->dirty_tx = 0;
		tx_queue->cur_tx = 0;
	}

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
    if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
        skb_queue_head_init(&priv->free_q);
    }
#endif
	
	if (netif_msg_hw(priv))
		stmmac_display_rings(priv);

	return 0;
	
tx_queue_free:
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);
		if (tx_queue->dma_tx) {
			dma_free_coherent(priv->device, txsize *
					  td_width, tx_queue->dma_tx, tx_queue->dma_tx_phy);
		}

		if (tx_queue->tx_skbuff_dma) {
			kfree(tx_queue->tx_skbuff_dma);
		}

		if (tx_queue->tx_skbuff) {
			kfree(tx_queue->tx_skbuff);
		}
	}
	
rx_queue_free:
	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		rx_queue = GET_RX_QUEUE(priv, chn);
		if (rx_queue->dma_rx) {
			dma_free_coherent(priv->device, rx_queue->dma_rx_size *
					  rd_width, rx_queue->dma_rx, rx_queue->dma_rx_phy);
		}

		if (rx_queue->rx_skbuff_dma) {
			kfree(rx_queue->rx_skbuff_dma);
		}

		if (rx_queue->rx_skbuff) {
			kfree(rx_queue->rx_skbuff);
		}

		stmmac_free_rx_buffers(priv, chn);
	}

	return ret;
}

static void dma_free_rx_skbufs(struct stmmac_priv *priv, int chn)
{
	int i;
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, chn);
	
	for (i = 0; i < rx_queue->dma_rx_size; i++) {
		if (rx_queue->rx_skbuff[i]) {
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
            if (!rx_queue->rx_skbuff[i]->spe_own)
#endif
			{
			dma_unmap_single(priv->device, rx_queue->rx_skbuff_dma[i],
					 priv->dma_buf_sz, DMA_FROM_DEVICE);
			dev_kfree_skb_any(rx_queue->rx_skbuff[i]);
			}
		}
		rx_queue->rx_skbuff[i] = NULL;
	}
}

static void dma_free_tx_skbufs(struct stmmac_priv *priv, int chn)
{
	int i;
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, chn);
	
	for (i = 0; i < tx_queue->dma_tx_size; i++) {
		if (tx_queue->tx_skbuff[i] != NULL) {
			union dma_desc *p;
			p = tx_queue->dma_tx + i;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
            if (tx_queue->tx_skbuff[i]->spe_own) {
                if (tx_queue->tx_skbuff_dma[i]) {
                    spe_hook.rd_config(priv->portno, 
                        tx_queue->tx_skbuff[i], tx_queue->tx_skbuff_dma[i]);
                }
                   
            } else
#endif
			{
				if (tx_queue->tx_skbuff_dma[i])
					dma_unmap_single(priv->device,
							 tx_queue->tx_skbuff_dma[i],
							 priv->hw->desc->get_tx_len(p),
							 DMA_TO_DEVICE);
				dev_kfree_skb_any(tx_queue->tx_skbuff[i]);
			}
			tx_queue->tx_skbuff[i] = NULL;
			tx_queue->tx_skbuff_dma[i] = 0;
		}
	}
}

static void free_dma_desc_resources(struct stmmac_priv *priv)
{
	int chn;
	struct gmac_rx_queue *rx_queue;
	struct gmac_tx_queue *tx_queue;
	
	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		
		/* Release the DMA TX/RX socket buffers */
		dma_free_rx_skbufs(priv, chn);

		/* Free DMA regions of consistent memory previously allocated */
		rx_queue = GET_RX_QUEUE(priv, chn);
		dma_free_coherent(priv->device,
				  rx_queue->dma_rx_size * sizeof(union dma_desc),
				  rx_queue->dma_rx, rx_queue->dma_rx_phy);

		kfree(rx_queue->rx_skbuff_dma);
		kfree(rx_queue->rx_skbuff);
	}

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		dma_free_tx_skbufs(priv, chn);
		tx_queue = GET_TX_QUEUE(priv, chn);
		dma_free_coherent(priv->device,
				  tx_queue->dma_tx_size * sizeof(union dma_desc),
				  tx_queue->dma_tx, tx_queue->dma_tx_phy);

		kfree(tx_queue->tx_skbuff_dma);
		kfree(tx_queue->tx_skbuff);
	}
		
}
#if 0
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
#endif
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

    return stmmac_xmit(skb, dev);
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

	stmmac_reset_rx_skb_spe(skb);
	skb_queue_tail(&priv->free_q, skb);
	stmmac_rx_refill(priv);

    return ret;
}
#endif

/**
 * stmmac_tx_clean:
 * @priv: driver private structure
 * Description: it reclaims resources after transmission completes.
 */
void stmmac_tx_clean(struct stmmac_priv *priv)
{
	unsigned int txsize;
	unsigned long flags[TX_CHN_NR];
	int chn;
	struct gmac_tx_queue *tx_queue = NULL;

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);
		txsize = tx_queue->dma_tx_size;
		spin_lock_irqsave(&tx_queue->tx_lock, flags[chn]);

		priv->xstats.tx_clean[chn]++;

		while (tx_queue->dirty_tx != tx_queue->cur_tx) {
			int last;
			unsigned int entry = tx_queue->dirty_tx % txsize;
			struct sk_buff *skb = tx_queue->tx_skbuff[entry];
			union dma_desc *p;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	        dma_addr_t dma;
#endif

			p = tx_queue->dma_tx + entry;

			/* Check if the descriptor is owned by the DMA. */
			if (priv->hw->desc->get_tx_owner(p)) {
				priv->xstats.tx_clean_own[chn]++;
				break;
			}

			/* Verify tx error by looking at the last segment. */
			last = priv->hw->desc->get_tx_ls(p);
			if (likely(last)) {
				int tx_error =
				    priv->hw->desc->tx_status(&priv->dev->stats,
							      &priv->xstats, p,
							      priv->ioaddr, chn);
				if (likely(tx_error == 0)) {
					priv->dev->stats.tx_packets++;
					priv->xstats.tx_pkt_n[chn]++;
				} else
					priv->dev->stats.tx_errors++;

				//stmmac_get_tx_hwtstamp(priv, entry, skb, i);
			}
			GMAC_DBG(("%s: chn[%d], curr %d, dirty %d\n", __func__, chn, 
			       tx_queue->cur_tx, tx_queue->dirty_tx));

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
		if (skb->spe_own) {
            tx_queue->tx_skbuff_dma[entry] = 0;
		}else
#endif
			{
				if (likely(tx_queue->tx_skbuff_dma[entry])) {
					dma_unmap_single(priv->device,
							 tx_queue->tx_skbuff_dma[entry],
							 priv->hw->desc->get_tx_len(p),
							 DMA_TO_DEVICE);
					tx_queue->tx_skbuff_dma[entry] = 0;
				}
			}

			/*clean the descriptor */
			
			if (likely(skb != NULL)) {
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	            if((spe_hook.is_enable && spe_hook.is_enable()) && (skb->spe_own) &&(spe_mode_normal==spe_hook.mode())){
					dma = spe_hook.get_skb_dma(skb);
                   	spe_hook.rd_config(priv->portno, skb, dma);
	            }else
#endif
				{
					dev_kfree_skb_any(skb);
				}
				tx_queue->tx_skbuff[entry] = NULL;
			}

			priv->hw->desc->release_tx_desc(p);
			tx_queue->dirty_tx++;
		}
		
		if (unlikely(__netif_subqueue_stopped(priv->dev, chn) && (
			stmmac_tx_avail(priv, chn) > STMMAC_TX_THRESH(priv, chn)))) {
				netif_tx_lock(priv->dev);
				
				GMAC_DBG(("%s: restart transmit\n", __func__));
				netif_wake_subqueue(priv->dev, chn);
				
				netif_tx_unlock(priv->dev);
			}

		if ((priv->eee_enabled) && (!tx_queue->tx_path_in_lpi_mode)) {
			stmmac_enable_eee_mode(priv, chn);
			mod_timer(&priv->eee_ctrl_timer, STMMAC_LPI_T(eee_timer));
		}
		spin_unlock_irqrestore(&tx_queue->tx_lock, flags[chn]);
	}
}

static inline void stmmac_enable_dma_irq(struct stmmac_priv *priv)
{
	int chn;
	
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		priv->hw->dma->enable_dma_irq(priv->ioaddr, chn);
	}
}

static inline void stmmac_disable_dma_irq(struct stmmac_priv *priv)
{
	int chn;

	/* disable all of the channels */
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		priv->hw->dma->disable_dma_irq(priv->ioaddr, chn);
	}
}

/**
 * stmmac_tx_err: irq tx error mng function
 * @priv: driver private structure
 * Description: it cleans the descriptors and restarts the transmission
 * in case of errors.
 */
static void stmmac_tx_err(struct stmmac_priv *priv, int chn)
{
#if 0
	int i;
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(priv, chn);
	int txsize = tx_queue->dma_tx_size;
	netif_stop_queue(priv->dev);

	priv->hw->dma->stop_tx(priv->ioaddr, chn);
	dma_free_tx_skbufs(priv, chn);
	for (i = 0; i < txsize; i++) {
		#ifdef GMAC_STUB
		GMAC_DBG(("Stubed! we should check the init_tx_desc.:%s", __func__));
		#endif
		priv->hw->desc->init_tx_desc(tx_queue->dma_tx + i);
	}
	
	tx_queue->dirty_tx = 0;
	tx_queue->cur_tx = 0;
	priv->hw->dma->start_tx(priv->ioaddr, chn);
#endif
	priv->dev->stats.tx_errors++;
	netif_wake_subqueue(priv->dev, chn);
}

/**
 * stmmac_mtl_interrupt: MTL ISR
 * @priv: driver private structure
 * Description: this is the MTL ISR. It is called by the main ISR.
 * It calls the dwmac mtl routine to understand which type of interrupt
 * happened. 
 */
static void stmmac_mtl_interrupt(struct stmmac_priv *priv)
{
#if 0
	unsigned int value, queue_value;
	void __iomem *ioaddr;
	struct stmmac_extra_stats *x;
	int i;

	x = &(priv->xstats);
	ioaddr = priv->ioaddr;
	value = readl(ioaddr + MTL_INT_STAT);

	for (i = 0; i < TX_CHN_NR; i++) {
		if (value & MTL_QIS(i)) {
			value &= ~(MTL_QIS(i));
			x->mtl_int[i]++;
			queue_value = readl(ioaddr + MTL_QINT_CTL_STAT(i));			
		}
	}
#endif

	/* we don't enbale any mtl interrupt in MTL_Q0_Interrupt_Control_Status , so stub it. */
	
	return ;
}

/**
 * stmmac_dma_interrupt: DMA ISR
 * @priv: driver private structure
 * Description: this is the DMA ISR. It is called by the main ISR.
 * It calls the dwmac dma routine to understand which type of interrupt
 * happened. In case of there is a Normal interrupt and either TX or RX
 * interrupt happened so the NAPI is scheduled.
 */
static void stmmac_dma_interrupt(struct stmmac_priv *priv, int chn)
{
	int status;

	status = priv->hw->dma->dma_interrupt(priv->ioaddr, &priv->xstats, chn);
	if (likely((status & handle_rx)) || (status & handle_tx)) {
		if (likely(napi_schedule_prep(&priv->napi))) {
			stmmac_disable_dma_irq(priv);
			
			#ifdef GMAC_STUB
			GMAC_DBG(("stubed! Add multi-chn handler here!:%s\n", __func__));
			#endif
			
			__napi_schedule(&priv->napi); 
		}
	}
	#if 0
	if (unlikely(status & tx_hard_error_bump_tc)) {
		/* Try to bump up the dma threshold on this failure */
		if (unlikely(tc != SF_DMA_MODE) && (tc <= 256)) {
			tc += 64;
			priv->hw->dma->dma_mode(priv->ioaddr, tc, SF_DMA_MODE);
			priv->xstats.threshold = tc;
		}
	} else if (unlikely(status == tx_hard_error))
		stmmac_tx_err(priv, 0);
	#endif

	if (unlikely(status == tx_hard_error))
		stmmac_tx_err(priv, chn);
	
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
 * stmmac_get_hw_features: get MAC capabilities from the HW cap. register.
 * @priv: driver private structure
 * Description:
 *  new GMAC chip generations have a new register to indicate the
 *  presence of the optional feature/functions.
 *  This can be also used to override the value passed through the
 *  platform and necessary for old MAC10/100 and GMAC chips.
 */
static int stmmac_get_hw_features(struct stmmac_priv *priv, 
							unsigned int *hw_cap1, unsigned int *hw_cap2)
{
	u32 hw_cap0 = 0;

	if (priv->hw->dma->get_hw_feature) {
		hw_cap0 = priv->hw->dma->get_hw_feature(priv->ioaddr, hw_cap1, hw_cap2);

		priv->dma_cap.mbps_10_100 = (hw_cap0 & DMA_HW_FEAT_MIISEL);
		priv->dma_cap.mbps_1000 = (hw_cap0 & DMA_HW_FEAT_GMIISEL) >> 1;
		priv->dma_cap.half_duplex = (hw_cap0 & DMA_HW_FEAT_HDSEL) >> 2;
		priv->dma_cap.pcs = (hw_cap0 & DMA_HW_FEAT_PCSSEL) >> 3;
		priv->dma_cap.vlan_hash_filter = (hw_cap0 & DMA_HW_FEAT_VLHASH) >> 4;
		priv->dma_cap.sma_mdio = (hw_cap0 & DMA_HW_FEAT_SMASEL) >> 5;
		priv->dma_cap.pmt_remote_wake_up = (hw_cap0 & DMA_HW_FEAT_RWKSEL) >> 6;
		priv->dma_cap.pmt_magic_frame = (hw_cap0 & DMA_HW_FEAT_MGKSEL) >> 7;
		
		/* MMC */
		priv->dma_cap.rmon            = (hw_cap0 & DMA_HW_FEAT_MMCSEL) >> 8;

		/*ARP offload*/
		priv->dma_cap.arp_offload     = (hw_cap0 & DMA_HW_FEAT_ARPOFFSEL) >> 9;
		
		/* IEEE 1588-2008 */
		priv->dma_cap.time_stamp      = (hw_cap0 & DMA_HW_FEAT_TSSEL) >> 12;

		/* 802.3az - Energy-Efficient Ethernet (EEE) */
		priv->dma_cap.eee             = (hw_cap0 & DMA_HW_FEAT_EEESEL) >> 13;
		
		/* TX and RX csum */
		priv->dma_cap.tx_coe          = (hw_cap0 & DMA_HW_FEAT_TXCOESEL) >> 14;
		priv->dma_cap.rx_coe          = (hw_cap0 & DMA_HW_FEAT_RXCOESEL) >> 16;
		priv->dma_cap.multi_addr      = (hw_cap0 & DMA_HW_FEAT_ADDMACADRSEL) >> 18;
		priv->dma_cap.multi_addr32    = (hw_cap0 & DMA_HW_FEAT_MACADR32SEL) >> 23;
		priv->dma_cap.multi_addr64    = (hw_cap0 & DMA_HW_FEAT_MACADR64SEL) >> 24;

		/*Timestamp system time source*/
		priv->dma_cap.systime_source  = (hw_cap0 & DMA_HW_FEAT_TSSTSSEL) >> 25;
		priv->dma_cap.sa_vlan_ins     = (hw_cap0 & DMA_HW_FEAT_SAVLANINS) >> 27;
		priv->dma_cap.phy_mode        = (hw_cap0 & DMA_HW_FEAT_ACTPHYSEL) >> 28;


		priv->dma_cap.rx_fifo_size    = ((*hw_cap1) & DMA_HW_FEAT_RXFIFOSIZE);
		priv->dma_cap.tx_fifo_size    = ((*hw_cap1) & DMA_HW_FEAT_TXFIFOSIZE) >> 6;
		priv->dma_cap.one_step_ts     = ((*hw_cap1) & DMA_HW_FEAT_OSTEN) >> 11;
		priv->dma_cap.ptp_offload     = ((*hw_cap1) & DMA_HW_FEAT_PTOEN) >> 12;
		priv->dma_cap.high_word_reg   = ((*hw_cap1) & DMA_HW_FEAT_ADVTHWORD) >> 13;
		priv->dma_cap.addr_width      = ((*hw_cap1) & DMA_HW_FEAT_ADDR64) >> 14;
		priv->dma_cap.dcb_enable      = ((*hw_cap1) & DMA_HW_FEAT_DCBEN) >> 16;
		priv->dma_cap.split_hdr       = ((*hw_cap1) & DMA_HW_FEAT_SPHEN) >> 17;
		priv->dma_cap.tso_en          = ((*hw_cap1) & DMA_HW_FEAT_TSOEN) >> 18;
		priv->dma_cap.debug_mem_if    = ((*hw_cap1) & DMA_HW_FEAT_DBGMEMA) >> 19;
		priv->dma_cap.av_en 		  = ((*hw_cap1) & DMA_HW_FEAT_AVSEL) >> 20;
		priv->dma_cap.hash_table_size = ((*hw_cap1) & DMA_HW_FEAT_HASHTBLSZ) >> 24;
		priv->dma_cap.l3l4_total_num  = ((*hw_cap1) & DMA_HW_FEAT_L3L4FNUM) >> 27;

		priv->dma_cap.num_rx_queue   = ((*hw_cap2) & DMA_HW_FEAT_RXQCNT);
		priv->dma_cap.num_tx_queue   = ((*hw_cap2) & DMA_HW_FEAT_TXQCNT) >> 6;
		priv->dma_cap.num_rx_channel = ((*hw_cap2) & DMA_HW_FEAT_RXCHCNT) >> 12;
		priv->dma_cap.num_tx_channel = ((*hw_cap2) & DMA_HW_FEAT_TXCHCNT) >> 18;
		priv->dma_cap.num_pps_output = ((*hw_cap2) & DMA_HW_FEAT_PPSOUTNUM) >> 24;
		priv->dma_cap.num_aux_snap 	 = ((*hw_cap2) & DMA_HW_FEAT_AUXSNAPNUM) >> 28;
	}

	return hw_cap0;
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
	/* check dev_addr if valid*/
	if (!is_valid_ether_addr(priv->dev->dev_addr)) {

		/* get the 0th mac address form register, and get it to dev_addr */
		priv->hw->mac->get_umac_addr((void __iomem *)
					     priv->dev->base_addr,
					     priv->dev->dev_addr, 0);
		
		/* check the dev_address if valid, or using random mac address */
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
	int fixed_burst = 0, burst_len = 0;
	int mixed_burst = 0;

	if (priv->plat->dma_cfg) {
		fixed_burst = priv->plat->dma_cfg->fixed_burst;
		mixed_burst = priv->plat->dma_cfg->mixed_burst;
		burst_len = priv->plat->dma_cfg->burst_len;
	}

	return priv->hw->dma->init(fixed_burst, mixed_burst, burst_len);
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
	int chn;
	struct gmac_tx_queue *tx_queue = NULL;
	
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);
		tx_queue->tx_coal_frames = STMMAC_TX_FRAMES;
		tx_queue->tx_coal_timer = STMMAC_COAL_TX_TIMER;
		init_timer(&(tx_queue->txtimer));
		tx_queue->txtimer.expires = STMMAC_COAL_TIMER(tx_queue->tx_coal_timer);
		tx_queue->txtimer.data = (unsigned long)priv;
		tx_queue->txtimer.function = stmmac_tx_timer;
		add_timer(&(tx_queue->txtimer));
	}
}

static void stmmac_get_line_speed(struct stmmac_priv *priv)
{
	unsigned int line_status;
	
	line_status = priv->hw->mac->get_line_spped(priv->ioaddr);
	
	switch (line_status) {
		case STMMAC_LINE_SPPED_1000M:
			priv->line_speed = 1000;
			break;

		case STMMAC_LINE_SPPED_100M:
			priv->line_speed = 100;
			break;

		default:
			priv->line_speed = 0;
			break;
	}
}

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
	int chn;
	struct gmac_rx_queue *rx_queue;

	ret = stmmac_clk_enable(priv->device);
	if (ret) {
		goto close_clk;
	}

	/* 获取mac地址并检查 */
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
	
	ret = init_dma_desc_rings(dev);
	if (ret) {
		goto open_error;
	}
	
	init_mtl_para(priv);
	
	/* DMA initialization and SW reset */
	ret = stmmac_init_dma_engine(priv);
	if (ret < 0) {
		GMAC_ERR(("%s: DMA initialization failed\n", __func__));
		goto open_error;
	}

	/* If required, perform hw setup of the bus. */
	if (priv->plat->bus_setup)
		priv->plat->bus_setup(priv->ioaddr);

	/*Initialize the MTL */
	priv->hw->mtl->mtl_init();
	
	/* Initialize the MAC Core */
	ret = priv->hw->mac->core_init(priv->ioaddr);
	if (ret) {
		goto open_error;
	}

	/* Copy the MAC addr into the HW  */
	priv->hw->mac->set_umac_addr(priv->ioaddr, dev->dev_addr, 0);

	/* Enable SPE port */
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		(void)spe_hook.port_enable(priv->portno);
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
	
	/* Request the Tx IRQ lines */
	ret = request_irq(priv->ch0_txirq, stmmac_interrupt,
			  IRQF_SHARED, dev->name, dev);
	if (unlikely(ret < 0)) {
		GMAC_ERR(("%s: ERROR: allocating the IRQ %d (error: %d)\n",
		       __func__, priv->ch0_txirq, ret));
		goto open_error_ch0_irq;
	}

	ret = request_irq(priv->ch1_txirq, stmmac_interrupt,
			  IRQF_SHARED, dev->name, dev);
	if (unlikely(ret < 0)) {
		GMAC_ERR(("%s: ERROR: allocating the IRQ %d (error: %d)\n",
		       __func__, priv->ch1_txirq, ret));
		goto open_error_ch1_irq;
	}

	ret = request_irq(priv->ch2_txirq, stmmac_interrupt,
			  IRQF_SHARED, dev->name, dev);
	if (unlikely(ret < 0)) {
		GMAC_ERR(("%s: ERROR: allocating the IRQ %d (error: %d)\n",
		       __func__, priv->ch2_txirq, ret));
		goto open_error_ch2_irq;
	}

	/* receive interrupt */
	ret = request_irq(priv->rx_irq, stmmac_interrupt,
			  IRQF_SHARED, dev->name, dev);
	if (unlikely(ret < 0)) {
		GMAC_ERR(("%s: ERROR: allocating the IRQ %d (error: %d)\n",
		       __func__, priv->rx_irq, ret));
		goto open_error_rx_irq;
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

	/* Set the HW DMA mode and the COE, store and forward. */
	//stmmac_dma_operation_mode(priv);

	/* Extra statistics */
	memset(&priv->xstats, 0, sizeof(struct stmmac_extra_stats));
	priv->xstats.threshold = tc;

	/* Support avb feature in default. */
	priv->avb_support = 1;	
	gmac_status = &priv->xstats;

	stmmac_get_line_speed(priv);
	
	/* mmc_setup*/
	stmmac_mmc_setup(priv);

	ret = stmmac_init_ptp(priv);
	if (ret)
		GMAC_WARNING(("%s: failed PTP initialisation\n", __func__));

	stmmac_init_vlan(priv);
	
#ifdef CONFIG_NEW_STMMAC_DEBUG_FS
	ret = stmmac_init_fs(dev);
	if (ret < 0)
		GMAC_WARNING(("%s: failed debugFS registration\n", __func__));
#endif
	
	GMAC_DBG(( "%s: DMA RX/TX processes started...\n", dev->name));
	for (chn = 0;chn < priv->tx_queue_cnt; chn++)
		priv->hw->dma->start_tx(priv->ioaddr, chn);

	for (chn = 0;chn < priv->tx_queue_cnt; chn++)
		priv->hw->dma->start_rx(priv->ioaddr, chn);

	/* Dump DMA/MAC registers */
	if (netif_msg_hw(priv)) {
		priv->hw->mac->dump_regs(priv->ioaddr);
		priv->hw->dma->dump_regs(priv->ioaddr);
	}

	if (priv->phydev)
		phy_start(priv->phydev);

	priv->tx_lpi_timer = STMMAC_DEFAULT_TWT_LS;

	/* we don't using eee */
	//priv->eee_enabled = stmmac_eee_init(priv);

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		rx_queue = GET_RX_QUEUE(priv, chn);
		if ((rx_queue->use_riwt) && (priv->hw->dma->rx_watchdog)) {
			rx_queue->rx_riwt = riwt_value;
			priv->hw->dma->rx_watchdog(priv->ioaddr, riwt_value, chn);
		}
	}
	/* We don't using TBI etc, so we close it */
	/*
	if (priv->pcs && priv->hw->mac->ctrl_ane)
		priv->hw->mac->ctrl_ane(priv->ioaddr, 0);
	*/
	
	napi_enable(&priv->napi);
	netif_tx_start_all_queues(dev);

	return 0;

open_error_lpiirq:
	if (priv->wol_irq != dev->irq)
		free_irq(priv->wol_irq, dev);

open_error_wolirq:
	free_irq(dev->irq, dev);

open_error_rx_irq:
	free_irq(priv->rx_irq, dev);	

open_error_ch2_irq:
	free_irq(priv->ch2_txirq, dev);	

open_error_ch1_irq:
	free_irq(priv->ch1_txirq, dev);	
	
open_error_ch0_irq:
	free_irq(priv->ch0_txirq, dev);
	
open_error:
	if (priv->phydev)
		phy_disconnect(priv->phydev);

close_clk:
	stmmac_clk_disable(priv->device);

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
	int chn;
	
	struct stmmac_priv *priv = netdev_priv(dev);
	struct gmac_tx_queue *tx_queue;
	
	if (priv->eee_enabled)
		del_timer_sync(&priv->eee_ctrl_timer);


#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		(void)spe_hook.port_disable(priv->portno);
	}
#endif
	/* Stop and disconnect the PHY */
	if (priv->phydev) {
		phy_stop(priv->phydev);
		phy_disconnect(priv->phydev);
		priv->phydev = NULL;
	}

	netif_tx_stop_all_queues(dev);
	priv->xstats.gmac_release++;

	napi_disable(&priv->napi);

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);
		del_timer_sync(&tx_queue->txtimer);
	}

	/* Free the IRQ lines */
	free_irq(dev->irq, dev);
	if (priv->wol_irq != dev->irq)
		free_irq(priv->wol_irq, dev);
	if (priv->lpi_irq != -ENXIO)
		free_irq(priv->lpi_irq, dev);

	free_irq(priv->ch0_txirq, dev);
	free_irq(priv->ch1_txirq, dev);
	free_irq(priv->ch2_txirq, dev);
	free_irq(priv->rx_irq, dev);

	/* Stop TX/RX DMA and clear the descriptors */
	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		priv->hw->dma->stop_tx(priv->ioaddr, chn);
	}

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		priv->hw->dma->stop_rx(priv->ioaddr, chn);
	}

	/* Release and free the Rx/Tx resources */
	free_dma_desc_resources(priv);

	/* Disable the MAC Rx/Tx */
	stmmac_set_mac(priv->ioaddr, false);

	netif_carrier_off(dev);

#ifdef CONFIG_NEW_STMMAC_DEBUG_FS
	stmmac_exit_fs();
#endif


	stmmac_release_ptp(priv);

	stmmac_clk_disable(priv->device);

	return 0;
}

int gmac_get_skb_chn(struct sk_buff *skb, struct stmmac_priv *priv)
{
	int chn;
	unsigned short type;
	unsigned char pcp = 0;
	struct vlan_ethhdr *mac_header;

	skb_reset_mac_header(skb);
	mac_header = vlan_eth_hdr(skb);
	type = ntohs(mac_header->h_vlan_proto);
	switch (type) {
		case ETH_P_IP:
			chn = TX_CHN_NET;
			break;
			
		case ETH_P_8021Q:
			pcp = (ntohs(mac_header->h_vlan_TCI) & VLAN_PRIO_MASK) >> VLAN_PRIO_SHIFT;			
			if (pcp == priv->pcp_class_a) {
				chn = TX_CHN_CLASS_A;
			} else if (pcp == priv->pcp_class_b) {
				chn = TX_CHN_CLASS_B;
			} else {
				chn =  -1;
			}
			break;
			
		default:
			chn =  -1;
	}
	
	return chn;
}

int gmac_rx_rate(void)
{	
	int chn;
	unsigned int rate = 0;
	int time =  0;
	unsigned int ratio = GMAC_LEN_RATIO / (GMAC_TIMER_RATIO * 8);
	unsigned int cal_len;
	unsigned int all_bytes = 0;
	struct gmac_rx_stat *rx_stat;
	
	if(!gmac_priv->enable_rate) {
		return -EACCES;
	}
	
	time = get_timer_slice_delta(gmac_priv->time, bsp_get_slice_value());
	if(time < GMAC_TMOUT) {
		return -EAGAIN;
	}

	gmac_priv->time = bsp_get_slice_value();

	for (chn = 0; chn < gmac_priv->tx_queue_cnt; chn++) {
		rx_stat =  GET_RX_STAT(gmac_priv, chn);
		cal_len = rx_stat->rx_bytes - rx_stat->rx_last_bytes;
		rx_stat->rx_last_bytes = rx_stat->rx_bytes;
		all_bytes += cal_len;
					
		rate = cal_len / (time * ratio);
		printk("[received chn%d] rx rate:%uMbps\n", chn, rate);
	}

	rate= all_bytes / (time * ratio);
	printk("[received all chn] rx rate:%uMbps\n", rate);
	
	return 0;
}
#if 0
static int stmmac_check_tx_chn(struct stmmac_priv *priv, int chn)
{
	/* check if tx channel stopped */
	
	return 0;
}

static int stmmac_stop_chn(struct stmmac_priv *priv, int chn)
{
	int value;
	int limit;
	void *ioaddr;

	ioaddr = priv->ioaddr;
	
	/* stop DMA tx channel */
	value = readl(ioaddr + DMA_CHN_TX_CTL(chn));
	value &= ~DMA_TX_START;	//clear this bit to disable tx
	writel(value, ioaddr + DMA_CHN_TX_CTL(chn));

	/* check if MTL tx channel emptied*/
	limit = 100;
	while (limit--) {
		value = readl(ioaddr + MTL_TXQ_DBG(chn));
		if ((((value & MTL_TX_TRCSTS) >> 1) != 1) && (!(value & MTL_TX_TXQSTS)))
			break;
		mdelay(10);
	}
	if (limit < 0)
		return -EBUSY;
	
	/* stop MAC tx and rx */
	value = readl(ioaddr + GMAC_CONTROL);
	value &= ~(GMAC_CONTROL_RE | GMAC_CONTROL_TE);
	writel(value, ioaddr + GMAC_CONTROL);

	/* disable rx channel */
	limit = 100;
	while (limit--) {
		value = readl(ioaddr + MTL_TXQ_DBG(chn));

		/*make sure Rx Queue are empty*/
		if ((!(value & MTL_RX_PRXQ)) && (!(value & MTL_RX_RXQSTS))) {
			value = readl(ioaddr + DMA_CHN_RX_CTL(chn));
			value &= ~DMA_RX_START; //clear this bit to disable rx
			writel(value, ioaddr + DMA_CHN_RX_CTL(chn));
			break;
		}
		mdelay(10);
	}
	if (limit < 0)
		return -EBUSY;
	
	return 0;
}
static int stmmac_start_chn(struct stmmac_priv *priv, int chn)
{
	int value;
	void __iomem *ioaddr; 

	ioaddr = priv->ioaddr;
	
	/* start DMA tx channel */
	value = readl(ioaddr + DMA_CHN_TX_CTL(chn));
	value |= DMA_RX_START; //Set this bit to enable rx
	writel(value, ioaddr + DMA_CHN_TX_CTL(chn));

	/* Enable mac */
	stmmac_set_mac(priv->ioaddr, true);
	
	return 0;
}

static int stmmac_prepare_audio_tx_desc(union dma_desc *p, int is_fs,
						int len, int pkt_len, int csum_flag)
{
	struct tx_nrd_desc *nrd;	
	nrd = &(p->tx_desc.nrd);
	
	nrd->first_desc = is_fs;
	nrd->buf1_len = len;
	nrd->time_stamp_enable = 1;	//enable time stamp
	nrd->vlan_ins_rplc = 0x2;	//Insert a VLAN tag
	nrd->pkt_len = pkt_len;		//assume that packet length equal to skb->len
	nrd->slot_num = 1;			//125us

	if (is_fs) {
		nrd->crc_pad = 0;		//CRC and Pad Insertion
	}
	
	nrd->ctx_type = 0;			//Normal descriptor
		
	if (csum_flag)
		nrd->csum_ins_ctl = cic_full;
}
#endif
void stmmac_prepare_ptp_ctx_desc(struct gmac_tx_queue *tx_queue)
{
	union dma_desc *desc;
	unsigned int entry;

	/* build a context desc */
	entry = (tx_queue->cur_tx) % (tx_queue->dma_tx_size);
	desc = tx_queue->dma_tx + entry;
	*(tx_queue->tx_skbuff + entry) = NULL;
	*(tx_queue->tx_skbuff_dma + entry) = 0;

	desc->tx_desc.ctx.tx_pkt_ts_low = 0x11111111;		//test
	desc->tx_desc.ctx.tx_pkt_ts_high = 0x22222222;		//test
	desc->tx_desc.ctx.ostc = 1;
	desc->tx_desc.ctx.tcmmsv = 1;
	desc->tx_desc.ctx.ctx_type= 1;
	desc->tx_desc.ctx.own = 1;

	tx_queue->cur_tx++;
}

int stmmac_identify_ptp_pkt(struct sk_buff *skb)
{
	struct ethhdr *mac_header = eth_hdr(skb);

	if (!mac_header) {
		GMAC_ERR(("[%s]:mac header is NULL!\n", __func__));
		return -EINVAL;
	}
	
	if (htons(ETH_P_1588) == mac_header->h_proto) {
		return 1;
	} else {
		return 0;
	}
}

/**
 *  stmmac_xmit: Tx entry point of the driver
 *  @skb : the socket buffer
 *  @dev : device pointer
 *  Description : this is the tx entry point of the driver.
 *  It programs the chain or the ring and supports oversized frames
 *  and SG feature.
 */
//static 
netdev_tx_t stmmac_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	unsigned int txsize;
	unsigned int entry;
	int chn, csum_insertion = 0, is_jumbo = 0;
	int cnt;
	int ptp_flag;
	int nfrags = skb_shinfo(skb)->nr_frags;
	union dma_desc *desc, *first;
	unsigned int nopaged_len = skb_headlen(skb);
	unsigned long flags;
	struct gmac_tx_queue *tx_queue = NULL;

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	if(skb->spe_own) {		
		chn = TX_CHN_NET;	//all skb from SPE using chn 0
		ptp_flag = 0;		//all skb from SPE need reset mac header
	} else
#endif
	{
		chn = skb_get_queue_mapping(skb);
		if ((chn > TX_CHN_CLASS_A) || (chn < TX_CHN_NET)) {
			chn = TX_CHN_NET;
			priv->xstats.tx_chn_err++;
			GMAC_ERR(("[%s]error chn(%d),it will be reset to TX_CHN_NET!\n", __func__, chn)); 
		}
		
		ptp_flag = stmmac_identify_ptp_pkt(skb);
		if (ptp_flag < 0) {
			ptp_flag = 0;		//Reset ptp_flag to zero
			priv->xstats.tx_mac_header_err++;
		}
	}

	priv->xstats.enter_xmit[chn]++;
	tx_queue = GET_TX_QUEUE(priv, chn);

	/* if avb switch is closed, we don't handle AVB channel. */
	if ((!priv->avb_support) && (chn > TX_CHN_NET)) {
		dev_kfree_skb_any(skb);
		priv->xstats.tx_losscarrier++;
		return NETDEV_TX_OK;
	}
	
	/* Check if tx avail */
	if (unlikely(stmmac_tx_avail(priv, chn) < nfrags + 1)) {	
		if (!netif_subqueue_stopped(dev, skb)) {
			netif_stop_subqueue(dev, chn);
			
			/* This is a hard error, log it. */
			GMAC_ERR(("%s: chn[%d] Tx Ring full when queue awake\n", __func__, chn));
		}

		priv->xstats.tx_desc_full[chn]++;
		return NETDEV_TX_BUSY;
	}
	
	txsize = tx_queue->dma_tx_size;
	spin_lock_irqsave(&tx_queue->tx_lock, flags);

	if (ptp_flag){
		GMAC_PTP_DBG(("[%s]ptp packet cnt:%d\n", __func__, priv->ptp_cnt));
		stmmac_prepare_ptp_ctx_desc(tx_queue);
		priv->ptp_cnt++;
		priv->ptp_bytes += skb->len;
	}
	
	//if (tx_queue->tx_path_in_lpi_mode)
	//stmmac_disable_eee_mode(priv);
	csum_insertion = (skb->ip_summed == CHECKSUM_PARTIAL);
	entry = tx_queue->cur_tx % txsize;
	GMAC_DBG(("entry:%d\n", entry));

	if ((skb->len > ETH_FRAME_LEN) || nfrags)
			GMAC_TX_DBG(("%s: [entry %d], chn[%d]: skb addr %p len: %d" 
			 " nopagedlen: %d\n\tn_frags: %d - ip_summed: %d - %s gso\n"
			 "\ttx_count_frames %d\n", __func__, entry,chn, skb, skb->len,
			 nopaged_len, nfrags, skb->ip_summed, !skb_is_gso(skb) ? "isn't" : "is",
			 tx_queue->tx_count_frames));
	
	/* Get the desc from dma tx queue. */
	desc = tx_queue->dma_tx + entry;
	first = desc;
	if ((nfrags > 0) || (skb->len > ETH_FRAME_LEN))
		GMAC_TX_DBG(("\tskb len: %d, nopaged_len: %d,\n"
			 "\t\tn_frags: %d, ip_summed: %d\n",
			 skb->len, nopaged_len, nfrags, skb->ip_summed));

	tx_queue->tx_skbuff[entry] = skb;

	/* To program the descriptors according to the size of the frame */
	is_jumbo = priv->hw->ring->is_jumbo_frm(skb->len);
	//if (unlikely(is_jumbo))
		//entry = priv->hw->ring->jumbo_frm(priv, skb, csum_insertion, i);

	if (likely(!is_jumbo)) {		
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
		if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
			if(skb->spe_own){
				desc->tx_desc.nrd.buf1_phy_addr = spe_hook.get_skb_dma(skb);
			}
			else{
				desc->tx_desc.nrd.buf1_phy_addr = dma_map_single(priv->device, 
						skb->data, nopaged_len, DMA_TO_DEVICE);
			}
		}else
#endif
		{
			desc->tx_desc.nrd.buf1_phy_addr = dma_map_single(priv->device, 
						skb->data, nopaged_len, DMA_TO_DEVICE);
		}

		tx_queue->tx_skbuff_dma[entry] = desc->tx_desc.nrd.buf1_phy_addr;
		priv->hw->desc->prepare_tx_desc(desc, 1, nopaged_len, csum_insertion);
		priv->hw->desc->config_tx_slotnum(chn, desc);
	} else {	
		desc = first;
	}

	for (cnt = 0; cnt < nfrags; cnt++) {
		const skb_frag_t *frag = &skb_shinfo(skb)->frags[cnt];
		int len = skb_frag_size(frag);

		entry = (++tx_queue->cur_tx) % txsize;

		desc = tx_queue->dma_tx + entry;
		GMAC_DBG(("\tchn[%d],[entry %d] segment len: %d\n", chn, entry, len));
		desc->tx_desc.nrd.buf1_phy_addr = skb_frag_dma_map(priv->device, frag, 
										0, len, DMA_TO_DEVICE);
	
		tx_queue->tx_skbuff_dma[entry] = desc->tx_desc.nrd.buf1_phy_addr;
		tx_queue->tx_skbuff[entry] = NULL;
		priv->hw->desc->prepare_tx_desc(desc, 0, len, csum_insertion);
		priv->hw->desc->config_tx_slotnum(chn, desc);
		wmb();
		priv->hw->desc->set_tx_owner(desc);
		wmb();
	}

	priv->hw->desc->close_tx_desc(desc);

	/* According to the coalesce parameter the IC bit for the latest
	 * segment could be reset and the timer re-started to invoke the
	 * stmmac_tx function. This approach takes care about the fragments.
	 */
	tx_queue->tx_count_frames += nfrags + 1;
	if (tx_queue->tx_coal_frames > tx_queue->tx_count_frames) {
		priv->hw->desc->clear_tx_ic(desc);
		priv->xstats.tx_reset_ic_bit[chn]++;
		GMAC_DBG(("\t[entry %d]: tx_count_frames %d\n", entry, tx_queue->tx_count_frames));
		mod_timer(&(tx_queue->txtimer),STMMAC_COAL_TIMER(tx_queue->tx_coal_timer));
	} else
		tx_queue->tx_count_frames = 0;

	/* To avoid raise condition */
	priv->hw->desc->set_tx_owner(first);
	wmb();

	tx_queue->cur_tx++;

	GMAC_TX_DBG(("%s: chn[%d], curr %d dirty=%d entry=%d, first=%p, nfrags=%d\n",
			__func__, chn, (tx_queue->cur_tx % txsize),
			(tx_queue->dirty_tx % txsize), entry, first, nfrags));

	stmmac_display_ring((void *)tx_queue->dma_tx, txsize, GMAC_LEVEL_RING_DBG);
	GMAC_TX_DBG((">>> frame to be transmitted: "));
	print_pkt(skb->data, 64, GMAC_LEVEL_PTP_DBG | GMAC_LEVEL_TX_DBG);

	if (unlikely(stmmac_tx_avail(priv, chn) <= (MAX_SKB_FRAGS + 1))) {
		GMAC_DBG(("%s: stop transmitted packets\n", __func__));
		netif_stop_subqueue(dev, chn);
		priv->xstats.xmit_td_full[chn]++;
		priv->xstats.tx_avail[chn] = stmmac_tx_avail(priv, chn);
	}

	dev->stats.tx_bytes += skb->len;
	tx_queue->tx_bytes += skb->len;
	tx_queue->tx_pkt_cnt++;

	if (priv->dma_cap.systime_source && tx_queue->hwts_tx_en) {
		if (skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP){
			/* declare that device is doing timestamping */
			skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
			priv->hw->desc->enable_tx_timestamp(first);
		}
	}

	if (!tx_queue->hwts_tx_en)
		skb_tx_timestamp(skb);
	
	priv->hw->dma->enable_dma_transmission(chn, entry);

	spin_unlock_irqrestore(&tx_queue->tx_lock, flags);		

	return NETDEV_TX_OK;
}

/**
 * stmmac_rx_refill: refill used skb preallocated buffers
 * @priv: driver private structure
 * Description : this is to reallocate the skb for the reception process
 * that is based on zero-copy.
 */
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
static inline void stmmac_rx_refill(struct stmmac_priv *priv)
{
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, RX_CHN);
	unsigned int rxsize = rx_queue->dma_rx_size;
	int bfsize = priv->dma_buf_sz;
	unsigned int index = 0;
	dma_addr_t dma;

	for (; rx_queue->cur_rx - rx_queue->dirty_rx > 0; rx_queue->dirty_rx++) {
		unsigned int entry = rx_queue->dirty_rx % rxsize;
		union dma_desc *p;

		p = rx_queue->dma_rx + entry;
		if (likely(rx_queue->rx_skbuff[entry] == NULL)) {
			struct sk_buff *skb;
            if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
                skb = skb_dequeue(&priv->free_q);
                if (unlikely(skb == NULL))
            				break;
                rx_queue->rx_skbuff[entry] = skb;
                dma = spe_hook.get_skb_dma(skb);
                p->rx_desc.nrd.buf1_phy_addr = dma;
            }else {
				skb = netdev_alloc_skb_ip_align(priv->dev, bfsize);
				if (unlikely(skb == NULL))
					break;

				rx_queue->rx_skbuff[entry] = skb;
				rx_queue->rx_skbuff_dma[entry] =
				    dma_map_single(priv->device, skb->data, bfsize,
						   DMA_FROM_DEVICE);

				p->rx_desc.nrd.buf1_phy_addr= rx_queue->rx_skbuff_dma[entry];
			}

			RX_DBG(KERN_INFO "\trefill entry #%d\n", entry);
		}
		wmb();
		p->rx_desc.nrd.buf1_addr_valid = 1;
		priv->hw->desc->set_rx_owner(p);
		priv->hw->desc->set_rx_ioc(p,rx_irq_flag);
		wmb();

		index = (entry + rxsize - 1) % rxsize;
		priv->hw->dma->set_rx_tail_ptr(index, RX_CHN);
	}
}
#else
static inline void stmmac_rx_refill(struct stmmac_priv *priv)
{
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, RX_CHN);
	unsigned int rxsize = rx_queue->dma_rx_size;
	int bfsize = priv->dma_buf_sz;
	unsigned int index = 0;

	for (; rx_queue->cur_rx - rx_queue->dirty_rx > 0; rx_queue->dirty_rx++) {
		unsigned int entry = rx_queue->dirty_rx % rxsize;
		union dma_desc *p;

		p = rx_queue->dma_rx + entry;
		if (likely(rx_queue->rx_skbuff[entry] == NULL)) {
			struct sk_buff *skb;
			skb = netdev_alloc_skb_ip_align(priv->dev, bfsize);

			if (unlikely(skb == NULL))
				break;

			rx_queue->rx_skbuff[entry] = skb;
			rx_queue->rx_skbuff_dma[entry] =
			    dma_map_single(priv->device, skb->data, bfsize,
					   DMA_FROM_DEVICE);

			p->rx_desc.nrd.buf1_phy_addr= rx_queue->rx_skbuff_dma[entry];
			RX_DBG(KERN_INFO "\trefill entry #%d\n", entry);
		}
		wmb();
		p->rx_desc.nrd.buf1_addr_valid = 1;
		priv->hw->desc->set_rx_owner(p);
		priv->hw->desc->set_rx_ioc(p,rx_irq_flag);
		wmb();

		index = (entry + rxsize - 1) % rxsize;
		priv->hw->dma->set_rx_tail_ptr(index, RX_CHN);
	}
}
#endif

/**
 * stmmac_rx_refill: refill used skb preallocated buffers
 * @priv: driver private structure
 * @limit: napi bugget.
 * Description :  this the function called by the napi poll method.
 * It gets all the frames inside the ring.
 */
static int stmmac_rx(struct stmmac_priv *priv, int limit)
{
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(priv, RX_CHN);
	struct gmac_rx_stat *rx_stat;
	unsigned int rxsize = rx_queue->dma_rx_size;
	unsigned int entry = rx_queue->cur_rx % rxsize;
	unsigned int next_entry;
	unsigned int count = 0;
	int chn;
	int coe = priv->plat->rx_coe;
	struct sk_buff *last_skb = NULL;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	unsigned long flags;
	int spe_enable = 0;
#endif
	GMAC_RING_DBG((">>> stmmac_rx: descriptor ring:\n"));
	stmmac_display_ring((void *)rx_queue->dma_rx, rxsize, GMAC_LEVEL_RING_DBG);
	
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	spe_enable = spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode());
	if(spe_enable){
		spin_lock_irqsave(&priv->rx_lock,flags);
	}
#endif

	while (count < limit) {
		int status;
		union dma_desc *p;

		p = rx_queue->dma_rx + entry;
		if (priv->hw->desc->get_rx_owner(p))
			break;

		count++;

		next_entry = (++rx_queue->cur_rx) % rxsize;
		prefetch(rx_queue->dma_rx + next_entry);

		/* read the status of the incoming frame */
		status = priv->hw->desc->rx_status(&priv->dev->stats,
						   &priv->xstats, p);
		
		if (unlikely(status == discard_frame)) {
			priv->dev->stats.rx_errors++;
			rx_queue->rx_skbuff[entry] = NULL;
			
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
		if (!spe_enable) {
			dma_unmap_single(priv->device,
						 rx_queue->rx_skbuff_dma[entry],
						 priv->dma_buf_sz, DMA_FROM_DEVICE);
		}
#else
		dma_unmap_single(priv->device,
						 rx_queue->rx_skbuff_dma[entry],
						 priv->dma_buf_sz, DMA_FROM_DEVICE);
#endif
		} else {
			struct sk_buff *skb;
			int frame_len;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
            dma_addr_t dma;
            int ret;
#endif
			frame_len = priv->hw->desc->get_rx_frame_len(p);

			/* ACS is set; GMAC core strips PAD/FCS for IEEE 802.3
			 * Type frames (LLC/LLC-SNAP)
			 */
			if (unlikely(status != llc_snap))
				frame_len -= ETH_FCS_LEN;
#ifdef STMMAC_RX_DEBUG
			if (frame_len > ETH_FRAME_LEN)
				GMAC_DBG(("\tRX frame size %d, COE status: %d\n",
					 frame_len, status));
#endif
			skb = rx_queue->rx_skbuff[entry];
			if (unlikely(!skb)) {
				GMAC_ERR(("%s: Inconsistent Rx descriptor chain\n",
				       priv->dev->name));
				priv->dev->stats.rx_dropped++;
				break;
			}
			prefetch(skb->data - NET_IP_ALIGN);
			rx_queue->rx_skbuff[entry] = NULL;

			//stmmac_get_rx_hwtstamp(priv, entry, skb);

			skb_put(skb, frame_len);
			chn = gmac_get_skb_chn(skb, priv);
			if (chn >= TX_CHN_NET) {		
				/* for the specific channel*/
				rx_stat = GET_RX_STAT(priv, chn);
				rx_stat->rx_bytes += skb->len;
				rx_stat->rx_cnt++;
			}
			gmac_rx_rate();
			
			/*all of packets*/
			rx_queue->rx_bytes += skb->len;
			rx_queue->rx_pkt_cnt++;
				
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
            if(spe_enable){
            }else
#endif
			{
			dma_unmap_single(priv->device,
					 rx_queue->rx_skbuff_dma[entry],
					 priv->dma_buf_sz, DMA_FROM_DEVICE);
			}

			GMAC_RX_DBG(("%s: frame received (%dbytes)",__func__, frame_len));
			print_pkt(skb->data, 64, GMAC_LEVEL_RX_DBG | GMAC_LEVEL_PTP_DBG);

			if (chn > TX_CHN_NET) {
				entry = next_entry;
				dev_kfree_skb_any(skb);
				continue;
			}

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
			if(spe_enable){
				dma = spe_hook.get_skb_dma(skb);
				ret = spe_hook.td_config(priv->portno, skb, dma, spe_l3_bottom, 0);
				if (ret == SPE_ERR_TDFULL) {
					GMAC_DBG(("%s: td full\n",__func__));
					priv->xstats.rx_collision++;
					stmmac_finish_td(priv->portno, skb, 0);
				}
				rx_queue->rx_skbuff[entry]=NULL;
			}else
#endif
			{
				skb->protocol = eth_type_trans(skb, priv->dev);

				if(last_skb){
                	if (unlikely(!coe)) {
	                    /* No RX COE for old mac10/100 devices */
	                    skb_checksum_none_assert(last_skb);
	                } else {
	                    last_skb->ip_summed = CHECKSUM_UNNECESSARY;
                    }

					//#ifdef CONFIG_GMAC_TEST
					//local_receive_skb(count-1, last_skb);
					//#else
					napi_gro_receive(&priv->napi, last_skb);
					//#endif
                }
#ifdef CONFIG_BALONG_SKB_MEMBER
				skb->psh = 0;
#endif
            	last_skb = skb;
			}
            
            gmac_status->rx_skb_count++;

			priv->dev->stats.rx_packets++;
			priv->dev->stats.rx_bytes += frame_len;
		}
		entry = next_entry;
	}
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	if(spe_enable){
		spin_unlock_irqrestore(&priv->rx_lock,flags);
	}
#endif
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

		//#ifdef CONFIG_GMAC_TEST
		//local_receive_skb(count-1, last_skb);
		//#else
		napi_gro_receive(&priv->napi, last_skb);
		//#endif
    } 

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	/* refill is moved to stmmac_finish_td in spe version */
	if(!spe_enable){
		stmmac_rx_refill(priv);	//SPE is enabled, and in stick mode
	}
#else
	stmmac_rx_refill(priv);		//SPE is closed.
#endif

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

	work_done = stmmac_rx(priv, budget);
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
	int i;
	struct stmmac_priv *priv = netdev_priv(dev);

	/* Clear Tx resources and restart transmitting again */
	for (i = TX_CHN_NET; i < priv->tx_queue_cnt; i++) {
		stmmac_tx_err(priv, i);
	}
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

static unsigned short stmmac_select_queue(struct net_device *dev,  struct sk_buff *skb)
{
	unsigned int vlan_qos;
	unsigned short queue_index = 0;
	struct stmmac_priv *priv = netdev_priv(dev);
	struct vlan_ethhdr *mac_header = NULL;

	mac_header = vlan_eth_hdr(skb);
	if (!mac_header) {
		GMAC_ERR(("[%s]:mac header is NULL!\n", __func__));
		return -EINVAL;
	}
	
    if (htons(ETH_P_8021Q) == mac_header->h_vlan_proto) { //Only upport single VLAN tag.	
        vlan_qos = (mac_header->h_vlan_TCI & VLAN_PRIO_MASK) >> VLAN_PRIO_SHIFT;
		if (vlan_qos == priv->pcp_class_a) {
			queue_index = TX_CHN_CLASS_A;
		} else if (vlan_qos == priv->pcp_class_b) {
			queue_index = TX_CHN_CLASS_B;
		} else {
			GMAC_ERR(("%s: wrong vlan_qos(%u)!\n", __func__, vlan_qos));
			return -EINVAL;
		}
    } else {
        queue_index =  TX_CHN_NET;
    }

    return queue_index;
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
	int max_mtu;

	if (netif_running(dev)) {
		GMAC_ERR(("%s: must be stopped to change its MTU\n", dev->name));
		return -EBUSY;
	}

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
	int ret;
	int i;
	struct net_device *dev = (struct net_device *)dev_id;
	struct stmmac_priv *priv = netdev_priv(dev);
	void __iomem * ioaddr = (void __iomem *)dev->base_addr;

	if (unlikely(!dev)) {
		GMAC_ERR(("%s: invalid dev pointer\n", __func__));
		return IRQ_NONE;
	}

	/* This register always be zero! */
	//isr_status = priv->hw->dma->get_dma_isr(ioaddr);
	//if (!isr_status) {
		//return IRQ_NONE;
	//}
	//
	//
	//mac_isr = GET_VALUE(isr_status, DMA_ISR_MAC_LPOS, DMA_ISR_MAC_HPOS);
	if (priv->plat->has_gmac/* && mac_isr */) {
		ret = priv->hw->mac->handle_irq(ioaddr, &priv->xstats);
		if (ret) {
			GMAC_ERR(("%s: Interrupt unclear! ret:0x%08x\n", __func__, ret));	
			return IRQ_NONE;
		}
	}

	/* To handle MTL interrupts */
	//if (GET_VALUE(isr_status, DMA_ISR_MTL_LPOS, DMA_ISR_MTL_HPOS)) {
		stmmac_mtl_interrupt(priv);
	//}
	
	/* To handle DMA interrupts */
	for (i = 0; i < priv->tx_queue_cnt; i++) {
		//if (GET_VALUE(isr_status, DMA_ISR_DCIS_LPOS(i), DMA_ISR_DCIS_HPOS(i)))
			stmmac_dma_interrupt(priv, i);
	}

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
	int ret=0;
	struct stmmac_priv *priv = netdev_priv(dev);

	printk("in %s \n",__func__);
	
	ret = eth_mac_addr(dev, addr);
	if(ret)
	{
		printk("eth_mac_addr return %d \n",ret);
		return ret;
	}

	/* set mac address to register */
	priv->hw->mac->set_umac_addr(priv->ioaddr, dev->dev_addr, 0);

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		spe_hook.port_ioctl(priv->portno,spe_port_ioctl_set_mac,(int)dev->dev_addr);
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
		if (!priv->phydev)
			return -EINVAL;
		ret = phy_mii_ioctl(priv->phydev, rq, cmd);
		break;
		
	case SIOCSHWTSTAMP:
		ret = stmmac_hwtstamp_ioctl(dev, rq);
		break;
		
	case STMMAC_CBS_REQUEST_BANDWIDTH:
 	case STMMAC_CBS_RELEASE_BANDWIDTH:
 	case STMMAC_CBS_QUERY_CHN:
 	case STMMAC_CBS_CONFIG_ALG:
		ret = stmmac_cbs_ioctl(dev, rq, cmd);
		break;
		
	default:
		break;
	}

	return ret;
}

#ifdef CONFIG_NEW_STMMAC_DEBUG_FS
static struct dentry *stmmac_fs_dir;
static struct dentry *stmmac_rings_status;
static struct dentry *stmmac_dma_cap;

static void sysfs_display_ring(void *head, int size, struct seq_file *seq)
{
	int i;
	union dma_desc *p = (union dma_desc *)head;

	for (i = 0; i < size; i++) {
		u32 *x;
		x = (u32 *)p;
		seq_printf(seq, "%d [0x%x]: 0x%x 0x%x 0x%x 0x%x\n",
			   i, (unsigned int)virt_to_phys(p),
			   *x, *(x + 1), *(x + 2), *(x + 3));
		p++;
		seq_printf(seq, "\n");
	}
}

static int stmmac_sysfs_ring_read(struct seq_file *seq, void *v)
{
	unsigned int chn;
	struct net_device *dev = seq->private;
	struct stmmac_priv *priv = netdev_priv(dev);
	struct gmac_rx_queue *rx_queue = NULL;
	struct gmac_tx_queue *tx_queue = NULL;

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		seq_printf(seq, "Chn%d RX descriptor ring:\n", chn);
		rx_queue = GET_RX_QUEUE(priv, chn);
		sysfs_display_ring((void *)rx_queue->dma_rx, rx_queue->dma_rx_size, seq);
	}

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		seq_printf(seq, "Chn%d TX descriptor ring:\n", chn);
		tx_queue = GET_TX_QUEUE(priv, chn);
		sysfs_display_ring((void *)tx_queue->dma_tx, tx_queue->dma_tx_size, seq);
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
	char *option;
	unsigned int number;
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
	seq_printf(seq, "\tPCS (TBI/SGMII/RTBI PHY interfatces): %s\n",
		   (priv->dma_cap.pcs) ? "Y" : "N");
	seq_printf(seq, "\tVlan Hash Filter: %s\n",
		   (priv->dma_cap.vlan_hash_filter) ? "Y" : "N");
	seq_printf(seq, "\tSMA (MDIO) Interface: %s\n",
		   (priv->dma_cap.sma_mdio) ? "Y" : "N");
	seq_printf(seq, "\tPMT Remote wake up: %s\n",
		   (priv->dma_cap.pmt_remote_wake_up) ? "Y" : "N");
	seq_printf(seq, "\tPMT Magic Frame: %s\n",
		   (priv->dma_cap.pmt_magic_frame) ? "Y" : "N");
	seq_printf(seq, "\tRMON module: %s\n",
		   (priv->dma_cap.rmon) ? "Y" : "N");
	seq_printf(seq, "\tARP Offload: %s\n",
		   (priv->dma_cap.arp_offload) ? "Y" : "N");
	seq_printf(seq, "\tIEEE 1588-2002 Time Stamp: %s\n",
		   (priv->dma_cap.time_stamp) ? "Y" : "N");
	seq_printf(seq, "\t802.3az - Energy-Efficient Ethernet (EEE) %s\n",
		   (priv->dma_cap.eee) ? "Y" : "N");
	seq_printf(seq, "\tChecksum Offload in TX: %s\n",
		   (priv->dma_cap.tx_coe) ? "Y" : "N");
	seq_printf(seq, "\tChecksum Offload in RX: %s\n",
		   (priv->dma_cap.rx_coe) ? "Y" : "N");
	seq_printf(seq, "\tNumber of Additional MAC addresses: %d\n",
		   priv->dma_cap.multi_addr);
	seq_printf(seq, "\tMAC address 32: %s\n",
		   (priv->dma_cap.multi_addr32) ? "Y" : "N");
	seq_printf(seq, "\tMAC address 64: %s\n",
		   (priv->dma_cap.multi_addr64) ? "Y" : "N");
	
	if (1 == priv->dma_cap.systime_source) {
		option = "Internal";			
	} else if (3 == priv->dma_cap.systime_source) {
		option = "Both";	
	} else {
		option = "Internal";			
	}
	switch (priv->dma_cap.systime_source) {
		case 1:
			option = "Internal";
			break;

		case 2:
			option = "External";
			break;			

		case 3:
			option = "both";
			break;		

		default:
			option = "reserved";
			break;
	}
	
	seq_printf(seq, "\tSystime Source address: %s\n", option);

	seq_printf(seq, "\tSource Address or VLAN Insertion: %s\n",
		   (priv->dma_cap.sa_vlan_ins) ? "Y" : "N");

	switch (priv->dma_cap.phy_mode) {
		case 0:
			option = "GMII or MII";
			break;
			
		case 1:
			option = "RGMII";
			break;
			
		case 2:
			option = "SGMII";
			break;
			
		case 3:
			option = "TBII";
			break;

		case 4:
			option = "RMII";
			break;
			
		case 5:
			option = "RTBII";
			break;
			
		case 6:
			option = "SMII";
			break;
			
		case 7:
			option = "RevMII";
			break;

		default:
			option = "Resevered";
			break;
	}
	seq_printf(seq, "\tPHY Mode: %s\n", option);

	if (priv->dma_cap.rx_fifo_size > 11) {
		seq_printf(seq, "\tMTL Rx Fifo Size: reserved\n");
	} else {
		number = (1 << (priv->dma_cap.rx_fifo_size + 7));
		seq_printf(seq, "\tMTL Rx Fifo Size: %dbytes\n", number);
	}

	if (priv->dma_cap.tx_fifo_size > 11) {
		seq_printf(seq, "\tMTL Rx Fifo Size: reserved\n");
	} else {
		number = (1 << (priv->dma_cap.tx_fifo_size + 7));
		seq_printf(seq, "\tMTL Rx Fifo Size: %dbytes\n", number);
	}
	
	seq_printf(seq, "\tOne step Time Stamp: %s\n",
		   (priv->dma_cap.one_step_ts) ? "Y" : "N");
	seq_printf(seq, "\tPTP Offload: %s\n", 
			(priv->dma_cap.ptp_offload) ? "Y" : "N");
	seq_printf(seq, "\tIEEE 1588 High Word Reister: %s\n", 
			(priv->dma_cap.high_word_reg) ? "Y" : "N");

	if (priv->dma_cap.addr_width > 2) {
		seq_printf(seq, "\tAddress Width: reserved\n");
	} else {
		number = (priv->dma_cap.addr_width * 8) + 32;
		seq_printf(seq, "\tAddress Width: %d\n", number);
	}

	seq_printf(seq, "\tDCB Feature: %s\n", 
			(priv->dma_cap.dcb_enable) ? "Y" : "N");
	seq_printf(seq, "\tSplit Header Feature: %s\n", 
			(priv->dma_cap.split_hdr) ? "Y" : "N");
	seq_printf(seq, "\tTCP Segmentation offload: %s\n", 
			(priv->dma_cap.tso_en) ? "Y" : "N");
	seq_printf(seq, "\tDebug memory interface: %s\n", 
			(priv->dma_cap.debug_mem_if) ? "Y" : "N");
	seq_printf(seq, "\tAV feature: %s\n", 
			(priv->dma_cap.av_en) ? "Y" : "N");

	if (priv->dma_cap.hash_table_size) {
		number = (1 << (priv->dma_cap.hash_table_size + 5));
		seq_printf(seq, "\tHash Table Size: %d\n", number);
	}

	if (!(priv->dma_cap.l3l4_total_num)) {
		seq_printf(seq, "\tNo L3 or L4 filter!\n");
	} else {
		number = priv->dma_cap.l3l4_total_num;
		seq_printf(seq, "\tTotal Number L3 or L4 Filters: %d\n", number);
	}
	
	number = priv->dma_cap.num_rx_queue + 1;
	seq_printf(seq, "\tNumber of MTL Rx queue:%d\n", number);

	number = priv->dma_cap.num_tx_queue + 1;
	seq_printf(seq, "\tNumber of MTL Tx queue:%d\n", number);

	number = priv->dma_cap.num_rx_channel + 1;
	seq_printf(seq, "\tNumber of DMA Rx Channel:%d\n", number);

	number = priv->dma_cap.num_tx_channel + 1;
	seq_printf(seq, "\tNumber of DMA Tx Channel:%d\n", number);

	if (!(priv->dma_cap.num_pps_output)) {
		seq_printf(seq, "\tNo PPS output!\n");
	} else if (4 < priv->dma_cap.num_pps_output) {
		seq_printf(seq, "\tPPS output: reserved!\n");
	} else {
		seq_printf(seq, "\tNumber of PPS output:%d\n", 
			priv->dma_cap.num_pps_output);
	}

	if (!(priv->dma_cap.num_aux_snap)) {
		seq_printf(seq, "\tNo Auxiliary Snapshot input!\n");
	} else if (4 < priv->dma_cap.num_aux_snap) {
		seq_printf(seq, "\tAuxiliary Snapshot input: reserved!\n");
	} else {
		seq_printf(seq, "\tNumber of Auxiliary Snapshot input:%d\n", 
			priv->dma_cap.num_aux_snap);
	}
	
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
#endif /* CONFIG_NEW_STMMAC_DEBUG_FS */

static const struct net_device_ops stmmac_netdev_ops = {
	.ndo_open = stmmac_open,
	.ndo_start_xmit = stmmac_xmit,
	.ndo_stop = stmmac_release,
	.ndo_change_mtu = stmmac_change_mtu,
	.ndo_fix_features = stmmac_fix_features,
	.ndo_set_rx_mode = stmmac_set_rx_mode,
	.ndo_tx_timeout = stmmac_tx_timeout,
	.ndo_do_ioctl = stmmac_ioctl,
	.ndo_set_config = stmmac_config,
	.ndo_select_queue = stmmac_select_queue,
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
	struct mac_device_info *mac = NULL;
	int hw_cap[DMA_CAP_NUM];

	/* Identify the MAC HW device */
	if (priv->plat->has_gmac) {
		priv->dev->priv_flags |= IFF_UNICAST_FLT;
		mac = dwmac1000_setup(priv->ioaddr);
	} else {
		//mac = dwmac100_setup(priv->ioaddr);
	}
	if (!mac)
		return -ENOMEM;

	priv->hw = mac;

	/* Get and dump the chip ID */
	priv->synopsys_id = stmmac_get_synopsys_id(priv);

	/* Get the HW capability (new GMAC newer than 3.50a) */
	hw_cap[0] = stmmac_get_hw_features(priv, &hw_cap[1], &hw_cap[2]);
	if (hw_cap[0] && hw_cap[1] && hw_cap[2]) {
		GMAC_INFO((" DMA HW capability register supported"));

		/* We can override some gmac/dma configuration fields: e.g.
		 *  tx_coe (e.g. that are passed through the
		 * platform) with the values from the HW capability
		 * register (if supported).
		 */		 
		priv->hw_cap_support = 1;
		priv->plat->pmt = priv->dma_cap.pmt_remote_wake_up;
		priv->plat->tx_coe = priv->dma_cap.tx_coe;
		if ((priv->dma_cap.rx_coe)) {
			priv->plat->rx_coe = STMMAC_RX_COE;
		}
		
	} else {
		GMAC_INFO((" No HW DMA feature register supported"));
	}

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
	int chn;
	struct net_device *ndev = NULL;
	struct stmmac_priv *priv;
	struct gmac_rx_queue *rx_queue;
	struct gmac_tx_queue *tx_queue;

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
    int portno;
    struct spe_port_attr stmmac_attr = {0};
#endif
	
	/* three tx queues, one rx queue*/
	ndev = alloc_etherdev_mqs(sizeof(struct stmmac_priv),TX_CHN_NR, RX_CHN_NR);	
	if (!ndev)
		return NULL;

	SET_NETDEV_DEV(ndev, device);

	priv = netdev_priv(ndev);
	priv->device = device;
	priv->dev = ndev;
	priv->pcp_class_a = 5;		//default set
	priv->pcp_class_b = 4;		//default set

	/*using for debug*/
	gmac_priv = priv;
	
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
	snprintf(ndev->name, sizeof(ndev->name), "%s%%d", "gmac");// output is gmac%d

	ret = gmac_alloc_queue_struct(priv);
	if (ret) {
		GMAC_ERR(("%s: Alloc queue failed (error: %d)\n", __func__, ret));
		goto error_free_mac;
	}

	ret = gmac_alloc_rx_stat(priv);
	if (ret) {
		GMAC_ERR (("%s: Alloc rx stat failed (error: %d)\n", __func__, ret));
		goto error_free_queue_struct;
	}

	/* using for debug */
	tx_queue_class_a = GET_TX_QUEUE(priv,TX_CHN_CLASS_A);
	tx_queue_class_b = GET_TX_QUEUE(priv,TX_CHN_CLASS_B);
	
    if (get_gmac_addr(stmmac_mac, priv->dev->dev_addr)) {
            GMAC_INFO(("%s:using random ethernet address\n", __func__));
    }
	ndev->netdev_ops = &stmmac_netdev_ops;
	ndev->flags |= IFF_ALLMULTI;

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
		for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
			rx_queue = GET_RX_QUEUE(priv, chn);
			rx_queue->use_riwt = 1;
		}
		pr_info(" Enable RX Mitigation via HW Watchdog Timer\n");
	}

	netif_napi_add(ndev, &priv->napi, stmmac_poll, 64);
	spin_lock_init(&priv->lock);

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue =  GET_TX_QUEUE(priv, chn);
		spin_lock_init(&(tx_queue->tx_lock));
	}
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
	
	/* Alloc SPE port, configure parameters */
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
    if(spe_hook.is_enable && spe_hook.is_enable()){
		stmmac_attr.enc_type = spe_enc_none;
        stmmac_attr.attach_brg = spe_attach_brg_normal;
        stmmac_attr.rd_depth = 256;
        stmmac_attr.td_depth = 256;/*same as rx */
        stmmac_attr.rd_skb_size = BUF_SIZE_1_8kiB;//to save memory
        stmmac_attr.rd_skb_num = 256;
        stmmac_attr.desc_ops.finish_rd = stmmac_finish_rd;
        stmmac_attr.desc_ops.finish_td = stmmac_finish_td;
        stmmac_attr.net = ndev;
        portno = spe_hook.port_alloc(&stmmac_attr);
        priv->portno = portno;
    }
#endif

	return priv;

error_mdio_register:
	unregister_netdev(ndev);
error_netdev_register:
	netif_napi_del(&priv->napi);
	gmac_free_rx_stat(priv);
error_free_queue_struct:
	gmac_free_queue_struct(priv);
error_free_mac:
	gmac_free_mac(priv);
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
	int chn;
	struct stmmac_priv *priv = netdev_priv(ndev);

	pr_info("%s:\n\tremoving driver", __func__);

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		priv->hw->dma->stop_tx(priv->ioaddr, chn);
	}

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {
		priv->hw->dma->stop_rx(priv->ioaddr, chn);
	}
	
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
	if(spe_hook.is_enable && spe_hook.is_enable()&&(spe_mode_normal==spe_hook.mode())){
		(void)spe_hook.port_free(priv->portno);
	}
#endif

	stmmac_set_mac(priv->ioaddr, false);
	if (priv->pcs != STMMAC_PCS_RGMII && priv->pcs != STMMAC_PCS_TBI &&
	    priv->pcs != STMMAC_PCS_RTBI)
		stmmac_mdio_unregister(ndev);
	netif_carrier_off(ndev);
	unregister_netdev(ndev);
	gmac_free_rx_stat(priv);
	gmac_free_queue_struct(priv);
	gmac_free_mac(priv);
	free_netdev(ndev);

	return 0;
}

void gmac_show_xstats(void)
{
    printk("stmmac_xmit td full:%d\n", (int)gmac_status->tx_desc_full);
    printk("stmmac_xmit td full2:%d\n", (int)gmac_status->xmit_td_full);
    printk("stmmac_release net stopped:%d\n", (int)gmac_status->gmac_release);
    printk("stmmac_suspend net stopped:%d\n", (int)gmac_status->gmac_suspend);
    printk("enter xmit:%d\n", (int)gmac_status->enter_xmit);
    printk("tx avail:%d\n", (int)gmac_status->tx_avail);
    printk("rx psh count:%d\n",(int)gmac_status->rx_psh_count);
    printk("rx skb count:%d\n",(int)gmac_status->rx_skb_count);
}

void gmac_show_tx_channel(int chn)
{
	struct gmac_tx_queue *tx_queue = GET_TX_QUEUE(gmac_priv, chn);

	printk("[tx queue %d]cur:%d, dirty:%d\n", chn, 
						(tx_queue->cur_tx % tx_queue->dma_tx_size),
						(tx_queue->dirty_tx % tx_queue->dma_tx_size));

	printk("[tx queue %d]TX bytes:%d, TX count:%d\n", chn,
						tx_queue->tx_bytes, tx_queue->tx_pkt_cnt);
}

void gmac_show_rx_chn_stat(int chn)
{
	struct gmac_rx_stat *rx_stat = GET_RX_STAT(gmac_priv, chn);

	printk("[received chn%d]bytes:%d, count:%d\n", chn, rx_stat->rx_bytes,
											rx_stat->rx_cnt);
}

void gmac_show_rx_channel(void)
{
	int chn;
	struct gmac_rx_queue *rx_queue = GET_RX_QUEUE(gmac_priv, 0);

	printk("[rx queue %d]cur:%d, dirty:%d\n", 0, 
						(rx_queue->cur_rx % rx_queue->dma_rx_size),
						(rx_queue->dirty_rx % rx_queue->dma_rx_size));

	printk("[rx queue %d]RX bytes:%d, RX count:%d\n", 0,
						rx_queue->rx_bytes, rx_queue->rx_pkt_cnt);

	for (chn = 0; chn < gmac_priv->tx_queue_cnt; chn++) {
		gmac_show_rx_chn_stat(chn);
	}
}

void gmac_show_all_channel(void)
{
	int chn;

	for (chn = 0; chn < gmac_priv->tx_queue_cnt; chn++) {
		gmac_show_tx_channel(chn);
	}

	gmac_show_rx_channel();	
}

void gmac_help(void)
{
	printk("show tx channel n:      gmac_show_tx_channel n\n");
	printk("show rx channel:        gmac_show_rx_channel\n");
	printk("show rx ch n stat:      gmac_show_rx_chn_stat n\n");
	printk("show all channels:      gmac_show_all_channel\n");
	printk("show gmac test help:    gmac_test_help\n");
	printk("show gmac xstats:       gmac_show_xstats\n");
}

#ifdef CONFIG_PM
int stmmac_suspend(struct net_device *ndev)
{
	struct stmmac_priv *priv = netdev_priv(ndev);
	unsigned long flags;
	int chn;

	if (!ndev || !netif_running(ndev)) {
		printk("[%s]suspend finished(no up)!\n", __func__);
		return 0;
	}

	if (priv->phydev)
		phy_stop(priv->phydev);

	spin_lock_irqsave(&priv->lock, flags);

	netif_device_detach(ndev);
	netif_tx_stop_all_queues(ndev);
	priv->xstats.gmac_suspend++;

	napi_disable(&priv->napi);

	/* Stop TX/RX DMA */
	for (chn = 0;chn < priv->tx_queue_cnt; chn++) {
		priv->hw->dma->stop_tx(priv->ioaddr, chn);
	}

	for (chn = 0;chn < priv->rx_queue_cnt; chn++) {	
		priv->hw->dma->stop_rx(priv->ioaddr, chn);
	}

	stmmac_clear_descriptors(priv);

	/* Enable Power down mode by programming the PMT regs */
	if (device_may_wakeup(priv->device))
		priv->hw->mac->pmt(priv->ioaddr, priv->wolopts);
	else {
		stmmac_set_mac(priv->ioaddr, false);
	}
	
	spin_unlock_irqrestore(&priv->lock, flags);

	/* disable clock */
	stmmac_clk_disable(priv->device);
	printk("[%s]suspend finished(after up)!\n", __func__);
	
	return 0;
}

int stmmac_resume(struct net_device *ndev)
{
	struct stmmac_priv *priv = netdev_priv(ndev);
	unsigned long flags;
	int chn;

	if (!netif_running(ndev)) {
		printk("[%s]resume finished(no up)!\n", __func__);
		return 0;
	}

	spin_lock_irqsave(&priv->lock, flags);

	/* Power Down bit, into the PM register, is cleared
	 * automatically as soon as a magic packet or a Wake-up frame
	 * is received. Anyway, it's better to manually clear
	 * this bit because it can generate problems while resuming
	 * from another devices (e.g. serial console).
	 */
	if (device_may_wakeup(priv->device))
		priv->hw->mac->pmt(priv->ioaddr, 0);	//dwmac1000_pmt

	netif_device_attach(ndev);

	/* open clock */
	stmmac_clk_enable(priv->device);
	
	/* Enable the MAC and DMA */
	stmmac_set_mac(priv->ioaddr, true);

	for (chn = 0;chn < priv->tx_queue_cnt; chn++) {
		priv->hw->dma->start_tx(priv->ioaddr, chn);
	}

	for (chn = 0;chn < priv->rx_queue_cnt; chn++) {	
		priv->hw->dma->start_rx(priv->ioaddr, chn);
	}

	napi_enable(&priv->napi);

	netif_tx_start_all_queues(ndev);

	spin_unlock_irqrestore(&priv->lock, flags);

	if (priv->phydev)
		phy_start(priv->phydev);

	printk("[%s]resume finished(after up)!\n", __func__);
	
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
