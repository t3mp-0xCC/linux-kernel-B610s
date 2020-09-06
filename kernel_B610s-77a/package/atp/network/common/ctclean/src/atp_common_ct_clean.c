/******************************************************************************
  *         版权所有 (C), 2001-2020, 华为技术有限公司
*******************************************************************************/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/device.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_l3proto.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <linux/version.h>
#include <net/ipv6.h>

#include "kcmsmonitormsgtypes.h"
#include "commonctcleanmsgapi.h"
#include "atp_common_ct_block.h"

//#define ATP_CT_CLEAN_DEBUG

#define PPP_TRIGER_MARK    0x200000

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
#include <net/netfilter/nf_conntrack_zones.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
extern struct list_head* nf_conntrack_hash;
#else
extern struct net init_net;
#endif

extern void nf_conntrack_trigger_clean(int trigger_type);


static void atp_common_ctclean_dumpInfo(ATP_COMMON_CT_CLEAN_INFO* pstCleanInfo)
{
#ifdef ATP_CT_CLEAN_DEBUG
    int index = 0;

    printk("----------------Start dump conntrack clean info in kernel-----------------\n");

    printk("ulSrcIpStart: ["NIPQUAD_FMT"]\n", NIPQUAD(pstCleanInfo->ulSrcIpStart));
    printk("ulSrcIpEnd: ["NIPQUAD_FMT"]\n", NIPQUAD(pstCleanInfo->ulSrcIpEnd));
    printk("ulSrcMask: ["NIPQUAD_FMT"]\n", NIPQUAD(pstCleanInfo->ulSrcMask));

    printk("ulDstIpStart: ["NIPQUAD_FMT"]\n", NIPQUAD(pstCleanInfo->ulDstIpStart));
    printk("ulDstIpEnd: ["NIPQUAD_FMT"]\n", NIPQUAD(pstCleanInfo->ulDstIpEnd));
    printk("ulDstMask: ["NIPQUAD_FMT"]\n", NIPQUAD(pstCleanInfo->ulDstMask));

    printk("stSrcIp6Start: ["NIP6_FMT"]\n", NIP6(pstCleanInfo->stSrcIp6Start));
    printk("stSrcIp6End: ["NIP6_FMT"]\n", NIP6(pstCleanInfo->stSrcIp6End));
    printk("stSrcMask6: ["NIP6_FMT"]\n", NIP6(pstCleanInfo->stSrcMask6));

    printk("stDstIp6Start: ["NIP6_FMT"]\n", NIP6(pstCleanInfo->stDstIp6Start));
    printk("stDstIp6End: ["NIP6_FMT"]\n", NIP6(pstCleanInfo->stDstIp6End));
    printk("stDstMask6: ["NIP6_FMT"]\n", NIP6(pstCleanInfo->stDstMask6));

    printk("usSrcPortStart: [%d]\n", ntohs(pstCleanInfo->usSrcPortStart));
    printk("usSrcPortEnd: [%d]\n", ntohs(pstCleanInfo->usSrcPortEnd));

    printk("usDstPortStart: [%d]\n", ntohs(pstCleanInfo->usDstPortStart));
    printk("usDstPortEnd: [%d]\n", ntohs(pstCleanInfo->usDstPortEnd));

    printk("ucProtocol: [%d]\n", pstCleanInfo->ucProtocol);
    printk("ucDscp: [%d]\n", pstCleanInfo->ucDscp);
    printk("ucDirection: [%d]\n", pstCleanInfo->enDirection);

    printk("acIntfName: [%s]\n", pstCleanInfo->acIntfName);
    printk("lIfIndex: [%d]\n", pstCleanInfo->lIfIndex);
    printk("acMac: [%s]\n", pstCleanInfo->acMac);
    if (strlen(pstCleanInfo->acMac))
    {
        for (index = 0; index < ATP_COMMON_MAX_IP_PER_MAC_NUM; index++)
        {
            printk("ip address for mac ipv4[%d]:["NIPQUAD_FMT"] ipv6[%d]:["NIP6_FMT"]\n",
                index, NIPQUAD(pstCleanInfo->ulIpv4Addrs[index]),
                index, NIP6(pstCleanInfo->stIpv6Addrs[index]));
        }
    }

    printk("enDirection: [%d]\n", pstCleanInfo->enDirection);
    printk("lBrNum: [%d]\n", pstCleanInfo->lBrNum);

    for (index = 0; index < pstCleanInfo->lBrNum && index < ATP_COMMON_CT_CLEAN_BR_MAX; index++)
    {
        printk("bridge[%d] ifindex [%d]\n", index, pstCleanInfo->lBrIfIndex[index]);
    }

    printk("enCtType: [%d]\n", pstCleanInfo->enCtType);

    printk("----------------End dump conntrack clean info in kernel-----------------\n");
#endif
    return;
}


