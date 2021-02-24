/*******************************************************************************
  Copyright (C) 2013  Vayavya Labs Pvt Ltd

 * 2016-2-18 - Modifed code to adapt Synopsys DesignWare Cores Ethernet 
 * Quality-of-Service (DWC_ether_qos) core, 4.10a. 
 * liufangyuan <liufangyuan2@huawei.com>
 * Copyright (C) Huawei Technologies Co., Ltd.

  This implements all the API for managing HW timestamp & PTP.

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

  Author: Rayagond Kokatanur <rayagond@vayavyalabs.com>
  Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
*******************************************************************************/

#include <linux/io.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include "common.h"
#include "stmmac_ptp.h"
#include "dwmac_dma.h"

static void stmmac_config_hw_tstamping(void __iomem *ioaddr, u32 data)
{
	writel(data, ioaddr + GMAC_TIMESTAMP_CONTROL);
}

/* It write one period nano seconds into register */
static void stmmac_config_sub_second_increment(void __iomem *ioaddr)
{
	unsigned int value = readl(ioaddr + GMAC_TIMESTAMP_CONTROL);
	unsigned int ret;
	unsigned long data;

	/* Convert the ptp_clock to nano second
	 * formula = (1/ptp_clock) * 1000000000
	 * where, ptp_clock = 50MHz.
	 */
	data = (1000000000ULL / 50000000);

	/* 0.465ns accuracy */
	if (!(value & GMAC_TCR_TSCTRLSSR))
		data = (data * 1000) / 465;

	ret = readl(ioaddr + GMAC_SUB_SECOND_INCREMENT);
	ret = (ret & SNSINC_BIT) | (data << 16);
	writel(ret, ioaddr + GMAC_SUB_SECOND_INCREMENT);
}

static int stmmac_init_systime(void __iomem *ioaddr, u32 sec, u32 nsec)
{
	int limit;
	unsigned int value;
	
	/* wait for previous(if any) time initialize to complete, poll it. */
	limit = 100;
	while (limit--){
		if (!(readl(ioaddr + GMAC_TIMESTAMP_CONTROL) & GMAC_TCR_TSINIT))
			break;
		mdelay(1);
	}
	
	nsec &= ~(BIT(31));			//Add time
	writel(sec, ioaddr + GMAC_SYSTEM_TIME_SECONDS_UPDATE);
	writel(nsec, ioaddr + GMAC_SYSTEM_TIME_NANOSECONDS_UPDATE);
	
	/* issue command to initialize the system time value */
	value = readl(ioaddr + GMAC_TIMESTAMP_CONTROL);
	value |= GMAC_TCR_TSINIT;
	writel(value, ioaddr + GMAC_TIMESTAMP_CONTROL);

	/* wait for present system time initialize to complete, poll*/
	limit = 100;
	while (limit--) {
		if (!(readl(ioaddr + GMAC_TIMESTAMP_CONTROL) & GMAC_TCR_TSINIT))
			break;
		mdelay(1);
	}
	if (limit < 0)
		return -EBUSY;

	return 0;
}

static int stmmac_config_addend(void __iomem *ioaddr, u32 addend)
{
	u32 value;
	int limit;
	
	/* wait for previous(if any) added update to complete, poll it */
	limit = 100;
	while (limit--) {
		if (!(readl(ioaddr + GMAC_TIMESTAMP_CONTROL) & GMAC_TCR_TSADDREG))
			break;
		mdelay(1);
	}
	if (limit < 0)
		return -EBUSY;
	
	writel(addend, ioaddr + GMAC_TIMESTAMP_ADDEND);
	
	/* issue command to update the addend value */
	value = readl(ioaddr + GMAC_TIMESTAMP_CONTROL);
	value |= GMAC_TCR_TSADDREG;
	writel(value, ioaddr + GMAC_TIMESTAMP_CONTROL);

	/* wait for present addend update to complete */
	limit = 100;
	while (limit--) {
		if (!(readl(ioaddr + GMAC_TIMESTAMP_CONTROL) & GMAC_TCR_TSADDREG))
			break;
		mdelay(1);
	}
	if (limit < 0)
		return -EBUSY;

	return 0;
}

static int stmmac_adjust_systime(void __iomem *ioaddr, u32 sec, u32 nsec,
				 int add_sub)
{
	unsigned int value;
	int limit;
	
	/* wait for previous(if any) time adjust/update to complete. */
	limit = 10;
	while (limit--) {
		if (!(readl(ioaddr + GMAC_TIMESTAMP_CONTROL) & GMAC_TCR_TSUPDT))
			break;
		mdelay(10);
	}
	if (limit < 0)
		return -EBUSY;

	if (add_sub) {
		/* If the new sec value needs to be subtracted with
	     * the system time, then GMAC_SYSTEM_TIME_SECONDS_UPDATE reg should be
	     * programmed with (2^32 - <new_sec_value>)
	     * */
	    sec = (0x100000000ull - sec);

	    /* If the new nsec value need to be subtracted with
	     * the system time, then GMAC_SYSTEM_TIME_SECONDS_UPDATE's TSSS field 
	     * should be programmed with,
	     * (10^9 - <new_nsec_value>) if TSCTRLSSR is set or
	     * (2^31 - <new_nsec_value> if TSCTRLSSR is reset)
	     * */
	     value = readl(ioaddr + GMAC_TIMESTAMP_CONTROL) & GMAC_TCR_TSCTRLSSR;
	  	if (value)
	      nsec = (0x3B9ACA00 - nsec);
	   	else
	      nsec = (0x80000000 - nsec);
	}
	
	writel(sec, ioaddr + GMAC_SYSTEM_TIME_SECONDS_UPDATE);
	writel(((add_sub << GMAC_STNSUR_ADDSUB_SHIFT) | nsec), 
				ioaddr + GMAC_SYSTEM_TIME_NANOSECONDS_UPDATE);
	
	/* issue command to initialize the system time value */
	value = readl(ioaddr + GMAC_TIMESTAMP_CONTROL);
	value |= GMAC_TCR_TSUPDT;
	writel(value, ioaddr + GMAC_TIMESTAMP_CONTROL);

	/* wait for present system time adjust/update to complete */
	limit = 100;
	while (limit--) {
		if (!(readl(ioaddr + GMAC_TIMESTAMP_CONTROL) & GMAC_TCR_TSUPDT))
			break;
		mdelay(1);
	}
	if (limit < 0)
		return -EBUSY;

	return 0;
}

