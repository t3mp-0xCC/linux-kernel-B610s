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
 
#ifndef _USB_CHG_MANAGER_
#define _USB_CHG_MANAGER_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/platform_device.h>
#include "usb_platform_comm.h"
typedef enum
{
    CHG_CHGR_UNKNOWN    = 0,
    CHG_WALL_CHGR       = 1,
    CHG_USB_HOST_PC     = 2,
    CHG_NONSTD_CHGR     = 3,
    CHG_WIRELESS_CHGR   = 4,
    CHG_EXGCHG_CHGR     = 5,
    CHG_500MA_WALL_CHGR = 6,
    CHG_USB_OTG_CRADLE  = 7,
    CHG_HVDCP_CHGR      = 8,/*��ѹ�����*/
    CHG_CHGR_INVALID    = 9,
    CHG_CHGR_TYPE_RSVD  = 0xFFFF /* */
}chg_chgr_type_t;
/**
*   CHARGER(�����)�����ϱ����¼�
*/
typedef enum _GPIO_CHARGER_EVENT
{
    GPIO_CHARGER_ATTATCH = 0,  /* �ڳ��������ʱ�ϱ�����ʾ�г�������� */
    GPIO_CHARGER_REMOVE = 1,       /* �ڳ�����γ�ʱ�ϱ�����ʾ��������γ� */  
    GPIO_WIRELESS_ATTATCH = 2,     /* �����߳��������ʱ�ϱ�����ʾ�����߳�������� */
    GPIO_WIRELESS_REMOVE = 3,      /* �����߳������γ�ʱ�ϱ�����ʾ�����߳�����γ� */
    GPIO_CHARGER_EVENT_MAX     /* ����¼�ֵ��С�ڴ�ֵ����Ϊ�Ƿ��¼� */
}GPIO_CHARGER_EVENT;

typedef struct mbb_charger
{
    chg_chgr_type_t current_chg_type;  /* �������*/
    USB_BOOL  usb_chg_action_flag;       /*�����룬�γ�*/
    USB_INT usb_chg_work_mode;       /*��繤��ģʽ*/
    struct workqueue_struct* usb_chg_notify_wq;
    struct delayed_work usb_chg_notify_work;
    /* ���ö�ʱ�����ڲ���PC�ͷǱ������Ժ�����10s��ʱ������Ƿ�ö�ٳɹ� */
    struct timer_list nonstd_det_timer ;
} mbb_charger_st;


/*���ڵ�����ģʽ���� ����*/
enum usb_chg_mode
{
    USB_CHG_NON = 0, /* �����ģʽ */
    USB_CHG_LINE,       /*USB ���������ģʽ */
    USB_CHG_WIRELESS, /*���߳��ģʽ*/
    USB_CHG_EXTERN,     /* ������ģʽ */
};

/* ����������͵ķ���ʱ�䶨�� */
#define MSM_CHG_DET_DELAY_TIME      (1000 * HZ / 1000)

#define CHG_DETECT_OVERTIME (10 * 1000)   /* 10s */
/* ȫ�ֱ�����¼��ǰ�ĳ������ */

#define MBB_USB_CHARGER_TYPE_HUAWEI                  1
#define MBB_USB_CHARGER_TYPE_NOT_HUAWEI          2
#define MBB_USB_CHARGER_TYPE_PC                           3
#define MBB_USB_CHARGER_TYPE_INVALID                  0

#define MBB_USB_DEVICE_INSERT                         1
#define MBB_USB_DEVICE_IDEN                             2
#define MBB_USB_ENUM_DONE                               3
#define MBB_USB_DEVICE_REMOVE                        0
USB_INT usb_charger_type_detect_event(struct notifier_block* nb, USB_ULONG val, USB_PVOID data);
USB_VOID usb_set_speed(USB_INT speed);
USB_VOID usb_charger_init(USB_VOID);
USB_VOID usb_charger_exit(USB_VOID);
USB_VOID usb_chg_set_work_mode(USB_INT chg_mode);
USB_BOOL usb_chg_check_current_mode(USB_INT chg_mode);

USB_BOOL usb_chg_wireless_detect(USB_VOID);
USB_BOOL usb_chg_wireless_remove(USB_VOID);	


#endif
