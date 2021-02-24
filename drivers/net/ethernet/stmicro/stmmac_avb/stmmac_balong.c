/*******************************************************************************
  This contains the functions to handle the platform driver.

  Copyright (C) 2007-2011  STMicroelectronics Ltd

 * 2016-2-18 - Modifed code to adapt Synopsys DesignWare Cores Ethernet 
 * Quality-of-Service (DWC_ether_qos) core, 4.10a.
 * liufangyuan <liufangyuan2@huawei.com>
 * Copyright (C) Huawei Technologies Co., Ltd.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
*******************************************************************************/

#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "bsp_regulator.h"
#include "bsp_sysctrl.h"
#include "stmmac.h"
#include "stmmac_debug.h"

#define STMMAC_PWR_NAME     "gmac-vcc"
struct stmmac_power_stat{
    unsigned int get_all;
    unsigned int get_fail;
    unsigned int put_all;
    unsigned int put_ok;
    unsigned int on_all;
    unsigned int on_null;
    unsigned int on_fail;
    unsigned int off_all;
    unsigned int off_null;
};

static struct stmmac_power_stat g_stmmac_power_stat = {0};
void *phy_inf_sel_base = NULL;

void stmmac_power_dump(void)
{
    printk("stmmac power dump   :\n");
    printk("get_all             :%d\n",g_stmmac_power_stat.get_all);
    printk("get_fail            :%d\n",g_stmmac_power_stat.get_fail);
    printk("put_all             :%d\n",g_stmmac_power_stat.put_all);
    printk("put_ok              :%d\n",g_stmmac_power_stat.put_ok);
    printk("on_all              :%d\n",g_stmmac_power_stat.on_all);
    printk("on_null             :%d\n",g_stmmac_power_stat.on_null);   
    printk("on_fail             :%d\n",g_stmmac_power_stat.on_fail);
    printk("off_all             :%d\n",g_stmmac_power_stat.off_all);
    printk("off_null            :%d\n",g_stmmac_power_stat.off_null);   
}
EXPORT_SYMBOL(stmmac_power_dump);

#if 0
static void stmmac_power_get(void)
{
    g_stmmac_power_stat.get_all++;
    
    /* get the usb supply to prepare to power on */
    stmmac_vcc = regulator_get(NULL, STMMAC_PWR_NAME);
    if (IS_ERR(stmmac_vcc)) {
        g_stmmac_power_stat.get_fail++;
        printk("regulator_get %s error:%d!/n",
            STMMAC_PWR_NAME,(int)PTR_ERR(stmmac_vcc));
    }
}

static void stmmac_power_put(void)
{
    g_stmmac_power_stat.put_all++;

    if(stmmac_vcc){
        g_stmmac_power_stat.put_ok++;
        regulator_put(stmmac_vcc); 
        stmmac_vcc = NULL;
    }
}

static void stmmac_power_on(void)
{
    int ret;

    g_stmmac_power_stat.on_all++;

    if (!stmmac_vcc) {
        g_stmmac_power_stat.on_null++;
        printk("stmmac:null stmmac_vcc!\n");
        return;
    }

    ret = regulator_enable(stmmac_vcc);
    if(ret) {
        g_stmmac_power_stat.on_fail++;
        printk("stmmac:failed to regulator_enable %s(err code:%d)\n",
            STMMAC_PWR_NAME, ret);
    }

    return;
}

static void stmmac_power_off(void)
{
    g_stmmac_power_stat.off_all++;

    if (!stmmac_vcc) {
        g_stmmac_power_stat.off_null++;
        printk("stmmac:null stmmac_vcc!\n");
        return;
    }

    regulator_disable(stmmac_vcc);
}
#endif

void stmmac_sysctrl_set(const struct device_node *np, const char *propname, int value)
{
	u32 para_value[3] = {0};
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
		printk("%s: bsp_sysctrl_addr_get failed!\n",__func__);
		return;
	}
	reg = readl(base);
	reg &= ~(((1<<bitnum)-1)<<para_value[1]);
	reg |= (value << para_value[1]);
	writel(reg, base);

}

void stmmac_srst_dis(const struct device_node *np)
{
	stmmac_sysctrl_set(np, "gmac_srst_dis", 1);
}

