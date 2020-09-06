
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

#define WL_CONFIG_EXE_PATH   "/system/bin/bcm4352/exe/wl " 
#define WIFI_INSMOD_SH        "/system/bin/bcm4352/exe/wifi_poweron_4352_AT.sh"

/*2.4g adapter interface*/
#define IF_2G   "-i wl1"
/*5g adapter interface*/
#define IF_5G   "-i wl0"

#define RF_2G_0   (0)
#define RF_2G_1   (1)
/*5g射频通路*/
#define RF_5G_0   (2)
#define RF_5G_1   (3)
#define RF_5G_2   (4)

/*MIMO*/
#define RF_2G_MIMO   (5)
#define RF_5G_MIMO   (6)

#define WL_CMD_BUF_LEN          128
#define WL_COUNTER_BUF_LEN      2048

#define WLAN_AT_2G_BAND    0
#define WLAN_AT_5G_BAND    1

#define WL_DEFAULT_POWER    (600) /* 默认功率 */

#define WLAN_AT_MODE_BCM4360_SUPPORT    "1,2,3,4,5"        /*支持的模式(a/b/g/n/ac)*/    
             
#define WLAN_AT_BCM4360_BAND_SUPPORT    "0,1,2"            /*支持的带宽(20M/40M/80M)*/

#define WLAN_AT_TSELRF_SUPPORT          "0,1,2,3,4"          /*支持的天线索引序列*/

#define WLAN_AT_GROUP_MAX               4                  /*支持的最大天线索引*/
#define WLAN_AT_TYPE_MAX                2                  /*支持获取的最大信息类型*/

/*WIFI功率的上下限*/
#define WLAN_AT_POWER_MIN               (-15)
#define WLAN_AT_POWER_MAX               (30)

/*WiFi增益模式*/
#define AT_WIFI_MODE_ONLY_PA            0x00  /*WIFI只支持PA模式*/
#define AT_WIFI_MODE_ONLY_NOPA          0x01  /*WIFI只支持NO PA模式*/
#define AT_WIFI_MODE_PA_NOPA            0x02  /*WIFI同时支持PA模式和NO PA模式*/

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

#define WLAN_FREQ_2G_MAX                2484                  /*2.4G最大频点*/

#define WLAN_FREQ_5G_W52_MIN            5180                  /*W52最小频点*/
#define WLAN_FREQ_5G_W53_MAX            5320                  /*W53最大频点*/

#define WLAN_FREQ_5G_W52_40M_MIN        5190                  /*W52 40M最小频点*/
#define WLAN_FREQ_5G_W53_40M_MAX        5310                  /*W53 40M最大频点*/

#define WLAN_FREQ_5G_W56_MIN            5500                  /*W56最小频点*/
#define WLAN_FREQ_5G_W56_MAX            5700                  /*W56最大频点*/

#define WLAN_FREQ_5G_W56_40M_MIN        5510                  /*W56 40M最小频点*/
#define WLAN_FREQ_5G_W56_40M_MAX        5670                  /*W56 40M最大频点*/

#define WLAN_FREQ_5G_W57_MIN            5745                  /*W57最小频点*/
#define WLAN_FREQ_5G_W57_MAX            5825                  /*W57最大频点*/

#define WLAN_FREQ_5G_W57_40M_MIN        5755                  /*W57最小频点*/
#define WLAN_FREQ_5G_W57_40M_MAX        5795                  /*W57最大频点*/

#define WIFI_CMD_MAX_SIZE               256                   /*cmd字符串256长度*/
#define WIFI_CMD_8_SIZE                 8                     /*cmd字符串8长度*/
#define HUNDRED                         100

#define DALEY_50_TIME     50
#define DALEY_100_TIME    100
#define DALEY_500_TIME    500
#define DALEY_1000_TIME   1000
#define DALEY_5000_TIME   5000

#define AT_WIFI_N_RATE_NUM          (16)
#define AT_WIFI_AC20_RATE_NUM     (9)
#define AT_WIFI_AC40_RATE_NUM     (10) 
#define AT_WIFI_AC80_RATE_NUM     (10)

#define RMMOD_TIME 20000
#define INSMOD_TIME 180000

#define VOS_OK                   (0)
#define VOS_ERROR           (-1)
#define VOS_TRUE               1L
#define VOS_FALSE              0L
#define STR_LEN_MAX       128

#define WIFI_CHARAC_NAME_LEN    (31)                      /* WIFI 特性名长度 */
#define  WIFI_WL_SW    ("WIFI_WL_REVERSE")
#define  WIFI_WL_KEEP  ("WIFI_WL_HOLD")

/*2.4G 频点定义 */
#define  FREQ_20M_2412    2412
#define  FREQ_20M_2417    2417
#define  FREQ_20M_2422    2422
#define  FREQ_20M_2427    2427
#define  FREQ_20M_2432    2432
#define  FREQ_20M_2437    2437
#define  FREQ_20M_2442    2442
#define  FREQ_20M_2447    2447
#define  FREQ_20M_2452    2452
#define  FREQ_20M_2457    2457
#define  FREQ_20M_2462    2462
#define  FREQ_20M_2467    2467
#define  FREQ_20M_2472    2472
#define  FREQ_20M_2484    2484

/* 5G 频点宏定义 */
#define  FREQ_20M_5180    5180
#define  FREQ_20M_5200    5200
#define  FREQ_20M_5220    5220
#define  FREQ_20M_5240    5240
#define  FREQ_20M_5260    5260
#define  FREQ_20M_5280    5280
#define  FREQ_20M_5300    5300
#define  FREQ_20M_5320    5320
#define  FREQ_20M_5500    5500
#define  FREQ_20M_5520    5520
#define  FREQ_20M_5540    5540
#define  FREQ_20M_5560    5560
#define  FREQ_20M_5580    5580
#define  FREQ_20M_5600    5600
#define  FREQ_20M_5620    5620
#define  FREQ_20M_5640    5640
#define  FREQ_20M_5660    5660
#define  FREQ_20M_5680    5680
#define  FREQ_20M_5700    5700
#define  FREQ_20M_5745    5745
#define  FREQ_20M_5765    5765
#define  FREQ_20M_5785    5785
#define  FREQ_20M_5805    5805
#define  FREQ_20M_5825    5825

#define  FREQ_40M_5190    5190
#define  FREQ_40M_5230    5230
#define  FREQ_40M_5270    5270
#define  FREQ_40M_5310    5310
#define  FREQ_40M_5510    5510
#define  FREQ_40M_5550    5550
#define  FREQ_40M_5590    5590
#define  FREQ_40M_5630    5630
#define  FREQ_40M_5670    5670
#define  FREQ_40M_5755    5755
#define  FREQ_40M_5795    5795

#define  FREQ_80M_5210    5210
#define  FREQ_80M_5290    5290
#define  FREQ_80M_5530    5530
#define  FREQ_80M_5610    5610
#define  FREQ_80M_5775    5775

/* Wi-Fi datarate define */
#define DATARATE_650        650
#define DATARATE_1300       1300
#define DATARATE_1350       1350
#define DATARATE_1950       1950
#define DATARATE_2600       2600
#define DATARATE_2700       2700
#define DATARATE_3900       3900
#define DATARATE_4050       4050
#define DATARATE_5200       5200
#define DATARATE_5400       5400
#define DATARATE_5850       5850
#define DATARATE_6500       6500
#define DATARATE_7800       7800
#define DATARATE_8100       8100
#define DATARATE_10400      10400
#define DATARATE_10800      10800
#define DATARATE_11700      11700
#define DATARATE_12150      12150
#define DATARATE_13000      13000
#define DATARATE_13500      13500
#define DATARATE_16200      16200
#define DATARATE_18000      18000
#define DATARATE_21600      21600
#define DATARATE_24300      24300
#define DATARATE_27000      27000
#define DATARATE_35100      35100
#define DATARATE_39000      39000

/* Wi-Fi power*/
#define WIFI_POWER_3175    3175

/* common number define */
#define WIFI_NUM_1    1
#define WIFI_NUM_2    2
#define WIFI_NUM_4    4
#define WIFI_NUM_20   20
#define WIFI_NUM_100  100

/* channel define */
#define CHANNEL_36    36
#define CHANNEL_64    64
#define CHANNEL_100   100
#define CHANNEL_124   124
#define CHANNEL_157   157

#define ANT_2G_NUM 2
#define ANT_5G_NUM 3
#define ANT_2G_CAL_NV_VALUE_BYTE_NUM 6
#define ANT_5G_CAL_NV_VALUE_BYTE_NUM 24

char    CMDValueStr[STR_LEN_MAX] = {0};
int      CMDRet = VOS_ERROR;

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
static unsigned int g_ul5GUP = 0;

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
}while(0)

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
        }while(0)

