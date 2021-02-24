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
/*lint --e{537 } */
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/suspend.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_device.h>
#include <linux/of_address.h>

#include <osl_bio.h>
#include <osl_irq.h>
#include <osl_sem.h>
#include <bsp_ipc.h>
#include <bsp_sram.h>
#include <bsp_version.h>
#include <bsp_pmu.h>
#include "regulator_balong.h"

/*lint --e{34} */
struct device_node *re_node = NULL;
LIST_HEAD(regulator_pmic_list);
static inline void regulator_pm_om_log(struct regulator_dev *rdev, int opsid)
{
	int regulator_id = 0;
	struct regulator_pm_om_log regu_log = {opsid, 0, rdev->use_count, current->pid};
	struct regulator_type_map * regulato_map = NULL;
	
	regulator_id = rdev_get_id(rdev);
	regulato_map = (struct regulator_type_map *)(rdev->reg_data);
	regu_log.regulator_id = regulator_id;
	memcpy(regu_log.pmutype,regulato_map->regulator_type, sizeof(char)*(PMU_TYPE_LEN - 1));
	regu_log.pmutype[(PMU_TYPE_LEN - 1)]= 0;
	bsp_pm_log_type(PM_OM_REGU, REGULATOR_TYPE, sizeof(struct regulator_pm_om_log), &regu_log);
}


int balong_regulator_is_enabled(struct regulator_dev *rdev)
{
	int ret = 0;
	int regulator_id = 0;
	struct regulator_type_map * regulato_map = NULL;
	
	regulator_id = rdev_get_id(rdev);
	regulato_map = (struct regulator_type_map *)(rdev->reg_data);
	if (regulato_map->ops && regulato_map->ops->is_enabled)
	{
		ret = regulato_map->ops->is_enabled(regulator_id);
	}
	else
	{
		ret = regulato_map->is_enabled_flag;
	}
	return ret;
}

int balong_regulator_enable(struct regulator_dev *rdev)
{
	int ret = 0;
	int regulator_id = 0;
	struct regulator_type_map * regulato_map = NULL;
	regulator_id = rdev_get_id(rdev);

	regulato_map = (struct regulator_type_map *)(rdev->reg_data);
	
	if (regulato_map->ops && regulato_map->ops->enable)
	{
		/*lint --e{737, 713 } */
		regu_pr_debug("balong enable id:%d pmic type:%s\n", regulator_id, regulato_map->regulator_type);
		ret = regulato_map->ops->enable(regulator_id);
	}
	else
	{
		regu_pr_info("ops enable not exist\n");
	}
	regulato_map->is_enabled_flag = 1;
	
	regulator_pm_om_log(rdev, REGULATOR_OPS_ENABLE);

	return ret;
}
int balong_regulator_disable(struct regulator_dev *rdev)
{

	int ret = 0;
	int regulator_id = 0;
	struct regulator_type_map * regulato_map = NULL;
	regulator_id = rdev_get_id(rdev);

	regulato_map = (struct regulator_type_map *)(rdev->reg_data);
	
	if (regulato_map->ops && regulato_map->ops->disable)
	{
		/*lint --e{737, 713 } */
		regu_pr_debug("balong enable id:%d pmic type:%s\n", regulator_id, regulato_map->regulator_type);
		ret = regulato_map->ops->disable(regulator_id);
	}
	else
	{
		regu_pr_info("ops enable not exist\n");
	}

	regulato_map->is_enabled_flag = 0;
	
	regulator_pm_om_log(rdev, REGULATOR_OPS_DISABLE);

	return ret;
}

