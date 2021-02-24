
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
 * �궨��                                       *
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
/* ��ʱ500ms */
#define EXTCHG_DELAY_COUNTER_SIZE               (500)

/* �����岹��� */
#if (MBB_CHG_COMPENSATE == FEATURE_ON)
/* ��ʱ10ms */
#define CHG_DELAY_COUNTER_SIZE                  (10)
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
/*��ز�����ֵ40%����*/
#define TBAT_SUPPLY_VOLT                        (3780)
/*��طŵ���ֵ80%����*/
#define TBAT_DISCHG_VOLT                        (4085)
 /*��س���ֹ��ѹ */
#define TBAT_SUPPLY_STOP_VOLT                   (3825)
 /*��طŵ��ֹ��ѹ */
#define TBAT_DISCHG_STOP_VOLT                   (4050)
#else
/*��ز�����ֵ40%����*/
#define TBAT_SUPPLY_VOLT                        (3696)
/*��طŵ���ֵ80%����*/
#define TBAT_DISCHG_VOLT                        (3970)
/*��س���ֹ��ѹ */
#define TBAT_SUPPLY_STOP_VOLT                   (3750)
/*��طŵ��ֹ��ѹ */
#define TBAT_DISCHG_STOP_VOLT                   (3865)
#endif/*HUAWEI_CHG_HIGH_VOLT_BATT*/
/*����ɹ�*/
#define TBAT_SUPPLY_CURR_SUCCESS                (0x0)
/*����Ҫ����*/
#define TBAT_NO_NEED_SUPPLY_CURR                (0x1)
/* ����ֹͣʱ�� */
#define TBAT_STOP_DELAY_COUNTER                 (100)
/* ��������ʱ�� */
#define TBAT_SUPLY_DELAY_COUNTER                (2300)
#endif /*MBB_CHG_COMPENSATE == FEATURE_ON */

/*�ػ����ػ�������**/
#define CHARGE_REMOVE_CHECK_MAX                 (1)

/*�Ǳ����س���������: ͬʱ������20pct���*/
#define CHG_DEF_BATT_CUR_LEVEL                  (512)
/*�Ǳ����س���ѹ����*/
#define CHG_DEF_BATT_VREG_LEVEL                 (4000)

#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
/*������ͱ�����������*/
/*����˵��*/

/*��ر����¶�����*/
#define CHG_BATTERY_PROTECT_TEMP            (45)
/*�˳���ر����¶�����*/
#define CHG_BATTERY_PROTECT_RESUME_TEMP     (42)
/*�¶ȳ�����ر�������ʱ����ص�ص�ѹ����*/
#define CHG_BATTERY_PROTECT_VOLTAGE         (4150)
/*�˳���ر�����ѹ����*/
#define CHG_BATTERY_PROTECT_RESUME_VOLTAGE  (4100)
/*��ر�����ֹ��ѹ����*/
#define CHG_BATTERY_PROTECT_CV_VOLTAGE      (4100)
/*����ͣ���ҳ������λ����suspend/ǰ������100MA��ʱ������*/
#define CHG_BATTERY_PROTECT_CHGER_TIME_THRESHOLD_IN_SECONDS (16 * 60 * 60)
/*����ͣ���ҳ������ʱ����λ״̬���¸�������*/
#define BATT_NORMAL_TEMP_RECHARGE_THR_LONG_TIME_NO_CHARGE    (4050)
#endif

/*----------------------------------------------*
 * �ṹ����                                      *
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

/*���¹ػ��¶Ȳ���NV50016�ṹ����*/
typedef struct
{
    uint32_t      ulIsEnable;             //���¹ػ�ʹ�ܿ���
    int32_t       lCloseAdcThreshold;     //���¹ػ��¶�����
    uint32_t      ulTempOverCount;        //���¹ػ��¶ȼ�����
}CHG_BATTERY_OVER_TEMP_PROTECT_NV;

