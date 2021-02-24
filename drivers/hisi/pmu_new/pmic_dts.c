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

#include <osl_malloc.h>
#include <osl_err.h>
#include <osl_spinlock.h>
#include <of.h>
#include "pmic_inner.h"
#include "pmic_volt.h"
#include "pmic_dr.h"


extern struct pmic_volt_info g_pmic_voltinfo;
extern struct pmic_dr_info g_pmic_drinfo;

typedef void (*of_pmu_init_cb_t)(struct device_node *);

void  pmic_volt_common_setup(struct device_node *dev_node , u32 id)
{
    struct pmic_volt_ctrl *volt_ctrl;
    u32 reg_size = sizeof(struct pmic_volt_reg)/sizeof(u32);/*dts 每个寄存器子节点的大小*/

    volt_ctrl = (struct pmic_volt_ctrl *)&(g_pmic_voltinfo.volt_ctrl[id]);

    (void)of_property_read_string_index(dev_node,"volt_name",0,&volt_ctrl->name);
    (void)of_property_read_u32_array(dev_node, "reg_ctrl",(u32*)&(volt_ctrl->volt_reg),reg_size);
    (void)of_property_read_u32_index(dev_node, "off_on_delay",0,&volt_ctrl->off_on_delay);

    spin_lock_init(&g_pmic_voltinfo.volt_ctrl[id].volt_lock);
}
void  pmic_volt_table_setup(struct device_node *dev_node)
{
    int ret = BSP_OK;
    struct device_node *child_node = NULL;
    struct pmic_volt_ctrl *volt_ctrl = NULL;
    u32 id = 0;
    u32 table_size = 0;

    for_each_child_of_node(dev_node, child_node)
    {
        ret = of_property_read_u32_index(child_node,"volt_id",0,&id);
        if (ret) {
            pmic_print_error("get volt_id info from dts failed!\n");
            return;
        };
        volt_ctrl = &g_pmic_voltinfo.volt_ctrl[id];
        volt_ctrl->id = id;
        volt_ctrl->voltage_flag = VOLT_TABLE;

        pmic_volt_common_setup(child_node,id);
        ret = of_property_read_u32_index(child_node, "voltage_nums",0,(u32*)&(volt_ctrl->voltage_nums));
        if(ret)
        {
            pmic_print_error("get voltage_nums info from dts failed!\n");
        }
        table_size = sizeof(u32)*(volt_ctrl->voltage_nums);
        volt_ctrl->voltage_table = osl_malloc(table_size);
        if(IS_ERR(volt_ctrl->voltage_table))
        {
            pmic_print_error("volt_id %d malloc voltage table failed!\n",id);
            return;
        }
        (void)memset((void *)volt_ctrl->voltage_table,0,table_size);

        ret = of_property_read_u32_array(child_node, "voltage_table",
                        (u32 *)(volt_ctrl->voltage_table), volt_ctrl->voltage_nums);
        if (ret) {
            pmic_print_error("volt_id %d get table info from dts failed,errid %d!\n",id,ret);
            return;
        };
    };
}
void  pmic_volt_linear_setup(struct device_node *dev_node)
{
    int ret = BSP_OK;
    struct device_node *child_node = NULL;
    struct pmic_volt_ctrl *volt_ctrl;
    u32 id = 0;

    for_each_child_of_node(dev_node, child_node)
    {
        ret = of_property_read_u32_index(child_node,"volt_id",0,&id);
        if (ret) {
            pmic_print_error("get volt_id info from dts failed!\n");
            return;
        };
        volt_ctrl = &g_pmic_voltinfo.volt_ctrl[id];
        volt_ctrl->id = id;
        volt_ctrl->voltage_flag = VOLT_LINEAR;

        pmic_volt_common_setup(child_node,id);
        ret = of_property_read_u32_index(child_node, "voltage_nums",0,(u32*)&(volt_ctrl->voltage_nums));
        ret |= of_property_read_u32_index(child_node, "voltage_base_step",0,&(volt_ctrl->voltage_base));
        ret |= of_property_read_u32_index(child_node, "voltage_base_step",1,&(volt_ctrl->voltage_step));

        if (ret) {
            pmic_print_error("get linear info from dts failed!\n");
            return;
        };
    };
}
void  pmic_volt_const_setup(struct device_node *dev_node)
{
    int ret = BSP_OK;
    struct device_node *child_node = NULL;
    struct pmic_volt_ctrl *volt_ctrl;
    u32 id = 0;

    for_each_child_of_node(dev_node, child_node)
    {
        ret = of_property_read_u32_index(child_node,"volt_id",0,&id);
        if (ret) {
            pmic_print_error("get volt_id info from dts failed\n");
            return;
        };
        volt_ctrl = &g_pmic_voltinfo.volt_ctrl[id];
        volt_ctrl->id = id;
        volt_ctrl->voltage_flag = VOLT_CONST;

        pmic_volt_common_setup(child_node,id);
        ret = of_property_read_u32_index(child_node, "voltage_nums",0,(u32*)&(volt_ctrl->voltage_nums));
        ret |= of_property_read_u32_index(child_node, "voltage_const",0,&(volt_ctrl->voltage_const));

        if (ret) {
            pmic_print_error("get const info from dts failed\n");
            return;
        };
    };
}

static const struct of_device_id g_pmic_of_device_init[] =
{
        { .compatible = "hisilicon,pmic_volt_table",    .data = pmic_volt_table_setup, },
        { .compatible = "hisilicon,pmic_volt_linear", .data = pmic_volt_linear_setup, },
        { .compatible = "hisilicon,pmic_volt_const",     .data = pmic_volt_const_setup, },
        {},
};
void  of_pmic_init(const struct of_device_id *matches)
{
    struct device_node *np;
    const struct of_device_id *match = NULL;
    of_pmu_init_cb_t pmu_init_cb;
    if (!matches)
    {
        return ;
    }

    for_each_matching_node(np, matches)
    {
        if (!(strcmp("pmic_lastnode", np->name)))
        {
            break;
        }
        match = of_match_node(matches, np);
        pmu_init_cb = match->data;/*lint !e158*/
        pmu_init_cb(np);
    }

    return ;
}
void pmic_volt_dts_init(void)
{
    struct device_node *dev_node = NULL;
    int ret = BSP_OK;
    u32 volt_num = 0;
    u32 attr_size = 0;

    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,pmic_volt_app");
    if(NULL == dev_node)
    {
        pmic_print_error("pmic device node not found!\n");
        return;
    }
    /*读取电源总数*/
    ret = of_property_read_u32_index(dev_node,"pmic_volt_num",0,(u32*)&g_pmic_voltinfo.volt_min);
    ret |= of_property_read_u32_index(dev_node,"pmic_volt_num",1,(u32*)&g_pmic_voltinfo.volt_max);
    if(ret)
    {
        pmic_print_error("read pmic_volt_num from dts failed,err_id %d!\n",ret);
        return;
    }
    volt_num = (g_pmic_voltinfo.volt_max - g_pmic_voltinfo.volt_min + 1);
    attr_size = sizeof(struct pmic_volt_ctrl)*volt_num;
    g_pmic_voltinfo.volt_ctrl= (struct pmic_volt_ctrl *)osl_malloc(attr_size);
    if(IS_ERR(g_pmic_voltinfo.volt_ctrl))
    {
        pmic_print_error("pmic malloc voltage ctrl failed!\n");
        return;
    }
    (void)memset((void *)g_pmic_voltinfo.volt_ctrl,0,attr_size);

    of_pmic_init(g_pmic_of_device_init);

}

