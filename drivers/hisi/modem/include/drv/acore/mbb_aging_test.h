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
  ͷ�ļ�����                            
**************************************************************************/

/**************************************************************************
  �궨�� 
**************************************************************************/
#define AGING_DRV_STATUS_OK     (1)    /* �ϻ�������ʼ��״̬��1��ʾ��ʼ����� */
#define AGING_DRV_STATUS_ERROR  (-1)   /* �ϻ�������ʼ��״̬��-1��ʾû�г�ʼ����� */
#define AGING_ENABLE            (1)   /* �ϻ�����ĳ����ʹ�� */
#define AGING_DISABLE           (0)   /* �ϻ�����ĳ����ʹ�� */
/**************************************************************************
  �����ṩ�������ṹ���� 
**************************************************************************/
/* ���ϻ�����ģ��ö�� */
typedef enum
{
    CHARGE_TEST_MODULE = 0, /*��ŵ�*/
    LED_TEST_MODULE,        /*LED*/
    LCD_TEST_MODULE,        /*LCD*/
    WLAN_TEST_MODULE,       /*WIFI*/
    RF_TEST_MODULE,        /*RF*/
    FLASH_TEST_MODULE,      /*FLASH*/
    CPU_TEST_MODULE,        /*CPU*/

    INVALID_MODULE = 32,    /*��Чֵ--���֧��32��*/
}AGING_TEST_MODULE_E;
/**************************************************************************
  �����ṩ�����ӿڶ��� 
**************************************************************************/
/********************************************************
*������   : aging_test_config_get
*�������� : ����ӿڣ���ѯ�ϻ����Ը�ģ���Ƿ���Ҫʹ�ܣ���NV����
*������� : AGING_TEST_MODULE_E aging_test_module
*������� : ��
*����ֵ   : ����1��ʾʹ�ܣ�0��ʾ��ʹ��, �������󷵻�-1
*�޸���ʷ :
*           2015-7-21 �쳬 ��������
********************************************************/
int aging_test_config_get(AGING_TEST_MODULE_E aging_test_module);
#endif

#ifdef __cplusplus
}
#endif

#endif

