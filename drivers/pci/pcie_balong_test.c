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
#include <linux/module.h>

#include <linux/io.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/random.h>
#include <linux/jiffies.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <linux/sched.h>

#include <linux/fs.h>
#include <linux/file.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>

#include <linux/pci.h>
#include <linux/aer.h>
#include <linux/pci_ids.h>
#include <linux/pci_regs.h>

#include <asm/mach/pci.h>

#include <hi_pcie_interface.h>

#include <product_config.h>

#include <bsp_edma.h>

#include "pcie_balong.h"


#ifndef DDR_MDM_ACP_ADDR
#define DDR_MDM_ACP_ADDR (0)
#endif

#define PCIE_TEST_BUFFER_ADDR       (DDR_MDM_ACP_ADDR)
#define PCIE_TEST_BUFFER_SIZE       (1024 * 1024)

#define PCIE_TEST_INT_MODE_INTX (0)
#define PCIE_TEST_INT_MODE_MSI  (1)
#define PCIE_TEST_INT_MODE_MSIX (2)
#define PCIE_TEST_INT_MODE_MAX  (PCIE_TEST_INT_MODE_MSIX)
#if defined(CONFIG_PCI_MSI)
#define PCIE_TEST_INT_MODE_DEFAULT  (PCIE_TEST_INT_MODE_MSI)
#else
#define PCIE_TEST_INT_MODE_DEFAULT  (PCIE_TEST_INT_MODE_INTX)
#endif

struct pcie_info
{
    struct pci_dev* balong_rc;
    struct pci_dev* balong_ep;

    u32 rc_vendor_id;   /* RC's vendor id */
    u32 rc_device_id;   /* RC's device id */
    u32 ep_vendor_id;   /* EP's vendor id */
    u32 ep_device_id;   /* EP's device id */

    u32 loopback_enable;

    u32 buffer_size;
    void *rc_phys_addr; /* RC buffer's CPU physical address */
    void *rc_virt_addr;
    void *ep_phys_addr; /* EP buffer's CPU physical address */
    void *ep_virt_addr;

    void *ep_in_rc_cpu_phys_addr;       /* EP buffer's PCI physical address (CPU domain) */
    void *ep_in_rc_cpu_virt_addr;
    void *ep_in_rc_pci_phys_addr;       /* EP buffer's PCI physical address (PCI domain) */
    void *ep_in_rc_cpu_sc_phys_addr;    /* EP sysctrl's CPU physical address */
    void *ep_in_rc_cpu_sc_virt_addr;

    volatile u32 interrupt_mode;        /* INTX, MSI or MSI-X */

    u32 msi_intx_interrupt_send_count;  /* MSI/MSIX/INTX interrupt send and recieve count */
    u32 msi_intx_interrupt_recieve_count;

    u32 dma_interrupt_done_number;      /* DMA interrupt number */
    u32 dma_interrupt_abort_number;

    dma_addr_t edma_list_addr_phys;
    struct edma_cb *edma_list_virt;

    struct semaphore dma_read_semaphore;
    struct semaphore dma_write_semaphore;

    struct pcie_dma_transfer_info dma_transfer_info;
};

struct pcie_info *g_pcie_info = NULL;
static void*    g_pcie_test_base_buffer_addr = (void*)PCIE_TEST_BUFFER_ADDR;
static u32      g_pcie_test_base_buffer_size = PCIE_TEST_BUFFER_SIZE;



static void calc_rate(char* description,    /* print description */
                      u32 trans_count,      /* transfer count */
                      u32 block_size,       /* transfer's block size */
                      u32 start_time,       /* transfer's start time */
                      u32 end_time)         /* transfer's end time */
{
    /* HZ: system timer interrupt number per seconds */
    u32 cost_ms = (end_time - start_time) * 1000 / HZ;

    u32 mbyte_size = (trans_count * block_size) >> 20;
    u32 gbyte_size = (trans_count * block_size) >> 30;

    u32 mbit_rate = (cost_ms) ? (mbyte_size * 8 * 1000) / (cost_ms) : 0;
    u32 gbit_rate = (cost_ms) ? (mbyte_size * 8) / (cost_ms) : 0;

    printk(KERN_ALERT"%s total size: %u MB(%u GB), cost times: %u ms, rate: %u Mb/s(%u Gb/s)\n",
        description, mbyte_size, gbyte_size, cost_ms, mbit_rate, gbit_rate);
}

static u32 get_random(u32 base, u32 scope)
{
    u32 random = 0;

    if (!scope)
        return base;

    get_random_bytes((void*)&random, (int)sizeof(random));

    random = random % scope + base;

    return random;
}

static void* get_buffer(u32 buffer_size)
{
    void *result = g_pcie_test_base_buffer_addr;

    g_pcie_test_base_buffer_addr += buffer_size;

    return result;
}

static void init_buffer(void)
{
    u32 id = 0;

    for (id = 0; id < balong_pcie_num; id++) {
        g_pcie_info[id].buffer_size = g_pcie_test_base_buffer_size;

        if (g_pcie_info[id].loopback_enable) {
            g_pcie_info[id].rc_phys_addr = get_buffer(g_pcie_info[id].buffer_size);
            g_pcie_info[id].ep_phys_addr = get_buffer(g_pcie_info[id].buffer_size);
        } else {
            if (balong_pcie[id].work_mode == PCIE_WORK_MODE_RC)
                g_pcie_info[id].rc_phys_addr = get_buffer(g_pcie_info[id].buffer_size);
            else
                g_pcie_info[id].ep_phys_addr = get_buffer(g_pcie_info[id].buffer_size);
        }

        if (g_pcie_info[id].edma_list_virt)
            dma_free_coherent(NULL, g_pcie_info[id].buffer_size,
                (void*)g_pcie_info[id].edma_list_virt, g_pcie_info[id].edma_list_addr_phys);

        g_pcie_info[id].edma_list_virt = (struct edma_cb *)dma_alloc_coherent(NULL,
            (g_pcie_info[id].buffer_size + SZ_32K - 1) / SZ_32K,
            &g_pcie_info[id].edma_list_addr_phys, GFP_KERNEL);
    }
}

static irqreturn_t msi_intx_isr(int irq, void* dev_info)
{
    struct balong_pcie_info *info = (struct balong_pcie_info *)dev_info;

    g_pcie_info[info->id].msi_intx_interrupt_recieve_count++;

    hi_pcie_clear_int_msi(info->virt_sc_addr, info->id);

    printk(KERN_ALERT"received msi/intx irq %d: send count %u, recieve count %u\n", irq,
                      g_pcie_info[info->id].msi_intx_interrupt_send_count,
                      g_pcie_info[info->id].msi_intx_interrupt_recieve_count);

    return IRQ_HANDLED;
}

static void dma_callback(u32 direction, u32 status, void* dev_info)
{
    struct balong_pcie_info *info = (struct balong_pcie_info *)dev_info;

    if (status) {
        g_pcie_info[info->id].dma_interrupt_abort_number++;
        pcie_trace("DMA transfer abort, status: 0x%08X\n", status);
    } else {
        g_pcie_info[info->id].dma_interrupt_done_number++;
    }

    /* the read/write semaphore is for calc rate */
    if (PCIE_DMA_DIRECTION_READ == direction)
        up(&g_pcie_info[info->id].dma_read_semaphore);
    else
        up(&g_pcie_info[info->id].dma_write_semaphore);
}

