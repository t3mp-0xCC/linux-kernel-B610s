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
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "chg_config.h"
#if ( FEATURE_ON == MBB_MLOG )
#include <linux/mlog_lib.h>
#endif
#include "bsp_coul.h"
#include "platform/hisi/chg_chip_platform.h"

#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
#include <bsp_hkadc.h>
#endif

#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
#if (FEATURE_ON == MBB_CHG_BQ25892)
#include "drv/fsa9688/fsa9688.h"
#endif
#endif
#ifndef CHG_STUB
#include <linux/usb/usb_interface_external.h>
#else
enum chg_current
{
    CHG_CURRENT_HS = 0,     //usb2.0 for 500mA
    CHG_CURRENT_SS ,        //usb3.0 for 900mA
    CHG_CURRENT_NO,         //invalid usb
};
static int usb_speed_work_mode(void)  // ������ usb_interface_external.h ���ظ����˴���� static
{
    return 0;
}

#define USB_OTG_CONNECT_DP_DM               (0x0001)
#define USB_OTG_DISCONNECT_DP_DM            (0x0002)  //ֱ������������HS_ID
#define USB_OTG_ID_PULL_OUT                 (0x0003)  //ֱ������������HS_ID
#define USB_OTG_DISABLE_ID_IRQ              (0x0004)
#define USB_OTG_ENABLE_ID_IRQ               (0x0005)
#endif/*CHG_STUB*/

/*USB�±�*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
#define TEMP_INITIAL_VALUE                  (0xFFFF)   /*�¶ȳ�ʼ��ֵ*/
#define USB_TEMP_DETECT_COUNT               (4)        /*USB�¶�����������+1*/
extern struct wake_lock chg_usb_temp_wake_lock;        /*USB�¶��쳣״̬��������*/
#endif

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
extern struct chargeIC_chip *g_chip;
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
extern struct softtimer_list g_chg_sleep_timer;
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/
/*Ӳ����Ʒ�汾��*/
extern uint32_t  g_hardware_version_id;
/*��ͬ�����������*/
extern const chg_hw_param_t chg_std_1A_chgr_hw_paras_def[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_std_2A_chgr_hw_paras_def[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_std_2A_chgr_hw_paras_4800mAh_batt[CHG_STM_MAX_ST];
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
extern const chg_hw_param_t chg_5v_hvdcp_chgr_hw_paras_def[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_9v_hvdcp_chgr_hw_paras_def[CHG_STM_MAX_ST];
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
extern const chg_hw_param_t chg_usb_chgr_hw_paras_def[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_usb_chgr_hw_paras_4800mAh_batt[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_usb3_chgr_hw_paras_def[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_usb3_chgr_hw_paras_4800mAh_batt[CHG_STM_MAX_ST];
extern const chg_hw_param_t chg_weak_chgr_hw_paras_def[CHG_STM_MAX_ST];
#if (MBB_CHG_WIRELESS == FEATURE_ON)
extern const chg_hw_param_t chg_wireless_chgr_hw_paras_def[CHG_STM_MAX_ST];
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
extern const chg_hw_param_t chg_wireless_poweroff_warmchg_paras;
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
#endif /* MBB_CHG_WIRELESS == FEATURE_ON */
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
extern int battery_monitor_blocking_notifier_call_chain(unsigned long val, unsigned long v);
#if defined(BSP_CONFIG_BOARD_E5_DC04)
#if (FEATURE_ON == MBB_CHG_EXTCHG)
extern void pcie_resume_before_fastoff(void);
#endif
#endif

struct chg_hardware_data *g_chg_hardware_data = NULL;

struct chg_batt_data *g_chg_batt_data = NULL;

/*��ͨ���*/
static struct chg_batt_data chg_batt_data_default = {
        .is_20pct_calib   = FALSE,
        .batt_id                = CHG_BATT_ID_DEF,
        .chg_batt_volt_paras    = {3450,3450,4220,4240,4100,4050,3550,3550,3610,3670,3770,4150,4100},/*batt data*/
};

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
static struct chg_batt_data chg_batt_data_xingwangda_1500mah = {
        .id_voltage_min = 1937,//batt id volt
        .id_voltage_max = 2187,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                = CHG_BATT_ID_XINGWANGDA_1500MAH,
        .chg_batt_volt_paras    = {3450,3450,4370,4390,4150,4030,3638,3638,3721,3805,4030,4250,4200},/*batt data*/

};

/*
static struct chg_batt_data chg_batt_data_feimaotui_1500mah = {
        .id_voltage_min = 887,  //batt id volt
        .id_voltage_max = 1137, //batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                = CHG_BATT_ID_FEIMAOTUI_1500MAH,
        .chg_batt_volt_paras    = {3450,3450,4370,4390,4150,4030,3620,3620,3708,3779,4030,4250,4200},//batt data


};
*/

static struct chg_batt_data chg_batt_data_lishen_1500mah = {
        .id_voltage_min = 102,//batt id volt
        .id_voltage_max = 352,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                = CHG_BATT_ID_LISHEN_1500MAH,
        .chg_batt_volt_paras    = {3450,3450,4370,4390,4150,4030,3621,3621,3709,3798,4030,4250,4200},/*batt data*/

};

/*��ë��2300mah��ѹ���*/
/*
static struct chg_batt_data chg_batt_data_feimaotui_2300mah = {
        .id_voltage_min = 852, //batt id volt
        .id_voltage_max = 1032,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI_2300MAH,
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4250,4000,3600,3600,3690,3803,3948,4280,4200},//batt data
};
*/

/*������3000mah��ѹ���*/
static struct chg_batt_data chg_batt_data_xinwangda_3000mah = {
        .id_voltage_min = 800, //batt id volt
        .id_voltage_max = 1100,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_XINWANGDA_3000MAH,
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4250,4050,3630,3630,3720,3800,4030,4280,4200},/*batt data*/
};

/*��ë��4800mah��ѹ���*/
static struct chg_batt_data chg_batt_data_feimaotui_4800mah = {
        .id_voltage_min = 220, //batt id volt
        .id_voltage_max = 420,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI_4800MAH,
        .chg_batt_volt_paras        = {3300,3300,4370,4390,4250,4050,3604,3450,3716,3824,4022,4280,4200},/*batt data*/
};

/*������4800mah��ѹ���*/
static struct chg_batt_data chg_batt_data_xinwangda_4800mah = {
        .id_voltage_min = 1530, //batt id volt
        .id_voltage_max = 1730,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_XINGWANGDA_4800MAH,
        .chg_batt_volt_paras        = {3300,3300,4370,4390,4250,4050,3604,3450,3716,3824,4022,4280,4200},/*batt data*/
};

static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
#if defined(BSP_CONFIG_BOARD_E5_SB03)
    &chg_batt_data_xinwangda_3000mah,
#elif defined(BSP_CONFIG_BOARD_E5_DC04)
    &chg_batt_data_feimaotui_4800mah,
    &chg_batt_data_xinwangda_4800mah,
#else
    //&chg_batt_data_feimaotui_2300mah,
    &chg_batt_data_xingwangda_1500mah,
    //&chg_batt_data_feimaotui_1500mah,
    &chg_batt_data_lishen_1500mah,
#endif
};

#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)

/*��ë��2300mah��ѹ���*/
static struct chg_batt_data chg_batt_data_feimaotui_2300mah = {
        .id_voltage_min = 852, //batt id volt
        .id_voltage_max = 1032,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI_2300MAH,
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4050,3600,3600,3690,3803,3948,4280,4200},/*batt data*/
};

/*������1900mah��ѹ���*/
static struct chg_batt_data chg_batt_data_xingwangda_1900mah = {
        .id_voltage_min = 697,//batt id volt
        .id_voltage_max = 878,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_XINGWANGDA_1900MAH,
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4050,3570,3570,3638,3721,3870,4280,4200},/*batt data*/
};
/*��ë��1900mah��ѹ���*/
static struct chg_batt_data chg_batt_data_feimaotui_1900mah = {
        .id_voltage_min = 187,//batt id volt
        .id_voltage_max = 367,//batt id volt
        .is_20pct_calib   = FALSE,
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI_1900MAH,
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4050,3563,3563,3620,3708,3839,4280,4200},/*batt data*/
};

static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_feimaotui_2300mah,
    &chg_batt_data_xingwangda_1900mah,
    &chg_batt_data_feimaotui_1900mah,
};

#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/

#define BATTERY_DATA_ARRY_SIZE sizeof(chg_batt_data_array) / sizeof(chg_batt_data_array[0])

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
/*0:-Charge Transition State TRIO function.*/
static void chg_transit_state_entry_func(void);
static void chg_transit_state_period_func(void);
static void chg_transit_state_exit_func(void);

/*1:-Fast Charge State TRIO function.*/
static void chg_fastchg_state_entry_func(void);
static void chg_fastchg_state_period_func(void);
static void chg_fastchg_state_exit_func(void);

/*2:-Battery Maintenance State TRIO function.*/
static void chg_maint_state_entry_func(void);
static void chg_maint_state_period_func(void);
static void chg_maint_state_exit_func(void);

/*3:-Invalid Charge Temperature State TRIO function.*/
static void chg_invalid_chg_temp_state_entry_func(void);
static void chg_invalid_chg_temp_state_period_func(void);
static void chg_invalid_chg_temp_state_exit_func(void);

/*4:-Battery Only State TRIO function.*/
static void chg_batt_only_state_entry_func(void);
static void chg_batt_only_state_period_func(void);
static void chg_batt_only_state_exit_func(void);

static void chg_stm_set_cur_state(chg_stm_state_type new_state);

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
/*5:-Warmchg State TRIO function.*/
static void chg_warmchg_state_entry_func(void);
static void chg_warmchg_state_period_func(void);
static void chg_warmchg_state_exit_func(void);
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
/*6:-HVDCP Charge State TRIO function.*/
static void chg_hvdcpchg_state_entry_func(void);
static void chg_hvdcpchg_state_period_func(void);
static void chg_hvdcpchg_state_exit_func(void);
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
void chg_sleep_batt_check_timer(void);
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/
/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
static uint32_t g_real_factory_mode;                  //�Ƿ񹤳�ģʽ
static boolean chg_batt_high_temp_58_flag = FALSE;    //����58�ȱ�־
static boolean chg_batt_low_battery_flag = FALSE;     //�͵��־
static boolean chg_batt_condition_error_flag = FALSE; //����쳣��־
static boolean chg_limit_supply_current_flag = FALSE; //�±�������ʶ
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
/*��ر�����ʶ*/
static boolean chg_battery_protect_flag = FALSE;
/*����ǰ��ѹ������ʶ*/
static boolean g_chg_over_temp_volt_protect_flag = FALSE;
/*����ͣ���Դ��ʱ����λ״̬��ʶ*/
static boolean g_chg_longtime_nocharge_protect_flag = FALSE;
/*������� ����ͣ���ҳ������ʱ����λ״̬ǰ���¸�������*/
static unsigned int g_batt_normal_temp_recherge_threshold = 0;
/*������ͣ�����ͣ��������ʱ����λ�����¸�������Ϊ4.0V��־*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
#define BATT_EXPAND_CAPACITY_RECHG 65
static boolean g_chg_batt_expand_change_normal_rechg_flag = FALSE;
#endif
#endif

/*USB�±��ݶ�ֵ��ͨ���ӿڿɸ��ģ�Ӳ��������ɺ�ɾ��*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
static int32_t chg_test_usb_temp_limit     = 80;
static int32_t chg_test_usb_temp_rusume    = 60;
#define CHG_USB_TEMP_LIMIT      chg_test_usb_temp_limit     /*USB�±����ƴ�������*/
#define CHG_USB_TEMP_RESUME     chg_test_usb_temp_rusume    /*USB�¶Ȼ��ƻָ�����*/
#endif

/*��Ƭ�汾�϶������⿪ʼ�ı�ʶλ*/
#if((FEATURE_ON == MBB_FACTORY)\
     && (FEATURE_ON == MBB_CHG_COULOMETER)\
     && (FEATURE_ON == MBB_CHG_EXTCHG ))
static int32_t g_extchg_start = 0;
#endif

/*��ŵ����NV*/
CHG_BATTERY_OVER_TEMP_PROTECT_NV   g_chgBattOverTempPeotect;  //NV50016
CHG_BATTERY_LOW_TEMP_PROTECT_NV    g_chgBattLowTempPeotect;   //NV52005
CHG_SHUTOFF_TEMP_PROTECT_NV_TYPE   g_chgShutOffTempProtect;   //NV50385
CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE   g_chgBattVoltProtect;      //NV50386
POWERUP_MODE_TYPE                  g_real_powerup_mode_value; //NV50364

/***************************begin NV50016 ���ݺ궨�� ************************/
/*SHUTOFF ���¹ػ�ʹ�ܿ���*/
#define SHUTOFF_OVER_TEMP_PROTECT_ENABLE g_chgBattOverTempPeotect.ulIsEnable
/*SHUTOFF ���¹ػ��¶�����*/
#define SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD g_chgBattOverTempPeotect.lCloseAdcThreshold
/*SHUTOFF ���¹ػ��¶�����*/
#define SHUTOFF_OVER_TEMP_SHUTOFF_CHECK_TIMES g_chgBattOverTempPeotect.ulTempOverCount
/***************************end   NV50016 ���ݺ궨�� ************************/

/***************************begin NV52005 ���ݺ궨�� ************************/
/*SHUTOFF ���¹ػ�ʹ�ܿ���*/
#define SHUTOFF_LOW_TEMP_PROTECT_ENABLE g_chgBattLowTempPeotect.ulIsEnable
/*SHUTOFF ���¹ػ��¶�����*/
#define SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD g_chgBattLowTempPeotect.lCloseAdcThreshold
/*SHUTOFF ���¹ػ��¶�����*/
#define SHUTOFF_LOW_TEMP_SHUTOFF_CHECK_TIMES g_chgBattLowTempPeotect.ulTempLowCount
/***************************end  NV52005 ���ݺ궨�� ************************/


/************************************begin NV50385 ���ݺ궨�� *******************************/
/*CHG ����±���ʹ�ܿ���*/
#define CHG_TEMP_PROTECT_ENABLE g_chgShutOffTempProtect.ulChargeIsEnable
/*CHG ������/���±�������*/
#define CHG_OVER_TEMP_STOP_THRESHOLD g_chgShutOffTempProtect.overTempchgStopThreshold
#define CHG_LOW_TEMP_STOP_THRESHOLD g_chgShutOffTempProtect.lowTempChgStopThreshold
/*CHG ������/���»ָ��¶�����*/
#define CHG_OVER_TEMP_RESUME_THRESHOLD g_chgShutOffTempProtect.overTempChgResumeThreshold
#define CHG_LOW_TEMP_RESUME_THRESHOLD g_chgShutOffTempProtect.lowTempChgResumeThreshold
/*CHG ���ͣ����ѯ����*/
#define CHG_TEMP_PROTECT_CHECK_TIMES g_chgShutOffTempProtect.chgTempProtectCheckTimes
/*CHG ��縴����ѯ����*/
#define CHG_TEMP_RESUME_CHECK_TIMES g_chgShutOffTempProtect.chgTempResumeCheckTimes
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
/*������³�����¶�����*/
#define CHG_WARM_CHARGE_ENTER_THRESHOLD g_chgShutOffTempProtect.subTempChgLimitCurrentThreshold
/*�ɸ��³��ָ������³�����¶�����*/
#define CHG_WARM_CHARGE_EXIT_THRESHOLD g_chgShutOffTempProtect.exitWarmChgToNormalChgThreshold
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
/**********************************end NV50385 ���ݺ궨�� *******************************/

/************************************begin NV50386 ���ݺ궨�� *******************************/
/*������ѹ����*/
#define BATT_VOLT_POWER_ON_THR g_chgBattVoltProtect.battVoltPowerOnThreshold
/*�ػ���ѹ����*/
#define BATT_VOLT_POWER_OFF_THR g_chgBattVoltProtect.battVoltPowerOffThreshold
/*����ѹ��������(ƽ��ֵ)*/
#define BATT_CHG_OVER_VOLT_PROTECT_THR g_chgBattVoltProtect.battOverVoltProtectThreshold
/*����ѹ��������(���βɼ�ֵ)*/
#define BATT_CHG_OVER_VOLT_PROTECT_ONE_THR g_chgBattVoltProtect.battOverVoltProtectOneThreshold
/*��ʱͣ���ж�����*/
#define BATT_CHG_TEMP_MAINT_THR g_chgBattVoltProtect.battChgTempMaintThreshold
/*���³����θ�������*/
#define BATT_HIGH_TEMP_RECHARGE_THR g_chgBattVoltProtect.battChgRechargeThreshold

/*�͵���������*/
#define BATT_VOLT_LEVELLOW_MAX g_chgBattVoltProtect.VbatLevelLow_MAX
/*0���ѹ��������*/
#define BATT_VOLT_LEVEL0_MAX g_chgBattVoltProtect.VbatLevel0_MAX
/*1���ѹ��������*/
#define BATT_VOLT_LEVEL1_MAX g_chgBattVoltProtect.VbatLevel1_MAX
/*2���ѹ��������*/
#define BATT_VOLT_LEVEL2_MAX g_chgBattVoltProtect.VbatLevel2_MAX
/*3���ѹ�������� */
#define BATT_VOLT_LEVEL3_MAX g_chgBattVoltProtect.VbatLevel3_MAX
/*�жϲ��������Ƿ��������*/
#define BATT_INSERT_CHARGE_THR g_chgBattVoltProtect.battInsertChargeThreshold
/*���³��ĸ�������*/
#define BATT_NORMAL_TEMP_RECHARGE_THR g_chgBattVoltProtect.battNormalTempRechergeThreshold

/************************************end NV50386 ���ݺ궨�� *******************************/

/*��ǰ�Ƿ�Ϊ����ģʽ: true:����ģʽ false :�ǹ���ģʽ*/
static boolean chg_current_ftm_mode = FALSE;
/*��¼����±�����־,��ʼ��Ϊ�¶�����**/
static uint32_t chg_temp_protect_flag = FALSE;
static uint32_t fact_release_flag = FALSE;
/*��¼����¶�״̬*/
TEMP_EVENT chg_batt_temp_state = TEMP_BATT_NORMAL;

/*��¼ʵ�ʵĳ��״̬��Ϣ**/
CHG_PROCESS_INFO chg_real_info = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0, 0, BATT_LEVEL_0, 0,0,0xFFFF,0xFFFF};
/*��¼USB�˿��¶ȱ�����Ϣ*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
USB_TEMP_PROTECT_INFO chg_usb_temp_info = {TEMP_INITIAL_VALUE,FALSE,FALSE};
#endif

static chg_stm_type chg_stm_state_machine[CHG_STM_MAX_ST + 1] =
{
    /* 0 Transition State, which is also the default after system boot-up.*/
    {
        (chg_stm_func_type)chg_transit_state_entry_func,
        (chg_stm_func_type)chg_transit_state_period_func,
        (chg_stm_func_type)chg_transit_state_exit_func,
    },

    /* 1 Fast Charge State, battery is charged during this state.*/
    {
        (chg_stm_func_type)chg_fastchg_state_entry_func,
        (chg_stm_func_type)chg_fastchg_state_period_func,
        (chg_stm_func_type)chg_fastchg_state_exit_func,
    },

    /* 2 Maintenance State, battery has been charged to full, system was supplied by
         external charger preferentially.*/
    {
        (chg_stm_func_type)chg_maint_state_entry_func,
        (chg_stm_func_type)chg_maint_state_period_func,
        (chg_stm_func_type)chg_maint_state_exit_func,
    },

    /* 3 Invalid Charge Temperature State, external charger is present, while battery
         is too hot/cold to charge.*/
    {
        (chg_stm_func_type)chg_invalid_chg_temp_state_entry_func,
        (chg_stm_func_type)chg_invalid_chg_temp_state_period_func,
        (chg_stm_func_type)chg_invalid_chg_temp_state_exit_func,
    },

    /* 4 Battery Only State, external charger is absent, or removed, system was supplied
         by battery.*/
    {
        (chg_stm_func_type)chg_batt_only_state_entry_func,
        (chg_stm_func_type)chg_batt_only_state_period_func,
        (chg_stm_func_type)chg_batt_only_state_exit_func,
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /* 5 warm chg State, battery is charged during this state when the battery temp is in
      warm chg area*/
    {
        (chg_stm_func_type)chg_warmchg_state_entry_func,
        (chg_stm_func_type)chg_warmchg_state_period_func,
        (chg_stm_func_type)chg_warmchg_state_exit_func,
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    /*6:-HVDCP Charge State TRIO function.*/
    {
        (chg_stm_func_type)chg_hvdcpchg_state_entry_func,
        (chg_stm_func_type)chg_hvdcpchg_state_period_func,
        (chg_stm_func_type)chg_hvdcpchg_state_exit_func,
    },
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    {
        NULL,NULL,NULL
    }
};
/*----------------------------------------------*
 * ȫ�ֱ���                                   *
 *----------------------------------------------*/

chg_stm_state_info_t chg_stm_state_info = {0,0,0,0,0,0,0,CHG_BAT_ONLY_MODE,0,0,0,0
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
,0
#endif
#if ( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
,0
#endif
};


/* ���ʹ�ܱ�־�ṩ��USB��������������1:ʹ�ܳ�� 0:û��ʹ��*/
int chg_en_flag = 0;
unsigned long chg_en_timestamp = 0;
#define CHG_EN_TIME_SLOT       (50)   //���ʹ��ʱ��
int g_default_dpm_volt = 0;           //DPMֵ
int g_timeout_to_poweroff_reason = 0; //����쳣��ʱ�ػ�ԭ��

static const chg_hw_param_t* chg_std_chgr_hw_paras = NULL;
static const chg_hw_param_t* chg_usb_chgr_hw_paras = NULL;
static const chg_hw_param_t* chg_usb3_chgr_hw_paras = NULL;
static const chg_hw_param_t* chg_weak_chgr_hw_paras = NULL;
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
static const chg_hw_param_t* chg_5v_hvdcp_chgr_hw_paras = NULL;
static const chg_hw_param_t* chg_9v_hvdcp_chgr_hw_paras = NULL;
static const chg_hw_param_t* chg_12v_hvdcp_chgr_hw_paras = NULL;
#endif/*FEATURE_ON == MBB_CHG_HVDCP_CHARGE*/
#if (MBB_CHG_WIRELESS == FEATURE_ON)
static const chg_hw_param_t* chg_wireless_chgr_hw_paras = NULL;
/*���߳������λ��־���������߳��Ͷ����绥���л�*/
boolean g_wireless_online_flag = OFFLINE;
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/

#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
#define HVDCP_CHARGE_SET_CV_COMPARE_TH    (4100)
#define HVDCP_CHARGE_CV_4200MV             (4200)
#define HVDCP_CHARGE_CV_4350MV             (4350)
#define HVDCP_CHARGE_0P5_CURRENT_TH       (1200)
#define HVDCP_CHARGE_CURRENT_500MA        (500)
/*4.2V-4.35V 0.5C���������־*/
boolean g_hvdcp_charge_currnt_limit_flag = FALSE;
chg_hvdcp_type_value g_hvdcp_init_vol = CHG_HVDCP_9V;
#endif/*FEATURE_ON == MBB_CHG_HVDCP_CHARGE*/

#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
/* 0 - 10 ������ز����趨*/
int chg_sub_low_temp_changed = -1;
#define CHG_SUB_LOW_TEMP_TOP                (10)
#define CHG_TEMP_RESUM                      ( 3)
extern const charger_current_limit_st charger_current_limit_paras[CHG_CHGR_INVALID];
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/

/*0:-PwrOff Charging; 1:-Normal Charging.*/
/*0:-USB/NoStd Chgr;  1:-Wall/Standard Chgr.*/
static const uint32_t chg_fastchg_timeout_value_in_sec[2][2] =
{
    /*Power-off charge.*/
    {
        CHG_POWEROFF_FAST_USB_TIMER_VALUE,    //USB/NoStd Chgr
        CHG_POWEROFF_FAST_CHG_TIMER_VALUE,    //Wall/Standard Chgr
    },
    /*Normal charge.*/
    {
        CHG_FAST_USB_TIMER_VALUE,             //USB/NoStd Chgr
        CHG_FAST_CHG_TIMER_VALUE,             //Wall/Standard Chgr
    },
};

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
static const char* chg_stm_name_table[CHG_STM_MAX_ST] =
{
    "STM_TRANSIT_ST",                   //����״̬
    "STM_FAST_CHARGE_ST",               //���״̬
    "STM_MAINT_ST",                     //����״̬
    "STM_INVALID_CHG_TEMP_ST",          //����״̬
    "STM_BATTERY_ONLY",                 //��ص�����λ״̬
    "STM_WARMCHG_ST",                   //���³��״̬
    "STM_HVDCP_CHARGE_ST"               //��ѹ���״̬
};

static const char* chg_chgr_type_name_table[CHG_CHGR_INVALID + 1] =
{
    "CHG_CHGR_UNKNOWN",                 //���������δ֪
    "CHG_WALL_CHGR",                    //��׼�����
    "CHG_USB_HOST_PC",                  //USB
    "CHG_NONSTD_CHGR",                  //�����������
    "CHG_WIRELESS_CHGR",                //���߳����
    "CHG_EXGCHG_CHGR",                  //��������
    "CHG_500MA_WALL_CHGR",              //����
    "CHG_USB_OTG_CRADLE",               //cradle
    "CHG_HVDCP_CHGR",                   //��ѹ�����
    "CHG_CHGR_INVALID"                  //�����������
};

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*Lookup charge state machine name from chg_stm_name_table.*/
/*BE CAREFUL: Pls DO make sure "i" MUSTN'T exceed (CHG_STM_INIT_ST, CHG_STM_MAX_ST) range.*/
#define TO_STM_NAME(i)   ((i >= CHG_STM_TRANSIT_ST && i < CHG_STM_MAX_ST) \
                         ? chg_stm_name_table[i] : "NULL")

/*Lookup charger type name from chg_chgr_type_name_table*/
/*BE CAREFUL: Pls DO make sure "i" MUSTN'T exceed [CHG_CHGR_UNKNOWN, CHG_CHGR_INVALID] range.*/
#define TO_CHGR_NAME(i)  ((i >= CHG_CHGR_UNKNOWN && i <= CHG_CHGR_INVALID) \
                         ? chg_chgr_type_name_table[i] : "NULL")

/*����¶�ƽ���ĳ��ȸ���:����ѯ����Ϊ30������ѯ����Ϊ5*/
#define  CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST   (30)
#define  CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW   (5)

/*��ص�ѹƽ���ĳ��ȸ���:����ѯ����Ϊ30������ѯ����Ϊ5*/
#define  CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST    (30)
#define  CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW    (5)

/*���¹ػ��ײ�ȴ�ʱ�䣬��APP�ڸ�ʱ�������δ�ػ�����ײ�ִ�йػ�*/
#define  OVER_TEMP_SHUTOFF_DURATION         (45000)
/*�͵�ػ��ײ�ȴ�ʱ�䣬��APP�ڸ�ʱ�������δ�ػ�����ײ�ִ�йػ�*/
#define  LOW_BATT_SHUTOFF_DURATION          (45000)

/*�������쳣��ؼ��*/
#define CHG_AUTO_CUR_CTRL_DIE_TEMP_MIN      (-30)
/* �����󻵵�ؼ��ĵ�ѹ����***/
#ifdef  BSP_CONFIG_BOARD_E5_DC04 
#define CHG_SHORT_CIRC_BATTERY_THRES        (1000)
#else
#define CHG_SHORT_CIRC_BATTERY_THRES        (2700)
#endif

/*���¹ػ����¶ȸ澯���ʹ����ڴﵽ�ػ�����ǰ�ĸ÷�Χ����ʾ����*/
#define SHUTOFF_HIGH_TEMP_WARN_LEN          (2)
/*������ʾ�Ļָ����ʹ������¶Ȼָ����ػ����޵ĸ÷�Χ���£��������¸澯��ʾ*/
#define SHUTOFF_HIGH_TEMP_RESUME_LEN        (5)

#define LIMIT_SUPPLY_CURR_TEMP              (58)
#define LIMIT_SUPPLY_CURR__RESUME_LEN       (1)

/*��ع��º��Ӧ���ϱ������¼��Ĵ���*/
#define BATTERY_EVENT_REPORT_TIMES          (5)

static int32_t g_chg_revise_count = 0;
/*��ʼ���ǰ�ĵ�ص�ѹ�����ڳ��ʱ�ĵ�ѹ��������ת*/
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
#define CHG_BATT_VOLT_REVISE_LIMIT_UP       (is_batttemp_in_warm_chg_area()?4080:4280) //����ص�ѹ��������
#else
#define CHG_BATT_VOLT_REVISE_LIMIT_UP       (is_batttemp_in_warm_chg_area()?4080:4150) //����ص�ѹ��������
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/
#define CHG_BATT_VOLT_REVISE_LIMIT_DOWN     (3350)//�ŵ��ص�ѹ��������

#define CHG_BATT_VOLT_REVISE_WINDOW         (35)  //��ѹ��߱仯̫С�򲻲�����
#define CHG_BATT_VOLT_REVISE_LIMIT          (200) //��߲�������ֵ
#define CHG_BATT_VOLT_REVISE_COUNT          (15)  //�״ν��е�ص�ѹ������ֵ����Ĵ���

/* ��Ҫ�ϱ��¼���Ӧ��ʱ��ʱ5�����ߣ���֤Ӧ���ܹ��յ��¼�*/
#define ALARM_REPORT_WAKELOCK_TIMEOUT (5000)
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
#define SHUTDOWN_WAKELOCK_TIMEOUT (1000 * 120)
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)
#define EXTCHG_OVER_TEMP_STOP_THRESHOLD     (58)
#define EXTCHG_OVER_TEMP_RESUME_THRESHOLD   (51)
#if defined(BSP_CONFIG_BOARD_E5_DC04)
#define EXTCHG_OVER_TEMP_LIMIT_500MA        (51)
#endif
#define EXTCHG_LOW_VOLTAGE_RESUME_THRESHOLD (3550)
static boolean is_otg_extchg    = FALSE;
static boolean is_extchg_ovtemp = FALSE;
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
static boolean is_surf_extchg_ovtemp = FALSE;
#endif
#if (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
static boolean is_usb_extchg_ovtemp = FALSE;
#endif

static int32_t g_extchg_revise_count = 0;
#define EXTCHG_BATT_VOLT_REVISE_LIMIT       (200)
#define EXTCHG_THRESHOLD_PATH   "/data/userdata/extchg_config/extchg_threshold"
#define EXTCHG_DISABLE_PATH     "/data/userdata/extchg_config/extchg_disable_st"
#if defined(BSP_CONFIG_BOARD_E5_DC04)
#define EXTCHG_STOP_CAPACITY_NO_LIMIT       (0)
#endif
#define EXTCHG_DEFAULT_STOP_THRESHOLD       (5)
#define EXTCHG_STOP_CAPACITY_TEN            (10)
#define EXTCHG_STOP_CAPACITY_TWENTY         (20)
#define EXTCHG_STOP_CAPACITY_THIRTY         (30)
#define EXTCHG_STOP_CAPACITY_FORTY          (40)
#define EXTCHG_STOP_CAPACITY_FIFTY          (50)
#define EXTCHG_STOP_CAPACITY_SIXTY          (60)
#define EXTCHG_STOP_CAPACITY_SEVENTY        (70)
#define EXTCHG_STOP_CAPACITY_EIGHTY         (80)
#define EXTCHG_STOP_CAPACITY_NINETY         (90)
#define EXTCHG_STOP_CAPACITY_HUNDRED        (100)

#define EXTCHG_DEFAULT_STOP_VOLTAGE         (3470)
#define EXTCHG_STOP_VOLTAGE_TEN             (3579)
#define EXTCHG_STOP_VOLTAGE_TWENTY          (3637)
#define EXTCHG_STOP_VOLTAGE_THIRTY          (3670)
#define EXTCHG_STOP_VOLTAGE_FORTY           (3696)
#define EXTCHG_STOP_VOLTAGE_FIFTY           (3728)
#define EXTCHG_STOP_VOLTAGE_SIXTY           (3769)
#define EXTCHG_STOP_VOLTAGE_SEVENTY         (3830)
#define EXTCHG_STOP_VOLTAGE_EIGHTY          (3902)
#define EXTCHG_STOP_VOLTAGE_NINETY          (3985)
#define EXTCHG_STOP_VOLTAGE_HUNDRED         (4130)

/*ֹͣ�������ŵ�ص���ֵĬ��Ϊ�͵�5%->3.55V*/
int32_t g_extchg_voltage_threshold = 0;
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
int32_t g_extchg_stop_soc_threshold = 0;
#endif
/*�Ƿ�����ֹͣ�������־1:����ֹͣ 0:���ÿ���*/
int32_t g_extchg_diable_st = 0;
/*��¼�û��ϴ����õ�����ֹͣ�������־1������ֹͣ��0�����ÿ���*/
static int32_t g_last_extchg_diable_st = 0;
#define EXTCHG_TEMP_COMPENSATE_VALUE        (4)
/*�û�ͨ��TOUCH UIѡ��Ķ������ģʽ��1:�������� 2:�����������ҵ��*/
int32_t g_ui_choose_exchg_mode = 0;
boolean g_exchg_enable_flag = FALSE;
/*������USB ID����λ��־�����ڶ���������߳��Ļ����л�*/
boolean g_exchg_online_flag = OFFLINE;
#endif /*MBB_CHG_EXTCHG == FEATURE_ON */

#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
/*����DCM�±������еļ�������״̬*/
enum dcm_cur_lmt_state
{
    DCM_CUR_LMT_INITIAL = -1,
    DCM_CUR_LMT_NORMAL,
    DCM_CUR_LMT_1ST_LVL,
    DCM_CUR_LMT_2ND_LVL
}dcm_cur_lmt_state;

enum dcm_cur_lmt_state g_dcm_current_limit_state = DCM_CUR_LMT_NORMAL;
CHG_MSG_TYPE g_dcm_dbg_msg_level = CHG_MSG_DEBUG;

/*ϵͳ����һ������״̬������*/
int32_t dcm_enter_to_1stlvl_surf_temp_threshold = 48;
/*ϵͳ�ָ���һ������״̬������*/
int32_t dcm_back_to_1stlvl_surf_temp_threshold = 49;
/*ϵͳ�����������״̬������*/
int32_t dcm_enter_to_2ndlvl_surf_temp_threshold = 51;
/*ϵͳ�ָ�����������״̬������*/
int32_t dcm_back_to_normal_surf_temp_threshold = 46;
/*ϵͳ���ò�ͬ�����ĵ����ж�����*/
int32_t dcm_cur_lmt_ignored_batt_cap_threshold = 25;
#if (MBB_CHG_EXTCHG == FEATURE_ON)
/*ϵͳֹͣ��������¶�����*/
int32_t dcm_stop_rvschg_surf_temp_threshold    = 51;
/*ϵͳ�ָ���������¶�����*/
int32_t dcm_backto_rvschg_surf_temp_threshold  = 46;
#endif

#define DCM_CUR_LMT_ENTER_TO_1ST_LVL_TH dcm_enter_to_1stlvl_surf_temp_threshold
#define DCM_CUR_LMT_BACK_TO_1ST_LVL_TH dcm_back_to_1stlvl_surf_temp_threshold
#define DCM_CUR_LMT_ENTER_TO_2ND_LVL_TH dcm_enter_to_2ndlvl_surf_temp_threshold
#define DCM_CUR_LMT_BACK_TO_NORMAL_TH dcm_back_to_normal_surf_temp_threshold
#define DCM_CUR_LMT_IGNORED_BATT_CAP_TH  dcm_cur_lmt_ignored_batt_cap_threshold
#if (MBB_CHG_EXTCHG == FEATURE_ON)
#define DCM_RVSCHG_STOP_TSURF_TH         dcm_stop_rvschg_surf_temp_threshold    
#define DCM_RVSCHG_BACKTO_TSURF_TH       dcm_backto_rvschg_surf_temp_threshold
#endif

