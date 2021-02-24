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

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/usb/bsp_usb.h>

#include "mdrv_udi.h"
#include "bsp_ncm.h"
#include "ncm_balong.h"


/*****************************************************************************
* �� �� ��  : bsp_ncm_open
*
* ��������  : ������ȡ����ʹ��NCM�豸ID,����ͨ�����ͣ�PSʹ��;����ͨ�����ͣ�
*              MSP����AT����ʹ��
*
* �������  : NCM_DEV_TYPE_E enDevType  �豸����
* �������  : pu32NcmDevId              NCM �豸ID
*
* �� �� ֵ  : BSP_ERR_NET_NOIDLEDEV
*             OK
*
* �޸ļ�¼  : 2010��3��17��   liumengcun  creat
*****************************************************************************/
unsigned int bsp_ncm_open(NCM_DEV_TYPE_E enDevType, unsigned int pu32NcmDevId)
{
    if(bsp_usb_is_ncm_bypass_mode())
    {
        return (unsigned int)ncm_vendor_open(enDevType, pu32NcmDevId);
    }
    else
    {
        return -1;
    }
}
EXPORT_SYMBOL(bsp_ncm_open);
    
/*****************************************************************************
* �� �� ��  : bsp_ncm_write
*
* ��������  : ��������
*
* �������  : unsigned int u32NcmDevId  NCM�豸ID
                             void *     net_priv tcp/ip�����Ĳ���������tcp/ip�Խ�ʱ�ò�������ΪNULL
* �������  : void *pPktEncap       ����װ�׵�ַ
*
* �� �� ֵ  : BSP_OK
*             BSP_ERR_NET_INVALID_PARA
*             BSP_ERR_NET_BUF_ALLOC_FAILED
*
* �޸ļ�¼  : 2010��3��17��   liumengcun  creat
*****************************************************************************/
unsigned int bsp_ncm_write(unsigned int u32NcmDevId, void *pPktEncap, void *net_priv)
{
    if(bsp_usb_is_ncm_bypass_mode())
    {
        return (unsigned int)ncm_vendor_write(u32NcmDevId, pPktEncap, net_priv);
    }
    else
    {
        return -1;
    }
}
    EXPORT_SYMBOL(bsp_ncm_write);
/*****************************************************************************
* �� �� ��  : bsp_ncm_ioctl
*
* ��������  : ����NCM�豸����
*
* �������  : ��
* �������  : ��
*
* �� �� ֵ  :
*
* �޸ļ�¼  : 2010��3��17��   liumengcun  creat
*****************************************************************************/
unsigned int bsp_ncm_ioctl(unsigned int u32NcmDevId, NCM_IOCTL_CMD_TYPE_E enNcmCmd, void *param)
{
    if(bsp_usb_is_ncm_bypass_mode())
    {
        return ncm_vendor_ioctl(u32NcmDevId, enNcmCmd, param);/* [false alarm]:fortify disable */
    }
    else
    {
        return -1;
    }
}
EXPORT_SYMBOL(bsp_ncm_ioctl);
/*****************************************************************************
* �� �� ��  : bsp_ncm_close
*
* ��������  : �ر�NCM�豸
*
* �������  : NCM_DEV_TYPE_E enDevType   �豸ID����
*             unsigned int u32NcmDevId        NCM�豸ID
* �������  : ��
*
* �� �� ֵ  : OK
*
* �޸ļ�¼  : 2010��3��17��   liumengcun  creat
*****************************************************************************/
unsigned int bsp_ncm_close(NCM_DEV_TYPE_E enDevType, unsigned int u32NcmDevId)
{
    if(bsp_usb_is_ncm_bypass_mode())
    {
        return ncm_vendor_close(enDevType, u32NcmDevId);/* [false alarm]:fortify disable */
    }
    else
    {
        return -1;
    }
}
EXPORT_SYMBOL(bsp_ncm_close);


