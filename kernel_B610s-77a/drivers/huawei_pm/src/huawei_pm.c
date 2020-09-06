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



#include <linux/irq.h>
#include <linux/gpio.h>
#include "bsp_icc.h"
#include "huawei_pm.h"
#include "mdrv_version.h"
#if (FEATURE_ON == MBB_USB)
#include "mdrv_usb.h"
#endif
/*Global status for huawei PM*/
/*It's only accessed in this file,
other modules can visit it through 
call none static function in this file*/
huawei_pm_type g_huawei_pm_status =
{
    TRUE,
    FALSE,
    FALSE,
    
    RMT_WK_CHN_PIN | RMT_WK_CHN_USB,
    {0},
    {
        REMOTE_WAKEUP_ON, 
        RMT_WK_CHN_PIN | RMT_WK_CHN_USB,
        RMT_WK_SRC_SMS | RMT_WK_SRC_VOICE | RMT_WK_SRC_DATA | RMT_WK_SRC_UR
    }
};

extern unsigned int g_PmAcorePrintOnOff;

dsat_curc_cfg_type g_curc_info = 
{
    DSAT_UR_ENABLE
};
static struct delayed_work wakeup_in_sleep_work;
static struct delayed_work wakeup_in_wake_work;
static struct delayed_work usb_resume_acore_work;
static struct delayed_work usb_resume_ccore_work;

struct wake_lock remote_wakeup_pin_lock; /*输出1s高电平过程中防止睡眠锁*/
struct wake_lock wakeup_in_lock;         /*wakeup_in控制睡眠锁*/
struct wake_lock flush_cache_lock;       /*清缓存防止睡眠锁*/

/*用于保存远程唤醒缓存的超时时间*/
unsigned int  remote_wakeup_shake_time = 2000;
unsigned int  shake_timeout_work_flag = 0;
static struct delayed_work  shake_timeout_work;
extern void huawei_pm_rsp_inform_flush_cache_acore(void);

/* for s3/s4 cann't report msg new request */
unsigned char remote_wakeup_ctrl_bakeup = REMOTE_WAKEUP_ON;
/*USB s3/s4的标记，FALSE表示进入S3/S4，TRUE表示退出*/
static unsigned char usb_s34_state_flag = FALSE;
static DEFINE_SPINLOCK(usb_s34_spin);
/*************************************************************
* 函   数   名 : huawei_pm_usb_s34_state_entry_handle
* 功能描述  : USB进入s3/s4阶段的处理函数
* 输入参数  : NA
* 输出参数  :
* 返 回 值     : NA
***************************************************************/
static void huawei_pm_usb_s34_state_entry_handle()
{
    int ret = 0;
    int value = PM_MSG_USB_S34_IN;
    unsigned int icc_channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_PM;
    unsigned long flag = 0;

    spin_lock_irqsave(&usb_s34_spin,flag); 

    if(TRUE == usb_s34_state_flag)
    {
        spin_unlock_irqrestore(&usb_s34_spin,flag);
        return;
    }

    /*备份远程唤醒开关，并关A核闭远程唤醒*/
    remote_wakeup_ctrl_bakeup = \
        g_huawei_pm_status.rmt_wk_cfg.remote_wakeup_ctrl;
    g_huawei_pm_status.rmt_wk_cfg.remote_wakeup_ctrl = REMOTE_WAKEUP_OFF;

    /*发消息给C核，关闭C核远程唤醒*/
    ret = bsp_icc_send(ICC_CPU_MODEM, icc_channel_id, (u8*)&value, sizeof(value));
    if (ret != (int)sizeof(value))
    {
        PRINT_ACORE_LOG(PM_MSG_PRINT_ON,"send Ccore USB entry s34 state msg fail !!!\n");
    }

    usb_s34_state_flag = TRUE;

    spin_unlock_irqrestore(&usb_s34_spin,flag);
}

