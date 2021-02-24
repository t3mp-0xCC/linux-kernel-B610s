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



#ifndef __BSP_NVIM_H__
#define __BSP_NVIM_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*lint -save -e537*/
#include "bsp_nvim_mem.h"
#include "mdrv_nvim_comm.h"
#if (defined(__KERNEL__) || defined(__OS_VXWORKS__)||defined(__OS_RTOSCK__) || (defined(__CMSIS_RTOS)))
#include "osl_common.h"
#include "bsp_om.h"
#define BSP_ERR_NV_BASE                         BSP_DEF_ERR(BSP_MODU_NV, 0)
#else
#include "types.h"
#define BSP_ERR_NV_BASE                         0x100f0000
#endif
/*lint -restore +e537*/

#define NV_GLOBAL_CTRL_INFO_SIZE    (sizeof(struct nv_ctrl_file_info_stru))

#define NV_GLOBAL_FILE_LIST_ADDR    (NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE)
#define NV_GLOBAL_FILE_ELEMENT_SIZE (sizeof(struct nv_file_list_info_stru))

#define NV_REF_LIST_ITEM_SIZE       (sizeof(struct nv_ref_data_info_stru))

#define NV_MAX_FILE_SIZE            (SHM_MEM_NV_SIZE - NV_GLOBAL_INFO_SIZE)
/****************************************************************/

/****************************************************************/

#ifndef NV_OK
#define NV_OK           (0)
#endif

#ifndef NV_ERROR
#define NV_ERROR        0xffffffff
#endif

#ifndef FILE
#define FILE    void
#endif

#ifndef NULL
#define NULL    (void*)0
#endif


/*�ⲿ�����ȡ˫��˫������sim������ʹ��һ�º궨��*/
#ifndef MODEM_ID_0
#define MODEM_ID_0          0   /*��1����*/
#endif

#ifndef MODEM_ID_1
#define MODEM_ID_1          1   /*��2����*/
#endif

#ifndef MODEM_ID_2
#define MODEM_ID_2          2   /*��2����*/
#endif

#ifndef MODEM_ID_BUTT
#define MODEM_ID_BUTT       3
#endif


/*�����ȡ˫��˫��nv����ʹ�������ö�ٶ���*/
/*sim card type*/
typedef enum
{
    NV_USIMM_CARD_1 = 1,       /*��1����*/
    NV_USIMM_CARD_2 = 2,       /*��2����*/
    NV_USIMM_CARD_3 = 3,       /*��3����*/
}NV_DOUBLE_CARD_TYPE_ENUM;

/*init state*/
enum
{
    NV_BOOT_INIT_FAIL = 0,
    NV_INIT_FAIL,
    NV_KERNEL_INIT_DOING,
    NV_BOOT_INIT_OK,
    NV_INIT_OK
};


/*func type*/
enum
{
    NV_API_READ = 0,     /*-- 0 --*/
    NV_FUN_READ_EX = 1,
    NV_API_AUTHREAD = 2,
    NV_API_DIRECTREAD = 3,
    NV_API_WRITE = 4,
    NV_FUN_WRITE_EX = 5,
    NV_API_AUTHWRITE = 6,
    NV_API_DIRECTWRITE = 7,
    NV_API_GETLEN = 8,
    NV_API_AUTHGETLEN = 9,
    NV_API_FLUSH = 10,
    NV_API_REVERT = 11,
    NV_API_BACKUP = 12,
    NV_API_REVERT_MANF = 13,
    NV_API_REVERT_USER = 14,
    NV_API_IMPORT = 15,
    NV_API_EXPORT = 16,
    NV_FUN_MEM_INIT = 17,
    NV_FUN_UPGRADE_PROC = 18,
    NV_FUN_KERNEL_INIT = 19,
    NV_FUN_REMAIN_INIT = 20,
    NV_FUN_NVM_INIT = 21,
    NV_FUN_XML_DECODE = 22,
    NV_FUN_SEARCH_NV = 23,
    NV_FUN_REVERT_DATA = 24,
    NV_FUN_UPDATE_DEFAULT = 25,
    NV_FUN_REVERT_DEFAULT = 26,
    NV_FUN_ERR_RECOVER = 27,
    NV_FUN_KEY_CHECK   = 28,
    NV_FUN_FLUSH_SYS   = 29,
#ifdef CONFIG_MULTI_CARRIER
    NV_FUN_MTCARRIER_BACKUP     = 30,
    NV_FUN_MTCARRIER_USR_REVERT = 31,
#endif
    NV_API_FLUSH_LIST = 32,
    NV_CRC32_MAKE_DDR = 33,
    NV_CRC32_DATA_CHECK = 34,
    NV_CRC32_GET_FILE_LEN = 35,
    NV_CRC32_MAKE_DDR_CRC = 36,
    NV_CRC32_DLOAD_FILE_CHECK = 37,
    NV_CRC32_DDR_CRC_CHECK = 38,
    NV_CRC32_DDR_RESUME_IMG = 39,
    NV_FUN_MAX_ID,
};
/*file ops*/
#ifndef  SEEK_SET
#define  SEEK_SET        0
#endif
#ifndef  SEEK_CUR
#define  SEEK_CUR        1
#endif
#ifndef  SEEK_END
#define  SEEK_END        2
#endif


