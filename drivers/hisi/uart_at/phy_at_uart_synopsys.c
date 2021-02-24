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
#include <osl_types.h>
#include <osl_irq.h>
#include <osl_bio.h>
#include "at_uart_reglist.h"
#include "at_uart_balong.h"

#define UART_ID_SYNOPSYS            0x000c21c0

OSL_IRQ_FUNC(irqreturn_t, phy_at_synop_inthandler, intLvl, dev)
{
	u32 reg_iir = 0;
	u32 reg_usr = 0;
	int i = 0;
	char c = 0;
	struct phy_at_port * port = (struct phy_at_port *)dev;

	reg_iir = readl(port->uart_base + SYNOPSYS_UART_IIR);
	reg_iir &= 0xF;
	if ((SYNOPSYS_UART_IIR_RECVALIDE == reg_iir) ||
		(SYNOPSYS_UART_IIR_TIMEOUT   == reg_iir) ||
		(SYNOPSYS_UART_IIR_RECVLSTA  == reg_iir))
	{
		do
		{
			c = (char)readl(port->uart_base + SYNOPSYS_UART_RBR);
			phy_at_insert(c);
			reg_usr = readl(port->uart_base + SYNOPSYS_UART_USR);
		}while((i++ < 16) && (reg_usr & SYNOPSYS_UART_USR_RFNE));
		(void)phy_at_flush();
	}
	
	return IRQ_HANDLED;
}

int uart_poll_out(struct phy_at_port * port, char * str_c, int size)
{
	int i = 0;
	const char * cc = str_c;

	for(i = 0; i < size; i++)
	{
		while(!(SYNOPSYS_UART_USR_TFNF & readl(port->uart_base + SYNOPSYS_UART_USR)));
		writel((unsigned)cc[i], port->uart_base + SYNOPSYS_UART_THR);
	}

	return 0;
}

int uart_reg_init(struct phy_at_port * port)
{
    u32  ul_divisor;
    u32  ul_divisor_high;
    u32  ul_divisor_low;
    u32  val;
    int  ret;

	val = SYNOPSYS_UART_FCR_FIFOE | SYNOPSYS_UART_FCR_RFIFOR | SYNOPSYS_UART_FCR_XFIFOR | SYNOPSYS_UART_FCR_TET4_8 | SYNOPSYS_UART_FCR_RT4_8;
    writel(val, port->uart_base + SYNOPSYS_UART_FCR);
    writel(SYNOPSYS_UART_IER_DISABLEALL, port->uart_base + SYNOPSYS_UART_IER);
    readl(port->uart_base + SYNOPSYS_UART_LSR);
    readl(port->uart_base + SYNOPSYS_UART_RBR);
    readl(port->uart_base + SYNOPSYS_UART_IIR);
    readl(port->uart_base + SYNOPSYS_UART_USR);
    ul_divisor = port->clk_rate / (u32)(16 * port->baud_rate);
    ul_divisor_high = (ul_divisor & 0xFF00) >> 8;
    ul_divisor_low = ul_divisor & 0xFF;

	do
	{
		(void)readl(port->uart_base + SYNOPSYS_UART_RBR);
		writel(SYNOPSYS_UART_LCR_DLAB, port->uart_base + SYNOPSYS_UART_LCR);
	}while(!(SYNOPSYS_UART_LCR_DLAB & readl(port->uart_base + SYNOPSYS_UART_LCR)));

	writel(ul_divisor_low, port->uart_base + SYNOPSYS_UART_DLL);
	writel(ul_divisor_high, port->uart_base + SYNOPSYS_UART_DLH);

	writel(SYNOPSYS_UART_LCR_DLS_8, port->uart_base + SYNOPSYS_UART_LCR);
	writel(SYNOPSYS_UART_IER_ERBFI, port->uart_base + SYNOPSYS_UART_IER);

	ret = request_irq(port->irq_num, (irq_handler_t)phy_at_synop_inthandler, 0, "uart-at", port);
	if (ret)
		return -1;
	return 0;
}

struct uart_ip_ops op_synopsys = {.uart_ip_init = uart_reg_init,
	                              .uart_ip_poll_out = uart_poll_out,
	                              .periphid = UART_ID_SYNOPSYS};
int uart_at_init(void)
{
	(void)phy_at_register(&op_synopsys);
	return 0;
}
subsys_initcall(uart_at_init);

