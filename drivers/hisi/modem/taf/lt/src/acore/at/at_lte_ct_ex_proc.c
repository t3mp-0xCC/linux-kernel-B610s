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
/*lint -save -e537 -e734 -e813 -e958 -e718 -e746*/
/*#include <stdlib.h>
 */
/*#include "at_common.h"
 */
#include "osm.h"
#include "gen_msg.h"

#include "at_lte_common.h"
#include "ATCmdProc.h"


/******************************************************************************
 */
/* ��������:  �����û����룬���������б���ʼ����Ӧ�Ľṹ
 */
/*
 */
/* ����˵��:
 */
/*   pPara     [in] ...
 */
/*   ulListLen [in] ...
 */
/*   pausList  [out] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/*
 */
/* ʹ��Լ��:
 */
/*    1��ֻ����ʮ���������ַ�����Ϊ����
 */
/*    2��0��Ϊ���ֿ�ͷ���Լ��ո��������������checklist�������²�������
 */
/*
 */
/* �ַ�������Checklist:
 */
/*    1������ַ����ܳ����Ƿ�Ϸ�
 */
/*    2������Ƿ��з��ڴ��ַ�(�ո����������)
 */
/*    3������ַ����������Ĳ���ȡֵ�Ƿ����Ҫ�󣬰��������ַ������Ȳ��ܳ���5��
 */
/*       �������� 000000123 �����ַǷ�����
 */
/*    4������ַ����������Ĳ��������Ƿ���û������һ��
 */
/******************************************************************************
 */
VOS_UINT32 initParaListS16( AT_PARSE_PARA_TYPE_STRU *pPara, VOS_UINT16 ulListLen, VOS_INT16* pausList)
{
    VOS_UINT16 ulTmp;
    VOS_UINT8 *pParaData    = pPara->aucPara;
    VOS_UINT8  ucDigitNum   = 0;     /* ��¼�����ַ����������ܳ���5
 */
    VOS_INT16 usDigitValue = 0;     /* ��¼������ֵ��С�����ܳ���65535
 */
    VOS_UINT16 usTmpListNum = 0;     /* ��¼�������������ָ��������ܳ���16
 */
    VOS_BOOL bDigit      = FALSE; /* �Ƿ�������
 */
	VOS_BOOL bNegative   = FALSE; /* �Ƿ���
 */
    VOS_UINT32 ulRst        = ERR_MSP_SUCCESS;

    if ((NULL == pPara) ||(pPara->usParaLen > 2048))
    {
        /* 1������ַ����ܳ����Ƿ�Ϸ�
 */
        return ERR_MSP_INVALID_PARAMETER;
    }

    for(ulTmp = 0 ; ulTmp < pPara->usParaLen ; ulTmp++)
    {
        VOS_UINT8 ucChr = *pParaData;

        if(isdigit(ucChr))
        {
            /* ��һ���ֽ�Ϊ���ֻ���ǰ���пո�
 */
            if(!bDigit)
            {
                bDigit = TRUE;
                ucDigitNum = 0;
                usDigitValue = 0;

                /* 4������ַ����������Ĳ��������Ƿ���û������һ��
 */
                if(++usTmpListNum > ulListLen)
                {
                    ulRst = ERR_MSP_INVALID_PARAMETER;
                    break;
                }
            }

            /* 3������ַ����������Ĳ���ȡֵ�Ƿ����Ҫ�󣬰��������ַ������Ȳ��ܳ���5��
 */
            if((++ucDigitNum > 5) ||((32767-usDigitValue*10) < (ucChr - 0x30)))
            {
                ulRst = ERR_MSP_INVALID_PARAMETER;
                break;
            }

            usDigitValue = (VOS_INT16)(usDigitValue*10+(ucChr-0x30));

			pausList[usTmpListNum-1] = (VOS_INT16)((bNegative == FALSE)?(usDigitValue):(usDigitValue*(-1)));
        }
        else if(isspace(ucChr))
        {
			/* ����'-'���ŵ��쳣����
 */
			if(!bDigit && bNegative)
			{
				break;
			}

            bDigit = FALSE;
			bNegative = FALSE;

			pParaData++;
			continue;
        }
        else if(('-' == ucChr) && !bDigit && !bNegative)
        {
			bNegative = TRUE;

			pParaData++;
			continue;
		}
        else
        {
            /* 2������Ƿ��з��ڴ��ַ�(�ո����������)
 */
            ulRst = ERR_MSP_INVALID_PARAMETER;
            break;
        }
        pParaData++;
    }

    /* 4��ǿ�Ƽ��:����ַ����������Ĳ��������Ƿ���û������һ��
 */
    if(usTmpListNum != ulListLen)
    {
        ulRst = ERR_MSP_INVALID_PARAMETER;
    }

    return ulRst;
}




/******************************************************************************
 */
/* ��������:  �����û����룬���������б���ʼ����Ӧ�Ľṹ
 */
/*
 */
/* ����˵��:
 */
/*   pPara     [in] ...
 */
/*   ulListLen [in] ...
 */
/*   pausList  [out] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/*
 */
/* ʹ��Լ��:
 */
/*    1��ֻ����ʮ���������ַ�����Ϊ����
 */
/*    2��0��Ϊ���ֿ�ͷ���Լ��ո��������������checklist�������²�������
 */
/*
 */
/* �ַ�������Checklist:
 */
/*    1������ַ����ܳ����Ƿ�Ϸ�
 */
/*    2������Ƿ��з��ڴ��ַ�(�ո����������)
 */
/*    3������ַ����������Ĳ���ȡֵ�Ƿ����Ҫ�󣬰��������ַ������Ȳ��ܳ���5��
 */
/*       �������� 000000123 �����ַǷ�����
 */
/*    4������ַ����������Ĳ��������Ƿ���û������һ��
 */
/******************************************************************************
 */
VOS_UINT32 initParaListU16( AT_PARSE_PARA_TYPE_STRU *pPara, VOS_UINT16 ulListLen, VOS_UINT16* pausList)
{
    VOS_UINT16 ulTmp;
    VOS_UINT8 *pParaData    = pPara->aucPara;
    VOS_UINT8  ucDigitNum   = 0;  /* ��¼�����ַ����������ܳ���5
 */
    VOS_UINT16 usDigitValue = 0;  /* ��¼������ֵ��С�����ܳ���65535
 */
    VOS_UINT16 usTmpListNum = 0;  /* ��¼�������������ָ��������ܳ���16
 */
    VOS_BOOL bDigit      = FALSE;
    VOS_UINT32 ulRst        = ERR_MSP_SUCCESS;

    if ((NULL == pPara) ||(pPara->usParaLen > 2048))
    {
        /* 1������ַ����ܳ����Ƿ�Ϸ�
 */
        return ERR_MSP_INVALID_PARAMETER;
    }

    for(ulTmp = 0 ; ulTmp < pPara->usParaLen ; ulTmp++)
    {
        VOS_UINT8 ucChr = *pParaData;

        if(isdigit(ucChr))
        {
            /* ��һ���ֽ�Ϊ���ֻ���ǰ���пո�
 */
            if(!bDigit)
            {
                bDigit = TRUE;
                ucDigitNum = 0;
                usDigitValue = 0;

                /* 4������ַ����������Ĳ��������Ƿ���û������һ��
 */
                if(++usTmpListNum > ulListLen)
                {
                    ulRst = ERR_MSP_INVALID_PARAMETER;
                    break;
                }
            }

            /* 3������ַ����������Ĳ���ȡֵ�Ƿ����Ҫ�󣬰��������ַ������Ȳ��ܳ���5��
 */
            if((++ucDigitNum > 5) ||((65535-usDigitValue*10) < (ucChr - 0x30)))
            {
                ulRst = ERR_MSP_INVALID_PARAMETER;
                break;
            }

            usDigitValue = (VOS_UINT16)(usDigitValue*10+(ucChr-0x30));
            pausList[usTmpListNum-1] = usDigitValue;
        }
        else if(isspace(ucChr))
        {
            bDigit = FALSE;

			pParaData++;
			continue;
        }
        else
        {
            /* 2������Ƿ��з��ڴ��ַ�(�ո����������)
 */
            ulRst = ERR_MSP_INVALID_PARAMETER;
            break;
        }
        pParaData++;
    }

    /* 4��ǿ�Ƽ��:����ַ����������Ĳ��������Ƿ���û������һ��
 */
    if(usTmpListNum != ulListLen)
    {
        ulRst = ERR_MSP_INVALID_PARAMETER;
    }

    return ulRst;
}


