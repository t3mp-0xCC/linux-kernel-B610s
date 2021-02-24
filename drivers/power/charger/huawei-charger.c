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
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/notifier.h>
#include <linux/debugfs.h>
#include <linux/workqueue.h>
#include <linux/ioctl.h>

#include "huawei-charger.h"
#include "chg_config.h"
#include "chg_charge_api.h"
#include "chg_charge_stm.h"
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
#ifdef CONFIG_MBB_FAST_ON_OFF
extern struct blocking_notifier_head g_fast_on_off_notifier_list;
#endif

#if (MBB_CHG_EXTCHG == FEATURE_ON)
extern int32_t g_ui_choose_exchg_mode;
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/

extern chg_stm_state_info_t chg_stm_state_info;
extern int smartstar_battery_current(void);
extern CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE   g_chgBattVoltProtect;      //NV50386
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

#if (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
boolean g_i2c_charger_suspend_flag = FALSE;
#endif

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
/* 接受USB各种事件用工作队列*/
struct workqueue_struct *mbb_chg_wq = NULL;

struct mbb_chg_work_stuct
{
    struct work_struct event_work;          /* 接受USB各种事件用处理任务*/
    unsigned long event;                    /* 事件类型*/
    int chg_type;                           /* 事件内容*/
};

struct mbb_chg_work_stuct mbb_chg_event_work;
/*USB温保*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
struct wake_lock chg_usb_temp_wake_lock ; /*USB温度异常状态防休眠锁*/
#endif

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
/*=======================/dev/charger IOCTL控制命令字 BEGIN================*/
/*===================新增特性命令必须按照此定义方式递增====================*/
#define CHG_CMD_BASE 0X88
/*通知开始补电*/
#define CHG_CMD                         _IOWR(CHG_CMD_BASE, 0, unsigned long)
/*查询是否需要补电*/
#define CHG_INQUIRE_CMD                 _IOWR(CHG_CMD_BASE, 1, unsigned long)
/*查询电池电压实时值*/
#define INQUIRE_VOLTAGE_CMD             _IOWR(CHG_CMD_BASE, 2, unsigned long)
/*查询校准后的电池电压*/
#define INQUIRE_CALIBRATION_VOLTAGE_CMD _IOWR(CHG_CMD_BASE, 3, unsigned long)
/*查询当前是否充电*/
#define UI_INQUIRE_BATT_STATE           _IOWR(CHG_CMD_BASE, 4, unsigned long)
/*查询充电器类型*/
#define UI_INQUIRE_CHARGER_TYPE         _IOWR(CHG_CMD_BASE, 5, unsigned long)
/*PT工位使能充电*/
#define INQUIRE_START_CHG_TASK_CMD      _IOWR(CHG_CMD_BASE, 6, unsigned long)
/*禁止本次对外充电*/
#define UI_DIABLE_EXTCHG_ONCE           _IOWR(CHG_CMD_BASE, 7, unsigned long)
/*开启本次对外充电*/
#define UI_ENABLE_EXTCHG_ONCE           _IOWR(CHG_CMD_BASE, 8, unsigned long)
/*查询是否支持库伦计*/
#define INQUIRE_COULOMETER_SUPPORT_CMD  _IOWR(CHG_CMD_BASE, 9, unsigned long)
/*查询电池温度*/
#define INQUIRE_BATT_TEMP_CMD  _IOWR(CHG_CMD_BASE, 10, unsigned long)
/*=======================/dev/charger IOCTL控制命令字 END==================*/

#define CHG_EVENT_WAKE_TIMEOUT            (6000)
#define CHG_EVENT_FTM_WAKE_TIMEOUT        (1000)

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
//#define pr_fmt(fmt) "%s: " fmt, __func__

/*充电LOG打印控制，默认不输出Log以满足可测试性Log打印需求*/
int32_t g_chg_log_level = 0;

//echo 0 > /sys/module/huawei_charger/parameters/g_chg_log_level
module_param_named(
    g_chg_log_level, g_chg_log_level, int, S_IRUGO | S_IWUSR | S_IWGRP
);

static struct class *g_charger_class = NULL;

struct i2c_client *g_i2c_client = NULL;
struct chargeIC_chip *g_chip = NULL;

#if (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
/*用于控制是否使用RTC进行待机休眠后的周期电压温度采集*/
static boolean rtc_alarm_use_flag = FALSE;

struct rtc_device *g_alarm_rtc_dev = NULL;
struct rtc_task *g_alarm_rtc_task = NULL;
int g_init_flag = 0;
#define BATT_VOLT_POWER_OFF_THRESHOLD       (3450)
#define CHARGE_ALARM_TIME_IN_SEC            (1200)
#define ALARM_REPORT_OVERTEMP_THRESHOLD     (58)
#define ALARM_REPORT_SHUTOFF_HIGH_THRESHOLD (60)
#define ALARM_REPORT_SHUTOFF_LOW_THRESHOLD  (-20)

#define ALARM_REPORT_WAKELOCK_TIMEOUT       (5 * HZ)
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/

#if (MBB_CHG_BQ24196 == FEATURE_ON)
#define CHARGE_IC_DRVNAME               "bq24192"
#define DTS_COMPATIBLE_NAME             "ti,bq24192"
/*BQ24196的I2C寄存器地址*/
#define I2C_CHARGER_IC_ADDR             (0x6B)
#define DEV_ID_REG                      (0x0a)
#define DEV_ID_DEV_REG_MASK             (0x3)
#define DEV_ID_DEV_REG_SHIFT            (0)
#define CHARER_CONCTRL_REG_START_ADDR   (0)
#define CHARER_CONCTRL_REGISTER_COUNT   (10)
#define CHARER_COMMAND_REG_START_ADDR   (0)
#define CHARER_COMMAND_REGISTER_COUNT   (0)
#define CHARER_STATUS_REG_START_ADDR    (0)
#define CHARER_STATUS_REGISTER_COUNT    (0)
#elif (MBB_CHG_SMB1351 == FEATURE_ON)
#define CHARGE_IC_DRVNAME               "smb1351"
#define DTS_COMPATIBLE_NAME             "qcom,smb1351"
/*smb1351的I2C寄存器地址*/
#define I2C_CHARGER_IC_ADDR             (0x55)
#define DEV_ID_REG                      (0x3F)
#define DEV_ID_DEV_REG_MASK             (0x60)
#define DEV_ID_DEV_REG_SHIFT            (0x5)
#define CHARER_CONCTRL_REG_START_ADDR   (0)
#define CHARER_CONCTRL_REGISTER_COUNT   (25)
#define CHARER_COMMAND_REG_START_ADDR   (48) //0x30
#define CHARER_COMMAND_REGISTER_COUNT   (5)
#define CHARER_STATUS_REG_START_ADDR    (54) //0x36
#define CHARER_STATUS_REGISTER_COUNT    (18)
#elif (MBB_CHG_BQ25892 == FEATURE_ON)
#define I2C_CHARGER_IC_ADDR             (0x6B)
#define DEV_ID_REG                      (0x0a)
#define DEV_ID_DEV_REG_MASK             (0x3)
#define DEV_ID_DEV_REG_SHIFT            (0)
#define CHARER_CONCTRL_REG_START_ADDR   (0)
#define CHARER_CONCTRL_REGISTER_COUNT   (10)
#define CHARER_COMMAND_REG_START_ADDR   (0)
#define CHARER_COMMAND_REGISTER_COUNT   (0)
#define CHARER_STATUS_REG_START_ADDR    (0)
#define CHARER_STATUS_REGISTER_COUNT    (0)
#endif/*MBB_CHG_SMB1351 == FEATURE_ON*/

