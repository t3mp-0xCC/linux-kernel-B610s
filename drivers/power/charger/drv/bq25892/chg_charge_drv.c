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

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
extern void chg_delay_ms(uint32_t delay_time_ms);
extern int32_t bsp_i2c_read(uint8_t reg);
extern int32_t bsp_i2c_write(uint8_t reg, int32_t value);
/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
int32_t bq25892_i2c_read(uint8_t reg, uint8_t *pval);
int32_t bq25892_i2c_write(uint8_t reg, uint8_t val);
/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

#define bq25892_MAX_REGISTER_CNT            (21)

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
#define CHARGER_IC_ENABLE_GPIO               CHG_ENABLE_GPIO
#else
#define CHARGER_IC_ENABLE_GPIO               (31)
#endif


/*����ʼ��Ĭ�ϲ���*/
#define CHG_DEFAULT_TERM_CURRENT            (200)
#define CHG_DEFAULT_VREG                    (4208)
#define CHG_INPUT_LIMIT_500MA               (500)

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
const charger_current_limit_st charger_current_limit_paras[CHG_CHGR_INVALID] =
{
    {
        CHG_WALL_CHGR,       /*���*/
        896,                 /*mA,Charge Current limit*/
        2000,                /*mA,USB Current limit*/
    },
    {
        CHG_USB_HOST_PC,    /*USB2.0/USB3.0*/
        576,                /*mA,Charge Current limit*/
        500,                /*mA,USB Current limit*/
    },
    {
        CHG_WIRELESS_CHGR,    /*�Ǳ�*/
        576,                /*mA,Charge Current limit*/
        500,                /*mA,USB Current limit*/
    },
    {
        CHG_HVDCP_CHGR,     /*��ѹ�����*/
        576,
        2000,                /*mA,USB Current limit*/
    },
};
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/

const chg_hw_param_t chg_std_1A_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4208,  /*mV, CV Voltage setting.*/
        128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};
const chg_hw_param_t chg_std_2A_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        1536,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4352,  /*mV, CV Voltage setting.*/
        128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

const chg_hw_param_t chg_5v_hvdcp_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
        1000,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         200,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
           0,  /*mA, Power Supply front-end Current limit.*/
           0,  /*mA, Charge Current limit.*/
           0,  /*mV, CV Voltage setting.*/
           0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
        1000,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         200,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
        1000,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         200,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
        1000,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         200,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
        1000,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         200,  /*mA, Taper(Terminate) current.*/
        TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
        1600,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         200,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

const chg_hw_param_t chg_9v_hvdcp_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
        1000,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         200,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
           0,  /*mA, Power Supply front-end Current limit.*/
           0,  /*mA, Charge Current limit.*/
           0,  /*mV, CV Voltage setting.*/
           0,  /*mA, Taper(Terminate) current.*/
       FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
        1000,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         200,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
        1000,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         200,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
        1000,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         200,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
        1000,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         200,  /*mA, Taper(Terminate) current.*/
        TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
     /*6:-hvdcp charge State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
        2200,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         200,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};
const chg_hw_param_t chg_usb_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

/*usb3.0��������*/
const chg_hw_param_t chg_usb3_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

const chg_hw_param_t chg_weak_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

#if (MBB_CHG_WIRELESS == FEATURE_ON)
const chg_hw_param_t chg_wireless_chgr_hw_paras_def[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */
    /*6:-hvdcp charge State*/
    {
            0,  /*mA, Power Supply front-end Current limit.*/
            0,  /*mA, Charge Current limit.*/
            0,  /*mV, CV Voltage setting.*/
            0,  /*mA, Taper(Terminate) current.*/
        FALSE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
};

/*�ػ����߳�����������*/
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
const chg_hw_param_t chg_wireless_poweroff_warmchg_paras =
{
     900,  /*mA, Power Supply front-end Current limit.*/
    1024,  /*mA, Charge Current limit.*/
    4100,  /*mV, CV Voltage setting.*/
     128,  /*mA, Taper(Terminate) current.*/
    TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
};
#endif /* MBB_CHG_WARM_CHARGE == FEATURE_ON */

#endif /* MBB_CHG_WIRELESS == FEATURE_ON */
/*BQ25892 Charge IC support.*/
static chg_chip_part_nr g_chg_chip_pn = CHG_CHIP_PN_INVALID;

/******************************************************************************
  Function      bq25892_i2c_read
  Description   ��ȡI2C�豸ָ���Ĵ�����ַ��ֵ
  Input         reg   :�Ĵ�����ַ
  Output        *pval :���ڽ��Ĵ�����ֵд��������������ڵ�ַ
  Return        0     :����ִ�гɹ�
                -1    :����ִ��ʧ��
  Others        N/A
******************************************************************************/
int32_t bq25892_i2c_read(uint8_t reg, uint8_t *pval)
{
    int32_t ret = 0;
    uint8_t chg_bq_reg = 0;

    chg_bq_reg = reg;
    chg_delay_ms(1);
    ret = bsp_i2c_read((uint8_t)chg_bq_reg);
    if (ret < 0)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: Failed due to I2c read error ret %d\n", ret);
        return -1;
    }
    else
    {
        *pval = (uint8_t)ret;
        return 0;
    }
}

/******************************************************************************
  Function      bq25892_i2c_write
  Description   ��I2C�豸�Ĵ���ָ����ַд����ֵ
  Input         reg  : �Ĵ�����ַ
                val  : ϣ��д�������Ĵ�����ַ��ֵ
  Output        N/A
  Return        0    : ����ִ�гɹ�
                -1   : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
int32_t bq25892_i2c_write(uint8_t reg, uint8_t val)
{
    int32_t ret = 0;
    uint8_t chg_bq_reg = 0;

    chg_bq_reg = reg;
    chg_delay_ms(1);
    ret = bsp_i2c_write((uint8_t)chg_bq_reg, (int32_t)val);
    if (ret < 0)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: Failed due to I2c read error ret %d\n", ret);
        return -1;
    }
    else
    {
        return 0;
    }
}

/******************************************************************************
  Function      chg_charger_event_api
  Description   Ϊ��Դ����\�γ��жϼ��ʹ�õ�api����
  Input         charger_status
                    TRUE:  ����
                    FALSE:  �γ�
  Output        N/A
  Return        N/A
  Others        N/A
******************************************************************************/
void chg_charger_event_api(boolean charger_status)
{
    return;
}

/******************************************************************************
Function:       chg_set_cur_level
Description:    �趨���оƬ�Ե�صĳ�����
Input:          ������ֵ
Output:         N/A
Return:         TRUE     : ����ִ�гɹ�
                FALSE    : ����ִ��ʧ��
Others:         N/A
******************************************************************************/
boolean chg_set_cur_level(uint32_t chg_cur)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret      = 0;
    int32_t cur_temp = 0;

    if ((BQ25892_ICHARGE_MAX < chg_cur)
        || (BQ25892_ICHARGE_OFFSET > chg_cur))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: chg_set_cur_level Para invalid!\n");
        return FALSE;
    }

    reg = BQ25892_CHG_CUR_CTL_REG04;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING BQ25892_CHG_CUR_CTL_REG04!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: CHG_DRV_DBG:chg_set_cur_level() reg %d, value 0x%x before!\n", reg, reg_val);

    reg_val &= ~(BQ25892_ICHARGE_MASK);

    cur_temp = chg_cur - BQ25892_ICHARGE_OFFSET;
    if (CHG_CURRENT_4096MA_B8 <= cur_temp)
    {
        reg_val |= BQ25892_ICHARGE_4096;
        cur_temp = cur_temp - CHG_CURRENT_4096MA_B8;
    }

    if (CHG_CURRENT_2048MA_B7 <= cur_temp)
    {
        reg_val |= BQ25892_ICHARGE_2048;
        cur_temp = cur_temp - CHG_CURRENT_2048MA_B7;
    }

    if (CHG_CURRENT_1024MA_B6 <= cur_temp)
    {
        reg_val |= BQ25892_ICHARGE_1024;
        cur_temp = cur_temp - CHG_CURRENT_1024MA_B6;
    }

    if (CHG_CURRENT_512MA_B5 <= cur_temp)
    {
        reg_val |= BQ25892_ICHARGE_512;
        cur_temp = cur_temp - CHG_CURRENT_512MA_B5;
    }

    if (CHG_CURRENT_256MA_B4 <= cur_temp)
    {
        reg_val |= BQ25892_ICHARGE_256;
        cur_temp = cur_temp - CHG_CURRENT_256MA_B4;
    }

    if (CHG_CURRENT_128MA_B3 <= cur_temp)
    {
        reg_val |= BQ25892_ICHARGE_128;
        cur_temp = cur_temp - CHG_CURRENT_128MA_B3;
    }

    if (CHG_CURRENT_64MA_B2 <= cur_temp)
    {
        reg_val |= BQ25892_ICHARGE_64;
    }

    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_cur_level() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR SETTING CHARGE CURRENT!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/******************************************************************************
Function:       chg_get_supply_limit
Description:    ��ȡ���оƬǰ������
Input:          ������ֵ
Output:         N/A
Return:         TRUE     : ����ִ�гɹ�
                FALSE    : ����ִ��ʧ��
Others:         N/A
******************************************************************************/
uint8_t chg_get_supply_limit(void)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    reg = BQ25892_INPUT_CTL_REG00;
    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING !!\n");
        return -1;
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: reg_val=0x%x \n", reg_val);

    return (reg_val & BQ25892_IINLIMIT_MASK);
}

