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

#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include "../usb/storage/usb.h"

#define SHORT_STRLEN    32
#define MAX_STRLEN      128
#define MAX_DISKS       6
#define MAX_DEVICEID_NUM    9
#define MAX_SERIALNAME_NUM  5
#define U_DISK_BLOCK_SIZE   512

struct usb_diskinfo
{
    char diskname[SHORT_STRLEN];        /*disk name:sda,sdb,....*/
    char vendor[MAX_STRLEN];            /*vendor name of the usb device*/
    char model[MAX_STRLEN];             /*product name of the usb device*/
    char serial[MAX_STRLEN];            /*serial number  of the usb device*/
    char connectiontype[SHORT_STRLEN];  /*USB1.1,USB2.0,...*/
    int  scsihostid ;                   /*scsi host num of this device*/
    bool isvalid;                       /* if this partion info is valid*/
    char usbport[SHORT_STRLEN];
    unsigned long long	capacity;       /*U������*/
}* usbdiskinfos;

struct usb_diskinfo* diskinfos = NULL;

/*get a free member*/
/*****************************************************************
Parameters    :  void
Return        :
Description   :  ��ȡ��ǰ���е�diskinfo
*****************************************************************/
static struct usb_diskinfo* get_freediskslot(void)
{
    int i = 0;
    int j = -1;

    if ( diskinfos != NULL )
    {
        for (i = 0; i < MAX_DISKS; i++)
        {
            if (false == (((struct usb_diskinfo*)diskinfos + i)->isvalid))
            {
                j = i;
                break;
            }
        }

        if (j >= 0 && j < MAX_DISKS)
        {
            return ((struct usb_diskinfo*)diskinfos + j);
        }
    }

    return NULL;
}

/*release a used member*/
/*****************************************************************
Parameters    :  disk_name
Return        :
Description   :  ����disk name��ص�diskinfo״̬Ϊ������
*****************************************************************/
static void release_useddiskslot(char* disk_name)
{
    int i = 0;

    if ( disk_name != NULL )
    {
        for (i = 0; i < MAX_DISKS; i++)
        {
            if (!strcmp(disk_name, ((struct usb_diskinfo*)diskinfos + i)->diskname))
            {
                ((struct usb_diskinfo*)diskinfos + i)->isvalid = false;
            }
        }
    }

    return;
}

/*****************************************************************
Parameters    :  buf
Return        :
Description   :  �ַ����ڵĿո��滻���»���'_'
*****************************************************************/
static char* replacespace(char* buf)
{
    int i = 0;
    int len = 0;

    if ( buf != NULL )
    {
        len = strlen(buf);
        if (len >= MAX_STRLEN)
        {
            len = MAX_STRLEN - 1;
        }

        for (i = 0; i < len; i++)
        {
            if (buf[i] == ' ')
            {
                buf[i] = '_';
            }
        }
    }

    return buf;
}

/* Show the /proc/proc_user_dev file content. */
/*****************************************************************
Parameters    :  sfile
                 v
Return        :
Description   :  ���ϲ㷵����ز���
*****************************************************************/
static int proc_user_physicalmedium_show(struct seq_file* sfile, void* v)
{
    int i = 0;
    struct usb_diskinfo* disk = NULL;
    if (NULL == sfile)
    {
        pr_err("sfile NULL \n");
        return 0;
    }
    seq_printf(sfile, "%s", "Diskname Vendor       ID       Type ConnectionType Capacity Port\n");
    for ( i = 0; i < MAX_DISKS; i++ )
    {
        if ( true == (((struct usb_diskinfo*)diskinfos + i)->isvalid) )
        {
            disk = ((struct usb_diskinfo*)diskinfos + i);

            /*phy �Ͽ���������hub��Ӳ����hub2ʱ��װ�����Ի������ܴ����⣬д��1-1*/
            snprintf(disk->usbport, sizeof(disk->usbport), "%s", "1-1");

            seq_printf(sfile, "%-8s %-12s %-8s %-6s %-14s %llu %-6s \n",
                       replacespace(disk->diskname),
                       replacespace(disk->vendor),
                       replacespace(disk->model),
                       replacespace(disk->serial),
                       replacespace(disk->connectiontype),
                       disk->capacity,
                       replacespace(disk->usbport) );
        }
    }

    return 0;
}

static int proc_user_physicalmedium_single_open(struct inode* inode, struct file* file)
{
    return (single_open(file, proc_user_physicalmedium_show, NULL));
}

static const struct file_operations proc_user_physicalmedium_fops =
{
    .open		= proc_user_physicalmedium_single_open,
    .read		= seq_read,
    .llseek		= seq_lseek,
    .release	= single_release,
};

/*****************************************************************
Parameters    :  void
Return        :
Description   :  ����proc�ڵ㣬����ά����diskinfo
*****************************************************************/
static void proc_user_physicalmedium_create(void)
{
    struct proc_dir_entry* proc_physicalmedium = NULL;
    int i = 0;
    
    proc_physicalmedium = proc_create("proc_user_usbdevs", S_IFREG | S_IRUSR ,
                                      NULL, &proc_user_physicalmedium_fops );
    if ( NULL == proc_physicalmedium)
    {
        pr_err("%s proc_create ERR \n",__FUNCTION__);
        return ;
    }
    /*initialize physicalmidum devs structure*/
    diskinfos = kmalloc(sizeof(struct usb_diskinfo) * MAX_DISKS, GFP_KERNEL);
    if (diskinfos)
    {
        memset(diskinfos, 0, sizeof(struct usb_diskinfo)*MAX_DISKS);
        for (i = 0; i < MAX_DISKS; i++)
        {
            /* Init the isvalid flag. */
            ((struct usb_diskinfo*)diskinfos + i)->isvalid = false;
        }
    }
}

