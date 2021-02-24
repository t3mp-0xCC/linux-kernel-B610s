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

/*****************************************************************************/
/*                                                                           */
/*                Copyright 1999 - 2003, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileName: APMInterface.h                                                  */
/*                                                                           */
/*    Author: Jiang KaiBo                                                    */
/*            Xu Cheng                                                       */
/*                                                                           */
/* Version: 1.0                                                              */
/*                                                                           */
/* Date: 2008-02                                                             */
/*                                                                           */
/* Description: process data of APM mailbox                                  */
/*                                                                           */
/* Others:                                                                   */
/*                                                                           */
/* History:                                                                  */
/* 1. Date:                                                                  */
/*    Author:                                                                */
/*    Modification: Create this file                                         */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/


#ifndef  _APM_INTERFACE_H
#define  _APM_INTERFACE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#include "vos.h"
#include "phyoaminterface.h"

/*---------------------------------------------*
 * �����궨��                                  *
 *---------------------------------------------*/

/*����DSP NV��ش���*/
#define SHPA_TMR_LOAD_PHY_INTERVAL          5000

#define DSP_DATA_WRITE_OK                   0x5555AAAA
#define DSP_DATA_READ_OK                    0x5A5A5A5A

/*����DSP���*/
#define CPHY_DSP_LOAD_SUCCESS               0
#define CPHY_DSP_LOAD_FAILURE               1
#define CPHY_DSP_LOAD_TIMEOUT               2

/* �ȴ����ر�־λ��Ч */
#define SHPA_PROTECT_DELAY_COUNT            (30)

/* ��ֹдDSPԽ��ı����� */
#define COMM_EXT_DATA_MEM_PROTECT_BIT_1     0xAAAA
#define COMM_EXT_DATA_MEM_PROTECT_BIT_2     0x5555
#define COMM_EXT_DATA_MEM_PROTECT_BIT_3     0x5A5A
#define COMM_EXT_DATA_MEM_PROTECT_BIT_4     0x5A5A
#define COMM_EXT_DATA_MEM_PROTECT_BIT_5     0xA5A5
#define COMM_EXT_DATA_MEM_PROTECT_BIT_6     0xA5A5
#define COMM_EXT_DATA_MEM_PROTECT_BIT_7     0x5555
#define COMM_EXT_DATA_MEM_PROTECT_BIT_8     0xAAAA

#define SHPA_LDF_DELAY_TIME                 (340)   /* ������������ʱ��Ҫ�ӳٵ�ʱ�� 10ms,����NMI �����Ϸ���DTCM��ARM���� */

#define SHPA_LDF_DELAY_1S_TIME              (34)   /* ������������ʱ��Ҫ�ӳٵ�ʱ�� 1ms,����NMI �����Ϸ���DTCM��ARM���� */

#define SHPA_LDF_LOAD_PHY_DELAY_TIME        (400)   /* ����DSPʱ����DSP��Ҫ��ʱ��ʱ�� 400ms */

#ifdef  __LDF_FUNCTION__
#define SHPA_Ldf_Check()                    UPHY_MNTN_PhyLdfFileSaving()
#endif

enum DSP_CONFIG_ERROR_EUNM
{
    DSP_NVREAD_NOERROR = 0,
    DSP_BOOTINFO_MEMERR,
    DSP_NVREAD_LENERR,
    DSP_NVREAD_READNVERR,
    DSP_NVREAD_ALLOCMEMERR,
    DSP_NVREAD_TOTALLENERR,
    DSP_LOAD_STATIC_ERROR,
    DSP_GET_COMMON_INFO_ERROR,
    DSP_CONFIG_INFO_TOO_LONG,
    DSP_ACTIVE_TIME_OUT,
    DSP_CONFIG_ERROR_BUTT
};
typedef VOS_UINT32 DSP_CONFIG_ERROR_EUNM_UINT32;

enum APM_CFG_STATE_EUNM
{
    APM_INIT_STATE   = 0,
    APM_SENDED_MSG,
    APM_RECEIVED_MSG,
    APM_CFG_STATE_BUTT
};
typedef VOS_UINT32 APM_CFG_STATE_EUNM_UINT32;

/*****************************************************************************
 ö����    :DUALMODEM_SCENE_TYPE_ENUM_UINT16
 Э����  :
 ö��˵��  :ͨ�����CCPU DDR��Ƶͨ������
*****************************************************************************/
enum
{
   DUALMODEM_SCENCE_GSM,
   DUALMODEM_SCENCE_WCDMA,
   DUALMODEM_SCENCE_APS,
   DUALMODEM_SCENCE_TL,
   DUALMODEM_SCENCE_GUDRX,
   DUALMODEM_SCENCE_BUTT,
};
typedef VOS_UINT16 DUALMODEM_SCENE_TYPE_ENUM_UINT16;

