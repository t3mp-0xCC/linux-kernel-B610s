
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include <product_nv_id.h>
#include <bsp_nvim.h>
#include <product_nv_def.h>
#include <linux/slab.h>

#include "wlan_at.h"
#include "wlan_at_api.h"
#include "wlan_utils.h"

#define WL_CONFIG_EXE_PATH    "/system/bin/bcm4352/exe/wl " 
#define WIFI_INSMOD_SH        "/system/bin/bcm4352/exe/wifi_poweron_4352_AT.sh"

/*2.4g adapter interface*/
#define IF_2G   "-i wl0"

#define RF_2G_0   (0)
#define RF_2G_1   (1)

/*MIMO*/
#define RF_2G_MIMO   (4)


#define WL_CMD_BUF_LEN          (128)
#define WL_COUNTER_BUF_LEN      (2048)

#define WLAN_AT_2G_BAND         (0)

#define WL_DEFAULT_POWER        (600) /* Ĭ�Ϲ��� */

#define WLAN_AT_MODE_BCM43217_SUPPORT    "2,3,4"             /*֧�ֵ�ģʽ(b/g/n)*/    
             
#define WLAN_AT_BCM43217_BAND_SUPPORT    "0,1"               /*֧�ֵĴ���(20M/40M)*/

#define WLAN_AT_TSELRF_SUPPORT          "0,1"                /*֧�ֵ�������������*/

#define WLAN_AT_GROUP_MAX               (1)                  /*֧�ֵ������������*/
#define WLAN_AT_TYPE_MAX                (2)                  /*֧�ֻ�ȡ�������Ϣ����*/

/*WIFI���ʵ�������*/
#define WLAN_AT_POWER_MIN               (-15)
#define WLAN_AT_POWER_MAX               (30)

/*WiFi����ģʽ*/
#define AT_WIFI_MODE_ONLY_PA            (0x00)              /*WIFIֻ֧��PAģʽ*/
#define AT_WIFI_MODE_ONLY_NOPA          (0x01)              /*WIFIֻ֧��NO PAģʽ*/
#define AT_WIFI_MODE_PA_NOPA            (0x02)              /*WIFIͬʱ֧��PAģʽ��NO PAģʽ*/

#define WLAN_CHANNEL_2G_MIN             (1)                 /*2.4G�ŵ���Сֵ*/
#define WLAN_CHANNEL_2G_MAX             (14)
#define WLAN_CHANNEL_2G_MIDDLE          (6)

#define WLAN_FREQ_2G_MAX                (2484)              /*2.4G���Ƶ��*/

#define WIFI_CMD_MAX_SIZE               (256)                 /*cmd�ַ���256����*/
#define WIFI_CMD_8_SIZE                 (8)                   /*cmd�ַ���8����*/
#define HUNDRED                         (100)

#define DALEY_50_TIME                   (50)
#define DALEY_100_TIME                  (100)
#define DELAY_150_TIME                  (150)
#define DALEY_500_TIME                  (500)
#define DALEY_1000_TIME                 (1000)
#define DALEY_5000_TIME                 (5000)

#define AT_WIFI_N_RATE_NUM              (16)

#define RMMOD_TIME                      (20000)
#define INSMOD_TIME                     (180000)

#define VOS_OK                          (0)
#define VOS_ERROR                       (-1)
#define VOS_TRUE                        (1L)
#define VOS_FALSE                       (0L)
#define STR_LEN_MAX                     (128)

#define ANT_2G_NUM                      (2)
#define ANT_2G_CAL_NV_VALUE_BYTE_NUM    (6)

char CMDValueStr[STR_LEN_MAX] = {0};
int  CMDRet                   = VOS_ERROR;

#define ANT_2G_NUM (2)           /* �����WiFi 2G ��Ƶ��������   */
#define ANT_2G_OLD_GRP_NUM (6)   /* nvm �ļ���WiFi 2G ��Ƶ��������*/
#define ANT_2G_CAL_NV_VALUE_BYTE_NUM (6)

#define NVM_2G_NEW_STR_LENTH     (8)
#define NVM_2G_OLD_STR_LENTH     (9)
/******************************************************************************
��������:   g_ulWifiRF
��������:   ����Wifi��Ƶ���߲���
ȡֵ��Χ:   
���ú���:

ע������:   �ñ���Ϊ�ڲ�ҵ����Чʹ�õı��������ṩ���ⲿ�ӿ�ʹ��
******************************************************************************/ 
static unsigned int g_ulWifiRF = 0xffffffff;

/******************************************************************************
��������:   g_ulWifiMCSIndex
��������:   ����wifi��ǰ���ʶ�Ӧ��MCS index
ȡֵ��Χ:   
���ú���:

ע������:   �ñ���Ϊ�ڲ�ҵ����Чʹ�õı��������ṩ���ⲿ�ӿ�ʹ��
******************************************************************************/ 
static unsigned int g_ulWifiMCSIndex;

static unsigned int g_ul2GUP = 0;

STATIC void Check_WIFI_CAL_NV(void);

WLAN_RETURN_TYPE prepare_wifi_driver_loaded(void);

/*��WiFiоƬ�·���������*/
#define WIFI_TEST_CMD(cmd) do{ \
    if (WLAN_SUCCESS != wlan_run_cmd(cmd)) \
    { \
        PLAT_WLAN_INFO("Run CMD Error!!\n"); \
        if(WLAN_DRV_NO_LOADED == prepare_wifi_driver_loaded()) \
        { \
            if (WLAN_SUCCESS != wlan_run_cmd(cmd)) \
            { \
                return AT_RETURN_FAILURE; \
            } \
        } \
        else \
        { \
            return AT_RETURN_FAILURE; \
        } \
    } \
}while (0)

#define WIFI_SHELL_CMD(cmd) do { \
            if (WLAN_SUCCESS != wlan_run_shell(cmd)) \
            { \
                PLAT_WLAN_INFO("Run shellcmd Error!!!"); \
                if(WLAN_DRV_NO_LOADED == prepare_wifi_driver_loaded()) \
                { \
                    if (WLAN_SUCCESS != wlan_run_shell(cmd)) \
                    { \
                        return AT_RETURN_FAILURE; \
                    } \
                } \
                else \
                { \
                    return AT_RETURN_FAILURE; \
                } \
            } \
        }while (0)

