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


#include <linux/mtd/mtd.h>
#include <linux/err.h>
#include <linux/vmalloc.h>
#include <ptable_inc.h>
#include "nandc_balong.h"
/*lint --e{737,564,746,745,565,808,718,746}*/
#include "mbb_config.h" /*include mbb product config file*/
/**
 * Name			:	find_partition_by_name
 * Arguments	:	@partition_name - partition name
 * Return		:	not NULL		- success
 *				:   NULL  			- failure
 * Desciption	:	NULL
 */
struct ST_PART_TBL * find_partition_by_name(const char *partition_name)
{
    struct ST_PART_TBL * ptable = NULL;

    ptable_ensure();

    ptable = ptable_get_ram_data();

    /* find the partition */
    while(0 != strcmp(PTABLE_END_STR, ptable->name))
    {
        if(0 == strcmp(partition_name, ptable->name))
        {
            break;
        }
        ptable++;
    }

    if(0 == strcmp(PTABLE_END_STR, ptable->name))
    {
    	printk("ERROR: can't find partition %s, function %s\n", partition_name, __FUNCTION__);
        return NULL;
    }

    return ptable;

}

/**
* 作用:nandc模块按分区名的使能ECC功能的读数据操作,注意此函数的读操作不能读OOB数据
*
* 参数:
* @partition_name          		---要读的分区名
* @partition_offset      	    ---要读分区的相对偏移地址
* @ptr_ram_addr      	        ---读到内存的地址
* @length      	        		---读的数据长度
* @skip_len                     ---过滤的长度,当在读的过程中遇到坏块时跳过要读的块，此结构记录跳过的长度
*
*
* 描述:根据分区名和分区的偏移地址来确定Flash的地址,把读到的数据存放在ptr_ram_addr中去
*/
int bsp_nand_read(const char *partition_name, loff_t partition_offset, void* ptr_ram_addr, u32 length, u32 *skip_len)
{
	int ret = 0;
	u32 retlen = 0;
	u32 offsetinblock;
	u32 readsize;
	u32 remainder = 0;
	char * dst_addr = ptr_ram_addr;

    /* coverity[assign_zero] */
    struct mtd_info *mtd = NULL;

    if(skip_len)
    {
        *skip_len = 0;
    }

    /* check param */
	if((!partition_name) || (!ptr_ram_addr))
	{
		printk(KERN_ERR"param error.\n");
		goto erro;
	}

	/* get mtd device */
	mtd = get_mtd_device_nm(partition_name);
	if (IS_ERR(mtd))
    {
		printk(KERN_ERR"get_mtd_device_nm error\n");
		ret = PTR_ERR(mtd);
		goto erro;
	}

	while(length > 0)
	{
	    if(partition_offset >= mtd->size)
        {
            printk(KERN_ERR"ERROR: invalid partition offset 0x%lld!\n", partition_offset);
            return EINVAL;
        }

        /* bad block management */
        div_u64_rem(partition_offset, mtd->erasesize, &remainder);  /* 解决64位长除法问题 */
		ret = mtd_block_isbad(mtd, (long long)(partition_offset - remainder));
		if (ret)
		{
			printk(KERN_ERR"bad block detected, skip. partition offset is 0x%lld\n", partition_offset);
			goto skip;
		}

		/* read in block */
		offsetinblock = remainder;
        readsize = (offsetinblock + length < mtd->erasesize) ? length : (mtd->erasesize - offsetinblock);
        ret = mtd_read(mtd, (long long)partition_offset, readsize, &retlen, (unsigned char*)dst_addr);
        if(ret)
        {
            printk(KERN_ERR"mtd read error, ret = 0x%x.\n", ret);
            goto erro;
        }
        partition_offset += readsize;
        length         -= readsize;
        dst_addr       += readsize;
        continue;
skip:
        if(skip_len)
        {
            *skip_len    += mtd->erasesize;
        }
        partition_offset += mtd->erasesize;
	} /* while(length > 0) */

    /* release mtd device */
    if (!IS_ERR(mtd))
    {
        put_mtd_device(mtd);
    }

    return 0;

erro:

    /* release mtd device */
    if (!IS_ERR(mtd))
    {
        /* coverity[var_deref_model] */
        put_mtd_device(mtd);
    }

    return ret;
}
EXPORT_SYMBOL_GPL(bsp_nand_read);
#if (FEATURE_ON == MBB_COMMON)
/**
 * Name			:bsp_nand_mtd_partition_exist
 * Arguments	:	@partition_name  	- partition name
 * 					
 * Return		:	0 				- not exist
 *				:   else  			- exist
 * Desciption	:	NULL
 */