/*************************************************************
* 函   数   名 : huawei_pm_usb_s34_state_exit_handle
* 功能描述  : USB退出s3/s4阶段的处理函数
* 输入参数  : NA
* 输出参数  :
* 返 回 值     : NA
***************************************************************/
static void huawei_pm_usb_s34_state_exit_handle()
{
    int ret = 0;
    int value = PM_MSG_USB_S34_OUT;
    unsigned int icc_channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_PM;
    unsigned long flag = 0;

    spin_lock_irqsave(&usb_s34_spin,flag); 

    if(FALSE == usb_s34_state_flag)
    {
        spin_unlock_irqrestore(&usb_s34_spin,flag);
        return;
    }

    /*恢复远程唤醒开关的原始状态值*/
    g_huawei_pm_status.rmt_wk_cfg.remote_wakeup_ctrl = remote_wakeup_ctrl_bakeup;
    /*发消息给C核，恢复C核远程唤醒的原始状态值*/
    ret = bsp_icc_send(ICC_CPU_MODEM, icc_channel_id, (u8*)&value, sizeof(value));
    if (ret != (int)sizeof(value))
    {
        PRINT_ACORE_LOG(PM_MSG_PRINT_ON,"send Ccore USB entry s34 state msg fail !!!\n");
    }

    usb_s34_state_flag = FALSE;

    spin_unlock_irqrestore(&usb_s34_spin,flag);

}

/*************************************************************
* 函   数   名 : huawei_pm_usb_s34_state_change_handle
* 功能描述  : 电源管理模块给USB提供的处理函数
* 输入参数  : enType --- TRUE   表示USB  进入S3/S4
                                             FALSE  表示USB  退出S3/S4
* 输出参数  :
* 返  回  值   :  NA
***************************************************************/
void huawei_pm_usb_s34_state_change_handle(unsigned char enType)
{
    nv_huawei_s3s4_remote_wakeup_enable  wakeup_enable = {0};
    if (NV_OK != NV_ReadEx(MODEM_ID_0, NV_HUAWEI_S3S4_REMOTE_WAKEUP_ENABLE, 
                                    &wakeup_enable, sizeof(wakeup_enable)))
    {
        PRINT_ACORE_LOG(PM_MSG_PRINT_ON," NV_HUAWEI_S3S4_REMOTE_WAKEUP_ENABLE read fail!!! \n"); 
        /*如果NV读失败，默认为1*/
        wakeup_enable.remote_wakeup_enable = 1;
    }
    /*如果NV值不为0，表示客户希望在WIN7系统上，
    主机进入S3S4后，远程唤醒继续生效，
    主动上报可报。版本默认为1，用户可用PCVENDOR定制*/	
    if (0 != wakeup_enable.remote_wakeup_enable)
    {
        return;
    }
    if(enType)
    {
        huawei_pm_usb_s34_state_entry_handle();
    }
    else
    {
        huawei_pm_usb_s34_state_exit_handle();
    }
}

/*===========================================================================
FUNCTION 
    huawei_pm_reset_pin_status
DESCRIPTION
    Reset the WAKEUP_OUT PIN status
DEPENDENCIES
    None
RETURN VALUE
    None.
SIDE EFFECTS
    None
===========================================================================*/
static void huawei_pm_reset_pin_status(void)
{
    gpio_direction_output(WAKEUP_OUT, GPIO_LOW_VALUE); 
    g_huawei_pm_status.pin_timer.is_start = FALSE;
}

/*===========================================================================
FUNCTION 
    huawei_pm_request_wakeup_pin_cb
DESCRIPTION
    Callback for WAKEUP_OUT PIN wakeup timer
DEPENDENCIES
  None
RETURN VALUE
  None.
SIDE EFFECTS
  None
===========================================================================*/
static void huawei_pm_request_wakeup_pin_cb(struct work_struct *w)
{
    huawei_pm_reset_pin_status();
    /*1s高电平远程唤醒唤醒信号输出结束，投赞成票，保证不影响整个系统睡眠*/
    wake_unlock(&remote_wakeup_pin_lock);
}

/*===========================================================================
FUNCTION 
    huawei_pm_sleep_in_event_isr
DESCRIPTION
    The interrupt handler for WAKEUP_IN interrupt.
DEPENDENCIES
    None
RETURN VALUE
    None
SIDE EFFECTS
    None
===========================================================================*/
static irqreturn_t huawei_pm_sleep_in_event_isr(void)
{
    int ucdata = 0;
    unsigned int uclevel = 0;
    pm_status_s *pm_st = (pm_status_s *)(SRAM_MODULE_PM_ADDR);
    
    ucdata = gpio_int_state_get(WAKEUP_IN);

    if (!ucdata)
    {
        return IRQ_NONE;
    }
    gpio_int_state_clear(WAKEUP_IN);  

    uclevel = gpio_get_value(WAKEUP_IN);

    if ( uclevel )
    {
        /*wakeup_in拉高,投票不允许待机*/
        wake_lock(&wakeup_in_lock);
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff, "high level irq happen !\n");
            
        pm_st->pin_status = PIN_RESUME_STATUS;
        gpio_int_trigger_set( WAKEUP_IN, IRQ_TYPE_EDGE_FALLING);        
        /*上位机通过wakeup_in唤醒模块，说明上位机处于noraml态，发送pm的缓存数据*/
        huawei_pm_remote_shake_complete();
    }
    else
    {
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff, "low level irq happen !\n");
        gpio_int_trigger_set(WAKEUP_IN,IRQ_TYPE_EDGE_RISING);
        pm_st->pin_status = PIN_SUSPEND_STATUS;
        /*wakeup_in拉低后,投票允许待机*/
        wake_unlock(&wakeup_in_lock);
    }

    return IRQ_HANDLED;
        
}

