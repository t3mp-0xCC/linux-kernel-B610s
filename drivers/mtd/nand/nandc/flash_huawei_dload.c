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


#include <linux/mtd/flash_huawei_dload.h>
#include <linux/kernel.h>
#include <linux/mtd/mtd.h>
#include <linux/slab.h>
#if (FEATURE_ON == MBB_DLOAD)
#include "ptable_com.h"
#include "ptable_inc.h"
#include "nandc_balong.h"
#include "bsp_version.h"
#if (FEATURE_ON == MBB_DLOAD_RECORD_BAD)
#include "bsp_nandc.h"
#endif

/*---------------------------------------------------------------------------
  Macro define region
---------------------------------------------------------------------------*/
#define    MBB_DLOAD_OEMINFO_PARTI_NAME     "oeminfo"
#define    HUAWEI_DLOAD_XML_MAGIC_NUM        0x454D5045
#define    HUAWEI_RESTORE_XML_MAGIC_NUM    0x45504D45

#define    TRUE     1
#define    FALSE   0
/*---------------------------------------------------------------------------
  Cust type define region
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
  Global Variable define region
---------------------------------------------------------------------------*/
#if (FEATURE_ON == MBB_DLOAD_RECORD_BAD)
static bad_block_info_s bb_info;
#endif


/*---------------------------------------------------------------------------
  Local Function decleare region
---------------------------------------------------------------------------*/

uint8 iso_header_buffer[MAX_PAGE_SIZE];

/*---------------------------------------------------------------------------
  Global table define region
---------------------------------------------------------------------------*/
child_region_t  child_region[] =
{
    /* SHARE子分区参数 */
    {MBB_DLOAD_OEMINFO_PARTI_NAME, MAGIC_DEFAULT,MAGIC_DEFAULT, \
     0, 4, NULL, 0},  /*0：起始block  4 总block 数*/

};

/*===========================================================================

                   FUNCTION DECLARATIONS

===========================================================================*/

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
=========================================================================== */