typedef struct
{
    VOS_UINT32                      ulDspNVConfigAddr;          /*��DSP��ȡ�ĵ�ǰDSP NV�����öε�ַ*/
    VOS_UINT32                      ulDspNvTotalLen;            /*��Ҫ���ø�DSP��NV��ĳ���*/
    VOS_UINT16                      *pusNVData;                 /*����NV���ݵ��ڴ�ռ�*/
    DSP_CONFIG_ERROR_EUNM_UINT32    enErrorCode;                /*��ǰ����DSP����ԭ��ֵ*/
    APM_CFG_STATE_EUNM_UINT32       enActiveState;
    APM_CFG_STATE_EUNM_UINT32       enLoadState;
    VOS_UINT32                      ulActiveResult;
    VOS_UINT32                      ulLoadResult;
    VOS_UINT32                      ulActiveSmPResult;          /*�ȴ�Active�ź����Ľ��*/
    VOS_UINT32                      ulLoadSmPResult;            /*�ȴ�Load�ź����Ľ��*/
    VOS_UINT32                      ulGetNvLenErrId;            /*��ȡNV���ȴ����NV ID��*/
    VOS_UINT32                      ulGetNvErrId;               /*��ȡNV���ݴ����NV ID��*/
    /*��ز�����ʱ�����¼*/
    VOS_UINT32                      ulGetNvLenSlice;            /*��ȡNV���ݵ�ʱ���*/
    VOS_UINT32                      ulRecNVReqSlice;            /*�յ�DSP NV����ʱ���*/
    VOS_UINT32                      ulConfigNVSlice;            /*����dSP NV��ʱ��*/
    VOS_UINT32                      ulDSPActiveSlice;           /*Active DSP��ʱ���*/
    VOS_UINT32                      ulDSPActCnfSlice;           /*�յ�Active�ظ���ʱ���*/
    VOS_UINT32                      ulDSPLoadSlice;             /*Load DSP��ʱ���*/
    VOS_UINT32                      ulDSPLoadCnfSlice;          /*�յ�Load�ظ���ʱ���*/
}DSP_CONFIG_CTRL_STRU;

/*����OM��͸����Ϣ*/
typedef struct
{
    VOS_MSG_HEADER                       /* ����Ϣͷ���TOOL_ID��PS��PID,��֤��Ϣ��ת������Ӧ��PSģ�� */
    VOS_UINT16      usTransPrimId;       /* �̶���д0x5001*/
    VOS_UINT16      usReserve;
    VOS_UINT8       ucFuncType;          /*���ݲ�ѯ04,�տ���Ϣ�ϱ�:02*/
    VOS_UINT8       ucReserve;
    VOS_UINT16      usAppLength;         /*��һ���ֽڵ���β�ĳ���*/
    VOS_UINT32      ulSn;                /* �ϱ���Ϣ�����к� */
    VOS_UINT32      ulTimeStamp;         /*��Ϣ��ʱ������ǰ��SOC Tickֵ*/
    VOS_UINT16      usPrimId;            /* ԭ��ID*/
    VOS_UINT16      usToolsId;           /* ����ID */
    VOS_UINT8       aucData[4];
}PS_APM_OM_TRANS_IND_STRU;

typedef struct
{
    VOS_UINT16 *pusArray;
    VOS_UINT32 ulNum;
}APM_RF_NV_INFO_STRU;

typedef struct
{
    VOS_UINT32 ulDSPPowerDownState;
    VOS_UINT32 ulReserved[7];
}LDF_EXT_INFO_STRU;

/*---------------------------------------------*
 * ��������                                    *
 *---------------------------------------------*/

extern VOS_SEM g_aulShpaLoadPhySemaphor[MODEM_ID_BUTT];

extern VOS_SEM g_aulShpaActivePhySemaphor[MODEM_ID_BUTT];

extern VOS_UINT32 g_aulShpaCurSysId[MODEM_ID_BUTT];

extern DSP_CONFIG_CTRL_STRU g_astDspConfigCtrl[MODEM_ID_BUTT];

extern VOS_UINT16 SHPA_LoadPhy( PHY_OAM_SET_WORK_MODE_REQ_STRU  stSetWorkMode );

extern VOS_UINT32 APM_PIDInit( enum VOS_INIT_PHASE_DEFINE ip );

extern VOS_VOID SHPA_ActivateHw(VOS_VOID);

extern VOS_VOID SHPA_DeactivateHw(VOS_VOID);

extern VOS_UINT32 SHPA_ActiveDSP(VOS_RATMODE_ENUM_UINT32 enRatMode, MODEM_ID_ENUM_UINT16 enModemId);

extern VOS_UINT32 SHPA_ReadWTxPower( VOS_VOID);

extern VOS_UINT32 GHPA_GetRealFN(MODEM_ID_ENUM_UINT16 enModemID);


extern VOS_UINT32 DualModem_DDRQosUpdate(MODEM_ID_ENUM_UINT16 enModem, DUALMODEM_SCENE_TYPE_ENUM_UINT16 enSceneType,  VOS_UINT32 ulValue);

#ifdef  __LDF_FUNCTION__
VOS_UINT32 SHPA_Ldf_Hifi_Saving(VOS_VOID);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* _APM_INTERFACE_H */

