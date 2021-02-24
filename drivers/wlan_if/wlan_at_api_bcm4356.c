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



#include <linux/module.h>
#include <linux/delay.h>
#include <linux/string.h>

#include <product_config.h>

#include "wlan_if.h"
#include "wlan_at_api.h"
#include "wlan_at.h"
#include "wlan_utils.h"
#include "wlu_at.h"

#define WLAN_AT_SSID_SUPPORT            2                  /*支持的SSID组数*/
#define WLAN_AT_KEY_SUPPORT             5                  /*支持的分组数*/
#define WLAN_AT_MODE_SUPPORT            "1,2,3,4,5"        /*支持的模式(a/b/g/n/ac)*/
#define WLAN_AT_BAND_SUPPORT            "0,1,2"            /*支持的带宽(0-20M/1-40M/2-80M/3-160M)*/
#define WLAN_AT_TSELRF_SUPPORT          "0,1,2,3"          /*支持的天线索引序列*/
#define WLAN_AT_GROUP_MAX               4                  /*支持的最大天线索引*/


/*WIFI功率的上下限*/
#define WLAN_AT_POWER_MIN               (-15)
#define WLAN_AT_POWER_MAX               (30)

/*WIFI发射机接收机模式*/
#define WLAN_AT_WIFI_TX_MODE            (17)
#define WLAN_AT_WIFI_RX_MODE            (18)

/*WiFi增益模式*/
#define AT_WIFI_MODE_ONLY_PA            0x00                /*WIFI只支持PA模式*/
#define AT_WIFI_MODE_ONLY_NOPA          0x01                /*WIFI只支持NO PA模式*/
#define AT_WIFI_MODE_PA_NOPA            0x02                /*WIFI同时支持PA模式和NO PA模式*/

#define RX_PACKET_SIZE                  1000                /*装备每次发包数*/

#define WLAN_CHANNEL_2G_MIN             1                   /*2.4G信道最小值*/
#define WLAN_CHANNEL_5G_MIN             36                  /*5G信道最小值*/
#define WLAN_CHANNEL_2G_MAX             14                  /*2.4G信道最大*/
#define WLAN_CHANNEL_2G_MIDDLE          6
#define WLAN_CHANNEL_5G_MAX             165                 /*5G信道最大*/

#define WLAN_CHANNEL_5G_W52_START       36
#define WLAN_CHANNEL_5G_W52_END         48
#define WLAN_CHANNEL_5G_W53_START       52
#define WLAN_CHANNEL_5G_W53_END         64
#define WLAN_CHANNEL_5G_W57_START       149
#define WLAN_CHANNEL_5G_W57_END         161

#define WLAN_CHANNEL_5G_INTERVAL        4                     /*5G信道间隔*/
#define WLAN_CHANNEL_5G_40M_INTERVAL    8                     /*5G 40M信道间隔*/
#define WLAN_CHANNEL_5G_80M_INTERVAL    16                     /*5G 80M信道间隔*/
#define WLAN_FREQ_5G_W52_80M_MIN        5180                  /*W52 80M最小频点*/
#define WLAN_FREQ_5G_W53_80M_MAX        5260                  /*W53 80M最大频点*/
#define WLAN_FREQ_5G_W56_80M_MIN        5500                  /*W56 80M最小频点*/
#define WLAN_FREQ_5G_W56_80M_MAX        5660                  /*W56 80M最大频点*/
#define WLAN_FREQ_5G_W57_80M_MIN        5745                  /*W57 80M最小频点*/
#define WLAN_FREQ_5G_W57_80M_MAX        5745                  /*W57 80M最大频点*/
#define WLAN_FREQ_2G_MAX                2484                  /*2.4G最大频点*/
#define WLAN_FREQ_5G_W52_MIN            5180                  /*W52最小频点*/
#define WLAN_FREQ_5G_W53_MAX            5320                  /*W53最大频点*/
#define WLAN_FREQ_5G_W52_40M_MIN        5180                  /*W52 40M最小频点*/
#define WLAN_FREQ_5G_W53_40M_MAX        5300                  /*W53 40M最大频点*/
#define WLAN_FREQ_5G_W56_MIN            5500                  /*W56最小频点*/
#define WLAN_FREQ_5G_W56_MAX            5720                  /*W56最大频点*/
#define WLAN_FREQ_5G_W56_40M_MIN        5500                  /*W56 40M最小频点*/
#define WLAN_FREQ_5G_W56_40M_MAX        5700                  /*W56 40M最大频点*/
#define WLAN_FREQ_5G_W57_MIN            5745                  /*W57最小频点*/
#define WLAN_FREQ_5G_W57_MAX            5825                  /*W57最大频点*/
#define WLAN_FREQ_5G_W57_40M_MIN        5745                  /*W57最小频点*/
#define WLAN_FREQ_5G_W57_40M_MAX        5785                  /*W57最大频点*/

#define WIFI_CMD_MAX_SIZE               256                   /*cmd字符串256长度*/
#define WIFI_CMD_8_SIZE                 8                     /*cmd字符串8长度*/
#define HUNDRED                         100

#define DALEY_100_TIME  100
#define DALEY_500_TIME  500
#define DALEY_1000_TIME 1000
#define DALEY_5000_TIME 5000
#define DALEY_2000_TIME 2000

/*******************************************************************************
    通用宏
*******************************************************************************/
#define WL_BEEN_DONE(ret, cmd, ...) do {    \
    WLAN_TRACE_INFO("ret:%d run:"cmd"\n",(ret),##__VA_ARGS__);    \
    if (ret) return (ret);    \
    msleep(DALEY_100_TIME);    \
}while(0)

/*向WiFi芯片下发配置命令*/
#define WIFI_TEST_CMD(cmd) do {                                 \
    int cmd_ret = wifi_tcmd(cmd);                             \
    WLAN_TRACE_INFO("[ret=%d]%s\n", cmd_ret, cmd);                \
    if (0 != cmd_ret)                                           \
    {                                                           \
        WLAN_TRACE_ERROR("Run CMD Error!!!\n");                   \
    }                                                           \
    msleep(DALEY_100_TIME);                                               \
}while(0)

#ifndef STATIC
    #define STATIC static
#endif
/*WiFi芯片使能断言检测*/
#define ASSERT_WiFi_OFF(ret)                    \
if (AT_WIENABLE_OFF == g_wlan_at_data.wifiStatus) \
{                                               \
    WLAN_TRACE_INFO("Exit on WiFi OFF\n");        \
    return ret;                                 \
}

/*WiFi全局变量结构体 */
typedef struct tagWlanATGlobal
{
    WLAN_AT_WIENABLE_TYPE   wifiStatus;    /*默认加载测试模式*/
    WLAN_AT_WIMODE_TYPE     wifiMode;      /*wifi协议模式*/
    WLAN_AT_WIBAND_TYPE     wifiBand;      /*wifi协议制式*/
    WLAN_AT_WIFREQ_STRU     wifiFreq;      /*wifi频点信息*/
    uint32                  wifiChannel;   /*wifi信道信息*/
    uint32                  wifiRate;      /*wifi发射速率*/
    int32                   wifiPower;     /*wifi发射功率*/
    WLAN_AT_FEATURE_TYPE    wifiTX;        /*wifi发射机状态*/
    WLAN_AT_WIRX_STRU       wifiRX;        /*wifi接收机状态*/
    WLAN_AT_WIRPCKG_STRU    wifiPckg;      /*wifi误包码*/
    uint32                  wifiGroup;     /*wifi天线模式*/
}WLAN_AT_GLOBAL_ST;



/*记录当前的WiFi模式，带宽，频率，速率等参数*/
STATIC WLAN_AT_GLOBAL_ST g_wlan_at_data = {AT_WIENABLE_TEST, AT_WIMODE_80211n, AT_WIBAND_20M
         , {2412, 0}, 1, 6500, 3175, AT_FEATURE_DISABLE, {AT_FEATURE_DISABLE, {0}, {0}}, AT_WiPARANGE_HIGH};/*开启WiFi的默认参数*/



STATIC int g_CT = 0;      /*取值为1或0:0表示TSELRF AT未发，在CT工位时需要下发WiFi初始化相关wl序列*/

STATIC int g_receivePackageFlag = 0; /* 收包正常标志位，0:正常，1:异常 */
STATIC int g_receivePackageTime = 0; /* 收包异常次数，异常一次加1 */

