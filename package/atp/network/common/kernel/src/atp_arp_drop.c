

#include <linux/if_arp.h>
#include <linux/if_vlan.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/netfilter_bridge.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <net/net_namespace.h>
#include <linux/atphooks.h>
#include "br_private.h"
#include "atp_interface.h"

extern int atp_register_hook(struct atp_hook_ops* reg);
extern void atp_unregister_hook(struct atp_hook_ops* reg);

/*子特性用到的通用API*/
#if DESC("Common API")

#ifdef DEBUG_ARP_DROP
#define ATP_ARP_debug(args...)   color_printk(__func__, __LINE__, _RET_IP_, ## args)
#else
#define ATP_ARP_debug(args...)
#endif

/*获取arp头指针*/
static struct arphdr* get_arp_hdr(const struct sk_buff* skb)
{
    __be16 ucprotocol = 0;
    struct vlan_ethhdr* vhdr = NULL;

    if (NULL == skb)
    {
        return NULL;
    }

    if (__constant_htons(ETH_P_8021Q) == skb->protocol)
    {
        vhdr = (struct vlan_ethhdr*)skb->data;
        if (ntohs(ETH_P_ARP) == vhdr->h_vlan_encapsulated_proto)
        {
            return (struct arphdr*)(vhdr + 1);
        }

        return NULL;
    }

    ucprotocol = *(__be16*) & (skb->data[12]);
    if ((htons(ETH_P_ARP) == ucprotocol) || (htons(ETH_P_ARP) == skb->protocol))
    {
        return arp_hdr(skb);
    }

    return NULL;
}



static int check_arp_with_all_br(unsigned char* arp_src_ptr, unsigned char* arp_dst_ptr, int should_check_mask)
{
    struct net* net;
    struct net_device* dev;
    struct in_device* indev = NULL;
    struct in_ifaddr* ifa = NULL;
    ifa = ifa;
    for_each_net(net)
    {
        for_each_netdev(net, dev)
        {
            if (!(dev->priv_flags & (IFF_EBRIDGE)))
            {
                continue;
            }

            indev = (struct in_device*)dev->ip_ptr;
            if (NULL == indev)
            {
                continue;
            }

            for_ifa(indev)
            {
                if (should_check_mask)
                {
                    /* 如果是从nas接口下来的ARP 包,而且目的IP或是源IP与br0接口IP相同的网段，则将其丢弃 */
                    if (((ifa->ifa_local & ifa->ifa_mask) == ((*((unsigned int*)arp_src_ptr)) & ifa->ifa_mask))
                        || ((ifa->ifa_local & ifa->ifa_mask) == ((*((unsigned int*)arp_dst_ptr)) & ifa->ifa_mask)))
                    {
                        ATP_ARP_debug("Drop arp packet! by mask\r\n");
                        return 1;
                    }
                }
                else
                {
                    /* 过滤转发的非网关发送的包含网关br0地址ip的arp报文*/
                    if ( ifa->ifa_local == *((unsigned int*)arp_src_ptr) || ifa->ifa_local == *((unsigned int*)arp_dst_ptr))
                    {
                        ATP_ARP_debug("Drop arp packet! when equal\r\n");
                        return 1;
                    }
                }
            }
            endfor_ifa(indev);
        }
    }

    return 0;
}


/* arp包含的地址是否与bridgeIP地址冲突*/
static int arp_match_bridge_ip(struct sk_buff* skb, const struct net_device* dev)
{
    struct arphdr* arp = NULL;
    unsigned char* arp_dst_ptr = NULL;
    unsigned char* arp_src_ptr = NULL;

    arp = get_arp_hdr(skb);
    if (NULL == arp)
    {
        return 0;
    }

    /* 防止伪装arp应答和arp请求 ，检查是否包含bridge IP*/
    arp_src_ptr   = (unsigned char*)(arp + 1);
    arp_src_ptr  += dev->addr_len;
    arp_dst_ptr  = arp_src_ptr;
    arp_dst_ptr += 4 + dev->addr_len;

    return check_arp_with_all_br(arp_src_ptr, arp_dst_ptr, 0);

}




