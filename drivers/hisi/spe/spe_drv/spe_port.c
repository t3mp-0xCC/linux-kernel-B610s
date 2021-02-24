

/**
 * spe_port.c - spe port management
 *
 * Copyright (C) 2012-2013 HiSilicon Technologies Co.,LTD.- http://www.hisilicon.com
 *
 * Authors: hisi-bsp4
 *
 *
 */
#include <linux/netdevice.h>
#include <linux/spe/spe_interface.h>
#include "spe_port.h"
#include "spe_desc.h"
#include "spe_dbg.h"

static inline int spe_attr_check(struct spe_port_attr *attr)
{
    if(!attr){
        return -EINVAL;
    }

    if(spe_attach_brg_br == attr->attach_brg){
        if(!attr->net){
            SPE_ERR("attach_brg_br has no net.\n");
            return -EINVAL;
        }
    }else{
        /* only cpu port has no td */
        if((!attr->desc_ops.finish_td || !attr->td_depth)
            && (spe_enc_cpu != attr->enc_type)){
            SPE_ERR("port(not cpu) has no finish_td/td_depth.\n");
            return -EINVAL;
        }

        /* every port should has rd */
        if(!attr->desc_ops.finish_rd || !attr->rd_depth){
            SPE_ERR("port has no finish_rd/rd_depth.\n");
            return -EINVAL;
        }

        if(attr->rd_skb_num > attr->rd_depth){
            SPE_ERR("rd_skb_num(%d) > attr->rd_depth(%d).\n",
                attr->rd_skb_num,attr->rd_depth);
            return -EINVAL;
        }

        if(spe_enc_bottom <= attr->enc_type){
            SPE_ERR("invalid enc_type(%d).\n",attr->enc_type);
            return -EINVAL;
        }

        if(spe_attach_brg_bottom <= attr->attach_brg){
            SPE_ERR("invalid attach_brg(%d).\n",attr->attach_brg);
            return -EINVAL;
        }
    }

    return 0;
}

void spe_set_ipf_mac(enum spe_mac_type mac_type, struct ether_addr * mac_addr)
{
    struct spe *spe = &spe_balong;
    uint8_t *mac;
    uint8_t *spe_balong_mac;
    uint32_t mac_hi;
    uint32_t mac_lo;
    uint32_t mac_hi_addr;
    uint32_t mac_low_addr;
	if(spe_mac_dst == mac_type){
		mac_hi_addr = HI_SPE_GLB_DMAC_ADDR_H_OFFSET;
		mac_low_addr = HI_SPE_GLB_DMAC_ADDR_L_OFFSET;
		spe_balong_mac = spe->ipfport.dst_mac.octet;
	}
	else if(spe_mac_src == mac_type){
		mac_hi_addr = HI_SPE_GLB_SMAC_ADDR_H_OFFSET;
		mac_low_addr = HI_SPE_GLB_SMAC_ADDR_L_OFFSET;
		spe_balong_mac = spe->ipfport.src_mac.octet;
	}
	else{
		SPE_ERR("Unknow ipf mac type\n");
		return;
	}

    mac = mac_addr->octet;
    mac_lo = (mac[3] << 24) | (mac[2] << 16) | (mac[1] << 8) | mac[0];
    mac_hi = (mac[5]<<8) | mac[4];

    spe_writel(spe->regs, mac_low_addr, mac_lo);
    spe_writel(spe->regs, mac_hi_addr, mac_hi);
    memcpy(spe_balong_mac, mac_addr, ETHER_ADDR_LEN);


}

void spe_set_ipf_port(int portno, struct ether_addr * src_mac, struct ether_addr * dst_mac)
{
    struct spe *spe = &spe_balong;

    spe->ipfport.portno = portno;
    spe_set_ipf_mac(spe_mac_dst, dst_mac);
    spe_set_ipf_mac(spe_mac_src, src_mac);

    return;
}

void free_rd_mem(struct spe_port_ctrl *port_ctrl)
{
    if(port_ctrl->rd_free_q){
        struct sk_buff *skb = NULL;

        /* cancel skbs from rd desc to rd_free_q */
        /* TODO:rd maybe using by spe hardware. */

        /* free all skbs in rd_free_q */
        while(NULL != (skb = skb_dequeue(port_ctrl->rd_free_q))){
            dev_kfree_skb_any(skb);
        }

        kfree(port_ctrl->rd_free_q);
        port_ctrl->rd_free_q = NULL;
    }
}

