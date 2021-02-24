
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

#define WL_DEFAULT_POWER        (600) /* 默认功率 */

#define WLAN_AT_MODE_BCM43217_SUPPORT    "2,3,4"             /*支持的模式(b/g/n)*/    
             
#define WLAN_AT_BCM43217_BAND_SUPPORT    "0,1"               /*支持的带宽(20M/40M)*/

#define WLAN_AT_TSELRF_SUPPORT          "0,1"                /*支持的天线索引序列*/

#define WLAN_AT_GROUP_MAX               (1)                  /*支持的最大天线索引*/
#define WLAN_AT_TYPE_MAX                (2)                  /*支持获取的最大信息类型*/

/*WIFI功率的上下限*/
#define WLAN_AT_POWER_MIN               (-15)
#define WLAN_AT_POWER_MAX               (30)

/*WiFi增益模式*/
#define AT_WIFI_MODE_ONLY_PA            (0x00)              /*WIFI只支持PA模式*/
#define AT_WIFI_MODE_ONLY_NOPA          (0x01)              /*WIFI只支持NO PA模式*/
#define AT_WIFI_MODE_PA_NOPA            (0x02)              /*WIFI同时支持PA模式和NO PA模式*/

#define WLAN_CHANNEL_2G_MIN             (1)                 /*2.4G信道最小值*/
#define WLAN_CHANNEL_2G_MAX             (14)
#define WLAN_CHANNEL_2G_MIDDLE          (6)

#define WLAN_FREQ_2G_MAX                (2484)              /*2.4G最大频点*/

#define WIFI_CMD_MAX_SIZE               (256)                 /*cmd字符串256长度*/
#define WIFI_CMD_8_SIZE                 (8)                   /*cmd字符串8长度*/
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

#define ANT_2G_NUM (2)           /* 处理后WiFi 2G 射频参数个数   */
#define ANT_2G_OLD_GRP_NUM (6)   /* nvm 文件中WiFi 2G 射频参数个数*/
#define ANT_2G_CAL_NV_VALUE_BYTE_NUM (6)

#define NVM_2G_NEW_STR_LENTH     (8)
#define NVM_2G_OLD_STR_LENTH     (9)
/******************************************************************************
变量名称:   g_ulWifiRF
变量作用:   保存Wifi射频天线参数
取值范围:   
调用函数:

注意事项:   该变量为内部业务生效使用的变量，不提供给外部接口使用
******************************************************************************/ 
static unsigned int g_ulWifiRF = 0xffffffff;

/******************************************************************************
变量名称:   g_ulWifiMCSIndex
变量作用:   保存wifi当前速率对应的MCS index
取值范围:   
调用函数:

注意事项:   该变量为内部业务生效使用的变量，不提供给外部接口使用
******************************************************************************/ 
static unsigned int g_ulWifiMCSIndex;

static unsigned int g_ul2GUP = 0;

STATIC void Check_WIFI_CAL_NV(void);

WLAN_RETURN_TYPE prepare_wifi_driver_loaded(void);

/*向WiFi芯片下发配置命令*/
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

/*空指针断言检测*/
#define ASSERT_NULL_POINTER(p, ret)  do { \
    if (NULL == (p)) \
    { \
        PLAT_WLAN_ERR("Exit ret = %d, on NULL Pointer %s", (int)ret, #p); \
        return ret; \
    } \
} while(0)


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
    WLAN_AT_WIBAND_TYPE     wifiBandwith;  /*wifi协议制式*/
    WLAN_AT_WIFREQ_TYPE     wifiBand;      /*wifi当前频段*/
    WLAN_AT_WIFREQ_STRU     wifiFreq;      /*wifi频点信息*/
    uint32                  wifiRate;      /*wifi发射速率*/
    int32                   wifiPower;     /*wifi发射功率*/
    WLAN_AT_FEATURE_TYPE    wifiTX;        /*wifi发射机状态*/
    WLAN_AT_WIRX_STRU       wifiRX;        /*wifi接收机状态*/
    WLAN_AT_WIRPCKG_STRU    wifiPckg;      /*wifi误包码*/  
    uint32                  wifiGroup;     /*wifi天线模式*/
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

