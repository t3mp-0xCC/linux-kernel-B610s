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
/****************************ͷ�ļ�����***************************************/
#include "../reb_config.h"
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#if (YES == Reb_Platform_V7R2 )
#include "bsp_dump.h"
#endif
#include "../os_adapter/reb_os_adapter.h"
#if (YES == MBB_FEATURE_REB_DFT )
/****************************����ԭ������************************************/

static int rebtestdev_read (struct file *file, char __user *buf,
                            size_t count, loff_t *offset);

static int rebtestdev_write (struct file *file, const char __user *buf,
                             size_t count, loff_t *offset);

static long rebtestdev_ioctl(struct file *file, unsigned int cmd,
                            unsigned long arg);

/******************************�ڲ��궨��***********************************/
/*�ڴ��ظ��ͷ�*/
#define DRIVER_REB_MEM_DOUBLE_FREE         (0x0A)
/*�ڴ��쳣�ͷ�*/
#define DRIVER_REB_MEM_ABNORMAL_FREE    (0x0B)
/*�쳣����*/
#define DRIVER_REB_ABNORMAL_RESTART     (0X0C)
/*�ڴ������ͷ�*/
#define DRIVER_REB_MEM_NORMAL_FREE    (0x0D)

#define DRIVER_REB_DATA_LEN     (100)
/*�쳣�ͷŵ�ַ*/
#define ABNORMAL_ADDRESS          (0x10000000)
/*****************************�ڲ���������**********************************/

static const struct file_operations g_rebtestdev_fops =
{
    .owner = THIS_MODULE,
    .read = rebtestdev_read,
    .write = rebtestdev_write,
    .unlocked_ioctl = rebtestdev_ioctl,
};

static struct class *g_rebtest_class;
/*******************************����ʵ��**************************************/

/*****************************************************************************
 ��������  : void reb_driver_mem_double_free(void)
 ��������  : �ظ��ͷ��ڴ�
 �������  : No
 �������  : No
 �� �� ֵ  : No
 �޸���ʷ  :
             1. 2013-11-14 :  00206465 qiaoyichuan created
*****************************************************************************/
void reb_driver_mem_double_free(void)
{
    char *str = NULL;
    /*�����ڴ� */
    str = (char *)kmalloc(DRIVER_REB_DATA_LEN, GFP_KERNEL);
    if (NULL == str)
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT: kmalloc failed \n");
        return;
    }
    memset(str, 0x0, DRIVER_REB_DATA_LEN);
    strncpy(str, "Hello_Reb_Test", DRIVER_REB_DATA_LEN - 1);
    /* ��ʾ�ַ��� */
    reb_msg( REB_MSG_ERR, "Reb_DFT: String is %s\n", str);
    /* �ظ��ͷ��ڴ�*/
    MBB_KFREE(str);
    MBB_KFREE(str);
}

/*****************************************************************************
 ��������  : reb_driver_mem_abnormal_free()
 ��������  : �ͷ��쳣��ַ�ڴ�
 �������  : None
 �������  : None
 �� �� ֵ  : No
 �޸���ʷ  : 
             1. 2013-11-20 :  00206465 qiaoyichuan created
*****************************************************************************/
void reb_driver_mem_abnormal_free(void)
{
    unsigned int *abnormal_addr = (unsigned int *)ABNORMAL_ADDRESS;
    /* �ظ��쳣��ַ�ڴ�*/
    MBB_KFREE(abnormal_addr);
}
/*****************************************************************************
 ��������  : void reb_driver_mem_normal_free()
 ��������  : �����ͷ��ڴ�
 �������  : No
 �������  : No
 �� �� ֵ  : No
 �޸���ʷ  : 
             1. 2013-12-2 :  00206465 qiaoyichuan created
*****************************************************************************/
void reb_driver_mem_normal_free(void)
{
    char *str = NULL;

    str = (char *)kmalloc(DRIVER_REB_DATA_LEN, GFP_KERNEL);
    if (NULL == str)
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT: kmalloc failed \n");
        return;
    }
    memset(str, 0x0, DRIVER_REB_DATA_LEN);
    strncpy(str, "Reb_Test_mem_normal_free", DRIVER_REB_DATA_LEN - 1);
    /* ��ʾ�ַ��� */
    reb_msg( REB_MSG_ERR, "Reb_DFT: String is %s\n", str);
    /*�ͷ��ڴ�*/
    MBB_KFREE(str);
}
/*****************************************************************************
 ��������  : static int   rebtestdev_read (struct file *file, char __user *buf,
                                     size_t count,loff_t *offset)
 ��������  : �ú�����δʹ��
 �������  : None
 �������  : None
 �� �� ֵ  : No
 �޸���ʷ  :
             1. 2013-11-14 :  00206465 qiaoyichuan created
*****************************************************************************/
static int   rebtestdev_read (struct file *file, char __user *buf,
                                                     size_t count, loff_t *offset)
{
    return 0;
}


