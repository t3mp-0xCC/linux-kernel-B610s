/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*******************************************************************/
/*                             ͷ�ļ�                                */
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
#include <linux/platform_device.h>
#include <asm-generic/sizes.h>
#include <bsp_shared_ddr.h>
#include <hi_uart.h>
#include <osl_irq.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include "bt_hsuart.h"


/*lint --e{18, 64, 102, 123, 160, 516, 522, 527, 529, 530, 550, 571, 629, 666, 681, 958, 959, 737, 730, 740,  } */
/*******************************************************************/
/*                                              �궨��                                              */
/*******************************************************************/
#define UART_NR 1 /* Number of uart supported */

#define UARTV7R5_LCRH 0x2c /*Offset of  Line control register. */
#define UARTV7R5_IFLS_RX4_8 (2 << 3)
#define UARTV7R5_IFLS_TX4_8 (2 << 0)
#define DW_ISR_PASS_LIMIT 64
#define SIZE_4K (4096)


#define UART_EXTINP_RTS_SHIFT 1
#define UART_EXTINP_RTS_MASK (1 << UART_EXTINP_RTS_SHIFT)
#define UART_EXTINP_DTR_SHIFT 0
#define UART_EXTINP_DTR_MASK (1 << UART_EXTINP_DTR_SHIFT)
/*******************************************************************/
/*                                             ��������                                          */
/*******************************************************************/
/* There is by now at least one vendor with differing details, so handle it */
struct vendor_data
{
    unsigned int ifls;
    unsigned int fifosize;
    unsigned int lcrh_tx;
    unsigned int lcrh_rx;
    bool oversampling;
    bool dma_threshold;
};


static struct vendor_data vendor_hisilicon =
{
    .ifls = UARTV7R5_IFLS_RX4_8 | UARTV7R5_IFLS_TX4_8,
    .fifosize = UART_FIFO_SIZE,
    .lcrh_tx = UARTV7R5_LCRH,
    .lcrh_rx = UARTV7R5_LCRH,
    .oversampling = false,
    .dma_threshold = false,
};

/*
 * We wrap our port structure around the generic uart_port.
 */
struct balongv7r5_huart_port
{
    struct uart_port port;
    struct clk* clk;
    const struct vendor_data* vendor;
    unsigned int dmacr; /* dma control reg */
    unsigned int im; /* interrupt mask */
    unsigned int old_status;
    unsigned int fifosize; /* vendor-specific */
    unsigned int lcrh_tx; /* vendor-specific */
    unsigned int lcrh_rx; /* vendor-specific */
    bool autorts;
    char type[12];
};

/*******************************************************************/
/*                                             ����ʵ��                                          */
/*******************************************************************/
int hsUartSend(unsigned char * pucSrcAddr, unsigned int u32TransLength,struct uart_port* port);

/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_stop_tx
*
* ��������  :  ֹͣ����
*
* �������  :  uart_port

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static void balongv7r5_huart_stop_tx(struct uart_port* port)
{
    struct balongv7r5_huart_port* uap = (struct balongv7r5_huart_port*)port; 

    uap->im &= ~(PTIME | UART_IER_TX_IRQ_ENABLE);
    writel(uap->im, uap->port.membase + UART_REGOFF_IER);
}

/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_start_tx
*
* ��������  :  ��ʼ���ʹ�������
*
* �������  :  uart_port

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static void balongv7r5_huart_start_tx(struct uart_port* port)
{
    struct balongv7r5_huart_port* uap = (struct balongv7r5_huart_port*)port; 

    uap->im |= UART_IER_TX_IRQ_ENABLE;

    writel(uap->im, uap->port.membase + UART_REGOFF_IER);
}

/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_stop_rx
*
* ��������  :  ֹͣ���մ�������
*
* �������  :  uart_port

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static void balongv7r5_huart_stop_rx(struct uart_port* port)
{
    struct balongv7r5_huart_port* uap = (struct balongv7r5_huart_port*)port;

    uap->im &= ~(UART_IER_LS_IRQ_ENABLE | UART_IER_RX_IRQ_ENABLE);

    writel(uap->im, uap->port.membase + UART_REGOFF_IER);
}

/*****************************************************************************
* �� �� ��     :  balongv7r5_uart_enable_ms
*
* ��������  :  ����modem status(�˴�Ϊ��)
*
* �������  :  uart_port

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static void balongv7r5_huart_enable_ms(struct uart_port* port)
{

    unsigned int value = 0;

    value = readl(port->membase + UART_REG_OFFSET_MCR);
    value |= (UART_RTS_ENABLE | UART_AFCE_ENABLE);
    writel(value, port->membase + UART_REG_OFFSET_MCR);

}

/*****************************************************************************
* �� �� ��     :  balongv7r5_uart_wait_idle
*
* ��������  :  �ȴ����ڴ������
*
* �������  :  uart_port  :��Ӧͨ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/

static inline int balongv7r5_huart_wait_idle(struct uart_port* port)
{
    unsigned int u32Discard = 0;
    unsigned int u32Times = 0;

    /* ��ȡUART ״̬�Ĵ�������UART�Ƿ���idle״̬*/
    u32Discard = readl(port->membase + UART_REGOFF_USR);
    while((u32Discard & 0x01)&&(u32Times < 1000))
    {
        u32Discard = readl(port->membase + UART_REGOFF_USR);
        u32Times++;
    }
    if(1000 == u32Times)
    {
        printk(KERN_ERR "UART BUSY\n");
        return ERROR;
    }
    return OK;
}

