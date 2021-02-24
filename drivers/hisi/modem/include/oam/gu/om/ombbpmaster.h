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



#ifndef __OMBBPMASTER_H__
#define __OMBBPMASTER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/

#include "vos.h"

#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define OM_BBPMASTER_DBG_ON                             (0x1234)

#define OM_BBPMASTER_WR_CHANNEL                         (0) /* BBP Masterдͨ�� */
#define OM_BBPMASTER_RD_CHANNEL                         (1) /* BBP Masterдͨ�� */

#define OM_BBPMASTER_IPC_LOCK_IDLE                      (0)
#define OM_BBPMASTER_IPC_LOCK_BUSY                      (9)

#define OM_BBPMASTER_V8R1_WR_CHAN_CNT                   (5) /* �����V8R1 GUX��дͨ����Ŀ */
#define OM_BBPMASTER_V8R1_RD_CHAN_CNT                   (4) /* �����V8R1 GUX�Ķ�ͨ����Ŀ */

#define OM_BBPMASTER_V7R5_WR_CHAN_CNT                   (6) /* �����V7R5 GUX��дͨ����Ŀ */
#define OM_BBPMASTER_V7R5_RD_CHAN_CNT                   (4) /* �����V7R5 GUX�Ķ�ͨ����Ŀ */

#define OM_BBPMASTER_V8R5_WR_CHAN_CNT                   (14) /* �����V8R5 GUX��дͨ����Ŀ */
#define OM_BBPMASTER_V8R5_RD_CHAN_CNT                   (11) /* �����V8R5 GUX�Ķ�ͨ����Ŀ */

#define OM_BBPMASTER_K3V6_WR_CHAN_CNT                   (24) /* �����V8R5 GUX��дͨ����Ŀ */
#define OM_BBPMASTER_K3V6_RD_CHAN_CNT                   (16) /* �����V8R5 GUX�Ķ�ͨ����Ŀ */

#define OM_BBPMASTER_MAX_WR_CHAN_CNT                    OM_BBPMASTER_K3V6_WR_CHAN_CNT /* ���дͨ����Ŀ */
#define OM_BBPMASTER_MAX_RD_CHAN_CNT                    OM_BBPMASTER_K3V6_RD_CHAN_CNT /* ����ͨ����Ŀ */

#define OM_BBP_MASTER_CHAN_REG_OFFSET                   (4)

#define OM_BBP_MASTER_CHAN_REG_WIDTH                    (16)

#define OM_BBPMASTER_MASK_BIT_0                         (1 << 0)
#define OM_BBPMASTER_MASK_BIT_1                         (1 << 1)
#define OM_BBPMASTER_MASK_BIT_2                         (1 << 2)
#define OM_BBPMASTER_MASK_BIT_6                         (1 << 6)
#define OM_BBPMASTER_MASK_BIT_8                         (1 << 8)
#define OM_BBPMASTER_MASK_BIT_16                        (1 << 16)
#define OM_BBPMASTER_MASK_BIT_17                        (1 << 17)
#define OM_BBPMASTER_MASK_BIT_24                        (1 << 24)

#define OM_BBPMASTER_MASK_INTMODE                       (0x30000)

#define OM_IPC_SEM_INT_MASK(i)                          (0x604 + (i * 0x10))                /* �ź����ͷ��ж�����Ĵ��� */
#define OM_IPC_SEM_INT_CLR(i)                           (0x60C + (i * 0x10))                /* �ź����ͷ��ж�����Ĵ��� */
#define OM_IPC_HS_CTRL(i,j)                             (0x800 + (i * 0x100) + (j * 0x8 ))  /* �ź�������Ĵ��� */

#define OM_BBPMASTER_NULL_PTR                           (0)

#define OM_BBPMASTER_WR_MODE_OFFSET                     (0)
#define OM_BBPMASTER_WR_CLK_SEL_OFFSET                  (0x4)
#define OM_BBPMASTER_WR_BASE_ADDR_OFFSET                (0x8)
#define OM_BBPMASTER_WR_END_ADDR_OFFSET                 (0xC)
#define OM_BBPMASTER_WR_BURST_BEAT_OFFSET               (0x10)
#define OM_BBPMASTER_WR_AXI_SEL_OFFSET                  (0x14)
#define OM_BBPMASTER_WR_INTR_MODE_OFFSET                (0x18)