/******************************************************************************
  Function      chg_set_supply_limit
  Description   �趨�����������
  Input         �������ֵ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_supply_limit(uint32_t lmt_val)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;
    boolean op_result = TRUE;

    if ((CHG_IINPUT_LIMIT_3250MA < lmt_val)
        || (CHG_IINPUT_LIMIT_100MA > lmt_val))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: chg_set_supply_limit Para invalid!\n");
        return FALSE;
    }

    reg = BQ25892_INPUT_CTL_REG00;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING BQ25892_INPUT_CTL_REG00!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
     chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_supply_limit() reg %d, value 0x%x before!\n", reg, reg_val);

    /*����Ĵ���ԭʼֵ*/
    reg_val &= ~(BQ25892_IINLIMIT_MASK);

    /*�����������ֵ�ԼĴ�����������*/
    switch (lmt_val)
    {
        case CHG_IINPUT_LIMIT_3250MA:
        {
            reg_val |= BQ25892_IINLIMIT_3250;

            break;
        }
        case CHG_IINPUT_LIMIT_3000MA:
        {
            reg_val |= BQ25892_IINLIMIT_3000;

            break;
        }
        case CHG_IINPUT_LIMIT_2000MA:
        {
            reg_val |= BQ25892_IINLIMIT_2000;

            break;
        }
        case CHG_IINPUT_LIMIT_1500MA:
        {
            reg_val |= BQ25892_IINLIMIT_1500;

            break;
        }
        case CHG_IINPUT_LIMIT_1200MA:
        {
            reg_val |= BQ25892_IINLIMIT_1200;

            break;
        }

        case CHG_IINPUT_LIMIT_1000MA:
        {
            reg_val |= BQ25892_IINLIMIT_1000;

            break;
        }

        case CHG_IINPUT_LIMIT_900MA:
        {
            reg_val |= BQ25892_IINLIMIT_900;

            break;
        }
        case CHG_IINPUT_LIMIT_500MA:
        {
            reg_val |= BQ25892_IINLIMIT_500;

            break;
        }
        case CHG_IINPUT_LIMIT_150MA:
        {
            reg_val |= BQ25892_IINLIMIT_150;

            break;
        }
        default:
        {
            reg_val |= BQ25892_IINLIMIT_100;

            break;
        }

    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_supply_limit() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_DRV: ERROR SETTING CHARGING SUPPLY CURRENT LIMIT!\n");
        return FALSE;
    }
    else
    {
        return op_result;
    }
}

/******************************************************************************
  Function      chg_set_term_current
  Description   ���ó���ֹ����
  Input         ��ֹ����ֵ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_term_current(uint32_t term_val)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;
    int32_t term_temp = 0;

    if ((BQ25892_ITERM_MAX < term_val)
        || (BQ25892_ITERM_OFFSET > term_val))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: chg_set_term_current Para invalid!\n");
        return FALSE;
    }

    reg = BQ25892_IPRECHG_TERM_CTL_REG05;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_set_term_current!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_term_current() reg %d, value 0x%x before!\n", reg, reg_val);

    reg_val &= ~(BQ25892_ITERM_MASK);

    term_temp = term_val - BQ25892_ITERM_OFFSET;
    if (CHG_TERM_CURRENT_512MA <= term_temp)
    {
        reg_val |= BQ25892_ITERM_512;
        term_temp = term_temp - CHG_TERM_CURRENT_512MA;
    }

    if (CHG_TERM_CURRENT_256MA <= term_temp)
    {
        reg_val |= BQ25892_ITERM_256;
        term_temp = term_temp - CHG_TERM_CURRENT_256MA;
    }

    if (CHG_TERM_CURRENT_128MA <= term_temp)
    {
        reg_val |= BQ25892_ITERM_128;
        term_temp = term_temp - CHG_TERM_CURRENT_128MA;
    }

    if (CHG_TERM_CURRENT_64MA <= term_temp)
    {
        reg_val |= BQ25892_ITERM_64;
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_term_current() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR SETTING CHARGING TERM CURRENT!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }

}

/******************************************************************************
  Function      chg_set_charge_enable
  Description   �趨�Ƿ�ʹ�ܳ��
  Input         en_val:TRUE   ����ʹ�ܳ��
                       FALSE  ������ֹ���
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_charge_enable(boolean enable)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    reg = BQ25892_POWER_ON_CFG_REG03;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_set_charge_enable!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_charge_enable() reg %d, value 0x%x before!\n", reg, reg_val);

    reg_val &= ~(BQ25892_CHG_CFG_MASK);

    if (TRUE == enable)
    {
        reg_val |= BQ25892_CHG_CFG_EN;
    }
    else
    {
        reg_val |= BQ25892_CHG_CFG_DIS;
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_charge_enable() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR SETTING CHARGING ENABLE VALUE!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/******************************************************************************
  Function      chg_set_vreg_level
  Description   ���ú�ѹ����ѹ
  Input         ����ѹ��ѹֵ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_vreg_level(uint32_t vreg_val)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;
    int32_t vreg_temp = 0;

    if ((BQ25892_BAT_REG_VOL_MAX < vreg_val) || (BQ25892_BAT_REG_VOL_OFFSET > vreg_val))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: chg_set_vreg_level Para invalid!\n");
        return FALSE;
    }

    reg = BQ25892_CHG_VOLT_CTL_REG06;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING BQ25892_CHG_VOLT_CTL_REG!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_vreg_level() reg %d, value 0x%x before!\n", reg, reg_val);

    /*��reg_val�����õ�ص�ѹ�ı���λ����*/
    reg_val &= ~(BQ25892_BAT_REG_VOL_MASK);

    vreg_temp = vreg_val - BQ25892_BAT_REG_VOL_OFFSET;
    if (CHG_BAT_VREG_512MV_B7 <= vreg_temp)
    {
        reg_val |= BQ25892_BAT_REG_VOL_512;
        vreg_temp = vreg_temp - CHG_BAT_VREG_512MV_B7;
    }

    if (CHG_BAT_VREG_256MV_B6 <= vreg_temp)
    {
        reg_val |= BQ25892_BAT_REG_VOL_256;
        vreg_temp = vreg_temp - CHG_BAT_VREG_256MV_B6;
    }

    if (CHG_BAT_VREG_128MV_B5 <= vreg_temp)
    {
        reg_val |= BQ25892_BAT_REG_VOL_128;
        vreg_temp = vreg_temp - CHG_BAT_VREG_128MV_B5;
    }

    if (CHG_BAT_VREG_64MV_B4 <= vreg_temp)
    {
        reg_val |= BQ25892_BAT_REG_VOL_64;
        vreg_temp = vreg_temp - CHG_BAT_VREG_64MV_B4;
    }

    if (CHG_BAT_VREG_32MV_B3 <= vreg_temp)
    {
        reg_val |= BQ25892_BAT_REG_VOL_32;
        vreg_temp = vreg_temp - CHG_BAT_VREG_32MV_B3;
    }

    if (CHG_BAT_VREG_16MV_B2 <= vreg_temp)
    {
        reg_val |= BQ25892_BAT_REG_VOL_16;
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_vreg_level() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR SETTING VREG LEVEL!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/******************************************************************************
  Function      chg_set_suspend_mode
  Description   ���ó��ICΪsuspendģʽ
  Input         hz_mode: TRUE   �����趨���оƬ����suspendģʽ
                         FALSE  �����趨оƬ�˳�suspendģʽ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_suspend_mode(boolean enable)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    reg = BQ25892_INPUT_CTL_REG00;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_set_suspend_mode!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_suspend_mode() reg %d, value 0x%x before!\n", reg, reg_val);

    /*���HZ����λ*/
    reg_val &= ~(BQ25892_EN_HIZ_MASK);

    if (TRUE == enable)
    {
        reg_val |= BQ25892_EN_HIZ;
    }
    else
    {
        reg_val |= BQ25892_DIS_HIZ;
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_suspend_mode() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR SETTING HZ_MODE!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/******************************************************************************
  Function      chg_set_supply_prefer
  Description   �趨���оƬ����˵����ȼ�
  Input         chg_cur: CHG_SUPPLY_PREFER_IN   �����������ȼ�ΪIN����
                         CHG_SUPPLY_PREFER_USB  �����������ȼ�ΪUSB����
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_supply_prefer(chg_supply_prefer_value sup_val)
{
    /*BQ25892 has no this function.*/
    return TRUE;
}

/******************************************************************************
  Function      chg_set_te_enable
  Description   �趨�Ƿ�ʹ�ܽ�ֹ����
  Input         chg_cur: TRUE   ����ʹ�ܳ���ֹ�������������ﵽ�趨��
                                ��ֵֹ����ͻ�ֹͣ
                         FALSE  ������ֹ����ֹ����
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_te_enable(boolean enable)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    reg = BQ25892_TERM_TIMER_CTL_REG07;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_set_te_enable!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_te_enable() reg %d, value 0x%x before!\n", reg, reg_val);

    reg_val &= ~(BQ25892_TERM_CUR_MASK);

    if(TRUE == enable)
    {
         reg_val |= BQ25892_EN_TERM_CUR;
    }
    else
    {
         reg_val |= BQ25892_FBD_TERM_CUR;
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_te_enable() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR SETTING CHARGING TE VALUE!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/******************************************************************************
  Function      chg_set_ts_enable
  Description   �趨оƬ�¶ȱ�������ʹ��
  Input         chg_cur: TRUE   ʹ��оƬ�Դ����¶ȱ�������
                         FALSE  ����оƬ�Դ����¶ȱ�������
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_ts_enable(boolean enable)
{
    /*BQ24192 has no this function.*/
    return TRUE;
}