static int stmmac_sel_phy_intf(const struct device_node *np, 
									unsigned int phy_intf)
{
	int ret;
	unsigned int sysctl_pcie_gmac[3] = {0};
	int intf_val = -1;

	/* select intface value according to sysctrl pcie */
	switch (phy_intf) {
		case  PHY_INTERFACE_MODE_MII:
			intf_val = 0;
			break;

		case PHY_INTERFACE_MODE_RMII:
			intf_val = 4;			
			break;

		case PHY_INTERFACE_MODE_RGMII:
			intf_val = 1;	
			break;

		default:
			ret = -EINVAL;
			break;
	}

	/* sysctl_pcie_gmac[0]:sysctrl_pcie base address
	 * sysctl_pcie_gmac[1]:sysctrl_pcie reg size.
	 * sysctl_pcie_gmac[2]:GMAC phy interface select controller offset
	*/
	ret = of_property_read_u32_array(np, "sysctrl_pcie_gmac", sysctl_pcie_gmac, 3);
	if(ret){
		printk(KERN_DEBUG "skip find of sysctrl_pcie_base\n");
		return ret;
	}

	if (!phy_inf_sel_base) {
		phy_inf_sel_base = ioremap(sysctl_pcie_gmac[0], sysctl_pcie_gmac[1]);
		if (!phy_inf_sel_base) {
			printk(KERN_DEBUG "[%s]base is NULL\n", __func__);
			return -EIO;
		}
	}

	/* select gmac phy interface, gmac_rmii_osc_sel fixed to 1 */	
	ret = intf_val | 0x10;	
	writel(ret, phy_inf_sel_base + sysctl_pcie_gmac[2]);

	return 0;
}

static int stmmac_mii_clk_enable(struct device *dev)
{
	struct clk *cfg_clk = NULL;	
	struct clk *ref_clk = NULL;
    struct clk *tx_clk = NULL;
	struct clk *ptp_clk = NULL;
    struct clk *rx_clk = NULL;
	int ret;

	/* open gmac aclk(bus config clock) */
	cfg_clk = devm_clk_get(dev,"gmac_aclk");
    if(IS_ERR(cfg_clk)){
            GMAC_ERR(("Can't find gmac_aclk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(cfg_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_aclk failed!\n"));
			return ret;
	     } 
    }

	/* Open rx clock */
	rx_clk = devm_clk_get(dev,"gmac_rx_clk");
    if(IS_ERR(rx_clk)){
            GMAC_ERR(("Can't find gmac_rx_clk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(rx_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_rx_clk failed!\n"));
			return ret;
	     } 
    }
	
	/* Open gmac reference clock */
    ref_clk = devm_clk_get(dev,"gmac_refclk");
    if(IS_ERR(ref_clk)){
            GMAC_ERR(("Can't find gmac_refclk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(ref_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_refclk failed!\n"));
			return ret;
	     } 
    }

	/* Open ptp clock */
	ptp_clk = devm_clk_get(dev,"gmac_ptp_clk");
    if(IS_ERR(ptp_clk)){
            GMAC_ERR(("Can't find gmac_ptp_clk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(ptp_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_ptp_clk failed!\n"));
			return ret;
	     } 
    }

	/* Open gtx clock */
	tx_clk = devm_clk_get(dev,"gmac_switch_clk");
    if(IS_ERR(tx_clk)){
            GMAC_ERR(("Can't find gmac_switch_clk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(tx_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_switch_clk failed!\n"));
			return ret;
	     } 
    }	
	
	return ret;
}

static int stmmac_rmii_clk_enable(struct device *dev)
{
	struct clk *rmii_clk = NULL;
	struct clk *cfg_clk = NULL;	
	struct clk *ref_clk = NULL;
	struct clk *ptp_clk = NULL;
	int ret;

	/* open gmac aclk(bus config clock) */
	cfg_clk = devm_clk_get(dev,"gmac_aclk");
    if(IS_ERR(cfg_clk)){
            GMAC_ERR(("Can't find gmac_aclk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(cfg_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_aclk failed!\n"));
			return ret;
	     } 
    }
	
	/* Open rx and RMII clock */
	rmii_clk = devm_clk_get(dev,"gmac_rmii_clk");
    if(IS_ERR(rmii_clk)){
            GMAC_ERR(("Can't find gmac_rmii_clk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(rmii_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_rmii_clk failed!\n"));
			return ret;
	     } 
    }

	/* Open gmac reference clock */
    ref_clk = devm_clk_get(dev,"gmac_refclk");
    if(IS_ERR(ref_clk)){
            GMAC_ERR(("Can't find gmac_refclk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(ref_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_refclk failed!\n"));
			return ret;
	     } 
    }

	/* Open ptp clock */
	ptp_clk = devm_clk_get(dev,"gmac_ptp_clk");
    if(IS_ERR(ptp_clk)){
            GMAC_ERR(("Can't find gmac_ptp_clk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(ptp_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_ptp_clk failed!\n"));
			return ret;
	     } 
    }

	return ret;
}

