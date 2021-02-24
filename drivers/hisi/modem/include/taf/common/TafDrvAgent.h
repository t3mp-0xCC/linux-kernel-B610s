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

#ifndef __ATAGENT_H__
#define __ATAGENT_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include  "vos.h"

#include "AtMnInterface.h"
#include "TafAppMma.h"
#include "mdrv.h"

#include "AtPhyInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/

#define AT_MAX_VERSION_LEN              (128)                 /* �汾��Ϣ��󳤶� */


#define AT_MAX_PDM_VER_LEN              (16)                                    /* PDM�汾�ŵ���󳤶� */

#define TAF_DLOAD_INFO_LEN              (1024)

#define TAF_AUTH_ID_LEN                 (50)

#define AT_DLOAD_TASK_DELAY_TIME        (500)

#define TAF_MAX_VER_INFO_LEN            (6)

#define TAF_MAX_PROPLAT_LEN             (63)                                    /*���ƽ̨����汾����*/

#define TAF_AUTHORITY_LEN               (50)                                    /* AUTHORITYVER�������������� */

#define AT_AGENT_DRV_VERSION_TIME_LEN   (32)                                    /* ����ӿ�DRV_GET_VERSION_TIME����ʱ���ַ�����󳤶� */

#define TAF_MAX_FLAFH_INFO_LEN          (255)

#define TAF_MAX_GPIO_INFO_LEN           (32)

/*Ӳ���汾����󳤶�*/
#define DRV_AGENT_MAX_MODEL_ID_LEN      (31)
#define DRV_AGENT_MAX_REVISION_ID_LEN   (31)

/* MS֧�ֵĽ��շּ����Ͷ��� */
#define DRV_AGENT_MS_SUPPORT_RX_DIV_2100           (0x0001)
#define DRV_AGENT_MS_SUPPORT_RX_DIV_1900           (0x0002)
#define DRV_AGENT_MS_SUPPORT_RX_DIV_1800           (0x0004)
#define DRV_AGENT_MS_SUPPORT_RX_DIV_AWS_1700       (0x0008)
#define DRV_AGENT_MS_SUPPORT_RX_DIV_850            (0x0010)
#define DRV_AGENT_MS_SUPPORT_RX_DIV_800            (0x0020)
#define DRV_AGENT_MS_SUPPORT_RX_DIV_2600           (0x0040)
#define DRV_AGENT_MS_SUPPORT_RX_DIV_900            (0x0080)
#define DRV_AGENT_MS_SUPPORT_RX_DIV_IX_1700        (0x0100)

#define DRV_AGENT_DSP_RF_SWITCH_ON      (1)

#define DRV_AGENT_SIMLOCK_MAX_ERROR_TIMES  (3)  /* Simlock�������ʧ�ܴ��� */

#define  AT_PID_MEM_INFO_LEN                        (150) /* VOS�ڴ��ѯʱ֧�ֵ����PID���� */

#define  AT_KB_TO_BYTES_NUM                         (1024) /* KB���ֽ�ת�����1024 */



#if (FEATURE_ON == FEATURE_SECURITY_SHELL)
/* ��ǿSHELL�����뱣����ض��� */
#define AT_SHELL_PWD_LEN                (16)
#endif

/* �趨һ������ֵ100�����ǵ�PAD ��NANDΪ256MB������block sizeΪ128kB������2048�飬
   ������һ��Ҫ����5%��Ϊ�Ǻϸ��FLASH����2048*5%=102.4��ȡ100Ϊ����������ޣ���
   ���������������100������ֻ���ǰ100����������������������л���������
   �����ص�<totalNum>�԰���ʵ���ܿ��������
 */
#define DRV_AGENT_NAND_BADBLOCK_MAX_NUM (100)

#define DRV_AGENT_CHIP_TEMP_ERR         (65535)

#define DRV_AGENT_HUK_LEN                       (16)        /* HUKΪ128Bits������, ����Ϊ16Bytes */
#define DRV_AGENT_PUBKEY_LEN                    (520)       /* ��Ȩ��Կ����, ����Ϊ520Bytes */
#define DRV_AGENT_PUBKEY_SIGNATURE_LEN          (32)        /* ��Ȩ��ԿSSKǩ������ */
#define DRV_AGENT_RSA_CIPHERTEXT_LEN            (128)       /* RSA���ĳ���, ͳһΪ128Bytes */
#define DRV_AGENT_SUPPORT_CATEGORY_NUM          (3)         /* ֧�ֵ���������CATEGORY�������Ŀǰֻ֧������:network/network subset/SP */
#define DRV_AGENT_PH_LOCK_CODE_GROUP_NUM        (20)        /* ���������Ŷ����� */
#define DRV_AGENT_PH_LOCK_CODE_LEN              (4)         /* ���������Ŷγ��� */
#define DRV_AGENT_SIMLOCKDATAWRITE_STRU_LEN     (548)       /* ��������������������ṹ�峤�� */
#define DRV_AGENT_PH_PHYNUM_LEN                 (16)        /* ����ų��� */
#define DRV_AGENT_PH_PHYNUM_IMEI_LEN            (15)        /* IMEI�ų��� */
#define DRV_AGENT_PH_PHYNUM_IMEI_NV_LEN         (16)        /* IMEI��NV��� */
#define DRV_AGENT_PH_PHYNUM_SN_LEN              (16)        /* SN���� */
#define DRV_AGENT_PH_PHYNUM_SN_NV_LEN           (DRV_AGENT_PH_PHYNUM_SN_LEN+4)  /* SN��NV��� */
#define DRV_AGENT_PORT_PASSWORD_LEN             (16)        /* ����ͨ�Ŷ˿������볤�� */

#define DRV_AGENT_DH_PUBLICKEY_LEN              (128)       /* DH�㷨ʹ�õķ�������Կ��Modem�๫Կ���� */
#define DRV_AGENT_DH_PRIVATEKEY_LEN             (48)        /* DH�㷨ʹ�õ�Modem��˽Կ���� */

#if (FEATURE_ON == MBB_SIMLOCK_FOUR)
#define AT_TAF_HWLOCKPARALEN    17
#define AT_TAF_HWLOCKOEMTYPE    1
#define AT_TAF_HWLOCKSIMLOCKTYPE 2
#endif

#if (FEATURE_ON == MBB_ANTENNA_TUNER)
/* �յ�maskλ����ʾδʹ�ô�maskλ */
#define ANTENCFG_NULL_BAND_MASK           0x00000000
/* ����֧�ֵ�Ƶ�� */
#define ANTENCFG_ALL_BAND_MASK            0xFFFFFFFFu

/* Ƶ��GPIOֻ���õ�4λ����4λ��Ԥ���� */
#define AT_ANTENCFG_PATTERN_MAX           0x0Fu

/* ��г����ĳ��ʽ�µ�������Ƶ�ε�������� */
#define ANTENCFG_BAND_CFG_MAX_GROUP       7

/* ��г��������һ��Ƶ��������Ĳ������� */
#define ANTENCFG_GROUP_ITEM_NUM           2

/* ��г��������������Ҫ2������ */
#define AT_ANTENCFG_PARAMETER_2           2

/* ��г����TEST�����ַ�����󳤶� */
#define AT_ANTENCFG_TEST_STRING_LEN       128

/* 
   ���¶�����Ҫ���ݲ�ƷӲ����ƶ���
   ME906S: ANT_CTL0/ANT_CTL1/ANT_CTL2/ANT_CTL3�ֱ��ӦANTPA_SEL13/ANTPA_SEL14/ANTPA_SEL15/ANTPA_SEL16
*/
#define AT_ANTENCFG_RF_GPIO_INDEX_ME906S  13

/* 
   NV10000(en_NV_Item_WG_RF_MAIN_BAND.ulGeBand)����GSMƵ�ε�֧�����
   ��bitλ�����������α�ʾGSM450��GSM480��GSM850��P900��R900��E900��DCS1800��PCS1900
*/
#define AT_ANTENCFG_GSM850_BOARD_SUPPORTED  0x04 /* 0x04��ʾ֧��GSM850 */
#define AT_ANTENCFG_GSM900_BOARD_SUPPORTED  0x38 /* 0x38��ʾ֧��GSM900 */
#define AT_ANTENCFG_GSM1800_BOARD_SUPPORTED 0x40 /* 0x40��ʾ֧��DCS1800 */
#define AT_ANTENCFG_GSM1900_BOARD_SUPPORTED 0x80 /* 0x80��ʾ֧��PCS1900 */

/* ����LTEB28����̫����V711ƽ̨��ʵ�ʷ�ΪB28L��B28H����Ƶ�Σ���NV�����У�B28L��B28, B28Hʹ��B128���� */
#define AT_ANTENCFG_LTE_B128_INDEX 128

/* TDSCDMA Band A��B34; Band F��B39 */
#define AT_ANTENCFG_TDSCDMA_BAND_A_INDEX    34
#endif /* MBB_ANTENNA_TUNER */

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

/*****************************************************************************
 ö����    : DRV_AGENT_TSELRF_SET_ERROR_ENUM
 �ṹ˵��  : ^TSELRF �������ò�������������б�
             0: DRV_AGENT_TSELRF_SET_ERROR_NO_ERROR ���ò����ɹ���
             1: DRV_AGENT_TSELRF_SET_ERROR_LOADDSP  ���ò���ʧ�ܣ�
*****************************************************************************/
enum DRV_AGENT_TSELRF_SET_ERROR_ENUM
{
    DRV_AGENT_TSELRF_SET_NO_ERROR      = 0,
    DRV_AGENT_TSELRF_SET_LOADDSP_FAIL,

    DRV_AGENT_TSELRF_SET_BUTT
};
typedef VOS_UINT32 DRV_AGENT_TSELRF_SET_ERROR_ENUM_UINT32;


/*****************************************************************************
 ö����    : DRV_AGENT_HKADC_GET_ERROR_ENUM
 �ṹ˵��  : ��ȡHKADC��ѹ��������������б�
             0: DRV_AGENT_HKADC_GET_NO_ERROR�����ɹ���
             1: DRV_AGENT_HKADC_GET_FAIL         ����ʧ�ܣ�
*****************************************************************************/
enum DRV_AGENT_HKADC_GET_ERROR_ENUM
{
    DRV_AGENT_HKADC_GET_NO_ERROR      = 0,
    DRV_AGENT_HKADC_GET_FAIL,

    DRV_AGENT_HKADC_GET_BUTT
};
typedef VOS_UINT32 DRV_AGENT_HKADC_GET_ERROR_ENUM_UINT32;

/*****************************************************************************
 ö����    : DRV_AGENT_USIMWRITESTUB_SET_ERROR_ENUM
 �ṹ˵��  : USIMWRITESTUB �������ò�������������б�
             0: ���ò����ɹ���
             1: ���ò���ʧ�ܣ�
*****************************************************************************/
enum DRV_AGENT_ERROR_ENUM
{
    DRV_AGENT_NO_ERROR      = 0,
    DRV_AGENT_ERROR,
    DRV_AGENT_PARAM_ERROR,
    DRV_AGENT_CME_ERROR,
    DRV_AGENT_CME_RX_DIV_OTHER_ERR,
    DRV_AGENT_CME_RX_DIV_NOT_SUPPORTED,
    DRV_AGENT_CME_RX_DIV_BAND_ERR,

    DRV_AGENT_BUTT
};
typedef VOS_UINT32 DRV_AGENT_ERROR_ENUM_UINT32;



enum DRV_AGENT_MSID_QRY_ERROR_ENUM
{
    DRV_AGENT_MSID_QRY_NO_ERROR                  = 0,
    DRV_AGENT_MSID_QRY_READ_PRODUCT_ID_ERROR     = 1,
    DRV_AGENT_MSID_QRY_READ_SOFT_VER_ERROR       = 2,
    DRV_AGENT_MSID_QRY_BUTT
};
typedef VOS_UINT32  DRV_AGENT_MSID_QRY_ERROR_ENUM_UINT32;


enum DRV_AGENT_APPDMVER_QRY_ERROR_ENUM
{
    DRV_AGENT_APPDMVER_QRY_NO_ERROR      = 0,
    DRV_AGENT_APPDMVER_QRY_ERROR         = 1,

    DRV_AGENT_APPDMVER_QRY_BUTT
};
typedef VOS_UINT32 DRV_AGENT_APPDMVER_QRY_ERROR_ENUM_UINT32;


enum DRV_AGENT_DLOADINFO_QRY_ERROR_ENUM
{
    DRV_AGENT_DLOADINFO_QRY_NO_ERROR      = 0,
    DRV_AGENT_DLOADINFO_QRY_ERROR         = 1,

    DRV_AGENT_DLOADINFO_QRY_BUTT
};
typedef VOS_UINT32 DRV_AGENT_DLOADINFO_QRY_ERROR_ENUM_UINT32;


enum DRV_AGENT_AUTHORITYVER_QRY_ERROR_ENUM
{
    DRV_AGENT_AUTHORITYVER_QRY_NO_ERROR      = 0,
    DRV_AGENT_AUTHORITYVER_QRY_ERROR         = 1,

    DRV_AGENT_AUTHORITYVER_QRY_BUTT
};
typedef VOS_UINT32 DRV_AGENT_AUTHORITYVER_QRY_ERROR_ENUM_UINT32;


enum DRV_AGENT_AUTHORITYID_QRY_ERROR_ENUM
{
    DRV_AGENT_AUTHORITYID_QRY_NO_ERROR      = 0,
    DRV_AGENT_AUTHORITYID_QRY_ERROR         = 1,

    DRV_AGENT_AUTHORITYID_QRY_BUTT
};
typedef VOS_UINT32 DRV_AGENT_AUTHORITYID_QRY_ERROR_ENUM_UINT32;


enum DRV_AGENT_GODLOAD_SET_ERROR_ENUM
{
    DRV_AGENT_GODLOAD_SET_NO_ERROR      = 0,
    DRV_AGENT_GODLOAD_SET_ERROR         = 1,

    DRV_AGENT_GODLOAD_SET_BUTT
};
typedef VOS_UINT32 DRV_AGENT_GODLOAD_SET_ERROR_ENUM_UINT32;


enum DRV_AGENT_SDLOAD_SET_ERROR_ENUM
{
    DRV_AGENT_SDLOAD_SET_NO_ERROR      = 0,
    DRV_AGENT_SDLOAD_SET_ERROR         = 1,

    DRV_AGENT_SDLOAD_SET_BUTT
};
typedef VOS_UINT32 DRV_AGENT_SDLOAD_SET_ERROR_ENUM_UINT32;


enum DRV_AGENT_HWNATQRY_QRY_ERROR_ENUM
{
    DRV_AGENT_HWNATQRY_QRY_NO_ERROR      = 0,
    DRV_AGENT_HWNATQRY_QRY_ERROR         = 1,

    DRV_AGENT_HWNATQRY_QRY_BUTT
};
typedef VOS_UINT32 DRV_AGENT_HWNATQRY_QRY_ERROR_ENUM_UINT32;


enum DRV_AGENT_CPULOAD_QRY_ERROR_ENUM
{
    DRV_AGENT_CPULOAD_QRY_NO_ERROR      = 0,
    DRV_AGENT_CPULOAD_QRY_ERROR         = 1,

    DRV_AGENT_CPULOAD_QRY_BUTT
};
typedef VOS_UINT32 DRV_AGENT_CPULOAD_QRY_ERROR_ENUM_UINT32;


