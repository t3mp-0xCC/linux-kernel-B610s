/*
 * u_ether_balong.c -- Ethernet-over-USB link layer utilities for Gadget stack
 *
 * Copyright (C) 2003-2005,2008 David Brownell
 * Copyright (C) 2003-2004 Robert Schwebel, Benedikt Spranger
 * Copyright (C) 2008 Nokia Corporation
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2016. Dongyue Chen  <foss@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/* #define VERBOSE_DEBUG */
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gfp.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
#include <linux/spe/spe_interface.h>
#endif

#include "u_ether_balong.h"

/*
 * This component encapsulates the Ethernet link glue needed to provide
 * one (!) network link through the USB gadget stack, normally "usb0".
 *
 * The control and data models are handled by the function driver which
 * connects to this code; such as CDC Ethernet (ECM or EEM),
 * "CDC Subset", or RNDIS.  That includes all descriptor and endpoint
 * management.
 *
 * Link level addressing is handled by this component using module
 * parameters; if no such parameters are provided, random link level
 * addresses are used.  Each end of the link uses one address.  The
 * host end address is exported in various ways, and is often recorded
 * in configuration databases.
 *
 * The driver which assembles each configuration using such a link is
 * responsible for ensuring that each configuration includes at most one
 * instance of is network link.  (The network layer provides ways for
 * this single "physical" link to be used by multiple virtual links.)
 */

#define UETH__VERSION   "29-May-2008"

unsigned int gnet_msg_level = GNET_LEVEL_ERR;
unsigned int gnet_dbg_level = GNET_DBG_RX2ADS;
static struct eth_dev *the_dev[GNET_USED_NUM] = {0};
static unsigned int geth_idx = 0;
unsigned int glink_idx = 0;
struct sk_buff_head *g_rx_skb_free_q = NULL;
struct sk_buff_head *g_rx_skb_done_q = NULL;


/*-------------------------------------------------------------------------*/

#define RX_EXTRA    20  /* bytes guarding against rx overflows */

#define DEFAULT_QLEN    2   /* double buffering by default */

