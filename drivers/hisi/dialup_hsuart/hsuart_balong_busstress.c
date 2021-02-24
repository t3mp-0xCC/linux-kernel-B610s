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

#include <linux/kernel.h>       /*kmalloc,printk*/
#include <linux/module.h>
#include <linux/kthread.h>      
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/skbuff.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <asm/irq.h>
#include "osl_malloc.h"
#include "osl_thread.h"
#include <bsp_edma.h>
#include <bsp_hardtimer.h>
#include "mdrv_udi.h"
#include "hi_hsuart.h"
#include "bsp_memmap.h"
#include "dialup_hsuart.h"

#define BUFF_SIZE 1024
struct hsuart_busstress_str
{  
	int irq;
    unsigned int clock;
    struct spinlock lock_irq;
    unsigned int phy_addr_base;
    volatile void __iomem * vir_addr_base;
	unsigned int reg_size;
    osl_sem_id tx_sema;
    osl_sem_id dma_sema;
	HSUART_TASK_ID rx_task_id;
	HSUART_TASK_ID tx_task_id;
	u32 init_flag;
}hs_uart_str;
u32 hsuart_vir_addr = 0;
struct _recv_buff_str
{
	volatile u32 ulread;
	volatile u32 ulwrite;
	u8 uldata[BUFF_SIZE];
}recv_buff_str;

void send4(u32 * pucSrcAddr ,u32 ulTimes)
{
    u32 regval;
    u32 *pu32Buffer;

    pu32Buffer = pucSrcAddr;

    while(ulTimes)
    {
        regval = readl(hsuart_vir_addr + HSUART_REGOFF_USR);
        if (0 != (regval & 0x02))
        {
            *(UINT32 *)(hsuart_vir_addr + HSUART_REGOFF_THR) = *pu32Buffer;
            pu32Buffer++;
        	ulTimes--;
        }
    }
    return;
}

void send2(u16 *pucSrcAddr ,u32 ulTimes)
{
    u32 regval;
    u16 *pu16Buffer;

    pu16Buffer = pucSrcAddr;

    while(ulTimes)
    {
        regval = readl(hsuart_vir_addr + HSUART_REGOFF_USR);
        if (0 != (regval & 0x02))
        {
            *(u16 *)(hsuart_vir_addr + HSUART_REGOFF_THR) = *pu16Buffer;
            pu16Buffer++;
        	ulTimes--;
        }
    }
    return;
}

void send1(u8 * pucSrcAddr ,u32 ulTimes)
{
    u32 regval;
    u8 * pu8Buffer;

    pu8Buffer = pucSrcAddr;

    while(ulTimes)
    {
        regval = readl(hsuart_vir_addr + HSUART_REGOFF_USR);
        if (0 != (regval & 0x02))
        {
            *(u8 *)(hsuart_vir_addr + HSUART_REGOFF_THR) = *pu8Buffer;
            pu8Buffer++;
        	ulTimes--;
        }
    }
    return;
}

