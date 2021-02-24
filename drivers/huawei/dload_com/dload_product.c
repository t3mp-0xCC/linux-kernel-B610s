
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



#include <linux/kernel.h>
#include "bsp_sram.h"
#include "bsp_shared_ddr.h"
#include "dload_nark_api.h"
#include "dload_product.h"
#include <bsp_version.h>

#ifdef  __cplusplus
    extern "C"{
#endif

/******************************************************************************
  1 ģ��˽�� (�ꡢö�١��ṹ�塢�Զ�����������) ������:
******************************************************************************/
#define DLOAD_PRINT(level, fmt, ...) (printk(level"[*DLOAD_PRODUCT*] %s:%d: "fmt"\n", __func__, __LINE__, ##__VA_ARGS__))


#define PRODUCT_OFFSET              (16)
#define SHOW_NOT_DELAY             (0)       /*��ʱ0s*/

static uint8 g_product_id = 0;
static uint16 g_big_ver = 0;
static boolean dload_burn = FALSE;

/* ·����ʶ�붨�� */
typedef enum 
{
    LCD_DEV_NAME,       /* LCD�豸���� */
    LCD_PIC_SUCCESS,    /* �����ɹ�ͼƬ���� */
    LCD_PIC_FAILED,     /* ����ʧ��ͼƬ���� */
    LCD_PIC_AUTOUG,     /* ��������ͼƬ���� */
    LCD_PIC_LOWPOWER,   /* �͵�ͼƬ���� */
    LCD_PIC_INSTALL0,   /* ��װ����ͼƬ0 */
    LCD_PIC_INSTALL1,   /* ��װ����ͼƬ1 */
    LCD_PIC_INSTALL2,   /* ��װ����ͼƬ2 */
    LCD_PIC_INSTALL3,   /* ��װ����ͼƬ3 */
    LCD_PIC_INSTALL4,   /* ��װ����ͼƬ4 */
    LCD_PIC_INSTALL5,   /* ��װ����ͼƬ5 */
    LCD_PIC_INSTALL6,   /* ��װ����ͼƬ6 */
    LCD_PIC_INSTALL7,   /* ��װ����ͼƬ7 */
    LCD_PIC_INSTALL8,   /* ��װ����ͼƬ8 */
    LCD_PIC_INSTALL9,   /* ��װ����ͼƬ9 */
    LCD_PIC_INSTALL10,  /* ��װ����ͼƬ10 */   
    LCD_PIC_INSTALL11,  /* ��װ����ͼƬ11 */
    
    LED_ATTR_ONOFF,     /* LED��������·�� */
    LED_ATTR_BRIGHT,    /* LED��������·�� */
    LED_ATTR_DELAY_ON,  /* LED DELAY_ON����·�� */
    LED_ATTR_DELAY_OFF, /* LED DELAY_OFF����·�� */
    
    LED_SIGNAL1_ATTR_ONOFF,
    LED_SIGNAL2_ATTR_ONOFF,
    LED_SIGNAL3_ATTR_ONOFF,
    LED_MODE_RED_ATTR_ONOFF,
    LED_MODE_GREEN_ATTR_ONOFF,
    LED_MODE_BLUE_ATTR_ONOFF,

    LED_LTE_RED_ATTR_ONOFF,
    LED_LTE_RED_ATTR_DELAY_ON,
    LED_LTE_RED_ATTR_DELAY_OFF,
    LED_LTE_GREEN_ATTR_ONOFF,
    LED_LTE_GREEN_ATTR_DELAY_ON,
    LED_LTE_GREEN_ATTR_DELAY_OFF,
    LED_LTE_BLUE_ATTR_ONOFF,
    LED_LTE_BLUE_ATTR_DELAY_ON,
    LED_LTE_BLUE_ATTR_DELAY_OFF,

    LED_BAT_RED_ATTR_ONOFF,
    LED_BAT_GREEN_ATTR_ONOFF,
    LED_BAT_GREEN_ATTR_DELAY_ON,
    LED_BAT_GREEN_ATTR_DELAY_OFF, 

    LED_POWER_GREEN_ATTR_ONOFF,  
    LED_POWER_GREEN_ATTR_TRIGGER,   
    LED_POWER_GREEN_ATTR_DELAY_ON, 
    LED_POWER_GREEN_ATTR_DELAY_OFF,  
    LED_POWER_RED_ATTR_ONOFF,
    LED_WIFI_GREEN_ATTR_ONOFF,   
    LED_SIM_LED_GREEN_ATTR_ONOFF,
    LED_SIM_LED_RED_ATTR_ONOFF,
    LED_STATUS_GREEN_ATTR_ONOFF,
    LED_STATUS_GREEN_ATTR_TRIGGER,
    LED_STATUS_GREEN_ATTR_DELAY_ON,
    LED_STATUS_GREEN_ATTR_DELAY_OFF,
    LED_SIGNAL1_RED_ATTR_ONOFF, 
    LED_SIGNAL1_BLUE_ATTR_ONOFF,
    LED_SIGNAL2_BLUE_ATTR_ONOFF,
    LED_SIGNAL3_BLUE_ATTR_ONOFF,

    
    LED_WORK_STATUS_GREEN_ATTR_ONOFF,
    LED_WORK_STATUS_RED_ATTR_ONOFF,
    LED_WORK_STATUS_YELLOW_ATTR_ONOFF,
    LED_MODE_STATUS_GREEN_ATTR_ONOFF,
    LED_MODE_STATUS_RED_ATTR_ONOFF,
    LED_MODE_STATUS_RED_ATTR_TRIGGER,
    LED_MODE_STATUS_RED_ATTR_DELAY_ON,
    LED_MODE_STATUS_RED_ATTR_DELAY_OFF,
    LED_MODE_STATUS_YELLOW_ATTR_ONOFF,
    LED_SIGNAL1_GREEN_ATTR_ONOFF,
    LED_SIGNAL2_GREEN_ATTR_ONOFF,
    LED_SIGNAL3_GREEN_ATTR_ONOFF,
    LED_SIGNAL4_GREEN_ATTR_ONOFF,
    LED_SIGNAL4_ATTR_ONOFF,
    LED_SIGNAL5_ATTR_ONOFF,
    LED_POWER_WHITE_ATTR_ONOFF,
    LED_LAN_WHITE_ATTR_ONOFF,
    LED_WIFI_WHITE_ATTR_ONOFF,
}show_path_t;

/******************************************************************************
  2 ģ��˽�� (ȫ�ֱ���) ������:
******************************************************************************/

/* ����̬��ƷID����
    E5�� touch E5��BV7R5E   
         ͨ��E5(LED)��BV7R5EU
         ͨ��E5(LCD)��BV7R5EC
    CPE�ࣺBV7R5C
    hilink�� BV7R5HS
    stick�� BV7R5HS
    wingle: BV7R5W
 */
/*ƽ̨��ʶ����Ʒ��̬��Ӳ���汾��ID()��Ӳ���汾��ID,ƽ̨���ƣ����ڲ�Ʒ���ƣ�����ID*/
/* ����������Ʒ��̬����,��MRE5,RE5,NE5,HILINK,CPE,SLU,ALL�� */
/*  hilink��Ʒ -> HILINK
    �ǲü���webui E5 -> RE5
    �ü���webui E5 -> MRE5
    ndis E5 ->��NE5
    wingle��Ʒ ->  RE5
    cpe��Ʒ -> CPE
    solution��Ʒ -> SLU
*/
/*  debug�汾��release�汾ʹ�ú궨������,������TA�汾֮���޸�����ID��������
    ��Ʒ��ʼ������ʱ��Ҫͬʱ��������*/
#if (FEATURE_ON == MBB_BUILD_DEBUG)
/*debug�汾������Ϣ*/
STATIC product_info_t product_info_st[] = 
{
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X0, "V7R5", "B610s-76a",  "BV7R5C",  "CPE"},/*B610s-76a*/
    {DLOAD_BALONG, DLOAD_E5,  0X0, 0X1, "V7R5", "HW-01H",       "DC04",  "NE5"},
    {DLOAD_BALONG, DLOAD_E5,  0X0, 0X3, "V7R5", "505HW",       "BV7R5ESB03",  "NE5"},
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X1, "V7R5", "B618",  "B618",  "CPE"},/*B618s-22d*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X2, "V7R5", "B610s-77a",  "B610",  "CPE"},  /*B610s-77a*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X3, "V7R5", "B618",  "B618",  "CPE"},/*B618s-65d*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X6, "V7R5", "KD11",  "KD11",  "CPE"},/*KD11*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X21, "V7R5", "KD15",  "KD11",  "CPE"},/*KD15*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X9, "V7R5", "B612", "B612", "CPE"},/*B612s-51d*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X12, "V7R5", "B715",  "B715",  "CPE"},/*B715s-23c*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X15, "V7R5", "B612", "B612", "CPE"},/*B612s-25d*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X18, "V7R5", "B610s-79a",  "AIR3P",  "CPE"},/*B610s-79a*/
};
#else
STATIC product_info_t product_info_st[] = 
{
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X0, "V7R5", "B610s-76a",  "BV7R5C",  "CPE"},/*B610s-76a*/
    {DLOAD_BALONG, DLOAD_E5,  0X0, 0X1, "V7R5", "HW-01H",       "DC04__0",  "NE5"},
    {DLOAD_BALONG, DLOAD_E5,  0X0, 0X3, "V7R5", "505HW",       "BV7R5ESB03",  "NE5"},
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X1, "V7R5", "B618",  "B618__0",  "CPE"},/*B618s-22d*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X2, "V7R5", "B610s-77a",  "B610__0",  "CPE"},/*B610s-77a*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X3, "V7R5", "B618",  "B618__0",  "CPE"},/*B618s-65d*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X6, "V7R5", "KD11",  "KD11__0",  "CPE"},/*KD11*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X21, "V7R5", "KD15",  "KD11__0",  "CPE"},/*KD15*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X9, "V7R5", "B612", "B612__0", "CPE"},/*B612s-51d*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X12, "V7R5", "B715",  "B715__0",  "CPE"},/*B715s-23c*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X15, "V7R5", "B612", "B612__0", "CPE"},/*B612s-25d*/
    {DLOAD_BALONG, DLOAD_CPE, 0X4, 0X18, "V7R5", "B610s-79a",  "AIR3P",  "CPE"},/*B610s-79a*/
};
#endif

