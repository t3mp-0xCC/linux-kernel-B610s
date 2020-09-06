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
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/clk.h>




#include <osl_types.h>
#include <osl_spinlock.h>
#include <osl_malloc.h>
#include <hi_uart.h>
#include <mdrv_udi.h>
#include <bsp_memmap.h>
#include <bsp_om.h>
#include <bsp_shared_ddr.h>
#include <bsp_pm.h>
#include <mdrv.h>
#include "at_uart_balong.h"
#define PHY_AT_ERR(fmt, ...)  bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_AT_UART, "[PHY_AT]: <%s> "fmt"\n", __FUNCTION__, ##__VA_ARGS__)

struct phy_at_ctl phy_at_chn;


struct phy_at_fifo * phy_at_fifo_get(unsigned int fifo_size)
{
	struct phy_at_fifo * new_fifo = NULL;

	new_fifo = (struct phy_at_fifo *)osl_malloc(sizeof(struct phy_at_fifo));
	if(!new_fifo)
		goto EXIT_FIFO_GET;
	INIT_LIST_HEAD(&new_fifo->list);
	new_fifo->read_idx = new_fifo->write_idx = 0;
	new_fifo->buf = osl_malloc(fifo_size);
	if(new_fifo->buf)
		new_fifo->size = fifo_size;
	else
	{
		new_fifo->size = 0;
		osl_free(new_fifo);
		new_fifo = NULL;
	}
EXIT_FIFO_GET:
	return new_fifo;
}

int phy_at_fifo_put(struct phy_at_fifo * fifo)
{
	osl_free(fifo->buf);
	osl_free(fifo);
	return 0;
}

int phy_at_insert(char c)
{
	unsigned long flag = 0;
	struct list_head * l_using = &(phy_at_chn.port.recv_buf.list_using);
	struct phy_at_fifo * fifo_using = (struct phy_at_fifo *)(l_using->next);
	struct list_head * entry = NULL;
	int ret = 0;
	if(!list_empty(l_using) && (fifo_using->write_idx < fifo_using->size))
	{
		*(char*)(fifo_using->buf + fifo_using->write_idx) = c;
		fifo_using->write_idx++;
		return ret;
	}

	spin_lock_irqsave(&phy_at_chn.port.recv_buf.lock, flag);
	if(!list_empty(&phy_at_chn.port.recv_buf.list_free))
	{
		entry = phy_at_chn.port.recv_buf.list_free.next;
		list_del(entry);
		list_add_tail(entry, l_using);

		fifo_using = (struct phy_at_fifo *)entry;
		*(char*)(fifo_using->buf + fifo_using->write_idx) = c;
		fifo_using->write_idx++;
	}
	else
	{
		/* error */
		phy_at_chn.recv_buf_full++;
		ret = -1;
	}
	spin_unlock_irqrestore(&phy_at_chn.port.recv_buf.lock, flag);
	return ret;
}

int phy_at_flush(void)
{
	unsigned long flag = 0;
	struct list_head * entry = &(phy_at_chn.port.recv_buf.list_using);

	spin_lock_irqsave(&phy_at_chn.port.recv_buf.lock, flag);
	while(!list_empty(&(phy_at_chn.port.recv_buf.list_using)))
	{
		entry = phy_at_chn.port.recv_buf.list_using.next;
		list_del(entry);
		list_add_tail(entry, &(phy_at_chn.port.recv_buf.list_filled));
	}
	spin_unlock_irqrestore(&phy_at_chn.port.recv_buf.lock, flag);

	//schedule_work(&phy_at_chn.work);
	queue_work(phy_at_chn.work_queue, &phy_at_chn.work);
	return 0;
}

int phy_at_register(struct uart_ip_ops * op)
{
	if(op->periphid != phy_at_chn.port.periphid)
		return -1;
	phy_at_chn.port.uart_ops = op;
	return 0;
}

int phy_at_unregister(void)
{
	phy_at_chn.port.uart_ops = NULL;
	return 0;
}

void phy_at_work_handler(struct work_struct *data)
{
	struct phy_at_ctl * at_ctl = &phy_at_chn;
	struct phy_at_port * at_port = (struct phy_at_port *)&phy_at_chn;
	struct list_head * entry = &(at_port->recv_buf.list_filled);
	unsigned long flag = 0;
	struct list_head * tmp = NULL;
	struct phy_at_fifo * tmp_fifo = NULL;

	while(down_interruptible(&phy_at_chn.work_sync_sem) != 0);

	while(!list_empty(entry))
	{
		if(!at_ctl->read_cb || (at_ctl->tst_flg.bit_tst_flg.flg_back_print != 0))
			(void)phy_at_print_recv();
		else
			(*at_ctl->read_cb)();
		/*
		 * Taf must read the data in the buffer obtained by it,
		 * otherwise all data will be cleared by the following code.
		 */
		spin_lock_irqsave(&at_port->recv_buf.lock, flag);
		tmp = entry->next;
		list_del(tmp);
		list_add_tail(tmp, &(at_port->recv_buf.list_free));
		tmp_fifo = (struct phy_at_fifo *)tmp;
		tmp_fifo->read_idx = tmp_fifo->write_idx = 0;
		spin_unlock_irqrestore(&(at_port->recv_buf.lock), flag);
	}

	up(&phy_at_chn.work_sync_sem);
}

