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

#include <linux/phy.h>
#include <linux/module.h>
#include <linux/delay.h>
#include "../mbb_net.h"
#include "rtk_switch.h"
#include "rtk_error.h"
#include "port.h"
#include "vlan.h"
#include "rtk_types.h"
#include "cpu.h"
#include "interrupt.h"
#include "trap.h"
#include "rtl8367c_asicdrv.h"
#include "rtl8367c_asicdrv_port.h"
#include "stat.h"
#include <product_config.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include "led.h"
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH_SVLAN)
#include "svlan.h"
#endif

struct mutex rtk_lock;

MODULE_DESCRIPTION("Driver for rtl8367");
MODULE_AUTHOR("jiangdihui <jiangdihui@huawei.com>");
MODULE_LICENSE("GPL");

static struct phy_device *s_phydev = NULL;

#define RTK_PRINT_ERRMSG_RETURN(str, code) {printk("SDK ASIC error ID:%d, error message: %s \n", code, str); return code;}
#define VLAN_PORT0          2
#define VLAN_PORT1          3
#define VLAN_PORT2          4
#define VLAN_WAN_PORT   5
#define PHY_RST_DELAY  10

#define LAN_LED_1              1
#define LAN_LED_2              2
#define LAN_LED_3              3
#define LAN_LED_4              4
#define LAN_LED_MAX            4

#define PHY_AUTO_EC 0xA42C
#define AUTO_EC_EN 0x0010
#define RESTART_AUTO_NEGO (0x0001 << 9)

#ifndef MBB_RTK_RGMII_RX_DELAY
#define MBB_RTK_RGMII_RX_DELAY 4
#endif

#if (MBB_RTK_RGMII_RX_DELAY < 0) || (MBB_RTK_RGMII_RX_DELAY > 7)
#error "MBB_RTK_RGMII_RX_DELAY invalid!!"
#endif

struct _led_adp
{
    unsigned char leds;
    unsigned port;
    unsigned group;
} led_adp;

/*****************************************************************************
函数名：   mbb_get_phy_device
功能描述:  提供给其它模块获取phy设备
输入参数： 无
返回值：   phy设备
*****************************************************************************/
struct phy_device *mbb_get_phy_device(void)
{
    return s_phydev;
}

/*****************************************************************************
函数名：   mbb_mdio_write
功能描述:  mdio写寄存器
输入参数： 无
返回值：    无
*****************************************************************************/
void mbb_mdio_write(rtk_uint32 mAddrs, rtk_uint32 wData)
{
    if (NULL != s_phydev)
    {
        phy_write(s_phydev, mAddrs, wData);
    }
}

/*****************************************************************************
函数名：   mbb_mdio_read
功能描述:  mdio读寄存器
输入参数： 无
返回值：   无
*****************************************************************************/
void mbb_mdio_read(rtk_uint32 mAddrs, rtk_uint32 *rData)
{
    if (NULL != s_phydev)
    {
        *rData = phy_read(s_phydev, mAddrs);
    }
}
/*****************************************************************************
函数名：   rtk_phyPatch_set
功能描述:  enable auto EC
输入参数： 无
返回值：   RT_ERR_OK 正常，其它：错误
*****************************************************************************/
rtk_api_ret_t rtk_phyPatch_set(void)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 port = 0;
    rtk_uint32 data = 0;

    for (port = 0; port <= UTP_PORT4; port++)
    {
        if ( (retVal = rtl8367c_getAsicPHYOCPReg(port, PHY_AUTO_EC, &data)) != RT_ERR_OK)
        { 
            return retVal; 
        }

        data |= AUTO_EC_EN;
        if ( (retVal = rtl8367c_setAsicPHYOCPReg(port, PHY_AUTO_EC, data)) != RT_ERR_OK)
        {
            return retVal; 
        }


        if ((retVal = rtl8367c_getAsicPHYReg(port, PHY_CONTROL_REG, &data)) != RT_ERR_OK)
        { 
            return retVal; 
        }

        data |= RESTART_AUTO_NEGO;
        if ((retVal = rtl8367c_setAsicPHYReg(port, PHY_CONTROL_REG, data)) != RT_ERR_OK)
        { 
            return retVal; 
        }

    }
    return RT_ERR_OK;
}

#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH_SVLAN)
/*****************************************************************************
函数名：   mbb_rtk_svlan_config
功能描述:  rtk8637 svlan配置
输入参数： UTP端口 svid
返回值：   错误码
*****************************************************************************/
rtk_api_ret_t mbb_rtk_svlan_config(rtk_port_t utp_port,rtk_uint32 svid)
{
    rtk_api_ret_t retVal = RT_ERR_OK;
    rtk_svlan_memberCfg_t svlan_cfg = {0};
    rtk_portmask_t portmask = {0};

    /* config port0--->port4PVID */
    rtk_vlan_portPvid_set(utp_port, svid, 0);

    /*set port PVID config*/
    memset(&svlan_cfg, 0, sizeof(rtk_svlan_memberCfg_t));
    svlan_cfg.svid = svid;
    RTK_PORTMASK_CLEAR(svlan_cfg.memberport);
    RTK_PORTMASK_CLEAR(svlan_cfg.untagport);
    RTK_PORTMASK_PORT_SET(svlan_cfg.memberport,EXT_PORT0);
    RTK_PORTMASK_PORT_SET(svlan_cfg.memberport,utp_port);
    RTK_PORTMASK_PORT_SET(svlan_cfg.untagport,UTP_PORT0);
    RTK_PORTMASK_PORT_SET(svlan_cfg.untagport,UTP_PORT1);
    RTK_PORTMASK_PORT_SET(svlan_cfg.untagport,UTP_PORT2);
    RTK_PORTMASK_PORT_SET(svlan_cfg.untagport,UTP_PORT3);
    svlan_cfg.fiden = 1;
    svlan_cfg.fid = svid;
    retVal = rtk_svlan_memberPortEntry_adv_set(svid, &svlan_cfg);
    if (RT_ERR_OK != retVal)
    {
        RTK_PRINT_ERRMSG_RETURN("rtk_svlan_memberPortEntry_adv_set",retVal);
    }

    retVal = rtk_svlan_defaultSvlan_set(utp_port, svid);
    if (RT_ERR_OK != retVal)
    {
        RTK_PRINT_ERRMSG_RETURN("rtk_svlan_defaultSvlan_set",retVal);
    }

    /* Port Isolation for packet ingress to LAN ports */
    RTK_PORTMASK_CLEAR(portmask);
    RTK_PORTMASK_PORT_SET(portmask, EXT_PORT0);
    RTK_PORTMASK_PORT_SET(portmask, utp_port);
    retVal = rtk_port_isolation_set(utp_port, &portmask);
    if (RT_ERR_OK != retVal)
    {
        RTK_PRINT_ERRMSG_RETURN("rtk_port_isolation_set",retVal);
    }

    retVal = rtk_vlan_portFid_set(utp_port, ENABLED, svid);
    if (RT_ERR_OK != retVal)
    {
        RTK_PRINT_ERRMSG_RETURN("rtk_vlan_portFid_set",retVal);
    }

    return RT_ERR_OK;
}

