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

/*******************************************************************/
/*                                              头文件                                              */
/*******************************************************************/



#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/amba/bus.h>
#include <linux/amba/serial.h>
#include <linux/slab.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <asm-generic/sizes.h>

#include <bsp_shared_ddr.h>
#include <hi_uart.h>
#include <osl_irq.h>

/*lint --e{18, 64, 102, 123, 160, 516, 522, 527, 529, 530, 550, 571, 629, 666, 681, 958, 959, 737, 730, 740,  } */
/*******************************************************************/
/*                                              宏定义                                              */
/*******************************************************************/
#define UART_NR			        4       /* Number of uart supported */
#define SERIAL_BALONG_MAJOR	    204
#define SERIAL_BALONG_MINOR	    64
#define UART_LCRH		    0x2c	/*Offset of  Line control register. */
#define UART_IFLS_RX4_8	    (2 << 3)
#define UART_IFLS_TX4_8	    (2 << 0)
#define DW_ISR_PASS_LIMIT       16


/*******************************************************************/
/*                                             变量定义                                          */
/*******************************************************************/
/* There is by now at least one vendor with differing details, so handle it */
struct vendor_data
{
	unsigned int		ifls;
	unsigned int		fifosize;
	unsigned int		lcrh_tx;
	unsigned int		lcrh_rx;
	bool			    oversampling;
	bool			    dma_threshold;
};


static struct vendor_data vendor_hisilicon =
{
	.ifls			= UART_IFLS_RX4_8 | UART_IFLS_TX4_8,
	.fifosize		= UART_FIFO_SIZE,
	.lcrh_tx		= UART_LCRH,
	.lcrh_rx		= UART_LCRH,
	.oversampling	= false,
	.dma_threshold	= false,
};

/*
 * We wrap our port structure around the generic uart_port.
 */
struct synopsys_uart_port
{
	struct uart_port	port;
	struct clk		    *clk;
	const struct vendor_data *vendor;
	unsigned int		dmacr;		    /* dma control reg */
	unsigned int		im;		        /* interrupt mask */
	unsigned int		old_status;
	unsigned int		fifosize;	    /* vendor-specific */
	unsigned int		lcrh_tx;	    /* vendor-specific */
	unsigned int		lcrh_rx;	    /* vendor-specific */
	bool			    autorts;
	char			    type[12];
	unsigned int		mcr;
};


/*******************************************************************/
/*                                             函数实现                                          */
/*******************************************************************/

/*****************************************************************************
* 函 数 名     :  synopsys_uart_stop_tx
*
* 功能描述  :  停止发送
*
* 输入参数  :  uart_port

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_stop_tx(struct uart_port *port)
{
    struct synopsys_uart_port *uap = (struct synopsys_uart_port *)port;

    uap->im &= ~(PTIME | UART_IER_TX_IRQ_ENABLE);
    writel(uap->im, uap->port.membase + UART_REGOFF_IER);
}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_start_tx
*
* 功能描述  :  开始发送串口数据
*
* 输入参数  :  uart_port

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_start_tx(struct uart_port *port)
{
    struct synopsys_uart_port *uap = (struct synopsys_uart_port *)port;

    uap->im |= UART_IER_TX_IRQ_ENABLE;

    writel(uap->im, uap->port.membase + UART_REGOFF_IER);
}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_stop_rx
*
* 功能描述  :  停止接收串口数据
*
* 输入参数  :  uart_port

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_stop_rx(struct uart_port *port)
{
    struct synopsys_uart_port *uap = (struct synopsys_uart_port *)port;

    uap->im &= ~(UART_IER_LS_IRQ_ENABLE | UART_IER_RX_IRQ_ENABLE);

    writel(uap->im, uap->port.membase + UART_REGOFF_IER);
}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_enable_ms
*
* 功能描述  :  设置modem status(此处为空)
*
* 输入参数  :  uart_port

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_enable_ms(struct uart_port *port)
{
    return;
}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_wait_idle
*
* 功能描述  :  等待串口传输完成
*
* 输入参数  :  uart_port  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/

static inline void synopsys_uart_wait_idle(struct uart_port *port)
{
	while(readl(port->membase + UART_REGOFF_USR) & UART_USR_UART_BUSY)
	{
		if(readl(port->membase + UART_REGOFF_LSR) & UART_LSR_DR)
		{
			readl(port->membase + UART_REGOFF_RBR);
		}
	}
}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_rx_int_proc
*
* 功能描述  :  串口接收中断处理程序
*
* 输入参数  :  synopsys_uart_port  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_rx_int_proc(struct synopsys_uart_port *uap)
{
    unsigned int status;
    unsigned int ch;
    unsigned int flag;
    unsigned int rsr;
    unsigned int max_count = DW_ISR_PASS_LIMIT;

    status = readl(uap->port.membase + UART_REGOFF_LSR);

    while ((status & UART_LSR_DR) && max_count--)
    {
        ch = readl(uap->port.membase + UART_REGOFF_RBR);
        flag = TTY_NORMAL;
        uap->port.icount.rx++;

        rsr = status;

        /* Handle recive error first */
        if (unlikely(rsr & UART_LSR_ERR_ANY))
        {
            if (rsr & UART_LSR_BI)
            {
                rsr &= ~(UART_LSR_FE|UART_LSR_PE);
                uap->port.icount.brk++;
                if (uart_handle_break(&uap->port))
                {
                	status = readl(uap->port.membase + UART_REGOFF_LSR);
                	continue;
                }
            }
            else if (rsr & UART_LSR_PE)
            {
                uap->port.icount.parity++;
            }
            else if (rsr & UART_LSR_FE)
            {
                uap->port.icount.frame++;
            }
            else if (rsr & UART_LSR_OE)
            {
                uap->port.icount.overrun++;
            }

            rsr &= uap->port.read_status_mask;

            if (rsr & UART_LSR_BI)
                flag = TTY_BREAK;
            else if (rsr & UART_LSR_PE)
                flag = TTY_PARITY;
            else if (rsr & UART_LSR_FE)
                flag = TTY_FRAME;
        }

        /* Handle sys char */
        if (uart_handle_sysrq_char(&uap->port, ch))
        {
            status = readl(uap->port.membase + UART_REGOFF_LSR);
            continue;
        }

        /* Insert char to tty flip buffer */
        uart_insert_char(&uap->port, rsr, UART_LSR_OE, ch, flag);

        status = readl(uap->port.membase + UART_REGOFF_LSR);
    }

    /* Flush tty flip buffer */
    //tty_flip_buffer_push(tty);
    tty_flip_buffer_push(&uap->port.state->port);

    return;
}


