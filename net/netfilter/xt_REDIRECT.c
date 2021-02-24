/*
 * (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2006 Netfilter Core Team <coreteam@netfilter.org>
 * Copyright (c) 2011 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Based on Rusty Russell's IPv4 REDIRECT target. Development of IPv6
 * NAT funded by Astaro.
 */

#include <linux/if.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/types.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter/x_tables.h>
#include <net/addrconf.h>
#include <net/checksum.h>
#include <net/protocol.h>
#include <net/netfilter/nf_nat.h>

/* Fix: Add kcmsmonitormsgtypes.h for L174 "ulRet = syswatch_nl_send( syswatch_rnic_connect );" */
#include "kcmsmonitormsgtypes.h"

#if defined(CONFIG_ATP_ONDEMAND_DIAL) || defined(CONFIG_ATP_QCT_ONDEMAND_DIAL)

extern int g_report_time ;

#ifndef CONFIG_ARCH_BALONG
#define RNIC_DIALRPT_UP 0x0600
#define DEVICE_ID_RNIC 11
#endif

#ifdef CONFIG_ARCH_BALONG
enum RNIC_DAIL_EVENT_TYPE_ENUM
{
    RNIC_DAIL_EVENT_UP                  = 0x0600,                               /*需要触发拨号*/
    RNIC_DAIL_EVENT_DOWN                        ,                               /*需要断开拨号 */
    RNIC_DAIL_EVENT_TYPE_BUTT
};
#endif

extern int  g_atp_redirect_dailmode;

enum AT_RNIC_DIAL_MODE_ENUM
{
    AT_RNIC_DIAL_MODE_MANUAL, /*Manual dial mode*/
    AT_RNIC_DIAL_MODE_DEMAND_CONNECT, /*Demand dial up*/
    AT_RNIC_DIAL_MODE_DEMAND_DISCONNECT, /*Demand dial down*/
    AT_RNIC_DIAL_MODE_BUTT
};
#endif
static const struct in6_addr loopback_addr = IN6ADDR_LOOPBACK_INIT;

static unsigned int
redirect_tg6(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct nf_nat_range *range = par->targinfo;
	struct nf_nat_range newrange;
	struct in6_addr newdst;
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;

	ct = nf_ct_get(skb, &ctinfo);
	if (par->hooknum == NF_INET_LOCAL_OUT)
		newdst = loopback_addr;
	else {
		struct inet6_dev *idev;
		struct inet6_ifaddr *ifa;
		bool addr = false;

		rcu_read_lock();
		idev = __in6_dev_get(skb->dev);
		if (idev != NULL) {
			list_for_each_entry(ifa, &idev->addr_list, if_list) {
				newdst = ifa->addr;
				addr = true;
				break;
			}
		}
		rcu_read_unlock();

		if (!addr)
			return NF_DROP;
	}

	newrange.flags		= range->flags | NF_NAT_RANGE_MAP_IPS;
	newrange.min_addr.in6	= newdst;
	newrange.max_addr.in6	= newdst;
	newrange.min_proto	= range->min_proto;
	newrange.max_proto	= range->max_proto;

	return nf_nat_setup_info(ct, &newrange, NF_NAT_MANIP_DST);
}

static int redirect_tg6_checkentry(const struct xt_tgchk_param *par)
{
	const struct nf_nat_range *range = par->targinfo;

	if (range->flags & NF_NAT_RANGE_MAP_IPS)
		return -EINVAL;
	return 0;
}

/* FIXME: Take multiple ranges --RR */
static int redirect_tg4_check(const struct xt_tgchk_param *par)
{
	const struct nf_nat_ipv4_multi_range_compat *mr = par->targinfo;

	if (mr->range[0].flags & NF_NAT_RANGE_MAP_IPS) {
		pr_debug("bad MAP_IPS.\n");
		return -EINVAL;
	}
	if (mr->rangesize != 1) {
		pr_debug("bad rangesize %u.\n", mr->rangesize);
		return -EINVAL;
	}
	return 0;
}
/*brief:此函数是用来上报按需拨号事件的*/
#if defined(CONFIG_ATP_ONDEMAND_DIAL) || defined(CONFIG_ATP_QCT_ONDEMAND_DIAL)
static void redirect_atp_send_ondemand_event(void)
{
    /* 系统启动后的时间*/
    struct timespec ts; 
    static struct timespec oldts = {0};
    int  ulRet = 0;
#ifdef CONFIG_ARCH_BALONG
    /*巴隆平台*/
    int  ulSize   = sizeof(DEVICE_EVENT);
    DEVICE_EVENT    stEvent;
    stEvent.device_id     = DEVICE_ID_WAN;
    stEvent.event_code    = RNIC_DAIL_EVENT_UP;
    stEvent.len           = 0;    
#else
    /*高通平台*/
    nl_dev_type syswatch_rnic_connect ;
    syswatch_rnic_connect.device_id = DEVICE_ID_RNIC;
    syswatch_rnic_connect.value = RNIC_DIALRPT_UP;
    syswatch_rnic_connect.desc = NULL;

#endif  

               

    memset(&ts,0,sizeof(struct timespec));
    
    /* 获取系统启动时间*/
    do_posix_clock_monotonic_gettime(&ts);

    /*控制包数，默认5s发一个*/
    if(ts.tv_sec - oldts.tv_sec >= g_report_time )
    {
   	    /*刷新oldts.tv_sec*/
		memcpy(&oldts,&ts,sizeof(struct timespec));
        /*上报netlink事件*/

#ifdef CONFIG_ARCH_BALONG
        ulRet = device_event_report(&stEvent,(int)ulSize);      /*巴隆*/ 
        if (0 != ulRet)
        {
           printk("%s\n","balong SendDialEvent failed");	 
           return ;
        }
#else
       ulRet = syswatch_nl_send( syswatch_rnic_connect );       /*高通*/ 
       if (0 != ulRet)
       {
           printk("%s\n","QCT SendDialEvent failed");
           return ;
       }
#endif
       printk("%s\n","syswatch_rnic_connect  send ondemand event");       
    }
}
#endif

