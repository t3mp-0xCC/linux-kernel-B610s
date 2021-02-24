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

#define WLAN_AT_SSID_SUPPORT            2                  /*֧�ֵ�SSID����*/
#define WLAN_AT_KEY_SUPPORT             5                  /*֧�ֵķ�����*/
#define WLAN_AT_MODE_SUPPORT            "1,2,3,4,5"        /*֧�ֵ�ģʽ(a/b/g/n/ac)*/
#define WLAN_AT_BAND_SUPPORT            "0,1,2"            /*֧�ֵĴ���(0-20M/1-40M/2-80M/3-160M)*/
#define WLAN_AT_TSELRF_SUPPORT          "0,1,2,3"          /*֧�ֵ�������������*/
#define WLAN_AT_GROUP_MAX               4                  /*֧�ֵ������������*/


/*WIFI���ʵ�������*/
#define WLAN_AT_POWER_MIN               (-15)
#define WLAN_AT_POWER_MAX               (30)

/*WIFI��������ջ�ģʽ*/
#define WLAN_AT_WIFI_TX_MODE            (17)
#define WLAN_AT_WIFI_RX_MODE            (18)

/*WiFi����ģʽ*/
#define AT_WIFI_MODE_ONLY_PA            0x00                /*WIFIֻ֧��PAģʽ*/
#define AT_WIFI_MODE_ONLY_NOPA          0x01                /*WIFIֻ֧��NO PAģʽ*/
#define AT_WIFI_MODE_PA_NOPA            0x02                /*WIFIͬʱ֧��PAģʽ��NO PAģʽ*/

#define RX_PACKET_SIZE                  1000                /*װ��ÿ�η�����*/

#define WLAN_CHANNEL_2G_MIN             1                   /*2.4G�ŵ���Сֵ*/
#define WLAN_CHANNEL_5G_MIN             36                  /*5G�ŵ���Сֵ*/
#define WLAN_CHANNEL_2G_MAX             14                  /*2.4G�ŵ����*/
#define WLAN_CHANNEL_2G_MIDDLE          6
#define WLAN_CHANNEL_5G_MAX             165                 /*5G�ŵ����*/

#define WLAN_CHANNEL_5G_W52_START       36
#define WLAN_CHANNEL_5G_W52_END         48
#define WLAN_CHANNEL_5G_W53_START       52
#define WLAN_CHANNEL_5G_W53_END         64
#define WLAN_CHANNEL_5G_W57_START       149
#define WLAN_CHANNEL_5G_W57_END         161

#define WLAN_CHANNEL_5G_INTERVAL        4                     /*5G�ŵ����*/
#define WLAN_CHANNEL_5G_40M_INTERVAL    8                     /*5G 40M�ŵ����*/
#define WLAN_CHANNEL_5G_80M_INTERVAL    16                     /*5G 80M�ŵ����*/
#define WLAN_FREQ_5G_W52_80M_MIN        5180                  /*W52 80M��СƵ��*/
#define WLAN_FREQ_5G_W53_80M_MAX        5260                  /*W53 80M���Ƶ��*/
#define WLAN_FREQ_5G_W56_80M_MIN        5500                  /*W56 80M��СƵ��*/
#define WLAN_FREQ_5G_W56_80M_MAX        5660                  /*W56 80M���Ƶ��*/
#define WLAN_FREQ_5G_W57_80M_MIN        5745                  /*W57 80M��СƵ��*/
#define WLAN_FREQ_5G_W57_80M_MAX        5745                  /*W57 80M���Ƶ��*/
#define WLAN_FREQ_2G_MAX                2484                  /*2.4G���Ƶ��*/
#define WLAN_FREQ_5G_W52_MIN            5180                  /*W52��СƵ��*/
#define WLAN_FREQ_5G_W53_MAX            5320                  /*W53���Ƶ��*/
#define WLAN_FREQ_5G_W52_40M_MIN        5180                  /*W52 40M��СƵ��*/
#define WLAN_FREQ_5G_W53_40M_MAX        5300                  /*W53 40M���Ƶ��*/
#define WLAN_FREQ_5G_W56_MIN            5500                  /*W56��СƵ��*/
#define WLAN_FREQ_5G_W56_MAX            5720                  /*W56���Ƶ��*/
#define WLAN_FREQ_5G_W56_40M_MIN        5500                  /*W56 40M��СƵ��*/
#define WLAN_FREQ_5G_W56_40M_MAX        5700                  /*W56 40M���Ƶ��*/
#define WLAN_FREQ_5G_W57_MIN            5745                  /*W57��СƵ��*/
#define WLAN_FREQ_5G_W57_MAX            5825                  /*W57���Ƶ��*/
#define WLAN_FREQ_5G_W57_40M_MIN        5745                  /*W57��СƵ��*/
#define WLAN_FREQ_5G_W57_40M_MAX        5785                  /*W57���Ƶ��*/