int balong_regulator_get_voltage(struct regulator_dev *rdev)
{
	int ret = 0;
	int regulator_id = 0;
	struct regulator_type_map * regulato_map = NULL;
	regulator_id = rdev_get_id(rdev);

	regulato_map = (struct regulator_type_map *)(rdev->reg_data);

	if (regulato_map->ops && regulato_map->ops->get_voltage)
	{
		/*lint --e{737, 713 } */
		regu_pr_debug("balong enable id:%d pmic type:%s\n", regulator_id, regulato_map->regulator_type);
		ret = regulato_map->ops->get_voltage(regulator_id);
	}
	else
	{
		regu_pr_info("ops get_voltage not exist\n");
	}
	return ret;
}
int balong_regulator_set_voltage(struct regulator_dev *rdev, int min_uV, int max_uV, unsigned *selector)
{
	int ret = 0;
	int regulator_id = 0;
	struct regulator_type_map * regulato_map = NULL;
	regulator_id = rdev_get_id(rdev);

	regulato_map = (struct regulator_type_map *)(rdev->reg_data);

	if (regulato_map->ops && regulato_map->ops->set_voltage)
	{
		/*lint --e{737, 713 } */
		regu_pr_debug("balong enable id:%d pmic type:%s\n", regulator_id, regulato_map->regulator_type);
		ret = regulato_map->ops->set_voltage(regulator_id, min_uV, max_uV, selector);
	}
	else
	{
		regu_pr_info("ops set_voltage not exist\n");
	}
	return ret;
}
static int balong_regulator_list_voltage(struct regulator_dev *rdev, unsigned selector)
{	
	int ret = 0;
	int regulator_id = 0;
	struct regulator_type_map * regulato_map = NULL;
	regulator_id = rdev_get_id(rdev);

	regulato_map = (struct regulator_type_map *)(rdev->reg_data);

	if (regulato_map->ops && regulato_map->ops->list_voltage)
	{
		/*lint --e{737, 713 } */
		regu_pr_debug("balong enable id:%d pmic type:%s\n", regulator_id, regulato_map->regulator_type);
		ret = regulato_map->ops->list_voltage(regulator_id, selector);
	}
	else
	{
		regu_pr_info("ops list_voltage not exist\n");
	}
	return ret;
}
int balong_regulator_set_current(struct regulator_dev *rdev, int min_uA, int max_uA)
{
	int ret = 0;
	int regulator_id = 0;
	unsigned int selector = 0;
	struct regulator_type_map * regulato_map = NULL;
	regulator_id = rdev_get_id(rdev);

	regulato_map = (struct regulator_type_map *)(rdev->reg_data);

	if (regulato_map->ops && regulato_map->ops->set_current)
	{
		/*lint --e{737, 713 } */
		regu_pr_debug("balong enable id:%d pmic type:%s\n", regulator_id, regulato_map->regulator_type);
		ret = regulato_map->ops->set_current(regulator_id, min_uA, max_uA, &selector);
	}
	else
	{
		regu_pr_info("ops set current not exist\n");
	}
	return ret;
}
int balong_regulator_get_current(struct regulator_dev *rdev)
{
	int ret = 0;
	int regulator_id = 0;
	struct regulator_type_map * regulato_map = NULL;
	regulator_id = rdev_get_id(rdev);

	regulato_map = (struct regulator_type_map *)(rdev->reg_data);

	if (regulato_map->ops && regulato_map->ops->get_current_limit)
	{
		/*lint --e{737, 713 } */
		regu_pr_debug("balong enable id:%d pmic type:%s\n", regulator_id, regulato_map->regulator_type);
		ret = regulato_map->ops->get_current_limit(regulator_id);
	}
	else
	{
		regu_pr_info("ops get_current not exist\n");
	}
	return ret;
}
unsigned int balong_regulator_get_mode(struct regulator_dev *rdev)
{
	unsigned int ret = 0;
	int regulator_id = 0;
	struct regulator_type_map * regulato_map = NULL;
	regulator_id = rdev_get_id(rdev);

	regulato_map = (struct regulator_type_map *)(rdev->reg_data);

	if (regulato_map->ops && regulato_map->ops->get_mode)
	{
		/*lint --e{737, 713 } */
		regu_pr_debug("balong enable id:%d pmic type:%s\n", regulator_id, regulato_map->regulator_type);
		ret = regulato_map->ops->get_mode(regulator_id);
	}
	else
	{
		regu_pr_info("ops get_mode not exist\n");
	}
	return ret;
}
int balong_regulator_set_mode(struct regulator_dev *rdev,unsigned int mode)
{
	unsigned int ret = 0;
	int regulator_id = 0;
	struct regulator_type_map * regulato_map = NULL;
	regulator_id = rdev_get_id(rdev);

	regulato_map = (struct regulator_type_map *)(rdev->reg_data);

	if (regulato_map->ops && regulato_map->ops->set_mode)
	{
		/*lint --e{737, 713 } */
		regu_pr_debug("balong enable id:%d pmic type:%s\n", regulator_id, regulato_map->regulator_type);
		ret = regulato_map->ops->set_mode(regulator_id, mode);
	}
	else
	{
		regu_pr_info("ops set_mode not exist\n");
	}
	return ret;
}
struct regulator_ops balong_regulator_ops = {
	.is_enabled = balong_regulator_is_enabled,
	.enable = balong_regulator_enable,
	.disable = balong_regulator_disable,
	.get_voltage = balong_regulator_get_voltage,
	.list_voltage = balong_regulator_list_voltage,
	.set_voltage = balong_regulator_set_voltage,
	.set_current_limit	= balong_regulator_set_current,
	.get_current_limit	= balong_regulator_get_current,
	.get_mode =balong_regulator_get_mode,
	.set_mode = balong_regulator_set_mode,
};
int regulator_init_consumers(struct regulator_init_data *regulators_init, struct device_node *regulator_node)
{
	int ret = 0;
	int i = 0;
	regulators_init->consumer_supplies = (struct regulator_consumer_supply *)kmalloc(sizeof(struct regulator_consumer_supply) * (regulators_init->num_consumer_supplies), GFP_KERNEL);
	if (NULL == regulators_init->consumer_supplies)
	{
		regu_pr_err("consumer_supplies malloc failed  %d\n", regulators_init->consumer_supplies);
		return ERROR;
	}
	for (i = 0; i < regulators_init->num_consumer_supplies; i++)
	{
		regulators_init->consumer_supplies[i].dev_name = NULL;
		ret = of_property_read_string_index(regulator_node, "hisilicon,hisi_regulator_consumer", i, &regulators_init->consumer_supplies[i].supply);
		if (ret != 0) {
			regu_pr_err("failed to get regulator consumers resource! ret=%d.i:%d\n", ret, i);
			continue;
		}
	}
	return OK;
}

