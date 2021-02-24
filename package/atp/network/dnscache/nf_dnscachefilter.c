
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/in.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include "nf_dnscache.h"

extern spinlock_t dnscache_lock;
extern int g_dnscacheswitch;

int g_filtertype = FILTERTYPEBLACK;
#define MACLENTH 20
static void filterdnscachemac(struct ethhdr *peth,char *pmac)
{
    if(peth)
    {
        snprintf(pmac,MACLENTH,"%02x:%02x:%02x:%02x:%02x:%02x",
        	peth->h_source[0],peth->h_source[1],peth->h_source[2],peth->h_source[3],peth->h_source[4],peth->h_source[5]);
    }
    return;
}

static unsigned int filterdnscachepacketv4(unsigned int hook,
        struct sk_buff* skb,
        const struct net_device* in,
        const struct net_device* out,
        int (*okfn)(struct sk_buff*)
                                          )
{

    struct iphdr* iph = NULL;
    
    struct ethhdr *peth = NULL;
    char devicemac[MACLENTH] = {0};
	
    if (TURNBRIDAGE == g_dnscacheswitch)
    {
        return NF_ACCEPT;
    }
    
    iph = ip_hdr(skb);

    if (NULL == iph)
    {
        return NF_ACCEPT;
    }
	
    if ((iph->daddr == 0) || (iph->daddr == 0xFFFFFFFF))
    {
        return NF_ACCEPT;
    }
    if ((iph->saddr == htonl(0x7F000001))
        || (iph->daddr == htonl(0x7F000001)))
    {
        return NF_ACCEPT;
    }
    if ((iph->protocol != IPPROTO_TCP)
        && (iph->protocol != IPPROTO_UDP)
        && (iph->protocol != IPPROTO_ICMP))
    {
        return NF_ACCEPT;
    }
    if (strncmp(skb->dev->name,LANINTERFACE,strlen(LANINTERFACE)))    
    {
        return NF_ACCEPT;
    }

    peth = skb->mac_header;
    filterdnscachemac(peth,devicemac);
    spin_lock_bh(&dnscache_lock);
    if (judge_ipcache_match(iph->daddr,devicemac))
    {
        spin_unlock_bh(&dnscache_lock);
        if (FILTERTYPEBLACK == g_filtertype)
        {
            return NF_DROP;
        }
        else
        {
            return NF_ACCEPT;
        }
    }
    spin_unlock_bh(&dnscache_lock);
    if (FILTERTYPEBLACK == g_filtertype)
    {
        return NF_ACCEPT;
    }

    return NF_DROP;
}




static unsigned int filterdnscachepacketv6(unsigned int hook,
        struct sk_buff* skb,
        const struct net_device* in,
        const struct net_device* out,
        int (*okfn)(struct sk_buff*)
                                          )
{
    struct ipv6hdr* ih6 = NULL;
    struct ethhdr *peth = NULL;
    char devicemac[MACLENTH] = {0};
		
    if (TURNBRIDAGE == g_dnscacheswitch)
    {
        return NF_ACCEPT;
    }
	
    ih6 = ipv6_hdr(skb);
    if (ih6 == NULL)
    {
        return NF_ACCEPT;
    }

    if ((!(__ipv6_addr_type(&ih6->saddr) & IPV6_ADDR_UNICAST)) ||
        (!(__ipv6_addr_type(&ih6->daddr) & IPV6_ADDR_UNICAST)))
    {
        return NF_ACCEPT;
    }
    if (strncmp(skb->dev->name,LANINTERFACE,strlen(LANINTERFACE)))    
    {
        return NF_ACCEPT;
    }

    peth = skb->mac_header;
    filterdnscachemac(peth,devicemac);
    spin_lock_bh(&dnscache_lock);
    if (judge_ipcache_matchv6(&ih6->daddr,devicemac))
    {
        spin_unlock_bh(&dnscache_lock);

        if (FILTERTYPEBLACK == g_filtertype)
        {
            return NF_DROP;
        }
        else
        {
            return NF_ACCEPT;
        }
    }
    spin_unlock_bh(&dnscache_lock);
    if (FILTERTYPEBLACK == g_filtertype)
    {
        return NF_ACCEPT;
    }

    return NF_DROP;
}




static struct nf_hook_ops dnscachefilter_ops[] __read_mostly =
{
    //ÔÚforwardÁ´Æ¥Åä
    {
        .hook = filterdnscachepacketv4,
        .pf = PF_INET,
        .hooknum = NF_INET_FORWARD,
        .priority = NF_IP_PRI_FILTER - 1,
    },

    
    {
        .hook = filterdnscachepacketv6,
        .pf = PF_INET6,
        .hooknum = NF_INET_FORWARD,
        .priority = NF_IP_PRI_FILTER - 1,
    },
};
static int init_hook_dnscachefilter(void)
{
    //×¢²á¹³×Óº¯Êý
    nf_register_hooks(dnscachefilter_ops, ARRAY_SIZE(dnscachefilter_ops));
    return 0;
}

static void fini_hook_dnscachefilter(void)
{
    //È¥×¢²á
    nf_unregister_hooks(dnscachefilter_ops, ARRAY_SIZE(dnscachefilter_ops));
    return;
}


MODULE_AUTHOR("Sliver iw ah");
module_init(init_hook_dnscachefilter);
module_exit(fini_hook_dnscachefilter);
