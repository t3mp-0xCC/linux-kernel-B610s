


#ifndef __MBB_AT_DEVICE_CMD_H__
#define __MBB_AT_DEVICE_CMD_H__

/*****************************************************************************
  1 头文件
*****************************************************************************/
#include "vos.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#if(FEATURE_ON == MBB_COMMON)


/* ^PSTANDBY命令解MAIN锁接口 */
#define LOCK_BUF_LEN       32

typedef enum
{
    APP_MAIN_LOCK,
    APP_MAIN_UNLOCK,
} app_main_lock_e;

static const char *off_state = "mem";
static const char *on_state = "on";
static const char *screen_state_path = "/sys/power/autosleep";

typedef struct
{
    unsigned int sim_flag;       /*sim 卡设置检测标志*/
    unsigned int powerkey_flag;  /*powerkey 设置检测标志*/
    unsigned int wpskey_flag;    /*wpskey 设置检测标志*/
    unsigned int resetkey_flag;  /*resetkey 设置检测标志*/
    unsigned int usb_flag;       /*usb 设置检测标志*/
    unsigned int eth_flag;       /*eth 设置检测标志*/
    unsigned int levelkey_flag;  /* levelkey 设置检测标志*/
    unsigned int modekey_flag;  /* modekey 设置检测标志*/
}SET_MMITEST_ST;

#if(FEATURE_ON == MBB_USB)
typedef enum _port_Type
{
    MODEMNUM       = 0,
    NDISNUM        = 1,
    PCUINUM,
    GPSNUM,
    MAXPORTTYPENUM
}port_Type;

extern VOS_UINT8 gportTypeNum[MAXPORTTYPENUM];
#endif


/*****************************************************************************
  对外接口
*****************************************************************************/

#if(FEATURE_ON == MBB_SIMLOCK_FOUR)
VOS_INT32 AT_QryOemLockEnable(VOS_VOID);
#endif

VOS_UINT32 At_RegisterDeviceMbbCmdTable(VOS_VOID);

VOS_UINT32 At_WriteVersionINIToDefault(
    AT_CUSTOMIZE_ITEM_DFLT_ENUM_UINT8   enCustomizeItem
);

VOS_UINT32 At_WriteWebCustToDefault(
    AT_CUSTOMIZE_ITEM_DFLT_ENUM_UINT8   enCustomizeItem
);

VOS_UINT32 At_CheckWiwepHex();

void version_info_build(VOS_UINT8 ucIndex, VOS_UINT8  *pucKey, VOS_UINT8  *pucValue);
void version_info_fill(VOS_UINT8  *pucDes, VOS_UINT8  *pucSrc);

int set_screen_state(app_main_lock_e on);
int set_key_press_event(VOS_VOID);

VOS_UINT32 Mbb_AT_SetWiFiRxPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiEnable(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiModePara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiBandPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiFreqPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiRatePara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiPowerPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiTxPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWiFiPacketPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWifiInfoPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetWifiPaRangePara(VOS_VOID);

VOS_UINT32 Mbb_AT_SetTmmiPara(VOS_VOID);
VOS_UINT32 Mbb_AT_SetChrgEnablePara(VOS_VOID);

#if (FEATURE_ON == MBB_MODULE_PM)
TAF_UINT32   Mbb_At_SetCurcPara(TAF_VOID);
#endif
VOS_UINT32  Mbb_AT_SetPhyNumPara(AT_PHYNUM_TYPE_ENUM_UINT32 enSetType, MODEM_ID_ENUM_UINT16 enModemId);

#if(FEATURE_ON == MBB_USB)
VOS_VOID At_SecCheckSamePortNUM(VOS_UINT32  ucTempnum);
VOS_VOID clearportTypeNum(VOS_VOID);
#endif

TAF_UINT32  Mbb_AT_SetFDac_Para_Valid(TAF_VOID);

VOS_UINT32 Mbb_AT_QryTmmiPara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryChrgEnablePara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWifiPaRangePara (VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWiFiPacketPara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWiFiRxPara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWiFiRatePara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWiFiFreqPara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWiFiBandPara(VOS_UINT8 ucIndex);
VOS_UINT32 Mbb_AT_QryWiFiModePara(VOS_UINT8 ucIndex);
VOS_UINT32  Mbb_At_QryVersion(VOS_UINT8 ucIndex );
TAF_UINT32 Mbb_At_QryCurcPara(TAF_UINT8 ucIndex);


VOS_UINT32 SetWiFiSsidParaCheck(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetWiFiSsidPara(VOS_UINT8 ucIndex);
VOS_UINT32 SetWiFiKeyParaCheck(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetWiFiKeyPara(VOS_UINT8 ucIndex);
#if (FEUATRE_ON == MBB_NFC)
VOS_UINT32 AT_SetNFCCFGPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryNFCCFGPavars(VOS_UINT8 ucIndex);
VOS_UINT32 At_TestNFCCFGPara(VOS_UINT8 ucIndex);
#endif


#endif/*#if(FEATURE_ON == MBB_COMMON)*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


#endif /*__MBB_AT_DEVICE_CMD_H__*/
