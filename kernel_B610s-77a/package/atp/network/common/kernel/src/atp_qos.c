

#include <linux/if_arp.h>
#include <linux/if_vlan.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/netfilter_bridge.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <net/net_namespace.h>
#include <linux/atphooks.h>
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
#include <linux/imq.h>
#endif
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_ftos_t.h>
#include <linux/netfilter_bridge/ebt_ip.h>
#include <linux/netfilter_bridge/ebt_mark_t.h>
#include "br_private.h"
#include "atp_interface.h"

extern int atp_register_hook(struct atp_hook_ops* reg);
extern void atp_unregister_hook(struct atp_hook_ops* reg);


#define QOS_IPP_MARK_ZERO   0x100 
#define QOS_TOS_MARK_ZERO   0x200  
#define QOS_DSCP_MARK       0x1 /* 区分ebtables ftos 是dscp还是tos或者ipp */

#define IPTOS_IPP_MASK		0xE0
#define IPTOS_DSCP_MASK		0xFC
#define IPTOS_DSCP(tos)		((tos)&IPTOS_DSCP_MASK)





static int dev_imq_flags(unsigned int hooknum,
        void* pvskb,
        void* arg2,
        void* arg3)
{

    //这个有问题，用返回值去修改
    struct sk_buff* skb = NULL;

    if (unlikely(NULL == pvskb))
    {
        return ATP_CONTINUE;
    }

    skb = (struct sk_buff*)pvskb;
	
    return skb->imq_flags & IMQ_F_ENQUEUE;
}		
static int qos_imq_mark(unsigned int hooknum,
        void* pvskb,
        void* pvdev,
        void* arg3)
{
    struct sk_buff* skb = NULL;

    if (unlikely(NULL == pvskb))
    {
        return ATP_CONTINUE;
    }

    skb = (struct sk_buff*)pvskb;

    #if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
    skb->mark |= QOS_DEFAULT_MARK;
    #endif
    return ATP_CONTINUE;
}		

static int qos_af_imq_mark(unsigned int hooknum,
        void* pvskb,
        void* pvsk,
        void* arg3)
{
    struct sk_buff* skb = NULL;
    struct sock *sk = NULL;
    if (unlikely(NULL == pvskb) || unlikely(NULL == pvsk))
    {
        return ATP_CONTINUE;
    }

    skb = (struct sk_buff*)pvskb;
    sk = (struct sock*)pvsk;

    #if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	skb->mark = sk->sk_mark;
    skb->mark |= QOS_DEFAULT_MARK;
    #endif
    return ATP_CONTINUE;
}
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
extern int downqos_enable;
extern int qos_enable;
static int qos_driver_imq_transmit(unsigned int hooknum,
        void* pskb,
        void* pdev,
        void* arg3)
{
    struct sk_buff* skb = NULL;
    struct net_device *dev = NULL;
	unsigned int ulLanLanPktFlag = 0;
	int ret = 0;
	if ((NULL == pskb) || (NULL == pdev))
    {
        return ATP_CONTINUE;
    }

	skb = (struct sk_buff*)pskb;
	dev = (struct net_device*)pdev;
	
	//判断是否为lan侧发起
    ulLanLanPktFlag = (skb->mark &0x200000);


    if (downqos_enable && !ulLanLanPktFlag&& IS_ETN_LAN(dev->name))
    { 
        //packet from non-br to imq
        if ((skb->mark & QOS_DOWNDEFAULT_MARK) != QOS_DOWNDEFAULT_MARK)
        {
            skb->imq_flags &= ~IMQ_F_ENQUEUE;
            skb->mark |= QOS_DOWNDEFAULT_MARK;
            skb->imq_flags |= QOS_DEV_IMQ4;

            skb->skb_iif = dev->ifindex;
            ret = imq_nf_queue(skb);
			
            //进入imq正常处理
            if (1 != ret)
            {
                return ATP_STOP;
            }       
        }
    }
    else if (qos_enable && IS_ETN_WAN(dev->name))
	{
	    if (skb->mark & QOS_DEFAULT_MARK)
        {
            skb->imq_flags &= ~IMQ_F_ENQUEUE;
            skb->mark &= ~QOS_DEFAULT_MARK;
            skb->imq_flags |= QOS_DEV_IMQ2;
 
            skb->skb_iif = dev->ifindex;
            ret = imq_nf_queue(skb);
            if (1 != ret)
            {	
                return ATP_STOP;
            }
            else
            {
                skb->imq_flags |= QOS_DEV_IMQ3;
                ret = imq_nf_queue(skb);
                if (1 != ret)
                {
                    return ATP_STOP;
                }    
            }
			
        }
        skb->mark |= QOS_DEFAULT_MARK;
	}
	return ATP_CONTINUE;
}		
#endif


