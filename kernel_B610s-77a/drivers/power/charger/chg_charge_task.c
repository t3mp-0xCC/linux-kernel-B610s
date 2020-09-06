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



/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

//包含共享内存头文件，由移植人员包含
#include "chg_config.h"
#if (MBB_CHG_COULOMETER == FEATURE_ON)
#include "bsp_coul.h"
#endif
#if (FEATURE_ON == MBB_DLOAD_SDUP)
#include <bsp_sram.h>
#endif
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern int g_timeout_to_poweroff_reason;
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
 /*快充时间统计*/
int chg_time_in_second = 0;
/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MS_ONE_SECOND   (1000)
#define SECOND_ONE_MIN  (60)
#if (MBB_CHG_COULOMETER == FEATURE_ON)
#define POLL_SOC_CYCLE    (SLOW_POLL_CYCLE/FAST_POLL_CYCLE)
#endif/*MBB_CHG_COULOMETER == FEATURE_ON*/


void chg_charge_time_static (void)
{
    chg_stm_state_type tmp_sta = chg_stm_get_cur_state();
    int ret_val = chg_poll_timer_get();

    if ( -1 == ret_val )
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_TASK:Chg poll time NOT correct!!!\r\n");
        return;
    }
    else
    {
        ret_val /= MS_ONE_SECOND; /*由毫秒转换成秒*/

        /*打印定时时间*/
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_TASK:Chg poll time is %d s!!!\r\n", ret_val);
    }

    if ( -1 != chg_time_in_second )
    {
        //如果当前处于快充/高温充电/高压快充状态, 则累计充电时间
        if((CHG_STM_FAST_CHARGE_ST == tmp_sta) \
            || (CHG_STM_WARMCHG_ST == tmp_sta)
            || (CHG_STM_HVDCP_CHARGE_ST == tmp_sta))
        {
            chg_time_in_second += ret_val;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_TASK:charging last %d minute and %d second!!!\r\n", \
                (chg_time_in_second / SECOND_ONE_MIN),  (chg_time_in_second % SECOND_ONE_MIN));
        }
        else if ( CHG_STM_MAINT_ST == tmp_sta )
        {
            chg_time_in_second = -1;
            chg_print_level_message(CHG_MSG_ERR, "CHG_TASK:Maint charge happen, static will STOP!!!\r\n");
        }
        else
        {
            //for lint
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_TASK:maint charge has happened clean the chg_time_in_second!\r\n");
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_TASK:chg_charge_time_static over!\r\n");
}


int32_t  chg_main_task( void  *task_para )
{
    uint32_t event_recv = 0;
    int32_t  ulResult2;
#if (MBB_CHG_COULOMETER == FEATURE_ON)
    uint32_t poll_count = 0;
#endif/*MBB_CHG_COULOMETER == FEATURE_ON*/

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_TASK:chg_main_task->frist init begin!!!\r\n");

#if (MBB_CHG_COULOMETER == FEATURE_ON)
    /*进行一次库仑计强制校准*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_TASK:COUL_CAIL_ON");
    bsp_coul_cail_on();
    msleep(COUL_READY_DELAY_MS);
    /*update the SOC*/
    chg_poll_batt_soc();
#endif/*MBB_CHG_COULOMETER == FEATURE_ON*/
    /* 做一次电池状态的轮询查询**/
    chg_poll_bat_level();

    /*防止过温条件下，插入充电器/USB后，2s左右的充电过程*/
    chg_poll_batt_temp();
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_TASK:Init chg_poll_timer 2S!!!\r\n");
    /*初始启动两秒快轮询*/
    chg_poll_timer_set( FAST_POLL_CYCLE );

#if (MBB_CHG_EXTCHG == FEATURE_ON)
    chg_extchg_config_data_init();
#endif /*MBB_CHG_EXTCHG == FEATURE_ON*/

    ulResult2 = chg_stm_init();
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_TASK:chg_main_task->frist init finish!!!\r\n");
    if(CHG_OK != ulResult2)
    {
        //打印初始化失败log
        return -1;
    }

    while(FALSE == this_kthread_should_stop())
    {
#ifdef MBB_LLT_STUB
        return 0;
#endif
        /*等待事件,任务会被调度*/
        if ( -1 == chg_main_wait_for_msg( &event_recv ) )
        {
            continue;
        }

        /*查看接收信号是否有效*/
        if ( CHG_ALL_EVENT & event_recv )
        {
            /*电源拔除事件上报*/
            if ( CHG_CHARGER_OUT_EVENT & event_recv )
            {
                event_recv &= ( ~CHG_CHARGER_OUT_EVENT );
                chg_print_level_message(CHG_MSG_ERR, "CHG_TASK:chg_main_task->charger_output!!!\r\n ");

                if(is_chg_charger_removed())
                {
                    chg_charger_remove_proc(chg_stm_get_chgr_type());
                }
                else
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:charger is online,do nothing!\r\n");
                }
                chg_print_level_message(CHG_MSG_ERR, "CHG_TASK:chg_main_task->charger_output event process over!\r\n ");
            }

            /*电源插入事件上报*/
            if (CHG_CHARGER_IN_EVENT & event_recv )
            {
                event_recv &= ( ~CHG_CHARGER_IN_EVENT );
                chg_print_level_message(CHG_MSG_ERR, "CHG_TASK:chg_main_task->charger_input!\r\n ");

                /*If chgr input event received during start-up phase, doNOT switch to
                  transit state again, since we have done already in chg_stm_init.*/
                if(is_chg_charger_removed())
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:charger is not online,do nothing!\r\n");
                }
                else
                {
                    chg_charger_insert_proc(chg_stm_get_chgr_type());
                }
                chg_print_level_message(CHG_MSG_ERR, "CHG_TASK:chg_main_task->charger_input event process over!\r\n ");
            }

            /*电源类型检测事件*/
            if( CHG_CHGR_TYPE_CHECKING_EVENT & event_recv )
            {
                event_recv &= ( ~CHG_CHGR_TYPE_CHECKING_EVENT );
                chg_print_level_message( CHG_MSG_INFO,"CHG_TASK:chg_main_task->CHG_CHGR_TYPE_CHECKING_EVENT!\r\n ");

                /*Call STM try to check chgr type and update hw parameters.*/
                chg_check_and_update_hw_param_per_chgr_type();
                chg_print_level_message(CHG_MSG_INFO, "CHG_TASK:chg_main_task->CHGR_TYPE_CHECK_EVENT process over!\r\n ");
            }

            /* 充电的轮询定时器事件上报*/
            if ( CHG_TIMER_EVENT & event_recv )
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_TASK:chg_main_task->CHG_TIMER_EVENT!\r\n");
                event_recv &= ( ~CHG_TIMER_EVENT );
                /*Feed the WDT of charge IC.*/
                if (FALSE == chg_set_tmr_rst())
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_TASK:Feed IC WDT error!\r\n");
                }
#if (MBB_CHG_COULOMETER == FEATURE_ON)
                if(FAST_POLL_CYCLE == chg_poll_timer_get())
                {
                    poll_count++;
                }
                else if(SLOW_POLL_CYCLE == chg_poll_timer_get())
                {
                    poll_count = POLL_SOC_CYCLE;
                }
                else
                {
                    poll_count = POLL_SOC_CYCLE;
                }
                if(POLL_SOC_CYCLE == poll_count)
                {
                    /*update the SOC*/
                    chg_poll_batt_soc();
                    poll_count = 0;
                }
#endif/*MBB_CHG_COULOMETER == FEATURE_ON*/
                /* 定时电池电压轮询查询**/
                chg_poll_bat_level();
                /* 定时电池温度轮询查询**/
                chg_poll_batt_temp();
                /*更新power supply 各节点的状态信息*/
                chg_update_power_suply_info();
                /*Call STM periodic checking function.*/
                chg_stm_periodic_checking_func();
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
                /*update charge state,if change,notify coul*/
                chg_poll_batt_charging_state_for_coul();
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/

                /*可维可测:统计快充时间*/
                chg_charge_time_static();
                chg_print_test_view_info();
                chg_print_level_message(CHG_MSG_INFO, "CHG_TASK:chg_main_task->CHG_TIMER_EVENT process over!\r\n");
            }
            /*  高低温应用关机异常充电模块强制关机事件*/
            if( CHG_TIMEROUT_TO_POWEROFF_EVENT & event_recv )
            {
                event_recv &= ( ~CHG_TIMEROUT_TO_POWEROFF_EVENT );
                chg_print_level_message(CHG_MSG_INFO, "CHG_TASK:chg_main_task->CHG_TIMEROUT_TO_POWEROFF_EVENT!!!\r\n ");
                chg_set_power_off(g_timeout_to_poweroff_reason);
            }

            /* 发生定时器时间切换问题*/
            if( CHG_POLL_TIMER_CHANGE & event_recv )
            {
                event_recv &= ( ~CHG_POLL_TIMER_CHANGE );
                chg_print_level_message(CHG_MSG_INFO, "CHG_TASK:chg_main_task->CHG_POLL_TIMER_CHANGE!!!\r\n ");
            }
        }
    }
    return 0;
}


