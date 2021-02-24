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

#ifndef  APP_VC_API_H
#define  APP_VC_API_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "TafTypeDef.h"
#include "vos.h"
#include "NasNvInterface.h"
#include "TafNvInterface.h"

/*****************************************************************************
  2 常量定义
*****************************************************************************/
#define APP_VC_MSD_DATA_LEN             (140)                                   /* 单位:btye */
#if(FEATURE_ON == MBB_WPG_PCM)
#define MN_CALL_PCM_FR_NUM                    (116)
#endif /* FEATURE_ON == MBB_WPG_PCM */


/*****************************************************************************
  3类型定义
*****************************************************************************/
#if(FEATURE_ON == MBB_WPG_PCM)
enum APP_VC_CPCM_MODE_ENUM
{
    APP_VC_CPCM_MODE_SLAVE      = 0,
    APP_VC_CPCM_MODE_MASTER     = 1,
    APP_VC_CPCM_MODE_BUTT
};
typedef VOS_UINT16    APP_VC_CPCM_MODE_ENUM_UINT16;

enum APP_VC_STN_ENUM
{
    APP_VC_STN_CLOSE            = 0,
    APP_VC_STN_ON               = 1,
    APP_VC_STN_BUTT
};
typedef VOS_UINT8 APP_VC_STN_ENUM_UINT8;

enum APP_VC_STATUS_ENUM
{
    APP_VC_STATUS_CLOSE            = 0,
    APP_VC_STATUS_ON               = 1,
    APP_VC_STATUS_BUTT
};
typedef VOS_UINT8 APP_VC_STATUS_ENUM_UINT8;

enum APP_VC_ECHOEX_TMODE_ENUM
{
    APP_VC_ECHOEX_TMODE_NOCHANGE                 = 0,       /* 不对对下行补偿、噪声抑制和回声抑制参数进行调整*/
    APP_VC_ECHOEX_TMODE_DOUBLE_MIC,                         /* 双讲模式 */
    APP_VC_ECHOEX_TMODE_SINGLE_MIC,                         /* 单讲模式 */
    APP_VC_ECHOEX_TMODE_BUTT
};
typedef VOS_UINT16 APP_VC_ECHOEX_TMODE_ENUM_UINT16;

typedef struct
{
    VOS_INT16                           ucEchoEnable;                     /* 回声抑制开关 */
    VOS_INT16                           uEchoTime;                          /*单讲拖尾长度*/
    VOS_INT16                           uEchoSingleLen;                    /*单讲拖尾长度*/
    VOS_INT16                           uEchoSingleDecStr;             /*拖尾残留抑制强度*/
    VOS_INT16                           uEchoDoubleDecStr;            /*双讲回音抑制强度*/
    VOS_INT16                           aucRsv[1];                        /* 保留位 */
}AT_ECHOEX_STRU;
#endif /* FEATURE_ON == MBB_WPG_PCM */
/* 语音设备：0 手持；1 手持免提；2 车载免提；3 耳机；4 蓝牙；7 超级免提；8 智音通话 */
enum VC_PHY_DEVICE_MODE_ENUM
{
    VC_PHY_DEVICE_MODE_HANDSET,                                                 /* 手持 */
    VC_PHY_DEVICE_MODE_HANDS_FREE,                                              /* 手持免提 */
    VC_PHY_DEVICE_MODE_CAR_FREE,                                                /* 车载免提 */
    VC_PHY_DEVICE_MODE_EARPHONE,                                                /* 耳机 */
    VC_PHY_DEVICE_MODE_BLUETOOTH,                                               /* 蓝牙 */
    VC_PHY_DEVICE_MODE_PCVOICE,                                                 /* PC语音模式 */
    VC_PHY_DEVICE_MODE_HEADPHONE,                                               /* 不带耳机MIC的耳机通话 */
    VC_PHY_DEVICE_MODE_SUPER_HANDFREE,                                              /* 超级免提 */
    VC_PHY_DEVICE_MODE_SMART_TALK,                                              /* 智音通话 */
    VC_PHY_DEVICE_MODEM_USBVOICE,                                               /* USB设备 */
    VC_PHY_DEVICE_MODE_BUTT                                                     /* invalid value */
};
typedef VOS_UINT16  VC_PHY_DEVICE_MODE_ENUM_U16;