int32_t alloc_rd_mem(struct spe_port_ctrl *port_ctrl)
{
    struct spe *spe = &spe_balong;
    struct sk_buff *skb = NULL;
    u32 cnt = 0;
    int ret = 0;

    if(!port_ctrl->rd_free_q){
        port_ctrl->rd_free_q =
            (struct sk_buff_head *)kzalloc(sizeof(struct sk_buff_head), GFP_ATOMIC);
        if(!port_ctrl->rd_free_q){
            SPE_ERR("alloc rd_free_q head failed\n");
            return -ENOMEM;
        }

        skb_queue_head_init(port_ctrl->rd_free_q);

        /* prealloc rx skbs */
        for(cnt = 0; cnt < port_ctrl->rd_skb_num; cnt++)
        {
            skb = alloc_skb(port_ctrl->rd_skb_size + port_ctrl->rd_skb_align, GFP_ATOMIC);
            if(!skb)
            {
                SPE_ERR("alloc rd skb failed\n");
                ret = -ENOMEM;
                goto nomem;
            }

			if (port_ctrl->portno != spe->cpuport.portno) {
				skb->spe_own = port_ctrl->portno;
                skb->dev = port_ctrl->net;
			}

            skb_queue_tail(port_ctrl->rd_free_q, skb);
        }
    }

    return 0;

nomem:
    free_rd_mem(port_ctrl);

    return ret;
}

int32_t fill_rd_desc(struct spe_port_ctrl *port_ctrl)
{
    struct spe *spe = &spe_balong;
    u32 cnt;
    u32 portno = port_ctrl->portno;
    dma_addr_t dma;

    for(cnt = 0; cnt < port_ctrl->rd_skb_num-1; cnt++){
        struct sk_buff *skb = skb_dequeue(port_ctrl->rd_free_q);
        if(skb){
            skb_reserve(skb, port_ctrl->rd_skb_align);
            dma = dma_map_single(spe->dev, skb->data, port_ctrl->rd_skb_size, DMA_FROM_DEVICE);
            if(dma){
				spe_set_skb_dma(skb, dma);
                (void)spe_rd_config(portno, skb, dma);
            }else{
                SPE_ERR("dma_map_single skb failed for %d desc.\n",cnt);
                break;
            }
        }else{
            SPE_ERR("no skb for %d desc.\n",cnt);
            break;
        }
    }

    port_ctrl->rd_skb_used = cnt;

    return cnt ? 0 : -ENOMEM;
}

int spe_port_exist(int portno)
{
    int isgrp = 0;
    struct spe *spe = &spe_balong;

    if(SPE_INVALID_PORT(portno)){
        SPE_ERR("invalid portno(%d)!\n", portno);
        return 0;
    }

    isgrp = (portno >= SPE_PORT_NUM)?1:0;

    return test_bit(portno, &spe->portmap[isgrp]);
}

int port_attach_brg_modify(int portno, enum spe_attach_brg_type attach_brg)
{
    struct spe *spe = &spe_balong;
    struct spe_port_ctrl *port_ctrl = NULL;
    HI_SPE_PORTX_PROPERTY_T ch_property;

	SPE_TRACE("enter\n");
    if(!SPE_NORM_PORT(portno)){
        SPE_ERR("invalid port no:%d.\n", portno);
        return -EINVAL;
    }

    if(!spe_port_exist(portno)){
        SPE_ERR("port(%d) is not exist.\n", portno);
        return -EINVAL;
    }

    if(attach_brg >= spe_attach_brg_bottom){
        SPE_ERR("invalid attach brg type:%d.\n", attach_brg);
        return -EINVAL;
    }

    port_ctrl = &spe->ports[portno].ctrl;

    ch_property.u32 = port_ctrl->property.u32;
    ch_property.bits.spe_port_attach_brg = attach_brg;
    port_ctrl->property.u32= ch_property.u32;

    spe_writel(spe->regs, SPE_CH_PROPERTY_OFFSET(portno), port_ctrl->property.u32);
    SPE_TRACE("leave\n");

    return 0;
}


int port_set_mac(int portno, char *mac)
{
    struct spe *spe = &spe_balong;
    uint32_t mac_hi;
    uint32_t mac_lo;

    if(!SPE_NORM_PORT(portno)){
        SPE_ERR("invalid port no:%d.\n", portno);
        return -EINVAL;
    }

    if(!spe_port_exist(portno)){
        SPE_ERR("port(%d) is not exist.\n", portno);
        return -EINVAL;
    }

    if(!mac){
        SPE_ERR("mac is null ptr.\n");
        return -EINVAL;
    }

    mac_lo = (mac[0]) | (mac[1] << 8) | (mac[2] << 16) | (mac[3] << 24);
    mac_hi = (mac[4]) | (mac[5]<<8);
    spe_writel(spe->regs, SPE_CH_MAC_ADDR_L(portno), mac_lo);
    spe_writel(spe->regs, SPE_CH_MAC_ADDR_H(portno), mac_hi);

    return 0;
}