/* �����������ñ�
   ��ƷӲ����ID, ��ƷӲ����ID, USB����, SD����
   ��������, ��ȫ����, HOTA����, FOTA����, FOTA���������Ϣ�ֶ�
   ��ȫ����v1.3���������Ϣ, Ԥ�� */
STATIC dload_feature_t dload_feature_st[] = 
{
    {0X4, 0X0, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, {FALSE}, {TRUE, TRUE, FALSE, TRUE}, {FALSE}},      /* B610s-76a */
    {0X0, 0X1, TRUE,  FALSE, TRUE, TRUE, FALSE, FALSE, {FALSE}, {TRUE, TRUE, FALSE, TRUE}, {FALSE}},
    {0X0, 0X3, TRUE,  FALSE, TRUE, TRUE, FALSE, FALSE, {FALSE}, {TRUE, TRUE, FALSE, TRUE}, {FALSE}},
    {0X4, 0X1, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, {FALSE}, {TRUE, TRUE, TRUE, TRUE}, {FALSE}},	  /* B618s-22d */	
    {0X4, 0X2, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, {FALSE}, {TRUE, TRUE, TRUE, TRUE}, {FALSE}},      /* B610s-77a */
    {0X4, 0X3, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, {FALSE}, {TRUE, TRUE, TRUE, TRUE}, {FALSE}},      /* B618s-65d */
    {0X4, 0X6, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, {FALSE}, {TRUE, TRUE, TRUE,  TRUE}, {FALSE}},      /* KD11 */
    {0X4, 0X21, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, {FALSE}, {TRUE, TRUE, TRUE,  TRUE}, {FALSE}},      /* KD15 */
    {0X4, 0X9, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, {FALSE}, {TRUE, TRUE, TRUE, TRUE}, {FALSE}},      /* B612s-51d */
    {0X4, 0X12, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, {FALSE}, {TRUE, TRUE, TRUE, TRUE}, {FALSE}},      /* B715s-23c */
    {0X4, 0X15, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, {FALSE}, {TRUE, TRUE, TRUE, TRUE}, {FALSE}},      /* B612s-25d */
    {0X4, 0X18, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, {FALSE}, {TRUE, TRUE, TRUE, TRUE}, {FALSE}},      /* B610s-79a */
};

