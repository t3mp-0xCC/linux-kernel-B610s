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
 
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/rwlock_types.h>
#include <linux/rwlock.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/phy.h>
#include <linux/ethtool.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include "mbb_net.h"
#include "SysNvId.h"
#include <bsp_nvim.h>
#include <mdrv.h>
#include <linux/dma-mapping.h>
/*MBB_FEATURE_ETH_PHY与MBB_FEATURE_ETH_SWITCH不能同时打开*/
#if (FEATURE_ON == MBB_FEATURE_ETH_PHY)
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
Pls_check_MBB_FEATURE_ETH_SWITCH_or_MBB_FEATURE_ETH_PHY
#endif
#endif

#if (FEATURE_ON == MBB_CTF_COMMON)
#include <linux/fake/typedefs.h>
#include <linux/fake/osl.h>
#include <linux/fake/linux_osl.h>
#include <linux/fake/ctf/hndctf.h>

static ctf_t *eth_cih __attribute__ ((aligned(32))) = NULL;/* ctf instance handle */
static osl_t *eth_fake_osh = NULL;
extern unsigned int g_ifctf;
#endif
#define   REPORT_ETH_REMOVE      (0)
#define   REPORT_ETH_INSERT      (1)
#define   REPORT_POWEROFF_EN     (2)
#define   REPORT_POWEROFF_DIS    (3)
#define   ETH_ROUTE_STATE_UNKNOWN    (0xf)

#if ((FEATURE_ON == MBB_FEATURE_FASTIP) && (FEATURE_ON == MBB_ETH_STMMAC_TX_THREAD))
#include <linux/if.h>
#include <linux/if_vlan.h>
#include <linux/fastip/fastip.h>
#include <linux/ip.h>
fastip_t  __attribute__ ((aligned(32))) *eth_fastip_handle  = NULL;
#endif
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
#define   WAN_PORT_INDEX     3
#define   NET_DEVICE_NAME   "et0"
#else
#define   NET_DEVICE_NAME   "eth0"
#endif

#define ETH_TRAFFIC_TIMES   3

/*多长时间查询端口的状态，单位为秒*/
#define PHY_LINK_TIME           1

/*组播地址的首字节为0x01*/
#define MUTICAST_MAC            0x01

/*目标端口的偏移量*/
#define DPORT_BYMASS             36

/*约定组播升级的目的端口为13456*/
#define MUTICAST_DPORT_1        0x34 
#define MUTICAST_DPORT_2        0x90

/*组播升级的MAGIC_NUMBER的偏移量*/
#define MAGIC_NUMBER_BYMASS     46 

/*组播报文魔数字*/
#define MAGIC_NUMBER            0xB0A555AA

#define MAC_CLONE_OFF           "OFF"
#define SZ_MAC_LENGTH           20   
#define MAC_CLONE_ENABLE        1
#define MAC_CLONE_DISABLE       0

/*ifctf的文件内容最大长度*/
#define IF_CTF_LENGTH           1
/*十进制*/
#define DECIMAL                 10

#define SYS_NODE_INT_BUF_LEN    6

#define LAN_PORT_1              1
#define LAN_PORT_2              2
#define LAN_PORT_3              3
#define LAN_PORT_4              4
#define NV_MAC_LENGTH           32

/*读写锁，避免全局变量被并发访问*/
rwlock_t mac_clone_lock; 

/*保存从用户空间传入的冒号分隔形式的用于克隆MAC地址*/
char sz_clone_macaddr[SZ_MAC_LENGTH] = {"00:00:00:00:00:00"};
char clone_mac_addr[ETH_ALEN] = {0};
char clone_init_addr[ETH_ALEN] = {0};
int  mac_clone_flag = MAC_CLONE_DISABLE;

/*标志当前是否检测到组播升级 0表示未检测到 1表示检测到*/
int g_muticast_flag = 0;

/*标志当前LAN侧是否存在流量*/
int g_traffic_flag = 0;

#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
extern void rtk_reset_allport(int wan_exclude);
extern int rtk_poll_port_status(unsigned int *all_port_status);
#endif
/*查询eth端口状态的work*/
struct delayed_work s_ethstatusqueue;

static struct class *lan_class;
static struct device *lan_dev;

static int translate_mac_address(char *adr_str, char *adr_dst);
static void mbb_device_event_report(int event);

/*****************************************************************************
函数名：   get_aneg_speed
功能描述:  获取CPE 网口指定端口速率
输入参数:  端口索引
返回值：  速率值(1000/100/10) 
*****************************************************************************/
static int get_aneg_speed(int portIndex)
{
    struct phy_device *phydev = mbb_get_phy_device();
    int retv = 0;

    if (NULL != phydev)
    {
        if (phydev->drv->read_status)
        {
            phydev->drv->read_status(phydev);
        }

        if (phydev->link)
        {
            retv = phydev->speed;
        }
    }

    return retv;
}

/*****************************************************************************
函数名：   PhyATQryPortPara
功能描述:  装备单网口信息获取接口
输入参数:  端口索引
返回值：  NET_RET_OK(0)为ok 
*****************************************************************************/
int PhyATQryPortPara(NET_PORT_ST *PortPara)
{
#if (FEATURE_ON == MBB_FEATURE_ETH_PHY)
    if (NULL == PortPara)
    {
        return NET_RET_FAIL;
    }
    
    PortPara->total_port = LAN_PORT_1;
    PortPara->port_index = 0;
    PortPara->port_rate = get_aneg_speed(PortPara->port_index);

    return NET_RET_OK;
#elif (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
    unsigned char uchIndex = 0;
    LAN_ETH_STATE_INFO_ST EthInfo[LAN_PORT_4];

    if (NULL == PortPara)
    {
        return NET_RET_FAIL;
    }

    memset(EthInfo, 0, sizeof(EthInfo));
    rtk_get_port_status_info(EthInfo, 0, LAN_PORT_4);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH_8363)
    for (uchIndex=0; uchIndex<LAN_PORT_2; uchIndex++, PortPara++)
    {
        PortPara->total_port = LAN_PORT_2;
        PortPara->port_index = uchIndex;
        if (1 == EthInfo[2*uchIndex + 1].link_state)
        {
            PortPara->port_rate = EthInfo[2*uchIndex + 1].speed;
        }
        else
        {
            PortPara->port_rate = 0;
        }
    }
#else
    for (uchIndex=0; uchIndex<LAN_PORT_4; uchIndex++, PortPara++)
    {
        PortPara->total_port = LAN_PORT_4;
        PortPara->port_index = uchIndex;
        if (1 == EthInfo[uchIndex].link_state)
        {
            PortPara->port_rate = EthInfo[uchIndex].speed;
        }
        else
        {
            PortPara->port_rate = 0;
        }
    }
#endif
    return NET_RET_OK;
#else
    return NET_RET_FAIL;
#endif
}

