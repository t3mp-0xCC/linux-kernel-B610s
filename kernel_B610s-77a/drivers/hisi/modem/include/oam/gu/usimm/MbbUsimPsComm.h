


#ifndef _MBB_USIMM_PS_COMM_H_
#define _MBB_USIMM_PS_COMM_H_

/*****************************************************************************
1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "UsimPsInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if(FEATURE_ON == MBB_COMMON)
#include "AtMnInterface.h"
#endif
#pragma pack(4)
#if (FEATURE_ON == MBB_WPG_COMMON)
/*******************************************************************************
2 宏定义
*******************************************************************************/
/* 快速开关机在PIN使能情况下执行复位SIM卡后操作结果回复 */
/* 需要注意该id不会被覆盖，check USIMM_SETMUTILFILE_CNF*/
#define MBB_USIMM_RESETCARD_CNF                  80 

#define RELIABILITY_STATUS_NO_NEED_DATA    (0x0c)
#define RELIABILITY_JUMP_TAB_NUM                   (4)
/*When RELIABILITY_MODE is zero RELIABILITY Module be executed */
#define RELIABILITY_MODE                                    (0)
#define RELIABILITY_NV_INITVALUE                      (0xFFFF)
#define TAF_GID1_EFID                                           (0x6F3E)
#define RELIABILITY_HDL(A)  g_stReliabilityHdlJumpTbl[(A) % (RELIABILITY_JUMP_TAB_NUM)].pReliabilityFunTbl.pProcFun
#define EFSPDI                                  0x6FCD

/*******************************************************************************
3 枚举定义
*******************************************************************************/
enum UE_RELIABILITY_ENUM
{
    RELIABILITY_TOP = 0,          
    RELIABILITY_NETWORK_REFUSE_INDEX,   /*Location updating not accepted */
    RELIABILITY_UICC_DITHER_INDEX,   /* UICC dithering*/
    RELIABILITY_ACCBARRED_INDEX,/*Access barred because of access class control*/
    RELIABILITY_BOTTOM
};

enum  RELIABLITY_CAUSE_ENUM
{
    RELIABILITY_CAUSE_TOP = 0,
    RELIABILITY_CAUSE_PLMN_SELECTION = 7,  /*corresponding to NAS_MMC_ADDITIONAL_ACTION_OPTIONAL_PLMN_SELECTION*/
    RELIABILITY_CAUSE_ROAM_NOT_ALLOW = 13,  /*Roaming not allowed in this location area*/
    RELIABILITY_CAUSE_NO_SUITABL_CELL = 15,  /*No Suitable Cells In Location Area*/
    RELIABILITY_UICC_DITHER_TEST = 33, /*Test UICC dither*/
    RELIABILITY_CAUSE_BOTTOM
};
enum RELIABLITY_MASK_ENUM
{
    RELIABILITY_MASK_TOP = 0,/*0000 0000 0000 0000*/
    RELIABILITY_MASK_CAUSE7 =  1,/*0000 0000 0000 0001*/
    RELIABILITY_MASK_CAUSE12 = 2,/*0000 0000 0000 0010*/
    RELIABILITY_MASK_ACCESSBAR = 4,/*0000 0000 0000 0100*/
    RELIABILITY_MASK_UICCDITHER = 8, /*0000 0000 0000 1000*/
    RELIABILITY_MASK_BOTTOM/*1111 1111 1111 1111*/
};

/* 用于VDF SOBE定制特性，表明SIM卡是否更新 */
/*******************************************************************************
4 struct定义
*******************************************************************************/
typedef enum
{
    GID1_LOCK_DISABLE = 0,       /*GID1锁卡未使能*/ 
    GID1_LOCK_ENABLE,            /*GID1锁卡使能*/  
    GID1_LOCK_BUTT
}GID1_LOCK_ENUM;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    VOS_UINT32                          ulResult;
}PS_USIM_RESET_CARD_CNF_STRU;

#if(FEATURE_ON == MBB_COMMON)
/*****************************************************************************
 结构名    : USIMM_RcvAtRsimSimStaQryReq_CNF_STRU
 结构说明  : AT^RSIM处理回复消息结构
*****************************************************************************/
typedef struct
{
    USIMM_CMDHEADER_REQ_STRU                          stMsgHeader;
    AT_APPCTRL_STRU                                   stAtAppCtrl;
    VOS_INT32                                         ulRelt;
    VOS_INT32                                         simStatus;
}USIMM_RCV_RSIM_STA_CNF_STRU;

