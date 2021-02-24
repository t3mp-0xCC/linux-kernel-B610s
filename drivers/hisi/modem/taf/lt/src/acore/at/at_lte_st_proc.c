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

/******************************************************************************
 */
/* PROJECT   :
 */
/* SUBSYSTEM :
 */
/* MODULE    :
 */
/* OWNER     :
 */
/******************************************************************************
 */


/******************************************************************************
 */
/*����AT����CheckList(chenpeng/00173035 2010-12-17):
 */
/*
 */
/*
 */
/* �������checklist:
 */
/*   1��AT���������Ƿ���ȷ
 */
/*      typedef VOS_UINT8 AT_CMD_OPT_TYPE;
 */
/*      #define AT_CMD_OPT_SET_CMD_NO_PARA     0
 */
/*      #define AT_CMD_OPT_SET_PARA_CMD        1
 */
/*      #define AT_CMD_OPT_READ_CMD            2
 */
/*      #define AT_CMD_OPT_TEST_CMD            3
 */
/*      #define AT_CMD_OPT_BUTT                4
 */
/*
 */
/*   2�����������Ƿ����Ҫ��
 */
/*      gucAtParaIndex
 */
/*
 */
/*   3��ÿ�������ĳ����Ƿ���ȷ���Ƿ�Ϊ0
 */
/*      gastAtParaList[0].usParaLen
 */
/*      gastAtParaList[1].usParaLen
 */
/*
 */
/*   4��ÿ������ȡֵ��Լ��(ȡֵ��Χ��������������������)�Ƿ�����
 */
/*      ע:����ȡֵԼ��Ӧ�÷��ھ���������ģ�鱣֤���˴���͸������
 */
/*      gastAtParaList[0].ulParaValue
 */
/*      gastAtParaList[1].ulParaValue
 */
/******************************************************************************
 */
/*lint -save -e537 -e516 -e830 -e713 -e734 -e813 -e958 -e718 -e746*/
#include "osm.h"
#include "gen_msg.h"

#include "at_lte_common.h"

#include "gen_msg.h"
#include "ATCmdProc.h"

/*lint -e826*/

#define    THIS_FILE_ID        MSP_FILE_ID_AT_LTE_ST_PROC_C


#define MAX_DATA_TYPE_LEN   50
/* BAND������Ŀ:A/E/F
 */
#define BAND_TYPE_NUM       3