/*****************************************************************
Parameters    :  void
Return        :
Description   :  ɾ��proc�ڵ㣬�ͷ�diskinfo
*****************************************************************/
static void proc_user_physicalmedium_remove(void)
{
    /* no problem if it was not registered */
    remove_proc_entry("proc_user_usbdevs", NULL);

    if (NULL != diskinfos )
    {
        kfree(diskinfos);
        diskinfos = NULL;
    }
}

/*****************************************************************
Parameters    :  disk
Return        :
Description   :  ��ʼ��diskinfo����ز���
*****************************************************************/
void usb_diskinfo_init(struct usb_diskinfo* disk)
{
    memset(disk->diskname, 0, sizeof(disk->diskname));
    memset(disk->vendor, 0, sizeof(disk->vendor));
    memset(disk->model, 0, sizeof(disk->model));
    memset(disk->serial, 0, sizeof(disk->serial));
    memset(disk->connectiontype, 0, sizeof(disk->connectiontype));
    memset(disk->usbport, 0, sizeof(disk->usbport));
    disk->scsihostid = 0;
    disk->isvalid = false;
    disk->capacity = 0;
}

/* Get the physical medium device information. */
/*****************************************************************
Parameters    :  sdp
                 sdkp
                 gd
Return        :
Description   :  ��sd�����н�ȡ�ϲ���Ҫ������ֶΣ�������ʽ�洢��diskinfo��
*****************************************************************/
static int get_physicalmedium_usbdev_info(struct scsi_device* sdp,
        struct scsi_disk* sdkp, struct gendisk* gd)
{
    int  giVendor;
    int  giProduct;
    int  giInterfaceClass;
    char DeviceId[MAX_DEVICEID_NUM] = {0};
    char serialnum[MAX_SERIALNAME_NUM] = {0};
    char* ser = NULL;
    int  length = 0;
    int  i = 0;

    if ( NULL != sdp && NULL != sdkp  && NULL != gd )
    {
        struct usb_diskinfo* disk = NULL;
        struct Scsi_Host* host = sdp->host;
        struct us_data* us = host_to_us(host);
        char* string = NULL;

        giVendor = le16_to_cpu(us->pusb_dev->descriptor.idVendor);
        giProduct = le16_to_cpu(us->pusb_dev->descriptor.iProduct);
        giInterfaceClass = us->pusb_intf->altsetting->desc.bInterfaceClass;

        disk = get_freediskslot();

        if ( NULL != disk )
        {
            usb_diskinfo_init(disk);
            disk->isvalid = true;

            //���кŴ��ڣ�ID�ĽṹΪvendor+serial����λ
            if ( NULL != us->pusb_dev->serial )
            {
                //��¼�����кŵ�ָ��
                ser = us->pusb_dev->serial;
                length = strlen(us->pusb_dev->serial);

                //ȡ���кŵĺ���λ
                //��������������кų���С��4����0����4λ
                if (length < 4)
                {
                    snprintf(serialnum, sizeof(serialnum), "%s%s", us->pusb_dev->serial, "0000");
                }
                else
                {
                    snprintf(serialnum, sizeof(serialnum), "%s", (ser + length - 4));
                }
                snprintf(DeviceId, sizeof(DeviceId), "%04x%s", giVendor, serialnum);
            }
            else
            {
                //���кŲ����ڣ�ID�ĽṹΪvendor+product
                snprintf(DeviceId, sizeof(DeviceId), "%04x%04x", giVendor, giProduct);
            }

            /*���Ԥ��һ���ַ���������*/
            for (i = 0 ; i < MAX_DEVICEID_NUM - 1 ; i++)
            {
                DeviceId[i] = toupper(DeviceId[i]);
            }

            //����д��proc�ļ���ID
            snprintf(disk->model, sizeof(disk->model), "%s", DeviceId);

            /*diskname,sda,sdb,...*/
            snprintf(disk->diskname, sizeof(disk->diskname), "%s", gd->disk_name);

            /* print the controller name */
            disk->scsihostid = host->host_no;

            /* print product, vendor, and serial number strings */
            if (NULL != us->pusb_dev->manufacturer)
            { string = us->pusb_dev->manufacturer; }
            else if (us->unusual_dev->vendorName)
            { string = us->unusual_dev->vendorName; }
            else
            { string = "Unknown"; }

            snprintf(disk->vendor, sizeof(disk->vendor), "%s", string);

            snprintf(disk->serial, sizeof(disk->serial), "%d", giInterfaceClass);

            if (USB_SPEED_SUPER == us->pusb_dev->speed )
            {
                snprintf(disk->connectiontype, sizeof(disk->connectiontype)
                         , "%s", "USB3.0");
            }
            else if (USB_SPEED_HIGH == us->pusb_dev->speed )
            {
                snprintf(disk->connectiontype, sizeof(disk->connectiontype)
                         , "%s", "USB2.0");
            }
            else
            {
                snprintf(disk->connectiontype, sizeof(disk->connectiontype)
                         , "%s", "USB1.1");
            }
            
            /*disk->capacity��MΪ��λ,sdkp->capacity���߼��������ÿ���߼��鵥λ��512�ֽ�*/
            disk->capacity = (((unsigned long long)(sdkp->capacity) * (U_DISK_BLOCK_SIZE) / 1024)) / 1024; 
            snprintf(disk->usbport, sizeof(disk->usbport), "%s", dev_name(&(us->pusb_dev->dev)));
        }
    }

    return 0;
}