/*****************************************************************************
函数名：   mbb_check_net_upgrade
功能描述:  判断报文是否是组播升级报文
输入参数:  skb报文
返回值：  NET_RET_OK(0)为ok 
*****************************************************************************/
int mbb_check_net_upgrade(struct sk_buff *skb)
{
    int iret = 0;
    unsigned short vlanid = 0;
    int vlanLen = 0;

    if (NULL == skb)
    {
        return NET_RET_FAIL;
    }

    /*在此判断一下是否为组播升级的报文*/
    if (0 == g_muticast_flag)
    {
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
        vlanLen = VLAN_HLEN;
#endif
    /*如果第一个地址为组播地址*/
        if (MUTICAST_MAC == (skb->data[0] & MUTICAST_MAC))
        {
            /*比较端口号  两个字节*/
            if ((MUTICAST_DPORT_1 == skb->data[DPORT_BYMASS + vlanLen])
            && (MUTICAST_DPORT_2 == skb->data[DPORT_BYMASS + vlanLen + 1]))
            {
                /*比较魔数字 四个字节*/
                if (MAGIC_NUMBER == (*((unsigned int *)(skb->data + MAGIC_NUMBER_BYMASS + vlanLen))))
                {
                    printk("the packet is for muticast\n");
                    mbb_device_event_report(CRADLE_MUTI_CAST);

                    g_muticast_flag = 1;
                }
            }
        }
    }

    return NET_RET_OK;
}

/*****************************************************************************
函数名：   mbb_mac_clone_rx_restore
功能描述:  MAC层接收PPP报文时mac clone
输入参数:  skb报文
返回值：  NET_RET_OK(0)为ok 
*****************************************************************************/
int mbb_mac_clone_rx_restore(struct sk_buff *skb)
{
    int L3Offset = ETH_HLEN;

    if (NULL == skb)
    {
        return NET_RET_FAIL;
    }
    
    read_lock(&mac_clone_lock);

    if (MAC_CLONE_ENABLE == mac_clone_flag)
    {
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
        L3Offset = VLAN_ETH_HLEN - sizeof(__be16);
#endif 

#if (FEATURE_ON == MBB_FEATURE_ETH_PHY)
        L3Offset = ETH_HLEN - sizeof(__be16);
#endif  

        /*如果是PPP发现报文或者会话报文就用克隆的MAC地址去替换*/
        if (((cpu_to_be16(ETH_P_PPP_DISC) == (*(unsigned short *)(skb->data + L3Offset)))
        || (cpu_to_be16(ETH_P_PPP_SES) == (*(unsigned short *)(skb->data + L3Offset))))
        && (0 == strncmp(skb->data, clone_mac_addr, ETH_ALEN)))
        {
            //LAN_DEBUG("mac clone in rx\r\n");
            memcpy(skb->data, clone_init_addr, ETH_ALEN); 
        }
    }

    read_unlock(&mac_clone_lock);

    return NET_RET_OK;
}

/*****************************************************************************
函数名：   mbb_mac_clone_tx_save
功能描述:  MAC层发送PPP报文时mac clone
输入参数:  skb报文
返回值：  NET_RET_OK(0)为ok 
*****************************************************************************/
int mbb_mac_clone_tx_save(struct sk_buff *skb)
{
    int L3Offset = ETH_HLEN;

    if (NULL == skb)
    {
        return NET_RET_FAIL;
    }

    read_lock(&mac_clone_lock);  

    if (MAC_CLONE_ENABLE == mac_clone_flag)
    {
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
        L3Offset = VLAN_ETH_HLEN - sizeof(__be16);
#endif 

#if (FEATURE_ON == MBB_FEATURE_ETH_PHY)
        L3Offset = ETH_HLEN - sizeof(__be16);
#endif

        /*如果是PPP发现报文或者会话报文就用克隆的MAC地址去替换*/
        if ((cpu_to_be16(ETH_P_PPP_DISC) == (*(unsigned short *)(skb->data + L3Offset)))
        || (cpu_to_be16(ETH_P_PPP_SES) == (*(unsigned short *)(skb->data + L3Offset))))
        {           
            //LAN_DEBUG("mac clone in tx\r\n");
            memcpy(clone_init_addr, skb->data + ETH_ALEN, ETH_ALEN);  
            memcpy(skb->data + ETH_ALEN, clone_mac_addr, ETH_ALEN);
        }
    }

    read_unlock(&mac_clone_lock);

    return NET_RET_OK;
}

static int wan_mirror_port = 0;
static int wan_mirror_enable = 0;

extern struct net init_net;
#if (FEATURE_ON == MBB_FEATURE_ETH_WAN_MIRROR)
#if ((FEATURE_ON == MBB_CTF_COMMON) || (FEATURE_ON == MBB_FEATURE_FASTIP)) 
extern void set_ctf_wan_mirror_flags(int value);
#endif
#endif

/*****************************************************************************
函数名：   RNIC_WANMirror
功能描述:  打开WAN口镜像
输入参数:  enable使能与否
                        port端口号
返回值：  NET_RET_OK(0)为ok 
*****************************************************************************/
#if (FEATURE_ON == MBB_BUILD_DEBUG)

int RNIC_WANMirror(unsigned int enable, unsigned int port)
{   
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
    if (port < LAN_PORT_1 || port > LAN_PORT_4)
    {        
        printk("port is illegal\n");
        return NET_RET_FAIL;
    }
#endif        

    wan_mirror_port = port;
    wan_mirror_enable = enable;
    
#if (FEATURE_ON == MBB_FEATURE_ETH_WAN_MIRROR)
#if ((FEATURE_ON == MBB_CTF_COMMON) || (FEATURE_ON == MBB_FEATURE_FASTIP))
    set_ctf_wan_mirror_flags(enable);
#endif
#endif
    return NET_RET_OK;
}

#else

int RNIC_WANMirror(unsigned int enable, unsigned int port)
{  
    return NET_RET_OK;
}

#endif/*MBB_BUILD_DEBUG*/ 

int RNIC_WanMirrorStatus()
{
    return wan_mirror_enable;
}
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))  
#define RNIC_SPE_WPORT_CACHE_HDR_SIZE        (sizeof(struct ethhdr))
#endif

#if (defined(CONFIG_BALONG_SPE))
unsigned int RNIC_IsSpeMemMirror(struct sk_buff *skb)
{
    return (0 != skb->spe_own);
}

dma_addr_t RNIC_GetMemDmaMirror(struct sk_buff *skb)
{
    return RNIC_SPE_MEM_CB_MIRROR(skb);
}

void  RNIC_SpeMemMapRequsetMirror(struct sk_buff *skb, unsigned int ulLen)
{
    unsigned char                      *pucData = NULL;
    dma_addr_t                          ulDmaAddr;

    ulDmaAddr = RNIC_GetMemDmaMirror(skb);
    pucData   = phys_to_virt(ulDmaAddr);
    dma_map_single(NULL, pucData, ulLen, DMA_FROM_DEVICE);
    return;
}

