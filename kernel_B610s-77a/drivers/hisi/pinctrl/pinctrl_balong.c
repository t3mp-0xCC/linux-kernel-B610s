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
#include <linux/gpio.h>
#include <soc_memmap.h>
#include <ios_ao_drv_macro.h>
#include <bsp_version.h>
#include <product_config.h>
#include "hi_gpio.h"

void* pinctrl_base = NULL;
void pinctrl_mmc_suspend(void)
{
    u32 temp = 0;
    if(CHIP_V750 == bsp_get_version_info()->chip_type){
        (void)gpio_request(GPIO_11_4, "GPIO_11_4");
        (void)gpio_direction_input(GPIO_11_4);
        gpio_free(GPIO_11_4);

        (void)gpio_request(GPIO_11_5, "GPIO_11_5");
        (void)gpio_direction_input(GPIO_11_5);
        gpio_free(GPIO_11_5);

        (void)gpio_request(GPIO_11_6, "GPIO_11_6");
        (void)gpio_direction_input(GPIO_11_6);
        gpio_free(GPIO_11_6);

        (void)gpio_request(GPIO_11_7, "GPIO_11_7");
        (void)gpio_direction_input(GPIO_11_7);
        gpio_free(GPIO_11_7);

        (void)gpio_request(GPIO_12_0, "GPIO_12_0");
        gpio_direction_input(GPIO_12_0);
        gpio_free(GPIO_12_0);

        (void)gpio_request(GPIO_12_1, "GPIO_12_1");
        (void)gpio_direction_input(GPIO_12_1);
        gpio_free(GPIO_12_1);

        temp = readl(pinctrl_base + 0x00c);
        writel(temp & ~0x3, pinctrl_base + 0x00c);

        temp = readl(pinctrl_base + 0x404);
        writel(temp | 0x1f80000, pinctrl_base + 0x404);

        writel(0x44, pinctrl_base + 0x880);
        writel(0x44, pinctrl_base + 0x884);
        writel(0x44, pinctrl_base + 0x888);
        writel(0x44, pinctrl_base + 0x88c);
        writel(0x44, pinctrl_base + 0x890);
        writel(0x44, pinctrl_base + 0x894);

    }
    else if(CHIP_V722 == bsp_get_version_info()->chip_type){
		(void)gpio_request(GPIO_10_1, "GPIO_10_1");
		(void)gpio_direction_input(GPIO_10_1);
		gpio_free(GPIO_10_1);
		
		(void)gpio_request(GPIO_10_2, "GPIO_10_2");
		(void)gpio_direction_input(GPIO_10_2);
		gpio_free(GPIO_10_2);
		
		(void)gpio_request(GPIO_10_3, "GPIO_10_3");
		(void)gpio_direction_input(GPIO_10_3);
		gpio_free(GPIO_10_3);
		
		(void)gpio_request(GPIO_10_4, "GPIO_10_4");
		(void)gpio_direction_input(GPIO_10_4); 
		gpio_free(GPIO_10_4);
		
		(void)gpio_request(GPIO_10_5, "GPIO_10_5");
		gpio_direction_input(GPIO_10_5);
		gpio_free(GPIO_10_5);
	
		(void)gpio_request(GPIO_10_6, "GPIO_10_6");
		(void)gpio_direction_input(GPIO_10_6);
		gpio_free(GPIO_10_6);

		writel(0x00, pinctrl_base + 0x008);
		writel(0x3f, pinctrl_base + 0x410);
		writel(0x00, pinctrl_base + 0x430);

		writel(0x44,  pinctrl_base + 0x868);
		writel(0x44,  pinctrl_base + 0x86c);
 		writel(0x44,  pinctrl_base + 0x870);
		writel(0x44,  pinctrl_base + 0x874);
		writel(0x44,  pinctrl_base + 0x878);
		writel(0x44,  pinctrl_base + 0x87c);
   }

}