static void edma_tx_callback(u32 id, u32 status)
{
    up(&g_pcie_info[id].dma_write_semaphore);
    if (status & (EDMA_INT_CONFIG_ERR | EDMA_INT_TRANSFER_ERR | EDMA_INT_READ_ERR))
        pcie_trace("DMA transfer abort, status: 0x%08X\n", status);
}

static void edma_rx_callback(u32 id, u32 status)
{
    up(&g_pcie_info[id].dma_read_semaphore);
    if (status & (EDMA_INT_CONFIG_ERR | EDMA_INT_TRANSFER_ERR | EDMA_INT_READ_ERR))
        pcie_trace("DMA transfer abort, status: 0x%08X\n", status);
}

/* Attention: the dbi must be enabled when this function is called */
u32 get_iatu_index(u32 id, u32 direction)
{
    u32 index = 0;
    struct balong_pcie_iatu_table iatu_table;

    if (direction == PCIE_TLP_DIRECTION_INBOUND)
        index |= 0x80000000;

    while (1) {
        writel(index, balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x200);

        iatu_table.control2.value = readl(balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x208);

        if (!iatu_table.control2.attr.enable)
            break;

        index++;
    }

    return index;
}

static int rc_to_ep_by_cpu(u32 id, void* destination, void* source, u32 size)
{
    pcie_app_req_clk(id, 1);
    memcpy(destination, source, size);
    pcie_app_req_clk(id, 0);

    return 0;
}

static int ep_to_rc_by_cpu(u32 id, void* destination, void* source, u32 size)
{
    pcie_app_req_clk(id, 1);
    memcpy(destination, source, size);
    pcie_app_req_clk(id, 0);

    return 0;
}

static int rc_to_ep_by_dma(u32 id, void* destination, void* source, u32 size)
{
    u32 i = 0;
    int ret = 0;
    u32 node_size = 0;
    int edma_channel_id = 0;
    static struct edma_cb *edma_list_node_temp;

    if (g_pcie_info[id].dma_transfer_info.dev) {
        g_pcie_info[id].dma_transfer_info.direction = PCIE_DMA_DIRECTION_WRITE;
        g_pcie_info[id].dma_transfer_info.dar_high = 0;
        g_pcie_info[id].dma_transfer_info.dar_low = (u32)destination;
        g_pcie_info[id].dma_transfer_info.sar_high = 0;
        g_pcie_info[id].dma_transfer_info.sar_low = (u32)source;
        g_pcie_info[id].dma_transfer_info.transfer_size = size;
        g_pcie_info[id].dma_transfer_info.callback = dma_callback;
        g_pcie_info[id].dma_transfer_info.callback_args = (void*)&balong_pcie[id];

        ret = bsp_pcie_dma_transfer(&g_pcie_info[id].dma_transfer_info);
        if (!ret)
            while (down_interruptible(&g_pcie_info[id].dma_write_semaphore)) ;
    } else {
        edma_list_node_temp = g_pcie_info[id].edma_list_virt;
        for (i = 0; size > 0; i++) {
            node_size = size >= SZ_32K ? SZ_32K : size;
            edma_list_node_temp->config = 0xCFF33001;
            edma_list_node_temp->src_addr = (u32)source + i * SZ_32K;
            edma_list_node_temp->des_addr = (u32)destination + i * SZ_32K;
            edma_list_node_temp->cnt0  = node_size;
            edma_list_node_temp->bindx = 0;
            edma_list_node_temp->cindx = 0;
            edma_list_node_temp->cnt1  = 0;

            size -= node_size;
            edma_list_node_temp->lli = mdrv_edma_set_lli((g_pcie_info[id].edma_list_addr_phys +
                (i+1) * sizeof(struct edma_cb)),
                (size == 0 ? 1 : 0));
            edma_list_node_temp++;
        }

        edma_channel_id = bsp_edma_channel_init(EDMA_PCIE0_M2M_TX + id*2, edma_tx_callback, id, EDMA_INT_ALL);
        if (edma_channel_id < 0) {
            pcie_trace("fail to init EDMA channel, ret = %d\n", edma_channel_id);
            return ret;
        }

        edma_list_node_temp = bsp_edma_channel_get_lli_addr((u32)edma_channel_id);
        if (!edma_list_node_temp) {
            pcie_trace("fail to get EDMA lli addr\n");
            ret = -1;
            goto error_get_lli_addr;
        }
        edma_list_node_temp->lli = g_pcie_info[id].edma_list_virt->lli;
        edma_list_node_temp->config = g_pcie_info[id].edma_list_virt->config & 0xFFFFFFFE;
        edma_list_node_temp->src_addr = g_pcie_info[id].edma_list_virt->src_addr;
        edma_list_node_temp->des_addr = g_pcie_info[id].edma_list_virt->des_addr;
        edma_list_node_temp->cnt0 = g_pcie_info[id].edma_list_virt->cnt0;
        edma_list_node_temp->bindx = 0;
        edma_list_node_temp->cindx = 0;
        edma_list_node_temp->cnt1  = 0;

        ret = bsp_edma_channel_lli_async_start((u32)edma_channel_id);
        if (ret) {
            pcie_trace("fail to start EDMA channel, ret = %d\n", ret);
            goto error_get_lli_addr;
        }

        while (down_interruptible(&g_pcie_info[id].dma_write_semaphore)) ;

error_get_lli_addr:
        bsp_edma_channel_free((u32)edma_channel_id);
    }

    return ret;
}

static int ep_to_rc_by_dma(u32 id, void* destination, void* source, u32 size)
{
    u32 i = 0;
    int ret = 0;
    u32 node_size = 0;
    int edma_channel_id = 0;
    static struct edma_cb *edma_list_node_temp;

    if (g_pcie_info[id].dma_transfer_info.dev) {
        g_pcie_info[id].dma_transfer_info.direction = PCIE_DMA_DIRECTION_READ;
        g_pcie_info[id].dma_transfer_info.dar_high = 0;
        g_pcie_info[id].dma_transfer_info.dar_low = (u32)destination;
        g_pcie_info[id].dma_transfer_info.sar_high = 0;
        g_pcie_info[id].dma_transfer_info.sar_low = (u32)source;
        g_pcie_info[id].dma_transfer_info.transfer_size = size;
        g_pcie_info[id].dma_transfer_info.callback = dma_callback;
        g_pcie_info[id].dma_transfer_info.callback_args = (void*)&balong_pcie[id];

        ret = bsp_pcie_dma_transfer(&g_pcie_info[id].dma_transfer_info);

        if (!ret)
            while (down_interruptible(&g_pcie_info[id].dma_read_semaphore)) ;
    } else {
        edma_list_node_temp = g_pcie_info[id].edma_list_virt;
        for (i = 0; size > 0; i++) {
            node_size = size >= SZ_32K ? SZ_32K : size;
            edma_list_node_temp->config = 0xCFF33001;
            edma_list_node_temp->src_addr = (u32)source + i * SZ_32K;
            edma_list_node_temp->des_addr = (u32)destination + i * SZ_32K;
            edma_list_node_temp->cnt0  = node_size;
            edma_list_node_temp->bindx = 0;
            edma_list_node_temp->cindx = 0;
            edma_list_node_temp->cnt1  = 0;

            size -= node_size;
            edma_list_node_temp->lli = mdrv_edma_set_lli((g_pcie_info[id].edma_list_addr_phys +
                (i+1) * sizeof(struct edma_cb)),
                (size == 0 ? 1 : 0));
            edma_list_node_temp++;
        }

        edma_channel_id = bsp_edma_channel_init(EDMA_PCIE0_M2M_RX + id*2, edma_rx_callback, id, EDMA_INT_ALL);
        if (edma_channel_id < 0) {
            pcie_trace("fail to init EDMA channel, ret = %d\n", edma_channel_id);
            return ret;
        }

        edma_list_node_temp = bsp_edma_channel_get_lli_addr((u32)edma_channel_id);
        if (!edma_list_node_temp) {
            pcie_trace("fail to get EDMA lli addr\n");
            ret = -1;
            goto error_get_lli_addr;
        }
        edma_list_node_temp->lli = g_pcie_info[id].edma_list_virt->lli;
        edma_list_node_temp->config = g_pcie_info[id].edma_list_virt->config & 0xFFFFFFFE;
        edma_list_node_temp->src_addr = g_pcie_info[id].edma_list_virt->src_addr;
        edma_list_node_temp->des_addr = g_pcie_info[id].edma_list_virt->des_addr;
        edma_list_node_temp->cnt0 = g_pcie_info[id].edma_list_virt->cnt0;
        edma_list_node_temp->bindx = 0;
        edma_list_node_temp->cindx = 0;
        edma_list_node_temp->cnt1  = 0;

        ret = bsp_edma_channel_lli_async_start((u32)edma_channel_id);
        if (ret) {
            pcie_trace("fail to start EDMA channel, ret = %d\n", ret);
            goto error_get_lli_addr;
        }

        while (down_interruptible(&g_pcie_info[id].dma_read_semaphore)) ;

error_get_lli_addr:
        bsp_edma_channel_free((u32)edma_channel_id);
    }

    return ret;
}

