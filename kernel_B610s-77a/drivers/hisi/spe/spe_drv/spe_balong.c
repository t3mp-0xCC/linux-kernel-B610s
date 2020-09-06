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


#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/spe/spe_interface.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "spe.h"
#include "spe_debugfs.h"
#include "spe_dbg.h"
#include "spe_desc.h"
#include "bsp_sysctrl.h"

static u64 spe_dmamask = DMA_BIT_MASK(64);
extern int spe_filter_init(void);
extern int spe_filter_exit(void);
extern void spe_sysctrl_init(void);
extern void spe_sysctrl_exit(void);


static struct spe_plat_data spe_platdata = 
{
    .sysctrl_init = spe_sysctrl_init,
    .sysctrl_exit = spe_sysctrl_exit
};

void spe_sysctrl_init(void)
{
    return ;
}

void spe_sysctrl_exit(void)
{
    return ;
}

static int spe_clk_enable(struct device *dev)
{
    struct clk *clk = NULL;
    int ret = 0;

    clk =clk_get(dev,"spe_clk");
    if(IS_ERR(clk)){
            SPE_ERR("clk:spe_clk is NULL, can't find it,please check!!\n");
            return ret;
    } else {
        ret = clk_prepare_enable(clk);
        if(ret){
            SPE_ERR("clk:spe_clk enable failed!!\n");
         } 
    }
    return ret;
}

int spe_clk_disable(struct device *dev)
{
    struct clk *clk = NULL;
    int ret = 0;

    clk =clk_get(dev,"spe_clk");
    if(IS_ERR(clk)){
            SPE_ERR("clk:spe_clk is NULL, can't find it,please check!!\n");
            return ret;
    } else {
        clk_disable_unprepare(clk);
    }
    return ret;
}

static void spe_sysctrl_set(const struct device_node *np, const char *propname, 
	unsigned int value)
{
	u32 para_value[3]={0};
	u32 bitnum;
	void *base;
	u32 reg = 0;
	int ret;
	
	ret = of_property_read_u32_array(np, propname, para_value, 3);
	if(ret){
		SPE_ERR("skip find of [%s]\n", propname);
		return;
	}

	bitnum = para_value[2] - para_value[1] + 1;
	base = bsp_sysctrl_addr_get((void*)para_value[0]);
	if(!base){
		SPE_ERR("bsp_sysctrl_addr_get return null\n");
		return;
	}
	
	reg = readl(base);
	reg &= ~(((1<<bitnum)-1)<<para_value[1]);
	reg |= (value << para_value[1]);
	writel(reg, base);

	return;
}

static void spe_open_ports(unsigned resume_flag)
{
    struct spe *spe = &spe_balong;
    HI_SPE_PORTX_PROPERTY_0_T property;
    int portno;
	
    for(portno =0; portno<SPE_PORT_NUM; portno++){
        if(resume_flag){
            spe->ports[portno].ctrl.td_busy = 0;
            spe->ports[portno].ctrl.td_free = 0;
        }
        if(spe->porten_bak & (1<<portno)){
            property.u32= spe_readl(spe->regs, SPE_CH_PROPERTY_OFFSET(portno));
            property.bits.spe_port_en = 1;
            spe->ports[portno].ctrl.flags |= SPE_PORT_ENABLED;
            spe_writel(spe->regs, SPE_CH_PROPERTY_OFFSET(portno), property.u32);
        }
    }
    spe->porten_bak = 0;
}

