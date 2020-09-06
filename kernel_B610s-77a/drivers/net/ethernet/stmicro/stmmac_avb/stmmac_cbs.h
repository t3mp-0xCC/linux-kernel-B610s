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

#ifndef __STMMAC_CBS_H__
#define __STMMAC_CBS_H__

#define STMMAC_CBS_REQUEST_BANDWIDTH	(SIOCDEVPRIVATE + 1)
#define STMMAC_CBS_RELEASE_BANDWIDTH 	(SIOCDEVPRIVATE + 2)
#define STMMAC_CBS_QUERY_CHN			(SIOCDEVPRIVATE + 3)
#define STMMAC_CBS_CONFIG_ALG			(SIOCDEVPRIVATE + 4)

#define STMMAC_LINE_SPPED_1000M		0x0		//PS = 0 FES = 0, 1000Mbps
#define STMMAC_LINE_SPPED_100M		0x3		//PS = 1 FES = 1, 100Mbps

#define STMMAC_CBS_CCOE			1024	//Credit Coefficient
#define STMMAC_CBS_ROUND		10		//CBS Bandwidth 
#define STMMAC_SRP_CLASS_A		6
#define STMMAC_SRP_CLASS_B		5

enum cbs_action {
	CBS_REQUEST_BW,
	CBS_RELEASE_BW,
};

#define STMMAC_CBS_ALIGN_DEC(x, n)	((((x) + (n) -1 ) / (n)) * (n))

struct tx_chn_cfg {
	unsigned int bandwidth;	//format:Mbps
	unsigned int algorithm;	//0:strict priority,1:CBS Algorithm
};

struct tx_chn_status {
	unsigned int bandwidth;	//format:Mbps
	unsigned int flow_rate;	//format:kbps
	unsigned int algorithm;	//0:strict priority,1:CBS Algorithm
};

struct cbs_config {
	struct tx_chn_cfg input[TX_CFG_CHN_NR];
	struct tx_chn_status output[TX_CHN_NR];
};

struct cbs_ipg_preamble {
	unsigned int ipg:16;
	unsigned int preamble:16;
};

union cbs_mac_para {
	struct cbs_ipg_preamble ipg_preamble;
	unsigned int value;
};

void stmmac_config_idle_slope(int chn, void __iomem *ioaddr,  unsigned int idle_slope);
void stmmac_config_send_slope(int chn, void __iomem *ioaddr,  unsigned int send_slope);
void stmmac_config_high_credit(int chn, void __iomem *ioaddr,  unsigned int high_credit);
void stmmac_config_low_credit(int chn, void __iomem *ioaddr,  unsigned int low_credit);
void stmmac_set_avb_algorithm(int chn, void __iomem *ioaddr,  unsigned int algorithm);
unsigned int stmmac_get_flow_rate(int chn, void __iomem *ioaddr);
int stmmac_cbs_config_bw(struct net_device *dev, int action, unsigned int bw, unsigned int class_id);
void stmmac_cbs_query_bw(void __iomem *ioaddr, struct cbs_config *config, int phy_speed);
void stmmac_cbs_config_alg(struct cbs_config *config, void __iomem *ioaddr);
int stmmac_cbs_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
int stmmac_cbs_get_bw(void __iomem *ioaddr, int chn, unsigned int line_speed);
void stmmac_cbs_cfg_slope(void __iomem *ioaddr, int chn, unsigned int bw, unsigned int line_speed);

#endif /* __STMMAC_CBS_H__ */
