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
#ifndef _ATDEVICECMD_H_
#define _ATDEVICECMD_H_

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "v_typdef.h"
#include "ATCmdProc.h"

#if (FEATURE_ON == MBB_COMMON)
#include "product_nv_id.h"
#include "product_nv_def.h"
#include "wlan_at_api.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/
/*����ʹ�õļ��ܽṹ*/

#if (FEATURE_ON == MBB_COMMON)
#define AT_NV_WLKEY_LEN1                (5)
#define AT_NV_WLKEY_LEN2                (13)
#define AT_NV_WLKEY_LEN3                (10)
#define AT_NV_WLKEY_LEN4                (26)

#define AT_WIFI_KEY_LEN_MIN           (8)
#define AT_WIFI_KEY_LEN_MAX_MBB           (64)

#define AT_WIFI_WEP_INDEX0           (0)
#define AT_WIFI_WEP_INDEX1           (1)
#define AT_WIFI_WEP_INDEX2           (2)
#define AT_WIFI_WEP_INDEX3           (3)

/*WEB UI ��ַ�洢��󳤶�*/
#define AT_WEBUI_SITE_NV_LEN_MAX      (36)
/*WEB UI ��ַ��д��󳤶�*/
#define AT_WEBUI_SITE_WR_LEN_MAX      (32)

/*WEB UI USER �û����洢��󳤶�*/
#define AT_WEBUI_USER_NV_LEN_MAX      (36)
/*WEB UI USER �û�����д��󳤶�*/
#define AT_WEBUI_USER_WR_LEN_MAX      (32)

/*WIFI PIN ���*/
#define AT_WIFI_PIN_NV_LEN_MAX        (36)
#define AT_WIFI_8BIT_PIN_LEN          (8)
#define AT_WIFI_4BIT_PIN_LEN          (4)

/* ^VERSION����INI���� */
#define AT_VERSION_INI_LEN               (32)
/* ^TBATDATA ��ص�ѹУ׼�������� */
#define VOLT_CALC_NUM_MAX               (2)
/* ^TBATDATA ����У׼�������� */
#define CURRENT_CALC_NUM_MAX            (0) 
/* ^TBATDATA ��ص�ѹ���������������*/
#define AT_TBATDATA_SET_PARA_NUM        (3)

/* ^TSELRF? FDD TDD Ƶ�η�Χ */
#define AT_TSELRF_FDD_BAND_MIN        (1)
#define AT_TSELRF_FDD_BAND_MAX        (32)
#define AT_TSELRF_TDD_BAND_MIN        (33)
#define AT_TSELRF_TDD_BAND_MAX        (64)

/* ^VERSION������Ϣ�ֶ�����󳤶��Լ�ֵ��󳤶ȶ��� */
#define TAF_MAX_VERSION_VALUE_LEN    (129)
#endif


/* WIFI KEY�ַ������ȣ���NVIMģ��Ipcom_parameters_handle.h�ļ��� NV_WLKEY_LEN ����һ�� */
#define AT_NV_WLKEY_LEN                 (27)

/* ��Ȩģʽ�ַ������ȣ���NVIMģ��Ipcom_parameters_handle.h�ļ��� NV_WLAUTHMODE_LEN ����һ�� */
#define AT_NV_WLAUTHMODE_LEN            (16)

/* ����ģʽ�ַ������ȣ���NVIMģ��Ipcom_parameters_handle.h�ļ��� NV_ENCRYPTIONMODES_LEN ����һ�� */
#define AT_NV_ENCRYPTIONMODES_LEN       (5)

/* WPA�������ַ������ȣ���NVIMģ��Ipcom_parameters_handle.h�ļ��� NV_WLWPAPSK_LEN ����һ�� */
#define AT_NV_WLWPAPSK_LEN              (65)

/* NV��50012�����ݽṹnv_wifisec_type WIFI KEY��������ݳ��� */
#define AT_NV_WIFISEC_OTHERDATA_LEN     (72)