#define OSA_SNPRINTF(str, sz, fmt, ...) \
    {                                            \
        (void)snprintf(str, sz, fmt, ##__VA_ARGS__); \
        str[sz - 1] = '\0';                     \
    }

#ifndef STATIC
    #define STATIC static
#endif

/*��ָ����Լ��*/
#define ASSERT_NULL_POINTER(p, ret)  do { \
    if (NULL == (p)) \
    { \
        PLAT_WLAN_ERR("Exit ret = %d, on NULL Pointer %s", (int)ret, #p); \
        return ret; \
    } \
} while(0)


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
    WLAN_AT_WIBAND_TYPE     wifiBandwith;  /*wifiЭ����ʽ*/
    WLAN_AT_WIFREQ_TYPE     wifiBand;      /*wifi��ǰƵ��*/
    WLAN_AT_WIFREQ_STRU     wifiFreq;      /*wifiƵ����Ϣ*/
    uint32                  wifiRate;      /*wifi��������*/
    int32                   wifiPower;     /*wifi���书��*/
    WLAN_AT_FEATURE_TYPE    wifiTX;        /*wifi�����״̬*/
    WLAN_AT_WIRX_STRU       wifiRX;        /*wifi���ջ�״̬*/
    WLAN_AT_WIRPCKG_STRU    wifiPckg;      /*wifi�����*/  
    uint32                  wifiGroup;     /*wifi����ģʽ*/
} WLAN_AT_GLOBAL_ST;

typedef struct tagWlanATPacketREP
{
    unsigned int TotalRxPkts;
    unsigned int BadRxPkts;
    unsigned int UcastRxPkts;
    unsigned int McastRxPkts;
} WLAN_AT_PACK_REP_ST;

typedef struct
{
    uint32 brate;
    int8 rate_str[WIFI_CMD_8_SIZE];
} BRATE_ST;

/*��¼��ǰ��WiFiģʽ������Ƶ�ʣ����ʵȲ���*/
STATIC WLAN_AT_GLOBAL_ST g_wlan_at_data = {AT_WIENABLE_TEST, AT_WIMODE_80211n, AT_WIBAND_20M, 0
         , {2412, 0}, 6500, 3175, AT_FEATURE_DISABLE, {AT_FEATURE_DISABLE, {0}, {0}}, {0, 0}, 0};/*����WiFi��Ĭ�ϲ���*/

STATIC unsigned int g_ulUcastWifiRxPkts;
STATIC unsigned int g_ulMcastWifiRxPkts;
STATIC unsigned int g_ulRxState = 0;

/*WiFi���ݰ�ͳ����Ϣ*/
STATIC WLAN_AT_PACK_REP_ST g_wifi_packet_report = {0};
STATIC WLAN_AT_PACK_REP_ST g_wifi_packet_new_rep = {0};/*ʵʱ��Ϣ*/

/*****************************************************************************
 ��������  : check_wifi_valid
 ��������  : �鿴wifiͨ·�Ƿ�ok
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
*****************************************************************************/
int32 check_wifi_valid()
{        
    WIFI_TEST_CMD(WL_CONFIG_EXE_PATH" ver");
    return AT_RETURN_SUCCESS;
}

/*****************************************************************************
 ��������  : check_wifi_driver_loaded
 ��������  : �鿴wifi�����Ƿ��Ѿ�����
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
*****************************************************************************/
WLAN_RETURN_TYPE prepare_wifi_driver_loaded(void)
{        
    int8 acCmdValueStr[WIFI_CMD_MAX_SIZE] = {0};
        
    memset(acCmdValueStr, 0, sizeof(acCmdValueStr));

    OSA_SNPRINTF(acCmdValueStr,sizeof(acCmdValueStr), WL_CONFIG_EXE_PATH" ver");
    if (WLAN_SUCCESS != wlan_run_cmd(acCmdValueStr)) 
    { 
        PLAT_WLAN_INFO("Wi-Fi driver is not loaded, now start to loaded Wi-Fi driver!\n"); 

        memset(acCmdValueStr, 0, sizeof(acCmdValueStr));
        
        OSA_SNPRINTF(acCmdValueStr, sizeof(acCmdValueStr), "/bin/sh %s",WIFI_INSMOD_SH);
        (void)wlan_run_cmd(acCmdValueStr);
        
        /*��鲢�����µ�wifiУ׼����*/
        Check_WIFI_CAL_NV();
        
        return WLAN_DRV_NO_LOADED;
    }

    return WLAN_SUCCESS;
}


STATIC unsigned int GetCmdRetValue(char * pValueStr)
{
    char *pcRecPcktsStart                     = NULL;
    char acMonitorStrValue[WIFI_CMD_MAX_SIZE] = {0};
    unsigned int uiRetPcktsNumBuf             = 0;
    unsigned int uiIdPcktsCntValue            = 0;
    char filebuff[WL_COUNTER_BUF_LEN + 1]     = {0};

    /* ��μ�� */
    ASSERT_NULL_POINTER(pValueStr, AT_RETURN_FAILURE);

    if (0 == wlan_read_file("/var/wifirxpkt", filebuff, sizeof(filebuff)))
    {
        PLAT_WLAN_ERR("read_from_file fail");
        return AT_RETURN_FAILURE;
    }

    pcRecPcktsStart = strstr(filebuff, pValueStr);

    if (NULL == pcRecPcktsStart)
    {
        return AT_RETURN_FAILURE;
    }    

    pcRecPcktsStart = pcRecPcktsStart + strlen(pValueStr)+ 1; /* 1:�ո�ռλ */
    
    while (' ' != *pcRecPcktsStart)
    {
        acMonitorStrValue[uiIdPcktsCntValue++] = *pcRecPcktsStart;
        pcRecPcktsStart++;
    }
    uiRetPcktsNumBuf = wlan_strtoi(acMonitorStrValue, NULL, WLAN_BASE10);
    return (uiRetPcktsNumBuf);
}


