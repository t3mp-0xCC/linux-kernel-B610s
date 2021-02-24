/******************************************************************************
 */
/*
 */
/*                  ��Ȩ���� (C), 2001-2011, ��Ϊ�������޹�˾
 */
/*
 */
/******************************************************************************
 */
/*  �� �� ��   : at_lte_eventreport_mbb.h
 */
/*  �� �� ��   : V1.0
 */

/*  ��������   : 2014-06-09
 */
/*  ��������   : ��Ʒ�����м��޸�at�����ļ�
 */
/*  �����б�   :
 */
/*  �޸���ʷ   :
 */
/*  1.��    �� : 2011-03-10
 */

/*    �޸����� : �����ļ�
 */
/*
 */
/******************************************************************************
 */
/*************************************************************************************
*************************************************************************************/



#ifndef _MBB_AT_GU_COMMON_H__
#define _MBB_AT_GU_COMMON_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "AtCtx.h"
#include "AtInputProc.h"
#include "product_nv_def.h"
#include "product_nv_id.h"
#include "product_config.h"
#include "mdrv.h"
#include "at_lte_common.h"
#include "AtEventReport.h"
#include "LPsNvInterface.h"
#include "ComInterface.h"

#if (FEATURE_ON == MBB_FEATURE_MODULE_AT)
#include "GasNvInterface.h"
#endif     /* FEATURE_ON == MBB_FEATURE_MODULE_AT */

#if (FEATURE_ON == MBB_WPG_FREQLOCK)
#include "AtMtaInterface.h"
#endif /* FEATURE_ON == MBB_WPG_FREQLOCK */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (FEATURE_ON == MBB_WPG_COMMON)

#ifdef __MBB_LINT__
#ifndef VOS_CHAR
typedef char VOS_CHAR;
#endif
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/(sizeof((a)[0])))
#endif

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define AT_HCSQ_RAT_NAME_MAX             (255)
#define AT_HCSQ_RSSI_VALUE_MIN           (-120)
#define AT_HCSQ_RSSI_VALUE_MAX           (-25)
#define AT_HCSQ_LEVEL_MIN                      (0)
#define AT_HCSQ_RSSI_LEVEL_MAX            (96)   
#define AT_HCSQ_RSCP_VALUE_MIN           (-120)
#define AT_HCSQ_RSCP_VALUE_MAX           (-25)
#define AT_HCSQ_RSCP_LEVEL_MAX            (96 )
#define AT_HCSQ_ECIO_VALUE_MIN           (-32)
#define AT_HCSQ_ECIO_VALUE_MAX           (0)
#define AT_HCSQ_ECIO_LEVEL_MAX           (65)
#define AT_HCSQ_VALUE_INVALID             (255)
#define MMA_PLMN_ID_LEN                         (6)
#define AT_COPN_LEN_AND_RT                  (7)
#define AT_MSG_7BIT_MASK                       (0x7f)

#define TIME_INFO_DEBUG_VAR      (3000)

#define SYSCFGEX_MODE_AUTO       (0x00)
#define SYSCFGEX_MODE_GSM        (0x01)
#define SYSCFGEX_MODE_WCDMA      (0x02)
#define SYSCFGEX_MODE_LTE        (0x03)
#define SYSCFGEX_MODE_CDMA       (0x04)
#define SYSCFGEX_MODE_TDSCDMA    (0x05)
#define SYSCFGEX_MODE_WIMAX      (0x06)
#define SYSCFGEX_MODE_NOT_CHANGE (0x99)
#define SYSCFGEX_MODE_INVALID    (0xFF)
#define SYSCFGEX_MAX_RAT_STRNUM  (7) /*gulÿ��ģʽ2���ַ��ټ���\0*/

extern AT_DEBUG_INFO_STRU g_stAtDebugInfo;

#if(FEATURE_ON == MBB_WPG_DIAL)
#define MAX_NDIS_NET                                     (8)
#define DEFAULT_MIN_NDIS_NET                             (1)
extern UDI_HANDLE g_ulAtUdiNdisMpdpHdl[MAX_NDIS_NET];
extern VOS_UINT8  g_ucPcuiPsCallFlg[TAF_MAX_CID + 1];

#endif   /*FEATURE_ON == MBB_WPG_DIAL*/

#define AT_DATACLASS_MAX                  AT_DATACLASS_DC_HSPAPLUS
#define AT_DATACLASS_BASE_VALUE           (0X01)
#define AT_DATACLASSLTE_MAX                  (0X01)
#define AT_DATACLASS_LTE           (0X01)
/* ϵͳģʽ���Ƶ��ַ������ȣ��ݶ�Ϊ16 */
#define AT_DATACLASS_NAME_LEN_MAX         (16)
#define AT_DATACLASS_NOT_SUPPORT          (0)
#define AT_DATACLASS_SUPPORT              (1)
#define AT_DATACLASS_HSPASTATUS_ACTIVED   (1)
#define AT_DATACLASS_ENASRELINDICATOR_R5  (2)
#define AT_DATACLASS_ENASRELINDICATOR_R6  (3)
#define AT_DATACLASS_ENASRELINDICATOR_R7  (4)
#define AT_DATACLASS_ENASRELINDICATOR_R8  (4)
#if (FEATURE_ON == MBB_WPG_COMMON)
#define AT_IPV6_FIRST_VERSION         (1)
#define AT_IPV6_SECOND_VERSION        (2)
#endif/*FEATURE_ON == MBB_WPG_COMMON*/

#if(FEATURE_ON == MBB_WPG_PCM)
#define AT_CMD_CLVL_CONST               (13)
#endif    /* FEATURE_ON == MBB_WPG_PCM */


#if (FEATURE_ON == MBB_FEATURE_MODULE_AT)
#define            HCSTUB_NV_MAX_NUM     (4)
typedef TAF_UINT8 AT_CMD_Q_TYPE;
#define AT_Q_NORMAL_MODE            0
#define AT_Q_QUIET_MODE             1
#define AT_NETSCAN_RAT_ONLY             (1)
/* NETSCAN �л���ģ�󣬵ȴ�פ�����ʱ�� */
#define AT_NETSCAN_WAIT_CAMP_ON_TIMER_LEN (10*1000)
#define MAX_ASCII_LENGTH 3

#define PORT_VALUE_RESERVED 0xFF
#define INITIAL_PORT_VALUE 0x00
#define PORT_VALUE_PCSC 0x04
#define PORT_VALUE_GPS 0x14

#define MAX_CONFIG_NUM 0x03
#define MULTI_CONFIG_FIRST 0x01
#define MULTI_CONFIG_SECOND 0x02
#define MULTI_CONFIG_THIRD 0x03
#endif /* FEATURE_ON == MBB_FEATURE_MODULE_AT */

#if( (FEATURE_ON == MBB_FEATURE_MODULE_AT)|| (FEATURE_ON == MBB_WPG_LTE_NETSCAN))
/* TD_SCDMA\LTE��ȫ��Ƶ�� */
#define AT_ALL_TL_BAND                  (0xFFFFFFFF)
/* PSC�������ЧֵΪ511 */
#define AT_NETSCAN_PSC_MAX_VALUE        (511)
/* PID�������ЧֵΪ503 */
#define AT_NETSCAN_PID_MAX_VALUE        (503)
#endif

#if(FEATURE_ON == MBB_WPG_AC)
#define  AC_REPORT_DISABLED             (0)        /* �ر� AC �����ϱ� */
#define  AC_REPORT_ENABLED              (1)        /* ���� AC �����ϱ� */
#endif/* FEATURE_ON == MBB_WPG_AC */
#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
#define AT_LED_ALWAYS_ON_OFF_PARA_NUM                       (3)
#define AT_LED_FLICKER_SINGLE_PARA_NUM                      (5)
#define AT_LED_FLICKER_DOUBLE_PARA_NUM                      (7)
#define LED_FLICKER_MAX_NUM_PER_CYCLE                       (2)
#define LED_DEFAULT_FLICKER_TIME                            (0xFF)
#define LED_COLOR_NULL                                      (0)
#define AT_SERVICE_STAT_ANY                                 (0x3FFFFFFF)
#define LED_SERVICE_STATE_DEFAUNT                           (0x00007FFF)
#define AT_LED_SERVICE_STATE_NUM                            (15)
#endif
#if(FEATURE_ON == MBB_NOISETOOL)
#define NOISECFG_PWR_OFFSET  (70)
#define NOISECFG_MAX_PARA_NUM (7)
#define NOISECFG_LTE_MAX_TX_PWR (23)
#define NOISECFG_W_MAX_TX_PWR (24)
#endif/*(FEATURE_ON == MBB_NOISETOOL)*/
#if (FEATURE_ON == MBB_WPG_ROAM)
#define ROAM_STATUS_NOCHANGE         (2)
#define ROAM_MODULE_TO_BL(A)\
    do{\
        A = (A == 1 ? TAF_MMA_ROAM_NATIONAL_ON_INTERNATIONAL_ON\
               : TAF_MMA_ROAM_NATIONAL_OFF_INTERNATIONAL_OFF);\
    }while(0)
