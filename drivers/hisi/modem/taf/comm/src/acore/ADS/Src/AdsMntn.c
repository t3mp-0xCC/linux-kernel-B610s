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

/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "AdsDebug.h"
#include "AdsMntn.h"



/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
/*lint -e767*/
#define    THIS_FILE_ID                 PS_FILE_ID_ADS_MNTN_C
/*lint +e767*/


/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/

spe_flow_stat_t                         g_stSpeFlowStat = {{0}};


/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


VOS_VOID ADS_MTNN_TransReport(
    VOS_UINT32                          ulMsgId,
    VOS_VOID                           *pData,
    VOS_UINT32                          ulLen
)
{
    DIAG_TRANS_IND_STRU                 stDiagTransInd;

    stDiagTransInd.ulModule  = ADS_MNTN_COMM_MOUDLE_ID;
    stDiagTransInd.ulPid     = ACPU_PID_ADS_UL;
    stDiagTransInd.ulMsgId   = ulMsgId;
    stDiagTransInd.ulReserve = 0;
    stDiagTransInd.ulLength  = ulLen;
    stDiagTransInd.pData     = pData;

    (VOS_VOID)DIAG_TransReport(&stDiagTransInd);
    return;
}


VOS_VOID ADS_MNTN_SndULIpfProcStatsInd(VOS_VOID)
{
    ADS_MNTN_UL_IPF_PROC_STATS_STRU     stStats;

    stStats.stCommHeader.usVer          = 100;
    stStats.stCommHeader.usReserved0    = 0;
    stStats.stCommHeader.usReserved1    = 0;
    stStats.stCommHeader.usReserved2    = 0;

    stStats.ulULQueNonEmptyTrigEvent    = g_stAdsStats.stUlComStatsInfo.ulULQueNonEmptyTrigEvent;
    stStats.ulULQueFullTrigEvent        = g_stAdsStats.stUlComStatsInfo.ulULQueFullTrigEvent;
    stStats.ulULQueHitThresTrigEvent    = g_stAdsStats.stUlComStatsInfo.ulULQueHitThresTrigEvent;
    stStats.ulULTmrHitThresTrigEvent    = g_stAdsStats.stUlComStatsInfo.ulULTmrHitThresTrigEvent;
    stStats.ulUL10MsTmrTrigEvent        = g_stAdsStats.stUlComStatsInfo.ulUL10MsTmrTrigEvent;
    stStats.ulULSpeIntTrigEvent         = g_stAdsStats.stUlComStatsInfo.ulULSpeIntTrigEvent;
    stStats.ulULProcEventNum            = g_stAdsStats.stUlComStatsInfo.ulULProcEventNum;

    stStats.ulULBdqCfgIpfHaveNoBd       = g_stAdsStats.stUlComStatsInfo.ulULBdqCfgIpfHaveNoBd;
    stStats.ulULBdqCfgBdSuccNum         = g_stAdsStats.stUlComStatsInfo.ulULBdqCfgBdSuccNum;
    stStats.ulULBdqCfgBdFailNum         = g_stAdsStats.stUlComStatsInfo.ulULBdqCfgBdFailNum;
    stStats.ulULBdqCfgIpfSuccNum        = g_stAdsStats.stUlComStatsInfo.ulULBdqCfgIpfSuccNum;
    stStats.ulULBdqCfgIpfFailNum        = g_stAdsStats.stUlComStatsInfo.ulULBdqCfgIpfFailNum;
    stStats.ulULBdqSaveSrcMemNum        = g_stAdsStats.stUlComStatsInfo.ulULBdqSaveSrcMemNum;
    stStats.ulULBdqFreeSrcMemNum        = g_stAdsStats.stUlComStatsInfo.ulULBdqFreeSrcMemNum;
    stStats.ulULBdqFreeSrcMemErr        = g_stAdsStats.stUlComStatsInfo.ulULBdqFreeSrcMemErr;

    stStats.ulULBuffThresholdCurrent    = g_stAdsCtx.stAdsIpfCtx.ulThredHoldNum;

    stStats.ulULBuffThreshold1          = g_stAdsCtx.stAdsIpfCtx.stUlAssemParmInfo.stThresholdLevel.ulThreshold1;
    stStats.ulULBuffThreshold2          = g_stAdsCtx.stAdsIpfCtx.stUlAssemParmInfo.stThresholdLevel.ulThreshold2;
    stStats.ulULBuffThreshold3          = g_stAdsCtx.stAdsIpfCtx.stUlAssemParmInfo.stThresholdLevel.ulThreshold3;
    stStats.ulULBuffThreshold4          = g_stAdsCtx.stAdsIpfCtx.stUlAssemParmInfo.stThresholdLevel.ulThreshold4;

    stStats.ulULWmLevel1HitNum          = g_stAdsStats.stUlComStatsInfo.ulULWmLevel1HitNum;
    stStats.ulULWmLevel2HitNum          = g_stAdsStats.stUlComStatsInfo.ulULWmLevel2HitNum;
    stStats.ulULWmLevel3HitNum          = g_stAdsStats.stUlComStatsInfo.ulULWmLevel3HitNum;
    stStats.ulULWmLevel4HitNum          = g_stAdsStats.stUlComStatsInfo.ulULWmLevel4HitNum;

    ADS_MTNN_TransReport(ID_DIAG_ADS_UL_IPF_PROC_STATS_IND,
                        (VOS_VOID *)&stStats,
                        sizeof(ADS_MNTN_UL_IPF_PROC_STATS_STRU));

    return;
}


