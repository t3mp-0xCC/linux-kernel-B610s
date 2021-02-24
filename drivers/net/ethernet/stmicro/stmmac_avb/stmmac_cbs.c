/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * liufangyuan <liufangyuan2@huawei.com>
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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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
 */

#include <linux/io.h>
#include "common.h"
#include "stmmac_cbs.h"
#include "stmmac.h"

void stmmac_config_idle_slope(int chn, void __iomem *ioaddr, 
							unsigned int idle_slope)
{
	writel(idle_slope, ioaddr + MTL_TXQ_IDLE_SLOPE_CREDIT(chn));
}

unsigned int stmmac_get_idle_slope(int chn, void __iomem *ioaddr)
{
	unsigned int idle_slope;

	idle_slope = readl(ioaddr + MTL_TXQ_IDLE_SLOPE_CREDIT(chn)) & MTL_TX_ISLC_MASK;

	return idle_slope;
}
void stmmac_config_send_slope(int chn, void __iomem *ioaddr,
							unsigned int send_slope)
{
	writel(send_slope, ioaddr + MTL_TXQ_SEND_SLOPE_CREDIT(chn));
}

unsigned int stmmac_get_send_slope(int chn, void __iomem *ioaddr)
{
	unsigned int send_slope;

	send_slope = readl(ioaddr + MTL_TXQ_SEND_SLOPE_CREDIT(chn)) & MTL_TX_SSLC_MASK;

	return send_slope;
}
void stmmac_config_high_credit(int chn, void __iomem *ioaddr,
							unsigned int high_credit)
{
	writel(high_credit, ioaddr + MTL_TXQ_HI_CREDIT(chn));
}

void stmmac_config_low_credit(int chn, void __iomem *ioaddr,
								unsigned int low_credit)
{
	writel(low_credit, ioaddr + MTL_TXQ_LO_CREDIT(chn));
}

int stmmac_cbs_get_chn(unsigned int class_id)
{
	int chn_id;
	
	switch (class_id) {
		case STMMAC_SRP_CLASS_A:
			chn_id = TX_CHN_CLASS_A;
			break;

		case STMMAC_SRP_CLASS_B:
			chn_id = TX_CHN_CLASS_B;
			break;

		default:
			chn_id = -EINVAL;
			break;
	}

	return chn_id;
}

static int stmmac_get_locredit(struct net_device *ndev, unsigned int class_id)
{
	int chn_id;
	unsigned int low_credit;
	struct stmmac_priv *priv;

	if (!ndev) {
		printk(KERN_ERR "[%s]netdev is NULL\n", __func__);
		return -EINVAL;
	}

	priv = netdev_priv(ndev);
	chn_id = stmmac_cbs_get_chn(class_id);
	if (chn_id < 0) {
		printk(KERN_ERR "[%s]wrong class id:0x%x\n", __func__, class_id);
		return chn_id;
	}

	low_credit = readl(priv->ioaddr + MTL_TXQ_LO_CREDIT(chn_id));

	return low_credit;
}

int stmmac_get_hicredit(struct net_device *ndev, unsigned int class_id)
{
	int chn_id;
	unsigned int hi_credit;
	struct stmmac_priv *priv;

	if (!ndev) {
		printk(KERN_ERR "[%s]netdev is NULL\n", __func__);
		return -EINVAL;
	}

	priv = netdev_priv(ndev);
	chn_id = stmmac_cbs_get_chn(class_id);
	if (chn_id < 0) {
		printk(KERN_ERR "[%s]wrong class id:0x%x\n", __func__, class_id);
		return chn_id;
	}

	hi_credit = readl(priv->ioaddr + MTL_TXQ_HI_CREDIT(chn_id));

	return hi_credit;
}

void stmmac_set_avb_algorithm(int chn, void __iomem *ioaddr,
								unsigned int algorithm)
{
	writel((!!algorithm) << ALG_OFFSET, ioaddr + MTL_TXQ_ETS_CTL(chn));
}

