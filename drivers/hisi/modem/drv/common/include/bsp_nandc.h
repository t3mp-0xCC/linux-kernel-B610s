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

#ifndef __BSP_NANDC_H__
#define __BSP_NANDC_H__

#ifdef __cplusplus /* __cplusplus */
extern "C"
{
#endif /* __cplusplus */

#include <osl_types.h>
#include <hi_nandc.h>
#include "ptable_com.h"

#define NV_WRITE_SUCCESS     ((unsigned char)0) /* for NV */

#define NANDC_SIZE_HK                       (512)
#define NANDC_SIZE_1K                       (1024)
#define NANDC_SIZE_2K                       (2048)
#define NANDC_SIZE_4K                       (4096)
#define NANDC_SIZE_8K                       (8192)
#define NANDC_SIZE_16K                      (16384)

#define NANDC_BLOCK_64PAGE                  (64)
#define NANDC_BLOCK_128PAGE                 (128)
#define NANDC_BLOCK_192PAGE                 (192)
#define NANDC_BLOCK_256PAGE  		        (256)
#define NANDC_BLOCK_512PAGE 		        (512)
#define NANDC_BLOCK_768PAGE                 (768)

#define NANDC_ADDRCYCLE_0                   (0)
#define NANDC_ADDRCYCLE_1                   (1)
#define NANDC_ADDRCYCLE_4                   (4)
#define NANDC_ADDRCYCLE_5                   (5)

/* ��֥nand flash�ӹ����գ�����ȷ��SLC Nand��ecc type��Benand��spare size */
#define NAND_TOSHIBA_43NM				4
#define NAND_TOSHIBA_32NM				5
#define NAND_TOSHIBA_24NM				6
#define NAND_TOSHIBA_TECH_MASK			7


#if defined(NANDC_USE_V400)
#define nandc4_ecc_none                      (0x0)
#define nandc4_ecc_1bit                      (0x2)
#define nandc4_ecc_4smb                      (0x4)
#define nandc4_ecc_24p512bit                 (0x6)
#define nandc4_ecc_24p1kbit                  (0x8)

#define NANDC_ECC_NONE              nandc4_ecc_none
#define NANDC_ECC_1BIT              nandc4_ecc_1bit
#define NANDC_ECC_4BIT              nandc4_ecc_4smb
#define NANDC_ECC_8BIT              nandc4_ecc_24p1kbit
#define NANDC_ECC_24BIT_1K          nandc4_ecc_24p1kbit

#else /* defined(NANDC_USE_V600) */
#define nandc6_ecc_none                      (0x0)
#define nandc6_ecc_1bit                      (0x2)
#define nandc6_ecc_4smb                      (0x4)
#define nandc6_ecc_8bit                      (0x6)
#define nandc6_ecc_24p1kbit                  (0x8)
#define nandc6_ecc_40bit                     (0xa)
#define nandc6_ecc_64bit                     (0xc)

#define NANDC_ECC_NONE              nandc6_ecc_none
#define NANDC_ECC_1BIT              nandc6_ecc_1bit
#define NANDC_ECC_4BIT              nandc6_ecc_4smb
#define NANDC_ECC_8BIT              nandc6_ecc_8bit
#define NANDC_ECC_24BIT_1K          nandc6_ecc_24p1kbit

#endif

#if (FEATURE_ON == MBB_COMMON)
#define ECC_1BIT_CORRECT_BITS                    (1)  /* 1bit����������512 byte�ɾ����bitλ */
#define ECC_4BIT_CORRECT_BITS                    (4)  /* 4bit����������512 byte�ɾ����bitλ */
#define ECC_8BIT_CORRECT_BITS                    (8)  /* 8bit����������512 byte�ɾ����bitλ */
#define ECC_24BIT_1K_CORRECT_BITS                (12) /* 24bit����������512 byte�ɾ����bitλ */
#endif
/*structure of nand chip specification parameter */
struct nand_spec
{
    u32 blocksize;          /*one block size of nand flash*/
    u32 pagesize;           /*one page size of nand flash*/
    u32 chipsize;           /*the total size of nand flash*/
    u32 sparesize;           /*the total size of nand flash*/

    u32 pagenumperblock;
	u32 ecctype;            /* selected by nandc, not nand itself */
	u32 buswidth;
	u32 addrnum;
};

struct nand_spec_shared_mem
{
	unsigned int chip_size;	            
    unsigned int page_size;             
	unsigned int spare_size;    
    unsigned int block_size;              
    unsigned int flag;             
    
    unsigned short reserved;    
    
    unsigned char ecc_type;             /* ���ֵ��Ĵ������õ�ecc typeֵ��ͬ */
    unsigned char addr_num;         
};
#define SHM_MEM_NAND_SPEC_ADDR          (SHM_BASE_ADDR + SHM_OFFSET_NAND_SPEC)
/* ����ӿ����� */
#if (FEATURE_ON == MBB_COMMON)

unsigned int ecc_type( unsigned int ctrl_cfg);
#endif

#ifdef __FASTBOOT__
/**
 * Name			:	bsp_nand_read
 * Arguments	:	@partition_name  	- partition name
 * 					@partition_offset	- offset from partition to read from
 * 					@ptr_ram_addr  	- pointer to ram addr to store the data read from flash
 * 					@length        	- number of bytes to read
 *                  @skip_len       - bad block length skipped(Byte)
 * Return		:	0				- success
 *				:   else  			- failure
 * Desciption	:	NULL
 */
int bsp_nand_read(const char *partition_name, u32 partition_offset, void* ptr_ram_addr, u32 length, u32 *skip_len);

/**
 * Name			:	bsp_nand_erase
 * Arguments	:	@partition_name  	- partition name
 * 					@partition_offset	- offset from partition to erase
 * Return		:	0 				- success
 *				:   else  			- failure
 * Desciption	:	NULL
 */
int bsp_nand_erase(const char *partition_name, u32 partition_offset);

/**
 * Name			:	bsp_nand_write
 * Arguments	:	@partition_name  	- partition name
 * 					@partition_offset	- offset from partition to write to, must be page align.
 * 					@ptr_ram_addr  	- pointer to ram addr to store the data write to flash
 * 					@length        	- number of bytes to write
 * Return		:	0 				- success
 *				:   else  			- failure
 * Description	: 	write flash
 */
int bsp_nand_write(const char *partition_name, u32 partition_offset, void* ptr_ram_addr, u32 length);

/**
 * Name			: 	bsp_nand_isbad
 * Arguments	: 	@partition_name  	- partition name
 * 					@partition_offset	- block offset from partition to check
 *
 * Return		:	0 				- good block
 *				:   1  			    - bad block
 *              :   -1              - error
 * Description	: 	check whether a block is bad
 */
int bsp_nand_isbad(const char *partition_name, u32 partition_offset);

/**
 * Name			:	bsp_nand_blk_write_nv
 * Arguments	:	@partition_name  	- partition name
 * 					@partition_offset   - offset from partition to read from, must be block aligned.
 * 					@flag        	- flag value, 1 or 0
 * Return		:	0 				- success
 *				:   else  			- failure
 * Description	: 	read nv_flag of a block
 */
u32 bsp_nand_read_flag_nv(const char *partition_name, u32 partition_offset, unsigned char *flag);

/****************************************************************
��������: ��fastboot�׶λ�ȡ������С
�������: partition_name-��������
�������: none
���ز���: ����size
ע������: ��fastboot�׶ε���
****************************************************************/
unsigned int bsp_get_part_cap( const char *partition_name );
/**
 * Name			: 	bsp_update_ptable_in_nandc
 * Arguments	: 	null
 *
 * Return		:	null
 * Description	: 	update ptable to nandc
 */
void bsp_update_ptable_to_nandc(void);

/**
 * Name			:	bsp_get_nand_info
 * Arguments	:	@spec  		    - to store flash info
 * Return		:	0 				- success
 *				:   else  			- failure
 * Desciption	:	the ecctype info is the type used by nandc, not nand flash itself recommend.
 */
int bsp_get_nand_info(struct nand_spec *spec);

/**
 * Name			: 	bsp_erase_yaffs_partitons
 * Arguments	: 	null
 *
 * Return		:	null
 * Description	: 	erase all yaffs partitions
 */
void bsp_erase_yaffs_partitons(void);

/**
 * Name			: 	bsp_nand_argument_check
 * Arguments	: 	@argu: string input to check
 *
 * Return		:	null
 * Description	: 	check whether partition name is legal.
 */
char *bsp_nand_argument_check(char *argu);

/**
 * Name			: 	bsp_nand_erase_force
 * Arguments	: 	@blockid - block id
 *
 * Return		:	null
 * Description	: 	erase a block force whether it is bad
 */
void bsp_nand_erase_force(char *blockid);

/**
 * Name			: 	bsp_nand_markbad
 * Arguments	: 	@blockid - block id
 *
 * Return		:	null
 * Description	: 	mark a block bad
 */
void bsp_nand_markbad(char *blockid);

/**
 * Name			: 	bsp_update_size_of_lastpart
 * Arguments	: 	@new_ptable - addr of new ptable
 *
 * Return		:	null
 * Description	: 	update the size of cdrom partition to shared memory
 */
void bsp_update_size_of_lastpart(struct ST_PART_TBL *new_ptable);

#elif defined(__KERNEL__)

#include <linux/mtd/mtd.h>

/**
 * Name			:	bsp_nand_blk_write_nv
 * Arguments	:	@mtd  	        - mtd device
 * 					@to	            - offset from partition to write to, must be page align.
 * 					@len  	        - write data lenth
 * 					@retlen        	- write length actually
 * 					@buf        	- source data address
 * Return		:	0 				- success
 *				:   else  			- failure
 * Description	: 	write flash for nv, as writing data and oob for last page, else only data
 */
int bsp_nand_blk_write_nv(struct mtd_info *mtd, loff_t to, size_t len, const uint8_t *buf);

/**
 * Name			:	bsp_nand_read
 * Arguments	:	@partition_name  	- partition name
 * 					@partition_offset	- offset from partition to read from
 * 					@ptr_ram_addr  	- pointer to ram addr to store the data read from flash
 * 					@length        	- number of bytes to read
 *                  @skip_len       - bad block length skipped(Byte)
 * Return		:	0				- success
 *				:   else  			- failure
 * Desciption	:	NULL
 */
int bsp_nand_read(const char *partition_name, loff_t partition_offset, void* ptr_ram_addr, size_t length, u32 *skip_len);

/**
 * Name			:	bsp_nand_erase
 * Arguments	:	@partition_name  	- partition name
 * 					@partition_offset	- offset from partition to erase
 * Return		:	0 				- success
 *				:   else  			- failure
 * Desciption	:	NULL
 */
int bsp_nand_erase(const char *partition_name, loff_t partition_offset);

/**
 * Name			:	bsp_nand_write
 * Arguments	:	@partition_name  	- partition name
 * 					@partition_offset	- offset from partition to write to, must be page align.
 * 					@ptr_ram_addr  	- pointer to ram addr to store the data write to flash
 * 					@length        	- number of bytes to write
 * Return		:	0 				- success
 *				:   else  			- failure
 * Description	: 	write flash
 */
s32 bsp_nand_write(const char *partition_name, loff_t partition_offset, void* ptr_ram_addr, size_t length);

/**
 * Name			: 	bsp_nand_isbad
 * Arguments	: 	@partition_name  	- partition name
 * 					@partition_offset	- block offset from partition to check
 *
 * Return		:	0 				- good block
 *				:   1  			    - bad block
 *              :   -1              - error
 * Description	: 	check whether a block is bad
 */
int bsp_nand_isbad(const char *partition_name, loff_t partition_offset);

/**
 * Name			:	bsp_nand_blk_write_nv
 * Arguments	:	@partition_name  	- partition name
 * 					@partition_offset   - offset from partition to read from, must be block aligned.
 * 					@flag        	- flag value, 1 or 0
 * Return		:	0 				- success
 *				:   else  			- failure
 * Description	: 	read nv_flag of a block
 */
unsigned int bsp_nand_read_flag_nv(const char *partition_name, loff_t partition_offset, unsigned char *flag);

#else

#endif /* #ifdef __FASTBOOT__ */

/**
 * Name			:	find_partition_by_name
 * Arguments	:	@partition_name - partition name
 * Return		:	not NULL		- success
 *				:   NULL  			- failure
 * Desciption	:	NULL
 */
struct ST_PART_TBL * find_partition_by_name(const char *partition_name);

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */

#endif    /*  __BSP_NANDC_H__ */
