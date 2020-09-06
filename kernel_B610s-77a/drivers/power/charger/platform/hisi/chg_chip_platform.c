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
#include <bsp_hkadc.h>
#include <bsp_nvim.h>
#include "bsp_sram.h"
#include "power_com.h"
#ifdef CONFIG_COUL
#include <bsp_coul.h>
#endif
#include <product_config.h>

#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
#include <linux/huawei_feature.h>
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/
#include "chg_chip_platform.h"

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern struct i2c_client *g_i2c_client;
extern struct chargeIC_chip *g_chip;
extern int chg_en_flag;
#if (MBB_CHG_EXTCHG == FEATURE_ON)

extern int32_t g_extchg_diable_st;
extern boolean g_exchg_online_flag;
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/

#if ((MBB_CHG_EXTCHG == FEATURE_ON) || (MBB_CHG_WIRELESS == FEATURE_ON))
#ifndef USB_OTG_ID_PULL_OUT
/*直连基带，拉高HS_ID*/
#define USB_OTG_ID_PULL_OUT      (0x0003)
#endif/*USB_OTG_ID_PULL_OUT*/
extern void usb_notify_event(unsigned long val, void *v);
#endif/*(MBB_CHG_EXTCHG == FEATURE_ON) || (MBB_CHG_WIRELESS == FEATURE_ON)*/

#if (MBB_CHG_WIRELESS == FEATURE_ON)

extern boolean g_wireless_online_flag;
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
/*硬件产品版本号*/
uint32_t g_hardware_version_id = 0;


/*用于保存NV中读取的电池电压校准最大端*/
int32_t g_vbatt_max = 0;
/*用于保存NV中读取的电池电压校准最小端*/
int32_t g_vbatt_min = 0;
/*用于保存是否进行了校准初始化的标志*/
boolean g_is_batt_volt_calib_init = FALSE;

#if (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
/*电池电压读取通道*/
#define VBAT_DETECT             P_MUX6_1_1
/*VBUS电压读取通道,未定义*/
#define VBUS_DETECT             0xFF
/*电池温度读取通道*/
#define BATTEMP_DETECT          P_MUX4_1_1
/*电池ID电压读取通道*/
#define BAT_ID_DETECT           LR_MUX2_BAT_ID
/*VPH_PWR电压检测*/
#define VPH_PWR_DETECT          LR_MUX4_AMUX_THM1


/*电池电压ADC采集分压值*/
#define BAT_VOL_MUIT_NUMBER        804/200
/*VPH_PWR电压ADC采集分压值*/
#define VPH_PWR_VOL_MUIT_NUMBER    804/200

/* notes: according to < ADC_PARAMETER_ENUM(adc_driver.h)  and CHG_PARAMETER_ENUM(chg_config.h) > */
typedef struct
{
    char *chg_ch_name;
    CHG_PARAMETER_ENUM chg_ch_typ;
    ADC_PARAMETER_ENUM adc_ch_sel;
}chg_adc_adapter;

chg_adc_adapter chg_adc_adapter_tbl[] =
{
    {"VBAT_VOLT", CHG_PARAMETER__BATTERY_VOLT,         ADC_PARAMETER__BATTERY_VOLT     },/* 电池电压 */
    {"VBAT_TEMP", CHG_PARAMETER__BATT_THERM_DEGC,      ADC_PARAMETER__BATT_THERM_DEGC  },/* 电池NTC温度对应电压 */
    {"VBUS_VOLT", CHG_PARAMETER__VBUS_VOLT,            ADC_PARAMETER__VBUS_VOLT        },/* 对外充电用于检测VBUS电压*/
    {"BOT_TEMP",  CHG_PARAMETER__BATT_BOT_THERM_DEGC,  ADC_PARAMETER__BOTT_THERM_DEGC  },/* 板级温度对应电池电压*/
    {"VBAT_ID",   CHG_PARAMETER__BATTERY_ID_VOLT,      ADC_PARAMETER__BATTERY_ID_VOLT  },/* 电池ID电压用于电池型号识别*/
    {"VPH_PWR",   CHG_PARAMETER__VPH_PWR_VOLT,         ADC_PARAMETER__VPH_PWR_VOLT     },/* VPH_PWR电压*/
};
/* end of notes: according to < ADC_PARAMETER_ENUM(adc_driver.h) / CHG_PARAMETER_ENUM(chg_config.h) > */
#endif/*#if (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)*/

#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
#define CHG_BATT_THERM_HKADC_ID    CHG_BATT_TEMP_CHAN/*电池温度读取通道*/
#define CHG_VBUS_VOLT_HKADC_ID     CHG_VBUS_VOLT_CHAN/*VBUS电压读取通道*/
#define CHG_BATT_VOLT_HKADC_ID     CHG_BATT_VOLT_CHAN/*电池电压读取通道*/
#define CHG_BATT_ID_VOLT_HKADC_ID     CHG_BATT_ID_CHAN/*电池电压读取通道*/
/*USB温保*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
#define CHG_USB_TEMP_HKADC_ID      CHG_USB_TEMP_CHAN /*USB温度读取通道*/
#endif

/*电池电压ADC采集分压值*/
#define BAT_VOL_MUIT_NUMBER        800 /330
#endif/*MBB_CHG_PLATFORM_BALONG == FEATURE_ON*/


/*该表由硬件提供*/
/* 电池内部NTC实际温度校准表，9x25平台该表的值在ADC模块修改*/
#if defined(BSP_CONFIG_BOARD_E5)
#if defined(BSP_CONFIG_BOARD_E5_DC04)
const  CHG_TEMP_ADC_TYPE  g_adc_batt_therm_map[] =
{
    {-30,      1702},   /*vol to temp*/
    {-29,      1696},   /*vol to temp*/
    {-28,      1690},   /*vol to temp*/
    {-27,      1683},   /*vol to temp*/
    {-26,      1676},   /*vol to temp*/
    {-25,      1669},   /*vol to temp*/
    {-24,      1662},   /*vol to temp*/
    {-23,      1654},   /*vol to temp*/
    {-22,      1646},   /*vol to temp*/
    {-27,      1638},   /*vol to temp*/
    {-26,      1629},   /*vol to temp*/
    {-25,      1620},   /*vol to temp*/
    {-24,      1610},   /*vol to temp*/
    {-22,      1601},   /*vol to temp*/
    {-21,      1590},   /*vol to temp*/
    {-20,      1580},   /*vol to temp*/
    {-19,      1569},   /*vol to temp*/
    {-18,      1558},   /*vol to temp*/
    {-17,      1546},   /*vol to temp*/
    {-16,      1534},   /*vol to temp*/
    {-15,      1521},    /*vol to temp*/
    {-14,      1508},    /*vol to temp*/
    {-12,      1495},    /*vol to temp*/
    {-11,      1481},    /*vol to temp*/
    {-10,      1467},    /*vol to temp*/
    {-9,       1453},    /*vol to temp*/
    {-8,       1438},    /*vol to temp*/
    {-7,       1423},    /*vol to temp*/
    {-5,       1407},    /*vol to temp*/
    {-4,       1391},    /*vol to temp*/
    {-3,       1375},    /*vol to temp*/
    {-3,       1358},    /*vol to temp*/
    {-2,       1341},    /*vol to temp*/
    {-1,       1324},    /*vol to temp*/
    {0,        1306},    /*vol to temp*/
    {1,        1289},    /*vol to temp*/
    {3,        1270},    /*vol to temp*/
    {4,        1252},    /*vol to temp*/
    {5,        1233},    /*vol to temp*/
    {6,        1215},    /*vol to temp*/
    {7,        1196},    /*vol to temp*/
    {8,        1176},    /*vol to temp*/
    {9,        1157},    /*vol to temp*/
    {10,       1138},    /*vol to temp*/
    {11,       1118},    /*vol to temp*/
    {12,       1098},    /*vol to temp*/
    {13,       1078},    /*vol to temp*/
    {14,       1059},    /*vol to temp*/
    {15,       1039},    /*vol to temp*/
    {16,       1019},    /*vol to temp*/
    {17,       999},    /*vol to temp*/
    {18,       979},    /*vol to temp*/
    {19,       959},    /*vol to temp*/
    {20,       939},    /*vol to temp*/
    {21,       920},    /*vol to temp*/
    {22,       900},    /*vol to temp*/
    {23,       881},    /*vol to temp*/
    {24,       861},    /*vol to temp*/
    {25,       842},    /*vol to temp*/
    {26,       823},    /*vol to temp*/
    {27,       804},    /*vol to temp*/
    {28,       785},    /*vol to temp*/
    {29,       767},    /*vol to temp*/
    {35,       749},    /*vol to temp*/
    {36,       731},    /*vol to temp*/
    {37,       713},    /*vol to temp*/
    {38,       700},    /*vol to temp*/
    {39,       691},    /*vol to temp*/
    {41,       680},    /*vol to temp*/
    {42,       672},    /*vol to temp*/
    {43,       661},    /*vol to temp*/
    {44,       651},    /*vol to temp*/
    {45,       644},    /*vol to temp*/
    {47,       600},    /*vol to temp*/
    {48,       565},    /*vol to temp*/
    {49,       550},    /*vol to temp*/
    {50,       535},    /*vol to temp*/
    {51,       521},    /*vol to temp*/
    {53,       507},    /*vol to temp*/
    {54,       493},    /*vol to temp*/
    {55,       480},    /*vol to temp*/
    {57,       467},    /*vol to temp*/
    {58,       454},    /*vol to temp*/
    {59,       441},    /*vol to temp*/
    {60,       429},    /*vol to temp*/
    {62,       417},    /*vol to temp*/
    {63,       406},    /*vol to temp*/
    {64,       395},    /*vol to temp*/
    {66,       384},    /*vol to temp*/
    {67,       373},    /*vol to temp*/
    {68,       362},     /*vol to temp*/
    {69,       352},     /*vol to temp*/
    {70,       342},     /*vol to temp*/
    {71,       333},     /*vol to temp*/
    {73,       323},     /*vol to temp*/
    {74,       314},     /*vol to temp*/
    {75,       305},     /*vol to temp*/
    {76,       297},     /*vol to temp*/
    {77,       288},     /*vol to temp*/
    {78,       280},     /*vol to temp*/
    {79,       272},     /*vol to temp*/
    {81,       264},     /*vol to temp*/
    {82,       257},     /*vol to temp*/
    {83,       250},     /*vol to temp*/
    {84,       243},     /*vol to temp*/
    {85,       236},     /*vol to temp*/
    {87,       229},     /*vol to temp*/
    {88,       223},     /*vol to temp*/
    {89,       217},     /*vol to temp*/
    {90,       211},     /*vol to temp*/
    {91,       205},     /*vol to temp*/
    {92,       199},     /*vol to temp*/
    {93,       194},     /*vol to temp*/
    {95,       188},     /*vol to temp*/
    {96,       183},     /*vol to temp*/
    {97,       178},     /*vol to temp*/
};