unsigned int stmmac_get_avb_algo(int chn, void __iomem *ioaddr)
{
	unsigned int algorithm;

	algorithm = ((readl(ioaddr + MTL_TXQ_ETS_CTL(chn)) & ALG_MASK) >> ALG_OFFSET);
	
	return algorithm;
}

unsigned int stmmac_get_flow_rate(int chn, void __iomem *ioaddr)
{
	unsigned int flow_rate;	//format:kbps

	flow_rate = readl(ioaddr + MTL_TXQ_ETS_STAT(chn)) & ABS_MASK;
	
	/* flow_rate is average bits per slot, one slot is 125us.
	* So 
	* rate = flow_rate/125us => 
	* rate = flow_rate * 8000 bps =>
	* rate = (flow_rate * 125)/16 kbps
	*/
	//flow_rate = (flow_rate * 125) >> 4;
	//flow_rate = flow_rate * 8;	//roughly equal to 8

	if (chn == TX_CHN_NET) {
		flow_rate = (flow_rate * 100) >> 10; //kbps
	} else {
		flow_rate = (flow_rate * 8); //kbps
	}

	return flow_rate;
}

int stmmac_cbs_get_bw(void __iomem *ioaddr, int chn, unsigned int line_speed)
{
	int bandwidth;
	unsigned int idle_slope;
	unsigned int sendslope;

	if ((line_speed != 1000) && (line_speed != 100)) {
		printk(KERN_ERR "[%s]wrong line_speed:%d\n", __func__, line_speed);
		return -EINVAL;
	}

	if ((chn != TX_CHN_CLASS_B) && (chn != TX_CHN_CLASS_A)) {
		printk(KERN_ERR "[%s]wrong channel:%d\n", __func__, chn);
		return -EINVAL;
	}
	
	/* Get bandwidth */
	idle_slope = stmmac_get_idle_slope(chn,ioaddr);
	sendslope = stmmac_get_send_slope(chn, ioaddr);
	bandwidth = (idle_slope * line_speed)/(idle_slope + sendslope);

	return bandwidth;
	
}

void stmmac_cbs_cfg_slope(void __iomem *ioaddr, int chn, 
							unsigned int bw, unsigned int line_speed)
{
	unsigned int send_slope;
	unsigned int idle_slope;
	unsigned int port_bits;
	unsigned int idle_frac_part;		//Sendslope Fractional Part

	if (line_speed == 1000) {
		port_bits = 8;		//1000Mbps
	} else if (line_speed == 100) {
		port_bits = 4;		//100Mbps
	} else {
		printk(KERN_ERR "[%s]wrong line_speed:0x%x\n", __func__, line_speed);
		return;
	}

	if ((chn < TX_CHN_CLASS_B) ||(chn > TX_CHN_CLASS_A)) {
		printk(KERN_ERR "[%s]wrong channel:0x%x\n", __func__, chn);
		return;
	}

	idle_slope = (bw * port_bits * STMMAC_CBS_CCOE)/line_speed;
	idle_frac_part = (bw * port_bits * STMMAC_CBS_CCOE) - (idle_slope * line_speed);
	if (idle_frac_part) {
		idle_slope += 1;
	}

	send_slope = port_bits * STMMAC_CBS_CCOE - idle_slope;		
	stmmac_config_idle_slope(chn, ioaddr, idle_slope);
	stmmac_config_send_slope(chn, ioaddr, send_slope);
}

