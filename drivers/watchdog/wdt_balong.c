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

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/watchdog.h>
#include <linux/platform_device.h>
#include <linux/hw_breakpoint.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <osl_types.h>
#include <hi_base.h>
#include <hi_wdt.h>
#include <hi_syssc_interface.h>
#include <hi_syscrg_interface.h>
#include <bsp_sram.h>
#include <bsp_ipc.h>
#include <bsp_wdt.h>

#ifndef CONFIG_HI3630_CCORE_WDT



unsigned int current_timeout = 0;
wdt_timeout_cb g_wdt_rebootfunc = WDT_NULL;
struct hi6930_wdt {
	 struct resource	*irq;
	 struct clk		*clock;
};

struct wdt_control{
	/*lint --e{958,959,43}*/
	struct hi6930_wdt *wdt;
	u32 wdt_size;
	wdt_timeout_cb wdt_ops[MAX_WDT_CORE_ID];
	struct watchdog_info info;
	struct watchdog_ops ops ;
	struct watchdog_device wdd;
	struct platform_device wpd;
	struct platform_driver plt_drv;
	spinlock_t wdt_lock;
	u32 wdt_m3_int_no;
    u32 wdt_mdm_int;
};

#ifdef CONFIG_OF
static const struct of_device_id hi6930_wdt_match[] = {
    { .compatible = "hisilicon,hi6930-wdt" },
    {},
};
MODULE_DEVICE_TABLE(of, hi6930_wdt_match);
#else
#define hi6930_wdt_match NULL
#endif

static struct wdt_control g_wdt_ctrl={
	.wdt_ops = {NULL},
};
struct wdt_info hi6930_wdt_ident;
static struct resource g_wdt_resource_init[] = {
    [0] = DEFINE_RES_IRQ(INT_LVL_TIMER3),
    [1] = DEFINE_RES_IRQ(INT_LVL_TIMER4),
};

/* functions */
static signed int hi6930_wdt_keepalive(struct watchdog_device *wdd)
{
	if(hi6930_wdt_ident.enable == BSP_FALSE)
	{
		return WDT_OK;
	}
	bsp_hardtimer_disable(ACORE_WDT_TIMER_ID);
	bsp_hardtimer_load_value(ACORE_WDT_TIMER_ID, hi6930_wdt_ident.my_timer.timeout);
	bsp_hardtimer_enable(ACORE_WDT_TIMER_ID);

    wdt_debug("exit wdt keepalive\n");
    return WDT_OK;
}

static int hi6930_wdt_stop(struct watchdog_device *wdd)
{
	s32 reg = 0;
	reg = bsp_hardtimer_disable(ACORE_WDT_TIMER_ID);
	hi6930_wdt_ident.enable = BSP_FALSE;
	/*A-M �˱�־for ������ͣ��*/
   STOP_WDT_TRACR_RUN_FLAG = 1;
	wdt_debug("exit wdt stop :%d\n", reg);
    return WDT_OK;
}

static int hi6930_wdt_start(struct watchdog_device *wdd)
{
	s32 reg = 0;
	DRV_WDT_INIT_PARA_STRU wdt_nv_param = {0};
	u32 retValue = 0;
    retValue = bsp_nvm_read(NV_ID_DRV_WDT_INIT_PARAM, (u8*)&wdt_nv_param, sizeof(DRV_WDT_INIT_PARA_STRU));
    if (NV_OK != retValue)
    {
    	wdt_err("read nv failed use default value\n");
		wdt_nv_param.wdt_enable = BSP_FALSE;
		wdt_nv_param.wdt_timeout = 30;
    }
	
    hi6930_wdt_ident.enable = (s32)wdt_nv_param.wdt_enable;
    hi6930_wdt_ident.my_timer.func = NULL;
    hi6930_wdt_ident.my_timer.mode = TIMER_PERIOD_COUNT;
    hi6930_wdt_ident.my_timer.timeout = WDT_HI_TIMER_CLK * wdt_nv_param.wdt_timeout;
    hi6930_wdt_ident.my_timer.timerId = ACORE_WDT_TIMER_ID;
    hi6930_wdt_ident.my_timer.unit = TIMER_UNIT_NONE;

    if (BSP_FALSE == hi6930_wdt_ident.enable)
	{
		wdt_err("wdt nv disable\n");
		return WDT_OK;
	}
    reg = bsp_hardtimer_start(&hi6930_wdt_ident.my_timer);
    if (reg)
    {
		wdt_err("wdt start hardtimer failed\n");
		return WDT_ERROR;
    }

    current_timeout = hi6930_wdt_ident.my_timer.timeout/WDT_HI_TIMER_CLK;

    /*A-M �˱�־for ������ͣ��*/
    STOP_WDT_TRACR_RUN_FLAG = 0;

    return WDT_OK;
}
/*���������λ����*/
static int hi6930_wdt_set_timeout(struct watchdog_device *wdd, u32 timeout)
{
	s32 reg;
	u32 value;
	value = timeout * WDT_HI_TIMER_CLK;
	hi6930_wdt_ident.my_timer.timeout = value;
	wdt_pinfo("wdt set timeout : %d\n", timeout);

	if(hi6930_wdt_ident.enable == BSP_FALSE)
	{
		wdt_err("wdt is disabled \n");
		return WDT_OK;
	}

	reg = bsp_hardtimer_start(&hi6930_wdt_ident.my_timer);
    if (reg)
    {
		wdt_err("wdt start hardtimer failed\n");
		return WDT_ERROR;
    }
	wdt_debug("exit wdt set timeout: %d\n", value);
	wdd->timeout = timeout;
	current_timeout = timeout;
	return WDT_OK;
}

