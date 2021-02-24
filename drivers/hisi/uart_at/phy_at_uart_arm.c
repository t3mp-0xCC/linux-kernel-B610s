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

#include <hi_uart.h>
#include <soc_interrupts.h>
#include <soc_clk.h>
#include <osl_types.h>
#include <osl_irq.h>
#include <osl_bio.h>
#include "at_uart_reglist.h"
#include "at_uart_balong.h"

#define CCORE_UART_FIFO_LEN 32
#define UART_ID_ARM         0x00041011

static int uart_poll_send(struct phy_at_port * port, char * str_c, int size)
{
	int i = 0;

	for(i = 0; i < size; i++)
	{
		while(ARM_UART011_FR_TXFF & readl(port->uart_base + ARM_UART011_FR));
		writel((unsigned int)str_c[i], port->uart_base + ARM_UART011_DR);
	}
	return 0;
}

OSL_IRQ_FUNC(irqreturn_t, phy_at_arm_inthandler, intLvl, dev)
{
	u32 reg_val_usr;
	u32 reg_val_iir;
	u32 rbr_char;
	int loop = 0;
	struct phy_at_port * port = (struct phy_at_port *)dev;
	
	reg_val_iir = readl(port->uart_base + ARM_UART011_MIS);
	writel(reg_val_iir & ~(ARM_UART011_RXIC | ARM_UART011_RTIC), port->uart_base + ARM_UART011_ICR);
	
	do
	{
		reg_val_usr = readl(port->uart_base + ARM_UART011_FR);
		if(reg_val_usr & ARM_UART011_FR_RXFE)
			break;
		rbr_char = readl(port->uart_base + ARM_UART011_DR);
		phy_at_insert((char)rbr_char);
	} while (loop++ <= CCORE_UART_FIFO_LEN);
	(void)phy_at_flush();

	return IRQ_HANDLED;
}

static int uart_init(struct phy_at_port * port)
{
	u32  ul_divisor;
	u32  frac;
	int  retval = 0;
	
	/*uart禁能*/
	writel(0, port->uart_base + ARM_UART011_CR);
	/* 屏蔽所有中断 */
	writel(0, port->uart_base + ARM_UART011_IMSC);
	/*清除所有中断*/
	writel(0xFFFFFFFF,port->uart_base + ARM_UART011_ICR);
	
	/*设置FIFO水线深度*/
	writel(ARM_UART011_IFLS_RX4_8 | ARM_UART011_IFLS_TX6_8, port->uart_base + ARM_UART011_IFLS);
	
	writel(0x181 , port->uart_base + ARM_UART011_CR);
	writel(1, port->uart_base + ARM_UART011_IBRD);
	writel(0, port->uart_base + ARM_UART011_FBRD);
	writel(0, port->uart_base + ARM_UART011_LCRH);
	writel(0, port->uart_base + ARM_UART011_DR);
	while(ARM_UART011_FR_BUSY & readl(port->uart_base + ARM_UART011_FR))
        ;
		//DWB();
	
	/* 设置串行通讯的波特率*/
	ul_divisor = port->clk_rate / (u32)(16 * port->baud_rate);/* [false alarm]:误报 */
	writel(ul_divisor, port->uart_base + ARM_UART011_IBRD);
	/*
	*小数部分计算方法:(clk % (16 * baud)) / (16 * baud) * 64 + 0.5
	*/
	frac = port->clk_rate % (u32)(16 * port->baud_rate);
	frac *= (1UL << 6);//(1 << 6 == 64)
	frac *= 2;
	frac += 16 * port->baud_rate;
	frac /= (u32)(2 * 16 * port->baud_rate);
	writel(frac, port->uart_base + ARM_UART011_FBRD);
	
	/* 配置数据长度为8bit,1停止位,无校验位,使能FIFO*/
	writel(ARM_UART011_LCRH_WLEN_8 | ARM_UART011_LCRH_FEN, port->uart_base + ARM_UART011_LCRH);
	/* 使能uart接受、发送*/
	writel(ARM_UART011_CR_TXE | ARM_UART011_CR_RXE | ARM_UART011_CR_UARTEN, port->uart_base + ARM_UART011_CR);

	writel(ARM_UART011_RXIM | ARM_UART011_RTIM, port->uart_base + ARM_UART011_IMSC);

	retval = request_irq(port->irq_num, (irq_handler_t)phy_at_arm_inthandler, 0, "uart-at", port);

	return retval;
}

struct uart_ip_ops op_arm = {.uart_ip_init = uart_init,
	                         .uart_ip_poll_out = uart_poll_send,
	                         .periphid = UART_ID_ARM
	                        };

static int uart_at_init(void)
{
	(void)phy_at_register(&op_arm);
	return 0;
}
subsys_initcall(uart_at_init);