/* ���ݲ�Ʒ����״̬���� �������*/
#ifdef BSP_CONFIG_BOARD_CPE_B612
STATIC path_info_t pathinfo_st[] =
{
    {LED_POWER_WHITE_ATTR_ONOFF,      "/sys/class/leds/power_led:white/brightness"},
    {LED_MODE_RED_ATTR_ONOFF,         "/sys/class/leds/mode_led:red/brightness"},
    {LED_MODE_GREEN_ATTR_ONOFF,       "/sys/class/leds/mode_led:green/brightness"},
    {LED_MODE_BLUE_ATTR_ONOFF,        "/sys/class/leds/mode_led:blue/brightness"},
    {LED_LAN_WHITE_ATTR_ONOFF,        "/sys/class/leds/lan_led:white/brightness"},
    {LED_WIFI_WHITE_ATTR_ONOFF,       "/sys/class/leds/wifi_led:white/brightness"},
    {LED_SIGNAL1_ATTR_ONOFF,          "/sys/class/leds/signal1_led:white/brightness"},
    {LED_SIGNAL2_ATTR_ONOFF,          "/sys/class/leds/signal2_led:white/brightness"},
    {LED_SIGNAL3_ATTR_ONOFF,          "/sys/class/leds/signal3_led:white/brightness"},
    {LED_SIGNAL4_ATTR_ONOFF,          "/sys/class/leds/signal4_led:white/brightness"},
    {LED_SIGNAL5_ATTR_ONOFF,          "/sys/class/leds/signal5_led:white/brightness"},
};

STATIC upstate_info_t upstateinfo_st[] =
{
    /* LED��ʾ���� */
    /* ͨ������״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_POWER_WHITE_ATTR_ONOFF},  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_RED_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_GREEN_ATTR_ONOFF},   SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_BLUE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_WIFI_WHITE_ATTR_ONOFF},   SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LAN_WHITE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL2_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL3_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL4_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL5_ATTR_ONOFF},      SHOW_NOT_DELAY},

    /* ����״̬��ʾ���� */
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_POWER_WHITE_ATTR_ONOFF},  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_RED_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_GREEN_ATTR_ONOFF},   SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_BLUE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_WIFI_WHITE_ATTR_ONOFF},   SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_LAN_WHITE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL2_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL3_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL4_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL5_ATTR_ONOFF},      SHOW_NOT_DELAY},

    /* ͨ�óɹ�״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_POWER_WHITE_ATTR_ONOFF},  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_MODE_RED_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_MODE_GREEN_ATTR_ONOFF},   SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_MODE_BLUE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_WIFI_WHITE_ATTR_ONOFF},   SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LAN_WHITE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL1_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL2_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL3_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL4_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL5_ATTR_ONOFF},      SHOW_NOT_DELAY},

    /* ͨ��ʧ��״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                          SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_POWER_WHITE_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                          SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_MODE_RED_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_MODE_GREEN_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_MODE_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_WIFI_WHITE_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LAN_WHITE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_ATTR_ONOFF},         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                          SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL2_ATTR_ONOFF},         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                          SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL3_ATTR_ONOFF},         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL4_ATTR_ONOFF},         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL5_ATTR_ONOFF},         SHOW_NOT_DELAY},

    /* ʧ��״̬��ʾ���� */
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_POWER_WHITE_ATTR_ONOFF},            SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_MODE_RED_ATTR_ONOFF},               SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_MODE_GREEN_ATTR_ONOFF},             SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_MODE_BLUE_ATTR_ONOFF},              SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_WIFI_WHITE_ATTR_ONOFF},             SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_LAN_WHITE_ATTR_ONOFF},              SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_ATTR_ONOFF},                SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL2_ATTR_ONOFF},                SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL3_ATTR_ONOFF},                SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL4_ATTR_ONOFF},                SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL5_ATTR_ONOFF},                SHOW_NOT_DELAY},
};
/* ���ݲ�Ʒ����״̬���� �������*/
#elif defined(BSP_CONFIG_BOARD_CPE_B618)
STATIC path_info_t pathinfo_st[] = 
{
    /* ������ʵ��led���ֲ���Ӧ������ΪB618���豸���б仯 */
    {LED_POWER_GREEN_ATTR_ONOFF,      "/sys/class/leds/power_led:green/brightness"},
    {LED_WIFI_GREEN_ATTR_ONOFF,       "/sys/class/leds/wifi_led:white/brightness"},
    {LED_SIM_LED_GREEN_ATTR_ONOFF,    "/sys/class/leds/lan_led:white/brightness"},
    {LED_LTE_GREEN_ATTR_ONOFF,        "/sys/class/leds/mode_led:green/brightness"},
    {LED_LTE_RED_ATTR_ONOFF,          "/sys/class/leds/mode_led:red/brightness"},
    {LED_SIGNAL1_RED_ATTR_ONOFF,      "/sys/class/leds/signal1_led:red/brightness"},
    {LED_SIGNAL1_BLUE_ATTR_ONOFF,     "/sys/class/leds/signal1_led:white/brightness"},
    {LED_SIGNAL2_BLUE_ATTR_ONOFF,     "/sys/class/leds/signal2_led:white/brightness"},
    {LED_SIGNAL3_BLUE_ATTR_ONOFF,     "/sys/class/leds/signal3_led:white/brightness"},
    {LED_STATUS_GREEN_ATTR_ONOFF,     "/sys/class/leds/status_led:green/brightness"},
    {LED_STATUS_GREEN_ATTR_TRIGGER,   "/sys/class/leds/status_led:green/trigger"},
    {LED_STATUS_GREEN_ATTR_DELAY_ON,  "/sys/class/leds/status_led:green/delay_on"},
    {LED_STATUS_GREEN_ATTR_DELAY_OFF, "/sys/class/leds/status_led:green/delay_off"},
};