void chg_basic_init( void )
{
    static int32_t s_chg_basic_init_flag = FALSE;
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_TASK:chg_basic_init \n ");

    if( FALSE == s_chg_basic_init_flag )
    {
        s_chg_basic_init_flag = TRUE;

/***************Note:平台相关代码，根据平台按需要添加，有的平台如V7R1需要
              移植人员根据需要，添加或者移除下边函数调用***************************/
        /* 核间通信初始化*/
        //BSP_CHGC_Init();

        /* 充电芯片初始化*/
        chg_chip_init();
#if (MBB_CHG_EXTCHG == FEATURE_ON)
        extchg_gpio_init();
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/

#if (MBB_CHG_WIRELESS == FEATURE_ON)
        wireless_gpio_init();
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
    }
}


void chg_task_init(void)
{
#if (FEATURE_ON == MBB_DLOAD_SDUP)
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
#endif
    /*获取硬件版本号*/
    chg_get_hw_version_id();
    /*根据硬件版本号对充电硬件相关参数初始化*/
    chg_charge_paras_init();
    /*非升级模式才进行充电相关参数初始化、启动充电任务*/
    if(FALSE == is_in_update_mode())
    {
        if (TRUE == chg_is_powdown_charging() && FALSE == chg_is_charger_present())
        {
           chg_print_level_message(CHG_MSG_ERR,"CHG_TASK:POWER OFF FOR CHARGER REMOVE !\n ");
           //chg_set_power_off(DRV_SHUTDOWN_CHARGE_REMOVE);
        }
        chg_basic_init();
        load_ftm_mode_init();
        load_on_off_mode_parameter();
        /*从NV中读取电池格数，停复充相关的电压参数*/
        chg_batt_volt_init();
        /*根据电池ID更新使用电池对应的电池格数，停复充相关的电压参数*/
        chg_batt_volt_paras_init();
        chg_batt_temp_init();
        chg_batt_volt_calib_init();
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_TASK:task init success!\n");
        /*非工厂模式启动充电任务*/
        if(FALSE == chg_is_ftm_mode())
        {
            /*创建充电任务*/
            chg_task_create();
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_TASK:task creat success!\n");
        }
        /*工厂模式不启动充电任务*/
        else
        {
            //烧片版本修改限流为1200mA
            if(FALSE == chg_set_supply_limit(CHG_IINPUT_LIMIT_1200MA))
            {
                 chg_print_level_message(CHG_MSG_ERR,"chg_set_supply_limit:chg_set_supply_limit failed!\n");
            }

            chg_set_sys_batt_capacity(BATT_CAPACITY_FULL);
            chg_print_level_message(CHG_MSG_ERR,"CHG_TASK:chg task does not creat in FTM!\n");
        }
    }
    /*升级模式进进行电池电压校准初始化，不启动充电任务以防止影响升级稳定性*/
    else
    {
#if (FEATURE_ON == MBB_DLOAD_SDUP)
        if((smem_data != NULL) && (smem_data->smem_sd_upgrade == SMEM_SDUP_FLAG_NUM))
        {
            chg_batt_volt_init();
        }
#endif
        chg_batt_volt_calib_init();
        chg_print_level_message(CHG_MSG_INFO,"CHG_TASK:Dload mode do not creat chg task!\n");
        return;
    }
}

/***************Note:平台相关代码，根据平台按需要添加，有的平台如9x25 Linux系统需要
              移植人员根据需要，添加或者移除下边函数调用***************************/
static int __init charger_init(void)
{
    //charge init
    chg_task_init();
    return 0;
}
module_init(charger_init);

static void __exit charger_exit(void)
{
    //ready to add exit code
    return;
}
module_exit(charger_exit);

MODULE_DESCRIPTION("Charger driver");

MODULE_LICENSE("GPL v2");

