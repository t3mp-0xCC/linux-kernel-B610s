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

int stmmac_clk_enable(struct device *dev)
{
    struct clk *aclk = NULL;
    struct clk *swclk = NULL;
    struct clk *gtx = NULL;
    struct clk *tx = NULL;
    struct clk *rx = NULL;
    int ret = 0;

    aclk =devm_clk_get(dev,"gmac_aclk");
    if(IS_ERR(aclk)){
            GMAC_ERR(("clk:gmac_aclk is NULL, can't find it,please check!!\n"));
            return 1;
    } else {
	    ret = clk_prepare_enable(aclk);
	    if(!ret){
	            GMAC_TRACE(("clk:gmac_aclk enable OK!!\n"));
	     } 
    }
    
    swclk = devm_clk_get(dev,"gmac_switch_clk");
    if(IS_ERR(swclk)){
            GMAC_ERR(("clk:gmac_swclk is NULL, can't find it,please check!!\n"));
            return 1;
    }else{
		ret = clk_prepare_enable(swclk);
	    if(!ret){
	            GMAC_TRACE(("clk:gmac_swclk enable OK!!\n"));
	     } 
    }
    
    gtx = devm_clk_get(dev,"gmac_gtx_clk");
    if(IS_ERR(gtx)){
            GMAC_ERR(("clk:gmac_gtx is NULL, can't find it,please check!!\n"));
            return 1;
    }else{
		ret = clk_prepare_enable(gtx);
	    if(!ret){
	            GMAC_TRACE(("clk:gmac_gtx enable OK!!\n"));
	     } 
    }
    
    tx = devm_clk_get(dev,"gmac_tx_clk");
    if(IS_ERR(tx)){
            GMAC_ERR(("clk:gmac_tx is NULL, can't find it,please check!!\n"));
            return 1;
    }else{
		ret = clk_prepare_enable(tx);
	    if(!ret){
	            GMAC_TRACE(("clk:gmac_tx enable OK!!\n"));
	     } 
    }
	
    rx = devm_clk_get(dev,"gmac_rx_clk");
    if(IS_ERR(rx)){
            GMAC_ERR(("clk:gmac_rx is NULL, can't find it,please check!!\n"));
            return 1;
    }else{
		ret = clk_prepare_enable(rx);
	    if(!ret){
	            GMAC_TRACE(("clk:gmac_rx enable OK!!\n"));
	    } 
    }
	
    return ret;
}

int stmmac_clk_disable(struct device *dev)
{
    struct clk *aclk = NULL;
    struct clk *swclk = NULL;
    struct clk *gtx = NULL;
    struct clk *tx = NULL;
    struct clk *rx = NULL;

    aclk =devm_clk_get(dev,"gmac_aclk");
    if(IS_ERR(aclk)){
            GMAC_ERR(("clk:gmac_aclk is NULL, can't find it,please check!!\n"));
            return 1;
    } else {
	    clk_disable_unprepare(aclk);
    }
    
    swclk = devm_clk_get(dev,"gmac_switch_clk");
    if(IS_ERR(swclk)){
            GMAC_ERR(("clk:gmac_swclk is NULL, can't find it,please check!!\n"));
            return 1;
    }else{
		clk_disable_unprepare(swclk);
    }
    
    gtx = devm_clk_get(dev,"gmac_gtx_clk");
    if(IS_ERR(gtx)){
            GMAC_ERR(("clk:gmac_gtx is NULL, can't find it,please check!!\n"));
            return 1;
    }else{
		clk_disable_unprepare(gtx);
    }
    
    tx = devm_clk_get(dev,"gmac_tx_clk");
    if(IS_ERR(tx)){
            GMAC_ERR(("clk:gmac_tx is NULL, can't find it,please check!!\n"));
            return 1;
    }else{
		clk_disable_unprepare(tx);
    }
	
    rx = devm_clk_get(dev,"gmac_rx_clk");
    if(IS_ERR(rx)){
            GMAC_ERR(("clk:gmac_rx is NULL, can't find it,please check!!\n"));
            return 1;
    }else{
		clk_disable_unprepare(rx);
    }
	
    return 0;
}

void stmmac_sysctrl_set(const struct device_node *np, const char *propname, int value)
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

void stmmac_mii_setup(struct device_node *np)
{
	int interface;
	
	interface = of_get_phy_mode(np);

	switch(interface)
	{
	case PHY_INTERFACE_MODE_RGMII:
		printk("interface is PHY_INTERFACE_MODE_RGMII\n");
		break;
	case PHY_INTERFACE_MODE_MII:
		printk("interface is PHY_INTERFACE_MODE_MII\n");
		break;
	case PHY_INTERFACE_MODE_RMII:
		printk("interface is PHY_INTERFACE_MODE_RMII\n");
		break;
	default:
		printk("mii not supported!\n");
		break;
	}
}

int stmmac_plat_init(struct platform_device *pdev)
{
	stmmac_srst_dis(pdev->dev.of_node);

	stmmac_mii_setup(pdev->dev.of_node);
	
	return stmmac_clk_enable(&pdev->dev);
}

extern struct platform_driver stmmac_pltfr_driver;
module_platform_driver(stmmac_pltfr_driver);

MODULE_DESCRIPTION("Balong STMMAC 10/100/1000 Ethernet PLATFORM driver");
MODULE_AUTHOR("Giuseppe Cavallaro <peppe.cavallaro@st.com>");
MODULE_LICENSE("GPL");