int pcie_send_interrupt(u32 id)
{
    if (id >= balong_pcie_num) {
        pcie_trace("controller id error, id %u\n", id);
        return -1;
    }

    if (!balong_pcie[id].linked) {
        pcie_trace("controller %u is link down\n", id);
        return -1;
    }

    hi_pcie_trigger_int_msi(balong_pcie[id].virt_sc_addr, id);

    return 0;
}

int rate_rc_to_ep_by_cpu(u32 id, u32 total_size, u32 block_size)
{
    u32 trans_times = 0;
    u32 start_time, end_time;

    if (id >= balong_pcie_num) {
        pcie_trace("controller id error, id %u\n", id);
        return -1;
    }

    if (!balong_pcie[id].linked) {
        pcie_trace("controller %u is link down\n", id);
        return -1;
    }

    if (block_size > g_pcie_info[id].buffer_size) {
        pcie_trace("block size is too big, the max block size is limit to 0x%08X\n", g_pcie_info[id].buffer_size);
        return -1;
    }

    start_time = jiffies;
    while (total_size >= block_size) {
        (void)rc_to_ep_by_cpu(id, g_pcie_info[id].ep_in_rc_cpu_virt_addr, g_pcie_info[id].rc_virt_addr, block_size);
        total_size -= block_size;
        trans_times++;
    }
    end_time = jiffies;

    calc_rate("Write to EP by CPU:", trans_times, block_size, start_time, end_time);

    return 0;
}

int rate_ep_to_rc_by_cpu(u32 id, u32 total_size, u32 block_size)
{
    u32 trans_times = 0;
    u32 start_time, end_time;

    if (id >= balong_pcie_num) {
        pcie_trace("controller id error, id %u\n", id);
        return -1;
    }

    if (!balong_pcie[id].linked) {
        pcie_trace("controller %u is link down\n", id);
        return -1;
    }

    if (block_size > g_pcie_info[id].buffer_size) {
        pcie_trace("block size is too big, the max block size is limit to 0x%08X\n", g_pcie_info[id].buffer_size);
        return -1;
    }

    start_time = jiffies;
    while (total_size >= block_size) {
        (void)ep_to_rc_by_cpu(id, g_pcie_info[id].rc_virt_addr, g_pcie_info[id].ep_in_rc_cpu_virt_addr, block_size);
        total_size -= block_size;
        trans_times++;
    }
    end_time = jiffies;

    calc_rate("Read from EP by CPU:", trans_times, block_size, start_time, end_time);

    return 0;
}

int rate_rc_to_ep_by_dma(u32 id, u32 total_size, u32 block_size)
{
    int ret = 0;
    u32 trans_times = 0;
    u32 start_time, end_time;

    if (id >= balong_pcie_num) {
        pcie_trace("controller id error, id %u\n", id);
        return -1;
    }

    if (!balong_pcie[id].linked) {
        pcie_trace("controller %u is link down\n", id);
        return -1;
    }

    if (block_size > g_pcie_info[id].buffer_size) {
        pcie_trace("block size is too big, the max block size is limit to 0x%08X\n", g_pcie_info[id].buffer_size);
        return -1;
    }

    start_time = jiffies;
    while (total_size >= block_size) {
        ret = rc_to_ep_by_dma(id, g_pcie_info[id].ep_in_rc_pci_phys_addr, g_pcie_info[id].rc_phys_addr, block_size);
        if (ret)
            return ret;
        total_size -= block_size;
        trans_times++;
    }
    end_time = jiffies;

    calc_rate("Write to EP by DMA:", trans_times, block_size, start_time, end_time);

    return 0;
}

int rate_ep_to_rc_by_dma(u32 id, u32 total_size, u32 block_size)
{
    int ret = 0;
    u32 trans_times = 0;
    u32 start_time, end_time;

    if (id >= balong_pcie_num) {
        pcie_trace("controller id error, id %u\n", id);
        return -1;
    }

    if (!balong_pcie[id].linked) {
        pcie_trace("controller %u is link down\n", id);
        return -1;
    }

    if (block_size > g_pcie_info[id].buffer_size) {
        pcie_trace("block size is too big, the max block size is limit to 0x%08X\n", g_pcie_info[id].buffer_size);
        return -1;
    }

    start_time = jiffies;
    while (total_size >= block_size) {
        ret = ep_to_rc_by_dma(id, g_pcie_info[id].rc_phys_addr, g_pcie_info[id].ep_in_rc_pci_phys_addr, block_size);
        if (ret)
            return ret;
        total_size -= block_size;
        trans_times++;
    }
    end_time = jiffies;

    calc_rate("Read from EP by DMA:", trans_times, block_size, start_time, end_time);

    return 0;
}

int pcie_random_rc_buffer(u32 id)
{
    if (id >= balong_pcie_num) {
        pcie_trace("controller id error, id %u\n", id);
        return -1;
    }

    get_random_bytes(g_pcie_info[id].rc_virt_addr, (int)g_pcie_info[id].buffer_size);

    return 0;
}

int pcie_set_id(u32 id, u32 rc_vendor_id, u32 rc_device_id, u32 ep_vendor_id, u32 ep_device_id)
{
    if (id >= balong_pcie_num) {
        pcie_trace("controller id error, id %u\n", id);
        return -1;
    }

    g_pcie_info[id].rc_vendor_id = rc_vendor_id & 0x0000FFFF;
    g_pcie_info[id].rc_device_id = rc_device_id & 0x0000FFFF;
    g_pcie_info[id].ep_vendor_id = ep_vendor_id & 0x0000FFFF;
    g_pcie_info[id].ep_device_id = ep_device_id & 0x0000FFFF;

    return 0;
}

/* dma_id   0 -- local inner DMA
            1 -- remote inner DMA
            2 -- EDMA
 */