VOS_VOID ADS_MNTN_SndDLIpfProcStatsInd(VOS_VOID)
{
    ADS_MNTN_DL_IPF_PROC_STATS_STRU     stStats;

    stStats.stCommHeader.usVer          = 100;
    stStats.stCommHeader.usReserved0    = 0;
    stStats.stCommHeader.usReserved1    = 0;
    stStats.stCommHeader.usReserved2    = 0;

    stStats.ulDLRcvIpfRdIntNum          = g_stAdsStats.stDlComStatsInfo.ulDLRcvIpfRdIntNum;
    stStats.ulDLProcIpfRdEventNum       = g_stAdsStats.stDlComStatsInfo.ulDLProcIpfRdEventNum;
    stStats.ulDLRcvIpfAdqEmptyIntNum    = g_stAdsStats.stDlComStatsInfo.ulDLRcvIpfAdqEmptyIntNum;
    stStats.ulDLProcIpfAdEventNum       = g_stAdsStats.stDlComStatsInfo.ulDLProcIpfAdEventNum;
    stStats.ulDLCCoreResetTrigEvent     = g_stAdsStats.stDlComStatsInfo.ulDLCCoreResetTrigEvent;

    stStats.ulDLRdqRxRdNum              = g_stAdsStats.stDlComStatsInfo.ulDLRdqRxRdNum;
    stStats.ulDLRdqGetRd0Num            = g_stAdsStats.stDlComStatsInfo.ulDLRdqGetRd0Num;
    stStats.ulDLRdqTransMemFailNum      = g_stAdsStats.stDlComStatsInfo.ulDLRdqTransMemFailNum;
    stStats.ulDLRdqRxNormPktNum         = g_stAdsStats.stDlComStatsInfo.ulDLRdqRxNormPktNum;
    stStats.ulDLRdqRxNdPktNum           = g_stAdsStats.stDlComStatsInfo.ulDLRdqRxNdPktNum;
    stStats.ulDLRdqRxDhcpPktNum         = g_stAdsStats.stDlComStatsInfo.ulDLRdqRxDhcpPktNum;
    stStats.ulDLRdqRxErrPktNum          = g_stAdsStats.stDlComStatsInfo.ulDLRdqRxErrPktNum;
    stStats.ulDLRdqFilterErrNum         = g_stAdsStats.stDlComStatsInfo.ulDLRdqFilterErrNum;

    stStats.ulDLAdqAllocSysMemSuccNum   = g_stAdsStats.stDlComStatsInfo.ulDLAdqAllocSysMemSuccNum;
    stStats.ulDLAdqAllocSysMemFailNum   = g_stAdsStats.stDlComStatsInfo.ulDLAdqAllocSysMemFailNum;
    stStats.ulDLAdqAllocMemSuccNum      = g_stAdsStats.stDlComStatsInfo.ulDLAdqAllocMemSuccNum;
    stStats.ulDLAdqAllocMemFailNum      = g_stAdsStats.stDlComStatsInfo.ulDLAdqAllocMemFailNum;
    stStats.ulDLAdqFreeMemNum           = g_stAdsStats.stDlComStatsInfo.ulDLAdqFreeMemNum;
    stStats.ulDLAdqRecycleMemSuccNum    = g_stAdsStats.stDlComStatsInfo.ulDLAdqRecycleMemSuccNum;
    stStats.ulDLAdqRecycleMemFailNum    = g_stAdsStats.stDlComStatsInfo.ulDLAdqRecycleMemFailNum;
    stStats.ulDLAdqGetFreeAdSuccNum     = g_stAdsStats.stDlComStatsInfo.ulDLAdqGetFreeAdSuccNum;
    stStats.ulDLAdqGetFreeAdFailNum     = g_stAdsStats.stDlComStatsInfo.ulDLAdqGetFreeAdFailNum;
    stStats.ulDLAdqCfgAdNum             = g_stAdsStats.stDlComStatsInfo.ulDLAdqCfgAdNum;
    stStats.ulDLAdqCfgAd0Num            = g_stAdsStats.stDlComStatsInfo.ulDLAdqCfgAd0Num;
    stStats.ulDLAdqCfgAd1Num            = g_stAdsStats.stDlComStatsInfo.ulDLAdqCfgAd1Num;
    stStats.ulDLAdqCfgIpfSuccNum        = g_stAdsStats.stDlComStatsInfo.ulDLAdqCfgIpfSuccNum;
    stStats.ulDLAdqCfgIpfFailNum        = g_stAdsStats.stDlComStatsInfo.ulDLAdqCfgIpfFailNum;
    stStats.ulDLAdqStartEmptyTmrNum     = g_stAdsStats.stDlComStatsInfo.ulDLAdqStartEmptyTmrNum;
    stStats.ulDLAdqEmptyTmrTimeoutNum   = g_stAdsStats.stDlComStatsInfo.ulDLAdqEmptyTmrTimeoutNum;
    stStats.ulDLAdqRcvAd0EmptyIntNum    = g_stAdsStats.stDlComStatsInfo.ulDLAdqRcvAd0EmptyIntNum;
    stStats.ulDLAdqRcvAd1EmptyIntNum    = g_stAdsStats.stDlComStatsInfo.ulDLAdqRcvAd1EmptyIntNum;

    ADS_MTNN_TransReport(ID_DIAG_ADS_DL_IPF_PROC_STATS_IND,
                        (VOS_VOID *)&stStats,
                        sizeof(ADS_MNTN_DL_IPF_PROC_STATS_STRU));

    return;
}


