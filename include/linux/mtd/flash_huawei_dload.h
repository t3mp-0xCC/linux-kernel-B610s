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
 
/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
#ifndef FLASH_DLOAD_H
#define FLASH_DLOAD_H

 /*because of nand_core.h need to define MAX_PAGE_SIZE
  So we need to define before all of include file*/
#include "mbb_config.h"
#if(FEATURE_ON == MBB_DLOAD)
#include <linux/mtd/mtd.h>
//#include "nandc_balong.h"
/*===========================================================================

                           MACRO DEFINITIONS

===========================================================================*/
#define MAX_SOFTWARE_VERSION_LENGTH       32
#define MAGIC_DEFAULT                         0x0
#define SHARE_RGN_MAGIC                       0x41454F4C
#define USER_INFO_FLAG_MAGIC_NUM              0x55534552   /* "USER" */

#ifndef FLASH_OEM_REGION_IN_USE_FLAG
#define FLASH_OEM_REGION_IN_USE_FLAG          0x59455320  
#endif
#ifndef ISO_INFO_DATA_MAGIC
#define ISO_INFO_DATA_MAGIC                   0x204F5349
#endif
#ifndef NAND_FILL_CHAR_APP
#define NAND_FILL_CHAR_APP                    0xFF
#endif

#ifndef MSG_ERROR
#define MSG_ERROR(x,a,b,c)
#endif

#define FLASH_NUM_PART_ENTRIE                 16         

#define INVALID_PAGE_ID                       0xFFFFFFFFU
#define INVALID_BLOCK_ID                      0xFFFFFFFFU
#define INVALID_CLUSTER_ID                    0xFFFFFFFFU
#define HUAWEI_NO_CDROM_MAGIC_NUM             0x47569842


#define  FS_HANDLE_ERR                       (-1)
#define  EFS_LOG_SAVE_SIZE                   80
#define  MAX_EFS_LOG_SAVE_SIZE               100
#define  SIG_UIM_TASK_STOP                   0x00004000
#define  OFFLINE_SLEEP_TIME                  3000
#define  MAX_PAGE_SIZE                       4096

/*OEM����ʹ��ħ����*/
#define DLOAD_FLASH_OEM_REGION_USE_FLAG                  0x59455320
#define DLOAD_ISO_VERSION_MAGIC                          0x204F5349
#define DLOAD_WEB_VERSION_MAGIC                          0x20534F49


typedef int int32;
typedef short int16;
typedef char int8;
typedef unsigned long long uint64;
typedef long long int64;
typedef unsigned long uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef unsigned char boolean;

/*===========================================================================

                      PUBLIC DATA DECLARATIONS

===========================================================================*/
typedef struct
{
  uint8  parent_name[FLASH_NUM_PART_ENTRIE];       /* �������� */
  uint32 magic_dload;                                /* ����ħ�� */
  uint32 magic_restore;
  uint32 offset;                                   /* �ӷ����븸�����׵�ַ��ƫ���� */ 
  uint32 length;                                   /* �ӷ������� */
  void   *content_ptr;
  uint16 content_size;
} child_region_t;

/* �ӷ����ṹ�� */

typedef struct 
{
  uint32 in_use_flag;
  uint32 nv_autorestore_magic;
  uint8  nv_autorestore_flag;
} nv_autorestore_type;

  
typedef enum
{
  AUTO_RESTORE_NONE = 0xFF,  /*��ֵ*/
  AUTO_RESTORE_ALLOW = 0,
  AUTO_RESTORE_FORBID,
  AUTO_RESTORE_FAIL,
  /* Newly introduced flag value to additionally indicate failure because of inexistent backup */
  AUTO_RESTORE_FORBID_NO_BACKUP,   
} nv_auto_restore_enum_t;

typedef enum                            
{
  SHARE_REGION = 0,
  REGION_MAX
} oper_region_type;


/* ���������ṹ�� */
typedef struct
{

  uint32 start_addr;
  uint32 block_size;
  uint32 page_size;
  uint8        *buffer;
} oper_region_struct_t;