STATIC upstate_info_t upstateinfo_st[] = 
{  
    /* LED��ʾ���� */
    /* ͨ������״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "255"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_POWER_GREEN_ATTR_ONOFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_WIFI_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIM_LED_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "timer"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_TRIGGER},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "200"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_OFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "300"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_ON},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL2_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL3_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},

    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "255"},                    SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_POWER_GREEN_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_WIFI_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIM_LED_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "timer"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_TRIGGER},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "200"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_OFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "300"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_ON},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL2_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL3_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    
    /* �����ɹ�״̬��ʾ���� ���鲥��������nv�ָ����ƣ���nv_base.c��ʵ�֣�*/
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL1_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL1_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL2_BLUE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},               SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL3_BLUE_ATTR_ONOFF},      SHOW_NOT_DELAY},

    /* ����ʧ��״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_POWER_GREEN_ATTR_ONOFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_WIFI_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIM_LED_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "timer"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_TRIGGER},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "200"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_OFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "300"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_ON},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL2_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL3_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_POWER_GREEN_ATTR_ONOFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_WIFI_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIM_LED_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "timer"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_TRIGGER},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "200"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_OFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "300"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_ON},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL2_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL3_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
};
#elif defined(BSP_CONFIG_BOARD_CPE_KD11)
STATIC path_info_t pathinfo_st[] = 
{
    {LED_WORK_STATUS_GREEN_ATTR_ONOFF,      "/sys/class/leds/work_status_led:green/brightness"},
    {LED_WORK_STATUS_RED_ATTR_ONOFF,        "/sys/class/leds/work_status_led:red/brightness"},
    {LED_WORK_STATUS_YELLOW_ATTR_ONOFF,     "/sys/class/leds/work_status_led:yellow/brightness"},
    {LED_MODE_STATUS_GREEN_ATTR_ONOFF,      "/sys/class/leds/mode_status_led:green/brightness"},
    {LED_MODE_STATUS_RED_ATTR_ONOFF,        "/sys/class/leds/mode_status_led:red/brightness"},
    {LED_MODE_STATUS_RED_ATTR_TRIGGER,      "/sys/class/leds/mode_status_led:red/trigger"},
    {LED_MODE_STATUS_RED_ATTR_DELAY_ON,     "/sys/class/leds/mode_status_led:red/delay_on"},
    {LED_MODE_STATUS_RED_ATTR_DELAY_OFF,    "/sys/class/leds/mode_status_led:red/delay_off"},
    {LED_MODE_STATUS_YELLOW_ATTR_ONOFF,     "/sys/class/leds/mode_status_led:yellow/brightness"},
    {LED_SIGNAL1_GREEN_ATTR_ONOFF,          "/sys/class/leds/signal1_led:green/brightness"},
    {LED_SIGNAL2_GREEN_ATTR_ONOFF,          "/sys/class/leds/signal2_led:green/brightness"},
    {LED_SIGNAL3_GREEN_ATTR_ONOFF,          "/sys/class/leds/signal3_led:green/brightness"},
    {LED_SIGNAL4_GREEN_ATTR_ONOFF,          "/sys/class/leds/signal4_led:green/brightness"},
};

STATIC upstate_info_t upstateinfo_st[] = 
{  
    /* LED��ʾ���� */
    /* ͨ������״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_WORK_STATUS_GREEN_ATTR_ONOFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_WORK_STATUS_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_WORK_STATUS_YELLOW_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_STATUS_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "timer"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_STATUS_RED_ATTR_TRIGGER},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "200"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_STATUS_RED_ATTR_DELAY_OFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "300"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_STATUS_RED_ATTR_DELAY_ON},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_STATUS_YELLOW_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_GREEN_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL2_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL3_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL4_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},

    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_WORK_STATUS_GREEN_ATTR_ONOFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_WORK_STATUS_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_WORK_STATUS_YELLOW_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_STATUS_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "timer"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_STATUS_RED_ATTR_TRIGGER},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "200"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_STATUS_RED_ATTR_DELAY_OFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "300"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_STATUS_RED_ATTR_DELAY_ON},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_STATUS_YELLOW_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_GREEN_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL2_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL3_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL4_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    /* �����ɹ�״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_WORK_STATUS_GREEN_ATTR_ONOFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_WORK_STATUS_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_WORK_STATUS_YELLOW_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_MODE_STATUS_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_MODE_STATUS_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_MODE_STATUS_YELLOW_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL1_GREEN_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL2_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL3_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL4_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},

    /* ����ʧ��״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_WORK_STATUS_GREEN_ATTR_ONOFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_WORK_STATUS_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_WORK_STATUS_YELLOW_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_MODE_STATUS_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_MODE_STATUS_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_MODE_STATUS_YELLOW_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_GREEN_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL2_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL3_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL4_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_WORK_STATUS_GREEN_ATTR_ONOFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_WORK_STATUS_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_WORK_STATUS_YELLOW_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_MODE_STATUS_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_MODE_STATUS_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_MODE_STATUS_YELLOW_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_GREEN_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL2_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL3_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL4_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
};
#elif defined(BSP_CONFIG_BOARD_CPE_B715)
STATIC path_info_t pathinfo_st[] =
{
    {LED_POWER_WHITE_ATTR_ONOFF,      "/sys/class/leds/power_led:white/brightness"},
    {LED_MODE_RED_ATTR_ONOFF,         "/sys/class/leds/mode_led:red/brightness"},
    {LED_MODE_GREEN_ATTR_ONOFF,       "/sys/class/leds/mode_led:green/brightness"},
    {LED_MODE_BLUE_ATTR_ONOFF,        "/sys/class/leds/mode_led:blue/brightness"},
    {LED_LAN_WHITE_ATTR_ONOFF,        "/sys/class/leds/lan_led:white/brightness"},
    {LED_WIFI_WHITE_ATTR_ONOFF,       "/sys/class/leds/wifi_led:white/brightness"},
    {LED_SIGNAL1_ATTR_ONOFF,          "/sys/class/leds/signal1_led:white/brightness"},
    {LED_SIGNAL2_ATTR_ONOFF,          "/sys/class/leds/signal2_led:white/brightness"},
    {LED_SIGNAL3_ATTR_ONOFF,          "/sys/class/leds/signal3_led:white/brightness"},
    {LED_SIGNAL4_ATTR_ONOFF,          "/sys/class/leds/signal4_led:white/brightness"},
    {LED_SIGNAL5_ATTR_ONOFF,          "/sys/class/leds/signal5_led:white/brightness"},
};

STATIC upstate_info_t upstateinfo_st[] =
{
    /* LED��ʾ���� */
    /* ͨ������״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_POWER_WHITE_ATTR_ONOFF},  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_RED_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_GREEN_ATTR_ONOFF},   SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_BLUE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_WIFI_WHITE_ATTR_ONOFF},   SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LAN_WHITE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL2_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL3_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL4_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL5_ATTR_ONOFF},      SHOW_NOT_DELAY},

    /* ����״̬��ʾ���� */
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_POWER_WHITE_ATTR_ONOFF},  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_RED_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_GREEN_ATTR_ONOFF},   SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_MODE_BLUE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_WIFI_WHITE_ATTR_ONOFF},   SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_LAN_WHITE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL2_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL3_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL4_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL5_ATTR_ONOFF},      SHOW_NOT_DELAY},

    /* ͨ�óɹ�״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_POWER_WHITE_ATTR_ONOFF},  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_MODE_RED_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_MODE_GREEN_ATTR_ONOFF},   SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_MODE_BLUE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_WIFI_WHITE_ATTR_ONOFF},   SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LAN_WHITE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL1_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL2_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL3_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL4_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL5_ATTR_ONOFF},      SHOW_NOT_DELAY},

    /* ͨ��ʧ��״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                          SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_POWER_WHITE_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                          SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_MODE_RED_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_MODE_GREEN_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_MODE_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_WIFI_WHITE_ATTR_ONOFF},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LAN_WHITE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_ATTR_ONOFF},         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                          SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL2_ATTR_ONOFF},         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                          SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL3_ATTR_ONOFF},         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL4_ATTR_ONOFF},         SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                            SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL5_ATTR_ONOFF},         SHOW_NOT_DELAY},

    /* ʧ��״̬��ʾ���� */
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_POWER_WHITE_ATTR_ONOFF},            SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_MODE_RED_ATTR_ONOFF},               SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_MODE_GREEN_ATTR_ONOFF},             SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_MODE_BLUE_ATTR_ONOFF},              SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_WIFI_WHITE_ATTR_ONOFF},             SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_LAN_WHITE_ATTR_ONOFF},              SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_ATTR_ONOFF},                SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL2_ATTR_ONOFF},                SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL3_ATTR_ONOFF},                SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL4_ATTR_ONOFF},                SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                                   SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL5_ATTR_ONOFF},                SHOW_NOT_DELAY},
};
/* ���ݲ�Ʒ����״̬���� �������*/