struct st_surf_temp_stat
{
    enum en_surf_sensor_connect_stat is_connected;
    int32_t cur_surf_temp;
    int32_t sys_surf_temp;
};

struct st_surf_temp_stat g_surf_temp_state = {SURF_SENSOR_CONNECT_UNKNOWN, 0, 0};
#endif

#if ((MBB_CHG_EXTCHG == FEATURE_ON) || (MBB_CHG_WIRELESS == FEATURE_ON))
/*�±ߺ�����ͬƽ̨ʵ�ַ�ʽ��ͬ����Ҫ��������*/

extern void usb_notify_event(unsigned long val, void *v);
#endif/*(MBB_CHG_EXTCHG == FEATURE_ON) || (MBB_CHG_WIRELESS == FEATURE_ON)*/

#if ( FEATURE_ON == MBB_MLOG )
extern void mlog_set_statis_info(char *item_name,unsigned int item_value);
#endif

void load_ftm_mode_init(void)
{
    if(CHG_OK == chg_config_para_read(NV_FACTORY_MODE_I, (char *) &g_real_factory_mode, sizeof(uint32_t)))
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:read factory mode ok,mode =  %d\n ", g_real_factory_mode);

        if(0 == g_real_factory_mode)
        {
            chg_current_ftm_mode = TRUE;
        }
        else
        {
            chg_current_ftm_mode = FALSE;
        }
    }
    else/*��NV��ʧ�ܣ���Ĭ��Ϊ�����汾*/
    {
#ifndef MBB_FACTORY_FEATURE
        chg_current_ftm_mode = FALSE;
#else
        chg_current_ftm_mode = TRUE;
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM: In ftm mode now \n ");
#endif
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:read factory mode nv fail \n ");
    }
}


void chg_batt_temp_init(void)
{
    if(CHG_OK != chg_config_para_read(NV_OVER_TEMP_SHUTOFF_PROTECT,&g_chgBattOverTempPeotect, \
    sizeof(CHG_BATTERY_OVER_TEMP_PROTECT_NV)))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NV_OVER_TEMP_SHUTOFF_PROTECT read fail \n ");
        g_chgBattOverTempPeotect.ulIsEnable =  1;
        g_chgBattOverTempPeotect.lCloseAdcThreshold = 61; /*power off temp +1 */
        g_chgBattOverTempPeotect.ulTempOverCount = 1;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NV_OVER_TEMP_SHUTOFF_PROTECT read success \n ");
    }

    if(CHG_OK != chg_config_para_read(NV_LOW_TEMP_SHUTOFF_PROTECT,&g_chgBattLowTempPeotect,\
    sizeof(CHG_BATTERY_LOW_TEMP_PROTECT_NV)))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NV_LOW_TEMP_SHUTOFF_PROTECT read fail \n ");
        g_chgBattLowTempPeotect.ulIsEnable =  1;
        g_chgBattLowTempPeotect.lCloseAdcThreshold = -20;
        g_chgBattLowTempPeotect.ulTempLowCount = 1;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NV_LOW_TEMP_SHUTOFF_PROTECT read success \n ");
    }

    if(CHG_OK != chg_config_para_read(NV_BATT_TEMP_PROTECT_I,&g_chgShutOffTempProtect,\
    sizeof(CHG_SHUTOFF_TEMP_PROTECT_NV_TYPE)))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NV_BATT_TEMP_PROTECT_I read fail \n ");
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
        g_chgShutOffTempProtect.ulChargeIsEnable = 1;                   //����±���ʹ��
        g_chgShutOffTempProtect.overTempchgStopThreshold = 55;          //�����±�������
        g_chgShutOffTempProtect.subTempChgLimitCurrentThreshold = 45;   //���³����ڵ���¶�����
        g_chgShutOffTempProtect.lowTempChgStopThreshold = 0;            //�����±�������
        g_chgShutOffTempProtect.overTempChgResumeThreshold = 52;        //�����»ָ��¶�����
        g_chgShutOffTempProtect.lowTempChgResumeThreshold = 3;          //�����»ָ��¶�����
        g_chgShutOffTempProtect.chgTempProtectCheckTimes = 1;           //���ͣ����ѯ����
        g_chgShutOffTempProtect.chgTempResumeCheckTimes = 1;            //��縴����ѯ����
        g_chgShutOffTempProtect.exitWarmChgToNormalChgThreshold = 42;   //���³��ָ������³���¶�����
        g_chgShutOffTempProtect.reserved2 = 0;                          //Ԥ��
#else
        g_chgShutOffTempProtect.ulChargeIsEnable = 1;                   //����±���ʹ��
        g_chgShutOffTempProtect.overTempchgStopThreshold = 45;          //�����±�������
        g_chgShutOffTempProtect.subTempChgLimitCurrentThreshold = 38;   //2A�����3000mA��ص������������
        g_chgShutOffTempProtect.lowTempChgStopThreshold = 0;            //�����±�������
        g_chgShutOffTempProtect.overTempChgResumeThreshold = 42;        //�����»ָ��¶�����
        g_chgShutOffTempProtect.lowTempChgResumeThreshold = 3;          //�����»ָ��¶�����
        g_chgShutOffTempProtect.chgTempProtectCheckTimes = 1;           //���ͣ����ѯ����
        g_chgShutOffTempProtect.chgTempResumeCheckTimes = 1;            //��縴����ѯ����
        g_chgShutOffTempProtect.exitWarmChgToNormalChgThreshold = 0;    //���³��ָ������³���¶�����
        g_chgShutOffTempProtect.reserved2 = 0;                          //Ԥ��
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NV_BATT_TEMP_PROTECT_I read success \n ");
    }
}


void load_on_off_mode_parameter(void)
{
    int32_t ret_val = CHG_ERROR;
    memset( (void *)&g_real_powerup_mode_value, 0, sizeof(POWERUP_MODE_TYPE) );

    /* ��nv�ж�ȡӲ�����Կ���ģʽ��־*/
    /* �����ȡʧ�ܲ����ã�����Ĭ��ֵ*/
    ret_val = chg_config_para_read(NV_POWERUP_MODE,&g_real_powerup_mode_value, \
        sizeof(POWERUP_MODE_TYPE) );
    if ( CHG_OK == ret_val )
    {
        chg_print_level_message(CHG_MSG_INFO,"CHG_STM:\r\nno_battery_powerup_enable=%d,\r\nexception_poweroff_poweron_enable=%d,\
            \r\nlow_battery_poweroff_disable=%d,\r\n ",\
            g_real_powerup_mode_value.no_battery_powerup_enable,
            g_real_powerup_mode_value.exception_poweroff_poweron_enable,
            g_real_powerup_mode_value.low_battery_poweroff_disable);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:read hw test powerup mode nv error, ret_val = %d \n ", ret_val, 0, 0);

        g_real_powerup_mode_value.no_battery_powerup_enable = FALSE;
        g_real_powerup_mode_value.exception_poweroff_poweron_enable = FALSE;
        g_real_powerup_mode_value.low_battery_poweroff_disable = FALSE;
    }
}


void chg_batt_volt_init(void)
{
    if(CHG_OK != chg_config_para_read(NV_BATT_VOLT_PROTECT_I,&g_chgBattVoltProtect,\
        sizeof(CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE)))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NV_BATT_VOLT_PROTECT_I read fail \n ");

        g_chgBattVoltProtect.battVoltPowerOnThreshold = 3450;     //������ѹ����
        g_chgBattVoltProtect.battVoltPowerOffThreshold = 3450;    //�ػ���ѹ����
        g_chgBattVoltProtect.battOverVoltProtectThreshold = 4220; //ƽ������ѹ��������(ƽ��ֵ)
        g_chgBattVoltProtect.battOverVoltProtectOneThreshold = 4240; //���γ���ѹ��������(����ֵ)
        g_chgBattVoltProtect.battChgTempMaintThreshold = 4100;    //��ʱͣ���ж�����
        g_chgBattVoltProtect.battChgRechargeThreshold = 4000;     //���³����θ�������
        g_chgBattVoltProtect.VbatLevelLow_MAX = 3550;             //�͵���������
        g_chgBattVoltProtect.VbatLevel0_MAX = 3550;               //0���ѹ��������
        g_chgBattVoltProtect.VbatLevel1_MAX = 3610;               //1���ѹ��������
        g_chgBattVoltProtect.VbatLevel2_MAX = 3670;               //2���ѹ��������
        g_chgBattVoltProtect.VbatLevel3_MAX = 3770;               //3���ѹ��������
        g_chgBattVoltProtect.battInsertChargeThreshold = 4150;    //���������ж��Ƿ�Ҫ��������
        g_chgBattVoltProtect.battNormalTempRechergeThreshold = 4100; //���³����θ�������
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NV_BATT_VOLT_PROTECT_I read success \n ");
    }
}

boolean chg_is_no_battery_powerup_enable(void)
{
#ifdef CHG_STUB
    g_real_powerup_mode_value.no_battery_powerup_enable = TRUE;
#endif/*CHG_STUB*/

#ifdef HUAWEI_USB_POWERON
    static_smem_vendor0 *smem_data = NULL;
    smem_data = (static_smem_vendor0 *)SMEM_HUAWEI_ALLOC(SMEM_ID_VENDOR0 , sizeof(static_smem_vendor0 ));
    if(NULL == smem_data)
    {
        pr_err("%s: SMEM_HUAWEI_ALLOC ERROR.\n",__func__);
    }
    if ( STATUS_POWER_ON_USB == smem_data->smem_huawei_poweroff_chg )
    {
        g_real_powerup_mode_value.no_battery_powerup_enable = TRUE;
    }
#endif  /*HUAWEI_USB_POWERON*/

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_no_battery_powerup_enable %d !\n",
                            g_real_powerup_mode_value.no_battery_powerup_enable);
    return g_real_powerup_mode_value.no_battery_powerup_enable;
}

boolean chg_is_low_battery_poweroff_disable(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_low_battery_poweroff_disable %d !\n",
                            g_real_powerup_mode_value.low_battery_poweroff_disable);
    return g_real_powerup_mode_value.low_battery_poweroff_disable;
}


boolean chg_is_exception_poweroff_poweron_mode(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_exception_poweroff_poweron_mode %d !\n",
                            g_real_powerup_mode_value.exception_poweroff_poweron_enable);
    return g_real_powerup_mode_value.exception_poweroff_poweron_enable;
}


BATT_LEVEL_ENUM chg_get_batt_level(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_get_batt_level=%d !\n",chg_real_info.bat_volt_lvl);

    return chg_real_info.bat_volt_lvl;
}


boolean chg_is_ftm_mode(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_ftm_mode=%d !\n",chg_current_ftm_mode);
    return chg_current_ftm_mode;
}


void chg_batt_error_handle(void)
{
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_batt_error_handle->chg_set_power_off\n");
#if (FEATURE_ON == MBB_CHG_COULOMETER)
    /*֪ͨ���ÿ��ؼ�ģ�����Ƴ��������Ӧ����*/
    hisi_battery_removal_handle();
#endif
    chg_set_power_off(DRV_SHUTDOWN_BATTERY_ERROR);
    return;
}


boolean chg_get_batt_id_valid(void)
{
    int32_t batt_temp = 0;
    boolean chg_ftm_mode = FALSE;
    boolean chg_no_battery_powerup_mode = FALSE;

    /* ����ģʽ��һֱ��Ϊ���Ϊ�Ϸ����**/
    chg_ftm_mode = chg_is_ftm_mode();
    chg_no_battery_powerup_mode = chg_is_no_battery_powerup_enable();

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_ftm_mode=%d ,no_battery_powerup_mode=%d!\n",
                            chg_ftm_mode,chg_no_battery_powerup_mode);

    if( (TRUE == chg_ftm_mode) || (TRUE == chg_no_battery_powerup_mode) )
    {
        return TRUE;
    }

    /* ʹ�õ���¶��ж�Ŀǰ����Ƿ�Ϊ�γ�**/
    batt_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
    if (CHG_AUTO_CUR_CTRL_DIE_TEMP_MIN == batt_temp)
    {
        batt_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
        if (CHG_AUTO_CUR_CTRL_DIE_TEMP_MIN ==  batt_temp)
        {
            batt_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
            if (CHG_AUTO_CUR_CTRL_DIE_TEMP_MIN == batt_temp)
            {
                chg_print_level_message(CHG_MSG_ERR,"CHG_STM:batt_temp below 30 degree bat not present!\n");
                return FALSE;
            }
        }
    }
    return TRUE;
}

boolean chg_is_emergency_state(void)
{
    if((chg_batt_high_temp_58_flag == TRUE) || (chg_batt_low_battery_flag == TRUE))
    {
        chg_batt_condition_error_flag = TRUE;
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:is_emergency_state!\n");
    }
    else
    {
        chg_batt_condition_error_flag = FALSE;
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:is not emergency state!\n");
    }

    return chg_batt_condition_error_flag;
}

void chg_batt_error_detect_temp(void)
{
    if(FALSE == chg_get_batt_id_valid())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:batt invalid system shutdown!\n");
        mlog_print(MLOG_CHG, mlog_lv_fatal, "battery ts error detected.\n");

        chg_batt_error_handle();
    }
    return;
}

void chg_batt_error_detect_volt(void)
{
    int32_t  batt_volt = 0;
    boolean  batt_err_flag = FALSE;
    boolean  chg_ftm_mode = FALSE;
    boolean  chg_no_battery_powerup_mode = FALSE;

    /* ����ģʽ��һֱ��Ϊ���Ϊ�Ϸ����**/
    chg_ftm_mode = chg_is_ftm_mode();
    chg_no_battery_powerup_mode = chg_is_no_battery_powerup_enable();

    if( (TRUE == chg_ftm_mode) || (TRUE == chg_no_battery_powerup_mode) )
    {
        return;
    }

    /* ��ѯ��ص�ѹ,�����ص�ѹ���ڹ涨ֵ,�ж������*/
    batt_volt = chg_get_batt_volt_value();
    if (CHG_SHORT_CIRC_BATTERY_THRES > batt_volt)
    {
        batt_volt = chg_get_batt_volt_value();

        if (CHG_SHORT_CIRC_BATTERY_THRES > batt_volt)
        {
            batt_volt = chg_get_batt_volt_value();
            if (CHG_SHORT_CIRC_BATTERY_THRES > batt_volt)
            {
                batt_err_flag = TRUE;
            }
        }
    }

    if(TRUE == batt_err_flag)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:batt shorted system shutdown!\n");
        mlog_print(MLOG_CHG, mlog_lv_fatal, "battery short error detected.\n");
        chg_batt_error_handle();
    }

    return;
}

static int32_t chg_calc_average_temp_value(int32_t new_data)
{
    int32_t    index = 0;
    int32_t    sum = 0;
    int32_t    bat_temp_avg = 0;
    int32_t    new_poll_mode;
    static int32_t  old_poll_mode = FAST_POLL_CYCLE;
    static int32_t record_avg_num_fast = 0;
    static int32_t record_avg_num_slow = 0;
    static int32_t record_value_fast[CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST] = {0};
    static int32_t record_value_slow[CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW] = {0};

    /*��ѯ��ǰ��ѯģʽ*/
    new_poll_mode = chg_poll_timer_get();
    if(new_poll_mode != old_poll_mode)
    {
        if(FAST_POLL_CYCLE == new_poll_mode)/*������ѯ�л�������ѯ*/
        {
            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;index++)
            {
                record_value_fast[index] = chg_real_info.battery_temp;
            }
            record_avg_num_fast = CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;
        }
        else//�ɿ���ѯ�л�������ѯ
        {
            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;index++)
            {
                record_value_slow[index] = chg_real_info.battery_temp;
            }
            record_avg_num_slow = CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;
        }

        old_poll_mode = new_poll_mode;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:new_poll_mode==old_poll_mode!\n");
    }

    if(FAST_POLL_CYCLE == new_poll_mode)/*����ѯģʽ*/
    {
        /*�����е�ǰԪ�ر��С��30*/
        if(CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST > record_avg_num_fast)
        {
            record_value_fast[record_avg_num_fast] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < record_avg_num_fast; index++)
            {
                sum += record_value_fast[index];
            }

            bat_temp_avg = sum / record_avg_num_fast;
        }
        else/*Ԫ�ظ�����Ŵ��ڵ���30��*/
        {
            record_value_fast[record_avg_num_fast % CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST; index++)
            {
                sum += record_value_fast[index];
            }

            bat_temp_avg = sum / CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;

            /*���Ԫ�ظ�������Ǿ�̬���鳤�ȵ�����,������Ԫ�ظ�������Ǿ�̬���鳤�ȼ�CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST * 2 == record_avg_num_fast)
            {
                record_avg_num_fast = CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;
            }
        }
    }
    else/*����ѯģʽ*/
    {
        /*�����е�ǰԪ�ر��С��5*/
        if(CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW > record_avg_num_slow)
        {
            record_value_slow[record_avg_num_slow] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < record_avg_num_slow; index++)
            {
                sum += record_value_slow[index];
            }

            bat_temp_avg = sum / record_avg_num_slow;
        }
        else/*Ԫ�ظ�����Ŵ��ڵ���5��*/
        {
            record_value_slow[record_avg_num_slow % CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW; index++)
            {
                sum += record_value_slow[index];
            }

            bat_temp_avg = sum / CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;

            /*���Ԫ�ظ�������Ǿ�̬���鳤�ȵ�����,������Ԫ�ظ�������Ǿ�̬���鳤�ȼ�CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW * 2 == record_avg_num_slow)
            {
                record_avg_num_slow = CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;
            }
        }
    }

    return bat_temp_avg;
}

int32_t chg_huawei_set_temp(int32_t temp)
{
    return temp;
}

#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
/*******************************************************************
            Helper Function: For Test Purpose Only
*******************************************************************/
int32_t chg_dcm_test_func(uint8_t op, uint8_t val)
{
    int32_t i = 0;
    const char* threshold_name_table[] = 
    {
        "Surface Back to Normal Current Limit",
        "Surface Enter to 1st Level Current Limit",
        "Surface Back to 1st Level Current Limit",
        "Surface Enter to 2nd Level Current Limit",
#if (MBB_CHG_EXTCHG == FEATURE_ON)
        /*���������ͣ���¶�����*/
        "Surface Stop Reverse Charge",
        /*��������Ǹ����¶�����*/
        "Surface Back to Reverse Charge",
#endif
    };

    int32_t* threshold_val_table[] =
    {
        &DCM_CUR_LMT_BACK_TO_NORMAL_TH,
        &DCM_CUR_LMT_ENTER_TO_1ST_LVL_TH,
        &DCM_CUR_LMT_BACK_TO_1ST_LVL_TH,
        &DCM_CUR_LMT_ENTER_TO_2ND_LVL_TH,
#if (MBB_CHG_EXTCHG == FEATURE_ON)
        &DCM_RVSCHG_STOP_TSURF_TH,
        &DCM_RVSCHG_BACKTO_TSURF_TH,
#endif
    };
    int threshold_array_size = 
        sizeof(threshold_val_table) / sizeof(threshold_val_table[0]);
    enum op_code
    {
        OP_PRT_HELP,
        OP_TH_ID_BEGIN = 1,
        /*�����END�±�Ҫ����������Ԫ�ظ�������һ��*/
#if (MBB_CHG_EXTCHG == FEATURE_ON)
        /*DCM�������±�����������������������*/
        OP_TH_ID_END   = 6,
#else
        /*һ����4����������*/
        OP_TH_ID_END   = 4,
#endif
        OP_SET_DBUG_LEVEL,
        OP_SET_BATT_CAP,
    };

    switch(op)
    {
        case OP_PRT_HELP:
            chg_print_level_message(CHG_MSG_ERR, "============ %s Help Message " \
                "============\n", __func__);
            chg_print_level_message(CHG_MSG_ERR, "0: Print this help message.\n");
            for (i = 0; i < threshold_array_size; ++i)
            {
                chg_print_level_message(CHG_MSG_ERR, "%d: Set %s Threshold to [val].\n",
                    i + 1, threshold_name_table[i]);
            }
            chg_print_level_message(CHG_MSG_ERR, "5: Set Debug Level: 0: ERROR;" \
                " 1: INFO; >=2: DEBUG.\n");
            chg_print_level_message(CHG_MSG_ERR, "6: Set Ingnore Current Limit " \
                "Battery Capacity Threshold to [val%%].\n");
            chg_print_level_message(CHG_MSG_ERR, "Others: Dump all relative threholds:\n");
            break;
        case OP_TH_ID_BEGIN ... OP_TH_ID_END:
            chg_print_level_message(CHG_MSG_ERR, "Set %s Threshold to %d.\n",
                threshold_name_table[op - 1], val);
            *(threshold_val_table[op - 1]) = val;
            break;
        case OP_SET_DBUG_LEVEL:
            chg_print_level_message(CHG_MSG_ERR, "Set Debug Level %d.\n", val);
            g_dcm_dbg_msg_level = val;
            break;
        case OP_SET_BATT_CAP:
            chg_print_level_message(CHG_MSG_ERR, "Set cur_lmt_ignored_batt_cap_th to %d%%.\n", val);
            dcm_cur_lmt_ignored_batt_cap_threshold = val;
            break;
        default:
            chg_print_level_message(CHG_MSG_ERR, "Dump all relative threholds as follows:\n");
            for (i = 0; i < threshold_array_size; ++i)
            {
                chg_print_level_message(CHG_MSG_ERR, "%s Threshold: %d'C\n",
                    threshold_name_table[i], *threshold_val_table[i]);
            }
            chg_print_level_message(CHG_MSG_ERR, "g_dcm_dbg_msg_level = %d.\n", 
                g_dcm_dbg_msg_level);
            chg_print_level_message(CHG_MSG_ERR, "cur_lmt_ignored_batt_cap_th = %d%%.\n",
                dcm_cur_lmt_ignored_batt_cap_threshold);
    }

    return 0;
}

/*******************************************************************
Function:      chg_calc_surf_avg_temp
Description:   ģ�µ���¶ȵĴ����㷨��ʵ�ֶԱ�����¶ȵ�ƽ������
Data Accessed: ��
Data Updated:  ��
Input:         �²ɼ����¶�ֵ
Return:        ƽ������¶�ֵ
*******************************************************************/
static int32_t chg_calc_surf_avg_temp(int32_t new_data)
{
    int32_t    index = 0;
    int32_t    sum = 0;
    int32_t    surface_temp = 0;
    int32_t    new_poll_mode;
    static int32_t  old_poll_mode = FAST_POLL_CYCLE;
    static int32_t record_avg_num_fast = 0;
    static int32_t record_avg_num_slow = 0;
    static int32_t record_value_fast[CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST] = {0};
    static int32_t record_value_slow[CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW] = {0};

    /*��ѯ��ǰ��ѯģʽ*/
    new_poll_mode = chg_poll_timer_get();

    if(new_poll_mode != old_poll_mode)
    {
        if(FAST_POLL_CYCLE == new_poll_mode)/*������ѯ�л�������ѯ*/
        {
            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;index++)
            {
                record_value_fast[index] = g_surf_temp_state.sys_surf_temp;
            }
            record_avg_num_fast = CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;
        }
        else//�ɿ���ѯ�л�������ѯ
        {
            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;index++)
            {
                record_value_slow[index] = g_surf_temp_state.sys_surf_temp;
            }
            record_avg_num_slow = CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;
        }

        old_poll_mode = new_poll_mode;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:new_poll_mode==old_poll_mode!\n");
    }

    if(FAST_POLL_CYCLE == new_poll_mode)/*����ѯģʽ*/
    {
        /*�����е�ǰԪ�ر��С��30*/
        if(CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST > record_avg_num_fast)
        {
            record_value_fast[record_avg_num_fast] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < record_avg_num_fast; index++)
            {
                sum += record_value_fast[index];
            }

            surface_temp = sum / record_avg_num_fast;
        }
        else/*Ԫ�ظ�����Ŵ��ڵ���30��*/
        {
            record_value_fast[record_avg_num_fast % CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST; index++)
            {
                sum += record_value_fast[index];
            }

            surface_temp = sum / CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;

            /*���Ԫ�ظ�������Ǿ�̬���鳤�ȵ�����,������Ԫ�ظ�������Ǿ�̬���鳤�ȼ�CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST * 2 == record_avg_num_fast)
            {
                record_avg_num_fast = CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;
            }
        }
    }
    else/*����ѯģʽ*/
    {
        /*�����е�ǰԪ�ر��С��5*/
        if(CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW > record_avg_num_slow)
        {
            record_value_slow[record_avg_num_slow] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < record_avg_num_slow; index++)
            {
                sum += record_value_slow[index];
            }

            surface_temp = sum / record_avg_num_slow;
        }
        else/*Ԫ�ظ�����Ŵ��ڵ���5��*/
        {
            record_value_slow[record_avg_num_slow % CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW; index++)
            {
                sum += record_value_slow[index];
            }

            surface_temp = sum / CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;

            /*���Ԫ�ظ�������Ǿ�̬���鳤�ȵ�����,������Ԫ�ظ�������Ǿ�̬���鳤�ȼ�CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW * 2 == record_avg_num_slow)
            {
                record_avg_num_slow = CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;
            }
        }
    }

    return surface_temp;
}

/*****************************************************************************
Function:       chg_dcm_perform_input_current_limit
Description:    Docomo "LATEST" charge thermal protect design.
                Do the input current limit control, if the condition meets.
Calls:          chg_set_supply_limit.
Data Accessed:  g_dcm_current_limit_state
Data Updated:  
Input:        None
Output:       None
Return:       None
Others:       None
*****************************************************************************/
void chg_dcm_perform_input_current_limit(enum dcm_cur_lmt_state state)
{
    uint8_t is_input_current_limit_needed = 
        (chg_get_sys_batt_capacity() > DCM_CUR_LMT_IGNORED_BATT_CAP_TH);
    uint32_t input_current_limit = 0;
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();

    /*[][0] for battery capacity <= 25%. [][1] for > 25%.*/
    uint32_t usb_chgr_input_current_limit_cfg [DCM_CUR_LMT_2ND_LVL + 1] [2] =
    {
        {500, 500}, {500, 500}, {500, 100}  /*All in mA*/
    };

    /*[][0] for battery capacity <= 25%. [][1] for > 25%.*/
    uint32_t usb3p0_chgr_input_current_limit_cfg [DCM_CUR_LMT_2ND_LVL + 1] [2] =
    {
        {900, 900}, {900, 500}, {900, 100}  /*All in mA*/
    };

     /*[][0] for battery capacity <= 25%. [][1] for > 25%.*/
    uint32_t std_chgr_input_current_limit_cfg [DCM_CUR_LMT_2ND_LVL + 1] [2] =
    {
        {2000, 2000}, {900,  500}, {900,  100}  /*All in mA*/
    };

    if (state > DCM_CUR_LMT_2ND_LVL || state < DCM_CUR_LMT_NORMAL)
    {
        chg_print_level_message(CHG_MSG_ERR, "Invalid state %d in %s.\n", state, __func__);
        return;
    }

    if (CHG_WALL_CHGR == cur_chgr_type)
    {
        input_current_limit = 
          std_chgr_input_current_limit_cfg[(uint8_t)state][is_input_current_limit_needed];
    }
    else if (CHG_USB_HOST_PC == cur_chgr_type && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        input_current_limit = 
          usb3p0_chgr_input_current_limit_cfg[(uint8_t)state][is_input_current_limit_needed];
    }
    else
    {
        input_current_limit =
          usb_chgr_input_current_limit_cfg[(uint8_t)state][is_input_current_limit_needed];
    }

    mlog_print(MLOG_CHG, mlog_lv_info, "CHG_DCM: Set input current limit to %dmA.\n",
              input_current_limit);
    chg_print_level_message(CHG_MSG_INFO, "%s: Set input current limit to %dmA.\n",
                            __func__, input_current_limit);
    mlog_print(MLOG_CHG, mlog_lv_info, "CHG_DCM: [State]:%d, [ChgrType]:%d, " \
              "[BattCap]:%d%%.\n", state, cur_chgr_type, chg_get_sys_batt_capacity());
    chg_print_level_message(CHG_MSG_INFO, "[State]:%d, [ChgrType]:%d, [BattCap]:%d%%.\n",
                            state, cur_chgr_type, chg_get_sys_batt_capacity());
    if (FALSE == chg_set_supply_limit(input_current_limit))
    {
        chg_print_level_message(CHG_MSG_ERR, "Set input current limit failed.\n");
    }
}

/****************************************************************************
Function:       chg_dcm_check_input_current_limit
Description:    Docomo "LATEST" charge thermal protect design.
                Here we polling the temperature surface, if the threshold
                reached, update the input current limit value.
                Note: If surface temperature sensor was not detected, power 
                supply input current limit would still be updated in charge
                state machine.
Calls:          chg_dcm_perform_input_current_limit
Data Accessed:  g_surf_temp_state
Data Updated:   g_surf_temp_state 
Input:        None
Output:       None
Return:       None
Others:       None
****************************************************************************/
void chg_dcm_check_input_current_limit(void)
{
    int16_t sys_surf_temp = g_surf_temp_state.sys_surf_temp;
    enum dcm_cur_lmt_state new_state = DCM_CUR_LMT_INITIAL;
    static enum dcm_cur_lmt_state cur_state = DCM_CUR_LMT_INITIAL;
    static chg_chgr_type_t cur_chgr = CHG_CHGR_UNKNOWN;

    /*USB�˿��¶ȱ������������õ�ǰ״̬ΪINITIAL�����ǰ��������ͻ��*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    if( TRUE == chg_get_usb_temp_protect_stat() )
    {
        cur_state = DCM_CUR_LMT_INITIAL;
        return;
    }
#endif
    /*������ͱ����������������õ�ǰ״̬ΪINITIAL�����ǰ��������ͻ��*/
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    if(TRUE == chg_battery_protect_flag)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:In battery protect condition " \
          "do not dcm input_current_limit!\n");
        cur_state = DCM_CUR_LMT_INITIAL;
        return;
    }
#endif
    /*If charge thermal protect disabled, don't perform checking.*/
    if (0 == CHG_TEMP_PROTECT_ENABLE)
    {
        /*"Set" surface temperature sensor as "disconnected", left charge
          state machine control the input power supply current limit.*/
        if (SURF_SENSOR_DISCONNECTED != g_surf_temp_state.is_connected)
        {
            chg_print_level_message(CHG_MSG_ERR, "Charge thermal protection " \
              "disabled, assume surf sensor disconnected.\n");
            g_surf_temp_state.is_connected = SURF_SENSOR_DISCONNECTED;
        }
        return;
    }

    /*Check if the surface temperature sensor connected, if not, ignore the 
     input current limit set, left the charge state machine finish it.*/
    if (SURF_SENSOR_CONNECT_UNKNOWN == g_surf_temp_state.is_connected)
    {
        if (SURF_FPC_FAKE_THRESHOLD == sys_surf_temp)
        {
            chg_print_level_message(CHG_MSG_ERR, "Detect surf temp sensor failed.\n");
            mlog_print(MLOG_CHG, mlog_lv_fatal,
                      "CHG_DCM: Detect surface temperature sensor failed.\n");
            g_surf_temp_state.is_connected = SURF_SENSOR_DISCONNECTED;
            return;
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "Detect surf temp sensor success," \
              " cur_surf_temp = %d'C.\n", sys_surf_temp);
            g_surf_temp_state.is_connected = SURF_SENSOR_CONNECTED;
        }
    }
    else if (SURF_SENSOR_DISCONNECTED == g_surf_temp_state.is_connected)
    {
        chg_print_level_message(g_dcm_dbg_msg_level, "Surf temp sensor not detected," \
          " %s do nothing.\n", __func__);
        return;
    }
    else
    {
        chg_print_level_message(g_dcm_dbg_msg_level, "TSurf = %d'C, TSurf_sys = %d'C.\n", 
          g_surf_temp_state.cur_surf_temp, g_surf_temp_state.sys_surf_temp);
    }

    /*Set the initial state*/
    if (DCM_CUR_LMT_INITIAL == cur_state)
    {
        if (sys_surf_temp >= DCM_CUR_LMT_ENTER_TO_2ND_LVL_TH)
        {
            new_state = DCM_CUR_LMT_2ND_LVL;
        }
        else if (sys_surf_temp >= DCM_CUR_LMT_ENTER_TO_1ST_LVL_TH)
        {
            new_state = DCM_CUR_LMT_1ST_LVL;
        }
        else
        {
            new_state = DCM_CUR_LMT_NORMAL;
        }
        chg_print_level_message(CHG_MSG_INFO, "System enter to %d state " \
                                "at initial.\n", new_state);
    }
    /*If we are at normal state, check if we reached the 1st level threshold.*/
    else if (DCM_CUR_LMT_NORMAL == cur_state)
    {
        if (sys_surf_temp >= DCM_CUR_LMT_ENTER_TO_1ST_LVL_TH)
        {
            chg_print_level_message(CHG_MSG_INFO, "System enter to 1st level" \
              " input current limit phase, surf_temp = %d'C.\n", sys_surf_temp);
            new_state = DCM_CUR_LMT_1ST_LVL;
        }
        else
        {
            chg_print_level_message(g_dcm_dbg_msg_level, "System keep staying at normal" \
              " input current limit state.\n");
        }
    }
    /*1st level: we need check if normal resumed, or 2nd level reached.*/
    else if (DCM_CUR_LMT_1ST_LVL == cur_state)
    {
        if (sys_surf_temp >= DCM_CUR_LMT_ENTER_TO_2ND_LVL_TH)
        {
            chg_print_level_message(CHG_MSG_INFO, "System enter to 2nd level" \
              " input current limit phase, surf_temp = %d'C.\n", sys_surf_temp);
            new_state = DCM_CUR_LMT_2ND_LVL;
        }
        else if (sys_surf_temp < DCM_CUR_LMT_BACK_TO_NORMAL_TH)
        {
            chg_print_level_message(CHG_MSG_INFO, "System resume from 1st level" \
              " input current limit phase, enter to normal, surf_temp = %d.\n", 
              sys_surf_temp);
            new_state = DCM_CUR_LMT_NORMAL;
        }
        else
        {
            chg_print_level_message(g_dcm_dbg_msg_level, "System keep staying at 1st level" \
              " input current limit state.\n");
        }
    }
    else if (DCM_CUR_LMT_2ND_LVL == cur_state)
    {
        if (sys_surf_temp < DCM_CUR_LMT_BACK_TO_1ST_LVL_TH)
        {
            chg_print_level_message(CHG_MSG_INFO, "System resume from 2nd level" \
              " input current limit phase, enter to 1st level, surf_temp = %d.\n", 
              sys_surf_temp);
            new_state = DCM_CUR_LMT_1ST_LVL;
        }
        else
        {
            chg_print_level_message(g_dcm_dbg_msg_level, "System keep staying at 2nd level" \
              " input current limit state.\n");
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "Invalid input current limit " \
                                "state %d.\n", cur_state);
    }

    /*new_state updated, and different from current state.*/
    if ((DCM_CUR_LMT_INITIAL != new_state) && (new_state != cur_state))
    {
        mlog_print(MLOG_CHG, mlog_lv_info, "CHG_DCM: System enter to %d state, " \
                  "TSurf_sys = %d.\n", new_state, sys_surf_temp);
        chg_dcm_perform_input_current_limit(new_state);
        cur_state = new_state;
    }

    /*Update the input current limit if chgr type changed.*/
    if (cur_chgr != chg_stm_get_chgr_type())
    {
        cur_chgr = chg_stm_get_chgr_type();
        chg_print_level_message(CHG_MSG_INFO, "New chgr type = %d, " \
                                "Update input current limit.\n", cur_chgr);
        chg_dcm_perform_input_current_limit(cur_state);
    }
}
#endif


void chg_temp_is_too_hot_or_too_cold_for_chg ( void )
{
    static uint32_t up_over_temp_flag = FALSE;
    static uint32_t low_over_temp_flag = FALSE;

    /*����±���ʹ�ܹر�*/
    if(0 == CHG_TEMP_PROTECT_ENABLE)
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:CHG_TEMP_PROTECT is disable!\n");
        chg_temp_protect_flag = FALSE;
        return;
    }
    else
    {
         chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:CHG_TEMP_PROTECT is enable!\n");
    }

    /*���¼��:���������λʱ���Գ���¶����쳣�ж�*/
    if (FALSE == chg_is_charger_present())
    {
        up_over_temp_flag = FALSE;
    }
    else if(FALSE == up_over_temp_flag)
    {   /*1�������³�繦�ܲ�Ʒͨ��NV50385������¶���������Ϊ55��
          2���������³�繦�ܲ�Ʒͨ��NV50385������¶���������Ϊ45��*/
        if(CHG_OVER_TEMP_STOP_THRESHOLD <= chg_real_info.battery_temp)
        {
            up_over_temp_flag = TRUE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery temp don't up_over_temp!\n");
        }
    }
    else
    {
          /*1�������³�繦�ܲ�Ʒͨ��NV50385������¶���������Ϊ52��
          2���������³�繦�ܲ�Ʒͨ��NV50385������¶���������Ϊ42��*/
        if(CHG_OVER_TEMP_RESUME_THRESHOLD > chg_real_info.battery_temp)
        {
            up_over_temp_flag = FALSE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery up temp don't resume nomal!\n");
        }
    }

    /*���¼��:���������λʱ���Գ���¶����쳣�ж�*/
    if (FALSE == chg_is_charger_present())
    {
        low_over_temp_flag = FALSE;
    }
    else if(FALSE == low_over_temp_flag)
    {
        /*��һ��������¶�С��0�ȵ���ͣ��*/
        if(CHG_LOW_TEMP_STOP_THRESHOLD > chg_real_info.battery_temp)
        {
            low_over_temp_flag = TRUE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery temp don't low_over_temp!\n");
        }
    }
    else
    {
        /*��һ��������¶ȴ��ڵ���3�Ȼָ����³��*/
        if(CHG_LOW_TEMP_RESUME_THRESHOLD <= chg_real_info.battery_temp)
        {
            low_over_temp_flag = FALSE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery low temp don't resume nomal!\n");
        }
    }

    /*���ݸߵ��¼�����жϳ���±���״̬*/
    if((up_over_temp_flag == TRUE) || (low_over_temp_flag == TRUE))
    {
        chg_temp_protect_flag = TRUE;
    }
    else
    {
        chg_temp_protect_flag = FALSE;
    }
}