struct regulator_id_ops *regulator_get_pmic_ops(char *regulator_pmic_type)
{
	unsigned long irqlock = 0;
	struct regulator_type_map *node = NULL;
	struct regulator_id_ops *regu_ops = NULL;
	if (NULL == regulator_pmic_type)
	{
		regu_pr_err("argc is null check it\n");
		return regu_ops;
	}
	local_irq_save(irqlock);
	list_for_each_entry(node, &regulator_pmic_list, list)
	{
		if (!strcmp(node->regulator_type, regulator_pmic_type))
		{
			regu_ops = node->ops;
			break;
		}
	}
	local_irq_restore(irqlock);
	return regu_ops;
}
EXPORT_SYMBOL_GPL(regulator_get_pmic_ops);

int regulator_pmic_ops_register(struct regulator_id_ops *pmic_ops, char *pmic_type)
{
	int len = 0;
	unsigned long irqlock = 0;
	struct regulator_type_map *regulator_type_l = NULL;
	struct regulator_type_map *regulator_pmic_type = NULL;
	if (NULL == pmic_type || NULL == pmic_ops)
	{
		regu_pr_err("argc is NULL check it\n");
		return ERROR;
	}
	regulator_pmic_type = (struct regulator_type_map *)kmalloc(sizeof(struct regulator_type_map), GFP_KERNEL);
	if (NULL == regulator_pmic_type)
	{
		regu_pr_err("regulator_type_map malloc failed %d\n", regulator_pmic_type);
		return ERROR;
	}
	len = strlen(pmic_type);
	regulator_pmic_type->regulator_type = (char *)kmalloc(len+1, GFP_KERNEL);
	if (NULL == regulator_pmic_type->regulator_type)
	{
		regu_pr_err("regulator_type malloc failed %d\n", regulator_pmic_type->regulator_type);
		kfree(regulator_pmic_type);
		return ERROR;
	}
	
	local_irq_save(irqlock);
	list_for_each_entry(regulator_type_l, &regulator_pmic_list, list)
	{
		if (!strcmp(regulator_type_l->regulator_type, pmic_type))
		{
			regu_pr_err("we have the same pmic_type (%s),check dts or your pmic name\n", pmic_type);
			kfree(regulator_pmic_type->regulator_type);		
			kfree(regulator_pmic_type);
			goto pmic_err;
		}
	}

	INIT_LIST_HEAD(&regulator_pmic_type->list);
	strncpy(regulator_pmic_type->regulator_type, pmic_type, len);
	regulator_pmic_type->regulator_type[len] = '\0';
	regulator_pmic_type->ops = pmic_ops;

	list_add(&regulator_pmic_type->list, &regulator_pmic_list);
pmic_err:
	local_irq_restore(irqlock);

	return OK;	
}
EXPORT_SYMBOL_GPL(regulator_pmic_ops_register);


