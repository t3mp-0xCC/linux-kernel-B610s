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

#ifndef __MBB_ATCMDCALLPROC_H__
#define __MBB_ATCMDCALLPROC_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "AtCtx.h"
#include "AtParse.h"
#include "AtMnInterface.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)  /*4�ֽڶ���*/

/*****************************************************************************
  2 �궨��
*****************************************************************************/


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
#if(FEATURE_ON == MBB_WPG_PCM)
#define AT_CMD_CMIC_VAL_MAX             (12)
enum AT_CMD_CMIC_VALUE_ENUM
{
    AT_CMD_CMIC_LEV_1                 = 64,    /*��1������ֵ*/
    AT_CMD_CMIC_LEV_2                 = 67,    /*��2������ֵ*/
    AT_CMD_CMIC_LEV_3                 = 70,    /*��3������ֵ*/
    AT_CMD_CMIC_LEV_4                 = 73,    /*��4������ֵ*/
    AT_CMD_CMIC_LEV_5                 = 76,    /*��5������ֵ*/
    AT_CMD_CMIC_LEV_6                 = 79,    /*��6������ֵ*/
    AT_CMD_CMIC_LEV_7                 = 82,    /*��7������ֵ*/
    AT_CMD_CMIC_LEV_8                 = 84,    /*��8������ֵ*/
    AT_CMD_CMIC_LEV_9                 = 86,    /*��9������ֵ*/
    AT_CMD_CMIC_LEV_10                = 88,    /*��10������ֵ*/
    AT_CMD_CMIC_LEV_11                = 90,    /*��11������ֵ*/
    AT_CMD_CMIC_LEV_12                = 92,    /*��12������ֵ*/
};
#endif /* FEATURE_ON == MBB_WPG_PCM */


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


/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
#if(FEATURE_ON == MBB_WPG_PCM)
VOS_UINT32 AT_QryModemloopStatePara(VOS_UINT8 ucIndex);

VOS_UINT32 At_QryEccListPara(VOS_UINT8 ucIndex);

VOS_UINT32 At_SetPcmfrPara(VOS_UINT8 ucIndex);

VOS_UINT32 At_QryPcmFrPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_TestPcmFrPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_IsModemLoopOn(VOS_VOID);

VOS_UINT32 AT_IsCallOn(VOS_UINT8 ucIndex);

/*****************************************************************************
 �� �� ��  : AT_SetCvhuPara
 ��������  : +CVHU ����CVHU����
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 AT_SetCvhuPara(VOS_UINT8 ucIndex);

/*****************************************************************************
 �� �� ��  : AT_QryCvhuPara
 ��������  : +CVHU  ��ѯCVHUֵ
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 AT_QryCvhuPara(VOS_UINT8 ucIndex);

/*****************************************************************************
 �� �� ��  : AT_TestCvhuPara
 ��������  : +CVHU ��ѯCVHU������Χ
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 AT_TestCvhuPara(VOS_UINT8 ucIndex);

VOS_VOID AT_APP_VC_CallStatus(TAF_UINT8 ucIndex);
VOS_UINT32 AT_MBB_SetCmutPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_MBB_QryCmutPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 �� �� ��  : AT_SetSmutPara
 ��������  : ^SMUT=<n> ��ѯ����״̬
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
extern VOS_UINT32 AT_SetSmutPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 �� �� ��  : AT_QrySmutPara
 ��������  : ^SMUT? ��ѯ����״̬
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
extern VOS_UINT32 AT_QrySmutPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 �� �� ��  : AT_SetCpcmPara
 ��������  : ^CPCM=<n> ��ѯ����״̬
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
extern VOS_UINT32 AT_SetCpcmPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 �� �� ��  : AT_QryCpcmPara
 ��������  : ^CPCM? ��ѯ����״̬
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
extern VOS_UINT32 AT_QryCpcmPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 �� �� ��  : AT_SetStnPara
 ��������  : ^STN=<n> ���ò�������״̬
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
extern VOS_UINT32 AT_SetStnPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 �� �� ��  : AT_QryStnPara
 ��������  : ^STN? ��ѯ��������״̬
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
extern VOS_UINT32 AT_QryStnPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 �� �� ��  : AT_SetNsswitchPara
 ��������  : ^NSSWITCH=<n> �����������ƿ���״̬
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
extern VOS_UINT32 AT_SetNsswitchPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 �� �� ��  : AT_QryNsswitchPara
 ��������  : ^NSSWITCH? ��ѯ�������ƿ���״̬
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
extern VOS_UINT32 AT_QryNsswitchPara(VOS_UINT8 ucIndex);

/*****************************************************************************
 �� �� ��  : At_SetCmicPara
 ��������  : ^CMIC=<n> ������������
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
extern VOS_UINT32 At_SetCmicPara(VOS_UINT8 ucIndex);

/*****************************************************************************
 �� �� ��  : At_QryCmicPara
 ��������  : ^CMIC? ��ѯ��������
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
extern VOS_UINT32 At_QryCmicPara(VOS_UINT8 ucIndex);
/******************************************************************************
 �� �� ��  : At_SetEchoexPara
 ��������  : ^ECHOEX=<n>[,<value>[,<tmode>[,<tmodepara1>,<tmodepara2>]]] 
���û������ƿ��ؼ�����
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
extern VOS_UINT32 At_SetEchoexPara(VOS_UINT8 ucIndex);
/*****************************************************************************
 �� �� ��  : At_QryEchoexPara
 ��������  : ^ECHOEX? ��ѯ�������ƿ��ؼ�����
 �������  : ucIndex --- �û�����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
extern VOS_UINT32 At_QryEchoexPara(VOS_UINT8 ucIndex);
VOS_VOID At_RcvMnCallEccListIndProc(
    MN_AT_IND_EVT_STRU                 *pstData);
VOS_VOID At_RcvEccListQryCnf(
    MN_AT_IND_EVT_STRU                 *pstData);
extern VOS_UINT32 At_ProcVcSetSmutStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
extern VOS_UINT32 At_ProcVcGetSmutStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo);
extern VOS_UINT32 At_ProcVcSetStnStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo);
extern VOS_UINT32 At_ProcVcGetStnStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo);
extern VOS_UINT32 At_ProcVcSetNsswitchStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo);
extern VOS_UINT32 At_ProcVcGetNsswitchStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
extern VOS_UINT32 At_ProcVcGetUpVolumeEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvt
);
extern VOS_UINT32 At_ProcVcSetEchoStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
extern VOS_UINT32 At_ProcVcGetEchoexEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
extern VOS_UINT32 At_ProcVcSetPcmFrStatusEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
extern VOS_UINT32 At_ProcVcGetPcmFrEvent(
    VOS_UINT8                           ucIndex,
    APP_VC_EVENT_INFO_STRU             *pstVcEvtInfo
);
#endif /* FEATURE_ON == MBB_WPG_PCM */

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

#endif /* end of AtCmdCallProc.h */
