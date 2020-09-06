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
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

#include <linux/clk.h>
#include <linux/msi.h>
#include <linux/irq.h>
#include <linux/irqdesc.h>
#include <linux/irqdomain.h>
#include <linux/irqchip/chained_irq.h>

#include <linux/regulator/consumer.h>

#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/pci_regs.h>
#include <linux/pci-aspm.h>
#include <asm/mach/pci.h>

#include <product_config.h>
#include <hi_pcie_interface.h>

#include <bsp_sram.h>
#include <bsp_sysctrl.h>
#include <bsp_pcie.h>

#include "pci.h"
#include <bsp_onoff.h>
#include "pcie_balong.h"
#include <mdrv_sysboot.h>

static char* balong_pcie_info_gen_str[] = {"Gen1", "Gen2", "Gen3"};
static char* balong_pcie_info_phy_rate_str[] = {"2.5-Gbps", "5-Gbps", "8-Gbps"};

static struct pci_ops balong_pcie_ops = {
    .read = balong_pcie_read_config,
    .write = balong_pcie_write_config,
};

u32 balong_pcie_num = 0;
struct balong_pcie_info *balong_pcie = NULL;

static struct balong_pcie_iatu_table balong_pcie_iatu[] = {
    /* PCIE_TLP_DIRECTION_OUTBOUND */
    {
        .index.attr.index = 0,
        .index.attr.direction= PCIE_TLP_DIRECTION_OUTBOUND,
        .control1.attr.type  = PCIE_TLP_TYPE_CFG0_RW,
        .control2.attr.message_code = 0,
        .control2.attr.bar_index    = 0,
        .control2.attr.reserved     = 0,
        .control2.attr.cfg_shift    = 1,
        .control2.attr.invert_mode  = 0,
        .control2.attr.match_mode   = PCIE_IATU_MATCH_MODE_ADDR,
        .control2.attr.enable       = 1,
        .lower_addr = 0,    /* dynamic init */
        .upper_addr = 0,    /* dynamic init */
        .limit_addr = 0,    /* dynamic init */
        .lower_target_addr = 0,
        .upper_target_addr = 0,
        .control3 = 0,
    },
    {
        .index.attr.index = 1,
        .index.attr.direction = PCIE_TLP_DIRECTION_OUTBOUND,
        .control1.attr.type  = PCIE_TLP_TYPE_CFG1_RW,
        .control2.attr.message_code = 0,
        .control2.attr.bar_index    = 0,
        .control2.attr.reserved     = 0,
        .control2.attr.cfg_shift    = 1,
        .control2.attr.invert_mode  = 0,
        .control2.attr.match_mode   = PCIE_IATU_MATCH_MODE_ADDR,
        .control2.attr.enable       = 1,
        .lower_addr = 0,    /* dynamic init */
        .upper_addr = 0,    /* dynamic init */
        .limit_addr = 0,    /* dynamic init */
        .lower_target_addr = 0,
        .upper_target_addr = 0,
        .control3 = 0,
    },
    {
        .index.attr.index = 2,
        .index.attr.direction= PCIE_TLP_DIRECTION_OUTBOUND,
        .control1.attr.type = PCIE_TLP_TYPE_IO_RW,
        .control2.attr.message_code = 0,
        .control2.attr.bar_index    = 0,
        .control2.attr.reserved     = 0,
        .control2.attr.cfg_shift    = 0,
        .control2.attr.invert_mode  = 0,
        .control2.attr.match_mode   = PCIE_IATU_MATCH_MODE_ADDR,
        .control2.attr.enable       = 1,
        .lower_addr = 0,    /* dynamic init */
        .upper_addr = 0,    /* dynamic init */
        .limit_addr = 0,    /* dynamic init */
        .lower_target_addr = 0, /* dynamic init */
        .upper_target_addr = 0, /* dynamic init */
        .control3 = 0,
    },
    {
        .index.attr.index = 3,
        .index.attr.direction = PCIE_TLP_DIRECTION_OUTBOUND,
        .control1.attr.type = PCIE_TLP_TYPE_MEM_RW,
        .control2.attr.message_code = 0,
        .control2.attr.bar_index    = 0,
        .control2.attr.reserved     = 0,
        .control2.attr.cfg_shift    = 0,
        .control2.attr.invert_mode  = 0,
        .control2.attr.match_mode   = PCIE_IATU_MATCH_MODE_ADDR,
        .control2.attr.enable       = 1,
        .lower_addr = 0,    /* dynamic init */
        .upper_addr = 0,    /* dynamic init */
        .limit_addr = 0,    /* dynamic init */
        .lower_target_addr = 0, /* dynamic init */
        .upper_target_addr = 0, /* dynamic init */
        .control3 = 0,
    },

    /* PCIE_TLP_DIRECTION_INBOUND */
    {
        /* this entry is for MSI */
        .index.attr.index = 0,
        .index.attr.direction = PCIE_TLP_DIRECTION_INBOUND,
        .control1.attr.type = PCIE_TLP_TYPE_MEM_RW,
        .control2.attr.message_code = 0,
        .control2.attr.bar_index    = 0,
        .control2.attr.reserved     = 0,
        .control2.attr.cfg_shift    = 0,
        .control2.attr.invert_mode  = 0,
        .control2.attr.match_mode   = PCIE_IATU_MATCH_MODE_ADDR,
        .control2.attr.enable       = 1,
        .lower_addr = 0,    /* dynamic init */
        .upper_addr = 0,    /* dynamic init */
        .limit_addr = 0,    /* dynamic init */
        .lower_target_addr = 0,   /* the msi tlp will never arrive here */
        .upper_target_addr = 0,
        .control3 = 0,
    },
    {
        /* this entry is for ep's DMA */
        .index.attr.index = 1,
        .index.attr.direction = PCIE_TLP_DIRECTION_INBOUND,
        .control1.attr.type = PCIE_TLP_TYPE_MEM_RW,
        .control2.attr.message_code = 0,
        .control2.attr.bar_index    = 0,
        .control2.attr.reserved     = 0,
        .control2.attr.cfg_shift    = 0,
        .control2.attr.invert_mode  = 0,
        .control2.attr.match_mode   = PCIE_IATU_MATCH_MODE_ADDR,
        .control2.attr.enable    = 1,
        .lower_addr = 0,    /* dynamic init */
        .upper_addr = 0,    /* dynamic init */
        .limit_addr = 0,    /* dynamic init */
        .lower_target_addr = 0, /* dynamic init */
        .upper_target_addr = 0, /* dynamic init */
        .control3 = 0,
    },
};
static u32 balong_pcie_iatu_num = sizeof(balong_pcie_iatu)/sizeof(balong_pcie_iatu[0]);

#if defined(CONFIG_PCI_MSI)
static void balong_pcie_msi_isr(unsigned int irq, struct irq_desc *desc);
#endif

void dbi_enable(u32 id)
{
    hi_pcie_dbi_enable(balong_pcie[id].virt_sc_addr, id);
}

void dbi_disable(u32 id)
{
    hi_pcie_dbi_disable(balong_pcie[id].virt_sc_addr, id);
}

/* Attention: the dbi must be enabled when this function is called */
void pcie_set_iatu(u32 id, struct balong_pcie_iatu_table *iatu_table, u32 iatu_table_entry_num)
{
    u32 i = 0;

    for (i = 0; i < iatu_table_entry_num; i++) {
        writel(iatu_table->index.value,
            balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x200);
        writel(iatu_table->lower_addr,
            balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x20C);
        writel(iatu_table->upper_addr,
            balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x210);
        writel(iatu_table->limit_addr,
            balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x214);
        writel(iatu_table->lower_target_addr,
            balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x218);
        writel(iatu_table->upper_target_addr,
            balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x21C);
        writel(iatu_table->control1.value,
            balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x204);
        /* writel(iatu_table->control3,
            balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x220); */
        writel(iatu_table->control2.value,
            balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x208);

        iatu_table++;
    }
}

void pcie_app_req_clk(u32 id, u32 request)
{
#if defined(CONFIG_BALONG_PCIE_L1SS)
#if !defined(CONFIG_BALONG_PCIE_L1SS_AUTO)
    unsigned long irq_flags = 0;

    spin_lock_irqsave(&balong_pcie[id].spinlock_req_clk, irq_flags);

    if (request) {
        if (++balong_pcie[id].req_clk_count == 1) {
            hi_pcie_app_clk_req_n(balong_pcie[id].virt_sc_addr, id, request);
        }
    } else {
        if (--balong_pcie[id].req_clk_count == 0) {
            hi_pcie_app_clk_req_n(balong_pcie[id].virt_sc_addr, id, request);
        }
    }

    spin_unlock_irqrestore(&balong_pcie[id].spinlock_req_clk, irq_flags);
#endif
#endif
}

u32 pcie_convert_to_pcie_domain_address(struct pci_bus *bus, unsigned int devfn, int where)
{
    u32 pcie_address = 0;
    struct balong_pcie_info *info = (struct balong_pcie_info *)((struct pci_sys_data *)bus->sysdata)->private_data;

    pcie_address = (u32)info->virt_device_config_addr
        + PCIE_CFG_SHIFT_BUS((bus->number - info->busnr))
        + PCIE_CFG_SHIFT_DEV(PCI_SLOT(devfn))
        + PCIE_CFG_SHIFT_FUN(PCI_FUNC(devfn))
        + PCIE_CFG_SHIFT_REG(where);

    return pcie_address;
}

int pcie_read_config_from_dbi(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 *value)
{
    unsigned long irq_flags = 0;
    struct balong_pcie_info *info = (struct balong_pcie_info *)((struct pci_sys_data *)bus->sysdata)->private_data;

    if(devfn > 0)
        return -ENXIO;

    spin_lock_irqsave(&info->spinlock, irq_flags);

    pcie_app_req_clk(info->id, 1);

    dbi_enable(info->id);

    if (1 == size)
        *value = readb((u8*)(info->virt_rc_cfg_addr + (u32)where));
    else if (2 == size)
        *value = readw((u16*)(info->virt_rc_cfg_addr + (u32)where));
    else if (4 == size)
        *value = readl((u32*)(info->virt_rc_cfg_addr + (u32)where));

    dbi_disable(info->id);

    pcie_app_req_clk(info->id, 0);

    spin_unlock_irqrestore(&info->spinlock, irq_flags);

    return PCIBIOS_SUCCESSFUL;
}

