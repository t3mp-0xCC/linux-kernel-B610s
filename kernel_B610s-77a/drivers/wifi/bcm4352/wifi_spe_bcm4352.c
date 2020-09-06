/******************************************************************************
  *         版权所有 (C), 2001-2020, 华为技术有限公司
*******************************************************************************/


#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/dma-mapping.h>

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
#include <linux/spe/spe_interface.h>
#include <../../hisi/spe/spe_drv/spe.h>

#if (defined(FEATURE_HUAWEI_MBB_BCM4352) || defined(FEATURE_HUAWEI_MBB_BCM4360))
#define WIFI_CHIP_SUM    2
#define WLAN_SPE_2G_IFNAME   "wl1"
#define WLAN_SPE_5G_IFNAME   "wl0"
#else
/* B612 只有2.4G WiFi，需要修改WiFi数以及wl的名字。为了保证编译通过，WLAN_SPE_5G_IFNAME也必须定义 */
#define WIFI_CHIP_SUM    1
#define WLAN_SPE_2G_IFNAME   "wl0"
#define WLAN_SPE_5G_IFNAME   "wl1"
#endif

#define CPE_WIFI_SKB_HEADROOM   256
#define CPE_WIFI_DMA_LEN        1800

struct wifi_spe_attr
{
    struct net_device * ndev;
    int portno;
    int initialized;
    struct spe_port_attr wifi_attr;
};

static struct wifi_spe_attr wl_spe_attr[WIFI_CHIP_SUM];

static spinlock_t wl_spe_create_lock;

#define SPE_SKB_DUMP(skb)   \
    do{\
        	int len = min((int)64, (int)(skb->len));\
            print_hex_dump(KERN_ERR, "skb dump: ", DUMP_PREFIX_ADDRESS, \
                16, 1, (skb->data), len, false);\
            printk(KERN_ERR"\n");\
    }while(0)


static struct spe spe_balong_local;

extern void mdrv_spe_get_spe_balong(struct spe *spe_copy);
    
static int wifi_finish_rd(int portno, int src_portno, struct sk_buff *skb, 
dma_addr_t dma, unsigned int flags)
{
    struct net_device *dev;
    struct spe *spe = &spe_balong_local;

	BUG_ON(!spe_hook.port_netdev);
	dev = spe_hook.port_netdev(portno);

    if (unlikely(NULL != dev) 
     && unlikely(NULL != skb))
    {
        skb->dev = dev;

        dma_unmap_single(spe->dev, dma, skb->len, DMA_FROM_DEVICE);

        return dev->netdev_ops->ndo_start_xmit(skb, dev);
    }
    
    return -1;
}

static int wifi_finish_td(int portno, struct sk_buff *skb, unsigned int flags)
{
    int ret = 0;
    struct net_device *dev;
    dma_addr_t dma;
	struct spe *spe = &spe_balong_local;

    BUG_ON(!spe_hook.port_netdev);
	dev = spe_hook.port_netdev(portno);

    dma = spe_hook.get_skb_dma(skb);
		
    if (unlikely(NULL != dev) 
     && unlikely(NULL != skb))
    {		
		dma_unmap_single(spe->dev, dma, skb->len, DMA_TO_DEVICE);
        dev_kfree_skb_any(skb);
    }
    
    return ret;
}