/* �����֧�ֵ�WIFI KEY���� */
#define AT_WIWEP_CARD_WIFI_KEY_TOTAL    (4)

/* ��λ��֧�ֵ�WIFI KEY���� */
#define AT_WIWEP_TOOLS_WIFI_KEY_TOTAL   (16)

/* PHYNUM�������������MAC������ų��� */
#define AT_PHYNUM_MAC_LEN               (12)

/* PHYNUM�������������MAC���������^WIFIGLOBAL����ƥ����Ҫ���ӵ�ð�Ÿ��� */
#define AT_PHYNUM_MAC_COLON_NUM         (5)

#define AT_TSELRF_PATH_TOTAL            (4)

#define AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS   (2)                             /* MAC��ַ��ð��֮����ַ�����*/


#if(FEATURE_ON == FEATURE_UE_MODE_TDS)
#define AT_TDS_SCALIB_STR (VOS_UINT8 *)"(\"CACHE\",\"USE\",\"SAVE\",\"GET\",\"READ\",\"INI\",\"BEGIN\",\"END\",\"SET\",\"\"),(\"APCOFFSETFLAG\",\"APCOFFSET\",\"APC\",\"APCFREQ\",\"AGC\",\"AGCFREQ\",\"\"),(1,5,6),(@pram)"
#define AT_TDS_SCALIB_TEST_STR "(CACHE,USE,SAVE,GET,READ,INI,BEGIN,END,SET),(APCOFFSETFLAG,APCOFFSET,APC,APCFREQ,AGC,AGCFREQ),(1,5,6),(DATA)"
#endif

#define BAND_WIDTH_NUMS 6

#define UIMID_DATA_LEN                              (4)
#if (FEATURE_ON == MBB_SIMLOCK_FOUR)
#define AT_HWLOCK_PARA_LEN               (16)
#endif

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

enum AT_TBAT_OPERATION_TYPE_ENUM
{
    AT_TBAT_BATTERY_ANALOG_VOLTAGE      = 0,
    AT_TBAT_BATTERY_DIGITAL_VOLTAGE     = 1,
    AT_TBAT_BATTERY_VOLUME              = 2,
    AT_TBAT_OPERATION_TYPE_BUTT
};
typedef VOS_UINT32  AT_TBAT_OPERATION_TYPE_ENUM_UINT32;


enum AT_TBAT_OPERATION_DIRECTION_ENUM
{
    AT_TBAT_READ_FROM_UUT,
    AT_TBAT_SET_TO_UUT,
    AT_TBAT_OPERATION_DIRECTION_BUTT
};
typedef VOS_UINT32  AT_TBAT_OPERATION_DIRECTION_ENUM_UINT32;


enum AT_TSELRF_PATH_ENUM
{
    AT_TSELRF_PATH_GSM                  = 1,
    AT_TSELRF_PATH_WCDMA_PRI            = 2,
    AT_TSELRF_PATH_WCDMA_DIV            = 3,
    AT_TSELRF_PATH_TD					= 6,
    AT_TSELRF_PATH_WIFI                 = 7,
    AT_TSELRF_PATH_BUTT
};
typedef VOS_UINT32  AT_TSELRF_PATH_ENUM_UINT32;

#if (FEATURE_ON == MBB_COMMON)
enum AT_MMI_TEST{
    AT_MMI_TEST_SUPPORT_NONSUPPORT = 0, /* ��֧�� */
    AT_MMI_TEST_SUPPORT_MANUAL,         /* ֧���ֶ� */
    AT_MMI_TEST_SUPPORT_AUTO,           /* ֧���Զ� */
    AT_MMI_TEST_SUPPORT_MANUAL_AUTO,    /* ֧���ֶ����Զ� */
    AT_MMI_TEST_SUPPORT_BUTT
};

