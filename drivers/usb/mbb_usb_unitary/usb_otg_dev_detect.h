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



#ifndef _USB_OTG_DEV_DETECT
#define _USB_OTG_DEV_DETECT

#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>

#define GPIO_0_17                     (17)
#define GPIO_2_19                     (83)
#define GPIO_2_20                     (84)

#define GPIO_2_23                       (87)
#define GPIO_2_31	                  (95) 


/*
#if (FEATURE_ON == MBB_CHG_PLATFORM_QUALCOMM)
#define GPIO_OTG_ID_DET      GPIO_0_17
#define GPIO_OTG_ID_SET      GPIO_2_19
#define GPIO_DMDP_CONNECT    GPIO_2_20
#endif
*/
#define GPIO_OTG_ID_DET      OTG_ID_DET_GPIO
#define GPIO_OTG_ID_SET      OTG_ID_SET_GPIO
#define GPIO_DMDP_CONNECT    DMDP_CONNECT_GPIO

/*****9x30************/
//#define GPIO_OTG_ID_DET   7
/*9x25*/
//#define GPIO_OTG_ID_DET      GPIO_0_17
//#define GPIO_OTG_ID_SET      GPIO_2_19
//#define GPIO_DMDP_CONNECT    GPIO_2_20

#define INVALID_STATE  -1
#define USB_OTG_CONNECT_DP_DM    0x0001  /*�̽� D+,D-*/
#define USB_OTG_DISCONNECT_DP_DM 0x0002  /*ֱ������������HS_ID*/
#define USB_OTG_ID_PULL_OUT      0x0003  /*ֱ������������HS_ID*/
#define USB_OTG_FAST_OFF   0x0004        /*�ٹػ�*/
#define USB_OTG_FAST_ON    0x0005       /*�ٿ���*/

/*OTG �豸֧��*/
#define USB_OTG_FEATURE_NONE 0x00
#define USB_OTG_FEATURE_CRADLE 0x01
#define USB_OTG_FEATURE_EXTCHG 0x10
#define USB_OTG_FEATURE_CRADLE_EXTCHG 0x11
#define USB_OTG_FEATURE_CRADLE_MASK 0x0f
#define USB_OTG_FEATURE_EXTCHG_MASK 0xf0


#define GPIO_HIGH   1
#define GPIO_LOW    0

#define HOST_ON 0
#define HOST_OFF 1

#define ID_FLOAT 1
#define ID_GROUND 0

#define VBUS_UP   1
#define VBUS_DOWN 0

#define PLATFORM_QUALCOMM   0
#define PLATFORM_BALONG     1

#define OTG_INSERT 1
#define OTG_REMOVE 0

#define OTG_DEV_EVENT_PROC   MBB_USB_TRUE
#define OTG_DEV_EVENT_NONPROC MBB_USB_FALSE

#if(MBB_CHG_EXTCHG == FEATURE_ON)
#define OTG_DET_LONG_DELAY_TIME        HZ  /*OTG  ID �жϷ����жϼ����ʱ*/
#else
#define OTG_DET_LONG_DELAY_TIME        HZ/2/*USB  ID �жϷ����жϼ����ʱ*/
#endif

#define OTG_DET_SHORT_DELAY_TIME       HZ/20  /*��Ҫ���ӳ�*/

/*
 ��¼OTG �豸�γ����̣�
 ����γ�ʱ�����Ϣ�ظ��ϱ�
*/
enum otg_dev_remove_process
{
    OTG_DEV_INVALID_FLAG,
    OTG_DEV_REMOVE_PROC,    /*��ʼ����otg �豸�γ�����*/
    OTG_DEV_REMOVE_DONE     /*otg �豸�γ������vbus �µ�
                                                            ���������øı�־*/
};

typedef enum _otg_device
{
    OTG_DEVICE_UNDEFINED= 0,
    OTG_DEVICE_EXTCHAGER = 1,
    OTG_DEVICE_CRADLE = 2, /*���������ͣ�ʵ���������*/
    OTG_DEVICE_CRADLE_CHARGE = 3, /*��Ʒ��֧��cradle�������*/
    OTG_DEVICE_MAX,
} OTG_DEVICE_TYPE;