void RNIC_SpeMemUnmapRequsetMirror(struct sk_buff *skb, unsigned int ulLen)
{
    dma_addr_t                          ulDmaAddr;

    ulDmaAddr = RNIC_GetMemDmaMirror(skb);
    dma_unmap_single(NULL, ulDmaAddr, ulLen, DMA_FROM_DEVICE);
    return;
}
#endif

/*****************************************************************************
函数名：   RNIC_Map_To_Lan_Forward
功能描述:  WAN口镜像
输入参数:  skb报文
返回值：  NET_RET_OK(0)为ok 
*****************************************************************************/
int RNIC_Map_To_Lan_Forward(struct sk_buff *skb, void *pdata, int len)
{
    struct net_device *dev = NULL;
    struct sk_buff *skb2 = NULL;
    int ret = 0;

#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH) 
    int vlan_id = 0;
#endif

    if (!wan_mirror_enable)
    {
        return NET_RET_FAIL;
    }

    if (NULL == skb)
    {
        return NET_RET_FAIL;
    }  

    /*打开mirror需要关闭ctf*/
#if (FEATURE_ON == MBB_CTF_COMMON)
    if (!g_ifctf)
    {
        g_ifctf = 1;
    }
#endif    
    RNIC_SPE_MEM_UNMAP_MIRROR(skb, skb->len);           
    skb2 = skb_copy(skb, GFP_ATOMIC);
    RNIC_SPE_MEM_MAP_MIRROR(skb, skb->len); 
    if(skb2 == NULL)
    {
        printk("\r\n%s %d skb_copy failed",__FUNCTION__,__LINE__);
        return NET_RET_FAIL;
    }
    RNIC_SPE_SKBOWN_RESET(skb2);
    /* copy mac header if necessary */
    if((pdata)&&(0 < len))
    {
        memcpy(skb2->data, (void *)pdata, len);        
    }    

#if (FEATURE_ON== MBB_FEATURE_ETH_SWITCH_8363)
    if ((LAN_PORT_2 != wan_mirror_port) && (LAN_PORT_4 != wan_mirror_port))
    {
        printk("\r\nwan mirror get dst failed");
        kfree_skb(skb2);
        return NET_RET_FAIL;
    }
#endif
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
    /*获取镜像的lan口设备节点*/
    switch(wan_mirror_port)
    {
        case LAN_PORT_1:
            dev = __dev_get_by_name(&init_net, "eth0.2");
            vlan_id = VLAN_PORT0;
            break;
        case LAN_PORT_2:
            dev = __dev_get_by_name(&init_net, "eth0.3");            
            vlan_id = VLAN_PORT1;
            break;
        case LAN_PORT_3:
            dev = __dev_get_by_name(&init_net, "eth0.4");
            vlan_id = VLAN_PORT2;
            break;
        case LAN_PORT_4:
            dev = __dev_get_by_name(&init_net, "eth0");
            vlan_id = VLAN_WAN_PORT;
            break;
        default:
            printk("\r\nwan mirror get dst failed");
            kfree_skb(skb2);
            return NET_RET_FAIL;
    }
#endif

#if (FEATURE_ON == MBB_FEATURE_ETH_PHY) 
    dev = __dev_get_by_name(&init_net, "eth0");
#endif

    if(dev)
    {    
        unsigned long flags;
        
        /*发送前的准备工作*/
        skb2->dev = dev;

        /*ether协议头*/
        skb2->protocol = htons(ETH_P_802_3);

        /*伪造组播报文，防止PC wireshark抓不到报文*/
        skb2->data[0] |= 0x01;

        local_irq_save(flags);
        local_irq_enable();
        if (skb2)
        {
            dev_queue_xmit(skb2);
        }
        local_irq_restore(flags);
                 
        return NET_RET_OK;
    }
    else
    {
        printk("\r\n%s %d didn't find port %d dev", __FUNCTION__, __LINE__, wan_mirror_port);
        kfree_skb(skb2);

        return NET_RET_FAIL;
    }

}

/*****************************************************************************
函数名：   get_carrier_state
功能描述: net_state节点处理函数
输入参数:  
返回值：
*****************************************************************************/
static ssize_t get_carrier_state(struct device *dev, struct device_attribute *attr, char *buf)
{
    int port_line_state = CRADLE_REMOVE;

#if (FEATURE_ON == MBB_FEATURE_ETH_PHY) 
    struct phy_device *phydev = mbb_get_phy_device();

    if (NULL != phydev)
    {
        if (phydev->drv->read_status)
        {
            phydev->drv->read_status(phydev);
        }

        if (phydev->link)
        {
            port_line_state = CRADLE_INSERT;
        }    
    }
#endif    

#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH) 
    LAN_ETH_STATE_INFO_ST wan_status;

    rtk_get_port_status_info(&wan_status, WAN_PORT_INDEX, 1);

    if (wan_status.link_state)
    {
        port_line_state = CRADLE_INSERT;
    }
#endif

    return snprintf(buf, SYS_NODE_INT_BUF_LEN, "%0x\n", port_line_state); 
}
 
static DEVICE_ATTR(net_state, S_IRUGO, get_carrier_state, NULL);
 
/*****************************************************************************
函数名：   set_clone_mac
功能描述: 文件clone_mac状态发生变化时，调用此函数判断是否开启MAC地址克隆功能
输入参数:  
返回值：
*****************************************************************************/
static ssize_t set_clone_mac(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    char sz_macaddr[SZ_MAC_LENGTH] = {0};

    strncpy(sz_clone_macaddr, buf, SZ_MAC_LENGTH - 1);
    strncpy(sz_macaddr, sz_clone_macaddr, SZ_MAC_LENGTH - 1);
    write_lock(&mac_clone_lock);
    mac_clone_flag = MAC_CLONE_DISABLE; /*先设置为0*/

    if (strncmp(buf, MAC_CLONE_OFF, strlen(MAC_CLONE_OFF)))
    {
        /*将冒号间隔的字符串格式MAC地址转换为6字节格式*/
        if (0 == translate_mac_address(sz_macaddr, clone_mac_addr))
        {
            mac_clone_flag = MAC_CLONE_ENABLE;
        }  
    }

    write_unlock(&mac_clone_lock);

    return count;
}

/*****************************************************************************
函数名：  get_clone_mac
功能描述: 获取设置的clone mac
输入参数:  
返回值：
*****************************************************************************/
static ssize_t get_clone_mac(struct device *dev, struct device_attribute *attr, char *buf)
{
    return snprintf(buf, SZ_MAC_LENGTH, "%s\n", sz_clone_macaddr);
}

static DEVICE_ATTR(clone_mac, S_IRUGO | S_IWUSR, get_clone_mac, set_clone_mac);
 

