
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/in.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>
#include <linux/ctype.h>
#include <net/ip.h>
#include "nf_dnscache.h"

/* dns header */
struct stDnsMsgHdr
{
    uint16_t u16ID;     /* query identification number */
    uint16_t u16Coms;   /* flags */
    uint16_t u16QDcnt;  /* number of question entries */
    uint16_t u16ANcnt;  /* number of answer entries */
    uint16_t u16NScnt;  /* number of authority entries */
    uint16_t u16ARcnt;  /* number of resource entries */
}  __attribute__((packed));

struct st_whilematch_Record
{
    struct list_head list;
    char* pacUrl;      
};
typedef struct tagDNS_QUESTION_ST
{
    unsigned char acName[URL_STRING_LEN];
    unsigned int  ulType;
    unsigned int  ulClass;
} DNS_QUESTION_ST;

#define DNS_QUERY_CODE                   0x0
#define DNS_RESP_CODE                    0x1
#define DNS_FLAGS_QR                     0x8000    /* response flag */  
#define DOMIN_IP_LOOKBACK_INF           "lo"
#define DNS_NAME_COMPRESS_D             (0xC0)
#define DNS_CLASS_IN                     (1)
#define DNS_TTL_MAX                 (604800)

/* dns type */
#define DNS_TYPE_A          1
#define DNS_TYPE_NS         2
#define DNS_TYPE_CNAME      5
#define DNS_TYPE_SOA        6
#define DNS_TYPE_PTR        12
#define DNS_TYPE_AAAA       28

typedef enum tagDNS_RET_CODE_EN
{
    // These errors are reserved for dns only
    DNS_RET_OK              = 0x0,
    DNS_RET_PARA_ERR        = 0x1,
    DNS_RET_INTERNEL_ERR    = 0x2,
    DNS_RET_FAKE_PACKET     = 0x3,
    DNS_RET_NOT_MATCH       = 0x4,
    DNS_RET_FULL_MATCH      = 0x5,
    DNS_RET_PART_MATCH      = 0x6,
} DNS_RET_CODE_EN;


//模糊匹配缓存节点
struct st_dnsprobableinfo
{
    unsigned char            url[URL_STRING_LEN + 1];
    char *pmac;
    struct list_head         probable_list;
};

#if 0
#define DNSCACHE_DEBUGP(format, args...) do {printk("[%s:%d]\n", __FUNCTION__, __LINE__);printk(format, ## args);printk("\n");} while (0)
#else
#define DNSCACHE_DEBUGP(format, args...)
#endif


#define LANINTERFACE    "br"
#define LOCALINTERFACE  "lo"
#define DNSPORT          53
#define MAXDNSCACHE     5*32
#ifdef CONFIG_DNSCACHE_MAXDNSNUM
    #define MAXDNSNUM          CONFIG_DNSCACHE_MAXDNSNUM
#else
    #define MAXDNSNUM       1000
#endif

int g_dnscacheswitch = TURNOFF;
DEFINE_SPINLOCK(dnscache_lock);
int dnssum = 0;


struct hlist_head dnshash[MAXDNSCACHE];
struct list_head  urlmatchlist;
struct list_head  urlprobablematchlist;
struct list_head  unmatchwhileurlmatchlist;
char* inputurlinfo = NULL;
static inline struct st_Domainmap_Record* lookuphash(char* url);

static inline unsigned short dnsHash(unsigned char* url)
{
    return jhash(url, strlen(url), 0) % MAXDNSCACHE;
}
extern int g_filtertype ;

static inline char* geturlbyline(char* buf, char* urlbuf)
{
    int i = 0;
    char* purl = buf;
    while (*purl != ' ')
    {
        if ((*purl == ';')
        	   ||(*purl == '\0'))
        {
            return NULL;
        }
        urlbuf[i] = *purl;
        i++;
        purl++;
    }
    purl++;
    return purl;
}

static void addurlprobablematchlist(void)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
    struct st_DomainIP_Record*   ptmpiprecord = NULL;
    struct st_Domainmap_Record*  phashnode = NULL;
    struct st_dnsprobableinfo*   ptmpprobableinfo = NULL;
    int k = 0;
    if (!list_empty(&urlprobablematchlist))
    {
        for (k = 0; k < MAXDNSCACHE; k++)
        {
            struct hlist_node* hp = NULL;
            hlist_for_each(hp, &dnshash[k])
            {
                phashnode = hlist_entry(hp, struct st_Domainmap_Record, i_hash);
                if ( phashnode && phashnode->pacUrl )
                {
                    list_for_each_safe(listp, listn, &urlprobablematchlist)
                    {
                        ptmpprobableinfo = list_entry(listp, struct st_dnsprobableinfo, probable_list);
                        if (strstr(phashnode->pacUrl, ptmpprobableinfo->url))
                        {
                            // coverity[overwrite_var]
                            ptmpiprecord = kmalloc(sizeof(struct st_DomainIP_Record), GFP_ATOMIC);
                            if (ptmpiprecord)
                            {
                                // coverity[overwrite_var]
                                
                                ptmpiprecord->pnodeinfo = phashnode;
                                if(ptmpprobableinfo->pmac)
                                {
                                    ptmpiprecord->pmac = ptmpprobableinfo->pmac;
                                }	
                                else
                                {
                                    ptmpiprecord->pmac = NULL;
                                }
                                list_add(&ptmpiprecord->list, &urlmatchlist);
                            }
                        }
                    }
                }

            }
        }
        list_for_each_safe(listp, listn, &urlprobablematchlist)
        {
            ptmpprobableinfo = list_entry(listp, struct st_dnsprobableinfo, probable_list);
            list_del(listp);
            kfree(ptmpprobableinfo);
        }
    }
    // coverity[leaked_storage]
    return ;
}


static inline void delallwhileunmatchlist()
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
    struct st_whilematch_Record* ptmpwhiterecord = NULL;
	
    if (list_empty(&unmatchwhileurlmatchlist))
    {
        return;
    }
	
    if(FILTERTYPEWHITE == g_filtertype)
    {
        list_for_each_safe(listp, listn, &unmatchwhileurlmatchlist)
        {
            ptmpwhiterecord = list_entry(listp, struct st_whilematch_Record, list);
            list_del(listp);
		    if(ptmpwhiterecord->pacUrl)
		    {
		        kfree(ptmpwhiterecord->pacUrl);
		    }
            kfree(ptmpwhiterecord);
        }
    }
    return;
}


static inline void sycwhileunmatchlist(char* url)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
	struct st_Domainmap_Record* ptmprecord = NULL;
    struct st_DomainIP_Record* ptmpiprecord = NULL;
    struct st_whilematch_Record* ptmpwhiterecord = NULL;
	
	if (list_empty(&unmatchwhileurlmatchlist))
	{
	    return;
	}
	
    if(FILTERTYPEWHITE == g_filtertype)
    {
        list_for_each_safe(listp, listn, &unmatchwhileurlmatchlist)
        {
            ptmpwhiterecord = list_entry(listp, struct st_whilematch_Record, list);
			if((ptmpwhiterecord->pacUrl)&&(0 == strcmp(ptmpwhiterecord->pacUrl,url)))
			{
			    list_del(listp);
		        kfree(ptmpwhiterecord->pacUrl);		        
                kfree(ptmpwhiterecord);
				
				ptmprecord = lookuphash(url);
                if (ptmprecord)
                {
	                DNSCACHE_DEBUGP("find %s\n",url);
                    ptmpiprecord = kmalloc(sizeof(struct st_DomainIP_Record), GFP_ATOMIC);
		            if (NULL == ptmpiprecord)
		            {
		                return ;
		            }
                    ptmpiprecord->pnodeinfo = ptmprecord;
                    ptmpiprecord->pmac = NULL;
                    list_add(&ptmpiprecord->list, &urlmatchlist);
                }
                return;
            }
        }
    }
    return;
}