#define WIFI_CMD_MAX_SIZE               256                   /*cmd�ַ���256����*/
#define WIFI_CMD_8_SIZE                 8                     /*cmd�ַ���8����*/
#define HUNDRED                         100

#define DALEY_100_TIME  100
#define DALEY_500_TIME  500
#define DALEY_1000_TIME 1000
#define DALEY_5000_TIME 5000
#define DALEY_2000_TIME 2000

/*******************************************************************************
    ͨ�ú�
*******************************************************************************/
#define WL_BEEN_DONE(ret, cmd, ...) do {    \
    WLAN_TRACE_INFO("ret:%d run:"cmd"\n",(ret),##__VA_ARGS__);    \
    if (ret) return (ret);    \
    msleep(DALEY_100_TIME);    \
}while(0)

/*��WiFiоƬ�·���������*/
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
/*WiFiоƬʹ�ܶ��Լ��*/
#define ASSERT_WiFi_OFF(ret)                    \
if (AT_WIENABLE_OFF == g_wlan_at_data.wifiStatus) \
{                                               \
    WLAN_TRACE_INFO("Exit on WiFi OFF\n");        \
    return ret;                                 \
}

/*WiFiȫ�ֱ����ṹ�� */
typedef struct tagWlanATGlobal
{
    WLAN_AT_WIENABLE_TYPE   wifiStatus;    /*Ĭ�ϼ��ز���ģʽ*/
    WLAN_AT_WIMODE_TYPE     wifiMode;      /*wifiЭ��ģʽ*/
    WLAN_AT_WIBAND_TYPE     wifiBand;      /*wifiЭ����ʽ*/
    WLAN_AT_WIFREQ_STRU     wifiFreq;      /*wifiƵ����Ϣ*/
    uint32                  wifiChannel;   /*wifi�ŵ���Ϣ*/
    uint32                  wifiRate;      /*wifi��������*/
    int32                   wifiPower;     /*wifi���书��*/
    WLAN_AT_FEATURE_TYPE    wifiTX;        /*wifi�����״̬*/
    WLAN_AT_WIRX_STRU       wifiRX;        /*wifi���ջ�״̬*/
    WLAN_AT_WIRPCKG_STRU    wifiPckg;      /*wifi�����*/
    uint32                  wifiGroup;     /*wifi����ģʽ*/
}WLAN_AT_GLOBAL_ST;



/*��¼��ǰ��WiFiģʽ������Ƶ�ʣ����ʵȲ���*/
STATIC WLAN_AT_GLOBAL_ST g_wlan_at_data = {AT_WIENABLE_TEST, AT_WIMODE_80211n, AT_WIBAND_20M
         , {2412, 0}, 1, 6500, 3175, AT_FEATURE_DISABLE, {AT_FEATURE_DISABLE, {0}, {0}}, AT_WiPARANGE_HIGH};/*����WiFi��Ĭ�ϲ���*/



STATIC int g_CT = 0;      /*ȡֵΪ1��0:0��ʾTSELRF ATδ������CT��λʱ��Ҫ�·�WiFi��ʼ�����wl����*/