#define ROAM_BL_TO_MODULE(B)\
    do{\
        B = (B == TAF_MMA_ROAM_NATIONAL_ON_INTERNATIONAL_ON ? 1 :0);\
    }while(0)
#endif /* FEATURE_ON == MBB_WPG_ROAM */  
/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
#if(FEATURE_ON == MBB_WPG_FD)

enum AT_RABM_M2M_FASTDORM_OPERATION_ENUM
{
    AT_RABM_M2M_SINGLE_FASTDORM = 1,                                          /* ���δ��� */
    AT_RABM_M2M_AUTOM_FASTDORM,                                                 /* �����Զ����� */
    AT_RABM_M2M_STOP_FASTDORM,                                                   /* ֹͣ�Զ����� */
    AT_RABM_M2M_FASTDORM_START_BUTT
};
#endif/*FEATURE_ON == MBB_WPG_FD*/

#if (FEATURE_ON == MBB_WPG_HFEATURESTAT)
enum AT_HFEATURE_ID_ENUM
{
    AT_HFEATURE_SINGLE_PDN = 1,/*Ŀǰ��֧��SINGLE PDN �������Դ�������չ*/
    AT_HFEATURE_BUTT,
};
enum AT_HFEATURE_STATE_ENUM
{
    AT_HFEATURE_NOT_OPEN = 0,/*����δ����*/
    AT_HFEATURE_OPEN,/*���Լ���*/
};
#endif/*FEATURE_ON == MBB_WPG_HFEATURESTAT*/

#if (FEATURE_ON == MBB_FEATURE_MODULE_AT)
#if (FEATURE_ON == MBB_FEATURE_MODULE_AT)

enum AT_NETSCAN_STATUS_ENUM
{
    AT_NETSCAN_STATUS_NULL = 0,                  /* NETSCAN ��ʼ״̬ */
    AT_NETSCAN_STATUS_SYSINFOEX_GET_REQ,         /* ��ѯSYSINFOEXϵͳģʽ״̬ */
    AT_NETSCAN_STATUS_SYSINFOEX_GET_CNF,         /* ��ȡ��SYSINFOEXϵͳģʽ״̬ */
    AT_NETSCAN_STATUS_SYSCFGEX_GET_REQ,          /* ��ѯSYSCFGEX��ǰ����״̬ */
    AT_NETSCAN_STATUS_SYSCFGEX_GET_CNF,          /* ��ȡ��SYSCFGEX��ǰ����״̬ */
    AT_NETSCAN_STATUS_CGCATT_DETACH_REQ,         /* ȥע����������״̬ */
    AT_NETSCAN_STATUS_CGCATT_DETACH_CNF,         /* ȥע������ɹ�״̬ */
    AT_NETSCAN_STATUS_SYSCFGEX_SET_REQ,          /* �л���ģ����״̬ */                                          
    AT_NETSCAN_STATUS_SYSCFGEX_SET_CNF,          /* �л���ģ�ɹ�״̬ */  
    AT_NETSCAN_STATUS_WAIT_CAMP_ON,              /* �л���ģ�󣬵ȴ�פ�������� */
    AT_NETSCAN_STATUS_CAMP_ON,                   /* �л���ģ������פ�������� */
    AT_NETSCAN_STATUS_SET_REQ,                   /* NETSCANɨƵ����״̬ */  
    AT_NETSCAN_STATUS_SET_CNF,                   /* NETSCANɨƵ�ظ�״̬ */  
    AT_NETSCAN_STATUS_CGCATT_ATTACH_REQ,         /* ע����������״̬ */
    AT_NETSCAN_STATUS_CGCATT_ATTACH_CNF,         /* ע������ɹ�״̬ */
    AT_NETSCAN_STATUS_SYSCFGEX_SET_REVERT_REQ,   /* �ָ���ģ����״̬ */ 
    AT_NETSCAN_STATUS_SYSCFGEX_SET_REVERT_CNF,   /* �ָ���ģ�ظ�״̬ */ 
    AT_NETSCAN_STATUS_END,                       /* NETSCAN����״̬ */ 
    AT_NETSCAN_STATUS_BUTT
};
typedef VOS_UINT8 AT_NETSCAN_STATUS_ENUM_UINT8;

/* SYSINFOEX��ѯ����ϵͳģʽ */
enum AT_NETSCAN_SYSINFO_MODE_ENUM
{
    AT_NETSCAN_SYSINFO_MODE_NO_SERVICE = 0,
    AT_NETSCAN_SYSINFO_MODE_GSM,
    AT_NETSCAN_SYSINFO_MODE_CDMA,
    AT_NETSCAN_SYSINFO_MODE_WCDMA,
    AT_NETSCAN_SYSINFO_MODE_TD_SCDMA,
    AT_NETSCAN_SYSINFO_MODE_WIMAX,
    AT_NETSCAN_SYSINFO_MODE_LTE,
    AT_NETSCAN_SYSINFO_MODE_BUTT
};
typedef VOS_UINT8 AT_NETSCAN_SYSINFO_MODE_ENUM_UINT8;
#endif/* FEATURE_ON == MBB_FEATURE_MODULE_AT */
enum A2fCount_ENUM
{
    AT_MBB_CMD_A_LEN_0                  = 0,
    AT_MBB_CMD_A_LEN_1,                  
    AT_MBB_CMD_A_LEN_2,
    AT_MBB_CMD_A_LEN_3,
    AT_MBB_CMD_A_LEN_4 
};
enum AT_DVCFG_PRIORITY_ENUM
{
    AT_DVCFG_CALL_PRIORITY = 0,
    AT_DVCFG_DATA_PRIORITY
};
typedef VOS_UINT32 AT_DVCFG_PRIORITY_UINT32;
#endif /* FEATURE_ON == MBB_FEATURE_MODULE_AT */ 
#if (FEATURE_ON == MBB_WPG_PDPSTATUS)
enum AT_REPORT_PDPSTATUS_ENUM
{
    AT_REPORT_PDPDEACTIVATE_NW   = 0,
    AT_REPORT_PSDETACH_NW,
    AT_REPORT_MSDEACTIVATE_MS
};
typedef VOS_UINT32 AT_REPORT_PDPSTATUS_ENUM_UINT32;

#endif/*FEATURE_ON == MBB_WPG_PDPSTATUS*/
#if (FEATURE_ON == MBB_WPG_DIAL)
enum AT_DIALTYPE_ENUM
{
    AT_DIALTYPE_BUTT                    = 0,
    AT_DIALTYPE_APP                     = 1,
    AT_DIALTYPE_NDIS                    = 2,
    AT_DIALTYPE_MODEM                   = 3
};
typedef VOS_UINT32 AT_DIALTYPE_ENUM_UINT32;
#endif/*FEATURE_ON == MBB_WPG_DIAL*/
/*****************************************************************************
  4 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  6 STRUCT����
*****************************************************************************/
typedef struct
{
    TAF_UINT16   MCC;
    TAF_INT8   Zone;
    TAF_UINT8   Reserved;
}MCC_ZONE_INFO_STRU;


typedef struct
{
    TAF_UINT8  ucLteSupport;
    TAF_UINT8  ucWcdmaSupport;
    TAF_UINT8  ucGsmSupport;
    TAF_UINT8  aucAutoAcqorder[SYSCFGEX_MAX_RAT_STRNUM];
    TAF_UINT8  reserve1;
    TAF_UINT8  reserve2;
}MBB_RAT_SUPPORT_STRU;

extern MBB_RAT_SUPPORT_STRU g_MbbIsRatSupport;

enum AT_DATACLASS_ENUM
{
    AT_DATACLASS_GSM,
    AT_DATACLASS_GPRS,
    AT_DATACLASS_EDGE,
    AT_DATACLASS_WCDMA,
    AT_DATACLASS_HSDPA,
    AT_DATACLASS_HSUPA,
    AT_DATACLASS_HSPA,
    AT_DATACLASS_HSPAPLUS,
    AT_DATACLASS_DC_HSPAPLUS,
};
#if(FEATURE_ON == MBB_NOISETOOL)