static u32 hi6930_wdt_get_timeleft(struct watchdog_device *wdd)
{
	u32 reg = 0;
	u32 timeleft = 0;

	reg = bsp_get_timer_current_value(ACORE_WDT_TIMER_ID);

	timeleft = reg/WDT_HI_TIMER_CLK;
	wdt_pinfo("exit wdt get timeleft: %d\n", timeleft);

	return timeleft;
}
static irqreturn_t wdt_m3core_irq(int irq, void *dev_id)
{
    disable_irq_nosync(g_wdt_ctrl.wdt_m3_int_no);
    wdt_err("m3 core wdt irq\n");
    if(g_wdt_ctrl.wdt_ops[WDT_MCORE_ID])
    {
         g_wdt_ctrl.wdt_ops[WDT_MCORE_ID]();
    }
	return IRQ_HANDLED;
}

static irqreturn_t wdt_mdm_irq(int irq, void *dev_id)
{
    disable_irq_nosync(g_wdt_ctrl.wdt_mdm_int);
    wdt_err("MDM wdt irq\n");
    if(g_wdt_ctrl.wdt_ops[WDT_CCORE_ID])
    {
         g_wdt_ctrl.wdt_ops[WDT_CCORE_ID]();
    }
	return IRQ_HANDLED;
}

/* interrupt handler code */
static void  hi6930_wdt_acore_irq(u32 para)
{
	s32 dscr = 0;
	/*lint --e{718, 746 } */
	ARM_DBG_READ(c0, c1, 0, dscr);
	wdt_err("a\n");

	STOP_WDT_TRACR_RUN_FLAG = 0;

	/* Ensure that halting mode is disabled. */
	if (dscr & ARM_DSCR_HDBGEN) {

		STOP_WDT_TRACR_RUN_FLAG = 1;
	}
	else
	{
		 STOP_WDT_TRACR_RUN_FLAG = 0;
	}
	wdt_err("a:%d flag:%d\n", dscr & ARM_DSCR_HDBGEN, STOP_WDT_TRACR_RUN_FLAG);
	return ;
}