/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_received_chars_once
*
* ��������  : ���մ�����
*
* �������  :  balongv7r5_huart_port  :��Ӧͨ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
void balongv7r5_huart_received_chars_once(struct balongv7r5_huart_port* uap)
{
    unsigned int status = 0;
    unsigned int ch = 0;
    unsigned int flag = 0;
    unsigned int rsr = 0;
    unsigned int max_count = 0;//DW_ISR_PASS_LIMIT;
    unsigned int times = 0;
    unsigned int offset = 0;
    unsigned int avail_num = 0;
    unsigned int tem_plus = 0;
    unsigned char rec_buf[5] = {0};
    unsigned int i = 0;
    unsigned int ulInt = 0;

    ulInt = readl(uap->port.membase + UART_REGOFF_IIR);
    ulInt &= 0xF;

    if (ulInt == 0xC)
    {
        avail_num = readl(uap->port.membase + UART_REGOFF_RFL);
        max_count = min(avail_num, DW_ISR_PASS_LIMIT);
        times = ((max_count % 4) == 0) ? (max_count / 4) : (max_count / 4 + 1) ;
        offset = max_count % 4;
        tem_plus = 4;

        status = readl(uap->port.membase + UART_REGOFF_LSR);

        while ((status & UART_LSR_DR) && times--)
        {
            ch = readl(uap->port.membase + UART_REGOFF_RBR);
            flag = TTY_NORMAL;

            if (offset && (0 == times))
            {
                tem_plus = offset;
            }
            uap->port.icount.rx += tem_plus;
            rsr = status;

            if (unlikely(rsr & UART_LSR_ERR_ANY))
            {
                if (rsr & UART_LSR_BI)
                {
                    rsr &= ~(UART_LSR_FE | UART_LSR_PE);
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
                { flag = TTY_BREAK; }
                else if (rsr & UART_LSR_PE)
                { flag = TTY_PARITY; }
                else if (rsr & UART_LSR_FE)
                { flag = TTY_FRAME; }
            }

            memset(rec_buf,0,sizeof(rec_buf));
            memcpy((void*)rec_buf,(void*)&ch, tem_plus);

            for (i = 0; i < tem_plus; i++)
            {
                /* Handle sys char */
                if (uart_handle_sysrq_char(&uap->port, rec_buf[i]))
                {
                    status = readl(uap->port.membase + UART_REGOFF_LSR);
                    continue;
                }
               /* Insert char to tty flip buffer */
                uart_insert_char(&uap->port, rsr, UART_LSR_OE, rec_buf[i], flag);
            }

            status = readl(uap->port.membase + UART_REGOFF_LSR);
        }

    }
    else if (ulInt == 0x4)
    {
        avail_num = readl(uap->port.membase + UART_REGOFF_USR);
        status = readl(uap->port.membase + UART_REGOFF_LSR);
        
        while(avail_num & 0x8)
        {
            ch = readl(uap->port.membase + UART_REGOFF_RBR);

            rsr = status;
            flag = TTY_NORMAL;
            /*ÿ��ȡ�ĸ��ֽ�*/
            tem_plus = 4;
            memset(rec_buf,0,sizeof(rec_buf));
            memcpy((void*)rec_buf,(void*)&ch, tem_plus);

            for(i = 0;i < tem_plus;i++)
            {
                /* Handle sys char */
                if (uart_handle_sysrq_char(&uap->port, rec_buf[i]))
                {
                    status = readl(uap->port.membase + UART_REGOFF_LSR);
                    continue;
                }
                /* Insert char to tty flip buffer */
                uart_insert_char(&uap->port, rsr, UART_LSR_OE, rec_buf[i], flag);
            }
            avail_num = readl(uap->port.membase + UART_REGOFF_USR);
        }

    }
    /* Flush tty flip buffer */
    tty_flip_buffer_push(&uap->port.state->port);

    return;
}

/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_tx_int_proc
*
* ��������  : �����жϴ�����
*
* �������  :  balongv7r5_huart_port  :��Ӧͨ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static void balongv7r5_huart_tx_int_proc(struct balongv7r5_huart_port* uap)
{
    struct circ_buf* xmit = &uap->port.state->xmit;
    int count = 0;
    unsigned int reg_usr = 0;
    unsigned int temptail = 0;
    if (uap->port.x_char)
    {
        reg_usr = readl(uap->port.membase + UART_REGOFF_USR);
        if (reg_usr & UART_USR_FIFO_NOT_FULL)
        {
            writel(uap->port.x_char, uap->port.membase + UART_REGOFF_THR);
            uap->port.icount.tx++;
            uap->port.x_char = 0;
        }
        return;
    }

    if (uart_circ_empty(xmit) || uart_tx_stopped(&uap->port))
    {
        balongv7r5_huart_stop_tx(&uap->port);
        return;
    }


    count = uart_circ_chars_pending(xmit); 

   /*������͵����ݳ�����buf����*/
    if(xmit->tail + count > UART_XMIT_SIZE - 1)
    {
        /* ���͵����ݴ��ڻ���buf��ǰ�������֣�����buf��벿*/
        temptail = xmit->tail;
        hsUartSend(&xmit->buf[xmit->tail], UART_XMIT_SIZE - temptail,&uap->port);
        xmit->tail = (xmit->tail + UART_XMIT_SIZE - temptail) & (UART_XMIT_SIZE - 1);
        uap->port.icount.tx += UART_XMIT_SIZE - temptail;

        if (uart_circ_empty(xmit))
        {
            balongv7r5_huart_stop_tx(&uap->port);
            return;
        }
        /*���͵����ݴ��ڻ���buf��ǰ�������֣�����bufǰ�벿*/
        hsUartSend(&xmit->buf[xmit->tail], count - (UART_XMIT_SIZE - temptail) ,&uap->port);
        xmit->tail = (xmit->tail + count - (UART_XMIT_SIZE - temptail)) & (UART_XMIT_SIZE - 1);
        uap->port.icount.tx += count - (UART_XMIT_SIZE - temptail);

        if (uart_circ_empty(xmit))
        {
            balongv7r5_huart_stop_tx(&uap->port);
            return;
        }
    }
    else
    {
        /*���͵����ݴ��ڻ���buf���м䲿�֣�ֱ�ӷ���*/
        hsUartSend(&xmit->buf[xmit->tail], count,&uap->port);
        xmit->tail = (xmit->tail + count) & (UART_XMIT_SIZE - 1);
        uap->port.icount.tx += count;
        if (uart_circ_empty(xmit))
        {
            balongv7r5_huart_stop_tx(&uap->port);
            return;
        }
    }

    if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
    { uart_write_wakeup(&uap->port); }

    /* If buffer is empty ,stop transmit */
    if (uart_circ_empty(xmit))
    { balongv7r5_huart_stop_tx(&uap->port); }
}


/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_irq_handler
*
* ��������  :  �����ж��ܵ���Ӧ����
*
* �������  : irq :�жϺ� dev_id:�豸ID

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static irqreturn_t balongv7r5_huart_irq_handler(int irq, void* dev_id)
{
    struct balongv7r5_huart_port* uap = dev_id;
    unsigned int interrupt_id;
    int handled = 0;

    interrupt_id = readl(uap->port.membase + UART_REGOFF_IIR);
    interrupt_id = 0x0F & interrupt_id;

    if (interrupt_id != UART_IIR_NO_INTERRUPT)
    {
        if ((UART_IIR_REV_VALID == interrupt_id) ||
            (UART_IIR_REV_TIMEOUT == interrupt_id))
        {
            balongv7r5_huart_received_chars_once(uap);/* Handle irq from receive */
        }
        else if (UART_IIR_THR_EMPTY == interrupt_id)
        {
            balongv7r5_huart_tx_int_proc(uap);   /* Handle irq from transmit */
        }
        else /* transe busy intr*/
        {
            /* ȥʹ�������ж�*/
            readl(uap->port.membase + UART_REGOFF_LSR);
            readl(uap->port.membase + UART_REGOFF_RBR);
            readl(uap->port.membase + UART_REGOFF_IIR);
            readl(uap->port.membase + UART_REGOFF_USR);
            readl(uap->port.membase + UART_REGOFF_MSR);
            return (irqreturn_t)IRQ_NONE;
        }
        handled = 1;
    }

    return (irqreturn_t)IRQ_RETVAL(handled);
}


/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_tx_empty
*
* ��������  :  �����Ƿ����
*
* �������  :  uart_port  :��Ӧͨ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static unsigned int balongv7r5_huart_tx_empty(struct uart_port* port)
{
    struct balongv7r5_huart_port* uap = (struct balongv7r5_huart_port*)port;

    unsigned int status = readl(uap->port.membase + UART_REGOFF_LSR);

    if (status & UART_LSR_TEMT)
    {
        return TIOCSER_TEMT;
    }

    return 0;
}


/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_get_mctrl
*
* ��������  :  ��ȡmodem���Ʒ�ʽ
*
* �������  :  uart_port

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/

static unsigned int balongv7r5_huart_get_mctrl(struct uart_port* port)
{
    unsigned int result = 0;

    u32 val = 0;
    unsigned int mctrl = 0;
    val = readl(port->membase + 0x10);
    if (val & UART_EXTINP_DTR_MASK)
    {
        mctrl |= TIOCM_DTR;
    }
    if (val & UART_EXTINP_RTS_MASK)
    {
        mctrl |= TIOCM_RTS;
    }

    mctrl |= TIOCM_CTS;

    return mctrl;
}


/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_set_mctrl
*
* ��������  :  ����modem���Ʒ�ʽ
*
* �������  :  uart_port

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/

static void balongv7r5_huart_set_mctrl(struct uart_port* port, unsigned int mctrl)
{
    return;
}


/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_break_ctl
*
* ��������  :  ����break signals
*
* �������  :  uart_port  :��Ӧͨ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static void balongv7r5_huart_break_ctl(struct uart_port* port, int break_state)
{
    struct balongv7r5_huart_port* uap = (struct balongv7r5_huart_port*)port;
    unsigned long flags;
    unsigned int lcr_h;

    spin_lock_irqsave(&uap->port.lock, flags);

    /* Wait until transmit done */
    balongv7r5_huart_wait_idle(port);

    /* Read data in LCR */
    lcr_h = readl(uap->port.membase + UART_REGOFF_LCR);
    if (break_state)
    {
        lcr_h |= UART_LCR_BREAK;/* [false alarm]:�� */
    }
    else
    {
        lcr_h &= ~UART_LCR_BREAK;/* [false alarm]:�� */
    }

    /* Write data to break bit in LCR */
    writel(lcr_h, uap->port.membase + UART_REGOFF_LCR);

    spin_unlock_irqrestore(&uap->port.lock, flags);
}

/*****************************************************************************
* �� �� ��     :  balongv7r5_uart_startup
*
* ��������  :  �������ڶ˿ڣ��ڴ򿪸��豸�ǻ���ã������жϵȹ���
*
* �������  :  uart_port  :��Ӧͨ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static int balongv7r5_huart_startup(struct uart_port* port)
{

    struct balongv7r5_huart_port* uap = (struct balongv7r5_huart_port*)port;
    int retval = 0;
    unsigned long interrupt_flg;

    /* disable FIFO and interrupt */
    writel(UART_IER_IRQ_DISABLE, uap->port.membase + UART_REGOFF_IER);
    writel(UART_FCR_FIFO_DISABLE, uap->port.membase + UART_REGOFF_FCR);


    /* Allocate the IRQ*/
    retval = request_irq(uap->port.irq, balongv7r5_huart_irq_handler, 0, "BalongV7R5 Huart", uap);
    if (retval)
    {
        return retval;
    }

    /* ���÷������ */
    writel(UART_DEF_TX_DEPTH, uap->port.membase + UART_REGOFF_TX_FF_DEPTH);
    /* enable FIFO */
    writel(UART_FCR_DEF_SET, uap->port.membase + UART_REGOFF_FCR);

    /* clear interrupt status */
    readl(uap->port.membase + UART_REGOFF_LSR);
    readl(uap->port.membase + UART_REGOFF_RBR);
    readl(uap->port.membase + UART_REGOFF_IIR);
    readl(uap->port.membase + UART_REGOFF_USR);


    /* Finally, enable interrupts*/
    spin_lock_irqsave(&uap->port.lock, interrupt_flg);

    uap->im = UART_IER_RX_IRQ_ENABLE | UART_IER_ETO_EN;
    writel(uap->im, uap->port.membase + UART_REGOFF_IER);

    spin_unlock_irqrestore(&uap->port.lock, interrupt_flg);

    return 0;

}


