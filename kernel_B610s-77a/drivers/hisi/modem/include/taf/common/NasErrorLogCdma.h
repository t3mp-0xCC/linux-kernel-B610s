/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may
* *    be used to endorse or promote products derived from this software
* *    without specific prior written permission.
*
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

#ifndef __NAS_ERRORLOGCDMA_H__
#define __NAS_ERRORLOGCDMA_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "NasErrorLogComm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define NAS_ERR_LOG_HRPD_SECTORID_LENGTH                (16)

#define NAS_ERR_LOG_CL_OOS_MAX_SEARCH_RECORD_NUM        (20)   /* CL多模搜网过程中，记录的搜网信息最大轮数 */
#define NAS_ERR_LOG_MAX_CDMA_PS_CALL_EXCEP_REDIAL_NUM   (10)   /* CDMA PS呼叫过程中记录的重拨信息最大条数 */
#define NAS_ERR_LOG_MAX_CDMA_PS_DISC_RECORD_NUM         (10)   /* CDMA PS异常断链过程中，记录的异常信息最大条数  */

#define NAS_ERR_LOG_1X_CS_CALL_MAX_REDIAL_NUM           (10)

#define NAS_ERR_LOG_1X_OOS_MAX_SEARCH_RECORD             (20)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/



enum NAS_ERR_LOG_SESSION_EXCEP_DEACT_CAUSE_ENUM
{
    NAS_ERR_LOG_SESSION_EXCEP_DEACT_CAUSE_BY_AN                  = 0,
    NAS_ERR_LOG_SESSION_EXCEP_DEACT_CAUSE_BY_KEEPALIVE_TIMEOUT   = 1,
    NAS_ERR_LOG_SESSION_EXCEP_DEACT_CAUSE_BY_SESSION_NEG_FAIL    = 2,
    NAS_ERR_LOG_SESSION_EXCEP_DEACT_CAUSE_BUTT
};
typedef  VOS_UINT32  NAS_ERR_LOG_SESSION_EXCEP_DEACT_CAUSE_ENUM_UINT32;



enum NAS_ERR_LOG_CL_SEARCH_RSLT_ENUM
{
    NAS_ERR_LOG_CL_SEARCH_RSLT_HRPD_SUCC,           /* HRPD搜网成功 */
    NAS_ERR_LOG_CL_SEARCH_RSLT_HRPD_FAIL,           /* HRPD搜网失败 */
    NAS_ERR_LOG_CL_SEARCH_RSLT_HRPD_ABORTED,        /* HRPD搜网被打断 */
    NAS_ERR_LOG_CL_SEARCH_RSLT_HRPD_NO_RF,          /* HRPD无资源 */
    NAS_ERR_LOG_CL_SEARCH_RSLT_HRPD_TIMEOUT,        /* 等待HSD搜网结果超时 */
    NAS_ERR_LOG_CL_SEARCH_RSLT_HRPD_MSCC_ABORT,     /* HRPD搜网在MSCC被打断 */
    NAS_ERR_LOG_CL_SEARCH_RSLT_LTE_NORMAL_SRV,      /* LTE搜网成功 */
    NAS_ERR_LOG_CL_SEARCH_RSLT_LTE_LIMITED_SRV,     /* LTE搜网失败 */
    NAS_ERR_LOG_CL_SEARCH_RSLT_LTE_NO_SRV,          /* LTE搜网失败 */
    NAS_ERR_LOG_CL_SEARCH_RSLT_LTE_INTERUPT,        /* LTE搜网被打断 */
    NAS_ERR_LOG_CL_SEARCH_RSLT_LTE_NO_RF,           /* LTE无资源 */
    NAS_ERR_LOG_CL_SEARCH_RSLT_LTE_L2C,             /* LTE重选到HRPD*/
    NAS_ERR_LOG_CL_SEARCH_RSLT_LTE_TIMEOUT,         /* MSCC等待MCC搜网结果超时*/
    NAS_ERR_LOG_CL_SEARCH_RSLT_LTE_MSCC_ABORT,      /* LTE搜网在MSCC被打断 */
    NAS_ERR_LOG_CL_SEARCH_RSLT_BUTT
};
typedef VOS_UINT32  NAS_ERR_LOG_CL_SEARCH_RSLT_ENUM_UINT32;


