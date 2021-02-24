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

#ifndef    _UART_AT_H_
#define    _UART_AT_H_

#include <linux/workqueue.h>
#include <osl_spinlock.h>
#include <osl_list.h>
#include <osl_sem.h>
#include <bsp_udi.h>
#include <mdrv.h>


#ifdef __cplusplus
extern "C" {
#endif


#define UDI_GET_MAIN_DEV_ID(id)	(((id) & 0xff00) >> 8 )
#define UDI_GET_DEV_TYPE(id)	(((id) & 0x00ff))

/* 驱动适配函数指针表 */
typedef void (*UART_WRITE_DONE_CB_T)(char* pDoneBuff, int s32DoneSize);
typedef void (*UART_READ_DONE_CB_T)(void);


#define BUF_FIFO_SIZE 16
#define BUF_FIFO_NUM  256

struct phy_at_fifo
{
	struct list_head list;
	void *           buf;
	unsigned int     size;
	unsigned int     read_idx;
	unsigned int     write_idx;
};

struct phy_at_buf
{
	spinlock_t       lock;
	struct list_head list_free;
	struct list_head list_filled;
	struct list_head list_using;
	unsigned int     total_buf_size;
};

struct phy_at_port;
struct uart_ip_ops
{
	int (*uart_ip_init)(struct phy_at_port * port);
	int (*uart_ip_poll_out)(struct phy_at_port * port, char * str, int size);
	unsigned int periphid;
};

struct phy_at_port
{
	osl_sem_id        sem;
	int               open;
	unsigned int      dev_id;
	struct phy_at_buf recv_buf;
	struct uart_ip_ops * uart_ops;
	unsigned int periphid;
	void * uart_base;
	unsigned int irq_num;
	unsigned long clk_rate;
	unsigned int baud_rate;
	unsigned int fifo_depth;
};

struct phy_at_ctl
{
	struct phy_at_port   port;
	struct work_struct   work;
	struct workqueue_struct * work_queue;
	struct semaphore work_sync_sem;
	UART_READ_DONE_CB_T  read_cb;
	UART_WRITE_DONE_CB_T write_cb;
	union
	{
		unsigned int uint_tst_flg;
		struct
		{
			unsigned int flg_back_print    :1;
			unsigned int reserved          :31;
		}bit_tst_flg;
	}tst_flg;
	unsigned int recv_buf_full;
};
int phy_at_print_recv(void);

#define UART_WR_ASYNC_INFO ACM_WR_ASYNC_INFO

#define UART_READ_BUFF_INFO ACM_READ_BUFF_INFO
/*外部函数extern区*/
extern int BSP_UDI_SetPrivate(UDI_DEVICE_ID_E devId, void* pPrivate);
extern int BSP_UDI_SetCapability(UDI_DEVICE_ID_E devId, BSP_U32 u32Capability);
extern int BSP_UDI_SetInterfaceTable(UDI_DEVICE_ID_E devId, UDI_DRV_INTEFACE_TABLE *pDrvInterface);
extern unsigned int bsp_get_amba_ports(void);

int phy_at_register(struct uart_ip_ops * op);
int phy_at_unregister(void);
int phy_at_insert(char c);
int phy_at_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* _UART_AT_H_ */