/******************************************************************************
 */
/* ��������:  �����û����룬���������б���ʼ����Ӧ�Ľṹ
 */
/*
 */
/* ����˵��:
 */
/*   pPara     [in] ...
 */
/*   ulListLen [in] ...
 */
/*   pausList  [out] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/*
 */
/* ʹ��Լ��:
 */
/*    1��ֻ����ʮ���������ַ�����Ϊ����
 */
/*    2��0��Ϊ���ֿ�ͷ���Լ��ո��������������checklist�������²�������
 */
/*
 */
/* �ַ�������Checklist:
 */
/*    1������ַ����ܳ����Ƿ�Ϸ�
 */
/*    2������Ƿ��з��ڴ��ַ�(�ո����������)
 */
/*    3������ַ����������Ĳ���ȡֵ�Ƿ����Ҫ�󣬰��������ַ������Ȳ��ܳ���5��
 */
/*       �������� 000000123 �����ַǷ�����
 */
/*    4������ַ����������Ĳ��������Ƿ���û������һ��
 */
/******************************************************************************
 */

static VOS_UINT32 initParaListU8( AT_PARSE_PARA_TYPE_STRU *pPara, VOS_UINT16 ulListLen, VOS_UINT8* paucList)
{
    VOS_UINT16 ulTmp;
    VOS_UINT8 *pParaData    = pPara->aucPara;
    VOS_UINT8  ucDigitNum   = 0;  /* ��¼�����ַ����������ܳ���5
 */
    VOS_UINT8  ucDigitValue = 0;  /* ��¼������ֵ��С�����ܳ���255
 */
    VOS_UINT16 usTmpListNum = 0;  /* ��¼�������������ָ��������ܳ���16
 */
    VOS_BOOL bDigit = FALSE;
    VOS_UINT32 ulRst        = ERR_MSP_SUCCESS;

    if ((NULL == pPara) ||(pPara->usParaLen > 2048))
    {
        /* 1������ַ����ܳ����Ƿ�Ϸ�
 */
        return ERR_MSP_INVALID_PARAMETER;
    }

    for(ulTmp = 0 ; ulTmp < pPara->usParaLen ; ulTmp++)
    {
        VOS_UINT8 ucChr = *pParaData;

        if(isdigit(ucChr))
        {
            if(!bDigit)
            {
                bDigit       = TRUE;
                ucDigitValue = 0;
                ucDigitNum   = 0;

                /* 4������ַ����������Ĳ��������Ƿ���û������һ��
 */
                if(++usTmpListNum > ulListLen)
                {
                    ulRst = ERR_MSP_INVALID_PARAMETER;
                    break;
                }
            }

            /* 3������ַ����������Ĳ���ȡֵ�Ƿ����Ҫ�󣬰��������ַ������Ȳ��ܳ���5��
 */
            if((++ucDigitNum>3) || ((255-ucDigitValue*10) < (ucChr-0x30)))
            {
                ulRst = ERR_MSP_INVALID_PARAMETER;
                break;
            }

            ucDigitValue = (VOS_UINT8)(ucDigitValue*10+(ucChr-0x30));
            paucList[usTmpListNum-1] = ucDigitValue;
        }
        else if(isspace(ucChr))
        {
            bDigit = FALSE;

			pParaData++;
			continue;
        }
        else
        {
            /* 2������Ƿ��з��ڴ��ַ�(�ո����������)
 */
            ulRst = ERR_MSP_INVALID_PARAMETER;
            break;
        }
        pParaData++;
    }

    /* 4��ǿ�Ƽ��:����ַ����������Ĳ��������Ƿ���û������һ��
 */
    if(usTmpListNum != ulListLen)
    {
        ulRst = ERR_MSP_INVALID_PARAMETER;
    }

    return ulRst;
}

/******************************************************************************
 */
/* ��������:  ^BANDSW ����Band�л�
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atSetBandSWPara(VOS_UINT8 ucClientId)
{
    FTM_SET_BANDSW_REQ_STRU stBandSwSetReq;
    VOS_UINT32 ulRst;

    /* 1��AT���������Ƿ���ȷ
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 2�����������Ƿ����Ҫ��
 */
 	if(5 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 3��ÿ�������ĳ����Ƿ���ȷ���Ƿ�Ϊ0
 */
    if((0 == gastAtParaList[0].usParaLen) ||
       (0 == gastAtParaList[1].usParaLen)||
       (0 == gastAtParaList[2].usParaLen)||
       (0 == gastAtParaList[3].usParaLen)||
       (0 == gastAtParaList[4].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /* 4��ÿ������ȡֵ��Լ��(ȡֵ��Χ��������������������)�Ƿ�����(ע:ҵ����ģ�鱣֤���˴���͸������)
 */

    stBandSwSetReq.enMode = (FCHAN_MODE_ENUM)(gastAtParaList[0].ulParaValue);
    stBandSwSetReq.ucBand = (VOS_UINT8)(gastAtParaList[1].ulParaValue);
    stBandSwSetReq.usUlCh = (VOS_UINT16)(gastAtParaList[2].ulParaValue);
    stBandSwSetReq.usDlCh = (VOS_UINT16)(gastAtParaList[3].ulParaValue);
	stBandSwSetReq.usDlAbbChanIndex= (VOS_UINT16)(gastAtParaList[4].ulParaValue);

    /* 5��������Ϣ�������ģ��:͸�������û����ϸ�ʽ�����룬���ж�����ֵ����ȷ��
 */
	
    /*ulRst = atLteSendMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_BANDSW_REQ, (VOS_VOID*)(&stBandSwSetReq), sizeof(stBandSwSetReq));
 */
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_BANDSW_REQ,ucClientId, (VOS_VOID*)(&stBandSwSetReq), sizeof(stBandSwSetReq));


    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_BANDSW_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;
}
VOS_UINT32 atSetBandSWParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_BANDSW_CNF_STRU *pstCnf = NULL;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_BANDSW_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}
/******************************************************************************
 */
/* ��������:   ^BANDSW ����Band�л�
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atQryBandSWPara(VOS_UINT8 ucClientId)
{
    FTM_RD_BANDSW_REQ_STRU stBandSWQryReq = {0};
    VOS_UINT32 ulRst;

	
    /*ulRst = atLteSendMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_BANDSW_REQ, (VOS_VOID*)(&stBandSWQryReq), sizeof(stBandSWQryReq));
 */
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_BANDSW_REQ, ucClientId,(VOS_VOID*)(&stBandSWQryReq), sizeof(stBandSWQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_BANDSW_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 atQryBandSWParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_BANDSW_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_BANDSW_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

    
    if(ERR_MSP_SUCCESS != pstCnf->ulErrCode)
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);
    }
    else
    {
    	usLength = 0;
    	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
    		"^BANDSW:%u,%u",pstCnf->enMode,pstCnf->ucBand);

    	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    }
    return AT_FW_CLIENT_STATUS_READY;
}

/******************************************************************************
 */
