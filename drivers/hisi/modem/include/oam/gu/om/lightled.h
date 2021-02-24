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
/*                                                                           */
/*                Copyright 1999 - 2003, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileName: LightLED.h                                                      */
/*                                                                           */
/* Author: Xu cheng                                                          */
/*                                                                           */
/* Version: 1.0                                                              */
/*                                                                           */
/* Date: 2008-06                                                             */
/*                                                                           */
/* Description: Turn on LED according to some Events which be supported by PS*/
/*                                                                           */
/* Others:                                                                   */
/*                                                                           */
/* History:                                                                  */
/* 1. Date: 2008-06                                                          */
/*    Author: Xu cheng                                                       */
/*    Modification: Create this file                                         */
/*                                                                           */

/*****************************************************************************/

#ifndef  _LIGHT_LED_H
#define  _LIGHT_LED_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#include "vos.h"
#include "NasOmInterface.h"
#include "WasOmInterface.h"

enum
{
    OAM_LED_STATE_CNF = 0x0001,
    OAM_LED_STATE_BUTT
};

#if(FEATURE_ON == MBB_FEATURE_M2M_LED)
/* 模块通用规格的闪灯服务状态 */
enum M2M_LED_STATUS
{
    M2M_LED_FLIGHT_MODE,                /* 飞行模式 */
    M2M_LED_SYSTEM_INITIAL,             /* 上电初始化 */
    M2M_LED_OFFLINE_UNREGISTERED,       /* 注册失败，无服务 */
    M2M_LED_OFFLINE_NO_SERVICE,         /* 掉网，无服务 */
    M2M_LED_GSM_REGISTERED,             /* GSM注册成功 */
    M2M_LED_GSM_BEAR_SETUP,             /* GSM下无线承载建立成功 */
    M2M_LED_GSM_TRANSMITING,            /* GSM下数传 */
    M2M_LED_WCDMA_REGISTERED,           /* WCDMA注册成功 */
    M2M_LED_WCDMA_BEAR_SETUP,           /* WCDMA下无线承载建立成功 */
    M2M_LED_WCDMA_TRANSMITING,          /* WCDMA下数传 */
    M2M_LED_HSPA_BEAR_SETUP,            /* HSPA下无线承载建立成功 */
    M2M_LED_HSPA_TRANSMITING,           /* HSPA下数传 */
    M2M_LED_LTE_REGISTERED,             /* LTE注册成功 */
    M2M_LED_LTE_BEAR_SETUP,             /* LTE下无线承载建立成功 */
    M2M_LED_LTE_TRANSMITING,            /* LTE下数传 */    
    M2M_LED_NORMAL_UPDATING,            /* 升级状态 */
    
    M2M_LED_STATE_MAX = 32
}; /*the state of M2M LED */
/* 模块定制闪灯服务状态 */
enum CE_LED_STATUS
{
    LED_RF_OFF          = 0,            /* 关闭RF */
    LED_RF_ON        = 1,            /* 开启RF*/
  
    CE_LED_STATE_MAX = 32,
}; /*the state of CE LED */
#endif

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                              ulMsgName;
    VOS_INT                                 lOldLedState;
    VOS_INT                                 lNewLedState;
    WAS_MNTN_OM_OUT_OF_SERVICE_ENUM_UINT8   enIsOutService;
    NAS_OM_REGISTER_STATE_ENUM_UINT8        enRegState;
    NAS_OM_SERVICE_TYPE_ENUM_UINT8          enServiceType;
    VOS_UINT8                               ucCardStatus;
    VOS_UINT32                              ulRatType;
}LED_STATE_STRUCT;

#if(FEATURE_ON == MBB_FEATURE_M2M_LED)
extern VOS_UINT8 NAS_MMA_GetPhoneMode(VOS_VOID);
#endif
#if (FEATURE_ON == MBB_WPG_LED_FLASH)
enum
{
    DEF = 0,                /*0:默认闪灯枚举*/
    HONGKONG,               /*1:香港PCCW闪灯枚举*/
    RUSSIA,                 /*2:俄罗斯MTS闪灯枚举*/
    VODAFONE,               /*3:vodafone闪灯枚举*/   
    CHINAUNION,             /*4:中国联通闪灯枚举*/
                            /*5:预留值*/
    SFR = 6,                /*6:法国SFR闪灯枚举*/
    BSNL,                   /*7:印度BSNL闪灯枚举*/
    KPN,                    /*8:荷兰KPN闪灯枚举*/
    SOFTBANK,               /*9:日本软银闪灯枚举*/
    EMOBILE                /*10:日本Emobile闪灯枚举*/
};

extern VOS_UINT32    g_ulLEDStatus;
extern VOS_UINT32    g_ulNetSearchStatus;

extern VOS_UINT32 OM_GetLedAsSearchStatus(VOS_VOID);
extern VOS_UINT32 OM_GetLedNasSearchStatus(VOS_VOID);
#endif/*FEATURE_ON == MBB_WPG_LED_FLASH*/

extern VOS_UINT32 OM_TraceMsgHook(VOS_VOID *pMsg);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* _LIGHT_LED_H */