void pinctrl_mmc_resume(void)
{
    u32 temp = 0;
    if(CHIP_V750 == bsp_get_version_info()->chip_type){
        temp = readl(pinctrl_base + 0x00c);
        writel(temp | 0x3, pinctrl_base + 0x00c);

        temp = readl(pinctrl_base + 0x404);
        writel(temp & ~0x1f80000, pinctrl_base + 0x404);

        writel(0xcc, pinctrl_base + 0x880);/*dirver 16ma*/
        writel(0x2c, pinctrl_base + 0x884);
        writel(0x2c, pinctrl_base + 0x888);
        writel(0x2c, pinctrl_base + 0x88c);
        writel(0x2c, pinctrl_base + 0x890);
        writel(0x0c, pinctrl_base + 0x894);
    }
    else if(CHIP_V722 == bsp_get_version_info()->chip_type){
		writel(0x3F, pinctrl_base + 0x008);
		writel(0x00, pinctrl_base + 0x410);
		writel(0x00, pinctrl_base + 0x430);

		writel(0xc4,  pinctrl_base + 0x868);
		writel(0x24,  pinctrl_base + 0x86c);
 		writel(0x24,  pinctrl_base + 0x870);
		writel(0x24,  pinctrl_base + 0x874);
		writel(0x24,  pinctrl_base + 0x878);
		writel(0x24,  pinctrl_base + 0x87c);
   }
}

void pinctrl_audio_supend(void)
{

}

void pinctrl_audio_resume(void)
{
    u32 temp = 0;
    if(CHIP_V750 == bsp_get_version_info()->chip_type){
        // pcm mux ctl
        temp = readl(pinctrl_base + 0x434);
        writel(temp | 0x8000, pinctrl_base + 0x434);

        temp = readl(pinctrl_base + 0x420);
        writel(temp & ~0xf00000, pinctrl_base + 0x420);

        temp = readl(pinctrl_base + 0x40c);
        writel(temp & ~0xc00, pinctrl_base + 0x40c);

        temp = readl(pinctrl_base + 0x10);
        writel(temp & ~0xf000000, pinctrl_base + 0x10);

        temp = readl(pinctrl_base + 0x904) & ~0x70;
        writel(temp | 0x20, pinctrl_base + 0x904);

        temp = readl(pinctrl_base + 0x908) & ~0x70;
        writel(temp | 0x20, pinctrl_base + 0x908);

        temp = readl(pinctrl_base + 0x90c) & ~0x70;
        writel(temp | 0x40, pinctrl_base + 0x90c);

        temp = readl(pinctrl_base + 0x910) & ~0x70;
        writel(temp | 0x20, pinctrl_base + 0x910);

        // i2c mux ctl
        temp = readl(pinctrl_base + 0x444);
        writel(temp | 0x2000000, pinctrl_base + 0x444);

        temp = readl(pinctrl_base + 0x434);
        writel(temp & ~0x60000, pinctrl_base + 0x434);

        temp = readl(pinctrl_base + 0x420);
        writel(temp & ~0x1000000, pinctrl_base + 0x420);

        temp = readl(pinctrl_base + 0x40c);
        writel(temp & ~0xc000, pinctrl_base + 0x40c);

        temp = readl(pinctrl_base + 0x10);
        writel(temp & ~0xc0000000, pinctrl_base + 0x10);

        temp = readl(pinctrl_base + 0x91c);
        writel(temp & ~0x30, pinctrl_base + 0x91c);

        temp = readl(pinctrl_base + 0x920);
        writel(temp & ~0x30, pinctrl_base + 0x920);

    }
    else if(CHIP_V722 == bsp_get_version_info()->chip_type){
        // pcm mux ctl
        temp = readl(pinctrl_base + 0x418);
        writel(temp & ~0x3c000, pinctrl_base + 0x418);

        temp = readl(pinctrl_base + 0x45c);
        writel(temp & ~0x9c, pinctrl_base + 0x45c);

        temp = readl(pinctrl_base + 0x464);
        writel(temp | 0x3, pinctrl_base + 0x464);

        temp = readl(pinctrl_base + 0x454);
        writel(temp | 0xc, pinctrl_base + 0x454);
    }

}


EXPORT_SYMBOL_GPL(pinctrl_mmc_suspend);
EXPORT_SYMBOL_GPL(pinctrl_mmc_resume);

EXPORT_SYMBOL_GPL(pinctrl_audio_supend);
EXPORT_SYMBOL_GPL(pinctrl_audio_resume);

static int  __init hi_pinctrl_init(void)
{
    struct device_node *dev_node   = NULL;
    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,pinctrl_balong");
    if(NULL == dev_node)
    {
        printk("pinctrl device node not found!\n");
        return -1;
    }

    pinctrl_base = of_iomap(dev_node, 0);
    if (NULL == pinctrl_base)
    {
        printk(KERN_ERR "pinctrl remap addr fail\n");
        return -1;
    }

    return 0;
}


module_init(hi_pinctrl_init);

