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

#ifndef _CHG_CHARGE_DRV_H
#define _CHG_CHARGE_DRV_H
 /*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
#include <product_config.h>
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/
/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#if (MBB_CHG_WIRELESS == FEATURE_ON)
#define CHG_WIRELESS_DPM_VOLT              (4760)
#endif/*MBB_CHG_WIRELESS == FEATURE_ON)*/

extern int g_default_dpm_volt;
#define CHG_DEFAULT_DPM_VOLT               (g_default_dpm_volt)

#define PLUG_UNKNOW                 (-1)
#define PLUG_IN                      1
#define PLUG_OUT                     0
#ifdef BSP_CONFIG_BOARD_E5_DC04
#define VBUS_JUDGEMENT_THRESHOLD     484     /*DC04 VBUS�жϵ�����ֵ0.4834V*/
#else
#define VBUS_JUDGEMENT_THRESHOLD     700     /*VBUS�жϵ�����ֵ0.7V*/
#endif
/*��������غ�*/
#define CHG_CURRENT_2048MA_B7       (2048)
#define CHG_CURRENT_1024MA_B6       (1024)
#define CHG_CURRENT_512MA_B5        (512)
#define CHG_CURRENT_256MA_B4        (256)
#define CHG_CURRENT_128MA_B3        (128)
#define CHG_CURRENT_64MA_B2         (64)


#define CHG_IINPUT_LIMIT_3000MA     (3000)
#define CHG_IINPUT_LIMIT_2000MA     (2000)
#define CHG_IINPUT_LIMIT_1500MA     (1500)
#define CHG_IINPUT_LIMIT_1200MA     (1200)

#define CHG_IINPUT_LIMIT_1000MA     (1000)
#define CHG_IINPUT_LIMIT_900MA      (900)
#define CHG_IINPUT_LIMIT_500MA      (500)
#define CHG_IINPUT_LIMIT_150MA      (150)
#define CHG_IINPUT_LIMIT_100MA      (100)

#define CHG_TERM_CURRENT_1024MA     (1024)
#define CHG_TERM_CURRENT_512MA      (512)
#define CHG_TERM_CURRENT_256MA      (256)
#define CHG_TERM_CURRENT_128MA      (128)

#define CHG_PRECHG_CURRENT_1024MA   (1024)
#define CHG_PRECHG_CURRENT_512MA    (512)
#define CHG_PRECHG_CURRENT_256MA    (256)
#define CHG_PRECHG_CURRENT_128MA    (128)

#define CHG_BAT_VREG_512MV_B7       (512)
#define CHG_BAT_VREG_256MV_B6       (256)
#define CHG_BAT_VREG_128MV_B5       (128)
#define CHG_BAT_VREG_64MV_B4        (64)
#define CHG_BAT_VREG_32MV_B3        (32)
#define CHG_BAT_VREG_16MV_B2        (16)

#define CHG_VINDPM_640MV_B6         (640)
#define CHG_VINDPM_320MV_B5         (320)
#define CHG_VINDPM_160MV_B4         (160)
#define CHG_VINDPM_80MV_B3          (80)


#define CHG_BOOSTV_OFFSET_512MV     (512)
#define CHG_BOOSTV_OFFSET_256MV     (256)
#define CHG_BOOSTV_OFFSET_128MV     (128)
#define CHG_BOOSTV_OFFSET_64MV      (64)

#define CHG_BCOLD_THOLD_MINUS20DEGC (-20)
#define CHG_BCOLD_THOLD_MINUS10DEGC (-10)
#define CHG_BHOT_THOLD_PLUS55DEGC   (55)
#define CHG_BHOT_THOLD_PLUS60DEGC   (60)
#define CHG_BHOT_THOLD_PLUS65DEGC   (65)

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

#define BQ24296_IINLIMIT_1000       (0x4 << BQ24192_IINLIMIT_BITPOS)
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

#define BQ24296_BOOST_LIM_1000MA    (0x0 << BQ24192_BOOST_LIM_BITPOS)
#define BQ24296_BOOST_LIM_1500MA    (0x1 << BQ24192_BOOST_LIM_BITPOS)
#define BQ24192_BOOST_LIM_MASK      (0x1 << BQ24192_BOOST_LIM_BITPOS)


/****************0x02 CONTROL Register (Read/Write)****************************/
/*Memory Location: 02, Reset State: 1000 1100*/
#define BQ24192_CHG_CUR_CTL_REG      0x02
/*All the 8bits and relevant mask value*/
#define BQ24192_ICHARGE_5_BITPOS     0x7
#define BQ24192_ICHARGE_2048        (0x1 << BQ24192_ICHARGE_5_BITPOS)

#define BQ24192_ICHARGE_4_BITPOS     0x6
#define BQ24192_ICHARGE_1024        (0x1 << BQ24192_ICHARGE_4_BITPOS)

#define BQ24192_ICHARGE_3_BITPOS     0x5
#define BQ24192_ICHARGE_512         (0x1 << BQ24192_ICHARGE_3_BITPOS)

#define BQ24192_ICHARGE_2_BITPOS     0x4
#define BQ24192_ICHARGE_256         (0x1 << BQ24192_ICHARGE_2_BITPOS)