#define NV_FILE_USING                0x766e       /*nv*/

/*�ڴ���������*/
#define  NV_MEM_DLOAD                   0x12345678
#define  NV_MEM_SYSTEM                  0xabcde123
#define  NV_MEM_BACKUP                  0xdce582e1

#define NV_CTRL_FILE_MAGIC_NUM          0x224e4944   /*�ļ�ͷ��־λ*/
#define NV_XML_DEC_SUCC_STATE           0x5687dcfe   /*fastboot �׶�xml�����Ƿ�ɹ���־*/
#define NV_FILE_TAIL_MAGIC_NUM          0xabcd8765   /*�ļ�β��־*/

#define NV_IMEI_LEN              15

#define NV_FILE_READ                 "rb"
#define NV_FILE_RW                   "rb+"
#define NV_FILE_WRITE                "wb+"

#define B_READ                        strcmp(NV_FILE_READ,NV_FILE_READ)

#define nv_check_file_mode(mode)\
    if(strcmp((char *)NV_FILE_READ,(char *)mode)&&strcmp((char *)NV_FILE_RW,(char *)mode)&&strcmp((char *)NV_FILE_WRITE,(char *)mode))\
    {\
        printf("mode = %s is not legal\n", mode);\
        return NV_ERROR;\
    }


#define NV_FILE_MAX_NUM                        20
#define NV_MAX_UNIT_SIZE                       2048

#define NV_FILE_SYS_NV_PATH                     "/system/sys_nv.bin"
#define NV_BACK_PATH                            "/system/nvbak.bin"
#define NV_DLOAD_PATH                           "/system/nvdload.bin"
#define NV_XNV_CARD1_PATH                       "/system/xnv_card1.xml"
#define NV_XNV_CARD1_MAP_PATH                   "/system/xnv_card1.map"
#define NV_XNV_CARD2_PATH                       "/system/xnv_card2.xml"
#define NV_XNV_CARD2_MAP_PATH                   "/system/xnv_card2.map"
#define NV_XNV_CARD3_PATH                       "/system/xnv_card3.xml"
#define NV_XNV_CARD3_MAP_PATH                   "/system/xnv_card3.map"
#define NV_CUST_CARD1_PATH                      "/system/cust_card1.xml"
#define NV_CUST_CARD2_PATH                      "/system/cust_card2.xml"
#define NV_CUST_CARD3_PATH                      "/system/cust_card2.xml"
#define NV_DEFAULT_PATH                         "/system/manufactrue_ver.bin"


#ifdef BSP_CONFIG_PHONE_TYPE
#define NV_IMG_PATH                             "/mnvm2:0/modem_nv/nv.bin"
#define NV_FACTORY_RIGHT_PATH                   "/mnvm2:0/modem_nv/factory.right"
#define NV_IMG_FLAG_PATH                        "/modem_log/modem_nv/img_flag.bin"
#define NV_BACK_FLAG_PATH                       "/modem_log/modem_nv/back_flag.bin"
#define NV_SYS_FLAG_PATH                        "/modem_log/modem_nv/sys_flag.bin"

#define NV_LOG_PATH                             "/modem_log/NvLog.txt"

