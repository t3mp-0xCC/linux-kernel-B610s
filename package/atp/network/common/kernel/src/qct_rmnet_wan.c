


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/wakelock.h>
#include <linux/icmp.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <linux/in.h>

#include <linux/device.h>
#ifdef CONFIG_ATP_QCT_ONDEMAND_DIAL
#include "QCTDemandDialFileIO.h" 
#include <linux/etherdevice.h>
#include <net/addrconf.h>
#endif

#define DEVICE_INACTIVE      0
#define DEVICE_ACTIVE        1
#define RMNET_DATA_LEN 1500
#define HEADROOM_FOR_BAM   8 /* for mux header */
#define TAILROOM            0 /* for padding by mux layer */
#define MAC_HEAD_LENGTH        14
#define HW_DEV_NUM 5
#define HW_DEV_ID_ONDEMAND 0 /*�������������id*/

typedef struct
{
    const char hw_dev_name[MAC_HEAD_LENGTH];
    struct net_device *hw_dev;
    /* ����ipv4��ipv6��ipv4v6���Ӷ�Ӧ�豸��index*/
    int hw_dev_rmnet_index;
    /* IPV4 ���ݰ���MACͷ,ԴMAC��HW_RMNET2��MAC��ַ,Ŀ��MAC��ַ�������*/
    char ipv4_mac_head[MAC_HEAD_LENGTH];
    /* IPV6 ���ݰ���MACͷ,ԴMAC��HW_RMNET2��MAC��ַ,Ŀ��MAC��ַ�������*/
    char ipv6_mac_head[MAC_HEAD_LENGTH];
}hw_dev_info_struct;
#define WAN_3G_NAME            "eth_x"        /* 3G/LTE���Žӿ��豸 */
#define WAN_BIP_NAME           "eth_bip"      /* ��WANBIP�豸�ӿ� */
#define WAN_TR069_NAME         "eth_tr069"    /* ��WANTR069�豸�ӿ� */
#define WAN_VOIP_NAME          "eth_voip"     /* ��WANVoIP�豸�ӿ� */
#define WAN_UPDATE_NAME        "eth_update"   /* ��WAN���������豸�ӿ� */
static hw_dev_info_struct hw_dev_info[HW_DEV_NUM] = {
    {WAN_3G_NAME, NULL, 0, 
        {0x58, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00, 0x11, 0x09, 0x64, 0x01, 0x01,0x08,0x00},
        {0x58, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00, 0x11, 0x09, 0x64, 0x01, 0x01,0x86,0xDD}},
    {WAN_BIP_NAME, NULL, 0, 
        {0x58, 0x02, 0x03, 0x04, 0x05, 0x07, 0x00, 0x11, 0x09, 0x64, 0x01, 0x01,0x08,0x00},
        {0x58, 0x02, 0x03, 0x04, 0x05, 0x07, 0x00, 0x11, 0x09, 0x64, 0x01, 0x01,0x86,0xDD}},
    {WAN_TR069_NAME, NULL, 0, 
        {0x58, 0x02, 0x03, 0x04, 0x05, 0x08, 0x00, 0x11, 0x09, 0x64, 0x01, 0x01,0x08,0x00},
        {0x58, 0x02, 0x03, 0x04, 0x05, 0x08, 0x00, 0x11, 0x09, 0x64, 0x01, 0x01,0x86,0xDD}},
    {WAN_VOIP_NAME, NULL, 0, 
        {0x58, 0x02, 0x03, 0x04, 0x05, 0x09, 0x00, 0x11, 0x09, 0x64, 0x01, 0x01,0x08,0x00},
        {0x58, 0x02, 0x03, 0x04, 0x05, 0x09, 0x00, 0x11, 0x09, 0x64, 0x01, 0x01,0x86,0xDD}},
    {WAN_UPDATE_NAME, NULL, 0, 
        {0x58, 0x02, 0x03, 0x04, 0x05, 0x0A, 0x00, 0x11, 0x09, 0x64, 0x01, 0x01,0x08,0x00},
        {0x58, 0x02, 0x03, 0x04, 0x05, 0x0A, 0x00, 0x11, 0x09, 0x64, 0x01, 0x01,0x86,0xDD}}};