/*****************************************************************************
函数名：  get_route_state
功能描述: 获取网口状态wan/lan
输入参数:
返回值：1:wan 0:lan 0xf:unknown
*****************************************************************************/
unsigned int eth_route_state = 0xf;
static ssize_t get_route_state(struct device *dev,
                struct device_attribute *attr, char *buf)
{
    return snprintf(buf, SYS_NODE_INT_BUF_LEN, "%d\n", eth_route_state);
}
/*****************************************************************************
函数名：  set_route_state
功能描述: 设置网口状态wan/lan
输入参数:
返回值：1:wan 0:lan
*****************************************************************************/
static ssize_t set_route_state(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t count)
{
    char value = *buf;
    if(value == '1')
    {
        eth_route_state = 1;
    }
    else if(value == '0')
    {
        eth_route_state = 0;
    }
    else
    {
        eth_route_state = ETH_ROUTE_STATE_UNKNOWN;
        printk("error : input state unknown\n");
    }

    printk("eth_route_state is %d\n", eth_route_state);
    return count;
}
static DEVICE_ATTR(route_state, S_IRUGO | S_IWUSR, get_route_state, set_route_state);
int eth_check_wan()
{
    return eth_route_state;
}
void set_eth_lan()
{
    eth_route_state = ETH_ROUTE_STATE_UNKNOWN;
    printk("when suspend or remove, set eth route state unknown\n");
}
EXPORT_SYMBOL(eth_check_wan);
/*****************************************************************************
函数名：  set_power_state
功能描述: 通过结点来对网口phy芯片上下电
输入参数:1：上电   0：下电
返回值：
*****************************************************************************/

static ssize_t set_power_state(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t count)
{
    char value = *buf;
    if(value == '1')
    {
         printk("power on\n");
		//不同的芯片相应的处理
    }
    else if(value == '0')
    {
        printk("power down\n");
		//不同的芯片相应的处理
    }
    else
    {
        printk("error : input state unknown\n");
    }

    return count;
}
static DEVICE_ATTR(power_on, S_IRUGO | S_IWUSR, NULL, set_power_state);


#ifdef  MBB_NET_RESERVED
/*全局变量 用来控制下行数据是否走ctf模块 0表示走 1表示不走*/
int g_if_eth_ctf;

 /*用来给应用控制是否走ctf*/
static ssize_t set_if_ctf(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    char temp_if_ctf[IF_CTF_LENGTH + 1] = {0};

    strncpy(temp_if_ctf, buf, IF_CTF_LENGTH);
    temp_if_ctf[IF_CTF_LENGTH] = '\0';
    g_if_eth_ctf = simple_strtol(temp_if_ctf, NULL, DECIMAL);

    printk("g_if_eth_ctf %d\n",g_if_eth_ctf);

    return count;
}
 
static ssize_t get_if_ctf(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", g_if_eth_ctf);
}
 
static DEVICE_ATTR(if_ctf, S_IRUGO | S_IWUSR, get_if_ctf, set_if_ctf);
#endif


/*****************************************************************************
函数名：  get_muticast_flag
功能描述: 组播升级报文标识
输入参数:  
返回值：
*****************************************************************************/
static ssize_t get_muticast_flag(struct device *dev, struct device_attribute *attr, char *buf)
{
    printk("get_muticast_flag = %d\n", g_muticast_flag);
    return snprintf(buf, SYS_NODE_INT_BUF_LEN, "%0x\n", g_muticast_flag);
}
 
static DEVICE_ATTR(muticast_flag, S_IRUGO, get_muticast_flag, NULL);

/*****************************************************************************
函数名：  get_traffic_flag
功能描述: 网口流量标识
输入参数:  
返回值：
*****************************************************************************/
static ssize_t get_traffic_flag(struct device *dev, struct device_attribute *attr, char *buf)
{
    printk("get_traffic_flag = %d\n", g_traffic_flag);
    return snprintf(buf, SYS_NODE_INT_BUF_LEN, "%0x\n", g_traffic_flag);
}
 
static DEVICE_ATTR(traffic_flag, S_IRUGO, get_traffic_flag, NULL);

/*****************************************************************************
函数名：  get_net_info
功能描述: 提供应用网卡信息查询
输入参数:  char *buf 返回字串
返回值：长度
*****************************************************************************/
static ssize_t get_net_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    char net_info[MAX_SHOW_LAN] = {0};
    int ret = 0;
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
    ret = rtk_get_port_phyStatus(&net_info,MAX_SHOW_LAN);
#endif
/*根据军规要求指针判空和边界判断，虽然linux sys架构传的buf空间为4K，
另sys框架设计没有长度参数，现有技术条件下还不能边界判断*/
    if(NULL != buf)
    {
        memcpy(buf, &net_info, MAX_SHOW_LAN);
    }
    return ret;   
}
 
static DEVICE_ATTR(net_info, S_IRUGO, get_net_info, NULL);


/*****************************************************************************
函数名：  mbb_eth_state_report
功能描述: DEVICE_ID_CRADLE消息沙包
输入参数:  
返回值
*****************************************************************************/
void mbb_eth_state_report(int new_state)
{
    if (new_state)
    {
        mbb_device_event_report(CRADLE_INSERT);
    }
    else
    {
        mbb_device_event_report(CRADLE_REMOVE);
    } 
}

/*****************************************************************************
函数名：  mbb_eth_traffic_status
功能描述:  网口LED控制函数
输入参数:  all_port_status所有端口状态
返回值：
*****************************************************************************/
void mbb_eth_traffic_status(unsigned int all_port_status)
{
    DEVICE_EVENT stusbEvent;
    struct net_device *pstDev =  dev_get_by_name(&init_net, NET_DEVICE_NAME);
    static int eth_rx_packets = 0;
    static int eth_tx_packets = 0;
    static int blink_delay_times = 0;
    static int on_delay_times = 0;

   /* 获取不到网口设备*/
    if (NULL == pstDev)
    {       
        return;
    }

    stusbEvent.device_id = DEVICE_ID_TRAFFIC;
    stusbEvent.len = 0;

     /*所有端口未连接*/
    if (!all_port_status)
    {       
        if (ETH_TRAFFIC_OFF != g_traffic_flag)
        {
            stusbEvent.event_code = ETH_TRAFFIC_OFF;
            device_event_report(&stusbEvent, sizeof(DEVICE_EVENT));
            blink_delay_times = 0;
            on_delay_times = 0;

            g_traffic_flag = ETH_TRAFFIC_OFF;
        }
        dev_put(pstDev);
        return;
    }

    /*有数据变化*/
    if ( (pstDev->stats.rx_packets != eth_rx_packets)  
        || (pstDev->stats.tx_packets != eth_tx_packets))
    {
        /*假如原来状态为不闪烁*/
        if (ETH_TRAFFIC_BLINK != g_traffic_flag)
        {
            /*防止状态切换太快，消息太多*/
            if (blink_delay_times++ >= ETH_TRAFFIC_TIMES)
            {
                stusbEvent.event_code = ETH_TRAFFIC_BLINK;
                device_event_report(&stusbEvent, sizeof(DEVICE_EVENT));
                blink_delay_times = 0;
                on_delay_times = 0;

                g_traffic_flag = ETH_TRAFFIC_BLINK;
            }
        }

        eth_rx_packets = pstDev->stats.rx_packets;
        eth_tx_packets = pstDev->stats.tx_packets;
    }
    else   /* 无数据变化*/
    {
        if (ETH_TRAFFIC_ON != g_traffic_flag)/*假如原来状态为闪烁*/
        {
            /*防止状态切换太快，消息太多*/
            if (on_delay_times++ >= ETH_TRAFFIC_TIMES)
            {
                stusbEvent.event_code = ETH_TRAFFIC_ON;
                device_event_report(&stusbEvent, sizeof(DEVICE_EVENT));
                blink_delay_times = 0;
                on_delay_times = 0;

                g_traffic_flag = ETH_TRAFFIC_ON;
            }
            
        }
    }

    dev_put(pstDev);
}