#define WIFI_TEST_CMD_NO_RETURN(cmd) do{ \
                    if (WLAN_SUCCESS != wlan_run_cmd(cmd)) \
                    { \
                        PLAT_WLAN_INFO("Run CMD Error!!\n"); \
                        if(WLAN_DRV_NO_LOADED == prepare_wifi_driver_loaded()) \
                        { \
                            if (WLAN_SUCCESS != wlan_run_cmd(cmd)) \
                            { \
                                return; \
                            } \
                        } \
                        else \
                        { \
                            return; \
                        } \
                    } \
                }while(0)

#define WIFI_SHELL_CMD_NO_RETURN(cmd) do { \
                            if (WLAN_SUCCESS != wlan_run_shell(cmd)) \
                            { \
                                PLAT_WLAN_INFO("Run shellcmd Error!!!"); \
                                if(WLAN_DRV_NO_LOADED == prepare_wifi_driver_loaded()) \
                                { \
                                    if (WLAN_SUCCESS != wlan_run_shell(cmd)) \
                                    { \
                                        return; \
                                    } \
                                } \
                                else \
                                { \
                                    return; \
                                } \
                            } \
                        }while(0)
                        
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
}while(0)


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
}WLAN_AT_GLOBAL_ST;

typedef struct tagWlanATPacketREP
{
    unsigned int TotalRxPkts;
    unsigned int BadRxPkts;
    unsigned int UcastRxPkts;
    unsigned int McastRxPkts;
}WLAN_AT_PACK_REP_ST;

typedef struct
{
    uint32 brate;
    int8 rate_str[WIFI_CMD_8_SIZE];
}BRATE_ST;

/*记录当前的WiFi模式，带宽，频率，速率等参数*/
STATIC WLAN_AT_GLOBAL_ST g_wlan_at_data = {AT_WIENABLE_TEST, AT_WIMODE_80211n, 
                    AT_WIBAND_20M, 0, {FREQ_20M_2412, 0}, DATARATE_6500, 
                    WIFI_POWER_3175, AT_FEATURE_DISABLE, 
                    {AT_FEATURE_DISABLE, {0}, {0}}, {0, 0}, 0};/*开启WiFi的默认参数*/

STATIC unsigned int g_ulUcastWifiRxPkts;
STATIC unsigned int g_ulMcastWifiRxPkts;
STATIC unsigned int g_ulRxState = 0;

/*****************************************************************************
 函数名称  : check_wifi_valid
 功能描述  : 查看wifi通路是否ok
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 check_wifi_valid(void)
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
#if (FEATURE_ON == MBB_PCIE_CUST_REVERSE)
    char wl_reverse[WIFI_CHARAC_NAME_LEN + 1] = {WIFI_WL_SW};
#else
    char wl_reverse[WIFI_CHARAC_NAME_LEN + 1] = {WIFI_WL_KEEP};
#endif

    memset(acCmdValueStr, 0, sizeof(acCmdValueStr));

    OSA_SNPRINTF(acCmdValueStr,sizeof(acCmdValueStr), WL_CONFIG_EXE_PATH" ver");
    if (WLAN_SUCCESS != wlan_run_cmd(acCmdValueStr)) 
    { 
        PLAT_WLAN_INFO("Wi-Fi driver is not loaded, now start to loaded Wi-Fi driver!\n"); 

        memset(acCmdValueStr, 0, sizeof(acCmdValueStr));
        OSA_SNPRINTF(acCmdValueStr, sizeof(acCmdValueStr), "/bin/sh %s %s",WIFI_INSMOD_SH, wl_reverse);
        wlan_run_cmd(acCmdValueStr);

        /*检查并配置新的wifi校准参数*/
        Check_WIFI_CAL_NV();
        
        return WLAN_DRV_NO_LOADED;
    }

    return WLAN_SUCCESS;
}


STATIC unsigned int GetCmdRetValue(char * pValueStr)
{
    char        *pcRecPcktsStart        = NULL;
    char         acMonitorStrValue[WIFI_CMD_MAX_SIZE] = {0};
    unsigned int uiRetPcktsNumBuf       = 0;
    unsigned int uiIdPcktsCntValue      = 0;
    char filebuff[WL_COUNTER_BUF_LEN + 1] = {0};

    /* 入参检测 */
    ASSERT_NULL_POINTER(pValueStr, AT_RETURN_FAILURE);

    if(0 == wlan_read_file("/var/wifirxpkt", filebuff, sizeof(filebuff)))
    {
        PLAT_WLAN_ERR("read_from_file fail");
        return AT_RETURN_FAILURE;
    }

    pcRecPcktsStart = strstr(filebuff, pValueStr);

    if(NULL == pcRecPcktsStart)
    {
        return AT_RETURN_FAILURE;
    }    

    pcRecPcktsStart = pcRecPcktsStart + strlen(pValueStr) + 1; /* 1:空格占位 */
    
    while(' ' != *pcRecPcktsStart)
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
    
    if(NULL == ucastPkts || NULL == mcastPkts)
    {
        return;
    }

    if (WLAN_AT_2G_BAND == g_wlan_at_data.wifiBand)
    {
        OSA_SNPRINTF(acCmdValueStr,sizeof(acCmdValueStr), 
            WL_CONFIG_EXE_PATH" %s counters > /var/wifirxpkt", IF_2G);
    }
    else if (WLAN_AT_5G_BAND == g_wlan_at_data.wifiBand)
    {
        OSA_SNPRINTF(acCmdValueStr,sizeof(acCmdValueStr), 
            WL_CONFIG_EXE_PATH" %s counters > /var/wifirxpkt", IF_5G);
    }
    else
    {
        return;
    }   
 
    WIFI_SHELL_CMD_NO_RETURN(acCmdValueStr);

    *mcastPkts = GetCmdRetValue("pktengrxdmcast");
    *ucastPkts = GetCmdRetValue("pktengrxducast");
    PLAT_WLAN_INFO("*mcastPkts=%d, *ucastPkts=%d \n", *mcastPkts, *ucastPkts);
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

                memset(wl_cmd, 0, WL_CMD_BUF_LEN);
                OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s down", IF_5G);
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
                OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s down", IF_5G);
                WIFI_TEST_CMD(wl_cmd);

                memset(wl_cmd, 0, WL_CMD_BUF_LEN);
                OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s up", IF_2G);
                WIFI_TEST_CMD(wl_cmd);

                memset(wl_cmd, 0, WL_CMD_BUF_LEN);
                OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s up", IF_5G);
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
                OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s down", IF_5G);
                WIFI_TEST_CMD(wl_cmd);

                memset(wl_cmd, 0, WL_CMD_BUF_LEN);
                OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s up", IF_2G);
                WIFI_TEST_CMD(wl_cmd);

                memset(wl_cmd, 0, WL_CMD_BUF_LEN);
                OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s up", IF_5G);
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
    PLAT_WLAN_INFO("WLAN_AT_WIMODE_TYPE:%d\n", mode);  

    if (mode > AT_WIMODE_80211ac)
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

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%s", WLAN_AT_MODE_BCM4360_SUPPORT);
    
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
STATIC int32 ATSetWifiBand(WLAN_AT_WIBAND_TYPE bandwith)
{
    int32 ret = AT_RETURN_SUCCESS;
    char wl_cmd[WL_CMD_BUF_LEN] = {0};
    
    PLAT_WLAN_INFO("%s:in,band = %d\n", __FUNCTION__, bandwith);
    
    switch(bandwith)
    {
        case AT_WIBAND_20M: 
        {
            g_wlan_at_data.wifiBandwith = AT_WIBAND_20M;
            break;
        }             
        case AT_WIBAND_40M:
        {                
            if(AT_WIMODE_80211n == g_wlan_at_data.wifiMode
            || AT_WIMODE_80211ac == g_wlan_at_data.wifiMode)
            {
                g_wlan_at_data.wifiBandwith = AT_WIBAND_40M;                    
            }
            else
            {   
                PLAT_WLAN_ERR("Error wifi mode,must in n/ac mode\n");
                ret = AT_RETURN_FAILURE;
            }
            break;
        }
        case AT_WIBAND_80M:
        {                
            if(AT_WIMODE_80211ac == g_wlan_at_data.wifiMode)
            {
                g_wlan_at_data.wifiBandwith = AT_WIBAND_80M;                    
            }
            else
            {   
                PLAT_WLAN_ERR("Error wifi mode,must in ac mode\n");
                ret = AT_RETURN_FAILURE;
            }
            break;
        }
        default:
        {
            ret = AT_RETURN_FAILURE;
            break;
        }
    }

    if (AT_RETURN_FAILURE == ret)
    {
        return ret;
    }

    if (WLAN_AT_2G_BAND == g_wlan_at_data.wifiBand)
    {
        memset(wl_cmd, 0, WL_CMD_BUF_LEN);
        OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s down", IF_2G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WL_CMD_BUF_LEN);
        OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, WL_CONFIG_EXE_PATH" %s mimo_txbw %d", 
            IF_2G, WIFI_NUM_2 * (bandwith + 1));
        WIFI_TEST_CMD(wl_cmd);
    }
    else
    {
         /* 5G在设置速率的wl命令里面下发bandwidth */
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

    OSA_SNPRINTF(strBuf->buf, WLAN_AT_BUFFER_SIZE, "%s", WLAN_AT_BCM4360_BAND_SUPPORT);
    
    return (AT_RETURN_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
/*(4)^WIFREQ 设置WiFi频点 */
//////////////////////////////////////////////////////////////////////////
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
    uint16 ulWifiFreq = 0;
    uint16 i = 0;    
    int32  ret = AT_RETURN_SUCCESS;
    
    /* 2.4G频点集合 信道1-14 */
    const uint16   ausChannels[] = {FREQ_20M_2412, FREQ_20M_2417, FREQ_20M_2422,
                                    FREQ_20M_2427, FREQ_20M_2432, FREQ_20M_2437,
                                    FREQ_20M_2442, FREQ_20M_2447, FREQ_20M_2452,
                                    FREQ_20M_2457, FREQ_20M_2462, FREQ_20M_2467,
                                    FREQ_20M_2472, FREQ_20M_2484}; 
    
    /* 2.4G 40M频点集合 中心信道 3-11 */
    const uint16   ausChannels_40M[] = {FREQ_20M_2422, FREQ_20M_2427, FREQ_20M_2432,
                                        FREQ_20M_2437, FREQ_20M_2442, FREQ_20M_2447,
                                        FREQ_20M_2452, FREQ_20M_2457, FREQ_20M_2462};
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

        if (!(((WLAN_CHANNEL_2G_MIN <= ulWifiFreq) 
        && (ulWifiFreq <= WLAN_CHANNEL_2G_MAX))))
        {
            PLAT_WLAN_INFO("Target Channel ERROR,ulWifiFreq = %u!\n", ulWifiFreq);
            return AT_RETURN_FAILURE;
        }
        
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s bw_cap 2g 1", IF_2G);
        WIFI_TEST_CMD(wl_cmd);
        
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE,
                WL_CONFIG_EXE_PATH" %s chanspec %d", IF_2G, ulWifiFreq);
        WIFI_TEST_CMD(wl_cmd);

    }
    else if(AT_WIBAND_40M == g_wlan_at_data.wifiBandwith)
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
    
        if (!(((WLAN_CHANNEL_2G_MIN <= ulWifiFreq) 
            && (WLAN_CHANNEL_2G_MAX >= ulWifiFreq))))
        {
            PLAT_WLAN_INFO("Target Channel ERROR!\n");
            return AT_RETURN_FAILURE;
        }
        
        /*40M信道加U/L处理*/
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
            WL_CONFIG_EXE_PATH" %s bw_cap 2g 3", IF_2G);
        WIFI_TEST_CMD(wl_cmd);
        
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
            WL_CONFIG_EXE_PATH" %s chanspec -c %d -b 2 -w 40 -s 1", 
            IF_2G, ulWifiFreq);
        WIFI_TEST_CMD(wl_cmd);
    }

    /* 保存全局变量里，以备查询 */
    g_wlan_at_data.wifiFreq.value = pFreq->value;
    g_wlan_at_data.wifiFreq.offset = pFreq->offset;
    return ret;

}

