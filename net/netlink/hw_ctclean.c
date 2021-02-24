

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
  �������ƣ� hw_ctclean_do_delete
  ���������� ������Ԫ��ɾ�����Ӹ���
  ���ú����� hw_ctclean_process
  ����������
  ���������
  ���������
  �� �� ֵ��
  ����˵����
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
  �������ƣ� hw_ctclean_check_ct
  ���������� �����ж��Ƿ�����Ҫ�����Ӹ���
  ���ú�����find_need_ct
  ����������
  ���������ATP_CT_CLEAN_INFO* pstRuleInfo �û�̬�������ݣ�
           struct nf_conn *ct ���Ƚϵ����Ӹ���,
		   int *rule_num ѭ������ֵ
  ���������
  �� �� ֵ������Ҫ�����Ӹ��٣�����true�����򷵻�false
  ����˵����
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
    
    /*Э������ΪUDP ����Э������ΪTCP��TCP״̬ΪESTABLISHED */
    /*��ע: tcp_conntrack_names������ESTABLISHED״̬������Ϊ3����ֹ�ͻ��˷������ӳ��ֺܶ���Ч���Ӹ��ٶԲ��ҽ��и���*/
    if (UDP_PROTOCOL != ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum
        && TCP_PROTOCOL != ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum)
    {
        return false;
    }

    if (TCP_PROTOCOL == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum && ESTABLISHED != ct->proto.tcp.state)
    {
        return false;
    }
    
    /*�����Դ��ַ������WAN��*/
    if ((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip & lanmask) == (pstRuleInfo->ruleinfo[0].lanhostIP & lanmask))
    {
        return false;
    }

    /*�����Դ��ַ������wan�ӿ�IP */
    if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip == pstRuleInfo->ruleinfo[0].interfaceIP)
    {
        return false;
    }

    /*�����Ŀ�ĵ�ַ����Ϊ���ؽӿ�IP */
    if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip != pstRuleInfo->ruleinfo[0].interfaceIP)
    {
        return false;
    }

    /*�ظ���Դ��ַ����ΪLAN������IP */
    if (ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip != pstRuleInfo->ruleinfo[0].lanhostIP)
    {
        return false;
    }

    /*�ظ���Ŀ�ĵ�ַ������WAN��*/
    if ((ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip & lanmask) == (pstRuleInfo->ruleinfo[0].lanhostIP & lanmask))
    {
        return false;
    }

    /*�ظ���Ŀ�ĵ�ַ������wan�ӿ�IP*/
    if (ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip == pstRuleInfo->ruleinfo[0].interfaceIP)
    {
        return false;
    }

    if (BREAK_PORTMAPPING != pstRuleInfo->action)
    {
        return true;
    }
                            
    /*Portmappingģʽʱ���ж϶˿��Ƿ����Ҫ��Э��Ͷ˿��Ƿ�ƥ��*/
    index = *rule_num;

    /*PortMappingҪ��Э�����ͱ���һ��*/
    if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum != pstRuleInfo->ruleinfo[index].protocol)
    {
        return false;
    }

    /*tcp���ĵ�Ŀ�Ķ˿�Ҫ��PortMapping�����start--end��Χ��*/
    if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.tcp.port >= htons(pstRuleInfo->ruleinfo[index].startport)
            && ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.tcp.port <= htons(pstRuleInfo->ruleinfo[index].endport))
    {
        return true;
    }

    /*udp���ĵ�Ŀ�Ķ˿�Ҫ��PortMapping�����start--end��Χ��*/
    if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.udp.port >= htons(pstRuleInfo->ruleinfo[index].startport)
            && ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.udp.port <= htons(pstRuleInfo->ruleinfo[index].endport))
    {
        return true;
    }

    /*ct��PortMapping����Ķ˿ڷ�Χ��һ��*/
    return false;
}

