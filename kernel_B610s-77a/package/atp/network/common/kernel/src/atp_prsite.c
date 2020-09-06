/* (C) 2001-2002 Magnus Boden <mb@ozaba.mine.nu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Version: 0.0.7
 *
 * Thu 21 Mar 2002 Harald Welte <laforge@gnumonks.org>
 * 	- Port to newnat API
 *
 * This module currently supports DNAT:
 * iptables -t nat -A PREROUTING -d x.x.x.x -j DNAT --to-dest x.x.x.y
 *
 * and SNAT:
 * iptables -t nat -A POSTROUTING { -j MASQUERADE , -j SNAT --to-source x.x.x.x }
 *
 * It has not been tested with
 * -j SNAT --to-source x.x.x.x-x.x.x.y since I only have one external ip
 * If you do test this please let me know if it works or not.
 *
 */

#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#include <linux/netfilter.h>
#include <net/checksum.h>
#include <net/tcp.h>
#include <linux/ctype.h>
#include "atp_prsite.h"
#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03)
#include "atp_guestwifiaccessui.h"
#endif

#define  IGNORESTRING  "/?updataredirect="
#define  SUFFIXLEN     8
#define  DATALENGTH    1500
#define  RANDTIMEOUT   5
#define  RANDLENGTH    11

DEFINE_SPINLOCK(prsiterand_lock);

#if 1
 #define PRSITE_DEBUGP(format, args...) printk(format, ## args)
#else
 #define PRSITE_DEBUGP(format, args...)
#endif
#if 1
 #define PRSITE_STATIC_DEBUGP(format, args...) do {printk(format, ## args);} while (0)
#else
 #define PRSITE_STATIC_DEBUGP(format, args...)
#endif
#if 0
extern int (*prsite_add_hook)(char __user * optval);
extern int (*prsite_del_hook)(char __user * optval);
#endif

#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03)
extern struct guestwifiaccessui_info g_stGuestwifiAccessuiInfo;
#endif

/*重定向最大次数*/
#define PRSITEMAXNUM   50
int g_prsitenum = 0;


struct prsite_url_info g_stPrsiteUrlInfo = {0};

#define MAXIPCACHE             (64)
#define MULTIUSER_MAC_LEN      (17)
#define MAC_BIT_0               0
#define MAC_BIT_1               1
#define MAC_BIT_2               2
#define MAC_BIT_3               3
#define MAC_BIT_4               4
#define MAC_BIT_5               5

DEFINE_SPINLOCK(multiuser_lock);

typedef struct tag_multiuser_mac_hash_info_st
{
    struct hlist_node i_hash;
    char              macaddr[MULTIUSER_MAC_LEN + 1];
    time_t            tv;
}multiuser_mac_hash_info_st;

struct hlist_head g_machash[MAXIPCACHE];

/******************************************************************************
  函数名称  : get_machash
  功能描述  : 根据mac地址获取哈希值
  输入参数  : char* macaddr 合法性需要由调用处保证
  调用函数  : 
  被调函数  : 
  输出参数  : 
  返 回 值  : 哈希值
******************************************************************************/
static inline unsigned short get_machash(char* macaddr)
{
    unsigned int hash = jhash(macaddr, strlen(macaddr), 0) % MAXIPCACHE;
    return hash;
}

/******************************************************************************
  函数名称  : multiuser_find_mac_hashlist
  功能描述  : 根据mac查找哈希记录，返回组节点
  输入参数  : char *macaddr mac地址
  调用函数  :
  被调函数  : 
  输出参数  : 
  返 回 值  : 组节点指针
******************************************************************************/
static inline multiuser_mac_hash_info_st *multiuser_find_mac_hashlist(char *macaddr)
{
    multiuser_mac_hash_info_st*  pipnode = NULL;
    struct hlist_node* hp = NULL;
    struct hlist_node* htmp = NULL;
    unsigned int hash = 0;

    if (NULL == macaddr)
    {
        PRSITE_DEBUGP("multiuser_find_mac_hashlist macaddr NULL\n");
        return NULL;
    }

    if (MULTIUSER_MAC_LEN != strlen(macaddr))
    {
        PRSITE_DEBUGP("multiuser_find_mac_hashlist strlen(macaddr) %d\n", strlen(macaddr));
        return NULL;
    }
    
    /* 根据mac遍历组信息 */
    hash = get_machash(macaddr);
    spin_lock_bh(&multiuser_lock);
    if (hlist_empty(&g_machash[hash]))
    {
        spin_unlock_bh(&multiuser_lock);
        return NULL;
    }
    hlist_for_each_safe(hp, htmp, &g_machash[hash])
    {
        /* 获取组节点 */
        pipnode = hlist_entry(hp, multiuser_mac_hash_info_st, i_hash);
        if (pipnode 
            && (0 == strcmp(pipnode->macaddr, macaddr)))
        {
            spin_unlock_bh(&multiuser_lock);
            return pipnode;
        }
    }
    spin_unlock_bh(&multiuser_lock);
    return NULL;
}