/*记录当前的WiFi模式，带宽，频率，速率等参数*/
STATIC WLAN_AT_GLOBAL_ST g_wlan_at_data = {AT_WIENABLE_TEST, AT_WIMODE_80211n, AT_WIBAND_20M, 0
         , {2412, 0}, 6500, 3175, AT_FEATURE_DISABLE, {AT_FEATURE_DISABLE, {0}, {0}}, {0, 0}, 0};/*开启WiFi的默认参数*/

STATIC unsigned int g_ulUcastWifiRxPkts;
STATIC unsigned int g_ulMcastWifiRxPkts;
STATIC unsigned int g_ulRxState = 0;

/*WiFi数据包统计信息*/
STATIC WLAN_AT_PACK_REP_ST g_wifi_packet_report = {0};
STATIC WLAN_AT_PACK_REP_ST g_wifi_packet_new_rep = {0};/*实时信息*/

/*****************************************************************************
 函数名称  : check_wifi_valid
 功能描述  : 查看wifi通路是否ok
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 check_wifi_valid()
{        
    WIFI_TEST_CMD(WL_CONFIG_EXE_PATH" ver");
    return AT_RETURN_SUCCESS;
}

/*****************************************************************************
 函数名称  : check_wifi_driver_loaded
 功能描述  : 查看wifi驱动是否已经加载
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
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
        
        /*检查并配置新的wifi校准参数*/
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

    /* 入参检测 */
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

    pcRecPcktsStart = pcRecPcktsStart + strlen(pValueStr)+ 1; /* 1:空格占位 */
    
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
 函数名称  : int32 WlanATSetWifiMode(WLAN_AT_WIMODE_TYPE mode)
 功能描述  : 设置WiFi AP支持的制式
 输入参数  :  0,  CW模式
             2,  802.11b制式
             3,  802.11g制式
             4,  802.11n制式

 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
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

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%s", WLAN_AT_MODE_BCM43217_SUPPORT);
    
    return (AT_RETURN_SUCCESS);
}

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

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%d", g_wlan_at_data.wifiBandwith);
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

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%s", WLAN_AT_BCM43217_BAND_SUPPORT);
    
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 函数名称  : int32 WlanATSet2GWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
 功能描述  : 设置WiFi频点
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 WlanATSet2GWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
{
    int8   wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    int8   channeStr[WIFI_CMD_8_SIZE] = {0}; 
    uint16 ulWifiFreq = 0;
    uint16 i = 0;    
    int32  ret = AT_RETURN_SUCCESS;
    
    /* 2.4G频点集合 信道1-14 */
    const uint16   ausChannels[] = {2412,2417,2422,2427,2432,2437,2442,2447,2452,2457,2462,2467,2472,2484}; 
    
    /* 2.4G 40M频点集合 中心信道 3-11 */
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

    /*20M带宽*/
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
        
        /* 从第3个信道开始，需要补2 */
        ulWifiFreq = ulWifiFreq + 2;
        PLAT_WLAN_INFO("Target Channel = %d\n", ulWifiFreq);
    
        if (!(((WLAN_CHANNEL_2G_MIN <= ulWifiFreq) && (WLAN_CHANNEL_2G_MAX >= ulWifiFreq))))
        {
            PLAT_WLAN_INFO("Target Channel ERROR!\n");
            return AT_RETURN_FAILURE;
        }
        
        /*40M信道加U/L处理*/
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s bw_cap 2g 3", IF_2G);
        WIFI_TEST_CMD(wl_cmd);
        
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s chanspec -c %d -b 2 -w 40 -s 1", IF_2G, ulWifiFreq);
        WIFI_TEST_CMD(wl_cmd);
    }

    /* 保存全局变量里，以备查询 */
    g_wlan_at_data.wifiFreq.value = pFreq->value;
    g_wlan_at_data.wifiFreq.offset = pFreq->offset;
    return ret;

}

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
    ASSERT_NULL_POINTER(pFreq, AT_RETURN_FAILURE);
    
    memcpy(pFreq, &(g_wlan_at_data.wifiFreq), sizeof(WLAN_AT_WIFREQ_STRU));
      
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 函数名称  : int32 WlanATSet2GWifiDataRate(uint32 rate)
 功能描述  : 设置WiFi发射速率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
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
                /* WIFI 20M n模式WL命令的速率值为0~7，共8个 */
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
    /*保存全局变量里，以备查询*/
    g_wlan_at_data.wifiRate = rate;
       
    return (AT_RETURN_SUCCESS);       
}


