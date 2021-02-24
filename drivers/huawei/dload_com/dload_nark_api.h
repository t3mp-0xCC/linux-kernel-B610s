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


#ifndef _DLOAD_NARK_API_H
#define _DLOAD_NARK_API_H
/******************************************************************************
  1 ����ͷ�ļ�����
******************************************************************************/
#include "dload_comm.h"

#ifdef  __cplusplus
    extern "C"{
#endif

/******************************************************************************
  2 ģ����� (�ꡢö�١��ṹ�塢�Զ�����������) ������:
******************************************************************************/
#define MAP_INDEX(X, Y)                ((0XFF00 & (X << 8)) | (0X00FF & Y)) 
typedef int32  (*msg_func_t)(uint8 *, uint32);

/* ����ͨ�Ÿ���ģ��ID���� */
typedef enum _DLOAD_MODULE
{
    DLOAD_MAIN_MODULE,    /* ������ģ�� */
    DLOAD_SHOW_MODULE,    /* ������ʾģ�� */
    DLOAD_USB_MODULE,     /* һ������ģ�� */
    DLOAD_ONLINE_MODULE,  /* ��������ģ�� */
    DLOAD_SCARD_MODULE,   /* SCARD����ģ�� */
    DLOAD_HOTA_MODULE,    /* HOTA����ģ�� */
    DLOAD_FOTA_MODULE,    /* FOTA����ģ�� */
    DLOAD_FLASH_MODULE,   /* FLASH����ģ�� */
    DLOAD_AT_MODULE,      /* ����ATģ�� */
    DLOAD_SEC_MODULE,     /* ��ȫ����ģ�� */
}dload_module_t;

/* ������ϢID�����б� */
enum _MAIN_MODULE_MSG_ID
{
    MTD_FLUSH_MSG_ID = 0,           /* ˢ��MTD�豸 */
    GET_BAT_LEVEL_MSG_ID = 1,       /* ��ȡ��ص������� */
    GET_PLATFORMINFO_MSG_ID = 2,    /* ��ȡƽ̨��Ϣ */
    VISIT_DLOAD_SMEM_MSG_ID = 3,    /* �����ڴ������ϢID*/
    GET_MTD_FS_TYPE_MSG_ID = 4,     /* ��ȡMTD�ļ�ϵͳ���� */
    GET_DLOAD_FEATURE_MSG_ID = 5,   /* ��ȡ����������Ϣ */
    GET_PATH_INFO_SIZE_MSG_ID = 6,  /* ��ȡ��ʾģ��·��ӳ����С */
    GET_PATH_INFO_MSG_ID = 7,       /* ��ȡ��ʾģ��·��ӳ��� */
    GET_SHOW_INFO_SIZE_MSG_ID = 8,  /* ��ȡ��ʾģ�������б��С */
    GET_SHOW_INFO_MSG_ID = 9,       /* ��ȡ��ʾģ�������б� */
    SET_DOUBLE_PART_FLAG = 10,      /*����˫�������Ա�־*/
    GET_IMG_PUB_KEY_ID = 11,        /* ��ȡ����ǩ����Ӧ�Ĺ�Կ */
};


/* ���������ڴ�ṹ�� */
typedef struct _DLOAD_SMEM_INFO
{
    void*  dload_mode_addr;           /* ����Ƿ��������ģʽ */
    void*  sd_upgrade_addr;           /* ��¼SD��������ʶλ */
    void*  usbport_switch_addr;      /* ��ʶNV�Զ��ָ��׶Σ�����ʱUSB�˿���̬����*/
    void*  app_inactivate_addr;      /* ��¼ ����������SD������ NV�Զ��ָ��׶Σ�APP������ */
    void*  force_mode_addr;           /* ����Ƿ��������ģʽ��ɿ����Զ��ָ�ģʽ */
    void*  ut_dload_addr;             /* ��̨����������ʶ*/        
    void*  multiupg_dload_addr;      /* �鲥������ʶ*/                
    void*  dload_times_addr;           /*����������ʶ*/
    void*  datalock_status_addr;       /* datalock ������ʶ */
    void*  dt_dload_addr;              /*dt������ʶ*/
    void*  smem_reserve4;
    void*  smem_reserve3;
    void*  smem_reserve2;
    void*  smem_reserve1;
}dload_smem_info_t;
/*******************************************************************
  ��������  : search_map_callback
  ��������  : ����ӳ���������һص�����
  �������  : msg_map_index : ӳ������
  �������  : NV
  ��������ֵ: msg_func_t    : �ص�����
********************************************************************/
msg_func_t search_map_callback(uint32  msg_map_index);

/*******************************************************************
  ��������  : nark_api_init
  ��������  : APIģ���ʼ��
  �������  : NA
  �������  : NA
  ��������ֵ: DLOAD_OK    : �ɹ�
              DLOAD_ERR   : ʧ��
********************************************************************/
int32 nark_api_init(void);

/*******************************************************************
  ��������  : visit_dload_smem
  ��������  : �����ڴ���ʽӿ�
  �������  : 
  �������  : NA
  ��������ֵ: TRUE  : �ɹ�
              FALSE : ʧ��
********************************************************************/
int32 visit_dload_smem(uint8 * data_buffer, uint32 data_len);

/*******************************************************************
  ��������  : get_smem_info
  ��������  : �����ṩ��ȡ�����ڴ�ӿ�
  �������  : NA
  �������  : NA
  ��������ֵ: �����ڴ�ָ��
********************************************************************/
dload_smem_info_t* get_smem_info(void);

/*******************************************************************
  ��������  : get_platform_information
  ��������  : ��ȡƽ̨��Ϣ
  �������  : data_buffer : ���ݻ���ָ��
              data_len    : ���ݳ���
  �������  : NA
  ��������ֵ: DLOAD_OK    : �ɹ�
              DLOAD_ERR   : ʧ��
********************************************************************/
int32 get_platform_information(uint8 * data_buffer, uint32 data_len);
#ifdef __cplusplus
}
#endif
#endif /* _DLOAD_NARK_API_H */