/*****************************************************************************
* 函 数 名     :  synopsys_uart_tx_int_proc
*
* 功能描述  : 发送中断处理函数
*
* 输入参数  :  synopsys_uart_port  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_tx_int_proc(struct synopsys_uart_port *uap)
{
	struct circ_buf *xmit = &uap->port.state->xmit;
    int count = 0;
	unsigned int reg_usr;

    if (uap->port.x_char)
	{
		reg_usr = readl(uap->port.membase + UART_REGOFF_USR);
		if(reg_usr & UART_USR_FIFO_NOT_FULL)
		{
        	writel(uap->port.x_char, uap->port.membase + UART_REGOFF_THR);
        	uap->port.icount.tx++;
        	uap->port.x_char = 0;
		}
        return;
    }

    if (uart_circ_empty(xmit) || uart_tx_stopped(&uap->port)) {
        synopsys_uart_stop_tx(&uap->port);
        return;
    }

    count = (int)uap->port.fifosize;
    do
    {
		reg_usr = readl(uap->port.membase + UART_REGOFF_USR);
		if(reg_usr & UART_USR_FIFO_NOT_FULL)
		{
        	/* write data to xmit buffer */
        	writel(xmit->buf[xmit->tail], uap->port.membase + UART_REGOFF_THR);
        	xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
        	uap->port.icount.tx++;
		}
		else
		{
			break;
		}

        if (uart_circ_empty(xmit))
        {
            break;
        }
    } while (--count > 0);

    if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
        uart_write_wakeup(&uap->port);

    /* If buffer is empty ,stop transmit */
    if (uart_circ_empty(xmit))
        synopsys_uart_stop_tx(&uap->port);
}