/* start of protocol mark precedence 或者tos字段需要保留原值A36D02507 by z45221 zhangchen 2006年8月12日
iph->tos = ftosinfo->ftos;
** NOTE:          tos field
**                bit 7 ~ bit 5 = precedence (0 = normal, 7 = extremely high)
**                bit 4 = D bit (minimize delay)
**                bit 3 = T bit (maximize throughput)
**                bit 2 = R bit (maximize reliability)
**                bit 1 = C bit (minimize transmission cost)
**                bit 0 = not used
iph->tos = ftosinfo->ftos;
*/
void ebt_get_DSCP_ftos(struct iphdr *iph, const struct ebt_ftos_t_info *ftosinfo)
{
	if (QOS_DSCP_MARK == (QOS_DSCP_MARK & ftosinfo->ftos))
    {
		iph->tos = (iph->tos & (~IPTOS_DSCP_MASK)) | IPTOS_DSCP(ftosinfo->ftos);
    }
    else
    {
        if ((QOS_TOS_MARK_ZERO & ftosinfo->ftos) && (QOS_IPP_MARK_ZERO & ftosinfo->ftos))
        {
            iph->tos = (iph->tos & 0x1);
        }
        else if (QOS_TOS_MARK_ZERO & ftosinfo->ftos)
        {
            if (IPTOS_PREC(ftosinfo->ftos))
            {
                iph->tos = IPTOS_PREC(ftosinfo->ftos) | (iph->tos & 0x1);
            }
            else
            {
                iph->tos = IPTOS_PREC(iph->tos) | (iph->tos & 0x1);
            }
        }
        else if (QOS_IPP_MARK_ZERO & ftosinfo->ftos)
        {
            if (IPTOS_TOS(ftosinfo->ftos))
            {
                iph->tos = IPTOS_TOS(ftosinfo->ftos) | (iph->tos & 0x1);
            }
            else
            {
                iph->tos = IPTOS_TOS(iph->tos) | (iph->tos & 0x1);
            }            
        }
        else
        {
            if (IPTOS_TOS(ftosinfo->ftos) 
                && IPTOS_PREC(ftosinfo->ftos))
            {
                iph->tos = ftosinfo->ftos;
            }
            else if (IPTOS_TOS(ftosinfo->ftos))
            {
                iph->tos = (iph->tos & (~IPTOS_TOS_MASK)) | IPTOS_TOS(ftosinfo->ftos);
            }
            else if (IPTOS_PREC(ftosinfo->ftos))
            {
                iph->tos = (iph->tos & (~IPTOS_PREC_MASK)) | IPTOS_PREC(ftosinfo->ftos);
            }
        }
    }
}
	/* end of protocol mark precedence 或者tos字段需要保留原值A36D02507 by z45221 zhangchen 2006年8月12日 */


static int qos_ebtables_tos_mark(unsigned int hooknum,
        void* arg1,
        void* arg2,
        void* arg3)
{
    struct iphdr *iph = NULL; 
    const struct ebt_ftos_t_info *ftosinfo = NULL;
	int *pflag = (int *)arg3;
	unsigned char tmp_ftos = 0;
	__u8 *ptos;
	if(1 == *pflag)
	{
	    if (unlikely(NULL == arg1) || unlikely(NULL == arg2))
        {
            return ATP_CONTINUE;
        }
        iph = (struct iphdr*)arg1;
        ftosinfo = (const struct ebt_ftos_t_info*)arg2;
	    ebt_get_DSCP_ftos(iph, ftosinfo);
	    return ATP_CONTINUE;
	}
	else if(2 == *pflag)
	{
		if (unlikely(NULL == arg1)||unlikely(NULL == arg2))
        {
            return ATP_CONTINUE;
        }
		ptos = (__u8 *)arg2;
        ftosinfo = (const struct ebt_ftos_t_info*)arg1;
	    tmp_ftos = ftosinfo->ftos & (~QOS_DSCP_MARK);
		tmp_ftos = tmp_ftos |(*ptos & 0x3);
		return tmp_ftos;
	}
    return ATP_CONTINUE;
}		



