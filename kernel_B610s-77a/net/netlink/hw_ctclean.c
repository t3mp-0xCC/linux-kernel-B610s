

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <net/sock.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/string.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_l3proto.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <linux/version.h>

#include "kcmsmonitormsgtypes.h"
#include "hw_ctclean.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
#include <net/netlink.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
extern struct list_head *nf_conntrack_hash;
#else
extern struct net init_net;
#endif

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
	

extern void clean_from_lists(struct nf_conn *ct);

extern int syswatch_nl_send(unsigned int type, unsigned char *buf, unsigned int len);

extern struct sock *syswatch_nl_sock;

static struct nf_conn *match_ct_list[MAX_RULE_NUM]; 


static void hw_ctclean_del_ct(unsigned long ul_conntrack)
{
    struct nf_conn *ct = (void *)ul_conntrack;
    struct nf_conn_help *help = nfct_help(ct);

    if (help && help->helper && help->helper->destroy)
    {
        help->helper->destroy(ct);
    }

    spin_lock_bh(&nf_conntrack_lock);
    /* Inside lock so preempt is disabled on module removal path.
     * Otherwise we can get spurious warnings. */
    clean_from_lists(ct);
    spin_unlock_bh(&nf_conntrack_lock);
    nf_ct_put(ct);

    return;
}


/* delete conntrack, refers to napt module */
static void hw_ctclean_del_ct_by_tuple(struct nf_conntrack_tuple *tuple)
{
    struct nf_conntrack_tuple_hash *h = NULL;
    struct nf_conn *ct = NULL;

    if (NULL == tuple)
    {
        return;
    }

    /* look for tuple match */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
    h = nf_conntrack_find_get(tuple, NULL);
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
    h = nf_conntrack_find_get(&init_net, NF_CT_DEFAULT_ZONE, tuple);
#else
    h = nf_conntrack_find_get(&init_net, tuple);
#endif
    
    if (!h) 
    {
        return;
    }

    ct = nf_ct_tuplehash_to_ctrack(h);
    if (!ct)
    {
        return;
    }

    if (del_timer(&ct->timeout)) 
    {
        hw_ctclean_del_ct((unsigned long)ct);
    }
    nf_ct_put(ct);

    return;
}