int port_set_ipaddr(int portno, bool is_ipv6, u32 *ip_addr)
{
    struct spe *spe = &spe_balong;

    if(!SPE_NORM_PORT(portno)){
        SPE_ERR("invalid port no:%d.\n", portno);
        return -EINVAL;
    }

    if(!spe_port_exist(portno)){
        SPE_ERR("port(%d) is not exist.\n", portno);
        return -EINVAL;
    }

    if(!ip_addr){
        SPE_ERR("ip_addr is null ptr.\n");
        return -EINVAL;
    }

    if(is_ipv6){
        spe_writel(spe->regs, SPE_CH_IPV6_ADDR_1ST(portno), ip_addr[0]);
        spe_writel(spe->regs, SPE_CH_IPV6_ADDR_2ND(portno), ip_addr[1]);
        spe_writel(spe->regs, SPE_CH_IPV6_ADDR_3RD(portno), ip_addr[2]);
        spe_writel(spe->regs, SPE_CH_IPV6_ADDR_4TH(portno), ip_addr[3]);
    }else{
        spe_writel(spe->regs, SPE_CH_IPV4_ADDR(portno), ip_addr[0]);
    }

    return 0;
}


int spe_port_pri_set(int portno, int token)
{
    struct spe *spe = &spe_balong;
    HI_SPE_TDQX_PRI_0_T pri;

    if (0 == (token & SPE_PORT_PRIO_MASK)) {
        token = SPE_DFT_PORT_PRIO;
    }
    pri.u32 = spe_readl(spe->regs, SPE_TDQ_PRI_OFFSET(portno));
    pri.bits.spe_tdq_pri= token;
    spe_writel(spe->regs, SPE_TDQ_PRI_OFFSET(portno), pri.u32);

    return 0;
}


int port_set_flow_cfg(int portno, enum spe_port_ioctl_cmd cfg, uint32_t param)
{
    struct spe *spe = &spe_balong;

    if(!SPE_NORM_PORT(portno)){
        SPE_ERR("invalid port no:%d.\n", portno);
        return -EINVAL;
    }

    if(!spe_port_exist(portno)){
        SPE_ERR("port(%d) is not exist.\n", portno);
        return -EINVAL;
    }

    if(!param){
        SPE_ERR("param is null ptr.\n");
        return -EINVAL;
    }

    switch(cfg){
        case spe_port_ioctl_set_udp_time:
            spe->ports[portno].ctrl.udp_limit_time = *((unsigned int *)param);
            spe_writel(spe->regs, SPE_CH_UDP_LIMIT_TIME_OFFSET(portno), *((unsigned int *)param));
            break;

        case spe_port_ioctl_set_udp_cnt:
            spe->ports[portno].ctrl.udp_limit_cnt = param;
            spe_writel(spe->regs, SPE_CH_UDP_LIMIT_CNT_OFFSET(portno), SPE_US_TO_TIMERCNT(param));
            break;

        case spe_port_ioctl_set_rate_time:
            spe->ports[portno].ctrl.rate_limit_time = param;
            spe_writel(spe->regs, SPE_CH_RATE_LIMIT_TIME_OFFSET(portno), SPE_US_TO_TIMERCNT(param));
            break;

        case spe_port_ioctl_set_rate_byte:
            spe->ports[portno].ctrl.rate_limit_byte = *((unsigned int *)param);
            spe_writel(spe->regs, SPE_CH_RATE_LIMIT_BYTE_OFFSET(portno), *((unsigned int *)param));
            break;

        default:
            SPE_ERR("invalid cfg value(%d)!\n", cfg);
            break;
    }

    return 0;
}

int spe_set_bypass_ops(struct spe_port_ops *ops)
{
	int cpuport_num = spe_balong.cpuport.portno;
	struct spe_port_ctrl *port_ctrl = &spe_balong.ports[cpuport_num].ctrl;

	if(NULL == ops){
        return -EINVAL;
	}

	port_ctrl->desc_ops.finish_rd_bypass = ops->finish_rd_bypass;

    return 0;
}

void spe_port_set_property(struct net_device *net, struct spe_port_attr_alt *attr, int portno)
{
    struct spe *spe = &spe_balong;
    struct spe_port_ctrl *port_ctrl = NULL;
    HI_SPE_PORTX_PROPERTY_T property;

    port_ctrl = &spe->ports[portno].ctrl;
    port_ctrl->net = net; 

    if(NULL == net || NULL == attr){
        return;
    }
    
    port_ctrl->net->spe_portno = portno;
    port_ctrl->net->spe_alloced = 1;
    port_set_mac(portno, port_ctrl->net->dev_addr);

    if(NULL != attr->desc_ops.finish_td){
        port_ctrl->desc_ops.finish_td = attr->desc_ops.finish_td;
    }
    if(NULL != attr->desc_ops.finish_rd){
        port_ctrl->desc_ops.finish_rd = attr->desc_ops.finish_rd;
    }
    if(NULL != attr->desc_ops.finish_rd_bypass){
        port_ctrl->desc_ops.finish_rd_bypass = attr->desc_ops.finish_rd_bypass;
    }

    property.u32 = spe_readl(spe->regs, SPE_CH_PROPERTY_OFFSET(portno));
    property.bits.spe_port_enc_type = attr->enc_type;
    spe_writel(spe->regs, SPE_CH_PROPERTY_OFFSET(portno),property.u32);/* in disable state */

}