/*===========================================================================
FUNCTION 
    huawei_pm_udpate_nv_configuration
DESCRIPTION
    Update NV Setting through dsatutil_put_nv_item.
DEPENDENCIES
    This API just allowed been used by DS task.
RETURN VALUE
    TRUE:   success.
    FALSE:  fail
SIDE EFFECTS
    None
===========================================================================*/
static boolean huawei_pm_udpate_nv_configuration
(
    remote_wakeup_info_type config
)
{
    /* status var for reading from NV */
    remote_wakeup_info_type local_nv_item;
    
    local_nv_item.remote_wakeup_ctrl 
                                = config.remote_wakeup_ctrl;
    local_nv_item.remote_wakeup_channel 
                                = config.remote_wakeup_channel;
    local_nv_item.remote_wakeup_source 
                                = config.remote_wakeup_source;
    
    if (NV_OK != NV_WriteEx(MODEM_ID_0,
        en_NV_Item_WAKEUP_CFG_FLAG, &local_nv_item, sizeof(local_nv_item)))
    {
        return FALSE;
    }

    return TRUE;
}

/*===========================================================================
FUNCTION 
    huawei_pm_disable_wakeup_pin
DESCRIPTION
    Set disable pull WAKEUP_OUT PIN flag.
DEPENDENCIES
    None
RETURN VALUE
    None.
SIDE EFFECTS
    None
===========================================================================*/
void huawei_pm_disable_wakeup_pin(boolean is_disable)
{
    g_huawei_pm_status.is_pm_pin_disable = is_disable;
}
 
/*===========================================================================
FUNCTION 
    huawei_pm_request_wakeup_pin
DESCRIPTION
    Wakeup the Host through WAKEUP_OUT PIN
DEPENDENCIES
    None
RETURN VALUE
    None.
SIDE EFFECTS
    None
===========================================================================*/
void huawei_pm_request_wakeup_pin(void)
{
    if (TRUE == g_huawei_pm_status.is_pm_pin_disable)
    {
        return;
    }

    /*If the timer starting,We do nothing*/
    if (FALSE == g_huawei_pm_status.pin_timer.is_start)
    {
        /* 投反对票，保证单板能够正常输出1秒PIN高电平信号，
          *否则单板在拉高PIN后无法被恢复拉低 */
        wake_lock(&remote_wakeup_pin_lock);
        g_huawei_pm_status.pin_timer.is_start = TRUE;
        gpio_direction_output(WAKEUP_OUT, GPIO_HIGH_VALUE);
        /*Start timer*/
        schedule_delayed_work(&g_huawei_pm_status.pin_timer.work, \
            msecs_to_jiffies(WAKEUP_OUT_TIMEOUT));
    }
}

/*****************************************************************************
* 函 数 名  : huawei_pm_remote_wakeup_time_update
* 功能描述  : 更新睡眠唤醒的握手超时时间
* 输入参数  : timeValue -- 设置时间
* 输出参数  : NA
* 返 回 值  : NA
*****************************************************************************/
void huawei_pm_remote_wakeup_time_update(unsigned int timeValue)
{
    remote_wakeup_shake_time = timeValue;
}

/*****************************************************************************
* 函 数 名  : huawei_pm_remote_wakeup_time_get
* 功能描述  : 获取睡眠唤醒的握手超时时间值
* 输入参数  : NA
* 输出参数  : NA
* 返 回 值  : remote_wakeup_shake_time -- 超时时间
*****************************************************************************/
unsigned int huawei_pm_remote_wakeup_time_get(void)
{
    return remote_wakeup_shake_time;
}