typedef struct
{
    VOS_BOOL            bPending;         /*��ǰ�Ƿ��Ѿ���������*/
    VOS_UINT16         usMode;             /*UE ģʽ*/
    VOS_UINT16         usBand;             /*Ƶ��*/
    VOS_UINT16         usDLStFreq;       /*����ɨƵ��ʼƵ��*/
    VOS_UINT16         usDLEdFreq;       /*����ɨƵ����Ƶ��*/
    VOS_UINT16         usFreqStep;       /*����ɨƵ����*/
    VOS_INT16           usWTxPwr;        /*wģ���з��书��*/
    VOS_INT16           usLTxPwr;         /*Lģ���з��书��*/
    VOS_UINT16         usBandWidth;   /*�������*/
    VOS_UINT8           ucIndex;
    VOS_UINT8           usRsv[3];
}AT_NOISE_TOOL_RF_CFG_STRU;

typedef struct
{
    VOS_UINT16         usMode;             /*UE ģʽ*/
    VOS_UINT32         usStFreq;           /*ɨƵ��ʼƵ��*/
    VOS_UINT32         usEdFreq;          /*ɨƵ����Ƶ��*/
    VOS_UINT16         usFreqStep;       /*ɨƵ����*/
    VOS_UINT8           ucIndex;
    VOS_BOOL            bPending;         /*��ǰ�Ƿ��Ѿ���������*/
    VOS_UINT16         usRsv;
}AT_NOISE_TOOL_GPS_CFG_STRU;

typedef struct
{
    AT_NOISE_TOOL_RF_CFG_STRU      stRfCfg;
    AT_NOISE_TOOL_GPS_CFG_STRU    stGpsCfg;
}AT_NOISE_TOOL_CFG_STRU;
#endif/*(FEATURE_ON == MBB_NOISETOOL)*/

#if (FEATURE_ON == MBB_FEATURE_MODULE_AT)
typedef struct
{
    AT_NETSCAN_STATUS_ENUM_UINT8   ucNetscanCurrStatus;
    TAF_NETSCAN_STATUS_DRIVE_FUN   pfNetscanStatusFunc;
}TAF_NETSCAN_STATUS_DRIVE_TABLE_STRU;

/* NETSCAN��Ҫ�������ر�־λ */
typedef struct
{
    VOS_BOOL                             bIsNetscanSetenAble;      /* �Ƿ�����ִ��NETSCAN��ز��� */
    AT_NETSCAN_STATUS_ENUM_UINT8         ucNetscanCurrStatus;      /* ��ǰNETSCAN��״̬������ִ�е���һ���� */
    VOS_BOOL                             bIsSyscfgSaved;           /* �Ƿ񱣴�ԭ��SYSCFGEX���� */
    AT_NETSCAN_SYSINFO_MODE_ENUM_UINT8   ucNetscanSysinfoMode;     /* SYSINFOEX��ѯ���ĵ�ǰϵͳģʽ��Ҳ������ģ */
    TAF_MMA_SYS_CFG_PARA_STRU            stSysCfg;                 /* ����ģ�ԭ��SYSCFGEX���� */
    VOS_BOOL                             bIsNetscanSetParaSaved;   /* �Ƿ񱣴�NETSCAN���õ���ز��� */
    VOS_BOOL                             bIsDetached;              /* �Ƿ���й�ȥע����� */
    VOS_BOOL                             bIsSyscfgexChanged;       /* �Ƿ���й�SYSCFGEX���ò��� */
    VOS_BOOL                             bIsNetscanAborted;        /* �Ƿ���Ҫ�жϣ���ֻ���յ��жϺ�������ҪABORT�������г���ERRORҲ��ҪABORT */
    TAF_MMA_NET_SCAN_REQ_STRU            stNetScanSetPara;         /* ����ģ�NETSCAN���õ���ز��� */
    VOS_UINT32                           ulNetscanResult;          /* NETSCAN�Ľ�� AT_RRETURN_CODE_ENUM */
    VOS_UINT8                            ucIndex;                  /* NETSCAN��index */
    VOS_BOOL                             bIsNetscanGotCnf;         /* �Ƿ��Ѿ���ʾ��NETSCAN��ɨƵ���������Ѿ���ʾ��ɨƵ�����������ʾulNetscanResult */
    VOS_UINT8                            ucReserved0[2];
    HTIMER                               hTimer;
    VOS_UINT32                           ulTimerName;
    VOS_BOOL                             bForbReport;              /* NETSCAN�ڼ䣬���������ϱ��ı�־λ��TRUEʱ��ʾ��ֹ�ϱ� */
    VOS_UINT8                            ucReserved1[3];
}TAF_NETSCAN_STATUS_STRU;

typedef struct
{
    VOS_UINT8                           ucUsed;                                 /*0��δʹ�ã�1��ʹ��*/
    VOS_UINT8                           aucReserved[3];
    TAF_PDP_AUTHDATA_STRU               stAuthData;                             /*��Ȩ����*/
} TAF_NDIS_AUTHDATA_TABLE_STRU;
#endif /* FEATURE_ON == MBB_FEATURE_MODULE_A */

#if (FEATURE_ON == MBB_FEATURE_M2M_LED)

enum AT_M2M_LED_MODE_ENUM
{
    AT_M2M_LED_MODE_CLOSED  = 0,                                                /* �������� */
    AT_M2M_LED_MODE_DEFAULT = 1,                                                /* ��ΪĬ�����Ʒ��� */
    AT_M2M_LED_MODE_USER    = 2,                                                /* �û��Զ������Ʒ��� */
    AT_M2M_LED_MODE_BUTT
};
typedef VOS_UINT8 AT_M2M_LED_MODE_ENUM_UINT8;


enum AT_M2M_LED_FLICKER_TYPE_ENUM
{
    AT_LED_FLICKER_TYPE_ALWAYS  = 0,                                                /* ���������� */
    AT_LED_FLICKER_TYPE_SINGLE  = 1,                                                /* ���� */
    AT_LED_FLICKER_TYPE_DOUBLE  = 2,                                                /* ˫�� */
    AT_LED_FLICKER_TYPE_BUTT
};
typedef VOS_UINT8 AT_LED_FLICKER_TYPE_ENUM_UINT8;

typedef struct
{
    VOS_UINT8                           ucOnDuration;                          /* ����ά�ֵ�����ʱ�� */
    VOS_UINT8                           ucOffDuration;                         /* ����ά�ֵ����ʱ�� */
}AT_M2M_LED_FLICKER_TIME_STRU;


typedef struct
{
    VOS_UINT8                           ucLedFlickerType;                       /* ������˸��ʽ */
    VOS_UINT8                           ucRsv[3];                               /* ����λ */
    AT_M2M_LED_FLICKER_TIME_STRU        stLedFlickerTime[2];                    /* ��������ʱ�� */
}AT_M2M_LED_FLICKER_CTRL_STRU;


typedef struct
{
    VOS_UINT32                          UlLedStateMask;                         /* ����״̬λ�� */
    VOS_UINT8                           ucLedIndex;                             /* ����GPIO�ܽ����� */
    VOS_UINT8                           ucRsv[3];                               /* ����λ */
    AT_M2M_LED_FLICKER_CTRL_STRU        stLedCtrl;                              /* ���Ƶ���˸��ʽ */
}AT_M2M_LED_CONTROL_STRU;


typedef struct
{
    VOS_UINT8                           ucLedConfigNum;                             /* �û������������� */
    VOS_UINT8                           ucRsv[3];                                   /* ����λ */
    AT_M2M_LED_CONTROL_STRU             stLedConfig[AT_LED_SERVICE_STATE_NUM];      /* �û����õ�����ģʽ */
}AT_M2M_LED_CONFIG_STRU;
#endif
#if(FEATURE_ON == MBB_WPG_WIRELESSPARAM)
VOS_UINT32 AT_RcvMtaTxPowerQryCnf(VOS_VOID *pMsg);
VOS_UINT32 AT_RcvMtaMcsSetCnf(VOS_VOID *pMsg);
VOS_UINT32 AT_RcvMtaTddQryCnf(VOS_VOID *pMsg);
#endif /* FEATURE_ON == MBB_WPG_WIRELESSPARAM */
#if (FEATURE_ON == MBB_WPG_CCLK)
typedef struct
{
    TAF_UINT32    tv_sec;            /* seconds */
    TAF_UINT32    tv_nsec;           /* nanoseconds */
}AT_TIMESPEC;
#endif

