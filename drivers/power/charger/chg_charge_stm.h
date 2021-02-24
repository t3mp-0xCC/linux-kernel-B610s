
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

#ifndef CHG_CHARGE_STM_H
#define CHG_CHARGE_STM_H

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define CHG_CHGR_TYPE_CHECK_MAX_RETRY_TIMES       (12)

#define CHG_CHGR_TYPE_CHECK_INTERVAL_IN_MS        (2000)

/*Fast charge protection timer, in second. */
#define MS_IN_SECOND                            (1000)
#define SECOND_IN_HOUR                          (60 * 60)
#if defined(BSP_CONFIG_BOARD_E5_DC04)
#define CHG_FAST_CHG_TIMER_VALUE                (4 * 2 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_CHG_TIMER_VALUE       (4 * 2 * SECOND_IN_HOUR)

#define CHG_FAST_USB_TIMER_VALUE                (12 * 2 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_USB_TIMER_VALUE       (12 * 2 * SECOND_IN_HOUR)
#else
#define CHG_FAST_CHG_TIMER_VALUE                (6 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_CHG_TIMER_VALUE       (6 * SECOND_IN_HOUR)

#define CHG_FAST_USB_TIMER_VALUE                (12 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_USB_TIMER_VALUE       (12 * SECOND_IN_HOUR)
#endif

/*Time Interval for toggle CEN.*/
#define CHG_TOGGLE_CEN_INTVAL_IN_MS             (100)

/*Time Interval for switch to SLOW POLLING while battery only.*/
#define CHG_SWITCH_TO_SLOW_POLL_INTERVAL_IN_SEC (60)

/*Indicate battery charging start/stop flag.*/
#define CHG_UI_START_CHARGING                   (1)
#define CHG_UI_STOP_CHARGING                    (0)
/* 延时500ms */
#define EXTCHG_DELAY_COUNTER_SIZE               (500)

/* 若定义补电宏 */
#if (MBB_CHG_COMPENSATE == FEATURE_ON)
/* 延时10ms */
#define CHG_DELAY_COUNTER_SIZE                  (10)
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
/*电池补电阈值40%电量*/
#define TBAT_SUPPLY_VOLT                        (3780)
/*电池放电阈值80%电量*/
#define TBAT_DISCHG_VOLT                        (4085)
 /*电池充电截止电压 */
#define TBAT_SUPPLY_STOP_VOLT                   (3825)
 /*电池放电截止电压 */
#define TBAT_DISCHG_STOP_VOLT                   (4050)
#else
/*电池补电阈值40%电量*/
#define TBAT_SUPPLY_VOLT                        (3696)
/*电池放电阈值80%电量*/
#define TBAT_DISCHG_VOLT                        (3970)
/*电池充电截止电压 */
#define TBAT_SUPPLY_STOP_VOLT                   (3750)
/*电池放电截止电压 */
#define TBAT_DISCHG_STOP_VOLT                   (3865)
#endif/*HUAWEI_CHG_HIGH_VOLT_BATT*/
/*补电成功*/
#define TBAT_SUPPLY_CURR_SUCCESS                (0x0)
/*不需要补电*/
#define TBAT_NO_NEED_SUPPLY_CURR                (0x1)
/* 补电停止时间 */
#define TBAT_STOP_DELAY_COUNTER                 (100)
/* 补电启动时间 */
#define TBAT_SUPLY_DELAY_COUNTER                (2300)
#endif /*MBB_CHG_COMPENSATE == FEATURE_ON */

/*关机充电关机检测次数**/
#define CHARGE_REMOVE_CHECK_MAX                 (1)

/*非标配电池充电电流设置: 同时会设置20pct标记*/
#define CHG_DEF_BATT_CUR_LEVEL                  (512)
/*非标配电池充电电压设置*/
#define CHG_DEF_BATT_VREG_LEVEL                 (4000)

#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
/*电池膨胀保护方案参数*/
/*参数说明*/