boolean flash_get_share_region_info(rgn_pos_e_type region_type,void* data,uint32 data_len)
{
    int ret = -1;
    unsigned int     page_index       = 0;
    unsigned int     blk_size             = 0;
    unsigned int     blk_count          = 0;
    unsigned int     blk_index          = INVALID_BLOCK_ID;
    rgn_hd_type* rgn_hd = NULL;
    unsigned int     offset                 = 0;
    unsigned int     start_offset       = 0;
    unsigned int     page_size          = 0;
    uint8*     read_buf          = NULL;
    int           retlength            = 0;
    struct mtd_info *p_mtd = NULL;
    oper_region_struct_t opregion = {0};

    if ((region_type >= RGN_MAX_NUM) || (NULL == data) || (0 == data_len))
    {
        printk(KERN_ERR "flash_get_share_region_info input para error \n");
        return FALSE;
    }

    ret = flash_nand_oper_region_init(SHARE_REGION, &p_mtd, &opregion);
    if ((0 != ret) || (NULL == p_mtd) || (NULL == opregion.buffer))
    {
        printk(KERN_ERR "flash_get_share_region_info init fail  \n");
        flash_nand_oper_region_close(p_mtd, &opregion);
        return FALSE;
    }
    
    /*Get Partition and Device Info*/
    blk_count      = child_region[SHARE_REGION].length;
    blk_size        = opregion.block_size;
    start_offset  = opregion.start_addr;
    page_size    = opregion.page_size;
    read_buf      = opregion.buffer;
    
    for (blk_index = 0; blk_index < blk_count; blk_index++)
    {
        offset = start_offset + blk_size * blk_index;
        /*If the blk is bad blk or it is erased*/
        if (mtd_block_isbad(p_mtd, offset))
        {
            printk(KERN_DEBUG "flash_get_share_region_info Block ID = 0x%x is bad block \n",
                   (offset / blk_size));
            continue;
        }

        /*If current blk is not erased, then read the first page and check its magic*/
        /*lint -e64*/
        if (!mtd_read(p_mtd, offset, page_size, &retlength, (unsigned char*)read_buf))
        /*lint +e64*/
        {
           /*Firstly we need to check the Share Region Info,If it was a valid block
            then read corresponding page*/
            rgn_hd = (rgn_hd_type*)read_buf;
            if(SHARE_RGN_MAGIC == rgn_hd->magic)
            {
                /*Read Spec Region Flag Info*/
                page_index += region_type;
                offset = offset + page_index * page_size;

                memset((void *)read_buf, NAND_FILL_CHAR_APP, page_size);
                /*lint -e64*/
                if(!mtd_read(p_mtd, offset, page_size, &retlength,
                   (unsigned char*)read_buf))
                /*lint +e64*/
                {
                    memcpy(data,read_buf,data_len);
                    flash_nand_oper_region_close(p_mtd, &opregion);
                    return TRUE;
                }  
                printk(KERN_DEBUG "flash_get_share_region_info fail  \n");
            }
        }
    }
    printk(KERN_ERR "flash_get_share_region_info error  \n");

    flash_nand_oper_region_close(p_mtd, &opregion);
    return FALSE;
}

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
#if (FEATURE_OFF == MBB_DLOAD_OEM_PART_IM)
boolean flash_update_share_region_info(rgn_pos_e_type region_type, void* data, size_t data_len)
{
    int ret = -1;
    unsigned int    block_index        = 0;
    unsigned int    start_index         = 0;
    unsigned int    blk_size              = 0;
    unsigned int    blk_count            = 0;
    unsigned int    max_rgn_num    = 0;
    unsigned int    dest_block          = 0;
    unsigned int    current_block     = INVALID_BLOCK_ID;
    unsigned int    current_page      = INVALID_PAGE_ID;
    unsigned int    dest_page           = INVALID_PAGE_ID;
    rgn_hd_type* rgn_hd       = NULL;
    unsigned int   offset                    = 0;
    unsigned int   page_size             = 0;
    uint8 *   cache_buf            = NULL;
    unsigned int   start_offset           = 0;
    struct   erase_info instr;
    int retlength = 0;
    rgn_hd_type rgn_hd_1 = {0};
    struct mtd_info *p_mtd = NULL;
    oper_region_struct_t opregion = {0};

    /*入参条件检测*/
    if (region_type >= RGN_MAX_NUM || NULL == data)
    {
        printk(KERN_ERR "Input parm error \n");
        return FALSE;
    }

    ret = flash_nand_oper_region_init(SHARE_REGION, &p_mtd, &opregion);
    if((0 != ret) || (NULL == p_mtd) || (NULL == opregion.buffer))
    {
        printk(KERN_ERR "flash_update_share_region_info init fail  \n");
        flash_nand_oper_region_close(p_mtd, &opregion);
        return FALSE;
    }

    /*Get Partition and Device Info*/
    blk_count    = child_region[SHARE_REGION].length;
    blk_size       = opregion.block_size;
    start_offset = opregion.start_addr;
    page_size   = opregion.page_size;
    cache_buf    = opregion.buffer;
    
    instr.mtd = p_mtd;
    instr.len = p_mtd->erasesize;
    instr.callback = NULL;
    instr.priv = 0;

    if (data_len > page_size)
    {
        printk(KERN_ERR "Warning: data is truncated.\n");
        data_len = page_size;
    }
    
    if ((0 == blk_count) || (0 == blk_size))
    {
        printk(KERN_ERR "blk_count or blk_size is zero, exit\n");
        goto FalseQuit;
    }

    /*instr.addr*/
    for (block_index = 0; block_index < blk_count; block_index++)
    {
        offset = start_offset + blk_size * block_index;
        
        /*If the blk is bad blk or it is erase */
        if (0 != mtd_block_isbad(p_mtd, offset))
        {
            printk(KERN_DEBUG "Find Current Block ID = 0x%x is bad block \n",(offset / blk_size));
            continue;
        }

        /*lint -e64*/
        if (0 == mtd_read(p_mtd, offset, page_size, &retlength, (unsigned char*)cache_buf))
        /*lint +e64*/
        {
            /*Firstly we need to check the Share Region Info,If it was a valid block
            then read corresponding page*/
            rgn_hd = (rgn_hd_type*)cache_buf;
            
            if (SHARE_RGN_MAGIC == rgn_hd->magic)
            {
                current_page = block_index * blk_size;
                max_rgn_num = rgn_hd->sub_rgn_num;
                current_block = block_index;
                printk(KERN_DEBUG "Print entery current page = 0x%x  max_rgn_num = 0x%x  \n",
                       current_page, max_rgn_num);
                break;
            }  
            else
            {
                instr.addr = offset;
                if (0 != mtd_erase(p_mtd, &instr))
                {
                    printk(KERN_DEBUG "Current Erase error ,Mark bad block \n");
                    /* mark bad */
                    (void)mtd_block_markbad(p_mtd, offset);
                }
                printk(KERN_DEBUG "Print nand_erase offset = 0x%x  \n", offset);
                continue;
            }
        }
        else
        {
            printk(KERN_ERR "Nand_read  error  !\n");
            goto FalseQuit;
        }
    }

    printk(KERN_DEBUG "Print first current page = %u  \n", current_page);

    if(block_index != blk_count)
    {
        /*从 当前block 的 下一个block 分区开始寻找下一个 没使用的BLOCK*/
        dest_block = (block_index + 1) % blk_count;
    }

    /*Find the first unused block*/
    for (block_index  = 0; block_index  < blk_count; block_index++)
    {
        offset = start_offset + dest_block * blk_size;
        if (0 != mtd_block_isbad(p_mtd, offset))
        {
                printk(KERN_DEBUG "Find Dest Block ID = 0x%x is bad block \n", (offset / blk_size));
                dest_block = (dest_block + 1) % blk_count;
                continue;
        }
        else
        {
            if (current_block != dest_block)
            {
                instr.addr = offset;
                if (0 != mtd_erase(p_mtd, &instr))
                {
                    /* mark bad */
                    (void)mtd_block_markbad(p_mtd, offset);
                    
                    dest_block = (dest_block + 1) % blk_count;
                    printk(KERN_DEBUG "Dest Erase error ,Mark bad block \n");
                    continue;
                }
                dest_page = dest_block * blk_size;
                break;
            }
        }
        dest_block = (dest_block + 1) % blk_count;
    }

    /*将当前页的数据和更新的数据分别取出写入目标BLOCK*/
    if (INVALID_PAGE_ID != dest_page)
    {   
        start_index = 0;
        /*第一次升级SHARE RGN分区  或者RGN_MAX_NUM有变化*/
        if ((unsigned int)region_type >= max_rgn_num)
        {
            offset = start_offset;
            memset(cache_buf,0xFF, page_size);  /*flash 操作buf 清为0XFF*/
            rgn_hd_1.magic = SHARE_RGN_MAGIC;
            rgn_hd_1.sub_rgn_num = RGN_MAX_NUM;
            memcpy(cache_buf,(void *)&rgn_hd_1,sizeof(rgn_hd_type));
            printk(KERN_DEBUG "Print First Dest page = %u  \n",dest_page);

            /*擦除当前BLOCK*/
            if ((INVALID_BLOCK_ID == current_block) || (0 < max_rgn_num))
            {
                offset = offset + dest_page;
            }
            else
            {
                offset = offset + current_page;
            }

            /*lint -e64*/
            if (0 != mtd_write(p_mtd, offset, page_size, &retlength,
                (unsigned char*)cache_buf))
            /*lint +e64*/
            {
                printk(KERN_ERR "nand_write rgn_hd error  \n");
                goto FalseQuit;
            }

            memset(cache_buf, 0xFF, page_size); /*flash 操作buf 清为0XFF*/
            memcpy(cache_buf, data, data_len);
            /*lint -e64*/
            if (0 != mtd_write(p_mtd, (offset  + region_type * page_size),
                page_size, &retlength, (unsigned char*)cache_buf))
            /*lint +e64*/
            {
                printk(KERN_ERR "nand_write region page error  \n");
                goto FalseQuit;
            }
            start_index = 1;

        }

        if ((INVALID_PAGE_ID != current_page) && (dest_page != current_page))
        {  
            offset = start_offset;
            printk(KERN_DEBUG "Print not First Dest page = %u  \n",dest_page);
            printk(KERN_DEBUG "Print last current page = %u  \n",current_page);
            for (; start_index < max_rgn_num; start_index++)            
            {
                if (start_index != region_type)
                {
                    memset(cache_buf,0xFF, page_size); /*flash 操作buf 清为0XFF*/
                    /*lint -e64*/
                    if (0 == mtd_read(p_mtd,(offset + current_page + start_index * page_size),
                    page_size, &retlength,(unsigned char*)cache_buf))
                    /*lint +e64*/
                    {
                        if (0 == start_index)
                        {
                            rgn_hd_type *temp_hd = NULL;
                            temp_hd = (rgn_hd_type *)cache_buf;
                            printk(KERN_DEBUG "magic = 0x%x   max_num = 0x%x \n", (unsigned int)temp_hd->magic,
                                    (unsigned int)temp_hd->sub_rgn_num);  
                        }
                        /*lint -e64*/
                        if (0 != mtd_write(p_mtd, (offset + dest_page 
                            + start_index * page_size),
                            page_size, &retlength, (unsigned char*)cache_buf))
                        /*lint -e64*/
                        {
                            printk(KERN_DEBUG "nand_write start_index = %u  offset = 0x%x\n",
                                   start_index, (offset + dest_page
                                   + region_type * page_size));
                            continue;
                        }
                    }
                }
                else
                {
                    memset(cache_buf, 0xFF, page_size); /*flash 操作buf 清为0XFF*/
                    memcpy(cache_buf, data, data_len);
                    /*lint -e64*/
                    if (0 != mtd_write(p_mtd, (offset + dest_page + region_type * page_size),
                        page_size, &retlength, (unsigned char*)cache_buf))
                    /*lint -e64*/
                    {
                        printk(KERN_ERR "nand_write error \n");
                        goto FalseQuit;
                    }
                    printk(KERN_DEBUG "Write offset = 0x%x ! \n",
                           (offset + dest_page + region_type * page_size)); 
                }
            }

            instr.addr = offset + current_page;
            if (0 == mtd_erase(p_mtd, &instr))
            {
                printk(KERN_DEBUG "nand_erase offset = 0x%x ! \n",(offset + current_page)); 
            }
            else
            {
                printk(KERN_DEBUG "Last Erase current error ,Mark bad block \n");
                /* mark bad */
                (void)mtd_block_markbad(p_mtd, offset);
                goto FalseQuit;
            } 
        }
        else if (dest_page == current_page)
        {
            printk(KERN_DEBUG "Error dest_page == current_page offset = 0x%x  \n", current_page);
            goto FalseQuit;
        }
        
        flash_nand_oper_region_close(p_mtd, &opregion);
        return TRUE;
    }
FalseQuit:
    printk(KERN_ERR "flash_update_share_region_info error \n");
    flash_nand_oper_region_close(p_mtd, &opregion);
    return FALSE;
}
#else
boolean flash_update_share_region_info(rgn_pos_e_type region_type, void* data, size_t data_len)
{
    int ret = -1;
    unsigned int    block_index        = 0;
    unsigned int    start_index        = 0;
    unsigned int    blk_size           = 0;
    unsigned int    blk_count          = 0;
    unsigned int    max_rgn_num        = 0;
    unsigned int    dest_block         = 0;
    unsigned int    current_block      = INVALID_BLOCK_ID;
    unsigned int    current_page       = INVALID_PAGE_ID;
    unsigned int    dest_page          = INVALID_PAGE_ID;
    rgn_hd_type     *rgn_hd            = NULL;
    unsigned int    offset             = 0;
    unsigned int    page_size          = 0;
    uint8           *cache_buf         = NULL;
    unsigned int   start_offset        = 0;
    struct  erase_info  instr;
    int retlength = 0;
    rgn_hd_type rgn_hd_1 = {0};
    struct mtd_info *p_mtd = NULL;
    oper_region_struct_t opregion = {0};
    uint8 *flash_block_buf = NULL;
    /* 当前数据页所在块和目的页所在块查找指示变量*/
    boolean  find_curr_blk = FALSE;
    boolean  find_dest_blk = FALSE;

    /*入参条件检测*/
    if (region_type >= RGN_MAX_NUM || NULL == data)
    {
        printk(KERN_ERR "Input parm error \n");
        return FALSE;
    }

    ret = flash_nand_oper_region_init(SHARE_REGION, &p_mtd, &opregion);
    if ((0 != ret) || (NULL == p_mtd) || (NULL == opregion.buffer))
    {
        printk(KERN_ERR "flash_update_share_region_info init fail  \n");
        flash_nand_oper_region_close(p_mtd, &opregion);
        return FALSE;
    }

    /*Get Partition and Device Info*/
    blk_count    = child_region[SHARE_REGION].length;
    blk_size     = opregion.block_size;
    start_offset = opregion.start_addr;
    page_size    = opregion.page_size;
    cache_buf    = opregion.buffer;

    instr.mtd = p_mtd;
    instr.len = p_mtd->erasesize;
    instr.callback = NULL;
    instr.priv = 0;

    if (data_len > page_size)
    {
        printk(KERN_ERR "Warning: input data is truncated\n");
        data_len = page_size;
    }

    if ((0 == blk_size) || (0 == blk_count))
    {
        printk(KERN_ERR "blk_size or blk_count is zero, exit.\n");
        goto FalseQuit;
    }
    /*instr.addr*/
    for (block_index = 0; block_index < blk_count; block_index++)
    {
        offset = start_offset + blk_size * block_index;

        /*If the blk is bad blk or it is erase */
        if (0 != mtd_block_isbad(p_mtd, offset))
        {
            printk(KERN_DEBUG "Find Current Block ID = 0x%x is bad block \n",(offset / blk_size));
            continue;
        }

        /*lint -e64*/
        if (0 == mtd_read(p_mtd, offset, page_size, &retlength, (unsigned char*)cache_buf))
        /*lint +e64*/
        {
            /*Firstly we need to check the Share Region Info,If it was a valid block
            then read corresponding page*/
            rgn_hd = (rgn_hd_type*)cache_buf;
            if (SHARE_RGN_MAGIC == rgn_hd->magic)
            {
                current_page = block_index * blk_size;
                max_rgn_num = rgn_hd->sub_rgn_num;
                current_block = block_index;
                /* 找到当前块 */
                find_curr_blk = TRUE;
                printk(KERN_DEBUG "Print entery current page = 0x%x  max_rgn_num = 0x%x  \n",
                       current_page, max_rgn_num);
                break;
            }
            else
            {
                instr.addr = offset;
                if (0 != mtd_erase(p_mtd, &instr))
                {
                    printk(KERN_DEBUG "Current Erase error ,Mark bad block \n");
                    /* mark bad */
                    (void)mtd_block_markbad(p_mtd, offset);
                }
                printk(KERN_DEBUG "Print nand_erase offset = 0x%x  \n", offset);
                continue;
            }
        }
        else
        {
            printk(KERN_ERR "Nand_read  error  !\n");
            goto FalseQuit;
        }
    }

    printk(KERN_DEBUG "Print first current page = %u  \n", current_page);

    if (block_index != blk_count)
    {
        /*从 当前block 的 下一个block 分区开始寻找下一个 没使用的BLOCK*/
        dest_block = (block_index + 1) % blk_count;
    }

    /*Find the first unused block*/
    for (block_index  = 0; block_index  < blk_count; block_index++)
    {
        offset = start_offset + dest_block * blk_size;

        /* 是坏块则跳过 */
        if (0 != mtd_block_isbad(p_mtd, offset))
        {
                printk(KERN_DEBUG "Find Dest Block ID = 0x%x is bad block \n", (offset / blk_size));
                dest_block = (dest_block + 1) % blk_count;
                continue;
        }
        else
        {
            /* 若不是当前数据块，则擦除 */
            if (current_block != dest_block)
            {
                instr.addr = offset;
                if (0 != mtd_erase(p_mtd, &instr))
                {
                    /* mark bad */
                    (void)mtd_block_markbad(p_mtd, offset);
                    dest_block = (dest_block + 1) % blk_count;
                    printk(KERN_DEBUG "Dest Erase error, Mark bad block \n");
                    continue;
                }
            }

            dest_page = dest_block * blk_size;
            /* 找到目的好块 */
            find_dest_blk = TRUE;
            break;
        }
    }

    /*将当前页的数据和更新的数据分别取出写入目标BLOCK*/
    if (TRUE == find_dest_blk)
    {
        /*第一次升级SHARE RGN分区  或者RGN_MAX_NUM有变化*/
        if (FALSE == find_curr_blk)
        {
            offset = start_offset;

            memset(cache_buf, NAND_FILL_CHAR_APP, page_size);  /*flash 操作buf 清为0XFF*/

            rgn_hd_1.magic = SHARE_RGN_MAGIC;
            rgn_hd_1.sub_rgn_num = RGN_MAX_NUM;
            memcpy(cache_buf, (void *)&rgn_hd_1, sizeof(rgn_hd_type));
            printk(KERN_DEBUG "Print First Dest page = %u  \n",dest_page);

            /*擦除当前BLOCK*/
            if ((INVALID_BLOCK_ID == current_block) || (0 < max_rgn_num))
            {
                offset = offset + dest_page;
            }
            else
            {
                offset = offset + current_page;
            }

            /*lint -e64*/
            if (0 != mtd_write(p_mtd, offset, page_size, &retlength,
                (unsigned char*)cache_buf))
            /*lint +e64*/
            {
                printk(KERN_ERR "nand_write rgn_hd error  \n");
                goto FalseQuit;
            }

            memset(cache_buf, NAND_FILL_CHAR_APP, page_size); /*flash 操作buf 清为0XFF*/
            memcpy(cache_buf, data, data_len);
            /*lint -e64*/
            if (0 != mtd_write(p_mtd, (offset  + region_type * page_size),
                page_size, &retlength, (unsigned char*)cache_buf))
            /*lint +e64*/
            {
                printk(KERN_ERR "nand_write region page error  \n");
                goto FalseQuit;
            }

        }
        else if ((TRUE == find_curr_blk) && (dest_page != current_page))
        {
            offset = start_offset;
            printk(KERN_DEBUG "Print not First Dest page = %u  \n", dest_page);
            printk(KERN_DEBUG "Print last current page = %u  \n", current_page);
            for (start_index = 0; start_index < max_rgn_num; start_index++)
            {
                if (start_index != region_type)
                {
                    memset(cache_buf, NAND_FILL_CHAR_APP, page_size); /*flash 操作buf 清为0XFF*/
                    /*lint -e64*/
                    if (0 == mtd_read(p_mtd, (offset + current_page + start_index * page_size),
                    page_size, &retlength, (unsigned char*)cache_buf))
                    /*lint +e64*/
                    {
                        if (0 == start_index)
                        {
                            rgn_hd_type *temp_hd = NULL;
                            temp_hd = (rgn_hd_type *)cache_buf;
                            printk(KERN_DEBUG "magic = 0x%x   max_num = 0x%x \n",
                                    (unsigned int)temp_hd->magic,
                                    (unsigned int)temp_hd->sub_rgn_num);
                        }
                        /*lint -e64*/
                        if (0 != mtd_write(p_mtd,
                                    (offset + dest_page + start_index * page_size),
                                    page_size,
                                    &retlength,
                                    (unsigned char*)cache_buf))
                        /*lint -e64*/
                        {
                            printk(KERN_DEBUG "nand_write start_index = %u  offset = 0x%x\n",
                                   start_index, (offset + dest_page
                                   + region_type * page_size));
                            continue;
                        }
                    }
                }
                else
                {
                    memset(cache_buf, NAND_FILL_CHAR_APP, page_size); /*flash 操作buf 清为0XFF*/
                    memcpy(cache_buf, data, data_len);
                    /*lint -e64*/
                    if (0 != mtd_write(p_mtd, (offset + dest_page + region_type * page_size),
                        page_size, &retlength, (unsigned char*)cache_buf))
                    /*lint -e64*/
                    {
                        printk(KERN_ERR "nand_write error \n");
                        goto FalseQuit;
                    }
                    printk(KERN_DEBUG "Write offset = 0x%x ! \n",
                           (offset + dest_page + region_type * page_size));
                }
            }

            /* 当前数据擦除 */
            instr.addr = offset + current_page;
            if (0 == mtd_erase(p_mtd, &instr))
            {
                printk(KERN_DEBUG "nand_erase offset = 0x%x ! \n",(offset + current_page));
            }
            else
            {
                printk(KERN_DEBUG "Last Erase current error ,Mark bad block \n");
                /* mark bad */
                (void)mtd_block_markbad(p_mtd, offset);
                goto FalseQuit;
            }
        }
        else if ((TRUE == find_curr_blk) && (dest_page == current_page))
        {
            /* 仅剩余一个好块 */
            offset = start_offset;

            /* 申请buffer */
            flash_block_buf = (uint8*)kmalloc(max_rgn_num * page_size, GFP_KERNEL);
            if (NULL == flash_block_buf)
            {
                printk(KERN_DEBUG, "malloc fail.");
                goto FalseQuit;
            }

            (void)memset(flash_block_buf, NAND_FILL_CHAR_APP, (max_rgn_num * page_size));

            for (start_index = 0; start_index < max_rgn_num; start_index++)
            {
                if(start_index == region_type)
                {
                    /* 待更新页不读取 */
                    continue;
                }

                /*lint -e64*/
                if (0 != mtd_read(p_mtd, (offset + current_page + start_index * page_size), page_size,
                            &retlength, (unsigned char*)(flash_block_buf + start_index * page_size)))
                /*lint +e64*/
                {
                    printk(KERN_DEBUG, "read error, start_index = %d offset = 0x%x.", start_index,
                            (offset + current_page + start_index * page_size));
                    (void)kfree(flash_block_buf);
                    flash_block_buf = NULL;
                    goto FalseQuit;
                }
            }

            /* 写入数据复制到对应位置 */
            (void)memcpy((void *)(flash_block_buf + region_type * page_size), (void *)data, data_len);

            instr.addr = offset + current_page;
            if (0 == mtd_erase(p_mtd, &instr))
            {
                printk(KERN_DEBUG, "Block ID = %d is erased.", (offset + current_page) / blk_size);
            }
            else
            {
                /* mark bad */
                (void)mtd_block_markbad(p_mtd, offset + current_page);
                (void)kfree(flash_block_buf);
                flash_block_buf = NULL;
                printk(KERN_DEBUG, "Block ID = %d is marked bad.", (offset + current_page) / blk_size);
                goto FalseQuit;
            }

            /* 按页写入 */
            for (start_index = 0; start_index < max_rgn_num; start_index++)
            {
                /*lint -e64*/
                if (0 != mtd_write(p_mtd, (offset + current_page + start_index * page_size), page_size,
                            &retlength, (unsigned char*)(flash_block_buf + start_index * page_size)))
                /*lint -e64*/
                {
                    printk(KERN_DEBUG, "write error, start_index = %d offset = 0x%x", start_index,
                            (offset + current_page + start_index * page_size));
                    (void)kfree(flash_block_buf);
                    flash_block_buf = NULL;
                    goto FalseQuit;
                }
            }

            (void)kfree(flash_block_buf);
            flash_block_buf = NULL;

        }
        else
        {
            goto FalseQuit;
        }

        flash_nand_oper_region_close(p_mtd, &opregion);
        return TRUE;
    }
FalseQuit:
    printk(KERN_ERR "flash_update_share_region_info error \n");
    flash_nand_oper_region_close(p_mtd, &opregion);
    return FALSE;
}
#endif