#elif (FEATURE_ON == MBB_DLOAD_MCPEUP)
#define SHOW_YES_DELAY         (200000) /*��ʱ200ms*/
STATIC path_info_t pathinfo_st[] = 
{
    {LED_POWER_GREEN_ATTR_ONOFF,      "/sys/class/leds/power_led:green/brightness"},
    {LED_POWER_GREEN_ATTR_TRIGGER,    "/sys/class/leds/power_led:green/trigger"},
    {LED_POWER_GREEN_ATTR_DELAY_ON,   "/sys/class/leds/power_led:green/delay_on"},
    {LED_POWER_GREEN_ATTR_DELAY_OFF,  "/sys/class/leds/power_led:green/delay_off"},        
    {LED_POWER_RED_ATTR_ONOFF,        "/sys/class/leds/power_led:red/brightness"},
    {LED_WIFI_GREEN_ATTR_ONOFF,       "/sys/class/leds/wifi_led:green/brightness"},
    {LED_SIM_LED_GREEN_ATTR_ONOFF,    "/sys/class/leds/sim_led:green/brightness"},
    {LED_SIM_LED_RED_ATTR_ONOFF,      "/sys/class/leds/sim_led:red/brightness"},
    {LED_LTE_GREEN_ATTR_ONOFF,        "/sys/class/leds/lte_led:green/brightness"},
    {LED_LTE_RED_ATTR_ONOFF,          "/sys/class/leds/lte_led:red/brightness"},
    {LED_STATUS_GREEN_ATTR_ONOFF,     "/sys/class/leds/status_led:green/brightness"},
    {LED_STATUS_GREEN_ATTR_TRIGGER,   "/sys/class/leds/status_led:green/trigger"},
    {LED_STATUS_GREEN_ATTR_DELAY_ON,  "/sys/class/leds/status_led:green/delay_on"},
    {LED_STATUS_GREEN_ATTR_DELAY_OFF, "/sys/class/leds/status_led:green/delay_off"},
    {LED_SIGNAL1_RED_ATTR_ONOFF,      "/sys/class/leds/signal1_led:red/brightness"},
    {LED_SIGNAL1_BLUE_ATTR_ONOFF,     "/sys/class/leds/signal1_led:blue/brightness"},
    {LED_SIGNAL2_BLUE_ATTR_ONOFF,     "/sys/class/leds/signal2_led:blue/brightness"},
    {LED_SIGNAL3_BLUE_ATTR_ONOFF,     "/sys/class/leds/signal3_led:blue/brightness"},
};