/*****************************************************************************
函数名：   mbb_rtk_svlan_init
功能描述:  rtk8637 svlan配置
输入参数： UTP端口 svid
返回值：   错误码
*****************************************************************************/
rtk_api_ret_t mbb_rtk_svlan_init(void)
{
    rtk_api_ret_t retVal = RT_ERR_OK;
    rtk_portmask_t portmask = {0};
    int i = 0;

    retVal = rtk_svlan_init();
    if (RT_ERR_OK != retVal)
    {
        RTK_PRINT_ERRMSG_RETURN("rtk_svlan_init",retVal);
    }

    /* set EXT_PORT0 service port */
    retVal = rtk_svlan_servicePort_add(EXT_PORT0);
    if (RT_ERR_OK != retVal)
    {
        RTK_PRINT_ERRMSG_RETURN("rtk_svlan_servicePort_add",retVal);
    }

    retVal = rtk_svlan_tpidEntry_set(ETH_P_8021Q);
    if (RT_ERR_OK != retVal)
    {
        RTK_PRINT_ERRMSG_RETURN("rtk_svlan_tpidEntry_set",retVal);
    }

    for (i = UTP_PORT0; i <= UTP_PORT3; i++)
    {
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH_8363)
        if ((UTP_PORT0 == port) || (UTP_PORT2 == port))
        {
            continue;
        }
#endif
        retVal = mbb_rtk_svlan_config(i,i + VLAN_PORT0);
        if (RT_ERR_OK != retVal)
        {
            RTK_PRINT_ERRMSG_RETURN("mbb_rtk_svlan_config",retVal);
        }
    }

    /* Port Isolation for packet ingress to LAN ports */
    RTK_PORTMASK_CLEAR(portmask);
    RTK_PORTMASK_PORT_SET(portmask, EXT_PORT0);
    RTK_PORTMASK_PORT_SET(portmask, UTP_PORT0);
    RTK_PORTMASK_PORT_SET(portmask, UTP_PORT1);
    RTK_PORTMASK_PORT_SET(portmask, UTP_PORT2);
    RTK_PORTMASK_PORT_SET(portmask, UTP_PORT3);
    retVal = rtk_port_isolation_set(EXT_PORT0, &portmask);
    if (RT_ERR_OK != retVal)
    {
        RTK_PRINT_ERRMSG_RETURN("rtk_port_isolation_set EXT_PORT0",retVal);
    }

    return RT_ERR_OK;
}
#endif

