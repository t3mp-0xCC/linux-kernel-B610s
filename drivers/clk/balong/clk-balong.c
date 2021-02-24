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
#include <linux/kernel.h>
#include <linux/clk-provider.h>
#include <linux/clk-private.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/string.h>
#include <linux/semaphore.h>
#include <osl_thread.h>
#include <bsp_icc.h>
#include <bsp_pm_om.h>
#include <mdrv_pm_common.h>

#define CLK_GATE_DISABLE_OFFSET		0x4


/* status reg offset to enable reg, V7R5:0xC,others:0x8*/
static unsigned int g_status_reg_offset_to_enable = 0; 



/* reset register offset */
#define RST_DISABLE_REG_OFFSET		0x4

#define WIDTH_TO_MASK(width)	((1 << (width)) - 1)

#define MAX_READ_DDRFREQ_TRY                   3
#define LPM3_CMD_LEN   0  /* 此值非0时注意放开cmd的赋值 */

/*
 * The reverse of DIV_ROUND_UP: The maximum number which
 * divided by m is r
 */
#define MULT_ROUND_UP(r, m) ((r) * (m) + (m) - 1)

enum {
	HS_AO_CRG,
	HS_PD_CRG,
	HS_MODEM_CRG,
};

struct hisi_periclk {
	struct clk_hw	hw;
	void __iomem	*enable;	/* enable register */
	void __iomem	*reset;		/* reset register */
	u32		ebits;		/* bits in enable/disable register */
	u32		rbits;		/* bits in reset/unreset register */
	const char 	*friend;
	u32     no_pm_log_flag;
    u32     reg_is_rw;     /* the en-dis reg is rw */
	spinlock_t	*lock;
};

struct hisi_muxclk {
	struct clk_hw	hw;
	void __iomem	*reg;		/* mux register */
	u8		shift;
	u8		width;
	u32		mbits;		/* mask bits in mux register */
	spinlock_t	*lock;
};

struct hisi_divclk {
	struct clk_hw	hw;
	void __iomem	*reg;		/* divider register */
	u8		shift;
	u8		width;
	u32		mbits;		/* mask bits in divider register */
	const struct clk_div_table	*table;
	u32     en_ref_before_set_div;  /* should enable refclk before set div */
	spinlock_t	*lock;
};

/* ppll0 */
struct hisi_ppll0_clk {
	struct clk_hw	hw;
	u32				ref_cnt;	/* reference count */
	u32				cmd[LPM3_CMD_LEN];
	spinlock_t		*lock;
};

struct hisi_dfs_ctrl_clk {
	struct clk_hw	hw;
	void __iomem	*dfs_ctrl_reg;
	u32		fout_mask;
	u32		pll_open_mask;
	u8		pd_bit;
	u8		postfout_clk_en_bit;
	u8		lock_bit;
	spinlock_t	*lock;
};

enum ENUM_PLL_CTRL_BY_CCORE{
	CTRL_BY_CCORE_WPLL    = 0,
	CTRL_BY_CCORE_GPLL    = 1,
	PLL_CTRL_BY_CCORE_BUTT
};

/* remote pll，A核发往C核的消息与C核回复给A核的消息使用同一个结构体 */
struct  remote_pll_ctrl_msg{
	unsigned int  pll_id;     /* CTRL_BY_CCORE_WPLL, CTRL_BY_CCORE_GPLL */
	unsigned int  op_type;    /* en:1,dis:0 */
	int           op_result;  /* C核操作结果，0，操作成功；-1，操作失败 */
};

/* pll controlled by ccore */
struct pll_ctrl_by_ccore {
	struct  clk_hw	             hw;
	unsigned long                rate;
	struct	remote_pll_ctrl_msg  stru_pll_ctrl_msg;
	spinlock_t	                 *lock;
};

/* PLL在C核，A核通过系统控制器中的硬投票打开PLL */
struct pll_hard_vote_to_cp {
	struct  clk_hw	             hw;
	unsigned long                rate;
    void __iomem	             *en_dis_reg_base;
    u32                          en_dis_offset;
    u32                          en_dis_bit;
    u32                          lock_status_offset;
    u32                          lock_status_bit;
	spinlock_t	                 *lock;
};


struct hisi_xfreq_clk {
	struct clk_hw	hw;
	void __iomem	*reg;	/* ctrl register */

	/* 0: A7; 1: A15;  2: gpu; 3:ddr */
	u32		id;
	u32		set_rate_cmd[LPM3_CMD_LEN];
	u32		get_rate_cmd[LPM3_CMD_LEN];

	void __iomem	*pll_sel_reg;
	u32		pll_sel_mask;
	void __iomem	*pll_div_reg;
	u32		pll_div_mask;
	u32		pll_div_aclk_mask;
	u32 	pll_div_busclk_mask;

	u32		rate;
};

struct hisi_xfreq_pll {
	struct clk_hw	hw;
	void __iomem	*reg;	/* pll ctrl0 register */
};
#define PLL_CTRL1_OFFSET	0x04

struct hisi_mclk {
	struct clk_hw	hw;
	u32				ref_cnt;	/* reference count */
	u32				en_cmd[LPM3_CMD_LEN];
	u32				dis_cmd[LPM3_CMD_LEN];
	spinlock_t		*lock;
};

struct hs_clk {
	void __iomem	*ao_crg;
	void __iomem	*pd_crg;
	void __iomem	*modem_crg;
	spinlock_t	lock;
};

enum clk_op_type{
    CLK_OP_TYPE_ENAB = 0x454E4142,  /* clk_enable              */
    CLK_OP_TYPE_DISA = 0x44495341,  /* clk_disable             */
    CLK_OP_TYPE_SETR = 0x53455452,  /* clk_set_rate            */
    CLK_OP_TYPE_SETP = 0x53455450,  /* clk_set_parent          */
    CLK_OP_TYPE_ENAU = 0x454E4155,  /* clk_enable_auto_gating  */
    CLK_OP_TYPE_DIAU = 0x44494155   /* clk_disable_auto_gating */
};

#define LOG_CLK_NAME_LEN   8  /* 记录时钟名字的前8个字符 */

struct clk_op_entry{
    pid_t             task_id;
	char              clk_name[LOG_CLK_NAME_LEN];      /* 被操作的时钟名字 */
    unsigned int      op_type;                         /* 操作类型 */
	unsigned int      enable_count_before_op;          /* 操作前的应用计数 */
};

static inline void __clk_pm_log(struct clk_hw *hw, enum clk_op_type op_type)
{
    struct clk_op_entry stru_clk_op_entry;

	stru_clk_op_entry.task_id = current->pid;
    memcpy((void *)stru_clk_op_entry.clk_name, hw->clk->name, LOG_CLK_NAME_LEN);
	stru_clk_op_entry.enable_count_before_op = hw->clk->enable_count;
	stru_clk_op_entry.op_type = op_type;
	bsp_pm_log(PM_OM_ACLK, sizeof(struct clk_op_entry), (void *)&stru_clk_op_entry);
}


static void __iomem __init *hs_clk_get_base(struct device_node *np);

static struct hs_clk hs_clk = {
	.lock = __SPIN_LOCK_UNLOCKED(hs_clk.lock),
};

extern int __clk_prepare(struct clk *clk);
extern int __clk_enable(struct clk *clk);
static int hisi_clkgate_enable(struct clk_hw *hw)
{
	struct hisi_periclk *pclk;
	struct clk *friend_clk;
	int ret = 0;
    u32 reg_val = 0;
	
	
	pclk = container_of(hw, struct hisi_periclk, hw);
    
	/***** pm log *****/
	if (1 == pclk->no_pm_log_flag)
	{
	}
	else
	{
	    __clk_pm_log(hw, CLK_OP_TYPE_ENAB);
	}
	
	if (pclk->enable)
    {   
        if (1 == pclk->reg_is_rw)
        {
            reg_val = readl(pclk->enable);
            reg_val |= pclk->ebits;
            writel(reg_val, pclk->enable); 
            (void)readl(pclk->enable);
            
        }
        else
        {
    		writel(pclk->ebits, pclk->enable); 
            (void)readl(pclk->enable + g_status_reg_offset_to_enable);
        }
    }

	/* disable reset register */
	if (pclk->reset)
		writel(pclk->rbits, pclk->reset + RST_DISABLE_REG_OFFSET);


	/*if friend clk exist,enable it*/
	if (pclk->friend) {
		friend_clk = __clk_lookup(pclk->friend);
		if (IS_ERR_OR_NULL(friend_clk)) {
			pr_err("%s get failed!\n", pclk->friend);
			return -1;
		}
		ret = __clk_prepare(friend_clk);
		if (ret) {
			pr_err("[%s], friend clock prepare faild!", __func__);
			return ret;
		}
		ret = __clk_enable(friend_clk);
		if (ret) {
			__clk_unprepare(friend_clk);
			pr_err("[%s], friend clock enable faild!", __func__);
			return ret;
		}
	}

	return 0;
}

