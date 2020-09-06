/* Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/sysfs.h>
#include <linux/stat.h>
#include <linux/spinlock.h>
#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/of.h>
#include <asm/sections.h>

#include "coresight-priv.h"
#include "bsp_coresight.h"
#include "coresight.h"
#include "of_coresight.h"

#define etm_writel_mm(drvdata, val, off)  \
			__raw_writel((val), drvdata->base + off)
#define etm_readl_mm(drvdata, off)        \
			__raw_readl(drvdata->base + off)

#define etm_writel(drvdata, val, off)					\
({									\
	if (drvdata->use_cp14)						\
		etm_writel_cp14(val, off);				\
	else								\
		etm_writel_mm(drvdata, val, off);			\
})
#define etm_readl(drvdata, off)						\
({									\
	uint32_t val;							\
	if (drvdata->use_cp14)						\
		val = etm_readl_cp14(off);				\
	else								\
		val = etm_readl_mm(drvdata, off);			\
	val;								\
})

#define ETM_LOCK(drvdata)						\
do {									\
	/* Recommended by spec to ensure ETM writes are committed */	\
	/* prior to resuming execution */				\
	mb();								\
	isb();								\
	etm_writel_mm(drvdata, 0x0, CORESIGHT_LAR);			\
} while (0)
#define ETM_UNLOCK(drvdata)						\
do {									\
	etm_writel_mm(drvdata, CORESIGHT_UNLOCK, CORESIGHT_LAR);	\
	/* Ensure unlock and any pending writes are committed prior */	\
	/* to programming ETM registers */				\
	mb();								\
	isb();								\
} while (0)

#define PORT_SIZE_MASK		(BM(21, 21) | BM(4, 6))

/*
 * Device registers:
 * 0x000 - 0x2FC: Trace		registers
 * 0x300 - 0x314: Management	registers
 * 0x318 - 0xEFC: Trace		registers
 *
 * Coresight registers
 * 0xF00 - 0xF9C: Management	registers
 * 0xFA0 - 0xFA4: Management	registers in PFTv1.0
 *		  Trace		registers in PFTv1.1
 * 0xFA8 - 0xFFC: Management	registers
 */

/* Trace registers (0x000-0x2FC) */
#define ETMCR			(0x000)
#define ETMCCR			(0x004)
#define ETMTRIGGER		(0x008)
#define ETMSR			(0x010)
#define ETMSCR			(0x014)
#define ETMTSSCR		(0x018)
#define ETMTECR2		(0x01c)
#define ETMTEEVR		(0x020)
#define ETMTECR1		(0x024)
#define ETMFFLR			(0x02C)
#define ETMACVRn(n)		(0x040 + (n * 4))
#define ETMACTRn(n)		(0x080 + (n * 4))
#define ETMCNTRLDVRn(n)		(0x140 + (n * 4))
#define ETMCNTENRn(n)		(0x150 + (n * 4))
#define ETMCNTRLDEVRn(n)	(0x160 + (n * 4))
#define ETMCNTVRn(n)		(0x170 + (n * 4))
#define ETMSQ12EVR		(0x180)
#define ETMSQ21EVR		(0x184)
#define ETMSQ23EVR		(0x188)
#define ETMSQ31EVR		(0x18C)
#define ETMSQ32EVR		(0x190)
#define ETMSQ13EVR		(0x194)
#define ETMSQR			(0x19C)
#define ETMEXTOUTEVRn(n)	(0x1A0 + (n * 4))
#define ETMCIDCVRn(n)		(0x1B0 + (n * 4))
#define ETMCIDCMR		(0x1BC)
#define ETMIMPSPEC0		(0x1C0)
#define ETMIMPSPEC1		(0x1C4)
#define ETMIMPSPEC2		(0x1C8)
#define ETMIMPSPEC3		(0x1CC)
#define ETMIMPSPEC4		(0x1D0)
#define ETMIMPSPEC5		(0x1D4)
#define ETMIMPSPEC6		(0x1D8)
#define ETMIMPSPEC7		(0x1DC)
#define ETMSYNCFR		(0x1E0)
#define ETMIDR			(0x1E4)
#define ETMCCER			(0x1E8)
#define ETMEXTINSELR		(0x1EC)
#define ETMTESSEICR		(0x1F0)
#define ETMEIBCR		(0x1F4)
#define ETMTSEVR		(0x1F8)
#define ETMAUXCR		(0x1FC)
#define ETMTRACEIDR		(0x200)
#define ETMVMIDCVR		(0x240)
/* Management registers (0x300-0x314) */
#define ETMOSLAR		(0x300)
#define ETMOSLSR		(0x304)
#define ETMOSSRR		(0x308)
#define ETMPDCR			(0x310)
#define ETMPDSR			(0x314)

#define ETM_MAX_ADDR_CMP	(16)
#define ETM_MAX_CNTR		(4)
#define ETM_MAX_CTXID_CMP	(3)

#define ETM_MODE_EXCLUDE	BIT(0)
#define ETM_MODE_CYCACC		BIT(1)
#define ETM_MODE_STALL		BIT(2)
#define ETM_MODE_TIMESTAMP	BIT(3)
#define ETM_MODE_CTXID		BIT(4)
#define ETM_MODE_ALL		(0x1F)

#define ETM_EVENT_MASK		(0x1FFFF)
#define ETM_SYNC_MASK		(0xFFF)
#define ETM_ALL_MASK		(0xFFFFFFFF)

#define ETM_SEQ_STATE_MAX_VAL	(0x2)

#define ETM_TRACEID 2

enum etm_addr_type {
	ETM_ADDR_TYPE_NONE,
	ETM_ADDR_TYPE_SINGLE,
	ETM_ADDR_TYPE_RANGE,
	ETM_ADDR_TYPE_START,
	ETM_ADDR_TYPE_STOP,
};

static int boot_enable;
module_param_named(
	boot_enable, boot_enable, int, S_IRUGO
);

struct etm_drvdata {
	void __iomem			*base;
	struct device			*dev;
	struct coresight_device		*csdev;
	struct clk			*clk;
	spinlock_t			spinlock;
	int				cpu;
	int				port_size;
	uint8_t				arch;
	bool				use_cp14;
	bool				enable;
	bool				sticky_enable;
	bool				boot_enable;
	bool				os_unlock;
	uint8_t				nr_addr_cmp;
	uint8_t				nr_cntr;
	uint8_t				nr_ext_inp;
	uint8_t				nr_ext_out;
	uint8_t				nr_ctxid_cmp;
	uint8_t				reset;
	uint32_t			mode;
	uint32_t			ctrl;
	uint32_t			trigger_event;
	uint32_t			startstop_ctrl;
	uint32_t			enable_event;
	uint32_t			enable_ctrl1;
	uint32_t			fifofull_level;
	uint8_t				addr_idx;
	uint32_t			addr_val[ETM_MAX_ADDR_CMP];
	uint32_t			addr_acctype[ETM_MAX_ADDR_CMP];
	uint32_t			addr_type[ETM_MAX_ADDR_CMP];
	uint8_t				cntr_idx;
	uint32_t			cntr_rld_val[ETM_MAX_CNTR];
	uint32_t			cntr_event[ETM_MAX_CNTR];
	uint32_t			cntr_rld_event[ETM_MAX_CNTR];
	uint32_t			cntr_val[ETM_MAX_CNTR];
	uint32_t			seq_12_event;
	uint32_t			seq_21_event;
	uint32_t			seq_23_event;
	uint32_t			seq_31_event;
	uint32_t			seq_32_event;
	uint32_t			seq_13_event;
	uint32_t			seq_curr_state;
	uint8_t				ctxid_idx;
	uint32_t			ctxid_val[ETM_MAX_CTXID_CMP];
	uint32_t			ctxid_mask;
	uint32_t			sync_freq;
	uint32_t			timestamp_event;
};

