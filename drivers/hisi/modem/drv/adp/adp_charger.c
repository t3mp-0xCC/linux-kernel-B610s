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

#include <osl_types.h>
#include "mdrv_chg.h"

/*lint --e{715,818}*/

/*************************CHGģ��START********************************/

/*****************************************************************************
 �� �� ��  : chg_getCbcState
 ��������  : ���ص��״̬�͵���
 �������  :pusBcs 0:����ڹ��� 1:�������ӵ��δ���� 2:û����������
                          pucBcl  0:���������û����������
 �������  : pusBcs 0:����ڹ��� 1:�������ӵ��δ���� 2:û����������
                          pucBcl  0:���������û����������
 ����ֵ��   0 �����ɹ�
                         -1����ʧ��

*****************************************************************************/
int chg_getCbcState(unsigned char *pusBcs,unsigned char *pucBcl)
{
    return -1;
}
/*****************************************************************************
 �� �� ��  : mdrv_misc_get_charge_state
 ��������  :��ѯ
 �������  :
 �������  :��
 ����ֵ��    ��
*****************************************************************************/
int mdrv_misc_get_charge_state(void)
{
    return -1;
}

/**********************************************************************
�� �� ��  : chg_tbat_read
��������  : AT���ӿ�
�������  : ������ʽ:��(У׼ǰ/��)��ص������Ƕ���ѹУ׼ֵ
�������  : ��ص���/��ص�ѹУ׼ֵ
�� �� ֵ  : �Ƿ�����ɹ�
ע������  : 
***********************************************************************/
int chg_tbat_read(ENUM_CHG_ATCM_TYPE atID, void *pItem)
{
    return -1;
}

/**********************************************************************
�� �� ��  : chg_tbat_write
��������  : ATд�ӿ�:���õ�ص���У׼ֵ
�������  : 4.2V��3.4V��Ӧ��ADC����ֵ
�������  : ��
�� �� ֵ  : �Ƿ�����ɹ�
ע������  : ��
***********************************************************************/
int chg_tbat_write(ENUM_CHG_ATCM_TYPE atID, void *pItem)
{
    return -1;
}
/**********************************************************************
�� �� ��  : chg_tbat_charge_mode_set
��������  : ���õ�س����߷ŵ�
�������  : �������
�������  : none
�� �� ֵ  : CHG_OK:success,CHG_ERROR:fail
ע������  : 
***********************************************************************/
int chg_tbat_charge_mode_set(ENUM_CHG_MODE_TYPE cmd, unsigned int arg)
{
    return -1;
}
/**********************************************************************
�� �� ��  :  chg_tbat_status_get
��������  :  TBAT AT^TCHRENABLE?�Ƿ���Ҫ����
�������  : ��
�������  : ��
�� �� ֵ      : 1:��Ҫ����
			    0:����Ҫ����
ע������  : ��
***********************************************************************/
int chg_tbat_is_batt_status_ok(void)
{
    return -1;
}
/**********************************************************************
�� �� ��  : chg_tbat_get_charging_status
��������  : ��ѯ��ǰ���״̬
�������  : none
�������  : none
�� �� ֵ  : 1:charging,0:no
ע������  : 
***********************************************************************/
int chg_tbat_get_charging_status(void)
{
    return -1;
}
/**********************************************************************
�� �� ��  : chg_tbat_get_charging_status
��������  : ��ѯ��ǰ���ģʽ
�������  : none
�������  : none
�� �� ֵ  : 0:�ǳ�磻1��丳䣻2�����
ע������  : 
***********************************************************************/
int chg_tbat_get_charging_mode(void)
{
    return -1;
}
/**********************************************************************
�� �� ��  : chg_tbat_get_discharging_status
��������  : ��ѯ��ǰ�Ƿ���suspendģʽ
�������  : none
�������  : none
�� �� ֵ  : 1:yes,0:no
ע������  : 
***********************************************************************/
int chg_tbat_get_discharging_status(void)
{
    return -1;
}
/**********************************************************************
�� �� ��  : chg_tbat_sfeature_inquiry_extchg
��������  : ��ѯ�Ƿ�֧�ֶ�����
�������  : none
�������  : none
�� �� ֵ  : 1:yes,0:no
ע������  : 
***********************************************************************/
int chg_tbat_sfeature_inquiry_extchg(void)
{
    return -1;
}
/**********************************************************************
�� �� ��  : chg_tbat_sfeature_inquiry_wireless
��������  : ��ѯ�Ƿ�֧�����߳��
�������  : none
�������  : none
�� �� ֵ  : 1:yes,0:no
ע������  : 
***********************************************************************/
int chg_tbat_sfeature_inquiry_wireless(void)
{
    return -1;
}
/**********************************************************************
�� �� ��  : chg_tbat_sfeature_inquiry_battery
��������  : ��ѯ��ص�ѹ����ֵ
�������  : none
�������  : none
�� �� ֵ  : 
ע������  : 
***********************************************************************/
int chg_tbat_sfeature_inquiry_battery(void)
{
    return -1;
}
/**********************************************************************
�� �� ��  : chg_MNTN_get_batt_state
��������  : AT���ӿ�
�������  : ��ѯ�Ƿ��ǿɲ�ж���
�������  : none
�� �� ֵ  : 1:�ɲ�ж��2�����ɲ�ж
ע������  : 
***********************************************************************/
int chg_MNTN_get_batt_state(void)
{
    return -1;
}
/**********************************************************************
�� �� ��  : chg_wireless_mmi_test
��������  : AT���ӿڣ����߳���·���
�������  : none
�������  : none
�� �� ֵ  : 
ע������  : 
***********************************************************************/
int chg_wireless_mmi_test(void)
{
    return -1;
}
/**********************************************************************
�� �� ��  : chg_extchg_mmi_test
��������  : AT���ӿڣ��������·���
�������  : none
�������  : none
�� �� ֵ  : 
ע������  : 
***********************************************************************/
int chg_extchg_mmi_test(void)
{
    return -1;
}

/**********************************************************************
�� �� ��  : chg_extchg_mmi_test
��������  : AT���ӿڣ���ȡAT^TCHRENABLE���õĽ��
�������  : none
�������  : none
�� �� ֵ  : 
ע������  : 
***********************************************************************/
int chg_tbat_get_tchrenable_status(CHG_TCHRENABLE_TYPE *tchrenable_state)
{
    return -1;
}

/*****************************************************************************
 �� �� ��  : mdrv_misc_get_battery_state
 ��������  :��ȡ�ײ���״̬��Ϣ
 �������  :battery_state ������Ϣ
 �������  :battery_state ������Ϣ
 ����ֵ��   0 �����ɹ�
                         -1����ʧ��

*****************************************************************************/
int app_get_battery_state(BATT_STATE_S *battery_state)
{
    return -1;
}

/*****************************************************************************
 �� �� ��  : mdrv_misc_set_charge_state
 ��������  :ʹ�ܻ��߽�ֹ���
 �������  :ulState      0:��ֹ���
 						1:ʹ�ܳ��
 �������  :��
 ����ֵ��    ��
*****************************************************************************/
void mdrv_misc_set_charge_state(unsigned long ulState)
{

}

/*****************************************************************************
 �� �� ��  : BSP_CHG_Sply
 ��������  :��ѯ
 �������  :
 �������  :��
 ����ֵ��    ��
*****************************************************************************/
int mdrv_misc_sply_battery(void)
{
    return -1;
}
/*************************CHGģ�� END*********************************/