/* ��������:   ^FCHANS ����У׼����֧�֣����÷�������ŵ�
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atSetFFCHANSPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_FCHANS_REQ_STRU stFFCHANSSetReq = {0};
    VOS_UINT32 ulRst;

    /* 1��AT���������Ƿ���ȷ
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 2�����������Ƿ����Ҫ��
 */
    if(4 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 3��ÿ�������ĳ����Ƿ���ȷ���Ƿ�Ϊ0
 */
    if((0 == gastAtParaList[0].usParaLen) ||
       (0 == gastAtParaList[1].usParaLen) ||
       (0 == gastAtParaList[2].usParaLen) ||
       (0 == gastAtParaList[3].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 4��ÿ������ȡֵ��Լ��(ȡֵ��Χ��������������������)�Ƿ�����(ע:ҵ����ģ�鱣֤���˴���͸������)
 */
    stFFCHANSSetReq.usULFreqNum =(VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if((stFFCHANSSetReq.usULFreqNum >0) && (stFFCHANSSetReq.usULFreqNum <= CT_F_FREQ_LIST_MAX_NUM))
    {
        ulRst = initParaListU16((AT_PARSE_PARA_TYPE_STRU *)(&(gastAtParaList[1])),stFFCHANSSetReq.usULFreqNum,stFFCHANSSetReq.usULFreqList);
        if(ulRst != ERR_MSP_SUCCESS)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    stFFCHANSSetReq.usDLFreqNum= (VOS_UINT16)(gastAtParaList[2].ulParaValue);

    if((stFFCHANSSetReq.usDLFreqNum >0) && (stFFCHANSSetReq.usDLFreqNum <= CT_F_FREQ_LIST_MAX_NUM))
    {
        ulRst = initParaListU16((AT_PARSE_PARA_TYPE_STRU *)(&(gastAtParaList[3])),stFFCHANSSetReq.usDLFreqNum,stFFCHANSSetReq.usDLFreqList);
        if(ulRst != ERR_MSP_SUCCESS)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 5��������Ϣ�������ģ��:͸�������û����ϸ�ʽ�����룬���ж�����ֵ����ȷ��
 */
    
    /*ulRst = atLteSendMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_SET_FCHANS_REQ, (VOS_VOID*)(&stFFCHANSSetReq), sizeof(stFFCHANSSetReq));
 */
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_SET_FCHANS_REQ,ucClientId, (VOS_VOID*)(&stFFCHANSSetReq), sizeof(stFFCHANSSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FCHANS_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetFFCHANSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{

	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_F_FCHANS_CNF_STRU *pFchansCnf = NULL;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pFchansCnf = (FTM_SET_F_FCHANS_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pFchansCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;
}

/******************************************************************************
 */
/* ��������:    ^FCHANS ����У׼����֧�֣����÷�������ŵ�
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atQryFFCHANSPara(VOS_UINT8 ucClientId)
{
    FTM_RD_F_FCHANS_REQ_STRU stFFCHANQryReq = {0};
    VOS_UINT32 ulRst;

	
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_RD_FCHANS_REQ, ucClientId,(VOS_VOID*)(&stFFCHANQryReq), sizeof(stFFCHANQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FCHANS_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}
VOS_UINT32 atQryFFCHANSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    OS_MSG_STRU*pEvent = NULL;
    FTM_RD_F_FCHANS_CNF_STRU *pstCnf = NULL;
	VOS_UINT16 usLength = 0;
	VOS_UINT8 ucTmp = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_F_FCHANS_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
        return ERR_MSP_FAILURE;
	}

    
    if(ERR_MSP_SUCCESS != pstCnf->ulErrCode)
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);
    }
    else
    {

    	usLength=0;

        /* ^FCHANS:
 */
    	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
            "\r\n^FCHANS:\r\n");

        /* < channel_ul_num >[,< channel_ul_list >]
 */

    	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
         	"\r\n%u",pstCnf->usULFreqNum);

    	if ((pstCnf->usULFreqNum > 0) && (pstCnf->usULFreqNum <= CT_F_FREQ_LIST_MAX_NUM))
    	{
    		usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
         			",%u",pstCnf->usULFreqList[0]);

    		for( ucTmp = 1 ; ucTmp < pstCnf->usULFreqNum ; ucTmp++)
    		{
    			usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
         			" %u",pstCnf->usULFreqList[ucTmp]);
    		}
    	}
    	else
    	{
    		/*pstCnf->usULFreqNum Ϊ�޷����˷�ֻ֧Ϊ0��û�и������
 */
    		/*TODO:���pstCnf->usULFreqNum����CT_F_FREQ_UL_LIST_MAX_NUM���˴�����SDM��Ϣ
 */
    	}

    	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
           			(VOS_CHAR *)pgucLAtSndCodeAddr,
           			(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
         			"%s",
        	 		"\r\n");

    	/* < channel_dl_num >[, < channel_dl_list >]
 */

    	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
         	"\r\n%u",pstCnf->usDLFreqNum);

    	if ((pstCnf->usDLFreqNum > 0) && (pstCnf->usDLFreqNum <= CT_F_FREQ_LIST_MAX_NUM))
    	{
    		usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
         			",%u",pstCnf->usDLFreqList[0]);

    		for( ucTmp = 1 ; ucTmp < pstCnf->usDLFreqNum ; ucTmp++)
    		{
    			usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
         			" %u", pstCnf->usDLFreqList[ucTmp]);
    		}
    	}
    	else
    	{
    		/*pstCnf->usDLFreqNum Ϊ�޷����˷�ֻ֧Ϊ0��û�и������
 */
    		/*TODO:���pstCnf->usDLFreqNum����CT_F_FREQ_DL_LIST_MAX_NUM���˴�����SDM��Ϣ
 */
    	}

    	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
         			"%s","\r\n");

    	 CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    }
    return AT_FW_CLIENT_STATUS_READY;
}
/******************************************************************************
 */