/*****************************************************************************
* 函 数 名  : huawei_pm_remote_wakeup_timerout_cb
* 功能描述  : 握手超时处理
* 输入参数  : NA
* 输出参数  : NA
* 返 回 值  : NA
*****************************************************************************/
void huawei_pm_remote_wakeup_timerout_cb(void)
{
    /*清除标志*/
    shake_timeout_work_flag = FALSE;
    /*通知A/C核，开始发送缓存数据*/
    schedule_delayed_work(&usb_resume_acore_work, 0);
    schedule_delayed_work(&usb_resume_ccore_work, 0);

    /*延时500ms，保证数据能够发送出去，再投赞成票允许单板睡眠*/
    msleep(500);
    /*数据发送完成后，投赞成票允许睡眠*/
    wake_unlock(&flush_cache_lock);
}

/*****************************************************************************
* 函 数 名  : huawei_pm_remote_wakeup_timer
* 功能描述  : 起握手超时定时器
* 输入参数  : NA
* 输出参数  : NA
* 返 回 值  : NA
*****************************************************************************/
void huawei_pm_remote_wakeup_timer(void)
{
    /*当定时器已启动，则不重复起定时器*/
    if(TRUE == shake_timeout_work_flag)
    {
        return;
    }
    
    /*使用按键的投票项投反对票，保证在定时器时间段内单板不睡眠*/
    wake_lock(&flush_cache_lock);

    /*Start timer*/
    schedule_delayed_work(&shake_timeout_work, \
        msecs_to_jiffies(remote_wakeup_shake_time));
    shake_timeout_work_flag = TRUE;
}

/*****************************************************************************
* 函 数 名  : huawei_pm_remote_wakeup_timer
* 功能描述  : 握手完成的处理函数
* 输入参数  : NA
* 输出参数  : NA
* 返 回 值  : NA
*****************************************************************************/
void huawei_pm_remote_shake_complete(void)
{
    /*通知A/C核，开始发送缓存数据*/
    schedule_delayed_work(&usb_resume_acore_work, 0);
    schedule_delayed_work(&usb_resume_ccore_work, 0);
}

/*===========================================================================
FUNCTION 
    huawei_pm_get_usb_suspended_status
DESCRIPTION
    Get USB Suspend Status.
DEPENDENCIES
    None
RETURN VALUE
    if USB has enter SUSPEND then return TRUE,
    else return FALSE;
SIDE EFFECTS
    None
===========================================================================*/
boolean huawei_pm_get_usb_suspended_status(void)
{
    PRINT_ACORE_LOG(g_PmAcorePrintOnOff," is_usb_suspend is %d!\n",\
                                 g_huawei_pm_status.is_usb_suspend );

    return g_huawei_pm_status.is_usb_suspend;
}

/*===========================================================================
* 函 数 名  : huawei_pm_get_pin_suspended_status
* 功能描述  : 通过PIN握手方式获取上位机的睡眠状态
* 输入参数  : NA
* 输出参数  : NA
* 返 回 值  : 1-上位机睡眠；0-上位机非睡眠
===========================================================================*/
unsigned int huawei_pm_get_pin_suspended_status(void)
{
    BSP_U32 pin_status = 0;
    pm_status_s *pm_st = (pm_status_s *)(SRAM_MODULE_PM_ADDR);

    pin_status = pm_st->pin_status;

    if( PIN_SUSPEND_STATUS ==  pin_status )
    {
        /*lint -e506*/
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff, "uart suspend!\n");
        /*lint +e506*/
        return 1;
    }
    if( PIN_RESUME_STATUS ==  pin_status )
    {
        /*lint -e506*/
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff, "uart resumed!\n");
        /*lint +e506*/
        return 0;
    }

    PRINT_ACORE_LOG(PM_MSG_PRINT_ON,"GET MEMORY_PIN_STATUS_ADDR ERROR!\n");

    return 1;
}

/*===========================================================================
FUNCTION 
    huawei_pm_set_usb_enum_status
DESCRIPTION
    Set USB has been enumerated.
DEPENDENCIES
    None
RETURN VALUE
    None
SIDE EFFECTS
    None
 ===========================================================================*/
void huawei_pm_set_usb_enum_status(void)
{
    pm_status_s *pm_st = (pm_status_s *)(SRAM_MODULE_PM_ADDR);
    pm_st->usb_status = USB_RESUME_STATUS;

    g_huawei_pm_status.is_usb_enum = TRUE;
    g_huawei_pm_status.is_usb_suspend = FALSE;
    //todo: BSP_PWRCTRL_SleepVoteLock(PWRCTRL_SLEEP_USB);
}

