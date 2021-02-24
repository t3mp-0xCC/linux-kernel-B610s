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

/*宏定义 节点名称*/
#define DRV_FTYRESET_PROC_FILE "norstfact_flag"
#define DATA_LEN    (4)

#define FTYRST_OK    (0)
#define FTYRST_ERR    (-1)

/*十进制格式*/
#define ALGORISM_FORMAT    (10)
/*记录恢复出厂设置文件节点状态*/
int g_node_state = 0;

static struct proc_dir_entry *g_drv_ftyreset_proc_file = NULL;

/*************************************************************************
* 函数名         :  drv_ftyreset_proc_read
* 功能描述   :  读取当前norstfact_flag节点状态
* 输入参数   :  
* 输出参数   :  buffer : 从内核将版本号传给用户态
* 返回值        :  EFAULT--异常退出
                              其他--正常退出
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
    /*第一次读取*/
    if (0 == *offset)
    {
        len_left = strlen(data_buf);
    }
    len = (length > len_left) ? (len_left) : length;
    /*读取完毕*/
    if(len_left == 0)
    {
        return FTYRST_OK;
    }

    /*检测指针是否有效*/
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
* 函数名         :  drv_ftyreset_proc_write
* 功能描述   :  norstfact_flag节点写操作函数
* 输入参数   :  
* 输出参数   :  buffer : 从内核将版本号传给用户态
* 返回值        :  -1--异常退出
                              其他--正常退出
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
* 函数名         :  drv_get_ftyreset_node_state
* 功能描述   : 获取当前恢复出厂设置节点状态
* 输入参数   :  void
* 输出参数   :  NA
* 返回值        : 恢复出厂设置状态NODE_STATE_TYPE
**************************************************************************/
NODE_STATE_TYPE drv_get_ftyreset_node_state(void)
{
    return (NODE_STATE_TYPE)g_node_state;
}

/*节点的操作函数数据结构*/
static struct file_operations drv_ftyreset_proc_ops = {
    .read  = drv_ftyreset_proc_read,
    .write =  drv_ftyreset_proc_write,
};

/*************************************************************************
* 函数名     :  create_drv_version_proc_file
* 功能描述   :  创建norstfact_flag节点
* 输入参数   :  void
* 输出参数   :  void
* 返回值     :  void
**************************************************************************/
static void create_drv_ftyreset_proc_file(void)
{
    struct proc_dir_entry *p = NULL;
    g_drv_ftyreset_proc_file = proc_create(DRV_FTYRESET_PROC_FILE,
                                /*权限设置*/0444, p, &drv_ftyreset_proc_ops);
    if(NULL == g_drv_ftyreset_proc_file)
    {
        pr_warning("%s: create proc entry for ftyreset failed\n", DRV_FTYRESET_PROC_FILE);
    }

}

/*************************************************************************
* 函数名     :  remove_drv_version_proc_file
* 功能描述   :  删除norstfact_flag节点
* 输入参数   :  void
* 输出参数   :  void
* 返回值     :  void
**************************************************************************/
static void remove_drv_ftyreset_proc_file(void)
{
    remove_proc_entry(DRV_FTYRESET_PROC_FILE, NULL);
}

/*************************************************************************
* 函数名     :  ftyreset_drv_init
* 功能描述   : 恢复触发设置线程初始化入口函数
* 输入参数   : NA
* 输出参数   :  无
* 返回值     :  0 表示线程成功退出
                           -1  表示内核异常退出。
**************************************************************************/
static int __init ftyreset_drv_init(void)
{
    create_drv_ftyreset_proc_file();
    pr_info("ftyreset module initialized.\n");
    return FTYRST_OK;
}

/*************************************************************************
* 函数名     :  ftyreset_drv_exit
* 功能描述   : 恢复触发设置线程退出函数
* 输入参数   : NA
* 输出参数   :  无
* 返回值     :  NA
**************************************************************************/
static void __exit ftyreset_drv_exit(void)
{
    remove_drv_ftyreset_proc_file();
}

module_init(ftyreset_drv_init);
module_exit(ftyreset_drv_exit);