VOS_VOID ADS_MNTN_SndULPktProcStatsInd(VOS_VOID)
{
    ADS_MNTN_UL_PKT_PROC_STATS_STRU     stStats;

    stStats.stCommHeader.usVer          = 100;
    stStats.stCommHeader.usReserved0    = 0;
    stStats.stCommHeader.usReserved1    = 0;
    stStats.stCommHeader.usReserved2    = 0;

    stStats.ulULRmnetRxPktNum           = g_stAdsStats.stUlComStatsInfo.ulULRmnetRxPktNum;
    stStats.ulULRmnetModemIdErrNum      = g_stAdsStats.stUlComStatsInfo.ulULRmnetModemIdErrNum;
    stStats.ulULRmnetRabIdErrNum        = g_stAdsStats.stUlComStatsInfo.ulULRmnetRabIdErrNum;
    stStats.ulULRmnetEnQueSuccNum       = g_stAdsStats.stUlComStatsInfo.ulULRmnetEnQueSuccNum;
    stStats.ulULRmnetEnQueFailNum       = g_stAdsStats.stUlComStatsInfo.ulULRmnetEnQueFailNum;
    stStats.ulULPktEnQueSuccNum         = g_stAdsStats.stUlComStatsInfo.ulULPktEnQueSuccNum;
    stStats.ulULPktEnQueFailNum         = g_stAdsStats.stUlComStatsInfo.ulULPktEnQueFailNum;

    ADS_MTNN_TransReport(ID_DIAG_ADS_UL_PKT_PROC_STATS_IND,
                         (VOS_VOID *)&stStats,
                         sizeof(ADS_MNTN_UL_PKT_PROC_STATS_STRU));

    return;
}