/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_shutdown
*
* ��������  :  �رմ��ڣ��ڹرո��豸ʱ���ã��ͷ��жϵ�
*
* �������  :  uart_port  :��Ӧͨ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static void balongv7r5_huart_shutdown(struct uart_port* port)
{
    struct balongv7r5_huart_port* uap = (struct balongv7r5_huart_port*)port;
    unsigned long val;
    unsigned long interrup_flg;

    spin_lock_irqsave(&uap->port.lock, interrup_flg);

    /*disable/clear all interrupts*/
    uap->im = 0;
    writel(uap->im, uap->port.membase + UART_REGOFF_IER);
    readl( uap->port.membase + UART_REGOFF_LSR);

    spin_unlock_irqrestore(&uap->port.lock, interrup_flg);

    /*Free the interrupt*/
    free_irq(uap->port.irq, uap);

    balongv7r5_huart_wait_idle(port);
    val = readl(uap->port.membase + UART_REGOFF_LCR);
    val &= ~(UART_LCR_BREAK);/* [false alarm]:�� */
    writel(val, uap->port.membase + UART_REGOFF_LCR);

    /* disable fifo*/
    writel(UART_FCR_FIFO_DISABLE, uap->port.membase + UART_REGOFF_FCR);

}

/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_set_data_len
*
* ��������  :  ���ô������ݳ��ȵĲ���
*
* �������  :  uart_port

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static u32 balongv7r5_huart_set_data_len(struct ktermios* termios)
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
* �� �� ��     :  balongv7r5_huart_set_termios
*
* ��������  :  ���ô��ڲ���
*
* �������  :  uart_port

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static void balongv7r5_huart_set_termios(struct uart_port* port, struct ktermios* termios, struct ktermios* old)
{
    unsigned int lcr_h, old_cr;
    unsigned long flags;
    unsigned int baud, quot;

    /*
        * Ask the core to calculate the divisor for us.
      */

    baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk / 16);

    quot = port->uartclk / (16 * baud);/* [false alarm]:�� */

    lcr_h = balongv7r5_huart_set_data_len(termios);

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
        lcr_h |= UART_LCR_EPS;/* [false alarm]:�� */
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
#define UART_DUMMY_RSR_RX 256
    if ((termios->c_cflag & CREAD) == 0)
    {
        port->ignore_status_mask |= (UART_DUMMY_RSR_RX);
    }

    if (UART_ENABLE_MS(port, termios->c_cflag))
    {
        balongv7r5_huart_enable_ms(port);
    }

    balongv7r5_huart_wait_idle(port);

    /* Enable DLL and DLH */
    old_cr = readl(port->membase + UART_REGOFF_LCR);
    old_cr |= UART_LCR_DLAB;
    writel(old_cr, port->membase + UART_REGOFF_LCR);

    /* Set baud rate */
    writel(((quot & 0xFF00) >> 8), port->membase + UART_REGOFF_DLH);
    writel((quot & 0xFF), port->membase + UART_REGOFF_DLL);

    /* Disable DLL and DLH */
    old_cr &= ~UART_LCR_DLAB;/* [false alarm]:�� */
    writel(old_cr, port->membase + UART_REGOFF_LCR);
    writel(lcr_h, port->membase + UART_REGOFF_LCR);

    spin_unlock_irqrestore(&port->lock, flags);
}