static int atp_common_ctclean_need_block_reverse(ATP_COMMON_CT_CLEAN_TYPE enCtType)
{
    if (ATP_COMMON_CT_CLEAN_ACL & enCtType)
    {
        return 1;
    }

    if (ATP_COMMON_CT_CLEAN_PTTRIGGER & enCtType)
    {
        return 1;
    }

    return 0;
}



static int atp_common_ctclean_ipv4_in_range(unsigned int ulIpStart,
        unsigned int ulIpEnd,
        unsigned int ulMask,
        unsigned int ulCheckIp)
{
    unsigned int ulCheckIpHost = 0;
    unsigned int ulIpStartHost = 0;
    unsigned int ulIpEndHost = 0;

    /*Start IP为0，说明不检查IP地址*/
    if (0 == ulIpStart)
    {
        return 1;
    }

    /*起始IP和结束IP相同，则进行网段检查*/
    if (ulIpStart == ulIpEnd)
    {
        /*网段匹配，判定为in range*/
        if ((ulIpStart & ulMask) == (ulCheckIp & ulMask))
        {
            return 1;
        }

        /*需检查网段，但网段不匹配，判定为not in range*/
        return 0;
    }

    ulCheckIpHost = ntohl(ulCheckIp);
    ulIpStartHost = ntohl(ulIpStart);
    ulIpEndHost = ntohl(ulIpEnd);

    /*起始IP和结束IP不同，检查IP范围*/
    if (ulCheckIpHost >= ulIpStartHost && ulCheckIpHost <= ulIpEndHost)
    {
        return 1;
    }

    /*需检查IP范围，但不在范围内，判定为not in range*/
    return 0;
}


static int atp_common_ctclean_ipv4_hit(const unsigned int ulIpAddrs[],
        unsigned int ulIpNum,
        unsigned int ulCheckIp)
{
    unsigned int ulIndex = 0;

    /*要检查的ip匹配ip列表中任一个，即判断匹配*/
    for (ulIndex = 0; ulIndex < ulIpNum; ulIndex++)
    {
        if (ulCheckIp == ulIpAddrs[ulIndex])
        {
            return 1;
        }
    }

    return 0;
}



static int atp_common_ctclean_ipv6_in_range(struct in6_addr stIpStart,
        struct in6_addr stIpEnd,
        struct in6_addr stMask,
        struct in6_addr stCheckIp)
{
    struct in6_addr stZeroIp6Addr;

    memset(&stZeroIp6Addr, 0, sizeof(stZeroIp6Addr));

    /*Start IP为0，说明不检查IP地址*/
    if (0 == memcmp(&stIpStart, &stZeroIp6Addr, sizeof(struct in6_addr)))
    {
        return 1;
    }

    /*起始IP和结束IP相同，则进行网段检查*/
    if (0 == memcmp(&stIpStart, &stIpEnd, sizeof(struct in6_addr)))
    {
        /*网段匹配，判定为in range*/
        if (0 == ipv6_masked_addr_cmp(&stIpStart,
                                      &stMask,
                                      &stCheckIp))
        {
            return 1;
        }

        /*需检查网段，但网段不匹配，判定为not in range*/
        return 0;
    }

    /*起始IP和结束IP不同，检查IP范围*/
    if (ipv6_addr_cmp(&stCheckIp, &stIpStart) >= 0 && ipv6_addr_cmp(&stCheckIp, &stIpEnd) <= 0)
    {
        return 1;
    }

    /*需检查IP范围，但不在范围内，判定为not in range*/
    return 0;
}


static int atp_common_ctclean_ipv6_hit(const struct in6_addr stIpAddrs[],
        unsigned int ulIpNum,
        struct in6_addr ulCheckIp)
{
    unsigned int ulIndex = 0;