int spe_port_ioctl(int portno, int cmd, int param)
{    
	enum spe_attach_brg_type attach_brg;	
	printk("in %s \n",__func__);  
	switch(cmd){        
		case spe_port_ioctl_set_attach_brg:            
			attach_brg = param;            
			return port_attach_brg_modify(portno,attach_brg); 
			
		case spe_port_ioctl_set_mac:            
			return port_set_mac(portno, (char *)param); 
			
		case spe_port_ioctl_set_ipv4:        
		case spe_port_ioctl_set_ipv6:            
			return port_set_ipaddr(portno, spe_port_ioctl_set_ipv6==cmd, (u32 *)param);        

		case spe_port_ioctl_set_udp_time:        
		case spe_port_ioctl_set_udp_cnt:        
		case spe_port_ioctl_set_rate_time:       
		case spe_port_ioctl_set_rate_byte:           
			return port_set_flow_cfg(portno, cmd, param);        

		default:            
			SPE_ERR("unsupport cmd value(%d).\n", cmd);            
			break;   
		}
	
	return 0;
}

int spe_port_alloc(struct spe_port_attr *attr)
{
    struct spe *spe = &spe_balong;
    struct spe_port_ctx *port_ctx = NULL;
    struct spe_port_ctrl *port_ctrl = NULL;
    unsigned int portno = 0;
    int isbrg = 0;
    int ret = 0;
    unsigned long flag;
    spinlock_t * port_alloc_lock = &spe->port_alloc_lock;
    HI_SPE_PACK_CTRL_T usb_pack_ctrl;
    union spe_desc_queue_ptr rd_ptr;
    union spe_desc_queue_ptr td_ptr;
    HI_SPE_MODE_T spe_stick_reg;


    /* check validity of attr param */
    if(0 != (ret = spe_attr_check(attr))){
        SPE_ERR("attr check failed!\n");
        return ret;
    }
    spin_lock_irqsave(port_alloc_lock, flag);
    /* get port index */
    isbrg = (spe_attach_brg_br==attr->attach_brg)?1:0;
    portno = find_first_zero_bit (&spe->portmap[isbrg], SPE_PORT_NUM);

    if( (portno >= SPE_PORT_NUM)&&(!isbrg)) {
        SPE_ERR("too many ports\n");
        ret = -EBUSY;
        spin_unlock_irqrestore(port_alloc_lock, flag);
        goto error_no_port;
    }
    set_bit (portno, &spe->portmap[isbrg]);
    if(isbrg){/* bridge has only software resource */
        SPE_INFO("alloc a bridge chan.\n");
        spin_unlock_irqrestore(port_alloc_lock, flag);
        return portno + SPE_PORT_NUM;
    }

    port_ctx = &spe->ports[portno];

    memset(&port_ctx->stat, 0, offsetof(struct spe_port_stat, port_alloc_cnt));
    port_ctx->stat.port_alloc_cnt++;

    port_ctrl = &port_ctx->ctrl;

    /* copy param from attr */
    port_ctrl->property.bits.spe_port_attach_brg = attr->attach_brg;
    port_ctrl->property.bits.spe_port_enc_type = attr->enc_type;
    port_ctrl->property.bits.spe_port_pad_en = attr->padding_enable;

    port_ctrl->td_depth = attr->td_depth;
    port_ctrl->rd_depth = attr->rd_depth;
    port_ctrl->rd_skb_size = attr->rd_skb_size;
    port_ctrl->rd_skb_num = attr->rd_skb_num;
    port_ctrl->desc_ops.finish_rd = attr->desc_ops.finish_rd;
    port_ctrl->desc_ops.finish_td = attr->desc_ops.finish_td;
    port_ctrl->net = attr->net;
    port_ctrl->port_priv = attr->port_priv;
    port_ctrl->stick_mode = attr->stick_mode;

    /* set some default value
     *
     */
    /* rate limit */
    port_ctrl->udp_limit_time = PORT_UDP_LIMIT_TIME_DEF;
    port_ctrl->udp_limit_cnt = PORT_UDP_LIMIT_CNT_DEF;
    port_ctrl->rate_limit_time = PORT_RATE_LIMIT_TIME_DEF;
    port_ctrl->rate_limit_byte = PORT_RATE_LIMIT_BYTE_DEF;

    port_ctrl->td_evt_gap = PORT_TD_EVT_GAP_DEF;
    port_ctrl->rd_skb_align = PORT_RD_SKB_ALIGN_DEF;
    port_ctrl->rd_evt_gap = PORT_RD_EVT_GAP_DEF;

    port_ctrl->rd_skb_align = PORT_RD_SKB_ALIGN_DEF;

/* B612 WiFiÐ¾Æ¬Ê¹ÓÃ BCM43217 */

    /* alloc td desc */
	if (port_ctrl->td_depth) {
	    port_ctrl->td_addr = (u32)dma_alloc_coherent(spe->dev,
	                    port_ctrl->td_depth*sizeof(struct spe_td_desc),
	                    &port_ctrl->td_dma, GFP_KERNEL);
        SPE_TRACE("td_addr is 0x%x \n", port_ctrl->td_addr);
	    if(!port_ctrl->td_addr){
	        SPE_ERR("port %d alloc td pool failed.\n",portno);
	        ret = -ENOMEM;
            spin_unlock_irqrestore(port_alloc_lock, flag);
	        goto error_alloc_td_desc;
	    }
	}

	/* when disable / enable port, td may be not in start pos.
     * we update the free / busy slot from the hardware.
     */
    td_ptr.raw = spe_readl(spe->regs, SPE_TDQ_PTR_OFFSET(portno));
    port_ctrl->td_free = td_ptr.bits.wptr;
    port_ctrl->td_busy = td_ptr.bits.rptr;
    spin_lock_init(&port_ctrl->td_lock);
    SPE_ERR("%s, restore td pos td_free:%d, td_busy:%d\n",
        __func__, port_ctrl->td_free, port_ctrl->td_busy);

    /* alloc rd desc */
	if (port_ctrl->rd_depth) {
	    port_ctrl->rd_addr = (u32)dma_alloc_coherent(spe->dev,
	                    port_ctrl->rd_depth*sizeof(struct spe_rd_desc),
	                    &port_ctrl->rd_dma, GFP_KERNEL);
	    if(!port_ctrl->rd_addr){
	        SPE_ERR("port %d alloc rd pool failed.\n",portno);
	        ret = -ENOMEM;
            spin_unlock_irqrestore(port_alloc_lock, flag);
	        goto error_alloc_rd_desc;
	    }
	}

	/* when disable / enable port, rd may be not in start pos.
     * we update the free / busy slot from the hardware.
     * rd_busy must restore to rptr
     */
	rd_ptr.raw = spe_readl(spe->regs, SPE_RDQ_PTR_OFFSET(portno));
    /* be care, rd_busy must restore to rptr */
    port_ctrl->rd_busy = rd_ptr.bits.rptr;
    port_ctrl->rd_free = port_ctrl->rd_busy;
    spin_lock_init(&port_ctrl->rd_lock);
    SPE_ERR("%s, restore rd pos rd_free:%d, rd_busy:%d\n",
        __func__, port_ctrl->rd_free, port_ctrl->rd_busy);

    spin_lock_init(&port_ctrl->lock);

    port_ctrl->portno = portno;
 
    if(port_ctrl->stick_mode){
        SPE_BUG(("stick src and dst port has been registered.\n"),
            (spe_mode_stick == spe->stick_ctrl.src_status
            && spe_mode_stick == spe->stick_ctrl.dst_status));

        if(spe_mode_stick != spe->stick_ctrl.src_status){
            spe->stick_ctrl.src_status = spe_mode_stick;
			spe->stick_ctrl.src_port = portno;
			spe->spe_mode = spe_mode_normal;
        }else{
			spe->stick_ctrl.dst_status = spe_mode_stick;
			spe->stick_ctrl.dst_port = portno;

			spe_stick_reg.bits.spe_mode = spe_mode_stick;
			spe_stick_reg.bits.spe_stick_sportnum = spe->stick_ctrl.src_port;
			spe_stick_reg.bits.spe_stick_dportnum = spe->stick_ctrl.dst_port;
			spe_writel(spe->regs, HI_SPE_MODE_OFFSET, spe_stick_reg.u32);
       			spe->spe_mode = spe_mode_stick;
	 }
    }

    spe_writel(spe->regs, SPE_PORT_NUM_OFFSET(portno), portno);

    /* td queue */
    spe_writel(spe->regs, SPE_TDQ_BADDR_OFFSET(portno), port_ctrl->td_dma);
    spe_writel(spe->regs, SPE_TDQ_LEN_OFFSET(portno), port_ctrl->td_depth);

    /* rd queue */
    spe_writel(spe->regs, SPE_RDQ_BADDR_OFFSET(portno), port_ctrl->rd_dma);
    spe_writel(spe->regs, SPE_RDQ_LEN_OFFSET(portno), port_ctrl->rd_depth);

    /* usb wrap parameter set, but pkt num limit not set */
    if(attr->enc_type == spe_enc_ncm_ntb32 ||
       attr->enc_type == spe_enc_ncm_ntb16) {

		spe_writel(spe->regs, HI_SPE_PACK_ADDR_CTRL_OFFSET, attr->ncm_align_parameter);
		/* ncm unpack setting */
    	spe_writel(spe->regs, HI_SPE_UNPACK_CTRL_OFFSET, attr->ncm_max_size);

        /* ncm pack setting */
    	usb_pack_ctrl.u32 = spe_readl(spe->regs, HI_SPE_PACK_CTRL_OFFSET);
        usb_pack_ctrl.bits.spe_pack_max_pkt_cnt = 8;
    	usb_pack_ctrl.bits.spe_pack_max_len = attr->rd_skb_size;
    	spe_writel(spe->regs, HI_SPE_PACK_CTRL_OFFSET, usb_pack_ctrl.u32);
    }

    if (attr->enc_type == spe_enc_rndis) {
        /* rndis pack setting */
        usb_pack_ctrl.u32 = spe_readl(spe->regs, HI_SPE_PACK_CTRL_OFFSET);
    	usb_pack_ctrl.bits.spe_pack_max_len = attr->rd_skb_size;
    	spe_writel(spe->regs, HI_SPE_PACK_CTRL_OFFSET, usb_pack_ctrl.u32);
    }

    /* alloc rd mem */
    ret = alloc_rd_mem(port_ctrl);
    if(ret){
        SPE_ERR("port %d alloc rd mem failed.\n",portno);
        spin_unlock_irqrestore(port_alloc_lock, flag);
        goto error_alloc_rd_mem;
    }


    /* port property */
    printk("new_reg: port %u sit property to 0x%x write addr 0x%x \n",
		portno, port_ctrl->property.u32, (unsigned int)(SPE_CH_PROPERTY_OFFSET(portno)+spe->regs));

    spe_writel(spe->regs, SPE_CH_PROPERTY_OFFSET(portno),port_ctrl->property.u32);/* in disable state */

    spe_port_pri_set(portno, attr->prio);

    /* config mac addr */
	if (port_ctrl->net) {
        if (!port_ctrl->net->spe_alloced){
            port_ctrl->net->spe_portno = portno;
            port_ctrl->net->spe_alloced = 1;
        }
    	port_set_mac(portno, port_ctrl->net->dev_addr);
	} else {
		SPE_TRACE("no dev\n");
	}

	if(spe_enc_ipf == attr->enc_type){
       spe_set_ipf_port(portno, &attr->ipf_src_mac, &attr->ipf_dst_mac);
	}

	port_ctrl->rd_desc_filled = 0;
	port_ctrl->wport_push = 0;
	
    spin_unlock_irqrestore(port_alloc_lock, flag);
    return portno;


error_alloc_rd_mem:
    if(port_ctrl->rd_addr){
        dma_free_coherent(spe->dev,
            port_ctrl->rd_depth*sizeof(struct spe_rd_desc),
            (void *)port_ctrl->rd_addr,
            port_ctrl->rd_dma);
        port_ctrl->rd_addr = 0;
        port_ctrl->rd_dma = 0;
    }

error_alloc_rd_desc:
    if(port_ctrl->td_addr){
        dma_free_coherent(spe->dev,
            port_ctrl->td_depth*sizeof(struct spe_td_desc),
            (void *)port_ctrl->td_addr,
            port_ctrl->td_dma);
        port_ctrl->td_addr = 0;
        port_ctrl->td_dma = 0;
    }

error_alloc_td_desc:
    clear_bit (portno, &spe->portmap[isbrg]);

error_no_port:
    return ret;
}

