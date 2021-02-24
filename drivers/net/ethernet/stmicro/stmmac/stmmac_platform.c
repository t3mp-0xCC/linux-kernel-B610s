/*******************************************************************************
  This contains the functions to handle the platform driver.

  Copyright (C) 2007-2011  STMicroelectronics Ltd
  Copyright (C) 2015 Huawei Technologies Co., Ltd.
  2015-04-02 - fix GMAC define error. <foss@huawei.com>

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
#include <linux/gpio.h>
#include "stmmac.h"

#ifdef CONFIG_BALONG_GMAC
extern int stmmac_plat_init(struct platform_device *pdev);
static struct stmmac_mdio_bus_data stmmac_mdio_data = {
#if (FEATURE_ON == MBB_FEATURE_ETH)
/* phy芯片 8367，Pin82若上拉, 则表示PHYID=29。Pin82若下拉, 则表示PHYID=0 */
/* B612 使用的 8367，硬件设计 Pin82 下拉，所以这里要把 phy_mask改成 0对应的掩码 */
#if defined(BSP_CONFIG_BOARD_CPE_B612) || defined(BSP_CONFIG_BOARD_CPE_B715)
    .phy_mask = 0xFFFFFFFE,
#else
    .phy_mask = 0xDFFFFFFF,
#endif
#else
    .phy_mask = 0xFFFFFFFC,
#endif
};

#ifdef V7R5_UDP_BOARD
int hisi_v7r5_udp_phy_reset(void *priv)
{

	
	if(gpio_request(7, "phy_reset"))
	{
		printk("gpio request failed!\n");
		return -1;
	}
	
	gpio_direction_output(7,1);
	udelay(40);
	gpio_direction_output(7,0);
	udelay(40);
	gpio_direction_output(7,1);

	return 0;
}
#endif

#endif

#ifdef CONFIG_OF
static int stmmac_probe_config_dt(struct platform_device *pdev,
				  struct plat_stmmacenet_data *plat,
				  const char **mac)
{
	struct device_node *np = pdev->dev.of_node;

	if (!np)
		return -ENODEV;

	*mac = of_get_mac_address(np);
	plat->interface = of_get_phy_mode(np);
#ifdef CONFIG_BALONG_GMAC
	plat->init = stmmac_plat_init;
	plat->dma_cfg = devm_kzalloc(&pdev->dev,
					   sizeof(struct stmmac_dma_cfg),
					   GFP_KERNEL);
	if(!plat->dma_cfg){
		pr_err("%s: ERROR: no memory", __func__);
		return -ENOMEM;
	}
#endif
    plat->mdio_bus_data = devm_kzalloc(&pdev->dev,
					   sizeof(struct stmmac_mdio_bus_data),
					   GFP_KERNEL);
	if(!plat->mdio_bus_data){
		pr_err("%s: ERROR: no memory", __func__);
		return -ENOMEM;
	}
#ifdef V7R5_UDP_BOARD
	plat->mdio_bus_data->phy_reset = hisi_v7r5_udp_phy_reset;
#endif
	/*
	 * Currently only the properties needed on SPEAr600
	 * are provided. All other properties should be added
	 * once needed on other platforms.
	 */
	if (of_device_is_compatible(np, "st,spear600-gmac") ||
		of_device_is_compatible(np, "snps,dwmac-3.70a") ||
		of_device_is_compatible(np, "snps,dwmac")) {
		plat->has_gmac = 1;
		plat->pmt = 1;
	}
#ifdef CONFIG_BALONG_GMAC
	if (of_device_is_compatible(np, "hisilicon,dwmac")) {
        plat->bus_id = 0;
        plat->phy_addr = -1;
		plat->clk_csr = 0;
        plat->has_gmac = 1;
        plat->enh_desc = 1;
		plat->enh_desc_atds= 0;
        plat->force_sf_dma_mode = 1;
        plat->mdio_bus_data->phy_mask = stmmac_mdio_data.phy_mask;
		plat->dma_cfg->fixed_burst = 1;
		plat->dma_cfg->burst_len = (DMA_AXI_BLEN_16 | (0xFF << 16));
		plat->dma_cfg->pbl = 16;
	}
#endif
	return 0;
}
#else
static int stmmac_probe_config_dt(struct platform_device *pdev,
				  struct plat_stmmacenet_data *plat,
				  const char **mac)
{
	return -ENOSYS;
}
#endif /* CONFIG_OF */

/**
 * stmmac_pltfr_probe
 * @pdev: platform device pointer
 * Description: platform_device probe function. It allocates
 * the necessary resources and invokes the main to init
 * the net device, register the mdio bus etc.
 */