void chg_set_supply_current_by_temp(void)
{
    uint8_t need_resume_supply = 0;
    uint8_t curr_limit;

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_set_supply_current_by_temp check begin!\n");
    /*֧�ֿ�ά�ɲ�*/
    if( TRUE == chg_is_ftm_mode() \
        || TRUE == chg_is_no_battery_powerup_enable()
        || (0 == CHG_TEMP_PROTECT_ENABLE))
    {
        return;
    }

    if( TRUE == chg_is_powdown_charging())/*�ػ����ģʽ*/
    {
        return;
    }

    /*USB�˿��¶ȱ������������õ�ǰ״̬ΪFALSE�����ǰ��������ͻ��*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    if( TRUE == chg_get_usb_temp_protect_stat() )
    {
        if(TRUE == chg_limit_supply_current_flag)
        {
            chg_limit_supply_current_flag = FALSE;
        }
        return;
    }
#endif
    /*������ͱ����������������õ�ǰ״̬ΪINITIAL�����ǰ��������ͻ��*/
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    if(TRUE == chg_battery_protect_flag)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:In battery protect condition " \
          "not set_supply_current_by_temp!\n");
        chg_limit_supply_current_flag = FALSE;
        return;
    }
#endif

    if(TRUE == is_chg_charger_removed())
    {
        if(TRUE == chg_limit_supply_current_flag)
        {
            chg_limit_supply_current_flag = FALSE;
        }
        return;
    }

    if((chg_real_info.battery_temp >= LIMIT_SUPPLY_CURR_TEMP)
        && (FALSE == chg_limit_supply_current_flag))
    {
        chg_limit_supply_current_flag = TRUE;
    }
    else if((chg_real_info.battery_temp <= LIMIT_SUPPLY_CURR_TEMP - LIMIT_SUPPLY_CURR__RESUME_LEN)
        && (TRUE == chg_limit_supply_current_flag))
    {
        chg_limit_supply_current_flag = FALSE;
        need_resume_supply = 1;
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:resume supply current in high temp!\n");
    }

    if(TRUE == chg_limit_supply_current_flag)
    {
        curr_limit = chg_get_supply_limit();
#if (MBB_CHG_BQ24196 == FEATURE_ON)
        if(BQ24192_IINLIMIT_100 != curr_limit)
        {
            chg_set_supply_limit(CHG_IINPUT_LIMIT_100MA);
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set supply current in high temp!\n");
        }
#elif (MBB_CHG_SMB1351 == FEATURE_ON)
        if(SMB1351_DCIN_CUR_LIMIT_ST_500MA != curr_limit)
        {
            chg_set_supply_limit(CHG_IINPUT_LIMIT_500MA);
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set supply current in high temp!\n");
        }

#endif/*MBB_CHG_SMB1351 == FEATURE_ON*/
    }
    else
    {
        if(need_resume_supply)
        {
            chg_set_supply_limit_by_stm_stat();
        }
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_set_supply_current_by_temp check finish!\n");
}

#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )

void chg_set_usb_stat_by_usb_temp(void)
{
    int32_t         usb_temp = TEMP_INITIAL_VALUE;
    int32_t         usb_temp_detect_count_t = USB_TEMP_DETECT_COUNT;

    /*��ȡ��ǰ�¶�ֵ*/
    usb_temp = chg_get_temp_value(CHG_PARAMETER__USB_PORT_TEMP_DEGC);
    chg_usb_temp_info.usb_cur_temp = usb_temp;

    /*����±���ʹ�ܹر�*/
    if(0 == CHG_TEMP_PROTECT_ENABLE)
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:CHG_TEMP_PROTECT is disable!\n");
        chg_usb_temp_info.usb_temp_protect_cur_stat = FALSE;
        return;
    }

    /*�ж������¶�ֵ�Ƿ���ϴ����ͻָ����ޣ�������Ӧ����.*/
    if( FALSE == chg_usb_temp_info.usb_temp_protect_cur_stat)
    {
        while(--usb_temp_detect_count_t)
        {
            if( CHG_USB_TEMP_LIMIT > usb_temp )
            {
                break;
            }
            usb_temp = chg_get_temp_value(CHG_PARAMETER__USB_PORT_TEMP_DEGC);
        }
        if( 0 == usb_temp_detect_count_t )
        {
            chg_usb_temp_info.usb_temp_protect_cur_stat = TRUE;
        }
    }
    else
    {
        while(--usb_temp_detect_count_t)
        {
            if( CHG_USB_TEMP_RESUME < usb_temp )
            {
                break;
            }
            usb_temp = chg_get_temp_value(CHG_PARAMETER__USB_PORT_TEMP_DEGC);
        }
        if( 0 == usb_temp_detect_count_t )
        {
            chg_usb_temp_info.usb_temp_protect_cur_stat = FALSE;
        }
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:usb_cur_temp=%d\n",chg_usb_temp_info.usb_cur_temp);
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:usb_temp_protect_stat=%d\n",
            chg_usb_temp_info.usb_temp_protect_cur_stat);
    /*�жϵ�ǰ����״̬,������Ӧ����*/
    if( chg_usb_temp_info.usb_temp_protect_cur_stat != chg_usb_temp_info.usb_temp_protect_pre_stat )
    {
        if( TRUE == chg_usb_temp_info.usb_temp_protect_cur_stat )
        {
            /*1.����������2.���ó��ICΪSUSPEND��3.����POWER_SUPPLY�ڵ㡣4.mlog��*/
            wake_lock(&chg_usb_temp_wake_lock);
            if ( TRUE != chg_set_suspend_mode(TRUE) )
            {
                wake_unlock(&chg_usb_temp_wake_lock);
                chg_usb_temp_info.usb_temp_protect_cur_stat = FALSE;
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Set charge IC suspend is fail in usb high temp!\n");
                mlog_print(MLOG_CHG, mlog_lv_error, "CHG_DCM: Set charge IC suspend is fail in usb temp %d.\n",
                    chg_usb_temp_info.usb_cur_temp);
                return;
            }
            chg_stm_state_info.usb_heath_type = POWER_SUPPLY_USB_TEMP_DEAD;
            chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)USB_TEMP_HIGH);
            chg_usb_temp_info.usb_temp_protect_pre_stat = chg_usb_temp_info.usb_temp_protect_cur_stat;
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Set charge IC suspend in usb high temp!\n");
#if ( FEATURE_ON == MBB_MLOG )
            printk(KERN_ERR "CHG_STM:USBPortOverTempCnt\n");
            mlog_set_statis_info("USBPortOverTempCnt",1);    /*USB�±�SUSPEND����+1*/
#endif/*MBB_MLOG*/
            mlog_print(MLOG_CHG, mlog_lv_error, "CHG_DCM: Entry usb temp protect in usb temp %d.\n",
                    chg_usb_temp_info.usb_cur_temp);
        }
        else
        {
            /*1.���IC�˳�SUSPEND��2.���ݵ�ǰ״̬�ָ����������3.����POWER_SEPPLY�ڵ㡣4.mlog��5.�������������*/
            if ( TRUE != chg_set_suspend_mode(FALSE) )
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:TO resume from charge IC suspend is fail in usb high temp!\n");
                mlog_print(MLOG_CHG, mlog_lv_error, "CHG_DCM: TO resume from suspend is fail in usb temp %d.\n",
                    chg_usb_temp_info.usb_cur_temp);
                return;
            }
            chg_set_supply_limit_by_stm_stat();
            chg_stm_state_info.usb_heath_type = POWER_SUPPLY_USB_TEMP_GOOD;
            chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)USB_TEMP_NORMAL);
            chg_usb_temp_info.usb_temp_protect_pre_stat = chg_usb_temp_info.usb_temp_protect_cur_stat;
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:To resume from charge IC suspend!\n");
            mlog_print(MLOG_CHG, mlog_lv_error, "CHG_DCM:To resume usb temp protect in usb temp %d.\n",
                    chg_usb_temp_info.usb_cur_temp);
            wake_unlock(&chg_usb_temp_wake_lock);
        }
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Set usb port stat by temp finish!\n");
}


int32_t chg_get_usb_health(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_health=%d\n",chg_stm_state_info.bat_heath_type);
    return chg_stm_state_info.usb_heath_type;
}

int32_t chg_get_usb_cur_temp(void)
{
    if( TEMP_INITIAL_VALUE == chg_usb_temp_info.usb_cur_temp )
    {
        chg_usb_temp_info.usb_cur_temp = chg_get_temp_value(CHG_PARAMETER__USB_PORT_TEMP_DEGC);
    }
    return chg_usb_temp_info.usb_cur_temp;
}

boolean chg_get_usb_temp_protect_stat(void)
{
    return chg_usb_temp_info.usb_temp_protect_cur_stat;
}

void chg_test_set_usb_temp_limit_and_resume(int32_t limit,int32_t resume)
{
    if( resume >= limit )
    {
        return;
    }
    chg_test_usb_temp_limit  = limit;
    chg_test_usb_temp_rusume = resume;
}
#endif

#if ( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )

boolean chg_temp_protect_exit_suspend_mode(void)
{
    boolean ret = TRUE;
    /*�Ƿ������˳�suspend����*/
    chg_print_level_message(CHG_MSG_DEBUG, \
        "CHG_STM:chg_temp_protect_exit_suspend_mode chg_battery_protect_flag is %d!\n", chg_battery_protect_flag);
    chg_print_level_message(CHG_MSG_DEBUG, \
        "CHG_STM:chg_usb_temp_info.usb_temp_protect_cur_stat chg_battery_protect_flag is %d!\n",\
        chg_usb_temp_info.usb_temp_protect_cur_stat);

    if ( (FALSE == chg_battery_protect_flag) && (FALSE == chg_usb_temp_info.usb_temp_protect_cur_stat) )
    {
        ret = chg_set_suspend_mode(FALSE);
    }

    return ret;
}


static boolean chg_battery_protect_enter_process(void)
{
#if defined(BSP_CONFIG_BOARD_E5_DC04)
    /*��֧��suspend��ʹ��ǰ������100MA*/
    if (TRUE != chg_set_supply_limit(CHG_IINPUT_LIMIT_100MA))
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Set supply current 100MA is fail in battery protect!\n");
        return FALSE;
    }
#else
    /*����charger 24196 ����suspendģʽ*/
    if (FALSE == chg_get_suspend_status())
    {
        if( TRUE != chg_set_suspend_mode(TRUE) )
        {
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Set ChargeIC suspend is fail in battery protect!\n");
            return FALSE;
        }
    }
#endif
    return TRUE;
}


static void chg_battery_protect_exit_process(void)
{
#if defined(BSP_CONFIG_BOARD_E5_DC04)
    /*��֧��suspend�Ļָ�ǰ������100MA*/
    chg_set_supply_limit_by_stm_stat();
#else
    /*�˳�suspendģʽ*/
    if (TRUE == chg_get_suspend_status())
    {
        chg_temp_protect_exit_suspend_mode();
    }
#endif
}


static void chg_poll_volt_temp_protect_state(void)
{
    chg_chgr_type_t cur_chgr_type = CHG_CHGR_INVALID;
    cur_chgr_type = chg_stm_get_chgr_type();

    /*������ͱ���TBAT>=45��and VBAT >=4.1V ״̬��ѯ*/
    if ((FALSE == g_chg_over_temp_volt_protect_flag)
        && (chg_get_sys_batt_temp () >= CHG_BATTERY_PROTECT_TEMP)
        && (chg_get_sys_batt_volt () >= CHG_BATTERY_PROTECT_VOLTAGE))
    {
        g_chg_over_temp_volt_protect_flag = TRUE;
        /*���¸�ѹ����������ͱ���+1*/
        mlog_set_statis_info("BattExpandOverTempVoltCnt",1);
    }
    else if ((TRUE == g_chg_over_temp_volt_protect_flag)
            && ((chg_get_sys_batt_temp () < CHG_BATTERY_PROTECT_RESUME_TEMP)
            || (chg_get_sys_batt_volt () < CHG_BATTERY_PROTECT_RESUME_VOLTAGE)))
    {
        g_chg_over_temp_volt_protect_flag = FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:g_chg_over_temp_volt_protect_flag is %d!\n",
            g_chg_over_temp_volt_protect_flag);
}


uint32_t chg_stm_get_no_charging_charger_lasted_time(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:charger_lasted_without_charging_in_seconds=%u\n",\
                    chg_stm_state_info.charger_lasted_without_charging_in_seconds);
    /*1. Return the current time.*/
    return chg_stm_state_info.charger_lasted_without_charging_in_seconds;
}


void set_long_time_no_charge_protect_recharge_volt(void)
{
    /*��ʱ���Դ��λ�������뱣�����޸ĳ��¸�������Ϊ4.0v*/
    g_chgBattVoltProtect.battNormalTempRechergeThreshold = BATT_NORMAL_TEMP_RECHARGE_THR_LONG_TIME_NO_CHARGE;
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    g_chg_batt_expand_change_normal_rechg_flag = TRUE;
#endif
}


void resume_long_time_no_charge_protect_recharge_volt(void)
{
    /*��Դ�γ��󣬻ָ����¸�������Ϊ4.2v*/
    g_chgBattVoltProtect.battNormalTempRechergeThreshold = g_batt_normal_temp_recherge_threshold;
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    g_chg_batt_expand_change_normal_rechg_flag = FALSE;
#endif
}


static void chg_poll_long_time_no_charge_state(void)
{
    uint32_t charger_lasted_time = 0;
    chg_chgr_type_t cur_chgr_type = CHG_CHGR_INVALID;
    cur_chgr_type = chg_stm_get_chgr_type();
    /*ͣ���ҳ������λ��ʱ��*/
    charger_lasted_time = chg_stm_get_no_charging_charger_lasted_time();

    /*������ͱ����������λ����>=16H�Ҳ����״̬��ѯ*/
    if ((FALSE == g_chg_longtime_nocharge_protect_flag)
        && (charger_lasted_time >= CHG_BATTERY_PROTECT_CHGER_TIME_THRESHOLD_IN_SECONDS)
        && (chg_get_sys_batt_volt () >= CHG_BATTERY_PROTECT_VOLTAGE))
    {
        g_chg_longtime_nocharge_protect_flag = TRUE;
        /*��ʱ���Դ��λ�������뱣�����޸ĳ��¸�������Ϊ4.0v*/
        (void)set_long_time_no_charge_protect_recharge_volt();
        /*USB��ʱ����λ����������ͱ���+1*/
        mlog_set_statis_info("BattExpandLongTimeCnt",1);
    }
    else if ((TRUE == g_chg_longtime_nocharge_protect_flag)
            && (chg_get_sys_batt_volt () < CHG_BATTERY_PROTECT_RESUME_VOLTAGE))
    {
        g_chg_longtime_nocharge_protect_flag = FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:g_chg_longtime_nocharge_protect_flag is %d!\n",
            g_chg_longtime_nocharge_protect_flag);
}


static void chg_battery_protect_proc(void)
{
    chg_chgr_type_t cur_chgr_type = CHG_CHGR_INVALID;
    chg_stm_state_type curr_state = CHG_STM_INIT_ST;
    cur_chgr_type = chg_stm_get_chgr_type();
    curr_state = chg_stm_get_cur_state();
    /*֧�ֿ�ά�ɲ�*/
    if ( (TRUE == chg_is_ftm_mode() ) \
        || (TRUE == chg_is_no_battery_powerup_enable() ) \
        || (0 == CHG_TEMP_PROTECT_ENABLE))
    {
        chg_battery_protect_flag = FALSE;
        g_chg_over_temp_volt_protect_flag = FALSE;
        g_chg_longtime_nocharge_protect_flag = FALSE;
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
        g_chg_batt_expand_change_normal_rechg_flag = FALSE;
#endif
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:In FTM mode or TEMP PROTECT DISABLE do not enable battery protect!\n");
        return;
    }

    /*��Դ�γ����˳�������ͱ���*/
    if (TRUE == is_chg_charger_removed())
    {
        if (TRUE == chg_battery_protect_flag)
        {
            chg_battery_protect_flag = FALSE;
            g_chg_over_temp_volt_protect_flag = FALSE;
            g_chg_longtime_nocharge_protect_flag = FALSE;
            (void)chg_battery_protect_exit_process();
        }
        (void)resume_long_time_no_charge_protect_recharge_volt();

        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:charger remove disenable battery protect!\n");
        return;
    }

    if (FALSE == chg_battery_protect_flag)
    {
        /*����ͣ���Ž���Suspend/��������100mA*/
        if ( (curr_state != CHG_STM_MAINT_ST) || (BATT_CAPACITY_FULL != chg_real_info.bat_capacity) )
        {
                chg_print_level_message(CHG_MSG_INFO,
                    "CHG_STM:battery is not in full state, no need to protect!\n");
                return;
        }
    }

    /*��ѯ����״̬*/
    (void)chg_poll_volt_temp_protect_state();
    (void)chg_poll_long_time_no_charge_state();

    /*���ֱ�����һ�����������뱣��*/
    if ( (TRUE == g_chg_over_temp_volt_protect_flag)
        || (TRUE == g_chg_longtime_nocharge_protect_flag) )
    {
        if (FALSE == chg_battery_protect_flag)
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:meet battery protect condition do battery protect!\n");
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:g_chg_over_temp_volt_protect_flag is %d!\n",
                g_chg_over_temp_volt_protect_flag);
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:g_chg_longtime_nocharge_protect_flag is %d!\n",
                g_chg_longtime_nocharge_protect_flag);
            if( TRUE != chg_battery_protect_enter_process() )
            {
                return;
            }
            chg_battery_protect_flag = TRUE;
            /*���������ͱ���+1*/
            mlog_set_statis_info("BattExpandProtectCnt",1);
            mlog_print(MLOG_CHG, mlog_lv_error, "CHG_STM: Entry battery expand protect by OverTempVolt:%d LongTime %d.\n",
                    g_chg_over_temp_volt_protect_flag,g_chg_longtime_nocharge_protect_flag);
        }
    }
    else
    {
        if (TRUE == chg_battery_protect_flag)
        {
            chg_battery_protect_flag = FALSE;
            chg_battery_protect_exit_process();
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:To resume from battery protect !\n");
            mlog_print(MLOG_CHG, mlog_lv_error, "CHG_STM: To resume from battery expand protect.\n");
        }
    }
}
#endif


void chg_temp_is_too_hot_or_too_cold_for_shutoff(void)
{
    static uint32_t countNum = 0;
    static uint32_t high_temp_58_flag = FALSE;
    static uint32_t high_timer_flag = FALSE;
    static uint32_t low_timer_flag = FALSE;
    static uint32_t up_over_temp_shutoff_falg = FALSE;
    static uint32_t low_over_temp_shutoff_falg = FALSE;

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_temp_is_too_hot_or_too_cold_for_shutoff check begin!\n");
    /*��Ƭ�汾��������*/
    /*��ά�ɲ�:�޵�ؿ���ʹ��*/
    if( TRUE == chg_is_ftm_mode() \
        || TRUE == chg_is_no_battery_powerup_enable() )
    {
        return;
    }

    /*�������ڿ���״̬�£��¶ȸ��ڵ���(�ػ��¶�-2��)ʱ�ϱ���Ӧ�ã��澯��ʾ*/
    if( FALSE == chg_is_powdown_charging())/*����ģʽ*/
    {
        if(chg_real_info.battery_temp >= (SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_WARN_LEN))
        {
            if(FALSE == high_temp_58_flag)
            {
                chg_batt_high_temp_58_flag = TRUE;
                /*�л���ѯ��ʽ�����½������ѯģʽ*/
                chg_poll_timer_set(FAST_POLL_CYCLE);
                high_temp_58_flag = TRUE;
            }
            else
            {
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP_BATT over 58 degree!\n");
            }
            /*�ϱ�APP�����¸澯��ʾ���ϱ�5�Σ����⿪�������£�APP���������ղ�����Ϣ*/
            if(BATTERY_EVENT_REPORT_TIMES >= countNum)
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send app msg with TEMP_BATT_HIGH \n ");

                chg_batt_temp_state = TEMP_BATT_HIGH;
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_OVERHEAT;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_HIGH);

                mlog_print(MLOG_CHG, mlog_lv_warn, "Battery over-heated WARNING!!\n");
                mlog_print(MLOG_CHG, mlog_lv_info, "Current Battery Info: [vBat]%dmV, [vBat_sys]%dmV, " \
                    "[tBat]%d'C, [tBat_sys]%d'C.\n", chg_real_info.battery_one_volt, chg_real_info.battery_volt,
                    chg_real_info.battery_one_temp, chg_real_info.battery_temp);
            }
            else
            {
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP_BATT_HIGH event report below 5 times!\n");
            }

            countNum++;
        }
        /*�������ڿ���״̬�£��¶ȵ��ڵ���(�ػ��¶�-5��)ʱ�ϱ���Ӧ�ã�ȡ���澯*/
        else if((chg_real_info.battery_temp <= (SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_RESUME_LEN)))
        {
            if(TRUE == high_temp_58_flag)
            {
                chg_batt_high_temp_58_flag = FALSE;
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send app msg with TEMP_BATT_NORMAL \n ");
                chg_batt_temp_state = TEMP_BATT_NORMAL;
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_GOOD;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_NORMAL);
                high_temp_58_flag = FALSE;
                countNum = 0;
            }
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP_BATT_HIGH!\n");
        }
    }
    else
    {
        /*�ػ�״̬��������*/
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System in power down chging do nothing!\n");
    }


    /*�¶ȹػ����------���¼��*/
    if(SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD <= chg_real_info.battery_temp)
    {
        if(TRUE == SHUTOFF_OVER_TEMP_PROTECT_ENABLE)
        {
            if( FALSE == chg_is_powdown_charging() )/*����ģʽ*/
            {
                up_over_temp_shutoff_falg = TRUE;

                /*todo �ϱ�APPִ�и��¹ػ��������԰����ػ���ʽ�ϱ�*/
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send MSG to app for high temp power off  \n ");
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_DEAD;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_KEY, (uint32_t)GPIO_KEY_POWER_OFF);

                mlog_print(MLOG_CHG, mlog_lv_warn, "Battery Over-heated, system down!!!\n");
                mlog_print(MLOG_CHG, mlog_lv_info, "Current Battery Info: [vBat]%dmV, [vBat_sys]%dmV, " \
                    "[tBat]%d'C, [tBat_sys]%d'C.\n", chg_real_info.battery_one_volt, chg_real_info.battery_volt,
                    chg_real_info.battery_one_temp, chg_real_info.battery_temp);

                if(FALSE == high_timer_flag)
                {
                    high_timer_flag = TRUE;

                    /*������ʱ�����ص�����Ϊ�ػ���������ʱʱ��Ϊ45�룻*/
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:begin 45s timer for high temp power off \n ");
                    g_timeout_to_poweroff_reason = DRV_SHUTDOWN_TEMPERATURE_PROTECT;
                    chg_bat_timer_set( OVER_TEMP_SHUTOFF_DURATION, \
                    chg_send_msg_to_main,CHG_TIMEROUT_TO_POWEROFF_EVENT);
                }
                else
                {
                    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Already start power off timer!\n");
                }
            }
            else //�ػ�ģʽ
            {
                /*�ػ�ģʽ�µ��¶ȸ��ڹػ����ޣ���������*/
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:high temp but power down chging do nothing!\n");
            }
        }
        else
        {
             chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Disable high temp protect!\n");
        }
    }

    /*�¶ȹػ����------���¼��*/
    if(SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD >= chg_real_info.battery_temp)
    {
        if(TRUE == SHUTOFF_LOW_TEMP_PROTECT_ENABLE)
        {
            if( FALSE == chg_is_powdown_charging() )/*����ģʽ*/
            {
                low_over_temp_shutoff_falg = TRUE;
                /*�ϱ�APPִ�е��¹ػ�����*/
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send MSG to app for low temp power off  \n ");

                chg_batt_temp_state = TEMP_BATT_LOW;
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_COLD;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_LOW);

                mlog_print(MLOG_CHG, mlog_lv_warn, "Battery too Cold, system down!!!\n");
                mlog_print(MLOG_CHG, mlog_lv_info, "Current Battery Info: [vBat]%dmV, [vBat_sys]%dmV, " \
                    "[tBat]%d'C, [tBat_sys]%d'C.\n", chg_real_info.battery_one_volt, chg_real_info.battery_volt,
                    chg_real_info.battery_one_temp, chg_real_info.battery_temp);

                if(FALSE == low_timer_flag )
                {
                    low_timer_flag = TRUE;
                    /*��ʱ����ʱ25�룬��45��ʱ�䵽��APP��δ�ػ�����ײ��Լ�ִ�йػ�������*/
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:begin 45s timer for low temp power off \n ");
                    g_timeout_to_poweroff_reason = DRV_SHUTDOWN_LOW_TEMP_PROTECT;
                    chg_bat_timer_set( OVER_TEMP_SHUTOFF_DURATION, \
                    chg_send_msg_to_main,CHG_TIMEROUT_TO_POWEROFF_EVENT);

                }
            }
            else
            {
                /*�ػ�ģʽ�µ��¶ȵ��ڹػ����ޣ���������*/
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:low temp but power down chging do nothing!\n");
            }
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Disable low temp protect!\n");
        }
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_temp_is_too_hot_or_too_cold_for_shutoff check over!\n");
}


void chg_poll_batt_temp(void)
{
    int32_t new_one_batt_temp = 0;
    int32_t new_batt_temp = 0;
    static uint32_t init_flag = FALSE;

    /*����ؼ�⴦����*/
    chg_batt_error_detect_temp();

    /*���ò�����ȡ�ӿں�����ȡ��ѹֵ*/
    new_one_batt_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);

    /*���浥�βɼ����¶�ֵ��ȫ�ֱ�����*/
    chg_real_info.battery_one_temp = new_one_batt_temp;

    /*���ƽ���¶ȳ�ʼ��Ϊ�״βɼ�ֵ*/
    if(FALSE == init_flag)
    {
        chg_real_info.battery_temp = chg_real_info.battery_one_temp;
        init_flag = TRUE;
    }
    /*�����¶Ȳ��������Ե��βɼ����¶Ƚ��в�������ͬƽ̨���¶Ȳ���Ҫ��ͬ
      �е�Ҫ�󲹳�ƽ������¶ȣ��е�Ҫ�󲹳�ƽ��ǰ���¶ȣ�9X25������Ե���
      ����¶Ƚ��в��������ɾ���±ߵ�ƽ���¶Ȳ���*/
    new_one_batt_temp = chg_huawei_set_temp(new_one_batt_temp);

    /*����ƽ���㷨�Ե�ǰ����¶Ƚ���ƽ������*/
    new_batt_temp = chg_calc_average_temp_value(new_one_batt_temp);

    /*�����¶Ȳ����������ж�ƽ������¶Ƚ��в���*/
    //new_batt_temp = chg_huawei_set_temp(new_batt_temp);

    /*�������ջ�ȡ���¶�ֵ��ȫ�ֱ�����*/
    chg_real_info.battery_temp = new_batt_temp;

#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
    bsp_hkadc_dcm_set_battery_temp(new_batt_temp);

    /*We'll check connectivity of surf sensor at 
     chg_dcm_check_input_current_limit_state, once we know it's not
     connected, polling to its temperature is meaningless.*/
    if (SURF_SENSOR_DISCONNECTED != g_surf_temp_state.is_connected)
    {
        /*��ӶԱ�����¶ȵ�ƽ������*/
        g_surf_temp_state.cur_surf_temp = 
          chg_get_temp_value(CHG_PARAMETER__SURFACE_THERM_DEGC);
        g_surf_temp_state.sys_surf_temp = 
          chg_calc_surf_avg_temp(g_surf_temp_state.cur_surf_temp);
    }

    chg_dcm_check_input_current_limit();
#endif

    /*���ó���±������¶ȼ�⺯��ִ��*/
    chg_temp_is_too_hot_or_too_cold_for_chg ( );

    /* log��ӡ�ӿڣ���ά�ɲ���������*/

    chg_print_level_message(CHG_MSG_DEBUG, "**********CHG_STM: chg_poll_batt_temp  begin *********\n");

    /*��ӡ����±���ʹ�ܿ���*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg temp protect enable flag = %d\n", CHG_TEMP_PROTECT_ENABLE);

    /*�ߡ������±������ͣ������*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_high_thr = %d,chg_low_thr = %d\n", \
        CHG_OVER_TEMP_STOP_THRESHOLD, CHG_LOW_TEMP_STOP_THRESHOLD);

    /*�ߡ������±�����縴������*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:rechg_high_thr = %d,rechg_low_thr = %d\n", \
        CHG_OVER_TEMP_RESUME_THRESHOLD, CHG_LOW_TEMP_RESUME_THRESHOLD);

    /*��ӡ��/���±����ػ�ʹ�ܿ��أ�*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:shutoff over temp protect enable flag = %d\n",
                            SHUTOFF_OVER_TEMP_PROTECT_ENABLE);
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:shutoff low  temp protect enable flag = %d\n",
                            SHUTOFF_LOW_TEMP_PROTECT_ENABLE);

    /*�ߡ������±����ػ�����*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:shutoff_high_thr = %d,shutoff_low_thr = %d\n", \
        SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD, SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD);

    /*��ӡ���ε���¶Ⱥʹ����ĵ���¶�*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.battery_one_temp = %d\n", chg_real_info.battery_one_temp);
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.battery_temp = %d\n", chg_real_info.battery_temp);

    /*��ӡ��ǰ���±���״̬chg_temp_protect_flag��ֵ*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_temp_protect_flag = %d\n", chg_temp_protect_flag);

    /*�����¶ȹػ��������ִ��*/
    chg_temp_is_too_hot_or_too_cold_for_shutoff( );
#if (MBB_CHG_CURRENT_SUPPLY_LIMIT == FEATURE_ON)
    chg_set_supply_current_by_temp();
#endif/*MBB_CHG_CURRENT_SUPPLY_LIMIT == FEATURE_ON*/
    /*������ͱ�������*/
#if ( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
    chg_battery_protect_proc();