#if (MBB_CHG_EXTCHG == FEATURE_ON)
const  CHG_TEMP_ADC_TYPE  g_adc_batt_therm_map_extchg[] =
{
    {-30,      1702},   /*vol to temp*/
    {-30,      1696},   /*vol to temp*/
    {-29,      1690},   /*vol to temp*/
    {-29,      1683},   /*vol to temp*/
    {-29,      1676},   /*vol to temp*/
    {-28,      1669},   /*vol to temp*/
    {-28,      1662},   /*vol to temp*/
    {-28,      1654},   /*vol to temp*/
    {-27,      1646},   /*vol to temp*/
    {-27,      1638},   /*vol to temp*/
    {-26,      1629},   /*vol to temp*/
    {-25,      1620},   /*vol to temp*/
    {-24,      1610},   /*vol to temp*/
    {-23,      1601},   /*vol to temp*/
    {-22,      1590},   /*vol to temp*/
    {-21,      1580},   /*vol to temp*/
    {-20,      1569},   /*vol to temp*/
    {-19,      1558},   /*vol to temp*/
    {-18,      1546},   /*vol to temp*/
    {-17,      1534},   /*vol to temp*/
    {-16,      1521},   /*vol to temp*/
    {-15,      1508},   /*vol to temp*/
    {-14,      1495},   /*vol to temp*/
    {-13,      1481},   /*vol to temp*/
    {-12,      1467},   /*vol to temp*/
    {-11,      1453},   /*vol to temp*/
    {-10,      1438},   /*vol to temp*/
    {-9,       1423},   /*vol to temp*/
    {-8,       1407},   /*vol to temp*/
    {-7,       1391},   /*vol to temp*/
    {-6,       1375},   /*vol to temp*/
    {-5,       1358},   /*vol to temp*/
    {-4,       1341},   /*vol to temp*/
    {-3,       1324},   /*vol to temp*/
    {-2,       1306},   /*vol to temp*/
    {-1,       1289},   /*vol to temp*/
    {0,        1270},   /*vol to temp*/
    {1,        1252},   /*vol to temp*/
    {2,        1233},   /*vol to temp*/
    {3,        1215},   /*vol to temp*/
    {4,        1196},   /*vol to temp*/
    {5,        1176},   /*vol to temp*/
    {6,        1157},   /*vol to temp*/
    {7,        1138},   /*vol to temp*/
    {8,        1118},   /*vol to temp*/
    {9,        1098},   /*vol to temp*/
    {10,       1078},   /*vol to temp*/
    {11,       1059},   /*vol to temp*/
    {12,       1039},   /*vol to temp*/
    {13,       1019},   /*vol to temp*/
    {14,       999},    /*vol to temp*/
    {15,       979},    /*vol to temp*/
    {16,       959},    /*vol to temp*/
    {17,       939},    /*vol to temp*/
    {18,       920},    /*vol to temp*/
    {19,       900},    /*vol to temp*/
    {20,       881},    /*vol to temp*/
    {21,       861},    /*vol to temp*/
    {22,       842},    /*vol to temp*/
    {23,       823},    /*vol to temp*/
    {24,       804},    /*vol to temp*/
    {25,       785},    /*vol to temp*/
    {26,       767},    /*vol to temp*/
    {27,       749},    /*vol to temp*/
    {28,       731},    /*vol to temp*/
    {29,       720},    /*vol to temp*/
    {30,       710},    /*vol to temp*/
    {31,       700},    /*vol to temp*/
    {33,       690},    /*vol to temp*/
    {34,       680},    /*vol to temp*/
    {35,       670},    /*vol to temp*/
    {36,       665},    /*vol to temp*/
    {37,       660},    /*vol to temp*/
    {38,       655},    /*vol to temp*/
    {39,       650},    /*vol to temp*/
    {40,       643},    /*vol to temp*/
    {41,       633},    /*vol to temp*/
    {42,       626},    /*vol to temp*/
    {43,       620},    /*vol to temp*/
    {44,       613},    /*vol to temp*/
    {45,       608},    /*vol to temp*/
    {46,       591},    /*vol to temp*/
    {47,       581},    /*vol to temp*/
    {48,       560},    /*vol to temp*/
    {49,       530},    /*vol to temp*/
    {49,       510},    /*vol to temp*/
    {50,       505},    /*vol to temp*/
    {50,       490},    /*vol to temp*/
    {51,       485},    /*vol to temp*/
    {52,       420},    /*vol to temp*/
    {53,       374},    /*vol to temp*/
    {54,       365},    /*vol to temp*/
    {55,       355},    /*vol to temp*/
    {56,       348},    /*vol to temp*/
    {57,       340},    /*vol to temp*/
    {58,       332},    /*vol to temp*/
    {59,       324},    /*vol to temp*/
    {60,       315},    /*vol to temp*/
    {61,       307},    /*vol to temp*/
    {62,       300},    /*vol to temp*/
    {63,       292},    /*vol to temp*/
    {64,       284},    /*vol to temp*/
    {65,       276},    /*vol to temp*/
    {66,       268},    /*vol to temp*/
    {67,       260},    /*vol to temp*/
    {68,       252},    /*vol to temp*/
    {69,       244},    /*vol to temp*/
    {70,       235},    /*vol to temp*/
    {71,       227},    /*vol to temp*/
    {72,       220},    /*vol to temp*/
    {73,       212},    /*vol to temp*/
    {74,       204},    /*vol to temp*/
    {75,       194},    /*vol to temp*/
    {76,       188},    /*vol to temp*/
    {77,       183},    /*vol to temp*/
    {78,       178},    /*vol to temp*/
};

const  CHG_TEMP_ADC_TYPE  g_adc_batt_therm_map_extchg_otg[] =
{
    {-30,      1702},    /*vol to temp*/
    {-29,      1696},    /*vol to temp*/
    {-28,      1690},    /*vol to temp*/
    {-27,      1683},    /*vol to temp*/
    {-26,      1676},    /*vol to temp*/
    {-25,      1669},    /*vol to temp*/
    {-24,      1662},    /*vol to temp*/
    {-23,      1654},    /*vol to temp*/
    {-22,      1646},    /*vol to temp*/
    {-21,      1638},    /*vol to temp*/
    {-20,      1629},    /*vol to temp*/
    {-19,      1620},    /*vol to temp*/
    {-18,      1610},    /*vol to temp*/
    {-17,      1601},    /*vol to temp*/
    {-16,      1590},    /*vol to temp*/
    {-15,      1580},    /*vol to temp*/
    {-14,      1569},    /*vol to temp*/
    {-13,      1558},    /*vol to temp*/
    {-12,      1546},    /*vol to temp*/
    {-11,      1534},    /*vol to temp*/
    {-10,      1521},     /*vol to temp*/
    {-9,       1508},     /*vol to temp*/
    {-8,       1495},     /*vol to temp*/
    {-7,       1481},     /*vol to temp*/
    {-6,       1467},     /*vol to temp*/
    {-5,       1453},    /*vol to temp*/
    {-4,       1438},    /*vol to temp*/
    {-3,       1423},     /*vol to temp*/
    {-2,       1407},     /*vol to temp*/
    {-1,       1391},     /*vol to temp*/
    {0,        1375},     /*vol to temp*/
    {1,        1358},     /*vol to temp*/
    {2,        1341},     /*vol to temp*/
    {3,        1324},     /*vol to temp*/
    {4,        1306},    /*vol to temp*/
    {5,        1289},    /*vol to temp*/
    {6,        1270},     /*vol to temp*/
    {7,        1252},     /*vol to temp*/
    {8,        1233},     /*vol to temp*/
    {9,        1215},     /*vol to temp*/
    {10,       1196},     /*vol to temp*/
    {11,       1176},     /*vol to temp*/
    {12,       1157},     /*vol to temp*/
    {13,       1138},      /*vol to temp*/
    {14,       1118},      /*vol to temp*/
    {15,       1098},      /*vol to temp*/
    {16,       1068},     /*vol to temp*/
    {17,       1039},     /*vol to temp*/
    {18,       998},     /*vol to temp*/
    {19,       978},     /*vol to temp*/
    {20,       959},     /*vol to temp*/
    {21,       939},     /*vol to temp*/
    {22,       919},     /*vol to temp*/
    {23,       900},     /*vol to temp*/
    {24,       880},     /*vol to temp*/
    {25,       861},     /*vol to temp*/
    {26,       841},     /*vol to temp*/
    {27,       822},     /*vol to temp*/
    {28,       804},     /*vol to temp*/
    {29,       785},     /*vol to temp*/
    {30,       766},     /*vol to temp*/
    {31,       748},     /*vol to temp*/
    {32,       730},     /*vol to temp*/
    {33,       712},     /*vol to temp*/
    {34,       695},     /*vol to temp*/
    {35,       677},     /*vol to temp*/
    {36,       660},     /*vol to temp*/
    {37,       644},     /*vol to temp*/
    {38,       627},     /*vol to temp*/
    {39,       611},     /*vol to temp*/
    {40,       595},     /*vol to temp*/
    {41,       580},     /*vol to temp*/
    {42,       565},     /*vol to temp*/
    {43,       550},     /*vol to temp*/
    {44,       535},     /*vol to temp*/
    {45,       521},     /*vol to temp*/
    {46,       507},     /*vol to temp*/
    {47,       492},     /*vol to temp*/
    {48,       478},     /*vol to temp*/
    {49,       470},     /*vol to temp*/
    {50,       459},     /*vol to temp*/
    {51,       450},     /*vol to temp*/
    {52,       438},     /*vol to temp*/
    {53,       430},     /*vol to temp*/
    {54,       421},     /*vol to temp*/
    {55,       414},     /*vol to temp*/
    {56,       403},     /*vol to temp*/
    {57,       387},     /*vol to temp*/
    {58,       379},     /*vol to temp*/
    {59,       371},     /*vol to temp*/
    {60,       366},      /*vol to temp*/
    {61,       360},      /*vol to temp*/
    {62,       352},      /*vol to temp*/
    {63,       341},      /*vol to temp*/
    {64,       333},      /*vol to temp*/
    {65,       327},      /*vol to temp*/
    {66,       323},      /*vol to temp*/
    {67,       317},      /*vol to temp*/
    {68,       310},      /*vol to temp*/
    {69,       304},      /*vol to temp*/
    {70,       298},      /*vol to temp*/
    {71,       292},      /*vol to temp*/
    {72,       284},      /*vol to temp*/
    {73,       276},      /*vol to temp*/
    {74,       270},      /*vol to temp*/
    {75,       264},      /*vol to temp*/
    {76,       258},      /*vol to temp*/
    {77,       252},      /*vol to temp*/
    {78,       246},      /*vol to temp*/
    {79,       240},      /*vol to temp*/
    {80,       234},      /*vol to temp*/
    {81,       228},      /*vol to temp*/
    {82,       226},      /*vol to temp*/
    {83,       220},      /*vol to temp*/
    {84,       214},      /*vol to temp*/
    {85,       208},      /*vol to temp*/
};
#endif

