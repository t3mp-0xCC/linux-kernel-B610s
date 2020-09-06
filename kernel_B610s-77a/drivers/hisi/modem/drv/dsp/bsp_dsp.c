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


/*lint --e{537} */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/jiffies.h>

#include <product_config.h>

#include <hi_dsp.h>
#include <hi_onoff.h>
#include <ptable_com.h>

#include <bsp_om.h>
#include <bsp_dsp.h>
#include <bsp_ipc.h>
#include <bsp_icc.h>
#include <bsp_sec.h>
#include <bsp_sram.h>
#include <bsp_nandc.h>
#include <bsp_shared_ddr.h>
#include <bsp_sysctrl.h>
#include <drv_nv_id.h>
#include <drv_nv_def.h>
#include <mdrv.h>
#include "../../adrv/adrv.h"
#include <bsp_nvim.h>
#include <product_config.h>
#include "bsp_sram.h"


#if (LPHY_BBE16_MUTI_IMAGE_SIZE > DDR_TLPHY_IMAGE_SIZE)
#error Dsp image over ddr_tlphy_image_size 
#endif

#define IS_VALID_ADDR(addr) ((addr != NULL) ? true:false)

enum{
    DSP_BASE_ADDR = 0,
    DSP_OFFSET,
    DSP_START_BIT,
    DSP_END_BIT,
    DSP_ADDR_BUTT
};

enum{
    BBE16_PD_CLK_STAT = 0,    
    BBE16_CORE_CLK_STAT,
    BBE16_PD_SRST_STAT,
    BBE16_CORE_SRST_STAT,
    BBE16_MTCMOS_RDY_STAT,
    BBE16_ISO_CTRL_STAT,
    DSP_DTS_REG_BUTT    
};

struct dsp_state
{
    u32 is_dsp_power_on;
    u32 is_dsp_clock_enable;
    u32 is_bbe_clock_enable;
    u32 is_dsp_unreset;
    u32 is_bbe_unreset;
};

struct dsp_sysctrl{
    void* base_addr;
    u32 offset;
    u32 start_bit;
    u32 end_bit;
};

struct dsp_mainctrl{
    struct dsp_sysctrl reg_sysctrl[DSP_DTS_REG_BUTT];
};

struct dsp_mainctrl g_dsp_ctrl;

int bsp_dsp_load_image(char* part_name)
{
    int ret = 0;
    u32 offset = 0;
    u32 skip_len = 0;

    void *bbe_ddr_addr = NULL;

    /*coverity[var_decl] */
    struct image_head head;

    /* clean ok flag */
    writel(0, (void*)(SHM_BASE_ADDR + SHM_OFFSET_DSP_FLAG));
    /* 指向一块DDR   空间用于存放镜像和配置数据 */
    bbe_ddr_addr = (void*)ioremap_nocache(DDR_TLPHY_IMAGE_ADDR, DDR_TLPHY_IMAGE_SIZE);
    if (NULL == bbe_ddr_addr)
    {
        printk("fail to io remap, %d \r\n", __LINE__);
        ret = -ENOMEM;
        goto err_unmap;
    }
    /* 获得在nand   中的bbe   镜像头 */
    if (NAND_OK != bsp_nand_read(part_name,  0, (void*)&head, (size_t)sizeof(struct image_head), &skip_len))
    {
        printk("fail to load dsp image head\r\n");
        ret = NAND_ERROR;
        goto err_unmap;
    }

    /*coverity[uninit_use_in_call] */
    /* 判断是否找到dsp   镜像 */
    if (memcmp(head.image_name, DSP_IMAGE_NAME, sizeof(DSP_IMAGE_NAME)))
    {
        printk("dsp image not found\r\n");
        goto err_unmap;
    }

    offset += LPHY_BBE16_MUTI_IMAGE_OFFSET + sizeof(struct image_head) + skip_len;
    /* 将镜像从nand   读入申请的ddr   中 */

    if (LPHY_BBE16_MUTI_IMAGE_SIZE > DDR_TLPHY_IMAGE_SIZE)
    {
        printk(KERN_ERR"*******************************************************************\n");
        printk(KERN_ERR"LPHY_TOTAL_IMG_SIZE, size: 0x%x, DDR_TLPHY_IMAGE_SIZE, size: 0x%x.\n",
            LPHY_BBE16_MUTI_IMAGE_SIZE, DDR_TLPHY_IMAGE_SIZE);
        printk(KERN_ERR"LPHY_TOTAL_IMG_SIZE is bigger than DDR_TLPHY_IMAGE_SIZE.\n");
        printk(KERN_ERR"Load dsp from flash to ddr failed.\n");
        printk(KERN_ERR"********************************************************************\n");
        ret = -1;
        goto err_unmap;
    }
    else
    {        
        if (NAND_OK == bsp_nand_read(part_name, offset, (void*)bbe_ddr_addr, LPHY_BBE16_MUTI_IMAGE_SIZE + 2*IDIO_LEN + OEM_CA_LEN, &skip_len))
        {
            printk(KERN_ERR"succeed to load dsp image, address: 0x%x, size: 0x%x.\n",
            DDR_TLPHY_IMAGE_ADDR, LPHY_BBE16_MUTI_IMAGE_SIZE);
        }
        else
        {
            printk("fail to load dsp image\r\n");
            ret = NAND_ERROR;
            goto err_unmap;
        }
        /*增加对 lphy.bin 的校验,校验函数格式和C核一致，使用MBB宏与基线区分*/
        ret = bsp_sec_check(bbe_ddr_addr, LPHY_BBE16_MUTI_IMAGE_SIZE);
        if (ret)
        {
            printk(KERN_ERR"fail to check dsp image, error code 0x%x\n", ret);
            goto err_unmap;
        }
    }

    /* set the ok flag of dsp image */
    writel(DSP_IMAGE_STATE_OK, (void*)(SHM_BASE_ADDR + SHM_OFFSET_DSP_FLAG));

err_unmap:
    if (NULL != bbe_ddr_addr)
        iounmap(bbe_ddr_addr);

    return ret;
}