#define NV_LOG_MAX_SIZE                         0x80000/*512k*/
#else

#if (FEATURE_ON == MBB_COMMON)
#define NV_REBAK_FLAG_PATH                      "/mnvm2:0/nvrebak.bin"  /*nv���ݰ�������ʶ�ļ�*/
#endif

#define NV_IMG_PATH                             "/mnvm2:0/nv.bin"
#define NV_FACTORY_RIGHT_PATH                   "/mnvm2:0/factory.right"
#define NV_IMG_FLAG_PATH                        "/modem_log/img_flag.bin"
#define NV_BACK_FLAG_PATH                       "/modem_log/back_flag.bin"
#define NV_SYS_FLAG_PATH                        "/modem_log/sys_flag.bin"

#define NV_LOG_PATH                             "/modem_log/NvLog.txt"

#ifdef CONFIG_MULTI_CARRIER
/*#define NV_BACK_MTCARRIER_FLAG_PATH             "/mtc/mt_carrier_current_info"*/
#define NV_BACK_MTCARRIER_FLAG_PATH             "/modem_log/mtca_back_flag.bin"
#define NV_IMG_TEST_PATH                        "/modem_log/nv_test.bin"
#endif

#define NV_LOG_MAX_SIZE                         0x4000/*16k*/
#endif



/* error code */

#define BSP_ERR_NV_MEM_INIT_FAIL                (BSP_ERR_NV_BASE + 0x1)
#define BSP_ERR_NV_INVALID_PARAM                (BSP_ERR_NV_BASE + 0x2)
#define BSP_ERR_NV_MALLOC_FAIL                  (BSP_ERR_NV_BASE + 0x3)
#define BSP_ERR_NV_ERASER_FAIL                  (BSP_ERR_NV_BASE + 0x4)
#define BSP_ERR_NV_FREE_BUF_FAIL                (BSP_ERR_NV_BASE + 0x5)
#define BSP_ERR_NV_NO_FILE                      (BSP_ERR_NV_BASE + 0x6)
#define	BSP_ERR_NV_READ_FILE_FAIL               (BSP_ERR_NV_BASE + 0x7)
#define	BSP_ERR_NV_WRITE_FILE_FAIL              (BSP_ERR_NV_BASE + 0x8)
#define	BSP_ERR_NV_FTELL_FILE_FAIL              (BSP_ERR_NV_BASE + 0x9)
#define	BSP_ERR_NV_SEEK_FILE_FAIL               (BSP_ERR_NV_BASE + 0xa)
#define	BSP_ERR_NV_REMOVE_FILE_FAIL             (BSP_ERR_NV_BASE + 0xb)
#define	BSP_ERR_NV_CLOSE_FILE_FAIL              (BSP_ERR_NV_BASE + 0xc)
#define	BSP_ERR_NV_READ_DATA_FAIL               (BSP_ERR_NV_BASE + 0xd)
#define	BSP_ERR_NV_BACKUP_FILE_FAIL             (BSP_ERR_NV_BASE + 0xe)
#define	BSP_ERR_NV_BACKUP_DATA_FAIL             (BSP_ERR_NV_BASE + 0xf)
#define	BSP_ERR_NV_UPDATE_FILE_FAIL             (BSP_ERR_NV_BASE + 0x10)
#define	BSP_ERR_NV_UPDATE_DATA_FAIL             (BSP_ERR_NV_BASE + 0x11)
#define	BSP_ERR_NV_SECTION_NOT_SUPPORT_UPDATE   (BSP_ERR_NV_BASE + 0x12)
#define	BSP_ERR_NV_NO_WRITE_RIGHT               (BSP_ERR_NV_BASE + 0x13)
#define	BSP_ERR_NV_WRITE_DATA_FAIL              (BSP_ERR_NV_BASE + 0x14)
#define	BSP_ERR_NV_FILE_ERROR                   (BSP_ERR_NV_BASE + 0x15)
#define	BSP_ERR_NV_NO_THIS_ID                   (BSP_ERR_NV_BASE + 0x16)
#define	BSP_ERR_NV_ICC_CHAN_ERR                 (BSP_ERR_NV_BASE + 0x17)
#define	BSP_ERR_NV_ITEM_LEN_ERR                 (BSP_ERR_NV_BASE + 0x18)
#define	BSP_ERR_NV_RECOVER_BAK_ERR              (BSP_ERR_NV_BASE + 0x19)
#define	BSP_ERR_NV_AUTH_FAIL                    (BSP_ERR_NV_BASE + 0x20)
#define BSP_ERR_NV_DDR_OPS_ERR                  (BSP_ERR_NV_BASE + 0x21)
#define BSP_ERR_NV_OVERTIME_WAIT                (BSP_ERR_NV_BASE + 0x22)
#define BSP_ERR_NV_NAND_ALL_BAD                 (BSP_ERR_NV_BASE + 0x23)
#define BSP_ERR_NV_OPEN_FILE_FAIL               (BSP_ERR_NV_BASE + 0x24)
#define BSP_ERR_NV_HEAD_MAGIC_ERR               (BSP_ERR_NV_BASE + 0x25)
#define BSP_ERR_NV_DDR_NOT_ENOUGH_ERR           (BSP_ERR_NV_BASE + 0x26)
#define BSP_ERR_NV_CRC_CODE_ERR                 (BSP_ERR_NV_BASE + 0x27)
#define BSP_ERR_NV_GET_NV_LEN_ERR               (BSP_ERR_NV_BASE + 0x28)
#define BSP_ERR_NV_DELOAD_CHECK_ERR             (BSP_ERR_NV_BASE + 0x29)
#define BSP_ERR_NV_MEMCPY_ERR                   (BSP_ERR_NV_BASE + 0x30)
#define BSP_ERR_NV_MEMSET_ERR                   (BSP_ERR_NV_BASE + 0x31)
#define BSP_ERR_NV_OVER_MEM_ERR                 (BSP_ERR_NV_BASE + 0x32)/*��СԽ��*/
#define BSP_ERR_NV_MEM_SIZE_ERR                 (BSP_ERR_NV_BASE + 0x33)/*��С����ȷ*/
#define BSP_ERR_NV_XML_CFG_ERR                  (BSP_ERR_NV_BASE + 0x34)/*NV��xml���ò���ȷ*/
#define BSP_ERR_NV_TIME_OUT_ERR                 (BSP_ERR_NV_BASE + 0x35)/*��ʱ����*/
#define BSP_ERR_NV_GIVE_IPC_ERR                 (BSP_ERR_NV_BASE + 0x36)/*give ipc����*/
#define BSP_ERR_NV_NOT_SUPPORT_ERR              (BSP_ERR_NV_BASE + 0x37)/*��֧�ִ���*/
#define BSP_ERR_NV_REG_DUMP_ERR                 (BSP_ERR_NV_BASE + 0x38)/*ע���ά�ɲ��쳣*/