enum DRV_AGENT_MFREELOCKSIZE_QRY_ERROR_ENUM
{
    DRV_AGENT_MFREELOCKSIZE_QRY_NO_ERROR      = 0,
    DRV_AGENT_MFREELOCKSIZE_QRY_ERROR         = 1,

    DRV_AGENT_MFREELOCKSIZE_QRY_BUTT
};
typedef VOS_UINT32 DRV_AGENT_MFREELOCKSIZE_QRY_ERROR_ENUM_UINT32;



enum DRV_AGENT_AUTHVER_QRY_ERROR_ENUM
{
    DRV_AGENT_AUTHVER_QRY_NO_ERROR      = 0,
    DRV_AGENT_AUTHVER_QRY_ERROR         = 1,

    DRV_AGENT_AUTHVER_QRY_BUTT
};
typedef VOS_UINT32 DRV_AGENT_AUTHVER_QRY_ERROR_ENUM_UINT32;


enum DRV_AGENT_FLASHINFO_QRY_ERROR_ENUM
{
    DRV_AGENT_FLASHINFO_QRY_NO_ERROR      = 0,
    DRV_AGENT_FLASHINFO_QRY_ERROR         = 1,

    DRV_AGENT_FLASHINFO_QRY_BUTT
};
typedef VOS_UINT32 DRV_AGENT_FLASHINFO_QRY_ERROR_ENUM_UINT32;


enum DRV_AGENT_PFVER_QRY_ERROR_ENUM
{
    DRV_AGENT_PFVER_QRY_NO_ERROR                 = 0,
    DRV_AGENT_PFVER_QRY_ERROR_GET_VERSION_TIME   = 1,
    DRV_AGENT_PFVER_QRY_ERROR_GET_VERSION        = 2,
    DRV_AGENT_PFVER_QRY_ERROR_ALLOC_MEM          = 3,

    DRV_AGENT_PFVER_QRY_BUTT
};
typedef VOS_UINT32 DRV_AGENT_PFVER_QRY_ERROR_ENUM_UINT32;

/*****************************************************************************
 ö����    : DRV_AGENT_DLOADVER_QRY_ERROR_ENUM
 �ṹ˵��  : DLOADVER �����ѯ��������������б�
             0: ��ѯ�����ɹ���
             1: ��ѯ����ʧ�ܣ�
*****************************************************************************/
enum DRV_AGENT_DLOADVER_QRY_ERROR_ENUM
{
    DRV_AGENT_DLOADVER_QRY_NO_ERROR      = 0,
    DRV_AGENT_DLOADVER_QRY_ERROR         = 1,

    DRV_AGENT_DLOADVER_QRY_BUTT
};
typedef VOS_UINT32 DRV_AGENT_DLOADVER_QRY_ERROR_ENUM_UINT32;



enum DRV_AGENT_PERSONALIZATION_ERR_ENUM
{
    DRV_AGENT_PERSONALIZATION_NO_ERROR         = 0,             /* �����ɹ� */
    DRV_AGENT_PERSONALIZATION_IDENTIFY_FAIL,                    /* ���߼�Ȩʧ�� */
    DRV_AGENT_PERSONALIZATION_SIGNATURE_FAIL,                   /* ǩ��У��ʧ�� */
    DRV_AGENT_PERSONALIZATION_DK_INCORRECT,                     /* �˿�������� */
    DRV_AGENT_PERSONALIZATION_PH_PHYNUM_LEN_ERROR,              /* ����ų��ȴ��� */
    DRV_AGENT_PERSONALIZATION_PH_PHYNUM_VALUE_ERROR,            /* ���������� */
    DRV_AGENT_PERSONALIZATION_PH_PHYNUM_TYPE_ERROR,             /* ��������ʹ��� */
    DRV_AGENT_PERSONALIZATION_RSA_ENCRYPT_FAIL,                 /* RSA����ʧ�� */
    DRV_AGENT_PERSONALIZATION_RSA_DECRYPT_FAIL,                 /* RSA����ʧ�� */
    DRV_AGENT_PERSONALIZATION_GET_RAND_NUMBER_FAIL,             /* ��ȡ�����ʧ��(crypto_rand) */
    DRV_AGENT_PERSONALIZATION_WRITE_HUK_FAIL,                   /* HUKд����� */
    DRV_AGENT_PERSONALIZATION_FLASH_ERROR,                      /* Flash���� */
    DRV_AGENT_PERSONALIZATION_OTHER_ERROR,                      /* �������� */

    DRV_AGENT_PERSONALIZATION_ERR_BUTT
};
typedef VOS_UINT32 DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32;


enum DRV_AGENT_PERSONALIZATION_CATEGORY_ENUM
{
    DRV_AGENT_PERSONALIZATION_CATEGORY_NETWORK                 = 0x00,          /* category:���� */
    DRV_AGENT_PERSONALIZATION_CATEGORY_NETWORK_SUBSET          = 0x01,          /* category:������ */
    DRV_AGENT_PERSONALIZATION_CATEGORY_SERVICE_PROVIDER        = 0x02,          /* category:��SP */
    DRV_AGENT_PERSONALIZATION_CATEGORY_CORPORATE               = 0x03,          /* category:������ */
    DRV_AGENT_PERSONALIZATION_CATEGORY_SIM_USIM                = 0x04,          /* category:��(U)SIM�� */

    DRV_AGENT_PERSONALIZATION_CATEGORY_BUTT
};
typedef VOS_UINT8 DRV_AGENT_PERSONALIZATION_CATEGORY_ENUM_UINT8;


enum DRV_AGENT_PERSONALIZATION_INDICATOR_ENUM
{
    DRV_AGENT_PERSONALIZATION_INDICATOR_INACTIVE               = 0x00,          /* δ���� */
    DRV_AGENT_PERSONALIZATION_INDICATOR_ACTIVE                 = 0x01,          /* �Ѽ��� */

    DRV_AGENT_PERSONALIZATION_INDICATOR_BUTT
};
typedef VOS_UINT8 DRV_AGENT_PERSONALIZATION_INDICATOR_ENUM_UINT8;


enum DRV_AGENT_PERSONALIZATION_STATUS_ENUM
{
    DRV_AGENT_PERSONALIZATION_STATUS_READY      = 0x00,                         /* �Ѿ����� */
    DRV_AGENT_PERSONALIZATION_STATUS_PIN        = 0x01,                         /* δ����״̬������������� */
    DRV_AGENT_PERSONALIZATION_STATUS_PUK        = 0x02,                         /* Block״̬��������UnBlock�� */

    DRV_AGENT_PERSONALIZATION_STATUS_BUTT
};
typedef VOS_UINT8 DRV_AGENT_PERSONALIZATION_STATUS_ENUM_UINT8;


enum DRV_AGENT_PH_PHYNUM_TYPE_ENUM
{
    DRV_AGENT_PH_PHYNUM_IMEI            = 0x00,          /* IMEI */
    DRV_AGENT_PH_PHYNUM_SN              = 0x01,          /* SN */

    DRV_AGENT_PH_PHYNUM_BUTT
};
typedef VOS_UINT8 DRV_AGENT_PH_PHYNUM_TYPE_ENUM_UINT8;


enum DRV_AGENT_PORT_STATUS_ENUM
{
    DRV_AGENT_PORT_STATUS_OFF              = 0,         /* �˿ڹر� */
    DRV_AGENT_PORT_STATUS_ON               = 1,         /* �˿ڴ� */

    DRV_AGENT_PORT_STATUS_BUTT
};
typedef VOS_UINT32 DRV_AGENT_PORT_STATUS_ENUM_UINT32;


enum DRV_AGENT_USIM_OPERATE_ENUM
{
    DRV_AGENT_USIM_OPERATE_DEACT                = 0,         /* ȥ����USIM */
    DRV_AGENT_USIM_OPERATE_ACT                  = 1,         /* ����USIM */

    DRV_AGENT_USIM_OPERATE_BUTT
};
typedef VOS_UINT32 DRV_AGENT_USIM_OPERATE_ENUM_UINT32;


enum DRV_AGENT_MSG_TYPE_ENUM
{
    /* ��Ϣ���� */                      /*��ϢID*/      /* ��ע */
    /* AT����DRV AGENT����Ϣ */
    DRV_AGENT_MSID_QRY_REQ               = 0x0000,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_GCF_IND                    = 0x0002,
    DRV_AGENT_GAS_MNTN_CMD               = 0x0006,
    DRV_AGENT_GPIOPL_SET_REQ             = 0x000E,
    DRV_AGENT_GPIOPL_QRY_REQ             = 0x0010,
    DRV_AGENT_DATALOCK_SET_REQ           = 0x0012,
    DRV_AGENT_TBATVOLT_QRY_REQ           = 0x0014,
    DRV_AGENT_VERTIME_QRY_REQ            = 0x0016,
    DRV_AGENT_YJCX_SET_REQ               = 0x0020,
    DRV_AGENT_YJCX_QRY_REQ               = 0x0022,
    DRV_AGENT_APPDMVER_QRY_REQ           = 0x0024,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/
    DRV_AGENT_DLOADINFO_QRY_REQ          = 0x0028,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/
    DRV_AGENT_FLASHINFO_QRY_REQ          = 0x002A,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/
    DRV_AGENT_AUTHVER_QRY_REQ            = 0x002C,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/
    DRV_AGENT_AUTHORITYVER_QRY_REQ       = 0x0030,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/
    DRV_AGENT_AUTHORITYID_QRY_REQ        = 0x0032,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/
    DRV_AGENT_GODLOAD_SET_REQ            = 0x0034,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/
    DRV_AGENT_HWNATQRY_QRY_REQ           = 0x0036,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/
    DRV_AGENT_CPULOAD_QRY_REQ            = 0x0038,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/
    DRV_AGENT_MFREELOCKSIZE_QRY_REQ      = 0x003A,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/

    DRV_AGENT_MEMINFO_QRY_REQ            = 0x003B,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/

    DRV_AGENT_HARDWARE_QRY               = 0x003E, /*��ѯӲ���汾��*/
    DRV_AGENT_FULL_HARDWARE_QRY          = 0x0040, /*��ѯ������Ӳ���汾��*/
    DRV_AGENT_TMODE_SET_REQ              = 0x0046,
    DRV_AGENT_VERSION_QRY_REQ            = 0x0048,
    DRV_AGENT_SECUBOOT_QRY_REQ           = 0x004A,
    DRV_AGENT_SECUBOOTFEATURE_QRY_REQ    = 0x004C,
    DRV_AGENT_PFVER_QRY_REQ              = 0x004E,
    DRV_AGENT_FCHAN_SET_REQ              = 0x0052,
    DRV_AGENT_SDLOAD_SET_REQ             = 0x0056,
    DRV_AGENT_SFEATURE_QRY_REQ           = 0x0058,
    DRV_AGENT_PRODTYPE_QRY_REQ           = 0x0060,
    DRV_AGENT_SDREBOOT_REQ               = 0x0062,
    DRV_AGENT_RXDIV_SET_REQ              = 0x0064,
    DRV_AGENT_DLOADVER_QRY_REQ           = 0x0066,

    DRV_AGENT_RXDIV_QRY_REQ              = 0x006A,
    DRV_AGENT_SIMLOCK_SET_REQ            = 0x006C,
    DRV_AGENT_IMSICHG_QRY_REQ            = 0x0072,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_NVRESTORE_SET_REQ          = 0x0074,                              /* _H2ASN_MsgChoice DRV_AGENT_NVRESTORE_RST_STRU */
    DRV_AGENT_INFORBU_SET_REQ            = 0x0076,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_NVRSTSTTS_QRY_REQ          = 0x0078,                              /* _H2ASN_MsgChoice DRV_AGENT_NVRESTORE_RST_STRU */
    DRV_AGENT_CPNN_TEST_REQ              = 0x007A,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_NVBACKUP_SET_REQ           = 0x007C,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_CPNN_QRY_REQ               = 0x007E,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_NVRESTORE_MANU_DEFAULT_REQ = 0x0080,                              /* _H2ASN_MsgChoice DRV_AGENT_NVRESTORE_RST_STRU */
    DRV_AGENT_ADC_SET_REQ                = 0x0082,                              /* _H2ASN_MsgChoice SPY_TEMP_THRESHOLD_PARA_STRU */
    DRV_AGENT_TSELRF_SET_REQ             = 0x0084,                              /* _H2ASN_MsgChoice DRV_AGENT_TSELRF_SET_REQ_STRU */
    DRV_AGENT_HKADC_GET_REQ              = 0x0086,                              /* _H2ASN_MsgChoice  */
    DRV_AGENT_TBAT_QRY_REQ               = 0x0088,
    DRV_AGENT_SECUBOOT_SET_REQ           = 0x008A,
    DRV_AGENT_SIMLOCK_NV_SET_REQ         = 0x008c,

    DRV_AGENT_SPWORD_SET_REQ             = 0x008e,

    DRV_AGENT_PSTANDBY_SET_REQ           = 0x0090,

    DRV_AGENT_NVBACKUPSTAT_QRY_REQ       = 0x0092,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_NANDBBC_QRY_REQ            = 0x0094,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_NANDVER_QRY_REQ            = 0x0096,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_CHIPTEMP_QRY_REQ           = 0x0098,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */


    DRV_AGENT_HUK_SET_REQ                = 0x009A,                              /* _H2ASN_MsgChoice DRV_AGENT_HUK_SET_REQ_STRU */
    DRV_AGENT_FACAUTHPUBKEY_SET_REQ      = 0x009C,                              /* _H2ASN_MsgChoice DRV_AGENT_FACAUTHPUBKEY_SET_REQ_STRU */
    DRV_AGENT_IDENTIFYSTART_SET_REQ      = 0x009E,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_IDENTIFYEND_SET_REQ        = 0x00A0,                              /* _H2ASN_MsgChoice DRV_AGENT_IDENTIFYEND_SET_REQ_STRU */
    DRV_AGENT_SIMLOCKDATAWRITE_SET_REQ   = 0x00A2,                              /* _H2ASN_MsgChoice DRV_AGENT_SIMLOCKDATAWRITE_SET_REQ_STRU */
    DRV_AGENT_PHONESIMLOCKINFO_QRY_REQ   = 0x00A4,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_SIMLOCKDATAREAD_QRY_REQ    = 0x00A6,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_PHONEPHYNUM_SET_REQ        = 0x00A8,                              /* _H2ASN_MsgChoice DRV_AGENT_PHONEPHYNUM_SET_REQ_STRU */
    DRV_AGENT_PHONEPHYNUM_QRY_REQ        = 0x00AA,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_PORTCTRLTMP_SET_REQ        = 0x00AC,                              /* _H2ASN_MsgChoice DRV_AGENT_PORTCTRLTMP_SET_REQ_STRU */
    DRV_AGENT_PORTATTRIBSET_SET_REQ      = 0x00AE,                              /* _H2ASN_MsgChoice DRV_AGENT_PORTATTRIBSET_SET_REQ_STRU */
    DRV_AGENT_PORTATTRIBSET_QRY_REQ      = 0x00B0,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */
    DRV_AGENT_OPWORD_SET_REQ             = 0x00B2,                              /* _H2ASN_MsgChoice DRV_AGENT_OPWORD_SET_REQ_STRU */
    DRV_AGENT_SARREDUCTION_SET_REQ       = 0x00B4,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */

    DRV_AGENT_INFORRS_SET_REQ            = 0x0100,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */

    DRV_AGENT_MAX_LOCK_TIMES_SET_REQ     = 0x0102,