enum AT_TCHRENABEL_SWITCH_ENUM
{
    AT_TCHRENABEL_SWITCH_CHARG_CLOSE = 0,      /* �رճ�繦�� */
    AT_TCHRENABEL_SWITCH_CHARG_OPEN,           /* �򿪳�繦�� */
    AT_TCHRENABEL_SWITCH_DISCHARG_CLOSE,       /* �رճ�繦�� */
    AT_TCHRENABEL_SWITCH_DISCHARG_OPEN,        /* �򿪳�繦�� */
    AT_TCHRENABEL_SWITCH_SUPPLY,               /* �򿪲��繦�� */
    AT_TCHRENABEL_SWITCH_BUTT
};

enum AT_TCHRENABEL_CHARGE_MODE_ENUM
{
    AT_TCHRENABEL_CHARGE_MODE_DEFAULT = 0,   /* Ĭ�ϳ��ģʽ */
    AT_TCHRENABEL_CHARGE_MODE_FAST,          /* ����ģʽ */
    AT_TCHRENABEL_CHARGE_MODE_TRICKLE,       /* 丳��ģʽ */
    AT_TCHRENABEL_CHARGE_MODE_BUTT
};

enum AT_TSELRF_PATH_TYPE_ENUM
{
    AT_TSELRF_ALL     = 0,                /* ����ͨ· */
    AT_TSELRF_GSM,                        /* GSMͨ· */
    AT_TSELRF_WCDMA_MAIN,                /* WCDMA����ͨ· */
    AT_TSELRF_WCDMA_DIV,                  /* WCDMA�ּ�ͨ· */
    AT_TSELRF_CDMA_MAIN,                  /* CDMA����ͨ· */
    AT_TSELRF_CDMA_DIV,                   /* CDMA�ּ�ͨ· */
    AT_TSELRF_TDS,                         /* TDSͨ· */
    AT_TSELRF_WIFI,                        /* WIFIͨ· */
    AT_TSELRF_WIMAX,                       /* WIMAXͨ· */
    AT_TSELRF_FDD_MAIN,                    /* LTE FDD����ͨ· */
    AT_TSELRF_FDD_DIV,                     /* LTE FDD�ּ�ͨ· */
    AT_TSELRF_FDD_MIMO,                    /* LTE FDD MIMOͨ· */
    AT_TSELRF_TDD_MAIN,                    /* LTE TDD����ͨ· */
    AT_TSELRF_TDD_DIV,                     /* LTE TDD�ּ�ͨ· */
    AT_TSELRF_TDD_MIMO,                    /* LTE TDD MIMOͨ· */
    AT_TSELRF_NAVIGATION,                  /* Navigationͨ· */
    AT_TSELRF_BUTT
};
#define AT_TSELRF_PATH_TYPE_NUM           (16)
#define AT_TSELRF_EXIST                     (1)

enum AT_WLAN_MODE_ENUM
{
    AT_WLAN_MODE_CW = 0,      /* CWģʽ */
    AT_WLAN_MODE_A,           /* 802.11 Aģʽ */
    AT_WLAN_MODE_B,           /* 802.11 Bģʽ */
    AT_WLAN_MODE_G,           /* 802.11 Gģʽ */
    AT_WLAN_MODE_N,           /* 802.11 Nģʽ */
    AT_WLAN_MODE_AC,          /* 802.11 ACģʽ */
    AT_WLAN_MODE_BUTT
};

enum AT_FWAVE_ENUM
{
    AT_FWAVE_CONTINUES  = 0,      /* ���������ź� */
    AT_FWAVE_WCDMA,                /* WCDMA�����ź� */
    AT_FWAVE_GSM,                  /* GSM�����ź� */
    AT_FWAVE_EDGE,                 /* EDGE�����ź� */
    AT_FWAVE_WIFI,                 /* WIFI�����ź� */
    AT_FWAVE_LTE,                  /* LTE�����ź� */
    AT_FWAVE_CDMA,                  /* CDMA�����ź� */
    AT_FWAVE_TDS,                  /* TDS�����ź� */
    AT_FWAVE_BUTT
};