#define   NV_MID_PRI_LEVEL_NUM   7

/*priority type*/
enum
{
    NV_HIGH_PRIORITY = 0,
    NV_MID_PRIORITY1,
    NV_MID_PRIORITY2,
    NV_MID_PRIORITY3,
    NV_MID_PRIORITY4,
    NV_MID_PRIORITY5,
    NV_MID_PRIORITY6,
    NV_LOW_PRIORITY = 7,
    NV_BUTT_PRIORITY
};


enum
{
    NV_FLAG_NO_CRC = 0,         /*������CRCУ��*/
    NV_FLAG_NEED_CRC = 1,       /*��Ҫ����CRCУ��*/
    NV_MARK_CRC_BUTTON,
};

struct nv_file_info_stru
{
    u32 magic_num;                  /*file valid flag*//*��ʶ�ļ�   0x766e(nv): ��*/
    u32 off;                        /*file offset in flash*/
    u32 len;                        /*length of the data in this section */
};

typedef struct
{
    struct nv_file_info_stru stXnvFile;     /*xnv.xml�ļ���Ϣ*/
    struct nv_file_info_stru stCustFile;    /*cust.xml�ļ���Ϣ*/
    struct nv_file_info_stru stMapFile;     /*xnv.xml map�ļ���Ϣ*/
}STRU_XNV_MAP_FILE_INFO;

