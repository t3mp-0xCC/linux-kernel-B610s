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
/*lint --e{438,451,537,550,651,661,737} */
/* 438 -- Last value assigned to variable not used
 * 451 -- repeatedly included but does not have a standard include guard
 * 537 -- Repeated include file
 * 550 -- Symbol not accessed
 * 651 -- Potentially confusing initializer
 * 661 -- Possible access of out-of-bounds pointer
 * 737 -- Loss of sign in promotion from int to unsigned int
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

#include <linux/clk.h>
#include <linux/msi.h>
#include <linux/irq.h>
#include <linux/irqdesc.h>
#include <linux/interrupt.h>
#include <linux/syscore_ops.h>
#include <linux/regulator/consumer.h>

#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/pci_regs.h>
#include <asm/mach/pci.h>

#include <product_config.h>
#include <hi_pcie_interface.h>

#include "pci.h"
#include "pcie_balong.h"

#if defined(BSP_CONFIG_BOARD_E5)
/*make sure pcie pm suspend and resume called in pairs*/
int g_lock_pcie_suspend = 0;
int g_lock_pcie_resume = 0;
#ifdef FEATURE_HUAWEI_MBB_BCM4356
extern void wifi_reg_on_pull_up(void);
extern void wifi_reg_on_pull_down(void);
extern int g_wifi_power_state;
#endif
#endif

bool balong_pcie_pm_is_manageable(struct pci_dev *dev)
{
    return true;
}

int balong_pcie_pm_set_state(struct pci_dev *dev, pci_power_t state)
{
    struct balong_pcie_info *info = NULL;

    info = (struct balong_pcie_info *)((struct pci_sys_data *)dev->bus->sysdata)->private_data;
    if (PCI_D0 == state) {
        /* assert the apps_pm_xmt_pme */
        hi_pcie_set_apps_pm_xmt_pme(info->virt_rc_cfg_addr, info->id, 1);
        udelay(1);
        hi_pcie_set_apps_pm_xmt_pme(info->virt_rc_cfg_addr, info->id, 0);

        bsp_pcie_ltssm_enable(info->id);
        bsp_pcie_wait_for_linkup(info->id);

        if (info->linked) {
            bsp_pcie_try_change_speed(info->id);
        } else {
            pcie_trace("link up time out, controller: %u\n", info->id);
        }
    }

    if (PCI_D3cold == state) {
        bsp_pcie_ltssm_disable(info->id);

        /* broadcast the PME_Turn_Off Message by asserting the signal for one clock */
        hi_pcie_set_apps_pm_xmt_turnoff(info->virt_rc_cfg_addr, info->id, 1);

        /* wait for the PME_TO_Ack Message */
        while (hi_pcie_get_radm_pm_to_ack_reg(info->virt_rc_cfg_addr, info->id)) ;

        hi_pcie_set_apps_pm_xmt_turnoff(info->virt_rc_cfg_addr, info->id, 0);
    }

    return 0;
}

pci_power_t balong_pcie_pm_choose_state(struct pci_dev *dev)
{
    return PCI_D0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))
bool balong_pcie_pm_can_wakeup(struct pci_dev *dev)
{
    return false;
}
#endif

int balong_pcie_pm_sleep_wake(struct pci_dev *dev, bool enable)
{
    return 0;
}

int balong_pcie_pm_run_wake(struct pci_dev *dev, bool enable)
{
    return 0;
}

struct pci_platform_pm_ops balong_pcie_platform_pm_ops =
{
    .is_manageable = balong_pcie_pm_is_manageable,
    .set_state = balong_pcie_pm_set_state,
    .choose_state = balong_pcie_pm_choose_state,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0))
    .can_wakeup = balong_pcie_pm_can_wakeup,
#endif
    .sleep_wake = balong_pcie_pm_sleep_wake,
    .run_wake = balong_pcie_pm_run_wake,
};

int balong_pcie_pm_prepare(struct device *dev)
{
    /*lint --e{*} */
    int ret = 0;
    struct pci_dev *pci_dev = to_pci_dev(dev);

    ret = pci_save_state(pci_dev);

    return ret;
    /*lint -restore */
}

int balong_pcie_pm_suspend(struct device *dev)
{
    return 0;
}

int balong_pcie_pm_suspend_noirq(struct device *dev)
{
    return 0;
}

int balong_pcie_pm_resume_noirq(struct device *dev)
{
    /*lint --e{*} */
    struct pci_dev *pci_dev = to_pci_dev(dev);

    pci_restore_state(pci_dev);

    return 0;
    /*lint -restore */
}

int balong_pcie_pm_resume(struct device *dev)
{
    return 0;
}

void balong_pcie_pm_complete(struct device *dev)
{
    struct balong_pcie_info *info = NULL;

    info = (struct balong_pcie_info *)dev->platform_data;

    bsp_pcie_link_up_check(info->id);
    if (info->linked) {
        bsp_pcie_try_change_speed(info->id);
#if defined(CONFIG_BALONG_PCIE_L1SS)
        hi_pcie_l1ss_auto_gate_enable(info->virt_sc_addr, info->id);
#endif
    }

    enable_irq(info->irqs.int_link_down);

    return;
}