extern int __clk_disable(struct clk *clk);
extern void __clk_unprepare(struct clk *clk);
static void hisi_clkgate_disable(struct clk_hw *hw)
{
	struct hisi_periclk *pclk;
	struct clk *friend_clk;
    u32    reg_val = 0;
    
	pclk = container_of(hw, struct hisi_periclk, hw);

	/***** pm log *****/
	if (1 == pclk->no_pm_log_flag)
	{
	}
	else
	{
	    __clk_pm_log(hw, CLK_OP_TYPE_DISA);
	}


	/* reset the ip, then disalbe clk */
	if (pclk->reset)
		writel(pclk->rbits, pclk->reset);

#ifndef CONFIG_BALONG_CLK_ALWAYS_ON
	if (pclk->enable)
    {  
        if (1 == pclk->reg_is_rw)
        {
            reg_val = readl(pclk->enable);
            reg_val &= (~pclk->ebits);
            writel(reg_val, pclk->enable); 
            (void)readl(pclk->enable);
            
        }
        else
        {
            writel(pclk->ebits, pclk->enable + CLK_GATE_DISABLE_OFFSET);
            (void)readl(pclk->enable + g_status_reg_offset_to_enable);
        }
		
    }
#endif

#ifndef CONFIG_BALONG_CLK_ALWAYS_ON
	/*if friend clk exist, disable it .*/
	if (pclk->friend) {
		friend_clk = __clk_lookup(pclk->friend);
		if (IS_ERR_OR_NULL(friend_clk)) {
			pr_err("%s get failed!\n", pclk->friend);
		}
		__clk_disable(friend_clk);
		__clk_unprepare(friend_clk);
	}
#endif
}

#ifdef CONFIG_CLK_DEBUG

static int hisi_clkgate_is_enabled(struct clk_hw *hw)
{
	struct hisi_periclk *pclk;
	u32 reg = 0;

	pclk = container_of(hw, struct hisi_periclk, hw);

	if (pclk->enable)
		reg = readl(pclk->enable + g_status_reg_offset_to_enable);
	else
		return 2;

	reg &= pclk->ebits;

	return reg ? 1 : 0;
}

static void __iomem *hisi_clkgate_get_reg(struct clk_hw *hw)
{
	struct hisi_periclk *pclk;
	void __iomem	*ret = NULL;
	u32 val = 0;

	pclk = container_of(hw, struct hisi_periclk, hw);

	if (pclk->enable) {
		ret = pclk->enable + g_status_reg_offset_to_enable;
		val = readl(ret);
		val &= pclk->ebits;
		pr_info("\n[%s]: reg = 0x%p, bits = 0x%x, regval = 0x%x\n",
			hw->clk->name, ret, pclk->ebits, val);
	}

	return ret;
}
#endif



static struct clk_ops hisi_clkgate_ops = {
	.enable		= hisi_clkgate_enable,
	.disable	= hisi_clkgate_disable,
#ifdef CONFIG_CLK_DEBUG
	.is_enabled = hisi_clkgate_is_enabled,
	.get_reg  = hisi_clkgate_get_reg,
#endif
};

static void __init hisi_clkgate_setup(struct device_node *np)
{
	struct hisi_periclk *pclk;
	struct clk_init_data *init;
	struct clk *clk;
	const char *clk_name, *name, *clk_friend, *parent_names;
	void __iomem *reg_base;
	u32 rdata[2] = {0};
	u32 gdata[2] = {0};

	reg_base = hs_clk_get_base(np);
	if (!reg_base) {
		pr_err("[%s] fail to get reg_base!\n", __func__);
		return;
	}

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] %s node doesn't have clock-output-name property!\n",
			 __func__, np->name);
		return;
	}
	if (of_property_read_u32_array(np, "hisilicon,hisi-clkgate",
				       &gdata[0], 2)) {
		pr_err("[%s] %s node doesn't have hisi-clkgate property!\n",
			 __func__, np->name);
		return;
	}

	if (of_property_read_string(np, "clock-friend-names", &clk_friend))
		clk_friend = NULL;

	/* gate only has the fixed parent */

	parent_names = of_clk_get_parent_name(np, 0);

	pclk = kzalloc(sizeof(struct hisi_periclk), GFP_KERNEL);
	if (!pclk) {
		pr_err("[%s] fail to alloc pclk!\n", __func__);
		goto err_pclk;
	}

	init = kzalloc(sizeof(struct clk_init_data), GFP_KERNEL);
	if (!init) {
		pr_err("[%s] fail to alloc init!\n", __func__);
		goto err_init;
	}
	init->name = kstrdup(clk_name, GFP_KERNEL);
	init->ops = &hisi_clkgate_ops;
	init->flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED;
	init->parent_names = &parent_names;
	init->num_parents = 1;

	if (of_property_read_u32_array(np, "hisilicon,hisi-clkreset",
				       &rdata[0], 2)) {
		pclk->reset = NULL;
		pclk->rbits = 0;
	} else {
		pclk->reset = reg_base + rdata[0];
		pclk->rbits = rdata[1];
	}

	/* if gdata[0] is 0x1, represents the enable reg is fake */
	if (gdata[0] == 0x1)
			pclk->enable = NULL;
	else
			pclk->enable = reg_base + gdata[0];

	/* no_pm_log or not */
	if (of_property_read_bool(np, "no_pm_log"))
	{
		pclk->no_pm_log_flag = 1;
	}

    /* the en-dis reg is rw or wo */
	if (of_property_read_bool(np, "reg_is_rw"))
	{
		pclk->reg_is_rw = 1;
	}
	

	pclk->ebits = gdata[1];
	pclk->lock = &hs_clk.lock;
	pclk->hw.init = init;
	pclk->friend = clk_friend;

	clk = clk_register(NULL, &pclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n",
			__func__, clk_name);
		goto err_clk;
	}
	if (!of_property_read_string(np, "clock-output-names", &name))
		clk_register_clkdev(clk, name, NULL);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	return;
err_clk:
	kfree(init);
err_init:
	kfree(pclk);
err_pclk:
	return;
}

static int hisi_ap_dfs_ctrl_prepare(struct clk_hw *hw)
{
	struct hisi_dfs_ctrl_clk *pll_clk;
	unsigned long flags = 0;
	unsigned long val, pd_val;
	pll_clk = container_of(hw, struct hisi_dfs_ctrl_clk, hw);

	if (pll_clk->lock)
	{
		spin_lock_irqsave(pll_clk->lock, flags);
	}

	/* pll is locked and working, nothing to do */
	val    = readl(pll_clk->dfs_ctrl_reg);
	pd_val = (val & (1 << pll_clk->pd_bit)); /* working status */
	val   &= (1 << pll_clk->lock_bit);       /* lock status */
	if (0 == pd_val && val != 0)
	{
		goto unlock_out;
	}

	/* disable POSTFOUT */
	val = readl(pll_clk->dfs_ctrl_reg);
	val &= ~(pll_clk->fout_mask);
	writel(val, pll_clk->dfs_ctrl_reg);

	/* open pll(vcopd/postdivpd/phase4_pd/dsmpd/reserved/pd) */
	writel(pll_clk->pll_open_mask, pll_clk->dfs_ctrl_reg);

	/* wait pll lock */
	do
	{
		val = readl(pll_clk->dfs_ctrl_reg);
		val &= (1 << pll_clk->lock_bit);
	} while (!val);

	/* enable postfout */
	val = readl(pll_clk->dfs_ctrl_reg);
	val |= (1 << pll_clk->postfout_clk_en_bit);
	writel(val, pll_clk->dfs_ctrl_reg);

unlock_out:
	if (pll_clk->lock)
	{
		spin_unlock_irqrestore(pll_clk->lock, flags);
	}

	return 0;
}


static void hisi_ap_dfs_ctrl_unprepare(struct clk_hw *hw)
{
	struct hisi_dfs_ctrl_clk *pll_clk;
	unsigned long flags = 0;
	unsigned long val;
	pll_clk = container_of(hw, struct hisi_dfs_ctrl_clk, hw);

	if (pll_clk->lock)
	{
		spin_lock_irqsave(pll_clk->lock, flags);
	}

	/* close pll */
	val = readl(pll_clk->dfs_ctrl_reg);
	val |= (1 << pll_clk->pd_bit);
	writel(val, pll_clk->dfs_ctrl_reg);

	if (pll_clk->lock)
	{
		spin_unlock_irqrestore(pll_clk->lock, flags);
	}
}