/*****************************************************************************
函数名：   rtk_switchDevice_init
功能描述:  rtk8637初始化
输入参数： 无
返回值：   无
*****************************************************************************/
#define LED_MUX_MODE 0x7c64
#define LED0_MODE 0x48
#define LED1_MODE 0x43
rtk_api_ret_t rtk_switchDevice_init(void)
{
    rtk_api_ret_t retVal;
    rtk_port_t  port;
    rtk_port_mac_ability_t port_cfg;
    rtk_vlan_cfg_t vlan_cfg;
    rtk_vlan_t vlanArry[] = {VLAN_PORT0, VLAN_PORT1, VLAN_PORT2, VLAN_WAN_PORT}; // 4 VLAN for 4 port;

    /*1, Init switch*/
    if ((retVal = rtk_switch_init()) != RT_ERR_OK)
    {
        RTK_PRINT_ERRMSG_RETURN("Switch init failure!",retVal);
    }

    /*2, Init CPU PORT, EXT_PORT0 is CPU port*/
    port_cfg.forcemode = MAC_FORCE;
    port_cfg.speed     = SPD_1000M;
    port_cfg.duplex    = FULL_DUPLEX;
    port_cfg.link      = 1;
    port_cfg.nway      = 0;
    port_cfg.rxpause   = 0;
    port_cfg.txpause   = 0;

    if((retVal = rtk_port_macForceLinkExt_set(EXT_PORT0, MODE_EXT_RGMII, &port_cfg)) != RT_ERR_OK)
    {
        RTK_PRINT_ERRMSG_RETURN("Cpu port init failure!",retVal);
    }

    // rgmii delay
    if (RT_ERR_OK != (retVal = rtk_port_rgmiiDelayExt_set(EXT_PORT0, 1, MBB_RTK_RGMII_RX_DELAY)))
    {
        RTK_PRINT_ERRMSG_RETURN("rtk_port_rgmiiDelayExt_set failure!", retVal);
        return retVal;
    }

    //egress from CPU port not insert CPU tag
    if((retVal = rtk_cpu_tagPort_set(EXT_PORT0, CPU_INSERT_TO_NONE)) != RT_ERR_OK)
    {
        RTK_PRINT_ERRMSG_RETURN("Cpu port Insert to none setting failure!",retVal);
    }
    if((retVal = rtk_cpu_enable_set(ENABLED)) != RT_ERR_OK)
    {
        RTK_PRINT_ERRMSG_RETURN("Cpu port Enabled failure!",retVal);
    } 

    if((retVal =  rtk_port_phyEnableAll_set(ENABLED)) != RT_ERR_OK)
    {
        RTK_PRINT_ERRMSG_RETURN("Phy port EnableAll failure!",retVal);
    }    

    /*3, Init LAN/WAN Port, port 3 is WAN, port0~2 is LAN*/
#ifdef  MBB_NET_RESERVED  //not need setting, default setting
    ability.AutoNegotiation = 1;
    ability.Full_1000 = 1;
    ability.AsyFC = 1;
    for (port = 0; port < 4; port++)
    {
        if((retVal = rtk_port_phyAutoNegoAbility_set(port,&ability)) != RT_ERR_OK)
        {
            RTK_PRINT_ERRMSG_RETURN("LAN/WAN port init failure!", retVal);
        }
    }    
#endif

    /*4, Init interrupt for linkstatus change*/
    if((retVal = rtk_int_control_set(INT_TYPE_LINK_STATUS, ENABLED)) != RT_ERR_OK)
    {
        RTK_PRINT_ERRMSG_RETURN("Enable linkstatus change interrupt failure!",retVal);
    }

    if((retVal = rtk_int_polarity_set(INT_POLAR_LOW)) != RT_ERR_OK)
    {
        RTK_PRINT_ERRMSG_RETURN("Set polarity LOW failure!",retVal);
    }

#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH_SVLAN)
    retVal = mbb_rtk_svlan_init();
    if (RT_ERR_OK != retVal)
    {
        RTK_PRINT_ERRMSG_RETURN("svlan init failure!",retVal);
    }
#else
    /*5, Init VLAN setting*/
    if((retVal = rtk_vlan_init()) != RT_ERR_OK)
    {
        RTK_PRINT_ERRMSG_RETURN("Vlan init failure!",retVal);
    }      

    /*VLAN-2 for P0,P5, VLAN-3 for P1,P5, VLAN-4 for P2,P5, VLAN-5 for P3,P5
     所有端口的报文数据均走到内核，如果希望两个端口直接直接转发，可以通过设置untag
     如P0、P1和P2直接转发，P0、P1和P2都需要修改untag为
    RTK_PORTMASK_PORT_SET(vlan_cfg.untag, UTP_PORT0)；
    RTK_PORTMASK_PORT_SET(vlan_cfg.untag, UTP_PORT1)；
    RTK_PORTMASK_PORT_SET(vlan_cfg.untag, UTP_PORT2)*/
#if (FEATURE_ON != MBB_FACTORY) && (defined(BSP_CONFIG_BOARD_CPE_B610s_77a) || defined(BSP_CONFIG_BOARD_CPE_B610s_79a))
    memset(&vlan_cfg, 0x00, sizeof(rtk_vlan_cfg_t));

    for (port = UTP_PORT0; port <= UTP_PORT3; port++)
    {
        RTK_PORTMASK_PORT_SET(vlan_cfg.mbr, port);
        RTK_PORTMASK_PORT_SET(vlan_cfg.untag, port);
        if ((retVal = rtk_vlan_portPvid_set(port,VLAN_PORT1,0)) != RT_ERR_OK)
        {
            RTK_PRINT_ERRMSG_RETURN("Set Port PVID failure!",retVal);
        }
    }
    
    RTK_PORTMASK_PORT_SET(vlan_cfg.mbr, EXT_PORT0);
    vlan_cfg.ivl_en = ENABLED;
    if ((retVal = rtk_vlan_set(VLAN_PORT1, &vlan_cfg)) != RT_ERR_OK)
    {
        RTK_PRINT_ERRMSG_RETURN("Set VLAN failure!",retVal);
    }
#else
    for (port = UTP_PORT0; port <= UTP_PORT3; port++)  /*as: ing*/
    {
#if (FEATURE_ON== MBB_FEATURE_ETH_SWITCH_8363)
            if ((UTP_PORT0 == port) || (UTP_PORT2 == port))
            {
                continue;
            }
#endif
        memset(&vlan_cfg, 0x00, sizeof(rtk_vlan_cfg_t));
        RTK_PORTMASK_PORT_SET(vlan_cfg.mbr, port);
        RTK_PORTMASK_PORT_SET(vlan_cfg.mbr, EXT_PORT0);
        RTK_PORTMASK_PORT_SET(vlan_cfg.untag, port);
        vlan_cfg.ivl_en = ENABLED;
        if((retVal = rtk_vlan_set(vlanArry[port], &vlan_cfg)) != RT_ERR_OK)
        {
            RTK_PRINT_ERRMSG_RETURN("Set VLAN failure!",retVal);
        }

        if ((retVal = rtk_vlan_portPvid_set(port, vlanArry[port], 0)) != RT_ERR_OK)
        {
            RTK_PRINT_ERRMSG_RETURN("Set Port PVID failure!",retVal);
        }
    }
#endif
#endif

#ifdef  MBB_NET_RESERVED
    //VLAN 500 for flood unknown DA from WLAN->LAN,
    memset(&vlan_cfg, 0x00, sizeof(rtk_vlan_cfg_t));  //ing
    RTK_PORTMASK_ALLPORT_SET(vlan_cfg.mbr);  /*rtk_switch_init中会设置valid_portmask*/
    for (port = UTP_PORT0; port < UTP_PORT3; port++)
    {
        RTK_PORTMASK_PORT_SET(vlan_cfg.untag, port);
    }

    vlan_cfg.ivl_en = ENABLED;
    if ((retVal = rtk_vlan_set(500,&vlan_cfg)) != RT_ERR_OK)
    {
        RTK_PRINT_ERRMSG_RETURN("Init Flood WLAN->LAN vlan seting failure!", retVal);
    }
#endif    

    /*6 Set unknown DA flood*/
    if((retVal = rtk_trap_unknownUnicastPktAction_set(UCAST_UNKNOWNDA, UCAST_ACTION_FLOODING)) != RT_ERR_OK)
    {
        RTK_PRINT_ERRMSG_RETURN("Init unknown unicast DA action failure!",retVal);
    }
    
    RTK_SCAN_ALL_LOG_PORT(port)
    {
        if((retVal = rtk_trap_unknownMcastPktAction_set(port, MCAST_L2, MCAST_ACTION_FORWARD)) != RT_ERR_OK)
        {
            RTK_PRINT_ERRMSG_RETURN("Init unknown l2 multicast action forward failure!",retVal);
        }

        if((retVal = rtk_trap_unknownMcastPktAction_set(port, MCAST_IPV4, MCAST_ACTION_FORWARD)) != RT_ERR_OK)
        {
            RTK_PRINT_ERRMSG_RETURN("Init unknown ipv4 multicast action forward failure!",retVal);
        }

        if((retVal = rtk_trap_unknownMcastPktAction_set(port, MCAST_IPV6, MCAST_ACTION_FORWARD)) != RT_ERR_OK)
        {
            RTK_PRINT_ERRMSG_RETURN("Init unknown ipv6 multicast action forward failure!",retVal);
        }
    }
   /*软银闪灯控制*/
    /*LED灯硬件接反，软件适配*/
    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_P1_LED_MUX, LED_MUX_MODE)) != RT_ERR_OK)
    {
        return retVal;
    }
