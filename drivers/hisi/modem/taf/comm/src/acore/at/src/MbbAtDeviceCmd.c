


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "AtCheckFunc.h"
#include "LPsNvInterface.h"
#include "AtTestParaCmd.h"
#include "AtSetParaCmd.h"
#include "AtDeviceCmd.h"
#include "GasNvInterface.h"
#include "mdrv_temp_cfg.h"
#include "bsp_shared_ddr.h"
#include "at_lte_common.h"
#include "mdrv_chg.h"
#include "MbbAtDeviceCmd.h"
#include "product_nv_id.h"
#include "drv_nv_id.h"
#include "AtCmdMsgProc.h"

#include "mbb_leds.h"






/*****************************************************************************
  1 宏定义
*****************************************************************************/
/*修改wifi校准温度返回值按照AT规范返回*/

#define   WICALTEMPNUM             (1)


#define    THIS_FILE_ID        PS_FILE_ID_AT_DEVICECMD_C

#define                           SetOemOprLen       (5)
#define                           SetSimlockOprLen   (9)

#define OUTPUT_BUF_LENGTH                               (64)    /*数组长度*/
typedef enum {
    REVERT_FAIL = 0,
    REVERT_SUCCESS = 1
}REVERT_NV_FLAG;
#define DISPLAY_BUF_LENGTH           (16)
typedef enum {
    NO_SUPPORT_FLAG = 0,
    SUPPORT_FLAG = 1,
}TEMP_INFO_MODULE_FLAGS;
typedef struct {
    TEMP_INFO_MODULE_FLAGS module_flag;
    VOS_CHAR module_display[DISPLAY_BUF_LENGTH];
    VOS_INT32 module_value;
}TEMP_INFO_MODULE_STRU;
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
VOS_UINT32              g_SetTseLrfParValue = 0;
/*LTE测试时，选择的分集天线号*/
VOS_UINT32 g_SetLteTseLrfSubParValue = 0;


/*记录当前是否下发过AT^FTYRESET设置命令*/
static SET_FTYRESET_TYPE g_set_ftyreset_flag = NOT_SET;

static SET_MMITEST_ST mmi={0,0,0,0,0,0,0}; /* 存储 mmi 工位检测标志位数值 需重启清除*/

#define MMI_PAR_NUM     (1)   /* MMI参数num定义*/
#define POWERKEY_OK     (1)   /* powerkey 按键测试OK 标志位数值*/
#define WPSKEY_OK       (2)   /* wpskey 按键测试OK 标志位数值*/
#define RESETKEY_OK     (3)   /* resetkey 按键测试OK 标志位数值*/
#define SIM_OK          (4)   /* SIM 卡测试OK 标志位数值*/
#define USB_OK          (5)   /* USB 接口测试OK 标志位数值*/
#define ETH_OK          (6)   /* 网口接口测试OK 标志位数值*/
#define LEVEL_OK        (7)  /* levelkey 按键测试OK 标志位数值*/
#define MODEKEY_OK      (8)   /* modekey 按键测试OK 标志位数值*/

#define POWERKEY_FAIL     (11)   /* powerkey 按键测试失败 标志位数值*/
#define WPSKEY_FAIL       (12)   /* wpskey 按键测试失败标志位数值*/
#define RESETKEY_FAIL     (13)   /* resetkey 按键测试失败标志位数值*/
#define SIM_FAIL          (14)   /* SIM 卡测试失败标志位数值*/
#define USB_FAIL          (15)   /* USB 接口测试失败标志位数值*/
#define ETH_FAIL          (16)   /* 网口接口测试失败标志位数值*/
#define LEVEL_FAIL        (17)  /* levelkey 按键测试失败标志位数值*/
#define MODEKEY_FAIL      (18)   /* modekey 按键测试失败 标志位数值*/

#define   LTE_FLOW_LED    "lte_led:green"
#define   LED_OFF         0
#define   BAT_GREEN_LED    "bat_led:green"


VOS_INT32 phy_led_status_set(VOS_INT8* led_name, VOS_INT32 onoff);

/*lint -e553*/
/*lint +e553*/

AT_LED_TEST_TAB_STRU g_led_test_config_tab[] =
{
    /*lint -e64*/
    /* 修改设备树之后，需要重新匹配灯*/
    {0, AT_GPIO_LED, MODE_BLUE_LED, led_kernel_status_set, 0, 0},
    {1, AT_GPIO_LED, MODE_GREEN_LED, led_kernel_status_set, 0, 0},
    {2, AT_GPIO_LED, MODE_RED_LED, led_kernel_status_set, 0, 0},
    {3, AT_GPIO_LED, WIFI_WHITE_LED, led_kernel_status_set, 0, 0},
    {4, AT_GPIO_LED, LAN_WHITE_LED, led_kernel_status_set, 0, 0},
    {5, AT_GPIO_LED, STATUS_GREEN_LED, led_kernel_status_set, 0, 0},
    {6, AT_GPIO_LED, POWER_GREEN_LED, led_kernel_status_set, 0, 0},
    {7, AT_GPIO_LED, SIGNAL1_RED_LED, led_kernel_status_set, 0, 0},
    {8, AT_GPIO_LED, SIGNAL1_WHITE_LED, led_kernel_status_set, 0, 0},
    {9, AT_GPIO_LED, SIGNAL2_WHITE_LED, led_kernel_status_set, 0, 0},
    {10, AT_GPIO_LED, SIGNAL3_WHITE_LED, led_kernel_status_set, 0, 0},
    {12, AT_PHY_LED, "phy_led1", phy_led_status_set, 0, 0},
    {13, AT_PHY_LED, "phy_led2", phy_led_status_set, 0, 0},
    {14, AT_PHY_LED, "phy_led3", phy_led_status_set, 0, 0},
    {15, AT_PHY_LED, "phy_led4", phy_led_status_set, 0, 0},
/*lint +e64*/
};
extern VOS_UINT32 AT_SetTmodeAutoPowerOff(VOS_UINT8 ucIndex);
extern TAF_UINT8 At_GetDspLoadMode(VOS_UINT32 ulRatMode);

extern VOS_UINT32                 g_ulNVRD;
extern VOS_UINT32                 g_ulNVWR;

extern VOS_VOID AT_GetNvRdDebug(VOS_VOID);


VOS_UINT8 gportTypeNum[MAXPORTTYPENUM] = {0};

extern TAF_UINT32 At_MacConvertWithColon(TAF_UINT8 *pMacDst, TAF_UINT8 *pMacSrc, TAF_UINT16 usSrcLen);
extern void report_power_key_up_for_suspend(void);
extern void report_power_key_down_for_suspend(void);
static REVERT_NV_FLAG g_revert_flag = REVERT_FAIL;
/*****************************************************************************
  3 函数定义
*****************************************************************************/

VOS_UINT32 At_TestFlnaPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    usLength  = 0;

    /* 该命令需在非信令模式下使用 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_ERROR;
    }

    /* 该命令需在设置非信令信道后使用 */
    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_ERROR;
    }
    
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        /* WDSP LNA等级取值为0-5 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s:6,0,1,2,3,4,5",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }

    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        /* WDSP LNA等级取值为0-3 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s:4,0,1,2,3",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }

    /* 参数LNA等级取值范围检查 */
    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        /* WDSP LNA等级取值为0-2 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s:3,0,1,2",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }

    if (AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        /* WDSP LNA等级取值为0-3 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s:256,(0-255)",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }
    
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32  At_SetVersionPara(VOS_UINT8 ucIndex )
{
    VOS_UINT8 iniVersion[AT_VERSION_INI_LEN] = {0};
    
    /* 设置命令无参数 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数过多 */
    if (2 != gucAtParaIndex)
    {
        return AT_ERROR;
    }
    
    if (0 == gastAtParaList[0].usParaLen
        || 0 == gastAtParaList[1].usParaLen)
    {
        return AT_ERROR;
    }
    
    /* 版本号类型目前只支持INI写入 */
    if (3 != gastAtParaList[0].usParaLen)
    {
        return AT_ERROR;
    }
    
    if (gastAtParaList[1].usParaLen >= AT_VERSION_INI_LEN)
    {
        return AT_ERROR;
    }
    
    /*lint -e64*/
    if (VOS_OK != VOS_StrNiCmp(gastAtParaList[0].aucPara, "INI", gastAtParaList[0].usParaLen))
    {
        return AT_ERROR;
    }
    /*lint +e64*/
    
    (void)VOS_MemCpy((void *)iniVersion, (void *)gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
    iniVersion[gastAtParaList[1].usParaLen] = '\0';
    
    if (NV_OK != NV_Write(en_NV_Item_PRI_VERSION, iniVersion, sizeof(iniVersion)))
    {
        AT_ERR_LOG("At_SetVersion: Write NV fail");
        return AT_ERROR;
    }
    else
    {
        return AT_OK;
    }
}


void version_info_build(VOS_UINT8 ucIndex, VOS_UINT8  *pucKey, VOS_UINT8  *pucValue)
{
    gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                          "%s:%s:%s%s\r\n",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          pucKey,
                                          pucValue,
                                          gaucAtCrLf);
}


void version_info_fill(VOS_UINT8  *pucDes, VOS_UINT8  *pucSrc)
{
    (void)memset(pucDes, 0, TAF_MAX_VERSION_VALUE_LEN);
    (void)strncpy(pucDes, pucSrc, strlen(pucSrc)); /*lint !e64*/
}




VOS_UINT32 At_TestTmmiPara(VOS_UINT8 ucIndex)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* TMMI支持的测试模式 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s=%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    AT_MMI_TEST_SUPPORT_MANUAL);
    return AT_OK;
}






VOS_UINT32 At_TestWiFiModePara(VOS_UINT8 ucIndex)
{

    WLAN_AT_BUFFER_STRU     strBuf = {0,{0}};

    /* 命令类型判断 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void *)&strBuf, 0, sizeof(strBuf));
    
    /* 读取WIFI模式 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiModeSupport(&strBuf))
    {
        return AT_ERROR;
    }
    
    /* WIFI模块模式 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
    return AT_OK;
}




VOS_UINT32 At_TestWiFiBandPara(VOS_UINT8 ucIndex)
{

    WLAN_AT_BUFFER_STRU     strBuf = {0,{0}};

    /* 命令类型判断 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void *)&strBuf, 0, sizeof(strBuf));

    /* 读取WIFI支持带宽 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiBandSupport(&strBuf))
    {
        return AT_ERROR;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
    return AT_OK;
}




VOS_UINT32 AT_SetWifiCalPara(VOS_UINT8 ucIndex)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 设置WIFI校准状态 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiCal(gastAtParaList[0].ulParaValue))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 AT_QryWifiCalPara(VOS_UINT8 ucIndex)
{
    VOS_INT32 ucWifiCal = 0;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 获取WIFI 校准模式信息 */
    ucWifiCal = WlanATGetWifiCal();

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiCal);
	return AT_OK;
}


VOS_UINT32 AT_TestWifiCalPara(VOS_UINT8 ucIndex)
{
    VOS_INT32 ucWifiCalSupport = 0;
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /*获取wical支持参数*/
    ucWifiCalSupport = WlanATGetWifiCalSupport();

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiCalSupport);
    return AT_OK;
}


VOS_UINT32 AT_SetWifiCalDataPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALDATA_STRU auWiCalDataTemp;
    (void)VOS_MemSet((void *)(&auWiCalDataTemp), 0, sizeof(auWiCalDataTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (WICALDATA_PARA_MAX != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /*DATALOCK解锁判断*/
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }



    auWiCalDataTemp.type = (WLAN_AT_WICALDATA_TYPE)gastAtParaList[0].ulParaValue;
    auWiCalDataTemp.group = (VOS_INT32)gastAtParaList[1].ulParaValue;
    auWiCalDataTemp.mode = (WLAN_AT_WIMODE_TYPE)gastAtParaList[2].ulParaValue;
    auWiCalDataTemp.band = (WLAN_AT_WIFREQ_TYPE)gastAtParaList[3].ulParaValue;
    auWiCalDataTemp.bandwidth = (WLAN_AT_WIBAND_TYPE)gastAtParaList[4].ulParaValue;
    auWiCalDataTemp.freq = (VOS_INT32)gastAtParaList[5].ulParaValue;
    PS_MEM_CPY(auWiCalDataTemp.data,
                   gastAtParaList[6].aucPara,
                   WICALDATA_DATA_PARA_MAX);

    /* 设置WIFI校准状态 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiCalData(&auWiCalDataTemp))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 AT_QryWifiCalDataPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALDATA_STRU ucWifiCalDataTemp;
    (void)VOS_MemSet((void *)(&ucWifiCalDataTemp), 0, sizeof(ucWifiCalDataTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 获取WIFI 校准参数信息 */
    if (AT_SUCCESS != WlanATGetWifiCalData(&ucWifiCalDataTemp))
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d,%d,%d,%d,%d,%d,%s%s%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiCalDataTemp.type,ucWifiCalDataTemp.group,
                                                    ucWifiCalDataTemp.mode,ucWifiCalDataTemp.band,
                                                    ucWifiCalDataTemp.bandwidth,ucWifiCalDataTemp.freq,
                                                    gaucAtQuotation,ucWifiCalDataTemp.data,gaucAtQuotation);
	return AT_OK;
}


VOS_UINT32 AT_SetWifiCalTempPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALTEMP_STRU auWiCalTempTemp;
    (void)VOS_MemSet((void *)(&auWiCalTempTemp), 0, sizeof(auWiCalTempTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (WICALTEMP_PARA_MAX != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    auWiCalTempTemp.index = (VOS_INT32)gastAtParaList[0].ulParaValue;
    auWiCalTempTemp.value = (VOS_INT32)gastAtParaList[1].ulParaValue;

    /* 设置WIFI校准状态 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiCalTemp(&auWiCalTempTemp))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 AT_QryWifiCalTempPara(VOS_UINT8 ucIndex)
{
#define   WICALTEMPNUM             (1)
    WLAN_AT_WICALTEMP_STRU ucWifiCalTempTemp;
    (void)VOS_MemSet((void *)(&ucWifiCalTempTemp), 0, sizeof(ucWifiCalTempTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 获取WIFI 校准参数信息 */
    if (AT_SUCCESS != WlanATGetWifiCalTemp(&ucWifiCalTempTemp))
    {
        return AT_ERROR;
    }
    /*按照规范返回WIFI校准温度*/
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d%s%s:%d,%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    WICALTEMPNUM,
                                                    gaucAtCrLf,
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiCalTempTemp.index,ucWifiCalTempTemp.value);
	return AT_OK;
}


VOS_UINT32 AT_SetWifiCalFreqPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALFREQ_STRU auWiCalFreqTemp;
    (void)VOS_MemSet((void *)(&auWiCalFreqTemp), 0, sizeof(auWiCalFreqTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (WICALFREQ_PARA_MAX != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    auWiCalFreqTemp.type = (WLAN_AT_WICALFREQ_TYPE)gastAtParaList[0].ulParaValue;
    auWiCalFreqTemp.value = (VOS_INT32)gastAtParaList[1].ulParaValue;

    /* 设置WIFI校准状态 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiCalFreq(&auWiCalFreqTemp))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 AT_QryWifiCalFreqPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALFREQ_STRU ucWifiCalFreqTemp;
    (void)VOS_MemSet((void *)(&ucWifiCalFreqTemp), 0, sizeof(ucWifiCalFreqTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 获取WIFI 校准参数信息 */
    if (AT_SUCCESS != WlanATGetWifiCalFreq(&ucWifiCalFreqTemp))
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d,%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiCalFreqTemp.type,ucWifiCalFreqTemp.value);
	return AT_OK;

}


VOS_UINT32 AT_SetWifiCalPowPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALPOW_STRU auWiCalPowTemp;
    (void)VOS_MemSet((void *)(&auWiCalPowTemp), 0, sizeof(auWiCalPowTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (WICALPOW_PARA_MAX != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    auWiCalPowTemp.type = (WLAN_AT_WICALPOW_TYPE)gastAtParaList[0].ulParaValue;
    auWiCalPowTemp.value = (VOS_INT32)gastAtParaList[1].ulParaValue;

    /* 设置WIFI校准状态 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiCalPOW(&auWiCalPowTemp))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 AT_QryWifiCalPowPara(VOS_UINT8 ucIndex)
{
    WLAN_AT_WICALPOW_STRU ucWifiCalPowTemp;
    (void)VOS_MemSet((void *)(&ucWifiCalPowTemp), 0, sizeof(ucWifiCalPowTemp));

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 获取WIFI 校准参数信息 */
    if (AT_SUCCESS != WlanATGetWifiCalPOW(&ucWifiCalPowTemp))
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d,%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiCalPowTemp.type,ucWifiCalPowTemp.value);
    return AT_OK;

}


VOS_UINT32 AT_SetNavTypePara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 AT_QryNavTypePara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 At_TestNavTypePara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 AT_SetNavEnablePara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 AT_SetNavFreqPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 AT_QryNavFreqPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 AT_QryNavRxPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


typedef int (*SlicATSetAudioEnableHandle)(unsigned int access, unsigned int type, unsigned int onoff);
SlicATSetAudioEnableHandle g_SlicATSetAudioEnableHandle = NULL;
VOS_VOID AT_RegisterSlicATSetAudioEnable(VOS_VOID *Handle)
{
    g_SlicATSetAudioEnableHandle = (SlicATSetAudioEnableHandle)Handle; /*lint !e611*/
}
EXPORT_SYMBOL(AT_RegisterSlicATSetAudioEnable);
#define AUDIO_PAR_NUM (3)

VOS_UINT32 AT_SetAudioPara(VOS_UINT8 ucIndex)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (AUDIO_PAR_NUM != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    if ((NULL != g_SlicATSetAudioEnableHandle)
        && (TAF_SUCCESS == g_SlicATSetAudioEnableHandle(gastAtParaList[0].ulParaValue,gastAtParaList[1].ulParaValue,gastAtParaList[2].ulParaValue))
        )
    {
        return AT_OK;
    }
    return AT_ERROR;

}




VOS_UINT32  AT_SetTnetPortPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR; 
}


VOS_UINT32 AT_QryTnetPortPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16 usLen = 0; 
    VOS_UINT32 cur_index = 0;
    NET_PORT_ST test_port_para[AT_LAN_PORT_NUM];
    unsigned char port_ip_add[] = {'1','9','2','.','1','6','8','.','8','.','1'}; 

    (void)VOS_MemSet(test_port_para, 0, sizeof(test_port_para));
    if (-1 == PhyATQryPortPara(test_port_para))
    {
        return AT_ERROR;
    }

    if(test_port_para[AT_LAN_PORT1].total_port > AT_LAN_PORT_NUM)
    {
        return AT_ERROR;
    }

    /* 根据网口数目对 ip_add 进行赋值。之前是写死的只对1、2端口 ip_add 赋值 */
    for (cur_index = 0; cur_index < test_port_para[AT_LAN_PORT1].total_port; cur_index++)
    {
        if (0 == (VOS_UINT16)AT_STRLEN((VOS_CHAR *)(test_port_para[cur_index].ip_add)))
        {
            PS_MEM_CPY(test_port_para[cur_index].ip_add, port_ip_add, sizeof(port_ip_add));
        }
    }
    
    usLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                            (TAF_CHAR *)pgucAtSndCodeAddr,
                            (TAF_CHAR *)pgucAtSndCodeAddr,
                            "%s:%d%s",
                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                            test_port_para[AT_LAN_PORT1].total_port,gaucAtCrLf); 
 
    for(cur_index = 0; cur_index < test_port_para[AT_LAN_PORT1].total_port; cur_index++) 
    {
        usLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                "%s:%d,%s%s",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                cur_index,
                                test_port_para[cur_index].ip_add,gaucAtCrLf); 

        usLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                "%s:%d%s",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                test_port_para[cur_index].port_rate,
                                gaucAtCrLf); 
    }
    
    gstAtSendData.usBufLen = usLen;
    
    return AT_OK;
}


