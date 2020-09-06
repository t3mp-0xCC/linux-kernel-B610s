/******************************************************************************
  *         版权所有 (C), 2001-2020, 华为技术有限公司
*******************************************************************************/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/device.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <linux/version.h>
#include <net/ipv6.h>
#include "hw_ssp_ker_objects.h"
#include "kcmsmonitormsgtypes.h"
#include "commonctcleanmsgapi.h"
#include "atp_common_ct_block.h"


extern hw_ssp_ker_obj_container* hw_ssp_ker_obj_create(const char* ObjProcName);

static ATP_COMMON_CT_BLOCK_HASH g_common_ct_block_hash[ATP_COMMON_CT_BLOCK_HASH_SIZE];

static struct timer_list g_block_timer;

static atomic_t g_need_block_check = ATOMIC_INIT(0);



static inline u32 atp_common_ct_block_hash(const struct nf_conntrack_tuple* tuple)
{
    unsigned int n;
    u32 h;

    /*计算数据区字长*/
    n = (sizeof(tuple->src) + sizeof(tuple->dst.u3)) / sizeof(u32);

    /*32位整形hash*/
    h = jhash2((u32*)tuple, n,
               (((__force __u16)tuple->dst.u.all << 16) |
                tuple->dst.protonum));

    return h % ATP_COMMON_CT_BLOCK_HASH_SIZE;
}



static void atp_common_ct_block_hash_init(void)
{
    int i = 0;

    for (i = 0; i < ATP_COMMON_CT_BLOCK_HASH_SIZE; i++)
    {
        INIT_LIST_HEAD(&g_common_ct_block_hash[i].list);
        spin_lock_init(&g_common_ct_block_hash[i].lock);
    }

    return;
}



void * atp_common_ct_block_lookup(const struct nf_conntrack_tuple* tuple)
{
    u32 hash_key = 0;
    ATP_COMMON_CT_BLOCK_NODE* block_node = NULL;

    if (NULL == tuple)
    {
        return NULL;
    }

    /*计算hash key*/
    hash_key = atp_common_ct_block_hash(tuple);

    spin_lock_bh(&g_common_ct_block_hash[hash_key].lock);

    /*在hash桶内查找*/
    list_for_each_entry(block_node, &g_common_ct_block_hash[hash_key].list, list)
    {
        if (nf_ct_tuple_equal(tuple, &block_node->tuple))
        {
            spin_unlock_bh(&g_common_ct_block_hash[hash_key].lock);
            return block_node;
        }
    }

    spin_unlock_bh(&g_common_ct_block_hash[hash_key].lock);

    return NULL;
}



static int atp_common_ct_block_need_add(const struct nf_conntrack_tuple* tuple)
{
    if (IPPROTO_TCP == tuple->dst.protonum)
    {
        return 1;
    }

    if (IPPROTO_UDP == tuple->dst.protonum)
    {
        return 1;
    }

    return 0;
}



void atp_common_ct_block_add(const struct nf_conntrack_tuple* tuple)
{
    u32 hash_key = 0;
    ATP_COMMON_CT_BLOCK_NODE* block_node = NULL;

    if (NULL == tuple)
    {
        return;
    }
    
    printk("%s: trace in\n", __FUNCTION__);

    /*当前只针对TCP/UDP加block规则*/
    if (0 == atp_common_ct_block_need_add(tuple))
    {
        COMMON_CTBLOCK_DEBUG("Ignore tuple [%p]\n", tuple);
        return;
    }

    /*如果已经加了block规则，则更新超时时间*/
    block_node = (ATP_COMMON_CT_BLOCK_NODE*)atp_common_ct_block_lookup(tuple);
    if (block_node)
    {
        COMMON_CTBLOCK_DEBUG("Tuple is exist, just update time\n");
        block_node->add_time = jiffies;
        return;
    }

    /*新分配block节点加入对应hash桶*/
    block_node = (ATP_COMMON_CT_BLOCK_NODE *)kmalloc(sizeof(ATP_COMMON_CT_BLOCK_NODE), GFP_ATOMIC);
    if (NULL == block_node)
    {
        return;
    }

    memset(block_node, 0, sizeof(ATP_COMMON_CT_BLOCK_NODE));

    memcpy(&block_node->tuple, tuple, sizeof(struct nf_conntrack_tuple));
    block_node->add_time = jiffies;

    hash_key = atp_common_ct_block_hash(tuple);

    spin_lock_bh(&g_common_ct_block_hash[hash_key].lock);
    list_add(&block_node->list, &g_common_ct_block_hash[hash_key].list);
    spin_unlock_bh(&g_common_ct_block_hash[hash_key].lock);

    return;
}