#endif
/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/

/* ���� g_stATDislogPwd �е� DIAG�ڵ�״̬Ҫ���뱸��NV�б�; �����벻�ñ���
   �ʽ� g_stATDislogPwd �е��������, ��ʹ�����е� DIAG ��״̬;
   ���¶���NV��������������  */
extern VOS_INT8                         g_acATOpwordPwd[AT_OPWORD_PWD_LEN+1];


/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/


typedef struct
{
    VOS_UINT16                          usMinAdc;
    VOS_UINT16                          usMaxAdc;
}AT_TBAT_BATTERY_ADC_INFO_STRU;

#if (FEATURE_ON == MBB_COMMON)

enum AT_TBATDATA_TYPE_ENUM
{
    AT_TBATDATA_VOLTAGE,
    AT_TBATDATA_CURRENT,
    AT_TBATDATA_TYPE_BUTT
};


enum AT_TBATDATA_INDEX_ENUM
{
    AT_TBATDATA_INDEX0,
    AT_TBATDATA_INDEX1,
    AT_TBATDATA_INDEX_BUTT
};


typedef struct
{
    VOS_UINT16                          usMinAdc;
    VOS_UINT16                          usMaxAdc;
}AT_TBATDATA_BATTERY_ADC_INFO_STRU;

#endif


typedef struct
{
    VOS_UINT8                           aucwlKeys[AT_NV_WLKEY_LEN];
}AT_WIFISEC_WIWEP_INFO_STRU;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/

VOS_UINT32 AT_SetTbatPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_QryTbatPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_SetPstandbyPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_SetWiwepPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_QryWiwepPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_TestWiwepPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetCmdlenPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_QryCmdlenPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_UpdateMacPara(
    VOS_UINT8                           aucMac[],
    VOS_UINT16                          usMacLength
);

VOS_UINT32 AT_SetTmodeAutoPowerOff(VOS_UINT8 ucIndex);

VOS_UINT32 AT_SetTseLrfPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_QryTseLrfPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_GetPhynumMac(VOS_UINT8 aucMac[]);

VOS_UINT32 AT_ExistSpecificPort(VOS_UINT8 ucPortType);

VOS_UINT32 AT_OpenDiagPort(VOS_VOID);
VOS_UINT32 AT_CloseDiagPort(VOS_VOID);
VOS_UINT32 AT_CheckSetPortRight(
    VOS_UINT8                           aucOldRewindPortStyle[],
    VOS_UINT8                           aucNewRewindPortStyle[]
);

VOS_UINT32 AT_TestHsicCmdPara(VOS_UINT8 ucIndex);

#if(FEATURE_ON == FEATURE_UE_MODE_TDS)
VOS_UINT32 At_TestTdsScalibPara(VOS_UINT8 ucIndex);
#endif

VOS_UINT32 AT_TestSimlockUnlockPara( VOS_UINT8 ucIndex );

VOS_UINT32 AT_SetNVReadPara(VOS_UINT8 ucClientId);
VOS_UINT32 AT_SetNVWritePara(VOS_UINT8 ucClientId);

#if (FEATURE_ON == MBB_COMMON)
VOS_UINT32 At_QryExtChargePara( VOS_UINT8 ucIndex );
VOS_UINT32 AT_SetWebSitePara( VOS_UINT8 ucIndex );
VOS_UINT32 AT_QryWebSitePara( VOS_UINT8 ucIndex );

VOS_UINT32 AT_SetWiFiPinPara( VOS_UINT8 ucIndex );
VOS_UINT32 AT_QryWiFiPinPara( VOS_UINT8 ucIndex );

VOS_UINT32 AT_SetWebUserPara( VOS_UINT8 ucIndex );
VOS_UINT32 AT_QryWebUserPara( VOS_UINT8 ucIndex );

VOS_UINT32 At_SetPortLockPara( VOS_UINT8 ucIndex );
VOS_UINT32 At_QryPortLockPara( VOS_UINT8 ucIndex );

