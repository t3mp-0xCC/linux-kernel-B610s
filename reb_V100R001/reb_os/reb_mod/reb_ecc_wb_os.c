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
 * ����ͷ�ļ�()                                   *
 *----------------------------------------------*/
#include "../reb_config.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/export.h>
#include <linux/string.h>
#include "../os_adapter/reb_os_adapter.h"
#include <mtd/mtd-abi.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>
#include <linux/syscalls.h>
#include "reb_func.h"
#if ( YES == Reb_Platform_V7R2 )
#include "ptable_com.h"
#endif
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define MTD_NAME_PREFIX "/dev/mtd/mtd%d"

/*ħ��,�����жϻ�д��Ϣ�Ƿ���Ч*/
#define OUR_MAGIC_NUM (0x19830609)

/*���֧��block��Χ��(������)*/
#define REB_ECC_SCAN_MAX_NUM            (16)                                         

/*���������ַ�����*/
#define REB_ECC_SCAN_MAX_LEN            (32)                                         

/*�������Դ�MTD�豸�Ĵ���*/
#define MAX_TRY_TIME   (5)

/*һ��Page���֧��8��section,��Ӧ8��ECC*/
#if ( YES == Reb_Platform_V7R2 )
#define REB_MAX_SECT_NUM       (4)
#else
#define REB_MAX_SECT_NUM       (4)
#endif

#define MAX_MTD_NAME_LEN       (32)

/*��Σ�����ݱ�����*/
#define PART_WB_DATA "wbdata"
/*��Σ�����ݱ�����ռ�õ�block��*/
#define WB_DATA_BLK_NUM (5)

/*��д��Ϣ������*/
#define PART_WB_INFO "WbInfo"
/*��Σ���д��Ϣ������ռ�õ�block��*/
#define WB_INFO_BLK_NUM (3)

#define MS_OF_ONE_SECOND (1000)
/*��Ʒflsh������ECC��������*/
#define FLASH_ECC_BITS (0)

/*ECC��������ͨ������Ӧ���,Ĭ����0bit*/
unsigned int flash_ecc_bits = FLASH_ECC_BITS;

/*Ҫɨ��ķ�����������, ��ֲʱ����д*/
#if ( YES == Reb_Platform_V7R2 )
#define PART_NAME0 "kernel"
#define PART_NAME1 "kernelbk"
#define PART_NAME2 "m3boot" 
#define PART_NAME3 "m3image"
#define PART_NAME4 "dsp"
#define PART_NAME5 "fastboot"
#define PART_NAME6 "vxworks"
#define PART_NAME7 "nvbacklte"
#define PART_NAME8 "nvdefault"
#define PART_NAME9 "nvdload"
#define PART_NAMEA "oeminfo"
#define PART_NAMEB NULL
#define PART_NAMEC NULL
#define PART_NAMED NULL
#define PART_NAMEE NULL
#define PART_NAMEF NULL
#else /*V3R3*/
#define PART_NAME0 "BootLoad"
#define PART_NAME1 "NvBackGU"
#define PART_NAME2 "BootRomA"
#define PART_NAME3 "BootRomB"
#define PART_NAME4 "VxWorks"
#define PART_NAME5 "FastBoot"
#define PART_NAME6 "kernel"
#define PART_NAME7 NULL
#define PART_NAME8 NULL
#define PART_NAME9 NULL
#define PART_NAMEA NULL
#define PART_NAMEB NULL
#define PART_NAMEC NULL
#define PART_NAMED NULL
#define PART_NAMEE NULL
#define PART_NAMEF NULL
#endif
/*----------------------------------------------*
 * �ڲ��������Ͷ���                             
 *----------------------------------------------*/
#define MAX_SCAN_PART_SIZE (50)
typedef struct __part_info_for_scan
{
    unsigned int mtd_idx;
//    unsigned int mtd_size;
    /*���ڳ���50����ķ���,���÷�����Ϊ������
    0-�ϲ��� ��-�²���*/
    unsigned int blk_offset;
}part_info_for_scan;

/*flash������ECCλ����Ϣ*/
typedef struct __ecc_cfg_info
{
    unsigned char bit_offset;   /*ƫ����*/
    unsigned char bit_sum;      /*������*/
    unsigned char rsv1;         /*������*/
    unsigned char rsv2;         /*������*/
}ecc_cfg_info;

/*(nandɨ��)������block��Χ�ṹ*/
typedef struct __nand_scan_config
{
    unsigned int start_blk_id;                               /*��ʼblock id*/
    unsigned int end_blk_id;                                 /*����block id*/
    unsigned int mtd_idx;                                    /*��ӦMTD������*/
}nand_scan_config;

/*flash������Ϣ*/
typedef struct __nand_dev_info
{
    unsigned int page_size;                               /*ҳ��С*/
    unsigned int blk_size;                                /*���С*/
}nand_dev_info;
 
/*ɨ���λ����Ϣ*/
typedef struct __nand_scan_pos_info
{
    /*��ǰɨ��ķ�������(��Ӧscan_blk_scope��idx)��
      ������������, ע���mtd�ķ���ʱ��һ����
      */
    unsigned int part_idx;      
    unsigned int blk_pos;                                /*��λ��*/
    unsigned int page_pos;                               /*ҳλ��*/
}nand_scan_pos_info;
/*
nand��д��Ϣ,Ϊ�˷�ֹ�ڻ�д�����е���,������������:
ɨ��blk->����Σblk���ݲ�д��������I(��Ҫ5��blk��Դ)->
nand��д��Ϣд�뱸����II(��Ҫ3��blk��Դ)->��д->���
������I�ڵ�nand��д��Ϣ.
Ϊ��ֹ��д�����з�������,��������boot�׶δӱ�����II
�ж�ȡnand��д��Ϣ,�����л�д����.
������I�ͱ�����II�Է�������ʽ����,�������ļ�ϵͳ.
*/
typedef struct __nand_wb_info
{
    unsigned int magic;                 /*ħ����*/
    unsigned int sick_mtd_id;           /*��ǰ���д�Ĵ������id*/
    
    /*��ǰ���д�Ĵ������block id, ע����sick_mtd_id������blkƫ��*/
    unsigned int sick_blk_id;           
    
    /*���ݴ����block id-��wbdata������ʼ��ַ��ƫ��*/
    unsigned int backup_blk_id;         

    /*���ݻ�д��Ϣ��block id-��wbdata������ʼ��ַ��ƫ��*/
    unsigned int wb_info_blk_id;        
}nand_wb_info;

/*nandɨ����Ϣ*/
typedef struct __nand_scan_info
{
    nand_scan_pos_info current_pos;                                /*��ǰɨ��ĵ�λ��*/
    unsigned int sec_ecc_sta[REB_MAX_SECT_NUM];                    /*����һ��page������section��ECC״̬��Ϣ*/
    nand_dev_info scan_dev_info;                                   /*��Ҫɨ��flash������Ϣ*/
//#define FLASH_BLK_SIZE scan_dev_info.blk_size
    nand_scan_config scan_blk_scope[REB_ECC_SCAN_MAX_NUM];         /*��Ҫɨ�������block��Χ��*/
}nand_scan_info;