/******************************************************************************
  Function      chg_set_charge_mode
  Description   �趨оƬ�Ƿ����������
  Input         chg_cur: TRUE   ʹ��LOW_CHG���ܣ�оƬ��100mAС������
                         ��ؽ��г��
                         FALSE  ����LOW_CHG���ܣ�оƬ��0x05�Ĵ����趨
                         �ĳ������Ե�ؽ��г��
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_charge_mode(boolean enable)
{
    /*BQ24192 has no this function.*/
    return TRUE;
}

/******************************************************************************
Function:       chg_set_prechg_cur_level
Description:    �趨���оƬ�Ե�ص�Ԥ������
Input:          Ԥ������ֵ
Output:         N/A
Return:         TRUE     : ����ִ�гɹ�
                FALSE    : ����ִ��ʧ��
Others:         BQ24161����
******************************************************************************/
boolean chg_set_prechg_cur_level(uint32_t chg_cur)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;
    int32_t term_temp = 0;

    if ((BQ25892_IPRECHG_MAX < chg_cur)
        || (BQ25892_IPRECHG_OFFSET > chg_cur))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: chg_set_prechg_cur_level Para invalid!\n");
        return FALSE;
    }

    reg = BQ25892_IPRECHG_TERM_CTL_REG05;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_set_prechg_cur_level!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_prechg_cur_level() reg %d, value 0x%x before!\n", reg, reg_val);

    reg_val &= ~(BQ25892_IPRECHG_MASK);

    term_temp = chg_cur - BQ25892_IPRECHG_OFFSET;
    if (CHG_PRECHG_CURRENT_512MA <= term_temp)
    {
        reg_val |= BQ25892_IPRECHG_512;
        term_temp = term_temp - CHG_PRECHG_CURRENT_512MA;
    }

    if (CHG_PRECHG_CURRENT_256MA <= term_temp)
    {
        reg_val |= BQ25892_IPRECHG_256;
        term_temp = term_temp - CHG_PRECHG_CURRENT_256MA;
    }

    if (CHG_PRECHG_CURRENT_128MA <= term_temp)
    {
        reg_val |= BQ25892_IPRECHG_128;
        term_temp = term_temp - CHG_PRECHG_CURRENT_128MA;
    }

    if (CHG_PRECHG_CURRENT_64MA <= term_temp)
    {
        reg_val |= BQ25892_IPRECHG_64;
    }

    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_prechg_cur_level() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR SETTING PRECHARGE CURRENT!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


/******************************************************************************
  Function      chg_set_stat_enable
  Description   �趨оƬ�Ƿ�ʹ��STAT����λ
  Input         chg_cur: TRUE   ʹ��STAT����λ
                         FALSE  ����STAT����λ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_stat_enable(boolean enable)
{
    /*BQ24192 has no this function.*/
    return TRUE;
}

/******************************************************************************
  Function      chg_set_dpm_val
  Description   �趨DPMֵ����������������ѹС�ڸ��趨ֵ֮��оƬ�����
                �����ѹ�Ľ�һ���½�(������������������������𻵳���豸)
                �Ӷ��𽥼�С����ֹͣ�Ե�ؽ��г�磬��ȫ���������ڸ�ϵͳ����
                ĿǰDPM�趨ֵΪ4.2V
  Input         N/A
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_dpm_val(uint32_t dpm_val)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;
    int32_t dpm_temp = 0;

    if ((BQ25892_DPM_VOL_MAX < dpm_val) || (BQ25892_DPM_VOL_OFFSET > dpm_val))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: Para invalid!\n");
        return FALSE;
    }

    reg = BQ25892_DPM_CTL_REG0D;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_set_dpm_val!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_dpm_val() reg %d, value 0x%x before!\n", reg, reg_val);

    reg_val &= ~(BQ25892_DPM_VOL_MASK);

    dpm_temp = dpm_val - BQ25892_DPM_VOL_OFFSET;
    if (CHG_VINDPM_6400MV_B6 <= dpm_temp)
    {
        reg_val |= BQ25892_DPM_VOL_6400MV;
        dpm_temp = dpm_temp - CHG_VINDPM_6400MV_B6;
    }

    if (CHG_VINDPM_3200MV_B5 <= dpm_temp)
    {
        reg_val |= BQ25892_DPM_VOL_3200MV;
        dpm_temp = dpm_temp - CHG_VINDPM_3200MV_B5;
    }

    if (CHG_VINDPM_1600MV_B4 <= dpm_temp)
    {
        reg_val |= BQ25892_DPM_VOL_1600MV;
        dpm_temp = dpm_temp - CHG_VINDPM_1600MV_B4;
    }

    if (CHG_VINDPM_800MV_B3 <= dpm_temp)
    {
        reg_val |= BQ25892_DPM_VOL_800MV;
        dpm_temp = dpm_temp - CHG_VINDPM_800MV_B3;
    }

    if (CHG_VINDPM_400MV_B2 <= dpm_temp)
    {
        reg_val |= BQ25892_DPM_VOL_400MV;
        dpm_temp = dpm_temp - CHG_VINDPM_400MV_B2;
    }

    if (CHG_VINDPM_200MV_B1 <= dpm_temp)
    {
        reg_val |= BQ25892_DPM_VOL_200MV;
        dpm_temp = dpm_temp - CHG_VINDPM_200MV_B1;
    }

    if (CHG_VINDPM_100MV_B0 <= dpm_temp)
    {
        reg_val |= BQ25892_DPM_VOL_100MV;
    }

    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_dpm_val() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR SETTING CHARGING DPM!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/******************************************************************************
  Function      chg_set_safety_timer_enable
  Description   �趨�Ƿ�ʹ�ܳ��оƬ�İ�ȫ��ʱ������
  Input         chg_cur: 00 (01,10) ʹ�ܰ�ȫ��ʱ����оƬĬ��ʹ�ܣ�����27min
                         �Կ��ٳ��ʱ���������(6h,9h)
                         11  ���ð�ȫ��ʱ��
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_safety_timer_enable(chg_safety_timer_value tmr_val)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    reg = BQ25892_TERM_TIMER_CTL_REG07;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_set_safety_timer_enable!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_safety_timer_enable() reg %d, value 0x%x before!\n", reg, reg_val);

    reg_val &= ~(BQ25892_CHG_TIMER_MASK);
    reg_val &= ~(BQ25892_CHG_TIMER_EN_MASK);

    switch (tmr_val)
    {
        case CHG_5HOUR_SAFETY_TIMER:
        {
            reg_val |= BQ25892_CHG_TIMER_5HRS;
            reg_val |= BQ25892_CHG_TIMER_EN;

            break;
        }
        case CHG_8HOUR_SAFETY_TIMER:
        {
            reg_val |= BQ25892_CHG_TIMER_8HRS;
            reg_val |= BQ25892_CHG_TIMER_EN;

            break;
        }
        case CHG_12HOUR_SAFETY_TIMER:
        {
            reg_val |= BQ25892_CHG_TIMER_12HRS;
            reg_val |= BQ25892_CHG_TIMER_EN;

            break;
        }
        case CHG_20HOUR_SAFETY_TIMER:
        {
            reg_val |= BQ25892_CHG_TIMER_20HRS;
            reg_val |= BQ25892_CHG_TIMER_EN;

            break;
        }
        default:
        {
            reg_val |= BQ25892_CHG_TIMER_DIS;

            break;
        }

    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_safety_timer_enable() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR SETTING CHARGING SAFETY TIMER!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/******************************************************************************
  Function      chg_set_tmr_setting
  Description   ���ó��оƬ�߹�ʱ��
  Input         N/A
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_tmr_setting(chg_watchdog_timer_value tmr_val)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    reg = BQ25892_TERM_TIMER_CTL_REG07;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_set_tmr_setting!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_tmr_setting() reg %d, value 0x%x before!\n", reg, reg_val);

    reg_val &= ~(BQ25892_WATCHDOG_TMR_MASK);

    switch (tmr_val)
    {
        case CHG_40S_WATCHDOG_TIMER:
        {
            reg_val |= BQ25892_WATCHDOG_TMR_40;

            break;
        }
        case CHG_80S_WATCHDOG_TIMER:
        {
            reg_val |= BQ25892_WATCHDOG_TMR_80;

            break;
        }
        case CHG_160S_WATCHDOG_TIMER:
        {
            reg_val |= BQ25892_WATCHDOG_TMR_160;

            break;
        }
        default:
        {
            reg_val |= BQ25892_WATCHDOG_TMR_DIS;

            break;
        }

    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_tmr_setting() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR SETTING CHARGING WATCHDOG TIMER!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }

}

/******************************************************************************
  Function      chg_set_tmr_rst
  Description   �Գ��оƬִ���߹�����
  Input         N/A
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_tmr_rst(void)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    reg = BQ25892_POWER_ON_CFG_REG03;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_set_tmr_rst!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_tmr_rst() reg %d, value 0x%x before!\n", reg, reg_val);

    reg_val |= BQ25892_TMR_RST;

    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_tmr_rst() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR KICKING WATCHDOG!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/******************************************************************************
  Function      chg_reset_all_reg
  Description   ��������bq25892�Ĵ�����Ĭ������
  Input         N/A
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_reset_all_reg(void)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    reg = BQ25892_REVISION_REG14;
    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_reset_all_reg!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_reset_all_reg() reg %d, value 0x%x before!\n", reg, reg_val);

    reg_val |= BQ25892_RESET_ALL;

    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_reset_all_reg() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR RESETTING ALL REGISTERS!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/******************************************************************************
  Function      chg_read_reg_dcdc
  Description   ��ȡ���оƬ���мĴ�����ֵ����������reg_val_dcdcȫ�ֱ�����
  Input         N/A
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
static boolean chg_read_reg_dcdc(uint8_t *reg_val_dcdc)
{
    uint8_t chg_bq_reg = 0;
    uint8_t reg_val = 0;
    int32_t ret = 0;

    if (NULL == reg_val_dcdc)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_read_reg_dcdc!\n");
        return FALSE;
    }

    memset(reg_val_dcdc, 0, (sizeof(uint8_t)*bq25892_MAX_REGISTER_CNT));

    ret = bq25892_i2c_read(0, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_read_reg_dcdc!\n");
        return FALSE;
    }
    else
    {
        //do nothing
    }

    chg_bq_reg = BQ25892_INPUT_CTL_REG00;
    for (; chg_bq_reg < bq25892_MAX_REGISTER_CNT; chg_bq_reg++)
    {
        (void)bq25892_i2c_read(chg_bq_reg, &reg_val);
        reg_val_dcdc[chg_bq_reg] = reg_val;
        /*��BQ25892�ļĴ���ֵ����ӡ��������������е����ⶨλ*/
        //chg_print_level_message(CHG_MSG_DEBUG, "CHG_DRV: chg_read_reg_dcdc() reg %d, value 0x%x!\n",
        //            chg_bq_reg, reg_val_dcdc[chg_bq_reg],0);
    }
    return TRUE;
}
/******************************************************************************
  Function      chg_get_suspend_status
  Description   ��ȡ���IC״̬���Ƿ���suspendģʽ
  Input         N/A
  Output
  Return        TRUE      : suspendģʽ
                FALSE     : ��suspendģʽ
  Others        N/A
******************************************************************************/
boolean chg_get_suspend_status(void)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    reg = BQ25892_INPUT_CTL_REG00;
    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING !!\n");
        return FALSE;
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: reg_val=0x%x \n", reg_val);

    return ((reg_val & BQ25892_EN_HIZ_MASK) >> BQ25892_EN_HIZ_BITPOS);
}

