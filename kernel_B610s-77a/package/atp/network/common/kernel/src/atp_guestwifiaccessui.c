
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#include <linux/netfilter.h>
#include <net/checksum.h>
#include <net/tcp.h>
#include "atp_guestwifiaccessui.h"
#define ATP_GUESTWIFI_DEBUG(msg, ...)    printk(KERN_DEBUG " [%s] [%d] [%s] "msg"\r\n", __FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__)
struct guestwifiaccessui_info g_stGuestwifiAccessuiInfo = {0};
#ifdef BSP_CONFIG_BOARD_E5_SB03
#define  DATALENGTH    1500
#define  GUESTWIFIWHITESTRING    "limitspeedforssidb"
#endif

unsigned int nf_guestwifiaccessui_in(struct sk_buff *skb, const struct net_device *pin, const struct net_device *pout)
{
#ifdef CONFIG_ATP_GETINDEV
    struct iphdr *iph = NULL;
    struct tcphdr *tcph = NULL;
    
#ifdef BSP_CONFIG_BOARD_E5_SB03
    unsigned char *data = NULL;
#endif

    
    /*如果禁止访问webui没有使能，不处理*/
    if (0 == g_stGuestwifiAccessuiInfo.lEnable)
    {
        return NF_ACCEPT;  
    }

    if (NULL == skb->lanindev )
    {
        ATP_GUESTWIFI_DEBUG("NULL == skb->lanindev");
        return NF_ACCEPT; 
    }

    /*如果不是从客人wifi过来的不处理*/
    if (strcmp(skb->lanindev->name, g_stGuestwifiAccessuiInfo.dev) != 0)
    {
        return NF_ACCEPT; 
    }

    iph = ip_hdr(skb);

    if (NULL == iph)
    {
        return NF_ACCEPT;  
    }
    
    if (iph->frag_off & htons(IP_OFFSET))
    {
        return NF_ACCEPT;
    }   

    /*如果目的地址不是网关或源地址是网关则不处理*/
    if ((iph->daddr != g_stGuestwifiAccessuiInfo.ul_lan_addr) || (iph->saddr == g_stGuestwifiAccessuiInfo.ul_lan_addr))
    {
        return NF_ACCEPT; 
    }

    /*如果不是tcp协议则不处理*/
    if (iph->protocol != IPPROTO_TCP)
    {
        return NF_ACCEPT;   
    }
    tcph = (void *)iph + iph->ihl * 4;


    /*如果不是http协议则不处理*/
    if ((tcph->dest != htons(80)) && (tcph->dest != htons(443)))
    {
        return NF_ACCEPT; 
    }
#ifdef BSP_CONFIG_BOARD_E5_SB03
    if (tcph->doff * 4 + iph->ihl * 4 == skb->len)
    {
        return NF_ACCEPT;
    }
    
    /*取出数据*/
    if (skb->len - tcph->doff * 4 - iph->ihl * 4 < DATALENGTH - 1)
    {
        data = (unsigned char *)kmalloc(DATALENGTH, GFP_KERNEL);
        if (data == NULL)
        {
            return NF_ACCEPT;
        }
        strncpy(data, (void *)tcph + tcph->doff * 4, skb->len - tcph->doff * 4 - iph->ihl * 4);
        if (strstr(data, GUESTWIFIWHITESTRING) != NULL)        
        {
            ATP_GUESTWIFI_DEBUG("it is guestwifi PRSITE request,so accept\n"); 
            kfree(data);
            return NF_ACCEPT;
        }
        else
        {
            kfree(data);
            return NF_DROP; 
        }
    }
    else
#endif
    {
        return NF_DROP;
    }

    return NF_DROP;
#else
    return NF_ACCEPT;
#endif
}
static unsigned int ipv4_guestwifiaccessui_in(unsigned int hooknum,
    struct sk_buff *skb,
    const struct net_device *in,
    const struct net_device *out,
    int (*okfn)(struct sk_buff *))
{
    return nf_guestwifiaccessui_in(skb, in, out);
}


static struct nf_hook_ops atp_guestwifiaccessui_hook =
{
    .hook       = ipv4_guestwifiaccessui_in,
    .owner      = THIS_MODULE,
    .pf    = NFPROTO_IPV4,
    .hooknum    = NF_INET_LOCAL_IN,
    .priority   = NF_IP_PRI_FIRST,
};

EXPORT_SYMBOL_GPL(nf_guestwifiaccessui_in);

static int __init   nf_guestwifiaccessui_init(void)
{
    int ret = 0;

    ret = nf_register_hook(&atp_guestwifiaccessui_hook);

    return ret;
}

static void __exit  nf_guestwifiaccessui_fini(void)
{
    nf_unregister_hook(&atp_guestwifiaccessui_hook);
    return;
}

module_init(nf_guestwifiaccessui_init);
module_exit(nf_guestwifiaccessui_fini);