/*****************************************************************************
 枚举名    : APP_VC_VOICE_MODE_ENUM
 结构说明  : STICK形态下，用户通过^CVOICE私有命令设置的语音模式枚举
*****************************************************************************/
enum APP_VC_VOICE_MODE_ENUM
{
    APP_VC_VOICE_MODE_PCVOICE           = 0,
    APP_VC_VOICE_MODE_EARPHONE          = 1,
    APP_VC_VOICE_MODE_HANDSET           = 2,
    APP_VC_VOICE_MODE_HANDS_FREE        = 3,
    APP_VC_VOICE_MODE_BUTT
};
typedef VOS_UINT16 APP_VC_VOICE_MODE_ENUM_U16;

/*****************************************************************************
 枚举名    : APP_VC_VOICE_PORT_ENUM
 结构说明  : 语言输出端口枚举
*****************************************************************************/
enum APP_VC_VOICE_PORT_ENUM
{
    APP_VC_VOICE_PORT_MODEM             = 1,
    APP_VC_VOICE_PORT_DIAG              = 2,
    APP_VC_VOICE_PORT_PCUI              = 3,
    APP_VC_VOICE_PORT_PCVOICE           = 4,
    APP_VC_VOICE_PORT_PCSC              = 5,
    APP_VC_VOICE_PORT_BUTT
};
typedef VOS_UINT8 APP_VC_VOICE_PORT_ENUM_U8;

enum APP_VC_EVENT_ENUM
{
    APP_VC_EVT_PARM_CHANGED,                                                /*service para changed*/
    APP_VC_EVT_SET_VOLUME,
    APP_VC_EVT_SET_VOICE_MODE,
    APP_VC_EVT_GET_VOLUME,
    APP_VC_EVT_SET_MUTE_STATUS,
    APP_VC_EVT_GET_MUTE_STATUS,

    APP_VC_EVT_SET_ECALL_CFG,
    APP_VC_EVT_ECALL_TRANS_STATUS,
#if(FEATURE_ON == MBB_WPG_PCM)
    APP_VC_EVT_SET_SMUT_STATUS,
    APP_VC_EVT_GET_SMUT_STATUS,
    APP_VC_EVT_SET_STN_STATUS,
    APP_VC_EVT_GET_STN_STATUS,
    APP_VC_EVT_SET_NSSWITCH_STATUS,
    APP_VC_EVT_GET_NSSWITCH_STATUS,
    APP_VC_EVT_SET_UP_VOLUME,
    APP_VC_EVT_GET_CMIC_STATUS,
    APP_VC_EVT_SET_ECHOEX,
    APP_VC_EVT_GET_ECHOEX,
    APP_VC_EVT_SET_PCMFR,
    APP_VC_EVT_GET_PCMFR,
#endif /* FEATURE_ON == MBB_WPG_PCM */

#if (FEATURE_ON == MBB_WPG_ECALL)
    APP_VC_EVT_ECALL_TRANS_ALACK,
#endif/*FEATURE_OFF == MBB_WPG_ECALL*/
    APP_VC_EVT_BUTT
};
typedef VOS_UINT32 APP_VC_EVENT_ENUM_U32;

enum APP_VC_MUTE_STATUS_ENUM
{
    APP_VC_MUTE_STATUS_UNMUTED          = 0,
    APP_VC_MUTE_STATUS_MUTED            = 1,
    APP_VC_MUTE_STATUS_BUTT
};
typedef VOS_UINT8 APP_VC_MUTE_STATUS_ENUM_UINT8;

#if (FEATURE_ON == FEATURE_DTMF)
enum APP_VC_DTMF_DETECT_ENUM
{
    APP_VC_DTMF_DETECT_OFF           = 0,
    APP_VC_DTMF_DETECT_ON            = 1,
    APP_VC_DTMF_DETECT_BUTT
};
typedef VOS_UINT8 APP_VC_DTMF_DETECT_ENUM_UINT8;
#endif


enum APP_VC_MSG_ENUM
{
    APP_VC_MSG_REQ_SET_VOLUME   = 0,                                            /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 设置音量请求消息 */
    APP_VC_MSG_REQ_SET_MODE,                                                    /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 设置通话模式请求消息 */

    APP_VC_MSG_REQ_QRY_MODE,                                                    /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 查询通话模式请求消息 */
    APP_VC_MSG_CNF_QRY_MODE,                                                    /* _H2ASN_MsgChoice APP_VC_EVENT_INFO_STRU *//* 查询通话模式回复消息 */