//////////////////////////////////////////////////////////////////////////
/*(4)^WIFREQ 设置WiFi频点 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSet5GWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
 功能描述  : 设置WiFi频点
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 WlanATSet5GWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
{
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    int8 channeStr[WIFI_CMD_8_SIZE] = {0}; 
    uint16 ulWifiFreq = 0;
    uint16 i = 0;    

    /*5G频点集合*/
    const uint16 aulChannel036[] = {FREQ_20M_5180, FREQ_20M_5200, FREQ_20M_5220, 
                                    FREQ_20M_5240, FREQ_20M_5260, FREQ_20M_5280,
                                    FREQ_20M_5300, FREQ_20M_5320};   
    const uint16 aulChannel100[] = {FREQ_20M_5500, FREQ_20M_5520, FREQ_20M_5540,
                                    FREQ_20M_5560, FREQ_20M_5580, FREQ_20M_5600,
                                    FREQ_20M_5620, FREQ_20M_5640, FREQ_20M_5660,
                                    FREQ_20M_5680, FREQ_20M_5700}; 
    const uint16 aulChannel149[] = {FREQ_20M_5745, FREQ_20M_5765, FREQ_20M_5785,
                                    FREQ_20M_5805, FREQ_20M_5825}; 

    /*5G 40M频点集合*/
    const uint16 aulChannel036_40M[] = {FREQ_40M_5190, FREQ_40M_5230, 
                                        FREQ_40M_5270, FREQ_40M_5310};
    const uint16 aulChannel100_40M[] = {FREQ_40M_5510, FREQ_40M_5550, 
                                        FREQ_40M_5590, FREQ_40M_5630,
                                        FREQ_40M_5670}; 
    const uint16 aulChannel149_40M[] = {FREQ_40M_5755, FREQ_40M_5795};

    /*5G 80M频点集合*/
    const uint16 aulChannelAC[] = {FREQ_80M_5210, FREQ_80M_5290, FREQ_80M_5530,
                                   FREQ_80M_5610, FREQ_80M_5775}; 

    /*5G 40M带宽需要加u的信道*/
    const uint16 channel_5g_40M_u[] = {40,48,56,64,104,112,120,128,136,153,161};    
    /*5G 40M带宽需要加l的信道*/
    const uint16 channel_5g_40M_l[] = {36,44,52,60,100,108,116,124,132,149,157};

    
    if (NULL == pFreq)
    {
        return (AT_RETURN_FAILURE);
    }

    if (g_ulRxState)
    {      
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s down", IF_5G);
        WIFI_TEST_CMD(wl_cmd);   
        g_ulRxState = 0;
    }

    /* 20M 带宽*/
    if (AT_WIBAND_20M == g_wlan_at_data.wifiBandwith)
    {
        if ((pFreq->value >= WLAN_FREQ_5G_W52_MIN) 
            && (pFreq->value <= WLAN_FREQ_5G_W53_MAX))
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
        else if ((pFreq->value >= WLAN_FREQ_5G_W56_MIN) 
                && (pFreq->value <= WLAN_FREQ_5G_W56_MAX))
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
        else if ((pFreq->value >= WLAN_FREQ_5G_W57_MIN) 
            && (pFreq->value <= WLAN_FREQ_5G_W57_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel149) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel149[i])
                {
                    ulWifiFreq = i * WLAN_CHANNEL_5G_INTERVAL \
                        + WLAN_CHANNEL_5G_W57_START;
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
    
        if (!((WLAN_CHANNEL_5G_MIN <= ulWifiFreq) 
            && (WLAN_CHANNEL_5G_MAX >= ulWifiFreq)))
        {
            PLAT_WLAN_INFO("Target Channel ERROR,ulWifiFreq = %u!\n", ulWifiFreq);
            return AT_RETURN_FAILURE;
        }

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH" %s down", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
            WL_CONFIG_EXE_PATH" %s phy_watchdog 0", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s band a", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s mpc 0", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s country ALL", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s scansuppress 1", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd,  WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s chanspec %d", IF_5G, ulWifiFreq);
        WIFI_TEST_CMD(wl_cmd);

    }
    else if (AT_WIBAND_40M == g_wlan_at_data.wifiBandwith)
    {
        if ((pFreq->value >= WLAN_FREQ_5G_W52_40M_MIN) 
            && (pFreq->value <= WLAN_FREQ_5G_W53_40M_MAX))
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
        else if ((pFreq->value >= WLAN_FREQ_5G_W56_40M_MIN) 
            && (pFreq->value <= WLAN_FREQ_5G_W56_40M_MAX))
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
        else if ((pFreq->value >= WLAN_FREQ_5G_W57_40M_MIN) 
            && (pFreq->value <= WLAN_FREQ_5G_W57_40M_MAX))
        {
            for (i = 0; i < (sizeof(aulChannel149_40M) / sizeof(uint16)); i++)
            {
                if (pFreq->value == aulChannel149_40M[i])
                {
                    ulWifiFreq = i * WLAN_CHANNEL_5G_40M_INTERVAL \ 
                                 + WLAN_CHANNEL_5G_W57_START;
                    break;
                }
            }
        }
        else
        {        
            PLAT_WLAN_INFO("Error 40M wifiFreq parameters\n");      
            return AT_RETURN_FAILURE;
        }

        PLAT_WLAN_INFO("Target Channel = %d\n", ulWifiFreq);
    
        /*40M信道加U/L处理*/
        if ((ulWifiFreq >= WLAN_CHANNEL_5G_MIN)
                && (ulWifiFreq <= WLAN_CHANNEL_5G_MAX))
        {            
            for (i = 0;i < (sizeof(channel_5g_40M_l) / sizeof(uint16));i++)
            {
                if(ulWifiFreq == channel_5g_40M_l[i])
                {
                    OSA_SNPRINTF(channeStr, sizeof(channeStr), "%dl", ulWifiFreq);                     
                    break;
                }                
            }

            if(i == (sizeof(channel_5g_40M_l) / sizeof(uint16)))
            {
                for (i = 0;i < (sizeof(channel_5g_40M_u) / sizeof(uint16));i++)
                {
                    if(ulWifiFreq == channel_5g_40M_u[i])
                    {
                        OSA_SNPRINTF(channeStr, sizeof(channeStr), "%du", ulWifiFreq);
                        break;
                    }                 
                } 
            }
            
            if(i == (sizeof(channel_5g_40M_u) / sizeof(uint16)))
            {
                OSA_SNPRINTF(channeStr, sizeof(channeStr), "%d", ulWifiFreq); 
            }
        }
        else
        {
            PLAT_WLAN_INFO("40M Channel Process ERROR!\n");
            return AT_RETURN_FAILURE;
        }       

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s down", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
            WL_CONFIG_EXE_PATH" %s phy_watchdog 0", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s band a", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s mpc 0", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
            WL_CONFIG_EXE_PATH" %s country ALL", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
            WL_CONFIG_EXE_PATH" %s scansuppress 1", IF_5G);
        WIFI_TEST_CMD(wl_cmd);
        
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd,  WIFI_CMD_MAX_SIZE, 
            WL_CONFIG_EXE_PATH" %s chanspec %d/40", IF_5G, ulWifiFreq);
        WIFI_TEST_CMD(wl_cmd);

    }
    else if (AT_WIBAND_80M == g_wlan_at_data.wifiBandwith)
    {
        for (i = 0; i < (sizeof(aulChannelAC) / sizeof(uint16)); i++)
        {
            if (pFreq->value == aulChannelAC[i])
            {
                ulWifiFreq = aulChannelAC[i];
                break;
            }
        }

        if (i == (sizeof(aulChannelAC) / sizeof(uint16)))
        {
            PLAT_WLAN_INFO("Error 80 wifiFreq parameters\n");      
            return AT_RETURN_FAILURE;
        }

        if (FREQ_80M_5210 == ulWifiFreq)
        {
            ulWifiFreq = CHANNEL_36;
        }
        else if (FREQ_80M_5290 == ulWifiFreq)
        {
            ulWifiFreq = CHANNEL_64;
        }
        else if (FREQ_80M_5530 == ulWifiFreq)
        {
            ulWifiFreq = CHANNEL_100;
        }
        else if (FREQ_80M_5610 == ulWifiFreq)
        {
            ulWifiFreq = CHANNEL_124;
        }
        else
        {
            ulWifiFreq = CHANNEL_157;
        }

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s down", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
            WL_CONFIG_EXE_PATH" %s phy_watchdog 0", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s band a", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s mpc 0", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
            WL_CONFIG_EXE_PATH" %s country ALL", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
            WL_CONFIG_EXE_PATH" %s scansuppress 1", IF_5G);
        WIFI_TEST_CMD(wl_cmd);

        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd,  WIFI_CMD_MAX_SIZE, 
            WL_CONFIG_EXE_PATH" %s chanspec %d/80", IF_5G, ulWifiFreq);
        WIFI_TEST_CMD(wl_cmd);
    }
    else
    {
        PLAT_WLAN_INFO("Error bandwidth!\n");      
        return AT_RETURN_FAILURE;
    }

    /* 保存全局变量里，以备查询 */
    g_wlan_at_data.wifiFreq.value = pFreq->value;
    g_wlan_at_data.wifiFreq.offset = pFreq->offset;
    return AT_RETURN_SUCCESS;

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
    if (NULL == pFreq)
    {
        return (AT_RETURN_FAILURE);
    }

    if (WLAN_AT_2G_BAND == g_wlan_at_data.wifiBand)
    {
        return WlanATSet2GWifiFreq(pFreq);
    }
    else if (WLAN_AT_5G_BAND == g_wlan_at_data.wifiBand)
    {
        return WlanATSet5GWifiFreq(pFreq);
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

//////////////////////////////////////////////////////////////////////////
/*(5)^WIDATARATE 设置和查询当前WiFi速率集速率
  WiFi速率，单位为0.01Mb/s，取值范围为0～65535 */
//////////////////////////////////////////////////////////////////////////
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
    //n ht20
    const uint32 wifi_20m_nrates_table[] = {DATARATE_650, DATARATE_1300, 
                                            DATARATE_1950, DATARATE_2600, 
                                            DATARATE_3900, DATARATE_5200,
                                            DATARATE_5850, DATARATE_6500};
    //n ht40 
    const uint32 wifi_40m_nrates_table[] = {DATARATE_1350, DATARATE_2700,
                                            DATARATE_4050, DATARATE_5400, 
                                            DATARATE_8100, DATARATE_10800,
                                            DATARATE_12150, DATARATE_13500};
    #define WIFI_20M_NRATES_TABLE_SIZE (sizeof(wifi_20m_nrates_table) / sizeof(uint32))
    #define WIFI_40M_NRATES_TABLE_SIZE (sizeof(wifi_40m_nrates_table) / sizeof(uint32))
    
    PLAT_WLAN_INFO("WifiRate = %u\n", (unsigned int)ulWifiRate);
    
    switch (g_wlan_at_data.wifiMode)
    {
        case AT_WIMODE_CW:
            PLAT_WLAN_INFO("AT_WIMODE_CW\n");
            return (AT_RETURN_FAILURE);

        case AT_WIMODE_80211b:
        case AT_WIMODE_80211g:
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s nrate -r %u", IF_2G, 
                        (unsigned int)ulWifiRate);
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
                        PLAT_WLAN_INFO("20M NRate Index = %u\n", (unsigned int)ulNRate);                        
                        break;
                    }
                }

                if (WIFI_20M_NRATES_TABLE_SIZE == ulNRate)
                {
                    PLAT_WLAN_INFO("20M NRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }

                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH" %s nrate -m %u", IF_2G, (unsigned int)ulNRate);
                WIFI_TEST_CMD(wl_cmd);
            }
            else if (AT_WIBAND_40M == g_wlan_at_data.wifiBandwith)
            {
                for (ulNRate = 0; ulNRate < WIFI_40M_NRATES_TABLE_SIZE; ulNRate++)
                {
                    if (wifi_40m_nrates_table[ulNRate] == rate)
                    {
                        PLAT_WLAN_INFO("40M NRate Index = %u\n", (unsigned int)ulNRate);
                        break;
                    }
                }

                if (WIFI_40M_NRATES_TABLE_SIZE == ulNRate)
                {
                    PLAT_WLAN_INFO("40M NRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }    
                
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH" %s nrate -m %u -s 3", 
                        IF_2G, (unsigned int)ulNRate);
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
    uint32  aucAtWifiN20Rate_Table[] = {DATARATE_650,  DATARATE_1300, DATARATE_1950,
                                        DATARATE_2600, DATARATE_3900, DATARATE_5200,
                                        DATARATE_5850, DATARATE_6500, DATARATE_1300,
                                        DATARATE_2600, DATARATE_3900, DATARATE_5200,
                                        DATARATE_7800, DATARATE_10400, DATARATE_11700,
                                        DATARATE_13000};

    uint32  aucAtWifiN40Rate_Table[] = {DATARATE_1350,  DATARATE_2700,  DATARATE_4050,
                                        DATARATE_5400,  DATARATE_8100,  DATARATE_10800,
                                        DATARATE_12150, DATARATE_13500, DATARATE_2700,
                                        DATARATE_5400,  DATARATE_8100,  DATARATE_10800,
                                        DATARATE_16200, DATARATE_21600, DATARATE_24300,
                                        DATARATE_27000};

    uint32  aucAtWifiAC20Rate_Table[] = {DATARATE_650,  DATARATE_1300, DATARATE_1950,
                                         DATARATE_2600, DATARATE_3900, DATARATE_5200,
                                         DATARATE_5850, DATARATE_6500, DATARATE_7800};

    uint32  aucAtWifiAC40Rate_Table[] = {DATARATE_650,  DATARATE_1300, DATARATE_1950,
                                         DATARATE_2600, DATARATE_3900, DATARATE_5200,
                                         DATARATE_5850, DATARATE_6500, DATARATE_16200,
                                         DATARATE_18000};

    uint32  aucAtWifiAC80Rate_Table[] = {DATARATE_650,  DATARATE_1300, DATARATE_1950,
                                         DATARATE_2600, DATARATE_3900, DATARATE_5200,
                                         DATARATE_5850, DATARATE_6500, DATARATE_35100,
                                         DATARATE_39000};

    if(AT_WIBAND_20M == g_wlan_at_data.wifiBandwith)
    {

        if(RF_2G_0 == g_ulWifiRF || RF_2G_1 == g_ulWifiRF)
        {
            for (ucIndex = 0; ucIndex < AT_WIFI_N_RATE_NUM / WIFI_NUM_2; ucIndex++)
            {
                if (aucAtWifiN20Rate_Table[ucIndex] == ulInRate)
                {
                    *pulOutRate = ucIndex;
                    break;
                }
            }

            if (ucIndex >= AT_WIFI_N_RATE_NUM / WIFI_NUM_2)
            {
                return AT_RETURN_FAILURE;
            }
        }
        else if(RF_5G_0 == g_ulWifiRF || RF_5G_1 == g_ulWifiRF || RF_5G_2 == g_ulWifiRF)
        {
            for (ucIndex = 0; ucIndex < AT_WIFI_AC20_RATE_NUM; ucIndex++)
            {
                if (aucAtWifiAC20Rate_Table[ucIndex] == ulInRate)
                {
                    *pulOutRate = ucIndex;
                    break;
                }
            }
            if (ucIndex >= AT_WIFI_AC20_RATE_NUM)
            {
                return AT_RETURN_FAILURE;
            }
        }
        else if(RF_2G_MIMO == g_ulWifiRF || RF_5G_MIMO == g_ulWifiRF)
        {
            for (ucIndex = AT_WIFI_N_RATE_NUM / WIFI_NUM_2; \
                ucIndex < AT_WIFI_N_RATE_NUM; ucIndex++)
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
    else if(AT_WIBAND_40M == g_wlan_at_data.wifiBandwith)
    { 
        if(RF_2G_0 == g_ulWifiRF || RF_2G_1 == g_ulWifiRF)
        {
            for (ucIndex = 0; ucIndex < AT_WIFI_N_RATE_NUM / WIFI_NUM_2; ucIndex++)
            {
                if (aucAtWifiN40Rate_Table[ucIndex] == ulInRate)
                {
                    *pulOutRate = ucIndex;
                    break;
                }
            }
            if (ucIndex >= AT_WIFI_N_RATE_NUM / WIFI_NUM_2)
            {
                return AT_RETURN_FAILURE;
            }
        }
        else if(RF_5G_0 == g_ulWifiRF || RF_5G_1 == g_ulWifiRF || RF_5G_2 == g_ulWifiRF)
        {
            if(AT_WIMODE_80211n == g_wlan_at_data.wifiMode)
            {
                for (ucIndex = 0; ucIndex < AT_WIFI_N_RATE_NUM ; ucIndex++)
                {
                    if (aucAtWifiN40Rate_Table[ucIndex] == ulInRate)
                    {
                        *pulOutRate = ucIndex;
                        break;
                    }
                }
                if (ucIndex >= AT_WIFI_N_RATE_NUM )
                {
                    return AT_RETURN_FAILURE;
                }
            }
            else
            {
                for (ucIndex = 0; ucIndex < AT_WIFI_AC40_RATE_NUM; ucIndex++)
                {
                    if (aucAtWifiAC40Rate_Table[ucIndex] == ulInRate)
                    {
                        *pulOutRate = ucIndex;
                        break;
                    }
                }
                if (ucIndex >= AT_WIFI_AC40_RATE_NUM)
                {
                    return AT_RETURN_FAILURE;
                }

            }
        }
        else if(RF_2G_MIMO == g_ulWifiRF || RF_5G_MIMO == g_ulWifiRF)
        {
            for (ucIndex = AT_WIFI_N_RATE_NUM / WIFI_NUM_2; \
                ucIndex < AT_WIFI_N_RATE_NUM; ucIndex++)
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
    else if(AT_WIBAND_80M == g_wlan_at_data.wifiBandwith)
    {
        if(RF_5G_0 == g_ulWifiRF || RF_5G_1 == g_ulWifiRF || RF_5G_2 == g_ulWifiRF)
        {
            for (ucIndex = 0; ucIndex < AT_WIFI_AC80_RATE_NUM; ucIndex++)
            {
                if (aucAtWifiAC80Rate_Table[ucIndex] == ulInRate)
                {
                    *pulOutRate = ucIndex;
                    break;
                }
            }

            if (ucIndex >= AT_WIFI_AC80_RATE_NUM)
            {
                return AT_RETURN_FAILURE;
            }
        }
    }

    return AT_RETURN_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
/*(5)^WIDATARATE 设置和查询当前WiFi速率集速率
  WiFi速率，单位为0.01Mb/s，取值范围为0～65535 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSet2GWifiDataRate(uint32 rate)
 功能描述  : 设置WiFi发射速率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 WlanATSet5GWifiDataRate(uint32 rate)
{
    uint8     wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    uint32   ulWifiRate = rate / HUNDRED;
    uint32   ulNRate;
    int32     ulRslt;
  
    if(WLAN_AT_5G_BAND == g_wlan_at_data.wifiBand)
    {
        switch(g_wlan_at_data.wifiMode)
        {
            case AT_WIMODE_CW:
            {
                PLAT_WLAN_INFO("AT_WIMODE_CW\n");
                return (AT_RETURN_FAILURE);
            }
            case AT_WIMODE_80211a:
            {
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE,
                        WL_CONFIG_EXE_PATH" %s 5g_rate -r %u -b %u", IF_5G, 
                        (unsigned int)ulWifiRate, 
                (unsigned int)((g_wlan_at_data.wifiBandwith + WIFI_NUM_1) * WIFI_NUM_20));
                WIFI_TEST_CMD(wl_cmd);
                break;   
            }
            case AT_WIMODE_80211n:
            {
                ulRslt = AT_TransferWifiRate(rate, &ulNRate);
                if(AT_RETURN_SUCCESS == ulRslt)
                {
                    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                                WL_CONFIG_EXE_PATH" %s 5g_rate -h %u -b %u", IF_5G, 
                                (unsigned int)ulNRate, 
                                (unsigned int)\
                                ((g_wlan_at_data.wifiBandwith + WIFI_NUM_1)\
                                * WIFI_NUM_20));
                    WIFI_TEST_CMD(wl_cmd);

                    g_ulWifiMCSIndex = ulNRate;
                }
                else
                {
                    PLAT_WLAN_INFO("5G NRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }
                break;
            }
            case AT_WIMODE_80211ac:
            {
                ulRslt = AT_TransferWifiRate(rate, &ulNRate);
                if(AT_RETURN_SUCCESS == ulRslt)
                {
                    if(AT_WIBAND_80M == g_wlan_at_data.wifiBandwith)
                    {
                        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                            WL_CONFIG_EXE_PATH" %s 5g_rate -v %u -s 1 -b 80", 
                            IF_5G, (unsigned int)ulNRate);
                    }
                    else
                    {
                        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                                WL_CONFIG_EXE_PATH" %s 5g_rate -v %u -s 1 -b %u", IF_5G, 
                                (unsigned int)ulNRate, 
                                (unsigned int)\
                                ((g_wlan_at_data.wifiBandwith + WIFI_NUM_1)\
                                * WIFI_NUM_20));
                    }

                    WIFI_TEST_CMD(wl_cmd);
                }
                else
                {
                    PLAT_WLAN_INFO("5G ACRate Error!\n");
                    return (AT_RETURN_FAILURE);
                }
                break;
            }
            default:
            {
                PLAT_WLAN_INFO("5G Mode Error!\n");
                return (AT_RETURN_FAILURE);
            }
        }
    }
    else
    {
        PLAT_WLAN_INFO("Band Error!\n");
        return (AT_RETURN_FAILURE);
    }
       
    g_wlan_at_data.wifiRate = rate;

    return AT_RETURN_SUCCESS;
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

    if (WLAN_AT_2G_BAND == g_wlan_at_data.wifiBand)
    {
        return WlanATSet2GWifiDataRate(rate);
    }
    else if (WLAN_AT_5G_BAND == g_wlan_at_data.wifiBand)
    {
        return WlanATSet5GWifiDataRate(rate);
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

//////////////////////////////////////////////////////////////////////////
/*(7)^WITX 来设置WiFi发射机开关 */
//////////////////////////////////////////////////////////////////////////
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
        if(AT_FEATURE_DISABLE == onoff)
        {
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH" %s pkteng_stop tx", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH " %s down", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH " %s txchain 3", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH " %s up", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH " %s phy_watchdog 1", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

        }
        else
        {
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s down", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s txchain 3", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s up", IF_2G);
            WIFI_TEST_CMD(wl_cmd);
            
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s ssid \"\"", IF_2G);
            WIFI_SHELL_CMD(wl_cmd);

            if(RF_2G_0 == g_ulWifiRF || RF_2G_1 == g_ulWifiRF)
            {
                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH" %s txant %d",  IF_2G,  g_ulWifiRF);
                WIFI_TEST_CMD(wl_cmd);

                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH" %s antdiv %d",  IF_2G,  g_ulWifiRF);
                WIFI_TEST_CMD(wl_cmd);
            }

            WifiPower = g_wlan_at_data.wifiPower / WIFI_NUM_100;
            
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s txpwr1 -o -q %ld", IF_2G, 
                WifiPower * WIFI_NUM_4);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s phy_forcecal 1", IF_2G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s pkteng_start 00:90:4c:21:00:8e tx 100 1500 0", 
                IF_2G);
            WIFI_TEST_CMD(wl_cmd);
        } 
    }
    
    /*保存全局变量里，已备查询*/
    g_wlan_at_data.wifiTX = onoff;
     
    return (AT_RETURN_SUCCESS);

}