VOS_UINT32 At_QryExtChargePara(VOS_UINT8 ucIndex)
{

    VOS_INT8 testRes;
    
    /* 参数检查 */
    if(AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    testRes = chg_extchg_mmi_test();
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%d",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 testRes);

	return AT_OK;
}


VOS_UINT32 AT_SetWebSitePara(VOS_UINT8 ucIndex)
{
    VOS_UINT8    aucWebSiteTmp[AT_WEBUI_SITE_NV_LEN_MAX + 1] = {0};
    
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数检查 */
    if (1 != gucAtParaIndex)
    {
        return  AT_TOO_MANY_PARA;
    }
    
    /* 参数长度过长 */
    if (gastAtParaList[0].usParaLen > AT_WEBUI_SITE_WR_LEN_MAX)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    /* 参数长度过短 */
    if (3 > gastAtParaList[0].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    /* 数据锁保护 */
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
    
    (void)VOS_MemSet((void *)aucWebSiteTmp, 0, sizeof(aucWebSiteTmp));
    (void)VOS_MemCpy((void *)aucWebSiteTmp, gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen);
    
    /* 写入WEB SITE对应的NV项 */
    if (VOS_OK != NV_Write(NV_ID_WEB_SITE , aucWebSiteTmp, AT_WEBUI_SITE_NV_LEN_MAX))
    {
        AT_WARN_LOG("AT_SetWebSitePara:WRITE NV ERROR");/*lint !e64 */
        return AT_ERROR;
    }
    
    return AT_OK;
}


VOS_UINT32  AT_QryWebSitePara(VOS_UINT8 ucIndex )
{
   VOS_CHAR     aucWebSiteTmp[AT_WEBUI_SITE_NV_LEN_MAX + 1];

   /* 参数检查 */
    if(AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (void)VOS_MemSet((void *)aucWebSiteTmp, 0, sizeof(aucWebSiteTmp));

    if (NV_OK != NV_Read(NV_ID_WEB_SITE, aucWebSiteTmp, AT_WEBUI_SITE_NV_LEN_MAX))
    {
        AT_WARN_LOG("AT_QryWebsite ERROR: NVIM Read en_NV_Item_Web_Site falied!");
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:%s%s%s",
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                 gaucAtQuotation,
                                                 aucWebSiteTmp,
                                                 gaucAtQuotation);
                    
    return AT_OK;
}



VOS_INT AT_WiFiPinChecksum(VOS_CHAR *acInputStr)
{
    VOS_UINT32  uiAccum  = 0;
    VOS_UINT32  ulValTmp = 0;
    VOS_INT16   iIndStr  = 0;

    if(NULL == acInputStr)
    {
        return VOS_ERROR;
    }
    
    if(AT_WIFI_8BIT_PIN_LEN != VOS_StrLen(acInputStr))
    {
        /* vos_printf("WiFiPINChecksum: ERROR, 8 != VOS_StrLen(acInputStr)!!\n"); */
        return VOS_ERROR;
    }
    
    for(iIndStr = 0; iIndStr < AT_WIFI_8BIT_PIN_LEN; iIndStr++)
    {
        if((acInputStr[iIndStr] < '0')
           || (acInputStr[iIndStr] > '9'))
        {
            /* vos_printf("WiFiPINChecksum: ERROR, acInputStr[iIndStr] = #%c# is not digit num!!\n", acInputStr[iIndStr]); */
            return VOS_ERROR;
        }
        
        ulValTmp = (VOS_UINT32)((char)acInputStr[iIndStr] - '0');/*lint !e571 */
        
        if(0 == (iIndStr % 2))
        {
            uiAccum = uiAccum + 3 * (ulValTmp % 10);
        }
        else
        {
            uiAccum = uiAccum + (ulValTmp % 10);
        }
    }
    
    if(0 == (uiAccum % 10))
    {
        return VOS_OK;
    }
    
    /* vos_printf("WiFiPINChecksum: ERROR, checksum failed!!\n"); */
    return VOS_ERROR;
}


VOS_UINT32 AT_SetWiFiPinPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8    aucWiFiPinTmp[AT_WIFI_PIN_NV_LEN_MAX + 1];
    
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数检查 */
    if (1 != gucAtParaIndex)
    {
        return  AT_TOO_MANY_PARA;
    }
    
    /* 参数长度检查, WPS PIN 长度必须为8 或者 4 */
    if (AT_WIFI_8BIT_PIN_LEN != gastAtParaList[0].usParaLen && AT_WIFI_4BIT_PIN_LEN != gastAtParaList[0].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    /* 数据锁保护 */
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
    
    (void)VOS_MemSet((void *)aucWiFiPinTmp, 0, sizeof(aucWiFiPinTmp));
    (void)VOS_MemCpy(aucWiFiPinTmp, gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen);
    
    /**8-Bit PIN Need to checksum**/
    if (AT_WIFI_8BIT_PIN_LEN == gastAtParaList[0].usParaLen)
    {
        if(VOS_OK != AT_WiFiPinChecksum((VOS_CHAR *)aucWiFiPinTmp))
        {
            AT_WARN_LOG("AT_SetWiFiPINPara: 8Bit-PIN checksum failed!");/*lint !e64 */
            return AT_ERROR;
        }
    }
    
    /* 写入WiFi PIN对应的NV项 */
    if (VOS_OK != NV_Write(NV_ID_WPS_PIN , aucWiFiPinTmp, AT_WIFI_PIN_NV_LEN_MAX))
    {
        AT_WARN_LOG("AT_SetWiFiPINPara:WRITE NV ERROR");/*lint !e64 */
        return AT_ERROR;
    }
    
    return AT_OK;
}


VOS_UINT32  AT_QryWiFiPinPara(VOS_UINT8 ucIndex )
{
   VOS_CHAR     aucWebPinTmp[AT_WIFI_PIN_NV_LEN_MAX + 1];

   /* 参数检查 */
    if(AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /* 数据锁保护 */
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
        
    (void)VOS_MemSet((void *)aucWebPinTmp, 0, sizeof(aucWebPinTmp));

    if (NV_OK != NV_Read(NV_ID_WPS_PIN, aucWebPinTmp, AT_WIFI_PIN_NV_LEN_MAX))
    {
        AT_WARN_LOG("AT_QryWebsite ERROR: NVIM Read en_NV_Item_Web_Site falied!");
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:%s%s%s",
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                 gaucAtQuotation,
                                                 aucWebPinTmp,
                                                 gaucAtQuotation);
    
    return AT_OK;
}


VOS_UINT32 AT_SetWebUserPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8  aucWebUserTmp[AT_WEBUI_USER_NV_LEN_MAX + 1];
    
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数检查 */
    if (1 != gucAtParaIndex)
    {
        return  AT_TOO_MANY_PARA;
    }
    
    /* 参数长度过长 */
    if (AT_WEBUI_USER_WR_LEN_MAX < gastAtParaList[0].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    if (0 == gastAtParaList[0].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    /**数据锁保护**/
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
    
    (void)VOS_MemSet((void *)aucWebUserTmp, 0, sizeof(aucWebUserTmp));
    (void)VOS_MemCpy((void *)aucWebUserTmp, gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen);
    
    /* 写入WEB USER对应的NV项 */
    if (VOS_OK != NV_Write(NV_ID_WEB_USER_NAME , aucWebUserTmp, AT_WEBUI_USER_NV_LEN_MAX))
    {
        AT_WARN_LOG("AT_SetWebUserPara:WRITE NV ERROR");/*lint !e64 */
        return AT_ERROR;
    }
    
    return AT_OK;
}


VOS_UINT32  AT_QryWebUserPara(VOS_UINT8 ucIndex )
{
    VOS_CHAR     aucWebUsermp[AT_WEBUI_SITE_NV_LEN_MAX + 1];

    /* 参数检查 */
    if(AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 数据锁保护 */
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }

    (void)VOS_MemSet((void *)aucWebUsermp, 0, sizeof(aucWebUsermp));
    
    if (NV_OK != NV_Read(NV_ID_WEB_USER_NAME, aucWebUsermp, AT_WIFI_PIN_NV_LEN_MAX))
    {
        AT_WARN_LOG("AT_QryWebUserPara ERROR: NVIM Read NV_ID_WEB_USER_NAME falied!");
        return AT_ERROR;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:%s%s%s",
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                 gaucAtQuotation,
                                                 aucWebUsermp,
                                                 gaucAtQuotation);

    return AT_OK;
}


#define PARA_NUM_4    (4)
#define PARA_LEN_STR    "16"
VOS_UINT32 At_SetPortLockPara(VOS_UINT8 ucIndex)
{
    VOS_INT32 ret = 0;
    DRV_AGENT_DATALOCK_SET_REQ_STRU     stDatalockInfo;
    CPE_TELNET_SWITCH_NVIM_STRU telnet_state;
    (void)VOS_MemSet((void *)(&stDatalockInfo), 0, sizeof(stDatalockInfo));
    PS_MEM_SET((void*)&telnet_state, 0x0, sizeof(telnet_state));
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数检查 */
    if (4 != gucAtParaIndex)
    {
        return  AT_TOO_MANY_PARA;
    }

    /* <unlock_code>必须为长度为16位的数字字符串 */
    /*lint -e{64}*/
    if ( ( TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX != gastAtParaList[3].usParaLen) \
        || (VOS_OK != VOS_StrNiCmp(gastAtParaList[2].aucPara, PARA_LEN_STR, gastAtParaList[2].usParaLen)) )
    {
        return AT_ERROR;
    }

    if (AT_FAILURE == At_CheckNumCharString(gastAtParaList[3].aucPara, \
        TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX))
    {
        return AT_ERROR;
    }

    /* 复制用户解锁码 */
    stDatalockInfo.aucPwd[TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX] = '\0';
    PS_MEM_CPY(stDatalockInfo.aucPwd, gastAtParaList[3].aucPara, \
        TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX);


    if((VOS_OK != AT_QryOemLockEnable()) && (VOS_OK != AT_ReadPhyNV()))    /*V7R11是否存在双卡的情况*/
    {
        /*第三个参数为datalock密码*/
        ret = hw_lock_verify_proc(gastAtParaList[3].aucPara, gastAtParaList[3].usParaLen, HW_LOCK_OEM_TYPE);
        if (0 != ret)
        {
            return AT_ERROR;
        }
        /*修改nv50501*/
        if (NV_OK != NV_ReadEx(MODEM_ID_0, NV_ID_TELNET_SWITCH_I, &telnet_state, sizeof(CPE_TELNET_SWITCH_NVIM_STRU)))
        {
            return AT_ERROR;
        }

        /*0--加锁(锁端口);1--解锁*/
        if ( (VOS_OK == VOS_StrNiCmp((VOS_CHAR*)gastAtParaList[0].aucPara, "1", gastAtParaList[0].usParaLen))
            || (VOS_OK == VOS_StrNiCmp((VOS_CHAR*)gastAtParaList[0].aucPara, "0", gastAtParaList[0].usParaLen)) )
        {
            telnet_state.nv_telnet_switch = gastAtParaList[0].aucPara[0] - '0';
            if (NV_OK != NV_WriteEx(MODEM_ID_0, NV_ID_TELNET_SWITCH_I, &telnet_state, sizeof(CPE_TELNET_SWITCH_NVIM_STRU)))
            {
                return AT_ERROR;
            }
        }
        else
        {
            return AT_ERROR;
        }

        return AT_OK;
    }
    else
    {
        return AT_OK;
    }

}


#define PORT_LOCKED    (2)
#define PORT_UNLOCKED    (1)
#define PORT_NO_NEED_LOCK    (0)
VOS_UINT32 At_QryPortLockPara(VOS_UINT8 ucIndex)
{
    CPE_TELNET_SWITCH_NVIM_STRU aucPortLockTmp;
    VOS_CHAR portstate = 0; 
    /* 参数检查 */
    if(AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (void)VOS_MemSet((void *)(&aucPortLockTmp), 0, sizeof(aucPortLockTmp));

    if (NV_OK != NV_Read(NV_TELNET_SWITCH_I, (VOS_VOID *)(&aucPortLockTmp), (VOS_UINT32)sizeof(aucPortLockTmp)))
    {
        AT_WARN_LOG("At_QryPortLockPara ERROR: NVIM Read NV_TELNET_SWITCH_I falied!");
        return AT_ERROR;
    }

    /*该AT目前只支持LANAT，不存在无需加锁的情况*/
    if (0 == aucPortLockTmp.nv_telnet_switch)
    {
        portstate = PORT_LOCKED;
    }
    else
    {
        portstate = PORT_UNLOCKED;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:LANAT,%d",
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                 portstate);
                    
    return AT_OK;
}


VOS_UINT32 AT_SetTbatDataPara(VOS_UINT8 ucIndex)
{

    AT_TBATDATA_BATTERY_ADC_INFO_STRU          stBatdata = {0};
    
    /* 参数有效性检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    if (3 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    if ((0 == gastAtParaList[0].usParaLen)
        || (0 == gastAtParaList[1].usParaLen)
        || (0 == gastAtParaList[2].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /*功能判断:若产品不支持电池，则直接返回ERROR*/
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_CHARGE))
    {
        return AT_ERROR;
    }
    
    if (AT_TBATDATA_VOLTAGE == gastAtParaList[0].ulParaValue)
    {
        /* 调用底软接口设置电池电压 */
        if(AT_TBATDATA_INDEX0 == gastAtParaList[1].ulParaValue)
        {
            stBatdata.usMinAdc = gastAtParaList[2].ulParaValue;
            stBatdata.usMaxAdc = TBAT_CHECK_INVALID;
        }
        else
        {
            stBatdata.usMinAdc = TBAT_CHECK_INVALID;
            stBatdata.usMaxAdc = gastAtParaList[2].ulParaValue;
        }
        
        /* 调用^TBAT命令数字电压写接口*/
        if (CHG_OK != chg_tbat_write(CHG_AT_BATTERY_CHECK, &stBatdata))
        {
            return AT_ERROR;
        }
        
        return AT_OK;
    }
    else if (AT_TBATDATA_CURRENT == gastAtParaList[0].ulParaValue)
    {
        /* 电流校准接口有待实现 */
        return AT_ERROR;
    }
    else
    {
        return AT_ERROR;
    }


}


VOS_UINT32 At_QryTbatDataPara(VOS_UINT8 ucIndex)
{

    AT_TBAT_BATTERY_ADC_INFO_STRU stAdcInfo;
    VOS_UINT16 usLen = 0;

    /*命令状态类型检查*/
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /*功能判断:若产品不支持电池，则直接返回ERROR*/
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_CHARGE) )
    {
        return AT_ERROR;
    }
    
    (void)VOS_MemSet((void *)&stAdcInfo, 0, sizeof(stAdcInfo));
    
    if (CHG_OK != chg_tbat_read(CHG_AT_BATTERY_CHECK, &stAdcInfo))          
    {              
        return AT_ERROR;         
    }
    
    usLen = (TAF_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   "%s:%d,%d%s",
                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                   VOLT_CALC_NUM_MAX,
                                   CURRENT_CALC_NUM_MAX,
                                   gaucAtCrLf);
    
    usLen += (TAF_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                   "%s:%d,%d,%d%s",
                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                   AT_TBATDATA_VOLTAGE,
                                   AT_TBATDATA_INDEX0,
                                   stAdcInfo.usMinAdc,
                                   gaucAtCrLf);
    
    usLen += (TAF_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                   "%s:%d,%d,%d",
                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                   AT_TBATDATA_VOLTAGE,
                                   AT_TBATDATA_INDEX1,
                                   stAdcInfo.usMaxAdc);
 
    gstAtSendData.usBufLen = usLen;

    return AT_OK;
}


VOS_UINT32 AT_QryWiFiPlatformPara(VOS_UINT8 ucIndex)
{

    WLAN_AT_WIPLATFORM_TYPE   ucWifiPlatform;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    ucWifiPlatform = WlanATGetWifiPlatform();
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiPlatform);
    return AT_OK;
}


ante_switch_handle ante_switch_array[SUPPORT_RF_BAND_FOR_ANTE] = 
{
        /*  射频通道             天线主分集       内置还是外置      软件支持类型      */
        {GSM_BAND_FOR_ANTE,       MIMO_ANTE,      Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {WCDMA_PRI_BAND_FOR_ANTE, PRIMARY_ANTE,   Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {WCDMA_SEC_BAND_FOR_ANTE, SECONDARY_ANTE, Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {WIFI_BAND_FOR_ANTE,      MIMO_ANTE,      Antenna_Set_IN,   NO_SUPPORT},
        {FDD_LTE_PRI_FOR_ANTE,    PRIMARY_ANTE,   Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {FDD_LTE_SEC_FOR_ANTE,    SECONDARY_ANTE, Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {FDD_LTE_MIMO_FOR_ANTE,   MIMO_ANTE,      Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {TDD_LTE_PRI_FOR_ANTE,    PRIMARY_ANTE,   Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {TDD_LTE_SEC_FOR_ANTE,    SECONDARY_ANTE, Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
        {TDD_LTE_MIMO_FOR_ANTE,   MIMO_ANTE,      Antenna_Set_Auto, SUPPORT_WITH_SNESOR},
};

VOS_UINT32 AT_SetAntennaPara(VOS_UINT8 ucIndex)
{
    int i = 0;
    
    for(i = 0; i < SUPPORT_RF_BAND_FOR_ANTE; i++)
    {
        if(g_SetTseLrfParValue == ante_switch_array[i].rf_band_for_ante)
        {
            if( (Antenna_Set_Auto == ante_switch_array[i].support_switch_type) 
                || (gastAtParaList[0].ulParaValue == ante_switch_array[i].support_switch_type) )
            {   
                if(-1 == anten_switch_set(ante_switch_array[i].ant_type, gastAtParaList[0].ulParaValue))
                {
                    return AT_ERROR; 
                }
            
                return AT_OK;
            }
            
            return AT_ERROR;
        }
    }

    return AT_ERROR;
    
}


VOS_UINT32 AT_QryAntennaPara(VOS_UINT8 ucIndex)
{
    int Antenna =0;
    VOS_UINT16 usLen = 0;
    int i = 0;
    
    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    for(i = 0; i < SUPPORT_RF_BAND_FOR_ANTE; i++)
    {
        if(g_SetTseLrfParValue == ante_switch_array[i].rf_band_for_ante)
        {
            Antenna = anten_switch_get(ante_switch_array[i].ant_type);
            if(-1 ==  Antenna)
            {
                return AT_ERROR;
            }

            
            usLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
            (TAF_CHAR *)pgucAtSndCodeAddr,
            (TAF_CHAR *)pgucAtSndCodeAddr,
            "%s:%d\n",
            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
            Antenna);
            gstAtSendData.usBufLen = usLen;
            
            return AT_OK;
        }
    }

     return AT_ERROR;
}


VOS_UINT32 At_TestAntenna(VOS_UINT8 ucIndex)
{
    VOS_UINT8 SupportSoftSwich = 0;
    VOS_UINT8 SupportSwich = 0;
    VOS_UINT16    usLength = 0;
    int i = 0;
    
    for(i = 0; i < SUPPORT_RF_BAND_FOR_ANTE; i++)
    {
        if(g_SetTseLrfParValue == ante_switch_array[i].rf_band_for_ante)
        {      
            SupportSoftSwich = ante_switch_array[i].support_soft_switch;
            SupportSwich = ante_switch_array[i].support_switch_type;
                
            usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
            (TAF_CHAR *)pgucAtSndCodeAddr,
            (TAF_CHAR *)pgucAtSndCodeAddr,
            "%s:%d,%d\n",
            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
            SupportSoftSwich,
            SupportSwich);
            gstAtSendData.usBufLen = usLength;
            
            return AT_OK;
        }
    }

     return AT_ERROR;

}

#define TEMPERATURE_MAGIC         0x5A5A5A5A
#define TEMPINIT                  (-25)
#define TEMPINIT_HISI             (-250)


VOS_UINT32 AT_QryTempInfo(VOS_UINT8 ucIndex)
{
    VOS_UINT16   usLength = 0;
    VOS_INT16    index_chan = 0;
    VOS_UINT8    phy_chan = 0xFF;
    VOS_INT16    loop = 0;
    VOS_INT16    loop_num = 0;
    /* display_buf和返回值中各字段的显示顺序有关 */
    VOS_CHAR display_buf[][DISPLAY_BUF_LENGTH] = {"BAT","SIM","SD","XO","WPA","GPA","CPA",
                                                  "LPA","USB","SHELL","MCP"};
    TEMP_INFO_MODULE_STRU temp_info[HKADC_LOGIC_CHAN_MAX] = {
        {NO_SUPPORT_FLAG, "BAT", TEMPINIT},        {NO_SUPPORT_FLAG, "LPA", TEMPINIT_HISI},
        {NO_SUPPORT_FLAG, "CPA", TEMPINIT_HISI},   {NO_SUPPORT_FLAG, "XO", TEMPINIT_HISI},
        {NO_SUPPORT_FLAG, "", TEMPINIT},           {NO_SUPPORT_FLAG, "", TEMPINIT},
        {NO_SUPPORT_FLAG, "", TEMPINIT},           {NO_SUPPORT_FLAG, "", TEMPINIT},
        {NO_SUPPORT_FLAG, "SIM", TEMPINIT},        {NO_SUPPORT_FLAG, "", TEMPINIT},
        {NO_SUPPORT_FLAG, "", TEMPINIT},           {NO_SUPPORT_FLAG, "", TEMPINIT},
        {NO_SUPPORT_FLAG, "USB", TEMPINIT},        {NO_SUPPORT_FLAG, "SHELL", TEMPINIT},
        {NO_SUPPORT_FLAG, "", TEMPINIT},           {NO_SUPPORT_FLAG, "", TEMPINIT},
        {NO_SUPPORT_FLAG, "", TEMPINIT},           {NO_SUPPORT_FLAG, "SD", TEMPINIT},
        {NO_SUPPORT_FLAG, "MCP", TEMPINIT},        {NO_SUPPORT_FLAG, "GPA", TEMPINIT_HISI},
        {NO_SUPPORT_FLAG, "WPA", TEMPINIT_HISI},   {NO_SUPPORT_FLAG, "", TEMPINIT},
        {NO_SUPPORT_FLAG, "", TEMPINIT},           {NO_SUPPORT_FLAG, "", TEMPINIT},
        {NO_SUPPORT_FLAG, "", TEMPINIT},           {NO_SUPPORT_FLAG, "", TEMPINIT},
        {NO_SUPPORT_FLAG, "", TEMPINIT},           {NO_SUPPORT_FLAG, "", TEMPINIT},
        {NO_SUPPORT_FLAG, "", TEMPINIT},           {NO_SUPPORT_FLAG, "", TEMPINIT},
        {NO_SUPPORT_FLAG, "", TEMPINIT},           {NO_SUPPORT_FLAG, "", TEMPINIT},};
    DRV_HKADC_DATA_AREA *p_area = (DRV_HKADC_DATA_AREA *)(SHM_BASE_ADDR + SHM_OFFSET_TEMPERATURE); /*lint !e124*/
    VOS_UINT8 *phy_tbl = p_area->phy_tbl;
    loop_num = sizeof(display_buf)/sizeof(display_buf[0]);
    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
 
    if((TEMPERATURE_MAGIC != p_area->magic_start) || (TEMPERATURE_MAGIC != p_area->magic_end))
    {
        (VOS_VOID)vos_printf("AT_QryTempInfo ERROR:tem mem is writed by others.\n");
        return AT_ERROR;
    }

    /* 根据对照表查询SIM卡温度、电池温度、USB温度、面壳温度对应的CHAN */
    for(index_chan = 0; index_chan < HKADC_LOGIC_CHAN_MAX; index_chan++)
    {
        phy_chan = phy_tbl[index_chan];
        if (HKADC_CHAN_MAX > phy_chan)
        {
            if (TEMPINIT_HISI == temp_info[index_chan].module_value)
            {
                temp_info[index_chan].module_value = p_area->chan_out[phy_chan].temp_l;
            }
            else
            {
                temp_info[index_chan].module_value = p_area->chan_out[phy_chan].temp_l * 10;
            }
            
            temp_info[index_chan].module_flag = SUPPORT_FLAG;
        }
    }

    for(loop = 0; loop < loop_num; loop++)
    {
        for(index_chan = 0; index_chan < HKADC_LOGIC_CHAN_MAX; index_chan++)
        {
            if (!(strcmp(display_buf[loop], temp_info[index_chan].module_display)))
            {
                if (SUPPORT_FLAG == temp_info[index_chan].module_flag)
                {
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                            (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),"%s:%s:%d%s",
                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                            temp_info[index_chan].module_display,temp_info[index_chan].module_value, gaucAtCrLf);
                }
                else
                {
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                            (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),"%s:%s%s",
                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                            temp_info[index_chan].module_display, gaucAtCrLf);
                }
                break;
            }
        }
    }
    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}


VOS_UINT32 AT_QryPOWERVolt(const VOS_UINT8 ucIndex)
{
    VOS_UINT16    usLength = 0;
    VOS_UINT16    volt = 0;
    VOS_UINT16    ret = 0;

    /* 入参判断 */
    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_WARN_LOG("AT_QryPOWERVolt: invalid CLIENT ID or index.");
        return AT_ERROR;
    }

    /* AT 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /*获取电源电压，并返回获取结果*/
    ret = mdrv_hkadc_get_temp(HKADC_TEMP_BATTERY, &volt,VOS_NULL_PTR, HKADC_CONV_DELAY);

    if (VOS_OK == ret)    /* 电源电压获取成功 */
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                         (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),
                                         "%s:%d%s",
                                         "^POWERVOLT", volt, gaucAtCrLf);
    }
    else
    {
       return AT_ERROR;
    }

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}















VOS_INT32 AT_QryOemLockEnable(VOS_VOID)
{
    VOS_INT32 ulRet ;
    NV_AUHT_OEMLOCK_STWICH_SRTU         OEMLOCK;
    PS_MEM_SET(&OEMLOCK, 0, sizeof(OEMLOCK));

    confidential_nv_opr_info *smem_data = NULL;
    smem_data = (confidential_nv_opr_info *)SRAM_CONFIDENTIAL_NV_OPR_ADDR; /*lint !e124*/
    if (NULL == smem_data)
    {
        vos_printf("DRV_SEC_HASH_HWOEMLOCK_CODE smem_confidential_nv_opr_flag malloc fail!\n");
        return VOS_ERROR;
    }
    /*设置纪要nv授权标记，授权读取nv*/
    smem_data->smem_confidential_nv_opr_flag = SMEM_CONFIDENTIAL_NV_OPR_FLAG_NUM;

    ulRet = NV_ReadEx(MODEM_ID_0, NV_HUAWEI_OEMLOCK_I, &OEMLOCK, sizeof(OEMLOCK));
    if (NV_OK != ulRet)
    {
        vos_printf("AT_PhyNumIsNull: Fail to read NV_HUAWEI_OEMLOCK_I");
        return VOS_ERROR;
    }
    if (DRV_OEM_SIMLOCK_ENABLE == OEMLOCK.reserved[0])
    {
        
        ulRet = VOS_ERROR;
    }
    else
    {
        
        ulRet = VOS_OK;
    }
    return ulRet;    

}



VOS_UINT32  At_SetHWLock(VOS_UINT8 ucIndex )
{
    VOS_INT32 ret = 0;
    AT_TAF_SET_HWLOCK_REQ_STRU  HWLOCK_REQ ;
    PS_MEM_SET(&HWLOCK_REQ, '\0', sizeof(AT_TAF_SET_HWLOCK_REQ_STRU));
    
    /* 设置命令无参数 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    /* 参数过多 */
    if (2 != gucAtParaIndex)
    {
        return AT_ERROR;/*是否转化为内部错误码,现在是按照装备规范只能返回OK or ERROR*/
    }
    /*参数长度判断*/
    if (0 == gastAtParaList[0].usParaLen
        || 0 == gastAtParaList[1].usParaLen)
    {
        vos_printf(" para len is error\n");
        return AT_ERROR;
    }
    /* 版本号类型目前只支持OEM和SIMLOCK的写入 */
    if ((SetOemOprLen != gastAtParaList[0].usParaLen) && (SetSimlockOprLen != gastAtParaList[0].usParaLen))
    {
        vos_printf(" compare len is error\n");
        return AT_ERROR;
    }
    /*解锁密码必须为16位*/
    if (gastAtParaList[1].usParaLen != AT_HWLOCK_PARA_LEN )
    {
        return AT_ERROR;
    }

    /*lint -e64*/
    /* 密码类型目前只支持"OEM"和"SIMLOCK"的写入 */
    if (VOS_OK != VOS_StrNiCmp(gastAtParaList[0].aucPara, "\"OEM\"", gastAtParaList[0].usParaLen) && \
        VOS_OK != VOS_StrNiCmp(gastAtParaList[0].aucPara, "\"SIMLOCK\"", gastAtParaList[0].usParaLen))
    
    {        
        return AT_ERROR;        
    }

    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
    
    if(VOS_OK == VOS_StrNiCmp(gastAtParaList[0].aucPara, "\"OEM\"", gastAtParaList[0].usParaLen))
    {
        HWLOCK_REQ.HWLOCKTYPE = AT_TAF_HWLOCKOEMTYPE; 
    }
    else /*lint !e830*/
    {
        HWLOCK_REQ.HWLOCKTYPE =  AT_TAF_HWLOCKSIMLOCKTYPE;
    }
    HWLOCK_REQ.usPara1Len = gastAtParaList[1].usParaLen;    
    PS_MEM_CPY((VOS_VOID*)HWLOCK_REQ.HWLOCKPARA,(VOS_VOID*)gastAtParaList[1].aucPara, HWLOCK_REQ.usPara1Len);
    HWLOCK_REQ.HWLOCKPARA[AT_HWLOCK_PARA_LEN] = '\0';
    ret = hw_lock_set_proc(HWLOCK_REQ.HWLOCKPARA, AT_HWLOCK_PARA_LEN, HWLOCK_REQ.HWLOCKTYPE);
    if ( 0 == ret)
    {
        return AT_OK;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32  At_TestHWlock(VOS_UINT8 ucIndex )
{
    VOS_INT32 ret = -1;
    AT_TAF_SET_HWLOCK_REQ_STRU  HWLOCK_QURY_REQ ;

    PS_MEM_SET(&HWLOCK_QURY_REQ, '\0', sizeof(AT_TAF_SET_HWLOCK_REQ_STRU));
     
    /* 设置命令无参数 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数过多 */
    if (2 != gucAtParaIndex)
    {
        return AT_ERROR;
    }
    /*参数长度*/
    if (0 == gastAtParaList[0].usParaLen
        || 0 == gastAtParaList[1].usParaLen)
    {
        return AT_ERROR;
    }
    /* 版本号类型目前只支持"OEM"和"SIMLOCK"的查询 */
    if ((SetOemOprLen != gastAtParaList[0].usParaLen) && (SetSimlockOprLen != gastAtParaList[0].usParaLen))
    {
        vos_printf(" compare len is error\n");
        return AT_ERROR;
    }
    /*解锁码必须是16位*/
    if (gastAtParaList[1].usParaLen != AT_HWLOCK_PARA_LEN )
    {
        return AT_ERROR;
    }
    
    /*lint -e64*/
    /* 密码类型目前只支持OEM和SIMLOCK的查询 */
    if (VOS_OK != VOS_StrNiCmp(gastAtParaList[0].aucPara, "\"OEM\"", gastAtParaList[0].usParaLen) && \
        VOS_OK != VOS_StrNiCmp(gastAtParaList[0].aucPara, "\"SIMLOCK\"", gastAtParaList[0].usParaLen))
    
    {        
        return AT_ERROR;        
    }
    /*受datalock保护*/
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
    
    if(VOS_OK == VOS_StrNiCmp(gastAtParaList[0].aucPara, "\"OEM\"", gastAtParaList[0].usParaLen))
    {
        HWLOCK_QURY_REQ.HWLOCKTYPE = AT_TAF_HWLOCKOEMTYPE; 
    }
    else /*lint !e830*/
    {
        HWLOCK_QURY_REQ.HWLOCKTYPE =  AT_TAF_HWLOCKSIMLOCKTYPE;
    }
    
    
    HWLOCK_QURY_REQ.usPara1Len = gastAtParaList[1].usParaLen;
    PS_MEM_CPY((VOS_VOID*)HWLOCK_QURY_REQ.HWLOCKPARA, (VOS_VOID*)gastAtParaList[1].aucPara, HWLOCK_QURY_REQ.usPara1Len);
    HWLOCK_QURY_REQ.HWLOCKPARA[AT_HWLOCK_PARA_LEN] = '\0';
    ret = hw_lock_verify_proc(HWLOCK_QURY_REQ.HWLOCKPARA, gastAtParaList[1].usParaLen, HWLOCK_QURY_REQ.HWLOCKTYPE);
    if ( 0 == ret)
    {
        return AT_OK;
    }
    else
    {
        return AT_ERROR;
    }
}










VOS_UINT32 At_TestSfm(VOS_UINT8 ucIndex)
{
    TAF_CHAR  *supportedMode = "(0,1,2)";
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:%s", 
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName, 
                                                 supportedMode);

    return AT_OK;
}





VOS_UINT32 At_CheckWiwepHex()
{
    int wepIndex = 0;
    VOS_UINT8 *PaucPara = gastAtParaList[1].aucPara;
    for(wepIndex = 0; wepIndex < gastAtParaList[1].usParaLen ; wepIndex++)
    {
        if(((PaucPara[wepIndex] >= '0') && (PaucPara[wepIndex] <= '9')) 
           || ((PaucPara[wepIndex] >= 'a') && (PaucPara[wepIndex] <= 'f')) 
           || ((PaucPara[wepIndex] >= 'A') && (PaucPara[wepIndex] <= 'F'))) 
        {
            //do nothing;
        }
        else
        {
            return AT_ERROR;
        }
    }

    if(wepIndex == gastAtParaList[1].usParaLen)
    {
        return AT_OK;
    }
    
    return AT_ERROR;
}


int set_key_press_event()
{
    struct file *file_handle = NULL;
    char lock_buf[LOCK_BUF_LEN];
    int len = -1;
    mm_segment_t old_fs = get_fs(); /*lint !e10 !e522*/
    
    (void)memset(lock_buf, 0x0, sizeof(lock_buf));/*lint !e506*/
    
    set_fs(KERNEL_DS);

    file_handle = (struct file*)filp_open(screen_state_path, O_RDWR, S_IRWXU);
    if(IS_ERR(file_handle))
    {
        printk("%s: fatal error opening \"%s\".\n", __func__,screen_state_path);
        set_fs(old_fs);
        return AT_ERROR;
    }

    len = file_handle->f_op->read(file_handle, lock_buf, len, &file_handle->f_pos); /*lint !e10 !e115*/
 
    if(len < 0) 
    {
        printk("%s: fatal error read \"%s\".\n", __func__,screen_state_path);
        (void)filp_close(file_handle, NULL);
        set_fs(old_fs);
        return AT_ERROR;
    }

    printk("%s: before keypress autosleep state is \"%s\".\n", __func__,lock_buf);

    /*如果autosleep 不是mem，则上报一次按键事件*/ 
    if (strncmp(lock_buf, "mem\n",strlen("mem\n"))) 
    {
        report_power_key_down_for_suspend();
     
        report_power_key_up_for_suspend();
    }
    
    (void)filp_close(file_handle, NULL);
    set_fs(old_fs);

    return AT_OK;
}





int set_screen_state(app_main_lock_e on)
{
    struct file *file_handle = NULL;
    char lock_buf[LOCK_BUF_LEN];
    int len = -1;
    mm_segment_t old_fs = get_fs(); /*lint !e10 !e522*/
    
    (void)memset(lock_buf, 0x0, sizeof(lock_buf));/*lint !e506*/
    
    if(APP_MAIN_LOCK == on)
    {
        len = snprintf(lock_buf, sizeof(lock_buf), "%s", on_state);
    }
    else if(APP_MAIN_UNLOCK == on)
    {
        len = snprintf(lock_buf, sizeof(lock_buf), "%s", off_state);
    }
    else
    {
        return AT_ERROR;
    }

    set_fs(KERNEL_DS);

    file_handle = (struct file*)filp_open(screen_state_path, O_RDWR, S_IRWXU);
    if(IS_ERR(file_handle))
    {
        printk("%s: fatal error opening \"%s\".\n", __func__,screen_state_path);
        set_fs(old_fs);
        return AT_ERROR;
    }
    
    len = file_handle->f_op->write(file_handle, lock_buf, len, &file_handle->f_pos); /*lint !e10 !e115*/
    if(len < 0) 
    {
        printk("%s: fatal error writing \"%s\".\n", __func__,screen_state_path);
        (void)filp_close(file_handle, NULL);
        set_fs(old_fs);
        return AT_ERROR;
    }
    
    (void)filp_close(file_handle, NULL);
    set_fs(old_fs);

    return AT_OK;
}


VOS_UINT32 At_WriteVersionINIToDefault(
    AT_CUSTOMIZE_ITEM_DFLT_ENUM_UINT8   enCustomizeItem
)
{
    VOS_UINT8 iniVersion[AT_VERSION_INI_LEN];
    
    PS_MEM_SET(iniVersion, 0, sizeof(iniVersion));
    /* 写入VERSION INI对应的NV项 */
    if (VOS_OK != NV_Write(en_NV_Item_PRI_VERSION , iniVersion, sizeof(iniVersion)))
    {
        AT_WARN_LOG("At_WriteVersionINIToDefault:WRITE NV ERROR");/*lint !e64 */
        return VOS_ERR;
    }
    
    return VOS_OK;
}


VOS_UINT32 At_WriteWebCustToDefault(
    AT_CUSTOMIZE_ITEM_DFLT_ENUM_UINT8   enCustomizeItem
)
{
    VOS_UINT8    aucWebUserTmp[AT_WEBUI_USER_NV_LEN_MAX + 1];
    VOS_UINT8    aucWiFiPinTmp[AT_WIFI_PIN_NV_LEN_MAX + 1];
    VOS_UINT8    aucWebSiteTmp[AT_WEBUI_SITE_NV_LEN_MAX + 1];
    VOS_UINT8    aucWebPwdTmp[AT_WEBUI_PWD_MAX + 1];
    
    (void)VOS_MemSet((void *)aucWebUserTmp, 0, sizeof(aucWebUserTmp));
    /* 写入WEB USER对应的NV项 */
    if (VOS_OK != NV_Write(NV_ID_WEB_USER_NAME , aucWebUserTmp, AT_WEBUI_USER_NV_LEN_MAX))
    {
        AT_WARN_LOG("At_WriteWebCustToDefault:WRITE NV ERROR");/*lint !e64 */
        return VOS_ERR;
    }
    
    (void)VOS_MemSet((void *)aucWiFiPinTmp, 0, sizeof(aucWiFiPinTmp));
    /* 写入WPS PIN对应的NV项 */
    if (VOS_OK != NV_Write(NV_ID_WPS_PIN , aucWiFiPinTmp, AT_WIFI_PIN_NV_LEN_MAX))
    {
        AT_WARN_LOG("At_WriteWebCustToDefault:WRITE NV ERROR");/*lint !e64 */
        return VOS_ERR;
    }
    
    (void)VOS_MemSet((void *)aucWebPwdTmp, 0, sizeof(aucWebPwdTmp));
    /* 写WEBPWD对应的NV项 */
    if (VOS_OK != NV_Write(en_NV_Item_WEB_ADMIN_PASSWORD_NEW_I, aucWebPwdTmp, AT_WEBUI_PWD_MAX))
    {
        AT_WARN_LOG("At_WriteWebCustToDefault:WRITE NV ERROR");/*lint !e64 */
        return VOS_ERR;
    }
    
    (void)VOS_MemSet((void *)aucWebSiteTmp, 0, sizeof(aucWebSiteTmp));
    /* 写入WEB SITE对应的NV项 */
    if (VOS_OK != NV_Write(NV_ID_WEB_SITE , aucWebSiteTmp, AT_WEBUI_SITE_NV_LEN_MAX))
    {
        AT_WARN_LOG("At_WriteWebCustToDefault:WRITE NV ERROR");/*lint !e64 */
        return VOS_ERR;
    }
    
    return VOS_OK;
}



/*****************************************************************************
 函 数 名  : at_get_ftyreset_set_flag
 功能描述  : 查询当前是否设置过AT^FTYRESET=0命令
 输入参数  : NA
 输出参数  : 无
 返 回 值  : SET_FTYRESET_TYPE;
 调用函数  :
 被调函数  :

 修改历史      :
*****************************************************************************/
SET_FTYRESET_TYPE at_get_ftyreset_set_flag(void)
{
    return g_set_ftyreset_flag;
}

/*****************************************************************************
 函 数 名  : AT_SetFtyResetPara
 功能描述  : 恢复出厂设置操作AT^FTYRESET设置命令处理
 输入参数  : ucIndex - 用户索引
 输出参数  : 无
 返 回 值  : AT_OK - 成功
             AT_DEVICE_OTHER_ERROR或 AT_DATA_UNLOCK_ERROR - 失败
 调用函数  :
 被调函数  :

 修改历史      :

*****************************************************************************/
VOS_UINT32 AT_SetFtyResetPara(VOS_UINT8 ucIndex)
{

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数个数过多 */
    if (gucAtParaIndex != 1)
    {
        return AT_ERROR;
    }

    if (0 != gastAtParaList[0].ulParaValue)
    {
        return AT_ERROR;
    }

    /*如果已下发过一次恢复出厂设置命令，直接返回ok*/
    if ( SET_DONE == g_set_ftyreset_flag )
    {
        return AT_OK;
    }

    /*设置已下发恢复出厂设置标记*/
    g_set_ftyreset_flag = SET_DONE;
    return AT_OK;
}



/*****************************************************************************
 函 数 名  : AT_QryFtyResetPara
 功能描述  : 恢复出厂设置操作AT^FTYRESET查询命令处理
 输入参数  : ucIndex - 用户索引
 输出参数  : 无
 返 回 值  : AT_OK - 成功
             AT_DEVICE_OTHER_ERROR或 AT_DATA_UNLOCK_ERROR - 失败
 调用函数  :
 被调函数  :

 修改历史      :

*****************************************************************************/
VOS_UINT32 AT_QryFtyResetPara(VOS_UINT8 ucIndex)
{
    RESTORE_STATE_TYPE restore_state = RESTORE_FAIL;
    NODE_STATE_TYPE node_state = APP_START_INVALID; /*lint !e578*/
    VOS_UINT32 ulRst = AT_OK;
    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /*获取节点状态*/
    node_state = drv_get_ftyreset_node_state();

    if ( SET_DONE == at_get_ftyreset_set_flag() )
    {
        restore_state = RESTORE_PROCESSING;

        /*如果恢复出厂设置完成*/
        if (APP_RESTORE_OK == node_state)
        {
            restore_state = RESTORE_OK;
        }

        /*如果应用启动ok，上报字串^NORSTFACT字串*/
        if (APP_START_READY == node_state)
        {
            if (REVERT_FAIL == g_revert_flag)
            {
                ulRst = NVM_RevertFNV();
                if (ulRst == AT_SUCCESS)
                {
                    g_revert_flag = REVERT_SUCCESS;
                }
                else
                {
                    return AT_ERROR;
                }
            }
            AT_PhSendRestoreFactParmNoReset();
        }
    }
    else
    {
        restore_state = RESTORE_FAIL;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    restore_state);
    return AT_OK;
}


VOS_UINT32 At_QrySnPara(VOS_UINT8 ucIndex)
{
    TAF_PH_SERIAL_NUM_STRU stSerialNum;
    TAF_UINT16            usLength = 0;

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    /* 从NV中读取 Serial Num,上报显示，返回 */

    PS_MEM_SET(&stSerialNum, 0, sizeof(TAF_PH_SERIAL_NUM_STRU));

    usLength = TAF_SERIAL_NUM_NV_LEN;
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_Serial_Num, stSerialNum.aucSerialNum, usLength))
    {
        AT_WARN_LOG("At_QrySnPara:WARNING:NVIM Read en_NV_Item_Serial_Num falied!");
        return AT_ERROR;
    }
    else
    {
        /*将sn的buf的后四位清0，目前仅使用前16位*/
        PS_MEM_SET((stSerialNum.aucSerialNum + TAF_SERIAL_NUM_LEN), 0, 4 * sizeof(stSerialNum.aucSerialNum[0]));
        usLength = 0;
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, \
            (TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%s:", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, \
            (TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%s", stSerialNum.aucSerialNum);
    }
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}



VOS_UINT32 AT_QryLteAntInfo(TAF_UINT8 ucIndex)
{
    LTE_ANT_INFO_STRU lte_ant_info;
    VOS_UINT8 output_buf[MAX_ANT_NUM][OUTPUT_BUF_LENGTH] = {"ANT0",
              "ANT1", "ANT2", "ANT3", "ANT4", "ANT5", "ANT6", "ANT7"};
    VOS_UINT32 iRet            = 0;
    VOS_UINT8 ant_num_index    = 0;
    VOS_UINT8 band_num_index   = 0;

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    PS_MEM_SET(&lte_ant_info, 0, sizeof(lte_ant_info));

    iRet = NV_Read(NV_ID_LTE_ANT_INFO, &lte_ant_info, sizeof(LTE_ANT_INFO_STRU));

    if(NV_OK != iRet)
    {
        vos_printf("AT_QryLteAntInfo: read nv %d failed!\n", NV_ID_LTE_ANT_INFO);
        return AT_ERROR;
    }
    
    if (lte_ant_info.ant_num > MAX_ANT_NUM)
    {
        vos_printf("AT_QryLteAntInfo: NV %d: ANT NUM ERROR!\n", NV_ID_LTE_ANT_INFO);
        return AT_ERROR;
    }

    for (ant_num_index = 0; ant_num_index < lte_ant_info.ant_num; ant_num_index++)
    {
        for (band_num_index = 0; band_num_index < lte_ant_info.ant_info[ant_num_index].band_num_main_div_flag.band_num;
                                 band_num_index++)
        {
            snprintf((VOS_INT8*)(&output_buf[ant_num_index][0]) + AT_STRLEN((VOS_INT8*)(&output_buf[ant_num_index][0])),
                        OUTPUT_BUF_LENGTH - AT_STRLEN((VOS_INT8*)(&output_buf[ant_num_index][0])),
                        ",B%d", lte_ant_info.ant_info[ant_num_index].band_idx[band_num_index]);
        }
    }

    gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                      "%s:%d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      lte_ant_info.ant_num,
                                      gaucAtCrLf);

    for (ant_num_index = 0; ant_num_index < lte_ant_info.ant_num; ant_num_index++)
    {
       gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                          "%s:%s%s",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          output_buf[ant_num_index],
                                          gaucAtCrLf);
    }
    return AT_OK;
}


