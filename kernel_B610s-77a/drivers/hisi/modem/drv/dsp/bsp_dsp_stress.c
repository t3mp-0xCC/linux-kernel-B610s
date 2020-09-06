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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/io.h>

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>

#include <product_config.h>
#include <osl_bio.h>
#include <hi_base.h>
#include <hi_pwrctrl.h>

#include <hi_dsp.h>
#include "bsp_shared_ddr.h"
#include <bsp_busstress.h>
#include "soc_memmap_comm.h"
#include "bsp_version.h"
#include "bsp_sysctrl.h"
#include <of.h>


#ifdef __cplusplus
extern "C" {
#endif

#define AXI_MEM_64_SRC_FOR_BBE16_PHY       SRAM_V2P(MEM_FOR_BBE16_BUSSTRESS)
#define AXI_MEM_FOR_BBE16_SRC_ADDR_PHY     AXI_MEM_64_SRC_FOR_BBE16_PHY

#define IS_VALID_ADDR(addr) ((addr != 0x0) ? true:false)

static u32 bbe16_tcm_addr;

typedef enum{
    DSP_BASE_ADDR = 0,
    DSP_OFFSET,
    DSP_START_BIT,
    DSP_END_BIT,
    DSP_ADDR_BUTT
}DSP_DTS_SYSCTRL_ADDR;

typedef enum{
    BBE16_PD_CLK_EN = 0,
    BBE16_PD_CLK_DIS,
    BBE16_PD_SRST_DIS,
    BBE16_PD_SRST_EN,
    BBE16_CORE_SRST_DIS,
    BBE16_DBG_SRST_DIS = 5,
    BBE16_CORE_SRST_EN,
    BBE16_DBG_SRST_EN,
    PD,
    BBE16_RUNSTALL,
    BBE_REFCLK_EN = 10,
    BBE_REFCLK_DIS,
    PLL_LOCK,
    PLL_EN,
    PLL_BP,
    PLL_CLK_GT = 15,
    DIV_MODEM_BBE16,
    SEL_MODEM_BBE16,
    CLKDIVMASKEN,
    DSP0_CORE_CLK_DIS,
    DSP0_DBG_CLK_DIS = 20,
    BBE_CORE_CLK_EN,
    DSP0_DBG_CLK_EN,
    BBE16_MTCMOS_CTRL_EN,
    BBE16_MTCMOS_RDY_STAT,
    BBE16_ISO_CTRL_DIS = 25,
    BBE16_MTCMOS_CTRL_DIS,
    BBE16_ISO_CTRL_EN,
    DSP_DTS_REG_BUTT
}DSP_DTS_SYSCTRL;

struct dsp_sysctrl{
    u32 base_addr;
    u32 offset;
    u32 start_bit;
    u32 end_bit;
};

struct dsp_info{
    struct dsp_sysctrl dsp_sysctrl[DSP_DTS_REG_BUTT];
};

static struct dsp_info g_adsp_info;

static inline int hi_dsp_get_reg(unsigned int *value,unsigned int base,unsigned int offset,unsigned int lowbit,unsigned int highbit)
 {
     unsigned int mask = 0;
     unsigned int temp = 0;
     unsigned int reg    = base + offset;
    
     temp   = readl(reg);
     mask   = ((1 << (highbit - lowbit + 1)) -1) << lowbit;
     *value = temp & mask;
     *value = (*value) >> lowbit; 
 
     return 0;
 }
 
 static inline int hi_dsp_set_reg(unsigned int value,unsigned int base,unsigned int offset,unsigned int lowbit,unsigned highbit)
 {
     unsigned int reg    = base + offset;
     unsigned int temp   = 0;
     unsigned int mask   = 0;
 
     temp   = readl(reg);
     mask   = ((1 << (highbit - lowbit + 1)) -1) << lowbit;
     value  = (temp & (~mask)) | ((value <<lowbit) & mask);
     writel(value  ,reg);
 
     return 0;    
 
 }


s32 show_bbe16_test_status(void)
{
	u32 status = readl(bbe16_tcm_addr+0x4018);

	printk(KERN_INFO"test result:0x%x.\n", status);

    /*
       success ---- 0xAAAAAAAA
       fail    ---- 0x55555555
     */
    if (0xAAAAAAAA == status)
        return 0;
    else if (0x55555555 == status)
        return -1;
    else
        return -2;
}

void bbe16_unreset(void)
{
    if(IS_VALID_ADDR(g_adsp_info.dsp_sysctrl[BBE16_PD_SRST_DIS].base_addr))
    {
        hi_dsp_set_reg(1, g_adsp_info.dsp_sysctrl[BBE16_PD_SRST_DIS].base_addr, g_adsp_info.dsp_sysctrl[BBE16_PD_SRST_DIS].offset, g_adsp_info.dsp_sysctrl[BBE16_PD_SRST_DIS].start_bit, g_adsp_info.dsp_sysctrl[BBE16_PD_SRST_DIS].end_bit);
    }
    if(IS_VALID_ADDR(g_adsp_info.dsp_sysctrl[BBE16_CORE_SRST_DIS].base_addr))
    {
        hi_dsp_set_reg(1, g_adsp_info.dsp_sysctrl[BBE16_CORE_SRST_DIS].base_addr, g_adsp_info.dsp_sysctrl[BBE16_CORE_SRST_DIS].offset, g_adsp_info.dsp_sysctrl[BBE16_CORE_SRST_DIS].start_bit, g_adsp_info.dsp_sysctrl[BBE16_CORE_SRST_DIS].end_bit);
    }
}

s32 bbe16_stress_test_start(void)
{
    u32 readlen = 0;
    unsigned int handle;
    mm_segment_t oldfs;
    u32 buffer[0x10];
    long ret;
    void __iomem    *dsp_freqmode;

    int i = 0;
    char reg_name[DSP_DTS_REG_BUTT][32] = {"bbe16_pd_clk_en", "bbe16_pd_clk_dis", "bbe16_pd_srst_dis", "bbe16_pd_srst_en",
                            "bbe16_core_srst_dis", "bbe16_dbg_srst_dis", "bbe16_core_srst_en", "bbe16_dbg_srst_en", 
                            "pd", "bbe16_runstall", "bbe_refclk_en", "bbe_refclk_dis", 
                            "pll_lock", "pll_en", "pll_bp", "pll_clk_gt", 
                            "div_modem_bbe16", "sel_modem_bbe16", "clkdivmasken", "dsp0_core_clk_dis", 
                            "dsp0_dbg_clk_dis", "bbe_core_clk_en", "dsp0_dbg_clk_en", "bbe16_mtcmos_ctrl_en", 
                            "bbe16_mtcmos_rdy_stat", "bbe16_iso_ctrl_dis", "bbe16_mtcmos_ctrl_dis", "bbe16_iso_ctrl_en"};
    u32 reg_addr[4];

    char* node_name[2] = {

        "hisilicon,dsp_balong_dtcm",
        "hisilicon,dsp_balong_itcm"
    };
	struct device_node *dev_node = NULL;

    dev_node = of_find_compatible_node(NULL,NULL,node_name[1]);
	if(!dev_node)
	{
		
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_DSP, "get dsp_balong_dtcm node failed!\n");
		return;
	}

    
    for(i = 0; i < DSP_DTS_REG_BUTT; i++)
    {
        of_property_read_u32_array(dev_node, reg_name[i], reg_addr, DSP_ADDR_BUTT);   
        g_adsp_info.dsp_sysctrl[i].base_addr = (u32)bsp_sysctrl_addr_get((void *)reg_addr[DSP_BASE_ADDR]);
        g_adsp_info.dsp_sysctrl[i].offset = reg_addr[DSP_OFFSET];
        g_adsp_info.dsp_sysctrl[i].start_bit = reg_addr[DSP_START_BIT];
        g_adsp_info.dsp_sysctrl[i].end_bit = reg_addr[DSP_END_BIT];
    }

    if(PLAT_PORTING == bsp_get_version_info()->plat_type && CHIP_V750 == bsp_get_version_info()->chip_type)
    {
        dsp_freqmode = bsp_sysctrl_addr_get(HI_MODEM_SC_BASE_ADDR + MDM_PERI_CLK_DIV1);
        writel((readl(dsp_freqmode)& (0xffffffcf)),dsp_freqmode);
    }

    bbe16_unreset();

	if(IS_VALID_ADDR(g_adsp_info.dsp_sysctrl[BBE_CORE_CLK_EN].base_addr))
    {
        hi_dsp_set_reg(1, g_adsp_info.dsp_sysctrl[BBE_CORE_CLK_EN].base_addr, g_adsp_info.dsp_sysctrl[BBE_CORE_CLK_EN].offset, g_adsp_info.dsp_sysctrl[BBE_CORE_CLK_EN].start_bit, g_adsp_info.dsp_sysctrl[BBE_CORE_CLK_EN].end_bit);
    }
    bbe16_tcm_addr = (u32)ioremap_nocache(BBE_TCM_ADDR, 2*BBE_TCM_SIZE);
    if (NULL == (void*)bbe16_tcm_addr)
    {
        printk(KERN_ERR"fail to io remap\n");
        return -ENOMEM;
    }

    oldfs = get_fs();
    set_fs(KERNEL_DS);

    handle = (unsigned int)sys_open("/system/bin/v7r5_dsp.bin", O_RDONLY, 0);
    if (IS_ERR((const void*)handle))
    {
        printk(KERN_ERR"fail to open file 'v7r5_dsp.bin'\n");
        return -1;
    }

    do
    {
        ret = sys_read(handle, (char*)buffer, sizeof(buffer));
        memcpy((void*)(bbe16_tcm_addr+readlen), (void*)buffer, sizeof(buffer));
        readlen += ret;
    }while(ret == sizeof(buffer));

    sys_close(handle);
    set_fs(oldfs);

    /* BBE16 DMEM1 Æ«ÒÆ0x4000 */
	writel(DDR_TLPHY_IMAGE_ADDR, bbe16_tcm_addr + 0x4000);
    writel(1024/4, bbe16_tcm_addr+0x4004);
    writel(AXI_MEM_64_SRC_FOR_BBE16_PHY, bbe16_tcm_addr+0x4008);
    writel(AXI_MEM_FOR_MEMCPY_SIZE*2/4, bbe16_tcm_addr+0x400C);
    writel(AXI_MEM_FOR_BBE16_SRC_ADDR_PHY, bbe16_tcm_addr+0x4010);
    writel(AXI_MEM_64_SIZE_FOR_BBE16*2/4, bbe16_tcm_addr+0x4014);
    writel(0, bbe16_tcm_addr+0x4018);

    if(IS_VALID_ADDR(g_adsp_info.dsp_sysctrl[BBE16_RUNSTALL].base_addr))
    {
        hi_dsp_set_reg(0, g_adsp_info.dsp_sysctrl[BBE16_RUNSTALL].base_addr, g_adsp_info.dsp_sysctrl[BBE16_RUNSTALL].offset, g_adsp_info.dsp_sysctrl[BBE16_RUNSTALL].start_bit, g_adsp_info.dsp_sysctrl[BBE16_RUNSTALL].end_bit);
    }

    printk(KERN_INFO"OK, image length: %d\n", readlen);

    return 0;
}