#endif
    /*USB�±�*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    chg_set_usb_stat_by_usb_temp();
#endif
    chg_print_level_message(CHG_MSG_INFO, "**********CHG_STM: chg_poll_batt_temp  end  *********\n");
}

int32_t chg_calc_average_volt_value(int32_t new_data)
{
    int32_t    index = 0;
    int32_t    sum = 0;
    int32_t    bat_volt_avg = 0;
    int32_t    new_poll_mode;
    static int32_t  old_poll_mode = FAST_POLL_CYCLE;
    static uint32_t record_avg_num_fast = 0;
    static uint32_t record_avg_num_slow = 0;
    static int32_t record_value_fast[CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST] = {0};
    static int32_t record_value_slow[CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW] = {0};

    /*��ѯ��ѯģʽ*/
    new_poll_mode = chg_poll_timer_get();

    if(new_poll_mode != old_poll_mode)
    {
        if(FAST_POLL_CYCLE == new_poll_mode)/*������ѯ�л�����ǰ�Ŀ���ѯ*/
        {
            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;index++)
            {
                record_value_fast[index] = chg_real_info.battery_volt;
            }
            record_avg_num_fast = CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;
        }
        else//�ɿ���ѯ�л�������ѯ
        {
            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;index++)
            {
                record_value_slow[index] = chg_real_info.battery_volt;
            }
            record_avg_num_slow = CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;
        }

        old_poll_mode = new_poll_mode;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_calc_average_volt_value new_poll_mode=old_poll_mode!\n");
    }

    if(FAST_POLL_CYCLE == new_poll_mode)//����ѯģʽ
    {
        /*�����е�ǰԪ�ر��С��30*/
        if(CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST > record_avg_num_fast)
        {
            record_value_fast[record_avg_num_fast] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < record_avg_num_fast; index++)
            {
                sum += record_value_fast[index];
            }

            bat_volt_avg = sum / record_avg_num_fast;
        }
        else/*Ԫ�ظ�����Ŵ��ڵ���30��*/
        {
            record_value_fast[record_avg_num_fast % CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST; index++)
            {
                sum += record_value_fast[index];
            }

            bat_volt_avg = sum / CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;

            /*���Ԫ�ظ�������Ǿ�̬���鳤�ȵ�����,������Ԫ�ظ�������Ǿ�̬���鳤�ȼ�CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST * 2 == record_avg_num_fast)
            {
                record_avg_num_fast = CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;
            }
        }
    }
    else//����ѯģʽ
    {
        /*�����е�ǰԪ�ر��С��5*/
        if(CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW > record_avg_num_slow)
        {
            record_value_slow[record_avg_num_slow] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < record_avg_num_slow; index++)
            {
                sum += record_value_slow[index];
            }

            bat_volt_avg = sum / record_avg_num_slow;
        }
        else/*Ԫ�ظ�����Ŵ��ڵ���5��*/
        {
            record_value_slow[record_avg_num_slow % CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW; index++)
            {
                sum += record_value_slow[index];
            }

            bat_volt_avg = sum / CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;

            /*���Ԫ�ظ�������Ǿ�̬���鳤�ȵ�����,������Ԫ�ظ�������Ǿ�̬���鳤�ȼ�CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW * 2 == record_avg_num_slow)
            {
                record_avg_num_slow = CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;
            }
        }

    }

    return bat_volt_avg;
}


int32_t chg_calc_average_vph_pwr_volt_value(int32_t new_data)
{
    int32_t    index = 0;
    int32_t    sum = 0;
    int32_t    vph_pwr_volt_avg = 0;
    int32_t    new_poll_mode;
    static int32_t  old_poll_mode = FAST_POLL_CYCLE;
    static uint32_t record_avg_num_fast = 0;
    static uint32_t record_avg_num_slow = 0;
    static int32_t record_value_fast[CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST] = {0};
    static int32_t record_value_slow[CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW] = {0};

    /*��ѯ��ѯģʽ*/
    new_poll_mode = chg_poll_timer_get();

    if(new_poll_mode != old_poll_mode)
    {
        if(FAST_POLL_CYCLE == new_poll_mode)/*������ѯ�л�����ǰ�Ŀ���ѯ*/
        {
            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;index++)
            {
                record_value_fast[index] = chg_real_info.battery_volt;
            }
            record_avg_num_fast = CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;
        }
        else//�ɿ���ѯ�л�������ѯ
        {
            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;index++)
            {
                record_value_slow[index] = chg_real_info.battery_volt;
            }
            record_avg_num_slow = CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;
        }

        old_poll_mode = new_poll_mode;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_calc_average_vph_pwr_volt_value new_poll_mode=old_poll_mode!\n");
    }

    if(FAST_POLL_CYCLE == new_poll_mode)//����ѯģʽ
    {
        /*�����е�ǰԪ�ر��С��30*/
        if(CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST > record_avg_num_fast)
        {
            record_value_fast[record_avg_num_fast] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < record_avg_num_fast; index++)
            {
                sum += record_value_fast[index];
            }

            vph_pwr_volt_avg = sum / record_avg_num_fast;
        }
        else/*Ԫ�ظ�����Ŵ��ڵ���30��*/
        {
            record_value_fast[record_avg_num_fast % CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST; index++)
            {
                sum += record_value_fast[index];
            }

            vph_pwr_volt_avg = sum / CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;

            /*���Ԫ�ظ�������Ǿ�̬���鳤�ȵ�����,������Ԫ�ظ�������Ǿ�̬���鳤�ȼ�CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST * 2 == record_avg_num_fast)
            {
                record_avg_num_fast = CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;
            }
        }
    }
    else//����ѯģʽ
    {
        /*�����е�ǰԪ�ر��С��5*/
        if(CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW > record_avg_num_slow)
        {
            record_value_slow[record_avg_num_slow] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < record_avg_num_slow; index++)
            {
                sum += record_value_slow[index];
            }

            vph_pwr_volt_avg = sum / record_avg_num_slow;
        }
        else/*Ԫ�ظ�����Ŵ��ڵ���5��*/
        {
            record_value_slow[record_avg_num_slow % CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW; index++)
            {
                sum += record_value_slow[index];
            }

            vph_pwr_volt_avg = sum / CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;

            /*���Ԫ�ظ�������Ǿ�̬���鳤�ȵ�����,������Ԫ�ظ�������Ǿ�̬���鳤�ȼ�CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW * 2 == record_avg_num_slow)
            {
                record_avg_num_slow = CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;
            }
        }

    }

    return vph_pwr_volt_avg;
}

void chg_volt_level_to_capacity(BATT_LEVEL_ENUM bat_volt_level)
{
    chg_stm_state_type cur_stat = chg_stm_get_cur_state();
    switch(bat_volt_level)
    {
        case BATT_LOW_POWER:
        {
            /*δ��糡����������������Ϊ�͵�͵��������ֹ�ϱ��͵������Ӧ���ڳ�糡�����е͵���ʾ*/
            if(FALSE == chg_get_charging_status())
            {
                chg_set_sys_batt_capacity(BATT_CAPACITY_LEVELLOW);
            }
            break;
        }
        case BATT_LEVEL_1:
        {
            chg_set_sys_batt_capacity(BATT_CAPACITY_LEVEL1);
            break;
        }
        case BATT_LEVEL_2:
        {
            chg_set_sys_batt_capacity(BATT_CAPACITY_LEVEL2);
            break;
        }
        case BATT_LEVEL_3:
        {
            chg_set_sys_batt_capacity(BATT_CAPACITY_LEVEL3);
            break;
        }
        case BATT_LEVEL_4:
        {
            /*����ͣ�����õ�ص���Ϊ100*/
            if(CHG_STM_MAINT_ST == cur_stat)
            {
                chg_set_sys_batt_capacity(BATT_CAPACITY_FULL);
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_volt_level_to_capacity-->CHG_STM_MAINT_ST!\n ");
            }
            else
            {
                /*������ͣ�䵫��ص�ѹ����4.1VҲ���õ�ص���Ϊ100*/
                if(chg_get_sys_batt_volt() >= BATT_CHG_TEMP_MAINT_THR)
                {
                    chg_set_sys_batt_capacity(BATT_CAPACITY_FULL);
                    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_volt_level_to_capacity VBAT>=4.1V!\n ");
                }
                else
                {
                    chg_set_sys_batt_capacity(BATT_CAPACITY_LEVEL4);
                }
            }

            break;
        }

        default:
            break;
    }

}

#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)

BATT_LEVEL_ENUM chg_soc2level(int soc)
{
    BATT_LEVEL_ENUM volt_level;

    if ( (soc > BATT_CAPACITY_FULL) || (soc < 0) )
    {
        chg_print_level_message(CHG_MSG_ERR,\
            "chg_percent2level param %d error!! not in [0 ~ 100]\n ", soc);
    }

    if (soc < BATT_CAPACITY_LEVEL1)
    {
        volt_level = BATT_LOW_POWER;
    }
    else if (soc < BATT_CAPACITY_LEVEL2)
    {
        volt_level = BATT_LEVEL_1;
    }
    else if (soc < BATT_CAPACITY_LEVEL3)
    {
        volt_level = BATT_LEVEL_2;
    }
    else if (soc < BATT_CAPACITY_LEVEL4)
    {
        volt_level = BATT_LEVEL_3;
    }
    else
    {
        volt_level = BATT_LEVEL_4;
    }

    return volt_level;
}

void chg_set_battery_level(void)
{
    int soc = 0;
    BATT_LEVEL_ENUM bat_volt_level = BATT_LEVEL_MAX;
    BATT_LEVEL_ENUM pre_bat_level = BATT_LEVEL_MAX;
    soc = chg_get_sys_batt_capacity();
    bat_volt_level = chg_soc2level(soc);
    pre_bat_level = chg_real_info.bat_volt_lvl;

   /*�ǳ��״̬���������ص�ѹ��ת*/
    if(FALSE == chg_get_charging_status())
    {
        if(bat_volt_level <= chg_real_info.bat_volt_lvl )
        {
            chg_real_info.bat_volt_lvl = bat_volt_level;
            chg_print_level_message(CHG_MSG_DEBUG,\
            "CHG_STM:NO chargin state volt_lvl decline to %d!\n ",bat_volt_level);
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,\
            "CHG_STM:NO chargin state only allow volt_lvl decline!\n ");
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG,\
        "CHG_STM:chargin state volt_lvl increase to %d!\n ",bat_volt_level);
        chg_real_info.bat_volt_lvl = bat_volt_level;
    }

    if(BATT_LOW_POWER == chg_real_info.bat_volt_lvl)
    {
        chg_batt_low_battery_flag = TRUE;
        if(FAST_POLL_CYCLE != chg_poll_timer_get())
        {
            chg_poll_timer_set(FAST_POLL_CYCLE);
        }
    }

    if(pre_bat_level != chg_real_info.bat_volt_lvl)
    {
        chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY,\
                            (uint32_t)CHG_EVENT_NONEED_CARE);
    }

}
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/

void chg_set_battery_volt_level(void)
{
    static uint32_t count = 0;
    BATT_LEVEL_ENUM bat_volt_level = BATT_LEVEL_MAX;
    int32_t batt_volt = chg_real_info.battery_volt;

    if(batt_volt < BATT_VOLT_LEVELLOW_MAX)
    {

#if (MBB_CHG_EXTCHG == FEATURE_ON)
        /*����������λ���û�ѡ���˽���������߶����������ҵ�����ϱ��͵�*/
        if(g_ui_choose_exchg_mode <= 0)
        {
            bat_volt_level = BATT_LOW_POWER;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:EXTCHG do not report Lowpower!\n");
        }
#else
        bat_volt_level = BATT_LOW_POWER;
#endif /*MBB_CHG_EXTCHG == FEATURE_ON*/
    }

    else if(batt_volt < BATT_VOLT_LEVEL1_MAX)
    {
        bat_volt_level = BATT_LEVEL_1;
    }
    else if(batt_volt < BATT_VOLT_LEVEL2_MAX)
    {
        bat_volt_level = BATT_LEVEL_2;
    }
    else if(batt_volt < BATT_VOLT_LEVEL3_MAX)
    {
        bat_volt_level = BATT_LEVEL_3;
    }
    else
    {
        bat_volt_level = BATT_LEVEL_4;
    }

   /*�ǳ��״̬���������ص�ѹ��ת*/
    if(FALSE == chg_get_charging_status())
    {
        if(bat_volt_level <= chg_real_info.bat_volt_lvl )
        {
            chg_real_info.bat_volt_lvl = bat_volt_level;
            chg_volt_level_to_capacity(bat_volt_level);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NO chargin state volt_lvl decline to LEVEL_%d!\n ",bat_volt_level);
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NO chargin state only allow volt_lvl decline!\n ");
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chargin state volt_lvl increase to LEVEL_%d!\n ",bat_volt_level);
        chg_real_info.bat_volt_lvl = bat_volt_level;
        chg_volt_level_to_capacity(bat_volt_level);
    }

    /*�͵�ǳ��,�ҷǹػ����״̬�£��ϱ�APP�͵��¼�*/
    if((BATT_LOW_POWER == chg_real_info.bat_volt_lvl) \
       && (FALSE == chg_is_powdown_charging()) \
       && (FALSE == chg_get_charging_status()))
    {
        /* �ϱ�5�� */
        if(BATTERY_EVENT_REPORT_TIMES > count)
        {
            /*���ýӿں����ϱ�APP�͵��¼�*/
            chg_batt_low_battery_flag = TRUE;
            chg_poll_timer_set(FAST_POLL_CYCLE);
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send MSG to app for show low power \n ");
            chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)BAT_LOW_POWER);
            count++;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:low power event report less than 5 times!\n");
        }
    }
    else
    {
        count = 0;
    }
}


void chg_detect_batt_volt_for_shutoff(void)
{
    static uint32_t timer_flag = FALSE;

    /*��Ƭ�汾��������*/
    /*��ά�ɲ�:�͵�ػ�/�޵�ؿ���ʹ��*/
    if( TRUE == chg_is_ftm_mode() \
        || TRUE == chg_is_low_battery_poweroff_disable() \
        || TRUE == chg_is_no_battery_powerup_enable() )
    {
        return;
    }
    /*��ص͵�ػ����*/
    if(BATT_VOLT_POWER_OFF_THR > chg_real_info.battery_volt)
    {
        chg_print_level_message(CHG_MSG_INFO,"CHG_STM:PowerOffThreshold=%d\n",BATT_VOLT_POWER_OFF_THR);
        if( FALSE == chg_is_powdown_charging() )/*����ģʽ*/
        {
            /*todo �ϱ�APPִ�е͵�ػ�����*/
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send MSG to app for low battery power off  \n ");
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
            /*����֧�ֿ��ؼƵĲ�Ʒ����Ҫ���ݵ�ѹ�������ػ���ͬʱ��Ҫcoul��socֵƽ����2%*/
            if((FALSE == timer_flag) && (BATT_CAPACITY_SHUTOFF == chg_get_sys_batt_capacity()))
            {
                timer_flag = TRUE;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)BAT_LOW_POWEROFF);
                /*������ʱ�����ص�����Ϊ�ػ���������ʱʱ��Ϊ45�룻*/
                chg_print_level_message(CHG_MSG_ERR, \
                "CHG_STM:begin 45s timer for low battery power off \n ");
                g_timeout_to_poweroff_reason = DRV_SHUTDOWN_LOW_BATTERY;
                chg_bat_timer_set( LOW_BATT_SHUTOFF_DURATION, \
                chg_send_msg_to_main,CHG_TIMEROUT_TO_POWEROFF_EVENT);
            }
#else

            /*����ģʽ��ص�ѹ����3.45V��Ӧ���ϱ�����Ϊ0���ػ����ģʽ���ϱ���ֹ�������ػ�*/
            chg_set_sys_batt_capacity(BATT_CAPACITY_SHUTOFF);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)BAT_LOW_POWEROFF);
#ifdef  MBB_MLOG
            mlog_print(MLOG_CHG, mlog_lv_warn, "Battery volt too low, report system down message!!!\n");
#endif/*MBB_MLOG*/
            if(FALSE == timer_flag)
            {
                timer_flag = TRUE;
                /*������ʱ�����ص�����Ϊ�ػ���������ʱʱ��Ϊ45�룻*/
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:begin 45s timer for low battery power off \n ");
#ifdef  MBB_MLOG
                mlog_print(MLOG_CHG, mlog_lv_warn, "CHG_STM:begin 45s timer for low battery power off!!!\n");
#endif/*MBB_MLOG*/
                g_timeout_to_poweroff_reason = DRV_SHUTDOWN_LOW_BATTERY;
                chg_bat_timer_set( LOW_BATT_SHUTOFF_DURATION, \
                chg_send_msg_to_main,CHG_TIMEROUT_TO_POWEROFF_EVENT);
            }
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
        }
        else/*�ػ�ģʽ*/
        {
            /*�ػ�ģʽ�µĵ�ѹ���ڹػ����ޣ�������������Ϊ�ػ�״̬*/
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:power off voltage but in powdown_charging do nothing!\n");
        }
    }
}


void chg_detect_batt_chg_for_shutoff(void)
{
    static int32_t charge_remove_check_count = 0;

    if(TRUE == chg_is_powdown_charging())
    {
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        /*�����߳�繦��ʱ���ػ��������������μ�⵽���߳���������߳����������λ��ػ�*/
        if((FALSE == chg_is_charger_present()) && (FALSE == chg_stm_get_wireless_online_st()))
#else
        /*�����߳�繦��ʱ���ػ��������������μ�⵽���߳������߳����������λ��ػ�*/
        if(FALSE == chg_is_charger_present())
#endif/*MBB_CHG_WIRELESS*/
        {
            if(CHARGE_REMOVE_CHECK_MAX <= charge_remove_check_count)
            {
                chg_print_level_message(CHG_MSG_ERR,"CHG_STM:POWER OFF FOR CHARGER REMOVE !\n ");
                chg_set_power_off(DRV_SHUTDOWN_CHARGE_REMOVE);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO,"CHG_STM:charge_remove_check_count = %d\n ",charge_remove_check_count);
                charge_remove_check_count++;
            }
        }
        else
        {
            charge_remove_check_count = 0;
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM: power on chging do noting!\n");
    }
}


void chg_set_revise_value(int32_t value)
{
    chg_real_info.volt_revise_value = value;
}

int32_t chg_get_revise_value(void)
{
    return chg_real_info.volt_revise_value;
}

#if (MBB_CHG_EXTCHG == FEATURE_ON)

int32_t chg_get_extchg_revise(void)
{
    int32_t revise_val = 0;

    if(chg_get_revise_value() <= 0)
    {
        return revise_val;
    }

    /*�ŵ�粹������*/
    if(chg_get_revise_value() < EXTCHG_BATT_VOLT_REVISE_LIMIT)
    {
        revise_val = chg_get_revise_value();
    }
    else
    {
        revise_val = EXTCHG_BATT_VOLT_REVISE_LIMIT;
    }

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:extchg revise_val = %d\n", revise_val);
    return revise_val;
}

boolean is_need_extchg_revise(void)
{
    static boolean is_need_revise = FALSE;
    int32_t tmp_volt_revise_val = 0;
    int32_t batt_volt_average = chg_get_sys_batt_volt();
    int32_t batt_volt = chg_get_batt_volt_value();

    /*��ض˿ڵ�ѹ����3.35v���ٲ���*/
    if(batt_volt < CHG_BATT_VOLT_REVISE_LIMIT_DOWN)
    {
        batt_volt = chg_get_batt_volt_value();
        if(batt_volt < CHG_BATT_VOLT_REVISE_LIMIT_DOWN)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:batt_volt=%d below 3.35V no need revise!\n", batt_volt);
            g_extchg_revise_count = 0;
            is_need_revise = FALSE;
            return  is_need_revise;
        }
    }
    if((!is_need_revise) || (g_extchg_revise_count < CHG_BATT_VOLT_REVISE_COUNT))
    {
        if(FALSE == g_exchg_enable_flag)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:stop extchg,no need revise!\n");
            g_extchg_revise_count = 0;
            is_need_revise = FALSE;
            return  is_need_revise;
        }
        if((batt_volt_average - batt_volt) > CHG_BATT_VOLT_REVISE_WINDOW)
        {
            batt_volt = chg_get_batt_volt_value();
            if((batt_volt_average - batt_volt) > CHG_BATT_VOLT_REVISE_WINDOW)
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:batt volt change = %d,start ext revise!\n",
                    (batt_volt_average - batt_volt));
                tmp_volt_revise_val = batt_volt_average - batt_volt;
                chg_set_revise_value(tmp_volt_revise_val);
                is_need_revise = TRUE;
            }
        }
        g_extchg_revise_count++ ;
    }
    else
    {
        if(FALSE == g_exchg_enable_flag)
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:stop extchg,no need revise!\n");
            g_extchg_revise_count = 0;
            is_need_revise = FALSE;
        }
        else
        {
            /*����������λ����û����ӱ�����豸�������*/
            if((batt_volt_average - batt_volt) <= CHG_BATT_VOLT_REVISE_WINDOW)
            {
                batt_volt = chg_get_batt_volt_value();
                if((batt_volt_average - batt_volt) <= CHG_BATT_VOLT_REVISE_WINDOW)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:batt volt change = %d,stop ext revise!\n",
                        (batt_volt - batt_volt_average));
                    g_extchg_revise_count = 0;
                    is_need_revise = FALSE;
                }
            }
            /*��͸���*/
            else
            {
                int32_t temp_revise_val = chg_get_revise_value();
                if(abs((batt_volt_average - batt_volt) - temp_revise_val) > CHG_BATT_VOLT_REVISE_WINDOW)
                {
                    batt_volt = chg_get_batt_volt_value();
                    if(abs((batt_volt_average - batt_volt) - temp_revise_val) > CHG_BATT_VOLT_REVISE_WINDOW)
                    {
                        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:extchg batt volt change=%d,dynamic ext revise!\n",
                            (batt_volt_average - batt_volt));
                        tmp_volt_revise_val = batt_volt_average - batt_volt;
                        chg_set_revise_value(tmp_volt_revise_val);

                    }
                }
            }
        }
    }
    return  is_need_revise;
}

#endif/*defined(MBB_CHG_EXTCHG*/


int32_t chg_get_chg_revise(void)
{
    int32_t revise_val = 0;

    if(chg_get_revise_value() <= 0)
    {
        return revise_val;
    }

    /*��粹������*/
    if(chg_get_revise_value() < CHG_BATT_VOLT_REVISE_LIMIT)
    {
        revise_val = 0 - chg_get_revise_value();
    }
    else
    {
        revise_val = 0 - CHG_BATT_VOLT_REVISE_LIMIT;
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg revise_val = %d\n", revise_val);
    return revise_val;
}


boolean is_need_chg_revise(void)
{
    static boolean is_need_revise = FALSE;
    int32_t tmp_volt_revise_val = 0;
    int32_t batt_volt_average = chg_get_sys_batt_volt();
    int32_t batt_volt = chg_get_batt_volt_value();

    /*��ض˿ڵ�ѹ���ڲ����������ٲ���*/
    if(batt_volt > CHG_BATT_VOLT_REVISE_LIMIT_UP)
    {
        batt_volt = chg_get_batt_volt_value();
        if(batt_volt > CHG_BATT_VOLT_REVISE_LIMIT_UP)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:batt_volt = %d\n", batt_volt);
            g_chg_revise_count = 0;
            is_need_revise = FALSE;
            return  is_need_revise;
        }
    }
    if((!is_need_revise) || (g_chg_revise_count < CHG_BATT_VOLT_REVISE_COUNT))
    {
        if(FALSE == chg_get_charging_status())
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:stop charging,no need revise!\n");
            g_chg_revise_count = 0;
            is_need_revise = FALSE;
            return  is_need_revise;
        }
        if((batt_volt - batt_volt_average) > CHG_BATT_VOLT_REVISE_WINDOW)
        {
            batt_volt = chg_get_batt_volt_value();
            if((batt_volt - batt_volt_average) > CHG_BATT_VOLT_REVISE_WINDOW)
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:batt volt change = %d,start revise!\n",
                    (batt_volt - batt_volt_average));
                tmp_volt_revise_val = batt_volt - batt_volt_average;
                chg_set_revise_value(tmp_volt_revise_val);
                is_need_revise = TRUE;
            }
        }
        g_chg_revise_count++ ;
    }
    else
    {
        if(FALSE == chg_get_charging_status())
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:stop charging ,stop revise!\n",
                (batt_volt - batt_volt_average));
            g_chg_revise_count = 0;
            is_need_revise = FALSE;
        }
        else
        {
            int32_t temp_revise_val = chg_get_revise_value ();
            if(abs((batt_volt - batt_volt_average) - temp_revise_val) > CHG_BATT_VOLT_REVISE_WINDOW)
            {
                batt_volt = chg_get_batt_volt_value();
                if(abs((batt_volt - batt_volt_average) - temp_revise_val) > CHG_BATT_VOLT_REVISE_WINDOW)
                {
                    /*��̬���²���ֵ*/
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:batt volt change = %d,dynamic revise!\n",
                    (batt_volt - batt_volt_average));
                    tmp_volt_revise_val = batt_volt - batt_volt_average;
                    chg_set_revise_value(tmp_volt_revise_val);
                }
            }
        }
    }
    return  is_need_revise;
}


int32_t chg_batt_volt_revise(int32_t batt_volt)
{
    int32_t revised_batt_volt = 0;
    int32_t revise_val = 0;
    if(TRUE == is_need_chg_revise())
    {
        revise_val = chg_get_chg_revise();
        revised_batt_volt = batt_volt + revise_val;
        chg_print_level_message(CHG_MSG_DEBUG, "VOLTAGE_COMPENCATE:need chg revise batt_volt_revised=%d\n",revised_batt_volt);
    }
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    else if(TRUE == is_need_extchg_revise())
    {
        revised_batt_volt = batt_volt + chg_get_extchg_revise();
        chg_print_level_message(CHG_MSG_DEBUG, "VOLTAGE_COMPENCATE:need extchg revise batt_volt_revised=%d\n",revised_batt_volt);
    }
#endif /*MBB_CHG_EXTCHG == FEATURE_ON*/
    else
    {
        revised_batt_volt = batt_volt;
        chg_set_revise_value(0);

        chg_print_level_message(CHG_MSG_DEBUG, "VOLTAGE_COMPENCATE:no need revise batt_volt_revised=%d\n",revised_batt_volt);
    }
    chg_print_level_message(CHG_MSG_INFO, "VOLTAGE_COMPENCATE:batt_volt=%d,revised_value=%d,batt_volt_revised=%d,real_revise_val=%d\n",\
                           batt_volt, revise_val, revised_batt_volt, chg_get_revise_value());

    return revised_batt_volt;
}
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
/*�����ɼ�����*/
#if (MBB_CHG_COULOMETER == FEATURE_ON)
/*Hi6421���ÿ��ؼ�֧��10����Ч��������*/
#define CURRENT_RECORD_NUM      (10)
#else
#define CURRENT_RECORD_NUM      (20)
#endif /*(MBB_CHG_COULOMETER == FEATURE_ON)*/

/*�����ɼ����ֵ�߽�ֵ*/
#define CURRENT_LIMIT_UP_MAX    (2000)
/*�����ɼ����ֵ�߽�ֵ*/
#define CURRENT_LIMIT_UP_MIN    (-2000)
/*�����ɼ���Сֵ�߽�ֵ*/
#define CURRENT_LIMIT_DOWN_MAX  (50)
/*�����ɼ���Сֵ�߽�ֵ*/
#define CURRENT_LIMIT_DOWN_MIN  (-50)
/*��λ����*/
#define CURRENT_UNIT_MA2UA      (1000)
/*�����С��Ҫ���ݵ�������ͳ���������*/
#ifdef BSP_CONFIG_BOARD_E5_SB03
#define R_BATT    (149)
#elif defined(BSP_CONFIG_BOARD_E5_DC04)
#define CHARGING_R_BATT    (38)
#define DISCHARGE_R_BATT    (108)
#else
#define R_BATT    (110)
#endif

int32_t chg_get_average_batt_current(void)
{
    int32_t i, used, current_ma, totalcur;

    used = 0;
    totalcur = 0;
    current_ma = 0;
    for (i = 0; i < CURRENT_RECORD_NUM; i++)
    {
        current_ma = bsp_coul_current_before(i);
        if((current_ma > CURRENT_LIMIT_UP_MAX) || (current_ma < CURRENT_LIMIT_UP_MIN))
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:invalid current = %d ma\n", current_ma);
            continue;
        }
        totalcur += current_ma;
        used++;
    }
    if(used > 0)
    {
        current_ma = totalcur / used;
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:current_ma = %d\n", current_ma);
    return current_ma;
}

/*������Ϊ��������ߣ��ŵ����Ϊ���������*/
int32_t chg_batt_volt_coul_revise(int32_t batt_volt)
{
    int32_t current_ma = 0,revise_val = 0;
    int32_t revised_batt_volt = 0;
    chg_stm_state_type cur_stat = chg_stm_get_cur_state();
#if defined(BSP_CONFIG_BOARD_E5_DC04)
    if( TRUE == chg_is_charger_present()
        && (CHG_STM_FAST_CHARGE_ST == cur_stat || CHG_STM_WARMCHG_ST == cur_stat || CHG_STM_HVDCP_CHARGE_ST == cur_stat) )
    {
        /*��������:Ӳ���ṩ�� DC04 R_BATT �̶�ֵ38  ��I������ؼƵĶ�ȡֵ�����з��ţ�*/
        current_ma = hisi_battery_current_avg();
        /*V=Vc+I*R/1000*/
        revised_batt_volt = batt_volt + (current_ma * CHARGING_R_BATT) / 1000;
        chg_print_level_message(CHG_MSG_ERR,
            "charging_vBat[In] = %dmV,        charging_iBat = %dmA.\n", batt_volt, current_ma);
        chg_print_level_message(CHG_MSG_ERR,
            "charging_rBat     = %dmOhm, charging_vBat[Out] = %dmV.\n", CHARGING_R_BATT, revised_batt_volt);
    }
    else
    {
        /*�ŵ�������:Ӳ���ṩ�� DC04 R_BATT �̶�ֵ108  ��I������ؼƵĶ�ȡֵ�����з��ţ�*/
        current_ma = hisi_battery_current_avg();
        /*V=Vc+I*R/1000*/
        revised_batt_volt = batt_volt + (current_ma * DISCHARGE_R_BATT) / 1000;
        chg_print_level_message(CHG_MSG_ERR,
            "discharge_vBat[In] = %dmV,        discharge_iBat = %dmA.\n", batt_volt, current_ma);
        chg_print_level_message(CHG_MSG_ERR,
            "discharge_rBat     = %dmOhm, discharge_vBat[Out] = %dmV.\n", DISCHARGE_R_BATT, revised_batt_volt);
    }
#else
    revised_batt_volt = batt_volt;
    current_ma = chg_get_average_batt_current();
    if((CURRENT_LIMIT_DOWN_MIN < current_ma) && (CURRENT_LIMIT_DOWN_MAX > current_ma))
    {
        return revised_batt_volt;
    }
    revise_val = (current_ma * R_BATT) / CURRENT_UNIT_MA2UA;/*��λת�������ת��ΪmV*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:revise_val = %d\n", revise_val);
    revised_batt_volt = batt_volt + revise_val;
#endif

    return revised_batt_volt;
}
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/


void chg_poll_bat_level(void)
{
    int32_t new_one_batt_volt = 0;
    int32_t new_batt_volt = 0;
    int32_t new_one_vph_pwr_volt = 0;
    int32_t new_avg_vph_pwr_volt = 0;
    static uint32_t init_flag = FALSE;

    /*����ؼ�⴦����*/
    chg_batt_error_detect_volt();

    /*��ȡ���ADC�ɼ���ѹ*/
    new_one_batt_volt = chg_get_batt_volt_value();

#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    new_one_batt_volt = chg_batt_volt_coul_revise(new_one_batt_volt);
#else
    /*�Ե��ε�ѹ�ɼ�ֵ����������У׼*/
    new_one_batt_volt = chg_batt_volt_revise(new_one_batt_volt);
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/

    /*�����ص��βɼ���ѹ��ȫ�ֱ���*/
    chg_real_info.battery_one_volt = new_one_batt_volt;

    /*��ȡVPH_PWR ADC�ɼ���ѹ*/
    new_one_vph_pwr_volt = chg_get_vph_pwr_volt_value();

    /*����VPH_PWR���βɼ���ѹ��ȫ�ֱ���*/
    chg_real_info.vph_pwr_one_volt = new_one_vph_pwr_volt;

    /*���ƽ����ѹ��ʼ��Ϊ�״βɼ�ֵ*/
    if(FALSE == init_flag)
    {
        chg_real_info.battery_volt = chg_real_info.battery_one_volt;
        chg_real_info.vph_pwr_avg_volt = chg_real_info.vph_pwr_one_volt;
        chg_real_info.bat_volt_lvl = BATT_LEVEL_MAX;
        init_flag = TRUE;
    }

    /*�Ե�ص�ѹ����ƽ���㷨����*/
    new_batt_volt = chg_calc_average_volt_value(new_one_batt_volt);

    /*�����ص�ѹƽ��ֵ��ȫ�ֱ���*/
    chg_real_info.battery_volt = new_batt_volt;

    /*��ص�����������*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    chg_set_battery_level();
#else
    chg_set_battery_volt_level();
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
    /*��VPH_PWR��ѹ����ƽ���㷨����*/
    new_avg_vph_pwr_volt = chg_calc_average_vph_pwr_volt_value(new_one_vph_pwr_volt);

    /*����VPH_PWRƽ����ѹ��ȫ�ֱ���*/
    chg_real_info.vph_pwr_avg_volt = new_avg_vph_pwr_volt;

    /* log��ӡ�ӿڣ���ά�ɲ���������*/
    chg_print_level_message(CHG_MSG_DEBUG, "***********CHG_STM: chg_poll_bat_level  begin **********\n");

    /*��ӡ����±���ʹ�ܿ���*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg temp protect enable flag = %d\n", CHG_TEMP_PROTECT_ENABLE);

    /*������ѹ����*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:shutoff_batt_volt_thr= %d\n", BATT_VOLT_POWER_ON_THR);

    /*��ӡ��ص��βɼ���ѹ*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.battery_one_volt = %d\n", chg_real_info.battery_one_volt);
    /*��ӡ��ص�ѹƽ��ֵ*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.battery_volt = %d\n", chg_real_info.battery_volt);
    /*��ӡ��ӡ��ص�������*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.bat_volt_level = %d\n", chg_real_info.bat_volt_lvl);

    /*��ӡVPH_PWR���βɼ���ѹ*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.vph_pwr_one_volt = %d\n", chg_real_info.vph_pwr_one_volt);
    /*��ӡVPH_PWR��ѹƽ��ֵ*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.vph_pwr_avg_volt = %d\n", chg_real_info.vph_pwr_avg_volt);

    /*��������Ƴ��ػ�*/
    chg_detect_batt_chg_for_shutoff();
    chg_detect_batt_volt_for_shutoff();
    chg_print_level_message(CHG_MSG_INFO, "*********** CHG_STM: chg_poll_bat_level  end **********\n");
}
#if (MBB_CHG_COULOMETER == FEATURE_ON)

void chg_poll_batt_soc(void)
{
    int32_t input_soc = 0;
    int32_t last_soc = chg_get_sys_batt_capacity();
    int32_t bat_stat_t = chg_get_bat_status();

    input_soc = smartstar_battery_capacity();
    if(input_soc - last_soc >= 1)
    {
        /*�ǳ��״̬��������ٷֱ�����*/
        if (bat_stat_t == POWER_SUPPLY_STATUS_NOT_CHARGING)
        {
            input_soc = last_soc;
        }
        else
        {
            input_soc = last_soc + 1;
        }
    }
    else if(last_soc - input_soc >= 1)
    {
        input_soc = last_soc - 1;
    }
    else
    {
    }

    chg_set_sys_batt_capacity(input_soc);
}
#endif/*MBB_CHG_COULOMETER == FEATURE_ON*/

#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)

int32_t chg_notify_app_charging_state(int32_t batt_state)
{
    if (NULL == g_chip)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG:NULL pointer,batt_state=%d\n ",batt_state);
        return -1;
    }
    g_chip->bat_stat = batt_state;
    power_supply_changed(&g_chip->bat);
    return 0;
}


void chg_poll_batt_charging_state_for_coul(void)
{
    static CHG_COUL_EVENT_TYPE charge_status = VCHRG_STOP_CHARGING_EVENT;
    chg_stm_state_type cur_stm_state;
    chg_print_level_message(CHG_MSG_DEBUG,"CHG:charging old state=%d\n ",charge_status);
    switch (charge_status)
    {
        case VCHRG_NOT_CHARGING_EVENT:
            if (chg_is_IC_charging())
            {
                hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                charge_status = VCHRG_START_CHARGING_EVENT;
            }
            else
            {
                if(!is_chg_charger_removed())
                {
                     /*IC����ͣ�䣬����soc����100%,��Ҫģ��һ�γ�ͣ����*/
                    /*Ŀǰ״̬������֣�IC�Ѿ�ͣ�䣬���ǳ��״̬������ֹͣ�ڿ����߸��³��״̬��
                      �����Ҫ�������������֪ͨ���ؼ�״̬�����ڶ�Ӧ״̬*/
                    if ( ( (CHG_STM_MAINT_ST == chg_stm_get_cur_state())
                        || (CHG_STM_FAST_CHARGE_ST == chg_stm_get_cur_state())
                        || (CHG_STM_WARMCHG_ST == chg_stm_get_cur_state()) )
                    && (FALSE == chg_is_batt_full()))
                    {
                        chg_notify_app_charging_state(POWER_SUPPLY_STATUS_CHARGING);
                        hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                        charge_status = VCHRG_START_CHARGING_EVENT;
                    }
                }
            }

            break;
        case VCHRG_START_CHARGING_EVENT:
            if (!chg_is_IC_charging())
            {
                if ((CHG_STM_MAINT_ST == chg_stm_get_cur_state())
                || (CHG_STOP_COMPLETE == chg_get_stop_charging_reason()))
                {
                    hisi_coul_charger_event_rcv(VCHRG_CHARGE_DONE_EVENT);
                    charge_status = VCHRG_CHARGE_DONE_EVENT;
                }
                else
                {
                    hisi_coul_charger_event_rcv(VCHRG_STOP_CHARGING_EVENT);
                    charge_status = VCHRG_STOP_CHARGING_EVENT;
                }
            }
            break;
        case VCHRG_STOP_CHARGING_EVENT:
            if (chg_is_IC_charging())
            {
                hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                charge_status = VCHRG_START_CHARGING_EVENT;
            }
            /*Ŀǰ״̬������֣�IC�Ѿ�ͣ�䣬���ǳ��״̬������ֹͣ�ڿ����߸��³��״̬��
              �����Ҫ�������������֪ͨ���ؼ�״̬�����ڶ�Ӧ״̬*/
            else if (!is_chg_charger_removed())
            {
                /*IC����ͣ�䣬����soc����100%,��Ҫģ��һ�γ�ͣ����*/
                if ( ( (CHG_STM_MAINT_ST == chg_stm_get_cur_state()) 
                    || (CHG_STM_FAST_CHARGE_ST == chg_stm_get_cur_state())
                    || (CHG_STM_WARMCHG_ST == chg_stm_get_cur_state()) )
                    && (FALSE == chg_is_batt_full()) )
                {
                    chg_notify_app_charging_state(POWER_SUPPLY_STATUS_CHARGING);
                    hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                    charge_status = VCHRG_START_CHARGING_EVENT;
                }
            }
            else
            {
                hisi_coul_charger_event_rcv(VCHRG_NOT_CHARGING_EVENT);
                charge_status = VCHRG_NOT_CHARGING_EVENT;
            }
            break;
        case VCHRG_CHARGE_DONE_EVENT:
            if(is_chg_charger_removed())
            {
                hisi_coul_charger_event_rcv(VCHRG_NOT_CHARGING_EVENT);
                charge_status = VCHRG_NOT_CHARGING_EVENT;
            }
            else
            {
                if (chg_is_IC_charging())
                {
                    hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                    charge_status = VCHRG_START_CHARGING_EVENT;
                }

                /*IC����ͣ�䣬����soc����100%,��Ҫ��ʾ��綯����ƽ���ٷֱ�*/
                else if ((CHG_STM_MAINT_ST == chg_stm_get_cur_state())
                    && (FALSE == chg_is_batt_full()))
                {
                    chg_notify_app_charging_state(POWER_SUPPLY_STATUS_CHARGING);
                }
            }
            break;
        default:
            break;
    }
    chg_print_level_message(CHG_MSG_DEBUG,"CHG:charging new state=%d\n ",charge_status);
}

