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
#include <linux/clk-provider.h>
#include <linux/clk-private.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/string.h>
#include <linux/semaphore.h>
#include <osl_thread.h>
#include <bsp_pmu.h>
#include <bsp_ipc.h>
#include <pmic_inner.h>

struct pmic_clk {
	struct  clk_hw	             hw;
	unsigned long                rate;
    u32                          en_dis_offset;
    u32                          en_dis_bit;
};



static int pmic_clk_enable(struct clk_hw *hw)
{
	struct pmic_clk *clk_pmic = container_of(hw, struct pmic_clk, hw);
    u32    reg_val = 0;
    
    (void)bsp_ipc_spin_lock(IPC_SEM_PMU);
    pmic_reg_read(clk_pmic->en_dis_offset, &reg_val);
    reg_val |= (((u32)0x1) << clk_pmic->en_dis_bit);
    pmic_reg_write(clk_pmic->en_dis_offset, reg_val);
    (void)bsp_ipc_spin_unlock(IPC_SEM_PMU); 
    
    return 0;

}

static void pmic_clk_disable(struct clk_hw *hw)
{
    struct pmic_clk *clk_pmic = container_of(hw, struct pmic_clk, hw);
    u32    reg_val = 0;
    
    (void)bsp_ipc_spin_lock(IPC_SEM_PMU);
    pmic_reg_read(clk_pmic->en_dis_offset, &reg_val);
    reg_val &= (~(((u32)0x1) << clk_pmic->en_dis_bit));
    pmic_reg_write(clk_pmic->en_dis_offset, reg_val);
    (void)bsp_ipc_spin_unlock(IPC_SEM_PMU); 
}

static struct clk_ops g_pmic_clk_ops = {
	.enable		= pmic_clk_enable,
	.disable	= pmic_clk_disable,
};

static void __init pmic_clk_setup(struct device_node *node)
{
    struct clk_init_data *init = NULL;
    struct clk *pclk = NULL;
    struct pmic_clk *clk_pmic = NULL;
    const char *clk_name;
    struct device_node *child_node = NULL;
    const char *parent_names = "tcxo";
    u32   data[2] = {0};
    
	for_each_available_child_of_node(node, child_node)
	{
        if (of_property_read_string(child_node, "clock-output-names", &clk_name)) 
        {
            pr_err("node %s doesn't have clock-output-name property!\n", child_node->name);
            return ;
        }

        if (of_property_read_u32_array(child_node, "reg_offset_bit", &data[0], 2)) 
        {
            pr_err("node %s doesn't have reg_offset_bit property!\n", child_node->name);
            return ;
        }
        
        pr_err("clk_name %s\n", clk_name);
        clk_pmic = kzalloc(sizeof(struct pmic_clk), GFP_KERNEL);
        if (!clk_pmic) 
        {
            pr_err("[%s] fail to alloc clk_pmic!\n", __func__);
            goto err_pmic_clk;
        }
        
        init = kzalloc(sizeof(struct clk_init_data), GFP_KERNEL);
        if (!init) 
        {
            pr_err("clk_register_ops malloc init err\n");
            goto err_init;
        }
    
        init->name = kstrdup(clk_name, GFP_KERNEL);
        init->ops = &g_pmic_clk_ops;
        init->flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED;
        init->parent_names = &parent_names;
        init->num_parents = 1;
    
        clk_pmic->hw.init = init;

        clk_pmic->en_dis_offset = data[0];
        clk_pmic->en_dis_bit = data[1];
        
        pclk = clk_register(NULL, &clk_pmic->hw);
        if (IS_ERR(pclk))
        {
            pr_err("register_ops ERR\n");
            goto out;
        }

        clk_register_clkdev(pclk, clk_name, NULL);
        of_clk_add_provider(child_node, of_clk_src_simple_get, pclk);
        continue;
    
    out:
        kfree(init);
    err_init:
        kfree(clk_pmic);
    err_pmic_clk:
        return ;
    }

    pr_err("pmic_clk setup ok\n");
    /* coverity[leaked_storage] */
    return ;     
}


void  pmic_clk_init(void)
{
    struct device_node *node = NULL;

    /* ½Úµã */
    node = of_find_compatible_node(NULL, NULL, "hisilicon,pmic_clk_app");
    if (!node)
    {
        pr_err("find pmic_clk_app err\n");
        return ;
    }

    pmic_clk_setup(node);
}


