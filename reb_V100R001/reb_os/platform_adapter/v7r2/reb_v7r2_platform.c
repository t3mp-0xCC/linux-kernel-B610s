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
#include "../../reb_config.h"
#include <linux/export.h>
#include "../../reb_mod/reb_func.h"
#include "../../os_adapter/reb_os_adapter.h"
#include "bsp_hardtimer.h"
#include "bsp_nvim.h"
#include "bsp_sram.h"
#include "bsp_softtimer.h"
#include <drv_fastOnOff.h>
#include "ptable_com.h"
/*----------------------------------------------*
 * �ⲿ��������                                 *
 *----------------------------------------------*/
/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
struct softtimer_list g_reb_poll_timer ;
reb_hard_timer_callback g_hard_timer_call_func = NULL;
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define   NV_ID_SOFT_RELIABLE_CFG  (50442)
#define   NV_ID_SOFT_FACTORY_CFG  (36)

extern void balong_power_off(void);
/*****************************************************************************
 �� �� ��  : hard_timer_creat(unsigned int time_in_ms )
 ��������  : �����ܹ����Ѳ���ϵͳ�Ķ�ʱ
 �������  : time_in_ms: ��λ��ms�Ķ�ʱ���
           input_para: �������,���ڴ���Ӳ��ʱ�ص�����
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void hard_timer_creat(unsigned int time_in_ms, unsigned int input_para )
{
    s32 ret = 0;
    if ( 0 == time_in_ms )
    {
        reb_msg( REB_MSG_ERR,"REB_DFT: time_in_ms is 0\r\n " );
        return;
    }

    if ( NULL == g_hard_timer_call_func )
    {
        reb_msg( REB_MSG_ERR,"REB_DFT: g_hard_timer_call_func is NULL\r\n " );
        return;
    }
    
    g_reb_poll_timer.func = g_hard_timer_call_func;
    g_reb_poll_timer.para = input_para;
    g_reb_poll_timer.timeout = time_in_ms;
    g_reb_poll_timer.wake_type = SOFTTIMER_WAKE;

    if ( g_reb_poll_timer.init_flags == TIMER_INIT_FLAG )
    {
        reb_msg( REB_MSG_ERR,"REB_DFT: g_reb_poll_timer was exist\r\n " );
        return;
    }

    ret = bsp_softtimer_create(&g_reb_poll_timer);
    if(ret)
    {
        reb_msg( REB_MSG_ERR,"REB_DFT: g_reb_poll_timer create error\r\n " );
        return;
    }

    bsp_softtimer_add(&g_reb_poll_timer);

}
//EXPORT_SYMBOL(hard_timer_creat);
/*****************************************************************************
 �� �� ��  : hard_timer_reset(unsigned int time_in_ms)
 ��������  : �ܹ����Ѳ���ϵͳ�Ķ�ʱ���¶�ʱ
 �������  : time_in_ms: ��λ��ms�Ķ�ʱ���
             none
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void hard_timer_reset( unsigned int time_in_ms )
{
    if ( 0 == time_in_ms )
    {
        reb_msg( REB_MSG_ERR,"REB_DFT: time_in_ms is 0\r\n " );
        return;
    }

    if ( g_reb_poll_timer.init_flags != TIMER_INIT_FLAG )
    {
        reb_msg( REB_MSG_ERR,"REB_DFT: g_reb_poll_timer NOT exist\r\n " );
        return;
    }
    
    (void)bsp_softtimer_modify( &g_reb_poll_timer, time_in_ms );
    bsp_softtimer_add(&g_reb_poll_timer);
    return;
}
/*****************************************************************************
 �� �� ��  : hard_timer_delete(void)
 ��������  : �����ܹ����Ѳ���ϵͳ�Ķ�ʱ
 �������  : time_in_ms: ��λ��ms�Ķ�ʱ���
             none
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void hard_timer_delete(void)
{
    int ret = 0;
    if ( g_reb_poll_timer.init_flags != TIMER_INIT_FLAG )
    {
        reb_msg( REB_MSG_ERR,"REB_DFT: g_reb_poll_timer NOT exist\r\n " );
        return;
    }

   ret = bsp_softtimer_delete(&g_reb_poll_timer);

    if (-1 == ret)
    {
        reb_msg(REB_MSG_ERR, "REB_DFT:hard_timer_delete fail \n");
    }

}
/*****************************************************************************
 ��������  : int reb_is_factory_mode(void)
 ��������  : 
 �������  : No
 �������  : No
 �� �� ֵ  : ��Ƭ�汾����0/�����汾����1
 �޸���ʷ  : 
             1. 2013-12-2 :  00206465 qiaoyichuan created
*****************************************************************************/
int reb_is_factory_mode(void)
{
/* �Ƿ�Ϊ��Ƭģʽͨ����Ƭ������ */
#if (FEATURE_ON == MBB_FACTORY)
    return 0;
#else
    return 1;
#endif
}