STATIC int32 AT_TransferWifiRate(uint32 ulInRate, uint32 *pulOutRate)
{
    uint8    ucIndex = 0;
    /* WIFI n模式 AT^WIDATARATE设置的速率值和WL命令速率值的对应表 */
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
    /*保存全局变量里，以备查询*/
    g_wlan_at_data.wifiPower = power_dBm_percent;
    return (AT_RETURN_SUCCESS);
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

/*****************************************************************************
 函数名称  : int32 WlanATSet2GWifiTX(WLAN_AT_FEATURE_TYPE onoff)
 功能描述  : 打开或关闭wifi发射机
 输入参数  : 0 关闭
             1 打开
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 WlanATSet2GWifiTX(WLAN_AT_FEATURE_TYPE onoff)
{
    int8   wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    int32 WifiPower = 0;

    if (AT_WIMODE_CW == g_wlan_at_data.wifiMode)
    {
        /* cw 测试 */
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

            msleep(DELAY_150_TIME); /* 自校准和发包间延时150毫秒，让校准生效，避免burst信号失真 */

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s pkteng_start 00:90:4c:21:00:8e tx 100 1500 0", IF_2G);
            WIFI_TEST_CMD(wl_cmd);
        } 
    }

    /*保存全局变量里，已备查询*/
    g_wlan_at_data.wifiTX = onoff;

    return (AT_RETURN_SUCCESS);
}

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
 函数名称  : WLAN_AT_FEATURE_TYPE WlanATSetWifiRX()
 功能描述  : 查询当前WiFi发射机状态信息
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 关闭发射机
             1 打开发射机
 其他说明  : 
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
                (g_wlan_at_data.wifiGroup % 2 + 1));   /*天线模式*/
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
            
            msleep(DELAY_150_TIME);  /* 自校准和收包间延时150毫秒，让校准生效 */

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
 函数名称  : WLAN_AT_FEATURE_TYPE WlanATSetWifiRX()
 功能描述  : 查询当前WiFi发射机状态信息
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 关闭发射机
             1 打开发射机
 其他说明  : 
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
 函数名称  : int32 WlanATGetWifiRX(WLAN_AT_WIRX_STRU *params)
 功能描述  : 获取wifi接收机的状态
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetWifiRX(WLAN_AT_WIRX_STRU *params)
{
    ASSERT_NULL_POINTER(params, AT_RETURN_FAILURE);

    memcpy(params, &g_wlan_at_data.wifiRX, sizeof(WLAN_AT_WIRX_STRU));
    
    return (AT_RETURN_SUCCESS);
}

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
    unsigned int ulUcastWifiRxPkts = 0;
    unsigned int ulMcastWifiRxPkts = 0;
    unsigned int ulWifiRxPkts = 0;
    unsigned int ulWifiMcastRxPkts = 0;

    int32 ret = AT_RETURN_SUCCESS; 
    ASSERT_NULL_POINTER(params, AT_RETURN_FAILURE);

    /* 判断接收机是否打开 */
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
/*(10)^WIINFO 查询WiFi的相关信息*/
//////////////////////////////////////////////////////////////////////////
#define SIZE_OF_INFOGROUP(group) (sizeof(group) / sizeof(WLAN_AT_WIINFO_MEMBER_STRU))
/*****************************************************************************
 函数名称  : uint32 ATGetWifiInfo(WLAN_AT_WIINFO_STRU *params)
 功能描述  : 查询WiFi的相关信息(内部接口)
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetWifiInfo(WLAN_AT_WIINFO_STRU *params)
{         
    static char sChannels24G[] = "3,7,11";

    /*信道信息*/
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
    
    /*功率信息*/
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

    /*频段信息:0表示2.4G,1表示5G*/  
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
 函数名称  : WLAN_AT_WIPLATFORM_TYPE WlanATGetWifiPlatform()
 功能描述  : 查询WiFi方案平台供应商信息
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 供应商类型
 其他说明  : 
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
 函数名称  : int32 WlanATSetTSELRF(uint32 group)
 功能描述  : 设置天线，非多通路传0
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 当前天线索引
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetTSELRF(void) /* 获取天线 */
{
    return g_wlan_at_data.wifiGroup;
}

