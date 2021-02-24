

/*****************************************************************************
  1 头文件包含
*****************************************************************************/

#include "AtParse.h"
#include "ATCmdProc.h"
#include "AtDataProc.h"
#include "MbbAtGuComm.h"
#include "AtCheckFunc.h"
#include "LPsNvInterface.h"
#include "AtTestParaCmd.h" 
//#if (FEATURE_ON == MBB_FEATURE_BIP)
#include "mbb_drv_bip.h"
//#endif/*FEATURE_ON == MBB_FEATURE_BIP*/
//#if(FEATURE_ON == MBB_WPG_PCM)
#include "AtTafAgentInterface.h"
#include "AtMsgPrint.h"
//#endif /* FEATURE_ON == MBB_WPG_PCM */


#include "MbbAtCmdFilter.h"


//#if ((FEATURE_ON == FEATURE_HISOCKET) || (VOS_WIN32 == VOS_OS_VER))
#include "semaphore.h" 
#include "product_config.h"
//#endif /*((FEATURE_ON == FEATURE_HISOCKET) || (VOS_WIN32 == VOS_OS_VER))*/


/*lint -e961 -e960*/
#define    THIS_FILE_ID        PS_FILE_ID_AT_EXTENDPRIVATECMD_C

/*****************************************************************************
  1 宏定义
*****************************************************************************/

#define MMA_PLMN_ID_LEN_5   (5)
#define MMA_PLMN_ID_LEN_6   (6)



PRIVATE_SYMBOL VOS_UINT8 g_McsFlag = 0; /* 默认为0；0表示查询上行MCS,1表示查询下行MCS */
#define UPMCS_FLAG          (0)
#define DOWNMCS_FLAG        (1)
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/



#define AT_PCUI_GET_CLIENT_ID(ucUserclient)          (gastAtClientTab[ucUserclient].usClientId)
VOS_UINT8             g_ucUserPasswordExist = VOS_FALSE;
/*DOCOMO鉴权特性，是否需要进行不鉴权*/
VOS_BOOL                              g_DocomoAuthActFlag = VOS_FALSE; 
AT_AUTH_FALLBACK_STRU                    g_AuthFallbackInfo = {0};
AT_NV_AUTHFALLBACK_STRU                  g_Nv_AuthFallbackInfo = {0};  
AT_PDP_STATE_ENUM_U8              g_stAtModemState[TAF_MAX_CID + 1];

/*用于区分HCSQ 命令与CERSSI命令*/
VOS_BOOL   g_isHcsqCmd  = VOS_FALSE;

SIGNAL_INFO_STRU                      g_stSignalInfo = {0};
AT_SYS_MODE_STRU                      g_stAtSysMode = {0};

/*多个特性会使用变量*/
extern AT_SEND_DATA_BUFFER_STRU                gstAtSendData;   /* 单个命令的返回信息存储区 */

AT_DEBUG_INFO_STRU g_stAtDebugInfo = {
    0, 
    VOS_TRUE, 
    AT_CLIENT_ID_APP,
    0, 
    0, 
    0
};

MBB_RAT_SUPPORT_STRU g_MbbIsRatSupport = {0};


typedef struct 
{
    VOS_UINT32 ucEnable; 
    VOS_CHAR*  pucAtName;    
}MBB_CUSTOM_AT;

/*lint -e64*/
MBB_CUSTOM_AT g_MbbCustomAtInfo[] =
{
        {VOS_FALSE,   "^MODEMLOOP" },
        {VOS_FALSE,   "^FASTDORM"},
        {VOS_FALSE,   "^RELEASERRC"},
        {VOS_FALSE,   "^HFDOR"},
        {VOS_FALSE,   "^SIMSWITCH"},
        {VOS_FALSE,   "^SIMSWITCHCFG"},
        {VOS_FALSE,   "+XADPCLKFREQINFO"},
        {VOS_FALSE,   "^MONSC"},
        {VOS_FALSE,   "^MONNC"},
        {VOS_FALSE,    "^NETSCAN"},
        {VOS_FALSE,   "^JDETEX"},
        {VOS_FALSE,   "^CELLLOCK"},
        {VOS_FALSE,   "^FREQLOCK"},
};
/*lint +e64*/
/*与g_MbbCustomAtInfo严格一一对应*/
enum AT_MBB_CUSTOM_ENUM
{
    AT_MBB_CUSTOM_MODEMLOOP   = 0,
    AT_MBB_CUSTOM_FASTDORM,
    AT_MBB_CUSTOM_RELEASERRC,
    AT_MBB_CUSTOM_HFDOR,
    AT_MBB_CUSTOM_SIMSWITCH,
    AT_MBB_CUSTOM_SIMSWITCHCFG,
    AT_MBB_CUSTOM_XADPCLKFREQINFO,
    AT_MBB_CUSTOM_MONSC,
    AT_MBB_CUSTOM_MONNC,
    AT_MBB_CUSTOM_NETSCAN,
    AT_MBB_CUSTOM_JDETEX,
    AT_MBB_CUSTOM_CELLLOCK,
    AT_MBB_CUSTOM_FREQLOCK,
    AT_MBB_CUSTOM_BUTT
};
NV_HUAWEI_MBB_FEATURE_STRU g_stMbbFeature = {0};

/*Socket OM 信号*/
PRIVATE_SYMBOL struct semaphore     g_stSockOmRunSem = {0};/*lint !e120 !e43 !e133*/

/*未解锁情况下SOCK AT 口允许使用的AT命令表*/
PRIVATE_SYMBOL  const VOS_CHAR   *g_pcSockAtCmdTbl[] = 
{
    "^DATALOCK",
    "^AUTHVER",
    "^SN",
    VOS_NULL_PTR
};


/*****************************************************************************
  特性公共 函数申明
*****************************************************************************/
extern TAF_UINT32 At_CheckNumString( TAF_UINT8 *pData,TAF_UINT16 usLen );
extern VOS_UINT32  AT_GetSysModeName(MN_PH_SYS_MODE_EX_ENUM_U8 enSysMode, 
                                                          VOS_CHAR *pucSysModeName);


VOS_VOID At_EventReportRoamStatus(VOS_UINT8 *ucRoam)
{
    VOS_UINT32 ulLength = 0;
    NAS_NVIM_ROAM_CFG_INFO_STRU          stRoamFeature = {0};

    (VOS_VOID)NV_GetLength(en_NV_Item_Roam_Capa, &ulLength);
    (VOS_VOID)NV_ReadEx(MODEM_ID_0, en_NV_Item_Roam_Capa, &stRoamFeature, ulLength);
    /* 读取nv 失败的情况不处理 */
    if (VOS_TRUE == stRoamFeature.ucRoamFeatureFlg)
    {
        if ((TAF_MMA_ROAM_NATIONAL_ON_INTERNATIONAL_ON == *ucRoam) 
            || (TAF_MMA_ROAM_NATIONAL_ON_INTERNATIONAL_OFF == *ucRoam) 
            || (TAF_MMA_ROAM_NATIONAL_OFF_INTERNATIONAL_ON == *ucRoam))
        {
            *ucRoam = AT_ROAM_FEATURE_ON;
        }
        else if (TAF_MMA_ROAM_NATIONAL_OFF_INTERNATIONAL_OFF == *ucRoam)
        {
            *ucRoam = AT_ROAM_FEATURE_OFF;
        }
        else
        {
            /* 漫游支持能力打开后，该情况不处理 */
        }
    }

    return ;
}


VOS_UINT32 AT_OperRoamStatus(VOS_UINT8* ucRoam)
{
    VOS_UINT8             ucTmpRoam = 0;
    VOS_UINT32            ulRet = VOS_FALSE;
    NAS_NVIM_ROAM_CFG_INFO_STRU   stRoamFeature = {0};
    
    *ucRoam = (VOS_UINT8)gastAtParaList[2].ulParaValue;
    /*---------------------------------------------------------
    华为MBB AT规格和海思平台AT规格存在着矛盾，对于模块产品遵从
    的AT规格only符合MBB规格，如是，则模块产品永远不会对漫游区分
    国内漫游和国际漫游，即取值只可能是0或1,即NV8266模块产品永远
    不会使用，故该修改，最好是使用模块宏而不是MBB_COMMON宏.华为
    AT规格:<roam> 漫游支持：0 不支持;1 可以漫游;2 无变化,应模块
    要求，修改如下。须特别注意!!!
    ---------------------------------------------------------*/
    if (ROAM_STATUS_NOCHANGE == *ucRoam) 
    {
        /* 如果当前漫游状态为2时，获取上次漫游状态不为2时的值 */
        ulRet = NV_ReadEx(MODEM_ID_0, en_NV_Item_Roam_Capa, 
            &stRoamFeature, sizeof(NAS_NVIM_ROAM_CFG_INFO_STRU));
        if (NV_OK == ulRet)
        {
            ucTmpRoam = stRoamFeature.ucRoamCapability;
            ROAM_BL_TO_MODULE(ucTmpRoam);
            
            *ucRoam = ucTmpRoam; 
        }
        else
        {
            return AT_ERROR;
        }
    }
    ROAM_MODULE_TO_BL(*ucRoam);
    stRoamFeature.ucRoamCapability = *ucRoam;
    stRoamFeature.ucRoamFeatureFlg = NV_ITEM_ACTIVE;

    ulRet = NV_WriteEx(MODEM_ID_0, en_NV_Item_Roam_Capa, 
                 &stRoamFeature,
                 sizeof(NAS_NVIM_ROAM_CFG_INFO_STRU));
    if (NV_OK != ulRet)
    {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_VOID AT_RssiConvert(VOS_INT16      lRssiValue, VOS_UINT8    *pucRssiLevel )
{
    if(AT_HCSQ_VALUE_INVALID <= lRssiValue)
    {
        *pucRssiLevel = AT_HCSQ_VALUE_INVALID;
    }
    else if (AT_HCSQ_RSSI_VALUE_MAX <= lRssiValue)
    {
        *pucRssiLevel = AT_HCSQ_RSSI_LEVEL_MAX;
    }
    else if (AT_HCSQ_RSSI_VALUE_MIN > lRssiValue)
    {
        *pucRssiLevel = AT_HCSQ_LEVEL_MIN;
    }
    else
    {
        *pucRssiLevel = (VOS_UINT8)((lRssiValue - AT_HCSQ_RSSI_VALUE_MIN) + 1);
    }

    return;
}

VOS_VOID AT_RscpConvert(VOS_INT16      lRscpValue, VOS_UINT8    *pucRscpLevel )
{
    if(AT_HCSQ_VALUE_INVALID <= lRscpValue)
    {
        *pucRscpLevel = AT_HCSQ_VALUE_INVALID;
    }
    else if (AT_HCSQ_RSCP_VALUE_MAX <= lRscpValue)
    {
        *pucRscpLevel = AT_HCSQ_RSCP_LEVEL_MAX;
    }
    else if (AT_HCSQ_RSCP_VALUE_MIN > lRscpValue)
    {
        *pucRscpLevel = AT_HCSQ_LEVEL_MIN;
    }
    else
    {
        *pucRscpLevel = (VOS_UINT8)((lRscpValue - AT_HCSQ_RSCP_VALUE_MIN) + 1);
    }

    return;
}


VOS_VOID AT_EcioConvert(VOS_INT16      lEcioValue, VOS_UINT8    *pucEcioLevel )
{
    if (AT_HCSQ_VALUE_INVALID <= lEcioValue)
    {
        *pucEcioLevel = AT_HCSQ_VALUE_INVALID;
    }
    else if (AT_HCSQ_ECIO_VALUE_MAX <= lEcioValue)
    {
        *pucEcioLevel = AT_HCSQ_ECIO_LEVEL_MAX;
    }
    else if (AT_HCSQ_ECIO_VALUE_MIN > lEcioValue)
    {
        *pucEcioLevel = AT_HCSQ_LEVEL_MIN;
    }
    else
    {
        *pucEcioLevel = (VOS_UINT8)(((lEcioValue - AT_HCSQ_ECIO_VALUE_MIN) * 2) + 1);/*calu ecio level*/
    }

    return;
}

/*****************************************************************************
 函 数 名  : AT_RsrpConvert
 功能描述  : rscp按规则转换
 输入参数  : VOS_INT16      sRsrpValue
 输出参数  :VOS_UINT8    *pucRsrpLevel
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_VOID AT_RsrpConvert(VOS_INT16      sRsrpValue, VOS_UINT8    *pucRsrpLevel )
{
    if(AT_HCSQ_VALUE_INVALID <= sRsrpValue)
    {
        *pucRsrpLevel = AT_HCSQ_VALUE_INVALID;
    }
    else if (AT_HCSQ_LTE_RSRP_VALUE_MAX <= sRsrpValue)
    {
        *pucRsrpLevel = AT_HCSQ_LTE_RSRP_LEVEL_MAX;
    }
    else if (AT_HCSQ_LTE_RSRP_VALUE_MIN > sRsrpValue)
    {
        *pucRsrpLevel = AT_HCSQ_LTE_RSRP_LEVEL_MIN;
    }
    else
    {
        *pucRsrpLevel = (VOS_UINT8)((sRsrpValue - AT_HCSQ_LTE_RSRP_VALUE_MIN) + 1);
    }

    return;
}
/*****************************************************************************
 函 数 名  : AT_RsrqConvert
 功能描述  : rscp按规则转换
 输入参数  : VOS_INT16      sRsrpValue
 输出参数  :VOS_UINT8    *pucRsrpLevel
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_VOID AT_RsrqConvert(VOS_INT16  sRsrqValue, VOS_UINT8    *pucRsrqLevel )
{
    if(AT_HCSQ_VALUE_INVALID <= sRsrqValue)
    {
        *pucRsrqLevel = AT_HCSQ_VALUE_INVALID;
    }
    else if (AT_HCSQ_LTE_RSRQ_VALUE_MAX <= sRsrqValue)
    {
        *pucRsrqLevel = AT_HCSQ_LTE_RSRQ_LEVEL_MAX;
    }
    else if (AT_HCSQ_LTE_RSRQ_VALUE_MIN >= sRsrqValue)
    {
        *pucRsrqLevel = AT_HCSQ_LTE_RSRQ_LEVEL_MIN;
    }
    else
    {
        /* rsrq 海思底层的取值分为为-40到-6db；上层AT命令中处理格式是最小值-19.5对应AT命令1(小于-19.5取值为0)，
        -3对应AT命令34*/
        *pucRsrqLevel = (VOS_UINT8)(sRsrqValue * 2 - AT_HCSQ_LTE_RSRQ_VALUE_MIN * 2);
    }

    return;
}
/*****************************************************************************
 函 数 名  : AT_SinrConvert
 功能描述  : sinr按规则转换
 输入参数  : VOS_INT16      sRsrpValue
 输出参数  :VOS_UINT8    *pucRsrpLevel
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_VOID AT_SinrConvert(VOS_INT16  lSinrValue, VOS_UINT8    *plSinrLevel )
{
    if(AT_HCSQ_VALUE_INVALID <= lSinrValue)
    {
        *plSinrLevel = AT_HCSQ_VALUE_INVALID;
    }
    else if (AT_HCSQ_LTE_SINR_VALUE_MAX <= lSinrValue)
    {
        *plSinrLevel = AT_HCSQ_LTE_SINR_LEVEL_MAX;
    }
    else if (AT_HCSQ_LTE_SINR_VALUE_MIN > lSinrValue)
    {
        *plSinrLevel = AT_HCSQ_LTE_SINR_LEVEL_MIN;
    }
    else
    {
        *plSinrLevel = (VOS_UINT8)((lSinrValue - AT_HCSQ_LTE_SINR_VALUE_MIN) * 5 + 1);
    }

    return;
}


VOS_UINT32 At_MbbMatchAtCmdName(VOS_CHAR *pszCmdName) 
{
    VOS_UINT32              ulEclNmLen = 0;
    VOS_UINT32              ulArrySize = 0;
    
    ulArrySize = sizeof(g_MbbCustomAtInfo) / sizeof(g_MbbCustomAtInfo[0]);
    
    for (ulEclNmLen  = 0; ulEclNmLen  < ulArrySize; ulEclNmLen++)
    {
        if ((AT_SUCCESS == AT_STRCMP((VOS_CHAR *)pszCmdName,
                    (VOS_CHAR *)g_MbbCustomAtInfo[ulEclNmLen].pucAtName))
                && (VOS_FALSE == g_MbbCustomAtInfo[ulEclNmLen].ucEnable))
        {
            return AT_FAILURE;
        }
    }
    return AT_SUCCESS;
}


VOS_VOID At_MbbMatchAtInit(VOS_VOID)
{
    if (g_stMbbFeature.ucPcmModemLoopEnable)
    {
        g_MbbCustomAtInfo[AT_MBB_CUSTOM_MODEMLOOP].ucEnable = VOS_TRUE;
    }

    if (g_stMbbFeature.ucMbbFastDormEnable)
    {
        g_MbbCustomAtInfo[AT_MBB_CUSTOM_HFDOR].ucEnable = VOS_TRUE;
    }
    else
    {
        g_MbbCustomAtInfo[AT_MBB_CUSTOM_FASTDORM].ucEnable = VOS_TRUE;
        g_MbbCustomAtInfo[AT_MBB_CUSTOM_RELEASERRC].ucEnable = VOS_TRUE;
    }

    if (g_stMbbFeature.ucSimSwitchEnable)
    {
        g_MbbCustomAtInfo[AT_MBB_CUSTOM_SIMSWITCH].ucEnable = VOS_TRUE;
        g_MbbCustomAtInfo[AT_MBB_CUSTOM_SIMSWITCHCFG].ucEnable = VOS_TRUE;
    }

    if (g_stMbbFeature.ucAdaptiveClockingEnable)
    {
        g_MbbCustomAtInfo[AT_MBB_CUSTOM_XADPCLKFREQINFO].ucEnable = VOS_TRUE;
    }

    if (g_stMbbFeature.ucNetMonitorEnable)
    {
        g_MbbCustomAtInfo[AT_MBB_CUSTOM_MONSC].ucEnable = VOS_TRUE;
        g_MbbCustomAtInfo[AT_MBB_CUSTOM_MONNC].ucEnable = VOS_TRUE;
    }

    if (g_stMbbFeature.ucNetScanEnable)
    {
        g_MbbCustomAtInfo[AT_MBB_CUSTOM_NETSCAN].ucEnable = VOS_TRUE;
    }

    if (g_stMbbFeature.ucJammingDetectionEnable)
    {
        g_MbbCustomAtInfo[AT_MBB_CUSTOM_JDETEX].ucEnable = VOS_TRUE;
    }

    if (g_stMbbFeature.ucCellLockEnable)
    {
        g_MbbCustomAtInfo[AT_MBB_CUSTOM_CELLLOCK].ucEnable = VOS_TRUE;
    }

    if (g_stMbbFeature.ucFreqLockEnable)
    {
        g_MbbCustomAtInfo[AT_MBB_CUSTOM_FREQLOCK].ucEnable = VOS_TRUE;
    }
}


/*****************************************************************************
函 数 名  : GetLteCatInfo
功能描述  : ^LTECAT GetLteCatInfo
输入参数  : TAF_UINT8 ucIndex
输出参数  : 无
返 回 值  : VOS_UINT32
调用函数  :
被调函数  :
修改历史:
*****************************************************************************/
PRIVATE_SYMBOL VOS_UINT32 GetLteCatInfo(Lte_Cat_Info_STRU* pstLteCatInfo)
{

    VOS_VOID*   pLteCatNvInfo = VOS_NULL; 
    VOS_UINT32 ulResult = 0;
    VOS_UINT32 i = 0;
    VOS_UINT32 ulLteCatTabLen = 0;

    if (VOS_NULL == pstLteCatInfo)
    {
        AT_ERR_LOG("At_QryLteCat: pstLteCatInfo is NULL.\n");
        return VOS_FALSE; 
    }

    Lte_Cat_Nv_Info_STRU LteCatNVInfoTab [] =
    {
        {EN_NV_ID_UE_CAPABILITY, sizeof(LRRC_NV_UE_EUTRA_CAP_STRU)},
        {EN_NV_ID_UE_CAP_V1020,  sizeof(RRC_UE_EUTRA_CAP_V1020_IES_STRU)},
        {EN_NV_ID_UE_CAP_V1060,  sizeof(RRC_UE_EUTRA_CAP_V1060_IES_STRU)},
        {EN_NV_ID_UE_CAP_V1090,  sizeof(RRC_UE_EUTRA_CAP_V1090_IES_STRU)},
        {EN_NV_ID_UE_CAP_V1130,  sizeof(RRC_UE_EUTRA_CAP_V1130_IES_STRU)},
        {EN_NV_ID_UE_CAP_V1170,  sizeof(RRC_UE_EUTRA_CAP_V1170_IES_STRU)},
        {EN_NV_ID_UE_CAP_V1180,  sizeof(RRC_UE_EUTRA_CAP_V1180_IES_STRU)},
        {EN_NV_ID_UE_CAP_V11A0,  sizeof(RRC_UE_EUTRA_CAP_V11A0_IES_STRU)},   
    };

    ulLteCatTabLen = sizeof(LteCatNVInfoTab) / sizeof(Lte_Cat_Nv_Info_STRU);

    for(i = 0; i < ulLteCatTabLen; i++)
    {
        pLteCatNvInfo = (VOS_VOID*)MBB_MEM_ALLOC(WUEPS_PID_AT, (VOS_INT32)LteCatNVInfoTab[i].ulNvLen);

        if(VOS_NULL == pLteCatNvInfo)
        {
            AT_ERR_LOG("At_QryLteCat: malloc LteCatNVInfo error.\n");
            return VOS_FALSE;
        }

        ulResult = NVM_Read(LteCatNVInfoTab[i].ulNvId, pLteCatNvInfo, LteCatNVInfoTab[i].ulNvLen);
        
        if (NV_OK != ulResult)
        {
            AT_ERR_LOG("At_QryLteCat: Nv Read fail!!\n");
            MBB_MEM_FREE(WUEPS_PID_AT, pLteCatNvInfo);
            pLteCatNvInfo = VOS_NULL; 
            return VOS_FALSE; /*lint !e438*/
        }

        switch(LteCatNVInfoTab[i].ulNvId)
        {
            case EN_NV_ID_UE_CAPABILITY:
                pstLteCatInfo->UeCapabilitybitUeEutraCapV940Present = 
                    ((LRRC_NV_UE_EUTRA_CAP_STRU*)pLteCatNvInfo)->stUeCapV920Ies.bitUeEutraCapV940Present;
                pstLteCatInfo->UeCapabilitybitUeEutraCapV1020Present = 
                    ((LRRC_NV_UE_EUTRA_CAP_STRU*)pLteCatNvInfo)->stUeCapV920Ies.stUeEutraCapV940.bitUeEutraCapV1020Present;
                pstLteCatInfo->UeCapabilityUeCatg = ((LRRC_NV_UE_EUTRA_CAP_STRU*)pLteCatNvInfo)->enUeCatg + 1;
                break;

            case EN_NV_ID_UE_CAP_V1020:
                pstLteCatInfo->UeCapV1020bitNonCritiExtPresent   = 
                    ((RRC_UE_EUTRA_CAP_V1020_IES_STRU*)pLteCatNvInfo)->bitNonCritiExtPresent;
                pstLteCatInfo->UeCapV1020bitUeCatgV1020Present = 
                    ((RRC_UE_EUTRA_CAP_V1020_IES_STRU*)pLteCatNvInfo)->bitUeCatgV1020Present;
                pstLteCatInfo->UeCapV1020UeCatgV1020              = 
                    ((RRC_UE_EUTRA_CAP_V1020_IES_STRU*)pLteCatNvInfo)->ucUeCatgV1020;
                break;

            case EN_NV_ID_UE_CAP_V1060:
                pstLteCatInfo->UeCapV1060bitNonCritiExtPresent   = 
                    ((RRC_UE_EUTRA_CAP_V1060_IES_STRU*)pLteCatNvInfo)->bitNonCritiExtPresent; 
                break;

            case EN_NV_ID_UE_CAP_V1090:
                pstLteCatInfo->UeCapV1090bitNonCritiExtPresent   = 
                    ((RRC_UE_EUTRA_CAP_V1090_IES_STRU*)pLteCatNvInfo)->bitNonCritiExtPresent;
                break;

            case EN_NV_ID_UE_CAP_V1130:
                pstLteCatInfo->UeCapV1130bitNonCritiExtPresent   = 
                    ((RRC_UE_EUTRA_CAP_V1130_IES_STRU*)pLteCatNvInfo)->bitNonCritiExtPresent;
                break;

            case EN_NV_ID_UE_CAP_V1170:
                pstLteCatInfo->UeCapV1170bitNonCritiExtPresent   = 
                    ((RRC_UE_EUTRA_CAP_V1170_IES_STRU*)pLteCatNvInfo)->bitNonCritiExtPresent;
                pstLteCatInfo->UeCapV1170bitucCategoryPresent   = 
                    ((RRC_UE_EUTRA_CAP_V1170_IES_STRU*)pLteCatNvInfo)->bitucUeCategoryV1170Present;
                pstLteCatInfo->UeCapV1170UeCategoryV1170        = 
                    ((RRC_UE_EUTRA_CAP_V1170_IES_STRU*)pLteCatNvInfo)->ucUeCategoryV1170;
                break;

            case EN_NV_ID_UE_CAP_V1180:
                pstLteCatInfo->UeCapV1180bitNonCritiExtPresent   = 
                    ((RRC_UE_EUTRA_CAP_V1180_IES_STRU*)pLteCatNvInfo)->bitNonCritiExtPresent;
                break;

            case EN_NV_ID_UE_CAP_V11A0:
                pstLteCatInfo->UeCapV11A0bitUeCatgV11a0Present = 
                    ((RRC_UE_EUTRA_CAP_V11A0_IES_STRU*)pLteCatNvInfo)->bitUeCatgV11a0Present;
                pstLteCatInfo->UeCapV11A0UeCatgV11a0               = 
                    ((RRC_UE_EUTRA_CAP_V11A0_IES_STRU*)pLteCatNvInfo)->ucUeCatgV11a0;
                break;

            default:
                break;
        }
        
        MBB_MEM_FREE(WUEPS_PID_AT, pLteCatNvInfo);
        pLteCatNvInfo = VOS_NULL; /*lint !e438*/
    }        

    return VOS_TRUE;
}


/*****************************************************************************
函 数 名  : At_QryLteCat
功能描述  : ^LTECAT查询命令处理函数;SBB定制开发
输入参数  : TAF_UINT8 ucIndex
输出参数  : 无
返 回 值  : VOS_UINT32
调用函数  :
被调函数  :
修改历史:
*****************************************************************************/
VOS_UINT32 At_QryLteCatEx(VOS_UINT8 ucIndex)
{
    VOS_UINT8                 ucCapability = 0;
    VOS_UINT32               ulLteCatGetResult = VOS_TRUE;
    Lte_Cat_Info_STRU      stLteCatInfo = {0};
    
    ulLteCatGetResult = GetLteCatInfo(&stLteCatInfo);

    if (VOS_FALSE == ulLteCatGetResult)
    {
        /*0xd22读取失败直接返回错误*/
        return AT_ERROR;
    }
    
    do
    {
        if((VOS_FALSE == stLteCatInfo.UeCapabilitybitUeEutraCapV940Present)
            || (VOS_FALSE == stLteCatInfo.UeCapabilitybitUeEutraCapV1020Present)
            || (VOS_FALSE == stLteCatInfo.UeCapV1020bitUeCatgV1020Present))
        { 
            /*显示0xd22c中的cat等级*/
            ucCapability = stLteCatInfo.UeCapabilityUeCatg;
            break;
        }

        if ((VOS_FALSE == stLteCatInfo.UeCapV1020bitNonCritiExtPresent)
            || (VOS_FALSE == stLteCatInfo.UeCapV1060bitNonCritiExtPresent)
            || (VOS_FALSE == stLteCatInfo.UeCapV1090bitNonCritiExtPresent)
            || (VOS_FALSE == stLteCatInfo.UeCapV1130bitNonCritiExtPresent)
            || (VOS_FALSE == stLteCatInfo.UeCapV1170bitucCategoryPresent))
        {
            /*显示0xD223中的cat等级*/
            ucCapability = stLteCatInfo.UeCapV1020UeCatgV1020;
            break;
        }
        
        if((VOS_FALSE == stLteCatInfo.UeCapV1170bitNonCritiExtPresent)
            || (VOS_FALSE == stLteCatInfo.UeCapV1180bitNonCritiExtPresent)
            || (VOS_FALSE == stLteCatInfo.UeCapV11A0bitUeCatgV11a0Present))
        {
            /*显示0xD342中的cat等级*/
            ucCapability = stLteCatInfo.UeCapV1170UeCategoryV1170;
            break;
        }
        else
        {
            /*显示0xD349中的cat等级*/
            ucCapability = stLteCatInfo.UeCapV11A0UeCatgV11a0;
            break;
        }
    }while(0);

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                      (TAF_CHAR *)pgucAtSndCodeAddr,
                      (TAF_CHAR*)pgucAtSndCodeAddr,
                      "%s: %d",
                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                      ucCapability);

    return AT_OK;
}


/*************************************************************/
/*特性控制区域*/
/*************************************************************/
VOS_CHAR g_szBufForDebug[BUFFER_LENGTH_256 + 1] = {0};



NAS_MM_INFO_IND_STRU    gstCCLKInfo;

VOS_VOID AT_InitCclkInfo(VOS_VOID)
{
    MBB_MEM_SET(&gstCCLKInfo, 0, sizeof(NAS_MM_INFO_IND_STRU));/*lint !e516 */
    /*lint -e63 */
    gstCCLKInfo.ucIeFlg = NAS_MM_INFO_IE_UTLTZ | NAS_MM_INFO_IE_LTZ;
    gstCCLKInfo.cLocalTimeZone = AT_INVALID_TZ_VALUE;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucYear = 0;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucMonth = 1;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucDay = 6;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucHour = 8;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.cTimeZone = AT_INVALID_TZ_VALUE;
    /*lint +e63 */
}


VOS_UINT32 At_CheckUnlockCmdName(VOS_CHAR *pszCmdName, VOS_UINT8 ucIndex)
{
    VOS_UINT32  i = 0;
    VOS_UINT32  ulAtLockCmdNum = 0;
    VOS_CHAR **ptrMbbAtCmdTable = VOS_NULL_PTR;

    if((VOS_NULL_PTR == pszCmdName) || (ucIndex >= AT_CLIENT_ID_BUTT))
    {
        return AT_FAILURE;
    }

    /*暂不对APP用户进行限制*/
    if (AT_APP_USER == gastAtClientTab[ucIndex].UserType)
    {
        return AT_SUCCESS;
    }
    
    ptrMbbAtCmdTable = (VOS_CHAR **)MbbAtCmdFilterGetTable();

    if (VOS_NULL_PTR == ptrMbbAtCmdTable)
    {
        return AT_FAILURE;
    }

    ulAtLockCmdNum = MbbAtCmdFilterGetTableCount();
    for (i = 0; i < ulAtLockCmdNum; i++)
    {
        if (VOS_NULL_PTR != ptrMbbAtCmdTable[i]) /*lint !e10*/
        {
            if (ERR_MSP_SUCCESS == AT_STRCMP(pszCmdName, ptrMbbAtCmdTable[i]))/*lint !e10*/
            {
                return AT_SUCCESS;
            }
        }
    }

    return AT_FAILURE;
}


VOS_UINT32 At_CheckAllowCmdName(VOS_CHAR* pszCmdName, VOS_UINT8 ucIndex)
{
    VOS_UINT32  i = 0;
    VOS_UINT32  ulAllowCmdNum = 0;
    VOS_CHAR **ptrMbbAtCmdTable = VOS_NULL_PTR;

    if((VOS_NULL_PTR == pszCmdName) || (ucIndex >= AT_CLIENT_ID_BUTT))
    {
        return AT_FAILURE;
    }

    /*暂不对APP用户进行限制*/
    if (AT_APP_USER == gastAtClientTab[ucIndex].UserType)
    {
        return AT_SUCCESS;
    }
    
    ptrMbbAtCmdTable = (VOS_CHAR**)MbbAtCmdAllowGetTable();
    if (VOS_NULL_PTR == ptrMbbAtCmdTable)
    {
        return AT_FAILURE;
    }

    ulAllowCmdNum = MbbAtCmdAllowGetTableCount();
    for (i = 0; i < ulAllowCmdNum; i++)
    {
        if (VOS_NULL_PTR != ptrMbbAtCmdTable[i]) /*lint !e10*/
        {
            if (ERR_MSP_SUCCESS == AT_STRCMP(pszCmdName, ptrMbbAtCmdTable[i]))/*lint !e10*/
            {
                return AT_SUCCESS;
            }
        }
    }

    return AT_FAILURE;
}

/*****************************************************************************
  4 函数定义
*****************************************************************************/
/*AT+CSIM定制*/
VOS_UINT8                               gucCsimActiveFlag;


VOS_UINT16 AT_IsCSIMCustommed(VOS_VOID)
{
    return (gucCsimActiveFlag == VOS_TRUE);
}


VOS_VOID    AT_ReadCsimCustomizationNV(VOS_VOID)
{
    VOS_UINT32                          ulResult;
    TAF_NV_CSIM_CUSTOMIZED              stCsimCustomized;

    stCsimCustomized.ucNvActiveFlag = 0;

    ulResult = NV_Read(NV_ID_CSIM_CUSTOMIZATION,&stCsimCustomized,sizeof(TAF_NV_CSIM_CUSTOMIZED));

    if((NV_OK == ulResult)
        && (VOS_TRUE == stCsimCustomized.ucNvActiveFlag))
    {
       gucCsimActiveFlag  = VOS_FALSE;
    }
    else
    {
       gucCsimActiveFlag = VOS_TRUE;
    }

    return;
}


/*德国 Vodafone CPBS定制*/
VOS_UINT16                              gucVodafoneCpbs;

VOS_UINT16 AT_IsVodafoneCustommed(VOS_VOID)
{
    return (gucVodafoneCpbs == VOS_TRUE);
}

VOS_VOID AT_SetVodafoneCustomFlag(VOS_UINT16 flag)
{
    gucVodafoneCpbs = flag;
}

VOS_VOID At_Pb_VodafoneCPBSCus(TAF_UINT16* usLength, TAF_UINT8 ucIndex)
{
    /*德国 Vodafone CPBS定制*/
    if(AT_IsVodafoneCustommed())
    {
        /*lint -e64 -e119*/
        *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,
                                          (TAF_CHAR *)pgucAtSndCodeAddr + *usLength,
                                          "%s: (\"SM\",\"EN\",\"ON\")",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /*lint +e64 +e119*/
    }
    else
    {
        /*lint -e64 -e119*/
        *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,
                                          (TAF_CHAR *)pgucAtSndCodeAddr + *usLength,
                                          "%s: (\"SM\",\"EN\",\"ON\",\"FD\")",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /*lint +e64 +e119*/
    }
}/*lint !e550*/

/*德国 Vodafone CPBS定制*/

VOS_VOID AT_ReadVodafoneCpbsNV( VOS_VOID )
{
    VOS_UINT32                          ulResult;
    NAS_NV_Vodafone_CPBS                stgucVodafoneCpbs;

    stgucVodafoneCpbs.Vodafone_CPBS = 0;

    ulResult = NV_Read(NV_ID_VODAFONE_CPBS,&stgucVodafoneCpbs,sizeof(NAS_NV_Vodafone_CPBS));

    if((NV_OK == ulResult)
        && (VOS_TRUE == stgucVodafoneCpbs.Vodafone_CPBS))
    {
       AT_SetVodafoneCustomFlag(VOS_TRUE);
    }
    else
    {
       AT_SetVodafoneCustomFlag(VOS_FALSE);
    }

    return;
}


#define DEST_STRING_MAX_LEN      (150)
#define SYSCFGEX_MODE_AUTO           (0x00)
#define SYSCFGEX_MODE_RAT_MAX        (0x06)
#define SYSCFGEX_MODE_NO_CHANGE      (0x99)
#define SYSCFGEX_MODE_INVALID        (0xFF)
#define TAF_PH_BAND_LEN              128
TAF_PH_BAND_NAME_STRU    gastSyscfgexLteLowBandStr[] =
{
    {MN_MMA_LTE_EUTRAN_BAND1,        "LTE BC1" },
    {MN_MMA_LTE_EUTRAN_BAND2,        "LTE BC2" },
    {MN_MMA_LTE_EUTRAN_BAND3,        "LTE BC3" },
    {MN_MMA_LTE_EUTRAN_BAND4,        "LTE BC4" },
    {MN_MMA_LTE_EUTRAN_BAND5,        "LTE BC5" },
    {MN_MMA_LTE_EUTRAN_BAND6,        "LTE BC6" },
    {MN_MMA_LTE_EUTRAN_BAND7,        "LTE BC7" },
    {MN_MMA_LTE_EUTRAN_BAND8,        "LTE BC8" },
    {MN_MMA_LTE_EUTRAN_BAND9,        "LTE BC9" },
    {MN_MMA_LTE_EUTRAN_BAND10,       "LTE BC10"},
    {MN_MMA_LTE_EUTRAN_BAND11,       "LTE BC11"},
    {MN_MMA_LTE_EUTRAN_BAND12,       "LTE BC12"},
    {MN_MMA_LTE_EUTRAN_BAND13,       "LTE BC13"},
    {MN_MMA_LTE_EUTRAN_BAND14,       "LTE BC14"},
    {MN_MMA_LTE_EUTRAN_BAND15,       "LTE BC15"},
    {MN_MMA_LTE_EUTRAN_BAND16,       "LTE BC16"},
    {MN_MMA_LTE_EUTRAN_BAND17,       "LTE BC17"},
    {MN_MMA_LTE_EUTRAN_BAND18,       "LTE BC18"},
    {MN_MMA_LTE_EUTRAN_BAND19,       "LTE BC19"},
    {MN_MMA_LTE_EUTRAN_BAND20,       "LTE BC20"},
    {MN_MMA_LTE_EUTRAN_BAND21,       "LTE BC21"},
    {MN_MMA_LTE_EUTRAN_BAND22,       "LTE BC22"},
    {MN_MMA_LTE_EUTRAN_BAND23,       "LTE BC23"},
    {MN_MMA_LTE_EUTRAN_BAND24,       "LTE BC24"},
    {MN_MMA_LTE_EUTRAN_BAND25,       "LTE BC25"},
    {MN_MMA_LTE_EUTRAN_BAND26,       "LTE BC26"},
    {MN_MMA_LTE_EUTRAN_BAND27,       "LTE BC27"},
    {MN_MMA_LTE_EUTRAN_BAND28,       "LTE BC28"},
    {MN_MMA_LTE_EUTRAN_BAND29,       "LTE BC29"},
    {MN_MMA_LTE_EUTRAN_BAND30,       "LTE BC30"},
    {MN_MMA_LTE_EUTRAN_BAND31,       "LTE BC31"},
    {MN_MMA_LTE_EUTRAN_BAND32,       "LTE BC32"}
};