/******************************************************************************
  �������ƣ� hw_ctclean_add_ct_to_del_list
  ���������� �����ҵ������Ӹ��ټ����ɾ���б�
  ���ú�����find_need_ct
  ����������
  ���������int *ct_num ��ǰ�����Ӹ���������, struct nf_conn *ct ���Ӹ���
  ���������int *ct_num �����ǰ�����Ӹ���������
  �� �� ֵ��
  ����˵����
******************************************************************************/
static void hw_ctclean_add_ct_to_del_list(int *ct_num, struct nf_conn *ct)
{
    int ct_search_index = 0;
    int repeat_flag = 0; /*���ڱ���Ƿ��Ѵ洢��ͬ�����Ӹ��ټ�¼*/

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
            /*�Ѽ�¼�����Ӹ���*/
            repeat_flag = 1;
            break;
        }
        ct_search_index++;
    }

    /*���֮ǰ��û��Ӵ����Ӹ��٣�����ӣ������������*/
    if (0 == repeat_flag) 
    {
        match_ct_list[*ct_num] = ct;
        DEBUGPC("find conntrack[%d]=%p\n", *ct_num, ct);
        (*ct_num)++;
    }
    
    return;
}


/******************************************************************************
  �������ƣ� hw_ctclean_find_ct
  ���������� ���ڲ��ҷ������������Ӹ��ٲ��Դ˹������ݷ���
  ���ú�����
  ����������
  ���������ATP_CT_CLEAN_INFO* pstCtRule ���ݰ���������Ϣ
  ���������ATP_CT_CLEAN_INFO* pstCtRule ���ݰ���ƥ������Ӹ�����Ϣ
  �� �� ֵ��
  ����˵����
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
    
    int rule_num = 0; /*���ڲ�ѯ����������*/
    int ct_num = 0; /*����ͳ�Ƽ�¼�����Ӹ�������*/
    int mode = 0;
    int LanHostIP = 0;
    int main_ct_num = 0; /*���ҵ��������Ӹ�������*/

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

    /*���������Ӹ��ټ�¼*/
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

            /*DMZģʽ��ƥ�����:*/
            /*���Ӹ����з����Դ��ַΪ����IP����LAN���Σ�������WAN IP��,�����Ŀ�ĵ�ַΪDMZ����ָ����WAN�ӿڵ�IP��*/
            /*���Ӹ����лظ���Դ��ַΪDMZ����ָ����LAN������IP���ظ���Ŀ�ĵ�ַΪ����IP(��LAN���Σ�������WAN IP����*/

            /*Port mapping ��ƥ�����*/
            /*Э�����ͺ�Port mapping�����Э������һ�£�*/
            /*���Ӹ����з����Դ��ַΪ����IP����LAN���Σ�������WAN IP��,�����Ŀ�ĵ�ַΪPort mapping����ָ����WAN�ӿڵ�IP�������Ŀ�Ķ˿���Port mapping���������Ŀ�Ŀ�ʼ�˿ںͽ����˿ڷ�Χ֮�ڣ�*/
            /*���Ӹ����лظ���Դ��ַΪPort mapping����ָ����LAN������IP���ظ���Ŀ�ĵ�ַΪ����IP(��LAN���Σ�������WAN IP�����ظ���Դ�˿���Port mapping���������Ŀ�Ŀ�ʼ�˿ںͽ����˿ڷ�Χ֮�ڡ�*/

            /*DMZģʽ�²���������Ҫ��������*/
            if (BREAK_DMZ == mode && true == hw_ctclean_check_ct(pstCtRule, ct, &rule_num))
            {
                hw_ctclean_add_ct_to_del_list(&ct_num, ct);
            }

            /*Portmappingģʽ�²���������Ҫ��������*/
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

    /*���������Ӹ��ٲ������е�Ԥ�����Ӹ���*/
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

    /*���ݲ�õĽ����������*/
    /*����û�̬�����ں�̬���鲻ͬ�����ں�̬�����û�̬ʱ��Ԫ��������иı�*/
    /*�鵽�����Ӹ������������֣�Ϊʵ�ִӷ���˷�£����ص�ֵ����*/
    
    /*ģʽ1: ipv4     2 tcp      6 431997 ESTABLISHED src=188.28.8.150 dst=189.28.99.48 sport=37113 dport=21 src=192.168.1.3 dst=188.28.8.150 sport=21 dport=37113*/
    /*���ڴ��֣����ػظ���Ŀ�ĵ�ַ��Ŀ�Ķ˿�dst=188.28.8.150 dport=37113*/
    
    /*ģʽ2: ipv4     2 tcp      6 431999 ESTABLISHED src=192.168.1.3 dst=188.28.8.150 sport=20 dport=37115 src=188.28.8.150 dst=189.28.99.48 sport=37115 dport=20*/
    /*���ڴ��֣����ط����Ŀ�ĵ�ַ��Ŀ�Ķ˿�dst=188.28.8.150  dport=37115*/
    /*protocol�洢Э��ţ�interfaceIP�洢���ص�IP��startport�洢���صĶ˿�*/
    
    DEBUGPC("Total ct is %d mode=%d\n", ct_num, mode);
    memset(pstCtRule, 0, sizeof(ATP_CT_CLEAN_INFO));

    /*�������ģʽд�أ���ΪУ��ʹ��*/
    pstCtRule->action = mode;

    /*���鵽�����Ӹ�������Ŀ��д��*/
    pstCtRule->entrynum = ct_num; 

    rule_num = 0;
    while (rule_num < ct_num)
    {
        ct = match_ct_list[rule_num];
        pstCtRule->ruleinfo[rule_num].protocol = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
        if (ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip == LanHostIP)   //ģʽ1
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

        if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip == LanHostIP)   //ģʽ2  
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
  �������ƣ� hw_ctclean_recv_msg
  ���������� У�鱨�ģ��Ϸ���������Ϣ���ݿ���pstRuleInfo
  ���ú�����
  ����������
  ���������struct sock *sk, u32 *pid, ATP_CT_CLEAN_INFO *pstRuleInfo
  ���������ATP_CT_CLEAN_INFO *pstRuleInfo
  �� �� ֵ��0��ʾ��ȷ��-1��ʾ����
  ����˵����
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
  �������ƣ� hw_ctclean_send_msg
  ���������� ������Ϣ,��Ϣ����Ϊ���ҵ�����Ҫblock�����Ӹ�����Ϣ
  ���ú�����
  ����������
  ���������u32 *pid, ATP_CT_CLEAN_INFO *pstCtInfo
  ���������
  �� �� ֵ��0��ʾ��ȷ��-1��ʾ����
  ����˵����
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
    
    /* ��ʼ����Ϣͷ������ */
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
  �������ƣ� hw_ctclean_process
  ���������� ������Ϣ
  ���ú�����
  ����������nl_recv_msg�� nl_send_msg�� hw_ctclean_find_ct
  ���������struct sock *sk, int len
  ���������
  �� �� ֵ��
  ����˵����
******************************************************************************/
void hw_ctclean_process(struct sk_buff *skb)
{
    u32 pid;
    ATP_CT_CLEAN_INFO stCtRuleInfo;

    memset(&stCtRuleInfo, 0, sizeof(ATP_CT_CLEAN_INFO));

    /*���չ�����Ϣ������stCtRuleInfo*/
    if (-1 == hw_ctclean_recv_msg((struct sk_buff *)skb, &pid, &stCtRuleInfo))
    {
        printk("hw_ctclean_process recv msg fail!\n");
        return;
    }

    /*���ݹ������ƥ���ct�����ظ�userspace����block*/
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

    /*ɾ��ct*/
    if (DELETE_CT == stCtRuleInfo.action)
    {
        hw_ctclean_do_delete();
    }

    return;     
}

