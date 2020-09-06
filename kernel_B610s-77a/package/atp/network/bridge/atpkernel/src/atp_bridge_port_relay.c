

#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/netfilter_bridge.h>
#include <linux/netdevice.h>
#include <linux/atphooks.h>
#include "br_private.h"
#include "atp_interface.h"
#include "atp_bridge.h"

extern int atp_register_hook(struct atp_hook_ops* reg);^M
extern void atp_unregister_hook(struct atp_hook_ops* reg);

/*特性用到的数据结构和API*/
#if DESC("Common API")

#define PORT_RELAY_NUM      16 //CONNECT_OPTION60_MAX
struct port_relay
{
    char acWanName[PORT_RELAY_NUM][IFNAMSIZ + 1];
    int iEnable[PORT_RELAY_NUM];  //设备类型
};

struct port_relay g_portrelay =
{
    .acWanName = {0},
    .iEnable = {0},
};


/*iIptype表示IP的版本，0:IPV6 1:IPV4 */
#define IPV4_TYPE_CODE   0
#define IPV6_TYPE_CODE   1
#define IPBOTH_TYPE_CODE 2
#define PORT_RELAY_TYPE IPV4_TYPE_CODE

static int IsSkbfromDhcpsPort(struct sk_buff* skb, int iIptype)
{
    int iIsdhcpsport = 0;
    unsigned short usIpHeadLen = 0;

    if (NULL == skb)
    {
        return iIsdhcpsport;
    }

    usIpHeadLen = (skb->data[0] & 0x0f) * 4;

    /*判断是否IPV6 dhcp报文*/
#ifdef CONFIG_IPV6
    if ((htons(ETH_P_IPV6) == eth_hdr(skb)->h_proto) && (IPV6_TYPE_CODE == iIptype || IPBOTH_TYPE_CODE == iIptype))
    {
        if (IPPROTO_UDP == skb->data[6])
        {
            /*IPV6的情况下端口在skb中所处的位置*/
            if (((0x02 == skb->data[40]) && (0x22 == skb->data[41]))
                && ((0x02 == skb->data[42]) && (0x23 == skb->data[43])))
            {
                iIsdhcpsport = 1;
            }
        }
        else if (IPPROTO_ICMPV6 == skb->data[6])          /* RS/RA */
        {
            if ((0x85 == skb->data[40]) || (0x86 == skb->data[40]))
            {
                iIsdhcpsport = 1;
            }
        }
    }
    else
#endif
    {
        /*判断是否IPV4 dhcp报文*/
        if ((skb->data[9] == IPPROTO_UDP) && (IPV4_TYPE_CODE == iIptype || IPBOTH_TYPE_CODE == iIptype))
        {
            if (skb->data[usIpHeadLen] == 0x00 && skb->data[usIpHeadLen + 1] == 0x44
                && skb->data[usIpHeadLen + 2] == 0x00 && skb->data[usIpHeadLen + 3] == 0x43)
            {
                iIsdhcpsport = 1;
            }
        }
    }

    return iIsdhcpsport;
}


static int DevDhcpRelayOn(const char* name)
{
    int i = 0;
    int iEnable = 0;

    for ( i = 0; i < PORT_RELAY_NUM; i++)
    {
        if (strstr(name, g_portrelay.acWanName[i]))
        {
            iEnable = g_portrelay.iEnable[i];
            break;
        }
    }

    return iEnable;
}

static int Br_port_dhcp_relay(struct sk_buff* skb, struct net_device* indev)
{
    struct net_bridge_port* p;
    int iEnable = 0;

    if (NULL == skb)
    {
        return iEnable;
    }

    /*判断是否开启端口relay*/
    if (IS_LAN_DEV(indev->name) && DevDhcpRelayOn(indev->name))
    {
        return 1;
    }

	p = br_port_get_rcu(indev);
	
    /*遍历接口所在的bridge是否有WAN开启DHCP Relay*/
    if (p)
    {
        /*判断br是否要开启relay*/
        if (DevDhcpRelayOn(p->br->dev->name))
        {
            return 1;
        }
    }
    return iEnable;
}

static int br_port_relay_exist(const char* value, int flag)
{
    int i;
    for (i = 0; i < PORT_RELAY_NUM; i++)
    {
        if (!memcmp(g_portrelay.acWanName[i], value, IFNAMSIZ))
        {
            g_portrelay.iEnable[i] = flag;
            return 1;
        }
    }
    return 0;
}
static void br_set_port_relay(const char* value, int flag)
{
    int i;
    unsigned char buff[IFNAMSIZ + 1] = {0};
    if (br_port_relay_exist(value, flag))
    {
        return;
    }
    for (i = 0; i < PORT_RELAY_NUM; i++)
    {
        if (0 == g_portrelay.iEnable[i])
        {
            memcpy(g_portrelay.acWanName[i], value, IFNAMSIZ);
            g_portrelay.iEnable[i] = flag;
            break;
        }
    }
    return;
}