/*电池保护温度门限*/
#define CHG_BATTERY_PROTECT_TEMP            (45)
/*退出电池保护温度门限*/
#define CHG_BATTERY_PROTECT_RESUME_TEMP     (42)
/*温度超过电池保护门限时，电池电池电压限制*/
#define CHG_BATTERY_PROTECT_VOLTAGE         (4150)
/*退出电池保护电压门限*/
#define CHG_BATTERY_PROTECT_RESUME_VOLTAGE  (4100)
/*电池保护截止电压门限*/
#define CHG_BATTERY_PROTECT_CV_VOLTAGE      (4100)
/*满电停充且充电器在位进入suspend/前段限流100MA的时间门限*/
#define CHG_BATTERY_PROTECT_CHGER_TIME_THRESHOLD_IN_SECONDS (16 * 60 * 60)
/*满电停充且充电器长时间在位状态常温复充门限*/
#define BATT_NORMAL_TEMP_RECHARGE_THR_LONG_TIME_NO_CHARGE    (4050)
#endif

/*----------------------------------------------*
 * 结构定义                                      *
 *----------------------------------------------*/
typedef struct
{
    uint32_t pwr_supply_current_limit_in_mA;
    uint32_t chg_current_limit_in_mA;
    uint32_t chg_CV_volt_setting_in_mV;
    uint32_t chg_taper_current_in_mA;
    boolean  chg_is_enabled;
}chg_hw_param_t;

typedef  void (*chg_stm_func_type )(void);
typedef struct
{
  chg_stm_func_type        chg_stm_entry_func;
  chg_stm_func_type        chg_stm_period_func;
  chg_stm_func_type        chg_stm_exit_func;
}chg_stm_type;

/*高温关机温度参数NV50016结构定义*/
typedef struct
{
    uint32_t      ulIsEnable;             //高温关机使能开关
    int32_t       lCloseAdcThreshold;     //高温关机温度门限
    uint32_t      ulTempOverCount;        //高温关机温度检测次数
}CHG_BATTERY_OVER_TEMP_PROTECT_NV;

/*高温关机温度参数NV52005结构定义*/
typedef struct
{
    uint32_t      ulIsEnable;
    int32_t       lCloseAdcThreshold;
    uint32_t      ulTempLowCount;
}CHG_BATTERY_LOW_TEMP_PROTECT_NV;


/*充电过程中温度参数NV 50385结构定义*/
typedef struct
{
    uint32_t    ulChargeIsEnable;                   //充电温保护使能
    int32_t     overTempchgStopThreshold;           //充电高温保护门限
    int32_t     subTempChgLimitCurrentThreshold;    //高温充电进入门限
    int32_t     lowTempChgStopThreshold;            //充电低温保护门限
    int32_t     overTempChgResumeThreshold;         //充电高温恢复温度门限
    int32_t     lowTempChgResumeThreshold;          //充电低温恢复温度门限
    uint32_t    chgTempProtectCheckTimes;           //充电停充轮询次数
    uint32_t    chgTempResumeCheckTimes;            //充电复充轮询次数
    int32_t     exitWarmChgToNormalChgThreshold;    //由高温充电恢复到常温充电温度门限
    int32_t     reserved2;                          //预留
}CHG_SHUTOFF_TEMP_PROTECT_NV_TYPE;

/*充电过程中温度参数NV50386结构定义*/
typedef struct
{
    int32_t         battVoltPowerOnThreshold;           //开机电压门限
    int32_t         battVoltPowerOffThreshold;          //关机电压门限
    int32_t         battOverVoltProtectThreshold;       //平滑充电过压保护门限(平滑值)
    int32_t         battOverVoltProtectOneThreshold;    //单次充电过压保护门限(单次值)
    int32_t         battChgTempMaintThreshold;          //超时停充判断门限
    int32_t         battChgRechargeThreshold;           //高温充电时的复充门限
    int32_t         VbatLevelLow_MAX;                   //低电上限门限
    int32_t         VbatLevel0_MAX;                     //0格电压上限门限
    int32_t         VbatLevel1_MAX;                     //1格电压上限门限
    int32_t         VbatLevel2_MAX;                     //2格电压上限门限
    int32_t         VbatLevel3_MAX;                     //3格电压上限门限
    int32_t         battInsertChargeThreshold;          //插入充电器判断是否使能充电的判断门限
    int32_t         battNormalTempRechergeThreshold;    //常温充电的复充门限
}CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE;