static void atp_common_ct_block_destroy(void)
{
    ATP_COMMON_CT_BLOCK_NODE* block_node = NULL;
    ATP_COMMON_CT_BLOCK_NODE* temp = NULL;
    int i = 0;

    /*遍历删除所有hash桶中的block规则*/
    for (i = 0; i < ATP_COMMON_CT_BLOCK_HASH_SIZE; i++)
    {
        spin_lock_bh(&g_common_ct_block_hash[i].lock);
        list_for_each_entry_safe(block_node, temp,
                                 &g_common_ct_block_hash[i].list, list)
        {
            list_del(&block_node->list);
            kfree(block_node);
        }
        spin_unlock_bh(&g_common_ct_block_hash[i].lock);
    }

    return;
}



static void atp_common_ct_block_timeout(unsigned long data)
{
    ATP_COMMON_CT_BLOCK_NODE* block_node = NULL;
    ATP_COMMON_CT_BLOCK_NODE* temp = NULL;
    u32 i = 0;

    data = data;

    /*检查所有block规则，删除其中的超时规则*/
    for (i = 0; i < ATP_COMMON_CT_BLOCK_HASH_SIZE; i++)
    {
        spin_lock_bh(&g_common_ct_block_hash[i].lock);
        list_for_each_entry_safe(block_node, temp,
                                 &g_common_ct_block_hash[i].list, list)
        {
            /*当前节点已超时*/
            if (time_after_eq(jiffies, block_node->add_time))
            {
                list_del(&block_node->list);
                kfree(block_node);
            }
        }
        spin_unlock_bh(&g_common_ct_block_hash[i].lock);
    }

    /*超时处理完成，意味着对应的一次立即生效block执行完毕*/
    atomic_dec(&g_need_block_check);

    return;
}



void atp_common_ct_block_reset_timer(void)
{
    mod_timer(&g_block_timer, (jiffies + ATP_COMMON_CT_BLOCK_TIME));

    COMMON_CTBLOCK_DEBUG("Mod timer at [%lu], timeout at [%lu]\n",
                         jiffies, (jiffies + ATP_COMMON_CT_BLOCK_TIME));

    return;
}



static unsigned int atp_common_ct_block_pkt(unsigned int hooknum,
        struct sk_buff* skb,
        const struct net_device* in,
        const struct net_device* out,
        int (*okfn)(struct sk_buff*))
{
    struct nf_conn* ct = NULL;
    struct nf_conntrack_tuple* orig_tuple = NULL;

    /*未执行需立即生效命令时，不需进行block处理*/
    if (0 == atomic_read(&g_need_block_check))
    {
        return NF_ACCEPT;
    }

    /*获取当前原始方向tuple*/
    ct = (struct nf_conn*)skb->nfct;
    if (NULL == ct)
    {
        return NF_ACCEPT;
    }

    orig_tuple = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;

    /*关联的连接正被block*/
    if (atp_common_ct_block_lookup(orig_tuple))
    {
        COMMON_CTBLOCK_DEBUG("DROP pkt for ct block\n");
        return NF_DROP;
    }

    return NF_ACCEPT;
}



static struct nf_hook_ops g_common_ct_block_ops[] __read_mostly =
{
    /*IPv4报文阻挡*/
    {
        .hook = atp_common_ct_block_pkt,
        .pf = PF_INET,
        .hooknum = NF_INET_LOCAL_OUT,
        .priority = NF_IP_PRI_FILTER,
    },
    {
        .hook = atp_common_ct_block_pkt,
        .pf = PF_INET,
        .hooknum = NF_INET_FORWARD,
        .priority = NF_IP_PRI_FILTER,
    },