    DRV_AGENT_AP_SIMST_SET_REQ           = 0x0104,                              /* _H2ASN_MsgChoice DRV_AGENT_AP_SIMST_SET_REQ_STRU */

    DRV_AGENT_SWVER_SET_REQ              = 0x0106,                              /* _H2ASN_MsgChoice DRV_AGENT_SWVER_INFO_STRU */
    DRV_AGENT_QRY_CCPU_MEM_INFO_REQ      = 0x0108,

    DRV_AGENT_HVPDH_REQ                  = 0x010A,

    DRV_AGENT_NVMANUFACTUREEXT_SET_REQ   = 0x010C,

    DRV_AGENT_ANTSWITCH_SET_REQ          = 0x010E,                              /* _H2ASN_MsgChoice DRV_AGENT_ANTSWITCH_SET_STRU */
    DRV_AGENT_ANTSWITCH_QRY_REQ          = 0x0110,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU */

#if(FEATURE_ON == MBB_COMMON)
    /***********************************************MBB�Զ���������ϢID��******************************************************/
    DRV_AGENT_DATALOCK_VERIFY_REQ = 0x0182,

#if (FEATURE_ON == MBB_SIMLOCK_FOUR)
    DRV_AGENT_HWLOCK_SET_REQ             = 0x0184,                  /*A������HWLOCK������*/
    DRV_AGENT_HWLOCK_QURY_REQ            = 0x0186,                  /*A�˲���HWLOCK������*/
#endif
#if (FEATURE_ON == MBB_MODULE_PM)
    DRV_AGENT_WAKEUPCFG_SET_REQ          = 0x0188,
    DRV_AGENT_CURC_SET_REQ               = 0x018A,
#endif/*MBB_MODULE_PM*/

#ifdef BSP_CONFIG_BOARD_SOLUTION
    DRV_AGENT_ADCREADEX_SET_REQ          = 0x018C,
#endif

#if(FEATURE_ON == MBB_FEATURE_CIMEI)
    DRV_AGENT_DEVNUMW_SET_FIRST_REQ      = 0x0192,
    DRV_AGENT_DEVNUMW_SET_SECOND_REQ     = 0x0194,
#endif
#ifdef BSP_CONFIG_BOARD_SOLUTION
    DRV_AGENT_GPIOLOOP_SET_REQ             = 0x0196,
    DRV_AGENT_GPIOLOOP_QRY_REQ             = 0x0198,
#endif
#endif
    /* DRV AGENT����AT����Ϣ */
    DRV_AGENT_MSID_QRY_CNF               = 0x0001,                              /* _H2ASN_MsgChoice DRV_AGENT_MSID_QRY_CNF_STRU */
    DRV_AGENT_GAS_MNTN_CMD_RSP           = 0x0005,
    DRV_AGENT_GPIOPL_SET_CNF             = 0x000D,
    DRV_AGENT_GPIOPL_QRY_CNF             = 0x000F,
    DRV_AGENT_DATALOCK_SET_CNF           = 0x0011,
    DRV_AGENT_TBATVOLT_QRY_CNF           = 0x0013,
    DRV_AGENT_VERTIME_QRY_CNF            = 0x0015,
    DRV_AGENT_YJCX_SET_CNF               = 0x001F,
    DRV_AGENT_YJCX_QRY_CNF               = 0x0021,
    DRV_AGENT_APPDMVER_QRY_CNF           = 0x0023,                              /* _H2ASN_MsgChoice DRV_AGENT_APPDMVER_QRY_CNF_STRU */
    DRV_AGENT_DLOADINFO_QRY_CNF          = 0x0027,                              /* _H2ASN_MsgChoice DRV_AGENT_DLOADINFO_QRY_CNF_STRU */
    DRV_AGENT_FLASHINFO_QRY_CNF          = 0x0029,                              /* _H2ASN_MsgChoice DRV_AGENT_FLASHINFO_QRY_CNF_STRU*/
    DRV_AGENT_AUTHVER_QRY_CNF            = 0x002B,                              /* _H2ASN_MsgChoice DRV_AGENT_AUTHVER_QRY_CNF_STRU*/
    DRV_AGENT_AUTHORITYVER_QRY_CNF       = 0x002F,                              /* _H2ASN_MsgChoice DRV_AGENT_AUTHORITYVER_QRY_CNF_STRU*/
    DRV_AGENT_AUTHORITYID_QRY_CNF        = 0x0031,                              /* _H2ASN_MsgChoice DRV_AGENT_AUTHORITYID_QRY_CNF_STRU*/
    DRV_AGENT_GODLOAD_SET_CNF            = 0x0033,                              /* _H2ASN_MsgChoice DRV_AGENT_GODLOAD_SET_CNF_STRU*/
    DRV_AGENT_HWNATQRY_QRY_CNF           = 0x0035,                              /* _H2ASN_MsgChoice DRV_AGENT_HWNATQRY_QRY_CNF_STRU*/
    DRV_AGENT_CPULOAD_QRY_CNF            = 0x0037,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/
    DRV_AGENT_MFREELOCKSIZE_QRY_CNF      = 0x0039,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/

    DRV_AGENT_MEMINFO_QRY_CNF            = 0x003A,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/

    DRV_AGENT_HARDWARE_QRY_RSP           = 0x003D, /*�ظ�Ӳ���汾��*/
    DRV_AGENT_FULL_HARDWARE_QRY_RSP      = 0x003F, /*�ظ�������Ӳ���汾��*/
    DRV_AGENT_VERSION_QRY_CNF            = 0x0047,
    DRV_AGENT_SECUBOOT_QRY_CNF           = 0x0049,
    DRV_AGENT_SECUBOOTFEATURE_QRY_CNF    = 0x004B,
    DRV_AGENT_PFVER_QRY_CNF              = 0x004D,                              /* _H2ASN_MsgChoice DRV_AGENT_PFVER_QRY_CNF_STRU */
    DRV_AGENT_FCHAN_SET_CNF              = 0x0053,
    DRV_AGENT_SDLOAD_SET_CNF             = 0x0057,
    DRV_AGENT_SFEATURE_QRY_CNF           = 0x0059,
    DRV_AGENT_PRODTYPE_QRY_CNF           = 0x0061,
    DRV_AGENT_RXDIV_SET_CNF              = 0x0063,
    DRV_AGENT_DLOADVER_QRY_CNF           = 0x0065,
    DRV_AGENT_RXDIV_QRY_CNF              = 0x0069,
    DRV_AGENT_SIMLOCK_SET_CNF            = 0x006B,
    DRV_AGENT_IMSICHG_QRY_CNF            = 0x0071,                              /* _H2ASN_MsgChoice DRV_AGENT_IMSICHG_QRY_CNF_STRU */
    DRV_AGENT_NVRESTORE_SET_CNF          = 0x0073,                              /* _H2ASN_MsgChoice DRV_AGENT_NVRESTORE_RST_STRU */
    DRV_AGENT_INFORBU_SET_CNF            = 0x0075,                              /* _H2ASN_MsgChoice DRV_AGENT_INFORBU_SET_CNF_STRU */
    DRV_AGENT_NVRSTSTTS_QRY_CNF          = 0x0077,                              /* _H2ASN_MsgChoice DRV_AGENT_NVRESTORE_RST_STRU */
    DRV_AGENT_CPNN_TEST_CNF              = 0x0079,                              /* _H2ASN_MsgChoice DRV_AGENT_CPNN_TEST_CNF_STRU */
    DRV_AGENT_NVBACKUP_SET_CNF           = 0x007B,                              /* _H2ASN_MsgChoice DRV_AGENT_NVBACKUP_SET_CNF_STRU */
    DRV_AGENT_CPNN_QRY_CNF               = 0x007D,                              /* _H2ASN_MsgChoice DRV_AGENT_CPNN_QRY_CNF_STRU */
    DRV_AGENT_NVRESTORE_MANU_DEFAULT_CNF = 0x007F,                              /* _H2ASN_MsgChoice DRV_AGENT_NVRESTORE_RST_STRU */
    DRV_AGENT_ADC_SET_CNF                = 0x0081,
    DRV_AGENT_TSELRF_SET_CNF             = 0x0083,                              /* _H2ASN_MsgChoice DRV_AGENT_TSELRF_SET_CNF_STRU */
    DRV_AGENT_HKADC_GET_CNF              = 0x0085,                              /* _H2ASN_MsgChoice DRV_AGENT_HKADC_GET_CNF_STRU */
    DRV_AGENT_TBAT_QRY_CNF               = 0x0087,
    DRV_AGENT_SECUBOOT_SET_CNF           = 0x0089,                              /* _H2ASN_MsgChoice DRV_AGENT_SECUBOOT_SET_CNF_STRU */
    DRV_AGENT_SPWORD_SET_CNF             = 0x008B,

    DRV_AGENT_NVBACKUPSTAT_QRY_CNF       = 0x008D,                              /* _H2ASN_MsgChoice DRV_AGENT_NVBACKUPSTAT_QRY_CNF_STRU */
    DRV_AGENT_NANDBBC_QRY_CNF            = 0x008F,                              /* _H2ASN_MsgChoice DRV_AGENT_NANDBBC_QRY_CNF_STRU */
    DRV_AGENT_NANDVER_QRY_CNF            = 0x0091,                              /* _H2ASN_MsgChoice DRV_AGENT_NANDVER_QRY_CNF_STRU */
    DRV_AGENT_CHIPTEMP_QRY_CNF           = 0x0093,                              /* _H2ASN_MsgChoice DRV_AGENT_CHIPTEMP_QRY_CNF_STRU */


    DRV_AGENT_HUK_SET_CNF                = 0x0095,                              /* _H2ASN_MsgChoice DRV_AGENT_HUK_SET_CNF_STRU */
    DRV_AGENT_FACAUTHPUBKEY_SET_CNF      = 0x0097,                              /* _H2ASN_MsgChoice DRV_AGENT_FACAUTHPUBKEY_SET_CNF_STRU */
    DRV_AGENT_IDENTIFYSTART_SET_CNF      = 0x0099,                              /* _H2ASN_MsgChoice DRV_AGENT_IDENTIFYSTART_SET_CNF_STRU */
    DRV_AGENT_IDENTIFYEND_SET_CNF        = 0x009B,                              /* _H2ASN_MsgChoice DRV_AGENT_IDENTIFYEND_SET_CNF_STRU */
    DRV_AGENT_SIMLOCKDATAWRITE_SET_CNF   = 0x009D,                              /* _H2ASN_MsgChoice DRV_AGENT_SIMLOCKDATAWRITE_SET_CNF_STRU */
    DRV_AGENT_PHONESIMLOCKINFO_QRY_CNF   = 0x009F,                              /* _H2ASN_MsgChoice DRV_AGENT_PHONESIMLOCKINFO_QRY_CNF_STRU */
    DRV_AGENT_SIMLOCKDATAREAD_QRY_CNF    = 0x00A1,                              /* _H2ASN_MsgChoice DRV_AGENT_SIMLOCKDATAREAD_QRY_CNF_STRU */
    DRV_AGENT_PHONEPHYNUM_SET_CNF        = 0x00A3,                              /* _H2ASN_MsgChoice DRV_AGENT_PHONEPHYNUM_SET_CNF_STRU */
    DRV_AGENT_PHONEPHYNUM_QRY_CNF        = 0x00A5,                              /* _H2ASN_MsgChoice DRV_AGENT_PHONEPHYNUM_QRY_CNF_STRU */
    DRV_AGENT_PORTCTRLTMP_SET_CNF        = 0x00A7,                              /* _H2ASN_MsgChoice DRV_AGENT_PORTCTRLTMP_SET_CNF_STRU */
    DRV_AGENT_PORTATTRIBSET_SET_CNF      = 0x00A9,                              /* _H2ASN_MsgChoice DRV_AGENT_PORTATTRIBSET_SET_CNF_STRU */
    DRV_AGENT_PORTATTRIBSET_QRY_CNF      = 0x00AB,                              /* _H2ASN_MsgChoice DRV_AGENT_PORTATTRIBSET_QRY_CNF_STRU */
    DRV_AGENT_OPWORD_SET_CNF             = 0x00AD,                              /* _H2ASN_MsgChoice DRV_AGENT_OPWORD_SET_CNF_STRU */
    DRV_AGENT_ANTSTATE_QRY_IND           = 0x00AF,                              /* _H2ASN_MsgChoice AT_APPCTRL_STRU*/
    DRV_AGENT_INFORRS_SET_CNF            = 0x0101,                              /* _H2ASN_MsgChoice DRV_AGENT_INFORRS_SET_CNF_STRU */


    DRV_AGENT_MAX_LOCK_TIMES_SET_CNF     = 0x0103,                              /* _H2ASN_MsgChoice DRV_AGENT_MAX_LOCK_TMS_SET_CNF_STRU */

    DRV_AGENT_AP_SIMST_SET_CNF           = 0x0105,                              /* _H2ASN_MsgChoice DRV_AGENT_AP_SIMST_SET_CNF_STRU */

    DRV_AGENT_SWVER_SET_CNF              = 0x0107,                              /* _H2ASN_MsgChoice DRV_AGENT_SWVER_SET_CNF_STRU */

    DRV_AGENT_HVPDH_CNF                  = 0x0109,

    DRV_AGENT_NVMANUFACTUREEXT_SET_CNF   = 0x010B,

    DRV_AGENT_ANTSWITCH_SET_CNF          = 0x010D,                              /* _H2ASN_MsgChoice DRV_AGENT_ANTSWITCH_SET_CNF_STRU */
    DRV_AGENT_ANTSWITCH_QRY_CNF          = 0x010F,                              /* _H2ASN_MsgChoice DRV_AGENT_ANTSWITCH_QRY_CNF_STRU */
#if(FEATURE_ON == MBB_COMMON)
    /***********************************************MBB�Զ��巵����ϢID��******************************************************/
    DRV_AGENT_DATALOCK_VERIFY_CNF    = 0x0183,
#if (FEATURE_ON == MBB_MODULE_PM)
    DRV_AGENT_WAKEUPCFG_SET_CNF          = 0x0189,
    DRV_AGENT_CURC_SET_CNF               = 0x018B,
#endif/*MBB_MODULE_PM*/

#ifdef BSP_CONFIG_BOARD_SOLUTION
    DRV_AGENT_ADCREADEX_SET_CNF          = 0x018D,
#endif

#if(FEATURE_ON == MBB_NOISETOOL)
    DRV_AGENT_NOISECFG_SET_REQ      =0x0190,
    DRV_AGENT_NOISECFG_SET_CNF      =0x0191,
#endif/*MBB_NOISETOOL*/

#if(FEATURE_ON == MBB_FEATURE_CIMEI)
    DRV_AGENT_DEVNUMW_SET_FIRST_CNF      = 0x0193,
    DRV_AGENT_DEVNUMW_SET_SECOND_CNF     = 0x0195,
#endif
#ifdef BSP_CONFIG_BOARD_SOLUTION
    DRV_AGENT_GPIOLOOP_SET_CNF             = 0x0197,
    DRV_AGENT_GPIOLOOP_QRY_CNF             = 0x0199,
#endif
#endif
    DRV_AGENT_MSG_TYPE_BUTT              = 0xFFFF
};
typedef VOS_UINT32 DRV_AGENT_MSG_TYPE_ENUM_UINT32;