STATIC upstate_info_t upstateinfo_st[] = 
{  
    /* LED��ʾ���� */
    /* ͨ������״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "255"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_POWER_GREEN_ATTR_ONOFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_POWER_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_WIFI_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIM_LED_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "timer"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_TRIGGER},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "200"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_OFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "300"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_ON},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL2_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL3_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},

    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "255"},                    SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_POWER_GREEN_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_POWER_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_WIFI_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIM_LED_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "timer"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_TRIGGER},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "200"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_OFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "300"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_ON},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL1_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL2_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_SIGNAL3_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    
    /* �����ɹ�״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL1_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL1_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL2_BLUE_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "255"},               SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_SIGNAL3_BLUE_ATTR_ONOFF},      SHOW_NOT_DELAY},

    /* ����ʧ��״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_POWER_GREEN_ATTR_ONOFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_POWER_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_WIFI_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIM_LED_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "timer"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_TRIGGER},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "200"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_OFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "300"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_ON},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL2_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL3_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_POWER_GREEN_ATTR_ONOFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "255"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_POWER_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_WIFI_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIM_LED_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "timer"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_TRIGGER},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "200"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_OFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "300"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_STATUS_GREEN_ATTR_DELAY_ON},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_RED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL1_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL2_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_READ,   {.str_value  = "0"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,   LED_WRITE,  {.path_index = LED_SIGNAL3_BLUE_ATTR_ONOFF},       SHOW_NOT_DELAY},
};
#elif (FEATURE_ON == MBB_DLOAD_STICK || FEATURE_ON == MBB_DLOAD_HILINK || FEATURE_ON == MBB_DLOAD_WINGLE)
STATIC path_info_t pathinfo_st[] = 
{
    {LED_LTE_GREEN_ATTR_ONOFF,       "/sys/class/leds/lte_led:green/trigger"},
    {LED_LTE_GREEN_ATTR_DELAY_ON,    "/sys/class/leds/lte_led:green/delay_on"},
    {LED_LTE_GREEN_ATTR_DELAY_OFF,   "/sys/class/leds/lte_led:green/delay_off"},
    
};

STATIC upstate_info_t upstateinfo_st[] = 
{
    /* ����״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "timer"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_OFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_ON},     SHOW_NOT_DELAY},

    /* ͨ������״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "timer"},                    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "100"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_OFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "100"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_ON},  SHOW_NOT_DELAY},

    /* �����ɹ�״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "timer"},                    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "100"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_OFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "100"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_ON},  SHOW_NOT_DELAY},

    /* ����ʧ��״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "timer"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_OFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_ON},     SHOW_NOT_DELAY},

};

STATIC path_info_t pathinfo_burn_st[] = 
{
    {LED_LTE_GREEN_ATTR_ONOFF,       "/sys/class/leds/lte_led:green/trigger"},
    {LED_LTE_GREEN_ATTR_DELAY_ON,    "/sys/class/leds/lte_led:green/delay_on"},
    {LED_LTE_GREEN_ATTR_DELAY_OFF,   "/sys/class/leds/lte_led:green/delay_off"},
    {LED_LTE_BLUE_ATTR_ONOFF,        "/sys/class/leds/lte_led:blue/trigger"},
    {LED_LTE_BLUE_ATTR_DELAY_ON,     "/sys/class/leds/lte_led:blue/delay_on"},
    {LED_LTE_BLUE_ATTR_DELAY_OFF,    "/sys/class/leds/lte_led:blue/delay_off"},
    {LED_LTE_RED_ATTR_ONOFF,         "/sys/class/leds/lte_led:red/trigger"},
    {LED_LTE_RED_ATTR_DELAY_ON,      "/sys/class/leds/lte_led:red/delay_on"},
    {LED_LTE_RED_ATTR_DELAY_OFF,     "/sys/class/leds/lte_led:red/delay_off"},
    
};

STATIC upstate_info_t upstateinfo_burn_st[] = 
{
    /* ����״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "timer"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_OFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_ON},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "timer"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_BLUE_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_BLUE_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_BLUE_ATTR_DELAY_OFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "timer"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_DELAY_OFF},     SHOW_NOT_DELAY},    

    /* ͨ������״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "timer"},                    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "100"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_OFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "100"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_ON},  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "timer"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_BLUE_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_BLUE_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_BLUE_ATTR_DELAY_OFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "timer"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_DELAY_OFF},     SHOW_NOT_DELAY},    

    /* �����ɹ�״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "timer"},                    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "100"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_OFF}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "100"},                        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_ON},  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "timer"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_BLUE_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_BLUE_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_BLUE_ATTR_DELAY_OFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "timer"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_DELAY_OFF},     SHOW_NOT_DELAY},    

    /* ����ʧ��״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "timer"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_OFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_ON},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "timer"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_BLUE_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_BLUE_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_BLUE_ATTR_DELAY_OFF},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "timer"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_ONOFF},        SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "100"},                           SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_RED_ATTR_DELAY_OFF},     SHOW_NOT_DELAY},    
};
#elif (FEATURE_ON == MBB_DLOAD_E5)
#if (FEATURE_ON == MBB_DLOAD_LCD)
#define SHOW_YES_DELAY         (1000000) /*��ʱ1s*/
STATIC path_info_t pathinfo_st[] = 
{
    {LCD_DEV_NAME,          "/dev/fb0"},
    {LCD_PIC_SUCCESS,       "/pic/%d_%d/upgrade_success"},
    {LCD_PIC_FAILED,        "/pic/%d_%d/upgrade_fail"},
    {LCD_PIC_AUTOUG,        "/pic/%d_%d/auto_update"},
    {LCD_PIC_LOWPOWER,      "/pic/%d_%d/lowpower"},
    {LCD_PIC_INSTALL0,      "/pic/%d_%d/installing_0"},
    {LCD_PIC_INSTALL1,      "/pic/%d_%d/installing_1"},
    {LCD_PIC_INSTALL2,      "/pic/%d_%d/installing_2"},
    {LCD_PIC_INSTALL3,      "/pic/%d_%d/installing_3"},
    {LCD_PIC_INSTALL4,      "/pic/%d_%d/installing_4"},
    {LCD_PIC_INSTALL5,      "/pic/%d_%d/installing_5"},
    {LCD_PIC_INSTALL6,      "/pic/%d_%d/installing_6"},
    {LCD_PIC_INSTALL7,      "/pic/%d_%d/installing_7"},
    {LCD_PIC_INSTALL8,      "/pic/%d_%d/installing_8"},
    {LCD_PIC_INSTALL9,      "/pic/%d_%d/installing_9"},
    {LCD_PIC_INSTALL10,     "/pic/%d_%d/installing_10"},
    {LCD_PIC_INSTALL11,     "/pic/%d_%d/installing_11"},
};

/* LCD ����״̬�б� */
STATIC upstate_info_t upstateinfo_st[] = 
{
    /* LCD��ʾ���� */
    /* ����״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_IDLE,      LCD_READ,   {.path_index = LCD_PIC_INSTALL0},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,      LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    
    /* ͨ������״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL0},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL1},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL2},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL3},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL4},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL5},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL6},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL7},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL8},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL9},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL10},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL11},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},

    /* ������������״̬��ʾ���� */
    {DLOAD_STATE_AUTOUG_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_AUTOUG},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},

    /* �͵�״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_LOW_POWER, LCD_READ,   {.path_index = LCD_PIC_LOWPOWER},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_LOW_POWER, LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},

    /* �����ɹ�״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_SUCCESS,   LCD_READ,   {.path_index = LCD_PIC_SUCCESS},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},

    /* ����ʧ��״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_FAIL,      LCD_READ,   {.path_index = LCD_PIC_FAILED},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,      LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},

};
#else
STATIC path_info_t pathinfo_st[] = 
{
    {LED_LTE_GREEN_ATTR_ONOFF,       "/sys/class/leds/lte_led:green/trigger"},
    {LED_LTE_GREEN_ATTR_DELAY_ON,    "/sys/class/leds/lte_led:green/delay_on"},
    {LED_LTE_GREEN_ATTR_DELAY_OFF,   "/sys/class/leds/lte_led:green/delay_off"},
    {LED_BAT_GREEN_ATTR_ONOFF,       "/sys/class/leds/bat_led:green/trigger"},
    {LED_BAT_GREEN_ATTR_DELAY_ON,    "/sys/class/leds/bat_led:green/delay_on"},
    {LED_BAT_GREEN_ATTR_DELAY_OFF,   "/sys/class/leds/bat_led:green/delay_off"},
    
};
STATIC upstate_info_t upstateinfo_st[] = 
{
    /* ����״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "timer"},                     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_OFF},SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_ON}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "timer"},                     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_BAT_GREEN_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_BAT_GREEN_ATTR_DELAY_OFF},SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,   LED_WRITE,  {.path_index = LED_BAT_GREEN_ATTR_DELAY_ON}, SHOW_NOT_DELAY},

    /* ͨ������״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "timer"},                     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_OFF},SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_ON}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "timer"},                     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_BAT_GREEN_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_BAT_GREEN_ATTR_DELAY_OFF},SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_BAT_GREEN_ATTR_DELAY_ON}, SHOW_NOT_DELAY},

    /* �����ɹ�״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "timer"},                     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_OFF},SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_ON}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "timer"},                     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_BAT_GREEN_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_BAT_GREEN_ATTR_DELAY_OFF},SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_BAT_GREEN_ATTR_DELAY_ON}, SHOW_NOT_DELAY},

    /* ����ʧ��״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "timer"},                     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_OFF},SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_LTE_GREEN_ATTR_DELAY_ON}, SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "timer"},                     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_BAT_GREEN_ATTR_ONOFF},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_BAT_GREEN_ATTR_DELAY_OFF},SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_READ,   {.str_value  = "500"},                       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,   LED_WRITE,  {.path_index = LED_BAT_GREEN_ATTR_DELAY_ON}, SHOW_NOT_DELAY},


};
#endif
#else
#define SHOW_YES_DELAY         (1000000) /*��ʱ1s*/

STATIC path_info_t pathinfo_st[] = 
{
    {LCD_DEV_NAME,          "/dev/fb0"},
    {LCD_PIC_SUCCESS,       "/pic/%d_%d/upgrade_success"},
    {LCD_PIC_FAILED,        "/pic/%d_%d/upgrade_fail"},
    {LCD_PIC_AUTOUG,        "/pic/%d_%d/auto_update"},
    {LCD_PIC_LOWPOWER,      "/pic/%d_%d/lowpower"},
    {LCD_PIC_INSTALL0,      "/pic/%d_%d/installing_0"},
    {LCD_PIC_INSTALL1,      "/pic/%d_%d/installing_1"},
    {LCD_PIC_INSTALL2,      "/pic/%d_%d/installing_2"},
    {LCD_PIC_INSTALL3,      "/pic/%d_%d/installing_3"},
    {LCD_PIC_INSTALL4,      "/pic/%d_%d/installing_4"},
    {LCD_PIC_INSTALL5,      "/pic/%d_%d/installing_5"},
    {LCD_PIC_INSTALL6,      "/pic/%d_%d/installing_6"},
    {LCD_PIC_INSTALL7,      "/pic/%d_%d/installing_7"},
    {LCD_PIC_INSTALL8,      "/pic/%d_%d/installing_8"},
    {LCD_PIC_INSTALL9,      "/pic/%d_%d/installing_9"},
    {LCD_PIC_INSTALL10,     "/pic/%d_%d/installing_10"},
    {LCD_PIC_INSTALL11,     "/pic/%d_%d/installing_11"},
    {LED_ATTR_ONOFF,        "/sys/class/leds/test-backlight/trigger"},
    {LED_ATTR_BRIGHT,       "/sys/class/leds/test-backlight/brightness"},
    {LED_ATTR_DELAY_ON,     "/sys/class/leds/test-backlight/delay_on"},
    {LED_ATTR_DELAY_OFF,    "/sys/class/leds/test-backlight/delay_off"},
};

/* LCD ����״̬�б� */
STATIC upstate_info_t upstateinfo_st[] = 
{
    /* LCD��ʾ���� */
    /* ����״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_IDLE,      LCD_READ,   {.path_index = LCD_PIC_INSTALL0},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,      LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    
    /* ͨ������״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL0},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL1},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL2},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL3},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL4},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL5},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL6},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL7},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL8},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL9},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL10},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_INSTALL11},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},

    /* ������������״̬��ʾ���� */
    {DLOAD_STATE_AUTOUG_RUNNING,   LCD_READ,   {.path_index = LCD_PIC_AUTOUG},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},

    /* �͵�״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_LOW_POWER, LCD_READ,   {.path_index = LCD_PIC_LOWPOWER},     SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_LOW_POWER, LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},

    /* �����ɹ�״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_SUCCESS,   LCD_READ,   {.path_index = LCD_PIC_SUCCESS},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},

    /* ����ʧ��״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_FAIL,      LCD_READ,   {.path_index = LCD_PIC_FAILED},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,      LCD_WRITE,  {.path_index = LCD_DEV_NAME},         SHOW_YES_DELAY},

    /* LED��ʾ���� */
    /* ����״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_IDLE,      LED_READ,   {.str_value  = "1"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,      LED_WRITE,  {.path_index = LED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,      LED_READ,   {.str_value  = "3000"},               SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,      LED_WRITE,  {.path_index = LED_ATTR_BRIGHT},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,      LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,      LED_WRITE,  {.path_index = LED_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,      LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_IDLE,      LED_WRITE,  {.path_index = LED_ATTR_DELAY_OFF},   SHOW_NOT_DELAY},

    {DLOAD_STATE_FAIL,      LED_READ,   {.str_value  = "1"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,      LED_WRITE,  {.path_index = LED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,      LED_READ,   {.str_value  = "3000"},               SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,      LED_WRITE,  {.path_index = LED_ATTR_BRIGHT},      SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,      LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,      LED_WRITE,  {.path_index = LED_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,      LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_FAIL,      LED_WRITE,  {.path_index = LED_ATTR_DELAY_OFF},   SHOW_NOT_DELAY},

    /* ͨ������״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "1"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "3000"},               SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_ATTR_BRIGHT},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_RUNNING,   LED_WRITE,  {.path_index = LED_ATTR_DELAY_OFF},   SHOW_NOT_DELAY},

    /* ��������״̬��ʾ���� */
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "1"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "3000"},               SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_ATTR_BRIGHT},      SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_AUTOUG_RUNNING,   LED_WRITE,  {.path_index = LED_ATTR_DELAY_OFF},   SHOW_NOT_DELAY},

    /* �͵�״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_LOW_POWER, LED_READ,   {.str_value  = "1"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_LOW_POWER, LED_WRITE,  {.path_index = LED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_LOW_POWER, LED_READ,   {.str_value  = "3000"},               SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_LOW_POWER, LED_WRITE,  {.path_index = LED_ATTR_BRIGHT},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_LOW_POWER, LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_LOW_POWER, LED_WRITE,  {.path_index = LED_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_LOW_POWER, LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_LOW_POWER, LED_WRITE,  {.path_index = LED_ATTR_DELAY_OFF},   SHOW_NOT_DELAY},

    /* �����ɹ�״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "1"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "3000"},               SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_ATTR_BRIGHT},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_SUCCESS,   LED_WRITE,  {.path_index = LED_ATTR_DELAY_OFF},   SHOW_NOT_DELAY},

    /* ����ʧ��״̬��ʾ���� */
    {DLOAD_STATE_NORMAL_FAIL,      LED_READ,   {.str_value  = "1"},                  SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,      LED_WRITE,  {.path_index = LED_ATTR_ONOFF},       SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,      LED_READ,   {.str_value  = "3000"},               SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,      LED_WRITE,  {.path_index = LED_ATTR_BRIGHT},      SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,      LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,      LED_WRITE,  {.path_index = LED_ATTR_DELAY_ON},    SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,      LED_READ,   {.str_value  = "50"},                 SHOW_NOT_DELAY},
    {DLOAD_STATE_NORMAL_FAIL,      LED_WRITE,  {.path_index = LED_ATTR_DELAY_OFF},   SHOW_NOT_DELAY},

};
#endif