/*****************************************************************************
* 函 数 名     :  synopsys_uart_modem_status
*
* 功能描述  :  设置modem status
*
* 输入参数  :  synopsys_uart_port  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_modem_status(struct synopsys_uart_port *uap)
{
    return;
}


/*****************************************************************************
* 函 数 名     :  synopsys_uart_irq_handler
*
* 功能描述  :  串口中断总的响应函数
*
* 输入参数  : irq :中断号 dev_id:设备ID

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static irqreturn_t synopsys_uart_irq_handler(int irq, void *dev_id)
{
    struct synopsys_uart_port *uap = dev_id;
    unsigned int interrupt_id;
    int handled = 0;

    spin_lock(&uap->port.lock);

    interrupt_id = readl(uap->port.membase + UART_REGOFF_IIR);
    interrupt_id = 0x0F & interrupt_id;

    /*if is "no interrupt pending" dap*/
    if(interrupt_id != UART_IIR_NO_INTERRUPT)
    {
        if ((UART_IIR_REV_VALID == interrupt_id) ||
            (UART_IIR_REV_TIMEOUT == interrupt_id) ||
            (UART_IIR_REV_STA == interrupt_id))
        {
            synopsys_uart_rx_int_proc(uap);    /* Handle irq from receive */
        }
        else if (UART_IIR_MODEM_INTERRUPT == interrupt_id)
        {
            synopsys_uart_modem_status(uap);
        }
        else if (UART_IIR_THR_EMPTY == interrupt_id)
        {
            synopsys_uart_tx_int_proc(uap);   /* Handle irq from transmit */
        }
		/* clear other interrupt status. */
        else
        {
            (void)readl(uap->port.membase + UART_REGOFF_LSR);
            (void)readl(uap->port.membase + UART_REGOFF_USR);
            (void)readl(uap->port.membase + UART_REGOFF_MSR);
        }
        handled = 1;
    }

    spin_unlock(&uap->port.lock);

    return (irqreturn_t)IRQ_RETVAL(handled);
}


/*****************************************************************************
* 函 数 名     :  synopsys_uart_tx_empty
*
* 功能描述  :  发送是否完成
*
* 输入参数  :  uart_port  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static unsigned int synopsys_uart_tx_empty(struct uart_port *port)
{
    struct synopsys_uart_port *uap = (struct synopsys_uart_port *)port;

    unsigned int status = readl(uap->port.membase + UART_REGOFF_LSR);

    if(status & UART_LSR_TEMT)
    {
        return TIOCSER_TEMT;
    }

    return 0;
}


/*****************************************************************************
* 函 数 名     :  synopsys_uart_get_mctrl
*
* 功能描述  :  获取modem控制方式
*
* 输入参数  :  uart_port

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static unsigned int synopsys_uart_get_mctrl(struct uart_port *port)
{
    struct synopsys_uart_port *uap = (struct synopsys_uart_port *)port;
    unsigned int result = 0;
    unsigned int status = readl(uap->port.membase + UART_REGOFF_MSR);

#define TIOCMBIT(uartbit, tiocmbit)	\
    if (status & uartbit)       \
        result |= tiocmbit

    TIOCMBIT(UART_MSR_DCD, TIOCM_CAR);
    TIOCMBIT(UART_MSR_DSR, TIOCM_DSR);
    TIOCMBIT(UART_MSR_CTS, TIOCM_CTS);
    TIOCMBIT(UART_MSR_RI, TIOCM_RNG);
#undef TIOCMBIT
    return result;
}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_set_mctrl
*
* 功能描述  :  设置modem控制方式
*
* 输入参数  :  uart_port

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{ 
    struct synopsys_uart_port *uap = (struct synopsys_uart_port *)port;
	unsigned int cr = 0;

#define	TIOCMBIT(tiocmbit, uartbit)		\
	if (mctrl & tiocmbit)		\
		cr |= uartbit;		\
	else				\
		cr &= ~uartbit

	TIOCMBIT(TIOCM_RTS, UART_MCR_RTS);
	TIOCMBIT(TIOCM_DTR, UART_MCR_DTR);
	TIOCMBIT(TIOCM_OUT1, UART_MCR_OUT1);
	TIOCMBIT(TIOCM_OUT2, UART_MCR_OUT2);
	TIOCMBIT(TIOCM_LOOP, UART_MCR_LOOP);
#undef TIOCMBIT

    cr |= uap->mcr;

	writel(cr, uap->port.membase + UART_REGOFF_MCR);
}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_break_ctl
*
* 功能描述  :  设置break signals
*
* 输入参数  :  uart_port  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_break_ctl(struct uart_port *port, int break_state)
{
    struct synopsys_uart_port *uap = (struct synopsys_uart_port *)port;
    unsigned long flags;
    unsigned int lcr_h;

    spin_lock_irqsave(&uap->port.lock, flags);


    /* Wait until transmit done */
	synopsys_uart_wait_idle(port);

    /* Read data in LCR */
    lcr_h = readl(uap->port.membase + UART_REGOFF_LCR);
    if (break_state)
    {
        lcr_h |= UART_LCR_BREAK;/* [false alarm]:误报 */
    }
    else
    {
        lcr_h &= ~UART_LCR_BREAK;/* [false alarm]:误报 */
    }

    /* Write data to break bit in LCR */
    writel(lcr_h, uap->port.membase + UART_REGOFF_LCR);

    spin_unlock_irqrestore(&uap->port.lock, flags);
}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_startup
*
* 功能描述  :  启动串口端口，在打开该设备是会调用，设置中断等工作
*
* 输入参数  :  uart_port  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static int synopsys_uart_startup(struct uart_port *port)
{

    struct synopsys_uart_port *uap = (struct synopsys_uart_port *)port;
    int retval;
	unsigned long interrupt_flg;

	/* disable FIFO and interrupt */
    writel(UART_IER_IRQ_DISABLE, uap->port.membase + UART_REGOFF_IER);
	writel(UART_FCR_FIFO_DISABLE, uap->port.membase + UART_REGOFF_FCR);

	/*
	 * Allocate the IRQ
	 */
	retval = request_irq(uap->port.irq, synopsys_uart_irq_handler, 0, "synopsys Uart", uap);
	if (retval)
	{
        return retval;

    }
	/* enable FIFO */
    writel(UART_FCR_DEF_SET, uap->port.membase + UART_REGOFF_FCR);

	/* clear interrupt status */
    readl(uap->port.membase + UART_REGOFF_LSR);
    readl(uap->port.membase + UART_REGOFF_IIR);
    readl(uap->port.membase + UART_REGOFF_USR);
    readl(uap->port.membase + UART_REGOFF_RBR);
	/*
     * Finally, enable interrupts
     */
    spin_lock_irqsave(&uap->port.lock, interrupt_flg);

    uap->im = (UART_IER_RX_IRQ_ENABLE);
    writel(uap->im, uap->port.membase + UART_REGOFF_IER);

    spin_unlock_irqrestore(&uap->port.lock, interrupt_flg);

	return 0;

}


