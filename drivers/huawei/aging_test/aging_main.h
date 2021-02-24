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

#define AGING_MAIN_THREAD_NAME    "aging_main_test"  /*�ϻ������������߳�*/
#define AGING_THREAD_NAME_LEN_MAX     16 /* �ϻ������߳�����󳤶� */
#define AGING_RETRY_TIME          (1000) /* �ϻ������ߵȴ�������Ϣʱ�� */
#define AGING_ATCMD_LEN   (128)
#define AGING_ATCMD_DELAY_TIME    (2000) /*at����ͼ��ms*/
#define AGING_MIN_TO_SEC       (60) /*ʱ��ת����λ(��-- ��)*/
#define AGING_MIN_TO_MSEC      (60 * 1000) /*ʱ��ת����λ(�� -- ����)*/
#define AGING_MAIN_WAIT_TIME  (5)  /*MAIN�̻߳��ѵȴ�ʱ��(����)*/

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)        (sizeof(a) / sizeof(a[0]))
#endif

#define  aging_print_error(fmt, ...)  (printk(KERN_ERR "[mbb aging test]:%s %d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define  aging_print_info(fmt, ...)   (printk(KERN_INFO "[mbb aging test]:%s %d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define  AGING_STATUS_SET_FAIL(module, value)  (value |= ((1 << module) & 0xFFFFFFFF))
#define  AGING_STATUS_SET_PASS(module, value)  (value &= (~((1 << module) & 0xFFFFFFFF)))

/* �����ڲ��������� */
struct aging_test_priv {
    unsigned int aging_total_status;  /*��ģ����Խ������,ÿλ����һ��������,˳����AGING_TEST_MODULE_Eһ��0ΪPASS,1ΪFAILE*/
    NV_AGING_TEST_CTRL_STRU aging_test_nv;
};

struct aging_test_config {
    struct task_struct    *test_thread;
    char test_thread_name[AGING_THREAD_NAME_LEN_MAX];
    int (*thread_func)(void *p);  /*�������̺߳���*/
    int (*init_func)(void *p);  /*�����Գ�ʼ������*/
    int (*exit_func)(void *p);  /*�������˳�����*/
};

/*�ϻ����Խ���״̬*/
typedef enum
{
    AGING_TEST_PASS    = 0,  /* ������������ */
    AGING_TEST_FAIL    = 1,  /* �����쳣���� */
}aging_test_status;

/*�ϻ����Թ���״̬*/
typedef enum
{
    AG_WORK_START = 0, /*������ʼ*/
    AG_WORK_RUN,  /*����ִ��*/
    AG_WORK_REST,  /*������Ϣ*/
    AG_WORK_END  /*��������*/
}aging_work_state;
/*�ϻ����Թ������нṹ*/
typedef struct _tag_aging_work_struct
{
    aging_work_state work_state;  /*����״̬����*/
    struct aging_test_priv *priv;  /*�ϻ����Թ����ṹ*/
    struct delayed_work d_work;  /*delay work�ṹ*/
    void *work_data;  /*������ģ��˽������--��ģ�������Ҫ����*/
}aging_work_struct;


/*at������Խṹ��*/
typedef struct _tag_aging_atcmd_info
{
    unsigned int atcmd;
    unsigned char *atname;
}aging_atcmd_info;


/*�ڲ���������*/
aging_work_struct *to_aging_work(struct work_struct *work);

/*�����ⲿģ�麯������*/
extern unsigned short At_CmdStreamPreProc(unsigned char ucIndex, unsigned char* pData, unsigned char usLen); 

#ifdef __cplusplus
}
#endif

#endif