int pcie_read_config_from_device(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 *value)
{
    u32 pcie_address = 0;
    struct balong_pcie_info *info = (struct balong_pcie_info *)((struct pci_sys_data *)bus->sysdata)->private_data;

    if (!info->linked) {
        pcie_trace("fail to read config from device, controller %d is link down\n", info->id);
        return -EIO;
    }

    pcie_app_req_clk(info->id, 1);

    pcie_address = pcie_convert_to_pcie_domain_address(bus, devfn, where);

    if (1 == size)
        *value = readb((u8*)pcie_address);
    else if (2 == size)
        *value = readw((u16*)pcie_address);
    else if (4 == size)
        *value = readl((u32*)pcie_address);

    pcie_app_req_clk(info->id, 0);

    return PCIBIOS_SUCCESSFUL;
}

int pcie_write_config_to_dbi(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 value)
{
    unsigned long irq_flags = 0;
    struct balong_pcie_info *info = (struct balong_pcie_info *)((struct pci_sys_data *)bus->sysdata)->private_data;

    if(devfn > 0)
        return -ENXIO;

    spin_lock_irqsave(&info->spinlock, irq_flags);

    pcie_app_req_clk(info->id, 1);

    dbi_enable(info->id);

    if (1 == size)
        writeb((u8)value, (u8*)(info->virt_rc_cfg_addr + (u32)where));
    else if (2 == size)
        writew((u16)value, (u16*)(info->virt_rc_cfg_addr + (u32)where));
    else if (4 == size)
        writel(value, (u32*)(info->virt_rc_cfg_addr + (u32)where));

    dbi_disable(info->id);

    pcie_app_req_clk(info->id, 0);

    spin_unlock_irqrestore(&info->spinlock, irq_flags);

    return PCIBIOS_SUCCESSFUL;
}

int pcie_write_config_to_device(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 value)
{
    u32 pcie_address = 0;
    struct balong_pcie_info *info = (struct balong_pcie_info *)((struct pci_sys_data *)bus->sysdata)->private_data;

    if (!info->linked) {
        pcie_trace("fail to write config from device, controller %d is link down\n", info->id);
        return -EIO;
    }

    pcie_app_req_clk(info->id, 1);

    pcie_address = pcie_convert_to_pcie_domain_address(bus, devfn, where);

    if (1 == size)
        writeb((u8)value, (u8*)pcie_address);
    else if (2 == size)
        writew((u16)value, (u16*)pcie_address);
    else if (4 == size)
        writel(value, (u32*)pcie_address);

    pcie_app_req_clk(info->id, 0);

    return PCIBIOS_SUCCESSFUL;
}

int balong_pcie_read_config(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 *value)
{
    int ret = 0;

    if (pci_is_root_bus(bus))
        ret = pcie_read_config_from_dbi(bus, devfn, where, size, value);
    else
        ret = pcie_read_config_from_device(bus, devfn, where, size, value);

    return ret;
}

int balong_pcie_write_config(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 value)
{
    int ret = 0;

    if (pci_is_root_bus(bus))
        ret = pcie_write_config_to_dbi(bus, devfn, where, size, value);
    else
        ret = pcie_write_config_to_device(bus, devfn, where, size, value);

    return ret;
}

int balong_pcie_setup(int nr, struct pci_sys_data *sys_data)
{
    int ret = 0;

    balong_pcie[nr].busnr = sys_data->busnr;

    sys_data->io_offset = 0;
    sys_data->mem_offset = 0;
    sys_data->private_data = (void*)&balong_pcie[nr];

    ret = request_resource(&ioport_resource, &balong_pcie[nr].res_io);
    if (ret) {
        pcie_trace("fail to request resource, controller: %d\n", nr);
        return ret;
    }

    ret = request_resource(&iomem_resource, &balong_pcie[nr].res_mem);
    if (ret) {
        release_resource(&balong_pcie[nr].res_io);
        pcie_trace("fail to request resource, controller: %d\n", nr);
        return ret;
    }

    /*lint -save -e* */
    pci_add_resource_offset(&sys_data->resources, &balong_pcie[nr].res_io, sys_data->io_offset);
    pci_add_resource_offset(&sys_data->resources, &balong_pcie[nr].res_mem, sys_data->mem_offset);
    /*lint -restore */

    return 1;
}

struct pci_bus* balong_pcie_scan(int nr, struct pci_sys_data *sys_data)
{
    struct pci_bus *bus;
    /*lint -save -e* */
    bus = pci_scan_root_bus(NULL, sys_data->busnr, &balong_pcie_ops, sys_data, &sys_data->resources);

    if (balong_pcie[nr].aspm_disable && bus)
        pcie_clear_aspm(bus);
    /*lint -restore */
    return bus;
}

/*lint -save -e958 */
int balong_pcie_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
    struct balong_pcie_info *info = (struct balong_pcie_info *)(((struct pci_sys_data *)(dev->bus->sysdata))->private_data);

    switch (pin) {
        case 1:
            return info->irqs.int_radm_a;
        case 2:
            return info->irqs.int_radm_b;
        case 3:
            return info->irqs.int_radm_c;
        case 4:
            return info->irqs.int_radm_d;
        default: {
            pcie_trace("Unkown pin %d for mapping irq!\n", pin);
            return -EINVAL;
        }
    }
}
/*lint -restore */

static irqreturn_t balong_pcie_linkdown_isr(int irq, void* device_info)
{
    struct balong_pcie_info *info = (struct balong_pcie_info *)device_info;

    hi_pcie_linkdown_interrupt_clear(info->virt_sc_addr, info->id);

    info->linked = 0;
    pcie_trace("link down interrupt, controller %d\n", info->id);

    return IRQ_HANDLED;
}

static irqreturn_t balong_pcie_dma_isr(int irq, void* device_info)
{
    u32 read_status = 0;
    u32 write_status = 0;
    u32 done_status = 0;
    u32 abort_status = 0;
    u32 channel_status = 0;
    u32 channel_index = 0;

    unsigned long irq_flags = 0;
    struct balong_pcie_info *info = (struct balong_pcie_info *)device_info;

    spin_lock_irqsave(&info->spinlock, irq_flags);
    dbi_enable(info->id);

    /* DMA read interrupt status */
    read_status = readl(info->virt_rc_cfg_addr+ 0x700 + 0x310);
    /* DMA read interrupt clear */
    writel(read_status, info->virt_rc_cfg_addr + 0x700 + 0x31C);
    /* DMA write interrupt status */
    write_status = readl(info->virt_rc_cfg_addr + 0x700 + 0x2BC);
    /* DMA write interrupt clear */
    writel(write_status, info->virt_rc_cfg_addr + 0x700 + 0x2C8);

    dbi_disable(info->id);
    spin_unlock_irqrestore(&info->spinlock, irq_flags);

    if (read_status & write_status) {
        pcie_trace("error interrupt status, read status: 0x%08X, write status: 0x%08X\n",
                    read_status, write_status);
        BUG();
    }

    if (read_status) {
        done_status = read_status & 0x0000FFFF;
        abort_status = (read_status >> 16) & 0x0000FFFF;
        channel_status = done_status | abort_status;

        for (channel_index = 0; channel_index < BALONG_PCIE_MAX_DMA_CHANNEL_NUM; channel_index++) {
            if (channel_status & (1U << channel_index)) {
                if (info->irqs.dma_int_callback[channel_index]) {
                    info->irqs.dma_int_callback[channel_index](PCIE_DMA_DIRECTION_READ, abort_status,
                        info->irqs.dma_int_callback_args[channel_index]);
                }
                info->irqs.dma_channel_state &= (~(1U << channel_index));
                pcie_app_req_clk(info->id, 0);
                up(&info->irqs.dma_semaphore[channel_index]);
            }
        }
    }

    if (write_status) {
        done_status = write_status & 0x0000FFFF;
        abort_status = (write_status >> 16) & 0x0000FFFF;
        channel_status = done_status | abort_status;

        for (channel_index = 0; channel_index < BALONG_PCIE_MAX_DMA_CHANNEL_NUM; channel_index++) {
            if (channel_status & (1U << channel_index)) {
                if (info->irqs.dma_int_callback[channel_index]) {
                    info->irqs.dma_int_callback[channel_index](PCIE_DMA_DIRECTION_WRITE, abort_status,
                        info->irqs.dma_int_callback_args[channel_index]);
                }
                info->irqs.dma_channel_state &= (~(1U << channel_index));
                pcie_app_req_clk(info->id, 0);
                up(&info->irqs.dma_semaphore[channel_index]);
            }
        }
    }

    return IRQ_HANDLED;
}

static irqreturn_t balong_pcie_pm_isr(int irq, void* device_info)
{
    struct balong_pcie_info *info = (struct balong_pcie_info *)device_info;

    pcie_trace("pm interrupt, controller %d\n", info->id);

    return IRQ_HANDLED;
}

/* assert a global reset through PCIE_PERST singnal */
#if(FEATURE_ON == MBB_COMMON)
#define GPIO_NAME_LEN    (32)
#endif
void bsp_pcie_assert_perst(u32 id)
{
    int ret = 0;
#if(FEATURE_ON == MBB_COMMON)
    char buf1[GPIO_NAME_LEN]= {0};
    memset(buf1, '\0', GPIO_NAME_LEN);
#endif
    if (balong_pcie[id].work_mode != PCIE_WORK_MODE_RC)
        return;

    if (balong_pcie[id].gpio_perst) {
#if(FEATURE_ON == MBB_COMMON)
        snprintf(buf1, GPIO_NAME_LEN, "%s%d","pcie-perst", id);
        pcie_trace("bsp_pcie_assert_perst:%s\n", buf1);
        ret = gpio_request(balong_pcie[id].gpio_perst, buf1);
#else
        ret = gpio_request(balong_pcie[id].gpio_perst, "pcie-perst");
#endif
        if (ret) {
            pcie_trace("fail to request for GPIO:%u\n", balong_pcie[id].gpio_perst);
            return;
        }

#if defined(BSP_CONFIG_BOARD_CPE)
        ret = gpio_direction_output(balong_pcie[id].gpio_perst, 1);
#else
        ret = gpio_direction_output(balong_pcie[id].gpio_perst, 0);
#endif
        if (ret) {
            pcie_trace("fail to set GPIO:%u\n", balong_pcie[id].gpio_perst);
            goto error_set_gpio;
        }

error_set_gpio:
        gpio_free(balong_pcie[id].gpio_perst);
    } else {
        hi_pcie_assert_perst(balong_pcie[id].virt_sc_addr, id);
    }

    return;
}