static int spe_port_wait_td_complete(int portno, uint32_t timeout)
{
    union spe_event event;
    unsigned long flags;
    HI_SPE_IDLE_T idle;
    struct spe *spe = &spe_balong;
    struct spe_port_ctx *port_ctx = NULL;
    struct spe_port_ctrl *port_ctrl = NULL;
    union spe_desc_queue_ptr td_ptr;

    port_ctx = &spe->ports[portno];
    port_ctrl = &port_ctx->ctrl;
	do {
	    /* complete the td desc ...
	     * we don't set irq_en to all td(s),
	     * may be last td(s) complete can't issue the td_complete interrupt.
	     * so we must check the complete td(s) here.
	     */
	    event.bits.portno = portno;
	    /* spinlock is used in spe_td_complete, just lock the irq */
	    local_irq_save(flags);
	    spe_td_complete(spe, &event);
	    local_irq_restore(flags);

    	/* judge spe_idle */
    	spin_lock_irqsave(&port_ctrl->lock, flags);
    	idle.u32 = spe_readl(spe->regs, HI_SPE_IDLE_OFFSET);

	    /* make sure td queue is empty */
	    td_ptr.raw = spe_readl(spe->regs, SPE_TDQ_PTR_OFFSET(portno));

	    if (idle.bits.spe_port_idle &(1<<portno)
			&& (port_ctx->ctrl.td_busy == td_ptr.bits.rptr)){
			spin_unlock_irqrestore(&port_ctrl->lock, flags);
        	break;
    	}
		spin_unlock_irqrestore(&port_ctrl->lock, flags);
		
		if (0 >= timeout--) {
            port_ctx->stat.free_busy++;
			SPE_ERR("spe port %d wait idle timed out\n",portno);

            return SPE_ERR_PORT_BUSY;
		}

		udelay(1);
	} while (1);

    return 0;
}