/*----------------------------------------------*
 * �ڲ�����˵��                             *
 *----------------------------------------------*/


/*----------------------------------------------*
 * �ⲿ��������                                 *
 *----------------------------------------------*/
extern void (*set_ecc_result_func)(unsigned int, unsigned int );
extern void (*get_ecc_cfg_func)( unsigned int );

extern unsigned int get_random_num(void);
extern int reb_get_ptable_num(void); 
extern struct ST_PART_TBL * ptable_get(void);   
/*----------------------------------------------*
 * �ڲ�����                                   *
 *----------------------------------------------*/
int ptable_part_num = 0; 
const ST_PART_TBL_ST *pt_part_table = NULL;

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
 /*����ɨ����*/
typedef enum{
    reb_invalid = -1,
    reb_ok = 0,
    reb_sick,
    reb_bad
}page_health_sta;


/*nandɨ����Ϣ*/
static nand_scan_info reb_scan_info = {0};

/*nand��д��Ϣ*/
static nand_wb_info reb_wb_info = {0, 0, 0, 0, 0};

/*һ���ڴ��ַ,����Blk��Ϣ��ΰ���*/
char *mem_for_page = NULL;

/*page��Сһ���ڴ��ַ,ǰ��sizeof(nand_wb_info)�ֽ����ڱ����д��Ϣ*/
char *mem_for_wb_info = NULL;
/*Nand������Ҫɨ��ķ�������*/
unsigned int scan_part_sum = 0;
ecc_cfg_info ecc_cfg[REB_MAX_SECT_NUM] = 
{
#if ( YES == Reb_Platform_V7R2 ) /*Ŀǰֻ���4bit ECC���������*/
    {  0, 8, 0, 0 },                 /*sector 0��ECC����*/
    {  8, 8, 0, 0 },                 /*sector 1��ECC����*/
    { 16, 8, 0, 0 },                 /*sector 2��ECC����*/
    { 24, 8, 0, 0 },                 /*sector 3��ECC����*/
#else
    0
#endif
};


/*Ҫɨ��ķ�������*/
static const char* reb_scan_part_name_array[REB_ECC_SCAN_MAX_NUM] = 
{
    PART_NAME0,
    PART_NAME1,
    PART_NAME2,
    PART_NAME3,
    PART_NAME4,
    PART_NAME5,
    PART_NAME6,
    PART_NAME7,
    PART_NAME8,
    PART_NAME9,
    PART_NAMEA,
    PART_NAMEB,
    PART_NAMEC,
    PART_NAMED,
    PART_NAMEE,
    PART_NAMEF
};


/*****************************************************************************
 �� �� ��  : part_name_to_mtd_index
 ��������  : ��Ҫɨ��MTD���������ҵ�MTD������(��0��ʼ)
 �������  : part_name-������������; 
 �������  : p_mtd_idx-Ҫɨ��MTD����������(��0��ʼ);
 �� �� ֵ  : 0:û���ҵ����������� 1:�ҵ�
*****************************************************************************/
int part_name_to_mtd_index( const char* part_name, unsigned int* p_mtd_idx ) 
{
    unsigned int idx = 0;
    
    if ( ( NULL == part_name ) || ( NULL == p_mtd_idx ) )
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT: addr is NULL\r\n" );
        return 0;
    }

    /*ƥ�����������*/
    for ( idx = 0; idx < ptable_part_num; idx++ )
    {
        if ( strlen( part_name ) != strlen( pt_part_table[idx].name) )
        {
            continue;
        }
        
        /*��ȡ�������ַ:*/
        if( 0 == strcmp(part_name, pt_part_table[idx].name) )
        {

            *p_mtd_idx = idx;
            reb_msg( REB_MSG_ERR, "Reb_DFT: find suited mtd idx\r\n" );
            return 1;
        }
    }

    return 0;
}

/*****************************************************************************
 �� �� ��  : part_name_to_scope
 ��������  : ��Ҫɨ��MTD���������ҵ���blk��Χ(��0��ʼ)
 �������  : input_array-������������; 
 �������  : output_arrayҪɨ�����������(��0��ʼ);
             *p_ret_num-��Ч��������
 �� �� ֵ  : -1:�������� 0:ִ�гɹ�
ע������   : Ŀǰֻ���������block size�ǿ��������
*****************************************************************************/
int part_name_to_scope( const char* input_array[REB_ECC_SCAN_MAX_NUM], 
    nand_scan_config scan_items[REB_ECC_SCAN_MAX_NUM], unsigned int* p_ret_num )
{
    unsigned int idx_input, idx_output = 0;
    unsigned int ret_num = 0;
    int ret_val = 0;
    
    if ( ( NULL == input_array ) || ( NULL == scan_items ) )
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT: addr is NULL\r\n" );
        return -1;
    }

    /*ƥ�����������*/
    for ( idx_input = 0; idx_input < REB_ECC_SCAN_MAX_NUM; idx_input++ )
    {
        if ( NULL == input_array[idx_input] )
        {
            break;
        }

        /*��ȡ��Ӧ�ķ�������*/
        ret_val = part_name_to_mtd_index( input_array[idx_input], &idx_output );
        if ( 0 == ret_val )
        {
            reb_msg( REB_MSG_ERR, "Reb_DFT:NOT find part:%s\r\n",input_array[idx_input] );
        }
        else
        {
            scan_items[ret_num].start_blk_id
                = pt_part_table[idx_output].offset / reb_scan_info.scan_dev_info.blk_size;

#if ( YES == Reb_Platform_V7R2 )
            scan_items[ret_num].end_blk_id = ( pt_part_table[idx_output].offset + \
                pt_part_table[idx_output].capacity ) / reb_scan_info.scan_dev_info.blk_size;
#else
            scan_items[ret_num].end_blk_id = ( pt_part_table[idx_output].offset + \
                pt_part_table[idx_output].loadsize ) / reb_scan_info.scan_dev_info.blk_size;
#endif      
            if ( scan_items[ret_num].end_blk_id > scan_items[ret_num].start_blk_id )
            {   
                scan_items[ret_num].end_blk_id -= 1;
            }
            else
            {
                reb_msg( REB_MSG_ERR, "Reb_DFT:mtd%u start and end blk equal\r\n", idx_output );
            }
            
            scan_items[ret_num].mtd_idx = idx_output;
            ret_num++;
        }
        
    }

    *p_ret_num = ret_num;
    return 0;
    
}
/*****************************************************************************
 �� �� ��  : reb_get_ecc_cfg(����ӿ�)
 ��������  : ��flash��������ȡECC������Ϣ
             ��ǰ��Ҫ��ECC��������(disable/1/4/8bit)
 �������  : ������������ecc type���ͣ�ÿ512byte��������
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void reb_get_ecc_cfg( unsigned int ecctype )
{
    static unsigned int fst_flag = 0;

    if ( 0 == fst_flag )
    {
#if ( YES == Reb_Platform_V7R2 )
        reb_msg( REB_MSG_ERR, "Reb_DFT: eccability = 0x%x\r\n",ecctype);
        flash_ecc_bits = ecctype * 2;  /*����תΪÿ1k byte�ľ���bit��*/
        fst_flag = 1;
