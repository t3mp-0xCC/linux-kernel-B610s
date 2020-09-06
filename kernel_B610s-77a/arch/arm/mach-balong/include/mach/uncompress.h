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
#include <mach/hardware.h>
#include <asm/mach-types.h>

#include <mach/board.h>
#include <bsp_memmap.h>
#include <hi_uart.h>

#if (UART_BALONG_ID == ARM_UART_PERIPHERAL_ID)
#define AMBA_UART_DR(base)	(*(volatile unsigned int *)(base + UART_REGOFFSET_THR))
#define AMBA_UART_FR(base)	(*(volatile unsigned int *)(base + UART_REGOFFSET_FR))
#elif (UART_BALONG_ID == SYNOPSYS_UART_PERIPHERAL_ID)
#define AMBA_UART_DR(base)	(*(volatile unsigned int *)(base + UART_REGOFF_THR))
#define AMBA_UART_USR(base)	(*(volatile unsigned int *)(base + UART_REGOFF_USR))
#endif

/*
 * This does not append a newline
 */
static inline void putc(int c)
{
#ifdef CONFIG_BALONG_MINI_SYSTEM
	unsigned long base = MINI_SYSTEM_UART_ADDR;

#if (UART_BALONG_ID == ARM_UART_PERIPHERAL_ID)
	while (AMBA_UART_FR(base) & UART_FR_TX_FIFO_FULL);
	AMBA_UART_DR(base) = c;
#elif (UART_BALONG_ID == SYNOPSYS_UART_PERIPHERAL_ID)
	while (!(AMBA_UART_USR(base) & UART_USR_FIFO_NOT_FULL));
	AMBA_UART_DR(base) = c;
#endif
#endif
}

static inline void flush(void)
{
}

/*
 * nothing to do
 */
#define arch_decomp_setup()
#define arch_decomp_wdog()