/*****************************************************************************
* 函 数 名     :  synopsys_uart_shutdown
*
* 功能描述  :  关闭串口，在关闭该设备时调用，释放中断等
*
* 输入参数  :  uart_port  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_shutdown(struct uart_port *port)
{
    struct synopsys_uart_port *uap = (struct synopsys_uart_port *)port;
    unsigned long val;
	unsigned long interrup_flg;

    spin_lock_irqsave(&uap->port.lock, interrup_flg);

    /*
        * disable/clear all interrupts
      */
    uap->im = 0;
    writel(uap->im, uap->port.membase + UART_REGOFF_IER);
    readl( uap->port.membase + UART_REGOFF_LSR);

    spin_unlock_irqrestore(&uap->port.lock, interrup_flg);

    /*
        * Free the interrupt
      */
    free_irq(uap->port.irq, uap);

	synopsys_uart_wait_idle(port);
    val = readl(uap->port.membase + UART_REGOFF_LCR);
    val &= ~(UART_LCR_BREAK);/* [false alarm]:误报 */
    writel(val, uap->port.membase + UART_REGOFF_LCR);

    /* disable fifo*/
    writel(UART_FCR_FIFO_DISABLE, uap->port.membase + UART_REGOFF_FCR);

}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_set_data_len
*
* 功能描述  :  设置串口数据长度的参数
*
* 输入参数  :  uart_port

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static u32 synopsys_uart_set_data_len(struct ktermios *termios)
{
    u32 data_len = 0;
    switch (termios->c_cflag & CSIZE)
    {
        case CS5:
            data_len = UART_LCR_DLS_5BITS;
            break;
        case CS6:
            data_len = UART_LCR_DLS_6BITS;
            break;
        case CS7:
            data_len = UART_LCR_DLS_7BITS;
            break;
        case CS8:
            data_len = UART_LCR_DLS_8BITS;
            break;
        default: // CS8
            data_len = UART_LCR_DLS_8BITS;
            break;
    }
    return data_len;
}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_set_termios
*
* 功能描述  :  设置串口参数
*
* 输入参数  :  uart_port

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
#if (FEATURE_ON == MBB_FEATURE_GPS_UART)
#define UART_REG_OFFSET_MCR		0x10
#define UART_AFCE_ENABLE 		0x20 
#define UART_RTS_ENABLE			0x02
#endif
static void synopsys_uart_set_termios(struct uart_port *port,struct ktermios *termios, struct ktermios *old)
{
    unsigned int lcr_h;
    unsigned long flags;
    unsigned int baud, quot;
    unsigned int ul_divisor_high, ul_divisor_low;

#if (FEATURE_ON == MBB_FEATURE_GPS_UART)
    unsigned int mcr = 0;
#endif
    /*
        * Ask the core to calculate the divisor for us.
      */
    baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk/16);
    quot = port->uartclk / (16 * baud);/* [false alarm]:误报 */
    ul_divisor_high = (quot & 0xFF00) >> 8;
    ul_divisor_low = quot & 0xFF;

    lcr_h = synopsys_uart_set_data_len(termios);

    if (termios->c_cflag & CSTOPB)
    {
        lcr_h |= UART_LCR_STOP_2BITS;
    }

    if (termios->c_cflag & PARENB)
    {
        lcr_h |= UART_LCR_PEN;

    }

    if (!(termios->c_cflag & PARODD))
    {
        lcr_h |= UART_LCR_EPS;/* [false alarm]:误报 */
    }
    spin_lock_irqsave(&port->lock, flags);

    /*
      * Update the per-port timeout.
      */
    uart_update_timeout(port, termios->c_cflag, baud);

    port->read_status_mask = UART_LSR_OE;

    if (termios->c_iflag & INPCK)
    {
        port->read_status_mask |= UART_LSR_FE | UART_LSR_PE;
    }
    if (termios->c_iflag & (BRKINT | PARMRK))
    {
        port->read_status_mask |= UART_LSR_BI;
    }

    /*
      * Characters to ignore
      */
    port->ignore_status_mask = 0;
    if (termios->c_iflag & IGNPAR)
    {
        port->ignore_status_mask |= UART_LSR_FE | UART_LSR_PE;
    }
    if (termios->c_iflag & IGNBRK)
    {
        port->ignore_status_mask |= UART_LSR_BI;
      /*
         * If we're ignoring parity and break indicators,
         * ignore overruns too (for real raw support).
         */
        if (termios->c_iflag & IGNPAR)
        {
            port->ignore_status_mask |= UART_LSR_OE;
        }
    }

    /*
     * Ignore all characters if CREAD is not set.
     */
    #define UART_DUMMY_RSR_RX	256
    if ((termios->c_cflag & CREAD) == 0)
    {
        port->ignore_status_mask |= (UART_DUMMY_RSR_RX);
    }

    if (UART_ENABLE_MS(port, termios->c_cflag))
    {
        synopsys_uart_enable_ms(port);
    }