#else
        fst_flag = 1;
#endif
    }
}
/*****************************************************************************
 �� �� ��  : reb_set_ecc_result(����ӿ�)
 ��������  : ��page������,��flash��������ȡÿ��ҳ��section��ECCУ����
             ���ѽ�����뵽reb_scan_info.sec_ecc_sta�ֶ�
 �������  : ��������ַ
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void reb_set_ecc_result( unsigned int reg_base, unsigned int offset )
{
    unsigned int value  = 0;
    unsigned int idx = 0;
    unsigned int temp = 0;

    if ( 0 == reg_base )
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT:flash controller reg_base err\r\n");
        return;
    }

    value = *( volatile unsigned int* )( reg_base + offset );
#ifdef REB_OS_DEBUG    
    reb_msg( REB_MSG_DEBUG, "Reb_DFT:flash controller ECC result: 0x%08x\r\n", value );
#endif
    for ( idx = 0; idx < REB_MAX_SECT_NUM; idx++ )
    {
        temp = ( value >> ecc_cfg[idx].bit_offset ) & ( ( 1 << ecc_cfg[idx].bit_sum ) - 1 );
        if ( temp > reb_scan_info.sec_ecc_sta[idx] )
        {
            reb_scan_info.sec_ecc_sta[idx] = temp;
        }
    }
    return;

    /*��flash��������ȡÿ��section�ĵ�λ��ת��Ϣ*/
    /*���������ο�����ƽ̨�Ĵ����ֲ�����
    ��Ϊÿ��һ��page�����ȡһ��λ��ת��Ϣ
    ���ǲ�ȡ������ԭ��: ֻ�е��µķ�ת��Ŀ����ʱ,���ǲ���sec_ecc_sta�и���
    ע���ڶ�ȡ��һ��blkʱ,�����ϢҪ�������*/
}

#ifdef REB_OS_DEBUG
/*****************************************************************************
 �� �� ��  : set_ecc_result_test(����ӿ�)
 ��������  : ��page������,��flash��������ȡÿ��ҳ��section��ECCУ����
             ���ѽ�����뵽reb_scan_info.sec_ecc_sta�ֶ�
 �������  : bit_turned-λ��ת�ĸ���
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void set_ecc_result_test( unsigned int bit_turned )
{
    reb_scan_info.sec_ecc_sta[0] = bit_turned;
    /*�˴���ǰֻ����4bit ECC�ĳ���*/
    /*��flash��������ȡÿ��section�ĵ�λ��ת��Ϣ*/
    /*���������ο�����ƽ̨�Ĵ����ֲ�����*/
}
#endif
/*****************************************************************************
 �� �� ��  : is_sick_blk
 ��������  : ��page/blk������,����reb_scan_info.sec_ecc_sta�ֶ�,�鿴�Ƿ�
             ��Σ
 �������  : none
 �������  : none
 �� �� ֵ  : reb_sick-��Σ reb_ok-���� reb_bad-��Ϊ����
ע������   : �����ɺ�Ҫ���sec_ecc_sta�е���Ϣ
*****************************************************************************/
static page_health_sta is_sick_blk(void)
{
    unsigned int idx = 0;
    unsigned int max_bit_change = 0;

    for ( idx = 0; idx < REB_MAX_SECT_NUM; idx++ )
    {
        if ( max_bit_change < reb_scan_info.sec_ecc_sta[idx] )
        {
            max_bit_change = reb_scan_info.sec_ecc_sta[idx];
        }
    }
    if ( max_bit_change == flash_ecc_bits )
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT:find sick page\r\n" );
        return reb_sick;
    }
    else if ( 0 == max_bit_change ) /*δ����λ��ת*/
    {
#ifdef REB_OS_DEBUG
        reb_msg( REB_MSG_DEBUG, "Reb_DFT:No bit change found\r\n" );
#endif
        return reb_ok;
    }
    else if ( max_bit_change < flash_ecc_bits )
    {
        reb_msg( REB_MSG_INFO, "Reb_DFT:find bit change but not sick page\r\n" );
        return reb_ok;
    }
    else
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT:Maybe find bad page\r\n" );
        return reb_bad;
    }
}
/*****************************************************************************
 �� �� ��  : reb_blk_to_mtd_id
 ��������  : ��blk��λ��part_idx��mtd����id��blockƫ��
            ע������part_idx�Ǽ�¼ɨ������scan_blk_scope[]���������
            ��mtd_id����������
 �������  : blk_id
 �������  : mtd_id blk_offset
 �� �� ֵ  : false:δ�ҵ� true:�ҵ�
*****************************************************************************/
int blk_to_mtd_id(unsigned int blk_id, unsigned int *part_idx,
    unsigned int *mtd_id, unsigned int* blk_offset)
{
    unsigned int idx = 0;
    for ( idx = 0; idx < REB_ECC_SCAN_MAX_NUM; idx++ )
    {
        if ( ( blk_id >= reb_scan_info.scan_blk_scope[idx].start_blk_id ) \
            && ( blk_id <= reb_scan_info.scan_blk_scope[idx].end_blk_id ) )
        {
            if ( NULL != part_idx )
            {
                *part_idx = idx;
            }
            if ( NULL != mtd_id )
            {
                *mtd_id = reb_scan_info.scan_blk_scope[idx].mtd_idx;
            }
            if ( NULL != blk_offset )
            {
                *blk_offset = blk_id - reb_scan_info.scan_blk_scope[idx].start_blk_id;
            }
            
            return true;
        }
    }

    return false;
}
/*****************************************************************************
 �� �� ��  : scan_one_page
 ��������  : ɨ��һ��page
 �������  : blk_id-Ҫɨ��page���ڵ�blk(0-xx), page_id-Ҫɨ���page(0-xx)
             rd_dst-���ݶ�ȡ��Ŀ���ַ
 �������  : none
 �� �� ֵ  : reb_invalid-�������� reb_sick-��Σ reb_ok-���� reb_bad-��Ϊ����
*****************************************************************************/
page_health_sta scan_one_page(unsigned int blk_id, unsigned int page_id, char* rd_dst)
{
    int ret = true;
    page_health_sta ret_val = reb_invalid;
    unsigned int tmp_mtd_idx = 0;
    unsigned int tmp_blk_offset = 0;
    static unsigned int blk_id_bak = 0; /*Ϊ�˱���ÿ�ζ���blk���л����ѯ,ֻ��blk�仯ʱ�ٲ�ѯ*/
    char mtd_name[MAX_MTD_NAME_LEN] = {0}; 
    unsigned long fs_old = 0;        
    int fd = 0;
    long long cur_pos = 0;
    int bad_blk_flg = 0;
    ret = blk_to_mtd_id( blk_id, NULL, &tmp_mtd_idx, &tmp_blk_offset );
    if ( false == ret )
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT: Invalid blk_id: %d\r\n", blk_id );
        return reb_invalid;
    }

    snprintf( mtd_name, sizeof(mtd_name),MTD_NAME_PREFIX, tmp_mtd_idx );

    /*�����ں˷���Ȩ�� */
    fs_old = get_fs();
    
    /*�ı��ں˷���Ȩ�� */
    set_fs(KERNEL_DS);

    /*���ļ�,��ȡ���*/
    fd = sys_open( mtd_name, O_RDONLY, 0 );
    if ( fd < 0 )
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT: Open file %s fail: %d\r\n", mtd_name, fd );
        ret_val = reb_invalid;
        goto err_ret;
    }


    cur_pos = tmp_blk_offset * reb_scan_info.scan_dev_info.blk_size;

    /*�Ƿ����һ���µ�blk*/
    if ( blk_id_bak != blk_id ) 
    {
        blk_id_bak = blk_id;
        memset( reb_scan_info.sec_ecc_sta, 0, sizeof(reb_scan_info.sec_ecc_sta) );
    
        /*�鿴�Ƿ��ǻ���*/
        bad_blk_flg = sys_ioctl(fd, MEMGETBADBLOCK, (unsigned long)&cur_pos);
        if ( bad_blk_flg != 0 )
        {
            reb_msg( REB_MSG_ERR, "Reb_DFT: Get bak blk info fail or bad blk: %d\r\n", bad_blk_flg );
            ret_val = reb_bad;
            goto err_ret;
        }
    }

    /*�ƶ��ļ��ڵ�ָ��,���嵽ĳ��ҳ*/
    cur_pos += page_id * reb_scan_info.scan_dev_info.page_size;
    sys_lseek( fd, cur_pos, SEEK_SET );

    ret = sys_read( fd, rd_dst, reb_scan_info.scan_dev_info.page_size );
    if( ret < 0 )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: %s: sys read fail,the ret is %d\n", __func__, ret );
        ret_val = reb_invalid;
        goto err_ret;
    }

    /*�Ƿ��ǲ�Σ��*/
    ret_val = is_sick_blk();