#if(FEATURE_ON == MBB_WPG_DIAL)
/*****************************************************************************
 �ṹ��    : AT_AUTH_FALLBACK_STRU
 �ṹ˵��  : ��Ȩ����ȫ�ֱ����ṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                             AuthFallbackFlag;   //�Ƿ���м�Ȩ����
    VOS_UINT16                            AuthNum;            //�����Ȩ���˴���
    VOS_UINT16                            AuthType ;          //�����״β��ŵļ�Ȩ����
}AT_AUTH_FALLBACK_STRU;
/*****************************************************************************
 �ṹ��    : AT_NV_AUTHFALLBACK_STRU
 �ṹ˵��  : ��Ȩ����NVȫ�ֱ����ṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                             NvAuthActFlag;        //��Ȩnv�Ƿ���
    VOS_UINT16                            NvAuthType;         //NV�б���ļ�Ȩ����
}AT_NV_AUTHFALLBACK_STRU;

#endif

#if (FEATURE_ON == MBB_WPG_CELLLOCK)

typedef struct
{
    VOS_UINT32                          ulMode;
    VOS_CHAR                           *acStrMode;
}AT_CELLLOCK_MODE_TBL_STRU;
#endif
#if (FEATURE_ON == MBB_FEATURE_MODULE_AT)
/*
<op>: 
0�����ñ�ǣ�����OK�������������flash���ģʽ������ɨ��
1��ȫ����д
2����д<blkid>ָ���Ŀ�
3���˳�flash���ģʽ������ǰ״̬Ϊ����ɨ��������ڻ�д������ERROR��
*/

typedef enum
{
    BITFLIP_OP_SCAN,
    BITFLIP_OP_WB_ALL,
    BITFLIP_OP_WB_BLK,
    BITFLIP_OP_EXIT,
    BITFLIP_OP_MAX,
} bitflip_op_type;
/*
<op_status>��
0������ɨ��
1��ɨ����ɣ�û��λ��ת
2��ɨ����ɣ���λ��ת
3�����ڻ�д
4����д���
5����дʧ��
6����������
*/
typedef enum
{
    BITFLIP_STATUS_SCAN,
    BITFLIP_STATUS_SCAN_NO_FLIP,
    BITFLIP_STATUS_SCAN_HAS_FLIP,
    BITFLIP_STATUS_WB,
    BITFLIP_STATUS_WB_SUC,
    BITFLIP_STATUS_WB_FAI,
    BITFLIP_STATUS_ERROR,
    BITFLIP_STATUS_MAX,
} bitflip_status_type;


typedef struct
{
    bitflip_op_type bitflip_op;             /* bitflip���� */
    bitflip_status_type bitflip_status;     /* bitflip����״̬ */
    int blkaddr_wb;                           /* ��д�Ŀ�ID*/
}bitflip_cmd_result_type;


typedef  struct
{
    unsigned int   error_type;  /**< Error type */
    unsigned int   error_addr;  /**< Address where error occurs */
    unsigned int   error_data;  /**< Data pattern where error occurs */
    unsigned int   ddr_status;  /*DDR ����״̬*/
    unsigned int   bitflip_status;/*bitflip ����״̬*/
}devtest_test_info_stype;


typedef  struct
{
    unsigned int smem_hw_mode;             /* �Լ�ģʽ*/
    unsigned int smem_hw_option;           /* ������ѡ��*/
    unsigned int smem_reserved;            /* ����*/
}at_devtest_info;

typedef enum
{
  DDR_TEST_IS_RUNNING,    
  DDR_TEST_IS_DONE_OK,
  DDR_TEST_IS_DONE_ERR,
  DDR_TEST_OTHER_ERR,
  DDR_TEST_MAX,
} ddr_test_sta;

#define HDTESTMODE  1
#define NOTHDTESTMODE  1
#define FLASH_BITFLIP_OPT  4
#define DDRTEST_OPT  5
#define HARDWARE_TEST_MODE_GET 4
#define DEV_TEST_BOOT_DDR_TEST_INFO_GET  (11)
#define DEV_TEST_MODE_GET (4)
#define DEV_TEST_DEV_NAME "/dev/hardwaretest"    /*Ӳ���Լ������ڵ�*/

#define RET_OK      0
#define RET_FAIL    (-1)
#endif

/*****************************************************************************
  7 UNION����
*****************************************************************************/


/*****************************************************************************
  8 OTHERS����
*****************************************************************************/
#if((FEATURE_ON == MBB_WPG_FREQLOCK) && (FEATURE_OFF == MBB_FEATURE_MODULE_AT))
extern AT_FREQLOCK_MODE_TBL_STRU    g_astFreqLockModeTbl[AT_FREQLOCK_MODE_TYPE_BUTT - 1];
extern AT_FREQLOCK_GSM_BAND_TBL_STRU    g_astFreqLockGsmBandTbl[AT_FREQLOCK_BAND_TYPE_BUTT];
#endif /* FEATURE_ON == MBB_WPG_FREQLOCK */

#if (FEATURE_ON == MBB_FEATURE_MODULE_AT)
extern VOS_BOOL g_bisJammed;
extern TAF_PH_INFO_RAT_TYPE                 g_stNetscanPara2SysMode[TAF_NETSCAN_RAT_BUTT];
extern TAF_NETSCAN_STATUS_STRU              g_stNetscanStatus;
extern TAF_MMA_SYS_CFG_PARA_STRU            g_stNetscanRatOnlySysCfg[TAF_NETSCAN_RAT_BUTT];
extern TAF_NETSCAN_STATUS_DRIVE_TABLE_STRU  g_stNetscanStatusDriveTable[AT_NETSCAN_STATUS_BUTT];
extern TAF_NAS_NETSCAN_RAT_TYPE_ENUM_UINT8  g_stNetscanUserRat2NasMmlRat[TAF_NETSCAN_RAT_BUTT];
#endif /* FEATURE_ON == MBB_FEATURE_MODULE_AT */

#if(FEATURE_ON == MBB_NOISETOOL)
extern AT_NOISE_TOOL_CFG_STRU        g_stAtNoiseToolCfg;
#endif/*(FEATURE_ON == MBB_NOISETOOL)*/

VOS_UINT32 AT_GetTimeInfoDebugFlag(VOS_VOID);
#define AT_EVENT_REPORT_LOG_1(str1, var1) \
{\
    if (VOS_TRUE == AT_GetTimeInfoDebugFlag())\
    {\
        (VOS_VOID)vos_printf("======>%s,%d, %s=%x\n", __func__, __LINE__, str1, var1);\
    }\
}


#if (FEATURE_ON == MBB_MLOG)
#define AT_HIGH_QULITY_RSCP_FDD_MIN      (-95)
#define AT_HIGH_QULITY_RSCP_TDD_MIN      (-84)
#define AT_HIGH_QULITY_RSSI_MIN                (-85)
#define MBB_LOG_RSSI_INFO(var1)\
{\
    if(AT_HIGH_QULITY_RSSI_MIN > var1)\
    {\
        mlog_print("at", mlog_lv_error, "rssi is %d.\n", var1);\
    }\
}

#define MBB_LOG_FDD_RSCP_INFO(var1,var2)\
{\
    if(AT_HIGH_QULITY_RSCP_FDD_MIN > var1)\
    {\
        mlog_print("at",mlog_lv_error,"rscp is %d, ecio is %d.\n",var1, var2);\
    }\
}

#define MBB_LOG_TDD_RSCP_INFO(var1)\
{\
    if(AT_HIGH_QULITY_RSCP_TDD_MIN > var1)\
    {\
        mlog_print("at", mlog_lv_error, "rscp is %d.\n",var1);\
    }\
}
#else
#define MBB_LOG_RSSI_INFO(var1)
#define MBB_LOG_FDD_RSCP_INFO(var1,var2)
#define MBB_LOG_TDD_RSCP_INFO(var1)
#endif

#if(FEATURE_ON == MBB_WPG_DIAL)
#define    MAX_AUTH_NUM        3  //����������
#define   AT_DIAL_Exist_User_Password()                (g_ucUserPasswordExist)
#endif
#if (FEATURE_ON == MBB_WPG_EONS)
extern VOS_VOID At_FormatAndSndEons0(TAF_UINT8 ucIndex, VOS_UINT32 RcvNwNameflag);
extern VOS_UINT32 At_CheckEonsTypeMoudlePara(AT_TAF_PLMN_ID* ptrPlmnID);
#endif/*FEATURE_ON == MBB_WPG_EONS*/

extern VOS_UINT32 At_MbbMatchAtCmdName(VOS_CHAR *pszCmdName);
extern VOS_VOID    At_MbbMatchAtInit(VOS_VOID);

#if(FEATURE_ON == MBB_WPG_FD)
extern VOS_VOID    MatchFastDormOperationType(VOS_UINT32* enFastDormOperationType);
extern VOS_VOID    FastDormTypeDisplay(VOS_UINT16* usLength,
                                                                        VOS_UINT32 enFastDormOperationType,
                                                                        VOS_UINT32 ulTimeLen);