/******************************************************************************
  函数名称： hw_ctclean_do_delete
  功能描述： 根据五元组删除连接跟踪
  调用函数： hw_ctclean_process
  被调函数：
  输入参数：
  输出参数：
  返 回 值：
  其它说明：
******************************************************************************/
static void hw_ctclean_do_delete(void)
{
    int index = 0;

    while (index < MAX_RULE_NUM && match_ct_list[index] != NULL)
    {
        hw_ctclean_del_ct_by_tuple(&match_ct_list[index]->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
        hw_ctclean_del_ct_by_tuple(&match_ct_list[index]->tuplehash[IP_CT_DIR_REPLY].tuple);
        index++;
    }

    return;
}


/******************************************************************************
  函数名称： hw_ctclean_check_ct
  功能描述： 用于判断是否是需要的连接跟踪
  调用函数：find_need_ct
  被调函数：
  输入参数：ATP_CT_CLEAN_INFO* pstRuleInfo 用户态传入数据，
           struct nf_conn *ct 待比较的连接跟踪,
		   int *rule_num 循环索引值
  输出参数：
  返 回 值：是需要的连接跟踪，返回true，否则返回false
  其它说明：
******************************************************************************/
static int hw_ctclean_check_ct(ATP_CT_CLEAN_INFO* pstRuleInfo, struct nf_conn *ct, int *rule_num)
{
    int lanmask = 0;
    int index = 0;

    if (NULL == pstRuleInfo)
    {
        return false;
    }

    if (NULL == ct)
    {
        return false;
    }

    if (NULL  == rule_num)
    {
        return false;
    }
    
    lanmask = pstRuleInfo->lanmask;
    
    /*协议类型为UDP 或者协议类型为TCP且TCP状态为ESTABLISHED */
    /*备注: tcp_conntrack_names变量中ESTABLISHED状态索引号为3，防止客户端反复连接出现很多无效连接跟踪对查找进行干扰*/
    if (UDP_PROTOCOL != ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum
        && TCP_PROTOCOL != ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum)
    {
        return false;
    }

    if (TCP_PROTOCOL == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum && ESTABLISHED != ct->proto.tcp.state)
    {
        return false;
    }
    
    /*发起的源地址必须在WAN侧*/
    if ((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip & lanmask) == (pstRuleInfo->ruleinfo[0].lanhostIP & lanmask))
    {
        return false;
    }

    /*发起的源地址不能是wan接口IP */
    if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip == pstRuleInfo->ruleinfo[0].interfaceIP)
    {
        return false;
    }

    /*发起的目的地址必须为网关接口IP */
    if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip != pstRuleInfo->ruleinfo[0].interfaceIP)
    {
        return false;
    }

    /*回复的源地址必须为LAN侧主机IP */
    if (ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip != pstRuleInfo->ruleinfo[0].lanhostIP)
    {
        return false;
    }

    /*回复的目的地址必须在WAN侧*/
    if ((ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip & lanmask) == (pstRuleInfo->ruleinfo[0].lanhostIP & lanmask))
    {
        return false;
    }

    /*回复的目的地址不能是wan接口IP*/
    if (ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip == pstRuleInfo->ruleinfo[0].interfaceIP)
    {
        return false;
    }

    if (BREAK_PORTMAPPING != pstRuleInfo->action)
    {
        return true;
    }
                            
    /*Portmapping模式时需判断端口是否符合要求，协议和端口是否匹配*/
    index = *rule_num;

    /*PortMapping要求协议类型必须一致*/
    if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum != pstRuleInfo->ruleinfo[index].protocol)
    {
        return false;
    }

    /*tcp报文的目的端口要在PortMapping规则的start--end范围内*/
    if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.tcp.port >= htons(pstRuleInfo->ruleinfo[index].startport)
            && ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.tcp.port <= htons(pstRuleInfo->ruleinfo[index].endport))
    {
        return true;
    }

    /*udp报文的目的端口要在PortMapping规则的start--end范围内*/
    if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.udp.port >= htons(pstRuleInfo->ruleinfo[index].startport)
            && ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.udp.port <= htons(pstRuleInfo->ruleinfo[index].endport))
    {
        return true;
    }

    /*ct与PortMapping规则的端口范围不一致*/
    return false;
}

/******************************************************************************
  函数名称： hw_ctclean_add_ct_to_del_list
  功能描述： 将查找到的连接跟踪加入待删除列表。
  调用函数：find_need_ct
  被调函数：
  输入参数：int *ct_num 当前的连接跟踪总条数, struct nf_conn *ct 连接跟踪
  输出参数：int *ct_num 加入后当前的连接跟踪总条数
  返 回 值：
  其它说明：
******************************************************************************/
static void hw_ctclean_add_ct_to_del_list(int *ct_num, struct nf_conn *ct)
{
    int ct_search_index = 0;
    int repeat_flag = 0; /*用于标记是否已存储相同的连接跟踪记录*/

    if (NULL == ct_num)
    {
        return;
    }

    if (NULL == ct)
    {
        return;
    }

    if (MAX_RULE_NUM <= *ct_num)
    {
        DEBUGPC("ct num hit max [%d]\n", MAX_RULE_NUM);
        return;
    }

    while (ct_search_index < MAX_RULE_NUM && NULL != match_ct_list[ct_search_index])
    {
        if (match_ct_list[ct_search_index] == ct)
        {
            /*已记录此连接跟踪*/
            repeat_flag = 1;
            break;
        }
        ct_search_index++;
    }

    /*如果之前并没添加此连接跟踪，则添加，否则跳过添加*/
    if (0 == repeat_flag) 
    {
        match_ct_list[*ct_num] = ct;
        DEBUGPC("find conntrack[%d]=%p\n", *ct_num, ct);
        (*ct_num)++;
    }
    
    return;
}