/*软银定制灯常亮，数传也不闪，烧片版本需要闪*/
#if defined(BSP_CONFIG_BOARD_CPE_B610s_77a) || defined(BSP_CONFIG_BOARD_CPE_B610s_79a)
#if(FEATURE_ON != MBB_FACTORY)
    /*设置LED0控制模式*/
    
    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_LED0_DATA_CTRL, LED0_MODE)) != RT_ERR_OK)
    {
        return retVal;
    }
	/*设置LED1控制模式*/
    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_LED1_DATA_CTRL, LED1_MODE)) != RT_ERR_OK)
    {
        return retVal;
    }
#endif /*end MBB_FACTORY*/
#endif /*end BSP_CONFIG_BOARD_CPE*/

    LAN_DEBUG("rtk_phyPatch_set enter\r\n");
    retVal = rtk_phyPatch_set();
    if(RT_ERR_OK != retVal)
    {
        LAN_DEBUG("rtk_phyPatch_set init failed !\r\n");
    }
    return RT_ERR_OK;
}

/*****************************************************************************
函数名：   rtk8367_cpu_port_init
功能描述:  switch CPU port是一个常连状态，强制配置成1G全双工
输入参数： phydev
返回值：  无
*****************************************************************************/
static void rtk8367_cpu_port_init(struct phy_device *phydev)
{
    if (NULL == phydev)
    {
        return;
    }

    LAN_DEBUG("rtk8367 CPU port init\r\n");

    phydev->link = 1;
    phydev->duplex = DUPLEX_FULL;
    phydev->speed = SPEED_1000;
    
    phydev->autoneg = AUTONEG_DISABLE;
}

/*****************************************************************************
函数名：   rtk8367_config_init
功能描述:  设备初始化
输入参数： phydev
返回值：   NET_RET_OK(0)为ok
*****************************************************************************/
static int rtk8367_config_init(struct phy_device *phydev)
{
    int reg, err;

    if (NULL == phydev)
    {
        return NET_RET_FAIL;
    }

    LAN_DEBUG("rtk8367 driver configinit\r\n");

    s_phydev = phydev;

    /*rtk8367初始化*/
    rtk_switchDevice_init();

    /*rtk8367强制为1G全双工，使CPU更新MAC状态*/
    rtk8367_cpu_port_init(phydev);

    /*网口模块sysfs节点初始化*/
    hw_net_sysfs_init();

    return NET_RET_OK;
}

/*****************************************************************************
函数名：   rtk8367_read_status
功能描述:  phy设备获取状态
输入参数： phydev
返回值：   NET_RET_OK(0)为获取状态OK
*****************************************************************************/
int rtk8367_read_status(struct phy_device *phydev)
{
    if (NULL == phydev)
    {
        return NET_RET_FAIL;
    }

    phydev->link = 1;
    phydev->duplex = DUPLEX_FULL;
    phydev->speed = SPEED_1000;
   
    //LAN_DEBUG("AR8035 phy qunedread status link:%d, speed:%d, duplex:%d\r\n", phydev->link, phydev->speed, phydev->duplex);

    return NET_RET_OK;
}

/*****************************************************************************
函数名：   rtk_get_port_status_info
功能描述:  为应用层获取端口的状态
输入参数： startPort 起始端口
                            num 连续的端口个数
输出参数：端口状态信息                            
返回值：   NET_RET_OK(0)为获取状态OK
*****************************************************************************/
void rtk_get_port_status_info(LAN_ETH_STATE_INFO_ST *pEthInfo, int startPort, int num)
{
    rtk_port_mac_ability_t Portstatus;
    int port_rtk;
    int link_speed[] = {SPEED_10, SPEED_100, SPEED_1000};

    if (NULL == pEthInfo)
    {    
        return;
    }
    mutex_lock(&rtk_lock);
    for (port_rtk = startPort; port_rtk < startPort + num; port_rtk++)
    {
        rtk_port_macStatus_get(port_rtk, &Portstatus);
        pEthInfo->link_state = Portstatus.link;
        pEthInfo->dpulex = Portstatus.duplex;
        pEthInfo->speed = link_speed[Portstatus.speed % (sizeof(link_speed) / sizeof(int))];

        LAN_DEBUG("%s: Port:%d, link:%d, duplex:%d, speed:%d\n",
            __FUNCTION__, port_rtk, pEthInfo->link_state, pEthInfo->dpulex, pEthInfo->speed);

        pEthInfo++;
    }
    mutex_unlock(&rtk_lock);
}

/*****************************************************************************
函数名：   rtk_get_port_phyStatus
功能描述:  为应用层获取端口的状态
输入参数： buf 用来保存网卡信息
输出参数：端口状态信息                            
返回值：   >=0为获取状态OK， <0 erro
*****************************************************************************/
int  rtk_get_port_phyStatus(char *buf,unsigned int len)
{
    int LinkStatus =0;
    int Speed = 0;
    int Duplex = 0;
    int port = 0;
    rtk_api_ret_t ret = -1;
    int idx = 0;
    char * pbuf = NULL;
    if ((NULL == buf) || (MAX_SHOW_LAN < len))
    {
        LAN_DEBUG("rtk_get_port_phyStatus error buf is null\n");
        return -1;
    }
    mutex_lock(&rtk_lock);
    pbuf = buf;
    for (port = UTP_PORT0; port <= UTP_PORT3; port++)  /*as: ing*/
    {
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH_8363)
        if ((UTP_PORT0 == port) || (UTP_PORT2 == port))
        {
            continue;
        }
#endif
        ret = rtk_port_phyStatus_get(port, &LinkStatus, &Speed, &Duplex);
        if (RT_ERR_OK != ret)
        {
            LAN_DEBUG("rtk_port_phyStatus_get error :%d\n", ret);
            return ret;
        }

        LAN_DEBUG("%s: Port:%d, link:%d,speed:%d, duplex:%d\n", __FUNCTION__, port, LinkStatus, Speed, Duplex);

        switch (port)
        {
#if (FEATURE_OFF == MBB_FEATURE_ETH_SWITCH_8363)
            case UTP_PORT0:
            {
                snprintf(pbuf, (MAX_SHOW_LAN-idx), "eth0.2:%d,%d,%d;", LinkStatus, Speed, Duplex);
            }
            break;
#endif
            case UTP_PORT1:
            {
                snprintf(pbuf, (MAX_SHOW_LAN-idx), "eth0.3:%d,%d,%d;", LinkStatus, Speed, Duplex);
                        }
            break;
#if (FEATURE_OFF == MBB_FEATURE_ETH_SWITCH_8363)
            case UTP_PORT2:
            {
                snprintf(pbuf, (MAX_SHOW_LAN-idx), "eth0.4:%d,%d,%d;", LinkStatus, Speed, Duplex);
            }
            break;
#endif
            case UTP_PORT3:
            {
                snprintf(pbuf, (MAX_SHOW_LAN-idx), "eth0:%d,%d,%d;", LinkStatus, Speed, Duplex);
            }
            break;
            default:
            {
                LAN_DEBUG("rtk_port_phyStatus_get port error :%d\n", port);
            }
        }
        idx = strlen(buf);
        pbuf = buf+idx; 
    }
    mutex_unlock(&rtk_lock);
    return idx;
}