STATIC int g_receivePackageFlag = 0; /* �հ�������־λ��0:������1:�쳣 */
STATIC int g_receivePackageTime = 0; /* �հ��쳣�������쳣һ�μ�1 */

extern WLAN_AT_PACK_REP_ST g_wifi_packet_report;
extern WLAN_AT_PACK_REP_ST g_wifi_packet_new_rep;/*ʵʱ��Ϣ*/

/*WiFi��Ч��У��*/
STATIC int32 check_wifi_valid()
{        
    int ret = 0;
    ret = wl_ver();
    WL_BEEN_DONE(ret, "wl ver");
    return ret;
}

#define OPT_FILE_PATH "/system/bin/wifi_brcm/exe/otp_4356.bin"
static unsigned char cis_dump[WLC_IOCTL_MAXLEN];

/*WiFi��Ч��У��*/
STATIC int32 check_write_otp(char *filepath)
{        
    int ret = 0;
    int loop_count = LOOP_COUNT;
    int if_write = 0;
    int cis_dump_len = 0;

    WLAN_TRACE_INFO("check_write_otp enter!\n");
    do
    {
        //�������д3�ξ�δд��ɹ��Ļ�����ô���ش���
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
                ret = wlu_ciswrite(1, filepath);   //1:��-p����;0:����-p����
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
/*(1)^WIENABLE ����WiFiģ��ʹ�� */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 ��������  : int32 WlanATSetWifiEnable(WLAN_AT_WIENABLE_TYPE onoff)
 ��������  : ����wifi �������ģʽ������ģʽ���ر�wifi
 �������  :  0  �ر�
              1  ������ģʽ
              2  �򿪲���ģʽ
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
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
 ��������  : WLAN_AT_WIENABLE_TYPE WlanATGetWifiEnable()
 ��������  : ��ȡ��ǰ��WiFiģ��ʹ��״̬
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0  �ر�
             1  ����ģʽ(����ģʽ)
             2  ����ģʽ(������ģʽ)
 ����˵��  : 
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
/*(2)^WIMODE ����WiFiģʽ���� Ŀǰ��Ϊ��ģʽ����*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 ��������  : int32 WlanATSetWifiMode(WLAN_AT_WIMODE_TYPE mode)
 ��������  : ����WiFi AP֧�ֵ���ʽ
 �������  : 0,  CWģʽ
             1,  802.11a��ʽ
             2,  802.11b��ʽ
             3,  802.11g��ʽ
             4,  802.11n��ʽ
             5,  802.11ac��ʽ
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
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
        /*2.4G �� 5G ѡ�����tselrf AT��ʵ�֣���ܲ����ʱ����wimode����Ƶ�δ��������*/
        /*do nothing��ģʽ���ŵ������ʵ�wl����ȷ��*/
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
        /*MIMO ģʽ*/
        if(4 == g_wlan_at_data.wifiGroup)
        {
            /*����ģʽ*/
            ret = wl_txchain(CHAIN_2X2);
            WL_BEEN_DONE(ret, "wl txchain 3");
            /*����ģʽ*/
            ret = wl_rxchain(CHAIN_2X2);
            WL_BEEN_DONE(ret, "wl rxchain 3");
        }
    }
    else if (AT_WIMODE_80211ac == mode)
    {
         /*MIMO ģʽ*/
        if(4 == g_wlan_at_data.wifiGroup)
        {
            /*����ģʽ*/
            ret = wl_txchain(CHAIN_2X2);
            WL_BEEN_DONE(ret, "wl txchain 3");
            /*����ģʽ*/
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
 ��������  : int32 WlanATGetWifiMode(WLAN_AT_BUFFER_STRU *strBuf)
 ��������  : ��ȡ��ǰWiFi֧�ֵ���ʽ
             ��ǰģʽ�����ַ�����ʽ����eg: 2
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 
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
 ��������  : int32 WlanATGetWifiModeSupport(WLAN_AT_BUFFER_STRU *strBuf)
 ��������  : ��ȡWiFiоƬ֧�ֵ�����Э��ģʽ
             ֧�ֵ�����ģʽ�����ַ�����ʽ����eg: 2,3,4
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
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
/*(3)^WIBAND ����WiFi������� */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 ��������  : int32 WlanATSetWifiBand(WLAN_AT_WIBAND_TYPE width)
 ��������  : ��������wifi����
 �������  : 0 20M
             1 40M
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : ֻ����nģʽ�²ſ�������40M����
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
 ��������  : int32 WlanATGetWifiBand(WLAN_AT_BUFFER_STRU *strBuf)
 ��������  :��ȡ��ǰ�������� 
            ��ǰ�������ַ�����ʽ����eg: 0
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
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
 ��������  : int32 WlanATGetWifiBandSupport(WLAN_AT_BUFFER_STRU *strBuf)
 ��������  :��ȡWiFi֧�ֵĴ������� 
            ֧�ִ������ַ�����ʽ����eg: 0,1
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
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
/*(4)^WIFREQ ����WiFiƵ�� */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 ��������  : int32 WlanATSetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
 ��������  : ����WiFiƵ��
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
*****************************************************************************/
STATIC int32 ATSetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
{
    /*2.4GƵ�㼯��*/
    const uint16   ausChannels[] = {2412,2417,2422,2427,2432,2437,2442,2447,2452,2457,2462,2467,2472,2484};/*2.4G*/

/*BCM4354оƬ Ӧװ����һ��Ҫ�󣬲��õ�ǰ�����µĵ�һ��20MƵ���ʾ��ǰ�ŵ���Ƶ��*/
    /*5G 20MƵ�㼯��*/
    const uint16 aulChannel036[] = {5180,5200,5220,5240,5260,5280,5300,5320};/*w52��w53*/  
    const uint16 aulChannel100[] = {5500,5520,5540,5560,5580,5600,5620,5640,5660,5680,5700,5720};/*w56*/
    const uint16 aulChannel149[] = {5745,5765,5785,5805,5825};/*w57*/

    /*2.4G 40MƵ�㼯��,��Ӧװ���ŵ�3��11*/
    const uint16   ausChannels_40M[] = {2422, 2427, 2432, 2437, 2442, 2447, 2452, 2457, 2462};/*2.4G 40M*/
    /*5G 40MƵ�㼯��*/
    const uint16 aulChannel036_40M[] = {5180, 5220, 5260, 5300};/*5G 40M*/
    const uint16 aulChannel100_40M[] = {5500, 5540, 5580, 5620, 5660,5700};/*5G 40M*/
    const uint16 aulChannel149_40M[] = {5745, 5785};/*5G 40M*/
    /*5G 80MƵ�㼯��*/
    const uint16 aulChannel036_80M[] = {5180, 5260};/*5G 80M*/
    const uint16 aulChannel100_80M[] = {5500, 5580,5660};/*5G 80M*/
    const uint16 aulChannel149_80M[] = {5745};/*5G 80M*/
    /*5G 40M������Ҫ��u���ŵ�*/
    const uint16 channel_5g_40M_u[] = {40,48,56,64,104,112,120,128,136,144,153,161};
    /*5G 40M������Ҫ��l���ŵ�*/
    const uint16 channel_5g_40M_l[] = {36,44,52,60,100,108,116,124,132,140,149,157};
    char ul_for_40M = '\0';//��ʼ��Ϊ0
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
    /*MIMO ģʽ*/
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
    /*20M����*/
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
        /*WIFI 5G �ŵ��㷨���£�Ƶ�����������ŵ��ţ������Ӧ��Ӧ��Ϊ

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
        /*WIFI 5G �ŵ��㷨���£�Ƶ�����������ŵ��ţ������Ӧ��Ӧ��Ϊ

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
        /*WIFI 5G �ŵ��㷨���£�Ƶ�����������ŵ��ţ������Ӧ��Ӧ��Ϊ
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
        msleep(DALEY_100_TIME*3); /* ��ʱ300ms */
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

        /*40M�ŵ���U/L����*/
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
            ret = wl_mimo_bw_cap(1);//mimo_bw_cap Ϊ1ʱ��ʾ40M��80M
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
        ret = wl_mimo_bw_cap(1);//mimo_bw_cap Ϊ1ʱ��ʾ40M��80M
        WL_BEEN_DONE(ret, "wl mimo_bw_cap 1");
        ret = wl_chanspec(ulWifiFreq, WL_CHANSPEC_BW_80, 0, 0, 0);
        WL_BEEN_DONE(ret, "wl chanspec %d/80", ulWifiFreq);
        msleep(DALEY_100_TIME * 3); /* ��ʱ300ms */
    }
    /* ����ȫ�ֱ�����Ա���ѯ */
    g_wlan_at_data.wifiFreq.value = pFreq->value;
    g_wlan_at_data.wifiFreq.offset = pFreq->offset;
    g_wlan_at_data.wifiChannel = ulWifiFreq;
    return ret;
}

/*****************************************************************************
 ��������  : int32 WlanATGetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
 ��������  : ��ȡWiFiƵ��
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
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
/*(5)^WIDATARATE ���úͲ�ѯ��ǰWiFi���ʼ�����
  WiFi���ʣ���λΪ0.01Mb/s��ȡֵ��ΧΪ0��65535 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 ��������  : int32 WlanATSetWifiDataRate(uint32 rate)
 ��������  : ����WiFi��������
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
*****************************************************************************/
STATIC int32 ATSetWifiDataRate(uint32 rate)
{
    int ret = 0;
    uint32  ulWifiRate = rate / HUNDRED;
    uint32  ulNRate = 0;  
    uint32  ulacSS = 1; /* 11ac spatial stream,1--SISO, 2--MIMO*/
    uint32  mimo_flag = 0; /* 0--SISO, 1--MIMO*/

    /*BGģʽ��ֱ�Ӵ�rate��������˲���Ҫ����ת��*/
    //const BRATE_ST wifi_brates_table[] = {{100, "1"}, {200, "2"}, {550, "5.5"}, {1100, "11"}};
    /* WIFI n ��ac ģʽ AT^WIDATARATE���õ�����ֵ��WL��������ֵ�Ķ�Ӧ�� */
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
                /* WIFI 20M nģʽWL���������ֵΪ0~15����16�� */
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
    /*����ȫ�ֱ�����Ա���ѯ*/
    g_wlan_at_data.wifiRate = rate;
    return (AT_RETURN_SUCCESS);    
}
/*****************************************************************************
 ��������  : uint32 WlanATGetWifiDataRate()
 ��������  : ��ѯ��ǰWiFi��������
 �������  : NA
 �������  : NA
 �� �� ֵ  : wifi����
 ����˵��  : 
*****************************************************************************/
STATIC uint32 ATGetWifiDataRate(void)
{
    return g_wlan_at_data.wifiRate;
}

//////////////////////////////////////////////////////////////////////////
/*(6)^WIPOW ������WiFi���书�� 
   WiFi���书�ʣ���λΪ0.01dBm��ȡֵ��ΧΪ -32768��32767 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 ��������  : int32 WlanATSetWifiPOW(int32 power_dBm_percent)
 ��������  : ����WiFi���书��
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
*****************************************************************************/
STATIC int32 ATSetWifiPOW(int32 power_dBm_percent)
{
    int ret = 0;
    int32 lWifiPower = power_dBm_percent / HUNDRED;

    if ((lWifiPower >= WLAN_AT_POWER_MIN) && (lWifiPower <= WLAN_AT_POWER_MAX))
    {

        ret = wl_txpwr1(lWifiPower);
        WL_BEEN_DONE(ret, "wl txpwr1 -d -o %u", lWifiPower);

        /*����ȫ�ֱ�����Ա���ѯ*/
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
 ��������  : int32 WlanATGetWifiPOW()
 ��������  : ��ȡWiFi��ǰ���书��
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 
*****************************************************************************/
STATIC int32 ATGetWifiPOW(void)
{
    return g_wlan_at_data.wifiPower;
}

//////////////////////////////////////////////////////////////////////////
/*(7)^WITX ������WiFi��������� */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 ��������  : int32 WlanATSetWifiTX(WLAN_AT_FEATURE_TYPE onoff)
 ��������  : �򿪻�ر�wifi�����
 �������  : 0 �ر�
             1 ��
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
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
            ret = wl_phy_txpwrindex(127, 0);//��ͨ�̶����������127δ������
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
            /* ���õ���ӿ� */
            ret = wl_mimo_bw_cap(1);
            WL_BEEN_DONE(ret, "wl mimo_bw_cap 1");
            if(AT_WIBAND_80M == g_wlan_at_data.wifiBand)
            {
                /*ֻ����11ac 80Mʱ����Ϊ-1�����������ó�4*/
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
            msleep(DALEY_100_TIME*4); /* ��ʱ400ms */
            ret = wl_phy_forcecal(1);
            WL_BEEN_DONE(ret, "wl phy_forcecal 1");
            for(i = 0; i < MAC_BYTE_MAX; ++i)
            {
                addr.octet[i] = 0x11 * i;//��װmac��ַ00:11:22:33:44:55
            }
            ret = wl_pkteng_start(&addr, PKTENG_TX, 100, 1500, 0);//�̶���������ʹ�С��100��ʱ��ʱ�䣬1500�ǹ̶����ݰ���С��
            WL_BEEN_DONE(ret, "wl pkteng_start 00:11:22:33:44:55 tx 100 1500 0");
            msleep(DALEY_100_TIME*4); /* ��ʱ400ms */
        } 
    }
    
    /*����ȫ�ֱ�����ѱ���ѯ*/
    g_wlan_at_data.wifiTX = onoff;
     
    return (AT_RETURN_SUCCESS);
}
/*****************************************************************************
 ��������  : WLAN_AT_FEATURE_TYPE WlanATGetWifiTX()
 ��������  : ��ѯ��ǰWiFi�����״̬��Ϣ
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �رշ����
             1 �򿪷����
 ����˵��  : 
*****************************************************************************/
STATIC WLAN_AT_FEATURE_TYPE ATGetWifiTX(void)
{
    return g_wlan_at_data.wifiTX;
}
/*****************************************************************************
 ��������  : static uint8 charToData()
 ��������  : �ַ�ת����
 �������  : NA
 �������  : NA
 �� �� ֵ  : 
 ����˵��  : 
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
 ��������  : bool StrToMac()
 ��������  : ʮ������תʮ����
 �������  : NA
 �������  : NA
 �� �� ֵ  : 
 ����˵��  : 
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
 ��������  : wl_ether_atoe()
 ��������  : ��ѯ��ǰWiFi�����״̬��Ϣ
 �������  : const char *a  ,struct ether_addr *n
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 

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
        n->octet[i] = charToData(*a++) * 16;    //ʮ������תʮ����
        n->octet[i] += charToData(*a++);
        if ('\0' == *a)
        {
            WLAN_TRACE_INFO("The End of MAC address!!\n");
            break;
        }
        a++; //����ð��
    }
    return ;
}


//////////////////////////////////////////////////////////////////////////
/*(8)^WIRX ����WiFi���ջ����� */
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
                /*ֻ����11ac 80Mʱ����Ϊ-1�����������ó�4*/
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
 ��������  : int32 WlanATGetWifiRX(WLAN_AT_WIRX_STRU *params)
 ��������  : ��ȡwifi���ջ���״̬
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 
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
/*(9)^WIRPCKG ��ѯWiFi���ջ�����룬�ϱ����յ��İ�������*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 ��������  : int32 WlanATSetWifiRPCKG(int32 flag)
 ��������  : ���Wifi����ͳ�ư�Ϊ��
 �������  : 0 ���wifiͳ�ư�
             ��0 ��Ч����
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
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
 ��������  : int32 WlanATSetWifiRPCKG(int32 flag)
 ��������  : ��ѯWiFi���ջ�����룬�ϱ����յ��İ�������
 �������  : WLAN_AT_WIRPCKG_STRU *params
 �������  : uint16 good_result; //������յ��ĺð�����ȡֵ��ΧΪ0~65535
             uint16 bad_result;  //������յ��Ļ�������ȡֵ��ΧΪ0~65535
 �� �� ֵ  : NA
 ����˵��  : 
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
    /* ���BCM 2.4&5G�հ��쳣���⣬�ж��հ���־λΪ1����ʹ��wl��������¶ȵ��л� */
  
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
            //H8 FW�Ѿ�����Ϊ���ֵ0xf90���ڶ����հ���Ȼ�쳣���˴�ʹ�������¶ȷ�Χ����һ�㣬����Ϊ0x490
            WL_BEEN_DONE(ret, "wl radioreg 0x8c 0x490 pll");
        }
        else
        {
             ret = wl_radioreg(0x8c, 0x790, "pll");
            //�����һ���հ�ʧ�ܣ�����˵�H7�ļĴ���ֵ0x690������fail��
            WL_BEEN_DONE(ret,"wl radioreg 0x8c 0x790 pll");
        }
    }
   

    /* �жϽ��ջ��Ƿ�� */
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

    /* �հ�С��900����Ϊ���հ��쳣����־λ��Ϊ1,�հ��쳣������1�� �����־λ��ȻΪ0*/
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
/*(11)^WIPLATFORM ��ѯWiFi����ƽ̨��Ӧ����Ϣ */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 ��������  : WLAN_AT_WIPLATFORM_TYPE WlanATGetWifiPlatform()
 ��������  : ��ѯWiFi����ƽ̨��Ӧ����Ϣ
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 
*****************************************************************************/
STATIC WLAN_AT_WIPLATFORM_TYPE ATGetWifiPlatform(void)
{
    return (AT_WIPLATFORM_BROADCOM);
}