/******************************************************************************
  函数名称： hw_ctclean_find_ct
  功能描述： 用于查找符合条件的连接跟踪并以此构造数据返回
  调用函数：
  被调函数：
  输入参数：ATP_CT_CLEAN_INFO* pstCtRule 数据包含规则信息
  输出参数：ATP_CT_CLEAN_INFO* pstCtRule 数据包含匹配的连接跟踪信息
  返 回 值：
  其它说明：
******************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
static void hw_ctclean_find_ct(ATP_CT_CLEAN_INFO* pstCtRule)
#else
static void hw_ctclean_find_ct(struct net *net, ATP_CT_CLEAN_INFO* pstCtRule)
#endif
{
    struct nf_conntrack_tuple_hash *h = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    struct hlist_nulls_node *n = NULL;
#endif
    struct nf_conn *ct = NULL;
    unsigned int hash = 0;
    
    int rule_num = 0; /*用于查询的索引变量*/
    int ct_num = 0; /*用于统计记录的连接跟踪条数*/
    int mode = 0;
    int LanHostIP = 0;
    int main_ct_num = 0; /*查找到的主连接跟踪条数*/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    if (NULL == net)
    {
        return;
    }
#endif

    if (NULL == pstCtRule)
    {
        return;
    }

    memset(match_ct_list, 0, sizeof(match_ct_list));
    mode = pstCtRule->action;
    LanHostIP = pstCtRule->ruleinfo[0].lanhostIP;

    spin_lock_bh(&nf_conntrack_lock);

    /*查找主连接跟踪记录*/
    for (hash = 0; hash < net->ct.htable_size; hash++)
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
		list_for_each_entry(h, &nf_conntrack_hash[hash], list)
#else
        hlist_nulls_for_each_entry(h, n, &net->ct.hash[hash], hnnode) 
#endif
        {
            if (NF_CT_DIRECTION(h) != IP_CT_DIR_ORIGINAL)
            {
                continue;
            }

            ct = nf_ct_tuplehash_to_ctrack(h);
            rule_num = 0;

            /*DMZ模式下匹配规则:*/
            /*连接跟踪中发起的源地址为公网IP（非LAN网段，非网关WAN IP）,发起的目的地址为DMZ规则指定的WAN接口的IP；*/
            /*连接跟踪中回复的源地址为DMZ规则指定的LAN侧主机IP，回复的目的地址为公网IP(非LAN网段，非网关WAN IP）。*/

            /*Port mapping 下匹配规则*/
            /*协议类型和Port mapping规则的协议类型一致；*/
            /*连接跟踪中发起的源地址为公网IP（非LAN网段，非网关WAN IP）,发起的目的地址为Port mapping规则指定的WAN接口的IP，发起的目的端口在Port mapping规则具体条目的开始端口和结束端口范围之内；*/
            /*连接跟踪中回复的源地址为Port mapping规则指定的LAN侧主机IP，回复的目的地址为公网IP(非LAN网段，非网关WAN IP），回复的源端口在Port mapping规则具体条目的开始端口和结束端口范围之内。*/

            /*DMZ模式下查找满足需要的主连接*/
            if (BREAK_DMZ == mode && true == hw_ctclean_check_ct(pstCtRule, ct, &rule_num))
            {
                hw_ctclean_add_ct_to_del_list(&ct_num, ct);
            }

            /*Portmapping模式下查找满足需要的主连接*/
            while (ct_num < MAX_RULE_NUM && rule_num < pstCtRule->entrynum)
            {   
                if (true == hw_ctclean_check_ct(pstCtRule, ct, &rule_num))
                {
                    hw_ctclean_add_ct_to_del_list(&ct_num, ct);
                }      
                rule_num++;
            }
        }
    }

    main_ct_num = ct_num;

    /*根据主连接跟踪查找所有的预期连接跟踪*/
    for (hash = 0; hash < net->ct.htable_size; hash++)
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
		list_for_each_entry(h, &nf_conntrack_hash[hash], list)
#else
        hlist_nulls_for_each_entry(h, n, &net->ct.hash[hash], hnnode) 