TAF_UINT32 At_MacConvertWithColon(TAF_UINT8 *pMacDst, TAF_UINT8 *pMacSrc, TAF_UINT16 usSrcLen)
{
    TAF_UINT16 ulLoop = 0;
    VOS_UINT32                          ulDstMacOffset = 0;
    VOS_UINT32                          ulSrcMacOffset = 0;
    
    if(NULL == pMacDst || NULL == pMacSrc || 0 == usSrcLen)
    {
        return AT_FAILURE;
    }
    
    /* MAC地址长度检查: 必须12位 */
    if (AT_PHYNUM_MAC_LEN != usSrcLen)
    {
        return AT_FAILURE;
    }
    
    /* MAC地址格式匹配: 7AFEE22111E4=>7A:FE:E2:21:11:E4*/
    for (ulLoop = 0; ulLoop < (AT_PHYNUM_MAC_COLON_NUM + 1); ulLoop++)
    {
        PS_MEM_CPY(&pMacDst[ulDstMacOffset], &pMacSrc[ulSrcMacOffset], AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS);
        
        ulDstMacOffset += AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS;
        ulSrcMacOffset += AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS;
        
        pMacDst[ulDstMacOffset] = ':';
        
        ulDstMacOffset++;
    }
    
    pMacDst[AT_PHYNUM_MAC_LEN + AT_PHYNUM_MAC_COLON_NUM] = '\0';
    
    return AT_SUCCESS;
}



