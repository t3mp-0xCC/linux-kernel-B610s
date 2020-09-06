


#include <linux/if_arp.h>
#include <linux/if_vlan.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/netfilter_arp.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <net/net_namespace.h>
#include <linux/atphooks.h>
#include "br_private.h"
#include "atp_interface.h"

extern int atp_register_hook(struct atp_hook_ops* reg);
extern void atp_unregister_hook(struct atp_hook_ops* reg);

/*ARP通用特性1:用户空间接收arp报文时获取lan dev*/
#if DESC("Feature 1:Receive lan devcie for arp PACKET socket")
#define PACKET_SKB_CB(__skb)    ((struct packet_skb_cb *)((__skb)->cb))

struct packet_skb_cb
{
    unsigned int origlen;
    union
    {
        struct sockaddr_pkt pkt;
        struct sockaddr_ll ll;
    } sa;
};

static int is_arp_pkt(const struct sk_buff* skb)
{
    __be16 ucprotocol = 0;
    struct vlan_ethhdr* vhdr = NULL;

    if (NULL == skb)
    {
        return 0;
    }

    if (__constant_htons(ETH_P_8021Q) == skb->protocol)
    {
        vhdr = (struct vlan_ethhdr*)skb->data;
        if (ntohs(ETH_P_ARP) == vhdr->h_vlan_encapsulated_proto)
        {
            return 1;
        }

        return 0;
    }

    ucprotocol = *(__be16*) & (skb->data[12]);
    if ((htons(ETH_P_ARP) == ucprotocol) || (htons(ETH_P_ARP) == skb->protocol))
    {
        return 1;
    }

    return 0;
}


static int arp_set_landev_to_skb(unsigned int hooknum,
        void* pvskb,
        void* arg2,
        void* arg3)
{
    struct sockaddr_pkt* spkt = NULL;
    struct sk_buff* skb = NULL;

    if (unlikely(NULL == pvskb))
    {
        return ATP_CONTINUE;
    }

    skb = (struct sk_buff*)pvskb;

    if (is_arp_pkt(skb))
    {
        spkt = &PACKET_SKB_CB(skb)->sa.pkt;
        strlcpy(spkt->spkt_device, skb->dev->name, sizeof(spkt->spkt_device));
    }

    return ATP_CONTINUE;
}


static struct atp_hook_ops landev_to_skb_hook_ops =
{
    .hook = arp_set_landev_to_skb,
    .hooknum = ATP_BR_LOCALIN_BF_CHG_DEV,
    .priority = ATP_PRI_DEFAULT,
};
#endif


/*ARP通用特性2:丢弃源mac与bridge mac相同的arp包*/
/*ARP通用特性3:丢弃目标是br0网段的来自IPOE wan侧的arp报文*/
#if DESC("Feature 2:not receive arp which src mac is same with br;" \
         "Feature 3:not learn lan side IP from IPOE WAN arp")
static unsigned int atp_security_arp_rcv(unsigned int hooknum,
        struct sk_buff* skb,
        const struct net_device* in,
        const struct net_device* out, int (*okfn) (struct sk_buff*))
{
    struct ethhdr* ethHeader = NULL;
    unsigned char* psMacTmp = NULL;
    struct net_device* brdev = NULL;
    struct in_device* indev = NULL;
    struct arphdr* arp = NULL;
    unsigned char* arp_ptr = NULL;
    __be32 sip = 0, tip = 0;

    if (unlikely(NULL == skb))
    {
        return NF_ACCEPT;
    }
    psMacTmp = psMacTmp;
    ethHeader = ethHeader;
    //711 skb中没有扩展lanindev字段,该小特性暂不支持
#ifndef CONFIG_ATP_ROUTE
    /*丢弃来自LAN侧源mac与bridge mac相同的arp包*/
#ifdef CONFIG_ATP_GETINDEV
    if ((NULL != skb->lanindev)
        && (NULL != skb->lanindev->br_port))
    {
        ethHeader = eth_hdr(skb);
        psMacTmp = skb->lanindev->br_port->br->dev->dev_addr;
        if (0 == memcmp(ethHeader->h_source, psMacTmp, ETH_ALEN))
        {
            return NF_DROP;
        }
    }
#endif    
#endif

    /*nas接口不能学习与br0接口IP同网段的ARP*/
    if (!IS_IP_WAN_DEVICE(skb->dev->name))
    {
        return NF_ACCEPT;
    }

    arp = arp_hdr(skb);
    arp_ptr = (unsigned char*)(arp + 1);
    arp_ptr += skb->dev->addr_len;
    memcpy(&sip, arp_ptr, 4);
    arp_ptr += 4;
    arp_ptr += skb->dev->addr_len;
    memcpy(&tip, arp_ptr, 4);

    brdev = dev_get_by_name(&init_net, ATP_DEFAULT_BRIDGE_NAME);
    if (NULL == brdev)
    {
        return NF_ACCEPT;
    }

    indev = in_dev_get(brdev);
    if (NULL == indev)
    {
        dev_put(brdev);
        return NF_ACCEPT;
    }

    for_ifa(indev)
    {
        /*ARP中sender IP 和 target IP 都不允许为br0同网段*/
        if (((ifa->ifa_local & ifa->ifa_mask) == (sip & ifa->ifa_mask))
            || ((ifa->ifa_local & ifa->ifa_mask) == (tip & ifa->ifa_mask)))
        {
            in_dev_put(indev);
            dev_put(brdev);
            return NF_DROP;
        }
    }
    endfor_ifa(indev);

    in_dev_put(indev);
    dev_put(brdev);
    return NF_ACCEPT;
}


static struct nf_hook_ops atp_arp_recv_hook =
{
    .hook = atp_security_arp_rcv,
    .pf = NFPROTO_ARP,
    .hooknum = NF_ARP_IN,
    .priority = NF_IP_PRI_SECURITY,
};
#endif

static int __init atp_arp_init(void)
{
    int ret = 0;

    ret = atp_register_hook(&landev_to_skb_hook_ops);
    if (0 != ret)
    {
        return ret;
    }

    ret = nf_register_hook(&atp_arp_recv_hook);
    if (0 != ret)
    {
        atp_unregister_hook(&landev_to_skb_hook_ops);
        return ret;
    }

    return 0;
}


static void __exit atp_arp_fini(void)
{
    nf_unregister_hook(&atp_arp_recv_hook);
    atp_unregister_hook(&landev_to_skb_hook_ops);

    return;
}


module_init(atp_arp_init);
module_exit(atp_arp_fini);


