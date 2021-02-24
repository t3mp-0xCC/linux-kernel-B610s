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
#include "../reb_config.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/delay.h>
#include <linux/irq.h>

#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/wakelock.h>
#include "reb_os_adapter.h"
/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/



/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
/*���ڶ�ʱ(���ܻ���ϵͳ,�лص�)*/
static struct timer_list s_reb_sta_timer;

/*Linux�н��յ����¼���¼*/
static unsigned int s_event_recv = 0;
static reb_msg_log s_print_lvl = REB_MSG_ERR;

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*�ɿ��������ID*/
static struct task_struct *s_RebTaskID = NULL;

struct wake_lock reb_sleep_lock;
/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
 
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
/*****************************************************************************
 �� �� ��  : void reb_lock_sleep( void* sleep_id )
 ��������  : ͶƱ���������͹���
 �������  : pt_sleep_id: wakelock or just vote id
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void reb_lock_sleep( unsigned int sleep_id )
{
    if ( 0 == sleep_id )
    {
        //for lint;
    }
    wake_lock( &reb_sleep_lock );
}

/*****************************************************************************
 �� �� ��  : void reb_unlock_sleep( void* sleep_id )
 ��������  : ͶƱ�������͹���
 �������  : pt_sleep_id: wakelock or just vote id
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void reb_unlock_sleep( unsigned int sleep_id )
{
    if ( 0 == sleep_id )
    {
        //for lint;
    }

    wake_unlock( &reb_sleep_lock );
}

/*****************************************************************************
 �� �� ��  : reb_msg
 ��������  : LINUXƽ̨ͨ�õĴ�ӡ����
 �������  : int print_level
             const char *fmt...
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
asmlinkage void reb_msg(int print_level,const char *fmt,...)
{
    va_list args;
    int ret_val = 0;

    /*С�ڵ���REB_MSG_INFO�����LOG���Ա���ӡ*/
    if((unsigned int)print_level <= s_print_lvl)
    {
        va_start(args, fmt);
        ret_val = vprintk(fmt, args);
        va_end(args);
    }

    if (ret_val)
    {
        //for lint
    }
        
    return;
}
/**********************************************************************
�� �� ��: reb_task_create
��������: �ɿ���ģ�鴴������
�������: ����ִ�к���
�������: None
�� �� ֵ: -1:��������ʧ��
***********************************************************************/
int reb_task_create( reb_task_fun reb_task )
{
    if ( NULL == reb_task )
    {
        reb_msg( REB_MSG_ERR,"Reb_DFT function is NULL!\r\n");
        return -1;
    }

    if ( 0 != IS_ERR(s_RebTaskID) )
    {
        /*�ɿ��������Ѿ�����*/
        reb_msg( REB_MSG_ERR,"Reb_DFT thread EXIST!\r\n");
        return -1;
    }
    /*��ʼ��wakelock*/
    wake_lock_init( &reb_sleep_lock, WAKE_LOCK_SUSPEND, "reb_wake_lock" );
    /*�ɿ���ģ�鴴������*/
    s_RebTaskID = kthread_run( reb_task, NULL, REB_TASK_NAME);

    if ( IS_ERR(s_RebTaskID) )
    {
        reb_msg( REB_MSG_ERR,"Reb_DFT thread create error!\r\n");
    }
    else
    {
        reb_msg( REB_MSG_INFO,"Reb_DFT thread create success!\r\n");
    }
    return 0;
}

/**********************************************************************
�� �� ��: reb_task_delete
��������: ɾ������
�������: None
�������: None
�� �� ֵ: None
***********************************************************************/
void reb_task_delete( void )
{
    int ret = 0;

    if (!IS_ERR(s_RebTaskID))
    {
        ret = kthread_stop(s_RebTaskID);
        if (ret)
        {
            //for lint
        }
    }
}
/**********************************************************************
�� �� ��: reb_delay_ms
��������: �ȴ���ʱ��ʱ�䣨��λ���룩
�������: ��Ҫ��ʱ��ʱ�䣨��λ���룩
�������: None
�� �� ֵ: None
ע������: Linux����ϵͳ�¾���Ϊ1ms
***********************************************************************/
void reb_delay_ms( unsigned int delay_time_ms )
{
    msleep(delay_time_ms);
}

/*************************************************************
  ��������: reb_timer_set
  ��������: �ṩ��״̬����ѯģ��Ķ�ʱ��,��ʱ����ִ�лص�����
  �������: ��ʱʱ��(��msΪ��λ), �ص�����
  �������: None
  �� �� ֵ: None
  ע������: 1-���ܻ��Ѳ���ϵͳ
            2-����ѭ����ʱ��
*************************************************************/
void reb_timer_set( unsigned int cycle_in_ms, reb_poll_timer_callback func)
{
    if ( NULL == s_reb_sta_timer.function )
    {
        /*Ϊ�ɿ���ģ���ṩ�Ķ�ʱ��*/
        setup_timer( &s_reb_sta_timer, (void *)func, 0 );
    }
    mod_timer( &s_reb_sta_timer, jiffies + msecs_to_jiffies(cycle_in_ms) );
}

/**********************************************************************
�� �� ��: reb_send_msg_to_main_task
��������: �����¼����ɿ�������
�������: �¼�Id
�������: none
�� �� ֵ: -1-���񲻴���/�¼�ID����
ע������: none
***********************************************************************/
void reb_send_msg_to_main_task( unsigned int set_event_id )
{
    if ( NULL == s_RebTaskID )
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT: Invalid Task!!!\n ");
        return;
    }

    if (0 == (REB_ALL_VALID_EVENT & set_event_id))
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT: Invalid event id!!!\n ");
        return;
    }

    /*����*/    
    /*��¼���¼�ID*/
    s_event_recv |= set_event_id;
    /*����*/    
#if 1
    /*���ѿɿ�������*/
    wake_up_process( s_RebTaskID );
#endif
    return;
}

/**********************************************************************
�� �� ��: reb_task_wait_for_msg
��������: �ɿ��������ڹ���״̬���ȴ����¼�����
�������: event_ids-����������Ҫ��ȡ���¼���,���Ϊ0˵���κ��¼�������
            ���������
�������: get_event_id-���յ��¼�
�� �� ֵ: -1-���ݴ��� 
ע������: Linuxϵͳ�У����øú���ʱ���ж��Ƿ����¼�û�д������
***********************************************************************/
void reb_task_wait_for_msg( unsigned int *get_event_id,
                            const unsigned int want_event_ids )
{

    if ( 0 == want_event_ids )
    {
        //for lint
    }

    /*���յ��¼��Ժ�ȡ���¼���¼*/
    if ( 0 != s_event_recv )
    {
        reb_msg( REB_MSG_INFO, "Reb_DFT: receive event %d\n ", *get_event_id);
        *get_event_id = s_event_recv;
        s_event_recv = 0;
    }

    return;
}
/*****************************************************************************
 �� �� ��  : reb_set_print_lvl( reb_msg_log new_level )
 ��������  : ���ô�ӡ����
 �������  : set_print_lvl( reb_msg_log new_level )
           input_para: �������,���ڴ���Ӳ��ʱ�ص�����
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
int reb_set_print_lvl( reb_msg_log new_level )
{
    if ( ( unsigned int )new_level > REB_MSG_DEBUG )
    {
        reb_msg( REB_MSG_ERR, "\r\nReb_DFT: ERR para:%u\n ", ( unsigned int )(new_level) );
        return -1;
    }

    s_print_lvl = new_level;
    return 0;

}