/*NV��������ͷ�ṹ*/
struct nv_dload_packet_head_stru
{
    struct nv_file_info_stru nv_bin;        /*nv.bin�ļ���Ϣ*/
    struct nv_file_info_stru xnv_xml[2];    /*˫��xnv.xml�ļ���Ϣ*/
    struct nv_file_info_stru cust_xml[2];   /*˫��cust.xml�ļ���Ϣ*/
    struct nv_file_info_stru xnv_map[2];    /*˫��xnv.xml map�ļ���Ϣ*/
    u32 ulSimNumEx;                         /*NV֧�ֵ���modem������2*/
    STRU_XNV_MAP_FILE_INFO xnv_file[0]; /*��˫������֧�ֵĿ�����Ϣ��ulSimNumEx -2��*/
};
/*lint -save -e46*/
typedef union nv_crc_mark_stru
{
    u8 value;
    struct _crc_bits_
    {
        u8 data_crc:1;  /*data��CRCУ���־*/
        u8 ctrl_crc:1;  /*ctrl��CRCУ���־*/
        u8 reserverd:6;
    }bits;
}crc_mark_stru;
/*lint -restore*/

/* NV Ctrl File Header Struct */
struct nv_ctrl_file_info_stru
{
    u32  magicnum;
    u32  ctrl_size;        /*ctrl file size*/
    u8   version[2];       /*file version*/
    u8   modem_num;        /*modem num*/
    crc_mark_stru   crc_mark;
    u32  file_offset;      /*Offset of the File list start address*/
    u32  file_num;         /* File num at file list*/
    u32  file_size;        /* File list size*/
    u32  ref_offset;       /* Offset of the NV reference data start address*/
    u32  ref_count;        /* NV ID num*/
    u32  ref_size;         /* NV reference data size*/
    u8   reserve2[12];
    u32  timetag[4];       /*time stamp*/
    u32  product_version[8];/*product version*/
};

/* NV File List Struct */
struct nv_file_list_info_stru
{
    u32  file_id;       /* NV File ID */
    u8   file_name[28]; /* NV File Name */
    u32  file_size;     /* NV File size*/
};

/* NV reference Data Struct */
struct nv_ref_data_info_stru
{
    u16  itemid;     /* NV ID */
    u16  nv_len;     /* NV Length */
    u32  nv_off;     /* Offset of the NV ID start address at the Common NV file */
    u16  file_id;    /* File ID */
    u16  priority;   /* NV ID priority*/
    u8   modem_num;  /* pid num */
    u8   reserved[3];   /* reserve*/
};

struct nv_global_file_handle_stru
{
    u32 file_id;
    u32 offset;                 /*compare with the ctrl file start addr*/
    u32 size;
};

/*common file head*/
struct nv_comm_file_head_stru
{
    u32 magic_num;
    u32 lookup_off;
    u32 lookup_num;
    u32 lookup_len;
    u32 nvdata_len;
    u32 reserved1[3];
    u32 time_tag[4];
    u32 reserved2[4];
};

/*common file lookup stru*/
struct nv_comm_file_lookup_stru
{
    u16 itemid;
    u16 nv_len;
    u32 nv_off;
    u16 priority;
    u8  modem_num;
    u8  reserved;
};

struct nv_fastboot_debug_stru
{
    u32 line;
    u32 ret;
    u32 reseverd1;
    u32 reseverd2;
};

struct nv_flush_info_stru
{
    u16  itemid;     /* NV ID */
    u16  nv_len;     /* NV Length */
    u32  nv_off;     /* Offset of the NV ID start address at the ctrl NV file */
};
/*���ڴ洢��Ҫflush��flash�е�nv��id�б�*/
#define NV_FLUSH_LIST_SIZE      (50)
#define NV_FLUSH_LIST_OVER_SIZE (NV_FLUSH_LIST_SIZE - 10)

