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
 


#ifndef __MDRV_ACORE_CHG_H__
#define __MDRV_ACORE_CHG_H__

#include "mdrv_public.h"
#include "product_config.h"
#ifdef __cplusplus
extern "C"
{
#endif


typedef enum CHARGING_STATE_E_tag
{
        CHARGING_INIT = -1,
        NO_CHARGING_UP = 0,  /*����δ���*/
        CHARGING_UP ,              /*���������*/
        NO_CHARGING_DOWN ,  /*�ػ�δ���*/
        CHARGING_DOWN         /*�ػ�δ���*/
}CHARGING_STATE_E;

typedef enum BATT_LEVEL_E_tag
{
        BATT_INIT = -2,
        BATT_LOW_POWER =-1,    /*��ص͵�*/
        BATT_LEVEL_0,          /*0���ص���*/
        BATT_LEVEL_1,          /*1���ص���*/
        BATT_LEVEL_2,          /*2���ص���*/
        BATT_LEVEL_3,          /*3���ص���*/
        BATT_LEVEL_4,          /*4���ص���*/
        BATT_LEVEL_MAX
}BATT_LEVEL_E;

/*��ظ����ϱ��¼�*/
typedef enum _TEMP_EVENT
{
   TEMP_BATT_LOW,    /* ��ص����¼� */
   TEMP_BATT_HIGH,   /* ��ظ����¼� */
   TEMP_BATT_NORMAL, /* ��ظ��½���¼� */
   TEMP_BATT_MAX     /* ����¼�ֵ��С�ڴ�ֵ����Ϊ�Ƿ��¼�*/
}TEMP_EVENT;

typedef struct BATT_STATE_tag
{
    CHARGING_STATE_E  charging_state;
    BATT_LEVEL_E      battery_level;
    TEMP_EVENT      batt_temp_state;
}BATT_STATE_S;

#if (MBB_COMMON == FEATURE_ON)

typedef BATT_STATE_S BATT_STATE_T;
#define CHG_OK                 0
#define CHG_ERROR              (-1)

extern BATT_LEVEL_E chg_get_batt_level(void);
unsigned char chg_is_powdown_charging (void);

#define  NV_BATT_VOLT_CALI_I           (58639)
#define  TBAT_CHECK_INVALID            (0xFFFF)

/*���У׼�������ݽṹ*/
typedef struct
{
    unsigned short min_value;
    unsigned short max_value;
}VBAT_CALIBRATION_TYPE;
/*AT�����ȡ��ص�ѹö������*/
typedef enum
{
    CHG_AT_BATTERY_LEVEL,        /* Battery Voltage after Calibration*/
    CHG_AT_BATTERY_ADC,          /* Battery Voltage before Calibration*/
    CHG_AT_BATTERY_CAPACITY,     /* Battery Capacity*/
    CHG_AT_BATTERY_CHECK,        /* Battery Calibration*/
    CHG_AT_BATTERY_INVALID
} ENUM_CHG_ATCM_TYPE;
typedef enum
{
    CHG_AT_CHARGE_DISALBE,        /* disable charge*/
    CHG_AT_CHARGE_ENABLE,         /* enable charge*/
    CHG_AT_DISCHARGE_DISALBE,     /* disable discharge*/
    CHG_AT_DISCHARGE_ENABLE,      /* enable discharge*/
    CHG_AT_START_CHARGE_SPLY,     /* start charge supply*/
    CHG_AT_CHARGE_MODE_INVALID
} ENUM_CHG_MODE_TYPE;
typedef enum 
{
    NO_BATT_MODE = 0,          /*�޵��*/
    KNOCK_DOWN_BATT_MODE = 1,  /*�ɲ�ж���*/
    FIX_BATT_MODE = 2,         /*һ�廯���*/  
    INVALID_BATT_MODE  
}BATT_FIX_MODE;
/*��ѯ���״̬��ģʽ*/
typedef struct
{
    int chg_state;
    int chg_mode;
}CHG_TCHRENABLE_TYPE;
#endif

#if (MBB_COMMON == FEATURE_ON)

int app_get_battery_state(BATT_STATE_S *battery_state);
#define mdrv_misc_get_battery_state app_get_battery_state
/**********************************************************************
�� �� ��  : chg_tbat_read
��������  : AT���ӿ�
�������  : ������ʽ:��(У׼ǰ/��)��ص������Ƕ���ѹУ׼ֵ
�������  : ��ص���/��ص�ѹУ׼ֵ
�� �� ֵ  : �Ƿ�����ɹ�
ע������  : 
***********************************************************************/
int chg_tbat_read(ENUM_CHG_ATCM_TYPE atID, void *pItem);
/**********************************************************************
�� �� ��  : chg_tbat_write
��������  : ATд�ӿ�:���õ�ص���У׼ֵ
�������  : 4.2V��3.4V��Ӧ��ADC����ֵ
�������  : ��
�� �� ֵ  : �Ƿ�����ɹ�
ע������  : ��
***********************************************************************/
int chg_tbat_write(ENUM_CHG_ATCM_TYPE atID, void *pItem);
/**********************************************************************
�� �� ��  : chg_tbat_charge_mode_set
��������  : ���õ�س����߷ŵ�
�������  : �������
�������  : none
�� �� ֵ  : CHG_OK:success,CHG_ERROR:fail
ע������  : 
***********************************************************************/
int chg_tbat_charge_mode_set(ENUM_CHG_MODE_TYPE cmd, unsigned int arg);
/**********************************************************************
�� �� ��  :  chg_tbat_status_get
��������  :  TBAT AT^TCHRENABLE?�Ƿ���Ҫ����
�������  : ��
�������  : ��
�� �� ֵ      : 1:��Ҫ����
			    0:����Ҫ����
ע������  : ��
***********************************************************************/
int chg_tbat_is_batt_status_ok(void);
/**********************************************************************
�� �� ��  : chg_tbat_get_charging_status
��������  : ��ѯ��ǰ���״̬
�������  : none
�������  : none
�� �� ֵ  : 1:charging,0:no
ע������  : 
***********************************************************************/
int chg_tbat_get_charging_status(void);
#define mdrv_misc_get_charging_status chg_tbat_get_charging_status
/**********************************************************************
�� �� ��  : chg_tbat_get_charging_status
��������  : ��ѯ��ǰ���ģʽ
�������  : none
�������  : none
�� �� ֵ  : 0:�ǳ�磻1��丳䣻2�����
ע������  : 
***********************************************************************/
int chg_tbat_get_charging_mode(void);
/**********************************************************************
�� �� ��  : chg_tbat_get_discharging_status
��������  : ��ѯ��ǰ�Ƿ���suspendģʽ
�������  : none
�������  : none
�� �� ֵ  : 1:yes,0:no
ע������  : 
***********************************************************************/
int chg_tbat_get_discharging_status(void);
/**********************************************************************
�� �� ��  : chg_tbat_sfeature_inquiry_extchg
��������  : ��ѯ�Ƿ�֧�ֶ�����
�������  : none
�������  : none
�� �� ֵ  : 1:yes,0:no
ע������  : 
***********************************************************************/
int chg_tbat_sfeature_inquiry_extchg(void);
/**********************************************************************
�� �� ��  : chg_tbat_sfeature_inquiry_wireless
��������  : ��ѯ�Ƿ�֧�����߳��
�������  : none
�������  : none
�� �� ֵ  : 1:yes,0:no
ע������  : 
***********************************************************************/
int chg_tbat_sfeature_inquiry_wireless(void);
/**********************************************************************
�� �� ��  : chg_tbat_sfeature_inquiry_battery
��������  : ��ѯ��ص�ѹ����ֵ
�������  : none
�������  : none
�� �� ֵ  : 
ע������  : 
***********************************************************************/
int chg_tbat_sfeature_inquiry_battery(void);
/**********************************************************************
�� �� ��  : chg_MNTN_get_batt_state
��������  : AT���ӿ�
�������  : ��ѯ�Ƿ��ǿɲ�ж���
�������  : none
�� �� ֵ  : 1:�ɲ�ж��2�����ɲ�ж
ע������  : 
***********************************************************************/
int chg_MNTN_get_batt_state(void);
/**********************************************************************
�� �� ��  : chg_wireless_mmi_test
��������  : AT���ӿڣ����߳���·���
�������  : none
�������  : none
�� �� ֵ  : 
ע������  : 
***********************************************************************/
int chg_wireless_mmi_test(void);
/**********************************************************************
�� �� ��  : chg_extchg_mmi_test
��������  : AT���ӿڣ��������·���
�������  : none
�������  : none
�� �� ֵ  : 
ע������  : 
***********************************************************************/
int chg_extchg_mmi_test(void);
/**********************************************************************
�� �� ��  : chg_extchg_mmi_test
��������  : AT���ӿڣ���ȡAT^TCHRENABLE���õĽ��
�������  : none
�������  : none
�� �� ֵ  : 
ע������  : 
***********************************************************************/
int chg_tbat_get_tchrenable_status(CHG_TCHRENABLE_TYPE *tchrenable_state);
/************************************************************************  
�� �� ��  : chg_getCbcState
��������  : ��������AT^CBC
         pucBcs
            0: ����ڹ���
            1: �������ӵ��δ����
            2: û����������
         pucBcl
            0:���������û����������
             100: ʣ������ٷֱ�
�������  : none
�������  : pucBcs��pucBcl
�� �� ֵ  : 
ע������  : 0 Success, -1 failed  
 ***********************************************************************/
int chg_getCbcState(unsigned char  *pucBcs ,unsigned char   *pucBcl);
#define mdrv_misc_get_cbc_state chg_getCbcState

/*****************************************************************************
 �� �� ��  : mdrv_misc_set_charge_state
 ��������  :ʹ�ܻ��߽�ֹ���
 �������  :ulState      0:��ֹ���
 						1:ʹ�ܳ��
 �������  :��
 ����ֵ��    ��
*****************************************************************************/
void mdrv_misc_set_charge_state(unsigned long ulState);

/*****************************************************************************
 �� �� ��  : mdrv_misc_get_charge_state
 ��������  :��ѯ
 �������  :
 �������  :��
 ����ֵ��    ��
*****************************************************************************/
int mdrv_misc_get_charge_state(void);

/*****************************************************************************
 �� �� ��  : mdrv_misc_sply_battery
 ��������  :��ѯ
 �������  :
 �������  :��
 ����ֵ��    ��
*****************************************************************************/
int mdrv_misc_sply_battery(void);

#endif /*MBB_COMMON*/


#ifdef __cplusplus
}
#endif
#endif