#define OM_BBPMASTER_RD_MODE_OFFSET                     (0)
#define OM_BBPMASTER_RD_CLK_SEL_OFFSET                  (0x4)
#define OM_BBPMASTER_RD_INIT_ADDR_OFFSET                (0x8)
#define OM_BBPMASTER_RD_BURST_BEAT_OFFSET               (0xC)
#define OM_BBPMASTER_RD_AXI_SEL_OFFSET                  (0x10)
#define OM_BBPMASTER_RD_INTR_MODE_OFFSET                (0x14)
#define OM_BBPMASTER_RD_BIT_WIDTH_IND_OFFSET            (0x18)
#if ( VOS_WIN32 == VOS_OS_VER )
#define IPC_SEM_BBPMASTER_0                             (10)    /* IPC��Դ��10������дͨ��0���Ᵽ�� */
#define IPC_SEM_BBPMASTER_5                             (15)    /* IPC��Դ��15�����ڶ�ͨ��0���Ᵽ�� */
#endif

#ifdef CSDR_FEATURE_ON
#define IPC_SEM_BBPMASTER_0                             (10)    /* IPC��Դ��10������дͨ��0���Ᵽ�� */

#define IPC_SEM_BBPMASTER_5                             (15)    /* IPC��Դ��15�����ڶ�ͨ��0���Ᵽ�� */

#define BBE16_PID_RESERVED                              (19)

#define IPC_CORE_COMARM                                 (1)

#define OM_BBPMASTER_READ_REG( M_uwCbbpAddr )               CSDR_CbbpReg32Rd( M_uwCbbpAddr )
#define OM_BBPMASTER_WRITE_REG( M_uwCbbpAddr, M_uwValue )   CSDR_CbbpReg32Wr( M_uwCbbpAddr, M_uwValue )

#define OM_BBPMASTER_YES                                (1)
#define OM_BBPMASTER_NO                                 (0)

#define OM_BBPMASTER_OK                                 (0)

typedef PHY_INT                                         OM_BBPMASTER_INT;
typedef PHY_UINT32                                      OM_BBPMASTER_UINT32;
typedef PHY_VOID                                        OM_BBPMASTER_VOID;
typedef CSDR_VOID_ISR_FUN                               VOS_VOIDFUNCPTR;
typedef PHY_UINT32                                      OM_BBPMASTER_CFG_DECFG_TYPE_ENUM_UINT32;

#else

#define OM_BBPMASTER_READ_REG( ulRegAddr )                  HPA_Read32Reg( ulRegAddr )
#define OM_BBPMASTER_WRITE_REG( ulRegAddr, ulRegVal )       HPA_Write32Reg( ulRegAddr, ulRegVal )

#define OM_BBPMASTER_YES                                VOS_YES
#define OM_BBPMASTER_NO                                 VOS_NO

#define OM_BBPMASTER_OK                                 VOS_OK

typedef VOS_INT                                         OM_BBPMASTER_INT;
typedef VOS_UINT32                                      OM_BBPMASTER_UINT32;
typedef VOS_PID                                         OM_BBPMASTER_UINT32;
typedef VOS_VOID                                        OM_BBPMASTER_VOID;
typedef VOS_UINT32                                      OM_BBPMASTER_CFG_DECFG_TYPE_ENUM_UINT32;

#endif

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
enum OM_BBPMASTER_CFG_DECFG_TYPE_ENUM
{
    OM_BBPMASTER_CONFIG_NONE                = 0,    /* ����ͨ���ӿڷ���ֵö��: δ���� */
    OM_BBPMASTER_CONFIG_SUCC                = 1,    /* ʹ������ͨ���ӿڷ���ֵö��: ���óɹ� */
    OM_BBPMASTER_PARA_CHANID_INVALID        = 2,    /* ʹ������ͨ���ӿڷ���ֵö��: ����ͨ������Ч */
    OM_BBPMASTER_PARA_CFG_NULL              = 3,    /* ʹ������ͨ���ӿڷ���ֵö��: ���ò���Ϊ��ָ�� */
    OM_BBPMASTER_LOCK_IPC_FAIL              = 4,    /* ʹ������ͨ���ӿڷ���ֵö��: ��ȡIPC��Դ��ʧ�� */
    OM_BBPMASTER_DECFG_SUCC                 = 5,    /* ȥʹ������ͨ���ӿڷ���ֵö��: ���óɹ� */
    OM_BBPMASTER_NOT_SUPPORT                = 6,    /* ��֧��BBP Master����ӿ�(��V7R5/V8RX/K3V5��֧��) */
    OM_BBPMASTER_CFG_DECFG_TYPE_BUTT
};

