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




#include "diag_common.h"

#define     DIAG_CTRL_NEED_DATALOCK    0x01


typedef struct {
    DIAG_MESSAGE_TYPE_U32   ulMsgType;
    VOS_UINT32                      ulCmdListCount;
    VOS_UINT32                      *szulAllowCmdList;
    VOS_UINT32                      ulCtrl;
}DIAG_OM_WHITELIST_ST;

VOS_UINT32   gulMspAllowCmdList[] = {
    0x10015310,    /* DIAG_CMD_LOG_CAT_PRINT  0x10015310 */
    0x10015311,    /* DIAG_CMD_LOG_CAT_LAYER  0x10015311 */
    0x10015312,    /* DIAG_CMD_LOG_CAT_AIR    0x10015312 */
    0x10015315,    /* DIAG_CMD_LOG_CAT_EVENT  0x10015315 */
    0x10065000,    /* DIAG_CMD_HOST_CONNECT   0x10065000 */
    0x10065001,    /* DIAG_CMD_HOST_DISCONNECT  0x10065001 */
    0x10014001,    /* DIAG_CMD_GET_TIMESTAMP_VALUE  0x10014001 */
    0x10014002,    /* DIAG_CMD_GET_MODEM_NUM    0x10014002 */
};

VOS_UINT32   gulPsAllowCmdList[] = {
    0x2038107c,    /* OM_L2_THROUGHPUT_REQ   0x2038107c  -- */
    0x2200A001,    /* 8.1.1 OM_GAS_CELL_INFO_REQ  0x2200A001  */
    0x2200A002,    /* 8.1.2 OM_GAS_CHANNEL_QUALITY_REQ  0x2200A002  */
    0x2200a003,    /* 8.1.3 OM_GAS_3G_NCELL_MEAS_REQ  0x2200a003  */
    0x2200a004,    /* 8.1.4 OM_GAS_GSM_INFO_REQ  0x2200a004  */
    0x2200a005,    /* 8.1.5 OM_GAS_OTA_REQ  0x2200a005  */
    0x2200a0d1,    /* 8.1.6 OM_GAS_LTE_NCELL_MEAS_REPORT_ REQ  0x2200a0d1  */
    0x2200aaaa,    /* 8.1.7 OM_SDT_GAS_CONNECTED_ IND  0x2200aaaa  */
    0x2200C101,    /* 8.1.8 OM_NAS_OTA_REQ  0x2200C101  */
    0x2300aaaa,    /* 8.2.1 OM_WAS_CONNECTED_IND  0x2300aaaa  */
    0x2300B000,    /* 8.2.2 OM_WAS_PRINT_LEVEL_CTRL_REQ  0x2300B000  */

    0x2300B010,    /* 8.2.3  OM_WAS_ACT_OR_SERV_CELL_INFO_REQ (Serving Cell)  */
    0x2300B013,    /* 8.2.3 OM_WAS_ACT_OR_SERV_CELL_INFO_REQ(Neightborhood Cell)  */
    0x2300B063,    /* 8.2.3 OM_WAS_ACT_OR_SERV_CELL_INFO_REQ(Second Cell)  */
    0x2300B016,    /* 8.2.4 GSM NCells REQ  OM_WAS_ACT_OR_SERV_CELL_INFO_REQ   0x2300B016  */
    0x2300B019,    /* 8.2.5 WAS CELL CHG REQ  0x2300B019  */
    0x2300B01C,    /* 8.2.6 OM_WAS_UE_STATE_REQ   0x2300B01C  */
    0x2300B020,    /* 8.2.7 OM_WAS_OTA_REQ  0x2300B020  */
    0x2300B060,    /* 8.2.8 OM_WAS_L_NCELL_INFO_REQ  0x2300B060  */
    0x2300B066,    /* 8.2.9 OM_WAS_TIMER_EVENT_REQ  0x2300B066  */
    0x2200D104,    /* 8.2.10 GRM_MNTN_OM_GRM_INFO_CFG_REQ  0x2200D104  */
    0x2200D10a,    /* 8.2.11 GSM_MNTN_OM_GTTF_ADV_TRACE_SWITCH_REQ  0x2200D10a   */
    0x2300D004,    /* 8.2.12 OM_WTTF_ADVANCED_TRACE_CONFIG_REQ  0x2300D004   */
    0x2300D313,    /* 8.2.13 OM_IPS_ADVANCED_TRACE_CONFIG_REQ  0x2300D313   */
    0x2300D513,    /* 8.2.14 OM_IPS_CCORE_ADVANCED_TRACE_CONFIG_REQ  0x2300D513 */
};