static enum power_supply_property battery_monitor_props[] = {
    POWER_SUPPLY_PROP_STATUS,
    POWER_SUPPLY_PROP_HEALTH,
    POWER_SUPPLY_PROP_PRESENT,
    POWER_SUPPLY_PROP_TECHNOLOGY,
    POWER_SUPPLY_PROP_VOLTAGE_AVG,
    POWER_SUPPLY_PROP_CAPACITY,
    POWER_SUPPLY_PROP_TEMP,
    POWER_SUPPLY_PROP_TIME_TO_FULL_AVG,
#if (FEATURE_ON == MBB_FACTORY)
    POWER_SUPPLY_PROP_CURRENT_NOW,
#endif
};

static enum power_supply_property battery_monitor_usb_props[] = {
    POWER_SUPPLY_PROP_ONLINE,
    /*USB温保*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    POWER_SUPPLY_PROP_USB_HEALTH, 
#endif
};

static enum power_supply_property battery_monitor_ac_props[] = {
    POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property battery_monitor_extchg_props[] = {
    POWER_SUPPLY_PROP_ONLINE,
    POWER_SUPPLY_PROP_EXTCHG_STA,
#if((FEATURE_ON == MBB_FACTORY)\
     && (FEATURE_ON == MBB_CHG_COULOMETER)\
     && (FEATURE_ON == MBB_CHG_EXTCHG ))
    POWER_SUPPLY_PROP_EXTCHG_START,
#endif
};
static enum power_supply_property battery_monitor_wireless_props[] = {
    POWER_SUPPLY_PROP_ONLINE,
};


static int battery_monitor_get_property(struct power_supply *psy,
                    enum power_supply_property psp,
                    union power_supply_propval *val)
{
    struct chargeIC_chip *chip = NULL;
    chip = container_of(psy, struct chargeIC_chip, bat);

    switch (psp)
    {
        case POWER_SUPPLY_PROP_STATUS:
            val->intval = chip->bat_stat;
            break;

        case POWER_SUPPLY_PROP_VOLTAGE_AVG:
            val->intval = chip->bat_avg_voltage;
            break;

        case POWER_SUPPLY_PROP_TEMP:
            val->intval = chip->bat_avg_temp;
            break;

        case POWER_SUPPLY_PROP_PRESENT:
            val->intval = chip->bat_present;
            break;
        case POWER_SUPPLY_PROP_HEALTH:
            val->intval = chip->bat_health;
            break;
        case POWER_SUPPLY_PROP_CAPACITY:
            val->intval = chip->bat_capacity;
            break;
        case POWER_SUPPLY_PROP_TECHNOLOGY:
            val->intval = chip->bat_technology;
            break;
        case POWER_SUPPLY_PROP_TIME_TO_FULL_AVG:
            val->intval = chip->bat_time_to_full;
            break;
#if (FEATURE_ON == MBB_FACTORY)
        case POWER_SUPPLY_PROP_CURRENT_NOW:
            val->intval = smartstar_battery_current();
            break;
#endif
        default:
            dev_err(&chip->client->dev, "%s defualt run.\n", __func__);
            return - EINVAL;
    }

    return 0;
}


static int battery_monitor_ac_get_property(struct power_supply *psy,
                    enum power_supply_property psp,
                    union power_supply_propval *val)
{
    struct chargeIC_chip *chip = NULL;
    chip = container_of(psy, struct chargeIC_chip, ac);

    switch (psp)
    {
        case POWER_SUPPLY_PROP_ONLINE:
            val->intval = chip->ac_online;
            break;
        default:
            return - EINVAL;
    }

    return 0;
}


static int battery_monitor_usb_get_property(struct power_supply *psy,
                    enum power_supply_property psp,
                    union power_supply_propval *val)
{
    struct chargeIC_chip *chip = NULL;
    chip = container_of(psy, struct chargeIC_chip, usb);

    switch (psp)
    {
        case POWER_SUPPLY_PROP_ONLINE:
            val->intval = chip->usb_online;
            break;
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
        case POWER_SUPPLY_PROP_USB_HEALTH:
            val->intval = chip->usb_health;
            break;
#endif
        default:
            return - EINVAL;
    }

    return 0;
}


static int battery_monitor_extchg_get_property(struct power_supply *psy,
                    enum power_supply_property psp,
                    union power_supply_propval *val)
{
    struct chargeIC_chip *chip = NULL;
    chip = container_of(psy, struct chargeIC_chip, extchg);

    switch (psp)
    {
        case POWER_SUPPLY_PROP_ONLINE:
            val->intval = chip->extchg_online;
            break;
        case POWER_SUPPLY_PROP_EXTCHG_STA:
            val->intval = chip->extchg_status;
            break;
#if((FEATURE_ON == MBB_FACTORY)\
     && (FEATURE_ON == MBB_CHG_COULOMETER)\
     && (FEATURE_ON == MBB_CHG_EXTCHG ))
        case POWER_SUPPLY_PROP_EXTCHG_START:
            val->intval = chip->extchg_start;
            break;
#endif
        default:
            return - EINVAL;
    }

    return 0;
}


static int battery_monitor_wireless_get_property(struct power_supply *psy,
                    enum power_supply_property psp,
                    union power_supply_propval *val)
{
    struct chargeIC_chip *chip = NULL;
    chip = container_of(psy, struct chargeIC_chip, wireless);

    switch (psp)
    {
        case POWER_SUPPLY_PROP_ONLINE:
            val->intval = chip->wireless_online;
            break;
        default:
            return - EINVAL;
    }

    return 0;
}


static int chargeIC_i2c_read(uint8_t reg, uint8_t *pval)
{
    int ret = 0;

    if(NULL == g_i2c_client)
    {
        pr_err("CHG_HUAWEI_CHARGER:%s:failed due to g_i2c_client doesn't exist@~\n", __FUNCTION__);
        return -1;
    }


    ret = i2c_smbus_read_byte_data(g_i2c_client, reg);

    if(ret < 0)
    {
        pr_err("CHG_HUAWEI_CHARGER:%s:fail to i2c_smbus_read_byte_data reg=0x%02X, ret=%d\n",
            __FUNCTION__, reg, ret);
        return -1;
    }
    else
    {
        *pval = ret;
        //pr_info("CHG_HUAWEI_CHARGER:read(0x%02X)=0x%02X\n", reg, *pval);
        return 0;
    }
}


static int chargeIC_i2c_write(uint8_t reg, uint8_t val)
{
    int32_t ret = 0;
    if(NULL == g_i2c_client)
    {
        pr_err("CHG_HUAWEI_CHARGER:%s:failed due to g_i2c_client doesn't exist@~\n", __FUNCTION__);
        return -1;
    }


    ret = i2c_smbus_write_byte_data(g_i2c_client, reg, val);
    if(0 == ret)
    {
        //pr_info("CHG_HUAWEI_CHARGER:write(0x%02X)=0x%02X\n", reg, val);
        return 0;
    }
    else
    {
        pr_err("CHG_HUAWEI_CHARGER:%s:fail to i2c_smbus_write_byte_data reg=0x%02X, ret=%d\n",
            __FUNCTION__, reg, ret);
        return -1;
    }
}


static int chargeIC_convert_atoi(char *name)
{
    int val = 0;

    for (;; name++)
    {
        switch (*name)
        {
            case '0' ... '9':
            val = ATOI_CONVERT_NUM * val + (*name - '0');
            break;
            default:
            return val;
        }
    }
}


static ssize_t chargeIC_proc_read(struct file *filp,
    char *buffer, size_t length, loff_t *offset)
{
    int ret  = 0;
    int ret2 = 0;
    int pval = 0;
    int reg_index = 0;
    int reg_address = 0;

    /*控制寄存器*/
    if(0 != CHARER_CONCTRL_REGISTER_COUNT)
    {
        reg_address = CHARER_CONCTRL_REG_START_ADDR;
        for(reg_index = 0; reg_index < CHARER_CONCTRL_REGISTER_COUNT; reg_index++)
        {
            ret = chargeIC_i2c_read(reg_address, (uint8_t *)&pval); //reg num
            pr_info("CHG_HUAWEI_CHARGER:IC Control REG 0x%X = 0x%02X\n", reg_address,pval);
            ret2 = copy_to_user(buffer, (uint8_t *)&pval, sizeof(pval));
            if(0 != ret || 0 != ret2)
            {
                pr_err("CHG_HUAWEI_CHARGER:READ chargeIC Registers Failed!\n");
            }
            reg_address ++;
        }
    }
    /*命令寄存器*/
    if(0 != CHARER_COMMAND_REGISTER_COUNT)
    {
        reg_address = CHARER_COMMAND_REG_START_ADDR;
        for(reg_index = 0; reg_index < CHARER_COMMAND_REGISTER_COUNT; reg_index++)
        {
            ret = chargeIC_i2c_read(reg_address, (uint8_t *)&pval); //reg num
            pr_info("CHG_HUAWEI_CHARGER:IC Command REG 0x%X = 0x%02X\n", reg_address,pval);
            ret2 = copy_to_user(buffer, (uint8_t *)&pval, sizeof(pval));
            if(0 != ret || 0 != ret2)
            {
                pr_err("CHG_HUAWEI_CHARGER:READ chargeIC Registers Failed!\n");
            }
            reg_address ++;
        }
    }
    /*状态寄存器*/
    if(0 != CHARER_STATUS_REGISTER_COUNT)
    {
        reg_address = CHARER_STATUS_REG_START_ADDR;
        for(reg_index = 0; reg_index < CHARER_STATUS_REGISTER_COUNT; reg_index++)
        {
            ret = chargeIC_i2c_read(reg_address, (uint8_t *)&pval); //reg num
            pr_info("CHG_HUAWEI_CHARGER:IC Status REG 0x%X = 0x%02X\n", reg_address,pval);
            ret2 = copy_to_user(buffer, (uint8_t *)&pval, sizeof(pval));
            if(0 != ret || 0 != ret2)
            {
                pr_err("CHG_HUAWEI_CHARGER:READ chargeIC Registers Failed!\n");
            }
            reg_address ++;
        }
    }

