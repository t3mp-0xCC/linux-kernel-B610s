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

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <hi_vic.h>
#include <osl_module.h>
#include <osl_types.h>
#include <osl_bio.h>
#include <osl_irq.h>
#include <bsp_vic.h>
#include <bsp_om.h>
#include "vic_balong.h"


#define VIC_PRINT(fmt, ...)  bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_VIC, "[VIC]: <%s> "fmt"\n", __FUNCTION__, ##__VA_ARGS__)


bsp_vic_st gstVicInfo[BALONG_VIC_NUM];



OSL_IRQ_FUNC(irqreturn_t, bsp_vic_inthandler, intLvl, dev)
{
	int newLevel = 0;
	u32 isr = 0;
	bsp_vic_st* pVic = (bsp_vic_st*)dev;

	isr = readl((unsigned long)pVic->vic_virt_base_addr + VIC_OFFSET_STATUS_FINAL_IRQ);

	if(0 == isr)
		return IRQ_NONE;

	for(; !((isr & 0x1) && (pVic->vic_intusemask & (0x1 << newLevel))); isr = isr >> 1, ++newLevel)
		;
	if(newLevel >= INT_VIC_MAX)
		return IRQ_NONE;

	if(NULL != pVic->victable[newLevel].routine)
	{
		pVic->victable[newLevel].routine(pVic->victable[newLevel].arg);
	}

	return IRQ_HANDLED;
}

s32 __init bsp_vic_init(void)
{
	int i = 0;
	int j = 0;
	unsigned int values[5] = {0, 0, 0, 0, 0};
	struct device_node * dts_node = NULL;
	static const char * vic_name[BALONG_VIC_NUM] = {"hisilicon,arm_vic0","hisilicon,arm_vic1","hisilicon,arm_vic2","hisilicon,arm_vic3"};

	for(i = 0; i < BALONG_VIC_NUM; i++)
	{
		dts_node = of_find_compatible_node(NULL, NULL, vic_name[i]);
		if(!dts_node)
		{
			VIC_PRINT("can not finde dts_node\n");
			return i;
		}
		of_property_read_u32_array(dts_node, "vic_core", &values[3], 1);
		if(values[3] != 0)
			continue;
		
		of_property_read_u32_array(dts_node, "reg", &values[0], 1);
		values[1] = irq_of_parse_and_map(dts_node, 0);
		of_property_read_u32_array(dts_node, "use_int", &values[2], 1);
		of_property_read_u32_array(dts_node, "polar", &values[4], 1);

		gstVicInfo[i].vic_phy_base_addr = (void *)values[0];
		gstVicInfo[i].vic_irq_num = values[1];
		gstVicInfo[i].vic_intusemask = values[2];
		gstVicInfo[i].vic_virt_base_addr = of_iomap(dts_node, 0);
		writel(VIC_MASK_ALL_INT, (unsigned long)gstVicInfo[i].vic_virt_base_addr + VIC_OFFSET_MASK_ENA);

		for(j = 0; j < INT_VIC_MAX; j++)
		{
			if((0x1U << j) & values[4])
				writel(0x1U << j, (unsigned long)gstVicInfo[i].vic_virt_base_addr + VIC_OFFSET_POLAR_ENA);
			else
				writel(0x1U << j, (unsigned long)gstVicInfo[i].vic_virt_base_addr + VIC_OFFSET_POLAR_CLR);
			gstVicInfo[i].victable[j].routine = NULL;
			gstVicInfo[i].victable[j].arg = 0;
		}
		if(request_irq(gstVicInfo[i].vic_irq_num, (irq_handler_t)bsp_vic_inthandler, 0, "vic", &gstVicInfo[i]) != OK)
		{
			VIC_PRINT("bsp_vic_init error \n");
			return ERROR;
		}
	}

	VIC_PRINT("bsp_vic_init ok \n");
	return OK;
}

