/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "reb_9x25_platform.h"
#include "../../reb_config.h"
#include "../../reb_mod/reb_func.h"
#include "../../os_adapter/reb_os_adapter.h"
#include <mach/huawei_smem.h>
#include <linux/export.h>
/*----------------------------------------------*
 * �ⲿ��������                                 *
 *----------------------------------------------*/
/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
#if (FEATURE_ON == MBB_FAST_ON_OFF)
extern volatile int  g_fast_on_off;
#endif
extern void huawei_kernel_power_off(void);
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#ifdef  CONFIG_MBB_KERNEL_NOTIFY
/*��ʱ��ͨ��Ϊ2*/
#define  REB_TIMER  2
extern int huawei_notify_kernel_A2M(unsigned int notify_id);
#endif

/*****************************************************************************
 ��������  : void reb_nv_kernel_read()
 ��������  : �������Ժ��ʼ��nv�ṹ�ĳ�ֵ
 �������  : No
 �������  : No
 �� �� ֵ  : No
 �޸���ʷ  : 
             1. 2013-12-30 :  00206465 qiaoyichuan created
*****************************************************************************/
int reb_nv_kernel_read(void)
{
/*ϵͳ�������̲����ļ�ϵͳ��������ʹ��*/
#if (YES == MBB_REB_WRITE_PROTECT)
    g_kernel_soft_reliable_info.BootNotOperaFileSysEnable = 1;
#else
    g_kernel_soft_reliable_info.BootNotOperaFileSysEnable = 0;
#endif
/*�ٹػ�ʧ�ܽ�����ػ�����ʹ��*/
#if (YES == MBB_REB_FAST_POWEROFF)
    g_kernel_soft_reliable_info.FastOffFailTrueShutdownEnable = 1;
#else
    g_kernel_soft_reliable_info.FastOffFailTrueShutdownEnable = 0;
#endif
/* Flash��Σ��ɨ�蹦��ʹ��*/
#if (YES == MBB_REB_BADZONE_SCAN)
    g_kernel_soft_reliable_info.BadZoneScanEnable = 1;
#else
    g_kernel_soft_reliable_info.BadZoneScanEnable = 0;
#endif
/* Flash��Σ���д����ʹ��*/
#if (YES == MBB_REB_BADZONE_REWRITE)
    g_kernel_soft_reliable_info.BadZoneReWriteEnable = 1;
#else
    g_kernel_soft_reliable_info.BadZoneReWriteEnable = 0;
#endif
/*����дԽ�籣��ʹ��*/
#if (YES == MBB_REB_ZONE_WRITE_PROTECT)
    g_kernel_soft_reliable_info.ZoneWriteProtectEnable = 1;
#else
    g_kernel_soft_reliable_info.ZoneWriteProtectEnable = 0;
#endif
    /*�ϵ�Σ��ʱ�����ã���λS*/
    g_kernel_soft_reliable_info.DangerTime = 5;
    return 0;
}

/*****************************************************************************
 �� �� ��  :hard_timer_creat(unsigned int time_in_ms, unsigned int input_para )
 ��������  : �����ܹ����Ѳ���ϵͳ�Ķ�ʱ
 �������  : none
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void hard_timer_creat(unsigned int time_in_ms, unsigned int input_para )
{
    huawei_smem_info *smem_data = NULL;
#ifdef  CONFIG_MBB_KERNEL_NOTIFY
    int ret = -1 ;
#endif
    smem_data = (huawei_smem_info *)SMEM_HUAWEI_ALLOC(
                            SMEM_ID_VENDOR0, sizeof(huawei_smem_info));
    /*�����ڴ�ʧ�ܲ�������ʱ��*/
    if(NULL == smem_data)
    {
        reb_msg(REB_MSG_ERR, "REB_DFT: get smem_data error\n");
        return;
    }
    /*���¹����ڴ棬���Ϊ������ʱ��*/
    smem_data->smem_reb_timer = SMEM_REB_TIMER_CREAT;
    /*�����ں˽ӿڷ���Q6��������ʱ��*/
#ifdef  CONFIG_MBB_KERNEL_NOTIFY
    ret = huawei_notify_kernel_A2M(REB_TIMER);
    if ( ret < 0 )
    {
        reb_msg(REB_MSG_ERR, "%s: REB_DFT: huawei_notify_kernel_A2M(0) fail \n",__func__);
    }
#endif
}

/*****************************************************************************
 �� �� ��  : hard_timer_set(unsigned int time_in_ms)
 ��������  :�������ö�ʱʱ��
 �������  : time_in_ms: ��λ��ms�Ķ�ʱ���
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void hard_timer_reset( unsigned int time_in_ms )
{
    huawei_smem_info *smem_data = NULL;
#ifdef  CONFIG_MBB_KERNEL_NOTIFY
    int ret = -1 ;
#endif
    /*�����ڴ�ʧ�ܲ�������ʱ��*/
    smem_data = (huawei_smem_info *)SMEM_HUAWEI_ALLOC(
                            SMEM_ID_VENDOR0, sizeof(huawei_smem_info));

    if(NULL == smem_data)
    {
        reb_msg(REB_MSG_ERR, "REB_DFT: get smem_data error\n");
        return;
    }

    if ( 0 == time_in_ms )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: time_in_ms is 0\r\n " );
        return;
    }
    /*���ݲ������¹����ڴ�*/
    if (FAST_ON_OFF_FST_CYCLE == time_in_ms)
    {
        smem_data->smem_reb_timer = SMEM_REB_TIMER_SET_FAST;
    }
    else if (FAST_ON_OFF_CYCLE == time_in_ms)
    {
        smem_data->smem_reb_timer = SMEM_REB_TIMER_SET_SLOW;
    }
    else
    {
        return;
    }
    /*�����ں˽ӿڷ���Q6��������ʱ��*/