#endif


/*Bridge port relay特性:绑定了桥接wan的bridge从桥接wan拿地址，否则从bridge local拿地址*/
#if DESC("Feature 1:Bridge dhcp port relay function")

static unsigned int atp_br_port_relay_proc(unsigned int hooknum,
        struct sk_buff* skb,
        const struct net_device* in,
        const struct net_device* out, int (*okfn) (struct sk_buff*))
{
    if (!IsSkbfromDhcpsPort(skb, PORT_RELAY_TYPE))
    {
        return NF_ACCEPT;
    }

    /*for DHCP端口绑定二层透传 */
    if (Br_port_dhcp_relay(skb, in))
    {
        /*接口被绑定需要桥透传，local in不收包不分地址*/
        if (NF_BR_LOCAL_IN == hooknum)
        {
            ATP_BRIDGE_DEBUG("dev [%s] need port relay, not send to lcoal in\n", in->name);
            return NF_DROP;
        }

        /*forward 转发此dhcp报文*/
        ATP_BRIDGE_DEBUG("dev [%s] need port relay, relay it\n", in->name);
        return NF_ACCEPT;
    }

    /*没有桥接wan需要relay此dhcp报文，不转发*/
    if (NF_BR_FORWARD == hooknum)
    {
        ATP_BRIDGE_DEBUG("dev [%s] need not relay, not relay it\n", in->name);
        return NF_DROP;
    }

    /*不需要relay，送local in分地址*/
    ATP_BRIDGE_DEBUG("dev [%s] need not relay, send to local in\n", in->name);
    return NF_ACCEPT;
}


static struct nf_hook_ops atp_br_relay_forward_hook =
{
    .hook = atp_br_port_relay_proc,
    .pf = PF_BRIDGE,
    .hooknum = NF_BR_FORWARD,
    .priority = NF_BR_PRI_BRNF,
};


static struct nf_hook_ops atp_br_relay_local_in_hook =
{
    .hook = atp_br_port_relay_proc,
    .pf = PF_BRIDGE,
    .hooknum = NF_BR_LOCAL_IN,
    .priority = NF_BR_PRI_BRNF,
};

#endif


/*bridge ioctl扩展，实现port relay参数设置*/
#if DESC("br ioctl extend")

static int br_port_relay_ioctl_handler(unsigned int hooknum,
        void* pvIntfName,
        void* pvStatus,
        void* arg3)
{
    char* psIntfName = NULL;
    int  lStatus = 0;

    if ((NULL == pvIntfName) || (NULL == pvStatus))
    {
        return ATP_CONTINUE;
    }

    psIntfName = (char*)pvIntfName;
    lStatus = *(int*)pvStatus;

    ATP_BRIDGE_DEBUG("Br ioctl for port relay, intf [%s] status [%d]\n", psIntfName, lStatus);
    br_set_port_relay(psIntfName, lStatus);

    return ATP_CONTINUE;
}


static struct atp_hook_ops br_ioctl_port_relay_ops =
{
    .hook = br_port_relay_ioctl_handler,
    .hooknum = ATP_BR_IOCTL_PORT_RELAY,
    .priority = ATP_PRI_DEFAULT,
};
#endif

static int __init atp_br_port_relay_init(void)
{
    int ret = 0;

    ret = nf_register_hook(&atp_br_relay_forward_hook);
    if (0 != ret)
    {
        return ret;
    }

    ret = nf_register_hook(&atp_br_relay_local_in_hook);
    if (0 != ret)
    {
        nf_unregister_hook(&atp_br_relay_forward_hook);
        return ret;
    }

    ret = atp_register_hook(&br_ioctl_port_relay_ops);
    if (0 != ret)
    {
        nf_unregister_hook(&atp_br_relay_local_in_hook);
        nf_unregister_hook(&atp_br_relay_forward_hook);
        return ret;
    }

    return 0;
}


static void __exit atp_br_port_relay_fini(void)
{
    atp_unregister_hook(&br_ioctl_port_relay_ops);
    nf_unregister_hook(&atp_br_relay_local_in_hook);
    nf_unregister_hook(&atp_br_relay_forward_hook);
    return;
}


module_init(atp_br_port_relay_init);
module_exit(atp_br_port_relay_fini);


