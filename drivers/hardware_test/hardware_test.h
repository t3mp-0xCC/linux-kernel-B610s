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
#ifndef __HARDWARE_TEST_H__
#define __HARDWARE_TEST_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************
  ͷ�ļ�����                            
**************************************************************************/
#include "bsp_sram.h"
/**************************************************************************
  �궨�� 
**************************************************************************/

#ifndef TRUE
#define TRUE                  1
#endif
#ifndef FALSE
#define FALSE                 0
#endif

/**************************************************************************
  �������ṹ���� 
**************************************************************************/
typedef enum
{
    HARDWARE_TEST_MODE_SET = 3,     /*����ģʽ*/
    HARDWARE_TEST_MODE_GET,         /*��ȡģʽ*/
    HARDWARE_TEST_BITFLIP_GET,      /*��ȡbitflip info*/
    HARDWARE_TEST_BITFLIP_CLR,      /*���bitflip info*/
    HARDWARE_TEST_SD_DET_GET,       /*��ȡsd detect ����״̬*/
    HARDWARE_TEST_SIM_DET_GET,      /*��ȡsim detect ����״̬*/
    HARDWARE_TEST_CHG_STATUS_GET,   /*��ȡ���оƬͨ��״̬*/
    HARDWARE_TEST_NAND_INFO_GET,    /*��ȡflash codewore��spare��ecc_correct��С*/    
    HARDWARE_TEST_BOOT_DDR_TEST_INFO_GET,
    HARDWARE_TEST_BITFLIP_TEST_INFO_SET,   
    HARDWARE_TEST_MODE_MAX
}HARDWARE_TEST_CMD;

typedef enum
{
    HARDWARE_TEST_OPT_MEM = 0,
    HARDWARE_TEST_OPT_FLASH,
    HARDWARE_TEST_OPT_PERI,
    HARDWARE_TEST_OPT_ALL,
    FLASH_BITFLIP_OPT = 4,
    DDRTEST_OPT = 5,
    HARDWARE_TEST_OPT_MAX,
}HARDWARE_TEST_OPT;

typedef  struct
{
    unsigned int codewore_size;
    unsigned int spare_size;
    unsigned int ecc_correct_num;
}hwtest_flash_info;
/**************************************************************************
  �������� 
**************************************************************************/

/********************************************************************
*Function    : set_hardware_test_mode
*Description : ����Ӳ���Լ�ģʽ���
*Input       : 
*Output      : none
*Return      : 0 :successed
               other:failed
********************************************************************/
extern int set_hardware_test_mode(hwtest_smem_info *hwtest_info);

/********************************************************************
*Function    : get_hardware_test_mode
*Description : ��ȡӲ���Լ�ģʽ���
*Input       : 
*Output      : none
*Return      : 0 :not
               1:in hardware test mode
********************************************************************/
extern int get_hardware_test_mode(hwtest_smem_info *hwtest_info);

/********************************************************************
*Function    : bit_flip_info_record
*Description : ��¼bit flip��Ϣ
*Input       : cw_id��corrected
*Output      : none
*Return      : 0 :successed
               other:failed
********************************************************************/
extern int bit_flip_info_record(unsigned int cw_id,unsigned int corrected);

extern void init_hardware_flash_info(unsigned int codewore_size, unsigned int spare_size, unsigned int ecc_correct_num);
#ifdef __cplusplus
}
#endif

#endif