static int should_drop_arp_pakcet(struct sk_buff* skb, const struct net_device* dev)
{
    struct arphdr* arp = NULL;
    unsigned char* arp_dst_ptr = NULL;
    unsigned char* arp_src_ptr = NULL;

    if (NULL == dev)
    {
        return 0;
    }

    arp = get_arp_hdr(skb);
    if (NULL == arp)
    {
        return 0;
    }

    /* 如果是从nas接口下来的ARP 包,而且目的IP或是源IP与br0接口IP相同的网段，则将其丢弃 */
    if (IS_IP_WAN_DEVICE(dev->name))
    {
        arp_src_ptr = (unsigned char*)(arp + 1);
        arp_src_ptr += dev->addr_len;
        arp_dst_ptr = arp_src_ptr;
        arp_dst_ptr += 4 + dev->addr_len;

        return check_arp_with_all_br(arp_src_ptr, arp_dst_ptr, 1);
    }

    return 0;
}
#endif

/*ARP drop特性1:丢弃bridge内转发的非网关发送的包含网关bridge ip的arp报文*/
/*ARP drop特性2:bridge转发时丢弃从wan侧收到的与lan网段匹配的arp报文*/
#if DESC("Feature 1:drop arp in bridge forward which match with lan ip address;" \
         "Feature 2:drop arp in bridge forward which received from wan and match with lan subnet")

static unsigned int atp_arp_check_proc(unsigned int hooknum,
                                       struct sk_buff* skb,
                                       const struct net_device* in,
                                       const struct net_device* out, int (*okfn) (struct sk_buff*))
{
    /*丢弃bridge内转发的非网关发送的包含网关bridge ip的arp报文*/
    if (arp_match_bridge_ip(skb, in))
    {
        ATP_ARP_debug("drop arp packet because it has same ip with our bridge\n");
        return NF_DROP;
    }

    /*bridge转发时丢弃从wan侧收到的与lan网段匹配的arp报文*/
    if (should_drop_arp_pakcet(skb, in))
    {
        ATP_ARP_debug("drop arp packet because it come from wan and match lan subnet\n");
        return NF_DROP;
    }

    return NF_ACCEPT;
}


static struct nf_hook_ops atp_arp_check_hook =
{
    .hook = atp_arp_check_proc,
    .pf = PF_BRIDGE,
    .hooknum = NF_BR_FORWARD,
    .priority = NF_BR_PRI_BRNF,
};
#endif


/*ARP drop特性3:wan接口发包，不允许发送lan侧网段的arp报文*/
#if DESC("Feature 3:drop arp which match with lan subnet in wan xmit")

static int check_arp_in_dev_xmit(unsigned int hooknum,
        void* pvskb,
        void* pvdev,
        void* arg3)
{
    struct sk_buff* skb = NULL;
    struct net_device* dev = NULL;

    if (unlikely(NULL == pvskb) || unlikely(NULL == pvdev))
    {
        return ATP_CONTINUE;
    }

    skb = (struct sk_buff*)pvskb;
    dev = (struct net_device*)pvdev;

    /*wan 发包检查是否丢弃arp*/
    if (should_drop_arp_pakcet(skb, dev))
    {
        ATP_ARP_debug("drop arp in dev [%s]\n", dev->name);
        kfree_skb(skb);
        return ATP_STOP;
    }

    return ATP_CONTINUE;
}


static struct atp_hook_ops xmit_drop_arp_hook_ops =
{
    .hook = check_arp_in_dev_xmit,
    .hooknum = ATP_DEV_XMIT,
    .priority = ATP_PRI_DEFAULT,
};
#endif

static int __init atp_arp_drop_init(void)
{
    int ret = 0;

    ret = nf_register_hook(&atp_arp_check_hook);
    if (0 != ret)
    {
        return ret;
    }

    ret = atp_register_hook(&xmit_drop_arp_hook_ops);
    if (0 != ret)
    {
        nf_unregister_hook(&atp_arp_check_hook);
        return ret;
    }

    return 0;
}


static void __exit atp_arp_drop_fini(void)
{
    nf_unregister_hook(&atp_arp_check_hook);
    atp_unregister_hook(&xmit_drop_arp_hook_ops);
    return;
}


module_init(atp_arp_drop_init);
module_exit(atp_arp_drop_fini);