static void regulator_init_desc(struct regulator_desc *regulators_desc, struct device_node *regulator_node)
{
	int ret = 0;
	memset(regulators_desc, 0, sizeof(struct regulator_desc));
	regulators_desc->name = of_get_property(regulator_node, "regulator_name", NULL);
	ret = of_property_read_u32_array(regulator_node, "hisilicon,hisi_regulator_id", &regulators_desc->id, 1);
	if(ret)
	{
		regu_pr_err("get regulator is from dts failed, %s\n", regulators_desc->name);
	}
	ret = of_property_read_u32_array(regulator_node, "regulator_type", &regulators_desc->type, 1);
	if(ret)
	{
		regu_pr_err("get regulator type from dts failed, %s\n", regulators_desc->name);
	}
	ret = of_property_read_u32_array(regulator_node, "hisilicon,hisi_regulator_nvoltage", &regulators_desc->n_voltages, 1);
	if(ret)
	{
		regulators_desc->n_voltages = 0;
	}

	regulators_desc->owner = THIS_MODULE;
	regulators_desc->ops = &balong_regulator_ops;
	return ;
}

static void regulator_config_init_data(struct regulator_init_data *regulators_init, struct device_node *regulator_node)
{
	int ret = 0;
	int regulator_type = 0;
	int regulator_uV[2] = {0};
	
	memset(regulators_init, 0, sizeof(struct regulator_init_data));
	regulators_init->constraints.name = of_get_property(regulator_node, "regulator_name", NULL);
	(void)of_property_read_u32_array(regulator_node, "regulator_type", &regulator_type, 1);
	ret = of_property_read_u32_array(regulator_node, "hisilicon,hisi_regulator_voltage", regulator_uV, 2);
	if (!ret)
	{
		regulators_init->constraints.min_uV = regulator_uV[0];
		regulators_init->constraints.max_uV = regulator_uV[1];
	}
	ret = of_property_read_u32_array(regulator_node, "hisilicon,hisi_regulator_current", regulator_uV, 2);
	if (!ret)
	{
		regulators_init->constraints.min_uA = regulator_uV[0];
		regulators_init->constraints.max_uA = regulator_uV[1];
	}
	ret = of_property_read_u32_array(regulator_node, "hisilicon,hisi_regulator_ops_mask", &regulators_init->constraints.valid_ops_mask, 1);
	if (!ret)
	{
		regulators_init->supply_regulator = of_get_property(regulator_node, "regulator_supply", NULL);
		regulators_init->num_consumer_supplies = of_property_count_strings(regulator_node, "hisilicon,hisi_regulator_consumer");
		if (0 > regulators_init->num_consumer_supplies)
			regulators_init->num_consumer_supplies = 0;
	}
	ret = regulator_init_consumers(regulators_init, regulator_node);
	if (ret)
	{
		regu_pr_err("init_consumers failed\n");
		return;
	}
	return;
}

static void regualtor_init_pmic_map(struct regulator_type_map *regulator_pmic_map, struct device_node *regulator_node)
{
	memset(regulator_pmic_map, 0, sizeof(struct regulator_type_map));
	regulator_pmic_map->regulator_type = (char *)of_get_property(regulator_node, "hisilicon,hisi_pmic_type", NULL);
	regulator_pmic_map->ops = regulator_get_pmic_ops(regulator_pmic_map->regulator_type);
	if (NULL == regulator_pmic_map->ops)
	{
		regu_pr_info("get <%s> pmic ops failed  use dummy ops\n", regulator_pmic_map->regulator_type);
		regulator_pmic_map->ops = regulator_get_pmic_ops("dummy_pmic");
	}
	regulator_pmic_map->is_enabled_flag = 0;
	return;
}