/*===========================================================================
 
FUNCTION  FLASH_NAND_OPER_REGION_INIT
 
DESCRIPTION
    Initilize user partition info,and get the handle of nand flash
RETURN VALUE
    执行OK返回0,否则返回非0
SIDE EFFECTS
  None
 
===========================================================================*/
int flash_nand_oper_region_init(oper_region_type  oper_region_idx,
                            struct mtd_info **pp_mtd, oper_region_struct_t *p_opregion)
{
    uint8 idx = (uint8)oper_region_idx;
    char *parent_name_ptr = NULL;
    unsigned int offset = 0;
    struct mtd_info *p_mtd = NULL;

    
    /* 范围保护 */
    if((idx >= REGION_MAX) || (NULL == pp_mtd) || (NULL == p_opregion))
    {
        printk(KERN_ERR "Parm error: idx = %d max_ids = %d\n",idx,REGION_MAX);
        return -1;
    }
 
    /*lint -e662*/
    parent_name_ptr = (char *)child_region[idx].parent_name;
    printk(KERN_DEBUG "parent_name_ptr =%s \n",parent_name_ptr);
    /*lint +e662*/
    /*lint -e661*/
    offset = child_region[idx].offset;
    /*lint +e661*/

    /* get mtd device */
    p_mtd = get_mtd_device_nm(parent_name_ptr);
    if (NULL == p_mtd)
    {
        printk(KERN_ERR "get_mtd_device_nm error\n");
        return -1;
    }