static unsigned qmult = 32;
module_param(qmult, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(qmult, "queue length multiplier at high/super speed");

static unsigned qmult_rx = CONFIG_GNET_PREALLOC_USB_RX_SKB_NUM/DEFAULT_QLEN;
module_param(qmult_rx, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(qmult_rx, "rx queue length multiplier at high/super speed");

/* for dual-speed hardware, use deeper queues at high/super speed */
static inline int qlen(struct usb_gadget *gadget)
{
    if (gadget_is_dualspeed(gadget) && (gadget->speed == USB_SPEED_HIGH ||
                        gadget->speed == USB_SPEED_SUPER))
        return qmult * DEFAULT_QLEN;
    else
        return DEFAULT_QLEN;
}

#ifndef DWC3_TRB_NUM
#define DWC3_TRB_NUM 128 /* should be consistent with the definition at core.h of dwc3 */
#endif
static inline int qlen_rx(struct usb_gadget *gadget)
{
    /* it's useless if the num of rx request is greater than trb num */
    if (gadget_is_dualspeed(gadget) && (gadget->speed == USB_SPEED_HIGH ||
                        gadget->speed == USB_SPEED_SUPER))
        return min_t(size_t, qmult_rx * DEFAULT_QLEN, DWC3_TRB_NUM);
    else
        return DEFAULT_QLEN;
}

/*-------------------------------------------------------------------------*/

/* REVISIT there must be a better way than having two sets
 * of debug calls ...
 */

#undef DBG
#undef VDBG
#undef ERROR
#undef INFO

#define xprintk(d, level, fmt, args...) \
    printk(level "%s: " fmt , (d)->net->name , ## args)

#ifdef DEBUG
#undef DEBUG
#define DBG(dev, fmt, args...) \
    xprintk(dev , KERN_DEBUG , fmt , ## args)
#else
#define DBG(dev, fmt, args...) \
    do { } while (0)
#endif /* DEBUG */

#ifdef VERBOSE_DEBUG
#define VDBG    DBG
#else
#define VDBG(dev, fmt, args...) \
    do { } while (0)
#endif /* DEBUG */

#define ERROR(dev, fmt, args...) \
    xprintk(dev , KERN_ERR , fmt , ## args)
#define INFO(dev, fmt, args...) \
    xprintk(dev , KERN_INFO , fmt , ## args)

int bsp_usb_netmode(void)
{
    bsp_usb_nv_init();
    return bsp_usb_is_ncm_bypass_mode();
}
EXPORT_SYMBOL(bsp_usb_netmode);

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
/*Only the first ncm device use spe.*/
#define SPE_GNET 0 

#if (SPE_GNET >= SPE_PORT_NUM)
#error "SPE_GNET must be < SPE_PORT_NUM"
#endif

unsigned int g_usb_portno;

extern u32 ncm_get_div_and_rem(void);
extern u32 ncm_get_max_length(void);

static void rx_spe_complete(struct usb_ep *ep, struct usb_request *req);
static void gnet_recycle_spe_rd(struct sk_buff *skb, struct eth_dev *dev);
#endif

static void rx_complete(struct usb_ep *ep, struct usb_request *req);
static inline void free_tx_ctx(tx_ctx_t *tx_ctx);

/*-------------------------------------------------------------------------*/

/* NETWORK DRIVER HOOKUP (to the layer above this driver) */

static int ueth_change_mtu(struct net_device *net, int new_mtu)
{
    struct eth_dev  *dev = netdev_priv(net);
    unsigned long   flags;
    int     status = 0;

    /* don't change MTU on "live" link (peer won't know) */
    spin_lock_irqsave(&dev->lock, flags);
    if (dev->port_usb)
        status = -EBUSY;
    else if (new_mtu <= ETH_HLEN || new_mtu > ETH_FRAME_LEN)
        status = -ERANGE;
    else
        net->mtu = new_mtu;
    spin_unlock_irqrestore(&dev->lock, flags);

    return status;
}

static int	ueth_set_mac_address(struct net_device *dev, void *addr)
{
	int ret=0;
	
	ret = eth_mac_addr(dev, addr);
	if(ret)
	{
		printk("eth_mac_addr return %d \n",ret);
		return ret;
	}

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
	if(spe_hook.is_enable && spe_hook.is_enable()){
		spe_hook.port_ioctl(g_usb_portno,spe_port_ioctl_set_mac,(int)dev->dev_addr);
	}
#endif
	return 0;
}

static void eth_get_drvinfo(struct net_device *net, struct ethtool_drvinfo *p)
{
    struct eth_dev *dev = netdev_priv(net);

    strlcpy(p->driver, "g_ether", sizeof(p->driver));
    strlcpy(p->version, UETH__VERSION, sizeof(p->version));
    strlcpy(p->fw_version, dev->gadget->name, sizeof(p->fw_version));
    strlcpy(p->bus_info, dev_name(&dev->gadget->dev), sizeof(p->bus_info));
}

/* REVISIT can also support:
 *   - WOL (by tracking suspends and issuing remote wakeup)
 *   - msglevel (implies updated messaging)
 *   - ... probably more ethtool ops
 */

static const struct ethtool_ops ops = {
    .get_drvinfo = eth_get_drvinfo,
    .get_link = ethtool_op_get_link,
};

static void defer_kevent(struct eth_dev *dev, int flag)
{
    if (test_and_set_bit(flag, &dev->todo))
        return;
    if (!schedule_delayed_work(&dev->work, 2))
    {
        dev->ctx.stats.rx_kevent_busy[flag]++;
    }else{
       dev->ctx.stats.rx_kevent_ok[flag]++;
    }
}

static void defer_rx(struct eth_dev *dev)
{
    if (!schedule_work(&dev->rx_work)) {
        dev->ctx.stats.rx_kevent_busy[WORK_RX_UPTASK]++;
    }
    else {
        dev->ctx.stats.rx_kevent_ok[WORK_RX_UPTASK]++;
    }
}

static void rx_ads_process(struct eth_dev *dev, gfp_t gfp_flags)
{
    struct sk_buff  *skb;
    int ret;

    while(NULL != (skb = skb_dequeue(&dev->rx_frames_to_ads)))
    {
        if(dev->handle_rx)
        {
            ret = dev->handle_rx(dev->port_usb,skb);
            if(ret)
            {
                dev->ctx.stats.rx_packets_fail++;
                dev->ctx.stats.rx_bytes_fail += skb->len;
                dev_kfree_skb_any(skb);
            }
        }
        else
        {
            dev_kfree_skb_any(skb);
        }
    }
}


static void eth_rx_work(struct work_struct *work)
{
    struct eth_dev  *dev = container_of(work, struct eth_dev, rx_work);
    rx_ads_process(dev, GFP_KERNEL);
}

static int
rx_submit(struct eth_dev *dev, struct usb_request *req, gfp_t gfp_flags)
{
    struct sk_buff  *skb;
    int     retval = -ENOMEM;
    size_t      size = 0;
    struct usb_ep   *out;
    unsigned long   flags;

    spin_lock_irqsave(&dev->lock, flags);
    if (dev->port_usb)
        out = dev->port_usb->out_ep;
    else
        out = NULL;
    spin_unlock_irqrestore(&dev->lock, flags);

    if (!out)
        return -ENOTCONN;


    /* Padding up to RX_EXTRA handles minor disagreements with host.
     * Normally we use the USB "terminate on short read" convention;
     * so allow up to (N*maxpacket), since that memory is normally
     * already allocated.  Some hardware doesn't deal well with short
     * reads (e.g. DMA must be N*maxpacket), so for now don't trim a
     * byte off the end (to force hardware errors on overflow).
     *
     * RNDIS uses internal framing, and explicitly allows senders to
     * pad to end-of-packet.  That's potentially nice for speed, but
     * means receivers can't recover lost synch on their own (because
     * new packets don't only start after a short RX).
     */
    size += sizeof(struct ethhdr) + dev->net->mtu + RX_EXTRA;
    size += dev->port_usb->header_len;
    size += out->maxpacket - 1;
    size -= size % out->maxpacket;

    if (dev->port_usb->is_fixed)
        size = max_t(size_t, size, dev->port_usb->fixed_out_len);

    skb = gnet_get_rx_skb(dev, size, gfp_flags);
    if (skb == NULL) {
        dev->ctx.stats.rx_no_skb++;
        goto enomem;
    }

    /* Some platforms perform better when IP packets are aligned,
     * but on at least one, checksumming fails otherwise.  Note:
     * RNDIS headers involve variable numbers of LE32 values.
     */
#if !defined(CONFIG_GNET_PREALLOC_RX_MEM)
    skb_reserve(skb, USB_NET_IP_ALIGN);
#endif
    req->buf = skb->data;
#if (defined(CONFIG_BALONG_RNDIS) || defined(CONFIG_BALONG_ECM)) && defined(CONFIG_GNET_PREALLOC_RX_MEM)
    req->length = dev->ctx.rx_skb_size;
#else
    req->length = size;
#endif

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    if(spe_hook.is_enable && spe_hook.is_enable()){
        req->complete = rx_spe_complete;
        
#ifdef CONFIG_USB_DWC3_BALONG
        req->skipcache = 1;
        /*The following line is for skipcache, disable it if skipcache=0.*/
        /*depend on spe init*/
        req->dma = spe_hook.get_skb_dma(skb); 
#endif

    }else
#endif
    {
        req->complete = rx_complete;
#ifdef CONFIG_USB_DWC3_BALONG
        req->skipcache = 0;
#endif
    }

    req->context = skb;
    retval = usb_ep_queue(out, req, gfp_flags);
    if (retval == -ENOMEM)
enomem:
        defer_kevent(dev, WORK_RX_MEMORY);
    if (retval) {
        if (skb){
            gnet_recycle_rx_skb(dev, skb);
        }
        dev->ctx.stats.rx_req_submitfail++;
        spin_lock_irqsave(&dev->req_lock, flags);
        list_add(&req->list, &dev->rx_reqs);
        spin_unlock_irqrestore(&dev->req_lock, flags);
    }else{
        atomic_inc(&dev->rx_qlen);
    }

    return retval;
}

void gnet_set_rx_protect_num(unsigned int num, int idx)
{
    struct eth_dev *dev;

    if (idx < 0 || idx >= GNET_USED_NUM) {
        return;
    }

    dev = the_dev[idx];
    dev->ctx.protect_num = num;
}

void gnet_rx_protect_init(struct eth_dev *dev)
{
    dev->ctx.protect_jiff = jiffies;
    dev->ctx.protect_num = GNET_RX_PROTECT_PKT;
}

int gnet_rx_need_protect(struct eth_dev *dev, unsigned char* data)
{
    static unsigned int s_rx_pkt = 0;

    s_rx_pkt++;

    /* calc time is over */
    if (time_after_eq(jiffies, dev->ctx.protect_jiff + GNET_RX_PROTECT_TIME)) {
        dev->ctx.protect_jiff = jiffies;
        s_rx_pkt = 0;
    }
    /* if the rx pkt reach the max pos, start protect */
    else if (s_rx_pkt >= dev->ctx.protect_num) {
        struct iphdr *ip = (struct iphdr *)(data + ETH_HLEN);
        struct ethhdr *ethhdr = (struct ethhdr *)data;

        /* we just protect for udp prot */
        if (htons(ETH_P_IP) == ethhdr->h_proto 
            &&  ip->protocol == IPPROTO_UDP) {
            dev->ctx.stats.rx_skb_protect_drop++;
            return 1;
        }
    }

    return 0;
}

static inline int eth_is_not_running(struct eth_dev *dev)
{
    if(bsp_usb_is_ncm_bypass_mode())
    {
        if (!dev->handle_rx)
            return 1;
    }
    else
    {
        if (!netif_running(dev->net)) 
            return 1;
    }
    return 0;
}

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
static void rx_spe_complete(struct usb_ep *ep, struct usb_request *req)
{
    struct sk_buff  *skb = req->context;
    struct eth_dev  *dev = ep->driver_data;
    struct sk_buff  *skb2;
    int status = req->status;
    int ret = 0;
    dma_addr_t dma;

    dev->ctx.stats.rx_completed++;

    switch (status) {
    /* normal completion */
    case 0:
        if(gnet_dbg_level & GNET_DBG_DROPREQ){
            dev->ctx.stats.rx_req_manudrop++;
            break;
        }
        if(!spe_hook.port_is_enable(g_usb_portno)){
            dev->ctx.stats.rx_spe_port_disable++;
            break;
        }
            
        dev->ctx.stats.rx_complete_ok++;
        if(!req->actual){
            dev->ctx.stats.rx_zlp++;
            GNET_DBG(("rx received zlp\n"));
            break;
        }
        print_bytes(dev, req->buf, 64);
        skb_put(skb, req->actual);
        skb_queue_tail(&dev->rx_frames, skb);
        skb = NULL;
        skb2 = skb_dequeue(&dev->rx_frames);
        
        /*diff host and spe skb*/
        while (skb2) {      
            print_skb(dev, skb2);

            /* check if user-defined drop */
            if(unlikely(gnet_rx_isdrop(skb2))){
                gnet_recycle_rx_skb(dev, skb2);
                status = 0;
                skb2 = skb_dequeue(&dev->rx_frames);
                continue;
            }

            if(!bsp_usb_is_ncm_bypass_mode()){
                dev->net->stats.rx_packets++;
                dev->net->stats.rx_bytes += skb2->len;
                dev->ctx.stats.rx_bytes += skb2->len;
                dev->ctx.stats.rx_packets++;
            }

            dma = spe_hook.get_skb_dma(skb2);
            ret = spe_hook.td_config(g_usb_portno, skb2, dma, spe_l3_bottom, 0);
            if (ret) {
                if(SPE_ERR_PORT_DISABLED == ret){
                    dev->ctx.stats.rx_spe_port_dis++;
                }
                dev->ctx.stats.rx_spe_drop++;
                gnet_recycle_rx_skb(dev, skb2);
            }
            skb2 = skb_dequeue(&dev->rx_frames);
        }

        break;

    /* software-driven interface shutdown */
    case -ECONNRESET:       /* unlink */
    case -ESHUTDOWN:        /* disconnect etc */
        VDBG(dev, "rx shutdown, code %d\n", status);
        goto quiesce;

    /* for hardware automagic (such as pxa) */
    case -ECONNABORTED:     /* endpoint reset */
        DBG(dev, "rx %s reset\n", ep->name);
        defer_kevent(dev, WORK_RX_MEMORY);
quiesce:
        gnet_recycle_rx_skb(dev, skb);
            goto clean;

    /* data overrun */
    case -EOVERFLOW:
        dev->net->stats.rx_over_errors++;
        /* FALLTHROUGH */

    default:
        dev->net->stats.rx_errors++;
        DBG(dev, "rx status %d\n", status);
        break;
    }

    if (skb){
        gnet_recycle_rx_skb(dev, skb);
    }

    if(eth_is_not_running(dev)){
clean:
        spin_lock(&dev->req_lock);
        list_add(&req->list, &dev->rx_reqs);
        
#ifdef CONFIG_USB_DWC3_BALONG
        req->skipcache = 0;
#endif
        spin_unlock(&dev->req_lock);
        req = NULL;
    }

    atomic_dec(&dev->rx_qlen);

    if (req)
        rx_submit(dev, req, GFP_ATOMIC);
}
#endif

static void rx_complete(struct usb_ep *ep, struct usb_request *req)
{
    struct sk_buff  *skb = req->context, *skb2;
    struct eth_dev  *dev = ep->driver_data;
    int     status = req->status;

    dev->ctx.stats.rx_completed++;

    switch (status) {
    /* normal completion */
    case 0:
        if(gnet_dbg_level & GNET_DBG_DROPREQ){
            dev->ctx.stats.rx_req_manudrop++;
            break;
        }
        dev->ctx.stats.rx_complete_ok++;
        if(!req->actual){
            dev->ctx.stats.rx_zlp++;
            GNET_DBG(("rx received zlp\n"));
            break;
        }
        print_bytes(dev, req->buf, 64);
        skb_put(skb, req->actual);

        if (dev->unwrap) {
            unsigned long   flags;

            spin_lock_irqsave(&dev->lock, flags);
            if (dev->port_usb) {
                status = dev->unwrap(dev->port_usb, skb, &dev->rx_frames);
            } else {
                dev_kfree_skb_any(skb);
                status = -ENOTCONN;
            }
            spin_unlock_irqrestore(&dev->lock, flags);
            if(status){
                dev->ctx.stats.rx_unwrap_fail++;
            }
        } else {
            skb_queue_tail(&dev->rx_frames, skb);
        }

        skb = NULL;
        skb2 = skb_dequeue(&dev->rx_frames);
        /*diff host and spe skb*/
        while (skb2) {
            if (status < 0 || ETH_HLEN > skb2->len
                    || skb2->len > VLAN_ETH_FRAME_LEN) {
                dev->net->stats.rx_errors++;
                dev->net->stats.rx_length_errors++;
                dev->ctx.stats.rx_packets_err++;
                DBG(dev, "rx length %d\n", skb2->len);
                dev_kfree_skb_any(skb2);
                skb2 = skb_dequeue(&dev->rx_frames);
                continue;
            }

            print_skb(dev, skb2);

            /* check if user-defined drop */
            if(unlikely(gnet_rx_isdrop(skb2))){
                dev_kfree_skb_any(skb2);
                status = 0;
                skb2 = skb_dequeue(&dev->rx_frames);
                continue;
            }

            if(bsp_usb_is_ncm_bypass_mode()){
#if (defined(CONFIG_BALONG_NET_DEV_RX_DFS) && defined(CONFIG_CPU_FREQ))
                /* dfs control for stick bypass mode */
                netif_rx_dfs_control();
#endif
                if (dev->handle_rx){
                    dev->ctx.stats.rx_bytes += skb2->len;
                    dev->ctx.stats.rx_packets++;
                    if(!(gnet_dbg_level & GNET_DBG_RX2ADS)){   /* don't call cb directly */
                        if(skb_queue_len(&dev->rx_frames_to_ads) <= dev->rx_frames_waterline){
                            skb_queue_tail(&dev->rx_frames_to_ads, skb2);
                        }else{
                            dev->ctx.stats.rx_packets_overwline++;
                            dev_kfree_skb_any(skb2);
                        }
                        status = 0;
                    }else{
                        status = dev->handle_rx(dev->port_usb, skb2);
                        if(status){
                            dev->ctx.stats.rx_packets_fail++;
                            dev->ctx.stats.rx_bytes_fail += skb2->len;
                            dev_kfree_skb_any(skb2);
                            status = 0;
                        }
                    }
                }else{
                    GNET_ERR(("dev->handle_rx not registered!\n"));
                    dev_kfree_skb_any(skb2);
                    status = 0;
                }
            }
            else{
                skb2->protocol = eth_type_trans(skb2, dev->net);
                dev->net->stats.rx_packets++;
                dev->net->stats.rx_bytes += skb2->len;
                dev->ctx.stats.rx_bytes += skb2->len;
                dev->ctx.stats.rx_packets++;

                /* no buffer copies needed, unless hardware can't
                 * use skb buffers.*/
                status = netif_rx(skb2);
            }
            skb2 = skb_dequeue(&dev->rx_frames);
        }

        if(bsp_usb_is_ncm_bypass_mode()){
            if(!(gnet_dbg_level & GNET_DBG_RX2ADS)){
                defer_rx(dev);
            }
        }
        break;

    /* software-driven interface shutdown */
    case -ECONNRESET:       /* unlink */
    case -ESHUTDOWN:        /* disconnect etc */
        VDBG(dev, "rx shutdown, code %d\n", status);
        goto quiesce;

    /* for hardware automagic (such as pxa) */
    case -ECONNABORTED:     /* endpoint reset */
        DBG(dev, "rx %s reset\n", ep->name);
        defer_kevent(dev, WORK_RX_MEMORY);
quiesce:
        gnet_recycle_rx_skb(dev, skb);
            goto clean;
    /* data overrun */
    case -EOVERFLOW:
        dev->net->stats.rx_over_errors++;
        /* FALLTHROUGH */

    default:
        dev->net->stats.rx_errors++;
        DBG(dev, "rx status %d\n", status);
        break;
    }

    if (skb){
        gnet_recycle_rx_skb(dev, skb);
    }

    if(eth_is_not_running(dev))
    {
clean:
        spin_lock(&dev->req_lock);
        list_add(&req->list, &dev->rx_reqs);
        
#ifdef CONFIG_USB_DWC3_BALONG
        req->skipcache = 0;
#endif
        spin_unlock(&dev->req_lock);
        req = NULL;
    }

    atomic_dec(&dev->rx_qlen);

    if (req)
        rx_submit(dev, req, GFP_ATOMIC);
}

static int prealloc(struct list_head *list, struct usb_ep *ep, unsigned n)
{
    unsigned        i;
    struct usb_request  *req;

    if (!n)
        return -ENOMEM;
    /* queue/recycle up to N requests */
    i = n;
    list_for_each_entry(req, list, list) {

        if (i-- == 0)
            goto extra;
    }
    while (i--) {
        req = usb_ep_alloc_request(ep, GFP_ATOMIC);
        if (!req){
            return list_empty(list) ? -ENOMEM : 0;
        }
        list_add(&req->list, list);
    }
    return 0;

extra:
    /* free extras */
    for (;;) {
        struct list_head    *next;

        next = req->list.next;
        list_del(&req->list);
        usb_ep_free_request(ep, req);

        if (next == list)
            break;

        req = container_of(next, struct usb_request, list);
    }
    return 0;
}

static int alloc_requests(struct eth_dev *dev, struct gether *link, unsigned int n_rx, unsigned int n_tx)
{
    int status;

    GNET_TRACE(("%s line %d:alloc %d rx request, %d tx request\n", __FUNCTION__, __LINE__, n_rx, n_tx));

    /* alloc tx requests */
    spin_lock(&dev->tx_req_lock);
    status = prealloc(&dev->tx_reqs, link->in_ep, n_tx);
    if (status < 0)
        goto fail_tx;
    
    status = alloc_tx_ctx(dev, n_tx);
    if (status < 0)
        goto fail_tx;
    spin_unlock(&dev->tx_req_lock);
    dev->tx_req_num = n_tx;

    /* alloc rx requests */
    spin_lock(&dev->req_lock);
    status = prealloc(&dev->rx_reqs, link->out_ep, n_rx);
    if (status < 0)
        goto fail_rx;
    spin_unlock(&dev->req_lock);
    dev->rx_req_num = n_rx;

    dev->rx_req_waterline = n_rx/RX_REQ_WATERLINE_DIV;
#ifdef CONFIG_GNET_PREALLOC_RX_MEM
    dev->ctx.rx_skb_waterline = min_t(size_t,dev->ctx.rx_skb_waterline,dev->rx_req_waterline);
#endif

    return status;
fail_tx:
    DBG(dev, "can't alloc tx requests\n");
    spin_unlock(&dev->tx_req_lock);
    return status;
fail_rx:
    DBG(dev, "can't alloc rx requests\n");
    spin_unlock(&dev->req_lock);
    return status;
}

static void rx_fill(struct eth_dev *dev, gfp_t gfp_flags)
{
    struct usb_request  *req;
    unsigned long       flags;

    /* fill unused rxq slots with some skb */
    spin_lock_irqsave(&dev->req_lock, flags);
    while (!list_empty(&dev->rx_reqs)) {
        if(atomic_read(&dev->rx_qlen) >= dev->rx_req_waterline){
            /*If all request have been send to usb dev, 
            there will be to many start work queue.*/
            break;
        }
        req = container_of(dev->rx_reqs.next,
                struct usb_request, list);
        list_del_init(&req->list);
        spin_unlock_irqrestore(&dev->req_lock, flags);

        if (rx_submit(dev, req, gfp_flags) < 0) {
            defer_kevent(dev, WORK_RX_MEMORY);
            return;
        }

        spin_lock_irqsave(&dev->req_lock, flags);
    }
    spin_unlock_irqrestore(&dev->req_lock, flags);
}

static void eth_work(struct work_struct *data)
{
    struct eth_dev  *dev = container_of(data, struct eth_dev, work.work);
    if (test_and_clear_bit(WORK_RX_MEMORY, &dev->todo)){
        if(!bsp_usb_is_ncm_bypass_mode()){
            if (netif_running(dev->net)){
                rx_fill(dev, GFP_KERNEL);
            }
        }
        else{
            rx_fill(dev, GFP_KERNEL);
        }
    }

    if (dev->todo){
        GNET_DBG(("work done, flags = 0x%lx\n", dev->todo));
    }
}

static void tx_complete(struct usb_ep *ep, struct usb_request *req)
{
    tx_ctx_t *ctx = (tx_ctx_t *)req->context;
    struct eth_dev  *dev = ep->driver_data;
    ulong flags;
    struct net_device *net = NULL;
    
    if(!bsp_usb_is_ncm_bypass_mode()){
        net = dev->net;
    }

    switch (req->status) {
    case 0:
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
        if (ctx->spe_skb) {
            dev->ctx.stats.tx_spe_complete_ok++;
        }
#endif
        dev->ctx.stats.tx_complete_ok++;
        break;

    default:
        VDBG(dev, "tx err %d\n", req->status);

        /* FALLTHROUGH */
    case -ECONNRESET:       /* unlink */
    case -ESHUTDOWN:        /* disconnect etc */
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
        if (ctx->spe_skb) {
            dev->ctx.stats.tx_spe_complete_fail++;
        }
#endif
        dev->ctx.stats.tx_complete_fail++;
        break;
    }

    gnet_recycle_ctx_info(ctx,  (0 == req->status));  /* memory recycle */

    /* request enqueue */
    spin_lock_irqsave(&dev->tx_req_lock, flags);
    list_add_tail(&req->list, &dev->tx_reqs);
#ifdef CONFIG_USB_DWC3_BALONG
    req->skipcache = 0;
#endif
    spin_unlock_irqrestore(&dev->tx_req_lock, flags);

    atomic_dec(&dev->tx_qlen);
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    if (ctx->spe_skb && ctx->spe_skb->spe_own) {
        return;
    }
#endif

    if(!bsp_usb_is_ncm_bypass_mode()){
        if (netif_carrier_ok(net) && netif_queue_stopped(net))
        {
            dev->ctx.stats.tx_wake_queue++;
            netif_wake_queue(net);
        }
    }
}

static inline int is_promisc(u16 cdc_filter)
{
    return cdc_filter & USB_CDC_PACKET_TYPE_PROMISCUOUS;
}

#if defined(CONFIG_BALONG_ECM) || defined(CONFIG_BALONG_RNDIS)
tx_ctx_t *skb_to_sgs(struct gether *port, tx_ctx_t *ctx)
{
#define TRB_CACHE_QUIRK_UPDATE_BYTES(trbs,bytes,maxpacket)\
    do\
    {\
        if((bytes) >= (maxpacket))\
        {\
            (bytes) &= ((maxpacket)-1);\
            (trbs) = (bytes)?1:0;\
        }\
    }while(0)

    struct eth_dev *dev = port->ioport;
    struct sk_buff  *skb_loop;
    struct sk_buff  *skb_next;
    struct sk_buff  *skb_last = NULL;
    struct scatterlist *sgl;

    u32 cnt = 0;
    u32 chain_node_cnt = 0;
    u32 maxpacket = port->in_ep->maxpacket;
#if defined(CONFIG_BALONG_RNDIS)
    u32 trbs_for_quirk = 0;
    u32 bytes_for_quirk = 0;
    u32 pkt_cnt_per_transfer = 0;
    u32 pkt_len_per_transfer = 0;
#endif

    GNET_TRACE(("%s: enter\n", __FUNCTION__));

    /* alloc scatter-gather list */
    if(gnet_get_sgnode(ctx, ctx->pkt_cnt + dev->ctx.sg_node_reserve, GFP_ATOMIC))
    {
        dev->ctx.stats.tx_no_sg++;
        GNET_ERR(("%s: fail to alloc sgnode\n", __FUNCTION__));
        return NULL;
    }

    /* assemble skb list */
    cnt = 0;
    ctx->sg_node->actual = 0;

    /* always use the sg node from the beginning */
    sgl = &ctx->sg_node->sg_tbl.sgl[0];

#if defined(CONFIG_BALONG_RNDIS)
    bytes_for_quirk = 0;
    trbs_for_quirk = 0;
    pkt_cnt_per_transfer = 0;
    pkt_len_per_transfer = 0;
#endif

    skb_queue_walk_safe(&ctx->pkt_list,skb_loop,skb_next)
    {
#if defined(CONFIG_BALONG_RNDIS)
        if(dev->ctx.quirks & QUIRK_DWC_TRB_CACHE)
        {
            bytes_for_quirk += skb_loop->len;
            if(bytes_for_quirk >= maxpacket)
            {
                TRB_CACHE_QUIRK_UPDATE_BYTES(trbs_for_quirk,bytes_for_quirk,maxpacket);
            }
            else
            {
                trbs_for_quirk++;    /* try to add current trb */
                if((USB_CDC_NCM_DWC_CACHE_TRB_NUM - 1) == trbs_for_quirk)
                {
                    struct rndis_packet_msg_type *header =
                        (struct rndis_packet_msg_type *)skb_last->data;
                    unsigned int msg_len = le32_to_cpu(header->MessageLength);
                    bytes_for_quirk -= skb_loop->len;   /* give up current skb */
                    /* modify last skb */
                    msg_len += maxpacket - bytes_for_quirk;
                    header->MessageLength = cpu_to_le32(msg_len);

                    ctx->sg_node->actual += maxpacket - bytes_for_quirk;
                    pkt_len_per_transfer += maxpacket - bytes_for_quirk;

                    /* ugly to clean the page_link of sg here */
                    sgl->page_link = 0;

                    sg_set_buf(sgl, (void *)dev->ctx.dwc_trb_cache_buf, maxpacket - bytes_for_quirk);
                    sgl++;
                    bytes_for_quirk += maxpacket - bytes_for_quirk;  /* fill dummy skb */
                    TRB_CACHE_QUIRK_UPDATE_BYTES(trbs_for_quirk,bytes_for_quirk,maxpacket);

                    trbs_for_quirk++;
                    bytes_for_quirk += skb_loop->len;
                    TRB_CACHE_QUIRK_UPDATE_BYTES(trbs_for_quirk,bytes_for_quirk,maxpacket);

                    dev->ctx.stats.tx_quirk++;
                }
            }
        }
#endif
        ctx->sg_node->actual += skb_loop->len;
#if defined(CONFIG_BALONG_RNDIS)
        pkt_len_per_transfer += skb_loop->len;
#endif

        skb_last = skb_loop;

        /* ugly to clean the page_link of sg here */
        sgl->page_link = 0;

        sg_set_buf(sgl, (const void *)skb_loop->data, skb_loop->len); /* let usb core flush cache */

#if defined(CONFIG_BALONG_RNDIS)
        pkt_cnt_per_transfer++;

        /* For RNDIS, we should mark the boundary of one transfer */
        if ((pkt_cnt_per_transfer > (GNET_MAX_PKT_PER_TRANSFER - 1))
            || (pkt_len_per_transfer > (GNET_IN_MAXSIZE_PER_TRANSFER - 1600)))
        {
            if(!(pkt_len_per_transfer % maxpacket))
            {
                dev->ctx.stats.tx_expand_zlp++;
                sg_set_buf(sgl, (const void *)skb_loop->data, skb_loop->len+1); /* add 1 byte for zero packet */
                ctx->sg_node->actual += 1;
            }
            sgl++;

            sgl->page_link = ((unsigned long)(sgl + 1) | 0x01) & ~0x02; /* mark for next transfer */
            sgl++;
            chain_node_cnt++;

            bytes_for_quirk = 0;
            trbs_for_quirk = 0;
            pkt_cnt_per_transfer = 0;
            pkt_len_per_transfer = 0;
        }
        else
        {
            if(!(pkt_len_per_transfer % maxpacket)
                && skb_queue_is_last(&ctx->pkt_list,skb_loop))
            {
                    dev->ctx.stats.tx_expand_zlp1++;
                    sg_set_buf(sgl, (const void *)skb_loop->data, skb_loop->len+1); /* add 1 byte for zero packet */
                    ctx->sg_node->actual += 1;
            }

            sgl++;
        }
                
        /* count the gathered packets statistics */             
        dev->ctx.stats.tx_gathered_bytes += skb_loop->len - RNDIS_PACKET_HEADER_LEN;

#else
        if(!(skb_loop->len % maxpacket))
        {
            dev->ctx.stats.tx_expand_zlp++;
            sg_set_buf(sgl, (const void *)skb_loop->data, skb_loop->len+1); /* add 1 byte for zero packet */
            ctx->sg_node->actual += 1;
        }
        sgl++;
                
        /* count the gathered packets statistics */
        dev->ctx.stats.tx_gathered_bytes += skb_loop->len;

#endif

    }

    cnt = ((unsigned long)sgl - (unsigned long)(&ctx->sg_node->sg_tbl.sgl[0])) / (sizeof(struct scatterlist));

    ctx->sg_node->sg_tbl.nents = cnt - chain_node_cnt;

    /* set the last flag */
    sg_mark_end((sgl-1));

    return ctx;
}
/**
 * eth_gather_pkt - gather skb to exploit the usb bus efficiency
 * @port: associated function link
 * @skb: sk_buff to be transmitted by usb.
 * Context: soft irq
 *
 * This is the dev->wrap function routine for RNDIS & ECM, which does NOT just "wrap" the packet.
 * The main purpose of this routine is to gather the skbs from TCP/IP protocol suite,
 * and deliver to the usb core in batch. While gathering packets, we call the link->wrap
 * to "wrap" every packet.
 *
 * Returns:
 *  NULL                :failed, the input skb is released.
 *  GNET_SKB_MAGIC      :pkts gathering done, to be enqueued to the usb core
 *  Other valid value   :pkts gathering.
 */
struct sk_buff *eth_gather_pkt(struct gether *port, struct sk_buff *skb)
{
    struct eth_dev *dev = port->ioport;
    struct usb_request *req;
    tx_ctx_t *ctx;
    ulong flags;

    GNET_TRACE(("%s: enter\n", __FUNCTION__));

    if(skb) /* caller is xmit */
    {
        /* wrap the packet according to the corresponding function wrap mechanism, e.g rndis */
        if (port->wrap)
            skb = port->wrap(port, skb);

        if (unlikely(!skb))
        {
            //skb already freed
            GNET_TRACE(("%s: function driver wrap fail\n", __FUNCTION__));
            return NULL;
        }

        spin_lock_irqsave(&dev->tx_req_lock, flags);

        /* get req tx_ctx */
        req = list_first_entry(&dev->tx_reqs, struct usb_request, list);
        ctx = (tx_ctx_t *)req->context;

        if(skb_queue_empty(&ctx->pkt_list))
        {
            /* start timer */
            GNET_TRACE(("%s: 1st skb, start timer\n", __FUNCTION__));
            dev->ctx.timer.expires = jiffies + dev->ctx.timeout;  /* jiffies unit */
            add_timer(&dev->ctx.timer);
            dev->ctx.stats.tx_timer_activated++;
        }

        /* pkt enqueue */
#ifdef CONFIG_BALONG_SKB_MEMBER
        skb->used_check = GNET_MAGIC_SKB_USED;
#endif
        __skb_queue_tail(&ctx->pkt_list, skb);

        /* update packets info */
        ctx->pkt_cnt++;
        ctx->pkt_len += skb->len;

        if((!skb->psh) && (ctx->pkt_cnt < dev->ctx.pkt_cnt_waterline)
            && (ctx->pkt_len < dev->ctx.pkt_len_waterline))
        {
            spin_unlock_irqrestore(&dev->tx_req_lock, flags);
            GNET_TRACE(("%s: put skb into list only\n", __FUNCTION__));
            /* this is a bad trick, as skb may be realloced. We should NOT assume the
                skb is the same as the input parameter skb
            */
            return (struct sk_buff *)GNET_MAGIC_WRAPING;
        }
        else
        {
            /*wrap aggregation in case of waterline reach*/
            del_timer(&dev->ctx.timer); /* delete timer */
            dev->ctx.stats.tx_psh += skb->psh;
            list_del(&req->list);
            spin_unlock_irqrestore(&dev->tx_req_lock, flags);
            dev->ctx.stats.tx_timer_activated--;
            GNET_TRACE(("%s: waterline reach,ctx=%p\n",__FUNCTION__,ctx));
        }
    }
    else    /* caller is timeout */
    {
        GNET_TRACE(("%s:caller is timeout\n",__FUNCTION__));

        /* check the done queue, it is possible that the normal wrap waterline and
            timer to trigger the packet send process by collision
        */
        spin_lock_irqsave(&dev->tx_req_lock, flags);
        if(!list_empty_careful(&dev->tx_done_reqs))    /* ready to send */
        {
            spin_unlock_irqrestore(&dev->tx_req_lock, flags);
            /* This should NOT happen, as timeout routine shouldn't preempt over the
             * tx xmit routine. So if happened, just to skip this xmit, as normal xmit
             * will dispose the done ctx.
             */
            return (struct sk_buff *)GNET_MAGIC_WRAPED;
        }

        if(list_empty_careful(&dev->tx_reqs))  /* no tx ctx */
        {
            spin_unlock_irqrestore(&dev->tx_req_lock, flags);
            dev->ctx.stats.tx_bug_noreq++;
            GNET_ERR(("%s: 700: no tx req found\n", __FUNCTION__));
            return (struct sk_buff *)GNET_MAGIC_WRAPING;
        }

        /* get req & tx_ctx */
        req = list_first_entry(&dev->tx_reqs, struct usb_request, list);
        ctx = (tx_ctx_t *)req->context;
        if(skb_queue_empty(&ctx->pkt_list))
        {
            spin_unlock_irqrestore(&dev->tx_req_lock, flags);
            GNET_ERR(("%s:800:timer triggered but not skb in tx_ctx_free_q, error\n", __FUNCTION__));
            return (struct sk_buff *)GNET_MAGIC_WRAPING;
        }

        list_del(&req->list);
        spin_unlock_irqrestore(&dev->tx_req_lock, flags);
    }

    if(skb_to_sgs(port, ctx))  /* wrap skbs according to rndis protocol */
    {
        /* push ctx to done queue */
        spin_lock_irqsave(&dev->tx_req_lock, flags);
        list_add_tail(&req->list, &dev->tx_done_reqs);
        spin_unlock_irqrestore(&dev->tx_req_lock, flags);
        GNET_TRACE(("%s:skbs to sg ok.\n",__FUNCTION__));
        return (struct sk_buff *)GNET_MAGIC_WRAPED;
    }
    else
    {
        spin_lock_irqsave(&dev->tx_req_lock, flags);
        gnet_recycle_ctx_info(ctx, 0);  /* memory recycle */
        list_add_tail(&req->list, &dev->tx_reqs);;  /* req enqueue */
        spin_unlock_irqrestore(&dev->tx_req_lock, flags);
        GNET_ERR(("%s:skbs to sg fail.\n",__FUNCTION__));
        return NULL;
    }
}
#endif

static netdev_tx_t eth_send_pkt(struct eth_dev *dev)
{
    struct gether *port = NULL;
    struct usb_ep   *in;
    struct net_device *net = dev->net;
    struct usb_request  *req = NULL;
    tx_ctx_t *ctx;
    ulong   flags = 0;
    int32_t   retval = 0;
    u32   length;

    /* get request */
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
again:
#endif
    spin_lock_irqsave(&dev->tx_req_lock, flags);

    if (list_empty_careful(&dev->tx_done_reqs)) {
        spin_unlock_irqrestore(&dev->tx_req_lock, flags);
        dev->ctx.stats.tx_no_donereq++;
        return NETDEV_TX_OK;
    }

    req = list_first_entry(&dev->tx_done_reqs, struct usb_request, list);
    list_del(&req->list);

    if (list_empty_careful(&dev->tx_reqs)) {
        if(!bsp_usb_is_ncm_bypass_mode()){
            netif_stop_queue(net);
        }
        dev->ctx.stats.tx_prestop_queue++;
    }

    ctx = (tx_ctx_t *)req->context;

    spin_unlock_irqrestore(&dev->tx_req_lock, flags);

    length = ctx->sg_node->actual;
#if defined(CONFIG_BALONG_NCM)
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    if (ctx->spe_skb && ctx->spe_skb->spe_own) {
        dev->ctx.stats.tx_spe_send++;
    } else
#endif
    {
        length += ctx->ntb->actual;
    }
#endif

    /* fill req */
    req->num_sgs = ctx->sg_node->sg_tbl.nents;

    GNET_TRACE(("%s:gathered sg num =%d in req\n",__FUNCTION__,req->num_sgs));

    spin_lock_irqsave(&dev->lock, flags);
    port = dev->port_usb;
    if(!port || !(in = port->in_ep)){
        spin_unlock_irqrestore(&dev->lock, flags);
        dev->ctx.stats.dev_no_port++;
        GNET_ERR(("%s: !port || !(in = port->in_ep\n", __FUNCTION__));
        goto req_enqueue;
    }

    /* NCM requires no zlp if transfer is dwNtbInMaxSize */
    if (dev->port_usb->is_fixed && length == dev->port_usb->fixed_in_len &&
        (length % in->maxpacket) == 0){
        req->zero = 0;
    }
    else if(dev->port_usb->is_fixed && length < dev->port_usb->fixed_in_len &&
        (length % in->maxpacket) == 0){
        req->zero = 1;
        dev->ctx.stats.tx_zlp++;
    }else{
        req->zero = 0;
    }
    
    /* throttle high/super speed IRQ rate back slightly */
    if (gadget_is_dualspeed(dev->gadget))
        req->no_interrupt = (dev->gadget->speed == USB_SPEED_HIGH ||
                     dev->gadget->speed == USB_SPEED_SUPER)
            ? ((atomic_read(&dev->tx_qlen) % qmult) != 0)
            : 0;
    retval = usb_ep_queue(in, req, GFP_ATOMIC);
    switch (retval) {
    case 0:
        net->trans_start = jiffies;
        atomic_inc(&dev->tx_qlen);
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
        dev->ctx.stats.tx_queue++;
#endif
        break;

    default:
        dev->ctx.stats.tx_req_dropped++;
        DBG(dev, "tx queue err %d\n", retval);
        break;
    }

    if (retval) {
        spin_unlock_irqrestore(&dev->lock, flags);
        goto req_enqueue;
    }

    spin_unlock_irqrestore(&dev->lock, flags);

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    if(spe_hook.is_enable && spe_hook.is_enable()){
        goto again;
    }
#endif

    return NETDEV_TX_OK;

req_enqueue:
    /* tx_reqs enqueue */
    spin_lock_irqsave(&dev->tx_req_lock, flags);
    gnet_recycle_ctx_info((tx_ctx_t *)req->context,0);
    if (list_empty(&dev->tx_reqs))
    {
        if(!bsp_usb_is_ncm_bypass_mode()){
            netif_start_queue(net);
            dev->ctx.stats.tx_wake_queue++;
        }
    }
    list_add(&req->list, &dev->tx_reqs);
    spin_unlock_irqrestore(&dev->tx_req_lock, flags);

    return NETDEV_TX_OK;
}

/* recycle the buffer resource when failed to resume the gadget*/
static void gnet_wakeup_fail_recycle(struct eth_dev *dev)
{
    struct usb_request  *req;
    ulong flags;

    spin_lock_irqsave(&dev->tx_req_lock, flags);
    req = list_first_entry(&dev->tx_done_reqs, struct usb_request, list);
    list_del(&req->list);

    gnet_recycle_ctx_info((tx_ctx_t *)req->context,0);

    list_add(&req->list, &dev->tx_reqs);
    spin_unlock_irqrestore(&dev->tx_req_lock, flags);
}

static netdev_tx_t eth_wrap_and_send(struct eth_dev *dev, struct sk_buff *skb)
{
    struct sk_buff *skb2;
    ulong flags;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    struct sk_buff *skb3 = NULL;
    struct usb_request *req = NULL;
    struct usb_request *next = NULL;
    tx_ctx_t    *ctx = NULL;
    tx_ctx_t    *next_ctx = NULL;
    int hostpkt_exist = 0;

    /*if pkt from spe*/
    if (skb && skb->spe_own) {
        hostpkt_exist = 2;
        spin_lock_irqsave(&dev->tx_req_lock, flags);

        /*get tx request from tx request list*/
        req = list_first_entry(&dev->tx_reqs, struct usb_request, list);
        ctx = (tx_ctx_t*)req->context;
        
        /*check have packet from host*/
        dev->ctx.stats.tx_packet_from_host = 0;
        if (!skb_queue_empty(&ctx->pkt_list)) {
            hostpkt_exist = 1;
            /*this branch means there is pkt from host, 
            when receive spe skb, send host skb at once*/
            list_del(&req->list);
            ctx->spe_skb = NULL;
            dev->ctx.stats.tx_packet_from_host = 1;
            
            if (list_empty(&dev->tx_reqs)) {
                /*Device have no requset to send?*/
                spin_unlock_irqrestore(&dev->tx_req_lock, flags);
                list_add(&req->list, &dev->tx_reqs);
                return NETDEV_TX_BUSY;
            } else {
                del_timer(&dev->ctx.timer);
                next = list_first_entry(&dev->tx_reqs, struct usb_request, list);
                next_ctx = (tx_ctx_t*)next->context;
            }
            list_add(&req->list, &dev->tx_reqs);/* timeout to send it */
        }
        if (next) {
            ctx = next_ctx;
            req = next;
        }
        list_del(&req->list);

        sg_set_buf(&ctx->sg_node->sg_tbl.sgl[0], (const void *)skb->data, skb->len);
#ifndef CONFIG_BALONG_SPE_SIM
        /* if the req skip cache, usb core don't set the dma address,
        * we must dma_address in function drvier
        */
        req->skipcache = 1;
        sg_dma_address(&ctx->sg_node->sg_tbl.sgl[0]) =
        (dma_addr_t)virt_to_phys((void*)skb->data);
#endif

        ctx->sg_node->sg_tbl.nents = 1;
        ctx->sg_node->actual = skb->len;

        ctx->spe_skb = skb;

        /*config sg list*/
        list_add_tail(&req->list, &dev->tx_done_reqs);
        spin_unlock_irqrestore(&dev->tx_req_lock, flags);
    }
#endif

    spin_lock_irqsave(&dev->lock, flags);

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    /*0-from host,1-from spe,*/
    if(0 == hostpkt_exist){
        skb3 = skb;
    } else if (1 == hostpkt_exist) {
        skb3 = NULL;
    } else {/*only a packet from spe */
        spin_unlock_irqrestore(&dev->lock, flags);
        return eth_send_pkt(dev);
    }
#endif

    if (dev->wrap && dev->port_usb) {
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
        skb2 = dev->wrap(dev->port_usb, skb3);
#else
        skb2 = dev->wrap(dev->port_usb, skb);
#endif
        if((struct sk_buff *)GNET_MAGIC_WRAPING == skb2)
        {
            spin_unlock_irqrestore(&dev->lock, flags);
            return NETDEV_TX_OK;
        }
        else if((struct sk_buff *)GNET_MAGIC_WRAPED == skb2)
        {/* wrapped OK */
            spin_unlock_irqrestore(&dev->lock, flags);
            /* This is the normal status.
            check whether the gadget is suspended.
            if so, try to wake up the host */
            if (unlikely(dev->is_suspend)) {
                if (gnet_wakeup_gadget(dev)) {
                    /* wakeup failed, recycle the buffer resource */
                    gnet_wakeup_fail_recycle(dev);
                    return NETDEV_TX_OK;
                }
            }
        }
        else
        {
            /* if failed, recycle mem in wrap api */
            spin_unlock_irqrestore(&dev->lock, flags);
            dev->ctx.stats.dev_wrap_fail++;
            GNET_ERR(("%s:wrap failed\n", __FUNCTION__));
            return NETDEV_TX_OK;
        }
    }
    else {
        spin_unlock_irqrestore(&dev->lock, flags);
        dev->ctx.stats.dev_no_wrap++;
        dev->net->stats.tx_dropped++;
        if(skb){
            dev_kfree_skb_any(skb);
        }
        GNET_ERR(("%s: dev->wrap or dev->port_usb null(%x,%x)\n",
                    __FUNCTION__,(unsigned int)dev->wrap,(unsigned int)dev->port_usb));
        return NETDEV_TX_OK;
    }

    return eth_send_pkt(dev);
}

void gnet_timeout_handle(unsigned long data)
{
    struct eth_dev  *dev = (struct eth_dev *)data;

    (void)eth_wrap_and_send(dev, NULL);

    /* count the tx triggered by timer */
    dev->ctx.stats.tx_by_timer++;
}

static netdev_tx_t eth_start_xmit(struct sk_buff *skb,
                    struct net_device *net)
{
    struct eth_dev  *dev = netdev_priv(net);
    struct usb_ep   *in;
    ulong   flags;
    u16 cdc_filter;

    print_skb(dev, skb);
    dev->ctx.stats.tx_total++;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    if (skb->spe_own) {
        dev->ctx.stats.tx_spe_total++;
    }
#endif

#ifdef CONFIG_BALONG_SKB_MEMBER
    if (GNET_MAGIC_SKB_USED == skb->used_check) {
        dev->ctx.stats.tx_skb_check_err++;
        return NETDEV_TX_OK;
    }
#endif

    spin_lock_irqsave(&dev->lock, flags);
    if (dev->port_usb) {
        in = dev->port_usb->in_ep;
        cdc_filter = dev->port_usb->cdc_filter;
    } else {
        in = NULL;
        cdc_filter = 0;
    }
    spin_unlock_irqrestore(&dev->lock, flags);

    if (!in) {
        dev->ctx.stats.tx_inep_null++;
        net->stats.tx_dropped++;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
        if (skb->spe_own) {
            dev->ctx.stats.tx_inep_null++;
            gnet_recycle_spe_rd(skb, dev);
            return NETDEV_TX_OK;
        } 
#endif
        dev_kfree_skb_any(skb);

        return NETDEV_TX_OK;
    }

    /* apply outgoing CDC or RNDIS filters */
    if (!is_promisc(cdc_filter)) {
        u8      *dest = skb->data;

        if (is_multicast_ether_addr(dest)) {
            u16 type;

            /* ignores USB_CDC_PACKET_TYPE_MULTICAST and host
             * SET_ETHERNET_MULTICAST_FILTERS requests
             */
            if (is_broadcast_ether_addr(dest))
                type = USB_CDC_PACKET_TYPE_BROADCAST;
            else
                type = USB_CDC_PACKET_TYPE_ALL_MULTICAST;
            if (!(cdc_filter & type)) {
                dev->ctx.stats.tx_filt_out++;
                net->stats.tx_dropped++;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
                if (skb->spe_own) {
                    gnet_recycle_spe_rd(skb, dev);
                    return NETDEV_TX_OK;
                }
#endif
                dev_kfree_skb_any(skb);
                return NETDEV_TX_OK;
            }
        }
        /* ignores USB_CDC_PACKET_TYPE_DIRECTED */
    }

    spin_lock_irqsave(&dev->tx_req_lock, flags);
    /*
     * this freelist can be empty if an interrupt triggered disconnect()
     * and reconfigured the gadget (shutting down this queue) after the
     * network stack decided to xmit but before we got the spinlock.
     */
    if (list_empty(&dev->tx_reqs)) {
        spin_unlock_irqrestore(&dev->tx_req_lock, flags);

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
        if (skb->spe_own) {
            gnet_recycle_spe_rd(skb, dev);
            dev->ctx.stats.tx_spe_no_req++;
            return NETDEV_TX_BUSY;
        }
#endif
        dev_kfree_skb_any(skb);

        if(!bsp_usb_is_ncm_bypass_mode()){
            netif_stop_queue(net);
            dev->ctx.stats.tx_stop_queue++;
        }
        
        dev->ctx.stats.tx_no_req++;

        return NETDEV_TX_OK;
    }
    spin_unlock_irqrestore(&dev->tx_req_lock, flags);

    return eth_wrap_and_send(dev, skb);
}

netdev_tx_t eth_vendor_start_xmit(struct sk_buff *skb,
                    struct net_device *net)
{
    struct eth_dev *dev = netdev_priv(net);

    if(unlikely(dev->is_suspend && !(dev->gadget->rwakeup))) {
        dev_kfree_skb_any(skb);
        dev->ctx.stats.tx_drop_suspend++;
        return NETDEV_TX_OK;
    }

    return eth_start_xmit(skb, net);
}

/*-------------------------------------------------------------------------*/

static void eth_start(struct eth_dev *dev, gfp_t gfp_flags)
{
    USB_DBG_GADGET(&dev->gadget->dev, "%s\n", __func__);

    dev->ctx.stats.dev_start++;

    /* fill the rx queue */
    rx_fill(dev, gfp_flags);

    /* and open the tx floodgates */
    atomic_set(&dev->tx_qlen, 0);

    if(!bsp_usb_is_ncm_bypass_mode()){
        netif_wake_queue(dev->net);
        dev->ctx.stats.tx_wake_queue++;
    }
}

void eth_suspend(struct eth_dev *dev)
{
    DBG(dev, "%s\n", __func__);

    dev->ctx.stats.dev_suspend++;

    if (!dev->gadget->rwakeup) {
        if(!bsp_usb_is_ncm_bypass_mode()){
            netif_stop_queue(dev->net);
            dev->ctx.stats.tx_stop_queue++;
        }
    }

    dev->is_suspend = 1;
}

void eth_resume(struct eth_dev *dev)
{
    DBG(dev, "%s\n", __func__);

    dev->ctx.stats.dev_resume++;

    eth_start(dev, GFP_ATOMIC);

    dev->is_suspend = 0;
}

static int eth_open(struct net_device *net)
{
    struct eth_dev  *dev = netdev_priv(net);
    struct gether   *link;
    int ret;
    
    dev->ctx.stats.dev_open++;


    USB_DBG_GADGET(&dev->gadget->dev, "%s\n", __func__);
    if (netif_carrier_ok(net))
        eth_start(dev, GFP_KERNEL);

    spin_lock_irq(&dev->lock);
    link = dev->port_usb;
    if (link && link->open)
        link->open(link);
    spin_unlock_irq(&dev->lock);
    
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    if(!spe_hook.port_is_enable(g_usb_portno)){
        ret = spe_hook.port_enable(g_usb_portno);
        if (ret) {
            return ret;
        }
    }
#endif
    return 0;
}

static int eth_stop(struct net_device *net)
{
    struct eth_dev  *dev = netdev_priv(net);
    unsigned long   flags;

    dev->ctx.stats.dev_stop++;

    VDBG(dev, "%s\n", __func__);
    
    if(!bsp_usb_is_ncm_bypass_mode()){
        netif_stop_queue(net);
        dev->ctx.stats.tx_stop_queue++;
    }
    
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    if(spe_hook.port_is_enable(g_usb_portno))
        (void)spe_hook.port_disable(g_usb_portno);
#endif
    DBG(dev, "stop stats: rx/tx %ld/%ld, errs %ld/%ld\n",
        net->stats.rx_packets, net->stats.tx_packets,
        net->stats.rx_errors, net->stats.tx_errors
        );

    /* ensure there are no more active requests */
    spin_lock_irqsave(&dev->lock, flags);
    if (dev->port_usb) {
        struct gether   *link = dev->port_usb;
    /*Disable and enable usb endpoint will cause usb ep fail.
        So we disable this process for the time being.*/
    /*
        const struct usb_endpoint_descriptor *in;
        const struct usb_endpoint_descriptor *out;
    */
        if (link->close)
            link->close(link);

        /* NOTE:  we have no abort-queue primitive we could use
         * to cancel all pending I/O.  Instead, we disable then
         * reenable the endpoints ... this idiom may leave toggle
         * wrong, but that's a self-correcting error.
         *
         * REVISIT:  we *COULD* just let the transfers complete at
         * their own pace; the network stack can handle old packets.
         * For the moment we leave this here, since it works.
         */
        /*Disable and enable usb endpoint will cause usb ep fail.
        So we disable this process for the time being.*/

        /*
        in = link->in_ep->desc;
        out = link->out_ep->desc;

        usb_ep_disable(link->in_ep);
        usb_ep_disable(link->out_ep);
        if (netif_carrier_ok(net)) {
            DBG(dev, "host still using in/out endpoints\n");
            link->in_ep->desc = in;
            link->out_ep->desc = out;
            usb_ep_enable(link->in_ep);
            usb_ep_enable(link->out_ep);
        
        }*/
    }
    spin_unlock_irqrestore(&dev->lock, flags);

    return 0;
}

/*-------------------------------------------------------------------------*/

/* initial value, changed by "ifconfig usb0 hw ether xx:xx:xx:xx:xx:xx" */
static char *dev_addr[GNET_MAX_NUM]={
    "0A:5B:8F:27:9A:64",
    "0A:5B:8F:27:9A:65",
    "0A:5B:8F:27:9A:66",
    "0A:5B:8F:27:9A:67"
    };
module_param_array(dev_addr, charp, NULL, S_IRUGO);
MODULE_PARM_DESC(dev_addr, "Device Ethernet Address");

/* this address is invisible to ifconfig */
static char *host_addr[GNET_MAX_NUM]={
    "0C:5B:8F:27:9A:64",
    "0C:5B:8F:27:9A:65",
    "0C:5B:8F:27:9A:66",
    "0C:5B:8F:27:9A:67"
};
module_param_array(host_addr, charp, NULL, S_IRUGO);
MODULE_PARM_DESC(host_addr, "Host Ethernet Address");

static int get_ether_addr(const char *str, u8 *dev_addr)
{
    if (str) {
        unsigned    i;

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
    eth_random_addr(dev_addr);
    return 1;
}

static const struct net_device_ops eth_netdev_ops = {
	.ndo_open		= eth_open,
	.ndo_stop		= eth_stop,
	.ndo_start_xmit		= eth_start_xmit,
	.ndo_change_mtu		= ueth_change_mtu,
	.ndo_set_mac_address 	= ueth_set_mac_address,
	.ndo_validate_addr	= eth_validate_addr,
};

static struct device_type gadget_type = {
    .name   = "gadget",
};

static void eth_stats_clear(struct eth_dev  *dev)
{
    memset((void *)&dev->ctx.stats, 0 , sizeof(dev->ctx.stats));
}

static ssize_t
gnet_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
    struct eth_dev      *dev = container_of(attr,struct eth_dev,dev_attr);
    struct usb_ep *notify=NULL;
    int count;
    int i;

    if (!dev)
    {
        printk("no dev\n");
        return -ENODEV;
    }

    count = snprintf(buf,PAGE_SIZE,       "|-+balong gnet %d dump:\n",dev->ctx.eth_idx);
    count += snprintf(buf+count,PAGE_SIZE,"| |--gnet mode                   :%s\n",bsp_usb_netmode()?"NETWORK BYPASS MODE":"NETWORK MODE");
    count += snprintf(buf+count,PAGE_SIZE,"| |--tx ep name                  :%s\n",dev->port_usb?dev->port_usb->in_ep->name:"UNKNOWN");
    count += snprintf(buf+count,PAGE_SIZE,"| |--rx ep name                  :%s\n",dev->port_usb?dev->port_usb->out_ep->name:"UNKNOWN");
#if defined(CONFIG_BALONG_NCM)
    notify = dev->port_usb?func_to_ncm(&dev->port_usb->func)->notify:NULL;
#elif defined(CONFIG_BALONG_ECM)
    notify = dev->port_usb?func_to_ecm(&dev->port_usb->func)->notify:NULL;
#else
    notify = dev->port_usb?func_to_rndis(&dev->port_usb->func)->notify:NULL;
#endif
    count += snprintf(buf+count,PAGE_SIZE,"| |--notify ep name              :%s\n",notify?notify->name:"UNKNOWN");
    count += snprintf(buf+count,PAGE_SIZE,"| |--is_suspend                  :%d\n",dev->is_suspend);
    count += snprintf(buf+count,PAGE_SIZE,"| |--tx_qlen                     :%u\n",atomic_read(&dev->tx_qlen));
    count += snprintf(buf+count,PAGE_SIZE,"| |--tx_req_num                  :%u\n",dev->tx_req_num);
    count += snprintf(buf+count,PAGE_SIZE,"| |--rx_qlen                     :%u\n",atomic_read(&dev->rx_qlen));
    count += snprintf(buf+count,PAGE_SIZE,"| |--rx_req_num                  :%u\n",dev->rx_req_num);
    count += snprintf(buf+count,PAGE_SIZE,"| |--rx_req_waterline            :%u\n",dev->rx_req_waterline);
    count += snprintf(buf+count,PAGE_SIZE,"| |--rx_doclone                  :%u\n",dev->doclone);
    count += snprintf(buf+count,PAGE_SIZE,"| |--dwc trb cache num           :%u\n",USB_CDC_NCM_DWC_CACHE_TRB_NUM);
    if(bsp_usb_is_ncm_bypass_mode()){
        count += snprintf(buf+count,PAGE_SIZE,"| |--rx_frames_waterline         :%u\n",dev->rx_frames_waterline);
    }
#ifdef CONFIG_GNET_PREALLOC_TX_MEM
    count += snprintf(buf+count,PAGE_SIZE,"| |--sg_node_nents               :%u\n",dev->ctx.sg_node_nents);
#if defined(CONFIG_BALONG_NCM)
    count += snprintf(buf+count,PAGE_SIZE,"| |--ntb_node_size               :%u\n",dev->ctx.ntb_node_size);
#endif
#endif
#if defined(CONFIG_BALONG_NCM)
    count += snprintf(buf+count,PAGE_SIZE,"| |--skb hdr resved len          :%u\n",
                    dev->port_usb?container_of(dev->port_usb, struct f_ncm, port)->header_room_resved:0xffffffff);  /* 0xffffffff means invalid */
#endif

#ifdef CONFIG_GNET_PREALLOC_RX_MEM
    count += snprintf(buf+count,PAGE_SIZE,"| |--rx_skb_num                  :%u\n",dev->ctx.rx_skb_num);
    count += snprintf(buf+count,PAGE_SIZE,"| |--rx_skb_size                 :%u\n",dev->ctx.rx_skb_size);
    count += snprintf(buf+count,PAGE_SIZE,"| |--rx_skb_waterline            :%u\n",dev->ctx.rx_skb_waterline);
    count += snprintf(buf+count,PAGE_SIZE,"| |--rx_skb_free                 :%u\n",
        dev->ctx.rx_skb_free_q?dev->ctx.rx_skb_free_q->qlen:0xffffffff);
    count += snprintf(buf+count,PAGE_SIZE,"| |--rx_skb_done                 :%u\n",dev->ctx.rx_skb_done_q->qlen);
#endif
    count += snprintf(buf+count,PAGE_SIZE,"| |--timeout                     :%u\n",dev->ctx.timeout);
    count += snprintf(buf+count,PAGE_SIZE,"| |--protect_num                 :%u\n",dev->ctx.protect_num);
    count += snprintf(buf+count,PAGE_SIZE,"| |--pkt_cnt_waterline           :%u\n",dev->ctx.pkt_cnt_waterline);
    count += snprintf(buf+count,PAGE_SIZE,"| |--pkt_len_waterline           :%u\n",dev->ctx.pkt_len_waterline);
    count += snprintf(buf+count,PAGE_SIZE,"| |--gnet_msg_level              :%08x\n",gnet_msg_level);
    count += snprintf(buf+count,PAGE_SIZE,"| |--gnet_dbg_level              :%08x\n",gnet_dbg_level);
    count += snprintf(buf+count,PAGE_SIZE,"| |--quirks                      :%08x\n",dev->ctx.quirks);
    count += snprintf(buf+count,PAGE_SIZE,"| |--dwc_trb_cache_buf           :%08x\n",(u32)dev->ctx.dwc_trb_cache_buf);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->dev_no_wrap           :%lu\n",dev->ctx.stats.dev_no_wrap);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->dev_no_port           :%lu\n",dev->ctx.stats.dev_no_port);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->dev_wrap_fail         :%lu\n",dev->ctx.stats.dev_wrap_fail);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->dev_disconnect        :%lu\n",dev->ctx.stats.dev_disconnect);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->dev_connect           :%lu\n",dev->ctx.stats.dev_connect);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->dev_open              :%lu\n",dev->ctx.stats.dev_open);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->dev_start             :%lu\n",dev->ctx.stats.dev_start);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->dev_stop              :%lu\n",dev->ctx.stats.dev_stop);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->dev_suspend           :%lu\n",dev->ctx.stats.dev_suspend);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->dev_resume            :%lu\n",dev->ctx.stats.dev_resume);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_inep_null          :%lu\n",dev->ctx.stats.tx_inep_null);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_skb_check_err      :%lu\n",dev->ctx.stats.tx_skb_check_err);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_skb_tailrom_lack   :%lu\n",dev->ctx.stats.tx_skb_tailrom_lack);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_skb_tailrom_expand :%lu\n",dev->ctx.stats.tx_skb_tailrom_expand);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_zlp                :%lu\n",dev->ctx.stats.tx_zlp);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_expand_zlp         :%lu\n",dev->ctx.stats.tx_expand_zlp);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_expand_zlp1        :%lu\n",dev->ctx.stats.tx_expand_zlp1);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_quirk              :%lu\n",dev->ctx.stats.tx_quirk);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_filt_out           :%lu\n",dev->ctx.stats.tx_filt_out);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_drop_suspend       :%lu\n",dev->ctx.stats.tx_drop_suspend);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_no_ctx             :%lu\n",dev->ctx.stats.tx_no_ctx);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_no_sg              :%lu\n",dev->ctx.stats.tx_no_sg);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_no_ntb             :%lu\n",dev->ctx.stats.tx_no_ntb);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_ntb_overflow       :%lu\n",dev->ctx.stats.tx_ntb_overflow);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_no_req             :%lu\n",dev->ctx.stats.tx_no_req);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_noreq_inwrap       :%lu\n",dev->ctx.stats.tx_noreq_inwrap);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_bug_noreq          :%lu\n",dev->ctx.stats.tx_bug_noreq);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_bug_nopkt          :%lu\n",dev->ctx.stats.tx_bug_nopkt);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_no_donereq         :%lu\n",dev->ctx.stats.tx_no_donereq);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_stop_queue         :%lu\n",dev->ctx.stats.tx_stop_queue);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_prestop_queue      :%lu\n",dev->ctx.stats.tx_prestop_queue);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_wake_queue         :%lu\n",dev->ctx.stats.tx_wake_queue);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_req_dropped        :%lu\n",dev->ctx.stats.tx_req_dropped);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_complete_ok        :%lu\n",dev->ctx.stats.tx_complete_ok);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_complete_fail      :%lu\n",dev->ctx.stats.tx_complete_fail);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_by_timer           :%lu\n",dev->ctx.stats.tx_by_timer);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_timer_activated    :%lu\n",dev->ctx.stats.tx_timer_activated);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_skb_realloc        :%lu\n",dev->ctx.stats.tx_skb_realloc);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_gathered_bytes     :%lu\n",dev->ctx.stats.tx_gathered_bytes);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_total              :%lu\n",dev->ctx.stats.tx_total);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_ok_bytes           :%lu\n",dev->ctx.stats.tx_ok_bytes);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_ok_pkts            :%lu\n",dev->ctx.stats.tx_ok_pkts);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_err_pkts           :%lu\n",dev->ctx.stats.tx_err_pkts);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_err_bytes          :%lu\n",dev->ctx.stats.tx_err_bytes);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_psh                :%lu\n",dev->ctx.stats.tx_psh);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_zlp                :%lu\n",dev->ctx.stats.rx_zlp);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_no_skb             :%lu\n",dev->ctx.stats.rx_no_skb);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_req_submitfail     :%lu\n",dev->ctx.stats.rx_req_submitfail);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_req_manudrop       :%lu\n",dev->ctx.stats.rx_req_manudrop);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_skb_protect_drop   :%lu\n",dev->ctx.stats.rx_skb_protect_drop);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_recycle_delay      :%lu\n",dev->ctx.stats.rx_skb_recycle_delay);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_completed          :%lu\n",dev->ctx.stats.rx_completed);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_complete_ok        :%lu\n",dev->ctx.stats.rx_complete_ok);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_spe_port_disable   :%lu\n",dev->ctx.stats.rx_spe_port_disable);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_unwrap_fail        :%lu\n",dev->ctx.stats.rx_unwrap_fail);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_packets            :%lu\n",dev->ctx.stats.rx_packets);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_bytes              :%lu\n",dev->ctx.stats.rx_bytes);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_spe_drop           :%lu\n",dev->ctx.stats.rx_spe_drop);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_spe_port_dis       :%lu\n",dev->ctx.stats.rx_spe_port_dis);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_packets_fail       :%lu\n",dev->ctx.stats.rx_packets_fail);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_bytes_fail         :%lu\n",dev->ctx.stats.rx_bytes_fail);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_packets_err        :%lu\n",dev->ctx.stats.rx_packets_err);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_packets_overwline  :%lu\n",dev->ctx.stats.rx_packets_overwline);
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_queue              :%lu\n",dev->ctx.stats.tx_queue);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_spe_total          :%lu\n",dev->ctx.stats.tx_spe_total);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_spe_complete_fail  :%lu\n",dev->ctx.stats.tx_spe_complete_fail);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_spe_complete_ok    :%lu\n",dev->ctx.stats.tx_spe_complete_ok);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_spe_rd_config      :%lu\n",dev->ctx.stats.tx_spe_rd_config);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_packet_from_host   :%lu\n",dev->ctx.stats.tx_packet_from_host);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_spe_no_req         :%lu\n",dev->ctx.stats.tx_spe_no_req);
    count += snprintf(buf+count,PAGE_SIZE,"| |--stat->tx_spe_send           :%lu\n",dev->ctx.stats.tx_spe_send);