static struct clk_ops hisi_ap_dfs_ctrl_ops = {
	.prepare   = hisi_ap_dfs_ctrl_prepare,
	.unprepare = hisi_ap_dfs_ctrl_unprepare,
};

static void __init hisi_pll_ctrl_by_ap_setup(struct device_node *np)
{
	struct hisi_dfs_ctrl_clk *pll_clk;
	struct clk_init_data *init;
	struct clk *clk;
	const char *clk_name, *parent_names;
	u32 data[4] = {0};
	void __iomem *reg_base;

	reg_base = hs_clk_get_base(np);
	if (!reg_base) {
		pr_err("[%s]fail to get reg_base!\n", __func__);
		return;
	}

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s]%s no clock-output-name property!\n", __func__, np->name);
		return;
	}

	parent_names = of_clk_get_parent_name(np, 0);

	pll_clk = kzalloc(sizeof(struct hisi_dfs_ctrl_clk), GFP_KERNEL);
	if (!pll_clk) {
		pr_err("[%s]fail to alloc pclk!\n", __func__);
		goto err_ap_pll_clk;
	}

	if (of_property_read_u32_array(np, "usb_pll_mask", &data[0], 2)){
		pr_err("[%s]%s no usb_pll_mask property!\n", __func__, np->name);
		goto err_init;
	}
	pll_clk->fout_mask     = data[0];
	pll_clk->pll_open_mask = data[1];

	if (of_property_read_u32_array(np, "usb_pll_ctrl", &data[0], 4)){
		pr_err("[%s]%s no usb_pll_ctrl property!\n", __func__, np->name);
		goto err_init;
	}
	pll_clk->dfs_ctrl_reg        = reg_base + data[0];
	pll_clk->pd_bit              = data[1];
	pll_clk->postfout_clk_en_bit = data[2];
	pll_clk->lock_bit            = data[3];

	init = kzalloc(sizeof(struct clk_init_data), GFP_KERNEL);
	if (!init) {
		pr_err("[%s] fail to alloc init_data!\n", __func__);
		goto err_init;
	}
	init->name = kstrdup(clk_name, GFP_KERNEL);
	init->ops = &hisi_ap_dfs_ctrl_ops;
	init->flags = CLK_SET_RATE_PARENT;
	init->parent_names = &parent_names;
	init->num_parents = 1;

	pll_clk->lock = &hs_clk.lock;
	pll_clk->hw.init = init;

	clk = clk_register(NULL, &pll_clk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n", __func__, clk_name);
		goto err_clk;
	}

	clk_register_clkdev(clk, clk_name, NULL);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	/* coverity[leaked_storage] */
	return;

err_clk:
	kfree(init);
err_init:
	kfree(pll_clk);
err_ap_pll_clk:
	return;
}

/* 被C核开关的PLL，A核的IP要使用此PLL，A核往C核投票以开关此PLL */
#define ENABLE_REMOTE_CLK    1
#define DISABLE_REMOTE_CLK   0
static struct semaphore g_remote_clk_osl_sem_id;

static struct remote_pll_ctrl_msg g_remote_pll_ctrl_msg;
static unsigned int g_remote_clk_chan_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_REMOTE_CLK;
#ifdef REMOTE_PLL_CTRL
#define to_pll_ctrl_by_ccore(_hw) container_of(_hw, struct pll_ctrl_by_ccore, hw)
#define REMOTE_CLK_WAIT_TIME    50
static int pll_ctrl_by_ccore_prepare(struct clk_hw *hw)
{
	struct pll_ctrl_by_ccore *pstru_clk = to_pll_ctrl_by_ccore(hw);
	int ret = 0;
    int len = 0;

	pstru_clk->stru_pll_ctrl_msg.op_type = ENABLE_REMOTE_CLK;  
	
	/* 往C核投票表示需要打开PLL */
    len = bsp_icc_send(ICC_CPU_MODEM, g_remote_clk_chan_id, (unsigned char*)&(pstru_clk->stru_pll_ctrl_msg), (u32)sizeof(struct remote_pll_ctrl_msg));
    if(len != sizeof(struct remote_pll_ctrl_msg))
    {
        pr_err("send len(0x%x) != expected len(0x%x).\n", len, sizeof(struct remote_pll_ctrl_msg));
        return -1;
    }

	/* 等待回复 */
	ret = down_timeout(&g_remote_clk_osl_sem_id, REMOTE_CLK_WAIT_TIME); /* 等待回复，返回PLL的操作结果 */
	if (ret)
	{
        pr_err("prepare wait for ccore reply timeout ret[%d]\n", ret);
	    return ret;
	}
	
	return g_remote_pll_ctrl_msg.op_result;  /* 0,操作成功；其他值，操作失败 */
}

static void pll_ctrl_by_ccore_unprepare(struct clk_hw *hw)
{
	struct pll_ctrl_by_ccore *pstru_clk = to_pll_ctrl_by_ccore(hw);
	int ret = 0;
	int len = 0;

	pstru_clk->stru_pll_ctrl_msg.op_type = DISABLE_REMOTE_CLK;  
	
	/* 调用ICC往C核投票表示需要打开PLL */
	len = bsp_icc_send(ICC_CPU_MODEM, g_remote_clk_chan_id, (unsigned char*)&(pstru_clk->stru_pll_ctrl_msg), (u32)sizeof(struct remote_pll_ctrl_msg));
	if(len != sizeof(struct remote_pll_ctrl_msg))
	{
		pr_err("send len(0x%x) != expected len(0x%x).\n", len, sizeof(struct remote_pll_ctrl_msg));
		return ;
	}

    /* 等待C核回复 */
	ret = down_timeout(&g_remote_clk_osl_sem_id, REMOTE_CLK_WAIT_TIME);  /* 等到回复后就返回 */
	if (ret)
	{
        pr_err("unprepare wait for ccore reply timeout ret[%d]\n", ret);
	}

	return ;
}


static unsigned long	pll_ctrl_by_ccore_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	UNUSED(parent_rate);
	return to_pll_ctrl_by_ccore(hw)->rate;
}


static struct clk_ops pll_ctrl_by_cp_ops = {
	.prepare		= pll_ctrl_by_ccore_prepare,      /* spin_lock_irqsave后不能等待信号量，所以在prepare里做 */
	.unprepare	    = pll_ctrl_by_ccore_unprepare,
	.recalc_rate    = pll_ctrl_by_ccore_recalc_rate,  /* 初始化频率时使用 */
};

#define WPLL_NAME  "WPLL"
#define WPLL_NAME_LEN  4
#define GPLL_NAME  "GPLL"
#define GPLL_NAME_LEN  4

static void __init hisi_pll_ctrl_by_cp_setup(struct device_node *np)
{
	struct pll_ctrl_by_ccore *pll_en_dis_by_ccore;
	struct clk_init_data *init;
	struct clk *clk;
	const char *clk_name;
	const char *pll_name;
	unsigned int rate = 0;

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] %s node doesn't have clock-output-name property!\n",
			 __func__, np->name);
		return;
	}

	/* 获取频率 */
	if (of_property_read_u32(np, "clock-frequency", &rate)){
		pr_err("[%s] %s node doesn't have clock-frequency property!\n",
			 __func__, np->name);
		return;
	}

    /* get the pll name */
	if (of_property_read_string(np, "pll_name", &pll_name)) {
		pr_err("[%s] %s node doesn't have pll_name property!\n",
			 __func__, np->name);
		return;
	}

	pll_en_dis_by_ccore = kzalloc(sizeof(struct pll_ctrl_by_ccore), GFP_KERNEL);
	if (!pll_en_dis_by_ccore) {
		pr_err("[%s] fail to alloc pll_ctrl_by_cp!\n", __func__);
		goto err_pll_ctrl_by_ccore;
	}

    if (0 == memcmp((const void*)pll_name, (const void*)WPLL_NAME, (unsigned int)WPLL_NAME_LEN)) /* WPLL */
	{
	    pll_en_dis_by_ccore->stru_pll_ctrl_msg.pll_id = (unsigned int)CTRL_BY_CCORE_WPLL;
	}
	else if (0 == memcmp((const void*)pll_name, (const void*)GPLL_NAME, (unsigned int)GPLL_NAME_LEN)) /* GPLL */
	{
		pll_en_dis_by_ccore->stru_pll_ctrl_msg.pll_id = (unsigned int)CTRL_BY_CCORE_GPLL;
	}
	else  /* invalid */
	{
		pr_err("[%s] invalid pll name\n", __func__);
		goto err_init;
	}
	
	init = kzalloc(sizeof(struct clk_init_data), GFP_KERNEL);
	if (!init) {
		pr_err("[%s] fail to alloc init!\n", __func__);
		goto err_init;
	}
	init->name = kstrdup(clk_name, GFP_KERNEL);
	init->ops = &pll_ctrl_by_cp_ops;
	init->flags = CLK_IS_ROOT;

    pll_en_dis_by_ccore->rate = (unsigned long)rate;
	pll_en_dis_by_ccore->lock = &hs_clk.lock;
	pll_en_dis_by_ccore->hw.init = init;

	clk = clk_register(NULL, &pll_en_dis_by_ccore->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n",
			__func__, clk_name);
		goto err_clk;
	}

	clk_register_clkdev(clk, clk_name, NULL);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	/* coverity[leaked_storage] */
	return;