#define ETM_NUMS    4
static struct etm_drvdata *etmdrvdata[ETM_NUMS];

/*
 * Memory mapped writes to clear os lock are not supported on some processors
 * and OS lock must be unlocked before any memory mapped access on such
 * processors, otherwise memory mapped reads/writes will be invalid.
 */
static void etm_os_unlock(void *info)
{
	struct etm_drvdata *drvdata = (struct etm_drvdata *)info;
	etm_writel(drvdata, 0x0, ETMOSLAR);
	isb();
}

static void etm_set_pwrdwn(struct etm_drvdata *drvdata)
{
	uint32_t etmcr;

	/* Ensure pending cp14 accesses complete before setting pwrdwn */
	mb();
	isb();
	etmcr = etm_readl(drvdata, ETMCR);
	etmcr |= BIT(0);
	etm_writel(drvdata, etmcr, ETMCR);
}

static void etm_clr_pwrdwn(struct etm_drvdata *drvdata)
{
	uint32_t etmcr;

	etmcr = etm_readl(drvdata, ETMCR);
	etmcr &= ~BIT(0);
	etm_writel(drvdata, etmcr, ETMCR);
	/* Ensure pwrup completes before subsequent cp14 accesses */
	mb();
	isb();
}

static void etm_set_pwrup(struct etm_drvdata *drvdata)
{
	uint32_t etmpdcr;

	etmpdcr = etm_readl_mm(drvdata, ETMPDCR);
	etmpdcr |= BIT(3);
	etm_writel_mm(drvdata, etmpdcr, ETMPDCR);
	/* Ensure pwrup completes before subsequent cp14 accesses */
	mb();
	isb();
}

static void etm_clr_pwrup(struct etm_drvdata *drvdata)
{
	uint32_t etmpdcr;

	/* Ensure pending cp14 accesses complete before clearing pwrup */
	mb();
	isb();
	etmpdcr = etm_readl_mm(drvdata, ETMPDCR);
	etmpdcr &= ~BIT(3);
	etm_writel_mm(drvdata, etmpdcr, ETMPDCR);
}

static void etm_set_prog(struct etm_drvdata *drvdata)
{
	uint32_t etmcr;
	int count;

	etmcr = etm_readl(drvdata, ETMCR);
	etmcr |= BIT(10);
	etm_writel(drvdata, etmcr, ETMCR);
	/*
	 * Recommended by spec for cp14 accesses to ensure etmcr write is
	 * complete before polling etmsr
	 */
	isb();
	for (count = TIMEOUT_US; BVAL(etm_readl(drvdata, ETMSR), 1) != 1
				&& count > 0; count--)
		udelay(1);
	WARN(count == 0, "timeout while setting prog bit, ETMSR: %#x\n",
	     etm_readl(drvdata, ETMSR));
}

static void etm_clr_prog(struct etm_drvdata *drvdata)
{
	uint32_t etmcr;
	int count;

	etmcr = etm_readl(drvdata, ETMCR);
	etmcr &= ~BIT(10);
	etm_writel(drvdata, etmcr, ETMCR);
	/*
	 * Recommended by spec for cp14 accesses to ensure etmcr write is
	 * complete before polling etmsr
	 */
	isb();
	for (count = TIMEOUT_US; BVAL(etm_readl(drvdata, ETMSR), 1) != 0
				&& count > 0; count--)
		udelay(1);
	WARN(count == 0, "timeout while clearing prog bit, ETMSR: %#x\n",
	     etm_readl(drvdata, ETMSR));
}

static void __etm_enable(void *info)
{
	uint32_t etmcr;
	struct etm_drvdata *drvdata = info;
	ETM_UNLOCK(drvdata);

    /* turn engine on */
	etm_clr_pwrdwn(drvdata);

    /* set program bit */
    etm_set_prog(drvdata);

	/* make sure all registers are accessible */
	etm_os_unlock(drvdata);

    /* set trace id */
    etm_writel(drvdata, ETM_TRACEID+drvdata->cpu, ETMTRACEIDR);

    /* monitor all */
    etm_writel(drvdata, 0x376f, ETMTEEVR);
    etm_writel(drvdata, 0x01000000, ETMTECR1);
    etm_writel(drvdata, 0x406f, ETMTRIGGER);

    /* set timestamp event */
    etm_writel(drvdata, 0x406f, ETMTSEVR);
    /* disable timestamp event */
    etmcr = etm_readl(drvdata, ETMCR);
    etmcr &= ~(BIT(28));
    etm_writel(drvdata, etmcr, ETMCR);

    /* set context id */
    etmcr = etm_readl(drvdata, ETMCR);
    etmcr |= BIT(14) | BIT(15);
    etm_writel(drvdata, etmcr, ETMCR);

    /* set sysnc requency */
    etm_writel(drvdata, 0x400, ETMSYNCFR);

    etm_clr_prog(drvdata);
	ETM_LOCK(drvdata);
	/* dev_dbg(drvdata->dev, "cpu: %d enable smp call done\n", drvdata->cpu); */
}

static int etm_enable(struct coresight_device *csdev)
{
	struct etm_drvdata *drvdata = dev_get_drvdata(csdev->dev.parent);
	int ret;

	ret = clk_prepare_enable(drvdata->clk);
	if (ret)
		goto err_clk;

	spin_lock(&drvdata->spinlock);
    __etm_enable(drvdata);
	drvdata->enable = true;
	drvdata->sticky_enable = true;

	spin_unlock(&drvdata->spinlock);

	/* dev_info(drvdata->dev, "ETM tracing enabled\n"); */
	return 0;

err_clk:
	return ret;
}

static void __etm_disable(void *info)
{
	struct etm_drvdata *drvdata = info;

	ETM_UNLOCK(drvdata);
	etm_set_prog(drvdata);

	/* Program trace enable to low by using always false event */
	etm_writel(drvdata, 0x6F | BIT(14), ETMTEEVR);

	etm_set_pwrdwn(drvdata);
	ETM_LOCK(drvdata);

	/* dev_dbg(drvdata->dev, "cpu: %d disable smp call done\n", drvdata->cpu); */
}