int stmmac_cbs_config_bw(struct net_device *dev, int action, 
										unsigned int bw, unsigned int class_id)
{
	int chn_id, chn;
	unsigned int chn_bw;
	unsigned int allocated_bw = 0;
	struct stmmac_priv *priv = netdev_priv(dev);   

	/* input check */
	if (!dev) {
		printk(KERN_ERR "[%s]net device is NULL!\n", __func__);
		return -EINVAL;
	}
	
	chn_id = stmmac_cbs_get_chn(class_id);
	if (chn_id < 0) {
		printk(KERN_ERR "[%s]wrong class id:0x%x\n", __func__, class_id);
		return -EINVAL;
	}

	for (chn = TX_CHN_CLASS_B; chn < TX_CHN_NR; chn++) {
		allocated_bw += stmmac_cbs_get_bw(priv->ioaddr, chn, priv->line_speed);
	}

	/* The request bandwidth must be a multiple of ten. */
	bw = STMMAC_CBS_ALIGN_DEC(bw, STMMAC_CBS_ROUND);
	chn_bw = stmmac_cbs_get_bw(priv->ioaddr, chn_id, priv->line_speed);
	switch (action) {
		case CBS_REQUEST_BW:
			if ((allocated_bw + bw) >= priv->line_speed) {
				printk(KERN_ERR "[%s]failed!chn_id:0x%x\n", __func__, chn_id);
				printk(KERN_ERR "[%s]failed!allocate:%d, bw:%d\n", __func__, allocated_bw, bw);
				return -EINVAL;
			}
			chn_bw += bw;
			break;

		case CBS_RELEASE_BW:
			if (chn_bw < bw) {
				printk(KERN_ERR "[%s]chn_id:0x%x\n", __func__, chn_id);
				printk(KERN_ERR "[%s]No bandwidth to release:%d\n", __func__, chn_bw);
				printk(KERN_ERR "[%s]The channel's bandwidth will release to zero.\n", __func__);
				chn_bw = 0;
			} else {
				chn_bw -= bw;
			}
			break;

		default:
			printk(KERN_ERR "[%s]wrong action:%d\n", __func__, action);
			return -EINVAL;
	}

	stmmac_cbs_cfg_slope(priv->ioaddr, chn_id, chn_bw, priv->line_speed);

	return 0;
}

static int stmmac_get_avb_algorithm(struct net_device *ndev, unsigned int class_id)
{
	int chn_id;
	unsigned int algorithm;
	struct stmmac_priv *priv;

	if (!ndev) {
		printk(KERN_ERR "[%s]netdev is NULL\n", __func__);
		return -EINVAL;
	}

	priv = netdev_priv(ndev);
	chn_id = stmmac_cbs_get_chn(class_id);
	if (chn_id < 0) {
		printk(KERN_ERR "[%s]wrong class id:0x%x\n", __func__, class_id);
		return chn_id;
	}
	
	algorithm = stmmac_get_avb_algo(chn_id, priv->ioaddr);

	return algorithm;
}

static int stmmac_get_preamble_ipg(struct net_device *ndev)
{
	union cbs_mac_para mac_para;
	struct stmmac_priv *priv;
	int ipg;
	int preamble;

	if (!ndev) {
		printk(KERN_ERR "[%s]netdev is NULL\n", __func__);
		return -EINVAL;
	}

	priv = netdev_priv(ndev);
	ipg = priv->hw->mac->get_mac_ipg(priv->ioaddr);
	preamble = priv->hw->mac->get_mac_preamble(priv->ioaddr);
	if (preamble < 0) {
		printk(KERN_ERR "[%s]wrong preamble!(%d)\n", __func__, mac_para.ipg_preamble.preamble);
		return -EINVAL;
	}
	
	mac_para.ipg_preamble.ipg = ipg;
	mac_para.ipg_preamble.preamble = preamble;

	return mac_para.value;
}

static int stmmac_set_class_pcp(struct net_device *ndev,
						unsigned int class_id, unsigned int pcp)
{
	struct stmmac_priv *priv;

	if (!ndev) {
		printk(KERN_ERR "[%s]netdev is NULL\n", __func__);
		return -EINVAL;
	}

	priv = netdev_priv(ndev);
	
	switch (class_id) {
		case STMMAC_SRP_CLASS_A:
			priv->pcp_class_a = pcp;
			break;

		case STMMAC_SRP_CLASS_B:
			priv->pcp_class_b = pcp;
			break;		

		default:
			printk(KERN_ERR "[%s]wrong class id!(%d)\n", __func__, class_id);
			return -EINVAL;
	}

	return 0;
}