    printk(" info :mtd->erasesize = %d ,mtd->writesize = %d \n",
           p_mtd->erasesize, p_mtd->writesize);
    
    /* get the flash address */
    p_opregion->block_size = p_mtd->erasesize;
    p_opregion->page_size = p_mtd->writesize;
    p_opregion->start_addr = (offset * p_opregion->block_size);
    p_opregion->buffer = (uint8 *)kmalloc(p_opregion->page_size, GFP_KERNEL);
    if (NULL == p_opregion->buffer)
    {
        printk(KERN_ERR "flash_nand_oper_region_init: malloc buffer error\n");
        put_mtd_device(p_mtd);
        return -1;
    }

    printk(KERN_DEBUG "operation region index = %d start_addr = %u \n", idx, (unsigned int)(p_opregion->start_addr));

    /* 初始化读写flash时使用的缓冲区 */
    memset((void *)(p_opregion->buffer), NAND_FILL_CHAR_APP,
           p_opregion->page_size);

    /*赋值mtd指针带出*/
    *pp_mtd = p_mtd;

    return 0;
}

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
void flash_nand_oper_region_close(struct mtd_info *p_mtd, oper_region_struct_t *p_opregion)
{
    if((NULL != p_opregion) && (NULL != p_opregion->buffer))
    {
        (void)kfree(p_opregion->buffer);
        p_opregion->buffer = NULL;
    }
    if(NULL != p_mtd)
    {
        put_mtd_device(p_mtd);
        p_mtd = NULL;
    }

    return;
}


