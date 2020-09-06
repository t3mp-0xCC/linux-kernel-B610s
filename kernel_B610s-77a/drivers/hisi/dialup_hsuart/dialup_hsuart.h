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



#ifndef    _HSUART_UDI_H_
#define    _HSUART_UDI_H_

#ifdef __cplusplus
extern "C" {
#endif
/**************************************************************************
  头文件包含                            
**************************************************************************/
#include <linux/semaphore.h>
#include <mach/gpio.h>
#include <osl_sem.h>
#include <osl_thread.h>
#include "bsp_softtimer.h"
#include "bsp_om.h"
#include "bsp_nvim.h"
#include "mdrv_usb.h"
#include "bsp_edma.h"
#include "drv_nv_id.h"
#include "drv_nv_def.h"
#include "mdrv_hsuart.h"
#include "mdrv_udi.h"


#ifdef _DRV_LLT_
#define memset
#endif

#define DIALUP_TRUE 	1
#define DIALUP_FALSE 	0
#define DIALUP_ENABLE 	1

#define dialup_error(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_HSUART, "DialUp:"fmt, ##__VA_ARGS__))
#define dialup_debug(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_DEBUG, BSP_MODU_HSUART, fmt, ##__VA_ARGS__))
#define dialup_info(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_INFO, BSP_MODU_HSUART, "DialUp:"fmt, ##__VA_ARGS__))
#define dialup_warn(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_WARNING, BSP_MODU_HSUART, "DialUp:"fmt, ##__VA_ARGS__))

#define UDI_UART1_ID		UDI_HSUART_0_ID
/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* TIMER LENTH*/
#define SWITCH_TIMER_LENGTH 	1000
#define SWITCH_TWO_TIMER_LENGTH 500

#define DEFAULT_BAUD_IDX		5
#define DEFAULT_BAUD			115200
#define SYS_TICK_32K			32768

/* buffer define */
#define UART_READ_BUF_SIZE 		1536
#define UART_READ_BUF_NUM 		16
#define UART_WRITE_BUF_NUM 		150
#define HSUART_WATER_NODE_HIGH	10
#define HSUART_WATER_NODE_LOW	30

#if ( FEATURE_ON == MBB_HSUART )
#define HSUART_DCD    			GPIO_2_20
#define HSUART_DSR        		GPIO_2_21
#define HSUART_DTR        		GPIO_2_22
#define HSUART_RING        		GPIO_2_23
#else
#define HSUART_DCD    			GPIO_2_4
#define HSUART_DSR        		GPIO_2_5
#define HSUART_DTR        		GPIO_2_16
#define HSUART_RING        		GPIO_2_26
#endif
#define HSUART_RX_TASK_PRI		81
#define HSUART_TX_TASK_PRI		81
#define HSUART_RX_TASK_SIZE		0x1000
#define HSUART_TX_TASK_SIZE		0x1000

#define UART011_RTSIFLS_FULL_2	(101 << 6)

#define BAND_SAMPLE_LENTH		(13)
typedef OSL_TASK_ID    	HSUART_TASK_ID;
typedef int* 	HTIMER;
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* +++检测阶段*/
typedef enum
{
    NO_DETECT_STAGE,
    DETECT_STAGE_1,
    DETECT_STAGE_2,
    DETECT_STAGE_3,
    DETECT_STAGE_MAX,
}detect_stage;

typedef enum tagUART_TRANSFER_MODE
{
    CPU_TRANSFER,
    DMA_TRANSFER,
    MODE_MAX,
}uart_transfer_mode;

typedef enum tagUART_MEM_STATE
{
    UART_MEM_NOT_IN_LIST = 0,
    UART_MEM_IN_FREELIST = 1,
    UART_MEM_IN_DONELIST = 2
}uart_mem_state;


/*****************************************************************************
  3 结构体定义
*****************************************************************************/
/* uart stat info */
typedef struct tagUART_STAT {
    unsigned int write_not_free_cnt;
    unsigned int lost_data_cnt;
    unsigned int syn_write_cnt;
    unsigned int burst_dma_done;
    unsigned int single_dma_done;
    unsigned int single_time_out;
    unsigned int alloc_skb_fail;
    unsigned int alloc_skb_cnt;
	unsigned int skb_free_cnt;
	unsigned int cur_baud;
}hsuart_stat;

typedef struct taguart_port_S
{
    int irq;
    unsigned int clock;
    struct spinlock lock_irq;
    unsigned int phy_addr_base;
    volatile void __iomem * vir_addr_base;
	unsigned int reg_size;
    enum edma_req_id dma_req_rx;
    enum edma_req_id dma_req_tx;
}uart_port;

typedef int (*uart_set_baud_cb)(uart_port *port, int baud);
typedef int (*uart_set_wlen_cb)(uart_port *port, int w_len);
typedef int (*uart_set_stp2_cb)(uart_port *port, int stp2);
typedef int (*uart_set_eps_cb)(uart_port *port, int eps);

typedef struct tagUART_UDI_CALLBACKS_S
{
    /* General Handlers */
    hsuart_free_cb_t free_cb;
    hsuart_read_cb_t read_cb;
    hsuart_msc_read_cb_t msc_read_cb;
    hsuart_switch_mode_cb_t switch_mode_cb;
    
    uart_set_baud_cb set_baud_cb;
    uart_set_wlen_cb set_wlen_cb;
    uart_set_stp2_cb set_stp2_cb;
    uart_set_eps_cb set_eps_cb;
	hsuart_water_cb water_ops;
}hsuart_udi_cbs;

typedef struct tagUART_WR_ASYNC_INFO
{
    char* pBuffer;
    unsigned int u32Size;
    void* pDrvPriv;
}hsuart_wr_async_info;

typedef struct tagUART_FEATURE
{
    int baud;
    int wlen;
    int stp2;
    int eps;
	u32 lcr;
	u32 mcr;
    volatile BSP_BOOL rts;
    volatile BSP_BOOL cts;
}hsuart_feature;

/* Transfer block descriptor */
struct mem_desc_s {
    void           *vaddr;
    unsigned int   paddr;
};

/* UART 内存信息 */
typedef struct skb_buff_str
{
    struct list_head list_node;
    uart_mem_state state;
    struct mem_desc_s mem;
    int valid_size;

    struct sk_buff *pSkb;
}skb_mem_str;

/* UART数据队列结构体*/
typedef struct tagUART_TRANS_INFO_S
{
    struct spinlock list_lock;
    struct list_head free_list;
    struct list_head done_list;
	
    skb_mem_str *pmem_start;                   /* MemNode数组首地址 */
    skb_mem_str *pmem_end;
    skb_mem_str *pcur_pos;                   	/* 当前使用节点*/
	
    unsigned int node_size;
    unsigned int node_num;
    unsigned int done_cnt;
	unsigned int free_node_cnt;
	unsigned int cur_bytes;
    unsigned int total_bytes;                      /* 总计数 */
}skb_buff_node;

struct uart_modem_gpio{
	unsigned int dcd;
	unsigned int dsr;
	unsigned int dtr;
	unsigned int ring;
};

typedef struct tagHSUART_CTX_S
{  
    uart_port port;
    UDI_DEVICE_ID_E udi_device_id;
    osl_sem_id tx_reqsem;
    osl_sem_id tx_endsem;
    BSP_BOOL init;
    BSP_BOOL open;
    BSP_BOOL baud_adapt;
    detect_stage stage;
    BSP_BOOL rts;
    uart_transfer_mode transfer_mode;
    skb_buff_node read_node;                     /* 读信息*/
    skb_buff_node write_node;                    /* 写信息*/
    hsuart_feature feature;
    hsuart_udi_cbs cbs;
    hsuart_stat stat;
	struct uart_modem_gpio gpio;
}dialup_ctx;

struct hsuart_ctrl
{
	struct softtimer_list switch_state3_timer;
#if ( FEATURE_ON == MBB_HSUART )
	struct softtimer_list adapt_timer;
#endif/*MBB_HSUART*/
	OSL_TASK_ID rx_task_id;
	OSL_TASK_ID tx_task_id;
	unsigned int ACSHELL_FLAG;
	unsigned int switch_cmdstr_cnt;
	unsigned int cur_baud_index;
	unsigned int free_node_cnt;
	unsigned int water_low_level_flag;
	unsigned int set_baud_flag;
};

struct hsuart_dma_ctrl
{
	unsigned int channel_id;
	unsigned int irq_flag;
	unsigned int total_size;
	unsigned int pMemNode_startaddr;
	unsigned int pMemNode_viraddr;
	unsigned int dma_done_flag;
	unsigned int dma_run_flag;
	unsigned int resume_flag;
	osl_sem_id edma_rx_sem;
};

struct dialup_water_level
{
	unsigned int band;
	unsigned int high_level;
    unsigned int low_level;
};


void switch_stage_three(u32 arg);
extern int BSP_UDI_SetPrivate(UDI_DEVICE_ID_E devId, VOID* pPrivate);
extern int BSP_UDI_SetCapability(UDI_DEVICE_ID_E devId, BSP_U32 u32Capability);
extern int BSP_UDI_SetInterfaceTable(UDI_DEVICE_ID_E devId, UDI_DRV_INTEFACE_TABLE *pDrvInterface);
extern unsigned int TTF_VIRT_TO_PHY(unsigned int ulVAddr);
extern s32 hsuart_shell_init(DRV_DIALUP_HSUART_STRU acshell_nv_stru);
extern void hsuart_acshell_resume(void);
extern dialup_ctx g_dialup_ctx;
#if ( FEATURE_ON == MBB_HSUART )
int hsuart_set_wlen(hsuart_dev *uart_dev, int w_len);
int hsuart_set_stpbit(hsuart_dev *uart_dev, int stp2);
int hsuart_set_eps(hsuart_dev *uart_dev, int eps);
#endif 
#if (FEATURE_ON == MBB_MODULE_PM)
/*****************************************************************************
* 函 数 名  : hsuart_sleep_monitor_time_update
* 功能描述  : hsuart监控空闲时间更新函数
* 输入参数  : NA
* 输出参数  :
* 返 回 值  : NA
*****************************************************************************/
extern void hsuart_sleep_time_update(unsigned int timeValue);
#endif

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */
#endif