typedef struct
{
    USIMM_CMDHEADER_REQ_STRU                          stMsgHeader;
    AT_APPCTRL_STRU                                   stAtAppCtrl;
}USIMM_QUERYCARD_REQ_STRU;
#endif

/*****************************************************************************
6 消息定义
*****************************************************************************/

/*****************************************************************************
7 api声明
*****************************************************************************/
#if (FEATURE_ON == MBB_WPG_RELIABILITY)
typedef struct
{
    VOS_UINT32        ucReliabilityNvEnable;
     /* 通信协议协议USIMM 可靠性保护功能掩码 */
    VOS_UINT32       ulUiccDitherEnable;
}RELIABLITY_USIMM_BIT_MAP_STRU;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32         ulPsRegFailCauseOrAction;
    VOS_UINT32         ulCardInOutStatus;
    VOS_UINT16         usReliabilityImsiVal;
    VOS_UINT16         usReliabilityExc;
}RELIABILITY_CBPARA_STRU;

typedef    VOS_UINT16  (*pReliabilityHdl)(RELIABILITY_CBPARA_STRU *);
typedef struct
{
    pReliabilityHdl                pProcFun;
}RELIABILITY_HANDLE;

typedef struct
{
    RELIABILITY_HANDLE           pReliabilityFunTbl;
    VOS_UINT16                   ulReliabilityType;
}RELIABILITY_HANDLE_TBL_ST;

extern VOS_VOID USIMM_DitherTest(VOS_UINT8 ucImsiTest, VOS_UINT8 ucStatusTest);
extern RELIABILITY_HANDLE_TBL_ST g_stReliabilityHdlJumpTbl[RELIABILITY_JUMP_TAB_NUM];
extern VOS_UINT16  NAS_MMC_ReliabilityNetworkRefusehdl(RELIABILITY_CBPARA_STRU *stPara);
extern VOS_UINT16  USIMM_ReliabilityUiccDitherhdl(RELIABILITY_CBPARA_STRU *stPara);
extern VOS_UINT16  NAS_MMC_ReliabilityPlatInterCause(RELIABILITY_CBPARA_STRU *stPara);
extern VOS_UINT16  NAS_MMC_ReliabilityAccessBarred(VOS_UINT16 ulCause);
extern VOS_UINT16  NAS_MMC_ReliabilityGprsServiceNotAllowed(VOS_UINT16 ulCause);
extern VOS_UINT16  NAS_MMC_ReliabilityLaNotAllowed(VOS_UINT16 ulCause);
#endif/*FEATURE_ON == MBB_WPG_RELIABILITY*/

#if (FEATURE_ON == MBB_WPG_CFUN) 
extern VOS_UINT32 USIMM_ResetCardReq(VOS_UINT32 ulSenderPid);
#endif/*FEATURE_OFF == MBB_WPG_CFUN*/

#if (FEATURE_ON == MBB_WPG_DUAL_IMSI)
enum USIMM_IS_REFRESHED_ENUM
{
    USIMM_NO_REFRESH                = 0,       /* SIM卡没有更新*/
    USIMM_IS_REFRESHED             = 1,         /* SIM卡已经更新*/
};
typedef VOS_UINT8      USIMM_IS_REFRESHED_ENUM_UINT8;

typedef struct
{
    USIMM_IS_REFRESHED_ENUM_UINT8                           IsRefreshed;
    USIMM_REFRESH_TYPE_ENUM_UINT32                          RefreshType;
}TAF_SIM_REFRESH_INFO_STRU;
extern TAF_SIM_REFRESH_INFO_STRU stSIMRefresh;
#endif/*FEATURE_ON == MBB_WPG_DUAL_IMSI*/
extern VOS_INT32 vos_printf_dump(VOS_UINT16 ulLen,VOS_UINT8 *pStr);

/**************************************************************************
8 OTHERS定义
*****************************************************************************/
#endif/*FEATURE_ON == MBB_WPG_COMMON*/

#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif                                                                          
/* __cpluscplus                             */

#endif