enum AT_DEVICE_CMD_RAT_MODE_ENUM
{
    AT_RAT_MODE_WCDMA = 0,
    AT_RAT_MODE_CDMA,
    AT_RAT_MODE_TDSCDMA,
    AT_RAT_MODE_GSM,
    AT_RAT_MODE_EDGE,
    AT_RAT_MODE_AWS,
    AT_RAT_MODE_FDD_LTE,
    AT_RAT_MODE_TDD_LTE,
    AT_RAT_MODE_WIFI,
    AT_RAT_MODE_BUTT
};
typedef VOS_UINT8 AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8;


/* ��ѯ����ڴ���������  */
enum AT_MEMQUERY_PARA_ENUM
{
    AT_MEMQUERY_VOS = 0,  /* ��ѯVOS�İ뾲̬�ڴ�ʵ�ʵ�ռ����� */
    AT_MEMQUERY_TTF,      /* ��ѯTTF�İ뾲̬�ڴ�ʵ�ʵ�ռ����� */
    AT_MEMQUERY_BUTT
};
typedef VOS_UINT32  AT_MEMQUERY_PARA_ENUM_UINT32;



enum DRV_AGENT_DH_KEY_TYPE_ENUM
{
    DRV_AGENT_DH_KEY_SERVER_PUBLIC_KEY  = 0x00,          /* ��������Կ */
    DRV_AGENT_DH_KEY_MODEM_PUBLIC_KEY   = 0x01,          /* MODEM�๫Կ */
    DRV_AGENT_DH_KEY_MODEM_PRIVATE_KEY  = 0x02,          /* MODEM��˽Կ */

    DRV_AGENT_DH_KEY_BUTT
};
typedef VOS_UINT32 DRV_AGENT_DH_KEY_TYPE_ENUM_UINT32;


enum DRV_AGENT_HVPDH_ERR_ENUM
{
    DRV_AGENT_HVPDH_NO_ERROR            = 0x00,          /* �����ɹ� */
    DRV_AGENT_HVPDH_AUTH_UNDO           = 0x01,          /* ��ʱ��δ���й����߼�Ȩ */
    DRV_AGENT_HVPDH_NV_READ_FAIL        = 0x02,          /* NV��ȡʧ�� */
    DRV_AGENT_HVPDH_NV_WRITE_FAIL       = 0x03,          /* NVд��ʧ�� */
    DRV_AGENT_HVPDH_OTHER_ERROR         = 0x04,          /* �������� */

    DRV_AGENT_HVPDH_ERR_BUTT
};
typedef VOS_UINT32 DRV_AGENT_HVPDH_ERR_ENUM_UINT32;


#if (FEATURE_ON == MBB_ANTENNA_MODE)
/* ^ANTMODE�����ּ����߹���ģʽ */
typedef enum AT_ANTMODE_ANT_MODE_ENUM
{
    AT_ANTMODE_ANT_MODE_MAIN_AND_AUX = 0,         /* ���ּ�ͬʱ�� */
    AT_ANTMODE_ANT_MODE_MAIN_ONLY    = 1,         /* �������� */
    AT_ANTMODE_ANT_MODE_AUX_ONLY     = 2,         /* ���ּ��� */

    AT_ANTMODE_ANT_MODE_INVALID
} at_antmode_ant_modes;

/* ^ANTMODE��������ʽ */
typedef enum AT_ANTMODE_NW_MODE_ENUM
{
    AT_ANTMODE_NW_MODE_ALL       = 0,         /* ȫ����ʽ */
    AT_ANTMODE_NW_MODE_RESERVED  = 1,         /* ������������ */
    AT_ANTMODE_NW_MODE_WCDMA     = 2,         /* WCDMA */
    AT_ANTMODE_NW_MODE_LTE       = 3,         /* LTE */
    AT_ANTMODE_NW_MODE_CDMA_1X   = 4,         /* CDMA 1X */
    AT_ANTMODE_NW_MODE_TDSCDMA   = 5,         /* TD-SCDMA */
    AT_ANTMODE_NW_MODE_WIMAX     = 6,         /* WIMAX */
    AT_ANTMODE_NW_MODE_CDMA_EVDO = 7,         /* CDMA EVDO */

    AT_ANTMODE_NW_MODE_MAX
} at_antmode_nw_modes;
#endif /* MBB_ANTENNA_MODE */

#if (FEATURE_ON == MBB_ANTENNA_TUNER)
/* ^ANTENCFG��������ʽ */
typedef enum AT_ANTENCFG_MODE_ENUM
{
    AT_ANTENCFG_NW_MODE_GSM       = 0,         /* GSM */
    AT_ANTENCFG_NW_MODE_WCDMA     = 1,         /* WCDMA */
    AT_ANTENCFG_NW_MODE_CDMA      = 2,         /* CDMA */
    AT_ANTENCFG_NW_MODE_LTE       = 3,         /* LTE */
    AT_ANTENCFG_NW_MODE_TDSCDMA   = 4,         /* TD-SCDMA */

    AT_ANTENCFG_NW_MODE_INVALID
} antencfg_mode_types;

/* NV10002 �ṹ�� UCOM_NV_WG_RF_BAND_CONFIG_STRU ��Ա uhwBand ��Ӧ��Ƶ�� */
enum
{
    ANTENCFG_NV10002_WB1RXMain           = 1,   /* WCDMA B1 �������� */
    ANTENCFG_NV10002_WB1RXDiv            = 1,   /* WCDMA B1 �ּ����� */
    ANTENCFG_NV10002_WB1TX               = 1,   /* WCDMA B1 ���� */
    ANTENCFG_NV10002_WB2RXMain           = 2,   /* WCDMA B2 �������� */
    ANTENCFG_NV10002_WB2RXDiv            = 2,   /* WCDMA B2 �ּ����� */
    ANTENCFG_NV10002_WB2TX               = 2,   /* WCDMA B2 ���� */
    ANTENCFG_NV10002_WB3RXMain           = 3,   /* WCDMA B3 �������� */
    ANTENCFG_NV10002_WB3RXDiv            = 3,   /* WCDMA B3 �ּ����� */
    ANTENCFG_NV10002_WB3TX               = 3,   /* WCDMA B3 ���� */
    ANTENCFG_NV10002_WB4RXMain           = 4,   /* WCDMA B4 �������� */
    ANTENCFG_NV10002_WB4RXDiv            = 4,   /* WCDMA B4 �ּ����� */
    ANTENCFG_NV10002_WB4TX               = 4,   /* WCDMA B4 ���� */
    ANTENCFG_NV10002_WB5RXMain           = 5,   /* WCDMA B5 �������� */
    ANTENCFG_NV10002_WB5RXDiv            = 5,   /* WCDMA B5 �ּ����� */
    ANTENCFG_NV10002_WB5TX               = 5,   /* WCDMA B5 ���� */
    ANTENCFG_NV10002_WB6RXMain           = 6,   /* WCDMA B6 �������� */
    ANTENCFG_NV10002_WB6RXDiv            = 6,   /* WCDMA B6 �ּ����� */
    ANTENCFG_NV10002_WB6TX               = 6,   /* WCDMA B6 ���� */
    ANTENCFG_NV10002_WB8RXMain           = 8,   /* WCDMA B8 �������� */
    ANTENCFG_NV10002_WB8RXDiv            = 8,   /* WCDMA B8 �ּ����� */
    ANTENCFG_NV10002_WB8TX               = 8,   /* WCDMA B8 ���� */
    ANTENCFG_NV10002_GSM_850_RX          = 128, /* GSM850 �������� */
    ANTENCFG_NV10002_GSM_900_RX          = 129, /* GSM900 �������� */
    ANTENCFG_NV10002_GSM_1800_RX         = 130, /* GSM1800 �������� */
    ANTENCFG_NV10002_GSM_1900_RX         = 131, /* GSM1900 �������� */
    ANTENCFG_NV10002_HB_GMSK_TX          = 132, /* GSM1800/1900 GMSK ���� */
    ANTENCFG_NV10002_LB_GMSK_TX          = 133, /* GSM850/900 GMSK ���� */
    ANTENCFG_NV10002_HB_8PSK_TX          = 134, /* GSM1800/1900 8PSK ���� */
    ANTENCFG_NV10002_LB_8PSK_TX          = 135  /* GSM850/900 8PSK ���� */
};

/* NV0xE49A �ṹ�� TDS_NV_LINECTRL_ALLOT_BY_HARDWARE_STRU ��Ա aulBandList ��Ӧ��Ƶ�� */
enum
{
    ANTENCFG_NV0xE49A_TDSBANDA           = 34,   /* TD-SCDMA BAND A/BAND34 */
    ANTENCFG_NV0xE49A_TDSBANDF           = 39,   /* TD-SCDMA BAND F/BAND39 */
};

/* GSM��Ƶ�ζ�Ӧ��maskλ */
enum
{
    ANTENCFG_GSM_850_MASK                = 0x00000001,
    ANTENCFG_GSM_900_MASK                = 0x00000002,
    ANTENCFG_GSM_1800_MASK               = 0x00000004,
    ANTENCFG_GSM_1900_MASK               = 0x00000008
};

/* WCDMA��Ƶ�ζ�Ӧ��maskλ */
enum
{
    ANTENCFG_WCDMA_I_IMT_2000_MASK        = 0x00000001,
    ANTENCFG_WCDMA_II_PCS_1900_MASK       = 0x00000002,
    ANTENCFG_WCDMA_III_1700_MASK          = 0x00000004,
    ANTENCFG_WCDMA_IV_1700_MASK           = 0x00000008,
    ANTENCFG_WCDMA_V_850_MASK             = 0x00000010,
    ANTENCFG_WCDMA_VI_800_MASK            = 0x00000020,
    ANTENCFG_WCDMA_VII_2600_MASK          = 0x00000040,
    ANTENCFG_WCDMA_VIII_900_MASK          = 0x00000080,
    ANTENCFG_WCDMA_IX_1700_MASK           = 0x00000100
};

/* CDMA��Ƶ�ζ�Ӧ��maskλ */
enum
{
    ANTENCFG_CDMA_BC0_A_MASK              = 0x00000001,
    ANTENCFG_CDMA_BC0_B_MASK              = 0x00000002,
    ANTENCFG_CDMA_BC1_MASK                = 0x00000004,
    ANTENCFG_CDMA_BC2_MASK                = 0x00000008,
    ANTENCFG_CDMA_BC3_MASK                = 0x00000010,
    ANTENCFG_CDMA_BC4_MASK                = 0x00000020,
    ANTENCFG_CDMA_BC5_MASK                = 0x00000040,
    ANTENCFG_CDMA_BC6_MASK                = 0x00000080,
    ANTENCFG_CDMA_BC7_MASK                = 0x00000100,
    ANTENCFG_CDMA_BC8_MASK                = 0x00000200,
    ANTENCFG_CDMA_BC9_MASK                = 0x00000400,
    ANTENCFG_CDMA_BC10_MASK               = 0x00000800,
    ANTENCFG_CDMA_BC11_MASK               = 0x00001000,
    ANTENCFG_CDMA_BC12_MASK               = 0x00002000,
    ANTENCFG_CDMA_BC13_MASK               = 0x00004000,
    ANTENCFG_CDMA_BC14_MASK               = 0x00008000,
    ANTENCFG_CDMA_BC15_MASK               = 0x00010000,
    ANTENCFG_CDMA_BC16_MASK               = 0x00020000,
    ANTENCFG_CDMA_BC17_MASK               = 0x00040000,
    ANTENCFG_CDMA_BC18_MASK               = 0x00080000,
    ANTENCFG_CDMA_BC19_MASK               = 0x00100000
};


/* LTE��Ƶ�ζ�Ӧ�ĵ��ֶ�maskλ */
enum
{
    ANTENCFG_LTE_BAND1_MASK               = 0x00000001,
    ANTENCFG_LTE_BAND2_MASK               = 0x00000002,
    ANTENCFG_LTE_BAND3_MASK               = 0x00000004,
    ANTENCFG_LTE_BAND4_MASK               = 0x00000008,
    ANTENCFG_LTE_BAND5_MASK               = 0x00000010,
    ANTENCFG_LTE_BAND6_MASK               = 0x00000020,
    ANTENCFG_LTE_BAND7_MASK               = 0x00000040,
    ANTENCFG_LTE_BAND8_MASK               = 0x00000080,
    ANTENCFG_LTE_BAND9_MASK               = 0x00000100,
    ANTENCFG_LTE_BAND10_MASK              = 0x00000200,
    ANTENCFG_LTE_BAND11_MASK              = 0x00000400,
    ANTENCFG_LTE_BAND12_MASK              = 0x00000800,
    ANTENCFG_LTE_BAND13_MASK              = 0x00001000,
    ANTENCFG_LTE_BAND14_MASK              = 0x00002000,
    ANTENCFG_LTE_BAND15_MASK              = 0x00004000,
    ANTENCFG_LTE_BAND16_MASK              = 0x00008000,
    ANTENCFG_LTE_BAND17_MASK              = 0x00010000,
    ANTENCFG_LTE_BAND18_MASK              = 0x00020000,
    ANTENCFG_LTE_BAND19_MASK              = 0x00040000,
    ANTENCFG_LTE_BAND20_MASK              = 0x00080000,
    ANTENCFG_LTE_BAND21_MASK              = 0x00100000,
    ANTENCFG_LTE_BAND22_MASK              = 0x00200000,
    ANTENCFG_LTE_BAND23_MASK              = 0x00400000,
    ANTENCFG_LTE_BAND24_MASK              = 0x00800000,
    ANTENCFG_LTE_BAND25_MASK              = 0x01000000,
    ANTENCFG_LTE_BAND26_MASK              = 0x02000000,
    ANTENCFG_LTE_BAND27_MASK              = 0x04000000,
    ANTENCFG_LTE_BAND28_MASK              = 0x08000000,
    ANTENCFG_LTE_BAND29_MASK              = 0x10000000,
    ANTENCFG_LTE_BAND30_MASK              = 0x20000000,
    ANTENCFG_LTE_BAND31_MASK              = 0x40000000,
    ANTENCFG_LTE_BAND32_MASK              = 0x80000000,
    ANTENCFG_LTE_BAND33_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND34_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND35_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND36_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND37_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND38_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND39_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND40_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND41_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND42_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND43_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND44_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND45_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND46_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND47_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND48_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND49_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND50_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND51_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND52_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND53_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND54_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND55_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND56_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND57_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND58_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND59_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND60_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND61_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND62_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND63_MASK              = 0x00000000,
    ANTENCFG_LTE_BAND64_MASK              = 0x00000000
};