/*USB温保，USBV-T,硬件提供*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
const  CHG_TEMP_ADC_TYPE  g_adc_usb_therm_map[] =
{
    {-11,      1664},   /*vol to temp*/
    {1,        1574},   /*vol to temp*/
    {6,        1516},   /*vol to temp*/
    {11,       1447},    /*vol to temp*/
    {16,       1370},    /*vol to temp*/
    {21,       1284},    /*vol to temp*/
    {26,       1192},    /*vol to temp*/
    {27,       1056},    /*vol to temp*/
    {30,       996},    /*vol to temp*/
    {35,       897},    /*vol to temp*/
    {40,       801},    /*vol to temp*/
    {45,       710},    /*vol to temp*/
    {47,       624},    /*vol to temp*/
    {52,       546},    /*vol to temp*/
    {57,       476},    /*vol to temp*/
    {58,       463},    /*vol to temp*/
    {59,       450},    /*vol to temp*/
    {60,       437},    /*vol to temp*/
    {61,       425},    /*vol to temp*/
    {62,       413},    /*vol to temp*/
    {63,       401},    /*vol to temp*/
    {64,       390},    /*vol to temp*/
    {66,       379},    /*vol to temp*/
    {70,       368},    /*vol to temp*/
    {75,       358},    /*vol to temp*/
    {78,       347},    /*vol to temp*/
    {80,       337},    /*vol to temp*/
    {82,       328},    /*vol to temp*/
    {84,       318},    /*vol to temp*/
    {86,       309},     /*vol to temp*/
    {90,       300},     /*vol to temp*/
    {92,       292},     /*vol to temp*/
    {94,       283},     /*vol to temp*/
    {96,       275},     /*vol to temp*/
    {100,       268},     /*vol to temp*/
    {101,       259},     /*vol to temp*/
    {102,       252},     /*vol to temp*/
    {103,       245},     /*vol to temp*/
   
};
#endif

#else /*!defined(BSP_CONFIG_BOARD_E5_DC04)*/
const  CHG_TEMP_ADC_TYPE  g_adc_batt_therm_map[] =
{
//该表由硬件提供
    {-30,      1651},   /*vol to temp*/
    {-29,      1644},   /*vol to temp*/
    {-28,      1637},   /*vol to temp*/
    {-27,      1629},   /*vol to temp*/
    {-26,      1621},   /*vol to temp*/
    {-25,      1613},   /*vol to temp*/
    {-24,      1605},   /*vol to temp*/
    {-23,      1596},   /*vol to temp*/
    {-22,      1587},   /*vol to temp*/
    {-21,      1578},   /*vol to temp*/
    {-20,      1568},   /*vol to temp*/
    {-19,      1558},   /*vol to temp*/
    {-18,      1548},   /*vol to temp*/
    {-17,      1538},   /*vol to temp*/
    {-16,      1527},   /*vol to temp*/
    {-15,      1516},   /*vol to temp*/
    {-14,      1505},   /*vol to temp*/
    {-13,      1493},   /*vol to temp*/
    {-12,      1481},   /*vol to temp*/
    {-11,      1469},   /*vol to temp*/
    {-10,      1456},    /*vol to temp*/
    {-9,       1444},    /*vol to temp*/
    {-8,       1431},    /*vol to temp*/
    {-7,       1417},    /*vol to temp*/
    {-6,       1404},    /*vol to temp*/
    {-5,       1390},    /*vol to temp*/
    {-4,       1376},    /*vol to temp*/
    {-3,       1361},    /*vol to temp*/
    {-2,       1347},    /*vol to temp*/
    {-1,       1332},    /*vol to temp*/
    {0,        1317},    /*vol to temp*/
    {1,        1301},    /*vol to temp*/
    {2,        1286},    /*vol to temp*/
    {3,        1270},    /*vol to temp*/
    {4,        1254},    /*vol to temp*/
    {5,        1238},    /*vol to temp*/
    {6,        1222},    /*vol to temp*/
    {7,        1206},    /*vol to temp*/
    {8,        1189},    /*vol to temp*/
    {9,        1173},    /*vol to temp*/
    {10,       1156},    /*vol to temp*/
    {11,       1139},    /*vol to temp*/
    {12,       1122},    /*vol to temp*/
    {13,       1105},    /*vol to temp*/
    {14,       1088},    /*vol to temp*/
    {15,       1071},    /*vol to temp*/
    {16,       1054},    /*vol to temp*/
    {17,       1036},    /*vol to temp*/
    {18,       1019},    /*vol to temp*/
    {19,       1002},    /*vol to temp*/
    {20,       985},    /*vol to temp*/
    {21,       968},    /*vol to temp*/
    {22,       950},    /*vol to temp*/
    {23,       933},    /*vol to temp*/
    {24,       916},    /*vol to temp*/
    {25,       900},    /*vol to temp*/
    {26,       883},    /*vol to temp*/
    {27,       866},    /*vol to temp*/
    {28,       849},    /*vol to temp*/
    {29,       833},    /*vol to temp*/
    {30,       817},    /*vol to temp*/
    {31,       800},    /*vol to temp*/
    {32,       784},    /*vol to temp*/
    {33,       768},    /*vol to temp*/
    {34,       753},    /*vol to temp*/
    {35,       737},    /*vol to temp*/
    {36,       722},    /*vol to temp*/
    {37,       706},    /*vol to temp*/
    {38,       691},    /*vol to temp*/
    {39,       677},    /*vol to temp*/
    {40,       662},    /*vol to temp*/
    {41,       648},    /*vol to temp*/
    {42,       634},    /*vol to temp*/
    {43,       620},    /*vol to temp*/
    {44,       606},    /*vol to temp*/
    {45,       592},    /*vol to temp*/
    {46,       579},    /*vol to temp*/
    {47,       566},    /*vol to temp*/
    {48,       553},    /*vol to temp*/
    {49,       540},    /*vol to temp*/
    {50,       528},    /*vol to temp*/
    {51,       516},    /*vol to temp*/
    {52,       504},    /*vol to temp*/
    {53,       492},    /*vol to temp*/
    {54,       481},    /*vol to temp*/
    {55,       470},    /*vol to temp*/
    {56,       459},    /*vol to temp*/
    {57,       448},    /*vol to temp*/
    {58,       437},    /*vol to temp*/
    {59,       427},    /*vol to temp*/
    {60,       417},     /*vol to temp*/
    {61,       407},     /*vol to temp*/
    {62,       397},     /*vol to temp*/
    {63,       388},     /*vol to temp*/
    {64,       379},     /*vol to temp*/
    {65,       370},     /*vol to temp*/
    {66,       361},     /*vol to temp*/
    {67,       352},     /*vol to temp*/
    {68,       344},     /*vol to temp*/
    {69,       335},     /*vol to temp*/
    {70,       327},     /*vol to temp*/
    {71,       320},     /*vol to temp*/
    {72,       312},     /*vol to temp*/
    {73,       304},     /*vol to temp*/
    {74,       297},     /*vol to temp*/
    {75,       290},     /*vol to temp*/
    {76,       283},     /*vol to temp*/
    {77,       276},     /*vol to temp*/
    {78,       270},     /*vol to temp*/
    {79,       263},     /*vol to temp*/
    {80,       257},     /*vol to temp*/
    {81,       251},     /*vol to temp*/
    {82,       245},     /*vol to temp*/
    {83,       239},     /*vol to temp*/
    {84,       233},     /*vol to temp*/
    {85,       228},     /*vol to temp*/
};