TAF_PH_BAND_NAME_STRU    gastSyscfgexLteHighBandStr[] =
{
    {MN_MMA_LTE_EUTRAN_BAND33,       "LTE BC33"},
    {MN_MMA_LTE_EUTRAN_BAND34,       "LTE BC34"},
    {MN_MMA_LTE_EUTRAN_BAND35,       "LTE BC35"},
    {MN_MMA_LTE_EUTRAN_BAND36,       "LTE BC36"},
    {MN_MMA_LTE_EUTRAN_BAND37,       "LTE BC37"},
    {MN_MMA_LTE_EUTRAN_BAND38,       "LTE BC38"},
    {MN_MMA_LTE_EUTRAN_BAND39,       "LTE BC39"},
    {MN_MMA_LTE_EUTRAN_BAND40,       "LTE BC40"},
    {MN_MMA_LTE_EUTRAN_BAND41,       "LTE BC41"},
    {MN_MMA_LTE_EUTRAN_BAND42,       "LTE BC42"},
    {MN_MMA_LTE_EUTRAN_BAND43,       "LTE BC43"},
    {MN_MMA_LTE_EUTRAN_BAND44,       "LTE BC44"},
    {MN_MMA_LTE_EUTRAN_BAND45,       "LTE BC45"},
    {MN_MMA_LTE_EUTRAN_BAND46,       "LTE BC46"},
    {MN_MMA_LTE_EUTRAN_BAND47,       "LTE BC47"},
    {MN_MMA_LTE_EUTRAN_BAND48,       "LTE BC48"},
    {MN_MMA_LTE_EUTRAN_BAND49,       "LTE BC49"},
    {MN_MMA_LTE_EUTRAN_BAND50,       "LTE BC50"},
    {MN_MMA_LTE_EUTRAN_BAND51,       "LTE BC51"},
    {MN_MMA_LTE_EUTRAN_BAND52,       "LTE BC52"},
    {MN_MMA_LTE_EUTRAN_BAND53,       "LTE BC53"},
    {MN_MMA_LTE_EUTRAN_BAND54,       "LTE BC54"},
    {MN_MMA_LTE_EUTRAN_BAND55,       "LTE BC55"},
    {MN_MMA_LTE_EUTRAN_BAND56,       "LTE BC56"},
    {MN_MMA_LTE_EUTRAN_BAND57,       "LTE BC57"},
    {MN_MMA_LTE_EUTRAN_BAND58,       "LTE BC58"},
    {MN_MMA_LTE_EUTRAN_BAND59,       "LTE BC59"},
    {MN_MMA_LTE_EUTRAN_BAND60,       "LTE BC60"},
    {MN_MMA_LTE_EUTRAN_BAND61,       "LTE BC61"},
    {MN_MMA_LTE_EUTRAN_BAND62,       "LTE BC62"},
    {MN_MMA_LTE_EUTRAN_BAND63,       "LTE BC63"},
    {MN_MMA_LTE_EUTRAN_BAND64,       "LTE BC64"}
};
NAS_NV_SYSCFGEX_MODE_LIST            g_stSyscfgexModeList = {0};


NAS_NV_SYSCFGEX_MODE_LIST* AT_GetSyscfgexModeListPtr(VOS_VOID)
{
    return &g_stSyscfgexModeList;
}


VOS_UINT8 AT_GetSyscfgexModeListItemNum(VOS_VOID)
{
    return g_stSyscfgexModeList.ucListItemNum;
}


VOS_UINT8 AT_GetSyscfgexModeRestrictFlag(VOS_VOID)
{
    return g_stSyscfgexModeList.ucRestrict;
}


VOS_UINT8* AT_GetSyscfgexModeList(VOS_VOID)
{
    return &(g_stSyscfgexModeList.aucModeList[0][0]);
}



VOS_UINT32 AT_CheckModeListParaValid( NAS_NV_SYSCFGEX_MODE_LIST *pstSyscfgexModeList )
{
    VOS_UINT8                i;
    VOS_UINT8                ucIndex;
    VOS_UINT8                aucModeListTmp[MODE_LIST_MAX_NUM][MODE_LIST_MAX_LEN];
    VOS_UINT8               *paucTemp = VOS_NULL_PTR;
    VOS_UINT32               ulSpecialSetting = VOS_FALSE;
    /*lint -e516*/
    MBB_MEM_SET(&(aucModeListTmp[0][0]), 0x00, MODE_LIST_MAX_NUM * MODE_LIST_MAX_LEN);
    /*lint +e516*/

    /*参数检查*/
    if ( VOS_NULL_PTR == pstSyscfgexModeList )
    {
        return VOS_FALSE;
    }

    /*列表数为0，认为NV未设置*/
    if ( 0 == pstSyscfgexModeList->ucListItemNum )
    {
        return VOS_FALSE;
    }
    /*lint -e516*/
    MBB_MEM_CPY( aucModeListTmp, pstSyscfgexModeList->aucModeList, MODE_LIST_MAX_NUM * MODE_LIST_MAX_LEN );
    /*lint +e516*/
    for ( i = 0; i < pstSyscfgexModeList->ucListItemNum; i++ )
    {
        /*读取一个组合*/
        paucTemp = aucModeListTmp[i];
        if ( SYSCFGEX_MODE_INVALID == *paucTemp )
        {
            return VOS_FALSE;
        }

        ucIndex = 0;
        ulSpecialSetting = VOS_FALSE;
        
        while ( (MODE_LIST_MAX_LEN != ucIndex) && (SYSCFGEX_MODE_INVALID != *paucTemp) )
        {
            /*之前出现过AUTO或者NOCHANGE，直接返回失败*/
            if ( VOS_TRUE == ulSpecialSetting )
            {
                return VOS_FALSE;
            }

            /*存在非法值*/
            if ( (*paucTemp > SYSCFGEX_MODE_RAT_MAX) 
              && (*paucTemp != SYSCFGEX_MODE_NO_CHANGE) )
            {
                return VOS_FALSE;
            }

            /*特殊标志*/
            if ( (SYSCFGEX_MODE_AUTO == *paucTemp) || (SYSCFGEX_MODE_NO_CHANGE == *paucTemp) )
            {
                /*非第一个接入技术，返回失败*/
                ulSpecialSetting = VOS_TRUE;
                if ( 0 != ucIndex )
                {
                    return VOS_FALSE;
                }
            }

            paucTemp++;
            ucIndex++;
        }
    }

    return VOS_TRUE;
}


VOS_VOID AT_ReadSyscfgexBandListFromNV( VOS_VOID )
{
    VOS_UINT32                            ulRet;
    NAS_NV_SYSCFGEX_MODE_LIST             stSyscfgexModeList = {0};

    ulRet = NV_Read( NV_ID_HUAWEI_SYSCFGEX_MODE_LIST, &stSyscfgexModeList,
                     sizeof(NAS_NV_SYSCFGEX_MODE_LIST) );
    if ( NV_OK == ulRet )
    {
        if ( VOS_TRUE == AT_CheckModeListParaValid(&stSyscfgexModeList) )
        {
            /*lint -e516*/
            MBB_MEM_CPY( AT_GetSyscfgexModeListPtr(), &stSyscfgexModeList,
                        sizeof(NAS_NV_SYSCFGEX_MODE_LIST) );
            /*lint +e516*/
        }
        else
        {
            /*NV非法，不处理*/
        }
    }
    else
    {
        /*读取NV失败, 什么都不做*/
    }
}
/*********************************************************************************************
 函 数 名  : AT_ReadRfCapabilityNV
 功能描述  : at 初始化时通过读取单板频段NV判断RAT支持情况
 输入参数  : 无
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :
 修改历史  :
*********************************************************************************************/
VOS_VOID AT_ReadRfCapabilityNV(VOS_VOID)
{
    AT_NV_WG_RF_MAIN_BAND_STRU       stRfMainBand = {0};
    VOS_UINT8                        *pucAcqOrder = NULL;
    VOS_UINT32                        ulRst = NV_OK;
    LRRC_NV_UE_EUTRA_CAP_STRU        *pstEutraCap = NULL;
    pstEutraCap = (LRRC_NV_UE_EUTRA_CAP_STRU*)PS_MEM_ALLOC(WUEPS_PID_AT,
                            sizeof(LRRC_NV_UE_EUTRA_CAP_STRU));

    if (VOS_NULL == pstEutraCap)
    {
        /*NV读取失败需要对全局变量赋值为GUL全部支持*/
        g_MbbIsRatSupport.ucLteSupport = VOS_TRUE;
        g_MbbIsRatSupport.ucWcdmaSupport = VOS_TRUE;
        g_MbbIsRatSupport.ucGsmSupport = VOS_TRUE;
        /*lint -e516*/
        MBB_MEM_CPY((VOS_CHAR *) g_MbbIsRatSupport.aucAutoAcqorder, "030201", SYSCFGEX_MAX_RAT_STRNUM - 1);
        /*lint +e516*/
        (VOS_VOID)vos_printf("AT_ReadRfCapabilityNV():malloc for 0xd22c Failed!");
        return;
    }
    /*lint -e516*/
    MBB_MEM_SET(pstEutraCap, 0x00, sizeof(LRRC_NV_UE_EUTRA_CAP_STRU));
    /*lint +e516*/

    ulRst = NV_Read(en_NV_Item_WG_RF_MAIN_BAND, &stRfMainBand, sizeof(stRfMainBand));

    if (NV_OK != ulRst)
    {
        /*NV读取失败需要对全局变量赋值为GUL全部支持*/
        g_MbbIsRatSupport.ucLteSupport = VOS_TRUE;
        g_MbbIsRatSupport.ucWcdmaSupport = VOS_TRUE;
        g_MbbIsRatSupport.ucGsmSupport = VOS_TRUE;
        /*lint -e516*/
        MBB_MEM_CPY((VOS_CHAR *) g_MbbIsRatSupport.aucAutoAcqorder, "030201", SYSCFGEX_MAX_RAT_STRNUM - 1);
        (VOS_VOID)vos_printf("AT_ReadRfCapabilityNV():Read en_NV_Item_WG_RF_MAIN_BAND Failed!\n");
        MBB_MEM_FREE(WUEPS_PID_AT, pstEutraCap);
        /*lint +e516*/
        return;
    }

    ulRst = NVM_Read(EN_NV_ID_UE_CAPABILITY, pstEutraCap, sizeof(LRRC_NV_UE_EUTRA_CAP_STRU));

    if (NV_OK != ulRst)
    {
        /*NV读取失败需要对全局变量赋值为GUL全部支持*/
        g_MbbIsRatSupport.ucLteSupport = VOS_TRUE;
        g_MbbIsRatSupport.ucWcdmaSupport = VOS_TRUE;
        g_MbbIsRatSupport.ucGsmSupport = VOS_TRUE;
        /*lint -e516*/
        MBB_MEM_CPY((VOS_CHAR *) g_MbbIsRatSupport.aucAutoAcqorder, "030201", SYSCFGEX_MAX_RAT_STRNUM - 1);
        (VOS_VOID)vos_printf("AT_ReadRfCapabilityNV():Read EN_NV_ID_UE_CAPABILITY Failed!\n");
        MBB_MEM_FREE(WUEPS_PID_AT, pstEutraCap);
        /*lint +e516*/
        return;
    }

    pucAcqOrder = g_MbbIsRatSupport.aucAutoAcqorder;

    /*LTE 频段不为空*/
    if(0 != pstEutraCap->stRfPara.usCnt)
    {
        g_MbbIsRatSupport.ucLteSupport = VOS_TRUE;
        /*lint -e516*/
        MBB_MEM_CPY((VOS_CHAR *) pucAcqOrder, "03", AT_SYSCFGEX_RAT_MODE_STR_LEN);
        /*lint +e516*/
        pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
    }

    if(0 != stRfMainBand.unWcdmaBand.ulBand)
    {
        g_MbbIsRatSupport.ucWcdmaSupport = VOS_TRUE;
        /*lint -e516*/
        MBB_MEM_CPY((VOS_CHAR *) pucAcqOrder, "02", AT_SYSCFGEX_RAT_MODE_STR_LEN);
        /*lint +e516*/
        pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
    }

    if(0 != stRfMainBand.unGsmBand.ulBand)
    {
        g_MbbIsRatSupport.ucGsmSupport = VOS_TRUE;
        /*lint -e516*/
        MBB_MEM_CPY((VOS_CHAR *) pucAcqOrder, "01", AT_SYSCFGEX_RAT_MODE_STR_LEN);
        /*lint +e516*/
        pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
    }

    *pucAcqOrder = '\0';

    MBB_MEM_FREE(WUEPS_PID_AT, pstEutraCap);
    return;
}

/*****************************************************************************
 函 数 名  : At_GetSupportRatOrderMbb
 功能描述  : 格式化syscfg的at返回
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_VOID   At_GetSupportRatOrderMbb(VOS_CHAR *pachDest)
{
    VOS_UINT32                          ulLength = 0;

    /*添加括号*/
    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest,  "(\"00\"," );/*lint !e64*/

    if(VOS_TRUE == g_MbbIsRatSupport.ucGsmSupport)
    {
        ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength,  "\"01\","); /*lint !e64*/
    }

    if(VOS_TRUE == g_MbbIsRatSupport.ucWcdmaSupport)
    {
        ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength,  "\"02\"," );/*lint !e64*/
    }

    /*LTE 频段不为空*/
    if(VOS_TRUE == g_MbbIsRatSupport.ucLteSupport)
    {
        ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength,  "\"03\"," );/*lint !e64*/
    }

    (VOS_VOID)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength, "\"99\")" );/*lint !e64*/

    return;
}


VOS_UINT32 TAF_SyscfgexLteHighBand2Str(TAF_PH_PREF_BAND band,TAF_CHAR *strBand)
{
    VOS_UINT32 i,ulLen;
    VOS_UINT32 ulBandStrLen = 0;
    TAF_CHAR *strSep = "/";
    TAF_CHAR buffer_temp[TAF_PH_BAND_LEN] = {0};
    VOS_UINT32 itemp_len = 0;

    ulLen = sizeof(gastSyscfgexLteHighBandStr) / sizeof(gastSyscfgexLteHighBandStr[0]);

    for (i = 0; i < ulLen; i++)
    {
        if(0 != (band & gastSyscfgexLteHighBandStr[i].BandMask))
        {
            /*lint -e516, -e534*/
            MBB_MEM_SET(buffer_temp, 0x00, TAF_PH_BAND_LEN);
            MBB_MEM_CPY(buffer_temp, gastSyscfgexLteHighBandStr[i].BandStr,
                            VOS_StrLen(gastSyscfgexLteHighBandStr[i].BandStr));
            itemp_len = VOS_StrLen(buffer_temp);            

            MBB_MEM_CPY(buffer_temp + itemp_len, strSep,VOS_StrLen(strSep));

            itemp_len += VOS_StrLen(strSep);
            /*字符串过长*/
            if((TAF_PH_SYSCFG_GROUP_BAND_LEN / 2) <= ulBandStrLen + itemp_len + VOS_StrLen(strSep))
            {
                break;
            }
            MBB_MEM_CPY(strBand + ulBandStrLen, buffer_temp, itemp_len);
            ulBandStrLen += itemp_len;
            /*lint +e516, +e534*/
        }
    }

    if (ulBandStrLen > 0)
    {
        strBand[ulBandStrLen - 1] = '\0';
    }

    return TAF_SUCCESS;
}


VOS_UINT32 TAF_SyscfgexLteLowBand2Str(TAF_PH_PREF_BAND band,TAF_CHAR *strBand)
{
    VOS_UINT32 i,ulLen;
    VOS_UINT32 ulBandStrLen = 0;
    TAF_CHAR *strSep = "/";
    TAF_CHAR buffer_temp[TAF_PH_BAND_LEN] = {0};
    VOS_UINT32 itemp_len = 0;

    ulLen = sizeof(gastSyscfgexLteLowBandStr) / sizeof(gastSyscfgexLteLowBandStr[0]);

    for (i = 0; i < ulLen; i++)
    {
        if(0 != (band & gastSyscfgexLteLowBandStr[i].BandMask))
        {
             /*lint -e534, -e516*/
            MBB_MEM_SET(buffer_temp, 0x00, TAF_PH_BAND_LEN);
           
            MBB_MEM_CPY(buffer_temp, gastSyscfgexLteLowBandStr[i].BandStr,
                            VOS_StrLen(gastSyscfgexLteLowBandStr[i].BandStr));
            itemp_len = VOS_StrLen(buffer_temp);            

            MBB_MEM_CPY(buffer_temp + itemp_len, strSep,VOS_StrLen(strSep));

            itemp_len += VOS_StrLen(strSep);

            if((TAF_PH_SYSCFG_GROUP_BAND_LEN / 2) <= ulBandStrLen + itemp_len + VOS_StrLen(strSep))
            {
                break;
            }
            MBB_MEM_CPY(strBand + ulBandStrLen, buffer_temp, itemp_len);
            ulBandStrLen += itemp_len;            
            /*lint +e516, +e534*/
        }
    }

    if (ulBandStrLen > 0)
    {
        strBand[ulBandStrLen - 1] = '\0';
    }

    return TAF_SUCCESS;
}

VOS_VOID    AT_GetLTEBandStrMbb(TAF_PH_SYSCFG_BAND_STR *pstSysCfgBandStr, TAF_MMA_SYSCFG_TEST_CNF_STRU* pstSysCfgTestCnf)
{ 
    TAF_PH_PREF_BAND  ltelowbandnone = MN_MMA_LTE_LOW_BAND_NONE;
    TAF_CHAR *strTempLteBandGroup = pstSysCfgBandStr->strSysCfgBandGroup;
    TAF_CHAR *LteBandStr1 = VOS_NULL_PTR;
    TAF_CHAR *LteBandStr2 = VOS_NULL_PTR;

    if ((0 == pstSysCfgTestCnf->ulSysCfgExLTEBandGroup1) && (0 == pstSysCfgTestCnf->ulSysCfgExLTEBandGroup2))
    {
        (VOS_VOID)At_sprintf(TAF_PH_SYSCFG_GROUP_BAND_LEN, strTempLteBandGroup,
                                strTempLteBandGroup, "((7fffffffffffffff,\"All bands\"))");/*lint !e64*/
        return;
    }

    /*高低位频段字符串的长度分别为总长度的一半*/
    LteBandStr1 = (TAF_CHAR*)MBB_MEM_ALLOC(WUEPS_PID_AT, (TAF_PH_SYSCFG_GROUP_BAND_LEN / 2));

    if (VOS_NULL_PTR == LteBandStr1)
    {
        (VOS_VOID)At_sprintf(TAF_PH_SYSCFG_GROUP_BAND_LEN, strTempLteBandGroup,
                                strTempLteBandGroup, "((7fffffffffffffff,\"All bands\"))");/*lint !e64*/
        return;
    }
    /*高低位频段字符串的长度分别为总长度的一半*/
    LteBandStr2 = (TAF_CHAR*)MBB_MEM_ALLOC(WUEPS_PID_AT, (TAF_PH_SYSCFG_GROUP_BAND_LEN / 2));

    if (VOS_NULL_PTR == LteBandStr2)
    {
        (VOS_VOID)At_sprintf(TAF_PH_SYSCFG_GROUP_BAND_LEN, strTempLteBandGroup,
                                strTempLteBandGroup, "((7fffffffffffffff,\"All bands\"))");/*lint !e64*/
        /*lint -e516*/
        MBB_MEM_FREE(WUEPS_PID_AT, LteBandStr1);
        /*lint +e516*/
        return;
    }
    /*高低位频段字符串的长度分别为总长度的一半*/
    /*lint -e516 -e449 -e668*/
    MBB_MEM_SET(LteBandStr1, 0,(TAF_PH_SYSCFG_GROUP_BAND_LEN / 2));
    /*高低位频段字符串的长度分别为总长度的一半*/
    MBB_MEM_SET(LteBandStr2, 0,(TAF_PH_SYSCFG_GROUP_BAND_LEN / 2));
    /*lint +e516 +e449 +e668*/
    if ((0 != pstSysCfgTestCnf->ulSysCfgExLTEBandGroup1) && (0 != pstSysCfgTestCnf->ulSysCfgExLTEBandGroup2))
    {
        (VOS_VOID)TAF_SyscfgexLteHighBand2Str(pstSysCfgTestCnf->ulSysCfgExLTEBandGroup1, LteBandStr1);
        (VOS_VOID)TAF_SyscfgexLteLowBand2Str(pstSysCfgTestCnf->ulSysCfgExLTEBandGroup2, LteBandStr2);

        (VOS_VOID)At_sprintf(TAF_PH_SYSCFG_GROUP_BAND_LEN,strTempLteBandGroup,strTempLteBandGroup, 
                                           "((%x%08x,\"%s/%s\"),(7fffffffffffffff,\"All bands\"))",/*lint !e64*/
                                           pstSysCfgTestCnf->ulSysCfgExLTEBandGroup1, 
                                           pstSysCfgTestCnf->ulSysCfgExLTEBandGroup2,
                                           LteBandStr2,
                                           LteBandStr1);/*lint !e119*/
    }
    else if  (0 != pstSysCfgTestCnf->ulSysCfgExLTEBandGroup1)
    {
        (VOS_VOID)TAF_SyscfgexLteHighBand2Str(pstSysCfgTestCnf->ulSysCfgExLTEBandGroup1, LteBandStr1);

        (VOS_VOID)At_sprintf(TAF_PH_SYSCFG_GROUP_BAND_LEN, strTempLteBandGroup,
                                strTempLteBandGroup, "((%x%08x,\"%s\"),(7fffffffffffffff,\"All bands\"))",/*lint !e64*/
                                 pstSysCfgTestCnf->ulSysCfgExLTEBandGroup1, ltelowbandnone, LteBandStr1);/*lint !e119*/
    }
    else
    {
        (VOS_VOID)TAF_SyscfgexLteLowBand2Str(pstSysCfgTestCnf->ulSysCfgExLTEBandGroup2, LteBandStr2);
        (VOS_VOID)At_sprintf(TAF_PH_SYSCFG_GROUP_BAND_LEN,strTempLteBandGroup,
                                strTempLteBandGroup, "((%x,\"%s\"),(7fffffffffffffff,\"All bands\"))",/*lint !e64*/
                                pstSysCfgTestCnf->ulSysCfgExLTEBandGroup2, LteBandStr2);/*lint !e119*/
    }
    /*lint -e516*/
    MBB_MEM_FREE(WUEPS_PID_AT, LteBandStr1);
    MBB_MEM_FREE(WUEPS_PID_AT, LteBandStr2);
    /*lint +e516*/
    return;
}

VOS_UINT32 AT_GenerateModeListFromNV( VOS_CHAR *pachDest )
{
    VOS_UINT8                i;
    VOS_UINT8                ucIndex;
    VOS_UINT8                aucModeList[MODE_LIST_MAX_NUM][MODE_LIST_MAX_LEN];
    VOS_UINT8               *paucTemp = VOS_NULL_PTR;           
    VOS_UINT32               ulLength = 0;
    VOS_UINT8                ucListItemNum = AT_GetSyscfgexModeListItemNum();
    VOS_UINT8               *paucModeList = AT_GetSyscfgexModeList();
    /*lint  -e516*/
    MBB_MEM_SET(aucModeList, 0x00, MODE_LIST_MAX_NUM * MODE_LIST_MAX_LEN);
    MBB_MEM_CPY( aucModeList, paucModeList, MODE_LIST_MAX_NUM * MODE_LIST_MAX_LEN);
    /*lint  +e516*/
    /*当前NV无效，走默认分支*/
    if ( 0 == ucListItemNum )
    {
        return VOS_FALSE;
    }

    /*添加括号*/
    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest, "%s", "(" );/*lint !e64 !e119*/

    for ( i = 0; i < ucListItemNum; i++ )
    {
        paucTemp = aucModeList[i];
        if ( SYSCFGEX_MODE_INVALID == *paucTemp )
        {
            continue;
        }

        ucIndex = 0;

        /*每个组合前添加引号*/
        ulLength += (VOS_UINT32)At_sprintf( DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength, "%s", "\"" );/*lint !e64 !e119*/
        while ( (MODE_LIST_MAX_LEN != ucIndex) && (SYSCFGEX_MODE_INVALID != *paucTemp) )
        {
            switch ( *paucTemp )
            {   
                /*针对每个接入技术，增加对应的字符串*/
                case SYSCFGEX_MODE_AUTO:
                    ulLength += (VOS_UINT32)At_sprintf( DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength, "%s", "00" );/*lint !e64 !e119*/
                    break;
                case SYSCFGEX_MODE_GSM:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength, "%s", "01" );/*lint !e64 !e119*/
                    break;
                case SYSCFGEX_MODE_WCDMA:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "02" );/*lint !e64 !e119*/
                    break;
                case SYSCFGEX_MODE_LTE:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "03" );/*lint !e64 !e119*/
                    break;
                case SYSCFGEX_MODE_CDMA:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "04" );/*lint !e64 !e119*/
                    break;
                case SYSCFGEX_MODE_TDSCDMA:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "05" );/*lint !e64 !e119*/
                    break;
                case SYSCFGEX_MODE_WIMAX:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "06" );/*lint !e64 !e119*/
                    break;
                case SYSCFGEX_MODE_NOT_CHANGE:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "99" );/*lint !e64 !e119*/
                    break;
                default:
                    return VOS_FALSE;       /*存在非法值，直接返回错误*/
            }

            paucTemp++;
            ucIndex++;
        }
        /*组合之后添加引号*/
        ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "\"" );/*lint !e64 !e119*/

        if ( i != (ucListItemNum - 1) )
        {
            /*每个组合结束后，添加逗号*/
            ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,   pachDest + ulLength, "%s", "," );/*lint !e64 !e119*/
        }
    }
    /*添加右括号*/
    (VOS_VOID)At_sprintf(DEST_STRING_MAX_LEN, pachDest,   pachDest + ulLength, "%s", ")" );/*lint !e64 !e119*/

    return VOS_TRUE;
}
/*****************************************************************************
 函 数 名  : At_FormatSyscfgMbb
 功能描述  : 格式化syscfg的at返回
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_VOID At_FormatSyscfgMbb(AT_MODEM_NET_CTX_STRU *pstNetCtx, TAF_MMA_SYSCFG_TEST_CNF_STRU* pstSysCfgTestCnf, VOS_UINT8 ucIndex)
{
    VOS_CHAR                            achModeStr[DEST_STRING_MAX_LEN] = { 0 };
    VOS_CHAR                            strSysCfgLTEBandGroup[TAF_PH_SYSCFG_GROUP_BAND_LEN]= {"\0"};

    AT_GetLTEBandStrMbb((TAF_PH_SYSCFG_BAND_STR*)strSysCfgLTEBandGroup, pstSysCfgTestCnf);
    if ( VOS_TRUE == AT_GenerateModeListFromNV( achModeStr ) )
    {
        AT_STOP_TIMER_CMD_READY(ucIndex);/*lint !e515*/
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                                            "%s: "
                                                            "%s,"
                                                            "%s,"
                                                            "(0-2),"
                                                            "(0-4),"
                                                            "%s",/*lint !e64 !e119*/
                                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                            achModeStr,
                                                            pstSysCfgTestCnf->stBandInfo.strSysCfgBandGroup,
                                                            strSysCfgLTEBandGroup);/*lint !e64 !e119*/
    }
    else
    {
        At_GetSupportRatOrderMbb(achModeStr);
        AT_STOP_TIMER_CMD_READY(ucIndex);/*lint !e515*/
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                                            "%s:"
                                                            "%s,"
                                                            "%s,"
                                                            "(0-2),"
                                                            "(0-4),"
                                                            "%s",/*lint !e64 !e119*/
                                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                            achModeStr,
                                                            pstSysCfgTestCnf->stBandInfo.strSysCfgBandGroup,
                                                            strSysCfgLTEBandGroup);/*lint !e64 !e119*/
    }
}/*lint !e550*/
/*****************************************************************************
 函 数 名  : At_FormatEons0
 功能描述  : 格式化eons=0的at返回
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_VOID At_FormatSysinfoExMbb(VOS_UINT16* usLength, TAF_PH_SYSINFO_STRU* stSysInfo)
{
    if (TAF_PH_INFO_NO_SERV == stSysInfo->ucSrvStatus)
    {
        *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *usLength,",%d",TAF_PH_INFO_NO_DOMAIN);/*lint !e64 !e119*/
    }
    else 
    {
        *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *usLength,",%d",stSysInfo->ucSrvDomain);/*lint !e64 !e119*/
    }
}


VOS_UINT32 At_CheckSpecRatOrderInModeList(AT_SYSCFGEX_RAT_ORDER_STRU *pstSysCfgExRatOrder)
{
    VOS_UINT8                   i, j;
    VOS_UINT8                  *pucTemp;
    VOS_UINT8                   aucModeList[MODE_LIST_MAX_NUM][MODE_LIST_MAX_LEN] = {0};
    VOS_UINT8                   ucListItemNum = AT_GetSyscfgexModeListItemNum();
    VOS_UINT8                  *paucModeList = AT_GetSyscfgexModeList();
    /*lint  -e516*/
    MBB_MEM_CPY( aucModeList, paucModeList, (MODE_LIST_MAX_NUM * MODE_LIST_MAX_LEN) );
    /*lint  +e516*/

    if ( VOS_NULL_PTR == pstSysCfgExRatOrder )
    {
        return VOS_FALSE;
    }

    for ( i = 0; i < ucListItemNum; i++ )
    {
        /*进行匹配*/
        pucTemp = aucModeList[i];
        for ( j = 0; j < pstSysCfgExRatOrder->ucRatOrderNum; j++ )
        {
            if ( (pstSysCfgExRatOrder->aenRatOrder[j] == pucTemp[j])
            /*对于99需要额外判断*/
              || ((99 == pstSysCfgExRatOrder->aenRatOrder[j]) && (0x99 == pucTemp[j])) )
            {
                continue;
            }
            else
            {
                break;
            }
        }
        /*匹配成功*/
        if ( (j == pstSysCfgExRatOrder->ucRatOrderNum) && (0xFF == pucTemp[j]) )
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


/*****************************************************************************
 函 数 名  : AT_MBBConverAutoMode
 功能描述  : 转化SYSCFGEX命令设置的ATUO模式为单板实际支持模式的全集
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_VOID AT_MBBConverAutoMode(TAF_MMA_RAT_ORDER_STRU    *pstSysCfgRatOrder)
{
    VOS_UINT8                           ucUserRatNum = 0;

        if (VOS_TRUE == g_MbbIsRatSupport.ucLteSupport)
        {
             pstSysCfgRatOrder->aenRatOrder[ucUserRatNum] = TAF_MMA_RAT_LTE;
             ucUserRatNum++;
        }

    /* 平台支持WCDMA */
    if (VOS_TRUE == g_MbbIsRatSupport.ucWcdmaSupport)
    {
         pstSysCfgRatOrder->aenRatOrder[ucUserRatNum] = TAF_MMA_RAT_WCDMA;
         ucUserRatNum++;
    }

    /* 平台支持GSM */
    if (VOS_TRUE == g_MbbIsRatSupport.ucGsmSupport)
    {
         pstSysCfgRatOrder->aenRatOrder[ucUserRatNum] = TAF_MMA_RAT_GSM;
         ucUserRatNum++;
    }

    pstSysCfgRatOrder->ucRatOrderNum = ucUserRatNum;
    return;
}

VOS_UINT32 AT_PS_CheckSyscfgexModeRestrictPara(VOS_UINT32* ulRst, AT_SYSCFGEX_RAT_ORDER_STRU* stSyscfgExRatOrder)
{
    VOS_UINT32 ulRet;
    VOS_UINT8 ucRestrict = AT_GetSyscfgexModeRestrictFlag();
    if ( VOS_TRUE == ucRestrict )
    {
        /*仅列表中设置的组合可以设置*/
        ulRet = At_CheckSpecRatOrderInModeList( stSyscfgExRatOrder );
        if ( VOS_FALSE == ulRet )
        {
            *ulRst = AT_CME_INCORRECT_PARAMETERS;
            return VOS_FALSE;
        }
    }
    return VOS_TRUE;
}


VOS_UINT32 At_GetTheCustomVersion(VOS_VOID)
{
    return VOS_TRUE;
}

extern TAF_VOID At_PbUnicode80FormatPrint(TAF_UINT8 *pucSrc, TAF_UINT16 usSrcLen, TAF_UINT8 *pucDst, TAF_UINT16 *pusDstLen);
extern TAF_VOID At_PbUnicode81FormatPrint(TAF_UINT8 *puc81Code, TAF_UINT8 *pucDst, TAF_UINT16 *pusDstLen);
extern TAF_VOID At_PbUnicode82FormatPrint(TAF_UINT8 *puc82Code, TAF_UINT8 *pucDst, TAF_UINT16 *pusDstLen);

TAF_UINT16 At_TafCallBackMakeEons(TAF_UINT8 ucIndex ,TAF_PH_EONS_INFO_STRU *pstEonsInfo)
{
    VOS_UINT16 usLength = 0;
    /*添加接收TYPE值，接收C核传入值进行打印显示*/
    VOS_UINT16 usType = 0;

    usType = pstEonsInfo->stPlmnInfo.ucEonsType;
    
    /*将^EONS:组装进入传输字符串*/
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s:%d", g_stParseContext[ucIndex].pstCmdElement->pszCmdName,usType);/*lint !e64 !e119*/

    if(TAF_REPORT_SRVSTA_NORMAL_SERVICE == pstEonsInfo->stPlmnInfo.ucServiceStatus || 0 != pstEonsInfo->stPlmnInfo.PlmnLen)
    {
        /*将MCC组装进入传输字符串*/
        pstEonsInfo->stPlmnInfo.PlmnId.Mcc &= 0x0FFF;/*MCC取三位*/
        if(AT_PLMN_ID_MAX_LEN == pstEonsInfo->stPlmnInfo.PlmnLen)
        {
            pstEonsInfo->stPlmnInfo.PlmnId.Mnc &= 0x0FFF;/*MNC取三位*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%03X%03X",/*lint !e64 !e119*/
                                                pstEonsInfo->stPlmnInfo.PlmnId.Mcc,
                                                pstEonsInfo->stPlmnInfo.PlmnId.Mnc);/*lint !e64 !e119*/
        }
        else
        {
            pstEonsInfo->stPlmnInfo.PlmnId.Mnc &= 0x00FF;/*MNC取两位位*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%03X%02X",/*lint !e64 !e119*/
                                                 pstEonsInfo->stPlmnInfo.PlmnId.Mcc,
                                                 pstEonsInfo->stPlmnInfo.PlmnId.Mnc);/*lint !e64 !e119*/
        }
        
        /* 长名输出 */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                      (TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"");/*lint !e64 !e119*/
        usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                      (VOS_INT8 *)pgucAtSndCodeAddr,
                                      (VOS_UINT8 *)(pgucAtSndCodeAddr + usLength),
                                      pstEonsInfo->stNWName.stLNameInfo.aucName, 
                                      pstEonsInfo->stNWName.stLNameInfo.ucNameLen);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                      (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");/*lint !e64 !e119*/

        /* 短名输出 */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                     (TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"");/*lint !e64 !e119*/
                                     
        usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                     (VOS_INT8 *)pgucAtSndCodeAddr,
                                                     (VOS_UINT8 *)(pgucAtSndCodeAddr + usLength),
                                                     pstEonsInfo->stNWName.stSNameInfo.aucName, 
                                                     pstEonsInfo->stNWName.stSNameInfo.ucNameLen);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                                     (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");/*lint !e64 !e119*/

        if( 0 < pstEonsInfo->stSpnInfo.ucSpnLen)          
        {
            /*将SPN 显示条件标志位组装入字符串*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                                  (TAF_CHAR *)pgucAtSndCodeAddr,
                                                                  (TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",/*lint !e64 !e119*/
                                                                  pstEonsInfo->stSpnInfo.ucDispMode);/*lint !e64 !e119*/

            /*将SPN 信息组装入字符串*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                                                  (TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"");/*lint !e64 !e119*/
            usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                         (VOS_INT8 *)pgucAtSndCodeAddr,
                                                         (VOS_UINT8 *)(pgucAtSndCodeAddr + usLength),
                                                         pstEonsInfo->stSpnInfo.aucSpn,
                                                         pstEonsInfo->stSpnInfo.ucSpnLen);
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");/*lint !e64 !e119*/
        }
    }


    /*将回车换行组装入字符串*/
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s",gaucAtCrLf);/*lint !e64 !e119*/
    
    return usLength;
}/*lint !e550*/


TAF_VOID At_Ascii2Unicode( TAF_UINT8 *pucSrc, TAF_UINT16 usSrcLen,  TAF_UINT8 *pusdest, TAF_UINT16 *pusDstLen)
{
    TAF_UINT16 usSrcOffSet = 0;
    TAF_UINT16 usDstOffSet = 0;

    if( (VOS_NULL_PTR == pucSrc) || (VOS_NULL_PTR == pusdest) || (VOS_NULL_PTR == pusDstLen) || ( 1 > usSrcLen ))
    {
         (VOS_VOID)vos_printf("At_Ascii2Unicode():paramater error!\n");
        return;
    }

    while( ('\0' != *pucSrc) && ( usSrcLen > usSrcOffSet ))
    {
        *(pusdest + usDstOffSet) = 0x00;

        usDstOffSet++;
        *(pusdest + usDstOffSet) = (*(pucSrc + usSrcOffSet));
         
        usDstOffSet++;
        usSrcOffSet++;
    }

    *pusDstLen = usDstOffSet;
}