#ifdef  CONFIG_MBB_KERNEL_NOTIFY
    ret = huawei_notify_kernel_A2M(REB_TIMER);
    if ( ret < 0 )
    {
        reb_msg(REB_MSG_ERR, "%s: REB_DFT: huawei_notify_kernel_A2M(2) fail \n", __func__);
    }
#endif

    return;
}

/*****************************************************************************
 �� �� ��  : hard_timer_delete(void)
 ��������  : ֹͣ�ܹ����Ѳ���ϵͳ�Ķ�ʱ��
 �������  : none
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void hard_timer_delete(void)
{
    huawei_smem_info *smem_data = NULL;
#ifdef  CONFIG_MBB_KERNEL_NOTIFY
    int ret = -1 ;
#endif
    /*�����ڴ�ʧ�ܲ�������ʱ��*/
    smem_data = (huawei_smem_info *)SMEM_HUAWEI_ALLOC(
                            SMEM_ID_VENDOR0, sizeof(huawei_smem_info));

    if(NULL == smem_data)
    {
        reb_msg(REB_MSG_ERR ,"REB_DFT: get smem_data error\n");
        return;
    }
    /*���¹����ڴ���Ϊֹͣ*/
    smem_data->smem_reb_timer = SMEM_REB_TIMER_DELETE;
    /*�����ں˽ӿڷ���Q6��ֹͣ��ʱ��*/
#ifdef  CONFIG_MBB_KERNEL_NOTIFY
    ret = huawei_notify_kernel_A2M(REB_TIMER);
    if ( ret < 0 )
    {
        reb_msg(REB_MSG_ERR, "%s: REB_DFT: huawei_notify_kernel_A2M(2) fail \n",__func__);
    }
#endif

}



/*****************************************************************************
 ��������  : FASTONOFF_MODE fastOnOffGetFastOnOffMode(void)
 ��������  : ��ȡ�ٹػ�״̬�ӿ�
 �������  : None
 �������  : None
 �� �� ֵ  : g_fast_on_off
 �޸���ʷ  : 
             1. 2014-1-3 :  00206465 qiaoyichuan created
*****************************************************************************/
FASTONOFF_MODE fastOnOffGetFastOnOffMode(void)
{
#if (FEATURE_ON == MBB_FAST_ON_OFF)
    return g_fast_on_off;
#endif
}
/*****************************************************************************
 ��������  : int reb_is_factory_mode(void)
 ��������  : 
 �������  : No
 �������  : No
 �� �� ֵ  : No
 �޸���ʷ  : 
             1. 2013-12-2 :  00206465 qiaoyichuan created
*****************************************************************************/
int reb_is_factory_mode(void)
{
    static int reb_ftm_mod = -1;
    if (-1 == reb_ftm_mod)
    {
#ifdef MBB_FACTORY_FEATURE
        reb_ftm_mod = 0;
#else
        reb_ftm_mod = 1;
#endif
    }
    return reb_ftm_mod;
}

/**********************************************************************
�� �� ��  :is_dload_mod
��������  :�鿴�Ƿ�������ģʽ
�������  :none
�������  :none
�� �� ֵ  :reb_true-������ģʽ reb_false-����ģʽ reb_err-ִ�д���
***********************************************************************/
reb_ret_val is_dload_mod(void)
{
#if (FEATURE_ON == MBB_DLOAD)
    huawei_smem_info *smem_data = NULL;

    smem_data = (huawei_smem_info *)SMEM_HUAWEI_ALLOC(SMEM_ID_VENDOR0, sizeof(huawei_smem_info));
    if ( NULL == smem_data )
    {
        reb_msg( REB_MSG_ERR, "DFT: Invalid Dload flag addr\r\n" );
        return reb_err;
    }

    if( SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag )
    {
        /*����ģʽ*/
        reb_msg( REB_MSG_DEBUG, "DFT: It's in Dload mode\r\n");
        return reb_true;  
    }
    else
    {
        reb_msg( REB_MSG_DEBUG, "DFT: It's NOT in Dload mode\r\n");
        return reb_false;  
    }
#else
    return reb_false;
#endif
}
EXPORT_SYMBOL(is_dload_mod);
/*****************************************************************************
 ��������  : int reb_get_current_systime(void)
 ��������  : ���ڻ�ȡ��ǰϵͳ����ʱ��
 �������  :none
 �������  :none
 �� �� ֵ  :time ��ǰϵͳ����ʱ�䣬��ȷ����
*****************************************************************************/
unsigned long  reb_get_current_systime(void)
{
    unsigned long time = 0;
    int this_cpu;
    unsigned long long t;
    unsigned long nanosec_rem;

    this_cpu = smp_processor_id();
    t = cpu_clock(this_cpu);
    nanosec_rem = do_div(t, 1000000000);/*���㴦��*/
    time = (unsigned long)t;

    return time;
}
/*****************************************************************************
 �� �� ��  : reb_for_power_off
 ��������  : sys_rebootϵͳ����ֱ�ӹػ�,������Ϊ:LINUX_REBOOT_CMD_POWER_OFF
 �������  : none
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void reb_for_power_off(void)
{
    /*����ƽ̨�ػ��ӿ�*/
#if (FEATURE_ON == MBB_FAST_ON_OFF)
#if (MBB_CHARGE == FEATURE_ON)
    huawei_kernel_power_off();
#endif
#endif    
    return;
}