VOS_UINT32 Mbb_AT_SetWiFiRxPara(VOS_VOID)
{
    WLAN_AT_WIRX_STRU    wifiRxStru;
    
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 > gucAtParaIndex || 3 < gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 参数值判断 */
    if(AT_FEATURE_DISABLE != gastAtParaList[0].ulParaValue && AT_FEATURE_ENABLE != gastAtParaList[0].ulParaValue)
    {
        return AT_ERROR;
    }
    
    /* 初始化 */
    (void)VOS_MemSet((void *)&wifiRxStru, 0, sizeof(wifiRxStru));
    
    if(1 == gucAtParaIndex)
    {
        wifiRxStru.onoff = (WLAN_AT_FEATURE_TYPE)gastAtParaList[0].ulParaValue;
        wifiRxStru.src_mac[0] = '\0';
        wifiRxStru.dst_mac[0] = '\0';
    }
    else if(2 == gucAtParaIndex)
    {
        wifiRxStru.onoff = (WLAN_AT_FEATURE_TYPE)gastAtParaList[0].ulParaValue;
        
        if(AT_SUCCESS != At_MacConvertWithColon(wifiRxStru.src_mac, gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen))
        {
            return AT_ERROR;
        }
        
        wifiRxStru.dst_mac[0] = '\0';
    }
    else if(3 == gucAtParaIndex)
    {
        wifiRxStru.onoff = (WLAN_AT_FEATURE_TYPE)gastAtParaList[0].ulParaValue;
        
        if(AT_SUCCESS != At_MacConvertWithColon(wifiRxStru.src_mac, gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen))
        {
            return AT_ERROR;
        }
        
        if(AT_SUCCESS != At_MacConvertWithColon(wifiRxStru.dst_mac, gastAtParaList[2].aucPara, gastAtParaList[2].usParaLen))
        {
            return AT_ERROR;
        }
    }
    
    /* 设置WIFI接收机 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiRX(&wifiRxStru))
    {
        return AT_ERROR;
    }
    return AT_OK;
}




VOS_UINT32 Mbb_AT_SetWiFiEnable(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* 参数取值判断 */
    if(gastAtParaList[0].ulParaValue > AT_WIENABLE_TEST)
    {
        return AT_ERROR;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
    {
        return AT_ERROR;
    }
    
    if (AT_RETURN_SUCCESS != WlanATSetWifiEnable((WLAN_AT_WIENABLE_TYPE)gastAtParaList[0].ulParaValue)) /*lint !e830*/
    {
        return AT_ERROR;
    }
	return AT_OK; /*lint !e539*/
}



VOS_UINT32 Mbb_AT_SetWiFiModePara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    if (AT_RETURN_SUCCESS != WlanATSetWifiMode((WLAN_AT_WIMODE_TYPE)gastAtParaList[0].ulParaValue)) /*lint !e830*/
    {
        return AT_ERROR;
    }
	return AT_OK; /*lint !e539*/
}


VOS_UINT32 Mbb_AT_SetWiFiBandPara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    if (AT_RETURN_SUCCESS != WlanATSetWifiBand((WLAN_AT_WIBAND_TYPE)gastAtParaList[0].ulParaValue))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWiFiFreqPara(VOS_VOID)
{
    WLAN_AT_WIFREQ_STRU wifiReqStru;
    
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex && 2 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 初始化 */
    (void)VOS_MemSet((void *)&wifiReqStru, 0, sizeof(wifiReqStru));
    
    if(1 == gucAtParaIndex)
    {
        wifiReqStru.value = (VOS_UINT16)gastAtParaList[0].ulParaValue;
        wifiReqStru.offset = 0;
    }
    else
    {
        wifiReqStru.value = (VOS_UINT16)gastAtParaList[0].ulParaValue;
        wifiReqStru.offset = (VOS_UINT16)gastAtParaList[1].ulParaValue;
    }
    
    if (AT_RETURN_SUCCESS != WlanATSetWifiFreq(&wifiReqStru))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWiFiRatePara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    if (AT_RETURN_SUCCESS != WlanATSetWifiDataRate(gastAtParaList[0].ulParaValue))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWiFiPowerPara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 设置WIFI功率 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiPOW(gastAtParaList[0].ulParaValue))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWiFiTxPara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 参数值判断 */
    if(AT_FEATURE_DISABLE != gastAtParaList[0].ulParaValue && AT_FEATURE_ENABLE != gastAtParaList[0].ulParaValue)
    {
        return AT_ERROR;
    }
    
    /* 设置WIFI功率 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiTX((WLAN_AT_FEATURE_TYPE)gastAtParaList[0].ulParaValue))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWiFiPacketPara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 清除WIFI统计包为0 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiRPCKG((VOS_INT32)gastAtParaList[0].ulParaValue))
    {
        return AT_ERROR;
    }
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWifiInfoPara(VOS_VOID)
{
    VOS_UINT16  usLen = 0;
    VOS_CHAR  *resultBuffer = VOS_NULL;
    WLAN_AT_WIINFO_STRU   wifiInfoStru;
    
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if(1 != gucAtParaIndex && 2 != gucAtParaIndex)
    {
        return AT_ERROR;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 初始化 */
    (void)VOS_MemSet((void *)&wifiInfoStru, 0, sizeof(wifiInfoStru));
    
    if(1 == gucAtParaIndex)
    {
        if (0 == gastAtParaList[0].usParaLen)
        {
            return AT_ERROR;
        }
        
        wifiInfoStru.type = (WLAN_AT_WIINFO_TYPE_ENUM)gastAtParaList[0].ulParaValue;
        wifiInfoStru.member.group = 0;//默认值
    }
    else if(2 == gucAtParaIndex)
    {
        if (0 == gastAtParaList[0].usParaLen
            || 0 == gastAtParaList[1].usParaLen)
        {
            return AT_ERROR;
        }
        
        wifiInfoStru.type = (WLAN_AT_WIINFO_TYPE_ENUM)gastAtParaList[0].ulParaValue;
        wifiInfoStru.member.group = (VOS_INT32)gastAtParaList[1].ulParaValue;
    }
    
    /* 获取WIFI信息 */
    if (AT_RETURN_SUCCESS != WlanATGetWifiInfo(&wifiInfoStru))
    {
        return AT_ERROR;
    }
    
    resultBuffer = (VOS_CHAR *)wifiInfoStru.member.content;
    while(0 != VOS_StrLen((VOS_CHAR *)resultBuffer))
    {
        usLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                        (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                        "^WIINFO:%s%s",
                                        resultBuffer, gaucAtCrLf);
                                        
        resultBuffer += VOS_StrLen(resultBuffer) + 1;
    }
    
    gstAtSendData.usBufLen = usLen - (VOS_UINT16)VOS_StrLen((char *)gaucAtCrLf);
    return AT_OK;
}