/******************************************************************************
  函数名称  : multiuser_add_mac_hashlist
  功能描述  : 根据mac地址增加对应的记录
  输入参数  : char *macaddr mac地址
  调用函数  :
  被调函数  : 
  输出参数  : 
  返 回 值  : 无
******************************************************************************/
static void multiuser_add_mac_hashlist(char* macaddr)
{
    unsigned int hash = 0;
    multiuser_mac_hash_info_st*  pipnode = NULL;
    struct timespec ts; 
    memset(&ts, 0, sizeof(ts));

    if (0 == g_stPrsiteUrlInfo.lMultiuserEnable)
    {
        PRSITE_DEBUGP("multiuser_add_mac_hashlist multiuser not enable\n");
        return;
    }
        
    if (NULL == macaddr)
    {
        PRSITE_DEBUGP("multiuser_add_mac_hashlist macaddr NULL\n");
        return;
    }

    if (MULTIUSER_MAC_LEN != strlen(macaddr))
    {
        PRSITE_DEBUGP("multiuser_add_mac_hashlist strlen(macaddr) %d\n", strlen(macaddr));
        return;
    }

    pipnode = multiuser_find_mac_hashlist(macaddr);
    if (NULL != pipnode)
    {
        /* 已经存在 */
        PRSITE_DEBUGP("multiuser_add_mac_hashlist already exist\n");
    }
    else
    {
        /* 根据mac遍历组信息 */
        hash = get_machash(macaddr);
        pipnode = kmalloc(sizeof(multiuser_mac_hash_info_st), GFP_ATOMIC);
        if (NULL == pipnode)
        {
            PRSITE_DEBUGP("multiuser_add_mac_hashlist malloc error \n");
            return;
        }

        do_posix_clock_monotonic_gettime(&ts);

        strncpy(pipnode->macaddr, macaddr, sizeof(pipnode->macaddr)-1);
        pipnode->tv = ts.tv_sec;
        
        spin_lock_bh(&multiuser_lock);
        hlist_add_head(&pipnode->i_hash, &g_machash[hash]);
        spin_unlock_bh(&multiuser_lock);

        PRSITE_DEBUGP("multiuser_add_mac_hashlist add hash info success \n");
    }
}

/******************************************************************************
  函数名称  : multiuser_del_mac_hashlist
  功能描述  : 根据mac删除对应的记录
  输入参数  : char *macaddr mac地址
  调用函数  :
  被调函数  : 
  输出参数  : 
  返 回 值  : 无
******************************************************************************/
static void multiuser_del_mac_hashlist(char *macaddr)
{
    struct hlist_node* hp = NULL;
    struct hlist_node* htmp = NULL;
    unsigned int hash = 0;
    multiuser_mac_hash_info_st*  pipnode = NULL;

    if (0 == g_stPrsiteUrlInfo.lMultiuserEnable)
    {
        PRSITE_DEBUGP("multiuser_del_mac_hashlist multiuser not enable\n");
        return;
    }

    if (NULL == macaddr)
    {
        PRSITE_DEBUGP("multiuser_del_mac_hashlist macaddr NULL\n");
        return;
    }

    if (MULTIUSER_MAC_LEN != strlen(macaddr))
    {
        PRSITE_DEBUGP("multiuser_del_mac_hashlist strlen(macaddr) %d\n", strlen(macaddr));
        return;
    }

    /* 根据mac遍历组信息 */
    hash = get_machash(macaddr);
    spin_lock_bh(&multiuser_lock);
    if (hlist_empty(&g_machash[hash]))
    {
        spin_unlock_bh(&multiuser_lock);
        PRSITE_DEBUGP("multiuser_del_mac_hashlist no this MAC\n");
        return;
    }

    hlist_for_each_safe(hp, htmp, &g_machash[hash])
    {
        /* 获取组节点 */
        pipnode = hlist_entry(hp, multiuser_mac_hash_info_st, i_hash);
        if (pipnode 
            && (0 == strcmp(pipnode->macaddr, macaddr)))
        {
            PRSITE_DEBUGP("delmaclistbymac success\n");
            hlist_del(&pipnode->i_hash);
            kfree(pipnode);
            pipnode = NULL;
        }
    }
    spin_unlock_bh(&multiuser_lock);
    return;
}

/******************************************************************************
  函数名称  : delmaclistbymac
  功能描述  : 根据mac删除对应的记录,封装了一层供atp_proc调用
  输入参数  : char *mac mac地址
  调用函数  :
  被调函数  : 
  输出参数  : 
  返 回 值  : 无
******************************************************************************/
void delmaclistbymac(char* mac)
{
    if (NULL == mac)
    {
        PRSITE_DEBUGP("delmaclistbymac NULL\n");
        return;
    }
    multiuser_del_mac_hashlist(mac);
    return;
}