/*===========================================================================
FUNCTION 
    huawei_pm_handle_usb_resume_complete
DESCRIPTION
    Do something when USB resume complete
DEPENDENCIES
    None
RETURN VALUE
    None.
SIDE EFFECTS
    None
===========================================================================*/
void huawei_pm_handle_usb_resume_complete(void)
{
    pm_status_s *pm_st = (pm_status_s *)(SRAM_MODULE_PM_ADDR);

    if (TRUE == g_huawei_pm_status.is_usb_suspend)
    {
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff,"usb resume !!!\n");
        g_huawei_pm_status.is_usb_suspend = FALSE;        
        /*Update wakeup channel mask*/
        g_huawei_pm_status.rmt_wake_host_mask 
                = g_huawei_pm_status.rmt_wk_cfg.remote_wakeup_channel;

        pm_st->usb_status = USB_RESUME_STATUS;
        schedule_delayed_work(&usb_resume_acore_work, 0);
        schedule_delayed_work(&usb_resume_ccore_work, 0);
    }
    return;

}

static void huawei_pm_usb_resume_acore_cb(struct work_struct *w)
{
    /*Indicate Flush all stored response*/
    huawei_pm_rsp_inform_flush_cache_acore();
}
static void huawei_pm_usb_resume_ccore_cb(struct work_struct *w)
{
    int ret = 0;
    int value = PM_MSG_FLUSH_CACHE;
    unsigned int icc_channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_PM;

    ret = bsp_icc_send(ICC_CPU_MODEM, icc_channel_id, (u8*)&value, sizeof(value));
    if (ret != (int)sizeof(value))
    {
        PRINT_ACORE_LOG(PM_MSG_PRINT_ON,"send Ccore flush cache msg fail !!!\n");
    }
}

/*===========================================================================
FUNCTION 
    huawei_pm_handle_usb_suspend
DESCRIPTION
    Do something when USB suspend.
DEPENDENCIES
    None.
RETURN VALUE
    None.
SIDE EFFECTS
    None
===========================================================================*/
void huawei_pm_handle_usb_suspend(void)
{
    pm_status_s *pm_st = (pm_status_s *)(SRAM_MODULE_PM_ADDR);

    if (FALSE == g_huawei_pm_status.is_usb_suspend)
    {
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff," usb suspend !!!\n");
        g_huawei_pm_status.is_usb_suspend = TRUE;
        
        /*Disable USB wakeup channel*/
        g_huawei_pm_status.rmt_wake_host_mask &= ~RMT_WK_CHN_USB;
        pm_st->usb_status = USB_SUSPEND_STATUS;
    }
    return;
}

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
boolean huawei_pm_get_wakeup_channel_status(rmt_wk_chl_type channel)
{
    if (channel == (g_huawei_pm_status.rmt_wk_cfg.remote_wakeup_channel & channel))
    {
        return TRUE;
    }
    
    return FALSE;    
}
int huawei_pm_get_usb_channel_status(void)
{
    if (RMT_WK_CHN_USB == (g_huawei_pm_status.rmt_wake_host_mask & RMT_WK_CHN_USB))
    {
        return TRUE;
    }
    
    return FALSE;    
}

/*===========================================================================
FUNCTION 
    huawei_pm_update_wakeup_channel_status
DESCRIPTION
    Update wakeup channels status.
DEPENDENCIES
    None
RETURN VALUE
    None
SIDE EFFECTS
    None
===========================================================================*/
void huawei_pm_update_wakeup_channel_status(void)
{
    /*Update wakeup channel mask*/
    g_huawei_pm_status.rmt_wake_host_mask 
                = g_huawei_pm_status.rmt_wk_cfg.remote_wakeup_channel;  
}