/*****************************************************************************
函数名：   rtk_reset_allport
功能描述:  对所有端口进行reset
输入参数： wan_exclude 是否排除WAN口                       
返回值：   无
*****************************************************************************/
void rtk_reset_allport(int wan_exclude)
{
    int port;
    int portNum = SWITCH_LAN_PORT_NUM;

    mutex_lock(&rtk_lock);
#if (FEATURE_ON== MBB_FEATURE_ETH_SWITCH_8363)
    port = UTP_PORT1;
    rtk_port_adminEnable_set((rtk_port_t)port, DISABLED) ;
    if (0 == wan_exclude)
    {
        port = UTP_PORT3;
        rtk_port_adminEnable_set((rtk_port_t)port, DISABLED) ;
    }
    mdelay(1);
    port = UTP_PORT1;
    rtk_port_adminEnable_set((rtk_port_t)port, ENABLED) ;
    if (0 == wan_exclude)
    {
        port = UTP_PORT3;
        rtk_port_adminEnable_set((rtk_port_t)port, ENABLED) ;
    }
#else
    if (!wan_exclude)
    {
        portNum++;
    }

    for (port = UTP_PORT0; port < UTP_PORT0 + portNum;port++)
    {
        rtk_port_adminEnable_set((rtk_port_t)port, DISABLED) ;
    }

    mdelay(1);

    for (port = UTP_PORT0; port < UTP_PORT0 + portNum; port++)
    {
        rtk_port_adminEnable_set((rtk_port_t)port, ENABLED) ;
    }
#endif
    mutex_unlock(&rtk_lock);

}

/*****************************************************************************
函数名：   rtk_poll_port_status
功能描述:  查询所有端口状态                    
返回值：  
    ETH_CHANGE_NONE  //无变化
    ETH_LAN_DOWN       //LAN口down
    ETH_LAN_UP             //LAN口up
    ETH_WAN_DOWN       //WAN口down
    ETH_WAN_UP             //WAN口up
*****************************************************************************/
int rtk_poll_port_status(unsigned int *all_port_status)
{
    int j;   
    rtk_int_info_t infodown, infoup;
    rtk_port_t port_rtk;
    rtk_port_mac_ability_t Portstatus;
    int change = ETH_CHANGE_NONE;
    static unsigned int port_status = 0;

    mutex_lock(&rtk_lock);
    rtk_int_advanceInfo_get(ADV_PORT_LINKDOWN_PORT_MASK, &infodown);
    rtk_int_advanceInfo_get(ADV_PORT_LINKUP_PORT_MASK, &infoup);

    if (0 != infoup.portMask.bits[0])
    {
        for (j = UTP_PORT0; j <= UTP_PORT3; j++)
        {
#if (FEATURE_ON== MBB_FEATURE_ETH_SWITCH_8363)
            if ((UTP_PORT0 == j) || (UTP_PORT2 == j))
            {
                continue;
            }
#endif
            if (infoup.portMask.bits[0] & (1 << j))
            {     
                    port_rtk = (rtk_port_t)j;
                    rtk_port_macStatus_get(port_rtk, &Portstatus);

                    if (UTP_PORT3 == j)
                    {
                        change = ETH_WAN_UP;
                    }
                    else
                    {
                        change = ETH_LAN_UP;
                    }

                    port_status |= 1 << j;

                    LAN_DEBUG("rtk8367 port%d link:%d, duplex:%d, speed:%d\r\n", j, Portstatus.link, Portstatus.duplex, Portstatus.speed);		   
                    eth_port_link_report(change);
            }
        }
    }

    if (0 != infodown.portMask.bits[0])
    {
        for (j = UTP_PORT0; j <= UTP_PORT3; j++)
        {
#if (FEATURE_ON== MBB_FEATURE_ETH_SWITCH_8363)
            if ((UTP_PORT0 == j) || (UTP_PORT2 == j))
            {
                continue;
            }
#endif
            if (infodown.portMask.bits[0] & (1 << j))
            {
                LAN_DEBUG("rtk8367 port%d linkdown\r\n", j); 

                if (UTP_PORT3 == j)
                {
                    change = ETH_WAN_DOWN;
                }
                else
                {
                    change = ETH_LAN_DOWN;
                }
                eth_port_link_report(change);
                port_status &= ~(1 << j);
            }
        }
    }

    *all_port_status = port_status;

    mutex_unlock(&rtk_lock);
    return change;
}

static struct phy_driver rtk8367_driver = {
    .phy_id         = RTK8367_ID,
    .phy_id_mask    = RTK8367_ID_MASK,      
    .name           = "rtk8367 switch",
    .read_status    = rtk8367_read_status,
    .config_init    = rtk8367_config_init,
    .driver         = { .owner = THIS_MODULE,},
};


/*****************************************************************************
函数名：   rtk8367_init
功能描述:  rtk8367设备注册
输入参数： 无
返回值：   
*****************************************************************************/
static int __init rtk8367_init(void)
{
    int ret;

    LAN_DEBUG("rtk8367 phy driver register\r\n");

    mutex_init(&rtk_lock);
    //reset T1信号不满足条件，需要reset
    if (gpio_is_valid(LAN_PHY_RESET))
    {
        ret = gpio_request(LAN_PHY_RESET, "8363nb");
        if ( 0 > ret)
        {
            LAN_DEBUG("rtk836x phy reset error\r\n");
        }
        else
        {
            (void)gpio_direction_output(LAN_PHY_RESET, 1);
            mdelay(PHY_RST_DELAY);
            (void)gpio_direction_output(LAN_PHY_RESET, 0);
        }

    }
    LAN_DEBUG("rtk836x phy reset\r\n");

    ret = phy_driver_register(&rtk8367_driver);

    return ret;
}