static inline void addwhileunmatchlist(char* url)
{
    struct st_whilematch_Record* ptmpwhiterecord = NULL;
    if (NULL == url)
    {
        return ;
    }
    if(FILTERTYPEWHITE == g_filtertype)
    {
        ptmpwhiterecord = kmalloc(sizeof(struct st_whilematch_Record), GFP_ATOMIC);
        if (NULL == ptmpwhiterecord)
        {
            return ;
        }
        ptmpwhiterecord->pacUrl = kmalloc(strlen(url) + 1, GFP_ATOMIC);
        if(NULL == ptmpwhiterecord->pacUrl)
        {
            kfree(ptmpwhiterecord);
            return ;
        }
        snprintf(ptmpwhiterecord->pacUrl, strlen(url) + 1, "%s", url);
        list_add(&ptmpwhiterecord->list, &unmatchwhileurlmatchlist);
    }
    return ;
}

static inline int addurlmatchlist(char* buf)
{
    struct st_Domainmap_Record* ptmprecord = NULL;
    struct st_DomainIP_Record* ptmpiprecord = NULL;
    char url[URL_STRING_LEN] = {0};
    char *pmac = NULL;
    int ret = 0;
    pmac = geturlbyline(buf, url);
    DNSCACHE_DEBUGP("addurlmatchlist %s\n", url);
    ptmprecord = lookuphash(url);
    if (ptmprecord)
    {
        DNSCACHE_DEBUGP("find %s\n", url);
        ptmpiprecord = kmalloc(sizeof(struct st_DomainIP_Record), GFP_ATOMIC);
        if(NULL == ptmpiprecord)
        {
            ret = -1;
            return ret;
        }
        if(pmac)
        {  
            DNSCACHE_DEBUGP("pmac %s\n", pmac);
            ptmpiprecord->pmac = kmalloc(strlen(pmac)+1, GFP_ATOMIC);
            if(NULL ==  ptmpiprecord->pmac)
            	{
            	    kfree(ptmpiprecord);
            	    ret = -1;
            	    return ret;
            	}
            snprintf(ptmpiprecord->pmac,strlen(pmac)+1,"%s",pmac);
        }
        else
        {
            ptmpiprecord->pmac = NULL;
        }
       	ptmpiprecord->pnodeinfo = ptmprecord;
        list_add(&ptmpiprecord->list, &urlmatchlist);
        ret = 1;
    }
	else
	{
	    if(FILTERTYPEWHITE == g_filtertype)
		{
	        addwhileunmatchlist(url);
			ret = 1;
	    }
	}
    // coverity[leaked_storage]
    return ret;
}

static void procaddmatchlist(char* pinfo)
{
    int i = 0;
    char buf[BUF_STRING_LEN] = {0};
    char* p = pinfo;
    char url[URL_STRING_LEN] = {0};
    char* pmac = NULL;
    struct st_dnsprobableinfo*   ptmpprobableinfo = NULL;
    while (*p)
    {
        if (*p == ';')
        {
            DNSCACHE_DEBUGP("addurlmatchlist %s\n", buf);
            if (0 == addurlmatchlist(buf))
            {
                memset(url, 0, sizeof(url));
                pmac = NULL;
                pmac = geturlbyline(buf, url);
                DNSCACHE_DEBUGP("url %s\n", url);
                ptmpprobableinfo = kmalloc(sizeof(struct st_dnsprobableinfo), GFP_ATOMIC);
                if (ptmpprobableinfo)
                {
                    ptmpprobableinfo->pmac = NULL;
                    snprintf(ptmpprobableinfo->url, URL_STRING_LEN + 1, "%s", url);
                    if (pmac)
                    {
                        DNSCACHE_DEBUGP("pmac %s\n", pmac);
                        ptmpprobableinfo->pmac = kmalloc(strlen(pmac) + 1, GFP_ATOMIC);
                        if (ptmpprobableinfo->pmac)
                        {
                            snprintf(ptmpprobableinfo->pmac, strlen(pmac) + 1, "%s", pmac);
                        }
                    }
                    list_add(&ptmpprobableinfo->probable_list, &urlprobablematchlist);
                }
            }
            memset(buf, 0, BUF_STRING_LEN);
            i = 0;
            p++;
            continue;
        }
        buf[i] = *p;
        i++;
        p++;
    }
    addurlprobablematchlist();
    DNSCACHE_DEBUGP("procaddmatchlist ok\n");
    // coverity[leaked_storage]
    return;
}
#define SHOW_IP_INFO(format, args...) printk(format, ## args)
static inline int showipinfoall()
{
    struct st_Domainmap_Record*  phashnode = NULL;
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
    struct dnsipv4info* list4ret  = NULL;
    struct dnsipv6info* list6ret  = NULL;
    int i = 0;
    int len = 0;
    char tempbuff[64] = {0};
    
    for (i = 0; i < MAXDNSCACHE; i++)
    {
        struct hlist_node* hp = NULL;
        hlist_for_each(hp, &dnshash[i])
        {
            phashnode = hlist_entry(hp, struct st_Domainmap_Record, i_hash);

            len += snprintf(tempbuff, sizeof(tempbuff),"%s:\n", phashnode->pacUrl);
            SHOW_IP_INFO("%s", tempbuff);
            list_for_each_safe(listp, listn, &phashnode->i_listv4)
            {
                list4ret = list_entry(listp, struct dnsipv4info, ipv4_list);
                len += snprintf(tempbuff, sizeof(tempbuff), "\t%d.%d.%d.%d\n", NIPQUAD(list4ret->dipv4));
                SHOW_IP_INFO("%s", tempbuff);
            }

            list_for_each_safe(listp, listn, &phashnode->i_listv6)
            {
                list6ret = list_entry(listp, struct dnsipv6info, ipv6_list);
                len += snprintf(tempbuff, sizeof(tempbuff), "\t%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6(list6ret->addrv6));
                SHOW_IP_INFO("%s", tempbuff);
            }
        }
    }
    return len;
}

static void deleteoldhashnode(void)
{
    struct st_Domainmap_Record*  phashnode = NULL;
    struct st_Domainmap_Record*  pret = NULL;
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
    struct dnsipv4info* list4ret  = NULL;
    struct dnsipv6info* list6ret  = NULL;
    unsigned int i = 0;
    unsigned int min = 0;
    for (i = 0; i < MAXDNSCACHE; i++)
    {
        struct hlist_node* hp = NULL;
        hlist_for_each(hp, &dnshash[i])
        {
            phashnode = hlist_entry(hp, struct st_Domainmap_Record, i_hash);
			if(0 == min)
			{
			    min = phashnode->ulTTL + phashnode->ts;
			}
			DNSCACHE_DEBUGP("phashnode->ulTTL %u,phashnode->ts %ld min %u\n",phashnode->ulTTL , phashnode->ts,min);
            if (min >= phashnode->ulTTL + phashnode->ts)
            {
                min = phashnode->ulTTL + phashnode->ts;
                pret = phashnode;
            }
        }
    }
    if (pret)
    {
	    DNSCACHE_DEBUGP("****delete hashnode %s *********\n",pret->pacUrl);
        list_for_each_safe(listp, listn, &pret->i_listv4)
        {
            list4ret = list_entry(listp, struct dnsipv4info, ipv4_list);
            list_del(listp);
            kfree(list4ret);
        }

        list_for_each_safe(listp, listn, &pret->i_listv6)
        {
            list6ret = list_entry(listp, struct dnsipv6info, ipv6_list);
            list_del(listp);
            kfree(list6ret);
        }
        hlist_del(&pret->i_hash);
        kfree(pret->pacUrl);
        kfree(pret);
    }
    return ;
}

static inline struct st_Domainmap_Record* lookuphash(char* url)
{
    struct st_Domainmap_Record*  phashnode = NULL;
    struct hlist_node* hp = NULL;
    unsigned int hash = 0;
    hash = dnsHash(url);
    hlist_for_each(hp, &dnshash[hash])
    {
        phashnode = hlist_entry(hp, struct st_Domainmap_Record, i_hash);
        if (0 == strcmp(phashnode->pacUrl, url))
        {
            return phashnode;
        }
    }
    return NULL;
}

static void deleteurlmatchlist(void)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
    struct st_DomainIP_Record* ptmpiprecord = NULL;
    list_for_each_safe(listp, listn, &urlmatchlist)
    {
        ptmpiprecord = list_entry(listp, struct st_DomainIP_Record, list);
        list_del(listp);
        if (ptmpiprecord->pmac)
        {
            kfree(ptmpiprecord->pmac);
        }
        kfree(ptmpiprecord);
    }
    return;
}