    APP_VC_MSG_REQ_SET_PORT,                                                    /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 设置语音端口请求消息 */
    APP_VC_MSG_CNF_SET_PORT,                                                    /* _H2ASN_MsgChoice APP_VC_EVENT_INFO_STRU *//* 设置语音端口回复消息 */

    APP_VC_MSG_REQ_QRY_PORT,                                                    /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 查询语音端口请求消息 */
    APP_VC_MSG_CNF_QRY_PORT,                                                    /* _H2ASN_MsgChoice APP_VC_EVENT_INFO_STRU *//* 查询语音端口回复消息 */

    APP_VC_MSG_REQ_QRY_VOLUME,                                                  /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU */
    APP_VC_MSG_CNF_QRY_VOLUME,

    APP_VC_MSG_SET_MUTE_STATUS_REQ,                                             /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 设置静音状态请求消息 */
    APP_VC_MSG_GET_MUTE_STATUS_REQ,                                             /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 获取静音状态请求消息 */

#if(FEATURE_ON == MBB_WPG_PCM)
    APP_VC_MSG_SET_SMUT_STATUS_REQ,                                             /* 设置下行静音状态请求消息 */
    APP_VC_MSG_GET_SMUT_STATUS_REQ,                                             /* 获取下行静音状态请求消息 */
    APP_VC_MSG_UPDATE_SMUT_STATUS_REQ,
    APP_VC_MSG_SET_STN_STATUS_REQ,                                              /* 设置侧音开关请求消息*/
    APP_VC_MSG_GET_STN_STATUS_REQ,                                              /* 获取侧音开关请求消息*/
    APP_VC_MSG_SET_NSSWITCH_STATUS_REQ,                                         /* 设置噪音抑制开关请求消息*/
    APP_VC_MSG_GET_NSSWITCH_STATUS_REQ,                                         /* 获取噪音抑制开关请求消息*/
    APP_VC_MSG_REQ_SET_UP_VOLUME,                                               /* 设置上行音量请求消息 */ 
    APP_VC_MSG_GET_CMIC_STATUS_REQ,                                             /* 获取上行音量请求消息 */
    APP_VC_MSG_SET_ECHOEX_REQ,                                                  /* 设置回声抑制请求消息*/
    APP_VC_MSG_GET_ECHOEX_REQ,                                                  /* 获取回声抑制请求消息*/
    APP_VC_MSG_SET_PCMFR_REQ, 
    APP_VC_MSG_GET_PCMFR_REQ,
#endif /* FEATURE_ON == MBB_WPG_PCM */
    APP_VC_MSG_SET_FOREGROUND_REQ,                                              /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 设置前台模式请求消息 */
    APP_VC_MSG_SET_FOREGROUND_CNF,                                              /* _H2ASN_MsgChoice APP_VC_MSG_CNF_ASN_STRU *//* 设置前台模式回复消息 */

    APP_VC_MSG_SET_BACKGROUND_REQ,                                              /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 设置后台模式请求消息 */
    APP_VC_MSG_SET_BACKGROUND_CNF,                                              /* _H2ASN_MsgChoice APP_VC_MSG_CNF_ASN_STRU *//* 设置后台模式回复消息 */

    APP_VC_MSG_FOREGROUND_QRY,                                                  /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 模式查询请求消息 */
    APP_VC_MSG_FOREGROUND_RSP,                                                  /* _H2ASN_MsgChoice APP_VC_QRY_GROUNG_RSP_STRU *//* 模式查询回复消息 */

    APP_VC_MSG_SET_MODEMLOOP_REQ,                                               /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 设置语音环回模式消息 */

    APP_VC_MSG_SET_MSD_REQ,                                                     /* _H2ASN_MsgChoice APP_VC_MSG_SET_MSD_REQ_STRU *//* 设置MSD数据请求 */
    APP_VC_MSG_SET_MSD_CNF,                                                     /* _H2ASN_MsgChoice APP_VC_MSG_CNF_ASN_STRU *//* 设置MSD数据回复 */

    APP_VC_MSG_QRY_MSD_REQ,                                                     /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 查询MSD数据请求 */
    APP_VC_MSG_QRY_MSD_CNF,                                                     /* _H2ASN_MsgChoice APP_VC_MSG_QRY_MSD_CNF_STRU *//* 查询MSD数据回复 */