static int hi6930_acpu_timer_init(void)
{
	DRV_WDT_INIT_PARA_STRU wdt_nv_param = {0};
	s32 retValue = -1;
    retValue = (s32)bsp_nvm_read(NV_ID_DRV_WDT_INIT_PARAM, (u8*)&wdt_nv_param, sizeof(DRV_WDT_INIT_PARA_STRU));
    if (NV_OK != retValue)
    {
    	wdt_err("read nv failed use default value\n");
		wdt_nv_param.wdt_enable = 0;
		wdt_nv_param.wdt_timeout = 30;
		wdt_nv_param.wdt_suspend_timerout = 120;
    }

	hi6930_wdt_ident.lowtaskid = 0;
	hi6930_wdt_ident.enable = (s32)wdt_nv_param.wdt_enable;
	hi6930_wdt_ident.wdt_timeout = wdt_nv_param.wdt_timeout;

	hi6930_wdt_ident.my_timer.func = NULL;
    hi6930_wdt_ident.my_timer.mode = TIMER_PERIOD_COUNT;
    hi6930_wdt_ident.my_timer.timeout = WDT_HI_TIMER_CLK * wdt_nv_param.wdt_timeout;
    hi6930_wdt_ident.my_timer.timerId = ACORE_WDT_TIMER_ID;
    hi6930_wdt_ident.wdt_suspend_timeout = wdt_nv_param.wdt_suspend_timerout;
	
    wdt_pinfo("starting watchdog timer\n");
    hi6930_wdt_start(&g_wdt_ctrl.wdd);

    return WDT_OK;
}
static int __init hi6930_wdt_probe(struct platform_device *pdev)
{
    int ret = 0;
	struct device_node *node = NULL;
	g_wdt_ctrl.wdt_size = WDT_NUM;
	g_wdt_ctrl.wdt = kmalloc(sizeof(struct hi6930_wdt) * g_wdt_ctrl.wdt_size, GFP_KERNEL);

	if(NULL == g_wdt_ctrl.wdt)
	{
		wdt_err("no memory to malloc\n");
        return -ENOENT;
	}

	node = of_find_compatible_node(NULL, NULL, "hisilicon,watchdog_m3");
	if (!node)
	{
		wdt_err("wdt m3 node not found!\n");
		return ERROR;
	}
	g_wdt_ctrl.wdt_m3_int_no = (u32)irq_of_parse_and_map(node, 0);
	if(g_wdt_ctrl.wdt_m3_int_no == 0)
	{
		wdt_err("read wdt m3 irq fail\n");
		return ERROR;
	}

    node = of_find_compatible_node(NULL, NULL, "hisilicon,watchdog_mdm");
	if (!node)
	{
		wdt_err("wdt mdm node not found!\n");
		return ERROR;
	}
	g_wdt_ctrl.wdt_mdm_int = (u32)irq_of_parse_and_map(node, 0);
	if(g_wdt_ctrl.wdt_mdm_int == 0)
	{
		wdt_err("read wdt mdm irq fail\n");
		return ERROR;
	}

    g_wdt_ctrl.wdt[WDT_INDEX].irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if (g_wdt_ctrl.wdt[WDT_INDEX].irq == NULL) {
        wdt_err("no irq resource specified\n");
        ret = -ENOENT;
        goto err;
    }

	if (bsp_ipc_int_connect(IPC_ACPU_INT_SRC_MCPU_WDT,hi6930_wdt_acore_irq,0))
	{
		wdt_err("bsp_ipc_int_connect failed\n");
		goto err;
	}
	if (bsp_ipc_int_enable(IPC_ACPU_INT_SRC_MCPU_WDT))
	{
		wdt_err("bsp_ipc_int_enable failed\n");
		goto err;
	}
    watchdog_set_nowayout(&g_wdt_ctrl.wdd, WATCHDOG_NOWAYOUT);/*lint !e747 */

    ret = watchdog_register_device(&g_wdt_ctrl.wdd);
    if (ret) {
        wdt_err("cannot register watchdog (%d)\n", ret);
        goto err_irq;
    }

	hi6930_acpu_timer_init();

	ret = request_irq(g_wdt_ctrl.wdt_m3_int_no, wdt_m3core_irq, 0, "m3 wdt", NULL);
   	if (ret != 0) {
    	wdt_err("failed to install mcore irq (%d)\n", ret);
	}

    ret = request_irq(g_wdt_ctrl.wdt_mdm_int, wdt_mdm_irq, 0, "mdm wdt", NULL);
   	if (ret != 0) {
    	wdt_err("failed to install mdm irq (%d)\n", ret);
	}
    wdt_err("wdt init ok\n");
    return ret;

err_irq:

err:
    g_wdt_ctrl.wdt[WDT_INDEX].irq = NULL;
	kfree(g_wdt_ctrl.wdt);
    return ret;
}


static int __exit hi6930_wdt_remove(struct platform_device *dev)
{
    int ret = 0;
    watchdog_unregister_device(&g_wdt_ctrl.wdd);

    clk_disable(g_wdt_ctrl.wdt[WDT_INDEX].clock);
    clk_put(g_wdt_ctrl.wdt[WDT_INDEX].clock);
    g_wdt_ctrl.wdt[WDT_INDEX].clock = NULL;
    g_wdt_ctrl.wdt[WDT_INDEX].irq = NULL;

    wdt_err("exit wdt remove\n");
    return ret;
}

static void hi6930_wdt_shutdown(struct platform_device *dev)
{
    hi6930_wdt_stop(&g_wdt_ctrl.wdd);
}