#define BQ24192_ICHARGE_1_BITPOS     0x3
#define BQ24192_ICHARGE_128         (0x1 << BQ24192_ICHARGE_1_BITPOS)

#define BQ24192_ICHARGE_0_BITPOS     0x2
#define BQ24192_ICHARGE_64          (0x1 << BQ24192_ICHARGE_0_BITPOS)
#define BQ24192_ICHARGE_OFFSET       512
#define BQ24192_ICHARGE_MAX          4544
#define BQ24192_ICHARGE_MASK        (0x1F << BQ24192_ICHARGE_0_BITPOS)

#define BQ24296_BCOLD_BITPOS        0x1
#define BQ24296_BCOLD_MASK          (0x1 << BQ24296_BCOLD_BITPOS)
#define BQ24296_BCOLD0_N10DEGC      (0x0 << BQ24296_BCOLD_BITPOS)
#define BQ24296_BCOLD1_N20DEGC      (0x1 << BQ24296_BCOLD_BITPOS)

#define BQ24192_FORCE_20PCT_BITPOS   0x0
#define BQ24192_FORCE_20PCT_EN      (0x1 << BQ24192_FORCE_20PCT_BITPOS)
#define BQ24192_FORCE_20PCT_DIS     (0x0 << BQ24192_FORCE_20PCT_BITPOS)
#define BQ24192_FORCE_20PCT_MASK    (0x1 << BQ24192_FORCE_20PCT_BITPOS)


/***************0x03 BAT_VOL/CTL Register (Read/Write)*************************/
/*Memory Location: 03, Reset State: 0001 0100*/
#define BQ24192_IPRECHG_TERM_CTL_REG 0x03
/*All the 8bits and relevant mask value*/
#define BQ24192_IPRECHG_3_BITPOS     0x7
#define BQ24192_IPRECHG_1024        (0x1 << BQ24192_IPRECHG_3_BITPOS)

#define BQ24192_IPRECHG_2_BITPOS     0x6
#define BQ24192_IPRECHG_512         (0x1 << BQ24192_IPRECHG_2_BITPOS)

#define BQ24192_IPRECHG_1_BITPOS     0x5
#define BQ24192_IPRECHG_256         (0x1 << BQ24192_IPRECHG_1_BITPOS)

#define BQ24192_IPRECHG_0_BITPOS     0x4
#define BQ24192_IPRECHG_128         (0x1 << BQ24192_IPRECHG_0_BITPOS)
#define BQ24192_IPRECHG_OFFSET       128
#define BQ24192_IPRECHG_MAX          2048
#define BQ24192_IPRECHG_MASK        (0xF << BQ24192_IPRECHG_0_BITPOS)

#define BQ24192_ITERM_3_BITPOS       0x3
#define BQ24192_ITERM_1024          (0x1 << BQ24192_ITERM_3_BITPOS)

#define BQ24192_ITERM_2_BITPOS       0x2
#define BQ24192_ITERM_512           (0x1 << BQ24192_ITERM_2_BITPOS)

#define BQ24192_ITERM_1_BITPOS       0x1
#define BQ24192_ITERM_256           (0x1 << BQ24192_ITERM_1_BITPOS)

#define BQ24192_ITERM_0_BITPOS       0x0
#define BQ24192_ITERM_128           (0x1 << BQ24192_ITERM_0_BITPOS)
#define BQ24192_ITERM_OFFSET         128
#define BQ24192_ITERM_MAX            2048
#define BQ24192_ITERM_MASK          (0xF << BQ24192_ITERM_0_BITPOS)


/*******************0x04 Vendor Register (Read only)***************************/
/*Memory Location: 04, Reset State: 0100 0000*/
#define BQ24192_CHG_VOLT_CTL_REG     0x04
/*All the 8bits and relevant mask value*/
#define BQ24192_BAT_REG_VOL_5_BITPOS 0x7
#define BQ24192_BAT_REG_VOL_512     (0x1 << BQ24192_BAT_REG_VOL_5_BITPOS)

#define BQ24192_BAT_REG_VOL_4_BITPOS 0x6
#define BQ24192_BAT_REG_VOL_256     (0x1 << BQ24192_BAT_REG_VOL_4_BITPOS)

#define BQ24192_BAT_REG_VOL_3_BITPOS 0x5
#define BQ24192_BAT_REG_VOL_128     (0x1 << BQ24192_BAT_REG_VOL_3_BITPOS)

#define BQ24192_BAT_REG_VOL_2_BITPOS 0x4
#define BQ24192_BAT_REG_VOL_64      (0x1 << BQ24192_BAT_REG_VOL_2_BITPOS)

#define BQ24192_BAT_REG_VOL_1_BITPOS 0x3
#define BQ24192_BAT_REG_VOL_32      (0x1 << BQ24192_BAT_REG_VOL_1_BITPOS)