/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_type
*
* ��������  :  ��ȡ�����豸
*
* �������  :  uart_port  :��Ӧͨ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static const char* balongv7r5_huart_type(struct uart_port* port)
{
    struct balongv7r5_huart_port* uap = (struct balongv7r5_huart_port*)port;

    return uap->port.type == PORT_AMBA ? uap->type : NULL;
}


/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_port_release_port
*
* ��������  :  �ͷŴ���ռ�õ��ڴ�ռ�
*
* �������  :  uart_port:��ǰ�˿�

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static void balongv7r5_huart_port_release_port(struct uart_port* port)
{
    release_mem_region(port->mapbase, SZ_4K);
}


/*****************************************************************************
* �� �� ��     :  balongv7r2_huart_port_request_port
*
* ��������  : Ϊ��������ռ�(4K)
*
* �������  :  AMBA_UART_CHAN  :��Ӧͨ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static int balongv7r5_huart_port_request_port(struct uart_port* port)
{
    return request_mem_region(port->mapbase, SZ_4K, "uart-balong") != NULL ? 0 : -EBUSY;
}


/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_port_config_port
*
* ��������  :  ��������,���ö˿����ͣ�����ռ�
*
* �������  :  AMBA_UART_CHAN  :��Ӧͨ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static void balongv7r5_huart_port_config_port(struct uart_port* port, int flags)
{
    if (flags & UART_CONFIG_TYPE)
    {
        port->type = PORT_AMBA;
        balongv7r5_huart_port_request_port(port);
    }
}


/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_init
*
* ��������  :  ��ʼ������
*
* �������  :  AMBA_UART_CHAN  :��Ӧͨ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static int balongv7r5_huart_port_verify_port(struct uart_port* port, struct serial_struct* ser)
{
    int ret = 0;
    if (ser->type != PORT_UNKNOWN && ser->type != PORT_AMBA)
    { ret = -EINVAL; }
    if (ser->irq < 0 || ser->irq >= NR_IRQS)
    { ret = -EINVAL; }
    if (ser->baud_base < 9600)
    { ret = -EINVAL; }
    return ret;
}
static struct balongv7r5_huart_port* huart_ports[UART_NR];
static struct uart_driver huart_reg =
{
    .owner = THIS_MODULE,
    .driver_name = "ttyHuart",
    .dev_name = "ttyHuart",
    .nr = 1,
    .cons = NULL,
};


/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_suspend
*
* ��������  :  ���ڹ���
*
* �������  :  struct device *dev

* �������  :  NA
*
* �� �� ֵ     :  OK:�ɹ�;����:ʧ��
*
* �޸ļ�¼  :
*****************************************************************************/
static int balongv7r5_huart_suspend(struct device *dev)
{
    struct balongv7r5_huart_port* uap = huart_ports[0];
    printk(KERN_INFO "balongv7r5_huart_suspend\n");

    if (!uap)
    {
        return -EINVAL;
    }
    disable_irq(INT_LVL_HSUART);
    return OK;

}

