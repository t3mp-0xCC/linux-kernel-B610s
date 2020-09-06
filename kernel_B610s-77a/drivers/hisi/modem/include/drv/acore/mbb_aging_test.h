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

#ifndef __MBB_AGING_TEST_H__
#define __MBB_AGING_TEST_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <product_config.h>

#if (FEATURE_ON == MBB_AGING_TEST)
/**************************************************************************
  头文件包含                            
**************************************************************************/

/**************************************************************************
  宏定义 
**************************************************************************/
#define AGING_DRV_STATUS_OK     (1)    /* 老化驱动初始化状态，1表示初始化完毕 */
#define AGING_DRV_STATUS_ERROR  (-1)   /* 老化驱动初始化状态，-1表示没有初始化完毕 */
#define AGING_ENABLE            (1)   /* 老化测试某功能使能 */
#define AGING_DISABLE           (0)   /* 老化测试某功能使能 */
/**************************************************************************
  对外提供变量、结构定义 
**************************************************************************/
/* 各老化测试模块枚举 */
typedef enum
{
    CHARGE_TEST_MODULE = 0, /*充放电*/
    LED_TEST_MODULE,        /*LED*/
    LCD_TEST_MODULE,        /*LCD*/
    WLAN_TEST_MODULE,       /*WIFI*/
    RF_TEST_MODULE,        /*RF*/
    FLASH_TEST_MODULE,      /*FLASH*/
    CPU_TEST_MODULE,        /*CPU*/

    INVALID_MODULE = 32,    /*无效值--最大支持32组*/
}AGING_TEST_MODULE_E;
/**************************************************************************
  对外提供函数接口定义 
**************************************************************************/
/********************************************************
*函数名   : aging_test_config_get
*函数功能 : 对外接口，查询老化测试各模块是否需要使能，在NV配置
*输入参数 : AGING_TEST_MODULE_E aging_test_module
*输出参数 : 无
*返回值   : 返回1表示使能，0表示不使能, 参数错误返回-1
*修改历史 :
*           2015-7-21 徐超 初版作成
********************************************************/
int aging_test_config_get(AGING_TEST_MODULE_E aging_test_module);
#endif

#ifdef __cplusplus
}
#endif

#endif

