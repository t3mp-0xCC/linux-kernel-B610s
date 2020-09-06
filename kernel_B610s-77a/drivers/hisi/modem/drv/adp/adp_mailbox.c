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

#include "drv_comm.h"
#include "drv_mailbox_cfg.h"
#include "bsp_icc.h"
#include "bsp_nvim.h"
#include "bsp_shared_ddr.h"


void drv_hifi_fill_mb_info(unsigned int addr)
{
    CARM_HIFI_DYN_ADDR_SHARE_STRU *pdata;

    pdata = (CARM_HIFI_DYN_ADDR_SHARE_STRU*)addr;

    pdata->stCarmHifiMB.uwHifi2CarmIccChannelLen   = (unsigned int)(STRU_SIZE + ICC_HIFI_VOS_SIZE);
    pdata->stCarmHifiMB.uwHifi2TphyIccChannelLen   = (unsigned int)(STRU_SIZE + ICC_HIFI_TPHY_SIZE);
    pdata->stCarmHifiMB.uwHifi2CarmIccChannelAddr  = (unsigned int)SHD_DDR_V2P(ADDR_HIFI_VOS_MSG_SEND);
    pdata->stCarmHifiMB.uwCarm2HifiIccChannelAddr  = (unsigned int)SHD_DDR_V2P(ADDR_HIFI_VOS_MSG_RECV);
    pdata->stCarmHifiMB.uwHifi2TphyIccChannelAddr  = (unsigned int)SHD_DDR_V2P(ADDR_HIFI_TPHY_MSG_SEND);
    pdata->stCarmHifiMB.uwTphy2HifiIccChannelAddr  = (unsigned int)SHD_DDR_V2P(ADDR_HIFI_TPHY_MSG_RECV);
    pdata->stCarmHifiMB.uwProtectWord           = HIFI_MB_ADDR_PROTECT;


    pdata->stAarmHifiMB.uwHifi2AarmMailBoxLen   = MAILBOX_QUEUE_SIZE(HIFI, ACPU, MSG);
    pdata->stAarmHifiMB.uwAarm2HifiMailBoxLen   = MAILBOX_QUEUE_SIZE(ACPU, HIFI, MSG);
    pdata->stAarmHifiMB.uwHifiAarmHeadAddr      = MAILBOX_HEAD_ADDR(HIFI, ACPU, MSG);
    pdata->stAarmHifiMB.uwHifiAarmBodyAddr      = MAILBOX_QUEUE_ADDR(HIFI, ACPU, MSG);
    pdata->stAarmHifiMB.uwAarmHifiHeadAddr      = MAILBOX_HEAD_ADDR(ACPU, HIFI, MSG);
    pdata->stAarmHifiMB.uwAarmHifiBodyAddr      = MAILBOX_QUEUE_ADDR(ACPU, HIFI, MSG);
    pdata->stAarmHifiMB.uwProtectWord           = HIFI_MB_ADDR_PROTECT;

	pdata->stNVShare.uwAddrPhy = (unsigned int)SHD_DDR_V2P(NV_GLOBAL_CTRL_INFO_ADDR);
	pdata->stNVShare.uwSize = SHM_MEM_NV_SIZE;
	pdata->stNVShare.uwProtectWord = HIFI_MB_ADDR_PROTECT;


    pdata->uwNvBaseAddrPhy = (unsigned int)SHD_DDR_V2P(NV_GLOBAL_CTRL_INFO_ADDR);
    pdata->uwNvBaseAddrVirt = 0; /*NV虚拟地址不需要提供*/

    pdata->uwProtectWord = HIFI_MB_ADDR_PROTECT;

    return ;

}