void wifi_bcm_spe_register(struct net_device * ndev,  uint32_t skb_size, uint32_t skb_num)
{
    int idx = 0;

    if (unlikely(NULL == ndev))
    {
        return;
    }


    if (0 != strcmp(ndev->name, WLAN_SPE_2G_IFNAME)
     && 0 != strcmp(ndev->name, WLAN_SPE_5G_IFNAME))
    {
        printk(KERN_ERR"[Wi-Fi SPE] do not register SPE for %s\n", ndev->name);
        return;
    }

    spin_lock(&(wl_spe_create_lock));
        
    if(spe_hook.is_enable && spe_hook.is_enable())
    {
        for (idx = 0; idx < WIFI_CHIP_SUM; idx++)
        {
            if (!wl_spe_attr[idx].initialized)
            {
                break;
            }
        }

        if (idx >= WIFI_CHIP_SUM)
        {
            printk("[Wi-Fi SPE] no more spe space for register %s\n", ndev->name);
            goto EXIT;
        }
        
        wl_spe_attr[idx].wifi_attr.enc_type = spe_enc_none;
        wl_spe_attr[idx].wifi_attr.attach_brg = spe_attach_brg_normal;
        wl_spe_attr[idx].wifi_attr.rd_depth = 2048;
        wl_spe_attr[idx].wifi_attr.td_depth = 2048;    /*same as rx */
        wl_spe_attr[idx].wifi_attr.rd_skb_size = 1800;
        wl_spe_attr[idx].wifi_attr.rd_skb_num = 2048;
        wl_spe_attr[idx].wifi_attr.desc_ops.finish_rd = wifi_finish_rd;
        wl_spe_attr[idx].wifi_attr.desc_ops.finish_td = wifi_finish_td;
        wl_spe_attr[idx].wifi_attr.net = ndev;
        
        wl_spe_attr[idx].portno = spe_hook.port_alloc(&(wl_spe_attr[idx].wifi_attr));
        wl_spe_attr[idx].ndev = ndev;
        wl_spe_attr[idx].initialized = 1;
		
		mdrv_spe_get_spe_balong(&spe_balong_local);

        printk("[Wi-Fi SPE]register:%s portno:%d skb_size:%d skb_num:%d\n", ndev->name, 
                  wl_spe_attr[idx].portno, wl_spe_attr[idx].wifi_attr.rd_skb_size, 
				  wl_spe_attr[idx].wifi_attr.rd_skb_num);
    }

EXIT:
    spin_unlock(&(wl_spe_create_lock));
}

int wifi_bcm_spe_td_config(struct net_device *ndev, struct sk_buff *skb)
{
    int i = 0;
    int idx = 0;
    int ret = 0;
	__be16 usprotocol = 0;
    dma_addr_t dma;
	struct spe *spe = &spe_balong_local;
    
    if (unlikely(NULL == ndev))
    {
        return -1;
    }

    if (unlikely(NULL == skb))
    {
        return -1;
    }
	
    i = 12;
    usprotocol = *(__be16*) & (skb->data[i]);
    
	/* 下行包带vlan的场景 */
    while (htons(ETH_P_8021Q) == usprotocol)
    {
        i += 4; 
        usprotocol = *(__be16*) & (skb->data[i]);
    }
       
    if (usprotocol != htons(ETH_P_IP)
	 && usprotocol != htons(ETH_P_IPV6))
	{
	    return -1;
	}

    if(spe_hook.is_enable && spe_hook.is_enable())
    {
        for (idx = 0; idx < WIFI_CHIP_SUM; idx++)
        {
            if (wl_spe_attr[idx].initialized 
             && wl_spe_attr[idx].ndev == ndev)
            {
			    dma = dma_map_single(spe->dev, skb->data,
				        skb->len, DMA_TO_DEVICE);
						    
               spe_hook.set_skb_dma(skb, dma);
                
                ret = spe_hook.td_config(wl_spe_attr[idx].portno, skb, dma, spe_l3_bottom, 0);
                if (0 != ret) 
                {
                    printk("[Wi-Fi SPE]: td fail, ret:%d\n", ret);
                    return -1;
                }

                return 0;
            }
        }
    }

    return -1;
}

int wifi_bcm_spe_rd_config(struct net_device *ndev, struct sk_buff *skb)
{
    int idx = 0;
    int ret = 0;
    dma_addr_t dma;
    struct spe *spe = &spe_balong_local;
    
    if (likely(NULL == ndev) && unlikely(NULL == skb))
    {
        return -1;
    }

    if(spe_hook.is_enable && spe_hook.is_enable())
	{
        if (skb->spe_own)
        {
            skb->data = skb->head;
            skb_reset_tail_pointer(skb);
            skb_reserve(skb, CPE_WIFI_SKB_HEADROOM);

            dma = dma_map_single(spe->dev, skb->data,
			     			skb->len,
				    		DMA_FROM_DEVICE);

             spe_hook.set_skb_dma(skb, dma);

             skb->priority = 0;

             spe_hook.rd_config((int)(skb->spe_own), skb, dma);
             return 0;
	     }
    }

    return -1;
}

