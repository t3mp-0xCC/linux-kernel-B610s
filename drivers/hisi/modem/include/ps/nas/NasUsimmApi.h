
#ifndef _NAS_USIMMAPI_H_
#define _NAS_USIMMAPI_H_

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "UsimPsInterface.h"
#include "siapppih.h"
#include "siapppb.h"
#include "siappstk.h"
#include "NasNvInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define NAS_USIMM_CARD_TYPE_INVALID         (0xFF)
#define NAS_USIMM_PIN_TYPE_INVALID          (0xFF)
#define NAS_USIMM_PIN_ENABLE_FLG_INVALID    (0xFF)
#define NAS_USIMM_PIN_NEED_FLG_INVALID      (0xFF)
#define NAS_USIMM_PIN_NUM_INVALID           (0xFF)

#define NAS_MAX_AUTH_RAND_LEN               (16)
#define NAS_MAX_AUTH_AUTN_LEN               (16)

/* USIM interface has removed the following define, here add them */
#define USIM_GAS_AUTH_FAIL         (2)  /* GSM authentication failure */
#define USIM_AUTH_FAIL_UNKNOWN     (3)
/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/



enum NAS_USIMM_SERVICES_TYPE_ENUM
{
    NAS_USIM_SVR_NONE           = 0,

    NAS_USIM_SVR_LOCAL_PHONEBOOK= 1,    /* Local Phonebook, UADF/Phonebook/ADN  */
    NAS_USIM_SVR_FDN,                   /* Forbidden Dialling Numbers           */
    NAS_USIM_SVR_EXT2,                  /* Extension 2                          */
    NAS_USIM_SVR_SDN,                   /* Service Dialling Numbers             */
    NAS_USIM_SVR_EXT3,                  /* Extension 3                          */
    NAS_USIM_SVR_BDN,                   /* Barred Dialing Numbers               */
    NAS_USIM_SVR_EXT4,                  /* Extension 4                          */
    NAS_USIM_SVR_OCI_OCT,               /* Outgoing Call information/timer      */

    NAS_USIM_SVR_ICI_ICT        = 9,    /* Incoming call information/timer      */
    NAS_USIM_SVR_SMS,                   /* Short Message Services               */
    NAS_USIM_SVR_SMSR,                  /* Short Message Status Reports         */
    NAS_USIM_SVR_SMSP,                  /* SMS Parameters                       */
    NAS_USIM_SVR_AOC,                   /* Advice of Charge                     */
    NAS_USIM_SVR_CCP,                   /* Configuration Capability Params      */
    NAS_USIM_SVR_CBMI,                  /* Cell Broadcast Message Identifier    */
    NAS_USIM_SVR_CBMIR,                 /* Cell Broadcast Message Identifiers Range*/

    NAS_USIM_SVR_GID1           = 17,   /* Group Identifier 1                   */
    NAS_USIM_SVR_GID2,                  /* Group Identifier 2                   */
    NAS_USIM_SVR_SPN,                   /* Service Provider Name                */
    NAS_USIM_SVR_UPLMN_SEL_WACT,        /* User controlled PLMN Selector with Access Technology */
    NAS_USIM_SVR_MSISDN,                /* MSISDN                               */
    NAS_USIM_SVR_IMAGE,                 /* Image (IMG)                          */
    NAS_USIM_SVR_SOLSA,                 /* Support of Local Service Area        */
    NAS_USIM_SVR_ENH_ML_SVC,            /* Enhanced Multi Level precedence and Premption Service */

    NAS_USIM_SVR_AA_EMLPP       = 25,   /* Automatic Answer for eMLPP           */
    NAS_USIM_SVR_RFU1,                  /* RFU                                  */
    NAS_USIM_SVR_GSM_ACCESS_IN_USIM,    /* USIM ADF Access to USIM files for GSM Roaming */
    NAS_USIM_SVR_DATA_DL_SMSPP,         /* Data download via SMS-PP             */
    NAS_USIM_SVR_DATA_DL_SMSCB,         /* Data download via SMS-CB             */
    NAS_USIM_SVR_CALL_CONTROL,          /* Call Control                         */
    NAS_USIM_SVR_MOSMS_CONTROL,         /* MO SMS control                       */
    NAS_USIM_SVR_RUN_AT_CMD,            /* RUN AT COMMAND command               */