static inline void hi_dsp_get_reg(unsigned int *value, struct dsp_sysctrl *pstreg)
{
    unsigned int mask = 0;
    unsigned int temp = 0;
   
    temp   = readl(pstreg->base_addr + pstreg->offset);
    mask   = ((1U << (pstreg->end_bit - pstreg->start_bit + 1)) -1) << pstreg->start_bit;
    *value = temp & mask;
    *value = (*value) >> pstreg->start_bit; 
}

int bsp_bbe_tcm_accessible(void)
{
    u32 temp = 0;
    u32 temp1 = 0;
    struct dsp_state dsp_state = {0,0,0,0,0};

    if(IS_VALID_ADDR(g_dsp_ctrl.reg_sysctrl[BBE16_MTCMOS_RDY_STAT].base_addr))
    {
        hi_dsp_get_reg(&temp, &g_dsp_ctrl.reg_sysctrl[BBE16_MTCMOS_RDY_STAT]);
    }
    if(IS_VALID_ADDR(g_dsp_ctrl.reg_sysctrl[BBE16_ISO_CTRL_STAT].base_addr))
    {
        hi_dsp_get_reg(&temp1, &g_dsp_ctrl.reg_sysctrl[BBE16_ISO_CTRL_STAT]);
    }
    if(temp == 1 && temp1 == 0)
        dsp_state.is_dsp_power_on = 1;
    
    if(IS_VALID_ADDR(g_dsp_ctrl.reg_sysctrl[BBE16_PD_CLK_STAT].base_addr))
    {
        hi_dsp_get_reg(&temp, &g_dsp_ctrl.reg_sysctrl[BBE16_PD_CLK_STAT]);
    }
    if(temp == 1)
        dsp_state.is_dsp_clock_enable = 1;
    
    if(IS_VALID_ADDR(g_dsp_ctrl.reg_sysctrl[BBE16_CORE_CLK_STAT].base_addr))
    {
        hi_dsp_get_reg(&temp, &g_dsp_ctrl.reg_sysctrl[BBE16_CORE_CLK_STAT]);
    }
    if(temp == 1)
        dsp_state.is_bbe_clock_enable = 1;

    if(IS_VALID_ADDR(g_dsp_ctrl.reg_sysctrl[BBE16_PD_SRST_STAT].base_addr))
    {
        hi_dsp_get_reg(&temp, &g_dsp_ctrl.reg_sysctrl[BBE16_PD_SRST_STAT]);
    }
    if(temp == 0)
        dsp_state.is_dsp_unreset = 1;

    if(IS_VALID_ADDR(g_dsp_ctrl.reg_sysctrl[BBE16_CORE_SRST_STAT].base_addr))
    {
        hi_dsp_get_reg(&temp, &g_dsp_ctrl.reg_sysctrl[BBE16_CORE_SRST_STAT]);
    }
    if(temp == 0)
        dsp_state.is_bbe_unreset = 1;

    if(dsp_state.is_dsp_power_on == 1 && 
        dsp_state.is_dsp_clock_enable == 1 && 
        dsp_state.is_bbe_clock_enable == 1 && 
        dsp_state.is_dsp_unreset == 1 && 
        dsp_state.is_bbe_unreset == 1)
        return 0;

    return (-1);
}