/*****************************************************************************
函数名：   rtk8367_exit
功能描述: rtk8367设备退出
输入参数： 无
返回值：   
*****************************************************************************/
static void __exit rtk8367_exit(void)
{
    phy_driver_unregister(&rtk8367_driver);

    hw_net_sysfs_uninit();
}

module_init(rtk8367_init);
module_exit(rtk8367_exit);

static struct mdio_device_id __maybe_unused rtk8367_tbl[] = {
    {RTK8367_ID, RTK8367_ID_MASK},
    { }
};

MODULE_DEVICE_TABLE(mdio, rtk8367_tbl);

/*****************************************************************************
函数名：   rtk_led_ctrl()
功能描述:  设置led灯亮灭    
输入参数: leds 灯的序号，mode:0 灭,1:   亮
返回值：0 OK  <0 err  

*****************************************************************************/
int rtk_led_ctrl(unsigned char leds, unsigned char mode)
{
    unsigned char led_idx = 0;
    unsigned char mode_rtk = 0;
    unsigned char port = 0;
    unsigned char group = 0;
    struct _led_adp led_adptors[LAN_LED_MAX] =
    {
        {LAN_LED_1, UTP_PORT1, 0},
        {LAN_LED_2, UTP_PORT1, 1},
        {LAN_LED_3, UTP_PORT3, 0},
        {LAN_LED_4, UTP_PORT3, 1},
    };
    printk("rtk_led_ctrl leds:%d mode:%d\n", (int)leds, (int)mode);
    if ((LAN_LED_MAX >= leds) && (0 < leds))
    {
        led_idx = (leds - 1);
        /*mode:0 灭,1:   亮 */
        if (0 == mode)
        {
            mode_rtk = LED_FORCE_OFF;
        }
        else if (1 == mode)
        {
            mode_rtk = LED_FORCE_ON;
        }
        else
        {
            //error
            printk("rtk_led_ctrl not support this mode!!!\n");
            return -1;

        }
        port = led_adptors[led_idx].port;
        group = led_adptors[led_idx].group;
        return rtk_led_modeForce_set(port, group, mode_rtk);
    }
    else
    {
        //error;
        printk("rtk_led_ctrl not support so many leds!!!\n");
        return -1;
    }
}

/*****************************************************************************
函数名:       rtk_setphymode
功能描述:   Set PHY in test mode.
输入参数:
       port - port id.
       mode - PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
 返回值：   
      RT_ERR_OK              	- OK
      RT_ERR_FAILED          	- Failed
      RT_ERR_SMI             	- SMI access error
      RT_ERR_PORT_ID 			- Invalid port number.
      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
      RT_ERR_NOT_ALLOWED      - The Setting is not allowed, caused by set more than 1 port in Test mode.
 * Note:
 *      Set PHY in test mode and only one PHY can be in test mode at the same time.
 *      It means API will return FAILED if other PHY is in test mode.
 *      This API only provide test mode 1 ~ 4 setup.
 *****************************************************************************/
rtk_api_ret_t rtk_setphymode(rtk_port_t port, rtk_port_phy_test_mode_t mode)
{
    rtk_uint32          data, regData, i, index, phy, reg;
    rtk_api_ret_t       retVal;

    RTK_CHK_PORT_IS_UTP(port);

    if(mode >= PHY_TEST_MODE_END)
    {
        return RT_ERR_INPUT;
    }
    
    if (PHY_TEST_MODE_NORMAL != mode)
    {
        /* Other port should be Normal mode */
        RTK_SCAN_ALL_LOG_PORT(i)
        {
            if(rtk_switch_isUtpPort(i) == RT_ERR_OK)
            {
                if(i != port)
                {
                    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(i), 9, &data)) != RT_ERR_OK)
                    {
                        return retVal;
                    }    

                    if((data & 0xE000) != 0)
                    {
                        return RT_ERR_NOT_ALLOWED;
                    }    
                }
            }
        }
    }

    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), 9, &data)) != RT_ERR_OK)
    {
        return retVal;
    }
    
    data &= ~0xE000;
    data |= (mode << 13);
    if ((retVal = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(port), 9, data)) != RT_ERR_OK)
    {
        return retVal;
    }
    
    if (PHY_TEST_MODE_4 == mode)
    {
        if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
        {
            return retVal;
        }    

        if((retVal = rtl8367c_getAsicReg(0x1300, &regData)) != RT_ERR_OK)
        {
            return retVal;
        }
        
        if( (regData == 0x0276) || (regData == 0x0597) )
        {
            if ((retVal = rtl8367c_setAsicPHYOCPReg(rtk_switch_port_L2P_get(port), 0xbcc2, 0xF4F4)) != RT_ERR_OK)
            {
                return retVal;
            }    
        }

        if( (regData == 0x6367) )
        {
            if ((retVal = rtl8367c_setAsicPHYOCPReg(rtk_switch_port_L2P_get(port), 0xbcc2, 0x77FF)) != RT_ERR_OK)
            {
                return retVal;
            }    
        }
    }

    
    printk("rtk_set port:%d mode:%d\n", (int)port, (int)mode);

    return RT_ERR_OK;
}

/*****************************************************************************
函数名:   rtk_getphymode
函数功能:         Get PHY in which test mode.
输入参数:
       port - Port id.
返回值:
      PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
 * Note:
 *      Get test mode of PHY from register setting 9.15 to 9.13.
 *****************************************************************************/
rtk_port_phy_test_mode_t rtk_getphymode(rtk_port_t port)
{
    rtk_uint32      data;
    rtk_api_ret_t   retVal;
    rtk_port_phy_test_mode_t phyMode;

    RTK_CHK_PORT_IS_UTP(port);

    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), 9, &data)) != RT_ERR_OK)
    {
        printk("rtk_getphymode err:%d\n", retVal);
        return PHY_TEST_MODE_END;
    }    

    phyMode = (data & 0xE000) >> 13;

    return phyMode;
}


ret_t rtk_setReg(rtk_uint32 reg, rtk_uint32 value)
{
    printk("%s: 0x%x value:%x \n", __FUNCTION__, reg, value);
    return rtl8367c_setAsicReg(reg, value);
}

rtk_uint32 rtk_getReg(rtk_uint32 reg)
{
    rtk_uint32 regValue;

    (void)rtl8367c_getAsicReg(reg, &regValue);

    printk("%s: 0x%x value:%x \n", __FUNCTION__, reg, regValue);
    return regValue;
}