struct otg_debug
{
    unsigned int stat_usb_id_insert;                   /*����ǰ�������*/
    unsigned int stat_usb_id_insert_proc;               /*������ ����ǰ�������*/
    unsigned int stat_usb_id_insert_proc_end;          /*������ ����ɺ�������*/
    unsigned int stat_usb_id_remove;                         /*����ǰ�γ�����*/
    unsigned int stat_usb_id_remove_proc;               /*��������ǰ�γ�����*/
    unsigned int stat_usb_id_remove_proc_end;        /*���������γ�����*/
    unsigned int stat_usb_id_no_trigger;             /*ID ��ƽ�仯û�д����жϵļ���*/
    unsigned int stat_usb_dpdm_connect;                    /*D+ D-ֱ��*/
    unsigned int stat_usb_dpdm_disconnect;                /*D+ D-�Ͽ�*/
    unsigned int stat_usb_otg_fast_off;                      /*����ٹػ������ж�*/
    unsigned int stat_usb_otg_fast_on;                       /*�˳��ٹػ�ʹ���ж�*/
    unsigned int stat_usb_kick_timeout ;                        /*������jiffies*/
};
struct otg_dev
{
    OTG_DEVICE_TYPE dev_type;
    int  status;
};
struct otg_dev_det
{
    int  platform;                   /*ƽ̨����*/
    /*otg detect basic elements*/
    int  id;                         /*USB ID״̬*/
    int  old_id;
    int  vbus;                       /*VUBS ״̬*/
    int  host;                       /*�����Ƿ���host ģʽ*/
    int  phy_id;                     /*����Ƿ�Ҫ�л���host*/
    
    int  id_det_gpio;                /*���OTG �豸Ӳ���ж�*/
    unsigned long id_irqflags;              /*id �жϴ�����ʽ����*/
    int  id_irq_shared;              /*id �жϺ��Ƿ���*/

    int  notify_event;               /*�ϱ���Ӧ�õ��¼�*/
    int  otg_old_event_id;       /*��¼OTG�յ���event�¼�*/
    int  charge_type;
    int  otg_remove_flag;         /*��¼otg �豸�γ���vbus �µ����*/
   // USB_INT phy_in_lpm;                 /*usb оƬ�Ƿ��ڵ͵�ģʽ*/
    struct wake_lock id_wake_lock;      
    struct delayed_work otg_id_detect_work;   /*OTG �豸����ж��°벿*/


    /* protects detect process*/
    //spinlock_t            lock;
    //spinlock_t            notify_lock;
    //wait_queue_head_t wait_wq;
    
    struct otg_debug debug;
    unsigned long int otg_feature;
    struct otg_dev  dev; 
    void* contex;
};


typedef int (*adp_otg_dev_cradle_state)(void);
typedef int (*adp_otg_dev_id_state)(void);
typedef void (*adp_otg_dev_set_remove_flags)(int remove_flag);
typedef int (*adp_otg_dev_get_remove_flags)(void);
typedef void (*adp_otg_usb_notify_event)(unsigned long val, void* v);

/*otg dev KO API��*/
typedef struct _usb_otg_hanlder
{
    adp_otg_dev_cradle_state    otg_dev_cradle_state_cb;
    adp_otg_dev_id_state    otg_dev_id_state_cb;
    adp_otg_dev_set_remove_flags  otg_dev_set_remove_flags_cb;
    adp_otg_dev_get_remove_flags  otg_dev_get_remove_flags_cb;
    adp_otg_usb_notify_event  otg_usb_notify_event_cb;
} usb_otg_hanlder_t;

void otg_dev_request_source(void);
void otg_dev_free_source(void);
void otg_dev_set_platform(struct otg_dev_det* otg);

/*otg dev adp API*/
void otg_gpio_clear_set(int id_gpio);
void otg_id_gpio_irq_set(int id_gpio);
void otg_id_trigger_set(struct otg_dev_det* otg);
void otg_host_on_off(struct otg_dev_det* otg);
int otg_get_vbus_state(struct otg_dev_det* otg);
int otg_id_irq_share_protect(struct otg_dev_det* otg);
void otg_id_irq_flags_set(struct otg_dev_det* otg);
void otg_first_report_charge_type(struct otg_dev_det* otg);
void otg_dev_source_set(void);
void otg_exchg_connect_dpdm(struct otg_dev_det* otg);
void otg_exchg_disconnect_dpdm(struct otg_dev_det* otg);
void otg_exchg_disconnect_dpdm_to_host(struct otg_dev_det* otg);
usb_otg_hanlder_t *usb_get_otg_ctx(void);
void usb_otg_device_detect_exit(void);
void  usb_otg_device_detect_init(void);
void  product_set_otg_dev_support_feature(USB_VOID);

#endif