int pcie_set_dma(u32 id, u32 dma_id, u32 channel)
{
    if (id >= balong_pcie_num) {
        pcie_trace("controller id error, id %u\n", id);
        return -1;
    }

    if (!g_pcie_info[id].balong_ep && (1 == dma_id)) {
        pcie_trace("the EP doesn't exists, force to use local DMA\n");
        dma_id = 0;
    }

    if (0 == dma_id)
        g_pcie_info[id].dma_transfer_info.dev = g_pcie_info[id].balong_rc;
    else if (1 == dma_id)
        g_pcie_info[id].dma_transfer_info.dev = g_pcie_info[id].balong_ep;
    else
        g_pcie_info[id].dma_transfer_info.dev = NULL;

    g_pcie_info[id].dma_transfer_info.channel = channel;

    return 0;
}

int pcie_set_buffer(u32 id, void* rc_addr, void* ep_addr, u32 size)
{
    if (id >= balong_pcie_num) {
        pcie_trace("controller id error, id %u\n", id);
        return -1;
    }

    if (g_pcie_info[id].edma_list_virt)
        dma_free_coherent(NULL, g_pcie_info[id].buffer_size,
            (void*)g_pcie_info[id].edma_list_virt, g_pcie_info[id].edma_list_addr_phys);

    g_pcie_info[id].edma_list_virt = (struct edma_cb *)dma_alloc_coherent(NULL,
        (size + SZ_32K - 1) / SZ_32K, &g_pcie_info[id].edma_list_addr_phys, GFP_KERNEL);

    g_pcie_info[id].rc_phys_addr = rc_addr;
    g_pcie_info[id].ep_phys_addr = ep_addr;
    g_pcie_info[id].buffer_size  = size;

    return 0;
}

int pcie_set_base_buffer(void* base_buffer_addr, u32 base_buffer_size)
{
    g_pcie_test_base_buffer_addr = base_buffer_addr;
    g_pcie_test_base_buffer_size = base_buffer_size;

    init_buffer();

    printk(KERN_ALERT"buffer address %p, size: %u\n",
        g_pcie_test_base_buffer_addr, g_pcie_test_base_buffer_size);

    return 0;
}

int pcie_set_interrupt_mode(u32 id, u32 interrupt_mode)
{
    int ret = 0;

    if (id >= balong_pcie_num) {
        pcie_trace("controller id error, id %u\n", id);
        return -1;
    }

#if !defined(CONFIG_PCI_MSI)
    ret = -1;
    pcie_trace("the MSI is not enable, force to use INTX\n");
#else
    if (interrupt_mode <= PCIE_TEST_INT_MODE_MAX)
        g_pcie_info[id].interrupt_mode = interrupt_mode;
    else {
        ret = -1;
        pcie_trace("invalid interrupt mode: 0x%X\n", interrupt_mode);
    }
#endif

    return ret;
}

struct pci_dev* pcie_get_rc(u32 id, u32 vendor_id, u32 device_id)
{
    struct pci_dev *pdev = NULL;

    do {
        pdev = pci_get_device(vendor_id, device_id, pdev);
        if (pdev && ((pdev->class >> 8) == PCI_CLASS_BRIDGE_PCI) &&
            (((struct balong_pcie_info *)((struct pci_sys_data *)pdev->bus->sysdata)->private_data)->id == id))
            break;
    } while (pdev != NULL);

    return pdev;
}

struct pci_dev* pcie_get_dev(u32 id, u32 vendor_id, u32 device_id)
{
    struct pci_dev *pdev = NULL;

    do {
        pdev = pci_get_device(vendor_id, device_id, pdev);
        if (pdev && ((pdev->class >> 8) != PCI_CLASS_BRIDGE_PCI) &&
            (((struct balong_pcie_info *)((struct pci_sys_data *)pdev->bus->sysdata)->private_data)->id == id))
            break;
    } while (pdev != NULL);

    return pdev;
}

static void pcie_test_int_init(u32 id)
{
    u32 i = 0;
    int ret = 0;
    struct msix_entry entries[1];

    for (i = 0; i < sizeof(entries)/sizeof(entries[0]); i++)
        entries[i].entry = i;

    switch (g_pcie_info[id].interrupt_mode) {
        case PCIE_TEST_INT_MODE_INTX:
            if (request_irq(g_pcie_info[id].balong_ep->irq, msi_intx_isr, IRQF_SHARED,
                            "PCIe INTX", (void*)&balong_pcie[id])) {
                pcie_trace("request_irq fail, msi irq = %d\n", g_pcie_info[id].balong_ep->irq);
                return;
            }

            break;
        case PCIE_TEST_INT_MODE_MSI:
            ret = pci_enable_msi(g_pcie_info[id].balong_ep);
            if (ret != 0) {
                pcie_trace("pci_enable_msi fail, ret = %d\n", ret);
                return;
            }

            if (request_irq(g_pcie_info[id].balong_ep->irq, msi_intx_isr, IRQF_SHARED,
                            "PCIe MSI", (void*)&balong_pcie[id])) {
                pcie_trace("request_irq fail, msi irq = %d\n", g_pcie_info[id].balong_ep->irq);
                return;
            }

            break;
        case PCIE_TEST_INT_MODE_MSIX:
            ret = pci_enable_msix(g_pcie_info[id].balong_ep, &entries[0], sizeof(entries)/sizeof(entries[0]));
            if (ret != 0) {
                pcie_trace("pci_enable_msix fail, ret = %d\n", ret);
                return;
            }

            for (i = 0; i < sizeof(entries)/sizeof(entries[0]); i++) {
                if (request_irq(entries[i].vector, msi_intx_isr, IRQF_SHARED,
                                "PCIe MSI", (void*)&balong_pcie[id])) {
                    pcie_trace("request_irq fail, msi irq = %d\n", entries[i].vector);
                    return;
                }
            }

            break;
    }
}