    return (ssize_t)ret;
}


static ssize_t chargeIC_proc_write(struct file *filp,
    const char *buff, size_t len, loff_t *off)
{
    uint8_t reg = 0;
    uint8_t val = 0;
    int32_t ret = 0;
    char tmp_buff[ATOI_MAX_LENGTH] = {0};
    char Reg_val[ATOI_MAX_LENGTH] = {0};
    char Reg_Num[ATOI_MAX_LENGTH] = {0};
    size_t cnt = 0;
    char *p = NULL;

    if(len >= ATOI_MAX_LENGTH)
    {
        pr_err("CHG_HUAWEI_CHARGER:chargeIC_proc_write Invalid argument!\n");
        return - EINVAL;
    }

    if (copy_from_user(tmp_buff,buff,len))
    {
        pr_err("CHG_HUAWEI_CHARGER:chargeIC_proc_write fail to copy from usr!\n");
        return - EINVAL;
    }

    /* 求得Reg_Num 字串*/
    p = tmp_buff;
    while (*p && (*p != ' '))
    {
        Reg_Num[cnt] = *p;
        cnt++;
        p++;
    }
    Reg_Num[cnt] = '\0';

    /* 求得Reg_val 字串*/
    cnt = 0;
    while (*p && (*p != '\0'))
    {
        if(' ' == *p)
        {
            p++;
        }
        else
        {
            Reg_val[cnt] = *p;
            cnt++;
            p++;
        }
    }
    Reg_val[cnt] = '\0';

    reg = (uint8_t)chargeIC_convert_atoi(Reg_Num);
    val = (uint8_t)chargeIC_convert_atoi(Reg_val);

    /*write to i2c register*/
    ret = chargeIC_i2c_write(reg, val);
    if (0 != ret)
    {
        pr_err("CHG_HUAWEI_CHARGER:ERROR write to REG 0x%X !\n",reg);
    }
    /*write successfully*/
    pr_info("CHG_HUAWEI_CHARGER:Write to REG 0x%X value 0x%02X successfully \n", reg,val);
    return len;
}

#if (defined(MBB_HWTEST) || defined(CONFIG_HARDWARE_TEST))
#define TEST_REG    0x03
#define TEST_REG_VAL    0x55

int chg_is_chargeIC_communication_normal(void)
{
    int ret = -1;
    uint8_t reg = 0;

    ret = chargeIC_i2c_write(TEST_REG, TEST_REG_VAL);
    if (0 != ret)
    {
        pr_err("CHG_HUAWEI_CHARGER: ERROR write to i2c register %d !\n",TEST_REG);
        return FALSE;
    }
    ret = chargeIC_i2c_read(TEST_REG,(uint8_t *)&reg);
    if(0 != ret )
    {
        pr_err("CHG_HUAWEI_CHARGER: READ chargeIC Registers Failed!\n");
        return FALSE;
    }
    pr_err("CHG_HUAWEI_CHARGER: reg:%d,reg_val:0x%02X\n",TEST_REG,reg);
    if(TEST_REG_VAL == reg)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}
#endif/*(FEATURE_ON == MBB_HWTEST) || defined(CONFIG_HARDWARE_TEST)*/


static ssize_t dload_chg_proc_read(struct file *filp,
    char *buffer, size_t length, loff_t *offset)
{
    int32_t batt_volt_value = 0;
    int ret = -1;
    BATT_LEVEL_ENUM bat_volt_level = BATT_LEVEL_MAX;
    int32_t bat_volt_level_value = 0;

    batt_volt_value = chg_get_batt_volt_value();

    pr_info("CHG_HUAWEI_CHARGER: dload_chg_proc_read: batt_volt %d!\n", batt_volt_value);

    if(batt_volt_value < g_chgBattVoltProtect.VbatLevelLow_MAX)
    {
        bat_volt_level = BATT_LOW_POWER;
    }

    else if(batt_volt_value < g_chgBattVoltProtect.VbatLevel1_MAX)
    {
        bat_volt_level = BATT_LEVEL_1;
    }
    else if(batt_volt_value < g_chgBattVoltProtect.VbatLevel2_MAX)
    {
        bat_volt_level = BATT_LEVEL_2;
    }
    else if(batt_volt_value < g_chgBattVoltProtect.VbatLevel3_MAX)
    {
        bat_volt_level = BATT_LEVEL_3;
    }
    else
    {
        bat_volt_level = BATT_LEVEL_4;
    }

    pr_info("CHG_HUAWEI_CHARGER: dload_chg_proc_read: bat_volt_level %d!\n", bat_volt_level);

    if(length < sizeof(bat_volt_level))
    {
        pr_err("CHG_HUAWEI_CHARGER:dload_chg_proc_read Invalid argument!\n");
        return - EINVAL;
    }

    bat_volt_level_value = (int32_t)bat_volt_level;
    ret = copy_to_user(buffer, (void *)&bat_volt_level_value, sizeof(bat_volt_level_value));
    if(0 != ret)
    {
        pr_err("CHG_HUAWEI_CHARGER: dload_chg_proc_read: READ dload chg Failed!\n");
        return - EINVAL;
    }
    return (ssize_t)length;
}