VOS_VOID ADS_MNTN_SndDLPktProcStatsInd(VOS_VOID)
{
    ADS_MNTN_DL_PKT_PROC_STATS_STRU     stStats;

    stStats.stCommHeader.usVer          = 100;
    stStats.stCommHeader.usReserved0    = 0;
    stStats.stCommHeader.usReserved1    = 0;
    stStats.stCommHeader.usReserved2    = 0;

    stStats.ulDLRmnetTxPktNum           = g_stAdsStats.stDlComStatsInfo.ulDLRmnetTxPktNum;
    stStats.ulDLRmnetNoFuncFreePktNum   = g_stAdsStats.stDlComStatsInfo.ulDLRmnetNoFuncFreePktNum;
    stStats.ulDLRmnetRabIdErrNum        = g_stAdsStats.stDlComStatsInfo.ulDLRmnetRabIdErrNum;

    ADS_MTNN_TransReport(ID_DIAG_ADS_DL_PKT_PROC_STATS_IND,
                        (VOS_VOID *)&stStats,
                        sizeof(ADS_MNTN_DL_PKT_PROC_STATS_STRU));

    return;
}


VOS_VOID ADS_MNTN_SndThroughputStatsInd(VOS_VOID)
{
    ADS_MNTN_THROUGHPUT_STATS_STRU      stStats;

    stStats.stCommHeader.usVer          = 100;
    stStats.stCommHeader.usReserved0    = 0;
    stStats.stCommHeader.usReserved1    = 0;
    stStats.stCommHeader.usReserved2    = 0;

    stStats.ulULDataRate                = g_stAdsCtx.stDsFlowStatsCtx.stULDataStats.ulULCurDataRate << 3;
    stStats.ulDLDataRate                = g_stAdsCtx.stDsFlowStatsCtx.stDLDataStats.ulDLCurDataRate << 3;

    ADS_MTNN_TransReport(ID_DIAG_ADS_THROUGHPUT_STATS_IND,
                        (VOS_VOID *)&stStats,
                        sizeof(ADS_MNTN_THROUGHPUT_STATS_STRU));

    return;
}


