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
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <asm/current.h>
#include <asm/traps.h>
#include "osl_malloc.h"
#include "osl_barrier.h"
#include "bsp_dump.h"
#include "drv_nv_id.h"
#include "drv_nv_def.h"
#include "bsp_nvim.h"
#include "mdrv.h"
#include "watchpoint.h"

wp_ctrl_t   g_wp_ctrl;
wp_step_ctrl_t  g_wp_step_ctrl;
wp_reg_back_t   g_wp_backup;

#define WP_CHECK_INIT() \
    do{\
        if(false == g_wp_ctrl.init_flag)\
        {\
            wp_print("watchpoint not init\n");\
            return BSP_ERROR;\
        }\
    }while(0)

#define WP_CHECK_ID(wp_id) \
    do{\
        if((wp_id > g_wp_ctrl.wp_cnt - 1) || (g_wp_ctrl.wp_info[wp_id].alloc != WATCHPOINT_ALLOCED))\
        {\
            wp_print("invalid watchpoint id<0x%x>\n", wp_id);\
            return BSP_ERROR;\
        }\
    }while(0)
    
/*****************************************************************************
 函 数 名  : read_wp_reg
 功能描述  : 读取指定watchpoint寄存器
 输入参数  : n，寄存器编号
 输出参数  : 无
 返 回 值  : u32, 读取到寄存器值
*****************************************************************************/
u32 read_wp_reg(int n)
{
	u32 val = 0;

	switch (n)
    {
	    GEN_WP_READ_REG_CASES(ARM_OP2_WVR, val);
	    GEN_WP_READ_REG_CASES(ARM_OP2_WCR, val);
        GEN_WP_READ_REG_CASES(ARM_OP2_BVR, val);
        GEN_WP_READ_REG_CASES(ARM_OP2_BCR, val);
	    default:
		    wp_print("attempt to read from unknown breakpoint register %d\n", n);
	}

	return val;
}

/*****************************************************************************
 函 数 名  : write_wp_reg
 功能描述  : 写入指定watchpoint寄存器
 输入参数  : n，寄存器编号, val, 待写入值
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void write_wp_reg(int n, u32 val)
{
	switch (n)
    {
	    GEN_WP_WRITE_REG_CASES(ARM_OP2_WVR, val);
	    GEN_WP_WRITE_REG_CASES(ARM_OP2_WCR, val);
	    GEN_WP_WRITE_REG_CASES(ARM_OP2_BVR, val);
	    GEN_WP_WRITE_REG_CASES(ARM_OP2_BCR, val);
	    default:
		    wp_print("attempt to write to unknown breakpoint register %d\n", n);
	}
	isb();
}

/*****************************************************************************
 函 数 名  : encode_ctrl_reg
 功能描述  : 构建WCR
 输入参数  : ctrl, WCR配置
 输出参数  : 无
 返 回 值  : 待写入WCR的值
*****************************************************************************/
u32 wp_encode_ctrl_reg(watchpoint_ctrl ctrl)
{
    return (ctrl.addr_range << 24) | (ctrl.len << 5) | (ctrl.type << 3) | (ctrl.privilege << 1) | ctrl.enabled;
}

/*****************************************************************************
 函 数 名  : get_addr_range
 功能描述  : 根据起始、结束地址，计算WCR地址范围掩码
 输入参数  : start_addr,监控起始地址；end_addr, 监控结束地址
 输出参数  : 无
 返 回 值  : 待写入WCR地址掩码值
*****************************************************************************/
u32 get_addr_range(u32 start_addr, u32 end_addr)
{
    u32 i;
    u32 mask;

    /* 计算start_addr, end_addr公共前缀长度 */
    for(i=0; i<32; i++)
    {
        mask = 1 << (31 - i);
        if((start_addr & mask) != (end_addr & mask))
        {
            break;
        }
    }

    return 32 - i;
}