static int spe_close_ports(void)
{
    struct spe *spe = &spe_balong;
    HI_SPE_PORTX_PROPERTY_0_T property;
	struct spe_port_ctrl *ctrl;
	struct spe_td_desc *td_base;
	union spe_desc_queue_ptr td_ptr;
    int portno;
	uint32_t td_depth;
	uint32_t idx;
	
	HI_SPE_IDLE_T idle;
	idle.u32 = spe_readl(spe->regs, HI_SPE_IDLE_OFFSET);
    spe->flags |= (SPE_FLAG_ENABLE);
        
    for(portno=0; portno<SPE_PORT_NUM; portno++){
        property.u32= spe_readl(spe->regs, SPE_CH_PROPERTY_OFFSET(portno));
		if(property.bits.spe_port_en == 1){

			if (!(idle.bits.spe_port_idle &(1<<portno))){
        		return -EBUSY;
    		}
            
            ctrl = &spe->ports[portno].ctrl;
            td_base = (struct spe_td_desc *)ctrl->td_addr;
            td_depth = ctrl->td_depth;
            
            property.bits.spe_port_en = 0;
            spe_writel(spe->regs, SPE_CH_PROPERTY_OFFSET(portno), property.u32);
			spe->porten_bak |= 1<<portno;
            spe->ports[portno].ctrl.flags &= (~SPE_PORT_ENABLED);
            
            property.u32= spe_readl(spe->regs, SPE_CH_PROPERTY_OFFSET(portno));
            
            if(spe->cpuport.portno != portno){

	            /* get tdq ptr, include rd and wr ptr */
		        td_ptr.raw = spe_readl(spe->regs, SPE_TDQ_PTR_OFFSET(portno));

	    		if(ctrl->td_busy == td_ptr.bits.rptr)/* no more td completed */
	    		{
	    			continue;
	    		}

	            /* recycle td from td_busy to tdq rptr */
	            for(idx = ctrl->td_busy; idx != td_ptr.bits.rptr; idx = (idx + 1) % td_depth){
	    			ctrl->desc_ops.finish_td(portno, (struct sk_buff *)td_base[idx].trb.skb_addr ,0);
	                td_base[idx].trb.buf_addr = 0;
	                td_base[idx].trb.pkt_len = 0;
	            }

	            ctrl->td_busy = idx;
			}
        }
    }
    spe->flags &= (~SPE_FLAG_ENABLE);

	return 0;
}