enum NAS_ERR_LOG_APS_RAT_TYPE_ENUM
{
    NAS_ERR_LOG_PS_CALL_RAT_TYPE_NULL,          /* 当前未在任何网络上驻留 */
    NAS_ERR_LOG_PS_CALL_RAT_TYPE_GSM,           /* GSM接入技术 */
    NAS_ERR_LOG_PS_CALL_RAT_TYPE_WCDMA,         /* WCDMA接入技术 */
    NAS_ERR_LOG_PS_CALL_RAT_TYPE_LTE,           /* LTE接入技术 */
    NAS_ERR_LOG_PS_CALL_RAT_TYPE_1X,            /* CDMA-1X接入技术 */
    NAS_ERR_LOG_PS_CALL_RAT_TYPE_HRPD,          /* CDMA-HRPD接入技术 */
    NAS_ERR_LOG_PS_CALL_RAT_TYPE_EHRPD,         /* CDMA-EHRPD接入技术 */
    NAS_ERR_LOG_PS_CALL_RAT_TYPE_BUTT           /* 无效的接入技术 */
};
typedef VOS_UINT32 NAS_ERR_LOG_PS_CALL_RAT_TYPE_ENUM_UINT32;


enum NAS_ERR_LOG_1X_OOS_CAUSE_ENUM
{
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_RESCAN,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_PROTO_MISMATCH,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_SYS_LOST,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_LOCK,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_UNLOCK,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_ACCESS_DENIED,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_ACCT_BLOCKED,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_NDSS_OFF,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_ERR,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_REDIRECTION,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_REGIST_REJ,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_RELEASE,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_RESELECTION,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_NEW_SYSTEM,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_MAX_ACCESS_FAILURE,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_ABNORMAL,
    NAS_ERR_LOG_1X_OOS_CAUSE_SD_ACQUISITION_FAIL,
    NAS_ERR_LOG_1X_OOS_CAUSE_NO_RF,
    NAS_ERR_LOG_1X_OOS_CAUSE_INVAID_REDIRECTION,
    NAS_ERR_LOG_1X_OOS_CAUSE_REDIRE_FAILURE,
    NAS_ERR_LOG_1X_OOS_CAUSE_PREFERRED_RESELECTION,
    NAS_ERR_LOG_1X_OOS_CAUSE_NOT_PREFERRED_RESELECTION,
    NAS_ERR_LOG_1X_OOS_CAUSE_HANDOFF_NEGATIVE_SYS,
    NAS_ERR_LOG_1X_OOS_CAUSE_CALL_RELEASE_ACQ_FAIL,
    NAS_ERR_LOG_1X_OOS_CAUSE_ENC_CALL_RELEASE_ACQ_FAIL,
    NAS_ERR_LOG_1X_OOS_CAUSE_SWITCH_ON_SEARCH_FAIL,

    NAS_ERR_LOG_1X_OOS_CAUSE_BUTT
};
typedef VOS_UINT32 NAS_ERR_LOG_1X_OOS_CAUSE_ENUM_UINT32;


enum  NAS_ERR_LOG_1X_OOS_SEARCH_ENUM
{
    NAS_ERR_LOG_1X_SEARCH_RSLT_SUCCESS,
    NAS_ERR_LOG_1X_SEARCH_RSLT_ABORTED_BY_SRV_ACQ,
    NAS_ERR_LOG_1X_SEARCH_RSLT_ABORTED_BY_CALL_REDIAL,
    NAS_ERR_LOG_1X_SEARCH_RSLT_ABORTED_BY_MO_CALL_END,
    NAS_ERR_LOG_1X_SEARCH_RSLT_ABORTED_BY_POWER_SAVE,
    NAS_ERR_LOG_1X_SEARCH_RSLT_ABORTED_BY_POWER_OFF,
    NAS_ERR_LOG_1X_SEARCH_RSLT_ABORTED_BY_END_EMC_CALLBACK,
    NAS_ERR_LOG_1X_SEARCH_RSLT_ABORTED_BY_CALLBACK_SRCH_TIMEOUT,
    NAS_ERR_LOG_1X_SEARCH_RSLT_ABORTED_BY_CALLBACK_MODE_TIMEOUT,
    NAS_ERR_LOG_1X_SEARCH_RSLT_FAILURE,
    NAS_ERR_LOG_1X_SEARCH_RSLT_NO_RF,
    NAS_ERR_LOG_1X_SEARCH_RSLT_BUTT
};
typedef VOS_UINT32 NAS_ERR_LOG_1X_SEARCH_RSLT_ENUM_UINT32;