/*****************************************************************************
 函 数 名  : wp_alloc_node
 功能描述  : 分配节点
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 成功返回未分配watchpoint id，若已经分配完，返回-1
*****************************************************************************/
s32 wp_alloc_node(void)
{
    unsigned long lock_flag;
    u32 i;

    spin_lock_irqsave(&g_wp_ctrl.lock, lock_flag);
    for(i=0; i<g_wp_ctrl.wp_cnt; i++)
    {
        if(g_wp_ctrl.wp_info[i].alloc == WATCHPOINT_UNALLOC)
        {
            g_wp_ctrl.wp_info[i].alloc = WATCHPOINT_ALLOCED;
            spin_unlock_irqrestore(&g_wp_ctrl.lock, lock_flag);
            return i;
        }
    }

    spin_unlock_irqrestore(&g_wp_ctrl.lock, lock_flag);
    wp_print("all watchpoint nodes alloced\n");
    return BSP_ERROR;
}

/*****************************************************************************
 函 数 名  : wp_free_node
 功能描述  : 释放节点
 输入参数  : wp_id，watchpoint id
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void wp_free_node(s32 wp_id)
{
    unsigned long lock_flag;

    spin_lock_irqsave(&g_wp_ctrl.lock, lock_flag);
    memset(&g_wp_ctrl.wp_info[wp_id], 0, sizeof(wp_info_t));
    spin_unlock_irqrestore(&g_wp_ctrl.lock, lock_flag);
}

/*****************************************************************************
 函 数 名  : wp_show_regs
 功能描述  : 格式化输出寄存器信息
 输入参数  : regs
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void wp_show_regs(struct pt_regs * regs)
{
	wp_print("pc : [<%08lx>]    lr : [<%08lx>]    psr: %08lx\n", regs->ARM_pc, regs->ARM_lr, regs->ARM_cpsr);
    wp_print("sp : %08lx  ip : %08lx  fp : %08lx\n", regs->ARM_sp, regs->ARM_ip, regs->ARM_fp);
	wp_print("r10: %08lx  r9 : %08lx  r8 : %08lx\n",
		regs->ARM_r10, regs->ARM_r9,
		regs->ARM_r8);
	wp_print("r7 : %08lx  r6 : %08lx  r5 : %08lx  r4 : %08lx\n",
		regs->ARM_r7, regs->ARM_r6,
		regs->ARM_r5, regs->ARM_r4);
	wp_print("r3 : %08lx  r2 : %08lx  r1 : %08lx  r0 : %08lx\n",
		regs->ARM_r3, regs->ARM_r2,
		regs->ARM_r1, regs->ARM_r0);
}

/*****************************************************************************
 函 数 名  : disable_single_step_ctrl
 功能描述  : 清除breakpoint，恢复watchpoint监控
 输入参数  : addr,异常指令地址
 输出参数  : 无
 返 回 值  : BSP_OK/BSP_ERROR
*****************************************************************************/
s32 disable_single_step_ctrl(u32 addr)
{
    u32 i;

    if(g_wp_step_ctrl.addr != addr)
    {
        return BSP_ERROR;
    }

    /* 清除breakpoint寄存器 */
    write_wp_reg(ARM_BASE_BVR + g_wp_step_ctrl.bp_index, 0);
    write_wp_reg(ARM_BASE_BCR + g_wp_step_ctrl.bp_index, 0);

    /* 恢复watchpoint监控 */
    for(i=0; i<g_wp_ctrl.wp_cnt; i++)
    {
        if(g_wp_ctrl.wp_info[i].status_chg == true)
        {
            (void)bsp_wp_enable(i);
            g_wp_ctrl.wp_info[i].status_chg = false;
        }
    }

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : enable_single_step_ctrl
 功能描述  : 检测到watchpoint异常，利用breakpoint做单步控制
 输入参数  : addr,异常指令地址
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void enable_single_step_ctrl(u32 addr)
{
    u32 reg_value;
    u32 i;

    /* 去使能所有watchpoint */
    for(i=0; i<g_wp_ctrl.wp_cnt; i++)
    {
        if(g_wp_ctrl.wp_info[i].cfg.enable == WATCHPOINT_ENABLE)
        {
            (void)bsp_wp_disable(i);
            g_wp_ctrl.wp_info[i].status_chg = true;
        }
    }

    g_wp_step_ctrl.addr = addr;
    g_wp_step_ctrl.ctrl.len = WATCHPOINT_LEN_4;
    g_wp_step_ctrl.ctrl.type      = 0;
    g_wp_step_ctrl.ctrl.privilege = WATCHPOINT_PRI;
    g_wp_step_ctrl.ctrl.enabled   = WATCHPOINT_ENABLE;

    /* 使用breakpoint做单步使能 */
    reg_value = wp_encode_ctrl_reg(g_wp_step_ctrl.ctrl);
    write_wp_reg(ARM_BASE_BVR + g_wp_step_ctrl.bp_index, addr);
    write_wp_reg(ARM_BASE_BCR + g_wp_step_ctrl.bp_index, reg_value);
}

/*****************************************************************************
 函 数 名  : wp_callback
 功能描述  : 处理watchpoint触发的debug事件
 输入参数  : addr,异常指令地址；dscr,异常时刻dscr寄存器值；regs，异常时刻寄存器信息
 输出参数  : 无
 返 回 值  : BSP_OK，watchpoint模块完成异常处理；BSP_ERROR，返回由内核处理
*****************************************************************************/
s32 wp_callback(u32 addr, u32 dscr, void * regs)
{
    wp_print("debug event: 0x%x\n", addr);
	switch(DSCR_MOE(dscr))
    {
    	case ARM_ENTRY_BREAKPOINT:
    		return disable_single_step_ctrl((u32)(((struct pt_regs *)regs)->ARM_pc));
        
    	case ARM_ENTRY_SYNC_WATCHPOINT:
            enable_single_step_ctrl((u32)(((struct pt_regs *)regs)->ARM_pc) + 4);
    		break;

    	default:
    		return BSP_ERROR;
	}

    wp_print("********wachpoint error begin********\n");
    wp_show_regs(regs);
    show_stack(get_current(), NULL);
    wp_print("********wachpoint error end**********\n");

    g_wp_ctrl.wp_hit_cnt++;

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : wp_register
 功能描述  : watchpoint注册非正式接口，用于串口调用临时使用
 输入参数  : type, 读、写；start_addr, 起始地址；end_addr,结束地址，监控单个地址填0
 输出参数  : 无
 返 回 值  : 成功返回分配的watchpoint id，失败返回-1
*****************************************************************************/
s32 wp_register(u32 type, u32 start_addr, u32 end_addr)
{
    wp_cfg_t cfg;

    cfg.enable = WATCHPOINT_ENABLE;
    cfg.type = type;
    cfg.start_addr = start_addr;
    cfg.end_addr = end_addr;

    return bsp_wp_register(&cfg);
}

/*****************************************************************************
 函 数 名  : bsp_wp_register
 功能描述  : watchpoint注册接口，配置监控地址
 输入参数  : cfg，watchpoint配置信息
 输出参数  : 无
 返 回 值  : 成功返回分配的watchpoint id，失败返回-1
*****************************************************************************/
s32 bsp_wp_register(wp_cfg_t * cfg)
{
    s32 wp_id;
    watchpoint_ctrl ctrl;
    u32 ctrl_reg;

    WP_CHECK_INIT();

    if(NULL == cfg)
    {
        wp_print("input cfg is null\n");
        return BSP_ERROR;
    }

    if((cfg->start_addr & 0x3) != 0)
    {
        wp_print("start addr<0x%x> is not 4 bytes align\n", cfg->start_addr);
        return BSP_ERROR;
    }

    if((cfg->type & WATCHPOINT_RW) == 0)
    {
        wp_print("invalid acceess type<0x%x>\n", cfg->type);
        return BSP_ERROR;
    }

    if((cfg->start_addr > cfg->end_addr) && (cfg->end_addr != 0))
    {
        wp_print("end addr<0x%x> is smaller than start addr<0x%x>\n", cfg->end_addr, cfg->start_addr);
        return BSP_ERROR;
    }

    wp_id = wp_alloc_node();
    if(wp_id == BSP_ERROR)
    {
        wp_print("there's no watchpoint valid\n");
        return BSP_ERROR;
    }
    memcpy(&g_wp_ctrl.wp_info[wp_id].cfg, cfg, sizeof(wp_cfg_t));

    memset(&ctrl, 0, sizeof(watchpoint_ctrl));
    /* 监控地址写入WVR */
    write_wp_reg(ARM_BASE_WVR + wp_id, cfg->start_addr); 
    /* 监控start addr开始的4字节 */
    if((cfg->end_addr == 0) || (cfg->end_addr - cfg->start_addr < 4))
    {   
        ctrl.addr_range = 0;
    }
    /* 地址范围监控 */
    else
    {
        ctrl.addr_range = get_addr_range(cfg->start_addr, cfg->end_addr);
    }
    ctrl.len = WATCHPOINT_LEN_4;
    ctrl.privilege = WATCHPOINT_PRI;
    ctrl.type = cfg->type & WATCHPOINT_RW;
    ctrl.enabled = cfg->enable;
    ctrl_reg = wp_encode_ctrl_reg(ctrl);
    /* 监控配置写入WCR */
    write_wp_reg(ARM_BASE_WCR + wp_id, ctrl_reg);

    return wp_id;
}

/*****************************************************************************
 函 数 名  : bsp_wp_enable
 功能描述  : 使能watchpoint
 输入参数  : wp_id, bsp_wp_register获取到的watchpoint id
 输出参数  : 无
 返 回 值  : 成功返回0，失败返回-1
*****************************************************************************/
s32 bsp_wp_enable(s32 wp_id)
{
    u32 ctrl_reg;

    WP_CHECK_INIT();
    WP_CHECK_ID(wp_id);

    ctrl_reg = read_wp_reg(ARM_BASE_WCR + wp_id);
    /* WCR bit0:enable */
    ctrl_reg = ctrl_reg | (1 << 0);
    write_wp_reg(ARM_BASE_WCR + wp_id, ctrl_reg);
    g_wp_ctrl.wp_info[wp_id].cfg.enable = WATCHPOINT_ENABLE;

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : bsp_wp_disable
 功能描述  : 去使能watchpoint
 输入参数  : wp_id, bsp_wp_register获取到的watchpoint id
 输出参数  : 无
 返 回 值  : 成功返回0，失败返回-1
*****************************************************************************/
s32 bsp_wp_disable(s32 wp_id)
{
    u32 ctrl_reg;

    WP_CHECK_INIT();
    WP_CHECK_ID(wp_id);

    ctrl_reg = read_wp_reg(ARM_BASE_WCR + wp_id);
    /* WCR bit0:enable */
    ctrl_reg = ctrl_reg & ~(1 << 0);
    write_wp_reg(ARM_BASE_WCR + wp_id, ctrl_reg);
    g_wp_ctrl.wp_info[wp_id].cfg.enable = WATCHPOINT_DISABLE;

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : bsp_wp_unregister
 功能描述  : 释放watchpoint
 输入参数  : wp_id, bsp_wp_register获取到的watchpoint id
 输出参数  : 无
 返 回 值  : 成功返回0，失败返回-1
*****************************************************************************/
s32 bsp_wp_unregister(s32 wp_id)
{
    WP_CHECK_INIT();
    WP_CHECK_ID(wp_id);

    if(g_wp_ctrl.wp_info[wp_id].cfg.enable == WATCHPOINT_ENABLE)
    {
        (void)bsp_wp_disable(wp_id);
    }

    wp_free_node(wp_id);
    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : wp_probe
 功能描述  : watchpoint模块初始化
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 成功返回0，失败返回-1
*****************************************************************************/
int wp_probe(struct platform_device *dev)
{
    u32 wp_count;
    u32 bp_count;
    void * wp_cfg;
    u32 wp_size;
    u32 dscr;
    u32 i;
    DRV_WATCHPOINT_CFG_STRU cfg;

    if(g_wp_ctrl.init_flag == true)
    {
        return BSP_OK;
    }

    /* read from DBGDIDR */
    WP_DBG_READ(c0, 0, wp_count);
    bp_count = wp_count;
    /* DBGDIDR bit31~28, wrp count */
    wp_count = ((wp_count >> 28) & 0xF) + 1;
    /* DBGDIDR bit27~24, brp count */
    bp_count = ((bp_count >> 24) & 0xF) + 1;
    g_wp_ctrl.wp_cnt = wp_count;
    g_wp_step_ctrl.bp_index = bp_count - 1;
    
    wp_size = sizeof(wp_info_t) * wp_count;
    wp_cfg = osl_malloc(wp_size);
    if(NULL == wp_cfg)
    {
        wp_print("malloc fail\n");
        return BSP_ERROR;
    }
    memset(wp_cfg, 0, wp_size);
    g_wp_ctrl.wp_info = (wp_info_t *)wp_cfg;
    spin_lock_init(&g_wp_ctrl.lock);

    /* enable monitor mode, clear halting mode */
    WP_DBG_READ(c1, 0, dscr);
    dscr = dscr | ARM_DSCR_MDBGEN;
    dscr = dscr & ~(ARM_DSCR_HDBGEN);
    WP_DBG_WRITE(c2, 2, dscr);

    g_wp_ctrl.wp_hit_cnt = 0;
    g_wp_ctrl.init_flag  = true;

    /* 读取NV初始配置 */
    if(BSP_OK != bsp_nvm_read(NV_ID_DRV_WATCHPOINT, (u8 *)&cfg, sizeof(DRV_WATCHPOINT_CFG_STRU)))
    {
        wp_print("read nv 0x%x error\n", NV_ID_DRV_WATCHPOINT);
        return MDRV_ERROR;
    }

    for(i=0; i<4; i++)
    {
        if(i >= g_wp_ctrl.wp_cnt)
        {
            break;
        }

        if(cfg.ap_cfg[i].enable == WATCHPOINT_ENABLE)
        {
            g_wp_ctrl.wp_info[i].cfg.enable = WATCHPOINT_ENABLE;
            g_wp_ctrl.wp_info[i].cfg.type   = cfg.ap_cfg[i].type;
            g_wp_ctrl.wp_info[i].cfg.start_addr = cfg.ap_cfg[i].start_addr;
            g_wp_ctrl.wp_info[i].cfg.end_addr   = cfg.ap_cfg[i].end_addr;
            (void)bsp_wp_register(&(g_wp_ctrl.wp_info[i].cfg));
        }
    }

    wp_print("watchpoint init ok\n");
    return BSP_OK;
}

/* clear halting mode */
void wp_clr_hdbg(void)
{
    u32 dscr;

    WP_DBG_READ(c1, 0, dscr);
    dscr = dscr & ~(ARM_DSCR_HDBGEN);
    WP_DBG_WRITE(c2, 2, dscr);
}

int wp_suspend(struct device * dev)
{
    u32 i;

    wp_print("suspend enter\n");

    if(g_wp_ctrl.init_flag != true)
    {
        return BSP_OK;
    }

    for(i=0; i<g_wp_ctrl.wp_cnt; i++)
    {
        g_wp_backup.wvr_reg[i] = read_wp_reg(ARM_BASE_WVR + i);
        g_wp_backup.wcr_reg[i] = read_wp_reg(ARM_BASE_WCR + i);
    }

    WP_DBG_READ(c1, 0, g_wp_backup.dscr);

    return BSP_OK;
}

int wp_resume(struct device * dev)
{
    u32 i;

    wp_print("resume enter\n");
    if(g_wp_ctrl.init_flag != true)
    {
        return BSP_OK;
    }

    WP_DBG_WRITE(c2, 2, g_wp_backup.dscr);

    for(i=0; i<g_wp_ctrl.wp_cnt; i++)
    {
        write_wp_reg(ARM_BASE_WVR + i, g_wp_backup.wvr_reg[i]);
        write_wp_reg(ARM_BASE_WCR + i, g_wp_backup.wcr_reg[i]);
    }

    return BSP_OK;
}

static const struct dev_pm_ops wp_pm_ops ={
	.suspend_noirq = wp_suspend,
	.resume_noirq = wp_resume,
};

static struct platform_driver wp_driver = {
	.probe = wp_probe,
	.driver = {
		.name   = "watchpoint",
		.owner  = THIS_MODULE,
		.pm     = &wp_pm_ops,
	},
};

static struct platform_device wp_device =
{
    .name = "watchpoint",
    .id   = -1,
    .num_resources = 0,
};

s32 bsp_wp_init(void)
{
	if(BSP_OK != platform_device_register(&wp_device))
	{
        wp_print("watchpoint register device fail\n");
        return BSP_ERROR;
	}

    if(BSP_OK != platform_driver_register(&wp_driver))
    {
        wp_print("watchpoint register driver fail\n");
        platform_device_unregister(&wp_device);
        return BSP_ERROR;
    }

    return BSP_OK;
}

arch_initcall(bsp_wp_init);

void wp_show(void)
{
    u32 i;
    u32 wvr_reg;
    u32 wcr_reg;

    wp_print("**********wachpoint info**********\n");
    for(i=0; i<g_wp_ctrl.wp_cnt; i++)
    {
        wvr_reg = read_wp_reg(ARM_BASE_WVR + i);
        wcr_reg = read_wp_reg(ARM_BASE_WCR + i);
        wp_print("ID      : %u\n", i);
        wp_print("USAGE   : %s\n", (g_wp_ctrl.wp_info[i].alloc == WATCHPOINT_ALLOCED) ? "ALLOCED" : "UNALLOC");
        wp_print("STATUS  : %s\n", (g_wp_ctrl.wp_info[i].cfg.enable == WATCHPOINT_ENABLE) ? "ENABLE" : "DISABLE");
        wp_print("TYPE    : %s%s\n", (g_wp_ctrl.wp_info[i].cfg.type & WATCHPOINT_R) ? "<READ>" : "", (g_wp_ctrl.wp_info[i].cfg.type & WATCHPOINT_W) ? "<WRITE>" : "");
        wp_print("START   : 0x%x\n", g_wp_ctrl.wp_info[i].cfg.start_addr);
        wp_print("END     : 0x%x\n", g_wp_ctrl.wp_info[i].cfg.end_addr);
        wp_print("WVR     : 0x%x\n", wvr_reg);
        wp_print("WCR     : 0x%x\n", wcr_reg);
        wp_print("--------------------------\n");
    }
}

void wp_debug(void)
{
    u32 dscr;

    WP_DBG_READ(c1, 0, dscr);
    wp_print("INIT STATUS       : %s\n", (g_wp_ctrl.init_flag == true) ? "INITED" : "NOT INIT");
    wp_print("VALID WP COUNT    : %u\n", g_wp_ctrl.wp_cnt);
    wp_print("WP HIT COUNT      : %u\n", g_wp_ctrl.wp_hit_cnt);
    wp_print("BP STEP INDEX     : %u\n", g_wp_step_ctrl.bp_index);
    wp_print("DSCR REG          : 0x%x(%s %s)\n", dscr, (dscr & ARM_DSCR_MDBGEN) ? "MDBG EN" : "MDBG DIS", (dscr & ARM_DSCR_HDBGEN) ? "HDBG EN" : "HDBG DIS");
}

EXPORT_SYMBOL(bsp_wp_register);
EXPORT_SYMBOL(bsp_wp_unregister);
EXPORT_SYMBOL(bsp_wp_enable);
EXPORT_SYMBOL(bsp_wp_disable);
EXPORT_SYMBOL(wp_debug);
EXPORT_SYMBOL(wp_show);
EXPORT_SYMBOL(wp_register);
EXPORT_SYMBOL(read_wp_reg);
EXPORT_SYMBOL(wp_clr_hdbg);