    NAS_USIM_SVR_SET_ONE        = 33,   /* shall be set to1                     */
    NAS_USIM_SVR_EST,                   /* Enable Service Table in USIM         */
    NAS_USIM_SVR_ACL,                   /* APN Control list                     */
    NAS_USIM_SVR_DEPERSON_KEYS,         /* Depersonalization Keys               */
    NAS_USIM_SVR_COOP_NETWK_LIST,       /* Co-operative Network List            */
    NAS_USIM_SVR_GSM_SECURITY_CONTEXT,  /* GSM Security Context                 */
    NAS_USIM_SVR_CPBCCH,                /* CP BCCH                              */
    NAS_USIM_SVR_INV_SCAN,              /* Investigation Scan                   */

    NAS_USIM_SVR_MEXE           = 41,   /* MExE info                            */
    NAS_USIM_SVR_OPLMN_SEL_WACT,        /* Operator controlled PLMN Selector with Access Technology */
    NAS_USIM_SVR_HPLMN_WACT,            /* HPLMN Selector with Access Technology*/
    NAS_USIM_SVR_EXT5,                  /* Extension 5                          */
    NAS_USIM_SVR_PLMN_NTWRK_NAME,       /* PLMN Network Name                    */
    NAS_USIM_SVR_OPLMN_LIST,            /* Operator PLMN list                   */
    NAS_USIM_SVR_MAILBOX_DIAL,          /* Mailbox dialling numbers             */
    NAS_USIM_SVR_MSG_WAIT,              /* Message Wait indication              */

    NAS_USIM_SVR_CALL_FWD_IND   = 49,   /* Call Forward indication status       */
    NAS_USIM_SVR_RFU2,                  /* RFU                                  */
    NAS_USIM_SVR_SP_DISP_INFO,          /* Service Provider Display Information */
    NAS_USIM_SVR_MMS,                   /* Multimedia Messaging Service         */
    NAS_USIM_SVR_EXT8,                  /* Extension 8                          */
    NAS_USIM_SVR_GPRS_USIM,             /* Call control on GPRS by USIM         */
    NAS_USIM_SVR_MMS_USR_P,             /* MMS User Connectivity Parameters     */
    NAS_USIM_SVR_NIA,                   /* Network's indication of alerting in the MS */

    NAS_USIM_SVR_VGCS_GID_LIST  = 57,   /* VGCS Group Identifier List           */
    NAS_USIM_SVR_VBS_GID_LIST,          /* VBS Group Identifier List            */
    NAS_USIM_SVR_PSEUDONYM,             /* Pseudonmy                            */
    NAS_USIM_SVR_UPLMN_WLAN_ACC,        /* User Controlled PLMNselector for WLAN access */
    NAS_USIM_SVR_OPLMN_WLAN_ACC,        /* Opeator Controlled PLMN selector for WLAN access */
    NAS_USIM_SVR_USER_CTRL_WSID,        /* User controlled WSID list            */
    NAS_USIM_SVR_OPER_CTRL_WSID,        /* Operator controlled WSID list        */
    NAS_USIM_SVR_VGCS_SECURITY,         /* VGCS Security                        */

    NAS_USIM_SVR_VBS_SECURITY   = 65,   /* VBS Security                         */
    NAS_USIM_SVR_WLAN_REAUTH_ID,        /* WLAN Reauthentication Identity       */
    NAS_USIM_SVR_MM_STORAGE,            /* Multimedia Messaging Storage         */
    NAS_USIM_SVR_GBA,                   /* Generic Bootstrapping Architecture   */
    NAS_USIM_SVR_MBMS_SECURITY,         /* MBMS security                        */
    NAS_USIM_SVR_DATA_DL_USSD,          /* Data Downlod via USSD and USSD application mode */
    NAS_USIM_SVR_EHPLMN,                /* EHPLMN                               */
    NAS_USIM_SVR_TP_AFTER_UICC_ACT,     /* additional TERMINAL PROFILE after UICC activation */