err_ret:
    if ( fd >= 0 )
    {
        sys_close(fd);
    }
    /*�ָ��ں˷���Ȩ�� */
    set_fs(fs_old);
    return ret_val;

}

/*****************************************************************************
 �� �� ��  : find_first_valid_blk
 ��������  : �ҵ�ĳ�������ڴ�ƫ������ʼ��һ��������(�ǻ���)
 �������  : fd-�򿪵��ļ����, offset-�ļ�ָ���ƫ����(��λ��blk)
 �������  : first_blk������(��0��ʼ)
 �� �� ֵ  : -1-��������(����������/���ǻ���)
 ע������  : ��Σ������ṹ
                    -------------------------------------
                        ��      |                        |
                        Σ      | ��Σ�����ݱ���(5��blk) |
                        ��      |                        |
                        ��      --------------------------
                        ��      | ��д��Ϣ����(3��blk)   |
                        ��      |                        |
                    -------------------------------------
*****************************************************************************/
static int find_first_valid_blk( unsigned int fd, unsigned int* first_blk, unsigned int offset )
{
    struct mtd_info_user as_mtd_info = {0};
    int ret = 0;
    int bad_flg = 0; /*������*/
    unsigned int blk_sum = 0; /*�÷���Blk����*/
    unsigned int idx = 0; /*�÷�����blk����,��0��ʼ*/
    unsigned long long cur_pos =  (unsigned long long)offset *  (unsigned long long)reb_scan_info.scan_dev_info.blk_size; /*��ǰλ��*/

    /*��ȡmtd������Ϣ*/
    ret = sys_ioctl(fd, MEMGETINFO, (unsigned long)(&as_mtd_info));
    if ( ret < 0 )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: sys ioctl get meminfo fail,the ret is %d\n", ret );
        return -1;
    }

    if ( 0 == as_mtd_info.size )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: get mtd size fail\r\n");
        return -1;
    }

    /*�����������������block��*/    
    blk_sum = as_mtd_info.size / as_mtd_info.erasesize;
    if ( blk_sum < ( WB_DATA_BLK_NUM + WB_INFO_BLK_NUM ) )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT:partition to small, size: %d\r\n", blk_sum );
        return -1;
    }

    if ( 0 == offset )
    {
        blk_sum = WB_DATA_BLK_NUM;
    }
    else if ( WB_DATA_BLK_NUM == offset )
    {
        blk_sum = WB_INFO_BLK_NUM;
    }
    else
    {
        reb_msg( REB_MSG_ERR, "REB_DFT:%s,pare offset err\r\n", __func__ );
        return -1;
    }

    for ( idx = 0; idx < blk_sum; idx++ )
    {
        /*�����ļ�ָ��*/
        sys_lseek(fd, cur_pos, SEEK_SET);
    
        bad_flg = sys_ioctl(fd, MEMGETBADBLOCK, (unsigned long)(&cur_pos));
        if( bad_flg < 0 )
        {
            reb_msg( REB_MSG_ERR, "REB_DFT: sys ioctl fail,the ret is %d\n",bad_flg );
            return -1;
        }

        if( bad_flg )
        {
            cur_pos += as_mtd_info.erasesize;
        }
        else
        {
            *first_blk = offset + idx;
            reb_msg( REB_MSG_INFO, "REB_DFT:Find first valid blk:%d success\n",idx );
            return 0;
        }
    }

    reb_msg( REB_MSG_INFO, "REB_DFT:Find first valid blk fail\n" );
    return -1;
}

/*****************************************************************************
 �� �� ��  : backup_wb_data
 ��������  : �ҵ�����/��Ϣ���ݷ����ڵ�һ��������(�ǻ���),�����ݲ�Σ���ݵ���blk
 �������  : part_name-���ݱ���MTD�������� sick_blk-��ǰ��Σ�� 
             src_addr:���ݱ���Ҫʹ�õ��ڴ��ַ
 �������  : pt_back_blk-����Ҫ����λ��(wbdata������blk)
 �� �� ֵ  : -1-��������(����������/���ǻ���)
                    -------------------------------------
                        ��      |                        |
                        Σ      | ��Σ�����ݱ���(5��blk) |
                        ��      |                        |
                        ��      --------------------------
                        ��      | ��д��Ϣ����(3��blk)   |
                        ��      |                        |
                    -------------------------------------
 *****************************************************************************/