static ssize_t poweron_chg_proc_read(struct file *filp,
    char *buffer, size_t length, loff_t *offset)
{
    int len = 0;
    boolean is_powdown_charging = FALSE;
    int start_mode = DRV_START_MODE_NORMAL;

    is_powdown_charging = chg_is_powdown_charging();
    pr_info("CHG_HUAWEI_CHARGER: poweron_chg_proc_read: is_powdown_charging=%d!\n", is_powdown_charging);
    if (TRUE == is_powdown_charging)
    {
        start_mode = DRV_START_MODE_CHARGING;
    }
    else
    {
        start_mode = DRV_START_MODE_NORMAL;
    }

    len = copy_to_user(buffer, (void *)&start_mode, sizeof(int));

    if(0 != len)
    {
        pr_err("CHG_HUAWEI_CHARGER: poweron_chg_proc_read: READ poweron chg Failed!\n");
    }

    return (ssize_t)len;
}

static struct file_operations chargeIC_proc_ops = {
    .read  = chargeIC_proc_read,
    .write = chargeIC_proc_write,
};

static struct file_operations dload_chg_proc_ops = {
    .read  = dload_chg_proc_read,
};

static struct file_operations poweron_chg_proc_ops = {
    .read  = poweron_chg_proc_read,
};


static void create_chargeIC_proc_file(void)
{
    struct proc_dir_entry *proc_file = NULL;
    /*创建文件节点，并设置节点权限为644*/
    proc_file = proc_create(CHARGEIC_PROC_FILE, 0644, NULL,&chargeIC_proc_ops);

    if(NULL == proc_file)

    {
        pr_err("CHG_HUAWEI_CHARGER:%s:create proc entry for chargeIC failed\n", __FUNCTION__);
    }
}


static void create_dload_chg_proc_file(void)
{
    struct proc_dir_entry *proc_file = NULL;
    /*创建文件节点，并设置节点权限为444*/
    proc_file = proc_create(DLOAD_CHG_PROC_FILE, 0444, NULL,&dload_chg_proc_ops);

    if(NULL == proc_file)

    {
        pr_err("CHG_HUAWEI_CHARGER:%s: create proc entry for dload_chg failed\n", __FUNCTION__);
    }
}


static void create_poweron_chg_proc_file(void)
{
    struct proc_dir_entry *proc_file = NULL;
    /*创建文件节点，并设置节点权限为444*/
    proc_file = proc_create(POWERON_CHG_PROC_FILE, 0444, NULL,&poweron_chg_proc_ops);

    if(NULL == proc_file)

    {
        pr_err("CHG_HUAWEI_CHARGER:%s: create proc entry for poweron failed\n", __FUNCTION__);
    }
}


static void charge_event_work(struct work_struct *w)
{
    static unsigned long old_plug = 0xFFFFFF;
    static int old_chg_type = -1;
    static int chg_wakelock_st = 0;

    if ((old_plug == mbb_chg_event_work.event)
        &&(old_chg_type == mbb_chg_event_work.chg_type))
    {
        pr_err("[MBB CHG]%s: plug event report same plug=%ld,chg_type=%d!!!\r\n",
            __func__,old_plug,old_chg_type);
        return;
    }
    old_plug = mbb_chg_event_work.event;
    old_chg_type = mbb_chg_event_work.chg_type;

    /*外电源插入加充电任务永久锁，防止系统待机*/
    if(old_plug)
    {
        if(0 == chg_wakelock_st)
        {
            chg_wakelock_st=1;
            wake_lock(&g_chip->chg_wake_lock);
            pr_err("CHG_HUAWEI_CHARGER:%s: Charger Plug IN Lock chg_wake_lock\r\n", __func__);
        }
        else
        {
            pr_err("CHG_HUAWEI_CHARGER:%s: Charger Plug IN But Already Lock chg_wake_lock\r\n", __func__);
        }
    }
    /*外电源拔出释放充电任务永久锁，允许系统待机*/
    else
    {
        chg_wakelock_st = 0;
        wake_unlock(&g_chip->chg_wake_lock);
        pr_err("CHG_HUAWEI_CHARGER:%s: Charger PullOut Unlock chg_wake_lock\r\n", __func__);
    }

    pr_err("[MBB CHG]%s :%ld %d.\r\n", __func__, mbb_chg_event_work.event, mbb_chg_event_work.chg_type);
    get_chgr_type_from_usb(mbb_chg_event_work.event, mbb_chg_event_work.chg_type);
}
#if (MBB_CHG_EXTCHG == FEATURE_ON)

static void chg_extchg_monitor_work(struct work_struct *work)
{
/*
    struct chargeIC_chip *chip = container_of(work,
				struct chargeIC_chip,
				extchg_monitor_work.work);
*/
    /*对外充电监控*/
    chg_extchg_monitor_func();

}
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/

#if (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)

static void start_charge_alarm(boolean enable, long time, int freq)
{
    int32_t ret = 0;
    struct rtc_wkalrm alarm = {0};
    long alarm_time = 0;

    ret = rtc_read_alarm(g_chip->rtc, &alarm);
    if (0 != ret)
    {
        pr_err("CHG_HUAWEI_CHARGER: Unable to read charge alarm ret %d\n", ret);
        return;
    }

    rtc_tm_to_time(&alarm.time, &alarm_time);
    alarm_time += time;
    rtc_time_to_tm(alarm_time, &alarm.time);
    pr_err("CHG_HUAWEI_CHARGER: Rtc_read_alarm charge alarm sec=%ld\n", alarm_time);



    alarm.enabled = TRUE;
    ret = rtc_set_alarm(g_chip->rtc, &alarm);
    if (0 != ret)
    {
        pr_err("CHG_HUAWEI_CHARGER:Unable to set charge alarm ret %d\n", ret);
        return;
    }
    else
    {
        pr_err("CHG_HUAWEI_CHARGER:Alarm enabled %d \n", alarm.enabled);
    }
}
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/


int battery_monitor_blocking_notifier_call_chain(unsigned long val, unsigned long v)
{
    if (NULL == g_chip)
    {
        return -1;
    }

    //return blocking_notifier_call_chain(&g_chip->notifier_list_bat, val,(void *)&v);

    mbb_chg_event_work.event = (int)val;
    mbb_chg_event_work.chg_type = (int)v;
    pr_err("[MBB CHG]%s :%ld %d.\r\n", __func__, mbb_chg_event_work.event, mbb_chg_event_work.chg_type);
    /*由于事件通知可能在中断上下文，因此将工作任务加入到工作队列中处理*/
    if (FALSE == chg_is_ftm_mode())
    {
        /*如果不是工厂模式，超时锁为6s*/
        wake_lock_timeout(&g_chip->alarm_wake_lock, (long)msecs_to_jiffies(CHG_EVENT_WAKE_TIMEOUT));
        pr_info("CHG_HUAWEI_CHARGER: wake lock timeout is 6s!\n");
    }
    else
    {
        /*如果是工厂模式，超时锁为1s*/
        wake_lock_timeout(&g_chip->alarm_wake_lock, (long)msecs_to_jiffies(CHG_EVENT_FTM_WAKE_TIMEOUT));
        pr_info("CHG_HUAWEI_CHARGER: wake lock timeout is 1s!\n");
    }
     queue_work(mbb_chg_wq, &(mbb_chg_event_work.event_work));

    return 0;
}