/*USB温保,USBV-T,硬件提供*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
const  CHG_TEMP_ADC_TYPE  g_adc_usb_therm_map[] =
{
    {-10,      1664},   /*vol to temp*/
    {0,        1574},   /*vol to temp*/
    {5,        1516},   /*vol to temp*/
    {10,       1447},    /*vol to temp*/
    {15,       1370},    /*vol to temp*/
    {20,       1284},    /*vol to temp*/
    {25,       1192},    /*vol to temp*/
    {30,       1095},    /*vol to temp*/
    {35,       996},    /*vol to temp*/
    {40,       897},    /*vol to temp*/
    {45,       801},    /*vol to temp*/
    {50,       710},    /*vol to temp*/
    {55,       624},    /*vol to temp*/
    {60,       546},    /*vol to temp*/
    {65,       476},    /*vol to temp*/
    {66,       463},    /*vol to temp*/
    {67,       450},    /*vol to temp*/
    {68,       437},    /*vol to temp*/
    {69,       425},    /*vol to temp*/
    {70,       413},    /*vol to temp*/
    {71,       401},    /*vol to temp*/
    {72,       390},    /*vol to temp*/
    {73,       379},    /*vol to temp*/
    {74,       368},    /*vol to temp*/
    {75,       358},    /*vol to temp*/
    {76,       347},    /*vol to temp*/
    {77,       337},    /*vol to temp*/
    {78,       328},    /*vol to temp*/
    {79,       318},    /*vol to temp*/
    {80,       309},     /*vol to temp*/
    {81,       300},     /*vol to temp*/
    {82,       292},     /*vol to temp*/
    {83,       283},     /*vol to temp*/
    {84,       275},     /*vol to temp*/
    {85,       268},     /*vol to temp*/
    {86,       259},     /*vol to temp*/
    {87,       252},     /*vol to temp*/
    {88,       245},     /*vol to temp*/
    {89,       238},     /*vol to temp*/
    {90,       231},     /*vol to temp*/
    {91,       224},     /*vol to temp*/
    {92,       218},     /*vol to temp*/
    {93,       211},     /*vol to temp*/
    {94,       205},     /*vol to temp*/
    {95,       199},     /*vol to temp*/
    {96,       194},     /*vol to temp*/
    {97,       188},     /*vol to temp*/
    {98,       183},     /*vol to temp*/
    {99,       178},     /*vol to temp*/
    {100,      173},     /*vol to temp*/
    {101,      168},     /*vol to temp*/
    {102,      163},     /*vol to temp*/
    {103,      158},     /*vol to temp*/
    {104,      154},     /*vol to temp*/
    {105,      150},     /*vol to temp*/
    {106,      145},     /*vol to temp*/
    {107,      141},     /*vol to temp*/
    {108,      137},     /*vol to temp*/
    {109,      134},     /*vol to temp*/
    {110,      130},     /*vol to temp*/
    {111,      126},     /*vol to temp*/
    {112,      123},     /*vol to temp*/
    {113,      119},     /*vol to temp*/
    {114,      116},     /*vol to temp*/
    {115,      113},     /*vol to temp*/
    {116,      110},     /*vol to temp*/
    {117,      107},     /*vol to temp*/
    {118,      104},     /*vol to temp*/
    {119,      101},     /*vol to temp*/
    {120,      98},     /*vol to temp*/
    {121,      96},     /*vol to temp*/
    {122,      93},     /*vol to temp*/
    {123,      91},     /*vol to temp*/
    {124,      88},     /*vol to temp*/
    {125,      86},     /*vol to temp*/
}; 
#endif
#endif /*defined(BSP_CONFIG_BOARD_E5_DC04)*/
#else
CHG_TEMP_ADC_TYPE  g_adc_batt_therm_map[] =
{
//该表由硬件提供

    {-30,      2327},   /*vol to temp*/
    {-29,      2319},   /*vol to temp*/
    {-28,      2311},   /*vol to temp*/
    {-27,      2302},   /*vol to temp*/
    {-26,      2293},   /*vol to temp*/
    {-25,      2283},   /*vol to temp*/
    {-24,      2274},   /*vol to temp*/
    {-23,      2263},   /*vol to temp*/
    {-22,      2252},   /*vol to temp*/
    {-21,      2241},   /*vol to temp*/
    {-20,      2229},   /*vol to temp*/
    {-19,      2217},   /*vol to temp*/
    {-18,      2204},   /*vol to temp*/
    {-17,      2191},   /*vol to temp*/
    {-16,      2177},   /*vol to temp*/
    {-15,      2163},    /*vol to temp*/
    {-14,      2148},    /*vol to temp*/
    {-13,      2133},    /*vol to temp*/
    {-12,      2117},    /*vol to temp*/
    {-11,      2101},    /*vol to temp*/
    {-10,      2084},    /*vol to temp*/
    {-9 ,       2066},    /*vol to temp*/
    {-8 ,       2048},    /*vol to temp*/
    {-7 ,       2030},    /*vol to temp*/
    {-6 ,       2010},    /*vol to temp*/
    {-5 ,       1991},    /*vol to temp*/
    {-4 ,       1971},    /*vol to temp*/
    {-3 ,       1950},    /*vol to temp*/
    {-2 ,       1929},    /*vol to temp*/
    {-1 ,       1908},    /*vol to temp*/
    {0     ,    1886},    /*vol to temp*/
    {1     ,    1863},    /*vol to temp*/
    {2     ,    1840},    /*vol to temp*/
    {3     ,    1817},    /*vol to temp*/
    {4     ,    1793},    /*vol to temp*/
    {5     ,    1769},    /*vol to temp*/
    {6     ,    1744},    /*vol to temp*/
    {7     ,    1719},    /*vol to temp*/
    {8     ,    1694},    /*vol to temp*/
    {9     ,    1668},    /*vol to temp*/
    {10 ,       1643},    /*vol to temp*/
    {11 ,       1617},    /*vol to temp*/
    {12 ,       1590},    /*vol to temp*/
    {13 ,       1564},    /*vol to temp*/
    {14 ,       1537},    /*vol to temp*/
    {15 ,       1510},    /*vol to temp*/
    {16 ,       1483},    /*vol to temp*/
    {17 ,       1456},    /*vol to temp*/
    {18 ,       1429},    /*vol to temp*/
    {19 ,       1402},    /*vol to temp*/
    {20 ,       1375},    /*vol to temp*/
    {21 ,       1348},    /*vol to temp*/
    {22 ,       1320},    /*vol to temp*/
    {23 ,       1293},    /*vol to temp*/
    {24 ,       1267},    /*vol to temp*/
    {25 ,       1240},    /*vol to temp*/
    {26 ,       1213},    /*vol to temp*/
    {27 ,       1187},    /*vol to temp*/
    {28 ,       1160},    /*vol to temp*/
    {29 ,       1134},    /*vol to temp*/
    {30 ,       1108},    /*vol to temp*/
    {31 ,       1083},    /*vol to temp*/
    {32 ,       1058},    /*vol to temp*/
    {33 ,       1033},    /*vol to temp*/
    {34 ,       1008},    /*vol to temp*/
    {35 ,        984},    /*vol to temp*/
    {36 ,        959},    /*vol to temp*/
    {37 ,        936},    /*vol to temp*/
    {38 ,        912},    /*vol to temp*/
    {39 ,        890},    /*vol to temp*/
    {40 ,        867},    /*vol to temp*/
    {41 ,        845},    /*vol to temp*/
    {42 ,        823},    /*vol to temp*/
    {43 ,        802},    /*vol to temp*/
    {44 ,        781},    /*vol to temp*/
    {45 ,        760},    /*vol to temp*/
    {46 ,        740},    /*vol to temp*/
    {47 ,        720},    /*vol to temp*/
    {48 ,        701},    /*vol to temp*/
    {49 ,        682},    /*vol to temp*/
    {50 ,        664},    /*vol to temp*/
    {51 ,        646},    /*vol to temp*/
    {52 ,        628},    /*vol to temp*/
    {53 ,        611},    /*vol to temp*/
    {54 ,        594},    /*vol to temp*/
    {55 ,        578},     /*vol to temp*/
    {56 ,        562},     /*vol to temp*/
    {57 ,        546},     /*vol to temp*/
    {58 ,        531},     /*vol to temp*/
    {59 ,        516},     /*vol to temp*/
    {60 ,        502},     /*vol to temp*/
    {61 ,        488},     /*vol to temp*/
    {62 ,        474},     /*vol to temp*/
    {63 ,        461},     /*vol to temp*/
    {64 ,        448},     /*vol to temp*/
    {65 ,        435},     /*vol to temp*/
    {66 ,        423},     /*vol to temp*/
    {67 ,        411},     /*vol to temp*/
    {68 ,        399},     /*vol to temp*/
    {69 ,        388},     /*vol to temp*/
    {70 ,        377},     /*vol to temp*/
    {71 ,        366},     /*vol to temp*/
    {72 ,        356},     /*vol to temp*/
    {73 ,        346},     /*vol to temp*/
    {74 ,        337},     /*vol to temp*/
    {75 ,        327},     /*vol to temp*/
    {76 ,        318},     /*vol to temp*/
    {77 ,        309},     /*vol to temp*/
    {78 ,        300},     /*vol to temp*/
    {79 ,        292},     /*vol to temp*/
    {80 ,        284},     /*vol to temp*/
    {81 ,        276},     /*vol to temp*/
    {82 ,        268},     /*vol to temp*/
    {83 ,        261},     /*vol to temp*/
    {84 ,        254},     /*vol to temp*/
    {85 ,        247},     /*vol to temp*/

};
#endif


/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

#define BAT_VOL_SIZE               4

#define SMEM_FACT_RELEASE_ON       0X19840a0b    /*产线放电魔术字*/
#define SMEM_FACT_RELEASE_OFF      0X19850b0a    /*产线放电魔术字*/

/* 电池校准的最小及最大电压**/
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
#define CHG_BATT_CALI_MIN_VOLT  (3450)/*电压校准门限*/
#define CHG_BATT_CALI_MAX_VOLT  (4350)/*电压校准门限*/
#else
#define CHG_BATT_CALI_MIN_VOLT  (3400)/*电压校准门限*/
#define CHG_BATT_CALI_MAX_VOLT  (4200)/*电压校准门限*/
#endif/*MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON*/

#define MICRO_TO_MILLI_V                    (1000)

int32_t bsp_i2c_read(uint8_t reg)
{
    int ret = 0;

    if(NULL == g_i2c_client)
    {
        pr_err("%s failed due to g_i2c_client doesn't exist@~\n", __FUNCTION__);
        return -1;
    }

    ret = i2c_smbus_read_byte_data(g_i2c_client, reg);

    if(ret < 0)
    {
        return -1;
    }
    else
    {
        return ret;
    }
}

int32_t bsp_i2c_write(uint8_t reg, int32_t value)
{
    if(NULL == g_i2c_client)
    {
        pr_err("%s failed due to g_i2c_client doesn't exist@~\n", __FUNCTION__);
        return -1;
    }

    if(0 == i2c_smbus_write_byte_data(g_i2c_client, reg, value))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}
/**********************************************************************
FUNCTION:    CHG_SET_POWER_OFF
DESCRIPTION: The poweroff func of CHG module, all the power-off operation
             except at boot phase MUST be performed via calling this.
INPUT:       The shutdown reason which triggered system poweroff.
             All VALID REASON:
             DRV_SHUTDOWN_BATTERY_ERROR --BATTERY ERROR;
             DRV_SHUTDOWN_TEMPERATURE_PROTECT --EXTREAM HIGH TEMPERATURE.
             DRV_SHUTDOWN_LOW_TEMP_PROTECT --EXTREAM LOW TEMPERATURE
             DRV_SHUTDOWN_CHARGE_REMOVE --CHGR REMOVAL WHILE POWEROFF CHG
             DRV_SHUTDOWN_LOW_BATTERY --LOW BATTERY
OUTPUT:      None.
RETURN:      None.
NOTE:        When this function get called to power system off, it record
             the shutdown reason, then simulate POWER_KEY event to APP to
             perform the real system shutdown process.
             THUS, THIS FUNCTION DOESN'T TAKE AFFECT IF APP DIDN'T STARTUP.
***********************************************************************/
/***************Note:平台相关代码，根据平台按需要添加，有的平台如9x25需要
              移植人员根据需要，添加或者移除下边函数调用***************************/
void chg_set_power_off(DRV_SHUTDOWN_REASON_ENUM real_reason)
{
    /*由于关闭充电函数使用I2C及延时函数msleep等非原子操作需要schedule CPU,
     在定时器中断中需要atomic使用，会导致内核BUG*/
    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:chg_set_power_off:power off by reason = %d \n ",real_reason);
#if (FEATURE_ON == MBB_CHG_PLATFORM_QUALCOMM)
    kernel_power_off();
#elif (FEATURE_ON == MBB_CHG_PLATFORM_BALONG)
    bsp_drv_set_power_off_reason(real_reason);
    bsp_drv_power_off();
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/
    return;
}
/**********************************************************************
FUNCTION:    chg_send_stat_to_app
DESCRIPTION: Charge module sends charge state to application layer.
INPUT:       uint32_t chg_device_id,
             uint32_t chg_event_id
OUTPUT:      None.
RETURN:      None.
NOTE:        None
***********************************************************************/
void chg_send_stat_to_app(uint32_t chg_device_id, uint32_t chg_event_id)
{
#if (MBB_CHG_POWER_SUPPLY == FEATURE_ON)
    /*先更新power supply各节点属性信息*/
    chg_update_power_suply_info();

    /*给应用上报power_supply_changed事件*/
    if((DEVICE_ID_TEMP == chg_device_id) || (DEVICE_ID_KEY == chg_device_id)
        || (DEVICE_ID_BATTERY == chg_device_id))
    {
        power_supply_changed(&g_chip->bat);
    }
    else if(DEVICE_ID_WIRELESS == chg_device_id)
    {
        power_supply_changed(&g_chip->wireless);
    }
    else if(DEVICE_ID_EXTCHG == chg_device_id)
    {
        power_supply_changed(&g_chip->extchg);
    }
    else if(DEVICE_ID_USB == chg_device_id)
    {
        power_supply_changed(&g_chip->usb);
    }
    else if(DEVICE_ID_CHARGER == chg_device_id)
    {
        power_supply_changed(&g_chip->ac);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:chg event device id unknow !\n ");
    }

    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:chg_send_stat_to_app->deviceid %d, eventid %d \n ",
                            chg_device_id,chg_event_id);