enum  NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_ENUM
{
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_SUCCESS,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_LOCK,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_NO_SERVICE,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_INTERCEPT,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_REORDER,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_NORMAL_RELEASE,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_SO_REJ,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_MAX_ACCESS_PROBES,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_ACCESS_IN_PROGRESS,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_ACC_FAIL,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_NOT_ACCEPT_BY_BS,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_ACCT_BLOCK,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_SIGNAL_FADE,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_CHANNEL_ASSIGN_TIMEROUT,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_RELEASE_TIMEOUT,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_RETRY_FAIL,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_NO_RF,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_EXIST_MT_VOICE,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_EXIST_MO_VOICE,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_EXIST_MO_CSMS,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_EXIST_MT_CSMS,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_EXIST_MO_AGPS,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_EXIST_MT_AGPS,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_EXIST_MO_CDATA,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_EXIST_MT_CDATA,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_DATA_SWITCHED,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_POWER_DOWN,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_INCOMING_RSP_FAIL,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_INCOMING_RSP_TIME_OUT,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_WAIT_AS_EST_CNF_TIME_OUT,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_WAIT_AS_CALL_INIT_IND_TIME_OUT,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_WAIT_FOR_ORDER_TIME_OUT,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_WAIT_WAIT_FOR_ANSWER_TIME_OUT,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_WAIT_WAIT_CONN_L2_ACK_TIME_OUT,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_REL_IND,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_MS_NORMAL_RELEASE,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_ACCESS_DENYIED,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_NDSS,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_REDIRECTION,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_SIGNAL_FADE_IN_TCH,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_ABORT,
    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_CLIENT_END,

    NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_BUTT
};
typedef VOS_UINT32 NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_CAUSE_ENUM_UINT32;



enum NAS_ERR_LOG_1X_MT_SERVICE_TYPE_ENUM
{
    NAS_ERR_LOG_1X_MT_SERVICE_TYPE,
    NAS_ERR_LOG_1X_MT_SERVICE_TYPE_VOICE_CALL,   /* 语音呼叫 */
    NAS_ERR_LOG_1X_MT_SERVICE_TYPE_DATA_CALL,    /* 数据呼叫 */
    NAS_ERR_LOG_1X_MT_SERVICE_TYPE_SMS,          /* 短消息 */
    NAS_ERR_LOG_1X_MT_SERVICE_TYPE_AGPS_CALL,    /* AGPS */

    NAS_ERR_LOG_1X_MT_SERVICE_TYPE_BUTT
};
typedef VOS_UINT32 NAS_ERR_LOG_1X_SERVICE_TYPE_ENUM_UINT32;



/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/



typedef struct
{
    OM_ERR_LOG_HEADER_STRU              stHeader;
    VOS_UINT16                          usSid;
    VOS_UINT16                          usNid;
    VOS_UINT16                          usBaseId;
    VOS_UINT8                           aucReserve[2];
    VOS_INT32                           lLatitude;
    VOS_INT32                           lLongitude;
    VOS_UINT16                          usBand;
    VOS_UINT16                          usFreq;
}NAS_ERR_LOG_1X_COMMON_INFO_STRU;


typedef struct
{
    OM_ERR_LOG_HEADER_STRU              stHeader;
    VOS_UINT8                           ucColorCode;
    VOS_UINT8                           ucSubnetMask;
    VOS_UINT8                           aucReserve[2];
    VOS_UINT8                           aucSectorId[NAS_ERR_LOG_HRPD_SECTORID_LENGTH];
    VOS_UINT32                          ulSector24;
    VOS_UINT16                          usBand;
    VOS_UINT16                          usFreq;
}NAS_ERR_LOG_HRPD_COMMON_INFO_STRU;


