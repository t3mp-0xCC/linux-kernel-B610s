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
#include <linux/of_device.h>
#include <linux/of_address.h>

#include <of.h>
#include <of_fdt.h>
#include <osl_bio.h>
#include <osl_irq.h>
#include <osl_sem.h>
#include <bsp_ipc.h>
#include <bsp_sram.h>
#include <bsp_version.h>
#include <bsp_pmu.h>
#include "regulator_balong.h"


extern struct list_head regulator_list;
struct regulator *test_regulator = NULL;
void regulator_get_test(int ch)
{
	switch (ch)
	{
		case 0:
			test_regulator = regulator_get(NULL, "dwc3_usb-vcc");
			break;
		case 1:
			test_regulator = regulator_get(NULL, "pcie_mtcmos-vcc");
			break;
		default:
			break;
	}
}
void regulator_enable_test(void)
{
	int ret = 0;
	ret = regulator_enable(test_regulator);
	if (ret)
	{
		regu_pr_err("enable error\n");
	}
}
void regulator_disable_test(void)
{
	(void)regulator_disable(test_regulator);
}

/*用于自动化测试:
	regulator_get:正确值，错误值
	regulator_enable disable:需要使能父，不需要使能父
	regulator get set voltage:设置正确的，错误的电压，获取电压
	regulator set get current:设置获取电流同电压
	regulator set get mode:设置获取模式，该接口暂不支持	
*/
struct device_node *re_node_rest = NULL;
#define REGULATOR_TEST_VOL_NUM (5)
void regulator_test(void)
{
	int ret = 0, i = 0;
	int test_vol[REGULATOR_TEST_VOL_NUM] = {0};
	char *test_regu = NULL;
	re_node_rest = of_find_compatible_node(NULL, NULL, "regulator_balong_test");
	if (!re_node_rest)
	{
		regu_pr_err("regulator test dts node not found!  %s\n");
		return;
	}
	ret = of_property_read_u32_array(re_node_rest, "hisilicon,hisi_regulator_voltage_test", test_vol, REGULATOR_TEST_VOL_NUM);
	if(IS_ERR(ret))
	{
		regu_pr_err("get hisi_regulator_voltage_test from dts failed\n");
		return;
	}

	test_regu = of_get_property(re_node_rest, "hisilicon,hisi_regulator_consumer", NULL);
	
	test_regulator = regulator_get(NULL, test_regu);
	if (IS_ERR(test_regulator))
	{
		regu_pr_info("regulator get err\n");
		return;
	}
	ret = regulator_enable(test_regulator);
	if (ret)
	{
		regu_pr_info("1enable err\n");
		return;
	}
	ret = regulator_is_enabled(test_regulator);
	if (!ret)
	{
		regu_pr_info("[enable]is enabled return %d\n", ret);
		return;
	}
	ret = regulator_enable(test_regulator);
	if (ret)
	{
		regu_pr_info("2enable err\n");
		return;
	}
	ret = regulator_disable(test_regulator);
	if (ret)
	{
		regu_pr_info("1disable err\n");
		return;
	}
	ret = regulator_is_enabled(test_regulator);
	if (!ret)
	{
		regu_pr_info("[disable]is enabled return %d\n", ret);
		return;
	}
	ret = regulator_disable(test_regulator);
	if (ret)
	{
		regu_pr_info("2disable err\n");
		return;
	}
	ret = regulator_is_enabled(test_regulator);
	if (ret)
	{
		regu_pr_info("[disable]is enabled return %d\n", ret);
		return;
	}
	for (i = 0; i < REGULATOR_TEST_VOL_NUM; i++)
	{
		ret = regulator_set_voltage(test_regulator, test_vol[i], test_vol[i]);
		if ((i < 3) && IS_ERR(ret))
		{
			regu_pr_err("set voltage failed %d, vol:%d\n", i, test_vol[i]);
			return;
		}
		if ((i >=3) && !IS_ERR(ret))
		{
			regu_pr_err("set voltage failed %d, vol:%d\n", i, test_vol[i]);
			return;
		}
	}
	ret = regulator_get_voltage(test_regulator);
	
	
}

static int test_enable(int volt_id)
{
	regu_pr_info("dummy_enable id:%d \n", volt_id);
	return 0;
}
static int test_disable(int volt_id)
{
	regu_pr_info("dummy_disable id:%d \n", volt_id);
	return 0;
}
extern struct regulator_id_ops *regulator_get_pmic_ops(char *regulator_pmic_type);
void test_regulator_get_set_pmic(void)
{
	char * test_pmic_type = "test_pmic";
	struct regulator_id_ops *test_ops = NULL;
	struct regulator_id_ops pmic_ops_test = {0};
	pmic_ops_test.enable = test_enable;
	pmic_ops_test.disable = test_disable;
	(void)regulator_pmic_ops_register(NULL, NULL);
	(void)regulator_pmic_ops_register(&pmic_ops_test, test_pmic_type);
	test_ops = regulator_get_pmic_ops(test_pmic_type);
	if (NULL == test_ops)
	{
		return;
	}
}

#ifdef __cplusplus
}
#endif