#else
    /*********NOTE:如果是非LINUX系统等不使用power supply事件上报APP方式的平台需要将平台
              上报事件的接口函数在此修改,如V7R1平台可使用如下函数BSP_CHGC_SendStaToApp************/
    //BSP_CHGC_SendStaToApp((uint32_t)chg_device_id, (uint32_t)chg_event_id);
    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:chg_send_stat_to_app->deviceid %d, eventid %d \n ",
                            chg_device_id,chg_event_id);
#endif/*MBB_CHG_POWER_SUPPLY == FEATURE_ON*/
}


#ifdef CHG_STUB
DRV_START_MODE_ENUM chg_get_start_mode(void)
{
    return DRV_START_MODE_NORMAL;
}
#else
DRV_START_MODE_ENUM chg_get_start_mode(void)
{
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
    DRV_START_MODE mode = DRV_START_MODE_NORMAL;
    DRV_RUN_MODE boot_mode = get_run_mode();
    DRV_CHARGING_MODE pwd_flag = get_pd_charge_flag();
#if (FEATURE_ON == MBB_DLOAD)
    if((RUN_MODE_NORMAL == boot_mode) && (POWER_DOWN_CHARGING_MODE != pwd_flag))
    {
        /* 正常模式 */
        mode = DRV_START_MODE_NORMAL;
    }
    else if((RUN_MODE_NORMAL == boot_mode) && ((POWER_DOWN_CHARGING_MODE == pwd_flag)))
    {
        /* 关机充电模式 */
        mode = DRV_START_MODE_CHARGING;
    }
    else if(RUN_MODE_RECOVERY == boot_mode)
    {
        /* 升级模式 */
        mode = DRV_START_MODE_UPDATE;
    }
#endif/*FEATURE_ON == MBB_DLOAD*/
    return mode;

#elif(MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
    static_smem_vendor0 *smem_data = NULL;

    /*通过读取共享内存，获取当前开机状态，此处由平台移植人员填写,
    根据开机状态，区分是充电开机模式，正常开机模式，异常开机模式*/
    smem_data = (static_smem_vendor0 *)SMEM_HUAWEI_ALLOC(SMEM_ID_VENDOR0 , sizeof(static_smem_vendor0 ));
    if(NULL == smem_data)
    {
        return DRV_START_MODE_NORMAL;
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:smem_data %x !\n", (unsigned int)(smem_data->smem_huawei_poweroff_chg));

    /*start_mode_flag = */
    switch(smem_data->smem_huawei_poweroff_chg)
    {
        /*case 0:*/
        case POWER_ON_STATUS_FINISH_CHARGE:
        {
            /*break;*/
            return DRV_START_MODE_CHARGING;
        }
        /*case 1:*/
        case POWER_ON_STATUS_FINISH_NORMAL:
        {
            /*break;*/
            return DRV_START_MODE_NORMAL;
        }
        /*case 2:*/
        case POWER_ON_STATUS_BOOTING:
        case POWER_ON_STATUS_CHARGING:
        case POWER_ON_STATUS_CAPACITY_LOW:
        case POWER_ON_STATUS_BATTERY_ERROR:
        default:
        {
            /*break;*/
            return DRV_START_MODE_EXCEPTION;
        }
    }

#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/

}
#endif/*CHG_STUB*/

 
boolean chg_is_powdown_charging (void)
{
    DRV_START_MODE_ENUM start_mode = DRV_START_MODE_BUTT;

    /*工厂模式单板不进入关机充电*/
    if(TRUE == chg_is_ftm_mode())
    {
        return FALSE;
    }

    start_mode = chg_get_start_mode();
    if(DRV_START_MODE_CHARGING == start_mode)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#if (MBB_CHG_WIRELESS == FEATURE_ON)
/*无线充电MMI测试前端限流*/
#define WIRELESS_MMI_IUSB_CURRENT               (900)
/*无线充电充电后端限流*/
#define WIRELESS_MMI_ICHG_CURRENT               (1024)
/*无线充电延迟6S*/
#define WIRELESS_MMI_TEST_DELAY_COUNT           (6000)


void wireless_mmi_test_proc(ulong64_t plug)
{
    int32_t vbus_volt = 0;

    if(0 == plug)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:MMI TEST WIRELESS_CHGR PLUG OUT!\n");
        (void)chg_set_charge_enable(FALSE);
        chg_stm_set_wireless_online_st(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
        return;
    }

    vbus_volt = chg_get_volt_from_adc(CHG_PARAMETER__VBUS_VOLT);
    if(vbus_volt < VBUS_JUDGEMENT_THRESHOLD)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:MMI TEST WIRELESS_CHGR PLUG IN!\n");
        chg_set_wireless_chg_enable(TRUE);
        (void)chg_set_dpm_val(CHG_WIRELESS_DPM_VOLT);
        (void)chg_set_cur_level(WIRELESS_MMI_ICHG_CURRENT);
        (void)chg_set_supply_limit(WIRELESS_MMI_IUSB_CURRENT);
        (void)chg_set_charge_enable(TRUE);
        chg_delay_ms(WIRELESS_MMI_TEST_DELAY_COUNT);

        if(TRUE == chg_is_IC_charging())
        {
            chg_stm_set_wireless_online_st(TRUE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
        else
        {
            chg_stm_set_wireless_online_st(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
        (void)chg_set_charge_enable(FALSE);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:MMI TEST VBUS above 700mv!\n");
        (void)chg_set_charge_enable(FALSE);
        chg_stm_set_wireless_online_st(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
    }
}
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)
#if((FEATURE_ON == MBB_FACTORY)\
     && (FEATURE_ON == MBB_CHG_COULOMETER))
#define MMI_EXTCHG_SHUTDOWN_TIME_MS    (60000)

struct delayed_work mmi_extchg_shutdown_work;
boolean is_extchg_shutdown_init = FALSE;

void mmi_extchg_shutdown(void)
{
    chg_set_extchg_chg_enable(FALSE);
    chg_stm_set_extchg_online_st(FALSE);
    chg_print_level_message(CHG_MSG_ERR, "CHG_MMI:mmi_extchg_shutdown !!!\n");
}
#endif/*(FEATURE_ON == MBB_FACTORY) && (FEATURE_ON == MBB_CHG_COULOMETER)*/

void extchg_mmi_test_proc(void)
{
    boolean extchg_online_st = FALSE;
    /*MMI测试模式通过使能对外充电检测VBUS电压判断对外冲充电设备是否在位*/
    extchg_online_st = chg_get_extchg_online_status();
    chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:extchg_mmi_test extchg_online_st = %d!!!\n",extchg_online_st);
    chg_stm_set_extchg_online_st(extchg_online_st);
    chg_set_extchg_status(POWER_SUPPLY_EXTCHGSTA_START_CHARGING);
    chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG, (uint32_t)CHG_EVENT_NONEED_CARE);
#if((FEATURE_ON == MBB_FACTORY)\
     && (FEATURE_ON == MBB_CHG_COULOMETER)\
     && (FEATURE_ON == MBB_CHG_EXTCHG ))
    /*MMI工位增加读取库仑计电流操作，在读取之前需要check Vbus电压是否正常*/
    chg_check_vbus_volt();
    /*Vbus电压check结束，上报应用可以读取库仑计电流*/
    chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG, (uint32_t)CHG_EVENT_NONEED_CARE);
    if(FALSE == is_extchg_shutdown_init)
    {
        INIT_DELAYED_WORK(&mmi_extchg_shutdown_work,mmi_extchg_shutdown);
        is_extchg_shutdown_init = TRUE;
    }
    /*为了防止1min内多次插拔，在重新插入后首先cancel上一次的delay work*/
    cancel_delayed_work_sync(&mmi_extchg_shutdown_work);
    schedule_delayed_work(&mmi_extchg_shutdown_work, msecs_to_jiffies(MMI_EXTCHG_SHUTDOWN_TIME_MS));
    chg_print_level_message(CHG_MSG_ERR, "CHG_MMI:stop extchg after one minute!\n");
#endif
    chg_print_level_message(CHG_MSG_ERR, "CHG_MMI:extchg_mmi_test over!\n");
}
#endif/*defined(MBB_CHG_EXTCHG)*/


/*PT工位使能充电前端限流*/
#define PT_ENABLECHG_IUSB_CURRENT          (500)
/*PT工位使能充电后端限流*/
#define PT_ENABLECHG_ICHG_CURRENT          (576)
/*PT工位使能充电恒压电压*/
#define PT_ENABLECHG_CV_VOLTAGE            (4200)
/*PT工位使能充电截止电流*/
#define PT_ENABLECHG_TERMINATE_CURRENT     (128)

boolean chg_pt_mmi_test_proc(void)
{
    if(FALSE == chg_set_supply_limit(PT_ENABLECHG_IUSB_CURRENT))
    {
            chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:chg_pt_mmi set supply limit fail\n");
            return FALSE;
    }
    if(FALSE == chg_set_cur_level(PT_ENABLECHG_ICHG_CURRENT))
    {
            chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:chg_pt_mmi set current level fail\n");
            return FALSE;
    }
    if(FALSE == chg_set_vreg_level(PT_ENABLECHG_CV_VOLTAGE))
    {
            chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:chg_pt_mmi set vreg level fail\n");
            return FALSE;
    }
    if(FALSE == chg_set_term_current(PT_ENABLECHG_TERMINATE_CURRENT))
    {
            chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:chg_pt_mmi set term level fail\n");
            return FALSE;
    }
    if(FALSE == chg_set_charge_enable(TRUE))
    {
            chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:chg_pt_mmi set charge enable fail\n");
            return FALSE;
    }
    return TRUE;
}

static inline char* get_chgr_name_by_id(int32_t chg_type)
{
    static char* chg_chgr_type_name_tab[CHG_CHGR_INVALID + 1] =
    {
        "Unknown",                      //充电器类型未知
        "Wall Charger",                 //标准充电器
        "USB HOST PC",                  //USB
        "Non-Standard Chgr",            //第三方充电器
        "wireless chgr"                 //无线充电器
        "extchg chgr"                   //对外充电器
        "Invalid",                      //充电器不可用
    };

    return ((chg_type >= CHG_CHGR_UNKNOWN && chg_type <= CHG_CHGR_INVALID) \
                         ? chg_chgr_type_name_tab[chg_type] : "null");
}

/*****************************************************************************
 函 数 名  : get_chgr_type_from_usb
 功能描述  : Get real charger type from USB module.
             This function called the platform interfaces to obtain the real
             charger type from USB module.
 输入参数  : plug:外电源插入拔出
 输出参数  : chg_type:充电器类型
 返 回 值  : Charger type of state machine needed.
 CALL-WHOM : Platform interfaces.
 WHO-CALLED: chg_check_and_update_hw_param_per_chgr_type
             chg_transit_state_period_func
 NOTICE    : 1. Platform chgr_type enum may be different from chg_stm design,
             we need convert or remap them before return.
             2. Balong V3R2/V7R1 platform didn't support CHG_USB_HOST_PC,
             CHG_NONSTD_CHGR and CHG_USB_HOST_PC would all be treated as
             CHG_NONSTD_CHGR.
             3. Export Required.
*****************************************************************************/
chg_chgr_type_t get_chgr_type_from_usb(ulong64_t plug,int32_t chg_type)
{

    chg_stm_state_type cur_stat = chg_stm_get_cur_state();
    CHG_MODE_ENUM cur_chg_mode = chg_get_cur_chg_mode();
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
printk("get_chgr_type_from_usb cur_chgr_type is %d\r\n", cur_chgr_type);
    chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:get_chgr_type_from_usb cur_stat=%d,cur_chg_mode=%d,cur_chgr_type=%d\n",
            cur_stat,cur_chg_mode,cur_chgr_type);
    chg_print_level_message(CHG_MSG_INFO, "CHG_PLT:get_chgr_type_from_usb plug=%d,chg_type=%d\n",
                            plug,chg_type);

    mlog_print(MLOG_CHG, mlog_lv_info, "charger type %s(%d) %s detected.\n",
               get_chgr_name_by_id(chg_type), chg_type,
               plug ? "insertion" : "removal");

    if(TRUE == chg_is_ftm_mode())
    {
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(CHG_WIRELESS_CHGR == chg_type)
        {
            wireless_mmi_test_proc(plug);
            return CHG_CHGR_INVALID;
        }
#endif /*MBB_CHG_WIRELESS == FEATURE_ON*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)
        if(CHG_EXGCHG_CHGR == chg_type)
        {
            /*USB ID插入进行MMI检测*/
            if(1 == plug)
            {
                extchg_mmi_test_proc();
                return CHG_CHGR_INVALID;
            }
            /*USB ID拔出调用USB驱动接口D+ D-直连基带，拉高HS_ID*/
            else
            {
#if((FEATURE_ON == MBB_FACTORY)\
     && (FEATURE_ON == MBB_CHG_COULOMETER))
/*库仑计MMI对外充电方案支持重新插入对外充电设备，因此在拔出对外充电设备后需要关闭对外充电设置*/
                mmi_extchg_shutdown();
#endif
                return CHG_CHGR_INVALID;
            }
        }
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/
        return CHG_CHGR_INVALID;
    }
    /*补电有单独的补电任务运行，因此不响应插拔事件*/
    if(CHG_SUPPLY_MODE == cur_chg_mode)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT: cur_chg_mode = CHG_SUPPLY_MODE!\n ");
        return CHG_USB_HOST_PC;
    }

    /*升级过程中不起充电任务，因此也不能进行状态切换，防止定时器未定义被使用导致单板死机*/
    if(TRUE == is_in_update_mode())
    {
        return CHG_USB_HOST_PC;
    }
    /*对外充电处理*/
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    if(CHG_EXGCHG_CHGR == chg_type)
    {
        chg_extchg_config_data_init();
        if (plug)
        {
            chg_extchg_insert_proc();
        }
        else
        {
            chg_extchg_remove_proc();
        }
        return CHG_EXGCHG_CHGR;
    }
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/
    /*对内充电处理*/
    chg_stm_set_chgr_type(chg_type);
    if (plug)
    {
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(CHG_WIRELESS_CHGR == chg_type)
        {
            chg_stm_set_wireless_online_st(TRUE);
        }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
        if(CHG_CHGR_UNKNOWN == chg_type)
        {
            chg_start_chgr_type_checking();
        }
        chg_charger_insert_proc(chg_stm_get_chgr_type());
        //chg_send_msg_to_main(CHG_CHARGER_IN_EVENT);
    }
    else
    {
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(CHG_WIRELESS_CHGR == chg_type)
        {
            chg_stm_set_wireless_online_st(FALSE);
        }
#endif/*MBB_CHG_WIRELESS == FEATURE_ON*/
        chg_charger_remove_proc(chg_stm_get_chgr_type());
        //chg_send_msg_to_main(CHG_CHARGER_OUT_EVENT);
    }

    return (chg_chgr_type_t)chg_type;
}