/* NV50364电池参数数据结构**/
typedef struct
{
    /*非工厂模式下电池不在位开机使能标志*/
    uint8_t no_battery_powerup_enable;
    /*异常关机后，下次插入电源进入开机模式使能标志*/
    uint8_t exception_poweroff_poweron_enable;
    /*低电关机禁止标志*/
    uint8_t low_battery_poweroff_disable;
    /*保留*/
    uint8_t reserved;
}POWERUP_MODE_TYPE;

/*高压电池0-10℃之间限流结构体*/
typedef struct
{
    /*充电器类型*/
    chg_chgr_type_t chgr_type;
    /*I_CHG限流参数*/
    uint32_t        current_limit;
    /*I_USB限流参数*/
    uint32_t        current_limit_usb;
}charger_current_limit_st;

/*电池ID信息索引*/
typedef enum
{
    CHG_BATT_ID_DEF                     = 0 ,
    CHG_BATT_ID_FEIMAOTUI_2300MAH       = 1 ,
    CHG_BATT_ID_XINGWANGDA_1900MAH      = 2 ,
    CHG_BATT_ID_FEIMAOTUI_1900MAH       = 3 ,
    CHG_BATT_ID_XINGWANGDA_1500MAH      = 4 ,
    CHG_BATT_ID_FEIMAOTUI_1500MAH       = 5 ,
    CHG_BATT_ID_LISHEN_1500MAH          = 6 ,
    CHG_BATT_ID_XINWANGDA_3000MAH       = 7 ,
    CHG_BATT_ID_FEIMAOTUI_4800MAH       = 8 ,
    CHG_BATT_ID_XINGWANGDA_4800MAH      = 9 ,
    CHG_BATT_ID_MAX
}CHG_BATT_ID_TYPE;

/*电池相关参数结构体*/
struct chg_batt_data {
    unsigned int        id_voltage_min;         //高压电池ID识别的最小电压根据参考电压不同电压不同
    unsigned int        id_voltage_max;         //高压电池ID识别的最大电压根据参考电压不同电压不同
    boolean             is_20pct_calib;         //是否需要充电电流20%标定标志
    CHG_BATT_ID_TYPE    batt_id;
    CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE    chg_batt_volt_paras;
};

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/


void chg_set_hardware_parameter(const chg_hw_param_t* ptr_hw_param);


void chg_poll_bat_level(void);

#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)

void chg_poll_batt_soc(void);


void chg_poll_batt_charging_state_for_coul(void);

#if defined(BSP_CONFIG_BOARD_E5_DC04)

boolean chg_is_batt_power_collapse(void);
#endif


boolean chg_is_batt_in_state_of_emergency();


void chg_low_battery_event_handler(void);
#endif/*MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON*/


void chg_poll_batt_temp(void);


void chg_batt_volt_init(void);


void chg_batt_temp_init(void);


extern chg_stm_state_type chg_stm_get_cur_state(void);


extern void chg_set_cur_chg_mode(CHG_MODE_ENUM chg_mode);


extern chg_chgr_type_t chg_stm_get_chgr_type(void);


extern CHG_MODE_ENUM chg_get_cur_chg_mode(void);


void chg_check_and_update_hw_param_per_chgr_type(void);


extern boolean chg_get_batt_id_valid(void);


extern BATT_LEVEL_ENUM chg_get_batt_level(void);


extern int32_t chg_get_sys_batt_capacity(void);


extern void chg_set_sys_batt_capacity(int32_t capacity);


extern void chg_set_batt_time_to_full(int32_t time_to_full);


extern int32_t chg_get_batt_time_to_full(void);


extern boolean chg_is_batt_full(void);


extern int32_t chg_get_bat_status(void);


extern int32_t chg_get_bat_health(void);

void chg_set_extchg_status(int32_t extchg_status);


