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
#ifndef BALONG_UART_REG_LIST_H
#define BALONG_UART_REG_LIST_H

/*
 * synopsys uart register offsets
 */
#define	SYNOPSYS_UART_RBR		0x00
#define SYNOPSYS_UART_THR       0x00
#define SYNOPSYS_UART_DLL       0x00
#define SYNOPSYS_UART_DLH       0x04
#define SYNOPSYS_UART_IER       0x04
#define SYNOPSYS_UART_FCR       0x08
#define SYNOPSYS_UART_IIR       0x08
#define SYNOPSYS_UART_LCR       0x0C
#define SYNOPSYS_UART_MCR       0x10
#define SYNOPSYS_UART_LSR       0x14
#define SYNOPSYS_UART_MSR       0x18
#define SYNOPSYS_UART_USR       0x7C

#define SYNOPSYS_UART_IER_DISABLEALL 0x0
#define SYNOPSYS_UART_IER_ERBFI      0x1

#define SYNOPSYS_UART_FCR_FIFOE           (0x1U << 0)
#define SYNOPSYS_UART_FCR_RFIFOR          (0x1U << 1)
#define SYNOPSYS_UART_FCR_XFIFOR          (0x1U << 2)
#define SYNOPSYS_UART_FCR_TET_2CHR        (0x1U << 4)
#define SYNOPSYS_UART_FCR_TET_EMPTY       (0x0U << 4)
#define SYNOPSYS_UART_FCR_TET2_8          (0x2U << 4)
#define SYNOPSYS_UART_FCR_TET4_8          (0x3U << 4)
#define SYNOPSYS_UART_FCR_RT_1CHR         (0x0U << 6)
#define SYNOPSYS_UART_FCR_RT_2CHRTOFULL   (0x0U << 6)
#define SYNOPSYS_UART_FCR_RT2_8           (0x1U << 6)
#define SYNOPSYS_UART_FCR_RT4_8           (0x2U << 6)

#define SYNOPSYS_UART_IIR_RECVALIDE 0x4
#define SYNOPSYS_UART_IIR_RECVLSTA  0x6
#define SYNOPSYS_UART_IIR_BUSY      0x7
#define SYNOPSYS_UART_IIR_TIMEOUT   0xC

#define SYNOPSYS_UART_LCR_DLAB  (0x1U << 7)
#define SYNOPSYS_UART_LCR_DLS_8 (0x3U << 0)
#define SYNOPSYS_UART_LCR_DLS_7 (0x2U << 0)
#define SYNOPSYS_UART_LCR_DLS_6 (0x1U << 0)
#define SYNOPSYS_UART_LCR_DLS_5 (0x0U << 0)
#define SYNOPSYS_UART_LCR_STOP2 (0x1U << 2)
#define SYNOPSYS_UART_LCR_PEN   (0x1U << 3)

#define SYNOPSYS_UART_USR_TFNF      (0x1U << 1)
#define SYNOPSYS_UART_USR_RFNE      (0x1U << 3)

/*
 * arm uart(pl011) register offsets
 */
#define ARM_UART011_DR          0x00
#define ARM_UART011_FR          0x18
#define ARM_UART011_IBRD        0x24
#define ARM_UART011_FBRD        0x28
#define ARM_UART011_LCRH        0x2C
#define ARM_UART011_CR          0x30
#define ARM_UART011_IFLS        0x34
#define ARM_UART011_IMSC        0x38
#define ARM_UART011_MIS         0x40
#define ARM_UART011_ICR         0x44

#define ARM_UART011_FR_RXFF     0x040
#define ARM_UART011_FR_TXFF     0x020
#define ARM_UART011_FR_RXFE     0x010
#define ARM_UART011_FR_BUSY     0x008

#define ARM_UART011_LCRH_SPS    0x80
#define ARM_UART011_LCRH_WLEN_8 0x60
#define ARM_UART011_LCRH_WLEN_7 0x40
#define ARM_UART011_LCRH_WLEN_6 0x20
#define ARM_UART011_LCRH_WLEN_5 0x00
#define ARM_UART011_LCRH_FEN    0x10
#define ARM_UART011_LCRH_STP2   0x08
#define ARM_UART011_LCRH_EPS    0x04
#define ARM_UART011_LCRH_PEN    0x02
#define ARM_UART011_LCRH_BRK    0x01

#define ARM_UART011_CR_RXE      0x0200
#define ARM_UART011_CR_TXE      0x0100
#define ARM_UART011_CR_UARTEN   0x0001

#define ARM_UART011_IFLS_RX1_8   (0x0 << 3)
#define ARM_UART011_IFLS_RX2_8   (0x1 << 3)
#define ARM_UART011_IFLS_RX4_8   (0x2 << 3)
#define ARM_UART011_IFLS_RX6_8   (0x3 << 3)
#define ARM_UART011_IFLS_RX7_8   (0x4 << 3)
#define ARM_UART011_IFLS_TX1_8   (0x0 << 0)
#define ARM_UART011_IFLS_TX2_8   (0x1 << 0)
#define ARM_UART011_IFLS_TX4_8   (0x2 << 0)
#define ARM_UART011_IFLS_TX6_8   (0x3 << 0)
#define ARM_UART011_IFLS_TX7_8   (0x4 << 0)
#define ARM_UART011_IFLS_RX_HALF (0x5 << 3)
#define ARM_UART011_IFLS_TX_HALF (0x5 << 0)

#define ARM_UART011_RTIM    (0x1U << 6)
#define ARM_UART011_TXIM    (0x1U << 5)
#define ARM_UART011_RXIM    (0x1U << 4)

#define ARM_UART011_RXIC    (0x1U << 4)
#define ARM_UART011_TXIC    (0x1U << 5)
#define ARM_UART011_RTIC    (0x1U << 6)

#endif /* BALONG_UART_REG_LIST_H */