    /*要检查的ip匹配ip列表中任一个，即判断匹配*/
    for (ulIndex = 0; ulIndex < ulIpNum; ulIndex++)
    {
        if (0 == memcmp(&ulCheckIp, &stIpAddrs[ulIndex], sizeof(struct in6_addr)))
        {
            return 1;
        }
    }

    return 0;
}


static int atp_common_ctclean_br_ifindex_in_range(const ATP_COMMON_CT_CLEAN_INFO* pstRuleInfo,
                                                  int ifindex)
{
    int index = 0;

    for (index = 0; index < pstRuleInfo->lBrNum && index < ATP_COMMON_CT_CLEAN_BR_MAX; index++)
    {
        if (ifindex == pstRuleInfo->lBrIfIndex[index])
        {
            return 1;
        }
    }

    return 0;
}

static int atp_common_ctclean_direction_match(const ATP_COMMON_CT_CLEAN_INFO* pstRuleInfo,
                                              struct nf_conn* ct)
{
    /*不关注LAN/WAN方向认为匹配*/
    if (ATP_COMMON_CT_CLEAN_DIRECTION_ALL == pstRuleInfo->enDirection)
    {
        return 1;
    }

    /*WAN方向处理*/
    if (ATP_COMMON_CT_CLEAN_DIRECTION_WAN == pstRuleInfo->enDirection)
    {
        /*指定了具体WAN接口，认为匹配，由后续接口检查规则判定*/
        if (strlen(pstRuleInfo->acIntfName))
        {
            return 1;
        }

        /*WAN方向规则为!br+，因此检查bridge是否不匹配*/
        if (0 == atp_common_ctclean_br_ifindex_in_range(pstRuleInfo, ct->ifindex))
        {
            return 1;
        }

        /*当前ct接口为bridge接口，因此WAN方向不匹配*/
        return 0;
    }

    /*LAN方向规则为br+，因此检查bridge是否匹配*/
    if (atp_common_ctclean_br_ifindex_in_range(pstRuleInfo, ct->ifindex))
    {
        return 1;
    }

    /*当前ct接口不为bridge接口，因此LAN方向不匹配*/
    return 0;
}


static int atp_common_ctclean_interface_match(const ATP_COMMON_CT_CLEAN_INFO* pstRuleInfo,
                                              struct nf_conn* ct)
{
    /*未设置接口，认为所有接口match*/
    if (0 == pstRuleInfo->lIfIndex)
    {
        return 1;
    }

    /*Route Add时，如果接口与新路由接口本身已经一致，则不需要删连接*/
    if (ATP_COMMON_CT_CLEAN_ROUTE & pstRuleInfo->enCtType)
    {
        if (pstRuleInfo->lIfIndex != ct->ifindex)
        {
            return 1;
        }

        /*接口相同，重新路由后仍走本接口，因此连接不需删除*/
        return 0;
    }

    if (pstRuleInfo->lIfIndex != ct->ifindex)
    {
        return 0;
    }

    return 1;
}



static int atp_common_ctclean_match_single(const ATP_COMMON_CT_CLEAN_INFO* pstRuleInfo,
                                    struct nf_conn* ct)
{
#ifdef CONFIG_ATP_HYBRID
    /*指定bypass，但连接跟踪不是在bypass上的，不清*/
    if ((ATP_COMMON_CT_CLEAN_BYPASS & pstRuleInfo->enCtType)
        && (!(ct->bonding_mark & ATP_CONNTRACK_BYPASS_MARK)))
    {
        return 0;
    }

    /*指定bonding，但连接跟踪不是在bonding上的，不清*/
    if ((ATP_COMMON_CT_CLEAN_BONDING & pstRuleInfo->enCtType)
        && (!(ct->bonding_mark & ATP_CONNTRACK_TUNNEL_MARK)))
    {
        return 0;
    }

    /*检查当前连接与要删除规则的dscp范围是否匹配, dscp为0也是合法的值*/
    if ((pstRuleInfo->ucDscp << DSCP_MARK_OFFSET)
        != ((pstRuleInfo->ucDscp << DSCP_MARK_OFFSET) & ct->bonding_mark))
    {
        return 0;
    }
#endif

    /*LAN/WAN方向与删除规则不匹配，则判断当前连接不需删除*/
    if (0 == atp_common_ctclean_direction_match(pstRuleInfo, ct))
    {
        return 0;
    }