void bsp_pcie_deassert_perst(u32 id)
{
    int ret = 0;
#if(FEATURE_ON == MBB_COMMON)
    char buf[GPIO_NAME_LEN] = {0};
    memset(buf, '\0', GPIO_NAME_LEN);
#endif
    if (balong_pcie[id].work_mode != PCIE_WORK_MODE_RC)
        return;

    /* stay in low for 100ms */
    mdelay(100);

    if (balong_pcie[id].gpio_perst) {
#if(FEATURE_ON == MBB_COMMON)
        snprintf(buf, GPIO_NAME_LEN - 1, "%s%d","pcie-perst", id);
        pcie_trace("bsp_pcie_deassert_perst:%s\n", buf);
        ret = gpio_request(balong_pcie[id].gpio_perst, buf);
#else
        ret = gpio_request(balong_pcie[id].gpio_perst, "pcie-perst");
#endif
        if (ret) {
            pcie_trace("fail to request for GPIO:%u\n", balong_pcie[id].gpio_perst);
            return;
        }
#if defined(BSP_CONFIG_BOARD_CPE)
        ret = gpio_direction_output(balong_pcie[id].gpio_perst, 0);
#else
        ret = gpio_direction_output(balong_pcie[id].gpio_perst, 1);
#endif
        if (ret) {
            pcie_trace("fail to set GPIO:%u\n", balong_pcie[id].gpio_perst);
            goto error_set_gpio;
        }

error_set_gpio:
        gpio_free(balong_pcie[id].gpio_perst);
    } else {
        hi_pcie_deassert_perst(balong_pcie[id].virt_sc_addr, id);
    }

    return;
}

#if defined(CONFIG_PCI_MSI)

static void balong_pcie_msi_isr(unsigned int irq, struct irq_desc *desc)
{
    u32 bit = 0;
    u32 index = 0;
    u32 status = 0;
    unsigned long irq_flags = 0;

    struct balong_pcie_info *info = NULL;
    struct irq_chip *irqchip = irq_desc_get_chip(desc);

    info = (struct balong_pcie_info *)irq_desc_get_handler_data(desc);

    chained_irq_enter(irqchip, desc);

    for(index = 0; index < 8; index++) {
        spin_lock_irqsave(&info->spinlock, irq_flags);
        pcie_app_req_clk(info->id, 1);
        dbi_enable(info->id);
        status = readl(info->virt_rc_cfg_addr + 0x700 + 0x130 + index*0x0C);
        writel(status, info->virt_rc_cfg_addr + 0x700 + 0x130 + index*0x0C);
        dbi_disable(info->id);
        pcie_app_req_clk(info->id, 0);
        spin_unlock_irqrestore(&info->spinlock, irq_flags);

        if (status) {
            for_each_set_bit(bit, (const void *)&status, 32)
                generic_handle_irq(irq_find_mapping(info->irq_domain, index * 32 + bit));
        }
    }

    chained_irq_exit(irqchip, desc);
}


static void msi_ack(struct irq_data *data)
{
    /* clear interrupt by user isr,not here. */
}

static void msi_mask(struct irq_data *data)
{
    u32 value = 0;
    u32 index = 0;
    u32 status = 0;
    unsigned long irq_flags = 0;

    struct balong_pcie_info *info = NULL;

    info = (struct balong_pcie_info *)irq_data_get_irq_chip_data(data);

    index = data->hwirq / 32;
    status = 1 << (data->hwirq % 32);

    spin_lock_irqsave(&info->spinlock, irq_flags);
    pcie_app_req_clk(info->id, 1);
    dbi_enable(info->id);
    value = readl(info->virt_rc_cfg_addr + 0x700 + 0x12C + index*0x0C);
    value |= status;
    writel(value, info->virt_rc_cfg_addr + 0x700 + 0x12C + index*0x0C);
    dbi_disable(info->id);
    pcie_app_req_clk(info->id, 0);
    spin_unlock_irqrestore(&info->spinlock, irq_flags);
}

static void msi_unmask(struct irq_data *data)
{
    u32 value = 0;
    u32 index = 0;
    u32 status = 0;
    unsigned long irq_flags = 0;

    struct balong_pcie_info *info = NULL;

    info = (struct balong_pcie_info *)irq_data_get_irq_chip_data(data);

    index = data->hwirq / 32;
    status = 1 << (data->hwirq % 32);

    spin_lock_irqsave(&info->spinlock, irq_flags);
    pcie_app_req_clk(info->id, 1);
    dbi_enable(info->id);
    value = readl(info->virt_rc_cfg_addr + 0x700 + 0x12C + index*0x0C);
    value &= ~status;
    writel(value, info->virt_rc_cfg_addr + 0x700 + 0x12C + index*0x0C);
    dbi_disable(info->id);
    pcie_app_req_clk(info->id, 0);
    spin_unlock_irqrestore(&info->spinlock, irq_flags);
}

static void msi_enable(struct irq_data *data)
{
    u32 value = 0;
    u32 index = 0;
    u32 status = 0;
    unsigned long irq_flags = 0;

    struct balong_pcie_info *info = NULL;

    info = (struct balong_pcie_info *)irq_data_get_irq_chip_data(data);

    index = data->hwirq / 32;
    status = 1 << (data->hwirq % 32);

    spin_lock_irqsave(&info->spinlock, irq_flags);
    pcie_app_req_clk(info->id, 1);
    dbi_enable(info->id);
    value = readl(info->virt_rc_cfg_addr + 0x700 + 0x128 + index*0x0C);
    value |= status;
    writel(value, info->virt_rc_cfg_addr + 0x700 + 0x128 + index*0x0C);
    dbi_disable(info->id);
    pcie_app_req_clk(info->id, 0);
    spin_unlock_irqrestore(&info->spinlock, irq_flags);
}

static void msi_disable(struct irq_data *data)
{
    u32 value = 0;
    u32 index = 0;
    u32 status = 0;
    unsigned long irq_flags = 0;

    struct balong_pcie_info *info = NULL;

    info = (struct balong_pcie_info *)irq_data_get_irq_chip_data(data);

    index = data->hwirq / 32;
    status = 1 << (data->hwirq % 32);

    spin_lock_irqsave(&info->spinlock, irq_flags);
    pcie_app_req_clk(info->id, 1);
    dbi_enable(info->id);
    value = readl(info->virt_rc_cfg_addr + 0x700 + 0x128 + index*0x0C);
    value &= ~status;
    writel(value, info->virt_rc_cfg_addr + 0x700 + 0x128 + index*0x0C);
    dbi_disable(info->id);
    pcie_app_req_clk(info->id, 0);
    spin_unlock_irqrestore(&info->spinlock, irq_flags);
}

/**
 * get_free_msi() - Get a free MSI number
 *
 * Checks for availability of MSI and returns the first available.
 */
static unsigned long get_free_msi(struct balong_pcie_info *info)
{
    unsigned long offset = 0;

    do {
        offset = find_first_zero_bit(info->msi_irq_in_use, BALONG_MAX_MSI_NUM);
        if (offset >= BALONG_MAX_MSI_NUM)
            return offset;
    } while (test_and_set_bit(offset, info->msi_irq_in_use));

    return offset;
}

/*
 * Note: mask/unmask on remote devices is NOT supported (mask_msi_irq and
 * unmask_msi_irq through mask bits capability on endpoints.
 */

static struct irq_chip balong_msi_chip = {
    .name = "PCIe-MSI",
    .irq_enable = msi_enable,
    .irq_disable = msi_disable,
    .irq_ack = msi_ack,
    .irq_mask = msi_mask,
    .irq_unmask = msi_unmask,
};

#if 0
int arch_setup_msi_irqs(struct pci_dev *dev, int nvec, int type)
{
    return 0;
}
#endif

int arch_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc)
{
    int ret = 0;
    unsigned int irq;
    unsigned long offset;

    struct msi_msg msg;
    struct balong_pcie_info *info;

    if ((!pdev) || (!pdev->bus) || (!pdev->bus->sysdata) ||
        (!((struct pci_sys_data *)pdev->bus->sysdata)->private_data)) {
        pcie_trace("error args\n");
        return -1;
    }

    info = (struct balong_pcie_info *)((struct pci_sys_data *)pdev->bus->sysdata)->private_data;

    offset = get_free_msi(info);
    if (offset >= BALONG_MAX_MSI_NUM) {
        pcie_trace("fail to get free MSI\n");
        return -1;
    } else {
        irq = irq_create_mapping(info->irq_domain, offset);

        ret = irq_set_msi_desc(irq, desc);
        if (!ret) {
            msg.address_lo = BALONG_PCIE_MSI_PCI_LOWER_ADDR(info->phys_pcie_cfg);
            msg.address_hi = BALONG_PCIE_MSI_PCI_UPPER_ADDR(info->phys_pcie_cfg);

            msg.data = offset;

            /* pcie_trace("MSI %d, irq = %d\n", msg.data, irq); */

            write_msi_msg(irq, &msg);
        }
    }

    return ret;
}

void arch_teardown_msi_irq(unsigned int irq)
{
    struct irq_data *data = NULL;
    struct balong_pcie_info *info = NULL;

    data = irq_get_irq_data(irq);
    info = (struct balong_pcie_info *)irq_get_chip_data(irq);

    irq_dispose_mapping(irq);
    clear_bit(data->hwirq, info->msi_irq_in_use);
}