int huawei_dload_set_swver_to_oeminfo(void)
{
    char *swver = NULL;
    char oem_swver[MAX_SOFTWARE_VERSION_LENGTH] = {0};
    version_buffer_page version_page = {0};

    /*获取单板的软件版本号*/
    swver = bsp_version_get_firmware();
    if(NULL == swver)
    {
        /*获取失败直接返回*/
        printk(KERN_ERR "huawei_dload_set_swver_to_oeminfo: get src ver failed.\n");
        return -1;
    }

    /*获取OEMINFO 分区里面存放的软件版本号*/
    (void)flash_get_share_region_info(RGN_SWVER_FLAG, (void *)(&version_page), sizeof(version_page));

    /*将版本号写入OEMINFO 分区，方便升级读取 */
    if(NV_SWVER_FLAG_MAGIC_NUM == version_page.magic_number)
    {
        memcpy((void*)oem_swver,
                        (void *)(version_page.version_page_buffer),
                        strlen(version_page.version_page_buffer));
        
        if((strlen(swver) == strlen(oem_swver))
                && (0 == strncmp(swver, (char *)(oem_swver), MAX_SOFTWARE_VERSION_LENGTH)))
        {
            /*版本号相同直接返回*/
            return 0;
        }
    }

    /*其他情况写入版本号*/
    version_page.magic_number = NV_SWVER_FLAG_MAGIC_NUM; 
    memset(version_page.version_page_buffer, 0, sizeof(version_page.version_page_buffer)); 
    memcpy((void*)(version_page.version_page_buffer), (void *)(swver), strlen(swver));
    (void)flash_update_share_region_info(RGN_SWVER_FLAG, (void *)(&version_page), sizeof(version_buffer_page));

    return 0;
}