    NAS_USIM_SVR_EHPLMN_IND     = 73,   /*Equivalent HPLMN Presentation Indication*/
    NAS_USIM_SVR_RPLMN_LACT,            /* RPLMN Last used Access Technology    */
    NAS_USIM_SVR_OMA_BCAST_PROFILE,     /* OMA BCAST Smart Card Profile         */
    NAS_USIM_SVR_GBAKEY_EM,             /* GBA-based Local Key Establishment Mechanism */
    NAS_USIM_SVR_TERMINAL_APP,          /* Terminal Applications                */
    NAS_USIM_SVR_SPN_ICON,              /* Service Provider Name Icon           */
    NAS_USIM_SVR_PNN_ICON,              /* PLMN Network Name Icon               */
    NAS_USIM_SVR_CONNECT_SIM_IP,        /* Connectivity Parameters for USIM IP connections */

    NAS_USIM_SVR_IWLEN_ID_LIST  = 81,   /* Home I-WLAN Specific Identifier List */
    NAS_USIM_SVR_IWLEN_EHPLMN_IND,      /* I-WLAN Equivalent HPLMN Presentation Indication */
    NAS_USIM_SVR_IWLEN_HPLMN_IND,       /* I-WLAN HPLMN Priority Indication     */
    NAS_USIM_SVR_IWLEN_RPLMN,           /* I-WLAN Last Registered PLMN          */
    NAS_USIM_SVR_EPS_INFO,              /* EPS Mobility Management Information  */
    NAS_USIM_SVR_CSG_COR_IND,           /* Allowed CSG Lists and corresponding indications */
    NAS_USIM_SVR_CALL_CTRL_EPS,         /* Call control on EPS PDN connection by USIM */
    NAS_USIM_SVR_HPLMN_DIR_ACCESS,      /* HPLMN Direct Access                  */

    NAS_USIM_SVR_ECALL_DATA     = 89,   /* eCall Data                           */
    NAS_USIM_SVR_OP_CSG,                /* Operator CSG Lists and corresponding indications */
    NAS_USIM_SVR_SM_VOER_IP,            /* Support for SM-over-IP               */

    NAS_USIM_SVR_CSG_CTRL,              /* Support of CSG Display Control */

    /* only SIM card*/
    NAS_USIM_SVR_CHV1_DISABLE,          /* CHV1 disabled function               */
    NAS_USIM_SVR_ADN,                   /* GSM Abbreviated Dialling Numbers     */
    NAS_USIM_SVR_PLMN_SELECTOR,         /* PLMN Selector                        */
    NAS_USIM_SVR_EXT1,                  /* Extension 1                          */
    NAS_USIM_SVR_LND,                   /* Last Number Dialled                  */
    NAS_USIM_SVR_MENU_SEL,              /* Menu Selection                       */

    NAS_USIM_SVR_PROACTIVE_SIM,         /* Proactive SIM                        */
    NAS_USIM_SVR_MO_SMS_BY_SIM,         /* Mobile Originated Short Message control by SIM    */
    NAS_USIM_SVR_GPRS,                  /* GPRS                                              */
    NAS_USIM_SVR_USSD,                  /* USSD String Data object supported in Call Control */
    NAS_USIM_SVR_EXT_CCP,               /* extended capability configuration param - in GSM  */

    NAS_USIM_SVR_MAX_SERVICE
};
typedef VOS_UINT32  NAS_USIMM_SERVICES_TYPE_ENUM_UINT32;

/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/

