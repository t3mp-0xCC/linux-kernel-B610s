/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef HUAWEI_CHARGER_H
#define HUAWEI_CHARGER_H

/******************************问题单修改记录**********************************
    日期       修改人       问题单号                 修改内容
   2012.X.X    XXXX         DTSXXXX                  XXXXXXX

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <linux/power_supply.h>
#include <linux/notifier.h>
#include "product_config.h"

/*----------------------------------------------*
 * 结构体定义                                    *
 *----------------------------------------------*/
struct chargeIC_chip {
    struct i2c_client   *client;
    struct power_supply bat;
    struct power_supply usb;
    struct power_supply ac;
    struct power_supply extchg;
    struct power_supply wireless;

    int usb_online;                    /*USB是否在位，1表示在位，0表示不在位*/
    int ac_online;                     /*AC是否在位，1表示在位，0表示不在位*/
    int wireless_online;               /*无线充电是否在位，1表示在位，0表示不在位*/

    int extchg_online;                 /*USB ID线是否在位，1表示在位，0表示不在位*/
    int extchg_status;                 /*对外充电状态，正在对外充电还是停充等*/

    int bat_present;                   /*电池是否在位，1表示在位，0表示不在位*/
    int bat_stat;                      /*电池电压状态，是否在充电，是否需要补电等*/
    int bat_health;                    /*电池温度状态，是否过温等*/
    int bat_technology;                /*电池类型，锂电池等*/
    int bat_avg_voltage;               /*电池平均电压*/
    int bat_avg_temp;                  /*电池平均温度*/
    int bat_capacity;                  /*电池电量百分比*/
    int bat_time_to_full;              /*电池充满需要时间*/
    struct blocking_notifier_head notifier_list_bat;
    struct notifier_block nb;

    /*以下暂未使用为后续功能扩展留用*/
    int charge_current_limit_ua;
    int input_current_limit_ua;
    int term_current_ua;
    bool charging_enabled;
    bool otg_mode_enabled;
    bool charging_allowed;
    bool usb_suspend_enabled;
};
/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define CHARGEIC_PROC_FILE "driver/bq24192"
#define DLOAD_CHG_PROC_FILE "dload_chg"
#define POWERON_CHG_PROC_FILE "power_on"

#define ATOI_REG_ADDR_CNT           (0)
#define ATOI_REG_VALUE_CNT          (2)
#define ATOI_CONVERT_NUM            (10)
#define ATOI_MAX_LENGTH             (256)
#define DEV_ID_REG                  (0x0a)
#define DEV_ID_PN_MASK              (0x38)
#define DEV_ID_PN_SHIFT             (3)
#define DEV_ID_DEV_REG_MASK         (0x3)
#define DEV_ID_DEV_REG_SHIFT        (0)

#define ONLINE     1
#define OFFLINE    0

#define PRESENT    1
#define UNPRESENT  0

#define CHG_IINPUT_LIMIT_3000MA     (3000)
#define CHG_IINPUT_LIMIT_2000MA     (2000)
#define CHG_IINPUT_LIMIT_1500MA     (1500)
#define CHG_IINPUT_LIMIT_1200MA     (1200)
#define CHG_IINPUT_LIMIT_900MA      (900)
#define CHG_IINPUT_LIMIT_500MA      (500)
#define CHG_IINPUT_LIMIT_150MA      (150)
#define CHG_IINPUT_LIMIT_100MA      (100)


/*******************  REGISTER INFORMATION ************************************/
/****************0x00 Status/Control Register (Read/Write)*********************/
/*Memory Location: 00, Reset State: x1xx 0xxx*/
#define BQ24192_INPUT_CTL_REG        0x00
/*All 8 Register bits and relevant mask value*/
#define BQ24192_EN_HIZ_BITPOS        0x7
#define BQ24192_EN_HIZ              (0x1 << BQ24192_EN_HIZ_BITPOS)
#define BQ24192_DIS_HIZ             (0x0 << BQ24192_EN_HIZ_BITPOS)
#define BQ24192_EN_HIZ_MASK         (0x1 << BQ24192_EN_HIZ_BITPOS)

#define BQ24192_DPM_3_VOL_BITPO      0x6
#define BQ24192_DPM_VOL_640MV       (0x1 << BQ24192_DPM_3_VOL_BITPO)