#if (FEATURE_ON == MBB_FEATURE_GPS_UART)
    if (termios->c_cflag & CRTSCTS) 
    {
        mcr = readl(port->membase + UART_REG_OFFSET_MCR);
        mcr |= (UART_RTS_ENABLE | UART_AFCE_ENABLE);
        (void)writel(mcr, port->membase + UART_REG_OFFSET_MCR);
    }
#endif

    /*config baudrate: uart must be idle */
    /*config baudrate: enable access DLL and DLH */
    do {
        readl(port->membase + UART_REGOFF_USR);
        writel(UART_FCR_DEF_SET, port->membase + UART_REGOFF_FCR);
        writel(UART_LCR_DLAB,port->membase + UART_REGOFF_LCR);
    }while(!(UART_LCR_DLAB & readl(port->membase + UART_REGOFF_LCR)));

    /* Set baud rate */
    writel(ul_divisor_high, port->membase + UART_REGOFF_DLH);
    writel(ul_divisor_low, port->membase + UART_REGOFF_DLL);

    do {
        readl(port->membase + UART_REGOFF_USR);
        writel(UART_FCR_DEF_SET, port->membase + UART_REGOFF_FCR);
        writel(UART_LCR_DEFAULTVALUE,port->membase + UART_REGOFF_LCR);
    }while(UART_LCR_DLAB & readl(port->membase + UART_REGOFF_LCR));

    writel(lcr_h, port->membase + UART_REGOFF_LCR);

    spin_unlock_irqrestore(&port->lock, flags);
}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_type
*
* 功能描述  :  获取串口设备
*
* 输入参数  :  uart_port  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static const char * synopsys_uart_type(struct uart_port *port)
{
	struct synopsys_uart_port *uap = (struct synopsys_uart_port *)port;

	return uap->port.type == PORT_AMBA ? uap->type : NULL;
}