VOS_UINT32  AT_UnPack7Bit(
    PRIVATE_CONST VOS_UINT8                     *pucOrgChar,
    VOS_UINT32                          ulLen,
    VOS_UINT8                           ucFillBit,
    VOS_UINT8                           *pucUnPackedChar
)
{
    /*存放字节地址*/
    VOS_UINT32                          ulPos = 0;
     /*存放位偏移*/
    VOS_UINT32                          ulOffset;
    VOS_UINT32                          ulLoop;

    if ((TAF_NULL_PTR == pucOrgChar)
     || (TAF_NULL_PTR == pucUnPackedChar))
    {
        AT_WARN_LOG("AT_UnPack7Bit ERROR: bad parameter ");
        return AT_FAILURE;
    }

    /*根据协议23040 9.2.3.24 UDHL和UDH后面是Fill Bits和SM，去掉Fill Bits后就是SM(Unit: Septet),可以获得SM中包含字符个数*/
    ulOffset = ucFillBit % 8;

    /*第一步，移出当前无效的偏移位ulOffset，得到字符的低(8 - ulOffset)位，
      第二步，若(8 - ulOffset)小于7位，需要从下一个OCTET中获取高(7 - (8 - ulOffset))位
      第三步，获取下一个数据源的下标(ulPos)和需要去除的数据位(偏移位ulOffset)*/
    for (ulLoop = 0; ulLoop < ulLen; ulLoop++)
    {
        pucUnPackedChar[ulLoop] = (VOS_UINT8)(pucOrgChar[ulPos] >> ulOffset);
        if (ulOffset > 1)
        {
            pucUnPackedChar[ulLoop] |= (VOS_UINT8)((pucOrgChar[ulPos + 1] << (8 - ulOffset)) & AT_MSG_7BIT_MASK);
        }
        else
        {
            pucUnPackedChar[ulLoop] &= AT_MSG_7BIT_MASK;
        }

        ulPos   += (ulOffset + 7) / 8;
        ulOffset = (ulOffset + 7) % 8;
    }

    return AT_SUCCESS;
}



TAF_UINT16 AT_EONSName2Unicode(TAF_UINT8 *pucSrc, TAF_UINT16 usSrcLen, TAF_UINT8 ucCode, TAF_UINT8 ucSrcMaxLen)
{
   TAF_UINT8     aucTmpSrc[TAF_PH_OPER_PNN_USC2_CODE_LEN + 1] = {0};
   TAF_UINT16    usTmplen = 0;

    /* 传入参数检查 */
    if( (VOS_NULL_PTR == pucSrc) || ( 1 > usSrcLen))
    {
        AT_WARN_LOG("At_TafEONSInfoFormat: Parameter is error.");
        return 0;
    }

    /* 把不同的编码转换为unicode编码 */
    switch(ucCode)
    {
        case TAF_PH_EONS_GSM_7BIT_DEFAULT:
            /* 转码后不能超过最大长度 */
            if ((usSrcLen * 4) > ucSrcMaxLen)
            {
                usSrcLen = ucSrcMaxLen / 4;
            }

            At_PbGsmToUnicode(pucSrc,usSrcLen,&aucTmpSrc[0],&usTmplen);
            break;

        case TAF_PH_EONS_UCS2_80:
            At_PbUnicode80FormatPrint(pucSrc,usSrcLen,&aucTmpSrc[0],&usTmplen);
            break;

        case TAF_PH_EONS_UCS2_81:
            At_PbUnicode81FormatPrint(pucSrc,&aucTmpSrc[0],&usTmplen);
            break;

        case TAF_PH_EONS_UCS2_82:
            At_PbUnicode82FormatPrint(pucSrc,&aucTmpSrc[0],&usTmplen);
            break;

        case TAF_PH_EONS_ASCII:
            /* 转码后不能超过最大长度 */
            if ((usSrcLen * 4) > ucSrcMaxLen)
            {
                usSrcLen = ucSrcMaxLen / 4;
            }

            At_Ascii2Unicode(pucSrc, usSrcLen, &aucTmpSrc[0], &usTmplen);
            break;

        default:
            break;
    }

    /* 转码后的数据保存 */
    if( 0 < usTmplen )
    {
        MBB_MEM_SET(pucSrc, 0, ucSrcMaxLen);/*lint !e516 */
        MBB_MEM_CPY((pucSrc), aucTmpSrc, usTmplen);/*lint !e516 */
        usSrcLen = usTmplen;
    }

    return usSrcLen;
}



TAF_VOID At_TafEONSInfoFormat(TAF_PH_EONS_INFO_STRU *pstEonsInfo)
{
    TAF_UINT8 ucStrLen = 0;
    TAF_UINT8 ausUnPack[TAF_PH_OPER_PNN_USC2_CODE_LEN] = {0};

    /* 参数检查 */
    if( VOS_NULL_PTR == pstEonsInfo )
    {
        AT_WARN_LOG("At_TafEONSInfoFormat: Parameter is null.");
        return;
    }

    /* 如果长名的编码方式为7bit，则先转换为8bit存储，然后在转换为uincode */
    if( TAF_PH_EONS_GSM_7BIT_PACK == pstEonsInfo->stNWName.stLNameInfo.ucNamecode)
    {
        /* 7bit编码方式的字符转换为8bit字符 */
        ucStrLen = (VOS_UINT8)((pstEonsInfo->stNWName.stLNameInfo.ucNameLen * 8) / 7);
         (VOS_VOID)AT_UnPack7Bit( pstEonsInfo->stNWName.stLNameInfo.aucName, ucStrLen, 0, &ausUnPack[0]);

        /* 对于长度超过32个字节的只输出前32个字节 */
        if( TAF_PH_OPER_NETWORTNAME_LEN < ucStrLen )
        {
            ucStrLen = TAF_PH_OPER_NETWORTNAME_LEN;
        }

        /* 保存转换后的字符和信息 */
        MBB_MEM_SET(&pstEonsInfo->stNWName.stLNameInfo, 0, sizeof(TAF_PH_NWNAME_INFO_STRU));/*lint !e516 */
        pstEonsInfo->stNWName.stLNameInfo.ucNamecode = TAF_PH_EONS_GSM_7BIT_DEFAULT;
        MBB_MEM_CPY(pstEonsInfo->stNWName.stLNameInfo.aucName, ausUnPack, ucStrLen );/*lint !e516 */
        pstEonsInfo->stNWName.stLNameInfo.ucNameLen = ucStrLen;
    }

    /* 转换长名为uincode */
    ucStrLen = (VOS_UINT8)AT_EONSName2Unicode(pstEonsInfo->stNWName.stLNameInfo.aucName,
                                                    pstEonsInfo->stNWName.stLNameInfo.ucNameLen,
                                                    pstEonsInfo->stNWName.stLNameInfo.ucNamecode,
                                                    TAF_PH_OPER_PNN_USC2_CODE_LEN);
    pstEonsInfo->stNWName.stLNameInfo.ucNameLen = ucStrLen;

    /* 如果短名的编码方式为7bit，则先转换为8bit存储，然后在转换为uincode */
    if( TAF_PH_EONS_GSM_7BIT_PACK == pstEonsInfo->stNWName.stSNameInfo.ucNamecode)
    {
        /* 7bit编码方式的字符转换为8bit字符 */
        ucStrLen = (VOS_UINT8)((pstEonsInfo->stNWName.stSNameInfo.ucNameLen * 8) / 7);
         (VOS_VOID)AT_UnPack7Bit( pstEonsInfo->stNWName.stSNameInfo.aucName, ucStrLen, 0, &ausUnPack[0]);

        /* 对于长度超过32个字节的只输出前32个字节 */
        if( TAF_PH_OPER_NETWORTNAME_LEN < ucStrLen )
        {
            ucStrLen = TAF_PH_OPER_NETWORTNAME_LEN;
        }

        /* 保存转换后的字符和信息 */
        MBB_MEM_SET(&pstEonsInfo->stNWName.stSNameInfo, 0, sizeof(TAF_PH_NWNAME_INFO_STRU));/*lint !e516 */
        pstEonsInfo->stNWName.stSNameInfo.ucNamecode = TAF_PH_EONS_GSM_7BIT_DEFAULT;
        MBB_MEM_CPY(pstEonsInfo->stNWName.stSNameInfo.aucName, ausUnPack, ucStrLen );/*lint !e516 */
        pstEonsInfo->stNWName.stSNameInfo.ucNameLen = (VOS_UINT8)ucStrLen;
    }

    /* 转换短名为uincode */
    ucStrLen = (VOS_UINT8)AT_EONSName2Unicode(pstEonsInfo->stNWName.stSNameInfo.aucName,
                                                    pstEonsInfo->stNWName.stSNameInfo.ucNameLen,
                                                    pstEonsInfo->stNWName.stSNameInfo.ucNamecode,
                                                    TAF_PH_OPER_PNN_USC2_CODE_LEN);
    pstEonsInfo->stNWName.stSNameInfo.ucNameLen = ucStrLen;

    /* 转换SPN为uincode */
    ucStrLen = (VOS_UINT8)AT_EONSName2Unicode(pstEonsInfo->stSpnInfo.aucSpn,
                                                    pstEonsInfo->stSpnInfo.ucSpnLen,
                                                    pstEonsInfo->stSpnInfo.ucSPNCode,
                                                    TAF_PH_OPER_SPN_USC2_CODE_LEN);
    pstEonsInfo->stSpnInfo.ucSpnLen = ucStrLen;
}


TAF_UINT16     At_TafCallBackNWNameProc(TAF_UINT8* pData)
{
    TAF_PH_EONS_INFO_RSP_STRU*        pstEvent = (TAF_PH_EONS_INFO_RSP_STRU*)pData;
    VOS_UINT8                         ucIndex = 0;
    VOS_UINT16                        usLength = 0;

    /*获取用户索引*/
    if(AT_FAILURE == At_ClientIdToUserId(pstEvent->ClientId,&ucIndex))
    {
        AT_WARN_LOG("At_TafCallBackNWNameProc: ClientId error.");
        return usLength;
    }

    /*检查是否是广播状态*/   
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_TafCallBackNWNameProc : AT_BROADCAST_INDEX.");
        return usLength;
    }

    /* 对获取到得数据进行格式化 */
    At_TafEONSInfoFormat(&pstEvent->stEonsInfo);

    /* 返回EONS信息 */
    usLength = At_TafCallBackMakeEons(ucIndex, &(pstEvent->stEonsInfo));

    return usLength;
}
/*****************************************************************************
 函 数 名  : At_FormatEons0
 功能描述  : 格式化dlck的at返回
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_VOID At_FormatAndSndEons0(TAF_UINT8 ucIndex, VOS_UINT32 RcvNwNameflag)
{
    TAF_UINT16 usLength = 0;
    if(VOS_TRUE == RcvNwNameflag)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "\r\n^EONS:0\r\n");/*lint !e64 !e119 */
        At_SendResultData(ucIndex,pgucAtSndCodeAddr, usLength);
    }
}

TAF_UINT32 TAF_EONSGetNWName ( MN_CLIENT_ID_T           ClientId,
                                 MN_OPERATION_ID_T        OpId,
                                 AT_TAF_PLMN_ID stPlmn)
{
    /*定义返回值类型*/
    VOS_UINT32                           ulRst;

    /* 发送消息 TAF_MSG_GET_PLMN_LIST 给 MMA 处理， */
    ulRst = MN_FillAndSndAppReqMsg(ClientId,OpId,
                                         TAF_MSG_SET_EONS_MSG,
                                         (VOS_VOID*)&stPlmn,
                                         sizeof(AT_TAF_PLMN_ID),
                                         I0_WUEPS_PID_MMA);
    return ulRst;
}


VOS_UINT32 At_CheckEonsTypeMoudlePara(AT_TAF_PLMN_ID* ptrPlmnID)
{
    VOS_UINT32 ulPlmnNameLen = 0;

    /* 入参判空检查 */
    if(VOS_NULL_PTR == ptrPlmnID)
    {
        return VOS_FALSE;
    }
    /* 参数过多，返回参数错误*/
    if (3 < gucAtParaIndex || 0 == gucAtParaIndex)
    {
        AT_WARN_LOG("At_CheckEonsPara: the aucPara too much.");
        return VOS_FALSE;
    }

     /*<plmn_name_len>：当<type>不为5时，不下发此字段,此字段必须是空*/
    if(AT_EONS_TYPE_MODULE != gastAtParaList[0].ulParaValue 
        && 0 != gastAtParaList[2].usParaLen)
    {
        return VOS_FALSE;
    } 

    /*<plmn_id>: 当Type为5时，此字段必须是空*/
    if (AT_EONS_TYPE_MODULE == gastAtParaList[0].ulParaValue
        && 0 != gastAtParaList[1].usParaLen)
    {
        return VOS_FALSE;
    }

    /* <plmn_id>: 当Type为5时, 检查输入的plmn_name_len是否合法 */
    if(AT_EONS_TYPE_MODULE == gastAtParaList[0].ulParaValue)
    {
        if(AT_FAILURE == At_Auc2ul(gastAtParaList[2].aucPara, 
                            gastAtParaList[2].usParaLen,
                            &ulPlmnNameLen))
        {
            return VOS_FALSE;
        }
        /* 运营商名称长度是否为空? 如果为空，使用默认值，否则使用当前值 */
        ptrPlmnID->PlmnNameLen.ulLsNameLen = (gastAtParaList[2].usParaLen? 
                ulPlmnNameLen : AT_ENOS_PLMN_NAME_DEFAULT_LEN);
    }

    return VOS_TRUE;
}


 VOS_UINT32 At_CheckEonsPara(AT_TAF_PLMN_ID* ptrPlmnID)
{
    VOS_UINT8 ucParaTmpLen = 0;
    VOS_UINT8 i = 0;

    VOS_UINT32  ulRet = VOS_FALSE;

    ulRet = At_CheckEonsTypeMoudlePara(ptrPlmnID);

    if (VOS_FALSE == ulRet)
    {
        AT_WARN_LOG("At_CheckEonsPara: the aucPara wrong.");
        
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*balong AT机制在入参传入时已经检查完毕，此处可以直接赋值*/
    ptrPlmnID->EonsType = (TAF_UINT8)gastAtParaList[0].ulParaValue;

    /* 第二个参数为空，获取入驻的PLMN的信息 */
    if (0 == gastAtParaList[1].usParaLen)
    {
        /* 将入驻的PLMN 信息获取 ，仅以此为标识*/
        ptrPlmnID->PlmnLen = 0;
    }
    else
    {
        ucParaTmpLen = (VOS_UINT8)gastAtParaList[1].usParaLen;
        
        /*如果当前PLMN LEN 去掉引号，不是5位或者6位返回参数错误*/
        if ((ucParaTmpLen < MMA_PLMN_ID_LEN_5) || (ucParaTmpLen > MMA_PLMN_ID_LEN_6))
        {
            AT_WARN_LOG("At_CheckEonsPara: the second aucPara len is wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /*需要判定如果传入的是非数字ID则返回参数错误*/
        for (i = 0;i < ucParaTmpLen; i++)
        {
            if((0x30 <= gastAtParaList[1].aucPara[i]) && (gastAtParaList[1].aucPara[i] <= 0x39))
            {
                ptrPlmnID->PLMNId[i] = (VOS_CHAR)gastAtParaList[1].aucPara[i];/*lint !e409  !e63*/
            }
            else
            {
                AT_WARN_LOG("At_CheckEonsPara: the second aucPara isn't 0-9.");
                return AT_CME_INCORRECT_PARAMETERS;
            }
        }

        ptrPlmnID->PlmnLen = ucParaTmpLen;

    }

    return AT_SUCCESS;
}


VOS_UINT32  At_SetEonsPara(TAF_UINT8 ucIndex)
{
   
    VOS_UINT32 ulresult = AT_CME_INCORRECT_PARAMETERS;
    AT_TAF_PLMN_ID stPlmnPara = {0};

    /*检查EONS 的入参*/
    ulresult = At_CheckEonsPara(&stPlmnPara); 
    if(AT_SUCCESS != ulresult)
    {
        AT_WARN_LOG("At_SetEonsPara: At_CheckEonsPara fail.");
        return ulresult;
    }

    /* 调用A核获取LIST 的函数 */   
    if(AT_SUCCESS == TAF_EONSGetNWName(gastAtClientTab[ucIndex].usClientId, gastAtClientTab[ucIndex].opId,stPlmnPara))
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EONS_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("At_SetCopnPara return ERROR.");
        return AT_ERROR;
    }

}


VOS_UINT32 AT_TestEonsPara(VOS_UINT8 ucIndex)
{
    /*合入测试命令支持1-5*/  
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR*)pgucAtSndCodeAddr,(TAF_CHAR*)pgucAtSndCodeAddr,
                                "%s: (1-5)",/*lint !e64 !e119 */
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119 */
    return AT_OK;
}/*lint !e550*/
MCC_ZONE_INFO_STRU g_mcc_zone_infoTbl[] =
{
    {0x202, 30  * 4 / 10, 0},  /* Greece */
    {0x204, 10  * 4 / 10, 0},  /* Greece */
    {0x206, 10  * 4 / 10, 0},  /* Holland */
    {0x208, 10  * 4 / 10, 0},  /* France */
    {0x212, 10  * 4 / 10, 0},  /* Monaco */
    {0x213, 10  * 4 / 10, 0},  /* Andorra */
    {0x214, 10  * 4 / 10, 0},  /* Spain */
    {0x216, 10  * 4 / 10, 0},  /* Hungary */
    {0x218, 10  * 4 / 10, 0},  /* Bosnia and Herzegovina, the Republic of */
    {0x219, 10  * 4 / 10, 0},  /* Croatia */
    {0x220, 10  * 4 / 10, 0},  /* Montenegro / Monte Nige Lu / Serbia*/
    {0x222, 10  * 4 / 10, 0},  /* Italy */
    {0x226, 20  * 4 / 10, 0},  /* Romania */
    {0x228, 10  * 4 / 10, 0},  /* Switzerland */
    {0x230, 10  * 4 / 10, 0},  /* Czech */
    {0x231, 10  * 4 / 10, 0},  /* Slovakia */
    {0x232, 10  * 4 / 10, 0},  /* Austria */
    {0x234, 10  * 4 / 10, 0},  /* Guernsey */
    {0x238, 10  * 4 / 10, 0},  /* Denmark */
    {0x240, 10  * 4 / 10, 0},  /* Sweden */
    {0x242, 10  * 4 / 10, 0},  /* Norway */
    {0x244, 20  * 4 / 10, 0},  /* Finland */
    {0x246, 20  * 4 / 10, 0},  /* Lithuania */
    {0x247, 20  * 4 / 10, 0},  /* Latvia */
    {0x248, 20  * 4 / 10, 0},  /* Estonia */
    {0x250, 80  * 4 / 10, 0},  /* Russia GMT+3 ~ GMT+12 */
    {0x255, 20  * 4 / 10, 0},  /* Ukraine */
    {0x257, 20  * 4 / 10, 0},  /* Belarus */
    {0x259, 20  * 4 / 10, 0},  /* Moldova */
    {0x260, 10  * 4 / 10, 0},  /* Poland */
    {0x262, 10  * 4 / 10, 0},  /* Germany */
    {0x266, 10  * 4 / 10, 0},  /* Gibraltar */
    {0x268, 0  * 4 / 10, 0},  /* Portugal */
    {0x270, 10  * 4 / 10, 0},  /* Luxembourg */
    {0x272, 0  * 4 / 10, 0},  /* Ireland  GMT+0 ~ GMT+1*/
    {0x274, 0  * 4 / 10, 0},  /*Iceland  */
    {0x276, 10  * 4 / 10, 0},  /* Albania */
    {0x278, 10  * 4 / 10, 0},  /* Malta */
    {0x280, 20  * 4 / 10, 0},  /* Cyprus */
    {0x282, 40  * 4 / 10, 0},  /* Georgia */
    {0x283, 40  * 4 / 10, 0},  /* Armenia */
    {0x284, 20  * 4 / 10, 0},  /* Bulgaria */
    {0x286, 20  * 4 / 10, 0},  /* Turkey */
    {0x288, 0  * 4 / 10, 0},  /* Faroe Islands */
    {0x290, - 20  * 4 / 10, 0},  /* Greenland GMT+0 ~ GMT-4 */
    {0x292, 10  * 4 / 10, 0},  /* San Marino */
    {0x293, 10  * 4 / 10, 0},  /* Slovenia */
    {0x294, 10  * 4 / 10, 0},  /* Macedonia */
    {0x295, 10  * 4 / 10, 0},  /* Liechtenstein */
    {0x302, - 40  * 4 / 10, 0},  /* Canada */
    {0x308, - 30  * 4 / 10, 0},  /* Saint-Pierre and Miquelon */
    {0x310, - 80 * 4 / 10, 0},  /* America */
    {0x311, - 80  * 4 / 10, 0},  /* America */
    {0x330, - 40 * 4 / 10, 0},  /* Puerto Rico */
    {0x334, - 70 * 4 / 10, 0},  /* Mexico */
    {0x338, - 50 * 4 / 10, 0},  /* Jamaica */
    {0x340, - 40 * 4 / 10, 0},  /* Martinique */
    {0x342, - 40 * 4 / 10, 0},  /* Barbados */
    {0x344, - 40 * 4 / 10, 0},  /* Antigua and Barbuda */
    {0x346, - 50 * 4 / 10, 0},  /* Cayman Islands */
    {0x348, - 40 * 4 / 10, 0},  /* The British Virgin Islands */
    {0x350, - 30 * 4 / 10, 0},  /* Bermuda */
    {0x352, - 40 * 4 / 10, 0},  /* Grenada */
    {0x354, - 40 * 4 / 10, 0},  /* Montserrat */
    {0x356, - 40 * 4 / 10, 0},  /* Saint Kitts and Nevis */
    {0x358, - 40 * 4 / 10, 0},  /* St. Lucia */
    {0x360, - 40 * 4 / 10, 0},  /* Saint Vincent and the Grenadines */
    {0x362, - 40 * 4 / 10, 0},  /* Netherlands Antilles */
    {0x363, - 40 * 4 / 10, 0},  /* Aruba */
    {0x364, - 50 * 4 / 10, 0},  /* Bahamas */
    {0x365, - 40 * 4 / 10, 0},  /* Anguilla */
    {0x366, - 50 * 4 / 10, 0},  /* Dominican */
    {0x368, - 50 * 4 / 10, 0},  /* Cuba */
    {0x370, - 50 * 4 / 10, 0},  /* Dominican Republic */
    {0x372, - 50 * 4 / 10, 0},  /* Haiti */
    {0x374, - 40 * 4 / 10, 0},  /* The Republic of Trinidad and Tobago */
    {0x376, - 50 * 4 / 10, 0},  /* Turks and Caicos Islands */
    {0x400, 40  * 4 / 10, 0},  /* Republic of Azerbaijan */
    {0x401, 40  * 4 / 10, 0},  /* Kazakhstan */
    {0x402, 60  * 4 / 10, 0},  /* Bhutan */
    {0x404, 55  * 4 / 10, 0},  /* India */
    {0x405, 55  * 4 / 10, 0},    /*India*/
    {0x410, + 50 * 4 / 10, 0},   /*Pakistan*/
    {0x412, + 45 * 4 / 10, 0},  /*Afghanistan*/
    {0x413, + 55 * 4 / 10, 0},  /*Sri Lanka*/
    {0x414, + 65 * 4 / 10, 0},  /* Myanmar */
    {0x415, + 30 * 4 / 10, 0},  /* Lebanon */
    {0x416, + 20 * 4 / 10, 0},  /* Jordan */
    {0x417, + 20 * 4 / 10, 0},  /* Syria */
    {0x418, + 30 * 4 / 10, 0},  /* Iraq */
    {0x419, + 30 * 4 / 10, 0},  /* Kuwait */
    {0x420, + 30 * 4 / 10, 0},  /* Saudi Arabia */
    {0x421, + 30 * 4 / 10, 0},  /* Yemen */
    {0x422, + 40 * 4 / 10, 0},  /* Oman */
    {0x424, + 40 * 4 / 10, 0},  /* United Arab Emirates */
    {0x425, + 20 * 4 / 10, 0},  /* Israel */
    {0x426, + 30 * 4 / 10, 0},  /* Bahrain  */
    {0x427, + 30 * 4 / 10, 0},  /* Qatar */
    {0x428, + 80 * 4 / 10, 0},  /* Mongolia */
    {0x429, + 58 * 4 / 10, 0},  /* Nepal */
    {0x432, + 35 * 4 / 10, 0},  /* Iran */
    {0x434, + 50 * 4 / 10, 0},  /* Uzbekistan */
    {0x436, + 50 * 4 / 10, 0},  /* Tajikistan */
    {0x437, + 60 * 4 / 10, 0},  /* Kyrgyzstan */
    {0x438, + 50 * 4 / 10, 0},  /* Turkmenistan */
    {0x440, + 90 * 4 / 10, 0},  /* Japan */
    {0x450, + 90 * 4 / 10, 0},  /* South Korea */
    {0x452, + 70 * 4 / 10, 0},  /* Vietnam */
    {0x454, + 80 * 4 / 10, 0},  /* Hong Kong */
    {0x455, + 80 * 4 / 10, 0},  /* Macau */
    {0x456, + 70 * 4 / 10, 0},  /* Cambodia */
    {0x457, + 70 * 4 / 10, 0},  /* Laos */
    {0x460, + 80 * 4 / 10, 0},  /*China*/
    {0x466, + 80 * 4 / 10, 0},  /* Taiwan */
    {0x467, + 90 * 4 / 10, 0},  /* North Korea */
    {0x470, + 60 * 4 / 10, 0},  /* Bangladesh */
    {0x472, + 50 * 4 / 10, 0},  /* Maldives */
    {0x502, + 80 * 4 / 10, 0},  /*  */
    {0x505, + 90 * 4 / 10, 0},  /*  */
    {0x510, + 80 * 4 / 10, 0},  /*  */
    {0x514, + 90 * 4 / 10, 0},  /*  */
    {0x515, + 80 * 4 / 10, 0},  /*  */
    {0x520, + 70 * 4 / 10, 0},  /*  */
    {0x525, + 80 * 4 / 10, 0},  /*  */
    {0x528, + 80 * 4 / 10, 0},  /*  */
    {0x530, + 120 * 4 / 10, 0},  /*  */
    {0x537, + 100 * 4 / 10, 0},  /*  */
    {0x539, + 130 * 4 / 10, 0},  /*  */
    {0x540, 110 * 4 / 10, 0},  /*  */
    {0x541, 110 * 4 / 10, 0},  /*  */
    {0x542, 120 * 4 / 10, 0},  /*  */
    {0x544, - 110 * 4 / 10, 0},  /* American Samoa*/
    {0x545, 130 * 4 / 10, 0},  /*GMT +13 - +15*/
    {0x546, 110 * 4 / 10, 0},  /*  */
    {0x547, - 100 * 4 / 10, 0},  /*  */
    {0x548, - 100 * 4 / 10, 0},  /*  */
    {0x549, - 110 * 4 / 10, 0},  /*  */
    {0x550, + 100 * 4 / 10, 0},  /*  */
    {0x552, + 90 * 4 / 10, 0},  /*  */
    {0x602, + 20 * 4 / 10, 0},  /*  */
    {0x603, + 10 * 4 / 10, 0},  /*  */
    {0x604, 0 * 4 / 10, 0},  /*  */
    {0x605, + 10 * 4 / 10, 0},  /*  */
    {0x606, + 20 * 4 / 10, 0},  /*  */
    {0x607, 0 * 4 / 10, 0},  /*  */
    {0x608, 0 * 4 / 10, 0},  /*  */
    {0x609, 0 * 4 / 10, 0},  /*  */
    {0x610, 0 * 4 / 10, 0},  /*  */
    {0x611, 0 * 4 / 10, 0},  /*  */
    {0x612, 0 * 4 / 10, 0},  /*  */
    {0x613, 0 * 4 / 10, 0},  /*  */
    {0x614, + 10 * 4 / 10, 0},  /*  */
    {0x615, 0 * 4 / 10, 0},  /*  */
    {0x616, + 10 * 4 / 10, 0},  /*  */
    {0x617, + 40 * 4 / 10, 0},  /*  */
    {0x618,   0 * 4 / 10, 0},  /*  */
    {0x619,   0 * 4 / 10, 0},  /*  */
    {0x620,   0 * 4 / 10, 0},  /*  */
    {0x621,  + 10 * 4 / 10, 0},  /*  */
    {0x622,  + 10 * 4 / 10, 0},  /*  */
    {0x623,  + 10 * 4 / 10, 0},  /*  */
    {0x624,  + 10 * 4 / 10, 0},  /*  */
    {0x625,  + 10 * 4 / 10, 0},  /*  */
    {0x626,   0 * 4 / 10, 0},  /*  */
    {0x627,   + 10 * 4 / 10, 0},  /*  */
    {0x628,   + 10 * 4 / 10, 0},  /*  */
    {0x629,   + 10 * 4 / 10, 0},  /*  */
    {0x630,   + 20 * 4 / 10, 0},  /*  */
    {0x631,   + 10 * 4 / 10, 0},  /*  */
    {0x632,   0 * 4 / 10, 0},  /*  */
    {0x633,   + 40 * 4 / 10, 0},  /*  */
    {0x634,   + 30 * 4 / 10, 0},  /*  */
    {0x635,   + 20 * 4 / 10, 0},  /*  */
    {0x636,   + 30 * 4 / 10, 0},    /* Ethiopia */
    {0x637,   + 30 * 4 / 10, 0},    /* Somalia */
    {0x638,   + 30 * 4 / 10, 0},    /* Djibouti */
    {0x639,   + 30 * 4 / 10, 0},    /* Kenya */
    {0x640,   + 30 * 4 / 10, 0},    /* Tanzania */
    {0x641,   + 30 * 4 / 10, 0},    /* Uganda */
    {0x642,   + 20 * 4 / 10, 0},    /* Burundi */
    {0x643,   + 20 * 4 / 10, 0},    /* Mozambique */
    {0x645,   + 20 * 4 / 10, 0},    /* Zambia */
    {0x646,   + 30 * 4 / 10, 0},    /* Madagascar */
    {0x647,   0 * 4 / 10, 0},    /* not sure */
    {0x648,   + 20 * 4 / 10, 0},    /* Zimbabwe */
    {0x649,   + 10 * 4 / 10, 0},    /* Namibia */
    {0x650,   + 20 * 4 / 10, 0},    /* Malawi */
    {0x651,   + 20 * 4 / 10, 0},    /* Lesotho */
    {0x652,   + 20 * 4 / 10, 0},    /* Botswana */
    {0x653,   + 20 * 4 / 10, 0},    /* Swaziland */
    {0x654,   + 20 * 4 / 10, 0},    /* Comoros */
    {0x655,   + 20 * 4 / 10, 0},    /* South Africa */
    {0x659,   + 30 * 4 / 10, 0},    /* sudan */
    {0x702,   - 60 * 4 / 10, 0},    /* Belize */
    {0x704,   - 60 * 4 / 10, 0},    /* Guatemala */
    {0x706,   - 60 * 4 / 10, 0},    /* Salvador */
    {0x708,   - 60 * 4 / 10, 0},    /* Honduras */
    {0x710,   - 60 * 4 / 10, 0},    /* Nicaragua */
    {0x712,   - 60 * 4 / 10, 0},    /* Costa Rica */
    {0x714,   - 50 * 4 / 10, 0},    /* Panama */
    {0x716,   - 50 * 4 / 10, 0},    /* Peru */
    {0x722,   - 30 * 4 / 10, 0},    /* Argentina */
    {0x724,   - 30 * 4 / 10, 0},    /* Brazil */
    {0x730,   - 50 * 4 / 10, 0},    /* Chile */
    {0x732,   - 50 * 4 / 10, 0},    /* Colombia */
    {0x734,   - 45 * 4 / 10, 0},    /* Venezuela */
    {0x736,   - 40 * 4 / 10, 0},    /* Bolivia */
    {0x738,   - 30 * 4 / 10, 0},    /* Guyana */
    {0x740,   - 50 * 4 / 10, 0},    /* Ecuador */
    {0x744,   - 40 * 4 / 10, 0},    /* Paraguay */
    {0x746,   - 30 * 4 / 10, 0},    /* Suriname */
    {0x748,   - 30 * 4 / 10, 0},    /* Uruguay */
};
VOS_UINT32 g_TimeInfoDebug = VOS_FALSE;
extern VOS_VOID At_AdjustLocalDate(TIME_ZONE_TIME_STRU *pstUinversalTime,
     VOS_INT8 cAdjustValue, TIME_ZONE_TIME_STRU *pstLocalTime);



extern unsigned long BSP_GetSeconds();
PRIVATE_SYMBOL VOS_UINT32 OM_GetSeconds(VOS_VOID)
{
    return BSP_GetSeconds();
}

VOS_UINT32 AT_GetTimeInfoDebugFlag(VOS_VOID)
{
    return g_TimeInfoDebug;
}

VOS_VOID AT_SetTimeInfoDebugFlag(VOS_UINT32 flag)
{
    g_TimeInfoDebug = flag;
}


TAF_UINT32 At_GetTimeZonebyPlmn(VOS_UINT16 PlmnMcc, VOS_INT8 *MccZoneInfo)
{
    TAF_UINT32  usTotalMccNum;
    TAF_UINT32  i;
    TAF_UINT32    Ret = VOS_FALSE;

    usTotalMccNum = sizeof(g_mcc_zone_infoTbl) / sizeof(g_mcc_zone_infoTbl[0]);

    for (i = 0; i < usTotalMccNum; i++)
    {
        if (PlmnMcc == g_mcc_zone_infoTbl[i].MCC)
        {
            *MccZoneInfo = g_mcc_zone_infoTbl[i].Zone;
            
            Ret = VOS_TRUE;
            break;
        }
    }
    
    return Ret;
}

VOS_UINT32 At_GetLiveTime( NAS_MM_INFO_IND_STRU *pstATtime, 
            NAS_MM_INFO_IND_STRU *pstNewTime )
{
    TIME_ZONE_TIME_STRU stNewTime;
    TAF_UINT32 SecondNow = 0;
    TAF_UINT32 Seconds = 0;
    TAF_UINT32 adjustDate = 0;
    TAF_UINT32 i = 0;


    if ( ( NULL == pstATtime ) || ( NULL == pstNewTime) )
    {
        return 0;
    }

    MBB_MEM_SET(&stNewTime, 0x00, sizeof(stNewTime));/*lint !e516*/

    if (NAS_MM_INFO_IE_UTLTZ == (pstATtime->ucIeFlg & NAS_MM_INFO_IE_UTLTZ))/*全局变量中保存有时间信息 */
    {
        SecondNow = OM_GetSeconds();/*lint !e128 !e58 slice to sec*/
        AT_EVENT_REPORT_LOG_1("SecondNow", SecondNow);/*lint !e515*/
        AT_EVENT_REPORT_LOG_1("pstATtime->ulTimeSeconds", pstATtime->ulTimeSeconds);/*lint !e515*/
        if (VOS_TRUE == AT_GetTimeInfoDebugFlag())
        {
            SecondNow *= TIME_INFO_DEBUG_VAR;
            AT_EVENT_REPORT_LOG_1("SecondNow", SecondNow);/*lint !e515*/
        }
        /*如果ulTimeSeconds=0表示没有收到网络侧下发的mm信息所以不进行计数*/
        if (0 == pstATtime->ulTimeSeconds || SecondNow < pstATtime->ulTimeSeconds)/*lint !e574*/
        {
            Seconds = 0;
        }
        else
        {
            Seconds = SecondNow - pstATtime->ulTimeSeconds;
        }

        MBB_MEM_CPY(pstNewTime, pstATtime, sizeof(NAS_MM_INFO_IND_STRU) );/*lint !e516*/
        /*pstATtime->ulTimeSeconds == 0 标明当前未获取到网络侧的时间下发信息*/
        if ( 0 != pstATtime->ulTimeSeconds )
        {
            pstNewTime->ulTimeSeconds = SecondNow; /*update slice time*/
            MBB_MEM_CPY(&stNewTime, 
                                      &pstATtime->stUniversalTimeandLocalTimeZone, sizeof(stNewTime) );/*lint !e516*/
            /*lint -e63*/
            stNewTime.ucSecond += (VOS_UINT8)(Seconds % 60);  /* slice secods*/
            stNewTime.ucMinute  += (VOS_UINT8)((Seconds / 60) % 60);   /* slice mitutes */
            stNewTime.ucHour    += (VOS_UINT8)((Seconds / 3600) % 24);   /*slice hours*/
            adjustDate = (Seconds / (3600 * 24));   /*slice days*/

            if ( stNewTime.ucSecond >= 60 ) /* check second*/
            {
                stNewTime.ucMinute += stNewTime.ucSecond / 60;  /* second to minute*/
                stNewTime.ucSecond = stNewTime.ucSecond % 60;  /* left second */
            }

            if ( stNewTime.ucMinute >= 60 ) /* check minute*/
            {
                stNewTime.ucHour += stNewTime.ucMinute / 60;  /* minute to hour*/
                stNewTime.ucMinute = stNewTime.ucMinute % 60;  /* left minute*/
            }

            if ( (adjustDate > 0) || (stNewTime.ucHour >= 24) ) /* check hour*/
            {
                adjustDate += (stNewTime.ucHour / 24);  /* hour to day*/
                stNewTime.ucHour = stNewTime.ucHour % 24;  /* left hour*/
                MBB_MEM_CPY(&pstNewTime->stUniversalTimeandLocalTimeZone,
                                        &stNewTime, sizeof(stNewTime));/*lint !e516*/

                for ( i = 0; i < adjustDate; i++ )
                {
                    At_AdjustLocalDate( &stNewTime, 1, &pstNewTime->stUniversalTimeandLocalTimeZone );
                    MBB_MEM_CPY(&stNewTime, 
                                            &pstNewTime->stUniversalTimeandLocalTimeZone, sizeof(stNewTime));/*lint !e516*/
                }
            }
            else
            {
                MBB_MEM_CPY(&pstNewTime->stUniversalTimeandLocalTimeZone,
                                        &stNewTime, sizeof(stNewTime) );/*lint !e516*/
            }
            /*lint +e63*/
        }

        AT_INFO_LOG("\r\nAt_GetLocalTime: Have Date and Time Info\n");
    }

    return (VOS_UINT32)pstATtime->ucIeFlg;
}

TAF_UINT16 At_PrintNwTimeInfo(
    NAS_MM_INFO_IND_STRU    *pstMmInfo,
    TAF_UINT16              usLength,
    CONST_T VOS_CHAR       *cmd,
    CONST_T VOS_CHAR       *cmd_sep,
    CONST_T VOS_CHAR       *ending_str,
    TIME_FORMAT_ENUM_UINT32 eTimeFormat
)
{
    TAF_INT8            cTimeZone;
    TAF_UINT8          ucDST = 0;
    TAF_UINT16        old_usLength = usLength;
    NAS_MM_INFO_IND_STRU    stLocalAtTimeInfo;

    /*时间显示格式: ^TIME: "yy/mm/dd,hh:mm:ss(+/-)tz,dst" */
    if ( NAS_MM_INFO_IE_UTLTZ == (pstMmInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) )
    {
        MBB_MEM_SET(&stLocalAtTimeInfo, 0x00, sizeof(stLocalAtTimeInfo) );
        
        (VOS_VOID)At_GetLiveTime( pstMmInfo, &stLocalAtTimeInfo );
        /* "^NWTIME: */
        /* 主动上报前面有分隔符号，非主动上报不需要: */
        if ((TIME_FORMAT_RPT_TIME == eTimeFormat) || (TIME_FORMAT_RPT_NWTIME == eTimeFormat) )
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%s",
                                (VOS_CHAR *)gaucAtCrLf);
        }

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%s%s",
                                (VOS_CHAR *)cmd, cmd_sep);

        if (TIME_FORMAT_QRY_CCLK == eTimeFormat)
        {
            
            /* YYYY */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                           "%4d/",
                           stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucYear + 2000); /*year*/            
        }
        else
        {
            /* YY */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                           "%d%d/",
                           stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucYear / 10, /*year high*/
                           stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucYear % 10); /*year low*/
        }
        /* MM */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%d%d/",
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucMonth / 10,/*month high*/
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucMonth % 10);/*month low*/
        /* dd */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%d%d,",
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucDay / 10,/*day high*/
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucDay % 10);/*day high*/

        /* hh */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%d%d:",
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucHour / 10,/*hour high*/
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucHour % 10);/*hour high*/

        /* mm */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%d%d:",
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucMinute / 10,/*minutes high*/
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucMinute % 10);/*minutes high*/

        /* ss */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%d%d",
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucSecond / 10,/*sec high*/
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucSecond % 10);/*sec high*/

        /* GMT±tz, Summer(Winter) Time" */

        /* 获得时区 */
        if (NAS_MM_INFO_IE_LTZ == (stLocalAtTimeInfo.ucIeFlg & NAS_MM_INFO_IE_LTZ))
        {
            cTimeZone = stLocalAtTimeInfo.cLocalTimeZone;
            AT_EVENT_REPORT_LOG_1("cTimeZone", cTimeZone);/*lint !e515*/
        }
        else
        {
             cTimeZone = stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.cTimeZone;
             AT_EVENT_REPORT_LOG_1("cTimeZone", cTimeZone);/*lint !e515*/
        }
        if (AT_INVALID_TZ_VALUE != cTimeZone)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%s%02d",
                               (cTimeZone < 0)? "-": "+",
                               (cTimeZone < 0)? ( - cTimeZone): cTimeZone);
        }


        /* 显示夏时制或冬时制信息 */
        if ( (NAS_MM_INFO_IE_DST == (stLocalAtTimeInfo.ucIeFlg & NAS_MM_INFO_IE_DST))
          && (stLocalAtTimeInfo.ucDST > 0))
        {
            ucDST = stLocalAtTimeInfo.ucDST;
        }
        else
        {
            ucDST = 0;
        }
    if (TIME_FORMAT_QRY_CCLK != eTimeFormat)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                           ",%d%d",
                           ucDST / 10, ucDST % 10); /*dst high/low*/
    }
    
    if (NULL != ending_str)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                           "%s",
                           ending_str );/*DST,endString*/
    }
    
    /*主动上报AT后面需要添加换行结束符号*/
    if ((TIME_FORMAT_RPT_TIME == eTimeFormat) || (TIME_FORMAT_RPT_NWTIME == eTimeFormat) )
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                           "%s",
                            (VOS_CHAR *)gaucAtCrLf);
    }

    }

    return usLength - old_usLength;
}