/*****************************************************************************
  7 STRUCT����
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucVsimCtrlFlg;
    VOS_UINT8                           aucReserved1[3];
}TAF_USIMM_VSIM_INFO_STRU;



typedef struct
{
    NAS_USIMM_SERVICES_TYPE_ENUM_UINT32  enSrvType;
    UICC_SERVICES_TYPE_ENUM_UINT32       enSimSrvType;
    UICC_SERVICES_TYPE_ENUM_UINT32       enUsimSrvType;
}NAS_USIMM_SRVICES_TYPE_STRU;



typedef struct
{
    USIMM_CARDAPP_ENUM_UINT32   enAppType;      /* Ӧ������ */
    VOS_UINT16                  usEfId;         /* �ļ�ID */
    VOS_UINT8                   ucRecordNum;    /* �ļ���¼�ţ��������ļ���0 */
    VOS_UINT8                   ucRsv;          /* ���� */
    VOS_UINT32                  ulEfLen;        /* �������ݳ��� */
    VOS_UINT8                  *pucEfContent;   /* ������������ */
}NAS_USIMM_UPDATEFILE_INFO_STRU;


typedef struct
{
    USIMM_CARDAPP_ENUM_UINT32   enAppType;      /* Ӧ������ */
    VOS_UINT16                  usEfId;         /* �ļ�ID */
    VOS_UINT8                   ucRecordNum;    /* �ļ���¼�ţ��������ļ���0 */
    VOS_UINT8                   ucRsv;          /* ���� */
}NAS_USIMM_GETFILE_INFO_STRU;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
VOS_UINT32 NAS_USIMMAPI_SetFileReq(
    VOS_UINT32                          ulSenderPid,
    VOS_UINT32                          ulSendPara,
    NAS_USIMM_UPDATEFILE_INFO_STRU     *pstSetFileInfo
);

VOS_UINT32 NAS_USIMMAPI_GetFileReq(
    VOS_UINT32                          ulSenderPid,
    VOS_UINT32                          ulSendPara,
    NAS_USIMM_GETFILE_INFO_STRU        *pstGetFileInfo
);

VOS_VOID NAS_USIMMAPI_AuthReq(
    VOS_UINT32                          ulSenderPid,
    VOS_UINT8                           ucChallengeType,
    VOS_UINT8                          *pucRand,
    VOS_UINT8                          *pucAutn,
    VOS_UINT8                           ucOpId
);

VOS_UINT32 NAS_USIMMAPI_PinReq(
    VOS_UINT32                          ulSenderPid,
    VOS_UINT8                           ucCmdType,
    VOS_UINT8                           ucPINType,
    VOS_UINT8                          *pucOldPIN,
    VOS_UINT8                          *pucNewPIN
);

VOS_UINT32  NAS_USIMMAPI_MaxEFRecordNumReq(
    VOS_UINT32                          ulSenderPid,
    USIMM_CARDAPP_ENUM_UINT32           enAppType,
    VOS_UINT16                          usEFid
);

VOS_UINT32  NAS_USIMMAPI_IsServiceAvailable(
    NAS_USIMM_SERVICES_TYPE_ENUM_UINT32 enService
);

VOS_UINT32 NAS_GetOamReceivePid(VOS_UINT32 ulRcvPid);


VOS_BOOL  NAS_USIMMAPI_IsTestCard(VOS_VOID);

VOS_UINT32  NAS_USIMMAPI_GetPinStatus(
    VOS_UINT8                          *pucType,
    VOS_UINT8                          *pucBeEnable,
    VOS_UINT8                          *pucBeNeed,
    VOS_UINT8                          *pucNum
);

VOS_UINT32 NAS_USIMMAPI_GetCardType(
    VOS_UINT8                          *pucCardStatus,
    VOS_UINT8                          *pucCardType
);

VOS_UINT32  NAS_USIMMAPI_GetCachedFile(
    VOS_UINT32                          ulFileID,
    VOS_UINT32                         *pulDataLen,
    VOS_UINT8                         **ppucData,
    USIMM_CARDAPP_ENUM_UINT32           enAppType
);

VOS_UINT32  NAS_USIMMAPI_GetCardIMSI(
    VOS_UINT8                          *pucImsi
);

VOS_VOID  NAS_USIMMAPI_BdnQuery(
    VOS_UINT32                          *pulState
);