/*****************************************************************************
* 函 数 名     :  synopsys_uart_port_release_port
*
* 功能描述  :  释放串口占用的内存空间
*
* 输入参数  :  uart_port:当前端口

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_port_release_port(struct uart_port *port)
{
	release_mem_region(port->mapbase, SZ_4K);
}


/*****************************************************************************
* 函 数 名     :  synopsys_uart_port_request_port
*
* 功能描述  : 为串口申请空间(4K)
*
* 输入参数  :  AMBA_UART_CHAN  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static int synopsys_uart_port_request_port(struct uart_port *port)
{
	return request_mem_region(port->mapbase, SZ_4K, "uart-balong") != NULL ? 0 : -EBUSY;
}


/*****************************************************************************
* 函 数 名     :  synopsys_uart_port_config_port
*
* 功能描述  :  串口配置,设置端口类型，申请空间
*
* 输入参数  :  AMBA_UART_CHAN  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_port_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE)
    {
		port->type = PORT_AMBA;
		synopsys_uart_port_request_port(port);
	}
}


/*****************************************************************************
* 函 数 名     :  synopsys_uart_init
*
* 功能描述  :  初始化串口
*
* 输入参数  :  AMBA_UART_CHAN  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static int synopsys_uart_port_verify_port(struct uart_port *port, struct serial_struct *ser)
{
    int ret = 0;
    if (ser->type != PORT_UNKNOWN && ser->type != PORT_AMBA)
        ret = -EINVAL;
    if (ser->irq < 0 || ser->irq >= NR_IRQS)
        ret = -EINVAL;
    if (ser->baud_base < 9600)
        ret = -EINVAL;
    return ret;
}

static struct uart_ops amba_synopsys_uart_pops = {
	.tx_empty	    = synopsys_uart_tx_empty,     /* transmit busy?          */
	.set_mctrl	    = synopsys_uart_set_mctrl,    /* set modem control    */
	.get_mctrl	    = synopsys_uart_get_mctrl,    /* get modem control    */
	.stop_tx	    = synopsys_uart_stop_tx,      /* stop transmit           */
	.start_tx	    = synopsys_uart_start_tx,     /* start transmit           */
	.stop_rx	    = synopsys_uart_stop_rx,      /* stop  receive            */
	.enable_ms	    = synopsys_uart_enable_ms,    /* enable modem status signals            */
	.break_ctl	    = synopsys_uart_break_ctl,    /* enable break signals            */
	.startup	    = synopsys_uart_startup,      /* start uart receive/transmit    */
	.shutdown	    = synopsys_uart_shutdown,     /* shut down uart           */
	.set_termios	= synopsys_uart_set_termios,  /* set termios            */
	.type		    = synopsys_uart_type,         /* get uart type            */
	.release_port	= synopsys_uart_port_release_port,
	.request_port	= synopsys_uart_port_request_port,
	.config_port	= synopsys_uart_port_config_port,
	.verify_port	= synopsys_uart_port_verify_port,
};

static struct synopsys_uart_port *amba_ports[UART_NR];

struct uart_port* bsp_get_amba_ports(void)
{
        return (struct uart_port*)(amba_ports[0]);
}


#ifdef CONFIG_SERIAL_SYNOPSYS_UART_CONSOLE
/*****************************************************************************
* 函 数 名     :  synopsys_uart_console_putchar
*
* 功能描述  :  控制台输出
*
* 输入参数  :  uart_port  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_console_putchar(struct uart_port *port, int ch)
{
    unsigned int status;

    /* Wait until send empty */
    do
    {
        status = readl(port->membase + UART_REGOFF_LSR);
    } while ((status & UART_LSR_THRE) != UART_LSR_THRE);

    writel(ch, port->membase + UART_REGOFF_THR);
}
/*****************************************************************************
* 函 数 名     :  synopsys_uart_console_write
*
* 功能描述  :  串口控制台写
*
* 输入参数  :  AMBA_UART_CHAN  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void synopsys_uart_console_write(struct console *co, const char *s, unsigned int count)
{
	struct synopsys_uart_port *uap = amba_ports[co->index];

    /* Write a console messge to serial port thougth function synopsys_uart_console_putchar */
	uart_console_write(&uap->port, s, count, synopsys_uart_console_putchar);
}
/*****************************************************************************
* 函 数 名     :  synopsys_uart_console_get_options
*
* 功能描述  :  获取当前配置
*
* 输入参数  :  synopsys_uart_port  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void __init synopsys_uart_console_get_options(struct synopsys_uart_port *uap, int *baud,int *parity, int *bits)
{
    unsigned int lcr_h, brd;
    unsigned long flags;
    unsigned char lbrd,hbrd;

    lcr_h = readl(uap->port.membase + UART_REGOFF_LCR);

    *parity = 'n';
    if (lcr_h & UART_LCR_PEN) {
        if (lcr_h & UART_LCR_EPS)
            *parity = 'e';
        else
            *parity = 'o';
    }

    if ((lcr_h & UART_LCR_DLS_MASK) == UART_LCR_DLS_5BITS)
        *bits = 5;
    else if((lcr_h & UART_LCR_DLS_MASK) == UART_LCR_DLS_6BITS)
        *bits = 6;
    else if((lcr_h & UART_LCR_DLS_MASK) == UART_LCR_DLS_7BITS)
        *bits = 7;
    else
        *bits = 8;


    spin_lock_irqsave(&uap->port.lock, flags);

    lbrd = readl(uap->port.membase + UART_REGOFF_DLL);
    hbrd = readl(uap->port.membase + UART_REGOFF_DLH);

    spin_unlock_irqrestore(&uap->port.lock, flags);

    brd = (hbrd << 8) + lbrd;

    *baud = uap->port.uartclk / (16 * brd);
}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_console_setup
*
* 功能描述  :  配置CONSOLE，包括波特率等信息
*
* 输入参数  :  AMBA_UART_CHAN  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static int __init synopsys_uart_console_setup(struct console *co, char *options)
{
	struct synopsys_uart_port *uap;
	int baud = 38400;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	/*
	 * Check whether an invalid uart number has been specified, and
	 * if so, search for the first available port that does have
	 * console support.
	 */
	if (co->index >= UART_NR)
		co->index = 0;
	uap = amba_ports[co->index];
	if (!uap)
		return -ENODEV;

	uap->port.uartclk = clk_get_rate(uap->clk);

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	else
		synopsys_uart_console_get_options(uap, &baud, &parity, &bits);

	return uart_set_options(&uap->port, co, baud, parity, bits, flow);
}