static void dnscache_changelower(unsigned char* pszUrl, unsigned int ulLen)
{
    int i = 0;

    for (i = 0; i < ulLen; i++)
    {
        pszUrl[i] = tolower(pszUrl[i]);
    }
}

static int lookuplist(unsigned int ip, struct st_Domainmap_Record* pinfo)
{
    struct list_head* list;
    struct dnsipv4info* ptmp = NULL;

    list_for_each(list, &pinfo->i_listv4)
    {
        ptmp = list_entry(list, struct dnsipv4info, ipv4_list);
        if (ptmp)
        { 
            if (ptmp->dipv4 == ip)
            {
                return 0;
            }
        }
    }
    return 1;
}

static inline int lookuplistv6(struct in6_addr* pipv6, struct st_Domainmap_Record* pinfo)
{
    struct list_head* list;
    struct dnsipv6info* ptmp = NULL;
   
    list_for_each(list, &pinfo->i_listv6)
    {
        ptmp = list_entry(list, struct dnsipv6info, ipv6_list);
        if (ptmp)
        {
            if (0 == memcmp(&ptmp->addrv6, pipv6, sizeof(struct in6_addr)))
            {
                return 0;
            }
        }
    }
    return 1;
}

int judge_ipcache_matchv6(struct in6_addr* pipv6,char *pmac)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
    struct st_DomainIP_Record* ptmpiprecord = NULL;
    list_for_each_safe(listp, listn, &urlmatchlist)
    {
        ptmpiprecord = list_entry(listp, struct st_DomainIP_Record, list);
        if (0 == lookuplistv6(pipv6, ptmpiprecord->pnodeinfo))
        {
            if (NULL == ptmpiprecord->pmac)
            {
                DNSCACHE_DEBUGP("judge_ipcache_matchv6 not for mac \n"); 
                return MATCHIP;
            }
            else
            {   
                DNSCACHE_DEBUGP("judge_ipcache_matchv6 for mac %s %s *********\n",ptmpiprecord->pmac, pmac); 
                if (strstr(ptmpiprecord->pmac, pmac))
                {
                    return MATCHIP;
                }
                else
                {
                    return NOMATCHIP;
                }
            }

        }
    }
    return NOMATCHIP;
}

int judge_ipcache_match( unsigned int ip,char *pmac)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
    struct st_DomainIP_Record* ptmpiprecord = NULL;
    list_for_each_safe(listp, listn, &urlmatchlist)
    {
        ptmpiprecord = list_entry(listp, struct st_DomainIP_Record, list);
        DNSCACHE_DEBUGP("==1111=======%s=======%s==\n", ptmpiprecord->pnodeinfo->pacUrl,pmac);
        if (0 == lookuplist(ip, ptmpiprecord->pnodeinfo))
        {
            /*不用mac匹配*/
            if (NULL == ptmpiprecord->pmac)
            {
                return MATCHIP;
            }
            else
            {
                DNSCACHE_DEBUGP("==2222=======%s====%s=====\n", ptmpiprecord->pmac,pmac);
                if (strstr(ptmpiprecord->pmac, pmac))
                {
                    return MATCHIP;
                }
                else
                {
                    return NOMATCHIP;
                }
            }
        }
    }
    return NOMATCHIP;
}

int dnscache_getipv4_by_Name(char *purl, struct list_head **pipv4)
{
    struct st_Domainmap_Record*  phashnode = NULL;
    if (NULL == purl)
    {
        return 1;
    }
    phashnode = lookuphash(purl);
    if (phashnode)
    {
        *pipv4 = &phashnode->i_listv4;
        return 0;
    }
    return 1;
}

