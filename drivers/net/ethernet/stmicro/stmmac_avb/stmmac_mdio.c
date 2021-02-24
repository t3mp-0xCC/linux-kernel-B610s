/*******************************************************************************
  STMMAC Ethernet Driver -- MDIO bus implementation
  Provides Bus interface for MII registers

  Copyright (C) 2007-2009  STMicroelectronics Ltd

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

  Author: Carl Shaw <carl.shaw@st.com>
  Maintainer: Giuseppe Cavallaro <peppe.cavallaro@st.com>
*******************************************************************************/

#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/slab.h>
#include <asm/io.h>

#include "stmmac.h"

#define MII_BUSY 0x00000001
#define MII_WRITE 0x00000004
#define MII_READ 0x0000000C
struct mii_bus *g_bus;

static int stmmac_mdio_busy_wait(void __iomem *ioaddr, unsigned int mii_addr)
{
	unsigned long curr;
	unsigned long finish = jiffies + 3 * HZ;

	do {
		curr = jiffies;
		if (readl(ioaddr + mii_addr) & MII_BUSY)
			cpu_relax();
		else
			return 0;
	} while (!time_after_eq(curr, finish));

	return -EBUSY;
}

/**
 * stmmac_mdio_read
 * @bus: points to the mii_bus structure
 * @phyaddr: MII addr reg bits 15-11
 * @phyreg: MII addr reg bits 10-6
 * Description: it reads data from the MII register from within the phy device.
 * For the 7111 GMAC, we must set the bit 0 in the MII address register while
 * accessing the PHY registers.
 * Fortunately, it seems this has no drawback for the 7109 MAC.
 */
static int stmmac_mdio_read(struct mii_bus *bus, int phyaddr, int phyreg)
{
	struct net_device *ndev = bus->priv;
	struct stmmac_priv *priv = netdev_priv(ndev);
	unsigned int mii_address = priv->hw->mii.addr;
	unsigned int mii_data = priv->hw->mii.data;
	unsigned int reg_address;

	int data;
	u32 regValue = (((phyaddr << 21) & (0x03E00000)) |
			((phyreg << 16) & (0x001F0000)) | MII_READ);
	regValue |= MII_BUSY | ((priv->clk_csr & 0xF) << 8);

	if (stmmac_mdio_busy_wait(priv->ioaddr, mii_address))
		return -EBUSY;
	
	reg_address =  readl(priv->ioaddr + mii_address);
	reg_address =  regValue | (reg_address & (unsigned long)(0x12));
	writel(regValue, priv->ioaddr + mii_address);

	if (stmmac_mdio_busy_wait(priv->ioaddr, mii_address))
		return -EBUSY;

	/* Read the data from the MII data register */
	data = (int)readl(priv->ioaddr + mii_data);
	data &= GMAC_MDIO_DATA_GD;

	return data;
}

/**
 * stmmac_mdio_write
 * @bus: points to the mii_bus structure
 * @phyaddr: MII addr reg bits 15-11
 * @phyreg: MII addr reg bits 10-6
 * @phydata: phy data
 * Description: it writes the data into the MII register from within the device.
 */
static int stmmac_mdio_write(struct mii_bus *bus, int phyaddr, int phyreg,
			     u16 phydata)
{
	struct net_device *ndev = bus->priv;
	struct stmmac_priv *priv = netdev_priv(ndev);
	unsigned int mii_address = priv->hw->mii.addr;
	unsigned int mii_data = priv->hw->mii.data;
	unsigned int reg_data;
	unsigned int reg_address;

	/* initiate the MII write operation by updating desired */
	/* phy address/id (0 - 31) */
	/* phy register offset */
	/* CSR Clock Range (20 - 35MHz) */
	/* Select write operation */
	/* set busy bit */
	u32 value =
	    (((phyaddr << 21) & (0x03E00000)) | ((phyreg << 16) & (0x001F0000)))
	    | MII_WRITE;

	value |= MII_BUSY | ((priv->clk_csr & 0xF) << 8);

	/* Wait until any existing MII operation is complete */
	if (stmmac_mdio_busy_wait(priv->ioaddr, mii_address))
		return -EBUSY;

	/* Set the MII address register to write */
	reg_data = readl(priv->ioaddr + mii_data);
	reg_data = (reg_data & GMAC_MDIO_DATA_RA) | 
					((unsigned long)phydata & GMAC_MDIO_DATA_GD);
	
	writel(reg_data, priv->ioaddr + mii_data);

	reg_address = readl(priv->ioaddr + mii_address);
	reg_address = value | (reg_address & (unsigned long)(0x12));
	writel(reg_address, priv->ioaddr + mii_address);

	/* Wait until any existing MII operation is complete */
	return stmmac_mdio_busy_wait(priv->ioaddr, mii_address);
}

/**
 * stmmac_mdio_reset
 * @bus: points to the mii_bus structure
 * Description: reset the MII bus
 */