/*******************************************************************
  ��������  : get_product_mark
  ��������  : ��ȡ��ƷӲ����ʶ��
  �������  : NA
  �������  : NA
  ��������ֵ: Ӳ����ʶ
********************************************************************/
static uint32  get_product_mark(void)
{
    uint32 hw_main = bsp_get_version_info()->board_id;
    
    g_product_id = (hw_main >> PRODUCT_OFFSET) & 0xFF;
    g_big_ver = hw_main & 0xFFFF;    
    
    return 0;
    
}

/*******************************************************************
  ��������  : get_product_info
  ��������  : ��ȡ��Ʒ������Ϣ
  �������  : NA
  �������  : NA
  ��������ֵ: ��Ʒ��Ϣ�ṹ
********************************************************************/
product_info_t*  get_product_info(void)
{
    uint32  ret = 0;
    uint32  index = 0;
    uint32  count = 0;
    product_info_t*  local_product_info_st = NULL;

    /* ��Ʒ��ʶ��ȡ */
    ret = get_product_mark();
    if(0 != ret)
    {
        return NULL;
    }
    
    count = sizeof(product_info_st) / sizeof(product_info_t);

    for(index = 0; index < count; index ++)
    {
        local_product_info_st = &product_info_st[index];
        if((g_product_id == (uint8)local_product_info_st->hw_major_id) && 
            (g_big_ver == (uint16)local_product_info_st->hw_minor_id))
        {
            return local_product_info_st;
        }
    }

    DLOAD_PRINT(KERN_ERR, "g_product_id = %u  g_big_ver = %u  product_info is NULL.", g_product_id, g_big_ver);
    
    return NULL;
}