/***********************************以下是网口调试打印信息:mib和OCP寄存器读取*********************************/
void rtk_get_port_mib(rtk_port_t port)
{
    rtk_stat_port_cntr_t  portCntrs = {0};
    
    memset((void *)&portCntrs, 0, sizeof(portCntrs));
    
    if (RT_ERR_OK != rtk_stat_port_getAll(port, &portCntrs))
    {
        printk(KERN_ERR"rtk_stat_port_getAll failed!!!\n");
        return;
    }

    printk("\nport %d: %-35s[%02d]:  %25lld",port,"ifInOctets",0, portCntrs.ifInOctets);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"dot3StatsFCSErrors",1, portCntrs.dot3StatsFCSErrors);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"dot3StatsSymbolErrors",2, portCntrs.dot3StatsSymbolErrors);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"dot3InPauseFrames",3, portCntrs.dot3InPauseFrames);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"dot3ControlInUnknownOpcodes",4, portCntrs.dot3ControlInUnknownOpcodes);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsFragments",5, portCntrs.etherStatsFragments);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsJabbers",6, portCntrs.etherStatsJabbers);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"ifInUcastPkts",7, portCntrs.ifInUcastPkts);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsDropEvents",8, portCntrs.etherStatsDropEvents);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsOctets",9, portCntrs.etherStatsOctets);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsUndersizePkts",10,portCntrs.etherStatsUndersizePkts);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsOversizePkts",11,portCntrs.etherStatsOversizePkts);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsPkts64Octets",12,portCntrs.etherStatsPkts64Octets);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsPkts65to127Octets",13,portCntrs.etherStatsPkts65to127Octets);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsPkts128to255Octets",14,portCntrs.etherStatsPkts128to255Octets);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsPkts256to511Octets",15,portCntrs.etherStatsPkts256to511Octets);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsPkts512to1023Octets",16,portCntrs.etherStatsPkts512to1023Octets);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsPkts1024toMaxOctets",17,portCntrs.etherStatsPkts1024toMaxOctets);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsMcastPkts",18,portCntrs.etherStatsMcastPkts);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsBcastPkts",19,portCntrs.etherStatsBcastPkts);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"ifOutOctets",20,portCntrs.ifOutOctets);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"dot3StatsSingleCollisionFrames",21,portCntrs.dot3StatsSingleCollisionFrames);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"dot3StatsMultipleCollisionFrames",22,portCntrs.dot3StatsMultipleCollisionFrames);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"dot3StatsDeferredTransmissions",23,portCntrs.dot3StatsDeferredTransmissions);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"dot3StatsLateCollisions",24,portCntrs.dot3StatsLateCollisions);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"etherStatsCollisions",25,portCntrs.etherStatsCollisions);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"dot3StatsExcessiveCollisions",26,portCntrs.dot3StatsExcessiveCollisions);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"dot3OutPauseFrames",27,portCntrs.dot3OutPauseFrames);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"dot1dBasePortDelayExceededDiscards",28,portCntrs.dot1dBasePortDelayExceededDiscards);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"dot1dTpPortInDiscards",29,portCntrs.dot1dTpPortInDiscards);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"ifOutUcastPkts",30,portCntrs.ifOutUcastPkts);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"ifOutMulticastPkts",31,portCntrs.ifOutMulticastPkts);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"ifOutBrocastPkts",32,portCntrs.ifOutBrocastPkts);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"outOampduPkts",33,portCntrs.outOampduPkts);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inOampduPkts",34,portCntrs.inOampduPkts);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"pktgenPkts",35,portCntrs.pktgenPkts);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inMldChecksumError",36,portCntrs.inMldChecksumError);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inIgmpChecksumError",37,portCntrs.inIgmpChecksumError);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inMldSpecificQuery",38,portCntrs.inMldSpecificQuery);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inMldGeneralQuery",39,portCntrs.inMldGeneralQuery);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inIgmpSpecificQuery",40,portCntrs.inIgmpSpecificQuery);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inIgmpGeneralQuery",41,portCntrs.inIgmpGeneralQuery);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inMldLeaves",42,portCntrs.inMldLeaves);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inIgmpLeaves",43,portCntrs.inIgmpLeaves);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inIgmpJoinsSuccess",44,portCntrs.inIgmpJoinsSuccess);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inIgmpJoinsFail",45,portCntrs.inIgmpJoinsFail);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inMldJoinsSuccess",46,portCntrs.inMldJoinsSuccess);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inMldJoinsFail",47,portCntrs.inMldJoinsFail);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inReportSuppressionDrop",48,portCntrs.inReportSuppressionDrop);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inLeaveSuppressionDrop",49,portCntrs.inLeaveSuppressionDrop);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"outIgmpReports",50,portCntrs.outIgmpReports);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"outIgmpLeaves",51,portCntrs.outIgmpLeaves);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"outIgmpGeneralQuery",52,portCntrs.outIgmpGeneralQuery);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"outIgmpSpecificQuery",53,portCntrs.outIgmpSpecificQuery);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"outMldReports",54,portCntrs.outMldReports);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"outMldLeaves",55,portCntrs.outMldLeaves);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"outMldGeneralQuery",56,portCntrs.outMldGeneralQuery);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"outMldSpecificQuery",57,portCntrs.outMldSpecificQuery);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"inKnownMulticastPkts",58,portCntrs.inKnownMulticastPkts);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"ifInMulticastPkts",59,portCntrs.ifInMulticastPkts);
    printk("\nport %d: %-35s[%02d]:  %25lld",port,"ifInBroadcastPkts",60,portCntrs.ifInBroadcastPkts);
    printk("\n");
}

void rtk_get_port_stat(rtk_port_t port)
{
    rtk_port_linkStatus_t LinkStatus = 0;
    rtk_data_t Speed = 0;
    rtk_data_t Duplex = 0;
    rtk_portmask_t Portmask = {0};
    rtk_vlan_t Pvid = 0;
    rtk_pri_t Priority = 0;
    
    if (RT_ERR_OK != rtk_port_phyStatus_get(port, &LinkStatus, &Speed, &Duplex))
        printk(KERN_ERR"rtk_port_phyStatus_get failed!!!\n");
    if (RT_ERR_OK != rtk_port_isolation_get(port, &Portmask))
        printk(KERN_ERR"rtk_port_isolation_get failed!!!\n");
    if (RT_ERR_OK != rtk_vlan_portPvid_get(port, &Pvid, &Priority))
        printk(KERN_ERR"rtk_vlan_portPvid_get failed!!!\n");

    printk(KERN_ERR"port %d : LinkStatus = %d, Speed = %d, Duplex = %d, Portmask = 0x%x, Pvid = %d, Priority = %d\n",
        port,LinkStatus,Speed,Duplex,Portmask.bits[0],Pvid,Priority);
}