/* LTE��Ƶ�ζ�Ӧ�ĸ��ֶ�maskλ */
enum
{
    ANTENCFG_LTE_BAND1_MASK_EXT           = 0x00000000,
    ANTENCFG_LTE_BAND2_MASK_EXT           = 0x00000000,
    ANTENCFG_LTE_BAND3_MASK_EXT           = 0x00000000,
    ANTENCFG_LTE_BAND4_MASK_EXT           = 0x00000000,
    ANTENCFG_LTE_BAND5_MASK_EXT           = 0x00000000,
    ANTENCFG_LTE_BAND6_MASK_EXT           = 0x00000000,
    ANTENCFG_LTE_BAND7_MASK_EXT           = 0x00000000,
    ANTENCFG_LTE_BAND8_MASK_EXT           = 0x00000000,
    ANTENCFG_LTE_BAND9_MASK_EXT           = 0x00000000,
    ANTENCFG_LTE_BAND10_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND11_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND12_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND13_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND14_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND15_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND16_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND17_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND18_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND19_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND20_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND21_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND22_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND23_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND24_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND25_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND26_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND27_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND28_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND29_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND30_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND31_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND32_MASK_EXT          = 0x00000000,
    ANTENCFG_LTE_BAND33_MASK_EXT          = 0x00000001,
    ANTENCFG_LTE_BAND34_MASK_EXT          = 0x00000002,
    ANTENCFG_LTE_BAND35_MASK_EXT          = 0x00000004,
    ANTENCFG_LTE_BAND36_MASK_EXT          = 0x00000008,
    ANTENCFG_LTE_BAND37_MASK_EXT          = 0x00000010,
    ANTENCFG_LTE_BAND38_MASK_EXT          = 0x00000020,
    ANTENCFG_LTE_BAND39_MASK_EXT          = 0x00000040,
    ANTENCFG_LTE_BAND40_MASK_EXT          = 0x00000080,
    ANTENCFG_LTE_BAND41_MASK_EXT          = 0x00000100,
    ANTENCFG_LTE_BAND42_MASK_EXT          = 0x00000200,
    ANTENCFG_LTE_BAND43_MASK_EXT          = 0x00000400,
    ANTENCFG_LTE_BAND44_MASK_EXT          = 0x00000800,
    ANTENCFG_LTE_BAND45_MASK_EXT          = 0x00001000,
    ANTENCFG_LTE_BAND46_MASK_EXT          = 0x00002000,
    ANTENCFG_LTE_BAND47_MASK_EXT          = 0x00004000,
    ANTENCFG_LTE_BAND48_MASK_EXT          = 0x00008000,
    ANTENCFG_LTE_BAND49_MASK_EXT          = 0x00010000,
    ANTENCFG_LTE_BAND50_MASK_EXT          = 0x00020000,
    ANTENCFG_LTE_BAND51_MASK_EXT          = 0x00040000,
    ANTENCFG_LTE_BAND52_MASK_EXT          = 0x00080000,
    ANTENCFG_LTE_BAND53_MASK_EXT          = 0x00100000,
    ANTENCFG_LTE_BAND54_MASK_EXT          = 0x00200000,
    ANTENCFG_LTE_BAND55_MASK_EXT          = 0x00400000,
    ANTENCFG_LTE_BAND56_MASK_EXT          = 0x00800000,
    ANTENCFG_LTE_BAND57_MASK_EXT          = 0x01000000,
    ANTENCFG_LTE_BAND58_MASK_EXT          = 0x02000000,
    ANTENCFG_LTE_BAND59_MASK_EXT          = 0x04000000,
    ANTENCFG_LTE_BAND60_MASK_EXT          = 0x08000000,
    ANTENCFG_LTE_BAND61_MASK_EXT          = 0x10000000,
    ANTENCFG_LTE_BAND62_MASK_EXT          = 0x20000000,
    ANTENCFG_LTE_BAND63_MASK_EXT          = 0x40000000,
    ANTENCFG_LTE_BAND64_MASK_EXT          = 0x80000000
};

/* TD-SCDMA��Ƶ�ζ�Ӧ��maskλ */
enum
{
    ANTENCFG_TDSCDMA_BAND_A_MASK          = 0x00000001,
    ANTENCFG_TDSCDMA_BAND_B_MASK          = 0x00000002,
    ANTENCFG_TDSCDMA_BAND_C_MASK          = 0x00000004,
    ANTENCFG_TDSCDMA_BAND_D_MASK          = 0x00000008,
    ANTENCFG_TDSCDMA_BAND_E_MASK          = 0x00000010,
    ANTENCFG_TDSCDMA_BAND_F_MASK          = 0x00000020
};

/* GSM��Ƶ�ζ�ӦNV 50377������±�ֵ */
enum
{
    ANTENCFG_GSM_850_INDEX                = 0,
    ANTENCFG_GSM_900_INDEX                = 1,
    ANTENCFG_GSM_1800_INDEX               = 2,
    ANTENCFG_GSM_1900_INDEX               = 3
};

/* WCDMA��Ƶ�ζ�ӦNV 50378������±�ֵ */
enum
{
    ANTENCFG_WCDMA_I_IMT_2000_INDEX       = 0,
    ANTENCFG_WCDMA_II_PCS_1900_INDEX      = 1,
    ANTENCFG_WCDMA_III_1700_INDEX         = 2,
    ANTENCFG_WCDMA_IV_1700_INDEX          = 3,
    ANTENCFG_WCDMA_V_850_INDEX            = 4,
    ANTENCFG_WCDMA_VI_800_INDEX           = 5,
    ANTENCFG_WCDMA_VII_2600_INDEX         = 6,
    ANTENCFG_WCDMA_VIII_900_INDEX         = 7,
    ANTENCFG_WCDMA_IX_1700_INDEX          = 8
};

/* CDMA��Ƶ�ζ�ӦNV 50379������±�ֵ */
enum
{
    ANTENCFG_CDMA_BC0_A_INDEX             = 0,
    ANTENCFG_CDMA_BC0_B_INDEX             = 1,
    ANTENCFG_CDMA_BC1_INDEX               = 2,
    ANTENCFG_CDMA_BC2_INDEX               = 3,
    ANTENCFG_CDMA_BC3_INDEX               = 4,
    ANTENCFG_CDMA_BC4_INDEX               = 5,
    ANTENCFG_CDMA_BC5_INDEX               = 6,
    ANTENCFG_CDMA_BC6_INDEX               = 7,
    ANTENCFG_CDMA_BC7_INDEX               = 8,
    ANTENCFG_CDMA_BC8_INDEX               = 9,
    ANTENCFG_CDMA_BC9_INDEX               = 10,
    ANTENCFG_CDMA_BC10_INDEX              = 11,
    ANTENCFG_CDMA_BC11_INDEX              = 12,
    ANTENCFG_CDMA_BC12_INDEX              = 13,
    ANTENCFG_CDMA_BC13_INDEX              = 14,
    ANTENCFG_CDMA_BC14_INDEX              = 15,
    ANTENCFG_CDMA_BC15_INDEX              = 16,
    ANTENCFG_CDMA_BC16_INDEX              = 17,
    ANTENCFG_CDMA_BC17_INDEX              = 18,
    ANTENCFG_CDMA_BC18_INDEX              = 19,
    ANTENCFG_CDMA_BC19_INDEX              = 20
};

/* LTE��Ƶ�ζ�ӦNV 50380������±�ֵ */
enum
{
    ANTENCFG_LTE_BAND1_INDEX              = 0,
    ANTENCFG_LTE_BAND2_INDEX              = 1,
    ANTENCFG_LTE_BAND3_INDEX              = 2,
    ANTENCFG_LTE_BAND4_INDEX              = 3,
    ANTENCFG_LTE_BAND5_INDEX              = 4,
    ANTENCFG_LTE_BAND6_INDEX              = 5,
    ANTENCFG_LTE_BAND7_INDEX              = 6,
    ANTENCFG_LTE_BAND8_INDEX              = 7,
    ANTENCFG_LTE_BAND9_INDEX              = 8,
    ANTENCFG_LTE_BAND10_INDEX             = 9,
    ANTENCFG_LTE_BAND11_INDEX             = 10,
    ANTENCFG_LTE_BAND12_INDEX             = 11,
    ANTENCFG_LTE_BAND13_INDEX             = 12,
    ANTENCFG_LTE_BAND14_INDEX             = 13,
    ANTENCFG_LTE_BAND15_INDEX             = 14,
    ANTENCFG_LTE_BAND16_INDEX             = 15,
    ANTENCFG_LTE_BAND17_INDEX             = 16,
    ANTENCFG_LTE_BAND18_INDEX             = 17,
    ANTENCFG_LTE_BAND19_INDEX             = 18,
    ANTENCFG_LTE_BAND20_INDEX             = 19,
    ANTENCFG_LTE_BAND21_INDEX             = 20,
    ANTENCFG_LTE_BAND22_INDEX             = 21,
    ANTENCFG_LTE_BAND23_INDEX             = 22,
    ANTENCFG_LTE_BAND24_INDEX             = 23,
    ANTENCFG_LTE_BAND25_INDEX             = 24,
    ANTENCFG_LTE_BAND26_INDEX             = 25,
    ANTENCFG_LTE_BAND27_INDEX             = 26,
    ANTENCFG_LTE_BAND28_INDEX             = 27,
    ANTENCFG_LTE_BAND29_INDEX             = 28,
    ANTENCFG_LTE_BAND30_INDEX             = 29,
    ANTENCFG_LTE_BAND31_INDEX             = 30,
    ANTENCFG_LTE_BAND32_INDEX             = 31,
    ANTENCFG_LTE_BAND33_INDEX             = 32,
    ANTENCFG_LTE_BAND34_INDEX             = 33,
    ANTENCFG_LTE_BAND35_INDEX             = 34,
    ANTENCFG_LTE_BAND36_INDEX             = 35,
    ANTENCFG_LTE_BAND37_INDEX             = 36,
    ANTENCFG_LTE_BAND38_INDEX             = 37,
    ANTENCFG_LTE_BAND39_INDEX             = 38,
    ANTENCFG_LTE_BAND40_INDEX             = 39,
    ANTENCFG_LTE_BAND41_INDEX             = 40,
    ANTENCFG_LTE_BAND42_INDEX             = 41,
    ANTENCFG_LTE_BAND43_INDEX             = 42,
    ANTENCFG_LTE_BAND44_INDEX             = 43,
    ANTENCFG_LTE_BAND45_INDEX             = 44,
    ANTENCFG_LTE_BAND46_INDEX             = 45,
    ANTENCFG_LTE_BAND47_INDEX             = 46,
    ANTENCFG_LTE_BAND48_INDEX             = 47,
    ANTENCFG_LTE_BAND49_INDEX             = 48,
    ANTENCFG_LTE_BAND50_INDEX             = 49,
    ANTENCFG_LTE_BAND51_INDEX             = 50,
    ANTENCFG_LTE_BAND52_INDEX             = 51,
    ANTENCFG_LTE_BAND53_INDEX             = 52,
    ANTENCFG_LTE_BAND54_INDEX             = 53,
    ANTENCFG_LTE_BAND55_INDEX             = 54,
    ANTENCFG_LTE_BAND56_INDEX             = 55,
    ANTENCFG_LTE_BAND57_INDEX             = 56,
    ANTENCFG_LTE_BAND58_INDEX             = 57,
    ANTENCFG_LTE_BAND59_INDEX             = 58,
    ANTENCFG_LTE_BAND60_INDEX             = 59,
    ANTENCFG_LTE_BAND61_INDEX             = 60,
    ANTENCFG_LTE_BAND62_INDEX             = 61,
    ANTENCFG_LTE_BAND63_INDEX             = 62,
    ANTENCFG_LTE_BAND64_INDEX             = 63
};

/* TD-SCDMA��Ƶ�ζ�ӦNV 50464������±�ֵ */
enum
{
    ANTENCFG_TDSCDMA_BAND_A_INDEX         = 0,
    ANTENCFG_TDSCDMA_BAND_B_INDEX         = 1,
    ANTENCFG_TDSCDMA_BAND_C_INDEX         = 2,
    ANTENCFG_TDSCDMA_BAND_D_INDEX         = 3,
    ANTENCFG_TDSCDMA_BAND_E_INDEX         = 4,
    ANTENCFG_TDSCDMA_BAND_F_INDEX         = 5
};

/* Ƶ��GPIO�Ƿ����� */
enum
{
    ANTENCFG_BAND_GPIO_DEFAULT            = 0,  /* δ���� */
    ANTENCFG_BAND_GPIO_CONFIGURE          = 1   /* ������ */
};

/* �Ƿ�֧��ĳ��Ƶ�� */
enum
{
    ANTENCFG_BAND_NOT_SUPPORT            = 0,  /* δ���� */
    ANTENCFG_BAND_SUPPORT                = 1   /* ������ */
};
#endif /* MBB_ANTENNA_TUNER */

/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


typedef struct
{
    VOS_UINT32                          ulStandbyTime;                          /* ����������״̬��ʱ�䳤�ȣ�(��λΪms),ȡֵ��Χ 0~65535,Ĭ��Ϊ5000�� */
    VOS_UINT32                          ulSwitchTime;                           /* ����ָ��PC�·������������������״̬���л�ʱ�䡣����λΪms����ȡֵ��ΧΪ 0~65535��Ĭ��Ϊ500�� */
} DRV_AGENT_PSTANDBY_REQ_STRU;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT8                           aucContent[4];
} DRV_AGENT_MSG_STRU;


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/



typedef enum
{
    GAS_AT_CMD_GAS_STA          =   0,  /* GAS״̬ */
    GAS_AT_CMD_NCELL            =   1,  /* ����״̬ */
    GAS_AT_CMD_SCELL            =   2,  /* ����С��״̬ */
    GAS_AT_CMD_MEASURE_DOWN     =   3,  /* �������� */
    GAS_AT_CMD_MEASURE_UP       =   4,  /* �������� */
    GAS_AT_CMD_FREQ_LOCK        =   5,  /* ��Ƶ�㣬��������ѡ */
    GAS_AT_CMD_FREQ_UNLOCK      =   6,  /* ���� */
    GAS_AT_CMD_FREQ_SPEC_SEARCH =   7,  /* ָ��Ƶ������ */
    GAS_AT_CMD_BUTT
}GAS_AT_CMD_ENUM;

typedef VOS_UINT8 GAS_AT_CMD_ENUM_U8;


typedef struct
{
    GAS_AT_CMD_ENUM_U8                  ucCmd;
    VOS_UINT8                           ucParaNum;
    VOS_UINT16                          usReserve;
    VOS_UINT32                          aulPara[10];
}GAS_AT_CMD_STRU;


typedef struct
{
    VOS_UINT32                          ulRsltNum;
    VOS_UINT32                          aulRslt[20];
}GAS_AT_RSLT_STRU;


typedef struct
{
    AT_APPCTRL_STRU                      stAtAppCtrl;                                   /* AT�û����ƽṹ */
    DRV_AGENT_MSID_QRY_ERROR_ENUM_UINT32 ulResult;                                      /* ��Ϣ������ */
    VOS_INT8                             acModelId[TAF_MAX_MODEL_ID_LEN + 1];           /* ģ����Ϣ */
    VOS_INT8                             acSoftwareVerId[TAF_MAX_REVISION_ID_LEN + 1];  /* ����汾��Ϣ */
    VOS_UINT8                            aucImei[TAF_PH_IMEI_LEN];
} DRV_AGENT_MSID_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucLoadDspMode;
    VOS_UINT8                           ucDeviceRatMode;
} DRV_AGENT_TSELRF_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                          stAtAppCtrl;
    DRV_AGENT_APPDMVER_QRY_ERROR_ENUM_UINT32 enResult;
    VOS_INT8                                 acPdmver[AT_MAX_PDM_VER_LEN + 1];
    VOS_UINT8                                aucReserved1[3];
} DRV_AGENT_APPDMVER_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                           stAtAppCtrl;
    DRV_AGENT_DLOADINFO_QRY_ERROR_ENUM_UINT32 enResult;
    VOS_UCHAR                                 aucDlodInfo[TAF_DLOAD_INFO_LEN];
} DRV_AGENT_DLOADINFO_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                              stAtAppCtrl;
    DRV_AGENT_AUTHORITYVER_QRY_ERROR_ENUM_UINT32 enResult;
    VOS_UINT8                                    aucAuthority[TAF_AUTHORITY_LEN + 1];
    VOS_UINT8                                    aucReserved1[1];
} DRV_AGENT_AUTHORITYVER_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                             stAtAppCtrl;
    DRV_AGENT_AUTHORITYID_QRY_ERROR_ENUM_UINT32 enResult;
    VOS_UINT8                                   aucAuthorityId[TAF_AUTH_ID_LEN + 1];
    VOS_UINT8                                   aucReserved1[1];
} DRV_AGENT_AUTHORITYID_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT8                           aucPfVer[TAF_MAX_PROPLAT_LEN + 1];
    VOS_CHAR                            acVerTime[AT_AGENT_DRV_VERSION_TIME_LEN];
}DRV_AGENT_PFVER_INFO_STRU;