void chg_lcd_display(CHG_BATT_DISPLAY_TYPE disp_type)
{
    switch(disp_type)
    {
        case CHG_DISP_OK:
            //LCD驱动接口，由移植人员根据实际情况封装；
            break;
        case CHG_DISP_FAIL:
            //LCD驱动接口，由移植人员根据实际情况封装；
            break;
        case CHG_DISP_BATTERY_LOWER:
            //LCD驱动接口，由移植人员根据实际情况封装；
            break;
        case CHG_DISP_BATTERY_BAD:
            //LCD驱动接口，由移植人员根据实际情况封装；
            break;
        case CHG_DISP_OVER_HEATED:
            //LCD驱动接口，由移植人员根据实际情况封装；
            break;
        case CHG_DISP_TEMP_LOW:
            //LCD驱动接口，由移植人员根据实际情况封装；
            break;
        default:
            chg_print_level_message( CHG_MSG_ERR,"CHG_PLT:chg_lcd_display->disp_type invalid!!!\r\n ");
            break;
    }
    return;
}


void chg_led_display(CHG_BATT_DISPLAY_TYPE disp_type)
{
    switch(disp_type)
    {
        case CHG_DISP_OK:
        case CHG_DISP_FAIL:
            //log指示，暂不支持(闪灯产品目前都是可拆卸电池);
            chg_print_level_message( CHG_MSG_ERR,"CHG_PLT:chg_led_display->led product not support!!!\r\n ");
            break;
        case CHG_DISP_BATTERY_LOWER:
            /*调用闪灯接口，红灯长亮，其他熄灭*/
            break;
        case CHG_DISP_BATTERY_BAD:
        case CHG_DISP_OVER_HEATED: /*指高温关机而非停充*/
        case CHG_DISP_TEMP_LOW: /*指低温关机而非停充*/
            /*调用绿灯全闪接口*/
            break;
        default:
            chg_print_level_message( CHG_MSG_ERR,"CHG_PLT:chg_led_display->disp_type invalid!!!\r\n ");
            break;
    }
    return;
}


void chg_display_interface(CHG_BATT_DISPLAY_TYPE disp_type)
{
#if (MBB_CHG_LCD == FEATURE_ON)
    chg_lcd_display(disp_type);
#elif (MBB_CHG_LED == FEATURE_ON)
    chg_led_display(disp_type);
#endif/*MBB_CHG_LED == FEATURE_ON*/

    return;
}

/******************************************************************************
  Function      batt_volt_calib_atoi
  Description   将字符数组中字符'0'至字符'9'组成的数据段转换为整型
  Input         *name: 需要进行转换的字符数组的地址
  Output        N/A
  Return        val  : 将转换后生成的整形变量返回
  Others        N/A
******************************************************************************/
static int batt_volt_calib_atoi(char *name)
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


/***************Note:平台相关代码，根据平台按需要添加，有的平台如9x25需要
              移植人员根据需要，添加或者移除下边函数调用***************************/
void chg_batt_volt_calib_init(void)
{
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
    int32_t ret = 0;
    VBAT_CALIBRATION_TYPE vbatt_calib_value;

    memset((void *)&vbatt_calib_value, 0, sizeof(VBAT_CALIBRATION_TYPE));

    ret = bsp_nvm_read(NV_BATT_VOLT_CALI_I, &vbatt_calib_value, sizeof(VBAT_CALIBRATION_TYPE));
    if(0 == ret)
    {
        g_vbatt_max = vbatt_calib_value.max_value;
        g_vbatt_min = vbatt_calib_value.min_value;

        chg_print_level_message( CHG_MSG_ERR,"read calibrate value successs! max=%d,min=%d\n",
        g_vbatt_max,g_vbatt_min,0);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR,"read chg nv failed when read check value\n");
        return;
    }
    if(g_vbatt_max <= g_vbatt_min)
    {
        chg_print_level_message( CHG_MSG_ERR,"CHG_PLT:chg_batt_volt_calib_init->max min value error!!!\r\n ");
        return;
    }
    /*已校准初始化标志位*/
    g_is_batt_volt_calib_init = TRUE;

#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)

    mm_segment_t old_fs = {0};
    struct file *fp_v_low = NULL;
    struct file *fp_v_high = NULL ;
    int bat_low = 0;
    int bat_high = 0;
    loff_t pos_v_low = 0;
    loff_t pos_v_high = 0;
    char bat_vol_low[BAT_VOL_SIZE + 1] = {0};
    char bat_vol_high[BAT_VOL_SIZE + 1] = {0};

    fp_v_low = filp_open("/data/voltage_low", O_RDONLY, 0);
    if (IS_ERR(fp_v_low))
    {
        fp_v_low = NULL;
        chg_print_level_message( CHG_MSG_INFO,"CHG_PLT: Open Bat vol Low file failed!\n");
        return;
    }
    pos_v_low = fp_v_low->f_pos;

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    vfs_read(fp_v_low, bat_vol_low, BAT_VOL_SIZE, &pos_v_low);

    set_fs(old_fs);

    filp_close(fp_v_low, NULL);
    fp_v_high = filp_open("/data/voltage_high", O_RDONLY, 0);
    if (IS_ERR(fp_v_high))
    {
        fp_v_high = NULL;
        chg_print_level_message( CHG_MSG_ERR,"CHG_PLT: Open Bat vol High file failed!\n");
        return;
    }
    pos_v_high = fp_v_high->f_pos;

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    vfs_read(fp_v_high, bat_vol_high, BAT_VOL_SIZE, &pos_v_high);

    set_fs(old_fs);

    filp_close(fp_v_high, NULL);

    bat_low = batt_volt_calib_atoi(bat_vol_low);
    bat_high = batt_volt_calib_atoi(bat_vol_high);

    chg_print_level_message( CHG_MSG_ERR,"CHG_PLT: bat_low = %d!\n",bat_low);
    chg_print_level_message( CHG_MSG_ERR,"CHG_PLT: bat_high = %d!\n",bat_high);

    g_vbatt_min = bat_low;
    g_vbatt_max = bat_high;

    if(g_vbatt_max <= g_vbatt_min)
    {
        chg_print_level_message( CHG_MSG_ERR,"CHG_PLT:chg_batt_volt_calib_init->max min value error!!!\r\n ");
        return;
    }

    /*已校准初始化标志位*/
    g_is_batt_volt_calib_init = TRUE;

#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/
}


int32_t chg_calc_batt_volt_calib_value(int32_t orig_val)
{
    int32_t rtn_vbat_val = 0;
    if(FALSE == g_is_batt_volt_calib_init)
    {
        chg_print_level_message( CHG_MSG_INFO,"CHG_PLT:calib is not init!!!\r\n ");
        return orig_val;
    }
    /*计算电池电压校准值*/
    rtn_vbat_val = CHG_BATT_CALI_MIN_VOLT + (orig_val - g_vbatt_min)
                * (CHG_BATT_CALI_MAX_VOLT - CHG_BATT_CALI_MIN_VOLT)
                / (g_vbatt_max - g_vbatt_min);
    return rtn_vbat_val;
}