static void pcie_test_rc_init(u32 id)
{
    /* EP buffer's CPU address */
    void* cpu_phys_start_addr = 0;
    void* cpu_virt_start_addr = 0;
    u32 ep_buffer_size = 0;

    sema_init(&g_pcie_info[id].dma_read_semaphore, 0);
    sema_init(&g_pcie_info[id].dma_write_semaphore, 0);

    g_pcie_info[id].balong_rc = pcie_get_rc(id, g_pcie_info[id].rc_vendor_id, g_pcie_info[id].rc_device_id);
    if(NULL == g_pcie_info[id].balong_rc) {
        pcie_trace("can't find balong pcie rc\n");
        return;
    }
    g_pcie_info[id].dma_transfer_info.dev = g_pcie_info[id].balong_rc;

    g_pcie_info[id].balong_ep = pcie_get_dev(id, g_pcie_info[id].ep_vendor_id, g_pcie_info[id].ep_device_id);
    if(NULL == g_pcie_info[id].balong_ep) {
        pcie_trace("can't find balong pcie ep\n");
        return;
    }

    if (pci_enable_device(g_pcie_info[id].balong_ep)) {
        pcie_trace("pci_enable_device fail\n");
        return;
    }

    if (pci_enable_pcie_error_reporting(g_pcie_info[id].balong_ep)) {
        pcie_trace("pci_enable_pcie_error_reporting fail\n");
        return;
    }

    pci_set_master(g_pcie_info[id].balong_ep);

    cpu_phys_start_addr = (void*)pci_resource_start(g_pcie_info[id].balong_ep, 0);
    if (!cpu_phys_start_addr) {
        pcie_trace("pci_resource_start fail\n");
        return;
    }
    pcie_trace("cpu_phys_start_addr: 0x%08X\n", (u32)cpu_phys_start_addr);

    /* pci_resource_len = pci_resource_end - pci_resource_start + 1 */
    ep_buffer_size = pci_resource_len(g_pcie_info[id].balong_ep, 0);
    if (!ep_buffer_size) {
        pcie_trace("pci_resource_len fail\n");
        return;
    }
    pcie_trace("ep_buffer_size: 0x%08X\n", ep_buffer_size);

    cpu_virt_start_addr = ioremap((u32)cpu_phys_start_addr, ep_buffer_size);
    if (!cpu_virt_start_addr) {
        pcie_trace("fail to ioremap, addr: 0x%08X\n", (u32)cpu_phys_start_addr);
        return;
    }
    pcie_trace("cpu_virt_start_addr: 0x%08X\n", (u32)cpu_virt_start_addr);

    g_pcie_info[id].ep_in_rc_cpu_phys_addr = cpu_phys_start_addr;
    g_pcie_info[id].ep_in_rc_cpu_virt_addr = cpu_virt_start_addr;

    /* in normal mode, pci domain address == cpu domain address */
    g_pcie_info[id].ep_in_rc_pci_phys_addr = g_pcie_info[id].ep_in_rc_cpu_phys_addr;

    cpu_phys_start_addr = (void*)pci_resource_start(g_pcie_info[id].balong_ep, 2);
    if (!cpu_phys_start_addr) {
        pcie_trace("pci_resource_start fail\n");
        return;
    }
    pcie_trace("cpu_phys_start_addr: 0x%08X\n", (u32)cpu_phys_start_addr);

    /* pci_resource_len = pci_resource_end - pci_resource_start + 1 */
    ep_buffer_size = pci_resource_len(g_pcie_info[id].balong_ep, 2);
    if (!ep_buffer_size) {
        pcie_trace("pci_resource_len fail\n");
        return;
    }
    pcie_trace("ep_buffer_size: 0x%08X\n", ep_buffer_size);

    cpu_virt_start_addr = ioremap((u32)cpu_phys_start_addr, ep_buffer_size);
    if (!cpu_virt_start_addr) {
        pcie_trace("fail to ioremap, addr: 0x%08X\n", (u32)cpu_phys_start_addr);
        return;
    }
    pcie_trace("cpu_virt_start_addr: 0x%08X\n", (u32)cpu_virt_start_addr);

    g_pcie_info[id].ep_in_rc_cpu_sc_phys_addr = cpu_phys_start_addr;
    g_pcie_info[id].ep_in_rc_cpu_sc_virt_addr = cpu_virt_start_addr;

    pcie_test_int_init(id);

    g_pcie_info[id].rc_virt_addr = (void*)ioremap_nocache((u32)g_pcie_info[id].rc_phys_addr, g_pcie_info[id].buffer_size);
    if (!g_pcie_info[id].rc_virt_addr) {
        pcie_trace("fail to ioremap, addr: 0x%08X\n", (u32)g_pcie_info[id].rc_phys_addr);
        return;
    }
    pcie_trace("g_pcie_rc_virt_addr: 0x%08X, g_pcie_rc_phys_addr: 0x%08X\n",
        (u32)g_pcie_info[id].rc_virt_addr, (u32)g_pcie_info[id].rc_phys_addr);

    get_random_bytes(g_pcie_info[id].rc_virt_addr, (int)g_pcie_info[id].buffer_size);
}

static void pcie_test_ep_init(u32 id)
{
    unsigned long irq_flags= 0;

    struct balong_pcie_iatu_table iatu_table[] = {
        {
            .index.attr.index = 0x0,
            .index.attr.direction = PCIE_TLP_DIRECTION_INBOUND,
            .control1.attr.type = PCIE_TLP_TYPE_MEM_RW,
            .control2.attr.message_code = 0,
            .control2.attr.bar_index    = 0,
            .control2.attr.reserved     = 0,
            .control2.attr.cfg_shift    = 0,
            .control2.attr.invert_mode  = 0,
            .control2.attr.match_mode   = PCIE_IATU_MATCH_MODE_BAR,
            .control2.attr.enable       = 1,
            .lower_addr = 0,
            .upper_addr = 0,
            .limit_addr = 0,
            .lower_target_addr = 0,
            .upper_target_addr = 0,
            .control3 = 0,
        },
        {
            .index.attr.index = 0x0,
            .index.attr.direction = PCIE_TLP_DIRECTION_INBOUND,
            .control1.attr.type = PCIE_TLP_TYPE_MEM_RW,
            .control2.attr.message_code = 0,
            .control2.attr.bar_index    = 2,
            .control2.attr.reserved     = 0,
            .control2.attr.cfg_shift    = 0,
            .control2.attr.invert_mode  = 0,
            .control2.attr.match_mode   = PCIE_IATU_MATCH_MODE_BAR,
            .control2.attr.enable       = 1,
            .lower_addr = 0,
            .upper_addr = 0,
            .limit_addr = 0,
            .lower_target_addr = 0,
            .upper_target_addr = 0,
            .control3 = 0,
        },
    };

    pcie_app_req_clk(id, 1);

    spin_lock_irqsave(&balong_pcie[id].spinlock, irq_flags);
    dbi_enable(id);

    iatu_table[0].index.value = get_iatu_index(id, PCIE_TLP_DIRECTION_INBOUND);
    iatu_table[0].lower_target_addr = (u32)g_pcie_info[id].ep_phys_addr;

    iatu_table[1].index.value = iatu_table[0].index.value + 1;
    iatu_table[1].lower_target_addr = balong_pcie[id].phys_sc_addr;

    pcie_set_iatu(id, &iatu_table[0], sizeof(iatu_table)/sizeof(iatu_table[0]));

    dbi_disable(id);
    spin_unlock_irqrestore(&balong_pcie[id].spinlock, irq_flags);

    pcie_app_req_clk(id, 0);
}

int pcie_test_init(void)
{
    u32 id = 0;
    u32 init_mode;

    static u32 is_inited = 0;

    if (!g_pcie_test_base_buffer_addr) {
        pcie_trace("error buffer addr: 0x%08X\n", (u32)g_pcie_test_base_buffer_addr);
        return -1;
    }

    /* there are two loops, the first one is for EP and the second one is for RC.
     * we must init EP at first to ensure that the EP is ready when the RC start the ltssm.
     */
    init_mode = PCIE_WORK_MODE_EP;
    for (id = 0; id < balong_pcie_num; id++) {
        if (!balong_pcie[id].enabled)
            continue;

        if (balong_pcie[id].work_mode != init_mode)
            continue;

        pcie_test_ep_init(id);
    }

    /* the ep can be inited for multiple times */
    if (is_inited)
        return 0;

    is_inited = 1;

    init_mode = PCIE_WORK_MODE_RC;
    for (id = 0; id < balong_pcie_num; id++) {
        if (!balong_pcie[id].enabled)
            continue;

        if (balong_pcie[id].work_mode != init_mode)
            continue;

        pcie_test_rc_init(id);
    }

    return 0;
}


#define PCIE_LOOPBACK_PCI_DOMAIN_ADDRESS    (0x60000000)

