
#include "atp_sipalg_sock.h"

#define ALG_SIP_START_FAIL            0

struct SipRouteInfoItem
{
    __be32 daddr;
    __be16 dport;
    uint32_t proto;
    char *call_id;
    int call_id_len;
};

struct SipRouteInfo
{
    struct list_head list;      /* list head */

    struct SipRouteInfoItem stRouteInfo;
};

enum {
    SipRouteSet = 0,            /* 设置alg端口 */
    SipRouteClear = 1,          /* 清空sipalg相关信息 */
    SipRouteStart = 2,          /* 开始记录sipalg相关信息 */
};

struct SipRouteCmd
{
    uint32_t CmdType;           /*命令类型:设置端口，重置list等*/

    uint32_t sipalg_portinfo;   /* 设置的alg端口 */
};

#define MAX_SIPROUTE_RECORD 50
uint32_t g_ulSipPortInfo = 0; /* default is 0, not eanbled.*/
uint32_t g_ulSiproxdStatus = ALG_SIP_START_FAIL;
spinlock_t g_SipRouteLock;
struct list_head g_SipRouteList;
uint32_t g_SipRouteListCount = 0; /* default is 0, no route info.*/
int lSipInfoInited = 0;// false

/*
 * Update sip route info base on call-id, add/remove/update.
 */
void UpdateSipRouteByCallid(unsigned char *call_id, unsigned int call_id_len, __be32 daddr, __be16 dport, uint32_t proto, bool bAdd)
{
    /* for now only saddr and daddr is used. */
    struct SipRouteInfo* pstInfo = NULL;
    struct SipRouteInfo* tmp_entry = NULL;

    if ((NULL == g_SipRouteList.next) ||(NULL == g_SipRouteList.prev))
    {
        printk("\n warning : access un-initialized pointer g_SipRouteList !!\n");
        return ;
    }
    spin_lock_bh(&g_SipRouteLock);

    if (bAdd)
    {
        /* although call-ids may be same, add it as a new one, to process the retrans request */

        /* it is a new one. add to list. */
        if (NULL != (pstInfo = (struct SipRouteInfo*)kmalloc(sizeof(struct SipRouteInfo), GFP_KERNEL)))
        {

            memset(pstInfo, 0, sizeof(struct SipRouteInfo));
            if (NULL != (pstInfo->stRouteInfo.call_id = (char *)kmalloc(call_id_len, GFP_KERNEL)))
            {
                memcpy(pstInfo->stRouteInfo.call_id, call_id, call_id_len);
                pstInfo->stRouteInfo.call_id_len = call_id_len;
                pstInfo->stRouteInfo.daddr = daddr;
                pstInfo->stRouteInfo.dport = dport;
                pstInfo->stRouteInfo.proto = proto;

                /* 新增的加到链表尾部 */
                list_add_tail_rcu(&pstInfo->list, &g_SipRouteList);
                g_SipRouteListCount++;
            }
            /* 超过限制的数量则删除最老的记录即链表的头部，防止无限扩张 */
            if (g_SipRouteListCount > MAX_SIPROUTE_RECORD)
            {
                pstInfo = list_entry(rcu_dereference(g_SipRouteList.next), typeof(*pstInfo), list);
                list_del_rcu(g_SipRouteList.next);
                if (NULL != pstInfo->stRouteInfo.call_id)
                {
                    kfree(pstInfo->stRouteInfo.call_id);
                    pstInfo->stRouteInfo.call_id = NULL;
                }
                kfree(pstInfo);
                pstInfo = NULL;
                g_SipRouteListCount--;
            }

        }
    }
    else
    {
        /* if find same sip, remove it. */
        list_for_each_entry_safe(pstInfo, tmp_entry, &g_SipRouteList, list)
        {
            if (NULL != pstInfo->stRouteInfo.call_id)
            {
                if (0 == strncmp(pstInfo->stRouteInfo.call_id, call_id, strlen(call_id)))
                {
                    list_del(&pstInfo->list);
                    kfree(pstInfo->stRouteInfo.call_id);
                    pstInfo->stRouteInfo.call_id = NULL;
                    kfree(pstInfo);
                    pstInfo = NULL;
                    g_SipRouteListCount--;
                }
            }
        }
    }
    spin_unlock_bh(&g_SipRouteLock);
}