void chg_debug_adc(void)
{
    int tmp_ret           = -1;
    uint16_t rtn_vbat_val = 0;
    unsigned int i        = 0;

    for (i = 0; i < 14; i++)
    {
        tmp_ret = bsp_hkadc_convert(i, &rtn_vbat_val);
        if (tmp_ret)
        {
            chg_print_level_message( CHG_MSG_ERR,"%s: bsp_hkadc_convert failed with %d.\n", 
                __func__, tmp_ret);
        }
        else
        {
            chg_print_level_message( CHG_MSG_ERR,"hkadc%d = %d\n", i, rtn_vbat_val);
        }
    }
}

int32_t chg_get_volt_from_adc(CHG_PARAMETER_ENUM param_type)
{
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)

    int ret = 0;
    int32_t rtn_vbat_val = 0;
    int tmp_ret = -1;
    u16 tmp_rtn_vbat_val = 0;

    if (CHG_PARAMETER__BATTERY_VOLT == param_type)
    {
        /*电池电压使用库仑计*/
#if (MBB_CHG_COULOMETER == FEATURE_ON)
        rtn_vbat_val = bsp_coul_voltage();
        chg_print_level_message( CHG_MSG_DEBUG,"CHG:VBAT = %d\n", rtn_vbat_val,0,0);

#elif (MBB_CHG_BQ27510 == FEATURE_ON)
        rtn_vbat_val = hisi_battery_voltage();
#else
        tmp_ret = bsp_hkadc_convert(CHG_BATT_VOLT_HKADC_ID, &tmp_rtn_vbat_val);
        printk("tmp_rtn_vbat_val =%d\r\n", tmp_rtn_vbat_val);
        rtn_vbat_val = (int32_t)(tmp_rtn_vbat_val * 800 /330);
        printk("tmp_rtn_vbat_val =%d\r\n", BAT_VOL_MUIT_NUMBER);
                rtn_vbat_val = (int32_t)(tmp_rtn_vbat_val * BAT_VOL_MUIT_NUMBER);
        printk("rtn_vbat_val =%d\r\n", rtn_vbat_val);
        if (tmp_ret < 0)
        {
            chg_print_level_message( CHG_MSG_ERR,"fail to convert, return value %d\n",
                                            tmp_ret,0,0);
        }
        else
        {
            chg_print_level_message( CHG_MSG_DEBUG,"CHG:VBAT = %d\n", rtn_vbat_val,0,0);
        }
#endif/*MBB_CHG_COULOMETER == FEATURE_ON*/
    }
    else if (CHG_PARAMETER__BATT_THERM_DEGC == param_type)
    {
#if (MBB_CHG_COULOMETER == FEATURE_ON)
        rtn_vbat_val = bsp_hkadc_get_volt(CHG_BATT_THERM_HKADC_ID);
        if (rtn_vbat_val < 0)
        {
            chg_print_level_message( CHG_MSG_ERR,
                "bsp_hkadc_get_volt: get adc value failed!!!\r\n", 0,0,0);
            return rtn_vbat_val;
        }
#else
        ret = bsp_hkadc_convert(CHG_BATT_THERM_HKADC_ID, &tmp_rtn_vbat_val);
        if(ret)
        {
            chg_print_level_message( CHG_MSG_ERR,
                "chg_boot_get_volt_from_adc: get adc value failed!!!\r\n", 0,0,0);
            return ret;
        }
        rtn_vbat_val = (int32_t)tmp_rtn_vbat_val;
#endif
        chg_print_level_message( CHG_MSG_DEBUG,"CHG:BAT_TEMP = %d\n", rtn_vbat_val,0,0);

	}
    else if (CHG_PARAMETER__VBUS_VOLT == param_type)
    {
        ret = bsp_hkadc_convert(CHG_VBUS_VOLT_HKADC_ID, &tmp_rtn_vbat_val);
        if(ret)
        {
            chg_print_level_message( CHG_MSG_ERR,"chg_boot_get_volt_from_adc: get adc value failed!!!\r\n ",0,0,0 );
            return ret;
        }
        rtn_vbat_val = (int32_t)tmp_rtn_vbat_val;
        chg_print_level_message( CHG_MSG_DEBUG,"CHG:VBUS = %d\n", rtn_vbat_val,0,0);

    }
    else if (CHG_PARAMETER__BATTERY_ID_VOLT == param_type)
    {
        ret = bsp_hkadc_convert(CHG_BATT_ID_VOLT_HKADC_ID, &tmp_rtn_vbat_val);
        if(ret)
        {
            chg_print_level_message( CHG_MSG_ERR,"chg_boot_get_volt_from_adc: get adc value failed!!!\r\n ",0,0,0 );
            return ret;
        }
        rtn_vbat_val = (int32_t)tmp_rtn_vbat_val;
        chg_print_level_message( CHG_MSG_DEBUG,"CHG:VBUS = %d\n", rtn_vbat_val,0,0);

    }
    /*USB温保*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    else if (CHG_PARAMETER__USB_PORT_TEMP_DEGC == param_type)
    {
        ret = bsp_hkadc_convert(CHG_USB_TEMP_CHAN, &tmp_rtn_vbat_val);
        if(ret)
        {
            chg_print_level_message( CHG_MSG_ERR,"chg_boot_get_usb_volt_from_adc: get adc value failed!!!\r\n ",0,0,0 );
            return ret;
        }
        rtn_vbat_val = (int32_t)tmp_rtn_vbat_val;
        chg_print_level_message( CHG_MSG_DEBUG,"CHG:USB_TEMP_VOLT = %d\n", rtn_vbat_val,0,0);
    } 
#endif
    else
    {
        chg_print_level_message( CHG_MSG_INFO,"VADC bad param_type %d\n", param_type,0,0);
        return 0;
    }

    return rtn_vbat_val;

#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)

    int32_t rtn_vadc_val = 0;
    int32_t cnt = 0;
    int32_t param_type_match_flag = 0;

    if (param_type >= CHG_PARAMETER__INVALID)
    {
        chg_print_level_message( CHG_MSG_INFO,"CHG_PLT:chg_get_volt_from_adc: invalid param_type %d\n", param_type);
        return 0;
    }

    for(cnt = 0; cnt < ARRAY_SIZE(chg_adc_adapter_tbl); cnt++)
    {
        if (param_type == chg_adc_adapter_tbl[cnt].chg_ch_typ)
        {
            param_type_match_flag = 1;

            rtn_vadc_val = get_volt_from_adc_channel(chg_adc_adapter_tbl[cnt].adc_ch_sel);

            if (CHG_PARAMETER__BATTERY_VOLT == param_type)
            {
                rtn_vadc_val = rtn_vadc_val * BAT_VOL_MUIT_NUMBER;
            }
            if (CHG_PARAMETER__VBUS_VOLT == param_type)
            {
                rtn_vadc_val = rtn_vadc_val * VPH_PWR_VOL_MUIT_NUMBER;
            }

            chg_print_level_message( CHG_MSG_INFO,"CHG_PLT:chg_get_volt_from_adc: %s = %d\n", chg_adc_adapter_tbl[cnt].chg_ch_name, rtn_vadc_val);
            break;
        }
    }

    if (0 == param_type_match_flag)
    {
        chg_print_level_message( CHG_MSG_ERR,"CHG_PLT:chg_get_volt_from_adc:Can't find param_type %d in chg_boot_adc_adapter_tbl[]!\n", param_type);
        return 0;
    }
    return rtn_vadc_val;

#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/
}


int32_t chg_get_batt_volt_value(void)
{
    int32_t vbatt_from_adc = 0;
    int32_t rtn_vbatt_value = 0;
    if (FALSE == g_is_batt_volt_calib_init)
    {
        chg_batt_volt_calib_init();
    }

    vbatt_from_adc = chg_get_volt_from_adc(CHG_PARAMETER__BATTERY_VOLT);
    rtn_vbatt_value = chg_calc_batt_volt_calib_value(vbatt_from_adc);

    return rtn_vbatt_value;
}


int32_t chg_get_vph_pwr_volt_value(void)
{
    int32_t vbatt_from_adc = 0;
    if (FALSE == g_is_batt_volt_calib_init)
    {
        chg_batt_volt_calib_init();
    }

    vbatt_from_adc = chg_get_volt_from_adc(CHG_PARAMETER__VPH_PWR_VOLT);
    chg_print_level_message( CHG_MSG_DEBUG,"CHG_PLT: vph_pwr_volt=%d\r\n",vbatt_from_adc);
    return vbatt_from_adc;
}



int32_t chg_volt_to_temp (int32_t volt_value, CHG_TEMP_ADC_TYPE *AdcTable, uint32_t table_size)
{
    boolean   desending_flag = TRUE;
    uint32_t  idx = 0;

    if(NULL == AdcTable)
    {
        chg_print_level_message( CHG_MSG_ERR,"CHG_PLT: invalid input para!!!\r\n ");
        /*直接返回电压*/
        return volt_value;
    }

    /*判断表是按电压值升序or降序*/
    if(1 < table_size)
    {
        if(AdcTable[0].voltage < AdcTable[1].voltage)
        {
            desending_flag = FALSE;
        }
    }

    /*查找入参在表格中的位置*/
    while(idx < table_size)
    {
        if((TRUE == desending_flag) && (AdcTable[idx].voltage < volt_value))
        {
            break;
        }
        else if((FALSE == desending_flag) && (AdcTable[idx].voltage > volt_value))
        {
            break;
        }
        else
        {
            idx++;
        }
    }

    /*极限值判断，其他值公式求解*/
    if(0 == idx)
    {
        return AdcTable[0].temperature;
    }
    else if(table_size == idx)
    {
        return AdcTable[table_size - 1].temperature;
    }
    else
    {
        /*防止斜率计算非法*/
        if(AdcTable[idx - 1].voltage == AdcTable[idx].voltage)
        {
            chg_print_level_message( CHG_MSG_ERR,"CHG_PLT: slop invalid!!!\r\n ");
            return AdcTable[idx - 1].temperature;
        }

        return (((AdcTable[idx - 1].temperature - AdcTable[idx].temperature )
            * (volt_value - AdcTable[idx - 1].voltage)) / (AdcTable[idx - 1].voltage - AdcTable[idx].voltage)
            + AdcTable[idx - 1].temperature);
    }
}