#endif    
        {
            if (NF_CT_DIRECTION(h) != IP_CT_DIR_ORIGINAL)
            {
                continue;
            }

            ct = nf_ct_tuplehash_to_ctrack(h);
            rule_num = 0;
            
            while (ct_num < MAX_RULE_NUM && rule_num < main_ct_num)
            {
                if (NULL != ct->master && ct->master == match_ct_list[rule_num])
                {
                    hw_ctclean_add_ct_to_del_list(&ct_num, ct);
                }
                rule_num++;
            }
        }
    }

    /*根据查得的结果构造数据*/
    /*与从用户态传入内核态的情不同，从内核态传回用户态时五元组的意义有改变*/
    /*查到的连接跟踪有如下两种，为实现从服务端封堵，传回的值如下*/
    
    /*模式1: ipv4     2 tcp      6 431997 ESTABLISHED src=188.28.8.150 dst=189.28.99.48 sport=37113 dport=21 src=192.168.1.3 dst=188.28.8.150 sport=21 dport=37113*/
    /*对于此种，传回回复的目的地址和目的端口dst=188.28.8.150 dport=37113*/
    
    /*模式2: ipv4     2 tcp      6 431999 ESTABLISHED src=192.168.1.3 dst=188.28.8.150 sport=20 dport=37115 src=188.28.8.150 dst=189.28.99.48 sport=37115 dport=20*/
    /*对于此种，传回发起的目的地址和目的端口dst=188.28.8.150  dport=37115*/
    /*protocol存储协议号，interfaceIP存储返回的IP，startport存储返回的端口*/
    
    DEBUGPC("Total ct is %d mode=%d\n", ct_num, mode);
    memset(pstCtRule, 0, sizeof(ATP_CT_CLEAN_INFO));

    /*将传入的模式写回，作为校验使用*/
    pstCtRule->action = mode;

    /*将查到的连接跟踪总条目数写回*/
    pstCtRule->entrynum = ct_num; 

    rule_num = 0;
    while (rule_num < ct_num)
    {
        ct = match_ct_list[rule_num];
        pstCtRule->ruleinfo[rule_num].protocol = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
        if (ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip == LanHostIP)   //模式1
        {
            pstCtRule->ruleinfo[rule_num].interfaceIP = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip;
            pstCtRule->ruleinfo[rule_num].lanhostIP = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip;
            if (TCP_PROTOCOL == pstCtRule->ruleinfo[rule_num].protocol)
            {
                pstCtRule->ruleinfo[rule_num].startport = ntohs(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.tcp.port);
                pstCtRule->ruleinfo[rule_num].endport = ntohs(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.tcp.port);
            }
            else if (UDP_PROTOCOL == pstCtRule->ruleinfo[rule_num].protocol)
            {
                pstCtRule->ruleinfo[rule_num].startport = ntohs(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.udp.port);
            }
        }

        if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip == LanHostIP)   //模式2  
        {
            pstCtRule->ruleinfo[rule_num].interfaceIP = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip;
            if (TCP_PROTOCOL == pstCtRule->ruleinfo[rule_num].protocol)
            {
                pstCtRule->ruleinfo[rule_num].startport = ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.tcp.port);
            }
            else if (UDP_PROTOCOL == pstCtRule->ruleinfo[rule_num].protocol)
            {
                pstCtRule->ruleinfo[rule_num].startport = ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.udp.port);
            }
        }
        DEBUGPC("needed_ct[%d]=%p protocol=%d interfaceip=%u.%u.%u.%u sport=%d\n",
            rule_num, match_ct_list[rule_num], pstCtRule->ruleinfo[rule_num].protocol,
            NIPQUAD(pstCtRule->ruleinfo[rule_num].interfaceIP), ntohs(pstCtRule->ruleinfo[rule_num].startport));
        rule_num++;
    }

    spin_unlock_bh(&nf_conntrack_lock);
    return;
}


