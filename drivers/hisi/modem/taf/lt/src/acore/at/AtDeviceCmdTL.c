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
/*lint --e{7,537,305,322}*/
#include "AtParse.h"
#include "ATCmdProc.h"
#include "AtDeviceCmd.h"
#include "AtCheckFunc.h"
#include "mdrv.h"
#include "AtCmdMsgProc.h"
#include "LNvCommon.h"
#include "at_lte_common.h"

/* ������LTE��TDS˽��װ��AT���� */
AT_PAR_CMD_ELEMENT_STRU g_astAtDeviceCmdTLTbl[] = {
    /*BEGIN: LTE ����У׼װ��AT���� */
    {AT_CMD_BANDSW,
    atSetBandSWPara,      AT_SET_PARA_TIME, atQryBandSWPara,     AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
	(VOS_UINT8*)"^BANDSW",(VOS_UINT8*)"(2,6,7,8),(0-65535),(0-65535),(0-65535),(0-3)"},
    {AT_CMD_F_FCHANS,
    atSetFFCHANSPara,     AT_SET_PARA_TIME, atQryFFCHANSPara,    AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FCHANS",(VOS_UINT8*)"(0-32),(@freq_list),(0-32),(@freq_list)"},

    {AT_CMD_F_FSEGMENT,
    atSetFFSEGMENTPara,   AT_SET_PARA_TIME, atQryFFSEGMENTPara,  AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FSEGMENT", (VOS_UINT8*)"(0-65535),(0-65535),(0-65535)"},

    {AT_CMD_F_FPOWS,
    atSetFFPOWSPara,      AT_SET_PARA_TIME, atQryFFPOWSPara,     AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FPOWS",        (VOS_UINT8*)"(0-30),(@txpow_list)"},

    {AT_CMD_F_FPAS,
    atSetFFPASPara,       AT_SET_PARA_TIME, atQryFFPASPara,      AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FPAS",  (VOS_UINT8*)"(0-30),(@pa_list)"},

    {AT_CMD_F_FLNAS,
    atSetFFLNASPara,      AT_SET_PARA_TIME, atQryFFLNASPara,     AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FLNAS", (VOS_UINT8*)"(0-30),(@lna_list)"},
    {AT_CMD_F_FTXWAVE,
    atSetFFTXWAVEPara,    AT_SET_PARA_TIME, atQryFFTXWAVEPara,   AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FTXWAVE", (VOS_UINT8*)"(0,1,11)"},
    {AT_CMD_F_FSTART,
    atSetFFSTARTPara,     AT_SET_PARA_TIME, atQryFFSTARTPara,    AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FSTART", (VOS_UINT8*)"(0,1,2,3)"},

    {AT_CMD_F_FRSSIS,
    atSetFFRSSISPara,     AT_SET_PARA_TIME, VOS_NULL_PTR,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FRSSIS", (VOS_UINT8*)"(0-65535)"},

    {AT_CMD_F_FCMTMS,
    VOS_NULL_PTR,         AT_NOT_SET_TIME, atSetFFCMTMSPara,   AT_QRY_PARA_TIME ,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FCMTMS",    NULL},

    {AT_CMD_F_FBLK,
    atSetFBLKPara,        AT_SET_PARA_TIME, atQryFBLKPara,       AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FBLK", (VOS_UINT8*)"(0,1)"},

    {AT_CMD_F_FIPSTART,
    atSetFIPSTARTPara,    AT_SET_PARA_TIME, VOS_NULL_PTR,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FIPSTART", (VOS_UINT8*)"(0-65535)"},

    {AT_CMD_F_FMAXPOWER,
    atSetFMAXPOWERPara,   AT_SET_PARA_TIME, atQryFMAXPOWERPara,  AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FMAXPOWER", (VOS_UINT8*)"(0-4),(4-20)"},

    {AT_CMD_F_FPAPOWER,
    atSetFPAPOWERPara,    AT_SET_PARA_TIME, atQryFPAPOWERPara,   AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FPAPOWER", (VOS_UINT8*)"(0-4),(@MaxPwr_List)"},

    {AT_CMD_F_FPDMS,
    atSetFPDMSPara,    AT_SET_PARA_TIME, atQryFPDMSPara,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FPDMS", (VOS_UINT8*)"(0-1)"},

    {AT_CMD_F_FCALIIP2S,
    atSetFCALIIP2SPara, AT_SET_PARA_TIME, atQryFCALIIP2SPara,        AT_QRY_PARA_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FCALIIP2S", (VOS_UINT8*)"(0-255),(@ChanFreq_List)"},

    {AT_CMD_F_FCALIDCOCS,
     atSetFCALIDCOCSPara, AT_SET_PARA_TIME, atQryFCALIDCOCSPara,       AT_QRY_PARA_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
     VOS_NULL_PTR,        AT_NOT_SET_TIME,
     AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
     (VOS_UINT8*)"^FCALIDCOCS", (VOS_UINT8*)"(0-65535)"},

    {AT_CMD_F_FCALIIP2SMRF,
    atSetFCALIIP2SMRFPara, AT_SET_PARA_TIME, atQryFCALIIP2SMRFPara,        AT_QRY_PARA_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
	(VOS_UINT8*)"^FCALIIP2SMRF", (VOS_UINT8*)"(0-255),(@ChanFreq_List),(0-65535),(0-3)"},
    {AT_CMD_F_FCALIDCOCSMRF,
     atSetFCALIDCOCSMRFPara, AT_SET_PARA_TIME, atQryFCALIDCOCSMRFPara,       AT_QRY_PARA_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
     VOS_NULL_PTR,        AT_NOT_SET_TIME,
     AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
	(VOS_UINT8*)"^FCALIDCOCSMRF", (VOS_UINT8*)"(0-65535),(0-65535),(0-3)"},
    {AT_CMD_F_FGAINSTATES,
     atSetFGAINSTATESPara, AT_SET_PARA_TIME, atQryFGAINSTATESPara,       AT_QRY_PARA_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
     VOS_NULL_PTR,        AT_NOT_SET_TIME,
     AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
     (VOS_UINT8*)"^FGAINSTATES", (VOS_UINT8*)"(0-30),(@Level_List)"},

	{AT_CMD_F_FDBBATTS,
	atSetFDBBATTSPara,		 AT_SET_PARA_TIME,	 atQryFDBBATTSPara,	AT_QRY_PARA_TIME,  VOS_NULL_PTR, AT_SET_PARA_TIME,
	VOS_NULL_PTR,        AT_NOT_SET_TIME,
	AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
	(VOS_UINT8*)"^FDBBATTS",(VOS_UINT8*)"(0-30),(0-31)"},
	{AT_CMD_F_FBBATTS,
	atSetFBBATTSPara,		 AT_SET_PARA_TIME,	 atQryFBBATTSPara,	AT_QRY_PARA_TIME,  VOS_NULL_PTR, AT_SET_PARA_TIME,
	VOS_NULL_PTR,        AT_NOT_SET_TIME,
	AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
	(VOS_UINT8*)"^FBBATTS",	  (VOS_UINT8*)"(0-30),(@BB_ATT_List)"},

	{AT_CMD_F_FCALITXIQS,
	atSetFCALITXIQSPara,     AT_SET_PARA_TIME,  atQryFCALITXIQSPara,	AT_QRY_PARA_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
	VOS_NULL_PTR,        AT_NOT_SET_TIME,
	AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
	(VOS_UINT8*)"^FCALITXIQS", (VOS_UINT8*)"(0-65535),(0-65535),(0-3)"},
    {AT_CMD_FVCTCXO,
    atSetVCTCXOPara,     AT_SET_PARA_TIME,  atQryVCTCXOPara,     AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FVCTCXO",      (VOS_UINT8*)"(0-65535)"},

    {AT_CMD_FCALPDDCS,
    atSetFCALPDDCSPara,     AT_SET_PARA_TIME,  atQryFCALPDDCSPara,     AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FCALPDDCS",      (VOS_UINT8*)"(0-1)"},/*on/off*/

    {AT_CMD_FPDPOWS,
    atSetFPDPOWSPara,     AT_SET_PARA_TIME,  VOS_NULL_PTR,     AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FPDPOWS",      (VOS_UINT8*)"(0-1),(0-65535),(0-65535)"},

    {AT_CMD_FQPDDCRES,
    atSetFQPDDCRESPara,     AT_SET_PARA_TIME,  VOS_NULL_PTR,     AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FQPDDCRES",      (VOS_UINT8*)"(0-65535)"},

	{AT_CMD_NVRDLEN,
	 atSetNVRDLenPara,		 AT_SET_PARA_TIME,	VOS_NULL_PTR,		 AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
	 VOS_NULL_PTR,		  AT_NOT_SET_TIME,
	 AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
	 (VOS_UINT8*)"^NVRDLEN",(VOS_UINT8*)"(0-65535)"},

	 {AT_CMD_NVRDEX,
	 atSetNVRDExPara,		AT_SET_PARA_TIME,  VOS_NULL_PTR,		AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
	 VOS_NULL_PTR,		  AT_NOT_SET_TIME,
	 AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
	 (VOS_UINT8*)"^NVRDEX",(VOS_UINT8*)"(0-65535),(0-2048),(0-2048)"},

    {AT_CMD_NVWREX,
    atSetNVWRExPara,       AT_SET_PARA_TIME,  VOS_NULL_PTR,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NVWREX",(VOS_UINT8*)"(0-65535),(0-2048),(0-2048),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data)"},
    /*END: LTE ����У׼װ��AT���� */

    /*BEGIN: LTE �������۲�װ��AT���� */
    {AT_CMD_SSYNC,
    atSetSSYNCPara,      AT_SET_PARA_TIME,     atQrySSYNCPara,      AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SSYNC",         (VOS_UINT8*)"(0-65535),(0-255),(0-65535),(0-65535)"},

    {AT_CMD_STXBW,
    atSetSTXBWPara,      AT_SET_PARA_TIME,     atQrySTXBWPara,      AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^STXBW",         (VOS_UINT8*)"(0-5)"},

    {AT_CMD_STXCHAN,
    atSetSTXCHANPara,    AT_SET_PARA_TIME,     atQrySTXCHANPara,   AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^STXCHAN",         (VOS_UINT8*)"(0-65535)"},

    {AT_CMD_SSUBFRAME,
    atSetSSUBFRAMEPara,  AT_SET_PARA_TIME,     atQrySSUBFRAMEPara,  AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SSUBFRAME",         (VOS_UINT8*)"(1-8),(1-8)"},

    {AT_CMD_SPARA,
    atSetSPARAPara,      AT_SET_PARA_TIME,     atQrySPARAPara,      AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SPARA",         (VOS_UINT8*)"(0-65535),(0-65535)"},

    {AT_CMD_SSEGNUM,
    atSetSSEGNUMPara,    AT_SET_PARA_TIME,     atQrySSEGNUMPara,    AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SSEGNUM",         (VOS_UINT8*)"(0-256)"},

    {AT_CMD_STXMODUS,
    atSetSTXMODUSPara,   AT_SET_PARA_TIME,     atQrySTXMODUSPara,   AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^STXMODUS",         (VOS_UINT8*)"(0-50),(@modulation_list)"},

    {AT_CMD_STXRBNUMS,
    atSetSTXRBNUMSPara,  AT_SET_PARA_TIME,     atQrySTXRBNUMSPara,  AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^STXRBNUMS",         (VOS_UINT8*)"(0-50),(@rb_list)"},

    {AT_CMD_STXRBPOSS,
    atSetSTXRBPOSSPara,  AT_SET_PARA_TIME,     atQrySTXRBPOSSPara,  AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^STXRBPOSS",         (VOS_UINT8*)"(0-50),(@rb_list)"},

    {AT_CMD_STXPOWS,
    atSetSTXPOWSPara,    AT_SET_PARA_TIME,     atQrySTXPOWSPara,    AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^STXPOWS",         (VOS_UINT8*)"(0-50),(@power_list)"},

    {AT_CMD_STXCHANTYPES,
    atSetSTXCHANTYPESPara, AT_SET_PARA_TIME,   atQrySTXCHANTYPESPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^STXCHANTYPES",         (VOS_UINT8*)"(0-50),(@type_list)"},

    {AT_CMD_SSEGLEN,
    atSetSSEGLENPara,    AT_SET_PARA_TIME,     atQrySSEGLENPara,    AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SSEGLEN",         (VOS_UINT8*)"(0-50)"},

    {AT_CMD_SRXSET,
    atSetSRXSETPara,     AT_SET_PARA_TIME,     atQrySRXSETPara,     AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SRXSET",         (VOS_UINT8*)"(0,1),(@ulPower),(0-2),(1-100),(0-99)"},

    {AT_CMD_SRXSUBFRA,
    atSetSRXSUBFRAPara,  AT_SET_PARA_TIME,     atQrySRXSUBFRAPara,  AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SRXSUBFRA",         (VOS_UINT8*)"(0-10000)"},

    {AT_CMD_SRXBLER,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,      atQrySRXBLERPara,    AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SRXMEAS",         NULL},//���������� SRXBLER ��Ϊ SRXMEAS  ���ṹ�����Ʋ���

    {AT_CMD_SSTART,
    atSetSSTARTPara,     AT_SET_PARA_TIME,     atQrySSTARTPara,     AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SSTART",         (VOS_UINT8*)"(0-65535)"},

    {AT_CMD_SSTOP,
    atSetSSTOPPara,      AT_SET_PARA_TIME,     VOS_NULL_PTR,        AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SSTOP",         (VOS_UINT8*)"(0,1)"},
    {AT_CMD_STCFGDPCH,
    atSetSTCFGDPCHPara,      AT_SET_PARA_TIME,     VOS_NULL_PTR,        AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^STCFGDPCH",    (VOS_UINT8*)"(0,1),(0-65535)"},

    {AT_CMD_FPOW,
     atSetFPOWPara,	  AT_SET_PARA_TIME, 	VOS_NULL_PTR,		 AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
     VOS_NULL_PTR,		  AT_NOT_SET_TIME,
     AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
     (VOS_UINT8*)"^FPOW",	  (VOS_UINT8*)"(@pow)"},

     {AT_CMD_STRXBER,
     VOS_NULL_PTR,        AT_NOT_SET_TIME,      atQrySTRXBERPara,    AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
     VOS_NULL_PTR,        AT_NOT_SET_TIME,
     AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
     (VOS_UINT8*)"^STRXMEAS",         NULL},  
    /*END: LTE �������۲�װ��AT���� */

    /*BEGIN: TDS װ��AT���� */
    {AT_CMD_SCELLINFO,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    At_QryTdsScellinfoPara,    AT_QRY_PARA_TIME,    VOS_NULL_PTR,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SCELLINFO",    VOS_NULL_PTR},

    {AT_CMD_SCALIB,
    At_SetTdsScalibPara,        AT_SET_PARA_TIME,    At_QryTdsScalibPara,    AT_QRY_PARA_TIME,    At_TestTdsScalibPara,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SCALIB",    AT_TDS_SCALIB_STR},

    {AT_CMD_CMTM,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    At_QryCmtm,         AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^CMTM",  VOS_NULL_PTR},
    /*END: TDS װ��AT���� */

    {AT_CMD_LTCOMMCMD,
    atSetLTCommCmdPara,     AT_SET_PARA_TIME,     atQryLTCommCmdPara,     AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^LTCOMMCMD",  (VOS_UINT8*)"(0-65535),(0-2000),(0-65535),(0-65535),(@data),(@data),(@data),(@data)"},    

};

/*****************************************************************************
 �� �� ��  : At_RegisterDeviceCmdTLTable
 ��������  : ע��װ�������
 �������  : VOS_VOID
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 At_RegisterDeviceCmdTLTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_astAtDeviceCmdTLTbl, sizeof(g_astAtDeviceCmdTLTbl)/sizeof(g_astAtDeviceCmdTLTbl[0]));
}