static void spe_registers_store(void)
{
    struct spe *spe = &spe_balong;
	struct spe_registers_stash *spe_registers = (struct spe_registers_stash *)spe->reg_bak;
    int portno;
    spe_registers->spe_userfied_ctrl = spe_readl(spe->regs, HI_SPE_USRFIELD_CTRL_OFFSET);
    spe_registers->spe_black_write = spe_readl(spe->regs, HI_SPE_BLACK_WHITE_OFFSET);
    spe_registers->spe_port_lmttime = spe_readl(spe->regs, HI_SPE_PORT_LMTTIME_OFFSET);
    spe_registers->spe_eth_minlen = spe_readl(spe->regs, HI_SPE_ETH_MINLEN_OFFSET);
    spe_registers->spe_mode = spe_readl(spe->regs, HI_SPE_MODE_OFFSET);
    spe_registers->spe_glb_dmac_addr_l = spe_readl(spe->regs, HI_SPE_GLB_DMAC_ADDR_L_OFFSET);
    spe_registers->spe_glb_dmac_addr_h = spe_readl(spe->regs, HI_SPE_GLB_DMAC_ADDR_H_OFFSET);
    spe_registers->spe_glb_smac_addr_l = spe_readl(spe->regs, HI_SPE_GLB_SMAC_ADDR_L_OFFSET);
    spe_registers->spe_glb_smac_addr_h = spe_readl(spe->regs, HI_SPE_GLB_SMAC_ADDR_H_OFFSET);
    spe_registers->spe_rd_burstlen = spe_readl(spe->regs, HI_SPE_RD_BURSTLEN_OFFSET);
    
    for(portno = 0; portno<SPE_PORT_NUM; portno++){
        spe_registers->spe_tdqx_baddr[portno] = spe_readl(spe->regs, SPE_TDQ_BADDR_OFFSET(portno));
        spe_registers->spe_tdqx_len[portno] = spe_readl(spe->regs, SPE_TDQ_LEN_OFFSET(portno));
        spe_registers->spe_rdqx_baddr[portno] = spe_readl(spe->regs, SPE_RDQ_BADDR_OFFSET(portno));
        spe_registers->spe_rdqx_len[portno] = spe_readl(spe->regs, SPE_RDQ_LEN_OFFSET(portno));
        spe_registers->spe_rdqx_ptr[portno] = spe_readl(spe->regs, SPE_RDQ_PTR_OFFSET(portno));
        spe_registers->spe_tdqx_pri[portno] = spe_readl(spe->regs, SPE_TDQ_PRI_OFFSET(portno));
        spe_registers->spe_chx_portnum[portno] = spe_readl(spe->regs, SPE_PORT_NUM_OFFSET(portno));
        spe_registers->spe_portx_udp_lmtnum[portno] = spe_readl(spe->regs, SPE_CH_UDP_LIMIT_CNT_OFFSET(portno));
        spe_registers->spe_portx_lmtbyte[portno] = spe_readl(spe->regs, SPE_CH_RATE_LIMIT_BYTE_OFFSET(portno));
        spe_registers->spe_portx_property[portno] = spe_readl(spe->regs, SPE_CH_PROPERTY_OFFSET(portno));
        spe_registers->spe_portx_mac_addr_l[portno] = spe_readl(spe->regs, SPE_CH_MAC_ADDR_L(portno));
        spe_registers->spe_protx_mac_addr_h[portno] = spe_readl(spe->regs, SPE_CH_MAC_ADDR_H(portno));
        spe_registers->spe_portx_ipv6_addr_1s[portno] = spe_readl(spe->regs, SPE_CH_IPV6_ADDR_1ST(portno));
        spe_registers->spe_portx_ipv6_addr_2n[portno] = spe_readl(spe->regs, SPE_CH_IPV6_ADDR_2ND(portno));
        spe_registers->spe_portx_ipv6_addr_3t[portno] = spe_readl(spe->regs, SPE_CH_IPV6_ADDR_3RD(portno));
        spe_registers->spe_portx_ipv6_addr_4t[portno] = spe_readl(spe->regs, SPE_CH_IPV6_ADDR_4TH(portno));
        spe_registers->spe_portx_ipv6_mask[portno] = spe_readl(spe->regs, SPE_CH_IPV6_MASK(portno));
        spe_registers->spe_portx_ipv4_addr[portno] = spe_readl(spe->regs, SPE_CH_IPV4_ADDR(portno));
        spe_registers->spe_portx_ipv4_mask[portno] = spe_readl(spe->regs, SPE_CH_IPV4_MASK(portno));
        
    }

    spe_registers->spe_hash_baddr = spe_readl(spe->regs, HI_SPE_HASH_BADDR_OFFSET);
    spe_registers->spe_hash_zone = spe_readl(spe->regs, HI_SPE_HASH_ZONE_OFFSET);
    spe_registers->spe_hash_band = spe_readl(spe->regs, HI_SPE_HASH_RAND_OFFSET);
    spe_registers->spe_hash_l3_proto = spe_readl(spe->regs, HI_SPE_HASH_L3_PROTO_OFFSET);
    spe_registers->spe_hash_width = spe_readl(spe->regs, HI_SPE_HASH_WIDTH_OFFSET);
    spe_registers->spe_hash_depth = spe_readl(spe->regs, HI_SPE_HASH_DEPTH_OFFSET);
    spe_registers->spe_aging_time = spe_readl(spe->regs, HI_SPE_AGING_TIME_OFFSET);
    spe_registers->spe_pack_max_time = spe_readl(spe->regs, HI_SPE_PACK_MAX_TIME_OFFSET);
    spe_registers->spe_pack_ctrl = spe_readl(spe->regs, HI_SPE_PACK_CTRL_OFFSET);
    spe_registers->spe_pack_addr_ctrl = spe_readl(spe->regs, HI_SPE_PACK_ADDR_CTRL_OFFSET);
    spe_registers->spe_pack_pemain_len = spe_readl(spe->regs, HI_SPE_PACK_REMAIN_LEN_OFFSET);
    spe_registers->spe_unpack_ctrl = spe_readl(spe->regs, HI_SPE_UNPACK_CTRL_OFFSET);
    spe_registers->spe_unpack_max_len = spe_readl(spe->regs, HI_SPE_UNPACK_MAX_LEN_OFFSET);
    spe_registers->spe_event_buf_len = spe_readl(spe->regs, HI_SPE_EVENT_BUFF_LEN_OFFSET);
    spe_registers->spe_int0_interval = spe_readl(spe->regs, HI_SPE_INT0_INTERVAL_OFFSET);
    spe_registers->spe_int1_interval = spe_readl(spe->regs, HI_SPE_INT1_INTERVAL_OFFSET);
    spe_registers->spe_event_buff0_mask = spe_readl(spe->regs, HI_SPE_EVENT_BUFF0_MASK_OFFSET);
    spe_registers->spe_event_buff1_mask = spe_readl(spe->regs, HI_SPE_EVENT_BUFF1_MASK_OFFSET);
}