struct hw_dev_private {
    struct net_device_stats stats;
    uint32_t ch_id;

    struct sk_buff *waiting_for_ul_skb;
    spinlock_t lock;
    spinlock_t tx_queue_lock;
    struct tasklet_struct tsklt;
    u32 operation_mode; /* IOCTL specified mode (protocol, QoS header) */
    uint8_t device_up;
    uint8_t in_reset;

    /*���豸�������豸�е�����id, ĿǰΪ0~4*/
    int hw_dev_id;
    /*���豸��Ӧ��rmnet�豸��ֵΪNULL��ʾ��*/
    struct net_device *rmnet_dev;
    /*���豸��Ӧ��rmnet�豸��ifindex*/
    int hw_dev_rmnet_index;
};

#ifdef CONFIG_ATP_QCT_ONDEMAND_DIAL
/*****************************************************************************
  Function:       wan_flux_monitor
  Description:   wan ���������
  Calls:            
  Called By:     
  Input:         skb_buff  
  Output:         
  Return:           

*****************************************************************************/
unsigned long wan_flux_monitor(
                    struct sk_buff     *pstSkb
)
{
    unsigned char nexthdr = 0;
    __be16 frag_off;
    int offset;
    
    if(ONDEMAND_DISCONN == g_DialMode.enDialMode || ONDEMAND_CONNECT == g_DialMode.enDialMode)
    {
            if (0 == (pstSkb->mark & 0x200000))
            {
                 /* ���ݰ�������LAN�����ݰ�������ҵ���ܴ������貦�� */
                 if (g_DialMode.is_dump_rmnet_skb)
                 {
                    printk(KERN_ERR"pkt is not from LAN. mark = 0x%x\n", pstSkb->mark);
                 }
                 return 1;
            }
            /*����ǹ㲥�����򲻷����貦�ţ�ֱ�ӹ��˵�*/  
            if (NULL != pstSkb->mac_header )
            {
                unsigned char * addr = (unsigned char *)pstSkb->mac_header;
                if(is_broadcast_ether_addr(addr))
                {
                    printk(KERN_ERR"it is BROADCAST  return\n");
                    return 1;
                }
            }
            /*�����IPv6 MLD ���� NS���ģ��򲻷����貦�ţ�ֱ�ӹ��˵�*/

            if (htons(ETH_P_IPV6) == pstSkb->protocol)
            {
                nexthdr = ipv6_hdr(pstSkb)->nexthdr;
                printk("nexthdr:%d\r\n", nexthdr);
                offset = ipv6_skip_exthdr(pstSkb, sizeof(struct ipv6hdr), &nexthdr, &frag_off);
                if (offset < 0)
                {
                    printk(KERN_ERR"nexthead offset < 0 error!\n");
                    return 1;
                }
                if ((IPPROTO_ICMPV6 == nexthdr)
                    && (NDISC_NEIGHBOUR_SOLICITATION == icmp6_hdr(pstSkb)->icmp6_type))
                {
                    printk(KERN_ERR"it is IPV6 NS return\n");
                    return 1;
                }
                else if (ipv6_is_mld(pstSkb, nexthdr, offset))
                {
                    printk(KERN_ERR"it is IPV6 MLD return\n");
                    return 1;
                }
            }

            g_DialMode.is_flow = true;
        
    }
    return 1;
}
#endif

/*************************************************
  Function:       write_hw_dev_rmnet_index
  Description:   sysfsд�ļ��ӿں���
  Calls:            
  Called By:     
  Input:           
  Output:        
  Return:         count д���ֽ���      
*************************************************/
static ssize_t write_hw_dev_rmnet_index(struct device *dev, struct device_attribute *attr,
                            const char *buf, size_t count)
{
    int id = 0;
    int devindex = 0;
    struct hw_dev_private *p = netdev_priv(to_net_dev(dev));
    
    devindex  = simple_strtoul(buf, NULL, 0);
    id = p->hw_dev_id;

    if (devindex > 0)
    {
        p->hw_dev_rmnet_index = devindex;
        p->rmnet_dev = dev_get_by_index(&init_net, devindex);
        hw_dev_info[id].hw_dev_rmnet_index = p->hw_dev_rmnet_index;
    }
    else
    {
        p->hw_dev_rmnet_index = 0;
        p->rmnet_dev = NULL;
        hw_dev_info[id].hw_dev_rmnet_index = 0;
    }
    
    return count;
}