/******************************************************************************
  函数名称  : delmaclistbydev
  功能描述  : 根据mac删除对应的记录,封装了一层供br_fdb调用
  输入参数  : char *mac mac地址
              char *dev 设备名称，暂时保留，未来可以针对设备进行扩展
  调用函数  :
  被调函数  : 
  输出参数  : 
  返 回 值  : 无
******************************************************************************/
void delmaclistbydev(char* dev, char* mac)
{
    char macaddr[MULTIUSER_MAC_LEN + 1] = {0};
    if (NULL == dev || NULL == mac)
    {
        return;
    }
    if (0 == g_stPrsiteUrlInfo.lMultiuserEnable)
    {
        PRSITE_DEBUGP("delmaclistbydev Multiuser not Enable\n");
        return;
    }

    memset(macaddr, 0, sizeof(macaddr));
    snprintf(macaddr, sizeof(macaddr), 
        "%02x:%02x:%02x:%02x:%02x:%02x", 
        mac[MAC_BIT_0], 
        mac[MAC_BIT_1],
        mac[MAC_BIT_2],
        mac[MAC_BIT_3],
        mac[MAC_BIT_4],
        mac[MAC_BIT_5]);

    delmaclistbymac(macaddr);
    return;
}

/******************************************************************************
  函数名称  : showmaclist
  功能描述  : 显示MAC表
  输入参数  : 
  调用函数  :
  被调函数  : 
  输出参数  : 
  返 回 值  : 无
******************************************************************************/
void showmaclist()
{
    multiuser_mac_hash_info_st*  pipnode = NULL;
    struct hlist_node* hp = NULL;
    struct hlist_node* htmp = NULL;
    int i = 1;

    PRSITE_DEBUGP("**************show hashlist info******************\n");
    if (0 == g_stPrsiteUrlInfo.lMultiuserEnable)
    {
        PRSITE_DEBUGP("Multiuser not Enable\n");
        return;
    }
    spin_lock_bh(&multiuser_lock);
    for (i = 0; i < MAXIPCACHE; i++)
    {
        if (hlist_empty(&g_machash[i]))
        {
            continue;
        }

        hlist_for_each_safe(hp, htmp, &g_machash[i])
        {
            /* 获取组节点 */
            pipnode = hlist_entry(hp, multiuser_mac_hash_info_st, i_hash);
            PRSITE_DEBUGP("hash:%d %c%c:%c%c %ul\n", i, 
                pipnode->macaddr[MAC_BIT_0], 
                pipnode->macaddr[MAC_BIT_1], 
                pipnode->macaddr[MAC_BIT_3], 
                pipnode->macaddr[MAC_BIT_4],
                pipnode->tv);
        }
    }
    spin_unlock_bh(&multiuser_lock);
    return;
}



struct st_randinfo
{
    __be32 id;
    long   ts; //时间戳（update）
    struct list_head          randlist;
};

struct list_head  randomlist;
/*遵循ugw方案，只识别如下代理协议*/
static struct user_agent_list g_browser_list[] =
{
    {"mozilla", "",     0         },
    {"Opera", "",       0         },
    {"iCab","",         0         },
	{NULL, NULL ,       0         }
};

static char g_blackurlsuffix [][SUFFIXLEN] = 
{
    "gif","jpeg","tiff","ico",
    "xbm","xpm","png","erf",
    "jpg","jpe","rgb","svf",
    "tif","pict","bmp"
};
/*strstr大小写不敏感实现*/
static char *strcasestr(const char *s1, const char *s2)
{   
	register const char *s = s1;
    register const char *p = s2;
    do
    {        
        if (!*p) 
        {          
            return (char *) s1;;        
        }       
        if((*p == *s)          
            || (tolower(*((unsigned char *)p)) == tolower(*((unsigned char *)s))))
        {         
            ++p;            
            ++s;        
        } 
        else {            
            p = s2;         
            if (!*s) 
            {              
            	return NULL;            
            }           
            s = ++s1;       
        }   
    } while (1);
}



