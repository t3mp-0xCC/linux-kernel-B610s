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


#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/slab.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>

#include <linux/usb/bsp_usb.h>

#include "bsp_sysctrl.h"
#include "bsp_pmu.h"
#include "dwc3_bcctrl_balong.h"

/*
 * bc(battery charger) interfaces
 */

struct bcctrl{
	void * regs;
	struct device *dev;
};

struct bcctrl balong_dwc3_bcctrl = 
{
	.regs = NULL,
	.dev = NULL,
};

static const struct of_device_id balong_bcctrl_match[] = {
			{.compatible = "hisilicon,bcctrl"},
			{},
};

void bcctrl_clk_enable(void)
{
	struct clk *aclk = NULL;
	int ret = 0;
	struct device *dev = balong_dwc3_bcctrl.dev;
	
	aclk =devm_clk_get(dev,"bcctrl_aclk");
	if(IS_ERR(aclk)){
		dev_err(dev, "clk:bcctrl_aclk is NULL, can't find it,please check!!\n");
	} else {
		ret = clk_prepare_enable(aclk);
		if(ret){
			dev_err(dev, "clk:bcctrl_aclk enable fail !!\n");
		} 
	}

}

void bcctrl_clk_disable(void)
{
	struct clk *aclk = NULL;
	struct device *dev = balong_dwc3_bcctrl.dev;

	aclk =devm_clk_get(dev,"bcctrl_aclk");
	if(IS_ERR(aclk)){
		dev_err(dev, "clk:bcctrl_aclk is NULL, can't find it,please check!!\n");
	} else {
		clk_disable_unprepare(aclk);
	}
}


void bc_set_soft_mode(void)
{
    void* addr = balong_dwc3_bcctrl.regs;
    unsigned reg;
    /* enable the usb bc clock to change the bc mode */
    bcctrl_clk_enable();

    reg = readl(addr + 0x0);
    reg &= ~0x3;
    reg |= 0x1;/* [false alarm]:Disable fortify false alarm */
    writel(reg, addr + 0x0);
    /* clk off the bc controller, when we never use it */
    bcctrl_clk_disable();

    return;
}/*lint !e550*/

void bc_set_bypass_mode(void)
{
    void* addr = balong_dwc3_bcctrl.regs;
    unsigned reg;

    /* enable the usb-bc clk at first */
    bcctrl_clk_enable();

    reg = readl(addr + 0x0);
    reg &= ~0x03;
    reg |=  0x02;/* [false alarm]:Disable fortify false alarm */
    writel(reg, addr + 0x0);
		
    /* clk off the bc controller, when we never use it */
    bcctrl_clk_disable();

    return;
}/*lint !e550*/

unsigned bc_set_test_volt(void)
{
    void* addr = balong_dwc3_bcctrl.regs;
    unsigned reg;
    unsigned record_reg;

#if(FEATURE_ON == MBB_USB)
    /* enable the usb-bc clk at first */
    bcctrl_clk_enable();
#endif
    reg = readl(addr + 0x4);
    record_reg = reg;
#if(FEATURE_ON == MBB_USB)
    /* test volt is set to D-*/
    reg |= 0x1C;
    //reg &= ~0x10;/* [false alarm]:Disable fortify false alarm */
#else
    reg |= 0xC;
    reg &= ~0x10;/* [false alarm]:Disable fortify false alarm */
#endif
    writel(reg, addr + 0x4);
#if(FEATURE_ON == MBB_USB)
    /* clk off the bc controller, when we never use it */
    bcctrl_clk_disable();
#endif

    return record_reg;
}

void bc_set_record_volt(unsigned record_reg)
{
    void* addr = balong_dwc3_bcctrl.regs;
#if(FEATURE_ON == MBB_USB)
    /* enable the usb-bc clk at first */
    bcctrl_clk_enable();
#endif
    writel(record_reg, addr + 0x4);
#if(FEATURE_ON == MBB_USB)
    /* clk off the bc controller, when we never use it */
    bcctrl_clk_disable();
#endif		
    return;
}

/*
 * 0:huawei charger , 1:non-huawei charger, -1:invalid charger
 */