/*===========================================================================
FUNCTION
    huawei_pm_sleep_status_output
DESCRIPTION
    SLEEP_STATUS pin output
DEPENDENCIES
    None
RETURN VALUE
    None
SIDE EFFECTS
    None
===========================================================================*/
void huawei_pm_sleep_status_output(int  level)
{
    gpio_direction_output(SLEEP_STATUS, level);
}
/******************************************************************************
Function:       huawei_pm_remote_wakuep_update
Description:   update remotewakeup info
Input:           None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void huawei_pm_remote_wakuep_update(remote_wakeup_info_type src_config)
{
    g_huawei_pm_status.rmt_wk_cfg = src_config;

    g_huawei_pm_status.rmt_wake_host_mask 
        = src_config.remote_wakeup_channel;
}
/******************************************************************************
Function:       huawei_pm_save_curc_info
Description:   save curc info
Input:           None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void huawei_pm_save_curc_info(dsat_curc_cfg_type *src_curc)
{
    int i;
    if( NULL == src_curc )
    {
        return;
    }
    g_curc_info.ctrl_status = src_curc->ctrl_status;

    return;
}
/******************************************************************************
Function:       huawei_pm_save_curc_info
Description:   save curc info
Input:           None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void huawei_pm_get_curc_info(dsat_curc_cfg_type *dst_curc)
{
    int i;
    if( NULL == dst_curc )
    {
        return;
    }
    dst_curc->ctrl_status = g_curc_info.ctrl_status;
}

/******************************************************************************
Function:       huawei_pm_get_curc_status
Description:   
Input:           huawei_pm_rsp_id_type send_id
Output:         None
Return:          dsat_curc_status
Others:          None
******************************************************************************/
dsat_curc_status huawei_pm_get_curc_status(void)
{
    return g_curc_info.ctrl_status;
}
/******************************************************************************
Function:       huawei_pm_wakeup_remote_host
Description:   
Input:           rmt_wk_src_type rmt_wk_src
Output:         None
Return:          boolean wether the src can wakeup host
Others:          None
******************************************************************************/
boolean huawei_pm_wakeup_remote_host(rmt_wk_src_type rmt_wk_src)
{
    /*Check wakeup source validity*/    
    if ((REMOTE_WAKEUP_OFF 
            == g_huawei_pm_status.rmt_wk_cfg.remote_wakeup_ctrl) 
            || (rmt_wk_src 
            != (rmt_wk_src 
            & g_huawei_pm_status.rmt_wk_cfg.remote_wakeup_source)) 
            || ( 0 == g_huawei_pm_status.rmt_wk_cfg.remote_wakeup_channel ))
    {
        return FALSE;
    }

    return TRUE;
}

/*****************************************************************************
 函 数 名  : bsp_pm_read_cb
 功能描述  : A核核间回调函数
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static s32 bsp_pm_read_cb( void )
{
    int ret = 0;
    int read_len = 0;
    int value;
    u32 channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_PM;

    read_len = bsp_icc_read(channel_id, (u8*)&value, sizeof(value));
    if(read_len != (int)sizeof(value))
    {
        PRINT_ACORE_LOG(PM_MSG_PRINT_ON,"read len(%x) != expected len(%x) !\n");

        return -1;
    }
    
    PRINT_ACORE_LOG(g_PmAcorePrintOnOff, "bsp_pm_read_cb: value = %d \n", value);

    switch(value)
    {
        case PM_MSG_RMT_CHANNEL_SET:
            huawei_pm_update_wakeup_channel_status();
            break;
        case PM_MSG_WAKEUP_PIN_SET:
            huawei_pm_request_wakeup_pin();
            break;
        case PM_MSG_WAKEUP_TIMER_SET:
            huawei_pm_remote_wakeup_timer();
            break;
        default:
            PRINT_ACORE_LOG(PM_MSG_PRINT_ON," invalid para !!! \n");
            break;
    }

    return 0;
}
/******************************************************************************
Function:       huawei_pm_bsp_ifc_init
Description:   初始化双核通信接口
Input:           None
Output:         None
Return:         uint32
Others:         None
******************************************************************************/
int huawei_pm_bsp_ifc_init(void)
{
    uint32 u32Ret = 0;

    int ret = 0;
    BSP_U32 icc_channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_PM;
    
    ret |= bsp_icc_event_register(icc_channel_id, \
        (read_cb_func)bsp_pm_read_cb, NULL, NULL, NULL);

    return ret;
}

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
void huawei_pm_remote_wakeupcfg_init(void)
{
    /* status var for reading from NV */
    /* NV interface data */
    remote_wakeup_info_type local_config = {0};
      
    /*If NV read failure then use the default*/
    if (NV_OK == NV_ReadEx(MODEM_ID_0, en_NV_Item_WAKEUP_CFG_FLAG, 
                                    &local_config, sizeof(local_config)))
    {
        /*Check the range of remote wakeup configuration*/
        if ((REMOTE_WAKEUP_ON != local_config.remote_wakeup_ctrl)
            && (REMOTE_WAKEUP_OFF != local_config.remote_wakeup_ctrl))
        {
            /*The NV values out of range,Use global value update it*/
            (void)huawei_pm_udpate_nv_configuration
                    (g_huawei_pm_status.rmt_wk_cfg);

        }
        else if(RMT_WK_CHN_MAX < local_config.remote_wakeup_channel)
        {
            /*The NV values out of range,Use global value update it*/
            (void)huawei_pm_udpate_nv_configuration
                    (g_huawei_pm_status.rmt_wk_cfg);        
        }
        else if(RMT_WK_SRC_MAX < local_config.remote_wakeup_source)
        {
            /*The NV values out of range,Use global value update it*/
            (void)huawei_pm_udpate_nv_configuration
                    (g_huawei_pm_status.rmt_wk_cfg);        
        }        
        else
        {
            /*Use NV values*/
            g_huawei_pm_status.rmt_wk_cfg = local_config;
            /*Update channel mask*/
            g_huawei_pm_status.rmt_wake_host_mask 
                = local_config.remote_wakeup_channel;
        }
    }/*end if (NV_DONE_S == nv_status)*/
    else
    {
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff, "read wakeupcfg nv fail !!! \n");
    }
}
/******************************************************************************
Function:       huawei_pm_wakeupout_gpio_init
Description:   Initialize the wakeup_out gpio
Input:           None
Output:         None
Return:         int, 0 = success, -1 = failed
Others:         None
******************************************************************************/
int huawei_pm_sleepstatus_gpio_init(void)
{
    int ret = 0;
    ret = gpio_request(SLEEP_STATUS, LABEL_SLEEP_STATUS);
    if(ret)
    {
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff, "request SLEEP_STATUS fail  \n");
        goto fail1;
    }
    gpio_direction_output(SLEEP_STATUS, GPIO_HIGH_VALUE); 

    return 0;