TAF_VOID At_QryMmPlmnIdRspProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
)
{
    TAF_UINT32 ulResult = AT_FAILURE;
    TAF_UINT16 usLength = 0;
    TAF_UINT16 Mcc = 0;
    TAF_PLMN_ID_STRU     *pstPlmnId;
    VOS_INT8 tz = 0;
    NAS_MM_INFO_IND_STRU    stInvalidAtTimeInfo;
    MBB_MEM_SET(&stInvalidAtTimeInfo, 0, sizeof(NAS_MM_INFO_IND_STRU));
    /*lint -e63*/
    stInvalidAtTimeInfo.ucIeFlg = NAS_MM_INFO_IE_UTLTZ;
    stInvalidAtTimeInfo.stUniversalTimeandLocalTimeZone.ucYear = 90;
    stInvalidAtTimeInfo.stUniversalTimeandLocalTimeZone.ucMonth = 1;
    stInvalidAtTimeInfo.stUniversalTimeandLocalTimeZone.ucDay = 6;
    stInvalidAtTimeInfo.stUniversalTimeandLocalTimeZone.ucHour = 8;
    pstPlmnId = (TAF_PLMN_ID_STRU *)pPara;
    Mcc = ((pstPlmnId->Mcc & 0x0f) << 8 ) | 
                ((pstPlmnId->Mcc & 0x0f00) >> 4 ) |
                ((pstPlmnId->Mcc & 0x0f0000) >> 16 );/*Mnc*/

    (VOS_VOID)vos_printf("pstPlmnId->Mcc: 0x%x, ORI:0x%x\n", pstPlmnId->Mcc, Mcc);/*lint !e515*/
    /*  ^NWTIME:<time><tz>,<DST>     */
    if ( (TAF_UINT32)VOS_TRUE == At_GetTimeZonebyPlmn( Mcc, &tz ) )
    {
        stInvalidAtTimeInfo.stUniversalTimeandLocalTimeZone.cTimeZone = tz;
        stInvalidAtTimeInfo.cLocalTimeZone = stInvalidAtTimeInfo.stUniversalTimeandLocalTimeZone.cTimeZone;

        usLength = At_PrintNwTimeInfo( &stInvalidAtTimeInfo, usLength, 
                        (VOS_CHAR*)g_stParseContext[ucIndex].pstCmdElement->pszCmdName, ":", NULL, TIME_FORMAT_QRY_NWTIME);
        ulResult = AT_OK; 
    }
    else
    {
        ulResult = AT_ERROR;
    }
    
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);
    /*lint +e63*/

}


NAS_MM_INFO_IND_STRU* At_GetTimeInfo(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                          ulRslt = VOS_OK;
    AT_MODEM_NET_CTX_STRU               *pstNetCtx = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16             enModemId;
    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);/*lint !e64*/

    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("At_PrintMmTimeInfo: Get modem id fail.");
        return NULL;
    }

    pstNetCtx = AT_GetModemNetCtxAddrFromModemId(enModemId);
    
    return &(pstNetCtx->stTimeInfo);
}




VOS_UINT32 At_QryMmPlmnTimeZonePara(VOS_UINT8 ucIndex)
{
    if(AT_SUCCESS == Taf_ParaQuery(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   TAF_MM_PLMN_TIMEZONE_QRY_PARA,
                                   TAF_NULL_PTR))
    {
        (VOS_VOID)vos_printf("%s AT_WAIT_ASYNC_RETURN\n",__FUNCTION__ );/*lint !e515*/
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PLMN_TIMEZONE_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        (VOS_VOID)vos_printf("%s AT_ERROR\n",__FUNCTION__ );/*lint !e515*/
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryNWTimePara (VOS_UINT8 ucIndex)
{
    VOS_UINT16 usLength = 0;
    NAS_MM_INFO_IND_STRU* pstTimeInfo = At_GetTimeInfo(ucIndex);
    if (pstTimeInfo != NULL)
    {
        if ( NAS_MM_INFO_IE_UTLTZ == (pstTimeInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) )
        {
            usLength = At_PrintNwTimeInfo( pstTimeInfo, usLength,
                            (VOS_CHAR*)g_stParseContext[ucIndex].pstCmdElement->pszCmdName, ":", NULL, TIME_FORMAT_QRY_NWTIME);
            gstAtSendData.usBufLen = usLength;
            return AT_OK;
        }        
    }
    return At_QryMmPlmnTimeZonePara( ucIndex );
}
extern int do_sys_settimeofday(const struct timespec *tv, const struct timezone *tz);
extern unsigned long mktime (unsigned int year, unsigned int mon,
    unsigned int day, unsigned int hour,
    unsigned int min, unsigned int sec);

/*****************************************************************************
 函 数 名  :  AT_UpdateCclkInfo
 功能描述  : 更新CCLK信息
 输入参数  : VOS_UINT8                        *pucDateStr
            VOS_UINT32                        ulDateStrLen
 输出参数  : 无
 返 回 值  : AT_ERROR   日期有效性检查失败
             AT_OK      日期有效性检查成功

*****************************************************************************/
VOS_VOID AT_UpdateCclkInfo(TAF_MMA_TIME_CHANGE_IND_STRU *pStMmInfo)
{
    /*lint -e63*/
    gstCCLKInfo.ulTimeSeconds = pStMmInfo->ulTimeSeconds;
    gstCCLKInfo.ucIeFlg = pStMmInfo->ucIeFlg;
    MBB_MEM_CPY(&gstCCLKInfo.stUniversalTimeandLocalTimeZone,
                    &pStMmInfo->stUniversalTimeandLocalTimeZone,
                    sizeof(gstCCLKInfo.stUniversalTimeandLocalTimeZone));/*lint !e516*/
    /*lint +e63*/
}


VOS_UINT32 AT_CheckCclkDataFormat(
    VOS_UINT8                           *pucDateStr,
    VOS_UINT16                          ulDateStrLen
)
{
    VOS_UINT8                           ucFirstJuncture;
    VOS_UINT8                           ucSecondJuncture;
    VOS_UINT8                           ucFirstColon;
    VOS_UINT8                           ucSecondColon;
    VOS_UINT8                           ucComma;
    VOS_UINT8                           ucPlus;
 
    /* 格式为yyyy/mm/dd,hh:mm:ss+tz 或者yyyy/mm/dd,hh:mm:ss的日期字符串: 第5个字节为'-', 第8个字节为'-' */
    ucFirstJuncture      = *(pucDateStr + 4);
    ucSecondJuncture  = *(pucDateStr + 7);
    if(('/' != ucFirstJuncture) || ('/' != ucSecondJuncture))
    {
        return AT_ERROR;
    }
    ucComma = *(pucDateStr + 10);
    if(',' != ucComma)
    {
        return AT_ERROR;
    }
    /* 格式为yyyy/mm/dd,hh:mm:ss+tz 或者yyyy/mm/dd,hh:mm:ss的日期字符串: 第14个字节为':', 第17个字节为':' */
    ucFirstColon = *(pucDateStr + 13);
    ucSecondColon = *(pucDateStr + 16);
    if((':' != ucFirstColon) || (':' != ucSecondColon))
    {
        return AT_ERROR;
    }

    /* 格式为yyyy/mm/dd,hh:mm:ss+tz 的日期字符串: 第20个字节为'+'/'-'*/
    if(sizeof("yyyy/mm/dd,hh:mm:ss+tz") - 1 == ulDateStrLen)
    {
        ucPlus = *(pucDateStr + 19);
        if(('+' != ucPlus) && ('-' != ucPlus))
        {
            return AT_ERROR;
        }
        
    }

    return AT_OK;
}
/*****************************************************************************
 函 数 名  : AT_GetTimeZone
 功能描述  : 获取时区
 输入参数  : VOS_UINT8                           *pucDateStr
             VOS_UINT32                          ulDateStrLen
 输出参数  : AT_DATE_STRU                        *pstDate
 返 回 值  : AT_ERROR   获取时区失败
             AT_OK      获取时区成功

*****************************************************************************/
/*lint -e550*/
VOS_UINT32  AT_GetTimeZone(
    VOS_UINT8                           *pucDateStr,
    VOS_UINT32                          ulDateStrLen,
    AT_DATE_STRU                        *pstDate
)
{
    VOS_UINT32                          ulRet;
    pstDate->slTimeZone = AT_INVALID_TZ_VALUE;/*lint !e63*/
    /*检查是否有时区参数*/
    if (sizeof("yyyy/mm/dd,hh:mm:ss+tz") - 1 > ulDateStrLen)
    {
        return AT_OK;
    }
    
    ulRet = At_Auc2ul((pucDateStr + 20), 2, (TAF_UINT32*)&pstDate->slTimeZone); /*得到2位时区数字*/
    if (AT_SUCCESS != ulRet)
    {
        return AT_ERROR;
    }
        
    if ('+' == *(pucDateStr + 19))  /*判断时区符号*/
    {
        /*nothing need to do*/
    }
    else if ('-' == *(pucDateStr + 19)) /*判断时区符号*/
    {
        pstDate->slTimeZone = - pstDate->slTimeZone;/*lint !e63*/
    }
    else
    {
        return AT_ERROR;
    }
    
    return AT_OK;   
}
 /*lint +e550*/

/*****************************************************************************
 函 数 名  : At_SetCCLK
 功能描述  : AT+CCLK=<value>(用于设置时间信息)
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
/*lint -e438*/
VOS_UINT32 At_SetCCLK(VOS_UINT8 ucIndex)
{
    TAF_UINT32 ulRet = 0;
    AT_DATE_STRU stDate;
    AT_TIMESPEC tv;
    
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数过多 */
    if (gucAtParaIndex > 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /*参数长度合法*/
    if ((sizeof("yyyy/mm/dd,hh:mm:ss") - 1 != gastAtParaList[0].usParaLen)
        && (sizeof("yyyy/mm/dd,hh:mm:ss+tz") - 1 != gastAtParaList[0].usParaLen)
        )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /*lint --e{160,506,522}*/
    MBB_MEM_SET( (VOS_VOID*)&stDate, 0, sizeof(stDate) );
    /*日期有效性检查*/
    ulRet = AT_CheckCclkDataFormat((VOS_UINT8*)gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen);
    if(AT_OK != ulRet)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRet = AT_GetDate((VOS_UINT8*)gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen, &stDate);
    if (AT_OK != ulRet)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRet = AT_GetTimeZone((VOS_UINT8*)gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen, &stDate);
    if (AT_OK != ulRet)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRet = AT_CheckDate(&stDate);
    if (AT_OK != ulRet)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /*时区有效性检查 */
    if ((stDate.slTimeZone > 96 && ( AT_INVALID_TZ_VALUE != stDate.slTimeZone )) 
        || (stDate.slTimeZone < -96)) /*时区的有效范围为[-96,96]*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ( (stDate.ulYear < 2000) || (stDate.ulYear > 2100) ) /*规范要求起始时间是2000年-2100年*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    tv.tv_sec = mktime(stDate.ulYear,stDate.ulMonth,stDate.ulDay,stDate.ulHour,stDate.ulMunite,stDate.ulSecond);
    tv.tv_nsec = 0;
    ulRet = do_sys_settimeofday((const struct timespec*)(&tv),NULL);
    /*lint -e63*/
    gstCCLKInfo.ulTimeSeconds = OM_GetSeconds();
    gstCCLKInfo.ucIeFlg = NAS_MM_INFO_IE_UTLTZ;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucYear = (TAF_UINT8)(stDate.ulYear - 2000); /*以默认的2000年开始计算*/
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucMonth = (TAF_UINT8)stDate.ulMonth;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucDay = (TAF_UINT8)stDate.ulDay;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucHour = (TAF_UINT8)stDate.ulHour;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucMinute = (TAF_UINT8)stDate.ulMunite;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucSecond = (TAF_UINT8)stDate.ulSecond;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.cTimeZone = (TAF_INT8)stDate.slTimeZone;    
    /*lint +e63*/
    return AT_OK;
}
/*lint +e438*/


VOS_UINT32 At_QryCCLK(VOS_UINT8 ucIndex)
{
    VOS_UINT16 usLength = 0;
    NAS_MM_INFO_IND_STRU* pstTimeInfo = &gstCCLKInfo;

    if ( NAS_MM_INFO_IE_UTLTZ == (pstTimeInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) )
    {
        usLength = At_PrintNwTimeInfo( pstTimeInfo, usLength,
                        (VOS_CHAR*)g_stParseContext[ucIndex].pstCmdElement->pszCmdName, ": \"", "\"", TIME_FORMAT_QRY_CCLK);
        gstAtSendData.usBufLen = usLength;
        return AT_OK;
    }
    
    return AT_ERROR;
}



/*****************************************************************************
 函 数 名  : At_FormatDlckCnf
 功能描述  : 格式化dlck的at返回
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_UINT32 At_FormatDlckCnf(TAF_UINT16* usLength, TAF_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU  *pEvent)
{
    *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                (TAF_CHAR *)pgucAtSndCodeAddr,
                (TAF_CHAR *)pgucAtSndCodeAddr + *usLength,
                "%s: ",/*lint !e64 !e119 */
                g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119 */
    *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                (TAF_CHAR *)pgucAtSndCodeAddr,
                (TAF_CHAR *)pgucAtSndCodeAddr + *usLength,
                "%d,%d",/*lint !e64 !e119 */
                pEvent->MePersonalisation.unReportContent.TataLockInfo.ulDeviceLockStatusFlg,
                pEvent->MePersonalisation.unReportContent.TataLockInfo.ulDeviceUnlockFlg);/*lint !e64 !e119 */

    return AT_OK;
}/*lint !e550*/



VOS_VOID    AT_ProcSimRefreshInd(
    VOS_UINT8                           ucIndex,
    const TAF_PHONE_EVENT_INFO_STRU          *pstEvent/*lint !e49 !e601 !e10 */
)
{/*lint !e49*/
    VOS_UINT16                          usLength = 0;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;
    usLength  = 0;
    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);/*lint !e64*/

    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("AT_ProcSimRefreshInd: Get modem id fail."); /*lint !e516*/
        return;
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,
        (VOS_CHAR *)pgucAtSndCodeAddr + usLength, "%s^SIMREFRESH:%d%s",/*lint !e64 !e119 */
        gaucAtCrLf, pstEvent->RefreshType, gaucAtCrLf);/*lint !e64 !e119  !e10*/
        
    At_SendResultData(ucIndex,pgucAtSndCodeAddr,usLength);
    
    return;
}

/* 多PDP的句柄，g_ulAtUdiNdisMpdpHdl[0] 等于 g_ulAtUdiNdisHdl，是主PDP */
VOS_INT32       g_ulAtUdiNdisMpdpHdl[MAX_NDIS_NET] = {UDI_INVALID_HANDLE};

/*NV50242中设置的MPDP个数*/
VOS_UINT8          g_MpdpNum = 0;
                            
int                        gMbimFeatureFlag = FALSE;
int                        gMEcmFeatureFlag = FALSE;

PRIVATE_SYMBOL VOS_UINT8          g_ActUsbNetNum = 0;

#define IS_MBIM_OS()   (gMbimFeatureFlag)
#define IS_MECM_OS()   (gMEcmFeatureFlag)

AT_CHDATA_NDIS_RMNET_ID_STRU            g_astAtChdataNdisRmNetIdTab[] =
{
    {AT_CH_DATA_CHANNEL_ID_1, NDIS_RM_NET_ID_0},
    {AT_CH_DATA_CHANNEL_ID_2, NDIS_RM_NET_ID_1},
    {AT_CH_DATA_CHANNEL_ID_3, NDIS_RM_NET_ID_2},
    {AT_CH_DATA_CHANNEL_ID_4, NDIS_RM_NET_ID_3},
    {AT_CH_DATA_CHANNEL_ID_5, NDIS_RM_NET_ID_4},
    {AT_CH_DATA_CHANNEL_ID_6, NDIS_RM_NET_ID_5},
    {AT_CH_DATA_CHANNEL_ID_7, NDIS_RM_NET_ID_6},
    {AT_CH_DATA_CHANNEL_ID_8, NDIS_RM_NET_ID_7}
};
VOS_UINT8  g_ucPcuiPsCallFlg[TAF_MAX_CID + 1] = {0};  

extern VOS_UINT8* AT_PutNetworkAddr32(VOS_UINT8 *pucAddr, VOS_UINT32 ulAddr);
extern VOS_UINT32 AT_ActiveUsbNet(VOS_VOID);
VOS_VOID AT_ReadWinblueProfileType(VOS_VOID);

VOS_UINT8 AT_GetUsbNetNum(VOS_VOID)
{
    return g_ActUsbNetNum;
}

VOS_VOID AT_IncreaseNumWhenAct(VOS_VOID)
{
    g_ActUsbNetNum++;
}

VOS_VOID AT_DecreaseNumWhenDeact(VOS_VOID)
{
    if (g_ActUsbNetNum >= 1)
    {
        g_ActUsbNetNum--;
    }
}
/*****************************************************************************
 函 数 名  : AT_Mbb_SetPcuiCallFlag
 功能描述  : 将使用PCUI口拨号的cid进行标志
 输入参数  : ucCid 
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/

VOS_VOID AT_Mbb_SetPcuiCallFlag(VOS_UINT8 ucCid)
{
    g_ucPcuiPsCallFlg[ucCid] = VOS_TRUE;
    return;
}
/*****************************************************************************
 函 数 名  : AT_Get_User_PcuiClientid
 功能描述  : 获取PCUI口模拟拨号的clientid
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : usUserClient

*****************************************************************************/
VOS_UINT16 AT_Get_User_PcuiClientid(VOS_UINT8 ucIndex)
{
    VOS_UINT8              ucUserIndex = ucIndex;
    VOS_UINT16             usUserClient;
    if (AT_USBCOM_USER == gastAtClientTab[ucIndex].UserType)
    {   
        if (VOS_TRUE == AT_GetPcuiPsCallFlag())
        {
            ucUserIndex = (VOS_UINT8)AT_GetPcuiUsertId();
        }
    }
    usUserClient = AT_PCUI_GET_CLIENT_ID(ucUserIndex);
    return usUserClient;
}

/*****************************************************************************
 函 数 名  : AT_Get_User_PcuiClientid
 功能描述  : 获取PCUI口模拟拨号的clientid
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : usUserClient

*****************************************************************************/
VOS_UINT32 AT_PS_SetCgactState(
VOS_UINT8   ucIndex,
TAF_CID_LIST_STATE_STRU stCidListStateInfo)
{
    /*判断是否PCUI口拨号标志位是否设置，如果是，clientid为PCUI口模拟拨号的clientid，否则为当前的index*/
    if(VOS_TRUE == g_ucPcuiPsCallFlg[gastAtParaList[1].ulParaValue])
    {
        if ( VOS_OK != TAF_PS_SetPdpContextState(WUEPS_PID_AT,
                                     AT_Get_User_PcuiClientid(ucIndex),
                                     0,
                                     &stCidListStateInfo) )
        {
            return VOS_FALSE;
        }
    }
    else
    {
        /* 执行命令操作 */
        if ( VOS_OK != TAF_PS_SetPdpContextState(WUEPS_PID_AT,
                                                 gastAtClientTab[ucIndex].usClientId,
                                                 0,
                                                 &stCidListStateInfo) )
        {
            return VOS_FALSE;
        }
    }
    
    return VOS_TRUE;
}
/*****************************************************************************
 函 数 名  : AT_PS_ProcDialCmdEx
 功能描述  : 处理支持MPDP时的拨号命令，简单封装，供AT_PS_ProcDialCmdMpdp调用
 输入参数  : ucIndex 索引值
 输出参数  : ulResult : 返回结果
 返 回 值  : AT_APS_IPV6_RA_INFO_STRU* pIPv6RaInfo
 调用函数  :
 被调函数  : 
*****************************************************************************/
VOS_UINT32 AT_PS_ProcDialCmdEx(VOS_UINT32* ulResult, VOS_UINT8 ucIndex)
{
    VOS_UINT32                                                  ulReturn;
    VOS_UINT8                                                    ucCid;
    AT_CH_DATA_CHANNEL_ENUM_UINT32      enDataChannelId;
    NDIS_RM_NET_ID_ENUM_UINT8                   enNdisRmNetId;
    AT_MODEM_PS_CTX_STRU                          *pstPsModemCtx = VOS_NULL_PTR;
    ucCid = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* 检查 CID */
    if (0 == gastAtParaList[0].usParaLen)
    {
        *ulResult =  AT_CME_INCORRECT_PARAMETERS;
        return VOS_FALSE;
    }

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(ucIndex);

    /*拨号CID的PDP若已经激活，则拨号命令直接返回OK */
    if ((VOS_TRUE == pstPsModemCtx->astChannelCfg[ucCid].ulUsed)
      && (VOS_TRUE == pstPsModemCtx->astChannelCfg[ucCid].ulRmNetActFlg))
    {
        *ulResult = AT_ERROR;
        return VOS_FALSE;
    }
    if ((FALSE == IS_MBIM_OS()) && (FALSE == IS_MECM_OS()))
    {
        enDataChannelId = AT_CH_DATA_CHANNEL_ID_1;
    }
    else
    {
        enDataChannelId = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    }
    
    /* 获取网卡ID */
    ulReturn = AT_PS_GetNdisRmNetIdFromChDataValue(ucIndex, enDataChannelId, &enNdisRmNetId);
    if (VOS_OK != ulReturn)
    {
        *ulResult = AT_ERROR;
        return VOS_FALSE;
    }
    
    /* 配置数传通道映射表 */
    pstPsModemCtx->astChannelCfg[ucCid].ulUsed     = VOS_TRUE;
    pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId  = enNdisRmNetId;
    if ((FALSE == IS_MBIM_OS()) && (FALSE == IS_MECM_OS()))
    {
        pstPsModemCtx->astChannelCfg[ucCid].ulHandle   = g_ulAtUdiNdisHdl;
    }
    else
    {
        pstPsModemCtx->astChannelCfg[ucCid].ulHandle   = g_ulAtUdiNdisMpdpHdl[ucCid - 1];/*lint !e63*/
    }
    return VOS_TRUE;
}
/*****************************************************************************
 函 数 名  : AT_PS_ProcDialCmdMpdp
 功能描述  : 处理支持MPDP时的拨号命令
 输入参数  : ucIndex 索引值
 输出参数  : ulResult : 返回结果
 返 回 值  : AT_APS_IPV6_RA_INFO_STRU* pIPv6RaInfo
 调用函数  :
 被调函数  : 
*****************************************************************************/
VOS_UINT32 AT_PS_ProcDialCmdMpdp(VOS_UINT32* ulResult, VOS_UINT8 ucIndex)
{   
    if ((0 != gastAtParaList[1].ulParaValue) && (AT_NDIS_USER == gastAtClientTab[ucIndex].UserType))
    {
        return AT_PS_ProcDialCmdEx(ulResult, ucIndex);
    }
    return VOS_TRUE;
}


VOS_UINT32 AT_SendNdisIPv6PdnInfoCfgReq(
    MODEM_ID_ENUM_UINT16                 enModemId,
    TAF_PS_IPV6_INFO_IND_STRU           *pIPv6RaNotify,
    VOS_INT32                           ulHandle
)
{
    AT_NDIS_PDNINFO_CFG_REQ_STRU        stNdisCfgReq;
    VOS_UINT32                          ulSpeed;
    AT_PDP_ENTITY_STRU                 *pstNdisPdpEntity;

    /* 初始化 */
    pstNdisPdpEntity = AT_NDIS_GetPdpEntInfoAddr();
    MBB_MEM_SET(&stNdisCfgReq, 0x00, sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU));/*lint !e516 */

    stNdisCfgReq.ulHandle               = ulHandle;/*lint !e63*/

    stNdisCfgReq.bitOpIpv6PdnInfo       = VOS_TRUE;
    /* Modified   for DSDA Phase II, 2012-12-27, Begin */
    stNdisCfgReq.enModemId              = enModemId;
    /* Modified   for DSDA Phase II, 2012-12-27, End */
    stNdisCfgReq.ucRabId                = pIPv6RaNotify->ucRabId;

    /* 填充主副DNS */
    stNdisCfgReq.stIpv6PdnInfo.stDnsSer.ucSerNum    = 0;
    if (VOS_TRUE == pstNdisPdpEntity->stIpv6Dhcp.bitOpIpv6PriDns)
    {
        MBB_MEM_CPY(stNdisCfgReq.stIpv6PdnInfo.stDnsSer.aucPriServer,
                    pstNdisPdpEntity->stIpv6Dhcp.aucIpv6PrimDNS,
                    TAF_IPV6_ADDR_LEN);/*lint !e516 */
        stNdisCfgReq.stIpv6PdnInfo.stDnsSer.ucSerNum += 1;
    }

    if (VOS_TRUE == pstNdisPdpEntity->stIpv6Dhcp.bitOpIpv6SecDns)
    {
        MBB_MEM_CPY(stNdisCfgReq.stIpv6PdnInfo.stDnsSer.aucSecServer,
                    pstNdisPdpEntity->stIpv6Dhcp.aucIpv6SecDNS,
                    TAF_IPV6_ADDR_LEN);/*lint !e516 */
        stNdisCfgReq.stIpv6PdnInfo.stDnsSer.ucSerNum += 1;
    }

    /* 填充MTU */
    if (VOS_TRUE == pIPv6RaNotify->stIpv6RaInfo.bitOpMtu)
    {
        stNdisCfgReq.stIpv6PdnInfo.ulBitOpMtu   = VOS_TRUE;
        stNdisCfgReq.stIpv6PdnInfo.ulMtu        = pIPv6RaNotify->stIpv6RaInfo.ulMtu;
    }

    stNdisCfgReq.stIpv6PdnInfo.ulBitCurHopLimit = pIPv6RaNotify->stIpv6RaInfo.ulBitCurHopLimit;
    stNdisCfgReq.stIpv6PdnInfo.ulBitM           = pIPv6RaNotify->stIpv6RaInfo.ulBitM;
    stNdisCfgReq.stIpv6PdnInfo.ulBitO           = pIPv6RaNotify->stIpv6RaInfo.ulBitO;
    stNdisCfgReq.stIpv6PdnInfo.ulPrefixNum      = pIPv6RaNotify->stIpv6RaInfo.ulPrefixNum;
    MBB_MEM_CPY(stNdisCfgReq.stIpv6PdnInfo.astPrefixList,
                pIPv6RaNotify->stIpv6RaInfo.astPrefixList,
                sizeof(TAF_PDP_IPV6_PREFIX_STRU)*TAF_MAX_PREFIX_NUM_IN_RA);/*lint !e516 */

    /* 填写INTERFACE，取IPV6地址的后8字节来填写INTERFACE */
    MBB_MEM_CPY(stNdisCfgReq.stIpv6PdnInfo.aucInterfaceId,
                pstNdisPdpEntity->stIpv6Dhcp.aucIpv6Addr,
                sizeof(VOS_UINT8)*AT_NDIS_IPV6_IFID_LENGTH);/*lint !e516 */

    /* 填充主副PCSCF地址  */
    stNdisCfgReq.stIpv6PdnInfo.stPcscfSer.ucSerNum      = 0;
    if (VOS_TRUE == pstNdisPdpEntity->stIpv6Dhcp.bitOpIpv6PriPCSCF)
    {
        stNdisCfgReq.stIpv6PdnInfo.stPcscfSer.ucSerNum += 1;
        MBB_MEM_CPY(stNdisCfgReq.stIpv6PdnInfo.stPcscfSer.aucPriServer,
                    pstNdisPdpEntity->stIpv6Dhcp.aucPrimPcscfAddr,
                    sizeof(pstNdisPdpEntity->stIpv6Dhcp.aucPrimPcscfAddr));/*lint !e516 */
    }

    if (VOS_TRUE == pstNdisPdpEntity->stIpv6Dhcp.bitOpIpv6SecPCSCF)
    {
        stNdisCfgReq.stIpv6PdnInfo.stPcscfSer.ucSerNum += 1;
        MBB_MEM_CPY(stNdisCfgReq.stIpv6PdnInfo.stPcscfSer.aucSecServer,
                    pstNdisPdpEntity->stIpv6Dhcp.aucSecPcscfAddr,
                    sizeof(pstNdisPdpEntity->stIpv6Dhcp.aucSecPcscfAddr));/*lint !e516 */
    }

    /* 获取接入理论带宽*/
    if (VOS_OK != AT_GetDisplayRate(AT_CLIENT_ID_NDIS, &ulSpeed))
    {
        AT_ERR_LOG("AT_SendNdisIPv6PdnInfoCfgReq : ERROR : AT_GetDisplayRate Error!");
        ulSpeed = AT_DEF_DISPLAY_SPEED;
    }

    stNdisCfgReq.ulMaxRxbps                 = ulSpeed;
    stNdisCfgReq.ulMaxTxbps                 = ulSpeed;

    /* 发送消息 */
    if (ERR_MSP_SUCCESS != AT_FwSendClientMsg(PS_PID_APP_NDIS,
                                            ID_AT_NDIS_PDNINFO_CFG_REQ,
                                            (VOS_UINT16)sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU),
                                            (VOS_VOID*)&stNdisCfgReq))/*lint !e64*/
    {
        AT_ERR_LOG("AT_SendNdisIPv6PdnInfoCfgReq: Send client msg fail.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_SendNdisIPv4PdnInfoCfgReqEx(MN_CLIENT_ID_T usClientId,
    VOS_VOID* IPv4DhcpParam,
    VOS_UINT32   ulHandle)
{
    AT_NDIS_PDNINFO_CFG_REQ_STRU        stNdisCfgReq;
    VOS_UINT32                          ulSpeed;
    VOS_UINT32                          ulRelt;
    AT_IPV4_DHCP_PARAM_STRU* pstIPv4DhcpParam = VOS_NULL;

    if (VOS_NULL == IPv4DhcpParam)
    {
        return VOS_ERR;
    }

    pstIPv4DhcpParam = (AT_IPV4_DHCP_PARAM_STRU*)IPv4DhcpParam;
    
    /* 初始化 */
    MBB_MEM_SET(&stNdisCfgReq, 0x00, sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU));/*lint !e516 */
    stNdisCfgReq.enModemId              = MODEM_ID_0;

    ulRelt = AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)usClientId, &stNdisCfgReq.enModemId);/*lint !e64 !e10*/

    if (VOS_OK != ulRelt)
    {
        AT_ERR_LOG("AT_SendNdisIPv4PdnInfoCfgReq:Get Modem Id fail");
        return VOS_ERR;
    }

    stNdisCfgReq.bitOpIpv4PdnInfo       = VOS_TRUE;
    stNdisCfgReq.ulHandle               = ulHandle;/*lint !e63*/
    /* 构造消息 */
    if (0 != pstIPv4DhcpParam->ucRabId )
    {
        stNdisCfgReq.ucRabId = pstIPv4DhcpParam->ucRabId;
    }

    /* 填写IPv4地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4Addr)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpPdnAddr     = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stPDNAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4Addr);
    }

    /* 填写掩码地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4NetMask)
    {
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stSubnetMask.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4NetMask);
    }

    /* 填写网关地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4GateWay)
    {
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stGateWayAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4GateWay);
    }

    /* 填写主DNS地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4PrimDNS)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpDnsPrim     = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stDnsPrimAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4PrimDNS);

    }

    /* 填写辅DNS地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4SecDNS)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpDnsSec      = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stDnsSecAddrInfo.aucIpV4Addr,
                          pstIPv4DhcpParam->ulIpv4SecDNS);

    }

    /* 填写主WINS地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4PrimWINNS)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpWinsPrim    = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stWinsPrimAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4PrimWINNS);
    }

    /* 填写辅WINS地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4SecWINNS)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpWinsSec     = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stWinsSecAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4SecWINNS);
    }

    /* 填写主PCSCF地址 */
    if (VOS_TRUE == pstIPv4DhcpParam->bitOpIpv4PriPCSCF)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpPcscfPrim   = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stPcscfPrimAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4PrimPCSCF);
    }

    /* 填写副PCSCF地址 */
    if (VOS_TRUE == pstIPv4DhcpParam->bitOpIpv4Secpcscf)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpPcscfSec    = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stPcscfSecAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4SecPCSCF);
    }

    /* 获取接入理论带宽*/
    if (VOS_OK != AT_GetDisplayRate(AT_CLIENT_ID_NDIS, &ulSpeed))
    {
        AT_ERR_LOG("AT_SendNdisIPv4PdnInfoCfgReq : ERROR : AT_GetDisplayRate Error!");
        ulSpeed = AT_DEF_DISPLAY_SPEED;
    }

    stNdisCfgReq.ulMaxRxbps                 = ulSpeed;
    stNdisCfgReq.ulMaxTxbps                 = ulSpeed;

    /* 发送消息 */
    if (ERR_MSP_SUCCESS != AT_FwSendClientMsg(PS_PID_APP_NDIS,
                                            ID_AT_NDIS_PDNINFO_CFG_REQ,
                                            (VOS_UINT16)sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU),
                                            (VOS_VOID*)&stNdisCfgReq))/*lint !e64*/
    {
        AT_ERR_LOG("AT_SendNdisIPv4PdnInfoCfgReq: Send client msg fail.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_VOID AT_PS_UpdateDnsInfo(AT_DIAL_PARAM_STRU* stUsrDialParam)
{
    if(VOS_TRUE == gstAtNdisAddParam.ulPrimIPv4DNSValidFlag)
    {
        MBB_MEM_CPY(stUsrDialParam->aucPrimIPv4DNSAddr,
                    gstAtNdisAddParam.aucPrimIPv4DNSAddr,
                    TAF_MAX_IPV4_ADDR_STR_LEN);/*lint !e516 */
        stUsrDialParam->ulPrimIPv4DNSValidFlag = gstAtNdisAddParam.ulPrimIPv4DNSValidFlag;
    }
    if(VOS_TRUE == gstAtNdisAddParam.ulSndIPv4DNSValidFlag)
    {
        MBB_MEM_CPY(stUsrDialParam->aucSndIPv4DNSAddr,
                    gstAtNdisAddParam.aucSndIPv4DNSAddr,
                    TAF_MAX_IPV4_ADDR_STR_LEN);/*lint !e516 */
        stUsrDialParam->ulSndIPv4DNSValidFlag = gstAtNdisAddParam.ulSndIPv4DNSValidFlag;
    }
}

VOS_VOID AT_PS_UpdateUserDialDnsInfo(VOS_UINT8 ucIndex, VOS_VOID* stUsrDialParam)
{
    if ((AT_PS_USER_CID_1 == gastAtParaList[0].ulParaValue) && (AT_NDIS_USER == gastAtClientTab[ucIndex].UserType))
    {
        AT_PS_UpdateDnsInfo((AT_DIAL_PARAM_STRU*)stUsrDialParam);
    }
}

VOS_VOID AT_OpenUsbNdisMpdp(VOS_VOID* stParam)
{
    
    VOS_UINT32                          Index = 0;    
    UDI_OPEN_PARAM_S * pParam = VOS_NULL;/*lint !e63*/
    /*lint -e10 -e522 -e505*/
    UDI_DEVICE_ID_E ExNcmDevIdArray[MAX_NDIS_NET] = {
                                     UDI_NCM_NDIS_ID, UDI_NCM_NDIS1_ID, UDI_NCM_NDIS2_ID, UDI_NCM_NDIS3_ID, 
                                     UDI_NCM_NDIS4_ID, UDI_NCM_NDIS5_ID, UDI_NCM_NDIS6_ID, UDI_NCM_NDIS7_ID};
    /*lint +e10 +e522 +e505*/

    if (VOS_NULL == stParam)
    {
        AT_ERR_LOG("AT_OpenUsbNdisMpdp:  fail.");
        return;
    }
    pParam = (UDI_OPEN_PARAM_S*)stParam;/*lint !e63 !e10 !e26*/
    if (VOS_OK != mdrv_udi_ioctl(g_ulAtUdiNdisHdl, NCM_IOCTL_GET_MBIM_FLAG, (VOS_VOID*)&gMbimFeatureFlag))
    {
        gMbimFeatureFlag = FALSE;
    }  
    if (VOS_OK != mdrv_udi_ioctl(g_ulAtUdiNdisHdl, NCM_IOCTL_GET_MNET_FLAG, (VOS_VOID*)&gMEcmFeatureFlag))
    {
        gMEcmFeatureFlag = FALSE;
    }  

        /* g_ulAtUdiNdisMpdpHdl[0] 等于 g_ulAtUdiNdisHdl */
    g_ulAtUdiNdisMpdpHdl[0] = g_ulAtUdiNdisHdl;/*lint !e63*/

    for (Index = 1; Index <= g_MpdpNum; Index++)
    {
        pParam->devid = ExNcmDevIdArray[Index];/*lint !e63 !e10 !e409*/
        g_ulAtUdiNdisMpdpHdl[Index] = mdrv_udi_open(pParam);/*lint !e63*/

        if (UDI_INVALID_HANDLE == g_ulAtUdiNdisMpdpHdl[Index]) 
        {
            break;
        }
    }
}

VOS_VOID AT_CloseUsbNdisMpdp(VOS_VOID)
{
    VOS_UINT32  Index = 0;

    /* g_ulAtUdiNdisMpdpHdl[0] 等于 g_ulAtUdiNdisHdl */
    g_ulAtUdiNdisMpdpHdl[0] = UDI_INVALID_HANDLE; /*lint !e63*/
    for (Index = 1; Index < g_MpdpNum; Index++)
    {
 
        if ((UDI_INVALID_HANDLE != g_ulAtUdiNdisMpdpHdl[Index]) && (0 != g_ulAtUdiNdisMpdpHdl[Index]))
        {
            (VOS_VOID)mdrv_udi_close(g_ulAtUdiNdisMpdpHdl[Index]);
            g_ulAtUdiNdisMpdpHdl[Index] = UDI_INVALID_HANDLE;/*lint !e63*/
        }
    }
    return;
}

VOS_INT AT_UsbCtrlBrkReqCBMpdp(VOS_VOID)
{
    VOS_UINT8  ucCid;
    VOS_UINT8  ucCallId;

    for( ucCid = 1; ucCid <= TAF_MAX_CID; ucCid++)
    {
        /*获取ucCid对应的ucCallId,由于MPDP目前只支持ndis口拨号，所以Index使用NDIS的*/
        ucCallId = AT_PS_TransCidToCallId(AT_CLIENT_TAB_NDIS_INDEX, ucCid);
        if (VOS_FALSE == AT_PS_IsCallIdValid(AT_CLIENT_TAB_NDIS_INDEX, ucCallId))
        {
            AT_ERR_LOG("AT_PS_ProcCallEndedEvent:ERROR: CallId is invalid!");
            continue;
        }
        else
        {
            /*调用拨号断开的函数*/
            (VOS_VOID)AT_PS_HangupCall(AT_CLIENT_TAB_NDIS_INDEX, ucCallId);
            (VOS_VOID)AT_PS_HangupCall(AT_CLIENT_TAB_MODEM_INDEX, ucCallId);
        }
    }
    return 0;
} 




VOS_UINT32 AT_DeRegNdisFCPointEx(
    VOS_UINT8                           ucRabId,
    VOS_UINT16                   enModemId,
    VOS_UINT8                   enFcId
)
{
    VOS_UINT32                          ulRet;

    /* 在调用FC_DeRegPoint前,先调用FC_ChannelMapDelete */
    FC_ChannelMapDelete(ucRabId, enModemId);
    
    ulRet = FC_DeRegPoint(enFcId, enModemId);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_DeRegNdisFCPoint: ERROR: de reg point Failed.");
        return VOS_ERR;
    }

    /* 清除FCID与FC Pri的映射关系 */
    g_stFcIdMaptoFcPri[enFcId].ulUsed      = VOS_FALSE;
    g_stFcIdMaptoFcPri[enFcId].enFcPri     = FC_PRI_BUTT;
    /* 有一张网卡上多个RABID的情况，所以需要将对应的RABID掩码清除掉 */
    g_stFcIdMaptoFcPri[enFcId].ulRabIdMask &= ~((VOS_UINT32)1 << ucRabId);
    g_stFcIdMaptoFcPri[enFcId].enModemId   = MODEM_ID_BUTT;

    /* 勾流控消息 */
    AT_MNTN_TraceDeregFcPoint(AT_CLIENT_TAB_NDIS_INDEX, AT_FC_POINT_TYPE_NDIS);

    return VOS_OK;
} 


FC_ID_ENUM_UINT8 AT_PS_GetFcIdFromNdisByRmNetId(VOS_UINT32 ulRmNetId)
{
    switch (ulRmNetId)
    {
        case NDIS_RM_NET_ID_0:
            return FC_ID_NIC_1;

        case NDIS_RM_NET_ID_1:
            return FC_ID_NIC_2;

        case NDIS_RM_NET_ID_2:
            return FC_ID_NIC_3;

        case NDIS_RM_NET_ID_3:
            return FC_ID_NIC_4;

        case NDIS_RM_NET_ID_4:
            return FC_ID_NIC_5;

        case NDIS_RM_NET_ID_5:
            return FC_ID_NIC_6;
            
        case NDIS_RM_NET_ID_6:
            return FC_ID_NIC_7;

        case NDIS_RM_NET_ID_7:
            return FC_ID_NIC_8;

        default:
            AT_WARN_LOG("AT_PS_GetFcIdFromRnidRmNetId: WARNING: data channel id is abnormal.");
            return FC_ID_BUTT;
    }
}

VOS_UINT8 AT_GetDefaultFcID(VOS_UINT8 ucUserType, VOS_UINT32 ulRmNetId)
{
    FC_ID_ENUM_UINT8                   enDefaultFcId;  

    enDefaultFcId = FC_ID_NIC_1;
    /* 获取网卡ID对应的FC ID */
    if(AT_NDIS_USER == ucUserType) 
    {
        enDefaultFcId = AT_PS_GetFcIdFromNdisByRmNetId(ulRmNetId);
    }
    else if(AT_APP_USER == ucUserType)
    {
        enDefaultFcId = AT_PS_GetFcIdFromRnicByRmNetId(ulRmNetId);
    }
    return enDefaultFcId;
}

VOS_UINT32 AT_PS_GetNdisRmNetIdFromChDataValue(VOS_UINT8  ucIndex,
    AT_CH_DATA_CHANNEL_ENUM_UINT32      enDataChannelId,
    RNIC_RMNET_ID_ENUM_UINT8          *penNdisRmNetId)
{
    VOS_UINT32                          i;
    MODEM_ID_ENUM_UINT16                enModemId;
    AT_CHDATA_NDIS_RMNET_ID_STRU       *pstChdataNdisRmNetIdTab;
    VOS_UINT32                          ulRslt;
    /*获取NDIS RNNET ID的表*/
    pstChdataNdisRmNetIdTab = g_astAtChdataNdisRmNetIdTab;

    *penNdisRmNetId = NDIS_RM_NET_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);/*lint !e64*/

    if (VOS_OK != ulRslt)
    {
        return VOS_ERR;
    }

    /*信道超出范围，直接返回ERROR*/
    if ((enDataChannelId < AT_CH_DATA_CHANNEL_ID_1)
    || (enDataChannelId > AT_CH_DATA_CHANNEL_ID_7))
    {
        return VOS_ERR;
    }

    /*  以上判断已能保证enDataChannelId的有效性，所以RM NET ID一定能在表中找到 */
    for (i = 0; i < ARRAY_SIZE(g_astAtChdataNdisRmNetIdTab); i++)/*lint !e514 !e84*/
    {
        if (enDataChannelId == pstChdataNdisRmNetIdTab[i].enChdataValue)
        {
            *penNdisRmNetId = pstChdataNdisRmNetIdTab[i].enNdisRmNetId;
            break;
        }
    }

    return VOS_OK;
}


