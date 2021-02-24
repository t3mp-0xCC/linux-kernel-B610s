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


#ifndef __MBB_AGING_LED_TEST_H__
#define __MBB_AGING_LED_TEST_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "aging_main.h"

#define LED_STATUS_COUNT      (2)    /* LED状态数，因为只有亮灭两种，所以是2 */
#define LED_DRV_STATUS_OK     (0)    /* LED驱动初始化状态，0表示初始化完毕 */
#define LED_DRV_STATUS_ERROR  (-1)   /* LED驱动初始化状态，-1表示没有初始化完毕 */
#define AGING_LED_ON          (1)    /* LED亮 */
#define AGING_LED_OFF         (0)    /* LED灭 */

/*LED测试状态*/
typedef enum
{
    AGING_LED_PASS    = 0,  /* 测试正常结束后需要点亮 */
    AGING_LED_FAIL    = 1,  /* 测试异常结束后需要点亮 */
    AGING_LED_TESTING = 2,  /* 正在测试中，所有灯一起操作 */
}led_end_status;

struct aging_test_led {
    char * aging_led_name;
    led_end_status end_status;
};

void aging_led_test_stop(aging_test_status test_status);
int aging_led_test_thread(void *p);
int aging_led_test_init(void *p);
int aging_led_test_exit(void *p);

#ifdef __cplusplus
}
#endif

#endif
