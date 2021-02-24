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

#ifndef __NAS_ERRORLOGCOMM_H__
#define __NAS_ERRORLOGCOMM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "errorlog.h"
#include "omerrorlog.h"
#include "errlog_cfg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define NAS_ERR_LOG_CTRL_LEVEL_NULL                     (0)                     /* ErrLog等级未定义*/
#define NAS_ERR_LOG_CTRL_LEVEL_CRITICAL                 (1)                     /* ErrLog等级为紧急 */
#define NAS_ERR_LOG_CTRL_LEVEL_MAJOR                    (2)                     /* ErrLog等级为重要 */
#define NAS_ERR_LOG_CTRL_LEVEL_MINOR                    (3)                     /* ErrLog等级为次要 */
#define NAS_ERR_LOG_CTRL_LEVEL_WARNING                  (4)                     /* ErrLog等级为提示 */


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


enum NAS_ERR_LOG_CALL_EX_STATE_ENUM
{
    NAS_ERR_LOG_CALL_S_ACTIVE,                                                  /* active */
    NAS_ERR_LOG_CALL_S_HELD,                                                    /* held */
    NAS_ERR_LOG_CALL_S_DIALING,                                                 /* dialing (MO call) */
    NAS_ERR_LOG_CALL_S_ALERTING,                                                /* alerting (MO call) */
    NAS_ERR_LOG_CALL_S_INCOMING,                                                /* incoming (MT call) */
    NAS_ERR_LOG_CALL_S_WAITING,                                                 /* waiting (MT call) */
    NAS_ERR_LOG_CALL_S_IDLE,                                                    /* idle */
    NAS_ERR_LOG_CALL_S_CCBS_WAITING_ACTIVE,                                     /* CCBS等待激活态 */
    NAS_ERR_LOG_CALL_S_CCBS_WAITING_RECALL,                                     /* CCBS等待回呼态 */
    NAS_ERR_LOG_CALL_S_UNKNOWN,                                                 /* unknown state */
    NAS_ERR_LOG_CALL_S_MO_FDN_CHECK,                                            /* FDN check */
    NAS_ERR_LOG_CALL_S_MO_CALL_CONTROL_CHECK,                                   /* CALL CONTROL check */

    NAS_ERR_LOG_CALL_S_BUTT
};
typedef VOS_UINT8  NAS_ERR_LOG_CALL_STATE_ENUM_U8;


enum NAS_ERR_LOG_UNSOLI_REPORT_TYPE_ENUM
{
    NAS_ERR_LOG_FAULT_REPORT                                = 0x01,             /* 故障上报 */
    NAS_ERR_LOG_ALARM_REPORT                                = 0x02,             /* 告警上报 */

    NAS_ERR_LOG_REPORT_TYPE_BUTT
};
typedef VOS_UINT16  NAS_ERR_LOG_UNSOLI_REPORT_TYPE_ENUM_U16;


enum NAS_ERR_LOG_ALM_TYPE_ENUM
{
    NAS_ERR_LOG_ALM_TYPE_COMMUNICATION    = 0x00,                               /* 通信 */
    NAS_ERR_LOG_ALM_TYPE_TRAFFIC_QUALITY  = 0x01,                               /* 业务质量 */
    NAS_ERR_LOG_ALM_TYPE_DEAL_WRONG       = 0x02,                               /* 处理出错 */
    NAS_ERR_LOG_ALM_TYPE_DEVICE_FAILURE   = 0x03,                               /* 设备故障 */
    NAS_ERR_LOG_ALM_TYPE_ENV_FAILURE      = 0x04,                               /* 环境故障 */
    NAS_ERR_LOG_ALM_TYPE_BUTT
};
typedef VOS_UINT16  NAS_ERR_LOG_ALM_TYPE_ENUM_U16;


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
    VOS_UINT32                          ulFsmId;                                /* 当前状态机标识 */
    VOS_UINT32                          ulState;                                /* 当前状态 */
}NAS_ERR_LOG_APS_FSM_CTX_STRU;


typedef struct
{
    NAS_ERR_LOG_APS_FSM_CTX_STRU        stMainFsm;                              /* 主状态机 */
    NAS_ERR_LOG_APS_FSM_CTX_STRU        stSubFsm;                               /* 子状态机 */
    VOS_UINT8                           ucPdpId;                                /* 当前状态机所对应的PDP ID */
    VOS_UINT8                           aucReserve[3];
}NAS_ERR_LOG_APS_PDP_ENTITY_FSM_INFO_STRU;


typedef struct
{
    NAS_ERR_LOG_ALM_ID_ENUM_U16             enAlmID;        /* 异常模块ID */
    VOS_UINT16                              usLogLevel;     /* 上报log等级 */
}NAS_ERR_LOG_ALM_LEVEL_STRU;


typedef struct
{
    NAS_ERR_LOG_ALM_ID_ENUM_U16                             enAlmID;            /* 异常模块ID */
    NAS_ERR_LOG_UNSOLI_REPORT_TYPE_ENUM_U16                 enReportType;       /* 上报log等级 */
}NAS_ERR_LOG_ALM_REPORT_TYPE_STRU;


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

#endif /* end of NasErrorLogComm.h */


