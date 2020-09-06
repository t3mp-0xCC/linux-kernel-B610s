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




#include "siappstk.h"
#include "si_stk.h"
#include "product_config.h"
#include "product_nv_id.h"
#include "product_nv_def.h"
#include "MbbSiStkBip.h"




/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
/*lint -e767*/
#define    THIS_FILE_ID PS_FILE_ID_STK_API_C
/*lint +e767*/


VOS_UINT32 SI_STK_SendReqMsg(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId, 
    VOS_UINT32                          MsgName,
    VOS_UINT32                          CmdType, 
    VOS_UINT32                          DataLen, 
    VOS_UINT8                           *pData)
{
    SI_STK_REQ_STRU         *pstSTKReq;
    VOS_UINT32              ulSendPid;
    VOS_UINT32              ulReceiverPid;

    ulSendPid = WUEPS_PID_AT;

    ulReceiverPid = MAPS_STK_PID;


    pstSTKReq = (SI_STK_REQ_STRU *)VOS_AllocMsg(ulSendPid, sizeof(SI_STK_REQ_STRU)-VOS_MSG_HEAD_LENGTH+DataLen);

    if(VOS_NULL_PTR == pstSTKReq)
    {
        STK_ERROR_LOG("SI_STK_SendReqMsg: VOS_AllocMsg Return Error");
        return VOS_ERR;
    }

    pstSTKReq->MsgName       = MsgName;
    pstSTKReq->ulReceiverPid = ulReceiverPid;
    pstSTKReq->OpId          = OpId;
    pstSTKReq->ClientId      = ClientId;
    pstSTKReq->SatType       = CmdType;
    pstSTKReq->Datalen       = DataLen;

    if(DataLen != 0)
    {
        /*lint -e534*/
        VOS_MemCpy(pstSTKReq->Data, pData, DataLen);
        /*lint +e534*/
    }

    if(VOS_OK != VOS_SendMsg(ulSendPid, pstSTKReq))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 SI_STK_GetBipNV(NV_BIP_FEATURE_STRU *pstBipFeatureNV)
{
    NV_BIP_FEATURE_STRU stBipFeatureNV;

    PS_MEM_SET((VOS_VOID*)&stBipFeatureNV, 0x00, sizeof(stBipFeatureNV));
    
    if ( NV_OK != NV_Read(en_NV_Item_BIP_FEATURE, &stBipFeatureNV, sizeof(stBipFeatureNV)) )
    {
        STK_WARNING_LOG("STK_InitGobal: Read en_NV_Item_BIP_FEATURE Fail");
        return VOS_FALSE;
    }

    if ( NULL != pstBipFeatureNV )
    {
        PS_MEM_CPY(pstBipFeatureNV, &stBipFeatureNV, sizeof(NV_BIP_FEATURE_STRU));
        return VOS_TRUE;
    }

    return VOS_FALSE;
}


static SI_UINT16 SI_STK_DataSendBipEventEx(SI_UINT32 ulMsgId, SI_UINT16 DataLen, SI_UINT8 *pData)
{
    SI_STK_REQ_STRU         *pstSTKReq;
    SI_UINT32               ulSendPid;
    SI_UINT32               ulReceiverPid;
    
    ulSendPid = WUEPS_PID_AT;
    ulReceiverPid = MAPS_STK_PID;
    pstSTKReq = (SI_STK_REQ_STRU *)VOS_AllocMsg(ulSendPid, (sizeof(SI_STK_REQ_STRU) - VOS_MSG_HEAD_LENGTH + DataLen));
    
    if (VOS_NULL_PTR == pstSTKReq)
    {
        STK_ERROR_LOG("SI_STK_SendReqMsg: VOS_AllocMsg Return Error");
        return VOS_ERR;
    }

    pstSTKReq->MsgName       = ulMsgId;

    pstSTKReq->ulReceiverPid = ulReceiverPid;
    pstSTKReq->Datalen       = DataLen;

    if (DataLen != 0)
    {
        VOS_MemCpy(pstSTKReq->Data, pData, DataLen);  /*lint !e419 */
    }

    if (VOS_OK != VOS_SendMsg(ulSendPid, pstSTKReq))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT16 STUB_USIMM_SatDataInd(VOS_UINT8   ucCmdType,
                                    VOS_UINT16 usDataLen,
                                    VOS_UINT8 *pucData)
{
    USIMM_STKDATA_IND_STRU    *pUsimMsg;
    NV_BIP_FEATURE_STRU     stBipFeatureNV;
    VOS_UINT32              ulRet;

    VOS_MemSet((VOS_VOID*)&stBipFeatureNV, 0x00, sizeof(NV_BIP_FEATURE_STRU));
    ulRet = SI_STK_GetBipNV(&stBipFeatureNV);

    if (VOS_TRUE != ulRet)
    {
        return VOS_ERR;
    }

    /*检查BIP NV确认是否可以进行AT模拟测试*/
    if ( !BIP_TEST_SUPPORT(stBipFeatureNV) )
    {
        return VOS_ERR;
    }
    
    pUsimMsg = (USIMM_STKDATA_IND_STRU*)VOS_AllocMsg(WUEPS_PID_USIM, sizeof(USIMM_STKDATA_IND_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pUsimMsg)
    {
        return VOS_ERR;
    }

    usDataLen = (usDataLen < USIMM_T0_APDU_MAX_LEN) ? usDataLen : USIMM_T0_APDU_MAX_LEN;
    pUsimMsg->stIndHdr.ulReceiverPid = MAPS_STK_PID;
    pUsimMsg->stIndHdr.enMsgName     = USIMM_STKDATA_IND;
    pUsimMsg->usLen         = usDataLen;
    pUsimMsg->ucCmdType     = ucCmdType;
    
    VOS_MemCpy(pUsimMsg->aucContent, pucData, usDataLen);

    if (VOS_OK != VOS_SendMsg(pUsimMsg->stIndHdr.ulSenderPid, pUsimMsg))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


SI_UINT16 SI_STK_DataSendBipEvent(SI_UINT16 DataLen, SI_UINT8 *pData)
{
    SI_UINT32 ulMsgId = 0;
    
    ulMsgId       = SI_STK_BIP_EVENT;

    return SI_STK_DataSendBipEventEx(ulMsgId, DataLen, pData);
}


SI_UINT16 SI_STK_DataSendBipTestEvent(SI_UINT16 DataLen, SI_UINT8 *pData)
{
    return SI_STK_DataSendBipEventEx(STK_AT_BIP_AT_TEST_EVENT, DataLen, pData);
}


VOS_UINT32 SI_STK_GetMainMenu(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId)
{
    return SI_STK_SendReqMsg(ClientId,OpId, SI_STK_GETMAINMNUE, SI_STK_NOCMDDATA,0,VOS_NULL_PTR);
}


VOS_UINT32 SI_STK_GetSTKCommand(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_STK_CMD_TYPE                     CmdType)
{
    return SI_STK_SendReqMsg(ClientId,OpId, SI_STK_GETCOMMAND, CmdType,0,VOS_NULL_PTR);
}


VOS_UINT32 SI_STK_QuerySTKCommand(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId)
{
    return SI_STK_SendReqMsg(ClientId,OpId, SI_STK_QUERYCOMMAND, SI_STK_NOCMDDATA,0,VOS_NULL_PTR);
}


VOS_UINT32 SI_STK_DataSendSimple(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_SEND_DATA_TYPE                   SendType,
    VOS_UINT32                          DataLen,
    VOS_UINT8                           *pData)
{
    return SI_STK_SendReqMsg(ClientId,OpId, SI_STK_SIMPLEDOWN,SendType,DataLen,pData);
}


VOS_UINT32 SI_STK_TerminalResponse(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_STK_TERMINAL_RSP_STRU            *pstTRStru)
{

    if(pstTRStru == VOS_NULL_PTR)
    {
        STK_ERROR_LOG("SI_STK_TerminalResponse: The input parameter is null.");

        return VOS_ERR;
    }

    return SI_STK_SendReqMsg(ClientId,OpId, SI_STK_TRDOWN, SI_STK_NOCMDDATA,sizeof(SI_STK_TERMINAL_RSP_STRU),(VOS_UINT8*)pstTRStru);
}



VOS_UINT32 SI_STKDualIMSIChangeReq(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId)
{
    SI_STK_REQ_STRU *pstSTKReq;

    pstSTKReq = (SI_STK_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_STK_REQ_STRU)-VOS_MSG_HEAD_LENGTH);

    if(VOS_NULL_PTR ==pstSTKReq)
    {
        STK_ERROR_LOG("SI_STKDualIMSIChangeReq: VOS_AllocMsg Return Error");
        return VOS_ERR;
    }

    pstSTKReq->ClientId      = ClientId;
    pstSTKReq->OpId          = OpId;
    pstSTKReq->MsgName       = SI_STK_IMSICHG;
    pstSTKReq->ulReceiverPid = MAPS_STK_PID;

    if(VOS_OK != VOS_SendMsg(MAPS_STK_PID, pstSTKReq))
    {
        STK_ERROR_LOG("SI_STKDualIMSIChangeReq: VOS_SendMsg Return Error");
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 SI_STKIsDualImsiSupport(VOS_VOID)
{
    VOS_UINT16 usDualIMSIFlag = 0;

    if ( NV_OK != NV_Read(en_NV_Item_NV_HUAWEI_DOUBLE_IMSI_CFG_I, &usDualIMSIFlag, sizeof(VOS_UINT16)) )
    {
        STK_WARNING_LOG("STK_InitGobal: Read en_NV_Item_NV_HUAWEI_DOUBLE_IMSI_CFG_I Fail");
    }

    /* 前后两个自节均为1，Dual IMSI功能才开启，第一个字节为NV激活标志，第二个为使能位 */
    if ( STK_NV_ENABLED == usDualIMSIFlag )
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}



VOS_UINT32 SI_STK_MenuSelection(
    MN_CLIENT_ID_T                      ClientId,
    MN_OPERATION_ID_T                   OpId,
    SI_STK_ENVELOPE_STRU                *pstENStru)
{
    if((VOS_NULL_PTR == pstENStru)||(SI_STK_ENVELOPE_MENUSEL != pstENStru->enEnvelopeType))
    {
        STK_ERROR_LOG("SI_STK_MenuSelection: The Input Data is Error");

        return VOS_ERR;
    }

    return SI_STK_SendReqMsg(ClientId,OpId, SI_STK_MENUSELECTION, pstENStru->enEnvelopeType,sizeof(SI_STK_ENVELOPE_STRU),(SI_UINT8*)pstENStru);
}


VOS_UINT32 SI_STK_SetUpCallConfirm(
    MN_CLIENT_ID_T                      ClientId, 
    SI_STK_SETUPCALLCONFIRM_ENUM_UINT32 enAction)
{
    /* 参数检查 */
    if (SI_STK_SETUPCALL_BUTT <= enAction)
    {
        STK_ERROR_LOG("SI_STK_SetUpCallConfirm: The Input Para is Error");

        return VOS_ERR;
    }

    return SI_STK_SendReqMsg(ClientId, 0, SI_STK_SETUPCALL_CONFIRM, SI_STK_SETUPCALL, sizeof(VOS_UINT32), (VOS_UINT8*)&enAction);
}