#ifdef CONFIG_PM
static int balong_wdt_suspend(struct device *dev)
{
	int ret = 0;
	bsp_wdt_suspend(ret);
	return WDT_OK;
}
static int balong_wdt_resume(struct device *dev)
{
    bsp_wdt_resume();
	return WDT_OK;
}

static const struct dev_pm_ops balong_wdt_dev_pm_ops ={
    .suspend = NULL,
    .resume = balong_wdt_resume,
};

#define BALONG_DEV_PM_OPS (&balong_wdt_dev_pm_ops)
#else
#define BALONG_DEV_PM_OPS NULL
#endif

#define OPTIONS (WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE)
int __init hi6930_wdt_init(void)
{
#if (FEATURE_ON == MBB_DLOAD)
    
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        wdt_err("hi6930_wdt_init: smem_data is NULL \n");
        return WDT_ERROR;  

    }

    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        /*����ģʽ��WDEģ�鲻����*/
        wdt_err("entry update not init wdt !\n");
        return WDT_OK;  
    }
#endif /*MBB_DLOAD*/

    g_wdt_ctrl.info.options = OPTIONS;
    g_wdt_ctrl.info.firmware_version = 0;
    memcpy(g_wdt_ctrl.info.identity, "HI6930 Watchdog", sizeof("HI6930 Watchdog"));

	g_wdt_ctrl.ops.owner = THIS_MODULE;
    g_wdt_ctrl.ops.start = hi6930_wdt_start;
    g_wdt_ctrl.ops.stop = hi6930_wdt_stop;
    g_wdt_ctrl.ops.ping = hi6930_wdt_keepalive;
    g_wdt_ctrl.ops.set_timeout = hi6930_wdt_set_timeout;
    g_wdt_ctrl.ops.get_timeleft = hi6930_wdt_get_timeleft;

    g_wdt_ctrl.wdd.info = &g_wdt_ctrl.info;
    g_wdt_ctrl.wdd.ops = &g_wdt_ctrl.ops;

    g_wdt_ctrl.wpd.name = "hi6930-wdt";
    g_wdt_ctrl.wpd.id = -1;
    g_wdt_ctrl.wpd.num_resources = ARRAY_SIZE(g_wdt_resource_init);/*lint !e30 !e84 */
    g_wdt_ctrl.wpd.resource = g_wdt_resource_init;

    g_wdt_ctrl.plt_drv.probe = hi6930_wdt_probe;
    g_wdt_ctrl.plt_drv.remove = hi6930_wdt_remove;
    g_wdt_ctrl.plt_drv.shutdown = hi6930_wdt_shutdown;
    g_wdt_ctrl.plt_drv.driver.pm = BALONG_DEV_PM_OPS;
    g_wdt_ctrl.plt_drv.driver.owner = THIS_MODULE;
    g_wdt_ctrl.plt_drv.driver.name = "hi6930-wdt";
    g_wdt_ctrl.plt_drv.driver.of_match_table = hi6930_wdt_match;


    if (platform_device_register(&g_wdt_ctrl.wpd))
    {
		return WDT_ERROR;
    }

     if (platform_driver_register(&g_wdt_ctrl.plt_drv))
     {
		platform_device_unregister(&g_wdt_ctrl.wpd);
		return WDT_ERROR;
     }
     return WDT_OK;
}

static void __exit hi6930_wdt_exit(void)
{
    platform_driver_unregister(&g_wdt_ctrl.plt_drv);
    platform_device_unregister(&g_wdt_ctrl.wpd);
}

s32 bsp_wdt_start(void)
{
	hi6930_wdt_start(&g_wdt_ctrl.wdd);
	return WDT_OK;
}

s32 bsp_wdt_stop(void)
{
    hi6930_wdt_stop(&g_wdt_ctrl.wdd);
    return WDT_OK;
}

s32 bsp_wdt_keepalive(void)
{
    hi6930_wdt_keepalive(&g_wdt_ctrl.wdd);
    return WDT_OK;
}

s32 bsp_wdt_get_timeleft(u32 *timeleft)
{
	if(WDT_NULL == timeleft)
	{
		wdt_err("timeleft is null\n");
		return WDT_ERROR;
	}
    *timeleft = hi6930_wdt_get_timeleft(&g_wdt_ctrl.wdd);
    return WDT_OK;
}

