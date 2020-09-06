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



#ifndef __CBPA_PS_INTERFACE_H__
#define __CBPA_PS_INTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "vos.h"


#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define CBPCA_MSG_DATA_RSV_LEN          (4)                 /* CBPCA收发的消息结构体中，数据临时长度 */

/*******************************************************************************
  3 枚举定义
*******************************************************************************/
/*****************************************************************************
 枚举名    : CBPCA_MSG_TYPE_ENUM
 枚举说明  : CBPCA与其他模块间的消息ID定义
*****************************************************************************/
enum CBPCA_MSG_TYPE_ENUM
{
    CSIMA_CBPCA_DATA_REQ                = 0x00000000,       /* CSIMA向CBPCA发送的数据请求 */
    CBPCA_CSIMA_DATA_IND                = 0x00001000,       /* CBPCA向CSIMA转发的数据 */

    CMMCA_CBPCA_DATA_REQ                = 0x00000001,       /* CMMCA向CBPCA发送的数据请求 */
    CBPCA_CMMCA_DATA_IND                = 0x00001001,       /* CBPCA向CMMCA转发的数据 */

    CSIMA_CBPCA_MODEM_RESET_IND         = 0x00001002,       /* CSIMA向CBPCA发送VIA Modem复位状态指示 */

    CBPCA_MTC_DATA_IND                  = 0x00001003,       /* CBPCA向MTC转发的数据 */

    CBPCA_MSG_TYPE_BUTT
};
typedef VOS_UINT32 CBPCA_MSG_TYPE_ENUM_UINT32;

/*****************************************************************************
 枚举名    : CSIMA_CBPCA_MODEM_RESET_ENUM
 枚举说明  : CSIMA向CBPCA发送VIA MODEM 复位状态指示时所用的枚举
*****************************************************************************/
enum CSIMA_CBPCA_MODEM_RESET_ENUM
{
    CSIMA_CBPCA_MODEM_RESET_START       = 0x0000,           /* VIA Modem开始复位 */
    CSIMA_CBPCA_MODEM_RESET_SUCC        = 0x0001,           /* VIA Modem复位成功 */
    CSIMA_CBPCA_MODEM_RESET_FAIL        = 0x0002,           /* VIA Modem复位失败 */

    CSIMA_CBPCA_MODEM_RESET_BUTT
};
typedef VOS_UINT32 CSIMA_CBPCA_MODEM_RESET_ENUM_UINT32;


/*******************************************************************************
  4 结构定义
*******************************************************************************/
/*****************************************************************************
 结构名    : CBPCA_DATA_REQ_MSG_STRU
 结构说明  : CSIMA或CMMCA等上层适配模块向CBPCA发送的数据请求消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    CBPCA_MSG_TYPE_ENUM_UINT32          enMsgType;          /* 消息ID */
    VOS_UINT32                          ulDataLen;          /* 指示aucData的数据长度，不包括aucData中预留的前6个字节 */
    VOS_UINT8                           aucData[CBPCA_MSG_DATA_RSV_LEN];
}CBPCA_DATA_REQ_MSG_STRU;

/*****************************************************************************
 结构名    : CBPCA_DATA_IND_MSG_STRU
 结构说明  : CBPCA向CSIMA或CMMCA等上层适配模块发送的数据消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    CBPCA_MSG_TYPE_ENUM_UINT32          enMsgType;          /* 消息ID */
    VOS_UINT32                          ulDataLen;          /* 指示aucData的数据长度 */
    VOS_UINT8                           aucData[CBPCA_MSG_DATA_RSV_LEN];
}CBPCA_DATA_IND_MSG_STRU;

/*****************************************************************************
 结构名    : CSIMA_CBPCA_MODEM_RESET_IND_MSG_STRU
 结构说明  : CSIMA向CBPCA发送的Modem复位状态指示
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    CBPCA_MSG_TYPE_ENUM_UINT32          enMsgType;          /* 消息ID */
    CSIMA_CBPCA_MODEM_RESET_ENUM_UINT32 enModemReset;       /* 指示Modem复位状态 */
}CSIMA_CBPCA_MODEM_RESET_IND_MSG_STRU;

/*****************************************************************************
  5 全局变量声明
*****************************************************************************/


/*****************************************************************************
  6 函数声明
*****************************************************************************/
extern VOS_UINT32 CBPCA_SndDataToCbpca(
    VOS_UINT32                          ulSndPid,
    CBPCA_MSG_TYPE_ENUM_UINT32          enMsgType,
    VOS_UINT8                          *pucData,
    VOS_UINT32                          ulLength);



#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of CbpaPsInterface.h*/

