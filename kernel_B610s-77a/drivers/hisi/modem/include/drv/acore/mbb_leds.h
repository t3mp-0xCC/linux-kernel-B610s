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

#ifndef __MBB_LEDS_H__
#define __MBB_LEDS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <product_config.h>
#include <hi_gpio.h>

#if (FEATURE_ON == MBB_LED)
/**************************************************************************
  头文件包含                            
**************************************************************************/

/**************************************************************************
  宏定义 
**************************************************************************/
#define LEDS_GPIO_DEFSTATE_OFF        0
#define LEDS_GPIO_DEFSTATE_ON        1
#define LEDS_GPIO_DEFSTATE_KEEP        2
#ifndef UNUSED_PARAMETER
/* Suppress unused parameter warning */
#define UNUSED_PARAMETER(x) (void)(x)
#endif

#define LEDS_ON    (1)
#define LEDS_OFF   (0)
#define LEDS_ERROR            (-1)
#define LEDS_NOT_READY        (-2)

#define MODE_RED_LED            "mode_led:red"
#define MODE_GREEN_LED          "mode_led:green"
#define MODE_BLUE_LED           "mode_led:blue"
#define SIGNAL1_WHITE_LED       "signal1_led:white"
#define SIGNAL2_WHITE_LED       "signal2_led:white"
#define SIGNAL3_WHITE_LED       "signal3_led:white"
#define SIGNAL4_WHITE_LED       "signal4_led:white"
#define SIGNAL5_WHITE_LED       "signal5_led:white"

#define POWER_WHITE_LED         "power_led:white"
#define POWER_GREEN_LED         "power_led:green"
#define POWER_RED_LED           "power_led:red"
#define WIFI_GREEN_LED          "wifi_led:green"
#define SIM_GREEN_LED           "sim_led:green"
#define SIM_RED_LED             "sim_led:red"
#define LTE_GREEN_LED           "lte_led:green"
#define LTE_BLUE_LED            "lte_led:blue"
#define LTE_RED_LED             "lte_led:red"
#define STATUS_GREEN_LED        "status_led:green"
#define SIGNAL1_RED_LED         "signal1_led:red"
#define SIGNAL1_BLUE_LED        "signal1_led:blue"
#define SIGNAL2_BLUE_LED        "signal2_led:blue"
#define SIGNAL3_BLUE_LED        "signal3_led:blue"
#define WIFI_WHITE_LED        "wifi_led:white"
#define LAN_WHITE_LED        "lan_led:white"

#define WORK_STATUS_GREEN_LED      ("work_status_led:green")
#define WORK_STATUS_RED_LED        ("work_status_led:red")
#define WORK_STATUS_YELLOW_LED     ("work_status_led:yellow")
#define MODE_STATUS_GREEN_LED      ("mode_status_led:green")
#define MODE_STATUS_RED_LED        ("mode_status_led:red")
#define MODE_STATUS_YELLOW_LED     ("mode_status_led:yellow")
#define SIGNAL1_GREEN_LED          ("signal1_led:green")
#define SIGNAL2_GREEN_LED          ("signal2_led:green")
#define SIGNAL3_GREEN_LED          ("signal3_led:green")
#define SIGNAL4_GREEN_LED          ("signal4_led:green")

/**************************************************************************
  变量、结构定义 
**************************************************************************/
/*
 * Generic LED platform data for describing LED names and default triggers.
 */
struct led_info {
    const char    *name;
    const char    *default_trigger;
    int        flags;
};

/* For the leds-gpio driver */
struct gpio_led {
    const char *name;
    const char *default_trigger;
    unsigned     gpio;
    unsigned    active_low : 1;
    unsigned    retain_state_suspended : 1;
    unsigned    default_state : 2;
    /* default_state should be one of LEDS_GPIO_DEFSTATE_(ON|OFF|KEEP) */
};

/**************************************************************************
  函数定义 
**************************************************************************/

int led_kernel_status_set(char* led_name, int onoff);

/************************************************************************
 *函数原型 ： int led_kernel_init_status(void)
 *描述     ： 判断df与gpio控制的LED是否初始化成功
 *输入     ： NA
 *输出     ： NA
 *返回值   ： 初始化成功 0；初始化失败-1
*************************************************************************/
int led_kernel_init_status(void);
#if(FEATURE_ON == MBB_LED_DR)
int dr_led_name2dr(char *led_name);
int dr_led_init_status(void);
int dr_led_get_default_brightness(int numb);
#endif
#if(FEATURE_ON == MBB_LED_GPIO)
int gpio_led_name2gpio(char *led_name);
int gpio_led_init_status(void);
#endif
#endif

#if(FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
extern int rtk_led_ctrl(unsigned char leds, unsigned char mode);
#endif
#ifdef __cplusplus
}
#endif

#endif