struct nv_flush_list_stru
{
    u32 count;
    struct nv_flush_info_stru list[0];
};
struct nv_crc_flush_info_stru
{
    u32 count;
    u32 offsetArray[3*NV_FLUSH_LIST_SIZE];/*�洢4K�����ݵ�ƫ�Ƶ�����*/
};
#pragma pack(push)
#pragma pack(4)
/*nv global ddr info, size < 1k*/
struct nv_global_ddr_info_stru
{
    u32 ddr_read;               /*whether ddr can to write*/
    u32 xml_dec_state;               /*xml dec state*/
    u32 acore_init_state;         /*acore init state,after kernel init OK ,then start next step */
    u32 ccore_init_state;         /*ccore init state,only after acore kernel init ok or init all ok*/
    u32 mcore_init_state;
    u32 mem_file_type;
    u32 priority;               /*reg the priority sum*/
    u32 cur_time;
    u32 file_num;                                  /*file list file num*/
    u32 file_len;                                  /*file toatl len,include ctrl file*/
    u32 mem_file_len;                            /*currently file len in ddr*/
    long long p_crc_code;                            /*CRC��λ��ָ�룬�����ַ��ʹ��ʱ��Ҫת��*/
    struct nv_global_file_handle_stru file_info[NV_FILE_MAX_NUM];  /*reg every file size&offset*/
    struct nv_fastboot_debug_stru  fb_debug;
    struct nv_flush_list_stru       flush_info;
#if (FEATURE_ON == MBB_DLOAD)
    u32 nvsys_boot_state;    /*fastboot�׶��Ƿ����nvsysʧ�ܱ�ʶ*/
    u32 nvback_boot_state;    /*fastboot�׶��Ƿ����nvbackʧ�ܱ�ʶ*/
#endif
};
#pragma pack(pop)

/*icc msg type*/
enum
{
    NV_ICC_REQ_SYS  = 64,
    NV_ICC_REQ_INSTANT_FLUSH = 65,   	/*����д������,��Ҫ�ȴ��ظ�*/
    NV_ICC_REQ_CCORE_DELAY_FLUSH = 66,  /*ccore �ӳ�д�����󣬲���Ҫ�ȴ��ظ�*/
    NV_REQ_ACORE_DELAY_FLUSH = 67,    	/*acore �ӳ�д�����󣬲���Ҫ�ȴ��ظ�*/
    NV_ICC_CNF      = 127,
    NV_ICC_RESUME   = 0xFF  			/*����ָ�DDR�е�nv����*/

};

struct nv_icc_stru
{
    u32 msg_type;          /*msg type*/
    u32 data_off;          /*data off */
    u32 data_len;          /*data len*/
    u32 ret;
    u32 itemid;
    u32 slice;
};

/*CRC32 start*/
#define NV_CRC32_FAIL_LOG_NUM   (50)
#define NV_CRC32_CHECK_SIZE     (4*1024)    /*CRC32 ����У�����С��λ*/


/*����Ϊlen�����ݵ�CRCУ���������*/
#define NV_CRC_CODE_COUNT(len)    ((len)/(NV_CRC32_CHECK_SIZE) + ((len)%(NV_CRC32_CHECK_SIZE)==0?0:1))
/*DDR��CRCУ�����λ��ָ��*/
#define NV_DDR_CRC_CODE_OFFSET    ((u32 *)(unsigned long)SHD_DDR_P2V((void *)(unsigned long)(((struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR)->p_crc_code)))
/*�Ƿ����CRCУ���־*/
#define NV_DATA_CRC_CHECK_YES      (((struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR)->crc_mark.bits.data_crc)
#define NV_CTRL_CRC_CHECK_YES      (((struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR)->crc_mark.bits.ctrl_crc)

/*DDR��NV�ļ��ĳ���*/
#define NV_FILE_LEN               (((struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR)->file_len)
#define NV_FILE_CTRL_SIZE         (((struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR)->ctrl_size)
#define NV_FILE_CTRL_CRC_MARK     (((struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR)->crc_mark.bits.ctrl_crc)
#define NV_FILE_CRC_MARK          ((((struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR))->crc_mark.value)
struct nv_crc32_fail_stat_stru
{
    u32 function;   /*ʧ�ܵĺ���*/
    u32 failReason; /*ʧ�ܵ�ԭ�򣬴�����*/
    u32 slice;      /*ʧ�ܵ�ʱ��*/
};
struct nv_crc32_debug_stru
{
    u32 count;   /*����*/
    struct nv_crc32_fail_stat_stru failLog[NV_CRC32_FAIL_LOG_NUM];

};
/*CRC32 end*/