int pcie_loopback_init(void)
{
    u32 id = 0;
    u32 value = 0;
    unsigned long irq_flags= 0;

    struct balong_pcie_iatu_table iatu_table[] = {
        {
            .index.attr.index = 0x0,
            .index.attr.direction = PCIE_TLP_DIRECTION_OUTBOUND,
            .control1.attr.type = PCIE_TLP_TYPE_MEM_RW,
            .control2.attr.message_code = 0,
            .control2.attr.bar_index    = 0,
            .control2.attr.reserved     = 0,
            .control2.attr.cfg_shift    = 0,
            .control2.attr.invert_mode  = 0,
            .control2.attr.match_mode   = PCIE_IATU_MATCH_MODE_ADDR,
            .control2.attr.enable       = 1,
            .lower_addr = 0,
            .upper_addr = 0,
            .limit_addr = 0,
            .lower_target_addr = PCIE_LOOPBACK_PCI_DOMAIN_ADDRESS,
            .upper_target_addr = 0,
            .control3 = 0,
        },
        {
            .index.attr.index =    0x0,
            .index.attr.direction = PCIE_TLP_DIRECTION_INBOUND,
            .control1.attr.type = PCIE_TLP_TYPE_MEM_RW,
            .control2.attr.message_code = 0,
            .control2.attr.bar_index    = 0,
            .control2.attr.reserved     = 0,
            .control2.attr.cfg_shift    = 0,
            .control2.attr.invert_mode  = 0,
            .control2.attr.match_mode   = PCIE_IATU_MATCH_MODE_ADDR,
            .control2.attr.enable       = 1,
            .lower_addr = PCIE_LOOPBACK_PCI_DOMAIN_ADDRESS,
            .upper_addr = 0,
            .lower_target_addr = 0,
            .upper_target_addr = 0,
            .control3 = 0,
        },
    };

    if (!g_pcie_test_base_buffer_addr) {
        pcie_trace("error buffer addr: 0x%08X\n", (u32)g_pcie_test_base_buffer_addr);
        return -1;
    }

    for (id = 0; id < balong_pcie_num; id++) {
        g_pcie_info[id].loopback_enable = 1;
    }

    init_buffer();

    for (id = 0; id < balong_pcie_num; id++) {
        if (!balong_pcie[id].enabled)
            continue;

        sema_init(&g_pcie_info[id].dma_read_semaphore, 0);
        sema_init(&g_pcie_info[id].dma_write_semaphore, 0);

        g_pcie_info[id].balong_rc = pcie_get_rc(id, g_pcie_info[id].rc_vendor_id, g_pcie_info[id].rc_device_id);
        if (NULL == g_pcie_info[id].balong_rc) {
            pcie_trace("can't find balong pcie rc\n");
            return -1;
        }
        g_pcie_info[id].dma_transfer_info.dev = g_pcie_info[id].balong_rc;

        g_pcie_info[id].rc_virt_addr = (void*)ioremap_nocache((u32)g_pcie_info[id].rc_phys_addr, g_pcie_info[id].buffer_size);
        if (!g_pcie_info[id].rc_virt_addr) {
            pcie_trace("fail to ioremap, addr: 0x%08X\n", (u32)g_pcie_info[id].rc_phys_addr);
            return -1;
        }
        pcie_trace("g_pcie_rc_virt_addr: 0x%08X, g_pcie_rc_phys_addr: 0x%08X\n",
            (u32)g_pcie_info[id].rc_virt_addr, (u32)g_pcie_info[id].rc_phys_addr);

        get_random_bytes(g_pcie_info[id].rc_virt_addr, (int)g_pcie_info[id].buffer_size);

        g_pcie_info[id].ep_virt_addr = (void*)ioremap_nocache((u32)g_pcie_info[id].ep_phys_addr, g_pcie_info[id].buffer_size);
        if (!g_pcie_info[id].ep_virt_addr) {
            pcie_trace("fail to ioremap, addr: 0x%08X\n", (u32)g_pcie_info[id].ep_phys_addr);
            return -1;
        }
        pcie_trace("g_pcie_ep_virt_addr: 0x%08X, g_pcie_ep_phys_addr: 0x%08X\n",
            (u32)g_pcie_info[id].ep_virt_addr, (u32)g_pcie_info[id].ep_phys_addr);

        get_random_bytes(g_pcie_info[id].ep_virt_addr, (int)g_pcie_info[id].buffer_size);

        g_pcie_info[id].ep_in_rc_cpu_phys_addr = (void*)balong_pcie[id].res_mem.start;
        g_pcie_info[id].ep_in_rc_cpu_virt_addr = (void*)ioremap_nocache((u32)g_pcie_info[id].ep_in_rc_cpu_phys_addr, g_pcie_info[id].buffer_size);
        if (!g_pcie_info[id].ep_in_rc_cpu_virt_addr) {
            pcie_trace("fail to ioremap, addr: 0x%08X\n", (u32)g_pcie_info[id].ep_in_rc_cpu_phys_addr);
            return -1;
        }
        pcie_trace("g_pcie_ep_in_rc_cpu_virt_addr: 0x%08X, g_pcie_ep_in_rc_cpu_phys_addr: 0x%08X\n",
            (u32)g_pcie_info[id].ep_in_rc_cpu_virt_addr, (u32)g_pcie_info[id].ep_in_rc_cpu_phys_addr);

        g_pcie_info[id].ep_in_rc_pci_phys_addr = (void*)PCIE_LOOPBACK_PCI_DOMAIN_ADDRESS;

        spin_lock_irqsave(&balong_pcie[id].spinlock, irq_flags);
        dbi_enable(id);

        /* disable Gen3 equalization feature */
        value = readl(balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x190);
        value &= (~(0x1<<16));
        writel(value, balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x190);

        /* enable PIPE loopback */
        value = readl(balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x1B8);
        value |= (0x1<<31);
        writel(value, balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x1B8);

        /* enable loopback */
        value = readl(balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x010);
        value |= (0x1<<2);
        writel(value, balong_pcie[id].virt_rc_cfg_addr + 0x700 + 0x010);

        /* set bar0 size */
        writel(g_pcie_info[id].buffer_size - 1, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_0));
        /* disable other bar */
        writel(0, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_1));
        writel(0, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_2));
        writel(0, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_3));
        writel(0, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_4));
        writel(0, balong_pcie[id].virt_rc_cfg_addr + ((0x1<<12)|PCI_BASE_ADDRESS_5));

        /* modify bar to non-prefetchmem */
        writel(PCIE_LOOPBACK_PCI_DOMAIN_ADDRESS, balong_pcie[id].virt_rc_cfg_addr + PCI_BASE_ADDRESS_0);
        writel(0x0, balong_pcie[id].virt_rc_cfg_addr + PCI_BASE_ADDRESS_1);
        writel(0x0, balong_pcie[id].virt_rc_cfg_addr + PCI_BASE_ADDRESS_2);
        writel(0x0, balong_pcie[id].virt_rc_cfg_addr + PCI_BASE_ADDRESS_3);
        writel(0x0, balong_pcie[id].virt_rc_cfg_addr + PCI_BASE_ADDRESS_4);
        writel(0x0, balong_pcie[id].virt_rc_cfg_addr + PCI_BASE_ADDRESS_5);

        iatu_table[0].lower_addr = balong_pcie[id].res_mem.start;
        iatu_table[0].limit_addr = balong_pcie[id].res_mem.end;
        iatu_table[1].limit_addr = PCIE_LOOPBACK_PCI_DOMAIN_ADDRESS + g_pcie_info[id].buffer_size - 1;
        iatu_table[1].lower_target_addr = (u32)g_pcie_info[id].ep_phys_addr;

        pcie_set_iatu(id, &iatu_table[0], sizeof(iatu_table)/sizeof(iatu_table[0]));

        g_pcie_info[id].ep_in_rc_cpu_sc_phys_addr = (void*)balong_pcie[id].phys_sc_addr;
        g_pcie_info[id].ep_in_rc_cpu_sc_virt_addr = (void*)balong_pcie[id].virt_sc_addr;

        dbi_disable(id);
        spin_unlock_irqrestore(&balong_pcie[id].spinlock, irq_flags);

        balong_pcie[id].linked = 1;
    }

    return 0;
}