void wifi_bcm_spe_unregister(struct net_device *ndev)
{
    int idx = 0;
    
    if (unlikely(NULL == ndev))
    {
        return;
    }

    spin_lock(&(wl_spe_create_lock));
        
    if(spe_hook.is_enable && spe_hook.is_enable())
    {
        for (idx = 0; idx < WIFI_CHIP_SUM; idx++)
        {
            if (wl_spe_attr[idx].initialized 
             && wl_spe_attr[idx].ndev == ndev)
            {      
		        (void)spe_hook.port_free(wl_spe_attr[idx].portno);
            }
        }
	}

    spin_unlock(&(wl_spe_create_lock));
}

void wifi_bcm_spe_port_enable(struct net_device * ndev)
{
    int idx = 0;
    int ret = 0;
    
    if (unlikely(NULL == ndev))
    {
        printk("[Wi-Fi SPE]: Error: PORT_ENABLE DEV is NULL\n");
        return -1;
    }

    if(spe_hook.is_enable && spe_hook.is_enable())
    {
        for (idx = 0; idx < WIFI_CHIP_SUM; idx++)
        {
            if (wl_spe_attr[idx].initialized 
             && wl_spe_attr[idx].ndev == ndev)
            {
                ret = spe_hook.port_enable(wl_spe_attr[idx].portno);

                printk("[Wi-Fi SPE] port enable for %s portno:%d ret:%d\n", ndev->name, wl_spe_attr[idx].portno, ret);
            }
        }
    }
}

void wifi_bcm_spe_port_disable(struct net_device * ndev)
{
    int idx = 0;
    int ret = 0;
    
    if (unlikely(NULL == ndev))
    {
        printk("[Wi-Fi SPE]: Error: PORT_DISABLE DEV is NULL\n");
        return;
    }

    if(spe_hook.is_enable && spe_hook.is_enable())
    {
        for (idx = 0; idx < WIFI_CHIP_SUM; idx++)
        {
            if (wl_spe_attr[idx].initialized 
             && wl_spe_attr[idx].ndev == ndev)
            {
                ret = spe_hook.port_disable(wl_spe_attr[idx].portno);

                printk("[Wi-Fi SPE] port disable for %s portno:%d ret:%d\n", ndev->name, wl_spe_attr[idx].portno, ret);
            }
        }
    }
}

static int get_wifi_intf_addr(const char *str, u8 *dev_addr)
{
	if (str) {
		unsigned	i;

		for (i = 0; i < 6; i++) {
			unsigned char num;

			if ((*str == '.') || (*str == ':'))
				str++;
			num = hex_to_bin(*str++) << 4;
			num |= hex_to_bin(*str++);
			dev_addr [i] = num;
		}
		if (is_valid_ether_addr(dev_addr))
			return 0;
	}
	return 1;
}

static int get_wifi_intf_str_addr(u8 *dev_addr, u8 *macAddr)
{
    u8 low, high;
    unsigned  i, j;
    
    for (i = 0, j = 0; i< 6; i++)
    {
        low = dev_addr[i] & 0x0f;
        high = (dev_addr[i] >> 4) & 0x0f;

        if (low <= 9)
        {
            low = '0' + low;
        }
        else
        {
            low = low - 10;
            low = 'a' + low;
        }

        if (high <= 9)
        {
            high = '0' + high;
        }
        else
        {
            high = high - 10;
            high = 'a' + high;
        }

        macAddr[j++] = high;
        macAddr[j++] = low;
        macAddr[j++] = ':';
    }

    macAddr[j-1] = '\0';

    return 0;
}