VOS_UINT32 Mbb_AT_SetWifiPaRangePara(VOS_VOID)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }
    
    /* 参数个数判断 */
    if (1 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* 参数个数判断 */
    if (1 != gastAtParaList[0].usParaLen)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    
    /* 小写字母转换 */
    gastAtParaList[0].aucPara[gastAtParaList[0].usParaLen] = '\0';
    (void)VOS_StrToLower((VOS_CHAR*)gastAtParaList[0].aucPara);
    
    /* 参数值判断 */
    if('l' != gastAtParaList[0].aucPara[0] && 'h' != gastAtParaList[0].aucPara[0])
    {
        return AT_ERROR;
    }
    
    /* 设置WIFI PA模式 */
    if (AT_RETURN_SUCCESS != WlanATSetWifiParange((WLAN_AT_WiPARANGE_TYPE)gastAtParaList[0].aucPara[0]))
    {
        return AT_ERROR;
    }
    return AT_OK;
}

#define TMMI_PARA_LEN_MAX    (5) /* MMI test */

VOS_UINT32 Mbb_AT_SetTmmiPara(VOS_VOID)
{
    VOS_UINT32                          ulResult;
    VOS_UINT8                           aucFacInfo[AT_FACTORY_INFO_LEN];
    VOS_UINT32 temp_mmi = 0;
    /* 参数过多 */
    if (gucAtParaIndex > 1)
    {
        return  AT_TOO_MANY_PARA;
    }


    /*参数长度过长*/
    if (TMMI_PARA_LEN_MAX < gastAtParaList[0].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    VOS_MemSet(aucFacInfo, 0x00, AT_FACTORY_INFO_LEN);

    /* 写入en_NV_Item_Factory_Info，需偏移24个字节，长度4为四个字节，因此需要先读 */
    ulResult = NV_ReadEx(MODEM_ID_0, en_NV_Item_Factory_Info, aucFacInfo, AT_FACTORY_INFO_LEN);

    if (NV_OK != ulResult)
    {
        AT_ERR_LOG("AT_SetTmmiPara: NV Read Fail!");
        return AT_ERROR;
    }

    if (gastAtParaList[0].ulParaValue > AT_MMI_RESULT_MAX)
    {
        return AT_ERROR;
    }

    temp_mmi = gastAtParaList[0].ulParaValue;

    VOS_MemCpy(&aucFacInfo[AT_MMI_TEST_FLAG_OFFSET], (VOS_UINT8*)(&temp_mmi), AT_MMI_TEST_FLAG_LEN);

    ulResult = NV_WriteEx(MODEM_ID_0, en_NV_Item_Factory_Info, aucFacInfo, AT_FACTORY_INFO_LEN);

    if (NV_OK != ulResult)
    {
        return AT_ERROR;
    }
    else
    {
        return AT_OK;
    }
}


VOS_UINT32 Mbb_AT_SetChrgEnablePara(VOS_VOID)
{
    /* 输入参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数过多 */
    if (1 != gucAtParaIndex && 2 != gucAtParaIndex)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    /*参数长度过长*/
    if (0 == gastAtParaList[0].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        if(2 == gucAtParaIndex)
        {
            if (0 == gastAtParaList[1].usParaLen)
            {
                return  AT_CME_INCORRECT_PARAMETERS;
            }
        }
    }
    
    /* 是否支持电池 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_CHARGE))
    {
        return AT_ERROR;
    }
    
    if(2 == gucAtParaIndex)
    {
        /* 调用充电接口 */
        if(CHG_OK != chg_tbat_charge_mode_set(gastAtParaList[0].ulParaValue, gastAtParaList[1].ulParaValue))
        {
            return AT_ERROR;
        }
    }
    else
    {
        /* 调用放电和补电接口 */
        if(CHG_OK != chg_tbat_charge_mode_set(gastAtParaList[0].ulParaValue, 0))
        {
            return AT_ERROR;
        }
    }

    return AT_OK;
}



VOS_UINT32  Mbb_AT_SetPhyNumPara(AT_PHYNUM_TYPE_ENUM_UINT32 enSetType, MODEM_ID_ENUM_UINT16 enModemId)
{
    VOS_UINT32                          ulRet;
	
    switch(enSetType)
    {
        case AT_PHYNUM_TYPE_IMEI:
            ulRet = AT_UpdateImei(enModemId, gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
            break;
            
        case AT_PHYNUM_TYPE_SVN:
            ulRet = AT_UpdateSvn(enModemId, gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
            break;

        case AT_PHYNUM_TYPE_MAC:
            if (TRUE == get_lan_support())
            {
                ulRet = AT_UpdateMacPara(gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
            }
            else
            {
                ulRet = AT_ERROR;
            }
            break;
        case AT_PHYNUM_TYPE_MACWLAN:
            if (BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
            {
                if (TRUE == get_lan_support())
                {
                   ulRet = AT_ERROR;
                }
                else               
                {
                   ulRet = AT_UpdateMacPara(gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
                }
            }
            else
            {
                ulRet = AT_ERROR;
            }
            break;
         case AT_PHYNUM_TYPE_ESN:
            ulRet = AT_ERROR;
            break;
            
         case AT_PHYNUM_TYPE_UMID:
            ulRet = AT_ERROR;
            break;   

         case AT_PHYNUM_TYPE_BUTT:
            ulRet = AT_ERROR;
            break;   

         default:
            ulRet = AT_ERROR;
            break;
    }
	
	return ulRet;
}

/*****************************************************************************
 函 数 名  : At_SecCheckSamePortNUM
 功能描述  :不能同时设置两个以上的PCUI、MODEM、NDIS及GPS接口，
            并且有且只能有一个PCUI、和NDIS接口
 输入参数  : 
 输出参数  : 无
 返 回 值  : VOS_UINT8 PortNum
 调用函数  :
 被调函数  :

*****************************************************************************/
VOS_VOID At_SecCheckSamePortNUM(VOS_UINT32  ucTempnum)
{

    /*记录设置的每类端口个数*/
    switch(ucTempnum)
    {
        case AT_DEV_MODEM:
        case AT_DEV_4G_MODEM:
        {
            gportTypeNum[MODEMNUM]++;
            break;
        }

        case AT_DEV_PCUI:
        case AT_DEV_4G_PCUI:
        {
            gportTypeNum[PCUINUM]++;
            break;
        }

        case AT_DEV_GPS:
        case AT_DEV_GPS_CONTROL:
        case AT_DEV_4G_GPS:
        {
           gportTypeNum[GPSNUM]++;
            break;
        }

        case AT_DEV_NDIS:
        case AT_DEV_4G_NDIS:
        case AT_DEV_NCM:
        {
           gportTypeNum[NDISNUM]++;
            break;
        }

        default :
            break;
    }
}

VOS_VOID clearportTypeNum(VOS_VOID)
{
    VOS_UINT8 i = 0;
    for(i = 0; i < MAXPORTTYPENUM; i++)
    {
        gportTypeNum[i] = 0;
    }
}


TAF_UINT32  Mbb_AT_SetFDac_Para_Valid(TAF_VOID)
{
	TAF_UINT16                           usDAC;

    /*调用 LTE 模的接口分支*/
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return AT_ERROR;
    }
    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return AT_ERROR;
    }
    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    /* 参数不符合要求 */
    if (gucAtParaIndex != 1)
    {
        return AT_ERROR;
    }
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_ERROR;
    }

    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_ERROR;
    }

    usDAC = (VOS_UINT16)gastAtParaList[0].ulParaValue;

    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        if (usDAC > WDSP_MAX_TX_AGC)
        {
            return AT_ERROR;
        }
        else
        {
            g_stAtDevCmdCtrl.usFDAC = (VOS_UINT16)gastAtParaList[0].ulParaValue;
			return AT_OK;
        }
    }
    else
    {
        if (usDAC > GDSP_MAX_TX_VPA)
        {
            return AT_ERROR;
        }
        else
        {
            g_stAtDevCmdCtrl.usFDAC = (VOS_UINT16)gastAtParaList[0].ulParaValue;
			return AT_OK;
        }
    }
}


VOS_UINT32 Mbb_AT_QryTmmiPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           aucFacInfo[AT_FACTORY_INFO_LEN];
    VOS_UINT32                           ucMmiFlag;
    VOS_UINT32                          ulResult;
    VOS_UINT32 temp_mmi = 0;
    ulResult = NV_ReadEx(MODEM_ID_0, en_NV_Item_Factory_Info,
                       aucFacInfo,
                       AT_FACTORY_INFO_LEN);

    if (NV_OK != ulResult)
    {
        return AT_ERROR;
    }

    VOS_MemCpy((VOS_UINT8*)(&temp_mmi), &aucFacInfo[AT_MMI_TEST_FLAG_OFFSET], AT_MMI_TEST_FLAG_LEN);
    if (AT_MMI_RESULT_MAX < temp_mmi)
    {
        ucMmiFlag = 0;
    }
    else
    {
        ucMmiFlag = temp_mmi;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            "%s:%d",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            ucMmiFlag);

    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryChrgEnablePara(VOS_UINT8 ucIndex)
{
    CHG_TCHRENABLE_TYPE chrenable_state;
    
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_CHARGE) )
    {
        return AT_ERROR;
    }

    (void)VOS_MemSet((void *)&chrenable_state, 0, sizeof(chrenable_state));
    
    if(CHG_OK != chg_tbat_get_tchrenable_status(&chrenable_state))
    {
        return AT_ERROR;
    }
    
    if(AT_TCHRENABEL_SWITCH_CHARG_OPEN == chrenable_state.chg_state)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                                       "%s:%d,%d",
                                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                       chrenable_state.chg_state,
                                                       chrenable_state.chg_mode);
    }
    else
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                                       "%s:%d",
                                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                       chrenable_state.chg_state);
    }

    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWifiPaRangePara (VOS_UINT8 ucIndex)
{

    WLAN_AT_WiPARANGE_TYPE   ucWifiPaType;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    ucWifiPaType = WlanATGetWifiParange();
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%c",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiPaType );
    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWiFiPacketPara(VOS_UINT8 ucIndex)
{

    WLAN_AT_WIRPCKG_STRU   wifiPckStru;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void *)&wifiPckStru, 0, sizeof(wifiPckStru));

    /* 获取WIFI接收机误包码 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiRPCKG(&wifiPckStru))
    {
        return AT_ERROR;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d,%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    wifiPckStru.good_result, wifiPckStru.bad_result);
    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWiFiRxPara(VOS_UINT8 ucIndex)
{

    WLAN_AT_WIRX_STRU   wifiRxStru;
    (void)VOS_MemSet((void *)&wifiRxStru, 0, sizeof(wifiRxStru));

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 获取WIFI RX信息 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiRX(&wifiRxStru))
    {
        return AT_ERROR;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    wifiRxStru.onoff);
    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWiFiRatePara(VOS_UINT8 ucIndex)
{

    VOS_UINT32 wifiDataRate;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 读取WIFI速率 */
    wifiDataRate = WlanATGetWifiDataRate();
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    wifiDataRate);
    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWiFiFreqPara(VOS_UINT8 ucIndex)
{

    WLAN_AT_WIFREQ_STRU wifiReqStru;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void *)&wifiReqStru, 0, sizeof(wifiReqStru));
    
    if (AT_RETURN_SUCCESS != WlanATGetWifiFreq(&wifiReqStru))
    {
        return AT_ERROR;
    }

    /* 查询设置值 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    wifiReqStru.value);
    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWiFiBandPara(VOS_UINT8 ucIndex)
{

    WLAN_AT_BUFFER_STRU     strBuf;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void *)&strBuf, 0, sizeof(strBuf));
    
    /* 读取WIFI带宽 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiBand(&strBuf))
    {
        return AT_ERROR;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
    return AT_OK;
}


VOS_UINT32 Mbb_AT_QryWiFiModePara(VOS_UINT8 ucIndex)
{

    WLAN_AT_BUFFER_STRU     strBuf;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void *)&strBuf, 0, sizeof(strBuf));

    /* 读取WIFI模式 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiMode(&strBuf))
    {
        return AT_ERROR;
    }
    
    /* WIFI模块模式 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
    return AT_OK;
}


VOS_UINT32  Mbb_At_QryVersion(VOS_UINT8 ucIndex )
{
    VOS_UINT16                          ulLen = 0;
    TAF_NVIM_CS_VER_STRU                stCsver = {0};
    VOS_UINT8                           *pInfoTemp = NULL;
    VOS_UINT8                           tempInfoArray[TAF_MAX_VERSION_VALUE_LEN] = {0};
    VOS_UINT8                           iniVersion[AT_VERSION_INI_LEN] = {0};
    
    gstAtSendData.usBufLen = 0;
    
    /* 1. 读取编译时间 */
    pInfoTemp = NULL;
    pInfoTemp = (VOS_UINT8 *)bsp_version_get_build_date_time();
    if(NULL == pInfoTemp)
    {
        printk("%s: build time read error.", __func__);
        return AT_ERROR;
    }

    version_info_fill(tempInfoArray, pInfoTemp);
    /*版本编译时间格式转换，将时间戳中连续两个空格的后一个空格用0替换 */
    (void)At_ZeroReplaceBlankInString((VOS_UINT8*)(&tempInfoArray[0]), VOS_StrLen((VOS_CHAR *)(&tempInfoArray[0])));
    version_info_build(ucIndex, (VOS_UINT8*)"BDT", tempInfoArray);

    /* 2. 读取软件外部版本号 */
    pInfoTemp = NULL;
    pInfoTemp = (VOS_UINT8*)bsp_version_get_firmware();
    if(NULL == pInfoTemp)
    {
        printk("%s: software version read error.", __func__);
        return AT_ERROR;
    }
    version_info_fill(tempInfoArray, pInfoTemp);
    version_info_build(ucIndex, (VOS_UINT8*)"EXTS", tempInfoArray);
    
    /* 3. 读取软件内部版本号 */
    if (VOS_FALSE == g_bAtDataLocked)
    {
        pInfoTemp = NULL;
        pInfoTemp = (VOS_UINT8*)bsp_version_get_firmware();
        if(NULL == pInfoTemp)
        {
            printk("%s: software version read error.", __func__);
            return AT_ERROR;
        }
        version_info_fill(tempInfoArray, pInfoTemp);
        version_info_build(ucIndex, (VOS_UINT8*)"INTS", tempInfoArray);
    }
    else
    {
        (void)memset(tempInfoArray, 0, TAF_MAX_VERSION_VALUE_LEN);
        version_info_build(ucIndex, (VOS_UINT8*)"INTS", tempInfoArray);
    }

    /* 4. 从OEM分区读取外部ISO版本号 */
    (void)memset(tempInfoArray, 0, TAF_MAX_VERSION_VALUE_LEN);
    ulLen = TAF_MAX_VERSION_VALUE_LEN;
    if (VOS_OK != mdrv_dload_getwebuiver((char*)tempInfoArray, TAF_MAX_VERSION_VALUE_LEN))
    {
        printk("%s: outer iso ver read error.", __func__);
        return AT_ERROR;
    }
    (void)At_DelCtlAndBlankCharWithEndPadding(tempInfoArray, &ulLen);
    version_info_build(ucIndex, (VOS_UINT8*)"EXTD", tempInfoArray);
    
    /* 5. 从OEM分区读取内部ISO版本号 */
    if (VOS_FALSE == g_bAtDataLocked)
    {
        (void)memset(tempInfoArray, 0, TAF_MAX_VERSION_VALUE_LEN);
        ulLen = TAF_MAX_VERSION_VALUE_LEN;
        if (VOS_OK != mdrv_dload_getwebuiver((char*)tempInfoArray, TAF_MAX_VERSION_VALUE_LEN))
        {
            printk("%s: outer iso ver read error.", __func__);
            return AT_ERROR;
        }
        (void)At_DelCtlAndBlankCharWithEndPadding(tempInfoArray, &ulLen);
        version_info_build(ucIndex, (VOS_UINT8*)"INTD", tempInfoArray);
    }
    else
    {
        (void)memset(tempInfoArray, 0, TAF_MAX_VERSION_VALUE_LEN);
        version_info_build(ucIndex, (VOS_UINT8*)"INTD", tempInfoArray);
    }
    
    /* 6. 获取外部硬件版本号  */
    (void)memset(tempInfoArray, 0, TAF_MAX_VERSION_VALUE_LEN);
    if (VOS_OK != BSP_HwGetHwVersion((char *)tempInfoArray, TAF_MAX_VERSION_VALUE_LEN))
    {
        printk("%s: outer hardware version read error.", __func__);
        return AT_ERROR;
    }
    version_info_build(ucIndex, (VOS_UINT8*)"EXTH", tempInfoArray);
  
    /* 7. 获取内部硬件版本号  */
    if (VOS_FALSE == g_bAtDataLocked)
    {
        (void)memset(tempInfoArray, 0, TAF_MAX_VERSION_VALUE_LEN);
        if (VOS_OK != BSP_HwGetHwVersion((char *)(tempInfoArray), TAF_MAX_VERSION_VALUE_LEN))
        {
            printk("%s: inner hardware version read error.", __func__);
            return AT_ERROR;
        }
        version_info_build(ucIndex, (VOS_UINT8*)"INTH", tempInfoArray);
    }
    else
    {
        (void)memset(tempInfoArray, 0, TAF_MAX_VERSION_VALUE_LEN);
        version_info_build(ucIndex, (VOS_UINT8*)"INTH", tempInfoArray);
    }

    /* 8. 获取外部产品名 */
    (void)memset(tempInfoArray, 0, TAF_MAX_VERSION_VALUE_LEN);
    if (VOS_OK != mdrv_dload_get_productname((char*)tempInfoArray, TAF_MAX_VERSION_VALUE_LEN))
    {
        printk("%s: outer product name read error.", __func__);
        return AT_ERROR;
    }
    version_info_build(ucIndex, (VOS_UINT8*)"EXTU", tempInfoArray);

    /* 9. 获取内部产品名 */
    if (VOS_FALSE == g_bAtDataLocked)
    {
        (void)memset(tempInfoArray, 0, TAF_MAX_VERSION_VALUE_LEN);
        if (VOS_OK != BSP_MNTN_GetProductIdInter((char*)tempInfoArray, TAF_MAX_VERSION_VALUE_LEN))
        {
            printk("%s: inner product name read error.", __func__);
            return AT_ERROR;
        }
        version_info_build(ucIndex, (VOS_UINT8*)"INTU", tempInfoArray);
    }
    else
    {
        (void)memset(tempInfoArray, 0, TAF_MAX_VERSION_VALUE_LEN);
        version_info_build(ucIndex, (VOS_UINT8*)"INTU", tempInfoArray);
    }

    /* 10. 生产可配置版本号 */
    (void)memset(tempInfoArray, 0, TAF_MAX_VERSION_VALUE_LEN);
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_Csver, &(stCsver.usCsver), sizeof(stCsver.usCsver)))
    {
        printk("%s: en_NV_Item_Csver read error.", __func__);
        return AT_ERROR;
    }
    (void)snprintf((char*)tempInfoArray, TAF_MAX_VERSION_VALUE_LEN, "%d", stCsver.usCsver);
    version_info_build(ucIndex, (VOS_UINT8*)"CFG", tempInfoArray);
    
    /* 11. PRL版本号 */
    (void)memset(tempInfoArray, 0, TAF_MAX_VERSION_VALUE_LEN);
    version_info_build(ucIndex, (VOS_UINT8*)"PRL", tempInfoArray);
    /* 13. INI配置文件版本号 */
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_PRI_VERSION, iniVersion, sizeof(iniVersion)))
    {
        printk("%s: en_NV_Item_PRI_VERSION read error.", __func__);
        return AT_ERROR;
    }
    version_info_fill(tempInfoArray, iniVersion);
    gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                          "%s:%s:%s%s",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          "INI",
                                          tempInfoArray,
                                          gaucAtCrLf);
    if(0 < gstAtSendData.usBufLen)
    {
        gstAtSendData.usBufLen = gstAtSendData.usBufLen - (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf);
    }
    
    return AT_OK;
}


