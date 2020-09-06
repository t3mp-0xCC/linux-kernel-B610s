
/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <net/sock.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <net/ipv6.h>
#include <net/ndisc.h>

#ifdef CONFIG_ATP_ROUTE
#include "kcmsmonitormsgtypes.h"
#include "./../../package/atp/network/common/inc/atplogdef.h"
#include "./../../package/atp/network/common/inc/atp_interface.h"
#endif


#ifdef CONFIG_ATP_CT_CLEAN
#include "hw_ctclean.h"
#endif
//MODULE_LICENSE("GPL");
//MODULE_DESCRIPTION("Netlink module for kernel<->userspace messge.");

/******************************************************************************
  结构名称  : ATP_NETLINK_MSG_HANDLER
  结构作用  : ATP netlink 内核态消息处理句柄
  调用函数  : ATP_Netlink_Register/ATP_Netlink_Unregister等

  注意事项  :模块注册该句柄时，句柄加入全局链表中
******************************************************************************/
typedef struct tagATP_NETLINK_MSG_HANDLER
{
    struct list_head list;
    unsigned short ulModuleId;
    PFNetlinkMsgProc pfMsgProc;
} ATP_NETLINK_MSG_HANDLER;


/******************************************************************************
  变量名称  : g_ATPNetlinkMsgHead
  变量作用  : 内核态netlink消息处理句柄链表头
  取值范围  : atp netlink内核态模块
  调用函数  : ATP_Netlink_Register/ATP_Netlink_Unregister等

  注意事项  :模块注册该句柄时，句柄加入此全局链表中
******************************************************************************/
static struct list_head g_ATPNetlinkMsgHead;


/******************************************************************************
  变量名称  : atp_netlink_mutex
  变量作用  : 保护内核态消息处理句柄链表
  取值范围  : atp netlink内核态模块
  调用函数  : ATP_Netlink_Register/ATP_Netlink_Unregister/hw_netlink_msg_dispatch等

  注意事项  :
******************************************************************************/
static DEFINE_MUTEX(atp_netlink_mutex);


void (*hw_ctclean_input)(struct sk_buff *skb) = NULL;

#ifdef CONFIG_DPI_PARSE
#include "hw_dpi.h"
void (*hw_dpi_input)(struct sk_buff *skb) = NULL;
extern void hw_dpi_process(struct sk_buff *skb);
#endif

#ifdef CONFIG_ATP_ROUTE_BALANCE
void (*hw_route_balance_input)(struct sk_buff *skb) = NULL;
#endif


#define MAC_ADDRESS_LEN (6)
#define INTERFACE_NAME_LEN (16)
#define ATP_ACL_SERVICE_ACTION_LEN (20)
#define ATP_ACL_LEN (8)
#define LOG_ACL_LEN (50)
#define cb_mutex 32
/*内核通知用户态DAD检测报文的信息结构体*/
typedef struct dad_ns_msg_st
{
    struct in6_addr target_address;
    unsigned char mac_addr[MAC_ADDRESS_LEN];
    char acInterfaceName[INTERFACE_NAME_LEN];
}dad_ns_msg;


/*默认记录攻击报文的时间间隔为8小时*/
#ifdef CONFIG_MTK_CHIP
#define ATTACK_LOG_TIME_INTERVAL 60 // HG532T修改防火墙日志限速打印时间
#else
#define ATTACK_LOG_TIME_INTERVAL 28800
#endif
static long lLastLogTime = 0;
int attack_log_ratelimit(void)
{
    struct timeval t;
    jiffies_to_timeval(jiffies, &t);
    
    if (((t.tv_sec - lLastLogTime) < ATTACK_LOG_TIME_INTERVAL)
        && (t.tv_sec - lLastLogTime > 0))
    {
        return 0;
    }
    lLastLogTime = t.tv_sec;
    return 1;
}

static inline int ipv6_addr_solicit_multicast(const struct in6_addr *a)
{
	return (((a->s6_addr32[0] ^ htonl(0xFF020000))
                | a->s6_addr32[1] 
                | (a->s6_addr32[2] ^ htonl(0x00000001))
                | ((a->s6_addr32[3] & htonl(0xFF000000)) ^ htonl(0xFF000000))) == 0); 
}