unsigned int atp_sipalg_setsock(struct sk_buff* skb, int optname, char __user *optval)
{
	 struct SipRouteCmd stCmd = {0};
	 
    /* need to initial list and lock, this will be executed always and once. */
    if (!lSipInfoInited)
    {
      INIT_LIST_HEAD(&g_SipRouteList);
      spin_lock_init(&g_SipRouteLock);
      lSipInfoInited = 1;
    }
    if (copy_from_user(&stCmd, optval, sizeof(struct SipRouteCmd)))
    {
      return -EFAULT;
    }

    switch (stCmd.CmdType)
    {
      /* set alg proxy port info*/
      case SipRouteSet:
      {
          g_ulSipPortInfo = stCmd.sipalg_portinfo;
          if (0 != g_SipRouteListCount)
          {
              printk("SipRouteSet but g_SipRouteListCount not 0");
          }
          break;
      }
      /* clear sip route info, this is done when siproxd stop */
      case SipRouteClear:
      {
          struct SipRouteInfo* pstInfo = NULL;
          struct SipRouteInfo* tmp_entry = NULL;
          struct SipRouteInfoItem stInfo;
          list_for_each_entry_safe(pstInfo, tmp_entry, &g_SipRouteList, list)
          {
              list_del(&pstInfo->list);
              if (NULL != pstInfo->stRouteInfo.call_id)
              {
                  kfree(pstInfo->stRouteInfo.call_id);
                  pstInfo->stRouteInfo.call_id = NULL;
              }
              kfree(pstInfo);
              pstInfo = NULL;
              g_SipRouteListCount--;
          }
          if (0 != g_SipRouteListCount)
          {
              printk("after SipRouteClear g_SipRouteListCount still not 0 g_SipRouteListCount = %d\n", g_SipRouteListCount);
              g_SipRouteListCount = 0;
          }
          g_ulSiproxdStatus = ALG_SIP_START_FAIL;
          break;
      }
      /* record siproxd status, because RecordSipContent should be done only when siproxd is started */
      case SipRouteStart:
      {
          g_ulSiproxdStatus = ALG_SIP_START_SUCCESS;
          break;
      }
      default:
      {
          printk("stCmd.CmdType unknown g_SipRouteListCount = %d\n", g_SipRouteListCount);
          break;
      }
    }

    return 0;
}


unsigned int atp_sipalg_getsock(struct sk_buff* skb, int optname, char __user *optval)
{
	struct SipRouteInfo* pstInfo = NULL;
    struct SipRouteInfo* tmp_entry = NULL;
    struct SipRouteInfoItem stInfo;
	
   if (!lSipInfoInited)
    {
      INIT_LIST_HEAD(&g_SipRouteList);
      spin_lock_init(&g_SipRouteLock);
      lSipInfoInited = 1;
    }
	
    if (copy_from_user(&stInfo, optval, sizeof(struct SipRouteInfoItem)))
    {
        return -EFAULT;
    }
	
    /* require destination of sip packet by call-id */
    spin_lock_bh(&g_SipRouteLock);

    list_for_each_entry_safe(pstInfo, tmp_entry, &g_SipRouteList, list)
    {

        if (NULL != pstInfo->stRouteInfo.call_id)
        {

            if (0 == strncmp(pstInfo->stRouteInfo.call_id, stInfo.call_id, strlen(stInfo.call_id)))
            {
                stInfo.daddr = pstInfo->stRouteInfo.daddr;
                stInfo.dport = pstInfo->stRouteInfo.dport;
                stInfo.proto= pstInfo->stRouteInfo.proto;
                spin_unlock_bh(&g_SipRouteLock);
                if (copy_to_user(optval,&stInfo,sizeof(struct SipRouteInfoItem)))
                {
                    return -EFAULT;
                }
                list_del(&pstInfo->list);
                kfree(pstInfo->stRouteInfo.call_id);
                pstInfo->stRouteInfo.call_id = NULL;
                kfree(pstInfo);
                pstInfo = NULL;
                g_SipRouteListCount--;

                return 0;
            }
        }
    }
    spin_unlock_bh(&g_SipRouteLock);
	
    return -EFAULT;
}