#define SB_SERIAL_NUM_LEN    (15)

TAF_UINT32 AT_SetSbSnPara(TAF_UINT8 ucIndex)
{
    SB_SERIAL_NUM_STRU stSerialNum;
    PS_MEM_SET((char*)(&stSerialNum), 0, sizeof(stSerialNum));

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("AT_SetSbSnPara:WARNING:ucCmdOptType err!");
        return AT_ERROR;
    }

    /* 参数个数不为1 */
    if (gucAtParaIndex != 1)
    {
        AT_WARN_LOG("AT_SetSbSnPara:WARNING:ucCmdOpt num err!");
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 如果参数长度不等于15，直接返回错误 */
    if (SB_SERIAL_NUM_LEN != gastAtParaList[0].usParaLen)
    {
        AT_WARN_LOG("AT_SetSbSnPara:WARNING:sn len err!");
        return AT_SN_LENGTH_ERROR;
    }

    /* 检查当前参数是否为数字字母字符串,不是则直接返回错误 */
    if (AT_FAILURE == At_CheckNumCharString(gastAtParaList[0].aucPara,
                                            gastAtParaList[0].usParaLen))
    {
        AT_WARN_LOG("AT_SetSbSnPara:WARNING:sn str err!");
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 拷贝设置的15位SN参数到结构体变量stSerialNum.aucSerialNum中 */
    PS_MEM_CPY(stSerialNum.serial_num, gastAtParaList[0].aucPara, SB_SERIAL_NUM_LEN);

    if (NV_OK != NV_WriteEx(MODEM_ID_0, NV_SB_SERIAL_NUM,
                          stSerialNum.serial_num,
                          TAF_SERIAL_NUM_NV_LEN))
    {
        AT_WARN_LOG("AT_SetSbSnPara:WARNING:NVIM Write NV_SB_SERIAL_NUM_STRU failed!");
        return AT_DEVICE_OTHER_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 AT_QrySbSnPara(VOS_UINT8 ucIndex)
{
    SB_SERIAL_NUM_STRU stSerialNum;

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    PS_MEM_SET((char*)(&stSerialNum), '\0', sizeof(stSerialNum));

    if (NV_OK != NV_ReadEx(MODEM_ID_0, NV_SB_SERIAL_NUM, stSerialNum.serial_num, SB_SERIAL_NUM_LEN))
    {
        AT_WARN_LOG("AT_QrySbSnPara:WARNING:NVIM Read NV_SB_SERIAL_NUM_STRU falied!");
        return AT_ERROR;
    }
    else
    {
        /*设置第16字节为字串结束符*/

        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:%s%s%s",
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                 gaucAtQuotation,
                                                 stSerialNum.serial_num,
                                                 gaucAtQuotation);
    }
    return AT_OK;
}

/* ^EXANTINSERT 命令参数。0表示获取天线插入状态，其他指令暂未定义 */
#define ANTENNA_GET_DETECT_STATUS (0)

/* 获取天线状态的命令参数个数，第一个参数表示获取天线插入状态，第二参数个表示天线编号 */
#define ANTENNA_GET_DETECT_STATUS_PARA_NUM (2)


TAF_UINT32 At_SetExantInsert(TAF_UINT8 ucIndex)
{
    VOS_UINT32 cmd = 0;
    VOS_INT32 exant_index = 0;
    VOS_INT32 exant_value = 0;
    
    /* 参数检查,看是否为set命令 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 第一个参数为命令类型，所以至少有1个参数 */
    if(1 > gucAtParaIndex)
    {
        return AT_ERROR;
    }

    cmd = gastAtParaList[0].ulParaValue;
    switch(cmd)
    {
        /* 获取天线插入状态 */
        case ANTENNA_GET_DETECT_STATUS:
            /* 检查参数个数 */
            if (ANTENNA_GET_DETECT_STATUS_PARA_NUM != gucAtParaIndex)
            {
                return AT_ERROR;
            }
            
            exant_index = gastAtParaList[1].ulParaValue;
            
            /* -1获取失败，0天线未插入，1天线已插入 */
            exant_value = anten_detect_get(exant_index);
            if(ANTEN_DETECT_STATUS_ERR == exant_value)
            {
                return AT_ERROR;
            }
            
            /* 返回格式（只返回一行） ^EXANTINSERT:exant_index(天线编号),exant_value（插入状态：0 未插入，1 已插入） */
            gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                    (VOS_CHAR *)(pgucAtSndCodeAddr),"%s:%d,%d%s",
                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                    (int)exant_index,(int)exant_value, gaucAtCrLf);
        
            return AT_OK;
            
        default:
            /* 不识别的cmd，返回 error */
            return AT_ERROR;
    }
}


VOS_UINT32 AT_QryExantInsert(VOS_UINT8 ucIndex)
{
    VOS_UINT16 usLength = 0;
    VOS_INT32 exant_index = 0;
    VOS_INT32 exant_value = 0;
    VOS_INT32 exant_nr = 0;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 618上面只有两个外置天线：主集、分集。但是615上面有4根，考虑以后可能天线会更多，所以这里的判断条件没有写死 */
    exant_nr = anten_number_get();
    if(0 > exant_nr)
    {
        return AT_ERROR;
    }
    
    /* 返回格式（第一行返回天线数目） ^EXANTINSERT:exant_nr */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                (VOS_CHAR *)(pgucAtSndCodeAddr),"%s:%d%s",
                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                (int)exant_nr, gaucAtCrLf);
        
    for(exant_index = 0; exant_index < exant_nr; exant_index++)
    {
        /* -1获取失败，0天线未插入，1天线已插入 */
        exant_value = anten_detect_get(exant_index);
        if(ANTEN_DETECT_STATUS_ERR == exant_value)
        {
            return AT_ERROR;
        }
        
        /* 返回格式（有多少根天线，返回多少行） ^EXANTINSERT:exant_index(天线编号),exant_value（插入状态：0 未插入，1 已插入） */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                    (VOS_CHAR *)(pgucAtSndCodeAddr + usLength),"%s:%d,%d%s",
                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                    (int)exant_index,(int)exant_value, gaucAtCrLf);
    }

    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}


VOS_INT32 phy_led_status_set(VOS_INT8* led_name, VOS_INT32 onoff)
{
    AT_PHY_LED_ENUM led_index = 0;
    VOS_INT32 ret = -1;
    if (NULL == led_name)
    {
        return -1;
    }

    if ( 0 == VOS_StrNiCmp((TAF_CHAR*)led_name,"phy_led1", strlen(led_name) + 1) )
    {
        led_index = PHY_LED1;
    }
    else if ( 0 == VOS_StrNiCmp((TAF_CHAR*)led_name,"phy_led2", strlen(led_name) + 1) )
    {
        led_index = PHY_LED2;
    }
    else if ( 0 == VOS_StrNiCmp((TAF_CHAR*)led_name,"phy_led3", strlen(led_name) + 1) )
    {
        led_index = PHY_LED3;
    }
    else if ( 0 == VOS_StrNiCmp((TAF_CHAR*)led_name,"phy_led4", strlen(led_name) + 1) )
    {
        led_index = PHY_LED4;
    }
    else
    {
        return -1;
    }
    ret = rtk_led_ctrl((unsigned char)led_index, onoff);
    return ret;
}
#define TLEDSWITCH_PAR_NUM    (2)    /*TLEDSWITCH参数num定义*/
#define TLEDSWITCH_SET_LED_ALL    (255)    /*设置全部LED状态index定义*/

VOS_UINT32 at_led_test_power_on_off_all(VOS_UINT32 state)
{
    VOS_UINT32 index = 0;
    VOS_UINT32 ret = AT_ERROR;
    if ( (0 != state) && (1 != state) )
    {
        vos_printf("at_led_test_power_on_off_all para err!\r\n");
        return AT_ERROR;
    }

    for (index = 0; index < sizeof(g_led_test_config_tab) / sizeof(AT_LED_TEST_TAB_STRU); index++)
    {
        /*函数指针合法性检测*/
        if (NULL == g_led_test_config_tab[index].pFunc)
        {
            vos_printf("at_led_test_power_on_off_all pFunc is NULL err!\r\n");
            return AT_ERROR;
        }

        /*设置LED状态*/
        ret = g_led_test_config_tab[index].pFunc((char*)g_led_test_config_tab[index].led_name, state);
        if (0 != ret)
        {
            vos_printf("at_led_test_power_on_off_all set led fail!\r\n");
            return AT_ERROR;
        }
    }

    return AT_OK;
}


VOS_UINT32 AT_SetTledSwitchPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32 ret = AT_ERROR;
    VOS_UINT32 led_index = 0;
    VOS_UINT32 led_state = 0;

    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        vos_printf("AT_SetTledSwitchPara not set para err!\r\n");
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (TLEDSWITCH_PAR_NUM != gucAtParaIndex)
    {
        vos_printf("AT_SetTledSwitchPara para num err!\r\n");
        return AT_TOO_MANY_PARA;
    }
    led_index = gastAtParaList[0].ulParaValue;
    led_state = gastAtParaList[1].ulParaValue;

    /*参数合法性检测*/
    if ( ( TLEDSWITCH_SET_LED_ALL != led_index) &&
        (led_index >= (sizeof(g_led_test_config_tab) / sizeof(AT_LED_TEST_TAB_STRU)) ) )
    {
        vos_printf("AT_SetTledSwitchPara led_index para invalid err!\r\n");
        return AT_ERROR;
    }

    if ( (led_state != 0) && (led_state != 1) )
    {
        vos_printf("AT_SetTledSwitchPara led_state para invalid err!\r\n");
        return AT_ERROR;
    }

    if (TLEDSWITCH_SET_LED_ALL == led_index)
    {
        ret = at_led_test_power_on_off_all(led_state);
        return ret;
    }

    /*函数指针合法性检测*/
    if (NULL == g_led_test_config_tab[led_index].pFunc)
    {
        vos_printf("AT_SetTledSwitchPara pFunc is NULL err!\r\n");
        return AT_ERROR;
    }

    /*设置LED状态*/
    ret = g_led_test_config_tab[led_index].pFunc((char*)g_led_test_config_tab[led_index].led_name, led_state);
    if (0 != ret)
    {
        return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 At_QryTledSwitchPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}

typedef int (*SlicATSelfTestHandle)(VOS_VOID);
static SlicATSelfTestHandle g_SlicATSelfTestHandle = NULL;
VOS_VOID AT_RegisterSlicATSelfTest(VOS_VOID *handle)
{
    g_SlicATSelfTestHandle = (SlicATSelfTestHandle)handle;/*lint !e611*/     
}
EXPORT_SYMBOL(AT_RegisterSlicATSelfTest);/*lint !e578*//*lint !e132*/


VOS_UINT32 AT_QrySelfTestPara(VOS_UINT8 ucIndex)
{
    VOS_INT32 err = 0;
    VOS_CHAR *aucErrCode[] = {"ERROR","OK"};

    if(NULL != g_SlicATSelfTestHandle)
    {
        err = g_SlicATSelfTestHandle();
    }

    if (((VOS_INT32)(sizeof(aucErrCode)/sizeof(aucErrCode[0]))<= err) || (0 > err))
    {
        err = 0;
    }
    
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                                    "^SELFTEST:%s",
                                                    aucErrCode[err]);

    return AT_OK;
}


VOS_UINT32 At_TestTledSwitchPara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}


VOS_UINT32 AT_SetSimTestPara(const VOS_UINT8 ucIndex)
{
    /* 入参判断 */
    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_WARN_LOG("AT_SetSimTestPara: invalid CLIENT ID or index.");
        return AT_ERROR;
    }

    /* AT 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        vos_printf("AT_SimTestPara not set para err!\r\n");
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (MMI_PAR_NUM != gucAtParaIndex)
    {
        vos_printf("AT_SimTestPara para num err!\r\n");
        return AT_TOO_MANY_PARA;
    }

    /* 设置 sim 标志 */
    mmi.sim_flag = gastAtParaList[0].ulParaValue;

    return AT_OK;
}



VOS_UINT32 At_QrySimTestPara(const VOS_UINT8 ucIndex)
{
    /* 入参判断 */
    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_WARN_LOG("At_QrySimTestPara: invalid CLIENT ID or index.");
        return AT_ERROR;
    }

    /* AT 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数个数过多 */
    if (gucAtParaIndex != 0)
    {
        return AT_ERROR;
    }

    if (SIM_OK == mmi.sim_flag)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%s",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 "sim test ok");
    }
    else if (SIM_FAIL == mmi.sim_flag)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%s",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 "sim test fail");
    }
    else
    {
        /* sim 卡未检测 */
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%s",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 "unknown");
    }

    return AT_OK;
}


VOS_UINT32 AT_SetKeyTestPara(const VOS_UINT8 ucIndex)
{
    /* 入参判断 */
    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_WARN_LOG("AT_SetKeyTestPara: invalid CLIENT ID or index.");
        return AT_ERROR;
    }

    /* AT 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        vos_printf("AT_SetkeytestPara not set para err!\r\n");
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (MMI_PAR_NUM != gucAtParaIndex)
    {
        vos_printf("AT_SetTledSwitchPara para num err!\r\n");
        return AT_TOO_MANY_PARA;
    }

    if ((POWERKEY_OK == gastAtParaList[0].ulParaValue) || (POWERKEY_FAIL == gastAtParaList[0].ulParaValue))
    {
        mmi.powerkey_flag = gastAtParaList[0].ulParaValue;  /* 设置 powerkey 标志 */
    }
    else if ((LEVEL_OK == gastAtParaList[0].ulParaValue) || (LEVEL_FAIL == gastAtParaList[0].ulParaValue))
    {
        mmi.levelkey_flag = gastAtParaList[0].ulParaValue;  /* 设置 levelkey 标志 */
    }
    else if ((WPSKEY_OK == gastAtParaList[0].ulParaValue) || (WPSKEY_FAIL == gastAtParaList[0].ulParaValue))
    {
        mmi.wpskey_flag = gastAtParaList[0].ulParaValue;    /* 设置 wpskey 标志 */
    }
    else if ((RESETKEY_OK == gastAtParaList[0].ulParaValue) || (RESETKEY_FAIL == gastAtParaList[0].ulParaValue))
    {
        mmi.resetkey_flag = gastAtParaList[0].ulParaValue;  /* 设置 resetkey 标志 */
    }
    else if ((MODEKEY_OK == gastAtParaList[0].ulParaValue) || (MODEKEY_FAIL == gastAtParaList[0].ulParaValue))
    {
        mmi.modekey_flag = gastAtParaList[0].ulParaValue;  /* 设置 modekey 标志 */
    }
    else
    {
        /*do nothing*/
    }

    return AT_OK;
}


VOS_UINT32 At_QryKeyTestPara(const VOS_UINT8 ucIndex)
{
    /* 入参判断 */
    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_WARN_LOG("At_QryKeyTestPara: invalid CLIENT ID or index.");
        return AT_ERROR;
    }

    /* AT 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数个数过多 */
    if (gucAtParaIndex != 0)
    {
        return AT_ERROR;
    }

    if (RESETKEY_OK == mmi.resetkey_flag)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%s\r\n",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 "reset key ok");
    }
    else if (RESETKEY_FAIL == mmi.resetkey_flag)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%s\r\n",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 "reset key fail");
    }
    else
    {
        /* resetkey 未检测 */
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%s\r\n",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 "reset key unknown");
    }

    if (WPSKEY_OK == mmi.wpskey_flag)
    {
        gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (TAF_CHAR *)pgucAtSndCodeAddr,
                                  (TAF_CHAR *)pgucAtSndCodeAddr+gstAtSendData.usBufLen,
                                  "%s:%s\r\n",
                                  g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                  "wps key ok");
    }
    else if (WPSKEY_FAIL == mmi.wpskey_flag)
    {
        gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (TAF_CHAR *)pgucAtSndCodeAddr,
                                  (TAF_CHAR *)pgucAtSndCodeAddr+gstAtSendData.usBufLen,
                                  "%s:%s\r\n",
                                  g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                  "wps key fail");
    }
    else
    {
        /* wpskey 未检测 */
        gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (TAF_CHAR *)pgucAtSndCodeAddr,
                                  (TAF_CHAR *)pgucAtSndCodeAddr+gstAtSendData.usBufLen,
                                  "%s:%s\r\n",
                                  g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                  "wps key unknown");
    }
    if (POWERKEY_OK == mmi.powerkey_flag)
    {
        gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (TAF_CHAR *)pgucAtSndCodeAddr,
                                  (TAF_CHAR *)pgucAtSndCodeAddr+gstAtSendData.usBufLen,
                                  "%s:%s\r\n",
                                  g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                  "power key ok");
    }
    else if (POWERKEY_FAIL == mmi.powerkey_flag)
    {
        gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (TAF_CHAR *)pgucAtSndCodeAddr,
                                  (TAF_CHAR *)pgucAtSndCodeAddr+gstAtSendData.usBufLen,
                                  "%s:%s\r\n",
                                  g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                  "power key fail");
    }
    else
    {
        /* powerkey 未检测 */
        gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (TAF_CHAR *)pgucAtSndCodeAddr,
                                  (TAF_CHAR *)pgucAtSndCodeAddr+gstAtSendData.usBufLen,
                                  "%s:%s\r\n",
                                  g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                  "power key unknown");
    }

    return AT_OK;
}