int spe_port_free(int portno)
{
    struct spe *spe = &spe_balong;
    struct spe_port_ctx *port_ctx = NULL;
    struct spe_port_ctrl *port_ctrl = NULL;
    int isgrp = 0;
    int ret;
    unsigned long flags;

    isgrp = (portno >= SPE_PORT_NUM)?1:0;
	if(isgrp)
		portno -= SPE_PORT_NUM;
		
    if(!(portno&(spe->portmap[isgrp]))){
        SPE_ERR("port %d not allocated!\n",portno);
        return -ENOENT;
    }

    if(isgrp){
        return 0;
    }

    port_ctx = &spe->ports[portno];
    port_ctrl = &port_ctx->ctrl;

    port_ctx->stat.port_free_cnt++;

    /* port must be disabled before port free */
    spin_lock_irqsave(&port_ctrl->lock, flags);
    if(port_ctrl->flags & SPE_PORT_ENABLED){
        spin_unlock_irqrestore(&port_ctrl->lock, flags);

        port_ctx->stat.free_while_enabled++;
        SPE_ERR("port %d free while enabled!\n",portno);
        return SPE_ERR_PORT_ENABLED;
    }
    spin_unlock_irqrestore(&port_ctrl->lock, flags);

    /* wait for port not busy. after port idle,td/rd queue is empty. */
	ret = spe_port_wait_td_complete(portno, 1000);
    if (ret) {
        return ret;
    }

    /* rewind the rd desc */
    spe_rd_rewind(spe, portno);

    /* free td queue */
    if(port_ctrl->td_addr){
        dma_free_coherent(spe->dev,
            port_ctrl->td_depth*sizeof(struct spe_td_desc),
            (void *)port_ctrl->td_addr,
            port_ctrl->td_dma);
        port_ctrl->td_addr = 0;
        port_ctrl->td_dma = 0;
    }

    /* free rx mem */
    spin_lock_irqsave(&port_ctrl->lock, flags);
    free_rd_mem(port_ctrl);
    spin_unlock_irqrestore(&port_ctrl->lock, flags);

    /* free rd queue */
    if(port_ctrl->rd_addr){
        dma_free_coherent(spe->dev,
            port_ctrl->rd_depth*sizeof(struct spe_rd_desc),
            (void *)port_ctrl->rd_addr,
            port_ctrl->rd_dma);
        port_ctrl->rd_addr = 0;
        port_ctrl->rd_dma = 0;
    }

    memset(&port_ctx->stat, 0, offsetof(struct spe_port_stat, port_alloc_cnt));

    if(port_ctrl->net){
        port_ctrl->net->spe_alloced = 0;
        port_ctrl->net->spe_portno = 0;
    }

	clear_bit(portno, &spe->portmap[isgrp]);
	
    return 0;
}