/*************************************************
  Function:       read_hw_dev_rmnet_index
  Description:   sysfs���ļ��ӿں���
  Calls:            
  Called By:     
  Input:           
  Output:         
  Return:         count д���ֽ���       
*************************************************/
static ssize_t read_hw_dev_rmnet_index(struct device *dev, struct device_attribute *attr,
                                char *buf)
{
    struct hw_dev_private *p = netdev_priv(to_net_dev(dev));

    return sprintf(buf, "%d\n", p->hw_dev_rmnet_index);
}

static DEVICE_ATTR(rmnet_index, S_IWUSR | S_IRUSR , read_hw_dev_rmnet_index, write_hw_dev_rmnet_index);

static struct attribute *hw_dev_attributes[] = {
    &dev_attr_rmnet_index.attr,
    NULL
};

static const struct attribute_group hw_dev_group = {
  .attrs = hw_dev_attributes,
};

static int hw_dev_open(struct net_device *dev)
{
    struct hw_dev_private *p = netdev_priv(dev);
    p->device_up = DEVICE_ACTIVE;
    netif_start_queue(dev);
    return 0;
}

static int hw_dev_stop(struct net_device *dev)
{
    struct hw_dev_private *p = netdev_priv(dev);
    if (p->device_up) 
    {
    /* do not close rmnet port once up,  this causes
	remote side to hang if tried to open again */
        p->device_up = DEVICE_INACTIVE;
    }
    netif_stop_queue(dev);
    return 0;
}

static struct net_device_stats *hw_dev_get_stats(struct net_device *dev)
{
    struct hw_dev_private *p = netdev_priv(dev);
    return &p->stats;
}

static int hw_dev_xmit(struct sk_buff *skb, struct net_device *dev)
{
    int rc = 0;
    struct hw_dev_private *p = netdev_priv(dev);  

    /* �Ƴ�MACͷ */
    skb_pull(skb, MAC_HEAD_LENGTH);
    /*��hw�豸���滻Ϊ��ͨrmnet�豸��*/
    skb->dev = p->rmnet_dev;
    /*�ڵ�һ���豸�Ͽ��������������*/
    if(HW_DEV_ID_ONDEMAND == p->hw_dev_id)
    {
#ifdef CONFIG_ATP_QCT_ONDEMAND_DIAL
        if(g_DialMode.is_dump_rmnet_skb)
        {
            printk(KERN_ERR"go to hw_rmnet2_xmit\n");
        }
        wan_flux_monitor(skb);
#endif
    }

    /* ipv4 ipv6��Ӧ���豸ΪNULLʱ�������ݰ� */
    if (NULL == skb->dev)
    {
        p->stats.tx_dropped++;
        dev_kfree_skb_irq(skb);
        return 0;
    }
    p->stats.tx_packets++;
    p->stats.tx_bytes += skb->len;

    rc = dev_queue_xmit(skb);
    return rc;
}

/*************************************************
  Function:       hw_dev_rx
  Description:   hw_dev_rx�豸�������ݰ�����
                      ���յ������ݰ�����MAC ͷ��
                      ����Э��ջ,��wanʹ�ú���
  Calls:            
  Called By:     bam_recv_notify
  Input:           skb ���ݰ�
  Output:         
  Return:         count д���ֽ���    
*************************************************/
void hw_dev_rx(struct sk_buff *skb, int rmnet_index)
{
    
    /*ͳ�����ݰ�*/
    struct hw_dev_private *p = NULL;
    unsigned char *tmpskbdata = NULL;
    int i = 0;

    for(i = 0; i < HW_DEV_NUM; i++)
    {
        if(rmnet_index == hw_dev_info[i].hw_dev_rmnet_index)
        {
            break;
        }
    }
    if((HW_DEV_NUM == i) || (0 == rmnet_index))
    {
        pr_err("%s: ifindex%d no hw_rmnet device!", __func__, rmnet_index);
		dev_kfree_skb_irq(skb);
        return;
    }

    p = netdev_priv(hw_dev_info[i].hw_dev);
    skb->dev = hw_dev_info[i].hw_dev;

    /* ����MACͷ */ 
    if (MAC_HEAD_LENGTH <= skb->data - skb->head)
    {
        tmpskbdata = skb_push(skb, MAC_HEAD_LENGTH);
        if (htons(ETH_P_IPV6) == skb->protocol)
        {
            memcpy(tmpskbdata, hw_dev_info[i].ipv6_mac_head, MAC_HEAD_LENGTH);
        }
        else if (htons(ETH_P_IP) == skb->protocol)
        {
            memcpy(tmpskbdata, hw_dev_info[i].ipv4_mac_head, MAC_HEAD_LENGTH);
        }
        else
        {
            p->stats.rx_dropped++;
            dev_kfree_skb_irq(skb);
            return;
        }            
        skb->protocol = eth_type_trans(skb, hw_dev_info[i].hw_dev);
                 
        p->stats.rx_packets++;
        p->stats.rx_bytes += skb->len;
        /* Deliver to network stack */                  
        netif_rx(skb);
    }
    else
    {
        p->stats.rx_dropped++;
        dev_kfree_skb_irq(skb);
    }
}