extern WLAN_AT_PACK_REP_ST g_wifi_packet_report;
extern WLAN_AT_PACK_REP_ST g_wifi_packet_new_rep;/*实时信息*/

/*WiFi有效性校验*/
STATIC int32 check_wifi_valid()
{        
    int ret = 0;
    ret = wl_ver();
    WL_BEEN_DONE(ret, "wl ver");
    return ret;
}

#define OPT_FILE_PATH "/system/bin/wifi_brcm/exe/otp_4356.bin"
static unsigned char cis_dump[WLC_IOCTL_MAXLEN];

/*WiFi有效性校验*/
STATIC int32 check_write_otp(char *filepath)
{        
    int ret = 0;
    int loop_count = LOOP_COUNT;
    int if_write = 0;
    int cis_dump_len = 0;

    WLAN_TRACE_INFO("check_write_otp enter!\n");
    do
    {
        //如果连续写3次均未写入成功的话，那么返回错误。
        if(loop_count <= 0)
        {
            WLAN_TRACE_ERROR("check_write_otp three times,return error!!\n");
            return AT_RETURN_FAILURE;
        }
        ret = wlu_cisdump(cis_dump, &cis_dump_len, &if_write);
        if(ret)
        {
            WLAN_TRACE_ERROR("Run wlu_cisdump error!\n");
            return ret;
        }
        else
        {
            WLAN_TRACE_INFO("wlu_cisdump OK!!!cis_dump_len is %d,if_write is %d\n",
                cis_dump_len, if_write);
            if((0 < cis_dump_len) && (1 == if_write))
            {
                break;
            }
            else
            {
                ret = wlu_ciswrite(1, filepath);   //1:加-p参数;0:不加-p参数
                if(ret)
                {
                    WLAN_TRACE_ERROR("Run wlu_ciswrite error!filepath is %s\n", filepath);
                    return ret;
                }
            }
        }
        loop_count--;
    }while(loop_count >= 0);

    WLAN_TRACE_INFO("check_write_otp exit!\n");
    return ret;
}

//////////////////////////////////////////////////////////////////////////
/*(1)^WIENABLE 设置WiFi模块使能 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiEnable(WLAN_AT_WIENABLE_TYPE onoff)
 功能描述  : 用于wifi 进入测试模式，正常模式，关闭wifi
 输入参数  :  0  关闭
              1  打开正常模式
              2  打开测试模式
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 ATSetWifiEnable(WLAN_AT_WIENABLE_TYPE onoff)
{
    int32 ret = AT_RETURN_SUCCESS;

    ret = check_wifi_valid();
    if (AT_RETURN_SUCCESS != ret)
    {       
        WLAN_TRACE_INFO("check_wifi_valid failed!\n");
        return ret;
    }
    switch (onoff)
    {
        case AT_WIENABLE_OFF:
            {                
                WLAN_TRACE_INFO("Set wifi to off mode\n");
                ret = wl_down();
                WL_BEEN_DONE(ret, "wl down");
                msleep(DALEY_2000_TIME);
                g_wlan_at_data.wifiStatus = AT_WIENABLE_OFF;
            }            
            break;
        case AT_WIENABLE_ON:
            {
                WLAN_TRACE_INFO("Set wifi to normal mode\n");
                ret = wl_up();
                WL_BEEN_DONE(ret, "wl up");
                ret = check_write_otp(OPT_FILE_PATH);
                if (AT_RETURN_SUCCESS != ret)
                {       
                    WLAN_TRACE_INFO("check_write_otp failed!\n");
                    return ret;
                }
                msleep(DALEY_100_TIME);
                
                g_wlan_at_data.wifiStatus = AT_WIENABLE_ON;
            }
            break;
        case AT_WIENABLE_TEST:
            {                
                WLAN_TRACE_INFO("Set wifi to test mode\n");    
                ret = wl_up();
                WL_BEEN_DONE(ret, "wl up");
                ret = check_write_otp(OPT_FILE_PATH);
                if (AT_RETURN_SUCCESS != ret)
                {       
                    WLAN_TRACE_INFO("check_write_otp failed!\n");
                    return ret;
                }
                g_wlan_at_data.wifiStatus = AT_WIENABLE_TEST;
            }         
            break;
        default: 
            ret = AT_RETURN_FAILURE;
            break;
    }   
    
    return ret;
}
/*****************************************************************************
 函数名称  : WLAN_AT_WIENABLE_TYPE WlanATGetWifiEnable()
 功能描述  : 获取当前的WiFi模块使能状态
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0  关闭
             1  正常模式(信令模式)
             2  测试模式(非信令模式)
 其他说明  : 
*****************************************************************************/
STATIC WLAN_AT_WIENABLE_TYPE ATGetWifiEnable(void)
{    
    int32 ret = AT_RETURN_SUCCESS;

    ret = check_wifi_valid();
    if (AT_RETURN_SUCCESS != ret)
    {       
        WLAN_TRACE_INFO("check_wifi_valid failed!\n");
        return AT_WIENABLE_OFF;
    }
    ret = check_write_otp(OPT_FILE_PATH);
    if (AT_RETURN_SUCCESS != ret)
    {       
        WLAN_TRACE_INFO("check_write_otp failed!\n");
        return AT_WIENABLE_OFF;
    }
    msleep(DALEY_100_TIME);
    
    return g_wlan_at_data.wifiStatus;
}