EXPORT_SYMBOL_GPL(battery_monitor_blocking_notifier_call_chain);

#if (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)

void modem_notify_get_batt_info_callback(void )
{
    int32_t batt_volt = 0;
    int32_t batt_temp = 0;
    static boolean low_vbat_report_flag = FALSE;
    static boolean overheat_report_flag = FALSE;

    batt_volt = (int32_t)chg_get_batt_volt_value();
    batt_temp = (int32_t)chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);

    pr_err("CHG_HUAWEI_CHARGER:***modem_notify_get_batt_info_callback, batt_volt %d,batt_temp %d!\n",
           batt_volt, batt_temp);
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    /*有线/无线/对外充电器在位不进行低电或者过温处理*/
    if ((FALSE == is_chg_charger_removed()) || (TRUE == chg_stm_get_extchg_online_st()))
    {
        pr_err("CHG_HUAWEI_CHARGER:***modem_notify_get_batt_info_callback charger plug in do nothing!***\n");
        return ;
    }
#else
    if (FALSE == is_chg_charger_removed())
    {
        pr_err("CHG_HUAWEI_CHARGER:***modem_notify_get_batt_info_callback charger plug in do nothing!***\n");
        return ;
    }
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/

    if (g_chgBattVoltProtect.VbatLevelLow_MAX < batt_volt)
    {
        pr_err("CHG_HUAWEI_CHARGER:***batt_volt > low battery threshold do nothing!***\n");
    }
    else if ((BATT_VOLT_POWER_OFF_THRESHOLD < batt_volt)
            && (g_chgBattVoltProtect.VbatLevelLow_MAX >= batt_volt))
    {
        pr_err("CHG_HUAWEI_CHARGER:***Batt_volt is in 3450-3550mV report low battery***\n");
        if(FALSE == low_vbat_report_flag)
        {
            wake_lock_timeout(&g_chip->alarm_wake_lock, ALARM_REPORT_WAKELOCK_TIMEOUT);
            chg_set_sys_batt_capacity(BATT_CAPACITY_LEVELLOW);
            pr_err("CHG_HUAWEI_CHARGER:send MSG to app for show low power! \n ");
            low_vbat_report_flag = TRUE;
        }
        else
        {
            pr_err("CHG_HUAWEI_CHARGER:low_vbat_report_flag = true!\n ");
        }
    }
    else
    {
        wake_lock_timeout(&g_chip->alarm_wake_lock, ALARM_REPORT_WAKELOCK_TIMEOUT);
        pr_err("CHG_HUAWEI_CHARGER:***batt_volt < poweroff voltage threshold report power off***\n");
        chg_set_sys_batt_capacity(BATT_CAPACITY_SHUTOFF);
    }

    if ((ALARM_REPORT_OVERTEMP_THRESHOLD > batt_temp)
        && (ALARM_REPORT_SHUTOFF_LOW_THRESHOLD < batt_temp))
    {
        pr_err("CHG_HUAWEI_CHARGER:***Batt_temp is in -20-58 deg do nothing***\n");
    }
    else if (ALARM_REPORT_OVERTEMP_THRESHOLD <= batt_temp)
    {
        if (ALARM_REPORT_SHUTOFF_HIGH_THRESHOLD <= batt_temp)
        {
            wake_lock_timeout(&g_chip->alarm_wake_lock, ALARM_REPORT_WAKELOCK_TIMEOUT);
            pr_err("CHG_HUAWEI_CHARGER:***Batt_temp is more than 60 deg, power off***\n");
            chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_DEAD;
            chg_send_stat_to_app((uint32_t)DEVICE_ID_KEY, (uint32_t)GPIO_KEY_POWER_OFF);
        }
        else
        {
            pr_err("CHG_HUAWEI_CHARGER:***Batt_temp is in 58-60 deg, report overheat***\n");
            if(FALSE == overheat_report_flag)
            {
                wake_lock_timeout(&g_chip->alarm_wake_lock, ALARM_REPORT_WAKELOCK_TIMEOUT);
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_OVERHEAT;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_HIGH);
                pr_err("CHG_HUAWEI_CHARGER:send MSG to app for show overheat! \n ");
                overheat_report_flag = TRUE;
            }
            else
            {
                pr_err("CHG_HUAWEI_CHARGER:overheat_report_flag = true!\n ");
            }
        }
    }
    else
    {
        wake_lock_timeout(&g_chip->alarm_wake_lock, ALARM_REPORT_WAKELOCK_TIMEOUT);
        pr_err("CHG_HUAWEI_CHARGER:***Batt_temp is less than -20 deg, report cold***\n");
        chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_COLD;
        chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_LOW);
    }
    pr_err("CHG_HUAWEI_CHARGER:***wait next modem time expire!***\n");
}

static void alarm_work_func(struct work_struct *w)
{
    int32_t batt_volt = 0;
    int32_t batt_temp = 0;
    static boolean low_vbat_report_flag = FALSE;
    static boolean overheat_report_flag = FALSE;

    batt_volt = (int32_t)chg_get_batt_volt_value();
    batt_temp = (int32_t)chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);

    pr_err("CHG_HUAWEI_CHARGER:***Charge alarm %d timer expired, batt_volt %d,batt_temp %d!***\n",
                CHARGE_ALARM_TIME_IN_SEC, batt_volt, batt_temp);

#if (MBB_CHG_EXTCHG == FEATURE_ON)
    /*有线/无线/对外充电器在位不进行RTC低电或者过温处理*/
    if ((FALSE == is_chg_charger_removed()) || (TRUE == chg_stm_get_extchg_online_st()))
    {
        pr_err("CHG_HUAWEI_CHARGER:***Charge alarm timer expired but charger plug in do nothing!***\n");
        return ;
    }