err_clk:
	kfree(init);
err_init:
	kfree(pll_en_dis_by_ccore);
err_pll_ctrl_by_ccore:
	return;
}




#ifndef SEM_FULL
#define SEM_FULL            (1)
#endif
#ifndef SEM_EMPTY
#define SEM_EMPTY           (0)
#endif

#endif
/* 被C核控制的PLL，C核操作完PLL后给A核发来ICC回复 */
static int remote_clk_deal_reply(unsigned int channel_id , unsigned int len, void* context)
{
	int rx_len = 0;

	UNUSED(channel_id);
	UNUSED(len);
	UNUSED(context);

	rx_len= bsp_icc_read(g_remote_clk_chan_id, (u8*)&g_remote_pll_ctrl_msg, sizeof(struct remote_pll_ctrl_msg));
	if(rx_len != (int)sizeof(struct remote_pll_ctrl_msg))
	{
		pr_err("read len(0x%x) != expected len(0x%x).\n", rx_len, sizeof(struct remote_pll_ctrl_msg));
		return -1;
	}
	
	up(&g_remote_clk_osl_sem_id);

    return 0;
}

static int clk_remote_init(void)
{
	int ret = 0;
	
    sema_init(&g_remote_clk_osl_sem_id, SEM_EMPTY);

	/* 注册ICC回调 */
	ret = bsp_icc_event_register(g_remote_clk_chan_id, (read_cb_func)remote_clk_deal_reply, NULL, NULL, NULL);
	if(ret)
	{
		pr_err("bsp_icc_event_register err ret[%d]\n", ret);
		return -1;
	}
	
	pr_err("clk_remote_init using icc ok\n");

    return 0;
}

module_init(clk_remote_init);

#define to_pll_hard_vote_to_cp(_hw) container_of(_hw, struct pll_hard_vote_to_cp, hw)

static int pll_hard_vote_to_cp_prepare(struct clk_hw *hw)
{
	struct pll_hard_vote_to_cp *pclk = to_pll_hard_vote_to_cp(hw);
    u32    en_dis_reg_val = 0;
    u32    lock_status = 0;

    en_dis_reg_val = readl(pclk->en_dis_reg_base + pclk->en_dis_offset);
    en_dis_reg_val |= 0x1 << pclk->en_dis_bit;
    writel(en_dis_reg_val, pclk->en_dis_reg_base + pclk->en_dis_offset); 
    (void)readl(pclk->en_dis_reg_base + pclk->en_dis_offset);

    /* 等稳定 */
    do {
        lock_status = readl(pclk->en_dis_reg_base + pclk->lock_status_offset);
        lock_status &= (0x1 << pclk->lock_status_bit);
    } while (lock_status != (0x1 << pclk->lock_status_bit));

    return 0;    
}

static void pll_hard_vote_to_cp_unprepare(struct clk_hw *hw)
{
	struct pll_hard_vote_to_cp *pclk = to_pll_hard_vote_to_cp(hw);
    u32    en_dis_reg_val = 0;

    en_dis_reg_val = readl(pclk->en_dis_reg_base + pclk->en_dis_offset);
    en_dis_reg_val &= (~(0x1 << pclk->en_dis_bit));
    writel(en_dis_reg_val, pclk->en_dis_reg_base + pclk->en_dis_offset); 
    (void)readl(pclk->en_dis_reg_base + pclk->en_dis_offset);
}

static unsigned long pll_hard_vote_to_cp_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	UNUSED(parent_rate);
	return to_pll_hard_vote_to_cp(hw)->rate;
}

static struct clk_ops pll_hard_vote_to_cp_ops = {
	.prepare		= pll_hard_vote_to_cp_prepare,      
	.unprepare	    = pll_hard_vote_to_cp_unprepare,
	.recalc_rate    = pll_hard_vote_to_cp_recalc_rate,  
};

static void __init hisi_hard_vote_to_mdm_pll_setup(struct device_node *np)
{
	struct pll_hard_vote_to_cp *pll_hard_vote_to_cp;
	struct clk_init_data *init;
	struct clk *clk;
	const char *clk_name;
	unsigned int rate = 0;
    u32 en_dis_data[2] = {0};
    u32 lock_status_data[2] = {0};
	void __iomem *reg_base;

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] %s node doesn't have clock-output-name property!\n",
			 __func__, np->name);
		return;
	}

	/* 获取频率 */
	if (of_property_read_u32(np, "clock-frequency", &rate)){
		pr_err("[%s] %s node doesn't have clock-frequency property!\n",
			 __func__, np->name);
		return;
	}

    /* 获取基址 */
	reg_base = hs_clk_get_base(np);
	if (!reg_base) {
		pr_err("[%s] fail to get reg_base!\n", __func__);
		return;
	}

    /* 获取打开和关闭的偏移及bit */
	if (of_property_read_u32_array(np, "en_dis_offset_bit", &en_dis_data[0], 2)) 
    {
		pr_err("[%s] %s node doesn't have en_dis_offset_bit property!\n",
			 __func__, np->name);
		return;
	}

    /* 获取锁定状态的偏移及bit */
	if (of_property_read_u32_array(np, "lock_offset_bit", &lock_status_data[0], 2)) 
    {
		pr_err("[%s] %s node doesn't have lock_offset_bit property!\n",
			 __func__, np->name);
		return;
	}

	pll_hard_vote_to_cp = kzalloc(sizeof(struct pll_hard_vote_to_cp), GFP_KERNEL);
	if (!pll_hard_vote_to_cp) {
		pr_err("[%s] fail to alloc pll_hard_vote_to_cp!\n", __func__);
		goto err_pll_hard_vote_to_cp;
	}
	
	init = kzalloc(sizeof(struct clk_init_data), GFP_KERNEL);
	if (!init) {
		pr_err("[%s] fail to alloc init!\n", __func__);
		goto err_init;
	}
	init->name = kstrdup(clk_name, GFP_KERNEL);
	init->ops = &pll_hard_vote_to_cp_ops;
	init->flags = CLK_IS_ROOT;

    pll_hard_vote_to_cp->rate = (unsigned long)rate;
    pll_hard_vote_to_cp->en_dis_reg_base = reg_base;    
    pll_hard_vote_to_cp->en_dis_offset = en_dis_data[0];
    pll_hard_vote_to_cp->en_dis_bit = en_dis_data[1];
    pll_hard_vote_to_cp->lock_status_offset = lock_status_data[0];
    pll_hard_vote_to_cp->lock_status_bit = lock_status_data[1];
	pll_hard_vote_to_cp->lock = &hs_clk.lock;
	pll_hard_vote_to_cp->hw.init = init;

	clk = clk_register(NULL, &pll_hard_vote_to_cp->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n",
			__func__, clk_name);
		goto err_clk;
	}

	clk_register_clkdev(clk, clk_name, NULL);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	/* coverity[leaked_storage] */
	return;

err_clk:
	kfree(init);
err_init:
	kfree(pll_hard_vote_to_cp);
err_pll_hard_vote_to_cp:
	return;
}


static int __init hi3xxx_parse_mux(struct device_node *np,
				   u8 *num_parents)
{
	int i, cnt;

	/* get the count of items in mux */
	for (i = 0, cnt = 0;; i++, cnt++) {
		/* parent's #clock-cells property is always 0 */
		if (!of_parse_phandle(np, "clocks", i))
			break;
	}

	for (i = 0; i < cnt; i++) {
		if (!of_clk_get_parent_name(np, i)) {
			pr_err("[%s] cannot get %dth parent_clk name!\n",
				__func__, i);
			return -ENOENT;
		}
	}
	*num_parents = cnt;

	return 0;
}