int balong_pcie_pm_sys_suspend(void)
{
    u32 id = 0;

    for (id = 0; id < balong_pcie_num; id++) {
        if ((!balong_pcie[id].enabled) || (balong_pcie[id].work_mode != PCIE_WORK_MODE_RC))
            continue;

        hi_pcie_l1ss_auto_gate_disable(balong_pcie[id].virt_sc_addr, id);

        /* broadcast the PME_Turn_Off Message by asserting the signal for one clock */
        hi_pcie_set_apps_pm_xmt_turnoff(balong_pcie[id].virt_sc_addr, id, 1);

        /* assert the app_ready_entr_123 */
        hi_pcie_set_app_ready_entr_l23(balong_pcie[id].virt_sc_addr, id, 1);

        /* wait for the PME_TO_Ack Message */
        /* while (!get_hi_sc_stat71_radm_pm_to_ack_reg()) ; */

        bsp_pcie_hardware_exit(id);
    }

    bsp_pcie_hardware_common_exit();
#if defined(BSP_CONFIG_BOARD_E5)
#ifdef FEATURE_HUAWEI_MBB_BCM4356
    if(!g_wifi_power_state)
    {
        wifi_reg_on_pull_down();
    }
#endif
#endif
    return 0;
}

void balong_pcie_pm_sys_resume(void)
{
    u32 id = 0;
  
    bsp_pcie_hardware_common_init();

    for (id = 0; id < balong_pcie_num; id++) {
        if ((!balong_pcie[id].enabled) || (balong_pcie[id].work_mode != PCIE_WORK_MODE_RC))
            continue;

        disable_irq(balong_pcie[id].irqs.int_link_down);

        /* stop to broadcast the PME_Turn_Off Message */
        hi_pcie_set_apps_pm_xmt_turnoff(balong_pcie[id].virt_sc_addr, id, 0);
        /* deassert the app_ready_entr_123 */
        hi_pcie_set_app_ready_entr_l23(balong_pcie[id].virt_sc_addr, id, 0);

        /* assert the apps_pm_xmt_pme */
        hi_pcie_set_apps_pm_xmt_pme(balong_pcie[id].virt_sc_addr, id, 1);
        udelay(1);
        hi_pcie_set_apps_pm_xmt_pme(balong_pcie[id].virt_sc_addr, id, 0);

        bsp_pcie_hardware_init(id);
#if defined(BSP_CONFIG_BOARD_E5)
#ifdef FEATURE_HUAWEI_MBB_BCM4356
        wifi_reg_on_pull_up();/*now only SB03 DC04 use*/
#endif
#endif
        bsp_pcie_wait_for_linkup(id);

        if (balong_pcie[id].linked) {
            bsp_pcie_try_change_speed(id);
        } else {
            pcie_trace("link up time out, controller: %u\n", id);
        }
    }
}

void balong_pcie_pm_sys_shutdown(void)
{
}

struct syscore_ops balong_pcie_pm_syscore_ops =
{
    .suspend = balong_pcie_pm_sys_suspend,
    .resume  = balong_pcie_pm_sys_resume,
    .shutdown= balong_pcie_pm_sys_shutdown,
};

int balong_pcie_pm_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    int ret = 0;

    /* ret = pci_set_platform_pm(&balong_pci_platform_pm_ops); */

    dev->dev.platform_data = (void *)((struct pci_sys_data *)dev->bus->sysdata)->private_data;

    return ret;
}

static DEFINE_PCI_DEVICE_TABLE(balong_pci_table) = {
    {PCI_DEVICE_CLASS(PCI_CLASS_BRIDGE_PCI<<8, ~0)},
    {0},
};

MODULE_DEVICE_TABLE(pci, balong_pci_table);

struct dev_pm_ops balong_pcie_dev_pm_ops = {
    .prepare    = balong_pcie_pm_prepare,
    .suspend    = balong_pcie_pm_suspend,
    .suspend_noirq  = balong_pcie_pm_suspend_noirq,
    .resume_noirq   = balong_pcie_pm_resume_noirq,
    .resume     = balong_pcie_pm_resume,
    .complete   = balong_pcie_pm_complete,
};

struct pci_driver balong_pcie_pci_driver =
{
    .name   = "pcie_balong",
    .id_table   = balong_pci_table,
    .probe  = balong_pcie_pm_probe,
    .remove = NULL,
    .err_handler = NULL,
    .driver = {
        .pm = &balong_pcie_dev_pm_ops,
    },
};

static int __init balong_pcie_pm_init(void)
{
    int ret = 0;

    register_syscore_ops(&balong_pcie_pm_syscore_ops);
#if 0
    ret = pci_register_driver(&balong_pcie_pci_driver);
#endif
    return ret;
}


subsys_initcall_sync(balong_pcie_pm_init);


