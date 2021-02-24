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

#ifndef CHG_OS_ADAPTER_H
#define CHG_OS_ADAPTER_H

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/


/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

extern void chg_task_create( void );
extern void chg_task_delete( void );

#if (MBB_CHG_COMPENSATE == FEATURE_ON)
extern void chg_sply_task_create( void );
#endif/*MBB_CHG_COMPENSATE*/

#define chg_timer_para unsigned long //uint32_t
typedef void (*chg_poll_timer_callback)(unsigned long);

/*�ṩ�����ѯ��ʱ��*/
extern void chg_poll_timer_set( uint32_t cycle_in_ms );
extern int32_t chg_poll_timer_get( void );

/*�ṩ��״̬����ѯģ��Ķ�ʱ��,��ʱ����ִ�лص�����*/
void chg_sta_timer_set( uint32_t cycle_in_ms, chg_poll_timer_callback func );

extern int32_t chg_main_wait_for_msg( uint32_t* get_event_id );

extern asmlinkage int chg_print_level_message(int print_level,const char *fmt,...);

extern void chg_delay_ms( uint32_t delay_time_ms );

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

void chg_sply_lock(void);

void chg_sply_unlock(void);


/*************************************************************
  ��������: chg_bat_timer_set
  ��������: �ṩ����ص�ѹ/�¶ȼ��ģ��Ķ�ʱ��,��ʱ��ʱִ�лص�����
  �������:
                        cycle_in_ms: ��ʱʱ��(ms)
                        func: �ص�����,�ص����������
                        para: func�����
  �������: None
  �� �� ֵ: None
  ע������: None
*************************************************************/
void chg_bat_timer_set( uint32_t cycle_in_ms, void (*func)(uint32_t), uint32_t para );


/**********************************************************************
�� �� ��  : chg_send_msg_to_main
��������  : ��ʱ������������������¼�
�������  : param �̶���ʽ����
�������  : �ޡ�
�� �� ֵ  : �ޡ�
ע������  : �ޡ�
***********************************************************************/
extern void chg_send_msg_to_main(uint32_t event_id);

/**********************************************************************
�� �� ��  : this_kthread_should_stop
��������  : When someone calls kthread_stop() on your kthread, it will be woken
and this will return true. You should then return, and your return value will
be passed through to kthread_stop()
�������  : ��
�������  : �ޡ�
�� �� ֵ  : TRUE:��ʾ�е���kthread_stop()��FALSE:��ʾû�У�
ע������  : �ޡ�
***********************************************************************/
boolean this_kthread_should_stop(void);
/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
/*Boolean data type.*/
#if defined(TRUE)
  #if (TRUE != 1)
    #error "TRUE" defined conflicted with native OS
  #endif
#else
  #define TRUE                   1
#endif /*defined(TRUE)*/

#if defined(FALSE)
  #if (FALSE != 0)
    #error "FALSE" defined conflicted with native OS
  #endif
#else
  #define FALSE                  0
#endif /*defined(FALSE)*/

/*���������*/
#define CHG_TASK_NAME "ChgTask"
#define CHG_SPLY_TASK_NAME "ChgSplyTask"

/*������ʱ����ʱģʽ(��Ӧms)*/
typedef enum{
   FAST_POLL_CYCLE = 2000,  /* */
   SLOW_POLL_CYCLE = 20000, /* */
   FAST_OFF_POLL_CYCLE = 300000,/* */
   SLEEP_POLL_CYCLE = 20 * 60 * 1000 /* ����֮��ʱ��ʱ�䣬20����*/
}CHG_POLL_MODE;

/* ������̵ĸ����¼�*/
#define     CHG_TIMER_EVENT                 (1 << 16)
#define     CHG_CHARGER_IN_EVENT            (1 << 17)
#define     CHG_CHARGER_OUT_EVENT           (1 << 18)
#define     CHG_POLL_TIMER_CHANGE           (1 << 19)
#define     CHG_CHGR_TYPE_CHECKING_EVENT    (1 << 20)

/*�͵�/���¹ػ��¼�*/
#define     CHG_TIMEROUT_TO_POWEROFF_EVENT  (1 << 21)

#define     CHG_ALL_EVENT   (CHG_TIMER_EVENT | CHG_CHARGER_IN_EVENT \
    | CHG_CHARGER_OUT_EVENT | CHG_POLL_TIMER_CHANGE \
    | CHG_CHGR_TYPE_CHECKING_EVENT | CHG_TIMEROUT_TO_POWEROFF_EVENT)

/*----------------------------------------------*
 * ö�ٶ���                                       *
 *----------------------------------------------*/
/*----------------------------------------------*
 * �ṹ����                                       *
 *----------------------------------------------*/
 /*ͨ�����Ͷ�Ӧͨ���Žṹ*/
#endif