/* ���ṹ��ISO��Ϣ��Ĵ洢ͷ��������blck�Ƿ�ʹ���Լ���־��blck��ħ�� */
/* ������˶�Ӧ����������ṹ��ĳ��ȵĺ꣬��Ϊsizeof(iso_info_flag_type)�Ľ��Ϊ12 */
#define ISO_INFO_FLAG_LEN  9
typedef struct 
{
  uint32 in_use_flag;
  uint32 iso_info_magic;
  uint8  iso_info_flag;
} iso_info_flag_type;

/* WEBUI �汾���п��ܺܳ� */
#define VDF_WEBUI_PREFIX           "THIRD_PARTY_VDF_"
#define HUAWEI_WEBUI_PREFIX        "WEBUI_"
#define WEBUI_VER_FLAG_MAGIC       (0x5756464D)
#define WEBUI_VER_BUF_LEN          (128)
typedef   struct
{  
    uint32  magic_number;  
    uint8   version_buffer[WEBUI_VER_BUF_LEN];
} webver_buffer_s;

typedef struct rgn_hd_struct
{
    uint32 magic;
    uint32   sub_rgn_num;
}rgn_hd_type;

typedef enum
{
    RGN_HD_INFO_FLAG = 0,
    RGN_RESTORE_FLAG,
    RGN_NVMBN_FLAG,
    RGN_ISOHD_FLAG,
    RGN_SWVER_FLAG,
    RGN_WEBHD_FLAG,
    RGN_PART_COMPLETE_FLAG,
    RGN_DLOAD_RECOVERY_FLAG,
    RGN_EFS_WHOLE_BACKUP_RESTORE_FLAG,
    RGN_TOUCHSCREEN_INFO,
    RGN_PRL_NAM0_FLAG,
    RGN_PRL_NAM1_FLAG,
    RGN_RECORD_UPINFO_FLAG,
    RGN_MTCARRIER_PLMN,
    RESERVE2_FLAG,
    RESERVE3_FLAG,
    RESERVE4_FLAG,
    RGN_CARRIER_ADAPTER_NV_FLAG = 17,       /* �濨ƥ��洢NV���ݶ�Ӧ��PLMN��Ϣ */
    RGN_BADBLOCK_INFO,
    RGN_MAX_NUM
}rgn_pos_e_type;

#define NV_SWVER_FLAG_MAGIC_NUM  0x9F8E7D6C



typedef   struct
{  
   uint32  magic_number;  
   uint8   version_page_buffer[MAX_SOFTWARE_VERSION_LENGTH];  
} version_buffer_page;

extern child_region_t  child_region[];
extern oper_region_struct_t   operation_region[];


#define PART_COMPLETE_FLAG_MAGIC_NUM 0x50434D4E   /*PCMN*/
#define AUTO_UPDATE_FLAG_MAGIC_NUM 0x4155464D  /*AUFM*/
typedef   struct
{  
   uint32   magic_number;  
   uint32   complete_flag;      /*0:����ģʽ��1: ����ģʽ*/
   uint32   dload_times;       /*��¼��������*/
   uint32   auto_flag;  /*����������������Զ�������־,����������������ǰ����,����������complete_flag���ú����*/
} part_complete_flag;

#define DLOAD_RECOVERY_FLAG_MAGIC_NUM 0x44524543   /*DREC*/
typedef   struct
{  
   uint32   magic_number;  
   uint32   new_recovery_flag;      /*0: RECOVERYA   1: RECOVERYB*/
} recovery_type_flag;


#define RECORD_MAX_TIMES  10   
#define DLOAD_RECORD_UPDATE_MAGIC       0x4452554D
#define VERSION_LENGTH     32
#define VER_TIME_LEN       32

typedef enum update_status
{
    UPDATE_BEGIN_STATUS = 1,
    UPDATE_ING_STATUS,
    UPDATE_END_STATUS
}update_progress_e;

typedef struct updswver_info
{
    uint8   bswver[VERSION_LENGTH];         //��������汾��
    uint8   aswver[VERSION_LENGTH];         //Ŀ������汾��
    uint8   uptime[VERSION_LENGTH];         //����ʱ��
    uint32  uptype;                         //������������
    uint32  upstatus;                       //����״̬
}updswver_info_st;