#endif

    for(i=0; i<WORK_RX_BOTTROM; i++)
    {
        count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_kevent_busy[%d]    :%lu\n",i,dev->ctx.stats.rx_kevent_busy[i]);
        count += snprintf(buf+count,PAGE_SIZE,"| |--stat->rx_kevent_ok[%d]      :%lu\n",i,dev->ctx.stats.rx_kevent_ok[i]);
    }
    return count;
}

#define CMD_LEN (32)
struct gnet_sysfs_cmd
{
    char name[CMD_LEN];
    u32 format; /* 0:none 1:dec 2:hex */
};

static struct gnet_sysfs_cmd g_gnet_cmdtbl[]=
{
    {"gnet_msg_level",2},
    {"gnet_dbg_level",2},
    {"pkt_cnt_waterline",1},
    {"pkt_len_waterline",1},
    {"timeout",1},
    {"rx_req_waterline",1},
    {"rx_skb_waterline",1},
    {"quirks",2},
    {"rx_doclone",1},
    {"clrstats",0},
    {"help",0},
    {"rx_frames_waterline",1}
};

void gnet_sysfs_help(void)
{
    u32 loop;
    u32 format;

    printk("gnet sysfs cmdlist:\n");
    for(loop=0; loop<sizeof(g_gnet_cmdtbl)/sizeof(struct gnet_sysfs_cmd); loop++)
    {
        format = g_gnet_cmdtbl[loop].format;/* [false alarm]:fortify disable */
        printk("->%s\n",g_gnet_cmdtbl[loop].name);
    }
}