#if (FEATURE_ON == MBB_DLOAD_RECORD_BAD)

int get_part_blocks(int part_index)
{
    int i = 0;
    struct ST_PART_TBL *ptable = NULL;
    struct nand_spec_shared_mem *spec = NULL;

    spec = (struct nand_spec_shared_mem *)SHM_MEM_NAND_SPEC_ADDR;

    if (-1 == part_index)
    {
        return (spec->chip_size / spec->block_size);
    }

    ptable = ptable_get_ram_data();

    /* find the partition */
    while (0 != strcmp(PTABLE_END_STR, ptable->name))
    {
        if (i == part_index)
        {
            break;
        }

        ptable++;
        i++;
    }

    if (0 == strcmp(PTABLE_END_STR, ptable->name))
    {
    	printk("ERROR: can't find partition %d\n", part_index);
        return 0;
    }

    return (ptable->capacity / spec->block_size);
}


int flash_get_bad_block(char *buff, int len)
{
    int rc = 0;
    int i = 0;
    boolean read_res = FALSE;
    int chip_block = 0;;

    if ((NULL == buff) || (len < 0))
    {
        printk(KERN_ERR "flash_get_bad_block: param error\n");
        return 0;
    }

    if (BAD_BLOCK_INFO_MAGIC != bb_info.magic_number)
    {
        memset(&bb_info, 0x0, sizeof(bb_info));
        read_res = flash_get_share_region_info(RGN_BADBLOCK_INFO,
                                    &bb_info, sizeof(bb_info));
        if ((TRUE != read_res)
            || (BAD_BLOCK_INFO_MAGIC != bb_info.magic_number))
        {
            printk(KERN_ERR "flash_get_bad_block: get_share_region fail or not init\n");
            return 0;
        }
    }

    chip_block = get_part_blocks(-1);

    rc += snprintf(buff, len, "%d,%d\r\n", bb_info.sum, chip_block);

    for (i = 0; i < MAX_BAD_PART_NUM; i++)
    {
        if (0 != bb_info.part_sum[i])
        {
            rc += snprintf(buff + rc, len - rc, "%d,%d,%d\r\n", 
                          i, bb_info.part_sum[i], get_part_blocks(i));
        }
    }

    rc += snprintf(buff + rc, len - rc, "\r\n");

    if (rc == len)
    {
        printk(KERN_ERR "flash_get_bad_block: output buffer too small\n");
        return 0;
    }
    
    return rc;
}
#endif

#endif