int pcie_test_startup_init(u32 id)
{
    int ret = 0;
    mm_segment_t old_fs;

    long file_length = 0;
    unsigned int file_handle = 0;

    void *cpu_virt_addr = NULL;
    struct pci_dev* balong_ep = NULL;

    balong_ep = pcie_get_dev(id, g_pcie_info[id].ep_vendor_id, g_pcie_info[id].ep_device_id);
    if(NULL == balong_ep) {
        pcie_trace("can't find balong pcie ep\n");
        return -1;
    }

    ret = pci_write_config_dword(balong_ep, 0x700 + 0x218, (u32)g_pcie_info[id].rc_phys_addr);
    if (ret) {
        pcie_trace("fail to write config space, ret = 0x%08X\n", ret);
        return -1;
    }

    cpu_virt_addr = (void*)ioremap_nocache((u32)g_pcie_info[id].rc_phys_addr, g_pcie_info[id].buffer_size);
    if (!cpu_virt_addr) {
        pcie_trace("fail to ioremap, addr: 0x%08X\n", (u32)g_pcie_info[id].rc_phys_addr);
        return -1;
    }

    old_fs = get_fs(); //lint !e63
    set_fs(KERNEL_DS);

    file_handle = (unsigned int)sys_open("/system/xloader.bin", O_RDONLY, 0);
    if (IS_ERR((const void*)file_handle)) {
        pcie_trace("fail to open file '/system/xloader.bin'\n");
        ret = -1;
        goto err_open_file;
    }

    do {
        ret = (int)sys_read(file_handle, (char*)cpu_virt_addr + file_length, 1024);
        if (ret < 0) {
            pcie_trace("fail to read xloader\n");
            goto err_read_file;
        }
        file_length += ret;
    } while (ret > 0);

    pcie_trace("xloader size: 0x%08X\n", (u32)file_length);

    ret = pci_enable_device(balong_ep);
    if (ret) {
        pcie_trace("pci_enable_device fail\n");
        goto err_read_file;
    }

    pci_set_master(balong_ep);

err_read_file:
    sys_close(file_handle);

err_open_file:
    set_fs(old_fs);
    iounmap(cpu_virt_addr);

    return ret;
}


static u32 g_pcie_stress_test_sleep_ms = 0;
static u32 g_pcie_stress_test_run_count = 0;
static u32 g_pcie_stress_test_task_running = 0;
struct task_struct *g_pcie_stress_test_task = NULL;


#define PCIE_STRESS_TEST_MODE_CPU_READ  (0x00000001)    /* enable CPU read */
#define PCIE_STRESS_TEST_MODE_CPU_WRITE (0x00000002)    /* enable CPU write */
#define PCIE_STRESS_TEST_MODE_DMA_READ  (0x00000004)    /* enable DMA read */
#define PCIE_STRESS_TEST_MODE_DMA_WRITE (0x00000008)    /* enable DMA write */
#define PCIE_STRESS_TEST_MODE_INTERRUPT (0x00000010)    /* enable interrupt */

#define PCIE_STRESS_TEST_MODE_RANDOM    (0x80000000)    /* enable random address and random size */

#define PCIE_STRESS_TEST_MODE_VALID    (PCIE_STRESS_TEST_MODE_CPU_READ | \
                                        PCIE_STRESS_TEST_MODE_CPU_WRITE | \
                                        PCIE_STRESS_TEST_MODE_DMA_READ | \
                                        PCIE_STRESS_TEST_MODE_DMA_WRITE | \
                                        PCIE_STRESS_TEST_MODE_INTERRUPT)

static volatile u32 g_pcie_stree_test_mode = (PCIE_STRESS_TEST_MODE_CPU_READ | \
                                              PCIE_STRESS_TEST_MODE_CPU_WRITE);

int pcie_stress_test_set_mode(u32 mode)
{
    if (!(mode & PCIE_STRESS_TEST_MODE_VALID))
        pcie_trace("[WARNING]No valid bit, valid mode is 0x%08X\n", PCIE_STRESS_TEST_MODE_VALID);

    g_pcie_stree_test_mode = mode;

    return 0;
}

int pcie_stress_test_thread(void *data)
{
    u32 id = 0;
    u32 size = 0;
    void* source = NULL;
    void* destination = NULL;

    /* the buffer of rc and ep is different,
     * we should make them the same in case of the random mode is enable,
     * otherwise the stress test result will be wrong.
     */
    for (id = 0; id < balong_pcie_num; id++) {
        if (!balong_pcie[id].enabled)
            continue;

        if (g_pcie_stree_test_mode & PCIE_STRESS_TEST_MODE_RANDOM) {
            (void)rc_to_ep_by_cpu(id, g_pcie_info[id].ep_in_rc_cpu_virt_addr, g_pcie_info[id].rc_virt_addr, g_pcie_info[id].buffer_size);
        }
    }

    while (g_pcie_stress_test_task_running) {
        for (id = 0; id < balong_pcie_num; id++) {
            if (!balong_pcie[id].enabled)
                continue;
            if (!balong_pcie[id].linked)
                continue;

            if (g_pcie_stree_test_mode & PCIE_STRESS_TEST_MODE_CPU_READ) {
                size = g_pcie_info[id].buffer_size;
                source = g_pcie_info[id].ep_in_rc_cpu_virt_addr;
                destination = g_pcie_info[id].rc_virt_addr;

                if (g_pcie_stree_test_mode & PCIE_STRESS_TEST_MODE_RANDOM) {
                    size = get_random(1, g_pcie_info[id].buffer_size - 1);
                    source += (g_pcie_info[id].buffer_size - size);
                    destination += (g_pcie_info[id].buffer_size - size);
                }

                (void)ep_to_rc_by_cpu(id, destination, source, size);
            }

            if (g_pcie_stree_test_mode & PCIE_STRESS_TEST_MODE_CPU_WRITE) {
                size = g_pcie_info[id].buffer_size;
                source = g_pcie_info[id].rc_virt_addr;
                destination = g_pcie_info[id].ep_in_rc_cpu_virt_addr;

                if (g_pcie_stree_test_mode & PCIE_STRESS_TEST_MODE_RANDOM) {
                    size = get_random(1, g_pcie_info[id].buffer_size - 1);
                    source += (g_pcie_info[id].buffer_size - size);
                    destination += (g_pcie_info[id].buffer_size - size);
                }

                (void)rc_to_ep_by_cpu(id, destination, source, size);
            }

            if (g_pcie_stree_test_mode & PCIE_STRESS_TEST_MODE_DMA_READ) {
                size = g_pcie_info[id].buffer_size;
                source = g_pcie_info[id].ep_in_rc_pci_phys_addr;
                destination = g_pcie_info[id].rc_phys_addr;

                if (g_pcie_stree_test_mode & PCIE_STRESS_TEST_MODE_RANDOM) {
                    size = get_random(1, g_pcie_info[id].buffer_size - 1);
                    source += (g_pcie_info[id].buffer_size - size);
                    destination += (g_pcie_info[id].buffer_size - size);
                }

                (void)ep_to_rc_by_dma(id, destination, source, size);
            }

            if (g_pcie_stree_test_mode & PCIE_STRESS_TEST_MODE_DMA_WRITE) {
                size = g_pcie_info[id].buffer_size;
                source = g_pcie_info[id].rc_phys_addr;
                destination = g_pcie_info[id].ep_in_rc_pci_phys_addr;

                if (g_pcie_stree_test_mode & PCIE_STRESS_TEST_MODE_RANDOM) {
                    size = get_random(1, g_pcie_info[id].buffer_size - 1);
                    source += (g_pcie_info[id].buffer_size - size);
                    destination += (g_pcie_info[id].buffer_size - size);
                }

                (void)rc_to_ep_by_dma(id, destination, source, size);
            }

            if (g_pcie_stree_test_mode & PCIE_STRESS_TEST_MODE_INTERRUPT) {
                g_pcie_info[id].msi_intx_interrupt_send_count++;
                pcie_send_interrupt(id);
            }
        }

        g_pcie_stress_test_run_count++;

        if (g_pcie_stress_test_sleep_ms)
            msleep(g_pcie_stress_test_sleep_ms);
    }

    return 0;
}