static ssize_t
gnet_store(struct device *pdev, struct device_attribute *attr,
                    const char *buf, size_t size)
{

    struct eth_dev      *dev = container_of(attr,struct eth_dev,dev_attr);
    u32 loop;
    u32 cmd_value = 0;
    char cmd_str[CMD_LEN] = {0};
    /* coverity[dead_error_condition] */
    if (!dev)/* [false alarm]:fortify disable */
    {
        /* coverity[dead_error_begin] */
        printk("no dev\n");/* [false alarm]:fortify disable */
        return -ENODEV;/* [false alarm]:fortify disable */
    }

    (void)sscanf(buf, "%s",cmd_str);/* [false alarm]:fortify disable */

    for(loop=0; loop<sizeof(g_gnet_cmdtbl)/sizeof(struct gnet_sysfs_cmd); loop++){
        if(!memcmp(cmd_str, g_gnet_cmdtbl[loop].name,strlen(g_gnet_cmdtbl[loop].name))){
            break;
        }
    }

    if(sizeof(g_gnet_cmdtbl)/sizeof(struct gnet_sysfs_cmd) == loop)
    {
        printk("unsupport cmd(%s)!\n",cmd_str);
        return size;
    }

    if(2 == g_gnet_cmdtbl[loop].format)
    {
    /* coverity[secure_coding] */
        (void)sscanf(buf, "%s %x",cmd_str,&cmd_value);/* [false alarm]:fortify disable */
    }
    else if(1 == g_gnet_cmdtbl[loop].format)
    {
    /* coverity[secure_coding] */
        (void)sscanf(buf, "%s %d",cmd_str,&cmd_value);/* [false alarm]:fortify disable */
    }
    else
    {
        ;
    }

    switch(loop)
    {
        case 0:
            gnet_msg_level = cmd_value;
            break;
        case 1:
            gnet_dbg_level = cmd_value;
            break;
        case 2:
            dev->ctx.pkt_cnt_waterline = cmd_value;
            break;
        case 3:
            dev->ctx.pkt_len_waterline = cmd_value;
            break;
        case 4:
            dev->rx_req_waterline = cmd_value;
            break;
        case 5:
#if defined(CONFIG_GNET_PREALLOC_RX_MEM)
            dev->ctx.rx_skb_waterline = cmd_value;
#endif
            break;
        case 6:
            dev->ctx.timeout = cmd_value;
            break;
        case 7:
            dev->ctx.quirks = cmd_value;
            break;
        case 9:
            eth_stats_clear(dev);
            break;
        case 10:
            gnet_sysfs_help();
            break;
        case 11:
            dev->rx_frames_waterline = cmd_value;
            break;
        default:
            break;
    }

    return size;
}

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))