void *spe_port_priv(int portno)
{
    struct spe *spe = &spe_balong;

    return spe->ports[portno].ctrl.port_priv;
}

struct net_device *spe_port_netdev(int portno)
{
    struct spe *spe = &spe_balong;

    return spe->ports[portno].ctrl.net;
}

int spe_port_enable(int portno)
{
    struct spe *spe = &spe_balong;
    HI_SPE_PORTX_PROPERTY_T property;
    struct spe_port_ctrl *port_ctrl = NULL;
    unsigned long flags = 0;
    int ret;

    if(!spe_port_exist(portno)){
        return -ENOENT;
    }

    port_ctrl = &spe->ports[portno].ctrl;

	if(!port_ctrl->rd_desc_filled){
    	spin_lock_irqsave(&port_ctrl->rd_lock, flags);
    	ret = fill_rd_desc(port_ctrl);	
    	spin_unlock_irqrestore(&port_ctrl->rd_lock, flags);
    	if (ret) {
        	return ret;
    	}
		port_ctrl->rd_desc_filled = 1;
	}
	
    spin_lock_irqsave(&port_ctrl->lock, flags);

    if(port_ctrl->flags & SPE_PORT_ENABLED){
        spin_unlock_irqrestore(&port_ctrl->lock, flags);
        SPE_DBG("port %d is enabled\n", portno);
        return 0;
    }

    property.u32 = spe_readl(spe->regs, SPE_CH_PROPERTY_OFFSET(portno));
    property.bits.spe_port_en = 1;
    spe->ports[portno].ctrl.property.u32 = property.u32;
    spe_writel(spe->regs, SPE_CH_PROPERTY_OFFSET(portno), property.u32);

    port_ctrl->flags |= SPE_PORT_ENABLED;

    spin_unlock_irqrestore(&port_ctrl->lock, flags);

    return 0;
}

int spe_port_is_enable(int portno)
{
    struct spe *spe = &spe_balong;
    struct spe_port_ctrl *port_ctrl = &spe->ports[portno].ctrl;
    
    if (port_ctrl->flags & SPE_PORT_ENABLED) 
        return 1;
    
    return 0;  
}