    /*检查当前连接与要删除规则的接口是否匹配*/
    if (0 == atp_common_ctclean_interface_match(pstRuleInfo, ct))
    {
        return 0;
    }

    /*检查协议类型是否匹配*/
    if (pstRuleInfo->ucProtocol
        && (pstRuleInfo->ucProtocol
            != ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum))
    {
        return 0;
    }

    /*IPv4地址匹配检查*/
    if (AF_INET == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num)
    {
        /*IPv4源地址需要检查，但不匹配删除规则则认为当前ct不删除*/
        if (0 == atp_common_ctclean_ipv4_in_range(pstRuleInfo->ulSrcIpStart,
                pstRuleInfo->ulSrcIpEnd,
                pstRuleInfo->ulSrcMask,
                ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip))
        {
            return 0;
        }

        /*IPv4目的地址需要检查，但不匹配删除规则则认为当前ct不删除*/
        if (0 == atp_common_ctclean_ipv4_in_range(pstRuleInfo->ulDstIpStart,
                pstRuleInfo->ulDstIpEnd,
                pstRuleInfo->ulDstMask,
                ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip))
        {
            return 0;
        }
    }

    /*IPv6地址匹配检查*/
    if (AF_INET6 == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num)
    {
        /*IPv6源地址需要检查，但不匹配删除规则则认为当前ct不删除*/
        if (0 == atp_common_ctclean_ipv6_in_range(pstRuleInfo->stSrcIp6Start,
                pstRuleInfo->stSrcIp6End,
                pstRuleInfo->stSrcMask6,
                ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.in6))
        {
            return 0;
        }

        /*IPv6目的地址需要检查，但不匹配删除规则则认为当前ct不删除*/
        if (0 == atp_common_ctclean_ipv6_in_range(pstRuleInfo->stDstIp6Start,
                pstRuleInfo->stDstIp6End,
                pstRuleInfo->stDstMask6,
                ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.in6))
        {
            return 0;
        }
    }

    /*检查当前连接与要删除规则的源端口范围是否匹配*/
    if (pstRuleInfo->usSrcPortStart
        && (ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all) < ntohs(pstRuleInfo->usSrcPortStart)
            || ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all) > ntohs(pstRuleInfo->usSrcPortEnd)))
    {
        return 0;
    }

    /*检查当前连接与要删除规则的目的端口范围是否匹配*/
    if (pstRuleInfo->usDstPortStart
        && (ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all) < ntohs(pstRuleInfo->usDstPortStart)
            || ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all) > ntohs(pstRuleInfo->usDstPortEnd)))
    {
        return 0;
    }

    /*如果有设置MAC，之前的源IP范围不会匹配，这里进行源IP 检查*/
    if (strlen(pstRuleInfo->acMac))
    {
        /*IPv4地址检查*/
        if (AF_INET == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num)
        {
            /*当前ct不匹配mac对应的ip地址*/
            if (0 == atp_common_ctclean_ipv4_hit(pstRuleInfo->ulIpv4Addrs,
                                                 ATP_COMMON_MAX_IP_PER_MAC_NUM,
                                                 ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip))
            {
                return 0;
            }
        }

        /*IPv6地址检查*/
        if (AF_INET6 == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num)
        {
            /*当前ct不匹配mac对应的ip地址*/
            if (0 == atp_common_ctclean_ipv6_hit(pstRuleInfo->stIpv6Addrs,
                                                 ATP_COMMON_MAX_IP_PER_MAC_NUM,
                                                 ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.in6))
            {
                return 0;
            }
        }
    }

    return 1;
}



static int atp_common_ctclean_match(const ATP_COMMON_CT_CLEAN_INFO* pstRuleInfos,
                                    unsigned int ulRuleNum,
                                    struct nf_conn* ct)
{
    unsigned int ulIndex = 0;
    int lmatch = 0;

    /*删除规则不匹配，则没必要进行保留规则检查*/
    lmatch = atp_common_ctclean_match_single(&pstRuleInfos[0], ct);
    if (0 == lmatch)
    {
        return 0;
    }

    /*从第二条开始，为保留规则*/
    for (ulIndex = 1; ulIndex < ulRuleNum; ulIndex++)
    {
        lmatch = atp_common_ctclean_match_single(&pstRuleInfos[ulIndex], ct);
        /*删除规则匹配的前提下，这里保留规则一旦匹配，即认为当前ct不需删除*/
        if (lmatch)
        {
            return 0;
        }
    }