VOS_UINT32 AT_SetUsbTestPara(const VOS_UINT8 ucIndex)
{
    /* 入参检查 */
    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_WARN_LOG("AT_SetUsbTestPara: invalid CLIENT ID or index.");
        return AT_ERROR;
    }

    /* AT 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        vos_printf("AT_UsbTestPara not set para err!\r\n");
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (MMI_PAR_NUM != gucAtParaIndex)
    {
        vos_printf("AT_UsbTestPara num err!\r\n");
        return AT_TOO_MANY_PARA;
    }

    /* 设置usb 标志位 */
    mmi.usb_flag = gastAtParaList[0].ulParaValue;

    return AT_OK;
}



VOS_UINT32 At_QryUsbTestPara(const VOS_UINT8 ucIndex)
{
    /* 入参判断 */
    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_WARN_LOG("At_QryUsbTestPara: invalid CLIENT ID or index.");
        return AT_ERROR;
    }

    /* AT 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数个数过多 */
    if (gucAtParaIndex != 0)
    {
        return AT_ERROR;
    }

    if (USB_OK == mmi.usb_flag)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%s",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 "usb test ok");
    }
    else if (USB_FAIL == mmi.usb_flag)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%s",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 "usb test fail");
    }
    else
    {
        /*usb 未检测*/
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%s",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 "unknown");
    }

    return AT_OK;
}



VOS_UINT32 AT_SetEthTestPara(const VOS_UINT8 ucIndex)
{
    /* 入参判断 */
    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_WARN_LOG("AT_SetEthTestPara: invalid CLIENT ID or index.");
        return AT_ERROR;
    }

    /* AT 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        vos_printf("AT_EthTestPara not set para err!\r\n");
        return AT_DEVICE_OTHER_ERROR;
    }

    /* 参数个数判断 */
    if (MMI_PAR_NUM != gucAtParaIndex)
    {
        vos_printf("AT_EthTestPara num err!\r\n");
        return AT_TOO_MANY_PARA;
    }

    /* 设置网口回环标志位 */
    mmi.eth_flag = gastAtParaList[0].ulParaValue;

    return AT_OK;
}



VOS_UINT32 At_QryEthTestPara(const VOS_UINT8 ucIndex)
{
    /* 入参判断 */
    if (ucIndex >= AT_MAX_CLIENT_NUM)
    {
        AT_WARN_LOG("At_QryEthTestPara: invalid CLIENT ID or index.");
        return AT_ERROR;
    }

    /* AT 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数个数过多 */
    if (gucAtParaIndex != 0)
    {
        return AT_ERROR;
    }

    if (ETH_OK == mmi.eth_flag)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%s",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 "eth test ok");
    }
    else if (ETH_FAIL == mmi.eth_flag)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%s",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 "eth test fail");
    }
    else
    {
        /* 网口回环未检测 */
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%s",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 "unknown");
    }

    return AT_OK;
}


VOS_UINT32 AT_QryWiFi2GPavars(VOS_UINT8 ucIndex)
{

    WLAN_AT_BUFFER_STRU     strBuf;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void *)&strBuf, 0, sizeof(strBuf));

    /* 读取WIFI的2.4g pavars执行的参数 */
    if(AT_RETURN_SUCCESS != WlanATGetWifi2GPavars(&strBuf))
    {
        return AT_ERROR;
    }
    
    /* WIFI模块射频参数 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:\n%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
    return AT_OK;
}




VOS_UINT32 AT_SetWiFi2GPavars(VOS_UINT8 ucIndex)
{
    WLAN_AT_PAVARS2G_STRU    wifiPavars2gStru;

    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

     /* 参数个数判断 */
    if (4 != gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    /* 参数值判断 */
    if(0 != gastAtParaList[0].ulParaValue && 1 != gastAtParaList[0].ulParaValue)
    {
        return AT_ERROR;
    }

   /* 初始化 */
    (void)VOS_MemSet((void *)&wifiPavars2gStru, 0, sizeof(wifiPavars2gStru));

    wifiPavars2gStru.ant_index = gastAtParaList[0].ulParaValue;
    (void)VOS_MemCpy((void *)wifiPavars2gStru.data[0], (void *)gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
    (void)VOS_MemCpy((void *)wifiPavars2gStru.data[1], (void *)gastAtParaList[2].aucPara, gastAtParaList[2].usParaLen);
    (void)VOS_MemCpy((void *)wifiPavars2gStru.data[2], (void *)gastAtParaList[3].aucPara, gastAtParaList[3].usParaLen);

    /* WIFI模块射频参数 */
    if (AT_RETURN_SUCCESS != WlanATSetWifi2GPavars(&wifiPavars2gStru))
    {
        return AT_ERROR;
    }
    return AT_OK;
}

VOS_UINT32 AT_QryWiFi5GPavars(VOS_UINT8 ucIndex)
{

    WLAN_AT_BUFFER_STRU     strBuf;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void *)&strBuf, 0, sizeof(strBuf));

    /* 读取WIFI的2.4g pavars执行的参数 */
    if(AT_RETURN_SUCCESS != WlanATGetWifi5GPavars(&strBuf))
    {
        return AT_ERROR;
    }
    
    /* WIFI模块射频参数 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:\n%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
    return AT_OK;
}




VOS_UINT32 AT_SetWiFi5GPavars(VOS_UINT8 ucIndex)
{
    WLAN_AT_PAVARS5G_STRU    wifiPavars5gStru;
    VOS_INT32  i;
    /* 命令类型判断 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

     /* 参数个数判断 */
    if (13!= gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }
    
    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        return AT_ERROR;
    }
    /* 参数值判断 */
    if(0 != gastAtParaList[0].ulParaValue && 1 != gastAtParaList[0].ulParaValue && 2 !=gastAtParaList[0].ulParaValue)
    {
        return AT_ERROR;
    }

   /* 初始化 */
    (void)VOS_MemSet((void *)&wifiPavars5gStru, 0, sizeof(wifiPavars5gStru));

    wifiPavars5gStru.ant_index = gastAtParaList[0].ulParaValue;
    for  ( i = 0 ; i < 12 ; i++ )
    {
        VOS_MemCpy(wifiPavars5gStru.data[i], gastAtParaList[i+1].aucPara, gastAtParaList[i+1].usParaLen);
    }

    /* WIFI模块射频参数 */
    if (AT_RETURN_SUCCESS != WlanATSetWifi5GPavars(&wifiPavars5gStru))
    {
        return AT_ERROR;
    }
    return AT_OK;
}



VOS_UINT32 AT_SetNVReadWiFi2GPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usNvId  = 0;
    VOS_UINT32                          ulNvLen = 0;
    VOS_UINT8                          *pucData = VOS_NULL_PTR;
    VOS_UINT32                          i       = 0;
    MODEM_ID_ENUM_UINT16                enModemId = MODEM_ID_0;
    VOS_UINT32                          ulRet;

    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        g_ulNVRD = 1;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if(gucAtParaIndex > 1)
    {
        g_ulNVRD = 2;
        return AT_CME_INCORRECT_PARAMETERS;
    }    
    else
    {
        usNvId = NV_ID_WIFI_2G_RFCAL;
        ulNvLen = 6;
    }

    pucData = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, ulNvLen);
    if(VOS_NULL_PTR == pucData)
    {
        g_ulNVRD = 5;
        return AT_ERROR;
    }

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_SetNVReadPara:Get modem id fail");
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR =8;
        return AT_ERROR;
    }

    if(0 == gastAtParaList[0].ulParaValue)
    {
        ulRet = NV_ReadEx(enModemId, usNvId, (VOS_VOID*)pucData, ulNvLen);
    }
    else
    {
        ulRet =NV_ReadPartEx(enModemId, usNvId, 6, (VOS_VOID*)pucData, ulNvLen);
    }

    if(VOS_OK != ulRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVRD = 6;
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;
    if(0 == gastAtParaList[0].ulParaValue)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr,
        "pa2ga0=");
    }
    else if(1 == gastAtParaList[0].ulParaValue)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr,
        "pa2ga1=");
    }
    else
    {
        g_ulNVRD = 3;
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    for(i = 0; i < ulNvLen; i++)
    {
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
            (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen,"0X%02X", pucData[i++]);
        if(i== ulNvLen-1)
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
               (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen,"%02X", pucData[i]);
            break;
        }
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
            (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen,"%02X,", pucData[i]);
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucData);
    g_ulNVRD = 7;
    return AT_OK;
}

VOS_UINT32 AT_SetNVWriteWiFi2GPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usNvId = NV_ID_WIFI_2G_RFCAL;
    VOS_UINT16                          usNvTotleLen = 6;
    VOS_UINT8                           *pucData = VOS_NULL_PTR;
    VOS_UINT32                          ulNvNum = 0; 
    MODEM_ID_ENUM_UINT16                enModemId = MODEM_ID_0;
    VOS_UINT32                          i = 0;
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulTmp = 0;
    VOS_UINT32                          j=0;                    
    VOS_UINT8                          *pu8Start   = VOS_NULL_PTR;

    gstAtSendData.usBufLen = 0;

    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        g_ulNVWR =1;
        return AT_CME_INCORRECT_PARAMETERS;
    }
     /* 参数过多 */
    if(gucAtParaIndex > 4 ||gucAtParaIndex < 1 )
    {
        g_ulNVRD = 2;
        return AT_CME_INCORRECT_PARAMETERS;
    } 

    if(0 != gastAtParaList[0].ulParaValue &&1 != gastAtParaList[0].ulParaValue)
    {
        g_ulNVRD = 2;
        return AT_CME_INCORRECT_PARAMETERS;
    } 
    pucData = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, usNvTotleLen); /*lint !e516*/
    if(VOS_NULL_PTR == pucData)
    {
        g_ulNVWR =3;
        return AT_ERROR;
    }

    for(i = 0; i < 3; i++)
    {
        pu8Start = gastAtParaList[i+1].aucPara;

        for(j = 0; j < 2 ; j++)
        {
            pu8Start +=2;
            ulRet = AT_String2Hex(pu8Start , 2 , &ulTmp);
            if((VOS_OK != ulRet) || (ulTmp > 0xff))
            {
                PS_MEM_FREE(WUEPS_PID_AT, pucData);
                return AT_ERROR;
            }
            *(pucData+ulNvNum) = (VOS_UINT8)ulTmp;
            ulNvNum++;
        }
    }
    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_SetNVWritePara:Get modem id fail");
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR =8;
        return AT_ERROR;
    }
    if(0 == gastAtParaList[0].ulParaValue)
    {
        ulRet = NV_WriteEx(enModemId, usNvId, (VOS_VOID*)pucData, usNvTotleLen);
    }
    else 
    {
        ulRet = NV_WritePartEx(enModemId, usNvId, 6, (VOS_VOID*)pucData, usNvTotleLen);
    }

    if(VOS_OK != ulRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR =9;
        return AT_ERROR;
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucData);
    g_ulNVWR =10;
    return AT_OK;
}


VOS_UINT32 AT_SetNVReadWiFi5GPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usNvId  = 0;
    VOS_UINT32                          ulNvLen = 0;
    VOS_UINT8                          *pucData = VOS_NULL_PTR;
    VOS_UINT32                          i       = 0;
    MODEM_ID_ENUM_UINT16                enModemId = MODEM_ID_0;
    VOS_UINT32                          ulRet;
 
    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        g_ulNVRD = 1;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if(gucAtParaIndex > 1)
    {
        g_ulNVRD = 2;
        return AT_CME_INCORRECT_PARAMETERS;
    }    
    else
    {
        usNvId = NV_ID_WIFI_5G_RFCAL;
        ulNvLen = 24;
    }

    pucData = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, ulNvLen);
    if(VOS_NULL_PTR == pucData)
    {
        g_ulNVRD = 5;
        return AT_ERROR;
    }

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_SetNVReadPara:Get modem id fail");
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR =8;
        return AT_ERROR;
    }

    if(0 == gastAtParaList[0].ulParaValue)
    {
        ulRet = NV_ReadEx(enModemId, usNvId, (VOS_VOID*)pucData, ulNvLen);
    }
    else if (1 == gastAtParaList[0].ulParaValue)
    {
        ulRet =NV_ReadPartEx(enModemId, usNvId, 24, (VOS_VOID*)pucData, ulNvLen);
    }
    else if (2 == gastAtParaList[0].ulParaValue)
    {
        ulRet =NV_ReadPartEx(enModemId, usNvId, 48, (VOS_VOID*)pucData, ulNvLen);
    }
    else
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        return AT_ERROR;
    }

    if(VOS_OK != ulRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVRD = 6;
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = 0;
    if(0 == gastAtParaList[0].ulParaValue)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr,
        "pa5ga0=");
    }
    else if(1 == gastAtParaList[0].ulParaValue)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr,
        "pa5ga1=");
    }
    else
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr,
        "pa5ga2=");

    }

    for(i = 0; i < ulNvLen; i++)
    {
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
            (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen,"0X%02X", pucData[i++]);
        if(i== ulNvLen-1)
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
               (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen,"%02X", pucData[i]);
        break;
        }
   
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
            (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen,"%02X,", pucData[i]);

    }

    PS_MEM_FREE(WUEPS_PID_AT, pucData);
    g_ulNVRD = 7;
    return AT_OK;
}

VOS_UINT32 AT_SetNVWriteWiFi5GPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usNvId = NV_ID_WIFI_5G_RFCAL;
    VOS_UINT16                          usNvTotleLen = 24;
    VOS_UINT8                           *pucData = VOS_NULL_PTR;
    VOS_UINT32                          ulNvNum = 0; 
    MODEM_ID_ENUM_UINT16                enModemId = MODEM_ID_0;
    VOS_UINT32                          i = 0;
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulTmp = 0;
    VOS_UINT32                          j=0;                    
    VOS_UINT8                          *pu8Start   = VOS_NULL_PTR;

    gstAtSendData.usBufLen = 0;

    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        g_ulNVWR =1;
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /* 参数过多 */
    if(gucAtParaIndex > 13 ||gucAtParaIndex < 1 )
    {
        g_ulNVRD = 2;
        return AT_CME_INCORRECT_PARAMETERS;
    } 

    if(0 != gastAtParaList[0].ulParaValue &&1 != gastAtParaList[0].ulParaValue&&2 != gastAtParaList[0].ulParaValue)
    {
        g_ulNVRD = 2;
        return AT_CME_INCORRECT_PARAMETERS;
    } 
 
    pucData = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, usNvTotleLen); /*lint !e516*/
    if(VOS_NULL_PTR == pucData)
    {
        g_ulNVWR =3;
        return AT_ERROR;
    }

    for(i=0; i< 12; i++)   
    {
        pu8Start = gastAtParaList[i+1].aucPara;
        for(j=0; j<2; j++)
        {
            pu8Start +=2;
            ulRet = AT_String2Hex(pu8Start , 2 , &ulTmp);
            if((VOS_OK != ulRet) || (ulTmp > 0xff))
            {
                PS_MEM_FREE(WUEPS_PID_AT, pucData);
                return AT_ERROR;
            }
            *(pucData+ulNvNum) = (VOS_UINT8)ulTmp;
            ulNvNum++;
        }
    }

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_SetNVWritePara:Get modem id fail");
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR =8;
        return AT_ERROR;
    }
    if(0 == gastAtParaList[0].ulParaValue)
    {
        ulRet = NV_WriteEx(enModemId, usNvId, (VOS_VOID*)pucData, usNvTotleLen);
    }
    else if(1 == gastAtParaList[0].ulParaValue)
    {
        ulRet = NV_WritePartEx(enModemId, usNvId, 24, (VOS_VOID*)pucData, usNvTotleLen);
    }
    else
    {
        ulRet = NV_WritePartEx(enModemId, usNvId, 48, (VOS_VOID*)pucData, usNvTotleLen);
    }

    if(VOS_OK != ulRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR =9;
        return AT_ERROR;
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucData);
    g_ulNVWR =10;
    return AT_OK;
}