    /*IPv6报文阻挡*/
    {
        .hook = atp_common_ct_block_pkt,
        .pf = PF_INET6,
        .hooknum = NF_INET_LOCAL_OUT,
        .priority = NF_IP_PRI_FILTER,
    },
    {
        .hook = atp_common_ct_block_pkt,
        .pf = PF_INET6,
        .hooknum = NF_INET_FORWARD,
        .priority = NF_IP_PRI_FILTER,
    },
};



static void atp_common_ct_block_print_rule(struct seq_file* m,
        ATP_COMMON_CT_BLOCK_NODE* block_node)
{
    /*规则加入时间*/
    seq_printf(m, "%lu ", block_node->add_time);

    /*IPv4和IPv6地址*/
    if (AF_INET == block_node->tuple.src.l3num)
    {
        seq_printf(m, NIPQUAD_FMT" ", NIPQUAD(block_node->tuple.src.u3.ip));
        seq_printf(m, NIPQUAD_FMT" ", NIPQUAD(block_node->tuple.dst.u3.ip));
    }
    else if (AF_INET6 == block_node->tuple.src.l3num)
    {
        seq_printf(m, NIP6_FMT" ", NIP6(block_node->tuple.src.u3.in6));
        seq_printf(m, NIP6_FMT" ", NIP6(block_node->tuple.dst.u3.in6));
    }

    /*协议*/
    seq_printf(m, "%d ", block_node->tuple.dst.protonum);

    /*源端口*/
    seq_printf(m, "%d ", ntohs(block_node->tuple.src.u.all));

    /*目的端口*/
    seq_printf(m, "%d\r\n", ntohs(block_node->tuple.dst.u.all));

    return;
}



static int atp_common_ct_block_show(struct seq_file* m, void* pData, unsigned int Size)
{
    ATP_COMMON_CT_BLOCK_NODE* block_node = NULL;
    u32 i = 0;

    pData = pData;
    Size = Size;

    seq_printf(m, "%s", "Time Source-IP  Dest-IP Protocol Source-Port Dest-Port\r\n");

    /*逐一输出block规则到seq文件*/
    for (i = 0; i < ATP_COMMON_CT_BLOCK_HASH_SIZE; i++)
    {
        spin_lock(&g_common_ct_block_hash[i].lock);
        list_for_each_entry(block_node,
                            &g_common_ct_block_hash[i].list, list)
        {
            atp_common_ct_block_print_rule(m, block_node);
        }
        spin_unlock(&g_common_ct_block_hash[i].lock);
    }

    return 0;
}



void atp_common_ct_block_start(void)
{
    printk("%s: trace in\n", __FUNCTION__);

    /*支持重复启动，每次启动的超时处理该值减1*/
    atomic_inc(&g_need_block_check);
    return;
}


static int __init atp_common_ct_block_init(void)
{
    hw_ssp_ker_obj_container* ct_block_obj = NULL;

    /*注册block规则检查处理hook*/
    if (0 > nf_register_hooks(g_common_ct_block_ops, ARRAY_SIZE(g_common_ct_block_ops)))
    {
        COMMON_CTCLEAN_DEBUG("nf_register_hooks failed\n");
        return -1;
    }

    /*block规则hash桶初始化*/
    atp_common_ct_block_hash_init();

    /*注册kobject接口，用于输出当前所有block规则*/
    ct_block_obj = hw_ssp_ker_obj_create(ATP_COMMON_CT_BLOCK_PROC);
    if (ct_block_obj)
    {
        ct_block_obj->ObjShow = atp_common_ct_block_show;
    }

    /*初始化block规则删除定时器*/
    init_timer(&g_block_timer);
    g_block_timer.expires = jiffies + ATP_COMMON_CT_BLOCK_TIME;
    g_block_timer.function = atp_common_ct_block_timeout;

    return 0;
}


static void __exit atp_common_ct_block_fini(void)
{
    del_timer_sync(&g_block_timer);

    atp_common_ct_block_destroy();

    nf_unregister_hooks(g_common_ct_block_ops, ARRAY_SIZE(g_common_ct_block_ops));
    return;
}


module_init(atp_common_ct_block_init);
module_exit(atp_common_ct_block_fini);

