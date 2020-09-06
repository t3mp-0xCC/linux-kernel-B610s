/******************************************************************************
  *         版权所有 (C), 2001-2020, 华为技术有限公司
*******************************************************************************/

#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/net.h>
#include <linux/in6.h>
#include <net/ipv6.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv6.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/version.h>


void atp_ipv6_skb_mark(struct sock *sk, struct msghdr *msg, struct sk_buff *skb)
{
    struct ipv6_pinfo *np = inet6_sk(sk);
    
    if (np->rxopt.bits.rxnfmark)
    {
        unsigned long nfmark = skb->mark;
        put_cmsg(msg, SOL_IPV6, IPV6_NFMARK, sizeof(nfmark), &nfmark);
    }
#ifdef CONFIG_ATP_GETINDEV
    if (np->rxopt.bits.rxindev)
    {
        char acName[IFNAMSIZ] = {0};
        
        if(skb->lanindev)
        {
            memcpy(acName, skb->lanindev->name, IFNAMSIZ);
        }
        put_cmsg(msg, SOL_IPV6, IPV6_ORIGINDEV, IFNAMSIZ, acName);
    }
#endif
    return;
}

EXPORT_SYMBOL_GPL(atp_ipv6_skb_mark);