/*****************************************************************************
函数名：   eth_port_link_report
功能描述:  为应用层上报网卡消息
输入参数： phylinkstate
ETH_CHANGE_NONE,        //无变化
ETH_LAN_DOWN,           //LAN口down
ETH_LAN_UP,             //LAN口up
ETH_WAN_DOWN,           //WAN口down
ETH_WAN_UP,             //WAN口up
输出参数：NA
返回值：  NA
*****************************************************************************/
void eth_port_link_report(ETH_LINK_STATUS phylinkstate)
{
    DEVICE_EVENT stusbEvent;
    switch (phylinkstate)
    {
        case ETH_LAN_DOWN:
        case ETH_LAN_UP:    
            stusbEvent.device_id = DEVICE_ID_ETH;
            stusbEvent.len = 0;
            stusbEvent.event_code = 0;

            LAN_DEBUG("eth report lan change\r\n");

            device_event_report(&stusbEvent,sizeof(DEVICE_EVENT));
            break;
        case ETH_WAN_DOWN:
            mbb_eth_state_report(0);
            break;
        case ETH_WAN_UP:
            mbb_eth_state_report(1);   
            break;
        default:
            break;
    }
}
/*****************************************************************************
函数名：  eth_port_status_poll
功能描述:  端口状态查询函数
输入参数:  
返回值：
*****************************************************************************/
static void eth_port_status_poll(struct work_struct *work)
{
    unsigned int all_port_status = 0;

   /*对于多网口在这里上报网口状态，单网口在phy.c的状态机中上报*/
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
    int phylinkstate = 0;
    DEVICE_EVENT stusbEvent;
    
    phylinkstate = rtk_poll_port_status(&all_port_status);

#endif

#if (FEATURE_ON == MBB_FEATURE_ETH_PHY)
    struct phy_device *phydev = mbb_get_phy_device();

    if (NULL == phydev)
    {
        return;
    }   

    all_port_status = phydev->link;
#endif

    /*网口流量的状态*/
    mbb_eth_traffic_status(all_port_status);

    schedule_delayed_work(&s_ethstatusqueue, PHY_LINK_TIME * HZ);  
}

/*****************************************************************************
函数名：  hw_net_sysfs_init
功能描述: lan_usb节点创建
输入参数:  
返回值：NET_RET_OK(0)为ok 
*****************************************************************************/
int hw_net_sysfs_init(void)
{
    int err;
    static int sysfs_init = 0;

    /*防止多次初始化*/
    if (sysfs_init > 0)
    {
        return NET_RET_OK;
    }
    
    lan_class = class_create(THIS_MODULE, "lan_usb");
    lan_dev = device_create(lan_class, NULL, MKDEV(0, 0), NULL, "lan");
    
    err = device_create_file(lan_dev, &dev_attr_net_state);
    if(err)
    {
        printk("et_net_fs_init create lan_usb file error\n");
        return err;
    }
    err = device_create_file(lan_dev, &dev_attr_clone_mac);
    if(err)
    {
        printk("et_net_fs_init create clone_mac file error\n");
        return err;
    }

#ifdef  MBB_NET_RESERVED
    err = device_create_file(lan_dev, &dev_attr_if_ctf);
    if(err)
    {
        printk("et_net_fs_init create if_ctf file error\n");
        return err;
    }
#endif

    err = device_create_file(lan_dev, &dev_attr_muticast_flag);
    if(err)
    {
        printk("et_net_fs_init create muticast_flag file error\n");
        return err;
    }

    err = device_create_file(lan_dev, &dev_attr_traffic_flag);
    if(err)
    {
        printk("et_net_fs_init create traffic_flag file error\n");
        return err;
    }

    err = device_create_file(lan_dev, &dev_attr_net_info);
    if(err)
    {
        printk("et_net_fs_init create net_info file error\n");
        /*按评审意见修改，增加健壮性*/
        //return err;
    }
    rwlock_init(&mac_clone_lock);

    INIT_DELAYED_WORK(&s_ethstatusqueue, eth_port_status_poll);
    schedule_delayed_work(&s_ethstatusqueue, PHY_LINK_TIME * HZ);

    sysfs_init++;
    
    return NET_RET_OK;
}

/*****************************************************************************
函数名：  hw_net_sysfs_init
功能描述: lan_usb节点删除
输入参数:  
返回值：NET_RET_OK(0)为ok 
*****************************************************************************/
void hw_net_sysfs_uninit(void)
{
    device_remove_file(lan_dev, &dev_attr_net_state);

    device_remove_file(lan_dev, &dev_attr_clone_mac);

#ifdef  MBB_NET_RESERVED
    device_remove_file(lan_dev, &dev_attr_if_ctf);
#endif

    device_remove_file(lan_dev, &dev_attr_muticast_flag);

    device_remove_file(lan_dev, &dev_attr_traffic_flag);
    device_remove_file(lan_dev, &dev_attr_net_info);


    device_destroy(lan_class, MKDEV(0, 0));
    
    class_destroy(lan_class);
    return;
}

#define MAC_ADDR_LEN    17
#define HALF_BYTE_LEN   4
#define ASCI_A          'A'        
#define ASCI_F          'F'
#define ASCI_a          'a'
#define ASCI_f          'f'
#define ASCI_0          '0'
#define ASCI_9          '9'
#define CHAR_NUM1       1
#define CHAR_NUM2       2
#define CHAR_NUM3       3