VOS_VOID AT_ReadWinblueProfileType(VOS_VOID)
{
    NAS_WINBLUE_PROFILE_TYPE_STRU stWinblueProfileType;
    VOS_UINT8  ucRslt;

/*lint -e160 -e522 -e506*/
    MBB_MEM_SET(&stWinblueProfileType, 0x00, sizeof(NAS_WINBLUE_PROFILE_TYPE_STRU));/*lint !e516 */
/*lint +e160 +e522 +e506*/
    /*目前协议栈用到NV50424的参数为ucMaxPdpSession,后续可能用到的是IPV4 MTU和IPV6 MTU*/
    ucRslt = NV_ReadEx(MODEM_ID_0, NV_ID_WINBLUE_PROFILE, &stWinblueProfileType,
                              sizeof(NAS_WINBLUE_PROFILE_TYPE_STRU));
    if (NV_OK != ucRslt)
    {
        g_MpdpNum = MAX_NDIS_NET;
        AT_WARN_LOG("NV read NV_ID_WINBLUE_PROFILE fail!");
    }
    else if (VOS_TRUE == stWinblueProfileType.MBIMEnable)
    {
        g_MpdpNum = stWinblueProfileType.MaxPDPSession;
    }
    else
    {
        g_MpdpNum = DEFAULT_MIN_NDIS_NET;
    }

    return;
}

/*****************************************************************************
 函 数 名  : AT_MbbActiveUsbNet
 功能描述  : 激活USB虚拟网卡
 输入参数  : VOS_UINT32                          ulRxbps,
             VOS_UINT32                          ulTxbps
 输出参数  : ucCid
 返 回 值  : VOS_UINT32
*****************************************************************************/
VOS_UINT32 AT_MbbActiveUsbNet(VOS_UINT8 ucCid)
{
    /*lint -e63 -e10 -e522*/
    NCM_IOCTL_CONNECTION_STATUS_S   ulLinkstus;
    VOS_INT32                       lRtn;
    NCM_IOCTL_CONNECTION_SPEED_S    stNcmConnectSpeed;
    VOS_UINT32                      ulSpeed;
    AT_IncreaseNumWhenAct();

    if (VOS_OK != AT_GetDisplayRate(AT_CLIENT_ID_NDIS, &ulSpeed))
    {
        AT_ERR_LOG("AT_ActiveUsbNet : ERROR : AT_GetDisplayRate Error!");
        ulSpeed = AT_DEF_DISPLAY_SPEED;
    }
    stNcmConnectSpeed.u32DownBitRate   = ulSpeed;
    stNcmConnectSpeed.u32UpBitRate     = ulSpeed;
    /*与usb沟通，双ECM及MPDP速率激活时需要增加cid参数*/
    stNcmConnectSpeed.u32NdisCID       = (VOS_UINT32)ucCid;

    lRtn        = mdrv_udi_ioctl(g_ulAtUdiNdisHdl, NCM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF, (VOS_VOID *)(&stNcmConnectSpeed));
    if (0 != lRtn)
    {
        AT_ERR_LOG("AT_ActiveUsbNet, Ctrl Speed Fail!" );
        return VOS_ERR;
    }

    ulLinkstus.u32ConnectionStatus = NCM_IOCTL_CONNECTION_LINKUP;
    ulLinkstus.u32NdisCID = (VOS_UINT32)ucCid;
    lRtn        = mdrv_udi_ioctl (g_ulAtUdiNdisHdl, NCM_IOCTL_NETWORK_CONNECTION_NOTIF, &ulLinkstus);
    if (0 != lRtn)
    {
        AT_ERR_LOG("AT_ActiveUsbNet, Active usb net Fail!" );
        return VOS_ERR;
    }

    return VOS_OK;
    /*lint +e63 +e10 +e522*/
}

/*****************************************************************************
 函 数 名  : AT_MbbDeActiveUsbNet
 功能描述  : 去激活USB虚拟网卡
 输入参数  : ucCid
 输出参数  : 无
 返 回 值  : VOS_UINT32
*****************************************************************************/
VOS_UINT32 AT_MbbDeActiveUsbNet(VOS_UINT8 ucCid)
{
    /*lint -e63 -e10 -e522*/
    NCM_IOCTL_CONNECTION_STATUS_S  ulLinkstus;
    VOS_INT32   lRtn;

    /*去激活，已和BSP确认，如果本来是去激活，再去激活并没有影响*/
    ulLinkstus.u32ConnectionStatus = NCM_IOCTL_CONNECTION_LINKDOWN;
    /*去激活对应的cid的网卡*/
    ulLinkstus.u32NdisCID = (VOS_UINT32)ucCid;
    AT_DecreaseNumWhenDeact();    
    if(0 == AT_GetUsbNetNum())
    {
        lRtn  = mdrv_udi_ioctl (g_ulAtUdiNdisHdl, NCM_IOCTL_NETWORK_CONNECTION_NOTIF, (VOS_VOID*)(&ulLinkstus));
        if(0 != lRtn)
        {
            AT_ERR_LOG("AT_ActiveUsbNet, Deactive usb net Fail!" );
            return VOS_ERR;
        }
    }
    return VOS_OK;
    /*lint +e63 +e10 +e522*/
}



VOS_VOID AT_PS_SndNdisPdpActInd(
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU        *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;
    VOS_UINT8                                     callId;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(pstEvent->stCtrl.usClientId);
    
    callId = AT_PS_TransCidToCallId(pstEvent->stCtrl.usClientId, ucCid);

    /*检查callId有效性*/
    if (VOS_FALSE == AT_PS_IsCallIdValid(pstEvent->stCtrl.usClientId, callId))
    {
        return;
    }

    /* 判断网卡的有效性 */
    if (pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId >= NDIS_RM_NET_ID_BUTT)
    {
        return;
    }

    /*激活网卡*/
    (VOS_VOID)AT_MbbActiveUsbNet(ucCid);

    /* 向NDIS发送PDP IPv4已经激活事件 */
    if (TAF_PDP_IPV4 == (enPdpType & TAF_PDP_IPV4))
    {
        /* 把IPV4的PDN信息发送给NDIS模块 */
        if (AT_PS_MAX_CALL_NUM <= callId  || TAF_MAX_CID < ucCid )
        {
            return ;
        }
        
        (VOS_VOID)AT_SendNdisIPv4PdnInfoCfgReqEx(pstEvent->stCtrl.usClientId,
                                     (VOS_VOID*)(&(pstPsModemCtx->astCallEntity[callId].stIpv4DhcpInfo)),
                                     pstPsModemCtx->astChannelCfg[ucCid].ulHandle);
    }

    return;

}

/*lint -e550*/
VOS_VOID AT_PS_SndNdisPdpDeactInd(
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(pstEvent->stCtrl.usClientId);

    /* 判断网卡的有效性 */
    if (pstPsModemCtx->astChannelCfg[ucCid].ulHandle == UDI_INVALID_HANDLE)
    {
        return;
    }
    /*向NDIS发送释放PDN消息*/
    (VOS_VOID)AT_SendNdisRelReq(pstEvent);

    /*去激活网卡*/
    (VOS_VOID)AT_MbbDeActiveUsbNet(ucCid);
    return;
}
/*lint +e550*/

VOS_VOID AT_PS_RegNdisFCPoint(
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU   *pstEvent
)
{
    VOS_UINT32                          ulRslt;
    AT_FCID_MAP_STRU                    stFCPriOrg;
    FC_ID_ENUM_UINT8                   enDefaultFcId;
    VOS_UINT32                          ulRmNetId;
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(pstEvent->stCtrl.usClientId);

    /* 寻找配套的通道ID */
    if ((VOS_TRUE == pstPsModemCtx->astChannelCfg[ucCid].ulUsed)
     && (pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId < NDIS_RM_NET_ID_BUTT))
    {
        ulRmNetId = pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId;
    }
    else
    {
        AT_ERR_LOG("AT_PS_RegNdisFCPoint: data channel id is abnormal.\n");
        return;
    }

    /* 上述分支已能保证网卡的有效性 */
    enDefaultFcId = AT_PS_GetFcIdFromNdisByRmNetId(ulRmNetId);

    ulRslt = AT_GetFcPriFromMap(enDefaultFcId ,&stFCPriOrg);
    if (VOS_OK == ulRslt)
    {
        /* 如果FC ID未注册，那么注册该流控点 */
        if (VOS_TRUE != stFCPriOrg.ulUsed)
        {
            /* 注册NDIS拨号使用的流控点 */
            (VOS_VOID)AT_RegNdisFCPoint(pstEvent, enDefaultFcId, MODEM_ID_0);
        }
        else
        {
            AT_NORM_LOG("AT_PS_RegNidsFCPoint: No need to change the default QOS priority.");
        }
    }
    return;
}


VOS_VOID AT_PS_DeRegNdisFCPoint(
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    VOS_UINT8                   enDefaultFcId;
    VOS_UINT32                          ulRmNetId;
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(pstEvent->stCtrl.usClientId);

    /* 寻找配套的通道ID */
    if ((VOS_TRUE == pstPsModemCtx->astChannelCfg[ucCid].ulUsed)
     && (pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId < NDIS_RM_NET_ID_BUTT))
    {
        ulRmNetId = (VOS_UINT8)pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId;
    }
    else
    {
        AT_ERR_LOG("AT_PS_DeRegAppFCPoint: data channel id is abnormal.\n");
        return;
    }

    /* 上述分支已能保证网卡的有效性 */
    enDefaultFcId = AT_PS_GetFcIdFromNdisByRmNetId(ulRmNetId);

    /* 去注册NDIS拨号使用的流控点 */
    (VOS_VOID)AT_DeRegNdisFCPointEx(pstEvent->ucRabId,MODEM_ID_0,enDefaultFcId);

    return;

}

VOS_VOID AT_PS_ReportNDISSTAT(
    VOS_UINT8                           ucCid,
    AT_PDP_STATUS_ENUM_UINT32           enStat,
    VOS_UINT8                           ucPortIndex,
    TAF_PDP_TYPE_ENUM_UINT8             ucPdpType,
    TAF_PS_CAUSE_ENUM_UINT32            enCause
)
{
    VOS_UINT16                          usLength;
    VOS_UINT8                           aucAtStrIpv4[] = "IPV4";
    VOS_UINT8                           aucAtStrIpv6[] = "IPV6";

    VOS_UINT16                          us3gppSmCause;

    usLength = 0;

    if (AT_PDP_STATUS_DEACT == enStat)
    {
        us3gppSmCause = AT_Get3gppSmCauseByPsCause(enCause);

        switch (ucPdpType)
        {
            case TAF_PDP_IPV4:
                usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",/*lint !e64 !e119 */
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv4,
                                            gaucAtCrLf);/*lint !e64 !e119 */
                break;

            case TAF_PDP_IPV6:
                usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",/*lint !e64 !e119 */
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv6,
                                            gaucAtCrLf);/*lint !e64 !e119 */
                break;

            case TAF_PDP_IPV4V6:

                usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",/*lint !e64 !e119 */
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv4,
                                            gaucAtCrLf);/*lint !e64 !e119 */

                At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

                usLength  = 0;

                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",/*lint !e64 !e119 */
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv6,
                                            gaucAtCrLf);/*lint !e64 !e119 */
                break;

            default:
                AT_ERR_LOG("AT_NdisStateChangeProc:ERROR: Wrong PDP type!");
                return;
        }
    }
    else
    {
        switch (ucPdpType)
        {
            case TAF_PDP_IPV4:
                usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\"%s",/*lint !e64 !e119 */
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv4,
                                            gaucAtCrLf);/*lint !e64 !e119 */
                break;

            case TAF_PDP_IPV6:
                usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\"%s",/*lint !e64 !e119 */
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv6,
                                            gaucAtCrLf);/*lint !e64 !e119 */
                break;

            case TAF_PDP_IPV4V6:
                usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\"%s",/*lint !e64 !e119 */
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv4,
                                            gaucAtCrLf);/*lint !e64 !e119 */

                At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

                usLength  = 0;

                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\"%s",/*lint !e64 !e119 */
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv6,
                                            gaucAtCrLf);/*lint !e64 !e119 */
                break;

            default:
                AT_ERR_LOG("AT_NdisStateChangeProc:ERROR: Wrong PDP type!");
                return;
        }
    }

    At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_UINT8 g_ucIPv6VerFlag[TAF_MAX_CID] = {AT_IPV6_SECOND_VERSION};
extern VOS_UINT32 AT_PS_ReportDhcp(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_PS_ReportDhcpv6(TAF_UINT8 ucIndex);
extern VOS_UINT32 AT_PS_ReportApraInfo(TAF_UINT8 ucIndex);
extern VOS_UINT32 TAF_AGENT_GetPdpCidPara(TAF_PDP_PRIM_CONTEXT_STRU *pstPdpPriPara,
    MN_CLIENT_ID_T usClientId, VOS_UINT8 ucCid);
extern AT_PDP_STATUS_ENUM_UINT32 AT_NdisGetConnStatus(AT_PDP_STATE_ENUM_U8 enPdpState);
/*****************************************************************************
 函 数 名  : AT_PS_ReportDefaultDhcp
 功能描述  : 默认返回第一个wan的信息
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_UINT32 AT_PS_ReportDefaultDhcp(VOS_UINT8 ucIndex)
{
    /*多WAN与原单WAN兼容处理，可以处理使用查询命令查询到第一个WAN的地址信息*/
    gucAtParaIndex = 1;
    gastAtParaList[0].ulParaValue = AT_PS_USER_CID_1;
    /* 查询指定CID的实体PDP上下文 */
    return AT_PS_ReportDhcp(ucIndex);
}
/*****************************************************************************
 函 数 名  : AT_PS_ReportDefaultDhcpV6
 功能描述  : 默认返回第一个wan的信息
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_UINT32 AT_PS_ReportDefaultDhcpV6(VOS_UINT8 ucIndex)
{
    /*多WAN与原单WAN兼容处理，可以处理使用查询命令查询到第一个WAN的地址信息*/
    gucAtParaIndex = 1;
    gastAtParaList[0].ulParaValue = AT_PS_USER_CID_1;

    /* 查询指定CID的实体PDP上下文 */
    return AT_PS_ReportDhcpv6(ucIndex);
}
/*****************************************************************************
 函 数 名  : AT_PS_ReportDefaultApraInfo
 功能描述  : 默认返回第一个wan的信息
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_UINT32 AT_PS_ReportDefaultApraInfo(TAF_UINT8 ucIndex)
{
    /*多WAN与原单WAN兼容处理，可以处理使用查询命令查询到第一个WAN的地址信息*/
    gucAtParaIndex = 1;
    gastAtParaList[0].ulParaValue = AT_PS_USER_CID_1;
    
    /* 查询指定CID的实体PDP上下文 */
    return AT_PS_ReportApraInfo(ucIndex);
}

VOS_UINT8 AT_GetIPv6VerFlag(VOS_UINT8  ucCid)
{
    return g_ucIPv6VerFlag[ucCid - 1];
}


VOS_VOID AT_SetIPv6VerFlag(VOS_UINT8  ucCid, VOS_UINT8 ucFlag )
{
    g_ucIPv6VerFlag[ucCid - 1] = ucFlag;
}


VOS_VOID AT_PS_ReportDconnNDISSTATEX(
    VOS_UINT8                           ucCid,
    VOS_UINT8                           ucPortIndex,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    VOS_UINT16                          usLength;
    VOS_UINT8                           aucAtStrIpv4[] = "IPV4";
    VOS_UINT8                           aucAtStrIpv6[] = "IPV6";
    usLength = 0;


    if((FALSE == IS_MBIM_OS()) && (FALSE == IS_MECM_OS()) && ((AT_CLIENT_TAB_NDIS_INDEX == ucPortIndex) 
        || (AT_CLIENT_TAB_PCUI_INDEX == ucPortIndex) || (AT_CLIENT_TAB_MODEM_INDEX == ucPortIndex)))
    {
        AT_PS_ReportNDISSTAT(ucCid, AT_PDP_STATUS_ACT, ucPortIndex, enPdpType, TAF_PS_CAUSE_SUCCESS);
        return;
    }
    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,1,,,\"%s\"%s",/*lint !e64 !e119 */
                                               gaucAtCrLf,
                                               ucCid,
                                               aucAtStrIpv4,
                                               gaucAtCrLf);/*lint !e64 !e119 */
            break;

        case TAF_PDP_IPV6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,1,,,\"%s\"%s",/*lint !e64 !e119 */
                                               gaucAtCrLf,
                                               ucCid,
                                               aucAtStrIpv6,
                                               gaucAtCrLf);/*lint !e64 !e119 */
            break;

        case TAF_PDP_IPV4V6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,1,,,\"%s\"%s",/*lint !e64 !e119 */
                                               gaucAtCrLf,
                                               ucCid,
                                               aucAtStrIpv4,
                                               gaucAtCrLf);/*lint !e64 !e119 */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,1,,,\"%s\"%s",/*lint !e64 !e119 */
                                               gaucAtCrLf,
                                               ucCid,
                                               aucAtStrIpv6,
                                               gaucAtCrLf);/*lint !e64 !e119 */
            break;

        default:
            AT_ERR_LOG("AT_PS_ReportDCONN: PDP type is invalid in ^NDISSTATEX.");
            return;
    }

    At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

    return;
}

VOS_VOID AT_PS_ReportDendNDISSTATEX(
    VOS_UINT8                           ucCid,
    VOS_UINT8                           ucPortIndex,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    TAF_PS_CAUSE_ENUM_UINT32            enCause
)
{
    VOS_UINT16                          usLength;
    VOS_UINT8                           aucAtStrIpv4[] = "IPV4";
    VOS_UINT8                           aucAtStrIpv6[] = "IPV6";
    VOS_UINT16                          us3gppSmCause;


    /*得到3gpp的原因值*/
    us3gppSmCause = AT_Get3gppSmCauseByPsCause(enCause);

    usLength = 0;
    if((FALSE == IS_MBIM_OS()) && (FALSE == IS_MECM_OS()) && ((AT_CLIENT_TAB_NDIS_INDEX == ucPortIndex) 
        || (AT_CLIENT_TAB_PCUI_INDEX == ucPortIndex) || (AT_CLIENT_TAB_MODEM_INDEX == ucPortIndex)))
    {
        AT_PS_ReportNDISSTAT(ucCid, AT_PDP_STATUS_DEACT, ucPortIndex, enPdpType, enCause);
        return;
    }

    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,0,%d,,\"%s\"%s",/*lint !e64 !e119 */
                                               gaucAtCrLf,
                                               ucCid,
                                               us3gppSmCause,
                                               aucAtStrIpv4,
                                               gaucAtCrLf);/*lint !e64 !e119 */

            break;
        case TAF_PDP_IPV6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,0,%d,,\"%s\"%s",/*lint !e64 !e119 */
                                               gaucAtCrLf,
                                               ucCid,
                                               us3gppSmCause,
                                               aucAtStrIpv6,
                                               gaucAtCrLf);/*lint !e64 !e119 */
            break;

        case TAF_PDP_IPV4V6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,0,%d,,\"%s\"%s",/*lint !e64 !e119 */
                                               gaucAtCrLf,
                                               ucCid,
                                               us3gppSmCause,
                                               aucAtStrIpv4,
                                               gaucAtCrLf);/*lint !e64 !e119 */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,0,%d,,\"%s\"%s",/*lint !e64 !e119 */
                                               gaucAtCrLf,
                                               ucCid,
                                               us3gppSmCause,
                                               aucAtStrIpv6,
                                               gaucAtCrLf);/*lint !e64 !e119 */
            break;

        default:
            AT_ERR_LOG("AT_PS_ReportDendNDISSTAT: PDP type is invalid in ^NDISSTATEX.");
            return;
    }
    mlog_print("at", mlog_lv_info, "Disconnect NDISSTATEX: %d, %d, %d",  ucCid, us3gppSmCause,enPdpType);/*lint !e64 !e119 */
    At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

    return;
}

VOS_UINT32 AT_QryNdisStatParaEx( VOS_UINT8 ucIndex )
{
    AT_PS_CALL_ENTITY_STRU                  *pstCallEntity;
    VOS_UINT8                               ucCallId;
    VOS_UINT8                               ucRslt;
    VOS_UINT16                              usLength = 0;

    VOS_UINT8                               aucAtStrIpv4[] = "IPV4";
    AT_PDP_STATUS_ENUM_UINT32               enIpv4Status = AT_PDP_STATUS_DEACT;
    VOS_UINT8                               aucAtStrIpv6[] = "IPV6";
    AT_PDP_STATUS_ENUM_UINT32               enIpv6Status = AT_PDP_STATUS_DEACT;

    ucCallId = AT_PS_TransCidToCallId( ucIndex, AT_PS_USER_CID_1 );
    ucRslt = AT_PS_IsCallIdValid( ucIndex, ucCallId );
    if ( VOS_TRUE == ucRslt )
    {
        pstCallEntity = AT_PS_GetCallEntity( ucIndex, ucCallId );
        enIpv4Status = AT_NdisGetConnStatus( pstCallEntity->enIpv4State );
        enIpv6Status = AT_NdisGetConnStatus( pstCallEntity->enIpv6State );
    }

    switch ( AT_GetIpv6Capability() )
    {
        case AT_IPV6_CAPABILITY_IPV4_ONLY:
            usLength  = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                "%s: ",/*lint !e64 !e119 */
                                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName );/*lint !e64 !e119 */
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                "%d,,,\"%s\"",/*lint !e64 !e119 */
                                                enIpv4Status,
                                                aucAtStrIpv4 );/*lint !e64 !e119 */
            break;

        case AT_IPV6_CAPABILITY_IPV6_ONLY:
            usLength  = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                "%s: ",/*lint !e64 !e119 */
                                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName );/*lint !e64 !e119 */
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                "%d,,,\"%s\"",/*lint !e64 !e119 */
                                                enIpv6Status,
                                                aucAtStrIpv6 );/*lint !e64 !e119 */
            break;

        case AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP:
        case AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP:
            usLength  = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                "%s: ",/*lint !e64 !e119 */
                                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName );/*lint !e64 !e119 */
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                "%d,,,\"%s\"",/*lint !e64 !e119 */
                                                enIpv4Status,
                                                aucAtStrIpv4 );/*lint !e64 !e119 */
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                ",%d,,,\"%s\"",/*lint !e64 !e119 */
                                                enIpv6Status,
                                                aucAtStrIpv6 );/*lint !e64 !e119 */
            break;

        default:
            break;
    }
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32 AT_SetNdisStatPara(VOS_UINT8 ucIndex)
{
    AT_PS_CALL_ENTITY_STRU                 *pstCallEntity;
    VOS_UINT8                               ucCallId;
    VOS_UINT16                              usLength;
    VOS_UINT8                               ucRslt;
    VOS_UINT8                               aucAtStrIpv4[] = "IPV4";
    AT_PDP_STATUS_ENUM_UINT32               enIpv4Status = AT_PDP_STATUS_DEACT;

    VOS_UINT8                               aucAtStrIpv6[] = "IPV6";
    AT_PDP_STATUS_ENUM_UINT32               enIpv6Status = AT_PDP_STATUS_DEACT;
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数错误 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    ucCallId = AT_PS_TransCidToCallId(ucIndex, (VOS_UINT8)gastAtParaList[0].ulParaValue);
    ucRslt = AT_PS_IsCallIdValid(ucIndex, ucCallId);
    if (VOS_TRUE == ucRslt)
    {
        pstCallEntity = AT_PS_GetCallEntity(ucIndex, ucCallId);
        enIpv4Status = AT_NdisGetConnStatus( pstCallEntity->enIpv4State );
        enIpv6Status = AT_NdisGetConnStatus( pstCallEntity->enIpv6State );
    }
    
    usLength = 0;
 
    switch(AT_GetIpv6Capability())
    {
        case AT_IPV6_CAPABILITY_IPV4_ONLY:
                usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",/*lint !e64 !e119 */
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119 */
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d,,,\"%s\"",/*lint !e64 !e119 */
                                       enIpv4Status,
                                       aucAtStrIpv4);/*lint !e64 !e119 */

            break;
        case AT_IPV6_CAPABILITY_IPV6_ONLY:
                usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",/*lint !e64 !e119 */
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119 */
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d,,,\"%s\"",/*lint !e64 !e119 */
                                       enIpv6Status,
                                       aucAtStrIpv6);/*lint !e64 !e119 */
            break;
        case AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP:
                usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",/*lint !e64 !e119 */
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119 */
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d,,,\"%s\"",/*lint !e64 !e119 */
                                       enIpv4Status,
                                       aucAtStrIpv4); /*lint !e64 !e119 */               
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d,,,\"%s\"",/*lint !e64 !e119 */
                                       enIpv6Status,
                                       aucAtStrIpv6);/*lint !e64 !e119 */
            break;
        case AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP:
                usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",/*lint !e64 !e119 */
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119 */
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d,,,\"%s\"",/*lint !e64 !e119 */
                                       enIpv4Status,
                                       aucAtStrIpv4);/*lint !e64 !e119 */
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       ",%d,,,\"%s\"",/*lint !e64 !e119 */
                                       enIpv6Status,
                                       aucAtStrIpv6);/*lint !e64 !e119 */
            break;
        default:
            break;
    }

    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}

/*****************************************************************************
 函 数 名  : At_TestNdisstatqry
 功能描述  : ^Ndisstatqry的测试命令，返回当前拨上号的cid
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
*****************************************************************************/
VOS_UINT32 At_TestNdisstatqry(VOS_UINT8 ucIndex)
{
    VOS_UINT16                              usLength;
    VOS_UINT8                               ucCallId;
    AT_PS_CALL_ENTITY_STRU                 *pstCallEntity;
    VOS_UINT32                              ulTmp;
    VOS_UINT8                               ucRslt;
    AT_PDP_STATUS_ENUM_UINT32               enIpv4Status = AT_PDP_STATUS_DEACT;
    AT_PDP_STATUS_ENUM_UINT32               enIpv6Status = AT_PDP_STATUS_DEACT;
    VOS_UINT32                              ulNum;
    
    usLength    = 0;
    ulNum = 0;
    /* 参数检查 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                    (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119 */
    
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                    (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s", "(");/*lint !e64 !e119 */

    for(ulTmp = 1;ulTmp <= TAF_MAX_CID; ulTmp++)
    {
    
        ucCallId = 0;
        enIpv4Status = AT_PDP_STATUS_DEACT;
        enIpv6Status = AT_PDP_STATUS_DEACT;
        
        ucCallId = AT_PS_TransCidToCallId(ucIndex, ulTmp);
        ucRslt = AT_PS_IsCallIdValid(ucIndex, ucCallId);
        if (VOS_TRUE != ucRslt)
        {
            continue;
        }

        pstCallEntity = AT_PS_GetCallEntity(ucIndex, ucCallId);
        enIpv4Status = AT_NdisGetConnStatus( pstCallEntity->enIpv4State );
        enIpv6Status = AT_NdisGetConnStatus( pstCallEntity->enIpv6State );
        if((AT_PDP_STATUS_ACT == enIpv4Status) || (AT_PDP_STATUS_ACT == enIpv6Status))
        {
        
            if (0 == ulNum )
            {   /*如果是第一个CID，则CID前不打印逗号*/
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%d",ulTmp);/*lint !e64 !e119 */
                ulNum++; 
            }
            else
            {   /*如果不是第一个CID，则CID前打印逗号*/
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",ulTmp);/*lint !e64 !e119 */
            }

        }
    }
    
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                    (TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s", ")");/*lint !e64 !e119 */
    gstAtSendData.usBufLen  = usLength;
    
    return AT_OK;
    
}


TAF_PDP_TYPE_ENUM_UINT8 AT_GetPdpTypeForNdisDialup(VOS_VOID)
{
    TAF_PDP_TYPE_ENUM_UINT8      enPdpType = TAF_PDP_IPV4;
    TAF_PDP_TYPE_ENUM_UINT8      ucIpv6Capability = AT_IPV6_CAPABILITY_IPV4_ONLY;

    ucIpv6Capability = AT_GetIpv6Capability();
    if(( AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP == ucIpv6Capability ) 
        || ( AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP == ucIpv6Capability ))
    {
        enPdpType = TAF_PDP_IPV4V6;
    }
    else if ( AT_IPV6_CAPABILITY_IPV6_ONLY == ucIpv6Capability )
    {
        enPdpType = TAF_PDP_IPV6;
    }
    else
    {
        enPdpType = TAF_PDP_IPV4;
    }

    return enPdpType;
}
extern AT_PDP_STATE_ENUM_U8 AT_PS_GetCallStateByType(VOS_UINT16 usClientId, VOS_UINT8 ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8 enPdpType);
extern VOS_UINT32  AT_PS_IsUsrDialTypeDualStack(VOS_UINT16 usClientId, VOS_UINT8 ucCallId);
extern AT_PS_USER_INFO_STRU* AT_PS_GetUserInfo(VOS_UINT16 usClientId, VOS_UINT8 ucCallId);

VOS_VOID AT_PS_ProcIpv4CallRejectEx(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent
)
{
    /* 用户发起IPv4v6类型的PDP激活, 而且被网络拒绝, 原因为28, 协议栈需要
       分别发起IPv4/IPv6类型的PDP激活, 协议栈首先发起IPv4, 再发起IPv6,
       如果IPv4类型的PDP激活再次被网络拒绝, 协议栈还需要尝试IPV6类型的
       PDP激活为了防止PDP激活嵌套, 如果IPv6类型的PDP激活失败, 将不再尝试
       IPv4类型的PDP激活 */
    AT_PDP_STATE_ENUM_U8                enPreCallState;

    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);


    enPreCallState = AT_PS_GetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4);

    /* 设置IPv4对应的CID为无效 */
    AT_PS_SetCid2CurrCall(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4, AT_PS_CALL_INVALID_CID);

    /* 将IPv4类型的PDP状态切换到IDLE */
    AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4, AT_PDP_STATE_IDLE);

    /*NDIS双栈拨号，保存当前ipv4被拒原因值*/
    if((AT_CLIENT_ID_NDIS == pstEvent->stCtrl.usClientId) && (TAF_PDP_IPV4V6 == pstCallEntity->stUsrDialParam.enPdpType))
    {
        pstCallEntity->enIpv4Cause = pstEvent->enCause;
    }
    else
    {
        /* 上报IPv4拨号失败 */

    /* 上报IPv4拨号失败 */
    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                             ucCallId,
                             TAF_PDP_IPV4,
                             pstEvent->enCause);

        /* 释放CALLID和CID的映射关系 */
        AT_PS_FreeCallIdToCid(pstEvent->stCtrl.usClientId, pstEvent->ucCid);

    }


    if ( (AT_PS_IsUsrDialTypeDualStack(pstEvent->stCtrl.usClientId, ucCallId))
      && (AT_PDP_STATE_ACTING == enPreCallState) )
    {
        if (VOS_OK == AT_PS_ProcIpv4ConnFailFallback(ucCallId, pstEvent))
        {
            return;
        }
        else
        {
            /* 记录呼叫错误码 */
            AT_PS_SetPsCallErrCause(pstEvent->stCtrl.usClientId, TAF_PS_CAUSE_UNKNOWN);
            /*NDIS双栈拨号回退ipv4被拒，发起ipv6失败时，如果实体中ipv4原因值不为0，上报ipv4失败*/
            if((AT_CLIENT_ID_NDIS == pstEvent->stCtrl.usClientId) && (TAF_PS_CAUSE_SUCCESS != pstCallEntity->enIpv4Cause))
            {
                AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                         ucCallId,
                                         TAF_PDP_IPV4,
                                         pstCallEntity->enIpv4Cause);
            }


            /* 上报IPv6拨号失败 */
            AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                     ucCallId,
                                     TAF_PDP_IPV6,
                                     AT_PS_GetPsCallErrCause(pstEvent->stCtrl.usClientId));
        }
    }

    /* 释放呼叫实体 */
    AT_PS_FreeCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    return;
}