/*****************************************************************************
 ��������  : static int  rebtestdev_write (struct file *file, const char __user *buf,
                                          size_t count,loff_t *offset)
 ��������  : �ú�����δʹ��
 �������  : No
 �������  : No
 �� �� ֵ  : No
 �޸���ʷ  :
             1. 2013-11-14 :  00206465 qiaoyichuan created
*****************************************************************************/
static int  rebtestdev_write (struct file *file, const char __user *buf,  
                                                     size_t count, loff_t *offset)
{
    return 0;
}


/*****************************************************************************
 ��������  : static int rebtestdev_ioctl(struct file *file, unsigned int cmd,
                                           unsigned long arg)
 ��������  : ���ڴ����ϲ��·��Ŀɿ��Բ��Ե�����
 �������  : No
 �������  : No
 �� �� ֵ  : No
 �޸���ʷ  :
             1. 2013-11-14 :  00206465 qiaoyichuan created
*****************************************************************************/
static long rebtestdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
        case DRIVER_REB_MEM_DOUBLE_FREE:
        {
            reb_msg( REB_MSG_ERR, "Reb_DFT: double free\n");
            reb_driver_mem_double_free();
            break;
        }

        case DRIVER_REB_MEM_ABNORMAL_FREE:
        {
            reb_msg( REB_MSG_ERR, "Reb_DFT:  free abnormal\n");
            reb_driver_mem_abnormal_free();
            break;
        }

        case DRIVER_REB_MEM_NORMAL_FREE:
        {
            reb_msg( REB_MSG_ERR, "Reb_DFT:  free normal\n");
            reb_driver_mem_normal_free();
            break;
        }

        case DRIVER_REB_ABNORMAL_RESTART:
        {
            reb_msg( REB_MSG_ERR, "Reb_DFT:  abnormal  restart\n");
#if (YES == Reb_Platform_V7R2 )
            system_error(0,0,0,0,0);
#endif
#if (YES == Reb_Platform_9x25)
            panic("abnormal  restart test");
#endif
            break;
        }

        default:
            break;
    }

    return 0;
}

/*****************************************************************************
 ��������  : static int __init reb_test_moudle_init(void)
 ��������  : �����ɿ��Բ���ģ�飬ע���ַ��豸
 �������  : None
 �������  : None
 �� �� ֵ  : No
 �޸���ʷ  :
             1. 2013-11-14 :  00206465 qiaoyichuan created
*****************************************************************************/
int reb_test_moudle_init(void)
{
    int rebtest_mager = 0;

    /*ע���ַ��豸*/
    rebtest_mager = register_chrdev(0, "reb_test", &g_rebtestdev_fops);

    /* dev�´����豸�ڵ�*/
    if (rebtest_mager < 0)
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT: REGISTER  REB TEST MOUDLE FAILED!\n");
        return -1;
    }
    else
    {
        g_rebtest_class = class_create(THIS_MODULE, "reb_test");
        device_create(g_rebtest_class, NULL, MKDEV(rebtest_mager, 0), "%s",
                      "reb_test");
    }
    reb_msg( REB_MSG_ERR, " INIT REB TEST MOUDLE !\n");

    return 0;
}

EXPORT_SYMBOL(reb_test_moudle_init);
/*****************************************************************************
 ��������  : void __exit reb_test_moudle_exit(void)
 ��������  : �˳��ɿ��Բ���ģ��
 �������  : No
 �������  : No
 �� �� ֵ  : No
 �޸���ʷ  :
             1. 2013-11-14 :  00206465 qiaoyichuan created
*****************************************************************************/
void reb_test_moudle_exit(void)
{
    unregister_chrdev(0, "reb_test");
    reb_msg( REB_MSG_ERR, "Reb_DFT: EXIT REB TEST MOUDLE!\n");
}
EXPORT_SYMBOL(reb_test_moudle_exit);
#endif