VOS_UINT32   gulPhyAllowCmdList[] = {
    0x30004101,    /*, LPHY_DT_UL_PUSCH_INFO_REQ  0x30004101 */
    0x30000800,    /*, LPHY_REQ_AGC_REPORT_ENA   0x30000800 */
    0x30000C00,    /*, LPHY_REQ_AFC_REPORT_ENA  0x30000C00  */
    0x30001400,    /*, LPHY_REQ_CMU_REPORT_ENA,  0x30001400  */
    0x30001800,    /*, LPHY_REQ_EMU_SG_EN   0x30001800  */
    0x30001801,    /*  LPHY_REQ_EMU_SG_SERVE_EN 0x30001801 --*/
    0x30004001,    /*, LPHY_REQ_UL_BASIC_INFO   0x30004001  */
    0x30004002,    /*, LPHY_REQ_UL_RANDOEM_ACCESS_INFO   0x30004002 */
    0x30004003,    /*, LPHY_REQ_UL_CTRL_INFO    0x30004003  */
    0x30004004,    /*, LPHY_REQ_UL_ACK_INFO    0x30004004  */
    0x30004006,    /*, LPHY_REQ_UL_CAPTURE_INFO    0x30004006  */
    0x30004006,    /*, LPHY_REQ_UL_CAPTURE_INFO    0x30004006  */
    0x3000400a,    /*  LPHY_REQ_UL_SCHEDUL_STATIC_INFO  0x3000400a  -- */
    0x3000400e,    /*  LPHY_SG_UL_SCHEDULE_STAT_INFO_REQ  0x3000400e -- */
    0x3000400f,    /*  LPHY_SG_UL_POWER_INFO_REQ  0x3000400f -- */
    0x30004403,    /*  LPHY_REQ_CHANCTRL_CELL_STAT_REQUEST  0x30004403  -- */
    0x30004899,    /*, LPHY_REQ_OM_ENA_RPT_ALL  0x30004899  */
    0x30004801,    /*  LPHY_REQ_OM_ENA_RPT_RFE  0x30004801  -- */
    0x30004802,    /*, LPHY_REQ_OM_ENA_RPT_AGC  0x30004802  */
    0x30004803,    /*  LPHY_REQ_OM_ENA_RPT_AFC   0x30004803 -- */
    0x30004805,    /*, LPHY_REQ_OM_ENA_RPT_CMU  0x30004805  */
    0x30004806,    /*, LPHY_REQ_OM_ENA_RPT_EMU  0x30004806  */
    0x3000480d,    /*, LPHY_REQ_OM_ENA_RPT_TIMING  0x3000480d  */
    0x30004810,    /*, LPHY_REQ_OM_ENA_RPT_UL  0x30004810  */
    0x30004811,    /*, LPHY_REQ_OM_ENA_RPT_CHANCTRL  0x30004811  */
    0x30004812,    /*, LPHY_REQ_OM_ENA_RPT_OM  0x30004812  */
    0x30004814,    /*, LPHY_REQ_OM_ENA_RPT_VITERBI  0x30004814  */
    0x30004815,    /*, LPHY_REQ_OM_ENA_RPT_TURBO  0x30004815  */
    0x30004900,    /*, LPHY_REQ_OM_ENA_PRINT_LEVEL  0x30004900  */
    0x30004901,    /*, LPHY_REQ_OM_ENA_ L1MSG_LEVEL  0x30004901  */
    0x30005004,    /* LPHY_REQ_VITERBI_RPT_PDCCH_SINR_ENA  0x30005004  -- */
    0x30005005,    /*, LPHY_REQ_VITERBI_RPT_PDCCH_DL_GRANT_ENA  0x30005005  */
    0x30005006,    /*, LPHY_REQ_VITERBI_RPT_PDCCH_UL_GRANT_ENA  0x30005006  */
    0x30005007,    /*  LPHY_REQ_VITERBI_RPT_PDCCH_PWR_GRANT_ENA  0x30005007  -- */
    0x30005008,    /*  LPHY_REQ_VITERBI_RPT_PDCCH_GRANT_STAT_ENA  0x30005008 -- */
    0x30005405,    /*, LPHY_REQ_TURBO_RPT_DL_THROUGHPUT_STA_ENA_STRU  0x30005405  */
    0x30008001,    /*, LPHY_REQ_OM_ENA_PRINT_XO  0x30008001  */
    0x31000401,    /*, TPHY_DRV_INTERFACE_REQ  0x31000401  */
    0x31000802,    /*, TPHY_REQ_AGC_REPORT_SWITCH  0x31000802  */
    0x31000C02,    /*, TPHY_REQ_AFC_REPORT_SG_ENA  0x31000C02  */
    0x31001001,    /*, TPHY_REQ_CS_REQST_SWITCH  0x31001001  */
    0x31001800,    /*, TPHY_MEAS_OM_SWITCH_REQ  0x31001800  */
    0x31001801,    /*  TPHY_MEAS_TIMER_SWITCH_REQ  0x31001801  -- */
    0x31002c00,    /*, TPHY_PC_REQ _ENUM    0x31002c00  */
    0x31003000,    /*, TPHY_REQ_SYNC_OM_REP_SWITCH   0x31003000  */
    0x31003402,    /*, 6.37 TDS_REQ_TIMING_REPORT_SG_ENA  0x31003402  */
    0x31004000,    /*, 6.38 TPHY_REQ_UL_REPORT _ENA  0x31004000  */
    0x31004800,    /*, 6.39 TPHY_REQ_OM_ENA_RPT_RESERVED   0x31004800  */
    0x31004801,    /*, 6.40 TPHY_REQ_OM_ENA_RPT_RFE  0x31004801  */
    0x31004802,    /*, 6.41 TPHY_REQ_OM_ENA_RPT_AGC  0x31004802  */
    0x31004803,    /*  6.42 TPHY_REQ_OM_ENA_RPT_AFC  0x31004803  */
    0x31004804,    /*  6.43 TPHY_REQ_OM_ENA_RPT_CS  0x31004804  */
    0x31004805,    /*  6.44 TPHY_REQ_OM_ENA_RPT_CMU  0x31004805  */
    0x31004806,    /*  6.45 TPHY_REQ_OM_ENA_RPT_EMU  0x31004806  */
    0x31004807,    /*  6.46 TPHY_REQ_OM_ENA_RPT_CHE   0x31004807  */
    0x31004808,    /*  6.47 TPHY_REQ_OM_ENA_RPT_DATA  0x31004808  */
    0x31004809,    /*  6.48 TPHY_REQ_OM_ENA_RPT_CORR  0x31004809  */
    0x3100480a,    /*  6.49 TPHY_REQ_OM_ENA_RPT_CQI   0x3100480a  */
    0x3100480b,    /*  6.50 TPHY_REQ_OM_ENA_RPT_PC  0x3100480b  */
    0x3100480c,    /*  6.51 TPHY_REQ_OM_ENA_RPT_SYNC  0x3100480c  */
    0x3100480d,    /*  6.52 TPHY_REQ_OM_ENA_RPT_TIMING  0x3100480d  */
    0x3100480e,    /*  6.53 TPHY_REQ_OM_ENA_RPT_DRX  0x3100480e  */
    0x3100480f,    /*  6.54 TPHY_REQ_OM_ENA_RPT_SERV  0x3100480f  */
    0x31004810,    /*  6.55 TPHY_REQ_OM_ENA_RPT_UL  0x31004810  */
    0x31004811,    /*  6.56 TPHY_REQ_OM_ENA_RPT_CHANCTRL  0x31004811  */
    0x3100480f,    /*  6.57 TPHY_REQ_OM_ENA_RPT_SERV  0x3100480f  */
    0x31004811,    /*  6.58 TPHY_REQ_OM_ENA_RPT_CHANCTRL  0x31004811  */
    0x31004812,    /*  6.59 TPHY_REQ_OM_ENA_RPT_CHANCTRL  0x31004812  */
    0x31004813,    /*  6.60 TPHY_REQ_OM_ENA_RPT_DMA  0x31004813  */
    0x31004814,    /*  6.61 TPHY_REQ_OM_ENA_RPT_CD  0x31004814  */
    0x31004815,    /*  6.62 TPHY_REQ_OM_ENA_VIT_TURBO  0x31004815  */
    0x31004816,    /*  6.63 TPHY_REQ_OM_ENA_RPT_SCHED  0x31004816  */
    0x31004817,    /*  6.64 TPHY_REQ_OM_ENA_RPT_INTERRUPT  0x31004817  */
    0x31004818,    /*  6.65 TPHY_REQ_OM_ENA_RPT_JD  0x31004818  */
    0x31005005,    /*  6.66 THY_REQ_CD_OM_REP_DSCH  0x31005005  */
    0x3100480b,    /*  6.67 TPHY_TRACE_DWPC_DPCH  0x3100480b  */

};