STATIC void wifi_get_rx_packet_report(unsigned int *ucastPkts, unsigned int *mcastPkts)
{

    char acCmdValueStr[WIFI_CMD_MAX_SIZE] = {0};
    
    if (NULL == ucastPkts || NULL == mcastPkts)
    {
        return;
    }

     if (WLAN_AT_2G_BAND == g_wlan_at_data.wifiBand)
    {
        OSA_SNPRINTF(acCmdValueStr,sizeof(acCmdValueStr), WL_CONFIG_EXE_PATH" %s counters > /var/wifirxpkt", IF_2G);
    }
    else
    {
        return;
    }   
 
    WIFI_SHELL_CMD(acCmdValueStr);

    *mcastPkts = GetCmdRetValue("pktengrxdmcast");
    *ucastPkts = GetCmdRetValue("pktengrxducast");
    PLAT_WLAN_INFO("*mcastPkts=%d, *ucastPkts=%d \n", *mcastPkts, *ucastPkts);
}

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
    WLAN_AT_RETURN_TYPE ret = AT_RETURN_SUCCESS;
    char wl_cmd[WL_CMD_BUF_LEN] = {0};

    ret = check_wifi_valid();
    if (AT_RETURN_SUCCESS != ret)
    {       
        PLAT_WLAN_INFO("check_wifi_valid failed!\n");
        return ret;
    }

    if (onoff == g_wlan_at_data.wifiStatus)
    {
        return AT_RETURN_SUCCESS;
    }
    
    switch (onoff)
    {
        case AT_WIENABLE_OFF:
            {                
                PLAT_WLAN_INFO("Set wifi to off mode\n");

                memset(wl_cmd, 0, WL_CMD_BUF_LEN);
                OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s down", IF_2G);
                WIFI_TEST_CMD(wl_cmd);

                g_wlan_at_data.wifiStatus = AT_WIENABLE_OFF;
            }            
            break;
        case AT_WIENABLE_ON:
            {
                PLAT_WLAN_INFO("Set wifi to on mode\n");

                memset(wl_cmd, 0, WL_CMD_BUF_LEN);
                OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s down", IF_2G);
                WIFI_TEST_CMD(wl_cmd);

                memset(wl_cmd, 0, WL_CMD_BUF_LEN);
                OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s up", IF_2G);
                WIFI_TEST_CMD(wl_cmd);
              
                g_wlan_at_data.wifiStatus = AT_WIENABLE_ON;
            }
            break;
        case AT_WIENABLE_TEST:
            {                
                PLAT_WLAN_INFO("Set wifi to test mode\n");    
                
                memset(wl_cmd, 0, WL_CMD_BUF_LEN);
                OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s down", IF_2G);
                WIFI_TEST_CMD(wl_cmd);

                memset(wl_cmd, 0, WL_CMD_BUF_LEN);
                OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s up", IF_2G);
                WIFI_TEST_CMD(wl_cmd);
                
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
    
    PLAT_WLAN_INFO("%s in\n",__FUNCTION__);
    
    ret = check_wifi_valid();
    if (AT_RETURN_SUCCESS != ret)
    {       
        PLAT_WLAN_ERR("check_wifi_valid failed!\n");        
        return AT_WIENABLE_OFF;
    }
    
    PLAT_WLAN_INFO("%s in, wifiStatus = %d\n",__FUNCTION__,g_wlan_at_data.wifiStatus);
    return g_wlan_at_data.wifiStatus;
}

/*****************************************************************************
 ��������  : int32 WlanATSetWifiMode(WLAN_AT_WIMODE_TYPE mode)
 ��������  : ����WiFi AP֧�ֵ���ʽ
 �������  :  0,  CWģʽ
             2,  802.11b��ʽ
             3,  802.11g��ʽ
             4,  802.11n��ʽ

 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
*****************************************************************************/
STATIC int32 ATSetWifiMode(WLAN_AT_WIMODE_TYPE mode)
{
    PLAT_WLAN_INFO("WLAN_AT_WIMODE_TYPE:%d\n", mode);  

    if (mode > AT_WIMODE_80211n || AT_WIMODE_80211a == mode)
    {
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

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%s", WLAN_AT_MODE_BCM43217_SUPPORT);
    
    return (AT_RETURN_SUCCESS);
}

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
STATIC int32 ATSetWifiBand(WLAN_AT_WIBAND_TYPE bandwith)
{
    int32 ret = AT_RETURN_SUCCESS;
    char wl_cmd[WL_CMD_BUF_LEN] = {0};
    
    PLAT_WLAN_INFO("%s:in,band = %d\n", __FUNCTION__, bandwith);
    
    switch (bandwith)
    {
        case AT_WIBAND_20M: 
            {
                g_wlan_at_data.wifiBandwith = AT_WIBAND_20M;
                break;
            }             
        case AT_WIBAND_40M:
            {                
                if (AT_WIMODE_80211n == g_wlan_at_data.wifiMode)
                {
                    g_wlan_at_data.wifiBandwith = AT_WIBAND_40M;                    
                }
                else
                {   
                    PLAT_WLAN_ERR("Error wifi mode,must in n mode\n");
                    ret = AT_RETURN_FAILURE;
                }
                break;
            }
        default:
            ret = AT_RETURN_FAILURE;
            break;
    }

    if (AT_RETURN_FAILURE == ret)
    {
        return ret;
    }

    memset(wl_cmd, 0, WL_CMD_BUF_LEN);
    OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s down", IF_2G);
    WIFI_TEST_CMD(wl_cmd);

    memset(wl_cmd, 0, WL_CMD_BUF_LEN);
    OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s mimo_txbw %d", 
        IF_2G, 2 * (bandwith + 1));
    WIFI_TEST_CMD(wl_cmd);
   
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

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%d", g_wlan_at_data.wifiBandwith);
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

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%s", WLAN_AT_BCM43217_BAND_SUPPORT);
    
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 ��������  : int32 WlanATSet2GWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
 ��������  : ����WiFiƵ��
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
*****************************************************************************/
STATIC int32 WlanATSet2GWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
{
    int8   wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    int8   channeStr[WIFI_CMD_8_SIZE] = {0}; 
    uint16 ulWifiFreq = 0;
    uint16 i = 0;    
    int32  ret = AT_RETURN_SUCCESS;
    
    /* 2.4GƵ�㼯�� �ŵ�1-14 */
    const uint16   ausChannels[] = {2412,2417,2422,2427,2432,2437,2442,2447,2452,2457,2462,2467,2472,2484}; 
    
    /* 2.4G 40MƵ�㼯�� �����ŵ� 3-11 */
    const uint16   ausChannels_40M[] = {2422, 2427, 2432, 2437, 2442, 2447, 2452, 2457, 2462};
       
    
    if (NULL == pFreq)
    {
        return (AT_RETURN_FAILURE);
    }

    if (g_ulRxState)
    {      
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s down", IF_2G);
        WIFI_TEST_CMD(wl_cmd);   
        g_ulRxState = 0;
    }

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s band b", IF_2G);
    WIFI_TEST_CMD(wl_cmd);
    
    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s country ALL", IF_2G);
    WIFI_TEST_CMD(wl_cmd);

    /*20M����*/
    if (AT_WIBAND_20M == g_wlan_at_data.wifiBandwith)
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
        else
        {        
            PLAT_WLAN_INFO("Error 20M wifiFreq parameters\n");      
            return AT_RETURN_FAILURE;
        }

        PLAT_WLAN_INFO("Target Channel = %d\n", ulWifiFreq);

        if (!(((WLAN_CHANNEL_2G_MIN <= ulWifiFreq) && (ulWifiFreq <= WLAN_CHANNEL_2G_MAX))))
        {
            PLAT_WLAN_INFO("Target Channel ERROR,ulWifiFreq = %u!\n", ulWifiFreq);
            return AT_RETURN_FAILURE;
        }
        
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s bw_cap 2g 1", IF_2G);
        WIFI_TEST_CMD(wl_cmd);
        
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE,WL_CONFIG_EXE_PATH" %s chanspec %d", IF_2G, ulWifiFreq);
        WIFI_TEST_CMD(wl_cmd);

    }
    else 
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
        else
        {        
            PLAT_WLAN_INFO("Error 40M wifiFreq parameters\n");      
            return AT_RETURN_FAILURE;
        }
        
        /* �ӵ�3���ŵ���ʼ����Ҫ��2 */
        ulWifiFreq = ulWifiFreq + 2;
        PLAT_WLAN_INFO("Target Channel = %d\n", ulWifiFreq);
    
        if (!(((WLAN_CHANNEL_2G_MIN <= ulWifiFreq) && (WLAN_CHANNEL_2G_MAX >= ulWifiFreq))))
        {
            PLAT_WLAN_INFO("Target Channel ERROR!\n");
            return AT_RETURN_FAILURE;
        }
        
        /*40M�ŵ���U/L����*/
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s bw_cap 2g 3", IF_2G);
        WIFI_TEST_CMD(wl_cmd);
        
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s chanspec -c %d -b 2 -w 40 -s 1", IF_2G, ulWifiFreq);
        WIFI_TEST_CMD(wl_cmd);
    }

    /* ����ȫ�ֱ�����Ա���ѯ */
    g_wlan_at_data.wifiFreq.value = pFreq->value;
    g_wlan_at_data.wifiFreq.offset = pFreq->offset;
    return ret;

}

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
    if (NULL == pFreq)
    {
        return (AT_RETURN_FAILURE);
    }

    if (WLAN_AT_2G_BAND == g_wlan_at_data.wifiBand)
    {
        return WlanATSet2GWifiFreq(pFreq);
    }
    else
    {
        return (AT_RETURN_FAILURE);
    }
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
    ASSERT_NULL_POINTER(pFreq, AT_RETURN_FAILURE);
    
    memcpy(pFreq, &(g_wlan_at_data.wifiFreq), sizeof(WLAN_AT_WIFREQ_STRU));
      
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 ��������  : int32 WlanATSet2GWifiDataRate(uint32 rate)
 ��������  : ����WiFi��������
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
*****************************************************************************/
STATIC int32 WlanATSet2GWifiDataRate(uint32 rate)
{    
    int8    wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    uint32  ulWifiRate = rate / HUNDRED;
    uint32  ulNRate = 0;  
    
    const BRATE_ST wifi_brates_table[] = {{100, "1"}, {200, "2"}, {550, "5.5"}, {1100, "11"}};//b 
    const uint32 wifi_20m_nrates_table[] = {650, 1300, 1950, 2600, 3900, 5200, 5850, 6500};//n ht20
    const uint32 wifi_40m_nrates_table[] = {1350, 2700, 4050, 5400, 8100, 10800, 12150, 13500};//n ht40 
    
    #define WIFI_BRATES_TABLE_SIZE (sizeof(wifi_brates_table) / sizeof(BRATE_ST))
    #define WIFI_20M_NRATES_TABLE_SIZE (sizeof(wifi_20m_nrates_table) / sizeof(uint32))
    #define WIFI_40M_NRATES_TABLE_SIZE (sizeof(wifi_40m_nrates_table) / sizeof(uint32))
    
    PLAT_WLAN_INFO("WifiRate = %u\n", ulWifiRate);
    
    switch (g_wlan_at_data.wifiMode)
    {
        case AT_WIMODE_CW:
            PLAT_WLAN_INFO("AT_WIMODE_CW\n");
            return (AT_RETURN_FAILURE);

        case AT_WIMODE_80211b:
        case AT_WIMODE_80211g:
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s nrate -r %d", IF_2G, ulWifiRate);
            WIFI_TEST_CMD(wl_cmd);
            break; 
            
        case AT_WIMODE_80211n:
            if (AT_WIBAND_20M == g_wlan_at_data.wifiBandwith)
            {
                /* WIFI 20M nģʽWL���������ֵΪ0~7����8�� */
                for (ulNRate = 0; ulNRate < WIFI_20M_NRATES_TABLE_SIZE; ulNRate++)
                {
                    if (wifi_20m_nrates_table[ulNRate] == rate)
                    {
                        PLAT_WLAN_INFO("20M NRate Index = %u\n", ulNRate);                        
                        break;
                    }
                }

                if (WIFI_20M_NRATES_TABLE_SIZE == ulNRate)
                {
                    PLAT_WLAN_INFO("20M NRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }

                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s nrate -m %d", IF_2G, ulNRate);
                WIFI_TEST_CMD(wl_cmd);
            }
            else if (AT_WIBAND_40M == g_wlan_at_data.wifiBandwith)
            {
                for (ulNRate = 0; ulNRate < WIFI_40M_NRATES_TABLE_SIZE; ulNRate++)
                {
                    if (wifi_40m_nrates_table[ulNRate] == rate)
                    {
                        PLAT_WLAN_INFO("40M NRate Index = %u\n", ulNRate);
                        break;
                    }
                }

                if (WIFI_40M_NRATES_TABLE_SIZE == ulNRate)
                {
                    PLAT_WLAN_INFO("40M NRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }    
                
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s nrate -m %d -s 3", IF_2G, ulNRate);
                WIFI_TEST_CMD(wl_cmd);              
            }    
            break;
        default:
            return (AT_RETURN_FAILURE);            
    } 
    
    WIFI_TEST_CMD(wl_cmd);
    /*����ȫ�ֱ�����Ա���ѯ*/
    g_wlan_at_data.wifiRate = rate;
       
    return (AT_RETURN_SUCCESS);       
}


STATIC int32 AT_TransferWifiRate(uint32 ulInRate, uint32 *pulOutRate)
{
    uint8    ucIndex = 0;
    /* WIFI nģʽ AT^WIDATARATE���õ�����ֵ��WL��������ֵ�Ķ�Ӧ�� */
    uint32  aucAtWifiN20Rate_Table[] = {650, 1300, 1950, 2600, 3900, 5200, 5850, 6500, \
                                                               1300, 2600, 3900, 5200, 7800, 10400, 11700, 13000};

    uint32  aucAtWifiN40Rate_Table[] = {1350, 2700, 4050, 5400, 8100, 10800, 12150, 13500, \
                                                               2700, 5400, 8100, 10800, 16200, 21600, 24300, 27000};
    

    if (AT_WIBAND_20M == g_wlan_at_data.wifiBandwith)
    {

        if (RF_2G_0 == g_ulWifiRF || RF_2G_1 == g_ulWifiRF)
        {
            for (ucIndex = 0; ucIndex < AT_WIFI_N_RATE_NUM / 2; ucIndex++)
            {
                if (aucAtWifiN20Rate_Table[ucIndex] == ulInRate)
                {
                    *pulOutRate = ucIndex;
                    break;
                }
            }

            if (ucIndex >= AT_WIFI_N_RATE_NUM / 2)
            {
                return AT_RETURN_FAILURE;
            }
        }
        else if (RF_2G_MIMO == g_ulWifiRF)
        {
            for (ucIndex = AT_WIFI_N_RATE_NUM / 2; ucIndex < AT_WIFI_N_RATE_NUM; ucIndex++)
            {
                if (aucAtWifiN20Rate_Table[ucIndex] == ulInRate)
                {
                    *pulOutRate = ucIndex;
                    break;
                }
            }
            if (ucIndex >= AT_WIFI_N_RATE_NUM)
            {
                return AT_RETURN_FAILURE;
            }
        }
    }
    else if (AT_WIBAND_40M == g_wlan_at_data.wifiBandwith)
    { 
        if (RF_2G_0 == g_ulWifiRF || RF_2G_1 == g_ulWifiRF)
        {
            for (ucIndex = 0; ucIndex < AT_WIFI_N_RATE_NUM / 2; ucIndex++)
            {
                if (aucAtWifiN40Rate_Table[ucIndex] == ulInRate)
                {
                    *pulOutRate = ucIndex;
                    break;
                }
            }
            if (ucIndex >= AT_WIFI_N_RATE_NUM / 2)
            {
                return AT_RETURN_FAILURE;
            }
        }
        else if (RF_2G_MIMO == g_ulWifiRF )
        {
            for (ucIndex = AT_WIFI_N_RATE_NUM / 2; ucIndex < AT_WIFI_N_RATE_NUM; ucIndex++)
            {
                if (aucAtWifiN40Rate_Table[ucIndex] == ulInRate)
                {
                    *pulOutRate = ucIndex;
                    break;
                }
            }
            if (ucIndex >= AT_WIFI_N_RATE_NUM)
            {
                return AT_RETURN_FAILURE;
            }
        }
    }

    return AT_RETURN_SUCCESS;
}

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

    if (WLAN_AT_2G_BAND == g_wlan_at_data.wifiBand)
    {
        return WlanATSet2GWifiDataRate(rate);
    }
    else
    {
        return (AT_RETURN_FAILURE);
    }
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
    /*����ȫ�ֱ�����Ա���ѯ*/
    g_wlan_at_data.wifiPower = power_dBm_percent;
    return (AT_RETURN_SUCCESS);
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

/*****************************************************************************
 ��������  : int32 WlanATSet2GWifiTX(WLAN_AT_FEATURE_TYPE onoff)
 ��������  : �򿪻�ر�wifi�����
 �������  : 0 �ر�
             1 ��
 �������  : NA
 �� �� ֵ  : 0 �ɹ�
             1 ʧ��
 ����˵��  : 
*****************************************************************************/
STATIC int32 WlanATSet2GWifiTX(WLAN_AT_FEATURE_TYPE onoff)
{
    int8   wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    int32 WifiPower = 0;

    if (AT_WIMODE_CW == g_wlan_at_data.wifiMode)
    {
        /* cw ���� */
    }
    else
    {
        if (AT_FEATURE_DISABLE == onoff)
        {
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s pkteng_stop tx", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH " %s down", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH " %s txchain 3", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH " %s up", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH " %s phy_watchdog 1", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

        }
        else
        {
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s down", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s txchain 3", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s up", IF_2G);
            WIFI_TEST_CMD(wl_cmd);
            
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s ssid \"\"", IF_2G);
            WIFI_SHELL_CMD(wl_cmd);

            if(RF_2G_0 == g_ulWifiRF || RF_2G_1 == g_ulWifiRF)
            {
                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s txant %d",  IF_2G,  g_ulWifiRF);
                WIFI_TEST_CMD(wl_cmd);

                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s antdiv %d",  IF_2G,  g_ulWifiRF);
                WIFI_TEST_CMD(wl_cmd);
            }

            WifiPower = g_wlan_at_data.wifiPower / 100;

            if ((WLAN_AT_POWER_MIN <= WifiPower) && (WLAN_AT_POWER_MAX >= WifiPower))
            {
                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s txpwr1 -o -q %d", IF_2G, WifiPower * 4);
                WIFI_TEST_CMD(wl_cmd);
            }
            else
            {
                PLAT_WLAN_INFO("2.4G invaild argument and will use power vaule in nv\n");
            }

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s phy_forcecal 1", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            msleep(DELAY_150_TIME); /* ��У׼�ͷ�������ʱ150���룬��У׼��Ч������burst�ź�ʧ�� */

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s pkteng_start 00:90:4c:21:00:8e tx 100 1500 0", IF_2G);
            WIFI_TEST_CMD(wl_cmd);
        } 
    }

    /*����ȫ�ֱ�����ѱ���ѯ*/
    g_wlan_at_data.wifiTX = onoff;

    return (AT_RETURN_SUCCESS);
}

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
    if (WLAN_AT_2G_BAND == g_wlan_at_data.wifiBand)
    {
        return WlanATSet2GWifiTX(onoff);
    }
    else
    {
        return (AT_RETURN_FAILURE);
    }
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
 ��������  : WLAN_AT_FEATURE_TYPE WlanATSetWifiRX()
 ��������  : ��ѯ��ǰWiFi�����״̬��Ϣ
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �رշ����
             1 �򿪷����
 ����˵��  : 
*****************************************************************************/
STATIC WLAN_AT_FEATURE_TYPE WlanATSet2GWifiRX(WLAN_AT_WIRX_STRU *params)
{
    int8    wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    ASSERT_NULL_POINTER(params, AT_RETURN_FAILURE);
        
    switch (params->onoff)
    {
        case AT_FEATURE_DISABLE:
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s pkteng_stop rx", IF_2G);
            WIFI_TEST_CMD(wl_cmd);      
            
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s down", IF_2G);
            WIFI_TEST_CMD(wl_cmd);
            
            g_ulRxState = AT_FEATURE_DISABLE;
            break;
            
        case AT_FEATURE_ENABLE:
            if (0 == strncmp(params->src_mac, "", MAC_ADDRESS_LEN))
            {
                PLAT_WLAN_INFO("src mac is NULL\n");
                return (AT_RETURN_FAILURE);
            }
            PLAT_WLAN_INFO("src mac is %s\n",params->src_mac);
            
            memset(wl_cmd, 0, WL_CMD_BUF_LEN);
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), WL_CONFIG_EXE_PATH" %s rxchain %u", IF_2G, 
                (g_wlan_at_data.wifiGroup % 2 + 1));   /*����ģʽ*/
            WIFI_TEST_CMD(wl_cmd);
            
            memset(wl_cmd, 0, WL_CMD_BUF_LEN);
            OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s ssid \"\"", IF_2G);
            WIFI_SHELL_CMD(wl_cmd);
            
            memset(wl_cmd, 0, WL_CMD_BUF_LEN);
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), WL_CONFIG_EXE_PATH"%s up", IF_2G);
            WIFI_TEST_CMD(wl_cmd);
            
            memset(wl_cmd, 0, WL_CMD_BUF_LEN);
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), WL_CONFIG_EXE_PATH"%s phy_forcecal 1", IF_2G);
            WIFI_TEST_CMD(wl_cmd);
            
            msleep(DELAY_150_TIME);  /* ��У׼���հ�����ʱ150���룬��У׼��Ч */

            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), WL_CONFIG_EXE_PATH" %s pkteng_start %s rx", IF_2G, params->src_mac);
            WIFI_TEST_CMD(wl_cmd);
            
            g_ulRxState = AT_FEATURE_ENABLE;           
            
            wifi_get_rx_packet_report(&g_ulUcastWifiRxPkts,  &g_ulMcastWifiRxPkts);
            break;
            
        default:
            return (AT_RETURN_FAILURE);
    }
    
    memcpy(&g_wlan_at_data.wifiRX, params, sizeof(WLAN_AT_WIRX_STRU));
        
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 ��������  : WLAN_AT_FEATURE_TYPE WlanATSetWifiRX()
 ��������  : ��ѯ��ǰWiFi�����״̬��Ϣ
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0 �رշ����
             1 �򿪷����
 ����˵��  : 