    APP_VC_MSG_SET_ECALL_CFG_REQ,                                               /* _H2ASN_MsgChoice APP_VC_MSG_SET_ECALL_CFG_REQ_STRU *//* 设置ecall配置请求 */

    APP_VC_MSG_QRY_ECALL_CFG_REQ,                                               /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 查询ecall配置信息请求 */
    APP_VC_MSG_QRY_ECALL_CFG_CNF,                                               /* _H2ASN_MsgChoice APP_VC_MSG_QRY_ECALL_CFG_CNF_STRU *//* 查询ecall配置信息回复 */

#if (FEATURE_ON == FEATURE_DTMF)
    APP_VC_MSG_SET_DDTMF_CFG_REQ,                                       /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//*  设置DDTMF检测 */
    APP_VC_MSG_SET_DDTMF_CFG_CNF,                                       /* _H2ASN_MsgChoice APP_VC_MSG_CNF_ASN_STRU *//* 设置DDTMF检测回复 */
    APP_VC_MSG_QRY_DDTMF_CFG_REQ,                                       /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//*  查询DDTMF检测 */
    APP_VC_MSG_QRY_DDTMF_CFG_CNF,                                       /* _H2ASN_MsgChoice APP_VC_QRY_DTMFCFG_RSP_STRU *//* 设置DDTMF检测回复 */
#endif

#if (FEATURE_ON == MBB_SLIC)
    APP_VC_MSG_SEND_DTMF_MUTE_REQ,
#endif

#if (FEATURE_ON == MBB_WPG_ECALL)
    APP_VC_MSG_ECALL_PUSH_REQ,
    APP_VC_MSG_ECALL_PUSH_CNF,
    APP_VC_MSG_ECALL_OPRTMODE_REQ,
#endif/*FEATURE_OFF == MBB_WPG_ECALL*/
    APP_VC_MSG_DTMF_DECODER_IND,                                                /* _H2ASN_MsgChoice APP_VC_DTMF_DECODER_IND_STRU *//* DTMF Decoder Ind上报消息 */

    APP_VC_MSG_SET_TTYMODE_REQ,                                                 /* _H2ASN_MsgChoice APP_VC_SET_TTYMODE_REQ_STRU *//* 设置TTY模式请求消息 */
    APP_VC_MSG_SET_TTYMODE_CNF,                                                 /* _H2ASN_MsgChoice APP_VC_MSG_CNF_ASN_STRU *//* 设置TTY模式回复消息 */
    APP_VC_MSG_QRY_TTYMODE_REQ,                                                 /* _H2ASN_MsgChoice APP_VC_REQ_MSG_ASN_STRU *//* 查询TTY模式请求消息 */
    APP_VC_MSG_QRY_TTYMODE_CNF,                                                 /* _H2ASN_MsgChoice APP_VC_QRY_TTYMODE_CNF_STRU *//* 查询TTY模式回复消息 */

    APP_VC_MSG_BUTT
};
typedef VOS_UINT16  APP_VC_MSG_ENUM_U16;


enum APP_VC_QRY_GROUND_ENUM
{
    APP_VC_FOREGROUND                   = 0,
    APP_VC_BACKGROUND                   = 1,
    APP_VC_GROUND_INVALID               = 2,
    APP_VC_GROUND_BUTT
};
typedef VOS_UINT16 APP_VC_QRY_GROUND_ENUM_U16;


enum APP_VC_START_HIFI_ORIG_ENUM
{
    APP_VC_START_HIFI_ORIG_CALL                   = 0,
    APP_VC_START_HIFI_ORIG_IMSA                   = 1,
    APP_VC_START_HIFI_ORIG_BUTT
};
typedef VOS_UINT8 APP_VC_START_HIFI_ORIG_ENUM_UINT8;


enum APP_VC_ECALL_TRANS_STATUS_ENUM
{
    APP_VC_ECALL_MSD_TRANSMITTING_START     = 0,                                /* 当前MSD数据已经开始发送 */
    APP_VC_ECALL_MSD_TRANSMITTING_SUCCESS,                                      /* 当前MSD数据已经发送成功 */
    APP_VC_ECALL_MSD_TRANSMITTING_FAIL,                                         /* 当前MSD数据已经发送失败 */
    APP_VC_ECALL_PSAP_MSD_REQUIRETRANSMITTING,                                  /* PSAP要求更新MSD数据 */
    APP_VC_ECALL_TRANS_STATUS_BUTT
};
typedef VOS_UINT8  APP_VC_ECALL_TRANS_STATUS_ENUM_UINT8;


