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
#include "reb_config.h"
#include <linux/export.h>
#include "os_adapter/reb_os_adapter.h"
#if (YES == Reb_Platform_9x25)
#include "platform_adapter/9x25/reb_9x25_platform.h"
#endif
#if (YES == Reb_Platform_V7R2)
#include "platform_adapter/v7r2/reb_v7r2_platform.h"
#endif
#include "reb_mod/reb_func.h"
/*----------------------------------------------*
 * �ⲿ��������                                 *
 *----------------------------------------------*/
#if (FEATURE_ON == MBB_FAST_ON_OFF)
#if (MBB_CHARGE == FEATURE_ON)
extern void chg_send_stat_to_app(unsigned int chg_device_id, 
    unsigned int chg_event_id);
extern void reb_fast_off_inspect_init(void);
#endif
#endif
#if (YES == MBB_FEATURE_REB_DFT)   
extern  int  reb_test_moudle_init(void);
extern void  reb_test_moudle_exit(void);
#endif
extern int  reb_reliability_nv_cfg_kernel_init(void);
extern reb_ret_val is_dload_mod(void);
#if (YES == Reb_Platform_V7R2)
extern int reb_scan_flash_parts( void );
extern int reb_part_scan_init(void);
extern void show_cur_scan_pos(void);
#endif
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

#define FIRST_RUN_TIME (30000) /*�ɿ�������30���Ժ�ʼ����*/
#define EVERY_SCAN_ALTERNATION (3000) /*�ɿ����������3000msɨ��flashһ��*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
/*reb_task��EVERY_SCAN_ALTERNATION���ڱ����ȵ��ۼ�ʱ��(us)*/
unsigned int  lately_idle_time = 0;

/*����ﵽlately_idle_time����min_idle_duration,
����Ϊ��ǰϵͳ��æ,������ɨ��*/
unsigned int min_idle_duration = (2500000);

int reb_idle_sign_set(unsigned int idle_time_in_us)
{
    lately_idle_time += idle_time_in_us;
    return 0;
}

EXPORT_SYMBOL(reb_idle_sign_set);

void reb_idle_sign_clr(void)
{
    lately_idle_time = 0;
}

/**********************************************************************
�� �� ��: reb_main_task
��������: ����ɿ�������
�������: ��δʹ��
�������: None
�� �� ֵ: None
***********************************************************************/
int reb_main_task( void *task_para )
{
    unsigned int event_recv = 0;
    static unsigned int os_idle_cnt = 0;
    if ( NULL == task_para )
    {
        //for lint
    }

    /*Ϊ�˲��Ͽ���������һ��,�ϵ�30���Ժ�ʼ����*/
    reb_delay_ms(FIRST_RUN_TIME);

#if (YES == Reb_Platform_V7R2 )
    reb_part_scan_init(); /*��ʼ��ɨ�跶Χ*/
#endif
#if (FEATURE_ON == MBB_FAST_ON_OFF)
#if (MBB_CHARGE == FEATURE_ON)
    reb_fast_off_inspect_init();
#endif
#endif
    for ( ; ; )
    {
        reb_task_wait_for_msg( &event_recv, 0 );
        if ( event_recv & REB_FAST_OFF_BAT_OVERUSE )
        {
            event_recv &= ~REB_FAST_OFF_BAT_OVERUSE;
#if (FEATURE_ON == MBB_FAST_ON_OFF)
#if (MBB_CHARGE == FEATURE_ON)
            /*todo:�ϱ�������ػ��¼�*/
            reb_msg(REB_MSG_ERR, "REB_DFT :power off for bat overuse\r\n");
            reb_for_power_off();
#endif
#endif
        }
#if (YES == Reb_Platform_9x25)
        /*����ʱ���¼�*/
        if ( event_recv &REB_HARD_TIMER_OUT_EVENT)
        {
            event_recv &= ~REB_HARD_TIMER_OUT_EVENT;
            hard_timer_reset(FAST_ON_OFF_CYCLE);
        }

        if ( event_recv &REB_HARD_TIMER_STOP_EVENT)
        {
            event_recv &= ~REB_HARD_TIMER_STOP_EVENT;
            hard_timer_delete();
        }
#endif
        event_recv &= ~REB_SYS_IDLE_EVENT;

        if ( lately_idle_time < min_idle_duration )
        {
            os_idle_cnt = 0;
            reb_msg( REB_MSG_DEBUG, 
                "REB_DFT :Os is busy, lately_idle_time:%u,\r\n", lately_idle_time );
        }
        else
        {   
            /*��һ��ƽ������*/
            if ( ++os_idle_cnt >= 3 )
            {

#if (YES == Reb_Platform_V7R2 )
#ifdef REB_OS_DEBUG
            /*����ʹ��:��ʾ��ǰɨ�����Ϣ*/
            show_cur_scan_pos();
#endif

            reb_scan_flash_parts();
#endif
            }
        }
        reb_msg(REB_MSG_DEBUG, "REB_DFT :lately_idle_time is:%u\r\n",
            lately_idle_time );                
        reb_idle_sign_clr();

        /*�ȴ�3����ٴε���*/
        reb_delay_ms( EVERY_SCAN_ALTERNATION );
    }

    return 0;
}
/**********************************************************************
�� �� ��  :reb_init
��������  :������ʼ��
�������  :none
�������  :none
�� �� ֵ  :none
***********************************************************************/
static int reb_init(void)
{
    reb_ret_val ret_val = is_dload_mod();
    if ( ( reb_err == ret_val ) || ( reb_true == ret_val ) )
    {
        /*���������ģʽ,�ɿ���ģ�鲻��ʼ��*/
        return 0;        
    }
    if(0 == reb_is_factory_mode())
    {
        return 0;
    }
    reb_msg(REB_MSG_INFO, "REB_DFT:%s exec...\r\n", __func__);
    reb_reliability_nv_cfg_kernel_init();
    reb_task_create( (reb_task_fun)reb_main_task );
#if (YES == MBB_FEATURE_REB_DFT)   
    reb_test_moudle_init();
#endif
    return 0;
}

late_initcall(reb_init);

static void reb_exit(void)
{
#if (YES == MBB_FEATURE_REB_DFT)   
    reb_test_moudle_exit();
#endif
    return;
}
module_exit(reb_exit);

