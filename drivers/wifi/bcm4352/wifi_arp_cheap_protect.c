/******************************************************************************
  *         版权所有 (C), 2001-2020, 华为技术有限公司
*******************************************************************************/

#include <linux/version.h>
#include <linux/if_arp.h>
#include <linux/if_vlan.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <net/net_namespace.h>
#include "br_private.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 11)
#define GET_BR_PORT(netdev) ((struct net_bridge_port *)br_port_get_rcu((struct net_device *)netdev))
#else
#define GET_BR_PORT(netdev) ((struct net_bridge_port *)rcu_dereference(((struct net_device *)netdev)->br_port))
#endif

static inline unsigned char* get_arp_src(const struct sk_buff* skb)
{
    __be16 usprotocol = 0;
    int i = 0;
    
    if (NULL == skb)
    {
        return NULL;
    }

    i = 12;
    usprotocol = *(__be16*) & (skb->data[i]);
    /* 下行包带vlan的场景 */
    while (htons(ETH_P_8021Q) == usprotocol)
    {
        i += 4; 
        usprotocol = *(__be16*) & (skb->data[i]);
    }
    /* 判断为arp*/
    if (htons(ETH_P_ARP) == usprotocol)
    {
        i += 16;
        return &(skb->data[i]);
    }
    return NULL;
}

static inline struct net_device * get_br_dev(struct net_device * wl_dev)
{
    struct net_bridge_port *port = NULL;

    port = GET_BR_PORT(wl_dev);
    if (port && port->br) {
        return port->br->dev;
    }

    return NULL;
}

static inline int check_arp_with_br(struct net_device * from_device, unsigned char* arp_src_ptr)
{
    struct in_device* indev = NULL;
    struct in_ifaddr* ifa = NULL;
    struct net_device * brdev = NULL;

    brdev = get_br_dev(from_device);
    if (NULL == brdev)
    {
        return 0;
    }

    indev = (struct in_device*)brdev->ip_ptr;
    if (NULL == indev)
    {
        return 0;
    }

    for_ifa(indev)
    {
        /* STA发送的包含arp源IP为当前桥接口地址ip的arp报文*/
        if (ifa->ifa_local == *((unsigned int*)arp_src_ptr))
        {
            printk("drop arp src %x\r\n",*((unsigned int*)arp_src_ptr));
            return 1;
        }
    }
    endfor_ifa(indev);

    return 0;
}

/*!
  \author      
  \brief       提供给驱动使用，用于判断STA->STA的报文是否满足arp欺骗的判断，
  \param[in]  *from_device: wlan接口的net_device结构指针
  \param[in]  *p: 该包的skb指针
  \return      1: 是欺骗报文则丢弃
  \retval ERROR_CODE 错误码说明
*/
int wl_chk_pkt_inBSS(struct net_device *from_device, void *p)
{
    unsigned char* arp_src_ptr = NULL;
    
    if (NULL == from_device)
    {
        return 0;
    }
    
    arp_src_ptr = get_arp_src((const struct sk_buff*)p);
    if (NULL == arp_src_ptr)
    {
        return 0;
    }
    
    return check_arp_with_br(from_device, arp_src_ptr);
}

EXPORT_SYMBOL(wl_chk_pkt_inBSS);