VOS_UINT32 At_SetVersionPara( VOS_UINT8 ucIndex );

VOS_UINT32 AT_SetTbatDataPara( VOS_UINT8 ucIndex );
VOS_UINT32 At_QryTbatDataPara( VOS_UINT8 ucIndex );

VOS_UINT32 AT_SetAntennaPara( VOS_UINT8 ucIndex );
VOS_UINT32 AT_QryAntennaPara( VOS_UINT8 ucIndex );

VOS_UINT32 AT_QryWiFiPlatformPara( VOS_UINT8 ucIndex );

VOS_UINT32 At_SetVersionPara( VOS_UINT8 ucIndex );
VOS_UINT32 At_TestFlnaPara( VOS_UINT8 ucIndex );
VOS_UINT32 At_TestWiFiModePara( VOS_UINT8 ucIndex );
VOS_UINT32 At_TestWiFiBandPara( VOS_UINT8 ucIndex );
VOS_UINT32 At_TestTmmiPara( VOS_UINT8 ucIndex );
VOS_UINT32 At_TestSfm( VOS_UINT8 ucIndex );
VOS_UINT32 At_TestAntenna( VOS_UINT8 ucIndex );

VOS_VOID AT_GetTseLrfLoadDspInfo(
    AT_TSELRF_PATH_ENUM_UINT32          enPath,
    VOS_BOOL                           *pbLoadDsp,
    DRV_AGENT_TSELRF_SET_REQ_STRU      *pstTseLrf
);

VOS_UINT32 AT_WriteWiWep(
    VOS_UINT32                          ulIndex,
    VOS_UINT8                           aucWiWep[],
    VOS_UINT16                          usWiWepLen,
    TAF_AT_MULTI_WIFI_SEC_STRU         *pstWifiSecInfo,
    VOS_UINT8                           ucGroup
);
#endif

VOS_UINT32 AT_QryFPllStatusPara(VOS_UINT8 ucIndex);
VOS_VOID At_RfPllStatusCnfProc(PHY_AT_RF_PLL_STATUS_CNF_STRU *pstMsg);

VOS_UINT32 AT_QryFpowdetTPara(VOS_UINT8 ucIndex);
VOS_VOID At_RfFpowdetTCnfProc(PHY_AT_POWER_DET_CNF_STRU *pstMsg);

VOS_UINT32 AT_NVWRGetParaInfo( AT_PARSE_PARA_TYPE_STRU * pstPara, VOS_UINT8 * pu8Data, VOS_UINT32 * pulLen);


VOS_UINT32 AT_SetNvwrSecCtrlPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_RcvMtaNvwrSecCtrlSetCnf( VOS_VOID *pMsg );
VOS_UINT32 AT_QryNvwrSecCtrlPara(VOS_UINT8 ucIndex);
VOS_BOOL AT_IsNVWRAllowedNvId(VOS_UINT16 usNvId);

extern VOS_UINT32 AT_AsciiToHex(
    VOS_UINT8                          *pucSrc,
    VOS_UINT8                          *pucDst
);

extern VOS_UINT32 AT_AsciiToHexCode_Revers(
    VOS_UINT8                          *pucSrc,
    VOS_UINT16                          usDataLen,
    VOS_UINT8                          *pucDst
);

extern VOS_UINT32 AT_Hex2Ascii_Revers(
    VOS_UINT8                           aucHex[],
    VOS_UINT32                          ulLength,
    VOS_UINT8                           aucAscii[]
);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
extern VOS_UINT32 AT_SetMeidPara(VOS_UINT8 ucIndex);

extern VOS_UINT32 AT_QryMeidPara(VOS_UINT8 ucIndex);
#endif

extern VOS_UINT32 AT_SetMipiWrPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_SetMipiRdPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_SetSSIWrPara(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_SetSSIRdPara(VOS_UINT8 ucIndex);

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

#endif /* end of AtDeviceCmd.h */