fail1:
    return -1;
}

/******************************************************************************
Function:       huawei_pm_wakeupout_gpio_init
Description:   Initialize the wakeup_out gpio
Input:           None
Output:         None
Return:         int, 0 = success, -1 = failed
Others:         None
******************************************************************************/
int huawei_pm_wakeupout_gpio_init(void)
{
    int ret = 0;

    /*配置该管脚为GPIO，并标志该GPIO已经在使用*/
    ret = gpio_request(WAKEUP_OUT, LABEL_WAKEUP_OUT);

    if(ret)
    {
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff, "request WAKEUP_OUT fail  \n");
        goto fail1;
    }
    
    /*设置GPIO为输出，低电平*/
    gpio_direction_output(WAKEUP_OUT, GPIO_LOW_VALUE); 
    return 0;

fail1:
    return -1;
}

/******************************************************************************
Function:       huawei_pm_wakeupin_gpio_init
Description:   Initialize the wakeup_in gpio
Input:           None
Output:         None
Return:         int, 0 = success, -1 = failed
Others:         None
******************************************************************************/
int huawei_pm_wakeupin_gpio_init(void)
{
    int ret = 0;
    int level = 0;
    unsigned int int_num;
    pm_status_s *pm_st = (pm_status_s *)(SRAM_MODULE_PM_ADDR);

    ret = gpio_request(WAKEUP_IN, LABEL_WAKEUP_IN);
    if(ret)
    {
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff, "request WAKEUP_IN fail  \n");
        goto fail1;
    }
    /*设置GPIO为输入*/
    gpio_direction_input(WAKEUP_IN);
    gpio_int_mask_set(WAKEUP_IN);

    /*设置GPIO为中断方式*/
    level = gpio_get_value(WAKEUP_IN);

    /*设置GPIO输入中断触发方式*/
    if(level)
    {
        gpio_int_trigger_set( WAKEUP_IN, IRQ_TYPE_EDGE_FALLING);
        /*高电平，不允许休眠*/
        wake_lock(&wakeup_in_lock);
        pm_st->pin_status = PIN_RESUME_STATUS;
    }
    else
    {
        gpio_int_trigger_set(WAKEUP_IN,IRQ_TYPE_EDGE_RISING);
        pm_st->pin_status = PIN_SUSPEND_STATUS;
        /*低电平，允许休眠*/
        wake_unlock(&wakeup_in_lock);
    }

    gpio_set_function(WAKEUP_IN,GPIO_INTERRUPT);
    int_num = gpio_to_irq(WAKEUP_IN);
    /*挂载中断号对应的回调函数*/
    ret = request_irq(int_num, \
            (irq_handler_t) huawei_pm_sleep_in_event_isr, IRQF_SHARED, LABEL_WAKEUP_IN, "wakeup_in");
    if (ret) 
    {
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff,"request WAKEUP_IN irq fail  \n");
        goto fail2;
    }
    
    /*使能GPIO中断*/
    gpio_int_unmask_set(WAKEUP_IN);

    /*初始清除中断标志位*/
    gpio_int_state_clear(WAKEUP_IN);

    return 0;