/******************************************************************************
  Function      chg_get_IC_status
  Description   ��ȡ���IC״̬����ȡ���оƬ���мĴ�����ֵ����ʶ�����е�ֻ��
                ����λ��д��chg_status_type�ṹ���У��ýṹ�����������г��оƬ
                ״̬
  Input         N/A
  Output
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_get_IC_status(chg_status_type *chg_stat_ptr)
{
    boolean ret = FALSE;
    uint8_t reg_val_dcdc_all[bq25892_MAX_REGISTER_CNT] = {0};
    if (NULL == chg_stat_ptr)
    {
        return FALSE;
    }

    ret = chg_read_reg_dcdc(reg_val_dcdc_all);
    if(FALSE == ret)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_DRV: ERROR READING chg_get_IC_status!\n");
        return FALSE;
    }
    else
    {
        //do nothing
    }
    chg_stat_ptr->chg_vbus_stat = (chg_dcdc_vbus_stat) \
    /* array offset */
        ((reg_val_dcdc_all[BQ25892_VBUS_STAT_REG0B] & BQ25892_VBUS_STAT_MASK) >> BQ25892_VBUS_STAT_BITPOS);

    chg_print_level_message(CHG_MSG_DEBUG,"CHG_DRV: chg_stat_ptr->chg_vbus_stat =%d !\n",
                            chg_stat_ptr->chg_vbus_stat);

    chg_stat_ptr->chg_chrg_stat = (chg_dcdc_chrg_stat) \
    /* array offset */
        ((reg_val_dcdc_all[BQ25892_VBUS_STAT_REG0B] & BQ25892_CHRG_STAT_MASK) >> BQ25892_CHRG_STAT_BITPOS);

    chg_print_level_message(CHG_MSG_DEBUG,"CHG_DRV: chg_stat_ptr->chg_chrg_stat =%d !\n",
                            chg_stat_ptr->chg_chrg_stat);

    chg_stat_ptr->chg_watchdog_fault = (chg_dcdc_watchdog_fault) \
    /* array offset */
        ((reg_val_dcdc_all[BQ25892_FAULT_REG0C] & BQ25892_WATCHDOG_FLT_MASK) >> BQ25892_WATCHDOG_FLT_BITPOS);

    chg_print_level_message(CHG_MSG_DEBUG,"CHG_DRV:chg_stat_ptr->chg_watchdog_fault=%d !\n",
                            chg_stat_ptr->chg_watchdog_fault);

    chg_stat_ptr->chg_chrg_fault = (chg_dcdc_chrg_fault) \
    /* array offset */
        ((reg_val_dcdc_all[BQ25892_FAULT_REG0C] & BQ25892_CHRG_FLT_MASK) >> BQ25892_CHRG_FLT_BITPOS);

    chg_print_level_message(CHG_MSG_DEBUG,"CHG_DRV: chg_stat_ptr->chg_chrg_fault=%d !\n",
                             chg_stat_ptr->chg_chrg_fault);

    chg_stat_ptr->chg_bat_fault = (chg_dcdc_bat_fault) \
    /* array offset */
        ((reg_val_dcdc_all[BQ25892_FAULT_REG0C] & BQ25892_BAT_FLT_MASK) >> BQ25892_BAT_FLT_BITPOS);

    chg_print_level_message(CHG_MSG_DEBUG,"CHG_DRV: chg_stat_ptr->chg_bat_fault=%d !\n",
                     chg_stat_ptr->chg_bat_fault);

    chg_stat_ptr->chg_ce_stat = (chg_dcdc_ce_status) \
    /* array offset */
        ((reg_val_dcdc_all[BQ25892_POWER_ON_CFG_REG03] & BQ25892_CHG_CFG_MASK) >> BQ25892_CHG_CFG_BITPOS);

    chg_print_level_message(CHG_MSG_DEBUG,"CHG_DRV: chg_stat_ptr->chg_ce_stat=%d !\n",
                     chg_stat_ptr->chg_ce_stat);
    return TRUE;
}

/******************************************************************************
  Function      chg_dump_ic_hwinfo
  Description   Dump���IC���е�Ӳ�������Ϣ�������Ĵ�������PIN��״̬��
  Input         N/A
  Output        N/A
  Return        N/A
  Others        N/A
******************************************************************************/
boolean chg_dump_ic_hwinfo(void)
{
    boolean ret = FALSE;
    uint8_t g_reg_val_dcdc[bq25892_MAX_REGISTER_CNT] = {0};

    ret = chg_read_reg_dcdc(g_reg_val_dcdc);

    if (TRUE == ret)
    {
        mlog_print(MLOG_CHG, mlog_lv_info, "Current Charge IC hardware info is:\n");
        /*Read the register 00-05 of bq24196.*/
        mlog_print(MLOG_CHG, mlog_lv_info, "[REG00]0x%x [REG01]0x%x [REG02]0x%x "
                                           "[REG03]0x%x [REG04]0x%x [REG05]0x%x.\n", 
                                           /*Register 0x00 - 0x01;*/
                                           g_reg_val_dcdc[0], g_reg_val_dcdc[1],
                                           /*Register 0x02 - 0x03;*/
                                           g_reg_val_dcdc[2], g_reg_val_dcdc[3],
                                           /*Register 0x04 - 0x05;*/
                                           g_reg_val_dcdc[4], g_reg_val_dcdc[5]);
        /*Read the register 06-0A of bq24196.*/
        mlog_print(MLOG_CHG, mlog_lv_info, "[REG06]0x%x [REG07]0x%x [REG08]0x%x "
                                           "[REG09]0x%x [REG0A]0x%x.\n", 
                                           /*Register 0x06 - 0x07;*/
                                           g_reg_val_dcdc[6], g_reg_val_dcdc[7],
                                           /*Register 0x08 - 0x09;*/
                                           g_reg_val_dcdc[8], g_reg_val_dcdc[9],
                                           /*Register 0x0a;*/
                                           g_reg_val_dcdc[10]);
        /*Read the register 0A-0F of bq24196.*/
        mlog_print(MLOG_CHG, mlog_lv_info, "[REG0B]0x%x [REG0B]0x%x [REG0D]0x%x "
                                           "[REG0E]0x%x [REG0F]0x%x.\n", 
                                           /*Register 0x06 - 0x07;*/
                                           g_reg_val_dcdc[11], g_reg_val_dcdc[12],
                                           /*Register 0x08 - 0x09;*/
                                           g_reg_val_dcdc[13], g_reg_val_dcdc[14],
                                           /*Register 0x0a;*/
                                           g_reg_val_dcdc[15]);
        /*Read the register 0A-0F of bq24196.*/
        mlog_print(MLOG_CHG, mlog_lv_info, "[REG10]0x%x [REG11]0x%x [REG12]0x%x "
                                           "[REG13]0x%x [REG14]0x%x.\n", 
                                           /*Register 0x06 - 0x07;*/
                                           g_reg_val_dcdc[16], g_reg_val_dcdc[17],
                                           /*Register 0x08 - 0x09;*/
                                           g_reg_val_dcdc[18], g_reg_val_dcdc[19],
                                           /*Register 0x0a;*/
                                           g_reg_val_dcdc[20]);
    }
    else
    {
        mlog_print(MLOG_CHG, mlog_lv_error, "Dump Charge IC hardware info failed.\n");
    }

    return ret;
}

