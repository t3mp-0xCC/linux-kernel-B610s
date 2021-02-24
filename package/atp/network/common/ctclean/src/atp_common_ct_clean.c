/******************************************************************************
  *         ��Ȩ���� (C), 2001-2020, ��Ϊ�������޹�˾
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

    /*Start IPΪ0��˵�������IP��ַ*/
    if (0 == ulIpStart)
    {
        return 1;
    }

    /*��ʼIP�ͽ���IP��ͬ����������μ��*/
    if (ulIpStart == ulIpEnd)
    {
        /*����ƥ�䣬�ж�Ϊin range*/
        if ((ulIpStart & ulMask) == (ulCheckIp & ulMask))
        {
            return 1;
        }

        /*�������Σ������β�ƥ�䣬�ж�Ϊnot in range*/
        return 0;
    }

    ulCheckIpHost = ntohl(ulCheckIp);
    ulIpStartHost = ntohl(ulIpStart);
    ulIpEndHost = ntohl(ulIpEnd);

    /*��ʼIP�ͽ���IP��ͬ�����IP��Χ*/
    if (ulCheckIpHost >= ulIpStartHost && ulCheckIpHost <= ulIpEndHost)
    {
        return 1;
    }

    /*����IP��Χ�������ڷ�Χ�ڣ��ж�Ϊnot in range*/
    return 0;
}


static int atp_common_ctclean_ipv4_hit(const unsigned int ulIpAddrs[],
        unsigned int ulIpNum,
        unsigned int ulCheckIp)
{
    unsigned int ulIndex = 0;

    /*Ҫ����ipƥ��ip�б�����һ�������ж�ƥ��*/
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

    /*Start IPΪ0��˵�������IP��ַ*/
    if (0 == memcmp(&stIpStart, &stZeroIp6Addr, sizeof(struct in6_addr)))
    {
        return 1;
    }

    /*��ʼIP�ͽ���IP��ͬ����������μ��*/
    if (0 == memcmp(&stIpStart, &stIpEnd, sizeof(struct in6_addr)))
    {
        /*����ƥ�䣬�ж�Ϊin range*/
        if (0 == ipv6_masked_addr_cmp(&stIpStart,
                                      &stMask,
                                      &stCheckIp))
        {
            return 1;
        }

        /*�������Σ������β�ƥ�䣬�ж�Ϊnot in range*/
        return 0;
    }

    /*��ʼIP�ͽ���IP��ͬ�����IP��Χ*/
    if (ipv6_addr_cmp(&stCheckIp, &stIpStart) >= 0 && ipv6_addr_cmp(&stCheckIp, &stIpEnd) <= 0)
    {
        return 1;
    }

    /*����IP��Χ�������ڷ�Χ�ڣ��ж�Ϊnot in range*/
    return 0;
}


static int atp_common_ctclean_ipv6_hit(const struct in6_addr stIpAddrs[],
        unsigned int ulIpNum,
        struct in6_addr ulCheckIp)
{
    unsigned int ulIndex = 0;

    /*Ҫ����ipƥ��ip�б�����һ�������ж�ƥ��*/
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
    /*����עLAN/WAN������Ϊƥ��*/
    if (ATP_COMMON_CT_CLEAN_DIRECTION_ALL == pstRuleInfo->enDirection)
    {
        return 1;
    }

    /*WAN������*/
    if (ATP_COMMON_CT_CLEAN_DIRECTION_WAN == pstRuleInfo->enDirection)
    {
        /*ָ���˾���WAN�ӿڣ���Ϊƥ�䣬�ɺ����ӿڼ������ж�*/
        if (strlen(pstRuleInfo->acIntfName))
        {
            return 1;
        }

        /*WAN�������Ϊ!br+����˼��bridge�Ƿ�ƥ��*/
        if (0 == atp_common_ctclean_br_ifindex_in_range(pstRuleInfo, ct->ifindex))
        {
            return 1;
        }

        /*��ǰct�ӿ�Ϊbridge�ӿڣ����WAN����ƥ��*/
        return 0;
    }

    /*LAN�������Ϊbr+����˼��bridge�Ƿ�ƥ��*/
    if (atp_common_ctclean_br_ifindex_in_range(pstRuleInfo, ct->ifindex))
    {
        return 1;
    }

