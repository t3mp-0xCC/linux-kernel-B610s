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

/*lint --e{537,958}*/
#include <of.h>
#include <product_config.h>
#include <osl_malloc.h>
#include <osl_err.h>
#include <bsp_pmu.h>
#include <pmic_inner.h>
#include <pmic_ioshare.h>
#include <pmic_comm.h>
#include <bsp_om.h>

#define reg_data_width 8

struct pmic_ioshare_info g_pmic_ioshare_info;

int pmic_ioshare_status_get(pmic_ioshare_e id)
{
    pmic_print_error("error, this product don't have this api,please check\n");
    return -1;
}

static inline void pmic_ioshare_set(void)
{
    u32 index = 0;
    for(index = 0; index < g_pmic_ioshare_info.ioshare_reg_num; index++)
    {
        pmic_reg_write(g_pmic_ioshare_info.ioshare_reg[index].addr, g_pmic_ioshare_info.ioshare_reg[index].data);
    }
}

void pmic_ioshare_init_show(void)
{
    u32 index;
    u32 reg_data;
    u32 reg_addr;
    int ret;
    pmic_print_error("ioshare reg:\n");
    for(index = 0; index < g_pmic_ioshare_info.ioshare_reg_num; index++)
    {
        reg_addr = g_pmic_ioshare_info.ioshare_reg[index].addr;
        reg_data = g_pmic_ioshare_info.ioshare_reg[index].data;
        pmic_print_error("reg %d addr:0x%x, data:0x%x!\n", index, reg_addr, reg_data);
    }
    pmic_print_error("ioshare info:\n");
    for(index = 1; index < PMIC_IOSHARE_E_MAX; index++)
    {
        ret = pmic_ioshare_status_get(index);
        pmic_print_error("id:%d,name:%s,ioshare:%d!\n", index, g_pmic_ioshare_info.ioshare_name[index], ret);
    }

}
/*****************************************************************************
 函 数 名  : pmic_ioshare_init
 功能描述  : PMIC pmic PMU模块初始化
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
void pmic_ioshare_init(void)
{
    struct device_node *dev_node = NULL;
    struct device_node *child_node = NULL;
    u32 reg_count = 0;
    u32 ioshare_id = 0;
    u32 ret = 0;
    u32 i = 0;

    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,pmic_ioshare_reg");
    if(NULL == dev_node)
    {
        pmic_print_error("pmic device node not found!\n");
        return;
    }

    reg_count = of_get_child_count(dev_node);
    g_pmic_ioshare_info.ioshare_reg_num = reg_count;
    g_pmic_ioshare_info.ioshare_reg = (struct ioshare_reg *)osl_malloc(sizeof(struct ioshare_reg)*reg_count);
    if(g_pmic_ioshare_info.ioshare_reg == NULL)
    {
        pmic_print_error("malloc ioshare reg failed\n");
        return;
    }

    i = 0;
    for_each_child_of_node(dev_node, child_node)
    {
        ret = of_property_read_u32_array(child_node, "ioshare_reg",&(g_pmic_ioshare_info.ioshare_reg[i++].addr), \
            (sizeof(struct ioshare_reg)/sizeof(u32)));
        if(ret)
        {
            pmic_print_error("pmic get ioshare reg reg from dts failed!\n");
            goto out_reg;
        }
    }

    dev_node = NULL;
    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,pmic_ioshare_info");
    if(NULL == dev_node)
    {
        pmic_print_error("pmic device node not found!\n");
        goto out_reg;
    }

    reg_count = of_get_child_count(dev_node);
    g_pmic_ioshare_info.ioshare_num = reg_count;
    g_pmic_ioshare_info.ioshare_info = (struct ioshare_info *)osl_malloc(sizeof(struct ioshare_info)*PMIC_IOSHARE_E_MAX);
    if(g_pmic_ioshare_info.ioshare_info == NULL)
    {
        pmic_print_error("malloc ioshare info failed\n");
        goto out_reg;
    }
    memset(g_pmic_ioshare_info.ioshare_info, 0, sizeof(struct ioshare_info)*PMIC_IOSHARE_E_MAX);

    g_pmic_ioshare_info.ioshare_name = (char**)osl_malloc(sizeof(char*)*PMIC_IOSHARE_E_MAX);
    if(g_pmic_ioshare_info.ioshare_name == NULL)
    {
        pmic_print_error("malloc ioshare name failed\n");
        goto out_info;
    }
    for_each_child_of_node(dev_node, child_node)
    {
        ret = of_property_read_u32(child_node, "ioshare_id",&ioshare_id);
        if(ret)
        {
            pmic_print_error("pmic get ioshare id from dts failed!\n");
            goto out;
        }
        ret = of_property_read_string(child_node, "ioshare_name", \
            (const char**)&g_pmic_ioshare_info.ioshare_name[ioshare_id]);
        if(ret)
        {
            pmic_print_error("pmic get ioshare id from dts failed!\n");
            goto out;
        }
        ret = of_property_read_u32_array(child_node, "ioshare_info", \
            &g_pmic_ioshare_info.ioshare_info[ioshare_id].addr, (sizeof(struct ioshare_info)/sizeof(u32)));
        if(ret)
        {
            pmic_print_error("pmic get ioshare info from dts failed!\n");
            goto out;
        }
    }

    pmic_ioshare_set();

    pmic_print_error("pmic ioshare init ok!\n");
    return;
out:
    kfree(g_pmic_ioshare_info.ioshare_name);
out_info:
    kfree(g_pmic_ioshare_info.ioshare_info);
out_reg:
    kfree(g_pmic_ioshare_info.ioshare_reg);
    return;

}
