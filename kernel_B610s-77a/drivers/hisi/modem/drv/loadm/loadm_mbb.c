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
/******************************************************************************
  File Name       : loadm.c
  Description     : load modem image(ccore image),run in ccore
  History         :
******************************************************************************/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>

#include "product_config.h"
#include <linux/dma-mapping.h>
#include <linux/decompress/generic.h>
#include <asm/dma-mapping.h>
#include <asm/cacheflush.h>
#include "loadm.h"
#include "bsp_memmap.h"
#include "ptable_com.h"
#include "hi_common.h"
#include "bsp_nandc.h"
#include <bsp_ipc.h>
#include <bsp_sec.h>
#include <soc_onchiprom.h>
#include <bsp_shared_ddr.h>
#include "load_image.h"
#include <mdrv.h>
#include "bsp_sram.h"

static void* g_ccore_entry = NULL;
struct image_head g_ccore_image_head;

/*****************************************************************************
 函 数 名  : 加载ccore镜像并解复位C核
 功能描述  : load partition image
 输入参数  : 无
 输出参数  : 镜像加载成功返回OK,失败返回ERROR
 返 回 值  : 无
 调用函数  :
 被调函数  :
*****************************************************************************/
int his_modem_load_ccore_image(void)
{
    int ret = 0;
    int offset = 0;
    int skip_len = 0;
    u32 image_total_length = 0;
    void *image_load_addr = 0;
    char *part_name = PTABLE_MODEM_NM;
    decompress_fn inflate_fn = NULL;

    printk(KERN_ERR ">>loading ccore image...\n");

    ret = bsp_nand_read(part_name, (FSZ)0, &g_ccore_image_head, sizeof(struct image_head) , &skip_len);
    if (NAND_OK != ret)
    {
        printk(KERN_ERR"fail to read ccore image head, error code 0x%x\n", ret);
        return NAND_ERROR;
    }

    /*coverity[uninit_use_in_call] */
    if (memcmp(g_ccore_image_head.image_name, CCORE_IMAGE_NAME, sizeof(CCORE_IMAGE_NAME)))
    {
        printk(KERN_ERR"ccore image error!!.\n");
        return NAND_ERROR;
    }

    /*coverity[uninit_use] */
    /* 镜像大小要与MBB产品实际分区大小比较 */
    if (g_ccore_image_head.image_length + 2*IDIO_LEN + OEM_CA_LEN > MBB_PTABLE_VXWORK_LEN)
    {
        printk(KERN_ERR"loadsize is incorrect, 0x%x!\n",
            g_ccore_image_head.image_length + 2*IDIO_LEN + OEM_CA_LEN);
        return NAND_ERROR;
    }

    /*coverity[uninit_use_in_call] */
    g_ccore_entry = ioremap_cached(g_ccore_image_head.load_addr, DDR_MCORE_SIZE - (MCORE_TEXT_START_ADDR - DDR_MCORE_ADDR));
    if(!g_ccore_entry)
    {
        printk(KERN_ERR"ioremap failed.\n");
        return NAND_ERROR;
    }

    offset += sizeof(struct image_head) + skip_len;
    image_total_length = (u32)g_ccore_image_head.image_length + 2*IDIO_LEN + OEM_CA_LEN;

    /*coverity[uninit_use] */
    if (g_ccore_image_head.is_compressed)
    {
        image_load_addr = g_ccore_entry - (MCORE_TEXT_START_ADDR - DDR_MCORE_ADDR)
            + DDR_MCORE_SIZE - image_total_length;
    }
    else
    {
        image_load_addr = g_ccore_entry;
    }

    ret = bsp_nand_read(part_name, offset, image_load_addr, image_total_length, &skip_len);
    if(NAND_OK != ret)
    {
        printk(KERN_ERR"fail to read ccore image, error code 0x%x\n", ret);
        goto exit;
    }

    ret = bsp_sec_check(image_load_addr, g_ccore_image_head.image_length);
    if (ret)
    {
        printk(KERN_ERR"fail to check ccore image, error code 0x%x\n", ret);
        goto exit;
    }

    if (g_ccore_image_head.is_compressed)
    {
        printk(KERN_ERR ">>start to decompress ccore image ...\n");
        inflate_fn = decompress_method((const unsigned char *)image_load_addr, 2, NULL);
        if (inflate_fn)
        {
            ret = inflate_fn((unsigned char*)image_load_addr,
                g_ccore_image_head.image_length, NULL, NULL, (unsigned char*)g_ccore_entry,
                NULL, (void(*)(char*))printk);
            if (ret)
            {
                printk(KERN_ERR"fail to decompress ccore image, error code 0x%x\n", ret);
                goto exit;
            }
        }
        else
        {
            printk(KERN_ERR"fail to get decompress method\n");
            goto exit;
        }
    }

    dmac_map_area(g_ccore_entry, DDR_MCORE_SIZE - (MCORE_TEXT_START_ADDR - DDR_MCORE_ADDR), DMA_TO_DEVICE);
    outer_clean_range(g_ccore_image_head.load_addr, DDR_MCORE_SIZE - (MCORE_TEXT_START_ADDR - DDR_MCORE_ADDR));

exit:
    iounmap(g_ccore_entry);

    printk(KERN_ERR ">>load ccore ok, entey %#x, length %#x.\n", g_ccore_image_head.load_addr, g_ccore_image_head.image_length);

    return ret;
}

static int __init his_modem_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct ST_PART_TBL * mc_part = NULL;

    printk(KERN_ERR"his_modem_probe.\n");

    mc_part = find_partition_by_name(PTABLE_MODEM_NM);
    if(NULL == mc_part)
    {
        printk(KERN_ERR"find ccore ptable fail.\n");
        return -EAGAIN;
    }

    if(NAND_OK == his_modem_load_ccore_image())
    {
        printk(KERN_ERR "load ccore image succeed\n");
    }
    else
    {
        printk(KERN_ERR"load ccore image failed\n");
        return -EAGAIN;
    }

    /*在MBB平台上需要发送复位请求中断*/
    ret = bsp_load_notify_ccpu_start();
    if (ret)
    {
        printk(KERN_ERR"send ipc to unreset ccore failed, ret=0x%x\n", ret);
    }
    else
    {
        printk(KERN_ERR"send ipc to unreset ccore succeed\n");
    }

    return ret;

}

static struct platform_device his_modem_device = {
    .name = "his_modem",
    .id = 0,
    .dev = {
    .init_name = "his_modem",
    },
};
static struct platform_driver his_modem_drv = {
    .probe      = his_modem_probe,
    .driver     = {
        .name     = "his_modem",
        .owner    = THIS_MODULE,
    },
};

static int __init his_modem_init_driver(void)
{
    int ret = 0;
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    
    if (NULL == smem_data)
    {
        printk(KERN_ERR "Dload smem_data malloc fail!\n");
        return -1;
    }

    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        return ret;
    }

    ret = platform_device_register(&his_modem_device);
    if(ret)
    {
        printk(KERN_ERR"register his_modem device failed\n");
        return ret;
    }

    ret = platform_driver_register(&his_modem_drv);
    if(ret)
    {
        printk(KERN_ERR"register his_modem driver failed\n");
        platform_device_unregister(&his_modem_device);
    }

    return ret;

}

module_init(his_modem_init_driver);
//module_exit(his_modem_exit_driver);

MODULE_DESCRIPTION("HIS Balong  Modem load ");
MODULE_LICENSE("GPL");