/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_resume
*
* ��������  :  ���ڻָ�
*
* �������  :  struct device *dev

* �������  :   NA
*
* �� �� ֵ     :  OK:�ɹ�;����:ʧ��
*
* �޸ļ�¼  :
*****************************************************************************/
static int balongv7r5_huart_resume(struct device *dev)
{
    struct balongv7r5_huart_port* uap = huart_ports[0];
    int ret = 0;
    unsigned int quot;
    u32 u32Times = 0;
    u32 u32Discard = 0;
    unsigned int result = 0;
    unsigned int baud = 115200;/*����������Ϊ115200*/

    if (!uap)
    {
        return -EINVAL;
    }
    printk(KERN_INFO "balongv7r5_huart_resume\n");

    /*���ò�����Ϊ115200*/
    quot = uap->port.uartclk / (16 * baud);

    /*ʹ��hsuartʱ��*/
    struct clk *hsuart_clk = NULL;
    hsuart_clk = clk_get(NULL, "hs_uart_clk");
    if(IS_ERR(hsuart_clk))
    {
        printk(KERN_ERR "hsuart get clk fail\n");
        return ERROR;
    }
    clk_prepare(hsuart_clk);
    clk_enable(hsuart_clk);
 
    /* enable FIFO */
    writel(UART_FCR_DEF_SET,  uap->port.membase + UART_REGOFF_FCR);

    /* ��ȡUART ״̬�Ĵ�������UART�Ƿ���idle״̬*/
    u32Discard = readl(uap->port.membase + UART_REGOFF_USR);
    while((u32Discard & 0x01)&&(u32Times < 1000))
    {
        u32Discard = readl(uap->port.membase + UART_REGOFF_USR);
        u32Times++;
    }
    if(1000 == u32Times)
    {
        printk(KERN_ERR "UART BUSY\n");
        return ERROR;
    }
    /*�������ؼĴ���*/
    writel(UART_RTS_ENABLE | UART_AFCE_ENABLE, uap->port.membase + UART_REG_OFFSET_MCR);

    /* Enable DLL and DLH */
    writel(UART_ENABLE_DLL, uap->port.membase + UART_REGOFF_LCR);

    /* ���ò����� */
    writel(((quot & 0xFF00) >> 8), uap->port.membase + UART_REGOFF_DLH);
    writel((quot & 0xFF),uap->port.membase + UART_REGOFF_DLL);

    /* Disable DLL and DLH */
    writel(UART_DISENABLE_DLL, uap->port.membase + UART_REGOFF_LCR);

   /* ���÷������ */
    writel(UART_DEF_TX_DEPTH, uap->port.membase + UART_REGOFF_TX_FF_DEPTH);

    /*��ֹ�����ж�*/
    writel(UART_IER_IRQ_DISABLE, uap->port.membase + UART_REGOFF_IER);

    /* ȥʹ�������ж�*/
    readl(uap->port.membase + UART_REGOFF_LSR);
    readl(uap->port.membase + UART_REGOFF_RBR);
    readl(uap->port.membase + UART_REGOFF_IIR);
    readl(uap->port.membase + UART_REGOFF_USR);

    /*ʹ���ж�*/
    enable_irq(INT_LVL_HSUART);

    /* ʹ�ܽ��պͳ�ʱ�ж� */
    writel(UART_IER_RX_IRQ_ENABLE | UART_IER_ETO_EN,uap->port.membase + UART_REGOFF_IER);

    return OK;

}