VOS_VOID AT_PS_SndIPV4FailedResult(VOS_UINT8 ucCallId, VOS_UINT16 usClientId)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    /*NDIS双栈回退，ipv6被拒，上一次拒绝的ipv4需要上报，在此处上报ipv4失败状态*/
    if((TAF_PS_CAUSE_SUCCESS != pstCallEntity->enIpv4Cause) && (AT_CLIENT_ID_NDIS == usClientId))
    {
        AT_PS_SndCallEndedResult(usClientId,
                                ucCallId,
                                TAF_PDP_IPV4,
                                pstCallEntity->enIpv4Cause);
    }
}


VOS_UINT32 AT_PS_ValidateDialParamEx(VOS_UINT8 ucIndex)
{
    AT_PS_DATA_CHANL_CFG_STRU          *pstChanCfg = VOS_NULL_PTR;
    VOS_UINT8                           aucIpv4Addr[TAF_IPV4_ADDR_LEN];


    /* 检查命令类型 */
    if (AT_CMD_OPT_SET_CMD_NO_PARA == g_stATParseCmd.ucCmdOptType)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: No parameter input.");
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /* 检查参数个数 */
    if (gucAtParaIndex > 7)  /*拨号加上IP Addr可以多达7个参数*/
    {
        AT_NORM_LOG1("AT_PS_ValidateDialParam: Parameter number is %d.\n", gucAtParaIndex);
        return AT_TOO_MANY_PARA;
    }

    /* 检查 CID */
    if (0 == gastAtParaList[0].usParaLen)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: Missing CID.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 拨号 CONN: 该参数不能省略, 1表示建立连接, 0表示断开断开连接 */
    if (0 == gastAtParaList[1].usParaLen)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: Missing connect state.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查 APN */
    if (0 != gastAtParaList[2].usParaLen)
    {
        /* APN长度检查 */
        if (gastAtParaList[2].usParaLen > TAF_MAX_APN_LEN)
        {
            AT_NORM_LOG("AT_PS_ValidateDialParam: APN is too long.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* APN格式检查 */
        if (VOS_OK != AT_CheckApnFormat((VOS_UINT8*)gastAtParaList[2].aucPara,
                                        gastAtParaList[2].usParaLen))
        {
            AT_NORM_LOG("AT_PS_ValidateDialParam: Format of APN is wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 检查 Username */
    if (gastAtParaList[3].usParaLen > TAF_MAX_GW_AUTH_USERNAME_LEN)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查 Password */
    if (gastAtParaList[4].usParaLen > TAF_MAX_GW_AUTH_PASSWORD_LEN)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*检查IP地址*/
    if (gastAtParaList[6].usParaLen >= TAF_MAX_IPV4_ADDR_STR_LEN) /*IP地址不能大于 15*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        if (gastAtParaList[6].usParaLen > 0) /*存在IP地址*/
        {
            if (VOS_OK != AT_Ipv4AddrAtoi((VOS_CHAR *)gastAtParaList[6].aucPara, aucIpv4Addr)) /*检查IP地址格式*/
            {
                return AT_CME_INCORRECT_PARAMETERS;
            }
        }
    }

    /* 检查通道映射 */
    pstChanCfg = AT_PS_GetDataChanlCfg(ucIndex, (VOS_UINT8)gastAtParaList[0].ulParaValue);
    /*E5,hilink,stick形态产品未分配通道时，统一进行静态data channel分配，避免app/pcui拨号问题，兼容使用AT^CHDATA指定*/
    if ( ((VOS_FALSE == pstChanCfg->ulUsed) || (AT_PS_INVALID_RMNET_ID == pstChanCfg->ulRmNetId)) )
    {
        pstChanCfg->ulUsed = VOS_TRUE;
        pstChanCfg->ulRmNetId = (gastAtParaList[0].ulParaValue - TAF_MIN_CID) + RNIC_RMNET_ID_0;
    }

    return AT_SUCCESS;
}
/*****************************************************************************
 函 数 名  : AT_PS_ParseUsrDialApn
 功能描述  : 获取用户APN信息
 输入参数  : ucIndex         - 端口号(ClientId)索引
 输出参数  : pstUsrDialParam - 拨号参数信息
 返 回 值  : 无
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_VOID AT_PS_ParseUsrDialApn(VOS_UINT8 ucIndex, 
    AT_DIAL_PARAM_STRU *pstUsrDialParam)
{
    TAF_PDP_PRIM_CONTEXT_STRU                    stPdpCtxInfo;
    VOS_UINT32                          ulRslt;

    PS_MEM_SET(&stPdpCtxInfo, 0x00, sizeof(TAF_PDP_PRIM_CONTEXT_STRU));/*lint !e516 */

    ulRslt = TAF_AGENT_GetPdpCidPara(&stPdpCtxInfo, ucIndex, pstUsrDialParam->ucCid);
    if ( (VOS_OK == ulRslt) && (stPdpCtxInfo.stApn.ucLength > 0) && (2 == gucAtParaIndex))
    {
        pstUsrDialParam->ucAPNLen        = stPdpCtxInfo.stApn.ucLength;
        MBB_MEM_CPY(pstUsrDialParam->aucAPN,
               stPdpCtxInfo.stApn.aucValue,
               stPdpCtxInfo.stApn.ucLength);/*lint !e516 */
    }
    else
    {
        /* APN */
        pstUsrDialParam->ucAPNLen      = (VOS_UINT8)gastAtParaList[2].usParaLen;
        MBB_MEM_CPY(pstUsrDialParam->aucAPN,
                   gastAtParaList[2].aucPara,
                   gastAtParaList[2].usParaLen);/*lint !e516 */
    }
    return;
}

VOS_UINT32 AT_PS_ParseUsrDialParamEx(VOS_UINT8 ucIndex, 
    AT_DIAL_PARAM_STRU *pstUsrDialParam, 
    TAF_PDP_PRIM_CONTEXT_STRU* stPdpCtxInfo)
{
    VOS_UINT32                          ulRslt;
    TAF_PDP_PRIM_CONTEXT_EXT_STRU       stPdpPrimContextExt;    
    /* 设置<PDP_addr> */
    if ( gastAtParaList[6].usParaLen > 0 )  /*存在静态IP*/
    {
        /* IP地址格式已经统一在前面检查 */
        MBB_MEM_CPY(pstUsrDialParam->aucIPv4Addr, 
                    gastAtParaList[6].aucPara, TAF_MAX_IPV4_ADDR_STR_LEN);/*保存*//*lint !e516 */
        pstUsrDialParam->aucIPv4Addr[TAF_MAX_IPV4_ADDR_STR_LEN - 1]  = '\0';
        pstUsrDialParam->ulIPv4ValidFlag                        = VOS_TRUE;
    }

    if ( AT_IPV6_FIRST_VERSION == AT_GetIPv6VerFlag(pstUsrDialParam->ucCid) )
    {
        /*恢复为2.0方案，以备下一次拨号*/
        AT_SetIPv6VerFlag(pstUsrDialParam->ucCid, AT_IPV6_SECOND_VERSION );

        /*从CID对应的全局变量中获取PDP类型*/
        ulRslt = TAF_AGENT_GetPdpCidPara(stPdpCtxInfo, ucIndex, pstUsrDialParam->ucCid);

        if ( (VOS_OK == ulRslt)
          && (AT_PS_IS_PDP_TYPE_SUPPORT(stPdpCtxInfo->stPdpAddr.enPdpType)) )
        {
            pstUsrDialParam->enPdpType = stPdpCtxInfo->stPdpAddr.enPdpType;
        }
        else
        {
            pstUsrDialParam->enPdpType = TAF_PDP_IPV4;
        }

        if (VOS_OK != AT_CheckIpv6Capability(pstUsrDialParam->enPdpType))
        {
            AT_INFO_LOG("AT_PS_ParseUsrDialParam: PDP type is not supported.");
            return VOS_ERR;
        }
    }
    else
    {
        pstUsrDialParam->enPdpType= AT_GetPdpTypeForNdisDialup();
        AT_GetPdpContextFromAtDialParam(&stPdpPrimContextExt, pstUsrDialParam);
        (VOS_VOID)TAF_AGENT_SetPdpCidPara(ucIndex, &stPdpPrimContextExt);
    }
    return VOS_OK;
}


/*lint -e550 -e438*/
VOS_VOID AT_PS_HangupAllCall(
    VOS_UINT16                          usClientId
)
{
    VOS_UINT8 ucCallId;
    AT_PS_USER_INFO_STRU               *pstUserInfo = VOS_NULL_PTR;
    
    for ( ucCallId = 0; ucCallId < AT_PS_MAX_CALL_NUM; ucCallId++ )
    {
        pstUserInfo = AT_PS_GetUserInfo(usClientId, ucCallId);
                
        if (VOS_OK != AT_PS_HangupCall(pstUserInfo->enUserIndex, ucCallId))
        {
            AT_ERR_LOG("AT_PS_HangupAllCall: Hangup call failed.");
        }
    }
    
    return;
}
/*lint +e550 +e438*/

TAF_UINT32 AT_SetDsFlowQryParaEx(TAF_UINT8 ucIndex)
{

    VOS_UINT8           ucUsrCid = TAF_MAX_CID; /*默认表示查询所有的CID流量*/
    
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD == g_stATParseCmd.ucCmdOptType )
    {
        if (0 == gastAtParaList[0].usParaLen)
        {
            AT_INFO_LOG("AT_AppCheckDialParam:cid not ext");
            return AT_CME_INCORRECT_PARAMETERS;
        }
        ucUsrCid = gastAtParaList[0].aucPara[0] - '0';
    }
    else if ( AT_CMD_OPT_SET_CMD_NO_PARA != g_stATParseCmd.ucCmdOptType )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

   /*  执行命令操作  */
    if ( VOS_OK != TAF_PS_GetDsFlowInfo(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, ucUsrCid, 0))
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt  = AT_CMD_DSFLOWQRY_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}



/*****************************************************************************
 函 数 名  : AT_PS_MbbProcCallConnectedEvent
 功能描述  : 处理PS域呼叫建立事件
 输入参数  :ucIndex                    - 客户端索引
             pEvtInfo                   - 事件内容, MN_PS_EVT_STRU去除EvtId
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_VOID  AT_PS_MbbProcCallConnectedEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent)
{
    if((AT_CMD_PPP_ORG_SET != gastAtClientTab[ucIndex].CmdCurrentOpt) 
        && (AT_CMD_D_IP_CALL_SET != gastAtClientTab[ucIndex].CmdCurrentOpt))
    {
        (VOS_VOID)AT_PS_ProcCallConnectedEvent(pstEvent);
    }
    return;
}

/*****************************************************************************
 函 数 名  : AT_PS_MbbProcCallEndEvent
 功能描述  : 处理PS域呼叫建立事件
 输入参数  :ucIndex                    - 客户端索引
             pEvtInfo                   - 事件内容, MN_PS_EVT_STRU去除EvtId
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_VOID  AT_PS_MbbProcCallEndEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU  *pstEvent)
{


    if(VOS_TRUE == g_ucPcuiPsCallFlg[pstEvent->ucCid])
    {
        /*断开拨号时或者拨号失败时，删除PCUI口拨号的标志位*/
        g_ucPcuiPsCallFlg[pstEvent->ucCid] = VOS_FALSE;
        if (AT_CMD_CGACT_END_SET == gastAtClientTab[AT_CLIENT_TAB_PCUI_INDEX].CmdCurrentOpt)
        {
            /*lint -e515*/
            AT_STOP_TIMER_CMD_READY(AT_CLIENT_TAB_PCUI_INDEX);
            At_SetMode(AT_CLIENT_TAB_PCUI_INDEX, AT_CMD_MODE, AT_NORMAL_MODE);
            AT_USB_WORK_MODE_PRINT(AT_CLIENT_TAB_PCUI_INDEX, AT_CMD_MODE, AT_NORMAL_MODE);
            /*lint +e515*/
            At_FormatResultData(AT_CLIENT_TAB_PCUI_INDEX, AT_OK);
        }
    }
    if((AT_CMD_PS_DATA_CALL_END_SET != gastAtClientTab[ucIndex].CmdCurrentOpt) 
        && (AT_CMD_H_PS_SET != gastAtClientTab[ucIndex].CmdCurrentOpt))
    {
        (VOS_VOID)AT_PS_ProcCallEndedEvent(pstEvent);
    }
    return;
}
/*****************************************************************************
 函 数 名  : AT_PS_MbbProcCallEndEvent
 功能描述  : 处理PS域呼叫建立事件
 输入参数  :ucIndex                    - 客户端索引
             pEvtInfo                   - 事件内容, MN_PS_EVT_STRU去除EvtId
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_VOID  AT_PS_MbbProcDeactiveInd(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU  *pstEvent)
{
    VOS_UINT8                           ucCallId;
    ucCallId = AT_PS_TransCidToCallId(pstEvent->stCtrl.usClientId, pstEvent->ucCid);
    if (VOS_FALSE == AT_PS_IsCallIdValid(pstEvent->stCtrl.usClientId, ucCallId))
    {
        AT_ERR_LOG("AT_PS_ProcCallEndedEvent:ERROR: CallId is invalid!");
        /*在如下端口，未进行NDIS拨号，需要判断是否进行了MODEM拨号*/
        if((AT_HSUART_USER == gastAtClientTab[ucIndex].UserType)
            || (AT_USBCOM_USER == gastAtClientTab[ucIndex].UserType)
            || (AT_MODEM_USER == gastAtClientTab[ucIndex].UserType)
            || (AT_CTR_USER == gastAtClientTab[ucIndex].UserType))
        {
            (VOS_VOID)AT_ModemPsRspPdpDeactivatedEvtProc(ucIndex, pstEvent);
        }
        return;
    }
    else
    {

        (VOS_VOID)AT_PS_ProcCallEndedEvent(pstEvent);
    }
    return;
}

/*****************************************************************************
 函 数 名  : AT_PS_ModemMbbProcCallRejEvent
 功能描述  : 处理PS域呼叫失败事件
 输入参数  :ucIndex                    - 客户端索引
             pEvtInfo                   - 事件内容, MN_PS_EVT_STRU去除EvtId
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_VOID  AT_PS_ModemMbbProcCallRejEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent)
{
    if ( (AT_CMD_D_PPP_CALL_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
      && (AT_CMD_PPP_ORG_SET != gastAtClientTab[ucIndex].CmdCurrentOpt) 
      && (AT_CMD_D_IP_CALL_SET != gastAtClientTab[ucIndex].CmdCurrentOpt))
    {
        (VOS_VOID)AT_PS_ProcCallRejectEvent(pstEvent);
    }
    else
    {
        AT_ModemPsRspPdpActEvtRejProc(ucIndex, pstEvent);
    }
    return;
}

/*****************************************************************************
 函 数 名  : Is_Ipv6CapabilityValid
 功能描述  : 判断IPV6能力值
 输入参数  : ucIpv6Capability
 输出参数  : 无
 返 回 值  : FRUE:FALSE
*****************************************************************************/
VOS_UINT32 Is_Ipv6CapabilityValid(VOS_UINT8 ucIpv6Capability)
{ 
    return ((AT_IPV6_CAPABILITY_IPV4_ONLY == ucIpv6Capability)
     || (AT_IPV6_CAPABILITY_IPV6_ONLY == ucIpv6Capability)
     || (AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP == ucIpv6Capability)
     || (AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP == ucIpv6Capability))? VOS_TRUE:VOS_FALSE;
}

/*****************************************************************************
 函 数 名  : AT_PS_MbbSetupCall
 功能描述  : 建立呼叫
 输入参数  : usClientId       - 端口ID
             ucCallId         - 呼叫实体索引
             pstCallDialParam - 呼叫拨号参数
 输出参数  : 无
 返 回 值  : VOS_OK           - 成功
             VOS_ERR          - 失败
*****************************************************************************/
VOS_UINT32 AT_PS_MbbSetupCall(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    AT_DIAL_PARAM_STRU                  *pstCallDialParam
)
{
    AT_DIAL_PARAM_STRU                  stCallDialParam;
    TAF_PDP_TYPE_ENUM_UINT8             enDialPdpType;
    VOS_UINT32                          ulResult;
    PS_MEM_SET(&stCallDialParam, 0x00, sizeof(AT_DIAL_PARAM_STRU));/*lint !e516 */
    /*当nv8514的ipv6能力值为8时，先发起单v4，再发起单v6*/
    if((TAF_PDP_IPV4V6 == pstCallDialParam->enPdpType)
        && (AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP == AT_GetIpv6Capability()))
    {
        enDialPdpType = TAF_PDP_IPV4;
    }
    else
    {
        enDialPdpType = pstCallDialParam->enPdpType;
    }

    /* 填写拨号参数 */
    AT_PS_GenCallDialParam(&stCallDialParam,
                           pstCallDialParam,
                           pstCallDialParam->ucCid,
                           enDialPdpType);

    ulResult = AT_PS_SetupCall(usClientId, ucCallId, &stCallDialParam);
    return ulResult;
}




VOS_UINT32 At_CheckDlckPara(VOS_VOID)
{
    if (0 == gastAtParaList[0].usParaLen) 
    {
        return AT_FAILURE;
    }

    /*DLCK命令在修改模式下最多有三个参数*/
    if(3 < gucAtParaIndex)
    {
        return AT_FAILURE;
    }

    /*操作模式检查*/
    if(DEVICE_LOCK_MODE_MAX <= gastAtParaList[0].ulParaValue)
    {
        return AT_FAILURE;
    }

    /*密码长度和非法值检查*/
    if ((MIN_DLCK_CODE_LEN > gastAtParaList[1].usParaLen) || (MAX_DLCK_CODE_LEN < gastAtParaList[1].usParaLen))
    {
        return AT_FAILURE;
    }

    if (AT_SUCCESS != At_CheckNumString(gastAtParaList[1].aucPara,gastAtParaList[1].usParaLen))/*lint !e64*/
    {
        return AT_FAILURE;
    }

    /*在修改模式下新密码长度和非法值检查*/
    if(DEVICE_LOCK_MODE_MODIFY == gastAtParaList[0].ulParaValue)
    {
        /*检查密码长度*/
        if ((MIN_DLCK_CODE_LEN > gastAtParaList[2].usParaLen) || (MAX_DLCK_CODE_LEN < gastAtParaList[2].usParaLen))
        {
            return AT_FAILURE;
        }
        /*检查密码非法值*/
        if (AT_SUCCESS != At_CheckNumString(gastAtParaList[2].aucPara,gastAtParaList[2].usParaLen))/*lint !e64*/
        {
            return AT_FAILURE;
        }
    }
    else
    {
        /*其他模式下不允许有新密码*/
        if(0 != gastAtParaList[2].usParaLen)
        {
            return AT_FAILURE;
        }
    }

    return AT_SUCCESS;
}


VOS_UINT32  AT_SetDlckPara (VOS_UINT8 ucIndex)
{
    VOS_UINT32    ulResult = AT_SUCCESS;
    TAF_ME_PERSONALISATION_DATA_STRU    stMePersonalisationData = {0};

    /* 参数检查 */
    ulResult = At_CheckDlckPara();
    if(AT_SUCCESS != ulResult)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*填充消息*/
    stMePersonalisationData.aucmode = gastAtParaList[0].ulParaValue;

    MBB_MEM_CPY(stMePersonalisationData.aucOldPwd,
               gastAtParaList[1].aucPara,
               TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX);/*lint !e516 */
       
    if(DEVICE_LOCK_MODE_MODIFY == stMePersonalisationData.aucmode)
    {
        MBB_MEM_CPY(stMePersonalisationData.aucNewPwd,
                   gastAtParaList[2].aucPara,
                   TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX);/*lint !e516 */
    }

    /* 安全命令类型为设置密码 */
    stMePersonalisationData.CmdType        = TAF_ME_PERSONALISATION_SET;
    /* 锁卡操作为DLCK设置 */
    stMePersonalisationData.MePersonalType = TAF_OPERATOR_PERSONALISATION_SETDLCK;
     /* 执行命令操作 */
    if(AT_SUCCESS == Taf_MePersonalisationHandle(gastAtClientTab[ucIndex].usClientId, 0,&stMePersonalisationData))
    {
        /* 设置当前操作类型 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CARD_DLCK_SET;
        return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */
    }
    else
    {
        return AT_ERROR;
    }
}


TAF_UINT32 AT_QryDlckPara(TAF_UINT8 ucIndex)
{
    TAF_ME_PERSONALISATION_DATA_STRU    stMePersonalisationData = {0};

    /* 安全命令类型为查询 */
    stMePersonalisationData.CmdType     = TAF_ME_PERSONALISATION_QUERY;
    /* 锁卡操作为DLCK查询 */
    stMePersonalisationData.MePersonalType = TAF_OPERATOR_PERSONALISATION_DLCK;
    /* 执行命令操作 */
    if(AT_SUCCESS == Taf_MePersonalisationHandle(gastAtClientTab[ucIndex].usClientId, 0,&stMePersonalisationData))
    {
        /* 设置当前操作类型 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CARD_DLCK_READ;
        return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 At_CmdTestDlck( VOS_UINT8 ucIndex )
{
    return AT_OK;
}

VOS_UINT32 AT_CheckNwScanPara( TAF_MMA_SYS_CFG_PARA_STRU *pstNwScanPara )
{
    /* 判断参数个数*/
    if ( gucAtParaIndex > AT_NWSCAN_MAX_PARA_NUM )
    {
        return AT_TOO_MANY_PARA;
    }

    /* 当前参数band_switch和channel不支持 */
    /* 获取接入技术*/
    switch( gastAtParaList[0].ulParaValue )
    {
        case AT_NWSCAN_GSM:
        case AT_NWSCAN_EDGE:
        case AT_NWSCAN_WCDMA:
        {
            /* 在BTU工位上会同时测试GSM和EDGE，因此在一起设置*/
            pstNwScanPara->stRatOrder.ucRatOrderNum = AT_NWSCAN_BTU_RAT_NUM;
            pstNwScanPara->ucUtranMode = TAF_PH_IS_WCDMA_RAT;
            pstNwScanPara->stRatOrder.aenRatOrder[0] = TAF_MMA_RAT_WCDMA;
            pstNwScanPara->stRatOrder.aenRatOrder[1] = TAF_MMA_RAT_GSM;
            break;
        }
        case AT_NWSCAN_TDSCDMA:
        {
            pstNwScanPara->stRatOrder.ucRatOrderNum = AT_NWSCAN_BTT_BTL_RAT_NUM;
            pstNwScanPara->ucUtranMode = TAF_PH_IS_TDSCDMA_RAT;
            pstNwScanPara->stRatOrder.aenRatOrder[0] = TAF_MMA_RAT_WCDMA;
            break;
        }
        case AT_NWSCAN_CDMA:
        {
            /* 当前该命令不支持*/
            return AT_CME_INCORRECT_PARAMETERS;
        }
        case AT_NWSCAN_LTE_FDD:
        case AT_NWSCAN_LTE_TDD:
        {
            pstNwScanPara->stRatOrder.ucRatOrderNum = AT_NWSCAN_BTT_BTL_RAT_NUM;
            pstNwScanPara->stRatOrder.aenRatOrder[0] = TAF_MMA_RAT_LTE;
            break;
        }
        default:
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    /*参数正确是将NWSCAN标志位设置为TRUE*/
    pstNwScanPara->ucNwScanFlag = VOS_TRUE;

    return AT_OK;
}


VOS_UINT32 AT_SetNwScanPara( VOS_UINT8 ucIndex )
{
    VOS_UINT32    ulResult;
    TAF_MMA_SYS_CFG_PARA_STRU    stNwScanSetPara;
    
    /* 命令类型检查*/
    if ( AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    MBB_MEM_SET( &stNwScanSetPara, 0, sizeof(stNwScanSetPara) );/*lint !e516*/
   
    /* 参数检查*/
    ulResult = AT_CheckNwScanPara( &stNwScanSetPara );
    if ( AT_OK != ulResult )
    {
        return ulResult;
    }

    /* 发送设置消息*/
    
    /* 与MMA进行交互不成功返回失败，交互成功返回命令处理挂起状态 */
    ulResult = TAF_MMA_SetSysCfgReq(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0, &stNwScanSetPara);
   if (VOS_TRUE == ulResult)    
    {
        /* 指示当前用户的命令操作类型为设置命令*/
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SYSCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}

#define AT_RSRP_MAX          (-44)
#define AT_RSRQ_MAX          (-3)
#define AT_RSRP_MIN          (-140)
#define AT_RSRQ_MIN          (-20)

VOS_UINT32 AT_QryLteRsrp( VOS_UINT8 ucIndex )
{
    TAF_PH_INFO_RAT_TYPE ucRat = TAF_PH_INFO_NONE_RAT;
    VOS_UINT32           ulRet = VOS_OK;

     /*L4A*/
    L4A_CSQ_INFO_REQ_STRU stCsqReq  = {0};
    stCsqReq.stCtrl.ulClientId      = gastAtClientTab[ucIndex].usClientId;
    stCsqReq.stCtrl.ulOpId          = 0;
    stCsqReq.stCtrl.ulPid           = WUEPS_PID_AT;

    ucRat = AT_QryCurrSysMode(ucIndex);

    if(TAF_PH_INFO_LTE_RAT == ucRat) 
    {
       ulRet = atSendDataMsg(MSP_L4_L4A_PID, ID_MSG_L4A_LTERSRP_INFO_REQ, (VOS_VOID*)(&stCsqReq), sizeof(stCsqReq));
    
        if(ERR_MSP_SUCCESS == ulRet)
        {
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTERSRP_READ;
            return AT_WAIT_ASYNC_RETURN;
        }
        else
        {
            return AT_ERROR;
        }
    }
    else 
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_TestLteRsrpPara(VOS_UINT8 ucIndex)
{
    /* 判断单板的接入模式，根据当前注册的网络模式判断是否返回RSRP RSRQ 范围 **/
    if(TAF_PH_INFO_LTE_RAT == AT_QryCurrSysMode(ucIndex))
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       "%s:(%d,%d),(%d,%d)",/*lint !e64 !e119*/
                                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                       AT_RSRP_MIN,
                                                       AT_RSRP_MAX,
                                                       AT_RSRQ_MIN,
                                                       AT_RSRQ_MAX);/*lint !e64 !e119*/
    }
    else
    {
        return AT_ERROR;
    }
    return AT_OK;

}

VOS_VOID AT_SetHcsqFlag(VOS_BOOL falg)
{
    g_isHcsqCmd = falg;
}


VOS_BOOL AT_GetHcsqFlag(VOS_VOID)
{
    return g_isHcsqCmd;
}


VOS_UINT16 AT_FormatHcsqResult(VOS_UINT8   ucIndex, VOS_UINT16 usLength, TAF_PH_RSSI_STRU *stHcsqData)
{
    TAF_PH_HCSQ_STRU   hcsqInfo = {0};
    /* WCDMA时，通过rscp和Ecio计算rssi值时需要使用 */
    VOS_INT16   sRssi = 0;
    
    if (VOS_NULL_PTR == stHcsqData)
    {
        AT_WARN_LOG("AT_FormatHcsqResult: Invalid input stHcsqData");
        return 0;
    }
    
    switch(stHcsqData->enRatType)
    {
        case TAF_MMA_RAT_GSM:
            AT_RssiConvert(stHcsqData->aRssi[0].u.stGCellSignInfo.sRssiValue, &(hcsqInfo.ucRssiValue));
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                             "%s\"%s\",%d",
                                             gastAtStringTab[AT_STRING_HCSQ].pucText,
                                             "GSM",
                                             hcsqInfo.ucRssiValue
                                             );

            /*TS 43.022 4.4.3 A PLMN shall be understood to be received with high quality signal if the signal level is above  -85 dBm*/
            g_stSignalInfo.sRssiValue = stHcsqData->aRssi[0].u.stGCellSignInfo.sRssiValue;
            break;
        case TAF_MMA_RAT_WCDMA:
            AT_RscpConvert(stHcsqData->aRssi[0].u.stWCellSignInfo.sRscpValue, &(hcsqInfo.ucRSCP));

            if (TAF_UTRANCTRL_UTRAN_MODE_FDD == stHcsqData->ucCurrentUtranMode)
            {
                AT_EcioConvert(stHcsqData->aRssi[0].u.stWCellSignInfo.sEcioValue, &(hcsqInfo.ucEcio));
                /* rscp和Ecio值，此处直接透传，不做异常处理，具体底层保证，只处理当rscp和Ecio都为无效值时的情况*/
                if((AT_HCSQ_VALUE_INVALID <= stHcsqData->aRssi[0].u.stWCellSignInfo.sRscpValue )
                    && (AT_HCSQ_VALUE_INVALID <= stHcsqData->aRssi[0].u.stWCellSignInfo.sEcioValue))
                {
                    sRssi = AT_HCSQ_VALUE_INVALID;
                }
                else
                {
                    sRssi = stHcsqData->aRssi[0].u.stWCellSignInfo.sRscpValue - stHcsqData->aRssi[0].u.stWCellSignInfo.sEcioValue;
                }
                AT_RssiConvert(sRssi, &(hcsqInfo.ucRssiValue));
               usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                 "%s\"%s\",%d,%d,%d",
                                                 gastAtStringTab[AT_STRING_HCSQ].pucText,
                                                 "WCDMA",
                                                 hcsqInfo.ucRssiValue,
                                                 hcsqInfo.ucRSCP,
                                                 hcsqInfo.ucEcio
                                                 );

                /* TS25304  5.1.2.2 UMTS PLMN for an FDD cell, the measured primary CPICH RSCP value shall be greater than or equal to -95 dBm.*/
                g_stSignalInfo.sRscpValue = stHcsqData->aRssi[0].u.stWCellSignInfo.sRscpValue;
                g_stSignalInfo.sEcioValue = stHcsqData->aRssi[0].u.stWCellSignInfo.sEcioValue;
            }
            else
            {
               /* 非fdd 3g 小区，rssi&ecio值为无效值255 */
               usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr,
                                                 (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                 "%s\"%s\",255,%d,255",
                                                 gastAtStringTab[AT_STRING_HCSQ].pucText,
                                                 "TD-SCDMA",
                                                 hcsqInfo.ucRSCP
                                                 );
                /*TS25304  5.1.2.2, For a TDD cell, the measured P-CCPCH RSCP shall be greater than or equal to -84 dBm*/
                g_stSignalInfo.sRscpValue = stHcsqData->aRssi[0].u.stWCellSignInfo.sRscpValue;
                if(AT_HIGH_QULITY_RSCP_TDD_MIN > stHcsqData->aRssi[0].u.stWCellSignInfo.sRscpValue)
                {
                   mlog_print("at",mlog_lv_error,"rscp is %d.\n", stHcsqData->aRssi[0].u.stWCellSignInfo.sRscpValue);/*lint !e64 !e119 !e515*/
                }
            }
            break;
        case TAF_MMA_RAT_LTE:
            AT_RssiConvert(stHcsqData->aRssi[0].u.stLCellSignInfo.sRssi, &(hcsqInfo.ucRssiValue));
            AT_RsrpConvert(stHcsqData->aRssi[0].u.stLCellSignInfo.sRsrp, &(hcsqInfo.ucRSCP));
            AT_RsrqConvert(stHcsqData->aRssi[0].u.stLCellSignInfo.sRsrq, &(hcsqInfo.ucRsrq));
            AT_SinrConvert(stHcsqData->aRssi[0].u.stLCellSignInfo.lSINR, &(hcsqInfo.ucEcio));

           usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                             (VOS_CHAR *)pgucAtSndCodeAddr,
                                             (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                             "%s\"%s\",%d,%d,%d,%d",
                                             gastAtStringTab[AT_STRING_HCSQ].pucText,
                                             "LTE",
                                             hcsqInfo.ucRssiValue,
                                             hcsqInfo.ucRSCP,
                                             hcsqInfo.ucEcio,
                                             hcsqInfo.ucRsrq
                                             );
            /*TS 36304 For an E-UTRAN cell, the measured RSRP value shall be greater than or equal to -110 dBm*/
            g_stSignalInfo.sRsrpValue = stHcsqData->aRssi[0].u.stLCellSignInfo.sRsrp;
            g_stSignalInfo.sRsrqValue = stHcsqData->aRssi[0].u.stLCellSignInfo.sRsrq;
            g_stSignalInfo.lSINRValue = stHcsqData->aRssi[0].u.stLCellSignInfo.lSINR;
            if(AT_HIGH_QULITY_RSRP_MIN > g_stSignalInfo.sRsrpValue)
            {
                mlog_print("at",mlog_lv_error,"rsrp: %d, rsrq: %d sinr: %d.\n", /*lint !e64 !e119 */
                g_stSignalInfo.sRsrpValue,  g_stSignalInfo.sRsrqValue, g_stSignalInfo.lSINRValue);/*lint !e64 !e119 */
            }
            break;
        case TAF_MMA_RAT_1X:
        case TAF_MMA_RAT_HRPD:
        default :
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%s\"NOSERVICE\"",
                                              gastAtStringTab[AT_STRING_HCSQ].pucText
                                              );
            break;
    }

    return usLength;
}


VOS_UINT32 AT_RcvMmaHcsqInfoQueryCnf(VOS_VOID *pstMsg)
{
    TAF_MMA_CERSSI_INFO_QRY_CNF_STRU   *pstCerssiInfoQueryCnf = VOS_NULL_PTR;
    VOS_UINT16                          usLength = 0;
    VOS_UINT8                           ucIndex = 0;

    /* 初始化 */
    pstCerssiInfoQueryCnf   = (TAF_MMA_CERSSI_INFO_QRY_CNF_STRU*)pstMsg;

    if (VOS_NULL_PTR == pstCerssiInfoQueryCnf)
    {
        AT_WARN_LOG("AT_RcvMmaHcsqInfoQueryCnf: Invalid input pstMsg");
        return VOS_ERR;
    }

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstCerssiInfoQueryCnf->stCtrl.usClientId,&ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaHcsqInfoQueryCnf:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMmaHcsqInfoQueryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /*判断当前操作类型是否为AT_CMD_CERSSI_READ */
    if (AT_CMD_CERSSI_READ != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);/*lint !e515*/

    /* 判断查询结果是否失败,如果失败则返回ERROR */
    if (TAF_ERR_NO_ERROR != pstCerssiInfoQueryCnf->enErrorCause)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    usLength += AT_FormatHcsqResult(ucIndex, usLength, &(pstCerssiInfoQueryCnf->stCerssi));

    gstAtSendData.usBufLen = usLength;
    /* 输出结果 */
    At_FormatResultData(ucIndex, AT_OK);
    return VOS_OK;
}


VOS_BOOL AT_HdlHcsqCmdResult(VOS_UINT32 *ulResult, VOS_VOID *pstMsg)
{
    VOS_BOOL      isHcsqCmd = VOS_FALSE;

    isHcsqCmd = AT_GetHcsqFlag();

    if(VOS_TRUE == isHcsqCmd)
    {
        AT_SetHcsqFlag(FALSE);
        *ulResult = AT_RcvMmaHcsqInfoQueryCnf(pstMsg);
    }

    return isHcsqCmd;
}


VOS_VOID AT_RptHcsqChangeInfo(TAF_UINT8 ucIndex,TAF_MMA_RSSI_INFO_IND_STRU *pEvent)
{
    MODEM_ID_ENUM_UINT16   enModemId = MODEM_ID_0;
    VOS_UINT32             ulRslt = VOS_ERR;
    VOS_UINT16             usLength = 0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);/*lint !e64*/

    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("AT_RptHcsqChangeInfo: Get modem id fail.");
        return;
    }

    if((VOS_NULL_PTR == pEvent) || (TAF_MMA_RAT_BUTT == pEvent->stRssiInfo.enRatType))
    {
        return;
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                              (TAF_CHAR *)pgucAtSndCodeAddr,
                              (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                              "%s",
                              gaucAtCrLf);

    usLength = AT_FormatHcsqResult(ucIndex, usLength, &(pEvent->stRssiInfo));

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                              (TAF_CHAR *)pgucAtSndCodeAddr,
                              (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                              "%s",
                              gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr,(VOS_UINT16)usLength);
}


