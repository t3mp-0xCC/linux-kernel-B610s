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

#ifndef __IMSACODECINTERFACE_H__
#define __IMSACODECINTERFACE_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
/* _H2ASN_Include v_typedef.h */
/* _H2ASN_Include ImsaCodecInterface.h */
#include "vos.h"
#include "CodecInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define IMSA_CODEC_MAX_DATA_LEN (32)				/* ��λ��16 bit */
#define IMSA_RTP_MAX_DATA_LEN (256)
#define IMSA_RTP_MAX_DATA_SIZE (IMSA_RTP_MAX_DATA_LEN * sizeof(uint8_t))
#define IMSA_RTP_MAX_FRAG (16)

/*
IMSA_TX_DATA_STRU�ṹ��������
usFrameType     --���б�֡��FrameType
ausData         --IF1Э���е�A��B��C������ABC�������У�Byte����
*/
#define IMSA_TX_DATA_STRU														\
	VOS_UINT16						usMsgId;		/* ԭ������ */ /*_H2ASN_Skip*/\
	VOS_UINT16						usReserve;		/* ������ */							  \
	CODEC_AMR_TYPE_TX_ENUM_UINT16	usStatus;		/* AMR���������� */							\
	VOS_UINT16						usFrameType;	/* ֡���� */ 						  \
	VOS_UINT16						ausData[IMSA_CODEC_MAX_DATA_LEN];

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

enum IMSA_VOICE_MSG_ID_ENUM
{
	/*====== VOICE->IMSA ======*/
	ID_VOICE_IMSA_TX_DATA_IND                       = 0x49A8, /* IMS�������������� */           /* _H2ASN_MsgChoice VOICE_IMSA_TX_DATA_IND_STRU */
	ID_VOICE_IMSA_CFG_CNF                           = 0x49A9, /* ����HIFI���еĽ������ڽ�� */    /* _H2ASN_MsgChoice VOICE_IMSA_CFG_CNF_STRU */
    ID_VOICE_IMSA_RX_DATA_ACK                       = 0x49AA,                   /* �ظ�RX_DATA������ */           /* _H2ASN_MsgChoice VOICE_IMSA_RX_DATA_ACK_STRU */
	ID_VOICE_IMSA_RTP_TX_DATA_IND                   = 0x4939,
	ID_VOICE_IMSA_FAIL_IND                          = 0x493A,
	ID_VOICE_IMSA_ENGINE_STATUS_NTF                 = 0x493B,

	/*====== IMSA->VOICE ======*/
	ID_IMSA_VOICE_RX_DATA_IND                       = 0x94A8, /* IMS������������ */               /* _H2ASN_MsgChoice IMSA_VOICE_RX_DATA_IND_STRU */
	ID_IMSA_VOICE_CFG_REQ                           = 0x94A9, /* ����HIFI���еĽ������� */        /* _H2ASN_MsgChoice IMSA_VOICE_CFG_REQ_STRU */
	ID_IMSA_VOICE_OPEN_ENGINE_IND                   = 0x9430,
	ID_IMSA_VOICE_CLOSE_ENGINE_IND                  = 0x9431,
	ID_IMSA_VOICE_SET_PAYLOAD_ENGINE_IND            = 0x9434,
	ID_IMSA_VOICE_SET_DTMF_PAYLOAD_ENGINE_IND       = 0x9435,
	ID_IMSA_VOICE_SET_PARAM_ENGINE_IND              = 0x9436,
	ID_IMSA_VOICE_SEND_DTMF_START_ENGINE_IND        = 0x9438,
	ID_IMSA_VOICE_RTP_RX_DATA_IND                   = 0x9439,

	IMSA_VOICE_MSG_ID_BUTT
};
typedef VOS_UINT16 IMSA_VOICE_MSG_ID_ENUM_UINT16;

enum IMSA_VOICE_ERRORFLAG_ENUM
{
	IMSA_VOICE_NO_ERROR = 0,	/* Jitter Buffer������ */
	IMSA_VOICE_ERROR,			/* Jitter Bufferû������ */
	IMSA_VOICE_ERRORFLAG_BUTT
};
typedef VOS_UINT16 IMSA_VOICE_ERRORFLAG_ENUM_UINT16;

enum IMSA_VOICE_CFG_ENUM
{
	IMSA_VOICE_CFG_SUCC = 0,		/* ���óɹ� */
	IMSA_VOICE_CFG_FAIL,			/* ����ʧ�� */
	IMSA_VOICE_CFG_BUTT
};
typedef VOS_UINT32 IMSA_VOICE_CFG_ENUM_UINT32;

