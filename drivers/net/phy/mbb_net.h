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
 
 #ifndef __PHY_MBB_NET_H__
#define __PHY_MBB_NET_H__

#define NET_RET_OK      0
#define NET_RET_FAIL    (-1)

#define IP_ADDR_LEN     16

#define _LAN_DEBUG_
#ifdef _LAN_DEBUG_
#define LAN_DEBUG  printk
#else
#define LAN_DEBUG
#endif
#define MAX_SHOW_LAN          512


/*装备要求的端口状态结构体*/
typedef struct tagEquipNetPartST
{
    unsigned int total_port;
    unsigned int port_index;
    unsigned int port_rate;
    unsigned char ip_add[IP_ADDR_LEN];
}NET_PORT_ST;

#define RTK8367_ID                            0x83676367   //用于作为rtk8367的驱动ID标识
#define RTK8367_ID_MASK                 0xffffffff

#define SIOCLINKSTATE                       0x89F8   //系统获取多网口状态
#define SWITCH_LAN_PORT_NUM         3           //作为LAN口的数目

#define VLAN_PORT0          2
#define VLAN_PORT1          3
#define VLAN_PORT2          4
#define VLAN_WAN_PORT   	5


typedef enum
{
    ETH_CHANGE_NONE,      //无变化
    ETH_LAN_DOWN,           //LAN口down
    ETH_LAN_UP,                 //LAN口up
    ETH_WAN_DOWN,           //WAN口down
    ETH_WAN_UP,                //WAN口up
}ETH_LINK_STATUS;

typedef struct tag_LAN_ETH_STATE_INFO_ST
{
    int link_state;     /* link state */ 
    int dpulex;         /* dpulex */ 
    int speed;          /* speed */
} LAN_ETH_STATE_INFO_ST;


#define SIOCLINKENABLE 	0x89FA   // 禁用/启用eth端口

typedef struct {
    unsigned int port;
    unsigned int enable;
} LAN_ETH_STATE_ENABLE_ST;

/*****************************************************************************
函数名：   PhyATQryPortPara
功能描述:  装备单网口信息获取接口
输入参数:  端口索引
返回值：  NET_RET_OK(0)为ok 
*****************************************************************************/
int PhyATQryPortPara(NET_PORT_ST *PortPara);

/*****************************************************************************
函数名：   mbb_check_net_upgrade
功能描述:  判断报文是否是组播升级报文
输入参数:  skb报文
返回值：  NET_RET_OK(0)为ok 
*****************************************************************************/
int mbb_check_net_upgrade(struct sk_buff *skb);

/*****************************************************************************
函数名：   mbb_mac_clone_rx_restore
功能描述:  MAC层接收PPP报文时mac clone
输入参数:  skb报文
返回值：  NET_RET_OK(0)为ok 
*****************************************************************************/
int mbb_mac_clone_rx_restore(struct sk_buff *skb);

/*****************************************************************************
函数名：   mbb_mac_clone_tx_save
功能描述:  MAC层发送PPP报文时mac clone
输入参数:  skb报文
返回值：  NET_RET_OK(0)为ok 
*****************************************************************************/
int mbb_mac_clone_tx_save(struct sk_buff *skb);

/*****************************************************************************
函数名：   mbb_get_phy_device
功能描述:  提供给其它模块获取phy设备
输入参数： 无
返回值：   phy设备
*****************************************************************************/
struct phy_device *mbb_get_phy_device(void);

/*****************************************************************************
函数名：  hw_net_sysfs_init
功能描述: lan_usb节点创建
输入参数:  
返回值：NET_RET_OK(0)为ok 
*****************************************************************************/
int hw_net_sysfs_init(void);

/*****************************************************************************
函数名：  hw_net_sysfs_init
功能描述: lan_usb节点删除
输入参数:  
返回值：NET_RET_OK(0)为ok 
*****************************************************************************/
void hw_net_sysfs_uninit(void);

/*****************************************************************************
函数名：   RNIC_WANMirror
功能描述:  打开WAN口镜像
输入参数:  enable使能与否
                        port端口号
返回值：  NET_RET_OK(0)为ok 
*****************************************************************************/
int RNIC_WANMirror(unsigned int enable, unsigned int port);

/*****************************************************************************
函数名：   RNIC_Map_To_Lan_Forward
功能描述:  WAN口镜像
输入参数:  skb报文
返回值：  NET_RET_OK(0)为ok 
*****************************************************************************/
int RNIC_Map_To_Lan_Forward(struct sk_buff *skb, void *pdata, int len);

#if (defined(CONFIG_BALONG_SPE))
#define VOS_TRUE   1
#define RNIC_SPE_MEM_CB_MIRROR(skb)       ((skb)->dma)

/*lint -emacro({717}, RNIC_SPE_MEM_MAP)*/
#define RNIC_SPE_MEM_MAP_MIRROR(skb, ulLen)\
            do\
            {\
                if (VOS_TRUE == RNIC_IsSpeMemMirror(skb))\
                {\
                    RNIC_SpeMemMapRequsetMirror(skb, ulLen);\
                }\
            } while(0)

/*lint -emacro({717}, RNIC_SPE_MEM_UNMAP)*/
#define RNIC_SPE_MEM_UNMAP_MIRROR(skb, ulLen)\
            do\
            {\
                if (VOS_TRUE == RNIC_IsSpeMemMirror(skb))\
                {\
                    RNIC_SpeMemUnmapRequsetMirror(skb, ulLen);\
                }\
            } while(0)
            
#define RNIC_SPE_SKBOWN_RESET(skb)   ((skb)->spe_own = 0)
#else
#define RNIC_SPE_MEM_MAP_MIRROR(skb, ulLen)
#define RNIC_SPE_MEM_UNMAP_MIRROR(skb, ulLen)
#define RNIC_SPE_SKBOWN_RESET(skb)
#endif


/*****************************************************************************
函数名：   rtk_get_port_status_info
功能描述:  为应用层获取端口的状态
输入参数： startPort 起始端口
                            num 连续的端口个数
输出参数：端口状态信息                            
返回值：   NET_RET_OK(0)为获取状态OK
*****************************************************************************/
void rtk_get_port_status_info(LAN_ETH_STATE_INFO_ST *pEthInfo, int startPort, int num);

/*****************************************************************************
函数名：   rtk_led_ctrl()
功能描述:  设置led灯亮灭    
输入参数: leds 灯的序号，mode:0 灭,1:   亮
返回值：0 OK  <0 err  
*****************************************************************************/
int rtk_led_ctrl(unsigned char leds, unsigned char mode);
/*****************************************************************************
函数名：   rtk_get_port_phyStatus
功能描述:  为应用层获取端口的状态
输入参数： buf 用来保存网卡信息
输出参数：端口状态信息                            
返回值：   >=0为获取状态OK， <0 erro
*****************************************************************************/

#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
int  rtk_get_port_phyStatus(char *buf,unsigned int len);
#endif

/*****************************************************************************
函数名：   eth_port_link_report
功能描述:  为应用层上报网卡消息
输入参数： phylinkstate
ETH_CHANGE_NONE,      //无变化
ETH_LAN_DOWN,         //LAN口down
ETH_LAN_UP,           //LAN口up
ETH_WAN_DOWN,         //WAN口down
ETH_WAN_UP,           //WAN口up
输出参数：NA
返回值：  NA
*****************************************************************************/
void eth_port_link_report(ETH_LINK_STATUS phylinkstate);

#endif