enum OM_BBPMASTER_IP_TYPE_ENUM
{
     OM_BBPMASTER_IP_V8R1                    = 0,    /* V8R1 BBP Master IP */
     OM_BBPMASTER_IP_V7R5,                           /* V7R5 BBP Master IP */
     OM_BBPMASTER_IP_V8R5,                           /* V8R5 BBP Master IP */
     OM_BBPMASTER_IP_K3V6,                           /* K3V5 BBP Master IP */
     OM_BBPMASTER_NO_IP                              /* ��֧��BBP Master IP */
};

enum OM_BBPMASTER_CHAN_STATE_ENUM
{
    OM_BBPMASTER_CHAN_STATE_IDLE            = 0,    /* ͨ��״̬:δʹ��״̬ */
    OM_BBPMASTER_CHAN_STATE_BUSY            = 1,    /* ͨ��״̬:��ʹ��״̬ */
    OM_BBPMASTER_CHAN_STATE_BUTT
};

#ifdef CSDR_FEATURE_ON
typedef unsigned long OM_BBPMASTER_CHAN_STATE_ENUM_UINT32;
#else
typedef VOS_UINT32 OM_BBPMASTER_CHAN_STATE_ENUM_UINT32;
#endif
/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  7 STRUCT����
*****************************************************************************/
typedef struct
{
    OM_BBPMASTER_UINT32                         ulShared;       /* ͨ���Ƿ��� */
    VOS_VOIDFUNCPTR                             pfnIntFunc;
}BBPMASTER_ENTITY_STRU;

typedef struct
{
    OM_BBPMASTER_UINT32                         ulWorkMode;     /* ʹ��дͨ�����ò���:����ģʽ */
    OM_BBPMASTER_UINT32                         ulClockSel;     /* ʹ��дͨ�����ò���:ʱ��ѡ��; V7R5��Ч��V8R1��Ч */
    OM_BBPMASTER_UINT32                         ulStartAddr;    /* ʹ��дͨ�����ò���:дͨ����ʼ��ַ */
    OM_BBPMASTER_UINT32                         ulEndAddr;      /* ʹ��дͨ�����ò���:дͨ����ֹ��ַ */
    OM_BBPMASTER_UINT32                         ulBurstLen;     /* ʹ��дͨ�����ò���:Burst�������� */
    OM_BBPMASTER_UINT32                         ulAxiSel;       /* ʹ��дͨ�����ò���:AXIͨ��ѡ�� */
    OM_BBPMASTER_UINT32                         ulIntMode;      /* �ж�ģʽ */
    VOS_VOIDFUNCPTR                             pfnIntFunc;     /* ��Ҫ�ҽӵ��жϴ����� */
}BBPMASTER_WR_CHANNEL_CONFIG_STRU;

typedef struct
{
    OM_BBPMASTER_UINT32                         ulWorkModeAddr; /* дͨ�����ò���:����ģʽ���üĴ�����ʹ�ܡ�ȥʹ�ܽӿڸ��� */
    OM_BBPMASTER_UINT32                         ulClockSelAddr; /* дͨ�����ò���:ʱ��ѡ�����üĴ�����ʹ�ܡ�ȥʹ�ܽӿڸ��� */
    OM_BBPMASTER_UINT32                         ulStartAddr;    /* дͨ�����ò���:дͨ����ʼ��ַ���üĴ�����ʹ�ܡ�ȥʹ�ܽӿڸ��� */
    OM_BBPMASTER_UINT32                         ulEndAddr;      /* дͨ�����ò���:дͨ����ֹ��ַ���üĴ�����ʹ�ܡ�ȥʹ�ܽӿڸ��� */
    OM_BBPMASTER_UINT32                         ulBurstLenAddr; /* дͨ�����ò���:Burst�����������üĴ�����ʹ�ܡ�ȥʹ�ܽӿڸ��� */
    OM_BBPMASTER_UINT32                         ulAxiSelAddr;   /* дͨ�����ò���:AXIͨ��ѡ�����üĴ�����ʹ�ܡ�ȥʹ�ܽӿڸ��� */
    OM_BBPMASTER_UINT32                         ulIntModeAddr;  /* дͨ�����ò���:�ж�ģʽ�Ĵ��� */
}BBPMASTER_WR_CHANNEL_CONFIG_ADDR_STRU;

typedef struct
{
    OM_BBPMASTER_UINT32                         ulWorkMode;     /* ȥʹ��ͨ�����ò���:����ģʽ���üĴ��� */
    OM_BBPMASTER_UINT32                         ulClockSel;     /* ȥʹ��ͨ�����ò���:ʱ��ѡ�����üĴ��� */
}BBPMASTER_CHANNEL_DECONFIG_STRU;