static const struct net_device_ops hw_dev_ops = {
        .ndo_open = hw_dev_open,
        .ndo_stop = hw_dev_stop,
        .ndo_start_xmit = hw_dev_xmit,
        .ndo_get_stats = hw_dev_get_stats,
        .ndo_set_mac_address = eth_mac_addr,
        .ndo_validate_addr = eth_validate_addr,
        .ndo_change_mtu = eth_change_mtu,
};

static void __init hw_dev_setup(struct net_device *dev)
{ 
    /* Using Ethernet mode by default */
    dev->netdev_ops = &hw_dev_ops;
    ether_setup(dev);

    /* set this after calling ether_setup */
    dev->mtu = RMNET_DATA_LEN;
    dev->needed_headroom = HEADROOM_FOR_BAM;
    dev->needed_tailroom = TAILROOM;
    random_ether_addr(dev->dev_addr);

    dev->watchdog_timeo = 1000; /* 10 seconds? */
}

static int __init hw_rmnet_wan_init(void)
{
    int ret;
    int i = 0;
    int j = 0;
    static struct net_device *dev = NULL;
    struct hw_dev_private *p = NULL;
    
    for(i = 0; i < HW_DEV_NUM; i++)
    {
        dev = alloc_netdev(sizeof(struct hw_dev_private), hw_dev_info[i].hw_dev_name, hw_dev_setup);

        if (NULL == dev)
        {
            /*�ͷ�����ǰ���Ѿ����뵽��dev*/
            if(i != 0)
            {
                for(j = 0; j < i; j++)
                {
                    free_netdev(hw_dev_info[j].hw_dev);
                }
            }
            return -ENOMEM;
        }
        memcpy(dev->dev_addr, hw_dev_info[i].ipv4_mac_head, MAC_HEAD_LENGTH);
        dev_net_set(dev, &init_net);
        ret = register_netdev(dev);
        if (ret) 
        {
            free_netdev(dev);
            /*�ͷ�����ǰ���Ѿ����뵽��dev*/
            for(j = 0; j < i; j++)
            {
                free_netdev(hw_dev_info[j].hw_dev);
            }
            return ret;
        }
        
        ret = sysfs_create_group(&(dev->dev.kobj), &hw_dev_group);
        
        if(HW_DEV_ID_ONDEMAND == i)
        {
#ifdef CONFIG_ATP_QCT_ONDEMAND_DIAL
            RNIC_InitDemandDialFile();
#endif
        }
        
        p = netdev_priv(dev);
        p->hw_dev_id = i;
        hw_dev_info[i].hw_dev = dev;
    }

    return ret;
}

static void __exit hw_rmnet_wan_cleanup(void)
{
    int i = 0;

    for(i = 0; i < HW_DEV_NUM; i++)
    {
        if(NULL != hw_dev_info[i].hw_dev)
        {
            unregister_netdev(hw_dev_info[i].hw_dev);
            free_netdev(hw_dev_info[i].hw_dev);
        }
    }
}

module_init(hw_rmnet_wan_init);
module_exit(hw_rmnet_wan_cleanup);