/******************************************************************************
  Function      chg_is_charger_present
  Description   ���Դ�Ƿ���λ
  Input         N/A
  Output        N/A
  Return        FALSE     : ���Դ����λ
                TRUE      : ���Դ��λ
  Others        N/A
******************************************************************************/
boolean chg_is_charger_present(void)
{
    chg_status_type  chg_stat = {0};
    boolean op_result = TRUE;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_DRV:check is charger present!\n");
    op_result = chg_get_IC_status(&chg_stat);
    if (FALSE == op_result)
    {
        return FALSE;
    }
    else
    {
        //do nothing
    }

    if ((CHG_STAT_UNKNOWN == chg_stat.chg_vbus_stat)
         || (CHG_STAT_OTG == chg_stat.chg_vbus_stat))
    {
        return FALSE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_DRV:chg_is_charger_present-> present\n");
        return TRUE;

    }
}
/******************************************************************************
  Function      chg_is_IC_charging
  Description   ���IC�Ƿ��ڳ��
  Input         N/A
  Output        N/A
  Return        TRUE     : ���ڳ��
                FALSE    : ֹͣ���
  Others        N/A
******************************************************************************/
boolean chg_is_IC_charging(void)
{
    chg_status_type  chg_stat = {0};
    boolean op_result = TRUE;

    op_result = chg_get_IC_status(&chg_stat);
    if (FALSE == op_result)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: chg_get_IC_status error !\n");
        return FALSE;
    }
    else
    {
        //do nothing
    }

    if ((CHG_STAT_PRE_CHARGING == chg_stat.chg_chrg_stat)
        || (CHG_STAT_FAST_CHARGING == chg_stat.chg_chrg_stat))
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_DRV: chg_get_IC_status IC status charging!\n");
        return TRUE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_DRV: chg_get_IC_status IC status not charging!\n");
        return FALSE;
    }
}

/******************************************************************************
  Function      chg_get_stop_charging_reason
  Description   ��ȡ���ֹͣԭ��
  Input         N/A
  Output        N/A
  Return        ���ֹͣԭ��chg_stop_reason:
                CHG_STOP_COMPLETE,
                CHG_STOP_TIMEOUT,
                CHG_STOP_INVALID_TEMP,
                CHG_STOP_BY_SW
  Others        N/A
******************************************************************************/
chg_stop_reason chg_get_stop_charging_reason(void)
{
    chg_status_type chg_stat = {0};
    boolean op_result = TRUE;

    op_result = chg_get_IC_status(&chg_stat);
    if (FALSE == op_result)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: chg_get_IC_status error CHG_STOP_OTHER_REASON!\n");
        return CHG_STOP_OTHER_REASON;
    }
    else
    {
        //do nothing
    }

    if (CHG_STAT_CHARGE_DONE == chg_stat.chg_chrg_stat)
    {
        return CHG_STOP_COMPLETE;
    }
    else if ((CHG_WATCHDOG_EXP == chg_stat.chg_watchdog_fault)
        || (CHG_FAULT_SAFTY_TIMER_EXP == chg_stat.chg_chrg_fault))
    {
        return CHG_STOP_TIMEOUT;
    }
    else if (CHG_FAULT_THERMAL_SHUTDOWN == chg_stat.chg_chrg_fault)
    {
        return CHG_STOP_INVALID_TEMP;
    }
    else if (CHG_FAULT_INPUT_FLT == chg_stat.chg_chrg_fault)
    {
        return CHG_STOP_SUPPLY_ERR;
    }
    else if (CHG_BAT_OVP == chg_stat.chg_bat_fault)
    {
        return CHG_STOP_BAT_ERR;
    }
    else if (CHG_CONFIG_CHG_DIS == chg_stat.chg_ce_stat)
    {
        return CHG_STOP_BY_SW;
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR OTHER STOP CHARGING REASON!\n");
        return CHG_STOP_OTHER_REASON;
    }
}

/******************************************************************************
  Function      chg_set_chip_cd
  Description   ����оƬƬѡʹ��
  Input         ����оƬ��Ӧ��CD��GPIO��ʾʹ��оƬ�����߱�ʾ�ر�оƬ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
void chg_set_chip_cd(boolean enable)
{
    if (!gpio_is_valid(CHARGER_IC_ENABLE_GPIO))         /* */
    {
        chg_print_level_message( CHG_MSG_DEBUG,"CHG_DRV: set charger enable GPIO%d failed!!\r\n ", CHARGER_IC_ENABLE_GPIO);
    }
    if (gpio_request(CHARGER_IC_ENABLE_GPIO, "chip_cd"))         /* */
    {
        chg_print_level_message( CHG_MSG_DEBUG,"CHG_DRV:request charger enable GPIO%d failed!!\r\n ", CHARGER_IC_ENABLE_GPIO);
    }

    if(TRUE == enable)
    {
        gpio_direction_output(CHARGER_IC_ENABLE_GPIO,TRUE);
        chg_print_level_message( CHG_MSG_DEBUG,"CHG_DRV:set charger enable GPIO%d success!\r\n ", CHARGER_IC_ENABLE_GPIO);
    }
    else
    {
        gpio_direction_output(CHARGER_IC_ENABLE_GPIO,FALSE);
        chg_print_level_message( CHG_MSG_DEBUG,"CHG_DRV:set charger enable GPIO%d success!\r\n ", CHARGER_IC_ENABLE_GPIO);
    }
}

/******************************************************************************
  Function      chg_set_boost_therm_protect_threshold
  Description   ���ó��оƬBOOSTģʽ���¶ȱ�������ֵ
  Input         temp      : �����õ�����ֵ�¶�, 0 ��ʾ���ñ������ܡ�
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        
******************************************************************************/
boolean chg_set_boost_therm_protect_threshold(int32_t temp)
{
    uint8_t  reg         = 0;
    uint8_t  reg_val     = 0;
    int32_t  ret         = 0;
    int32_t temp_in     = temp;

    /*BCOLD��BHOT���������ڲ�ͬ�ļĴ��������ж�Ҫ�����ĸ��Ĵ���*/
    if (temp_in >= 0)
    {
        reg = BQ25892_REG01;
        chg_print_level_message(CHG_MSG_DEBUG,
            "Temp[%d'C] to set >= 0, Reg[0x%02x] selected.\n",
            temp_in, BQ25892_BOOST_REG0A);

        ret = bq25892_i2c_read(reg, &reg_val);
        if (0 != ret)
        {
            chg_print_level_message(CHG_MSG_ERR,
                "Read BOOST_THERMPRT_REG[0x01] error with 0x%x!\n", ret, 0, 0);
            return FALSE;
        }

        chg_print_level_message(CHG_MSG_DEBUG,
            "CHG_DRV_DBG:chg_set_boost_therm_protect_threshold() " \
            "reg %d, value 0x%x before!\n", reg, reg_val,0);

        reg_val &= ~(BQ25892_BHOT_MASK);
        if (0 == temp_in)
        {
            /*Disable Boost Mode Thermal Protect Function.*/
            reg_val |= BQ25892_BHOT_DISABLE;

            chg_print_level_message(CHG_MSG_DEBUG,
                "Boost Mode Thermal Protection Disabled.\n", 0, 0, 0);
        }
        else
        {
            if (temp_in >= CHG_BHOT_THOLD_PLUS65DEGC)
            {
                reg_val |= BQ25892_BHOT2_65DEGC;
            }
            else if (temp_in >= CHG_BHOT_THOLD_PLUS60DEGC)
            {
                reg_val |= BQ25892_BHOT0_60DEGC;
            }
            else if (temp_in >= CHG_BHOT_THOLD_PLUS55DEGC)
            {
                reg_val |= BQ25892_BHOT1_55DEGC;
            }
            else
            {
                chg_print_level_message(CHG_MSG_ERR,
                    "Invalid BHOT THERM_PRT_TH %d.\n", temp_in, 0, 0);

                reg_val |= BQ25892_BHOT1_55DEGC;
                return FALSE;
            }
        }

        ret = bq25892_i2c_write(reg, reg_val);
        if (0 != ret)
        {
            chg_print_level_message(CHG_MSG_ERR,
                "Write BOOST_THERMPRT_REG[0x06] error with 0x%x!\n", ret, 0, 0);
            return FALSE;
        }

        chg_print_level_message(CHG_MSG_DEBUG,
            "CHG_DRV_DBG:chg_set_boost_therm_protect_threshold() " \
            "reg[0x%02x], value 0x%x after!\n", reg, reg_val,0);
    }
    else
    {
        reg = BQ25892_REG01;
        chg_print_level_message(CHG_MSG_DEBUG,
            "Temp[%d'C] to set < 0, Reg[0x%02x] selected.\n",
            temp_in, BQ25892_REG01);

        ret = bq25892_i2c_read(reg, &reg_val);
        if (0 != ret)
        {
            chg_print_level_message(CHG_MSG_ERR,
                "Read Boost BCOLD REG[0x01] error with 0x%x!\n", ret, 0, 0);
            return FALSE;
        }

        chg_print_level_message(CHG_MSG_DEBUG,
            "CHG_DRV_DBG:chg_set_boost_therm_protect_threshold() " \
            "reg[0x%02x], value 0x%x before!\n", reg, reg_val,0);

        reg_val &= ~(BQ25892_BCOLD_MASK);
        if (temp_in <= CHG_BCOLD_THOLD_MINUS20DEGC)
        {
            reg_val |= BQ25892_BCOLD1_N20DEGC;
        }
        else if (temp_in <= CHG_BCOLD_THOLD_MINUS10DEGC)
        {
            reg_val |= BQ25892_BCOLD0_N10DEGC;
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR,
                "Invalid BCOLD THERM_PRT_TH %d.\n", temp_in, 0, 0);

            reg_val |= BQ25892_BCOLD0_N10DEGC;
            return FALSE;
        }

        ret = bq25892_i2c_write(reg, reg_val);
        if (0 != ret)
        {
            chg_print_level_message(CHG_MSG_ERR,
                "Write Boost BCOLD REG[0x02] error with 0x%x!\n", ret, 0, 0);
            return FALSE;
        }

        chg_print_level_message(CHG_MSG_DEBUG,
            "CHG_DRV_DBG:chg_set_boost_therm_protect_threshold() " \
            "reg[0x%02x], value 0x%x after!\n", reg, reg_val,0);
    }

    return TRUE;
}