int phy_at_open(UDI_OPEN_PARAM_S *param, UDI_HANDLE handle)
{
	(void)BSP_UDI_SetPrivate(param->devid, (void*)&phy_at_chn.port);
	phy_at_chn.port.open = 1;
	if(phy_at_chn.port.uart_ops && phy_at_chn.port.uart_ops->uart_ip_init)
		(*phy_at_chn.port.uart_ops->uart_ip_init)(&phy_at_chn.port);
	return 0;
}

int phy_at_close(void * dev)
{
	struct phy_at_port * port = (struct phy_at_port *)dev;
	port->open = 0;
	return 0;
}

int phy_at_write(void * dev, void * buffer, unsigned int size)
{
	struct phy_at_port * port = (struct phy_at_port *)dev;
	int ret = 0;

	osl_sem_down(&port->sem);
	if(port->uart_ops && port->uart_ops->uart_ip_poll_out)
		(*port->uart_ops->uart_ip_poll_out)(port, buffer, size);
	osl_sem_up(&port->sem);
	return ret;
}

int phy_at_ioctl(void * dev, unsigned int cmd, void * arg)
{
	struct phy_at_port * at_port = (struct phy_at_port *)dev;
	struct phy_at_ctl * ctl = (struct phy_at_ctl *)dev;
	int ret = 0;

	switch(cmd)
	{
	case ACM_IOCTL_SET_WRITE_CB:
		ctl->write_cb = (UART_WRITE_DONE_CB_T)arg;
		break;
	case ACM_IOCTL_SET_READ_CB:
		ctl->read_cb = (UART_READ_DONE_CB_T)arg;
		break;
	case ACM_IOCTL_GET_RD_BUFF:
		{
			ACM_WR_ASYNC_INFO * wr_info = (ACM_WR_ASYNC_INFO *)arg;
			struct phy_at_fifo *       fifo    = (struct phy_at_fifo *)(at_port->recv_buf.list_filled.next);

			wr_info->pVirAddr = fifo->buf;
			wr_info->pPhyAddr = NULL;
			wr_info->u32Size = fifo->write_idx - fifo->read_idx;
			wr_info->pDrvPriv = &fifo->list;

			/*
			spin_lock_irqsave(&at_port->recv_buf.lock, flag);
			list_del(&fifo->list);
			spin_unlock_irqrestore(&(at_port->recv_buf.lock), flag);
			*/
		}
		break;
	case ACM_IOCTL_RETURN_BUFF:
		{
			/*
			ACM_WR_ASYNC_INFO * wr_info = (ACM_WR_ASYNC_INFO *)arg;
			struct phy_at_fifo *       fifo    = (struct phy_at_fifo *)wr_info->pDrvPriv;
			unsigned long flag = 0;

			fifo->read_idx = fifo->write_idx = 0;
			spin_lock_irqsave(&at_port->recv_buf.lock, flag);
			list_add_tail(&fifo->list, &(at_port->recv_buf.list_free));
			spin_unlock_irqrestore(&(at_port->recv_buf.lock), flag);
			*/
		}
		break;
	default:
		break;
	}
	return ret;
}

UDI_DRV_INTEFACE_TABLE drv_interface_table;