s32 bsp_vic_enable(int level)
{
	int key = 0;
	u32 vicnum = 0;
	u32 vic_interrupt_num = 0;

	if((level < 0) || (level >= (BALONG_VIC_NUM * INT_VIC_MAX)))
	{
		VIC_PRINT("vic input level error! level:%d\n", level);
		return 	ERROR;
	}

	vicnum = (u32)level >> 5;
	vic_interrupt_num = (u32)level & 0x1F;

	if(!((0x1U << vic_interrupt_num) & gstVicInfo[vicnum].vic_intusemask))
	{
		VIC_PRINT("vic input level not exist! level:%d\n", level);
		return ERROR;
	}

	local_irq_save(key);
	writel(0x1U << vic_interrupt_num, (unsigned long)gstVicInfo[vicnum].vic_virt_base_addr + VIC_OFFSET_MASK_CLR);
	local_irq_restore(key);

	return OK;
}

s32 bsp_vic_disable (int level)
{
	int key = 0;
	u32 vicnum = 0;
	u32 vic_interrupt_num = 0;

	if((level < 0) || (level >= (BALONG_VIC_NUM * INT_VIC_MAX)))
	{
		VIC_PRINT("vic input level error! level:%d\n", level);
		return 	ERROR;
	}

	vicnum = (u32)level >> 5;
	vic_interrupt_num = (u32)level & 0x1F;

	if(!((0x1U << vic_interrupt_num) & gstVicInfo[vicnum].vic_intusemask))
	{
		VIC_PRINT("vic input level not exist! level:%d\n", level);
		return ERROR;
	}

	local_irq_save(key);
	writel(0x1U << vic_interrupt_num, (unsigned long)gstVicInfo[vicnum].vic_virt_base_addr + VIC_OFFSET_MASK_ENA);
	local_irq_restore(key);

	return OK;
}

s32 bsp_vic_connect(int level, vicfuncptr routine, s32 parameter)
{
	u32 vicnum = 0;
	u32 vic_interrupt_num = 0;

	if((level < 0) || (level >= (BALONG_VIC_NUM * INT_VIC_MAX)))
	{
		VIC_PRINT("bsp_vic_connect input error! level:%d\n", level);
		return 	ERROR;
	}

	vicnum = (u32)level >> 5;
	vic_interrupt_num = (u32)level & 0x1F;

	if(!((0x1U << vic_interrupt_num) & gstVicInfo[vicnum].vic_intusemask))
	{
		VIC_PRINT("vic input level not exist! level:%d\n", level);
		return ERROR;
	}

	gstVicInfo[vicnum].victable[vic_interrupt_num].routine = routine;
	gstVicInfo[vicnum].victable[vic_interrupt_num].arg = parameter;

	return OK;
}

s32 bsp_vic_disconnect(int level)
{
	u32 vicnum = 0;
	u32 vic_interrupt_num = 0;

	if((level < 0) || (level >= (BALONG_VIC_NUM * INT_VIC_MAX)))
	{
		VIC_PRINT("bsp_vic_disconnect input error! level:%d\n", level);
		return 	ERROR;
	}

	vicnum = (u32)level >> 5;
	vic_interrupt_num = (u32)level & 0x1F;

	if(!((0x1U << vic_interrupt_num) & gstVicInfo[vicnum].vic_intusemask))
	{
		VIC_PRINT("vic input level not exist! level:%d\n", level);
		return ERROR;
	}

	gstVicInfo[vicnum].victable[vic_interrupt_num].routine = NULL;
	gstVicInfo[vicnum].victable[vic_interrupt_num].arg = 0;

	return OK;
}

postcore_initcall(bsp_vic_init);
EXPORT_SYMBOL(bsp_vic_enable);
EXPORT_SYMBOL(bsp_vic_disable);
EXPORT_SYMBOL(bsp_vic_connect);
EXPORT_SYMBOL(bsp_vic_disconnect);