s32 bbe16_stress_test_stop(void)
{
    if (bbe16_tcm_addr)
        iounmap((void*)bbe16_tcm_addr);
    bbe16_tcm_addr = 0;
    if(IS_VALID_ADDR(g_adsp_info.dsp_sysctrl[BBE16_RUNSTALL].base_addr))
    {
        hi_dsp_set_reg(1, g_adsp_info.dsp_sysctrl[BBE16_RUNSTALL].base_addr, g_adsp_info.dsp_sysctrl[BBE16_RUNSTALL].offset, g_adsp_info.dsp_sysctrl[BBE16_RUNSTALL].start_bit, g_adsp_info.dsp_sysctrl[BBE16_RUNSTALL].end_bit);
    }

    if(IS_VALID_ADDR(g_adsp_info.dsp_sysctrl[BBE16_CORE_SRST_EN].base_addr))
    {
        hi_dsp_set_reg(1, g_adsp_info.dsp_sysctrl[BBE16_CORE_SRST_EN].base_addr, g_adsp_info.dsp_sysctrl[BBE16_CORE_SRST_EN].offset, g_adsp_info.dsp_sysctrl[BBE16_CORE_SRST_EN].start_bit, g_adsp_info.dsp_sysctrl[BBE16_CORE_SRST_EN].end_bit);
    }
    if(IS_VALID_ADDR(g_adsp_info.dsp_sysctrl[BBE16_PD_SRST_EN].base_addr))
    {
        hi_dsp_set_reg(1, g_adsp_info.dsp_sysctrl[BBE16_PD_SRST_EN].base_addr, g_adsp_info.dsp_sysctrl[BBE16_PD_SRST_EN].offset, g_adsp_info.dsp_sysctrl[BBE16_PD_SRST_EN].start_bit, g_adsp_info.dsp_sysctrl[BBE16_PD_SRST_EN].end_bit);
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