int backup_wb_data( char* part_name, unsigned int sick_blk, unsigned int* pt_back_blk, char* src_addr )
{
    int ret_val = -1;
    int fd_sick = -1, fd_bak = -1;
    unsigned long fs_old = 0;        
    char mtd_name[MAX_MTD_NAME_LEN] = {0}; 
    unsigned int idx = 0; 
    struct erase_info_user erase_info = {0};
    unsigned int page_sum_of_blk = ( reb_scan_info.scan_dev_info.blk_size 
        / reb_scan_info.scan_dev_info.page_size );
    unsigned long long sick_fd_pos = 0, bak_fd_pos = 0;
    unsigned int sick_mtd_idx = 0, bak_mtd_idx = 0;
    unsigned int sick_blk_offset = 0;

    if ( ( NULL == part_name ) || ( NULL == pt_back_blk ) || ( NULL == src_addr ) )
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT: Invalid part_name pt_back_blk or src_addr\r\n" );
        return ret_val;
    }

    /*��λ��Σ�����ڵ�MTD, �������MTD�豸�ļ���ƫ��blk*/
    ret_val = blk_to_mtd_id( sick_blk, NULL, &sick_mtd_idx, &sick_blk_offset );
    if ( false == ret_val )
    {
        reb_msg( REB_MSG_ERR, "Reb_DFT: Invalid sick_blk: %d\r\n", sick_blk );
        return ret_val;
    }

    /*��ȡ����MTD�豸����*/
    ret_val = part_name_to_mtd_index( part_name, &bak_mtd_idx );
    if ( 0 == ret_val )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: Invalid partition name\r\n" );
        return -1;
    }
    
    /* �ı��ں˷���Ȩ�� */
    fs_old = get_fs();
    set_fs(KERNEL_DS);

    /*��ϲ�Σ���ݱ���mtd�豸����*/
    snprintf( mtd_name, sizeof(mtd_name),MTD_NAME_PREFIX, bak_mtd_idx);

    /*���ļ�,��ȡ���*/
    fd_bak = sys_open( mtd_name, O_RDWR, 0 );
    if ( 0 > fd_bak )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: Open mtd%d fail, maybe mtd driver not init\r\n", bak_mtd_idx );
        goto err_ret;
    }

    /*��ȡ���ݱ�������һ����Чblock, �������*pt_back_blk*/
    ret_val = find_first_valid_blk( fd_bak, pt_back_blk, 0 );
    if( ret_val < 0 )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: No valid block\r\n" );
        goto err_ret;
    }

    /*����������(��Σ���ݱ��ݿ�)*/
    erase_info.start  = ( *pt_back_blk ) * ( reb_scan_info.scan_dev_info.blk_size );
    erase_info.length = reb_scan_info.scan_dev_info.blk_size;
    ret_val = sys_ioctl(fd_bak, MEMERASE, (unsigned long)&erase_info);
    if( ret_val < 0 )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: erase blk %d fail\r\n", erase_info.start );
        goto err_ret;
    }
    
    /*���ԭ�б�������MTD�豸����,����ϲ�Σ��������MTD�豸����*/
    memset( mtd_name, 0, sizeof(mtd_name) );
    snprintf( mtd_name, sizeof(mtd_name),MTD_NAME_PREFIX, sick_mtd_idx );

    /*ֻ����ʽ�򿪲�Σ��MTD�豸�ļ�,��ȡ���*/
    fd_sick = sys_open( mtd_name, O_RDONLY, 0 );
    if ( 0 > fd_sick )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: Open mtd%d fail, maybe mtd driver not init\r\n", sick_mtd_idx );
        goto err_ret;
    }
    
    /*��ҳΪ��λ���а���:��Σ��->���ݱ��ݿ�*/
    sick_fd_pos =  (unsigned long long)sick_blk_offset *  (unsigned long long)reb_scan_info.scan_dev_info.blk_size;
    bak_fd_pos  =  (unsigned long long)( *pt_back_blk ) *  (unsigned long long)reb_scan_info.scan_dev_info.blk_size;    
    for ( idx = 0; idx < page_sum_of_blk; idx++ )
    {
        /*�ƶ���Σ���ļ��ڵ�ָ��,���嵽ĳ��ҳ,����ȡ��ҳ����*/
        sys_lseek( fd_sick, sick_fd_pos, SEEK_SET );
        ret_val = sys_read( fd_sick, src_addr, reb_scan_info.scan_dev_info.page_size );
        if( ret_val < 0 )
        {
            reb_msg( REB_MSG_ERR, "REB_DFT: %s: sys read fail,the ret is %d\n", __func__, ret_val );
            goto err_ret;
        }
        sick_fd_pos += reb_scan_info.scan_dev_info.page_size;

        reb_delay_ms(1);

        /*�ƶ����ݱ��ݿ��ļ��ڵ�ָ��,���嵽ĳ��ҳ,��д��ҳ����*/
        sys_lseek( fd_bak, bak_fd_pos, SEEK_SET );
        ret_val = sys_write( fd_bak, src_addr, reb_scan_info.scan_dev_info.page_size );
        if( ret_val < 0 )
        {
            reb_msg( REB_MSG_ERR, "REB_DFT: %s:sys write fail,the ret is %d\n", __func__, ret_val );
            goto err_ret;
        }
        bak_fd_pos += reb_scan_info.scan_dev_info.page_size;

        reb_delay_ms(1);
    }
    
err_ret:
    if ( fd_sick >= 0 )
    {
        sys_close(fd_sick);
    }
    if ( fd_bak >= 0 )
    {
        sys_close(fd_bak);
    }
    set_fs(fs_old);
    return ret_val;
}
/*****************************************************************************
 �� �� ��  : record_wb_info
 ��������  : ��¼��д��Ϣ
 �������  : part_name-MTD��������; Դ���ݾ���nand_wb_info�ṹ��ַ
 �������  : none
 �� �� ֵ  : -1-��������(����������/���ǻ���)
                    -------------------------------------
                        ��      |                        |
                        Σ      | ��Σ�����ݱ���(5��blk) |
                        ��      |                        |
                        ��      --------------------------
                        ��      | ��д��Ϣ����(3��blk)   |
                        ��      |                        |
                    -------------------------------------
 *****************************************************************************/