/*****************************************************************************
函数名：  translate_mac_address
功能描述: 用于将冒号间隔的字符串格式MAC地址转换为6字节格式

输入参数:  
返回值：NET_RET_OK(0)为ok 
*****************************************************************************/
static int translate_mac_address(char *adr_str, char *adr_dst)
{
    int ret = 0;
    int i = 0, j = 0;
    int data;
    unsigned char c = 0;

    if (!adr_dst)
    {
        return NET_RET_FAIL;
    }
    
    if (!adr_str)
    {
        return NET_RET_FAIL;
    }
    
    data = 0;
    i = 0;

    while(i < MAC_ADDR_LEN)
    {
        c = adr_str[i];
        data = data << HALF_BYTE_LEN;
        j = i % CHAR_NUM3;
        if(CHAR_NUM2 == j)
        {
            if(':' == c)
            { 
                i++;
                continue;
            }
            else
            {
                ret = NET_RET_FAIL;
                break;
            } 
        }
        if(ASCI_0 <= c && ASCI_9 >= c)
        {
            data += c - ASCI_0;
        }    
        else if(ASCI_A <= c && ASCI_F >= c)
        {
            data += c - ASCI_A + DECIMAL;
        }    
        else if(ASCI_a <= c && ASCI_f >= c)
        {
            data += c - ASCI_a + DECIMAL;
        }    
        else
        {
            ret = NET_RET_FAIL;
            break;
        }
        if(CHAR_NUM1 == j)
        {
            adr_str[i / CHAR_NUM3] = data;
            data = 0;
        }
        i++;
    }

    if(NET_RET_FAIL != ret)
    {
        memcpy(adr_dst, adr_str, ETH_ALEN);
    }
    else
    {
        printk("%s: error mac addr\n", __FUNCTION__);
    }

    return ret;
}

/*****************************************************************************
函数名：  mbb_device_event_report
功能描述: DEVICE_ID_CRADLE消息沙包
输入参数:  
返回值
*****************************************************************************/
static void mbb_device_event_report(int event)
{
    DEVICE_EVENT stusbEvent;
    stusbEvent.device_id = DEVICE_ID_CRADLE;
    stusbEvent.len = 0;
    stusbEvent.event_code = event;

    LAN_DEBUG("eth report device event:%d\r\n", event);

    device_event_report(&stusbEvent,sizeof(DEVICE_EVENT));
}

#if (FEATURE_ON == MBB_CTF_COMMON)
static void eth_ctf_detach(ctf_t *ci, void *arg)
{
    eth_cih = NULL;
    return;
}

unsigned int eth_ctf_forward(struct sk_buff *skb)
{
    /* use slow path if ctf is disabled */
    if (!CTF_ENAB(eth_cih))
    {
        return (BCME_ERROR);
    }
    /* try cut thru first */
    if (BCME_ERROR != ctf_forward(eth_cih, skb, skb->dev))
    {
        return (BCME_OK);
    }

    /* clear skipct flag before sending up */
    PKTCLRSKIPCT(NULL /* et->osh*/, skb);

    return (BCME_ERROR);
}

/*****************************************************************************
函数名：  mbb_ctf_forward
功能描述: 报文通过ctf转发
输入参数:  
返回值
*****************************************************************************/
int mbb_ctf_forward(struct sk_buff *skb,  struct net_device *dev)
{
    if (NULL == skb || NULL == dev)
    {
        printk("mbb_ctf_forward skb or dev is null\n");
        return NET_RET_FAIL;
    }   
        
    if (0 == g_ifctf)
    {
        /*对skb->dev, protocol等赋值*/
        skb->protocol = eth_type_trans(skb,  dev);

        /*ether header*/
        skb_push(skb, ETH_HLEN);

        if (eth_cih && (BCME_ERROR != eth_ctf_forward(skb)))
        {
            return NET_RET_OK;
        }
    }

    return NET_RET_FAIL;
}

/*****************************************************************************
函数名：  mbb_ctf_init
功能描述: ctf设备注册
输入参数:  
返回值
*****************************************************************************/
void mbb_ctf_init(struct net_device *net)
{
    unsigned int msglevel = 1;

    if (NULL != eth_cih || NULL == net)
    {
        return;
    }

    LAN_DEBUG("Init CTF for eth.\n");
    eth_fake_osh = (osl_t *)0xdeadbeaf; /* watch here, it might die */
    eth_cih = ctf_attach(eth_fake_osh, net->name, &msglevel, eth_ctf_detach, NULL /* et*/ );
    if (eth_cih)
    {
        LAN_DEBUG("Successful attach eth_cih = 0x%08x \n", (unsigned int)eth_cih);
    }

    if ((ctf_dev_register(eth_cih, net, FALSE) != BCME_OK) ||
        (ctf_enable(eth_cih, net, TRUE,NULL) != BCME_OK)) 
    {
        printk("ctf_dev_register() failed for eth.\n");
    }
    else
    {
        printk("Register CTF for eth successful.\n");
    }
}


/*****************************************************************************
函数名：  mbb_ctf_exit
功能描述: ctf设备退出
输入参数:  
返回值
*****************************************************************************/
void mbb_ctf_exit(struct net_device *net)
{
    if (NULL == eth_cih || NULL == net)
    {
        return;
    }
    
    ctf_dev_unregister(eth_cih, net);
    eth_cih = NULL;
}

EXPORT_SYMBOL(mbb_ctf_forward);
EXPORT_SYMBOL(mbb_ctf_init);
EXPORT_SYMBOL(mbb_ctf_exit);
#endif

/*************************************************
函数名:       int et_reset_linkstate(int mode)
功能描述:    重置网口link状态，使其重新获取IP  
输入参数:        int mode, 0 重启port1-port4 1 重启port1-port3
返回值:       0 处理成功， 非0  处理异常     
*************************************************/
int et_reset_linkstate(int mode)
{
    int regvalue;
#if (FEATURE_ON == MBB_FEATURE_ETH_PHY)
    struct phy_device* phydev = mbb_get_phy_device();
#endif


    /*组播升级状态不做reset*/
    if (g_muticast_flag)
    {
        return NET_RET_FAIL;
    }
    
#if (FEATURE_ON == MBB_FEATURE_ETH_PHY)   
    if (NULL != phydev)
    { 
        regvalue = phy_read(phydev, MII_BMCR);

        regvalue |= BMCR_RESET;
        phy_write(phydev, MII_BMCR, regvalue);

        return NET_RET_OK;
    }
#endif

#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)   
    rtk_reset_allport(mode);
    return NET_RET_OK;
#endif

    return NET_RET_FAIL;

}

/*******************************************************************************
 函数名称  : mbb_get_eth_macAddr
 功能描述  : 设置出厂MAC, dev->dev_addr
 输入参数  :eth_macAddr
 输出参数  : 无
 返 回 值     :  0 : 成功,  -1 : 失败
*******************************************************************************/
int mbb_get_eth_macAddr(char *eth_macAddr)
{
    char baseMacaddr[NV_MAC_LENGTH] = {0};
    
    if (NULL == eth_macAddr)
    {
        return -1;
    }
   
    if (0 != NVM_Read(en_NV_Item_WIFI_MAC_ADDR,  baseMacaddr, sizeof(baseMacaddr)))
    {
        LAN_DEBUG("NVM Read MAC addr fail\n");
        return -1;
    }
    else
    {      
        if (0 != translate_mac_address(baseMacaddr, eth_macAddr))
        {
            LAN_DEBUG("factory macAddr format err\n");
            return -1;
        }  

      
        return 0;
    }  
}