extern int32_t chg_get_extchg_status(void);


TEMP_EVENT chg_get_batt_temp_state(void);


extern void chg_stm_switch_state(chg_stm_state_type new_state);


void chg_stm_periodic_checking_func(void);


void chg_charge_paras_init(void);


int32_t chg_stm_init(void);


boolean chg_get_charging_status(void);


extern boolean chg_is_ftm_mode(void);


extern void chg_stm_set_chgr_type(chg_chgr_type_t chgr_type);


extern void chg_start_chgr_type_checking(void);

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)

void chg_stm_set_pre_state(chg_stm_state_type pre_state);


chg_stm_state_type chg_stm_get_pre_state(void);


boolean is_batttemp_in_warm_chg_area( void );
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

#if (MBB_CHG_COMPENSATE == FEATURE_ON)

int32_t chg_tbat_status_get(void);


boolean chg_is_sply_finish(void);


int32_t chg_batt_supply_proc(void *task_data);


int32_t chg_tbat_chg_sply(void);
#endif /* MBB_CHG_COMPENSATE == FEATURE_ON */


extern boolean chg_get_charging_status(void);


extern boolean chg_is_exception_poweroff_poweron_mode(void);


extern int chg_get_cur_batt_temp(void);


extern int chg_get_sys_batt_temp(void);


extern int32_t chg_get_sys_batt_volt(void);


extern int32_t chg_get_avg_vph_pwr_volt(void);


void load_on_off_mode_parameter(void);


void load_ftm_mode_init(void);


void chg_detect_batt_chg_for_shutoff(void);


extern void chg_update_power_suply_info(void);


void chg_print_test_view_info(void);


void chg_charger_insert_proc(chg_chgr_type_t chg_type);


void chg_charger_remove_proc(chg_chgr_type_t chg_type);


int32_t chg_get_batt_id_volt(void);

#ifdef CONFIG_MBB_FAST_ON_OFF

extern void chg_get_system_suspend_status(ulong64_t suspend_status);
#endif/*CONFIG_MBB_FAST_ON_OFF*/

#if (MBB_CHG_WIRELESS == FEATURE_ON)

extern void chg_stm_set_wireless_online_st(boolean online);


extern boolean chg_stm_get_wireless_online_st(void);
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)

extern void chg_stm_set_extchg_online_st(boolean online);


boolean chg_stm_get_extchg_online_st(void);

#if defined(BSP_CONFIG_BOARD_E5_DC04)

boolean chg_stm_is_extchg_enabled(void);


boolean chg_is_extchg_in_otg_mode(void);
#endif


void chg_extchg_config_data_init(void);


extern boolean chg_get_extchg_online_status(void);


extern void chg_extchg_insert_proc(void);


extern void chg_extchg_remove_proc(void);


boolean chg_is_extchg_overtemp(void);


extern void chg_extchg_monitor_func(void);

#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/


boolean chg_get_usb_online_status(void);


void chg_set_usb_online_status(boolean online);


boolean chg_get_ac_online_status(void);


void chg_set_ac_online_status(boolean online);


extern boolean is_chg_charger_removed(void);


extern struct chg_batt_data *chg_get_batt_data(unsigned int id_voltage);


extern int32_t chg_batt_volt_paras_init(void);


extern uint32_t chg_get_batt_id(void);

void chg_batt_data_dump(void);


void chg_set_supply_limit_by_stm_stat(void);

#if((FEATURE_ON == MBB_FACTORY)\
     && (FEATURE_ON == MBB_CHG_COULOMETER)\
     && (FEATURE_ON == MBB_CHG_EXTCHG ))

int32_t  chg_get_extchg_start(void);

void chg_check_vbus_volt(void);
#endif
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )

void chg_set_usb_stat_by_usb_temp(void);

int32_t chg_get_usb_health(void);

int32_t chg_get_usb_cur_temp(void);

boolean chg_get_usb_temp_protect_stat(void);

void chg_test_set_usb_temp_limit_and_resume(int32_t limit,int32_t resume);
#endif
#endif /*CHG_CHARGE_STM_H*/

