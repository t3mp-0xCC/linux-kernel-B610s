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
#include "chg_config.h"

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
#include <bsp_nvim.h>
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/

/*----------------------------------------------*
 * 宏                                 *
 *----------------------------------------------*/
#define TBAT_ADC_READ_TIMES   (3)

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
#if defined(BSP_CONFIG_BOARD_E5_DC04)
#if (MBB_CHG_EXTCHG == FEATURE_ON)
extern int32_t g_ui_choose_exchg_mode;
#endif
#endif

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*Keep ret-code and data of intercore calling.*/
static int32_t chg_intercore_reply_data = 0;
static int32_t chg_intercore_reply_ret  = CHG_ERROR;

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

static CHG_TCHRENABLE_TYPE g_chg_tchrenable_status =
{
        -1,
        -1
};
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/


int32_t app_get_battery_state(BATT_STATE_T *battery_state)
{
    boolean power_off_charging_flag = FALSE;
    chg_stm_state_type real_charger_state = CHG_STM_INIT_ST;

    if(NULL == battery_state)
    {
        return CHG_ERROR;
    }

    battery_state->battery_level = chg_get_batt_level();
    battery_state->batt_temp_state = chg_get_batt_temp_state();
    real_charger_state = chg_stm_get_cur_state();
    power_off_charging_flag = chg_is_powdown_charging();

    if(FALSE == power_off_charging_flag)
    {
        /*如果目前为非关机充电模式，是否充电根据实际状态
        显示**/
        if((CHG_STM_FAST_CHARGE_ST == real_charger_state) \
            || (CHG_STM_WARMCHG_ST == real_charger_state))
        {
            battery_state->charging_state = CHARGING_UP;
        }
        else
        {
            battery_state->charging_state = NO_CHARGING_UP;
        }
    }
    else
    {
        /* 目前进入关机充电后初始化状态也认为是充电
        模式此时的关机充电动画已经开启***/
        if((CHG_STM_FAST_CHARGE_ST == real_charger_state) \
           || (CHG_STM_TRANSIT_ST  == real_charger_state) \
           || (CHG_STM_WARMCHG_ST == real_charger_state))
        {
            battery_state->charging_state = CHARGING_DOWN;
        }
        else
        {
            battery_state->charging_state = NO_CHARGING_DOWN;
        }
    }

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_API: app charging_state =%d !!!\r\n", battery_state->charging_state);
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_API: app battery_level =%d !!!\r\n", battery_state->battery_level);
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_API: app batt_temp_state =%d !!!\r\n", battery_state->batt_temp_state);
    return CHG_OK;
}


#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)

int chg_tbat_read(ENUM_CHG_ATCM_TYPE atID, void *pItem)
{
    int32_t ret = 0;

    if(NULL == pItem)
    {
        chg_print_level_message(CHG_MSG_ERR,"invalid parameter!!\n");
        return CHG_ERROR;
    }
    switch(atID)
    {
        case CHG_AT_BATTERY_CHECK:
        {
            /*读取电池电量校准值*/
            VBAT_CALIBRATION_TYPE * ptr = (VBAT_CALIBRATION_TYPE *)pItem;
            VBAT_CALIBRATION_TYPE tbat_check;

            ret = bsp_nvm_read(NV_BATT_VOLT_CALI_I, &tbat_check, sizeof(VBAT_CALIBRATION_TYPE));
            if(0 == ret)
            {
                ptr->max_value = tbat_check.max_value;
                ptr->min_value = tbat_check.min_value;
            }
            else
            {
                chg_print_level_message(CHG_MSG_ERR,"read chg nv failed when read check value\n");
                return CHG_ERROR;
            }
        }
        break;
        case CHG_AT_BATTERY_LEVEL:
        {
            /*读取校准后的电池电压*/
            *(int32_t*)pItem = chg_get_batt_volt_value();
        }
        break;

        /*校准新增AT接口*/
        case CHG_AT_BATTERY_ADC:
        {
            /*读取电池未校准的电池电压*/
            *(int32_t*)pItem = chg_get_volt_from_adc( CHG_PARAMETER__BATTERY_VOLT );
        }
        break;
        /*获取电池电量*/
        case CHG_AT_BATTERY_CAPACITY:
        {
            /*读取电池电量，暂未实现*/
// *(int32_t*)pItem = chg_get_batt_volt_value();
        }
        break;

        default:
            return CHG_ERROR;
    }
    return  CHG_OK;
}


