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


#ifndef _AGING_MAIN_H_
#define _AGING_MAIN_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include <product_config.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <product_nv_def.h>
#include <product_nv_id.h>
#include <bsp_nvim.h>
#include <mbb_aging_test.h>

#define AGING_MAIN_THREAD_NAME    "aging_main_test"  /*老化测试主控制线程*/
#define AGING_THREAD_NAME_LEN_MAX     16 /* 老化测试线程名最大长度 */
#define AGING_RETRY_TIME          (1000) /* 老化测试线等待重试休息时间 */
#define AGING_ATCMD_LEN   (128)
#define AGING_ATCMD_DELAY_TIME    (2000) /*at命令发送间隔ms*/
#define AGING_MIN_TO_SEC       (60) /*时间转换单位(分-- 秒)*/
#define AGING_MIN_TO_MSEC      (60 * 1000) /*时间转换单位(分 -- 毫秒)*/
#define AGING_MAIN_WAIT_TIME  (5)  /*MAIN线程唤醒等待时间(分钟)*/

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)        (sizeof(a) / sizeof(a[0]))
#endif

#define  aging_print_error(fmt, ...)  (printk(KERN_ERR "[mbb aging test]:%s %d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define  aging_print_info(fmt, ...)   (printk(KERN_INFO "[mbb aging test]:%s %d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define  AGING_STATUS_SET_FAIL(module, value)  (value |= ((1 << module) & 0xFFFFFFFF))
#define  AGING_STATUS_SET_PASS(module, value)  (value &= (~((1 << module) & 0xFFFFFFFF)))

/* 驱动内部公共数据 */
struct aging_test_priv {
    unsigned int aging_total_status;  /*各模块测试结果汇总,每位代表一个测试项,顺序与AGING_TEST_MODULE_E一致0为PASS,1为FAILE*/
    NV_AGING_TEST_CTRL_STRU aging_test_nv;
};

struct aging_test_config {
    struct task_struct    *test_thread;
    char test_thread_name[AGING_THREAD_NAME_LEN_MAX];
    int (*thread_func)(void *p);  /*各测试线程函数*/
    int (*init_func)(void *p);  /*各测试初始化函数*/
    int (*exit_func)(void *p);  /*各测试退出函数*/
};

/*老化测试结束状态*/
typedef enum
{
    AGING_TEST_PASS    = 0,  /* 测试正常结束 */
    AGING_TEST_FAIL    = 1,  /* 测试异常结束 */
}aging_test_status;

/*老化测试工作状态*/
typedef enum
{
    AG_WORK_START = 0, /*工作开始*/
    AG_WORK_RUN,  /*工作执行*/
    AG_WORK_REST,  /*工作休息*/
    AG_WORK_END  /*工作结束*/
}aging_work_state;
/*老化测试工作队列结构*/
typedef struct _tag_aging_work_struct
{
    aging_work_state work_state;  /*工作状态描述*/
    struct aging_test_priv *priv;  /*老化测试公共结构*/
    struct delayed_work d_work;  /*delay work结构*/
    void *work_data;  /*各工作模块私有数据--各模块根据需要定义*/
}aging_work_struct;


/*at命令测试结构体*/
typedef struct _tag_aging_atcmd_info
{
    unsigned int atcmd;
    unsigned char *atname;
}aging_atcmd_info;


/*内部函数声明*/
aging_work_struct *to_aging_work(struct work_struct *work);

/*引用外部模块函数声明*/
extern unsigned short At_CmdStreamPreProc(unsigned char ucIndex, unsigned char* pData, unsigned char usLen); 

#ifdef __cplusplus
}
#endif

#endif