/*���¹ػ��¶Ȳ���NV52005�ṹ����*/
typedef struct
{
    uint32_t      ulIsEnable;
    int32_t       lCloseAdcThreshold;
    uint32_t      ulTempLowCount;
}CHG_BATTERY_LOW_TEMP_PROTECT_NV;


/*���������¶Ȳ���NV 50385�ṹ����*/
typedef struct
{
    uint32_t    ulChargeIsEnable;                   //����±���ʹ��
    int32_t     overTempchgStopThreshold;           //�����±�������
    int32_t     subTempChgLimitCurrentThreshold;    //���³���������
    int32_t     lowTempChgStopThreshold;            //�����±�������
    int32_t     overTempChgResumeThreshold;         //�����»ָ��¶�����
    int32_t     lowTempChgResumeThreshold;          //�����»ָ��¶�����
    uint32_t    chgTempProtectCheckTimes;           //���ͣ����ѯ����
    uint32_t    chgTempResumeCheckTimes;            //��縴����ѯ����
    int32_t     exitWarmChgToNormalChgThreshold;    //�ɸ��³��ָ������³���¶�����
    int32_t     reserved2;                          //Ԥ��
}CHG_SHUTOFF_TEMP_PROTECT_NV_TYPE;

/*���������¶Ȳ���NV50386�ṹ����*/
typedef struct
{
    int32_t         battVoltPowerOnThreshold;           //������ѹ����
    int32_t         battVoltPowerOffThreshold;          //�ػ���ѹ����
    int32_t         battOverVoltProtectThreshold;       //ƽ������ѹ��������(ƽ��ֵ)
    int32_t         battOverVoltProtectOneThreshold;    //���γ���ѹ��������(����ֵ)
    int32_t         battChgTempMaintThreshold;          //��ʱͣ���ж�����
    int32_t         battChgRechargeThreshold;           //���³��ʱ�ĸ�������
    int32_t         VbatLevelLow_MAX;                   //�͵���������
    int32_t         VbatLevel0_MAX;                     //0���ѹ��������
    int32_t         VbatLevel1_MAX;                     //1���ѹ��������
    int32_t         VbatLevel2_MAX;                     //2���ѹ��������
    int32_t         VbatLevel3_MAX;                     //3���ѹ��������
    int32_t         battInsertChargeThreshold;          //���������ж��Ƿ�ʹ�ܳ����ж�����
    int32_t         battNormalTempRechergeThreshold;    //���³��ĸ�������
}CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE;

/* NV50364��ز������ݽṹ**/
typedef struct
{
    /*�ǹ���ģʽ�µ�ز���λ����ʹ�ܱ�־*/
    uint8_t no_battery_powerup_enable;
    /*�쳣�ػ����´β����Դ���뿪��ģʽʹ�ܱ�־*/
    uint8_t exception_poweroff_poweron_enable;
    /*�͵�ػ���ֹ��־*/
    uint8_t low_battery_poweroff_disable;
    /*����*/
    uint8_t reserved;
}POWERUP_MODE_TYPE;

/*��ѹ���0-10��֮�������ṹ��*/
typedef struct
{
    /*���������*/
    chg_chgr_type_t chgr_type;
    /*I_CHG��������*/
    uint32_t        current_limit;
    /*I_USB��������*/
    uint32_t        current_limit_usb;
}charger_current_limit_st;

/*���ID��Ϣ����*/
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

/*�����ز����ṹ��*/
struct chg_batt_data {
    unsigned int        id_voltage_min;         //��ѹ���IDʶ�����С��ѹ���ݲο���ѹ��ͬ��ѹ��ͬ
    unsigned int        id_voltage_max;         //��ѹ���IDʶ�������ѹ���ݲο���ѹ��ͬ��ѹ��ͬ
    boolean             is_20pct_calib;         //�Ƿ���Ҫ������20%�궨��־
    CHG_BATT_ID_TYPE    batt_id;
    CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE    chg_batt_volt_paras;
};

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
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