int bc_get_charger_type(void)
{
    void* addr = balong_dwc3_bcctrl.regs;
    unsigned reg;
#if(FEATURE_ON == MBB_USB)
    /* enable the usb-bc clk at first */
    bcctrl_clk_enable();
#endif
    reg = readl(addr + 0xc);
    if (0 != (0x2000 & reg)) {
#if(FEATURE_ON == MBB_USB)
        /* clk off the bc controller, when we never use it */
        bcctrl_clk_disable();
#endif
        return USB_CHARGER_TYPE_HUAWEI;
    }
#if(FEATURE_ON == MBB_USB)
    /* clk off the bc controller, when we never use it */
    bcctrl_clk_disable();
#endif
    return USB_CHARGER_TYPE_NOT_HUAWEI;
}

/*
 * note: the function will sleep in long time !!!
 * 0:huawei charger , 1:non-huawei charger, -1:invalid charger
 */
int bc_charger_type_identify(void)
{
    unsigned record_reg;
    int type = USB_CHARGER_TYPE_INVALID;
		
    /* enable the usb-bc clk at first */
    bcctrl_clk_enable();

    /* setup to soft mode */
    bc_set_soft_mode();

    /*
    VBUS ------
    D+     ----
    D-     ----
    RND  ------
    when usb insert into host, VBUS and RND connect first, then D+/D- signal on
    we need msleep to wait the signal ok.
    */
    msleep(2000);

    /*
    bit[2]=1
    bit[3]=1
    bit[4]=0
    supply D+ 0.6V volt(normal volt is 0.8V)for charger identify
    */
    record_reg = bc_set_test_volt();

    /* wait 50ms for D+ signal go to D- */
    mdelay(50);/*lint !e62*/

    /*
    get D- signal, and detect the charger type
    bit[13]==0 : SDP (PC)
    bit[13]==1 : DCP (CHARGER)
    */
    type = bc_get_charger_type();

    /* restore the default setting */
    bc_set_record_volt(record_reg);

    mdelay(20);/*lint !e62*/

    /* restore to bypass mode */
    bc_set_bypass_mode();

    mdelay(2);/*lint !e62*/
		
    /* clk off the bc controller, when we never use it */
    bcctrl_clk_disable();

    return type;
}



/*
	para_value[0]:Reigster address;
	para_value[1]:Reigster bit range, start;
	para_value[2]:Reigster bit range, end;
*/
void usb3_bc_sysctrl_set(const struct device_node *np, const char *propname, 
	unsigned int value)
{
	u32 para_value[3]={0};
	u32 bitnum;
	void *base;
	u32 reg = 0;
	int ret;
	
	ret = of_property_read_u32_array(np, propname, para_value, 3);
	if(ret){
		printk(KERN_DEBUG "skip find of [%s]\n", propname);
		return ;
	}

	bitnum = para_value[2] - para_value[1] + 1;
	base = bsp_sysctrl_addr_get((void*)para_value[0]);
		if(!base){
		printk(KERN_DEBUG "Get sysctrl fail\n");
		return;
	}

	reg = readl(base);
	reg &= ~(((1<<bitnum)-1)<<para_value[1]);
	reg |= (value << para_value[1]);
	writel(reg, base);
}



int bsp_bcctrl_init(struct platform_device *pdev)
{
	struct device_node * bc_node;
	struct bcctrl *balong_bcctrl = &balong_dwc3_bcctrl;

	bc_node = of_find_compatible_node(NULL, NULL, balong_bcctrl_match[0].compatible);
	if(NULL == bc_node){
		dev_err(&pdev->dev, "bc_ctrl device node not found \n");
		return -EINVAL;
	}
	
	if(NULL == balong_bcctrl->regs){
		balong_bcctrl->regs = of_iomap(bc_node, 0);
		if(NULL == balong_bcctrl->regs){
			dev_err(&pdev->dev, "balong_bcctrl reg iomap fail. \n");
			return -ENOMEM;
		}
	}
	balong_bcctrl->dev = &pdev->dev;

	usb3_bc_sysctrl_set(bc_node, "bc_srst_dis", 1);

	printk("balong_bcctrl_init success. \n");
	return 0;

}



MODULE_AUTHOR("BALONG USBNET GROUP");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("BALONG USB3 DRD Controller Driver");