int record_wb_info( char* part_name, nand_wb_info* pt_wb_info, const unsigned int wr_num )
{
    int fd = 0;
    int ret_val = -1;
    unsigned long fs_old = 0;        
    char mtd_name[MAX_MTD_NAME_LEN] = {0}; 
    unsigned int idx = 0; 
    struct erase_info_user erase_info = {0};

    /*��ȡ����MTD�豸����*/
    ret_val = part_name_to_mtd_index( part_name, &idx );
    if ( 0 == ret_val )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: Invalid partition name\r\n" );
        return ret_val;
    }

    /* �ı��ں˷���Ȩ�� */
    fs_old = get_fs();
    set_fs(KERNEL_DS);

    /*����mtdx*/
    snprintf( mtd_name,sizeof(mtd_name), MTD_NAME_PREFIX, idx );


    /*���ļ�,��ȡ���*/
    fd = sys_open( mtd_name, O_RDWR, 0 );
    if ( 0 > fd )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: Open mtd%d fail, maybe mtd driver not init\r\n", idx );
        goto err_ret;
    }

    /*��ȡ��һ����Чblock*/
    ret_val = find_first_valid_blk( fd, &idx, WB_DATA_BLK_NUM );
    if( ret_val < 0 )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: No valid block\r\n" );
        goto err_ret;
    }

    /*Ҫ��¼��д��Ϣ*/
    if ( OUR_MAGIC_NUM == pt_wb_info->magic )
    {
        pt_wb_info->wb_info_blk_id = idx;
    }

    /*����������*/
    erase_info.start  = idx * reb_scan_info.scan_dev_info.blk_size;
    erase_info.length = reb_scan_info.scan_dev_info.blk_size;
    ret_val = sys_ioctl(fd, MEMERASE, (unsigned long)&erase_info);
    if( ret_val < 0 )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: erase blk %d fail\r\n", erase_info.start );
        goto err_ret;
    }
    
    /*�����ļ�ָ��*/
    sys_lseek(fd, idx * reb_scan_info.scan_dev_info.blk_size, SEEK_SET);

    /*��ҳΪ��λ����д����*/
    memcpy( mem_for_wb_info, pt_wb_info, wr_num );
    ret_val = sys_write( fd, mem_for_wb_info, reb_scan_info.scan_dev_info.page_size );
    if( ret_val < 0 )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: %s:sys write fail,the ret is %d\n", __func__, ret_val );
        goto err_ret;
    }
    
err_ret:
    if ( fd >= 0 )
    {
        sys_close(fd);
    }
    set_fs(fs_old);
    return ret_val;
}
/*****************************************************************************
 �� �� ��  : wb_sick_blk_data
 ��������  : (���ݲ�Σ���ݼ���д��Ϣ��)��д��Σ��
 �������  : part_name-��Σ�����ݱ��ݷ������� 
             src_addr-��page�������ݰ���ʱ���ڴ��ַ pt_wb_info-��д��Ϣ
 �������  : none
 �� �� ֵ  : -1-��������(����������/���ǻ���)
 ע������  : �������kernel����:�ڴ�->��Σ��
 *****************************************************************************/
int wb_sick_blk_data( char* part_name, char* src_addr, nand_wb_info* pt_wb_info )
{
    int fd_sick = -1, fd_bak = -1;
    int ret_val = -1;
    unsigned int idx = 0; 
    unsigned long fs_old = 0;        
    char mtd_name[MAX_MTD_NAME_LEN] = {0}; 
    unsigned long long sick_fd_pos = 0, bak_fd_pos = 0;
    unsigned int sick_mtd_idx = 0, bak_mtd_idx = 0;
    struct erase_info_user erase_info = {0};
    unsigned int page_sum_of_blk = ( reb_scan_info.scan_dev_info.blk_size 
        / reb_scan_info.scan_dev_info.page_size );

    /*�鿴����Ƿ�Ϸ�*/
    if ( ( NULL == src_addr ) || ( NULL == pt_wb_info ) || ( NULL == part_name ) )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT:Invalid addr\r\n" );
        return ret_val;
    }

    /* �ı��ں˷���Ȩ�� */
    fs_old = get_fs();
    set_fs(KERNEL_DS);

    /*��ȡ����MTD�豸����*/
    ret_val = part_name_to_mtd_index( part_name, &bak_mtd_idx );
    if ( 0 == ret_val )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: Invalid partition name\r\n" );
        set_fs(fs_old);
        return ret_val;
    }

    /*��ϱ��ݿ�����MTD�豸����*/
    snprintf( mtd_name, sizeof(mtd_name),MTD_NAME_PREFIX, bak_mtd_idx );

    /*ֻ����ʽ���ļ�,��ȡ���ݿ�����MTD�豸�ļ����*/
    fd_bak = sys_open( mtd_name, O_RDONLY, 0 );
    if ( 0 > fd_bak )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: Open mtd%d fail, maybe mtd driver not init\r\n", bak_mtd_idx );
        goto err_ret;
    }

    /*��ϲ�Σ������MTD�豸����*/
    sick_mtd_idx = pt_wb_info->sick_mtd_id;
    memset( mtd_name, 0, sizeof(mtd_name) );
    snprintf( mtd_name, sizeof(mtd_name), MTD_NAME_PREFIX, sick_mtd_idx );

    /*���ļ�,��ȡ��Σ������MTD�豸�ļ����*/
    fd_sick = sys_open( mtd_name, O_RDWR, 0 );
    if ( 0 > fd_sick )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: Open mtd%d fail, maybe mtd driver not init\r\n", sick_mtd_idx );
        goto err_ret;
    }

    /*������Σ������*/
    erase_info.start  = (pt_wb_info->sick_blk_id )*reb_scan_info.scan_dev_info.blk_size;
    erase_info.length = reb_scan_info.scan_dev_info.blk_size;
    ret_val = sys_ioctl(fd_sick, MEMERASE, (unsigned long)&erase_info);
    if( ret_val < 0 )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: erase blk %d fail\r\n", erase_info.start );
        goto err_ret;
    }
    
    /*��ҳΪ��λ���л�д:���ݱ��ݿ�->��Σ��*/
    bak_fd_pos  =  (unsigned long long)( pt_wb_info->backup_blk_id ) *  (unsigned long long)reb_scan_info.scan_dev_info.blk_size;    
    sick_fd_pos =  (unsigned long long)( pt_wb_info->sick_blk_id ) *  (unsigned long long)reb_scan_info.scan_dev_info.blk_size;
    for ( idx = 0; idx < page_sum_of_blk; idx++ )
    {
        /*�ƶ����ݱ��ݿ��ļ��ڵ�ָ��,���嵽ĳ��ҳ,����ȡ��ҳ����*/
        sys_lseek( fd_bak, bak_fd_pos, SEEK_SET );
        ret_val = sys_read( fd_bak, src_addr, reb_scan_info.scan_dev_info.page_size );
        if( ret_val < 0 )
        {
            reb_msg( REB_MSG_ERR, "REB_DFT: %s: sys read fail,the ret is %d\n", __func__, ret_val );
            goto err_ret;
        }
        bak_fd_pos += reb_scan_info.scan_dev_info.page_size;

        /*�ƶ���Σ���ļ��ڵ�ָ��,���嵽ĳ��ҳ,����д��ҳ����*/
        sys_lseek( fd_sick, sick_fd_pos, SEEK_SET );
        ret_val = sys_write( fd_sick, src_addr, reb_scan_info.scan_dev_info.page_size );
        if( ret_val < 0 )
        {
            reb_msg( REB_MSG_ERR, "REB_DFT: sys write fail,the ret is %d\n",ret_val );
            goto err_ret;
        }
        sick_fd_pos += reb_scan_info.scan_dev_info.page_size;
    }
    