//////////////////////////////////////////////////////////////////////////
/*(7)^WITX 来设置WiFi发射机开关 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSet5GWifiTX(WLAN_AT_FEATURE_TYPE onoff)
 功能描述  : 打开或关闭wifi发射机
 输入参数  : 0 关闭
             1 打开
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
STATIC int32 WlanATSet5GWifiTX(WLAN_AT_FEATURE_TYPE onoff)
{
    int8   wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    int32 WifiPower = 0;
    
    if (AT_WIMODE_CW == g_wlan_at_data.wifiMode)
    {
        /* cw 测试 */
    }
    else 
    {
        if(AT_FEATURE_DISABLE == onoff)
        {
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s pkteng_stop tx", IF_5G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH " %s down", IF_5G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH " %s phy_watchdog 1", IF_5G);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH " %s up", IF_5G);
            WIFI_TEST_CMD(wl_cmd);

        }
        else
        {            
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s down", IF_5G);
            WIFI_TEST_CMD(wl_cmd);

            if(RF_5G_0 == g_wlan_at_data.wifiGroup)
            {
                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH" %s txchain 1", IF_5G);
                WIFI_TEST_CMD(wl_cmd);

                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s rxchain 1", IF_5G);
                WIFI_TEST_CMD(wl_cmd);
            }
            else if (RF_5G_1 == g_wlan_at_data.wifiGroup)
            {
                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH" %s txchain 2", IF_5G);
                WIFI_TEST_CMD(wl_cmd);

                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH" %s rxchain 2", IF_5G);
                WIFI_TEST_CMD(wl_cmd);
            }
            else 
            {
                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH" %s txchain 4", IF_5G);
                WIFI_TEST_CMD(wl_cmd);

                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s rxchain 4", IF_5G);
                WIFI_TEST_CMD(wl_cmd);
            }

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s up", IF_5G);
            WIFI_TEST_CMD(wl_cmd);

           

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s ssid \"\"", IF_5G);
            WIFI_SHELL_CMD(wl_cmd);

            WifiPower = g_wlan_at_data.wifiPower / WIFI_NUM_100;

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s txpwr1 -o -q %ld", 
                IF_5G, WifiPower * WIFI_NUM_4);
            WIFI_TEST_CMD(wl_cmd);

            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s phy_forcecal 1", IF_5G);
            WIFI_TEST_CMD(wl_cmd);

            if(AT_WIMODE_80211ac ==  g_wlan_at_data.wifiMode &&
                (AT_WIBAND_40M == g_wlan_at_data.wifiBandwith 
                || AT_WIBAND_80M == g_wlan_at_data.wifiBandwith))
            {
                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH" %s pkteng_start 00:90:4c:21:00:8e tx 30 1500 0", 
                    IF_5G);
                WIFI_TEST_CMD(wl_cmd);
            }
            else
            {
                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                    WL_CONFIG_EXE_PATH" %s pkteng_start 00:90:4c:21:00:8e tx 100 1500 0", 
                    IF_5G);
                WIFI_TEST_CMD(wl_cmd);
            }
        } 
    }

    /*保存全局变量里，已备查询*/
    g_wlan_at_data.wifiTX = onoff;
     
    return (AT_RETURN_SUCCESS);
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
    if (WLAN_AT_2G_BAND == g_wlan_at_data.wifiBand)
    {
        return WlanATSet2GWifiTX(onoff);
    }
    else if (WLAN_AT_5G_BAND == g_wlan_at_data.wifiBand)
    {
        return WlanATSet5GWifiTX(onoff);
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
//////////////////////////////////////////////////////////////////////////
/*(8)^WIRX 设置WiFi接收机开关 */
//////////////////////////////////////////////////////////////////////////
STATIC WLAN_AT_FEATURE_TYPE WlanATSet2GWifiRX(WLAN_AT_WIRX_STRU *params)
{
    int8    wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    ASSERT_NULL_POINTER(params, AT_RETURN_FAILURE);
        
    switch (params->onoff)
    {
        case AT_FEATURE_DISABLE:
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s pkteng_stop rx", IF_2G);
            WIFI_TEST_CMD(wl_cmd);      
            
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s down", IF_2G);
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
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), 
                WL_CONFIG_EXE_PATH" %s rxchain %u", IF_2G, 
                (unsigned int)(g_wlan_at_data.wifiGroup % 2 + 1));   /*天线模式*/
            WIFI_TEST_CMD(wl_cmd);
            
            memset(wl_cmd, 0, WL_CMD_BUF_LEN);
            OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, 
                WL_CONFIG_EXE_PATH" %s ssid \"\"", IF_2G);
            WIFI_SHELL_CMD(wl_cmd);
            
            memset(wl_cmd, 0, WL_CMD_BUF_LEN);
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), WL_CONFIG_EXE_PATH"%s up", IF_2G);
            WIFI_TEST_CMD(wl_cmd);
            
            memset(wl_cmd, 0, WL_CMD_BUF_LEN);
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), 
                WL_CONFIG_EXE_PATH"%s phy_forcecal 1", IF_2G);
            WIFI_TEST_CMD(wl_cmd);
            
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), 
                WL_CONFIG_EXE_PATH" %s pkteng_start %s rx", IF_2G, params->src_mac);
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
 函数名称  : WLAN_AT_FEATURE_TYPE WlanATSet5GWifiRX()
 功能描述  : 查询当前WiFi发射机状态信息
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 关闭发射机
             1 打开发射机
 其他说明  : 