static int pcie_irq_domain_map(struct irq_domain *d, unsigned int virq, irq_hw_number_t hw)
{
    irq_set_chip_and_handler_name(virq, &balong_msi_chip, handle_simple_irq,
                      "PCIe");
    irq_set_chip_data(virq, d->host_data);
    set_irq_flags(virq, IRQF_VALID);

    return 0;
}

static const struct irq_domain_ops pcie_balong_irq_domain_ops = {
    .map    = pcie_irq_domain_map,
    .xlate  = irq_domain_xlate_onecell,
};
#endif


void bsp_pcie_software_common_init(void)
{
    int i = 0;
    int id = 0;
    int ret = 0;

    struct device_node *dev_node = NULL, *child_node = NULL;
#if defined(CONFIG_PCIE_CFG)
    struct pcie_cfg *pcie_cfg_table = (struct pcie_cfg *)SRAM_PCIE_BALONG_ADDR;
#endif

    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,pcie_balong");
    if (dev_node) {
        balong_pcie_num = of_get_child_count(dev_node);
        if (!balong_pcie_num) {
            pcie_trace("no controller found\n");
            return;
        }
        balong_pcie = (struct balong_pcie_info*)kzalloc(sizeof(struct balong_pcie_info)*balong_pcie_num, GFP_KERNEL);
        if (!balong_pcie) {
            panic("kzalloc fail, no memory to init balong pcie, size: 0x%08X\n", sizeof(struct balong_pcie_info)*balong_pcie_num);
        }

        for_each_child_of_node(dev_node, child_node) {
            if (of_device_is_available(child_node))
                balong_pcie[id].enabled = PCIE_ENABLE;

            balong_pcie[id].id = id;
            balong_pcie[id].dev_node  = child_node;
            balong_pcie[id].work_mode = PCIE_WORK_MODE_RC;
            balong_pcie[id].link_mode = PCIE_LINK_MODE_X1;
            balong_pcie[id].clock_mode = PCIE_CLOCK_MODE_DEFAULT;
            balong_pcie[id].speed_mode = CONFIG_BALONG_PCIE_SPEED_MODE;

#if !defined(CONFIG_PCIEPORTBUS)
            balong_pcie[id].port_mode = PCIE_PORT_MODE_AUTO;
#else
            balong_pcie[id].port_mode = PCIE_PORT_MODE_NATIVE;
#endif

#if !defined(CONFIG_PCI_MSI)
            balong_pcie[id].msi_disable = PCIE_ENABLE;
#else
            balong_pcie[id].msi_disable = PCIE_DISABLE;
#endif

#if !defined(CONFIG_PCIEASPM)
            balong_pcie[id].aspm_disable = PCIE_ENABLE;
#else
            balong_pcie[id].aspm_disable = PCIE_DISABLE;
#endif

#if !defined(CONFIG_BALONG_PCIE_L1SS)
            balong_pcie[id].l1ss_disable = PCIE_ENABLE;
#else
            balong_pcie[id].l1ss_disable = PCIE_DISABLE;
#endif

#if !defined(CONFIG_BALONG_PCIE_OUTPUT_CLK)
            balong_pcie[id].output_clock_disable    = PCIE_ENABLE;
#else
            balong_pcie[id].output_clock_disable    = PCIE_DISABLE;
#endif
            balong_pcie[id].compliance_test_enable  = PCIE_DISABLE;

#if defined(CONFIG_PCIE_CFG)
            if (pcie_cfg_table[id].valid == PCIE_CFG_VALID) {
                balong_pcie[id].enabled   = pcie_cfg_table[id].enabled;
                balong_pcie[id].work_mode = pcie_cfg_table[id].work_mode;
                balong_pcie[id].port_mode = pcie_cfg_table[id].port_mode;
                balong_pcie[id].clock_mode = pcie_cfg_table[id].clock_mode;
                balong_pcie[id].speed_mode = pcie_cfg_table[id].speed_mode ? pcie_cfg_table[id].speed_mode : balong_pcie[id].speed_mode;
                balong_pcie[id].msi_disable  = pcie_cfg_table[id].msi_disable;
                balong_pcie[id].aspm_disable = pcie_cfg_table[id].aspm_disable;
                balong_pcie[id].l1ss_disable = pcie_cfg_table[id].l1ss_disable;
                balong_pcie[id].output_clock_disable    = pcie_cfg_table[id].output_clock_disable;
                balong_pcie[id].compliance_test_enable  = pcie_cfg_table[id].compliance_test_enable;
            }
#endif

#if defined(CONFIG_PCIEPORTBUS)
            if (balong_pcie[id].port_mode == PCIE_PORT_MODE_AUTO) {
                pcie_ports_disabled = false;
                pcie_ports_auto = true;
            } else if (balong_pcie[id].port_mode == PCIE_PORT_MODE_NATIVE) {
                pcie_ports_disabled = false;
                pcie_ports_auto = false;
            } else {
                pcie_ports_disabled = true;
            }
#endif
            if (balong_pcie[id].msi_disable)
                pci_no_msi();

            ret |= of_property_read_u32_index(dev_node, "vendor_id", 0, &balong_pcie[id].vendor_id);
            ret |= of_property_read_u32_index(dev_node, "device_id", 0, &balong_pcie[id].device_id);

            ret |= of_property_read_u32_index(dev_node, "sc_addr", 0, &balong_pcie[id].phys_sc_addr);
            ret |= of_property_read_u32_index(dev_node, "sc_addr", 1, &balong_pcie[id].phys_sc_size);

            ret |= of_property_read_string_index(dev_node, "clock_sc_id", 0, (const char**)&balong_pcie[id].clock_sc_id);

            ret |= of_property_read_u32_index(child_node, "pcie_cfg", 0, &balong_pcie[id].phys_pcie_cfg);
            ret |= of_property_read_u32_index(child_node, "pcie_data", 0, &balong_pcie[id].phys_pcie_data);
            ret |= of_property_read_u32_index(child_node, "device_cfg_size", 0, &balong_pcie[id].phys_device_config_size);
            ret |= of_property_read_u32_index(child_node, "device_io_size", 0, &balong_pcie[id].phys_device_io_size);
            ret |= of_property_read_u32_index(child_node, "device_mem_size", 0, &balong_pcie[id].phys_device_mem_size);

            ret |= of_property_read_u32_index(child_node, "int_link_down", 0, &balong_pcie[id].irqs.int_link_down);
            ret |= of_property_read_u32_index(child_node, "int_dma", 0, &balong_pcie[id].irqs.int_edma);
            ret |= of_property_read_u32_index(child_node, "int_pm", 0, &balong_pcie[id].irqs.int_pm);
            ret |= of_property_read_u32_index(child_node, "int_radm_a", 0, &balong_pcie[id].irqs.int_radm_a);
            ret |= of_property_read_u32_index(child_node, "int_radm_b", 0, &balong_pcie[id].irqs.int_radm_b);
            ret |= of_property_read_u32_index(child_node, "int_radm_c", 0, &balong_pcie[id].irqs.int_radm_c);
            ret |= of_property_read_u32_index(child_node, "int_radm_d", 0, &balong_pcie[id].irqs.int_radm_d);
            ret |= of_property_read_u32_index(child_node, "int_msi", 0, &balong_pcie[id].irqs.int_msi);

            ret |= of_property_read_u32_index(child_node, "gpio_perst", 0, &balong_pcie[id].gpio_perst);

            ret |= of_property_read_string_index(child_node, "regulator_id", 0, (const char**)&balong_pcie[id].regulator_id);
            ret |= of_property_read_string_index(child_node, "clock_core_id", 0, (const char**)&balong_pcie[id].clock_core_id);
            ret |= of_property_read_string_index(child_node, "clock_aux_id", 0, (const char**)&balong_pcie[id].clock_aux_id);
            ret |= of_property_read_string_index(child_node, "clock_phy_id", 0, (const char**)&balong_pcie[id].clock_phy_id);

            ret |= of_property_read_u32_index(child_node, "phy_assert_reset", 0, &balong_pcie[id].phy_assert_reset_addr);
            ret |= of_property_read_u32_index(child_node, "phy_assert_reset", 1, &balong_pcie[id].phy_assert_reset_offset);
            ret |= of_property_read_u32_index(child_node, "phy_assert_reset", 2, &balong_pcie[id].phy_assert_reset_value);
            ret |= of_property_read_u32_index(child_node, "phy_deassert_reset", 0, &balong_pcie[id].phy_deassert_reset_addr);
            ret |= of_property_read_u32_index(child_node, "phy_deassert_reset", 1, &balong_pcie[id].phy_deassert_reset_offset);
            ret |= of_property_read_u32_index(child_node, "phy_deassert_reset", 2, &balong_pcie[id].phy_deassert_reset_value);

            ret |= of_property_read_u32_index(child_node, "core_assert_reset", 0, &balong_pcie[id].core_assert_reset_addr);
            ret |= of_property_read_u32_index(child_node, "core_assert_reset", 1, &balong_pcie[id].core_assert_reset_offset);
            ret |= of_property_read_u32_index(child_node, "core_assert_reset", 2, &balong_pcie[id].core_assert_reset_value);
            ret |= of_property_read_u32_index(child_node, "core_deassert_reset", 0, &balong_pcie[id].core_deassert_reset_addr);
            ret |= of_property_read_u32_index(child_node, "core_deassert_reset", 1, &balong_pcie[id].core_deassert_reset_offset);
            ret |= of_property_read_u32_index(child_node, "core_deassert_reset", 2, &balong_pcie[id].core_deassert_reset_value);

            if (ret) {
                pcie_trace("fail to get dts info\n");
            }

            balong_pcie[id].phys_rc_cfg_addr = balong_pcie[id].phys_pcie_cfg;
            balong_pcie[id].phys_rc_cfg_size = SZ_64K;
            balong_pcie[id].phys_device_config_addr = balong_pcie[id].phys_pcie_data;

            balong_pcie[id].res_io.name = "PCIe I/O Space";
            balong_pcie[id].res_io.start= balong_pcie[id].phys_device_config_addr + balong_pcie[id].phys_device_config_size;
            balong_pcie[id].res_io.end  = balong_pcie[id].res_io.start + balong_pcie[id].phys_device_io_size - 1;
            balong_pcie[id].res_io.flags= IORESOURCE_IO;

            balong_pcie[id].res_mem.name = "PCIe Mem Space";
            balong_pcie[id].res_mem.start= balong_pcie[id].res_io.end + 1;
            balong_pcie[id].res_mem.end  = balong_pcie[id].res_mem.start + balong_pcie[id].phys_device_mem_size - 1;
            balong_pcie[id].res_mem.flags= IORESOURCE_MEM;

            balong_pcie[id].res_cfg0.name   = "PCIe CFG0 Space";
            balong_pcie[id].res_cfg0.start  = balong_pcie[id].phys_device_config_addr;
            balong_pcie[id].res_cfg0.end    = balong_pcie[id].res_cfg0.start + SZ_2M - 1;
            balong_pcie[id].res_cfg0.flags  = IORESOURCE_MEM;
            balong_pcie[id].res_cfg1.name   = "PCIe CFG1 Space";
            balong_pcie[id].res_cfg1.start  = balong_pcie[id].res_cfg0.end + 1;
            balong_pcie[id].res_cfg1.end    = balong_pcie[id].res_cfg1.start + balong_pcie[id].phys_device_config_size - SZ_2M - 1;
            balong_pcie[id].res_cfg1.flags  = IORESOURCE_MEM;

            id++;
        }
    }

    for (id = 0; id < balong_pcie_num; id++) {
        for (i = 0; (id > 0) && (i < id); i++) {
            if (!strncmp(balong_pcie[i].regulator_id, balong_pcie[id].regulator_id,
                strnlen(balong_pcie[i].regulator_id, 255U))) {
                balong_pcie[id].regulator = balong_pcie[i].regulator;
                break;
            }
        }
        for (i = 0; (id > 0) && (i < id); i++) {
            if (balong_pcie[i].phys_sc_addr == balong_pcie[id].phys_sc_addr) {
                balong_pcie[id].virt_sc_addr = balong_pcie[i].virt_sc_addr;
                break;
            }
        }

        if (!balong_pcie[id].regulator) {
            balong_pcie[id].regulator = regulator_get(NULL, balong_pcie[id].regulator_id);
            if (IS_ERR(balong_pcie[id].regulator)) {
                pcie_trace("fail to get regulator\n");
            }
        }

        if (strnlen(balong_pcie[id].clock_sc_id, 255U) != 0) {
            balong_pcie[id].clock_sc = clk_get(NULL, balong_pcie[id].clock_sc_id);
            if (IS_ERR(balong_pcie[id].clock_sc)) {
                pcie_trace("fail to get sc clock\n");
            }
        }

        if (strnlen(balong_pcie[id].clock_core_id, 255U) != 0) {
            balong_pcie[id].clock_core = clk_get(NULL, balong_pcie[id].clock_core_id);
            if (IS_ERR(balong_pcie[id].clock_core)) {
                pcie_trace("fail to get core clock\n");
            }
        }

        if (strnlen(balong_pcie[id].clock_aux_id, 255U) != 0) {
            balong_pcie[id].clock_aux = clk_get(NULL, balong_pcie[id].clock_aux_id);
            if (IS_ERR(balong_pcie[id].clock_aux)) {
                pcie_trace("fail to get aux clock\n");
            }
        }

        if (strnlen(balong_pcie[id].clock_phy_id, 255U) != 0) {
            balong_pcie[id].clock_phy = clk_get(NULL, balong_pcie[id].clock_phy_id);
            if (IS_ERR(balong_pcie[id].clock_phy)) {
                pcie_trace("fail to get phy clock\n");
            }
        }

        if (!balong_pcie[id].virt_sc_addr) {
            balong_pcie[id].virt_sc_addr = ioremap(balong_pcie[id].phys_sc_addr, balong_pcie[id].phys_sc_size);
            if (!balong_pcie[id].virt_sc_addr) {
                pcie_trace("fail to remap\n");
                return;
            }
        }
    }
}