/******************************************************************************
  Function      chg_set_boost_volt
  Description   ���ó��оƬBOOSTģʽ��ѹֵ
  Input         �����õ�Boostģʽ��ѹֵ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        
******************************************************************************/
boolean chg_set_boost_volt(uint32_t boostv)
{
    uint8_t  reg         = 0;
    uint8_t  reg_val     = 0;
    int32_t  ret         = 0;
    uint32_t boostv_left = 0;
    reg = BQ25892_BOOST_REG0A;
    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR,
            "Read BOOSTV_REG[0x0A] error with 0x%x!\n", ret, 0, 0);
        return FALSE;
    }

    chg_print_level_message(CHG_MSG_DEBUG,
        "CHG_DRV_DBG:chg_set_boost_volt() reg[0x%02x], value 0x%x before!\n",
        reg, reg_val,0);

    reg_val &= ~(BQ25892_BOOSTV_MASK);

    if (boostv >= BQ25892_BOOSTV_MAX)
    {
        reg_val |= (BQ25892_BOOSTV_512MV | BQ25892_BOOSTV_256MV
                  | BQ25892_BOOSTV_128MV | BQ25892_BOOSTV_64MV);
    }
    else if (boostv <= BQ25892_BOOSTV_OFFSET)
    {
        /*NOP*/
        reg_val = reg_val;
    }
    else
    {
        boostv_left = boostv - BQ25892_BOOSTV_OFFSET;
        if (boostv_left >= CHG_BOOSTV_OFFSET_512MV)
        {
            reg_val |= BQ25892_BOOSTV_512MV;
            boostv_left -= CHG_BOOSTV_OFFSET_512MV;
        }

        if (boostv_left >= CHG_BOOSTV_OFFSET_256MV)
        {
            reg_val |= BQ25892_BOOSTV_256MV;
            boostv_left -= CHG_BOOSTV_OFFSET_256MV;
        }

        if (boostv_left >= CHG_BOOSTV_OFFSET_128MV)
        {
            reg_val |= BQ25892_BOOSTV_128MV;
            boostv_left -= CHG_BOOSTV_OFFSET_128MV;
        }

        if (boostv_left >= CHG_BOOSTV_OFFSET_64MV)
        {
            reg_val |= BQ25892_BOOSTV_64MV;
        }
    }

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR,
            "Write BOOSTV_REG[0x0A] error with 0x%x!\n", ret, 0, 0);
        return FALSE;
    }

    chg_print_level_message(CHG_MSG_DEBUG,
        "CHG_DRV_DBG:chg_set_boost_volt() reg[0x%02x], value 0x%x after!\n",
        reg, reg_val,0);

    return TRUE;
}

/******************************************************************************
  Function      chg_get_part_nr
  Description   ��ȡ���оƬ���ͺ�
  Input         N/A
  Output        pn        : ���ص�оƬ�ͺ�
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_get_part_nr(chg_chip_part_nr *pn)
{
    uint8_t  reg     = 0;
    uint8_t  reg_val = 0;
    int32_t  ret     = 0;

    if (NULL == pn)
    {
        chg_print_level_message(CHG_MSG_ERR,
            "chg_boot_get_part_nr: pn arg is NULL.\n", 0, 0, 0);
        return FALSE;
    }

    if (CHG_CHIP_PN_INVALID != g_chg_chip_pn)
    {
        *pn = g_chg_chip_pn;
        return TRUE;
    }
    else
    {
        reg = BQ25892_REVISION_REG14;
        ret = bq25892_i2c_read(reg, &reg_val);
        if (0 != ret)
        {
            chg_print_level_message(CHG_MSG_ERR,
                "Get Charge IC PN Error with 0x%x!\n", ret, 0, 0);
            return FALSE;
        }
        else
        {
            /*First, Check whether We have BQ25892 charger IC.*/
            if (BQ25892_PART_NR_VALUE == (reg_val & BQ2589x_PART_NR_MASK))
            {
                chg_print_level_message(CHG_MSG_INFO,
                    "BQ24196 Charger IC attached.\n", 0, 0, 0);
                g_chg_chip_pn = CHG_CHIP_PN_BQ25892;
            }
            /*Then, check BQ25890.*/
            else if (BQ25890_PART_NR_VALUE == (reg_val & BQ2589x_PART_NR_MASK))
            {
                chg_print_level_message(CHG_MSG_INFO,
                    "BQ25890 Charge IC attached.\n", 0, 0, 0);
                g_chg_chip_pn = CHG_CHIP_PN_BQ25890;
            }
            else
            {
                chg_print_level_message(CHG_MSG_ERR,
                    "Unknown Charge IC. Reg[0x0A] = 0x%x.\n", reg_val, 0, 0);
                return FALSE;
            }
        }
    }

    *pn = g_chg_chip_pn;
    return TRUE;
}

/****************���BQ25892��д�Ĵ����ɲ����Խӿ�****************************/
uint8_t chg_test_i2c_read_reg(uint8_t reg)
{
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR,
            "Read Reg[0x%02x] Error with 0x%x!\n", reg, ret, 0);
        return (0xFF);
    }

    chg_print_level_message(CHG_MSG_ERR,
                "Reg[0x%02x] = 0x%02x.\n", reg, reg_val, 0);
    return reg_val;
}

boolean chg_test_i2c_write_reg(uint8_t reg, uint8_t val)
{
    int32_t ret     = 0;

    ret = bq25892_i2c_write(reg, val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR,
            "Write Reg[0x%02x] Error with 0x%x!\n", reg, ret, 0);
        return (FALSE);
    }

    chg_print_level_message(CHG_MSG_ERR,
                "Write 0x%02x to Reg[0x%02x] successful.\n", reg, val, 0);
    return TRUE;
}
#define ONE_LINE_PRINT_NUM    (4)
boolean chg_test_dump_all_regs(void)
{
    uint8_t chg_bq_reg = 0;
    uint8_t reg_val = 0;
    int32_t ret = 0;

    chg_print_level_message(CHG_MSG_ERR, "Dump all BQ24x96 Regs:\n",
                chg_bq_reg, reg_val,0);

    chg_bq_reg = BQ25892_INPUT_CTL_REG00;
    for (; chg_bq_reg < bq25892_MAX_REGISTER_CNT; chg_bq_reg++)
    {
        ret = bq25892_i2c_read(chg_bq_reg, &reg_val);
        if (0 != ret)
        {
            chg_print_level_message(CHG_MSG_ERR,
                "Read Reg[0x%02x] Error with 0x%x!\n", chg_bq_reg, ret, 0);
            return (FALSE);
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x ",
                chg_bq_reg, reg_val, 0);
            if (0 == ((chg_bq_reg + 1) % ONE_LINE_PRINT_NUM))
            {
                chg_print_level_message(CHG_MSG_ERR, "\n", 0, 0, 0);
            }
        }
    }
    chg_print_level_message(CHG_MSG_ERR, "\n", 0, 0, 0);
    return TRUE;

}

boolean chg_test_get_ic_name(void)
{
    if (CHG_CHIP_PN_BQ25892 == g_chg_chip_pn)
    {
        chg_print_level_message(CHG_MSG_ERR, "Charge IC BQ25892 Attached.\n", 0, 0, 0);
    }
    else if (CHG_CHIP_PN_BQ25892 == g_chg_chip_pn)
    {
        chg_print_level_message(CHG_MSG_ERR, "Charge IC BQ25890 Attached.\n", 0, 0, 0);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "Unknown Charge IC, please check 0x0A Register.\n",
            0, 0, 0);
    }

    return TRUE;
}

/******************************************************************************
  Function      chg_adc_convert_enable
  Description   ����ADCʹ�ܻ��ʹ��
  Input         enable
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_adc_convert_enable(boolean enable)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;
    reg = BQ25892_POWER_ON_CFG_REG02;
    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR,
            "Read BOOSTV_REG[0x02] error with 0x%x!\n", ret, 0, 0);
        return FALSE;
    }

    if (TRUE == enable)
    {
        reg_val |= BQ25892_CONV_START;
    }
    else
    {
        reg_val &= BQ25892_CONV_DIS;
    }

    /*����Ϊ1sת��1��*/
    reg_val |= BQ25892_CONV_RATE_CONTINUOUS;
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV:  reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR SETTING ADC ENABLE!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }

}

