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

#include "u_usb_ctrl_sd.h"
#include "f_mass_storage_api.h"
#include "mbb_usb_adp.h"
#include "usb_config.h"

/*------------------------------------------------------------
  ����ԭ��:virtualsd_ioctl()
  ����: 
  ���: 
  ����ֵ: USB_INT
-------------------------------------------------------------*/
USB_INT32 virtualsd_ioctl(struct file *file, USB_UINT cmd, USB_UINT32 arg)
{
    USB_INT ret = 0;
    USB_INT blocknum = 0;
    USB_INT err = -1;

    if (_IOC_TYPE(cmd) != SDDEV_MAJOR) 
    {
        return -EINVAL;
    }
    if (_IOC_NR(cmd) > SDDEV_IOC_MAXNR) 
    {
        return -EINVAL;
    }
    /* �����������ͣ��������ռ��Ƿ���Է��� */
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (USB_PVOID)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (USB_PVOID)arg, _IOC_SIZE(cmd));
    if (err) 
    {
        return -EFAULT;
    }
    
    switch (cmd)
    {
        case IOCTL_DISK_BLOCK_NUM:
            /*��ȡ�豸�Žӿں���*/
            blocknum = usb_get_mmc_dev_idex();
            if ( blocknum < 0)
            {
                DBG_E(MBB_MASS, 
                    "ioctl usb_get_mmc_dev_idex error, get mmcblocknum = %d\n",blocknum);
                ret = -1;
                break;
            }
            ret = __put_user(blocknum, (USB_INT *)arg);
            break;
#ifdef USB_SD
        case IOCTL_MOUNT_DISK:
            mass_storage_set_sd_card_workmode(USB_MODE);
            ret = mass_storage_open_usb_sd();
            if ( ret < 0)
            {
                DBG_W(MBB_MASS, "can not open usb_sd \n");
            }
            break;
        case IOCTL_UNMOUNT_DISK:
            mass_storage_set_sd_card_workmode(WEB_MODE);
            break;
        case IOCTL_IS_MOUNTED_DISK:
            ret = mass_storage_usb_sd_is_open();
            if ( ret < 0)
            {
                DBG_W(MBB_MASS, "usb_sd is not open\n");
            }
            break;
#endif
        default:
            DBG_E(MBB_MASS, "virtualsd_ioctl cmd error\r\n");
            return -ENOTTY;
    }
    return ret;
}



static const struct file_operations virtualsd_ops =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = virtualsd_ioctl,
};


static struct miscdevice virtualsd_fd =
{
    MISC_DYNAMIC_MINOR, //��̬���豸�ţ�ϵͳѡ��
    SDDEV_NAME,
    &virtualsd_ops         //�����ļ�����
};

/*------------------------------------------------------------
  ����ԭ��:virtualsd_init()
  ����: 
  ���: 
  ����ֵ: 
-------------------------------------------------------------*/
static USB_INT  __init virtualsd_init(USB_VOID)
{
    USB_INT ret = 0;
    ret = misc_register(&virtualsd_fd);
    if (ret < 0)
    {
       DBG_E(MBB_MASS, "virtualsd_init: failed,ret = %x\r\n", ret);
    }
    
    return ret;
}
/*------------------------------------------------------------
  ����ԭ��:virtualsd_exit()
  ����: 
  ���: 
  ����ֵ: 
-------------------------------------------------------------*/
static USB_VOID __exit virtualsd_exit(USB_VOID)
{
    USB_INT ret = 0;
    ret = misc_deregister(&virtualsd_fd);
    if (ret < 0)
    {
        DBG_E(MBB_MASS, "virtualsd_exit: failed,ret = %x\n", ret);
    }
    return;
}
module_init(virtualsd_init);
module_exit(virtualsd_exit);