    /*当前CT反向加入临时block列表,避免连接被反向重建*/
    if (atp_common_ctclean_need_block_reverse(pstRuleInfos->enCtType))
    {
        atp_common_ct_block_add(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);
    }

    return 1;

}

static int atp_common_kill_marked(struct nf_conn *i, void *data)
{
    if (test_bit(IPS_COMMON_DELETE_BIT, &i->status))
    {
        return 1;
    }

    return 0;
}



static int atp_common_ctclean_mark_clean(struct net* net,
                                         const ATP_COMMON_CT_CLEAN_INFO* pstCtRules,
                                         unsigned int ulRuleNum)
{
    struct nf_conntrack_tuple_hash* h = NULL;
    struct hlist_nulls_node* n = NULL;
    struct nf_conn* ct = NULL;
    unsigned int hash = 0;
    int need_del = 0;
    int need_reset_timer = 0;

    printk("%s: trace in\n", __FUNCTION__);

    /*兼容bonding*/
    if (ATP_COMMON_CT_CLEAN_NONE & pstCtRules[0].enCtType)
    {
        COMMON_CTCLEAN_DEBUG("Not delete for NONE type.\n");
        return 0;
    }

    spin_lock_bh(&nf_conntrack_lock);

    /*第一轮匹配，找到需要删除的连接并标记*/
    for (hash = 0; hash < net->ct.htable_size; hash++)
    {
        hlist_nulls_for_each_entry(h, n, &net->ct.hash[hash], hnnode)
        {
            if (NF_CT_DIRECTION(h) != IP_CT_DIR_ORIGINAL)
            {
                continue;
            }

            ct = nf_ct_tuplehash_to_ctrack(h);


            /*根据删除规则，检查当前连接*/
            if (atp_common_ctclean_match(pstCtRules, ulRuleNum, ct))
            {
                /*标记当前连接需要删除*/
                set_bit(IPS_COMMON_DELETE_BIT, &ct->status);
                need_del = 1;
                COMMON_CTCLEAN_DEBUG("mark clean ct [%p]\n", ct);
            }
        }
    }

    /*没有连接需要删除*/
    if (0 == need_del)
    {
        spin_unlock_bh(&nf_conntrack_lock);
        COMMON_CTCLEAN_DEBUG("Not find conntrack to delete.\n");
        return 0;
    }

    /*第二轮匹配，找到alg建立的连接并标记删除
         *alg建立的数据连接，IP/端口很可能与主控制连接不一致，
         *无法匹配主控制连接的删除规则，但主控连接需要删除时，
         *其建立的数据连接也应删除
         */
    for (hash = 0; hash < net->ct.htable_size; hash++)
    {
        hlist_nulls_for_each_entry(h, n, &net->ct.hash[hash], hnnode)
        {
            if (NF_CT_DIRECTION(h) != IP_CT_DIR_ORIGINAL)
            {
                continue;
            }

            ct = nf_ct_tuplehash_to_ctrack(h);

            /*非alg数据连接，在第一轮已经处理过了*/
            if (NULL == ct->master)
            {
                continue;
            }

            /*如果子连接未被标记，根据主连接判断是否需要删除*/
            if (!test_bit(IPS_COMMON_DELETE_BIT, &ct->status)
                && test_bit(IPS_COMMON_DELETE_BIT, &ct->master->status))
            {
                /*主连接要被删除，子连接这里也标记删除*/
                set_bit(IPS_COMMON_DELETE_BIT, &ct->status);
                COMMON_CTCLEAN_DEBUG("mark clean ct [%p] for ALG\n", ct);

                /*走到这里说明ALG的数据连接不匹配删除规则*/
                /*但父连接匹配，为避免数据连接被删除后，*/
                /*LAN侧继续使用原来的端口直接重建数据连接，*/
                /*对这里被删除的数据连接进行block*/
                /*因数据连接可能是LAN侧发起，也可能是WAN侧发起，*/
                /*因此需要进行双向block*/
                atp_common_ct_block_add(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
                atp_common_ct_block_add(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);

                need_reset_timer = 1;
            }
        }
    }
    spin_unlock_bh(&nf_conntrack_lock);

    /*删除标记的连接*/
    nf_ct_iterate_cleanup(net, atp_common_kill_marked, NULL);

    /*添加过block规则，重置定时器*/
    if (need_reset_timer)
    {
        atp_common_ct_block_reset_timer();
    }

    return 0;
}



static void atp_common_ctclean_input_convert(ATP_COMMON_CT_CLEAN_INFO* pstCtCleanInfo)
{
    struct net_device* if_dev = NULL;

    if (0 == strlen(pstCtCleanInfo->acIntfName))
    {
        return;
    }

    if_dev = dev_get_by_name(&init_net, pstCtCleanInfo->acIntfName);
    if (NULL == if_dev)
    {
        COMMON_CTCLEAN_DEBUG("Not find device for [%s]\n", pstCtCleanInfo->acIntfName);
        return;
    }

    /*删除规则的接口名转换为接口索引，便于连接跟踪比较*/
    pstCtCleanInfo->lIfIndex = if_dev->ifindex;

    dev_put(if_dev);

    /*输入了接口，则根据接口匹配，忽略ACL方向*/
    pstCtCleanInfo->enDirection = ATP_COMMON_CT_CLEAN_DIRECTION_ALL;

    return;
}



static int atp_common_ctclean_msg_process(unsigned short usModuleId, void* pvData, unsigned int ulDataLen)
{
    unsigned int ulRuleNum = 0;
    unsigned int ulIndex = 0;
    ATP_COMMON_CT_CLEAN_INFO *pstCtcleanInfos = NULL;
    int lNeedBlock = 0;

    printk("get in atp_common_ctclean_msg_process, datalen [%d]\n", ulDataLen);

    if (NULL == pvData)
    {
        return -1;
    }

    /*消息长度必须和删除信息结构匹配*/
    if (ulDataLen < sizeof(ATP_COMMON_CT_CLEAN_INFO))
    {
        COMMON_CTCLEAN_DEBUG("Rcv length [%d] not match size [%d]\n",
                             ulDataLen, sizeof(ATP_COMMON_CT_CLEAN_INFO));
        return -1;
    }

    /*计算下发了多少个删除规则*/
    ulRuleNum = ulDataLen / sizeof(ATP_COMMON_CT_CLEAN_INFO);

    pstCtcleanInfos = (ATP_COMMON_CT_CLEAN_INFO *)kmalloc(sizeof(ATP_COMMON_CT_CLEAN_INFO) * ulRuleNum, GFP_KERNEL);
    if (NULL == pstCtcleanInfos)
    {
        return -1;
    }

    /*接收到的消息内容转换为连接删除信息结构*/
    memset(pstCtcleanInfos, 0, sizeof(ATP_COMMON_CT_CLEAN_INFO) * ulRuleNum);
    memcpy(pstCtcleanInfos, pvData, ulDataLen);

    for (ulIndex = 0; ulIndex < ulRuleNum; ulIndex++)
    {
        /*进行必要的输入转换，以便进行规则匹配*/
        atp_common_ctclean_input_convert(&pstCtcleanInfos[ulIndex]);

        atp_common_ctclean_dumpInfo(&pstCtcleanInfos[ulIndex]);
    }

    lNeedBlock = atp_common_ctclean_need_block_reverse(pstCtcleanInfos[0].enCtType);
    if (lNeedBlock)
    {
        /*启用block检查*/
        atp_common_ct_block_start();
    }

    /*执行连接跟踪删除处理*/
    atp_common_ctclean_mark_clean(&init_net, pstCtcleanInfos, ulRuleNum);

    /*CT反向已加入临时block列表,重置time out定时器*/
    if (lNeedBlock)
    {
        atp_common_ct_block_reset_timer();
    }

    kfree(pstCtcleanInfos);

    return 0;
}


static int __init atp_common_ctclean_init(void)
{
    ATP_Netlink_Register(ATP_KCMSMAIN_NETLINK_CT_CLEAN, atp_common_ctclean_msg_process);
    printk("ctclean reg netlink [%d]\n", ATP_KCMSMAIN_NETLINK_CT_CLEAN);
    return 0;
}


static void __exit atp_common_ctclean_fini(void)
{
    ATP_Netlink_Unregister(ATP_KCMSMAIN_NETLINK_CT_CLEAN);
    return;
}


module_init(atp_common_ctclean_init);
module_exit(atp_common_ctclean_fini);