int bsp_pcie_software_init(u32 id)
{
    u32 i = 0;
    int ret = 0;

    spin_lock_init(&balong_pcie[id].spinlock);
    spin_lock_init(&balong_pcie[id].spinlock_req_clk);

    for (i = 0; i < sizeof(balong_pcie[id].irqs.dma_semaphore)/sizeof(balong_pcie[id].irqs.dma_semaphore[0]); i++) {
        sema_init(&balong_pcie[id].irqs.dma_semaphore[i], 1);
    }

    ret |= request_irq(balong_pcie[id].irqs.int_link_down, balong_pcie_linkdown_isr,
            IRQF_DISABLED, "pcie link down", (void*)&balong_pcie[id]);

    ret |= request_irq(balong_pcie[id].irqs.int_edma, balong_pcie_dma_isr,
            IRQF_DISABLED, "pcie dma", (void*)&balong_pcie[id]);

    ret |= request_irq(balong_pcie[id].irqs.int_pm, balong_pcie_pm_isr,
            IRQF_DISABLED, "pcie pm", (void*)&balong_pcie[id]);
    if (ret) {
        pcie_trace("fail to request irq, controller: %u\n", id);
        return ret;
    }

    balong_pcie[id].virt_device_config_addr = ioremap(balong_pcie[id].phys_device_config_addr, balong_pcie[id].phys_device_config_size);
    if (!balong_pcie[id].virt_device_config_addr) {
        pcie_trace("fail to remap\n");
        return -1;
    }

    if (balong_pcie[id].phys_rc_cfg_addr != balong_pcie[id].phys_device_config_addr) {
        balong_pcie[id].virt_rc_cfg_addr = ioremap(balong_pcie[id].phys_rc_cfg_addr, balong_pcie[id].phys_rc_cfg_size);
        if (!balong_pcie[id].virt_rc_cfg_addr) {
            pcie_trace("fail to remap\n");
            return -1;
        }
    } else {
        balong_pcie[id].virt_rc_cfg_addr = balong_pcie[id].virt_device_config_addr;
    }

    if (PCIE_WORK_MODE_RC == balong_pcie[id].work_mode) {
#if defined(CONFIG_PCI_MSI)
        /*setup msi irq handler*/
        irq_set_chained_handler(balong_pcie[id].irqs.int_msi, balong_pcie_msi_isr);
        irq_set_handler_data(balong_pcie[id].irqs.int_msi, (void*)&balong_pcie[id]);
        balong_pcie[id].irq_domain = irq_domain_add_linear(balong_pcie[id].dev_node, BALONG_MAX_MSI_NUM,
                         &pcie_balong_irq_domain_ops, (void*)&balong_pcie[id]);
#endif
    }

    return 0;
}

void bsp_pcie_power_on(u32 id)
{
    int ret = 0;

    if (balong_pcie[id].regulator)
        ret |= regulator_enable(balong_pcie[id].regulator);

    if (ret)
        pcie_trace("fail to enable regulator\n");
}

void bsp_pcie_power_off(u32 id)
{
    int ret = 0;

    if (balong_pcie[id].regulator)
        ret |= regulator_disable(balong_pcie[id].regulator);

    if (ret)
        pcie_trace("fail to disable regulator\n");
}

void bsp_pcie_clk_init(u32 id)
{
#if defined(BALONG_PCI_INNER_CLK)
    if (!balong_pcie[id].enabled)
        return;

    if (balong_pcie[id].clock_mode == PCIE_CLOCK_MODE_OUTER)
        return;

#if defined(BSP_CONFIG_BOARD_E5)
    u32 pcie_p2p_connent_mode = 0;
    /*端接方式寄存器，设置成内部端接*/
    pcie_p2p_connent_mode = 0x40000018;
    writel(pcie_p2p_connent_mode,balong_pcie[id].virt_sc_addr + 0xc10);
#endif

#if (FEATURE_ON == MBB_PCIE1_INNER_CONNECT)
    u32 pcie_p2p_connent_mode = 0;
    /* 端接方式寄存器，上电默认是外部端接 */
    /* SC_PCIE_CTRL4 寄存器偏移地址是 0xc10，bit18 控制 pcie1时钟选择，bit2控制 pcie0时钟选择（0内部，1外部） */
    /* B612 和 B610s-79a 的 pcie1是内部端接(bit18设为0)，pcie0是外部端接(bit2设为1)，在这里设置。 */
    pcie_p2p_connent_mode = readl(balong_pcie[id].virt_sc_addr + 0xc10);
    pcie_p2p_connent_mode = pcie_p2p_connent_mode & (~((1 << 18) | (1 << 2))) | ((0 << 18) | (1 << 2));
    writel(pcie_p2p_connent_mode, balong_pcie[id].virt_sc_addr + 0xc10);
#endif

    /* switch to inner clock */
    hi_pcie_inner_clk_enable(balong_pcie[id].virt_sc_addr, id);

    if (!balong_pcie[id].output_clock_disable) {
        /* enable to output clock */
        hi_pcie_output_clk_enable(balong_pcie[id].virt_sc_addr, id);
    }
#endif  /* BALONG_PCI_INNER_CLK */
}

void bsp_pcie_hardware_common_init(void)
{
    u32 id = 0;
    int ret = 0;

    for (id = 0; id < balong_pcie_num; id++) {
        if (!balong_pcie[id].enabled)
            continue;

        if (!IS_ERR_OR_NULL(balong_pcie[id].clock_sc)) {
            clk_prepare(balong_pcie[id].clock_sc);
            ret |= clk_enable(balong_pcie[id].clock_sc);
        }
        if (ret)
            pcie_trace("fail to enable clock\n");

        bsp_pcie_clk_init(id);
    }
}

void bsp_pcie_hardware_common_exit(void)
{
    u32 id = 0;

    for (id = 0; id < balong_pcie_num; id++) {
        if (!balong_pcie[id].enabled)
            continue;

        if (!IS_ERR_OR_NULL(balong_pcie[id].clock_sc)) {
            clk_disable(balong_pcie[id].clock_sc);
            clk_unprepare(balong_pcie[id].clock_sc);
        }
    }
}