s32 hsuart_send(u8 * pucSrcAddr, u32 u32TransLength)
{
    u8 *pu8Buffer;
    u32 ulTimes;
    u32 ulLeft;

	pu8Buffer = pucSrcAddr;

	ulTimes = u32TransLength / 4;
    ulLeft = u32TransLength % 4;

    send4((u32 *)pu8Buffer, ulTimes);
    pu8Buffer = pu8Buffer + ulTimes * 4;

    if(ulLeft == 1)
    {
        send1(pu8Buffer, 1);
    }
    else if(ulLeft == 2)
    {
        send2((u16 *)pu8Buffer, 1);
    }
    else if(ulLeft == 3)
    {
        send2((u16 *)pu8Buffer, 1);
        pu8Buffer = pu8Buffer + 1*2;
        send1(pu8Buffer, 1);
    }
    return OK;
}
static irqreturn_t hsuart_busstress_irq(int irq, void *ctx)
{
   	u32 ulInt = 0;
	u32 ulCharNum = 0;
	u32 Data = 0;
	u32 i = 0;
	u8 *recvTemp = 0;
	struct hsuart_busstress_str *uart_dev = &hs_uart_str;
	
    ulInt = readl((u32)uart_dev->vir_addr_base + HSUART_REGOFF_IIR);
    ulInt &= HSUART_INT_MASK;
	
    if(ulInt == HSUART_IIR_REV_TIMEOUT) 
    {	
		ulCharNum = readl((u32)uart_dev->vir_addr_base + HSUART_REGOFF_RFL);
		
		while(ulCharNum >= 4)
		{
			Data = readl((u32)uart_dev->vir_addr_base + HSUART_REGOFF_RBR);
			recvTemp = (u8 *)&Data;
			memcpy((void *)&recv_buff_str.uldata[recv_buff_str.ulwrite], (void *)&recvTemp[i], 4);
			recv_buff_str.ulwrite += 4;
			if(recv_buff_str.ulwrite >= BUFF_SIZE)
				recv_buff_str.ulwrite = 0;
			ulCharNum = ulCharNum - 4;
		}
		if (ulCharNum > 0)
		{
			Data = readl((u32)uart_dev->vir_addr_base + HSUART_REGOFF_RBR);
			recvTemp = (u8 *)&Data;
			memcpy((void *)&recv_buff_str.uldata[recv_buff_str.ulwrite], (void *)&recvTemp[i], ulCharNum);
			recv_buff_str.ulwrite += ulCharNum;
		}

    }

	else if(ulInt == 0x4)
    {
		ulCharNum = readl((u32)uart_dev->vir_addr_base + HSUART_REGOFF_USR);
        while(ulCharNum & 0x8)
        {
            Data = readl((u32)uart_dev->vir_addr_base + HSUART_REGOFF_RBR);
            recvTemp = (u8*)&Data;
            for(i = 0;i < 4;i++)
            {
				recv_buff_str.uldata[recv_buff_str.ulwrite] = recvTemp[i];
				recv_buff_str.ulwrite++;
				if(recv_buff_str.ulwrite >= BUFF_SIZE)
					recv_buff_str.ulwrite = 0;
            }
            ulCharNum = readl((u32)uart_dev->vir_addr_base + HSUART_REGOFF_USR);
        }
    }
	up(&(uart_dev->tx_sema));
	return (irqreturn_t)IRQ_HANDLED;
}
s32 hsuart_rx_task(void *ctx)
{
    struct hsuart_busstress_str *uart_ctx = &hs_uart_str;
    u8 test_buff[60];
	u32 i = 0;

    while (1)
    {
		down(&(hs_uart_str.tx_sema));
		for(i = 0;i<60;i++)
		{
			test_buff[i] = i;
		}
		
		hsuart_send(test_buff, 60);
		recv_buff_str.ulwrite = 0;
  	}
    return 0;
}
s32 hsuart_driver_init(struct hsuart_busstress_str *uart_ctx)
{
    s32 ret = ERROR;
	u32 regval = 0;
	u32 divisor = 0;
	u32 divisor_dll = 0;
    u32 divisor_dlh = 0;
	u32 val = 0;
	u32 u32Times = 0;
	u32 u32Discard = 0;
	
    spin_lock_init(&uart_ctx->lock_irq);
		
    osl_sem_init(SEM_EMPTY,&(hs_uart_str.tx_sema));
	regval = (HSUART_DEF_RT_ONEFOUR|HSUART_DEF_TET_ONETWO|HSUART_FCR_DMA_MODE|HSUART_TX_FIFO_RESET|HSUART_RX_FIFO_RESET|HSUART_FCR_FIFO_ENABLE);
	writel(regval , uart_ctx->vir_addr_base+ HSUART_REGOFF_FCR);		
    
	divisor =  uart_ctx->clock/ (u32)(16 * 115200);
	divisor_dll = divisor & 0xFF;
	divisor_dlh = (divisor & 0xFF00) >> 8;

    u32Discard = readl(uart_ctx->vir_addr_base + HSUART_REGOFF_USR);
    while((u32Discard & 0x01)&&(u32Times<1000))
    {
    	u32Discard = readl(uart_ctx->vir_addr_base + HSUART_REGOFF_USR);
        u32Times++;
    }
    if(1000 == u32Times)
    {
		hsuart_error("UART BUSY\n");
		return ERROR;
    }	
	val = readl(uart_ctx->vir_addr_base + HSUART_REGOFF_LCR);
    writel(HSUART_LCR_DLAB_EN, uart_ctx->vir_addr_base + HSUART_REGOFF_LCR);
    writel(divisor_dll, uart_ctx->vir_addr_base + HSUART_REGOFF_DLL);
    writel(divisor_dlh, uart_ctx->vir_addr_base + HSUART_REGOFF_DLH);
	val &= ~HSUART_LCR_DLAB_EN;
    writel(val, uart_ctx->vir_addr_base + HSUART_REGOFF_LCR); 
	
    writel(HSUART_LCR_DLS_8BITS | HSUART_LCR_STOP_1BITS |HSUART_LCR_PEN_NONE, uart_ctx->vir_addr_base + HSUART_REGOFF_LCR);
    writel(HSUART_DEF_TX_DEPTH, uart_ctx->vir_addr_base +  HSUART_REGOFF_TX_FF_DEPTH);
	writel(HSUART_IER_ALL_DIS, uart_ctx->vir_addr_base + HSUART_REGOFF_IER);
	readl(uart_ctx->vir_addr_base + HSUART_REGOFF_LSR);
	readl(uart_ctx->vir_addr_base + HSUART_REGOFF_RBR);
	readl(uart_ctx->vir_addr_base + HSUART_REGOFF_IIR);
	readl(uart_ctx->vir_addr_base + HSUART_REGOFF_USR);
	
	writel(HSUART_IER_ETO_EN | HSUART_IER_ERBFI_EN,uart_ctx->vir_addr_base+ HSUART_REGOFF_IER);
	
    return OK;
}