enum IMSA_VOICE_IP_TYPE_ENUM
{
	IMSA_VOICE_IP_TYPE_IPV4 = 0,
	IMSA_VOICE_IP_TYPE_IPV6 = 1,
	IMSA_VOICE_IP_TYPE_BUTT
};
typedef VOS_UINT32 IMSA_VOICE_IP_TYPE_ENUM_UINT32;

enum IMSA_VOICE_SET_PARAM_ENGINE_ENUM
{
	IMSA_VOICE_CONFIG_SEND = 0,
	IMSA_VOICE_CONFIG_RECV,
	IMSA_VOICE_CONFIG_PLAY,
	IMSA_VOICE_CONFIG_RTCP_ENABLE,
	IMSA_VOICE_CONFIG_PARAM_BUTT,
};
typedef VOS_UINT32 IMSA_VOICE_SET_PARAM_ENGINE_ENUM_UINT32;

enum VOICE_IMSA_ENGINE_STATUS_ENUM
{
	VOICE_IMSA_ENGINE_RX_RTP_STATUS,
	VOICE_IMSA_ENGINE_STATUS_BUTT,
};
typedef VOS_UINT32 VOICE_IMSA_ENGINE_STATUS_ENUM_UINT32;

enum VOICE_IMSA_PARAM_ENUM
{
	VOICE_IMSA_OK = 0,
	VOICE_IMSA_NOK,
};
typedef VOS_UINT32 VOICE_IMSA_PARAM_ENUM_UINT32;

enum IMSA_VOICE_PARAM_ENUM
{
	IMSA_VOICE_DISABLE = 0,
	IMSA_VOICE_ENABLE,
};
typedef VOS_UINT32 IMSA_VOICE_PARAM_ENUM_UINT32;

enum IMSA_VOICE_PORT_ENUM
{
	IMSA_VOICE_PORT_RTP = 0,
	IMSA_VOICE_PORT_RTCP,
	IMSA_VOICE_PORT_BUTT,
};
typedef VOS_UINT32 IMSA_VOICE_PORT_ENUM_UINT32;

/*****************************************************************************
  4 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣ����
*****************************************************************************/

/*****************************************************************************
 ʵ������  : IMSA_VOICE_RX_DATA_IND_STRU
 ��������  : IMS��ȡIP�������RTP�����ͨ��IMSA���͸���Ϣ��VOICE�����ݷ��͹�ȥ
*****************************************************************************/
typedef struct
{
	VOS_MSG_HEADER																/*_H2ASN_Skip*/
	VOS_UINT16							usMsgId;								/* 0x94A8*/ /*_H2ASN_Skip*/
	VOS_UINT16							usSN;									/* �����к� */
	VOS_UINT32							ulTS;									/* ��ʱ��� */
	CODEC_ENUM_UINT16					usCodecType;							/* ���������� */
	CODEC_DTX_ENUM_UINT16				usDtxEnable;							/* ���б����DTX */
	VOS_UINT16							usRateMode; 							/* ���б����RateMode */
	IMSA_VOICE_ERRORFLAG_ENUM_UINT16	usErrorFlag;							/* Bufferû����ʱ��ErrorFlag */
	VOS_UINT16							usFrameType;							/* ���б�֡��FrameType */
	VOS_UINT16							usQualityIdx;							/* Quality indification */
	VOS_UINT16							ausData[IMSA_CODEC_MAX_DATA_LEN];		/* IF1Э���е�A��B��C������ABC�������У�Byte���� */
	VOS_UINT32							ulSSRC; 								/* SSRC��Ϣ */
	VOS_UINT32							ulNeedAckFlag;							/* �Ƿ���ҪHIFI�ظ�ack */
	VOS_UINT32							ulOpid;									/* id�� */
}IMSA_VOICE_RX_DATA_IND_STRU;

/*****************************************************************************
 ʵ������  : VOICE_IMSA_RX_DATA_ACK_STRU
 ��������  : ��ҪHIFI�ظ�ack,HIFI�Ļظ��ṹ��
*****************************************************************************/
typedef struct
{
	VOS_MSG_HEADER																/*_H2ASN_Skip*/
	VOS_UINT16							usMsgId;								/* 0x94AA*/ /*_H2ASN_Skip*/
	VOS_UINT16							usReserved; 							/* ������	*/
	VOS_UINT32							ulOpid;									/* id�� */
}VOICE_IMSA_RX_DATA_ACK_STRU;