void bsp_pcie_clk_enable(u32 id)
{
    int ret = 0;

    /* enable core_clk, aux_clk and phy clock */
    if (!IS_ERR_OR_NULL(balong_pcie[id].clock_core)) {
        clk_prepare(balong_pcie[id].clock_core);
        ret |= clk_enable(balong_pcie[id].clock_core);
    }

    if (!IS_ERR_OR_NULL(balong_pcie[id].clock_aux)) {
        clk_prepare(balong_pcie[id].clock_aux);
        ret |= clk_enable(balong_pcie[id].clock_aux);
    }

    if (!IS_ERR_OR_NULL(balong_pcie[id].clock_phy)) {
        clk_prepare(balong_pcie[id].clock_phy);
        ret |= clk_enable(balong_pcie[id].clock_phy);
    }

    if (ret)
        pcie_trace("fail to enable clock\n");
}

void bsp_pcie_clk_disable(u32 id)
{
    /* disable core_clk, aux_clk and phy clock */
    if (!IS_ERR_OR_NULL(balong_pcie[id].clock_core)) {
        clk_disable(balong_pcie[id].clock_core);
        clk_unprepare(balong_pcie[id].clock_core);
    }

    if (!IS_ERR_OR_NULL(balong_pcie[id].clock_aux)) {
        clk_disable(balong_pcie[id].clock_aux);
        clk_unprepare(balong_pcie[id].clock_aux);
    }

    if (!IS_ERR_OR_NULL(balong_pcie[id].clock_phy)) {
        clk_disable(balong_pcie[id].clock_phy);
        clk_unprepare(balong_pcie[id].clock_phy);
    }
}

void bsp_pcie_ltssm_enable(u32 id)
{
    unsigned long irq_flags = 0;

    spin_lock_irqsave(&balong_pcie[id].spinlock, irq_flags);

    hi_pcie_ltssm_enable(balong_pcie[id].virt_sc_addr, id);

    spin_unlock_irqrestore(&balong_pcie[id].spinlock, irq_flags);
}

void bsp_pcie_ltssm_disable(u32 id)
{
    unsigned long irq_flags = 0;

    spin_lock_irqsave(&balong_pcie[id].spinlock, irq_flags);

    hi_pcie_ltssm_disable(balong_pcie[id].virt_sc_addr, id);

    spin_unlock_irqrestore(&balong_pcie[id].spinlock, irq_flags);
}

void bsp_pcie_phy_init(u32 id)
{
    hi_pcie_phy_init(balong_pcie[id].virt_sc_addr, id);
}

void bsp_pcie_assert_reset(u32 id)
{
    unsigned long irq_flags = 0;

    spin_lock_irqsave(&balong_pcie[id].spinlock, irq_flags);

    hi_pcie_set(bsp_sysctrl_addr_get((void*)balong_pcie[id].core_assert_reset_addr),
        balong_pcie[id].core_assert_reset_offset, 1,
        balong_pcie[id].core_assert_reset_value);

    hi_pcie_set(bsp_sysctrl_addr_get((void*)balong_pcie[id].phy_assert_reset_addr),
        balong_pcie[id].phy_assert_reset_offset, 1,
        balong_pcie[id].phy_assert_reset_value);

    spin_unlock_irqrestore(&balong_pcie[id].spinlock, irq_flags);
}

void bsp_pcie_deassert_reset(u32 id)
{
    unsigned long irq_flags = 0;

    spin_lock_irqsave(&balong_pcie[id].spinlock, irq_flags);

    hi_soc_peri_usb_deassert_reset(balong_pcie[id].virt_sc_addr, id);

    hi_pcie_phy_deassert_reset(balong_pcie[id].virt_sc_addr, id);

    hi_pcie_set(bsp_sysctrl_addr_get((void*)balong_pcie[id].core_deassert_reset_addr),
        balong_pcie[id].core_deassert_reset_offset, 1,
        balong_pcie[id].core_deassert_reset_value);

    hi_pcie_set(bsp_sysctrl_addr_get((void*)balong_pcie[id].phy_deassert_reset_addr),
        balong_pcie[id].phy_deassert_reset_offset, 1,
        balong_pcie[id].phy_deassert_reset_value);

    spin_unlock_irqrestore(&balong_pcie[id].spinlock, irq_flags);

    //udelay(100);

    while (hi_pcie_is_under_reset(balong_pcie[id].virt_sc_addr, id)) ;
}

enum PCIE_CAPABILITY_TYPE
{
    PCIE_CAPABILITY_TYPE_PCI_STANDARD,
    PCIE_CAPABILITY_TYPE_PCIE_EXTENDED,
};

/* Attention: the dbi must be enabled when called this function */
u32 pcie_get_capability_by_id(u32 id, u32 capability_id, enum PCIE_CAPABILITY_TYPE type)
{
    u32 value = 0;
    u32 offset = 0;
    u32 current_id = 0;

    if (PCIE_CAPABILITY_TYPE_PCI_STANDARD == type) {
        /* PCI Standard Capability */
        offset = readl(balong_pcie[id].virt_rc_cfg_addr + PCI_CAPABILITY_LIST) & 0xFF;
        do {
            value = readl(balong_pcie[id].virt_rc_cfg_addr + offset);
            current_id = value & 0xFF;

            if (current_id == capability_id)
                break;

            offset = (value >> 8) & 0xFF;
        } while (offset);
    } else {
        /* PCI Express Extended Capability */
        offset = 0x100;
        do {
            value = readl(balong_pcie[id].virt_rc_cfg_addr + offset);
            current_id = PCI_EXT_CAP_ID(value);

            if (current_id == capability_id)
                break;

            offset = PCI_EXT_CAP_NEXT(value);
        } while (offset);
    }

    if (current_id != capability_id)
        offset = 0;

    return offset;
}

void bsp_pcie_ctrl_init(u32 id)
{
    u32 value = 0;
    u32 offset = 0;
    unsigned long irq_flags = 0;

    spin_lock_irqsave(&balong_pcie[id].spinlock, irq_flags);

    /* set work mode */
    hi_pcie_set_work_mode(balong_pcie[id].virt_sc_addr, id, balong_pcie[id].work_mode);

    pcie_app_req_clk(id, 1);

    dbi_enable(id);

    /* set to correct class mode
     * High 24 bits are class, low 8 revision
     */
    value = readl(balong_pcie[id].virt_rc_cfg_addr + PCI_CLASS_REVISION);
    value &= 0x000000FF;
    if (PCIE_WORK_MODE_RC == balong_pcie[id].work_mode)
        value |= ((PCI_CLASS_BRIDGE_PCI << 8) << 8);
    else
        value |= ((PCI_CLASS_COMMUNICATION_MODEM << 8) << 8);
    writel(value, balong_pcie[id].virt_rc_cfg_addr + PCI_CLASS_REVISION);

    /* set to correct link mode */
    value = readl(balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x010);
    value &= (~(0x3F<<16));
    value |= (balong_pcie[id].link_mode<<16);
    writel(value, balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x010);

    value = readl(balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x10C);
    value &= (~(0x1FF<<8));
    if (PCIE_LINK_MODE_X1 == balong_pcie[id].link_mode)
        value |= (0x1<<8);
    else if (PCIE_LINK_MODE_X2 == balong_pcie[id].link_mode)
        value |= (0x2<<8);
    writel(value, balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x10C);

    /* PCI_CAP_ID_EXP = 0x10, PCI Express Capability */
    offset = pcie_get_capability_by_id(id, PCI_CAP_ID_EXP, PCIE_CAPABILITY_TYPE_PCI_STANDARD);
    if (offset) {
        /* force to set speed mode */
        value = readl(balong_pcie[id].virt_rc_cfg_addr + offset + 0x0C);
        value = ((value & (~0xF)) | balong_pcie[id].speed_mode);
        writel(value, balong_pcie[id].virt_rc_cfg_addr + offset + 0x0C);

        if (balong_pcie[id].compliance_test_enable == PCIE_ENABLE) {
            printk(KERN_ALERT"[PCIe]: Balong PCIe %u is force to enter Polling.Compliance mode\n", id);
            value = readl(balong_pcie[id].virt_rc_cfg_addr + offset + 0x30);
            value |= (0x01<<4);
            writel(value, balong_pcie[id].virt_rc_cfg_addr + offset + 0x30);
        }

        if (PCIE_WORK_MODE_EP == balong_pcie[id].work_mode) {
            /* set acceptable latency for L0s and L1 */
            value = readl(balong_pcie[id].virt_rc_cfg_addr + offset + 0x04);
            value |= (0x7 << 6);    /* L0s  : more than 4 us */
            value |= (0x7 << 9);    /* L1   : more than 4 us */
            writel(value, balong_pcie[id].virt_rc_cfg_addr + offset + 0x04);
        }
    }

    if (!balong_pcie[id].l1ss_disable) {
        /* L1SS Setting L1 Substates Capabality ID == 0x1E */
        offset = pcie_get_capability_by_id(id, 0x1E, PCIE_CAPABILITY_TYPE_PCIE_EXTENDED);
        if (offset) {
            value = readl(balong_pcie[id].virt_rc_cfg_addr + offset + 0x04);
            value &= 0x0F;
            value |= readl(balong_pcie[id].virt_rc_cfg_addr + offset + 0x08);
            writel(value, balong_pcie[id].virt_rc_cfg_addr + offset + 0x08);
        }
    }

    if (PCIE_WORK_MODE_RC == balong_pcie[id].work_mode) {
        /* disable BAR */
        writel(0x00, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_0));
        writel(0x00, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_1));
        writel(0x00, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_2));
        writel(0x00, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_3));
        writel(0x00, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_4));
        writel(0x00, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_5));

#if defined(CONFIG_PCI_MSI)
        /* setup msi addr for msi interrupt controller */
        writel(BALONG_PCIE_MSI_PCI_LOWER_ADDR(balong_pcie[id].phys_pcie_cfg),
               balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x120);
        writel(BALONG_PCIE_MSI_PCI_UPPER_ADDR(balong_pcie[id].phys_pcie_cfg),
               balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x124);