*****************************************************************************/
//////////////////////////////////////////////////////////////////////////
/*(8)^WIRX 设置WiFi接收机开关 */
//////////////////////////////////////////////////////////////////////////
STATIC WLAN_AT_FEATURE_TYPE WlanATSet5GWifiRX(WLAN_AT_WIRX_STRU *params)
{
    int8    wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    ASSERT_NULL_POINTER(params, AT_RETURN_FAILURE);
        
    switch (params->onoff)
    {
        case AT_FEATURE_DISABLE:
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s pkteng_stop rx", IF_5G);
            WIFI_TEST_CMD(wl_cmd);      
            
            memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
            OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
                WL_CONFIG_EXE_PATH" %s down", IF_5G);
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
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), 
                WL_CONFIG_EXE_PATH"%s phy_watchdog 0", IF_5G);
            WIFI_TEST_CMD(wl_cmd);
            
            memset(wl_cmd, 0, WL_CMD_BUF_LEN);
            if(RF_5G_0 == g_wlan_at_data.wifiGroup)
            {
                OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), 
                WL_CONFIG_EXE_PATH" %s rxchain 1", IF_5G); /*天线模式*/
                WIFI_TEST_CMD(wl_cmd);
            }
            else if(RF_5G_1 == g_wlan_at_data.wifiGroup)
            {
                OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), 
                WL_CONFIG_EXE_PATH" %s rxchain 2", IF_5G);  /*天线模式*/
                WIFI_TEST_CMD(wl_cmd);
            }
            else if(RF_5G_2 == g_wlan_at_data.wifiGroup)
            {
                OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), 
                WL_CONFIG_EXE_PATH" %s rxchain 4", IF_5G);  /*天线模式*/
                WIFI_TEST_CMD(wl_cmd);
            }
            
            memset(wl_cmd, 0, WL_CMD_BUF_LEN);
            OSA_SNPRINTF(wl_cmd, WL_CMD_BUF_LEN, 
                WL_CONFIG_EXE_PATH" %s ssid \"\"", IF_5G);
            WIFI_SHELL_CMD(wl_cmd);
            
            memset(wl_cmd, 0, WL_CMD_BUF_LEN);
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), 
                WL_CONFIG_EXE_PATH"%s up", IF_5G);
            WIFI_TEST_CMD(wl_cmd);
            
            memset(wl_cmd, 0, WL_CMD_BUF_LEN);
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), 
                WL_CONFIG_EXE_PATH"%s phy_forcecal 1", IF_5G);
            WIFI_TEST_CMD(wl_cmd);
            
            OSA_SNPRINTF(wl_cmd, sizeof(wl_cmd), 
                WL_CONFIG_EXE_PATH" %s pkteng_start %s rx", 
                IF_5G, params->src_mac);
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
//////////////////////////////////////////////////////////////////////////
/*(8)^WIRX 设置WiFi接收机开关 */
//////////////////////////////////////////////////////////////////////////
STATIC int32 ATSetWifiRX(WLAN_AT_WIRX_STRU *params)
{
    if (WLAN_AT_2G_BAND == g_wlan_at_data.wifiBand)
    {
        return WlanATSet2GWifiRX(params);
    }
    else if (WLAN_AT_5G_BAND == g_wlan_at_data.wifiBand)
    {
        return WlanATSet5GWifiRX(params);
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
    ASSERT_WiFi_OFF(AT_RETURN_FAILURE);

    if (0 != flag)
    {
        PLAT_WLAN_INFO("Exit on flag = %ld\n", flag);
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
    if(AT_FEATURE_DISABLE == g_wlan_at_data.wifiRX.onoff)
    {
        PLAT_WLAN_INFO("Not Rx Mode.\n");
        ret = AT_RETURN_FAILURE;
        return ret;
    }   
    
    wifi_get_rx_packet_report(&ulUcastWifiRxPkts,  &ulMcastWifiRxPkts);
    ulWifiRxPkts = (ulUcastWifiRxPkts - g_ulUcastWifiRxPkts);
    ulWifiMcastRxPkts = (ulMcastWifiRxPkts - g_ulMcastWifiRxPkts);
    if(ulWifiMcastRxPkts > ulWifiRxPkts)
    {
        ulWifiRxPkts = ulWifiMcastRxPkts;
    }
   
    params->good_result = ulWifiRxPkts;
    params->bad_result = 0;   
    
    PLAT_WLAN_INFO("Exit [good = %d, bad = %d]\n", 
        params->good_result, params->bad_result);   
    
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
    static char sChannels5G[] = "36,40,44,48,52,56,60,64,100, \
                                 104,108,112,116,120,124,128, \
                                 132,136,140,149,153,157,161,165";   
    static char sChannels5G_ac4[] = "36,38,44,52,60,62,100,102,108,116,118,124,132,140,149,157";   
    static char sChannels5G_ac8[] = "36,42,52,58,100,106,116,122,132,149";   
    
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
    static WLAN_AT_WIINFO_MEMBER_STRU sChannelGroup2[] = 
    {       
        {"a", sChannels5G},
        {"n", sChannels5G},
        {"n4", sChannels5G_ac4},
        {"ac", sChannels5G},
        {"ac4", sChannels5G_ac4},
        {"ac8", sChannels5G_ac8}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sChannelGroup3[] = 
    {               
        {"a", sChannels5G},
        {"n", sChannels5G},
        {"n4", sChannels5G_ac4},
        {"ac", sChannels5G},
        {"ac4", sChannels5G_ac4},
        {"ac8", sChannels5G_ac8}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sChannelGroup4[] = 
    {               
        {"a", sChannels5G},
        {"n", sChannels5G},
        {"n4", sChannels5G_ac4},
        {"ac", sChannels5G},
        {"ac4", sChannels5G_ac4},
        {"ac8", sChannels5G_ac8}
    };

    /*功率信息*/
    static WLAN_AT_WIINFO_MEMBER_STRU sPowerGroup0[] =
    {      
        {"b", "160"},
        {"g", "170"},
        {"n", "170"},
        {"n4","170"}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sPowerGroup1[] =
    {            
        {"b", "160"},
        {"g", "170"},
        {"n", "170"},
        {"n4","170"}
    };
    static  WLAN_AT_WIINFO_MEMBER_STRU sPowerGroup2[] =
    {             
        {"a",   "160"},
        {"n",   "160"},
        {"n4",  "160"},
        {"ac",  "160"},
        {"ac4", "160"},
        {"ac8", "160"}
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sPowerGroup3[] =
    {     
        {"a",   "160"},
        {"n",   "160"},
        {"n4",  "160"},
        {"ac",  "160"},
        {"ac4", "160"},
        {"ac8", "160"}

    };
    static WLAN_AT_WIINFO_MEMBER_STRU sPowerGroup4[] =
    {     
        {"a",   "160"},
        {"n",   "160"},
        {"n4",  "160"},
        {"ac",  "160"},
        {"ac4", "160"},
        {"ac8", "160"}

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
    static WLAN_AT_WIINFO_MEMBER_STRU sFreqGroup2[] =
    {              
        {"a",   "1"},
        {"n",   "1"},
        {"n4",  "1"},
        {"ac",  "1"},
        {"ac4", "1"},
        {"ac8", "1"}    
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sFreqGroup3[] =
    {         
        {"a",   "1"},
        {"n",   "1"},
        {"n4",  "1"},
        {"ac",  "1"},
        {"ac4", "1"},
        {"ac8", "1"}    
    };
    static WLAN_AT_WIINFO_MEMBER_STRU sFreqGroup4[] =
    {         
        {"a",   "1"},
        {"n",   "1"},
        {"n4",  "1"},
        {"ac",  "1"},
        {"ac4", "1"},
        {"ac8", "1"}    
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

    static WLAN_AT_WIINFO_GROUP_STRU sInfoGroup2[] =
    {
        {sChannelGroup2, SIZE_OF_INFOGROUP(sChannelGroup2)},
        {sPowerGroup2, SIZE_OF_INFOGROUP(sPowerGroup2)},
        {sFreqGroup2, SIZE_OF_INFOGROUP(sFreqGroup2)}
   
    };

    static WLAN_AT_WIINFO_GROUP_STRU sInfoGroup3[] =
    {
        {sChannelGroup3, SIZE_OF_INFOGROUP(sChannelGroup3)},
        {sPowerGroup3, SIZE_OF_INFOGROUP(sPowerGroup3)},
        {sFreqGroup3, SIZE_OF_INFOGROUP(sFreqGroup3)}
    };

    static WLAN_AT_WIINFO_GROUP_STRU sInfoGroup4[] =
    {
        {sChannelGroup4, SIZE_OF_INFOGROUP(sChannelGroup4)},
        {sPowerGroup4, SIZE_OF_INFOGROUP(sPowerGroup4)},
        {sFreqGroup4, SIZE_OF_INFOGROUP(sFreqGroup4)}
    };

    static WLAN_AT_WIINFO_GROUP_STRU *sTotalInfoGroups[] =
    {
        sInfoGroup0,
        sInfoGroup1,
        sInfoGroup2,
        sInfoGroup3,
        sInfoGroup4
    };
   
    char *strBuf = NULL;
    int32 idx = 0, iLen = 0, igroup = 0,itype = 0, iTmp = 0;
    WLAN_AT_WIINFO_GROUP_STRU *pstuInfoGrup = NULL;
    WLAN_AT_WIINFO_GROUP_STRU *pstuInfoType = NULL;
    
    ASSERT_NULL_POINTER(params, AT_RETURN_FAILURE);
    PLAT_WLAN_INFO("Enter ATGetWifiInfo [group=%ld,type=%u]\n", 
        params->member.group, params->type);

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


STATIC void  AT_InitWiFiBCM2G(void)
{
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    
    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s down", IF_2G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s country ALL", IF_2G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s mpc 0", IF_2G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
        WL_CONFIG_EXE_PATH" %s interference 0", IF_2G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
        WL_CONFIG_EXE_PATH" %s phy_watchdog 0", IF_2G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
        WL_CONFIG_EXE_PATH" %s scansuppress 1", IF_2G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
        WL_CONFIG_EXE_PATH" %s frameburst 0", IF_2G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
        WL_CONFIG_EXE_PATH" %s ampdu 0", IF_2G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
        WL_CONFIG_EXE_PATH" %s mimo_bw_cap 1", IF_2G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
        WL_CONFIG_EXE_PATH" %s bi 65535", IF_2G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
        WL_CONFIG_EXE_PATH" %s up", IF_2G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    if (0 == g_ul2GUP)
    {
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, "/system/bin/ifconfig wl1 up");
        WIFI_SHELL_CMD_NO_RETURN(wl_cmd);

        g_ul2GUP = 1;
    }
}


STATIC void  AT_InitWiFiBCM5G(void)
{
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
        
    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s down", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s mpc 0", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s country ALL", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s wsec 0", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s stbc_tx 0", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s stbc_rx 1", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, 
        WL_CONFIG_EXE_PATH" %s scansuppress 1", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s txbf 0", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s spect 0", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s bw_cap 5g 7", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WL_CMD_BUF_LEN);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s mbss 0", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s frameburst 1", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s ampdu 1", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s bi 65535", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s up", IF_5G);
    WIFI_TEST_CMD_NO_RETURN(wl_cmd);

    if (0 == g_ul5GUP)
    {
        memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
        OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, "/system/bin/ifconfig wl0 up");
        WIFI_SHELL_CMD_NO_RETURN(wl_cmd);

        g_ul5GUP = 1;
    }
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
STATIC int32 ATGetTSELRF(void) /* 获取天线 */
{
    return g_wlan_at_data.wifiGroup;
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
STATIC int32 ATGetTSELRFSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    ASSERT_NULL_POINTER(strBuf, AT_RETURN_FAILURE);

    OSA_SNPRINTF(strBuf->buf, sizeof(strBuf->buf), "%s", WLAN_AT_TSELRF_SUPPORT);
    return (AT_RETURN_SUCCESS);
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
    if(WLAN_AT_GROUP_MAX < group)
    {
        return AT_RETURN_FAILURE;
    }
     
    g_wlan_at_data.wifiGroup = group;
    g_ulWifiRF = group;

    if (g_wlan_at_data.wifiGroup == RF_2G_0 
     || g_wlan_at_data.wifiGroup == RF_2G_1)
    {
        g_wlan_at_data.wifiBand = WLAN_AT_2G_BAND;
    }
    else if (g_wlan_at_data.wifiGroup == RF_5G_0 
      || g_wlan_at_data.wifiGroup == RF_5G_1
      || g_wlan_at_data.wifiGroup == RF_5G_2)
    {
        g_wlan_at_data.wifiBand = WLAN_AT_5G_BAND;
    }
    else
    {
        PLAT_WLAN_INFO("Set incorrect TSELRF group %d\n", g_ulWifiRF);
        return AT_RETURN_FAILURE;
    }
    
    PLAT_WLAN_INFO("[%s]:Enter,group = %u\n", __FUNCTION__, (unsigned int)group); 
    
    if ( g_wlan_at_data.wifiBand == WLAN_AT_2G_BAND )
    {
        AT_InitWiFiBCM2G();
    }
    else
    {
        AT_InitWiFiBCM5G();
    }
     
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

/*****************************************************************************
 函数名称  : int32 WlanATGetWifi2GPavars(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 2.4G射频参数查询封装函数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetWifi2GPavars(WLAN_AT_BUFFER_STRU *strBuf)
{
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    int8 filebuff[WL_COUNTER_BUF_LEN + 1] = {0};
    int8 foramtdata2g[2][8] ={"pa2ga0=","pa2ga1="};
    int8 realdata2g[6][9]   ={"pa2gw0a0","pa2gw1a0","pa2gw2a0","pa2gw0a1","pa2gw1a1","pa2gw2a1"};
    int32 i,j;
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
    for ( i = 0, j=0 ; i < 2 ; i++ )
    {
        strcat(strBuf->buf, foramtdata2g[i]);

        temp = strstr(filebuff, realdata2g[j++]);
        temp = temp + 9;
        strncat(strBuf->buf, temp, 6);
        strcat(strBuf->buf, ",");

        temp = strstr(filebuff, realdata2g[j++]);
        temp = temp + 9;
        strncat(strBuf->buf, temp, 6);
        strcat(strBuf->buf, ",");

        temp =strstr(filebuff, realdata2g[j++]);
        temp = temp + 9;
        strncat(strBuf->buf, temp, 6);
        strcat(strBuf->buf, "\n");	
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
    for(i =0 ; i<3; i++)
    {
        OSA_SNPRINTF(wl_temp, WIFI_CMD_MAX_SIZE, "pa2gw%da%d=%s ", i, strBuf->ant_index, strBuf->data[i]);
        strcat(wl_cmd, wl_temp);   
    }
    WIFI_SHELL_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s up", IF_2G);
    WIFI_SHELL_CMD(wl_cmd);

    return (AT_RETURN_SUCCESS);
}
/*****************************************************************************
 函数名称  : int32 WlanATGetWifi5GPavars(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 5G射频参数查询封装函数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 ATGetWifi5GPavars(WLAN_AT_BUFFER_STRU *strBuf)
{
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    int8 filebuff[WL_COUNTER_BUF_LEN + 1] = {0};
    int8* temp = NULL;
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }
           
    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s pavars > /var/tmpfile", IF_5G);
    WIFI_SHELL_CMD(wl_cmd);
 
    /*pavars命令执行结果*/
    if(0 == wlan_read_file("/var/tmpfile", filebuff, sizeof(filebuff)))
    {
        PLAT_WLAN_ERR("read_from_file fail");
        return AT_RETURN_FAILURE;
    }

    /*获取需要的5g参数*/
    temp = strstr(filebuff,"pa5ga0") ;
    strcpy(strBuf->buf, temp);

    return (AT_RETURN_SUCCESS);
}
/*****************************************************************************
 函数名称  : int32 WlanATSetWifi5GPavars(WLAN_AT_PAVARS5G_STRU *strBuf)
 功能描述  : 5G射频参数设置封装函数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 ATSetWifi5GPavars(WLAN_AT_PAVARS5G_STRU *strBuf)
{
    int8 wl_cmd[WIFI_CMD_MAX_SIZE] = {0};
    int8 wl_temp[WIFI_CMD_MAX_SIZE] = {0};
    int32 i;
    if (NULL == strBuf)
    {
        return (AT_RETURN_FAILURE);
    }
           
    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH"%s down", IF_5G);
    WIFI_SHELL_CMD(wl_cmd);

    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH"%s pavars pa5ga%d=", IF_5G, strBuf->ant_index);
    for(i =0 ; i<12; i++)
    {
        OSA_SNPRINTF(wl_temp, WIFI_CMD_MAX_SIZE, "%s,", strBuf->data[i]);
        strcat(wl_cmd, wl_temp);	   
    }
    wl_cmd[strlen(wl_cmd)]='\0';
    WIFI_SHELL_CMD(wl_cmd);
 
    memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
    OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s up", IF_5G);
    WIFI_SHELL_CMD(wl_cmd);

    return (AT_RETURN_SUCCESS);
}

    
/*****************************************************************************
 函数名称  : int32 GET_VALUE_FROM_NV(viod)
 功能描述  : 获取wifi的nv(50601,50602)中的值
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
STATIC int32 GET_VALUE_FROM_NV(unsigned short type, unsigned int len,char *buf)
{
    unsigned short nv_id = type;  
    /* 读取nv数据 */
    if (NV_OK != NV_Read(nv_id, buf, len))
    {
        PLAT_WLAN_ERR("Error, nv read Failed! nv_id = %d, len = %d", nv_id, len);
        return WLAN_PARA_ERR;
    }   
    return WLAN_SUCCESS;
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
    int8 temp[30]={0};
    unsigned short nv_id_2g = NV_ID_WIFI_2G_RFCAL;
    unsigned short nv_id_5g = NV_ID_WIFI_5G_RFCAL; 
    unsigned short nv_wifi_2g_len = 0;
    unsigned short nv_wifi_5g_len = 0;
    char buf_2g[WLAN_AT_BUFFER_SIZE] = {0};
    char buf_5g[WLAN_AT_BUFFER_SIZE] =  {0};
    int i,j;
    
    nv_wifi_2g_len = sizeof(NV_WIFI_2G_RFCAL_STRU);
    nv_wifi_5g_len = sizeof(NV_WIFI_5G_RFCAL_STRU);

    if(WLAN_PARA_ERR == GET_VALUE_FROM_NV(nv_id_2g, nv_wifi_2g_len, buf_2g)
       &&WLAN_PARA_ERR == GET_VALUE_FROM_NV(nv_id_5g, nv_wifi_5g_len, buf_5g))
    {
        return WLAN_PARA_ERR ;
    }
    
    if(WLAN_SUCCESS == GET_VALUE_FROM_NV(nv_id_2g, nv_wifi_2g_len, buf_2g))
    {
         /*2.4g配置校准参数*/
        for(i =0; i< ANT_2G_NUM;i++)
        {   
            if(0 != *(buf_2g+i*ANT_2G_CAL_NV_VALUE_BYTE_NUM))
            {
                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s pavars ", IF_2G);
                for(j = 0; j < ANT_2G_CAL_NV_VALUE_BYTE_NUM/2; j++)
                {
                    memset(temp, 0, sizeof(temp));
                    OSA_SNPRINTF(temp, WIFI_CMD_MAX_SIZE, "pa2gw%da%d=0x%02x%02x ", j, i,*(buf_2g + i*ANT_2G_CAL_NV_VALUE_BYTE_NUM +j*2), \
                        *(buf_2g +i*ANT_2G_CAL_NV_VALUE_BYTE_NUM+j*2+1));
                    strcat(wl_cmd, temp);
                }
                WIFI_SHELL_CMD(wl_cmd);
            }
        }
    }   
    /*5g校准参数配置*/
    if(WLAN_SUCCESS == GET_VALUE_FROM_NV(nv_id_5g, nv_wifi_5g_len, buf_5g))
    {
         /*配置校准参数*/
        for(i =0; i< ANT_5G_NUM;i++)
        {   
            if(0 != *(buf_5g + i*ANT_5G_CAL_NV_VALUE_BYTE_NUM ))
            {
                memset(wl_cmd, 0, WIFI_CMD_MAX_SIZE);
                OSA_SNPRINTF(wl_cmd, WIFI_CMD_MAX_SIZE, WL_CONFIG_EXE_PATH" %s pavars pa5ga%d=", IF_5G, i);
                for(j = 0; j < ANT_5G_CAL_NV_VALUE_BYTE_NUM/2; j++)
                {
                    memset(temp, 0, sizeof(temp));
                    OSA_SNPRINTF(temp, WIFI_CMD_MAX_SIZE, "0x%02x%02x,", *(buf_5g +i*ANT_5G_CAL_NV_VALUE_BYTE_NUM + j*2), \
                        *(buf_5g + i*ANT_5G_CAL_NV_VALUE_BYTE_NUM + j*2 + 1));
                    strcat(wl_cmd, temp);
                }
                WIFI_SHELL_CMD(wl_cmd);
            }
        }  
    }
    return WLAN_SUCCESS;
}

WLAN_CHIP_OPS bcm4360_ops = 
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
    .WlanATSetWifi5GPavars = ATSetWifi5GPavars,
    .WlanATGetWifi5GPavars = ATGetWifi5GPavars
};