/**********************************************************************
�� �� ��  :is_dload_mod
��������  :�鿴�Ƿ�������ģʽ
�������  :none
�������  :none
�� �� ֵ  :reb_true-������ģʽ reb_false-����ģʽ reb_err-ִ�д���
***********************************************************************/
reb_ret_val  is_dload_mod(void)
{
#if (FEATURE_ON == MBB_DLOAD)
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;

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
    unsigned long long t, tmp;
    unsigned long timer_get = 0;
    timer_get = bsp_get_slice_value();
    t = (unsigned long long)(timer_get & 0xFFFFFFFF);/*���㴦��*/
    tmp = t;
    tmp = (tmp * 1000) >> 15;/*timer clock is 32768 per second*/
    time = (unsigned long)tmp / 1000;/*�������*/
    return time;
}

/*****************************************************************************
 ��������  : unsigned int  get_random_num(void)
 ��������  : ���ڻ�ȡ��ǰϵͳ����ʱ��
 �������  :none
 �������  :none
 �� �� ֵ  :time ��ǰϵͳ����ʱ�䣬��ȷ����
*****************************************************************************/
unsigned int  get_random_num(void)
{
    unsigned int num = bsp_get_slice_value();
    reb_msg( REB_MSG_INFO, "Reb_DFT get_random_num %u\r\n", num );
    return num;
}
EXPORT_SYMBOL(get_random_num);
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
    balong_power_off();
#endif
#endif
    return;
}
/*****************************************************************************
 ��������  : int reb_nv_kernel_read(void)
 ��������  : ���ݲ�ͬƽ̨��ȡnvֵ
 �������  : No
 �������  : No
 �� �� ֵ  : No
 �޸���ʷ  : 
             1. 2013-11-28 :  00206465 qiaoyichuan created
*****************************************************************************/
int reb_nv_kernel_read(void)
{
    int ret = 0;
    ret = bsp_nvm_read(NV_ID_SOFT_RELIABLE_CFG,
            (unsigned char *)(& g_kernel_soft_reliable_info), 
            sizeof(SOFT_RELIABLE_CFG_STRU));

    if (0 != ret)
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: get nv config err \r\n" );
        return -1;
    }
    return 0;
}

int reb_get_ptable_num(void)
{
    int i = 0;
    int len = 0;
    struct ST_PART_TBL * ptable = NULL;
    ptable = ptable_get_ram_data();
    if(NULL == ptable)
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: ptable_get_ram_data err \r\n" );
        return -1;
    }
    len = strlen(PTABLE_END_STR);
    while (0 != strncmp(PTABLE_END_STR, ptable->name,len))
    {
        i++;
        ptable++;
    }
	
    return i;
}
EXPORT_SYMBOL(reb_get_ptable_num);


struct ST_PART_TBL * ptable_get(void)
{
    struct ST_PART_TBL * ptable = NULL;
    ptable = ptable_get_ram_data();
    if(NULL == ptable)
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: ptable get error \r\n" );
    }
    return ptable;
}
EXPORT_SYMBOL(ptable_get);