typedef struct
{
    OM_ERR_LOG_HEADER_STRU                                  stHeader;
    NAS_ERR_LOG_1X_OOS_CAUSE_ENUM_UINT32                    enOosCause;
}NAS_ERR_LOG_1X_OOS_REPORT_STRU;


typedef struct
{
    OM_ERR_LOG_HEADER_STRU                                  stHeader;
    VOS_UINT8                                               ucAlreadyReportOosToApFlag; /*区分丢网上报是否被用户所感知，被感知为TRUE；即延迟上报定时器超时后还未搜到网*/
    VOS_UINT8                                               aucReserve[3];
}NAS_ERR_LOG_1X_OOS_RECOVERY_STRU;


typedef struct
{
    VOS_UINT32                                              ulIndex;
    VOS_UINT32                                              ulStartTimer;
    VOS_UINT32                                              ulEndTimer;
    NAS_ERR_LOG_1X_SEARCH_RSLT_ENUM_UINT32                  enSearchRslt;
} NAS_ERR_LOG_1X_OOS_SEARCH_RECORD_STRU;


typedef struct
{
    OM_ERR_LOG_HEADER_STRU                                  stHeader;
    VOS_UINT32                                              ulSearchRecordCnt;
    NAS_ERR_LOG_1X_OOS_SEARCH_RECORD_STRU                   astSearchRecord[NAS_ERR_LOG_1X_OOS_MAX_SEARCH_RECORD];
}NAS_ERR_LOG_1X_OOS_SEARCH_STRU;


typedef struct
{
    OM_ERR_LOG_HEADER_STRU              stHeader;
    VOS_UINT32                          ulStartTimer;
    VOS_UINT32                          ulEndTimer;
    VOS_UINT8                           ucUATITriggerCause;     /* 发起UATI的原因 */
    VOS_UINT8                           ucUATIFailCause;        /* UATI失败的原因 */
    VOS_UINT8                           aucRsv[2];
}NAS_ERR_LOG_HRPD_UATI_FAIL_STRU;



typedef struct
{
    OM_ERR_LOG_HEADER_STRU              stHeader;
    VOS_UINT8                           ucSessionActReason;     /* 发起Session激活的原因 */
    VOS_UINT8                           ucSessionActType;       /* Session激活的类型，包含recovery，init，restore prio */
    VOS_UINT8                           ucSupportEhrpdFlg;      /* 是否支持eHRPD */
    VOS_UINT8                           ucSessionFailCause;     /* Session失败的原因 */
    VOS_UINT32                          ulStartTimer;
    VOS_UINT32                          ulEndTimer;
}NAS_ERR_LOG_HRPD_SESSION_FAIL_STRU;



typedef struct
{
    OM_ERR_LOG_HEADER_STRU                                  stHeader;
    VOS_UINT32                                              ulTimer;
    NAS_ERR_LOG_SESSION_EXCEP_DEACT_CAUSE_ENUM_UINT32       enSessionDeactCause;     /* 区分网络主动发送Session close或keep alive超时或重配置失败 */
} NAS_ERR_LOG_HRPD_SESSION_EXCEP_DEACT_STRU;


typedef struct
{
    VOS_UINT32                                              ulIndex;
    VOS_UINT32                                              ulStartTimer;
    VOS_UINT32                                              ulEndTimer;
    VOS_UINT32                                              ulSearchRat;
    VOS_UINT32                                              ulSearchType;
    NAS_ERR_LOG_CL_SEARCH_RSLT_ENUM_UINT32                  enSearchRslt;
} NAS_ERR_LOG_CL_OOS_SEARCH_RECORD_STRU;


typedef struct
{
    OM_ERR_LOG_HEADER_STRU                                  stHeader;
    VOS_UINT32                                              ulIsHrpdAndLteBothSupport; /* 当前系统是否同时支持HRPD和LTE, TRUE：支持，FALSE：不支持 */
    VOS_UINT32                                              ulSearchRecordCnt;
    NAS_ERR_LOG_CL_OOS_SEARCH_RECORD_STRU                   astSearchRecord[NAS_ERR_LOG_CL_OOS_MAX_SEARCH_RECORD_NUM];
} NAS_ERR_LOG_CL_OOS_SEARCH_STRU;