VOS_UINT32 AT_QryHcsqPara( VOS_UINT8 ucIndex )
{
    VOS_UINT32 ulResult = AT_ERROR;

    AT_SetHcsqFlag(TRUE);
    ulResult = At_QryCerssiPara(ucIndex);

    if(AT_ERROR == ulResult)
    {
        AT_SetHcsqFlag(FALSE);
    }
    return ulResult;
}


VOS_UINT32 AT_TestHcsqPara(VOS_UINT8 ucIndex)
{
    
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR*)pgucAtSndCodeAddr,(TAF_CHAR*)pgucAtSndCodeAddr,
                                "%s:\"NOSERVICE\",\"GSM\",\"WCDMA\",\"TD-SCDMA\",\"LTE\"",/*lint !e64 !e119*/
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119*/
    
    return (VOS_UINT32)AT_OK;
}/*lint !e550*/

AT_HFREQINFO_REPORT_TYPE      g_AtHFreqinforeport = AT_HFREQINFO_NO_REPORT;
/*****************************************************************************
 函 数 名  : At_SetHfreqinfo
 功能描述  : AT^Hfreqinfo=<value>(用于设置是否使能主动上报)
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 At_SetHFreqinfo(VOS_UINT8 ucIndex)
{
    TAF_UINT8 ucReprotMode = AT_HFREQINFO_NO_REPORT;
    
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (gucAtParaIndex > 1)
    {
        return AT_ERROR;
    }
    
    /*参数长度不为1*/
    if (1 != gastAtParaList[0].usParaLen)
    {
        return AT_ERROR;
    }

    ucReprotMode = (TAF_UINT8)(gastAtParaList[0].ulParaValue);

    /*1表示使能主动上报，0表示禁止主动上报*/
    if((AT_HFREQINFO_NO_REPORT != ucReprotMode) &&  (AT_HFREQINFO_REPORT != ucReprotMode))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    g_AtHFreqinforeport = ucReprotMode;

    return AT_OK;
}

/*****************************************************************************
 函 数 名  : At_QryHFreqinfo
 功能描述  : ^HFreqinfo查询命令处理,调用lwcrash函数接口
 输入参数  : TAF_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
TAF_UINT32 At_QryHFreqinfo(TAF_UINT8 ucIndex)
{
    TAF_UINT32 ulRat = TAF_PH_INFO_NONE_RAT;
    TAF_UINT32 ulRet = ERR_MSP_SUCCESS;
    L4A_READ_LWCLASH_REQ_STRU stLwclash = {0};

     ulRat = AT_QryCurrSysMode(ucIndex);
     
    stLwclash.stCtrl.ulClientId = gastAtClientTab[ucIndex].usClientId;
    stLwclash.stCtrl.ulOpId = 0;
    stLwclash.stCtrl.ulPid = WUEPS_PID_AT;

    if(TAF_PH_INFO_LTE_RAT != ulRat)
    {
        (VOS_VOID)vos_printf("At_QryHFreqinfo:current rat is not LTE.\n");
        return AT_ERROR;
    }
    
    ulRet = atSendDataMsg(MSP_L4_L4A_PID, ID_MSG_L4A_LWCLASHQRY_REQ, (VOS_VOID*)(&stLwclash), sizeof(L4A_READ_LWCLASH_REQ_STRU));
    if(ERR_MSP_SUCCESS == ulRet)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_HFREQINFO_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    return AT_ERROR;
}

/*****************************************************************************
 函 数 名  :  At_TestHFreqinfo
 功能描述  : ^HFREQINFO命令的测试命令
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 At_TestHFreqinfo(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s:(0,1),(%d)",/*lint !e64 !e119*/
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       AT_HFREQINFO_RAT_TYPE_LTE);/*lint !e64 !e119*/
    return AT_OK;
}/*lint !e550*/


VOS_UINT32 At_SetSmsregstPara(VOS_UINT8 ucIndex)
{
    SMS_AUTO_REG_STRU stSmsAutoReg = {0};

    /*检查定制NV是否开启*/
    if((NV_OK != NV_Read(NV_ID_SMS_AUTO_REG,&stSmsAutoReg,sizeof(SMS_AUTO_REG_STRU)))
    || (VOS_FALSE == stSmsAutoReg.ucNvActiveFlag))
    {
        return AT_ERROR;
    }

    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (1 < gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*IMSI长度检查和非法值检查*/
    if( MAX_IMSI_LEN != gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_SUCCESS != At_CheckNumString((TAF_UINT8*)(gastAtParaList[0]).aucPara,gastAtParaList[0].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*写入NV*/
    stSmsAutoReg.ucSmsRegFlag = VOS_TRUE;
    MBB_MEM_CPY(stSmsAutoReg.ucSmsRegImsi, gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen);/*lint !e516 */
    
    if(NV_OK != NV_Write(NV_ID_SMS_AUTO_REG,&stSmsAutoReg,sizeof(SMS_AUTO_REG_STRU)))
    {
        return AT_ERROR;
    }
    else
    {
        return AT_OK;
    }
}


VOS_UINT32 At_QrySmsregstPara(VOS_UINT8 ucIndex)
{
    SMS_AUTO_REG_STRU stSmsAutoReg = {0};
    VOS_UINT8 aucRegImsi[MAX_IMSI_LEN + 1] = {0};

    if((NV_OK != NV_Read(NV_ID_SMS_AUTO_REG,&stSmsAutoReg,sizeof(SMS_AUTO_REG_STRU)))
    || (VOS_FALSE == stSmsAutoReg.ucNvActiveFlag))
    {
        return AT_ERROR;
    }

    if(VOS_FALSE == stSmsAutoReg.ucSmsRegFlag)
    {
        /*lint -e516 -e160 -e506 -e522*/
        MBB_MEM_SET(aucRegImsi, '0', MAX_IMSI_LEN);
        /*lint +e516 +e160 +e506 +e522*/

        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: %s",/*lint !e64 !e119*/
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    aucRegImsi);/*lint !e64 !e119*/

        return AT_OK;
    }
    else
    {
        MBB_MEM_CPY(aucRegImsi, stSmsAutoReg.ucSmsRegImsi, MAX_IMSI_LEN);/*lint !e516 */
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: %s",/*lint !e64 !e119*/
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    aucRegImsi);/*lint !e64 !e119*/

        return AT_OK;
    }
}/*lint !e550*/


VOS_UINT32 At_TestSmsregst(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr,
                    "%s: (000000000000000-999999999999999)",/*lint !e64 !e119*/
                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119*/
    

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}/*lint !e550*/

/*****************************************************************************
 Function     : AT_MBB_GetWcdmaClass
 Description  : 
 Input        :
 Output       : 
 Return Value : VOS_UINT32
 Calls        : 
 Called By    : 
*****************************************************************************/
VOS_UINT32 AT_MBB_GetWcdmaClass(VOS_VOID)
{
    VOS_UINT32                  ulWcdmaDataclass = 0;
    VOS_UINT32                  ulHspa = 0;
    AT_NVIM_UE_CAPA_STRU        stUECapa = {0};

    MBB_MEM_SET(&stUECapa, 0x00, sizeof(AT_NVIM_UE_CAPA_STRU));/*lint !e516*/
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_WAS_RadioAccess_Capa_New, 
                            &stUECapa, sizeof(AT_NVIM_UE_CAPA_STRU)))
    {
        (VOS_VOID)vos_printf("AT_MBB_GetWcdmaClass: Read nv 9008 error");
        return ulWcdmaDataclass;
    }

    /* DC-HSPA+是否支持 */
    if ((AT_DATACLASS_ENASRELINDICATOR_R8 <= stUECapa.enAsRelIndicator)
         && (AT_DATACLASS_SUPPORT == stUECapa.enMultiCellSupport)
         && (AT_DATACLASS_SUPPORT == stUECapa.enAdjFreqMeasWithoutCmprMode)
         && (AT_DATACLASS_NOT_SUPPORT != stUECapa.ucHSDSCHPhyCategoryExt)
         && (AT_DATACLASS_NOT_SUPPORT != stUECapa.ucHSDSCHPhyCategoryExt2))
    {
         ulWcdmaDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_DC_HSPAPLUS);
    }

    /* HSPA+是否支持 */
    if ((AT_DATACLASS_ENASRELINDICATOR_R7 <= stUECapa.enAsRelIndicator)
         && (AT_DATACLASS_SUPPORT == stUECapa.enMacEhsSupport)
         && (AT_DATACLASS_NOT_SUPPORT != stUECapa.ucHSDSCHPhyCategoryExt))
    {
         ulWcdmaDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSPAPLUS);
    }

    /* DPA是否支持 */
    if (AT_DATACLASS_ENASRELINDICATOR_R5 <= stUECapa.enAsRelIndicator)
    {
        if (AT_DATACLASS_HSPASTATUS_ACTIVED == stUECapa.ulHspaStatus)
        {
            if (AT_DATACLASS_SUPPORT == stUECapa.enHSDSCHSupport)
            {
                ulWcdmaDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSDPA);
                ulHspa      |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSDPA);
            }
        }
        else
        {
            ulWcdmaDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSDPA);
            ulHspa      |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSDPA);
        }
    }

    /*UPA是否支持*/
    if (AT_DATACLASS_ENASRELINDICATOR_R6 <= stUECapa.enAsRelIndicator)
    {
        if (AT_DATACLASS_HSPASTATUS_ACTIVED == stUECapa.ulHspaStatus)
        {

            /* 这个地方与^SFEATURE的判断不一样 */
            if (AT_DATACLASS_SUPPORT == stUECapa.enEDCHSupport)
            {
                ulWcdmaDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSUPA);
                ulHspa      |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSUPA);
            }
        }
        else
        {
            ulWcdmaDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSUPA);
            ulHspa      |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSUPA);
        }
    }

    /* 如果支持 DPA和UPA，则支持HSPA */
    if (((AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSUPA)  | (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSDPA)) == ulHspa)
    {
        ulWcdmaDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSPA);
    }

    /* WCDMA设置为支持 */    
    ulWcdmaDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_WCDMA);
 
    return ulWcdmaDataclass;

}
/*****************************************************************************
 Function     : AT_QryDataclassPara
 Description  : 此函数模仿^SFEATURE查询函数。
                WCDMA、GPRS、GSM 默认支持，不做判断。
 Input        : VOS_UINT8 ucIndex
 Output       : 
 Return Value : VOS_UINT32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/1
    Author       : wenlong
    Modification : Created function
*****************************************************************************/
VOS_UINT32 AT_QryDataclassPara( VOS_UINT8 ucIndex )
{
    VOS_UINT32                  ulDataclass = 0;
    VOS_UINT32                  ulDataclassLte = 0;
    VOS_INT32                    i = 0;
    VOS_UINT16    usEgprsFlag = 0;
    VOS_UINT8     aucDataclassLteName[AT_DATACLASSLTE_MAX][AT_DATACLASS_NAME_LEN_MAX] = {"LTE"} ;
    VOS_UINT8     aucDataclassName[AT_DATACLASS_MAX + 1][AT_DATACLASS_NAME_LEN_MAX] =
                                {
                                    "GSM",
                                    "GPRS",
                                    "EDGE",
                                    "WCDMA",
                                    "HSDPA",
                                    "HSUPA",
                                    "HSPA",
                                    "HSPA+",
                                    "DC-HSPA+",
                                };

    ulDataclass = AT_MBB_GetWcdmaClass();

    /*v7平台产品可能会定制不支持GSM，做此判断*/
    if(VOS_TRUE == g_MbbIsRatSupport.ucGsmSupport)
    {
        /* EDGE是否支持 */
        if (NV_OK != NV_Read(en_NV_Item_Egprs_Flag, &usEgprsFlag,
                        sizeof(VOS_UINT16)))
        {
            return AT_ERROR;
        }
        else
        {
            if (AT_DATACLASS_NOT_SUPPORT != usEgprsFlag)
            {
                ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_EDGE);        
            }
        }
    
        /* GPRS设置为支持 */
        ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_GPRS);
    
        /* GSM设置为支持 */
        ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_GSM);
    }

    if(VOS_TRUE == g_MbbIsRatSupport.ucLteSupport)
    {   
        ulDataclassLte = AT_DATACLASS_LTE;
    }
    /* 将当前所有支持的能力等级aucDataclass与当前支持的最高等级从高位按位与，
      第一个不为0的数则是当前支持的最大能力等级，让后利用得到的i值，选出对应的字符串 */
    for(i = AT_DATACLASS_MAX; i >= 0; i--)
    {
        if (0 != ((AT_DATACLASS_BASE_VALUE << i) & ulDataclass))
        {
            break;
        }
    }
    if(i < 0)
    {
        return AT_ERROR;
    }
    
    if(AT_DATACLASS_LTE == ulDataclassLte)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               "%s:,,0x%08x,%s,,,,,%x,%s",/*lint !e64 !e119*/
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               ulDataclass,
                                               aucDataclassName[i],
                                               ulDataclassLte,
                                               aucDataclassLteName[0]);/*lint !e64 !e119*/
        }
    else
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               "%s:,,0x%08x,%s,,,,,,",/*lint !e64 !e119*/
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               ulDataclass,
                                               aucDataclassName[i]);/*lint !e64 !e119*/
    }
    return AT_OK;    
}/*lint !e550*/


PRIVATE_SYMBOL TAF_UINT32 AT_TestCsimPara( TAF_UINT8 ucIndex )
{
    if(!AT_IsCSIMCustommed())
    {
        return AT_CMD_NOT_SUPPORT;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
               (TAF_CHAR*)pgucAtSndCodeAddr, (TAF_CHAR*)pgucAtSndCodeAddr,
               "%s: %s",/*lint !e64 !e119*/
               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
               g_stParseContext[ucIndex].pstCmdElement->pszParam);/*lint !e64 !e119*/
    
    return AT_OK;
}/*lint !e550*/
static TAF_UINT8 gucBipTestClientIndex = AT_MAX_CLIENT_NUM;
extern VOS_UINT32 AT_ConvertCharToHex(VOS_UINT8 ucChar, VOS_UINT8 *pucHexValue);


VOS_UINT32 AT_Hex_Convert(VOS_UINT16 usLen, PRIVATE_CONST VOS_UINT8 *src, VOS_UINT8 *dst)
{
    VOS_UINT32 ulReslt = AT_FAILURE;
    VOS_UINT8 hexHigh = 0;
    VOS_UINT8 hexLow = 0;

    if ((NULL == src) || (NULL == dst))
    {
        return ulReslt;
    }

    while(usLen > 1)
    {
        ulReslt = AT_ConvertCharToHex(*src, &hexHigh);
        if ( AT_SUCCESS != ulReslt )
        {
            break;
        }
        ulReslt = AT_ConvertCharToHex(*(src + 1), &hexLow);
        if ( AT_SUCCESS != ulReslt )
        {
            break;
        }
        
        *dst++ = ((hexHigh & 0x0F) << 4) | (hexLow & 0x0F);
        src += 2;   /*2 ASCII to 1 Hex*/
        usLen -= 2;  /*2 ASCII to 1 Hex*/
    }
    
    return ulReslt;
}


TAF_UINT32 At_SetBipTestCmd( TAF_UINT8 ucIndex )
{
    TAF_UINT32 ulRslt = AT_ERROR;
    TAF_UINT16 usDataLen;
    TAF_UINT8 *pucData = NULL;
    VOS_UINT8 cmdType;

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)    /* 参数检查 */
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gucAtParaIndex > 1)/* 参数过多 */
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    if (gastAtParaList[0].usParaLen < 12)  /*命令最少需要12个字符长度*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    usDataLen = (gastAtParaList[0].usParaLen + 1) / 2;  /* 2 ASCII to 1 Hex, so half buffer size is enough*/

    pucData = (TAF_UINT8*)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, usDataLen);
    if (VOS_NULL_PTR == pucData)
    {
        return AT_ERROR;
    }

    ulRslt = AT_Hex_Convert(gastAtParaList[0].usParaLen,  (VOS_UINT8*)gastAtParaList[0].aucPara, pucData);
    if (AT_SUCCESS != ulRslt)
    {
        (VOS_VOID)VOS_MemFree(WUEPS_PID_AT, pucData);
        
        return AT_CME_INCORRECT_PARAMETERS;
    }

    gucBipTestClientIndex = ucIndex;
    cmdType = pucData[5];   /*get bip command type*/
    ulRslt = STUB_USIMM_SatDataInd(cmdType, usDataLen, pucData);

    ulRslt  = ( VOS_OK == ulRslt )? AT_OK : AT_ERROR;

    (VOS_VOID)VOS_MemFree(WUEPS_PID_AT, pucData);

    return ulRslt;
}


VOS_VOID AT_BipTestResultReport(VOS_VOID *data,  VOS_UINT32 u32Len)
{
    BIP_TestCmdResult_STRU  *pstBipResult = (BIP_TestCmdResult_STRU*)data;
    VOS_UINT16 usLength = 0;
    VOS_UINT8  ucIndex  = 0;
    
    if ( (NULL == data) || (u32Len > sizeof(BIP_TestCmdResult_STRU)) )
    {
        return;
    }

    if (u32Len < (sizeof(BIP_TestCmdResult_STRU) - sizeof(pstBipResult->aucData) + pstBipResult->ucDataLen))
    {
        return;
    }
    
    /*打印BIP命令码字*/
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s^BIP: %02x",/*lint !e64 !e119*/
                                       gaucAtCrLf,
                                       pstBipResult->ucCmdType);/*lint !e64 !e119*/

    /*打印结果码字*/
    for (ucIndex = 0; ucIndex < pstBipResult->ucResultCodeLen; ucIndex++)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%02x",/*lint !e64 !e119*/
                                       pstBipResult->aucResultCode[ucIndex]);/*lint !e64 !e119*/
    }
    
    /*打印附加信息*/
    for (ucIndex = 0; ucIndex < pstBipResult->ucExtendInfoLen; ucIndex++)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%04x",/*lint !e64 !e119*/
                                       pstBipResult->ausExtInfoData[ucIndex]);/*lint !e64 !e119*/
    }

    /*打印数据信息*/
    if (pstBipResult->ucDataLen > 0)
    {
        /*打印数据信息前添加逗号引号*/
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",/*lint !e64 !e119*/
                                       ",\"");/*lint !e64 !e119*/
        /*打印数据*/
        for (ucIndex = 0; ucIndex < pstBipResult->ucDataLen; ucIndex++)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%02X",/*lint !e64 !e119*/
                                           pstBipResult->aucData[ucIndex]);/*lint !e64 !e119*/
        }
        /*打印数据信息后添加引号*/
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",/*lint !e64 !e119*/
                                       "\"");/*lint !e64 !e119*/
    }
    
    /*打印结束换行符*/
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",/*lint !e64 !e119*/
                                       gaucAtCrLf);/*lint !e64 !e119*/
    
    gstAtSendData.usBufLen = usLength;
    (VOS_VOID)vos_printf("\n%s\n", pgucAtSndCodeAddr);  /*lint !e64 !e119 !e515*//*打印到串口，STICK使用后台占用PCUI口时，便于串口中调测查看*/
    
    /* 回复用户命令结果 */
    At_SendResultData(gucBipTestClientIndex, pgucAtSndCodeAddr, usLength);

    return;
}






VOS_UINT32 AT_TestChldPara(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: (0,1,1x,2,2x)",/*lint !e64 !e119*/
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119*/

    return AT_OK;
}/*lint !e550*/


VOS_VOID AT_CLCC_Report(VOS_UINT8 numType, VOS_UINT16 *usLength, VOS_UINT8 *aucAsciiNum)
{
     if (MN_MSG_TON_INTERNATIONAL == ((numType >> 4) & 0x07))
    {
        *usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + *usLength,
                                        ",\"+%s\",%d",/*lint !e64 !e119*/
                                        aucAsciiNum,
                                        (numType| AT_NUMBER_TYPE_EXT));/*lint !e64 !e119*/
    }
    else
    {
        *usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + *usLength,
                                        ",\"%s\",%d",/*lint !e64 !e119*/
                                        aucAsciiNum,
                                        (numType | AT_NUMBER_TYPE_EXT));/*lint !e64 !e119*/
    }

    return ;
}


/*lint -e438*/
VOS_BOOL At_CmdAddDoubleQuotes(VOS_UINT8** pHead, VOS_UINT16 * usCount, VOS_CHAR * pAtcmdString,
                               VOS_UINT16 uiLocation1, VOS_UINT16 uiLocation2)
{
    VOS_UINT8 *p_Temp_Data = VOS_NULL;
    VOS_CHAR  *p_Temp = VOS_NULL;
    /* 计数用的 */
    VOS_UINT16 usTemp = 0;
    /* 当前位置发现逗号标记 */
    VOS_BOOL   bool_mark = VOS_FALSE;
    /* 逗号个数计数 */
    VOS_UINT16 usCount_douhao = 0;
    if ((VOS_NULL == pAtcmdString) || (uiLocation1 >= uiLocation2)|| (0 == *usCount) || (NULL == pHead) || (NULL == *pHead))
    {
        return VOS_FALSE;
    }
    p_Temp = AT_STRSTR((VOS_CHAR *)(*pHead), pAtcmdString);/*lint !e64*/
    /* 处理的命令是需要匹配的命令 */
    if (VOS_NULL != p_Temp)
    {
        /* 检查字符串中有几个逗号，逗号个数是否大于目标逗号的位置 */
        while (usTemp < *usCount)
        {
            /* 如果逗号个数满足目标逗号的位置，没必要将所有逗号个数都统计出来，跳出循环 */
            if (uiLocation2 <= usCount_douhao)
            {
                break;
            }
            if (',' == (*pHead)[usTemp])
            {
                usCount_douhao++;
                /* 如果目标逗号之间，已经有引号括起，则返回FALSE */
                if ((uiLocation2 == usCount_douhao) && ('"' == (*pHead)[usTemp-1]))
                {
                    return VOS_FALSE;
                }
            }
            usTemp++;
        }
        /* 字符串中的逗号个数，大于需要处理的逗号位置 */
        if (uiLocation2 <= usCount_douhao)
        {
            p_Temp_Data = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT,*usCount+2);
            if (VOS_NULL == p_Temp_Data)
            {
                AT_ERR_LOG("At_CmdAddDoubleQuotes: Memory malloc failed!");
                return VOS_FALSE;
            }
            PS_MEM_SET(p_Temp_Data,0,*usCount+2);/*lint !e516 */
            PS_MEM_CPY(p_Temp_Data,*pHead,*usCount);/*lint !e516 */
            usTemp = 0;
            usCount_douhao = 0;
            /* 在指定逗号之间，添加双引号 */
            while (usTemp < *usCount)
            {
                /* 当前位置是逗号，逗号计数加1，标识位置为1 */
                if (',' == p_Temp_Data[usTemp])
                {
                    usCount_douhao++;
                    bool_mark = VOS_TRUE;
                }
                /* 前一个逗号后面，添加双引号 */
                if ((VOS_TRUE == bool_mark) && (uiLocation1 == usCount_douhao))
                {
                    p_Temp_Data[usTemp+1] = '"';
                    PS_MEM_CPY(p_Temp_Data+usTemp+2,*pHead+usTemp+1,*usCount-usTemp);/*lint !e516*/
                    bool_mark = VOS_FALSE;
                }
                /* 后一个逗号前面，添加双引号，然后跳出循环 */
                if ((VOS_TRUE == bool_mark) && (uiLocation2 == usCount_douhao))
                {
                    p_Temp_Data[usTemp] = '"';
                    PS_MEM_CPY(p_Temp_Data+usTemp+1,*pHead+usTemp-1,*usCount-usTemp+1);/*lint !e516*/
                    bool_mark = VOS_FALSE;
                    break;
                }
                usTemp++;
            }
            /* 将修改的内存赋值给pHead，源字符串长度加2，两个引号的长度，并返回TRUE */
            /* 返回TRUE后，说明申请了内存，之后的处理，需要记得FREE内存 */
            *pHead = p_Temp_Data;
            *usCount += 2;
            return VOS_TRUE;
        }
    }
    
    return VOS_FALSE;
}
/*lint +e438*/


VOS_UINT32 At_TestCgdcont_IP(VOS_UINT8 ucIndex)
{
    VOS_UINT16 usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr, (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: (0-31),\"IP\",,,(0-2),(0-3)",/*lint !e64 !e119*/
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119*/
    if (AT_IPV6_CAPABILITY_IPV4_ONLY != AT_GetIpv6Capability())
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR*)pgucAtSndCodeAddr,(TAF_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "\r\n%s: (0-31),\"IPV6\",,,(0-2),(0-3)",/*lint !e64 !e119*/
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119*/
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR*)pgucAtSndCodeAddr,(TAF_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "\r\n%s: (0-31),\"IPV4V6\",,,(0-2),(0-3)",/*lint !e64 !e119*/
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119*/
    }

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}/*lint !e550*/
PRIVATE_SYMBOL AT_QUERY_TYPE_FUNC_STRU g_aAtQryTypeProcFuncTblMbb[] =
{
    {(VOS_UINT32)TAF_MM_PLMN_TIMEZONE_QRY_PARA, At_QryMmPlmnIdRspProc},
    {TAF_TELE_PARA_BUTT,               TAF_NULL_PTR}
};

VOS_VOID At_RunQryParaRspProcCus(TAF_UINT8 ucIndex,TAF_UINT8 OpId, TAF_VOID *pPara, TAF_PARA_TYPE QueryType)
{
    VOS_UINT32 i = 0;
    for (i = 0; i != TAF_TELE_PARA_BUTT; i++ )
    {
        if (QueryType == g_aAtQryTypeProcFuncTblMbb[i].QueryType)
        {
            g_aAtQryTypeProcFuncTblMbb[i].AtQryParaProcFunc(ucIndex,OpId,pPara);
            return;
        }
    }
}

VOS_UINT32 AT_QryCurrSysMode( VOS_UINT8 ucIndex )
{
    VOS_UINT32 ReturnMode = TAF_PH_INFO_NONE_RAT;
    TAF_AGENT_SYS_MODE_STRU stSysMode = {0};
    if (VOS_OK == TAF_AGENT_GetSysMode(ucIndex, &stSysMode))
    {
        ReturnMode = stSysMode.enRatType;
    }
    else
    {
        (VOS_VOID)vos_printf("AT_QryCurrSysMode: read sysmode failed!\n");
    }
    return ReturnMode;
}

/*****************************************************************************
 函 数 名  : AT_ReadAuthFallBackInfoNV
 功能描述  : 从NV中读取鉴权回退特性是否开启，及nv中保存的鉴权类型
*****************************************************************************/
VOS_VOID  AT_ReadAuthFallBackInfoNV(VOS_VOID)
{
    VOS_UINT32                          ulResult;
    VOS_UINT32                          ulLength = 0;
    
    NV_HUAWEI_AUTHFALLBACK_FEATURE_STRU        stAuthFallback;
    
    (VOS_VOID)NV_GetLength(en_NV_Item_AUTHFALLBACK_FEATURE, &ulLength);
    
    PS_MEM_SET((VOS_VOID*)&stAuthFallback, 0x00, ulLength);

    ulResult = NV_ReadEx(MODEM_ID_0,en_NV_Item_AUTHFALLBACK_FEATURE, &stAuthFallback, ulLength);
    
    if((NV_OK == ulResult) && (VOS_TRUE == stAuthFallback.ucNvActiveFlag))
    {
        g_Nv_AuthFallbackInfo.NvAuthActFlag = VOS_TRUE;
        g_Nv_AuthFallbackInfo.NvAuthType = stAuthFallback.ucAuthType;
    }
    else
    {
        g_Nv_AuthFallbackInfo.NvAuthActFlag = VOS_FALSE;
        g_Nv_AuthFallbackInfo.NvAuthType = PPP_CHAP_AUTH_TYPE;
    }
    return;
}

/*****************************************************************************
 函 数 名  : Is_Need_AuthFallback
 功能描述  : 判断是否需要进行鉴权回退
*****************************************************************************/
VOS_UINT32 Is_Need_AuthFallback(VOS_VOID)
{ 
    return (((gastAtParaList[5].usParaLen > 0) 
            && ((PPP_NO_AUTH_TYPE == gastAtParaList[5].ulParaValue) 
            || (PPP_MS_CHAPV2_AUTH_TYPE == gastAtParaList[5].ulParaValue)))
            || ((0 == gastAtParaList[5].usParaLen ) 
             && (gastAtParaList[3].usParaLen != 0) && (gastAtParaList[4].usParaLen != 0)))? VOS_TRUE:VOS_FALSE;
}
/*****************************************************************************
 函 数 名  : AT_PS_GetUsrAuthParam
 功能描述  : 获取用户拨号参数信息
*****************************************************************************/
VOS_UINT32 AT_PS_GetUsrAuthParam(
    VOS_UINT8                           ucIndex,
    VOS_UINT16                          *usAuthType
)
{
    NV_HUAWEI_CUSTOMAUTH_FEATURE_STRU          stDocomoAuthInfo;
    VOS_UINT32                          ulLength = 0;
    VOS_UINT32                          ulDcmResult;
    
    PS_MEM_SET((void *)&g_AuthFallbackInfo, 0x00, sizeof(g_AuthFallbackInfo));
/*docomo定制特性nv开启，并且拨号用户名为空，鉴权方式为不鉴权*/
    (VOS_VOID)NV_GetLength(en_NV_Item_CUSTOMAUTH_FEATURE, &ulLength);
    ulDcmResult = NV_Read(en_NV_Item_CUSTOMAUTH_FEATURE,
                         &stDocomoAuthInfo,
                         ulLength);

    if((NV_OK == ulDcmResult) && (VOS_TRUE == stDocomoAuthInfo.ucNvActiveFlag) 
        && (VOS_TRUE == stDocomoAuthInfo.ucNvDocomoActiveFlag)
        && (0 == gastAtParaList[3].usParaLen))
    {
        g_DocomoAuthActFlag = VOS_TRUE;
    }
    /*鉴权回退特性nv开启，从nv中读取鉴权类型*/
    if((VOS_TRUE == g_Nv_AuthFallbackInfo.NvAuthActFlag) && (VOS_FALSE == g_DocomoAuthActFlag))
    {   
    /*在以下两种情况时需要进行鉴权回退:
    1. 下发鉴权类型，并且鉴权类型为0或者3
    2. 不下发鉴权类型，存在用户名和密码*/
        if (VOS_TRUE == Is_Need_AuthFallback())
        {
            *usAuthType = g_Nv_AuthFallbackInfo.NvAuthType;     //使用nv中的鉴权类型发起拨号
            g_AuthFallbackInfo.AuthNum++; //拨号次数+1
            g_AuthFallbackInfo.AuthType = g_Nv_AuthFallbackInfo.NvAuthType;   //保存首次拨号的鉴权类型
            g_AuthFallbackInfo.AuthFallbackFlag = VOS_TRUE;  //需要进行鉴权回退
        }
        else if(gastAtParaList[5].usParaLen > 0)
        {
            *usAuthType = gastAtParaList[5].ulParaValue;
        }
        else
        {
            *usAuthType = PPP_NO_AUTH_TYPE;
        }
        return VOS_TRUE;
    }
    else if (VOS_TRUE == g_DocomoAuthActFlag)
    {
        *usAuthType = PPP_NO_AUTH_TYPE;
        return VOS_TRUE;
    }
    else
    {   
        *usAuthType = PPP_CHAP_AUTH_TYPE;
        return VOS_FALSE;
    }
    
}
/*****************************************************************************
 函 数 名  : AT_PS_ParseUsrDialAuthtype
 功能描述  : 获取用户拨号鉴权类型
*****************************************************************************/
VOS_VOID AT_PS_ParseUsrDialAuthtype(VOS_UINT8 ucIndex, 
    AT_DIAL_PARAM_STRU *pstUsrDialParam)
{
    VOS_UINT16                          usNvAuthType;
    if(VOS_TRUE == AT_PS_GetUsrAuthParam(ucIndex,&usNvAuthType))
    {
        pstUsrDialParam->usAuthType = usNvAuthType;
    }
    else
    {
        if ((gastAtParaList[5].usParaLen > 0) && (0 == gastAtParaList[5].ulParaValue)
            && (VOS_TRUE == AT_DIAL_Exist_User_Password()))
        {
            pstUsrDialParam->usAuthType = PPP_CHAP_AUTH_TYPE;
        }
        else if(gastAtParaList[5].usParaLen > 0)
        {
            pstUsrDialParam->usAuthType = AT_CtrlGetPDPAuthType(gastAtParaList[5].ulParaValue,
                                                                    gastAtParaList[5].usParaLen);
        }
        else
        {
            /* 如果用户名和密码长度均不为0, 且鉴权类型未设置, 则默认使用CHAP类型 */
            if ( (0 != gastAtParaList[3].usParaLen)
              && (0 != gastAtParaList[4].usParaLen) )
            {
                pstUsrDialParam->usAuthType = PPP_CHAP_AUTH_TYPE;
            }
            else
            {
                pstUsrDialParam->usAuthType = PPP_NO_AUTH_TYPE;
            }
        }
    }
    return;
}

/*****************************************************************************
 函 数 名  : AT_PS_MatchConnFailAuthFallbackCause
 功能描述  : 匹配连接建立失败触发鉴权回退处理流程的原因值
 输入参数  : enCause   - 错误码
 输出参数  : 无
 返 回 值  : VOS_TRUE  - 匹配成功
             VOS_FALSE - 匹配失败
*****************************************************************************/
VOS_UINT32 AT_PS_MatchConnFailAuthFallbackCause(
    TAF_PS_CAUSE_ENUM_UINT32            enCause
)
{
    VOS_UINT32                          ulRslt = VOS_FALSE;

    /* 检查是否匹配拒绝原因值为#29/#31 */
    if ((TAF_PS_CAUSE_SM_NW_USER_AUTHENTICATION_FAIL == enCause)
         || (TAF_PS_CAUSE_SM_NW_ACTIVATION_REJECTED_UNSPECIFIED == enCause))
    {
        ulRslt = VOS_TRUE;
    }

    return ulRslt;
}

/*****************************************************************************
 函 数 名  : AT_PS_ProcAuthCallReject
 功能描述  : 鉴权失败时进行回退
*****************************************************************************/
VOS_UINT32 AT_PS_ProcAuthCallReject(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             ucPdpType
)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity       = VOS_NULL_PTR;
    AT_DIAL_PARAM_STRU                  stCallDialParam;
    
    PS_MEM_SET((void *)&stCallDialParam, 0x00, sizeof(AT_DIAL_PARAM_STRU));
    
    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);
    
    /* 鉴权失败回退处理 */
    if((VOS_TRUE == g_AuthFallbackInfo.AuthFallbackFlag) && (VOS_TRUE == AT_PS_MatchConnFailAuthFallbackCause(pstEvent->enCause)))
    {
        if(g_AuthFallbackInfo.AuthNum < MAX_AUTH_NUM)
        {
        
            /* 设置PDPtype对应的CID为无效 */
            AT_PS_SetCid2CurrCall(pstEvent->stCtrl.usClientId, ucCallId, ucPdpType, AT_PS_CALL_INVALID_CID);

            /* 将pdptype类型的PDP状态切换到IDLE */
            AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, ucPdpType, AT_PDP_STATE_IDLE);

            /* 释放CALLID和CID的映射关系 */
            AT_PS_FreeCallIdToCid(pstEvent->stCtrl.usClientId, pstEvent->ucCid);

            g_AuthFallbackInfo.AuthNum++;
        
            if(PPP_CHAP_AUTH_TYPE == pstCallEntity->stUsrDialParam.usAuthType)
            {
                pstCallEntity->stUsrDialParam.usAuthType = PPP_PAP_AUTH_TYPE;
            }
            else if(PPP_PAP_AUTH_TYPE == pstCallEntity->stUsrDialParam.usAuthType)
            {
               pstCallEntity->stUsrDialParam.usAuthType = PPP_NO_AUTH_TYPE;
            }
            else
            {
               pstCallEntity->stUsrDialParam.usAuthType = PPP_CHAP_AUTH_TYPE;
            }
            
            /* 填写拨号参数 */
            AT_PS_GenCallDialParam(&stCallDialParam,
                                   &pstCallEntity->stUsrDialParam,
                                   pstCallEntity->stUserInfo.ucUsrCid,
                                   pstEvent->enPdpType);
            
            if (VOS_OK == AT_PS_SetupCall(pstEvent->stCtrl.usClientId, ucCallId, &stCallDialParam))
            {
                return VOS_TRUE;
            }
        }
        else
        {
            /*如果鉴权3次鉴权回退都未成功，则将鉴权参数返回为最初发起拨号的鉴权参数*/
            pstCallEntity->stUsrDialParam.usAuthType = g_AuthFallbackInfo.AuthType;
            PS_MEM_SET((void *)&g_AuthFallbackInfo, 0x00, sizeof(g_AuthFallbackInfo));

            return VOS_FALSE;
        }
    }
    return VOS_FALSE;
}
/*****************************************************************************
 函 数 名  : AT_PS_SavePSCallAuthRecord
 功能描述  : 保存鉴权信息
*****************************************************************************/
VOS_VOID AT_PS_SavePSCallAuthRecord(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent
)
{
    NV_HUAWEI_AUTHFALLBACK_FEATURE_STRU        stAuthFallback;
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity;
    
    PS_MEM_SET((void *)&stAuthFallback, 0x00, sizeof(NV_HUAWEI_AUTHFALLBACK_FEATURE_STRU));
    
    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);
    
    /*NV中存储的鉴权类型与拨号成功的鉴权类型不一致时，将鉴权类型保存到鉴权nv中*/
    if((VOS_TRUE == g_AuthFallbackInfo.AuthFallbackFlag) 
        && (g_Nv_AuthFallbackInfo.NvAuthType != pstCallEntity->stUsrDialParam.usAuthType))
    {
        stAuthFallback.ucNvActiveFlag = VOS_TRUE;
        stAuthFallback.ucAuthType = (VOS_UINT8)pstCallEntity->stUsrDialParam.usAuthType;
        (VOS_VOID)NV_Write(en_NV_Item_AUTHFALLBACK_FEATURE,&stAuthFallback,sizeof(stAuthFallback));
    }
    PS_MEM_SET((void *)&g_AuthFallbackInfo, 0x00, sizeof(g_AuthFallbackInfo));
    return;
}


