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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS"
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
/*lint -save -e34 -e537 -e737*/

/*lint -e34*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/gfp.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include "product_config.h"
#if (FEATURE_ON == MBB_USB_CPE)
#include <linux/gpio.h>
#endif

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/usb/bsp_usb.h>

#include <asm/mach/irq.h>
#include "bsp_memmap.h"
#include "bsp_sysctrl.h"
#include "bsp_pmu.h"
#include "dwc3_bcctrl_balong.h"

#if (FEATURE_ON == MBB_USB)
#include "usb_vendor.h"
#else
#include "../gadget/usb_vendor.h"
#endif

#if (FEATURE_ON == MBB_USB)
void sysctrl_set_usbid(unsigned int usbid);
void syssc_usb_iddig_en(unsigned int value);
#include "usb_otg_ctrl.c"
#endif

u64 dwc3_dma_mask = 0xffffffffUL;

static const struct of_device_id balong_dwc3_match[] = {
			{.compatible = "hisilicon,usb3"},
			{},
};

struct dwc3_balong {
	struct device		*dev;
};
static struct dwc3_balong hisi_balong = {0};

int usb3_core_clk_enable(void)
{
    struct device *dev = hisi_balong.dev;
    struct clk *aclk = NULL;
    int ret = 0;

    aclk =devm_clk_get(dev,"usb_core_aclk");
    if(IS_ERR(aclk)){
        dev_err(dev, "clk:usb_core_aclk is NULL, can't find it,please check!!\n");
        return -ENXIO;
    } else {
        ret = clk_prepare_enable(aclk);
        if(ret){
            dev_err(dev, "clk:usb_core_aclk enable fail !!\n");
        } 
    }
    return ret;
}

int usb3_core_clk_disable(void)
{
    struct device *dev = hisi_balong.dev;
    struct clk *aclk = NULL;

    aclk =devm_clk_get(dev,"usb_core_aclk");
    if(IS_ERR(aclk)){
        dev_err(dev,"clk:usb_core_aclk is NULL, can't find it,please check!!\n");
        return -ENXIO;
    } else {
	    clk_disable_unprepare(aclk);
    }
    return 0;
}

int usb3_sysctrl_get(const struct device_node *np, const char *propname)
{
	u32 para_value[3] = {0};
	u32 bitnum;
	void *base;
	u32 reg = 0;
	int ret;
	
	ret = of_property_read_u32_array(np, propname, para_value, 3);
	if(ret){
		printk(KERN_DEBUG "skip find of [%s]\n", propname);
		return 0;
	}

	bitnum = para_value[2] - para_value[1] + 1;
	base = bsp_sysctrl_addr_get((void*)para_value[0]);
	if(!base){
		printk(KERN_DEBUG "Get sysctrl fail\n");
		return 0;
	}
	reg = readl(base);
	reg &= (((1<<bitnum)-1)<<para_value[1]);
	reg = (reg >> para_value[1]);
	return reg;
}
/*
	para_value[0]:Reigster address;
	para_value[1]:Reigster bit range, start;
	para_value[2]:Reigster bit range, end;
*/
void usb3_sysctrl_set(const struct device_node *np, const char *propname, 
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

/*
	para_value[0]:Reigster address;
	para_value[1]:Reigster bit range, start;
	para_value[2]:Reigster bit range, end;
	para_value[3]:Use DTS setting instead of reigster default setting? 
						Yes=1 Use setting read from dts, 
						No=0  Use reigster default setting;
	para_value[4]:Reigster setting from dts;
*/

/*This func reads board specific settings from dts and apply them to reigister */
void usb3_sysctrl_set_from_dts(const struct device_node *np, const char *propname)
{
	unsigned int para_value[5]={0};
	int ret;

	ret = of_property_read_u32_array(np, propname, para_value, 5);
	if(ret) {
		printk(KERN_DEBUG "skip find of [%s]\n", propname);
		return;
	}		
	if(para_value[3])
		usb3_sysctrl_set(np, propname, para_value[4]);
}

/*follow functions can be used to set sysctrl*/
void syssc_usb_powerdown_hsp(unsigned int value)
{
	struct device_node *np = hisi_balong.dev->of_node;

	usb3_sysctrl_set(np, "usb_phy_ctrl0_test_powerdown_hsp", value);
}

void syssc_usb_powerdown_ssp(unsigned int value)
{
	struct device_node *np = hisi_balong.dev->of_node;

	usb3_sysctrl_set(np, "usb_phy_ctrl0_test_powerdown_ssp", value);
}

void syssc_usb_ref_ssp(unsigned int value)
{
	struct device_node *np = hisi_balong.dev->of_node;

	usb3_sysctrl_set(np, "usb_phy_ctrl0_ref_ssp_en", value);
}

void syssc_usb_vbusvldext(unsigned int value)
{
	struct device_node *np = hisi_balong.dev->of_node;

	usb3_sysctrl_set(np, "usb_phy_ctrl0_vbusvldext", value);
	usb3_sysctrl_set(np, "usb_phy_ctrl0_vbusvldextsel", value);
}

void syssc_usb_override(unsigned int value)
{
	struct device_node *np = hisi_balong.dev->of_node;

	usb3_sysctrl_set(np, "usb_controller_ctrl0_usb3_pmu_powerpresent_override_en", value);
	usb3_sysctrl_set(np, "usb_controller_ctrl0_usb3_pmu_bvalid_override_en", value);
	usb3_sysctrl_set(np, "usb_controller_ctrl0_usb3_pmu_vbusvalid_override_en", value);
	usb3_sysctrl_set(np, "usb_controller_ctrl0_usb3_pmu_avalid_override_en", value);
	usb3_sysctrl_set(np, "usb_controller_ctrl0_usb3_pmu_powerpresent_override", value);
	usb3_sysctrl_set(np, "usb_controller_ctrl0_usb3_pmu_avalid_override", value);
	usb3_sysctrl_set(np, "usb_controller_ctrl0_usb3_pmu_bvalid_override", value);
	usb3_sysctrl_set(np, "usb_controller_ctrl0_usb3_pmu_vbusvalid_override", value);
}

void syssc_usb_iddig_en(unsigned int value)
{
	struct device_node *np = hisi_balong.dev->of_node;

	usb3_sysctrl_set(np, "usb_controller_ctrl0_usb3_pmu_iddig_override_en", value);
}

void syssc_usb_txpreempamptune(int value)
{
	struct device_node *np = hisi_balong.dev->of_node;

	usb3_sysctrl_set(np, "usb_phy_ctrl2_txpreempamptune", value);
}

void syssc_usb_phy3_init(void)
{
	struct device_node *np = hisi_balong.dev->of_node;

	usb3_sysctrl_set_from_dts(np, "usb_phy_ctrl2_txpreempamptune");
	usb3_sysctrl_set_from_dts(np, "usb_phy_ctrl3_los_level4_3");
	usb3_sysctrl_set_from_dts(np, "usb_phy_ctrl3_pcs_tx_deemph_3p5db");
	usb3_sysctrl_set_from_dts(np, "usb_phy_ctrl3_pcs_tx_deemph_6db");
	usb3_sysctrl_set_from_dts(np, "usb_phy_ctrl3_pcs_tx_swing_full");
	usb3_sysctrl_set_from_dts(np, "usb_phy_ctrl3_lane0_tx_term_offset");
	usb3_sysctrl_set_from_dts(np, "usb_phy_ctrl3_tx_vboost_lvl");
	usb3_sysctrl_set_from_dts(np, "usb_phy_ctrl1_ssc_en");
	usb3_sysctrl_set_from_dts(np, "usb_phy_ctrl1_usb3phy_idpullup");
	usb3_sysctrl_set_from_dts(np, "usb_phy_ctrl2_los_bias");
	/*set usb reset chirp K to 1.5ms*/
	usb3_sysctrl_set_from_dts(np, "usb_controller_ctrl2_usb3_chirp_time_sel");

}

void syscrg_usb_release(void)
{
	struct device_node *np = hisi_balong.dev->of_node;

	usb3_sysctrl_set(np, "usbphy_vcc_srst_dis", 1);
	usb3_sysctrl_set(np, "usbotg_ctrl_srst_dis", 1);
	usb3_sysctrl_set(np, "usbctrl_vaux_srst_dis", 1);
	usb3_sysctrl_set(np, "usbctrl_vcc_srst_dis", 1);
}

void syscrg_usb_reset(void)
{
	struct device_node *np = hisi_balong.dev->of_node;
	
	usb3_sysctrl_set(np, "usbctrl_vcc_srst_en", 1);
	usb3_sysctrl_set(np, "usbctrl_vaux_srst_en", 1);
	usb3_sysctrl_set(np, "usbotg_ctrl_srst_en", 1);
	usb3_sysctrl_set(np, "usbphy_vcc_srst_en", 1);
}/*above functions can be used to set sysctrl*/


void sysctrl_set_usbid(unsigned int usbid)
{
	struct device_node *np = hisi_balong.dev->of_node;

	usb3_sysctrl_set(np, "usb_controller_ctrl0_usb3_pmu_iddig_override", usbid);
}

static void sysctrl_poll_phy_state_ack(void)
{
	struct device_node *np = hisi_balong.dev->of_node;
	int result = 0;
	int i = 500;
	
	do{
		result = usb3_sysctrl_get(np, "usb3_phy_state_cr_ack");
		udelay(1);
		if(0 == i){
			printk("poll_phy_state_ack time out. \n");
			break;
		}
		i--;
	}while(0 != result);
}


void sysctrl_set_rx_scope_lfps_en(void)
{	
	struct device_node *np = hisi_balong.dev->of_node;

	usb3_sysctrl_set(np, "usb_phy_ctrl4", 0x1026);
	usb3_sysctrl_set(np, "usb_phy_ctrl4", 0x11026);
	sysctrl_poll_phy_state_ack();
	
	usb3_sysctrl_set(np, "usb_phy_ctrl4", 0x1);
	usb3_sysctrl_set(np, "usb_phy_ctrl4", 0x20001);
	sysctrl_poll_phy_state_ack();

	usb3_sysctrl_set(np, "usb_phy_ctrl4", 0x1);
	usb3_sysctrl_set(np, "usb_phy_ctrl4", 0x40001);
	sysctrl_poll_phy_state_ack();
}

#ifdef CONFIG_GADGET_CORE_HIBERNATION

void syscrg_vcc_reset_for_lpm(void)
{
	struct device_node *np = hisi_balong.dev->of_node;
	usb3_sysctrl_set(np, "usbctrl_vcc_srst_en", 1);

}
void syscrg_vcc_unreset_for_lpm(void)
{
	struct device_node *np = hisi_balong.dev->of_node;
	usb3_sysctrl_set(np, "usbctrl_vcc_srst_dis", 1);
}

void syscrg_usb_set_pmu_state(unsigned int value)
{
		struct device_node *np = hisi_balong.dev->of_node;
		usb3_sysctrl_set(np, "usb_controller_ctrl0_usb3_pm_power_state_requset", value);
}

/*need more work, done*/
int syscrg_usb_get_pmu_state(unsigned int is_superspeed)
{
	struct device_node *np = hisi_balong.dev->of_node;
	int ret1,ret2;

	ret1 = usb3_sysctrl_get(np, "usb3_pmu_current_power_state_u3pmu");
	ret2 = usb3_sysctrl_get(np, "usb3_pmu_current_power_state_u2pmu");
	if(ret1 != ret2){
		
		return -1;
	}
	return ret2;
}

static int dwc3_pmuctrl_set(const struct device_node *np, 
	const char *propname, int hibernation_en)
{
	/*
	u32 para_value[0] pmu addr
	u32 para_value[1] pmu setting when usb wakeup disable
	u32 para_value[2] pmu setting when usb wakeup enable
	*/
	u32 para_value[3] = {0};
	u32 base;
	u32 addr;
	u32 reg = 0;
	int ret;
	
	ret = of_property_read_u32_array(np, propname, para_value, 3);
	if(ret){
		printk(KERN_DEBUG "skip find of [%s]\n", propname);
		return 0;
	}

	base =  bsp_pmic_get_base_addr();
	if(!base){
		printk(KERN_DEBUG "Get sysctrl fail\n");
		return -ENXIO;
	}
	if(hibernation_en){
		reg = para_value[2];
	}
	else{
		reg = para_value[1];
	}
	
	addr= para_value[0];
	
    writel(reg, (void*)(base + (addr << 2)));

	return 0;

}


int dwc3_pmuctrl_set_usbphy_clk_mode(int is_superspeed)
{
	struct device_node *np = hisi_balong.dev->of_node;

	int ret;
		
	ret = dwc3_pmuctrl_set(np, "usb_clk_buff_usb_en", is_superspeed);
	if(ret)
		return ret;
	ret = dwc3_pmuctrl_set(np, "usb_clk_ana_clk_en", is_superspeed);
	return ret;
}


int dwc3_pmuctrl_set_usbphy_pwctrl_mode(int is_support_wakeup)
{
	struct device_node *np = hisi_balong.dev->of_node;
	int ret;
	
	ret = dwc3_pmuctrl_set(np, "ldo7_peri_ctrl1", is_support_wakeup);
	return ret;
}
#endif

void dwc3_sysctrl_init(void)
{
#if (FEATURE_ON == MBB_USB_CPE) 
    usb3_otg_cpe_init_mode();
#endif
   /* power up the usb2.0 and usb3.0 phy */
	syssc_usb_powerdown_hsp(0);
	syssc_usb_powerdown_ssp(0);

	/* ref_ssp_en */
	syssc_usb_ref_ssp(1);

#ifdef CONFIG_USB_DWC3_VBUS_DISCONNECT
    /* vbusvldext and vbusvldextsel */
	syssc_usb_vbusvldext(1);

    /* override_en and override_value*/
	syssc_usb_override(1);
#else
    /* vbusvldext and vbusvldextsel */
	syssc_usb_vbusvldext(0);

    /* override_en and override_value*/
	syssc_usb_override(0);
#endif

#ifdef CONFIG_USB_OTG_USBID_BYGPIO
	syssc_usb_iddig_en(1);
#endif

	syssc_usb_phy3_init();

	/* set bc_mode to bypass */
	bc_set_bypass_mode();

	/* enable the otg clock */
	usb3_core_clk_enable();
	
    /* release controller and PHY */

	syscrg_usb_release();
   
	mdelay(10);/*lint !e62*/
}

#if (FEATURE_ON == MBB_CHARGE)
extern int dp_dm_connected;
extern int chg_en_flag;

int bc_get_dp_dm_status(void)
{
    unsigned record_reg;
    int type = USB_CHARGER_TYPE_INVALID;
    /*bcctrl_clk_enable();*/

    /* setup to soft mode */
    bc_set_soft_mode();

    msleep(50);

    record_reg = bc_set_test_volt();

    /* wait 50ms for D+ signal go to D- */
    mdelay(50);

    type = bc_get_charger_type();
    printk(KERN_ERR "bc_get_dp_dm_status chg_type %d.\n", type);
    /* restore the default setting */
    bc_set_record_volt(record_reg);

    mdelay(20);

    /* restore to bypass mode */
    bc_set_bypass_mode();

    mdelay(2);
    /*bcctrl_clk_disable();*/

    return type;
}
#endif /* MBB_CHARGE */
void dwc3_sysctrl_exit(void)
{	
    /* reset controller and phy */
	syscrg_usb_reset();

	/* disable the usb otg clock */
	usb3_core_clk_disable();

	/* reset vbusvldext and override_en */
	syssc_usb_override(0);
	syssc_usb_vbusvldext(0);

    /* reset ref_ssp_en */
	syssc_usb_ref_ssp(0);

#if (FEATURE_ON == MBB_CHARGE)
    if(1 == bc_get_dp_dm_status() && 1 == chg_en_flag) {
        dp_dm_connected = 1;
    } else {
        dp_dm_connected = 0;
    }
    printk(KERN_ERR "dp_dm_connected %d.\n", dp_dm_connected);
#endif /* MBB_CHARGE */
    /* reset bc_mode to soft modes */
    bc_set_soft_mode();

	/* power down the usb2.0 and usb3.0 phy */
	syssc_usb_powerdown_hsp(1);
	syssc_usb_powerdown_ssp(1);

	mdelay(2);/*lint !e62*/
}
#if (FEATURE_ON == MBB_USB)
void usb_bc_iden(void)
{
    struct dwc3_balong *balong = &hisi_balong;
    struct platform_device *pdev = to_platform_device(balong->dev);
    bsp_bcctrl_init(pdev);
    
    /* if the version support charger, identify the charger type
    * we must detect charger type before usb core init*/
#if defined(CONFIG_USB_CHARGER_DETECT)
    bsp_usb_set_charger_type(bc_charger_type_identify());
#endif
    /* init otg controller and phy */
    dwc3_sysctrl_init();


}
#endif
static int dwc3_balong_probe(struct platform_device *pdev)
{
	struct device_node	*node = pdev->dev.of_node;
	struct dwc3_balong	*balong = &hisi_balong;
	struct device		*dev = &pdev->dev;
	int			ret = -ENOMEM;

	if (!node) {
		dev_err(dev, "device node not found\n");
		return -EINVAL;
	}

	platform_set_drvdata(pdev, balong);

	balong->dev	= dev;
	dev->dma_mask	= &dwc3_dma_mask;
	dev->coherent_dma_mask = DMA_BIT_MASK(64);

#if (FEATURE_ON != MBB_USB)
    /*bcctrl should be init(unreset) before the core*/
    bsp_bcctrl_init(pdev);
    /* if the version support charger, identify the charger type
    * we must detect charger type before usb core init*/
#if defined(CONFIG_USB_CHARGER_DETECT)
    bsp_usb_set_charger_type(bc_charger_type_identify());
#endif
    /* init otg controller and phy */
    dwc3_sysctrl_init();

#endif
	ret = of_platform_populate(node, NULL, NULL, dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to create dwc3 core\n");
		return ret;
	}

	return 0;
}

static int dwc3_balong_remove_core(struct device *dev, void *c)
{
	struct platform_device *pdev = to_platform_device(dev);

	of_device_unregister(pdev);
	return 0;
}

static int dwc3_balong_remove(struct platform_device *pdev)
{
	struct dwc3_balong	*balong = &hisi_balong;
	device_for_each_child(&pdev->dev, NULL, dwc3_balong_remove_core);

    /* reset the otg controller and phy */
	dwc3_sysctrl_exit();

	balong->dev = NULL;
	return 0;
}


static struct platform_driver dwc3_balong_driver = {
	.probe		= dwc3_balong_probe,
	.remove		= dwc3_balong_remove,
	.driver		= {
		.name	= "balong-dwc3",
		.of_match_table	= of_match_ptr(balong_dwc3_match),
	},
};

int usb_dwc3_balong_drv_init(void)
{
    int ret;

    ret = platform_driver_register(&dwc3_balong_driver);
    if(ret)
    {
        printk("%s:platform_driver_register fail.\n",__FUNCTION__);
    }

    return ret;
}

void usb_dwc3_balong_drv_exit(void)
{
    platform_driver_unregister(&dwc3_balong_driver);
}

MODULE_AUTHOR("BALONG USBNET GROUP");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("BALONG USB3 DRD Controller Driver");
/*lint -restore*/
