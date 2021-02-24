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



#ifndef HUAWEI_PM_H
#define HUAWEI_PM_H

#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/delay.h>
#include "huawei_pm_at_cache.h"
#include "product_nv_def.h"
#include "product_nv_id.h"
#include "bsp_sram.h"
#include "bsp_nvim.h"
#include "mdrv_nvim_comm.h"

#ifndef FALSE
#define FALSE	    (0)
#endif

#ifndef TRUE
#define TRUE	    (1)  
#endif

#ifndef BSP_OK
#define BSP_OK       (0)
#endif

#ifndef BSP_ERROR
#define BSP_ERROR    (-1)
#endif

#ifndef BSP_VOID
typedef void       BSP_VOID;
#endif

/********************909s����*****begin*********************/
/*Define WAKEUP_IN*/
#define WAKEUP_IN               GPIO_0_2
#define LABEL_WAKEUP_IN         "wakeup_in"
/*Define SLEEP_STATUS*/
#define SLEEP_STATUS            GPIO_1_18 
#define LABEL_SLEEP_STATUS      "sleep_status"
/*Define WAKEUP_OUT*/
#define WAKEUP_OUT              GPIO_1_20
#define LABEL_WAKEUP_OUT        "wakeup_out"
/********************909s����*****end*********************/


/********************906s����*****begin*********************/

/********************906s����*****end*********************/

/*WAKEUP_OUT PIN expire time*/
#define WAKEUP_OUT_TIMEOUT          (1000)      

#define GPIO_LOW_VALUE        0
#define GPIO_HIGH_VALUE       1

#define PRINT_ACORE_LOG(print_onoff, fmt, ...)  do { \
    if((print_onoff) == PM_MSG_PRINT_ON) { \
        printk("[module_pm]: <%s> <%d> "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
}while(0)

/*WAKEUP_OUT PIN timer struct*/
typedef struct huawei_pm_timer_type
{
    boolean is_start;                   /*Timer start flag*/  
    struct delayed_work work;           /*timer_type*/
}huawei_pm_timer_type;

/*Define huawei PM struct*/
typedef struct huawei_pm_struct
{
    boolean is_usb_suspend;              /*USB suspend flag*/
    boolean is_usb_enum;                 /*USB enum flag*/
    boolean is_pm_pin_disable;           /*Disable pm pin flag*/
    uint32 rmt_wake_host_mask;           /*Mask for USB&PIN channel*/
    huawei_pm_timer_type pin_timer;      /*Timer for PIN wakeup*/ 
    remote_wakeup_info_type rmt_wk_cfg;  /*Remote wakeup config param*/
}huawei_pm_type;

/*===========================================================================
FUNCTION 
    huawei_pm_remote_wakeupcfg_init
DESCRIPTION
    Init Remote Wakuep Configuration. If NV read failure then use the default
    setting.
DEPENDENCIES
    None
RETURN VALUE
    None
SIDE EFFECTS
    None
===========================================================================*/
void huawei_pm_remote_wakeupcfg_init(void);
/*===========================================================================
 
FUNCTION 
    huawei_pm_get_wakeup_channel_status
 
DESCRIPTION
    Get wakeup channels status.
 
DEPENDENCIES
    None
 
RETURN VALUE
    TRUE:   permit
    FALSE:  forbid
  
SIDE EFFECTS
    None
 
===========================================================================*/
boolean huawei_pm_get_wakeup_channel_status(rmt_wk_chl_type channel);
/******************************************************************************
Function:       huawei_pm_remote_wakuep_update
Description:   update remotewakeup info
Input:           None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void huawei_pm_remote_wakuep_update(remote_wakeup_info_type src_config);
/******************************************************************************
Function:       huawei_pm_save_curc_info
Description:   save curc info
Input:           None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void huawei_pm_save_curc_info(dsat_curc_cfg_type *src_curc);
/******************************************************************************
Function:       huawei_pm_save_curc_info
Description:   save curc info
Input:           None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void huawei_pm_get_curc_info(dsat_curc_cfg_type *dst_curc);

/*****************************************************************************
* �� �� ��  : huawei_pm_remote_wakeup_time_update
* ��������  : ����˯�߻��ѵ����ֳ�ʱʱ��
* �������  : timeValue -- ����ʱ��
* �������  : NA
* �� �� ֵ  : NA
*****************************************************************************/
void huawei_pm_remote_wakeup_time_update(unsigned int timeValue);

/*****************************************************************************
* �� �� ��  : huawei_pm_remote_wakeup_time_get
* ��������  : ��ȡ˯�߻��ѵ����ֳ�ʱʱ��ֵ
* �������  : NA
* �������  : NA
* �� �� ֵ  : remote_wakeup_shake_time -- ��ʱʱ��
*****************************************************************************/
unsigned int huawei_pm_remote_wakeup_time_get(void);

/*****************************************************************************
* �� �� ��  : huawei_pm_remote_wakeup_timer
* ��������  : ������ɵĴ�����
* �������  : NA
* �������  : NA
* �� �� ֵ  : NA
*****************************************************************************/
void huawei_pm_remote_shake_complete(void);

/*===========================================================================
* �� �� ��  : huawei_pm_get_pin_suspended_status
* ��������  : ͨ��PIN���ַ�ʽ��ȡ��λ����˯��״̬
* �������  : NA
* �������  : NA
* �� �� ֵ  : 1-��λ��˯�ߣ�0-��λ����˯��
===========================================================================*/
unsigned int huawei_pm_get_pin_suspended_status(void);

/*************************************************************
* ��   ��   �� : huawei_pm_usb_s34_state_change_handle
* ��������  : ��Դ����ģ���USB�ṩ�Ĵ�����
* �������  : enType --- 1   ��ʾUSB  ����S3/S4
                        0  ��ʾUSB  �˳�S3/S4
* �������  :
* ��  ��  ֵ   :  NA
***************************************************************/
void huawei_pm_usb_s34_state_change_handle(unsigned char enType);

#endif /*HUAWEI_PM_H*/