#if (FEATURE_ON == MBB_FEATURE_CAT_MODULE_SET)
/*****************************************************************************
 �ṹ��    : NV_PLATFORM_CATEGORY_SET_INFO
 �ṹ˵��  : ���õ���CAT�ȼ�
 �ṹ ID   : 50458
*****************************************************************************/
struct nv_platform_catgory_set_info
{
    u32   nv_status;
    u32   cat_value;
    u32   reserve1;
    u32   reserve2;
    u32   reserve3;
};
#endif

u32 nv_readEx(u32 modem_id, u32 itemid, u32 offset, u8 * pdata, u32 datalen);
u32 nv_writeEx(u32 modem_id, u32 itemid, u32 offset, u8 * pdata, u32 datalen);

u32 bsp_nvm_dcread(u32 modem_id, u32 itemid, u8 * pdata, u32 datalen);
u32 bsp_nvm_dcwrite(u32 modem_id, u32 itemid, u8 * pdata, u32 datalen);

u32 bsp_nvm_dcreadpart(u32 modem_id, u32 itemid, u32 offset, u8 * pdata, u32 datalen);
u32 bsp_nvm_dcwritepart(u32 modem_id, u32 itemid, u32 offset, u8 * pdata, u32 datalen);

u32 bsp_nvm_dcread_direct(u32 modem_id, u32 itemid, u8 * pdata, u32 datalen);
u32 bsp_nvm_dcwrite_direct(u32 modem_id, u32 itemid, u8 * pdata, u32 datalen);

u32 bsp_nvm_auth_dcread(u32 modem_id, u32 itemid, u8 * pdata, u32 datalen);
u32 bsp_nvm_auth_dcwrite(u32 modem_id, u32 itemid, u8 * pdata, u32 datalen);


#if defined(INSTANCE_1)

#define bsp_nvm_read(itemid,pdata,datalen)             bsp_nvm_dcread(NV_USIMM_CARD_2,itemid,pdata,datalen)
#define bsp_nvm_write(itemid,pdata,datalen)            bsp_nvm_dcwrite(NV_USIMM_CARD_2,itemid,pdata,datalen)

#define bsp_nvm_readpart(itemid,offset,pdata,datalen)  bsp_nvm_dcreadpart(NV_USIMM_CARD_2,itemid,offset,pdata,datalen)
#define bsp_nvm_writepart(itemid,offset,pdata,datalen) bsp_nvm_dcwritepart(NV_USIMM_CARD_2,itemid,offset,pdata,datalen)

#define bsp_nvm_read_direct(itemid,pdata,datalen)      bsp_nvm_dcread_direct(NV_USIMM_CARD_2,itemid,pdata,datalen)
#define bsp_nvm_write_direct(itemid,pdata,datalen)     bsp_nvm_dcwrite_direct(NV_USIMM_CARD_2,itemid,pdata,datalen)

#define bsp_nvm_authread(itemid,pdata,datalen)         bsp_nvm_auth_dcread(NV_USIMM_CARD_2,itemid,pdata,datalen)
#define bsp_nvm_authwrite(itemid,pdata,datalen)        bsp_nvm_auth_dcwrite(NV_USIMM_CARD_2,itemid,pdata,datalen)

#elif defined(INSTANCE_2)

#define bsp_nvm_read(itemid,pdata,datalen)             bsp_nvm_dcread(NV_USIMM_CARD_3,itemid,pdata,datalen)
#define bsp_nvm_write(itemid,pdata,datalen)            bsp_nvm_dcwrite(NV_USIMM_CARD_3,itemid,pdata,datalen)

#define bsp_nvm_readpart(itemid,offset,pdata,datalen)  bsp_nvm_dcreadpart(NV_USIMM_CARD_3,itemid,offset,pdata,datalen)
#define bsp_nvm_writepart(itemid,offset,pdata,datalen) bsp_nvm_dcwritepart(NV_USIMM_CARD_3,itemid,offset,pdata,datalen)

#define bsp_nvm_read_direct(itemid,pdata,datalen)      bsp_nvm_dcread_direct(NV_USIMM_CARD_3,itemid,pdata,datalen)
#define bsp_nvm_write_direct(itemid,pdata,datalen)     bsp_nvm_dcwrite_direct(NV_USIMM_CARD_3,itemid,pdata,datalen)