static int stmmac_rgmii_clk_enable(struct device *dev)
{
	struct clk *cfg_clk = NULL;	
	struct clk *ref_clk = NULL;
    struct clk *tx_clk = NULL;
	struct clk *ptp_clk = NULL;
    struct clk *rx_clk = NULL;
	int ret;

	/* open gmac aclk(bus config clock) */
	cfg_clk = devm_clk_get(dev,"gmac_aclk");
    if(IS_ERR(cfg_clk)){
            GMAC_ERR(("Can't find gmac_aclk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(cfg_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_aclk failed!\n"));
			return ret;
	     } 
    }
	
	/* Open gmac reference clock */
    ref_clk = devm_clk_get(dev,"gmac_refclk");
    if(IS_ERR(ref_clk)){
            GMAC_ERR(("Can't find gmac_refclk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(ref_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_refclk failed!\n"));
			return ret;
	     } 
    }

	/* Open tx clock */
	tx_clk = devm_clk_get(dev,"gmac_switch_clk");
    if(IS_ERR(tx_clk)){
            GMAC_ERR(("Can't find gmac_switch_clk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(tx_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_switch_clk failed!\n"));
			return ret;
	     } 
    }

	/* Open ptp clock */
	ptp_clk = devm_clk_get(dev,"gmac_ptp_clk");
    if(IS_ERR(ptp_clk)){
            GMAC_ERR(("Can't find gmac_ptp_clk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(ptp_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_ptp_clk failed!\n"));
			return ret;
	     } 
    }

	/* Open rx clock */
	rx_clk = devm_clk_get(dev,"gmac_rx_clk");
    if(IS_ERR(rx_clk)){
            GMAC_ERR(("Can't find gmac_rx_clk,please check!\n"));
            return -ENXIO;
    } else {
	    ret = clk_prepare_enable(rx_clk);
	    if(ret){
			GMAC_ERR(("Enable gmac_rx_clk failed!\n"));
			return ret;
	     } 
    }
	
	return ret;
}

int stmmac_clk_enable(struct device *dev)
{
	struct device_node *np;
	unsigned int phy_interface;
	int ret;

	np = dev->of_node;
	phy_interface = of_get_phy_mode(np);
	ret = stmmac_sel_phy_intf(dev->of_node, phy_interface);
	if (ret)
		GMAC_ERR(("set phy intf failed,error:0x%x, intf:0x%x\n", ret, phy_interface));
	
	switch (phy_interface) {
		case  PHY_INTERFACE_MODE_MII:
			ret = stmmac_mii_clk_enable(dev);
			break;

		case PHY_INTERFACE_MODE_RMII:
			ret = stmmac_rmii_clk_enable(dev);						
			break;

		case PHY_INTERFACE_MODE_RGMII:
			ret = stmmac_rgmii_clk_enable(dev);
			break;

		default:
			ret = -EINVAL;
			break;
	}
	
	if (ret)
		GMAC_ERR(("phy mode:%d,error:0x%x\n", phy_interface, ret));
	
	return ret;

}

static void stmmac_mii_clk_disable(struct device *dev)
{
	struct clk *cfg_clk = NULL;	
	struct clk *ref_clk = NULL;
    struct clk *tx_clk = NULL;
	struct clk *ptp_clk = NULL;
    struct clk *rx_clk = NULL;

	/* close gtx clock */
	tx_clk = devm_clk_get(dev,"gmac_switch_clk");
    if(IS_ERR(tx_clk)){
            GMAC_ERR(("Can't find gmac_switch_clk,please check!\n"));
    } else {
	    clk_disable_unprepare(tx_clk);
    }	

	/* close ptp clock */
	ptp_clk = devm_clk_get(dev,"gmac_ptp_clk");
    if(IS_ERR(ptp_clk)){
            GMAC_ERR(("Can't find gmac_ptp_clk,please check!\n"));
    } else {
	    clk_disable_unprepare(ptp_clk);
    }

	/* close gmac reference clock */
    ref_clk = devm_clk_get(dev,"gmac_refclk");
    if(IS_ERR(ref_clk)){
            GMAC_ERR(("Can't find gmac_refclk,please check!\n"));
    } else {
	    clk_disable_unprepare(ref_clk);
    }

	/* close rx clock */
	rx_clk = devm_clk_get(dev,"gmac_rx_clk");
    if(IS_ERR(rx_clk)){
            GMAC_ERR(("Can't find gmac_rx_clk,please check!\n"));
    } else {
	    clk_disable_unprepare(rx_clk);
    }

	/* close gmac aclk(bus config clock) */
	cfg_clk = devm_clk_get(dev,"gmac_aclk");
    if(IS_ERR(cfg_clk)){
            GMAC_ERR(("Can't find gmac_aclk,please check!\n"));
    } else {
	    clk_disable_unprepare(cfg_clk);
    }

}