typedef struct rec_updinfo
{
    uint32 magic_num;               //record magic num
    uint32 total_times;             //record total update num
    uint32 upinfo_record_times;     //record record times,  value from 1 - 10
    updswver_info_st upswver_info[RECORD_MAX_TIMES];
}rec_updinfo_st;

typedef struct _rec_tool_upinfo_
{
    uint8 time_info[VER_TIME_LEN];  
    uint8 ver_info[VERSION_LENGTH];
    uint32 tooltype;   
}update_info_st;

#if (FEATURE_ON == MBB_DLOAD_RECORD_BAD)
/* how many partitions have bad blocks */
#define MAX_BAD_PART_NUM                      (64)
/* within a page */
#define MAX_BAD_BLOCK_NUM                     (512)
/* ascii of BBIM */
#define BAD_BLOCK_INFO_MAGIC                  (0x4242494D)
typedef   struct
{  
    uint32  magic_number;  
    uint32  sum;
    uint16  part_sum[MAX_BAD_PART_NUM];
    uint16  block_no[MAX_BAD_BLOCK_NUM];
} bad_block_info_s;
#endif

/*===========================================================================

                      PUBLIC FUNCTION DECLARATIONS

===========================================================================*/

/************************************************************************
FUNCTION  FLASH_NAND_OPER_REGION_INIT
 
DESCRIPTION
    Initilize user partition info,and get the handle of nand flash
RETURN VALUE
    ִ��OK����0,���򷵻ط�0
SIDE EFFECTS
  None
*************************************************************************/
int flash_nand_oper_region_init(oper_region_type  oper_region_idx,
                            struct mtd_info **p_mtd, oper_region_struct_t *p_opregion);



/*===========================================================================
FUNCTION FLASH_NAND_OPER_REGION_CLOSE
 
DESCRIPTION
  close flash device and release flash handle
 
DEPENDENCIES
  None.
 
RETURN VALUE
  TRUE:  valid
  FALSE: invalid
 
SIDE EFFECTS
  None.
 
===========================================================================*/
void flash_nand_oper_region_close(struct mtd_info *p_mtd, oper_region_struct_t *p_opregion);

/*===========================================================================
 
FUNCTION  FLASH_UPDATE_SHARE_REGION_INFO
 
DESCRIPTION
    update user flag,nv_mbn,and iso header info in the share region.
    This function use page to store these info, 

    Page 0: Share region Info:
           Magic:
           Sub region len:
    Page 1: Nv restore flag:
           In Used Flag Magic: 4 Byte
           Auto Restore Magic: 4Byte
           Auto Restore Flag: 
    Page 2:
          In User Flag Magic:
          MBN Magic:
          MBN Info Num:
          MBN: Version:
          MBN nv data
    Page 3:
          ISO Header Info;
          
    We use two block to implement this feature, when the first time to update this feature directly write it to the 
first available block,and then we update this info then firstly read the page info to cache, update the cache value
and then write it to another block, erase lasted block.

Author: ChenFeng 2010-3-20

RETURN VALUE
    TRUE if Op Succeed
    FALSE if Op Failure
 
SIDE EFFECTS
  None
 
===========================================================================*/
boolean flash_update_share_region_info(rgn_pos_e_type region_type, void* data, size_t data_len);
/*===========================================================================
 
FUNCTION  flash_get_share_region_info
 
DESCRIPTION
    Get Correspoding field info
Author: ChenFeng 2010-3-20

RETURN VALUE
    TRUE if Op Succeed
    FALSE if Op Failure
 
SIDE EFFECTS
  None
 
===========================================================================*/
boolean flash_get_share_region_info(rgn_pos_e_type region_type,void* data,
                                    uint32 data_len);


int huawei_dload_set_swver_to_oeminfo(void);

#if (FEATURE_ON == MBB_DLOAD_RECORD_BAD)

int flash_get_bad_block(char *buff, int len);
#endif

#endif
#endif/*FLASH_DLOAD_H*/