#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/

#if (MBB_CHG_COMPENSATE == FEATURE_ON)

int32_t chg_tbat_status_get(void)
{
    int32_t tbat_v = 0;
    int32_t ret = FALSE;
    (void)chg_set_charge_enable(FALSE);
    chg_delay_ms(CHG_DELAY_COUNTER_SIZE);

    /*��ȡ��ص���*/
    tbat_v = chg_get_batt_volt_value();

    /*�ж��Ƿ�����ŵ�򲹵磬������3��*/
    if((tbat_v > TBAT_DISCHG_VOLT) || (tbat_v < TBAT_SUPPLY_VOLT))
    {
        tbat_v = chg_get_batt_volt_value();
        if((tbat_v > TBAT_DISCHG_VOLT) || (tbat_v < TBAT_SUPPLY_VOLT))
        {
            tbat_v = chg_get_batt_volt_value();
            if((tbat_v > TBAT_DISCHG_VOLT) || (tbat_v < TBAT_SUPPLY_VOLT))
            {
                ret = TRUE;
            }
        }
    }
    (void)chg_set_charge_enable(TRUE);
    chg_print_level_message(CHG_MSG_ERR,"CHG_SUP:BATTERY SPLY FINISH LEVEL:%d\n ",tbat_v);
    return ret;
}


boolean chg_is_sply_finish(void)
{
    uint32_t tbat_v = 0;

    /*��ȡ��ص���*/
    tbat_v = chg_get_batt_volt_value();
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY SPLY LEVEL:%d\n ",tbat_v);
    /*�ж��Ƿ�����ŵ�򲹵磬������3��*/
    if((tbat_v < TBAT_DISCHG_STOP_VOLT) && (tbat_v > TBAT_SUPPLY_STOP_VOLT))
    {
        tbat_v = chg_get_batt_volt_value();
        if((tbat_v < TBAT_DISCHG_STOP_VOLT) && (tbat_v > TBAT_SUPPLY_STOP_VOLT))
        {
            tbat_v = chg_get_batt_volt_value();
            if((tbat_v < TBAT_DISCHG_STOP_VOLT) && (tbat_v > TBAT_SUPPLY_STOP_VOLT))
            {
                if( TRUE == fact_release_flag )
                {
                    chg_set_fact_release_mode(FALSE);
                    fact_release_flag = FALSE;
                }
                chg_stm_state_info.bat_stat_type = POWER_SUPPLY_STATUS_SUPPLY_SUCCESS;
                /*֪ͨӦ�ò������*/
                chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY,(uint32_t)CHG_EVENT_NONEED_CARE);
                chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY SPLY FINISH LEVEL:%d\n ", tbat_v);
                chg_sply_unlock();

                return TRUE;
            }
        }
    }
    return FALSE;
}


int32_t chg_batt_supply_proc(void *task_data)
{
    int32_t tbat_v = 0;
    uint32_t wait_idx = 0;
    /*�����־ 0:��ʾ��磬1: ��ʾ�ŵ�*/
    boolean chg_flag = 0;
    uint32_t  tc_on = TBAT_SUPLY_DELAY_COUNTER;
    uint32_t  tc_off = TBAT_STOP_DELAY_COUNTER;

    if ( NULL == task_data )
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:supply_proc task_data is NULL !\n");
    }

    /*��ȡ��ص���*/
    tbat_v = chg_get_batt_volt_value();
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:START DISCHARGE OR CHARGE\n ");

    /* �ŵ�ɾ���������*/
    chg_task_delete();

    chg_sply_lock();
    chg_set_cur_chg_mode(CHG_SUPPLY_MODE);

    chg_stm_set_cur_state(CHG_STM_MAX_ST);

    chg_stm_state_info.bat_stat_type = POWER_SUPPLY_STATUS_NEED_SUPPLY;
    /*֪ͨӦ����Ҫ����*/
    chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY,(uint32_t)CHG_EVENT_NONEED_CARE);

    do{
        tbat_v = chg_get_batt_volt_value();
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY CURRENT LEVEL:%d\n ",tbat_v);

        /*�����ص������ڷŵ���ֵ*/
        if(tbat_v > TBAT_DISCHG_STOP_VOLT)
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY DISCHARGING BEGIN\n");
            chg_flag = 1;

            (void)chg_set_charge_enable(FALSE);
            if(TRUE != chg_set_suspend_mode(TRUE))
            {
                chg_print_level_message(CHG_MSG_ERR,"CHG_SUP:CHG SET SUSPEND MODE TRUE fail\n");
            }
            chg_set_fact_release_mode(TRUE);
            fact_release_flag = TRUE;
            for (wait_idx = 0; wait_idx < tc_on; wait_idx++)
            {
                chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
            }

            /*���巽ʽ*/
            if(TRUE != chg_set_suspend_mode(FALSE))
            {
                chg_print_level_message(CHG_MSG_ERR,"CHG_SUP:CHG SET SUSPEND MODE FALSE fail\n");
            }

            for (wait_idx = 0; wait_idx < tc_off; wait_idx++)
            {
                chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
            }

            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY DISCHARGING END\n");
        }
        /*���*/
        else if(tbat_v < TBAT_SUPPLY_STOP_VOLT)
        {
            /* log��ӡ�ӿڣ���ά�ɲ��������ݣ��˴��ݶ�Ϊ�ýӿ����������ӿ�ȷ���������޸� */
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY CHARGING BEGIN\n");
            chg_flag = 0;

            //�����������Ϊfast������
            chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
            (void)chg_set_charge_enable(TRUE);
            for (wait_idx = 0; wait_idx < tc_on; wait_idx++)
            {
                chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
            }
            /*���巽ʽ*/
            (void)chg_set_charge_enable(FALSE);
            for (wait_idx = 0; wait_idx < tc_off; wait_idx++)
            {
                chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
            }
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY CHARGING END\n");
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY CHARGING OR DISCHARGING EXIT:%d\n ",tbat_v);
        }
    }while(!chg_is_sply_finish());
    /* ��ʹ�� */
    (void)chg_set_charge_enable(FALSE);

#if (MBB_CHG_POWER_SUPPLY == FEATURE_OFF)
    /*LCD״̬����*/
    chg_display_interface( CHG_DISP_OK );
#endif/*MBB_CHG_POWER_SUPPLY == FEATURE_OFF*/
    tbat_v = chg_get_batt_volt_value();
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:BATTERY EXIT LEVEL:%d\n ",tbat_v);
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:BATTERY CHARGE or DISCHARGER FINISH\n ");

    return TBAT_SUPPLY_CURR_SUCCESS;
}


int32_t chg_tbat_chg_sply(void)
{
    /* ���ù�һ���ӿ�������������**/
    chg_sply_task_create();
/***************Note:ƽ̨��ش��룬����ƽ̨����Ҫ��ӣ��е�ƽ̨��V7R1��Ҫ
              ��ֲ��Ա������Ҫ����ӻ����Ƴ��±ߺ�������***************************/

#if (MBB_CHG_POWER_SUPPLY == FEATURE_OFF)
    /*���翪ʼ��ʾ����δ���ͼ��*/
    chg_display_interface( CHG_DISP_FAIL );
#endif/*MBB_CHG_POWER_SUPPLY == FEATURE_OFF*/
    return TBAT_SUPPLY_CURR_SUCCESS;
}
#endif /* MBB_CHG_COMPENSATE == FEATURE_ON */

static int32_t chg_get_cur_batt_volt(void)
{
    if( 0xFFFF == chg_real_info.battery_one_volt )
    {
        chg_real_info.battery_one_volt = chg_get_batt_volt_value();
    }

    return chg_real_info.battery_one_volt;
}


int32_t chg_get_sys_batt_volt(void)
{
    if( 0xFFFF == chg_real_info.battery_volt )
    {
        chg_real_info.battery_volt = chg_get_batt_volt_value();
    }
    return chg_real_info.battery_volt;
}


int32_t chg_get_avg_vph_pwr_volt(void)
{
    if( 0xFFFF == chg_real_info.vph_pwr_avg_volt )
    {
        chg_real_info.vph_pwr_avg_volt = chg_get_vph_pwr_volt_value();
    }
    return chg_real_info.vph_pwr_avg_volt;
}


int chg_get_cur_batt_temp(void)
{
    if( 0xFFFF == chg_real_info.battery_one_temp )
    {
        chg_real_info.battery_one_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
    }

    return chg_real_info.battery_one_temp;
}
EXPORT_SYMBOL_GPL(chg_get_cur_batt_temp);


int chg_get_sys_batt_temp(void)
{
    if( 0xFFFF == chg_real_info.battery_temp )
    {
        chg_real_info.battery_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
    }

    return chg_real_info.battery_temp;
}
EXPORT_SYMBOL_GPL(chg_get_sys_batt_temp);


void chg_set_sys_batt_capacity(int32_t capacity)
{
    /*���������仯ʱ�Ž����ϱ�����ϴ��ϱ����������ε�����ͬʱ�Ÿ�Ӧ���ϱ�*/
    if(chg_real_info.bat_capacity != capacity)
    {
        chg_real_info.bat_capacity = capacity;
        /*��ص����ٷֱȷ����仯֪ͨӦ�ò�ѯ*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:set_batt_capacity=%d\n",capacity);
    }
}


int32_t chg_get_sys_batt_capacity(void)
{
    if(chg_is_no_battery_powerup_enable())
    {
        return BATT_CAPACITY_FULL;
    }
#if (MBB_CHG_COULOMETER == FEATURE_ON)
    if(0 == chg_real_info.bat_capacity)
    {
        return smartstar_battery_capacity();
    }
    /*ʹ�ܽ�ֹ�͵�ػ��󣬴ﵽ�͵�ػ�����ʱ�����عػ�����+1*/
    if( unlikely( TRUE == chg_is_low_battery_poweroff_disable() ) )
    {
        if( BATT_CAPACITY_SHUTOFF >= chg_real_info.bat_capacity )
        {
            chg_real_info.bat_capacity = BATT_CAPACITY_SHUTOFF + 1;
        }
    }
#endif/*MBB_CHG_COULOMETER == FEATURE_ON*/    
    return chg_real_info.bat_capacity;
}

#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
#if defined(BSP_CONFIG_BOARD_E5_DC04)

boolean chg_is_batt_power_collapse(void)
{
    int32_t vbat_sys = chg_get_sys_batt_volt();
    /*���ڿ���״̬�£������ʱ״̬����ѹ�����趨����ʱ���е���ƽ����*/
    return (!chg_is_powdown_charging() && ((BATT_VOLT_LEVEL0_MAX >= vbat_sys)
          && (vbat_sys >= BATT_VOLT_POWER_OFF_THR))
          && (CHG_STM_BATTERY_ONLY == chg_stm_get_cur_state()));
}
#endif

boolean chg_is_batt_in_state_of_emergency(void)
{
    /*���ڿ���״̬���ж��Ƿ���Ҫ�ػ�*/
    return (!chg_is_powdown_charging() && BATT_VOLT_POWER_OFF_THR > chg_get_sys_batt_volt());
}
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/


void chg_set_batt_time_to_full(int32_t time_to_full)
{
    chg_real_info.bat_time_to_full = time_to_full;
    /*֪ͨӦ��ʣ����ʱ�䷢���仯*/
    chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)CHG_EVENT_NONEED_CARE);
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:set_batt_time_to_full=%d\n",time_to_full);
}


int32_t chg_get_batt_time_to_full(void)
{
    return chg_real_info.bat_time_to_full;
}


boolean chg_is_batt_full(void)
{
/*֧�ֿ��ؼƵĲ�Ʒ���������޶���Ϊ95%*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    return (BATT_CAPACITY_FULL == chg_get_sys_batt_capacity());
#else
    return (chg_get_sys_batt_volt() >= BATT_CHG_TEMP_MAINT_THR);
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
}


boolean chg_is_need_insert_charge(void)
{
/*֧�ֿ��ؼƵĲ�Ʒ���������޶���Ϊ95%*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    return (BATT_CAPACITY_FULL != chg_get_sys_batt_capacity());
#else
    return (chg_get_sys_batt_volt() < BATT_INSERT_CHARGE_THR);
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
}


static boolean chg_is_batt_need_rechg(void)
{
/*�������λʱ��ֻҪ�ٷֱȵ���100%�����������*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    /*��ظ���״̬��֧�ֿ��ؼƵ�ʹ�õ�ѹ��Ϊ���������ж�*/
    if(TRUE == is_batttemp_in_warm_chg_area())
    {
        return (chg_get_sys_batt_volt() < BATT_HIGH_TEMP_RECHARGE_THR);
    }
    if (TRUE == g_chg_batt_expand_change_normal_rechg_flag)
    {
        /*�����������ӶԵ������жϣ���������ؼƵ����������⵼�µ���һֱ�½���*/
        return (chg_get_sys_batt_volt() < BATT_NORMAL_TEMP_RECHARGE_THR
            || chg_get_sys_batt_capacity() <= BATT_EXPAND_CAPACITY_RECHG);
    }
#endif/*FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT*/
    return (chg_get_sys_batt_capacity() <= BATT_CAPACITY_RECHG);
#else
    if(TRUE == is_batttemp_in_warm_chg_area())
    {
        return (chg_get_sys_batt_volt() < BATT_HIGH_TEMP_RECHARGE_THR);
    }
    else
    {
        return (chg_get_sys_batt_volt() < BATT_NORMAL_TEMP_RECHARGE_THR);
    }
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
}

static boolean chg_is_batt_ovp(void)
{
    return (boolean)((chg_get_sys_batt_volt() >= BATT_CHG_OVER_VOLT_PROTECT_THR)
            || (chg_get_cur_batt_volt() >= BATT_CHG_OVER_VOLT_PROTECT_ONE_THR));
}

static boolean chg_is_batt_temp_valid(void)
{
    return !chg_temp_protect_flag;
}


TEMP_EVENT chg_get_batt_temp_state(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:batt_temp_state=%d\n",chg_batt_temp_state);
    return chg_batt_temp_state;
}


void chg_stm_set_chgr_type(chg_chgr_type_t chgr_type)
{
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = CHG_HVDCP_INVALID;
    hvdcp_type = chg_get_hvdcp_type();
    /*�ж��Ƿ�Ϊ��ѹ�����*/
    if (CHG_HVDCP_INVALID == hvdcp_type)
    {
        if (CHG_WALL_CHGR == fsa9688_get_charger_type())
        {
            chg_stm_state_info.cur_chgr_type = CHG_WALL_CHGR;
        }
        else
        {
            chg_stm_state_info.cur_chgr_type = chgr_type;
        }
    }
    else
    {
       chg_stm_state_info.cur_chgr_type = CHG_HVDCP_CHGR;
    }
#else
    chg_stm_state_info.cur_chgr_type = chgr_type;
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/

    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:set_chgr_type=%d\n",chg_stm_state_info.cur_chgr_type);
}


chg_chgr_type_t chg_stm_get_chgr_type(void)
{

    if(TRUE == chg_is_no_battery_powerup_enable())
    {
        chg_stm_state_info.cur_chgr_type = CHG_WALL_CHGR;
    }
    /*1. Return the current external charger type.*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_chgr_type=%d\n",chg_stm_state_info.cur_chgr_type);
    return chg_stm_state_info.cur_chgr_type;
}


boolean is_chg_charger_removed(void)
{
    boolean is_chgr_present      = chg_is_charger_present();
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    boolean is_wireless_online   = chg_stm_get_wireless_online_st();
    if ((FALSE == is_chgr_present) && (FALSE == is_wireless_online))
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_chg_charger_removed: is_chgr_present=%d, is_wireless_online=%d.\n",
                    is_chgr_present,is_wireless_online);
        return TRUE;
    }
#else
    /*1. External charger removed, swith to battery only state.*/
    if (FALSE == is_chgr_present)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_chg_charger_removed: is_chgr_present=%d !\n",is_chgr_present);
        return TRUE;
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_chg_charger_removed: charger attached !\n");
        return FALSE;
    }
}

#if (MBB_CHG_WIRELESS == FEATURE_ON)

void chg_stm_set_wireless_online_st(boolean online)
{
    chg_stm_state_info.wireless_online_st = online;
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:set_wireless_online_st=%d\n",online);
}


boolean chg_stm_get_wireless_online_st(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:get_wireless_online_st=%d\n",\
                chg_stm_state_info.wireless_online_st);

    return chg_stm_state_info.wireless_online_st;
}
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)

void chg_stm_set_extchg_online_st(boolean online)
{
    chg_stm_state_info.extchg_online_st = online;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:set_extchg_online_st=%d\n",online);
}


boolean chg_stm_get_extchg_online_st(void)
{
     chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_extchg_online_st=%d\n",\
                            chg_stm_state_info.extchg_online_st);
    return chg_stm_state_info.extchg_online_st;
}

#if defined(BSP_CONFIG_BOARD_E5_DC04)

boolean chg_stm_is_extchg_enabled(void)
{
    return !!g_exchg_enable_flag;
}


boolean chg_is_extchg_in_otg_mode(void)
{
    return !!is_otg_extchg;
}
#endif
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/


static void chg_stm_set_cur_state(chg_stm_state_type new_state)
{
    /*1. Update the current state machine state.*/
    chg_stm_state_info.cur_stm_state = new_state;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:set_cur_state=%d\n",new_state);
}


chg_stm_state_type chg_stm_get_cur_state(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_cur_state=%d\n",\
                    chg_stm_state_info.cur_stm_state);
    /*1. Return the current state.*/
    return chg_stm_state_info.cur_stm_state;
}


void chg_set_cur_chg_mode(CHG_MODE_ENUM chg_mode)
{
    chg_stm_state_info.cur_chg_mode = chg_mode;
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set_cur_chg_mode=%d\n",chg_mode);
}


CHG_MODE_ENUM chg_get_cur_chg_mode(void)
{

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_cur_chg_mode =%d\n",\
                  chg_stm_state_info.cur_chg_mode);

    /*1. Return the current state.*/
    return chg_stm_state_info.cur_chg_mode;
}


boolean chg_get_charging_status(void)
{
    chg_stm_state_type chg_stm_cur_state = CHG_STM_INIT_ST;
    chg_stm_cur_state = chg_stm_get_cur_state();
    /*1. Return the current state.*/
    if((CHG_STM_FAST_CHARGE_ST == chg_stm_cur_state) \
       || (CHG_STM_WARMCHG_ST == chg_stm_cur_state) \
       || (CHG_STM_HVDCP_CHARGE_ST == chg_stm_cur_state) )
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_charging_status is charging!\n");
        return TRUE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_charging_status is not charging!\n");
        return FALSE;
    }
}


void chg_set_supply_limit_by_stm_stat(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
    chg_stm_state_type curr_state = chg_stm_get_cur_state();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/

    /*curr_stateҪ��Ϊ�����±��������������������Ҫ�ж�һ�ºϷ��ԡ�*/
    if (curr_state <= CHG_STM_INIT_ST || curr_state >= CHG_STM_MAX_ST)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid state %d in %s.\n", 
            curr_state, __func__);
        return;
    }

    switch(cur_chgr_type)
    {
        case CHG_WALL_CHGR:
        {
            chg_set_supply_limit(chg_std_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                    chg_std_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            break;
        }
        case CHG_USB_HOST_PC:
        {
            if(CHG_CURRENT_SS == usb_speed_work_mode())
            {
                chg_set_supply_limit(chg_usb3_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                        chg_usb3_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            }
            else
            {
                chg_set_supply_limit(chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                        chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            }
            break;
        }
        case CHG_500MA_WALL_CHGR:
        case CHG_NONSTD_CHGR:
        case CHG_CHGR_UNKNOWN:
        {
            chg_set_supply_limit(chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                    chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            break;
        }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
        case CHG_HVDCP_CHGR:
        {
            if(CHG_HVDCP_5V == hvdcp_type)
            {
                chg_set_supply_limit(chg_5v_hvdcp_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                        chg_5v_hvdcp_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            }
            else if(CHG_HVDCP_9V == hvdcp_type)
            {
                chg_set_supply_limit(chg_9v_hvdcp_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                        chg_9v_hvdcp_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            }
            else if(CHG_HVDCP_12V == hvdcp_type)
            {
                chg_set_supply_limit(chg_12v_hvdcp_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                        chg_12v_hvdcp_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            }
            else
            {
                //reserve
            }
            break;
        }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
        default:
            break;
    }
}


void chg_set_hardware_parameter(const chg_hw_param_t* ptr_hw_param)
{
    uint32_t ret_code = 0;

    /* ASSERT(ptr_hw_param != NULL); */
    if (NULL == ptr_hw_param)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NULL pointer in chg_set_hardware_parameter!!\n");
        return;
    }

#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    if(TRUE == chg_battery_protect_flag)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:In battery protect condition do not update parameter!!\n");
        return;
    }
#endif

    /*Dump all the parameters to set.*/
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[ILIMIT]: %dmA, [ICHG]: %dmA, [VBATREG]: %dmV.\n",
               ptr_hw_param->pwr_supply_current_limit_in_mA,
               ptr_hw_param->chg_current_limit_in_mA,
               ptr_hw_param->chg_CV_volt_setting_in_mV);

    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[ITERM]: %dmA, [CHARGE]: %s.\n",
               ptr_hw_param->chg_taper_current_in_mA,
               ptr_hw_param->chg_is_enabled ? "enabled" : "disabled");

    /*1. Set power supply front-end current limit.*/
#if( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
#if( FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL )
    /*��Ǵ������������ӣ�USB�˿��±�δ����*/
    if((SURF_SENSOR_CONNECTED == g_surf_temp_state.is_connected) && (FALSE == chg_get_usb_temp_protect_stat()))
    {
        /*do nothing*/
    }
    /*��Ǵ�����δ���ӣ�USB�˿��±�δ����*/
    else if((SURF_SENSOR_CONNECTED != g_surf_temp_state.is_connected) && (FALSE == chg_get_usb_temp_protect_stat()))
    {
        if ( FALSE == chg_set_supply_limit(ptr_hw_param->pwr_supply_current_limit_in_mA))
        {
            /*If error occured, set 1st bit of ret_code.*/
            ret_code |= (1 << 1);
        }
    }
    /*USB�˿��±�����*/
    else
    {
        if ( FALSE == chg_set_supply_limit(CHG_IINPUT_LIMIT_100MA) )
        {
            /*If error occured, set 1st bit of ret_code.*/
            ret_code |= (1 << 1);
        }
    }
#else/*FEATURE_OFF == MBB_DCM_CUST_CHG_THERMAL*/
    /*USB�˿��±�����*/
    if( TRUE == chg_get_usb_temp_protect_stat() )
    {
        if ( FALSE == chg_set_supply_limit(CHG_IINPUT_LIMIT_100MA) )
        {
            /*If error occured, set 1st bit of ret_code.*/
            ret_code |= (1 << 1);
        }
    }
    else
    {
        if ( FALSE == chg_set_supply_limit(ptr_hw_param->pwr_supply_current_limit_in_mA))
        {
            /*If error occured, set 1st bit of ret_code.*/
            ret_code |= (1 << 1);
        }
    }
#endif/*FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL*/
#else/*FEATURE_OFF == MBB_CHG_USB_TEMPPT_ILIMIT*/
#if( FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL )
    if (SURF_SENSOR_CONNECTED != g_surf_temp_state.is_connected)
    {
        if ( FALSE == chg_set_supply_limit(ptr_hw_param->pwr_supply_current_limit_in_mA))
        {
            /*If error occured, set 1st bit of ret_code.*/
            ret_code |= (1 << 1);
        }
    }
#else
    if ( FALSE == chg_set_supply_limit(ptr_hw_param->pwr_supply_current_limit_in_mA))
    {
        /*If error occured, set 1st bit of ret_code.*/
        ret_code |= (1 << 1);
    }
#endif/*FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL*/
#endif/*FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT*/
    if (CHG_BATT_ID_DEF != chg_get_batt_id())
    {
        /*2. Set charge current.*/
        if ( FALSE == chg_set_cur_level(ptr_hw_param->chg_current_limit_in_mA))
        {
            /*If error occured, set 2nd bit of ret_code.*/
            ret_code |= (1 << 2);
        }

        /*3. Set CV voltage, IC type dependent, may not work on some IC, e.g. max8903c.*/
        if ( FALSE == chg_set_vreg_level(ptr_hw_param->chg_CV_volt_setting_in_mV))
        {
            /*If error occured, set 3rd bit of ret_code.*/
            ret_code |= (1 << 3);
        }
    }
    else
    {
#if (FEATURE_ON == MBB_CHG_BQ24196)
        /*2. Set charge current.*/
        if ( FALSE == chg_set_current_level(CHG_DEF_BATT_CUR_LEVEL, TRUE))
        {
            /*If error occured, set 2nd bit of ret_code.*/
            ret_code |= (1 << 2);
        }
#else /*!MBB_CHG_BQ24196*/
/*chg_set_current_level���������IC������δ����*/
#error You need check HOWTO implement this on your charger IC!!
#endif
        /*3. Set CV voltage, IC type dependent, may not work on some IC, e.g. max8903c.*/
        if ( FALSE == chg_set_vreg_level(CHG_DEF_BATT_VREG_LEVEL))
        {
            /*If error occured, set 3rd bit of ret_code.*/
            ret_code |= (1 << 3);
        }
    }

    /*4. Set taper(terminate) current. Also IC type dependent, may not work on
         some IC, e.g. max8903c.*/
    if ( FALSE == chg_set_term_current(ptr_hw_param->chg_taper_current_in_mA))
    {
        /*If error occured, set 4th bit of ret_code.*/
        ret_code |= (1 << 4);
    }

    /*5. Enable/Disable Charge.*/
    if ( FALSE == chg_set_charge_enable(ptr_hw_param->chg_is_enabled))
    {
        /*If error occured, set 5th bit of ret_code.*/
        ret_code |= (1 << 5);
    }

    if (ret_code)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Set charge IC hw parameter error 0x%x.\n", ret_code);
    }

}


static void chg_chgr_type_checking_timer_cb(chg_timer_para data)
{
    (void)data;

    (void)chg_send_msg_to_main(CHG_CHGR_TYPE_CHECKING_EVENT);

     chg_print_level_message(CHG_MSG_INFO, "CHG_STM: chg_send_msg_to_main CHG_CHGR_TYPE_CHECKING_EVENT!\n ");
}


void chg_start_chgr_type_checking(void)
{
    chg_sta_timer_set(CHG_CHGR_TYPE_CHECK_INTERVAL_IN_MS,chg_chgr_type_checking_timer_cb);
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM: chg_start_chgr_type_checking!\n ");
}


void chg_check_and_update_hw_param_per_chgr_type(void)
{
    static uint8_t chgr_type_checking_cnter = 0;
    chg_chgr_type_t chgr_type_from_usb = chg_stm_get_chgr_type();
    chg_stm_state_type curr_state = chg_stm_get_cur_state();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type =  chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    /*1. If charger removal detected, give warning msg, return.*/
    if (FALSE == chg_is_charger_present())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Chgr has been removed, type checking ignored.\n");
        return;
    }

    /*curr_stateҪ��Ϊ�����±��������������������Ҫ�ж�һ�ºϷ��ԡ�*/
    if (curr_state <= CHG_STM_INIT_ST || curr_state >= CHG_STM_MAX_ST)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid state %d in %s.\n", 
            curr_state, __func__);
        return;
    }