const VOS_UCHAR g_AtTdsCnfStr[][MAX_DATA_TYPE_LEN] =
{
    "APCOFFSETFLAG",
    "APCOFFSET(BAND A)","APCOFFSET(BAND E)","APCOFFSET(BAND F)",
    "APC(BAND A)","APC(BAND E)","APC(BAND F)",
    "APCFREQ(BAND A)","APCFREQ(BAND E)","APCFREQ(BAND F)",
    "AGC(BAND A)","AGC(BAND E)","AGC(BAND F)",
    "AGCFREQ(BAND A)","AGCFREQ(BAND E)","AGCFREQ(BAND F)"
};
VOS_UINT32 atSetSSYNCPara(VOS_UINT8 ucClientId)
{
    FTM_SET_SSYNC_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(4 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usMode = (VOS_UINT16)(gastAtParaList[0].ulParaValue);


    if(0 == gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usBand = (VOS_UINT16)(gastAtParaList[1].ulParaValue);

    if(0 == gastAtParaList[2].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usulChannel = (VOS_UINT16)(gastAtParaList[2].ulParaValue);

    if(0 == gastAtParaList[3].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usdlChannel = (VOS_UINT16)(gastAtParaList[3].ulParaValue);


    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_SSYNC_REQ,ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SSYNC_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSSYNCParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{

	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_SSYNC_CNF_STRU *pstCnf = NULL;


    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_SSYNC_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySSYNCPara(VOS_UINT8 ucClientId)
{
    FTM_RD_SSYNC_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_SSYNC_REQ,ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SSYNC_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySSYNCParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_SSYNC_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_SSYNC_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^SSYNC:%u",pstCnf->ulStatus);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/*********************************************************************************
 */
/*
 */
/*********************************************************************************
 */
VOS_UINT32 atSetSTXBWPara(VOS_UINT8 ucClientId)
{
    FTM_SET_STXBW_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stSetReq.ulBandwide = (gastAtParaList[0].ulParaValue);


    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_STXBW_REQ, ucClientId,(VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXBW_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSTXBWParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_STXBW_CNF_STRU *pstCnf = NULL;


    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_STXBW_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySTXBWPara(VOS_UINT8 ucClientId)
{
    FTM_RD_STXBW_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

	
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_STXBW_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXBW_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySTXBWParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    FTM_RD_STXBW_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_STXBW_CNF_STRU *)pEvent->ulParam1;

    if(NULL == pstCnf)
    {
        return ERR_MSP_FAILURE;
    }

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
    	"^STXBW:%u",pstCnf->ulBandwide);

    CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/*********************************************************************************
 */
/*
 */
/*********************************************************************************
 */
VOS_UINT32 atSetSTXCHANPara(VOS_UINT8 ucClientId)
{
    FTM_SET_STXCHAN_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stSetReq.ulStxChannel= (gastAtParaList[0].ulParaValue);


    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_STXCHAN_REQ,ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXCHAN_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSTXCHANParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_STXCHAN_CNF_STRU *pstCnf = NULL;


    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_STXCHAN_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySTXCHANPara(VOS_UINT8 ucClientId)
{
    FTM_RD_STXCHAN_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

	
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_STXCHAN_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXCHAN_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySTXCHANParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_STXCHAN_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_STXCHAN_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^STXCHAN:%u",pstCnf->ulStxChannel);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/*********************************************************************************
 */
/*
 */
/*********************************************************************************
 */
VOS_UINT32 atSetSSUBFRAMEPara(VOS_UINT8 ucClientId)
{
    FTM_SET_SSUBFRAME_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usSubFrameAssign= (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if(0 == gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usSubFramePattern= (VOS_UINT16)(gastAtParaList[1].ulParaValue);

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_SSUBFRAME_REQ,ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SSUBFRAME_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSSUBFRAMEParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_SSUBFRAME_CNF_STRU *pstCnf = NULL;


    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_SSUBFRAME_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySSUBFRAMEPara(VOS_UINT8 ucClientId)
{
    FTM_RD_SSUBFRAME_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_SSUBFRAME_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SSUBFRAME_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySSUBFRAMEParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_SSUBFRAME_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_SSUBFRAME_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^SSUBFRAME:%u,%u",pstCnf->usSubFrameAssign,pstCnf->usSubFramePattern);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/*********************************************************************************
 */
/*
 */
/*********************************************************************************
 */
VOS_UINT32 atSetSPARAPara(VOS_UINT8 ucClientId)
{
    FTM_SET_SPARA_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usType = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if(0 == gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usValue = (VOS_UINT16)(gastAtParaList[1].ulParaValue);

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_SPARA_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SPARA_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSPARAParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_SPARA_CNF_STRU *pstCnf = NULL;

    /*vos_printf("\n enter atSetFFCHANSParaCnfProc !!!\n");
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_SPARA_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySPARAPara(VOS_UINT8 ucClientId)
{
    FTM_RD_SPARA_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_SPARA_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SPARA_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySPARAParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_SPARA_CNF_TOTAL_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;
	VOS_UINT32 ulTypeNum=0;
	VOS_UINT32 i = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_SPARA_CNF_TOTAL_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}
	ulTypeNum = pstCnf->ulTypeNum;

	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^SPARA:%d\n\r",pstCnf->ulTypeNum);

    for(i=0;i<ulTypeNum;i++)
    {
		usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
			"\n\r%d,%d\n\r",pstCnf->stSpara[i].usType,pstCnf->stSpara[i].usValue);

    }

    CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/*****************************************************************************
 */
/*
 */
/*****************************************************************************
 */
VOS_UINT32 atSetSSEGNUMPara(VOS_UINT8 ucClientId)
{
    FTM_SET_SSEGNUM_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usSegNum = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_SSEGNUM_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SSEGNUM_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSSEGNUMParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_SSEGNUM_CNF_STRU *pstCnf = NULL;

    /*vos_printf("\n enter atSetFFCHANSParaCnfProc !!!\n");
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_SSEGNUM_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySSEGNUMPara(VOS_UINT8 ucClientId)
{
    FTM_RD_SSEGNUM_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_SSEGNUM_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SSEGNUM_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySSEGNUMParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_SSEGNUM_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_SSEGNUM_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^SSEGNUM:%u",pstCnf->usSegNum);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/*****************************************************************************
 */
/*
 */
/*****************************************************************************
 */
VOS_UINT32 atSetSTXMODUSPara(VOS_UINT8 ucClientId)
{
    FTM_SET_STXMODUS_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if( 0 == gastAtParaList[0].usParaLen ||
		0 == gastAtParaList[1].usParaLen )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usmodulution_num = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if((stSetReq.usmodulution_num >0) && (stSetReq.usmodulution_num <= 256))
    {
        ulRst = initParaListU16((AT_PARSE_PARA_TYPE_STRU *)(&(gastAtParaList[1])),stSetReq.usmodulution_num,stSetReq.usmodulation_list);
        if(ulRst != ERR_MSP_SUCCESS)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_STXMODUS_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXMODUS_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSTXMODUSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_STXMODUS_CNF_STRU *pstCnf = NULL;

    /*vos_printf("\n enter atSetFFCHANSParaCnfProc !!!\n");
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_STXMODUS_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySTXMODUSPara(VOS_UINT8 ucClientId)
{
    FTM_RD_STXMODUS_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_STXMODUS_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXMODUS_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySTXMODUSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_STXMODUS_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;
	VOS_UINT32 ulTemp = 0;
	VOS_UINT32 i = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_STXMODUS_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^STXMODUS:%u",pstCnf->usmodulution_num);
	ulTemp = pstCnf->usmodulution_num;
	if( ulTemp > 256 )
	{
	  return ERR_MSP_FAILURE;
	}
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		",%u",pstCnf->usmodulation_list[0]);

	for(i=1;i<ulTemp;i++)
	{

	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		" %u",pstCnf->usmodulation_list[i]);
	}

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/*****************************************************************************
 */
/*
 */
/*****************************************************************************
 */
VOS_UINT32 atSetSTXRBNUMSPara(VOS_UINT8 ucClientId)
{
    FTM_SET_STXRBNUMS_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if( 0 == gastAtParaList[0].usParaLen ||
		0 == gastAtParaList[1].usParaLen )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usrb_num = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if((stSetReq.usrb_num >0) && (stSetReq.usrb_num <= 256))
    {
        ulRst = initParaListU16((AT_PARSE_PARA_TYPE_STRU *)(&(gastAtParaList[1])),stSetReq.usrb_num,stSetReq.usrb_list);
        if(ulRst != ERR_MSP_SUCCESS)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_STXRBNUMS_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXRBNUMS_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSTXRBNUMSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_STXRBNUMS_CNF_STRU *pstCnf = NULL;

    /*vos_printf("\n enter atSetFFCHANSParaCnfProc !!!\n");
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_STXRBNUMS_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySTXRBNUMSPara(VOS_UINT8 ucClientId)
{
    FTM_RD_STXRBNUMS_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_STXRBNUMS_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXRBNUMS_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySTXRBNUMSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_STXRBNUMS_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;
	VOS_UINT32 ulTemp = 0;
	VOS_UINT32 i = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_STXRBNUMS_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^STXRBNUMS:%u",pstCnf->usrb_num);
	ulTemp = pstCnf->usrb_num;
	if( ulTemp > 256 )
	{
	  return ERR_MSP_FAILURE;
	}
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		",%u",pstCnf->usrb_list[0]);

	for(i=1;i<ulTemp;i++)
	{

	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		" %u",pstCnf->usrb_list[i]);
	}

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/*****************************************************************************
 */
/*
 */
/*****************************************************************************
 */
VOS_UINT32 atSetSTXRBPOSSPara(VOS_UINT8 ucClientId)
{
    FTM_SET_STXRBPOSS_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if( 0 == gastAtParaList[0].usParaLen||
		0 == gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usrbpos_num= (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if((stSetReq.usrbpos_num >0) && (stSetReq.usrbpos_num <= 256))
    {
        ulRst = initParaListU16((AT_PARSE_PARA_TYPE_STRU *)(&(gastAtParaList[1])),stSetReq.usrbpos_num,stSetReq.usrbpos_list);
        if(ulRst != ERR_MSP_SUCCESS)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_STXRBPOSS_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXRBPOSS_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSTXRBPOSSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_STXRBPOSS_CNF_STRU *pstCnf = NULL;

    /*vos_printf("\n enter atSetFFCHANSParaCnfProc !!!\n");
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_STXRBPOSS_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySTXRBPOSSPara(VOS_UINT8 ucClientId)
{
    FTM_RD_STXRBPOSS_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_STXRBPOSS_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXRBPOSS_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySTXRBPOSSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_STXRBPOSS_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;
	VOS_UINT32 ulTemp = 0;
	VOS_UINT32 i = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_STXRBPOSS_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^STXRBPOSS:%u",pstCnf->usrbpos_num);
	ulTemp = pstCnf->usrbpos_num;
	if( ulTemp > 256 )
	{
	  return ERR_MSP_FAILURE;
	}
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		",%u",pstCnf->usrbpos_list[0]);

	for(i=1;i<ulTemp;i++)
	{

	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		" %u",pstCnf->usrbpos_list[i]);
	}

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/*****************************************************************************
 */
/*
 */
/*****************************************************************************
 */
VOS_UINT32 atSetSTXPOWSPara(VOS_UINT8 ucClientId)
{
    FTM_SET_STXPOWS_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if( 0 == gastAtParaList[0].usParaLen||
		0 == gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.uspower_num = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if((stSetReq.uspower_num >0) && (stSetReq.uspower_num <= 256))
    {
        ulRst = initParaListS16((AT_PARSE_PARA_TYPE_STRU *)(&(gastAtParaList[1])),stSetReq.uspower_num,stSetReq.uspower_list);
        if(ulRst != ERR_MSP_SUCCESS)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_STXPOWS_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXPOWS_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSTXPOWSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_STXPOWS_CNF_STRU *pstCnf = NULL;

    /*vos_printf("\n enter atSetFFCHANSParaCnfProc !!!\n");
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_STXPOWS_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySTXPOWSPara(VOS_UINT8 ucClientId)
{
    FTM_RD_STXPOWS_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_STXPOWS_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXPOWS_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySTXPOWSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_STXPOWS_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;
	VOS_UINT32 ulTemp = 0;
	VOS_UINT32 i = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_STXPOWS_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^STXPOWS:%d",pstCnf->uspower_num);
	ulTemp = pstCnf->uspower_num;
	if( ulTemp > 256 )
	{
	  return ERR_MSP_FAILURE;
	}
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		",%d",pstCnf->uspower_list[0]);

	for(i=1;i<ulTemp;i++)
	{

	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		" %d",pstCnf->uspower_list[i]);
	}

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/*****************************************************************************
 */
/*
 */
/*****************************************************************************
 */
VOS_UINT32 atSetSTXCHANTYPESPara(VOS_UINT8 ucClientId)
{
    FTM_SET_STXCHANTYPES_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if( 0 == gastAtParaList[0].usParaLen||
		0 == gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.ustype_num = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if((stSetReq.ustype_num > 0) && (stSetReq.ustype_num <= 256))
    {
        ulRst = initParaListU16((AT_PARSE_PARA_TYPE_STRU *)(&(gastAtParaList[1])),stSetReq.ustype_num,stSetReq.ustype_list);
        if(ulRst != ERR_MSP_SUCCESS)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_STXCHANTYPES_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXCHANTYPES_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSTXCHANTYPESParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_STXCHANTYPES_CNF_STRU *pstCnf = NULL;

    /*vos_printf("\n enter atSetFFCHANSParaCnfProc !!!\n");
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_STXCHANTYPES_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySTXCHANTYPESPara(VOS_UINT8 ucClientId)
{
    FTM_RD_STXCHANTYPES_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_STXCHANTYPES_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STXCHANTYPES_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySTXCHANTYPESParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_STXCHANTYPES_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;
	VOS_UINT32 ulTemp = 0;
	VOS_UINT32 i = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_STXCHANTYPES_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^STXCHANTYPES:%u",pstCnf->ustype_num);
	ulTemp = pstCnf->ustype_num;
	if( ulTemp > 256 )
	{
	  return ERR_MSP_FAILURE;
	}
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		",%u",pstCnf->ustype_list[0]);

	for(i=1;i<ulTemp;i++)
	{

	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		" %u",pstCnf->ustype_list[i]);
	}

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/*********************************************************************************
 */
/*
 */
/*********************************************************************************
 */
VOS_UINT32 atSetSSEGLENPara(VOS_UINT8 ucClientId)
{
    FTM_SET_SSEGLEN_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.ussegment_len= (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_SSEGLEN_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SSEGLEN_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSSEGLENParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_SSEGLEN_CNF_STRU *pstCnf = NULL;

    /*vos_printf("\n enter atSetFFCHANSParaCnfProc !!!\n");
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_SSEGLEN_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySSEGLENPara(VOS_UINT8 ucClientId)
{
    FTM_RD_SSEGLEN_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_SSEGLEN_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SSEGLEN_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;


}

VOS_UINT32 atQrySSEGLENParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_SSEGLEN_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_SSEGLEN_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^SSEGLEN:%u",pstCnf->ussegment_len);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atSetSRXSETPara(VOS_UINT8 ucClientId)
{
    FTM_SET_SRXSET_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(5 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if( 0 == gastAtParaList[0].usParaLen ||
        0 == gastAtParaList[1].usParaLen ||
        0 == gastAtParaList[2].usParaLen ||
        0 == gastAtParaList[3].usParaLen ||
        0 == gastAtParaList[4].usParaLen )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stSetReq.usSwitch = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if((0 == stSetReq.usSwitch)||(1 == stSetReq.usSwitch))
    {
        ulRst = initParaListS16((AT_PARSE_PARA_TYPE_STRU *)(&(gastAtParaList[1])),1,&stSetReq.sPower);
        if(ulRst != ERR_MSP_SUCCESS)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    stSetReq.usMod  = (VOS_UINT16)(gastAtParaList[2].ulParaValue);
    stSetReq.usRBNum = (VOS_UINT16)(gastAtParaList[3].ulParaValue);
    stSetReq.usRBPos = (VOS_UINT16)(gastAtParaList[4].ulParaValue);

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_SRXSET_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SRXSET_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;
}

VOS_UINT32 atSetSRXSETParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_SRXSET_CNF_STRU *pstCnf = NULL;

    /*vos_printf("\n enter atSetFFCHANSParaCnfProc !!!\n");
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_SRXSET_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySRXSETPara(VOS_UINT8 ucClientId)
{
    FTM_RD_SRXSET_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_SRXSET_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SRXSET_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;


}

VOS_UINT32 atQrySRXSETParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_SRXSET_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_SRXSET_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^SRXSET:%u",pstCnf->usSwitch);
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		",%d,%u,%u,%u",pstCnf->sPower,pstCnf->usMod,pstCnf->usRBNum,pstCnf->usRBPos);


	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}
/*********************************************************************************
 */
/*
 */
/*********************************************************************************
 */
VOS_UINT32 atSetSRXPOWPara(VOS_UINT8 ucClientId)
{
    FTM_SET_SRXPOW_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return ERR_MSP_INVALID_PARAMETER;
    }

    if(2 != gucAtParaIndex)
    {
        return ERR_MSP_INVALID_PARAMETER;
    }

    if( 0 == gastAtParaList[0].usParaLen ||
        0 == gastAtParaList[1].usParaLen)
    {
        return ERR_MSP_INVALID_PARAMETER;
    }
    stSetReq.usSwitch = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if((stSetReq.usSwitch == 0) || (stSetReq.usSwitch == 1))
    {
        ulRst = initParaListS16((AT_PARSE_PARA_TYPE_STRU *)(&(gastAtParaList[1])),1,&stSetReq.usulPower);
        if(ulRst != ERR_MSP_SUCCESS)
        {
            return ERR_MSP_INVALID_PARAMETER;
        }
    }

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_SRXPOW_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SRXPOW_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSRXPOWParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_SRXPOW_CNF_STRU *pstCnf = NULL;

    /*vos_printf("\n enter atSetFFCHANSParaCnfProc !!!\n");
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_SRXPOW_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySRXPOWPara(VOS_UINT8 ucClientId)
{
    FTM_RD_SRXPOW_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_SRXPOW_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SRXPOW_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySRXPOWParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_SRXPOW_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_SRXPOW_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^SRXPOW:%d,%d",pstCnf->usSwitch,pstCnf->usulPower);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/*********************************************************************************
 */
/*
 */
/*********************************************************************************
 */
VOS_UINT32 atSetSRXSUBFRAPara(VOS_UINT8 ucClientId)
{
    FTM_SET_SRXSUBFRA_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if( 0 == gastAtParaList[0].usParaLen )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.ussubFrameNum= (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_SRXSUBFRA_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SRXSUBFRA_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSRXSUBFRAParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_SRXSUBFRA_CNF_STRU *pstCnf = NULL;

    /*vos_printf("\n enter atSetFFCHANSParaCnfProc !!!\n");
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_SRXSUBFRA_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySRXSUBFRAPara(VOS_UINT8 ucClientId)
{
    FTM_RD_SRXSUBFRA_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_SRXSUBFRA_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SRXSUBFRA_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySRXSUBFRAParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_SRXSUBFRA_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_SRXSUBFRA_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^SRXSUBFRA:%u",pstCnf->ussubFrameNum);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}
/**********************************************************************************
 */
/*
 */
/**********************************************************************************
 */
VOS_UINT32 atQrySRXBLERPara(VOS_UINT8 ucClientId)
{
    FTM_RD_SRXBLER_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_SRXBLER_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SRXBLER_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySRXBLERParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_SRXBLER_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_SRXBLER_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"\n\r^SRXMEAS:BLER:%u,%u\n\r",pstCnf->usbler[0], pstCnf->usbler[1]);
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"\n\r^SRXMEAS:SNR:%d,%d\n\r",pstCnf->usSnr[0], pstCnf->usSnr[1]);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;


}

/*********************************************************************************
 */
/*
 */
/*********************************************************************************
 */
VOS_UINT32 atSetSSTARTPara(VOS_UINT8 ucClientId)
{
    FTM_SET_SSTART_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if( 0 == gastAtParaList[0].usParaLen )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usType = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_SSTART_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SSTART_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSSTARTParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_SSTART_CNF_STRU *pstCnf = NULL;

    /*vos_printf("\n enter atSetFFCHANSParaCnfProc !!!\n");
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_SSTART_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQrySSTARTPara(VOS_UINT8 ucClientId)
{
    FTM_RD_SSTART_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_SSTART_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SSTART_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 atQrySSTARTParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_SSTART_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_SSTART_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^SSTART:%u,%u",pstCnf->usType,pstCnf->usStatus);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/*********************************************************************************
 */
/*
 */
/*********************************************************************************
 */
VOS_UINT32 atSetSSTOPPara(VOS_UINT8 ucClientId)
{
    FTM_SET_SSTOP_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if( 0 == gastAtParaList[0].usParaLen )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usType = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_SSTOP_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SSTOP_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetSSTOPParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_SSTOP_CNF_STRU *pstCnf = NULL;

    /*vos_printf("\n enter atSetFFCHANSParaCnfProc !!!\n");
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_SSTOP_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}
VOS_UINT32 atSetSTCFGDPCHPara(VOS_UINT8 ucClientId)
{
    FTM_SET_STCFGDPCH_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
	if(2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

 
    stSetReq.ulTransType = (VOS_UINT16)(gastAtParaList[0].ulParaValue);
    stSetReq.usTPCStepSize = (VOS_UINT16)(gastAtParaList[1].ulParaValue);
    HAL_SDMLOG("-----[%s]:at^STCFGDPCH receive TYPE =%d, TPC Step size=%d \n", __FUNCTION__,(VOS_INT)stSetReq.ulTransType, stSetReq.usTPCStepSize);

    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_STCFGDPCH_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������*/
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_STCFGDPCH_SET;
        return AT_WAIT_ASYNC_RETURN;    /*������������״̬ */
 
    }
    return AT_ERROR;

}
VOS_UINT32 atSetSTCFGDPCHParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_STCFGDPCH_CNF_STRU *pstCnf = NULL;

    vos_printf("\r\n atSetSTCFGDPCHParaCnfProc\r\n");

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_STCFGDPCH_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;
}

//TDS �²�ѯBER
VOS_UINT32 atQrySTRXBERPara(VOS_UINT8 ucClientId)
{
    FTM_RD_STRXBER_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;
	


    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_STRXBER_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SRXBLER_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}
VOS_UINT32 atQrySTRXBERParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_STRXBER_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_STRXBER_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"\n\r^STRXMEAS:BerSum:%u\n\r",pstCnf->ulbersum);
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"\n\r^STRXMEAS:BerErr:%d\n\r",pstCnf->ulberErr);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;


}



/*****************************************************************************
 �� �� ��  : TransHexIntoAsciiChar
 ��������  : ��16��������ת��ΪASCII���ַ�
 �������  : VOS_UINT8
 �������  : ��
 �� �� ֵ  : VOS_UCHAR
*****************************************************************************/
VOS_UCHAR TransHexIntoAsciiChar(VOS_UINT8 hexInteger)
{
	VOS_UINT8    v_returnVal;

	if(hexInteger <= 0x09)
	{
		v_returnVal = hexInteger + '0';
	}
	else if((hexInteger >= 0x0A) && (hexInteger <= 0x0F))
	{
		v_returnVal = hexInteger + '7'; /* decimal interger is 55.
 */
	}
	else
	{
		v_returnVal = 0xFF;
	}

	return v_returnVal;
}

/*****************************************************************************
 �� �� ��  : TranslateTpduIntoAsciiChar
 ��������  : ��TPDU����ת��ΪASCII���ַ�����
 �������  : tpduResult       --- ��ת������
			 asciiChar        --- Ŀ������
			 tpduLength       --- TPDU���г���
			 maxOutputDataLen --- ���ת������
 �������  : ��
 �� �� ֵ  : VOS_UINT16       --- ת���õ���ASCII���г���
*****************************************************************************/
VOS_VOID TranslateTpduIntoAsciiChar(VOS_UINT8 * tpduContent, VOS_UCHAR * asciiResult,
											  VOS_UINT16 tpduLength, VOS_UINT16 maxOutputDataLen)
{
	VOS_UINT16   indexOfTpduCon = 0;
	VOS_UINT16   indexOfResult  = 0;

	MSP_MEMSET(asciiResult, 0x00, maxOutputDataLen);

	for(indexOfTpduCon = 0, indexOfResult = 0;\
		(indexOfTpduCon < tpduLength) && (indexOfResult < (maxOutputDataLen - 1));\
		indexOfTpduCon++)
	{
		asciiResult[indexOfResult++] = TransHexIntoAsciiChar((VOS_UINT8)((tpduContent[indexOfTpduCon] & 0xF0) >> 4));
		asciiResult[indexOfResult++] = TransHexIntoAsciiChar((VOS_UINT8)(tpduContent[indexOfTpduCon] & 0x0F));
	}
}


VOS_VOID CreateCnfStr(VOS_UINT32 dataType, VOS_UINT32 bandType, VOS_UCHAR * strSrc, VOS_UINT32 strLength)
{
    VOS_UINT8    a_conversion[AT_CALIB_PARAM_APC];
    VOS_UCHAR a_Assicconversion[AT_CALIB_PARAM_APC * 2 + 1];
    /* ָʾȫ�ֱ���g_AtTdsCnfStr����������ʶ��ǰ��Ҫ���ص���������
 */
    VOS_UINT16   cnfStrIndex = 0;

    if(NULL == strSrc)
    {
        return;
    }

    MSP_MEMSET(a_conversion, 0x00, sizeof(a_conversion));
    MSP_MEMSET(a_Assicconversion, 0x00, sizeof(a_Assicconversion));

	MSP_MEMCPY(a_conversion, strSrc, strLength);
	TranslateTpduIntoAsciiChar(a_conversion, a_Assicconversion, strLength, strLength * 2 + 1);

    /* ����cnfStrIndex
 */
    if(AT_CALIB_DATA_TYPE_APCOFFSETFLAG == dataType)
    {
        cnfStrIndex = 0;
    }
    else
    {
        /* ��������ָʾ�ĵ�һ������(BAND A)
 */
        cnfStrIndex = (dataType - 1) * BAND_TYPE_NUM + 1;
        /* ͬһ���������²�ͬband���Ͷ�Ӧ������
 */
        if(AT_CALIB_BAND_TYPE_A == bandType)
        {
            cnfStrIndex += 0;
        }
        else if(AT_CALIB_BAND_TYPE_E == bandType)
        {
            cnfStrIndex += 1;
        }
        else if(AT_CALIB_BAND_TYPE_F == bandType)
        {
            cnfStrIndex += 2;
        }
        else
        {
            return;
        }
    }

    gstLAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + gstLAtSendData.usBufLen,
                                "^SCALIB:%s,%s\r\n", g_AtTdsCnfStr[cnfStrIndex], a_Assicconversion);
}


VOS_UINT32 atTdsCreateCnfStr(FTM_SET_SCALIB_CNF_STRU * pstCnf)
{
    AT_TDS_OPT_TYPE_ENUM_U32    opType;
    AT_TDS_DATA_TYPE_ENUM_U32   dataType;
    AT_TDS_BAND_TYPE_ENUM_U32   bandType;
    FTM_SCALIB_SYSTEM_SETTING_STRU      * systemSetting;
    FTM_SCALIB_APCOFFSET_SETTING_STRU   * apcoffsetFlag;

    if(NULL == pstCnf)
    {
        return ERR_MSP_FAILURE;
    }

    opType   = pstCnf->tds_OpType;
    dataType = pstCnf->tds_DataType;
    bandType = pstCnf->tds_BandType;
    systemSetting = (FTM_SCALIB_SYSTEM_SETTING_STRU *)&(pstCnf->systemSetting);
    apcoffsetFlag = (FTM_SCALIB_APCOFFSET_SETTING_STRU *)&(pstCnf->apcoffsetFlag);

    if(AT_CALIB_DATA_TYPE_BUTT != dataType)
    {
        switch(dataType)
        {
            case AT_CALIB_DATA_TYPE_APCOFFSETFLAG:

                /* APCOFFSETFLAGֻ�����ڻ����У�READ������ȡNV������
 */
                if(AT_CALIB_OP_TYPE_READ == opType)
                {
                    return ERR_MSP_FAILURE;
                }

                CreateCnfStr(dataType, bandType, (VOS_UCHAR *)apcoffsetFlag, sizeof(VOS_UINT16)*2);

                break;

            case AT_CALIB_DATA_TYPE_APCOFFSET:

                if(AT_CALIB_BAND_TYPE_A == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsTxPaPowerBand_A, AT_CALIB_PARAM_APCOFF);
                }
                else if(AT_CALIB_BAND_TYPE_E == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsTxPaPowerBand_E, AT_CALIB_PARAM_APCOFF);
                }
                else if(AT_CALIB_BAND_TYPE_F == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsTxPaPowerBand_F, AT_CALIB_PARAM_APCOFF);
                }
                else
                {
                    /* ָ����������ֻ����BAND A/E/F����
 */
                    return ERR_MSP_FAILURE;
                }

                break;

            case AT_CALIB_DATA_TYPE_APC:

                if(AT_CALIB_BAND_TYPE_A == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsTxApcCompBand_A, AT_CALIB_PARAM_APC);
                }
                else if(AT_CALIB_BAND_TYPE_E == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsTxApcCompBand_E, AT_CALIB_PARAM_APC);
                }
                else if(AT_CALIB_BAND_TYPE_F == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsTxApcCompBand_F, AT_CALIB_PARAM_APC);
                }
                else
                {
                    return ERR_MSP_FAILURE;
                }

                break;

            case AT_CALIB_DATA_TYPE_APCFREQ:

                if(AT_CALIB_BAND_TYPE_A == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsTxApcFreqCompBand_A, AT_CALIB_PARAM_APCFREQ);
                }
                else if(AT_CALIB_BAND_TYPE_E == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsTxApcFreqCompBand_E, AT_CALIB_PARAM_APCFREQ);
                }
                else if(AT_CALIB_BAND_TYPE_F == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsTxApcFreqCompBand_F, AT_CALIB_PARAM_APCFREQ);
                }
                else
                {
                   return ERR_MSP_FAILURE;
                }

                break;

            case AT_CALIB_DATA_TYPE_AGC:

                if(AT_CALIB_BAND_TYPE_A == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsRxAgcCompBand_A, AT_CALIB_PARAM_AGC);
                }
                else if(AT_CALIB_BAND_TYPE_E == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsRxAgcCompBand_E, AT_CALIB_PARAM_AGC);
                }
                else if(AT_CALIB_BAND_TYPE_F == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsRxAgcCompBand_F, AT_CALIB_PARAM_AGC);
                }
                else
                {
                    return ERR_MSP_FAILURE;
                }

                break;

            case AT_CALIB_DATA_TYPE_AGCFREQ:

                if(AT_CALIB_BAND_TYPE_A == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsRxAgcFreqCompBand_A, AT_CALIB_PARAM_AGCFREQ);
                }
                else if(AT_CALIB_BAND_TYPE_E == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsRxAgcFreqCompBand_E, AT_CALIB_PARAM_AGCFREQ);
                }
                else if(AT_CALIB_BAND_TYPE_F == bandType)
                {
                    CreateCnfStr(dataType, bandType, (VOS_UCHAR *)systemSetting->tdsRxAgcFreqCompBand_F, AT_CALIB_PARAM_AGCFREQ);
                }
                else
                {
                    return ERR_MSP_FAILURE;
                }

                break;

            default:

                return ERR_MSP_FAILURE;
        }
    }
    /* ����������������
 */
    else
    {
        /* APCOFFSETFLAGֻ�����ڻ����У�READ������ȡNV������
 */
        if(AT_CALIB_OP_TYPE_READ != opType)
        {
            /* APCOFFSETFLAG
 */
            CreateCnfStr(AT_CALIB_DATA_TYPE_APCOFFSETFLAG, AT_CALIB_BAND_TYPE_A, (VOS_UCHAR *)apcoffsetFlag, sizeof(VOS_UINT16)*2);
        }
        /* APCOFFSET BAND A/E/F
 */
        CreateCnfStr(AT_CALIB_DATA_TYPE_APCOFFSET, AT_CALIB_BAND_TYPE_A, (VOS_UCHAR *)systemSetting->tdsTxPaPowerBand_A, AT_CALIB_PARAM_APCOFF);
        CreateCnfStr(AT_CALIB_DATA_TYPE_APCOFFSET, AT_CALIB_BAND_TYPE_E, (VOS_UCHAR *)systemSetting->tdsTxPaPowerBand_E, AT_CALIB_PARAM_APCOFF);
        CreateCnfStr(AT_CALIB_DATA_TYPE_APCOFFSET, AT_CALIB_BAND_TYPE_F, (VOS_UCHAR *)systemSetting->tdsTxPaPowerBand_F, AT_CALIB_PARAM_APCOFF);
        /* APC BAND A/E/F
 */
        CreateCnfStr(AT_CALIB_DATA_TYPE_APC, AT_CALIB_BAND_TYPE_A, (VOS_UCHAR *)systemSetting->tdsTxApcCompBand_A, AT_CALIB_PARAM_APC);
        CreateCnfStr(AT_CALIB_DATA_TYPE_APC, AT_CALIB_BAND_TYPE_E, (VOS_UCHAR *)systemSetting->tdsTxApcCompBand_E, AT_CALIB_PARAM_APC);
        CreateCnfStr(AT_CALIB_DATA_TYPE_APC, AT_CALIB_BAND_TYPE_F, (VOS_UCHAR *)systemSetting->tdsTxApcCompBand_F, AT_CALIB_PARAM_APC);
        /* APCFREQ BAND A/E/F
 */
        CreateCnfStr(AT_CALIB_DATA_TYPE_APCFREQ, AT_CALIB_BAND_TYPE_A, (VOS_UCHAR *)systemSetting->tdsTxApcFreqCompBand_A, AT_CALIB_PARAM_APCFREQ);
        CreateCnfStr(AT_CALIB_DATA_TYPE_APCFREQ, AT_CALIB_BAND_TYPE_E, (VOS_UCHAR *)systemSetting->tdsTxApcFreqCompBand_E, AT_CALIB_PARAM_APCFREQ);
        CreateCnfStr(AT_CALIB_DATA_TYPE_APCFREQ, AT_CALIB_BAND_TYPE_F, (VOS_UCHAR *)systemSetting->tdsTxApcFreqCompBand_E, AT_CALIB_PARAM_APCFREQ);
        /* AGC BAND A/E/F
 */
        CreateCnfStr(AT_CALIB_DATA_TYPE_AGC, AT_CALIB_BAND_TYPE_A, (VOS_UCHAR *)systemSetting->tdsRxAgcCompBand_A, AT_CALIB_PARAM_AGC);
        CreateCnfStr(AT_CALIB_DATA_TYPE_AGC, AT_CALIB_BAND_TYPE_E, (VOS_UCHAR *)systemSetting->tdsRxAgcCompBand_E, AT_CALIB_PARAM_AGC);
        CreateCnfStr(AT_CALIB_DATA_TYPE_AGC, AT_CALIB_BAND_TYPE_F, (VOS_UCHAR *)systemSetting->tdsRxAgcCompBand_F, AT_CALIB_PARAM_AGC);
        /* AGCFREQ BAND A/E/F
 */
        CreateCnfStr(AT_CALIB_DATA_TYPE_AGCFREQ, AT_CALIB_BAND_TYPE_A, (VOS_UCHAR *)systemSetting->tdsRxAgcFreqCompBand_A, AT_CALIB_PARAM_AGCFREQ);
        CreateCnfStr(AT_CALIB_DATA_TYPE_AGCFREQ, AT_CALIB_BAND_TYPE_E, (VOS_UCHAR *)systemSetting->tdsRxAgcFreqCompBand_E, AT_CALIB_PARAM_AGCFREQ);
        CreateCnfStr(AT_CALIB_DATA_TYPE_AGCFREQ, AT_CALIB_BAND_TYPE_F, (VOS_UCHAR *)systemSetting->tdsRxAgcFreqCompBand_F, AT_CALIB_PARAM_AGCFREQ);
    }

    return ERR_MSP_SUCCESS;
}


VOS_UINT32 At_SetTdsScalibPara(VOS_UINT8 ucClientId)
{
    FTM_SET_SCALIB_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(gucAtParaIndex < 1 || gucAtParaIndex > 4)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������
 */
    if (0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        stSetReq.tds_OpType = (AT_TDS_OPT_TYPE_ENUM_U32)(gastAtParaList[0].ulParaValue);
    }

    /* ��������
 */
    if (0 == gastAtParaList[1].usParaLen)
    {
        stSetReq.tds_DataType = (AT_TDS_DATA_TYPE_ENUM_U32)AT_CALIB_DATA_TYPE_BUTT;
    }
    else
    {
        stSetReq.tds_DataType = (AT_TDS_BAND_TYPE_ENUM_U32)(gastAtParaList[1].ulParaValue);
    }

    /* band����
 */
    if (0 == gastAtParaList[2].usParaLen)
    {
        stSetReq.tds_BandType = (AT_TDS_BAND_TYPE_ENUM_U32)AT_CALIB_BAND_TYPE_BUTT;
    }
    else
    {
        stSetReq.tds_BandType = (AT_TDS_BAND_TYPE_ENUM_U32)(gastAtParaList[2].ulParaValue);
    }

    /* ����
 */
    if(gastAtParaList[3].usParaLen > MAX_SCALIB_DATA_LEN)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    else if (0 == gastAtParaList[3].usParaLen)
    {
        MSP_MEMSET(stSetReq.tds_Data, 0x00, sizeof(stSetReq.tds_Data));
        stSetReq.tds_DataLen =  0;
    }
    else
    {
        MSP_MEMCPY(stSetReq.tds_Data, gastAtParaList[3].aucPara, gastAtParaList[3].usParaLen);
        stSetReq.tds_DataLen =  gastAtParaList[3].usParaLen;
    }

    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_SCALIB_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SCALIB_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;


}


VOS_UINT32 At_QryTdsScalibPara(VOS_UINT8 ucClientId)
{
    FTM_RD_SCALIB_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_SCALIB_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SCALIB_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;


}


VOS_UINT32 atQrySCALIBParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID * pMsgBlock)
{
	FTM_RD_SCALIB_CNF_STRU * pstCnf = NULL;
	OS_MSG_STRU * pEvent = NULL;
	
    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_SCALIB_CNF_STRU *)pEvent->ulParam1;

	CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;
}


VOS_UINT32 atSetSCALIBParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID * pMsgBlock)
{
    VOS_UINT32 ret = ERR_MSP_FAILURE;
    FTM_SET_SCALIB_CNF_STRU           * pstCnf      = NULL;
    OS_MSG_STRU                       * pEvent      = NULL;
    AT_TDS_OPT_TYPE_ENUM_U32            opType;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_SCALIB_CNF_STRU *)pEvent->ulParam1;

    opType = pstCnf->tds_OpType;
    gstLAtSendData.usBufLen = 0;

    switch(opType)
    {
        case AT_CALIB_OP_TYPE_CACHE:
        case AT_CALIB_OP_TYPE_USE:
        case AT_CALIB_OP_TYPE_SAVE:
        case AT_CALIB_OP_TYPE_INI:
        case AT_CALIB_OP_TYPE_BEGIN:
        case AT_CALIB_OP_TYPE_END:
        case AT_CALIB_OP_TYPE_SET:
            /* ֻ���ش����룬û���������
 */
            CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

            break;

        case AT_CALIB_OP_TYPE_GET:
        case AT_CALIB_OP_TYPE_READ:

            /* GET����ʧ�ܣ�ֱ�ӷ���
 */
            if(ERR_MSP_SUCCESS != pstCnf->ulErrCode)
            {
                CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

                break;
            }
            /* �����������͡�band���͹��������ַ���
 */
            ret = atTdsCreateCnfStr(pstCnf);
            if(ERR_MSP_SUCCESS != ret)
            {
                CmdErrProc(ucClientId, ret, 0, NULL);

                break;
            }

            CmdErrProc(ucClientId, pstCnf->ulErrCode, gstLAtSendData.usBufLen, pgucLAtSndCodeAddr);

            break;

        default:

            CmdErrProc(ucClientId, ERR_MSP_FAILURE, 0, NULL);

            break;

    }

    return AT_FW_CLIENT_STATUS_READY;

}


VOS_UINT32 At_QryTdsScellinfoPara(VOS_UINT8 ucClientId)
{
    FTM_RD_SCELLINFO_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_SCELLINFO_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SCELLINFO_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;


}


VOS_UINT32 atQryScellinfoParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID * pMsgBlock)
{
	FTM_RD_SCELLINFO_CNF_STRU * pstCnf    = NULL;
    FTM_SCELLINFO_STRU        * scellInfo = NULL;
	OS_MSG_STRU * pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_SCELLINFO_CNF_STRU *)pEvent->ulParam1;
    scellInfo = (FTM_SCELLINFO_STRU *)&(pstCnf->scellinfoStru);

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
                                "^SCELLINFO:%u,%u,%u,%u,%u,%u,%u,%u", scellInfo->freq, scellInfo->bsic, scellInfo->rscp,
                                scellInfo->txRxSlot, scellInfo->txPower, scellInfo->protocolStatus, scellInfo->rrcStatus,
                                scellInfo->userNumInCurSlot);

    if(ERR_MSP_SUCCESS == pstCnf->ulErrCode)
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);

        return AT_FW_CLIENT_STATUS_READY;
    }

    return AT_ERROR;
}



VOS_UINT32 At_QryCmtm(VOS_UINT8 ucClientId)
{
    FTM_RD_TEMPERATURE_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_TEMPERATURE_REQ, ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_CMTM_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;


}


VOS_UINT32 atQryCmtmCnfProc(VOS_UINT8 ucClientId, VOS_VOID * pMsgBlock)
{
    FTM_RD_TEMPERATURE_CNF_STRU * pstCnf    = NULL;
    OS_MSG_STRU * pEvent = NULL;
    VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_TEMPERATURE_CNF_STRU *)pEvent->ulParam1;

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
                                "^CMTM:%u", pstCnf->ulTemperature);

    if(ERR_MSP_SUCCESS == pstCnf->ulErrCode)
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    }
    else
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);
    }

    return AT_FW_CLIENT_STATUS_READY;
}
/*lint -restore*/