static int http_set_appdata(struct sk_buff *poldskb, void *pdata, int len)
{
    struct iphdr *iph = NULL;
    struct tcphdr *tcph = NULL;
    u_int32_t tcplen;
    u_int32_t datalen;
    //struct sk_buff *newskb = NULL;
    unsigned int taddr = 0;           /* Temporary IP holder */
    short tport = 0;
    u_int32_t tseq = 0;
    struct ethhdr *peth = NULL;
    unsigned char t_hwaddr[ETH_ALEN] = {0};
    //struct net_device *dev = NULL;
    struct sk_buff *pskb = NULL;
    int tmplen = 0;
    int tmptcplen = 0;
    struct sk_buff * skb2;

    if (NULL == poldskb || (NULL == pdata))
    {
        return 1;
    }
    
    pskb = skb_copy(poldskb, GFP_ATOMIC);
    
    if (NULL == pskb)
    {
        return 1;
    }
    iph = ip_hdr(pskb);
    
    if (NULL == iph)
    {
        kfree_skb(pskb);
        return 1;
    } 
    tcph = (struct tcphdr *)(pskb->data + iph->ihl * 4);
    /*if(NULL == tcph)
    {
        kfree_skb(pskb);
        return 1;
    }*/
#if 0    
    if (skb_shinfo(pskb)->frag_list)
    {
        newskb = skb_copy(pskb, GFP_ATOMIC);
        if (!newskb)
        {
            return 1;
        }

        kfree_skb(pskb);
        pskb = newskb;
    }
#endif
    if ((pskb->end - pskb->data - iph->ihl * 4 - tcph->doff * 4) < len)
    {   
        PRSITE_DEBUGP("data len %d expand len %d\n",
                       pskb->end - pskb->data - iph->ihl * 4 - tcph->doff * 4,len);
        skb2 = skb_copy_expand(pskb, 0, len, GFP_ATOMIC);
        if (NULL == skb2)
        {
            kfree_skb(pskb);
            return 1;
        }

        kfree_skb(pskb);
        pskb = skb2;
    }
    
    iph = ip_hdr(pskb);
    
    if (NULL == iph)
    {
        kfree_skb(pskb);
        return 1;
    } 
    tcph = (struct tcphdr *)(pskb->data + iph->ihl * 4);
    /*if(NULL == tcph)
      {
          kfree_skb(pskb);
          return 1;
      }*/

    tmplen = pskb->len - iph->ihl * 4 - sizeof(struct tcphdr);
    PRSITE_DEBUGP("tcp data %d \n",tmplen);
    memset(pskb->data + iph->ihl * 4 + sizeof(struct tcphdr),
           0, pskb->len - iph->ihl * 4 - sizeof(struct tcphdr));
      

    memcpy(pskb->data + iph->ihl * 4 + sizeof(struct tcphdr),
           pdata, len);

    tmptcplen = (tcph->doff * 4);
    tcph->doff	= sizeof(struct tcphdr) / 4;
    tport = tcph->source;
    tcph->source = tcph->dest;
    tcph->dest = tport;
    
    tseq = tcph->ack_seq;
  
    //tcph->ack_seq = htonl(ntohl(tcph->seq) + tcph->syn + tcph->fin +
	//			      pskb->len - ip_hdrlen(pskb) -
	//			      (tcph->doff << 2)); 
    tcph->ack_seq = htonl(ntohl(tcph->seq) + tcph->syn + tcph->fin +
				      pskb->len - ip_hdrlen(pskb) -
				      tmptcplen);                       
    tcph->seq = tseq;
    tcph->ack = 1;  
   // tcph->fin = 1;

    taddr = iph->saddr;
    iph->saddr = iph->daddr;
    iph->daddr = taddr;
    iph->ttl -= 1;

    iph->tot_len = htons(iph->ihl * 4 + sizeof(struct tcphdr) + len);
    iph->check = 0;
    iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
    /* fix checksum information */
    if (tmplen > len)
    {
        tcplen  = pskb->len - iph->ihl * 4 - (tmplen - len);    
    }
    else
    {
       tcplen  = pskb->len - iph->ihl * 4;
    }
    
    datalen = tcplen - tcph->doff * 4;
    pskb->csum = csum_partial((char *)tcph + tcph->doff * 4, datalen, 0);

    tcph->check = 0;
    tcph->check = tcp_v4_check(tcplen, iph->saddr, iph->daddr,
                               csum_partial((char *)tcph, tcph->doff * 4,
                                            pskb->csum));

    //pskb->pkt_type = PACKET_OTHERHOST;
    pskb->pkt_type = PACKET_OUTGOING;
//===================================


    pskb->data = (unsigned char *)pskb->mac_header;
    pskb->tail = pskb->data + ETH_HLEN  + sizeof(struct iphdr) + sizeof(struct tcphdr) + len;
    //pskb->tail = pskb->data + ETH_HLEN  + (iph->ihl * 4) + sizeof(struct tcphdr) + len;
    pskb->len = pskb->tail - pskb->data;
    peth = (struct ethhdr *)(pskb->mac_header);
    if (NULL == peth)
    {
        PRSITE_DEBUGP("eth_hdr error ======>\n");
        kfree_skb(pskb);
        return 1;
    }        
    if (pskb->tail > pskb->end)
    {
        PRSITE_DEBUGP("prsite deal with error,ignore this buf \n");
        kfree_skb(pskb);
        return 1;
    }
   
    memcpy(t_hwaddr, peth->h_dest, ETH_ALEN); 
    memcpy(peth->h_dest, peth->h_source, ETH_ALEN);  
    memcpy(peth->h_source, t_hwaddr, ETH_ALEN);
    
//==========================================    

    
    dev_queue_xmit(pskb);
    /*重定向计数，最大重定向50次，多用户强制门户除外*/
	if((0 == g_stPrsiteUrlInfo.lMultiuserEnable)
        && ( g_prsitenum < PRSITEMAXNUM ))
	{
        g_prsitenum ++;
	}
    return 0;
}
#if 0