/*In interrupt Ctx*/

static int gnet_finish_spe_td(int portno, struct sk_buff *skb, unsigned int flags)
{
    struct eth_dev  *dev = the_dev[SPE_GNET];

    gnet_recycle_rx_skb(dev, skb);

    
    return 0;
}

static void gnet_recycle_spe_rd(struct sk_buff *skb, struct eth_dev *dev)
{
    dma_addr_t dma;
    dma = spe_hook.get_skb_dma(skb);
    if(dev != NULL){
        dev->ctx.stats.tx_spe_rd_config++;
    }
    spe_hook.rd_config(g_usb_portno, skb, dma);
}


/*In interrupt Ctx*/
static int gnet_finish_spe_rd(int portno, int src_portno,
    struct sk_buff *skb, dma_addr_t dma, unsigned int flags)
{
    struct eth_dev  *dev = the_dev[SPE_GNET];

    if(dev  && dev->net ){
        dev->net->stats.tx_packets++;
        eth_start_xmit(skb, dev->net);
    }else{
        gnet_recycle_spe_rd(skb, NULL);
    }
    return 0;
}

int eth_adp_for_spe_init(void)
{
    int port_num;
    struct spe_port_attr  ncm_spe_port_attr;

    memset(&ncm_spe_port_attr, 0, sizeof (struct spe_port_attr));

    ncm_spe_port_attr.desc_ops.finish_rd =  gnet_finish_spe_rd;
    ncm_spe_port_attr.desc_ops.finish_td =  gnet_finish_spe_td;
    ncm_spe_port_attr.rd_depth = CONFIG_GNET_PREALLOC_SPE_RD_SKB_NUM;

    ncm_spe_port_attr.td_depth = 64;
#ifdef CONFIG_BALONG_NCM
    ncm_spe_port_attr.enc_type = spe_enc_ncm_ntb32;
#endif

#ifdef CONFIG_BALONG_RNDIS
    ncm_spe_port_attr.enc_type = spe_enc_rndis;
#endif

    ncm_spe_port_attr.attach_brg = spe_attach_brg_normal;
    ncm_spe_port_attr.net = NULL;
    ncm_spe_port_attr.rd_skb_num = ncm_spe_port_attr.rd_depth;
    ncm_spe_port_attr.rd_skb_size = NCMPORT_RD_SKB_SIZE;
#ifdef CONFIG_BALONG_NCM
    ncm_spe_port_attr.ncm_align_parameter = ncm_get_div_and_rem();
    ncm_spe_port_attr.ncm_max_size = ncm_get_max_length();
#endif

    if(bsp_usb_is_ncm_bypass_mode()){
        ncm_spe_port_attr.stick_mode = 1;
    }
    else{
        ncm_spe_port_attr.stick_mode = 0;
    }

    port_num = spe_hook.port_alloc(&ncm_spe_port_attr);
    if(port_num > 8 || port_num < 0){
        GNET_ERR(("spe port alloc failed \n"));
        return -ENOMEM;
    }
    g_usb_portno = port_num;

    
    return 0;
}