static struct uart_driver amba_reg;
static struct console amba_console = {
	.name		= "ttyAMA",
	.write		= synopsys_uart_console_write,
	.device		= uart_console_device,
	.setup		= synopsys_uart_console_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data		= &amba_reg,
};

#define AMBA_CONSOLE	(&amba_console)
#else
#define AMBA_CONSOLE	NULL
#endif  /* End of  CONFIG_SERIAL_synopsys_UART_CONSOLE */

static struct uart_driver amba_reg = {
	.owner			= THIS_MODULE,
	.driver_name	= "ttyAMA",
	.dev_name		= "ttyAMA",
	.major			= SERIAL_BALONG_MAJOR,
	.minor			= SERIAL_BALONG_MINOR,
	.nr			    = UART_NR,
	.cons			= AMBA_CONSOLE,
};

/*****************************************************************************
* 函 数 名     :  bsp_get_uart_console
*
* 功能描述  :  获取console
*
* 输入参数  :  void

* 输出参数  :  当前console
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
struct console* bsp_get_uart_console(void)
{
	struct console* uart_console = amba_reg.cons;
	return uart_console;
}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_probe
*
* 功能描述  :  初始化串口
*
* 输入参数  :  AMBA_UART_CHAN  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static int synopsys_uart_probe(struct amba_device *dev, const struct amba_id *id)
{
	struct synopsys_uart_port *uap;
	struct vendor_data *vendor = id->data;
	void __iomem *base;
	int i, ret;

	for (i = 0; i < (int)ARRAY_SIZE(amba_ports); i++)
		if (amba_ports[i] == NULL)
			break;

	if (i == ARRAY_SIZE(amba_ports))
    {
		ret = -EBUSY;
		goto out;
	}

	uap = kzalloc(sizeof(struct synopsys_uart_port), GFP_KERNEL);
	if (uap == NULL)
    {
		ret = -ENOMEM;
		goto out;
	}

	base = ioremap(dev->res.start, resource_size(&dev->res));
	if (!base)
    {
		ret = -ENOMEM;
		goto free;
	}
#if (FEATURE_ON == MBB_FEATURE_GPS_UART)
    if (HI_UART0_REGBASE_ADDR == dev->res.start)
    {
        uap->clk = clk_get(&dev->dev, "uart0_clk");
        if (IS_ERR(uap->clk))
        {
            ret = PTR_ERR(uap->clk);
            goto unmap;
        }
    }
    else
    {
        uap->clk = clk_get(&dev->dev, "uart2_clk");
        if (IS_ERR(uap->clk))
        {
            ret = PTR_ERR(uap->clk);
            goto unmap;
        }
    }
#else
	uap->clk = clk_get(&dev->dev, "uart0_clk");
	if (IS_ERR(uap->clk))
    {
		ret = PTR_ERR(uap->clk);
		goto unmap;
	}
#endif
	uap->vendor     = vendor;
	uap->lcrh_rx    = vendor->lcrh_rx;
	uap->lcrh_tx    = vendor->lcrh_tx;
	uap->fifosize   = vendor->fifosize;
	uap->port.dev   = &dev->dev;
	uap->port.mapbase   = dev->res.start;
	uap->port.membase   = base;
	uap->port.iotype    = UPIO_MEM;
	uap->port.irq       = dev->irq[0];
	uap->port.fifosize  = uap->fifosize;
	uap->port.ops   = &amba_synopsys_uart_pops;
	uap->port.flags = UPF_BOOT_AUTOCONF;
	uap->port.line  = (unsigned int)i;
#if (FEATURE_ON == MBB_FEATURE_GPS_UART)
    uap->port.uartclk = clk_get_rate(uap->clk);
#endif
	snprintf(uap->type, sizeof(uap->type), "Balong rev%u", amba_rev(dev));

	amba_ports[i] = uap;

	amba_set_drvdata(dev, uap);

    /* Add one port ,it will register console if we have already config console */
	ret = uart_add_one_port(&amba_reg, &uap->port);
	if (ret)
    {
		amba_set_drvdata(dev, NULL);
		amba_ports[i] = NULL;
		clk_put(uap->clk);
 unmap:
		iounmap(base);
 free:
		kfree(uap);
    }
 out:
	return ret;
}


