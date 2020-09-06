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


#ifndef __MBB_AGING_WLAN_TEST_H__
#define __MBB_AGING_WLAN_TEST_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <linux/workqueue.h>
#include "aging_main.h"

/*wlan测试AT命令序列*/
typedef enum
{
    AG_WLAN_ATCMD_TSELRF = 0,
    AG_WLAN_ATCMD_MODE,
    AG_WLAN_ATCMD_BAND,
    AG_WLAN_ATCMD_FREQ,
    AG_WLAN_ATCMD_DATARATE,
    AG_WLAN_ATCMD_POW,
    AG_WLAN_ATCMD_TX,
    AG_WLAN_ATCMD_TX_STOP,
    AG_WLAN_ATCMD_MAX
}aging_wlan_atcmd;

/*WLAN测试工作队列名称*/
#define WLAN_AGING_WORK_NAME   ("agine_wlan_test_work")

/*WLAN测试工作队列私有结构*/
typedef struct _aging_wlan_work_data
{
    unsigned short work_index;  /*当前测试wifi模式*/
    unsigned int wifi_band_num; /*wifi band个数*/ 
    unsigned int antenna;       /*当前测试天线*/
    unsigned int on_time;       /*发射时间*/
    unsigned int off_time;      /*休息时间*/
    unsigned int dualband;      /*是否2.4G和5G同时并发*/
}aging_wlan_work_data;

int aging_wlan_test_thread(void *p);
int aging_wlan_test_init(void *p);
int aging_wlan_test_exit(void *p);

#ifdef __cplusplus
}
#endif

#endif

