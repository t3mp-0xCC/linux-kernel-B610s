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


#include <mbb_leds.h>
#include "aging_led_test.h"

/* LED老化测试状态，默认为未完成测试 */
static led_end_status aging_led_test_status = AGING_LED_TESTING;
static DEFINE_MUTEX(aging_led_mutex); /* LED操作锁 */

/* 所有需要老化的灯，第一个是名字，第二个是测试结束后是否需要亮 */
struct aging_test_led aging_leds[] =
{
#ifdef BSP_CONFIG_BOARD_CPE
        {POWER_GREEN_LED,  AGING_LED_PASS},  
        {WIFI_GREEN_LED,   AGING_LED_PASS},   
        {SIM_GREEN_LED,    AGING_LED_PASS},       
        {LTE_GREEN_LED,    AGING_LED_PASS},    
        {STATUS_GREEN_LED, AGING_LED_PASS}, 
        {SIGNAL1_BLUE_LED, AGING_LED_PASS}, 
        {SIGNAL2_BLUE_LED, AGING_LED_PASS}, 
        {SIGNAL3_BLUE_LED, AGING_LED_PASS},
        {SIGNAL1_RED_LED,  AGING_LED_FAIL}, 
        {LTE_RED_LED,      AGING_LED_FAIL},  
        {SIM_RED_LED,      AGING_LED_FAIL}, 
        {POWER_RED_LED,    AGING_LED_FAIL}, 
#endif
};

/********************************************************
*函数名   : aging_led_ctrl
*函数功能 : led 老化测试显示控制函数
*输入参数 : aging_test_status test_status
*输出参数 : 无
*返回值   : 测试结束返回0，测试中返回非0值
*修改历史 :
*           2015-8-28 徐超 初版作成
********************************************************/
static void aging_led_ctrl(led_end_status test_status, int onoff)
{
    int i = 0;
    int array_size = 0;

    array_size = ARRAYSIZE(aging_leds);
    for(i = 0; i < array_size; i++)
    {
        if( (AGING_LED_TESTING == test_status) || (aging_leds[i].end_status == test_status) )
        {
            led_kernel_status_set(aging_leds[i].aging_led_name, onoff);
        }
    }
}

/********************************************************
*函数名   : aging_led_test_stop
*函数功能 : led 老化测试停止状态显示函数
*输入参数 : aging_test_status test_status
*输出参数 : 无
*返回值   : 测试结束返回0，测试中返回非0值
*修改历史 :
*           2015-8-28 徐超 初版作成
********************************************************/
void aging_led_test_stop(aging_test_status test_status)
{
    int led_init_sta = led_kernel_init_status();
    if(LED_DRV_STATUS_OK != led_init_sta)
    {
        aging_print_error("led drv has not been inited!!\n");
        return;
    }
    
    mutex_lock(&aging_led_mutex);
    switch(test_status)
    {
        case AGING_TEST_PASS:
            /* 老化测试成功结束 */
            aging_led_test_status = AGING_LED_PASS;
            aging_led_ctrl(AGING_LED_TESTING, AGING_LED_OFF);
            aging_led_ctrl(AGING_LED_PASS, AGING_LED_ON);
            break;
        default:
            aging_led_test_status = AGING_LED_FAIL;
            /* 老化测试发生异常后结束 */
            aging_led_ctrl(AGING_LED_TESTING, AGING_LED_OFF);
            aging_led_ctrl(AGING_LED_FAIL, AGING_LED_ON);
            break;
    }
    mutex_unlock(&aging_led_mutex);
}

/********************************************************
*函数名   : aging_led_test_func
*函数功能 : led 老化测试函数
*输入参数 : int time_count, int on_time, int off_time
*输出参数 : 无
*返回值   : 测试结束返回0，测试中返回非0值
*修改历史 :
*           2015-7-21 徐超 初版作成
********************************************************/
static int aging_led_test_func(int time_count, int on_time, int off_time)
{
    
    mutex_lock(&aging_led_mutex); 
    if(AGING_LED_TESTING != aging_led_test_status)
    {
        aging_print_error("aging led test over!!\n");
        goto PASS;
    }

    if ((0 == time_count % LED_STATUS_COUNT) && (0 != on_time))
    {
        aging_led_ctrl(AGING_LED_TESTING, AGING_LED_ON);
        if (0 == off_time)
        {
            aging_print_error("aging led test over!!\n");
            goto PASS;
        }
        else
        {
            mutex_unlock(&aging_led_mutex);
            msleep(on_time);
            goto TESTING;
        }
    }
    else if ((0 != time_count % LED_STATUS_COUNT) && (0 != off_time))
    {
        aging_led_ctrl(AGING_LED_TESTING, AGING_LED_OFF);
        if (0 == on_time)
        {
            aging_print_error("aging led test over!!\n");
            return AGING_LED_PASS;
        }
        else
        {
            mutex_unlock(&aging_led_mutex); 
            msleep(off_time);
            goto TESTING;
        }
    }
    else
    {
        /* 如果常亮和常灭时间都是0，那么不需要处理，直接退出 */
        aging_print_error("on time and off time is 0 together!!\n");
        goto PASS;
    }

PASS:
    mutex_unlock(&aging_led_mutex); 
    return AGING_LED_PASS;
    
TESTING:
    return AGING_LED_TESTING;
}

/********************************************************
*函数名   : aging_led_test_thread
*函数功能 : led 老化测试线程
*输入参数 : void *p，即:struct platform_device *pdev
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
*           2015-7-21 徐超 初版作成
********************************************************/
int aging_led_test_thread(void *p)
{
    int time_count = 0;
    int led_init_sta = 0;
    int ret = 0;
    
    struct aging_test_priv *priv = NULL;
    if(NULL == p)
    {
        aging_print_error("p is NULL !!\n");
        return -EINVAL;
    }
        
    priv = platform_get_drvdata(p);
    if(NULL == priv)
    {
        aging_print_error("priv is NULL !!\n");
        return -EINVAL;
    }

    if(AGING_ENABLE != priv->aging_test_nv.led_parameter.led_test_enable)
    {
        aging_print_error("led test disabled in nv!!\n");
        return 0;
    }

    led_init_sta = led_kernel_init_status();
    
    do {
        if(LED_DRV_STATUS_OK == led_init_sta)
        {
            ret = aging_led_test_func(time_count, priv->aging_test_nv.led_parameter.on_time,
                priv->aging_test_nv.led_parameter.off_time);
            if(AGING_LED_PASS == ret)
            {
                break;
            }
        }
        else
        {
            msleep(AGING_RETRY_TIME);
            led_init_sta = led_kernel_init_status();
        }
        aging_print_info("test %d times!\n", ++time_count);
    }while(!kthread_should_stop());
    return 0;
}


int aging_led_test_init(void *p)
{
    /*led测试原流程不影响,暂时打桩*/
    return 0;
}


int aging_led_test_exit(void *p)
{
    return 0;
}