//////////////////////////////////////////////////////////////////////////
/*(12)^TSELRF ��ѯ���õ����WiFi��Ƶͨ·*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 ��������  : int32 WlanATSetTSELRF(uint32 group)
 ��������  : �������ߣ��Ƕ�ͨ·��0
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 
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
        WL_BEEN_DONE(ret, "wl txchain %u", (group % 2 + 1));   /*����ģʽ*/
        ret = wl_rxchain(group % NUMBER_2 + 1);
        WL_BEEN_DONE(ret, "wl rxchain %u", (group % 2 + 1));   /*����ģʽ*/
    }
    else
    {
        ;/*group=4ʱ��ATSetWifiMode����������ģʽ��
           ����ֵ�� ������ʼ�Ѿ�У�飬���ﲻ�ٴ���*/
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
 ��������  : int32 WlanATGetTSELRFSupport(WLAN_AT_BUFFER_STRU *strBuf)
 ��������  : ֧�ֵ������������У����ַ�����ʽ����eg: 0,1,2,3
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 
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
/*(13)^WiPARANGE���á���ȡWiFi PA���������*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 ��������  : int wifi_set_pa_mode(int wifiPaMode)
 ��������  : ����WiFi PA���������
 �������  : ����ģʽ
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : Ŀǰ��ֻ֧��NO PAģʽ
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
 ��������  : int wifi_set_pa_mode(int wifiPaMode)
 ��������  : ��ȡ֧�ֵ�WiFi PA���������
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : Ŀǰ��ֻ֧��NO PAģʽ
*****************************************************************************/
STATIC int wifi_get_pa_mode(void)
{
    return AT_WIFI_MODE_ONLY_PA;
}
/*****************************************************************************
 ��������  : int32 WlanATSetWifiParange(WLAN_AT_WiPARANGE_TYPE pa_type)
 ��������  : ����WiFi PA���������
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 
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
 ��������  : WLAN_AT_WiPARANGE_TYPE WlanATGetWifiParange()
 ��������  : ��ȡWiFi PA���������
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 
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
 ��������  : int32 WlanATGetWifiParangeSupport(WLAN_AT_BUFFER_STRU *strBuf)
 ��������  : ֧�ֵ�paģʽ���У����ַ�����ʽ����eg: l,h
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 
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