int phy_at_init(void)
{
	int ret = 0;
	int i = 0;
	struct phy_at_fifo * malloc_fifo = NULL;
	struct phy_at_fifo * free_fifo   = NULL;
	const char * node_name = "arm,amba-bus";
	struct device_node * dts_node = NULL;
	struct device_node * dts_node_uart = NULL;
	unsigned int dts_val[3] = {0};
	const char * clk_name = NULL;
	struct clk * uart_clk = NULL;

	if(AT_UART_USE_FLAG != (*(u32 *)(SHM_BASE_ADDR + SHM_OFFSET_AT_FLAG)))
		return 0;

	dts_node = of_find_compatible_node(NULL, NULL, node_name);
	if(!dts_node)
	{
		PHY_AT_ERR("fail to get dts amba node\n");
		return -1;
	}
	dts_node_uart = of_get_child_by_name(dts_node, "uart_at");
	if(!dts_node_uart)
	{
		PHY_AT_ERR("fail to get uart node\n");
		return -1;
	}
	of_property_read_u32_array(dts_node_uart, "arm,primecell-periphid", &dts_val[0], 1);
	of_property_read_u32_array(dts_node_uart, "baud-rate",  &dts_val[1], 1);
	of_property_read_u32_array(dts_node_uart, "fifo-depth", &dts_val[2], 1);
	of_property_read_string   (dts_node_uart, "clock-name", &clk_name);

	phy_at_chn.port.uart_base  = of_iomap(dts_node_uart, 0);
	phy_at_chn.port.irq_num    = irq_of_parse_and_map(dts_node_uart, 0);
	phy_at_chn.port.periphid   = dts_val[0];
	phy_at_chn.port.baud_rate  = dts_val[1];
	phy_at_chn.port.fifo_depth = dts_val[2];
	uart_clk = clk_get(NULL, clk_name);
	if(IS_ERR(uart_clk))
		return -1;
	phy_at_chn.port.clk_rate = clk_get_rate(uart_clk);
	ret = clk_prepare_enable(uart_clk);
	if(ret)
	{
		PHY_AT_ERR("fail to enable phy at uart clk\n");
		return -1;
	}

	phy_at_chn.port.dev_id = UDI_BUILD_DEV_ID(UDI_DEV_UART, 0);
	phy_at_chn.port.open   = 0;
	phy_at_chn.port.uart_ops = NULL;
	osl_sem_init(1, &phy_at_chn.port.sem);
	spin_lock_init( &phy_at_chn.port.recv_buf.lock);
	INIT_LIST_HEAD( &phy_at_chn.port.recv_buf.list_free);
	INIT_LIST_HEAD( &phy_at_chn.port.recv_buf.list_filled);
	INIT_LIST_HEAD( &phy_at_chn.port.recv_buf.list_using);
	sema_init(&phy_at_chn.work_sync_sem, 1);
	phy_at_chn.tst_flg.uint_tst_flg = 0;
	phy_at_chn.recv_buf_full        = 0;

	for(i = 0; i < BUF_FIFO_NUM; i++)
	{
		malloc_fifo = phy_at_fifo_get(phy_at_chn.port.fifo_depth);
		if(!malloc_fifo)
		{
			PHY_AT_ERR("buffer malloc err! idx:%d\n", i);
			goto ERR_EXIT_RELEASE_BUF;
		}
		list_add_tail(&malloc_fifo->list, &(phy_at_chn.port.recv_buf.list_free));
		phy_at_chn.port.recv_buf.total_buf_size += phy_at_chn.port.fifo_depth;
	}

	phy_at_chn.work_queue = create_workqueue("phy-at-workqueue");
	if(!phy_at_chn.work_queue)
	{
		PHY_AT_ERR("create_workqueue err!\n");
		goto ERR_EXIT_RELEASE_BUF;
	}
	INIT_WORK(&phy_at_chn.work, phy_at_work_handler);
	phy_at_chn.read_cb  = NULL;
	phy_at_chn.write_cb = NULL;

	drv_interface_table.udi_open_cb  = phy_at_open;
	drv_interface_table.udi_close_cb = phy_at_close;
	drv_interface_table.udi_write_cb = phy_at_write;
	drv_interface_table.udi_ioctl_cb = phy_at_ioctl;
	drv_interface_table.udi_read_cb = NULL;
	drv_interface_table.udi_get_capability_cb = NULL;
	ret = BSP_UDI_SetCapability((UDI_DEVICE_ID_E)phy_at_chn.port.dev_id, 0);
	if(ret)
	{
		PHY_AT_ERR("BSP_UDI_SetCapability err! ret:%X\n", ret);
		goto ERR_EXIT_RELEASE_BUF;
	}
	ret = BSP_UDI_SetInterfaceTable((UDI_DEVICE_ID_E)phy_at_chn.port.dev_id, &drv_interface_table);
	if(ret)
	{
		PHY_AT_ERR("BSP_UDI_SetInterfaceTable err! ret:%X\n", ret);
		goto ERR_EXIT_RELEASE_BUF;
	}

	pm_enable_wake_src(PM_WAKE_SRC_UART0);

	PHY_AT_ERR("UART AT init OK!");
	return ret;
ERR_EXIT_RELEASE_BUF:
	while(!list_empty(&phy_at_chn.port.recv_buf.list_free))
	{
		free_fifo = (struct phy_at_fifo *)(phy_at_chn.port.recv_buf.list_free.next);
		osl_free(free_fifo->buf);
		list_del(&free_fifo->list);
		osl_free(free_fifo);
	}
	destroy_workqueue(phy_at_chn.work_queue);
	return ret;
}
subsys_initcall(phy_at_init);

int phy_at_print_recv(void)
{
	int retval = 0;
	ACM_WR_ASYNC_INFO wr_info;
	retval = phy_at_ioctl(&phy_at_chn.port, ACM_IOCTL_GET_RD_BUFF, &wr_info);
	retval = phy_at_write((void*)&phy_at_chn.port, wr_info.pVirAddr, wr_info.u32Size);
	return retval;
}

int phy_at_set_tstflg(unsigned int flg)
{
	phy_at_chn.tst_flg.uint_tst_flg = flg;
	return 0;
}