/*增加强制门户节点*/
int prsite_add(char __user *optval)
{
    int ret = 0;
    struct affined_bind bind   = {0};

    PRSITE_DEBUGP("prsite_add ======>\n");
    if (copy_from_user(&bind, optval, sizeof(struct affined_bind)))
    {
        ret = 1;
        return ret;
    }

    if (strlen(bind.url) == 0)
    {
        g_stPrsiteUrlInfo.lEnable = 0;
        return ret;
    }
    else
    {
        g_stPrsiteUrlInfo.ul_lan_addr = ntohl(bind.addr);
        g_stPrsiteUrlInfo.ul_lan_mask = ntohl(bind.mask);
        strncpy(g_stPrsiteUrlInfo.ac_stb_url, bind.url, HTTP_URL_MAX);
        g_stPrsiteUrlInfo.lEnable = bind.flag;
    }

    PRSITE_DEBUGP("++++++ ul_lan_addr %u ul_lan_mask %u ac_stb_url %s g_stPrsiteUrlInfo.lEnable %d\n",
        g_stPrsiteUrlInfo.ul_lan_addr,g_stPrsiteUrlInfo.ul_lan_mask,g_stPrsiteUrlInfo.ac_stb_url,g_stPrsiteUrlInfo.lEnable);


   // PRSITE_STATIC_DEBUGP("\n\t add ipaddr <%u.%u.%u.%u>", NIPQUAD(g_stPrsiteUrlInfo.ul_lan_addr));
  //  PRSITE_STATIC_DEBUGP("\n\t add mask <%u.%u.%u.%u>", NIPQUAD(g_stPrsiteUrlInfo.ul_lan_mask));
    return ret;
}

/*删除强制门户节点*/
int prsite_del(char __user *optval)
{
    PRSITE_DEBUGP("prsite_del ======>\n");
    memset(&g_stPrsiteUrlInfo,0,sizeof(g_stPrsiteUrlInfo));
    return 0;
}

//atoi实现原型
int s_atoi(const char *nptr)
{
    int c;              /* current char */
    int total;         /* current total */
    int sign;           /* if '-', then negative, otherwise positive */

    /* skip whitespace */
    while ( isspace((int)(unsigned char)*nptr) )
    {
        ++nptr;
    }
    c = (int)(unsigned char)*nptr++;
    sign = c;           /* save sign indication */
    if ('-' == c || '+' == c)
    {
        c = (int)(unsigned char)*nptr++;    /* skip sign */
    }

    total = 0;

    while (isdigit(c)) 
    {
        total = 10 * total + (c - '0');     /* accumulate digit */
        c = (int)(unsigned char)*nptr++;    /* get next char */
    }

    if ('-' == sign)
    {
        return -total;
    }
    else
    {
        return total;   /* return result, negated if necessary */
    }
}
#endif


static int checkuser_agent_version(char *puser_agent)
{
    int i,j = 0;
    char *temp = NULL;
    char str_version[32] = {0};
    temp = puser_agent;
    for (i = 0; NULL != g_browser_list[i].key_type; i++)
    {
        //关键字在浏览器列表中
        if (NULL != strcasestr(puser_agent,g_browser_list[i].key_type))
        {
		    return 0;
        }
    }
    return 1;
}

static void checkrandtimeout(time_t tv)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;

	struct st_randinfo* listret  = NULL;

	list_for_each_safe(listp, listn, &randomlist)
	{
		listret = list_entry(listp, struct st_randinfo, randlist);
		if ((tv - listret->ts) >= RANDTIMEOUT)
        {
            list_del(listp);
            kfree(listret);
        }
	}
    return;
}
	

void delrandall(void)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
    struct st_randinfo* listret  = NULL;
	
	spin_lock_bh(&prsiterand_lock);
	list_for_each_safe(listp, listn, &randomlist)
	{
		listret = list_entry(listp, struct st_randinfo, randlist);
        list_del(listp);
        kfree(listret);
	}
	spin_unlock_bh(&prsiterand_lock);
    return;
}


void delrandbyid(char *pid)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
	struct st_randinfo* listret  = NULL;
	__be32 id = simple_strtoul(pid, NULL, 10);

	spin_lock_bh(&prsiterand_lock);
	list_for_each_safe(listp, listn, &randomlist)
	{
	    listret = list_entry(listp, struct st_randinfo, randlist);
	    if(listret->id == id)
	    {    

            list_del(listp);
            kfree(listret);
	    }
	}
	spin_unlock_bh(&prsiterand_lock);
    return;
}


