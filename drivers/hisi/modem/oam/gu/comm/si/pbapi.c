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




#include "TafTypeDef.h"
#include "si_pb.h"
#include "product_config.h"

/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
/*lint -e767*/
#define      THIS_FILE_ID     PS_FILE_ID_PBAPI_C
/*lint +e767*/



VOS_UINT32 SI_PB_GetReceiverPid(MN_CLIENT_ID_T  ClientId, VOS_UINT32 *pulReceiverPid)
{
    *pulReceiverPid = MAPS_PB_PID;

    return VOS_OK;
}


SI_UINT32 SI_PB_Read(  MN_CLIENT_ID_T           ClientId,
                            MN_OPERATION_ID_T        OpId,
                            SI_PB_STORATE_TYPE       Storage,
                            SI_UINT16                Index1,
                            SI_UINT16                Index2)
{
    SI_PB_READ_REQ_STRU     *pMsg;

    pMsg = (SI_PB_READ_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_READ_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Read:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ACPU_PID_PB;

    pMsg->ulMsgName     = SI_PB_READ_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->usIndex1      = Index1;
    pMsg->usIndex2      = Index2;

    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Read:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}


SI_UINT32 SI_PB_SRead(  MN_CLIENT_ID_T           ClientId,
                            MN_OPERATION_ID_T        OpId,
                            SI_PB_STORATE_TYPE       Storage,
                            SI_UINT16                Index1,
                            SI_UINT16                Index2)
{
    SI_PB_READ_REQ_STRU     *pMsg;

    pMsg = (SI_PB_READ_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_READ_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Read:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ACPU_PID_PB;

    pMsg->ulMsgName     = SI_PB_SREAD_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->usIndex1      = Index1;
    pMsg->usIndex2      = Index2;

    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Read:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}


SI_UINT32 SI_PB_Query(     MN_CLIENT_ID_T           ClientId,
                                MN_OPERATION_ID_T        OpId)
{
    SI_PB_QUERY_REQ_STRU    *pMsg;

    pMsg = (SI_PB_QUERY_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_QUERY_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Query:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = MAPS_PB_PID;
    pMsg->ulMsgName     = SI_PB_QUERY_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->ulStorage     = gstPBCtrlInfo.enPBCurType;

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Query:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}


SI_UINT32 SI_PB_Set(    MN_CLIENT_ID_T           ClientId,
                            MN_OPERATION_ID_T        OpId,
                            SI_PB_STORATE_TYPE      Storage)
{
    SI_PB_SET_REQ_STRU  *pMsg;
    VOS_UINT32          ulReceiverPid;


    if (VOS_OK != SI_PB_GetReceiverPid(ClientId, &ulReceiverPid))
    {
        PB_ERROR_LOG("SI_PB_Set:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    pMsg = (SI_PB_SET_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_SET_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Set:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ulReceiverPid;
    pMsg->ulMsgName     = SI_PB_SET_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->ulStorage     = Storage;

    if(VOS_OK != VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Set:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}


SI_UINT32 SI_PB_Add(    MN_CLIENT_ID_T          ClientId,
                            MN_OPERATION_ID_T        OpId,
                             SI_PB_STORATE_TYPE      Storage,
                             SI_PB_RECORD_STRU       *pRecord)
{
    SI_PB_ADD_REP_STRU  *pMsg;
    VOS_UINT32          ulReceiverPid;

    if (VOS_OK != SI_PB_GetReceiverPid(ClientId, &ulReceiverPid))
    {
        PB_ERROR_LOG("SI_PB_Add:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    if(VOS_NULL_PTR == pRecord)
    {
        PB_ERROR_LOG("SI_PB_Add:pRecord is a NULL pointer");

        return TAF_FAILURE;
    }

    pMsg = (SI_PB_ADD_REP_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_ADD_REP_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Add:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ulReceiverPid;
    pMsg->ulMsgName     = SI_PB_ADD_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;

    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }

    pRecord->Index = 1;

    /*lint -e534*/
    VOS_MemCpy(&pMsg->stRecord, pRecord, sizeof(SI_PB_RECORD_STRU));
    /*lint +e534*/

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Add:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

SI_UINT32 SI_PB_SAdd(    MN_CLIENT_ID_T          ClientId,
                            MN_OPERATION_ID_T        OpId,
                             SI_PB_STORATE_TYPE      Storage,
                             SI_PB_RECORD_STRU       *pRecord)
{
    SI_PB_ADD_REP_STRU  *pMsg;

    if(VOS_NULL_PTR == pRecord)
    {
        PB_ERROR_LOG("SI_PB_Add:pRecord is a NULL pointer");

        return TAF_FAILURE;
    }

    pMsg = (SI_PB_ADD_REP_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_ADD_REP_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Add:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = MAPS_PB_PID;
    pMsg->ulMsgName     = SI_PB_SADD_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;

    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }

    pRecord->Index = 1;

    /*lint -e534*/
    VOS_MemCpy(&pMsg->stRecord, pRecord, sizeof(SI_PB_RECORD_STRU));
    /*lint +e534*/

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Add:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}


SI_UINT32 SI_PB_Modify(    MN_CLIENT_ID_T          ClientId,
                                MN_OPERATION_ID_T       OpId,
                                SI_PB_STORATE_TYPE      Storage,
                                SI_PB_RECORD_STRU       *pRecord )
{
    SI_PB_MODIFY_REP_STRU  *pMsg;
    VOS_UINT32              ulReceiverPid;

    if (VOS_OK != SI_PB_GetReceiverPid(ClientId, &ulReceiverPid))
    {
        PB_ERROR_LOG("SI_PB_Modify:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    if(VOS_NULL_PTR == pRecord)
    {
        PB_ERROR_LOG("SI_PB_Modify:pRecord is a NULL pointer");

        return TAF_FAILURE;
    }

    pMsg = (SI_PB_MODIFY_REP_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_MODIFY_REP_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Modify:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ulReceiverPid;
    pMsg->ulMsgName     = SI_PB_MODIFY_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;

    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }

    /*lint -e534*/
    VOS_MemCpy(&pMsg->Record, pRecord, sizeof(SI_PB_RECORD_STRU));
    /*lint +e534*/

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Modify:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}


SI_UINT32 SI_PB_SModify(    MN_CLIENT_ID_T          ClientId,
                                MN_OPERATION_ID_T       OpId,
                                SI_PB_STORATE_TYPE      Storage,
                                SI_PB_RECORD_STRU       *pRecord )
{
    SI_PB_MODIFY_REP_STRU  *pMsg;

    if(VOS_NULL_PTR == pRecord)
    {
        PB_ERROR_LOG("SI_PB_Modify:pRecord is a NULL pointer");

        return TAF_FAILURE;
    }

    pMsg = (SI_PB_MODIFY_REP_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_MODIFY_REP_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Modify:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = MAPS_PB_PID;
    pMsg->ulMsgName     = SI_PB_SMODIFY_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;

    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }

    /*lint -e534*/
    VOS_MemCpy(&pMsg->Record, pRecord, sizeof(SI_PB_RECORD_STRU));
    /*lint +e534*/

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Modify:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}


SI_UINT32 SI_PB_Delete(     MN_CLIENT_ID_T             ClientId,
                                MN_OPERATION_ID_T           OpId,
                                SI_PB_STORATE_TYPE          Storage,
                                SI_UINT16                   Index)
{
    SI_PB_DELETE_REQ_STRU  *pMsg;
    VOS_UINT32              ulReceiverPid;

    if (VOS_OK != SI_PB_GetReceiverPid(ClientId, &ulReceiverPid))
    {
        PB_ERROR_LOG("SI_PB_Modify:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    pMsg = (SI_PB_DELETE_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_DELETE_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Delete:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ulReceiverPid;
    pMsg->ulMsgName     = SI_PB_DELETE_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->usIndex       = Index;

    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Delete:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}


SI_UINT32 SI_PB_Search(    MN_CLIENT_ID_T             ClientId,
                                MN_OPERATION_ID_T           OpId,
                                SI_PB_STORATE_TYPE          Storage,
                                SI_UINT8                    ucLength,
                                SI_UINT8                    *pucContent)
{
    SI_PB_SEARCH_REQ_STRU  *pMsg;

    pMsg = (SI_PB_SEARCH_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_SEARCH_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Search:VOS_AllocMsg Failed");
        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ACPU_PID_PB;

    pMsg->ulMsgName     = SI_PB_SEARCH_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->ucLength      = ucLength;

    pMsg->ulStorage = ((Storage == SI_PB_STORAGE_UNSPECIFIED)?(gstPBCtrlInfo.enPBCurType):(Storage));

    /*lint -e534*/
    VOS_MemCpy(pMsg->aucContent, pucContent, ucLength);
    /*lint +e534*/

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Search:VOS_SendMsg Failed");
        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}


VOS_UINT32 SI_PB_GetStorateType(VOS_VOID)
{
    return (VOS_UINT32)gstPBCtrlInfo.enPBCurType;
}


VOS_UINT32 SI_PB_GetSPBFlag(VOS_VOID)
{
    return (VOS_UINT32)gstPBConfigInfo.ucSPBFlag;
}