/******************************************************************************
  Function      boolean chg_get_hvdcp_adapter_vol
  Description   ��ȡ�������ǰ�����ѹ
  Input         N/A
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
int chg_get_hvdcp_adpter_vol(void)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;
    /*enable adc convert*/
    if (FALSE == chg_adc_convert_enable(TRUE) )
    {
        chg_print_level_message(CHG_MSG_ERR,"chg_get_hvdcp_adpter_vol:enable ADC err!\r\n ");
        return (-1);
    }
    chg_delay_ms(DELAY_20_MS);

    reg = BQ25892_VBUSV_VOL_REG11;
    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR,
            "Read BOOSTV_REG[0x11] error with 0x%x!\n", ret, 0, 0);
        return (-1);
    }
 
    ret = BQ25892_VBUSV_VOL_OFFSET
        + (int32_t)((reg_val & BQ25892_VBUSV_VOL_6400MV ) >> BQ25892_VBUSV_6_VOL_BITPOS) \
        * CHG_VBUS_VOL_6400
        + (int32_t)((reg_val & BQ25892_VBUSV_VOL_3200MV ) >> BQ25892_VBUSV_5_VOL_BITPOS) \
        * CHG_VBUS_VOL_3200
        + (int32_t)((reg_val & BQ25892_VBUSV_VOL_1600MV ) >> BQ25892_VBUSV_4_VOL_BITPOS) \
        * CHG_VBUS_VOL_1600
        + (int32_t)((reg_val & BQ25892_VBUSV_VOL_800MV ) >> BQ25892_VBUSV_3_VOL_BITPOS) \
        * CHG_VBUS_VOL_800
        + (int32_t)((reg_val & BQ25892_VBUSV_VOL_400MV ) >> BQ25892_VBUSV_2_VOL_BITPOS) \
        * CHG_VBUS_VOL_400
        + (int32_t)((reg_val & BQ25892_VBUSV_VOL_200MV ) >> BQ25892_VBUSV_1_VOL_BITPOS) \
        * CHG_VBUS_VOL_200
        + (int32_t)((reg_val & BQ25892_VBUSV_VOL_100MV ) >> BQ25892_VBUSV_0_VOL_BITPOS) \
        * CHG_VBUS_VOL_100;
    return ret;
}

/******************************************************************************
  Function      chg_get_hvdcp_charge_current
  Description   ��ȡ��������С����1C+0.5C����ж�
  Input
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
int32_t chg_get_hvdcp_charge_current(void)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;
    int32_t current_value = 0;
#if 1
    /*enable adc convert*/
    if (FALSE == chg_adc_convert_enable(TRUE) )
    {
        chg_print_level_message(CHG_MSG_ERR,"chg_get_hvdcp_adpter_vol:enable ADC err!\r\n ");
        return current_value;
    }
#endif
    chg_delay_ms(DELAY_20_MS);
    reg = BQ25892_ICHGR_CUR_REG12;
    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR,
            "Read BOOSTV_REG[0x12] error with 0x%x!\n", ret, 0, 0);
        return current_value;
    }

    current_value += (int32_t)( (reg_val & BQ25892_ICHGR_CUR_3200MA) >> BQ25892_ICHGR_6_CUR_BITPOS )\
                            * CHG_ICHG_CUR_3200MA
                            + (int32_t)((reg_val & BQ25892_ICHGR_CUR_1600MA) >> BQ25892_ICHGR_5_CUR_BITPOS)\
                            * CHG_ICHG_CUR_1600MA
                            + (int32_t)((reg_val & BQ25892_ICHGR_CUR_800MA) >> BQ25892_ICHGR_4_CUR_BITPOS)\
                            * CHG_ICHG_CUR_800MA
                            + (int32_t)((reg_val & BQ25892_ICHGR_CUR_400MA) >> BQ25892_ICHGR_3_CUR_BITPOS)\
                            * CHG_ICHG_CUR_400MA
                            + (int32_t)((reg_val & BQ25892_ICHGR_CUR_200MA) >> BQ25892_ICHGR_2_CUR_BITPOS)\
                            * CHG_ICHG_CUR_200MA
                            + (int32_t)((reg_val & BQ25892_ICHGR_CUR_100MA) >> BQ25892_ICHGR_1_CUR_BITPOS)\
                            * CHG_ICHG_CUR_100MA
                            + (int32_t)((reg_val & BQ25892_ICHGR_CUR_50MA) >> BQ25892_ICHGR_0_CUR_BITPOS)\
                            * CHG_ICHG_CUR_50MA;

    return current_value;
}

/******************************************************************************
  Function      chg_set_afvc_voltage
  Description   ��ֹ/���ó��оƬ��ѹ������ѹ
  Input         ��ѹ������ѹֵ
                    CHG_AFVC_32MV,
                    CHG_AFVC_64MV,
                    CHG_AFVC_96MV,
                    CHG_AFVC_128MV,
                    CHG_AFVC_160MV,
                    CHG_AFVC_192MV,
                    CHG_AFVC_224MV,
                    CHG_AFVC_DIS
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_afvc_voltage(chg_afvc_voltage_type afvc_voltage_type)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    reg = BQ25892_REG08;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV:chg_set_afvc_voltage: ERROR READING !\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_afvc_voltage reg 0x%x, value 0x%x before!\n", reg, reg_val);

    reg_val &= ~(BQ25892_VCLAMP_MASK);

    switch (afvc_voltage_type)
    {
        case CHG_AFVC_32MV:
        {
            reg_val |= BQ25892_VCLAMP_32MV;
            break;
        }
        case CHG_AFVC_64MV:
        {
            reg_val |= BQ25892_VCLAMP_64MV;
            break;
        }
        case CHG_AFVC_96MV:
        {
            reg_val |= (BQ25892_VCLAMP_32MV | BQ25892_VCLAMP_64MV);
            break;
        }
        case CHG_AFVC_128MV:
        {
            reg_val |= BQ25892_VCLAMP_128MV;
            break;
        }
        case CHG_AFVC_160MV:
        {
            reg_val |= (BQ25892_VCLAMP_32MV | BQ25892_VCLAMP_128MV);
            break;
        }
        case CHG_AFVC_192MV:
        {
            reg_val |= (BQ25892_VCLAMP_64MV | BQ25892_VCLAMP_128MV);
            break;
        }
        case CHG_AFVC_224MV:
        {
            reg_val |= (BQ25892_VCLAMP_32MV | BQ25892_VCLAMP_64MV | BQ25892_VCLAMP_128MV);
            break;
        }
        case CHG_AFVC_DIS:
        {
            break;
        }
        default:
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_DRV:chg_set_afvc_voltage input para error! \n");
            break;
        }
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_afvc_voltage reg 0x%x, value 0x%x after!\n", reg, reg_val);
    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV:chg_set_afvc_voltage: ERROR SETTING !\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/******************************************************************************
  Function      boolean chg_chip_init
  Description   ���оƬ��ʼ��
  Input         N/A
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_chip_init(void)
{
    int32_t ret_code = 0;
    static int s_chg_chip_init_flag = 0;

    /*�����жԳ��IC����֮ǰ�����жϵ�ǰ�ĳ��IC�ͺš�*/
    chg_chip_part_nr cur_chip_pn = CHG_CHIP_PN_INVALID;

    chg_print_level_message(CHG_MSG_INFO,"CHG_DRV: chg_chip_init \n ");

    if(0 == s_chg_chip_init_flag)
    {
        s_chg_chip_init_flag = 1;
    }
    else
    {
        return TRUE;
    }

    /*�����жԳ��IC����֮ǰ���Ȼ�ȡ��ǰ�ĳ��IC�ͺš�*/
    if (FALSE == chg_get_part_nr(&cur_chip_pn))
    {
        /*If error, treat as bq25892 as default.*/
        chg_print_level_message(CHG_MSG_ERR,
            "Get Charge IC PN error, treat as BQ25892.\n", 0, 0, 0);
        g_chg_chip_pn = CHG_CHIP_PN_BQ25892;
    }

    /*�رճ��ʹ��*/
    if (FALSE == chg_set_charge_enable(FALSE))
    {
        /*If error occured, set 0 bit of ret_code.*/
        ret_code |= (1 << 0);
    }

    /*���ó�����*/
    else if (FALSE == chg_set_cur_level(BQ25892_ICHARGE_OFFSET))
    {
        /*If error occured, set 1 bit of ret_code.*/
        ret_code |= (1 << 1);
    }
    /*������������*/
    else if (FALSE == chg_set_supply_limit(CHG_INPUT_LIMIT_500MA))
    {
        /*If error occured, set 2 bit of ret_code.*/
        ret_code |= (1 << 2);
    }
    /*���ý�ֹ����*/
    else if (FALSE == chg_set_term_current(CHG_DEFAULT_TERM_CURRENT))
    {
        /*If error occured, set 3 bit of ret_code.*/
        ret_code |= (1 << 3);
    }
    /*���ú�ѹ��ѹ*/
    else if (FALSE == chg_set_vreg_level(CHG_DEFAULT_VREG))
    {
        /*If error occured, set 4 bit of ret_code.*/
        ret_code |= (1 << 4);
    }
    /*����DPM��ѹֵ*/
    else if (FALSE == chg_set_dpm_val(CHG_DEFAULT_DPM_VOLT))
    {
        /*If error occured, set 5 bit of ret_code.*/
        ret_code |= (1 << 5);
    }
    /*����Ϊ��suspendģʽ*/
    else if (FALSE == chg_set_suspend_mode(FALSE))
    {
        /*If error occured, set 6 bit of ret_code.*/
        ret_code |= (1 << 6);
    }
    /*���ý�ֹ����ʹ��*/
    else if (FALSE == chg_set_te_enable(TRUE))
    {
        /*If error occured, set 7 bit of ret_code.*/
        ret_code |= (1 << 7);
    }
    /*�ر�оƬ�¶ȱ�������*/
    else if (FALSE == chg_set_ts_enable(FALSE))
    {
        /*If error occured, set 8 bit of ret_code.*/
        ret_code |= (1 << 8);
    }
    /*����STATʹ��.*/
    else if (FALSE == chg_set_stat_enable(TRUE))
    {
        /*If error occured, set 9 bit of ret_code.*/
        ret_code |= (1 << 9);
    }
    /*����LOW_CHG��ʹ�ܣ���оƬ�Ե�����������õĳ��������г��.*/
    else if (FALSE == chg_set_charge_mode(FALSE))
    {
        /*If error occured, set 10 bit of ret_code.*/
        ret_code |= (1 << 10);
    }
    /*�رհ�ȫ��ʱ�����ܣ���Ҫʱ�������ô�*/
    else if (FALSE == chg_set_safety_timer_enable(CHG_DISABLE_SAFETY_TIMER))
    {
        /*If error occured, set 11 bit of ret_code.*/
        ret_code |= (1 << 11);
    }
    /*�Գ��оƬ�����߹�����.*/
    else if (FALSE == chg_set_tmr_rst())
    {
        /*If error occured, set 12 bit of ret_code.*/
        ret_code |= (1 << 12);
    }
    /*��ʱ�ȹر�watchdog*/
    else if (FALSE == chg_set_tmr_setting(CHG_DISABLE_WATCHDOG_TIMER))
    {
        /*If error occured, set 13 bit of ret_code.*/
        ret_code |= (1 << 13);
    }
    /*Disable Boost Mode Thermal Protect Function.*/
    else if (FALSE == chg_set_boost_therm_protect_threshold(FALSE))
    {
        /*If error occured, set 14 bit of ret_code.*/
        ret_code |= (1 << 14);
    }

    /*ʹ���Զ���ѹ��ѹ����AFVC������Ϊ128MV*/
    else if(FALSE == chg_set_afvc_voltage(CHG_AFVC_32MV))
    {
        /*If error occured, set 21 bit of ret_code.*/
        ret_code |= (1 << 15);
    }
    /*ʹ��bq25892�ڲ�ADC*/
    else if (FALSE == chg_adc_convert_enable(TRUE) )
    {
        chg_print_level_message(CHG_MSG_ERR,"chg_adc_convert_enable:enable ADC err!\r\n ");
        ret_code |= (1 << 16);
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_DRV: CHARGE INIT SUCCESS!\n");
    }
    /*���оƬʹ��*/
    chg_set_chip_cd(TRUE);

    if (0 != ret_code)
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_DRV: CHARGE INIT: ERRNO %d!\n",ret_code);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