static int qos_ebtables_ip_mark(unsigned int hooknum,
        void* pinfo,
        void* arg2,
        void* arg3)
{
    const struct ebt_ip_info *info = NULL;
	const struct iphdr *ih = NULL;
	if(unlikely(NULL == pinfo))
	{
	    return ATP_CONTINUE;
	}
	
	if(unlikely(NULL == arg2))
	{
	    return ATP_CONTINUE;
	}
	info = (const struct ebt_ip_info *)pinfo;
	ih = (const struct iphdr *)arg2;
    if (QOS_DSCP_MARK == (QOS_DSCP_MARK & info->tos))
    {
    	if (info->bitmask & EBT_IP_TOS &&
    	   FWINV((info->tos & 0xFC) != (ih->tos & 0xFC), EBT_IP_TOS))
    	return false;
    }
    else
    {
        if ((QOS_TOS_MARK_ZERO & info->tos) && (QOS_IPP_MARK_ZERO & info->tos))
        {
        	if (info->bitmask & EBT_IP_TOS &&
        	   FWINV(IPTOS_TOS(ih->tos) != 0, EBT_IP_TOS))
        		return false; 
            
        	if (info->bitmask & EBT_IP_TOS &&
        	   FWINV(IPTOS_PREC(ih->tos) != 0, EBT_IP_TOS))
        		return false;
        }
        else if (QOS_TOS_MARK_ZERO & info->tos)
        {
        	if (info->bitmask & EBT_IP_TOS &&
        	   FWINV(IPTOS_TOS(ih->tos) != 0, EBT_IP_TOS))
        		return false; 
            
        	if (info->bitmask & EBT_IP_TOS &&
        	   FWINV(IPTOS_PREC(info->tos) != IPTOS_PREC(ih->tos), EBT_IP_TOS))
        		return false;
        }
        else if (QOS_IPP_MARK_ZERO & info->tos)
        {
        	if (info->bitmask & EBT_IP_TOS &&
        	   FWINV(IPTOS_TOS(info->tos) != IPTOS_TOS(ih->tos), EBT_IP_TOS))
        		return false; 
            
        	if (info->bitmask & EBT_IP_TOS &&
        	   FWINV(IPTOS_PREC(ih->tos) != 0, EBT_IP_TOS))
        		return false;
        }
        else 
        {
            if (IPTOS_TOS(info->tos) 
                && IPTOS_PREC(info->tos))
            {
            	if (info->bitmask & EBT_IP_TOS &&
            	   FWINV(info->tos != ih->tos, EBT_IP_TOS))
            		return false;        
            }
            else if (IPTOS_TOS(info->tos))
            {
            	if (info->bitmask & EBT_IP_TOS &&
            	   FWINV(IPTOS_TOS(info->tos) != IPTOS_TOS(ih->tos), EBT_IP_TOS))
            		return false;          
            }
            else if (IPTOS_PREC(info->tos))
            {
            	if (info->bitmask & EBT_IP_TOS &&
            	   FWINV(IPTOS_PREC(info->tos) != IPTOS_PREC(ih->tos), EBT_IP_TOS))
            		return false;          
            }
        }
    }
	return ATP_CONTINUE;
}




static int qos_ebtables_mark(unsigned int hooknum,
        void* pinfo,
        void* pskb,
        void* arg3)
{
    const struct ebt_mark_t_info *info = (const struct ebt_mark_t_info *)pinfo;
	struct sk_buff *skb = (struct sk_buff *)pskb;
	if( skb && info)
    {
        if (info->mark & 0x000ff000)
        {
    		skb->mark = (skb->mark & (~0x000ff000)) | info->mark;
        }
        else
        {
		    skb->mark = info->mark;
        }
    }
	return ATP_CONTINUE;
}		

