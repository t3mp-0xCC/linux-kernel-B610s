
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/netfilter.h>
#include <net/checksum.h>
#include <net/tcp.h>
#include "atp_guestwifiaccessui.h"


DEFINE_SPINLOCK(ifaccess_lock);


#if 0
 #define IFACCESS_DEBUGP(format, args...) printk(format, ## args)
#else
 #define IFACCESS_DEBUGP(format, args...)
#endif


#define BUF_IFACCESSSTRING_LEN      64


struct st_ifaccessinfo
{
    char			          name[IFNAMSIZ];
    struct list_head          list;
};


struct list_head  iflist ;

void delifnamelist(void)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
    struct st_ifaccessinfo* listret  = NULL;
	
	spin_lock_bh(&ifaccess_lock);
	list_for_each_safe(listp, listn, &iflist )
	{
		listret = list_entry(listp, struct st_ifaccessinfo, list);
        list_del(listp);
        kfree(listret);
	}
	spin_unlock_bh(&ifaccess_lock);
    return;
}



int showifnamelist(char *buffer)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
	struct st_ifaccessinfo* listret  = NULL;
	int retlen = 0;

	spin_lock_bh(&ifaccess_lock);
	list_for_each_safe(listp, listn, &iflist )
	{
	    listret = list_entry(listp, struct st_ifaccessinfo, list);
        retlen += snprintf(buffer + retlen, GUESTWIFI_BUFF_LEN_MAX - retlen, "%s\n", listret->name);
	}
	spin_unlock_bh(&ifaccess_lock);
    return retlen;
}



int checkifnamelist(char *pname)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
	struct st_ifaccessinfo* listret  = NULL;

    if(0 == strlen(pname))
    {
        return 0;
    }
	spin_lock_bh(&ifaccess_lock);
	list_for_each_safe(listp, listn, &iflist)
	{
	    listret = list_entry(listp, struct st_ifaccessinfo, list);
        if(0 == strcmp(listret->name,pname))
        {
            spin_unlock_bh(&ifaccess_lock);
            return 1;
        }
	}
	spin_unlock_bh(&ifaccess_lock);
    return 0;
}


static void addifnamelist(char *ifname) 
{
	struct st_ifaccessinfo *pnameinfo = NULL;
	pnameinfo = kmalloc(sizeof(struct st_ifaccessinfo), GFP_ATOMIC);
	if (NULL == pnameinfo)
	{
		return ;
	}
    memset(pnameinfo->name,0,sizeof(pnameinfo->name));
	snprintf(pnameinfo->name,IFNAMSIZ,"%s",ifname);
    spin_lock_bh(&ifaccess_lock);
    list_add(&pnameinfo->list,&iflist);
    spin_unlock_bh(&ifaccess_lock);
    return ;
}


void procaddifnamelist(char* pinfo)
{
    char buf[BUF_IFACCESSSTRING_LEN] = {0};
    char* p = pinfo;
    int i = 0;
	
	delifnamelist();

    while (*p)
    {
        if (*p == ';')
        {
            addifnamelist(buf);
            memset(buf, 0, BUF_IFACCESSSTRING_LEN);
            i = 0;
            p++;
            continue;
        }

        buf[i] = *p;
        i++;
        p++;
    }
    return;
}


#ifdef BSP_CONFIG_BOARD_E5_SB03

#define  HTTPDATALENGTH    1500
#define  GUESTWIFIWHITESTRING    "limitspeedforssidb"

int guestwifiaccessui(struct sk_buff *skb,struct tcphdr* tcph,struct iphdr* iph)
{
    unsigned char* data = NULL;

    if (tcph->doff * 4 + iph->ihl * 4 == skb->len)
    {
        return NF_ACCEPT;
    }

    /*取出数据*/
    if (skb->len - tcph->doff * 4 - iph->ihl * 4 < HTTPDATALENGTH - 1)
    {
        data = (unsigned char*)kmalloc(HTTPDATALENGTH, GFP_KERNEL);

        if (data == NULL)
        {
            return NF_ACCEPT;
        }

        memset(data, 0, HTTPDATALENGTH);
        strncpy(data, (void*)tcph + tcph->doff * 4, skb->len - tcph->doff * 4 - iph->ihl * 4);

        if (strstr(data, GUESTWIFIWHITESTRING) != NULL)
        {
            IFACCESS_DEBUGP("it is guestwifi PRSITE request,so accept\n");
            kfree(data);
            return NF_ACCEPT;
        }
        else
        {
            kfree(data);
            return NF_DROP;
        }
    }
    return NF_ACCEPT;
}