#else
    if (FALSE == is_chg_charger_removed())
    {
        pr_err("CHG_HUAWEI_CHARGER:***Charge alarm timer expired but charger plug in do nothing!***\n");
        return ;
    }
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/

    if (g_chgBattVoltProtect.VbatLevelLow_MAX < batt_volt)
    {
        pr_err("CHG_HUAWEI_CHARGER:***batt_volt > low battery threshold continue timer***\n");
    }
    else if ((BATT_VOLT_POWER_OFF_THRESHOLD < batt_volt)
            && (g_chgBattVoltProtect.VbatLevelLow_MAX >= batt_volt))
    {
        pr_err("CHG_HUAWEI_CHARGER:***Batt_volt is in 3450-3550mV report low battery***\n");
        if(FALSE == low_vbat_report_flag)
        {
            wake_lock_timeout(&g_chip->alarm_wake_lock, ALARM_REPORT_WAKELOCK_TIMEOUT);
            chg_set_sys_batt_capacity(BATT_CAPACITY_LEVELLOW);
            pr_err("CHG_HUAWEI_CHARGER:send MSG to app for show low power! \n ");
            low_vbat_report_flag = TRUE;
        }
        else
        {
            pr_err("CHG_HUAWEI_CHARGER:low_vbat_report_flag = true!\n ");
        }
    }
    else
    {
        pr_err("CHG_HUAWEI_CHARGER:***batt_volt < poweroff voltage threshold report power off***\n");
        chg_set_sys_batt_capacity(BATT_CAPACITY_SHUTOFF);
    }

    if ((ALARM_REPORT_OVERTEMP_THRESHOLD > batt_temp)
        && (ALARM_REPORT_SHUTOFF_LOW_THRESHOLD < batt_temp))
    {
        pr_err("CHG_HUAWEI_CHARGER:***Batt_temp is in -20-58 deg continue timer***\n");
    }
    else if (ALARM_REPORT_OVERTEMP_THRESHOLD <= batt_temp)
    {
        if (ALARM_REPORT_SHUTOFF_HIGH_THRESHOLD <= batt_temp)
        {
            wake_lock_timeout(&g_chip->alarm_wake_lock, ALARM_REPORT_WAKELOCK_TIMEOUT);
            pr_err("CHG_HUAWEI_CHARGER:***Batt_temp is more than 60 deg, power off***\n");
            chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_DEAD;
            chg_send_stat_to_app((uint32_t)DEVICE_ID_KEY, (uint32_t)GPIO_KEY_POWER_OFF);
        }
        else
        {
            pr_err("CHG_HUAWEI_CHARGER:***Batt_temp is in 58-60 deg, report overheat***\n");
            if(FALSE == overheat_report_flag)
            {
                wake_lock_timeout(&g_chip->alarm_wake_lock, ALARM_REPORT_WAKELOCK_TIMEOUT);
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_OVERHEAT;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_HIGH);
                pr_err("CHG_HUAWEI_CHARGER:send MSG to app for show overheat! \n ");
                overheat_report_flag = TRUE;
            }
            else
            {
                pr_err("CHG_HUAWEI_CHARGER:overheat_report_flag = true!\n ");
            }
        }
    }
    else
    {
        wake_lock_timeout(&g_chip->alarm_wake_lock, ALARM_REPORT_WAKELOCK_TIMEOUT);
        pr_err("CHG_HUAWEI_CHARGER:***Batt_temp is less than -20 deg, report cold***\n");
        chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_COLD;
        chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_LOW);
    }

    start_charge_alarm(TRUE, CHARGE_ALARM_TIME_IN_SEC, 0);
    pr_err("CHG_HUAWEI_CHARGER:*** Enable charge %d seconds alarm again!\n", CHARGE_ALARM_TIME_IN_SEC);
}
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/
#ifdef CONFIG_MBB_FAST_ON_OFF

static int suspend_monitor_charger_event(struct notifier_block *nb,
                                            unsigned long val, void *data)
{
    int ret = 0;
    pr_info("CHG_HUAWEI_CHARGER:%s :suspend_monitor_charger_event%ld \n", __func__, val);
    chg_get_system_suspend_status(val);
    return ret;
}

static struct notifier_block suspend_monitor_charger_block = {
    .notifier_call = suspend_monitor_charger_event
};
#endif/*CONFIG_MBB_FAST_ON_OFF*/


boolean chg_fact_enable_chg_supplement(void)
{
    boolean ret = FALSE;

    ret = chg_tbat_chg_sply();

    return ret;
}


static int   chargerdev_read (struct file *file, char __user *buf,
    size_t count,loff_t *offset)
{
    return 0;
}


static int  chargerdev_write (struct file *file, const char __user *buf,
            size_t count,loff_t *offset)
{
    return 0;
}


static long chargerdev_ioctl(struct file *file, unsigned int cmd,
    unsigned long arg)
{
    boolean batt_state = FALSE;
    boolean batt_fact_chg = 0;
    boolean batt_coulometer_support = NO;
    chg_chgr_type_t cur_chgr_type = CHG_CHGR_INVALID;
    int32_t batt_volt = 0;
    int32_t batt_sys_temp = 0;
    unsigned long ret = 0;

    switch(cmd)
    {
        /*0:开始补电*/
        case CHG_CMD:
        {
            batt_fact_chg = chg_fact_enable_chg_supplement();

            ret = copy_to_user((void*)arg, &batt_fact_chg, sizeof(batt_fact_chg));

            if(0 != ret)
            {
                printk("CHG_HUAWEI_CHARGER: Cannot start fact_chg!\n");

                return -1;
            }

            break;
        }
        /*1:查询是否需要补电*/
        case CHG_INQUIRE_CMD:
        {
            batt_fact_chg = chg_tbat_status_get();

            ret = copy_to_user((void*)arg, &batt_fact_chg, sizeof(batt_fact_chg));

            if(0 != ret)
            {
                printk("CHG_HUAWEI_CHARGER: Cannot obtain fact_chg state!\n");

                return -1;
            }

            break;
        }
        /*2:查询电池电压实时值*/
        case INQUIRE_VOLTAGE_CMD:
        {
            batt_volt = (int32_t)chg_get_volt_from_adc(CHG_PARAMETER__BATTERY_VOLT);

            ret = copy_to_user((void*)arg, &batt_volt, sizeof(batt_volt));

            if(0 != ret)
            {
                printk("CHG_HUAWEI_CHARGER: Cannot obtain orig bat_volt!\n");

                return -1;
            }

            break;
        }
        /*3:查询校准后的电池电压*/
        case INQUIRE_CALIBRATION_VOLTAGE_CMD:
        {
            batt_volt = chg_get_batt_volt_value();

            ret = copy_to_user((void*)arg, &batt_volt, sizeof(batt_volt));

            if(0 != ret)
            {
                printk("CHG_HUAWEI_CHARGER: Cannot obtain bat_volt!\n");

                return -1;
            }

            break;
        }
         /*4:查询当前是否充电*/
        case UI_INQUIRE_BATT_STATE:
        {
            if(TRUE == chg_get_charging_status())
            {
                batt_state = TRUE;
            }
            else
            {
                batt_state = FALSE;
            }

            ret = copy_to_user((void*)arg, &batt_state, sizeof(batt_state));
            if(0 != ret)
            {
                printk("CHG_HUAWEI_CHARGER: Cannot obtain batt_state!\n");

                return -1;
            }

            break;
        }
         /*5:查询充电器类型*/
        case UI_INQUIRE_CHARGER_TYPE:
        {
            cur_chgr_type = chg_stm_get_chgr_type();

            ret = copy_to_user((void*)arg, &cur_chgr_type, sizeof(cur_chgr_type));
            if(0 != ret)
            {
                printk("CHG_HUAWEI_CHARGER: Cannot obtain cur_chgr_type!\n");

                return -1;
            }

            break;
        }

        /*6:PT工位使能充电*/
        case INQUIRE_START_CHG_TASK_CMD:
        {
            batt_fact_chg = chg_pt_mmi_test_proc();
            //set 0 as success flag
            /*batt_fact_chg = 0;*/
            ret = copy_to_user((void*)arg, &batt_fact_chg, sizeof(batt_fact_chg));

            if(0 != ret)
            {
                printk("CHG_HUAWEI_CHARGER: Cannot start chg task!\n");

                return -1;
            }

            break;
        }

#if (MBB_CHG_EXTCHG == FEATURE_ON)
        /* 7:禁止本次对外充电*/
        case UI_DIABLE_EXTCHG_ONCE:
        {
            printk("CHG_HUAWEI_CHARGER: UI_DIABLE_EXTCHG_ONCE!\n");

            //chg_set_extchg_chg_enable(FALSE);
            /*禁止对外充电输出*/
            chg_set_charge_otg_enable(FALSE);
            /*EN1=1, EN2=0 防止对外充电设置停止本次充电后放到无线充电器上拔出USB ID线后无
                 硬件中断*/
#if (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
            extchg_gpio_control(EXTCHG_CONTROL_GPIO_EN1,TRUE); /* */
            extchg_gpio_control(EXTCHG_CONTROL_GPIO_EN2,FALSE); /* */
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/
            chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
            chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
            break;
        }
        /*8:开启本次对外充电，若过温，则不开启。*/
        case UI_ENABLE_EXTCHG_ONCE:
        {
            if (FALSE == chg_is_extchg_overtemp())
            {
                printk("CHG_HUAWEI_CHARGER: UI_ENABLE_EXTCHG!\n");
                chg_set_extchg_chg_enable(TRUE);
                chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_START_CHARGING;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
            }
            else
            {
                pr_info("CHG_HUAWEI_CHARGER: UI_ENABLE_EXTCHG, WHILE STOPPED DUE TO OTP!\n");
                chg_set_extchg_chg_enable(FALSE);
                chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_OVERHEAT_STOP_CHARGING;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
            }

            ret = copy_from_user(&g_ui_choose_exchg_mode, (void __user *)arg, sizeof(int32_t));
            printk("CHG_HUAWEI_CHARGER: g_ui_choose_exchg_mode=%d\n",g_ui_choose_exchg_mode);
            if(0 != ret)
            {
                printk("CHG_HUAWEI_CHARGER: Cannot obtain g_ui_choose_exchg_mode!\n");
                return -1;
            }
            break;
        }
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/

        /*9:查询是否支持库仑计*/
        case INQUIRE_COULOMETER_SUPPORT_CMD:
        {
            batt_coulometer_support = chg_get_coulometer_support();

            ret = copy_to_user((void*)arg, &batt_coulometer_support, sizeof(batt_coulometer_support));

            if(0 != ret)
            {
                printk("CHG_HUAWEI_CHARGER: Cannot obtain batt_coulometer_support!\n");

                return -1;
            }

            break;
        }
        /*10:查询电池温度*/
        case INQUIRE_BATT_TEMP_CMD:
        {
            batt_sys_temp = chg_get_sys_batt_temp();

            ret = copy_to_user((void*)arg, &batt_sys_temp, sizeof(batt_sys_temp));

            if(0 != ret)
            {
                printk("CHG_HUAWEI_CHARGER: Cannot obtain batt_sys_temp!\n");

                return -1;
            }

            break;
        }
        default:
            break;
    }
    return 0;
}
static const struct file_operations g_chargerdev_fops =
{
        .owner          = THIS_MODULE,
        .read           = chargerdev_read,
        .write          = chargerdev_write,
        .unlocked_ioctl = chargerdev_ioctl,
};

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
struct i2c_board_info  i2c_charger_info __initdata =
{
    .type = "bq24192",
    .addr = I2C_CHARGER_IC_ADDR,
};