extern VOS_UINT32    MbbSetFastDormPara(VOS_VOID);
#endif/*FEATURE_ON == MBB_WPG_FD*/
#if(FEATURE_ON == MBB_WPG_NWNAME)
extern VOS_VOID At_ReportNWName(TAF_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU *pEvent);
extern VOS_VOID At_ReportMMInfo(VOS_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU *pEvent);
extern VOS_VOID AT_RcvMmInfoInd(VOS_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU *pEvent);
#endif /* FEATURE_ON == MBB_WPG_NWNAME */
#if(FEATURE_ON == MBB_WPG_DLCK)
extern VOS_UINT32 At_FormatDlckCnf(TAF_UINT16* usLength, TAF_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU  *pEvent);
#endif/*FEATURE_ON == MBB_WPG_DLCK*/

#if (FEATURE_ON == MBB_WPG_AT)
extern VOS_VOID At_RunQryParaRspProcCus(TAF_UINT8 ucIndex,TAF_UINT8 OpId, TAF_VOID *pPara, TAF_PARA_TYPE QueryType);
#endif/*FEATURE_ON == MBB_WPG_AT*/

#if(FEATURE_ON == MBB_WPG_CPBS)
extern VOS_VOID At_Pb_VodafoneCPBSCus(TAF_UINT16* usLength, TAF_UINT8 ucIndex);
extern VOS_UINT16 AT_IsVodafoneCustommed(VOS_VOID);
#endif/*FEATURE_ON == MBB_WPG_CPBS*/

#if (FEATURE_ON == MBB_WPG_HCSQ)
extern VOS_VOID AT_RptHcsqChangeInfo(TAF_UINT8 ucIndex,TAF_MMA_RSSI_INFO_IND_STRU *pEvent);
extern VOS_BOOL AT_HdlHcsqCmdResult(VOS_UINT32 *ulResult, VOS_VOID *pstMsg);
#endif/*FEATURE_ON == MBB_WPG_HCSQ*/

#if (FEATURE_ON == MBB_WPG_DUAL_IMSI)
extern VOS_VOID AT_ProcSimRefreshInd(VOS_UINT8 ucIndex, const TAF_PHONE_EVENT_INFO_STRU *pstEvent);
#endif/*FEATURE_ON == MBB_WPG_DUAL_IMSI*/

#if (FEATURE_ON == MBB_WPG_CSIM)
extern VOS_UINT16 AT_IsCSIMCustommed(VOS_VOID);
#endif/*FEATURE_OFF == MBB_WPG_CSIM*/

#if (FEATURE_ON == MBB_WPG_ROAM)
VOS_VOID At_EventReportRoamStatus(VOS_UINT8 *ucRoam);
VOS_UINT32 AT_OperRoamStatus(VOS_UINT8* ucRoam);
#endif /* FEATURE_ON == MBB_WPG_ROAM */
#if (FEATURE_ON == MBB_WPG_DIAL)
extern VOS_VOID AT_Mbb_SetPcuiCallFlag(VOS_UINT8 ucCid);
extern VOS_UINT32 AT_PS_SetCgactState(
VOS_UINT8   ucIndex,
TAF_CID_LIST_STATE_STRU stCidListStateInfo);
extern VOS_UINT32 AT_PS_ReportDefaultDhcp(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_PS_ReportDefaultDhcpV6(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_PS_ReportDefaultApraInfo(TAF_UINT8 ucIndex);
extern VOS_VOID AT_SetIPv6VerFlag(VOS_UINT8  ucCid, VOS_UINT8 ucFlag );
extern VOS_UINT32 AT_PS_ValidateDialParamEx(VOS_UINT8 ucIndex);
extern VOS_VOID AT_PS_SndIPV4FailedResult(VOS_UINT8 ucCallId, VOS_UINT16 usClientId);
extern VOS_VOID AT_PS_ProcIpv4CallRejectEx(VOS_UINT8 ucCallId, TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent);
extern VOS_VOID AT_PS_ParseUsrDialApn(VOS_UINT8 ucIndex, 
    AT_DIAL_PARAM_STRU *pstUsrDialParam);
extern VOS_UINT32 AT_PS_ParseUsrDialParamEx(VOS_UINT8 ucIndex, 
    AT_DIAL_PARAM_STRU *pstUsrDialParam, 
    TAF_PDP_PRIM_CONTEXT_STRU* stPdpCtxInfo);
extern TAF_UINT32 AT_SetDsFlowQryParaEx(TAF_UINT8 ucIndex);
extern VOS_UINT32 AT_GetNdisDialParamEx(TAF_PS_DIAL_PARA_STRU *pstDialParaInfo, VOS_UINT8 ucIndex);
/*��PC�ϱ�����״̬��at����*/
extern VOS_VOID AT_PS_ReportDendNDISSTATEX(VOS_UINT8 ucCid,
    VOS_UINT8                           ucPortIndex,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    TAF_PS_CAUSE_ENUM_UINT32            enCause);
/*��PC�ϱ��Ͽ�״̬��at����*/
extern VOS_VOID AT_PS_ReportDconnNDISSTATEX(VOS_UINT8 ucCid,
    VOS_UINT8                           ucPortIndex,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType);
extern VOS_UINT32 AT_QryNdisStatParaEx( VOS_UINT8 ucIndex );
extern VOS_VOID  AT_PS_MbbProcCallConnectedEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent);
VOS_VOID  AT_PS_MbbProcCallEndEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU  *pstEvent);
extern VOS_VOID  AT_PS_MbbProcDeactiveInd(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU  *pstEvent);

extern VOS_VOID  AT_PS_ModemMbbProcCallRejEvent(
    VOS_UINT8                           ucIndex,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent);
extern VOS_UINT32 Is_Ipv6CapabilityValid(VOS_UINT8 ucIpv6Capability);
extern VOS_UINT32 AT_PS_MbbSetupCall(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucCallId,
    AT_DIAL_PARAM_STRU                  *pstCallDialParam
);
extern VOS_VOID AT_SetModemStat(VOS_UINT8  ucCid, AT_PDP_STATE_ENUM_U8 enPdpState );
#endif/*FEATURE_ON == MBB_WPG_DIAL*/
extern VOS_VOID At_MbbReportCerssiInfo(VOS_UINT32 ulCerssiFlag,
                    VOS_UINT8 ucCerssiReportType,
                    VOS_UINT8 ucCerssiMinTimerInterval,
                    VOS_UINT16 *ptrLength, 
                    TAF_MMA_L_CELL_SIGN_INFO_STRU *pstLCellSignInfo);
#if (FEATURE_ON == MBB_WPG_SYSCFGEX)
extern VOS_VOID At_FormatSyscfgMbb(AT_MODEM_NET_CTX_STRU *pstNetCtx, TAF_MMA_SYSCFG_TEST_CNF_STRU* pstSysCfgTestCnf, VOS_UINT8 ucIndex);
extern VOS_VOID At_FormatSysinfoExMbb(VOS_UINT16* usLength, TAF_PH_SYSINFO_STRU* stSysInfo);
extern VOS_UINT32 AT_PS_CheckSyscfgexModeRestrictPara(VOS_UINT32* ulRst, AT_SYSCFGEX_RAT_ORDER_STRU* stSyscfgExRatOrder);
extern VOS_VOID AT_MBBConverAutoMode(TAF_MMA_RAT_ORDER_STRU    *pstSysCfgRatOrder);
#endif /*FEATURE_ON == MBB_WPG_SYSCFGEX*/

#if(FEATURE_ON == MBB_WPG_CCLK)
extern VOS_VOID AT_UpdateCclkInfo(TAF_MMA_TIME_CHANGE_IND_STRU* pStMmInfo);
#endif/*FEATURE_ON == MBB_WPG_CCLK*/


extern VOS_UINT32 At_RegisterExPrivateMbbCmdTable(VOS_VOID);
extern VOS_VOID AT_ReadNvMbbCustorm(VOS_VOID);
extern VOS_UINT32 TAF_AGENT_GetSysMode(VOS_UINT16 usClientId,  TAF_AGENT_SYS_MODE_STRU*pstSysMode);
#if (FEATURE_ON == MBB_FEATURE_MODULE_AT)
extern VOS_UINT8 AT_MBB_CheckCsca_Param(VOS_UINT8 *ptScaType);
extern TAF_UINT32  At_MBB_CheckUssdNumLen(
    AT_MODEM_SS_CTX_STRU                *pstSsCtx,
    TAF_SS_DATA_CODING_SCHEME           dcs,
    TAF_UINT16                          usLen
);