extern void regulator_pmic_dummy_init(void);

struct device_node *regu_node = NULL;
static int balong_regulator_probe(struct platform_device *pdev)
{
	struct regulator_config config = {0};
	struct regulator_dev *regualtor_info = NULL;
	struct device_node *regu_node_child = NULL;
	struct regulator_desc *regulators_desc = NULL;
	struct regulator_init_data *regulators_init = NULL;
	struct regulator_type_map *regulator_pmic_map = NULL;

	config.dev = &pdev->dev;
	regu_node = of_find_compatible_node(NULL, NULL, "hisilicon,regulator_balong");
	if (!regu_node)
	{
		regu_pr_err("regulator dts node not found!  %s\n");
		return -1;
	}
	regulator_pmic_dummy_init();
	for_each_child_of_node(regu_node, regu_node_child)
	{
		regulators_desc = (struct regulator_desc *)kmalloc(sizeof(struct regulator_desc), GFP_KERNEL);
		if (NULL == regulators_desc)
		{
			regu_pr_err("malloc regulators_desc error \n");
			continue;
		}	
		regulator_init_desc(regulators_desc, regu_node_child);
		
		regulators_init = (struct regulator_init_data *)kmalloc(sizeof(struct regulator_init_data), GFP_KERNEL);
		if (NULL == regulators_init)
		{
			regu_pr_err("malloc regulators_init error \n");
			continue;
		}
		regulator_config_init_data(regulators_init, regu_node_child);

		regulator_pmic_map = (struct regulator_type_map *)kmalloc(sizeof(struct regulator_type_map), GFP_KERNEL);
		if (NULL == regulator_pmic_map)
		{
			regu_pr_err("malloc regulator_pmic_map error \n");
			continue;
		}
		regualtor_init_pmic_map(regulator_pmic_map, regu_node_child);
		
		config.driver_data = regulator_pmic_map;
		config.init_data = regulators_init;
		
		regualtor_info = regulator_register(regulators_desc, &config);
		if (IS_ERR(regualtor_info))
		{
			regu_pr_err("regulator %s register failed!!%d\n", regulators_desc->name, PTR_ERR(regualtor_info));
		}
	}
	regu_pr_err("regulator register ok\n");
	return 0;
}


/*************************device and driver start****************************/
static struct platform_driver balong_regulator_driver = {
	.probe 		= balong_regulator_probe,
	.driver		= {
		.name	= "balong_regulator",
	},
};

static struct platform_device balong_regulator_device = {
	.name = "balong_regulator",
	.id = 0,
	.dev = {
		.platform_data = NULL,
	},
};
/*************************device and driver start****************************/
static int __init balong_regulator_init(void)
{
	int ret = 0;

	ret = platform_device_register(&balong_regulator_device);
	if (ret)
	{
		regu_pr_debug("register devices failed\n");
		return ret;
	}
	ret = platform_driver_register(&balong_regulator_driver);
	if (ret)
	{
		regu_pr_debug("register driver failed\n");
		platform_device_unregister(&balong_regulator_device);
		return ret;
	}

	return 0;
}
static void __exit balong_regulator_exit(void)
{
	platform_driver_unregister(&balong_regulator_driver);
	platform_device_unregister(&balong_regulator_device);
}

subsys_initcall(balong_regulator_init);
module_exit(balong_regulator_exit);

/***************************************test code***************************************/
extern struct list_head regulator_list;


void regulator_print_status(void)
{
	int ret = 0;
	struct regulator_dev *rdev = NULL;
    list_for_each_entry(rdev, &regulator_list, list)
    {
    	ret = 0;
    	if (rdev->desc->ops->is_enabled)
    		ret = rdev->desc->ops->is_enabled(rdev);
    	regu_pr_err("name : %s\n", rdev->desc->name);
		regu_pr_err("status: %d\n", ret);
		regu_pr_err("count : %d\n", rdev->use_count);
		if (rdev->desc->ops->get_voltage)
			/*lint --e{737, 713 } */
			ret = rdev->desc->ops->get_voltage(rdev);
		regu_pr_err("volt : %d\n", ret);
    }
    regu_pr_err("buck3:%d\n", BUCK3_ONOFF_FLAG);
}
/*************************************************************************************/

#ifdef __cplusplus
}
#endif