err_ret:
    if ( fd_sick >= 0 )
    {
        sys_close(fd_sick);
    }
    if ( fd_bak >= 0 )
    {
        sys_close(fd_bak);
    }
    set_fs(fs_old);
    return ret_val;
}
/*****************************************************************************
 �� �� ��  : reb_scan_flash_parts
 ��������  : �������ɨ�財Σ��(��main task����)
            ����->�����д��Ϣ->��д->�����д��Ϣ
 �������  : none
 �������  : the_pos:��Σ���λ����Ϣ
 �� �� ֵ  : reb_invalid-�������� reb_sick-��Σ reb_ok-���� reb_bad-��Ϊ����
*****************************************************************************/
int reb_scan_flash_parts( void )
{
    int ret_val = 0;

    /*����scan_blk_scope������*/
    unsigned int cur_part_idx = reb_scan_info.current_pos.part_idx; 
    unsigned int cur_blk_ops = reb_scan_info.current_pos.blk_pos;
    unsigned int cur_page_ops = reb_scan_info.current_pos.page_pos; /*����:0~63*/
    unsigned int page_sum_of_blk = ( reb_scan_info.scan_dev_info.blk_size 
        / reb_scan_info.scan_dev_info.page_size );

    if ( 0 == scan_part_sum )
    {
        reb_msg( REB_MSG_DEBUG, "REB_DFT:No block need to scan\r\n" );
        return reb_ok;
    }
    /*ɨ��һ��page*/
    ret_val = scan_one_page( cur_blk_ops, cur_page_ops, mem_for_page );
    if ( 0 == flash_ecc_bits )
    {
        scan_part_sum = 0;
        reb_msg( REB_MSG_INFO, "REB_DFT:Set scan_part_sum = 0\r\n" );
        return reb_ok;
    }
    /* �����д����û�д� */
    if( false == g_kernel_soft_reliable_info.BadZoneReWriteEnable )
    {
        goto next_page;
    }

    if ( reb_sick == ret_val )
    {
        /*���ݲ�Σblk���ݵ�flash������I,���������blk����Ϣ*/
        ret_val = backup_wb_data( PART_WB_DATA, cur_blk_ops, &reb_wb_info.backup_blk_id, mem_for_page );
        if ( ret_val < 0 )
        {
            goto next_block;
        }

        /*�����д��Ϣ*/
        reb_wb_info.magic = OUR_MAGIC_NUM;
        reb_wb_info.sick_mtd_id = reb_scan_info.scan_blk_scope[cur_part_idx].mtd_idx;
        reb_wb_info.sick_blk_id = ( reb_scan_info.current_pos.blk_pos \
            - reb_scan_info.scan_blk_scope[cur_part_idx].start_blk_id );

        /*��¼��д��Ϣ��������II, �������д��Ϣblk*/
        ret_val = record_wb_info( PART_WB_DATA, &reb_wb_info, sizeof(nand_wb_info) );
        if ( ret_val < 0 )
        {
            goto next_block;
        }
        
        /*��д��Σblk*/
        ret_val = wb_sick_blk_data( PART_WB_DATA, mem_for_page, &reb_wb_info );
        if ( ret_val < 0 )
        {
            goto next_block;
        }

        /*���flash������II�Ļ�д��Ϣ*/
        reb_wb_info.magic = 0;
        ret_val = record_wb_info( PART_WB_DATA, &reb_wb_info, sizeof(nand_wb_info) );
        if ( ret_val < 0 )
        {
            reb_msg( REB_MSG_INFO, "REB_DFT:record_wb_info fail\r\n" );
        }
        goto next_block;
    }
    else if ( ( reb_bad == ret_val ) || ( reb_invalid == ret_val ) )
    {
        reb_msg( REB_MSG_INFO, "REB_DFT:Find a bad or invalid blk\r\n" );
        goto next_block;
    }
    
next_page:
    cur_page_ops = ++reb_scan_info.current_pos.page_pos; /*page++*/

    /*ĳ��blk����page�Ѿ�ɨ�����,ָ����һ��blk*/
    if ( page_sum_of_blk == cur_page_ops )
    {
    
next_block:
        cur_blk_ops = ++reb_scan_info.current_pos.blk_pos;      /*blk++*/
        reb_scan_info.current_pos.page_pos = 0;  /*page=0*/

        /*ĳ��part(ition)���е�blkɨ�����, ָ����һ��part*/
        if ( cur_blk_ops > reb_scan_info.scan_blk_scope[cur_part_idx].end_blk_id )
        {
            cur_part_idx = ++reb_scan_info.current_pos.part_idx;  /*part++*/
            reb_scan_info.current_pos.blk_pos = \
                reb_scan_info.scan_blk_scope[cur_part_idx].start_blk_id;
            cur_blk_ops = 0;

            /*���з����Ѿ�ɨ�����,ֹͣɨ��*/
            if ( scan_part_sum <= cur_part_idx )
            {
                reb_msg( REB_MSG_INFO, "REB_DFT:all scan is complete\r\n" );
                scan_part_sum = 0;
                cur_part_idx = 0;
            }
        }
        
    }
    
    return ret_val;
}
EXPORT_SYMBOL(reb_scan_flash_parts);

/*****************************************************************************
 �� �� ��  : get_flash_dev_info
 ��������  : ��ȡflash�豸��ҳ/����Ϣ
 �������  : none 
 �������  : nand_dev_info
 �� �� ֵ  : -1: ����mtd�豸ʧ��
*****************************************************************************/
int get_flash_dev_info( nand_dev_info* pt_dev_info )
{
    long fd = 0;
    int ret_val = 0;
    unsigned long fs_old = 0;        
    struct mtd_info_user as_mtd_info = {0};
    char mtd_name[MAX_MTD_NAME_LEN] = {0}; 

    /*����mtd0*/
    snprintf( mtd_name,sizeof(mtd_name),MTD_NAME_PREFIX, ret_val );

    /* �ı��ں˷���Ȩ�� */
    fs_old = get_fs();
    set_fs(KERNEL_DS);

    /*���ļ�,��ȡ���*/
    fd = sys_open( mtd_name, O_RDONLY, 0 );
    if ( fd < 0 )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: Open mtd0 fail, maybe mtd driver not init\r\n");
        ret_val = -1;
        goto err_ret;
    }

    /*��ȡmtd������Ϣ*/
    ret_val = sys_ioctl(fd, MEMGETINFO, (unsigned long)(&as_mtd_info));
    if ( ret_val < 0 )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: sys_ioctl fails, maybe mtd driver not init\r\n");
        goto err_ret;
    }
    
    pt_dev_info->blk_size = as_mtd_info.erasesize;
    pt_dev_info->page_size = as_mtd_info.writesize;
    