typedef struct
{
    AT_APPCTRL_STRU                       stAtAppCtrl;
    DRV_AGENT_PFVER_QRY_ERROR_ENUM_UINT32 enResult;
    DRV_AGENT_PFVER_INFO_STRU             stPfverInfo;
} DRV_AGENT_PFVER_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                               stAtAppCtrl;
    DRV_AGENT_SDLOAD_SET_ERROR_ENUM_UINT32        enResult;
} DRV_AGENT_SDLOAD_SET_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                               stAtAppCtrl;
    DRV_AGENT_DLOADVER_QRY_ERROR_ENUM_UINT32      enResult;
    VOS_CHAR                                      aucVersionInfo[TAF_MAX_VER_INFO_LEN + 1];
    VOS_UINT8                                     ucReserved1;
} DRV_AGENT_DLOADVER_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                          stAtAppCtrl;
    DRV_AGENT_GODLOAD_SET_ERROR_ENUM_UINT32  enResult;
} DRV_AGENT_GODLOAD_SET_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                          stAtAppCtrl;
    DRV_AGENT_HWNATQRY_QRY_ERROR_ENUM_UINT32 enResult;
    VOS_UINT                                 ulNetMode;
} DRV_AGENT_HWNATQRY_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                          stAtAppCtrl;
    DRV_AGENT_CPULOAD_QRY_ERROR_ENUM_UINT32  enResult;
    VOS_UINT32                               ulCurACpuLoad;
    VOS_UINT32                               ulCurCCpuLoad;
} DRV_AGENT_CPULOAD_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;
    DRV_AGENT_MFREELOCKSIZE_QRY_ERROR_ENUM_UINT32   enResult;
    VOS_INT32                                       lMaxFreeLockSize;
}DRV_AGENT_MFREELOCKSIZE_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT32                              ulBlockCount;                       /*Block����*/
    VOS_UINT32                              ulPageSize;                         /*pageҳ��С*/
    VOS_UINT32                              ulPgCntPerBlk;                      /*һ��Block�е�page����*/
}DRV_AGENT_DLOAD_FLASH_STRU;


typedef struct
{
    AT_APPCTRL_STRU                           stAtAppCtrl;
    DRV_AGENT_FLASHINFO_QRY_ERROR_ENUM_UINT32 enResult;
    DRV_AGENT_DLOAD_FLASH_STRU                stFlashInfo;
} DRV_AGENT_FLASHINFO_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                           stAtAppCtrl;
    DRV_AGENT_AUTHVER_QRY_ERROR_ENUM_UINT32   enResult;
    VOS_UINT32                                ulSimLockVersion;
} DRV_AGENT_AUTHVER_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                          stAtAppCtrl;
    DRV_AGENT_APPDMVER_QRY_ERROR_ENUM_UINT32 enResult;
} DRV_AGENT_SDLOAD_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucType;                                 /* �������: COMP_TYPE_I */
    VOS_UINT8                           ucMode;
    VOS_UINT8                           aucReserved[1];                         /* ����ָ�� */
    VOS_UINT8                           ucLen;                                  /* ���ݳ��� */
    VOS_CHAR                            aucData[AT_MAX_VERSION_LEN];
} DRV_AGENT_VERSION_CTRL_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT32                          ulResult;
    GAS_AT_CMD_ENUM_U8                  ucCmd;
    VOS_UINT8                           aucReserved[3];
    GAS_AT_RSLT_STRU                    stAtCmdRslt;
} DRV_AGENT_GAS_MNTN_CMD_CNF_STRU;


typedef struct
{
    VOS_UINT16                          usDspBand;       /*DSP��ʽ��Ƶ��ֵ*/
    VOS_UINT8                           ucRxOnOff;       /* Rx on offֵ*/
    VOS_UINT8                           ucReserve;       /*����λ*/
    VOS_UINT32                          ulSetLowBands;   /*ת��λ���ָ�ʽ�ķּ���λ*/
    VOS_UINT32                          ulSetHighBands;  /*ת��λ���ָ�ʽ�ķּ���λ*/
} AT_DRV_AGENT_RXDIV_SET_STRU;

typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;        /*��Ϣͷ*/
    VOS_UINT32                          ulResult;           /*���ؽ��*/
    VOS_UINT8                           ucRxOnOff;          /*RX�Ƿ��Ѿ���*/
    VOS_UINT8                           ucReserve;          /*����λ*/
    VOS_UINT16                          usSetDivBands;      /*Ҫ���õ�BAND*/
} DRV_AGENT_AT_RXDIV_CNF_STRU;

typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;        /*��Ϣͷ*/
    VOS_UINT32                          ulResult;           /*���ؽ��*/
    VOS_UINT16                          usDrvDivBands;      /*֧�ֵ�BAND*/
    VOS_UINT16                          usCurBandSwitch;    /*��ǰ���õ�BAND*/
} DRV_AGENT_QRY_RXDIV_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulPwdLen;
    VOS_UINT8                           aucPwd[TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX +4];
}DRV_AGENT_SIMLOCK_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;                            /*��Ϣͷ*/
    VOS_UINT32                          ulResult;                               /*���ؽ��*/
}DRV_AGENT_SET_SIMLOCK_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;                            /*��Ϣͷ*/
    VOS_UINT32                          ulResult;                               /*���ؽ��*/
    VOS_UINT8                           aucHwVer[DRV_AGENT_MAX_HARDWARE_LEN+1]; /*Ӳ���汾��*/
} DRV_AGENT_HARDWARE_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;                                        /*��Ϣͷ*/
    VOS_UINT32                          ulResult;                                           /*���ؽ��*/
    VOS_UINT8                           aucModelId[DRV_AGENT_MAX_MODEL_ID_LEN + 1];         /*MODE ID��*/
    VOS_UINT8                           aucRevisionId[DRV_AGENT_MAX_REVISION_ID_LEN + 1];   /*����汾��*/
    VOS_UINT8                           aucHwVer[DRV_AGENT_MAX_HARDWARE_LEN+1];             /*Ӳ���汾��*/
} DRV_AGENT_FULL_HARDWARE_QRY_CNF_STRU;



typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;                            /*��Ϣͷ*/
    VOS_UINT32                          ulResult;                               /*���ؽ��*/
} DRV_AGENT_NVRESTORE_RST_STRU;


#define AT_SD_DATA_UNIT_LEN             (512)
#define AT_GPIOPL_MAX_LEN               (20)



typedef struct
{
    VOS_UINT32                          enGpioOper;
    VOS_UINT8                           aucGpiopl[AT_GPIOPL_MAX_LEN];

}DRV_AGENT_GPIOPL_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_BOOL                            bFail;

}DRV_AGENT_GPIOPL_SET_CNF_STRU;



typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_BOOL                            bFail;
    VOS_UINT8                           aucGpiopl[AT_GPIOPL_MAX_LEN];

}DRV_AGENT_GPIOPL_QRY_CNF_STRU;
#ifdef BSP_CONFIG_BOARD_SOLUTION
#define AT_GPIOLOOP_MAX_LEN               (20)


typedef struct
{
    VOS_UINT8                           aucGpioloop;
}DRV_AGENT_GPIOLOOP_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_BOOL                            bFail;
}DRV_AGENT_GPIOLOOP_SET_CNF_STRU;



typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_BOOL                            bFail;
    VOS_UINT8                           aucGpioloop[AT_GPIOLOOP_MAX_LEN];
}DRV_AGENT_GPIOLOOP_QRY_CNF_STRU;
#endif
#if(FEATURE_ON == MBB_FEATURE_CIMEI)

typedef struct
{
    VOS_UINT8               aucPw[TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX + 1];
}DRV_AGENT_DEVNUMW_SET_REQ_PW_STRU;


typedef struct
{
    VOS_UINT8                auGlobalImeiVariable[TAF_PH_IMEI_LEN];
}DRV_AGENT_DEVNUMW_SET_REQ_IMEI_STRU;

#endif

typedef struct
{
    VOS_UINT8                           aucPwd[TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX +1];

}DRV_AGENT_DATALOCK_SET_REQ_STRU;



typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_BOOL                            bFail;           /* DATALOCKУ���Ƿ�ɹ� */

}DRV_AGENT_DATALOCK_SET_CNF_STRU;


typedef struct
{

    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_BOOL                            bFail;
    VOS_INT32                           lBatVol;

}DRV_AGENT_TBATVOLT_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_BOOL                            bFail;

}DRV_AGENT_TMODE_SET_CNF_STRU;


enum DRV_AGENT_VERSION_QRY_ERROR_ENUM
{
    DRV_AGENT_VERSION_QRY_NO_ERROR              = 0,
    DRV_AGENT_VERSION_QRY_VERTIME_ERROR         = 1,

    DRV_AGENT_VERSION_QRY_BUTT
};
typedef VOS_UINT32 DRV_AGENT_VERSION_QRY_ERROR_ENUM_UINT32;



typedef struct
{
    AT_APPCTRL_STRU                          stAtAppCtrl;
    TAF_PH_REVISION_ID_STRU                  stSoftVersion;                                 /*����汾��*/
    TAF_PH_HW_VER_STRU                       stFullHwVer;                                   /*�ⲿӲ���汾��*/
    TAF_PH_HW_VER_STRU                       stInterHwVer;                                  /*�ڲ�Ӳ���汾��*/
    TAF_PH_MODEL_ID_STRU                     stModelId;                                     /*�ⲿ��ƷID */
    TAF_PH_MODEL_ID_STRU                     stInterModelId;                                /*�ڲ���ƷID */
    TAF_PH_CDROM_VER_STRU                    stIsoVer;                                      /*��̨����汾��*/
    DRV_AGENT_VERSION_QRY_ERROR_ENUM_UINT32  enResult;                                      /*������ö�� */
    VOS_CHAR                                 acVerTime[AT_AGENT_DRV_VERSION_TIME_LEN];      /*����ʱ��*/

}DRV_AGENT_VERSION_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT32                          BandWCDMA_I_2100      :1;
    VOS_UINT32                          BandWCDMA_II_1900     :1;
    VOS_UINT32                          BandWCDMA_III_1800    :1;
    VOS_UINT32                          BandWCDMA_IV_1700     :1;
    VOS_UINT32                          BandWCDMA_V_850       :1;
    VOS_UINT32                          BandWCDMA_VI_800      :1;
    VOS_UINT32                          BandWCDMA_VII_2600    :1;
    VOS_UINT32                          BandWCDMA_VIII_900    :1;
    VOS_UINT32                          BandWCDMA_IX_J1700    :1;
    VOS_UINT32                          BandSpare1            :1;
    VOS_UINT32                          BandWCDMA_XI_1500     :1;
    VOS_UINT32                          BandSpare7            :7;
    VOS_UINT32                          BandWCDMA_XIX_850     :1;
    VOS_UINT32                          BandSpare13           :13;
}AT_WCDMA_PREF_BAND_STRU;


typedef struct
{
    VOS_UINT32                          BandGsm450      :1;
    VOS_UINT32                          BandGsm480      :1;
    VOS_UINT32                          BandGsm850      :1;
    VOS_UINT32                          BandGsmP900     :1;
    VOS_UINT32                          BandGsmR900     :1;
    VOS_UINT32                          BandGsmE900     :1;
    VOS_UINT32                          BandGsm1800     :1;
    VOS_UINT32                          BandGsm1900     :1;
    VOS_UINT32                          BandGsm700      :1;
    VOS_UINT32                          BandSpare7      :23;

}AT_GSM_PREF_BAND_STRU;


typedef union
{
    VOS_UINT32                          ulBand;
    AT_WCDMA_PREF_BAND_STRU             BitBand;
}AT_WCDMA_BAND_SET_UN;


typedef union
{
    VOS_UINT32                          ulBand;
    AT_GSM_PREF_BAND_STRU               BitBand;
}AT_GSM_BAND_SET_UN;



typedef struct
{
    VOS_UINT32                          ulUeGSptBand;
    VOS_UINT32                          ulUeWSptBand;
    VOS_UINT32                          ulAllUeBand;
    AT_WCDMA_BAND_SET_UN                unWRFSptBand;
    AT_GSM_BAND_SET_UN                  unGRFSptBand;
}AT_UE_BAND_CAPA_ST;


enum AT_FEATURE_DRV_TYPE_ENUM
{
    AT_FEATURE_LTE = 0,
    AT_FEATURE_HSPAPLUS,
    AT_FEATURE_HSDPA,
    AT_FEATURE_HSUPA,
    AT_FEATURE_DIVERSITY,
    AT_FEATURE_UMTS,
    AT_FEATURE_EVDO,
    AT_FEATURE_EDGE,
    AT_FEATURE_GPRS,
    AT_FEATURE_GSM,
    AT_FEATURE_CDMA,
    AT_FEATURE_WIMAX,
    AT_FEATURE_WIFI,
    AT_FEATURE_GPS,
    AT_FEATURE_TDSCDMA
#if (FEATURE_ON == MBB_COMMON)
    ,AT_FEATURE_BATTERY,
    AT_FEATURE_EXTCHARGE,
    AT_FEATURE_SDCARD,
    AT_FEATURE_WIRELESS,
    AT_FEATURE_USB3_0,
    AT_FEATURE_LAN,
#if (FEATURE_ON == MBB_BLUETOOTH)
    AT_FEATURE_BLUETOOTH,
#endif
#if (FEATURE_ON == MBB_USB_TYPEC)
    AT_FEATURE_TYPEC,
#endif
#if (FEATURE_ON == MBB_NFC)
    AT_FEATURE_NFC,
#endif
#endif
};
typedef VOS_UINT32  AT_FEATURE_DRV_TYPE_ENUM_UINT32;

#if (FEATURE_ON == MBB_COMMON)
    #define AT_FEATURE_NAME_LEN_MAX       (21)
#else
    #define AT_FEATURE_NAME_LEN_MAX       (16)
#endif
#define AT_FEATURE_CONTENT_LEN_MAX    (56)
#if (FEATURE_ON == MBB_NFC)
#define AT_FEATURE_CONTENT_NFC_LEN_MAX (16)
#endif



typedef struct
{
    VOS_UINT8        ucFeatureFlag;
    VOS_UINT8        aucFeatureName[AT_FEATURE_NAME_LEN_MAX];
    VOS_UINT8        aucContent[AT_FEATURE_CONTENT_LEN_MAX];
}AT_FEATURE_SUPPORT_ST;


typedef struct
{
    VOS_UINT8                           ucPowerClass;       /* UE���ʼ���                      */
    VOS_UINT8                           enTxRxFreqSeparate; /* Tx/Rx Ƶ������                  */
    VOS_UINT8                           aucReserve1[2];     /* 4�ֽڶ��룬����                 */
} AT_RF_CAPA_STRU;