void gnet_spe_usb_unplug(struct eth_dev *dev)
{
    struct spe_port_attr_alt port_property;
    (void)spe_hook.port_disable(g_usb_portno);
    port_property.enc_type = spe_enc_none;
    port_property.desc_ops.finish_td = NULL;
    port_property.desc_ops.finish_rd = NULL;
    port_property.desc_ops.finish_rd_bypass = NULL;
    spe_hook.set_port_property(NULL, &port_property, g_usb_portno);
}

int gnet_spe_usb_plug(struct eth_dev *dev, unsigned int portno)
{
    struct net_device *net = NULL;
    struct spe_port_attr_alt port_property;
    int format = GNET_NTB32_FORMAT;
    int ret = 0;
	
    if(dev != the_dev[SPE_GNET])
    {
        GNET_ERR(("Spe does not support this ncm device. \n"));
        return 0;
    }
    
    if(portno >= SPE_PORT_NUM){
        GNET_ERR(("gnet_spe_usb_plug input port num(%u) err \n", portno));
        return -EINVAL;
    }

    port_property.enc_type = spe_enc_ncm_ntb32;
    port_property.desc_ops.finish_td = NULL;
    port_property.desc_ops.finish_rd = NULL;
    port_property.desc_ops.finish_rd_bypass = NULL;

    dev->spe_port_num = portno;
    
    net = dev->net;        
    /* config mac addr */
        
#ifdef CONFIG_BALONG_NCM
    if (dev->get_wrap_format && dev->port_usb) {
        format = dev->get_wrap_format(dev->port_usb);
    }
#endif

#ifdef CONFIG_BALONG_RNDIS
        format = GNET_RNDIS_FORMAT;
#endif

#ifdef CONFIG_BALONG_ECM
        format = GNET_ECM_FORMAT;
#endif

    switch (format) {
    case GNET_NTB32_FORMAT:
        port_property.enc_type = spe_enc_ncm_ntb32;
        break;
    case GNET_NTB16_FORMAT:
        port_property.enc_type = spe_enc_ncm_ntb16;
        break;
    case GNET_RNDIS_FORMAT:
        port_property.enc_type = spe_enc_rndis;
        break;
    case GNET_ECM_FORMAT:
        port_property.enc_type = spe_enc_none;
        break;
    default:
        GNET_ERR(("Currnet usb net device is not supported by spe. \n"));
        return 0;

    }

    spe_hook.set_port_property(net, &port_property, portno);
        
    ret = spe_hook.port_enable(portno);
    if (ret) {
        return ret;
    }

    return 0;
}
void gether_spe_port_disable(struct gether *link)
{
    struct eth_dev      *dev = link->ioport;
    if(spe_hook.is_enable && spe_hook.is_enable()){
    gnet_spe_usb_unplug(dev);
    }
}
#endif

/**
 * gether_setup_name - initialize one ethernet-over-usb link
 * @g: gadget to associated with these links
 * @ethaddr: NULL, or a buffer in which the ethernet address of the
 *  host side of the link is recorded
 * @netname: name for network device (for example, "usb")
 * Context: may sleep
 *
 * This sets up the single network link that may be exported by a
 * gadget driver using this framework.  The link layer addresses are
 * set up using module parameters.
 *
 * Returns negative errno, or zero on success
 */
struct eth_dev *gether_setup_name(struct usb_gadget *g, u8 ethaddr[ETH_ALEN],
        const char *netname)
{
    struct eth_dev      *dev;
    struct net_device   *net;
    int         status;

    net = alloc_etherdev(sizeof *dev);
    if (!net)
        return ERR_PTR(-ENOMEM);

    dev = netdev_priv(net);
    spin_lock_init(&dev->lock);
    spin_lock_init(&dev->req_lock);
    spin_lock_init(&dev->tx_req_lock);

    INIT_DELAYED_WORK(&dev->work, eth_work);
    INIT_LIST_HEAD(&dev->tx_done_reqs);
    INIT_LIST_HEAD(&dev->tx_reqs);
    INIT_LIST_HEAD(&dev->rx_reqs);
    atomic_set(&dev->rx_qlen, 0);

    skb_queue_head_init(&dev->rx_frames);
    
    if(bsp_usb_is_ncm_bypass_mode()){
        dev->rx_frames_waterline = RX_FRAMES_MAX;
        skb_queue_head_init(&dev->rx_frames_to_ads);
        INIT_WORK(&dev->rx_work, eth_rx_work);
    }

#ifdef CONFIG_GNET_PREALLOC_TX_MEM
    dev->ctx.sg_node_nents = GNET_PKT_NUM_WATERLINE;
#ifdef CONFIG_BALONG_NCM
    dev->ctx.ntb_node_size = (USB_CDC_NCM_NDP32_INDEX_MIN +
        USB_CDC_NCM_NDP32_MIN + GNET_PKT_NUM_WATERLINE * 8);
#endif  /* CONFIG_BALONG_NCM */
#endif  /* CONFIG_GNET_PREALLOC_TX_MEM */

#ifdef CONFIG_GNET_PREALLOC_RX_MEM
    /* init rx skb queue */
    /* skb_queue_head_init(&dev->ctx.rx_skb_done_q); */
    dev->ctx.rx_skb_size = GNET_OUT_MAXSIZE_PER_TRANSFER;
    dev->ctx.rx_skb_num = CONFIG_GNET_PREALLOC_USB_RX_SKB_NUM;
    dev->ctx.rx_skb_waterline = dev->ctx.rx_skb_num/4;
#endif

    /* init timer */
    init_timer(&dev->ctx.timer);
    dev->ctx.timer.function = gnet_timeout_handle;
    dev->ctx.timer.data = (unsigned long)dev;
    dev->ctx.timeout = USB_CDC_GNET_TIMEOUT;

    dev->ctx.pkt_cnt_waterline = GNET_PKT_NUM_WATERLINE;
    dev->ctx.pkt_len_waterline = GNET_PKT_LEN_WATERLINE;

    dev->ctx.quirks = QUIRK_DWC_TRB_CACHE;
    if(dev->ctx.quirks & QUIRK_DWC_TRB_CACHE)
    {
        dev->ctx.sg_node_reserve =
            dev->ctx.sg_node_nents/(USB_CDC_NCM_DWC_CACHE_TRB_NUM-2) + 2;
    }

#if defined(CONFIG_BALONG_RNDIS)
    dev->ctx.sg_node_reserve += (GNET_PKT_NUM_WATERLINE/GNET_MAX_PKT_PER_TRANSFER - 1);
#endif

    dev->ctx.sg_node_nents += dev->ctx.sg_node_reserve + 1;

    /* network device setup */
    dev->net = net;
    snprintf(net->name, sizeof(net->name), "%s%%d", netname);

    if (get_ether_addr(dev_addr[geth_idx], net->dev_addr))
    {
        GNET_INFO(("using random %s ethernet address\n", "self"));
    }

    if (get_ether_addr(host_addr[geth_idx], dev->host_mac))
    {
        GNET_INFO(("using random %s ethernet address\n", "host"));
    }

    if (ethaddr)
        memcpy(ethaddr, dev->host_mac, ETH_ALEN);

    net->netdev_ops = &eth_netdev_ops;

#if defined(CONFIG_BALONG_RNDIS)
    net->hard_header_len = ETH_HLEN;
    net->needed_headroom = RNDIS_PACKET_HEADER_LEN;
    net->needed_tailroom = GNET_ALIGN_LEN;
#endif

    SET_ETHTOOL_OPS(net, &ops);

    /* two kinds of host-initiated state changes:
     *  - iff DATA transfer is active, carrier is "on"
     *  - tx queueing enabled if open *and* carrier is "on"
     */
    netif_carrier_off(net);

    dev->gadget = g;
    SET_NETDEV_DEV(net, &g->dev);
    SET_NETDEV_DEVTYPE(net, &gadget_type);

    snprintf(dev->dev_sysfs_name, sizeof(dev->dev_sysfs_name), "gnet%d", geth_idx);
    dev->dev_attr.show = gnet_show;
    dev->dev_attr.store = gnet_store;
    dev->dev_attr.attr.name = dev->dev_sysfs_name;
    dev->dev_attr.attr.mode = S_IRUGO | S_IWUSR;
    status = device_create_file(&g->dev, &dev->dev_attr);
    if(status < 0)/* [false alarm]:fortify disable */
    {
        dev_dbg(&g->dev, "device_create_file failed, %d\n", status);
        free_netdev(net);
        return NULL;
    }

    status = bsp_usb_netmode()?0:register_netdev(net); /*stick mode needn't register netdev*/
    if (status < 0) {/* [false alarm]:fortify disable */
        dev_dbg(&g->dev, "register_netdev failed, %d\n", status);
        free_netdev(net);
        (void)device_remove_file(&g->dev, &dev->dev_attr);
        dev = ERR_PTR(status);
    } else {
        GNET_INFO(("MAC %pM\n", net->dev_addr));
        GNET_INFO(("HOST MAC %pM\n", dev->host_mac));
        dev->ctx.eth_idx = geth_idx;
        the_dev[geth_idx] = dev;
        geth_idx++;

        /* two kinds of host-initiated state changes:
         *  - iff DATA transfer is active, carrier is "on"
         *  - tx queueing enabled if open *and* carrier is "on"
         */
        netif_carrier_off(net);
    }
    atomic_set(&dev->rx_qlen, 0);

    return dev;
}