//////////////////////////////////////////////////////////////////////////
/*(2)^WIMODE 设置WiFi模式参数 目前均为单模式测试*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiMode(WLAN_AT_WIMODE_TYPE mode)
 功能描述  : 设置WiFi AP支持的制式
 输入参数  : 0,  CW模式
             1,  802.11a制式
             2,  802.11b制式
             3,  802.11g制式
             4,  802.11n制式
             5,  802.11ac制式
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 ATSetWifiMode(WLAN_AT_WIMODE_TYPE mode)
{
    int ret = 0;
    WLAN_TRACE_INFO("%s:enter\n", __FUNCTION__);
    WLAN_TRACE_INFO("WLAN_AT_WIMODE_TYPE:%d\n", mode);  

    if (mode > AT_WIMODE_MAX)
    {
        return (AT_RETURN_FAILURE);
    }

    if (AT_WIMODE_CW == mode)
    {
        WLAN_TRACE_INFO("wifi mode:%d\n", mode);  
    }
    else if (AT_WIMODE_80211a == mode)
    {
        /*2.4G 和 5G 选择放在tselrf AT中实现，规避测接收时不发wimode导致频段错误的问题*/
        /*do nothing，模式由信道和速率的wl命令确定*/
        WLAN_TRACE_INFO("wifi mode:%d\n", mode);  
    }
    else if (AT_WIMODE_80211b == mode)
    {
        WLAN_TRACE_INFO("wifi mode:%d\n", mode);  
    }
    else if (AT_WIMODE_80211g == mode)
    {
        WLAN_TRACE_INFO("wifi mode:%d\n", mode);  
    }
    else if (AT_WIMODE_80211n == mode)
    {
        /*MIMO 模式*/
        if(4 == g_wlan_at_data.wifiGroup)
        {
            /*天线模式*/
            ret = wl_txchain(CHAIN_2X2);
            WL_BEEN_DONE(ret, "wl txchain 3");
            /*天线模式*/
            ret = wl_rxchain(CHAIN_2X2);
            WL_BEEN_DONE(ret, "wl rxchain 3");
        }
    }
    else if (AT_WIMODE_80211ac == mode)
    {
         /*MIMO 模式*/
        if(4 == g_wlan_at_data.wifiGroup)
        {
            /*天线模式*/
            ret = wl_txchain(CHAIN_2X2);
            WL_BEEN_DONE(ret, "wl txchain 3");
            /*天线模式*/
            ret = wl_rxchain(CHAIN_2X2);
            WL_BEEN_DONE(ret, "wl rxchain 3");
        }
    }
    else
    {
        WLAN_TRACE_ERROR("WLAN_AT_WIMODE_TYPE:%d unknow!\n", mode); 
        return (AT_RETURN_FAILURE);
    }

    g_wlan_at_data.wifiMode = mode;
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiMode(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 获取当前WiFi支持的制式
             当前模式，以字符串形式返回eg: 2
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetWifiMode(WLAN_AT_BUFFER_STRU *strBuf)
{
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%d", g_wlan_at_data.wifiMode);
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiModeSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 获取WiFi芯片支持的所有协议模式
             支持的所有模式，以字符串形式返回eg: 2,3,4
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetWifiModeSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }
    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%s", WLAN_AT_MODE_SUPPORT);
    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
/*(3)^WIBAND 设置WiFi带宽参数 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiBand(WLAN_AT_WIBAND_TYPE width)
 功能描述  : 用于设置wifi带宽
 输入参数  : 0 20M
             1 40M
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 只有在n模式下才可以设置40M带宽
*****************************************************************************/
STATIC int32 ATSetWifiBand(WLAN_AT_WIBAND_TYPE band)
{
    int32 ret = AT_RETURN_SUCCESS;
    
    WLAN_TRACE_INFO("%s:enter\n", __FUNCTION__);
    WLAN_TRACE_INFO("WLAN_AT_WIBAND_TYPE:%d\n", band);
    
    switch(band)
    {
        case AT_WIBAND_20M: 
            {
                g_wlan_at_data.wifiBand = AT_WIBAND_20M;
                break;
            }
        case AT_WIBAND_40M:
        {
            if((AT_WIMODE_80211n == g_wlan_at_data.wifiMode ) || (AT_WIMODE_80211ac == g_wlan_at_data.wifiMode ))
            {
                g_wlan_at_data.wifiBand = AT_WIBAND_40M;
            }
            else
            {
                WLAN_TRACE_ERROR("Error wifi mode for bandwidth 40M,must in n or ac mode\n");
                ret = AT_RETURN_FAILURE;
            }
            break;
        }
        case AT_WIBAND_80M:
        {
            if(AT_WIMODE_80211ac == g_wlan_at_data.wifiMode )
            {
                g_wlan_at_data.wifiBand = AT_WIBAND_80M;
            }
            else
            {
                WLAN_TRACE_ERROR("Error wifi mode for bandwidth 80M,must in ac mode\n");
                ret = AT_RETURN_FAILURE;
            }
            break;
        }
        default:
            ret = AT_RETURN_FAILURE;
            break;
    }
     
    return ret;
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiBand(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  :获取当前带宽配置 
            当前带宽，以字符串形式返回eg: 0
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetWifiBand(WLAN_AT_BUFFER_STRU *strBuf)
{
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%d", g_wlan_at_data.wifiBand);
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiBandSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  :获取WiFi支持的带宽配置 
            支持带宽，以字符串形式返回eg: 0,1
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetWifiBandSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%s", WLAN_AT_BAND_SUPPORT);
    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
/*(4)^WIFREQ 设置WiFi频点 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
 功能描述  : 设置WiFi频点
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 ATSetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
{
    /*2.4G频点集合*/
    const uint16   ausChannels[] = {2412,2417,2422,2427,2432,2437,2442,2447,2452,2457,2462,2467,2472,2484};/*2.4G*/

/*BCM4354芯片 应装备归一化要求，采用当前带宽下的第一个20M频点表示当前信道的频点*/
    /*5G 20M频点集合*/
    const uint16 aulChannel036[] = {5180,5200,5220,5240,5260,5280,5300,5320};/*w52和w53*/  
    const uint16 aulChannel100[] = {5500,5520,5540,5560,5580,5600,5620,5640,5660,5680,5700,5720};/*w56*/
    const uint16 aulChannel149[] = {5745,5765,5785,5805,5825};/*w57*/

    /*2.4G 40M频点集合,对应装备信道3至11*/
    const uint16   ausChannels_40M[] = {2422, 2427, 2432, 2437, 2442, 2447, 2452, 2457, 2462};/*2.4G 40M*/
    /*5G 40M频点集合*/
    const uint16 aulChannel036_40M[] = {5180, 5220, 5260, 5300};/*5G 40M*/
    const uint16 aulChannel100_40M[] = {5500, 5540, 5580, 5620, 5660,5700};/*5G 40M*/
    const uint16 aulChannel149_40M[] = {5745, 5785};/*5G 40M*/
    /*5G 80M频点集合*/
    const uint16 aulChannel036_80M[] = {5180, 5260};/*5G 80M*/
    const uint16 aulChannel100_80M[] = {5500, 5580,5660};/*5G 80M*/
    const uint16 aulChannel149_80M[] = {5745};/*5G 80M*/
    /*5G 40M带宽需要加u的信道*/
    const uint16 channel_5g_40M_u[] = {40,48,56,64,104,112,120,128,136,144,153,161};
    /*5G 40M带宽需要加l的信道*/
    const uint16 channel_5g_40M_l[] = {36,44,52,60,100,108,116,124,132,140,149,157};
    char ul_for_40M = '\0';//初始化为0
    uint16 ulWifiFreq = 0;
    uint16 i = 0;
    int32 ret = AT_RETURN_SUCCESS;
    
    if (NULL == pFreq)
    {
        return (AT_RETURN_FAILURE);
    }
    if (g_CT == 0)
    {
        ret = wl_down();
        WL_BEEN_DONE(ret, "wl down");
        ret = wl_mpc(0);
        WL_BEEN_DONE(ret, "wl mpc 0");
        ret = wl_scansuppress(1);
        WL_BEEN_DONE(ret, "wl scansuppress 1");
        ret = wl_country("ALL");
        WL_BEEN_DONE(ret, "wl country ALL");
        if (pFreq->value <= WLAN_FREQ_2G_MAX)
        {
            ret = wl_band(WLC_BAND_2G);
            WL_BEEN_DONE(ret, "wl band b");
        }
        else
        {
            ret = wl_band(WLC_BAND_5G);
            WL_BEEN_DONE(ret, "wl band a");
        }
        ret = wl_txchain(CHAIN_1X1_2);
        WL_BEEN_DONE(ret, "wl txchain 2");
        ret = wl_rxchain(CHAIN_1X1_2);
        WL_BEEN_DONE(ret, "wl rxchain 2");
    }
    /*MIMO 模式*/
    if(4 == g_wlan_at_data.wifiGroup)
    {
        if (pFreq->value <= WLAN_FREQ_2G_MAX)
        {
            ret = wl_band(WLC_BAND_2G);
            WL_BEEN_DONE(ret, "wl band b");
        }
        else
        {
            ret = wl_band(WLC_BAND_5G);
            WL_BEEN_DONE(ret, "wl band a");
        }
    }
    /*20M带宽*/
    if (AT_WIBAND_20M == g_wlan_at_data.wifiBand)
    {
        if (pFreq->value <= WLAN_FREQ_2G_MAX)
        {
            for (i = 0; i < (sizeof(ausChannels) / sizeof(uint16)); i++)
            {
                if (pFreq->value == ausChannels[i])
                {
                    ulWifiFreq = (i + 1);
                    break;
                }
            }
        }
        /*WIFI 5G 信道算法如下，频点对照上面的信道号，具体对应对应表为

        case 36:
            iWIFIchannel = 5180;
            break;
        case 40:
            iWIFIchannel = 5200;
            break;
        case 44:
            iWIFIchannel = 5220;
            break;
        case 48:
            iWIFIchannel = 5240;
            break;
        case 52:
            iWIFIchannel = 5260;
            break;
        case 56:
            iWIFIchannel = 5280;
            break;
        case 60:
            iWIFIchannel = 5300;
            break;
        case 64:
            iWIFIchannel = 5320;
            break;
        */
        else if ((pFreq->value >= WLAN_FREQ_5G_W52_MIN) && (pFreq->value <= WLAN_FREQ_5G_W53_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel036) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel036[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_INTERVAL + WLAN_CHANNEL_5G_MIN);
                    break;
                }
            }
        }
        /*WIFI 5G 信道算法如下，频点对照上面的信道号，具体对应对应表为

        case 100:
            iWIFIchannel = 5500;
            break;
        case 104: 
            iWIFIchannel = 5520;
            break;
        case 108:
            iWIFIchannel = 5540;
            break;
        case 112: 
            iWIFIchannel = 5560;
            break;
        case 116:
            iWIFIchannel = 5580;
            break;
        case 120: 
            iWIFIchannel = 5600;
            break;
        case 124:
            iWIFIchannel = 5620;
            break;
        case 128: 
            iWIFIchannel = 5640;
            break;
        case 132: 
            iWIFIchannel = 5660;
            break;
        case 136:
            iWIFIchannel = 5680;
            break;
        case 140: 
            iWIFIchannel = 5700;
            break;
        */
        else if ((pFreq->value >= WLAN_FREQ_5G_W56_MIN) && (pFreq->value <= WLAN_FREQ_5G_W56_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel100) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel100[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_INTERVAL + HUNDRED);
                    break;
                }
            }

        }
        /*WIFI 5G 信道算法如下，频点对照上面的信道号，具体对应对应表为
        case 149: 
            iWIFIchannel = 5745;
            break;
        case 153:
            iWIFIchannel = 5765;
            break;
        case 157: 
            iWIFIchannel = 5785;
            break;
        case 161: 
            iWIFIchannel = 5805;
            break;
        case 165:
            iWIFIchannel = 5825;
            break;
        */
        else if ((pFreq->value >= WLAN_FREQ_5G_W57_MIN) && (pFreq->value <= WLAN_FREQ_5G_W57_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel149) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel149[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_INTERVAL + WLAN_CHANNEL_5G_W57_START);
                    break;
                }
            }
        }
        else
        {
            WLAN_TRACE_INFO("Error 20M wifiFreq parameters\n");      
            return AT_RETURN_FAILURE;
        }

        WLAN_TRACE_INFO("Target Channel = %d\n", ulWifiFreq);
    
        if (!(((WLAN_CHANNEL_2G_MIN <= ulWifiFreq) && (WLAN_CHANNEL_2G_MAX >= ulWifiFreq))
            || ((WLAN_CHANNEL_5G_MIN <= ulWifiFreq) && (WLAN_CHANNEL_5G_MAX >= ulWifiFreq))))
        {
            WLAN_TRACE_INFO("Target Channel ERROR,ulWifiFreq = %u!\n", ulWifiFreq);
            return AT_RETURN_FAILURE;
        }
        ret = wl_channel(ulWifiFreq);
        WL_BEEN_DONE(ret, "wl channel %d", ulWifiFreq);
        msleep(DALEY_100_TIME*3); /* 延时300ms */
    }
    else if(AT_WIBAND_40M == g_wlan_at_data.wifiBand)
    {
        if (pFreq->value <= WLAN_FREQ_2G_MAX)
        {
            for (i = 0; i < (sizeof(ausChannels_40M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == ausChannels_40M[i])
                {
                    ulWifiFreq = (i + 1);
                    break;
                }
            }
        }       
        else if ((pFreq->value >= WLAN_FREQ_5G_W52_40M_MIN) && (pFreq->value <= WLAN_FREQ_5G_W53_40M_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel036_40M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel036_40M[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_40M_INTERVAL + WLAN_CHANNEL_5G_MIN);
                    break;
                }
            }
        }        
        else if ((pFreq->value >= WLAN_FREQ_5G_W56_40M_MIN) && (pFreq->value <= WLAN_FREQ_5G_W56_40M_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel100_40M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel100_40M[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_40M_INTERVAL + HUNDRED);
                    break;
                }
            }

        }        
        else if ((pFreq->value >= WLAN_FREQ_5G_W57_40M_MIN) && (pFreq->value <= WLAN_FREQ_5G_W57_40M_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel149_40M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel149_40M[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_40M_INTERVAL + WLAN_CHANNEL_5G_W57_START);
                    break;
                }
            }
        }
        else
        {        
            WLAN_TRACE_INFO("Error 40M wifiFreq parameters\n");
            return AT_RETURN_FAILURE;
        }

        WLAN_TRACE_INFO("Target Channel = %d\n", ulWifiFreq);
        if (!(((WLAN_CHANNEL_2G_MIN <= ulWifiFreq) && (WLAN_CHANNEL_2G_MAX >= ulWifiFreq))
            || ((WLAN_CHANNEL_5G_MIN <= ulWifiFreq) && (WLAN_CHANNEL_5G_MAX >= ulWifiFreq))))
        {
            WLAN_TRACE_INFO("Target Channel ERROR!\n");
            return AT_RETURN_FAILURE;
        }

        /*40M信道加U/L处理*/
        if ((ulWifiFreq > 0) 
                && (ulWifiFreq <= WLAN_CHANNEL_2G_MIDDLE))
        {      
            ul_for_40M = 'l';
        }
        else if ((ulWifiFreq > WLAN_CHANNEL_2G_MIDDLE)
                && (ulWifiFreq <= WLAN_CHANNEL_2G_MAX))
        {
            ul_for_40M = 'l';
        }
        else if ((ulWifiFreq >= WLAN_CHANNEL_5G_MIN)
                && (ulWifiFreq <= WLAN_CHANNEL_5G_MAX))
        {            
            for (i = 0;i < (sizeof(channel_5g_40M_l) / sizeof(uint16));i++)
            {
                if(ulWifiFreq == channel_5g_40M_l[i])
                {                 
                    ul_for_40M = 'l';
                    break;
                }                
            }

            if(i == (sizeof(channel_5g_40M_l) / sizeof(uint16)))
            {
                for (i = 0;i < (sizeof(channel_5g_40M_u) / sizeof(uint16));i++)
                {
                    if(ulWifiFreq == channel_5g_40M_u[i])
                    {
                        ul_for_40M = 'u';
                        break;
                    }                 
                } 
            }
            
            if(i == (sizeof(channel_5g_40M_u) / sizeof(uint16)))
            {
                ul_for_40M = '\0';
            }
        }

        if ('\0' != ul_for_40M)
        {
            if(AT_FEATURE_ENABLE == g_wlan_at_data.wifiRX.onoff)
            {
                ret = wl_down();
                WL_BEEN_DONE(ret, "wl down");
            }
            ret = wl_mimo_bw_cap(1);//mimo_bw_cap 为1时表示40M或80M
            WL_BEEN_DONE(ret, "wl mimo_bw_cap 1");
        }
        ret = wl_chanspec(ulWifiFreq, WL_CHANSPEC_BW_40, ul_for_40M, 0, 0);
        WL_BEEN_DONE(ret, "wl chanspec %d%c", ulWifiFreq, ul_for_40M);
        ret = wl_status();
        WL_BEEN_DONE(ret, "wl status");
    }
    else if(AT_WIBAND_80M == g_wlan_at_data.wifiBand)
    {
        if ((pFreq->value >= WLAN_FREQ_5G_W52_80M_MIN) && (pFreq->value <= WLAN_FREQ_5G_W53_80M_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel036_80M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel036_80M[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_80M_INTERVAL + WLAN_CHANNEL_5G_MIN);
                    break;
                }
            }
        }
        else if ((pFreq->value >= WLAN_FREQ_5G_W56_80M_MIN) && (pFreq->value <= WLAN_FREQ_5G_W56_80M_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel100_80M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel100_80M[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_80M_INTERVAL + HUNDRED);
                    break;
                }
            }
        }
        else if ((pFreq->value >= WLAN_FREQ_5G_W57_80M_MIN) && (pFreq->value <= WLAN_FREQ_5G_W57_80M_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel149_80M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel149_80M[i])
                {
                    ulWifiFreq = (i * WLAN_CHANNEL_5G_80M_INTERVAL + WLAN_CHANNEL_5G_W57_START);
                    break;
                }
            }
        }
        else
        {        
            WLAN_TRACE_ERROR("Error 80M wifiFreq parameters %u!\n",pFreq->value);
            return AT_RETURN_FAILURE;
        }
        WLAN_TRACE_INFO("Target Channel = %d\n", ulWifiFreq);
        if (!(WLAN_CHANNEL_5G_MIN <= ulWifiFreq) && (WLAN_CHANNEL_5G_MAX >= ulWifiFreq))
        {
            WLAN_TRACE_INFO("Target Channel ERROR,ulWifiFreq = %u!\n", ulWifiFreq);
            return AT_RETURN_FAILURE;
        }
        if(AT_FEATURE_ENABLE == g_wlan_at_data.wifiRX.onoff)
        {
            ret = wl_down();
            WL_BEEN_DONE(ret, "wl down");
        }
        ret = wl_mimo_bw_cap(1);//mimo_bw_cap 为1时表示40M或80M
        WL_BEEN_DONE(ret, "wl mimo_bw_cap 1");
        ret = wl_chanspec(ulWifiFreq, WL_CHANSPEC_BW_80, 0, 0, 0);
        WL_BEEN_DONE(ret, "wl chanspec %d/80", ulWifiFreq);
        msleep(DALEY_100_TIME * 3); /* 延时300ms */
    }
    /* 保存全局变量里，以备查询 */
    g_wlan_at_data.wifiFreq.value = pFreq->value;
    g_wlan_at_data.wifiFreq.offset = pFreq->offset;
    g_wlan_at_data.wifiChannel = ulWifiFreq;
    return ret;
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
 功能描述  : 获取WiFi频点
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
{   
    if (NULL == pFreq)
    {
        return (AT_RETURN_FAILURE);
    }    
    
    memcpy(pFreq, &(g_wlan_at_data.wifiFreq), sizeof(WLAN_AT_WIFREQ_STRU));
    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
/*(5)^WIDATARATE 设置和查询当前WiFi速率集速率
  WiFi速率，单位为0.01Mb/s，取值范围为0～65535 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiDataRate(uint32 rate)
 功能描述  : 设置WiFi发射速率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 ATSetWifiDataRate(uint32 rate)
{
    int ret = 0;
    uint32  ulWifiRate = rate / HUNDRED;
    uint32  ulNRate = 0;  
    uint32  ulacSS = 1; /* 11ac spatial stream,1--SISO, 2--MIMO*/
    uint32  mimo_flag = 0; /* 0--SISO, 1--MIMO*/

    /*BG模式下直接传rate参数，因此不需要进行转换*/
    //const BRATE_ST wifi_brates_table[] = {{100, "1"}, {200, "2"}, {550, "5.5"}, {1100, "11"}};
    /* WIFI n 和ac 模式 AT^WIDATARATE设置的速率值和WL命令速率值的对应表 */
    const uint32 wifi_20m_nrates_table[] = {650, 1300, 1950, 2600, 3900, 5200, 5850, 6500, \
                                                                1300, 2600, 3900, 5200, 7800, 10400, 11700, 13000};
    const uint32 wifi_40m_nrates_table[] =  {1350, 2700, 4050, 5400, 8100, 10800, 12150, 13500, \
                                                                 2700, 5400, 8100, 10800, 16200, 21600, 24300, 27000};
    const uint32 wifi_20m_acrates_table[] = {650, 1300, 1950, 2600, 3900, 5200, 5850, 6500, 7800,\
                                                                 1300, 2600, 3900, 5200, 7800, 10400, 11700, 13000,15600};
    const uint32 wifi_40m_acrates_table[] = {650, 1300, 1950, 2600, 3900, 5200, 5850, 6500, 16200, 18000,\
                                                                 1300, 2600, 3900, 5200, 7800, 10400, 11700, 13000,32400,36000};
    const uint32 wifi_80m_acrates_table[] = {650, 1300, 1950, 2600, 3900, 5200, 5850, 6500, 35100, 39000,\
                                                                 1300, 2600, 3900, 5200, 7800, 10400, 11700, 13000,70200,78000};

    //#define WIFI_BRATES_TABLE_SIZE (sizeof(wifi_brates_table) / sizeof(BRATE_ST))
    #define WIFI_20M_NRATES_TABLE_SIZE (sizeof(wifi_20m_nrates_table) / sizeof(uint32))
    #define WIFI_40M_NRATES_TABLE_SIZE (sizeof(wifi_40m_nrates_table) / sizeof(uint32))

    #define WIFI_20M_ACRATES_TABLE_SIZE (sizeof(wifi_20m_acrates_table) / sizeof(uint32))
    #define WIFI_40M_ACRATES_TABLE_SIZE (sizeof(wifi_40m_acrates_table) / sizeof(uint32))
    #define WIFI_80M_ACRATES_TABLE_SIZE (sizeof(wifi_80m_acrates_table) / sizeof(uint32))
 
    WLAN_TRACE_INFO("WifiRate = %u wifiGroup = %d\n", ulWifiRate, g_wlan_at_data.wifiGroup);


    if(4 == g_wlan_at_data.wifiGroup)
    {
        mimo_flag = 1;
    }
    else
    {
        mimo_flag = 0;
    }
    
    switch (g_wlan_at_data.wifiMode)
    {
        case AT_WIMODE_CW:
            WLAN_TRACE_INFO("AT_WIMODE_CW\n");
            return (AT_RETURN_FAILURE);
        case AT_WIMODE_80211a:
            ret = wl_rate(rate);
            WL_BEEN_DONE(ret, "wl rate %u", rate);
            break;
        case AT_WIMODE_80211b:
            ret = wl_rate(rate);
            WL_BEEN_DONE(ret, "wl rate %d", rate);
            break;
        case AT_WIMODE_80211g:
            ret = wl_rate(rate);
            WL_BEEN_DONE(ret, "wl rate %u", rate);
            break;
        case AT_WIMODE_80211n:
            if (AT_WIBAND_20M == g_wlan_at_data.wifiBand)
            {
                /* WIFI 20M n模式WL命令的速率值为0~15，共16个 */
                for (ulNRate = (WIFI_20M_NRATES_TABLE_SIZE / 2) * mimo_flag; ulNRate < WIFI_20M_NRATES_TABLE_SIZE; ulNRate++)
                {
                    if (wifi_20m_nrates_table[ulNRate] == rate)
                    {
                        WLAN_TRACE_INFO("20M NRate Index = %u\n", ulNRate);
                        break;
                    }
                }

                if (WIFI_20M_NRATES_TABLE_SIZE == ulNRate)
                {
                    WLAN_TRACE_ERROR("20M NRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }
            }
            else if (AT_WIBAND_40M == g_wlan_at_data.wifiBand)
            {
                for (ulNRate = (WIFI_40M_NRATES_TABLE_SIZE / 2) * mimo_flag; ulNRate < WIFI_40M_NRATES_TABLE_SIZE; ulNRate++)
                {
                    if (wifi_40m_nrates_table[ulNRate] == rate)
                    {
                        WLAN_TRACE_INFO("40M NRate Index = %u\n", ulNRate);
                        break;
                    }
                }

                if (WIFI_40M_NRATES_TABLE_SIZE == ulNRate)
                {
                    WLAN_TRACE_ERROR("40M NRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }
            }
            ret = wl_nrate(ulNRate);
            WL_BEEN_DONE(ret, "wl nrate -m %u", ulNRate);
            break;
        case AT_WIMODE_80211ac:
            if (AT_WIBAND_20M == g_wlan_at_data.wifiBand)
            {
                for (ulNRate = (WIFI_20M_ACRATES_TABLE_SIZE / 2) * mimo_flag; ulNRate < WIFI_20M_ACRATES_TABLE_SIZE; ulNRate++)
                {
                    if (wifi_20m_acrates_table[ulNRate] == rate)
                    {
                        if(ulNRate < (WIFI_20M_ACRATES_TABLE_SIZE/2))
                        {
                            ulacSS = 1; 
                            WLAN_TRACE_INFO("20M ACRate Index = %u\n", ulNRate);
                        }
                        else
                        {
                            ulacSS = 2;
                            ulNRate = ulNRate - WIFI_20M_ACRATES_TABLE_SIZE/2;
                            WLAN_TRACE_INFO("20M ACRate Index = %u\n", ulNRate);
                        }
                        break; 
                    }
                }
                if (WIFI_20M_ACRATES_TABLE_SIZE == ulNRate)
                {
                    WLAN_TRACE_ERROR("20M ACRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }
            }
            else if (AT_WIBAND_40M == g_wlan_at_data.wifiBand)
            {
                for (ulNRate = (WIFI_40M_ACRATES_TABLE_SIZE / 2) * mimo_flag; ulNRate < WIFI_40M_ACRATES_TABLE_SIZE; ulNRate++)
                {
                    if (wifi_40m_acrates_table[ulNRate] == rate)
                    {
                        if(ulNRate < (WIFI_40M_ACRATES_TABLE_SIZE/2))
                        {
                            ulacSS = 1; 
                            WLAN_TRACE_INFO("40M ACRate Index = %u\n", ulNRate);
                        }
                        else
                        {
                            ulacSS = 2;
                            ulNRate = ulNRate - WIFI_40M_ACRATES_TABLE_SIZE/2;
                            WLAN_TRACE_INFO("40M ACRate Index = %u\n", ulNRate);
                        }
                        break; 
                    }
                }
                if (WIFI_40M_ACRATES_TABLE_SIZE == ulNRate)
                {
                    WLAN_TRACE_ERROR("40M ACRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }
            } 
            else if(AT_WIBAND_80M == g_wlan_at_data.wifiBand)
            {
                for (ulNRate = (WIFI_80M_ACRATES_TABLE_SIZE / 2) * mimo_flag; ulNRate < WIFI_80M_ACRATES_TABLE_SIZE; ulNRate++)
                {
                    if (wifi_80m_acrates_table[ulNRate] == rate)
                    {
                        if(ulNRate < (WIFI_80M_ACRATES_TABLE_SIZE/2))
                        {
                            ulacSS = 1; 
                            WLAN_TRACE_INFO("80M ACRate Index = %u\n", ulNRate);
                        }
                        else
                        {
                            ulacSS = 2;
                            ulNRate = ulNRate - WIFI_80M_ACRATES_TABLE_SIZE/2;
                            WLAN_TRACE_INFO("80M ACRate Index = %u\n", ulNRate);
                        } 
                        break;
                    }
                }
                if (WIFI_80M_ACRATES_TABLE_SIZE == ulNRate)
                {
                    WLAN_TRACE_ERROR("80M ACRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }
            }
            ret = wl_5g_rate(ulNRate, ulacSS);
            WL_BEEN_DONE(ret, "wl 5g_rate -v %u -s %u", ulNRate,ulacSS);
            break;
        default:
            return (AT_RETURN_FAILURE);
    }
    /*保存全局变量里，以备查询*/
    g_wlan_at_data.wifiRate = rate;
    return (AT_RETURN_SUCCESS);    
}
/*****************************************************************************
 函数名称  : uint32 WlanATGetWifiDataRate()
 功能描述  : 查询当前WiFi速率设置
 输入参数  : NA
 输出参数  : NA
 返 回 值  : wifi速率
 其他说明  : 
*****************************************************************************/
STATIC uint32 ATGetWifiDataRate(void)
{
    return g_wlan_at_data.wifiRate;
}

//////////////////////////////////////////////////////////////////////////
/*(6)^WIPOW 来设置WiFi发射功率 
   WiFi发射功率，单位为0.01dBm，取值范围为 -32768～32767 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiPOW(int32 power_dBm_percent)
 功能描述  : 设置WiFi发射功率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 ATSetWifiPOW(int32 power_dBm_percent)
{
    int ret = 0;
    int32 lWifiPower = power_dBm_percent / HUNDRED;

    if ((lWifiPower >= WLAN_AT_POWER_MIN) && (lWifiPower <= WLAN_AT_POWER_MAX))
    {

        ret = wl_txpwr1(lWifiPower);
        WL_BEEN_DONE(ret, "wl txpwr1 -d -o %u", lWifiPower);

        /*保存全局变量里，以备查询*/
        g_wlan_at_data.wifiPower = power_dBm_percent;
        return (AT_RETURN_SUCCESS);
    }
    else
    {
        WLAN_TRACE_INFO("Invalid argument\n");
        return (AT_RETURN_FAILURE);
    }    
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiPOW()
 功能描述  : 获取WiFi当前发射功率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetWifiPOW(void)
{
    return g_wlan_at_data.wifiPower;
}

//////////////////////////////////////////////////////////////////////////
/*(7)^WITX 来设置WiFi发射机开关 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiTX(WLAN_AT_FEATURE_TYPE onoff)
 功能描述  : 打开或关闭wifi发射机
 输入参数  : 0 关闭
             1 打开
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 ATSetWifiTX(WLAN_AT_FEATURE_TYPE onoff)
{
    int ret = 0;
    int i = 0;
    struct ether_addr addr = {0};
    memset(&addr, 0, sizeof(struct ether_addr));
    
    if (AT_WIMODE_CW == g_wlan_at_data.wifiMode)
    {
        if (AT_FEATURE_DISABLE == onoff)
        {
            WIFI_TEST_CMD("wl fqacurcy 0");
            ret = wl_fqacurcy(0);
            WL_BEEN_DONE(ret, "wl fqacurcy 0");
        }
        else
        {
            ret = wl_up();
            WL_BEEN_DONE(ret, "wl up");
            ret = wl_band(WLC_BAND_2G);
            WL_BEEN_DONE(ret, "wl band b");

            ret = wl_phy_txpwrctrl(0);
            WL_BEEN_DONE(ret, "wl phy_txpwrctrl 0");
            ret = wl_phy_txpwrindex(127, 0);//博通固定参数，因此127未创建宏
            WL_BEEN_DONE(ret, "wl phy_txpwrindex 127 0");
            ret = wl_phy_txpwrctrl(1);
            WL_BEEN_DONE(ret, "wl phy_txpwrctrl 1");
            ret = wl_out();
            WL_BEEN_DONE(ret, "wl out");
            ret = wl_fqacurcy(g_wlan_at_data.wifiChannel);
            WL_BEEN_DONE(ret, "wl fqacurcy %d", (g_wlan_at_data.wifiChannel));
        }
    }
    else 
    {
        if(AT_FEATURE_DISABLE == onoff)
        {
            ret = wl_pkteng_stop(PKTENG_TX);
            WL_BEEN_DONE(ret, "wl pkteng_stop tx");
            ret = wl_down();
            WL_BEEN_DONE(ret, "wl down");
        }
        else
        {
            /* 调用底软接口 */
            ret = wl_mimo_bw_cap(1);
            WL_BEEN_DONE(ret, "wl mimo_bw_cap 1");
            if(AT_WIBAND_80M == g_wlan_at_data.wifiBand)
            {
                /*只有在11ac 80M时设置为-1，其他均设置成4*/
                ret = wl_mimo_txbw(-1);
                WL_BEEN_DONE(ret, "wl mimo_txbw -1");
            }
            else
            {
                ret = wl_mimo_txbw(4);
                WL_BEEN_DONE(ret, "wl mimo_txbw 4");
            }
            ret = wl_up();
            WL_BEEN_DONE(ret, "wl up");
            msleep(DALEY_100_TIME*4); /* 延时400ms */
            ret = wl_phy_forcecal(1);
            WL_BEEN_DONE(ret, "wl phy_forcecal 1");
            for(i = 0; i < MAC_BYTE_MAX; ++i)
            {
                addr.octet[i] = 0x11 * i;//组装mac地址00:11:22:33:44:55
            }
            ret = wl_pkteng_start(&addr, PKTENG_TX, 100, 1500, 0);//固定封包数量和大小，100是时延时间，1500是固定数据包大小。
            WL_BEEN_DONE(ret, "wl pkteng_start 00:11:22:33:44:55 tx 100 1500 0");
            msleep(DALEY_100_TIME*4); /* 延时400ms */
        } 
    }
    
    /*保存全局变量里，已备查询*/
    g_wlan_at_data.wifiTX = onoff;
     
    return (AT_RETURN_SUCCESS);
}
/*****************************************************************************
 函数名称  : WLAN_AT_FEATURE_TYPE WlanATGetWifiTX()
 功能描述  : 查询当前WiFi发射机状态信息
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 关闭发射机
             1 打开发射机
 其他说明  : 
*****************************************************************************/
STATIC WLAN_AT_FEATURE_TYPE ATGetWifiTX(void)
{
    return g_wlan_at_data.wifiTX;
}
/*****************************************************************************
 函数名称  : static uint8 charToData()
 功能描述  : 字符转整数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 
 其他说明  : 
*****************************************************************************/
static uint8 charToData(const char ch)
{
    switch(ch)
    {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
    case 'A':
        return 10;
    case 'b':
    case 'B':
        return 11;
    case 'c':
    case 'C':
        return 12;
    case 'd':
    case 'D':
        return 13;
    case 'e':
    case 'E':
        return 14;
    case 'f':
    case 'F':
        return 15;
    }
    return 0;
}
/*****************************************************************************
 函数名称  : bool StrToMac()
 功能描述  : 十六进制转十进制
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 
 其他说明  : 
*****************************************************************************/

bool StrToMac(const char * szMac,uint8 * pMac)
{ 
    int i = 0;
    const char * pTemp = szMac; 
    for (i = 0; i < 6; i++) 
    { 
        pMac[i] = charToData(*pTemp++) * 16; 
        pMac[i] += charToData(*pTemp++); 
        pTemp++; 
    } 
    return 0;
}
/***************************************************************************
 函数名称  : wl_ether_atoe()
 功能描述  : 查询当前WiFi发射机状态信息
 输入参数  : const char *a  ,struct ether_addr *n
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 

****************************************************************************/
static void wl_ether_atoe(const char *a, struct ether_addr *n)
{
    char *c = NULL;
    int i = 0;
    int j = 0;
    if (NULL == a || NULL == n || '\0' == *a)
    {
        WLAN_TRACE_ERROR("params error\n");
        return;
    }
    memset(n, 0, ETHER_ADDR_LEN);
    for (i = 0; i < ETHER_ADDR_LEN; i++)
    {
        n->octet[i] = charToData(*a++) * 16;    //十六进制转十进制
        n->octet[i] += charToData(*a++);
        if ('\0' == *a)
        {
            WLAN_TRACE_INFO("The End of MAC address!!\n");
            break;
        }
        a++; //跳过冒号
    }
    return ;
}


//////////////////////////////////////////////////////////////////////////
/*(8)^WIRX 设置WiFi接收机开关 */
//////////////////////////////////////////////////////////////////////////
STATIC int32 ATSetWifiRX(WLAN_AT_WIRX_STRU *params)
{
    int ret = 0;
    struct ether_addr addr = {0};
    if (NULL == params)
    {
        return (AT_RETURN_FAILURE);
    }

    switch (params->onoff)
    {
        case AT_FEATURE_DISABLE:
            ret = wl_pkteng_stop(PKTENG_RX);
            WL_BEEN_DONE(ret, "wl pkteng_stop rx");
            break;
        case AT_FEATURE_ENABLE:
            if (0 == strncmp(params->src_mac,"",MAC_ADDRESS_LEN))
            {
                WLAN_TRACE_ERROR("src mac is NULL\n");
                return (AT_RETURN_FAILURE);
            }
            WLAN_TRACE_INFO("src mac is %s\n",params->src_mac);
            memset(&addr, 0, sizeof(struct ether_addr));
            wl_ether_atoe(params->src_mac, &addr);
            ret = wl_down();
            WL_BEEN_DONE(ret, "wl down");
            ret = wl_phy_watchdog(0);
            WL_BEEN_DONE(ret, "wl phy_watchdog 0");
            ret = wl_PM(0);
            WL_BEEN_DONE(ret, "wl PM 0");
            ret = wl_mimo_bw_cap(1);
            WL_BEEN_DONE(ret, "wl mimo_bw_cap 1");
            if(AT_WIBAND_80M == g_wlan_at_data.wifiBand)
            {
                /*只有在11ac 80M时设置为-1，其他均设置成4*/
                ret = wl_mimo_txbw(-1);
                WL_BEEN_DONE(ret, "wl mimo_txbw -1");
            }
            else
            {
                ret = wl_mimo_txbw(4);
                WL_BEEN_DONE(ret, "wl mimo_txbw 4");
            }
            ret = wl_up();
            WL_BEEN_DONE(ret, "wl up");
            ret = wl_pkteng_start(&addr, PKTENG_RX, 0, 0, 0);
            WL_BEEN_DONE(ret, "wl pkteng_start %s rx", (params->src_mac));
            ret = wl_phy_forcecal(1);
            WL_BEEN_DONE(ret, "wl phy_forcecal 1");
            ret = wl_counters();
            WL_BEEN_DONE(ret, "wl counters"); 
            memcpy(&g_wifi_packet_report, &g_wifi_packet_new_rep, sizeof(g_wifi_packet_report));

            break;
        default:
            return (AT_RETURN_FAILURE);
    }
    
    memcpy(&g_wlan_at_data.wifiRX, params, sizeof(WLAN_AT_WIRX_STRU));
    
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiRX(WLAN_AT_WIRX_STRU *params)
 功能描述  : 获取wifi接收机的状态
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetWifiRX(WLAN_AT_WIRX_STRU *params)
{
    if (NULL == params)
    {
        return (AT_RETURN_FAILURE);
    }    
    
    memcpy(params, &g_wlan_at_data.wifiRX, sizeof(WLAN_AT_WIRX_STRU));
    
    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
/*(9)^WIRPCKG 查询WiFi接收机误包码，上报接收到的包的数量*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiRPCKG(int32 flag)
 功能描述  : 清除Wifi接收统计包为零
 输入参数  : 0 清除wifi统计包
             非0 无效参数
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 ATSetWifiRPCKG(int32 flag)
{
    int ret = 0;
    ASSERT_WiFi_OFF(AT_RETURN_FAILURE);

    if (0 != flag)
    {
        WLAN_TRACE_INFO("Exit on flag = %d\n", flag);
        return (AT_RETURN_FAILURE);
    }
    ret = wl_counters();
    WL_BEEN_DONE(ret, "wl counters");
    memcpy(&g_wifi_packet_report, &g_wifi_packet_new_rep, sizeof(g_wifi_packet_report));
    
    return (AT_RETURN_SUCCESS);
}


/*****************************************************************************
 函数名称  : int32 WlanATSetWifiRPCKG(int32 flag)
 功能描述  : 查询WiFi接收机误包码，上报接收到的包的数量
 输入参数  : WLAN_AT_WIRPCKG_STRU *params
 输出参数  : uint16 good_result; //单板接收到的好包数，取值范围为0~65535
             uint16 bad_result;  //单板接收到的坏包数，取值范围为0~65535
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetWifiRPCKG(WLAN_AT_WIRPCKG_STRU *params)
{
    int32 ret = AT_RETURN_SUCCESS; 
    if (NULL == params)
    {        
        WLAN_TRACE_ERROR("%s:POINTER_NULL!\n", __FUNCTION__);
        ret = AT_RETURN_FAILURE;
        return ret;
    }
    /* 解决BCM 2.4&5G收包异常问题，判断收包标志位为1，则使用wl命令进行温度点切换 */
  
    if(1 == g_receivePackageFlag)
    {
        ret = wl_mpc(0);
        WL_BEEN_DONE(ret, "wl mpc 0");
        if(0 == g_wlan_at_data.wifiGroup || 1 == g_wlan_at_data.wifiGroup)
        {
            ret = wl_band(WLC_BAND_2G);
            WL_BEEN_DONE(ret, "wl band b");
        }
        else if(2 == g_wlan_at_data.wifiGroup || 3 == g_wlan_at_data.wifiGroup)
        {
            ret = wl_band(WLC_BAND_5G);
            WL_BEEN_DONE(ret, "wl band a");
        }
        else
        {
            WLAN_TRACE_ERROR("%s:Not support group.\n", __FUNCTION__);
        }
        
        if(g_receivePackageTime > 1)
        {
             ret = wl_radioreg(0x8c, 0x490, "pll");
            //H8 FW已经设置为最低值0xf90，第二次收包依然异常，此处使跳出的温度范围增大一点，设置为0x490
            WL_BEEN_DONE(ret, "wl radioreg 0x8c 0x490 pll");
        }
        else
        {
             ret = wl_radioreg(0x8c, 0x790, "pll");
            //如果第一次收包失败，则回退到H7的寄存器值0x690，跳过fail点
            WL_BEEN_DONE(ret,"wl radioreg 0x8c 0x790 pll");
        }
    }
   

    /* 判断接收机是否打开 */
    if(AT_FEATURE_DISABLE == g_wlan_at_data.wifiRX.onoff)
    {
        WLAN_TRACE_ERROR("%s:Not Rx Mode.\n", __FUNCTION__);
        ret = AT_RETURN_FAILURE;
        return ret;
    }   

    ret = wl_counters();
    WL_BEEN_DONE(ret, "wl counters");
    WLAN_TRACE_INFO("Enter [old = %d, new = %d]\n", g_wifi_packet_report.UcastRxPkts, g_wifi_packet_new_rep.UcastRxPkts);      

    params->good_result = (uint16)(g_wifi_packet_new_rep.UcastRxPkts - g_wifi_packet_report.UcastRxPkts);
    params->bad_result = 0;   
    
    WLAN_TRACE_INFO("Exit [good = %d, bad = %d]\n", params->good_result, params->bad_result);

    /* 收包小于900则认为其收包异常，标志位置为1,收包异常次数加1， 否则标志位依然为0*/
        if(params->good_result < 900 && 0 != params->good_result)
        {
            g_receivePackageFlag = 1; 
            g_receivePackageTime++;
        }
        else
        {
            g_receivePackageFlag = 0;
            g_receivePackageTime = 0;
        }

    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
/*(11)^WIPLATFORM 查询WiFi方案平台供应商信息 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : WLAN_AT_WIPLATFORM_TYPE WlanATGetWifiPlatform()
 功能描述  : 查询WiFi方案平台供应商信息
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC WLAN_AT_WIPLATFORM_TYPE ATGetWifiPlatform(void)
{
    return (AT_WIPLATFORM_BROADCOM);
}

//////////////////////////////////////////////////////////////////////////
/*(12)^TSELRF 查询设置单板的WiFi射频通路*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetTSELRF(uint32 group)
 功能描述  : 设置天线，非多通路传0
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 ATSetTSELRF(uint32 group)
{
    int ret = 0;    
    if(WLAN_AT_GROUP_MAX < group)
    {
        return AT_RETURN_FAILURE;
    }
    g_wlan_at_data.wifiGroup = group;
    WLAN_TRACE_INFO("[%s]:Enter,group = %u\n", __FUNCTION__, group);
    ret = wl_down();
    WL_BEEN_DONE(ret, "wl down");
    ret = wl_mpc(0);
    WL_BEEN_DONE(ret, "wl mpc 0");
    ret = wl_scansuppress(1);
    WL_BEEN_DONE(ret, "wl scansuppress 1");
    ret = wl_country("ALL");
    WL_BEEN_DONE(ret, "wl country ALL");
    if(group <= 3)
    {
        ret = wl_txchain(group % NUMBER_2 + 1);
        WL_BEEN_DONE(ret, "wl txchain %u", (group % 2 + 1));   /*天线模式*/
        ret = wl_rxchain(group % NUMBER_2 + 1);
        WL_BEEN_DONE(ret, "wl rxchain %u", (group % 2 + 1));   /*天线模式*/
    }
    else
    {
        ;/*group=4时在ATSetWifiMode中设置天线模式，
           其他值在 函数开始已经校验，这里不再处理*/
    }

    if((group == 0) || (group == 1))
    {
        ret = wl_band(WLC_BAND_2G);
        WL_BEEN_DONE(ret, "wl band b");
    }
    else if((group == 2) || (group == 3))
    {
        ret = wl_band(WLC_BAND_5G);
        WL_BEEN_DONE(ret, "wl band a");
    }
    g_CT = 1;
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetTSELRFSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 支持的天线索引序列，以字符串形式返回eg: 0,1,2,3
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetTSELRFSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }
    OSA_SNPRINTF(strBuf->buf, sizeof(strBuf->buf), "%s", WLAN_AT_TSELRF_SUPPORT);
    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
/*(13)^WiPARANGE设置、读取WiFi PA的增益情况*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int wifi_set_pa_mode(int wifiPaMode)
 功能描述  : 设置WiFi PA的增益情况
 输入参数  : 增益模式
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 目前仅只支持NO PA模式
*****************************************************************************/
STATIC int wifi_set_pa_mode(int wifiPaMode)
{
    if (AT_WIFI_MODE_ONLY_PA == wifiPaMode)
    {
        return AT_RETURN_SUCCESS;
    }
    else
    {
        return AT_RETURN_FAILURE;
    }
}

/*****************************************************************************
 函数名称  : int wifi_set_pa_mode(int wifiPaMode)
 功能描述  : 获取支持的WiFi PA的增益情况
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 目前仅只支持NO PA模式
*****************************************************************************/
STATIC int wifi_get_pa_mode(void)
{
    return AT_WIFI_MODE_ONLY_PA;
}
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiParange(WLAN_AT_WiPARANGE_TYPE pa_type)
 功能描述  : 设置WiFi PA的增益情况
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 ATSetWifiParange(WLAN_AT_WiPARANGE_TYPE pa_type)
{
    int32 ret = AT_RETURN_FAILURE;
    switch (pa_type)
    {
        case AT_WiPARANGE_LOW:
            ret = wifi_set_pa_mode(AT_WIFI_MODE_ONLY_NOPA);
            break;
        case AT_WiPARANGE_HIGH:
            ret = wifi_set_pa_mode(AT_WIFI_MODE_ONLY_PA);
            break;
        default:
            break;
    }
    return ret;
}

/*****************************************************************************
 函数名称  : WLAN_AT_WiPARANGE_TYPE WlanATGetWifiParange()
 功能描述  : 读取WiFi PA的增益情况
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC WLAN_AT_WiPARANGE_TYPE ATGetWifiParange(void)
{
    WLAN_AT_WiPARANGE_TYPE lWifiPAMode = AT_WiPARANGE_BUTT;
    switch (wifi_get_pa_mode())
    {
        case AT_WIFI_MODE_ONLY_NOPA:
            lWifiPAMode = AT_WiPARANGE_LOW;
            break;
        case AT_WIFI_MODE_ONLY_PA:
            lWifiPAMode = AT_WiPARANGE_HIGH;
            break;
        default:
            break;
    }
    return lWifiPAMode;
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiParangeSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 支持的pa模式序列，以字符串形式返回eg: l,h
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetWifiParangeSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }
    switch (wifi_get_pa_mode())
    {
        case AT_WIFI_MODE_ONLY_NOPA:
            OSA_SNPRINTF(strBuf->buf, sizeof(strBuf->buf), "%c", AT_WiPARANGE_LOW);
            break;
        case AT_WIFI_MODE_ONLY_PA:
            OSA_SNPRINTF(strBuf->buf, sizeof(strBuf->buf), "%c", AT_WiPARANGE_HIGH);
            break;
        case AT_WIFI_MODE_PA_NOPA:
            OSA_SNPRINTF(strBuf->buf, sizeof(strBuf->buf), "%c, %c"
                                                 , AT_WiPARANGE_LOW ,AT_WiPARANGE_HIGH);
            break;
        default:
            return (AT_RETURN_FAILURE);
    }
    
    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////

WLAN_CHIP_OPS bcm4356_ops = 
{
    .WlanATSetWifiEnable = ATSetWifiEnable,
    .WlanATGetWifiEnable = ATGetWifiEnable,

    .WlanATSetWifiMode   = ATSetWifiMode,
    .WlanATGetWifiMode   = ATGetWifiMode,
    .WlanATGetWifiBandSupport = ATGetWifiModeSupport,

    .WlanATSetWifiBand = ATSetWifiBand,
    .WlanATGetWifiBand = ATGetWifiBand,
    .WlanATGetWifiBandSupport = ATGetWifiBandSupport,

    .WlanATSetWifiFreq = ATSetWifiFreq,
    .WlanATGetWifiFreq = ATGetWifiFreq,

    .WlanATSetWifiDataRate = ATSetWifiDataRate,
    .WlanATGetWifiDataRate = ATGetWifiDataRate,

    .WlanATSetWifiPOW = ATSetWifiPOW,
    .WlanATGetWifiPOW = ATGetWifiPOW,

    .WlanATSetWifiTX = ATSetWifiTX,
    .WlanATGetWifiTX = ATGetWifiTX,

    .WlanATSetWifiRX = ATSetWifiRX,
    .WlanATGetWifiRX = ATGetWifiRX,

    .WlanATSetWifiRPCKG = ATSetWifiRPCKG,
    .WlanATGetWifiRPCKG = ATGetWifiRPCKG,
    .WlanATGetWifiPlatform = ATGetWifiPlatform,
    
    .WlanATSetTSELRF = ATSetTSELRF,
    .WlanATGetTSELRF = NULL,
    .WlanATGetTSELRFSupport = ATGetTSELRFSupport,

    .WlanATSetWifiParange = ATSetWifiParange,
    .WlanATGetWifiParange = ATGetWifiParange,

    .WlanATGetWifiParangeSupport = ATGetWifiParangeSupport,

    .WlanATGetWifiCalTemp = NULL,
    .WlanATSetWifiCalTemp = NULL,
    .WlanATSetWifiCalData = NULL,
    .WlanATGetWifiCalData = NULL,
    .WlanATSetWifiCal = NULL,
    .WlanATGetWifiCal = NULL,
    .WlanATGetWifiCalSupport = NULL,
    .WlanATSetWifiCalFreq = NULL,
    .WlanATGetWifiCalFreq = NULL,
    .WlanATSetWifiCalPOW = NULL,
    .WlanATGetWifiCalPOW = NULL,
    .WlanATSetWifi2GPavars = NULL,
    .WlanATGetWifi2GPavars = NULL,
    .WlanATSetWifi5GPavars = NULL,
    .WlanATGetWifi5GPavars = NULL
};

