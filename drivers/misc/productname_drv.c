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
 
 
 
/*lint -save -e7 -e10 -e63 -e64  -e115 -e132 -e533 -e539*/
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <product_config.h>
#include <bsp_version.h>
#include <asm/uaccess.h>

/*�궨�� �ڵ�����*/
#define DRV_PRODUCTNAME_PROC_FILE "productname"


/*ȫ�ֱ��� �ڵ�ɲ��������ݽṹ*/
static struct proc_dir_entry *g_product_name_proc_file = NULL;

/*************************************************************************
* ������     :  drv_product_name_proc_read
* ��������   :  ��ѯ��Ʒ����
* �������   :  void
* �������   :  buffer : ���ں˽��汾�Ŵ����û�̬
* ����ֵ     :  VER_RET_OK : �����ɹ�
*               VER_RET_ERROR : ����ʧ��
**************************************************************************/
static ssize_t drv_product_name_proc_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    void __user *buf_usr = (void __user *)buffer;
    static size_t  len_left = 0;
    size_t  len;

    int ret = 0;
    /*��Ʒ���Ƴ���*/
    unsigned int name_length = 31;
    char * pName = NULL;
    /*��Ų�Ʒ�����ַ���*/
    char product_name[40] = {0};
    pName = bsp_version_get_product_inner_name();
    if( NULL == pName )
	{
	    pr_err("drv_product_name_proc_read : BSP_GetProductInnerName is failed.\n");
	}
    /*��ȡ��Ʒ������Ϣ*/
	strncpy(product_name, pName, name_length);


    /*��һ�ζ�ȡ*/
    if (0 == *offset)
    {
        len_left = strlen(product_name);
    }

    len      = (length > len_left) ? (len_left) : length;
    /*��ȡ���*/
    if(len_left == 0)
    {
        return 0;
    }

    if (!access_ok(VERIFY_WRITE, buf_usr, len))
    {
        pr_err("%s: Verify user buffer write fail.\n", __FUNCTION__);
        return -1;
    }

    if(copy_to_user(buf_usr, product_name + (strlen(product_name) - len_left), len))
    {
        pr_err("%s: copy_to_user failed, nothing copied\n", __FUNCTION__);
        return -1;
    }

    *offset  += len;
    len_left -= len;

    return len;
}

/*�ڵ�Ĳ����������ݽṹ*/
static struct file_operations drv_product_name_proc_ops = {
    .read  = drv_product_name_proc_read,
};

/*************************************************************************
* ������     :  create_product_name_proc_file
* ��������   :  �����ڵ�
* �������   :  void
* �������   :  void
* ����ֵ     :  void
**************************************************************************/
static void create_product_name_proc_file(void)
{
    struct proc_dir_entry *p = NULL;
    g_product_name_proc_file = proc_create(DRV_PRODUCTNAME_PROC_FILE,
                                /*Ȩ������*/0444, p, &drv_product_name_proc_ops);
        
    if(NULL == g_product_name_proc_file)
    {
		pr_err("productname_drv.c:Error:Could not initialize /proc/%s\n",DRV_PRODUCTNAME_PROC_FILE);
    }
}

/*************************************************************************
* ������     :  remove_drv_version_proc_file
* ��������   :  ɾ���ڵ�
* �������   :  void
* �������   :  void
* ����ֵ     :  void
**************************************************************************/
static void remove_product_name_proc_file(void)
{
    remove_proc_entry(DRV_PRODUCTNAME_PROC_FILE, NULL);
}

static int __init product_name_drv_init(void)
{
    create_product_name_proc_file();
    return 0;
}

static void __exit product_name_drv_exit(void)
{
    remove_product_name_proc_file();
}

module_init(product_name_drv_init);

module_exit(product_name_drv_exit);

/*lint -restore +e7 +e10 +e63 +e64  +e115 +e132 +e533 +e539*/