VOS_VOID ADS_MNTN_HookIpPkt(
    IMM_ZC_STRU                        *pstImmZc,
    VOS_UINT16                          usMsgType,
    VOS_UINT32                          ulParam1,
    VOS_UINT32                          ulParam2,
    VOS_UINT32                          ulParam3,
    VOS_UINT32                          ulParam4
)
{
    VOS_UINT32                          ulDataLen;
    VOS_UINT32                          ulCacheLen;

    if (VOS_FALSE == IPS_MNTN_GetIPInfoCfg(usMsgType))
    {
        return;
    }

    ulDataLen  = IMM_ZcGetUsedLen(pstImmZc);
    ulCacheLen = (ulDataLen < ADS_MNTN_HOOK_PKT_MAX_SIZE) ?
                    (IMM_MAC_HEADER_RES_LEN + ulDataLen) :
                    (IMM_MAC_HEADER_RES_LEN + ADS_MNTN_HOOK_PKT_MAX_SIZE);

    ADS_IPF_SPE_MEM_UNMAP(pstImmZc, ulCacheLen);

    IPS_MNTN_TraceIpInfo(pstImmZc, usMsgType,
                         ulParam1, ulParam2, ulParam3, ulParam4);

    ADS_IPF_SPE_MEM_MAP(pstImmZc, ulCacheLen);
    return;
}


VOS_VOID ADS_MNTN_ReportAllStatsInfo(VOS_VOID)
{
    ADS_MNTN_SndULIpfProcStatsInd();
    ADS_MNTN_SndDLIpfProcStatsInd();
    ADS_MNTN_SndULPktProcStatsInd();
    ADS_MNTN_SndDLPktProcStatsInd();
    ADS_MNTN_SndThroughputStatsInd();

    return;
}


VOS_VOID SPE_MNTN_SndPortStatsInd(VOS_VOID)
{
    spe_flow_stat_t                    *pstSpeFlowStat = VOS_NULL_PTR;
    SPE_MNTN_PORT_PROC_STATS_STRU       stStats = {{0}};
    VOS_UINT8                           ucPort;

    pstSpeFlowStat = &g_stSpeFlowStat;
    if (MDRV_OK != mdrv_spe_wport_ioctl(ADS_GET_IPF_SPE_WPORT(),
                                        SPE_WPORT_IOCTL_DIAG_UPLOAD, pstSpeFlowStat))
    {
        return;
    }

    for (ucPort = 0; ucPort < SPE_PORT_NUM; ucPort++)
    {
        if (0 == pstSpeFlowStat->port_enable[ucPort])
        {
            continue;
        }

        stStats.stCommHeader.usVer       = 100;
        stStats.stCommHeader.usReserved0 = 0;
        stStats.stCommHeader.usReserved1 = 0;
        stStats.stCommHeader.usReserved2 = 0;

        stStats.ulTdInputRate           = pstSpeFlowStat->td_input_bps[ucPort];
        stStats.ulRdOutputRate          = pstSpeFlowStat->rd_output_bps[ucPort];

        stStats.ulTdPtrA                = pstSpeFlowStat->td_ptr_a[ucPort];
        stStats.ulTdUsingNum            = pstSpeFlowStat->td_using_num[ucPort];
        stStats.ulTdFullH               = pstSpeFlowStat->td_ful_h[ucPort];
        stStats.ulTdEmptyH              = pstSpeFlowStat->td_empty_h[ucPort];
        stStats.ulTdFullS               = pstSpeFlowStat->td_ful_s[ucPort];

        stStats.ulRdPtrA                = pstSpeFlowStat->rd_ptr_a[ucPort];
        stStats.ulRdUsingNum            = pstSpeFlowStat->rd_using_num[ucPort];
        stStats.ulRdFullH               = pstSpeFlowStat->rd_ful_h[ucPort];
        stStats.ulRdEmptyH              = pstSpeFlowStat->rd_empty_h[ucPort];
        stStats.ulRdFullS               = pstSpeFlowStat->rd_ful_s[ucPort];

        ADS_MTNN_TransReport((ID_DIAG_SPE_PORT_0_STATS_IND + ucPort),
                             (VOS_VOID *)&stStats,
                             sizeof(SPE_MNTN_PORT_PROC_STATS_STRU));
    }

    return;
}


VOS_VOID SPE_MNTN_ReportAllStatsInfo(VOS_VOID)
{
    SPE_MNTN_SndPortStatsInd();

    return;
}