int __init bsp_dsp_probe(struct platform_device *pdev)
{
    int ret = 0;
    u32 i;
    struct device_node *dev_node = NULL;
    char* node_name = "hisilicon,dsp_balong_dtcm";
    u32 reg_addr[4];
    char* reg_name[DSP_DTS_REG_BUTT] = {"bbe16_pd_clk_stat",
                                        "bbe16_core_clk_stat",
                                        "bbe16_pd_srst_stat",
                                        "bbe16_core_srst_stat",
                                        "bbe16_mtcmos_rdy_stat",
                                        "bbe16_iso_ctrl_stat"
                                        };
    struct ST_PART_TBL* dsp_part = NULL;
    
    dev_node = of_find_compatible_node(NULL,NULL,node_name);
    if(dev_node == NULL)
    {
        printk(KERN_ERR"DSP get node_name failed!\n");
    }

    for(i = 0; i < DSP_DTS_REG_BUTT; i++)
    {
        ret = of_property_read_u32_array(dev_node, reg_name[i], reg_addr, DSP_ADDR_BUTT);   
        if(ret)
        {
            printk(KERN_ERR"dsp_prob g_dsp_ctrl init failed : dtcm dev_node reg read failed!\n");
            break;
        }
        
        g_dsp_ctrl.reg_sysctrl[i].base_addr = bsp_sysctrl_addr_get((void *)(unsigned long)reg_addr[DSP_BASE_ADDR]);
        g_dsp_ctrl.reg_sysctrl[i].offset    = reg_addr[DSP_OFFSET];
        g_dsp_ctrl.reg_sysctrl[i].start_bit = reg_addr[DSP_START_BIT];
        g_dsp_ctrl.reg_sysctrl[i].end_bit   = reg_addr[DSP_END_BIT];
    }
    
    /* 通过模块名来查找相应模块的镜像 */
    dsp_part = find_partition_by_name(PTABLE_DSP_NM);
    if(NULL == dsp_part)
    {
        printk("load dsp image succeed\r\n");
        ret = -EAGAIN;
        goto err_no_part;
    }
    /* 通过镜像名来加载dsp  镜像 */
    ret = bsp_dsp_load_image(dsp_part->name);
    if(ret < 0)
    {
        printk("bsp_dsp_load_image fail.\r\n");
        ret = -EAGAIN;
        goto err_no_part;
    }

err_no_part:


    return ret;
}

static struct platform_device bsp_dsp_device = {
    .name = "bsp_dsp",
    .id = 0,
    .dev = {
    .init_name = "bsp_dsp",
    },
};

static struct platform_driver bsp_dsp_drv = {
    .probe      = bsp_dsp_probe,
    .driver     = {
        .name     = "bsp_dsp",
        .owner    = THIS_MODULE,
    },
};

static int bsp_dsp_acore_init(void);
static void bsp_dsp_acore_exit(void);

static int __init bsp_dsp_acore_init(void)
{
    int ret = 0;
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        printk(KERN_ERR "nandc_mtd_dload_proc_deal:get smem_data error\n");
        ret = - EINVAL;
        return ret;
    }
    /*升级模式下，不启动加载DSP*/
    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        return ret;
    }

    ret = platform_device_register(&bsp_dsp_device);
    if(ret)
    {
        printk("register his_modem device failed\r\n");
        return ret;
    }

    ret = platform_driver_register(&bsp_dsp_drv);
    if(ret)
    {
        printk("register his_modem driver failed\r\n");
        platform_device_unregister(&bsp_dsp_device);
    }

    return ret;
}

static void __exit bsp_dsp_acore_exit(void)
{
    platform_driver_unregister(&bsp_dsp_drv);
    platform_device_unregister(&bsp_dsp_device);
}

module_init(bsp_dsp_acore_init);
module_exit(bsp_dsp_acore_exit);

MODULE_AUTHOR("HUAWEI DRIVER GROUP");
MODULE_DESCRIPTION("Driver for huawei product");
MODULE_LICENSE("GPL");