enum APP_VC_ECALL_TRANS_FAIL_CAUSE_ENUM
{
    APP_VC_ECALL_CAUSE_WAIT_PSAP_TIME_OUT   = 0,                                /* 等待PSAP的传输指示超时 */
    APP_VC_ECALL_CAUSE_MSD_TRANSMITTING_TIME_OUT,                               /* MSD数据传输超时 */
    APP_VC_ECALL_CAUSE_WAIT_AL_ACK_TIME_OUT,                                    /* 等待应用层的确认超时 */
    APP_VC_ECALL_CAUSE_UNSPECIFIC_ERROR,                                        /* 其他异常错误 */
    APP_VC_ECALL_CAUSE_BUTT
};
typedef VOS_UINT8  APP_VC_ECALL_TRANS_FAIL_CAUSE_ENUM_UINT8;


enum APP_VC_ECALL_MSD_MODE_ENUM
{
    APP_VC_ECALL_MSD_MODE_TRANSPARENT   = 0,                                    /* 透传模式 */
    APP_VC_ECALL_MSD_MODE_BUTT
};
typedef VOS_UINT16  APP_VC_ECALL_MSD_MODE_ENUM_UINT16;


enum APP_VC_ECALL_VOC_CONFIG_ENUM
{
    APP_VC_ECALL_VOC_CONFIG_NOT_ABANDON = 0,                                    /* MSD数据传输过程带语音*/
    APP_VC_ECALL_VOC_CONFIG_ABANDON,                                            /* MSD数据传输过程不带语音*/
    APP_VC_ECALL_VOC_CONFIG_BUTT
};
typedef VOS_UINT16  APP_VC_ECALL_VOC_CONFIG_ENUM_UINT16;



typedef struct
{
    VOS_MSG_HEADER                                                              /* _H2ASN_Skip */
    APP_VC_MSG_ENUM_U16                 enMsgName;                              /* _H2ASN_Skip *//* 消息名 */
    VOS_UINT8                           aucReserve1[2];                         /* 保留 */
    MN_CLIENT_ID_T                      clientId;                               /* Client ID */
    MN_OPERATION_ID_T                   opId;                                   /* Operation ID */
    VOS_UINT8                           aucReserve2;                            /* 保留 */
    VOS_UINT8                           aucContent[4];                          /* 消息内容 */
#if(FEATURE_ON == MBB_WPG_PCM) 
    VOS_INT16                           ucEchoEnable;                     /* 回声抑制开关 */
    VOS_INT16                           uEchoTime;                          /*单讲拖尾长度*/
    VOS_INT16                           uEchoSingleLen;                    /*单讲拖尾长度*/
    VOS_INT16                           uEchoSingleDecStr;             /*拖尾残留抑制强度*/
    VOS_INT16                           uEchoDoubleDecStr;            /*双讲回音抑制强度*/
    VOS_INT16                           ashwCompTx[(MN_CALL_PCM_FR_NUM >> 1) / sizeof(VOS_INT16)];
    VOS_INT16                           ashwCompRx[(MN_CALL_PCM_FR_NUM >> 1) / sizeof(VOS_INT16)];
    VOS_INT16                           aucRsv[1];                        /* 保留位 */
#endif /* FEATURE_ON == MBB_WPG_PCM */
} APP_VC_REQ_MSG_STRU;