int bsp_nand_mtd_partition_exist(char *partition_name)
{
    struct mtd_info *mtd = NULL;
    mtd = get_mtd_device_nm(partition_name);
    if (IS_ERR(mtd))
    {
        printk(KERN_ERR "get_mtd_device_nm: %s error\n", partition_name);
        return 0;
    }
    return 1;
}
#endif
/**
* 作用:nandc模块按分区名和分区偏移地址来擦除flash操作
*
* 参数:
* @partition_name          		---要擦除的分区名
* @partition_offset      	    ---要擦除的相对偏移地址
*
*
* 描述:根据分区名和分区的偏移地址来确定Flash的地址,再来擦除一个block,注意是擦除一个block数据块
*/
int bsp_nand_erase(const char *partition_name, loff_t partition_offset)
{
	struct erase_info instr;
	u32 remainder = 0;
	int ret = 0;

    /* coverity[assign_zero] */
    struct mtd_info *mtd = NULL;

	if(!partition_name)
	{
		printk(KERN_ERR"param error.\n");
		goto out;
	}

	mtd = get_mtd_device_nm(partition_name);
	if (IS_ERR(mtd))
    {
		printk(KERN_ERR"get_mtd_device_nm error\n");
		ret = PTR_ERR(mtd);
		goto out;
	}

    div_u64_rem(partition_offset, mtd->erasesize, &remainder);
	instr.mtd = mtd;
	instr.addr = partition_offset - remainder;
	instr.len = mtd->erasesize;
	instr.time = 1000;
	instr.retries = 2;
	instr.callback = NULL;
	instr.priv = 0;

	ret = mtd_erase(mtd, &instr);
	if (0 != ret)
	{
		printk(KERN_ERR"mtd nand_erase error\n");
		goto out;
	}

    /* release mtd device */
    if (!IS_ERR(mtd))
    {
        put_mtd_device(mtd);
    }
    return ret;
out:

    /* release mtd device */
    if (!IS_ERR(mtd))
    {
        /* coverity[var_deref_model] */
        put_mtd_device(mtd);
    }

    return ret;
}
EXPORT_SYMBOL_GPL(bsp_nand_erase);
/**
* 作用:nandc模块按分区名和分区相对偏移来使能ECC功能的写数据操作,注意此函数的写操作带OOB数据且在写的过程中会擦除nandflash
*
* 参数:
* @partition_name          		---要写数据的分区名
* @partition_offset      	    ---要写数据的分区相对偏移地址
* @ptr_ram_addr          		---要写数据的地址
* @length      	    			---要写数据的长度
*
* 描述:根据分区名和分区的偏移地址来确定Flash的地址,再来写Flash操作
*/
s32 bsp_nand_write(const char *partition_name, loff_t partition_offset, void* ptr_ram_addr, u32 length)
{
    /* coverity[assign_zero] */
	long long addrblockalign;
	struct erase_info instr;
    struct mtd_info *mtd = NULL;
	int ret = 0;
	u32 retlen = 0;
	u32 offsetinblock;
	u32 writesize;
	u32 remainder = 0;

	unsigned char *ram_addr = ptr_ram_addr;
	unsigned char *buffer = NULL;
    unsigned char *tmp_buf = NULL;

    /* check param */
	if((!partition_name) || (!ptr_ram_addr))
	{
		printk(KERN_ERR"param error.\n");
		goto erro;
	}

	/* get mtd device */
	mtd = get_mtd_device_nm(partition_name);
	if (IS_ERR(mtd))
    {
		printk(KERN_ERR"get_mtd_device_nm error\n");
		ret = PTR_ERR(mtd);
		goto erro;
	}

	while(length > 0)
	{
	    if(partition_offset >= mtd->size)
        {
            printk(KERN_ERR"ERROR: invalid partition offset 0x%lld!\n", partition_offset);
            return EINVAL;
        }

		/* quary whether bad block */
        div_u64_rem(partition_offset, mtd->erasesize, &remainder);
		ret = mtd_block_isbad(mtd, (long long)(partition_offset - remainder));
		if (ret)
		{
		    /* bad block, skip */
			printk(KERN_ERR"bad block detected, skip. partition offset is 0x%lld\n", partition_offset);
            goto skip;
		}
        /* coverity[overflow_before_widen] */
		addrblockalign = (long long)(div_u64(partition_offset, mtd->erasesize) * mtd->erasesize); /*lint !e647*/

        /* not a whole block, need flash read and ddr copy */
        if((addrblockalign != partition_offset) || (length < mtd->erasesize))
        {
            /* get ram buffer */
            /* coverity[alloc_fn] */
        	/*在A核的后面没有连续的128K的内存来分配,因此用vmalloc来分配内存*/
        	tmp_buf = (unsigned char *)vmalloc(mtd->erasesize);
        	if(!tmp_buf)
        	{
        		printk(KERN_ERR"get ram buffer failed!\n");
        		goto erro;
        	}

    		/* read block into DDR */
    		ret = mtd_read(mtd, addrblockalign, mtd->erasesize, &retlen, (unsigned char*)tmp_buf);
    		if(ret)
            {
                ret = mtd_block_markbad(mtd, addrblockalign);
                if(ret)
            	{
            		printk(KERN_ERR"mtd mark block bad failed as reading!\n");
            		goto erro;
            	}
                /* bad block, skip */
                goto skip;
            }

            /* copy data to buffer */
    		offsetinblock = remainder;
            writesize     = (offsetinblock + length < mtd->erasesize) ? length : (mtd->erasesize - offsetinblock);
            memcpy(tmp_buf + offsetinblock, ram_addr, writesize);
            buffer = tmp_buf;
        }
        else
        {
            writesize = mtd->erasesize;
            buffer = ram_addr;
        }

		/* erase a block */
		instr.mtd      = mtd;
		instr.addr     = (unsigned long long)addrblockalign;
		instr.len      = mtd->erasesize;
		instr.time     = 1000;
		instr.retries  = 2;
		instr.callback = NULL;
		instr.priv     = 0;

		ret = mtd_erase(mtd, &instr);
		if(ret)
        {
            /* erase fail, mark bad */
            ret = mtd_block_markbad(mtd, (long long)instr.addr);
            if(ret)
        	{
        		printk(KERN_ERR"mtd mark block bad failed as erasing!\n");
        		goto erro;
        	}
            /* bad block, skip */
            goto skip;
        }

        /* write data to flash */
        /* coverity[noescape] */
        ret = mtd_write(mtd, addrblockalign, mtd->erasesize, &retlen, (unsigned char*)buffer);
        if(ret)
        {
            /* write failed, mark bad */
            ret = mtd_block_markbad(mtd, addrblockalign);
            if(ret)
        	{
        		printk(KERN_ERR"mtd mark block bad failed as writing!\n");
        		goto erro;
        	}
            goto skip;
        }

        partition_offset += writesize;
        length         -= writesize;
        ram_addr       += writesize;
        continue;
skip:
        partition_offset += mtd->erasesize;
    } /* while(length > 0) */

    if(tmp_buf)
    {
        vfree(tmp_buf);
    }

    /* release mtd device */
    if (!IS_ERR(mtd))
    {
        /* coverity[var_deref_model] */
        put_mtd_device(mtd);
    }

    return 0;

erro:
    if(tmp_buf)
    {
        vfree(tmp_buf);
    }

    /* release mtd device */
    if (!IS_ERR(mtd))
    {
        /* coverity[var_deref_model] */
        put_mtd_device(mtd);
    }

	return ret;
}
EXPORT_SYMBOL_GPL(bsp_nand_write);
/*提供给NV模块,用来更新nv dload分区*/
/**
* 作用:nandc模块写dload分区相对偏移来使能ECC功能的写数据操作
*
* 参数:
* @partition_name          		---要写数据的分区名
* @partition_offset      	    ---要写数据的分区相对偏移地址
* @ptr_ram_addr          		---要写数据的地址
* @length      	    			---要写数据的长度
*
* 描述:根据分区名和分区的偏移地址来确定Flash的地址,再来写Flash操作
*/
s32 bsp_nand_write_dload(const char *partition_name, u32 partition_offset, void* ptr_ram_addr, u32 length)
{
    struct erase_info instr;
    struct mtd_info *mtd =(struct mtd_info * )-1; /*注意这个初值必须为-1*/
    int ret = 0;
    unsigned int ret_len = 0;
    /*对nv dload分区进行先全擦除,然后在更新length*/
    unsigned int partition_offset_use = partition_offset;
    unsigned int length_use = length;

    unsigned char *ram_addr = ptr_ram_addr;
    unsigned char *buffer = NULL;
    unsigned char *tmp_buf = NULL;

    /* check param */
    if((!partition_name) || (!ptr_ram_addr))
    {
        printk(KERN_ERR "%s:param error.\n", __func__);
        goto erro;
    }

    /* get mtd device */
    mtd = get_mtd_device_nm(partition_name);
    if (IS_ERR(mtd))
    {
        printk(KERN_ERR"get_mtd_device_nm error\n");
        ret = PTR_ERR(mtd);
        goto erro;
    }

    /*检查写开始地址是否是blk对齐的*/
    ret_len = partition_offset % mtd->erasesize;
    if( 0 != ret_len )
    {
        printk(KERN_ERR "%s:partition_offset is 0x%08x.\n", __func__, partition_offset);
        goto erro;
    }

    /*erase first:擦除整个分区*/
    while( partition_offset_use < ( mtd->size - 1 ) )
    {
        /* quary whether bad block */
        ret = mtd_block_isbad( mtd, (long long)(partition_offset_use) );
        if (ret)
        {
            /* bad block, skip */
            printk( KERN_ERR "%s: bad block skip. partition_offset_use is: 0x%x\n",
                __func__, partition_offset_use );
            goto erase_skip;
        }

        /* erase a block */
        instr.mtd      = mtd;
        instr.addr     = (unsigned long long)partition_offset_use;
        instr.len      = mtd->erasesize;
        instr.time     = 1000; /*时间*/
        instr.retries  = 2;    /*重试次数*/
        instr.callback = NULL;
        instr.priv     = 0;

        ret = mtd_erase(mtd, &instr);
        if(ret)
        {
            /* erase fail*/
            printk( KERN_ERR "%s: mtd erase fail, offset: 0x%x\n",
                __func__, partition_offset_use );

            /* erase fail, mark bad */
            ret = mtd_block_markbad(mtd, (long long)instr.addr);
            if(ret)
            {
                printk(KERN_ERR"mtd mark block bad failed as erasing!\n");
                goto erro;
            }
        }
erase_skip:
    partition_offset_use += mtd->erasesize;
    }

    partition_offset_use = partition_offset;

    /*then write*/
    while( length_use > 0 )
    {
        if( partition_offset_use >= mtd->size )
        {
            printk(KERN_ERR"ERROR: invalid partition offset 0x%x!\n",
                partition_offset_use);
            goto erro;
        }

        /* quary whether bad block */
        ret = mtd_block_isbad( mtd, ( long long )( partition_offset_use ) );
        if (ret)
        {
            /* bad block, skip */
            printk( KERN_ERR "bad block detected, skip. partition offset is 0x%x\n",
            partition_offset_use );
            goto write_skip;
        }

        if( length_use < mtd->erasesize )
        {
            /* get ram buffer, 并初始化为全0 */
            if ( NULL == tmp_buf )
            {
                tmp_buf = (unsigned char *)vmalloc(mtd->erasesize);
            }

            if( NULL == tmp_buf )
            {
                printk( KERN_ERR "%s: malloc tem_buf failed!\n", __func__ );
                goto erro;
            }

            memset( (void *)tmp_buf, 0, mtd->erasesize );

            /* copy data to buffer */
            memcpy( (void *)tmp_buf, ( const void * )ram_addr, length_use);
            buffer = tmp_buf;
        }
        else
        {
            buffer = ram_addr;
        }

        /* write data to flash */
        /* coverity[noescape] */
        ret = mtd_write(mtd, (long long)partition_offset_use, mtd->erasesize, &ret_len, (unsigned char*)buffer);
        if(ret)
        {
            /*write fail*/
            printk( KERN_ERR "%s: mtd write fail, offset: 0x%x\n",
                __func__, partition_offset_use );

            /* write failed, mark bad */
            ret = mtd_block_markbad( mtd, (long long)partition_offset_use );
            if(ret)
            {
                /*写的过程中出现异常,并且标坏块失败*/
                printk( KERN_ERR "bad! mtd mark block bad failed as writing!\n");
                goto erro;
            }
            else
            {
                /*写失败,但是标坏块成功*/
                goto write_skip;
            }
        }

        if ( length_use < mtd->erasesize )
        {
            length_use = 0;
        }
        else
        {
            length_use -= mtd->erasesize;
        }
        ram_addr += mtd->erasesize;
        /*continue;*/

write_skip:
        /*跳过坏块*/
        partition_offset_use += mtd->erasesize;
        continue;
    } /* while(length > 0) */

erro:
    if(tmp_buf)
    {
        vfree(tmp_buf);
    }

    /* release mtd device */
    if (!IS_ERR(mtd))
    {
        /* coverity[var_deref_model] */
        put_mtd_device(mtd);
    }

    return ret;
}
EXPORT_SYMBOL_GPL(bsp_nand_write_dload);