#define BQ24192_BAT_REG_VOL_0_BITPOS 0x2
#define BQ24192_BAT_REG_VOL_16      (0x1 << BQ24192_BAT_REG_VOL_0_BITPOS)
#define BQ24192_BAT_REG_VOL_OFFSET   3504
#define BQ24192_BAT_REG_VOL_MAX      4400
#define BQ24192_BAT_REG_VOL_MASK    (0x3F << BQ24192_BAT_REG_VOL_0_BITPOS)

#define BQ24192_BATLOW_VOL_BITPOS    0x1
#define BQ24192_BATLOW_VOL          (0x1 << BQ24192_BATLOW_VOL_BITPOS)
#define BQ24192_BATLOW_VOL_MASK     (0x1 << BQ24192_BATLOW_VOL_BITPOS)

#define BQ24192_RECHG_VOL_BITPOS     0x0
#define BQ24192_RECHG_VOL           (0x1 << BQ24192_RECHG_VOL_BITPOS)
#define BQ24192_RECHG_VOL_MASK      (0x1 << BQ24192_RECHG_VOL_BITPOS)


/*****0x05 BATTERY_CUTOFF_CURRENT/FAST_CHG_CURRENT Register (Read / Write)*****/
/*Memory Location: 05, Reset State: 0011 0010*/
#define BQ24192_TERM_TIMER_CTL_REG   0x05
/*All the 8bits and relevant mask value*/
#define BQ24192_TERM_CUR_BITPOS      0x7
#define BQ24192_FBD_TERM_CUR        (0x0 << BQ24192_TERM_CUR_BITPOS)
#define BQ24192_EN_TERM_CUR         (0x1 << BQ24192_TERM_CUR_BITPOS)
#define BQ24192_TERM_CUR_MASK       (0x1 << BQ24192_TERM_CUR_BITPOS)

#define BQ24192_TERM_STAT_BITPOS     0x6
#define BQ24192_EN_MATCH_ITERM      (0x0 << BQ24192_TERM_STAT_BITPOS)
#define BQ24192_DIS_MATCH_ITERM     (0x1 << BQ24192_TERM_STAT_BITPOS)
#define BQ24192_MATCH_ITERM_MASK    (0x1 << BQ24192_TERM_STAT_BITPOS)

#define BQ24192_WATCHDOG_TMR_BITPOS  0x4
#define BQ24192_WATCHDOG_TMR_DIS    (0x0 << BQ24192_WATCHDOG_TMR_BITPOS)
#define BQ24192_WATCHDOG_TMR_40     (0x1 << BQ24192_WATCHDOG_TMR_BITPOS)
#define BQ24192_WATCHDOG_TMR_80     (0x2 << BQ24192_WATCHDOG_TMR_BITPOS)
#define BQ24192_WATCHDOG_TMR_160    (0x3 << BQ24192_WATCHDOG_TMR_BITPOS)
#define BQ24192_WATCHDOG_TMR_MASK   (0x3 << BQ24192_WATCHDOG_TMR_BITPOS)

#define BQ24192_CHG_TIMER_EN_BITPOS  0x3
#define BQ24192_CHG_TIMER_EN        (0x1 << BQ24192_CHG_TIMER_EN_BITPOS)
#define BQ24192_CHG_TIMER_DIS       (0x0 << BQ24192_CHG_TIMER_EN_BITPOS)
#define BQ24192_CHG_TIMER_EN_MASK   (0x1 << BQ24192_CHG_TIMER_EN_BITPOS)

#define BQ24192_CHG_TIMER_BITPOS     0x1
#define BQ24192_CHG_TIMER_5HRS      (0x0 << BQ24192_CHG_TIMER_BITPOS)
#define BQ24192_CHG_TIMER_8HRS      (0x1 << BQ24192_CHG_TIMER_BITPOS)
#define BQ24192_CHG_TIMER_12HRS     (0x2 << BQ24192_CHG_TIMER_BITPOS)
#define BQ24192_CHG_TIMER_20HRS     (0x3 << BQ24192_CHG_TIMER_BITPOS)
#define BQ24192_CHG_TIMER_MASK      (0x3 << BQ24192_CHG_TIMER_BITPOS)


/************0x06 DPM STATUS Register (Read / Write)***************************/
/*Memory Location: 06, Reset State: xx00 0000*/
#define BQ24192_IRCOMP_THERM_CTL_REG 0x06
/*All the 8bits and relevant mask value*/
#define BQ24192_IR_BATCOMP_2_BITPOS  0x7
#define BQ24192_IR_BATCOMP_40MOHM   (0x1 << BQ24192_IR_BATCOMP_2_BITPOS)

#define BQ24192_IR_BATCOMP_1_BITPOS  0x6
#define BQ24192_IR_BATCOMP_20MOHM   (0x1 << BQ24192_IR_BATCOMP_1_BITPOS)

#define BQ24192_IR_BATCOMP_0_BITPOS  0x5
#define BQ24192_IR_BATCOMP_10MOHM   (0x1 << BQ24192_IR_BATCOMP_0_BITPOS)
#define BQ24192_IR_BATCOMP_OFFSET    0
#define BQ24192_IR_BATCOMP_MASK     (0x7 << BQ24192_IR_BATCOMP_0_BITPOS)