static void __init hisi_clkmux_setup(struct device_node *np)
{
	struct clk *clk;
	const char *clk_name, **parent_names = NULL;
	u32 rdata[2] = {0};
	u32 width = 0;
	u8 num_parents, shift, flag = 0;
	void __iomem *reg, *base;
	int i, ret;

	base = hs_clk_get_base(np);
	if (!base) {
		pr_err("[%s] fail to get base!\n", __func__);
		return;
	}

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] %s node doesn't have clock-output-name property!\n",
				__func__, np->name);
		return;
	}
	if (of_property_read_u32_array(np, "hisilicon,clkmux-reg",
				       &rdata[0], 2)) {
		pr_err("[%s] %s node doesn't have clkmux-reg property!\n",
				__func__, np->name);
		return;
	}

	//if (of_property_read_bool(np, "hiword"))
		//flag = CLK_MUX_HIWORD_MASK;

	ret = hi3xxx_parse_mux(np, &num_parents);
	if (ret) {
		pr_err("[%s] %s node cannot get num_parents!\n",
			__func__, np->name);
		return;
	}

	parent_names = kzalloc(sizeof(char *) * num_parents, GFP_KERNEL);
	if (!parent_names) {
		pr_err("[%s] fail to alloc parent_names!\n", __func__);
		return;
	}
	for (i = 0; i < num_parents; i++)
		parent_names[i] = of_clk_get_parent_name(np, i);

	reg = base + rdata[0];
	shift = ffs(rdata[1]) - 1;/*cal reg shift*/
	width = fls(rdata[1]) - ffs(rdata[1]) + 1;/*cal reg width*/

	clk = clk_register_mux(NULL, clk_name, parent_names, num_parents,
				     CLK_SET_RATE_PARENT, reg, shift, width,
				     flag, &hs_clk.lock);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to register mux clk %s!\n",
			__func__, clk_name);
		goto err_clk;
	}

	clk_register_clkdev(clk, clk_name, NULL);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);

	return;
err_clk:
	kfree(parent_names);
}

static unsigned int hisi_get_table_maxdiv(const struct clk_div_table *table)
{
	unsigned int maxdiv = 0;
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div > maxdiv)
			maxdiv = clkt->div;
	return maxdiv;
}

static unsigned int hisi_get_table_div(const struct clk_div_table *table,
							unsigned int val)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->val == val)
			return clkt->div;
	return 0;
}

static unsigned int hisi_get_table_val(const struct clk_div_table *table,
					 unsigned int div)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div == div)
			return clkt->val;
	return 0;
}

static unsigned long hisi_clkdiv_recalc_rate(struct clk_hw *hw,
					       unsigned long parent_rate)
{
	struct hisi_divclk *dclk = container_of(hw, struct hisi_divclk, hw);
	unsigned int div, val;


	val = readl(dclk->reg) >> dclk->shift;
	val &= WIDTH_TO_MASK(dclk->width);

	div = hisi_get_table_div(dclk->table, val);
	if (!div) {
		pr_warn("%s: Invalid divisor for clock %s\n", __func__,
			   __clk_get_name(hw->clk));
		return parent_rate;
	}


	return parent_rate / div;
}

static bool hisi_is_valid_table_div(const struct clk_div_table *table,
				      unsigned int div)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->div == div)
			return true;
	return false;
}

static int hisi_clkdiv_bestdiv(struct clk_hw *hw, unsigned long rate,
				 unsigned long *best_parent_rate)
{
	struct hisi_divclk *dclk = container_of(hw, struct hisi_divclk, hw);
	struct clk *clk_parent = __clk_get_parent(hw->clk);
	int i, bestdiv = 0;
	unsigned long parent_rate, best = 0, now, maxdiv;

	maxdiv = hisi_get_table_maxdiv(dclk->table);

	if (!(__clk_get_flags(hw->clk) & CLK_SET_RATE_PARENT)) {
		parent_rate = *best_parent_rate;
		bestdiv = DIV_ROUND_UP(parent_rate, rate);
		bestdiv = bestdiv == 0 ? 1 : bestdiv;
		bestdiv = bestdiv > maxdiv ? maxdiv : bestdiv;
		return bestdiv;
	}

	/*
	 * The maximum divider we can use without overflowing
	 * unsigned long in rate * i below
	 */
	maxdiv = min(ULONG_MAX / rate, maxdiv);

	for (i = 1; i <= maxdiv; i++) {
		if (!hisi_is_valid_table_div(dclk->table, i))
			continue;
		parent_rate = __clk_round_rate(clk_parent,
					       MULT_ROUND_UP(rate, i));
		now = parent_rate / i;
		if (now <= rate && now > best) {
			bestdiv = i;
			best = now;
			*best_parent_rate = parent_rate;
		}
	}

	if (!bestdiv) {
		bestdiv = hisi_get_table_maxdiv(dclk->table);
		*best_parent_rate = __clk_round_rate(clk_parent, 1);
	}

	return bestdiv;
}

static long hisi_clkdiv_round_rate(struct clk_hw *hw, unsigned long rate,
				     unsigned long *prate)
{
	int div;

	if (!rate)
		rate = 1;
	div = hisi_clkdiv_bestdiv(hw, rate, prate);

	return *prate / div;
}

static int hisi_clkdiv_set_rate(struct clk_hw *hw, unsigned long rate,
				  unsigned long parent_rate)
{
	struct hisi_divclk *dclk = container_of(hw, struct hisi_divclk, hw);
	unsigned int div, value;
	unsigned long flags = 0;
	u32 data;
	int ret = 0;

	/***** pm log *****/
    __clk_pm_log(hw, CLK_OP_TYPE_SETR);

    /* should en refclk before set div,or the new div will not take effect */	
	if (1 == dclk->en_ref_before_set_div)
	{
		ret = __clk_prepare(hw->clk);
		if (ret) 
		{
			pr_err("[%s], en_ref_before_set_div __clk_prepare err ret=%d\n", __func__, ret);
			return ret;
		}
		ret = __clk_enable(hw->clk);
		if (ret) 
		{
			__clk_unprepare(hw->clk);
			pr_err("[%s], en_ref_before_set_div __clk_enable err ret=%d\n", __func__, ret);
			return ret;
		}
	}

	div = parent_rate / rate;
	value = hisi_get_table_val(dclk->table, div);

	if (value > WIDTH_TO_MASK(dclk->width))
		value = WIDTH_TO_MASK(dclk->width);

	if (dclk->lock)
		spin_lock_irqsave(dclk->lock, flags);

	data = readl(dclk->reg);
	data &= ~(WIDTH_TO_MASK(dclk->width) << dclk->shift);
	data |= value << dclk->shift;
	data |= dclk->mbits;
	writel(data, dclk->reg);
    (void)readl(dclk->reg);

	if (dclk->lock)
		spin_unlock_irqrestore(dclk->lock, flags);

	if (1 == dclk->en_ref_before_set_div)
	{
		__clk_disable(hw->clk);
		__clk_unprepare(hw->clk);
	}
	
	return 0;
}

#ifdef CONFIG_CLK_DEBUG
static int hisi_divreg_check(struct clk_hw *hw)
{
	unsigned long rate;
	struct clk *clk = hw->clk;
	struct clk *pclk = clk_get_parent(clk);

	rate = hisi_clkdiv_recalc_rate(hw, clk_get_rate(pclk));
	if (rate == clk_get_rate(clk))
		return 1;
	else
		return 0;
}

static void __iomem *hisi_clkdiv_get_reg(struct clk_hw *hw)
{
	struct hisi_divclk *dclk;
	void __iomem	*ret = NULL;
	u32 val = 0;

	dclk = container_of(hw, struct hisi_divclk, hw);

	if (dclk->reg) {
		ret = dclk->reg;
		val = readl(ret);
		val &= dclk->mbits;
		pr_info("\n[%s]: reg = 0x%p, bits = 0x%x, regval = 0x%x\n",
			hw->clk->name, ret, dclk->mbits, val);
	}

	return ret;
}
#endif

static struct clk_ops hisi_clkdiv_ops = {
	.recalc_rate = hisi_clkdiv_recalc_rate,
	.round_rate = hisi_clkdiv_round_rate,
	.set_rate = hisi_clkdiv_set_rate,
#ifdef CONFIG_CLK_DEBUG
	.check_divreg = hisi_divreg_check,
	.get_reg = hisi_clkdiv_get_reg,
#endif
};