#endif

        /* init iatu table */
        balong_pcie_iatu[0].lower_addr = balong_pcie[id].res_cfg0.start;
        balong_pcie_iatu[0].upper_addr = 0;
        /* cfg shift mode bit[27:12] B:F:D - 8:5:3 */
        balong_pcie_iatu[0].limit_addr = balong_pcie[id].res_cfg0.end;
        balong_pcie_iatu[1].lower_addr = balong_pcie[id].res_cfg1.start;
        balong_pcie_iatu[1].upper_addr = 0;
        balong_pcie_iatu[1].limit_addr = balong_pcie[id].res_cfg1.end;
        balong_pcie_iatu[2].lower_addr = balong_pcie[id].res_io.start;
        balong_pcie_iatu[2].upper_addr = 0;
        balong_pcie_iatu[2].limit_addr = balong_pcie[id].res_io.end;
        balong_pcie_iatu[2].lower_target_addr = 0;
        balong_pcie_iatu[2].upper_target_addr = 0;
        balong_pcie_iatu[3].lower_addr = balong_pcie[id].res_mem.start;
        balong_pcie_iatu[3].upper_addr = 0;
        balong_pcie_iatu[3].limit_addr = balong_pcie[id].res_mem.end;
        balong_pcie_iatu[3].lower_target_addr = balong_pcie[id].res_mem.start;
        balong_pcie_iatu[3].upper_target_addr = 0;
        balong_pcie_iatu[4].lower_addr = BALONG_PCIE_MSI_PCI_LOWER_ADDR(balong_pcie[id].phys_pcie_cfg);
        balong_pcie_iatu[4].upper_addr = BALONG_PCIE_MSI_PCI_UPPER_ADDR(balong_pcie[id].phys_pcie_cfg);
        balong_pcie_iatu[4].limit_addr = balong_pcie_iatu[4].lower_addr + BALONG_PCIE_MSI_PCI_SIZE - 1;
        balong_pcie_iatu[4].lower_target_addr = BALONG_PCIE_MSI_PCI_LOWER_ADDR(balong_pcie[id].phys_pcie_cfg);
        balong_pcie_iatu[4].upper_target_addr = BALONG_PCIE_MSI_PCI_UPPER_ADDR(balong_pcie[id].phys_pcie_cfg);
        balong_pcie_iatu[5].lower_addr = 0;
        balong_pcie_iatu[5].upper_addr = 0;
        balong_pcie_iatu[5].limit_addr = 0xFFFFFFFF;
        balong_pcie_iatu[5].lower_target_addr = 0;
        balong_pcie_iatu[5].upper_target_addr = 0;

        pcie_set_iatu(id, &balong_pcie_iatu[0], balong_pcie_iatu_num);
    } else {
        /* BAR Register
         * bit[0:0]   0 = Memory BAR; 1 = I/O BAR
         * bit[2:1]   00= 32-bit BAR; 1 = 64-bit BAR
         * bit[3:3]   0 = Non-prefetchable; 1 = Prefetchable
         * bit[31:4]  Base Address bits
         */
        /* modify bar to non-prefetchmem */
        writel(0x0, balong_pcie[id].virt_rc_cfg_addr + PCI_BASE_ADDRESS_0);
        writel(0x0, balong_pcie[id].virt_rc_cfg_addr + PCI_BASE_ADDRESS_1);
        writel(0x0, balong_pcie[id].virt_rc_cfg_addr + PCI_BASE_ADDRESS_2);
        writel(0x0, balong_pcie[id].virt_rc_cfg_addr + PCI_BASE_ADDRESS_3);
        writel(0x0, balong_pcie[id].virt_rc_cfg_addr + PCI_BASE_ADDRESS_4);
        writel(0x0, balong_pcie[id].virt_rc_cfg_addr + PCI_BASE_ADDRESS_5);

        /* BAR Mask Register
         * bit[0:0]   0 = BAR is disable; 1 = BAR is enable
         * bit[31:1]  Indicates which BAR bits to mask
         */
        /* set bar0 size to 4KB */
        writel(0xFFFFF, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_0));
        /* set bar2 size to 4KB
         * we use BAR2 here because the BAR1 can not be resize on 6930(default size is 64KB)
         */
        writel(0x0FFF, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_2));
        /* disable other bar */
        writel(0, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_1));
        writel(0, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_3));
        writel(0, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_4));
        writel(0, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_5));


        /* set subsystem vender id to 0x1 for distingushing hisilicon rc or ep.
         * rc subsystem vendor id is 0.
         * ep subsystem vendor id is 1.
         */
        writel(0x1, balong_pcie[id].virt_rc_cfg_addr + PCI_SUBSYSTEM_VENDOR_ID);
    }

    dbi_disable(id);

    pcie_app_req_clk(id, 0);

    spin_unlock_irqrestore(&balong_pcie[id].spinlock, irq_flags);
}

void bsp_pcie_link_up_check(u32 id)
{
    balong_pcie[id].linked = hi_pcie_is_linkup(balong_pcie[id].virt_sc_addr, id);
}

void bsp_pcie_wait_for_linkup(u32 id)
{
    int count = 0;

    if (PCIE_WORK_MODE_RC == balong_pcie[id].work_mode) {
        do {
            count++;
            udelay(1000);
            bsp_pcie_link_up_check(id);
        }while ((!balong_pcie[id].linked) && (count < 1*1000));
    } else {
        printk(KERN_ERR "*************************************************\n"
                        "* [PCIE]: EP init ok, waiting for RC connect... *\n"
                        "*************************************************\n");
    }

    if (!balong_pcie[id].l1ss_disable) {
        if ((PCIE_WORK_MODE_RC == balong_pcie[id].work_mode && balong_pcie[id].linked) ||
            (PCIE_WORK_MODE_EP == balong_pcie[id].work_mode)) {
            hi_pcie_l1ss_auto_gate_enable(balong_pcie[id].virt_sc_addr, id);
        }
    }
}

void bsp_pcie_try_change_speed(u32 id)
{
    u32 value = 0;
    u32 link_speed = 0;
    unsigned long irq_flags = 0;

    spin_lock_irqsave(&balong_pcie[id].spinlock, irq_flags);

    pcie_app_req_clk(id, 1);

    dbi_enable(id);

    value = readl(balong_pcie[id].virt_rc_cfg_addr + 0x70 + 0x10);
    link_speed = ((value >> 16) & 0x0F);
    if (link_speed < balong_pcie[id].speed_mode) {
        value = readl(balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x10C);
        value |= (0x01<<17);
        writel(value, balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x10C);

        udelay(200);
    }

    dbi_disable(id);

    pcie_app_req_clk(id, 0);

    spin_unlock_irqrestore(&balong_pcie[id].spinlock, irq_flags);
}

void bsp_pcie_show_link_status(u32 id)
{
    u32 value = 0;
    u32 link_speed = 0;
    u32 link_width = 0;
    unsigned long irq_flags = 0;

    spin_lock_irqsave(&balong_pcie[id].spinlock, irq_flags);

    pcie_app_req_clk(id, 1);

    dbi_enable(id);
    value = readl(balong_pcie[id].virt_rc_cfg_addr + 0x70 + 0x10);
    dbi_disable(id);

    pcie_app_req_clk(id, 0);

    spin_unlock_irqrestore(&balong_pcie[id].spinlock, irq_flags);

    link_width = ((value >> 20) & 0x3F);
    link_speed = ((value >> 16) & 0x0F) / 2;

    printk(KERN_ALERT"[PCIe]: Balong PCIe %u Info: %s %s, Lane: x%u\n", id,
        balong_pcie_info_phy_rate_str[link_speed],
        balong_pcie_info_gen_str[link_speed],
        link_width);
}

void bsp_pcie_hardware_init(u32 id)
{
    bsp_pcie_power_on(id);
    bsp_pcie_clk_enable(id);
    bsp_pcie_ltssm_disable(id);
    bsp_pcie_phy_init(id);
    bsp_pcie_assert_perst(id);
    bsp_pcie_assert_reset(id);
    bsp_pcie_deassert_perst(id);
    bsp_pcie_deassert_reset(id);
    bsp_pcie_ctrl_init(id);
    bsp_pcie_ltssm_enable(id);
}

void bsp_pcie_hardware_exit(u32 id)
{
    bsp_pcie_ltssm_disable(id);
    bsp_pcie_assert_perst(id);
    bsp_pcie_assert_reset(id);
    bsp_pcie_clk_disable(id);
    bsp_pcie_power_off(id);
}

int bsp_pcie_dma_transfer(struct pcie_dma_transfer_info *transfer_info)
{
    int ret = 0;
    struct balong_pcie_info *info = NULL;

    if ((NULL == transfer_info) || (NULL == transfer_info->dev)) {
        pcie_trace("error args, transfer_info is NULL\n");
        return -1;
    }

    if (transfer_info->channel > BALONG_PCIE_MAX_DMA_CHANNEL_NUM) {
        pcie_trace("error channel index, the max channel index is %u\n", BALONG_PCIE_MAX_DMA_CHANNEL_NUM);
        return -1;
    }

    info = (struct balong_pcie_info *)((struct pci_sys_data *)transfer_info->dev->bus->sysdata)->private_data;

    while(down_interruptible(&info->irqs.dma_semaphore[transfer_info->channel])) ;
    pcie_app_req_clk(info->id, 1);

    info->irqs.dma_channel_state |= (1U << transfer_info->channel);
    info->irqs.dma_int_callback[transfer_info->channel] = transfer_info->callback;
    info->irqs.dma_int_callback_args[transfer_info->channel] = transfer_info->callback_args;

    if (PCIE_DMA_DIRECTION_READ == transfer_info->direction) {
        /* engine enable */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x29C, 0x01);
        /* interrupt mask */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x318, 0x00);
        /* channel context */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x36C, transfer_info->channel | 0x80000000);
        /* channel control 1 */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x370, 0x04000008);
        /* transfer size */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x378, transfer_info->transfer_size);
        /* transfer address */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x37C, transfer_info->sar_low);
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x380, transfer_info->sar_high);
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x384, transfer_info->dar_low);
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x388, transfer_info->dar_high);
        /* doorbell */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x2A0, transfer_info->channel);
    } else {
        /* engine enable */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x27C, 0x01);
        /* interrupt mask */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x2C4, 0x00);
        /* channel context */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x36C, transfer_info->channel);
        /* channel control 1 */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x370, 0x04000008);
        /* transfer size */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x378, transfer_info->transfer_size);
        /* transfer address */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x37C, transfer_info->sar_low);
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x380, transfer_info->sar_high);
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x384, transfer_info->dar_low);
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x388, transfer_info->dar_high);
        /* doorbell */
        ret |= pci_write_config_dword(transfer_info->dev, 0x700 + 0x280, transfer_info->channel);
    }

    return ret;
}

