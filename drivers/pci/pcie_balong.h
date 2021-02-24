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
#ifndef __PCIE_BALONG_H__
#define __PCIE_BALONG_H__

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/printk.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/regulator/consumer.h>

#include <asm-generic/sizes.h>

#include <hi_pcie_interface.h>

#ifdef __cplusplus
extern "C" {
#endif


/********************************* Definition **********************************/

/******* work mode **********/
/* ep mode */
#define PCIE_WORK_MODE_EP   (0x0)
/* legacy ep mode */
#define PCIE_WORK_MODE_LEP  (0x1)
/* rc mode */
#define PCIE_WORK_MODE_RC   (0x4)

/******* link mode **********/
#define PCIE_LINK_MODE_X1   (0x1)
#define PCIE_LINK_MODE_X2   (0x3)


#define pcie_trace(format, args...) do {printk("[PCIE]:line: %d: "format, __LINE__, ##args);} while(0)

#define PCIE_CFG_SHIFT_BUS(x) (x << 20)
#define PCIE_CFG_SHIFT_DEV(x) (x << 15)
#define PCIE_CFG_SHIFT_FUN(x) (x << 12)
#define PCIE_CFG_SHIFT_REG(x) (x & 0xFFF)


#define PCIE_TLP_DIRECTION_OUTBOUND (0x0)
#define PCIE_TLP_DIRECTION_INBOUND  (0x1)

#define PCIE_TLP_TYPE_MEM_RW        (0x0)
#define PCIE_TLP_TYPE_IO_RW         (0x2)
#define PCIE_TLP_TYPE_CFG0_RW       (0x4)
#define PCIE_TLP_TYPE_CFG1_RW       (0x5)

#define PCIE_IATU_MATCH_MODE_ADDR   (0x0)
#define PCIE_IATU_MATCH_MODE_BAR    (0x1)
/******************************************************************************/


/******************************* Configuration ********************************/

#define BALONG_PCIE_MSI_PCI_LOWER_ADDR(x)   (0x0)
#define BALONG_PCIE_MSI_PCI_UPPER_ADDR(x)   (0x0)
#define BALONG_PCIE_MSI_PCI_SIZE            (SZ_1M)
#define BALONG_PCIE_MSI_CPU_LOWER_ADDR(x)   (x)
#define BALONG_PCIE_MSI_CPU_UPPER_ADDR(x)   (0x0)

#define BALONG_MAX_MSI_NUM                  (32*8)


/******************************************************************************/

struct balong_pcie_iatu_table {
    /* +0x200 iATU Index Register */
    union
    {
        struct
        {
            u32 index       :31;   /* Defines which region is being accessed */
            u32 direction   :1 ;   /* 0: Outbound; 1: Inbound */
        } attr;
        u32 value;
    } index;

    /* +0x204 iATU Region Control 1 Register */
    union
    {
        struct
        {
            u32 type   :32;/* bit[4:0]   ----  4b'0000: Memory read/write
                            *                  4b'0010: IO read/write
                            *                  4b'0100: Config Type 0 read/write
                            *                  4b'0101: Config Type 1 read/write
                            */
            /* u32 reserved:27; */
        } attr;
        u32 value;
    } control1;

    /* +0x208 iATU Region Control 2 Register */
    union
    {
        struct
        {
            u32 message_code:8;
            u32 bar_index   :3;
            u32 reserved    :17;
            u32 cfg_shift   :1 ;    /* CFG Shift Mode */
            u32 invert_mode :1 ;
            u32 match_mode  :1 ;
            u32 enable      :1 ;    /* Region Enable */
        } attr;
        u32 value;
    } control2;

    /* +0x20C iATU Region Lower Base Address Register */
    u32 lower_addr;
    /* +0x210 iATU Region Upper Base Address Register */
    u32 upper_addr;
    /* +0x214 iATU Region Limit Address Register */
    u32 limit_addr;
    /* +0x218 iATU Region Lower Target Address Register */
    u32 lower_target_addr;
    /* +0x21C iATU Region Upper Target Address Register */
    u32 upper_target_addr;
    /* +0x220 iATU Region Control 3 Register */
    u32 control3;
};

typedef void (*pcie_callback)(u32, u32, void*);

struct pcie_irq {
    u32 int_link_down;
    u32 int_edma;
    u32 int_pm;
    int int_radm_a;
    int int_radm_b;
    int int_radm_c;
    int int_radm_d;

    u32 int_msi;

    u32 dma_channel_state; /* 1 for busy, each bit corresponds to a DMA channel */
    struct semaphore dma_semaphore[BALONG_PCIE_MAX_DMA_CHANNEL_NUM];
    pcie_callback dma_int_callback[BALONG_PCIE_MAX_DMA_CHANNEL_NUM];
    void *dma_int_callback_args[BALONG_PCIE_MAX_DMA_CHANNEL_NUM];
};

struct balong_pcie_info {
    u32 id;
    u32 busnr;
    struct pcie_irq irqs;
    struct resource res_io;
    struct resource res_mem;
    struct resource res_cfg0;
    struct resource res_cfg1;

    u32 enabled;
    u32 vendor_id;
    u32 device_id;
    u32 work_mode;
    u32 port_mode;
    u32 link_mode;
    u32 clock_mode;
    u32 speed_mode;
    u32 msi_disable;
    u32 aspm_disable;
    u32 l1ss_disable;
    u32 output_clock_disable;
    u32 compliance_test_enable;
    u32 phys_pcie_cfg;
    u32 phys_pcie_data;

    u32 phys_sc_addr;
    u32 phys_sc_size;
    u32 phys_rc_cfg_addr;
    u32 phys_rc_cfg_size;
    u32 phys_device_config_addr;
    u32 phys_device_config_size;
    u32 phys_device_io_size;
    u32 phys_device_mem_size;

    u32 gpio_perst;

    u32 linked;
    void* virt_sc_addr;
    void* virt_rc_cfg_addr;
    void* virt_device_config_addr;
    char *regulator_id;
    struct regulator *regulator;
    char *clock_sc_id;
    struct clk *clock_sc;
    char *clock_core_id;
    struct clk *clock_core;
    char *clock_aux_id;
    struct clk *clock_aux;
    char *clock_phy_id;
    struct clk *clock_phy;
    struct device_node *dev_node;
    struct irq_domain *irq_domain;
    DECLARE_BITMAP(msi_irq_in_use, BALONG_MAX_MSI_NUM);
    u32 phy_assert_reset_addr;
    u32 phy_assert_reset_offset;
    u32 phy_assert_reset_value;
    u32 phy_deassert_reset_addr;
    u32 phy_deassert_reset_offset;
    u32 phy_deassert_reset_value;
    u32 core_assert_reset_addr;
    u32 core_assert_reset_offset;
    u32 core_assert_reset_value;
    u32 core_deassert_reset_addr;
    u32 core_deassert_reset_offset;
    u32 core_deassert_reset_value;

    volatile u32 req_clk_count;

    spinlock_t spinlock;    /* for controller */
    spinlock_t spinlock_req_clk;
};


#define PCIE_DMA_DIRECTION_READ     (0)
#define PCIE_DMA_DIRECTION_WRITE    (1)

struct pcie_dma_transfer_info{
    struct pci_dev* dev;
    u32 channel;
    u32 direction;  /* 0 for read, 1 for write */
    pcie_callback callback; /* NULL for sync transfer */
    void *callback_args;
    u32 sar_low;
    u32 sar_high;
    u32 dar_low;
    u32 dar_high;
    u32 transfer_size;
};

int bsp_pcie_dma_transfer(struct pcie_dma_transfer_info *transfer_info);

int balong_pcie_read_config(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 *value);
int balong_pcie_write_config(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 value);
int balong_pcie_setup(int nr, struct pci_sys_data *sys_data);
int balong_pcie_map_irq(const struct pci_dev *dev, u8 slot, u8 pin);
struct pci_bus* balong_pcie_scan(int nr, struct pci_sys_data *sys_data);


extern u32 balong_pcie_num;
extern struct balong_pcie_info *balong_pcie;
void dbi_enable(u32 id);
void dbi_disable(u32 id);
void pcie_app_req_clk(u32 id, u32 request);
void pcie_set_iatu(u32 id, struct balong_pcie_iatu_table *iatu_table, u32 iatu_table_entry_num);

void bsp_pcie_ltssm_enable(u32 id);
void bsp_pcie_ltssm_disable(u32 id);

void bsp_pcie_link_up_check(u32 id);
void bsp_pcie_wait_for_linkup(u32 id);
void bsp_pcie_try_change_speed(u32 id);

void bsp_pcie_hardware_common_init(void);
void bsp_pcie_hardware_common_exit(void);

void bsp_pcie_hardware_init(u32 id);
void bsp_pcie_hardware_exit(u32 id);


#ifdef __cplusplus
}
#endif


#endif  /* #ifndef __PCIE_BALONG_H__ */