#define BQ24192_IR_VCLAMP_2_BITPOS   0x4
#define BQ24192_IR_VCLAMP_64        (0x1 << BQ24192_IR_VCLAMP_2_BITPOS)

#define BQ24192_IR_VCLAMP_1_BITPOS   0x3
#define BQ24192_IR_VCLAMP_32        (0x1 << BQ24192_IR_VCLAMP_1_BITPOS)

#define BQ24192_IR_VCLAMP_0_BITPOS   0x2
#define BQ24192_IR_VCLAMP_16        (0x1 << BQ24192_IR_VCLAMP_0_BITPOS)
#define BQ24192_IR_VCLAMP_OFFSET     0
#define BQ24192_IR_VCLAMP_MASK      (0x7 << BQ24192_IR_VCLAMP_0_BITPOS)

#define BQ24192_TERM_REG_BITPOS      0x0
#define BQ24192_TERM_REG_60         (0x0 << BQ24192_TERM_REG_BITPOS)
#define BQ24192_TERM_REG_80         (0x1 << BQ24192_TERM_REG_BITPOS)
#define BQ24192_TERM_REG_100        (0x2 << BQ24192_TERM_REG_BITPOS)
#define BQ24192_TERM_REG_120        (0x3 << BQ24192_TERM_REG_BITPOS)
#define BQ24192_TERM_REG_MASK       (0x3 << BQ24192_TERM_REG_BITPOS)



/************************0x06 (Read / Write) **********************************/
/************* Boost Voltage/Thermal Regulation Control Register **************/
/*Memory Location: 06, Reset State: OB0111 0011 or 0x73*/
#define BQ24x96_BOOSTV_THERMREG_REG 0x06
/*All the 8bits and relevant mask value*/
#define BQ24296_BOOSTV_BITPOS       0x4
#define BQ24296_BOOSTV_MASK         (0xF << BQ24296_BOOSTV_BITPOS)
#define BQ24296_BOOSTV_512MV        (0x8 << BQ24296_BOOSTV_BITPOS)
#define BQ24296_BOOSTV_256MV        (0x4 << BQ24296_BOOSTV_BITPOS)
#define BQ24296_BOOSTV_128MV        (0x2 << BQ24296_BOOSTV_BITPOS)
#define BQ24296_BOOSTV_64MV         (0x1 << BQ24296_BOOSTV_BITPOS)
#define BQ24296_BOOSTV_OFFSET       4550
#define BQ24296_BOOSTV_MAX          5510

#define BQ24296_BHOT_BITPOS         0x2
#define BQ24296_BHOT_MASK           (0x3 << BQ24296_BHOT_BITPOS)
#define BQ24296_BHOT1_55DEGC        (0x0 << BQ24296_BHOT_BITPOS)
#define BQ24296_BHOT0_60DEGC        (0x1 << BQ24296_BHOT_BITPOS)
#define BQ24296_BHOT2_65DEGC        (0x2 << BQ24296_BHOT_BITPOS)
#define BQ24296_BHOT_DISABLE        (0x3 << BQ24296_BHOT_BITPOS)

#define BQ24x96_TREG_BITPOS         0x0
#define BQ24x96_TREG_MASK           (0x3 << BQ24x96_TREG_BITPOS)
#define BQ24x96_TREG_60DEGC         (0x0 << BQ24x96_TREG_BITPOS)
#define BQ24x96_TREG_80DEGC         (0x1 << BQ24x96_TREG_BITPOS)
#define BQ24x96_TREG_100DEGC        (0x2 << BQ24x96_TREG_BITPOS)
#define BQ24x96_TREG_120DEGC        (0x3 << BQ24x96_TREG_BITPOS)
/*********0x07 Safety Timer / NTC Monitor Register (Read / Write)**************/
/*Memory Location: 07, Reset State: 1001 1xxx*/
#define BQ24192_MISC_OP_CTL_REG      0x07
/*All the 8bits and relevant mask value*/
#define BQ24192_DPDM_EN_BITPOS       0x7
#define BQ24192_DPDM_EN             (0x1 << BQ24192_DPDM_EN_BITPOS)
#define BQ24192_DPDM_DIS            (0x0 << BQ24192_DPDM_EN_BITPOS)
#define BQ24192_DPDM_MASK           (0x1 << BQ24192_DPDM_EN_BITPOS)

#define BQ24192_TMR2X_EN_BITPOS      0x6
#define BQ24192_TMR2X_EN            (0x1 << BQ24192_TMR2X_EN_BITPOS)
#define BQ24192_TMR2X_DIS           (0x0 << BQ24192_TMR2X_EN_BITPOS)
#define BQ24192_TMR2X_MASK          (0x1 << BQ24192_TMR2X_EN_BITPOS)

#define BQ24192_BATFET_OFF_BITPOS    0x5
#define BQ24192_BATFET_OFF_EN       (0x1 << BQ24192_BATFET_OFF_BITPOS)
#define BQ24192_BATFET_OFF_DIS      (0x0 << BQ24192_BATFET_OFF_BITPOS)
#define BQ24192_BATFET_OFF_MASK     (0x1 << BQ24192_BATFET_OFF_BITPOS)