static int stmmac_pltfr_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct resource *res;
	struct device *dev = &pdev->dev;
	void __iomem *addr = NULL;
	struct stmmac_priv *priv = NULL;
	struct plat_stmmacenet_data *plat_dat = NULL;
	const char *mac = NULL;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	addr = devm_ioremap_resource(dev, res);
	if (IS_ERR(addr))
		return PTR_ERR(addr);

	if (pdev->dev.of_node) {
		plat_dat = devm_kzalloc(&pdev->dev,
					sizeof(struct plat_stmmacenet_data),
					GFP_KERNEL);
		if (!plat_dat) {
			pr_err("%s: ERROR: no memory", __func__);
			return  -ENOMEM;
		}

		ret = stmmac_probe_config_dt(pdev, plat_dat, &mac);
		if (ret) {
			pr_err("%s: main dt probe failed", __func__);
			return ret;
		}
	} else {
		plat_dat = pdev->dev.platform_data;
	}

	/* Custom initialisation (if needed)*/
	if (plat_dat->init) {
		ret = plat_dat->init(pdev);
		if (unlikely(ret))
			return ret;
	}

	priv = stmmac_dvr_probe(&(pdev->dev), plat_dat, addr);
	if (!priv) {
		pr_err("%s: main driver probe failed", __func__);
		return -ENODEV;
	}

	/* Get MAC address if available (DT) */
	if (mac)
		memcpy(priv->dev->dev_addr, mac, ETH_ALEN);

	/* Get the MAC information */
	priv->dev->irq = platform_get_irq_byname(pdev, "macirq");
	if (priv->dev->irq == -ENXIO) {
		pr_err("%s: ERROR: MAC IRQ configuration "
		       "information not found\n", __func__);
		return -ENXIO;
	}

	/*
	 * On some platforms e.g. SPEAr the wake up irq differs from the mac irq
	 * The external wake up irq can be passed through the platform code
	 * named as "eth_wake_irq"
	 *
	 * In case the wake up interrupt is not passed from the platform
	 * so the driver will continue to use the mac irq (ndev->irq)
	 */
	priv->wol_irq = platform_get_irq_byname(pdev, "eth_wake_irq");
	if (priv->wol_irq == -ENXIO)
		priv->wol_irq = priv->dev->irq;

	priv->lpi_irq = platform_get_irq_byname(pdev, "eth_lpi");

	platform_set_drvdata(pdev, priv->dev);

#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH_V1310)
    /** 海思gmac初始化完成后,不关闭clock.
        关闭clock会造成部分lan switch芯片rgmii buffer紊乱.**/
#else
    stmmac_clk_disable(dev);
#endif

	pr_debug("STMMAC platform driver registration completed");

	return 0;
}

/**
 * stmmac_pltfr_remove
 * @pdev: platform device pointer
 * Description: this function calls the main to free the net resources
 * and calls the platforms hook and release the resources (e.g. mem).
 */
static int stmmac_pltfr_remove(struct platform_device *pdev)
{
	struct net_device *ndev = 0;
	struct stmmac_priv *priv = 0;
	int ret = 0;

	ndev = (struct net_device *)platform_get_drvdata(pdev);
	if(!ndev){
		pr_err("%s: get drvdata failed\n", __func__);
		return -ENODEV;
	}

	priv = (struct stmmac_priv *)netdev_priv(ndev);
	
	ret = stmmac_dvr_remove(ndev);

	if (priv->plat->exit)
		priv->plat->exit(pdev);

	platform_set_drvdata(pdev, NULL);

	return ret;
}

#ifdef CONFIG_PM
static int stmmac_pltfr_suspend(struct device *dev)
{
	struct net_device *ndev = dev_get_drvdata(dev);

	return stmmac_suspend(ndev);
}

static int stmmac_pltfr_resume(struct device *dev)
{
	struct net_device *ndev = dev_get_drvdata(dev);

	return stmmac_resume(ndev);
}

int stmmac_pltfr_freeze(struct device *dev)
{
	int ret;
	struct plat_stmmacenet_data *plat_dat = dev_get_platdata(dev);
	struct net_device *ndev = dev_get_drvdata(dev);
	struct platform_device *pdev = to_platform_device(dev);

	ret = stmmac_freeze(ndev);
	if (plat_dat->exit)
		plat_dat->exit(pdev);

	return ret;
}

int stmmac_pltfr_restore(struct device *dev)
{
	struct plat_stmmacenet_data *plat_dat = dev_get_platdata(dev);
	struct net_device *ndev = dev_get_drvdata(dev);
	struct platform_device *pdev = to_platform_device(dev);

	if (plat_dat->init)
		plat_dat->init(pdev);

	return stmmac_restore(ndev);
}

static const struct dev_pm_ops stmmac_pltfr_pm_ops = {
	.suspend = stmmac_pltfr_suspend,
	.resume = stmmac_pltfr_resume,
	.freeze = stmmac_pltfr_freeze,
	.thaw = stmmac_pltfr_restore,
	.restore = stmmac_pltfr_restore,
};
#else
static const struct dev_pm_ops stmmac_pltfr_pm_ops;
#endif /* CONFIG_PM */

static const struct of_device_id stmmac_dt_ids[] = {
	{ .compatible = "st,spear600-gmac"},
	{ .compatible = "snps,dwmac-3.70a"},
	{ .compatible = "snps,dwmac"},
#ifdef CONFIG_BALONG_GMAC
	{ .compatible = "hisilicon,dwmac"},
#endif
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, stmmac_dt_ids);

struct platform_driver stmmac_pltfr_driver = {
	.probe = stmmac_pltfr_probe,
	.remove = stmmac_pltfr_remove,
	.driver = {
		   .name = STMMAC_RESOURCE_NAME,
		   .owner = THIS_MODULE,
		   .pm = &stmmac_pltfr_pm_ops,
		   .of_match_table = of_match_ptr(stmmac_dt_ids),
		   },
};

#if (FEATURE_ON == MBB_FEATURE_ETH_PHY)
extern int ar8035_phy_hwreset(void *priv);
#endif
MODULE_DESCRIPTION("STMMAC 10/100/1000 Ethernet PLATFORM driver");
MODULE_AUTHOR("Giuseppe Cavallaro <peppe.cavallaro@st.com>");
MODULE_LICENSE("GPL");