int showrandall(char *buffer, size_t len)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
	struct st_randinfo* listret  = NULL;
    struct timespec ts; 
	int retlen = 0;
    memset(&ts, 0, sizeof(ts));
    do_posix_clock_monotonic_gettime(&ts);
	spin_lock_bh(&prsiterand_lock);
	list_for_each_safe(listp, listn, &randomlist)
	{
	    listret = list_entry(listp, struct st_randinfo, randlist);
		if (((ts.tv_sec - listret->ts) < RANDTIMEOUT)
			&& ((len - retlen) > RANDLENGTH))
        {
            retlen += sprintf(buffer + retlen, "%u\n", listret->id);
        }
	}
	spin_unlock_bh(&prsiterand_lock);
    return retlen;
}



static __be32 getrandomnum(void)
{
	struct st_randinfo *prandominfo = NULL;
	struct timespec ts; 
	memset(&ts, 0, sizeof(ts));
    do_posix_clock_monotonic_gettime(&ts);

	prandominfo = kmalloc(sizeof(struct st_randinfo), GFP_ATOMIC);
	if (NULL == prandominfo)
	{
		return 0;
	}
	get_random_bytes(&prandominfo->id, sizeof(__be32));
    prandominfo->ts = ts.tv_sec;
	spin_lock_bh(&prsiterand_lock);
	checkrandtimeout(ts.tv_sec);
    list_add(&prandominfo->randlist,&randomlist);
	spin_unlock_bh(&prsiterand_lock);
    return prandominfo->id;
}
#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03) && defined(CONFIG_ATP_GETINDEV)
extern int checkifnamelist(char *pname);
#endif

static int http_changeto_newurl(struct sk_buff *pskb, char *url)
{
    char http_newdata[HTTP_RESPONSE_BUF_MAX + 1] = {0};
    /*
    HTTP/1.1 302 Redirection\r\n
    Content-length: 0\r\n
    Location: http://192.168.1.1/html/update.html?updataredirect=10.14.10.153/onlineupdate/\r\n
    Connection: Close\r\n\r\n
     */
    //snprintf(http_newdata,HTTP_RESPONSE_BUF_MAX + 1,
    //    "%s\r\nContent-length: 0\r\nLocation: http://%s?updataredirect=%s\r\nConnection: Close\r\n\r\n",
    //    "HTTP/1.1 302 Redirection",g_stPrsiteUrlInfo.ac_stb_url,url);
#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03) && defined(CONFIG_ATP_GETINDEV)
    /*如果禁止客人wifi访问webUI*/

    /*如果从客人wifi过来的*/

    if (checkifnamelist(pskb->lanindev->name))
    {
        PRSITE_DEBUGP("http_changeto_newurl gusetwifi======>\n");
        snprintf(http_newdata, HTTP_RESPONSE_BUF_MAX + 1,
                 "%s\r\nContent-length: 0\r\nLocation: http://%s?randid=%u?updataredirect=%s\r\nConnection: Close\r\n\r\n",
                 "HTTP/1.1 307 Temporary Redirect",
                 g_stPrsiteUrlInfo.ac_stb_url_guestwifi,
                 getrandomnum(),
                 url);
    }
    else
    {
        PRSITE_DEBUGP("http_changeto_newurl ======>\n");
        snprintf(http_newdata, HTTP_RESPONSE_BUF_MAX + 1,
                 "%s\r\nContent-length: 0\r\nLocation: http://%s?randid=%u?updataredirect=%s\r\nConnection: Close\r\n\r\n",
                 "HTTP/1.1 307 Temporary Redirect",
                 g_stPrsiteUrlInfo.ac_stb_url,
                 getrandomnum(),
                 url);
    }

#else

    PRSITE_DEBUGP("http_changeto_newurl ======>\n");
    snprintf(http_newdata, HTTP_RESPONSE_BUF_MAX + 1,
             "%s\r\nContent-length: 0\r\nLocation: http://%s?randid=%u?updataredirect=%s\r\nConnection: Close\r\n\r\n",
             "HTTP/1.1 307 Temporary Redirect", g_stPrsiteUrlInfo.ac_stb_url, getrandomnum(), url);
#endif

    PRSITE_DEBUGP("HTTP ALG: new http response \n%s\n", http_newdata);

    return http_set_appdata(pskb, http_newdata, strlen(http_newdata));
}