typedef struct
{
    VOS_BOOL                            bSuccess;                               /*set(get) success or failure */
    VOS_UINT32                          ulErrCode;
    MN_CLIENT_ID_T                      clientId;
    MN_OPERATION_ID_T                   opId;
    APP_VC_VOICE_PORT_ENUM_U8           enVoicePort;
    VC_PHY_DEVICE_MODE_ENUM_U16         enDevMode;
    VOS_UINT16                          usVolume;
    APP_VC_MUTE_STATUS_ENUM_UINT8       enMuteStatus;
    VOS_UINT8                           aucRsv[2];

    APP_VC_ECALL_TRANS_STATUS_ENUM_UINT8 enEcallState;
    VOS_UINT32                          ulEcallDescription;
#if(FEATURE_ON == MBB_WPG_PCM)
    APP_VC_MUTE_STATUS_ENUM_UINT8       enSmutStatus;
    APP_VC_STN_ENUM_UINT8               enCurrStnStatus;
    VOS_UINT16                          usUpVolume;
    VOS_UINT8                           ucCurrNsswitchStatus;
    VOS_INT16                           aucEcho[5];
    VOS_UINT16                          usPcmFr[MN_CALL_PCM_FR_NUM / 2];
#endif /* FEATURE_ON == MBB_WPG_PCM */

#if (FEATURE_ON == MBB_WPG_ECALL)
    VOS_UINT8                               enEcallReportAlack;
    VOS_UINT8                               aucReserved[3]; 
#endif/*FEATURE_OFF == MBB_WPG_ECALL*/

}APP_VC_EVENT_INFO_STRU;


#if (FEATURE_ON == MBB_WPG_ECALL)
#define MN_ECALL_ALACK_NUM                    (20)

typedef struct
{
    VOS_UINT8                                 auEcallAlackValue;
    NAS_MM_INFO_IND_STRU         stEcallAlackTimeInfo;
}AT_ECALL_ALACK_VALUE_STRU;

typedef struct
{
    VOS_UINT8                                auEcallAlackNum;
    VOS_UINT8                                auEcallAlackBeginNum;
    VOS_UINT8                                aucReserved[2];
    AT_ECALL_ALACK_VALUE_STRU     stEcallAlackInfo[MN_ECALL_ALACK_NUM];
}AT_ECALL_ALACK_INFO_STRU;
#endif/*FEATURE_OFF == MBB_WPG_ECALL*/


typedef struct
{
    VOS_UINT8                           ucQryRslt;                              /* 查询 success or failure */
    VOS_UINT8                           aucReserved;
    APP_VC_QRY_GROUND_ENUM_U16          enGround;                               /* 查询结果 */
}APP_VC_QRY_GROUNG_RSP_STRU;

#if (FEATURE_ON == FEATURE_DTMF)

typedef struct
{
    VOS_UINT8                                           ucQryRslt;                              /* 查询 success or failure */
    APP_VC_DTMF_DETECT_ENUM_UINT8                       enDtmfDetect;                           /* 查询结果 */
    VOS_UINT8                                           aucReserved[2];                         /*align*/
}APP_VC_QRY_DTMFCFG_RSP_STRU;
#endif


typedef struct
{
    VOS_UINT8                           aucMsdData[APP_VC_MSD_DATA_LEN];        /* MSD数据内容 */
}APP_VC_MSG_SET_MSD_REQ_STRU;


typedef struct
{
    VOS_UINT8                           ucQryRslt;                              /* 查询结果 */
    VOS_UINT8                           aucReserved[3];
    VOS_UINT8                           aucMsdData[APP_VC_MSD_DATA_LEN];        /* MSD数据内容 */
}APP_VC_MSG_QRY_MSD_CNF_STRU;


typedef struct
{
    APP_VC_ECALL_MSD_MODE_ENUM_UINT16   enMode;                                 /* 数据传送模式 */
    APP_VC_ECALL_VOC_CONFIG_ENUM_UINT16 enVocConfig;                            /* 传送数据时是否打开声码器 */
#if (FEATURE_ON == MBB_WPG_ECALL)
    APP_VC_ECALL_VOC_CONFIG_ENUM_UINT16 enRedialConfig;
    VOS_UINT8                                               aucReserved[2];
#endif/*FEATURE_OFF == MBB_WPG_ECALL*/
}APP_VC_MSG_SET_ECALL_CFG_REQ_STRU;