int spe_port_disable(int portno)
{
    struct spe *spe = &spe_balong;
    HI_SPE_PORTX_PROPERTY_T property;
    struct spe_port_ctrl *port_ctrl = NULL;
    unsigned long flags = 0;
    int ret;

	SPE_TRACE("enter\n");

    if(!spe_port_exist(portno)){
        return -ENOENT;
    }

    port_ctrl = &spe->ports[portno].ctrl;

    spin_lock_irqsave(&port_ctrl->lock, flags);
    if(!(port_ctrl->flags & SPE_PORT_ENABLED)){
        spin_unlock_irqrestore(&port_ctrl->lock, flags);
        SPE_DBG("port %d is disabled\n", portno);
        return 0;
    }
    spin_unlock_irqrestore(&port_ctrl->lock, flags);

    property.u32 = spe_readl(spe->regs, SPE_CH_PROPERTY_OFFSET(portno));
    property.bits.spe_port_en = 0;
    port_ctrl->property.u32 = property.u32;
    spe_writel(spe->regs, SPE_CH_PROPERTY_OFFSET(portno), property.u32);
	port_ctrl->flags &= ~SPE_PORT_ENABLED;

    ret = spe_port_wait_td_complete(portno, 1000);

	SPE_TRACE("leave\n");

    return ret;
}

int spe_port_l4portno_to_cpu_add(unsigned short portnum, int l4_type)
{
    struct spe *spe = &spe_balong;
    HI_SPE_L4_PORTNUM_CFG_0_T l4_portnum_cfg;
    int i = 0;
    int cfg = 0;

    switch(l4_type){
    case SPE_UDP:
        cfg = cpu_to_be16(portnum) | BIT(17);
        break;
    case SPE_TCP:
        cfg = cpu_to_be16(portnum) | BIT(16);
        break;
    default:
        SPE_ERR("L4 type %d is not supported! \n", l4_type);
        return -EPERM;
    }

    spe_port_l4portno_to_cpu_del(portnum, l4_type);
    for(; i < MAX_L4_PORTNUM_CFG_NUM; i++){
        l4_portnum_cfg.u32 = spe_readl(spe->regs, SPE_L4_PORTNUM_CFG_OFFSET(i));
        if(!l4_portnum_cfg.u32){
            l4_portnum_cfg.bits.spe_l4_portnum_cfg = cfg;
            spe_writel(spe->regs, SPE_L4_PORTNUM_CFG_OFFSET(i), l4_portnum_cfg.u32);
            return 0;
    }
    }
    SPE_ERR("l4_portnum_cfg already full! \n");
    return -ENOMEM;
}

int spe_port_l4portno_to_cpu_del(unsigned short portnum, int l4_type)
{
    struct spe *spe = &spe_balong;
    HI_SPE_L4_PORTNUM_CFG_0_T l4_portnum_cfg;
    int i = 0;
    int cfg = 0;

    switch(l4_type){
    case SPE_UDP:
        cfg = cpu_to_be16(portnum) | BIT(17);
        break;
    case SPE_TCP:
        cfg = cpu_to_be16(portnum) | BIT(16);
        break;
    default:
        SPE_ERR("L4 type %d is not supported! \n", l4_type);
        return -EPERM;
    }

    for(; i < MAX_L4_PORTNUM_CFG_NUM; i++){
        l4_portnum_cfg.u32 = spe_readl(spe->regs, SPE_L4_PORTNUM_CFG_OFFSET(i));
        if(l4_portnum_cfg.bits.spe_l4_portnum_cfg == cfg){
            l4_portnum_cfg.bits.spe_l4_portnum_cfg = 0;
            spe_writel(spe->regs, SPE_L4_PORTNUM_CFG_OFFSET(i), l4_portnum_cfg.u32);
            return 0;
        }
    }
    return -ENOMEM;
}

void spe_port_l4portno_to_cpu_dump(void)
{
    struct spe *spe = &spe_balong;
    HI_SPE_L4_PORTNUM_CFG_0_T l4_portnum_cfg;
    unsigned short portnum = 0;
    int i = 0;
    printk("==================l4_portnum start=====================\n");
    for(; i < MAX_L4_PORTNUM_CFG_NUM; i++){
        l4_portnum_cfg.u32 = spe_readl(spe->regs, SPE_L4_PORTNUM_CFG_OFFSET(i));
        if(l4_portnum_cfg.u32){
            if(l4_portnum_cfg.bits.spe_l4_portnum_cfg & BIT(17)){
                portnum = be16_to_cpu((unsigned short)(l4_portnum_cfg.bits.spe_l4_portnum_cfg & 0xFFFF));
                printk("[UDP] l4_portnum : %d \n", portnum);
                continue;
            }else if(l4_portnum_cfg.bits.spe_l4_portnum_cfg & BIT(16)){
                portnum = be16_to_cpu((unsigned short)(l4_portnum_cfg.bits.spe_l4_portnum_cfg & 0xFFFF));
                printk("[TCP] l4_portnum : %d \n", portnum);
            }else{
                l4_portnum_cfg.bits.spe_l4_portnum_cfg = 0;
                spe_writel(spe->regs, SPE_L4_PORTNUM_CFG_OFFSET(i), l4_portnum_cfg.u32);
            }
        }
    }
    printk("==================l4_portnum end=====================\n");
}

MODULE_LICENSE("GPL");