/* ��������:   ^FSEGMENT ����У׼����֧�֣�����SEGMENT����
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atSetFFSEGMENTPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_FSEGMENT_REQ_STRU stFFSegmentSetReq = {0};
    VOS_UINT32 ulRst;

    /* 1��AT���������Ƿ���ȷ
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 2�����������Ƿ����Ҫ��
 */
    if(3 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 3��ÿ�������ĳ����Ƿ���ȷ���Ƿ�Ϊ0
 */
    if((0 == gastAtParaList[0].usParaLen)||
        (0 == gastAtParaList[1].usParaLen)||
        (0 == gastAtParaList[2].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 4��ÿ������ȡֵ��Լ��(ȡֵ��Χ��������������������)�Ƿ�����(ע:ҵ����ģ�鱣֤���˴���͸������)
 */
    stFFSegmentSetReq.usSegmentLen   = (VOS_UINT16)(gastAtParaList[0].ulParaValue);
    stFFSegmentSetReq.usMeasOffset   = (VOS_UINT16)(gastAtParaList[1].ulParaValue);
    stFFSegmentSetReq.usMeasLen      = (VOS_UINT16)(gastAtParaList[2].ulParaValue);

    /* 5��������Ϣ�������ģ��:͸�������û����ϸ�ʽ�����룬���ж�����ֵ����ȷ��
 */
    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_SET_FSEGMENT_REQ, ucClientId,(VOS_VOID*)(&stFFSegmentSetReq), sizeof(stFFSegmentSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FSEGMENT_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}
VOS_UINT32 atSetFFSEGMENTParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{

   	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_F_FSEGMENT_CNF_STRU *pFsegmentCnf = NULL;


    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pFsegmentCnf = (FTM_SET_F_FSEGMENT_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pFsegmentCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;
}
/******************************************************************************
 */
/* ��������:    ^FSEGMENT ����У׼����֧�֣�����SEGMENT����
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atQryFFSEGMENTPara(VOS_UINT8 ucClientId)
{
    FTM_RD_F_FSEGMENT_REQ_STRU stFFSegmentQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_RD_FSEGMENT_REQ,ucClientId, (VOS_VOID*)(&stFFSegmentQryReq), sizeof(stFFSegmentQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FSEGMENT_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
VOS_UINT32 atQryFFSEGMENTParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_F_FSEGMENT_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_F_FSEGMENT_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
		return ERR_MSP_FAILURE;
	}

	usLength = 0;
	 usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^FSEGMENT:%u,%u,%u",pstCnf->usSegmentLen,pstCnf->usMeasOffset,pstCnf->usMeasLen);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}
/******************************************************************************
 */
/* ��������:   ^FPOWS ����У׼����֧�֣����÷�����ķ��书��
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atSetFFPOWSPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_FPOWS_REQ_STRU stFFPOWSSetReq = {0};
    VOS_UINT32 ulRst;

    /* 1��AT���������Ƿ���ȷ
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 2�����������Ƿ����Ҫ��
 */
    if(2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 3��ÿ�������ĳ����Ƿ���ȷ���Ƿ�Ϊ0
 */
    if((0 == gastAtParaList[0].usParaLen) ||
       (0 == gastAtParaList[1].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 4��ÿ������ȡֵ��Լ��(ȡֵ��Χ��������������������)�Ƿ�����(ע:ҵ����ģ�鱣֤���˴���͸������)
 */
    stFFPOWSSetReq.usTxPowerNum =(VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if((stFFPOWSSetReq.usTxPowerNum > 0) && (stFFPOWSSetReq.usTxPowerNum <= CT_F_TXPOW_SET_MAX_NUM))
    {
        ulRst = initParaListS16((AT_PARSE_PARA_TYPE_STRU *)(&(gastAtParaList[1])),stFFPOWSSetReq.usTxPowerNum,stFFPOWSSetReq.asTxPower);
        if(ulRst != ERR_MSP_SUCCESS)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 5��������Ϣ�������ģ��:͸�������û����ϸ�ʽ�����룬���ж�����ֵ����ȷ��
 */
	
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_SET_FPOWS_REQ, ucClientId,(VOS_VOID*)(&stFFPOWSSetReq), sizeof(stFFPOWSSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FPOWS_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;
}
VOS_UINT32 atSetFFPOWSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_F_FPOWS_CNF_STRU *pFpowsCnf = NULL;


    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pFpowsCnf = (FTM_SET_F_FPOWS_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pFpowsCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}
/******************************************************************************
 */
/* ��������:    ^FPOWS ����У׼����֧�֣����÷�����ķ��书��
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atQryFFPOWSPara(VOS_UINT8 ucClientId)
{
    FTM_RD_F_FPOWS_REQ_STRU stFFPowsQryReq = {0};
    VOS_UINT32 ulRst;

	
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_RD_FPOWS_REQ,ucClientId, (VOS_VOID*)(&stFFPowsQryReq), sizeof(stFFPowsQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FPOWS_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
VOS_UINT32 atQryFFPOWSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_F_FPOWS_CNF_STRU *pstCnf =  NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;
	VOS_UINT8 ucTmp = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_F_FPOWS_CNF_STRU *)pEvent->ulParam1;


	if(NULL == pstCnf)
	{
	    return ERR_MSP_FAILURE;
	}

	/* ^FPOWS:< power_num >[,< power_list >]
 */
    usLength = 0;
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^FPOWS:%u",pstCnf->usTxPowerNum);

	if ((pstCnf->usTxPowerNum > 0) && (pstCnf->usTxPowerNum <= CT_F_TXPOW_SET_MAX_NUM))
	{
		usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
				",%d",pstCnf->asTxPower[0]);

		for( ucTmp = 1 ; ucTmp < pstCnf->usTxPowerNum ; ucTmp++)
		{
			usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
				" %d",pstCnf->asTxPower[ucTmp]);
		}
	}
	else
	{
		/*pstCnf->usTxPowerNum Ϊ�޷����˷�ֻ֧Ϊ0��û�и������
 */
		/*TODO:���pstCnf->usTxPowerNum����CT_F_TXPOW_SET_MAX_NUM���˴�����SDM��Ϣ
 */
	}
	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}
/******************************************************************************
 */
/* ��������:   ^FPAS ����У׼����֧�֣����÷����PA�ĵȼ�
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atSetFFPASPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_FPAS_REQ_STRU stFFPASSetReq = {0};
    VOS_UINT32 ulRst;

    /* 1��AT���������Ƿ���ȷ
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 2�����������Ƿ����Ҫ��
 */
    if(2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 3��ÿ�������ĳ����Ƿ���ȷ���Ƿ�Ϊ0
 */
    if((0 == gastAtParaList[0].usParaLen) ||
       (0 == gastAtParaList[1].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 4��ÿ������ȡֵ��Լ��(ȡֵ��Χ��������������������)�Ƿ�����(ע:ҵ����ģ�鱣֤���˴���͸������)
 */
    stFFPASSetReq.usPaLevelNum =(VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if((stFFPASSetReq.usPaLevelNum > 0) && (stFFPASSetReq.usPaLevelNum <= CT_F_PA_SET_MAX_NUM))
    {
        ulRst = initParaListU8((AT_PARSE_PARA_TYPE_STRU *)(&(gastAtParaList[1])),stFFPASSetReq.usPaLevelNum,stFFPASSetReq.aucPaLevelList);
        if(ulRst != ERR_MSP_SUCCESS)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 5��������Ϣ�������ģ��:͸�������û����ϸ�ʽ�����룬���ж�����ֵ����ȷ��
 */
	
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_SET_FPAS_REQ, ucClientId,(VOS_VOID*)(&stFFPASSetReq), sizeof(stFFPASSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FPAS_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;
}
VOS_UINT32 atSetFFPASParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_F_FPAS_CNF_STRU *pFpasCnf = NULL;


    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pFpasCnf = (FTM_SET_F_FPAS_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pFpasCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}
/******************************************************************************
 */
/* ��������:    ^FPAS ����У׼����֧�֣����÷����PA�ĵȼ�
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atQryFFPASPara(VOS_UINT8 ucClientId)
{
    FTM_RD_F_FPAS_REQ_STRU stFFPasQryReq = {0};
    VOS_UINT32 ulRst;

	
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_RD_FPAS_REQ,ucClientId, (VOS_VOID*)(&stFFPasQryReq), sizeof(stFFPasQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FPAS_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}
VOS_UINT32 atQryFFPASParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	FTM_RD_F_FPAS_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;
	VOS_UINT8 ucTmp = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_F_FPAS_CNF_STRU *)pEvent->ulParam1;


	if(NULL == pstCnf)
	{
	    return ERR_MSP_FAILURE;
	}
	/* ^FPAS:< level_num >,< level_list >
 */

	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^FPAS:%u",pstCnf->usPaLevelNum);

	if ((pstCnf->usPaLevelNum > 0) && (pstCnf->usPaLevelNum <= CT_F_PA_SET_MAX_NUM))
	{
		usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
				",%u",pstCnf->aucPaLevelList[0]);

		for( ucTmp = 1 ; ucTmp < pstCnf->usPaLevelNum ; ucTmp++)
		{
			usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
				" %u",pstCnf->aucPaLevelList[ucTmp]);
		}
	}
	else
	{
		/*pstCnf->usPaLevelNum Ϊ�޷����˷�ֻ֧Ϊ0��û�и������
 */
		/*TODO:���pstCnf->usPaLevelNum����CT_F_PA_SET_MAX_NUM���˴�����SDM��Ϣ
 */
	}
	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}
/******************************************************************************
 */
/* ��������:   ^FLNAS ����У׼����֧�֣����ý��ջ�LNA�ĵȼ�
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atSetFFLNASPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_AAGC_REQ_STRU stFAAGCSetReq = {0};
    VOS_UINT32 ulRst;

    /* 1��AT���������Ƿ���ȷ
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 2�����������Ƿ����Ҫ��
 */
    if(2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 3��ÿ�������ĳ����Ƿ���ȷ���Ƿ�Ϊ0
 */
    if((0 == gastAtParaList[0].usParaLen) ||
       (0 == gastAtParaList[1].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 4��ÿ������ȡֵ��Լ��(ȡֵ��Χ��������������������)�Ƿ�����(ע:ҵ����ģ�鱣֤���˴���͸������)
 */
    stFAAGCSetReq.usAagcNum =(VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if((stFAAGCSetReq.usAagcNum > 0 ) && (stFAAGCSetReq.usAagcNum <= CT_F_AAGC_SET_MAX_NUM))
    {
        ulRst = initParaListU8((AT_PARSE_PARA_TYPE_STRU *)(&(gastAtParaList[1])),stFAAGCSetReq.usAagcNum,stFAAGCSetReq.aucAAGCValue);
        if(ulRst != ERR_MSP_SUCCESS)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 5��������Ϣ�������ģ��:͸�������û����ϸ�ʽ�����룬���ж�����ֵ����ȷ��
 */
    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_SET_FLNAS_REQ,ucClientId ,(VOS_VOID*)(&stFAAGCSetReq), sizeof(stFAAGCSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FLNAS_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;
}
VOS_UINT32 atSetFFLNASParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{

	OS_MSG_STRU* pEvent = NULL;
	FTM_SET_F_AAGC_CNF_STRU *pFlnasCnf = NULL;

	pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pFlnasCnf = (FTM_SET_F_AAGC_CNF_STRU *)pEvent->ulParam1;

	CmdErrProc(ucClientId, pFlnasCnf->ulErrCode, 0, NULL);

	return AT_FW_CLIENT_STATUS_READY;

}
/******************************************************************************
 */
/* ��������:    ^FLNAS ����У׼����֧�֣����ý��ջ�LNA�ĵȼ�
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atQryFFLNASPara(VOS_UINT8 ucClientId)
{
    FTM_RD_F_AAGC_REQ_STRU stFAagcsQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_RD_FLNAS_REQ, ucClientId,(VOS_VOID*)(&stFAagcsQryReq), sizeof(stFAagcsQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FLNAS_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
VOS_UINT32 atQryFFLNASParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    FTM_RD_F_AAGC_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;
	VOS_UINT8 ucTmp = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_F_AAGC_CNF_STRU *)pEvent->ulParam1;


	if(NULL == pstCnf)
	{
	    return ERR_MSP_FAILURE;
	}
	/* ^FLNAS:< level_num >[,< level_list >]
 */
    usLength = 0;
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^FLNAS:%u",pstCnf->usAagcNum);

	if ((pstCnf->usAagcNum > 0) && (pstCnf->usAagcNum <= CT_F_AAGC_SET_MAX_NUM))
	{
		usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
				",%u",pstCnf->aucAAGCValue[0]);

		for( ucTmp = 1 ; ucTmp < pstCnf->usAagcNum ; ucTmp++)
		{
			usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
				" %u",pstCnf->aucAAGCValue[ucTmp]);
		}
	}
	else
	{
		/*pstCnf->usAagcNum Ϊ�޷����˷�ֻ֧Ϊ0��û�и������
 */
		/*TODO:���pstCnf->usAagcNum����CT_F_AAGC_SET_MAX_NUM���˴�����SDM��Ϣ
 */
	}
	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
	return AT_FW_CLIENT_STATUS_READY;

}
/******************************************************************************
 */
/* ��������:   ^FTXWAVE ����У׼����֧�֣����÷���Ĳ���
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atSetFFTXWAVEPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_FTXWAVE_REQ_STRU stFTxWaveSetReq = {0};
    VOS_UINT32 ulRst;

    /* 1��AT���������Ƿ���ȷ
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 2�����������Ƿ����Ҫ��
 */
    if(1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 3��ÿ�������ĳ����Ƿ���ȷ���Ƿ�Ϊ0
 */
    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 4��ÿ������ȡֵ��Լ��(ȡֵ��Χ��������������������)�Ƿ�����(ע:ҵ����ģ�鱣֤���˴���͸������)
 */
    stFTxWaveSetReq.usWaveType =(VOS_UINT16)(gastAtParaList[0].ulParaValue);

    /* 5��������Ϣ�������ģ��:͸�������û����ϸ�ʽ�����룬���ж�����ֵ����ȷ��
 */
    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_SET_FTXWAVE_REQ,ucClientId, (VOS_VOID*)(&stFTxWaveSetReq), sizeof(stFTxWaveSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FTXWAVE_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;
}
VOS_UINT32 atSetFFTXWAVEParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
	FTM_SET_F_FTXWAVE_CNF_STRU *pFtxwaveCnf = NULL;


	pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pFtxwaveCnf = (FTM_SET_F_FTXWAVE_CNF_STRU *)pEvent->ulParam1;

	CmdErrProc(ucClientId, pFtxwaveCnf->ulErrCode, 0, NULL);

	return AT_FW_CLIENT_STATUS_READY;

}
/******************************************************************************
 */
/* ��������:   ^FTXWAVE ����У׼����֧�֣����÷���Ĳ���
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atQryFFTXWAVEPara(VOS_UINT8 ucClientId)
{
    FTM_RD_F_FTXWAVE_REQ_STRU stFFTxwaveQryReq = {0};
    VOS_UINT32 ulRst;
    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_RD_FTXWAVE_REQ, ucClientId,(VOS_VOID*)(&stFFTxwaveQryReq), sizeof(stFFTxwaveQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FTXWAVE_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
VOS_UINT32 atQryFFTXWAVEParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    FTM_RD_F_FTXWAVE_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;
	/*VOS_UINT8 ucTmp = 0;
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_F_FTXWAVE_CNF_STRU *)pEvent->ulParam1;


	if(NULL == pstCnf)
	{
	    return ERR_MSP_FAILURE;
	}

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
    	"^FTXWAVE:%u",pstCnf->usWaveType);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}
/******************************************************************************
 */
/* ��������:   ^FSTART ����У׼����֧�֣�����TRIGGER��У׼����
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atSetFFSTARTPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_TRIGGER_REQ_STRU stFTriggerSetReq = {0};
    VOS_UINT32 ulRst;

    /* 1��AT���������Ƿ���ȷ
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 2�����������Ƿ����Ҫ��
 */
    if(1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 3��ÿ�������ĳ����Ƿ���ȷ���Ƿ�Ϊ0
 */
    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 4��ÿ������ȡֵ��Լ��(ȡֵ��Χ��������������������)�Ƿ�����(ע:ҵ����ģ�鱣֤���˴���͸������)
 */

    stFTriggerSetReq.ucType = (VOS_UINT8)(gastAtParaList[0].ulParaValue);

    /* 5��������Ϣ�������ģ��:͸�������û����ϸ�ʽ�����룬���ж�����ֵ����ȷ��
 */
    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_SET_FSTART_REQ,ucClientId ,(VOS_VOID*)(&stFTriggerSetReq), sizeof(stFTriggerSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FSTART_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;
}
VOS_UINT32 atSetFFSTARTParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{

	OS_MSG_STRU* pEvent = NULL;
	FTM_SET_F_TRIGGER_CNF_STRU *pFstartCnf = NULL;

	pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pFstartCnf = (FTM_SET_F_TRIGGER_CNF_STRU *)pEvent->ulParam1;

	CmdErrProc(ucClientId, pFstartCnf->ulErrCode, 0, NULL);

	return AT_FW_CLIENT_STATUS_READY;

}
/******************************************************************************
 */
/* ��������:   ^FSTART ����У׼����֧�֣�����TRIGGER��У׼����
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atQryFFSTARTPara(VOS_UINT8 ucClientId)
{
    FTM_RD_F_TRIGGER_REQ_STRU stFTriggerQryReq = {0};
    VOS_UINT32 ulRst;
    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_RD_FSTART_REQ, ucClientId,(VOS_VOID*)(&stFTriggerQryReq), sizeof(stFTriggerQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FSTART_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 atQryFFSTARTParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    FTM_RD_F_TRIGGER_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;
	/*VOS_UINT8 ucTmp = 0;
 */

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_F_TRIGGER_CNF_STRU *)pEvent->ulParam1;


	if(NULL == pstCnf)
	{
	    return ERR_MSP_FAILURE;
	}

	usLength = 0;
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^FSTART:%u,%u",pstCnf->ucStatus,pstCnf->ucType);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}
/******************************************************************************
 */
/* ��������:   ^FRSSIS ����У׼֧�֣���ѯRSSI��������
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atSetFFRSSISPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_FRSSIS_REQ_STRU stFFRSSISSetReq = {0};
    VOS_UINT32 ulRst;

    /* 1��AT���������Ƿ���ȷ
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 2�����������Ƿ����Ҫ��
 */
    if(1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 3��ÿ�������ĳ����Ƿ���ȷ���Ƿ�Ϊ0
 */
    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 4��ÿ������ȡֵ��Լ��(ȡֵ��Χ��������������������)�Ƿ�����(ע:ҵ����ģ�鱣֤���˴���͸������)
 */
    stFFRSSISSetReq.usDLFreq = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    /* 5��������Ϣ�������ģ��:͸�������û����ϸ�ʽ�����룬���ж�����ֵ����ȷ��
 */
    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_SET_FRSSIS_REQ, ucClientId,(VOS_VOID*)(&stFFRSSISSetReq), sizeof(stFFRSSISSetReq));
    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FRSSIS_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;
}
VOS_UINT32 atSetFFRSSISParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{

	OS_MSG_STRU* pEvent = NULL;
	FTM_SET_F_FRSSIS_CNF_STRU *pstCnf = NULL;
	VOS_UINT16 usLength = 0;
	VOS_UINT8 ucTmp = 0;

	pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pstCnf = (FTM_SET_F_FRSSIS_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
	    return ERR_MSP_FAILURE;
	}

	/* ^FRSSIS:< rvalue_num >[,< rvalue_list_1 >,< rvalue_list_2 >]
 */
	usLength = 0;
	usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
			"^FRSSIS:%d",pstCnf->usRSSIPowNum);

	/* ����1�Ĺ����б�
 */
	if ((pstCnf->usRSSIPowNum > 0) && (pstCnf->usRSSIPowNum <= RSSI_POW_MAX_NUM))
	{
		usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
			",%d",pstCnf->ausRxANT1RSSI[0]);
		for( ucTmp = 1 ; ucTmp < pstCnf->usRSSIPowNum ; ucTmp++)
		{
			usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
				" %d",pstCnf->ausRxANT1RSSI[ucTmp]);
		}
	}
	else
	{
		/*pstCnf->usAagcNum Ϊ�޷����˷�ֻ֧Ϊ0��û�и������
 */
		/*TODO:���pstCnf->usAagcNum����CT_F_AAGC_SET_MAX_NUM���˴�����SDM��Ϣ
 */
	}

	/* ����2�Ĺ����б�
 */
	if ((pstCnf->usRSSIPowNum > 0) && (pstCnf->usRSSIPowNum <= RSSI_POW_MAX_NUM))
	{
		usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
			",%d",pstCnf->ausRxANT2RSSI[0]);
		for( ucTmp = 1 ; ucTmp < pstCnf->usRSSIPowNum ; ucTmp++)
		{
			usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
				" %d",pstCnf->ausRxANT2RSSI[ucTmp]);
		}
	}
	else
	{
		/*pstCnf->usAagcNum Ϊ�޷����˷�ֻ֧Ϊ0��û�и������
 */
		/*TODO:���pstCnf->usAagcNum����CT_F_AAGC_SET_MAX_NUM���˴�����SDM��Ϣ
 */
	}
	CmdErrProc(ucClientId, pstCnf->ulErrCode,usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}


/******************************************************************************
 */
/* ��������:   ^FCMTMS ����У׼֧�֣���ѯоƬ�¶ȹ�������
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */
VOS_UINT32 atSetFFCMTMSPara(VOS_UINT8 ucClientId)
{
    FTM_RD_F_FCMTMS_REQ_STRU stFFCmtmsQryReq = {0};
    VOS_UINT32 ulRst;
    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_RD_FCMTMS_REQ,ucClientId, (VOS_VOID*)(&stFFCmtmsQryReq), sizeof(stFFCmtmsQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FCMTMS_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
VOS_UINT32 atSetFFCMTMSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
	FTM_RD_F_FCMTMS_CNF_STRU *pstCnf = NULL;
	VOS_UINT16 usLength = 0;
	VOS_UINT8 ucTmp = 0;

	pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pstCnf = (FTM_RD_F_FCMTMS_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
	    return ERR_MSP_FAILURE;
	}

	/* ^FCMTMS:< channel_num >[,< temperature_list >
 */
	usLength = 0;
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^FCMTMS:%u",pstCnf->usCmtmsNum);

	if ((pstCnf->usCmtmsNum > 0) && (pstCnf->usCmtmsNum <= CT_F_FREQ_LIST_MAX_NUM))
	{
		usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
				",%u",pstCnf->ausCmtms[0]);

		for( ucTmp = 1 ; ucTmp < pstCnf->usCmtmsNum ; ucTmp++)
		{
			usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
				" %u",pstCnf->ausCmtms[ucTmp]);
		}
	}
	else
	{
		/*pstCnf->usAagcNum Ϊ�޷����˷�ֻ֧Ϊ0��û�и������
 */
		/*TODO:���pstCnf->usAagcNum����CT_F_AAGC_SET_MAX_NUM���˴�����SDM��Ϣ
 */
	}
	CmdErrProc(ucClientId, pstCnf->ulErrCode,usLength, pgucLAtSndCodeAddr);

    return ERR_MSP_SUCCESS;

}

/******************************************************************************
 */
/* ��������:   ^FBLK ����У׼֧�֣�
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */

VOS_UINT32 atSetFBLKPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_FBLK_REQ_STRU stFBLK = {0};
    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������
 */
    if(gucAtParaIndex > 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���û��Ĭ�����õ�ģʽ������ʧ��
 */
    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        stFBLK.ulBlocking= gastAtParaList[0].ulParaValue;
    }

    /* ִ���������
 */
    
    if(AT_SUCCESS == atSendFtmDataMsg(MSP_SYS_FTM_PID,ID_MSG_FTM_F_SET_FBLK_REQ,ucClientId,
        (VOS_VOID*)(&stFBLK), sizeof(FTM_SET_F_FBLK_REQ_STRU)))
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FBLK_SET;

        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetFBLKParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_F_FBLK_CNF_STRU *pFBLKCnf = NULL;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pFBLKCnf = (FTM_SET_F_FBLK_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pFBLKCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}
VOS_UINT32 atQryFBLKPara(VOS_UINT8 ucClientId)
{
    FTM_RD_F_FBLK_REQ_STRU stQryReq = {0};
    VOS_UINT32 ulRst;

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_RD_FBLK_REQ,ucClientId, (VOS_VOID*)(&stQryReq), sizeof(stQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FBLK_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}
VOS_UINT32 atQryFBLKParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    FTM_RD_F_FBLK_CNF_STRU *pstCnf = NULL;
	OS_MSG_STRU*pEvent = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_F_FBLK_CNF_STRU *)pEvent->ulParam1;


	if(NULL == pstCnf)
	{
	    return ERR_MSP_FAILURE;
	}

	usLength = 0;
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^FBLK:%u",pstCnf->ulBlocking);

	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/******************************************************************************
 */
/* ��������:   ^FIPSTART ����У׼֧�֣�
 */
/*
 */
/* ����˵��:
 */
/*   ulIndex [in] ...
 */
/*                ...
 */
/*
 */
/* �� �� ֵ:
 */
/*    TODO: ...
 */
/******************************************************************************
 */

VOS_UINT32 atSetFIPSTARTPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_FIPSTART_REQ_STRU stFIPSTART = {0};
    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������
 */
    if(gucAtParaIndex > 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���û��Ĭ�����õ�ģʽ������ʧ��
 */
    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        stFIPSTART.ulChannel= gastAtParaList[0].ulParaValue;
    }

    /* ִ���������
 */
    
    if(AT_SUCCESS == atSendFtmDataMsg(MSP_SYS_FTM_PID,ID_MSG_FTM_F_SET_FIPSTART_REQ,ucClientId,
        (VOS_VOID*)(&stFIPSTART), sizeof(FTM_SET_F_FIPSTART_REQ_STRU)))
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FIPSTART_SET;

        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetFIPSTARTParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_F_FIPSTART_CNF_STRU *pFIPSTART = NULL;
	VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pFIPSTART = (FTM_SET_F_FIPSTART_CNF_STRU *)pEvent->ulParam1;

	usLength = 0;
	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
		"^FIPSTART:%d,%d",pFIPSTART->ulIp2Value1,pFIPSTART->ulIp2Value2);

    CmdErrProc(ucClientId, pFIPSTART->ulErrCode, usLength, pgucLAtSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atSetFPDMSPara(VOS_UINT8 ucClientId)
{

    FTM_SET_F_FPDMS_REQ_STRU stFPDMS = {0};
    /* �������
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��������
 */
    if(gucAtParaIndex > 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���û��Ĭ�����õ�ģʽ������ʧ��
 */
    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        stFPDMS.ulSwitch= gastAtParaList[0].ulParaValue;
    }

    /* ִ���������
 */
    
    if(AT_SUCCESS == atSendFtmDataMsg(MSP_SYS_FTM_PID,ID_MSG_FTM_SET_FPDMS_REQ,ucClientId,
        (VOS_VOID*)(&stFPDMS), sizeof(FTM_SET_F_FPDMS_REQ_STRU)))
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FPDMS_SET;

        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}

VOS_UINT32 atSetFPDMSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    OS_MSG_STRU* pEvent = NULL;
    FTM_SET_F_FPDMS_CNF_STRU *pFPDMS = NULL;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pFPDMS = (FTM_SET_F_FPDMS_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pFPDMS->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;
}
VOS_UINT32 atQryFPDMSPara(VOS_UINT8 ucClientId)
{
    FTM_RD_F_FPDMS_REQ_STRU stFPDMSQryReq = {0};
    VOS_UINT32 ulRst;

    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_RD_FPDMS_REQ, ucClientId, (VOS_VOID*)(&stFPDMSQryReq), sizeof(stFPDMSQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FPDMS_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
VOS_UINT32 atQryFPDMSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    FTM_RD_F_FPDMS_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_F_FPDMS_CNF_STRU *)pEvent->ulParam1;


    if(NULL == pstCnf)
    {
        return ERR_MSP_FAILURE;
    }

    usLength = 0;
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
        (VOS_CHAR *)pgucLAtSndCodeAddr,
        (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
        "^FPDMS:%d",
        pstCnf->ulSwitch);

    CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}

/***************************************************************************************
 */
/*
 */
/*
 */
VOS_UINT32 atSetFPAPOWERPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_FPAPOWER_REQ_STRU stSetReq = {0};
    VOS_UINT32 ulRst;

    /* 1��AT���������Ƿ���ȷ
 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 2�����������Ƿ����Ҫ��
 */
    if(2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 3��ÿ�������ĳ����Ƿ���ȷ���Ƿ�Ϊ0
 */
    if((0 == gastAtParaList[0].usParaLen) ||
       (0 == gastAtParaList[1].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 4��ÿ������ȡֵ��Լ��(ȡֵ��Χ��������������������)�Ƿ�����(ע:ҵ����ģ�鱣֤���˴���͸������)
 */
    stSetReq.usPaLeverNum = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    if((stSetReq.usPaLeverNum > 0) && (stSetReq.usPaLeverNum <= CT_F_TXPOW_SET_MAX_NUM))
    {
        ulRst = initParaListS16((AT_PARSE_PARA_TYPE_STRU *)(&(gastAtParaList[1])),stSetReq.usPaLeverNum,(VOS_INT16*)(stSetReq.usMaxPowerList));
        if(ulRst != ERR_MSP_SUCCESS)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 5��������Ϣ�������ģ��:͸�������û����ϸ�ʽ�����룬���ж�����ֵ����ȷ��
 */
    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_SET_FPAPOWER_REQ,ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FPAPOWER_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}
VOS_UINT32 atSetFPAPOWERParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_F_FPAPOWER_CNF_STRU *pstCnf = NULL;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_F_FPAPOWER_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}
VOS_UINT32 atQryFPAPOWERPara(VOS_UINT8 ucClientId)
{
    
    FTM_RD_F_FPAPOWER_REQ_STRU stFPAPOWERQryReq = {0};
    VOS_UINT32 ulRst;

    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_RD_FPAPOWER_REQ, ucClientId, (VOS_VOID*)(&stFPAPOWERQryReq), sizeof(stFPAPOWERQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FPAPOWER_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
VOS_UINT32 atQryFPAPOWERParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    
    FTM_RD_F_FPAPOWER_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    VOS_UINT16 usLength = 0;
    VOS_UINT8 ucTmp = 0;
    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_F_FPAPOWER_CNF_STRU *)pEvent->ulParam1;
    if(NULL == pstCnf)
    {
        return ERR_MSP_FAILURE;
    }
    usLength = 0;
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
        (VOS_CHAR *)pgucLAtSndCodeAddr,
        (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
        "^FPAPOWER:%u,",
        pstCnf->usPaLeverNum);

	if ((pstCnf->usPaLeverNum > 0) && (pstCnf->usPaLeverNum <= CT_F_TXPOW_SET_MAX_NUM))
	{
		for( ucTmp = 0 ; ucTmp < pstCnf->usPaLeverNum ; ucTmp++)
		{
			usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
				" %d",pstCnf->usMaxPowerList[ucTmp]);
		}
	}
	else
	{
		/*pstCnf->usPaLeverNum Ϊ�޷����˷�ֻ֧Ϊ0��û�и������
 */
		/*TODO:���pstCnf->usTxPowerNum����CT_F_TXPOW_SET_MAX_NUM���˴�����SDM��Ϣ
 */
	}
	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

/***************************************************************************************
 */
/*
 */
/*
 */
VOS_UINT32 atSetFMAXPOWERPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_FMAXPOWER_REQ_STRU stSetReq = {0};
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

    if((0 == gastAtParaList[0].usParaLen)||
    (0 == gastAtParaList[1].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    stSetReq.usPaLevelNum= (VOS_UINT16)(gastAtParaList[0].ulParaValue);
	stSetReq.usPaReduceGain = (VOS_UINT16)(gastAtParaList[1].ulParaValue);

    
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_SET_FMAXPOWER_REQ, ucClientId, (VOS_VOID*)(&stSetReq), sizeof(stSetReq));

    if(AT_SUCCESS == ulRst)
    {
        /* ���õ�ǰ��������
 */
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FMAXPOWER_SET;
        return AT_WAIT_ASYNC_RETURN;    /* ������������״̬
 */
    }

    return AT_ERROR;

}
VOS_UINT32 atSetFMAXPOWERParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
    FTM_SET_F_FMAXPOWER_CNF_STRU *pstCnf = NULL;


    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_SET_F_FMAXPOWER_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;

}

VOS_UINT32 atQryFMAXPOWERPara(VOS_UINT8 ucClientId)
{
    
    FTM_RD_F_FMAXPOWER_REQ_STRU stFMAXPOWERQryReq = {0};
    VOS_UINT32 ulRst;

    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_RD_FMAXPOWER_REQ,ucClientId, (VOS_VOID*)(&stFMAXPOWERQryReq), sizeof(stFMAXPOWERQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FMAXPOWER_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
VOS_UINT32 atQryFMAXPOWERParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    

    FTM_RD_F_FMAXPOWER_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    VOS_UINT16 usLength = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_F_FMAXPOWER_CNF_STRU *)pEvent->ulParam1;


    if(NULL == pstCnf)
    {
        return ERR_MSP_FAILURE;
    }

    usLength = 0;
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
        (VOS_CHAR *)pgucLAtSndCodeAddr,
        (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
        "^FMAXPOWER:%u,%d",
        pstCnf->usPaLevelNum,pstCnf->usPaReduceGain);

    CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}

// *****************************************************************************
// ��������:   ^FCALPDDCS ����У׼֧�֣���������PDֱ��У׵
//
// ����˵��:
//   ulIndex [in] ...�ŵ���
//                ...
//
// �� �� ֵ:
//    TODO: ...
// *****************************************************************************
VOS_UINT32 atSetFCALPDDCSPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_FCALPDDCS_REQ_STRU stFCALPDDCSSetReq = {0};
    VOS_UINT32 ulRst;

    // 1��AT���������Ƿ���ȷ
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    // 2�����������Ƿ����Ҫ��
    if(1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    // 3��ÿ�������ĳ����Ƿ���ȷ���Ƿ�Ϊ0
    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    // 4��ÿ������ȡֵ��Լ��(ȡֵ��Χ��������������������)�Ƿ�����(ע:ҵ����ģ�鱣֤���˴���͸������)
    stFCALPDDCSSetReq.ulPdAutoFlg = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    // 5��������Ϣ�������ģ��:͸�������û����ϸ�ʽ�����룬���ж�����ֵ����ȷ��
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_SET_FCALPDDCS_REQ, ucClientId,(VOS_VOID*)(&stFCALPDDCSSetReq), sizeof(stFCALPDDCSSetReq));
    if(AT_SUCCESS == ulRst)
    {
        // ���õ�ǰ��������
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FCALPDDCS_SET;
        return AT_WAIT_ASYNC_RETURN;    // ������������״̬
    }

    return AT_ERROR;
}

VOS_UINT32 atSetFCALPDDCSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    OS_MSG_STRU* pEvent = NULL;
    FTM_SET_F_FCALPDDCS_CNF_STRU *pFCalPDDCSCnf = NULL;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pFCalPDDCSCnf = (FTM_SET_F_FCALPDDCS_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pFCalPDDCSCnf->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;
}


VOS_UINT32 atQryFCALPDDCSPara(VOS_UINT8 ucClientId)
{
    FTM_RD_NOPRARA_REQ_STRU stCALPDDCSQryReq = {0};
    VOS_UINT32 ulRst;

    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_RD_FCALPDDCS_REQ, ucClientId,(VOS_VOID*)(&stCALPDDCSQryReq), sizeof(stCALPDDCSQryReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FCALPDDCS_RD;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 atQryFCALPDDCSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    FTM_RD_F_FCALPDDCS_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    VOS_UINT16 usLength = 0;
    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pstCnf = (FTM_RD_F_FCALPDDCS_CNF_STRU *)pEvent->ulParam1;

    if(NULL == pstCnf)
    {
    	return ERR_MSP_FAILURE;
    }

    if(ERR_MSP_SUCCESS != pstCnf->ulErrCode)
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);
    }
    else
    {
    	usLength = 0;
    	usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
    		"^FCALPDDCS: %u \r\n ^FCALPDDCS:DAC:%u \r\n ^FCALPDDCS:VGA:%u", pstCnf->usStatus, pstCnf->usDacCfg, pstCnf->usVgaCfg);

    	CmdErrProc(ucClientId, pstCnf->ulErrCode, usLength, pgucLAtSndCodeAddr);
    }
    return AT_FW_CLIENT_STATUS_READY;
}


// *****************************************************************************
// ��������:   ^FPDPOWS ����У׼����֧�֣�����PD���ʵ�ѹ����У׼���� ��������
//
// ����˵��:
//   ulIndex [in] ...ON / OFF
//                ...
//
// �� �� ֵ:
//    TODO: ...
// *****************************************************************************
VOS_UINT32 atSetFPDPOWSPara(VOS_UINT8 ucClientId)
{
    FTM_SET_F_FPDPOWS_REQ_STRU stFPDPOWS = {0};
    // �������
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    // ��������
    if(gucAtParaIndex > 3)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    // ���û��Ĭ�����õ�ģʽ������ʧ��
    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        stFPDPOWS.ulPdEnbFlg = gastAtParaList[0].ulParaValue;
		stFPDPOWS.usDacCfg = gastAtParaList[1].ulParaValue;
		stFPDPOWS.usVgaCfg = gastAtParaList[2].ulParaValue;
    }

    if(AT_SUCCESS == atSendFtmDataMsg(MSP_SYS_FTM_PID,ID_MSG_FTM_SET_FPDPOWS_REQ,ucClientId,
        (VOS_VOID*)(&stFPDPOWS), sizeof(FTM_SET_F_FPDPOWS_REQ_STRU)))
    {
        // ���õ�ǰ��������
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FPDPOWS_SET;

        return AT_WAIT_ASYNC_RETURN;    // ������������״̬
    }

    return AT_ERROR;

}

VOS_UINT32 atSetFPDPOWSParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    OS_MSG_STRU* pEvent = NULL;
    FTM_SET_F_FPDPOWS_CNF_STRU *pFPDPOWS = NULL;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
    pFPDPOWS = (FTM_SET_F_FPDPOWS_CNF_STRU *)pEvent->ulParam1;

    CmdErrProc(ucClientId, pFPDPOWS->ulErrCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;
}

// *****************************************************************************
// ��������:   ^FQPDDCRES ����У׼֧�֣���ѯPD��ѹ�빦������У׼�ĵ�ѹֵ
//
// ����˵��:
//   ulIndex [in] ...�ŵ���
//                ...
//
// �� �� ֵ:
//    TODO: ...
// *****************************************************************************
VOS_UINT32 atSetFQPDDCRESPara(VOS_UINT8 ucClientId)
{
    FTM_SET_FQPDDCRES_REQ_STRU stFQPDDCRESSetReq = {0};
    VOS_UINT32 ulRst;

    // 1��AT���������Ƿ���ȷ
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    // 2�����������Ƿ����Ҫ��
    if(1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    // 3��ÿ�������ĳ����Ƿ���ȷ���Ƿ�Ϊ0
    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    // 4��ÿ������ȡֵ��Լ��(ȡֵ��Χ��������������������)�Ƿ�����(ע:ҵ����ģ�鱣֤���˴���͸������)
    stFQPDDCRESSetReq.ulChannel = (VOS_UINT16)(gastAtParaList[0].ulParaValue);

    // 5��������Ϣ�������ģ��:͸�������û����ϸ�ʽ�����룬���ж�����ֵ����ȷ��
    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_F_SET_FQPDDCRES_REQ, ucClientId,(VOS_VOID*)(&stFQPDDCRESSetReq), sizeof(stFQPDDCRESSetReq));
    if(AT_SUCCESS == ulRst)
    {
        // ���õ�ǰ��������
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_F_FQPDDCRES_SET;
        return AT_WAIT_ASYNC_RETURN;    // ������������״̬
    }

    return AT_ERROR;
}

VOS_UINT32 atSetFQPDDCRESParaCnfProc(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
	OS_MSG_STRU* pEvent = NULL;
	FTM_SET_FQPDDCRES_CNF_STRU *pstCnf = NULL;
	VOS_UINT16 usLength = 0;
	VOS_UINT8 ucTmp = 0;

	pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pstCnf = (FTM_SET_FQPDDCRES_CNF_STRU *)pEvent->ulParam1;

	if(NULL == pstCnf)
	{
	    return ERR_MSP_FAILURE;
	}

	// ^FQPDDCRES:< rvalue_num >[,< rvalue_list_1 >]
	usLength = 0;

	for( ucTmp = 0 ; ucTmp < RSSI_POW_MAX_NUM ; ucTmp++)
	{
		usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,(VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
			" %u",pstCnf->ausPDDCValue[ucTmp]);
	}

	CmdErrProc(ucClientId, pstCnf->ulErrCode,usLength, pgucLAtSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;

}


/*lint -restore*/