/*****************************************************************************
 ʵ������  : VOICE_IMSA_TX_DATA_IND_STRU
 ��������  : VOICE���������ݴ���ͱ���OK��ͨ������Ϣ�����ݷ��͸�IMSA
*****************************************************************************/
typedef struct
{
	VOS_MSG_HEADER																/*_H2ASN_Skip*/
	IMSA_TX_DATA_STRU
}VOICE_IMSA_TX_DATA_IND_STRU;

/*****************************************************************************
 ʵ������  : IMSA_VOICE_CFG_REQ_STRU
 ��������  : ����HIFI���еĽ�������
*****************************************************************************/
typedef struct
{
	VOS_MSG_HEADER																/*_H2ASN_Skip*/
	VOS_UINT16							usMsgId;								/* 0x94a9 */ /*_H2ASN_Skip*/
	VOS_UINT16							usReserve;
	VOS_UINT32							ulTransTime;							/* ���к�IMSA�Ľ������� */
	VOS_UINT32							ulSendBitrate;							/* Ims���������� */
	VOS_UINT32							ulVoiceEncrypt; 						/* 0 = not Encrypt��1 = Encrypt�� */
}IMSA_VOICE_CFG_REQ_STRU;

/*****************************************************************************
 ʵ������  : IMSA_VOICE_CFG_CNF_STRU
 ��������  : IMSA_VOICE_CFG_REQ�Ļظ�
*****************************************************************************/

typedef struct
{
	VOS_MSG_HEADER																/*_H2ASN_Skip*/
	VOS_UINT16								usMsgId;							/* 0x49a9 */ /*_H2ASN_Skip*/
	VOS_UINT16								usReserve;
	IMSA_VOICE_CFG_ENUM_UINT32				ulResult;							/* ִ�н�� */
}VOICE_IMSA_CFG_CNF_STRU;

typedef struct
{
	VOS_MSG_HEADER
	VOS_UINT16									usMsgName;
	VOS_UINT16									usReserve;
	VOS_UINT32									ulChannelId;
	VOS_UINT32									ulIPType; /* 0: IPV4, 1: IPV6 */
 } IMSA_VOICE_OPEN_ENGINE_IND_STRU;

typedef struct
{
	VOS_MSG_HEADER
	VOS_UINT16									usMsgName;
	VOS_UINT16									usReserve;
	VOS_UINT32									ulChannelId;
} IMSA_VOICE_CLOSE_ENGINE_IND_STRU;

typedef struct
{
	VOS_MSG_HEADER
	VOS_UINT16									usMsgName;
	VOS_UINT16									usReserve;
	VOS_UINT32									ulChannelId;
	VOS_UINT32									ulCodecType;
	VOS_UINT32									ulRecivePayload; 			/* codec recive payload type */
	VOS_UINT32									ulSendPayload;				/* codec send payload type */
	VOS_UINT32									ulOctetAlign;				/* Octet Align */
	VOS_UINT32									ulBitRate;					/* bitrate */
	VOS_UINT32									ulSplRate;					/* sample rate (Hz) */
	VOS_UINT32									ulTsIncrease;				/* Ts increase value */
} IMSA_VOICE_SET_PAYLOAD_ENGINE_IND_STRU;

typedef struct
{
	VOS_MSG_HEADER
	VOS_UINT16									usMsgName;
	VOS_UINT16									usReserve;
	VOS_UINT32									ulChannelId;
	VOS_UINT32									ulDTMFRecivePayload; 		/* dtmf recive payload type */
	VOS_UINT32									ulDTMFSendPayload;			/* dtmf send payload type */
} IMSA_VOICE_SET_DTMF_PAYLOAD_ENGINE_IND_STRU;


typedef struct
{
	VOS_MSG_HEADER
	VOS_UINT16								usMsgName;
	VOS_UINT16								usReserve;
	VOS_UINT32								ulChannelId;
	IMSA_VOICE_SET_PARAM_ENGINE_ENUM_UINT32	ulType;
	IMSA_VOICE_PARAM_ENUM_UINT32			ulParam;
} IMSA_VOICE_SET_PARAM_ENGINE_IND_STRU;

typedef struct
{
	VOS_MSG_HEADER
	VOS_UINT16								usMsgName;
	VOS_UINT16								usReserve;
	VOS_UINT32								ulChannelId;
	VOICE_IMSA_ENGINE_STATUS_ENUM_UINT32	ulStatus;
	VOICE_IMSA_PARAM_ENUM_UINT32			ulParam;
} VOICE_IMSA_ENGINE_STATUS_NTF_STRU;

typedef struct
{
	VOS_MSG_HEADER
	VOS_UINT16							usMsgName;
	VOS_UINT16							usReserve1;
	VOS_UINT32							ulChannelId;
	VOS_UINT32							ulLen;
	VOS_CHAR							ucEvntType;
	VOS_CHAR							usReserve2[3];
} IMSA_VOICE_SEND_DTMF_START_ENGINE_IND_STRU;