static void spe_registers_restore(void)
{
    struct spe *spe = &spe_balong;
	struct spe_registers_stash *spe_registers = (struct spe_registers_stash *)spe->reg_bak;
    int portno;
    spe_writel(spe->regs, HI_SPE_USRFIELD_CTRL_OFFSET, spe_registers->spe_userfied_ctrl);
    spe_writel(spe->regs, HI_SPE_BLACK_WHITE_OFFSET, spe_registers->spe_black_write);
    spe_writel(spe->regs, HI_SPE_PORT_LMTTIME_OFFSET, spe_registers->spe_port_lmttime);
    spe_writel(spe->regs, HI_SPE_ETH_MINLEN_OFFSET, spe_registers->spe_eth_minlen);
    spe_writel(spe->regs, HI_SPE_MODE_OFFSET, spe_registers->spe_mode);
    spe_writel(spe->regs, HI_SPE_GLB_DMAC_ADDR_L_OFFSET, spe_registers->spe_glb_dmac_addr_l);
    spe_writel(spe->regs, HI_SPE_GLB_DMAC_ADDR_H_OFFSET, spe_registers->spe_glb_dmac_addr_h);
    spe_writel(spe->regs, HI_SPE_GLB_SMAC_ADDR_L_OFFSET, spe_registers->spe_glb_smac_addr_l);
    spe_writel(spe->regs, HI_SPE_GLB_SMAC_ADDR_H_OFFSET, spe_registers->spe_glb_smac_addr_h);
    spe_writel(spe->regs, HI_SPE_RD_BURSTLEN_OFFSET, spe_registers->spe_rd_burstlen);

    for(portno = 0; portno<SPE_PORT_NUM; portno++){
        spe_writel(spe->regs, SPE_TDQ_BADDR_OFFSET(portno), spe_registers->spe_tdqx_baddr[portno]);
        spe_writel(spe->regs, SPE_TDQ_LEN_OFFSET(portno), spe_registers->spe_tdqx_len[portno]);
        spe_writel(spe->regs, SPE_RDQ_BADDR_OFFSET(portno), spe_registers->spe_rdqx_baddr[portno]);
        spe_writel(spe->regs, SPE_RDQ_LEN_OFFSET(portno), spe_registers->spe_rdqx_len[portno]);
        spe_writel(spe->regs, SPE_RDQ_PTR_OFFSET(portno), spe_registers->spe_rdqx_ptr[portno]);
        spe_writel(spe->regs, SPE_TDQ_PRI_OFFSET(portno), spe_registers->spe_tdqx_pri[portno]);
        spe_writel(spe->regs, SPE_PORT_NUM_OFFSET(portno), spe_registers->spe_chx_portnum[portno]);
        spe_writel(spe->regs, SPE_CH_UDP_LIMIT_CNT_OFFSET(portno), spe_registers->spe_portx_udp_lmtnum[portno]);
        spe_writel(spe->regs, SPE_CH_RATE_LIMIT_BYTE_OFFSET(portno), spe_registers->spe_portx_lmtbyte[portno]);
        spe_writel(spe->regs, SPE_CH_PROPERTY_OFFSET(portno), spe_registers->spe_portx_property[portno]);
        spe_writel(spe->regs, SPE_CH_MAC_ADDR_L(portno), spe_registers->spe_portx_mac_addr_l[portno]);
        spe_writel(spe->regs, SPE_CH_MAC_ADDR_H(portno), spe_registers->spe_protx_mac_addr_h[portno]);
        spe_writel(spe->regs, SPE_CH_IPV6_ADDR_1ST(portno), spe_registers->spe_portx_ipv6_addr_1s[portno]);
        spe_writel(spe->regs, SPE_CH_IPV6_ADDR_2ND(portno), spe_registers->spe_portx_ipv6_addr_2n[portno]);
        spe_writel(spe->regs, SPE_CH_IPV6_ADDR_3RD(portno), spe_registers->spe_portx_ipv6_addr_3t[portno]);
        spe_writel(spe->regs, SPE_CH_IPV6_ADDR_4TH(portno), spe_registers->spe_portx_ipv6_addr_4t[portno]);
        spe_writel(spe->regs, SPE_CH_IPV6_MASK(portno), spe_registers->spe_portx_ipv6_mask[portno]);
        spe_writel(spe->regs, SPE_CH_IPV4_ADDR(portno), spe_registers->spe_portx_ipv4_addr[portno]);
        spe_writel(spe->regs, SPE_CH_IPV4_MASK(portno), spe_registers->spe_portx_ipv4_mask[portno]);        
    }

     spe_writel(spe->regs, HI_SPE_HASH_BADDR_OFFSET, spe_registers->spe_hash_baddr);
     spe_writel(spe->regs, HI_SPE_HASH_ZONE_OFFSET, spe_registers->spe_hash_zone);
     spe_writel(spe->regs, HI_SPE_HASH_RAND_OFFSET, spe_registers->spe_hash_band);
     spe_writel(spe->regs, HI_SPE_HASH_L3_PROTO_OFFSET, spe_registers->spe_hash_l3_proto);
     spe_writel(spe->regs, HI_SPE_HASH_WIDTH_OFFSET, spe_registers->spe_hash_width);
     spe_writel(spe->regs, HI_SPE_HASH_DEPTH_OFFSET, spe_registers->spe_hash_depth);
     spe_writel(spe->regs, HI_SPE_AGING_TIME_OFFSET, spe_registers->spe_aging_time);
     spe_writel(spe->regs, HI_SPE_PACK_MAX_TIME_OFFSET, spe_registers->spe_pack_max_time);
     spe_writel(spe->regs, HI_SPE_PACK_CTRL_OFFSET, spe_registers->spe_pack_ctrl);
     spe_writel(spe->regs, HI_SPE_PACK_ADDR_CTRL_OFFSET, spe_registers->spe_pack_addr_ctrl);
     spe_writel(spe->regs, HI_SPE_PACK_REMAIN_LEN_OFFSET, spe_registers->spe_pack_pemain_len);
     spe_writel(spe->regs, HI_SPE_UNPACK_CTRL_OFFSET, spe_registers->spe_unpack_ctrl);
     spe_writel(spe->regs, HI_SPE_UNPACK_MAX_LEN_OFFSET, spe_registers->spe_unpack_max_len);
     spe_writel(spe->regs, HI_SPE_EVENT_BUFF_LEN_OFFSET, spe_registers->spe_event_buf_len);
     spe_writel(spe->regs, HI_SPE_INT0_INTERVAL_OFFSET, spe_registers->spe_int0_interval);
     spe_writel(spe->regs, HI_SPE_INT1_INTERVAL_OFFSET, spe_registers->spe_int1_interval);
     spe_writel(spe->regs, HI_SPE_EVENT_BUFF0_MASK_OFFSET, spe_registers->spe_event_buff0_mask);
     spe_writel(spe->regs, HI_SPE_EVENT_BUFF1_MASK_OFFSET, spe_registers->spe_event_buff1_mask);
}