typedef struct
{
    OM_BBPMASTER_UINT32                         ulWorkMode;     /* ʹ�ܶ�ͨ�����ò���:����ģʽ */
    OM_BBPMASTER_UINT32                         ulClockSel;     /* ʹ�ܶ�ͨ�����ò���:ʱ��ѡ�� V7R5��Ч��V8R1��Ч */
    OM_BBPMASTER_UINT32                         ulStartAddr;    /* ʹ�ܶ�ͨ�����ò���:��ͨ����ʼ��ַ */
    OM_BBPMASTER_UINT32                         ulBurstLen;     /* ͨ�����ò���:Burst�������� */
    OM_BBPMASTER_UINT32                         ulAxiSel;       /* ʹ�ܶ�ͨ�����ò���:AXIͨ��ѡ�� */
    OM_BBPMASTER_UINT32                         ulIntMode;      /* �ж�ģʽ */
    OM_BBPMASTER_UINT32                         ulValidLen;     /* ʹ�ܶ�ͨ�����ò���:��ͨ�� V7R5��Ч��V8R1��Ч  */
    VOS_VOIDFUNCPTR                             pfnIntFunc;     /* ��Ҫ�ҽӵ��жϴ����� */
}BBPMASTER_RD_CHANNEL_CONFIG_STRU;

typedef struct
{
    OM_BBPMASTER_UINT32                         ulWorkModeAddr; /* ��ͨ�����ò���:����ģʽ���üĴ�����ʹ�ܡ�ȥʹ�ܽӿڸ��� */
    OM_BBPMASTER_UINT32                         ulClockSelAddr; /* ��ͨ�����ò���:ʱ��ѡ�����üĴ�����ʹ�ܡ�ȥʹ�ܽӿڸ��� */
    OM_BBPMASTER_UINT32                         ulStartAddr;    /* ��ͨ�����ò���:��ͨ����ʼ��ַ���üĴ�����ʹ�ܡ�ȥʹ�ܽӿڸ��� */
    OM_BBPMASTER_UINT32                         ulBurstLenAddr; /* ��ͨ�����ò���:Burst�����������üĴ�����ʹ�ܡ�ȥʹ�ܽӿڸ��� */
    OM_BBPMASTER_UINT32                         ulAxiSelAddr;   /* ��ͨ�����ò���:AXIͨ��ѡ�����üĴ�����ʹ�ܡ�ȥʹ�ܽӿڸ��� */
    OM_BBPMASTER_UINT32                         ulValidLenAddr; /* ��Ч�������õ�ַ */
    OM_BBPMASTER_UINT32                         ulIntModeAddr;  /* �ж�ģʽ */
}BBPMASTER_RD_CHANNEL_CONFIG_ADDR_STRU;

/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/

/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
extern OM_BBPMASTER_UINT32 OM_BbpMasterInit( OM_BBPMASTER_VOID );

extern OM_BBPMASTER_CFG_DECFG_TYPE_ENUM_UINT32 OM_BbpMasterRdChannelConfig(OM_BBPMASTER_UINT32 ulPID,
                                                               OM_BBPMASTER_UINT32 ulChanId,
                                                               BBPMASTER_RD_CHANNEL_CONFIG_STRU *pstRdChanCfg);

extern OM_BBPMASTER_CFG_DECFG_TYPE_ENUM_UINT32 OM_BbpMasterRdChannelDeConfig(OM_BBPMASTER_UINT32 ulPID,
                                                               OM_BBPMASTER_UINT32 ulChanId,
                                                               BBPMASTER_CHANNEL_DECONFIG_STRU *pstChanDeCfg);

extern OM_BBPMASTER_CFG_DECFG_TYPE_ENUM_UINT32 OM_BbpMasterWrChannelConfig(OM_BBPMASTER_UINT32 ulPID,
                                                               OM_BBPMASTER_UINT32 ulChanId,
                                                               BBPMASTER_WR_CHANNEL_CONFIG_STRU *pstWrChanCfg);
extern OM_BBPMASTER_CFG_DECFG_TYPE_ENUM_UINT32 OM_BbpMasterWrChannelDeConfig(OM_BBPMASTER_UINT32 ulPID,
                                                               OM_BBPMASTER_UINT32 ulChanId,
                                                               BBPMASTER_CHANNEL_DECONFIG_STRU *pstChanDeCfg);

extern OM_BBPMASTER_UINT32 OM_BbpMasterGetIpInfo(OM_BBPMASTER_VOID);

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

#endif /* end of omnosig.h */