static int stmmac_mdio_reset(struct mii_bus *bus)
{
#if defined(CONFIG_STMMAC_PLATFORM)
	struct net_device *ndev = bus->priv;
	struct stmmac_priv *priv = netdev_priv(ndev);
	unsigned int mii_address = priv->hw->mii.addr;

	if (priv->plat->mdio_bus_data->phy_reset) {
		pr_debug("stmmac_mdio_reset: calling phy_reset\n");
		priv->plat->mdio_bus_data->phy_reset(priv->plat->bsp_priv);
	}

	/* This is a workaround for problems with the STE101P PHY.
	 * It doesn't complete its reset until at least one clock cycle
	 * on MDC, so perform a dummy mdio read.
	 */
	writel(0, priv->ioaddr + mii_address);
#endif
	return 0;
}

/**
 * stmmac_mdio_register
 * @ndev: net device structure
 * Description: it registers the MII bus
 */
int stmmac_mdio_register(struct net_device *ndev)
{
	int err = 0;
	struct mii_bus *new_bus;
	int *irqlist;
	struct stmmac_priv *priv = netdev_priv(ndev);
	struct stmmac_mdio_bus_data *mdio_bus_data = priv->plat->mdio_bus_data;
	int addr, found;

	if (!mdio_bus_data)
		return 0;

	new_bus = mdiobus_alloc();
	if (new_bus == NULL)
		return -ENOMEM;

	if (mdio_bus_data->irqs)
		irqlist = mdio_bus_data->irqs;
	else
		irqlist = priv->mii_irq;

	new_bus->name = "stmmac";
	new_bus->read = &stmmac_mdio_read;
	new_bus->write = &stmmac_mdio_write;
	new_bus->reset = &stmmac_mdio_reset;
	snprintf(new_bus->id, MII_BUS_ID_SIZE, "%s-%x",
		 new_bus->name, priv->plat->bus_id);
	new_bus->priv = ndev;
	new_bus->irq = irqlist;
	new_bus->phy_mask = mdio_bus_data->phy_mask;
	new_bus->parent = priv->device;
	err = mdiobus_register(new_bus);
	if (err != 0) {
		pr_err("%s: Cannot register as MDIO bus\n", new_bus->name);
		goto bus_register_fail;
	}

	found = 0;
	for (addr = 0; addr < PHY_MAX_ADDR; addr++) {
		struct phy_device *phydev = new_bus->phy_map[addr];
		if (phydev) {
			int act = 0;
			char irq_num[4];
			char *irq_str;

			/*
			 * If an IRQ was provided to be assigned after
			 * the bus probe, do it here.
			 */
			if ((mdio_bus_data->irqs == NULL) &&
			    (mdio_bus_data->probed_phy_irq > 0)) {
				irqlist[addr] = mdio_bus_data->probed_phy_irq;
				phydev->irq = mdio_bus_data->probed_phy_irq;
			}

			/*
			 * If we're  going to bind the MAC to this PHY bus,
			 * and no PHY number was provided to the MAC,
			 * use the one probed here.
			 */
			if (priv->plat->phy_addr == -1)
				priv->plat->phy_addr = addr;

			act = (priv->plat->phy_addr == addr);
			switch (phydev->irq) {
			case PHY_POLL:
				irq_str = "POLL";
				break;
			case PHY_IGNORE_INTERRUPT:
				irq_str = "IGNORE";
				break;
			default:
				snprintf(irq_num, sizeof(irq_num), "%d", phydev->irq);
				irq_str = irq_num;
				break;
			}
			pr_info("%s: PHY ID %08x at %d IRQ %s (%s)%s\n",
				ndev->name, phydev->phy_id, addr,
				irq_str, dev_name(&phydev->dev),
				act ? " active" : "");
			found = 1;
		}
	}

	if (!found) {
		pr_warning("%s: No PHY found\n", ndev->name);
		mdiobus_unregister(new_bus);
		mdiobus_free(new_bus);
		return -ENODEV;
	}

	priv->mii = new_bus;
	g_bus = new_bus;

	return 0;

bus_register_fail:
	mdiobus_free(new_bus);
	return err;
}

/**
 * stmmac_mdio_unregister
 * @ndev: net device structure
 * Description: it unregisters the MII bus
 */
int stmmac_mdio_unregister(struct net_device *ndev)
{
	struct stmmac_priv *priv = netdev_priv(ndev);

	if (!priv->mii)
		return 0;

	mdiobus_unregister(priv->mii);
	priv->mii->priv = NULL;
	mdiobus_free(priv->mii);
	priv->mii = NULL;

	return 0;
}

/* phyaddr:PHY ID,fixed to 1 in RGMII
 * phyreg: PHY register offset
 */
int stmmac_phy_read(int phyaddr, int phyreg)
{
	return stmmac_mdio_read(g_bus, phyaddr, phyreg);
}
EXPORT_SYMBOL(stmmac_phy_read);

int stmmac_phy_write(int phyaddr, int phyreg, u16 phydata)
{
	return stmmac_mdio_write(g_bus, phyaddr, phyreg, phydata);
}
EXPORT_SYMBOL(stmmac_phy_write);