static void spe_entry_store(void)
{
    int i;
	struct spe *spe = &spe_balong;
    unsigned int * cur_buf = spe->entry_bak;
      
    for(i=0; i<SPE_MAC_ENTRY_NUM; i++){
        *cur_buf = spe_readl(spe->regs, SPE_MAC_TAB_OFFSET(i));
        cur_buf++;
    }
}

static void spe_entry_restore(void)
{
    int i;
	struct spe *spe = &spe_balong;
    unsigned int * cur_buf = spe->entry_bak;
    
    
    for(i=0; i<SPE_MAC_ENTRY_NUM; i++){
        spe_writel(spe->regs, SPE_MAC_TAB_OFFSET(i), *cur_buf);
        cur_buf++;
    }
}

static int spe_bakspace_init(struct spe* spe)
{
	int ret = 0;

	spe->porten_bak = 0;
	spe->suspend_count = 0;
	spe->not_idle = 0;
	
	spe->reg_bak = kmalloc(sizeof(struct spe_registers_stash), GFP_ATOMIC);
	if(!spe->reg_bak){
		ret = -ENOMEM;
		}

	spe->entry_bak = kmalloc((sizeof(unsigned int))*SPE_MAC_ENTRY_NUM, GFP_ATOMIC);
	if(!spe->entry_bak){
		ret = -ENOMEM;
		}
	
	return ret;
}