static int stmmac_get_class_pcp(struct net_device *ndev,
									unsigned int class_id, unsigned int *pcp)
{
	struct stmmac_priv *priv;

	if (!ndev) {
		printk(KERN_ERR "[%s]netdev is NULL\n", __func__);
		return -EINVAL;
	}

	priv = netdev_priv(ndev);
	
	switch (class_id) {
		case STMMAC_SRP_CLASS_A:
			*pcp = priv->pcp_class_a;
			break;

		case STMMAC_SRP_CLASS_B:
			*pcp = priv->pcp_class_b;
			break;		

		default:
			printk(KERN_ERR "[%s]wrong class id!(%d)\n", __func__, class_id);
			return -EINVAL;
	}

	return 0;
}

void stmmac_cbs_get_alg(struct cbs_config *config, void __iomem *ioaddr)
{
	unsigned int chn;
	struct tx_chn_status *chn_output;

	for (chn = TX_CHN_CLASS_B; chn < TX_CHN_NR; chn++) {
		chn_output = config->output+ chn;
		chn_output->algorithm = stmmac_get_avb_algo(chn, ioaddr);
	}

	chn_output = config->output+ TX_CHN_NET;
	chn_output->algorithm = 0;		//strick algorithm
}

void stmmac_cbs_query_bw(void __iomem *ioaddr, 
					struct cbs_config *config, int line_speed)
{
	int chn;
	unsigned int audio_bw, video_bw;
	struct tx_chn_status *chn_output;

	if ((line_speed != 1000) && (line_speed != 100)) {
		printk(KERN_ERR "[GMAC CBS]wrong line_speed:0x%x\n", line_speed);
		return;
	}
	
	/* Get flow rate */
	for (chn = TX_CHN_NET; chn < TX_CHN_NR; chn++) {
		chn_output = config->output + chn;
		chn_output->flow_rate = stmmac_get_flow_rate(chn,ioaddr);
	}

	/* Get Class A bandwidth */
	chn_output = config->output + TX_CHN_CLASS_A;
	audio_bw = stmmac_cbs_get_bw(ioaddr, TX_CHN_CLASS_A, line_speed);
	chn_output->bandwidth = audio_bw;

	/* Get Class B bandwidth */
	chn_output = config->output + TX_CHN_CLASS_B;
	video_bw = stmmac_cbs_get_bw(ioaddr, TX_CHN_CLASS_B, line_speed);
	chn_output->bandwidth = video_bw;

	chn_output = config->output + TX_CHN_NET;
	chn_output->bandwidth = line_speed - audio_bw - video_bw;

	/* Get algorithm */
	stmmac_cbs_get_alg(config, ioaddr);	
}

void stmmac_cbs_config_alg(struct cbs_config *config, void __iomem *ioaddr)
{
	struct tx_chn_cfg *chn_input;

	/* Set audio channel */
	chn_input = config->input;
	stmmac_set_avb_algorithm(TX_CHN_CLASS_A, ioaddr, chn_input->algorithm);

	/* Set video channel */
	chn_input = config->input + 1;
	stmmac_set_avb_algorithm(TX_CHN_CLASS_B, ioaddr, chn_input->algorithm);
}