typedef struct
{
    VOS_UINT8                           ucQryRslt;                              /* 查询结果 */
    VOS_UINT8                           aucReserved[3];
    APP_VC_ECALL_MSD_MODE_ENUM_UINT16   enMode;                                 /* MSD数据传送模式 */
    APP_VC_ECALL_VOC_CONFIG_ENUM_UINT16 enVocConfig;                            /* 传送数据时是否打开声码器 */
#if (FEATURE_ON == MBB_WPG_ECALL)
     /* 语音重拨模式 */
    APP_VC_ECALL_MSD_MODE_ENUM_UINT16   enEcallRedialMode;
    VOS_UINT8                                              aucReserved1[2];
#endif/*FEATURE_OFF == MBB_WPG_ECALL*/
}APP_VC_MSG_QRY_ECALL_CFG_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucRslt;                                  /* 结果 */
    VOS_UINT8                           aucReserved[3];
}APP_VC_SET_MSD_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucQryRslt;                              /* 查询 success or failure */
    TAF_VC_TTYMODE_ENUM_UINT8           enTTYMode;                              /* 查询结果 */
    VOS_UINT8                           aucReserved[2];
}APP_VC_QRY_TTYMODE_CNF_STRU;


typedef struct
{
    TAF_VC_TTYMODE_ENUM_UINT8           enTTYMode;                              /* 设置TTY MODE*/
    VOS_UINT8                           aucReserved[3];
}APP_VC_SET_TTYMODE_REQ_STRU;


typedef struct
{
    VOS_UINT8                           ucRslt;
    VOS_UINT8                           aucReserved[3];
}APP_VC_MSG_CNF_ASN_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[4];
}APP_VC_REQ_MSG_ASN_STRU;

#if(FEATURE_ON == MBB_WPG_PCM)
typedef struct
{
    VOS_UINT8            ucPcmFr[MN_CALL_PCM_FR_NUM];                     /*频响参数 */
}AT_PCMFR_STRU; 
#endif /* FEATURE_ON == MBB_WPG_PCM */

/*****************************************************************************
  4 宏定义
*****************************************************************************/
#if(FEATURE_ON == MBB_WPG_PCM)
#define CPCM_PARA_INDEX_NUMER       (5)
#define CPCM_MASTER_MODE            (0)
#define CPCM_SLAVE_MODE             (2)
#define NO_VALUE                    (2001)
#define ECHOEX_VALUE_MAX            (960)
#define ECHOEX_TMODEPARA1_MAX       (200)
#define ECHOEX_TMODEPARA2_MAX       (2000)
#endif /* FEATURE_ON == MBB_WPG_PCM */
/*****************************************************************************
  5 全局变量声明
*****************************************************************************/


/*****************************************************************************
  6 接口函数声明
*****************************************************************************/


VOS_UINT32  APP_VC_SetVoiceVolume(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    VOS_UINT8                           ucVoiceVolume
);


VOS_UINT32  APP_VC_SetVoiceMode(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    VOS_UINT8                           ucVoiceMode
);


VOS_UINT32 APP_VC_SetVoicePort(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    APP_VC_VOICE_PORT_ENUM_U8           ucVoicePort
);


VOS_UINT32 APP_VC_GetVoicePort(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId
);


VOS_UINT32 APP_VC_GetVoiceMode(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId
);


extern  VC_PHY_DEVICE_MODE_ENUM_U16  APP_VC_AppVcVoiceMode2VcPhyVoiceMode(
    APP_VC_VOICE_MODE_ENUM_U16          usVoiceMode
);


extern  APP_VC_VOICE_MODE_ENUM_U16  APP_VC_VcPhyVoiceMode2AppVcVoiceMode(
    VC_PHY_DEVICE_MODE_ENUM_U16         usVoiceMode
);

extern VOS_UINT32  APP_VC_GetVoiceVolume(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId
);

/*****************************************************************************
 函 数 名  : APP_VC_SetMuteStatus
 功能描述  : 设置静音状态
 输入参数  : usClientId   - 客户端ID
             ucOpId       - 操作码ID
             enMuteStatus - 静音状态
 输出参数  : 无
 返 回 值  : VOS_OK       - 成功
             VOS_ERR      - 失败
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32  APP_VC_SetMuteStatus(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId,
    APP_VC_MUTE_STATUS_ENUM_UINT8       enMuteStatus
);


/*****************************************************************************
 函 数 名  : APP_VC_GetMuteStatus
 功能描述  : 获取静音状态
 输入参数  : usClientId   - 客户端ID
             ucOpId       - 操作码ID
 输出参数  : 无
 返 回 值  : VOS_OK       - 成功
             VOS_ERR      - 失败
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 APP_VC_GetMuteStatus(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId
);

/* Added   for VOICE_LOOP, 2013/07/05, begin */
/*****************************************************************************
 函 数 名  : APP_VC_SetModemLoop
 功能描述  : 设置进入或退出语音换回
 输入参数  : usClientId   - 客户端ID
             ucOpId       - 操作码ID
             ucModemLoop  - 进入或退出语音环回状态
 输出参数  : 无
 返 回 值  : VOS_OK       - 成功
             VOS_ERR      - 失败
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 APP_VC_SetModemLoop(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucModemLoop
);
/* Added   for VOICE_LOOP, 2013/07/05, end */


