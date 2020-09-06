
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/stddef.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/version.h>
#include <linux/inetdevice.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_l3proto.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_extend.h>
#include <net/netfilter/nf_conntrack_acct.h>

#include <linux/atphooks.h>

extern int nf_conntrack_clean;
extern void nf_ct_iterate_cleanup(struct net *net, int (*iter)(struct nf_conn *i, void *data), void *data);


#ifdef CONFIG_ATP_COMMON_CT_CLEAN
/*检查标记了COMMON Delete的连接，返回1用于删除识别*/
static int kill_common_marked(struct nf_conn *i, void *data)
{
	if (test_bit(IPS_COMMON_DELETE_BIT, &i->status))
	{
		return 1;
	}

	return 0;
}

/*触发连接跟踪删除*/
void nf_conntrack_trigger_clean(int trigger_type)
{
	nf_conntrack_clean = trigger_type;

	return;
}
#endif



static int nf_conntrack_user_cleanup(unsigned int hooknum, void* pvNet, void *arg2, void *arg3)
{
	struct net *net = NULL;
	
	if (NULL == pvNet)
    {
        return ATP_CONTINUE;
    }

    net = (struct net *)pvNet;
	
    switch (nf_conntrack_clean)
    {
        case 0:
            /**  do nothing */
            break;

#ifdef CONFIG_ATP_COMMON_CT_CLEAN
        /*执行删除连接动作*/
        case IPS_COMMON_DELETE:
            nf_ct_iterate_cleanup(net, kill_common_marked, NULL);
            break;
#endif

        default:
            break;
    }
    nf_conntrack_clean = 0;
    
    return ATP_CONTINUE;
}

static struct atp_hook_ops nat_conntrack_clean_ops = {
    .hook = nf_conntrack_user_cleanup,
    .hooknum = ATP_CT_BEFOREINIT,
    .priority = ATP_PRI_DEFAULT,
};

static int __init nat_conntrack_clean_init(void)
{
	int ret = 0;

	ret = atp_register_hook(&nat_conntrack_clean_ops);
    if (ret < 0)
	{
		return ret;
	}

	return ret;
}

static void __exit nat_conntrack_clean_fini(void)
{
    atp_unregister_hook(&nat_conntrack_clean_ops);

    return;
}

module_init(nat_conntrack_clean_init);
module_exit(nat_conntrack_clean_fini);