void updatehashinfo(DNS_RECORDS_ST* pstRecord, time_t tv)
{
    struct st_Domainmap_Record*  phashnode = NULL;
    struct dnsipv4info* pdnsipv4info = NULL;
    struct dnsipv6info* pdnsipv6info = NULL;
    struct in6_addr addrv6;
    unsigned int hash = 0;
    int cleanflag = 0;
    phashnode = lookuphash(pstRecord->acName);
    //已经被记录
    if (phashnode)
    {
        phashnode->ts = tv ;
        phashnode->ulTTL = pstRecord->ulTTL;

        //查到是新ip记录添加到链表中
        if (DNS_TYPE_A == pstRecord->ulType)
        {
            DNSCACHE_DEBUGP("ip is [%d.%d.%d.%d]",NIPQUAD(*((unsigned int*)pstRecord->pucData)));            	
            if (lookuplist((*((unsigned int*)pstRecord->pucData)), phashnode))
            {
                pdnsipv4info = kmalloc(sizeof(struct dnsipv4info), GFP_ATOMIC);
                if (NULL == pdnsipv4info)
                {
                    DNSCACHE_DEBUGP("malloc error");
                    return ;
                }
                pdnsipv4info->dipv4 = (*((unsigned int*)pstRecord->pucData));

                list_add(&pdnsipv4info->ipv4_list, &phashnode->i_listv4);
            }
        }
        else if (DNS_TYPE_AAAA == pstRecord->ulType)
        {
            memcpy(&addrv6, pstRecord->pucData, pstRecord->ulDataLen);
            DNSCACHE_DEBUGP("ipv6 is [%d:%d:%d:%d:%d:%d:%d:%d]", NIP6(addrv6));
            if (lookuplistv6(&addrv6, phashnode))
            {
                pdnsipv6info = kmalloc(sizeof(struct dnsipv6info), GFP_ATOMIC);
                if (NULL == pdnsipv6info)
                {
                    DNSCACHE_DEBUGP("malloc error");
                    return ;
                }
				memcpy(&pdnsipv6info->addrv6, &addrv6, pstRecord->ulDataLen);
                list_add(&pdnsipv6info->ipv6_list, &phashnode->i_listv6);
            }
        }
    }
    else//新的记录
    {
        phashnode = kmalloc(sizeof(struct st_Domainmap_Record), GFP_ATOMIC);
        if (NULL == phashnode)
        {
            DNSCACHE_DEBUGP("malloc error");
            return ;
        }
        DNSCACHE_DEBUGP("pstRecord->acName %s",pstRecord->acName);
        hash = dnsHash(pstRecord->acName);

        phashnode->pacUrl = kmalloc(strlen(pstRecord->acName) + 1, GFP_ATOMIC);
        if (NULL == phashnode->pacUrl)
        {
            DNSCACHE_DEBUGP("malloc error");
            kfree(phashnode);
            return;
        }
        snprintf(phashnode->pacUrl, strlen(pstRecord->acName) + 1, "%s", pstRecord->acName);
        DNSCACHE_DEBUGP("phashnode->pacUrl %s",phashnode->pacUrl);
        phashnode->ts = tv ;
        phashnode->ulTTL = pstRecord->ulTTL;

        INIT_LIST_HEAD(&phashnode->i_listv4);
        INIT_LIST_HEAD(&phashnode->i_listv6);

        if (DNS_TYPE_A == pstRecord->ulType)
        {
            pdnsipv4info = kmalloc(sizeof(struct dnsipv4info), GFP_ATOMIC);
            if (NULL == pdnsipv4info)
            {
                DNSCACHE_DEBUGP("malloc error");
                kfree(phashnode->pacUrl);
                kfree(phashnode);
                return ;
            }
            pdnsipv4info->dipv4 = (*((unsigned int*)pstRecord->pucData));
            list_add(&pdnsipv4info->ipv4_list, &phashnode->i_listv4);
        }
        else if (DNS_TYPE_AAAA == pstRecord->ulType)
        {
            pdnsipv6info = kmalloc(sizeof(struct dnsipv6info), GFP_ATOMIC);
            if (NULL == pdnsipv6info)
            {
                DNSCACHE_DEBUGP("malloc error");
                kfree(phashnode->pacUrl);
                kfree(phashnode);
                return ;
            }
			DNSCACHE_DEBUGP("pstRecord->ulDataLen %d",pstRecord->ulDataLen);
			DNSCACHE_DEBUGP("%0x:%0x:%0x:%0x:%0x:%0x:%0x:%0x:%0x:%0x:%0x:%0x:%0x:%0x:%0x:%0x\n",
			pstRecord->pucData[0],pstRecord->pucData[1],pstRecord->pucData[2],pstRecord->pucData[3],
			pstRecord->pucData[4],pstRecord->pucData[5],pstRecord->pucData[6],pstRecord->pucData[7],
			pstRecord->pucData[8],pstRecord->pucData[9],pstRecord->pucData[10],pstRecord->pucData[11],
			pstRecord->pucData[12],pstRecord->pucData[13],pstRecord->pucData[14],pstRecord->pucData[15]);
            memcpy(&pdnsipv6info->addrv6, pstRecord->pucData, pstRecord->ulDataLen);
			DNSCACHE_DEBUGP("%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6(pdnsipv6info->addrv6));
            list_add(&pdnsipv6info->ipv6_list, &phashnode->i_listv6);
        }

        if (dnssum >= MAXDNSNUM)
        {
		    DNSCACHE_DEBUGP("dnssum %d MAXDNSNUM %d",dnssum,MAXDNSNUM);
            deleteoldhashnode();
            dnssum --;
            cleanflag = 1;
        }
        hlist_add_head(&phashnode->i_hash, &dnshash[hash]);
        if (cleanflag)
        {
            deleteurlmatchlist();
            delallwhileunmatchlist();
            if (inputurlinfo)
            {
                if (strlen(inputurlinfo))
                {
                    dnscache_changelower(inputurlinfo, strlen(inputurlinfo));
                    procaddmatchlist(inputurlinfo);
                }
            }
        }
		sycwhileunmatchlist(pstRecord->acName);
        dnssum ++;
    }
    return;
}

static unsigned int dnscache_GetName(const char* pcPacketHead, unsigned int ulPacketLen,
                                     const unsigned char* pucNamePos, unsigned char* pcNameBuf,
                                     unsigned int ulBufLen, unsigned int* pulNameLen, unsigned int bRecursive)
{
    unsigned int      ulCnt;
    unsigned int      ulTmp;
    unsigned int      ulMaxCnt;
    unsigned int      ulLabelLen;
    unsigned int      ulRealNameCnt;

    ulMaxCnt = (unsigned int)(pucNamePos - (unsigned char*)(pcPacketHead));
    if (ulMaxCnt > ulPacketLen)
    {
        return DNS_RET_FAKE_PACKET;
    }
    ulMaxCnt = ulPacketLen - ulMaxCnt;
    // Check if it is compressed name
    if (DNS_NAME_COMPRESS_D == (pucNamePos[0] & DNS_NAME_COMPRESS_D))
    {
        if (0 != bRecursive)
        {
            // Only one compress allowed
            return DNS_RET_FAKE_PACKET;
        }
        if (ulMaxCnt <= 2)
        {
            // Invalid packet
            return DNS_RET_FAKE_PACKET;
        }
        ulLabelLen = (unsigned int)(pucNamePos[0]);
        ulLabelLen &= (~DNS_NAME_COMPRESS_D);
        ulLabelLen = ulLabelLen << 8;
        ulLabelLen += pucNamePos[1];
        if (ulLabelLen > ulPacketLen)
        {
            return DNS_RET_FAKE_PACKET;
        }

        // Do it again
        pucNamePos = ((unsigned char*)(pcPacketHead)) + ulLabelLen;
        if (NULL != pulNameLen)
        {
            (*pulNameLen) = 2;
        }
        return dnscache_GetName(pcPacketHead, ulPacketLen, pucNamePos, pcNameBuf, ulBufLen, NULL, 1);
    }
    // Not compressed name, need to iterate
    ulCnt = 0;
    ulRealNameCnt = 0;
    while (ulCnt < ulMaxCnt)
    {
        ulLabelLen = (unsigned int)(pucNamePos[ulCnt]);
        ulCnt++;
        if (0 == ulLabelLen)
        {
            if (0 == ulRealNameCnt)
            {
                pcNameBuf[0] = '\0';
            }
            else
            {
                pcNameBuf[ulRealNameCnt - 1] = '\0';
            }
            if (NULL != pulNameLen)
            {
                (*pulNameLen) = ulCnt;
            }
            return DNS_RET_OK;
        }
        if (DNS_NAME_COMPRESS_D == (ulLabelLen & DNS_NAME_COMPRESS_D))
        {
            ulLabelLen &= (~DNS_NAME_COMPRESS_D);
            ulLabelLen = ulLabelLen << 8;
            ulLabelLen += pucNamePos[ulCnt];
            if (ulLabelLen > ulPacketLen)
            {
                return DNS_RET_FAKE_PACKET;
            }

            // Do it again
            if (DNS_RET_OK != dnscache_GetName(pcPacketHead, ulPacketLen, (((unsigned char*)(pcPacketHead)) + ulLabelLen),
                                               pcNameBuf + ulRealNameCnt, (ulBufLen - ulRealNameCnt), NULL, 1))
            {
                return DNS_RET_FAKE_PACKET;
            }
            return DNS_RET_OK;
        }
        // Check if it is invalid
        if ((ulLabelLen > URL_STRING_LEN) || (ulLabelLen > (ulMaxCnt - ulCnt)))
        {
            // Invalide packet
            return DNS_RET_FAKE_PACKET;
        }

        // Read this label
        ulTmp = 0;
        while (ulTmp < ulLabelLen)
        {
            //if (pucNamePos[ulCnt] < 0x21 || pucNamePos[ulCnt] > 0x7E || pucNamePos[ulCnt] == '.')
            // Just passthrough all characters
            if (pucNamePos[ulCnt] == '.')
            {
                return DNS_RET_FAKE_PACKET;
            }
            pcNameBuf[ulRealNameCnt] = (unsigned char)pucNamePos[ulCnt];
            ulRealNameCnt++;
            ulCnt++;
            ulTmp++;
            if (ulRealNameCnt >= ulBufLen)
            {
                return DNS_RET_FAKE_PACKET;
            }
        }
        pcNameBuf[ulRealNameCnt] = '.';
        ulRealNameCnt++;
        pcNameBuf[ulRealNameCnt] = '\0';
    }

    return DNS_RET_FAKE_PACKET;
}

unsigned int dnscache_getvaluesofquestion(const unsigned char* pcPacketHead, unsigned int ulPacketLen,
        const unsigned char* pucQuestionPos, DNS_QUESTION_ST* pstQuestion,
        unsigned int* pulQuestionLen)
{
    unsigned int      ulRetVal;
    unsigned int      ulMaxCnt;
    unsigned short    usValue;
    unsigned int      ulFieldLen;

    if (NULL != pulQuestionLen)
    {
        (*pulQuestionLen) = 0;
    }

    if ((NULL == pcPacketHead) || (NULL == pstQuestion) || (NULL == pucQuestionPos))
    {
        return DNS_RET_PARA_ERR;
    }

    ulRetVal = dnscache_GetName(pcPacketHead, ulPacketLen, pucQuestionPos, pstQuestion->acName, sizeof(pstQuestion->acName), &ulFieldLen, 0);
    if (DNS_RET_OK != ulRetVal)
    {
        return ulRetVal;
    }
    pucQuestionPos += ulFieldLen;  // Skip Name field

    // Check if it has run out of the packet
    ulMaxCnt = (unsigned int)(pucQuestionPos - (unsigned char*)(pcPacketHead));
    if ((ulMaxCnt + 4) > ulPacketLen)
    {
        return DNS_RET_FAKE_PACKET;
    }

    // Get Type field
    memcpy((void*)(&usValue), (void*)pucQuestionPos, sizeof(unsigned short));
    pstQuestion->ulType = (unsigned int)(ntohs(usValue));
    pucQuestionPos += 2;

    // Get Class field
    memcpy((void*)(&usValue), (void*)pucQuestionPos, sizeof(unsigned short));
    pstQuestion->ulClass = (unsigned int)(ntohs(usValue));

    if (NULL != pulQuestionLen)
    {
        (*pulQuestionLen) = ulFieldLen + 4;
    }
    return DNS_RET_OK;
}

static unsigned int dnscache_addrecordtoList(DNS_RECORDS_ST* pstRecord)
{
    struct timespec ts;             /* 系统启动后的时间*/

    if (NULL == pstRecord)
    {
        return DNS_RET_PARA_ERR;
    }

    memset(&ts, 0, sizeof(ts));
    /* 获取系统启动时间*/
    do_posix_clock_monotonic_gettime(&ts);
    DNSCACHE_DEBUGP("current time is [%ld]", ts.tv_sec);

    spin_lock_bh(&dnscache_lock);
    updatehashinfo(pstRecord, ts.tv_sec);
    spin_unlock_bh(&dnscache_lock);
    return DNS_RET_OK;
}

static unsigned int dnscache_packetgetvaluesofrr(const unsigned char* pcPacketHead, unsigned int ulPacketLen,
        const unsigned char* pucRRPos, DNS_RECORDS_ST* pstRR,
        unsigned int* pulRRLen)
{
    unsigned int      ulRetVal;
    unsigned int      ulMaxCnt;
    unsigned short      usValue;
    unsigned int      ulValue;
    unsigned int      ulFieldLen;

    if (NULL != pulRRLen)
    {
        (*pulRRLen) = 0;
    }

    if ((NULL == pcPacketHead) || (NULL == pstRR) || (NULL == pucRRPos))
    {
        return DNS_RET_PARA_ERR;
    }

    ulRetVal = dnscache_GetName(pcPacketHead, ulPacketLen, pucRRPos, pstRR->acName, sizeof(pstRR->acName), &ulFieldLen, 0);
    if (DNS_RET_OK != ulRetVal)
    {
        return ulRetVal;
    }
    pucRRPos += ulFieldLen;  // Skip Name field

    // Check if it has run out of the packet
    ulMaxCnt = (unsigned int)(pucRRPos - (unsigned char*)(pcPacketHead));
    if ((ulMaxCnt + 10) > ulPacketLen)
    {
        return DNS_RET_FAKE_PACKET;
    }

    // Get Type field
    memcpy((void*)(&usValue), (void*)pucRRPos, sizeof(unsigned short));
    pstRR->ulType = (unsigned int)(ntohs(usValue));
    pucRRPos += 2;

    // Get Class field
    memcpy((void*)(&usValue), (void*)pucRRPos, sizeof(unsigned short));
    pstRR->ulClass = (unsigned int)(ntohs(usValue));
    pucRRPos += 2;

    // Get TTL field
    memcpy((void*)(&ulValue), (void*)pucRRPos, sizeof(unsigned int));
    pstRR->ulTTL = (unsigned int)(ntohl(ulValue));
    pucRRPos += 4;

    // Get Data Length field
    memcpy((void*)(&usValue), (void*)pucRRPos, sizeof(unsigned short));
    pstRR->ulDataLen = (unsigned int)(ntohs(usValue));
    pucRRPos += 2;

    // Check if it has run out of the packet
    ulMaxCnt = (unsigned int)(pucRRPos - (unsigned char*)(pcPacketHead));
    if ((ulMaxCnt + pstRR->ulDataLen) > ulPacketLen)
    {
        return DNS_RET_FAKE_PACKET;
    }

    pstRR->pucData = pucRRPos;
    if (NULL != pulRRLen)
    {
        (*pulRRLen) = ulFieldLen + pstRR->ulDataLen + 10;
    }
    return DNS_RET_OK;
}



void updatecnamehashinfo(DNS_RECORDS_ST* pstRecord, time_t tv)
{
    struct st_Domainmap_Record*  phashnode = NULL;
    struct dnsipv4info* pdnsipv4info = NULL;
    struct dnsipv6info* pdnsipv6info = NULL;
    struct in6_addr addrv6;
    unsigned int hash = 0;
    int cleanflag = 0;
    phashnode = lookuphash(pstRecord->acName);

    if (phashnode)
    {
        return ;
    }
    else
    {
        phashnode = kmalloc(sizeof(struct st_Domainmap_Record), GFP_ATOMIC);
        if (NULL == phashnode)
        {
            DNSCACHE_DEBUGP("malloc error");
            return ;
        }
        DNSCACHE_DEBUGP("pstRecord->acName %s",pstRecord->acName);
        hash = dnsHash(pstRecord->acName);

        phashnode->pacUrl = kmalloc(strlen(pstRecord->acName) + 1, GFP_ATOMIC);
        if (NULL == phashnode->pacUrl)
        {
            DNSCACHE_DEBUGP("malloc error");
            kfree(phashnode);
            return;
        }
        snprintf(phashnode->pacUrl, strlen(pstRecord->acName) + 1, "%s", pstRecord->acName);
        DNSCACHE_DEBUGP("phashnode->pacUrl %s",phashnode->pacUrl);
        phashnode->ts = tv ;
        phashnode->ulTTL = pstRecord->ulTTL;

        INIT_LIST_HEAD(&phashnode->i_listv4);
        INIT_LIST_HEAD(&phashnode->i_listv6);

        if (dnssum >= MAXDNSNUM)
        {
		          DNSCACHE_DEBUGP("dnssum %d MAXDNSNUM %d",dnssum,MAXDNSNUM);
            deleteoldhashnode();
            dnssum --;
            cleanflag = 1;
        }
        hlist_add_head(&phashnode->i_hash, &dnshash[hash]);
        if (cleanflag)
        {
            deleteurlmatchlist();
            delallwhileunmatchlist();
            if (inputurlinfo)
            {
                if (strlen(inputurlinfo))
                {
                    dnscache_changelower(inputurlinfo, strlen(inputurlinfo));
                    procaddmatchlist(inputurlinfo);
                }
            }
        }
		sycwhileunmatchlist(pstRecord->acName);
        dnssum ++;
    }
    return ;
}


static unsigned int dnscache_addcnamerecordtoList(DNS_RECORDS_ST* pstRecord)
{
    struct timespec ts;             /* 系统启动后的时间*/

    if (NULL == pstRecord)
    {
        return DNS_RET_PARA_ERR;
    }

    memset(&ts, 0, sizeof(ts));
    /* 获取系统启动时间*/
    do_posix_clock_monotonic_gettime(&ts);
    DNSCACHE_DEBUGP("current time is [%ld]", ts.tv_sec);
    spin_lock_bh(&dnscache_lock);
    updatecnamehashinfo(pstRecord, ts.tv_sec);
    //dnscache_pushdnscache(pstRecord);
    spin_unlock_bh(&dnscache_lock);
    return DNS_RET_OK;
}



static unsigned int dnscache_recordanswer(const unsigned char* pcPacketHead, unsigned int ulPacketLen,
        const unsigned char* pucAnswerPos, unsigned int ulAnCnt)
{
    unsigned int i = 0;         /* 循环变量*/
    unsigned int ulRRLen = 0;   /* 每个回复的长度*/
    DNS_RECORDS_ST stRecord;
    unsigned int ulRet = 0;
    unsigned int ulFind = 0;
    unsigned int ulCnameTTl = 0;
    unsigned int ulmaxttl = 0;
    int cnameflag = 0;
    unsigned char acName[URL_STRING_LEN]= {0};
    /* 依次取出Answer*/
    for (i = 0; i < ulAnCnt; i++)
    {
        pucAnswerPos += ulRRLen;
        memset(&stRecord, 0, sizeof(stRecord));
        ulRet = dnscache_packetgetvaluesofrr(pcPacketHead, ulPacketLen, pucAnswerPos, &stRecord, &ulRRLen);

        if (DNS_RET_OK != ulRet)
        {
            DNSCACHE_DEBUGP("get RR error at cnt [%d]", i);
            return DNS_RET_FAKE_PACKET;
        }
        DNSCACHE_DEBUGP("get RR at cnt [%d]\n name [%s]\n class [%d]\n type [%d]\n ttl [%d]\n datalen [%d]",
                        i, stRecord.acName, stRecord.ulClass, stRecord.ulType, stRecord.ulTTL, stRecord.ulDataLen);
        /* CNAME的TTL时间比A长，避免应用不发送DNS更新查询*/
        if ((DNS_TYPE_CNAME == stRecord.ulType)
        	    && (DNS_CLASS_IN == stRecord.ulClass)
        	    && (0 == cnameflag))
        {
            if (ulCnameTTl < stRecord.ulTTL)
            {
                ulCnameTTl = stRecord.ulTTL;
            }
            cnameflag = 1;
			dnscache_changelower(stRecord.acName,strlen(stRecord.acName));
            DNSCACHE_DEBUGP("type cname add url %s\n",stRecord.acName);
            dnscache_addcnamerecordtoList(&stRecord);
            snprintf(acName, URL_STRING_LEN, "%s", stRecord.acName);
        }
        /* CLASS_IN TYPE A /AAAA 且TTL非异常的才添加记录*/
        else if (((DNS_TYPE_A == stRecord.ulType) || (DNS_TYPE_AAAA == stRecord.ulType))
                 && (DNS_CLASS_IN == stRecord.ulClass)
                 && (DNS_TTL_MAX > stRecord.ulTTL))
        {
            if (ulmaxttl < stRecord.ulTTL)
            {
                ulmaxttl = stRecord.ulTTL;
            }
            stRecord.ulTTL = ulmaxttl;
            if ((stRecord.ulTTL < ulCnameTTl) && (ulCnameTTl < DNS_TTL_MAX))
            {
                stRecord.ulTTL = ulCnameTTl;
            }
            DNSCACHE_DEBUGP("need add to list");
            if(1 == cnameflag)
            {
                if(strlen(acName))
                {
                    memset(stRecord.acName,0,sizeof(stRecord.acName));   
                    snprintf(stRecord.acName,URL_STRING_LEN,"%s",acName);
                }
            }
			DNSCACHE_DEBUGP("name add url begin %s\n",stRecord.acName);
			dnscache_changelower(stRecord.acName,strlen(stRecord.acName));
			DNSCACHE_DEBUGP("name add url end %s\n",stRecord.acName);
            dnscache_addrecordtoList(&stRecord);
            ulFind = 1;
        }
    }

    if (ulFind)
    {
        return DNS_RET_OK;
    }
    return DNS_RET_NOT_MATCH;
}
static unsigned int dnscache_getquestion(const unsigned char* pcPacketHead, unsigned int ulPacketLen,
        const unsigned char* pucQuestionPos, DNS_QUESTION_ST* pstQuestion,
        unsigned int* pulQuestionLen)
{
    unsigned int ulRet = 0;


    if ((NULL == pcPacketHead) || (NULL == pstQuestion) || (NULL == pucQuestionPos))
    {
        return DNS_RET_PARA_ERR;
    }

    ulRet = dnscache_getvaluesofquestion(pcPacketHead, ulPacketLen, pucQuestionPos, pstQuestion, pulQuestionLen);
    if (DNS_RET_OK != ulRet)
    {
        DNSCACHE_DEBUGP("get value of question error");
        return DNS_RET_PARA_ERR;
    }

    DNSCACHE_DEBUGP("Question name is [%s] type is [%d] class is[%d]\n",
                    pstQuestion->acName, pstQuestion->ulType, pstQuestion->ulClass);

    if  ((DNS_CLASS_IN != pstQuestion->ulClass)
	       ||((DNS_TYPE_A != pstQuestion->ulType)
	          &&(DNS_TYPE_AAAA != pstQuestion->ulType)
	          &&(DNS_TYPE_CNAME != pstQuestion->ulType)))
    {
        DNSCACHE_DEBUGP("type or value not match we not need care");
        return DNS_RET_PARA_ERR;
    }

    /* 转换为小写域名*/
    dnscache_changelower(pstQuestion->acName, strlen(pstQuestion->acName));

    return DNS_RET_OK;
}



/*检查畸形报文和不需要的报文*/
static int dnscache_checkdata(struct stDnsMsgHdr* pstDnsHdr)
{
    /* 只处理查询报文 */
    if (!(ntohs(pstDnsHdr->u16Coms) & DNS_FLAGS_QR))
    {
        DNSCACHE_DEBUGP("no response packet......");
        return 0;
    }
    /* 只处理正常有回复内容的报文, 只有1个query*/
    if ((0 == ntohs(pstDnsHdr->u16ANcnt))
        || (1 != ntohs(pstDnsHdr->u16QDcnt))
        || (0 != (ntohs(pstDnsHdr->u16Coms) & 0x000F)))
    {
        DNSCACHE_DEBUGP("pstDnsHdr->u16ANcnt [%d]", ntohs(pstDnsHdr->u16ANcnt));
        DNSCACHE_DEBUGP("pstDnsHdr->u16QDcnt [%d]", ntohs(pstDnsHdr->u16QDcnt));
        DNSCACHE_DEBUGP("pstDnsHdr->u16Coms [%d]", ntohs(pstDnsHdr->u16Coms));
        DNSCACHE_DEBUGP("error response packet......");
        return 0;
    }
    return 1;
}


static int dnscache_type_record_v4(const struct sk_buff* skb)
{
    struct iphdr* iph = NULL;
    struct tcphdr* tcph = NULL;
    struct udphdr* udph = NULL;
    struct stDnsMsgHdr* pstDnsHdr = NULL;
    unsigned char* ptr = NULL;
    unsigned int ulPacketLen = 0;
    DNS_QUESTION_ST stQuestion;
    unsigned int ulQuestionLen = 0;
    unsigned int ulRet = 0;
    iph = ip_hdr(skb);

    if (NULL == iph)
    {
        return 0;
    }

    if (iph->frag_off & htons(IP_OFFSET))
    {
        return 0;
    }

  
  /* 不处理环回口和lan桥的报文*/
    if (NULL != skb->dev)
    {
        if (0 == strncmp(skb->dev->name,LANINTERFACE,strlen(LANINTERFACE)))    
        {
            //DNSCACHE_DEBUGP("skb dev is : [%s] \n", skb->dev->name);
            return 0;
        }
        if (0 == strncmp(skb->dev->name,LOCALINTERFACE,strlen(LOCALINTERFACE)))
        {
            //DNSCACHE_DEBUGP("skb dev is : [%s] \n", skb->dev->name);
            return 0;
        }
    }
	
    if (iph->protocol == IPPROTO_UDP)
    {
	    DNSCACHE_DEBUGP("\n");
        udph = (void*)iph + iph->ihl * 4;
        if (udph->source != htons(DNSPORT))
        {
            return 0;
        }

        //udpdata len
        ulPacketLen = ntohs(udph->len) - sizeof(struct udphdr);


        /* 判断是否是畸形报文*/
        if ((ulPacketLen <= sizeof(struct stDnsMsgHdr))
            || (((unsigned char*)udph + ulPacketLen) > skb->end))
        {
            DNSCACHE_DEBUGP("fake packet");
            return 0;
        }
        /* 跳过UDP头*/
        ptr = (unsigned char*)(udph + 1);
        pstDnsHdr = (struct stDnsMsgHdr*)ptr;
        ptr += sizeof(struct stDnsMsgHdr);

        if (0 == dnscache_checkdata(pstDnsHdr))
        {
            return 0;
        }
        if (DNS_RET_OK ==
            dnscache_getquestion((unsigned char*)pstDnsHdr, ulPacketLen, ptr, &stQuestion, &ulQuestionLen))
        {
            ulRet = dnscache_recordanswer((unsigned char*)pstDnsHdr, ulPacketLen,
                                          (ptr + ulQuestionLen), ntohs(pstDnsHdr->u16ANcnt));
        }
    }
    else if (iph->protocol == IPPROTO_TCP)
    {
        tcph = (void*)iph + iph->ihl * 4;
        if (tcph->source != htons(53))
        {
            return 0;
        }
        ulPacketLen = ntohs(iph->tot_len) - iph->ihl * 4 - tcph->doff * 4;

        /* 判断是否是畸形报文*/
        if ((ulPacketLen <= sizeof(struct stDnsMsgHdr))
            || (((unsigned char*)tcph + ulPacketLen) > skb->end))
        {
            DNSCACHE_DEBUGP("fake packet");
            return 0;
        }
        /* 跳过TCP头*/
        ptr = (void*)tcph + tcph->doff * 4 + 2;
        pstDnsHdr = (struct stDnsMsgHdr*)ptr;
        ptr += sizeof(struct stDnsMsgHdr);
             
        if (0 == dnscache_checkdata(pstDnsHdr))
        {
            return 0;
        }
        if (DNS_RET_OK ==
            dnscache_getquestion((unsigned char*)pstDnsHdr, ulPacketLen, ptr, &stQuestion, &ulQuestionLen))
        {
            ulRet = dnscache_recordanswer((unsigned char*)pstDnsHdr, ulPacketLen,
                                          (ptr + ulQuestionLen), ntohs(pstDnsHdr->u16ANcnt));
        }
    }
    return 1;
}





static unsigned int getdnsinfobyv4(unsigned int hook,
                                   struct sk_buff* skb,
                                   const struct net_device* in,
                                   const struct net_device* out,
                                   int (*okfn)(struct sk_buff*)
                                  )
{
    if (TURNOFF == g_dnscacheswitch)
    {
        return NF_ACCEPT;
    }
    dnscache_type_record_v4(skb);
    return NF_ACCEPT;
}

static int dnscache_type_record_v6(const struct sk_buff* skb)
{
    struct ipv6hdr* ih6 = NULL;
    struct tcphdr* tcph = NULL;
    struct udphdr* udph = NULL;
    struct stDnsMsgHdr* pstDnsHdr = NULL;
    unsigned char* ptr = NULL;
    unsigned int ulPacketLen = 0;
    DNS_QUESTION_ST stQuestion;
    unsigned int ulQuestionLen = 0;
    unsigned int ulRet = 0;

    ih6 = ipv6_hdr(skb);
    if (ih6 == NULL)
    {
        return 0;
    }


    /* 不处理环回口和lan桥的报文*/
    if (NULL != skb->dev)
    {
        if (0 == strncmp(skb->dev->name,LANINTERFACE,strlen(LANINTERFACE)))    
        {
            //DNSCACHE_DEBUGP("skb dev is : [%s] \n", skb->dev->name);
            return 0;
        }
        if (0 == strncmp(skb->dev->name,LOCALINTERFACE,strlen(LOCALINTERFACE)))
        {
            //DNSCACHE_DEBUGP("skb dev is : [%s] \n", skb->dev->name);
            return 0;
        }
    }
    if ((!(__ipv6_addr_type(&ih6->saddr) & IPV6_ADDR_UNICAST)) ||
        (!(__ipv6_addr_type(&ih6->daddr) & IPV6_ADDR_UNICAST)))
    {
        DNSCACHE_DEBUGP("addr is not unicast.\n");
        return 0;
    }



    if (ih6->nexthdr == IPPROTO_UDP)
    {
        udph = udp_hdr(skb);
        if (udph->source != htons(DNSPORT))
        {
            return 0;
        }

        //udpdata len
        ulPacketLen = ntohs(udph->len) - sizeof(struct udphdr);


        /* 判断是否是畸形报文*/
        if ((ulPacketLen <= sizeof(struct stDnsMsgHdr))
            || (((unsigned char*)udph + ulPacketLen) > skb->end))
        {
            DNSCACHE_DEBUGP("fake packet");
            return 0;
        }
        /* 跳过UDP头*/
        ptr = (unsigned char*)(udph + 1);
        pstDnsHdr = (struct stDnsMsgHdr*)ptr;
        ptr += sizeof(struct stDnsMsgHdr);
        /* 只处理查询报文 */
        if ((ntohs(pstDnsHdr->u16Coms) >> 15) != DNS_RESP_CODE)
        {
            DNSCACHE_DEBUGP("no response packet......");
            return 0;
        }
        /* 只处理正常有回复内容的报文, 只有1个query*/
        if ((0 == ntohs(pstDnsHdr->u16ANcnt))
            || (1 != ntohs(pstDnsHdr->u16QDcnt))
            || (0 != (ntohs(pstDnsHdr->u16Coms) & 0x000F)))
        {
            DNSCACHE_DEBUGP("error response packet......");
            return 0;
        }
        if (DNS_RET_OK ==
            dnscache_getquestion((unsigned char*)pstDnsHdr, ulPacketLen, ptr, &stQuestion, &ulQuestionLen))
        {
            ulRet = dnscache_recordanswer((unsigned char*)pstDnsHdr, ulPacketLen,
                                          (ptr + ulQuestionLen), ntohs(pstDnsHdr->u16ANcnt));
        }
    }
    else if (ih6->nexthdr == IPPROTO_TCP)
    {
        tcph = tcp_hdr(skb);
        if (tcph->source != htons(DNSPORT))
        {
            return 0;
        }
        ulPacketLen = ntohs(ih6->payload_len) - tcph->doff * 4;

        DNSCACHE_DEBUGP("ipv6 tcp data len %u", ulPacketLen);

        /* 判断是否是畸形报文*/
        if ((ulPacketLen <= sizeof(struct stDnsMsgHdr))
            || (((unsigned char*)tcph + ulPacketLen) > skb->end))
        {
            DNSCACHE_DEBUGP("fake packet");
            return 0;
        }
        /* 跳过TCP头*/
        ptr = (void*)tcph + tcph->doff * 4 + 2;
        pstDnsHdr = (struct stDnsMsgHdr*)ptr;
        ptr += sizeof(struct stDnsMsgHdr);
        /* 只处理查询报文 */
        if ((ntohs(pstDnsHdr->u16Coms) >> 15) != DNS_RESP_CODE)
        {
            DNSCACHE_DEBUGP("no response packet......");
            return 0;
        }
        /* 只处理正常有回复内容的报文, 只有1个query*/
        if ((0 == ntohs(pstDnsHdr->u16ANcnt))
            || (1 != ntohs(pstDnsHdr->u16QDcnt))
            || (0 != (ntohs(pstDnsHdr->u16Coms) & 0x000F)))
        {
            DNSCACHE_DEBUGP("error response packet......");
            return 0;
        }
        if (DNS_RET_OK ==
            dnscache_getquestion((unsigned char*)pstDnsHdr, ulPacketLen, ptr, &stQuestion, &ulQuestionLen))
        {
            ulRet = dnscache_recordanswer((unsigned char*)pstDnsHdr, ulPacketLen,
                                          (ptr + ulQuestionLen), ntohs(pstDnsHdr->u16ANcnt));
        }
    }
    return 1;
}


static unsigned int getdnsinfobyv6(unsigned int hook,
                                   struct sk_buff* skb,
                                   const struct net_device* in,
                                   const struct net_device* out,
                                   int (*okfn)(struct sk_buff*)
                                  )
{
    if (TURNOFF == g_dnscacheswitch)
    {
        return NF_ACCEPT;
    }
    dnscache_type_record_v6(skb);
    return NF_ACCEPT;
}

int proc_dnscache_list_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;
    spin_lock_bh(&dnscache_lock);
    ret = showipinfoall();
    spin_unlock_bh(&dnscache_lock);
    return ret;
}