/*after ipv6_rcv, the necessary check is done*/
void dad_skb_send_up(void *pskb, void *phdr)
{
	struct sk_buff *skb = (struct sk_buff *)pskb;
    struct ipv6hdr *hdr = (struct ipv6hdr *)phdr;
    
    if ((ipv6_addr_any(&(hdr->saddr)))
        &&(ipv6_addr_solicit_multicast(&(hdr->daddr)))
        &&(NEXTHDR_ICMP == hdr->nexthdr)
        && IS_BRIDGE_DEV(skb->dev->name))
    {
        dad_ns_msg st_DadNsMsg;
		struct nd_msg *msg;
        memset(&st_DadNsMsg, 0, sizeof(dad_ns_msg));
        msg = (struct nd_msg *)skb_transport_header(skb);

        memcpy(&st_DadNsMsg.target_address, &msg->target, sizeof(st_DadNsMsg.target_address));
        memcpy(st_DadNsMsg.mac_addr, skb_mac_header(skb)+6, MAC_ADDRESS_LEN);
        snprintf(st_DadNsMsg.acInterfaceName, sizeof(st_DadNsMsg.acInterfaceName), "%s", skb->dev->name);
        syswatch_nl_send(ATP_MSG_MONITOR_EVT_LAN_DEVICE_UP, (unsigned char *)&st_DadNsMsg, sizeof(dad_ns_msg));
    }
}

void eth_link_change_hooker(const char *pszChangeEthName,int portNum,int result)
{
    char *ptrEthName = NULL;
    netlink_common_msg *pstMsg = NULL;
    
    if(NULL == pszChangeEthName || 0 == strlen(pszChangeEthName))
    {
        printk("\nInvalid para,null pointer.\n");

        return;
    }

    ptrEthName = (char *)kmalloc((sizeof(netlink_common_msg)), GFP_KERNEL);
    if (NULL == ptrEthName)
    {        
        printk("\nKmalloc mem error.\n");
        
        return;
    }

    pstMsg = (netlink_common_msg *)ptrEthName;
    pstMsg->eventType = ATP_MSG_MONITOR_EVT_LAN_ETH;
    pstMsg->eventResult = result;
    pstMsg->eventPortNum = portNum;    
    snprintf(pstMsg->acPortName,NETLINK_MSG_CMD_LEN,"%s",pszChangeEthName);

    syswatch_nl_send(ATP_MSG_MONITOR_EVT_LAN_ETH, (unsigned char *)pstMsg,sizeof(netlink_common_msg));

    kfree((void*)ptrEthName);

    return;
}

EXPORT_SYMBOL(eth_link_change_hooker);

static u32 us_pid = -1;

static int syswatch_nl_inited = 0;
struct sock *syswatch_nl_sock = NULL;
static DEFINE_MUTEX(hw_netlink_mutex); /* serialization of message processing */

typedef struct tagQUEUED_SKB
{
    struct tagQUEUED_SKB  *pstNext;
    struct sk_buff *skb;
} QUEUED_SKB;

QUEUED_SKB *m_pstQueuedSkbs = NULL;

static void syswatch_queue_skb(struct sk_buff *skb)
{
    QUEUED_SKB **ppstItem = NULL;

    ppstItem = &m_pstQueuedSkbs;
    while (NULL != (*ppstItem))
    {
        ppstItem = &((*ppstItem)->pstNext);
    }
    (*ppstItem) = (QUEUED_SKB *)kmalloc(sizeof(QUEUED_SKB), GFP_ATOMIC);
    
    if (NULL == (*ppstItem))
    {
        return;
    }
    
    (*ppstItem)->pstNext = NULL;
    (*ppstItem)->skb = skb;
}

static void syswatch_queue_destroy(void)
{
    QUEUED_SKB *pstItem = NULL;
    QUEUED_SKB *pstNext = NULL;

    pstItem = m_pstQueuedSkbs;
    while (NULL != pstItem)
    {
        pstNext = pstItem->pstNext;
        kfree((void *)pstItem);
        pstItem = pstNext;
    }
    m_pstQueuedSkbs = NULL;
}

/*----------------------------------------------------------------------------
函数原型： static void get_service_status(const char *str, char *service, char *action)
描    述： 从字符串中获取到 服务和动作XXX_XXX
输    入： service 得到的服务如ICMP
           action 接受的动作如ACCEPT/DROP
输    出： 无
返 回 值:	无
------------------------------------------------------------------------------*/