*****************************************************************************/
STATIC WLAN_AT_FEATURE_TYPE ATSetWifiRX(WLAN_AT_WIRX_STRU *params)
{
    if (WLAN_AT_2G_BAND == g_wlan_at_data.wifiBand)
    {
        return WlanATSet2GWifiRX(params);
    }
    else
    {
        return (AT_RETURN_FAILURE);
    }
    
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
    ASSERT_NULL_POINTER(params, AT_RETURN_FAILURE);

    memcpy(params, &g_wlan_at_data.wifiRX, sizeof(WLAN_AT_WIRX_STRU));
    
    return (AT_RETURN_SUCCESS);
}

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
    ASSERT_WiFi_OFF(AT_RETURN_FAILURE);

    if (VOS_OK != flag)
    {
        PLAT_WLAN_INFO("Exit on flag = %d\n", flag);
        return (AT_RETURN_FAILURE);
    }

    wifi_get_rx_packet_report(&g_ulUcastWifiRxPkts,  &g_ulMcastWifiRxPkts);
    
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
    unsigned int ulUcastWifiRxPkts = 0;
    unsigned int ulMcastWifiRxPkts = 0;
    unsigned int ulWifiRxPkts = 0;
    unsigned int ulWifiMcastRxPkts = 0;

    int32 ret = AT_RETURN_SUCCESS; 
    ASSERT_NULL_POINTER(params, AT_RETURN_FAILURE);

    /* �жϽ��ջ��Ƿ�� */
    if (AT_FEATURE_DISABLE == g_wlan_at_data.wifiRX.onoff)
    {
        PLAT_WLAN_INFO("Not Rx Mode.\n");
        ret = AT_RETURN_FAILURE;
        return ret;
    }   
    
    wifi_get_rx_packet_report(&ulUcastWifiRxPkts,  &ulMcastWifiRxPkts);
    ulWifiRxPkts = (ulUcastWifiRxPkts - g_ulUcastWifiRxPkts);
    ulWifiMcastRxPkts = (ulMcastWifiRxPkts - g_ulMcastWifiRxPkts);

    if (ulWifiMcastRxPkts > ulWifiRxPkts)
    {
        ulWifiRxPkts = ulWifiMcastRxPkts;
    }
   
    params->good_result = ulWifiRxPkts;
    params->bad_result = 0;   
    
    PLAT_WLAN_INFO("Exit [good = %d, bad = %d]\n", params->good_result, params->bad_result);   
    
    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