VOS_UINT32 AT_QryWiFiInit2GPavars(VOS_UINT8 ucIndex)
{
    WLAN_AT_BUFFER_STRU     strBuf;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void *)&strBuf, 0, sizeof(strBuf));

    /* 读取WIFI的2.4g 初始pavars执行的参数 */
    if(AT_RETURN_SUCCESS != WlanATGetInitWifi2GPavars(&strBuf))
    {
        return AT_ERROR;
    }
    
    /* WIFI模块射频参数 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:\n%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
    return AT_OK;
}

VOS_UINT32 AT_QryWiFiInit5GPavars(VOS_UINT8 ucIndex)
{
    WLAN_AT_BUFFER_STRU     strBuf;

    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void *)&strBuf, 0, sizeof(strBuf));

    /* 读取WIFI的5g 初始pavars执行的参数 */
    if(AT_RETURN_SUCCESS != WlanATGetInitWifi5GPavars(&strBuf))
    {
        return AT_ERROR;
    }
    
    /* WIFI模块射频参数 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:\n%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
    return AT_OK;
}

/*****************************************************************************
 函 数 名  : SetWiFiSsidParaCheck
 功能描述  : check Wifi的ssid参数输入
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
 修改内容  : 新生成函数
*****************************************************************************/
VOS_UINT32 SetWiFiSsidParaCheck(VOS_UINT8 ucIndex)
{

    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        AT_WARN_LOG("SetWiFiSsidParaCheck :wifi not suppt\n");
        return AT_ERROR;
    }

    /* SSID最多4组 */
    if (gastAtParaList[0].ulParaValue >= AT_WIFI_MAX_SSID_NUM)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }

    if (VOS_TRUE == g_bAtDataLocked)
    {
        AT_WARN_LOG("SetWiFiSsidParaCheck :g_bAtDataLocked not unlock\n");
        return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 AT_SetWiFiSsidPara(VOS_UINT8 ucIndex)
{
    TAF_AT_MULTI_WIFI_SSID_STRU         stWifiSsid;
    VOS_UINT8                           ucGroup;
    VOS_UINT32 wifissidcheckresult = AT_ERROR;

    /* 参数过多*/
    if (2 != gucAtParaIndex)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    if (0 == gastAtParaList[0].usParaLen
        || 0 == gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /*参数长度过长*/
    if (gastAtParaList[1].usParaLen >= AT_WIFI_SSID_LEN_MAX)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }

    wifissidcheckresult = SetWiFiSsidParaCheck(ucIndex);
    if(AT_OK != wifissidcheckresult)
    {
        return wifissidcheckresult;
    }
    else
    {
        /*do nothing*/
    }

    PS_MEM_SET(&stWifiSsid, 0, sizeof(stWifiSsid));

    ucGroup = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /*读取WIFI KEY对应的NV项*/
    if (VOS_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_MULTI_WIFI_STATUS_SSID,&stWifiSsid, sizeof(TAF_AT_MULTI_WIFI_SSID_STRU)))
    {
        AT_WARN_LOG("AT_SetWiFiSsidPara:READ NV ERROR");
        return AT_ERROR;
    }
    else
    {
        PS_MEM_CPY(&(stWifiSsid.aucWifiSsid[ucGroup][0]), gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
        stWifiSsid.aucWifiSsid[ucGroup][gastAtParaList[1].usParaLen] = '\0';

        /*写入WIFI SSID对应的NV项*/
        if (VOS_OK != NV_WriteEx(MODEM_ID_0, en_NV_Item_MULTI_WIFI_STATUS_SSID,&stWifiSsid, sizeof(TAF_AT_MULTI_WIFI_SSID_STRU)))
        {
            AT_WARN_LOG("AT_SetWiFiSsidPara:WRITE NV ERROR");
            return AT_ERROR;
        }
    }

    return AT_OK;
}

/*****************************************************************************
 函 数 名  : SetWiFiKeyParaCheck
 功能描述  : check Wifi的key参数输入
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
 修改内容  : 新生成函数
*****************************************************************************/
VOS_UINT32 SetWiFiKeyParaCheck(VOS_UINT8 ucIndex)
{

    VOS_UINT8  pindex;
    VOS_UINT8 *paucPara = NULL;

    if (BSP_MODULE_UNSUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        AT_WARN_LOG("SetWiFiSsidParaCheck :wifi not support\n");
        return AT_ERROR;
    }

    if (VOS_TRUE == g_bAtDataLocked)
    {
        AT_WARN_LOG("SetWiFiSsidParaCheck :not unlock\n");
        return AT_ERROR;
    }
    /* AT^WIKEY 协议支持 8-63位ASCII码或64位16进制*/

    /*AT^NFCCFG与AT^SSID都会调用该接口写SSID,且均为传入参数的最后一个参数，
    因此采用[gucAtParaIndex - 1]来获取SSID 参数进行有效性判断*/

    if (AT_WIFI_KEY_LEN_MAX < gastAtParaList[gucAtParaIndex - 1].usParaLen || 
        AT_WIFI_KEY_LEN_MIN > gastAtParaList[gucAtParaIndex - 1].usParaLen)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    else if(AT_WIFI_KEY_LEN_MAX == gastAtParaList[gucAtParaIndex - 1].usParaLen)
    {
        paucPara = gastAtParaList[gucAtParaIndex - 1].aucPara;
        /*wikey长度为64时，只支持16进制*/
        for(pindex = 0; pindex < gastAtParaList[gucAtParaIndex - 1].usParaLen ; pindex++)
        {
            if(((paucPara[pindex] >= '0') && (paucPara[pindex] <= '9'))
               || ((paucPara[pindex] >= 'a') && (paucPara[pindex] <= 'f'))
               || ((paucPara[pindex] >= 'A') && (paucPara[pindex] <= 'F')))
            {
                //do nothing;
            }
            else
            {
                AT_WARN_LOG("SetWiFiSsidParaCheck :Para error\n");
                return AT_ERROR;
            }
        }
    }
    else
    {
        //for lint;
    }
    /* 最多4组SSID */
    if (gastAtParaList[0].ulParaValue >= AT_WIFI_MAX_SSID_NUM)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;

}


VOS_UINT32 AT_SetWiFiKeyPara(VOS_UINT8 ucIndex)
{
    TAF_AT_MULTI_WIFI_SEC_STRU          stWifiKey;
    VOS_UINT8                           ucGroup;
    VOS_UINT32 wifikeycheckresult = AT_ERROR;

    /* 参数过多*/
    if (2 != gucAtParaIndex)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }
    
    if (0 == gastAtParaList[0].usParaLen
        || 0 == gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /* 参数长度过长 */
    if (gastAtParaList[1].usParaLen > AT_WIFI_WLWPAPSK_LEN)
    {
        return  AT_CME_INCORRECT_PARAMETERS;
    }


    wifikeycheckresult = SetWiFiKeyParaCheck(ucIndex);
    if(AT_OK != wifikeycheckresult)
    {
        return wifikeycheckresult;
    }
    else
    {
        /*do nothing*/
    }


    PS_MEM_SET(&stWifiKey, 0, sizeof(stWifiKey));

    ucGroup = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* 读取WIFI KEY对应的NV项 */
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_MULTI_WIFI_KEY,&stWifiKey, sizeof(TAF_AT_MULTI_WIFI_SEC_STRU)))
    {
        AT_WARN_LOG("AT_SetWiFiKeyPara:READ NV ERROR");
        return AT_ERROR;
    }
    else
    {
        /* 写入KEY */
        PS_MEM_CPY(&(stWifiKey.aucWifiWpapsk[ucGroup][0]), gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
        stWifiKey.aucWifiWpapsk[ucGroup][gastAtParaList[1].usParaLen] = '\0';


        if (NV_OK != NV_WriteEx(MODEM_ID_0, en_NV_Item_MULTI_WIFI_KEY, &stWifiKey, sizeof(TAF_AT_MULTI_WIFI_SEC_STRU)))
        {
            AT_WARN_LOG("AT_SetWiFiKeyPara:WRITE NV ERROR");
            return AT_ERROR;
        }
    }


    return AT_OK;
}
/* Modified by f62575 for AT Project, 2011-10-28, end */

/*lint -e553*/
/*lint +e553*/

/******************************************************************************
                       MBB AT Device list
******************************************************************************/
AT_PAR_CMD_ELEMENT_STRU g_astMbbAtDeviceCmdTbl[] = {

	{AT_CMD_FLNA,
    At_SetFlnaPara,      AT_SET_PARA_TIME,   At_QryFlnaPara,        AT_QRY_PARA_TIME,   At_TestFlnaPara ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR,     CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FLNA",     (VOS_UINT8*)"(0-255)"},

	{AT_CMD_VERSION,
    At_SetVersionPara,        AT_NOT_SET_TIME,    At_QryVersion,         AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^VERSION",  VOS_NULL_PTR},

    {AT_CMD_TMMI,
    AT_SetTmmiPara,      AT_NOT_SET_TIME,    AT_QryTmmiPara,        AT_NOT_SET_TIME,   At_TestTmmiPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TMMI",  (VOS_UINT8*)"(0-65535)"},

    {AT_CMD_WIMODE,
    AT_SetWiFiModePara,   AT_NOT_SET_TIME, AT_QryWiFiModePara,   AT_NOT_SET_TIME, At_TestWiFiModePara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIMODE",  (VOS_UINT8*)"(0,1,2,3,4,5)"},
    
    {AT_CMD_WIBAND,
    AT_SetWiFiBandPara,   AT_NOT_SET_TIME, AT_QryWiFiBandPara,   AT_NOT_SET_TIME, At_TestWiFiBandPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIBAND",  (VOS_UINT8*)"(0,1,2,3)"},

    {AT_CMD_WICAL,
    AT_SetWifiCalPara, AT_NOT_SET_TIME, AT_QryWifiCalPara, AT_NOT_SET_TIME, AT_TestWifiCalPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICAL",  (VOS_UINT8*)"(0,1)"},

    {AT_CMD_WICALDATA,
    AT_SetWifiCalDataPara, AT_NOT_SET_TIME, AT_QryWifiCalDataPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICALDATA",  (VOS_UINT8*)"(0-255),(0-32767),(0-32767),(0-32767),(0-32767),(0-32767),(@data)"},

    {AT_CMD_WICALTEMP,
    AT_SetWifiCalTempPara, AT_NOT_SET_TIME, AT_QryWifiCalTempPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICALTEMP",  (VOS_UINT8*)"(0-255),(0-32767)"},

    {AT_CMD_WICALFREQ,
    AT_SetWifiCalFreqPara, AT_NOT_SET_TIME, AT_QryWifiCalFreqPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICALFREQ",  (VOS_UINT8*)"(0-1),(0-32767)"},

    {AT_CMD_WICALPOW,
    AT_SetWifiCalPowPara, AT_NOT_SET_TIME, AT_QryWifiCalPowPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICALPOW",  (VOS_UINT8*)"(0-1),(0-32767)"},


    {AT_CMD_NAVTYPE,
    AT_SetNavTypePara, AT_NOT_SET_TIME, AT_QryNavTypePara, AT_NOT_SET_TIME, At_TestNavTypePara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NAVTYPE",  (VOS_UINT8*)"(0-255)"},

    {AT_CMD_NAVENABLE,
    AT_SetNavEnablePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NAVENABLE",  (VOS_UINT8*)"(0-1)"},

    {AT_CMD_NAVFREQ,
    AT_SetNavFreqPara, AT_NOT_SET_TIME, AT_QryNavFreqPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NAVFREQ",  (VOS_UINT8*)"(0-65535),(0-999),(0-999)"},

    {AT_CMD_NAVRX,
    VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryNavRxPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NAVRX",  VOS_NULL_PTR},

    {AT_CMD_AUDIO,
    AT_SetAudioPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^AUDIO",  (VOS_UINT8*)"(0-255),(0-1),(0-1)"},

    {AT_CMD_SN,
    At_SetSnPara,        AT_NOT_SET_TIME,    At_QrySnPara,           AT_NOT_SET_TIME,    At_CmdTestProcOK, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_DEVICE_OTHER_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"^SN",       VOS_NULL_PTR},

    {AT_CMD_LTEANTINFO,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    AT_QryLteAntInfo,       AT_NOT_SET_TIME,    VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"^LTEANTINFO",       VOS_NULL_PTR},

    {AT_CMD_TNETPORT,
    AT_SetTnetPortPara,    AT_NOT_SET_TIME,    AT_QryTnetPortPara,      AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TNETPORT",   VOS_NULL_PTR},

    {AT_CMD_EXTCHARGE,
    VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryExtChargePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^EXTCHARGE",   VOS_NULL_PTR},
    
    {AT_CMD_WUSITE,
    AT_SetWebSitePara,   AT_NOT_SET_TIME,    AT_QryWebSitePara,     AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WUSITE", (VOS_UINT8*)"(@WUSITE)"},

    {AT_CMD_WIPIN,
    AT_SetWiFiPinPara,   AT_NOT_SET_TIME,    AT_QryWiFiPinPara,     AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIPIN", (VOS_UINT8*)"(@WIPIN)"},

    {AT_CMD_WUUSER,
    AT_SetWebUserPara,   AT_NOT_SET_TIME,    AT_QryWebUserPara,     AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WUUSER", (VOS_UINT8*)"(@WUUSER)"},

    {AT_CMD_PORTLOCK,
    At_SetPortLockPara, AT_NOT_SET_TIME, At_QryPortLockPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^PORTLOCK",   VOS_NULL_PTR},

    {AT_CMD_TBATDATA,
    AT_SetTbatDataPara, AT_NOT_SET_TIME, At_QryTbatDataPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TBATDATA",   (VOS_UINT8 *)"(0,1),(0-255),(0-65535)"},

    {AT_CMD_WIPLATFORM,
    VOS_NULL_PTR,     AT_NOT_SET_TIME, AT_QryWiFiPlatformPara,     AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIPLATFORM",  VOS_NULL_PTR},

    {AT_CMD_ANTENNA,
    AT_SetAntennaPara,     AT_NOT_SET_TIME, AT_QryAntennaPara,     AT_NOT_SET_TIME, At_TestAntenna, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^ANTENNA",  (VOS_UINT8 *)"(0,1,2)"},

    {AT_CMD_TEMPINFO,
    VOS_NULL_PTR,     AT_NOT_SET_TIME, AT_QryTempInfo,     AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TEMPINFO",  VOS_NULL_PTR},

    {AT_CMD_POWERVOLT,
    VOS_NULL_PTR,     AT_NOT_SET_TIME, AT_QryPOWERVolt, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^POWERVOLT",  VOS_NULL_PTR},

    {AT_CMD_FTYRESET,
    AT_SetFtyResetPara, AT_NOT_SET_TIME, AT_QryFtyResetPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FTYRESET",  (VOS_UINT8 *)"(0,1,2)"},

    {AT_CMD_SBSN,
    AT_SetSbSnPara, AT_NOT_SET_TIME, AT_QrySbSnPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SBSN",  VOS_NULL_PTR},

    {AT_CMD_EXANTINSERT,
    At_SetExantInsert,     AT_NOT_SET_TIME, AT_QryExantInsert,     AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^EXANTINSERT",  (VOS_UINT8*)"(0-255),(0-255)"},



    {AT_CMD_SETHWLOCK,
    At_SetHWLock,        AT_NOT_SET_TIME,    VOS_NULL_PTR,         AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SETHWLOCK",  VOS_NULL_PTR},
    
    {AT_CMD_TESTHWLOCK,
    At_TestHWlock,        AT_NOT_SET_TIME,    VOS_NULL_PTR,         AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TESTHWLOCK",  VOS_NULL_PTR},



	{AT_CMD_SFM,
    At_SetSfm,          AT_SET_PARA_TIME,   At_QrySfm,            AT_NOT_SET_TIME,    At_TestSfm , AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SFM",     (VOS_UINT8*)"(0,1,2)"},



    {AT_CMD_TLEDSWITCH,
    AT_SetTledSwitchPara, AT_SET_PARA_TIME,  At_QryTledSwitchPara,  AT_QRY_PARA_TIME,    At_TestTledSwitchPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TLEDSWITCH",    (VOS_UINT8*)"(0-255),(0-255)"},

    {AT_CMD_SIMTEST,
    AT_SetSimTestPara, AT_SET_PARA_TIME, At_QrySimTestPara, AT_QRY_PARA_TIME,VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SIMTEST",    (VOS_UINT8 *)"(0-255)"},

    {AT_CMD_KEYTEST,
    AT_SetKeyTestPara, AT_SET_PARA_TIME, At_QryKeyTestPara, AT_QRY_PARA_TIME,VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^KEYTEST",    (VOS_UINT8 *)"(0-255)"},

    {AT_CMD_USBTEST,
    AT_SetUsbTestPara, AT_SET_PARA_TIME, At_QryUsbTestPara, AT_QRY_PARA_TIME,VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^USBTEST",    (VOS_UINT8 *)"(0-255)"},

    {AT_CMD_ETHTEST,
    AT_SetEthTestPara, AT_SET_PARA_TIME, At_QryEthTestPara, AT_QRY_PARA_TIME,VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^ETHTEST",   (VOS_UINT8 *)"(0-255)"},

    {AT_CMD_SELFTEST,
    VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QrySelfTestPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SELFTEST",  VOS_NULL_PTR},

    {AT_CMD_WIPAVARS2G,
    AT_SetWiFi2GPavars, AT_SET_PARA_TIME,  AT_QryWiFi2GPavars,  AT_QRY_PARA_TIME,    VOS_NULL_PTR,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIPAVARS2G",    (VOS_UINT8*)"(0-1),(@data),(@data),(@data)"},

    {AT_CMD_WIPAVARS5G,
    AT_SetWiFi5GPavars, AT_SET_PARA_TIME,  AT_QryWiFi5GPavars,  AT_QRY_PARA_TIME,    VOS_NULL_PTR,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIPAVARS5G",    (VOS_UINT8*)"(0-2),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data)"},

    {AT_CMD_NVRDWIFI2G,
    AT_SetNVReadWiFi2GPara,     AT_SET_PARA_TIME,  VOS_NULL_PTR,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NVRDWI2G",(VOS_UINT8*)"(0-1)"},

    {AT_CMD_NVWRWIFI2G,
    AT_SetNVWriteWiFi2GPara,    AT_SET_PARA_TIME,  VOS_NULL_PTR,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NVWRWI2G",(VOS_UINT8*)"(0-1),(@data),(@data),(@data)"},
    
    {AT_CMD_NVRDWIFI5G,
    AT_SetNVReadWiFi5GPara,     AT_SET_PARA_TIME,  VOS_NULL_PTR,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NVRDWI5G",(VOS_UINT8*)"(0-2)"},

    {AT_CMD_NVWRWIFI5G,
    AT_SetNVWriteWiFi5GPara,    AT_SET_PARA_TIME,  VOS_NULL_PTR,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NVWRWI5G",(VOS_UINT8*)"(0-2),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data)"},

    {AT_CMD_WIINITIALPAVARS2G,
    VOS_NULL_PTR,  AT_NOT_SET_TIME,  AT_QryWiFiInit2GPavars,  AT_QRY_PARA_TIME,    VOS_NULL_PTR,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIINITIALPAVARS2G",  VOS_NULL_PTR},

    {AT_CMD_WIINITIALPAVARS5G,
    VOS_NULL_PTR,  AT_NOT_SET_TIME,  AT_QryWiFiInit5GPavars,  AT_QRY_PARA_TIME,    VOS_NULL_PTR,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIINITIALPAVARS5G",  VOS_NULL_PTR},


/*lint -e553*/
/*lint +e553*/

};/*g_astMbbAtDeviceCmdTbl*/


VOS_UINT32 At_RegisterDeviceMbbCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_astMbbAtDeviceCmdTbl, sizeof(g_astMbbAtDeviceCmdTbl)/sizeof(g_astMbbAtDeviceCmdTbl[0]));
}



