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

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <drv_fastOnOff.h>
#include "fastOnOff.h"

#if (FEATURE_ON == MBB_FAST_ON_OFF)

/*�ٹػ���־��Ĭ��Ϊ����ģʽ*/
FASTONOFF_MODE gFastOnOffMode = FASTONOFF_MODE_CLR;
/* �ٹػ��ں�֪ͨ��ͷ����ʼ�� */
BLOCKING_NOTIFIER_HEAD(g_fast_on_off_notifier_list);

/********************************************************
*������   : fastOnOffSetFastOnOffMode
*�������� : ���ÿ��ٿ��ػ���־λ�������ڵ����ж��Ƿ����ٹػ�״̬
*������� : FASTONOFF_MODE fastOnOffMode
*������� : ��
*����ֵ   : ִ�гɹ�����0��ʧ�ܷ���-1
*�޸���ʷ :
*           2013-5-25 �쳬 ��������
********************************************************/
static int fastOnOffSetFastOnOffMode(FASTONOFF_MODE fastOnOffMode)
{
    int ret = 0;

    switch(fastOnOffMode)
    {
        case FASTONOFF_MODE_CLR:
			if( FASTONOFF_MODE_CLR == gFastOnOffMode )
            {
                printk(KERN_ERR "\r\n [FAST ON OFF DRV] Already in normal mode!");
                return ret;
            }
            /*�˳��ٹػ�����������״̬*/
            gFastOnOffMode = FASTONOFF_MODE_CLR;
            break;
        case FASTONOFF_MODE_SET:
			if( FASTONOFF_MODE_SET == gFastOnOffMode )
            {
                printk(KERN_ERR "\r\n [FAST ON OFF DRV] Already in fast off mode!");
                return ret;
            }
            /*����ٹػ�״̬*/
            gFastOnOffMode = FASTONOFF_MODE_SET;
            break;
        default:
            printk(KERN_ERR "\r\n [FAST ON OFF DRV] fastOnOffSetFastOnOffMode error! fastOnOffMode=%d!\r\n", fastOnOffMode);
            ret = -EINVAL;
            return ret;
    }

	/* ʹ���ں�֪ͨ��֪ͨ��������������˳��ٹػ� */
	ret = blocking_notifier_call_chain(&g_fast_on_off_notifier_list,(unsigned long)gFastOnOffMode,NULL);
	return ret;
}

/********************************************************
*������   : fastOnOffGetFastOnOffMode
*�������� : ��ȡ���ٿ��ػ���־λ�������ڵ����ж��Ƿ����ٹػ�״̬
*������� : ��
*������� : ��
*����ֵ   : ִ�гɹ����ؼٹػ���־��ʧ�ܷ���-1
*�޸���ʷ :
*           2013-5-25 �쳬 ��������
********************************************************/
FASTONOFF_MODE fastOnOffGetFastOnOffMode(void)
{
    printk(KERN_ERR "\r\n [FAST ON OFF DRV] fastOnOffGetFastOnOffMode! fastOnOffMode=%d!\r\n", gFastOnOffMode);
    return gFastOnOffMode;
}

/********************************************************
*������   : fastOnOffIoctl
*�������� : ���ٿ��ػ�����ioctl���������ڴ����û�̬�����·���ָ��
*������� : 
*������� : ��
*����ֵ   : ִ�гɹ�����0��ʧ�ܷ��ط�0ֵ
*�޸���ʷ :
*           2013-5-25 �쳬 ��������
********************************************************/
long fastOnOffIoctl(struct file *file, unsigned int cmd, unsigned long data)
{
    int ret = 0;

    printk(KERN_ERR "\r\n [FAST ON OFF DRV] fastOnOffIoctl: cmd=%u, data = %lu!\r\n",cmd,data);
    
    if(NULL == file )
    {
        printk(KERN_ERR "\r\n [FAST ON OFF DRV]fastOnOffIoctl: file is NULL!\r\n");
        return -1;
    }

    switch(cmd)
    {
        case FASTONOFF_FAST_OFF_MODE:
            /*����ٹػ�ģʽ*/
            ret = fastOnOffSetFastOnOffMode(FASTONOFF_MODE_SET);
            break;
        case FASTONOFF_FAST_ON_MODE:
            /*�˳��ٹػ�������ٿ���ģʽ*/
            ret = fastOnOffSetFastOnOffMode(FASTONOFF_MODE_CLR);
            break;
        default:
            /*������֧�ָ�����*/
            return -ENOTTY;
    }
    
    return ret;
}

static const struct file_operations fastOnOffFops = {
    .owner         = THIS_MODULE,
    .unlocked_ioctl = fastOnOffIoctl,
};

static struct miscdevice fastOnOffMiscdev = {
    .minor    = MISC_DYNAMIC_MINOR,
    .name    = "fastOnOff",
    .fops    = &fastOnOffFops
};

static int __init fastOnOffInit(void)
{
    int ret = 0;
    
    ret = misc_register(&fastOnOffMiscdev);
    if (0 > ret)
    {
        printk(KERN_ERR "\r\n [FAST ON OFF DRV] misc_register [fastOnOff module] failed.\r\n");
        return ret;
    }
    
    return ret;
}

static void __exit fastOnOffExit(void)
{
    int ret = 0;
    ret = misc_deregister(&fastOnOffMiscdev);
    if (0 > ret)
    {
        printk(KERN_ERR "\r\n [FAST ON OFF DRV] misc_deregister [fastOnOff module] failed.\r\n");
    }
}

module_init(fastOnOffInit);
module_exit(fastOnOffExit);

MODULE_AUTHOR("MBB.Huawei Device");
MODULE_DESCRIPTION("Fast On Off Driver");
MODULE_LICENSE("GPL");

#endif /* #if (FEATURE_ON == MBB_FAST_ON_OFF) */