static unsigned int
redirect_tg4(struct sk_buff *skb, const struct xt_action_param *par)
{
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	__be32 newdst;
	const struct nf_nat_ipv4_multi_range_compat *mr = par->targinfo;
	struct nf_nat_range newrange;

	NF_CT_ASSERT(par->hooknum == NF_INET_PRE_ROUTING ||
		     par->hooknum == NF_INET_LOCAL_OUT);

	ct = nf_ct_get(skb, &ctinfo);
	NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED));

	/* Local packets: make them go to loopback */
	if (par->hooknum == NF_INET_LOCAL_OUT)
		newdst = htonl(0x7F000001);
	else {
		struct in_device *indev;
		struct in_ifaddr *ifa;

		newdst = 0;

		rcu_read_lock();
		indev = __in_dev_get_rcu(skb->dev);
		if (indev && (ifa = indev->ifa_list))
			newdst = ifa->ifa_local;
		rcu_read_unlock();

		if (!newdst)
			return NF_DROP;
	}

	/* Transfer from original range. */
	memset(&newrange.min_addr, 0, sizeof(newrange.min_addr));
	memset(&newrange.max_addr, 0, sizeof(newrange.max_addr));
	newrange.flags	     = mr->range[0].flags | NF_NAT_RANGE_MAP_IPS;
	newrange.min_addr.ip = newdst;
	newrange.max_addr.ip = newdst;
	newrange.min_proto   = mr->range[0].min;
	newrange.max_proto   = mr->range[0].max;
/*判断是否需要上报按需拨号事件*/
#if defined(CONFIG_ATP_ONDEMAND_DIAL) || defined(CONFIG_ATP_QCT_ONDEMAND_DIAL)
    if(AT_RNIC_DIAL_MODE_DEMAND_DISCONNECT == g_atp_redirect_dailmode)
    {
        redirect_atp_send_ondemand_event();
    }
#endif       
	/* Hand modified range to generic setup. */
	return nf_nat_setup_info(ct, &newrange, NF_NAT_MANIP_DST);
}

static struct xt_target redirect_tg_reg[] __read_mostly = {
	{
		.name       = "REDIRECT",
		.family     = NFPROTO_IPV6,
		.revision   = 0,
		.table      = "nat",
		.checkentry = redirect_tg6_checkentry,
		.target     = redirect_tg6,
		.targetsize = sizeof(struct nf_nat_range),
		.hooks      = (1 << NF_INET_PRE_ROUTING) |
		              (1 << NF_INET_LOCAL_OUT),
		.me         = THIS_MODULE,
	},
	{
		.name       = "REDIRECT",
		.family     = NFPROTO_IPV4,
		.revision   = 0,
		.table      = "nat",
		.target     = redirect_tg4,
		.checkentry = redirect_tg4_check,
		.targetsize = sizeof(struct nf_nat_ipv4_multi_range_compat),
		.hooks      = (1 << NF_INET_PRE_ROUTING) |
		              (1 << NF_INET_LOCAL_OUT),
		.me         = THIS_MODULE,
	},
};

static int __init redirect_tg_init(void)
{
	return xt_register_targets(redirect_tg_reg,
				   ARRAY_SIZE(redirect_tg_reg));
}

static void __exit redirect_tg_exit(void)
{
	xt_unregister_targets(redirect_tg_reg, ARRAY_SIZE(redirect_tg_reg));
}

module_init(redirect_tg_init);
module_exit(redirect_tg_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Patrick McHardy <kaber@trash.net>");
MODULE_DESCRIPTION("Xtables: Connection redirection to localhost");
MODULE_ALIAS("ip6t_REDIRECT");
MODULE_ALIAS("ipt_REDIRECT");