void rtk_force_stat(rtk_port_t port)
{
    rtk_mode_ext_t Mode = 0;
    rtk_port_mac_ability_t Portability = {0};
    if (RT_ERR_OK != rtk_port_macForceLinkExt_get(port, &Mode, &Portability))
        printk(KERN_ERR"rtk_port_macForceLinkExt_get failed!!!\n");
    
    printk(KERN_ERR"port %d : forcemode = %d, speed = %d, duplex = %d, link = 0x%x, nway = %d, txpause = %d, rxpause = %d\n",
        port,Portability.forcemode,Portability.speed,Portability.duplex,Portability.link,Portability.nway,Portability.txpause,Portability.rxpause);
}

void rtk_vlan(rtk_vlan_t vid)
{
    rtk_vlan_cfg_t VlanCfg = {0};
    if (RT_ERR_OK != rtk_vlan_get(vid, &VlanCfg))
        printk(KERN_ERR"rtk_stat_port_getAll failed!!!\n");
    printk(KERN_ERR"vid %d : mbr = 0x%x, untag = 0x%x, ivl_en = %d, fid_msti= 0x%x, envlanpol = %d, meteridx = %d, vbpen = %d, vbpri = %d\n",
        vid,VlanCfg.mbr.bits[0],VlanCfg.untag.bits[0],VlanCfg.ivl_en,VlanCfg.fid_msti,VlanCfg.envlanpol,VlanCfg.meteridx,VlanCfg.vbpen,VlanCfg.vbpri);
}

void rtk_help(void)
{
    printk("rtk_api_ret_t rtk_stat_global_reset\(void\)\n");
    printk("void rtk_get_port_mib\(rtk_port_t port\)\n");
    printk("void rtk_get_port_stat\(rtk_port_t port\)\n");
    printk("void rtk_force_stat\(rtk_port_t port\)\n");
    printk("void rtk_vlan\(rtk_vlan_t vid\)\n");
}

/* ocp add  0xbcdc 0xbcde */
void rtk_get_port_ocp(rtk_port_t port, rtk_uint32 ocpadd)
{
    rtk_uint32 data = 0;
    if (RT_ERR_OK != rtl8367c_getAsicPHYOCPReg(rtk_switch_port_L2P_get(port), ocpadd, &data))
    {
        printk("rtk_get_port_ocp failed!!! \n");
    }
    else
    {
        printk("ocp_reg_value: PORT = %d, ocpadd = %x, DATA = %x\n", port, ocpadd, data);
    }
}

EXPORT_SYMBOL(rtk_vlan);
EXPORT_SYMBOL(rtk_force_stat);
EXPORT_SYMBOL(rtk_get_port_mib);
EXPORT_SYMBOL(rtk_get_port_stat);
/***********************************end 网口调试打印信息:mib和OCP寄存器读取*********************************/

EXPORT_SYMBOL(rtk_setphymode);
EXPORT_SYMBOL(rtk_getphymode);
EXPORT_SYMBOL(rtk_setReg);
EXPORT_SYMBOL(rtk_getReg);
#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH_8363)
extern struct net_device * mbb_get_vlan_dev(rtk_uint16 vlan_id);

/*******************************************************************************
 函数名称  : mbb_vlan_flow_statistic
 功能描述  : MBB VLan MODE flow statistic for RTK8367
*******************************************************************************/

void mbb_vlan_flow_statistic(void)
{
    rtk_port_t port = UNDEFINE_PORT;
    rtk_stat_port_cntr_t  portCnt ;
    rt_error_code_t ret = 0;
    rtk_stat_counter_t rx_packets = 0;
    rtk_stat_counter_t rx_bytes = 0;
    rtk_stat_counter_t tx_packets = 0;
    rtk_stat_counter_t tx_bytes = 0;
    rtk_stat_counter_t tx_errors = 0;
    rtk_stat_counter_t rx_drops = 0;
    rtk_stat_counter_t tx_drops = 0;
    struct net_device* dev = NULL;
    mutex_lock(&rtk_lock);
    port = UTP_PORT1;
    for (port; port < UTP_PORT4; port++)
    {
        if ((UTP_PORT0 == port) || (UTP_PORT2 == port))
        {
            continue;
        }
        if ( (ret = rtk_stat_port_getAll(port, &portCnt) != RT_ERR_OK))
        {
            printk("^^^^current read port error\n");
            mutex_unlock(&rtk_lock);
            return ret ;
        }
        rx_packets = portCnt.ifInUcastPkts + portCnt.ifInMulticastPkts + portCnt.ifInBroadcastPkts;
        rx_bytes = portCnt.ifInOctets;
        tx_packets = portCnt.ifOutBrocastPkts + portCnt.ifOutMulticastPkts + portCnt.ifOutUcastPkts;
        tx_bytes = portCnt.ifOutOctets;
        tx_errors = portCnt.dot3StatsDeferredTransmissions + portCnt.dot3StatsExcessiveCollisions +
                    portCnt.dot3StatsLateCollisions + portCnt.dot3StatsMultipleCollisionFrames +
                    portCnt.dot3StatsSingleCollisionFrames + portCnt.etherStatsCollisions;
        rx_drops = portCnt.dot1dTpPortInDiscards;
        tx_drops = portCnt.ifOutDiscards;
        if ( UTP_PORT1 == port)
        {
            dev = mbb_get_vlan_dev(VLAN_PORT1);
        }
        else
        {
            dev = mbb_get_vlan_dev(VLAN_WAN_PORT);
        }
        if (dev)
        {
            dev->stats.rx_packets = (unsigned long)rx_packets;
            dev->stats.rx_bytes = (unsigned long)rx_bytes;
            dev->stats.tx_packets = (unsigned long)tx_packets;
            dev->stats.tx_bytes = (unsigned long)tx_bytes;
            dev->stats.tx_errors = (unsigned long)tx_errors;
            dev->stats.rx_dropped = (unsigned long)rx_drops;
            dev->stats.tx_dropped = (unsigned long)tx_drops;
        }
    }
    mutex_unlock(&rtk_lock);
}
EXPORT_SYMBOL(mbb_vlan_flow_statistic);
#endif