static void get_service_status(const char *str, char *pszservice, char *pszaction)
{
    const char *pcstr = NULL;
    int laction_index = 0;
    int lservice_index = 0;
    char acService_action[ATP_ACL_SERVICE_ACTION_LEN];

    if (NULL == str)
    {
        return;
    }

    memset(acService_action, 0, sizeof(acService_action));

    pcstr = &str[ATP_ACL_LEN];
    while ('_' != *pcstr && '\0' != *pcstr && lservice_index < ATP_ACL_SERVICE_ACTION_LEN)
    {
        pszservice[lservice_index] = *pcstr;
        pcstr++;
        lservice_index++;
    }

    if (ATP_ACL_SERVICE_ACTION_LEN == lservice_index)
    {
        pszservice[lservice_index - 1] = '\0';
        return;
    }

    pszservice[lservice_index] = '\0';

    pcstr++;
    while ('_' != *pcstr && '\0' != *pcstr && laction_index < ATP_ACL_SERVICE_ACTION_LEN)
    {
        pszaction[laction_index] = *pcstr;
        pcstr++;
        laction_index++;
    }

    if (ATP_ACL_SERVICE_ACTION_LEN == laction_index)
    {
        pszaction[laction_index - 1] = '\0';
        return;
    }

    pszaction[laction_index] = '\0';

}


/*-------------------------------------------------
函数原型： static int syswatch_nl_send(char *name, char comm);
描    述：		通过netlink发送消息到userspace 
输    入： 		name  发送的消息内容
                       		comm 发送的消息类型参考syswatch_msg_comm
输    出：  	无
返 回 值:		 正确时返回0
                       		错误时返回<0
--------------------------------------------------*/
int syswatch_nl_send(unsigned int type, unsigned char *buf, unsigned int len)
{
    struct sk_buff *skb;
    struct generic_nl_msg *body_msg;
    struct nlmsghdr *nlh;
    static int seq = 0;
    int size;

    if (len > 1024)
    {
        len = 1024;
    }
    if ((NULL == buf) || (0 == len))
    {
        size = NLMSG_SPACE(sizeof(*body_msg));
    }
    else 
    {
        size = NLMSG_SPACE(sizeof(*body_msg) + len);
    }

    skb = alloc_skb(size, GFP_ATOMIC);
    if (!skb)
    {
        //printk(KERN_ALERT "OOM when syswatch_nl_send");
        return -ENOMEM;
    }

    nlh = NLMSG_PUT(skb, KERNEL_PID, seq++, NLMSG_DONE, size - sizeof(*nlh));
    body_msg = (struct generic_nl_msg *)NLMSG_DATA(nlh);

    memset(body_msg, 0, size - sizeof(*nlh)); 
    body_msg->comm = type;

    if ((NULL != buf) && (len > 0))
    {
        memcpy(body_msg->data, buf, len);
        body_msg->len = len;
    }
    else 
    {
        body_msg->len = 0;
    }

    NETLINK_CB(skb).dst_group = SYSWATCH_USERSPACE_GROUP;

    //return netlink_broadcast(syswatch_nl_sock, skb, 0, SYSWATCH_USERSPACE_GROUP, GFP_ATOMIC);
    if (-1 == us_pid)
    {
        printk(KERN_ALERT "No userspace pid joined yet, queue it.\n");
        syswatch_queue_skb(skb);
        return 0;
    }
    
nlmsg_failure:
    return netlink_unicast(syswatch_nl_sock, skb, us_pid, 1);
    
}



extern volatile int log_limit;

int syswatch_sendLog(unsigned int logType, unsigned int logLevel, unsigned int logNum, unsigned char *str)
{
    netlink_log_header  stHeader;
    netlink_log_header *pstHeader;
    unsigned char *pucBuf;
    unsigned int len;
    int ret;
    char pstLogStr[LOG_ACL_LEN];
    char service[ATP_ACL_SERVICE_ACTION_LEN];
    char action[ATP_ACL_SERVICE_ACTION_LEN];
    int need_log = 0;

    memset(pstLogStr, 0, sizeof(pstLogStr));
    memset(service, 0,sizeof(service));
    memset(action, 0, sizeof(action));

    if (str && strstr(str, "ATP_ACL"))
    {
        need_log = 1;
    }

#ifdef CONFIG_ATP_FIREWALL_LOG
    if ((ATP_LOG_TYPE_FIREWALL == logType) && (0 == need_log))
    {
    	if (log_limit)
	    {
	    	return -1;
	    }
	    log_limit = 1;
    }
#endif

    if (NULL == str)
    {
        stHeader.logType = logType;
        stHeader.logLevel = logLevel;
        stHeader.logNum = logNum;
        return syswatch_nl_send(ATP_MSG_MONITOR_EVT_LOG, (unsigned char *)(&stHeader), sizeof(netlink_log_header));
    }

    if (need_log)
    {
        get_service_status(str, service, action);
        snprintf(pstLogStr, sizeof(pstLogStr), "%s %s Request", action, service);
        len = (unsigned int)(strlen(pstLogStr) + 1);
    }
    else
    {
        len = (unsigned int)(strlen(str) + 1);
    }

    pucBuf = (unsigned char *)kmalloc((len + sizeof(netlink_log_header)), GFP_ATOMIC);
    if (NULL == pucBuf)
    {
        return -1;
    }

    pstHeader = (netlink_log_header *)pucBuf;
    pstHeader->logType = logType;
    pstHeader->logLevel = logLevel;
    pstHeader->logNum = logNum;
    if (need_log)
    {
        memcpy((void *)(pucBuf + sizeof(netlink_log_header)), pstLogStr, len);
    }
    else
    {
        memcpy((void *)(pucBuf + sizeof(netlink_log_header)), str, len);
    }

    len += sizeof(netlink_log_header);
    ret = syswatch_nl_send(ATP_MSG_MONITOR_EVT_LOG, pucBuf, len);
    kfree((void *)pucBuf);
    return ret;
}