void __init hisi_clkdiv_setup(struct device_node *np)
{
	struct clk *clk;
	const char *clk_name, *parent_names;
	struct clk_init_data *init;
	struct clk_div_table *table;
	struct hisi_divclk *dclk;
	void __iomem *reg_base;
	unsigned int table_num;
	int i;
	u32 data[3] = {0};
	unsigned int max_div, min_div, div_flag;

	reg_base = hs_clk_get_base(np);
	if (!reg_base) {
		pr_err("[%s] fail to get reg_base!\n", __func__);
		return;
	}

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] node %s doesn't have clock-output-names property!\n",
			__func__, np->name);
		return;
	}

	/* process the div_table */
	if (of_property_read_u32_array(np, "hisilicon,clkdiv-table",
				       &data[0], 3)) {
		pr_err("[%s] node %s doesn't have clkdiv-table property!\n",
			__func__, np->name);
		return;
	}

	max_div = data[0];
	min_div = data[1];
    div_flag = data[2];/*diff 0:N-N and 1:N-N+1*/

	if (of_property_read_u32_array(np, "hisilicon,clkdiv",
								&data[0], 2)) {
		pr_err("[%s] node %s doesn't have clkdiv property!\n",
			__func__, np->name);
		return;
	}

	table_num = max_div - min_div + 1;

	/* table ends with <0, 0>, so plus one to table_num */
	table = kzalloc(sizeof(struct clk_div_table) * (table_num + 1), GFP_KERNEL);
	if (!table) {
		pr_err("[%s] fail to alloc table!\n", __func__);
		return;
	}

	for (i = 0; i < table_num; i++) {
		table[i].div = min_div + i;
		table[i].val = table[i].div - div_flag;
	}

	/* mux has the fixed parent */

	parent_names = of_clk_get_parent_name(np, 0);

	dclk = kzalloc(sizeof(struct hisi_divclk), GFP_KERNEL);
	if (!dclk) {
		pr_err("[%s] fail to alloc dclk!\n", __func__);
		goto err_par;
	}

	if (of_property_read_bool(np, "en_ref_before_set_div"))  
	{
	    dclk->en_ref_before_set_div = 1;
	}
	
	init = kzalloc(sizeof(struct clk_init_data), GFP_KERNEL);
	if (!init) {
		pr_err("[%s] fail to alloc init!\n", __func__);
		goto err_init;
	}
	init->name = kstrdup(clk_name, GFP_KERNEL);
	init->ops = &hisi_clkdiv_ops;
	init->parent_names = &parent_names;
	init->num_parents = 1;

	dclk->reg = reg_base + data[0];
	dclk->shift = ffs(data[1]) - 1;
	dclk->width = fls(data[1]) - ffs(data[1]) + 1;
	dclk->mbits = data[1] << 16;
	dclk->lock = &hs_clk.lock;
	dclk->hw.init = init;
	dclk->table = table;
	clk = clk_register(NULL, &dclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to register div clk %s!\n",
				__func__, clk_name);
		goto err_clk;
	}
	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	clk_register_clkdev(clk, clk_name, NULL);
	return;
err_clk:
	kfree(init);
err_init:
	kfree(dclk);

err_par:
	kfree(table);
}

char *xfreq_devname[] = {
	"XFREQ_A15_CPU",
	"XFREQ_A7_CPU",
	"XFREQ_GPU",
	"XFREQ_DDR"
};

#define BITS(i,n) (i & (1 << n))

#ifdef XFREQ_CLK

unsigned long pll_freq(unsigned long param0, unsigned long param1)
{
       if (!BITS(param0,0) || !BITS(param1,26) || !BITS(param1,25)) {
		return 0;
       } else if (BITS(param0,0) & BITS(param0,1)) {
		return 192;
       } else
		return (((((192 * ((param0 & 0xFFF00) + ((param1 >> 16) & 0xFF))) / (((param0 >> 2) & 0x3F) * ((param0 >> 23) & 0x07) * ((param0 >> 20) & 0x07))) + 0x80) >> 8));
}

static unsigned long hisi_xfreq_clk_recalc_rate(struct clk_hw *hw,
					       unsigned long parent_rate)
{
	struct hisi_xfreq_clk *xfreq_clk = container_of(hw, struct hisi_xfreq_clk, hw);
	u32 rate, pll_sel,pllreg,pllreg2 = 0;
	u32 rate2,ddrdiv,ddrdiv2 = 0;
	char clk_name[20] = {0};
	struct clk *clk;
	u32 ddrc_div, aclk_div, busclk_div;
	int loop = MAX_READ_DDRFREQ_TRY;

	switch(xfreq_clk->id) {
	/* A15 */
	case 0:
	/* A7 */
	case 1:
		pll_sel = readl(xfreq_clk->pll_sel_reg) & xfreq_clk->pll_sel_mask;

		if (pll_sel > 2)
				pll_sel = 2;

		strncpy(clk_name, "clk_appll0", 10);
		clk_name[strlen(clk_name) - 1] = pll_sel + '0';

		clk = __clk_lookup(clk_name);
		if (IS_ERR_OR_NULL(clk)) {
			if (xfreq_clk->rate == 0)
				xfreq_clk->rate = clk_get_rate(hw->clk->parent);

			rate = xfreq_clk->rate;
			break;
		}

		rate = clk_get_rate(clk);
		break;

	/* DDR get freq */
	case 3:
		/*according the process of the ddr Freq and that signal sequence.
		 * the dynamicly process of ddr Freq refer to the register of ppll0,
		 * ppll1 ,ddr sel and ddr div. the ppll0 is fixed,but ppll1 is changeable.
		 * the signal sequence of ddr Freq in LPM3 contain the 60us rasing or failing
		 * signal which is intermediate state seting ddr freq,and the 20ms
		 * stable state.
		*/
		strncpy(clk_name, "ddrc_ppll0", 10);
		do {
			pllreg = readl(xfreq_clk->pll_sel_reg);
			pll_sel = pllreg & xfreq_clk->pll_sel_mask;
			pll_sel = pll_sel >> (ffs(xfreq_clk->pll_sel_mask) - 1);
			pll_sel = ffs(pll_sel) - 1;

			ddrdiv = readl(xfreq_clk->pll_div_reg);

			clk_name[strlen(clk_name) - 1] = pll_sel + '0';
			clk = __clk_lookup(clk_name);
			if (IS_ERR_OR_NULL(clk)) {
				pr_err("[%s]get ddrc_ppll failed pll_sel 0x%x\n",__func__, pll_sel);
				rate = xfreq_clk->rate;
				return rate;
			}
			rate = clk_get_rate(clk);
			udelay(100);

			dsb();
			/*reback read*/
			pllreg2 = readl(xfreq_clk->pll_sel_reg);
			pll_sel = pllreg2 & xfreq_clk->pll_sel_mask;
			pll_sel = pll_sel >> (ffs(xfreq_clk->pll_sel_mask) - 1);
			pll_sel = ffs(pll_sel) - 1;

			ddrdiv2 = readl(xfreq_clk->pll_div_reg);

			clk_name[strlen(clk_name) - 1] = pll_sel + '0';
			clk = __clk_lookup(clk_name);
			if (IS_ERR_OR_NULL(clk)) {
				pr_err("[%s]get ddrc_ppll failed pll_sel 0x%x\n",__func__, pll_sel);
				rate2 = xfreq_clk->rate;
				return rate2;
			}
			rate2 = clk_get_rate(clk);

			loop--;
		}while((pllreg2 != pllreg || rate2 != rate || rate == 0 || ddrdiv2 != ddrdiv)  && loop > 0);

		ddrc_div = ddrdiv;
		aclk_div = ddrdiv & xfreq_clk->pll_div_aclk_mask;
		busclk_div = ddrdiv & xfreq_clk->pll_div_busclk_mask;

		ddrc_div = ddrc_div & xfreq_clk->pll_div_mask;
		ddrc_div = ddrc_div + 1;
		aclk_div = aclk_div >> (ffs(xfreq_clk->pll_div_aclk_mask) - 1);
		aclk_div = aclk_div + 1;
		busclk_div = busclk_div >> (ffs(xfreq_clk->pll_div_busclk_mask) - 1);
		busclk_div = busclk_div + 1;

		if (busclk_div == 1)
			rate = (2 *rate) / (ddrc_div * aclk_div);
		else
			rate = rate / (ddrc_div * aclk_div);

		break;
	/* DDR set min */
	case 4:
	default:
		rate = xfreq_clk->rate;
	}

	return rate;
}

static long hisi_xfreq_clk_round_rate(struct clk_hw *hw, unsigned long rate,
				     unsigned long *prate)
{
	return rate;
}

static long hisi_xfreq_clk_determine_rate(struct clk_hw *hw, unsigned long rate,
					unsigned long *best_parent_rate,
					struct clk **best_parent_clk)
{
	return rate;
}