VOS_VOID AT_ReadHuaweiMbbFeature( VOS_VOID )
{
    VOS_UINT32                        ulResult = NV_OK;

    ulResult = NV_ReadEx(MODEM_ID_0,
                            NV_ID_HUAWEI_MBB_FEATURE,
                            &g_stMbbFeature,
                            sizeof(NV_HUAWEI_MBB_FEATURE_STRU));

    if(NV_OK != ulResult)
    {
        PS_MEM_SET(&g_stMbbFeature, 0, sizeof(g_stMbbFeature));/*lint !e516 */
        g_stMbbFeature.ucPcmModemLoopEnable = VOS_TRUE;
    }
    else
    {
        return;
    }
}
PRIVATE_SYMBOL  TAF_UINT8  gucIsSinglePdn = AT_HFEATURE_NOT_OPEN;
/*********************************************************************************************
 函 数 名  : AT_ReadTafPdpParaNV
 功能描述  : at 初始化时通过读取单板频段NV判断RAT支持情况
 输入参数  : 无
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :
 修改历史  :
*********************************************************************************************/
VOS_VOID  AT_ReadTafPdpParaNV(VOS_VOID)
{
   TAF_NVIM_PDP_PARA_STRU              *pstNvPdpTable = VOS_NULL_PTR;

    pstNvPdpTable = (TAF_NVIM_PDP_PARA_STRU *)MBB_MEM_ALLOC(WUEPS_PID_TAF, sizeof(TAF_NVIM_PDP_PARA_STRU));
    if (VOS_NULL_PTR == pstNvPdpTable)
    {
        return;
    }
    /*lint -e516*/
    MBB_MEM_SET(pstNvPdpTable, 0x00, sizeof(TAF_NVIM_PDP_PARA_STRU));
    /*lint +e516*/
    if (NV_OK != NV_Read(en_NV_Item_Taf_PdpPara_0, pstNvPdpTable, sizeof(TAF_NVIM_PDP_PARA_STRU)))
    {
       /*lint -e516*/
        MBB_MEM_FREE(WUEPS_PID_AT, pstNvPdpTable);
       /*lint +e516*/
        return;
    }

    /*该PDP没有定义或者该PDP被定义成二次PDP*/
    if ((TAF_FREE == pstNvPdpTable->aucPdpPara[0])  || (TAF_USED == pstNvPdpTable->aucPdpPara[2]))
    {
        gucIsSinglePdn = AT_HFEATURE_OPEN;
    }
    /*lint -e516*/
    MBB_MEM_FREE(WUEPS_PID_AT, pstNvPdpTable);
    /*lint +e516*/
    pstNvPdpTable = VOS_NULL_PTR;
}/*lint !e438*/

VOS_VOID AT_ReadNvMbbCustorm(VOS_VOID)
{
   AT_ReadWinblueProfileType();
    /*德国 Vodafone CPBS定制*/
    AT_ReadVodafoneCpbsNV();
    /*AT+CSIM定制*/
    AT_ReadCsimCustomizationNV();
    AT_ReadSyscfgexBandListFromNV();
    AT_ReadRfCapabilityNV();

    AT_ReadAuthFallBackInfoNV();
    AT_InitCclkInfo();
    AT_ReadTafPdpParaNV();

    AT_ReadHuaweiMbbFeature();

    At_MbbMatchAtInit();
}



VOS_BOOL  Mbb_IsDataLockPass(VOS_VOID)
{
    return (VOS_BOOL)(FALSE == g_bAtDataLocked);
}

/*****************************************************************************
 函 数 名  : AT_GetModemStat
 功能描述  : 返回modem拨号的状态
 输入参数  : cid
 输出参数  : 无
 返 回 值  : VOS_UINT8
*****************************************************************************/
VOS_UINT8 AT_GetModemStat(VOS_UINT8  ucCid)
{
    return g_stAtModemState[ucCid];
}

/*****************************************************************************
 函 数 名  : AT_SetModemStat
 功能描述  : 保存modem拨号的状态
 输入参数  : cid,AT_PDP_STATE_ENUM_U8
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
VOS_VOID AT_SetModemStat(VOS_UINT8  ucCid, AT_PDP_STATE_ENUM_U8 enPdpState )
{
    g_stAtModemState[ucCid] = enPdpState;
}

/*****************************************************************************
 函 数 名  : At_MBB_QryDconnStatPara
 功能描述  : 查询当前拨号状态
 输入参数  : TAF_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : TAF_UINT32
*****************************************************************************/

VOS_UINT32 At_MBB_QryDconnStatPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                             enModemId = 0;
    TAF_NVIM_PDP_PARA_STRU                 *stPdpPara = VOS_NULL_PTR;      

    VOS_UINT8                              ucTmp;
    VOS_UINT8                              ucNum = 0;
    VOS_UINT16                             usLength = 0;
    VOS_UINT8                              aucStr[TAF_MAX_APN_LEN + 1];
    AT_PS_CALL_ENTITY_STRU                 *pstCallEntity;
    VOS_UINT8                              ucCallId;
    VOS_UINT8                              ucRslt;
    AT_DIALTYPE_ENUM_UINT32                enDialType = AT_DIALTYPE_BUTT;
    AT_PDP_STATUS_ENUM_UINT32              enIpv4Status = AT_PDP_STATUS_DEACT;
    AT_PDP_STATUS_ENUM_UINT32              enIpv6Status = AT_PDP_STATUS_DEACT;
    AT_PDP_STATUS_ENUM_UINT32              enModemStatus = AT_PDP_STATUS_DEACT;

    enModemId = VOS_GetModemIDFromPid(I0_WUEPS_PID_TAF);
    stPdpPara = (TAF_NVIM_PDP_PARA_STRU*)MBB_MEM_ALLOC(WUEPS_PID_AT,
                            sizeof(TAF_NVIM_PDP_PARA_STRU));

    if(VOS_NULL_PTR == stPdpPara)
    {
        AT_ERR_LOG("At_MBB_QryDconnStatPara: malloc fail.");
        return AT_ERROR;
    }

    for (ucTmp = 0; ucTmp <= TAF_MAX_CID_NV; ucTmp++)
    {
        MBB_MEM_SET(stPdpPara, 0x00, sizeof(TAF_NVIM_PDP_PARA_STRU));/*lint !e516*/

        if (NV_OK != NV_ReadEx(enModemId,(NV_ID_ENUM_U16)(en_NV_Item_Taf_PdpPara_0 + ucTmp),
                                        stPdpPara,
                                        sizeof(TAF_NVIM_PDP_PARA_STRU)))
        {
            /*NV读取失败，在单板正常状态下不会出现该场景*/
            continue;
        }
        
        /*该PDP没有定义，该PDP被定义成二次PDP，直接返回*/
        if((TAF_FREE == stPdpPara->aucPdpPara[0]) || (TAF_USED == stPdpPara->aucPdpPara[2]))
        {
            continue;
        }
        
        /*dconnstat返回结果，dconnstat前换行*/

        if(0 != ucNum)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,
                        (VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);/*lint !e64 !e119*/
        }

        /* +DCONNSTAT:  */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,
                    (VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119*/
        /* <cid> */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,
                    (VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d",ucTmp);/*lint !e64 !e119*/

        ucNum++;

        enModemStatus = AT_NdisGetConnStatus(AT_GetModemStat(ucTmp));
        if(AT_PDP_STATUS_ACT == enModemStatus)
        {
            /* <APN> */
            MBB_MEM_SET(aucStr, 0x00, sizeof(aucStr));/*lint !e516 */
            MBB_MEM_CPY(aucStr, &stPdpPara->aucPdpPara[9], stPdpPara->aucPdpPara[8]);/*lint !e516 */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,
                        (VOS_CHAR*)pgucAtSndCodeAddr + usLength,",\"%s\"", aucStr);/*lint !e64 !e119*/
            /*<modem: ipv4 state,ipv6 state,dialtype>*/
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               ",%d,0,%d",/*lint !e64 !e119*/
                                               enModemStatus,AT_DIALTYPE_MODEM);/*lint !e64 !e119*/
            continue;
        }

        /*<app&ndis: ipv4 state,ipv6 state,dialtype>*/
        ucCallId = 0;
        enIpv4Status = AT_PDP_STATUS_DEACT;
        enIpv6Status = AT_PDP_STATUS_DEACT;
        
        ucCallId = AT_PS_TransCidToCallId(ucIndex, ucTmp);
        ucRslt = AT_PS_IsCallIdValid(ucIndex, ucCallId);
        if (VOS_TRUE != ucRslt)
        {
            continue;
        }
        pstCallEntity = AT_PS_GetCallEntity(ucIndex, ucCallId);
        enIpv4Status = AT_NdisGetConnStatus( pstCallEntity->enIpv4State );
        enIpv6Status = AT_NdisGetConnStatus( pstCallEntity->enIpv6State );
        if((AT_PDP_STATUS_DEACT == enIpv4Status) && (AT_PDP_STATUS_DEACT == enIpv6Status))
        {
            continue;
        }
        if(AT_CLIENT_TAB_NDIS_INDEX == pstCallEntity->stUserInfo.enUserIndex) 
        {
            enDialType = AT_DIALTYPE_NDIS;
        }
        else if(AT_CLIENT_TAB_APP_INDEX == pstCallEntity->stUserInfo.enUserIndex)
        {
            enDialType = AT_DIALTYPE_APP;
        }
        else
        {
            enDialType = AT_DIALTYPE_BUTT;
        }
        /* <APN> */
        PS_MEM_SET(aucStr, 0x00, sizeof(aucStr));/*lint !e516 */
        PS_MEM_CPY(aucStr, pstCallEntity->stUsrDialParam.aucAPN, pstCallEntity->stUsrDialParam.ucAPNLen);/*lint !e516 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,/*lint !e64 !e119*/
                    (VOS_CHAR*)pgucAtSndCodeAddr + usLength,",\"%s\"", aucStr);/*lint !e64 !e119*/
        /*<ipv4 state,ipv6 state,dialtype>*/


        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           ",%d,%d,%d",/*lint !e64 !e119*/
                                           enIpv4Status,enIpv6Status,enDialType);/*lint !e64 !e119*/

    }

    gstAtSendData.usBufLen  = usLength;
    /*lint -e516*/
    MBB_MEM_FREE(WUEPS_PID_AT, stPdpPara);
    /*lint +e516*/
    return AT_OK;
}
/*****************************************************************************
 函 数 名  : AT_MBB_TestDconnStatPara
 功能描述  : 返回已拨号成功的cid
 输入参数  : TAF_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : TAF_UINT32
*****************************************************************************/
VOS_UINT32 AT_MBB_TestDconnStatPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                              usLength;
    VOS_UINT8                               ucCallId;
    AT_PS_CALL_ENTITY_STRU                  *pstCallEntity;
    VOS_UINT32                              ulTmp;
    VOS_UINT8                               ucRslt;
    AT_PDP_STATUS_ENUM_UINT32               enIpv4Status = AT_PDP_STATUS_DEACT;
    AT_PDP_STATUS_ENUM_UINT32               enIpv6Status = AT_PDP_STATUS_DEACT;
    AT_PDP_STATUS_ENUM_UINT32               enModemStatus = AT_PDP_STATUS_DEACT;
    VOS_UINT32                              ulNum;

    usLength    = 0;
    ulNum = 0;
    /* 参数检查 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                    (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);/*lint !e64 !e119*/

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                    (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s", "(");/*lint !e64 !e119*/

    for(ulTmp = 1;ulTmp <= TAF_MAX_CID; ulTmp++)
    {

        enModemStatus = AT_NdisGetConnStatus(AT_GetModemStat(ulTmp));
        if(AT_PDP_STATUS_ACT == enModemStatus)
        {
            if (0 == ulNum )
            {   /*如果是第一个CID，则CID前不打印逗号*/
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%d",ulTmp);/*lint !e64 !e119*/
                ulNum++; 
            }
            else
            {   /*如果不是第一个CID，则CID前打印逗号*/
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",ulTmp);/*lint !e64 !e119*/
            }
        }
        ucCallId = 0;
        enIpv4Status = AT_PDP_STATUS_DEACT;
        enIpv6Status = AT_PDP_STATUS_DEACT;

        ucCallId = AT_PS_TransCidToCallId(ucIndex, ulTmp);
        ucRslt = AT_PS_IsCallIdValid(ucIndex, ucCallId);
        if (VOS_TRUE != ucRslt)
        {
            continue;
        }

        pstCallEntity = AT_PS_GetCallEntity(ucIndex, ucCallId);
        enIpv4Status = AT_NdisGetConnStatus( pstCallEntity->enIpv4State );
        enIpv6Status = AT_NdisGetConnStatus( pstCallEntity->enIpv6State );

        if((AT_PDP_STATUS_ACT == enIpv4Status) || (AT_PDP_STATUS_ACT == enIpv6Status))
        {

            if (0 == ulNum )
            {   /*如果是第一个CID，则CID前不打印逗号*/
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%d",ulTmp);/*lint !e64 !e119*/
                ulNum++; 
            }
            else
            {   /*如果不是第一个CID，则CID前打印逗号*/
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",ulTmp);/*lint !e64 !e119*/
            }

        }
    }

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                    (TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s", ")");/*lint !e64 !e119*/
    gstAtSendData.usBufLen  = usLength;

    return AT_OK;

}



VOS_VOID At_MbbReportCerssiInfo(VOS_UINT32 ulCerssiFlag,
                    VOS_UINT8 ucCerssiReportType,
                    VOS_UINT8 ucCerssiMinTimerInterval,
                    VOS_UINT16 *ptrLength, 
                    TAF_MMA_L_CELL_SIGN_INFO_STRU *pstLCellSignInfo)
{
    VOS_INT16 usTempRsrp = 0;
    VOS_UINT32 ulIndex = 0;

    if ((VOS_NULL_PTR == ptrLength) || (VOS_NULL_PTR == pstLCellSignInfo))
    {
        return;
    }
    
    usTempRsrp = pstLCellSignInfo->stRxAntInfo.asRsrpRx[0];

    for (ulIndex = 1; ulIndex < pstLCellSignInfo->stRxAntInfo.ucRxANTNum; ulIndex++)
    {
        if(usTempRsrp < pstLCellSignInfo->stRxAntInfo.asRsrpRx[ulIndex])
        {
            usTempRsrp = pstLCellSignInfo->stRxAntInfo.asRsrpRx[ulIndex];
        }
    }
    
    *ptrLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + *ptrLength,
                           "%s%s",
                           gaucAtCrLf,
                           gastAtStringTab[AT_STRING_CERSSI].pucText);

    /* 如果为查询，那么需要上报信号质量变化值，和时间间隔 */
    if(VOS_TRUE == ulCerssiFlag)
    {
        *ptrLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + *ptrLength,
                           "%d,%d,",
                           ucCerssiReportType,
                           ucCerssiMinTimerInterval);
    }
    *ptrLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                       (VOS_CHAR *)pgucAtSndCodeAddr,
                       (VOS_CHAR *)pgucAtSndCodeAddr + *ptrLength,
                       "0,0,255,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                        usTempRsrp,
                        pstLCellSignInfo->sRsrq,
                        pstLCellSignInfo->lSINR,
                        pstLCellSignInfo->stCQI.usRI,
                        pstLCellSignInfo->stCQI.ausCQI[0],
                        pstLCellSignInfo->stCQI.ausCQI[1],
                        pstLCellSignInfo->stRxAntInfo.ucRxANTNum,
                        pstLCellSignInfo->stRxAntInfo.asRsrpRx[0],
                        pstLCellSignInfo->stRxAntInfo.asRsrpRx[1],
                        pstLCellSignInfo->stRxAntInfo.asRsrpRx[2],
                        pstLCellSignInfo->stRxAntInfo.asRsrpRx[3],
                        pstLCellSignInfo->stRxAntInfo.alSINRRx[0],
                        pstLCellSignInfo->stRxAntInfo.alSINRRx[1],
                        pstLCellSignInfo->stRxAntInfo.alSINRRx[2],
                        pstLCellSignInfo->stRxAntInfo.alSINRRx[3]);
    /* 如果为主动上报，保留逗号 */
    if(VOS_FALSE == ulCerssiFlag)
    {
        *ptrLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + *ptrLength,
                           ",%s",
                           gaucAtCrLf);
    }
    else
    {
        *ptrLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + *ptrLength,
                           "%s",
                           gaucAtCrLf);
    }
    return;
}



VOS_VOID   AT_MBB_RecordMlogDialFail(VOS_UINT8 ucCallId, TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU   *pstEvent)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    AT_MODEM_PS_CTX_STRU              *pstPsModemCtx = VOS_NULL_PTR;
    AT_PS_USER_INFO_STRU                stUsrInfo    = {0};
    TAF_PDP_TYPE_ENUM_UINT8           enPdpType = TAF_PDP_IPV4;
    VOS_UINT8 ucCid = 0;
    VOS_UINT8 aucApn[TAF_MAX_APN_LEN + 1] = {'\0'}; 

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(pstEvent->stCtrl.usClientId);
    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);
    stUsrInfo      = pstPsModemCtx->astCallEntity[ucCallId].stUserInfo;
    enPdpType = pstCallEntity->enCurrPdpType;
    ucCid = stUsrInfo.ucUsrCid;
    (VOS_VOID)PS_MEM_CPY(aucApn, pstCallEntity->stUsrDialParam.aucAPN, pstCallEntity->stUsrDialParam.ucAPNLen);/*lint !e560!506*/

    mlog_print("at", mlog_lv_error, "Dial fail, PDP type is %d, CID is %d, apn is %s.\n", enPdpType, ucCid, aucApn); /*lint !e516*/

    if(TAF_PH_INFO_LTE_RAT == g_stAtSysMode.ucRatType)
    {
        mlog_print("at", mlog_lv_error, "Dial fail, rsrp is %d, rsrq is %d, sinr is %d.\n", 
                   g_stSignalInfo.sRsrpValue, g_stSignalInfo.sRsrqValue, g_stSignalInfo.lSINRValue);/*lint !e515 !e516*/
    }
    else if((TAF_PH_INFO_WCDMA_RAT == g_stAtSysMode.ucRatType))
    {
        mlog_print("at", mlog_lv_error, "Dial fail, rscp is %d, ecio is %d.\n", 
                   g_stSignalInfo.sRscpValue, g_stSignalInfo.sEcioValue);/*lint !e515 !e516*/
    }
    else
    {
        mlog_print("at", mlog_lv_error, "Dial fail, rssi is %d.\n", g_stSignalInfo.sRssiValue);/*lint !e515 !e516*/
    }
}

VOS_VOID AT_MBB_MlogPrintPdpInfo(VOS_UINT8 ucCallId, TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU   *pEvtInfo)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    TAF_PDP_TYPE_ENUM_UINT8           enPdpType = TAF_PDP_IPV4;
    VOS_UINT8                           aucIpv6PrimDnsStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN] = {0};
    VOS_UINT8                           aucIpv6SecDnsStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN] = {0};

    pstCallEntity = AT_PS_GetCallEntity(pEvtInfo->stCtrl.usClientId, ucCallId); 
    enPdpType = pstCallEntity->enCurrPdpType;

    mlog_print("at", mlog_lv_info, "Dial success, PDP type is %d\n", enPdpType);/*lint !e515*/

    if((TAF_PDP_IPV4 == enPdpType) || (TAF_PDP_PPP == enPdpType))
    {
        mlog_print("at", mlog_lv_info, "IPv4 Bearer Primary DNS 0x%x\n", pstCallEntity->stIpv4DhcpInfo.ulIpv4PrimDNS);/*lint !e515 !e516*/
        mlog_print("at", mlog_lv_info, "IPv4 Bearer Secondary DNS 0x%x\n", pstCallEntity->stIpv4DhcpInfo.ulIpv4SecDNS);/*lint !e515 !e516*/
        mlog_print("at", mlog_lv_info, "IPv4 Bearer Gate Way 0x%x\n", pstCallEntity->stIpv4DhcpInfo.ulIpv4GateWay);/*lint !e515 !e516*/
    }
    else
    {
        PS_MEM_SET(aucIpv6PrimDnsStr, 0x00, sizeof(aucIpv6PrimDnsStr));
        PS_MEM_SET(aucIpv6SecDnsStr, 0x00, sizeof(aucIpv6SecDnsStr));

        (VOS_VOID)AT_ConvertIpv6AddrToCompressedStr(aucIpv6PrimDnsStr,
                                          pstCallEntity->stIpv6DhcpInfo.aucIpv6PrimDNS,
                                          TAF_IPV6_STR_RFC2373_TOKENS);

        (VOS_VOID)AT_ConvertIpv6AddrToCompressedStr(aucIpv6SecDnsStr,
                                          pstCallEntity->stIpv6DhcpInfo.aucIpv6SecDNS,
                                          TAF_IPV6_STR_RFC2373_TOKENS);

        mlog_print("at", mlog_lv_info, "IPv6 Bearer Primary DNS %s", aucIpv6PrimDnsStr);/*lint !e515 !e516*/
        mlog_print("at", mlog_lv_info, "IPv6 Bearer Secondary DNS %s", aucIpv6SecDnsStr);/*lint !e515 !e516*/
        if((TAF_PDP_IPV4V6 == enPdpType))
        {
            mlog_print("at", mlog_lv_info, "IPv4 Bearer Primary DNS 0x%x", pstCallEntity->stIpv4DhcpInfo.ulIpv4PrimDNS);/*lint !e515 !e516*/
            mlog_print("at", mlog_lv_info, "IPv4 Bearer Secondary DNS 0x%x", pstCallEntity->stIpv4DhcpInfo.ulIpv4SecDNS);/*lint !e515 !e516*/
            mlog_print("at", mlog_lv_info, "IPv4 Bearer Gate Way 0x%x", pstCallEntity->stIpv4DhcpInfo.ulIpv4GateWay);/*lint !e515 !e516*/
        }
    }

}     




VOS_VOID AT_InitSockOmRunSem()
{
    sema_init(&g_stSockOmRunSem, 0);
}


VOS_VOID AT_SuspendSockOmServer()
{
    if(TRUE == g_bAtDataLocked)
    {
        down(&g_stSockOmRunSem);
    }
}


VOS_VOID AT_WakeUpSockOmServer()
{
    if(FALSE == g_bAtDataLocked)
    {
        up(&g_stSockOmRunSem);
    }
}


VOS_BOOL AT_IsSockForbidAtCmd(const VOS_UINT8 ucIndex, const VOS_CHAR *pszCmdName)
{
    VOS_INT32 lCmpResult = 0;
    VOS_UINT32 i = 0;

    /*非SOCKET AT命令或已经解锁*/
    if((AT_CLIENT_ID_SOCK != ucIndex) || (VOS_FALSE == g_bAtDataLocked))
    {
        return VOS_FALSE;
    }

    if(VOS_NULL_PTR == pszCmdName)
    {
        return VOS_FALSE;
    }

    while(VOS_NULL_PTR != g_pcSockAtCmdTbl[i])
    {
        lCmpResult = AT_STRCMP((VOS_CHAR*)pszCmdName, (VOS_CHAR*)g_pcSockAtCmdTbl[i]);
        if (0 == lCmpResult)
        {
            /*该命令为允许处理命令*/
            return VOS_FALSE;
        }
        i++;
    }

    /*如果到这说明该AT不可用*/
    return VOS_TRUE;
}

PRIVATE_SYMBOL TAF_UINT32 At_SetHFeaturestat( TAF_UINT8 ucIndex )
{
   VOS_UINT8 ucFeatureId = 0;

   /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /*参数长度不为1*/
    if (1 != gastAtParaList[0].usParaLen)
    {
        return AT_ERROR;
    }

    ucFeatureId = (TAF_UINT8)(gastAtParaList[0].ulParaValue);

    if(AT_HFEATURE_SINGLE_PDN != ucFeatureId)
    {
        return AT_ERROR;/*目前仅支持single_pdn特性的查询设置*/
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
               (TAF_CHAR*)pgucAtSndCodeAddr, (TAF_CHAR*)pgucAtSndCodeAddr,
               "%s: %d,%d",
               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
               ucFeatureId,
               gucIsSinglePdn);
    
    return AT_OK;
}

PRIVATE_SYMBOL TAF_UINT32 At_QryHFeaturestat( TAF_UINT8 ucIndex )
{
    VOS_UINT8 ucFeatureId = AT_HFEATURE_SINGLE_PDN;/*目前仅支持SINGLE PDN特性*/
   
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                (TAF_CHAR*)pgucAtSndCodeAddr, (TAF_CHAR*)pgucAtSndCodeAddr,
                "%s: %d,%d",
                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                ucFeatureId,
                gucIsSinglePdn);
    
    return AT_OK;
}







VOS_UINT32 AT_QryTxPowerPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult = AT_ERROR;

    /* 发送消息*/
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_QRY_TXPOWER_REQ,
                                      VOS_NULL_PTR,
                                      0,
                                      I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_QryTxPowerPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_TXPOWER_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaTxPowerQryCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                *pstRcvMsg               = VOS_NULL_PTR;
    AT_MTA_TX_PWR_QRY_CNF          *pstMtaAtQryTxPowerCnf = VOS_NULL_PTR;
    VOS_UINT32                      ulResult = AT_ERROR;
    VOS_UINT8                       ucIndex = 0;
    VOS_UINT16                      usLenth = 0;

    /* 入参检查 */
    if(VOS_NULL_PTR == pMsg)
    {
        return VOS_ERR;
    }

    /* 初始化 */
    pstRcvMsg              = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaAtQryTxPowerCnf  = (AT_MTA_TX_PWR_QRY_CNF *)pstRcvMsg->aucContent;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaTxPowerQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaTxPowerQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_TXPOWER_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaTxPowerQryCnf : Current Option is not AT_CMD_RRC_VERSION_QUERY.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (MTA_AT_RESULT_NO_ERROR != pstMtaAtQryTxPowerCnf->enResult)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;

        usLenth = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "^TXPOWER: %d,%d,%d,%d,%d",
                                       pstMtaAtQryTxPowerCnf->stxPwr,
                                       pstMtaAtQryTxPowerCnf->sPuschPwr,
                                       pstMtaAtQryTxPowerCnf->sPucchPwr,
                                       pstMtaAtQryTxPowerCnf->sSrsPwr,
                                       pstMtaAtQryTxPowerCnf->sPrachPwr);
    }

    gstAtSendData.usBufLen = usLenth;

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_SetMcsPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32 ulResult = AT_ERROR;

    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数过多，返回参数错误*/
    if (1 < gucAtParaIndex)
    {
        AT_WARN_LOG("AT_SetMcsPara: the aucPara error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    g_McsFlag = gastAtParaList[0].ulParaValue;
    /* 发送消息*/
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_SET_MCS_REQ,
                                      VOS_NULL_PTR,
                                      0,
                                      I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_SetMcsPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MCS_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaMcsSetCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                *pstRcvMsg         = VOS_NULL_PTR;
    AT_MTA_MCS_QRY_CNF             *pstMtaAtQryMcsCnf = VOS_NULL_PTR;
    VOS_UINT32                      ulResult = AT_ERROR;
    VOS_UINT8                       ucIndex = 0;
    VOS_UINT16                      usLenth = 0;

    /* 入参检查 */
    if(VOS_NULL_PTR == pMsg)
    {
        return VOS_ERR;
    }

    /* 初始化 */
    pstRcvMsg          = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaAtQryMcsCnf  = (AT_MTA_MCS_QRY_CNF *)pstRcvMsg->aucContent;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMcsQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMcsQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_MCS_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaMcsQryCnf : Current Option is not AT_CMD_RRC_VERSION_QUERY.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (MTA_AT_RESULT_NO_ERROR != pstMtaAtQryMcsCnf->enResult)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
        if (UPMCS_FLAG == g_McsFlag)
        {
            usLenth = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               "^LTEMCS: %d,%d",
                                               g_McsFlag,
                                               pstMtaAtQryMcsCnf->stMcsInfo[0].usUlMcs);
        }
        else 
        {
            usLenth = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               "^LTEMCS: %d,%d,%d",
                                               g_McsFlag,
                                               pstMtaAtQryMcsCnf->stMcsInfo[0].usDlMcs[0],
                                               pstMtaAtQryMcsCnf->stMcsInfo[0].usDlMcs[1]);
        }
    }

    gstAtSendData.usBufLen = usLenth;
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}


VOS_UINT32 AT_QryTddPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult = AT_ERROR;

    /* 发送消息*/
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_QRY_TDD_REQ,
                                      VOS_NULL_PTR,
                                      0,
                                      I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_QryTddPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_TDD_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvMtaTddQryCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU     *pstRcvMsg         = VOS_NULL_PTR;
    AT_MTA_TDD_QRY_CNF  *pstMtaAtQryTddCnf = VOS_NULL_PTR;
    VOS_UINT32          ulResult = AT_ERROR;
    VOS_UINT8           ucIndex = 0;
    VOS_UINT16          usLenth = 0;

    /* 入参检查 */
    if(VOS_NULL_PTR == pMsg)
    {
        return VOS_ERR;
    }

    /* 初始化 */
    pstRcvMsg          = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaAtQryTddCnf  = (AT_MTA_TDD_QRY_CNF *)pstRcvMsg->aucContent;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaTddQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaTddQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_TDD_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaTddQryCnf : Current Option is not AT_CMD_RRC_VERSION_QUERY.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    if (MTA_AT_RESULT_NO_ERROR != pstMtaAtQryTddCnf->enResult)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;

        usLenth = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "^LTETDDSUBFRAME: %d,%d",
                                       pstMtaAtQryTddCnf->ucSubframeAssign,
                                       pstMtaAtQryTddCnf->ucSubframePatterns);
    }

    gstAtSendData.usBufLen = usLenth;
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*********************************************************************
MBB AT列表g_astAtPrivateMbbCmdTbl，MBB新增AT添加到此处
**********************************************************************/
/*lint -e64  -e133 -e10*/
AT_PAR_CMD_ELEMENT_STRU g_astAtPrivateMbbCmdTbl[] = {

    {AT_CMD_NDISSTATQRY,
    AT_SetNdisStatPara , AT_SET_PARA_TIME, AT_QryNdisStatPara, AT_QRY_PARA_TIME, At_TestNdisstatqry, AT_TEST_PARA_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NDISSTATQRY", (VOS_UINT8*)"(1-11)"},
    
    /*BIP*/
    {AT_CMD_BIPCMD,
    At_SetBipTestCmd,     AT_SET_PARA_TIME,   TAF_NULL_PTR,    AT_NOT_SET_TIME,   At_CmdTestProcOK, AT_SET_PARA_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE | CMD_TBL_NO_LIMITED,
    (TAF_UINT8*)"^BIPCMD",    (TAF_UINT8*)"(text)"},
    {AT_CMD_DLCK,
    AT_SetDlckPara,     AT_SET_PARA_TIME,    AT_QryDlckPara,    AT_QRY_PARA_TIME,    At_CmdTestDlck,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^DLCK",  (VOS_UINT8*)"(0-3),(oldpassword),(newpassword)"},
    {AT_CMD_NWSCAN,
    AT_SetNwScanPara, AT_NWSCAN_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, 
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    (VOS_UINT32)AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
    (VOS_UINT8*)"^NWSCAN", (VOS_UINT8*)"(0-7),(@band),(0-65535)"},
    {(VOS_UINT32)AT_CMD_NWTIME,
    VOS_NULL_PTR,    AT_NOT_SET_TIME,  AT_QryNWTimePara, AT_QRY_PARA_TIME,   
    VOS_NULL_PTR, AT_NOT_SET_TIME, /*lint !e64*/
    VOS_NULL_PTR,    AT_NOT_SET_TIME,
    (VOS_UINT32)AT_CME_INCORRECT_PARAMETERS, (VOS_UINT32)CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NWTIME",   VOS_NULL_PTR},
    {AT_CMD_LTERSRP,
    VOS_NULL_PTR,           AT_NOT_SET_TIME,  AT_QryLteRsrp,            AT_QRY_PARA_TIME,   
    AT_TestLteRsrpPara, AT_TEST_PARA_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
    (VOS_UINT8*)"^LTERSRP", VOS_NULL_PTR},
    {(VOS_UINT32)AT_CMD_HCSQ,
    VOS_NULL_PTR,    AT_NOT_SET_TIME,  AT_QryHcsqPara, AT_QRY_PARA_TIME,   
    AT_TestHcsqPara, AT_NOT_SET_TIME, /*lint !e64*/
    VOS_NULL_PTR,    AT_NOT_SET_TIME,
    (VOS_UINT32)AT_CME_INCORRECT_PARAMETERS, (VOS_UINT32)CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^HCSQ",   VOS_NULL_PTR},
    {AT_CMD_EONS,
    At_SetEonsPara,     AT_SET_PARA_TIME,  VOS_NULL_PTR,  AT_NOT_SET_TIME,   
    AT_TestEonsPara, AT_NOT_SET_TIME, 
    VOS_NULL_PTR,    AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
    (VOS_UINT8*)"^EONS",      (VOS_UINT8*)"(1,2,3,4,5),(@plmn)"},  
   {AT_CMD_HFREQINFO,
    At_SetHFreqinfo,     AT_SET_PARA_TIME,  At_QryHFreqinfo, AT_QRY_PARA_TIME, At_TestHFreqinfo, AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL | CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^HFREQINFO",    (VOS_UINT8*)"(0,1)"},
    {AT_CMD_SMSREGST,
    At_SetSmsregstPara,     AT_SET_PARA_TIME, At_QrySmsregstPara,   AT_QRY_PARA_TIME,
    At_TestSmsregst, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,  CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SMSREGST",    (VOS_UINT8*)"(@imsi)"},
    {AT_CMD_CFUN,
    At_SetCfunPara,     AT_SET_PARA_TIME, At_QryCfunPara,   AT_QRY_PARA_TIME,   VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_E5_IS_LOCKED | CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"+CFUN",       (VOS_UINT8*)"(0,1,4,5,6,7,8,10,11),(0,1)"},
    {AT_CMD_CCLK,
     At_SetCCLK,     AT_SET_PARA_TIME,  At_QryCCLK, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
     VOS_NULL_PTR,   AT_NOT_SET_TIME,
     AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
     (VOS_UINT8*)"+CCLK",    (VOS_UINT8*)"(@time)"},
    {AT_CMD_CSIM,
    At_SetCsimPara, AT_SET_PARA_TIME, TAF_NULL_PTR,    AT_NOT_SET_TIME,  AT_TestCsimPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE_E5 | CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"+CSIM",    (TAF_UINT8*)"(1-520),(cmd)"},
    {AT_CMD_DATACLASS,
     VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryDataclassPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
     VOS_NULL_PTR,   AT_NOT_SET_TIME,
     AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
     (TAF_UINT8*)"^DATACLASS",   VOS_NULL_PTR},

    {AT_CMD_HFEATURESTAT,
    At_SetHFeaturestat,  AT_SET_PARA_TIME,  At_QryHFeaturestat, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,  AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,  CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^HFEATURESTAT",  (VOS_UINT8*)"(1)"},



    {AT_CMD_SYSCFGEX,
    AT_SetSysCfgExPara,       AT_SYSCFG_SET_PARA_TIME, AT_QrySysCfgExPara,  AT_QRY_PARA_TIME,   AT_TestSyscfgEx, AT_TEST_PARA_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SYSCFGEX",      (VOS_UINT8*)"(Acqorder),(@Band),(0-2),(0-4),(@Lteband)"},



    {AT_CMD_GCAP,
    At_SetGcapPara,     AT_NOT_SET_TIME,  VOS_NULL_PTR,     AT_NOT_SET_TIME,    At_CmdTestProcOK, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"+GCAP",VOS_NULL_PTR},

    {AT_CMD_CHLD,
    At_SetChldPara,     AT_SET_PARA_TIME, VOS_NULL_PTR,     AT_NOT_SET_TIME,    AT_TestChldPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"+CHLD", (VOS_UINT8*)"(0-5,11-19,21-29)"},


    {AT_CMD_DCONNSTAT,
    VOS_NULL_PTR, AT_NOT_SET_TIME, At_MBB_QryDconnStatPara,  AT_QRY_PARA_TIME,  AT_MBB_TestDconnStatPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^DCONNSTAT",  (VOS_UINT8*)"(0-31)"},

    {AT_CMD_LTECAT,
    VOS_NULL_PTR,     AT_NOT_SET_TIME,  At_QryLteCatEx, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL | CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^LTECAT",    VOS_NULL_PTR},


    {AT_CMD_LCELLINFO,
    At_SetLCellInfoPara,    AT_SET_PARA_TIME, VOS_NULL_PTR , AT_NOT_SET_TIME,   
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,   
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^LCELLINFO",   (VOS_UINT8*)"(0,1)"},
    


    {AT_CMD_TXPOWER,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,  AT_QryTxPowerPara,   AT_QRY_PARA_TIME,    VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
    (VOS_UINT8*)"^TXPOWER",       VOS_NULL_PTR},

    {AT_CMD_LTEMCS,
    AT_SetMcsPara,   AT_SET_PARA_TIME,  VOS_NULL_PTR,   AT_NOT_SET_TIME,    At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
    (VOS_UINT8*)"^LTEMCS",       (VOS_UINT8*)"(0-1)"},

    {AT_CMD_LTETDDSUBFRAME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,  AT_QryTddPara,   AT_QRY_PARA_TIME,    VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
    (VOS_UINT8*)"^LTETDDSUBFRAME",       VOS_NULL_PTR},
};
/*lint +e64  +e133 +e10*/

VOS_UINT32 At_RegisterExPrivateMbbCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_astAtPrivateMbbCmdTbl, sizeof(g_astAtPrivateMbbCmdTbl)/sizeof(g_astAtPrivateMbbCmdTbl[0]));
}

VOS_UINT32  AT_RcvMmaCfunSimStatusInd(
    VOS_VOID                           *pMsg)
{
    MODEM_ID_ENUM_UINT16                            enModemId;
    TAF_MMA_CFUN_USIM_STATUS_IND_STRU              *pstCfunUsimStatusInd;
    AT_USIM_INFO_CTX_STRU                          *pstUsimInfoCtx = VOS_NULL_PTR;

    if (VOS_NULL_PTR == pMsg)
    {
        return VOS_ERR;
    }
    /* 初始化 */
    pstCfunUsimStatusInd = (TAF_MMA_CFUN_USIM_STATUS_IND_STRU*)pMsg;

    enModemId = AT_GetModemIDFromPid(pstCfunUsimStatusInd->ulSenderPid);

    if (enModemId >= MODEM_ID_BUTT)
    {
        return VOS_ERR;
    }

    pstUsimInfoCtx = AT_GetUsimInfoCtxFromModemId(enModemId);

    /*把C核传过来的SIM状态更新给A核的全局变量*/
    pstUsimInfoCtx->enCardStatus = pstCfunUsimStatusInd->ucSimServiceStatus;
    return VOS_OK;
}




