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

#include <asm/io.h>
#include "stmmac.h"
#include "dwmac1000.h"

static void dwmac1000_mtl_init(void)
{
	unsigned int value;
	unsigned int queue_feature_en;
	int chn;
	
	struct gmac_tx_queue *tx_queue;
	struct gmac_rx_queue *rx_queue;
	struct stmmac_priv *priv = gmac_priv;

	/* Program the Tx Scheduling and Receive Arbitration Algorithm*/
	value = readl(priv->ioaddr + MTL_OPERT_MODE);
	value |= MTL_OPT_DTXSTS | MTL_OPT_SCHALG;
	writel(value, priv->ioaddr + MTL_OPERT_MODE);

	/* Program the Rx Queue DMA map
	*  when rx have more than one channel, it is configurable.
	*  we just using one Rx channel, so we don't need to configure MTL_RXQ_DMA_MAP.
	*/

	for (chn = 0; chn < priv->tx_queue_cnt; chn++) {
		tx_queue = GET_TX_QUEUE(priv, chn);

		if (chn > TX_CHN_NET) {
			/* enable credit control, and usging CBS */
			value = AV_ALGORITHM | SLOT_COUNT | CREDIT_CONTROL;
			writel(value, priv->ioaddr + MTL_TXQ_ETS_CTL(chn));

			/* program idleslope/sendslope/hicredit/locredit */
			writel(IDLE_SLOPE, priv->ioaddr + MTL_TXQ_IDLE_SLOPE_CREDIT(chn));
			writel(SEND_SLOPE, priv->ioaddr + MTL_TXQ_SEND_SLOPE_CREDIT(chn));
			writel(HI_CREDIT, priv->ioaddr + MTL_TXQ_HI_CREDIT(chn));
			writel(LO_CREDIT, priv->ioaddr + MTL_TXQ_LO_CREDIT(chn));

			/* AV feature */
			queue_feature_en = MTL_TXQ_AV_EN;
		} else {
			/* normal queue */
			queue_feature_en = MTL_TXQ_EN;
		}

		/* Program the tx operation mode, enable txq */
		value = MTL_TX_TTC | queue_feature_en | MTL_TX_TSF | 
				((tx_queue->mtl_fifo_size) << MTL_TXQ_SIZE_OFFSET);
		writel(value, priv->ioaddr + MTL_TXQ_OPRT_MODE(chn));
	}

	for (chn = 0; chn < priv->rx_queue_cnt; chn++) {	
		rx_queue = GET_RX_QUEUE(priv, chn);
		
		/* Program the rx operation mode */
		value = MTL_RX_EHFC | MTL_RXQ_DIS_TCP_EF | MTL_RX_FUP | MTL_RX_RTC 
				| MTL_RX_RSF | ((rx_queue->mtl_fifo_size) << MTL_RXQ_SIZE_OFFSET);		
		writel(value, priv->ioaddr + MTL_RXQ_OPRT_MODE(chn));
	}
}

const struct stmmac_mtl_ops dwmac1000_mtl_ops = {
	.mtl_init = dwmac1000_mtl_init,
};
