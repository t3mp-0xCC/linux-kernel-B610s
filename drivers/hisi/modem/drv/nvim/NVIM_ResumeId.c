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

#include "NVIM_ResumeId.h"
#include "NvIdList.h"

#include "mdrv_nvim_comm.h"
#include <drv_nv_id.h>

/*lint -e553 */
#include "LNvCommon.h"
#include "lt_phy_nv_define.h"
#include "product_nv_id.h"
/*lint +e553 */
#include "mbb_config.h"
#include "product_nv_id.h"

/* Manufacture ID need to restore */
unsigned short  g_ausNvResumeManufactureIdList[] =
{
/********* Ӧ������END *********/
/************************������MBB&HOME��ӵ�NV��*************************/

/************************�����Ǻ�˼ԭʼ��NV��*************************/


};

/* User ID need to restore */
unsigned short  g_ausNvResumeUserIdList[] =
{

};


/* �Ǽ��ܰ汾��Ҫ���лָ��Ļ�Ҫ����NV�� */
unsigned short  g_ausNvResumeSecureIdList[] =
{
};

/*��С������������Ҫ�ָ���NV��*/

/* MBB ��Ʒ�߻ָ���������NV�б� ����Ʒ���ݶ���������Լ����*/
/* ����2015/5/25 V7R5�ָ�����NV����������ͨ�ûָ���NV */
unsigned short  g_ausNvResumeDefualtIdList[] =
{

};

/* �޸�nv��Ҫ�Զ�����NVģ����ط�����NV�б� ����Ʒ���ݶ���������Լ����*/
unsigned short  g_ausNvAutoBakeupIdList[] =
{
    en_NV_Item_IMEI,
    /*nv���ڿ���telnet at����,�������ȱ���,�����˿�,���������˿�(�޸�nv)ʱ����Ҫ��ʱ����*/
    NV_ID_TELNET_SWITCH_I,
};

/*****************************************************************************
Function   : NV_GetResumeNvIdNum
Description: Return the number of resumed NV.
Input      : NV_RESUME_ITEM_ENUM_UINT32 - resumed NV's type.
Return     : Zero or others.
Other      :
*****************************************************************************/
unsigned long bsp_nvm_getRevertNum(unsigned long enNvItem)
{
    if (NV_MANUFACTURE_ITEM == enNvItem)
    {
        return sizeof(g_ausNvResumeManufactureIdList)/sizeof(g_ausNvResumeManufactureIdList[0]);
    }

    if (NV_USER_ITEM == enNvItem)
    {
        return sizeof(g_ausNvResumeUserIdList)/sizeof(g_ausNvResumeUserIdList[0]);
    }

    if (NV_SECURE_ITEM == enNvItem)
    {
        return sizeof(g_ausNvResumeSecureIdList)/sizeof(g_ausNvResumeSecureIdList[0]);
    }

    if(NV_MBB_DEFUALT_ITEM == enNvItem)
    {
        return sizeof(g_ausNvResumeDefualtIdList) / sizeof(g_ausNvResumeDefualtIdList[0]);
    }
    if(NV_MBB_AUTOBACKEUP_ITEM == enNvItem)
    {
        return sizeof(g_ausNvAutoBakeupIdList) / sizeof(g_ausNvAutoBakeupIdList[0]);
    }


    return 0;
}