typedef struct
{
    VOS_CHAR                            ucDtmfCode;
    VOS_UINT8                           ucReserved;
    VOS_UINT16                          usOntime;
}APP_VC_DTMF_DECODER_IND_STRU;

#if (FEATURE_ON == FEATURE_DTMF)
VOS_UINT32 APP_VC_SetDDtmfCfg(
    VOS_UINT16                                           usClientId,
    VOS_UINT8                                            ucOpId,
    APP_VC_DTMF_DETECT_ENUM_UINT8                        enDetectOn
);

VOS_UINT32 APP_VC_QryDDtmfCfg(
    VOS_UINT16                                           usClientId,
    VOS_UINT8                                            ucOpId
);

#endif

#if 0 /* 转换AT到VC的消息时，请打开 */

/*****************************************************************************
  H2ASN顶级消息结构定义
*****************************************************************************/
typedef struct
{
    APP_VC_MSG_ENUM_U16                 enMsgName;                              /* _H2ASN_MsgChoice_Export APP_VC_MSG_ENUM_UINT16 */
    VOS_UINT8                           aucReserve1[2];
    MN_CLIENT_ID_T                      clientId;
    MN_OPERATION_ID_T                   opId;
    VOS_UINT8                           aucReserve2;
    VOS_UINT8                           aucMsg[4];
    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          APP_VC_MSG_ENUM_UINT16
    ****************************************************************************/
} VC_APP_MSG_DATA;

/* _H2ASN_Length UINT32 */

typedef struct
{
    VOS_MSG_HEADER
    VC_APP_MSG_DATA                     stMsgData;
} AppVcApi_MSG;
#endif

#if 0 /* 转换VC到AT EVENT事件 ASN时，请打开，并且将文件名修改为 AppVcEvent.h */
/*****************************************************************************
  H2ASN顶级消息结构定义
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usMsgName;                              /* _H2ASN_MsgChoice_Export APP_VC_MSG_ENUM_UINT16 */
    VOS_UINT8                           aucReserved1[2];
    MN_CLIENT_ID_T                      clientId;
    VOS_UINT16                          usLen;
    VOS_UINT8                           aucContent[4];
    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          APP_VC_MSG_ENUM_UINT16
    ****************************************************************************/
} APP_VC_EVENT_DATA;
/* _H2ASN_Length UINT32 */


typedef struct
{
    VOS_MSG_HEADER
    APP_VC_EVENT_DATA stMsgData;
}AppVcEvent_MSG;
#endif
#if(FEATURE_ON == MBB_WPG_PCM)
VOS_UINT32 APP_VC_SetSmutStatus(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId,
    APP_VC_MUTE_STATUS_ENUM_UINT8       enMuteStatus
);

VOS_UINT32 APP_VC_GetSmutStatus(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId
);

VOS_UINT32 APP_VC_UpdateMuteStatus(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucMute
);

VOS_UINT32 APP_VC_SetStnStatus(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucStnStatus
);

VOS_UINT32 APP_VC_GetStnStatus(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId
);

VOS_UINT32 APP_VC_SetNsswitchStatus(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucNsswitchStatus
);

VOS_UINT32 APP_VC_GetNsswitchStatus(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId
);

VOS_UINT32  APP_VC_SetUpVoiceVolume(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    VOS_UINT8                           ucVoiceVolume
);

VOS_UINT32 APP_VC_GetCmicStatus(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId
);

VOS_UINT32 APP_VC_SetEchoex(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId,
    AT_ECHOEX_STRU                      stEchoex
);

VOS_UINT32 APP_VC_GetEchoex(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId
);
VOS_UINT32 APP_VC_SetPcmFr(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId,
    AT_PCMFR_STRU                       stPcmfr
);

VOS_UINT32 APP_VC_GetPcmFr(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId
);
#endif /* FEATURE_ON == MBB_WPG_PCM */
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

#endif /* APP_VC_API_H */

