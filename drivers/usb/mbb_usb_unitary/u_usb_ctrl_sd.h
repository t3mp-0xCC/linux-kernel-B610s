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


#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/types.h>  
#include <linux/fs.h>  
#include <linux/init.h>  
#include <linux/delay.h>  
#include <asm/uaccess.h>  
#include <asm/irq.h>  
#include <asm/io.h>  
#include <linux/miscdevice.h> 
#include <linux/ioctl.h>
#include "usb_debug.h"

#define SDDEV_MAJOR 10   /*Ԥ���sd�����豸��,д0��ʾ��̬���䣬д����ֵ���Ծ�̬����
                          *����ע�����һ�������豸�������豸�����豸�Ź̶���10
                          */
/* ����cmd���� */
#define IOCTL_DISK_BLOCK_NUM _IOR( SDDEV_MAJOR, 0, USB_INT)
#define IOCTL_MOUNT_DISK _IO( SDDEV_MAJOR, 1 )
#define IOCTL_UNMOUNT_DISK _IO( SDDEV_MAJOR, 2 )
#define IOCTL_IS_MOUNTED_DISK _IO( SDDEV_MAJOR, 3 )

#define SDDEV_IOC_MAXNR 3
/*�豸����*/
#define SDDEV_NAME "usbware_disk_0"	

#define WEB_MODE 0
#define USB_MODE 1