static int hisi_xfreq_clk_set_rate(struct clk_hw *hw, unsigned long rate,
				  unsigned long parent_rate)
{
	struct hisi_xfreq_clk *xfreq_clk = container_of(hw, struct hisi_xfreq_clk, hw);
	unsigned long new_rate = rate/1000000;
	int ret = 0;

	pr_debug("[%s] %s set rate = %ldMHZ\n", __func__,
				xfreq_devname[xfreq_clk->id], new_rate);
	xfreq_clk->set_rate_cmd[1] = new_rate;

#ifdef CONFIG_HI3630_CLK_MAILBOX_SUPPORT
	ret = hisi_clkmbox_send_msg(xfreq_clk->set_rate_cmd, NULL, AUTO_ACK);
	if (ret < 0) {
		pr_err("[%s] %s fail to send msg to LPM3!\n",
					__func__, xfreq_devname[xfreq_clk->id]);

		return -EINVAL;
	}
#endif

	xfreq_clk->rate = rate;
	return ret;
}

static struct clk_ops hisi_xfreq_clk_ops = {
	.recalc_rate = hisi_xfreq_clk_recalc_rate,
//	.determine_rate = hisi_xfreq_clk_determine_rate,
	.round_rate = hisi_xfreq_clk_round_rate,
	.set_rate = hisi_xfreq_clk_set_rate,
};

/*
 * xfreq_clk is used for cpufreq & devfreq.
 */
void __init hisi_xfreq_clk_setup(struct device_node *np)
{
	struct clk *clk;
	const char *clk_name, *parent_names;
	struct clk_init_data *init = NULL;
	struct hisi_xfreq_clk *xfreqclk;
	u32 get_rate_cmd[LPM3_CMD_LEN], set_rate_cmd[LPM3_CMD_LEN];
	u32 device_id = 0;
	#if (0 != LPM3_CMD_LEN)
	u32 i = 0;
	#endif
	void __iomem *reg_base;
	u32 pll_sel_array[2] = {0}, pll_div_array[4] = {0};

	reg_base = hs_clk_get_base(np);
	if (!reg_base) {
		pr_err("[%s] fail to get reg_base!\n", __func__);
		return;
	}

	if (NULL == of_clk_get_parent_name(np, 0))
	    parent_names = NULL;
	else
		parent_names = of_clk_get_parent_name(np, 0);

	if (of_property_read_u32(np, "hisilicon,hisi-xfreq-devid", &device_id)) {
		pr_err("[%s] node %s doesn't have clock-output-names property!\n",
			__func__, np->name);
		goto err_prop;
	}

	if (of_property_read_u32_array(np, "hisilicon,pll-sel", &pll_sel_array[0], 2)) {
		pr_err("[%s] node %s doesn't hisilicon,pll-sel property!\n",
			__func__, np->name);
		goto err_prop;
	}

	if (of_property_read_u32_array(np, "hisilicon,pll-div", &pll_div_array[0], 4)) {
		pr_err("[%s] node %s doesn't hisilicon,pll-div property!\n",
			__func__, np->name);
		goto err_prop;
	}

	if (of_property_read_u32_array(np, "hisilicon,get-rate-ipc-cmd", &get_rate_cmd[0], LPM3_CMD_LEN)) {
		pr_err("[%s] node %s doesn't get-rate-ipc-cmd property!\n",
			__func__, np->name);
		goto err_prop;
	}

	if (of_property_read_u32_array(np, "hisilicon,set-rate-ipc-cmd", &set_rate_cmd[0], LPM3_CMD_LEN)) {
		pr_err("[%s] node %s doesn't set-rate-ipc-cmd property!\n",
			__func__, np->name);
		goto err_prop;
	}

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] node %s doesn't have clock-output-names property!\n",
			__func__, np->name);
		goto err_prop;
	}

	xfreqclk = kzalloc(sizeof(struct hisi_xfreq_clk), GFP_KERNEL);
	if (!xfreqclk) {
		pr_err("[%s] fail to alloc xfreqclk!\n", __func__);
		goto err_prop;
	}
	init = kzalloc(sizeof(struct clk_init_data), GFP_KERNEL);
	if (!init) {
		pr_err("[%s] fail to alloc init!\n", __func__);
		goto err_init;
	}
	init->name = kstrdup(clk_name, GFP_KERNEL);
	init->ops = &hisi_xfreq_clk_ops;
	init->parent_names = (parent_names ? &parent_names: NULL);
	init->num_parents = (parent_names ? 1 : 0);
	init->flags = CLK_IS_ROOT | CLK_GET_RATE_NOCACHE;

    #if (0 != LPM3_CMD_LEN)
	for (i = 0; i < LPM3_CMD_LEN; i++) {
		xfreqclk->set_rate_cmd[i] = set_rate_cmd[i];
		xfreqclk->get_rate_cmd[i] = get_rate_cmd[i];
	}
	#endif

	if (2 == device_id)
		xfreqclk->rate = 360000000;

	xfreqclk->hw.init = init;
	xfreqclk->id = device_id;
	xfreqclk->pll_sel_reg = reg_base + pll_sel_array[0];
	xfreqclk->pll_sel_mask = pll_sel_array[1];
	xfreqclk->pll_div_reg = reg_base + pll_div_array[0];
	xfreqclk->pll_div_mask = pll_div_array[1];
	xfreqclk->pll_div_aclk_mask = pll_div_array[2];
	xfreqclk->pll_div_busclk_mask = pll_div_array[3];

	clk = clk_register(NULL, &xfreqclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to register xfreqclk %s!\n",
				__func__, clk_name);
		goto err_clk;
	}

	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	clk_register_clkdev(clk, clk_name, NULL);

	return;

err_clk:
	kfree(init);
err_init:
	kfree(xfreqclk);
err_prop:
	return;
}

static unsigned long hisi_xfreq_pll_recalc_rate(struct clk_hw *hw,
					       unsigned long parent_rate)
{
	struct hisi_xfreq_pll *xfreq_pll = container_of(hw, struct hisi_xfreq_pll, hw);
	unsigned long val1, val2, rate;

	val1 = readl(xfreq_pll->reg);
	val2 = readl(xfreq_pll->reg + PLL_CTRL1_OFFSET);

	rate = pll_freq(val1, val2) * 100000;

	return 	rate;
}

static long hisi_xfreq_pll_round_rate(struct clk_hw *hw, unsigned long rate,
				     unsigned long *prate)
{
	return rate;
}

static long hisi_xfreq_pll_determine_rate(struct clk_hw *hw, unsigned long rate,
					unsigned long *best_parent_rate,
					struct clk **best_parent_clk)
{
	return rate;
}

static int hisi_xfreq_pll_set_rate(struct clk_hw *hw, unsigned long rate,
				  unsigned long parent_rate)
{
	return 0;
}

static struct clk_ops hisi_xfreq_pll_ops = {
	.recalc_rate = hisi_xfreq_pll_recalc_rate,
//	.determine_rate = hisi_xfreq_pll_determine_rate,
	.round_rate = hisi_xfreq_pll_round_rate,
	.set_rate = hisi_xfreq_pll_set_rate,
};

/*
 * xfreq_clk is used for cpufreq & devfreq.
 */
void __init hisi_xfreq_pll_setup(struct device_node *np)
{
	struct clk *clk;
	const char *clk_name,*parent_names;
	struct clk_init_data *init;
	struct hisi_xfreq_pll *xfreqpll;
	u32 ctrl0_reg = 0;
	void __iomem *reg_base;

	reg_base = hs_clk_get_base(np);
	if (!reg_base) {
		pr_err("[%s] fail to get reg_base!\n", __func__);
		return;
	}

	if (NULL == of_clk_get_parent_name(np, 0))
		parent_names = NULL;
	else
		parent_names = of_clk_get_parent_name(np, 0);

	if (of_property_read_u32(np, "hisilicon,pll-ctrl-reg", &ctrl0_reg)) {
		pr_err("[%s] node %s doesn't have hisilicon,pll-ctrl-reg property!\n",
			__func__, np->name);
		goto err_prop;
	}

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] node %s doesn't have clock-output-names property!\n",
			__func__, np->name);
		goto err_prop;
	}

	xfreqpll = kzalloc(sizeof(struct hisi_xfreq_pll), GFP_KERNEL);
	if (!xfreqpll) {
		pr_err("[%s] fail to alloc xfreqpll!\n", __func__);
		goto err_prop;
	}
	init = kzalloc(sizeof(struct clk_init_data), GFP_KERNEL);
	if (!init) {
		pr_err("[%s] fail to alloc init!\n", __func__);
		goto err_init;
	}
	init->name = kstrdup(clk_name, GFP_KERNEL);
	init->ops = &hisi_xfreq_pll_ops;
	init->parent_names = (parent_names ? &parent_names: NULL);
	init->num_parents = (parent_names ? 1 : 0);
	init->flags = CLK_IS_ROOT | CLK_GET_RATE_NOCACHE;

	xfreqpll->hw.init = init;
	xfreqpll->reg = ctrl0_reg + reg_base;

	clk = clk_register(NULL, &xfreqpll->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to register xfreqpll %s!\n",
				__func__, clk_name);
		goto err_clk;
	}

	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	clk_register_clkdev(clk, clk_name, NULL);

	return;