struct hw_pci balong_hw_pci = {
#ifdef CONFIG_PCI_DOMAINS
    .domain         = 0,
#endif
    .ops            = &balong_pcie_ops,
    .nr_controllers = 0,
    .private_data   = NULL,
    .setup          = balong_pcie_setup,
    .scan           = balong_pcie_scan,
    .preinit        = NULL,
    .postinit       = NULL,
    .swizzle        = pci_common_swizzle,
    .map_irq        = balong_pcie_map_irq,
    .align_resource = NULL,
};

static int __init balong_pcie_init(void)
{
    u32 id = 0;
    int ret = 0;

    u32 init_mode;

#ifdef CONFIG_BALONG_MODEM_ONOFF
    if( DRV_START_MODE_CHARGING == bsp_start_mode_get())
    {
        return ret; 
    }
#endif    
    bsp_pcie_software_common_init();
    bsp_pcie_hardware_common_init();

    /* there are two loops, the first one is for EP and the second one is for RC.
     * we must init EP at first to ensure that the EP is ready when the RC start the ltssm.
     */
    init_mode = PCIE_WORK_MODE_EP;
    for (id = 0; id < balong_pcie_num; id++) {
        if (!balong_pcie[id].enabled)
            continue;

        if (balong_pcie[id].work_mode != init_mode)
            continue;

        ret = bsp_pcie_software_init(id);
        if (ret)
            continue;
        bsp_pcie_hardware_init(id);
        bsp_pcie_wait_for_linkup(id);
    }

    init_mode = PCIE_WORK_MODE_RC;
    for (id = 0; id < balong_pcie_num; id++) {
        if (!balong_pcie[id].enabled)
            continue;

        if (balong_pcie[id].work_mode != init_mode)
            continue;

        ret = bsp_pcie_software_init(id);
        if (ret)
            continue;

        balong_hw_pci.nr_controllers++;

        bsp_pcie_hardware_init(id);
        bsp_pcie_wait_for_linkup(id);

        if (balong_pcie[id].linked) {
            bsp_pcie_try_change_speed(id);
            bsp_pcie_show_link_status(id);
        } else {
            pcie_trace("link up time out, controller: %u\n", id);
        }
    }

    pci_common_init(&balong_hw_pci);

    return ret;
}



subsys_initcall_sync(balong_pcie_init);


/* Debug Function */
int d_sc_config(u32 id, u32 offset)
{
    u32 result = 0;

    id %= balong_pcie_num;

    offset &= (~(0x03));
    offset %= balong_pcie[id].phys_sc_size;

    result = readl(balong_pcie[id].virt_sc_addr + offset);

    printk(KERN_ERR"%08X: 0x%08X\n", (u32)balong_pcie[id].virt_sc_addr + offset, result);

    return 0;
}

int m_sc_config(u32 id, u32 offset, u32 value)
{
    id %= balong_pcie_num;

    offset &= (~(0x03));
    offset %= balong_pcie[id].phys_sc_size;

    writel(value, balong_pcie[id].virt_sc_addr + offset);

    return d_sc_config(id, offset);
}

int d_rc_config(u32 id, u32 offset)
{
    u32 result = 0;
    unsigned long irq_flags = 0;

    id %= balong_pcie_num;

    offset &= (~(0x03));
    offset %= balong_pcie[id].phys_rc_cfg_size;

    spin_lock_irqsave(&balong_pcie[id].spinlock, irq_flags);

    pcie_app_req_clk(id, 1);

    dbi_enable(id);

    result = readl(balong_pcie[id].virt_rc_cfg_addr + offset);

    printk(KERN_ERR"%08X: 0x%08X\n", balong_pcie[id].phys_rc_cfg_addr + offset, result);

    dbi_disable(id);

    pcie_app_req_clk(id, 0);

    spin_unlock_irqrestore(&balong_pcie[id].spinlock, irq_flags);

    return 0;
}

int m_rc_config(u32 id, u32 offset, u32 value)
{
    unsigned long irq_flags = 0;

    id %= balong_pcie_num;

    offset &= (~(0x03));
    offset %= balong_pcie[id].phys_rc_cfg_size;

    spin_lock_irqsave(&balong_pcie[id].spinlock, irq_flags);

    pcie_app_req_clk(id, 1);

    dbi_enable(id);

    writel(value, balong_pcie[id].virt_rc_cfg_addr + offset);

    dbi_disable(id);

    pcie_app_req_clk(id, 0);

    spin_unlock_irqrestore(&balong_pcie[id].spinlock, irq_flags);

    return d_rc_config(id, offset);
}

int d_ep_config(u32 id, u32 offset)
{
    u32 result = 0;
    unsigned long irq_flags = 0;

    id %= balong_pcie_num;

    offset &= (~(0x03));
    offset %= balong_pcie[id].phys_device_config_size;

    spin_lock_irqsave(&balong_pcie[id].spinlock, irq_flags);

    pcie_app_req_clk(id, 1);

    result = readl(balong_pcie[id].virt_device_config_addr + offset);

    printk(KERN_ERR"%08X: 0x%08X\n", balong_pcie[id].phys_rc_cfg_addr + offset, result);

    pcie_app_req_clk(id, 0);

    spin_unlock_irqrestore(&balong_pcie[id].spinlock, irq_flags);

    return 0;
}

int m_ep_config(u32 id, u32 offset, u32 value)
{
    unsigned long irq_flags = 0;

    id %= balong_pcie_num;

    offset &= (~(0x03));
    offset %= balong_pcie[id].phys_device_config_size;

    spin_lock_irqsave(&balong_pcie[id].spinlock, irq_flags);

    pcie_app_req_clk(id, 1);

    writel(value, balong_pcie[id].virt_device_config_addr + offset);

    pcie_app_req_clk(id, 0);

    spin_unlock_irqrestore(&balong_pcie[id].spinlock, irq_flags);

    return d_ep_config(id, offset);
}


#define YES_OR_NO_STR(value)   (value ? "Yes" : "No")

int pcie_info(void)
{
    u32 id = 0;
    u32 value = 0;
    char* pcieport_driver_mode_str[] = {"Auto", "Native", "Compat"};

    printk(KERN_ALERT"SC Physical Address: 0x%08X\n\n", balong_pcie[id].phys_sc_addr);

    for (id = 0; id < balong_pcie_num; id++) {
        printk(KERN_ALERT"PCIe %u:\n", id);
        printk(KERN_ALERT"enabled:      %s\n", YES_OR_NO_STR(balong_pcie[id].enabled));
        printk(KERN_ALERT"work mode:    %s\n", balong_pcie[id].work_mode == PCIE_WORK_MODE_RC ? "RC" : "EP");
        printk(KERN_ALERT"port mode:    %s\n", pcieport_driver_mode_str[balong_pcie[id].port_mode]);
        printk(KERN_ALERT"clock mode:   %s\n", balong_pcie[id].clock_mode == PCIE_CLOCK_MODE_INNER ? "Inner" : "Outer");
        printk(KERN_ALERT"speed mode:   GEN%d\n", balong_pcie[id].speed_mode);
        printk(KERN_ALERT"msi disable:  %s\n", YES_OR_NO_STR(balong_pcie[id].msi_disable));
        printk(KERN_ALERT"aspm disable: %s\n", YES_OR_NO_STR(balong_pcie[id].aspm_disable));
        printk(KERN_ALERT"l1ss disable: %s\n", YES_OR_NO_STR(balong_pcie[id].l1ss_disable));
        printk(KERN_ALERT"output clock disable:   %s\n", YES_OR_NO_STR(balong_pcie[id].output_clock_disable));
        printk(KERN_ALERT"compliance test enable: %s\n", YES_OR_NO_STR(balong_pcie[id].compliance_test_enable));

        if (balong_pcie[id].enabled && balong_pcie[id].work_mode == PCIE_WORK_MODE_RC) {
            value = (u32)pci_get_domain_bus_and_slot(0, balong_pcie[id].busnr, 0);
            printk(KERN_ALERT"pci_dev address: 0x%08X\n", value);
            if (value)
                pci_dev_put((struct pci_dev *)value);
        }
        printk(KERN_ALERT"Vendor ID: 0x%04X, Device ID: 0x%04X\n",
            (u16)balong_pcie[id].vendor_id, (u16)balong_pcie[id].device_id);
        printk(KERN_ALERT"Config Physical Address: 0x%08X, Data Physical Address: 0x%08X\n",
            balong_pcie[id].phys_pcie_cfg, balong_pcie[id].phys_pcie_data);

        bsp_pcie_link_up_check(id);
        if (balong_pcie[id].linked)
            bsp_pcie_show_link_status(id);
        else
            printk(KERN_ALERT"Balong PCIe %u is link down\n", id);

        value = hi_pcie_get_link_status(balong_pcie[id].virt_sc_addr, id);
        printk(KERN_ALERT"Controller status: 0x%08X\n\n", value);
    }

    return 0;
}

int pcie_help(void)
{
    printk(KERN_ALERT"d/m_sc_config id offset [value]\n");
    printk(KERN_ALERT"d/m_rc_config id offset [value]\n");
    printk(KERN_ALERT"d/m_ep_config id offset [value]\n");

    return 0;
}