/*******************************************************************
  ��������  : get_dload_feature
  ��������  : ��ȡ��������������Ϣ
  �������  : NA
  �������  : NA
  ��������ֵ: ��Ʒ��Ϣ�ṹ
********************************************************************/
dload_feature_t*  get_dload_feature(void)
{
    uint32  index = 0;
    uint32  count = 0;
    dload_feature_t*  local_dload_feature_st = NULL;
    dload_smem_info_t* smem_info = NULL;
    count = sizeof(dload_feature_st) / sizeof(dload_feature_t);

    smem_info = get_smem_info();

    for(index = 0; index < count; index ++)
    {
        local_dload_feature_st = &dload_feature_st[index];
        if((g_product_id == (uint8)local_dload_feature_st->hw_major_id) && 
            (g_big_ver == (uint16)local_dload_feature_st->hw_minor_id))
        {
#if  (FEATURE_ON == MBB_DLOAD_VER_VERIFY)
            local_dload_feature_st->safety1_3_dload.safe_support = TRUE;
#else
            local_dload_feature_st->safety1_3_dload.safe_support = FALSE;
#endif
            if(SMEM_BURN_UPDATE_FLAG_NUM == *((uint32*)(smem_info->ut_dload_addr)))
            {
                /* Ϊ��Ƭ�汾������Ƭ�汾��ʶ����������ȫУ�� */
                /* ���ݿ���Ƭ�汾�������汾led��ʾ��һ�� */
#if (FEATURE_ON == MBB_DLOAD_STICK || FEATURE_ON == MBB_DLOAD_HILINK || FEATURE_ON == MBB_DLOAD_WINGLE)
                dload_burn = TRUE;
#endif
                if(SMEM_DT_UPDATE_FLAG_NUM == *((uint32*)(smem_info->dt_dload_addr)))
                {
                    local_dload_feature_st->safety_dload = FALSE;
                    local_dload_feature_st->safety1_3_dload.safe_support = FALSE;
                }
            }
            return local_dload_feature_st;
        }
    }
    
    return NULL;
}

/*******************************************************************
  ��������  : get_show_info
  ��������  : ��ȡpath ��path_indexת��������
  �������  : NA
  �������  : pppath_info_st : pathinfo_st ����ĵ�ַ
                            info_size: pathinfo_st��size
  ��������ֵ: DLOAD_OK    : �ɹ�
              DLOAD_ERR   : ʧ��
********************************************************************/
uint32  get_show_path_info(path_info_t **pppath_info_st, uint32 *info_size)
{
    if(NULL != pppath_info_st)
    {
        if(FALSE == dload_burn)
        {
            *pppath_info_st = pathinfo_st;
        }
#if (FEATURE_ON == MBB_DLOAD_STICK || FEATURE_ON == MBB_DLOAD_HILINK || FEATURE_ON == MBB_DLOAD_WINGLE)
        else
        {
            *pppath_info_st = pathinfo_burn_st;
        }
#endif
    }
    if(NULL != info_size)
    {
        if(FALSE == dload_burn)
        {
            *info_size= sizeof(pathinfo_st) / sizeof(path_info_t);
        }
#if (FEATURE_ON == MBB_DLOAD_STICK || FEATURE_ON == MBB_DLOAD_HILINK || FEATURE_ON == MBB_DLOAD_WINGLE)
        else
        {
            *info_size= sizeof(pathinfo_burn_st) / sizeof(path_info_t);
        }
#endif
    }
    return DLOAD_OK;

}

/*******************************************************************
  ��������  : get_show_info
  ��������  : ��ȡlcd/led������Ϣ
  �������  : NA
  �������  : ppupstate_info : upstateinfo_st ����ĵ�ַ
                            data_len: upstate_info_st��size
  ��������ֵ: DLOAD_OK    : �ɹ�
              DLOAD_ERR   : ʧ��
********************************************************************/
uint32  get_show_info(upstate_info_t **ppupstate_info, uint32 *info_size)
{
    if(NULL != ppupstate_info)
    {
        if(FALSE == dload_burn)
        {
            *ppupstate_info = upstateinfo_st;
        }
#if (FEATURE_ON == MBB_DLOAD_STICK || FEATURE_ON == MBB_DLOAD_HILINK || FEATURE_ON == MBB_DLOAD_WINGLE)
        else
        {
            *ppupstate_info = upstateinfo_burn_st;
        }
#endif
    }
    if(NULL != info_size)
    {
        if(FALSE == dload_burn)
        {
            *info_size= sizeof(upstateinfo_st) / sizeof(upstate_info_t);
        }
#if (FEATURE_ON == MBB_DLOAD_STICK || FEATURE_ON == MBB_DLOAD_HILINK || FEATURE_ON == MBB_DLOAD_WINGLE)
        else
        {
            *info_size= sizeof(upstateinfo_burn_st) / sizeof(upstate_info_t);
        }
#endif
    }
    return DLOAD_OK;

}



#ifdef  __cplusplus
    }
#endif