static u64 stmmac_get_systime(void __iomem *ioaddr)
{
	u64 ns;

	ns = readl(ioaddr + GMAC_SYSTEM_TIME_NANOSECONDS) & (~(BIT(31)));
	/* convert sec time value to nanosecond */
	ns += readl(ioaddr + GMAC_SYSTEM_TIME_SECONDS) * 1000000000ULL;

	return ns;
}

static int ptp_init(void __iomem *ioaddr)
{
	int value;
	int limit;
	
	/* Mask the timestamp trigger interrupt */
	value = readl(ioaddr + GMAC_INT_EN);
	value &= ~(BIT(12));
	writel(value, ioaddr + GMAC_INT_EN);

	/* Enable timestamping */
	value = readl(ioaddr + GMAC_TIMESTAMP_CONTROL);
	value |= BIT(0);
	writel(value, ioaddr + GMAC_TIMESTAMP_CONTROL);
	
	/* The value programmed is accumulated every clock cycle. */
	writel(GMAC_SSINC | GMAC_SNSINC, ioaddr + GMAC_SUB_SECOND_INCREMENT);

#if 0
	/* Program mac timestamp added */
	value = readl(ioaddr + GMAC_TIMESTAMP_ADDEND);
	writel(0, ioaddr + GMAC_TIMESTAMP_ADDEND);

	/* Poll the MAC_Timestamp_Control Register until Bit 5 is cleared. */
	limit = 100;
	while (limit--) {
		if (!(readl(ioaddr + GMAC_TIMESTAMP_CONTROL) & BIT(5)))
			break;
		mdelay(10);
	}
	if (limit < 0)
		return -EBUSY;
#endif
	
	/* Program system time update value */
	writel(0, ioaddr + GMAC_SYSTEM_TIME_SECONDS_UPDATE);
	writel(0, ioaddr + GMAC_SYSTEM_TIME_NANOSECONDS_UPDATE);

	/* Initialize Timestamp, ptp over ethernet, IEEE 1588 version 2,etc */
	value = readl(ioaddr + GMAC_TIMESTAMP_CONTROL);
	value |= GMAC_TSC_TSCFUPDT | GMAC_TSC_TSINIT | GMAC_TSC_TSVER2ENA | 
				GMAC_TSC_TSIPENA | GMAC_TSC_SNAPTYPSEL | GMAC_TSC_TSMSTRENA |
				GMAC_TSC_TSEVNTENA;
	writel(value, ioaddr + GMAC_TIMESTAMP_CONTROL);

	limit = 100;
	while (limit--) {
		if (!(readl(ioaddr + GMAC_TIMESTAMP_CONTROL) & BIT(2)))
			break;
		mdelay(10);
	}
	if (limit < 0)
		return -EBUSY;

	/* enable timestamp trigger interrupt */
	value = readl(ioaddr + GMAC_INT_EN);
	value |= BIT(12);						
	writel(value, ioaddr + GMAC_INT_EN);

	return 0;
}

void stmmac_cfg_updt_method(void __iomem *ioaddr, unsigned int method)
{
	int value;

	if (method) {
		method = 1;
	}
		
	value = readl(ioaddr + GMAC_TIMESTAMP_CONTROL);
	writel(value| method, ioaddr + GMAC_TIMESTAMP_CONTROL);	
}

static int stmmac_ptp_ioctl(struct net_device *ndev, struct ifreq *ifr, int cmd)
{
	return 0;
}

const struct stmmac_hwtimestamp stmmac_ptp = {
	.init = ptp_init,
	.config_hw_tstamping = stmmac_config_hw_tstamping,
	.init_systime = stmmac_init_systime,
	.config_sub_second_increment = stmmac_config_sub_second_increment,
	.config_addend = stmmac_config_addend,
	.adjust_systime = stmmac_adjust_systime,
	.get_systime = stmmac_get_systime,
	.config_updt_method = stmmac_cfg_updt_method,
	.ptp_ioctl = stmmac_ptp_ioctl,
};

static void stmmac_config_slot_compare(void __iomem *ioaddr, int chn,
									unsigned int compare)
{
	unsigned int value;

	value = readl(ioaddr + DMA_CHN_SFCS(chn));
	value = (value & (~(BIT(0)))) | (!!compare);
	writel(value, ioaddr + DMA_CHN_SFCS(chn));
}

static void stmmac_config_slot_advance_check(void __iomem *ioaddr,
								int chn, unsigned int check)
{
	unsigned int value;

	value = readl(ioaddr + DMA_CHN_SFCS(chn));
	value = (value & (~(BIT(1)))) | ((!!check) << 1);
	writel(value, ioaddr + DMA_CHN_SFCS(chn));
}

const struct stmmac_slot_ops stmmac_slot = {
	.config_slot_compare = stmmac_config_slot_compare,
	.config_slot_advance_check = stmmac_config_slot_advance_check,
};