static const struct dev_pm_ops balongv7r5_uart_pm_ops = {
    .suspend = balongv7r5_huart_suspend,
    .resume = balongv7r5_huart_resume,
};


static struct uart_ops balongv7r5_huart_pops =
{
    .tx_empty = balongv7r5_huart_tx_empty,     /* transmit busy?          */
    .set_mctrl = balongv7r5_huart_set_mctrl,    /* set modem control    */
    .get_mctrl = balongv7r5_huart_get_mctrl,    /* get modem control    */
    .stop_tx = balongv7r5_huart_stop_tx,      /* stop transmit           */
    .start_tx = balongv7r5_huart_start_tx,     /* start transmit           */
    .stop_rx = balongv7r5_huart_stop_rx,      /* stop  receive            */
    .enable_ms = balongv7r5_huart_enable_ms,    /* enable modem status signals            */
    .break_ctl = balongv7r5_huart_break_ctl,    /* enable break signals            */
    .startup = balongv7r5_huart_startup,      /* start uart receive/transmit    */
    .shutdown = balongv7r5_huart_shutdown,     /* shut down uart           */
    .set_termios = balongv7r5_huart_set_termios,  /* set termios            */
    .type = balongv7r5_huart_type,         /* get uart type            */
    .release_port = balongv7r5_huart_port_release_port,
    .request_port = balongv7r5_huart_port_request_port,
    .config_port = balongv7r5_huart_port_config_port,
    .verify_port = balongv7r5_huart_port_verify_port,
};

static const struct of_device_id balongv7r5_uart_of_match[] = {
    { .compatible = "btHuart", },
    {},
};