#define BQ24192_INT_ON_CHGFLT_BITPOS 0x1
#define BQ24192_INT_ON_CHGFLT_EN    (0x1 << BQ24192_INT_ON_CHGFLT_BITPOS)
#define BQ24192_INT_ON_CHGFLT_DIS   (0x0 << BQ24192_INT_ON_CHGFLT_BITPOS)
#define BQ24192_INT_ON_CHGFLT_MASK  (0x1 << BQ24192_INT_ON_CHGFLT_BITPOS)

#define BQ24192_INT_ON_BATFLT_BITPOS 0x0
#define BQ24192_INT_ON_BATFLT_EN    (0x1 << BQ24192_INT_ON_BATFLT_BITPOS)
#define BQ24192_INT_ON_BATFLT_DIS   (0x0 << BQ24192_INT_ON_BATFLT_BITPOS)
#define BQ24192_INT_ON_BATFLT_MASK  (0x1 << BQ24192_INT_ON_BATFLT_BITPOS)


/*********0x08 System State Register Register (Read Only)**************/
/*Memory Location: 08, Reset State: xxxx xxxx*/
#define BQ24192_SYS_STAT_REG         0x08
/*All the 8bits and relevant mask value*/
#define BQ24192_VBUS_STAT_BITPOS     0x6
#define BQ24192_VBUS_STAT_UNKNOWN   (0x0 << BQ24192_VBUS_STAT_BITPOS)
#define BQ24192_VBUS_STAT_USB       (0x1 << BQ24192_VBUS_STAT_BITPOS)
#define BQ24192_VBUS_STAT_ADAPTER   (0x2 << BQ24192_VBUS_STAT_BITPOS)
#define BQ24192_VBUS_STAT_OTG       (0x3 << BQ24192_VBUS_STAT_BITPOS)
#define BQ24192_VBUS_STAT_MASK      (0x3 << BQ24192_VBUS_STAT_BITPOS)

#define BQ24192_CHRG_STAT_BITPOS     0x4
#define BQ24192_CHRG_STAT_NOCHG     (0x0 << BQ24192_CHRG_STAT_BITPOS)
#define BQ24192_CHRG_STAT_PRECHG    (0x1 << BQ24192_CHRG_STAT_BITPOS)
#define BQ24192_CHRG_STAT_FASTCHG   (0x2 << BQ24192_CHRG_STAT_BITPOS)
#define BQ24192_CHRG_STAT_CHGDONE   (0x3 << BQ24192_CHRG_STAT_BITPOS)
#define BQ24192_CHRG_STAT_MASK      (0x3 << BQ24192_CHRG_STAT_BITPOS)

#define BQ24192_DPM_STAT_BITPOS      0x3
#define BQ24192_DPM_STAT            (0x1 << BQ24192_DPM_STAT_BITPOS)
#define BQ24192_NODPM_STAT          (0x0 << BQ24192_DPM_STAT_BITPOS)
#define BQ24192_DPM_STAT_MASK       (0x1 << BQ24192_DPM_STAT_BITPOS)

#define BQ24192_PG_STAT_BITPOS       0x2
#define BQ24192_PG_STAT             (0x1 << BQ24192_PG_STAT_BITPOS)
#define BQ24192_NOPG_STAT           (0x0 << BQ24192_PG_STAT_BITPOS)
#define BQ24192_PG_STAT_MASK        (0x1 << BQ24192_PG_STAT_BITPOS)

#define BQ24192_THERM_STAT_BITPOS    0x1
#define BQ24192_THERM_STAT_NORMAL   (0x1 << BQ24192_THERM_STAT_BITPOS)
#define BQ24192_THERM_STAT_THERMAL  (0x0 << BQ24192_THERM_STAT_BITPOS)
#define BQ24192_THERM_STAT_MASK     (0x1 << BQ24192_THERM_STAT_BITPOS)

#define BQ24192_VSYS_STAT_BITPOS     0x0
#define BQ24192_VSYS_STAT_INSYSMIN  (0x1 << BQ24192_VSYS_STAT_BITPOS)
#define BQ24192_VSYS_STAT_NOSYSMIN  (0x0 << BQ24192_VSYS_STAT_BITPOS)
#define BQ24192_VSYS_STAT_MASK      (0x1 << BQ24192_VSYS_STAT_BITPOS)

/*********0x09 Fault Register Register (Read Only)**************/
/*Memory Location: 09, Reset State: xxxx xxxx*/
#define BQ24192_FAULT_REG            0x09
/*All the 8bits and relevant mask value*/
#define BQ24192_WATCHDOG_FLT_BITPOS  0x7
#define BQ24192_WATCHDOG_EXP        (0x1 << BQ24192_WATCHDOG_FLT_BITPOS)
#define BQ24192_WATCHDOG_NORMAL     (0x0 << BQ24192_WATCHDOG_FLT_BITPOS)
#define BQ24192_WATCHDOG_FLT_MASK   (0x1 << BQ24192_WATCHDOG_FLT_BITPOS)