int proc_dnscache_match_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;
    if (inputurlinfo)
    {
        ret = strlen(inputurlinfo) + 1;
        if(copy_to_user(buffer, inputurlinfo, len)) 
        {
            return -1;
        }
    }
    return ret;
}

int proc_dnscache_match_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    char* pinput = NULL;
    if (inputurlinfo)
    {
        kfree(inputurlinfo);
        inputurlinfo = NULL;
    }
    pinput = kmalloc(len + 1, GFP_ATOMIC);
    if (NULL == pinput)
    {
        return -ENOMEM;
    }
    memset(pinput,0,len + 1);
    if (copy_from_user(pinput, userBuf, len) != 0)
    {
        kfree(pinput);
        return -EFAULT;
    }
    pinput[len] = 0;
    spin_lock_bh(&dnscache_lock);
    deleteurlmatchlist();
	delallwhileunmatchlist();
    if (strlen(pinput))
    {
        dnscache_changelower(pinput, strlen(pinput));
        procaddmatchlist(pinput);
    }
    spin_unlock_bh(&dnscache_lock);

    inputurlinfo = pinput;
    return len;
}

static struct nf_hook_ops dnscache_ops[] __read_mostly =
{
    //在v4 input链收集dns响应包
    {
        .hook = getdnsinfobyv4,
        .pf = PF_INET,
        .hooknum = NF_INET_LOCAL_IN,
        .priority = NF_IP_PRI_FILTER + 1,
    },
    //在v4 forward链收集dns响应包
    {
        .hook = getdnsinfobyv4,
        .pf = PF_INET,
        .hooknum = NF_INET_FORWARD,
        .priority = NF_IP_PRI_FILTER + 1,
    },