err_ret:
    if ( fd >= 0 )
    {
        sys_close(fd);
    }
    set_fs(fs_old);
    return ret_val;

}
/*****************************************************************************
 �� �� ��  : part_scan_pos_init
 ��������  : ��ʼ��(��ʼ)ɨ��λ��
 �������  : none 
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void part_scan_pos_init(void)
{
    part_info_for_scan temp_struct[32] = {0}; /*���֧��32�������λ*/
    unsigned int idx = 0;
    unsigned int struct_idx = 0;
    unsigned int random = 0;
    unsigned int blk_sum = 0;

    for ( idx = 0; idx < scan_part_sum; idx++ )
    {
        temp_struct[struct_idx].mtd_idx = reb_scan_info.scan_blk_scope[idx].mtd_idx;
        temp_struct[struct_idx].blk_offset
            = reb_scan_info.scan_blk_scope[idx].start_blk_id;

        /*�鿴�Ƿ񳬹�50����*/
        blk_sum = ( reb_scan_info.scan_blk_scope[idx].end_blk_id + 1 )
            - reb_scan_info.scan_blk_scope[idx].start_blk_id;
        if ( blk_sum >= MAX_SCAN_PART_SIZE )
        {
            struct_idx++;
            temp_struct[struct_idx].mtd_idx = reb_scan_info.scan_blk_scope[idx].mtd_idx;
            /*��¼�������°벿*/
            temp_struct[struct_idx].blk_offset
                = reb_scan_info.scan_blk_scope[idx].start_blk_id
                + blk_sum / 2;/*��������°벿*/
        }
        /*�ۼ�,׼����һ�μ�¼*/
        struct_idx++;
    }

    if(!struct_idx)
    {
        return;
    }
    /*��ȡ�����*/
    random = get_random_num();
    random &= 0xff; /*0~255*/
    reb_msg( REB_MSG_INFO, "Reb_DFT now random is %u, struct_idx is: %u\r\n", 
        random, struct_idx );
    random %= struct_idx;
    reb_msg( REB_MSG_INFO, "Reb_DFT at last random is %u\r\n", random );

    /*��ʼ��ɨ��λ��*/
    reb_scan_info.current_pos.blk_pos = temp_struct[random].blk_offset;
    (void)blk_to_mtd_id( reb_scan_info.current_pos.blk_pos, 
        &reb_scan_info.current_pos.part_idx, NULL, NULL );
    reb_scan_info.current_pos.page_pos = 0;    
}
/*****************************************************************************
 �� �� ��  : reb_part_scan_init
 ��������  : ��ʼ��(ע��Ҫ��linux mtd������ʼ�����֮��)
 �������  : none 
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
int reb_part_scan_init(void)
{
    int ret_val = 0;
    unsigned int cnt = 0;

    /*��ȡNV��*/
    if ( false == g_kernel_soft_reliable_info.BadZoneScanEnable )
    {
        reb_msg( REB_MSG_INFO, "REB_DFT:Nand scan not enable\r\n");
        goto notice_main_task;
    }

    pt_part_table = ptable_get();
    if(NULL == pt_part_table)
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: invalid pt_part_table\n");
        return 0;
    }

    ptable_part_num = reb_get_ptable_num();
    if(0 >= ptable_part_num)
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: invalid part num %d\n",ptable_part_num);
        return 0;
    }
    reb_msg( REB_MSG_ERR, "REB_DFT: ptable_part_num %d\n",ptable_part_num);

    /*��ȡflash��������Ϣ*/
    for ( cnt = 0; cnt < MAX_TRY_TIME; cnt++ )
    {
        ret_val = get_flash_dev_info( &reb_scan_info.scan_dev_info );
        if ( 0 == ret_val )
        {
            break;
        }

        /*���ʱ����Ҫ�ȴ�һ��,��Ϊ����mtd�豸���ܻ�û�м���*/
        reb_delay_ms(MS_OF_ONE_SECOND);
    }
    if ( MAX_TRY_TIME == cnt )
    {
        goto notice_main_task;
    }

    /*��ȡҪɨ��ķ�����Ϣ*/
    ret_val = part_name_to_scope( reb_scan_part_name_array, reb_scan_info.scan_blk_scope, &scan_part_sum );
    if ( ret_val )
    {
        scan_part_sum = 0;
        goto notice_main_task;
    }
    if(0 == scan_part_sum)
    {
        goto notice_main_task;
    }
    /*��ʼ��ɨ��λ��*/
    part_scan_pos_init();

    /*����һ���ڴ�:ÿ��ɨ��һ��page*/
    mem_for_page = (char *)kmalloc(reb_scan_info.scan_dev_info.page_size + 16, GFP_KERNEL);
    if ( NULL == mem_for_page )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT:Not enough memory\r\n");
        scan_part_sum = 0;
        goto notice_main_task;
    }

    /*���ڴ���һ����־,��ֹ�ڴ�дԽ��*/
    *(unsigned int *)( mem_for_page ) = OUR_MAGIC_NUM;

    mem_for_wb_info = (char *)kmalloc( reb_scan_info.scan_dev_info.page_size, GFP_KERNEL );
    if ( NULL == mem_for_wb_info )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT:Not enough memory for mem_for_wb_info\r\n");
        scan_part_sum = 0;
        goto notice_main_task;
    }
    set_ecc_result_func = ( void(*)( unsigned int, unsigned int ) )reb_set_ecc_result;
    get_ecc_cfg_func = ( void(*)( unsigned int ) )reb_get_ecc_cfg;
notice_main_task:
    /*֪ͨ��������NANDɨ��ģ���ʼ�����*/
    reb_send_msg_to_main_task( REB_NAND_SCAN_INIT_EVENT );
    return ret_val;
}
#ifdef REB_OS_DEBUG
/*****************************************************************************
 �� �� ��  : show_cur_scan_pos
 ��������  : ��ʾ��ǰɨ����Ϣ
 �������  : none
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void show_cur_scan_pos(void)
{
    unsigned int idx = reb_scan_info.current_pos.part_idx;
    reb_msg( REB_MSG_DEBUG, "\r\nREB_DFT:Current mtd: %d\r\n", reb_scan_info.scan_blk_scope[idx].mtd_idx );
    reb_msg( REB_MSG_DEBUG, "REB_DFT:Current blk: %d\r\n", reb_scan_info.current_pos.blk_pos );
    reb_msg( REB_MSG_DEBUG, "REB_DFT:Current page: %d\r\n", reb_scan_info.current_pos.page_pos );
}

/*****************************************************************************
 �� �� ��  : show_all_scan_info
 ��������  : ��ʾ��Σ��ɨ����Ϣ
 �������  : none
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void show_all_scan_info(void)
{
    unsigned int part_idx = 0;

    for ( part_idx = 0; part_idx < scan_part_sum; part_idx++ )
    {
        reb_msg( REB_MSG_DEBUG, "\r\nREB_DFT:mtd idx: mtd%d\r\n", reb_scan_info.scan_blk_scope[part_idx].mtd_idx );
        reb_msg( REB_MSG_DEBUG, "REB_DFT:start blk: %d\r\n", reb_scan_info.scan_blk_scope[part_idx].start_blk_id );
        reb_msg( REB_MSG_DEBUG, "REB_DFT:  end blk: %d\r\n", reb_scan_info.scan_blk_scope[part_idx].end_blk_id );
    }
}
#endif
//late_initcall(reb_part_scan_init);