static struct platform_driver huart_platform_driver = {
    .probe = NULL,
    .remove = NULL,
    .driver = {
        .name   = "btHuart",
        .owner  = THIS_MODULE,
        .pm     = &balongv7r5_uart_pm_ops,
        .of_match_table = of_match_ptr(balongv7r5_uart_of_match),
    },
};


void __iomem* g_base = NULL;


/*****************************************************************************
* �� �� ��     :  balongv7r5_huart_remove
*
* ��������  :  �Ƴ�һ���˿ڣ�����ȥע��ʱ����
*
* �������  :  AMBA_UART_CHAN  :��Ӧͨ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static int balongv7r5_huart_remove( )
{
    struct balongv7r5_huart_port* uap = huart_ports[0];
    int i = 0;    

    /* Remove the port */
    uart_remove_one_port(&huart_reg, &uap->port);
    platform_driver_unregister(&huart_platform_driver);

    for (i = 0; i < (int)ARRAY_SIZE(huart_ports); i++)
    {
        if (huart_ports[i] == uap)
        {
            huart_ports[i] = NULL;
        }
    }

    iounmap(uap->port.membase);

    kfree(uap);

    return 0;
}



/*****************************************************************************
* �� �� ��     :  balongv7r5_uart_init
*
* ��������  :  ��ʼ�����ڣ���������ע��
*
* �������  :  ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static int __init balongv7r5_huart_init(void)
{

    struct balongv7r5_huart_port* uap;
    struct vendor_data* vendor = &vendor_hisilicon;
    void __iomem* base;
    int i = 0;
    int ret = 0;

    /*ʹ��hsuartʱ��*/
    struct clk *hsuart_clk = NULL;
    hsuart_clk = clk_get(NULL, "hs_uart_clk");
    if(IS_ERR(hsuart_clk))
    {
        printk(KERN_ERR "hsuart get clk fail\n");
        return ERROR;
    }
    clk_prepare(hsuart_clk);
    clk_enable(hsuart_clk);

    /*ע��uart driver*/
    ret = uart_register_driver(&huart_reg);
    if(ret)
    {
        printk(KERN_ERR "uart driver register is failed!\n");
        clk_disable(hsuart_clk);
        return ret;
    }

    /*ע��platform driver*/
    ret = platform_driver_register(&huart_platform_driver);
    if (ret)
    {
        printk(KERN_ERR "Platform driver register is failed!\n");
        uart_unregister_driver(&huart_reg);
        clk_disable(hsuart_clk);
        return ret;
    }


    for (i = 0; i < (int)ARRAY_SIZE(huart_ports); i++)
        if (huart_ports[i] == NULL)
        { break; }

    if (i == ARRAY_SIZE(huart_ports))
    {
        ret = -EBUSY;
        goto out;
    }
    uap = kzalloc(sizeof(struct balongv7r5_huart_port), GFP_KERNEL);
    if (uap == NULL)
    {
        ret = -ENOMEM;
        goto out;
    }

    base = ioremap(HI_HSUART_REGBASE_ADDR, SIZE_4K);
 
    if (!base)
    {
        ret = -ENOMEM;
        kfree(uap);
        goto out;
    }

    uap->port.uartclk = HI_HSUARTCLK;
    uap->vendor = vendor;
    uap->lcrh_rx = vendor->lcrh_rx;
    uap->lcrh_tx = vendor->lcrh_tx;
    uap->fifosize = vendor->fifosize;
    uap->port.mapbase = HI_HSUART_REGBASE_ADDR;
    uap->port.membase = base;
    uap->port.iotype = UPIO_MEM;
    uap->port.irq = INT_LVL_HSUART;
    uap->port.fifosize = uap->fifosize;
    uap->port.flags = UPF_BOOT_AUTOCONF;
    uap->port.line = (unsigned int)i;
    uap->port.ops   = &balongv7r5_huart_pops;
    snprintf(uap->type, sizeof(uap->type), "Balong huart bluetooth");

    huart_ports[i] = uap;
    g_base = base;


    /* Add one port ,it will register console if we have already config console */
    ret = uart_add_one_port(&huart_reg, &uap->port);

    if (ret)
    {
        huart_ports[i] = NULL;
        goto out;
    }
    return OK;
out:
    uart_unregister_driver(&huart_reg);
    platform_driver_unregister(&huart_platform_driver);
    clk_disable(hsuart_clk);

    return ret;

}

/*****************************************************************************
* �� �� ��     :  balongv7r5_uart_exit
*
* ��������  :  �����˳�
*
* �������  :  ��

* �������  :  ��
*
* �� �� ֵ     :  ��
*
* �޸ļ�¼  :
*****************************************************************************/
static void __exit balongv7r5_huart_exit(void)
{   
    balongv7r5_huart_remove();
}


arch_initcall(balongv7r5_huart_init);
/*lint -save -e19 */
module_exit(balongv7r5_huart_exit);
/*lint -restore */