static struct atp_hook_ops qos_dev_hook_ops =
{
    .hook = dev_imq_flags,
    .hooknum = ATP_DEV_QOS_IMQ_JUDGE,
    .priority = ATP_PRI_DEFAULT,
};


static struct atp_hook_ops qos_br_mark_hook_ops =
{
    .hook = qos_imq_mark,
    .hooknum = ATP_BR_FORWARD,
    .priority = ATP_PRI_DEFAULT,
};

static struct atp_hook_ops qos_nfoutput_mark_hook_ops =
{
    .hook = qos_imq_mark,
    .hooknum = ATP_NF_OUTPUT,
    .priority = ATP_PRI_DEFAULT,
};


static struct atp_hook_ops qos_afpacket_mark_hook_ops =
{
    .hook = qos_af_imq_mark,
    .hooknum = ATP_AF_PACKET,
    .priority = ATP_PRI_DEFAULT,
};


static struct atp_hook_ops qos_ebtables_tos_hook_ops =
{
    .hook = qos_ebtables_tos_mark,
    .hooknum = ATP_EB_TOS,
    .priority = ATP_PRI_DEFAULT,
};


static struct atp_hook_ops qos_ebtables_ip_hook_ops =
{
    .hook = qos_ebtables_ip_mark,
    .hooknum = ATP_EB_IP,
    .priority = ATP_PRI_DEFAULT,
};

static struct atp_hook_ops qos_ebtables_mark_hook_ops =
{
    .hook = qos_ebtables_mark,
    .hooknum = ATP_EB_MARK,
    .priority = ATP_PRI_DEFAULT,
};


static struct atp_hook_ops qos_driver_imq_hook_ops =
{
    .hook = qos_driver_imq_transmit,
    .hooknum = ATP_DEV_IMQ,
    .priority = ATP_PRI_DEFAULT,
};
static int __init atp_qos_init(void)
{
    int ret = 0;
    ret = atp_register_hook(&qos_dev_hook_ops);
    if (0 != ret)
    {
        return ret;
    }

	ret = atp_register_hook(&qos_br_mark_hook_ops);
    if (0 != ret)
    {
        atp_unregister_hook(&qos_dev_hook_ops);
        return ret;
    }
	
	ret = atp_register_hook(&qos_nfoutput_mark_hook_ops);
    if (0 != ret)
    {
        atp_unregister_hook(&qos_dev_hook_ops);
	    atp_unregister_hook(&qos_br_mark_hook_ops);
        return ret;
    }
	
	ret = atp_register_hook(&qos_afpacket_mark_hook_ops);
    if (0 != ret)
    {
        atp_unregister_hook(&qos_dev_hook_ops);
	    atp_unregister_hook(&qos_br_mark_hook_ops);
	    atp_unregister_hook(&qos_nfoutput_mark_hook_ops);
        return ret;
    }
	atp_register_hook(&qos_ebtables_tos_hook_ops);
	atp_register_hook(&qos_ebtables_ip_hook_ops);
	atp_register_hook(&qos_driver_imq_hook_ops);
	atp_register_hook(&qos_ebtables_mark_hook_ops);
    return 0;
}


static void __exit atp_qos_fini(void)
{
    atp_unregister_hook(&qos_dev_hook_ops);
	atp_unregister_hook(&qos_br_mark_hook_ops);
	atp_unregister_hook(&qos_nfoutput_mark_hook_ops);
	atp_unregister_hook(&qos_afpacket_mark_hook_ops);
	atp_unregister_hook(&qos_ebtables_tos_hook_ops);
	atp_unregister_hook(&qos_ebtables_ip_hook_ops);
	atp_unregister_hook(&qos_driver_imq_hook_ops);
	atp_unregister_hook(&qos_ebtables_mark_hook_ops);
    return;
}


module_init(atp_qos_init);
module_exit(atp_qos_fini);