fail2:
    gpio_free(WAKEUP_IN);
fail1:
    return -1;
}
/******************************************************************************
Function:       huawei_pm_init
Description:   Initialize huawei PM
Input:           None
Output:         None
Return:          None
Others:         None
******************************************************************************/
int huawei_pm_init(void)
{
    int error = 0;
    pm_status_s *pm_st = (pm_status_s *)(SRAM_MODULE_PM_ADDR);
    
    MDRV_VER_SOLUTION_TYPE module_type = MDRV_VER_TYPE_M2M;
    module_type = mdrv_ver_get_solution_type();
    
    PRINT_ACORE_LOG(g_PmAcorePrintOnOff,"begin module_pm init... \n");

    /*初始化时对共享内存值进行初值处理*/
    pm_st->pin_status = PIN_SUSPEND_STATUS;
    pm_st->usb_status = 0;


    nv_huawei_sleepcfg_type Huawei_sleep_cfg = {0};

    if( BSP_ERROR == huawei_pm_bsp_ifc_init())
    {
        goto fail1;
    }

    INIT_DELAYED_WORK(&usb_resume_acore_work, 
                        huawei_pm_usb_resume_acore_cb);
    INIT_DELAYED_WORK(&usb_resume_ccore_work, 
                        huawei_pm_usb_resume_ccore_cb);
    
    INIT_DELAYED_WORK(&g_huawei_pm_status.pin_timer.work, 
                        huawei_pm_request_wakeup_pin_cb);
    
    INIT_DELAYED_WORK(&shake_timeout_work, 
                        huawei_pm_remote_wakeup_timerout_cb);

    /* 防止输出远程唤醒信号1s高电平过程中系统睡眠 */
    wake_lock_init(&remote_wakeup_pin_lock, WAKE_LOCK_SUSPEND, "pm_remotepin");
    /* 用于响应wakeup_in唤醒睡眠单板系统*/
    wake_lock_init(&wakeup_in_lock, WAKE_LOCK_SUSPEND, "pm_wakeupin");
    /* 防止发送pm buffer缓存数据过程中系统睡眠*/
    wake_lock_init(&flush_cache_lock, WAKE_LOCK_SUSPEND, "pm_flush_cache");
    
    /* 通过NV50467获取远程唤醒握手超时的时间 */
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_SLEEP_CFG_FLAG, 
                           &Huawei_sleep_cfg, sizeof(Huawei_sleep_cfg)))
    {
        PRINT_ACORE_LOG(PM_MSG_PRINT_ON, "read sleepcfg NV fail !\n");
    }
    else
    {
        remote_wakeup_shake_time = \
            Huawei_sleep_cfg.huawei_sleep_cfg[0].sleepcfg_value;
    }

    /*config the WAKEUP_OUT GPIO*/
    error = huawei_pm_wakeupout_gpio_init();
    if ( error )
    {
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff,"wakeup_out init fail  \n");
        goto fail1;
    }

    /* CE 没有Sleep_status/WAKEUP_IN管教 */
    if(MDRV_VER_TYPE_CE == module_type)
    {
        PRINT_ACORE_LOG(PM_MSG_PRINT_ON,"CE no Sleep_status and WAKEUP_IN pin !! \n");
    }
    else
    {
        /*config the Sleep_status GPIO*/
        error = huawei_pm_sleepstatus_gpio_init();
        if(error)
        {
            PRINT_ACORE_LOG(g_PmAcorePrintOnOff,"sleep_status init fail  \n");
            goto fail1;
        }

        /*config the WAKEUP_IN GPIO*/
        error = huawei_pm_wakeupin_gpio_init();
        if ( error )
        {
            PRINT_ACORE_LOG(g_PmAcorePrintOnOff,"wakeup_in init fail  \n");
            goto fail1;
        }
    }
    
    huawei_pm_remote_wakeupcfg_init();

#if (FEATURE_ON == MBB_USB)
    mdrv_hw_usb_register_cb(HWUSBCB_PM_REMOTE_WAKEUP_CTRL,huawei_pm_usb_s34_state_change_handle);
#endif

    return 0;
fail1:
    return -1;
}

module_init(huawei_pm_init);