#if (MBB_CHG_WIRELESS == FEATURE_ON)

void chg_set_wireless_chg_enable(boolean enable)
{
    if(TRUE == enable)
    {
        //TO DO:����WirelessEN1=LOW ;WirelessEN2=LOW;
        //WirelessEN1��Ӧ��GPIOΪ:GPIO_59/GSM_TX_PHASE_D1 WirelessEN2��Ӧ�Ĺܽ�Ϊ:GPIO_60/GSM_TX_PHASE_D0
        wireless_gpio_control(WIRELESS_CONTROL_GPIO_EN1,FALSE); /* */
#if (FEATURE_ON == MBB_CHG_PLATFORM_BALONG)
#else
        /* V7R2ƽ̨��EN2��Ӳ��ǿ�����ͣ�������ÿ��� */
        wireless_gpio_control(WIRELESS_CONTROL_GPIO_EN2,FALSE); /* */
#endif
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_PLT:enable wireless chging \n ");
    }
    else
    {
        //TO DO:����WirelessEN1=HIGH ;WirelessEN2=LOW;
        //WirelessEN1��Ӧ��GPIOΪ:GPIO_59/GSM_TX_PHASE_D1 WirelessEN2��Ӧ�Ĺܽ�Ϊ:GPIO_60/GSM_TX_PHASE_D0
        wireless_gpio_control(WIRELESS_CONTROL_GPIO_EN1,TRUE);  /* */
#if (FEATURE_ON == MBB_CHG_PLATFORM_BALONG)
#else
        /* V7R2ƽ̨��EN2��Ӳ��ǿ�����ͣ�������ÿ��� */
        wireless_gpio_control(WIRELESS_CONTROL_GPIO_EN2,FALSE); /* */
#endif
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_PLT:enable wireless chging \n ");
    }
}
#endif/*MBB_CHG_WIRELESS*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)
/******************************************************************************
  Function      chg_extchg_ocp_detect
  Description   ������������
  Input         N/A
  Output
  Return        TURE:����
                FALSE:û�й���
  Others        N/A
******************************************************************************/
boolean chg_extchg_ocp_detect(void)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    reg = BQ25892_FAULT_REG0C;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_extchg_ocp_detect!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_DRV:  chg_extchg_ocp_detect() reg %d, value 0x%x!\n", reg, reg_val);
    reg_val &= BQ25892_BOOST_FLT;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_DRV: chg_extchg_ocp_detect() ocp_state=%d!\n",(reg_val >> BQ25892_BOOST_FLT_BITPOS));
    return (reg_val >> BQ25892_BOOST_FLT_BITPOS);
}

/******************************************************************************
  Function      chg_set_charge_otg_enable
  Description   �趨�Ƿ�ʹ�ܶ������OTGģʽ
  Input         en_val:TRUE   ����ʹ��OTG
                       FALSE  ������ֹOTG
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_charge_otg_enable(boolean enable)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    reg = BQ25892_POWER_ON_CFG_REG03;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_set_charge_otg_enable!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_charge_otg_enable() reg %d, value 0x%x before!\n", reg, reg_val);

    reg_val &= ~(BQ25892_CHG_CFG_MASK);

    if (TRUE == enable)
    {
        reg_val |= BQ25892_OTG_CFG_EN;
    }
    else
    {
        reg_val |= BQ25892_OTG_CFG_DIS;
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_charge_otg_enable() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR SETTING OTG ENABLE VALUE!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/******************************************************************************
  Function      chg_set_extchg_current_limit
  Description   �趨�������1A����
  Input         N/A
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_extchg_current_limit(void)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    reg = BQ25892_BO0ST_REG0A;

    ret = bq25892_i2c_read(reg, &reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR READING chg_set_extchg_current_limit!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }

    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_extchg_current_limit() reg %d, value 0x%x before!\n", reg, reg_val);

    reg_val &= ~(BQ25892_BOOST_LIM_MASK);

    reg_val |= BQ25892_BOOST_LIM_1000MA;

    chg_print_level_message(CHG_MSG_INFO, "CHG_DRV: chg_set_extchg_current_limit() reg %d, value 0x%x after!\n", reg, reg_val);

    ret = bq25892_i2c_write(reg, reg_val);
    if (0 != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: ERROR SETTING OTG 1A VALUE!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

extern boolean g_exchg_enable_flag;

void chg_set_extchg_chg_enable(boolean enable)
{
    /*����1A����������*/
    chg_set_extchg_current_limit();
    /*�����оƬ����ΪOTGģʽ*/
    chg_set_charge_otg_enable(enable);
    /*����MOS��ʹ�ܶ�����*/
    if(TRUE == enable)
    {
        g_exchg_enable_flag = TRUE;
        //TO DO:����WirelessEN1=HIGH ;WirelessEN2=LOW;
        //WirelessEN1��Ӧ��GPIOΪ:GPIO_59/GSM_TX_PHASE_D1 WirelessEN2��Ӧ�Ĺܽ�Ϊ:GPIO_60/GSM_TX_PHASE_D0
#if (FEATURE_ON != MBB_CHG_PLATFORM_BALONG)
        extchg_gpio_control(EXTCHG_CONTROL_GPIO_EN1,TRUE); /* */
        /* V7R2ƽ̨��EN2��Ӳ��ǿ�����ͣ�������ÿ��� */
        extchg_gpio_control(EXTCHG_CONTROL_GPIO_EN2,FALSE); /* */
#endif
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_PLT:enable EXTCHG chging \n");
    }
    else
    {
        g_exchg_enable_flag = FALSE;
        //TO DO:����WirelessEN1=LOW ;WirelessEN2=LOW;
        //WirelessEN1��Ӧ��GPIOΪ:GPIO_59/GSM_TX_PHASE_D1 WirelessEN2��Ӧ�Ĺܽ�Ϊ:GPIO_60/GSM_TX_PHASE_D0
#if (FEATURE_ON != MBB_CHG_PLATFORM_BALONG)
        extchg_gpio_control(EXTCHG_CONTROL_GPIO_EN1,FALSE); /* */
        /* V7R2ƽ̨��EN2��Ӳ��ǿ�����ͣ�������ÿ��� */
        extchg_gpio_control(EXTCHG_CONTROL_GPIO_EN2,FALSE); /* */
#endif
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_PLT:disable EXTCHG chging \n");
    }
}

#endif/*MBB_CHG_EXTCHG*/