EXPORT_SYMBOL(PhyATQryPortPara);
EXPORT_SYMBOL(mbb_check_net_upgrade);
EXPORT_SYMBOL(mbb_mac_clone_rx_restore);
EXPORT_SYMBOL(mbb_mac_clone_tx_save);
EXPORT_SYMBOL(RNIC_WANMirror);
EXPORT_SYMBOL(RNIC_Map_To_Lan_Forward);
EXPORT_SYMBOL(mbb_eth_state_report);
EXPORT_SYMBOL(et_reset_linkstate);

#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
#define ET_MMITEST_SLEEP_TIMEORT        10
#define ET_MMITEST_SKB_PACKET_SIZE      1500
#define ET_MMITEST_SKB_ONE_DATA_SIZE    76
#define ET_MMITEST_SKB_DATA_COPY_NUM    15
#define SET_RES_BIT(n)  (g_et_mmitest |= 1 <<(n))
#define CLEAR_RES_BIT(n)  (g_et_mmitest &= ~(1 <<(n)))

static  unsigned char g_et_mmitest_skb_data[ET_MMITEST_SKB_ONE_DATA_SIZE]  = 
{
    0x00,0x00,0x5e,0x00,0x01,0x69,0x00,0xe0,0x4c,0x97,
    0xa7,0x6a,0x08,0x00,0x45,0x00,0x00,0x3e,0xdd,0x3a,
    0x00,0x00,0x40,0x11,0x44,0x6c,0x0a,0x91,0x3d,0xae,
    0xda,0x19,0x36,0xb0,0x22,0x68,0x1f,0x77,0x00,0x2a,
    0x38,0xa1,0x00,0x00,0x01,0x03,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x16,0x0f,0xa0,0x00,0x00,0x00,0x10,
    0x71,0xef,0x7f,0xea,0x3f,0x6c,0x46,0x6a,0x96,0x7c,
    0xb4,0x6b,0xaf,0xaa,0xa9,0x04,
};

typedef int (*check_recv_packet)(struct sk_buff *skb, int frame_len);
extern void mbb_mmitest_check_recv(struct net_device *dev, check_recv_packet check_fun);
extern void mbb_mmitest_xmit(struct sk_buff *skb, struct net_device *dev);
extern int g_mmitest_enable;

typedef enum
{
    ET_MMITEST_LINK_12_FAIL,
    ET_MMITEST_LINK_34_FAIL,
    ET_MMITEST_SPEED_12_FAIL,
    ET_MMITEST_SPEED_34_FAIL,
    ET_MMITEST_DATA_12_FAIL,
    ET_MMITEST_DATA_34_FAIL,
    ET_MMITEST_RESULT,
}ET_MMITEST_RESULT_ST;

static unsigned int g_et_mmitest = (1 << ET_MMITEST_RESULT);


/*******************************************************************************
 函数名称  : mbb_mmitest_send_packet
 功能描述  : MMI测试构造skb报文通过驱动发送
 输入参数  :pstDev: et0设备
                         vid 插入的vlanID
 输出参数  : 无
 返 回 值     :  0 : 成功,  -1 : 失败
*******************************************************************************/
static int mbb_mmitest_send_packet(struct net_device *pstDev, int vid)
{
    int i = 0;
    struct sk_buff *skb = NULL;
    
    skb = netdev_alloc_skb_ip_align(pstDev, ET_MMITEST_SKB_PACKET_SIZE);
    if (NULL == skb)
    {
        printk("%s skb malloc fail\n", __FUNCTION__);
        return NET_RET_FAIL;
    }

    for (i = 0; i < ET_MMITEST_SKB_DATA_COPY_NUM; i++)
    {
        memcpy(skb->data + i * ET_MMITEST_SKB_ONE_DATA_SIZE, 
            g_et_mmitest_skb_data, ET_MMITEST_SKB_ONE_DATA_SIZE);   
    }

    skb->len = ET_MMITEST_SKB_ONE_DATA_SIZE * ET_MMITEST_SKB_DATA_COPY_NUM;

    skb->vlan_proto = 0x81;
    skb = vlan_insert_tag(skb, skb->vlan_proto,vid);  
    if (skb)
    {
        mbb_mmitest_xmit(skb, pstDev);
    }
    return NET_RET_OK;    
}


/*******************************************************************************
 函数名称  : mmitest_check_packet
 功能描述  : MMI测试驱动收包函数调用，确定受到报文是否正确
 输入参数  :skb: 收到的skb
                         frame_len收到的报文长度
 输出参数  : 无
 返 回 值     :  0 : 成功,  -1 : 失败
*******************************************************************************/
static int mmitest_check_packet(struct sk_buff *skb, int frame_len)
{
    int ret = NET_RET_OK;
    int i, j = 0;    
    unsigned char rx_data[ET_MMITEST_SKB_PACKET_SIZE] = {0};    
    unsigned char *pRxData = NULL;
    struct vlan_ethhdr *vlanh = NULL;
    int vlanID;
 
    pRxData = (unsigned char *)(skb->data);
    
    /* 检测数据包长度是否正确*/
    if (ET_MMITEST_SKB_ONE_DATA_SIZE * ET_MMITEST_SKB_DATA_COPY_NUM + VLAN_HLEN != frame_len)
    {        
        LAN_DEBUG("ERR frame_len=%d\n", frame_len);
        return NET_RET_FAIL;
    }
    else
    {
        /* 检测数据包macaddr是否正确*/
        if (0 != memcmp(g_et_mmitest_skb_data, pRxData, ETH_ALEN + ETH_ALEN))
        {               
            LAN_DEBUG("MAC ERR\n");
            return NET_RET_FAIL;
        }
    }
         
    memcpy(rx_data, (unsigned char *)(skb->data), frame_len);        
    pRxData = rx_data;

    vlanh = (struct vlan_ethhdr *)(pRxData);    
    vlanID = ntohs(vlanh->h_vlan_TCI);

    /* 检测是否是端口收到的vlan报文*/
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH_8363)
    if (VLAN_WAN_PORT != vlanID)
#else
    if ((VLAN_PORT1 != vlanID) && (VLAN_WAN_PORT != vlanID))