//7、设置wifi mac地址的时候
void wifi_bcm_spe_set_macaddr(struct net_device * ndev,  char *macaddr)
{
    int idx = 0;
    int ret = 0;
    u8 macpool[32] = {0};
    u8 mac_addr[32] = {0};
   
    if (unlikely(NULL == ndev))
    {
        return;
    }

    if (unlikely(NULL == macaddr))
    {
        return;
    }

    memset(macpool, 0, sizeof(macpool));
    memset(mac_addr, 0, sizeof(mac_addr));

    get_wifi_intf_str_addr(macaddr, mac_addr);
    
    if(spe_hook.is_enable && spe_hook.is_enable())
    {
        for (idx = 0; idx < WIFI_CHIP_SUM; idx++)
        {
            if (wl_spe_attr[idx].initialized 
             && wl_spe_attr[idx].ndev == ndev)
            {
                spe_hook.port_ioctl(wl_spe_attr[idx].portno, spe_port_ioctl_set_mac, macaddr);
            }
        }
    }
}

void wifi_bcm_spe_set_skb_dma(struct sk_buff *skb, dma_addr_t dma)
{
    if (NULL == skb)
    {
        return;
    }

    if(spe_hook.is_enable && spe_hook.is_enable())
    {
        spe_hook.set_skb_dma(skb, dma);
    }

    return;
}

static int __init wifi_spe_init(void)
{
    
    memset(wl_spe_attr, 0, sizeof(wl_spe_attr));
    
    spin_lock_init(&wl_spe_create_lock);
	
	mdrv_spe_get_spe_balong(&spe_balong_local);

    printk("[Wi-Fi SPE] module init!\n");

    return 0;
}

/*****************************************************************************
 函数名称  : int wifi_spe_exit(void)
 功能描述  : 驱动spe初始化
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
*****************************************************************************/
static void __exit wifi_spe_exit(void)
{
    int idx = 0;

    for (idx = 0; idx < WIFI_CHIP_SUM; idx ++)
    {
        (void)spe_hook.port_free(wl_spe_attr[idx].portno);
    }
        
    memset(wl_spe_attr, 0, sizeof(wl_spe_attr));

    printk("[Wi-Fi SPE] module exit!\n");
    
    return;
}

#else

static int wifi_finish_rd(int portno, int src_portno, struct sk_buff *skb, 
dma_addr_t dma, unsigned int flags)
{
    return -1;
}

static int wifi_finish_td(int portno, struct sk_buff *skb, unsigned int flags)
{
    return -1;
}

void wifi_bcm_spe_register(struct net_device * ndev,  uint32_t skb_size, uint32_t skb_num)
{
    return; 
}

int wifi_bcm_spe_td_config(struct net_device *ndev, struct sk_buff *skb)
{
    return -1;
}

int wifi_bcm_spe_rd_config(struct net_device *ndev, struct sk_buff *skb)
{
    return -1;
}

void wifi_bcm_spe_unregister(struct net_device *ndev)
{
    return;
}

void wifi_bcm_spe_port_enable(struct net_device * ndev)
{
    return;
}

void wifi_bcm_spe_port_disable(struct net_device * ndev)
{
    return;
}

void wifi_bcm_spe_set_macaddr(struct net_device * ndev,  char *macaddr)
{
    return;
}

void wifi_bcm_spe_set_skb_dma(struct sk_buff *skb, dma_addr_t dma)
{
    return;
}

static int __init wifi_spe_init(void)
{
    return 0;
}

static void __exit wifi_spe_exit(void)
{    
    return;
}
#endif
module_init(wifi_spe_init); /*lint !e529*/
module_exit(wifi_spe_exit); /*lint !e529*/

EXPORT_SYMBOL(wifi_bcm_spe_register);
EXPORT_SYMBOL(wifi_bcm_spe_td_config);
EXPORT_SYMBOL(wifi_bcm_spe_rd_config);
EXPORT_SYMBOL(wifi_bcm_spe_unregister);
EXPORT_SYMBOL(wifi_bcm_spe_port_enable);
EXPORT_SYMBOL(wifi_bcm_spe_port_disable);
EXPORT_SYMBOL(wifi_bcm_spe_set_macaddr);
EXPORT_SYMBOL(wifi_bcm_spe_set_skb_dma);