/*(10)^WIINFO ��ѯWiFi�������Ϣ*/
//////////////////////////////////////////////////////////////////////////
#define SIZE_OF_INFOGROUP(group) (sizeof(group) / sizeof(WLAN_AT_WIINFO_MEMBER_STRU))
/*****************************************************************************
 ��������  : uint32 ATGetWifiInfo(WLAN_AT_WIINFO_STRU *params)
 ��������  : ��ѯWiFi�������Ϣ(�ڲ��ӿ�)
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 
*****************************************************************************/
STATIC int32 ATGetWifiInfo(WLAN_AT_WIINFO_STRU *params)
{         
    static char sChannels24G[] = "3,7,11";

    /*�ŵ���Ϣ*/
    static WLAN_AT_WIINFO_MEMBER_STRU sChannelGroup0[] = 
    {       
        {"b", sChannels24G},
        {"g", sChannels24G},
        {"n", sChannels24G},
        {"n4", sChannels24G}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sChannelGroup1[] = 
    {              
        {"b", sChannels24G},
        {"g", sChannels24G},
        {"n", sChannels24G},
        {"n4", sChannels24G}
    };
    
    /*������Ϣ*/
    static WLAN_AT_WIINFO_MEMBER_STRU sPowerGroup0[] =
    {      
        {"b", "160"},
        {"g", "160"},
        {"n", "160"},
        {"n4","160"}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sPowerGroup1[] =
    {            
        {"b", "160"},
        {"g", "160"},
        {"n", "160"},
        {"n4","160"}
    };

    /*Ƶ����Ϣ:0��ʾ2.4G,1��ʾ5G*/  
    static WLAN_AT_WIINFO_MEMBER_STRU sFreqGroup0[] =
    {         
        {"b", "0"},
        {"g", "0"},
        {"n", "0"},
        {"n4", "0"}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sFreqGroup1[] =
    {               
        {"b", "0"},
        {"g", "0"},
        {"n", "0"},
        {"n4", "0"}
    };

    static WLAN_AT_WIINFO_GROUP_STRU sInfoGroup0[] =
    {
        {sChannelGroup0, SIZE_OF_INFOGROUP(sChannelGroup0)},
        {sPowerGroup0, SIZE_OF_INFOGROUP(sPowerGroup0)},
        {sFreqGroup0, SIZE_OF_INFOGROUP(sFreqGroup0)}
    };   

    static WLAN_AT_WIINFO_GROUP_STRU sInfoGroup1[] =
    {
        {sChannelGroup1, SIZE_OF_INFOGROUP(sChannelGroup1)},
        {sPowerGroup1, SIZE_OF_INFOGROUP(sPowerGroup1)},
        {sFreqGroup1, SIZE_OF_INFOGROUP(sFreqGroup1)}
  
    };
    
    static WLAN_AT_WIINFO_GROUP_STRU *sTotalInfoGroups[] =
    {
        sInfoGroup0,
        sInfoGroup1 
    };
   
    char *strBuf = NULL;
    int32 idx = 0, iLen = 0, igroup = 0,itype = 0, iTmp = 0;
    WLAN_AT_WIINFO_GROUP_STRU *pstuInfoGrup = NULL;
    WLAN_AT_WIINFO_GROUP_STRU *pstuInfoType = NULL;
    
    ASSERT_NULL_POINTER(params, AT_RETURN_FAILURE);
    PLAT_WLAN_INFO("Enter ATGetWifiInfo [group=%d,type=%u]\n", params->member.group, params->type);

    igroup = (int32)params->member.group;
    if (WLAN_AT_GROUP_MAX < igroup)
    {
        return (AT_RETURN_FAILURE);
    }   

    itype = (int32)params->type;
    if (WLAN_AT_TYPE_MAX < itype)
    {
        return (AT_RETURN_FAILURE);
    }      
   
    strBuf = (char *)params->member.content;
    iLen = (int32)(sizeof(params->member.content) - 1);

    pstuInfoGrup = sTotalInfoGroups[igroup];

    pstuInfoType = &pstuInfoGrup[itype]; 
    for(idx = 0; idx < pstuInfoType->size; idx++)
    {
        if (NULL == pstuInfoType->member[idx].name
          || NULL == pstuInfoType->member[idx].value)
        {
            continue;
        }
    
        OSA_SNPRINTF(strBuf, iLen, "%s,%s"
                    , pstuInfoType->member[idx].name
                    , pstuInfoType->member[idx].value);
        
        iTmp = (int32)(strlen(strBuf) + 1);
        iLen -= iTmp;
        strBuf += iTmp;
        if (iLen <= 0)
        {
            return (AT_RETURN_FAILURE);
        }
    }   
   
    *strBuf = '\0';
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 ��������  : WLAN_AT_WIPLATFORM_TYPE WlanATGetWifiPlatform()
 ��������  : ��ѯWiFi����ƽ̨��Ӧ����Ϣ
 �������  : NA
 �������  : NA
 �� �� ֵ  : ��Ӧ������
 ����˵��  : 
*****************************************************************************/
STATIC WLAN_AT_WIPLATFORM_TYPE ATGetWifiPlatform(void)
{
    return (AT_WIPLATFORM_BROADCOM);
}


STATIC void  AT_InitWiFiBCM2G(void)
{
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    
    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s down", IF_2G);
    WIFI_TEST_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s country ALL", IF_2G);
    WIFI_TEST_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s mpc 0", IF_2G);
    WIFI_TEST_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s interference 0", IF_2G);
    WIFI_TEST_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s phy_watchdog 0", IF_2G);
    WIFI_TEST_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s scansuppress 1", IF_2G);
    WIFI_TEST_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s frameburst 0", IF_2G);
    WIFI_TEST_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s ampdu 0", IF_2G);
    WIFI_TEST_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s mimo_bw_cap 1", IF_2G);
    WIFI_TEST_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s bi 65535", IF_2G);
    WIFI_TEST_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s up", IF_2G);
    WIFI_TEST_CMD(wl_cmd);

    if (0 == g_ul2GUP)
    {
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, "/system/bin/ifconfig %s up", IF_2G);
        WIFI_SHELL_CMD(wl_cmd);

        g_ul2GUP = 1;
    }
}

/*****************************************************************************
 ��������  : int32 WlanATSetTSELRF(uint32 group)
 ��������  : �������ߣ��Ƕ�ͨ·��0
 �������  : NA
 �������  : NA
 �� �� ֵ  : ��ǰ��������
 ����˵��  : 
*****************************************************************************/
STATIC int32 ATGetTSELRF(void) /* ��ȡ���� */
{
    return g_wlan_at_data.wifiGroup;
}

/*****************************************************************************
 ��������  : int32 WlanATSetTSELRF(uint32 group)
 ��������  : �������ߣ��Ƕ�ͨ·��0
 �������  : strBuf ֧��������������
 �������  : NA
 �� �� ֵ  : AT_RETURN_SUCCESS �ɹ�
             AT_RETURN_FAILURE ʧ��
 ����˵��  : 
*****************************************************************************/
STATIC int32 ATGetTSELRFSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    ASSERT_NULL_POINTER(strBuf, AT_RETURN_FAILURE);

    OSA_SNPRINTF(strBuf->buf, sizeof(strBuf->buf), "%s", WLAN_AT_TSELRF_SUPPORT);
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 ��������  : int32 WlanATSetTSELRF(uint32 group)
 ��������  : �������ߣ��Ƕ�ͨ·��0
 �������  : group ��������
 �������  : NA
 �� �� ֵ  : AT_RETURN_SUCCESS �ɹ�
             AT_RETURN_FAILURE ʧ��
 ����˵��  : 
*****************************************************************************/
STATIC int32 ATSetTSELRF(uint32 group)
{
    int8 acCmd[WIFI_CMD_MAX_SIZE] = {0};
     
     if(WLAN_AT_GROUP_MAX < group)
     {
         return AT_RETURN_FAILURE;
     }
     
     g_wlan_at_data.wifiGroup = group;
     g_ulWifiRF = group;

     if (g_wlan_at_data.wifiGroup == RF_2G_0 
        || g_wlan_at_data.wifiGroup == RF_2G_1
        || g_wlan_at_data.wifiGroup == RF_2G_MIMO)
     {
         g_wlan_at_data.wifiBand = WLAN_AT_2G_BAND;
     }
     else
     {
         PLAT_WLAN_INFO("Set incorrect TSELRF group %d\n", g_ulWifiRF);
         return AT_RETURN_FAILURE;
     }
    
     PLAT_WLAN_INFO("[%s]:Enter,group = %u\n", __FUNCTION__, group); 
    
     AT_InitWiFiBCM2G();
     
     return (AT_RETURN_SUCCESS);

}

/*****************************************************************************
 ��������  : int wifi_set_pa_mode(int wifiPaMode)
 ��������  : ����WiFi PA���������
 �������  : wifiPaMode ����ģʽ
 �������  : NA
 �� �� ֵ  : AT_RETURN_SUCCESS �ɹ�
             AT_RETURN_FAILURE ʧ��
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
 �������  : pa_type  paģʽ
 �������  : NA
 �� �� ֵ  : AT_RETURN_SUCCESS �ɹ�
             AT_RETURN_FAILURE ʧ��
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
 �� �� ֵ  : AT_RETURN_SUCCESS �ɹ�
             AT_RETURN_FAILURE ʧ��
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
 �������  : strBuf  ֧�ֵ�paģʽ����
 �������  : NA
 �� �� ֵ  : AT_RETURN_SUCCESS �ɹ�
             AT_RETURN_FAILURE ʧ��
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

/*****************************************************************************
 ��������  : int32 WlanATGetWifi2GPavars(WLAN_AT_BUFFER_STRU *strBuf)
 ��������  : 2.4G��Ƶ������ѯ��װ����
 �������  : strBuf 
 �������  : NA
 �� �� ֵ  : AT_RETURN_SUCCESS �ɹ�
             AT_RETURN_FAILURE ʧ��
 ����˵��  : 
*****************************************************************************/
STATIC int32 ATGetWifi2GPavars(WLAN_AT_BUFFER_STRU *strBuf)
{
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    int8 filebuff[WL_COUNTER_BUF_LEN + 1] = {0};
    int8 foramtdata2g[ANT_2G_NUM][NVM_2G_NEW_STR_LENTH] ={"pa2ga0=","pa2ga1="};
    int8 realdata2g[ANT_2G_OLD_GRP_NUM][NVM_2G_OLD_STR_LENTH] ={"pa2gw0a0","pa2gw1a0","pa2gw2a0","pa2gw0a1","pa2gw1a1","pa2gw2a1"};
    int32 i = 0;
    int32 j = 0;
    char *temp = NULL;

    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }
           
    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s pavars > /var/tmpfile", IF_2G);
    WIFI_SHELL_CMD(wl_cmd);
 
    /*pavars����ִ�н��*/
    if(0 == wlan_read_file("/var/tmpfile", filebuff, sizeof(filebuff)))
    {
        PLAT_WLAN_ERR("read_from_file fail");
        return AT_RETURN_FAILURE;
    }

    /*��װ2.4g����ֵ����5g��ʾ��ʽһ��*/
    for ( i = 0; i < ANT_2G_NUM ; i++ )
    {
        strncat(strBuf->buf, foramtdata2g[i], strlen(foramtdata2g[i]));

        for ( ; j < ((ANT_2G_OLD_GRP_NUM / ANT_2G_NUM) * (i + 1)); j++)
        {
            temp = strstr(filebuff, realdata2g[j]);
            temp = temp + sizeof(realdata2g[j]);
            strncat(strBuf->buf, temp, ANT_2G_CAL_NV_VALUE_BYTE_NUM);
            if (((ANT_2G_OLD_GRP_NUM / ANT_2G_NUM) * (i + 1) - 1) > j)
            {
                strncat(strBuf->buf, ",", 1);
            }
            else
            {
                strncat(strBuf->buf, "\n", 1);
            }
        }
    }
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 ��������  : int32 WlanATSetWifi2GPavars(WLAN_AT_PAVARS2G_STRU *strBuf)
 ��������  : 2.4G��Ƶ�������÷�װ����
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 
*****************************************************************************/
STATIC int32 ATSetWifi2GPavars(WLAN_AT_PAVARS2G_STRU *strBuf)
{
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    int8 wl_temp[WIFI_CMD_MAX_SIZE] = {0};
    int32 i;
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }
           
    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH"%s down", IF_2G);
    WIFI_SHELL_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH"%s pavars ", IF_2G);
    for(i =0 ; i < (ANT_2G_OLD_GRP_NUM / ANT_2G_NUM); i++)
    {
        OSA_SNPRINTF(wl_temp, WIFI_CMD_MAX_SIZE, "pa2gw%da%d=%s ", i, strBuf->ant_index, strBuf->data[i]);
        strncat(wl_cmd, wl_temp, strlen(wl_temp));   
    }
    WIFI_SHELL_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s up", IF_2G);
    WIFI_SHELL_CMD(wl_cmd);

    return (AT_RETURN_SUCCESS);
}
    
