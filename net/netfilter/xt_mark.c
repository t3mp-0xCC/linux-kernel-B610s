/*
 *	xt_mark - Netfilter module to match NFMARK value
 *
 *	(C) 1999-2001 Marc Boucher <marc@mbsi.ca>
 *	Copyright © CC Computer Consultants GmbH, 2007 - 2008
 *	Jan Engelhardt <jengelh@medozas.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>

#include <linux/netfilter/xt_mark.h>
#include <linux/netfilter/x_tables.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marc Boucher <marc@mbsi.ca>");
MODULE_DESCRIPTION("Xtables: packet mark operations");
MODULE_ALIAS("ipt_mark");
MODULE_ALIAS("ip6t_mark");
MODULE_ALIAS("ipt_MARK");
MODULE_ALIAS("ip6t_MARK");



static unsigned int
mark_tg0(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_mark_target_info *markinfo = par->targinfo;
	skb->mark = markinfo->mark;
	return XT_CONTINUE;
}


static unsigned int
mark_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_mark_tginfo2 *info = par->targinfo;

	skb->mark = (skb->mark & ~info->mask) ^ info->mark;
	return XT_CONTINUE;
}

static unsigned int
mark_tg1(struct sk_buff *skb, const struct xt_action_param *par)
{
    const struct xt_mark_target_info_v1 *markinfo = par->targinfo;
	int mark = 0;

	switch (markinfo->mode) {
	case XT_MARK_SET:
        if (markinfo->mark & 0x000ffff0)
        {
    		mark = (skb->mark & (~0x000ffff0)) | markinfo->mark;
        }
        else
        {
    		mark = markinfo->mark;
        }
		break;

	case XT_MARK_AND:
		mark = skb->mark & markinfo->mark;
		break;

	case XT_MARK_OR:
		mark = skb->mark | markinfo->mark;
		break;
	}
	skb->mark = mark;
	return XT_CONTINUE;
}


static bool mark_tg_check_v0(const struct xt_tgchk_param *par)
{
	const struct xt_mark_target_info *markinfo = par->targinfo;

	if (markinfo->mark > 0xffffffff) {
		printk(KERN_WARNING "MARK: Only supports 32bit wide mark\n");
		return false;
	}
	return true;
}

static bool mark_tg_check_v1(const struct xt_tgchk_param *par)
{
	const struct xt_mark_target_info_v1 *markinfo = par->targinfo;

	if (markinfo->mode != XT_MARK_SET
	    && markinfo->mode != XT_MARK_AND
	    && markinfo->mode != XT_MARK_OR) {
		printk(KERN_WARNING "MARK: unknown mode %u\n",
		       markinfo->mode);
		return false;
	}
	if (markinfo->mark > 0xffffffff) {
		printk(KERN_WARNING "MARK: Only supports 32bit wide mark\n");
		return false;
	}
	return true;
}

static bool
mark_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_mark_mtinfo1 *info = par->matchinfo;

	return ((skb->mark & info->mask) == info->mark) ^ info->invert;
}

static bool
mark_mt_v0(const struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_mark_info *info = par->matchinfo;

	return ((skb->mark & info->mask) == info->mark) ^ info->invert;
}

static bool mark_mt_check_v0(const struct xt_mtchk_param *par)
{
	const struct xt_mark_info *minfo = par->matchinfo;

	if (minfo->mark > 0xffffffff || minfo->mask > 0xffffffff) {
		printk(KERN_WARNING "mark: only supports 32bit mark\n");
		return false;
	}
	return true;
}


static struct xt_target mark_tg_reg __read_mostly = {
	.name           = "MARK",
	.revision       = 2,
	.family         = NFPROTO_UNSPEC,
	.target         = mark_tg,
	.targetsize     = sizeof(struct xt_mark_tginfo2),
	.me             = THIS_MODULE,
};


static struct xt_target mark_tg_reg1 __read_mostly = {
	.name           = "MARK",
	.revision       = 1,
	.checkentry	= mark_tg_check_v1,
	.family         = NFPROTO_UNSPEC,
	.target         = mark_tg1,
	.targetsize     = sizeof(struct xt_mark_target_info_v1),
	.me             = THIS_MODULE,
};

static struct xt_target mark_tg_reg0 __read_mostly = {
	.name           = "MARK",
	.revision       = 0,
	.checkentry	= mark_tg_check_v0,
	.family         = NFPROTO_UNSPEC,
	.target         = mark_tg0,
	.targetsize     = sizeof(struct xt_mark_target_info),
	.me             = THIS_MODULE,
};



static struct xt_match mark_mt_reg __read_mostly = {
	.name           = "mark",
	.revision       = 1,
	.family         = NFPROTO_UNSPEC,
	.match          = mark_mt,
	.matchsize      = sizeof(struct xt_mark_mtinfo1),
	.me             = THIS_MODULE,
};


static struct xt_match mark_mt_reg0 __read_mostly = {
	.name           = "mark",
	.revision       = 0,
	.family         = NFPROTO_UNSPEC,
	.match          = mark_mt_v0,
	.checkentry	    = mark_mt_check_v0,
	.matchsize      = sizeof(struct xt_mark_info),
	.me             = THIS_MODULE,
};
static int __init mark_mt_init(void)
{
	int ret;

	ret = xt_register_target(&mark_tg_reg0);
	ret = xt_register_target(&mark_tg_reg1);
	ret = xt_register_target(&mark_tg_reg);
	if (ret < 0)
		return ret;
	ret = xt_register_match(&mark_mt_reg);
	if (ret < 0) {
		xt_unregister_target(&mark_tg_reg);
		return ret;
	}
	ret = xt_register_match(&mark_mt_reg0);
	return 0;
}

static void __exit mark_mt_exit(void)
{
    xt_unregister_match(&mark_mt_reg0);
	xt_unregister_match(&mark_mt_reg);
	xt_unregister_target(&mark_tg_reg);
	xt_unregister_target(&mark_tg_reg0);
	xt_unregister_target(&mark_tg_reg1);
}

module_init(mark_mt_init);
module_exit(mark_mt_exit);
