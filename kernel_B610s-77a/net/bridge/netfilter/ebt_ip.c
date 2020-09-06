/*
 *  ebt_ip
 *
 *	Authors:
 *	Bart De Schuymer <bdschuym@pandora.be>
 *
 *  April, 2002
 *
 *  Changes:
 *    added ip-sport and ip-dport
 *    Innominate Security Technologies AG <mhopf@innominate.com>
 *    September, 2002
 */
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/in.h>
#include <linux/module.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_ip.h>

struct tcpudphdr {
	__be16 src;
	__be16 dst;
};

#ifdef CONFIG_IP_PREC_TOS_REMARK
#define QOS_DSCP_MARK       0x1 /* 区分ebtables ftos 是dscp还是tos或者ipp */
#define QOS_IPP_MARK_ZERO   0x100 
#define QOS_TOS_MARK_ZERO   0x200 

#define IPTOS_IPP_MASK		0xE0
#define IPTOS_DSCP_MASK		0xFC
#define IPTOS_DSCP(tos)		((tos)&IPTOS_DSCP_MASK)
#endif
static bool
ebt_ip_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct ebt_ip_info *info = par->matchinfo;
	const struct iphdr *ih;
	struct iphdr _iph;
	const struct tcpudphdr *pptr;
	struct tcpudphdr _ports;

	ih = skb_header_pointer(skb, 0, sizeof(_iph), &_iph);
	if (ih == NULL)
		return false;
#if !defined CONFIG_IP_PREC_TOS_REMARK
	if (info->bitmask & EBT_IP_TOS &&
	   FWINV(info->tos != ih->tos, EBT_IP_TOS))
		return false;
#else
    if (false == atp_hook_once(ATP_EB_IP,info,ih,NULL))
	{
	    return false;
	}
#endif
	if (info->bitmask & EBT_IP_SOURCE &&
	   FWINV((ih->saddr & info->smsk) !=
	   info->saddr, EBT_IP_SOURCE))
		return false;
	if ((info->bitmask & EBT_IP_DEST) &&
	   FWINV((ih->daddr & info->dmsk) !=
	   info->daddr, EBT_IP_DEST))
		return false;
#ifdef CONFIG_BRIDGE_EBT_IP_IPRANGE		

    if (info->bitmask & EBT_IP_SRANGE) {
        u32 saddr = ntohl(ih->saddr);
        //printk("Ritchie line:%d saddr %x, min_p:%x, max_ip:%x \n",__LINE__,saddr,(info->src).min_ip,(info->src).max_ip);
        if (FWINV(saddr < ntohl((info->src).min_ip) ||
              saddr > ntohl((info->src).max_ip),
              EBT_IP_SRANGE))
        return false;
    }
    if (info->bitmask & EBT_IP_DRANGE) {
        u32 daddr = ntohl(ih->daddr);
        if (FWINV(daddr < ntohl((info->dst).min_ip) ||
              daddr > ntohl((info->dst).max_ip),
              EBT_IP_DRANGE))
        return false;
    }
#endif	
	if (info->bitmask & EBT_IP_PROTO) {
		if (FWINV(info->protocol != ih->protocol, EBT_IP_PROTO))
			return false;
		if (!(info->bitmask & EBT_IP_DPORT) &&
		    !(info->bitmask & EBT_IP_SPORT))
			return true;
		if (ntohs(ih->frag_off) & IP_OFFSET)
			return false;
		pptr = skb_header_pointer(skb, ih->ihl*4,
					  sizeof(_ports), &_ports);
		if (pptr == NULL)
			return false;
		if (info->bitmask & EBT_IP_DPORT) {
			u32 dst = ntohs(pptr->dst);
			if (FWINV(dst < info->dport[0] ||
				  dst > info->dport[1],
				  EBT_IP_DPORT))
			return false;
		}
		if (info->bitmask & EBT_IP_SPORT) {
			u32 src = ntohs(pptr->src);
			if (FWINV(src < info->sport[0] ||
				  src > info->sport[1],
				  EBT_IP_SPORT))
			return false;
		}
	}
	return true;
}

static int ebt_ip_mt_check(const struct xt_mtchk_param *par)
{
	const struct ebt_ip_info *info = par->matchinfo;
	const struct ebt_entry *e = par->entryinfo;

	if (e->ethproto != htons(ETH_P_IP) ||
	   e->invflags & EBT_IPROTO)
		return -EINVAL;
	if (info->bitmask & ~EBT_IP_MASK || info->invflags & ~EBT_IP_MASK)
		return -EINVAL;
	if (info->bitmask & (EBT_IP_DPORT | EBT_IP_SPORT)) {
		if (info->invflags & EBT_IP_PROTO)
			return -EINVAL;
		if (info->protocol != IPPROTO_TCP &&
		    info->protocol != IPPROTO_UDP &&
		    info->protocol != IPPROTO_UDPLITE &&
		    info->protocol != IPPROTO_SCTP &&
		    info->protocol != IPPROTO_DCCP)
			 return -EINVAL;
	}
	if (info->bitmask & EBT_IP_DPORT && info->dport[0] > info->dport[1])
		return -EINVAL;
	if (info->bitmask & EBT_IP_SPORT && info->sport[0] > info->sport[1])
		return -EINVAL;
#ifdef CONFIG_BRIDGE_EBT_IP_IPRANGE				
	if (info->bitmask & EBT_IP_SRANGE && (info->src).min_ip > (info->src).max_ip)
    {   
        //printk("Ritchie minip(%x) bigger than maxip(%x)\n",(info->src).min_ip, (info->src).max_ip);
		return -EINVAL;
    }
	if (info->bitmask & EBT_IP_DRANGE && (info->dst).min_ip > (info->dst).max_ip)
    {   
        //printk("Ritchie minip(%x) bigger than maxip(%x)\n",(info->dst).min_ip, (info->dst).max_ip);    
		return -EINVAL;
    }
#endif	
	return 0;
}

static struct xt_match ebt_ip_mt_reg __read_mostly = {
	.name		= "ip",
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.match		= ebt_ip_mt,
	.checkentry	= ebt_ip_mt_check,
	.matchsize	= sizeof(struct ebt_ip_info),
	.me		= THIS_MODULE,
};

static int __init ebt_ip_init(void)
{
	return xt_register_match(&ebt_ip_mt_reg);
}

static void __exit ebt_ip_fini(void)
{
	xt_unregister_match(&ebt_ip_mt_reg);
}

module_init(ebt_ip_init);
module_exit(ebt_ip_fini);
MODULE_DESCRIPTION("Ebtables: IPv4 protocol packet match");
MODULE_LICENSE("GPL");