/******************************************************************************
  函数名称： hw_ctclean_recv_msg
  功能描述： 校验报文，合法将接收消息内容拷入pstRuleInfo
  调用函数：
  被调函数：
  输入参数：struct sock *sk, u32 *pid, ATP_CT_CLEAN_INFO *pstRuleInfo
  输出参数：ATP_CT_CLEAN_INFO *pstRuleInfo
  返 回 值：0表示正确，-1表示错误
  其它说明：
******************************************************************************/
static int hw_ctclean_recv_msg(struct sk_buff *skb, u32 *pid, ATP_CT_CLEAN_INFO *pstRuleInfo)
{
    struct nlmsghdr *nlhdr = NULL;

    if (NULL == skb)
    {
        return -1;
    }

    if (NULL == pid)
    {
        return -1;
    }

    if (NULL == pstRuleInfo)
    {
        return -1;
    }

    nlhdr = (struct nlmsghdr *)skb->data;

    if (skb->len < NLMSG_SPACE(0))
    {
        kfree_skb(skb);
        return -1;
    }

    if (skb->len < nlhdr->nlmsg_len)
    {
        kfree_skb(skb);
        return -1;
    }

    if (nlhdr->nlmsg_len < NLMSG_LENGTH(sizeof(ATP_CT_CLEAN_INFO)))
    {
        kfree_skb(skb);
        return -1;
    }
    
    memcpy(pstRuleInfo, NLMSG_DATA(nlhdr), sizeof(ATP_CT_CLEAN_INFO));

    if (BREAK_ERROR == pstRuleInfo->action)
    {
        kfree_skb(skb);
        return -1;
    }
    
    if ((0 == pstRuleInfo->lanmask) && ((BREAK_DMZ == pstRuleInfo->action) || (BREAK_PORTMAPPING == pstRuleInfo->action)))
    {
        kfree_skb(skb);
        return -1;
    }
    
    *pid = nlhdr->nlmsg_pid;

    return 0;
}


/******************************************************************************
  函数名称： hw_ctclean_send_msg
  功能描述： 发送消息,消息内容为查找到的需要block的连接跟踪信息
  调用函数：
  被调函数：
  输入参数：u32 *pid, ATP_CT_CLEAN_INFO *pstCtInfo
  输出参数：
  返 回 值：0表示正确，-1表示错误
  其它说明：
******************************************************************************/
static void hw_ctclean_send_msg(u32 pid, ATP_CT_CLEAN_INFO *pstCtInfo)
{
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlhdr = NULL;

    skb = alloc_skb(NLMSG_SPACE(MAX_MSGSIZE), GFP_ATOMIC);
    if(!skb)
    {
        return;
    }
    
    /* 初始化消息头并发送 */
    nlhdr = nlmsg_put(skb, 0, 0, 0, MAX_MSGSIZE, 0);
    if (NULL == nlhdr)
    {
        kfree_skb(skb);
        return;
    }
    
    //NETLINK_CB(skb).pid = KERNEL_PID;
    //NETLINK_CB(skb).dst_group = SYSWATCH_USERSPACE_GROUP;
    memcpy(NLMSG_DATA(nlhdr), pstCtInfo, sizeof(ATP_CT_CLEAN_INFO));

    netlink_unicast(syswatch_nl_sock, skb, pid, 0);
	
	return;
}

/******************************************************************************
  函数名称： hw_ctclean_process
  功能描述： 发送消息
  调用函数：
  被调函数：nl_recv_msg， nl_send_msg， hw_ctclean_find_ct
  输入参数：struct sock *sk, int len
  输出参数：
  返 回 值：
  其它说明：
******************************************************************************/
void hw_ctclean_process(struct sk_buff *skb)
{
    u32 pid;
    ATP_CT_CLEAN_INFO stCtRuleInfo;

    memset(&stCtRuleInfo, 0, sizeof(ATP_CT_CLEAN_INFO));

    /*接收规则信息，存入stCtRuleInfo*/
    if (-1 == hw_ctclean_recv_msg((struct sk_buff *)skb, &pid, &stCtRuleInfo))
    {
        printk("hw_ctclean_process recv msg fail!\n");
        return;
    }

    /*根据规则查找匹配的ct，返回给userspace用于block*/
    if ((BREAK_DMZ == stCtRuleInfo.action) || (BREAK_PORTMAPPING == stCtRuleInfo.action))
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
        hw_ctclean_find_ct(&stCtRuleInfo);
#else
        hw_ctclean_find_ct(&init_net, &stCtRuleInfo);
#endif
        hw_ctclean_send_msg(pid, &stCtRuleInfo);
        return;
    }

    /*删除ct*/
    if (DELETE_CT == stCtRuleInfo.action)
    {
        hw_ctclean_do_delete();
    }

    return;     
}