int32_t chg_get_temp_value(CHG_PARAMETER_ENUM param_type)
{
    int32_t voltage_from_adc = 0;
    int32_t rtn_temp_value = 0;

    voltage_from_adc = chg_get_volt_from_adc(param_type);

    chg_print_level_message(CHG_MSG_DEBUG,"CHG_PLT:voltage_from_adc =%d \r\n ",voltage_from_adc);

    switch(param_type)
    {
        case CHG_PARAMETER__BATT_THERM_DEGC:
        {
#if ((MBB_CHG_BQ27510 == FEATURE_ON) && (MBB_CHG_PLATFORM_BALONG == FEATURE_ON))
            rtn_temp_value = hisi_battery_temperature();
#else
#if (MBB_CHG_EXTCHG == FEATURE_ON)
#if defined(BSP_CONFIG_BOARD_E5_DC04)
            if (TRUE == chg_stm_get_extchg_online_st() && TRUE == chg_stm_is_extchg_enabled())
            {
                if (TRUE == chg_is_extchg_in_otg_mode())
                {
                    rtn_temp_value = chg_volt_to_temp(voltage_from_adc, 
                        g_adc_batt_therm_map_extchg_otg,
                        sizeof(g_adc_batt_therm_map_extchg_otg)
                        / sizeof(g_adc_batt_therm_map_extchg_otg[0]));
                }
                else
                {
                     rtn_temp_value = chg_volt_to_temp(voltage_from_adc, 
                         g_adc_batt_therm_map_extchg,
                         sizeof(g_adc_batt_therm_map_extchg)
                         / sizeof(g_adc_batt_therm_map_extchg[0]));
                }
            }
            else
            {
                rtn_temp_value = chg_volt_to_temp(voltage_from_adc,
                    g_adc_batt_therm_map,
                    sizeof(g_adc_batt_therm_map)
                    / sizeof(g_adc_batt_therm_map[0]));
            }
#else
            rtn_temp_value = chg_volt_to_temp(voltage_from_adc,
                g_adc_batt_therm_map,
                sizeof(g_adc_batt_therm_map)
                / sizeof(g_adc_batt_therm_map[0]));
#endif
#else/*MBB_CHG_EXTCHG != FEATURE_ON*/
            rtn_temp_value = chg_volt_to_temp(voltage_from_adc, g_adc_batt_therm_map,
                                        sizeof(g_adc_batt_therm_map) / sizeof(g_adc_batt_therm_map[0]));
#endif/*MBB_CHG_EXTCHG == FEATURE_ON*/
#endif/*(MBB_CHG_BQ27510 == FEATURE_ON) && (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)*/
            chg_print_level_message( CHG_MSG_DEBUG,"CHG_PLT:rtn_temp_value =%d \r\n ",rtn_temp_value);

            break;
        }
        /*板级温度，对应的电压温度对照表使用电池的电压温度对照表，不同产品根据需要重新
         添加板级温度的电压温度对照表*/
        case CHG_PARAMETER__BATT_BOT_THERM_DEGC:
        {
            rtn_temp_value = chg_volt_to_temp(voltage_from_adc, g_adc_batt_therm_map,
                                        sizeof(g_adc_batt_therm_map) / sizeof(g_adc_batt_therm_map[0]));
            break;
        }
#if (FEATURE_ON == MBB_DCM_CUST_CHG_THERMAL)
        /*如果是采集面壳温度，直接从HKADC驱动返回*/
        case CHG_PARAMETER__SURFACE_THERM_DEGC:
        {
            rtn_temp_value = (int32_t)bsp_hkadc_dcm_get_surface_temp();
            break;
        }
#endif
        /*USB温保*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
        case CHG_PARAMETER__USB_PORT_TEMP_DEGC:
        {
            rtn_temp_value = chg_volt_to_temp(voltage_from_adc, g_adc_usb_therm_map,
                                        sizeof(g_adc_usb_therm_map) / sizeof(g_adc_usb_therm_map[0]));
            chg_print_level_message( CHG_MSG_DEBUG,"CHG_PLT:USB_TEMP:rtn_temp_value =%d.\n",rtn_temp_value);
            break;
        } 
#endif
        default:
        {
            /*先返回电压值*/
            rtn_temp_value = voltage_from_adc;
            break;
        }
    }

    return rtn_temp_value;
}

#if (MBB_CHG_EXTCHG == FEATURE_ON)
/*以下函数不同平台内部实现不同，移植人员请注意*/

int extchg_gpio_init(void)
{
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
    /*Balong平台不适用GPIO控制*/
#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
    if (!gpio_is_valid(EXTCHG_CONTROL_GPIO_EN1))         /* */
    {
        chg_print_level_message( CHG_MSG_DEBUG,"CHG_PLT: set EXTCHG_EN1 GPIO59 failed!!\r\n ");
        return -1;
    }
    if (!gpio_is_valid(EXTCHG_CONTROL_GPIO_EN2))         /* */
    {
        chg_print_level_message( CHG_MSG_DEBUG,"CHG_PLT: set EXTCHG_EN2 GPIO60 failed!!\r\n ");
        return -1;
    }
    if (gpio_request(EXTCHG_CONTROL_GPIO_EN1, "EXTCHG_EN1"))         /* */
    {
        chg_print_level_message( CHG_MSG_DEBUG,"CHG_PLT: request EXTCHG_EN1 GPIO59 failed!!\r\n ");
        return -1;
    }
    if (gpio_request(EXTCHG_CONTROL_GPIO_EN2, "EXTCHG_EN2"))         /* */
    {
        chg_print_level_message( CHG_MSG_DEBUG,"CHG_PLT: request EXTCHG_EN2 GPIO60 failed!!\r\n ");
        return -1;
    }
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/
    return 0;
}

void extchg_gpio_control(uint32_t gpio,int32_t level)
{
   gpio_direction_output(gpio,level);
}

#endif/*MBB_CHG_EXTCHG*/


#if (MBB_CHG_WIRELESS == FEATURE_ON)
/*以下函数不同平台内部实现不同，移植人员请注意*/

int wireless_gpio_init(void)
{
    if (!gpio_is_valid(WIRELESS_CONTROL_GPIO_EN1))         /* */
    {
        chg_print_level_message( CHG_MSG_DEBUG,"CHG_PLT: set WIRELESS_EN1 GPIO59 failed!!\r\n ");
        return -1;
    }
    if (!gpio_is_valid(WIRELESS_CONTROL_GPIO_EN2))         /* */
    {
        chg_print_level_message( CHG_MSG_DEBUG,"CHG_PLT: set WIRELESS_EN2 GPIO60 failed!!\r\n ");
        return -1;
    }
    if (gpio_request(WIRELESS_CONTROL_GPIO_EN1, "WIRELESS_EN1"))         /* */
    {
        chg_print_level_message( CHG_MSG_DEBUG,"CHG_PLT: request WIRELESS_EN1 GPIO59 failed!!\r\n ");
        return -1;
    }
    if (gpio_request(WIRELESS_CONTROL_GPIO_EN2, "WIRELESS_EN2"))         /* */
    {
        chg_print_level_message( CHG_MSG_DEBUG,"CHG_PLT: request WIRELESS_EN2 GPIO60 failed!!\r\n ");
        return -1;
    }
    return 0;
}

void wireless_gpio_control(uint32_t gpio,int32_t level)
{
   gpio_direction_output(gpio,level);
}

#endif/*MBB_CHG_WIRELESS*/


#ifdef CHG_STUB
boolean is_in_update_mode(void)
{
    return FALSE;
}
#elif (FEATURE_ON == MBB_CHG_PLATFORM_QUALCOMM)
boolean is_in_update_mode(void)
{
    static_smem_vendor0 *smem_data = NULL;
    smem_data = (static_smem_vendor0 *)SMEM_HUAWEI_ALLOC(SMEM_ID_VENDOR0, sizeof(static_smem_vendor0));
    if(NULL == smem_data)
    {
        return FALSE;
    }
    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#elif (FEATURE_ON == MBB_CHG_PLATFORM_BALONG)
boolean is_in_update_mode(void)
{
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;


    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif/*CHG_STUB*/

/**********************************************************************
函 数 名  : int32_t chg_config_para_read(int32_t num)
功能描述  : 读充电可配置参数
输入参数  : none
输出参数  : 无。
返 回 值  : 无。
注意事项  : 可根据平台不同修改实现方法，比如NV等。
***********************************************************************/
int32_t chg_config_para_read(uint16_t nvID, void *pItem, uint32_t ulLength)
{
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
    if(bsp_nvm_read(nvID,pItem,ulLength))
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:READ NV ERROR!!!\n");
        return CHG_ERROR;
    }
    else
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:READ NV OK!!!\n");
        return CHG_OK;
    }
#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
    /*高通平台不支持*/
    return CHG_ERROR;
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/
}

#if (MBB_CHG_COMPENSATE == FEATURE_ON)

#ifdef CHG_STUB
void chg_set_fact_release_mode(boolean on)
{
    return;
}
#else

#ifdef  CONFIG_MBB_KERNEL_NOTIFY
extern int huawei_notify_kernel_A2M(unsigned int notify_id);
#endif

void chg_set_fact_release_mode(boolean on)
{
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
    /*Balong平台不使用共享内存的方式*/
#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)

    dynamic_smem_vendor1 *smem_data = NULL;

#ifdef  CONFIG_MBB_KERNEL_NOTIFY
    int ret = -1 ;
#endif/*CONFIG_MBB_KERNEL_NOTIFY*/
    smem_data = (dynamic_smem_vendor1 *)SMEM_HUAWEI_ALLOC(SMEM_ID_VENDOR1 ,
                                        sizeof(dynamic_smem_vendor1 ));
    if(NULL == smem_data)
    {
        chg_print_level_message( CHG_MSG_ERR,"CHG_PLT: smem alloc fail!!!\r\n ");
        return;
    }

    if (TRUE == on)
    {
        smem_data->smem_fact_chg_flag = SMEM_FACT_RELEASE_ON;
    }
    else
    {
        smem_data->smem_fact_chg_flag = SMEM_FACT_RELEASE_OFF;
    }

#ifdef  CONFIG_MBB_KERNEL_NOTIFY
    ret = huawei_notify_kernel_A2M(NOTIFY_ID_0);
    if ( ret < 0 )
    {
        printk(KERN_EMERG "%s: huawei_notify_kernel_A2M(0) fail \n",__func__);
    }
#endif/*CONFIG_MBB_KERNEL_NOTIFY*/

#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/

    return;
}
#endif/*CHG_STUB*/
#endif/*MBB_CHG_COMPENSATE == FEATURE_ON*/


#ifdef CHG_STUB
void chg_get_hw_version_id(void)
{
}
#else
void chg_get_hw_version_id(void)
{
#if (MBB_CHG_PLATFORM_BALONG == FEATURE_ON)
    g_hardware_version_id = mbb_version_get_board_type();
    chg_print_level_message(CHG_MSG_ERR, 
        "CHG_PLT:get g_hardware_version_id = 0x%x.\n",g_hardware_version_id);
#elif (MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON)
        dynamic_smem_vendor1 *smem_data = NULL;
        smem_data = (dynamic_smem_vendor1 *)SMEM_HUAWEI_ALLOC(SMEM_ID_VENDOR1 ,
                                             sizeof(dynamic_smem_vendor1 ));
        if(NULL == smem_data)
        {
                chg_print_level_message(CHG_MSG_ERR, "chg_get_hw_version_id smem_data is NULL\n");
                return;
        }
        g_hardware_version_id = (uint32_t)smem_data->hwid.hard_main_version_id;
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:get g_hardware_version_id == %d\n",g_hardware_version_id);
#endif/*MBB_CHG_PLATFORM_QUALCOMM == FEATURE_ON*/
}

#endif/*CHG_STUB*/