void send4(unsigned int* pucSrcAddr , unsigned int ulTimes, struct uart_port* port)
{
    unsigned int regval;
    unsigned int* pu32Buffer;

    pu32Buffer = pucSrcAddr;

    while (ulTimes)
    {
        regval = readl(port->membase + UART_REGOFF_USR);
        if (0 != (regval & 0x02))
        {
            /* ���������е����ݷŵ����ݼĴ�����*/
            *(unsigned int *)(port->membase + UART_REGOFF_THR) = *pu32Buffer;
            pu32Buffer++;
            ulTimes--;
        }
        else
        {
        }
    }
    return;
}

void send2(unsigned short* pucSrcAddr , unsigned int ulTimes, struct uart_port* port)
{
    unsigned int regval;
    unsigned short* pu16Buffer;

    pu16Buffer = pucSrcAddr;

    while (ulTimes)
    {
        regval = readl(port->membase + UART_REGOFF_USR);
        if (0 != (regval & 0x02))
        {
            /* ���������е����ݷŵ����ݼĴ�����*/
            *(unsigned short *)(port->membase + UART_REGOFF_THR) = *pu16Buffer;
           
            pu16Buffer++;
            ulTimes--;
        }
        else
        {
        }
    }
    return;
}

void send1(unsigned char* pucSrcAddr , unsigned int ulTimes, struct uart_port* port)
{
    unsigned int regval;
    unsigned char* pu8Buffer;

    pu8Buffer = pucSrcAddr;

    while (ulTimes)
    {
        regval = readl(port->membase + UART_REGOFF_USR);
        if (0 != (regval & 0x02))
        {        
            /* ���������е����ݷŵ����ݼĴ�����*/
            *(unsigned char *)(port->membase + UART_REGOFF_THR) = *pu8Buffer;
           
            pu8Buffer++;
            ulTimes--;
        }
        else
        {
        }
    }
    return;
}

/*****************************************************************************
* �� �� ��  : hsUartSend
*
* ��������  : HS UART�������ݽӿں���
*
* �������  : unsigned int  u32SrcAddr       �跢�͵����ݵ��׵�ַ
*             unsigned int  u32TransLength   �跢�͵����ݵĳ���
*
* �������  : ��
* �� �� ֵ  : OK       �ɹ�
*             ERROR    ʧ��
*
* �޸ļ�¼  :2010��12��16��   ³��  ����
*****************************************************************************/
int hsUartSend(unsigned char * pucSrcAddr, unsigned int u32TransLength,struct uart_port* port)
{

    unsigned char * pu8Buffer;
    unsigned int ulTimes = 0;
    unsigned int ulLeft = 0;
   
    /* ��������Ч�Լ�� */
    if((NULL == pucSrcAddr) || (0 == u32TransLength))
    {
        return -1;
    }

    pu8Buffer = pucSrcAddr;

    ulTimes = u32TransLength / 4;
    ulLeft = u32TransLength % 4;

    send4((unsigned int*)pu8Buffer, ulTimes,port); /*lint !e826*/
    pu8Buffer = pu8Buffer + ulTimes * SEND_FOURBYTE;

    if(ulLeft == SEND_ONEBYTE)
    {
        send1(pu8Buffer, 1,port);
    }
    else if(ulLeft == SEND_TWOBYTE)
    {
        send2((unsigned short*)pu8Buffer, 1,port); /*lint !e826*/
    }
    else if(ulLeft == SEND_THREBYTE)
    {
        send2((unsigned short*)pu8Buffer, 1,port); /*lint !e826*/
        pu8Buffer = pu8Buffer + SEND_TWOBYTE;
        send1(pu8Buffer, 1,port);
    }

    return 0;
}
/*****************************************************************************
* �� �� ��  : hsuart_read_reg
* ��������  :���Ժ�������ȡ���ڼĴ���ֵ
* �������  : u32 offset       �Ĵ���ƫ�Ƶ�ַ
* �������  : ��
* �� �� ֵ  : 
* �޸ļ�¼  :2015��11��10��
*****************************************************************************/
void hsuart_read_reg(u32 offset)
{
    u32 status;
    status = readl(g_base + offset);
    printk(KERN_INFO "offset:0x%x   value:0x%x !\n", offset, status);
}
/*****************************************************************************
* �� �� ��  : hsuart_write_reg
* ��������  :���Ժ�����д���ڼĴ���
* �������  : u32 offset       �Ĵ���ƫ�Ƶ�ַ
*             u32 value        �Ĵ���ֵ
* �������  : ��
* �� �� ֵ  : 
* �޸ļ�¼  :2015��11��10��
*****************************************************************************/
void hsuart_write_reg(u32 offset, u32 value)
{
    (void)writel(value, g_base + offset);   
     printk(KERN_INFO "write offset:0x%x   value:0x%x !\n", offset, value);
}

MODULE_AUTHOR("ARM Ltd/Deep Blue Solutions Ltd");
MODULE_DESCRIPTION("ARM AMBA serial port driver");
MODULE_LICENSE("GPL");