typedef struct
{
	VOS_MSG_HEADER
	VOS_UINT16							usMsgName;
	VOS_UINT16							usReserve;
	VOS_UINT32							ulChannelId;
} IMSA_VOICE_START_ENGINE_IND_STRU;

typedef struct
{
	VOS_MSG_HEADER
	VOS_UINT16							usMsgName;
	VOS_UINT16							usReserve;
	VOS_UINT32							ulChannelId;
} IMSA_VOICE_STOP_ENGINE_IND_STRU;

typedef struct
{
	VOS_MSG_HEADER
	VOS_UINT16							usMsgName;
	VOS_UINT16							usReserve;
	VOS_UINT16							usFailMsgId;
	VOS_UINT16							usReserve2;
	VOS_INT32							lExecRslt;
} VOICE_IMSA_FAIL_IND_STRU;


/* IMSA send RTP packet to RTP module */
typedef struct
{
	VOS_MSG_HEADER
	VOS_UINT16							usMsgId;
	VOS_UINT16							usReserve;
	VOS_UINT32							ulChannelId;  /* Channel ID */
	IMSA_VOICE_PORT_ENUM_UINT32 		ulPortType;   /* Port Type: 0 RTP; 1 RTCP */
	VOS_UINT16							usRtpRtcpLen; /* RTP/RTCP data length��sizeof(uint_16) */
	VOS_UINT16							usDataSeq;	  /* RTP/PCTP packet sequence��sizeof(uint_16) */
	VOS_UINT8							ucFragSeq;	  /* RTP/PCTP frag sequence��the whole data is discarded if the data is not continuous. */
	VOS_UINT8							ucFragSeqMax; /* RTP/PCTP frag data length ,from 1 to 6, abnormal then discard */
	VOS_UINT16							usReserve2;
	VOS_UINT8							aucData[IMSA_RTP_MAX_DATA_LEN];
	VOS_UINT32							ulNeedAckFlag;/* Whether HIFI need acknowledge */
	VOS_UINT32							ulOpid;		  /* operate id */
} IMSA_VOICE_RTP_RX_DATA_IND_STRU;

/* RTP module send RTP data to IMSA */
typedef struct
{
	VOS_MSG_HEADER
	VOS_UINT16							usMsgId;
	VOS_UINT16							usReserve;
	VOS_UINT32							ulChannelId;
	IMSA_VOICE_PORT_ENUM_UINT32 		ulPortType;
	VOS_UINT16							usRtpRtcpLen;
	VOS_UINT16							usDataSeq;
	VOS_UINT8							ucFragSeq;
	VOS_UINT8							ucFragSeqMax;
	VOS_UINT16							usReserve2;
	VOS_UINT8							aucData[IMSA_RTP_MAX_DATA_LEN];
} VOICE_IMSA_RTP_TX_DATA_IND_STRU;

/*****************************************************************************
  6 STRUCT����
*****************************************************************************/


/*****************************************************************************
  7 UNION����
*****************************************************************************/


/*****************************************************************************
  8 OTHERS����
*****************************************************************************/
#if 0                                                                           /* _H2ASN_Skip */

/*****************************************************************************
  H2ASN������Ϣ�ṹ����
*****************************************************************************/
/* ��Ҫ������Ӧ��ASN����,ȥ��L��ʽ,������V��ʽ */
typedef struct
{
	IMSA_VOICE_MSG_ID_ENUM_UINT16		enMsgID; 							 /* _H2ASN_MsgChoice_Export IMSA_VOICE_MSG_ID_ENUM_UINT16 */

	VOS_UINT8							aucMsgBlock[2];
	/***************************************************************************
		_H2ASN_MsgChoice_When_Comment		IMSA_VOICE_MSG_ID_ENUM_UINT16
	****************************************************************************/
}VOICE_IMSA_TRACE_MSG_DATA;

/* Э����Ϣ�ṹ*/
typedef struct
{
	VOS_UINT32 uwSenderCpuId;
	VOS_UINT32 uwSenderPid;
	VOS_UINT32 uwReceiverCpuId;
	VOS_UINT32 uwReceiverPid;
	VOS_UINT32 uwLength;
	VOICE_IMSA_TRACE_MSG_DATA			stMsgData;
}VoiceImsaTrace_MSG;

#endif    																		/* _H2ASN_Skip */
/*****************************************************************************
  9 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/

#ifdef __cplusplus
#if __cplusplus
	}
#endif
#endif

#endif /* end of imsacodecinterface.h */