    //在v6 input链收集dns响应包
    {
        .hook = getdnsinfobyv6,
        .pf = PF_INET6,
        .hooknum = NF_INET_LOCAL_IN,
        .priority = NF_IP6_PRI_FILTER + 1,
    },
    //在v6 forward链收集dns响应包
    {
        .hook = getdnsinfobyv6,
        .pf = PF_INET6,
        .hooknum = NF_INET_FORWARD,
        .priority = NF_IP6_PRI_FILTER + 1,
    },
};
static int init_hook_dnscache(void)
{
    int i = 0;
	printk("***dnscache max num %d\n",MAXDNSNUM);
    //注册钩子函数
    if (0 > nf_register_hooks(dnscache_ops, ARRAY_SIZE(dnscache_ops)))
    {
        DNSCACHE_DEBUGP("nf_register_hooks failed");    
    }
    for (i = 0; i < MAXDNSCACHE; i++)
    {
        INIT_HLIST_HEAD(&dnshash[i]);
    }
    INIT_LIST_HEAD(&urlmatchlist);
	INIT_LIST_HEAD(&urlprobablematchlist);
	INIT_LIST_HEAD(&unmatchwhileurlmatchlist);
    return 0;
}

static void fini_hook_dnscache(void)
{
    //去注册
    nf_unregister_hooks(dnscache_ops, ARRAY_SIZE(dnscache_ops));
    return;
}



MODULE_AUTHOR("Sliver iw ah");
module_init(init_hook_dnscache);
module_exit(fini_hook_dnscache);
