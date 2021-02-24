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
#ifndef __ANTEN_DETECT_H__
#define __ANTEN_DETECT_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/wait.h>

#define BUFFER_LEN       (2 )
#define TIME_DELAY       (50)    /*������ʱ   ��λms*/

#define ANTE_ERR         (-1)
#define ANTE_OK          (0 )

#define MSG_CONTAINER_LEN   (100)
typedef struct gpio_ant_detect_s
{
    const char* gpio_name;                    /* gpio name */
    u32   gpio_num;                           /* gpio ���� */
    u32   insert_value;                       /* gpio ������Ч�ĵ�ƽ*/
    u32   detect_state;                       /* ������gpio�ĵ�ƽ״̬ */
    u32   poll_flag;
    spinlock_t ops_lock;                     
    struct timer_list timer;
    wait_queue_head_t wait;
}ant_detect_info;

typedef struct gpio_ant_switch_s
{
    const char* gpio_name;
    u32 gpio_num;
    u32 inner_value;                          /*�л�Ϊ�������ߵĵ�ƽ*/
}ant_switch_info;

typedef struct gpio_ant_gpio_s
{
    const char*  proc_name;
    u32          ant_type;                    /*0--�����ߣ�1--������*/
    u32          switch_gpio_num;             /*�л�gpio�ĸ�������ʱ���������л�gpio����2��*/
    u32          switch_state;                /*�л�gpio�ĵ�ƽ״̬�����û�������*/
    ant_detect_info  ant_detect;
    ant_switch_info* ant_switch;
}ant_gpio_info;

typedef struct ant_dev_s
{
    u32 ant_num;
    ant_gpio_info* ant_info;
}ant_dev_info;


enum of_switch_current_e
{
    CURRENT_IS_INNER_ANTEN = 0x0,          /* �л�����Ŀǰ״̬�����������߻����������� */
    CURRENT_IS_OUTER_ANTEN = 0x1,      
    OF_GPIO_CURRENT_MAX    = 0xFF, 
};

enum of_switch_change_e
{
    SWITCH_TO_INNER_ANTEN  = 0x0,          /* �л����������߻����������� */
    SWITCH_TO_OUTER_ANTEN  = 0x1,
    OF_GPIO_SWITCH_MAX     = 0xff,
};

enum of_detect_gpio_e
{
    DETECT_IS_OUTSIDE = 0x0,               /* ��������δ���� */
    DETECT_IS_INSERT  = 0x1,               /* �������߲��� */
    OF_GPIO_DETECT_MAX = 0xFF,         
};



#ifdef __cplusplus
}
#endif

#endif /*__ANTEN_DETECT_H__*/
 