#define BQ24192_BOOST_FLT_BITPOS     0x6
#define BQ24192_BOOST_FLT           (0x1 << BQ24192_BOOST_FLT_BITPOS)
#define BQ24192_BOOST_NORMAL        (0x0 << BQ24192_BOOST_FLT_BITPOS)
#define BQ24192_BOOST_FLT_MASK      (0x1 << BQ24192_BOOST_FLT_BITPOS)

#define BQ24192_CHRG_FLT_BITPOS      0x4
#define BQ24192_CHRG_NORMAL         (0x0 << BQ24192_CHRG_FLT_BITPOS)
#define BQ24192_INPUT_FLT           (0x1 << BQ24192_CHRG_FLT_BITPOS)
#define BQ24192_THERMAL_SHUTDOWM    (0x2 << BQ24192_CHRG_FLT_BITPOS)
#define BQ24192_CHRG_TIMER_EXP      (0x3 << BQ24192_CHRG_FLT_BITPOS)
#define BQ24192_CHRG_FLT_MASK       (0x3 << BQ24192_CHRG_FLT_BITPOS)

#define BQ24192_BAT_FLT_BITPOS       0x3
#define BQ24192_BAT_OVP             (0x1 << BQ24192_BAT_FLT_BITPOS)
#define BQ24192_BAT_NORMAL          (0x0 << BQ24192_BAT_FLT_BITPOS)
#define BQ24192_BAT_FLT_MASK        (0x1 << BQ24192_BAT_FLT_BITPOS)


#define BQ24192_NTC_FLT_BITPOS       0x0
#define BQ24192_NTC_NORMAL          (0x0 << BQ24192_NTC_FLT_BITPOS)
#define BQ24192_TS1_COLD            (0x1 << BQ24192_NTC_FLT_BITPOS)
#define BQ24192_TS1_HOT             (0x2 << BQ24192_NTC_FLT_BITPOS)
#define BQ24192_TS2_COLD            (0x3 << BQ24192_NTC_FLT_BITPOS)
#define BQ24192_TS2_HOT             (0x4 << BQ24192_NTC_FLT_BITPOS)
#define BQ24192_BOTH_COLD           (0x5 << BQ24192_NTC_FLT_BITPOS)
#define BQ24192_BOTH_HOT            (0x6 << BQ24192_NTC_FLT_BITPOS)
#define BQ24192_NTC_FLT_MASK        (0x7 << BQ24192_NTC_FLT_BITPOS)

#define BQ24x96_NTC_NORMAL          (0x0 << BQ24192_NTC_FLT_BITPOS)
#define BQ24196_TS_COLD_VAL         (0x5 << BQ24192_NTC_FLT_BITPOS)
#define BQ24196_TS_HOT_VAL          (0x6 << BQ24192_NTC_FLT_BITPOS)
#define BQ24296_TS_HOT_VAL          (0x1 << BQ24192_NTC_FLT_BITPOS)
#define BQ24296_TS_COLD_VAL         (0x2 << BQ24192_NTC_FLT_BITPOS)
#define BQ24x96_NTC_FLT_MASK        (0x7 << BQ24192_NTC_FLT_BITPOS)



/*********0x0A Vender Register Register (Read Only)**************/
/*Memory Location: 0A, Fix State: 0010 1011*/
#define BQ24x96_REVISION_REG        0x0A
/*BQ24296/BQ24297 Vendor/Part/Revision Reg Info.*/
#define BQ2429x_PART_NR_BITPOS      0x5
#define BQ2429x_PART_NR_MASK        (0x7 << BQ2429x_PART_NR_BITPOS)
#define BQ24296_PART_NR_VALUE       (0x1 << BQ2429x_PART_NR_BITPOS)
#define BQ24297_PART_NR_VALUE       (0x3 << BQ2429x_PART_NR_BITPOS)

#define BQ2429x_REVISION_BITPOS     0x0
#define BQ2429x_REVISION_MASK       (0x7 << BQ2429x_REVISION_BITPOS)
#define BQ2429x_REVISION_1P0        (0x0 << BQ2429x_REVISION_BITPOS)
#define BQ2429x_REVISION_1P1        (0x1 << BQ2429x_REVISION_BITPOS)

/*BQ24296/BQ24297 Vendor/Part/Revision Reg Info.*/
#define BQ2419x_PART_NR_BITPOS      0x3
#define BQ2419x_PART_NR_MASK        (0x7 << BQ2419x_PART_NR_BITPOS)
#define BQ24190_PART_NR_VALUE       (0x4 << BQ2419x_PART_NR_BITPOS)
#define BQ24196_PART_NR_VALUE       (0x5 << BQ2419x_PART_NR_BITPOS)

#define BQ2419x_REVISION_BITPOS     0x0
#define BQ2419x_REVISION_MASK       (0x7 << BQ2419x_REVISION_BITPOS)
#define BQ2419x_REVISION_VAL        (0x3 << BQ2419x_REVISION_BITPOS)

/*****************************�ڲ��ṹ�嶨��********************************/
typedef enum
{
    CHG_STAT_UNKNOWN,
    CHG_STAT_USB_HOST,
    CHG_STAT_ADAPTER_PORT,
    CHG_STAT_OTG
}chg_dcdc_vbus_stat;