EXPORT_SYMBOL(syswatch_sendLog);
EXPORT_SYMBOL(syswatch_nl_send);

static ATP_NETLINK_MSG_HANDLER * hw_netlink_findhandler(unsigned short ulModuleId)
{
    ATP_NETLINK_MSG_HANDLER *stMsgNode = NULL;
    struct list_head *stListNode = NULL;

    mutex_lock(&atp_netlink_mutex);
    list_for_each(stListNode, &g_ATPNetlinkMsgHead)
    {
        stMsgNode = list_entry(stListNode, ATP_NETLINK_MSG_HANDLER, list);

        if (stMsgNode->ulModuleId == ulModuleId)
        {
            mutex_unlock(&atp_netlink_mutex);
            return stMsgNode;
        }

    }
    mutex_unlock(&atp_netlink_mutex);

    return NULL;
}


static int hw_netlink_msg_dispatch(struct sk_buff *skb)
{
    struct nlmsghdr *nlhdr = NULL;
    ATP_NETLINK_MSG_HANDLER *stMsgHandler = NULL;

    nlhdr = (struct nlmsghdr *)skb->data;
    stMsgHandler = hw_netlink_findhandler(nlhdr->nlmsg_type);
    if (NULL == stMsgHandler)
    {
        return -1;
    }

    if (!NLMSG_OK(nlhdr, skb->len))
    {
        return -1;
    }

    return stMsgHandler->pfMsgProc(stMsgHandler->ulModuleId, NLMSG_DATA(nlhdr), NLMSG_PAYLOAD(nlhdr, 0));
}


/*-------------------------------------------------
函数原型： static void syswatch_nl_input(struct sk_buff *__skb)；
描    述： 		此函数处理userspace下发的消息
输    入： 		封装了userspace消息体的sk_buff
输    出： 		无
返 回 值： 	无
--------------------------------------------------*/
static void syswatch_nl_input(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
	QUEUED_SKB *pstItem;

    if(NULL == skb)
    {
        printk("\n Recv empty netlink data.\n");
        return;
    }
    
    nlh = (struct nlmsghdr *)skb->data;
    printk("received netlink message payload from pid %d:%d:%d:%d\n", nlh->nlmsg_pid, skb->data_len, nlh->nlmsg_len, nlh->nlmsg_type);

    /*已注册的module自己处理*/
    if (0 == hw_netlink_msg_dispatch(skb))
    {
        return;
    }

    if (ATP_KCMSMAIN_NETLINK_NAT == nlh->nlmsg_type)
    {
        if (hw_ctclean_input)
        {
            hw_ctclean_input(skb);
        }
        return;
    }
    
#ifdef CONFIG_DPI_PARSE
    else if(ATP_KCMSMAIN_NETLINK_DPI == nlh->nlmsg_type)
    {
        if (hw_dpi_input)
        {
            hw_dpi_input(skb);
        }
        return;
    }
#endif

#ifdef CONFIG_ATP_ROUTE_BALANCE
    else if(ATP_KCMSMAIN_NETLINK_ROUTE == nlh->nlmsg_type)
    {
        if (hw_route_balance_input)
        {
            hw_route_balance_input(skb);
        }
        return;
    }
#endif

	if ((u32)-1 == us_pid) {
    	us_pid = nlh->nlmsg_pid;
	}

    
    pstItem = m_pstQueuedSkbs;
    while (NULL != pstItem)
    {
        netlink_unicast(syswatch_nl_sock, pstItem->skb, us_pid, 1);
        pstItem = pstItem->pstNext;
    }
    syswatch_queue_destroy();
}
#if defined(CONFIG_RT63365) || defined(CONFIG_BCM5358) || defined(CONFIG_RTL8676S)
static void syswatch_nl_input_sk(struct sock *sk, int len)
{
    struct sk_buff *skb;
    unsigned int qlen = skb_queue_len(&sk->sk_receive_queue);

    while (qlen-- && (skb = skb_dequeue(&sk->sk_receive_queue))) {
        syswatch_nl_input(skb);
        kfree_skb(skb);
    }
}
#endif