typedef struct
{
    VOS_UINT32                          ulHspaStatus;       /* 0��ʾδ����,��ôDPA��UPA��֧��;1��ʾ����           */
    VOS_UINT8                           ucMacEhsSupport;
    VOS_UINT8                           ucHSDSCHPhyCat;
    VOS_UINT8                           aucReserve1[2];     /* ����һ��������ulHspaStatus���п��ƣ��˱�������ʹ�� */
    AT_RF_CAPA_STRU                     stRfCapa;           /* RF ������Ϣ  */
    VOS_UINT8                           enDlSimulHsDschCfg; /* ENUMERATED  OPTIONAL  */
    VOS_UINT8                           enAsRelIndicator;   /* Access Stratum Release Indicator  */
    VOS_UINT8                           ucHSDSCHPhyCategory; /* ֧��HS-DSCH���������ͱ�־ */
    VOS_UINT8                           enEDCHSupport;       /* �Ƿ�֧��EDCH�ı�־ */
    VOS_UINT8                           ucEDCHPhyCategory;   /* ֧��UPA�ĵȼ� */
    VOS_UINT8                           enFDPCHSupport;      /* �Ƿ�֧��FDPCH�ı�־  */
    VOS_UINT8                           ucV690Container;     /* V690Container��д */
    VOS_UINT8                           enHSDSCHSupport;     /* �Ƿ�֧��enHSDSCHSupport�ı�־  */
} AT_UE_CAPA_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    AT_UE_BAND_CAPA_ST                  stBandFeature;

}DRV_AGENT_SFEATURE_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_BOOL                            bFail;
}DRV_AGENT_SECUBOOT_SET_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_BOOL                            bFail;
    VOS_UINT8                           ucSecuBootEnable;
    VOS_UINT8                           aucReserve1[3];                          /* 4�ֽڶ��룬���� */
}DRV_AGENT_SECUBOOT_QRY_CNF_STRU;



typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_BOOL                            bFail;
    VOS_UINT8                           ucSecuBootFeature;
    VOS_UINT8                           aucReserve1[7];                          /* 4�ֽڶ��룬���� */
}DRV_AGENT_SECUBOOTFEATURE_QRY_CNF_STRU;


enum AT_DEVICE_CMD_BAND_ENUM
{
    AT_BAND_2100M = 0,
    AT_BAND_1900M,
    AT_BAND_1800M,
    AT_BAND_1700M,
    AT_BAND_1600M,
    AT_BAND_1500M,
    AT_BAND_900M,
    AT_BAND_850M,
    AT_BAND_800M,
    AT_BAND_450M,
    AT_BAND_2600M,
    AT_BAND_1X_BC0,
    AT_BAND_WIFI = 15,
    AT_BAND_BUTT
};
typedef VOS_UINT8 AT_DEVICE_CMD_BAND_ENUM_UINT8;


typedef struct
{
    VOS_UINT16                          usUlArfcn;  /*���е�Channel No*/
    VOS_UINT16                          usDlArfcn;  /*���е�Channel No*/
    VOS_UINT16                          usDspBand;  /*DSP��ʽ��Ƶ��ֵ*/
    VOS_UINT8                           aucReserved[2];
}AT_DSP_BAND_ARFCN_STRU;


typedef struct
{
    VOS_UINT16             usArfcnMin;
    VOS_UINT16             usArfcnMax;
}AT_ARFCN_RANGE_STRU;


enum AT_TMODE_ENUM
{
    AT_TMODE_NORMAL = 0,   /*����ģʽ,��������״̬*/
    AT_TMODE_FTM ,         /*������ģʽ,У׼ģʽ*/
    AT_TMODE_UPGRADE ,     /*����ģʽ,�汾����ʱʹ�� */
    AT_TMODE_RESET,        /* ��������  */
    AT_TMODE_POWEROFF,
    AT_TMODE_SIGNALING = 11,
    AT_TMODE_OFFLINE,
#if((FEATURE_ON == FEATURE_LTE) || (FEATURE_ON == FEATURE_UE_MODE_TDS))
    AT_TMODE_F_NONESIGNAL   = 13,
    AT_TMODE_SYN_NONESIGNAL = 14,
    AT_TMODE_SET_SLAVE      = 15,
    AT_TMODE_GU_BT          = 16,
    AT_TMODE_TDS_FAST_CT    = 17,
    AT_TMODE_TDS_BT         = 18,
    AT_TMODE_COMM_CT        = 19,
#endif
    AT_TMODE_BUTT
};
typedef VOS_UINT8  TAF_PH_TMODE_ENUM_UINT8;


typedef struct
{
    VOS_UINT8                           ucLoadDspMode;
    VOS_UINT8                           ucCurrentDspMode;
    VOS_UINT16                          usChannelNo;
    AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8   ucDeviceRatMode;
    AT_DEVICE_CMD_BAND_ENUM_UINT8       ucDeviceAtBand;
    VOS_UINT8                           aucReserved[2];
    VOS_BOOL                            bDspLoadFlag;
    AT_DSP_BAND_ARFCN_STRU              stDspBandArfcn;
}DRV_AGENT_FCHAN_SET_REQ_STRU;



enum DRV_AGENT_FCHAN_SET_ERROR_ENUM
{
    DRV_AGENT_FCHAN_SET_NO_ERROR                            = 0,
    DRV_AGENT_FCHAN_BAND_NOT_MATCH                          = 1,
    DRV_AGENT_FCHAN_BAND_CHANNEL_NOT_MATCH                  = 2,
    DRV_AGENT_FCHAN_OTHER_ERR                               = 3,

    DRV_AGENT_FCHAN_SET_BUTT
};
typedef VOS_UINT32 DRV_AGENT_FCHAN_SET_ERROR_ENUM_UINT32;



typedef struct
{
    AT_APPCTRL_STRU                       stAtAppCtrl;
    DRV_AGENT_FCHAN_SET_REQ_STRU          stFchanSetReq;
    DRV_AGENT_FCHAN_SET_ERROR_ENUM_UINT32 enResult;

}DRV_AGENT_FCHAN_SET_CNF_STRU;

#if(FEATURE_ON == MBB_NOISETOOL)

typedef struct
{
    VOS_UINT16         usMode;             /*UE ģʽ*/
    VOS_UINT16         usBand;             /*Ƶ��*/
    VOS_UINT16         usDLStFreq;       /*����ɨƵ��ʼƵ��*/
    VOS_UINT16         usDLEdFreq;       /*����ɨƵ����Ƶ��*/
}DRV_AGENT_NOISECFG_SET_REQ_STRU;

typedef struct
{
    AT_APPCTRL_STRU                       stAtAppCtrl;/* ��Ϣͷ */
    VOS_UINT32                                ulResult;
}DRV_AGENT_NOISECFG_SET_CNF_STRU;
#endif/*(FEATURE_ON == MBB_NOISETOOL)*/


#define AT_MS_SUPPORT_RX_DIV_2100           0x0001
#define AT_MS_SUPPORT_RX_DIV_1900           0x0002
#define AT_MS_SUPPORT_RX_DIV_1800           0x0004
#define AT_MS_SUPPORT_RX_DIV_AWS_1700       0x0008
#define AT_MS_SUPPORT_RX_DIV_850            0x0010
#define AT_MS_SUPPORT_RX_DIV_800            0x0020
#define AT_MS_SUPPORT_RX_DIV_2600           0x0040
#define AT_MS_SUPPORT_RX_DIV_900            0x0080
#define AT_MS_SUPPORT_RX_DIV_IX_1700        0x0100



enum AT_DSP_RF_ON_OFF_ENUM
{
    AT_DSP_RF_SWITCH_OFF = 0,
    AT_DSP_RF_SWITCH_ON,
    AT_DSP_RF_SWITCH_BUTT
};
typedef VOS_UINT8 AT_DSP_RF_ON_OFF_ENUM_UINT8;


typedef struct
{
    VOS_PID                             ulPid;
    VOS_UINT32                          ulMsgPeakSize;
    VOS_UINT32                          ulMemPeakSize;
}AT_PID_MEM_INFO_PARA_STRU;


typedef struct
{
    VOS_UINT32                          ulSetLowBands;
    VOS_UINT32                          ulSetHighBands;
    AT_DSP_BAND_ARFCN_STRU              stDspBandArfcn;
}DRV_AGENT_RXPRI_SET_REQ_STRU;



typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    DRV_AGENT_ERROR_ENUM_UINT32         enResult;
    DRV_AGENT_RXPRI_SET_REQ_STRU        stRxpriSetReq;
    VOS_UINT16                          usWPriBands;
    VOS_UINT16                          usGPriBands;
    VOS_UINT16                          usSetPriBands;
    VOS_UINT8                           aucReserved[2];
}DRV_AGENT_RXPRI_SET_CNF_STRU;



typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT32                          ulProdType;

}DRV_AGENT_PRODTYPE_QRY_CNF_STRU;



typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT32                          ulTbatType;

}DRV_AGENT_TBAT_QRY_CNF_STRU;



typedef struct
{
    AT_APPCTRL_STRU                      stAtAppCtrl;
    VOS_UINT16                           usWDrvPriBands;
    VOS_UINT16                           usGDrvPriBands;
    VOS_BOOL                             bFail;
}DRV_AGENT_RXPRI_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT32                  ulIsEnable;           /*�Ƿ���Ҫ������������, 0Ϊ�رգ�1Ϊ����*/
    VOS_INT32                   lCloseAdcThreshold;   /*����͹���ģʽ���¶�����*/
    VOS_INT32                   lAlarmAdcThreshold;   /*��Ҫ���и����������¶�����*/
    VOS_INT32                   lResumeAdcThreshold;  /*�ָ�������ģʽ���¶�����*/
}SPY_TEMP_THRESHOLD_PARA_STRU;


typedef struct
{
    AT_APPCTRL_STRU                        stAtAppCtrl;
    VOS_BOOL                               bFail;

}DRV_AGENT_ADC_SET_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT8                           ucLen;
    VOS_UINT8                           aucReserved[2];
    VOS_CHAR                            aucData[AT_AGENT_DRV_VERSION_TIME_LEN+1];
} DRV_AGENT_VERSION_TIME_STRU;

/*****************************************************************************
 �ṹ��    : DRV_AGENT_YJCX_SET_CNF_STRU
 �ṹ˵��  : AT��AT AGENT AT^YJCX��������ظ��ṹ
*****************************************************************************/
typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT32                          ulResult;
    VOS_UCHAR                           aucflashInfo[TAF_MAX_FLAFH_INFO_LEN + 1];
} DRV_AGENT_YJCX_SET_CNF_STRU;

/*****************************************************************************
 �ṹ��    : DRV_AGENT_YJCX_QRY_CNF_STRU
 �ṹ˵��  : AT��AT AGENT AT^YJCX��������ظ��ṹ
*****************************************************************************/
typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT32                          ulResult;
    VOS_UCHAR                           aucgpioInfo[TAF_MAX_GPIO_INFO_LEN + 1];
    VOS_UINT8                           aucReserved[3];
} DRV_AGENT_YJCX_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT32                          ulCurImsiSign;
    VOS_UINT16                          usDualIMSIEnable;
    VOS_UINT8                           aucReserved[2];
}DRV_AGENT_IMSICHG_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT32                          ulRslt;
}DRV_AGENT_INFORBU_SET_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT32                          ulRslt;
}DRV_AGENT_INFORRS_SET_CNF_STRU;



typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_BOOL                            bNormalSrvStatus;
    VOS_UINT32                          ulPnnExistFlg;
    VOS_UINT32                          ulOplExistFlg;

}DRV_AGENT_CPNN_TEST_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT32                          ulRslt;
}DRV_AGENT_NVBACKUP_SET_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_BOOL                            bNormalSrvStatus;
}DRV_AGENT_CPNN_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                         stAtAppCtrl;
    VOS_UINT32                              ulResult;
    AT_MEMQUERY_PARA_ENUM_UINT32            ulMemQryType;                       /* ��ѯ�����ͣ�0:VOS�ڴ�, 1: TTF�ڴ� */
    VOS_UINT32                              ulPidNum;                           /* PID�ĸ��� */
    VOS_UINT8                               aucData[4];                         /* ÿ��PID���ڴ�ʹ����� */
}DRV_AGENT_MEMINFO_QRY_RSP_STRU;


typedef struct
{
    AT_APPCTRL_STRU                         stAtAppCtrl;
    DRV_AGENT_TSELRF_SET_ERROR_ENUM_UINT32  enResult;
    VOS_UINT8                               ucDeviceRatMode;
    VOS_UINT8                               aucReserved[3];
}DRV_AGENT_TSELRF_SET_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                             stAtAppCtrl;
    DRV_AGENT_HKADC_GET_ERROR_ENUM_UINT32  enResult;
    VOS_INT32                                                   TbatHkadc;
}DRV_AGENT_HKADC_GET_CNF_STRU;



#if (FEATURE_ON == FEATURE_SECURITY_SHELL)

typedef struct
{
    VOS_CHAR                            acShellPwd[AT_SHELL_PWD_LEN];
}DRV_AGENT_SPWORD_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT32                          ulResult;
}DRV_AGENT_SPWORD_SET_CNF_STRU;
#endif


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;                            /* ��Ϣͷ */
    VOS_UINT32                          ulResult;                               /* ���ؽ�� */
    VOS_UINT32                          ulNvBackupStat;                         /* NV����״̬ */
}DRV_AGENT_NVBACKUPSTAT_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;                            /* ��Ϣͷ */
    VOS_UINT32                          ulResult;                               /* ���ؽ�� */
    VOS_UINT32                          ulBadBlockNum;                          /* �������� */
    VOS_UINT32                          aulBadBlockIndex[0];                    /* ���������� */
} DRV_AGENT_NANDBBC_QRY_CNF_STRU;



#define DRV_AGENT_NAND_MFU_NAME_MAX_LEN     16                                  /* ����������󳤶� */
#define DRV_AGENT_NAND_DEV_SPEC_MAX_LEN     32                                  /* �豸�����󳤶� */

typedef struct
{
    VOS_UINT32      ulMufId;                                                    /* ����ID */
    VOS_UINT8       aucMufName[DRV_AGENT_NAND_MFU_NAME_MAX_LEN];                /* ���������ַ��� */
    VOS_UINT32      ulDevId;                                                    /* �豸ID */
    VOS_UINT8       aucDevSpec[DRV_AGENT_NAND_DEV_SPEC_MAX_LEN];                /* �豸����ַ��� */
} DRV_AGENT_NAND_DEV_INFO_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;                 /* ��Ϣͷ */
    VOS_UINT32                          ulResult;                    /* ���ؽ�� */
    DRV_AGENT_NAND_DEV_INFO_STRU        stNandDevInfo;               /* �豸��Ϣ */
} DRV_AGENT_NANDVER_QRY_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;                 /* ��Ϣͷ */
    VOS_UINT32                          ulResult;                    /* ���ؽ�� */
    VOS_INT                             lGpaTemp;                    /* G PA�¶� */
    VOS_INT                             lWpaTemp;                    /* W PA�¶� */
    VOS_INT                             lLpaTemp;                    /* L PA�¶� */
    VOS_INT                             lSimTemp;                    /* SIM���¶� */
    VOS_INT                             lBatTemp;                    /* ����¶� */
} DRV_AGENT_CHIPTEMP_QRY_CNF_STRU;