typedef enum
{
    CHG_STAT_NOT_CHARGING,
    CHG_STAT_PRE_CHARGING,
    CHG_STAT_FAST_CHARGING,
    CHG_STAT_CHARGE_DONE
}chg_dcdc_chrg_stat;

typedef enum
{
    CHG_WATCHDOG_NORMAL,
    CHG_WATCHDOG_EXP
}chg_dcdc_watchdog_fault;

typedef enum
{
    CHG_FAULT_NORMAL,
    CHG_FAULT_INPUT_FLT,
    CHG_FAULT_THERMAL_SHUTDOWN,
    CHG_FAULT_SAFTY_TIMER_EXP
}chg_dcdc_chrg_fault;

typedef enum
{
    CHG_BAT_NORMAL,
    CHG_BAT_OVP
}chg_dcdc_bat_fault;

/*���оƬ���״̬�����ݽṹ���Ͷ���*/
typedef enum
{
    CHG_BATT_NORMAL,
    CHG_BATT_OVP,
    CHG_BATT_ABSENT,
    CHG_BATT_INVALID
}chg_dcdc_batt_status;

/*���CE״̬*/
typedef enum
{
    CHG_CONFIG_CHG_DIS,
    CHG_CONFIG_CHG_BAT,
    CHG_CONFIG_CHG_OTG,
}chg_dcdc_ce_status;

/*���оƬ����״̬�����ݽṹ���Ͷ���*/
typedef struct
{
    chg_dcdc_vbus_stat          chg_vbus_stat;
    chg_dcdc_chrg_stat          chg_chrg_stat;
    chg_dcdc_watchdog_fault     chg_watchdog_fault;
    chg_dcdc_chrg_fault         chg_chrg_fault;
    chg_dcdc_bat_fault          chg_bat_fault;
    chg_dcdc_ce_status          chg_ce_stat;
}chg_status_type;

/*���оƬ������ص����ݽṹ���Ͷ���*/
typedef enum
{
    LOW_CTRL_VAL,
    HIGH_CTRL_VAL
}
chg_charge_control_value;

typedef enum
{
    CHG_5HOUR_SAFETY_TIMER,
    CHG_8HOUR_SAFETY_TIMER,
    CHG_12HOUR_SAFETY_TIMER,
    CHG_20HOUR_SAFETY_TIMER,
    CHG_DISABLE_SAFETY_TIMER
}
chg_safety_timer_value;

typedef enum
{
    CHG_DISABLE_WATCHDOG_TIMER,
    CHG_40S_WATCHDOG_TIMER,
    CHG_80S_WATCHDOG_TIMER,
    CHG_160S_WATCHDOG_TIMER
}
chg_watchdog_timer_value;

typedef enum
{
    CHG_STOP_COMPLETE,
    CHG_STOP_TIMEOUT,
    CHG_STOP_INVALID_TEMP,
    CHG_STOP_BY_SW,
    CHG_STOP_SUPPLY_ERR,
    CHG_STOP_BAT_ERR,
    CHG_STOP_OTHER_REASON
}
chg_stop_reason;

typedef enum
{
    CHG_SUPPLY_PREFER_IN,
    CHG_SUPPLY_PREFER_USB
}
chg_supply_prefer_value;


typedef enum
{
    CHG_CHIP_PN_BQ24196,
    CHG_CHIP_PN_BQ24296,
    CHG_CHIP_PN_INVALID
}
chg_chip_part_nr;
/******************************************************************************
Function:       chg_set_cur_level
Description:    �趨���оƬ�Ե�صĳ�����
Input:          ������ֵ
Output:         N/A
Return:         TRUE     : ����ִ�гɹ�
                FALSE    : ����ִ��ʧ��
Others:         N/A
******************************************************************************/
boolean chg_set_cur_level(uint32_t chg_cur);

/******************************************************************************
  Function      chg_set_supply_limit
  Description   �趨�����������
  Input         �������ֵ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_supply_limit(uint32_t lmt_val);

/******************************************************************************
Function:       chg_get_supply_limit
Description:    ��ȡ���оƬǰ������
Input:          ������ֵ
Output:         N/A
Return:         TRUE     : ����ִ�гɹ�
                FALSE    : ����ִ��ʧ��
Others:         N/A
******************************************************************************/
uint8_t chg_get_supply_limit(void);
/******************************************************************************
Function:       chg_set_current_level
Description:    �趨���оƬ�Ե�صĳ�����,����BQ24296M����ѹ�������
Input:          1��chg_cur��������ֵ
                2�������Ƿ�ʹ���趨���оƬ�Ե�صĳ�������20%���
Output:         N/A
Return:         TRUE     : ����ִ�гɹ�
                FALSE    : ����ִ��ʧ��
Others:         N/A
******************************************************************************/
boolean chg_set_current_level(uint32_t chg_cur,boolean is_20pct);