/* priority: 0   -- 99  real time
             100 -- 139 normal */
int pcie_stress_test_start(u32 priority, u32 sleep_ms)
{
    u32 id = 0;
    int policy = 0;
    struct sched_param sched_para;

    if (g_pcie_stress_test_task_running)
        return 0;

    if (priority < 100)
        policy = SCHED_FIFO;
    else {
        priority = 0;
        policy = SCHED_NORMAL;
    }

    sched_para.sched_priority = priority;
    g_pcie_stress_test_sleep_ms = sleep_ms;

    for (id = 0; id < balong_pcie_num; id++) {
        if (!balong_pcie[id].enabled)
            continue;

        g_pcie_info[id].msi_intx_interrupt_send_count = 0;
        g_pcie_info[id].msi_intx_interrupt_recieve_count = 0;
    }

    g_pcie_stress_test_run_count = 0;
    g_pcie_stress_test_task_running = 1;

    g_pcie_stress_test_task = kthread_run(pcie_stress_test_thread, NULL, "PCIe_Test");
    if (IS_ERR(g_pcie_stress_test_task)) {
        g_pcie_stress_test_task = NULL;
        printk(KERN_ALERT"kthread_run %s fail\n", "PCIe_Test");
        return -1;
    }

    if (sched_setscheduler(g_pcie_stress_test_task, policy, &sched_para)) {
        printk(KERN_ALERT"sched_setscheduler %s fail\n", "PCIe_Test");
        return -1;
    }

    return 0;
}

int pcie_stress_test_stop(void)
{
    g_pcie_stress_test_task_running = 0;

    if (!g_pcie_stress_test_task)
        return kthread_stop(g_pcie_stress_test_task);

    return 0;
}

int pcie_stress_test_count(void)
{
    if (g_pcie_stress_test_task_running)
        printk(KERN_ALERT"Running, PCIe stress test run count: %u\n", g_pcie_stress_test_run_count);
    else
        printk(KERN_ALERT"Stopped, PCIe stress test run count: %u\n", g_pcie_stress_test_run_count);

    return 0;
}

int pcie_stress_test_result(void)
{
    u32 id = 0;
    int ret = 0;
    int error = 0;

    for (id = 0; id < balong_pcie_num; id++) {
        if (!balong_pcie[id].enabled)
            continue;

        if (!balong_pcie[id].linked) {
            printk(KERN_ALERT"Controller %u is link down\n", id);
            continue;
        }

        pcie_app_req_clk(id, 1);

        ret = memcmp(g_pcie_info[id].rc_virt_addr,
                     g_pcie_info[id].ep_in_rc_cpu_virt_addr,
                     g_pcie_info[id].buffer_size);

        pcie_app_req_clk(id, 0);

        if (ret) {
            printk(KERN_ALERT"[FAIL]PCIe controller id: %u, stress test run count: %u\n",
                id, g_pcie_stress_test_run_count);
        } else {
            printk(KERN_ALERT"[PASS]PCIe controller id: %u, stress test run count: %u\n",
                id, g_pcie_stress_test_run_count);
        }

        error |= ret;
    }

    return error;
}


static int pcie_test_common_init(void)
{
    u32 id = 0;

    if (!balong_pcie_num)
        return 0;

    g_pcie_info = (struct pcie_info *)kzalloc(sizeof(struct pcie_info) * balong_pcie_num, GFP_KERNEL);
    if (!g_pcie_info) {
        pcie_trace("fail to kzalloc, size: 0x%08X\n", sizeof(struct pcie_info) * balong_pcie_num);
        return -1;
    }

    init_buffer();

    for (id = 0; id < balong_pcie_num; id++) {
        g_pcie_info[id].rc_vendor_id = balong_pcie[id].vendor_id;
        g_pcie_info[id].rc_device_id = balong_pcie[id].device_id;
        g_pcie_info[id].ep_vendor_id = balong_pcie[id].vendor_id;
        g_pcie_info[id].ep_device_id = balong_pcie[id].device_id;

        g_pcie_info[id].interrupt_mode = PCIE_TEST_INT_MODE_DEFAULT;
    }

    return 0;
}

module_init(pcie_test_common_init);


int pcie_test_info(void)
{
    u32 i = 0;

    for (i = 0; i < balong_pcie_num; i++) {
        printk(KERN_ALERT"Controller %u\n", i);
        printk(KERN_ALERT"RC's Vendor ID: 0x%04X, Device ID: 0x%04X, PCI_DEV @0x%08X\n",
            g_pcie_info[i].rc_vendor_id, g_pcie_info[i].rc_device_id, (u32)g_pcie_info[i].balong_rc);

        printk(KERN_ALERT"EP's Vendor ID: 0x%04X, Device ID: 0x%04X, PCI_DEV @0x%08X\n",
            g_pcie_info[i].ep_vendor_id, g_pcie_info[i].ep_device_id, (u32)g_pcie_info[i].balong_ep);

        printk(KERN_ALERT"PCIe Test buffer size: 0x%08X\n", g_pcie_info[i].buffer_size);

        printk(KERN_ALERT"RC buffer's CPU physical address: 0x%08X, virtual address: 0x%08X\n",
            (u32)g_pcie_info[i].rc_phys_addr, (u32)g_pcie_info[i].rc_virt_addr);

        printk(KERN_ALERT"EP buffer's CPU physical address: 0x%08X, virtual address: 0x%08X\n",
            (u32)g_pcie_info[i].ep_phys_addr, (u32)g_pcie_info[i].ep_virt_addr);

        printk(KERN_ALERT"EP buffer's PCI physical address: 0x%08X, virtual address: 0x%08X\n",
            (u32)g_pcie_info[i].ep_in_rc_cpu_phys_addr, (u32)g_pcie_info[i].ep_in_rc_cpu_virt_addr);

        printk(KERN_ALERT"EP buffer's PCI physical address in PCI domain: 0x%08X\n",
            (u32)g_pcie_info[i].ep_in_rc_pci_phys_addr);

        printk(KERN_ALERT"EP sysctrl's PCI physical address: 0x%08X, virtual address: 0x%08X\n",
            (u32)g_pcie_info[i].ep_in_rc_cpu_sc_phys_addr, (u32)g_pcie_info[i].ep_in_rc_cpu_sc_virt_addr);

        printk(KERN_ALERT"DMA interrupt done number: %u, abort number: %u\n",
            g_pcie_info[i].dma_interrupt_done_number, g_pcie_info[i].dma_interrupt_abort_number);

        if (g_pcie_info[i].loopback_enable)
            printk(KERN_ALERT"PCIe loopback is enabled\n\n");
        else
            printk(KERN_ALERT"PCIe loopback is disabled\n\n");
    }
    return 0;
}