s32 hsuart_balong_busstress_init(void)
{
	struct clk *hsuart_clk = NULL;
	struct device_node *node = NULL;
	u32 ret = 0;
	hsuart_error("hsuart balong busstress init\n");
	node = of_find_compatible_node(NULL, NULL, "hisilicon,hsuart_app");
	if (!node)
	{
		hsuart_error("dts node hsuart app not found!\n");
		return ERROR;
	}
	if(of_property_read_u32(node, "hsuart_clk", &hs_uart_str.clock))
	{
		hsuart_error("read reg from dts is failed!\n");
		return ERROR;
	}
	hs_uart_str.irq = (u32)irq_of_parse_and_map(node, 0);
	if(hs_uart_str.irq == 0)
	{
		hsuart_error("read hsuart irq fail\n");
		return ERROR;
	}
	hs_uart_str.vir_addr_base = of_iomap(node, 0);
	if(NULL == hs_uart_str.vir_addr_base)
	{
		hsuart_error("vir addr get fail\n");
		return ERROR;
	}
	hsuart_vir_addr = hs_uart_str.vir_addr_base;
	hsuart_clk = clk_get(NULL, "hs_uart_clk");
	if(IS_ERR(hsuart_clk))
	{
		hsuart_error("hsuart get clk fail\n");
		return ERROR;
	}
	clk_prepare(hsuart_clk);
	clk_enable(hsuart_clk);
	hsuart_driver_init(&hs_uart_str);

	if (0 != osl_task_init("hsuart_rx_task",HSUART_TX_TASK_PRI,HSUART_TX_TASK_SIZE,(void *)hsuart_rx_task,(void *)&hs_uart_str,
										&hs_uart_str.rx_task_id))
	{
		hsuart_error("uart tx thread create failed!\n");
		return -1;
	}
	ret = request_irq((unsigned int)hs_uart_str.irq, (irq_handler_t)hsuart_busstress_irq,0,"HS UART ISR", &hs_uart_str);
	if (ret != OK)
	{
	   	hsuart_error("request_irq is failed!\n");
	    return ERROR;
	}
	return 0;
}

void hsuart_sem_up(void)
{
	up(&(hs_uart_str.tx_sema));
}
s32 hsuart_balong_busstress_start(void)
{
	if(hs_uart_str.init_flag == 0)
	{
		hsuart_balong_busstress_init();
		hs_uart_str.init_flag = 1;
	}
	else
	{
		enable_irq((unsigned int)hs_uart_str.irq);
	}
	up(&(hs_uart_str.tx_sema));

	return 0;	
}

s32 hsuart_balong_busstress_stop(void)
{
	disable_irq((unsigned int)hs_uart_str.irq);
	return 0;
}