#define BQ24192_DPM_2_VOL_BITPO      0x5
#define BQ24192_DPM_VOL_320MV       (0x1 << BQ24192_DPM_2_VOL_BITPO)

#define BQ24192_DPM_1_VOL_BITPO      0x4
#define BQ24192_DPM_VOL_160MV       (0x1 << BQ24192_DPM_1_VOL_BITPO)

#define BQ24192_DPM_0_VOL_BITPO      0x3
#define BQ24192_DPM_VOL_80MV        (0x1 << BQ24192_DPM_0_VOL_BITPO)
#define BQ24192_DPM_VOL_OFFSET       3880
#define BQ24192_DPM_VOL_MAX          5080
#define BQ24192_DPM_VOL_MASK        (0xF  << BQ24192_DPM_0_VOL_BITPO)

#define BQ24192_IINLIMIT_BITPOS      0
#define BQ24192_IINLIMIT_100        (0x0 << BQ24192_IINLIMIT_BITPOS)
#define BQ24192_IINLIMIT_150        (0x1 << BQ24192_IINLIMIT_BITPOS)
#define BQ24192_IINLIMIT_500        (0x2 << BQ24192_IINLIMIT_BITPOS)
#define BQ24192_IINLIMIT_900        (0x3 << BQ24192_IINLIMIT_BITPOS)
#define BQ24192_IINLIMIT_1200       (0x4 << BQ24192_IINLIMIT_BITPOS)
#define BQ24192_IINLIMIT_1500       (0x5 << BQ24192_IINLIMIT_BITPOS)
#define BQ24192_IINLIMIT_2000       (0x6 << BQ24192_IINLIMIT_BITPOS)
#define BQ24192_IINLIMIT_3000       (0x7 << BQ24192_IINLIMIT_BITPOS)
#define BQ24192_IINLIMIT_MASK       (0x7 << BQ24192_IINLIMIT_BITPOS)


/**************0x01 BAT/SUPPLY STATUS Register (Read/Write)********************/
/*Memory Location: 01, Reset State: xxxx 0xxx*/
#define BQ24192_POWER_ON_CFG_REG     0x01
/*All 8 Register bits and relevant mask value*/
#define BQ24192_RESET_BITPOS         0x7
#define BQ24192_RESET_ALL           (0x1 << BQ24192_RESET_BITPOS)
#define BQ24192_RESET_ALL_MASK      (0x1 << BQ24192_RESET_BITPOS)

#define BQ24192_TMR_RST_BITPOS       0x6
#define BQ24192_TMR_RST             (0x1 << BQ24192_TMR_RST_BITPOS)
#define BQ24192_TMR_RST_MASK        (0x1 << BQ24192_TMR_RST_BITPOS)


#define BQ24192_CHG_CFG_BITPOS       0x4
#define BQ24192_CHG_CFG_DIS         (0x0 << BQ24192_CHG_CFG_BITPOS)
#define BQ24192_CHG_CFG_BAT         (0x1 << BQ24192_CHG_CFG_BITPOS)
#define BQ24192_CHG_CFG_OTG         (0x2 << BQ24192_CHG_CFG_BITPOS)
#define BQ24192_CHG_CFG_MASK        (0x3 << BQ24192_CHG_CFG_BITPOS)

#define BQ24192_SYS_MIN_2_BITPOS     0x3
#define BQ24192_SYS_MIN_400MV       (0x1 << BQ24192_SYS_MIN_2_BITPOS)

#define BQ24192_SYS_MIN_1_BITPOS     0x2
#define BQ24192_SYS_MIN_200MV       (0x1 << BQ24192_SYS_MIN_1_BITPOS)

#define BQ24192_SYS_MIN_0_BITPOS     0x1
#define BQ24192_SYS_MIN_100MV       (0x1 << BQ24192_SYS_MIN_0_BITPOS)
#define BQ24192_SYS_MIN_MASK        (0x7 << BQ24192_SYS_MIN_0_BITPOS)
#define BQ24192_SYS_MIN_OFFSET       3000

#define BQ24192_BOOST_LIM_BITPOS     0x0
#define BQ24192_BOOST_LIM_500MA     (0x0 << BQ24192_BOOST_LIM_BITPOS)
#define BQ24192_BOOST_LIM_1300MA    (0x1 << BQ24192_BOOST_LIM_BITPOS)
#define BQ24192_BOOST_LIM_MASK      (0x1 << BQ24192_BOOST_LIM_BITPOS)




#endif