int chg_tbat_write(ENUM_CHG_ATCM_TYPE atID, void *pItem)
{
    /*设置电池电量校准值*/
    int32_t ret = 0;
    VBAT_CALIBRATION_TYPE * ptr = (VBAT_CALIBRATION_TYPE *)pItem;
    VBAT_CALIBRATION_TYPE tbat_org = {0};

    if(NULL == pItem)
    {
        chg_print_level_message(CHG_MSG_ERR,"invalid parameter!!\n");
        return CHG_ERROR;
    }

    switch(atID)
    {
        case CHG_AT_BATTERY_CHECK:
        {
            ret = bsp_nvm_read(NV_BATT_VOLT_CALI_I, &tbat_org, sizeof(VBAT_CALIBRATION_TYPE));
            if(ret)
            {
                chg_print_level_message(CHG_MSG_ERR,"read chg nv failed!!!\n");
                return CHG_ERROR;
            }

            if(TBAT_CHECK_INVALID == ptr->max_value)
            {
                ptr->max_value = tbat_org.max_value;
            }
            if(TBAT_CHECK_INVALID == ptr->min_value)
            {
                ptr->min_value = tbat_org.min_value;
            }

            ret = bsp_nvm_write(NV_BATT_VOLT_CALI_I, ptr, sizeof(VBAT_CALIBRATION_TYPE));
            if(ret)
            {
                chg_print_level_message(CHG_MSG_ERR,"write chg nv failed when set check value\n", 0, 0, 0);
                return CHG_ERROR;
            }


        }
        break;

        default:
            return CHG_ERROR;
    }
    return  CHG_OK;
}


int chg_tbat_charge_mode_set(ENUM_CHG_MODE_TYPE cmd, unsigned int arg)
{
    int ret = -1;
    boolean set_status = FALSE;

    switch(cmd)
    {
        /*PT工位，打开充电*/
        case CHG_AT_CHARGE_ENABLE:
        {
            //BQ24192不支持涓充和快充模式的设置

            //chg_set_hardware_parameter(&chg_pt_chgr_hw_paras[0]);
            set_status = chg_pt_mmi_test_proc();

        }
        break;
        /*关闭充电*/
        case CHG_AT_CHARGE_DISALBE:
        {
            set_status = chg_set_charge_enable(FALSE);
        }
        break;
        /*打开放电*/
        case CHG_AT_DISCHARGE_ENABLE:
        {
            set_status = chg_set_suspend_mode(TRUE);
        }
        break;
        /*关闭放电*/
        case CHG_AT_DISCHARGE_DISALBE:
        {
            set_status = chg_set_suspend_mode(FALSE);
        }
        break;
        /*打开补电*/
        case CHG_AT_START_CHARGE_SPLY:
        {
            ret = chg_tbat_chg_sply();
            /*return ret;*/
        }
        break;

        default:
            return CHG_ERROR;
    }
    if((TRUE == set_status) || (TBAT_SUPPLY_CURR_SUCCESS == ret))
    {
        g_chg_tchrenable_status.chg_state = cmd;
        g_chg_tchrenable_status.chg_mode = arg;
        return  CHG_OK;
    }
    else
    {
        return  CHG_ERROR;
    }
}


int chg_tbat_is_batt_status_ok(void)
{
    return chg_tbat_status_get();
}


int chg_tbat_get_charging_status(void)
{
    return chg_is_IC_charging();
}


int chg_tbat_get_charging_mode(void)
{
    chg_status_type  chg_stat = {0};
    int op_result = TRUE;

    op_result = chg_get_IC_status(&chg_stat);
    if (FALSE == op_result)
    {
        chg_print_level_message(CHG_MSG_ERR,"get IC status failed!!\n");
        return -1;
    }

    return chg_stat.chg_chrg_stat;
}


int chg_tbat_get_discharging_status(void)
{
    return chg_get_suspend_status();
}
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/


int chg_tbat_sfeature_inquiry_extchg(void)
{
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    return TRUE;
#else
    return FALSE;
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/
}


int chg_tbat_sfeature_inquiry_wireless(void)
{
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    return TRUE;
#else
    return FALSE;
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
}


int chg_tbat_sfeature_inquiry_battery(void)
{
    return -1;
}


int chg_MNTN_get_batt_state(void)
{
    /*需要根据具体产品来确定是否是可拆卸电池*/
#if (MBB_CHG_BAT_KNOCK_DOWN == FEATURE_ON)
    /*可拆卸电池*/
    return KNOCK_DOWN_BATT_MODE;
#elif (MBB_CHG_BAT_KNOCK_DOWN == FEATURE_OFF)
    return FIX_BATT_MODE;
#endif/*MBB_CHG_BAT_KNOCK_DOWN == FEATURE_OFF*/
}


int chg_wireless_mmi_test(void)
{
    return -1;
}


int chg_extchg_mmi_test(void)
{
    return -1;
}


int chg_tbat_get_tchrenable_status(CHG_TCHRENABLE_TYPE *tchrenable_state)
{

    if(NULL == tchrenable_state)
    {
        return -1;
    }

    *tchrenable_state = g_chg_tchrenable_status;

    return 0;

}


int chg_getCbcState(unsigned char  *pucBcs ,unsigned char   *pucBcl)
{
    return 0;
}


int chg_get_coulometer_support(void)
{
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    return YES;
#else
    return NO;
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
}


int chg_usb_state_callback(int STA, int Msg_Error)
{
    chg_intercore_reply_data = (int32_t)STA;
    chg_intercore_reply_ret =  (int32_t)Msg_Error;

    if (CHG_OK == chg_intercore_reply_ret)
    {
        return STA;
    }
    else
    {
        return Msg_Error;
    }
}