err_clk:
	kfree(init);
err_init:
	kfree(xfreqpll);
err_prop:
	return;
}
#endif

#ifdef MCORE_CLK
static int hisi_mclk_enable(struct clk_hw *hw)
{
	struct hisi_mclk *mclk;
#ifdef CONFIG_HI3630_CLK_MAILBOX_SUPPORT
	s32 ret;
#endif
	mclk = container_of(hw, struct hisi_mclk, hw);
	mclk->ref_cnt++;

#ifdef CONFIG_HI3630_CLK_MAILBOX_SUPPORT
	/* notify m3 when the ref_cnt of mclk is 1 */
	if (mclk->ref_cnt < 2) {
		ret = hisi_clkmbox_send_msg(&mclk->en_cmd[0], NULL, 0);
		if (ret)
			pr_err("[%s] fail to enable clk, ret = %d!\n",__func__, ret);
	}
#endif

	return 0;
}

static void hisi_mclk_disable(struct clk_hw *hw)
{
	struct hisi_mclk *mclk;
#ifdef CONFIG_HI3630_CLK_MAILBOX_SUPPORT
	s32 ret;
#endif
	mclk = container_of(hw, struct hisi_mclk, hw);
	mclk->ref_cnt--;

#ifdef CONFIG_HI3630_CLK_MAILBOX_SUPPORT
	/* notify m3 when the ref_cnt of gps_clk is 0 */
	if (!mclk->ref_cnt) {
		ret = hisi_clkmbox_send_msg(&mclk->dis_cmd[0], NULL, 0);
		if (ret)
			pr_err("[%s] fail to disable clk, ret = %d!\n",__func__, ret);
	}
#endif
}

static struct clk_ops hisi_mclk_ops = {
	.enable		= hisi_mclk_enable,
	.disable	= hisi_mclk_disable,
};

static void __init hisi_mclk_setup(struct device_node *np)
{
	struct hisi_mclk *mclk;
	struct clk_init_data *init;
	struct clk *clk;
	const char *clk_name, *parent_names;
	u32 en_cmd[LPM3_CMD_LEN];
	u32 dis_cmd[LPM3_CMD_LEN];
	#if (0 != LPM3_CMD_LEN)
	u32 i;
	#endif

	if (of_property_read_string(np, "clock-output-names", &clk_name)) {
		pr_err("[%s] %s node doesn't have clock-output-name property!\n",
			 __func__, np->name);
		return;
	}

	if (of_property_read_u32_array(np, "hisilicon,ipc-lpm3-cmd-en", &en_cmd[0], LPM3_CMD_LEN)) {
		pr_err("[%s] %s node doesn't have hisilicon,ipc-modem-cmd property!\n",
			 __func__, np->name);
		return;
	}

	if (of_property_read_u32_array(np, "hisilicon,ipc-lpm3-cmd-dis", &dis_cmd[0], LPM3_CMD_LEN)) {
		pr_err("[%s] %s node doesn't have hisilicon,ipc-modem-cmd property!\n",
			 __func__, np->name);
		return;
	}


	parent_names = of_clk_get_parent_name(np, 0);

	mclk = kzalloc(sizeof(struct hisi_mclk), GFP_KERNEL);
	if (!mclk) {
		pr_err("[%s] fail to alloc pclk!\n", __func__);
		goto err_mclk;
	}

	init = kzalloc(sizeof(struct clk_init_data), GFP_KERNEL);
	if (!init) {
		pr_err("[%s] fail to alloc init!\n", __func__);
		goto err_init;
	}
	init->name = kstrdup(clk_name, GFP_KERNEL);
	init->ops = &hisi_mclk_ops;
	init->flags = CLK_SET_RATE_PARENT;
	init->parent_names = &parent_names;
	init->num_parents = 1;

    #if (0 != LPM3_CMD_LEN)
	for (i = 0; i < LPM3_CMD_LEN; i++)
		mclk->en_cmd[i] = en_cmd[i];
	for (i = 0; i < LPM3_CMD_LEN; i++)
		mclk->dis_cmd[i] = dis_cmd[i];
	#endif

	/* initialize the reference count */
	mclk->ref_cnt = 0;
	mclk->lock = &hs_clk.lock;
	mclk->hw.init = init;

	clk = clk_register(NULL, &mclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n",
			__func__, clk_name);
		goto err_clk;
	}

	clk_register_clkdev(clk, clk_name, NULL);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);

	return;

err_clk:
	kfree(init);
err_init:
	kfree(mclk);
err_mclk:
	return;
}
#endif

CLK_OF_DECLARE(hisi_mux, "hisilicon,hisi-clk-mux", hisi_clkmux_setup)
CLK_OF_DECLARE(hisi_div, "hisilicon,hisi-clk-div", hisi_clkdiv_setup)
CLK_OF_DECLARE(hisi_gate, "hisilicon,hisi-clk-gate", hisi_clkgate_setup)
CLK_OF_DECLARE(hisi_ap_dfs_ctrl, "hisilicon,pll_ctrl_by_ap", hisi_pll_ctrl_by_ap_setup)
//CLK_OF_DECLARE(hisi_pll_ctrl_by_cp, "hisilicon,pll_ctrl_by_cp", hisi_pll_ctrl_by_cp_setup)
//CLK_OF_DECLARE(hisi_cpu, "hisilicon,hisi-xfreq-clk", hisi_xfreq_clk_setup)
//CLK_OF_DECLARE(hisi_xfreq_pll, "hisilicon,hisi-xfreq-pll", hisi_xfreq_pll_setup)
//CLK_OF_DECLARE(hisi_mclk, "hisilicon,modem-clk", hisi_mclk_setup)
CLK_OF_DECLARE(hisi_hard_vote_to_mdm_pll, "hisilicon,hard_vote_to_mdm_pll", hisi_hard_vote_to_mdm_pll_setup)



static const struct of_device_id hs_of_match[] = {
	{ .compatible = "hisilicon,clk-ao-crg",	.data = (void *)HS_AO_CRG, },
	{ .compatible = "hisilicon,clk-pd-crg",	.data = (void *)HS_PD_CRG, },
	{ .compatible = "hisilicon,clk-modem-crg",	.data = (void *)HS_MODEM_CRG, },
    {}
};

static void __iomem __init *hs_clk_get_base(struct device_node *np)
{
	struct device_node *parent;
	struct device_node *grandpa;
	const struct of_device_id *match;
	void __iomem *ret = NULL;
	unsigned int read_data_from_node = 0;

	parent = of_get_parent(np);
	if (!parent) {
		pr_err("[%s] node %s doesn't have parent node!\n", __func__, np->name);
		goto out;
	}
	match = of_match_node(hs_of_match, parent);
	if (!match) {
		pr_err("[%s] parent node %s doesn't match!\n", __func__, parent->name);
		goto out;
	}

	/* get the offset status reg to enable reg */
	if (!g_status_reg_offset_to_enable)  /* only once */
	{
		grandpa = of_get_parent(parent);
		if (!grandpa) 
		{
			pr_err("node %s doesn't have parent node!\n", parent->name);
			goto out;
		}
		
		if (of_property_read_u32(grandpa, "status_reg_offset", &read_data_from_node)) 
		{
			pr_err("[%d] %s node doesn't have status_reg_offset property!\n",
				                  __LINE__, grandpa->name);
			goto out;
		}

		g_status_reg_offset_to_enable = read_data_from_node;
		//pr_err("acore---------------g_status_reg_offset_to_enable = %d\n", g_status_reg_offset_to_enable);

	}

	
	switch ((unsigned int)match->data) {
	case HS_AO_CRG:
		if (!hs_clk.ao_crg) {
			ret = of_iomap(parent, 0);
			WARN_ON(!ret);
			hs_clk.ao_crg = ret;
		} else {
			ret = hs_clk.ao_crg;
		}
		break;
	case HS_PD_CRG:
		if (!hs_clk.pd_crg) {
			ret = of_iomap(parent, 0);
			WARN_ON(!ret);
			hs_clk.pd_crg = ret;
		} else {
			ret = hs_clk.pd_crg;
		}
		break;
	case HS_MODEM_CRG:
		if (!hs_clk.modem_crg) {
			ret = of_iomap(parent, 0);
			WARN_ON(!ret);
			hs_clk.modem_crg = ret;
		} else {
			ret = hs_clk.modem_crg;
		}
		break;
	default:
		pr_err("[%s] cannot find the match node!\n", __func__);
		ret = NULL;
	}
out:
	return ret;
}