/**
* 作用:nandc模块按分区名和分区相对偏移来确定此块是不是坏块
*
* 参数:
* @partition_name          		---查询数据块的分区名
* @partition_offset      	    ---查询数据块的分区相对偏移地址
*
* 描述:根据分区名和分区的偏移地址来确定Flash的地址,再来判断此block是不是坏块
返回值:
* 0表示此块是好块
* 1表示是坏块
* 其他的表示有错误
*/
int bsp_nand_isbad(const char *partition_name, loff_t partition_offset)
{
	int ret = -1;
	struct mtd_info *mtd = get_mtd_device_nm(partition_name);
	if (IS_ERR(mtd))
    {
		printk(KERN_ERR"get_mtd_device_nm error\n");
		ret = PTR_ERR(mtd);
		goto erro;
	}

	ret = mtd_block_isbad(mtd, (long long)partition_offset);

    /* release mtd device */
    if (!IS_ERR(mtd))
    {
        put_mtd_device(mtd);
    }

    return ret;

erro:
    /*lint -save -e438 */
    /* release mtd device */
    if (!IS_ERR(mtd))
    {
        /* coverity[var_deref_model] */
        put_mtd_device(mtd);
    }
    return ret;
    /*lint -restore*/
}
EXPORT_SYMBOL_GPL(bsp_nand_isbad);