static void stmmac_rmii_clk_disable(struct device *dev)
{
	struct clk *rmii_clk = NULL;
	struct clk *cfg_clk = NULL;	
	struct clk *ref_clk = NULL;
	struct clk *ptp_clk = NULL;

	/* close ptp clock */
	ptp_clk = devm_clk_get(dev,"gmac_ptp_clk");
    if(IS_ERR(ptp_clk)){
            GMAC_ERR(("Can't find gmac_ptp_clk,please check!\n"));
    } else {
	    clk_disable_unprepare(ptp_clk);
    }

	/* close gmac reference clock */
    ref_clk = devm_clk_get(dev,"gmac_refclk");
    if(IS_ERR(ref_clk)){
            GMAC_ERR(("Can't find gmac_refclk,please check!\n"));
    } else {
	    clk_disable_unprepare(ref_clk);
    }
	
	/* close rx and RMII clock */
	rmii_clk = devm_clk_get(dev,"gmac_rmii_clk");
    if(IS_ERR(rmii_clk)){
            GMAC_ERR(("Can't find gmac_rmii_clk,please check!\n"));
    } else {
	    clk_disable_unprepare(rmii_clk);
    }

	/* close gmac aclk(bus config clock) */
	cfg_clk = devm_clk_get(dev,"gmac_aclk");
    if(IS_ERR(cfg_clk)){
            GMAC_ERR(("Can't find gmac_aclk,please check!\n"));
    } else {
	    clk_disable_unprepare(cfg_clk);
    }
}

static void stmmac_rgmii_clk_disable(struct device *dev)
{
	struct clk *cfg_clk = NULL;
	struct clk *ref_clk = NULL;
    struct clk *tx_clk = NULL;
	struct clk *ptp_clk = NULL;
    struct clk *rx_clk = NULL;

	/* close rx clock */
	rx_clk = devm_clk_get(dev,"gmac_rx_clk");
    if(IS_ERR(rx_clk)){
            GMAC_ERR(("Can't find gmac_rx_clk,please check!\n"));
    } else {
	    clk_disable_unprepare(rx_clk); 
    }

	/* close ptp clock */
	ptp_clk = devm_clk_get(dev,"gmac_ptp_clk");
    if(IS_ERR(ptp_clk)){
            GMAC_ERR(("Can't find gmac_ptp_clk,please check!\n"));
    } else {
	    clk_disable_unprepare(ptp_clk);
    }

	/* close tx clock */
	tx_clk = devm_clk_get(dev,"gmac_switch_clk");
    if(IS_ERR(tx_clk)){
            GMAC_ERR(("Can't find gmac_switch_clk,please check!\n"));
    } else {
	    clk_disable_unprepare(tx_clk);
    }

	/* close gmac reference clock */
    ref_clk = devm_clk_get(dev,"gmac_refclk");
    if(IS_ERR(ref_clk)){
            GMAC_ERR(("Can't find gmac_refclk,please check!\n"));
    } else {
	    clk_disable_unprepare(ref_clk);
    }

	/* close gmac config clock */
	cfg_clk = devm_clk_get(dev,"gmac_aclk");
    if(IS_ERR(cfg_clk)){
            GMAC_ERR(("Can't find gmac_aclk,please check!\n"));
    } else {
	    clk_disable_unprepare(cfg_clk); 
    }
	
}

void stmmac_clk_disable(struct device *dev)
{
	struct device_node *np;
	unsigned int phy_interface;

	np = dev->of_node;
	phy_interface = of_get_phy_mode(np);
	switch (phy_interface) {
		case  PHY_INTERFACE_MODE_MII:
			stmmac_mii_clk_disable(dev);
			break;

		case PHY_INTERFACE_MODE_RMII:
			stmmac_rmii_clk_disable(dev);						
			break;

		case PHY_INTERFACE_MODE_RGMII:
			stmmac_rgmii_clk_disable(dev);
			break;

		default:
			GMAC_ERR(("[stmmac_clk_disable]:wrong interface!\n"));
			break;
	}

}

int stmmac_plat_init(struct platform_device *pdev)
{
	stmmac_srst_dis(pdev->dev.of_node);
	return stmmac_clk_enable(&pdev->dev);
}

extern struct platform_driver stmmac_pltfr_driver;
module_platform_driver(stmmac_pltfr_driver);

MODULE_DESCRIPTION("Balong STMMAC 10/100/1000 Ethernet PLATFORM driver");
MODULE_AUTHOR("Giuseppe Cavallaro <peppe.cavallaro@st.com>");
MODULE_LICENSE("GPL");