extern VOS_UINT8 AT_MBB_CUSD_USSDMode_Check(VOS_UINT16 usUssdTransMode, 
                        TAF_SS_DATA_CODING_SCHEME DatacodingScheme);
extern TAF_UINT32   At_SetHcstubPara(TAF_UINT8 ucIndex);
extern TAF_UINT32   At_QryHcstubPara(TAF_UINT8 ucIndex);
extern VOS_VOID At_Ndis_PS_CALL_Info(TAF_PS_DIAL_PARA_STRU *pstCallDialParam);
extern VOS_VOID AT_ReportCssuHoldCallReleased(
    VOS_UINT8                           ucIndex,
    MN_CALL_INFO_STRU                  *pstCallInfo
);
extern VOS_VOID AT_MBB_CopyNotSupportRetValueNV(VOS_UINT32 ulRetLen, VOS_CHAR *acRetVal);
extern VOS_UINT8 AT_MBB_CNMICheckType(AT_CNMI_MODE_TYPE CnmiTmpModeType, AT_CNMI_MT_TYPE CnmiTmpMtType);
extern TAF_UINT8 At_SetCfunSecPara(
    AT_PARSE_PARA_TYPE_STRU *stAtParaList,
    TAF_UINT8   ucIndexNum,
    TAF_UINT8 ucIndex);
extern VOS_VOID AT_Report_Cend(VOS_UINT16 *usLength, MN_CALL_INFO_STRU *pstCallInfo);
extern TAF_VOID At_DvcfgConditionHangupCallingProc(
    TAF_UINT8                           ucIndex,
    MN_CALL_EVENT_ENUM_U32              enEvent,
    MN_CALL_INFO_STRU                   *pstCallInfo);
extern TAF_UINT32 At_MBB_SetVPara(TAF_UINT8 ucIndex);
extern TAF_UINT32 At_MBB_SetQPara(TAF_UINT8 ucIndex);
extern TAF_UINT32 At_MBB_SetZPara(TAF_UINT8 ucIndex);
extern TAF_UINT32 At_SetS0Para(TAF_UINT8 ucIndex);
extern VOS_UINT32 At_MBB_QryS0Para(TAF_UINT8 ucIndex);
extern TAF_UINT32 At_MBB_SetCgact0(TAF_UINT8 ucIndex);
extern VOS_VOID AT_Modem_EventProc(
NAS_OM_EVENT_ID_ENUM_UINT16 enEventId,
VOS_UINT8                   ucIndex
);
extern VOS_VOID AT_GetApnFromPdpContext(
TAF_PDP_PRIM_CONTEXT_EXT_STRU      *pstPdpPrimContextEExt,
VOS_UINT8                           ucCid);

#endif /* FEATURE_ON == MBB_FEATURE_MODULE_AT */ 

#if (FEATURE_ON == MBB_WPG_PDPSTATUS)
VOS_VOID AT_PS_ReportPDPSTATUS(
    AT_REPORT_PDPSTATUS_ENUM_UINT32     enStat,
    VOS_UINT8                           ucPortIndex
);
#endif/*FEATURE_ON == MBB_WPG_PDPSTATUS*/
#if (FEATURE_ON == MBB_WPG_3GPP_STK)
TAF_VOID  At_StkCusatpIndPrint(TAF_UINT8 ucIndex,SI_STK_EVENT_INFO_STRU *pEvent);
TAF_VOID  At_StkCusatmIndPrint(TAF_UINT8 ucIndex,SI_STK_EVENT_INFO_STRU *pEvent);
TAF_VOID At_PrintSTKStandardCMDRsp(
    TAF_UINT8                           ucIndex,
    SI_STK_EVENT_INFO_STRU             *pstEvent
);

#endif /* FEATURE_ON == MBB_WPG_3GPP_STK */

/* MBB_WPG_COMMON */
extern VOS_VOID AT_CLCC_Report(VOS_UINT8 numType, VOS_UINT16 *usLength, VOS_UINT8 *aucAsciiNum);
extern VOS_UINT32 At_TestCgdcont_IP(VOS_UINT8 ucIndex);

#if((FEATURE_ON == MBB_WPG_FREQLOCK) && (FEATURE_OFF == MBB_FEATURE_MODULE_AT))
extern VOS_VOID AT_QryGULFreqlockCnfDisplay(VOS_UINT8 ucIndex, VOS_UINT16 *usLength, MTA_AT_QRY_M2M_FREQLOCK_CNF_STRU *pstQryFreqLockCnf);
extern VOS_UINT32 AT_FreqlockCheckRat(VOS_VOID);  
#endif /* ((FEATURE_ON == MBB_WPG_FREQLOCK) && (FEATURE_OFF == MBB_FEATURE_MODULE_AT)) */

#if (FEATURE_ON == MBB_WPG_DIAL)
extern VOS_VOID AT_PS_SavePSCallAuthRecord(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent);
extern VOS_UINT32 AT_PS_ProcAuthCallReject(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             ucPdpType
);
extern VOS_VOID AT_PS_GenCallDialParam(
    AT_DIAL_PARAM_STRU                 *pstCallDialParam,
    AT_DIAL_PARAM_STRU                 *pstUsrDialParam,
    VOS_UINT8                           ucCid,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
);
extern VOS_VOID AT_PS_ParseUsrDialAuthtype(VOS_UINT8 ucIndex, 
    AT_DIAL_PARAM_STRU *pstUsrDialParam);
#endif/*FEATURE_ON == MBB_WPG_DIAL*/
#if (FEATURE_ON == FEATURE_DTMF)
extern VOS_UINT32 At_SetDdtmfCfgPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_QryDdtmfCfgPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_TestDdtmfCfgPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_SetVtsexPara(VOS_UINT8 ucIndex);
#endif
#if (FEATURE_ON == MBB_FEATURE_MODULE_AT)
extern VOS_VOID AT_NetscanStatusDrive(VOS_VOID);
extern VOS_VOID AT_NetscanStatusClean(VOS_VOID);
extern VOS_BOOL isNetscanEnabled(VOS_VOID);
extern VOS_BOOL isNetscanAbort(VOS_VOID);
extern VOS_BOOL isNetscanNeedRevert(VOS_VOID);
extern VOS_VOID AT_NetscanAbortEnable(VOS_VOID);
extern VOS_VOID AT_NetscanResultSet(VOS_UINT32 ulResult);
extern VOS_VOID AT_NetscanDetached(VOS_VOID);
extern VOS_VOID AT_NetscanStatusEnable(VOS_VOID);
extern VOS_VOID AT_NetscanGotCnf(VOS_VOID);
extern VOS_VOID AT_NetscanSaveSetPara(TAF_MMA_NET_SCAN_REQ_STRU *pstNetScanSetPara);
extern VOS_VOID AT_NetscanSaveSysPara(TAF_MMA_SYS_CFG_PARA_STRU *pstSysPara);
extern VOS_VOID AT_NetscanSaveSysinfoModePara(AT_NETSCAN_SYSINFO_MODE_ENUM_UINT8 ucSysMode);
extern AT_NETSCAN_STATUS_ENUM_UINT8 AT_NetscanStatusGet(VOS_VOID);
extern VOS_VOID AT_NetscanSaveucIndex(VOS_UINT8 ucIndex);
extern VOS_VOID AT_NetscanStatusSet(AT_NETSCAN_STATUS_ENUM_UINT8 ucNetscanCurrStatus);
extern VOS_BOOL AT_NetscanStatusCheck(AT_NETSCAN_STATUS_ENUM_UINT8 ucNetscanCurrStatus);
extern VOS_BOOL AT_NetscanSendReq (VOS_UINT8 ucIndex, TAF_MMA_NET_SCAN_REQ_STRU *pstNetScanSetPara);
extern VOS_BOOL AT_NetscanSysinfoex(VOS_UINT8 ucIndex);
extern VOS_BOOL AT_NetscanSyscfgexRead(VOS_UINT8 ucIndex);
extern VOS_BOOL AT_NetscanSyscfgexSetRatOnly(VOS_UINT8 ucIndex);
extern VOS_BOOL AT_NetscanSyscfgexSetRevert(VOS_UINT8 ucIndex);
extern VOS_BOOL AT_NetscanCgcattDetach(VOS_UINT8 ucIndex);
extern VOS_BOOL AT_NetscanCgcattAttach(VOS_UINT8 ucIndex);
extern VOS_VOID AT_NetscanStatusNullProc(VOS_VOID);
extern VOS_VOID AT_NetscanStatusSysinfoexCnfProc(VOS_VOID);
extern VOS_VOID AT_NetscanStatusSyscfgexGetCnfProc(VOS_VOID);
extern VOS_VOID AT_NetscanStatusDetachCnfProc(VOS_VOID);
extern VOS_VOID AT_NetscanStatusAttachCnfProc(VOS_VOID);
extern VOS_VOID AT_NetscanStatusSyscfgexSetCnfProc(VOS_VOID);
extern VOS_VOID AT_NetscanStatusCampOnProc(VOS_VOID);
extern VOS_VOID AT_NetscanStatusSetCnfProc(VOS_VOID);
extern VOS_VOID AT_NetscanStatusSyscfgexSetRevertCnfProc(VOS_VOID);
extern VOS_UINT32 At_SetDhcpPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_QryDhcpPara(VOS_UINT8 ucIndex);
#endif/* FEATURE_ON == MBB_FEATURE_MODULE_AT */
#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
extern VOS_UINT32 At_TestLedCtrlPara(VOS_UINT8 ucIndex);