/**
 * gether_cleanup - remove Ethernet-over-USB device
 * Context: may sleep
 *
 * This is called to free all resources allocated by @gether_setup().
 */
void gether_cleanup(struct eth_dev *dev)
{
    if (!dev)
        return;

    (void)device_remove_file(&dev->gadget->dev, &dev->dev_attr);

    if(!bsp_usb_is_ncm_bypass_mode()){
        unregister_netdev(dev->net);
    }
    else{
        flush_work(&dev->rx_work);
    }

    flush_delayed_work(&dev->work);
    free_netdev(dev->net);
    the_dev[geth_idx] = NULL;
    geth_idx--;
}


/**
 * gether_connect - notify network layer that USB link is active
 * @link: the USB link, set up with endpoints, descriptors matching
 *  current device speed, and any framing wrapper(s) set up.
 * Context: irqs blocked
 *
 * This is called to activate endpoints and let the network layer know
 * the connection is active ("carrier detect").  It may cause the I/O
 * queues to open and start letting network packets flow, but will in
 * any case activate the endpoints so that they respond properly to the
 * USB host.
 *
 * Verify net_device pointer returned using IS_ERR().  If it doesn't
 * indicate some error code (negative errno), ep->driver_data values
 * have been overwritten.
 */
struct net_device *gether_connect(struct gether *link)
{
    struct eth_dev      *dev = link->ioport;
    int         result = 0;

    if (!dev)
        return ERR_PTR(-EINVAL);

    USB_DBG_GADGET(&dev->gadget->dev, "%s\n", __func__);

    link->in_ep->driver_data = dev;

    dev->ctx.stats.dev_connect++;
    link->in_ep->masknotready = bsp_usb_netmode()?0:1;/*mask not ready interrupt for e5 mode*/
    link->in_ep->remote_wakeup_en = 1;/*enable remote wakeup en interrupt*/
    link->in_ep->xfer_cmd_monitor =
        (GNET_DBG_XFERMON == (gnet_dbg_level & GNET_DBG_XFERMON));

    result = usb_ep_enable(link->in_ep);
    if (result != 0) {
        DBG(dev, "enable %s --> %d\n", link->in_ep->name, result);
        goto fail0;
    }

    link->out_ep->driver_data = dev;
    result = usb_ep_enable(link->out_ep);
    if (result != 0) {
        DBG(dev, "enable %s --> %d\n", link->out_ep->name, result);
        link->out_ep->enable_xfer_in_progress = 1;  /*mask not ready interrupt*/
        goto fail1;
    }

    result = alloc_requests(dev, link, qlen_rx(dev->gadget), qlen(dev->gadget));

#if defined(CONFIG_GNET_PREALLOC_RX_MEM)
    if (result == 0)
        result = alloc_rx_mem(dev);
#endif  /* CONFIG_GNET_PREALLOC_RX_MEM */


    if (result == 0) {
        dev->zlp = link->is_zlp_ok;
        USB_DBG_GADGET(&dev->gadget->dev, "qlen %d\n", qlen(dev->gadget));

        dev->header_len = link->header_len;
        dev->unwrap = link->unwrap;
        dev->get_wrap_format = link->get_wrap_format;

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
        if(spe_hook.is_enable && spe_hook.is_enable()){
            result = gnet_spe_usb_plug(dev, g_usb_portno);
            if (result != 0){
                GNET_ERR(("Spe usb plug config fail\n"));
                return ERR_PTR(result);
            }
        }
#endif

#if defined(CONFIG_BALONG_ECM) || defined(CONFIG_BALONG_RNDIS)
        dev->wrap = eth_gather_pkt;
#else
        dev->wrap = link->wrap;
#endif
        if(bsp_usb_is_ncm_bypass_mode()){
            dev->handle_rx = link->handle_rx;
        }

        spin_lock(&dev->lock);
        dev->port_usb = link;

        if(!bsp_usb_is_ncm_bypass_mode()){

            if (netif_running(dev->net)){
                if (link->open)
                    link->open(link);
            } else {
                if (link->close)
                    link->close(link);
            }
        }

        spin_unlock(&dev->lock);

        if(!bsp_usb_is_ncm_bypass_mode()){
            netif_carrier_on(dev->net);
            if (netif_running(dev->net)){
                eth_start(dev, GFP_ATOMIC);
            }
        }
        else{
            eth_start(dev, GFP_ATOMIC);
        }
    /* on error, disable any endpoints  */
    } else {
        (void) usb_ep_disable(link->out_ep);
fail1:
        (void) usb_ep_disable(link->in_ep);
    }
fail0:
    /* caller is responsible for cleanup on error */
    if (result < 0)
        return ERR_PTR(result);

    /* init the rx protect */
    gnet_rx_protect_init(dev);

    return dev->net;
}

/**
 * gether_disconnect - notify network layer that USB link is inactive
 * @link: the USB link, on which gether_connect() was called
 * Context: irqs blocked
 *
 * This is called to deactivate endpoints and let the network layer know
 * the connection went inactive ("no carrier").
 *
 * On return, the state is as if gether_connect() had never been called.
 * The endpoints are inactive, and accordingly without active USB I/O.
 * Pointers to endpoint descriptors and endpoint private data are nulled.
 */
void gether_disconnect(struct gether *link)
{
    struct eth_dev      *dev = link->ioport;
    struct usb_request  *req;


    WARN_ON(!dev);
    if (!dev)
        return;

    USB_DBG_GADGET(&dev->gadget->dev, "%s\n", __func__);

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    if(spe_hook.is_enable && spe_hook.is_enable()){
        gnet_spe_usb_unplug(dev);
    }
#endif

    if(!bsp_usb_is_ncm_bypass_mode()){
        netif_stop_queue(dev->net);
        netif_carrier_off(dev->net);
        dev->ctx.stats.tx_stop_queue++;
    }

    (void)del_timer(&dev->ctx.timer); /* delete timer */
    dev->ctx.stats.dev_disconnect++;

    /* disable endpoints, forcing (synchronous) completion
     * of all pending i/o.  then free the request objects
     * and forget about the endpoints.
     */
    usb_ep_disable(link->in_ep);
    spin_lock(&dev->tx_req_lock);

    while (!list_empty(&dev->tx_reqs)) {
        req = container_of(dev->tx_reqs.next, struct usb_request, list);
        list_del(&req->list);
        spin_unlock(&dev->tx_req_lock);
        if(req->context){
            free_tx_ctx((tx_ctx_t *)req->context);
            req->context = NULL;
        }
        usb_ep_free_request(link->in_ep, req);
        spin_lock(&dev->tx_req_lock);
    }
    spin_unlock(&dev->tx_req_lock);

    /* free the dwc_trb_cache_buf */
    if(dev->ctx.dwc_trb_cache_buf){
        kfree(dev->ctx.dwc_trb_cache_buf);
        dev->ctx.dwc_trb_cache_buf = NULL;
    }

    link->in_ep->driver_data = NULL;
    link->in_ep->desc = NULL;

    usb_ep_disable(link->out_ep);
    spin_lock(&dev->req_lock);
    while (!list_empty(&dev->rx_reqs)) {
        req = container_of(dev->rx_reqs.next, struct usb_request, list);
        list_del(&req->list);

        spin_unlock(&dev->req_lock);
        usb_ep_free_request(link->out_ep, req);
        spin_lock(&dev->req_lock);
    }
    spin_unlock(&dev->req_lock);
    link->out_ep->driver_data = NULL;
    link->out_ep->desc = NULL;

    free_rx_mem(dev);

    /* finish forgetting about this USB link episode */
    spin_lock(&dev->lock);  
    dev->header_len = 0;
    dev->unwrap = NULL;
    dev->wrap = NULL;
    dev->get_wrap_format = NULL;
    if(bsp_usb_is_ncm_bypass_mode()){
        dev->handle_rx = NULL;
    }

    dev->port_usb = NULL;
    dev->is_suspend = 0;
    spin_unlock(&dev->lock);
}

void free_sg_node(sg_node_t *sg_node)
{
    if(sg_node)
    {
        if(sg_node->sg_tbl.sgl)
        {
            sg_free_table(&sg_node->sg_tbl);
        }

        kfree(sg_node);
    }
}

sg_node_t *alloc_sg_node(u32 nents, gfp_t gfp_msk)
{
    sg_node_t *sg_node;

    sg_node = (sg_node_t *)kzalloc(sizeof(sg_node_t), gfp_msk);
    if(!sg_node)
    {
        goto fail;
    }

    if(sg_alloc_table(&sg_node->sg_tbl, nents, gfp_msk))
    {
        goto fail;
    }

    INIT_LIST_HEAD(&sg_node->entry);

    return sg_node;

fail:
    free_sg_node(sg_node);

    return NULL;
}

inline int32_t gnet_get_sgnode(tx_ctx_t *ctx, u32 nents, gfp_t gfp_msk)
{
#if defined(CONFIG_GNET_PREALLOC_TX_MEM)
    return 0;
#else
    sg_node_t *sg_node = NULL;

    sg_node = alloc_sg_node(nents, gfp_msk);
    if(!sg_node)
        return -ENOMEM;

    ctx->sg_node = sg_node;

    return 0;
#endif
}

static inline void gnet_put_sgnode(tx_ctx_t *ctx)
{
#if defined(CONFIG_GNET_PREALLOC_TX_MEM)
#else
    if(ctx->sg_node)
    {
        free_sg_node(ctx->sg_node);
        ctx->sg_node = NULL;
    }
#endif
}

static inline void free_tx_ctx(tx_ctx_t *tx_ctx)
{
    if(tx_ctx)
    {
        if(!skb_queue_empty(&tx_ctx->pkt_list))
        {
            gnet_recycle_ctx_info(tx_ctx, 0);
        }

#ifdef CONFIG_GNET_PREALLOC_TX_MEM
        if(tx_ctx->sg_node)
        {
            free_sg_node(tx_ctx->sg_node);
            tx_ctx->sg_node = NULL;
        }
#endif

#if defined(CONFIG_BALONG_NCM)
        if(tx_ctx->ntb)
        {
            ncm_free_ntbnode(tx_ctx->ntb);
            tx_ctx->ntb = NULL;
        }
#endif
        kfree(tx_ctx);
    }
}

int32_t alloc_tx_ctx(struct eth_dev *dev, unsigned n)
{
    int32_t   status = 0;
    tx_ctx_t    *tx_ctx = NULL;
    sg_node_t *sg_node = NULL;
#ifdef CONFIG_BALONG_NCM
    ntb_node_t *ntb_node = NULL;
#endif
    struct usb_request *req,*req_next;
    gfp_t gfp_msk = GFP_ATOMIC;

    if(!dev->ctx.dwc_trb_cache_buf)
    {
        dev->ctx.dwc_trb_cache_buf = kzalloc(1024, gfp_msk);
        if(!dev->ctx.dwc_trb_cache_buf)
        {
            GNET_ERR(("alloc dwc_trb_cache_buf failed\n"));
            return -ENOMEM;
        }
    }

    /* alloc tx ctx for tx req */
    list_for_each_entry_safe(req, req_next, &dev->tx_reqs, list)
    {
        /* alloc tx ctx node */
        tx_ctx = (tx_ctx_t *)kzalloc(sizeof(tx_ctx_t), gfp_msk);
        if(!tx_ctx)
            goto fail;

#ifdef CONFIG_GNET_PREALLOC_TX_MEM
        /* alloc sg node */
        sg_node = alloc_sg_node(dev->ctx.sg_node_nents, gfp_msk);
        if(!sg_node)
        {
            kfree(tx_ctx);
            goto fail;
        }

        tx_ctx->sg_node = sg_node;

#ifdef CONFIG_BALONG_NCM
        /* alloc ntb node */
        ntb_node = ncm_alloc_ntbnode(dev->ctx.ntb_node_size, gfp_msk);
        if(!ntb_node)
        {
            free_sg_node(sg_node);
            tx_ctx->sg_node = NULL;
            kfree(tx_ctx);
            goto fail;
        }

        tx_ctx->ntb = ntb_node;
#endif  /* CONFIG_BALONG_NCM */
#endif  /* CONFIG_GNET_PREALLOC_TX_MEM */

        skb_queue_head_init(&tx_ctx->pkt_list);
        tx_ctx->dev = dev;
        tx_ctx->req = req;
        req->complete = tx_complete;
        req->context = (void *)tx_ctx;
        req->length = 1;    /* dummy, just for usb_gadget_map_request para check */
        req->sg = tx_ctx->sg_node->sg_tbl.sgl;
#if defined(CONFIG_BALONG_ECM)
        req->sg_mode = USB_REQUEST_M_TRANSFER;  /* For ecm, we treat sg as a special use */
#endif
    }

    goto done;

fail:
    /* free all tx ctx */
    list_for_each_entry_safe(req, req_next, &dev->tx_reqs, list)
    {
        if(req->context)
        {
            free_tx_ctx((tx_ctx_t *)req->context);
            req->context = NULL;
        }
    }

    if(dev->ctx.dwc_trb_cache_buf)
    {
        kfree(dev->ctx.dwc_trb_cache_buf);
        dev->ctx.dwc_trb_cache_buf = NULL;
    }

    status = -ENOMEM;
done:
    return status;
}