/*****************************************************************************
 函数名称  : int32 WlanATSetTSELRF(uint32 group)
 功能描述  : 设置天线，非多通路传0
 输入参数  : strBuf 支持天线索引集合
 输出参数  : NA
 返 回 值  : AT_RETURN_SUCCESS 成功
             AT_RETURN_FAILURE 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetTSELRFSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    ASSERT_NULL_POINTER(strBuf, AT_RETURN_FAILURE);

    OSA_SNPRINTF(strBuf->buf, sizeof(strBuf->buf), "%s", WLAN_AT_TSELRF_SUPPORT);
    return (AT_RETURN_SUCCESS);
}

/*****************************************************************************
 函数名称  : int32 WlanATSetTSELRF(uint32 group)
 功能描述  : 设置天线，非多通路传0
 输入参数  : group 天线索引
 输出参数  : NA
 返 回 值  : AT_RETURN_SUCCESS 成功
             AT_RETURN_FAILURE 失败
 其他说明  : 
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
 函数名称  : int wifi_set_pa_mode(int wifiPaMode)
 功能描述  : 设置WiFi PA的增益情况
 输入参数  : wifiPaMode 增益模式
 输出参数  : NA
 返 回 值  : AT_RETURN_SUCCESS 成功
             AT_RETURN_FAILURE 失败
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
 输入参数  : pa_type  pa模式
 输出参数  : NA
 返 回 值  : AT_RETURN_SUCCESS 成功
             AT_RETURN_FAILURE 失败
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
 返 回 值  : AT_RETURN_SUCCESS 成功
             AT_RETURN_FAILURE 失败
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
 输入参数  : strBuf  支持的pa模式序列
 输出参数  : NA
 返 回 值  : AT_RETURN_SUCCESS 成功
             AT_RETURN_FAILURE 失败
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

/*****************************************************************************
 函数名称  : int32 WlanATGetWifi2GPavars(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 2.4G射频参数查询封装函数
 输入参数  : strBuf 
 输出参数  : NA
 返 回 值  : AT_RETURN_SUCCESS 成功
             AT_RETURN_FAILURE 失败
 其他说明  : 
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
 
    /*pavars命令执行结果*/
    if(0 == wlan_read_file("/var/tmpfile", filebuff, sizeof(filebuff)))
    {
        PLAT_WLAN_ERR("read_from_file fail");
        return AT_RETURN_FAILURE;
    }

    /*组装2.4g参数值，与5g显示格式一致*/
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
 函数名称  : int32 WlanATSetWifi2GPavars(WLAN_AT_PAVARS2G_STRU *strBuf)
 功能描述  : 2.4G射频参数设置封装函数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
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
 函数名称  : int32 GET_VALUE_FROM_NV(viod)
 功能描述  : 获取wifi的nv(50579,50580)中的值
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 GET_VALUE_FROM_NV(unsigned short type, unsigned int len,char *buf)
{
    unsigned short nv_id = type;  
    int32 ret = AT_RETURN_FAILURE;
    /* 读取nv数据 */
    ret = NV_Read(nv_id, buf, len);
    if (NV_OK != ret)
    {
        PLAT_WLAN_ERR("Error, nv read Failed! nv_id = %d, len = %d", nv_id, len);
        return AT_RETURN_FAILURE;
    }   
    return AT_RETURN_SUCCESS;
}
    
/*****************************************************************************
 函数名称  : int32 Check_WIFI_CAL_NV(viod)
 功能描述  : 检查wifi的nv(50579,50580)中的值
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
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
    
    /*2.4g配置校准参数*/
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