#endif



unsigned int nf_ifaceaccess_in_v6(struct sk_buff *skb, const struct net_device *pin, 
	                                                const struct net_device *pout)
{
    struct ipv6hdr* ih6 = NULL;
    struct tcphdr* tcph = NULL;
    
    ih6 = ipv6_hdr(skb);
	if(NULL == ih6)
	{
        return NF_ACCEPT;
	}

    if (ih6->nexthdr == IPPROTO_TCP)
    {
        tcph = tcp_hdr(skb);
		if(NULL == tcph)
		{
            return NF_ACCEPT;
		}
        if ((tcph->dest == htons(80))||(tcph->dest == htons(443)))
        {
            return NF_DROP;
        }
    }
    return NF_ACCEPT;
}




unsigned int nf_ifaceaccess_in_v4(struct sk_buff *skb, const struct net_device *pin, 
	                                                const struct net_device *pout)
{
    struct iphdr* iph = NULL;
    struct tcphdr* tcph = NULL;

    iph = ip_hdr(skb);

    if (NULL == iph)
    {
        return NF_ACCEPT;
    }

    if (iph->protocol == IPPROTO_TCP)
    {
        tcph = (void*)iph + iph->ihl * 4;
        if (tcph->dest == htons(80) || tcph->dest == htons(443))
        {
#ifdef BSP_CONFIG_BOARD_E5_SB03
            if(tcph->dest == htons(80))
            {
                return guestwifiaccessui(skb,tcph,iph);
            }
#endif        
            return NF_DROP;
        }
    }

    return NF_ACCEPT;
}


static unsigned int ifaceaccess_in_v4(unsigned int hooknum,
				      struct sk_buff *skb,
				      const struct net_device *in,
				      const struct net_device *out,
				      int (*okfn)(struct sk_buff *))
{
#ifdef CONFIG_ATP_GETINDEV
   if(NULL == skb->lanindev)
   {
       return NF_ACCEPT;
   }
   if(checkifnamelist(skb->lanindev->name))
   {
      return nf_ifaceaccess_in_v4(skb,in,out);
   }
#endif   
   return NF_ACCEPT;
}



static unsigned int ifaceaccess_in_v6(unsigned int hooknum,
				      struct sk_buff *skb,
				      const struct net_device *in,
				      const struct net_device *out,
				      int (*okfn)(struct sk_buff *))
{
#ifdef CONFIG_ATP_GETINDEV
	if(NULL == skb->lanindev)
	{
		return NF_ACCEPT;
	}

	if(checkifnamelist(skb->lanindev->name))
	{
	   return nf_ifaceaccess_in_v6(skb,in,out);
	}
#endif    
	return NF_ACCEPT;
}



static struct nf_hook_ops atp_ifaccess_hooks[] __read_mostly =
{
    {
        .hook = ifaceaccess_in_v4,
        .pf = PF_INET,
        .hooknum = NF_INET_LOCAL_IN,
        .priority = NF_IP_PRI_FILTER - 1,
    },

    
    {
        .hook = ifaceaccess_in_v6,
        .pf = PF_INET6,
        .hooknum = NF_INET_LOCAL_IN,
        .priority = NF_IP_PRI_FILTER - 1,
    },
};


static int __init   nf_ifaccess_init(void)
{
    int ret = 0;
    INIT_LIST_HEAD(&iflist);

    ret = nf_register_hooks(atp_ifaccess_hooks, ARRAY_SIZE(atp_ifaccess_hooks));
    return ret;
}

static void __exit  nf_ifaccess_fini(void)
{
    nf_unregister_hooks(&atp_ifaccess_hooks, ARRAY_SIZE(atp_ifaccess_hooks));
    return;
}

module_init(nf_ifaccess_init);
module_exit(nf_ifaccess_fini);