static void spe_bakspace_deinit(struct spe * spe)
{
	if(spe->reg_bak){
		kfree(spe->reg_bak);
		}

	if(spe->entry_bak){
		kfree(spe->entry_bak);
		}
}

static int spe_suspend(struct device *dev)
{
    struct spe *spe = &spe_balong;
    HI_SPE_IDLE_T idle;
	unsigned long flags;
	unsigned int count;
    int ret = 0;


	spin_lock_irqsave(&spe->pm_lock,flags);
    //judge spe_idle    
    idle.u32 = spe_readl(spe->regs, HI_SPE_IDLE_OFFSET);
    if (!idle.bits.spe_idle){
        spe->not_idle++;
		spin_unlock_irqrestore(&spe->pm_lock,flags);
        return -EBUSY;
    }
	
    //close spe_en
    spe_disable(spe);

    //close spe_port 0-7
    if(spe_close_ports()) {
		ret = -EBUSY;
		goto open_spe_port;
    }

    //judge spe_idle again
    idle.u32 = spe_readl(spe->regs, HI_SPE_IDLE_OFFSET);
    if(!idle.bits.spe_idle){
        SPE_DBG("SPE is busy, suspend failed!");
        ret = -EBUSY;
        goto open_spe_port;
    }
    //open dbgen
    spe_dbgen_enable();
    
    //store registers
    spe_registers_store();

    //store entrys
    spe_entry_store();

	count = spe_readl(spe->regs, SPE_EVENTBUF_RPT_CNT_OFFSET(BUFFER_0_EVENT));
	spe_writel(spe->regs, SPE_EVENTBUF_PRO_CNT_OFFSET(BUFFER_0_EVENT),count);
	count = spe_readl(spe->regs, SPE_EVENTBUF_RPT_CNT_OFFSET(BUFFER_1_EVENT));
	spe_writel(spe->regs, SPE_EVENTBUF_PRO_CNT_OFFSET(BUFFER_1_EVENT),count);
	
    //close dbgen
    spe_dbgen_disable();

	spin_unlock_irqrestore(&spe->pm_lock,flags);

    //close clk
    spe_clk_disable(dev);

	spe->suspend_count++;

	
    return ret;

open_spe_port:
    //open spe_port 0-7
    spe_open_ports(0);
    spe_enable(spe);
	spe->not_idle++;
	spin_unlock_irqrestore(&spe->pm_lock,flags);
    return ret;
}

