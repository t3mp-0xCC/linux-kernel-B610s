#ifndef REB_OS_ADAPTER_H
#define REB_OS_ADAPTER_H
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

#include <linux/slab.h> /*kmalloc/kfree*/
#include <linux/sched.h>
/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
typedef void (*reb_poll_timer_callback)(void);
typedef int (*reb_task_fun)(void *para);
typedef void (*reb_hard_timer_callback)(unsigned int);
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/


extern  void reb_msg(int print_level, const char *fmt, ...);
extern int reb_task_create( int (*Pfunc)( void * ) );
extern void reb_task_delete( void );
extern void reb_delay_ms( unsigned int delay_time_ms );
extern void reb_timer_set( unsigned int cycle_in_ms, reb_poll_timer_callback func);
extern void reb_send_msg_to_main_task( unsigned int set_event_id );
extern void reb_task_wait_for_msg( unsigned int *get_event_id,
                            const unsigned int want_event_ids );
extern void reb_lock_sleep( unsigned int sleep_id );
extern void reb_unlock_sleep( unsigned int sleep_id );

/*----------------------------------------------*
 * ��������                                      *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#ifndef NULL
#define NULL (void *)0
#endif

typedef enum
{
    reb_err   = -1,
    reb_false = 0,
    reb_true  = 1
}reb_ret_val;

typedef enum
{
    REB_MSG_ERR = 0,
    REB_MSG_INFO = 1,
    REB_MSG_DEBUG = 2 /*use for debug*/
}reb_msg_log;

/*����ɿ���������*/
#define REB_TASK_NAME "RebTask"

/*�ɿ����¼�ID�б�*/
/*ϵͳ�����¼�*/
#define     REB_SYS_IDLE_EVENT             (1 << 0)
/*����дԽ���¼�*/
#define     REB_PRT_BYND_EVENT             (1 << 1)
/*��Σ��ɨ��ģ���ʼ������¼�*/
#define     REB_NAND_SCAN_INIT_EVENT       (1 << 2)
/*����ڼٹػ��¹���*/
#define     REB_FAST_OFF_BAT_OVERUSE       (1 << 3)


/*Ӳ����ʱ����ʱ�¼�*/
#define     REB_HARD_TIMER_OUT_EVENT       (1 << 4)
/*���������ļ�ϵͳд�¼�*/
#define     REB_FILE_WRITE_EVENT         (1 << 5)
/*Ӳ����ʱ��ֹͣ�¼�*/
#define     REB_HARD_TIMER_STOP_EVENT       (1 << 6)
/*�ɿ�����Ч�¼�*/
#define     REB_ALL_VALID_EVENT \
(\
      REB_SYS_IDLE_EVENT  \
    | REB_PRT_BYND_EVENT \
    | REB_NAND_SCAN_INIT_EVENT \
    | REB_FAST_OFF_BAT_OVERUSE \
    | REB_HARD_TIMER_OUT_EVENT \
    | REB_FILE_WRITE_EVENT \
    | REB_HARD_TIMER_STOP_EVENT )

/*���������¼���,��Ҫ�����¼����ܼ�������*/
#define     REB_ALL_ACTIVE_EVENT              ( REB_SYS_IDLE_EVENT )

/*�ڴ��ͷŵ�ħ����*/
#define MBB_FREE_MAGIC ( 0x5a5a5a5a )
/*�ڴ���Ч��ַ�ռ�*/
#define MBB_FREE_MASK ( 0xC0000000 )

#define MBB_KFREE( mem_addr )  \
    {\
        if ( MBB_FREE_MAGIC == (unsigned int) mem_addr )\
        {\
            reb_msg( REB_MSG_ERR, "REB_DFT:one memory free twice\r\n");\
        }\
        else if ( MBB_FREE_MASK != ( MBB_FREE_MASK & (unsigned int)mem_addr ) ) \
        {\
            reb_msg( REB_MSG_ERR, "REB_DFT:Invalid memory free\r\n");\
        }\
        else\
        {\
            kfree((void *)mem_addr);\
            mem_addr = (void *) MBB_FREE_MAGIC;\
        }\
    }

#ifndef TRUE
#define TRUE                  1
#endif
#ifndef FALSE
#define FALSE                 0
#endif
/*----------------------------------------------*
 * ö�ٶ���                                       *
 *----------------------------------------------*/
#endif