void stmmac_show_cbs_chn(struct stmmac_priv *priv)
{
	int chn;
	unsigned int bw;
	unsigned int avb_bw = 0;
	unsigned int flow_rate;
	unsigned int algorithm;
	
	printk(KERN_ERR "Show cbs status:\n");

	for (chn= TX_CHN_CLASS_B; chn < TX_CHN_NR; chn++) {
		bw = stmmac_cbs_get_bw(priv->ioaddr, chn, priv->line_speed);
		avb_bw += bw;
		flow_rate = stmmac_get_flow_rate(chn,priv->ioaddr);
		algorithm = stmmac_get_avb_algo(chn, priv->ioaddr);
		printk(KERN_ERR "[AVB CBS]chn:%d,bandwidth:%d, flow rate:%d, algorithm:%d\n", 
			chn, bw, flow_rate, algorithm);
	}	

	bw = priv->line_speed - avb_bw;
	flow_rate = stmmac_get_flow_rate(TX_CHN_NET,priv->ioaddr);
	algorithm = 0;
	
	printk(KERN_ERR "[AVB CBS]chn:0,bandwidth:%d, flow rate:%d, algorithm:%d\n", 
			bw, flow_rate, algorithm);

}

int stmmac_cbs_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int ret = 0;
	int chn;
	unsigned int bw = 0;
	unsigned int class_id;
    struct cbs_config config;
    struct stmmac_priv *priv = netdev_priv(dev);    
	
    if (copy_from_user(&config, ifr->ifr_data, sizeof(struct cbs_config))) {
        return -EFAULT;
    }
	
	switch (cmd) {
		case STMMAC_CBS_REQUEST_BANDWIDTH:
			for (chn = 0; chn < TX_CFG_CHN_NR; chn++) {
				if (chn == 0) {
					class_id = STMMAC_SRP_CLASS_B;
				} else if (chn == 1) {
					class_id = STMMAC_SRP_CLASS_A;
				}

				/* The request bandwidth must be a multiple of ten. */
				bw = config.input[chn].bandwidth;
				config.input[chn].bandwidth = STMMAC_CBS_ALIGN_DEC(bw, STMMAC_CBS_ROUND);
				ret = stmmac_cbs_config_bw(dev, CBS_REQUEST_BW, config.input[chn].bandwidth, class_id);
				if (ret) {
					return -1;
				}
			}
			break;
			
		case STMMAC_CBS_RELEASE_BANDWIDTH:
			for (chn = 0; chn < TX_CFG_CHN_NR; chn++) {
				if (chn == 0) {
					class_id = STMMAC_SRP_CLASS_B;
				} else if (chn == 1) {
					class_id = STMMAC_SRP_CLASS_A;
				}

				/* The request bandwidth must be a multiple of ten. */
				bw= config.input[chn].bandwidth;
				config.input[chn].bandwidth = STMMAC_CBS_ALIGN_DEC(bw, STMMAC_CBS_ROUND);
				ret = stmmac_cbs_config_bw(dev, CBS_RELEASE_BW, config.input[chn].bandwidth, class_id);
				if (ret) {
					return -1;
				}
			}
			break;
 		
		case STMMAC_CBS_CONFIG_ALG:
			stmmac_cbs_config_alg(&config, priv->ioaddr);
			break;

		case STMMAC_CBS_QUERY_CHN:
			stmmac_show_cbs_chn(priv);

		default:
            break;
	}
	
	stmmac_cbs_query_bw(priv->ioaddr, &config, priv->line_speed);
	
    return copy_to_user(ifr->ifr_data, &config,
			    sizeof(struct cbs_config)) ? -EFAULT : 0;    
}

const struct stmmac_cbs_ops stmmac_cbs = {
	.config_idle_slope = stmmac_config_idle_slope,
	.config_send_slope = stmmac_config_send_slope,
	.config_high_credit = stmmac_config_high_credit,
	.config_low_credit  = stmmac_config_low_credit,
	.set_avb_algorithm = stmmac_set_avb_algorithm,
	.get_locredit = stmmac_get_locredit,
	.get_hicredit = stmmac_get_hicredit,
	.get_avb_algorithm = stmmac_get_avb_algorithm,
	.get_preamble_ipg = stmmac_get_preamble_ipg,
	.set_class_pcp = stmmac_set_class_pcp,
	.get_class_pcp = stmmac_get_class_pcp,
};