static int spe_resume(struct device *dev)
{
    int ret = 0;
	struct spe *spe = &spe_balong;
	unsigned long flags;
	
    //open clk
    spe_clk_enable(dev);
	
	spin_lock_irqsave(&spe->pm_lock,flags);

    //reset
    spe_sysctrl_set(dev->of_node, "spe_pd_crg_srsten3_spe_srst_en", 1);

    //unreset
    spe_sysctrl_set(dev->of_node, "spe_pd_crg_srstdis3_spe_srst_dis", 1);

    //open dbgen
    spe_dbgen_enable();

    //restore entrys   timestamp?
    spe_entry_restore();

    //restore registers
    spe_registers_restore();

    //close dbgen
    spe_dbgen_disable();
    
    //open port 0-7
    spe_open_ports(1);

    spe->ev_buffs[0]->lpos = 0;
    spe->ev_buffs[1]->lpos = 0;

    //open spe_en
    spe_enable(spe);

	spe->resume_count++;
	
	spin_unlock_irqrestore(&spe->pm_lock,flags);
    
    return ret;
}

int do_spe_suspend(void)
{
    struct device *dev = spe_balong.dev;
    int ret = 0;
    ret = spe_suspend(dev);
    return ret;        
}

int do_spe_resume(void)
{
    struct device *dev = spe_balong.dev;
    int ret = 0;
    ret = spe_resume(dev);
    return ret;        
}



static int spe_probe(struct platform_device *pdev)
{
	struct spe_plat_data *pdata = pdev->dev.platform_data;
	struct resource *res;
	void __iomem *regs;
    struct device *dev = &pdev->dev;
    struct spe *spe = &spe_balong;
	int ret = 0;
    int	irq;

    dev->dma_mask = &spe_dmamask;
    dev->coherent_dma_mask = DMA_BIT_MASK(64);
    dev->platform_data = kmalloc(sizeof(struct spe_plat_data), GFP_ATOMIC);

    /* don't need kfree platform_data in err process,
     * platform_driver_unregister will do it.
     */
    if (dev->platform_data) {
        memcpy(dev->platform_data, &spe_platdata, sizeof(spe_platdata));
    }
    else {
        SPE_ERR("%s:platform_data alloc fail.\n", __func__);
        ret = -ENOMEM;
        goto spe_probe_err0;
    }

    memset(spe, 0, offsetof(struct spe, dev));

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		SPE_ERR("missing resource\n");
		ret = -ENODEV;
		goto spe_probe_err1;
	}

	regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(regs)){
		SPE_ERR("ioremap failed\n");
		ret = -ENOMEM;
		goto spe_probe_err1;
	}
	
	spe_clk_enable(dev);

	irq = platform_get_irq_byname(pdev, "spe_irq");
	if (irq == -ENXIO) {
		SPE_ERR("SPE IRQ configuration information not found\n");
		ret = -ENXIO;
		goto spe_probe_err2;
	}

	platform_set_drvdata(pdev, spe);
	
	spe->res = res;
	spe->regs = regs;
	spe->dev = dev;
	spe->irq	= irq;

    /* sysctrl initialization */
    if(pdata && pdata->sysctrl_init){
        SPE_INFO("sysctrl init\n");
        pdata->sysctrl_init();
    }else{
        SPE_TRACE("missing platform data\n");
    }


	ret = spe_core_init(spe);
	if (ret) {
		SPE_ERR("spe_init fail, ret = %d\n", ret);
		goto spe_probe_err3;
	}

	ret = spe_bakspace_init(spe);
	if(ret) {
		SPE_ERR("failed to alloc memory for backup space\n");
		ret = -EIO;
		goto spe_probe_err_pm;
	}

    /* int cpu port */
    ret = spe_cpuport_init(spe);
    if(ret){
        SPE_ERR("failed to init cpuport\n");
        goto spe_probe_err4;
    }

    /* init debug fs */
    ret = spe_debugfs_init(spe);
	if (ret) {
		SPE_ERR("failed to initialize debugfs\n");
        ret = -EIO;
		goto spe_probe_err5;
	}

    ret = spe_filter_init();
    if (ret) {
		SPE_ERR("failed to initialize filter sysfs\n");
        ret = -EIO;
		goto spe_probe_err6;
	}

    spe_hook_register();

	ret = spe_enable(spe);
    if (ret) {
		SPE_ERR("failed to enable debugfs\n");
        ret = -EIO;
		goto spe_probe_err7;
	}

	return 0;