typedef struct
{
    VOS_UINT32                                              ulIndex;
    VOS_UINT32                                              ulStartTimer;
    VOS_UINT32                                              ulEndTimer;
    NAS_ERR_LOG_PS_CALL_RAT_TYPE_ENUM_UINT32                enRatMode;       /* 本次重拨发起的系统模式，区分1x,HRPD,eHRPD */
    VOS_UINT32                                              ulRedialRslt;    /* 复用下面enPsCause取值类型,TAF_PS_CAUSE_ENUM_UINT32 */
} NAS_ERR_LOG_CDMA_PS_CALL_REDIAL_INFO_STRU;


typedef struct
{
    OM_ERR_LOG_HEADER_STRU                                  stHeader;
    VOS_UINT32                                              ulPsCause;     /* 最后一次失败原因,TAF_PS_CAUSE_ENUM_UINT32 */
    NAS_ERR_LOG_APS_PDP_ENTITY_FSM_INFO_STRU                stApsFsmInfo;  /* 状态机信息 */
    VOS_UINT32                                              ulRedialCnt;
    NAS_ERR_LOG_CDMA_PS_CALL_REDIAL_INFO_STRU               astRedialInfo[NAS_ERR_LOG_MAX_CDMA_PS_CALL_EXCEP_REDIAL_NUM];
}NAS_ERR_LOG_CDMA_PS_CALL_EXCEPTION_STRU;



typedef struct
{
    VOS_UINT32                                              ulIndex;
    VOS_UINT32                                              ulPsCause;     /* 最后一次失败原因 */
    NAS_ERR_LOG_APS_PDP_ENTITY_FSM_INFO_STRU                stApsFsmInfo;  /* 状态机信息 */
    VOS_UINT32                                              ulRedialCnt;
    NAS_ERR_LOG_CDMA_PS_CALL_REDIAL_INFO_STRU               astRedialInfo[NAS_ERR_LOG_MAX_CDMA_PS_CALL_EXCEP_REDIAL_NUM];
}NAS_ERR_LOG_CDMA_PS_DISC_RECORD_STRU;


typedef struct
{
    OM_ERR_LOG_HEADER_STRU                                  stHeader;
    VOS_UINT32                                              ulCdmaPsDiscRecdCnt;         /* PS断链后，发起链接重建失败次数 */
    NAS_ERR_LOG_CDMA_PS_DISC_RECORD_STRU                    astErrLogCdmaPsDiscRecd[NAS_ERR_LOG_MAX_CDMA_PS_DISC_RECORD_NUM];    /* PS断链异常信息 */
}NAS_ERR_LOG_CDMA_PS_DISC_INFO_STRU;


typedef struct
{
    VOS_UINT32                          ulIndex;         /* 结构体数组索引*/
    VOS_UINT32                          ulStartTimer;    /* 本次建链开始时间*/
    VOS_UINT32                          ulEndTimer;      /* 本次建链结束时间*/
    VOS_UINT32                          ulRedialRslt;    /* 失败原因值 */
} NAS_ERR_LOG_1X_CS_CALL_REDIAL_INFO_STRU;



typedef struct
{
    OM_ERR_LOG_HEADER_STRU                                  stHeader;
    VOS_UINT32                                              ulCcCause;           /* 呼叫最后一次失败原因值 */
    VOS_UINT32                                              ulRedialCnt;         /* 重拨次数*/
    NAS_ERR_LOG_1X_CS_CALL_REDIAL_INFO_STRU                 astRedialInfo[NAS_ERR_LOG_1X_CS_CALL_MAX_REDIAL_NUM];
}NAS_ERR_LOG_1X_CS_CALL_EXCEPTION_STRU;


typedef struct
{
    OM_ERR_LOG_HEADER_STRU               stHeader;
    VOS_UINT32                           ulCcCause;      /* 在主叫的原因值上扩展 */
    VOS_UINT32                           ulPagingTimer;  /* 接收到Paging消息的时间 */
    VOS_UINT32                           ulPagingType;   /* 被叫业务类型，区分语音，短信和数据*/
    VOS_UINT32                           ulXccState;     /* 如果异常出现在XCC模块的被叫状态机中，返回XCC当前的子状态*/
}NAS_ERR_LOG_1X_MT_SERVICE_EXCEPTION_STRU;




/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/


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

#endif /* end of NasErrorLoCdmag.h */


