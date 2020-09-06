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

#ifdef CONFIG_IFCID_CFG
#define BRIDGE_MAX_NUM 8
struct bridge_ifcid_set_msg_st g_stLinkLocal_IPv6_Ifc[BRIDGE_MAX_NUM] = {0};
#endif


int atp_ipv6_setsock(struct sock* sk, int optname, char __user* optval, int valbool)
{
    int retv = 0;
    struct ipv6_pinfo *np = inet6_sk(sk);

    switch (optname)
    {   
        case IPV6_RECVNFMARK:
            np->rxopt.bits.rxnfmark = valbool; 
            break;
        case IPV6_ORIGINDEV:
            np->rxopt.bits.rxindev = valbool; 
            break;
        default: retv = -ENOPROTOOPT;
    }

    return retv;
}


int atp_ipv6_getsock(struct sock* sk, int optname, char __user* optval, int valbool)
{
    int val = 0;
    struct ipv6_pinfo *np = inet6_sk(sk);
    
    switch (optname)
    { 
        case IPV6_RECVNFMARK:
            val = np->rxopt.bits.rxnfmark;
            break;

        default: val = -ENOPROTOOPT;
    }
    
    return val;
}

EXPORT_SYMBOL_GPL(atp_ipv6_setsock);
EXPORT_SYMBOL_GPL(atp_ipv6_getsock);