#define bsp_nvm_authread(itemid,pdata,datalen)         bsp_nvm_auth_dcread(NV_USIMM_CARD_3,itemid,pdata,datalen)
#define bsp_nvm_authwrite(itemid,pdata,datalen)        bsp_nvm_auth_dcwrite(NV_USIMM_CARD_3,itemid,pdata,datalen)

#else

#define bsp_nvm_read(itemid,pdata,datalen)             bsp_nvm_dcread(NV_USIMM_CARD_1,itemid,pdata,datalen)
#define bsp_nvm_write(itemid,pdata,datalen)            bsp_nvm_dcwrite(NV_USIMM_CARD_1,itemid,pdata,datalen)

#define bsp_nvm_readpart(itemid,offset,pdata,datalen)  bsp_nvm_dcreadpart(NV_USIMM_CARD_1,itemid,offset,pdata,datalen)
#define bsp_nvm_writepart(itemid,offset,pdata,datalen) bsp_nvm_dcwritepart(NV_USIMM_CARD_1,itemid,offset,pdata,datalen)

#define bsp_nvm_read_direct(itemid,pdata,datalen)      bsp_nvm_dcread_direct(NV_USIMM_CARD_1,itemid,pdata,datalen)
#define bsp_nvm_write_direct(itemid,pdata,datalen)     bsp_nvm_dcwrite_direct(NV_USIMM_CARD_1,itemid,pdata,datalen)

#define bsp_nvm_authread(itemid,pdata,datalen)         bsp_nvm_auth_dcread(NV_USIMM_CARD_1,itemid,pdata,datalen)
#define bsp_nvm_authwrite(itemid,pdata,datalen)        bsp_nvm_auth_dcwrite(NV_USIMM_CARD_1,itemid,pdata,datalen)


#endif

#if (FEATURE_ON == MBB_DLOAD)
/*�ж��Ƿ�Ϊ��Ҫnv*/
bool nv_isSecListNv(u16 itemid);
#endif 

#if(FEATURE_ON == MBB_FEATURE_NVAUTOBACKUP)
bool nv_isAutoBakupListNv(u16 itemid);
#endif

u32 bsp_nvm_backup(u32 crc_flag);


#ifdef CONFIG_MULTI_CARRIER
u32 bsp_nvm_mtcarrier_backup(char* back_path);

u32 bsp_nvm_revert_mtcarrier_user(void);

u32 bsp_nvm_mtcarrier_cust_xml_decode(void);
#endif

#if (FEATURE_ON == MBB_CARRIER_ADAPTER)
u32 carrier_name_match_cust_nv_path(int carrier_name, char *cust_nv_path);
u32 bsp_nvm_ca_xml_decode(char *cust_path);
#endif
/*����������ˢ���ļ�ϵͳ��*/
u32 bsp_nvm_flush(void);

/*����ʽ����������ˢ���ļ�ϵͳ�У�����*/
u32 bsp_nvm_flushEn(u32 crc_flag);

/*���ڴ���ƫ��Ϊoff������Ϊlen������ˢ���ļ�ϵͳ��*/
u32 bsp_nvm_flushEx(u32 off,u32 len,u32 itemid);

/*���ڴ��е�����ˢ��sys nv��*/
u32 bsp_nvm_flushSys(u32 itemid);

u32 bsp_nvm_get_len(u32 itemid, u32 * length);
u32 bsp_nvm_authgetlen(u32 itemid, u32 * len);

u32 bsp_nvm_revert_default(void);

u32 bsp_nvm_update_default(void);

u32 bsp_nvm_revert(void);

u32 bsp_nvm_revert_user(void);

u32 bsp_nvm_revert_manufacture(void);

u32 bsp_nvm_xml_decode(void);

u32 nvm_read_rand(u32 nvid);
u32 nvm_read_randex(u32 nvid, u32 modem_id);

u32 bsp_nvm_get_nv_num(void);
u32 bsp_nvm_get_nvidlist(NV_LIST_INFO_STRU*  nvlist);

/*ccore/mcore init*/
int bsp_nvm_init(void);

s32 bsp_nvm_kernel_init(void);
s32 bsp_nvm_remain_init(void);

/*char device */
s32 nvchar_init(void);

/*fastboot init*/
u32 nv_init(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /*__BSP_NVIM_H__*/