void gnet_recycle_ctx_info(tx_ctx_t *ctx, uint success)
{
    struct eth_dev  *dev = ctx->dev;
    struct net_device *net = dev->net;
    struct sk_buff  *skb_loop = NULL;
    struct sk_buff  *skb_next = NULL;

    if(success)
    {
        dev->ctx.stats.tx_ok_bytes += ctx->pkt_len;
        dev->ctx.stats.tx_ok_pkts += ctx->pkt_cnt;
        net->stats.tx_bytes += ctx->pkt_len;
        net->stats.tx_packets += ctx->pkt_cnt;
    }
    else
    {
        dev->ctx.stats.tx_err_bytes += ctx->pkt_len;
        dev->ctx.stats.tx_err_pkts += ctx->pkt_cnt;
        net->stats.tx_dropped += ctx->pkt_cnt;
    }
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    if (ctx->spe_skb && ctx->spe_skb->spe_own) {
        gnet_recycle_spe_rd(ctx->spe_skb ,dev);
#ifdef CONFIG_BALONG_SKB_MEMBER
        ctx->spe_skb->used_check = 0;
#endif
        ctx->spe_skb = NULL;
    } else
#endif
    {
        //printk("%s, cnt:%d, list:0x%p\n",__func__, ctx->pkt_cnt, &ctx->pkt_list);
        skb_queue_walk_safe(&ctx->pkt_list,skb_loop,skb_next)
        {
     //     printk("     loop=%d skb 0x%x skb_next 0x%x spe_own %d\n ",
    //              loop, skb_loop, skb_loop->next,skb_loop->spe_own);
            /* coverity[var_assigned] */
            skb_loop = __skb_dequeue(&ctx->pkt_list);
#ifdef CONFIG_BALONG_SKB_MEMBER
            skb_loop->used_check = 0;
#endif
            /* coverity[dereference] */
            dev_kfree_skb_any(skb_loop);
        }

        gnet_put_sgnode(ctx);
#ifdef CONFIG_BALONG_NCM
        ncm_put_ntbnode(ctx);
#endif
    }

    ctx->pkt_cnt = 0;
    ctx->pkt_len = 0;
}

void gnet_skb_refresh(struct sk_buff *skb)
{
#if defined(CONFIG_GNET_PREALLOC_RX_MEM)
    skb->data = skb->head + USB_NET_IP_ALIGN;
    skb_reset_tail_pointer(skb);
    skb->len = 0;
    skb->cloned = 0;
#endif
}

struct sk_buff *gnet_get_rx_skb(struct eth_dev  *dev, u32 size, gfp_t gfp_msk)
{
#if defined(CONFIG_GNET_PREALLOC_RX_MEM)
    struct sk_buff *skb = NULL;
    struct sk_buff *skb_next = NULL;
    unsigned long flags;
    u32 recycle_cnt = 0;

    spin_lock_irqsave(&dev->ctx.rx_skb_free_q->lock, flags);
    skb = __skb_dequeue(dev->ctx.rx_skb_free_q);
    if(skb) {
        spin_unlock_irqrestore(&dev->ctx.rx_skb_free_q->lock, flags);
        return skb;
    }

    /* we have locked the local irq in rx_skb_free_q->lock */
    spin_lock(&dev->ctx.rx_skb_done_q->lock);
    skb_queue_walk_safe(dev->ctx.rx_skb_done_q, skb, skb_next)
    {
        if(skb_cloned(skb))
        {
            /* contine to recycle skb when no free skb to use
             * in the non-interrupt process context
             */
            if (skb_queue_empty(dev->ctx.rx_skb_free_q) && !in_interrupt()) {
                continue;
            }
            else {
                break;
            }
        }

        __skb_unlink(skb, dev->ctx.rx_skb_done_q);

        gnet_skb_refresh(skb);

        __skb_queue_tail(dev->ctx.rx_skb_free_q, skb);

        if(++recycle_cnt >= dev->ctx.rx_skb_waterline)
        {
            break;
        }
    }
    skb = __skb_dequeue(dev->ctx.rx_skb_free_q);
    spin_unlock(&dev->ctx.rx_skb_done_q->lock);

    spin_unlock_irqrestore(&dev->ctx.rx_skb_free_q->lock, flags);
    return skb;
#else
    return alloc_skb(size + USB_NET_IP_ALIGN, gfp_msk);
#endif
}

void gnet_recycle_rx_skb(struct eth_dev  *dev, struct sk_buff *skb)
{
#if defined(CONFIG_GNET_PREALLOC_RX_MEM)
    if(skb_cloned(skb))
    {
        if(dev){
            dev->ctx.stats.rx_skb_recycle_delay++;
        }
        return;
    }

    gnet_skb_refresh(skb);
    skb_queue_tail(g_rx_skb_free_q, skb);
#else
    dev_kfree_skb_any(skb);
#endif
}

void gnet_put_rx_skb_to_done(struct eth_dev  *dev, struct sk_buff *skb)
{
#if defined(CONFIG_GNET_PREALLOC_RX_MEM)
    skb_queue_tail(dev->ctx.rx_skb_done_q, skb);
#else
#endif
}

void free_rx_mem(struct eth_dev *dev)
{
#if defined(CONFIG_GNET_PREALLOC_RX_MEM)
    struct sk_buff *skb = NULL, *skb_next = NULL;
    ulong flags;

    spin_lock_irqsave(&dev->ctx.rx_skb_done_q->lock, flags);
    skb_queue_walk_safe(dev->ctx.rx_skb_done_q, skb, skb_next)
    {
        if(skb_cloned(skb))
        {
            GNET_DBG(("%s:skb %08x now cloned, stay in done queue.\n",__FUNCTION__,(u32)skb));
            continue;
        }

        __skb_unlink(skb, dev->ctx.rx_skb_done_q);

        gnet_skb_refresh(skb);

        skb_queue_tail(dev->ctx.rx_skb_free_q, skb);
    }
    spin_unlock_irqrestore(&dev->ctx.rx_skb_done_q->lock, flags);
#endif
}

int usb_prealloc_eth_rx_mem(void)
{
#ifdef CONFIG_GNET_PREALLOC_RX_MEM
    struct sk_buff *skb = NULL;
    u32 cnt = 0;
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    dma_addr_t dma;
#endif

    if(!g_rx_skb_free_q)
    {
        g_rx_skb_free_q = kzalloc(sizeof(struct sk_buff_head), GFP_ATOMIC);
        if(!g_rx_skb_free_q)
        {
            goto nomem;
        }

        skb_queue_head_init(g_rx_skb_free_q);

        /* prealloc rx skbs */
        for(cnt = 0; cnt < CONFIG_GNET_PREALLOC_USB_RX_SKB_NUM; cnt++)
        {
            skb = alloc_skb(GNET_OUT_MAXSIZE_PER_TRANSFER + USB_NET_IP_ALIGN, GFP_ATOMIC);
            if(!skb)
            {
                goto nomem;
            }

            skb_reserve(skb, USB_NET_IP_ALIGN);
            skb_queue_tail(g_rx_skb_free_q, skb);
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
            if(spe_hook.is_enable && spe_hook.is_enable()){
                dma = dma_map_single(NULL, skb->data, skb->len, DMA_FROM_DEVICE);
                spe_hook.set_skb_dma(skb, dma);
            }
#endif
        }
    }

    if(!g_rx_skb_done_q)
    {
        g_rx_skb_done_q = kzalloc(sizeof(struct sk_buff_head), GFP_ATOMIC);
        if(!g_rx_skb_done_q)
        {
            goto nomem;
        }

        skb_queue_head_init(g_rx_skb_done_q);
    }

    return 0;

nomem:
    if(g_rx_skb_free_q)
    {
        while(NULL != (skb = skb_dequeue(g_rx_skb_free_q)))
        {
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
            if(spe_hook.is_enable && spe_hook.is_enable()){
                dma = spe_hook.get_skb_dma(skb);
                dma_unmap_single(NULL, dma, skb->len, DMA_FROM_DEVICE);
            }
#endif
            dev_kfree_skb_any(skb);
        }

        kfree(g_rx_skb_free_q);
        g_rx_skb_free_q = NULL;
    }

    if(g_rx_skb_done_q)
    {
        kfree(g_rx_skb_done_q);
        g_rx_skb_done_q = NULL;
    }

    return -ENOMEM;
#else
    return 0;
#endif
}

int32_t alloc_rx_mem(struct eth_dev *dev)
{
#if defined(CONFIG_GNET_PREALLOC_RX_MEM)
    int32_t ret;

    ret = usb_prealloc_eth_rx_mem();
    if(ret)
    {
        return ret;
    }

    dev->ctx.rx_skb_free_q = g_rx_skb_free_q;
    dev->ctx.rx_skb_done_q = g_rx_skb_done_q;
    return 0;
#endif
}

/**
 * make_clone_skb_writable
 * @skb: sk_buff to be written by tcp/ip layer.
 * Context: all
 *
 * This is the clone skb optimazation function routine for NCM/RNDIS/ECM, which revise
 * the skb state to permit the tcp/ip layer modifing the cloned skb data buffer as
 * needed as possible without making another copy of the data buffer.
 *
 * Returns:none
 */

void make_clone_skb_writable(struct sk_buff *skb)
{
#if defined(CONFIG_GNET_PREALLOC_RX_MEM)
    if(!bsp_usb_is_ncm_bypass_mode()){
        skb_header_release(skb);
        /* ip_hdr + ip_options + tcp_hdr  = 80 at most */
        skb->hdr_len = skb_headroom(skb) + 80;      
    }
#endif
}

/**
 * gnet_copy_skb
 * @skb: sk_buff to be copied
 * @offset: src offset from the beginning of skb->data
 * @len: total length to be copied
 * context: in irq
 *
 * This is the skb receive copy routine for NCM. According to the current
 * implementation, ncm driver prepares 16KB buffer to receive the
 * aggregated packets, it would be better to clone the skb to the tcp/ip
 * protocol. However, in ip_forward, ip_cow seems to COPY the cloned
 * skb in an unexpected way, so before the clone bug is fixed, we do the
 * copy at the receive routine in advance.
 */
struct sk_buff * gnet_copy_skb(struct sk_buff *skb, int offset, int len)
{
    struct sk_buff *n;

    if (skb->len <= (len + offset)) {
        GNET_ERR(("====ERROR:%s skb->len=%d should be greater than len=%d \
            + offset=%d====\n",__FUNCTION__, skb->len, len, offset));
        return NULL;
    }

    /* reserve 8 bytes tailroom */
    n = dev_alloc_skb(len + 8);
    if(!n) {
        GNET_ERR(("====WARNING:%s memery not sufficient====\n",__FUNCTION__));
        return n;
    }

    /* Set the tail pointer and length */
    skb_put(n, len);

    /* NO need to copy skb header, as not filled this moment, so just copy the buffer*/
    if (skb_copy_bits(skb, offset, n->data, len)) {
        GNET_ERR(("====WARNING:%s failure to copy skb buff====\n",__FUNCTION__));
        dev_kfree_skb_any(n);
        n = NULL;
    }

    return n;
}


/*
os_set_thread_attr
    设置任务调度优先级,调度算法,性能调试使用
pid:
    look for ps cmd
priority:
    0~99
sched:
    SCHED_NORMAL        0
    SCHED_FIFO          1
    SCHED_RR            2
*/
int os_set_thread_attr(int pid, int priority, int sched)
{
    struct sched_param param;
    struct pid *pstPid;
    struct task_struct *pstTask;
    int ret = 0;

    param.sched_priority = priority;
    pstPid = find_get_pid(pid);
    pstTask = pid_task(pstPid, PIDTYPE_PID);
    if (sched > 2 || sched < 0)
    {
        sched = SCHED_FIFO;
    }
    ret = sched_setscheduler(pstTask, sched, &param);
    if(ret)
        printk("os_set_thread_attr fail. \n");
    return 0;
}


int ncm_eth_mem_dump(char* buffer, unsigned int buf_size)
{
    unsigned int need_size;
    struct eth_dev_ctx*  ctx;
    unsigned int i;
    int num = 0;
    char* cur = buffer;

    /* no devs exist */
    if (0 == geth_idx) {
        return 0;
    }

    need_size = geth_idx * sizeof(struct eth_dev_ctx) + sizeof(int);
    /* no room left */
    if (need_size + 8> buf_size) {
        return -1;
    }
    cur += 2 * sizeof(int);
    /* record port num */
    for (i = 0; i < geth_idx && i < GNET_USED_NUM; i++) {
        if (the_dev[i]) {
            ctx = &the_dev[i]->ctx;
            ctx->offset = DUMP_OFFSET_OF(struct eth_dev_ctx, eth_idx);
            memcpy(cur, ctx, sizeof(struct eth_dev_ctx));
            cur += sizeof(struct eth_dev_ctx);
            num++;
        }
    }
    cur = buffer;
    *((int*)cur) = num;
    cur += sizeof(int);
    *((int*)cur) = sizeof(struct eth_dev_ctx);
    cur += sizeof(int);

    return (int)need_size + 8;
}


int gnet_wakeup_gadget(struct eth_dev *dev)
{
    if (dev->gadget->ops && dev->gadget->ops->wakeup) {
        return dev->gadget->ops->wakeup(dev->gadget);
    }

    return 1;
}


/*lint -restore*/