spe_probe_err7:
    spe_hook_unregister();

    spe_filter_exit();

spe_probe_err6:
    spe_debugfs_exit(spe);

spe_probe_err5:
    spe_cpuport_exit(spe);

spe_probe_err4:
    spe_core_exit(spe);

spe_probe_err_pm:
	spe_bakspace_deinit(spe);

spe_probe_err3:
    if(pdata && pdata->sysctrl_exit){
        pdata->sysctrl_exit();
    }

spe_probe_err2:
    devm_iounmap(dev, (void *)res->start);
    devm_release_mem_region(dev, res->start, resource_size(res));

spe_probe_err1:
    kfree(dev->platform_data);
		
spe_probe_err0:
	return ret;
}


static int spe_remove(struct platform_device *pdev)
{
    struct spe *spe = platform_get_drvdata(pdev);
	struct spe_plat_data *pdata = pdev->dev.platform_data;
    struct resource	*res;
    int ret;

	if(!spe){
		SPE_ERR("no spe exist!\n");
		return -EFAULT;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!res){
		SPE_ERR("get resouce failed!\n");
		return -EIO;
	}

	ret = spe_disable(spe);
    if (ret) {
		SPE_ERR("failed to diable spe\n");
	}

    spe_filter_exit();

    spe_debugfs_exit(spe);

    (void)spe_cpuport_exit(spe);

    (void)spe_core_exit(spe);

    devm_iounmap(&pdev->dev, (void *)res->start);
    devm_release_mem_region(&pdev->dev, res->start, resource_size(res));

    if(pdata && pdata->sysctrl_exit){
        pdata->sysctrl_exit();
    }

    spe_hook_unregister();

	return 0;
}

static const struct dev_pm_ops spe_dev_pm_ops ={
	.suspend_noirq = spe_suspend,
	.resume_noirq = spe_resume,
};

static const struct of_device_id spe_dt_ids[] = {
	{ .compatible = "hisilicon,spe"},
	{ /* sentinel */ }
};

static struct platform_driver spe_driver = {
	.probe = spe_probe,
	.remove = spe_remove,
	.driver =
	{
	   .name = SPE_MODULE_NAME,
	   .owner = THIS_MODULE,
	   .of_match_table = of_match_ptr(spe_dt_ids),
	   .pm = &spe_dev_pm_ops,
	},
};

module_platform_driver(spe_driver);

void spe_balong_exit(void)
{

    /* don't need kfree platform_data in exit process,
     * platform_driver_unregister will do it.
     */
    platform_driver_unregister(&spe_driver);

    return ;
}

module_exit(spe_balong_exit);

MODULE_DEVICE_TABLE(of, spe_dt_ids);
MODULE_AUTHOR("hisi bsp4 network");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("smart packet engine(spe) driver");