typedef struct
{
    VOS_UINT32                          ulState;                    /* ����״̬ */

} DRV_AGENT_ANTSWITCH_SET_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;                 /* ��Ϣͷ */
    VOS_UINT32                          ulResult;                    /* ���ؽ�� */

} DRV_AGENT_ANTSWITCH_SET_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;                 /* ��Ϣͷ */
    VOS_UINT32                          ulState;                     /* ����״̬ */
    VOS_UINT32                          ulResult;                    /* ���ؽ�� */

} DRV_AGENT_ANTSWITCH_QRY_CNF_STRU;




typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;                 /* ��Ϣͷ */
    VOS_UINT16                          usAntState;
    VOS_UINT8                           aucRsv[2];
} DRV_AGENT_ANT_STATE_IND_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT32                          ulResult;
}DRV_AGENT_MAX_LOCK_TMS_SET_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulUsimState;
}DRV_AGENT_AP_SIMST_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT32                          ulResult;
}DRV_AGENT_AP_SIMST_SET_CNF_STRU;



typedef struct
{
    VOS_UINT8                           aucHUK[DRV_AGENT_HUK_LEN];  /* HUK���� 128Bits */
}DRV_AGENT_HUK_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32       enResult;                   /* ����ִ�н�� */
}DRV_AGENT_HUK_SET_CNF_STRU;


typedef struct
{
    /* ��Ȩ��Կ���� */
    VOS_UINT8                           aucPubKey[DRV_AGENT_PUBKEY_LEN];
    /* ��Ȩ��ԿSSKǩ������ */
    VOS_UINT8                           aucPubKeySign[DRV_AGENT_PUBKEY_SIGNATURE_LEN];
}DRV_AGENT_FACAUTHPUBKEY_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32       enResult;                   /* ����ִ�н�� */
}DRV_AGENT_FACAUTHPUBKEY_SET_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32       enResult;                   /* ����ִ�н�� */
    /* ʹ�ù�Կ����RSA���ܺ������ */
    VOS_UINT8                                       aucRsaText[DRV_AGENT_RSA_CIPHERTEXT_LEN];
}DRV_AGENT_IDENTIFYSTART_SET_CNF_STRU;


typedef struct
{
    /* ʹ��˽Կ����RSA���ܺ������ */
    VOS_UINT8                           aucRsaText[DRV_AGENT_RSA_CIPHERTEXT_LEN];
}DRV_AGENT_IDENTIFYEND_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32       enResult;                   /* ����ִ�н�� */
}DRV_AGENT_IDENTIFYEND_SET_CNF_STRU;


typedef struct
{
    /* �ṹ������, �����������������͵İ���״̬, �����Ŷ�, CK, UK��������Ϣ */
    VOS_UINT8       aucCategoryData[DRV_AGENT_SIMLOCKDATAWRITE_STRU_LEN];
}DRV_AGENT_SIMLOCKDATAWRITE_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32       enResult;                   /* ����ִ�н�� */
}DRV_AGENT_SIMLOCKDATAWRITE_SET_CNF_STRU;


typedef struct
{
   VOS_UINT8        aucPhLockCodeBegin[DRV_AGENT_PH_LOCK_CODE_LEN];
   VOS_UINT8        aucPhLockCodeEnd[DRV_AGENT_PH_LOCK_CODE_LEN];
}DRV_AGENT_PH_LOCK_CODE_STRU;


typedef struct
{
    DRV_AGENT_PERSONALIZATION_CATEGORY_ENUM_UINT8           enCategory;             /* ����������category��� */
    DRV_AGENT_PERSONALIZATION_INDICATOR_ENUM_UINT8          enIndicator;            /* ���������ļ���ָʾ */
    DRV_AGENT_PERSONALIZATION_STATUS_ENUM_UINT8             enStatus;
    VOS_UINT8                                               ucMaxUnlockTimes;       /* �������������������� */
    VOS_UINT8                                               ucRemainUnlockTimes;    /* ����������ʣ��������� */
    VOS_UINT8                                               aucRsv[3];              /* �����ֽڣ��������ֽڶ��� */
}DRV_AGENT_SIMLOCK_DATA_CATEGORY_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32       enResult;                   /* ����ִ�н�� */
    /* Ŀǰ֧��3��category���ṹ�����鰴��network->network subset->SP��˳������ */
    DRV_AGENT_SIMLOCK_DATA_CATEGORY_STRU            astCategoryData[DRV_AGENT_SUPPORT_CATEGORY_NUM];
}DRV_AGENT_SIMLOCKDATAREAD_QRY_CNF_STRU;


typedef struct
{
    DRV_AGENT_PERSONALIZATION_CATEGORY_ENUM_UINT8           enCategory;             /* ����������category��� */
    DRV_AGENT_PERSONALIZATION_INDICATOR_ENUM_UINT8          enIndicator;            /* ���������ļ���ָʾ */
    VOS_UINT8                                               ucGroupNum;             /* �Ŷθ�����һ��begin/end��һ���Ŷ� */
    VOS_UINT8                                               ucRsv;                  /* 4�ֽڶ���, ����λ */
    /* ���������ĺŶ�����*/
    DRV_AGENT_PH_LOCK_CODE_STRU                             astLockCode[DRV_AGENT_PH_LOCK_CODE_GROUP_NUM];
}DRV_AGENT_SIMLOCK_INFO_CATEGORY_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32       enResult;                   /* ����ִ�н�� */
    /* Ŀǰ֧��3��category���ṹ�����鰴��network->network subset->SP��˳������ */
    DRV_AGENT_SIMLOCK_INFO_CATEGORY_STRU            astCategoryInfo[DRV_AGENT_SUPPORT_CATEGORY_NUM];
}DRV_AGENT_PHONESIMLOCKINFO_QRY_CNF_STRU;


typedef struct
{
    DRV_AGENT_PH_PHYNUM_TYPE_ENUM_UINT8 enPhynumType;                                   /* ��������� */
    VOS_UINT8                           aucPhynumValue[DRV_AGENT_RSA_CIPHERTEXT_LEN];   /* �����RSA�������� */
}DRV_AGENT_PHONEPHYNUM_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32       enResult;                   /* ����ִ�н�� */
}DRV_AGENT_PHONEPHYNUM_SET_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32       enResult;                   /* ����ִ�н�� */
    VOS_UINT8                                       aucImeiRsa[DRV_AGENT_RSA_CIPHERTEXT_LEN];
    VOS_UINT8                                       aucSnRsa[DRV_AGENT_RSA_CIPHERTEXT_LEN];
}DRV_AGENT_PHONEPHYNUM_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT8       aucPortPassword[DRV_AGENT_PORT_PASSWORD_LEN];               /* ͨ�Ŷ˿������� */
}DRV_AGENT_PORTCTRLTMP_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32       enResult;                   /* ����ִ�н�� */
}DRV_AGENT_PORTCTRLTMP_SET_CNF_STRU;


typedef struct
{
    DRV_AGENT_PORT_STATUS_ENUM_UINT32   enPortStatus;                                   /* ͨ�Ŷ˿�״̬ */
    VOS_UINT8                           aucPortPassword[DRV_AGENT_RSA_CIPHERTEXT_LEN];  /* ͨ�Ŷ˿������� */
}DRV_AGENT_PORTATTRIBSET_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32       enResult;                   /* ����ִ�н�� */
}DRV_AGENT_PORTATTRIBSET_SET_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32       enResult;                   /* ����ִ�н�� */
    DRV_AGENT_PORT_STATUS_ENUM_UINT32               enPortStatus;               /* ͨ�Ŷ˿�״̬ */
}DRV_AGENT_PORTATTRIBSET_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT8       aucPortPassword[DRV_AGENT_PORT_PASSWORD_LEN];               /* ͨ�Ŷ˿������� */
}DRV_AGENT_OPWORD_SET_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_PERSONALIZATION_ERR_ENUM_UINT32       enResult;                   /* ����ִ�н�� */
}DRV_AGENT_OPWORD_SET_CNF_STRU;



typedef struct
{
    DRV_AGENT_DH_KEY_TYPE_ENUM_UINT32   enKeyType;                              /* DH��Կ���� */
    VOS_UINT32                          ulKeyLen;                               /* DH��Կ���� */
    VOS_UINT8                           aucKey[DRV_AGENT_DH_PUBLICKEY_LEN];     /* DH��Կ������󳤶����������飬�Ա����ڴ洢��Կ��˽Կ */
}DRV_AGENT_HVPDH_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                                 stAtAppCtrl;                /* ��Ϣͷ */
    DRV_AGENT_HVPDH_ERR_ENUM_UINT32                 enResult;                   /* ����ִ�н�� */
}DRV_AGENT_HVPDH_CNF_STRU;


typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT32                          ulRslt;
}DRV_AGENT_NVMANUFACTUREEXT_SET_CNF_STRU;

#if (FEATURE_ON == MBB_ANTENNA_MODE)
/*****************************************************************************
 �ṹ��    : DRV_AGENT_AT_ANTMODE_CNF_STRU
 �ṹ˵��  : ^ANTMODE����Ʒ������ʽ�������ּ��Ĵ������ṹ��
*****************************************************************************/
typedef TAF_UINT32 (*antmode_configure_nw_func)(at_antmode_ant_modes ant_mode);
typedef TAF_UINT32 (*antmode_qry_mode_func)(at_antmode_ant_modes *ant_mode);
#endif /* MBB_ANTENNA_MODE */

#if (FEATURE_ON == MBB_ANTENNA_TUNER)
typedef struct
{
    TAF_UINT32 band_mask;              /* Ƶ��maskλ����ʾƵ�εĵ�32λ */
    TAF_UINT32 band_mask_ext;          /* ��չ��Ƶ��maskλ����ʾƵ�εĸ�32λ */
    TAF_UINT8 is_supported;            /* �Ƿ�֧�ָ�Ƶ�� */
    TAF_UINT8 is_setted;               /* ��Ƶ���Ƿ����� */
    TAF_UINT8 ant_tuner_gpio_setting;  /* GPIO������ֵ */
    TAF_UINT8 band_index;              /* ��Ƶ���ڶ�ӦNV�е����� */
} antencfg_band_gpio_cfg_type;

/* ��ʽ�����Ϣ�ṹ�� */
typedef struct
{
    antencfg_band_gpio_cfg_type *antencfg_band_info; /* Ƶ������ָ�� */
    TAF_UINT32 nv_id;                                /* ����ʽ��Ӧ��NV ID */
} antencfg_info_type;

typedef struct
{
    TAF_UINT32 adc_hw_version;  /* ADC��������ʵ�ʵ�Ӳ���汾�� */
    TAF_UINT32 at_hw_version;   /* ��Ӧantencfg_band_gpio_cfg_type.hw_version�İ汾�ţ�ÿ����Ʒ��1��bit��ʾ */
} antencfg_hw_version_type;
#endif /* MBB_ANTENNA_TUNER */

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/
/*****************************************************************************
  H2ASN������Ϣ�ṹ����
*****************************************************************************/
typedef struct
{
    DRV_AGENT_MSG_TYPE_ENUM_UINT32      ulMsgId;   /*_H2ASN_MsgChoice_Export DRV_AGENT_MSG_TYPE_ENUM_UINT32*/
    VOS_UINT8                           aucMsgBlock[4];
    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          DRV_AGENT_MSG_TYPE_ENUM_UINT32
    ****************************************************************************/
}DRV_AGENT_MSG_DATA;
/*_H2ASN_Length UINT32*/

typedef struct
{
    VOS_MSG_HEADER
    DRV_AGENT_MSG_DATA                  stMsgData;
}TafDrvAgent_MSG;



typedef struct
{
    VOS_UINT8                           aucSWVer[TAF_MAX_REVISION_ID_LEN + 1];
    VOS_CHAR                            acVerTime[AT_AGENT_DRV_VERSION_TIME_LEN];
}DRV_AGENT_SWVER_INFO_STRU;



typedef struct
{
    AT_APPCTRL_STRU                       stAtAppCtrl;
    DRV_AGENT_ERROR_ENUM_UINT32           enResult;
    DRV_AGENT_SWVER_INFO_STRU             stSwverInfo;
} DRV_AGENT_SWVER_SET_CNF_STRU;
#if (FEATURE_ON == MBB_SIMLOCK_FOUR)

typedef struct
{
    VOS_UINT8   HWLOCKTYPE;         /* �������ý������Ĳ����ַ��� */
    VOS_UINT8   HWLOCKPARA[AT_TAF_HWLOCKPARALEN];         /* �������ý������Ĳ����ַ��� */
    VOS_UINT16  usPara1Len;                             /* ������ʶ�����ַ������� */
}AT_TAF_SET_HWLOCK_REQ_STRU;
#endif

#if (FEATURE_ON == MBB_MODULE_PM)
/*****************************************************************************
 �ṹ��   : DRV_AGENT_SET_CURC_CNF_STRU
 �ṹ˵�� : DRV��AT AGENT curc
*****************************************************************************/
typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;                            /*��Ϣͷ*/
    VOS_UINT32                          ulResult;                               /*���ؽ��*/
}DRV_AGENT_SET_CURC_CNF_STRU;
/*****************************************************************************
 �ṹ��   : DRV_AGENT_SET_WAKEUPCFG_CNF_STRU
 �ṹ˵�� : DRV��AT AGENT wakeupcfg
*****************************************************************************/
typedef struct
{
    AT_APPCTRL_STRU                     stAtAppCtrl;                            /*��Ϣͷ*/
    VOS_UINT32                          ulResult;                               /*���ؽ��*/
}DRV_AGENT_SET_WAKEUPCFG_CNF_STRU;
#endif/*MBB_MODULE_PM*/

#ifdef BSP_CONFIG_BOARD_SOLUTION

typedef struct
{
    VOS_UINT8   ADCEXNUM;         /* �������ý������Ĳ����ַ��� */
}AT_TAF_SET_ADCREADEX_REQ_STRU;


typedef struct
{
    AT_APPCTRL_STRU                          stAtAppCtrl;
    DRV_AGENT_VERSION_QRY_ERROR_ENUM_UINT32  enResult;
    unsigned short                           AdcReadExResult;
}DRV_AGENT_ADCREADEX_SET_CNF_STRU;
#endif

/*****************************************************************************
  10 ��������
*****************************************************************************/

extern VOS_UINT32   GAS_AtCmd(GAS_AT_CMD_STRU *pstAtCmd,GAS_AT_RSLT_STRU *pstAtCmdRslt);

extern VOS_VOID MMA_GetProductionVersion(VOS_CHAR *pcDest);
extern VOS_UINT32 MMA_VerifyOperatorLockPwd(VOS_UINT8 *pucPwd);
extern VOS_UINT32 AT_GetWcdmaBandStr(VOS_UINT8 *pucGsmBandstr, AT_UE_BAND_CAPA_ST *pstBandCapa);
extern VOS_UINT32 AT_GetGsmBandStr(VOS_UINT8 *pucGsmBandstr , AT_UE_BAND_CAPA_ST *pstBandCapa);
extern VOS_UINT32 At_SendRfCfgAntSelToHPA(
    VOS_UINT8                           ucDivOrPriOn,
    VOS_UINT8                           ucIndex
);
extern VOS_UINT32 At_DelCtlAndBlankCharWithEndPadding(VOS_UINT8 *pucData, VOS_UINT16  *pusCmdLen);
extern VOS_UINT32 Spy_SetTempPara(SPY_TEMP_THRESHOLD_PARA_STRU *stTempPara);

#if (FEATURE_ON == FEATURE_LTE)
extern VOS_VOID L_ExtSarPowerReductionPRI(VOS_UINT32 para);
#endif


#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of TafDrvAgent.h */