/*****************************************************************************
 ��������  : int32 GET_VALUE_FROM_NV(viod)
 ��������  : ��ȡwifi��nv(50579,50580)�е�ֵ
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 
*****************************************************************************/
STATIC int32 GET_VALUE_FROM_NV(unsigned short type, unsigned int len,char *buf)
{
    unsigned short nv_id = type;  
    int32 ret = AT_RETURN_FAILURE;
    /* ��ȡnv���� */
    ret = NV_Read(nv_id, buf, len);
    if (NV_OK != ret)
    {
        PLAT_WLAN_ERR("Error, nv read Failed! nv_id = %d, len = %d", nv_id, len);
        return AT_RETURN_FAILURE;
    }   
    return AT_RETURN_SUCCESS;
}
    
/*****************************************************************************
 ��������  : int32 Check_WIFI_CAL_NV(viod)
 ��������  : ���wifi��nv(50579,50580)�е�ֵ
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
 ����˵��  : 
*****************************************************************************/
STATIC void Check_WIFI_CAL_NV(void)
{
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    int8 temp[WIFI_CMD_MAX_SIZE]={0};
    unsigned short nv_id_2g = NV_ID_WIFI_2G_RFCAL;
    unsigned short nv_wifi_2g_len = 0;
    char buf_2g[WLAN_AT_BUFFER_SIZE] = {0};
    int i = 0;
    int j = 0;
    int32 ret = AT_RETURN_FAILURE;
    
    nv_wifi_2g_len = sizeof(NV_WIFI_2G_RFCAL_STRU);
    
    ret = GET_VALUE_FROM_NV(nv_id_2g, nv_wifi_2g_len, buf_2g);
    if (WLAN_PARA_ERR == ret)
    {
        return AT_RETURN_FAILURE ;
    }
    
    /*2.4g����У׼����*/
    for (i = 0; i < ANT_2G_NUM; i++)
    {   
        if (0 != *(buf_2g + i*ANT_2G_CAL_NV_VALUE_BYTE_NUM))
        {
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s pavars ", IF_2G);
            for(j = 0; j < ANT_2G_CAL_NV_VALUE_BYTE_NUM/2; j++)
            {
                memset(temp, 0, sizeof(temp));
                OSA_SNPRINTF(temp, WIFI_CMD_MAX_SIZE, "pa2gw%da%d=0x%02x%02x ", j, i,*(buf_2g + i*ANT_2G_CAL_NV_VALUE_BYTE_NUM + j*2), \
                    *(buf_2g + i*ANT_2G_CAL_NV_VALUE_BYTE_NUM + j*2 + 1));
                strcat(wl_cmd, temp);
            }
            WIFI_SHELL_CMD(wl_cmd);
        }
    }
      
    return AT_RETURN_SUCCESS;
}

WLAN_CHIP_OPS bcm43217_ops = 
{
    .WlanATSetWifiEnable = ATSetWifiEnable,
    .WlanATGetWifiEnable = ATGetWifiEnable,
    
    .WlanATSetWifiMode   = ATSetWifiMode,
    .WlanATGetWifiMode   = ATGetWifiMode,
    .WlanATGetWifiModeSupport = ATGetWifiModeSupport,

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
    
    .WlanATGetWifiInfo = ATGetWifiInfo,
    
    .WlanATGetWifiPlatform = ATGetWifiPlatform,
    
    .WlanATGetTSELRF = ATGetTSELRF,
    .WlanATSetTSELRF = ATSetTSELRF,
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
    .WlanATSetWifi2GPavars = ATSetWifi2GPavars,
    .WlanATGetWifi2GPavars = ATGetWifi2GPavars,
    .WlanATSetWifi5GPavars = NULL,
    .WlanATGetWifi5GPavars = NULL
};

