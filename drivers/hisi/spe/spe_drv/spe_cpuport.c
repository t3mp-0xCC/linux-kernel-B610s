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
 *
 */


#include <linux/spe/spe_interface.h>

#include "spe.h"
#include "spe_port.h"
#include "spe_desc.h"
#include "spe_dbg.h"

#define CPUPORT_RD_SKB_SIZE             SPE_DFT_SKB_LEN
#define CPUPORT_RECYCLE_THRESHOLD_DIV   (2)
#define CPUPORT_RD_DEPTH                (16)

void cpuport_kevent(struct spe *spe, int flags)
{
    struct spe_port_stat *stat = &spe->ports[spe->cpuport.portno].stat;

	if (test_and_set_bit(flags, &spe->todo))
		return;

	if (!schedule_delayed_work(&spe->work, spe->work_delay)){
        stat->cpu_kevent_cancel++;
    }else{
        stat->cpu_kevent_ok++;
    }
}

int spe_cpuport_replenish_rd(struct sk_buff *skb, int portno)
{
    struct spe *spe = &spe_balong;
	struct sk_buff *new = NULL;
	dma_addr_t dma;
	int ret = 0;
	int err = 0;

	 /* alloc a new skb */
	new = dev_alloc_skb(CPUPORT_RD_SKB_SIZE);
	if(!new){
		spe->ports[portno].stat.cpu_port_alloc_skb_fail++;
		new = skb;
		ret = -ENOMEM;
	}

	/* map skb->data */
	dma = dma_map_single(spe->dev, new->data, CPUPORT_RD_SKB_SIZE, DMA_FROM_DEVICE);
	
	err = spe_rd_config(portno, new, dma);
	if(err){
		dev_kfree_skb(new);
	}

	return ret;
}

int spe_cpuport_finish_rd(int portno, int src_portno, struct sk_buff *skb, dma_addr_t dma, unsigned int updata_only)
{
    struct spe *spe = &spe_balong;
    struct net_device *net = spe_port_netdev(src_portno);
    struct spe_port_ctx *cpu_port = &spe->ports[portno];
    int ret;

	SPE_TRACE("enter\n");

    SPE_BUG(("portno(%d) != spe->cpuport(%d)\n",portno,spe->cpuport.portno),
        portno != spe->cpuport.portno);

    dma_unmap_single(spe->dev, dma, skb->len, DMA_FROM_DEVICE);

    if(updata_only){
       spe_skb_set_updateonly(skb);
    }

	ret = spe_cpuport_replenish_rd(skb, spe->cpuport.portno);
	if (ret) {
		cpu_port->stat.rd_drop++;
		return 0;
	}
	
    if(net == NULL){   
        cpu_port->stat.rd_net_null++;
        dev_kfree_skb_any(skb);
		return 0;
    }
	
    /* if it is bypass mode(stick mode) */
    if(spe_mode_stick == spe->spe_mode){
        if(cpu_port->ctrl.desc_ops.finish_rd_bypass){
            cpu_port->ctrl.desc_ops.finish_rd_bypass(portno, src_portno, skb);
        }
    }
    else{
        skb->protocol = eth_type_trans(skb, net);

        if (spe_balong.msg_level & SPE_MSG_CPU_SKB) {
			int len = min((int)60, (int)(skb->len));\
            print_hex_dump(KERN_ERR, "cpu skb: ", DUMP_PREFIX_ADDRESS, \
                        16, 1, (skb->data), len, true);
            printk("\n");
        }

        if(in_irq()){
            ret = netif_rx(skb);
        }else{
            ret = netif_rx_ni(skb);
        }

        if(NET_RX_SUCCESS != ret){/* netif_rx has drop it */
            /*SPE_ERR("netif rx failed ret = %d\n",ret);*/
            cpu_port->stat.netif_rx_drop++;
        }
    }

    cpu_port->stat.rd_send_success++;

    return 0;
}

int spe_cpuport_init(struct spe *spe)
{
    struct spe_port_attr attr = {0};
    unsigned int portno = 0;
	int ret;

    attr.desc_ops.finish_rd = spe_cpuport_finish_rd;
    attr.enc_type = spe_enc_cpu;
    attr.attach_brg = spe_attach_brg_normal;
    attr.rd_depth = CPUPORT_RD_DEPTH;
    attr.td_depth = 0;
    attr.rd_skb_num = attr.rd_depth;
    attr.rd_skb_size = CPUPORT_RD_SKB_SIZE;
    attr.stick_mode = 0;

    ret = spe_port_alloc(&attr);
    if(ret<0 || ret>SPE_PORT_NUM){
        SPE_ERR("alloc cpuport failed,portno %d!\n",portno);
        return SPE_ERR_NOCHAN;
    }

	portno = ret;

    spe->cpuport.portno = portno;
    spe->cpuport.threshold = attr.rd_skb_num/CPUPORT_RECYCLE_THRESHOLD_DIV;
	spe_port_enable((int)portno);

    return 0;
}

int spe_cpuport_exit(struct spe *spe)
{
    int ret;
    int portno = spe->cpuport.portno;

    ret = spe_port_disable(portno);
    if(ret){
        SPE_ERR("fail to disable cpu port %d",portno);
        return ret;
    }

    ret = spe_port_free(portno);
    if(ret){
        SPE_ERR("fail to free cpu port %d",portno);
        return ret;
    }

    return 0;
}

MODULE_LICENSE("GPL");