extern VOS_UINT32 At_SetLedCtrlPara(VOS_UINT8 ucIndex);

extern VOS_UINT32 At_QryLedCtrlPara(VOS_UINT8 ucIndex);
#endif

#if(FEATURE_ON == MBB_WPG_KDDI_SIMLOCK_FREE)
extern VOS_UINT32 At_QrySimLockPlmnInfo(VOS_UINT8 ucIndex, VOS_UINT32 ulSimLockFlag);
extern VOS_UINT32 At_MBBQrySimLockPlmnInfo(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_QrySimFreeInfoPlmnInfo(VOS_UINT8 ucIndex);
#endif /* FEATURE_ON == MBB_WPG_KDDI_SIMLOCK_FREE */

#if(FEATURE_ON == MBB_NOISETOOL)
extern VOS_UINT32 AT_SetNoiseCfgPara(VOS_UINT8 ucIndex);
extern VOS_VOID At_NoiseRssi_Report
(
    VOS_UINT16 usDLStFreq,
    VOS_UINT16 usDLRssiNum,
    VOS_INT16 *usDLPriRssi,
    VOS_INT16 *usDLDivRssi
);
extern TAF_UINT32 At_NoiseToolSendRxOnOffToHPA
(
    AT_NOISE_TOOL_RF_CFG_STRU *pstNoiseCfg
);
extern TAF_UINT32 At_NoiseToolSendRxOnOffToGHPA
(
    TAF_UINT8 ucIndex, 
    AT_NOISE_TOOL_RF_CFG_STRU *pstNoiseCfg
);
extern TAF_UINT32 AT_SetRfonoffPara(TAF_UINT8 ucIndex);
extern TAF_UINT32 At_QryRFonoffStatePara(TAF_UINT8 ucIndex);
#endif  /*(FEATURE_ON == MBB_NOISETOOL)*/

extern VOS_UINT32  At_QryLteCatEx(VOS_UINT8 ucIndex);

typedef struct
{
    VOS_UINT32    UeCapabilitybitUeEutraCapV940Present                     : 1;
    VOS_UINT32    UeCapabilitybitUeEutraCapV1020Present                   : 1;
    VOS_UINT32    UeCapV1020bitUeCatgV1020Present                             : 1;
    VOS_UINT32    UeCapV1020bitNonCritiExtPresent                             : 1;
    VOS_UINT32    UeCapV1060bitNonCritiExtPresent                             : 1; 
    VOS_UINT32    UeCapV1090bitNonCritiExtPresent                             : 1;
    VOS_UINT32    UeCapV1130bitNonCritiExtPresent                             : 1;
    VOS_UINT32    UeCapV1170bitNonCritiExtPresent                             : 1;
    VOS_UINT32    UeCapV1170bitucCategoryPresent                             : 1;
    VOS_UINT32    UeCapV1180bitNonCritiExtPresent                             : 1;    
    VOS_UINT32    UeCapV11A0bitUeCatgV11a0Present                         : 1;
    VOS_UINT32    UebitSpare                                                               : 21;

    VOS_UINT8     UeCapabilityUeCatg;
    VOS_UINT8     UeCapV1020UeCatgV1020;
    VOS_UINT8     UeCapV1170UeCategoryV1170; 
    VOS_UINT8     UeCapV11A0UeCatgV11a0;
}Lte_Cat_Info_STRU;

typedef struct 
{
    VOS_UINT32 ulNvId; 
    VOS_UINT32 ulNvLen;    
}Lte_Cat_Nv_Info_STRU;


#if (FEATURE_ON == MBB_WPG_ECALL)
extern VOS_UINT32 AT_MbbSetEclstartPara(VOS_UINT8 ucIndex);
VOS_VOID AT_EcallStatusError(VOS_VOID);
extern VOS_UINT32 At_MatchEcallCmdName(VOS_CHAR *pszCmdName);
extern VOS_UINT8 At_HaveEcall(TAF_UINT8 ucIndex);
extern VOS_VOID AT_EcallRedial(VOS_VOID);
extern VOS_UINT32 EcallRedailClear(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_SetEclpushPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_QryEclListPara(VOS_UINT8 ucIndex);
extern VOS_VOID At_EcallAlackDisplay(AT_ECALL_ALACK_VALUE_STRU    EcallAlackInfo, VOS_UINT16   *usLength);
extern AT_ECALL_ALACK_INFO_STRU* AT_GetEcallAlAckInfoAddr(VOS_VOID);
extern VOS_UINT32 At_ProcVcReportEcallAlackEvent(VOS_UINT8   ucIndex, APP_VC_EVENT_INFO_STRU   *pstVcEvtInfo);
extern VOS_VOID AT_EcallAlAckListDisplay(VOS_VOID);
extern VOS_VOID At_RcvAmrcfgQryCnf(MN_AT_IND_EVT_STRU  *pstData);
extern VOS_VOID AT_RcvTafCallArmcfgCnf(MN_AT_IND_EVT_STRU    *pstData);
extern VOS_UINT32 At_SetAmrcfgPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 At_QryAmrcfgPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_RcvVcMsgEcallPushCnfProc(MN_AT_IND_EVT_STRU  *pstData);
#endif/*FEATURE_OFF == MBB_WPG_ECALL*/

/* ��Ҫ�ص�check���� */
extern VOS_BOOL At_CmdAddDoubleQuotes(VOS_UINT8** pHead, VOS_UINT16 * usCount, VOS_CHAR * pAtcmdString,
                               VOS_UINT16 uiLocation1, VOS_UINT16 uiLocation2);
#endif/*FEATURE_ON == MBB_WPG_COMMON*/
#if (FEATURE_ON == MBB_FEATURE_MODULE_AT)
VOS_VOID AT_ProcSimresetInd(
    TAF_UINT8                           ucIndex,
    TAF_PHONE_EVENT_INFO_STRU           *pEvent
);
VOS_VOID  AT_ProcCsRspEvtAlerting(
    TAF_UINT8                           ucIndex,
    MN_CALL_INFO_STRU                  *pstCallInfo
);
/* NETSCAN��Ӧ״̬�Ĵ����� */
typedef VOS_VOID ( * TAF_NETSCAN_STATUS_DRIVE_FUN )
(
    VOS_VOID
);
#endif /* FEATURE_ON == MBB_FEATURE_MODULE_AT */
#if (FEATURE_ON == MBB_FEATURE_M2M_LED)
VOS_VOID BSP_CombineLedNVCtrlPara(
    NV_LED_SET_PARA_STRU               *pstLedNvStru,
    NV_LED_SET_PARA_STRU_EXPAND        *pstLedNvExStru,
    NV_LED_SET_PARA_STRU_COMBINED      *pstLedNvCombinedStru
);

VOS_VOID BSP_PartLedNVCtrlPara(
    NV_LED_SET_PARA_STRU               *pstLedNvStru,
    NV_LED_SET_PARA_STRU_EXPAND        *pstLedNvExStru,
    NV_LED_SET_PARA_STRU_COMBINED      *pstLedNvCombinedStru
);

VOS_VOID BSP_M2M_LedUpdate(VOS_VOID *pstLedNvStru);
#endif
#if (FEATURE_ON == MBB_WPG_CPBREADY)
VOS_VOID AT_ProcPbreadyInd(VOS_UINT8   ucIndex);
#endif/*FEATURE_ON == MBB_WPG_CPBREADY*/
#if (FEATURE_OFF == MBB_WPG_COMMON)
#define MBB_LOG_RSSI_INFO(var1)
#define MBB_LOG_FDD_RSCP_INFO(var1,var2)
#define MBB_LOG_TDD_RSCP_INFO(var1)
#define AP_CONN_DEBUG_STR_VAR(str, var)
#define AP_CONN_DEBUG()
#endif/*FEATURE_ON == MBB_WPG_COMMON*/
#if(FEATURE_ON == MBB_WPG_AC)

extern TAF_VOID AT_ACReportMsgProc(TAF_MMA_MSG_AC_REPORT_STRU *pMsg);
#endif/* FEATURE_ON == MBB_WPG_AC */

#if(FEATURE_ON == MBB_WPG_SIM_SWITCH)
extern TAF_VOID      At_QryParaRspSimSwitchProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
);
extern TAF_VOID      At_SimSwitchMsgProc(MN_MSG_SIM_SWITCH_STRU* pMsg);
extern VOS_VOID      AT_ReadSimSwitchFlag( VOS_VOID );
#endif/* FEATURE_ON == MBB_WPG_SIM_SWITCH */

#if(FEATURE_ON == MBB_MLOG)
typedef struct
{
    VOS_INT16      sRsrpValue;/* ��Χ��(-141,-44), 99Ϊ��Ч */
    VOS_INT16      sRsrqValue;/* ��Χ��(-40, -6) , 99Ϊ��Ч */
    VOS_INT32      lSINRValue;/* SINR */
    VOS_INT16      sRscpValue;  /* С���ź���������3g��*/
    VOS_INT16      sEcioValue;  /* С�����������3g*/
    VOS_INT16      sRssiValue;   /* С���ź���������2g��^cerssi�ϱ�ʹ��,2gû��rscp�ĸ����õ���rssi */
    VOS_UINT8      aucReserve1[2];  /*reserv*/
}SIGNAL_INFO_STRU;

typedef struct
{
    TAF_PH_INFO_RAT_TYPE               ucRatType;           /* ���뼼��   */
    TAF_SYS_SUBMODE_ENUM_UINT8         ucSysSubMode;        /* ϵͳ��ģʽ */
} AT_SYS_MODE_STRU;

extern SIGNAL_INFO_STRU       g_stSignalInfo;
extern AT_SYS_MODE_STRU     g_stAtSysMode;
extern VOS_VOID   AT_MBB_RecordMlogDialFail(VOS_UINT8 ucCallId, TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU   *pstEvent);
extern VOS_VOID   AT_MBB_MlogPrintPdpInfo(VOS_UINT8 ucCallId, TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU   *pEvtInfo);
#endif

#if (FEATURE_ON == MBB_WPG_SBB_RRCSTAT)
extern VOS_UINT32 AT_RcvMmaRrcStatQueryCnf(VOS_VOID *pstMsg);
#endif /*(FEATURE_ON == MBB_WPG_SBB_RRCSTAT)*/

/*#if (FEATURE_ON == MBB_WPG_TDD_FDD_CHANGE_SBB), Ϊ����UT���̣�����ʹ��COMMON����ƣ�
�궨��ͽṹ�嶨�岻Ӱ������ģ����߼����ʿ�����COMMON��������*/
#if (FEATURE_ON == MBB_WPG_COMMON)
#define LTE_BAND1                (1)
#define LTE_BAND3                (3)
#define LTE_BAND8                (8)
#define LTE_BAND41               (41)
#define LTE_BAND42               (42)


#define LTE_BAND1_BIT                (1)
#define LTE_BAND3_BIT                (0x4)
#define LTE_BAND8_BIT                (0x80)                                                 /* 0b10000000 */
#define LTE_BAND41_BIT               (0x10000000000ULL)     /* 0b10000000000000000000000000000000000000000 */
#define LTE_BAND42_BIT               (0x20000000000ULL)    /* 0b100000000000000000000000000000000000000000 */
#define LTE_TDD_ONLY          (1)
#define LTE_FDD_ONLY          (2)
#define LTE_TDD_AND_FDD       (3)
#define FGI_DISABLE              (0)
#define FGI_ENABLE               (1)

/* AIR3 project */
#define FDD_FGI_V1020             (0x0)           /* �ӵ�λ����λ����ΪFGI BIT101, FGI BIT102...... */
/* TDDģʽ�����B1��B8��ģʽ1�°��տͻ�Ҫ���split FGI��ʽ�ϱ�*/
#define TDD_FGI_V1020             (0x000007FC)    /* �ӵ�λ����λ����ΪFGI BIT101, FGI BIT102......*/
#define TDD_FDD_FGI_V1020         (0x00000008)    /* �ӵ�λ����λ����ΪFGI BIT101, FGI BIT102......*/
#define FDD_FGI_V9                (0x411BB03A)    /* �ӵ�λ����λ����ΪFGI BIT1, FGI BIT2...... */
/* TDDģʽ�����B1��B8��ģʽ1�°��տͻ�Ҫ���split FGI��ʽ�ϱ�*/
/*#define TDD_FGI_V9              (0x611BB03B)    /*�ӵ�λ����λ����ΪFGI BIT1, FGI BIT2...... */
#define TDD_FGI_V9                (0x411BB03B)    /*�ӵ�λ����λ����ΪFGI BIT1, FGI BIT2...... */
#define TDD_FDD_FGI_V9            (0x611BB03A)    /*�ӵ�λ����λ����ΪFGI BIT1, FGI BIT2...... */

#define SBB_LTE_NO_BAND   (0x0)
#define SBB_LTE_B1_B8     (0x81)
#define SBB_LTE_B1_B3_B8  (0x85)
#define SBB_LTE_B41_B42   (0x300)

#define AT_CMD_STR_LBANDSWITCHSBB    "^LBANDSWITCHSBB"
#define AT_CMD_STR_CASWITCHSBB       "^CASWITCHSBB"

/*#if (FEATURE_ON == MBB_WPG_RAT_BAND_PRIORITY), Ϊ����UT���̣�����ʹ��COMMON����ƣ�
�궨��ͽṹ�嶨�岻Ӱ������ģ����߼����ʿ�����COMMON��������*/

enum NV_RAT_TYPE_ENUM
{
    NV_RAT_GSM,                                                       /*GSM���뼼�� */
    NV_RAT_WCDMA,                                                     /* WCDMA���뼼�� */
    NV_RAT_LTE,                                                       /* LTE���뼼�� */

    NV_RAT_BUTT
};

#define  AT_MAX_SET_BAND_NUM    (10)
#define  AT_MAX_SET_PARA_NUM    (AT_MAX_SET_BAND_NUM + 2)
#define  AT_MAX_FDD_BAND_NUM    (32)

#define  AT_MAX_RAT_NUM    (3)
#define  AT_MIN_RAT_NUM    (1)

/* ɾ������ö��ֵ, ����Э��ջ����ö������PS_BOOL_ENUM */

/* ���ݻ���������ۣ��޸�����BAND���ȼ�AT�������� */
#define AT_CMD_STR_BANDLOCK    "^BANDPRIORITY"

typedef struct
{
    VOS_UINT8                   aucUeWcdmaBandPriodiry[NVIM_MAX_FDD_FREQ_BANDS_NUM];
}NVIM_WAS_FREQ_BAND_PRIORITY_STRU;

/* #endif */

typedef union {
    LRRC_NV_UE_EUTRA_CAP_STRU stUeCapR9;
    /* ����V9a0�ṹ���������л�ģʽʱ��Ҫ�޸�Fdd\Tdd R9Present������չλ*/
    RRC_UE_EUTRA_CAP_V9A0_IES_STRU stUeCapV9a0;
    RRC_UE_EUTRA_CAP_V1020_IES_STRU stUeCapV1020;
    RRC_UE_EUTRA_CAP_V1060_IES_STRU stUeCapV1060;
    RRC_UE_EUTRA_CAP_V1170_IES_STRU stUeCapV1170;
	RRC_UE_EUTRA_CAP_V11A0_IES_STRU stUeCapV11a0;
}LTE_CAPA_INFO;
#endif
/*#endif*/

#if (FEATURE_ON == MBB_WPG_CELLLOCK_CPE)
VOS_VOID AT_CellLock_CalculateDialTimeDereg(VOS_VOID);
VOS_UINT8 At_GetCellLockStatus(VOS_VOID);
VOS_UINT32 At_RcvMmaCellLockStatusChangeNotify(VOS_VOID *pMsg);
VOS_UINT32 At_RcvMmaCellLockDialTimeChangeNotify(VOS_VOID *pMsg);
VOS_VOID AT_Celllock_CalculateDialTimeCb(VOS_VOID);
VOS_VOID At_CellLock_init(VOS_VOID);
VOS_VOID AT_CellLock_CalculateDialTimeReg(VOS_VOID);
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


#endif /*__AT_H__
 */