static int checkurlsuffix(char *purl)
{
    char *p = purl; 
    char *ptmp = NULL;
    if (0 == strcmp("/",purl))
    {
        PRSITE_DEBUGP("checkurlsuffix+++++++++++++++\n");
        return 0;
    }
    else
    {
        ptmp = strrchr(p,'.');
        if(NULL == ptmp)
        {
            return 1;
        }
        else
        {
            ptmp += 1;
            if(0 == strcmp(ptmp,"jsp")
               || 0 == strcmp(ptmp,"asp")
               || 0 == strcmp(ptmp,"htm")
               || 0 == strcmp(ptmp,"xhtml")
               || 0 == strcmp(ptmp,"aspx")
               || 0 == strcmp(ptmp,"php")
               || 0 == strcmp(ptmp,"cgi"))
            {
                return 0;
            }
            else
            {
                if(0 == strcmp(ptmp,"html"))
                {
                    if(strstr(purl, "library/test/success"))
                    {
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                }
                return 1;
            }
        }
    }
}


static int checkblacksuffix(char *purl)
{
    /*使用ugw新方案，只关心uri最后一位为‘/’的报文才处理，其他忽略*/
	if( '/' == purl[strlen(purl) -1 ])
    {
	    return 0;
	}

	return 1;
}

static unsigned int http_prsite_info(unsigned char *pdata,unsigned char *purl,int useflag)
{
    unsigned char *p = NULL;
    unsigned char *ptmp = NULL;
    char uri[224]  = {0}; 
    char user_agent[224] = {0};   
    char host[64] = {0};
    int i = 0 ;
    p = pdata + 4;
    while (*p != ' ')
    {
        if (i >= 223)
        {
            return 1;
        }        
        uri[i] = *p;
        i++;
        p++;    
    }
    PRSITE_DEBUGP("uri %s\n", uri);
    if (strstr(uri,IGNORESTRING))
    {
        return 1;
    }

    if (checkblacksuffix(uri))
    {
        return 1;
    }
	
    if (2 == useflag)
    {
        if(checkurlsuffix(uri))
        {
            PRSITE_DEBUGP("checkurlsuffix not need======>\n");
            return 1;
        }
    }
    ptmp = strstr(p, "\r\nHost: "); 
    if (NULL == ptmp) 
    {
        PRSITE_DEBUGP("no HOST\n");
        return 1; 
    }
    ptmp = ptmp + strlen("\r\nHost: "); 
    i = 0;  
    while ((*ptmp != '\r') && (*ptmp != '\n'))
    {
        if (i >= 63)     
        {
            return 1; 
        }
        host[i] = *ptmp;   
        i++;
        ptmp++; 
    }
    PRSITE_DEBUGP("host %s\n", host);  
    ptmp = NULL;   
    ptmp = strstr(p, "\r\nUser-Agent: ");  
    if (NULL == ptmp) 
    {
        PRSITE_DEBUGP("User-Agent not need======>\n");
        return 1;     
    }
    ptmp = ptmp + strlen("\r\nUser-Agent: "); 

    i = 0;   
    while ((*ptmp != '\r') && (*ptmp != '\n'))  
    {
        if (i >= 223)  
        {
            user_agent[223] = 0;
            break;   
        }
        user_agent[i] = *ptmp;  
        i++;   
        ptmp++;
    }
     
    if(checkuser_agent_version(user_agent)) 
    {        
        PRSITE_DEBUGP("checkuser_agent,the boswer is not support======>\n");
        return 1; 
    }
    if (0 == strcmp("/",uri))   
    {
        snprintf(purl, HTTP_URL_MAX,"%s", host);   
    }    
    else  
    {     
        snprintf(purl, HTTP_URL_MAX,"%s%s", host, uri); 
    } 
    return 0;
}



unsigned int nf_prsite_in(struct sk_buff *skb, const struct net_device *pin, const struct net_device *pout)
{
    struct iphdr *iph = NULL;
    struct tcphdr *tcph = NULL;
    
    unsigned char url[HTTP_URL_MAX] = {0};
    int useflag = g_stPrsiteUrlInfo.lEnable;
    unsigned char *data = NULL;

    //multimac
    char          macaddr[MULTIUSER_MAC_LEN + 1] = {0};
    struct ethhdr *ethhdr = NULL;

    if (0 == g_stPrsiteUrlInfo.lMultiuserEnable)
    {
        if (!useflag)
        {
            //PRSITE_STATIC_DEBUGP("prsite is not enable\n");
            return NF_ACCEPT;
        }
    }
    /*当重定向次数大于上限，主动关闭重定向功能*/
    if(g_prsitenum >= PRSITEMAXNUM)
	{
	    return NF_ACCEPT;
	}
	
    
    iph = ip_hdr(skb);

    if (NULL == iph)
    {
        return NF_ACCEPT;   
    }
    
    if (iph->frag_off & htons(IP_OFFSET))	
    {
        return NF_ACCEPT;
    }

    if (iph->protocol != IPPROTO_TCP)
    {
        return NF_ACCEPT;   
    }

    tcph = (void *)iph + iph->ihl * 4;

    if (tcph->dest != htons(80))
    {
        return NF_ACCEPT; 
    }
    
    if ((iph->saddr == htonl(0x7F000001))||(iph->daddr == htonl(0x7F000001)))
    {
        PRSITE_DEBUGP("lo accpet ======>\n");
        return NF_ACCEPT; 
    }
    
    if ((iph->saddr == g_stPrsiteUrlInfo.ul_lan_addr)
        ||(iph->daddr == g_stPrsiteUrlInfo.ul_lan_addr)) 
    {          
        return NF_ACCEPT;   
    }   
    if ((iph->saddr & g_stPrsiteUrlInfo.ul_lan_mask) == (iph->daddr & g_stPrsiteUrlInfo.ul_lan_mask))  
    {     
        //PRSITE_STATIC_DEBUGP("prsite ipaddr is local address(lan)\n");    
        return NF_ACCEPT;  
    }

    if ((iph->saddr & g_stPrsiteUrlInfo.ul_lan_mask) != (g_stPrsiteUrlInfo.ul_lan_addr & g_stPrsiteUrlInfo.ul_lan_mask))
    {
        return NF_ACCEPT;
    } 
#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03) && defined(CONFIG_ATP_GETINDEV)

    if (NULL == skb->lanindev)
    { 
        return NF_ACCEPT;
    }
#endif  
    if (tcph->doff * 4 + iph->ihl * 4 == skb->len)
    {
        return NF_ACCEPT;
    }
    
    /*data = (void *)tcph + tcph->doff * 4;

    if (NULL == data)
    {
        return NF_ACCEPT;
    }*/

    if (strncmp((void *)tcph + tcph->doff * 4, HTTP_PROTO_GET, strlen(HTTP_PROTO_GET)) != 0)
    {
        return NF_ACCEPT;
    }
    
    //multimac
    if (1 == g_stPrsiteUrlInfo.lMultiuserEnable)
    {
        ethhdr = (struct ethhdr *)skb_mac_header(skb);
        if (NULL == ethhdr)
        {
            return NF_ACCEPT; 
        }
        memset(macaddr, 0, sizeof(macaddr));
        snprintf(macaddr, sizeof(macaddr), 
            "%02x:%02x:%02x:%02x:%02x:%02x", 
            ethhdr->h_source[MAC_BIT_0],
            ethhdr->h_source[MAC_BIT_1],
            ethhdr->h_source[MAC_BIT_2],
            ethhdr->h_source[MAC_BIT_3],
            ethhdr->h_source[MAC_BIT_4],
            ethhdr->h_source[MAC_BIT_5]);

        if (NULL != multiuser_find_mac_hashlist(macaddr))
        {
            return NF_ACCEPT;
        }
    }
    
    PRSITE_DEBUGP("skb->len %d - %d - %d = %d>\n", skb->len, tcph->doff * 4, iph->ihl * 4, skb->len - tcph->doff * 4 - iph->ihl * 4);
    if (skb->len - tcph->doff * 4 - iph->ihl * 4 < DATALENGTH - 1)
    {
        data = (unsigned char *)kmalloc(DATALENGTH, GFP_KERNEL);
        if (data == NULL)
        {
            return NF_ACCEPT;
        }
        strncpy(data, (void *)tcph + tcph->doff * 4, skb->len - tcph->doff * 4 - iph->ihl * 4);
    }
    else
    {
        return NF_ACCEPT;
    }
 
    if (http_prsite_info(data,url,useflag) == 1)
    {
        kfree(data);
        return NF_ACCEPT;
    }

    if (http_changeto_newurl(skb, url) == 1)
    {
        kfree(data);
        return NF_ACCEPT;
    }
    //multimac
    else if (1 == g_stPrsiteUrlInfo.lMultiuserEnable)
    {
        multiuser_add_mac_hashlist(macaddr);
    }

    /*重定向一次*/
    if (2 == useflag)
    {
        g_stPrsiteUrlInfo.lEnable = 0;
    }
    kfree(data);

    return NF_DROP;
}
static unsigned int ipv4_prsite_in(unsigned int hooknum,
				      struct sk_buff *skb,
				      const struct net_device *in,
				      const struct net_device *out,
				      int (*okfn)(struct sk_buff *))
{
   return nf_prsite_in(skb,in,out);
}


static struct nf_hook_ops atp_prsite_hook =
{
    .hook		= ipv4_prsite_in,
    .owner		= THIS_MODULE,
    .pf		= NFPROTO_IPV4,
    .hooknum	= NF_INET_PRE_ROUTING,
    .priority	= NF_IP_PRI_FIRST,
};

EXPORT_SYMBOL_GPL(nf_prsite_in);

static int __init   nf_prsite_init(void)
{
    int ret = 0;
    INIT_LIST_HEAD(&randomlist);

    //multimac
    int i = 0;
    for (i = 0; i < MAXIPCACHE; i++)
    {
        INIT_HLIST_HEAD(&g_machash[i]);
    }

    ret = nf_register_hook(&atp_prsite_hook);

    return ret;
}

static void __exit  nf_prsite_fini(void)
{
    nf_unregister_hook(&atp_prsite_hook);
    return;
}

module_init(nf_prsite_init);
module_exit(nf_prsite_fini);
