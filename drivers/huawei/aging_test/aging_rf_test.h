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


#ifndef __MBB_AGING_RF_TEST_H__
#define __MBB_AGING_RF_TEST_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <linux/workqueue.h>
#include "aging_main.h"

/* 驱动内部公共数据 */
struct aging_rf_test_para {
        unsigned short mode;             /*射频模式*/ 
        unsigned short band_switch;  /*频段选择*/
        unsigned short path;        /*射频通路*/
};

/*rf测试AT命令序列*/
typedef enum
{
    AG_ATCMD_CFUN0 = 0,
    AG_ATCMD_TMOD1,
    AG_ATCMD_FCHAN,
    AG_ATCMD_FTXON0,
    AG_ATCMD_TSELRF,
    AG_ATCMD_FWAVE,
    AG_ATCMD_FTXON1,
    AG_ATCMD_MAX
}aging_rf_atcmd;

/*rf制式枚举,取值与nv一致*/
typedef enum
{
    /*GU*/
    AG_RF_CDMA = 1,  /*cdma*/
    AG_RF_GSM = 2,  /*gsm*/
    AG_RF_WCDMA = 3,  /*wcdma*/
    /*LTE*/
    AG_RF_LTE = 4,  /*lte*/

    AG_RF_MAX
}aging_rf_mode;

/*rf测试工作队列名称*/
#define AGING_WORK_NAME   ("agine_rf_test_work")

/*rf测试工作队列私有结构*/
typedef struct _aging_rf_work_data
{
    unsigned short work_index;
    unsigned int circle_flag; /*rf测试遍历周期数*/
    unsigned int lte_band_num;  /*lte band个数*/ 
}aging_rf_work_data;

int aging_rf_test_thread(void *p);
int aging_rf_test_init(void *p);
int aging_rf_test_exit(void *p);

#ifdef __cplusplus
}
#endif

#endif