VOS_UINT32   gulBspAllowCmdList[] = {
    0x90015003,    /*  DIAG_CMD_NV_RD    0x90015003  */
    0x90015005,    /*DIAG_CMD_GET_NV_LIST    0x90015005 */
};

DIAG_OM_WHITELIST_ST  gstOmWhiteList[] = {
    {
        DIAG_MSG_TYPE_MSP,
        (sizeof(gulMspAllowCmdList) / sizeof(gulMspAllowCmdList[0])),
        gulMspAllowCmdList,
        0,
    },
    {
        DIAG_MSG_TYPE_PS,
        (sizeof(gulPsAllowCmdList) / sizeof(gulPsAllowCmdList[0])),
        gulPsAllowCmdList,
        0,
    },
    {
        DIAG_MSG_TYPE_PHY,
        (sizeof(gulPhyAllowCmdList) / sizeof(gulPhyAllowCmdList[0])),
        gulPhyAllowCmdList,
        0,
    },
    {
        DIAG_MSG_TYPE_BSP,
        (sizeof(gulBspAllowCmdList) / sizeof(gulBspAllowCmdList[0])),
        gulBspAllowCmdList,
        DIAG_CTRL_NEED_DATALOCK,
    },
};


VOS_UINT32 MbbDiag_OmFilter(DIAG_FRAME_INFO_STRU *pData)
{
    DIAG_MESSAGE_TYPE_U32 ulMsgType;
    VOS_UINT32 i;
    VOS_UINT32 j;
    
    /*Èë²Î¼ì²é*/
    if (VOS_NULL_PTR == pData)
    {
        return VOS_ERR;
    }

    ulMsgType = pData->stID.pri4b;
    
    for ( i = 0; i < (sizeof(gstOmWhiteList) / sizeof(gstOmWhiteList[0])); i++)
    {
        if (gstOmWhiteList[i].ulMsgType == ulMsgType)
        {
            if (DIAG_CTRL_NEED_DATALOCK == (gstOmWhiteList[i].ulCtrl & DIAG_CTRL_NEED_DATALOCK))
            {
                if (VOS_TRUE != Mbb_IsDataLockPass())
                {
                    return VOS_ERR;
                }
            }
            
            for ( j = 0; j < gstOmWhiteList[i].ulCmdListCount; j++)
            {
                if (pData->ulCmdId == gstOmWhiteList[i].szulAllowCmdList[j])
                {
                    return VOS_OK;
                }
            }

            return VOS_ERR;
        }
    }

    return VOS_ERR;
}



VOS_UINT32 MbbMsp_OmFilter(VOS_VOID *pHeader)
{
    VOS_UINT32  uMspSIDtype;

    if(VOS_NULL_PTR == pHeader)
    {
        return VOS_ERR;
    }

    uMspSIDtype = ((DIAG_SERVICE_HEAD_STRU*)pHeader)->sid8b;

    /* only allow DiagServiceType */
    if (MSP_SID_DIAG_SERVICE != uMspSIDtype)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}



