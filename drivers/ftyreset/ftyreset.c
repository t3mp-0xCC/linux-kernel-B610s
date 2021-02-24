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


#include <linux/string.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include "mdrv_ftyreset.h"
#include "osl_sem.h"
#include "osl_thread.h"

/*�궨�� �ڵ�����*/
#define DRV_FTYRESET_PROC_FILE "norstfact_flag"
#define DATA_LEN    (4)

#define FTYRST_OK    (0)
#define FTYRST_ERR    (-1)

/*ʮ���Ƹ�ʽ*/
#define ALGORISM_FORMAT    (10)
/*��¼�ָ����������ļ��ڵ�״̬*/
int g_node_state = 0;

static struct proc_dir_entry *g_drv_ftyreset_proc_file = NULL;

/*************************************************************************
* ������         :  drv_ftyreset_proc_read
* ��������   :  ��ȡ��ǰnorstfact_flag�ڵ�״̬
* �������   :  
* �������   :  buffer : ���ں˽��汾�Ŵ����û�̬
* ����ֵ        :  EFAULT--�쳣�˳�
                              ����--�����˳�
**************************************************************************/
static ssize_t drv_ftyreset_proc_read(struct file *filp,
                char *buffer, size_t length, loff_t *offset)
{
    void __user *buf_usr = (void __user *)buffer;
    static size_t  len_left = 0;
    size_t  len;
    char data_buf[DATA_LEN  + 1] = {0};
    int ret = 0;
    printk(KERN_EMERG "%s:enter ................\n", __FUNCTION__);
    snprintf(data_buf, DATA_LEN,"%d", g_node_state);
    /*��һ�ζ�ȡ*/
    if (0 == *offset)
    {
        len_left = strlen(data_buf);
    }
    len = (length > len_left) ? (len_left) : length;
    /*��ȡ���*/
    if(len_left == 0)
    {
        return FTYRST_OK;
    }

    /*���ָ���Ƿ���Ч*/
    if (!access_ok(VERIFY_WRITE, buf_usr, len))
    {
        pr_err("%s: Verify user buffer write fail.\n", __FUNCTION__);
        return FTYRST_ERR;
    }

    if(copy_to_user(buf_usr, data_buf + (strlen(data_buf) - len_left), len))
    {
        pr_err("%s: copy_to_user failed, nothing copied\n", __FUNCTION__);
        return FTYRST_ERR;
    }

    *offset  += len;
    len_left -= len;

    return len;
}

/*************************************************************************
* ������         :  drv_ftyreset_proc_write
* ��������   :  norstfact_flag�ڵ�д��������
* �������   :  
* �������   :  buffer : ���ں˽��汾�Ŵ����û�̬
* ����ֵ        :  -1--�쳣�˳�
                              ����--�����˳�
**************************************************************************/
static int drv_ftyreset_proc_write(struct file *filp, const char __user *buff, size_t len, loff_t *off)
{ 
    int ret = 0;
    unsigned char dst[DATA_LEN + 1] = {0};
    unsigned int len_max = 0;

    len_max = sizeof(g_node_state);

    if ( len > len_max )
    {
        pr_emerg("input len is over %u \n", len_max);
        return FTYRST_ERR;
    }
    if (copy_from_user((void *)dst, buff, len)) 
    {
        return FTYRST_ERR;
    }
    ret = kstrtoul(dst, ALGORISM_FORMAT, &g_node_state);
    if ( ret )
    {
        return FTYRST_ERR;
    }

    return len;
}

/*************************************************************************
* ������         :  drv_get_ftyreset_node_state
* ��������   : ��ȡ��ǰ�ָ��������ýڵ�״̬
* �������   :  void
* �������   :  NA
* ����ֵ        : �ָ���������״̬NODE_STATE_TYPE
**************************************************************************/
NODE_STATE_TYPE drv_get_ftyreset_node_state(void)
{
    return (NODE_STATE_TYPE)g_node_state;
}

/*�ڵ�Ĳ����������ݽṹ*/
static struct file_operations drv_ftyreset_proc_ops = {
    .read  = drv_ftyreset_proc_read,
    .write =  drv_ftyreset_proc_write,
};

/*************************************************************************
* ������     :  create_drv_version_proc_file
* ��������   :  ����norstfact_flag�ڵ�
* �������   :  void
* �������   :  void
* ����ֵ     :  void
**************************************************************************/
static void create_drv_ftyreset_proc_file(void)
{
    struct proc_dir_entry *p = NULL;
    g_drv_ftyreset_proc_file = proc_create(DRV_FTYRESET_PROC_FILE,
                                /*Ȩ������*/0444, p, &drv_ftyreset_proc_ops);
    if(NULL == g_drv_ftyreset_proc_file)
    {
        pr_warning("%s: create proc entry for ftyreset failed\n", DRV_FTYRESET_PROC_FILE);
    }

}

/*************************************************************************
* ������     :  remove_drv_version_proc_file
* ��������   :  ɾ��norstfact_flag�ڵ�
* �������   :  void
* �������   :  void
* ����ֵ     :  void
**************************************************************************/
static void remove_drv_ftyreset_proc_file(void)
{
    remove_proc_entry(DRV_FTYRESET_PROC_FILE, NULL);
}

/*************************************************************************
* ������     :  ftyreset_drv_init
* ��������   : �ָ����������̳߳�ʼ����ں���
* �������   : NA
* �������   :  ��
* ����ֵ     :  0 ��ʾ�̳߳ɹ��˳�
                           -1  ��ʾ�ں��쳣�˳���
**************************************************************************/
static int __init ftyreset_drv_init(void)
{
    create_drv_ftyreset_proc_file();
    pr_info("ftyreset module initialized.\n");
    return FTYRST_OK;
}

/*************************************************************************
* ������     :  ftyreset_drv_exit
* ��������   : �ָ����������߳��˳�����
* �������   : NA
* �������   :  ��
* ����ֵ     :  NA
**************************************************************************/
static void __exit ftyreset_drv_exit(void)
{
    remove_drv_ftyreset_proc_file();
}

module_init(ftyreset_drv_init);
module_exit(ftyreset_drv_exit);