#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    if(CHG_HVDCP_INVALID != hvdcp_type)
    {
        chg_stm_set_chgr_type(CHG_HVDCP_CHGR);
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
    /*����Ǹ�ѹ�����0-10���Ѿ������������ٸ��³�����*/
    if( (0 == chg_sub_low_temp_changed)
        && (CHG_BATT_ID_FEIMAOTUI_4800MAH != g_chg_batt_data->batt_id)
        && (CHG_BATT_ID_XINGWANGDA_4800MAH != g_chg_batt_data->batt_id) )
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[%s]High Volt Batt below 10 degree did`t update parameter!\n", __func__);
        return;
    }
#endif/*MBB__CHG_HIGH_VOLT_BATT == FEATURE_ON*/
    if (CHG_CHGR_UNKNOWN == chgr_type_from_usb)
    {
        if (chgr_type_checking_cnter >= CHG_CHGR_TYPE_CHECK_MAX_RETRY_TIMES)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Chgr type check timeout failed," \
                       " treat as non-standard.\n");
            mlog_print(MLOG_CHG, mlog_lv_warn, "CHG_STM:Charger type check timeout failed," \
                       " treat as non-standard.\n");
            chg_stm_set_chgr_type(CHG_NONSTD_CHGR);
            /*���������*/
            chgr_type_checking_cnter = 0;
        }
        else
        {
            chgr_type_checking_cnter++;
            chg_sta_timer_set(CHG_CHGR_TYPE_CHECK_INTERVAL_IN_MS,
                              chg_chgr_type_checking_timer_cb);
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Raise %d time chgr type checking.\n",
                       chgr_type_checking_cnter);
        }

        return;
    }
    else if((CHG_WALL_CHGR == chgr_type_from_usb) || (CHG_USB_HOST_PC == chgr_type_from_usb)
             || (CHG_NONSTD_CHGR == chgr_type_from_usb))
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Chgr type %d attached after %d times polling.\n",
            chgr_type_from_usb, chgr_type_checking_cnter + 1);
        mlog_print(MLOG_CHG, mlog_lv_info, "CHG_STM:Charger type %d attached " \
                   "after %d times polling.\n",
                   chgr_type_from_usb, chgr_type_checking_cnter + 1);
        if (CHG_WALL_CHGR != chgr_type_from_usb)
        {
            if ((CHG_USB_HOST_PC == chgr_type_from_usb) && (CHG_CURRENT_SS == usb_speed_work_mode()))
            {
                chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_check_and_update_hw_param_per_chgr_type]set usb 3.0 charge parameter\n\r");
                chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[curr_state]);
            }
            else
            {
                 chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[%s]USB/3rd chgr, no need update hw paras.\n", __func__);
            }
        }
        else
        {
            if (curr_state == CHG_STM_FAST_CHARGE_ST
                || curr_state == CHG_STM_WARMCHG_ST
                || curr_state == CHG_STM_MAINT_ST
                || curr_state == CHG_STM_INVALID_CHG_TEMP_ST)
            {
                chg_en_flag = 1;
                chg_en_timestamp = jiffies;
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[%s]set std_chgr charge parameter!\n", __func__);
                chg_set_hardware_parameter(&chg_std_chgr_hw_paras[curr_state]);
            }
            else
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:[%s]Invalid stm_state=%d, "    \
                           "chgr_type=%d.\n", __func__, curr_state, chgr_type_from_usb);
            }
        }

        /*���������*/
        chgr_type_checking_cnter = 0;
        return;
    }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    else if(CHG_HVDCP_CHGR == chgr_type_from_usb)
    {
        if(CHG_STM_HVDCP_CHARGE_ST == curr_state)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[%s]set HVDCP_chgr charge parameter!\n", __func__);
            /*Set charging hardware parameter according to HVDCP voltage type.*/
            if(CHG_HVDCP_9V == hvdcp_type)
            {
                chg_set_hardware_parameter(&chg_9v_hvdcp_chgr_hw_paras[curr_state]);
            }
            else if( CHG_HVDCP_12V == hvdcp_type)
            {
                chg_set_hardware_parameter(&chg_12v_hvdcp_chgr_hw_paras[curr_state]);
            }
            else
            {
                chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[curr_state]);
            }
        }
        else
        {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:[%s]Invalid stm_state=%d, "    \
                           "chgr_type=%d.\n", __func__, curr_state, chgr_type_from_usb);
        }

        /*���������*/
        chgr_type_checking_cnter = 0;
        return;
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    else if(CHG_500MA_WALL_CHGR == chgr_type_from_usb)
    {
        chg_set_hardware_parameter(&chg_weak_chgr_hw_paras[curr_state]);

        /*���������*/
        chgr_type_checking_cnter = 0;
        return;
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if(CHG_WIRELESS_CHGR == chgr_type_from_usb)
    {
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[curr_state]);
        mlog_print(MLOG_CHG, mlog_lv_info, "Wireless charger detected in polling check.\n");

        /*���������*/
        chgr_type_checking_cnter = 0;
        return;
    }
#endif/*MBB_CHG_WIRELESS*/
    else
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid chgr_type!\n");
        /*���������*/
        chgr_type_checking_cnter = 0;
    }
}


void chg_stm_switch_state(chg_stm_state_type new_state)
{
    chg_stm_state_type cur_state = chg_stm_get_cur_state();
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_stm_switch_state begin!\n");
    if((cur_state >= CHG_STM_MAX_ST) || (cur_state <= CHG_STM_INIT_ST))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_stm_switch_state: cur_state Invalid \n");
    }

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    chg_stm_set_pre_state(cur_state);
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

    if (new_state >= CHG_STM_MAX_ST || new_state <= CHG_STM_INIT_ST)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_stm_switch_state: Invalid state %d.\n", new_state);
        return;
    }

    chg_stm_set_cur_state(new_state);
    if (cur_state != new_state)
    {
        if (NULL != chg_stm_state_machine[cur_state].chg_stm_exit_func)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_switch_state exit %d state.\n", cur_state);
            chg_stm_state_machine[cur_state].chg_stm_exit_func();
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Cur_state %d exit func doesN'T exist.\n", cur_state);
        }

        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Charge state machine switch from %d to %d state.\n",
            cur_state, new_state);
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Charge state machine switch from %s to %s state.\n",
            (CHG_STM_INIT_ST == cur_state) ? "Init" : TO_STM_NAME(cur_state),
            TO_STM_NAME(new_state));


        if (NULL != chg_stm_state_machine[new_state].chg_stm_entry_func)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_switch_state entry %d state.\n", new_state);
            chg_stm_state_machine[new_state].chg_stm_entry_func();
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:New_state %d entry func doesN'T exist.\n", new_state);
        }

        if ((CHG_STM_TRANSIT_ST == new_state)
            && (NULL != chg_stm_state_machine[new_state].chg_stm_period_func))
        {
            /*�л�ǰ����һ�ε�ص�ѹ��ѯ��ѯ**/
            chg_poll_bat_level();
            /* �л�ǰ����һ�ε���¶���ѯ��ѯ**/
            chg_poll_batt_temp();
            chg_stm_state_machine[new_state].chg_stm_period_func();
        }

        if (CHG_STM_INVALID_CHG_TEMP_ST == new_state)
        {
            mlog_print(MLOG_CHG, mlog_lv_warn, "CHG STM switch from %s to invalid chg temp state. "
                "tBat = %d'C, tBat_sys = %d'C.\n",
                (CHG_STM_INIT_ST == cur_state) ? "Init" : TO_STM_NAME(cur_state),
                chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        }

        mlog_print(MLOG_CHG, mlog_lv_info, "CHG STM switch from %s to %s state.\n",
            (CHG_STM_INIT_ST == cur_state) ? "Init" : TO_STM_NAME(cur_state),
            TO_STM_NAME(new_state));
        (void)chg_dump_ic_hwinfo();
        mlog_print(MLOG_CHG, mlog_lv_info, "Current Battery Info: [vBat]%dmV, [vBat_sys]%dmV, " \
            "[tBat]%d'C, [tBat_sys]%d'C.\n", chg_get_cur_batt_volt(), chg_get_sys_batt_volt(),
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
#if ( FEATURE_ON == MBB_MLOG )
        if(((CHG_STM_FAST_CHARGE_ST == new_state) && (CHG_STM_WARMCHG_ST != cur_state)) || 
            ((CHG_STM_WARMCHG_ST == new_state) && (CHG_STM_FAST_CHARGE_ST != cur_state)))
        {
            printk(KERN_ERR "CHG_STM:charge_count\n");
            mlog_set_statis_info("charge_count",1);//����ܴ��� ��1
        }
        if(CHG_STM_WARMCHG_ST == new_state)
        {
            printk(KERN_ERR "CHG_STM:overtemp_charge_count\n");
            mlog_set_statis_info("overtemp_charge_count",1);//���³�������1
        }
        if(CHG_STM_INVALID_CHG_TEMP_ST == new_state) 
        {
            printk(KERN_ERR "CHG_STM:overtemp_charge_stop_count\n");
            mlog_set_statis_info("overtemp_charge_stop_count",1); //����ͣ�������1
        }
#endif        
    }
    else
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_switch_state: charge has already be %s[%d] state, " \
                         "no need switch.\n", TO_STM_NAME(cur_state), cur_state);
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_stm_switch_state over!\n");
}


void chg_stm_periodic_checking_func(void)
{
    chg_stm_state_type chg_current_st = CHG_STM_INIT_ST;
    chg_chgr_type_t    chg_cur_chgr   = CHG_CHGR_UNKNOWN;

    uint32_t chg_prt_timer_val = 0;
    int32_t  cur_poll_timer_period  = 0;
    /*0:-PwrOff Charging; 1:-Normal Charging.*/
    uint8_t  is_normal_chg_mode     = !chg_is_powdown_charging();
    /*0:-USB/NoStd Chgr;  1:-Wall/Standard Chgr.*/
    uint8_t  is_std_wall_chgr       = 0;

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_stm_periodic_checking_func begin!!\n");
    if((CHG_WALL_CHGR == chg_stm_get_chgr_type())
     || (CHG_HVDCP_CHGR == chg_stm_get_chgr_type()))
     {
        is_std_wall_chgr = 1;
     }

    /* Get the current battery charging state */
    chg_current_st = chg_stm_get_cur_state();

    if(1 == chg_en_flag)
    {
        unsigned long time_now = jiffies;
        if((time_now - chg_en_timestamp) > CHG_EN_TIME_SLOT)
        {
            chg_en_flag = 0;
        }
    }
    else
    {
        //for lint
    }

    /*Make sure that we are within the charger state machine
      configuration table*/
    if (chg_current_st > CHG_STM_INIT_ST && chg_current_st < CHG_STM_MAX_ST)
    {
        /* If we have a valid state function, call it. */
        if (NULL != chg_stm_state_machine[chg_current_st].chg_stm_period_func)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:stm_periodic_checking_func polling at %d state\n", chg_current_st);
            chg_stm_state_machine[chg_current_st].chg_stm_period_func();
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:State %d haveNOT period func.\n", chg_current_st);
        }

        /*Obtain current_st once again, since chg state may be updated in period_func*/
        chg_current_st = chg_stm_get_cur_state();
        chg_cur_chgr   = chg_stm_get_chgr_type();

        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:stm periodic checking: Charge stay at %s[%d] state now.\n",
            TO_STM_NAME(chg_current_st), chg_current_st);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:stm periodic checking: Current chgr is %s[%d].\n",
            TO_CHGR_NAME(chg_cur_chgr), chg_cur_chgr);

        if(TRUE == chg_get_charging_status())
        {
            chg_prt_timer_val = chg_fastchg_timeout_value_in_sec[is_normal_chg_mode][is_std_wall_chgr];
            if (chg_stm_state_info.charging_lasted_in_sconds < chg_prt_timer_val)
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System keep staying at charging state.\n");

                cur_poll_timer_period = chg_poll_timer_get();
                if (CHG_ERROR == cur_poll_timer_period)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_poll_timer_get error.\n");
                }
                else
                {
                    chg_stm_state_info.charging_lasted_in_sconds +=
                                   ((uint32_t)cur_poll_timer_period / MS_IN_SECOND);
                    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:charging has lasted for %d Seconds.\n",
                                            chg_stm_state_info.charging_lasted_in_sconds);
                }
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:charging SW timer expired, " \
                       "battery voltage(sys) %dmV.\n", chg_get_sys_batt_volt());

                /*If battery is full already, or at pwroff chg mode, goto maint state.*/
                if (TRUE == chg_is_batt_full() || FALSE == is_normal_chg_mode)
                {
                   /*4. Reset fast charge counter.and switch to manit state,disable chg*/
                   chg_stm_switch_state(CHG_STM_MAINT_ST);
                   chg_stm_state_info.charging_lasted_in_sconds = 0;
                }
                else
                {
                    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Reset charging counter, " \
                           "continue to charge.\n");
                 /*Reset fast charge counter and continue to charge */
                    chg_stm_state_info.charging_lasted_in_sconds = 0;
                }

                mlog_print(MLOG_CHG, mlog_lv_warn, "Charging SW timer expired, " \
                       "battery voltage(sys) %dmV.\n", chg_get_sys_batt_volt());
                mlog_print(MLOG_CHG, mlog_lv_warn, "current state: %s, current charger:%s.\n",
                    TO_STM_NAME(chg_current_st), TO_CHGR_NAME(chg_cur_chgr));
            }
        }
        else
        {
            chg_stm_state_info.charging_lasted_in_sconds = 0;
        }
        /*���ͣ�䵫�������λ��ʱ��*/
#if ( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
        if ((TRUE ==  chg_is_charger_present())
            && (FALSE == chg_get_charging_status())
            && (BATT_CAPACITY_FULL == chg_real_info.bat_capacity))
        {
            cur_poll_timer_period = chg_poll_timer_get();
            if (CHG_ERROR == cur_poll_timer_period)
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_poll_timer_get error.\n");
            }
            else
            {
                chg_stm_state_info.charger_lasted_without_charging_in_seconds +=
                               ((uint32_t)cur_poll_timer_period / MS_IN_SECOND);
                chg_print_level_message(CHG_MSG_INFO,
                                        "CHG_STM:charger has existed without charging for %d Seconds.\n",
                                        chg_stm_state_info.charging_lasted_in_sconds);
            }
        }
        else
        {
            chg_stm_state_info.charger_lasted_without_charging_in_seconds = 0;
        }
#endif
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid state %d in period checking.\n", chg_current_st);
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_periodic_checking_func over!!\n");
}


static void chg_notify_UI_charging_state(uint8_t bc_state)
{
    /*Don't display battery charging as default.*/
    uint8_t   new_bc_state         = !!bc_state;
    uint8_t   is_pwr_off_charging  = chg_is_powdown_charging();
    BATTERY_EVENT  event_to_send   = BAT_EVENT_MAX;
    static uint8_t cur_ui_bc_state = 0;

    if (new_bc_state != cur_ui_bc_state)
    {
        if (new_bc_state)
        {
            if (is_pwr_off_charging)
            {
                event_to_send = BAT_CHARGING_OFF_START;
            }
            else
            {
                event_to_send = BAT_CHARGING_ON_START;
            }
        }
        else
        {
            if (is_pwr_off_charging)
            {
                event_to_send = BAT_CHARGING_DOWN_STOP;
            }
            else
            {
                event_to_send = BAT_CHARGING_UP_STOP;
            }
        }
        cur_ui_bc_state = new_bc_state;

        /*Send the event to UI module.*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, event_to_send);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Send battery event %d to APP.\n", event_to_send);
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:UI bc state is already %d.\n", cur_ui_bc_state);
    }
}


static void chg_transit_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into transition state.\n");

    /*1. Reset charger type to INVALID will be done at:
         a. chg_stm_init() when system just powered up.
         b. Entry function of battery only state when normal running.
     */

    /*2. Don't change UI battery charging state, just KEEP is OK.*/

    /*3. Set poll timer mode, FAST or CHGR_INPUT, will be done:
         a. When system just powered up: chg_stm_init or other module init function.
         b. For CHGR_INPUT: period func.
     */
    chg_poll_timer_set(FAST_POLL_CYCLE);
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:transit_state_entry_func poll_timer_set FAST_POLL_CYCLE.\n");

    /*4. IF chgr_is_present
       ->Y: 1. Read chgr type from USB, Set chgr_type
            IF chgr type still unknown
          ->Y: Raise chgr type checking timer.
            2. Set charging hardware parameter per charger type
     */
        /*Config charge hardware parameter according to chgr_type.*/
    /*���*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
    }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    /*��ѹ�����*/
    else if(CHG_HVDCP_CHGR == cur_chgr_type)
    {
        if(CHG_HVDCP_5V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
        }
        else if(CHG_HVDCP_9V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_9v_hvdcp_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
        }
        else if(CHG_HVDCP_12V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_12v_hvdcp_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
        }
        else
        {
            //reserve
        }
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    //usb 3.0����
    else if ((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_transit_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
    }
    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_weak_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
    }
    else if ((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type))
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
    }

#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        /*һֱ�����߳��оƬʹ�ܣ����߳������и�״̬��ʹ����BQ24196����*/
        chg_set_wireless_chg_enable(TRUE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_transit_state_entry_func disable wireless chg!\n");
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_TRANSIT_ST]);

        mlog_print(MLOG_CHG, mlog_lv_info, "wireless charger detected.\n", cur_chgr_type);
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
    else if(CHG_CHGR_UNKNOWN == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Chgr type unknown use usb para and raise checking timer.\n");
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
    }
    else
    {
        if(TRUE == is_chg_charger_removed())
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg not exist force transit_state_entry to batt_only_st!\n");
            chg_stm_switch_state(CHG_STM_BATTERY_ONLY);

        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:transit_state_entry Chgr type invaild!\n");
        }
    }
    mlog_print(MLOG_CHG, mlog_lv_info, "charger type %d insertion detected.\n", cur_chgr_type);
}


static void chg_transit_state_period_func(void)
{
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
     /*1. IF charger_remove
             Switch to batttery only state.
       */
    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg not exist force transit_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);

    }

    /*2. IF vBat > BATT_INSERT_CHARGE_THR || batt OVP
            Switch to maintenance state.
     */
    else if (FALSE == chg_is_need_insert_charge() || TRUE == chg_is_batt_ovp())
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Batt full/OVP, vBat = %dmV, vBat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        mlog_print(MLOG_CHG, mlog_lv_warn, "CHG_STM:Batt full/OVP, vBat = %dmV, vBat_sys = %dmV.\n",
                   chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        chg_stm_switch_state(CHG_STM_MAINT_ST);
    }

    /*3. IF Battery is too hot or too cold
            Switch to invalid charge battery temperature state.
     */
    else if (FALSE == chg_is_batt_temp_valid())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid batt-temp, tBat = %d'C, tBat_sys = %d'C.\n",
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        mlog_print(MLOG_CHG, mlog_lv_warn, "CHG_STM:Invalid batt-temp, tBat = %d'C, " \
                   "tBat_sys = %d'C.\n", chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
    }

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    else if(TRUE == is_batttemp_in_warm_chg_area())
    {
         chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Switch transit state to warmchg state, tBat = %d'C, tBat_sys = %d'C.\n",
         chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
         chg_stm_switch_state(CHG_STM_WARMCHG_ST);
    }
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    else if(CHG_HVDCP_INVALID != hvdcp_type)
    {
         chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Switch transit state to hvdcpchg state\n");
         chg_stm_switch_state(CHG_STM_HVDCP_CHARGE_ST);
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    /*4. ELSE Switch to fast chg state.*/
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:form transit_state to fast_stste!\n");
        chg_stm_switch_state(CHG_STM_FAST_CHARGE_ST);
    }
}


static void chg_transit_state_exit_func(void)
{
   chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from transition state.\n");
}

#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)

void chg_set_high_volt_batt_current(chg_chgr_type_t cur_chgr_type )
{
    uint32_t index_num = 0;

    /*4800mAh���0-10��10-45�Ȳ�����ͬ�ĳ��������˴�ֱ�ӷ��ء�*/
    if (CHG_BATT_ID_FEIMAOTUI_4800MAH == g_chg_batt_data->batt_id
        || CHG_BATT_ID_XINGWANGDA_4800MAH == g_chg_batt_data->batt_id)
    {
        chg_print_level_message(CHG_MSG_INFO, 
            "CHG_STM: It's NOT necessary to change chg paras for 4800mAh batt.\n");
        return;
    }

    for(index_num = 0; index_num < CHG_CHGR_INVALID; index_num++)
    {
        if(charger_current_limit_paras[index_num].chgr_type == cur_chgr_type)
        {
#if (MBB_CHG_HVDCP_CHARGE == FEATURE_ON)
            switch(g_chg_batt_data->batt_id)
            {
                case CHG_BATT_ID_FEIMAOTUI_2300MAH:
                {
                    chg_set_hvdcp_adpter_vol(CHG_HVDCP_5V);
#if (MBB_CHG_SMB1351 == FEATURE_OFF)
                    chg_set_supply_limit(HVDCP_CHARGE_CURRENT_500MA);
#endif/*MBB_CHG_SMB1351 == FEATURE_OFF*/
                    break;
                }

                default:
                    chg_set_hvdcp_adpter_vol(CHG_HVDCP_5V);
#if (MBB_CHG_SMB1351 == FEATURE_OFF)
                    chg_set_supply_limit(HVDCP_CHARGE_CURRENT_500MA);
#endif/*MBB_CHG_SMB1351 == FEATURE_OFF*/
                    break;
            }
#endif/*MBB_CHG_HVDCP_CHARGE == FEATURE_ON*/
            if (CHG_BATT_ID_DEF != chg_get_batt_id())
            {
                chg_set_current_level(charger_current_limit_paras[index_num].current_limit,
                    g_chg_batt_data->is_20pct_calib);
                chg_set_supply_limit(charger_current_limit_paras[index_num].current_limit_usb);
            }
            else
            {
#if (FEATURE_ON == MBB_CHG_BQ24196)
                chg_set_current_level(CHG_DEF_BATT_CUR_LEVEL, TRUE);
#else /*!MBB_CHG_BQ24196*/
/*chg_set_current_level���������IC������δ����*/
#error You need check HOWTO implement this on your charger IC!!
#endif
            }
            break;
        }
    }
}


void chg_high_volt_batt_entry_func(void)
{
    uint32_t battry_temp = chg_get_sys_batt_temp();
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_high_volt_batt_entry_func begin!\n");
    /*���������δʶ�����֮ǰ��������ֹʶ���������������*/
    if(CHG_CHGR_UNKNOWN == cur_chgr_type)
    {
        return;
    }

    chg_sub_low_temp_changed = -1;
    if (battry_temp < CHG_SUB_LOW_TEMP_TOP)
    {
        chg_sub_low_temp_changed = 0;
        chg_set_high_volt_batt_current(cur_chgr_type);
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set high batt current in 0-10c!\n");
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_high_volt_batt_entry_func over!\n");
}


void chg_high_volt_batt_period_func(void)
{
    uint32_t battry_temp = chg_get_sys_batt_temp();
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = CHG_HVDCP_5V;
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    /*���������δʶ�����֮ǰ��������ֹʶ���������������*/
    if(CHG_CHGR_UNKNOWN == cur_chgr_type)
    {
        return;
    }
    /*USB�˿��¶ȱ������������õ�ǰ״̬ΪFALSE�����ǰ��������ͻ��*/
#if( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
    if( TRUE == chg_get_usb_temp_protect_stat() )
    {
        if( 0 == chg_sub_low_temp_changed )
        {
            chg_sub_low_temp_changed = -1;
        }
        return;
    }
#endif
    /*������ͱ����������������õ�ǰ״̬ΪINITIAL�����ǰ��������ͻ��*/
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    if(TRUE == chg_battery_protect_flag)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:In battery protect condition " \
          "do not high_volt_batt on 0-10!\n");
        if( 0 == chg_sub_low_temp_changed )
        {
            chg_sub_low_temp_changed = -1;
        }
        return;
    }
#endif
    /* �����¶�����ת�����ó����� */
    if (battry_temp < CHG_SUB_LOW_TEMP_TOP && -1 == chg_sub_low_temp_changed)
    {
        chg_sub_low_temp_changed = 0;
        chg_set_high_volt_batt_current(cur_chgr_type);
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set high batt current in 0-10c!\n");
    }
    else if (battry_temp >= (CHG_SUB_LOW_TEMP_TOP + CHG_TEMP_RESUM) && 0 == chg_sub_low_temp_changed)
    {
        chg_sub_low_temp_changed = -1;
        if(CHG_WALL_CHGR == cur_chgr_type)
        {
            if (CHG_BATT_ID_DEF != chg_get_batt_id())
            {
                chg_set_cur_level(chg_std_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST].chg_current_limit_in_mA);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "Unknown batt type, use safer chg paras.\n");
#if (FEATURE_ON == MBB_CHG_BQ24196)
                chg_set_current_level(CHG_DEF_BATT_CUR_LEVEL, TRUE);
#else /*!MBB_CHG_BQ24196*/
/*chg_set_current_level���������IC������δ����*/
#error You need check HOWTO implement this on your charger IC!!
#endif
            }
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP rise up to 13'C use normal std_chgr current_limit\n");
        }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
        else if(CHG_HVDCP_CHGR == cur_chgr_type)
        {
            (void)chg_set_hvdcp_adpter_vol(g_hvdcp_init_vol);
            chg_delay_ms(1);
            hvdcp_type = chg_get_hvdcp_type();

            if(CHG_HVDCP_9V == hvdcp_type)
            {
                chg_set_supply_limit(chg_9v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST].pwr_supply_current_limit_in_mA);
                if (CHG_BATT_ID_DEF != chg_get_batt_id())
                {
                    chg_set_cur_level(chg_9v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST].chg_current_limit_in_mA);
                }
                else
                {
                    chg_print_level_message(CHG_MSG_INFO, 
                        "Unknown batt type, use safer chg paras.\n");
#if (FEATURE_ON == MBB_CHG_BQ24196)
                    chg_set_current_level(CHG_DEF_BATT_CUR_LEVEL, TRUE);
#else /*!MBB_CHG_BQ24196*/
/*chg_set_current_level���������IC������δ����*/
#error You need check HOWTO implement this on your charger IC!!
#endif
                }
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP rise up to 13'C use normal 9v_hvdcp current_limit\n");
            }
            else if( CHG_HVDCP_12V == hvdcp_type)
            {
                chg_set_supply_limit(chg_12v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST].pwr_supply_current_limit_in_mA);
                if (CHG_BATT_ID_DEF != chg_get_batt_id())
                {
                    chg_set_cur_level(chg_12v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST].chg_current_limit_in_mA);
                }
                else
                {
                    chg_print_level_message(CHG_MSG_INFO, 
                        "Unknown batt type, use safer chg paras.\n");
#if (FEATURE_ON == MBB_CHG_BQ24196)
                    chg_set_current_level(CHG_DEF_BATT_CUR_LEVEL, TRUE);
#else /*!MBB_CHG_BQ24196*/
/*chg_set_current_level���������IC������δ����*/
#error You need check HOWTO implement this on your charger IC!!
#endif
                }
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP rise up to 13'C use normal 12v_hvdcp current_limit\n");
            }
            else
            {
                chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST]);
            }
        }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
        else if(CHG_USB_HOST_PC == cur_chgr_type)
        {
            if(CHG_CURRENT_SS == usb_speed_work_mode())
            {
                if (CHG_BATT_ID_DEF != chg_get_batt_id())
                {
                    chg_set_cur_level(chg_usb3_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST].chg_current_limit_in_mA);
                }
                else
                {
                    chg_print_level_message(CHG_MSG_INFO, 
                        "Unknown batt type, use safer chg paras.\n");
#if (FEATURE_ON == MBB_CHG_BQ24196)
                    chg_set_current_level(CHG_DEF_BATT_CUR_LEVEL, TRUE);
#else /*!MBB_CHG_BQ24196*/
/*chg_set_current_level���������IC������δ����*/
#error You need check HOWTO implement this on your charger IC!!
#endif
                }
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP rise up to 13'C use normal usb3 current_limit\n");

            }
            else
            {
                if (CHG_BATT_ID_DEF != chg_get_batt_id())
                {
                    chg_set_cur_level(chg_usb_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST].chg_current_limit_in_mA);
                }
                else
                {
                    chg_print_level_message(CHG_MSG_INFO, 
                        "Unknown batt type, use safer chg paras.\n");
#if (FEATURE_ON == MBB_CHG_BQ24196)
                    chg_set_current_level(CHG_DEF_BATT_CUR_LEVEL, TRUE);
#else /*!MBB_CHG_BQ24196*/
/*chg_set_current_level���������IC������δ����*/
#error You need check HOWTO implement this on your charger IC!!
#endif
                }
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP rise up to 13'C use normal usb current_limit\n");
            }
        }
        else
        {
            if (CHG_BATT_ID_DEF != chg_get_batt_id())
            {
                chg_set_cur_level(chg_usb_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST].chg_current_limit_in_mA);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "Unknown batt type, use safer chg paras.\n");
#if (FEATURE_ON == MBB_CHG_BQ24196)
                chg_set_current_level(CHG_DEF_BATT_CUR_LEVEL, TRUE);
#else /*!MBB_CHG_BQ24196*/
/*chg_set_current_level���������IC������δ����*/
#error You need check HOWTO implement this on your charger IC!!
#endif
            }
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP rise up to 13'C weak or nonstd chgr use usb current_limit\n");
        }
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume high batt current in 0-10c!\n");
    }
}
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/


static void chg_fastchg_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();

    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into fast-charge state.\n");

    /*1. Notify UI that battery charging started.*/
    chg_notify_UI_charging_state(CHG_UI_START_CHARGING);

    /*2. Set charging hardware parameter per charger type.*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {
        /*ʹ�ܳ�磬���ñ�־*/
        chg_en_flag = 1;
        chg_en_timestamp = jiffies;
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:fastchg_state_entry set std_chgr_hw_paras!\n");
        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }

    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:fastchg_state_entry set chg_weak_chgr_hw_paras!\n");
        chg_set_hardware_parameter(&chg_weak_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_fastchg_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_fastchg_state_entry_func enable wireless chg!\n");
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
    else if((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type)
            || (CHG_CHGR_UNKNOWN == cur_chgr_type))
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:fastchg_state_entry set usb_chgr_hw_paras!\n");
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
    else
    {
        if(TRUE == is_chg_charger_removed())
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg not exist force fastchg_state_entry to batt_only_st!\n");
            chg_stm_switch_state(CHG_STM_BATTERY_ONLY);

        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:fastchg_state_entry Chgr type invaild!\n");
        }
    }
    /*3. Enable IC Charge function.*/
    /*Already done in chg_set_hardware_parameter.*/

#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
    chg_high_volt_batt_entry_func();
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/

}


static void chg_fastchg_state_period_func(void)
{
    chg_stop_reason stp_reas        = CHG_STOP_COMPLETE;
    uint32_t ret_code               = 0;
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/

    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg removed force fastchg_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    else if(hvdcp_type != CHG_HVDCP_INVALID)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:HVDCP attach fastchg_st to hvdcp_charge_st!\n");
        chg_stm_switch_state(CHG_STM_HVDCP_CHARGE_ST);
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    else if (FALSE == chg_is_IC_charging())
    {
        stp_reas = chg_get_stop_charging_reason();

        if (CHG_STOP_COMPLETE == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge Completed, vBat = %dmV, vBat_sys = %dmV.\n",
                chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
            if(TRUE == chg_is_batt_full())
            {
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge Completed, but soc not 100, need adjust.\n");
            }
#else
            chg_stm_switch_state(CHG_STM_MAINT_ST);
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
        }
        else if (CHG_STOP_TIMEOUT == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge timeout from IC.\n");
            if (TRUE == chg_is_batt_full())
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery is already full, vBat_sys = %dmV.\n",
                    chg_get_sys_batt_volt());
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Toggle CEN, continue to charge.\n");

                /*Toggle CEN, reset charge IC protection timer.*/
                if (FALSE == chg_set_charge_enable(FALSE))
                {
                    /*If error occured, set 1st bit of ret_code.*/
                    ret_code |= (1 << 1);
                }
                chg_delay_ms(CHG_TOGGLE_CEN_INTVAL_IN_MS);
                if (FALSE == chg_set_charge_enable(TRUE))
                {
                    /*If error occured, set 2nd bit of ret_code.*/
                    ret_code |= (1 << 2);
                }

                if (ret_code)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Toggle CEN pin error 0x%x.\n", ret_code);
                }
            }
        }
        else if (CHG_STOP_INVALID_TEMP == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stopped due to " \
                       "IC invalid temperature detected.\n");
            chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
        }
        else if (CHG_STOP_BY_SW == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stop due to SW control"   \
                       " in fast charge state.\n");
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid stop reason %d.\n", stp_reas);
        }
    }

    /*3. battery OVP, switch to maint state.*/
    else if (TRUE == chg_is_batt_ovp())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Batt-OVP while fastchg, vBat = %dmV, vbat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        chg_stm_switch_state(CHG_STM_MAINT_ST);
    }
    /*4. battery too cold/hot, switch to invalid charge temperature state.*/
    else if (FALSE == chg_is_batt_temp_valid())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid batt-temp while fastchg, tBat = %d'C, tBat_sys = %d'C.\n",
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
    }
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    else if(TRUE == is_batttemp_in_warm_chg_area())
    {

         chg_print_level_message(CHG_MSG_INFO,"CHG_STM:Switch fastchg state to warmchg state, tBat=%d'C,tBat_sys=%dC.\n",
         chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
         chg_stm_switch_state(CHG_STM_WARMCHG_ST);
    }
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    else
    {
        /* �Ǳ�׼��������������׶ξ�һ�� */
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
        chg_high_volt_batt_period_func();
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System keep staying at fast charging state.\n");
    }
}


static void chg_fastchg_state_exit_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from fast-charge state.\n");
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
    chg_sub_low_temp_changed = -1;      /*0-10�ȳ����ز�����ʼ��*/
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/
}


static void chg_maint_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into maintenance state.\n");

    /*1. Notify UI that battery charging stopped.*/
    chg_notify_UI_charging_state(CHG_UI_STOP_CHARGING);

    /*2. Update charge parameter, set maximal PS. current limit.*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    else if(CHG_HVDCP_CHGR == cur_chgr_type)
    {
        if(CHG_HVDCP_5V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_MAINT_ST]);
        }
        else if(CHG_HVDCP_9V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_9v_hvdcp_chgr_hw_paras[CHG_STM_MAINT_ST]);
        }
        else if(CHG_HVDCP_12V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_12v_hvdcp_chgr_hw_paras[CHG_STM_MAINT_ST]);
        }
        else
        {
            //reserve
        }
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_weak_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_maint_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
    else if((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type)
            || (CHG_CHGR_UNKNOWN == cur_chgr_type))
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }

    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_maint_state_entry_func disable wireless chg!\n");
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_maint_state_entry_func Is not wireless !\n");
    }
#else
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_maint_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
    else
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
}


static void chg_maint_state_period_func(void)
{
    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg removed force maint_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }
    else if (TRUE == chg_is_batt_need_rechg() && FALSE == chg_is_powdown_charging())
    {
        if (FALSE == chg_is_batt_temp_valid())
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery temperature %d invlaid for charge.\n",  \
                       chg_get_sys_batt_temp());
            chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:batt_need_rechg force to TRANSIT_ST.\n");
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Keep staying at maint_state.\n");
    }
}


static void chg_maint_state_exit_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from maintenance state.\n");
}


static void chg_invalid_chg_temp_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into invalid chg temp state.\n");

    /*1. Notify UI that battery charging stopped.*/
    chg_notify_UI_charging_state(CHG_UI_STOP_CHARGING);

    /*2. Update charge parameter, set maximal PS. current limit.*/
    /*3. Stop charging.*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {

        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    else if(CHG_HVDCP_CHGR == cur_chgr_type)
    {
        if(CHG_HVDCP_5V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
        }
        else
        {
           (void)chg_set_hvdcp_adpter_vol(CHG_HVDCP_5V);
            chg_delay_ms(1);
            chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
        }
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_weak_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:chg_invalid_chg_temp_state_entry_funcset usb 3.0 parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
    else if((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type)
            || (CHG_CHGR_UNKNOWN == cur_chgr_type))
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }

    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_invalid_chg_temp_state_entry_func disable wireless chg!\n");
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_invalid_chg_temp_state_entry_func Is not wireless !\n");
    }
#else
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_invalid_chg_temp_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
    else
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
}


static void chg_invalid_chg_temp_state_period_func(void)
{
    /*1. IF charger_remove
            Switch to battery only state.
      */
    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg removed force invalid_chg_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }
    /*2. If the battery temperature resumed?
     ->Y a. If battery OVP detected, switch to maintenance state.
            else switch to fast charge state.
     ->N b. Keep staying at invalid charge temperature state.
     */
    else if (TRUE == chg_is_batt_temp_valid())
    {
        if (chg_is_batt_ovp())
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery OVP, batt-volt %dmV while temp resume.\n", \
                       chg_get_sys_batt_volt());
            chg_stm_switch_state(CHG_STM_MAINT_ST);
        }
        else
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Battery temp not resumed, staying at invalid temp.\n");
    }

}


static void chg_invalid_chg_temp_state_exit_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from invalid chg temp state.\n");
}


static void chg_batt_only_state_entry_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into batt-only state.\n");
    /*1. Notify UI to stop battery charging display.*/
    chg_notify_UI_charging_state(CHG_UI_STOP_CHARGING);

    chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
    /*2. Set current charger type to invalid, means charger absent.*/
    chg_stm_set_chgr_type(CHG_CHGR_INVALID);

    /*����Ѿ����л��������״̬�ˣ���⵽USB���߱�׼�����������ONLINE���ͽ�����ΪOFFLINE*/
    if(TRUE == chg_get_usb_online_status())
    {
        chg_set_usb_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System enter batt-only state USB already OFFLINE!.\n");
    }

    if(TRUE == chg_get_ac_online_status())
    {
        chg_set_ac_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System enter batt-only state AC already OFFLINE!.\n");
    }
    /*3. Config charge hw parameter, disable charge, PS current limit min.*/
    chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_BATTERY_ONLY]);
}


static void chg_batt_only_state_period_func(void)
{
    static uint32_t chg_poll_timer_switch_cnter = 0;
    int32_t         curr_polling_timer_mode     = chg_poll_timer_get();
    boolean         is_emergency_state          = chg_is_emergency_state();
    boolean is_chgr_present      = chg_is_charger_present();

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:batt_only_state_period is_chgr_present=%d.\n",
                            is_chgr_present);

    /*Get poll timer period error, do nothing, return.*/
    if (CHG_ERROR == curr_polling_timer_mode)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Get poll timer period error.\n");
        return;
    }

    /*1. If we stay at batt_only state more than CHG_SWITCH_TO_SLOW_POLL_INTERVAL_IN_SEC
         seconds, switch poll timer to slow mode.
         Notice: DON'T switch to slow timer mode if system in emergency state. e.g.,
                 low power, battery hot/cold, etc, supported by sampling sub-module.*/
    if (FAST_POLL_CYCLE == curr_polling_timer_mode && !is_emergency_state)
    {
        if (chg_poll_timer_switch_cnter >= CHG_SWITCH_TO_SLOW_POLL_INTERVAL_IN_SEC)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System stay at batt-only state" \
                " more than %d seconds, switch to slow timer mode.\n",
                CHG_SWITCH_TO_SLOW_POLL_INTERVAL_IN_SEC);
            /*Clear cnter for next count.*/
            chg_poll_timer_switch_cnter = 0;
            chg_poll_timer_set(SLOW_POLL_CYCLE);
        }
        else
        {
            chg_poll_timer_switch_cnter += ((uint32_t)curr_polling_timer_mode / MS_IN_SECOND);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Poll switch cnter = %d.\n", chg_poll_timer_switch_cnter);
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:%s polling mode, system %s at emergency State.\n",
            (curr_polling_timer_mode == FAST_POLL_CYCLE) ? "Fast" : "Slow",
            (is_emergency_state) ? "is" : "isn't");

        /*Clear cnter for next count.*/
        chg_poll_timer_switch_cnter = 0;
    }

#ifdef CHG_STUB
    /*��ŵ������ֲ���ڣ����USB������������ͼ��û��ʵ��
      �����������д����ͬʱ�ڵ����״̬ͨ������ԴоƬ���ж�
      ������Ƿ���λ*/
    if( TRUE == is_chgr_present )
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:CHG_STUB:batt_only_state_period: CHGR IS PRESENT switch BATT_ONLY to TRANSIT_ST!\n");
        chg_stm_switch_state(CHG_STM_TRANSIT_ST);
    }
#endif/*CHG_STUB*/


#if (MBB_CHG_WIRELESS == FEATURE_ON)
    if(ONLINE == g_wireless_online_flag)
    {
        chg_stm_set_wireless_online_st(TRUE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_stm_set_chgr_type(CHG_WIRELESS_CHGR);
        chg_set_cur_chg_mode(CHG_WIRELESS_MODE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:wireless online change to CHG_WIRELESS_MODE!\n");
        chg_stm_switch_state(CHG_STM_TRANSIT_ST);
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System staying at batt-only state.\n");
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
}


static void chg_batt_only_state_exit_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from batt-only state.\n");
}

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)

void chg_stm_set_pre_state(chg_stm_state_type pre_state)
{
    chg_stm_state_info.pre_stm_state = pre_state;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:set_pre_stm_state=%d\n",pre_state);
}


chg_stm_state_type chg_stm_get_pre_state(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_pre_stm_state=%d\n",\
                        chg_stm_state_info.pre_stm_state);
    return chg_stm_state_info.pre_stm_state;
}


boolean is_batttemp_in_warm_chg_area( void )
{
    boolean rtnValue = FALSE;
    static boolean last_rtnValue = FALSE;

    /*���ǰһ��״̬�ǵ�ص�����λ״̬��˵�������˲�����Դ�¼���
       ������״̬��¼�����ָ���Ĭ��ֵ������*/
    if (CHG_STM_BATTERY_ONLY == chg_stm_get_pre_state())
    {
        last_rtnValue = FALSE;
    }

    if ( FALSE == last_rtnValue )
    {
        /*����¶ȴ��ڵ���45��С��55��Ϊ���³����*/
        if ( ( CHG_WARM_CHARGE_ENTER_THRESHOLD <= chg_real_info.battery_temp ) \
             && ( CHG_OVER_TEMP_STOP_THRESHOLD > chg_real_info.battery_temp ) )
        {
            rtnValue = TRUE;
        }
        else
        {
            rtnValue = FALSE;
        }
    }
    else
    {
        /*С��42�Ȼ��ߴ���55�ȵ��岻�ڸ��³������ */
        if ( (CHG_WARM_CHARGE_EXIT_THRESHOLD > chg_real_info.battery_temp)\
             || (CHG_OVER_TEMP_STOP_THRESHOLD <= chg_real_info.battery_temp))
        {
            rtnValue = FALSE;
        }
        else
        {
            rtnValue = TRUE;
        }
    }

    last_rtnValue = rtnValue;
    return rtnValue;
}


static void chg_warmchg_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into warm-charge state.\n");

    /*1.If the pre chg state is not CHG_STM_FAST_CHARGE_ST Notify UI
       that battery charging started.*/
    if ( CHG_STM_FAST_CHARGE_ST != chg_stm_get_pre_state())
    {
        chg_notify_UI_charging_state(CHG_UI_START_CHARGING);
    }
#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)
    else if(CHG_HVDCP_CHGR == cur_chgr_type)
    {
        if(CHG_HVDCP_5V == hvdcp_type)
        {
            chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
        }
        else
        {
           (void)chg_set_hvdcp_adpter_vol(CHG_HVDCP_5V);
            chg_delay_ms(1);
            chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
        }
    }
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
    /*2. Set charging hardware parameter per charger type.*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {
        /*ʹ�ܳ�磬���ñ�־*/
        chg_en_flag = 1;
        chg_en_timestamp = jiffies;

        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }
    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_weak_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_warmchg_state_entry_func]:set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
    else if((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type)
            || (CHG_CHGR_UNKNOWN == cur_chgr_type))
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }

    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_warmchg_state_entry_func enable wireless chg!\n");
        /*�������߳�����������ĸ��³�����*/
        if(FALSE == chg_is_powdown_charging())
        {
            chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
        }
        /*�ػ����߳�����ùػ��ĸ��³�������ǰ������900MA,�������1024MA,��ѹ��ѹ4.1V
            ��ֹ����256MA*/
        else
        {
            chg_set_hardware_parameter(&chg_wireless_poweroff_warmchg_paras);
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_warmchg_state_entry_func Is not wireless !\n");
    }
#else
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_warmchg_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }
    else
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
}


static void chg_warmchg_state_period_func(void)
{
    chg_stop_reason stp_reas        = CHG_STOP_COMPLETE;
    uint32_t ret_code               = 0;

    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
    /*1. IF charger_remove
            Switch to battery only state.
      */
    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg removed force warmchg_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }

    else if (FALSE == chg_is_IC_charging())
    {
        stp_reas = chg_get_stop_charging_reason();

        if (CHG_STOP_COMPLETE == stp_reas)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Charge Completed, vBat = %dmV, vBat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
            if (TRUE == chg_is_batt_full())
            {
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
#else
            chg_stm_switch_state(CHG_STM_MAINT_ST);
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
        }
        else if (CHG_STOP_TIMEOUT == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge timeout from IC.\n");
            if (TRUE == chg_is_batt_full())
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery is already full, vBat_sys = %dmV.\n",
                    chg_get_sys_batt_volt());
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Toggle CEN, continue to charge.\n");

                /*Toggle CEN, reset charge IC protection timer.*/
                if (FALSE == chg_set_charge_enable(FALSE))
                {
                    /*If error occured, set 1st bit of ret_code.*/
                    ret_code |= (1 << 1);
                }
                chg_delay_ms(CHG_TOGGLE_CEN_INTVAL_IN_MS);
                if (FALSE == chg_set_charge_enable(TRUE))
                {
                    /*If error occured, set 2nd bit of ret_code.*/
                    ret_code |= (1 << 2);
                }

                if (ret_code)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Toggle CEN pin error 0x%x.\n", ret_code);
                }
            }
        }
        else if (CHG_STOP_INVALID_TEMP == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stopped due to " \
                       "IC invalid temperature detected.\n");
            chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
        }
        else if (CHG_STOP_BY_SW == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stop due to SW control"   \
                       " in fast charge state.\n");
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid stop reason %d.\n", stp_reas);
        }
    }
    /*3. battery OVP, switch to maint state.*/
    else if (TRUE == chg_is_batt_ovp())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Batt-OVP while warmchg, vBat = %dmV, vbat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        chg_stm_switch_state(CHG_STM_MAINT_ST);
    }

    /*4. battery too cold/hot, switch to invalid charge temperature state.*/
    else if (FALSE == chg_is_batt_temp_valid())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid batt-temp while warmchg, tBat = %d'C, tBat_sys = %d'C.\n",
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
    }
    /*5.battery temp resume switch to fastchg state*/
    else if(FALSE == is_batttemp_in_warm_chg_area())
    {

         chg_print_level_message(CHG_MSG_INFO, "CHG_STM:batt-temp resume form warmchg to fastchg, tBat = %d'C, tBat_sys = %d'C.\n",
         chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
         if(CHG_HVDCP_CHGR == cur_chgr_type)
         {
            chg_stm_switch_state(CHG_STM_HVDCP_CHARGE_ST);
         }
         else
         {
            chg_stm_switch_state(CHG_STM_FAST_CHARGE_ST);
         }
    }
    else
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System keep staying at warmchg state.\n");
    }

}


static void chg_warmchg_state_exit_func(void)
{
     chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from warmchg state.\n");

}
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

#if (FEATURE_ON == MBB_CHG_HVDCP_CHARGE)

static void chg_hvdcpchg_state_entry_func(void)
{
    int32_t cur_batt_volt =  chg_get_cur_batt_volt();
    chg_hvdcp_type_value hvdcp_type = chg_get_hvdcp_type();

    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into hvdcp-charge state.\n");

    /*1. Notify UI that battery charging started.*/
    chg_notify_UI_charging_state(CHG_UI_START_CHARGING);

    /*2. Set HVDCP adapter voltage.*/
    (void)chg_set_hvdcp_adpter_vol(g_hvdcp_init_vol);
    chg_delay_ms(1);
    hvdcp_type = chg_get_hvdcp_type();

    /*3. Set charging hardware parameter according to HVDCP voltage type.*/
    if(CHG_HVDCP_9V == hvdcp_type)
    {
        chg_set_hardware_parameter(&chg_9v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST]);
    }
    else if( CHG_HVDCP_12V == hvdcp_type)
    {
        chg_set_hardware_parameter(&chg_12v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST]);
    }
    else
    {
        chg_set_hardware_parameter(&chg_5v_hvdcp_chgr_hw_paras[CHG_STM_HVDCP_CHARGE_ST]);
    }


    if(cur_batt_volt <= HVDCP_CHARGE_SET_CV_COMPARE_TH)
    {
        if (CHG_BATT_ID_DEF != chg_get_batt_id())
        {
            chg_set_vreg_level(HVDCP_CHARGE_CV_4200MV);
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "Unknown batt type, use safer chg paras.\n");
            chg_set_vreg_level(CHG_DEF_BATT_VREG_LEVEL);
        }
    }
    else
    {
        if (CHG_BATT_ID_DEF != chg_get_batt_id())
        {
            chg_set_vreg_level(HVDCP_CHARGE_CV_4350MV);
            chg_set_cur_level(HVDCP_CHARGE_0P5_CURRENT_TH);
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "Unknown batt type, use safer chg paras.\n");
            chg_set_vreg_level(CHG_DEF_BATT_VREG_LEVEL);
#if (FEATURE_ON == MBB_CHG_BQ24196)
            chg_set_current_level(CHG_DEF_BATT_CUR_LEVEL, TRUE);
#else /*!MBB_CHG_BQ24196*/
/*chg_set_current_level���������IC������δ����*/
#error You need check HOWTO implement this on your charger IC!!
#endif
        }
        g_hvdcp_charge_currnt_limit_flag = TRUE;
    }

#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
    chg_high_volt_batt_entry_func();
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/
}


static void chg_hvdcpchg_state_period_func(void)
{

    chg_stop_reason stp_reas        = CHG_STOP_COMPLETE;
    uint32_t ret_code               = 0;
    uint32_t charge_current         = chg_get_hvdcp_charge_current();

    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chgr removed force hvdcpchg_state to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }
    else if((charge_current <= HVDCP_CHARGE_0P5_CURRENT_TH)
        && (FALSE == g_hvdcp_charge_currnt_limit_flag))
    {
        g_hvdcp_charge_currnt_limit_flag = TRUE;
        if (CHG_BATT_ID_DEF != chg_get_batt_id())
        {
            chg_set_vreg_level(HVDCP_CHARGE_CV_4350MV);
            chg_set_cur_level(HVDCP_CHARGE_0P5_CURRENT_TH);
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "Unknown batt type, use safer chg paras.\n");
            chg_set_vreg_level(CHG_DEF_BATT_VREG_LEVEL);
#if (FEATURE_ON == MBB_CHG_BQ24196)
            chg_set_current_level(CHG_DEF_BATT_CUR_LEVEL, TRUE);
#else /*!MBB_CHG_BQ24196*/
/*chg_set_current_level���������IC������δ����*/
#error You need check HOWTO implement this on your charger IC!!
#endif
        }
    }
    else if (FALSE == chg_is_IC_charging())
    {
        stp_reas = chg_get_stop_charging_reason();

        if (CHG_STOP_COMPLETE == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge Completed, vBat = %dmV, vBat_sys = %dmV.\n",
                chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
            chg_stm_switch_state(CHG_STM_MAINT_ST);
        }
        else if (CHG_STOP_TIMEOUT == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge timeout from IC.\n");
            if (TRUE == chg_is_batt_full())
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery is already full, vBat_sys = %dmV.\n",
                    chg_get_sys_batt_volt());
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Toggle CEN, continue to charge.\n");

                if (FALSE == chg_set_charge_enable(FALSE))
                {
                    ret_code |= (1 << 1);
                }
                chg_delay_ms(CHG_TOGGLE_CEN_INTVAL_IN_MS);
                if (FALSE == chg_set_charge_enable(TRUE))
                {
                    ret_code |= (1 << 2);
                }

                if (ret_code)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Toggle CEN pin error 0x%x.\n", ret_code);
                }
            }
        }
        else if (CHG_STOP_BY_SW == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stop due to SW control"   \
                       " in fast charge state.\n");
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid stop reason %d.\n", stp_reas);
        }
    }

    /*3. battery OVP, switch to maint state.*/
    else if (TRUE == chg_is_batt_ovp())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Batt-OVP while hvdcpchg, vBat = %dmV, vbat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        chg_stm_switch_state(CHG_STM_MAINT_ST);
    }
    /*4. battery too cold/hot, switch to invalid charge temperature state.*/
    else if (FALSE == chg_is_batt_temp_valid())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid batt-temp while hvdcpchg, tBat = %d'C, tBat_sys = %d'C.\n",
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
    }
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    else if(TRUE == is_batttemp_in_warm_chg_area())
    {

         chg_print_level_message(CHG_MSG_INFO,"CHG_STM:Switch hvdcpchg state to warmchg state, tBat=%d'C,tBat_sys=%dC.\n",
         chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
         (void)chg_set_hvdcp_adpter_vol(CHG_HVDCP_5V);
         chg_stm_switch_state(CHG_STM_WARMCHG_ST);
    }
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    else
    {
        /* �Ǳ�׼��������������׶ξ�һ�� */
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
        chg_high_volt_batt_period_func();
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System keep staying at hvdcpchg charging state.\n");
    }

}


static void chg_hvdcpchg_state_exit_func(void)
{
    g_hvdcp_charge_currnt_limit_flag = FALSE;
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from hvdcp-charge state.\n");
}
#endif/*(FEATURE_ON == MBB_CHG_HVDCP_CHARGE)*/
#if (MBB_CHG_EXTCHG == FEATURE_ON)

void extchg_stop_threshold_to_voltage(int32_t capacity)
{
    switch (capacity)
    {
        case EXTCHG_DEFAULT_STOP_THRESHOLD:
            g_extchg_voltage_threshold = EXTCHG_DEFAULT_STOP_VOLTAGE;
            break;
        case EXTCHG_STOP_CAPACITY_TEN:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_TEN;
            break;
        case EXTCHG_STOP_CAPACITY_TWENTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_TWENTY;
            break;
        case EXTCHG_STOP_CAPACITY_THIRTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_THIRTY;
            break;
        case EXTCHG_STOP_CAPACITY_FORTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_FORTY;
            break;
        case EXTCHG_STOP_CAPACITY_FIFTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_FIFTY;
            break;
        case EXTCHG_STOP_CAPACITY_SIXTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_SIXTY;
            break;
        case EXTCHG_STOP_CAPACITY_SEVENTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_SEVENTY;
            break;
        case EXTCHG_STOP_CAPACITY_EIGHTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_EIGHTY;
            break;
        case EXTCHG_STOP_CAPACITY_NINETY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_NINETY;
            break;
        case EXTCHG_STOP_CAPACITY_HUNDRED:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_HUNDRED;
            break;
        default:
            break;
    }
}


void chg_extchg_config_data_init(void)
{
    int32_t fd    = 0;
    mm_segment_t fs;
    char extchg_threshold_temp = 0;
    char extchg_diable_st_temp = 0;
    int32_t extchg_capacity_threshold = 0;

    fs = get_fs();
    set_fs(KERNEL_DS);
    fd = sys_open(EXTCHG_THRESHOLD_PATH, O_RDWR, 0);
    if(fd >= 0)
    {
        sys_read(fd, &extchg_threshold_temp, 1);
        extchg_capacity_threshold = extchg_threshold_temp;
        chg_print_level_message(CHG_MSG_INFO,"extchg_capacity_threshold=%d.\n", extchg_capacity_threshold);
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
        g_extchg_stop_soc_threshold = extchg_capacity_threshold;
#else
        extchg_stop_threshold_to_voltage(extchg_capacity_threshold);
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
        sys_close(fd);
    }
    else
    {
/*DC04 Ĭ�϶�����ֹͣ�ĵ���������30%��*/
#if defined(BSP_CONFIG_BOARD_E5_DC04)
        extchg_capacity_threshold = EXTCHG_STOP_CAPACITY_THIRTY;
#else
        extchg_capacity_threshold = EXTCHG_DEFAULT_STOP_THRESHOLD;
#endif
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
        g_extchg_stop_soc_threshold = extchg_capacity_threshold;
#else
        extchg_stop_threshold_to_voltage(extchg_capacity_threshold);
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
        chg_print_level_message(CHG_MSG_INFO,"UI set extchg_threshold read FAIL use default value 5!\n");
    }

/*DC04 ��֧��Touch UI�˵�����/���ö����繦�ܣ������繦�����ÿ�����*/
#if !defined(BSP_CONFIG_BOARD_E5_DC04)
    fd = sys_open(EXTCHG_DISABLE_PATH, O_RDWR, 0);
    if(fd >= 0)
    {
        sys_read(fd, &extchg_diable_st_temp, 1);
        g_extchg_diable_st = extchg_diable_st_temp;
        chg_print_level_message(CHG_MSG_INFO,"g_extchg_diable_st=%d.\n", g_extchg_diable_st);
        sys_close(fd);
    }
    else
    {
        g_extchg_diable_st = 0;
        chg_print_level_message(CHG_MSG_INFO,"UI set extchg_diable_st read FAIL use default value 0!\n");
    }
#endif
    set_fs(fs);
}


boolean chg_get_extchg_online_status(void)
{
    int32_t vbus_volt = 0;

    //TO DO:1:����USB�����ӿڶ̽�D+ ,D-
    usb_notify_event(USB_OTG_CONNECT_DP_DM,NULL);

    /*ʹ�����߳��*/
    chg_set_extchg_chg_enable(TRUE);

    /*��ʱ500MS,��ֹʹ�ܳ�����Ϊ��·�ӳٵ��²ɼ���ص�ѹ�쳣*/
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_PLT:chg_delay_ms 500MS! \n");
    chg_delay_ms(EXTCHG_DELAY_COUNTER_SIZE);
    vbus_volt = chg_get_volt_from_adc(CHG_PARAMETER__VBUS_VOLT);
    chg_print_level_message( CHG_MSG_INFO,"CHG_DRV:get_extchg_online vbus_volt = %d\r\n ",vbus_volt);
    if(vbus_volt > VBUS_JUDGEMENT_THRESHOLD)
    {
        return ONLINE;
    }
    else
    {
        chg_print_level_message( CHG_MSG_ERR,"CHG_PLT: VBUS detect failure extchg is not online\r\n ");
        return OFFLINE;
    }
}

#endif /* MBB_CHG_EXTCHG */


int32_t chg_get_batt_id_volt(void)
{
    if(0 == chg_real_info.battery_id_volt)
    {
        chg_real_info.battery_id_volt = chg_get_volt_from_adc(CHG_PARAMETER__BATTERY_ID_VOLT);
    }
    chg_print_level_message(CHG_MSG_ERR,"CHG_STM:chg_get_batt_id_volt id_volt=%d\n",chg_real_info.battery_id_volt);
    return chg_real_info.battery_id_volt;
}


int32_t chg_get_bat_status(void)
{
    int32_t bat_stat_t = POWER_SUPPLY_STATUS_UNKNOWN;
    chg_stm_state_type stm_status = CHG_STM_MAX_ST;

    stm_status = chg_stm_get_cur_state();
    if((CHG_STM_INIT_ST == stm_status) || (CHG_STM_TRANSIT_ST == stm_status)
        || (CHG_STM_INVALID_CHG_TEMP_ST == stm_status)
        || (CHG_STM_BATTERY_ONLY == stm_status))
    {
        bat_stat_t = POWER_SUPPLY_STATUS_NOT_CHARGING;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=NOT_CHARGING\n");
    }
    else if ((CHG_STM_FAST_CHARGE_ST == stm_status)
            || (CHG_STM_WARMCHG_ST == stm_status)
            || (CHG_STM_HVDCP_CHARGE_ST == stm_status))
    {
        bat_stat_t = POWER_SUPPLY_STATUS_CHARGING;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=CHARGING\n");
    }
    else if(CHG_STM_MAINT_ST == stm_status)
    {
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
        if(BATT_CAPACITY_FULL == chg_get_sys_batt_capacity())
        {
            bat_stat_t = POWER_SUPPLY_STATUS_FULL;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=STATUS_FULL\n");
        }
#else
        bat_stat_t = POWER_SUPPLY_STATUS_FULL;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=STATUS_FULL\n");
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
    }
    else
    {
#if (MBB_CHG_COMPENSATE == FEATURE_ON)
        if(POWER_SUPPLY_STATUS_NEED_SUPPLY == chg_stm_state_info.bat_stat_type)
        {
            bat_stat_t = POWER_SUPPLY_STATUS_NEED_SUPPLY;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=NEED_SUPPLY\n");
        }
        else if(POWER_SUPPLY_STATUS_SUPPLY_SUCCESS == chg_stm_state_info.bat_stat_type)
        {
            bat_stat_t = POWER_SUPPLY_STATUS_SUPPLY_SUCCESS;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=SUPPLY_SUCCESS\n");
        }
        else
        {
            bat_stat_t = POWER_SUPPLY_STATUS_UNKNOWN;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=STATUS_UNKNOWN\n");
        }
#else
        bat_stat_t = POWER_SUPPLY_STATUS_UNKNOWN;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=STATUS_UNKNOWN\n");
#endif/*MBB_CHG_COMPENSATE == FEATURE_ON*/
    }
    return bat_stat_t;
}


int32_t chg_get_bat_health(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_health=%d\n",chg_stm_state_info.bat_heath_type);
    return chg_stm_state_info.bat_heath_type;
}


void chg_set_extchg_status(int32_t extchg_status)
{
    chg_stm_state_info.extchg_status = extchg_status;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:extchg_status=%d\n",chg_stm_state_info.extchg_status);
}
#if((FEATURE_ON == MBB_FACTORY)\
     && (FEATURE_ON == MBB_CHG_COULOMETER)\
     && (FEATURE_ON == MBB_CHG_EXTCHG ))

void chg_check_vbus_volt(void)
{

    int32_t checkstarttime = 0;
    int32_t vbus_volt = 0;
    int32_t checkvbustime = 0;

    msleep(COUL_CHECK_CURRENT_WAIT_MS);
    /*���ѭ����ȡVbus��ѹ10�Σ��ܼ�5s*/
    while(COUL_CHECK_CURRENT_TIME_MAX > checkstarttime)
    {
        checkstarttime++;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:checkstarttime=%d\n",checkstarttime);
        msleep(COUL_CHECK_CURRENT_DELAY_MS);
        /*��⵽Vbus��ѹ���ߺ�ѭ����ȡ3�Σ���ֹ��ѹ�������*/
        while(checkvbustime < COUL_CHECK_VBUS_TIME_MAX)
        {
            checkvbustime++;
            vbus_volt = chg_get_volt_from_adc(CHG_PARAMETER__VBUS_VOLT);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:vbus_volt=%d\n",vbus_volt);
            if(VBUS_JUDGEMENT_THRESHOLD >= vbus_volt)
            {
                break;
            }
            else
            {
                //do nothing
            }
            msleep(COUL_CHECK_VBUS_DELAY_MS);
        }
        /*�ж�3��Vbus��ѹ�Ƿ��Ǳ���ȷ����*/
        if(COUL_CHECK_VBUS_TIME_MAX == checkvbustime)
        {
            g_extchg_start = 1;
            return;
        }
        else
        {
            g_extchg_start = 0;
        }
    }
}


int32_t chg_get_extchg_start(void)
{
    return g_extchg_start;
}
#endif


int32_t chg_get_extchg_status(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:extchg_status=%d\n",chg_stm_state_info.extchg_status);
    return chg_stm_state_info.extchg_status;
}


void chg_charge_paras_init(void)
{
    switch(g_hardware_version_id)
    {
        /*��Ʒ���ݰ汾�������Ӧ��صĳ������������Ͳ�ƷӲ����صĲ�����������������*/
        case HW_VER_PRODUCT_SB03:
        {
            chg_std_chgr_hw_paras = chg_std_2A_chgr_hw_paras_def;
            chg_usb_chgr_hw_paras = chg_usb_chgr_hw_paras_def;
            chg_weak_chgr_hw_paras = chg_weak_chgr_hw_paras_def;
            g_default_dpm_volt = 4520;/*����Ĭ��DPM*/
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Read SB03 chg parameter.\n");
            break;
        }
        case HW_VER_PRODUCT_DC04:
        {
            chg_std_chgr_hw_paras  = chg_std_2A_chgr_hw_paras_4800mAh_batt;
            chg_usb_chgr_hw_paras  = chg_usb_chgr_hw_paras_4800mAh_batt;
            chg_usb3_chgr_hw_paras = chg_usb3_chgr_hw_paras_4800mAh_batt;
            chg_weak_chgr_hw_paras = chg_weak_chgr_hw_paras_def;
            g_default_dpm_volt     = 4520;/*����Ĭ��DPM*/
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Read DC04 chg parameter.\n");
            break;
        }
        default:
        {
            chg_std_chgr_hw_paras = chg_std_1A_chgr_hw_paras_def;
            chg_usb_chgr_hw_paras = chg_usb_chgr_hw_paras_def;
            chg_usb3_chgr_hw_paras = chg_usb3_chgr_hw_paras_def;
            chg_weak_chgr_hw_paras = chg_weak_chgr_hw_paras_def;
#if (MBB_CHG_WIRELESS == FEATURE_ON)
            chg_wireless_chgr_hw_paras = chg_wireless_chgr_hw_paras_def;
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
            g_default_dpm_volt = 4520;/*����Ĭ��DPM*/
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Use default chg parameter!.\n");
            break;
        }
    }
}


int32_t chg_stm_init(void)
{
    static boolean chg_stm_inited = FALSE;
    chg_stm_state_type cur_chg_state = chg_stm_get_cur_state();
    chg_chgr_type_t cur_chg_type = chg_stm_get_chgr_type();
    CHG_MODE_ENUM cur_chg_mode = chg_get_cur_chg_mode();
    chg_stm_state_type pre_chg_state = chg_stm_get_pre_state();
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    boolean is_wireless_online   = chg_stm_get_wireless_online_st();
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    boolean is_extchg_online   = chg_stm_get_extchg_online_st();
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/

    if (FALSE == chg_stm_inited)
    {
        /*Clear fast charge sw protection timer(cnter).*/

        chg_stm_state_info.charging_lasted_in_sconds = 0;
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
        chg_stm_state_info.charger_lasted_without_charging_in_seconds = 0;
#endif
        /*Initialize default chgr type and stm state. */
        if((cur_chg_mode <= CHG_MODE_INIT) || (cur_chg_mode >= CHG_MODE_UNKNOW))
        {
            chg_set_cur_chg_mode(CHG_MODE_UNKNOW);
        }
        if((cur_chg_type <= CHG_CHGR_UNKNOWN) || (cur_chg_type >= CHG_CHGR_INVALID))
        {
            chg_stm_set_chgr_type(CHG_CHGR_UNKNOWN);
        }
        if((cur_chg_state <= CHG_STM_INIT_ST) || (cur_chg_state >= CHG_STM_MAX_ST))
        {
            chg_stm_set_cur_state(CHG_STM_TRANSIT_ST);
        }

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
        if((pre_chg_state <= CHG_STM_INIT_ST) || (pre_chg_state >= CHG_STM_MAX_ST))
        {
            chg_stm_set_pre_state(CHG_STM_TRANSIT_ST);
        }
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

#if (MBB_CHG_EXTCHG == FEATURE_ON)
        if(TRUE != is_extchg_online)
        {
            chg_stm_set_extchg_online_st(FALSE);
        }
#endif /* MBB_CHG_EXTCHG == FEATURE_ON */

#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE != is_wireless_online)
        {
            chg_stm_set_wireless_online_st(FALSE);
        }
#endif /* MBB_CHG_WIRELESS == FEATURE_ON */

        if(CHG_STM_INIT_ST == cur_chg_state)
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
        mlog_print(MLOG_CHG, mlog_lv_info, "CHG STM initial state:\n");
        mlog_print(MLOG_CHG, mlog_lv_info, "current charge state: %d, pre-chargestate: %d.\n",
                   cur_chg_state, pre_chg_state);
        mlog_print(MLOG_CHG, mlog_lv_info, "charger type: %d, charge mode: %d.\n",
                   cur_chg_type, cur_chg_mode);

#if (MBB_CHG_EXTCHG == FEATURE_ON)
        mlog_print(MLOG_CHG, mlog_lv_info, "extchg_online: %d.\n", is_extchg_online);
#endif /*MBB_CHG_EXTCHG == FEATURE_ON*/

#if (MBB_CHG_WIRELESS == FEATURE_ON)
        mlog_print(MLOG_CHG, mlog_lv_info, "wireless_online: %d.\n", is_wireless_online);
#endif /*MBB_CHG_WIRELESS == FEATURE_ON*/

        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge State Machine init successfully.\n");
        chg_stm_inited = TRUE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge STM init be called more than one time.\n");
    }

    return CHG_OK;
}


void chg_set_usb_online_status(boolean online)
{
    chg_stm_state_info.usb_online_st = online;
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_state_info.usb_online_st=%d\n",online);
}


boolean chg_get_usb_online_status(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_get_usb_online_status=%d\n",\
                chg_stm_state_info.usb_online_st);
    return chg_stm_state_info.usb_online_st;
}


void chg_set_ac_online_status(boolean online)
{
    chg_stm_state_info.ac_online_st = online;
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_state_info.ac_online_st=%d\n",online);
}


boolean chg_get_ac_online_status(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_get_ac_online_status=%d\n",\
                chg_stm_state_info.ac_online_st);
    return chg_stm_state_info.ac_online_st;
}


boolean chg_get_bat_present_status(void)
{
    if(TRUE == chg_get_batt_id_valid())
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_present_status=PRESENT\n");
        return PRESENT;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_present_status=UNPRESENT\n");
        return UNPRESENT;
    }
}


void chg_update_power_suply_info(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "******CHG_STM:update_power_suply_info begin!******\n");
    if (NULL == g_chip)
    {
        return;
    }
    g_chip->ac_online = chg_get_ac_online_status();
    g_chip->usb_online = chg_get_usb_online_status();
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    g_chip->wireless_online = chg_stm_get_wireless_online_st();
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    g_chip->extchg_online = chg_stm_get_extchg_online_st();
    g_chip->extchg_status = chg_get_extchg_status();
#if(FEATURE_ON == MBB_FACTORY) && (FEATURE_ON == MBB_CHG_COULOMETER)
    g_chip->extchg_start = chg_get_extchg_start();
#endif
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/
    g_chip->bat_present = chg_get_bat_present_status();
    g_chip->bat_stat = chg_get_bat_status();
    g_chip->bat_health = chg_get_bat_health();
    g_chip->bat_technology = POWER_SUPPLY_TECHNOLOGY_LION;
    g_chip->bat_avg_voltage = chg_get_sys_batt_volt();
    g_chip->bat_avg_temp = chg_get_sys_batt_temp();
    g_chip->bat_capacity = chg_get_sys_batt_capacity();
    g_chip->bat_time_to_full = chg_get_batt_time_to_full();
    /*USB�±�*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    g_chip->usb_health = chg_get_usb_health();
#endif

    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:update_power_suply_info successfully!\n");
}


void chg_print_test_view_info(void)
{
    static uint8_t count = 0;

    /*����ѯ4�����ڴ�ӡһ�γ����Ϣ������ѯÿ���ڴ�ӡһ��*/
    if ((FAST_POLL_CYCLE == chg_poll_timer_get()) && (count < 3))
    {
        count++;
        return;
    }
    
    count = 0;
    chg_print_level_message(CHG_MSG_ERR, "**************CHG Tester View Info Begin**********\n");
    /*��ӡ��ǰ�����ĳ��״̬*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:current_stm_state=%d\n",
                         chg_stm_state_info.cur_stm_state);
    /*��ӡ��ǰ���������*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:current_charger_type=%d\n",
                        chg_stm_state_info.cur_chgr_type);
    /*��ӡ��ǰ�����ĳ��ģʽ�����ߣ����߻��Ƕ�����*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:current_chg_mode=%d\n",
                        chg_stm_state_info.cur_chg_mode);
    /*��ӡ��ǰ������ʱ��*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:current_charging_lasted_time=%d\n",
                        chg_stm_state_info.charging_lasted_in_sconds);
    /*��ӡ���ε���¶�*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.battery_one_temp = %d\n",
                        chg_real_info.battery_one_temp);
     /*��ӡƽ����ĵ���¶�*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.battery_temp = %d\n",
                        chg_real_info.battery_temp);

    /*��ӡ��ص��βɼ���ѹ*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.battery_one_volt = %d\n",
                        chg_real_info.battery_one_volt);
    /*��ӡ��ص�ѹƽ��ֵ*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.battery_volt = %d\n",
                        chg_real_info.battery_volt);
    /*��ӡVPH_PWR���βɼ���ѹ*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.vph_pwr_one_volt=%d\n",
                        chg_real_info.vph_pwr_one_volt);
    /*��ӡVPH_PWR��ѹƽ��ֵ*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.vph_pwr_avg_volt= %d\n",
                        chg_real_info.vph_pwr_avg_volt);
    /*��ӡ��ص����ٷֱ�*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.bat_capacity = %d\n",
                        chg_real_info.bat_capacity);
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
    /*Dump current surface temperature.*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_DCM: TSurf: %d'C, TSurf_sys = %d'C.\n",
        g_surf_temp_state.cur_surf_temp, g_surf_temp_state.sys_surf_temp);
#endif
    /*��ӡUSB��ǰ�¶�*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_usb_cur_temp = %d'C.\n",
                        chg_usb_temp_info.usb_cur_temp);
#endif
/*��ӡ��ǰ����ͣ���������λʱ��*/
#if ( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:charger_lasted_without_charging_in_seconds=%d\n",
                        chg_stm_state_info.charger_lasted_without_charging_in_seconds);
#endif
    chg_print_level_message(CHG_MSG_ERR, "**************CHG Tester View Info End**********\n");
}

#ifdef CONFIG_MBB_FAST_ON_OFF
#if(MBB_REB == FEATURE_ON)

unsigned int chg_is_bat_only(void)
{
    return ( CHG_BAT_ONLY_MODE == chg_stm_state_info.cur_chg_mode );
}


int get_low_bat_level(void)
{
    /*Ҫ�ڳ��ģ���ʼ���Ժ����*/
    return g_chgBattVoltProtect.VbatLevelLow_MAX;
}
#endif/*MBB_REB*/


void chg_get_system_suspend_status(ulong64_t suspend_status)
{
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    if(suspend_status)
    {
        /*����ٹػ���ģ��USB ID�γ�*/
        if(TRUE == chg_stm_get_extchg_online_st())
        {
#if defined(BSP_CONFIG_BOARD_E5_DC04)
            /*Ϊ���Ż�DC04���ģ��ڶ����翪��������£�Wi-Fi�ر�ʱ��Suspend PCIE�������ڽ���ٹ�
            ��֮ǰ��Ҫ�Ƚ�PCIE��Ϊresume״̬�������ظ�suspend pcieʱ��ϵͳ������*/
            printk(KERN_ERR "CHG_STM:Enter Fastboot extchg online resume pcie!\n");
            pcie_resume_before_fastoff();
#endif
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Enter Fastboot extchg online stop extchg!\n");
            /*ģ��USB ID�γ�*/
            (void)battery_monitor_blocking_notifier_call_chain(0, CHG_EXGCHG_CHGR);
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Enter Fastboot extchg offline do nothing!\n");
        }
    }
    else
    {
        /*�˳��ٹػ�����USB�����ϱ� ID�жϣ����ģ�鲻������*/
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Exit Fastboot extchg offline do nothing!\n");
    }
#endif
}
#endif/*CONFIG_MBB_FAST_ON_OFF*/

#if (MBB_CHG_COULOMETER == FEATURE_ON)
/*�͵��ж�������*/
#define LOW_BATT_DET_THRESHOLD    (3450)

void chg_low_battery_event_handler(void)
{
    int32_t batt_volt = chg_get_batt_volt_value();

    if(TRUE == chg_is_powdown_charging()
        || TRUE == chg_is_low_battery_poweroff_disable())
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:no need to care��set bypass!\n");
        return;
    }
    if(LOW_BATT_DET_THRESHOLD < batt_volt)
    {
        batt_volt = chg_get_batt_volt_value();
        if(LOW_BATT_DET_THRESHOLD < batt_volt)
        {
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:invalid event��set bypass!\n");
            return;
        }
    }
    chg_print_level_message(CHG_MSG_ERR,"CHG_STM:battery is low power��system will shutdown!\n");
    wake_lock_timeout(&g_chip->alarm_wake_lock,(long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
    chg_set_sys_batt_capacity(BATT_CAPACITY_SHUTOFF);
    chg_bat_timer_set( LOW_BATT_SHUTOFF_DURATION, chg_set_power_off, DRV_SHUTDOWN_LOW_BATTERY);
}
#endif/*MBB_CHG_COULOMETER == FEATURE_ON*/

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)

void chg_sleep_batt_check_timer(void)
{
    int32_t batt_volt = 0;
    int32_t batt_temp = 0;

    wake_lock(&g_chip->chg_wake_lock);
    #ifdef CONFIG_COUL
    /*����һ�ο��ؼ�ǿ��У׼*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bsp_coul_cail_on COUL_CAIL_ON");
    bsp_coul_cail_on();
    msleep(COUL_READY_DELAY_MS);
    #endif
    batt_volt = (int32_t)chg_get_batt_volt_value();
    batt_temp = (int32_t)chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);

    chg_print_level_message(CHG_MSG_ERR,"CHG_STM:modem_notify_get_batt_info_callback, batt_volt %d,batt_temp %d!\n",
           batt_volt, batt_temp);

#if (MBB_CHG_EXTCHG == FEATURE_ON)
    /*����/����/����������λ�����е͵���߹��´���*/
    if ((FALSE == is_chg_charger_removed()) || (TRUE == chg_stm_get_extchg_online_st()))
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:modem_notify_get_batt_info_callback charger plug in do nothing!*****\n");
        goto end;
    }
#else
    if (FALSE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:modem_notify_get_batt_info_callback charger plug in do nothing!*****\n");
        goto end;
    }
#endif/*defined(MBB_CHG_EXTCHG)*/
#if (MBB_CHG_COULOMETER == FEATURE_ON)
    /*��ѯsoc���Ա�Ӧ�õ͵���ʾ*/
    chg_poll_batt_soc();
    if(batt_volt < BATT_VOLT_POWER_OFF_THR)
    {
        /*�͵�ػ�����״̬��ϵͳ�Ժ�ػ�*/
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(SHUTDOWN_WAKELOCK_TIMEOUT));
        chg_print_level_message(CHG_MSG_ERR,\
        "CHG_STM:batt_volt < poweroff voltage threshold report power off*****\n");
    }
#elif (MBB_CHG_BQ27510 == FEATURE_ON)
    /*just monitor batt volt*/
    if(batt_volt < BATT_VOLT_POWER_OFF_THR)
    {
        /*�͵�ػ�����״̬��ϵͳ�Ժ�ػ�*/
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(SHUTDOWN_WAKELOCK_TIMEOUT));
        chg_print_level_message(CHG_MSG_ERR,\
        "CHG_STM:batt_volt < poweroff voltage threshold report power off*****\n");
    }
#else
    /* �����ص�ѹ���ڵ͵�������ʲô����������������¼��ϱ�Ӧ�� */
    if (BATT_VOLT_LEVELLOW_MAX < batt_volt)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:batt_volt > low battery threshold do nothing!*****\n");
    }
    else if ((BATT_VOLT_POWER_OFF_THR < batt_volt) && (BATT_VOLT_LEVELLOW_MAX >= batt_volt))
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_volt is in 3450-3550mV report low battery*****\n");
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
        chg_set_sys_batt_capacity(BATT_CAPACITY_LEVELLOW);
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:send MSG to app for show low power! \n ");
    }
    else
    {
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:batt_volt < poweroff voltage threshold report power off*****\n");
        chg_set_sys_batt_capacity(BATT_CAPACITY_SHUTOFF);
    }
#endif
    /* ����¶ȱ�����ʹ�� ���� ����¶���������Χ��ʲô����������������¼��ϱ�Ӧ�� */
    if ((((SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_WARN_LEN) > batt_temp)
        && (SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD < batt_temp))
        || (FALSE == SHUTOFF_LOW_TEMP_PROTECT_ENABLE))
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_temp is in -20-58 deg do nothing*****\n");
    }
    else if ((SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_WARN_LEN) <= batt_temp)
    {
        if (SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD <= batt_temp)
        {
            wake_lock_timeout(&g_chip->alarm_wake_lock,
                (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_temp is more than 60 deg, power off*****\n");
            chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_DEAD;
            chg_send_stat_to_app((uint32_t)DEVICE_ID_KEY, (uint32_t)GPIO_KEY_POWER_OFF);
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_temp is in 58-60 deg, report overheat*****\n");
            wake_lock_timeout(&g_chip->alarm_wake_lock,
                (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
            chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_OVERHEAT;
            chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_HIGH);
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:send MSG to app for show overheat! \n ");
        }
    }
    else
    {
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_temp is less than -20 deg, report cold*****\n");
        chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_COLD;
        chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_LOW);
    }
    chg_print_level_message(CHG_MSG_ERR,"CHG_STM:wait next modem time expire!\n");

end:
    bsp_softtimer_add(&g_chg_sleep_timer);
    wake_unlock(&g_chip->chg_wake_lock);
}
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)

void chg_extchg_info_dump(void)
{
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:extchg_diable_st=%d\n",g_extchg_diable_st);
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:ui_mode=%d\n",g_ui_choose_exchg_mode);
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:otg_extchg=%d\n",is_otg_extchg);
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:is_extchg_ovtemp=%d\n",is_extchg_ovtemp);
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:is_surf_ovtemp=%d\n",is_surf_extchg_ovtemp);
#endif
#if (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:is_usb_ovtemp=%d\n",is_usb_extchg_ovtemp);
#endif
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:extchg_status=%d\n",chg_stm_state_info.extchg_status);
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:extchg_online_st=%d\n",chg_stm_state_info.extchg_online_st);
    chg_print_level_message(CHG_MSG_INFO, "EXTCHG:exchg_enable=%d\n",g_exchg_enable_flag);
}


static void chg_set_charge_otg_mode(void)
{
    /*�����оƬ����Ϊ ��OTGģʽ��5V �µ�*/
    chg_set_charge_otg_enable(FALSE);
    /*��ʱ500MS*/
    chg_delay_ms(EXTCHG_DELAY_COUNTER_SIZE);
    /*Ϊ�˱�����оƬ�µ�ǰ��VBUS 5V�µ�ǰ�ٴη���OCP�����µ�����¶�ȡ�Ĵ���REG09ȷ����оƬ�����ϵ�ǰBAT_FAULTλ����*/
    (void)chg_extchg_ocp_detect();
    /*TO DO:����USB�����ӿ�ֱ������������HS_ID*/
    usb_notify_event(USB_OTG_DISCONNECT_DP_DM,NULL);
    /*��ʱ500MS*/
    chg_delay_ms(EXTCHG_DELAY_COUNTER_SIZE);
    /*�����оƬ����ΪOTGģʽ5V�ϵ�*/
    chg_set_charge_otg_enable(TRUE);
    /*���˶������л�ΪOTG 500MA���ģʽ*/
    is_otg_extchg = TRUE;
}


void chg_extchg_insert_proc(void)
{
    CHG_MODE_ENUM cur_chg_mode = chg_get_cur_chg_mode();

    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:CHG_EXGCHG_CHGR PLUG IN!\n ");
    if(OFFLINE == g_exchg_online_flag)
    {
        g_exchg_online_flag = ONLINE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:disable extchg,do nothing!\n");
        return;
    }
    /*���UI�������ý�ֹ������˴�����USB ID �����ж�*/
    if(1 == g_extchg_diable_st)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:disable extchg,do nothing!\n");
        return;
    }
    /*���߳�糡��������������¼�*/
    if(CHG_WIRELESS_MODE == cur_chg_mode)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:chg_mode in CHG_WIRELESS_MODE,do nothing!\n ");
        return;
    }

    chg_stm_set_extchg_online_st(TRUE);
    chg_set_cur_chg_mode(CHG_EXTCHG_MODE);
    //TO DO:1:����USB�����ӿڶ̽�D+ ,D-
    usb_notify_event(USB_OTG_CONNECT_DP_DM,NULL);
    chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG, (uint32_t)CHG_EVENT_NONEED_CARE);
#if ( FEATURE_ON == MBB_MLOG )
    printk(KERN_ERR "CHG_STM:otg_charge_count\n");
    mlog_set_statis_info("otg_charge_count",1);//�������ܴ��� ��1
#endif/*MBB_MLOG*/
    /*������������work*/
    if(NULL != g_chip)
    {
        schedule_delayed_work(&g_chip->extchg_monitor_work, msecs_to_jiffies(0));
    }
}


void chg_extchg_remove_proc(void)
{
    if(ONLINE == g_exchg_online_flag)
    {
        g_exchg_online_flag = OFFLINE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:disable extchg,do nothing!\n");
        return;
    }
    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:CHG_EXGCHG_CHGR PLUG OUT!\n ");
    /*��ֹ���е�work*/
    cancel_delayed_work_sync(&g_chip->extchg_monitor_work);

    /*������USB ID�߰γ����¶Ȳ����ı�־λ���*/
    g_ui_choose_exchg_mode = 0;
    g_last_extchg_diable_st = 0;
    is_otg_extchg = FALSE;
    is_extchg_ovtemp = FALSE;
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
    is_surf_extchg_ovtemp = FALSE;
#endif
#if (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
    is_usb_extchg_ovtemp = FALSE;
#endif
    chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
    chg_set_extchg_chg_enable(FALSE);
    chg_stm_set_extchg_online_st(FALSE);
    chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);

}


boolean chg_is_extchg_overtemp(void)
{
    boolean is_extchg_otp = FALSE;
    /*��Ϊ��UIδ�·�����������ǰ��������״̬��
      ��������Ҫʵʱ��ȡһ���¶ȣ���һ�μ�顣*/
    int32_t bat_temp = chg_get_sys_batt_temp();
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
    int32_t surf_temp = g_surf_temp_state.sys_surf_temp;
#endif

    /*��ع���ֹͣ������*/
    if(bat_temp >= EXTCHG_OVER_TEMP_STOP_THRESHOLD)
    {
        is_extchg_ovtemp = TRUE;
        is_extchg_otp    = TRUE;
    }
    /*��ǹ���ֹͣ������*/
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
    if(surf_temp >= DCM_RVSCHG_STOP_TSURF_TH)
    {
        is_surf_extchg_ovtemp = TRUE;
        is_extchg_otp         = TRUE;
    }
#endif
    /*USB����ֹͣ������*/
#if (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
    if (TRUE == chg_get_usb_temp_protect_stat())
    {
        is_usb_extchg_ovtemp = TRUE;
        is_extchg_otp        = TRUE;
    }
#endif

    return is_extchg_otp;
}


void chg_extchg_monitor_func(void)
{
    int32_t bat_temp = chg_get_sys_batt_temp();
    int32_t bat_vol = chg_get_sys_batt_volt();
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    int32_t batt_soc = chg_get_sys_batt_capacity();
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
    int32_t surf_temp = g_surf_temp_state.sys_surf_temp;
#endif

    if(OFFLINE == g_exchg_online_flag)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:extchg device not exit,do nothing!\n");
        return;
    }

/*���δͨ���ϲ�UI���������磬�򲻽���״̬���*/
#if defined(BSP_CONFIG_BOARD_E5_DC04)
    if (0 == g_ui_choose_exchg_mode)
    {
        goto checking_over;
    }
#endif

    /*����UI����*/
    chg_extchg_config_data_init();
    /*״̬���*/
    /*��������������Ϊ��OTGģʽ����ͨ��5V���µ�reset usbģ��*/
    if(TRUE == chg_extchg_ocp_detect())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:EXTCHG OCP happened is_otg_extchg=%d!\n",
            is_otg_extchg);

        if(FALSE == is_otg_extchg)
        {
            chg_set_charge_otg_mode();
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:1A EXTCHG OCP change to USB OTG 500MA!\n");
        }
        /*���ΪOTGģʽ��⵽����ֱ��ֹͣ������*/
        else
        {
            chg_set_extchg_chg_enable(FALSE);
            chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_STOP_FAULT;
            /*֪ͨӦ�ò��������������ֹͣ������*/
            chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:USB OTG 500MA EXTCHG OCP STOP EXTGHG!\n");
        }
    }
#if defined(BSP_CONFIG_BOARD_E5_DC04)
    /*����¶ȹ�����Ϊ��OTGģʽ�����ƶ��������500MA*/
    else if( (bat_temp >= EXTCHG_OVER_TEMP_LIMIT_500MA) && (FALSE == is_otg_extchg)
        /*����¶ȳ�������ͣ�����ޣ�������500MA���*/
        && (FALSE == is_extchg_ovtemp)
        /*��ǹ��²�����500MA���*/
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
        && (FALSE == is_surf_extchg_ovtemp)
#endif
        /*USB����ֹͣ������*/
#if(FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
        && (FALSE == is_usb_extchg_ovtemp)
#endif
        )
    {
        chg_set_charge_otg_mode();
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:1A EXTCHG battery over temp change to USB OTG 500MA!\n");
    }
#endif
    /*��ع���ֹͣ������*/
    else if((bat_temp >= EXTCHG_OVER_TEMP_STOP_THRESHOLD && FALSE == is_extchg_ovtemp)
    /*��ǹ���ֹͣ������*/
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
        || (surf_temp >= DCM_RVSCHG_STOP_TSURF_TH && FALSE == is_surf_extchg_ovtemp)
#endif
    /*USB����ֹͣ������*/
#if (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
        || (TRUE == chg_get_usb_temp_protect_stat() && FALSE == is_usb_extchg_ovtemp)
#endif
        )
    {
        /*����ֹͣ������*/
        chg_set_extchg_chg_enable(FALSE);
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL) || (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
        if (bat_temp >= EXTCHG_OVER_TEMP_STOP_THRESHOLD && FALSE == is_extchg_ovtemp)
        {
            chg_print_level_message(CHG_MSG_INFO, "EXTCHG:BATTERY OVER TEMP STOP EXTGHG!\n");
            is_extchg_ovtemp = TRUE;
        }
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)        
        if (surf_temp >= DCM_RVSCHG_STOP_TSURF_TH && FALSE == is_surf_extchg_ovtemp)
        {
            chg_print_level_message(CHG_MSG_INFO, "EXTCHG:SURFACE OVER TEMP STOP EXTGHG!\n");
            is_surf_extchg_ovtemp = TRUE;
        }
#endif
#if (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
        if (TRUE == chg_get_usb_temp_protect_stat() && FALSE == is_usb_extchg_ovtemp)
        {
            chg_print_level_message(CHG_MSG_INFO, "EXTCHG:USB OVER TEMP STOP EXTGHG!\n");
            is_usb_extchg_ovtemp = TRUE;
        }
#endif
#else /*!((FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL) || (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT))*/
        is_extchg_ovtemp = TRUE;
#endif
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_OVERHEAT_STOP_CHARGING;
        /*֪ͨӦ���¶��쳣ֹͣ������*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);

        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:EXTCHG OVER TEMP STOP EXTGHG!\n");
    }
    /*����¶Ȼָ�ʹ�ܶ�����*/
    else if((bat_temp <= EXTCHG_OVER_TEMP_RESUME_THRESHOLD) && (TRUE == is_extchg_ovtemp)
    /*����¶Ȼָ�ʹ�ܶ�����*/
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
        || (surf_temp < DCM_RVSCHG_BACKTO_TSURF_TH && TRUE == is_surf_extchg_ovtemp)
#endif
    /*USB�¶Ȼָ�ʹ�ܶ�����*/
#if (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
        || (FALSE == chg_get_usb_temp_protect_stat() && TRUE == is_usb_extchg_ovtemp)
#endif
        )
    {
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL) || (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
        if ((bat_temp <= EXTCHG_OVER_TEMP_RESUME_THRESHOLD) && (TRUE == is_extchg_ovtemp))
        {
            chg_print_level_message(CHG_MSG_INFO, "EXTCHG:BATTERY TEMP NORMAL RESUME EXTGHG!\n");
            is_extchg_ovtemp = FALSE;
        }
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
        if (surf_temp < DCM_RVSCHG_BACKTO_TSURF_TH && TRUE == is_surf_extchg_ovtemp)
        {
            chg_print_level_message(CHG_MSG_INFO, "EXTCHG:SURFACE TEMP NORMAL RESUME EXTGHG!\n");
            is_surf_extchg_ovtemp = FALSE;
        }
#endif
#if (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
        if (FALSE == chg_get_usb_temp_protect_stat() && TRUE == is_usb_extchg_ovtemp)
        {
            chg_print_level_message(CHG_MSG_INFO, "EXTCHG:USB TEMP NORMAL RESUME EXTGHG!\n");
            is_usb_extchg_ovtemp = FALSE;
        }
#endif
#else /*!((FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL) || (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT))*/
        is_extchg_ovtemp = FALSE;
#endif

        /*�����е��¶ȶ��ָ��󣬲�ʹ�ܶ�����*/
        if (FALSE == is_extchg_ovtemp
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
            && FALSE == is_surf_extchg_ovtemp
#endif
#if (FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT)
            && FALSE == is_usb_extchg_ovtemp
#endif
            )
        {
#if defined(BSP_CONFIG_BOARD_E5_DC04)
            /*������ͣ���ָ���500MA���*/
            chg_set_charge_otg_mode();
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:EXTCHG TEMP NORMAL RESUME EXTGHG TO USB OTG 500MA!\n");
#endif
            chg_set_extchg_chg_enable(TRUE);
            chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_START_CHARGING;
            /*�¶Ȼָ�����֪ͨӦ�ÿ�ʼ������*/
            chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:EXTCHG TEMP NORMAL RESUME EXTGHG!\n");
        }
    }
   /*��ص͵�ֹͣ������*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
#if defined(BSP_CONFIG_BOARD_E5_DC04)
    else if(batt_soc < g_extchg_stop_soc_threshold)
#else
    else if(batt_soc <= g_extchg_stop_soc_threshold)
#endif
#else
    else if(bat_vol <= g_extchg_voltage_threshold)
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/
    {
        chg_set_extchg_chg_enable(FALSE);
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_LOWPOWER_STOP_CHARGING;
        /*֪ͨӦ�õ͵�ֹͣ������*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:EXTCHG LOW VOLTAGE STOP EXTGHG!\n");
    }
/*DC04 ��֧��Touch UI�˵�����/���ö����繦�ܣ������繦�����ÿ�����*/
#if !defined(BSP_CONFIG_BOARD_E5_DC04)
    /*���ڶ���������������⵽�û�ͨ��TOUCH UI��������ֹͣ��������ֱ��ֹͣ������*/
    else if((1 == g_extchg_diable_st) && (0 == g_last_extchg_diable_st))
    {
        g_last_extchg_diable_st = 1;
        chg_set_extchg_chg_enable(FALSE);
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_STOP_FAULT;
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:UI set disable extchg permanently!\n");
    }
    /*���ڶ���������������⵽�û�ͨ��TOUCH UI��������ֹͣ��������ֱ��ֹͣ�������
       ����û������ô�*/
    else if((0 == g_extchg_diable_st) && (1 == g_last_extchg_diable_st))
    {
        g_last_extchg_diable_st = 0;
        /*����ʹ�ܺ�STA�ڵ�״̬��ΪUNKNOWN,��ѡ��򵯳������û�ѡ�������Ƿ������*/
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:UI set enable extchg permanently!\n");
    }
#endif
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:staying at extchg_state !\n");
    }

#if defined(BSP_CONFIG_BOARD_E5_DC04)
checking_over:
#endif
    chg_extchg_info_dump();
    schedule_delayed_work(&g_chip->extchg_monitor_work, msecs_to_jiffies(2000));
}

#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/


void chg_charger_insert_proc(chg_chgr_type_t chg_type)
{
    chg_stm_state_type cur_stat = chg_stm_get_cur_state();

    /* �����Դ����,��������͵��־ */
    chg_batt_low_battery_flag = FALSE;

    /*USB����*/
    if(CHG_USB_HOST_PC == chg_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_USB_HOST_PC PLUG IN force to TRANSIT_ST!\n ");
        if(FALSE == chg_get_usb_online_status())
        {
            chg_set_ac_online_status(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
            chg_set_usb_online_status(TRUE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
        //chg_stm_set_chgr_type(CHG_USB_HOST_PC);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);
        if((CHG_STM_INIT_ST >= cur_stat) || (CHG_STM_MAX_ST <= cur_stat)
            || (CHG_STM_BATTERY_ONLY == cur_stat) || (CHG_STM_TRANSIT_ST == cur_stat))
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }

#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE == chg_stm_get_wireless_online_st())
        {
            /*USB��������߳������λ��ONLINE�ڵ���ΪOFFLINE����ֹ���߳��
                �����в��������豸��ONLINE�ڵ�û�������ʾ���߳��ͼ��*/
            chg_stm_set_wireless_online_st(FALSE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#endif /*defined(MBB_CHG_WIRELESS)*/
#if ( FEATURE_ON == MBB_MLOG )
        /*usb���룬����һ��*/
        if(CHG_CURRENT_SS == usb_speed_work_mode())
        {
            /*usb3.0����*/
            mlog_set_statis_info("usb3_charge_insert_count",1);
        }
        else
        {
            /*usb2.0����*/
            mlog_set_statis_info("usb2_charge_insert_count",1);
        }
#endif/*FEATURE_ON == MBB_MLOG*/
        return ;
    }
    /*���/CRADLE/HVDCP����*/
    else if((CHG_WALL_CHGR == chg_type)
        || (CHG_USB_OTG_CRADLE == chg_type)
        || (CHG_HVDCP_CHGR == chg_type))
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_WALL_CHGR PLUG IN force to TRANSIT_ST!\n ");
        //if(CHG_USB_OTG_CRADLE == chg_type)
        //{
        //    chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:AF18 PLUG IN set Dpm.\n ");
        //    chg_set_dpm_val(CHG_AF18_DPM_VOLT);
        //}
        /*�����⵽����������Ǳ��ͽ� USB ����ΪOFFINE,��AC����ΪONLINE*/
        chg_set_usb_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);

        chg_set_ac_online_status(TRUE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        //chg_stm_set_chgr_type(CHG_WALL_CHGR);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);

        if((CHG_STM_INIT_ST >= cur_stat) || (CHG_STM_MAX_ST <= cur_stat)
            || (CHG_STM_BATTERY_ONLY == cur_stat) || (CHG_STM_TRANSIT_ST == cur_stat))
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE == chg_stm_get_wireless_online_st())
        {
            /*����������߳������λ��ONLINE�ڵ���ΪOFFLINE����ֹ���߳��
                 �����в��������豸��ONLINE�ڵ�û�������ʾ���߳��ͼ��*/
            chg_stm_set_wireless_online_st(FALSE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#endif /*MBB_CHG_WIRELESS == FEATURE_ON*/
#if ( FEATURE_ON == MBB_MLOG )
            chg_print_level_message(CHG_MSG_ERR, "std charger insert!\n ");
            mlog_set_statis_info("std_charge_insert_count",1);
#endif
        return ;
    }
    /*�������*/
    else if(CHG_500MA_WALL_CHGR == chg_type)
    {
        chg_en_flag = 0;
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_500MA_WALL_CHGR PLUG IN force to TRANSIT_ST!\n ");
        /*�����⵽���������������ͽ� USB ����ΪOFFINE,��AC����ΪONLINE*/
        chg_set_usb_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);

        chg_set_ac_online_status(TRUE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        //chg_stm_set_chgr_type(CHG_500MA_WALL_CHGR);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);
        if((CHG_STM_INIT_ST >= cur_stat) || (CHG_STM_MAX_ST <= cur_stat)
            || (CHG_STM_BATTERY_ONLY == cur_stat) || (CHG_STM_TRANSIT_ST == cur_stat))
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE == chg_stm_get_wireless_online_st())
        {
            /*�����������߳������λ��ONLINE�ڵ���ΪOFFLINE����ֹ���߳��
                 �����в��������豸��ONLINE�ڵ�û�������ʾ���߳��ͼ��*/
            chg_stm_set_wireless_online_st(FALSE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#endif /*MBB_CHG_WIRELESS == FEATURE_ON*/
        return ;
    }
    /*�Ǳ�����*/
    else if((CHG_NONSTD_CHGR == chg_type) || (CHG_HVDCP_CHGR == chg_type))
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_NONSTD_CHGR PLUG IN force to TRANSIT_ST!\n ");
        //�Ǳ�����ʱ����ΪUSB��λ
        chg_set_ac_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);

        chg_set_usb_online_status(TRUE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        //chg_stm_set_chgr_type(CHG_NONSTD_CHGR);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);

        if((CHG_STM_INIT_ST >= cur_stat) || (CHG_STM_MAX_ST <= cur_stat)
            || (CHG_STM_BATTERY_ONLY == cur_stat) || (CHG_STM_TRANSIT_ST == cur_stat))
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE == chg_stm_get_wireless_online_st())
        {
            /*�����������������߳������λ��ONLINE�ڵ���ΪOFFLINE����ֹ���߳��
                �����в��������豸��ONLINE�ڵ�û�������ʾ���߳��ͼ��*/
            chg_stm_set_wireless_online_st(FALSE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);

        }
#endif /*defined(MBB_CHG_WIRELESS)*/
#if ( FEATURE_ON == MBB_MLOG )
            chg_print_level_message(CHG_MSG_ERR, "no_std charger insert!\n ");
            mlog_set_statis_info("no_std_charge_insert_count",1);
#endif
        return ;
    }

    /*���������δ֪���ڴ��ж������߳�绹�����߳���豸����*/
    else if(CHG_CHGR_UNKNOWN == chg_type)
    {
        /*��������߳��������������AC ONLINE��֤Ӧ�ÿ���Ѹ�ٵ���
              �����⵽�����������usb�������������ͽ�AC ����ΪOFFINE,��USB����ΪONLINE*/
        chg_set_ac_online_status(TRUE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        /*���߳������������߳������λ��ONLINE�ڵ���ΪOFFLINE����ֹ���߳��
        �����в��������豸��ONLINE�ڵ�û�������ʾ���߳��ͼ��*/
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        chg_stm_set_wireless_online_st(FALSE);
#endif /*MBB_CHG_WIRELESS == FEATURE_ON*/
        //chg_stm_set_chgr_type(CHG_CHGR_UNKNOWN);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);
        chg_print_level_message( CHG_MSG_INFO,"CHG_PLT: wired charger but chg type unknow force to TRANSIT_ST!\n");
        chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        return ;
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if(CHG_WIRELESS_CHGR == chg_type)
    {
        /*֪ͨUSB�������߳��������,��ֹUSB����δ������������͸����ģ��
        �ϱ�����������������³�����̻���*/
        g_wireless_online_flag = ONLINE;
        usb_notify_event(USB_WIRELESS_CHGR_DET,NULL);
        if(0 == g_ui_choose_exchg_mode)
        {
            (void)chg_set_dpm_val(CHG_WIRELESS_DPM_VOLT);
            chg_stm_set_wireless_online_st(TRUE);
            chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_WIRELESS_CHGR PLUG IN force to TRANSIT_ST!\n ");
            /*�����������߳������Ϳ������߳��оƬGPIOʹ�����߳�磬�������е�ͣ������BQ24196����*/
            chg_set_wireless_chg_enable(TRUE);
            //chg_stm_set_chgr_type(CHG_WIRELESS_CHGR);
            chg_set_cur_chg_mode(CHG_WIRELESS_MODE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
            return ;
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:WIRELESS_CHGR PLUG IN but current not CHG_BAT_ONLY_MODE!\n");
            return ;
        }
#if ( FEATURE_ON == MBB_MLOG )        
            chg_print_level_message(CHG_MSG_ERR, "wireless charger insert!\n ");
            mlog_set_statis_info("wireless_charge_insert_count",1);
#endif
    }
#endif/*MBB_CHG_WIRELESS*/
    else
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:PLUG IN CHG TYPE UNKNOW!\n ");
    }
}


void chg_charger_remove_proc(chg_chgr_type_t chg_type)
{
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    CHG_MODE_ENUM cur_chg_mode = chg_get_cur_chg_mode();
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
    chg_stm_state_type cur_stat = chg_stm_get_cur_state();
/*������ͷ�ֹ���ٲ�Σ�״̬��ˢ��*/
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    if (TRUE == chg_battery_protect_flag)
    {
        chg_battery_protect_flag = FALSE;
        g_chg_over_temp_volt_protect_flag = FALSE;
        g_chg_longtime_nocharge_protect_flag = FALSE;
        (void)chg_battery_protect_exit_process();
    }
    (void)resume_long_time_no_charge_protect_recharge_volt();
#endif

    /*USB �γ�*/
    if(CHG_USB_HOST_PC == chg_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_USB_HOST_PC PLUG OUT force to BATTERY_ONLY_st!\n ");
        chg_set_usb_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);
        return ;
    }
    /*���/cradle/HVDCP�γ�*/
    else if((CHG_WALL_CHGR == chg_type)
        || (CHG_USB_OTG_CRADLE == chg_type)
        || (CHG_HVDCP_CHGR == chg_type))
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_WALL_CHGR PLUG OUT force to BATTERY_ONLY_st!\n ");
        //AF18 �γ���ָ�Ĭ��DPM��ѹֵ
        if(CHG_USB_OTG_CRADLE == chg_type)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:AF18 PLUG out set Dpm.\n ");
            chg_set_dpm_val(CHG_DEFAULT_DPM_VOLT);
        }
        chg_set_ac_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);
        return ;
    }
    /*����γ�*/
    else if(CHG_500MA_WALL_CHGR == chg_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_500MA_WALL_CHGR PLUG OUT force to BATTERY_ONLY_st!\n ");
        chg_set_ac_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);
        return ;
    }
    /*�Ǳ��/HVDCP*/
    else if((CHG_NONSTD_CHGR == chg_type) || (CHG_HVDCP_CHGR == chg_type))
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_NONSTD_CHGR PLUG OUT force to BATTERY_ONLY_st!\n ");
        /*�Ǳ��γ�����USB�����γ���Ӧ���ϱ�*/
        chg_set_usb_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);

        return ;
    }
    else if(CHG_CHGR_UNKNOWN == chg_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_CHGR_UNKNOWN PLUG OUT force to BATTERY_ONLY_st!\n ");
        if (TRUE == chg_get_usb_online_status())
        {
            chg_set_usb_online_status(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
        if(TRUE == chg_get_ac_online_status())
        {
            chg_set_ac_online_status(TRUE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if (TRUE == chg_stm_get_wireless_online_st())
        {
            chg_stm_set_wireless_online_st(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
#if (MBB_CHG_EXTCHG == FEATURE_ON)
        if (TRUE == chg_stm_get_extchg_online_st())
        {
            chg_stm_set_extchg_online_st(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/

        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        return ;
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    /*���߳�����γ�*/
    else if(CHG_WIRELESS_CHGR == chg_type)
    {
        /*�����ǰ�����߳��ģʽ�����߳��������λ��Ž���״̬�л�����ֻ�����޳����
            ONLINE�ڵ�����ΪOFFLINE,������״̬�л�*/
        g_wireless_online_flag = OFFLINE;
        if((CHG_WIRELESS_MODE == cur_chg_mode) || (TRUE == chg_is_powdown_charging()))
        {
            chg_stm_set_wireless_online_st(FALSE);
            chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:CHG_WIRELESS_CHGR PLUG OUT force to BATTERY_ONLY_st!\n ");
            chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
            chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
            chg_stm_set_chgr_type(CHG_CHGR_INVALID);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
            return ;
        }

    }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
    else
    {
        if( (TRUE == chg_get_usb_temp_protect_stat() && CHG_STM_BATTERY_ONLY != cur_stat)
            || (FALSE == chg_is_charger_present() && CHG_STM_BATTERY_ONLY != cur_stat) )
        {
            chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        }
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:PLUG OUT CHG TYPE UNKNOW!\n ");
    }
}


struct chg_batt_data *chg_get_batt_data(unsigned int id_voltage)
{
    int i;

    /*���ID��ѹΪ0˵��Ϊ��ͨ�����ʹ����ͨ��صĲ���*/
    if(0 == id_voltage)
    {
        return chg_batt_data_array[0];
    }
    /*��ѹ��������ж�*/
    for (i = (BATTERY_DATA_ARRY_SIZE - 1); i > 0; i--){
        if ((id_voltage >= chg_batt_data_array[i]->id_voltage_min)
            && (id_voltage <= chg_batt_data_array[i]->id_voltage_max))
        {
            break;
        }
    }
    chg_print_level_message(CHG_MSG_ERR,"bat_id_index=%d\n",i);
    return chg_batt_data_array[i];
}


int32_t chg_batt_volt_paras_init(void)
{
    int32_t batt_id = 0;
    batt_id = chg_get_batt_id_volt();
    g_chg_batt_data = chg_get_batt_data(batt_id);
    if(NULL == g_chg_batt_data)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_INIT:get batt data fail use default NV parameter!!\n");
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
        g_batt_normal_temp_recherge_threshold = g_chgBattVoltProtect.battNormalTempRechergeThreshold;
#endif
        return -1;
    }

    if (CHG_BATT_ID_DEF == g_chg_batt_data->batt_id)
    {
        mlog_print(MLOG_CHG, mlog_lv_info, 
            "Unknown battery type detected,use default charge parameters.\n");
        chg_print_level_message(CHG_MSG_INFO, "Unknown battery type detected.\n");
    }

    //init batt para
    memcpy(&g_chgBattVoltProtect, (uint8_t*)&g_chg_batt_data->chg_batt_volt_paras, sizeof(CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE));
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    g_batt_normal_temp_recherge_threshold = g_chgBattVoltProtect.battNormalTempRechergeThreshold;
#endif
    chg_batt_data_dump();
    return 0;
}


uint32_t chg_get_batt_id(void)
{
    if(NULL == g_chg_batt_data)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_INIT:get batt data fail!!\n");
        return CHG_BATT_ID_DEF;
    }
    chg_print_level_message(CHG_MSG_DEBUG,"CHG: batt brand is %d\n",g_chg_batt_data->batt_id);

    return g_chg_batt_data->batt_id;
}


void chg_batt_data_dump(void)
{
    chg_print_level_message(CHG_MSG_ERR,"battVoltPowerOnThreshold=%d\n",
        g_chgBattVoltProtect.battVoltPowerOnThreshold);
    chg_print_level_message(CHG_MSG_ERR,"battVoltPowerOffThreshold=%d\n",
        g_chgBattVoltProtect.battVoltPowerOffThreshold);
    chg_print_level_message(CHG_MSG_ERR,"battOverVoltProtectThreshold=%d\n",
        g_chgBattVoltProtect.battOverVoltProtectThreshold);
    chg_print_level_message(CHG_MSG_ERR,"battOverVoltProtectOneThreshold=%d\n",
        g_chgBattVoltProtect.battOverVoltProtectOneThreshold);
    chg_print_level_message(CHG_MSG_ERR,"battChgTempMaintThreshold=%d\n",
        g_chgBattVoltProtect.battChgTempMaintThreshold);
    chg_print_level_message(CHG_MSG_ERR,"battChgRechargeThreshold=%d\n",
        g_chgBattVoltProtect.battChgRechargeThreshold);
    chg_print_level_message(CHG_MSG_ERR,"VbatLevelLow_MAX=%d\n",g_chgBattVoltProtect.VbatLevelLow_MAX);
    chg_print_level_message(CHG_MSG_ERR,"VbatLevel0_MAX=%d\n",g_chgBattVoltProtect.VbatLevel0_MAX);
    chg_print_level_message(CHG_MSG_ERR,"VbatLevel1_MAX=%d\n",g_chgBattVoltProtect.VbatLevel1_MAX);
    chg_print_level_message(CHG_MSG_ERR,"VbatLevel2_MAX=%d\n",g_chgBattVoltProtect.VbatLevel2_MAX);
    chg_print_level_message(CHG_MSG_ERR,"VbatLevel3_MAX=%d\n",g_chgBattVoltProtect.VbatLevel3_MAX);
    chg_print_level_message(CHG_MSG_ERR,"battInsertChargeThreshold=%d\n",
    g_chgBattVoltProtect.battInsertChargeThreshold);
    chg_print_level_message(CHG_MSG_ERR,"battNormalTempRechergeThreshold=%d\n",
    g_chgBattVoltProtect.battNormalTempRechergeThreshold);
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    chg_print_level_message(CHG_MSG_ERR,"g_batt_normal_temp_recherge_threshold=%d\n",
        g_batt_normal_temp_recherge_threshold);
    chg_print_level_message(CHG_MSG_ERR,"g_chg_batt_expand_change_normal_rechg_flag=%d\n",
        g_chg_batt_expand_change_normal_rechg_flag);
#endif
}