static int __init i2c_charger_init(void)
{
    return i2c_register_board_info(0,&i2c_charger_info,1);
}
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/


static int chargeIC_probe(struct i2c_client *client,
                const struct i2c_device_id *id)
{
    struct chargeIC_chip *chip;
    struct device_node *node = client->dev.of_node;
    int devreg_rev;
    uint8_t dev_id = 0;
    int32_t ret = 0;
    int32_t charger_major = 0;

    dev_info(&client->dev, "CHG_HUAWEI_CHARGER:%s: Begin:***chargeIC_probe***\n", __func__);

    if (!node) {
        dev_err(&client->dev, "CHG_HUAWEI_CHARGER:%s: device tree information missing\n", __func__);
        //return - ENODEV;
    }
    //dev_info(&client->dev.of_node, "%s: print the dec node info!\n", __func__);

    if (!i2c_check_functionality(client->adapter,
            I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_err(&client->dev, "CHG_HUAWEI_CHARGER:%s: SMBUS_BYTE_DATA unsupported\n", __func__);
        return - EIO;
    }

    chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
    if (!chip) {
        dev_err(&client->dev, "CHG_HUAWEI_CHARGER:%s: devm_kzalloc failed\n", __func__);
        return - ENOMEM;
    }

    chip->client = client;

    chip->bat.name = "battery";
    chip->bat.type = POWER_SUPPLY_TYPE_BATTERY;
    chip->bat.properties = battery_monitor_props;
    chip->bat.num_properties = ARRAY_SIZE(battery_monitor_props);
    chip->bat.get_property = battery_monitor_get_property;
    chip->bat_health = POWER_SUPPLY_HEALTH_GOOD;
    chip->bat_avg_temp = 0;
    chip->bat_present = 0; //ready to add

    chip->usb.name = "usb_test";
    chip->usb.type = POWER_SUPPLY_TYPE_USB;
    chip->usb.properties = battery_monitor_usb_props;
    chip->usb.num_properties = ARRAY_SIZE(battery_monitor_usb_props);
    chip->usb.get_property = battery_monitor_usb_get_property;
    chip->usb_online = OFFLINE;
    /*USB温保*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    chip->usb_health = POWER_SUPPLY_USB_TEMP_GOOD;
#endif

    chip->ac.name = "ac";
    chip->ac.type = POWER_SUPPLY_TYPE_MAINS;
    chip->ac.properties = battery_monitor_ac_props;
    chip->ac.num_properties = ARRAY_SIZE(battery_monitor_ac_props);
    chip->ac.get_property = battery_monitor_ac_get_property;
    chip->ac_online = OFFLINE;

    chip->extchg.name = "extchg";
    chip->extchg.type = POWER_SUPPLY_TYPE_EXTCHG;
    chip->extchg.properties = battery_monitor_extchg_props;
    chip->extchg.num_properties = ARRAY_SIZE(battery_monitor_extchg_props);
    chip->extchg.get_property = battery_monitor_extchg_get_property;

    chip->wireless.name = "wireless";
    chip->wireless.type = POWER_SUPPLY_TYPE_WIRELESS;
    chip->wireless.properties = battery_monitor_wireless_props;
    chip->wireless.num_properties = ARRAY_SIZE(battery_monitor_wireless_props);
    chip->wireless.get_property = battery_monitor_wireless_get_property;

    /* 防止给应用上报事件过程中休眠的超时锁 */
    wake_lock_init(&(chip->alarm_wake_lock), WAKE_LOCK_SUSPEND, "MBB_CHG_ALARM");
    /* 任务处理过程中防止休眠的永久锁 */
    wake_lock_init(&(chip->chg_wake_lock), WAKE_LOCK_SUSPEND, "MBB_CHG_WORK");
     /* USB温保温度异常中防止休眠的永久锁 */
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    wake_lock_init((&chg_usb_temp_wake_lock), WAKE_LOCK_SUSPEND, "MBB_CHG_USB_PROTECT");
#endif
    i2c_set_clientdata(client, chip);

    ret = power_supply_register(&client->dev, &chip->bat);
    if (0 != ret)
    {
        dev_err(&client->dev, "CHG_HUAWEI_CHARGER:failed to register main battery\n");
        goto batt_failed;
    }

    ret = power_supply_register(&client->dev, &chip->usb);
    if (0 != ret)
    {
        dev_err(&client->dev, "CHG_HUAWEI_CHARGER:failed to register usb power supply\n");
        goto usb_failed;
    }

    ret = power_supply_register(&client->dev, &chip->ac);
    if (0 != ret)
    {
        dev_err(&client->dev, "CHG_HUAWEI_CHARGER:failed to register ac power supply\n");
        goto ac_failed;
    }

    ret = power_supply_register(&client->dev, &chip->extchg);
    if (0 != ret)
    {
        dev_err(&client->dev, "CHG_HUAWEI_CHARGER:failed to register extchg power supply\n");
        goto extchg_failed;
    }

    ret = power_supply_register(&client->dev, &chip->wireless);
    if (0 != ret)
    {
        dev_err(&client->dev, "CHG_HUAWEI_CHARGER:failed to register wireless power supply\n");
        goto wireless_failed;
    }

    g_i2c_client = chip->client;
    g_chip = chip;
    create_chargeIC_proc_file();
    create_dload_chg_proc_file();
    create_poweron_chg_proc_file();

    printk("CHG_HUAWEI_CHARGER: %s: ***creat proc filesystem***\n", __func__);

    //registering dev file node
    charger_major = register_chrdev(0, "charger", &g_chargerdev_fops);
    if(charger_major < 0)
    {
        printk("CHG_HUAWEI_CHARGER:REGISTER CHARGE DEVICE FAILED %d!\n", charger_major);
        //do not quit this probe function
        //return -1;
    }
    else
    {
        g_charger_class = class_create(THIS_MODULE, "charger");
        device_create(g_charger_class, NULL,MKDEV(charger_major,0),"%s",
            "charger");
    }

    ret = chargeIC_i2c_read(DEV_ID_REG, &dev_id);
    if (0 != ret)
    {
        dev_err(&client->dev, "CHG_HUAWEI_CHARGER:failed to chargeIC_i2c_read\n");
        //do not quit this probe function
        //return ret;
    }

    devreg_rev = (dev_id & DEV_ID_DEV_REG_MASK)
            >> DEV_ID_DEV_REG_SHIFT;

    dev_info(&client->dev, "CHG_HUAWEI_CHARGER:%s: chargeIC probed successfully, devreg_rev=%d\n",
        __func__, devreg_rev);

    dev_info(&client->dev, "CHG_HUAWEI_CHARGER:%s: g_i2c_client->addr [0x%02X]\n", __func__, g_i2c_client->addr);

    /* 创建充电工作队列，用来处理各种插入拔出事件 */
    mbb_chg_wq = create_workqueue("MBB_CHG_WQ");
    if( NULL == mbb_chg_wq )
    {
        pr_err("[MBB CHG]kernel create MBB_CHG_WQ workqueue failed.\r\n");
        return -EIO;
    }

    INIT_WORK(&(mbb_chg_event_work.event_work), charge_event_work);
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    INIT_DELAYED_WORK(&chip->extchg_monitor_work,chg_extchg_monitor_work);
    schedule_delayed_work(&chip->extchg_monitor_work, msecs_to_jiffies(0));
#endif
#ifdef CONFIG_MBB_FAST_ON_OFF
    ret = blocking_notifier_chain_register(&g_fast_on_off_notifier_list,
        &suspend_monitor_charger_block);
    if ( ret < 0 )
    {
        pr_err("CHG_HUAWEI_CHARGER:suspend_monitor_charger_block register g_fast_on_off_notifier fail \n");
    }
#endif/*CONFIG_MBB_FAST_ON_OFF*/

#if (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
    //wake_lock_init(&chip->alarm_wake_lock, WAKE_LOCK_SUSPEND, "charge_alarm");
    if ((TRUE == rtc_alarm_use_flag) && (NULL != g_alarm_rtc_dev))
    {
        chip->rtc = g_alarm_rtc_dev;
        INIT_WORK(&chip->alarm_work, alarm_work_func);
        g_init_flag = TRUE;
        start_charge_alarm(TRUE, CHARGE_ALARM_TIME_IN_SEC, 0);
        pr_err("CHG_HUAWEI_CHARGER:*** Enable charge alarm firstly! ***\n");
    }
    else if(TRUE == rtc_alarm_use_flag)
    {
        pr_err("CHG_HUAWEI_CHARGER:g_alarm_rtc_dev is NULL, do not set charge alarm!\n");
    }
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/

    return 0;

wireless_failed:
    power_supply_unregister(&chip->extchg);
extchg_failed:
    power_supply_unregister(&chip->ac);
ac_failed:
    power_supply_unregister(&chip->usb);
usb_failed:
    power_supply_unregister(&chip->bat);
batt_failed:
    i2c_set_clientdata(client, NULL);

    kfree(chip);
    chip = NULL;

    return ret;
}

static int chargeIC_remove(struct i2c_client *client)
{

    wake_lock_destroy(&g_chip->alarm_wake_lock);
    wake_lock_destroy(&g_chip->chg_wake_lock);
    return 0;
}
static int i2c_charger_suspend(struct device* dev)
{
    dev_err(dev, "CHG_HUAWEI_CHARGER:I2C charger PM suspend!\n");
#if (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
    g_i2c_charger_suspend_flag = TRUE;
    pr_err("CHG_HUAWEI_CHARGER:%s,g_i2c_charger_suspend_flag=%d\r\n",
            __func__,g_i2c_charger_suspend_flag);
#endif
    return 0;
}

static int i2c_charger_resume(struct device* dev)
{
    dev_err(dev, "CHG_HUAWEI_CHARGER:I2C charger PM resume!\n");
#if (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
    g_i2c_charger_suspend_flag = FALSE;
    pr_err("CHG_HUAWEI_CHARGER:%s,g_i2c_charger_suspend_flag=%d\r\n",
        __func__,g_i2c_charger_suspend_flag);
#endif
    return 0;
}

static const struct dev_pm_ops i2c_charger_pm_ops =
{
    .suspend = i2c_charger_suspend,
    .resume  = i2c_charger_resume,
};


static const struct i2c_device_id chargeIC_id[] = {
    { .name = CHARGE_IC_DRVNAME, },
    {},
};

static const struct of_device_id chargeIC_match[] = {
    { .compatible = DTS_COMPATIBLE_NAME, },
    { },
};
static struct i2c_driver chargeIC_driver = {
    .driver    = {
        .name           = CHARGE_IC_DRVNAME,
        .owner          = THIS_MODULE,
        .of_match_table = chargeIC_match,
        .pm             = &i2c_charger_pm_ops,
    },
    .probe          = chargeIC_probe,
    .remove         = chargeIC_remove,
    .id_table       = chargeIC_id,
};

static int __init chargeIC_init(void)
{
    return i2c_add_driver(&chargeIC_driver);
}
module_init(chargeIC_init);

static void __exit chargeIC_exit(void)
{
    return i2c_del_driver(&chargeIC_driver);
}
module_exit(chargeIC_exit);
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
postcore_initcall(i2c_charger_init);
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/
MODULE_DESCRIPTION(CHARGE_IC_DRVNAME "Charger");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("i2c:" CHARGE_IC_DRVNAME);