/*ϵͳ����*/
void bsp_wdt_reboot_direct(void)
{
    //hi_syssc_wdt_reboot();
}

int bsp_wdt_register_hook(WDT_CORE_ID core_id, void *func)
{
    if(NULL == func)
    {
    	printk("hookNULL\n");
		return ERROR;
    }

	if(core_id >= MAX_WDT_CORE_ID)
	{
    	printk("pare err\n");
		return ERROR;
    }
	g_wdt_ctrl.wdt_ops[core_id] = func;

    return OK;
}

int bsp_wdt_unregister_hook(WDT_CORE_ID core_id)
{
	if(core_id >= MAX_WDT_CORE_ID)
	{
    	printk("pare err\n");
		return ERROR;
    }

	g_wdt_ctrl.wdt_ops[core_id] = NULL;
	return WDT_OK;
}

s32 bsp_wdt_reboot_register_hook(void *hook)
{
    if(WDT_NULL == hook)
    {
		 wdt_err("hook is NULL\n");
        return WDT_ERROR;
    }
	g_wdt_rebootfunc = hook;

	return WDT_OK;
}

s32  bsp_wdt_reboot_unregister_hook(void)
{
	g_wdt_rebootfunc = WDT_NULL;
	return WDT_OK;
}
s32 bsp_wdt_set_timeout(u32 timeout)
{
	s32 result;
	result = hi6930_wdt_set_timeout(&g_wdt_ctrl.wdd, timeout);
	return result;
}

void bsp_wdt_irq_disable(WDT_CORE_ID core_id)
{
    u32 id = core_id;

    switch(id)
    {
        case WDT_CCORE_ID:
             disable_irq(g_wdt_ctrl.wdt_mdm_int);
        break;

        case WDT_MCORE_ID:
             disable_irq(g_wdt_ctrl.wdt_m3_int_no);
        break;
    }
}

void bsp_wdt_irq_enable(WDT_CORE_ID core_id)
{
    u32 id = core_id;

    switch(id)
    {
        case WDT_CCORE_ID:
            enable_irq(g_wdt_ctrl.wdt_mdm_int);
        break;

        case WDT_MCORE_ID:
            enable_irq(g_wdt_ctrl.wdt_m3_int_no);
        break;
    }
}

s32 bsp_wdt_suspend(u32 timeout)
{
	u32 suspend_timeout = hi6930_wdt_ident.wdt_suspend_timeout;
	if (BSP_FALSE == hi6930_wdt_ident.enable)
	{
		return WDT_OK;
	}
	(void)bsp_wdt_set_timeout(suspend_timeout*10);
	return WDT_OK;
}

void bsp_wdt_resume(void)
{
	if (BSP_FALSE == hi6930_wdt_ident.enable)
	{
		wdt_pinfo("wdt nv disable\n");
		return;
	}
	(void)bsp_wdt_set_timeout(current_timeout);
}

module_init(hi6930_wdt_init);
module_exit(hi6930_wdt_exit);

EXPORT_SYMBOL(bsp_wdt_set_timeout);
EXPORT_SYMBOL(bsp_wdt_suspend);
EXPORT_SYMBOL(bsp_wdt_resume);
EXPORT_SYMBOL(bsp_wdt_start);
EXPORT_SYMBOL(bsp_wdt_stop);
EXPORT_SYMBOL(bsp_wdt_reboot_register_hook);
EXPORT_SYMBOL(bsp_wdt_reboot_unregister_hook);
EXPORT_SYMBOL(bsp_wdt_get_timeleft);
EXPORT_SYMBOL(bsp_wdt_keepalive);
EXPORT_SYMBOL(bsp_wdt_register_hook);
EXPORT_SYMBOL(bsp_wdt_unregister_hook);

/*lint --e{39 } */
MODULE_DESCRIPTION("HI6930 Watchdog Device Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
MODULE_ALIAS("platform:hi6930-wdt");


/*��ӡWDT ��Ϣ*/

void bsp_wdt_print_debug(void)
{
    unsigned int timeleft = 0;
    bsp_wdt_get_timeleft(&timeleft);
    wdt_err("*******************wdt debug  start*******************\n");
    wdt_err("timeleft: %d\n", timeleft);
    wdt_err("cur_timeout: %d\n", current_timeout);
    wdt_err("*******************wdt debug  end*******************\n");
}
#endif     /*end CONFIG_HI3630_CCORE_WDT*/

#ifdef __cplusplus
}
#endif