/******************************************************************************
  Function      chg_set_term_current
  Description   ���ó���ֹ����
  Input         ��ֹ����ֵ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_term_current(uint32_t term_val);

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
boolean chg_set_charge_enable(boolean enable);

/******************************************************************************
  Function      chg_set_vreg_level
  Description   ���ú�ѹ����ѹ
  Input         ����ѹ��ѹֵ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_vreg_level(uint32_t vreg_val);

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
boolean chg_set_suspend_mode(boolean enable);

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
boolean chg_set_supply_prefer(chg_supply_prefer_value sup_val);

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
boolean chg_set_te_enable(boolean enable);

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
boolean chg_set_ts_enable(boolean enable);

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
boolean chg_set_charge_mode(boolean enable);

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
boolean chg_set_stat_enable(boolean enable);

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
boolean chg_set_dpm_val(uint32_t dpm_val);

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
boolean chg_set_safety_timer_enable(chg_safety_timer_value tmr_val);

/******************************************************************************
  Function      chg_set_tmr_rst
  Description   �Գ��оƬִ���߹�����
  Input         N/A
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_tmr_rst(void);

/******************************************************************************
  Function      chg_reset_all_reg
  Description   ��������BQ24192�Ĵ�����Ĭ������
  Input         N/A
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_reset_all_reg(void);

/******************************************************************************
  Function      chg_get_suspend_status
  Description   ��ȡ���IC״̬���Ƿ���suspendģʽ
  Input         N/A
  Output
  Return        TRUE      : suspendģʽ
                FALSE     : ��suspendģʽ
  Others        N/A
******************************************************************************/
boolean chg_get_suspend_status(void);

/******************************************************************************
  Function      chg_get_IC_status
  Description   ��ȡ���IC״̬����ȡ���оƬ���мĴ�����ֵ����ʶ�����е�ֻ��
                ����λ��д��chg_status_type�ṹ���У��ýṹ�����������г��оƬ
                ״̬
  Input         N/A
  Output        chg_status_type: �ýṹ�У�
                chg_dcdc_status           ������������״̬��
                chg_dcdc_fault_status     ��������Ƿ���ִ���
                chg_dcdc_in_supply_status ����IN����Դ�Ƿ�������
                chg_dcdc_usb_supply_status����USB����Դ�Ƿ�������
                chg_dcdc_batt_status      ��������Ƿ�����
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_get_IC_status(chg_status_type *chg_stat_ptr);

/******************************************************************************
  Function      chg_is_charger_present
  Description   ���Դ�Ƿ���λ
  Input         N/A
  Output        N/A
  Return        FALSE     : ���Դ����λ
                TRUE      : ���Դ��λ
  Others        N/A
******************************************************************************/
extern boolean chg_is_charger_present(void);

/******************************************************************************
  Function      chg_is_IC_charging
  Description   ���IC�Ƿ��ڳ��
  Input         N/A
  Output        N/A
  Return        TRUE     : ���ڳ��
                FALSE    : ֹͣ���
  Others        N/A
******************************************************************************/
boolean chg_is_IC_charging(void);

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
chg_stop_reason chg_get_stop_charging_reason(void);

/******************************************************************************
  Function      chg_set_chip_cd
  Description   ����оƬƬѡʹ��
  Input         ����оƬ��Ӧ��CD��GPIO��ʾʹ��оƬ�����߱�ʾ�ر�оƬ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
void chg_set_chip_cd(boolean enable);


/******************************************************************************
  Function      chg_set_boost_therm_protect_threshold
  Description   ���ó��оƬBOOSTģʽ���¶ȱ�������ֵ
  Input         temp      : �����õ�����ֵ�¶�, 0 ��ʾ���ñ������ܡ�
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        �ýӿ�Ŀǰ��BQ24296оƬ֧�֣�����оƬֱ�ӷ���TRUE.
******************************************************************************/
boolean chg_set_boost_therm_protect_threshold(int32_t temp);

/******************************************************************************
  Function      chg_set_boost_volt
  Description   ���ó��оƬBOOSTģʽ��ѹֵ
  Input         �����õ�Boostģʽ��ѹֵ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        �ýӿ�Ŀǰ��BQ24296оƬ֧�֣�����оƬ����TRUE.
******************************************************************************/
boolean chg_set_boost_volt(uint32_t boostv);

/******************************************************************************
  Function      chg_get_part_nr
  Description   ��ȡ���оƬ���ͺţ�Ŀǰ֧��bq24196��bq24296
  Input         N/A
  Output        pn        : ���ص�оƬ�ͺ�
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_get_part_nr(chg_chip_part_nr *pn);

/******************************************************************************
  Function      boolean chg_chip_init
  Description   ���оƬ��ʼ��
  Input         N/A
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_chip_init(void);

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
void chg_charger_event_api(boolean charger_status);

/******************************************************************************
  Function      chg_dump_ic_hwinfo
  Description   Dump���IC���е�Ӳ�������Ϣ�������Ĵ�������PIN��״̬��
  Input         N/A
  Output        N/A
  Return        N/A
  Others        N/A
******************************************************************************/
boolean chg_dump_ic_hwinfo(void);

#if (MBB_CHG_WIRELESS == FEATURE_ON)

void chg_set_wireless_chg_enable(boolean enable);
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/

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
boolean chg_extchg_ocp_detect(void);
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
boolean chg_set_charge_otg_enable(boolean enable);


void chg_set_extchg_chg_enable(boolean enable);

#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/

#endif