VOS_VOID  NAS_USIMMAPI_FdnQuery(
    VOS_UINT32                         *pulUsimState,
    VOS_UINT32                         *pulCsimState
);

VOS_UINT32 NAS_USIMMAPI_RestrictedAccessReq(
    VOS_UINT32                          ulSenderPid,
    VOS_UINT32                          ulSendPara,
    USIMM_RACCESS_REQ_STRU             *pstData
);

VOS_UINT32 NAS_USIMMAPI_ResetReq(
    VOS_UINT32                          ulSenderPid,
    USIMM_STK_COMMAND_DETAILS_STRU     *pstCMDDetail,
    USIMM_RESET_INFO_STRU              *pstRstInfo
);


VOS_UINT32  NAS_USIMMAPI_GetPinRemainTime(
    USIMM_PIN_REMAIN_STRU              *pstRemainTime
);

VOS_UINT32 NAS_PIHAPI_RegUsimCardStatusIndMsg(
    VOS_UINT32                          ulRegPID
);

VOS_UINT32 NAS_PIHAPI_DeregUsimCardStatusIndMsg(
    VOS_UINT32                          ulDeregPID
);

VOS_VOID NAS_PIHAPI_RegCardRefreshIndMsg(
    VOS_UINT32                          ulRegPID
);

VOS_UINT32 NAS_PIHAPI_DeregCardRefreshIndMsg(
    VOS_UINT32                          ulDeregPID
);

VOS_VOID NAS_PBAPI_GetEccNumber(
    SI_PB_ECC_DATA_STRU                *pstEccData
);

VOS_UINT32 NAS_PBAPI_FdnNumCheck(
    VOS_UINT32                          ulPid,
    VOS_UINT32                          ulContextIndex,
    VOS_UINT32                          ulSendPara,
    SI_PB_FDN_NUM_STRU                 *pstFdnInfo
);

VOS_UINT32 NAS_STKAPI_EnvelopeRsp_Decode(
    SI_STK_ENVELOPE_TYPE_UINT32         enDataType,
    VOS_UINT32                          ulDataLen,
    VOS_UINT8                          *pucCmdData,
    SI_STK_ENVELOPE_RSP_STRU           *pstRspData
);

VOS_VOID NAS_STKAPI_EnvelopeRspDataFree(
    SI_STK_ENVELOPE_RSP_STRU           *pstData
);

VOS_UINT32 NAS_STKAPI_EnvelopeDownload(
    VOS_UINT32                          ulSenderPid,
    VOS_UINT32                          ulClientId,
    SI_STK_ENVELOPE_STRU               *pstENStru
);

/*
    NAS_STKAPI_MtCallEvent
*/




VOS_UINT32 NAS_USIMMAPI_DeactiveCardReq(
    VOS_UINT32 ulSenderPid
);

VOS_VOID NAS_STKAPI_CCResultInd(
    SI_STK_ENVELOPE_RSP_STRU           *pstRspData
);

VOS_VOID NAS_STKAPI_SMSCtrlResultInd(
    SI_STK_ENVELOPE_RSP_STRU           *pstRspData
);



VOS_UINT32 NAS_VSIM_IsRequireVsimCtrl(VOS_VOID);
VOS_VOID NAS_VSIM_InitVsimCfgInfo(VOS_VOID);


VOS_VOID NAS_PBAPI_QryECallNumber(
    VOS_UINT32                          ulPid,
    SI_PB_STORAGE_ENUM_UINT32           enPbType,
    VOS_UINT8                           ucListLen,
    VOS_UINT8                          *pucList
);


VOS_VOID NAS_USIMMAPI_SetModem0PlatRat(
    PLATAFORM_RAT_CAPABILITY_STRU      *pstModem0PlatRat
);

VOS_VOID NAS_USIMMAPI_InitModem0PlatRat(
    VOS_VOID
);

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_VOID NAS_PBAPI_GetXeccNumber(
    SI_PB_ECC_DATA_STRU                *pstEccData
);
#endif

#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of NasUsimmApi.h */