#endif
    {
        LAN_DEBUG("ERR VLANID%d\n", vlanID);
        return NET_RET_FAIL;
    }

    /*移除端口加的vlanID*/    
    memmove(pRxData + VLAN_HLEN, pRxData, ETH_ALEN + ETH_ALEN);
    pRxData += VLAN_HLEN;

    /* 检测报文是否正确*/    
    for (j = 0; j < ET_MMITEST_SKB_DATA_COPY_NUM; j++)
    {
        for(i = 0; i < ET_MMITEST_SKB_ONE_DATA_SIZE; i++)
        {
            if(pRxData[i + j * ET_MMITEST_SKB_ONE_DATA_SIZE] != g_et_mmitest_skb_data[i])
            {                              
                LAN_DEBUG("port=%d data err\n", vlanID);
                ret = NET_RET_FAIL;
                break;
            }
        }
    }


 #if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH_8363)
     if (NET_RET_OK == ret)
    {
        CLEAR_RES_BIT(ET_MMITEST_DATA_34_FAIL);
        if (VLAN_WAN_PORT == vlanID) /*Port12的报文结果*/
        {
            CLEAR_RES_BIT(ET_MMITEST_DATA_12_FAIL);
        }
    }
 #else
    if (NET_RET_OK == ret)
    {
        if (VLAN_PORT1 == vlanID) /*Port12的报文结果*/
        {
            CLEAR_RES_BIT(ET_MMITEST_DATA_12_FAIL);
        }
        else if (VLAN_WAN_PORT == vlanID)     /*Port34的报文结果*/
        {    
            CLEAR_RES_BIT(ET_MMITEST_DATA_34_FAIL);
        }
    }
 #endif
    LAN_DEBUG("vlanID=%d, frame_len=%d, ret=%d\n", vlanID, frame_len, ret);
   
    return ret;
}

/*******************************************************************************
 函数名称  : mmitest_check_port
 功能描述  : MMI测试校验端口是否有网线插入和link速率是否为1G
*******************************************************************************/
static void mmitest_check_port(void)
{        
    LAN_ETH_STATE_INFO_ST EthInfo[LAN_PORT_4];

    memset(EthInfo, 0, sizeof(EthInfo));
    rtk_get_port_status_info(EthInfo, 0, LAN_PORT_4);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH_8363)
    CLEAR_RES_BIT(ET_MMITEST_LINK_34_FAIL);
    CLEAR_RES_BIT(ET_MMITEST_SPEED_34_FAIL);
    if (EthInfo[LAN_PORT_2 - 1].link_state && EthInfo[LAN_PORT_4 - 1].link_state)
    {
        CLEAR_RES_BIT(ET_MMITEST_LINK_12_FAIL);
        if ((SPEED_1000 == EthInfo[LAN_PORT_2 - 1].speed) && (SPEED_1000 == EthInfo[LAN_PORT_4 - 1].speed))
        {
            CLEAR_RES_BIT(ET_MMITEST_SPEED_12_FAIL);
        }
        else
        {
            SET_RES_BIT(ET_MMITEST_SPEED_12_FAIL);
        }
    }
    else
    {
        SET_RES_BIT(ET_MMITEST_LINK_12_FAIL);
    }
#else

    if (EthInfo[LAN_PORT_1 - 1].link_state && EthInfo[LAN_PORT_2 - 1].link_state)
    {
        CLEAR_RES_BIT(ET_MMITEST_LINK_12_FAIL);        

        if ((SPEED_1000 == EthInfo[LAN_PORT_1 - 1].speed) && (SPEED_1000 == EthInfo[LAN_PORT_2 - 1].speed))
        {        
            CLEAR_RES_BIT(ET_MMITEST_SPEED_12_FAIL);
        }
        else
        {            
            SET_RES_BIT(ET_MMITEST_SPEED_12_FAIL);
        }
    }
    else
    {        
        SET_RES_BIT(ET_MMITEST_LINK_12_FAIL);
    }

    if (EthInfo[LAN_PORT_3 - 1].link_state && EthInfo[LAN_PORT_4 - 1].link_state)
    {        
        CLEAR_RES_BIT(ET_MMITEST_LINK_34_FAIL);        

        if ((SPEED_1000 == EthInfo[LAN_PORT_3 - 1].speed) && (SPEED_1000 == EthInfo[LAN_PORT_4 - 1].speed))
        {        
            CLEAR_RES_BIT(ET_MMITEST_SPEED_34_FAIL);
        }
        else
        {            
            SET_RES_BIT(ET_MMITEST_SPEED_34_FAIL);
        }
    } 
    else
    {        
        SET_RES_BIT(ET_MMITEST_LINK_34_FAIL);
    }
#endif
}

/*****************************************************************************
函数名：  get_lan_mmitest
功能描述:mmitest结果
输入参数:  
返回值：
*****************************************************************************/
static ssize_t get_lan_mmitest(struct device *dev, struct device_attribute *attr, char *buf)
{
    LAN_DEBUG("g_et_mmitest = 0x%x\n", g_et_mmitest);
    return snprintf(buf, SYS_NODE_INT_BUF_LEN, "%d\n", g_et_mmitest);
}
 
static DEVICE_ATTR(lan_mmitest, S_IRUGO, get_lan_mmitest, NULL);
/*******************************************************************************
 函数名称  : ET_MMI_test_start
 功能描述  : 装备调用开始mmi测试
 输入参数  : 无
 输出参数  : 无
 返 回 值     : 无
*******************************************************************************/
void ET_MMI_test_start(void)
{
    
    struct net_device *pstDev =  dev_get_by_name(&init_net, NET_DEVICE_NAME);

    if (NULL == pstDev)
    {
        return;
    }
    
    g_mmitest_enable = 1;

    /*初值则创建mmitest节点*/
    if (1 << ET_MMITEST_RESULT == g_et_mmitest)
    {
        if(device_create_file(lan_dev, &dev_attr_lan_mmitest))
        {
            return;
        }
    }

    g_et_mmitest = 0;

    netif_stop_queue(pstDev);

    mmitest_check_port();
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH_8363)
    /*如果PORT12连接则进行报文测试*/
    if (0 == ((1 << ET_MMITEST_LINK_12_FAIL) & g_et_mmitest))
    {       
        SET_RES_BIT(ET_MMITEST_DATA_12_FAIL);
        if (NET_RET_OK == mbb_mmitest_send_packet(pstDev, VLAN_PORT1))
        {
            msleep(ET_MMITEST_SLEEP_TIMEORT);
            mbb_mmitest_check_recv(pstDev, mmitest_check_packet);
        }
    }
 #else
    /* 装备测试调用的接口 禁止网口的1和2端口进行网口回环测试 */
    CLEAR_RES_BIT(ET_MMITEST_LINK_12_FAIL);
    CLEAR_RES_BIT(ET_MMITEST_SPEED_12_FAIL);
    CLEAR_RES_BIT(ET_MMITEST_DATA_12_FAIL);
 
    msleep(ET_MMITEST_SLEEP_TIMEORT);

    /*如果PORT34连接则进行报文测试*/
    if (0 == ((1 << ET_MMITEST_LINK_34_FAIL) & g_et_mmitest))
    {                
        SET_RES_BIT(ET_MMITEST_DATA_34_FAIL);
        if (NET_RET_OK == mbb_mmitest_send_packet(pstDev, VLAN_PORT2))
        {
            msleep(ET_MMITEST_SLEEP_TIMEORT);
            mbb_mmitest_check_recv(pstDev, mmitest_check_packet);
        }
    }
#endif
       
    dev_put(pstDev);

    netif_start_queue(pstDev);

    g_mmitest_enable = 0;
    LAN_DEBUG("g_et_mmitest = 0x%x\n", g_et_mmitest);
}

EXPORT_SYMBOL(ET_MMI_test_start);
#endif