/*****************************************************************************
* 函 数 名     :  synopsys_uart_remove
*
* 功能描述  :  移除一个端口，驱动去注册时调用
*
* 输入参数  :  AMBA_UART_CHAN  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static int synopsys_uart_remove(struct amba_device *dev)
{
	struct synopsys_uart_port *uap = amba_get_drvdata(dev);
	int i;

	amba_set_drvdata(dev, NULL);

    /* Remove the port */
	uart_remove_one_port(&amba_reg, &uap->port);

	for (i = 0; i < (int)ARRAY_SIZE(amba_ports); i++)
    {
		if (amba_ports[i] == uap)
        {
			amba_ports[i] = NULL;
        }
    }

	iounmap(uap->port.membase);
	clk_put(uap->clk);
	kfree(uap);
	return 0;
}

//#ifdef CONFIG_PM
/*****************************************************************************
* 函 数 名     :  synopsys_uart_suspend
*
* 功能描述  :  串口挂起
*
* 输入参数  :  amba_device

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static int synopsys_uart_suspend(struct amba_device *dev, pm_message_t state)
{
	struct synopsys_uart_port *uap = amba_get_drvdata(dev);

	if (!uap)
		return -EINVAL;

	return uart_suspend_port(&amba_reg, &uap->port);
}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_resume
*
* 功能描述  :  串口恢复
*
* 输入参数  :  AMBA_UART_CHAN  :对应通道

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static int synopsys_uart_resume(struct amba_device *dev)
{
	struct synopsys_uart_port *uap = amba_get_drvdata(dev);

	if (!uap)
		return -EINVAL;

	return uart_resume_port(&amba_reg, &uap->port);
}
//#endif

static struct amba_id synopsys_uart_ids[] = {
	{
		.id	    = UART_BALONG_ID,
		.mask	= UART_BALONG_ID_MASK,
		.data	= &vendor_hisilicon,
	},
	{ 0, 0 },
};

MODULE_DEVICE_TABLE(amba, synopsys_uart_ids);

static struct amba_driver synopsys_uart_driver = {
	.drv = {
		.name	= "uart-BlongV7R2",
	},
	.id_table	= synopsys_uart_ids,
	.probe		= synopsys_uart_probe,
	.remove		= synopsys_uart_remove,
//#ifdef CONFIG_PM
	.suspend	= synopsys_uart_suspend,
	.resume		= synopsys_uart_resume,
//#endif
};


/*****************************************************************************
* 函 数 名     :  synopsys_uart_init
*
* 功能描述  :  初始化串口，进行驱动注册
*
* 输入参数  :  无

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static int __init synopsys_uart_init(void)
{

	int ret = 0;

	printk(KERN_INFO "Serial: synopsys UART driver\n");

	ret = uart_register_driver(&amba_reg);
	if (ret == 0) {
		ret = amba_driver_register(&synopsys_uart_driver);
		if (ret)
			uart_unregister_driver(&amba_reg);
	}

	return ret;

}

/*****************************************************************************
* 函 数 名     :  synopsys_uart_exit
*
* 功能描述  :  串口退出
*
* 输入参数  :  无

* 输出参数  :  无
*
* 返 回 值     :  无
*
* 修改记录  :
*****************************************************************************/
static void __exit synopsys_uart_exit(void)
{
	amba_driver_unregister(&synopsys_uart_driver);
	uart_unregister_driver(&amba_reg);
}

arch_initcall(synopsys_uart_init);
/*lint -save -e19 */
module_exit(synopsys_uart_exit);
/*lint -restore */

MODULE_AUTHOR("ARM Ltd/Deep Blue Solutions Ltd");
MODULE_DESCRIPTION("ARM AMBA serial port driver");
MODULE_LICENSE("GPL");