static void etm_disable(struct coresight_device *csdev)
{
	struct etm_drvdata *drvdata = dev_get_drvdata(csdev->dev.parent);

	/*
	 * Taking hotplug lock here protects from clocks getting disabled
	 * with tracing being left on (crash scenario) if user disable occurs
	 * after cpu online mask indicates the cpu is offline but before the
	 * DYING hotplug callback is serviced by the ETM driver.
	 */
	get_online_cpus();
	spin_lock(&drvdata->spinlock);

	/*
	 * Executing __etm_disable on the cpu whose ETM is being disabled
	 * ensures that register writes occur when cpu is powered.
	 */
	//smp_call_function_single(drvdata->cpu, __etm_disable, drvdata, 1);
    __etm_disable(drvdata);
	drvdata->enable = false;

	spin_unlock(&drvdata->spinlock);
	put_online_cpus();

	clk_disable_unprepare(drvdata->clk);

	/* dev_info(drvdata->dev, "ETM tracing disabled\n"); */
}

static const struct coresight_ops_source etm_source_ops = {
	.enable		= etm_enable,
	.disable	= etm_disable,
};

static const struct coresight_ops etm_cs_ops = {
	.source_ops	= &etm_source_ops,
};

static ssize_t debugfs_show_nr_addr_cmp(struct file *file,
					char __user *user_buf,
					size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->nr_addr_cmp;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static const struct file_operations debugfs_nr_addr_cmp_ops = {
	.open = simple_open,
	.read = debugfs_show_nr_addr_cmp,
};

static const struct coresight_ops_entry debugfs_nr_addr_cmp_entry = {
	.name = "nr_addr_cmp",
	.mode =  S_IRUGO,
	.ops = &debugfs_nr_addr_cmp_ops,
};

static ssize_t debugfs_show_nr_cntr(struct file *file,
				    char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->nr_cntr;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static const struct file_operations debugfs_nr_cntr_ops = {
	.open = simple_open,
	.read = debugfs_show_nr_cntr,
};

static const struct coresight_ops_entry debugfs_nr_cntr_entry = {
	.name = "nr_cntr",
	.mode =  S_IRUGO,
	.ops = &debugfs_nr_cntr_ops,
};

static ssize_t debugfs_show_nr_ctxid_cmp(struct file *file,
					 char __user *user_buf,
					 size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->nr_ctxid_cmp;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static const struct file_operations debugfs_nr_ctxid_cmp_ops = {
	.open = simple_open,
	.read = debugfs_show_nr_ctxid_cmp,
};

static const struct coresight_ops_entry debugfs_nr_ctxid_cmp_entry = {
	.name = "nr_ctxid_cmp",
	.mode =  S_IRUGO,
	.ops = &debugfs_nr_ctxid_cmp_ops,
};

static ssize_t debugfs_show_reset(struct file *file,
				  char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->reset;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

/* Reset to trace everything i.e. exclude nothing. */
static ssize_t debugfs_store_reset(struct file *file,
				   const char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	int i;
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	spin_lock(&drvdata->spinlock);
	if (val) {
		drvdata->mode = ETM_MODE_EXCLUDE;
		drvdata->ctrl = 0x0;
		drvdata->trigger_event = 0x406F;
		drvdata->startstop_ctrl = 0x0;
		drvdata->enable_event = 0x6F;
		drvdata->enable_ctrl1 = 0x1000000;
		drvdata->fifofull_level = 0x28;
		drvdata->addr_idx = 0x0;
		for (i = 0; i < drvdata->nr_addr_cmp; i++) {
			drvdata->addr_val[i] = 0x0;
			drvdata->addr_acctype[i] = 0x0;
			drvdata->addr_type[i] = ETM_ADDR_TYPE_NONE;
		}
		drvdata->cntr_idx = 0x0;
		for (i = 0; i < drvdata->nr_cntr; i++) {
			drvdata->cntr_rld_val[i] = 0x0;
			drvdata->cntr_event[i] = 0x406F;
			drvdata->cntr_rld_event[i] = 0x406F;
			drvdata->cntr_val[i] = 0x0;
		}
		drvdata->seq_12_event = 0x406F;
		drvdata->seq_21_event = 0x406F;
		drvdata->seq_23_event = 0x406F;
		drvdata->seq_31_event = 0x406F;
		drvdata->seq_32_event = 0x406F;
		drvdata->seq_13_event = 0x406F;
		drvdata->seq_curr_state = 0x0;
		drvdata->ctxid_idx = 0x0;
		for (i = 0; i < drvdata->nr_ctxid_cmp; i++)
			drvdata->ctxid_val[i] = 0x0;
		drvdata->ctxid_mask = 0x0;
		drvdata->sync_freq = 0x100;
		drvdata->timestamp_event = 0x406F;
	}
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_reset_ops = {
	.open = simple_open,
	.read = debugfs_show_reset,
	.write = debugfs_store_reset,
};

static const struct coresight_ops_entry debugfs_reset_entry = {
	.name = "reset",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_reset_ops,
};

static ssize_t debugfs_show_mode(struct file *file,
				 char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->mode;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_mode(struct file *file,
				  const char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	spin_lock(&drvdata->spinlock);
	drvdata->mode = val & ETM_MODE_ALL;

	if (drvdata->mode & ETM_MODE_EXCLUDE)
		drvdata->enable_ctrl1 |= BIT(24);
	else
		drvdata->enable_ctrl1 &= ~BIT(24);

	if (drvdata->mode & ETM_MODE_CYCACC)
		drvdata->ctrl |= BIT(12);
	else
		drvdata->ctrl &= ~BIT(12);

	if (drvdata->mode & ETM_MODE_STALL)
		drvdata->ctrl |= BIT(7);
	else
		drvdata->ctrl &= ~BIT(7);

	if (drvdata->mode & ETM_MODE_TIMESTAMP)
		drvdata->ctrl |= BIT(28);
	else
		drvdata->ctrl &= ~BIT(28);

	if (drvdata->mode & ETM_MODE_CTXID)
		drvdata->ctrl |= (BIT(14) | BIT(15));
	else
		drvdata->ctrl &= ~(BIT(14) | BIT(15));
	spin_unlock(&drvdata->spinlock);

	return count;
}

static const struct file_operations debugfs_mode_ops = {
	.open = simple_open,
	.read = debugfs_show_mode,
	.write = debugfs_store_mode,
};

static const struct coresight_ops_entry debugfs_mode_entry = {
	.name = "mode",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_mode_ops,
};

static ssize_t debugfs_show_trigger_event(struct file *file,
					  char __user *user_buf,
					  size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->trigger_event;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_trigger_event(struct file *file,
					   const char __user *user_buf,
					   size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	drvdata->trigger_event = val & ETM_EVENT_MASK;
	return count;
}

static const struct file_operations debugfs_trigger_event_ops = {
	.open = simple_open,
	.read = debugfs_show_trigger_event,
	.write = debugfs_store_trigger_event,
};

static const struct coresight_ops_entry debugfs_trigger_events_entry = {
	.name = "trigger_event",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_trigger_event_ops,
};

static ssize_t debugfs_show_enable_event(struct file *file,
					 char __user *user_buf,
					 size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->enable_event;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_enable_event(struct file *file,
					  const char __user *user_buf,
					  size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	drvdata->enable_event = val & ETM_EVENT_MASK;
	return count;
}

static const struct file_operations debugfs_enable_event_ops = {
	.open = simple_open,
	.read = debugfs_show_enable_event,
	.write = debugfs_store_enable_event,
};

static const struct coresight_ops_entry debugfs_enable_events_entry = {
	.name = "enable_event",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_enable_event_ops,
};

static ssize_t debugfs_show_fifofull_level(struct file *file,
					   char __user *user_buf,
					   size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->fifofull_level;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_fifofull_level(struct file *file,
					    const char __user *user_buf,
					    size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	drvdata->fifofull_level = val;
	return count;
}

static const struct file_operations debugfs_fifofull_level_ops = {
	.open = simple_open,
	.read = debugfs_show_fifofull_level,
	.write = debugfs_store_fifofull_level,
};

static const struct coresight_ops_entry debugfs_fifofull_level_entry = {
	.name = "fifofull_level",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_fifofull_level_ops,
};

static ssize_t debugfs_show_addr_idx(struct file *file,
				     char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->addr_idx;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_addr_idx(struct file *file,
				      const char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;
	if (val >= drvdata->nr_addr_cmp)
		return -EINVAL;

	/*
	 * Use spinlock to ensure index doesn't change while it gets
	 * dereferenced multiple times within a spinlock block elsewhere.
	 */
	spin_lock(&drvdata->spinlock);
	drvdata->addr_idx = val;
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_addr_idx_ops = {
	.open = simple_open,
	.read = debugfs_show_addr_idx,
	.write = debugfs_store_addr_idx,
};

static const struct coresight_ops_entry debugfs_addr_idx_entry = {
	.name = "addr_idx",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_addr_idx_ops,
};

static ssize_t debugfs_show_addr_single(struct file *file,
					char __user *user_buf,
					size_t count, loff_t *ppos)
{
	int ret;
	uint8_t idx;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	spin_lock(&drvdata->spinlock);
	idx = drvdata->addr_idx;
	if (!(drvdata->addr_type[idx] == ETM_ADDR_TYPE_NONE ||
	      drvdata->addr_type[idx] == ETM_ADDR_TYPE_SINGLE)) {
		spin_unlock(&drvdata->spinlock);
		return -EPERM;
	}

	val = drvdata->addr_val[idx];
	spin_unlock(&drvdata->spinlock);
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_addr_single(struct file *file,
					 const char __user *user_buf,
					 size_t count, loff_t *ppos)
{
	uint8_t idx;
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	spin_lock(&drvdata->spinlock);
	idx = drvdata->addr_idx;
	if (!(drvdata->addr_type[idx] == ETM_ADDR_TYPE_NONE ||
	      drvdata->addr_type[idx] == ETM_ADDR_TYPE_SINGLE)) {
		spin_unlock(&drvdata->spinlock);
		return -EPERM;
	}

	drvdata->addr_val[idx] = val;
	drvdata->addr_type[idx] = ETM_ADDR_TYPE_SINGLE;
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_addr_single_ops = {
	.open = simple_open,
	.read = debugfs_show_addr_single,
	.write = debugfs_store_addr_single,
};

static const struct coresight_ops_entry debugfs_addr_single_entry = {
	.name = "addr_single",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_addr_single_ops,
};

static ssize_t debugfs_show_addr_range(struct file *file,
				       char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	int ret;
	uint8_t idx;
	unsigned long val1, val2;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	spin_lock(&drvdata->spinlock);
	idx = drvdata->addr_idx;
	if (idx % 2 != 0) {
		spin_unlock(&drvdata->spinlock);
		return -EPERM;
	}
	if (!((drvdata->addr_type[idx] == ETM_ADDR_TYPE_NONE &&
	       drvdata->addr_type[idx + 1] == ETM_ADDR_TYPE_NONE) ||
	      (drvdata->addr_type[idx] == ETM_ADDR_TYPE_RANGE &&
	       drvdata->addr_type[idx + 1] == ETM_ADDR_TYPE_RANGE))) {
		spin_unlock(&drvdata->spinlock);
		return -EPERM;
	}

	val1 = drvdata->addr_val[idx];
	val2 = drvdata->addr_val[idx + 1];
	spin_unlock(&drvdata->spinlock);
	ret = scnprintf(buf, PAGE_SIZE, "%#lx %#lx\n", val1, val2);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_addr_range(struct file *file,
					const char __user *user_buf,
					size_t count, loff_t *ppos)
{
	uint8_t idx;
	unsigned long val1, val2;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx %lx", &val1, &val2) != 2)
		return -EINVAL;
	/* Lower address comparator cannot have a higher address value */
	if (val1 > val2)
		return -EINVAL;

	spin_lock(&drvdata->spinlock);
	idx = drvdata->addr_idx;
	if (idx % 2 != 0) {
		spin_unlock(&drvdata->spinlock);
		return -EPERM;
	}
	if (!((drvdata->addr_type[idx] == ETM_ADDR_TYPE_NONE &&
	       drvdata->addr_type[idx + 1] == ETM_ADDR_TYPE_NONE) ||
	      (drvdata->addr_type[idx] == ETM_ADDR_TYPE_RANGE &&
	       drvdata->addr_type[idx + 1] == ETM_ADDR_TYPE_RANGE))) {
		spin_unlock(&drvdata->spinlock);
		return -EPERM;
	}

	drvdata->addr_val[idx] = val1;
	drvdata->addr_type[idx] = ETM_ADDR_TYPE_RANGE;
	drvdata->addr_val[idx + 1] = val2;
	drvdata->addr_type[idx + 1] = ETM_ADDR_TYPE_RANGE;
	drvdata->enable_ctrl1 |= (1 << (idx/2));
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_addr_range_ops = {
	.open = simple_open,
	.read = debugfs_show_addr_range,
	.write = debugfs_store_addr_range,
};

static const struct coresight_ops_entry debugfs_addr_range_entry = {
	.name = "addr_range",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_addr_range_ops,
};

static ssize_t debugfs_show_addr_start(struct file *file,
				       char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	int ret;
	uint8_t idx;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	spin_lock(&drvdata->spinlock);
	idx = drvdata->addr_idx;
	if (!(drvdata->addr_type[idx] == ETM_ADDR_TYPE_NONE ||
	      drvdata->addr_type[idx] == ETM_ADDR_TYPE_START)) {
		spin_unlock(&drvdata->spinlock);
		return -EPERM;
	}

	val = drvdata->addr_val[idx];
	spin_unlock(&drvdata->spinlock);
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_addr_start(struct file *file,
					const char __user *user_buf,
					size_t count, loff_t *ppos)
{
	uint8_t idx;
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	spin_lock(&drvdata->spinlock);
	idx = drvdata->addr_idx;
	if (!(drvdata->addr_type[idx] == ETM_ADDR_TYPE_NONE ||
	      drvdata->addr_type[idx] == ETM_ADDR_TYPE_START)) {
		spin_unlock(&drvdata->spinlock);
		return -EPERM;
	}

	drvdata->addr_val[idx] = val;
	drvdata->addr_type[idx] = ETM_ADDR_TYPE_START;
	drvdata->startstop_ctrl |= (1 << idx);
	drvdata->enable_ctrl1 |= BIT(25);
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_addr_start_ops = {
	.open = simple_open,
	.read = debugfs_show_addr_start,
	.write = debugfs_store_addr_start,
};

static const struct coresight_ops_entry debugfs_addr_start_entry = {
	.name = "addr_start",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_addr_start_ops,
};

static ssize_t debugfs_show_addr_stop(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	uint8_t idx;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	spin_lock(&drvdata->spinlock);
	idx = drvdata->addr_idx;
	if (!(drvdata->addr_type[idx] == ETM_ADDR_TYPE_NONE ||
	      drvdata->addr_type[idx] == ETM_ADDR_TYPE_STOP)) {
		spin_unlock(&drvdata->spinlock);
		return -EPERM;
	}

	val = drvdata->addr_val[idx];
	spin_unlock(&drvdata->spinlock);
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_addr_stop(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	uint8_t idx;
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	spin_lock(&drvdata->spinlock);
	idx = drvdata->addr_idx;
	if (!(drvdata->addr_type[idx] == ETM_ADDR_TYPE_NONE ||
	      drvdata->addr_type[idx] == ETM_ADDR_TYPE_STOP)) {
		spin_unlock(&drvdata->spinlock);
		return -EPERM;
	}

	drvdata->addr_val[idx] = val;
	drvdata->addr_type[idx] = ETM_ADDR_TYPE_STOP;
	drvdata->startstop_ctrl |= (1 << (idx + 16));
	drvdata->enable_ctrl1 |= BIT(25);
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_addr_stop_ops = {
	.open = simple_open,
	.read = debugfs_show_addr_stop,
	.write = debugfs_store_addr_stop,
};

static const struct coresight_ops_entry debugfs_addr_stop_entry = {
	.name = "addr_stop",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_addr_stop_ops,
};

static ssize_t debugfs_show_addr_acctype(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	spin_lock(&drvdata->spinlock);
	val = drvdata->addr_acctype[drvdata->addr_idx];
	spin_unlock(&drvdata->spinlock);
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_addr_acctype(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	spin_lock(&drvdata->spinlock);
	drvdata->addr_acctype[drvdata->addr_idx] = val;
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_addr_acctype_ops = {
	.open = simple_open,
	.read = debugfs_show_addr_acctype,
	.write = debugfs_store_addr_acctype,
};

static const struct coresight_ops_entry debugfs_addr_acctype_entry = {
	.name = "addr_acctype",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_addr_acctype_ops,
};

static ssize_t debugfs_show_cntr_idx(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->cntr_idx;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_cntr_idx(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;
	if (val >= drvdata->nr_cntr)
		return -EINVAL;

	/*
	 * Use spinlock to ensure index doesn't change while it gets
	 * dereferenced multiple times within a spinlock block elsewhere.
	 */
	spin_lock(&drvdata->spinlock);
	drvdata->cntr_idx = val;
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_cntr_idx_ops = {
	.open = simple_open,
	.read = debugfs_show_cntr_idx,
	.write = debugfs_store_cntr_idx,
};

static const struct coresight_ops_entry debugfs_cntr_idx_entry = {
	.name = "cntr_idx",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_cntr_idx_ops,
};

static ssize_t debugfs_show_cntr_rld_val(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	spin_lock(&drvdata->spinlock);
	val = drvdata->cntr_rld_val[drvdata->cntr_idx];
	spin_unlock(&drvdata->spinlock);
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_cntr_rld_val(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	spin_lock(&drvdata->spinlock);
	drvdata->cntr_rld_val[drvdata->cntr_idx] = val;
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_cntr_rld_val_ops = {
	.open = simple_open,
	.read = debugfs_show_cntr_rld_val,
	.write = debugfs_store_cntr_rld_val,
};

static const struct coresight_ops_entry debugfs_cntr_rld_val_entry = {
	.name = "cntr_rld_val",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_cntr_rld_val_ops,
};

static ssize_t debugfs_show_cntr_event(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	spin_lock(&drvdata->spinlock);
	val = drvdata->cntr_event[drvdata->cntr_idx];
	spin_unlock(&drvdata->spinlock);
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_cntr_event(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	spin_lock(&drvdata->spinlock);
	drvdata->cntr_event[drvdata->cntr_idx] = val & ETM_EVENT_MASK;
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_cntr_event_ops = {
	.open = simple_open,
	.read = debugfs_show_cntr_event,
	.write = debugfs_store_cntr_event,
};

static const struct coresight_ops_entry debugfs_cntr_event_entry = {
	.name = "cntr_event",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_cntr_event_ops,
};

static ssize_t debugfs_show_cntr_rld_event(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	spin_lock(&drvdata->spinlock);
	val = drvdata->cntr_rld_event[drvdata->cntr_idx];
	spin_unlock(&drvdata->spinlock);
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_cntr_rld_event(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	spin_lock(&drvdata->spinlock);
	drvdata->cntr_rld_event[drvdata->cntr_idx] = val & ETM_EVENT_MASK;
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_cntr_rld_event_ops = {
	.open = simple_open,
	.read = debugfs_show_cntr_rld_event,
	.write = debugfs_store_cntr_rld_event,
};

static const struct coresight_ops_entry debugfs_cntr_rld_event_entry = {
	.name = "cntr_rld_event",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_cntr_rld_event_ops,
};

static ssize_t debugfs_show_cntr_val(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	spin_lock(&drvdata->spinlock);
	val = drvdata->cntr_val[drvdata->cntr_idx];
	spin_unlock(&drvdata->spinlock);
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_cntr_val(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	spin_lock(&drvdata->spinlock);
	drvdata->cntr_val[drvdata->cntr_idx] = val;
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_cntr_val_ops = {
	.open = simple_open,
	.read = debugfs_show_cntr_val,
	.write = debugfs_store_cntr_val,
};

static const struct coresight_ops_entry debugfs_cntr_val_entry = {
	.name = "cntr_val",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_cntr_val_ops,
};

static ssize_t debugfs_show_12_event(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->seq_12_event;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_12_event(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	drvdata->seq_12_event = val & ETM_EVENT_MASK;
	return count;
}

static const struct file_operations debugfs_12_event_ops = {
	.open = simple_open,
	.read = debugfs_show_12_event,
	.write = debugfs_store_12_event,
};

static const struct coresight_ops_entry debugfs_12_event_entry = {
	.name = "seq_12_event",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_12_event_ops,
};

static ssize_t debugfs_show_21_event(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->seq_21_event;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_21_event(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	drvdata->seq_21_event = val & ETM_EVENT_MASK;
	return count;
}

static const struct file_operations debugfs_21_event_ops = {
	.open = simple_open,
	.read = debugfs_show_21_event,
	.write = debugfs_store_21_event,
};

static const struct coresight_ops_entry debugfs_21_event_entry = {
	.name = "seq_21_event",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_21_event_ops,
};

static ssize_t debugfs_show_23_event(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->seq_23_event;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_23_event(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	drvdata->seq_23_event = val & ETM_EVENT_MASK;
	return count;
}

static const struct file_operations debugfs_23_event_ops = {
	.open = simple_open,
	.read = debugfs_show_23_event,
	.write = debugfs_store_23_event,
};

static const struct coresight_ops_entry debugfs_23_event_entry = {
	.name = "seq_23_event",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_23_event_ops,
};

static ssize_t debugfs_show_31_event(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->seq_31_event;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_31_event(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	drvdata->seq_31_event = val & ETM_EVENT_MASK;
	return count;
}

static const struct file_operations debugfs_31_event_ops = {
	.open = simple_open,
	.read = debugfs_show_31_event,
	.write = debugfs_store_31_event,
};

static const struct coresight_ops_entry debugfs_31_event_entry = {
	.name = "seq_31_event",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_31_event_ops,
};

static ssize_t debugfs_show_32_event(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->seq_32_event;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_32_event(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	drvdata->seq_32_event = val & ETM_EVENT_MASK;
	return count;
}
static const struct file_operations debugfs_32_event_ops = {
	.open = simple_open,
	.read = debugfs_show_32_event,
	.write = debugfs_store_32_event,
};

static const struct coresight_ops_entry debugfs_32_event_entry = {
	.name = "seq_32_event",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_32_event_ops,
};

static ssize_t debugfs_show_13_event(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->seq_13_event;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_13_event(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	drvdata->seq_13_event = val & ETM_EVENT_MASK;
	return count;
}
static const struct file_operations debugfs_13_event_ops = {
	.open = simple_open,
	.read = debugfs_show_13_event,
	.write = debugfs_store_13_event,
};

static const struct coresight_ops_entry debugfs_13_event_entry = {
	.name = "seq_13_event",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_13_event_ops,
};

static ssize_t debugfs_show_seq_curr_state(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->seq_curr_state;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_seq_curr_state(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;
	if (val > ETM_SEQ_STATE_MAX_VAL)
		return -EINVAL;

	drvdata->seq_curr_state = val;
	return count;
}

static const struct file_operations debugfs_seq_curr_state_ops = {
	.open = simple_open,
	.read = debugfs_show_seq_curr_state,
	.write = debugfs_store_seq_curr_state,
};

static const struct coresight_ops_entry debugfs_seq_curr_state_entry = {
	.name = "seq_curr_state",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_seq_curr_state_ops,
};

static ssize_t debugfs_show_ctxid_idx(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->ctxid_idx;

	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_ctxid_idx(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;
	if (val >= drvdata->nr_ctxid_cmp)
		return -EINVAL;

	/*
	 * Use spinlock to ensure index doesn't change while it gets
	 * dereferenced multiple times within a spinlock block elsewhere.
	 */
	spin_lock(&drvdata->spinlock);
	drvdata->ctxid_idx = val;
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_ctxid_idx_ops = {
	.open = simple_open,
	.read = debugfs_show_ctxid_idx,
	.write = debugfs_store_ctxid_idx,
};

static const struct coresight_ops_entry debugfs_ctxid_idx_entry = {
	.name = "ctxid_idx",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_ctxid_idx_ops,
};

static ssize_t debugfs_show_ctxid_val(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	spin_lock(&drvdata->spinlock);
	val = drvdata->ctxid_val[drvdata->ctxid_idx];
	spin_unlock(&drvdata->spinlock);
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_ctxid_val(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	spin_lock(&drvdata->spinlock);
	drvdata->ctxid_val[drvdata->ctxid_idx] = val;
	spin_unlock(&drvdata->spinlock);
	return count;
}

static const struct file_operations debugfs_ctxid_val_ops = {
	.open = simple_open,
	.read = debugfs_show_ctxid_val,
	.write = debugfs_store_ctxid_val,
};

static const struct coresight_ops_entry debugfs_ctxid_val_entry = {
	.name = "ctxid_val",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_ctxid_val_ops,
};

static ssize_t debugfs_show_ctxid_mask(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->ctxid_mask;

	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_ctxid_mask(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	drvdata->ctxid_mask = val;
	return count;
}

static const struct file_operations debugfs_ctxid_mask_ops = {
	.open = simple_open,
	.read = debugfs_show_ctxid_mask,
	.write = debugfs_store_ctxid_mask,
};

static const struct coresight_ops_entry debugfs_ctxid_mask_entry = {
	.name = "ctxid_mask",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_ctxid_mask_ops,
};

static ssize_t debugfs_show_sync_freq(struct file *file,
				      char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->sync_freq;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_sync_freq(struct file *file,
				       const char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	drvdata->sync_freq = val & ETM_SYNC_MASK;
	return count;
}

static const struct file_operations debugfs_sync_freq_ops = {
	.open = simple_open,
	.read = debugfs_show_sync_freq,
	.write = debugfs_store_sync_freq,
};

static const struct coresight_ops_entry debugfs_sync_freq_entry = {
	.name = "sync_freq",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_sync_freq_ops,
};

static ssize_t debugfs_show_timestamp_event(struct file *file,
					    char __user *user_buf,
					    size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	val = drvdata->timestamp_event;
	ret = scnprintf(buf, PAGE_SIZE, "%#lx\n", val);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static ssize_t debugfs_store_timestamp_event(struct file *file,
					    const char __user *user_buf,
					    size_t count, loff_t *ppos)
{
	unsigned long val;
	struct etm_drvdata *drvdata = file->private_data;

	if (sscanf(user_buf, "%lx", &val) != 1)
		return -EINVAL;

	drvdata->timestamp_event = val & ETM_EVENT_MASK;
	return count;
}

static const struct file_operations debugfs_timestamp_event_ops = {
	.open = simple_open,
	.read = debugfs_show_timestamp_event,
	.write = debugfs_store_timestamp_event,
};

static const struct coresight_ops_entry debugfs_timestamp_event_entry = {
	.name = "timestamp_event",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_timestamp_event_ops,
};

static ssize_t debugfs_status_read(struct file *file, char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	ssize_t ret;
	uint32_t val;
	unsigned long flags;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	struct etm_drvdata *drvdata = file->private_data;

	if (!buf)
		return -ENOMEM;

	ret = clk_prepare_enable(drvdata->clk);
	if (ret)
		goto out;

	spin_lock_irqsave(&drvdata->spinlock, flags);

	ETM_UNLOCK(drvdata);
	val = etm_readl(drvdata, ETMCCR);
	ret += sprintf(buf, "ETMCCR: 0x%08x\n", val);
	val = etm_readl(drvdata, ETMCCER);
	ret += sprintf(buf + ret, "ETMCCER: 0x%08x\n", val);
	val = etm_readl(drvdata, ETMSCR);
	ret += sprintf(buf + ret, "ETMSCR: 0x%08x\n", val);
	val = etm_readl(drvdata, ETMIDR);
	ret += sprintf(buf + ret, "ETMIDR: 0x%08x\n", val);
	val = etm_readl(drvdata, ETMCR);
	ret += sprintf(buf + ret, "ETMCR: 0x%08x\n", val);
	val = etm_readl(drvdata, ETMTEEVR);
	ret += sprintf(buf + ret, "Enable event: 0x%08x\n", val);
	val = etm_readl(drvdata, ETMTSSCR);
	ret += sprintf(buf + ret, "Enable start/stop: 0x%08x\n", val);
	ret += sprintf(buf + ret,
		       "Enable control: CR1 0x%08x CR2 0x%08x\n",
		       etm_readl(drvdata, ETMTECR1),
		       etm_readl(drvdata, ETMTECR2));

	ETM_LOCK(drvdata);

	spin_unlock_irqrestore(&drvdata->spinlock, flags);
	clk_disable_unprepare(drvdata->clk);

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);
out:
	kfree(buf);
	return ret;
}

static const struct file_operations debugfs_status_ops = {
	.open = simple_open,
	.read = debugfs_status_read,
};

static const struct coresight_ops_entry debugfs_status_entry = {
	.name = "status",
	.mode =  S_IRUGO,
	.ops = &debugfs_status_ops,
};

static const struct coresight_ops_entry *etm_attr_grps[] = {
	&debugfs_nr_addr_cmp_entry,
	&debugfs_nr_cntr_entry,
	&debugfs_nr_ctxid_cmp_entry,
	&debugfs_reset_entry,
	&debugfs_mode_entry,
	&debugfs_trigger_events_entry,
	&debugfs_enable_events_entry,
	&debugfs_fifofull_level_entry,
	&debugfs_addr_idx_entry,
	&debugfs_addr_single_entry,
	&debugfs_addr_range_entry,
	&debugfs_addr_start_entry,
	&debugfs_addr_stop_entry,
	&debugfs_addr_acctype_entry,
	&debugfs_cntr_idx_entry,
	&debugfs_cntr_rld_val_entry,
	&debugfs_cntr_event_entry,
	&debugfs_cntr_rld_event_entry,
	&debugfs_cntr_val_entry,
	&debugfs_12_event_entry,
	&debugfs_21_event_entry,
	&debugfs_23_event_entry,
	&debugfs_31_event_entry,
	&debugfs_32_event_entry,
	&debugfs_13_event_entry,
	&debugfs_seq_curr_state_entry,
	&debugfs_ctxid_idx_entry,
	&debugfs_ctxid_val_entry,
	&debugfs_ctxid_mask_entry,
	&debugfs_sync_freq_entry,
	&debugfs_timestamp_event_entry,
	&debugfs_status_entry,
	NULL,
};

int etm_cpu_enable(unsigned int cpu)
{
    if(etmdrvdata[cpu])
        return coresight_enable(etmdrvdata[cpu]->csdev);
    return -1;
}

void etm_cpu_disable(unsigned int cpu)
{
    if(etmdrvdata[cpu] == NULL)
    {
        printk("%s: etm 0x%x drv data is null\n", __FUNCTION__, cpu);
        return;
    }

    if(cpu == CP_CORE_SET && etmdrvdata[cpu]->csdev->enable == false)
    {
        coresight_set_path_to_enable(etmdrvdata[cpu]->csdev);
    }

    coresight_disable(etmdrvdata[cpu]->csdev);
}

void etm_get_status(unsigned int cpu)
{
    if(etmdrvdata[cpu])
    {
        coresight_get_path_status(etmdrvdata[cpu]->csdev);
    }
}

static int etm_cpu_callback(struct notifier_block *nfb, unsigned long action,
			    void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;

	if (!etmdrvdata[cpu])
		goto out;

	switch (action & (~CPU_TASKS_FROZEN)) {
	case CPU_STARTING:
		spin_lock(&etmdrvdata[cpu]->spinlock);
		if (!etmdrvdata[cpu]->os_unlock) {
			etm_os_unlock(etmdrvdata[cpu]);
			etmdrvdata[cpu]->os_unlock = true;
		}

		if (etmdrvdata[cpu]->enable)
			__etm_enable(etmdrvdata[cpu]);
		spin_unlock(&etmdrvdata[cpu]->spinlock);
		break;

	case CPU_ONLINE:
		if (etmdrvdata[cpu]->boot_enable &&
		    !etmdrvdata[cpu]->sticky_enable)
			coresight_enable(etmdrvdata[cpu]->csdev);
		break;

	case CPU_DYING:
		spin_lock(&etmdrvdata[cpu]->spinlock);
		if (etmdrvdata[cpu]->enable)
			__etm_disable(etmdrvdata[cpu]);
		spin_unlock(&etmdrvdata[cpu]->spinlock);
		break;
	}
out:
	return NOTIFY_OK;
}

static struct notifier_block etm_cpu_notifier = {
	.notifier_call = etm_cpu_callback,
};

static bool etm_arch_supported(uint8_t arch)
{
	return true;
}

static void etm_init_arch_data(void *info)
{
	uint32_t etmidr;
	uint32_t etmccr;
	struct etm_drvdata *drvdata = info;

	ETM_UNLOCK(drvdata);

	/* first dummy read */
	(void)etm_readl(drvdata, ETMPDSR);
	/* Provide power to ETM: ETMPDCR[3] == 1 */
	etm_set_pwrup(drvdata);
	/*
	 * Clear power down bit since when this bit is set writes to
	 * certain registers might be ignored.
	 */
	etm_clr_pwrdwn(drvdata);
	/*
	 * Set prog bit. It will be set from reset but this is included to
	 * ensure it is set
	 */
	etm_set_prog(drvdata);

	/* Find all capabilities */
	etmidr = etm_readl(drvdata, ETMIDR);
	drvdata->arch = BMVAL(etmidr, 4, 11);
	drvdata->port_size = etm_readl(drvdata, ETMCR) & PORT_SIZE_MASK;

	etmccr = etm_readl(drvdata, ETMCCR);
	drvdata->nr_addr_cmp = BMVAL(etmccr, 0, 3) * 2;
	drvdata->nr_cntr = BMVAL(etmccr, 13, 15);
	drvdata->nr_ext_inp = BMVAL(etmccr, 17, 19);
	drvdata->nr_ext_out = BMVAL(etmccr, 20, 22);
	drvdata->nr_ctxid_cmp = BMVAL(etmccr, 24, 25);

	etm_set_pwrdwn(drvdata);
	etm_clr_pwrup(drvdata);
	ETM_LOCK(drvdata);
}

static void etm_init_default_data(struct etm_drvdata *drvdata)
{
	int i;

	uint32_t flags = (1 << 0 | /* instruction execute*/
			  3 << 3 | /* ARM instruction */
			  0 << 5 | /* No data value comparison */
			  0 << 7 | /* No exact mach */
			  0 << 8 | /* Ignore context ID */
			  0 << 10); /* Security ignored */

	drvdata->ctrl = (BIT(12) | /* cycle accurate */
			 BIT(28)); /* timestamp */
	drvdata->trigger_event = 0x406F;
	drvdata->enable_event = 0x6F;
	drvdata->enable_ctrl1 = 0x1;
	drvdata->fifofull_level	= 0x28;
	if (drvdata->nr_addr_cmp >= 2) {
		drvdata->addr_val[0] = (uint32_t) _stext;
		drvdata->addr_val[1] = (uint32_t) _etext;
		drvdata->addr_acctype[0] = flags;
		drvdata->addr_acctype[1] = flags;
		drvdata->addr_type[0] = ETM_ADDR_TYPE_RANGE;
		drvdata->addr_type[1] = ETM_ADDR_TYPE_RANGE;
	}
	for (i = 0; i < drvdata->nr_cntr; i++) {
		drvdata->cntr_event[i] = 0x406F;
		drvdata->cntr_rld_event[i] = 0x406F;
	}
	drvdata->seq_12_event = 0x406F;
	drvdata->seq_21_event = 0x406F;
	drvdata->seq_23_event = 0x406F;
	drvdata->seq_31_event = 0x406F;
	drvdata->seq_32_event = 0x406F;
	drvdata->seq_13_event = 0x406F;
	drvdata->sync_freq = 0x100;
	drvdata->timestamp_event = 0x406F;
}

static int etm_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct coresight_platform_data *pdata = NULL;
	struct etm_drvdata *drvdata;
	struct resource *res;
	static int count;
	struct coresight_desc *desc;

	drvdata = devm_kzalloc(dev, sizeof(*drvdata), GFP_KERNEL);
	if (!drvdata)
		return -ENOMEM;

	if (pdev->dev.of_node) {
		pdata = of_get_coresight_platform_data(dev, pdev->dev.of_node);
		if (IS_ERR(pdata))
			return PTR_ERR(pdata);
		pdev->dev.platform_data = pdata;
		drvdata->use_cp14 = of_property_read_bool(pdev->dev.of_node,
							  "arm,cp14");
	}

	drvdata->dev = &pdev->dev;
	platform_set_drvdata(pdev, drvdata);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	drvdata->base = devm_ioremap(dev, res->start, resource_size(res));
	if (!drvdata->base)
		return -ENOMEM;

	spin_lock_init(&drvdata->spinlock);

	if (pdata && pdata->clk) {
		drvdata->clk = pdata->clk;
		ret = clk_prepare_enable(drvdata->clk);
		if (ret)
			return ret;
	}

	if(count > ETM_NUMS)
	{
        ret = -EINVAL;
        printk("%s: invalid etm count %d\n", __FUNCTION__, count);
        goto err;
    }

	get_online_cpus();
    if(0 == strncmp("coresight-ptm-ap", pdata->name, sizeof("coresight-ptm-ap")))
    {
        etmdrvdata[AP_CORE_SET] = drvdata;
        drvdata->cpu = AP_CORE_SET;
    }
    else if(0 == strncmp("coresight-ptm-cp", pdata->name, sizeof("coresight-ptm-cp")))
    {
        etmdrvdata[CP_CORE_SET] = drvdata;
        drvdata->cpu = CP_CORE_SET;
    }
    else
    {
        drvdata->cpu = pdata ? pdata->cpu : 0;
    }

    if(0 == drvdata->cpu)
    {
        if (!smp_call_function_single(drvdata->cpu, etm_os_unlock, drvdata, 1))
            drvdata->os_unlock = true;

        if (smp_call_function_single(drvdata->cpu,
                         etm_init_arch_data,  drvdata, 1))
            dev_err(dev, "ETM arch init failed\n");
    }

	if (!count++)
		register_hotcpu_notifier(&etm_cpu_notifier);

	put_online_cpus();

	if (etm_arch_supported(drvdata->arch) == false) {
		clk_disable_unprepare(drvdata->clk);
		return -EINVAL;
	}
	etm_init_default_data(drvdata);

	clk_disable_unprepare(drvdata->clk);

	desc = devm_kzalloc(dev, sizeof(*desc), GFP_KERNEL);
	if (!desc) {
		ret = -ENOMEM;
		goto err;
	}
	desc->type = CORESIGHT_DEV_TYPE_SOURCE;
	desc->subtype.source_subtype = CORESIGHT_DEV_SUBTYPE_SOURCE_PROC;
	desc->ops = &etm_cs_ops;
	desc->pdata = pdev->dev.platform_data;
	desc->dev = &pdev->dev;
	desc->debugfs_ops = etm_attr_grps;
	desc->owner = THIS_MODULE;
	drvdata->csdev = coresight_register(desc);
	if (IS_ERR(drvdata->csdev)) {
		ret = PTR_ERR(drvdata->csdev);
		goto err;
	}

	dev_info(dev, "ETM initialized\n");

	if (boot_enable) {
		coresight_enable(drvdata->csdev);
		drvdata->boot_enable = true;
	}

	return 0;
err:
	if (drvdata->cpu == 0)
		unregister_hotcpu_notifier(&etm_cpu_notifier);
	return ret;
}

static int etm_remove(struct platform_device *pdev)
{
	struct etm_drvdata *drvdata = platform_get_drvdata(pdev);

	coresight_unregister(drvdata->csdev);
	if (drvdata->cpu == 0)
		unregister_hotcpu_notifier(&etm_cpu_notifier);
	return 0;
}

static struct of_device_id etm_match[] = {
	{.compatible = "arm,coresight-etm"},
	{}
};

static struct platform_driver etm_driver = {
	.probe          = etm_probe,
	.remove         = etm_remove,
	.driver         = {
		.name   = "coresight-etm",
		.owner	= THIS_MODULE,
		.of_match_table = etm_match,
	},
};

int __init etm_init(void)
{
	return platform_driver_register(&etm_driver);
}
module_init(etm_init);

void __exit etm_exit(void)
{
	platform_driver_unregister(&etm_driver);
}
module_exit(etm_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("CoreSight Program Flow Trace driver");