int ATP_Netlink_SendToUserspace(unsigned short usModuleId, void *pvData, unsigned int ulDataLen)
{
    return syswatch_nl_send((unsigned int)usModuleId, pvData, ulDataLen);
}



int ATP_Netlink_Register(unsigned short ulModuleId, PFNetlinkMsgProc pfMsgProc)
{
    ATP_NETLINK_MSG_HANDLER *pstMsgHandler = NULL;

    pstMsgHandler = hw_netlink_findhandler(ulModuleId);
    if (NULL != pstMsgHandler)
    {
        printk("Module [%x] has been registered\n", ulModuleId);
        return -1;
    }

    pstMsgHandler = (ATP_NETLINK_MSG_HANDLER *)kmalloc(sizeof(ATP_NETLINK_MSG_HANDLER), GFP_KERNEL);
    if (NULL == pstMsgHandler)
    {
        return -1;
    }

    memset(pstMsgHandler, 0, sizeof(ATP_NETLINK_MSG_HANDLER));
    pstMsgHandler->ulModuleId = ulModuleId;
    pstMsgHandler->pfMsgProc = pfMsgProc;
    
    mutex_lock(&atp_netlink_mutex);
    list_add(&pstMsgHandler->list, &g_ATPNetlinkMsgHead);
    mutex_unlock(&atp_netlink_mutex);

    return 0;
}



int ATP_Netlink_Unregister(unsigned short ulModuleId)
{
    struct list_head *stListNode = NULL;
    struct list_head *stListTmpNode = NULL;
    ATP_NETLINK_MSG_HANDLER *stMsgHandler = NULL;

    /*查找并删除*/
    mutex_lock(&atp_netlink_mutex);
    list_for_each_safe(stListNode, stListTmpNode, &g_ATPNetlinkMsgHead)
    {
        stMsgHandler = list_entry(stListNode, ATP_NETLINK_MSG_HANDLER, list);

        if (stMsgHandler->ulModuleId == ulModuleId)
        {
            list_del(&stMsgHandler->list);
            kfree(stMsgHandler);
        }
    }
    mutex_unlock(&atp_netlink_mutex);

    return 0;
}

EXPORT_SYMBOL(ATP_Netlink_SendToUserspace);
EXPORT_SYMBOL(ATP_Netlink_Register);
EXPORT_SYMBOL(ATP_Netlink_Unregister);


/*-------------------------------------------------
函数原型： static int syswatch_nl_init(void);
描    述： 		初始化内核syswatch netlink socket
输    入： 		无
输    出： 		无
返 回 值： 	创建成功返回0
				否则返回< 0
--------------------------------------------------*/
static int syswatch_nl_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .groups = cb_mutex,
        .input = syswatch_nl_input,
    };
    /*init atp netlink head*/
    INIT_LIST_HEAD(&g_ATPNetlinkMsgHead);

#ifdef CONFIG_ATP_CT_CLEAN
    /*register ct clean hook*/
    hw_ctclean_input = hw_ctclean_process;
    printk("set hook ct_clean_input to hw_ctclean_process\n");
#endif

#ifdef CONFIG_DPI_PARSE
    /*register dpi hook*/
    hw_dpi_input = hw_dpi_process;
    printk("set hook hw_dpi_process success.\n");
#endif


#if defined(CONFIG_RT63365) || defined(CONFIG_BCM5358) || defined(CONFIG_RTL8676S)  //#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
	syswatch_nl_sock = netlink_kernel_create(NETLINK_SYSWATCH, 0,
					  syswatch_nl_input_sk, THIS_MODULE);
#else
    syswatch_nl_sock = netlink_kernel_create(&init_net, NETLINK_SYSWATCH, &cfg);
#endif
    if (!syswatch_nl_sock)
    {
        printk(KERN_ALERT "kernel create netlink failed.\n");
        return -EIO;
    }
    syswatch_nl_inited = 1;

    return 0;
}

static void syswatch_nl_fini(void)
{
    syswatch_nl_inited = 0;
    us_pid = -1;
    //netlink_kernel_release(syswatch_nl_sock);
}

subsys_initcall(syswatch_nl_init);
module_exit(syswatch_nl_fini);
