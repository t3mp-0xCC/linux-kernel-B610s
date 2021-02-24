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



#ifdef __cplusplus
extern "C"
{
#endif
/*lint --e{537}*/
/*#define MODULE_MAIN          NAND*/
#define SUB_MODULE              NANDC_TRACE_PTABLE

#if defined(__FASTBOOT__)
typedef unsigned char       BSP_U8;
#include "config.h"
extern struct nandc_host*  nandc_nand_host;

#elif defined(__KERNEL__)
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/mtd/partitions.h>
#define PTABLE_PROC_FILE_NAME "ptable_bin"
#endif
#include "nandc_port.h"
#include "ptable_com.h"
#include "ptable_inc.h"
#include "bsp_nandc.h"
#include "mbb_config.h" /*����MBB��Ʒ�����ļ�*/
#include <bsp_shared_ddr.h>


extern struct ST_PART_TBL ptable_product[PTABLE_PARTITION_MAX];

/**********************************************************************************
 * FUNC NAME:
 * ptable_get_ram_head() - external API: for get flash table ram address
 *
 * PARAMETER:
 * @none
 *
 * DESCRIPTION:
 * this function get flash table address WITH TABLE HEAD, whitch description flash useage
 *      information
 *
 * CALL FUNC:
 *
 *********************************************************************************/
struct ST_PTABLE_HEAD * ptable_get_ram_head(void)
{

    return (struct ST_PTABLE_HEAD *)(SHM_MEM_PTABLE_ADDR);
}

/**********************************************************************************
 * FUNC NAME:
 * nand_get_spec() - external API: for get flash table ram address
 *
 * PARAMETER:
 * @none
 *
 * DESCRIPTION:
 * this function get flash table address WITHOUT TABLE HEAD,
 *    whitch description flash useage information.
 *
 * CALL FUNC:
 *
 *********************************************************************************/
struct ST_PART_TBL * ptable_get_ram_data(void)
{
    return (struct ST_PART_TBL *)((u32)(SHM_MEM_PTABLE_ADDR) + PTABLE_HEAD_SIZE) ;
}
#ifdef __FASTBOOT__
/**********************************************************************************
 * FUNC NAME:
 * ptable_adapt()
 *
 * PARAMETER:
 * @none
 *
 * DESCRIPTION:
 *    ��nand��ʼ����ʱ�򣬶�̬�������������һ������(cdromiso)�Ĵ�С��ʹnand���Ŀռ�ȫ�����������
 *    ����ӿ�ֻ���ڲ�ʹ�ã��������ṩ
 *
 *
 * CALL FUNC:
 *
 *********************************************************************************/
void ptable_adapt(void)
{
    struct ST_PART_TBL * ptable = ptable_get_ram_data();
	struct nandc_host *host = nandc_nand_host;
    FSZ chipsize;

    if(!host->nandchip)
    {
        cprintf("ERROR: pointer null\n");
        goto OUT;
    }

#if (FEATURE_ON == MBB_COMMON)
    /*����˼��cdrom������ĳ����һ������������*/
    while(0 != strcmp(PTABLE_END_STR, ptable->name))
    {
        ptable++;
    }
    ptable--;
#else
    while(0 != strcmp(PTABLE_END_STR, ptable->name))
    {
        if(0 == strcmp(PTABLE_CDROMISO_NM, ptable->name))
        {
            break;
        }
        ptable++;
    }

    if(0 == strcmp(PTABLE_END_STR, ptable->name))
    {
        cprintf("ERROR: can not find cdromiso partition null\n");
        goto OUT;
    }
#endif
    chipsize = host->nandchip->spec.chipsize;
    if(chipsize >= ptable->offset)
    {
        ptable->capacity = chipsize - ptable->offset;
    }

OUT:
    return;
}

#else /* __KERNEL__ */
/**********************************************************************************
 * FUNC NAME:
 * ptable_adapt()
 *
 * PARAMETER:
 * @none
 *
 * DESCRIPTION:
 *
 *
 *
 * CALL FUNC:
 *
 *********************************************************************************/
void ptable_adapt(void)
{
    /* A��nand��ʼ����ʱ�򣬷������Ѿ��ڹ����ڴ��У��˴����������� */
}
#endif
/**********************************************************************************
 * FUNC NAME:
 * ptable_ensure() - external API: make sure there is a ptable to be use
 *
 * PARAMETER:
 * @none
 *
 * DESCRIPTION:
 *    check the flash ram table, and use symbol table when ram table not exist
 *
 *
 * CALL FUNC:
 *
 *********************************************************************************/
u32 ptable_ensure(void)
{
	struct ST_PTABLE_HEAD *phead = ptable_get_ram_head();
    struct ST_PART_TBL * ram_parts = ptable_get_ram_data();
    u32 copy_addr;

    if(0 != strcmp(PTABLE_HEAD_STR, phead->head_str))
    {
        hiout(("partition tabel: 0x"));
        BSPLOGU32((u32)phead);
        hiout((" not find, use default\r\n"));

        memset((void *)phead, 0x00, SHM_MEM_PTABLE_SIZE);

        memcpy((void *)phead, (void *)&ptable_head, sizeof(struct ST_PTABLE_HEAD));

        memcpy((void *)ram_parts, ptable_product, sizeof(ptable_product));

        /*skip table*/
        copy_addr = (u32)ram_parts + sizeof(ptable_product);

        /*set end string*/
        strncpy((char*)copy_addr, PTABLE_TAIL_STR, (unsigned long)strlen(PTABLE_TAIL_STR));
    }

    return NAND_OK;
}

/**********************************************************************************
 * FUNC NAME:
 * ptable_show() - external API: show flash partition table in shell
 *
 * PARAMETER:
 * @part -[input] if "part" is not NULL, show flash partition info in "part",else
 *                get default flash partition info and show.
 *
 * DESCRIPTION:
 *     show flash partition table in shell
 *
 * CALL FUNC:
 *
 *********************************************************************************/
u32 ptable_show(struct ST_PART_TBL *part)
{
    int number = 1;
	struct ST_PTABLE_HEAD *phead = ptable_get_ram_head();

    BSPLOGSTR("Partition Table list(HEX):");
	/*������İ汾��ƫ��*/
    BSPLOGSTR((u8 *)phead->product_ver_id);
    BSPLOGSTR((u8 *)phead->ver_name);
    BSPLOGSTR("\r\n");
    BSPLOGSTR("NO. |offset    |loadsize  |capacity  |loadaddr  |entry     |property   |count    |id         |name     |\r\n");
    BSPLOGSTR("-------------------------------------------------------------------------------------------------------\r\n");

    /*skip head*/
    part = ptable_get_ram_data();

    while(0 != strcmp(PTABLE_END_STR, part->name))
    {
        BSPLOGU8(number);
        BSPLOGSTR(":  ");
        BSPLOGU32(part->offset);
        BSPLOGSTR("  ,");
        BSPLOGU32(part->loadsize);
        BSPLOGSTR("  ,");
        BSPLOGU32(part->capacity);
        BSPLOGSTR("  ,");
        BSPLOGU32(part->loadaddr);
        BSPLOGSTR("  ,");
        BSPLOGU32(part->entry);
        BSPLOGSTR("  ,");
        BSPLOGU32(part->property);
        BSPLOGSTR("  ,");
		BSPLOGU32(part->count);
        BSPLOGSTR("  ,");
        BSPLOGU32(part->image);
        BSPLOGSTR("  ,");
        BSPLOGSTR(part->name);
        BSPLOGSTR("\r\n");
        part++;
        number++;
    }
    BSPLOGSTR("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\r\n");

    return 0;
}

/**********************************************************************************
 * FUNC NAME:
 * ptable_parse_mtd_partitions() - external API: get mtd partitions from flash table
 *
 * PARAMETER:
 * @mtd_parts -[output] pointer to mtd partitions
 * @nr_parts - [output] number of mtd partitions
 *
 *
 * DESCRIPTION:
 *     get mtd partitions from flash table
 *
 * CALL FUNC:
 *
 *********************************************************************************/
u32 ptable_parse_mtd_partitions(struct mtd_partition** mtd_parts, u32 *nr_parts)
{
    struct ST_PART_TBL * ptable = NULL;
    struct mtd_partition *parts = NULL;
    u32 npart;
    u32 last_end = 0;
    u32 last_id = 0;

	if(!(mtd_parts && nr_parts))
	{
		goto ERRO;
	}

    /*ensure that there's a ram table exist*/
    /*lint -save -e539*/
    /*lint -restore*/

    ptable_adapt();

#ifdef __FASTBOOT__
    /*show all ptable info*/
#ifndef BSP_CONFIG_EDA
    ptable_show(NULL);
#endif
#endif
   	ptable = ptable_get_ram_data();

    /*get number of partitions*/
    npart = 0;

    while(0 != strcmp(PTABLE_END_STR, ptable->name))
    {
        if(ptable->capacity)    /* skip empty part */
        {
            npart++;
        }

        ptable++;
    }

    /*this memory never free */
#if (FEATURE_ON == MBB_COMMON)
    parts = himalloc(sizeof(struct mtd_partition) * MAX_PARTITIONS_NUM);
    if(NULL == parts)
    {
        hiout(("ptable_to_mtd_partition, error malloc\r\n"));
        goto ERRO;
    }

    memset(parts, 0x00 , (sizeof(struct mtd_partition) * MAX_PARTITIONS_NUM));

    /*lint -e613*/
    *mtd_parts = parts;
    /*lint +e613*/
    *nr_parts  = MAX_PARTITIONS_NUM;
#else
    parts = himalloc(sizeof(struct mtd_partition) * npart);
    if(NULL == parts)
    {
        hiout(("ptable_to_mtd_partition, error malloc\r\n"));
        goto ERRO;
    }

    memset(parts, 0x00 , (sizeof(struct mtd_partition) * npart));

    /*lint -save -e613*/
    *mtd_parts = parts;
    /*lint -restore*/
    *nr_parts  = npart;
#endif
    /*get address again*/
    ptable = ptable_get_ram_data();

    /*��0�Ժ����¼���ʵ��ת�Ƶ�MTD�ķ�����*/
    npart = 0;

    /*form flash table to mtd partitions */
    while(0 != strcmp(PTABLE_END_STR, ptable->name))
    {
        /*not first partition (PTABLE_HEAD_STR)*/
        if((last_end != ptable->offset) && (0 != last_id))
        {
            hiout(("ptable check error! "));
            hiout((ptable->name));
            hiout(("\r\n"));
            goto ERRO;
        }

        #ifdef __KERNEL__
        if(ptable->property & MOUNT_MTD){
        #endif
        parts->name   = ptable->name;
        parts->offset = ptable->offset;
        parts->size   = ptable->capacity;
        parts++;
        npart++;
        #ifdef __KERNEL__
        }
        #endif

        last_end = ptable->offset + ptable->capacity;
        last_id  = ptable->image;

        ptable++;
    }
#if (FEATURE_ON == MBB_COMMON)
    /*�������Ӽ����յķ�������Ҫ�������������У�û�ж���������������������ʱ�����ܹ��ҵ���Ӧ�������ڵ㣬�������ʧ�ܹ���*/
    while(npart < MAX_PARTITIONS_NUM)
    {
        parts->name   = "nullMTD";
        parts->offset = 0;
        parts->size   = -1; /*���ó�-1��ʾ�÷�����С��0*/
        parts++;
        npart++;
    }
    *nr_parts  = MAX_PARTITIONS_NUM;
#else
    *nr_parts  = npart;
#endif
  
  
    return NANDC_OK;

ERRO:
    if(NULL != parts)
    {
        hifree(parts);
    }

    return NANDC_ERROR;
}

/****************************************************************************************
 * FUNC NAME:
 * ptable_find_by_type() - external API: find one partition address in flash partition
 *                           table
 *
 * PARAMETER:
 * @part_type -[input] which type of partition to find
 * @ptable    -[input] form which partition table to find
 *
 *
 * DESCRIPTION:
 *     find partition description by type���ҵ����µķ���(��Ч)���ڼ��غͶ�ȡ
 *
 * CALL FUNC:
 *
 ***************************************************************************************/
struct ST_PART_TBL * ptable_find_by_type(u32 type_idx, struct ST_PART_TBL * ptable)
{
    struct ST_PART_TBL * the_newer = NULL; /*�������µķ���*/

    if(NULL == ptable)
    {
        goto EXIT;
    }

    while(0 != strcmp(PTABLE_END_STR, ptable->name))
    {
        if(type_idx == ptable->image)
        {
            if(NULL == the_newer)
            {
                if(DATA_VALID == ptable_get_validity(ptable))
                {
                    the_newer = ptable;
                }
            }
            else
            {
                if(DATA_VALID == ptable_get_validity(ptable))
                {
                    /*������µķ���*/
                    the_newer = (the_newer->count < ptable->count) ? ptable : the_newer ;
                }
            }
        }

        ptable++;
    }

EXIT:
    return the_newer;

}

/****************************************************************************************
 * FUNC NAME:
 * ptable_get_validity() - external API:  find if the  partition data's validity
 *
 *
 * PARAMETER:
 * @part_type -[input] which type of partition to find
 * @ptable    -[input] form which partition table to find
 *
 *
 * DESCRIPTION:
 *     ��÷��������ݵ���Ч��
 *
 * CALL FUNC:
 *
 ***************************************************************************************/
u32 ptable_get_validity(struct ST_PART_TBL * ptable)
{
    return NANDC_REG_GETBIT32(&(ptable->property), PTABLE_VALIDITY_OFFSET, PTABLE_VALIDITY_MASK);
}

/****************************************************************************************
 * FUNC NAME:
 * ptable_find_by_type() - external API: find one partition address in flash partition
 *                           table
 *
 * PARAMETER:
 * @name_to_find -[input] which name of partition to find
 * @ptable    -[input] form which partition table to find
 *
 *
 * DESCRIPTION:
 *     find partition description by name,�ҵ����µķ���,��ʶ�������Ч��
 *
 * CALL FUNC:
 *
 ***************************************************************************************/
struct ST_PART_TBL * ptable_find_by_name(char* name_to_find, struct ST_PART_TBL * ptable)
{
    struct ST_PART_TBL * the_newer = NULL; /*�������µķ���*/

    if(NULL == ptable)
    {
        goto EXIT;
    }

    while(0 != strcmp(PTABLE_END_STR, ptable->name))
    {
        if(0 == strcmp(name_to_find, ptable->name))
        {
            if(NULL == the_newer)
            {
                the_newer = ptable;
            }
            else
            {
                /*������µķ���*/
                the_newer = (the_newer->count < ptable->count) ? ptable : the_newer ;
            }
        }

        ptable++;
    }

EXIT:

    return the_newer;

}

#ifdef __KERNEL__
#if (FEATURE_ON == MBB_COMMON)
#define MTDBLK_BASE_NAME "/dev/block/mtdblock"
u32 ptable_get_cdromiso_capacity(void)
{
    struct ST_PART_TBL *ptable = NULL;

    ptable = ptable_get_ram_data();
    while(0 != strcmp(PTABLE_END_STR, ptable->name))
    {
        if(ptable->capacity)
        {
            if(0 == strcmp(PTABLE_CDROMISO_NM, ptable->name))
            {
                return ptable->capacity;
            }
        }
        ptable++;
    }

    return NAND_ERROR;
}
#endif
/*��ȡCDROM MTD����*/
u32 ptable_get_cdromiso_mtdname(char * ptab_name, int len)
{
#if (FEATURE_OFF == MBB_COMMON)
#define MTDBLK_BASE_NAME "/dev/block/mtdblock"
#endif
    struct ST_PART_TBL *ptable = NULL;
    u32 cnt = 0;
    char str[64] = {0};

    if(NULL == ptab_name)
    {
        return NAND_ERROR;
    }

    ptable = ptable_get_ram_data();
    while(0 != strcmp(PTABLE_END_STR, ptable->name))
    {
        if(ptable->capacity)
        {
            if(0 == strcmp(PTABLE_CDROMISO_NM, ptable->name))
            {
                /* coverity[secure_coding] */
                sprintf(str,"%s%d",MTDBLK_BASE_NAME,cnt);
                strncpy(ptab_name, str, strlen(str));
                ptab_name[strlen(str)]=0;
                return NAND_OK;
            }

            cnt++;
        }

        ptable++;
    }

    return NAND_ERROR;
}

#endif

#ifdef __KERNEL__

static struct proc_dir_entry *ptable_proc_file=NULL;

static int ptable_proc_show(struct seq_file *sfile,void *v)
{
	struct ST_PTABLE_HEAD *phead;
	struct ST_PART_TBL *ptab_product;

	phead = ptable_get_ram_head();

	seq_printf(sfile,"%x %s %s\n",
				phead->property,
				phead->product_ver_id,
				phead->ver_name);
	seq_puts(sfile,"image    loadsize capacity loadaddr entry    offset property count   name \n");

	ptab_product=ptable_get_ram_data();

	while(0 != strcmp(PTABLE_END_STR, ptab_product->name))
	{
		seq_printf(sfile,"%8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x \"%s\"\n",
		ptab_product->image,
		ptab_product->loadsize,
		ptab_product->capacity,
		ptab_product->loadaddr,
		ptab_product->entry,
		ptab_product->offset,
		ptab_product->property,
		ptab_product->count,
		ptab_product->name);

		ptab_product++;
	}
	return 0;
}
static int ptable_proc_open(struct inode *inode,struct file *file)
{
	return single_open(file,ptable_proc_show,NULL);
}

static struct file_operations ptable_proc_ops={
	.open=ptable_proc_open,
	.read=seq_read,
	.llseek=seq_lseek,
	.release=seq_release
};

static int __init ptable_proc_file_init(void)
{
	struct proc_dir_entry *p;
	char *proc_file_name = PTABLE_PROC_FILE_NAME;

	p=proc_mkdir("ptable",NULL);
	if(!p){
		printk("ERROR: can't create /proc/ptable\n");
	}

	ptable_proc_file=proc_create(proc_file_name, S_IRUGO | S_IFREG,p,&ptable_proc_ops);
	if(NULL==ptable_proc_file)
	{
		remove_proc_entry(proc_file_name, NULL);
		printk("Error:Could not initialize /proc/%s\n",proc_file_name);
		return -1;
	}

	return 0;
}

static void __exit ptable_proc_file_exit(void)
{
	if(ptable_proc_file){
		remove_proc_entry(PTABLE_PROC_FILE_NAME, NULL);
		remove_proc_entry("ptable",NULL);
	}
}
module_init(ptable_proc_file_init); /*lint !e19 */
module_exit(ptable_proc_file_exit); /*lint !e19 */


#endif

#ifdef __cplusplus
}
#endif