    /*��ǰct�ӿڲ�Ϊbridge�ӿڣ����LAN����ƥ��*/
    return 0;
}


static int atp_common_ctclean_interface_match(const ATP_COMMON_CT_CLEAN_INFO* pstRuleInfo,
                                              struct nf_conn* ct)
{
    /*δ���ýӿڣ���Ϊ���нӿ�match*/
    if (0 == pstRuleInfo->lIfIndex)
    {
        return 1;
    }

    /*Route Addʱ������ӿ�����·�ɽӿڱ����Ѿ�һ�£�����Ҫɾ����*/
    if (ATP_COMMON_CT_CLEAN_ROUTE & pstRuleInfo->enCtType)
    {
        if (pstRuleInfo->lIfIndex != ct->ifindex)
        {
            return 1;
        }

        /*�ӿ���ͬ������·�ɺ����߱��ӿڣ�������Ӳ���ɾ��*/
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
    /*ָ��bypass�������Ӹ��ٲ�����bypass�ϵģ�����*/
    if ((ATP_COMMON_CT_CLEAN_BYPASS & pstRuleInfo->enCtType)
        && (!(ct->bonding_mark & ATP_CONNTRACK_BYPASS_MARK)))
    {
        return 0;
    }

    /*ָ��bonding�������Ӹ��ٲ�����bonding�ϵģ�����*/
    if ((ATP_COMMON_CT_CLEAN_BONDING & pstRuleInfo->enCtType)
        && (!(ct->bonding_mark & ATP_CONNTRACK_TUNNEL_MARK)))
    {
        return 0;
    }

    /*��鵱ǰ������Ҫɾ�������dscp��Χ�Ƿ�ƥ��, dscpΪ0Ҳ�ǺϷ���ֵ*/
    if ((pstRuleInfo->ucDscp << DSCP_MARK_OFFSET)
        != ((pstRuleInfo->ucDscp << DSCP_MARK_OFFSET) & ct->bonding_mark))
    {
        return 0;
    }
#endif

    /*LAN/WAN������ɾ������ƥ�䣬���жϵ�ǰ���Ӳ���ɾ��*/
    if (0 == atp_common_ctclean_direction_match(pstRuleInfo, ct))
    {
        return 0;
    }

    /*��鵱ǰ������Ҫɾ������Ľӿ��Ƿ�ƥ��*/
    if (0 == atp_common_ctclean_interface_match(pstRuleInfo, ct))
    {
        return 0;
    }

    /*���Э�������Ƿ�ƥ��*/
    if (pstRuleInfo->ucProtocol
        && (pstRuleInfo->ucProtocol
            != ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum))
    {
        return 0;
    }

    /*IPv4��ַƥ����*/
    if (AF_INET == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num)
    {
        /*IPv4Դ��ַ��Ҫ��飬����ƥ��ɾ����������Ϊ��ǰct��ɾ��*/
        if (0 == atp_common_ctclean_ipv4_in_range(pstRuleInfo->ulSrcIpStart,
                pstRuleInfo->ulSrcIpEnd,
                pstRuleInfo->ulSrcMask,
                ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip))
        {
            return 0;
        }

        /*IPv4Ŀ�ĵ�ַ��Ҫ��飬����ƥ��ɾ����������Ϊ��ǰct��ɾ��*/
        if (0 == atp_common_ctclean_ipv4_in_range(pstRuleInfo->ulDstIpStart,
                pstRuleInfo->ulDstIpEnd,
                pstRuleInfo->ulDstMask,
                ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip))
        {
            return 0;
        }
    }

    /*IPv6��ַƥ����*/
    if (AF_INET6 == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num)
    {
        /*IPv6Դ��ַ��Ҫ��飬����ƥ��ɾ����������Ϊ��ǰct��ɾ��*/
        if (0 == atp_common_ctclean_ipv6_in_range(pstRuleInfo->stSrcIp6Start,
                pstRuleInfo->stSrcIp6End,
                pstRuleInfo->stSrcMask6,
                ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.in6))
        {
            return 0;
        }

        /*IPv6Ŀ�ĵ�ַ��Ҫ��飬����ƥ��ɾ����������Ϊ��ǰct��ɾ��*/
        if (0 == atp_common_ctclean_ipv6_in_range(pstRuleInfo->stDstIp6Start,
                pstRuleInfo->stDstIp6End,
                pstRuleInfo->stDstMask6,
                ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.in6))
        {
            return 0;
        }
    }

    /*��鵱ǰ������Ҫɾ�������Դ�˿ڷ�Χ�Ƿ�ƥ��*/
    if (pstRuleInfo->usSrcPortStart
        && (ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all) < ntohs(pstRuleInfo->usSrcPortStart)
            || ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all) > ntohs(pstRuleInfo->usSrcPortEnd)))
    {
        return 0;
    }

    /*��鵱ǰ������Ҫɾ�������Ŀ�Ķ˿ڷ�Χ�Ƿ�ƥ��*/
    if (pstRuleInfo->usDstPortStart
        && (ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all) < ntohs(pstRuleInfo->usDstPortStart)
            || ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all) > ntohs(pstRuleInfo->usDstPortEnd)))
    {
        return 0;
    }

    /*���������MAC��֮ǰ��ԴIP��Χ����ƥ�䣬�������ԴIP ���*/
    if (strlen(pstRuleInfo->acMac))
    {
        /*IPv4��ַ���*/
        if (AF_INET == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num)
        {
            /*��ǰct��ƥ��mac��Ӧ��ip��ַ*/
            if (0 == atp_common_ctclean_ipv4_hit(pstRuleInfo->ulIpv4Addrs,
                                                 ATP_COMMON_MAX_IP_PER_MAC_NUM,
                                                 ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip))
            {
                return 0;
            }
        }

        /*IPv6��ַ���*/
        if (AF_INET6 == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num)
        {
            /*��ǰct��ƥ��mac��Ӧ��ip��ַ*/
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

    /*ɾ������ƥ�䣬��û��Ҫ���б���������*/
    lmatch = atp_common_ctclean_match_single(&pstRuleInfos[0], ct);
    if (0 == lmatch)
    {
        return 0;
    }

    /*�ӵڶ�����ʼ��Ϊ��������*/
    for (ulIndex = 1; ulIndex < ulRuleNum; ulIndex++)
    {
        lmatch = atp_common_ctclean_match_single(&pstRuleInfos[ulIndex], ct);
        /*ɾ������ƥ���ǰ���£����ﱣ������һ��ƥ�䣬����Ϊ��ǰct����ɾ��*/
        if (lmatch)
        {
            return 0;
        }
    }

    /*��ǰCT���������ʱblock�б�,�������ӱ������ؽ�*/
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

    /*����bonding*/
    if (ATP_COMMON_CT_CLEAN_NONE & pstCtRules[0].enCtType)
    {
        COMMON_CTCLEAN_DEBUG("Not delete for NONE type.\n");
        return 0;
    }

    spin_lock_bh(&nf_conntrack_lock);

    /*��һ��ƥ�䣬�ҵ���Ҫɾ�������Ӳ����*/
    for (hash = 0; hash < net->ct.htable_size; hash++)
    {
        hlist_nulls_for_each_entry(h, n, &net->ct.hash[hash], hnnode)
        {
            if (NF_CT_DIRECTION(h) != IP_CT_DIR_ORIGINAL)
            {
                continue;
            }

            ct = nf_ct_tuplehash_to_ctrack(h);


            /*����ɾ�����򣬼�鵱ǰ����*/
            if (atp_common_ctclean_match(pstCtRules, ulRuleNum, ct))
            {
                /*��ǵ�ǰ������Ҫɾ��*/
                set_bit(IPS_COMMON_DELETE_BIT, &ct->status);
                need_del = 1;
                COMMON_CTCLEAN_DEBUG("mark clean ct [%p]\n", ct);
            }
        }
    }

    /*û��������Ҫɾ��*/
    if (0 == need_del)
    {
        spin_unlock_bh(&nf_conntrack_lock);
        COMMON_CTCLEAN_DEBUG("Not find conntrack to delete.\n");
        return 0;
    }

    /*�ڶ���ƥ�䣬�ҵ�alg���������Ӳ����ɾ��
         *alg�������������ӣ�IP/�˿ںܿ��������������Ӳ�һ�£�
         *�޷�ƥ�����������ӵ�ɾ�����򣬵�����������Ҫɾ��ʱ��
         *�佨������������ҲӦɾ��
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

            /*��alg�������ӣ��ڵ�һ���Ѿ��������*/
            if (NULL == ct->master)
            {
                continue;
            }

            /*���������δ����ǣ������������ж��Ƿ���Ҫɾ��*/
            if (!test_bit(IPS_COMMON_DELETE_BIT, &ct->status)
                && test_bit(IPS_COMMON_DELETE_BIT, &ct->master->status))
            {
                /*������Ҫ��ɾ��������������Ҳ���ɾ��*/
                set_bit(IPS_COMMON_DELETE_BIT, &ct->status);
                COMMON_CTCLEAN_DEBUG("mark clean ct [%p] for ALG\n", ct);

                /*�ߵ�����˵��ALG���������Ӳ�ƥ��ɾ������*/
                /*��������ƥ�䣬Ϊ�����������ӱ�ɾ����*/
                /*LAN�����ʹ��ԭ���Ķ˿�ֱ���ؽ��������ӣ�*/
                /*�����ﱻɾ�����������ӽ���block*/
                /*���������ӿ�����LAN�෢��Ҳ������WAN�෢��*/
                /*�����Ҫ����˫��block*/
                atp_common_ct_block_add(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
                atp_common_ct_block_add(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);

                need_reset_timer = 1;
            }
        }
    }
    spin_unlock_bh(&nf_conntrack_lock);

    /*ɾ����ǵ�����*/
    nf_ct_iterate_cleanup(net, atp_common_kill_marked, NULL);

    /*��ӹ�block�������ö�ʱ��*/
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

    /*ɾ������Ľӿ���ת��Ϊ�ӿ��������������Ӹ��ٱȽ�*/
    pstCtCleanInfo->lIfIndex = if_dev->ifindex;

    dev_put(if_dev);

    /*�����˽ӿڣ�����ݽӿ�ƥ�䣬����ACL����*/
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

    /*��Ϣ���ȱ����ɾ����Ϣ�ṹƥ��*/
    if (ulDataLen < sizeof(ATP_COMMON_CT_CLEAN_INFO))
    {
        COMMON_CTCLEAN_DEBUG("Rcv length [%d] not match size [%d]\n",
                             ulDataLen, sizeof(ATP_COMMON_CT_CLEAN_INFO));
        return -1;
    }

    /*�����·��˶��ٸ�ɾ������*/
    ulRuleNum = ulDataLen / sizeof(ATP_COMMON_CT_CLEAN_INFO);

    pstCtcleanInfos = (ATP_COMMON_CT_CLEAN_INFO *)kmalloc(sizeof(ATP_COMMON_CT_CLEAN_INFO) * ulRuleNum, GFP_KERNEL);
    if (NULL == pstCtcleanInfos)
    {
        return -1;
    }

    /*���յ�����Ϣ����ת��Ϊ����ɾ����Ϣ�ṹ*/
    memset(pstCtcleanInfos, 0, sizeof(ATP_COMMON_CT_CLEAN_INFO) * ulRuleNum);
    memcpy(pstCtcleanInfos, pvData, ulDataLen);

    for (ulIndex = 0; ulIndex < ulRuleNum; ulIndex++)
    {
        /*���б�Ҫ������ת�����Ա���й���ƥ��*/
        atp_common_ctclean_input_convert(&pstCtcleanInfos[ulIndex]);

        atp_common_ctclean_dumpInfo(&pstCtcleanInfos[ulIndex]);
    }

    lNeedBlock = atp_common_ctclean_need_block_reverse(pstCtcleanInfos[0].enCtType);
    if (lNeedBlock)
    {
        /*����block���*/
        atp_common_ct_block_start();
    }

    /*ִ�����Ӹ���ɾ������*/
    atp_common_ctclean_mark_clean(&init_net, pstCtcleanInfos, ulRuleNum);

    /*CT�����Ѽ�����ʱblock�б�,����time out��ʱ��*/
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

