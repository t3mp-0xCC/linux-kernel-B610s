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



/*lint -save -e537 -e64 -e124 -e413 -e613 -e533 -e713 -e718 -e732 -e737 -e730 -e529*/
/*lint -save -e550 -e438 -e527 -e716 -e958 -e718 -e746 -e650 -e734 -e545*/
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
#include <linux/amba/serial.h>
#include <asm/irq.h>
#include "osl_malloc.h"
#include "osl_thread.h"
#include <bsp_edma.h>
#include <bsp_hardtimer.h>
#include "mdrv_udi.h"
#include "bsp_memmap.h"
#include "dialup_hsuart.h"
#if (FEATURE_ON == MBB_MODULE_PM)
#include "product_nv_id.h"
#include "product_nv_def.h"
#include <linux/wakelock.h>
#endif

/*lint -save -e110 -e82*/
dialup_ctx  g_dialup_ctx = {
    .port = {
        .dma_req_rx = EDMA_UART1_RX,
        .dma_req_tx = EDMA_UART1_TX
    },
    .udi_device_id = UDI_UART1_ID,
    .transfer_mode = DMA_TRANSFER,
};

struct baudsample
{
    unsigned int bandrate;           /*��Ӧ������ֵ*/
    unsigned int len;              /*�������ֵ����*/
    char   ch[BAND_SAMPLE_LENTH];    /*�������ֵ*/
};

/*115200�����ʲɼ�ֵ�б�*/
const struct baudsample g_sample_table[] = 
{
    { 115200 ,  2, {0x41,0x54}},  					/*AT*/
    { 115200 ,  2, {0x61,0x74}},  					/*at*/
    { 57600  ,  4, {0x06,0x3c,0x60,0x3f}}, 				/*at*/
    { 57600  ,  4, {0x06,0x30,0x60,0x33}}, 				/*AT*/
    { 38400  ,  2, {0x1c,0x00}}, 	/*at*/
    { 38400  ,  1, {0x1c}}, 	/*AT*/
    { 19200  ,  1, {0xe0}}, 						/*AT-at*/
    { 9600   ,  1, {0x0}}, 						/*AT-at*/
};

/*
����Ķ����Ӱ��ipstack���������ܣ�����Ӱ��ϵͳ����������
ipstack�·���������Ϊ1500��ÿ0.01�뷢��һ������
����:������Ϊ9600��һ���ӿɴ��ֽ���Ϊ: (9600λ / 8) * 0.8 (ͷβУ��λ) = 960�ֽ�

9600, 19200, 38400, 57600             ���建��2s������Ϊ��ˮλ��	����0.5s������Ϊ��ˮλ
115200, 230400, 460800, 921600        ���建��1s������Ϊ��ˮλ��	����0.25s������Ϊ��ˮλ
1000000, 2764800                      ���建��0.5s������Ϊ��ˮλ��	����0.13s������Ϊ��ˮλ
*/

/*�ߵ�ˮλ�ֽ���*/
/*{{1920, 480}, {3840, 960}, {7680, 1920}, {11520, 2880}, 
{11520, 2880}, {23040, 5760}, {46080, 11520}, {92160, 23040}, 
{50000, 13000}, {138240, 35942}};*/

const struct dialup_water_level free_node[] = 
{
	{9600	 , 1920	  , 480},
	{19200	 , 3840	  , 960},
	{38400	 , 7680	  , 1920},
	{57600	 , 11520  , 2880},
	{115200  , 11520  , 2880},
	{230400  , 23040  , 5760},
	{460800  , 46080  , 11520},
	{921600  , 92160  , 23040},
	{1000000 , 50000  , 13000},
	{2764800 , 138240 , 35942},
};

static const char switch_cmd[] = "+++";
struct hsuart_ctrl g_hsuart_ctrl ={{0},0};
struct hsuart_dma_ctrl g_dma_ctrl ={0};
#if ( FEATURE_ON == MBB_HSUART )
/*��ʾ����Ӧ��־:�����������**
 **0Ϊ����Ӧģʽ��δ����Ӧ�ɹ���1Ϊnormal or adapt success**/
static unsigned int allow_recv_flag = 1;
#define  BAND_ADAPT_RESULT      "\r\nOK\r\n"
#define  BAND_ADAPT_LENTH           (6)
#define  HSUART_SELFADAPT_TIMEOUT   (200)
unsigned int adapt_success_flag  = 0;
#endif/*MBB_HSUART*/
extern int clk_enable(struct clk * c);
extern void clk_disable(struct clk * c);

#if (FEATURE_ON == MBB_MODULE_PM)
#define  HSUART_PM_TASK_PRI		    (81)

/*begin*****************ȫ�ֱ�������************************/
/*Hsuart�����ݵļ��ʱ�䣬Ĭ������Ϊ10,��λΪ�룬��10s*/
unsigned int hsuart_sleep_time  = 10;
/*hsuartͶƱ���־: TRUE��ʾ˯�ߣ�FALSE��ʾ��˯��. Ĭ��δͶƱ��ʾ˯��*/
unsigned int   hsuart_sleep_flag = HSUART_TRUE;

/*wakeup_in����˯����*/
struct wake_lock hsuart_pm_lock;  
HSUART_TASK_ID hsuart_sleep_id   = 0;
unsigned int   hsuart_com_jiff   = 0;

static DEFINE_SPINLOCK(hsuart_sleep_spin);

/*end**********************ȫ�ֱ�������************************/

/*��������*/
static void hsuart_sleep_lock(void);
static void hsuart_sleep_unlock(void);

/*************************************************************
* �� �� ��  : hsuart_sleep_lock
* ��������  : hsuart���ѷ���˯�ߵĲ���
* �������  : NA
* �������  :
* �� �� ֵ  : NA
***************************************************************/
void hsuart_sleep_lock(void)
{
    unsigned long flag = 0;
    
    spin_lock_irqsave(&hsuart_sleep_spin,flag); 
    if(HSUART_FALSE == hsuart_sleep_flag)
    {
        spin_unlock_irqrestore(&hsuart_sleep_spin,flag);
        return;
    }
    /*Ͷ����Ʊ���������*/
    wake_lock(&hsuart_pm_lock);
    hsuart_sleep_flag = HSUART_FALSE;
    
    spin_unlock_irqrestore(&hsuart_sleep_spin,flag);

    return;
}

/*****************************************************************************
* �� �� ��  : hsuart_sleep_unlock
* ��������  : hsuart˯��ִ�в���
* �������  : NA
* �������  :
* �� �� ֵ  : NA
*****************************************************************************/
void hsuart_sleep_unlock(void)
{
    unsigned long flag = 0;

    spin_lock_irqsave(&hsuart_sleep_spin,flag);

    if(HSUART_TRUE == hsuart_sleep_flag)
    {
        spin_unlock_irqrestore(&hsuart_sleep_spin,flag);
        return;
    }

    /*Ͷ�޳�Ʊ���������*/
    wake_unlock(&hsuart_pm_lock);
    hsuart_sleep_flag = HSUART_TRUE;
    
    spin_unlock_irqrestore(&hsuart_sleep_spin,flag);
    return;
}

int hsuart_sleep_debug(void)
{
    printk(KERN_ERR "###uart sleep mng: 0x%x, flag: %d  timer:%d s ###\n", \
        hsuart_com_jiff, hsuart_sleep_flag, hsuart_sleep_time);

    return 0;
}

/*****************************************************************************
* �� �� ��  : hsuart_sleep_monitor_time_update
* ��������  : hsuart��ؿ���ʱ����º���
* �������  : NA
* �������  :
* �� �� ֵ  : NA
*****************************************************************************/
void hsuart_sleep_time_update(unsigned int timeValue)
{
    hsuart_sleep_time = timeValue;
}

/*****************************************************************************
* �� �� ��  : hsuart_pm_thread
* ��������  : hsuart��Դ������̣����hsuart 10s������ͨ��ʱ������˯��
* �������  : NA
* �������  :
* �� �� ֵ  : NA
*****************************************************************************/
int hsuart_pm_thread(void)
{
    unsigned long  jiff_temp = 0;
    
    while(1)
    {
        jiff_temp = hsuart_com_jiff;

        set_current_state(TASK_INTERRUPTIBLE);
        
        /* �������10s��û������ͨ�ţ�ģ������ٴν���˯��״̬,HZ��ʾ�� */
        schedule_timeout(HZ * hsuart_sleep_time);/*lint !e534*/
        
        barrier();
        
        if (jiff_temp == hsuart_com_jiff)
        {
            hsuart_sleep_unlock();
        }
    }

    return 0;
}

/*****************************************************************************
* �� �� ��  : hsuart_pm_init
* ��������  : hsuart��Դ�����ʼ������
* �������  : NA
* �������  :
* �� �� ֵ  : NA
*****************************************************************************/
void hsuart_pm_init(void)
{
    unsigned int ret = 0;
    unsigned int ulSwVerFlag = 0xff;
    unsigned int timevalue = 0;
    nv_huawei_sleepcfg_type Huawei_sleep_cfg = {0};

    printk(KERN_ERR"hsuart_pm_init begin... \n");

    wake_lock_init(&hsuart_pm_lock, WAKE_LOCK_SUSPEND, "pm_hsuart");

    /*��NV50467�ж�ȡsleepcfg��Ϣ*/
    ret = bsp_nvm_read(en_NV_Item_SLEEP_CFG_FLAG,\
                (u8 *)&Huawei_sleep_cfg,sizeof(Huawei_sleep_cfg));
    if ( NV_OK != ret )
    {
        /*NV��ȡʧ�ܣ�Ĭ������Ϊ10,��λΪ�룬��10s*/
        hsuart_sleep_time = 10;
        printk(KERN_ERR " hsuart_sleep_mng:read nv50467 fail !!! \n");
    }
    else
    {
        hsuart_sleep_time = Huawei_sleep_cfg.huawei_sleep_cfg[1].sleepcfg_value;
    }

    
	if (ERROR == osl_task_init("hsuart_pm_thread",\
            HSUART_RX_TASK_PRI, HSUART_RX_TASK_SIZE,\
            (void *)hsuart_pm_thread,NULL, &hsuart_sleep_id))
	{
	  	hsuart_error(KERN_ERR "<hsuart_pm_init> Create hsuart_pm_thread error !!!\n");
		goto init_fail;
	}

    /*��ʼ����10s�ڲ���hsuart˯��*/
    hsuart_sleep_lock();
    hsuart_com_jiff = jiffies;
	hsuart_error(KERN_ERR "<hsuart_pm_init> init end... \n");
    return;
    
init_fail:
	hsuart_error(KERN_ERR "<hsuart_pm_init> init fail !!!\n");
	return;

}
#endif /* MBB_MODULE_PM */


/*******************************************************************************
* �� �� ��  : uart_fifo_reset
* ��������  : ��λuart fifo
* �������  : uart_dev �豸ָ��
* �������  : NA
* �� �� ֵ  : NA
*******************************************************************************/
static void uart_disable(uart_port *port)
{
    u32 val = 0;
    val = readl((volatile void *)port->vir_addr_base + UART011_CR);
    val &= ~(UART01x_CR_UARTEN);
    (void)writel(val, (volatile void *)port->vir_addr_base + UART011_CR);
}

static void uart_enable(uart_port *port)
{
    u32 val = 0;
    val = readl((volatile void *)port->vir_addr_base + UART011_CR);
    val |= (UART01x_CR_UARTEN);
    (void)writel(val, (volatile void *)port->vir_addr_base + UART011_CR);
}
/*****************************************************************************
* �� �� ��  : uart_set_baud
* ��������  : UART ���ò����ʺ���
* �������  : uart_dev: �豸����ṹָ��
*             baud: ������ֵ
* �������  :
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
static int uart_set_baud(uart_port *port, int baud)
{
	u32 i = 0;
	unsigned int ibrd,fbrd;
    dialup_ctx *uart_ctx = NULL;
	unsigned int regval = 0;

    if (NULL == port || baud < 0 )
    {
        dialup_error("uart_port:0x%x   baud:%d\n", (unsigned int)port, baud);
        return ERROR;
    }
    dialup_info("uart_dev->clock: %d\n", port->clock);
    uart_ctx = container_of(port, dialup_ctx, port);
    if (0 == baud)
    {
		uart_ctx->baud_adapt = DIALUP_TRUE;
        baud = DEFAULT_BAUD;
#if ( FEATURE_ON == MBB_HSUART )
        allow_recv_flag = 0;
#endif
        dialup_info("baud adapt start");
    }
#if ( FEATURE_ON == MBB_HSUART )
    else
    {
        uart_ctx->baud_adapt = HSUART_FALSE;
    }
#endif
    uart_ctx->feature.baud = baud;
	
    dialup_info("set baud: %d\n", baud);

   	while(UART01x_FR_BUSY & readl(port->vir_addr_base + UART01x_FR))
       barrier();

    ibrd = port->clock / (16 * baud);
    writel(ibrd, port->vir_addr_base + UART011_IBRD);
    fbrd = port->clock % (16 * baud);
    fbrd *= (1UL << 6);
    fbrd *= 2;
    fbrd += 16 * baud;
    fbrd /= (u32)(2 * 16 * baud);
    writel(fbrd, port->vir_addr_base + UART011_FBRD);
	regval = readl(port->vir_addr_base + UART011_LCRH);
	writel(regval, port->vir_addr_base + UART011_LCRH);

	uart_ctx->stat.cur_baud = baud;
	
	for(i = 0; i < sizeof(free_node)/sizeof(struct dialup_water_level) ; i++)
	{
		if(free_node[i].band == baud)
		{
			g_hsuart_ctrl.cur_baud_index = i;
			dialup_debug("cur_baud_index %d\n", g_hsuart_ctrl.cur_baud_index);
			break;
		}
	}
	if(i == sizeof(free_node)/sizeof(struct dialup_water_level))
	{
		g_hsuart_ctrl.cur_baud_index = 0;
		dialup_info("water defult index\n");
	}
#if ( FEATURE_ON == MBB_HSUART )
    if ( uart_ctx->baud_adapt )
    {
        /*��������ӦģʽԼ����֡��ʽ*/
        hsuart_set_wlen(uart_dev, WLEN_8_BITS);
        hsuart_set_stpbit(uart_dev, STP2_OFF);
        hsuart_set_eps(uart_dev, PARITY_NO_CHECK);
    }
#endif/*MBB_HSUART*/
    return 0;
}

/*****************************************************************************
* �� �� ��  : uart_set_wlen
* ��������  : UART ��������λ������
* �������  : uart_dev: �豸����ṹָ��
*             w_len: ����λ��
* �������  :
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
static int uart_set_wlen(uart_port *port, int w_len)
{
	u32 val = 0;
    u32 wlen = 0;
    
    dialup_info("set wlen: %d\n", w_len);
	val = readl((const volatile void *)(port->vir_addr_base + UART011_LCRH));
    switch(w_len)
    {
        case WLEN_5_BITS:
            wlen = UART01x_LCRH_WLEN_5;
            break;
        case WLEN_6_BITS:
            wlen = UART01x_LCRH_WLEN_6;
            break;
        case WLEN_7_BITS:
            wlen = UART01x_LCRH_WLEN_7;
            break;
        case WLEN_8_BITS:
            wlen = UART01x_LCRH_WLEN_8;
            break;
        default:
            dialup_error("unsupport w_len:%d\n", w_len);
            return ERROR;
    }
    val &= (~UART01x_LCRH_WLEN_8); 
    val |= (unsigned int)wlen;
    (void)writel(val, (volatile void *)(port->vir_addr_base + UART011_LCRH));
    return OK;
}
/*****************************************************************************
* �� �� ��  : uart_set_stp2
* ��������  : UART ����ֹͣλ������
* �������  : uart_dev: �豸����ṹָ��
*             stp2: ����ֹͣλ����
* �������  :
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
static int uart_set_stpbit(uart_port *port, int stp2)
{
    u32 val = 0;  
    dialup_info("set stp2: %d\n", stp2);
	val = readl((const volatile void *)(port->vir_addr_base + UART011_LCRH));
    if(stp2 == STP2_OFF)
    {
		val &= ~UART01x_LCRH_STP2;
    }
   	else if(stp2 == STP2_ON)
   	{
        val |= UART01x_LCRH_STP2;
   	}
	else
	{
		dialup_error("unsupport stp2:%d\n", stp2);
        return ERROR;
    }
	(void)writel(val, (volatile void *)(port->vir_addr_base + UART011_LCRH));
    return OK;
}
/*****************************************************************************
* �� �� ��  : uart_set_eps
* ��������  : UART ����У�鷽ʽ����
* �������  : uart_dev: �豸����ṹָ��
*             eps: У�鷽ʽ
* �������  :
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
static int uart_set_eps(uart_port *port, int epscmd)
{
    u32 val = 0;
	dialup_info("set eps: %d\n", epscmd);
	val = readl((const volatile void *)(port->vir_addr_base + UART011_LCRH));
	switch(epscmd)
	{
		case PARITY_NO_CHECK:			/*��У��λ*/
			val &= ~UART01x_LCRH_PEN;
			break;
		case PARITY_CHECK_ODD: 			/*��У��*/
            val |= UART01x_LCRH_PEN;
			val &= ~UART01x_LCRH_EPS;
    		break;	
    	case PARITY_CHECK_EVEN:			/*żУ��*/
			val |= (UART01x_LCRH_PEN | UART01x_LCRH_EPS);
			break;	
		case PARITY_CHECK_MARK:
            val |= (UART01x_LCRH_PEN | UART011_LCRH_SPS);
            val &= ~UART01x_LCRH_EPS;
            break;
		case PARITY_CHECK_SPACE:
			val |= (UART01x_LCRH_PEN | UART011_LCRH_SPS | UART01x_LCRH_EPS);
			break;
		default:
			dialup_error("unsupport eps:%d\n", epscmd);
        	return ERROR;
			break;
	}
	g_dialup_ctx.feature.eps = val;
	(void)writel(val, (volatile void *)(port->vir_addr_base + UART011_LCRH));
    return OK;
}

/*****************************************************************************
* �� �� ��  : uart_alloc_skb
* ��������  :����SKB
* �������  : pMemNode ���ݽڵ�size �����С
* �������  : NA
* �� �� ֵ  : ʧ��/�ɹ�
*****************************************************************************/
s32 dialup_alloc_skb(skb_mem_str* pMemNode, u32 size)
{
	pMemNode->pSkb = dev_alloc_skb(size);   
	if (NULL == pMemNode->pSkb)
    {
        dialup_error("pMemNode buff alloc fail, size:%d\n", size);
        return ERROR;
    }
    pMemNode->mem.vaddr = (void*)pMemNode->pSkb->data;
  	pMemNode->mem.paddr = dma_map_single(NULL, pMemNode->mem.vaddr, size, DMA_FROM_DEVICE);
    (void)memset(pMemNode->mem.vaddr, 0, size);
    return OK;
}
/*****************************************************************************
* �� �� ��  : add_node_to_freelist
* ��������  : UART UDI ��ӽڵ㵽 FreeList ��
* �������  : trans_info: ��д�ṹ��
*             pMemNode: ����ӵĽڵ�
* �������  : NA
* �� �� ֵ  : NA
*****************************************************************************/
void add_node_to_freelist(skb_buff_node *trans_info, skb_mem_str* pMemNode)
{
    unsigned long flags = 0;   
    spin_lock_irqsave(&trans_info->list_lock, flags); 
    list_add_tail(&pMemNode->list_node, &(trans_info->free_list));
    /*ȥ��ʼ��pMemNode��Ա*/
    pMemNode->state = UART_MEM_IN_FREELIST;
    pMemNode->valid_size = 0;
    pMemNode->pSkb = NULL;
    pMemNode->mem.vaddr = (void*)NULL;
    pMemNode->mem.paddr = (unsigned int)NULL;  
	trans_info->free_node_cnt++;
    spin_unlock_irqrestore(&trans_info->list_lock, flags);
}
/*****************************************************************************
* �� �� ��  : get_node_from_freelist
* ��������  : UART UDI ��FreeList ͷȡ��һ���ڵ�
* �������  : trans_info: ��д�ṹ��
* �������  : NA
* �� �� ֵ  : ���صĽڵ�ָ��
*****************************************************************************/
skb_mem_str* get_node_from_freelist(skb_buff_node *trans_info)
{
    struct list_head *pListNode;
    skb_mem_str* pMemNode;
    unsigned long flags = 0;  
    spin_lock_irqsave(&trans_info->list_lock, flags);  
    if (list_empty(&(trans_info->free_list)))
    {
        pMemNode = NULL;
    }
    else
    {
        pListNode = trans_info->free_list.next;
        pMemNode = list_entry(pListNode, skb_mem_str, list_node);
        
        list_del_init(pListNode);
        pMemNode->state = UART_MEM_NOT_IN_LIST;
    }   
    spin_unlock_irqrestore(&trans_info->list_lock, flags);   
    return pMemNode;
}
/*****************************************************************************
* �� �� ��  : add_node_to_donelist
* ��������  : UART UDI ��ӽڵ㵽 ������� ��
* �������  : trans_info: uart ��д�ṹ��ָ��
*             pMemNode: ����ӵĽڵ�
* �������  : NA
* �� �� ֵ  : NA
*****************************************************************************/
void add_node_to_donelist(skb_buff_node *trans_info, skb_mem_str* pMemNode)
{
	unsigned long flags = 0;

    spin_lock_irqsave(&trans_info->list_lock, flags);
    list_add_tail(&pMemNode->list_node, &(trans_info->done_list));
    trans_info->done_cnt++;
	trans_info->free_node_cnt--;
    pMemNode->state = UART_MEM_IN_DONELIST;    
    spin_unlock_irqrestore(&trans_info->list_lock, flags);
}
/*****************************************************************************
* �� �� ��  : get_node_from_donelist
* ��������  : UART UDI ��DoneList ͷȡ��һ���ڵ�
* �������  : trans_info: ��д�ṹ��
* �������  : NA
* �� �� ֵ  : ���صĽڵ�ָ��
*****************************************************************************/
skb_mem_str* get_node_from_donelist(skb_buff_node *trans_info)
{
    struct list_head *pListNode = NULL;
    skb_mem_str* pMemNode = NULL;
    unsigned long flags = 0;

    spin_lock_irqsave(&trans_info->list_lock, flags);   
    if (list_empty(&(trans_info->done_list)))
    {
		pMemNode = NULL;
    }
    else
    {    
        pListNode = trans_info->done_list.next;
        pMemNode = list_entry(pListNode, skb_mem_str, list_node);
        list_del_init(pListNode);
        trans_info->done_cnt--;
        pMemNode->state = UART_MEM_NOT_IN_LIST;
    }    
    spin_unlock_irqrestore(&trans_info->list_lock, flags); 
    return pMemNode;
}

/*****************************************************************************
* �� �� ��  : uart_init_list
* ��������  : ��ʼ����д����
* �������  : trans_info: ��д�ṹ��
* �������  : NA
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
s32 diaup_node_init(skb_buff_node *node)
{
    u8* pMemHead = NULL;
    u32 idx = 0;
    s32 ret = OK;
    unsigned long flags = 0;
	struct list_head *pListNode = NULL;
	
    spin_lock_irqsave(&node->list_lock, flags);//�����ź���
    INIT_LIST_HEAD(&(node->free_list));
    INIT_LIST_HEAD(&(node->done_list));
    
    pMemHead = osl_malloc(node->node_num*sizeof(skb_mem_str));
    if (pMemHead)
    {
        (void)memset(pMemHead, 0, (node->node_num*sizeof(skb_mem_str)));
        node->pmem_start = (skb_mem_str*)pMemHead;
        node->pmem_end = (skb_mem_str*)(pMemHead + (node->node_num*sizeof(skb_mem_str)));
        node->pcur_pos = NULL;
        node->done_cnt = 0;
        node->total_bytes = 0;

        /* ��ʼ��ʱ,���нڵ㶼��FreeList */
        for (idx = 0; idx < node->node_num; idx++)
        {
            pListNode = &(node->pmem_start[idx].list_node);
            list_add_tail(pListNode, &(node->free_list));
            node->pmem_start[idx].state = UART_MEM_IN_FREELIST;
            node->pmem_start[idx].valid_size = 0;
            (void)memset(&(node->pmem_start[idx].mem), 0, sizeof(struct mem_desc_s));
            node->pmem_start[idx].pSkb = NULL;
        }
    }
    else
    {
        ret = ERROR;
    }
    spin_unlock_irqrestore(&node->list_lock, flags);

    return ret;
}
/*****************************************************************************
* �� �� ��  : hsuart_buf_init
* ��������  :��ʼ����д�������
* �������  : uart_ctx ������
* �������  : NA
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
s32 dialup_buf_init(dialup_ctx *uart_ctx)
{
    s32 ret = OK;
	
    memset(&(uart_ctx->read_node), 0, sizeof(skb_buff_node));
    memset(&(uart_ctx->write_node), 0, sizeof(skb_buff_node));
    
    spin_lock_init(&(uart_ctx->read_node.list_lock));
    spin_lock_init(&(uart_ctx->write_node.list_lock));

    uart_ctx->read_node.node_num = UART_READ_BUF_NUM;   //���ڵ�����16
    uart_ctx->read_node.node_size = UART_READ_BUF_SIZE; //ÿ���ڵ��С1536�ֽ�
    uart_ctx->write_node.free_node_cnt = UART_WRITE_BUF_NUM;
    uart_ctx->write_node.node_num = UART_WRITE_BUF_NUM; //д�ڵ�����64
    uart_ctx->write_node.node_size = 0;					//ÿ��д�ڵ��С0
	if (diaup_node_init(&(uart_ctx->read_node)) || diaup_node_init(&(uart_ctx->write_node)))
    {
        dialup_error("node init error.\n");
        ret = ERROR;
    }
    return ret;
}
/*****************************************************************************
* �� �� ��  : gpio_dtr_irq
* ��������  : GPIO �жϴ���
* �������  : irq �жϺ� dev_id ����
* �������  : NA
* �� �� ֵ  : �ɹ�/ʧ��bcmsdh_register_oob_intr
*****************************************************************************/
static irqreturn_t gpio_dtr_irq(int irq, void *dev_id)
{
	dialup_ctx* uart_ctx = (dialup_ctx*)dev_id;
    MODEM_MSC_STRU modem_msc = {0};

    memset(&modem_msc, 0, sizeof(MODEM_MSC_STRU));
    modem_msc.OP_Dtr = SIGNALCH;
    modem_msc.ucDtr = !(u8)gpio_get_value(g_dialup_ctx.gpio.dtr);
    dialup_info("gpio_dtr_irq:%d\n", modem_msc.ucDtr);
	if (uart_ctx->cbs.msc_read_cb)
    {
        uart_ctx->cbs.msc_read_cb(&modem_msc);
    }
    else
    {
        dialup_error("value:%d  msc_read_cb is NULL.\n", modem_msc.ucDtr);
    }
	
    return IRQ_HANDLED;
}
/*****************************************************************************
* �� �� ��  : dialup_gpio_init
* ��������  : uart gpio ģ��ܽų�ʼ������
* �������  : uart_ctx ������
* �������  : NA
* �� �� ֵ  : �ɹ�/ʧ��bcmsdh_register_oob_intr
* ��	ע	:����gpio�����ֲ�ͨ������
*****************************************************************************/
s32 dialup_gpio_init(dialup_ctx *uart_ctx)
{
    s32 ret;
	
    if (gpio_request(g_dialup_ctx.gpio.ring, "GPIO_RING"))
    {
        dialup_error("gpio=%d is busy\n", g_dialup_ctx.gpio.ring);
        return ERROR;
    }
    if (gpio_request(g_dialup_ctx.gpio.dsr, "GPIO_DSR"))
    {
        dialup_error("gpio=%d is busy\n", g_dialup_ctx.gpio.dsr);
        return ERROR;
    }
    if (gpio_request(g_dialup_ctx.gpio.dcd, "GPIO_DCD"))
    {
        dialup_error("gpio=%d is busy\n", g_dialup_ctx.gpio.dcd);
        return ERROR;
    } 
    gpio_direction_output(g_dialup_ctx.gpio.ring, 1); 
    gpio_direction_output(g_dialup_ctx.gpio.dsr, 1); 
    gpio_direction_output(g_dialup_ctx.gpio.dcd, 1); 
    
    if (gpio_request(g_dialup_ctx.gpio.dtr, "GPIO_DTR"))
    {
        dialup_error("gpio=%d is busy\n", g_dialup_ctx.gpio.dtr);
        return ERROR;
    }  
    gpio_direction_input(g_dialup_ctx.gpio.dtr);     

    ret = request_irq(gpio_to_irq(g_dialup_ctx.gpio.dtr), gpio_dtr_irq, (IRQF_SHARED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING),"dialup dtr", (void*)uart_ctx);
    if (ret != 0) 
	{
        dialup_error("register irq err\n");
        return ERROR;
    }   
    return OK;
}
/*****************************************************************************
* �� �� ��  : uart_dma_tx_complete
* ��������  : UART DMA ģʽ������ɺ���
* �������  : arg: �豸����ṹָ��
*             int_status: �ж�״̬
* �������  :
* �� �� ֵ  : NA
*****************************************************************************/
void uart_dma_tx_complete(u32 arg, u32 int_status)
{
    dialup_ctx *uart_ctx = (dialup_ctx *)arg;
    osl_sem_up(&uart_ctx->tx_endsem);
}
/*****************************************************************************
* �� �� ��  : uart_dma_tx_config
* ��������  : UART DMA ģʽ�������ݺ���
* �������  : uart_dev: �豸����ṹָ��
*             dma_handle: �������ݻ����ַ
*             length: �������ݳ���
* �������  :
* �� �� ֵ  : �������ݳ���
*****************************************************************************/
s32 uart_dma_tx_config(uart_port *port, u32 dma_handle, u32 length)
{
    s32 ret_id = 0;
    dialup_ctx *uart_ctx = NULL;

    uart_ctx = container_of(port, dialup_ctx, port);
	ret_id = bsp_edma_channel_init(port->dma_req_tx, uart_dma_tx_complete, (u32)uart_ctx, BALONG_DMA_INT_DONE);
	if (ret_id < 0)
    {
        dialup_error("dma error ret_id = %d\n", ret_id);
        return ERROR;
    }
    if (bsp_edma_channel_set_config(ret_id, BALONG_DMA_M2P, EDMA_TRANS_WIDTH_8, EDMA_BUR_LEN_16))
    {
        dialup_error("dma config err ret_id = %d\n", ret_id);
        return ERROR;
    }
    if (bsp_edma_channel_async_start(ret_id, dma_handle, (u32)(port->phy_addr_base + UART01x_DR), length))
    {
        dialup_error("dma start err ret_id = %d\n", ret_id);
        return ERROR;
    }
    return length;
}
/*******************************************************************************
* �� �� ��  : switch_stage_three
* ��������  : ģʽ�л������׶�
* �������  : arg ����
* �������  : NA
* �� �� ֵ  : NA
*******************************************************************************/
void switch_stage_three(u32 arg)
{
    dialup_ctx *uart_ctx = (dialup_ctx *)arg;
    skb_mem_str* pMemNode = NULL;
#if ( FEATURE_ON == MBB_HSUART )
#else
    pMemNode = uart_ctx->read_node.pcur_pos;
	pMemNode->pSkb->len -= 3;		
	pMemNode->valid_size = 0;
#endif
    uart_ctx->stage = DETECT_STAGE_1;
	if (uart_ctx->cbs.switch_mode_cb)
 	{
      	uart_ctx->cbs.switch_mode_cb();
		dialup_info("switch cmd mode\n");
  	}
  	else
	{
        dialup_error("switch_mode_cb is NULL.\n");
    }
}
#if (FEATURE_ON == MBB_HSUART)
extern int AT_QueryHSUartRawDataMode();
#endif /*MBB_HSUART*/
/*****************************************************************************
* �� �� ��  : switch_mode_cmd_detect
* ��������  : ģʽ�л������⺯��
* �������  : uart_ctx ������ pMemNode ���ݽڵ�ָ��
* �������  : NA
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
s32 switch_mode_cmd_detect(dialup_ctx *uart_ctx, skb_mem_str* pMemNode)
{
    int result = 0;
	
    if (DETECT_STAGE_2 == uart_ctx->stage)
    {
        if (pMemNode)	/*�����ݽڵ�������Ϊ"+++"*/
        {
			if((pMemNode->valid_size <= 3) 
				&& (!memcmp((void*)switch_cmd, (void*)pMemNode->mem.vaddr, pMemNode->valid_size)))
			{
				g_hsuart_ctrl.switch_cmdstr_cnt += pMemNode->valid_size;
				if(g_hsuart_ctrl.switch_cmdstr_cnt > 3)
				{
					dialup_debug("STAGE_2 to STAGE_1\n");
					g_hsuart_ctrl.switch_cmdstr_cnt = 0;
	            	uart_ctx->stage = DETECT_STAGE_1;
					return 0;
				}
				else if(g_hsuart_ctrl.switch_cmdstr_cnt == 3)
        		{
					uart_ctx->stage = DETECT_STAGE_3;
					dialup_debug("STAGE_2 to STAGE_3\n");
	            	bsp_softtimer_add(&g_hsuart_ctrl.switch_state3_timer);
					g_hsuart_ctrl.switch_cmdstr_cnt = 0;
#if (FEATURE_ON == MBB_HSUART)
                    /*���ַ�UART͸��ģʽ��ԭ�����߼���
                    �൱���ڷ�͸��ģʽ��"+++"�ᱻ������ֻ�޸�͸��ģʽ�µ��߼�*/
                    result = AT_QueryHSUartRawDataMode();
                    return result;
#else
                    return 1;
#endif /*MBB_HSUART*/
        		}
				else if(g_hsuart_ctrl.switch_cmdstr_cnt < 3)
				{
					dialup_debug("STAGE_2 wait\n");
					
					pMemNode->mem.vaddr = ((unsigned char *)pMemNode->mem.vaddr) + pMemNode->valid_size;
#if (FEATURE_ON == MBB_HSUART)
                    /*���ַ�UART͸��ģʽ��ԭ�����߼���
                    �൱���ڷ�͸��ģʽ��"+++"�ᱻ������ֻ�޸�͸��ģʽ�µ��߼�*/
                    result = AT_QueryHSUartRawDataMode();
                    return result;
#else
                    return 1;
#endif /*MBB_HSUART*/
				}
			}
        	else
        	{
				dialup_debug("DETECT_STAGE_2 to DETECT_STAGE_1\n");
	            uart_ctx->stage = DETECT_STAGE_1;
				g_hsuart_ctrl.switch_cmdstr_cnt = 0;
        	}
        }
    }
    else
    {
		uart_ctx->stage = DETECT_STAGE_1;
	}
    return result;
}
/*****************************************************************************
* �� �� ��  : wait_tx_fifo_empty
* ��������  : UART �ȴ��������
* �������  : ctx ����
* �������  : NA
* �� �� ֵ  : NA
*****************************************************************************/
void wait_tx_fifo_empty(dialup_ctx *uart_ctx)
{
    u32 status = 0;
	u32 u32Times = 0;

    status = readl((const volatile void *)uart_ctx->port.vir_addr_base + UART01x_FR);
    while (0 == (status & UART011_FR_TXFE))
    {
        msleep(1);
        if (10000 < u32Times++)
        {
			dialup_debug("hsuart line busy\n");
            break;
        }
        status = readl((const volatile void *)uart_ctx->port.vir_addr_base + UART01x_FR);
    }
}
/*****************************************************************************
* �� �� ��  : set_hsuart_feature
* ��������  : uart ��������
* �������  : uart_ctx:uart������
* �������  : NA
* �� �� ֵ  : NA
*****************************************************************************/
void uart_reset_termios(dialup_ctx *uart_ctx)
{
    unsigned long flags = 0;
    
    if (uart_ctx->cbs.set_baud_cb || uart_ctx->cbs.set_wlen_cb || uart_ctx->cbs.set_stp2_cb || uart_ctx->cbs.set_eps_cb)
    {
        wait_tx_fifo_empty(uart_ctx);
        uart_disable(&uart_ctx->port);
		spin_lock_irqsave(&(uart_ctx->port.lock_irq), flags);
        if (uart_ctx->cbs.set_baud_cb)
        {
            uart_ctx->cbs.set_baud_cb(&uart_ctx->port, uart_ctx->feature.baud);
            uart_ctx->cbs.set_baud_cb = NULL;
        }
        if (uart_ctx->cbs.set_wlen_cb)
        {
            uart_ctx->cbs.set_wlen_cb(&uart_ctx->port, uart_ctx->feature.wlen);
            uart_ctx->cbs.set_wlen_cb = NULL;
        }
        if (uart_ctx->cbs.set_stp2_cb)
        {
            uart_ctx->cbs.set_stp2_cb(&uart_ctx->port, uart_ctx->feature.stp2);
            uart_ctx->cbs.set_stp2_cb = NULL;
        }
        if (uart_ctx->cbs.set_eps_cb)
        {
            uart_ctx->cbs.set_eps_cb(&uart_ctx->port, uart_ctx->feature.eps);
            uart_ctx->cbs.set_eps_cb = NULL;
        }
        spin_unlock_irqrestore(&(uart_ctx->port.lock_irq), flags);
        uart_enable(&uart_ctx->port);
    }
}
/*******************************************************************************
* �� �� ��  : get_read_done_node
* ��������  : UART ��ȡ��ǰд��ַ
* �������  : uart_ctx ������
* �������  : NA
* �� �� ֵ  : д��ַ
*******************************************************************************/
skb_mem_str* get_free_node(dialup_ctx *uart_ctx)
{
    skb_mem_str* pMemNode;
	
    if (NULL == uart_ctx->read_node.pcur_pos)
    {
        pMemNode = get_node_from_freelist(&(uart_ctx->read_node));
        if (pMemNode)
        {
            if (!pMemNode->pSkb)
            {
                if (OK != dialup_alloc_skb(pMemNode, uart_ctx->read_node.node_size))
                {
                    add_node_to_freelist(&(uart_ctx->read_node), pMemNode);
                    pMemNode = NULL;
                    uart_ctx->stat.alloc_skb_fail++;
                }
				uart_ctx->stat.alloc_skb_cnt++;
            }
            uart_ctx->read_node.pcur_pos = pMemNode;
        }
    }
    else
    {
        pMemNode = uart_ctx->read_node.pcur_pos;        
    }
    
    return pMemNode;
}
/*******************************************************************************
* �� �� ��  : check_mem_to_donelist
* ��������  : �������ݼ��
* �������  : uart_ctx ������ pMemNode �ڴ�ڵ�
* �������  : NA
* �� �� ֵ  : NA
*******************************************************************************/
s32 check_mem_to_donelist(dialup_ctx *uart_ctx, skb_mem_str* pMemNode)
{
    s32 ret = ERROR;
	u32 loop = 0;
	//u32 lenth = 0;
	u8 *pchar = NULL;
	u32 i = 0;
#if ( FEATURE_ON == MBB_HSUART )
    static unsigned int adaptSuccessFlag = 0;

    if ((uart_ctx->baud_adapt) || (0 == allow_recv_flag))
#else
    if (uart_ctx->baud_adapt)
#endif/*MBB_HSUART*/
    {
		pchar = (u8 *)(pMemNode->mem.vaddr);
        for(i = 0; i < pMemNode->valid_size; i++)
        {
           	dialup_debug("0x%x ",pchar[i]);
        }
#if ( FEATURE_ON == MBB_HSUART )
        /*����Ӧ�ѳɹ�������δ����λ�������ź�ǰ�����յ����ݶ���Чֱ�Ӷ���*/
        if(1 == adaptSuccessFlag)
        {
            kfree_skb(pMemNode->pSkb);
            /*�������ݽڵ�Ż�Freelist��������������*/
            add_mem_to_freelist(&uart_ctx->read_info, pMemNode);
            uart_ctx->read_info.pcur_pos = NULL;
            hsuart_error("adapt success but still not return OK... discard data  !! \n");
            return ret;
        }

#endif/*MBB_HSUART*/
        for(loop = 0;loop < sizeof(g_sample_table)/sizeof(struct baudsample); loop++)
		{
			
			if(!memcmp(g_sample_table[loop].ch , pMemNode->mem.vaddr ,g_sample_table[loop].len))
			{
				uart_ctx->feature.baud = g_sample_table[loop].bandrate;
#if ( FEATURE_ON == MBB_HSUART )
                /*����Ӧ�ɹ������ñ�־λ*/
                adaptSuccessFlag = 1;
                /*��������Ӧ������*/
                dev_kfree_skb(pMemNode->pSkb);
                add_mem_to_freelist(&(uart_ctx->read_info), pMemNode);
                uart_ctx->read_info.pcur_pos = NULL;
                pMemNode->valid_size = 0;
                /*��ʱ�������ò���������ɹ�*/
                bsp_softtimer_add(&g_hsuart_ctrl.adapt_timer);
#else
                if(OK != uart_set_baud(&uart_ctx->port, uart_ctx->feature.baud))
                {
                    break;
                }
                uart_ctx->baud_adapt = DIALUP_FALSE;
                add_node_to_donelist(&uart_ctx->read_node, pMemNode);
#endif
                dialup_info("bandrate adapt success!  bandrate is %d \n", g_sample_table[loop].bandrate);
				return OK;
			}
			else
			{
				dialup_debug("dump next baud adapt : %d\n",g_sample_table[loop].bandrate);
			}
		}
		if(loop == sizeof(g_sample_table)/sizeof(struct baudsample))
		{
			dialup_error("baud adapt fail\n");
			dev_kfree_skb(pMemNode->pSkb);
			add_node_to_freelist(&(uart_ctx->read_node),pMemNode);
			uart_ctx->read_node.pcur_pos = NULL;
            pMemNode->valid_size = 0;
		}
		
    }
    else
    {
        if (!switch_mode_cmd_detect(uart_ctx, pMemNode))
        {
            add_node_to_donelist(&uart_ctx->read_node, pMemNode);
            ret = OK;
        }
#if ( FEATURE_ON == MBB_HSUART )
        else
        {
            kfree_skb(pMemNode->pSkb);
            /*�������ݽڵ�Ż�Freelist��������������*/
            add_mem_to_freelist(&uart_ctx->read_info, pMemNode);
            uart_ctx->read_info.pcur_pos = NULL;
        }
#endif/*MBB_HSUART*/
    }
    return ret;
}
/*****************************************************************************
* �� �� ��  : uart_dma_tx_complete
* ��������  : UART DMA ģʽ������ɺ���
* �������  : arg: �豸����ṹָ��
*             int_status: �ж�״̬
* �������  :
* �� �� ֵ  : NA
*****************************************************************************/
void hsuart_dma_rx_complete(u32 arg, u32 int_status)
{
	dialup_ctx *uart_ctx = &g_dialup_ctx;
	
	disable_irq_nosync(uart_ctx->port.irq);
	g_dma_ctrl.total_size = uart_ctx->read_node.node_size;
    osl_sem_up(&g_dma_ctrl.edma_rx_sem);
	dialup_info("UART RX DMA transfer complete\n");
}
/*****************************************************************************
* �� �� ��  : uart_rx_dma_config
* ��������  : UART DMA ģʽ�������ݺ���
* �������  : uart_dev: �豸����ṹָ��
*             dma_handle: �������ݻ����ַ
*             length: �������ݳ���
* �������  :
* �� �� ֵ  : DMA channel id
*****************************************************************************/
s32 uart_rx_dma_config(uart_port *port, u32 dma_handle, u32 length)
{
    s32 ret_id = 0;
    dialup_ctx *uart_ctx = NULL;

    uart_ctx = container_of(port, dialup_ctx, port);
    ret_id = bsp_edma_channel_init(port->dma_req_rx, hsuart_dma_rx_complete, (u32)uart_ctx, BALONG_DMA_INT_DONE);
    if (ret_id < 0)
    {
        dialup_error("hsuart init dma err, ret_id = %d\n", ret_id);
        return ERROR;
    }
    if (bsp_edma_channel_set_config(ret_id, BALONG_DMA_P2M, EDMA_TRANS_WIDTH_8, EDMA_BUR_LEN_16))
    {
        dialup_error("hsuart config dma err, ret_id = %d\n", ret_id);
        return ERROR;
    }  
    if (bsp_edma_channel_async_start(ret_id, (u32)(port->phy_addr_base + UART01x_DR), dma_handle, length))
    {
        dialup_error("hsuart start dma err, ret_id = %d\n", ret_id);
        return ERROR;
    } 
    return ret_id;
}
/*******************************************************************************
* �� �� ��  : uart_rx_complete
* ��������  : �������
* �������  : uart_ctx ������
* �������  : NA
* �� �� ֵ  : NA
*******************************************************************************/
void uart_rx_complete(dialup_ctx *uart_ctx)
{
    skb_mem_str* pMemNode;
    pMemNode = uart_ctx->read_node.pcur_pos;
    if (!pMemNode)
    {
		dialup_error("pMemNode is null!\n");
		return;
    }
    
   	if (uart_ctx->cbs.read_cb)
   	{
       	skb_put(pMemNode->pSkb, pMemNode->valid_size);		/*ʹ�ñ�׼��API���޸�len */
		dialup_debug("put pMemNode :0x%x\n",(u32)pMemNode);
		dialup_debug("put skb_data addr:0x%x\n",(u32)pMemNode->pSkb->data);
		dialup_debug("put skb_len:0x%x\n",(u32)pMemNode->pSkb->len);
		if (OK == check_mem_to_donelist(uart_ctx, pMemNode))
        {
            uart_ctx->read_node.total_bytes += pMemNode->valid_size;
            uart_ctx->cbs.read_cb();
            uart_ctx->read_node.pcur_pos = NULL;
        }
    }
    else
    {
       dialup_error("ReadCb is null.\n");
       pMemNode->valid_size = 0;
    }
}
/*****************************************************************************
* �� �� ��  : uart_dma_burst_transfer
* ��������  : UART dma���ݰ���
* �������  : uart_ctx ��pMemNode
* �������  : NA
* �� �� ֵ  : NA
*****************************************************************************/
void uart_dma_burst_transfer(dialup_ctx *uart_ctx, skb_mem_str* pMemNode)
{
    u32 start_addr = 0;
	uart_port *port = &uart_ctx->port;
	u32 switch_scurrent_time = 0;
	u32 switch_delta_time = 0;
	u32 switch_last_time = 0;
	
	pMemNode->mem.paddr = dma_map_single(NULL, pMemNode->mem.vaddr , uart_ctx->read_node.node_size,DMA_FROM_DEVICE);
	
    start_addr = (u32)pMemNode->mem.paddr;
	
	g_dma_ctrl.pMemNode_startaddr = (u32)pMemNode->mem.paddr;
	g_dma_ctrl.pMemNode_viraddr = (u32)pMemNode->mem.vaddr;
	g_dma_ctrl.channel_id = uart_rx_dma_config(port, start_addr, uart_ctx->read_node.node_size);
	g_dma_ctrl.dma_run_flag = DIALUP_TRUE;
	
	enable_irq(port->irq);
	switch_scurrent_time = bsp_get_slice_value();

	osl_sem_down(&g_dma_ctrl.edma_rx_sem);
	
#if (FEATURE_ON == MBB_MODULE_PM)
    hsuart_sleep_lock();
    hsuart_com_jiff = jiffies;
#endif
	switch_last_time = bsp_get_slice_value();
	switch_delta_time = get_timer_slice_delta(switch_scurrent_time ,switch_last_time);
	dialup_debug("switch_scurrent_time %d\n",switch_delta_time);
	switch_delta_time = (switch_delta_time/SYS_TICK_32K)*1000;
	if(switch_delta_time >= SWITCH_TIMER_LENGTH && uart_ctx->stage == DETECT_STAGE_1)
	{
		uart_ctx->stage = DETECT_STAGE_2;
		dialup_debug("stage 2\n");
	}
	
	else if(switch_delta_time >= SWITCH_TWO_TIMER_LENGTH && uart_ctx->stage == DETECT_STAGE_2)
	{
		uart_ctx->stage = DETECT_STAGE_1;
		g_hsuart_ctrl.switch_cmdstr_cnt = 0;
		dialup_debug("change stage 1\n");
	}
	pMemNode->valid_size =  g_dma_ctrl.total_size;
	g_dma_ctrl.dma_run_flag = DIALUP_FALSE;
}
/*****************************************************************************
* �� �� ��  : hsuart_rx_thread
* ��������  : UART �ײ�����߳�
* �������  : ctx ����
* �������  : NA
* �� �� ֵ  : NA
*****************************************************************************/
static int dialup_rx_thread(void *ctx)
{
    dialup_ctx *uart_ctx = (dialup_ctx *)ctx;
    skb_mem_str* pMemNode = NULL;
    
	/* coverity[no_escape] */
    while (1)
    {
		if (!uart_ctx->rts)
        {
            pMemNode = get_free_node(uart_ctx);
            if (pMemNode)
            {
                while (uart_ctx->read_node.pcur_pos)
                {
                    uart_dma_burst_transfer(uart_ctx, pMemNode);									
					bsp_softtimer_delete(&g_hsuart_ctrl.switch_state3_timer);
                    if (DETECT_STAGE_3 == uart_ctx->stage)
                    {
                        uart_ctx->stage = DETECT_STAGE_1;
						g_hsuart_ctrl.switch_cmdstr_cnt = 0;
						dialup_debug("STAGE_3 to STAGE_1\n");
                    }

                    if (pMemNode->valid_size)
                    {
                        dialup_info("valid_size:0x%x\n", (u32)pMemNode->valid_size);
                        uart_rx_complete(uart_ctx);
                    }
                }
				dialup_info("**********\n");
            }
            else
            {
                msleep(1);
				dialup_debug("pmemnode null\n");
            }
        }
        else
        {
            msleep(1);
			dialup_debug("rx thread idle\n");
        }   
  }
   
    return 0;
}
/*****************************************************************************
* �� �� ��  : hsuart_tx_thread
* ��������  : UART �ײ㷢���߳�
* �������  : ctx ����
* �������  : NA
* �� �� ֵ  : NA
*****************************************************************************/
static int dialup_tx_thread(void *ctx)
{
    dialup_ctx *uart_ctx = (dialup_ctx *)ctx;
	uart_port *uart_dev = &uart_ctx->port;    

    skb_mem_str* pMemNode = NULL;
#if(FEATURE_ON == MBB_HSUART)
    skb_mem_str* resultMemNode = NULL;
    unsigned long flags = 0;
#endif/*lint !e578*/
	if(NULL == ctx)
	{
       dialup_error("uart_ctx is NULL\n");
	   return ERROR;
	}
	/* coverity[no_escape] */
    while (1)
    {
        if (!uart_ctx->baud_adapt)
        {
            osl_sem_down(&(uart_ctx->tx_reqsem));
			if(g_dma_ctrl.resume_flag == DIALUP_FALSE)
			{
				while(1)
				{
					msleep(1);
					if(g_dma_ctrl.resume_flag == DIALUP_TRUE)
					{
						break;
					}
				}
			}
            pMemNode = get_node_from_donelist(&(uart_ctx->write_node));
            while (pMemNode)
            {
				dialup_info("enter. pSkb:0x%x, valid_size:0x%x\n", (unsigned int)pMemNode->pSkb, (unsigned int)pMemNode->valid_size);//debug
				uart_dma_tx_config(uart_dev, (u32)pMemNode->mem.paddr, pMemNode->valid_size); 				
				osl_sem_down(&uart_ctx->tx_endsem); 
				uart_ctx->write_node.cur_bytes -= pMemNode->valid_size;
                if (uart_ctx->cbs.free_cb)
                {
                    uart_ctx->cbs.free_cb((char*)pMemNode->pSkb);
					if (pMemNode->pSkb)
					{
						pMemNode->pSkb = NULL;
					}
                }				
                else
                {
                    uart_ctx->stat.write_not_free_cnt++;
                    dialup_error("no write free func register\n");
                    kfree_skb(pMemNode->pSkb);
					pMemNode->pSkb = NULL;
                }
                add_node_to_freelist(&(uart_ctx->write_node), pMemNode);

				if(NULL != uart_ctx->cbs.water_ops)
				{
					if((g_hsuart_ctrl.water_low_level_flag == DIALUP_TRUE) && 
						((uart_ctx->write_node.cur_bytes < free_node[g_hsuart_ctrl.cur_baud_index].low_level 
					    && uart_ctx->write_node.free_node_cnt > HSUART_WATER_NODE_LOW)))
					{
						uart_ctx->cbs.water_ops(LOW_LEVEL);
						g_hsuart_ctrl.water_low_level_flag = DIALUP_FALSE;
					}
				}
				else
				{
					dialup_error("water_ops null\n");
				}
                pMemNode = get_node_from_donelist(&(uart_ctx->write_node));
            }
            uart_reset_termios(uart_ctx);		/* NAS�յ�AT������ظ��Է�OK��ʼ����uart���� */
        }
        else
        {
#if( FEATURE_ON == MBB_HSUART )/*lint -e516 -e160 -e10 -e522 -e666*/
            osl_sem_down(&(uart_ctx->tx_sema));
            if (1 == adapt_success_flag)
            {
                /*����ڵ��SKB�洢�ռ�*/
                resultMemNode = kmalloc(sizeof(uart_mem_node), GFP_ATOMIC);
                if (resultMemNode)
                {
                    (void)memset(resultMemNode, 0, sizeof(uart_mem_node));
                    if (BSP_OK != hsuart_alloc_skb(resultMemNode, (BAND_ADAPT_LENTH + 2)))
                    {
                        hsuart_error("hsuart_tx_thread: hsuart_alloc_skb resultNode fail !!! \n");
                        /*coverity�޸ģ������ڴ�й¶�ķ���*/
                        kfree(resultMemNode);
                        msleep(10);
                        continue;
                    }
                    else
                    {
                        hsuart_error("hsuart_tx_thread: hsuart_alloc_skb resultNode success !!! \n");
                    }
                }
                else
                {
                    hsuart_error("hsuart_tx_thread: kmalloc resultNode fail !!! \n");
                    msleep(10);
                    continue;
                }

                /*�������Ӧ����ڵ�����*/
                resultMemNode->valid_size = BAND_ADAPT_LENTH;
                memcpy((unsigned char*)resultMemNode->mem.vaddr, \
                       (unsigned char*)BAND_ADAPT_RESULT, BAND_ADAPT_LENTH);
                hsuart_error("hsuart_tx_thread: resultMemNode valid_size %d, %s \n", \
                       resultMemNode->valid_size, (unsigned char*)resultMemNode->mem.vaddr);
                resultMemNode->mem.paddr = dma_map_single(NULL, resultMemNode->mem.vaddr,\ 
                    resultMemNode->valid_size, DMA_TO_DEVICE);/*lint !e434*/
                hsuart_tx_dma(uart_dev, (u32)resultMemNode->mem.paddr, resultMemNode->valid_size);
                osl_sem_down(&uart_ctx->dma_sema);
                /* resultMemNode��Աȥ��ʼ��SKB*/
                if (uart_ctx->cbs.free_cb)
                {
                    uart_ctx->cbs.free_cb((char*)resultMemNode->pSkb);
                }
                else
                {
                    hsuart_error("hsuart_tx_thread: free_cb fail !!! \n");
                    kfree_skb(resultMemNode->pSkb);
                }
                /*�ͷŶ�̬����ռ�*/
                kfree(resultMemNode);
#if 0
                /*����OK��Ԥ��200ms��ʱ��������λ���л�֡ģʽ*/
                msleep(50);
#endif
                /*����ΪNV���õ�֡��ʽ*/
                spin_lock_irqsave(&(uart_ctx->dev.lock_irq), flags);
                hsuart_set_wlen(&uart_ctx->dev, uart_ctx->feature.wlen);
                hsuart_set_stpbit(&uart_ctx->dev, uart_ctx->feature.stp2);
                hsuart_set_eps(&uart_ctx->dev, uart_ctx->feature.eps);
                hsuart_error("hsuart_tx_thread after adapt ok -- wlen:%d, stp2:%d, eps:%d \n", \
                       uart_ctx->feature.wlen, uart_ctx->feature.stp2, uart_ctx->feature.eps);
                spin_unlock_irqrestore(&(uart_ctx->dev.lock_irq), flags);
                uart_ctx->baud_adapt = FALSE;
                allow_recv_flag = 1;
                /*�ͷ��źŵ�Ŀ���Ǳ�֤�û���������ϱ���������������ȥ*/
                osl_sem_up(&(uart_ctx->tx_sema));
            }
#else
            msleep(10);
#endif/*lint +e516 +e160 +e10 +e522 +e666*/

        }
    }
    return 0;
}
/*****************************************************************************
* �� �� ��  : hsuart_irq
* ��������  : uart �жϷ������
* �������  : irq:�жϺ�
*             dev_id:����
* �������  : NA
* �� �� ֵ  : �жϷ���ֵ
*****************************************************************************/
static irqreturn_t uart_int(int irq, void *ctx)
{
   	u32 status = 0;
	u32 remain = 0;
	u8  ch = 0;
    u32 cur_addr = 0;
	u32 cur_num = 0;
	u8 *recv_addr = 0;
	uart_port *port = NULL;
	port = (uart_port *)ctx;
	
    status = readl((const volatile void *)(port->vir_addr_base+ UART011_MIS));
	if(UART011_RTIS == (status & UART011_RTIS))
    {	
		dialup_info("irq\n");
		disable_irq_nosync(port->irq);
		cur_addr = bsp_edma_channel_stop(g_dma_ctrl.channel_id);
		cur_num = cur_addr - g_dma_ctrl.pMemNode_startaddr;
		recv_addr = (u8 *)(g_dma_ctrl.pMemNode_viraddr + cur_num);
		remain = (UART_READ_BUF_SIZE - cur_num);
		dialup_info("free room: 0x%x\n",remain);
		g_dma_ctrl.total_size = cur_num;
		do{
			ch = readl((const volatile void *)(port->vir_addr_base + UART01x_DR));
			*recv_addr = ch;
			recv_addr++;
			g_dma_ctrl.total_size++;
			status = readl((const volatile void *)(port->vir_addr_base+ UART01x_FR));	
		}while((remain--) && ((status & UART01x_FR_RXFE) != UART01x_FR_RXFE));
		osl_sem_up(&g_dma_ctrl.edma_rx_sem);
    }
	else
	{
		dialup_info("interrup id err 0x%x\n",status);
		writel(0xFF, port->vir_addr_base+ UART011_MIS);
		return (irqreturn_t)IRQ_NONE;
	}

	return (irqreturn_t)IRQ_HANDLED;
}

/*****************************************************************************
* �� �� ��  : dialup_write_async
* ��������  : UART UDI �豸�첽дʵ��
* �������  : uart_ctx: ������
*             pWRInfo: �ڴ���Ϣ
* �������  : NA
* �� �� ֵ  : ����� �ɹ�/ʧ��
*****************************************************************************/
int dialup_write_async(dialup_ctx* uart_ctx, hsuart_wr_async_info* pWRInfo)
{
    struct sk_buff* tx_skb;
    skb_mem_str* pMemNode =NULL;

    if (NULL == pWRInfo)
    {
        dialup_error("pWRInfo:0x%x\n", (unsigned int)pWRInfo);
        return ERROR;
    }
	
    tx_skb = (struct sk_buff*)pWRInfo->pBuffer;
    pMemNode = get_node_from_freelist(&(uart_ctx->write_node));
    if (pMemNode)
    {
        pMemNode->pSkb = tx_skb;
        pMemNode->valid_size = pMemNode->pSkb->len;
        pMemNode->mem.vaddr = (void*)pMemNode->pSkb->data;
        pMemNode->mem.paddr = dma_map_single(NULL, pMemNode->mem.vaddr, pMemNode->valid_size, DMA_TO_DEVICE);
        add_node_to_donelist(&(uart_ctx->write_node), pMemNode);
		uart_ctx->write_node.cur_bytes += pMemNode->valid_size;
    }
    else
    {
        dialup_info("no free node, tx_skb:0x%x\n", (unsigned int)tx_skb);
        return ERROR;
    }
	if(uart_ctx->cbs.water_ops)
	{
		if((g_hsuart_ctrl.water_low_level_flag == DIALUP_FALSE)
			&& ((uart_ctx->write_node.cur_bytes > free_node[g_hsuart_ctrl.cur_baud_index].high_level 
	   		|| uart_ctx->write_node.free_node_cnt < HSUART_WATER_NODE_HIGH)))
		{
			uart_ctx->cbs.water_ops(HIGH_LEVEL);
			g_hsuart_ctrl.water_low_level_flag = DIALUP_TRUE;
		}
	}
	else
	{
		dialup_error("water_ops null\n");
	}

#if (FEATURE_ON == MBB_MODULE_PM)
    hsuart_sleep_lock();
    hsuart_com_jiff = jiffies;
#endif

    osl_sem_up(&(uart_ctx->tx_reqsem));

    return OK;
}

/*****************************************************************************
* �� �� ��  : get_read_done_node
* ��������  : �ϲ��ȡ����buf ����Ϣ
* �������  :  uart_ctx ������, 
* �������  :  pWRInfo ���ڴ���Ϣ
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
static int get_read_done_node(dialup_ctx* uart_ctx, hsuart_wr_async_info* pWRInfo)
{
    skb_mem_str* pMemNode = NULL;
    if (NULL == pWRInfo)
    {
        dialup_error("pWRInfo:0x%x.\n", (unsigned int)pWRInfo);
        return ERROR;
    }
    pMemNode = get_node_from_donelist(&(uart_ctx->read_node));
    if (pMemNode)
    {
        pWRInfo->pBuffer = (char *)pMemNode->pSkb;
        pWRInfo->u32Size = (u32)pMemNode->pSkb->len;
        pWRInfo->pDrvPriv = (void*)pMemNode->pSkb;
        add_node_to_freelist(&(uart_ctx->read_node), pMemNode);
    }
    else
    {
        dialup_error("no done mem node in list\n");
        return ERROR;
    }
    return OK;
}

/*****************************************************************************
* �� �� ��  : release_read_done_node
* ��������  : �ϲ��ͷŽ���Buffer �Ľӿ�
* �������  : uart_ctx ������, pWRInfo �ڴ���Ϣ
* �������  :
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
static int release_read_done_node(dialup_ctx* uart_ctx, hsuart_wr_async_info* pWRInfo)
{
    if(NULL == pWRInfo)
	{
       dialup_error("pWRInfo is NULL\n");
	   return ERROR;
	}
    dev_kfree_skb((struct sk_buff *)pWRInfo->pBuffer);
	pWRInfo->pBuffer = NULL;
	uart_ctx->stat.skb_free_cnt++;
    return OK;
}
/*****************************************************************************
* �� �� ��  : dialup_set_modem_state
* ��������  :дmodem �ܽ��ź�
* �������  :uart_dev �豸�ṹ��pModemMsc �ܽ���Ϣ
* �������  :
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
s32 dialup_set_modem_state(dialup_ctx *uart_ctx, MODEM_MSC_STRU* pModemMsc)
{    
	if(NULL == uart_ctx || NULL == pModemMsc)
	{
       dialup_error("param is null\n");
	   return ERROR;
	}   
    if (uart_ctx->feature.rts)					/*����DCE rts �ܽ�*/
    {
        if (SIGNALCH == pModemMsc->OP_Cts)
        {
            if (LOWLEVEL == pModemMsc->ucCts)	/*����rts �ܽ�*/
            {
                uart_ctx->rts = DIALUP_TRUE;
                dialup_info("rts valued:%d\n", pModemMsc->ucCts);
            }
            else								/*����rts �ܽ�*/
            {               
                uart_ctx->rts = DIALUP_FALSE;
                dialup_info("rts invalued:%d\n", pModemMsc->ucCts);
            }
        }
    }

    if (SIGNALCH == pModemMsc->OP_Ri)		/*Ring SIGNAL CHANGE*/
    {
        gpio_set_value(g_dialup_ctx.gpio.ring, !pModemMsc->ucRi); 
        dialup_info("Ring change:%d\n", !pModemMsc->ucRi);//DEBUG
    }   
    if (SIGNALCH == pModemMsc->OP_Dsr)		/*DSR SIGNAL CHANGE*/
    {
        gpio_set_value(g_dialup_ctx.gpio.dsr, !pModemMsc->ucDsr); 
        dialup_info("dsr change:%d\n", !pModemMsc->ucDsr);//DEBUG
    }  
    if (SIGNALCH == pModemMsc->OP_Dcd)		/* DCD SIGNAL CHANGE*/
    {
        gpio_set_value(g_dialup_ctx.gpio.dcd, !pModemMsc->ucDcd); 
        dialup_info("dcd change:%d\n", !pModemMsc->ucDcd);
    }

    return OK;
}


int release_write_done_node(dialup_ctx* uart_ctx)
{
	skb_mem_str* pMemNode = NULL;
	
    if(NULL == uart_ctx)
    {
        dialup_error("uart_ctx:0x%x\n", (unsigned int)uart_ctx);
        return ERROR;
    }  
	
    pMemNode = get_node_from_donelist(&(uart_ctx->write_node));
    while (pMemNode)
    {
		uart_ctx->write_node.cur_bytes -= pMemNode->valid_size;
    	if (uart_ctx->cbs.free_cb)
        {
            uart_ctx->cbs.free_cb((char*)pMemNode->pSkb);
			if (pMemNode->pSkb)
			{
				pMemNode->pSkb = NULL;
			}
        }				
        else
        {
            uart_ctx->stat.write_not_free_cnt++;
            dialup_error("no write free func register\n");
            kfree_skb(pMemNode->pSkb);
			pMemNode->pSkb = NULL;
        }
        add_node_to_freelist(&(uart_ctx->write_node), pMemNode);
        pMemNode = get_node_from_donelist(&(uart_ctx->write_node));
    }
	
    return OK;
}

/*****************************************************************************
* �� �� ��  : dialup_mode_change_register
* ��������  : ģʽ�л�����
* �������  : uart_ctx ������s32Arg ���ò���
* �������  :
* �� �� ֵ  :NA
*****************************************************************************/
void dialup_mode_change_register(dialup_ctx* uart_ctx, void* s32Arg)
{
    if (NULL == s32Arg)
    {
		bsp_softtimer_delete(&g_hsuart_ctrl.switch_state3_timer);	
        uart_ctx->stage = NO_DETECT_STAGE;
		uart_ctx->cbs.switch_mode_cb= NULL;
        dialup_info("detect mode switch disable\n");
    }
    else
    {
        uart_ctx->stage = DETECT_STAGE_1;
		uart_ctx->cbs.switch_mode_cb = (hsuart_switch_mode_cb_t)s32Arg;
		dialup_info("detect mode switch enable\n");
    }
}

/*****************************************************************************
* �� �� ��  : dialup_set_flowctrl
* ��������  : uart���ؿ���
* �������  : uart_ctx ������s32Arg ���ò���
* �������  :
* �� �� ֵ  :NA
*****************************************************************************/
void dialup_set_flowctrl(dialup_ctx* uart_ctx, uart_flow_ctrl_union* pParam)
{
	u32 regval = 0;
    unsigned long flags = 0;
	
	if(NULL == pParam)
	{
       dialup_error("pParam null\n");
	   return ERROR;
	}
   	spin_lock_irqsave(&(uart_ctx->port.lock_irq), flags);
	regval = readl(uart_ctx->port.vir_addr_base + UART011_CR);
	regval &= 0x3fff;
	regval |= pParam->value;
	(void)writel(regval, (volatile void *)(uart_ctx->port.vir_addr_base + UART011_CR));

    uart_ctx->feature.rts = pParam->reg.rtsen;
    uart_ctx->feature.cts = pParam->reg.ctsen;
    spin_unlock_irqrestore(&(uart_ctx->port.lock_irq), flags);
}

/*****************************************************************************
* �� �� ��  : dialup_udi_open
* ��������  : ��uart udi�豸
* �������  : param ����, handle ���
* �������  :
* �� �� ֵ  : �򿪽��
*****************************************************************************/
static int dialup_udi_open(UDI_OPEN_PARAM_S *param, UDI_HANDLE handle)
{
    dialup_ctx *uart_ctx = &g_dialup_ctx;
    uart_ctx->open = DIALUP_TRUE;
    (void)BSP_UDI_SetPrivate(param->devid, (VOID*)uart_ctx);
    return OK;
}
/*****************************************************************************
* �� �� ��  : dialup_udi_close
* ��������  : �ر�uart udi�豸
* �������  : handle
* �������  :
* �� �� ֵ  :ʧ��/�ɹ�
*****************************************************************************/
static int dialup_udi_close(BSP_S32 s32UartDevCtx)
{
    dialup_ctx *pUartDevCtx = (dialup_ctx *)s32UartDevCtx;   
    pUartDevCtx->open = DIALUP_FALSE;
    return 0;
}

/*****************************************************************************
* �� �� ��  : uart_write_sync
* ��������  : uartУ׼ͬ�����ͽӿ�
* �������  : BSP_S32 s32UartDev, BSP_U8* pBuf, u32 u32Size
* �������  : ʵ��д�������
* �� �� ֵ  :ʧ��/ʵ�ʵ�����
*****************************************************************************/
static int dialup_write_sync(VOID* pPrivate, void* pMemObj, u32 u32Size)
{
	dialup_ctx *uart_ctx = (dialup_ctx *)pPrivate;
	unsigned int regval;
    unsigned char * pu8Buffer;

    if (NULL == uart_ctx || NULL == pMemObj || 0 == u32Size)
    {
        dialup_error("Invalid params.\n");
        return ERROR;
    }
    if (!uart_ctx->open)
    {
        dialup_error("uart is not open.\n");
        return ERROR;
    }    
    uart_ctx->stat.syn_write_cnt++;
    dialup_info("start addr:0x%x  len:%d  done list num:%d.\n", (unsigned int)pMemObj, u32Size, uart_ctx->write_node.done_cnt);
    while (uart_ctx->write_node.done_cnt)
    {
        msleep(1);
    }

    pu8Buffer = (u8 *)pMemObj;
    
    while(u32Size)
    {
        regval = readl(uart_ctx->port.vir_addr_base + UART01x_FR);	
        if(0 == (regval & UART01x_FR_TXFF))
        {
        	writel(*pu8Buffer, uart_ctx->port.vir_addr_base + UART01x_DR);
            pu8Buffer++;
			u32Size--;
		}
    }
    return OK;
}

/*****************************************************************************
* �� �� ��  : dialup_udi_ioctl
* ��������  : UART ����ҵ����� ioctl ����
* �������  : s32UartDevId: �豸����ṹָ��
*             s32Cmd: ������
*             pParam: �������
* �������  :
* �� �� ֵ  : �ɹ�/ʧ�ܴ�����
*****************************************************************************/
static int dialup_udi_ioctl(VOID* pPrivate, u32 u32Cmd, VOID* pParam)
{
    s32 ret = OK;
    unsigned long flags = 0;
    dialup_ctx* uart_ctx = (dialup_ctx*)pPrivate;
    uart_port *uart_dev = NULL;
	DRV_DIALUP_HSUART_STRU dialup_acshell_nv;
		
	if(NULL == uart_ctx)
	{
       dialup_error("pParam null\n");
	   return ERROR;
	}
    dialup_info("cmd:0x%x pParam:0x%x\n", u32Cmd, (unsigned int)pParam);
    
    if (!uart_ctx->open)
    {
        dialup_error("uart is not open\n");
        return ERROR;
    }  
    uart_dev = &uart_ctx->port;
    switch(u32Cmd)
    {      
        case UART_IOCTL_WRITE_ASYNC:		/* �����첽д�ӿ�*/
        	ret = dialup_write_async(uart_ctx, (hsuart_wr_async_info *)pParam);
        	break;
        case UART_IOCTL_SET_FREE_CB:		/* NASע����������buf �ͷź���*/
        	uart_ctx->cbs.free_cb = (hsuart_free_cb_t)pParam;
        	break;
        case UART_IOCTL_SET_READ_CB:		/* �յ����ݺ���ô�ע��ӿ�֪ͨ�ϲ����*/
        	uart_ctx->cbs.read_cb = (hsuart_read_cb_t)pParam;
        	break;        
        case UART_IOCTL_GET_RD_BUFF:		/* �ϲ���ô˽ӿڻ������buf ��ַ���ϲ������ǵ�read�ص���������ʵ�ֵ�*/
        	ret = get_read_done_node(uart_ctx, (hsuart_wr_async_info*)pParam);
        	break;
        case UART_IOCTL_RETURN_BUFF:		/* �ϲ���ô˽ӿ��ͷ�����buf */
           	ret = release_read_done_node(uart_ctx, (hsuart_wr_async_info*)pParam);
        	break;       
		case UART_IOCTL_RELEASE_BUFF:
			ret = release_write_done_node(uart_ctx);
			break;
        case UART_IOCTL_SET_MSC_READ_CB:	/*ע��DTR�ܽű仯֪ͨ�ص�����ΪpMODEM_MSC_STRU ָ�򱾵�ȫ��*/
        	uart_ctx->cbs.msc_read_cb = (hsuart_msc_read_cb_t)pParam;
	        break;       
        case UART_IOCTL_MSC_WRITE_CMD:		 /*дmodem�ܽ��źţ�����ΪpMODEM_MSC_STRU*/
        	ret = dialup_set_modem_state(uart_ctx, (MODEM_MSC_STRU*)(pParam));
        	break;        
        case UART_IOCTL_SWITCH_MODE_CB:		/* +++�л�����ģʽ�ص�������NULL�رոù��� */
        	dialup_mode_change_register(uart_ctx, pParam);
        	break;            
        case UART_IOCTL_SET_BAUD:			/* ���ò����� */
            spin_lock_irqsave(&(uart_ctx->port.lock_irq), flags);
			uart_ctx->cbs.set_baud_cb = uart_set_baud;
            uart_ctx->feature.baud = *(int*)pParam;
            spin_unlock_irqrestore(&(uart_ctx->port.lock_irq), flags);
            dialup_info("set baud cb: %d\n", *(int*)pParam);
			if(g_hsuart_ctrl.set_baud_flag == 0)
			{
				uart_reset_termios(uart_ctx);
				g_hsuart_ctrl.set_baud_flag = 1;
			}
			break;
        case UART_IOCTL_SET_WLEN:			/* �������ݳ��� */
            spin_lock_irqsave(&(uart_ctx->port.lock_irq), flags);
            uart_ctx->cbs.set_wlen_cb = uart_set_wlen;
            uart_ctx->feature.wlen = *(int*)pParam;
            spin_unlock_irqrestore(&(uart_ctx->port.lock_irq), flags);
            dialup_info("set wlen cb: %d\n", *(int*)pParam);
        	break;
        case UART_IOCTL_SET_STP2:			/* ����ֹͣλ */
            spin_lock_irqsave(&(uart_ctx->port.lock_irq), flags);           
            uart_ctx->cbs.set_stp2_cb = uart_set_stpbit;
            uart_ctx->feature.stp2 = *(int*)pParam;            
            spin_unlock_irqrestore(&(uart_ctx->port.lock_irq), flags);
            dialup_info("set stp2 cb: %d\n", *(int*)pParam);
        	break;
        case UART_IOCTL_SET_EPS:		 	/* ����У������ */
            spin_lock_irqsave(&(uart_ctx->port.lock_irq), flags);
            uart_ctx->cbs.set_eps_cb = uart_set_eps;
           	uart_ctx->feature.eps = *(int*)pParam;
            spin_unlock_irqrestore(&(uart_ctx->port.lock_irq), flags);
            dialup_info("set eps cb: %d\n", *(int*)pParam);
        	break;
        case UART_IOCTL_SET_FLOW_CONTROL:	/* �������� */
            dialup_set_flowctrl(uart_ctx, (uart_flow_ctrl_union*)pParam);
        	break;
		case UART_IOCTL_SET_WATER_CB:
			uart_ctx->cbs.water_ops = (hsuart_water_cb)pParam;
			break;
        case UART_IOCTL_SET_AC_SHELL:		 /*set A/C shell �ն����� debug*/
			dialup_acshell_nv.DialupACShellCFG = *(unsigned int*)pParam;
			dialup_acshell_nv.DialupEnableCFG  = DIALUP_ENABLE;
			ret = bsp_nvm_write(NV_ID_DRV_DIALUP_ACSHELL, (u8 *)&dialup_acshell_nv, sizeof(DRV_DIALUP_HSUART_STRU));
			if (ret != OK)
   			{
        		dialup_error("write %d NV ERROR\n",NV_ID_DRV_DIALUP_ACSHELL);
        		return ERROR;
    		}
            break;
#if ( FEATURE_ON == MBB_HSUART )
/*lint -e10 -e160 -e522 -e666*/
        case UART_IOCTL_INIT_BAUD:
            {
                spin_lock_irqsave(&(uart_ctx->dev.lock_irq), flags);
                uart_ctx->feature.baud = *(int*)pParam;
                hsuart_fifo_reset(&uart_ctx->dev);
                hsuart_set_baud(&uart_ctx->dev,uart_ctx->feature.baud);
                hsuart_error("init baud : %d\n", *(int*)pParam);
                spin_unlock_irqrestore(&(uart_ctx->dev.lock_irq), flags);
            }
            break;
        case UART_IOCTL_INIT_WLEN:
            {
                spin_lock_irqsave(&(uart_ctx->dev.lock_irq), flags);
                uart_ctx->feature.wlen = *(int*)pParam;
                /*������Ӧģʽ����������֡��ʽ*/
                if(FALSE == uart_ctx->baud_adapt)
                {
                    hsuart_fifo_reset(&uart_ctx->dev);
                    hsuart_set_wlen(&uart_ctx->dev,uart_ctx->feature.wlen);
                }
                spin_unlock_irqrestore(&(uart_ctx->dev.lock_irq), flags);
                hsuart_error("hsuart_udi_ioctl init wlen : %d\n", *(int*)pParam);
            }
            break;
        case UART_IOCTL_INIT_STP2:
            {
                spin_lock_irqsave(&(uart_ctx->dev.lock_irq), flags);
                uart_ctx->feature.stp2 = *(int*)pParam;
                /*������Ӧģʽ����������֡��ʽ*/
                if(FALSE == uart_ctx->baud_adapt)
                {
                    hsuart_fifo_reset(&uart_ctx->dev);
                    hsuart_set_stpbit(&uart_ctx->dev,uart_ctx->feature.stp2);
                }
                hsuart_error("hsuart_udi_ioctl init stp2 : %d\n", *(int*)pParam);
                spin_unlock_irqrestore(&(uart_ctx->dev.lock_irq), flags);
            }
            break;
        case UART_IOCTL_INIT_EPS:
            {
                spin_lock_irqsave(&(uart_ctx->dev.lock_irq), flags);
                uart_ctx->feature.eps = *(int*)pParam;
                /*������Ӧģʽ����������֡��ʽ*/
                if(FALSE == uart_ctx->baud_adapt)
                {
                    hsuart_fifo_reset(&uart_ctx->dev);
                    hsuart_set_eps(&uart_ctx->dev,uart_ctx->feature.eps);
                }
                hsuart_error("hsuart_udi_ioctl init eps : %d\n", *(int*)pParam);
                spin_unlock_irqrestore(&(uart_ctx->dev.lock_irq), flags);
            }
            break;
/*lint +e10 +e160 +e522 +e666*/
#endif/*MBB_HSUART*/
        default:
            dialup_error("unknow Cmd: 0x%x\n", u32Cmd);
            ret = ERROR;
            break;
    }
    return ret;
}
/*****************************************************************************
* �� �� ��  : dialup_timer_init
* ��������  : 
* �������  : uart_ctx ������
* �������  : NA
* �� �� ֵ  : �ɹ�/ʧ��bcmsdh_register_oob_intr
*****************************************************************************/
static int uart_set_termios(dialup_ctx *uart_ctx)
{
	uart_port *port = NULL;
	
    port = &uart_ctx->port;
    /* disable uart */
    writel(0, (volatile void *)(port->vir_addr_base) + UART011_CR);
    /* Clear all interrupts */
    writel(0, (volatile void *)(port->vir_addr_base) + UART011_IMSC);
	writel(0xFF, (volatile void *)(port->vir_addr_base+ UART011_ICR));
    writel(UART011_RTSIFLS_FULL_2 | UART011_IFLS_RX2_8, (volatile void *)(port->vir_addr_base) + UART011_IFLS);

	uart_set_baud(port, uart_ctx->feature.baud);
	
	while(UART01x_FR_BUSY & readl((volatile void *)(port->vir_addr_base) + UART01x_FR))
		barrier();
	writel(uart_ctx->feature.wlen | uart_ctx->feature.stp2 | uart_ctx->feature.eps | UART01x_LCRH_FEN, 
		(volatile void *)(port->vir_addr_base) + UART011_LCRH);
	writel(UART011_TXDMAE | UART011_RXDMAE, (volatile void *)(port->vir_addr_base) + UART011_DMACR);
	writel(UART01x_CR_UARTEN | UART011_CR_RXE | UART011_CR_TXE | UART011_CR_RTS | uart_ctx->feature.rts | uart_ctx->feature.cts,
		(volatile void *)(port->vir_addr_base) + UART011_CR);

	return OK;
}

/*****************************************************************************
* �� �� ��  : dialup_timer_init
* ��������  : 
* �������  : uart_ctx ������
* �������  : NA
* �� �� ֵ  : �ɹ�/ʧ��bcmsdh_register_oob_intr
*****************************************************************************/
int dialup_timer_init(dialup_ctx *uart_ctx)
{
	struct softtimer_list *statethree_timer = &g_hsuart_ctrl.switch_state3_timer;
	statethree_timer->func = switch_stage_three;
	statethree_timer->para = (u32)&g_dialup_ctx;
	statethree_timer->timeout = SWITCH_TIMER_LENGTH;
	statethree_timer->wake_type = SOFTTIMER_NOWAKE;

	if(OK != bsp_softtimer_create(statethree_timer))
	{
		dialup_error("statethree_timer create fail\n");
		return ERROR;	
	}

	return 0;
}
#if ( FEATURE_ON == MBB_HSUART )
/*****************************************************************************
* �� �� ��  : hsuart_selfadapt_timer_cb
* ��������  : hsuart����������Ӧ�ɹ���timer�ĳ�ʱ���������˳�����Ӧģʽ
* �������  : NA
* �������  : NA
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
void hsuart_selfadapt_timer_cb(u32 arg)
{
    hsuart_error("hsuart_selfadapt_timer_cb entry \n");
    hsuart_ctx *uart_ctx = (hsuart_ctx *)arg;
    hsuart_dev *uart_dev = &uart_ctx->dev;

    /*����Ϊ����Ӧ�Ĳ�����*/
    hsuart_set_baud(uart_dev,uart_ctx->feature.baud);
    adapt_success_flag = 1;
    osl_sem_up(&(uart_ctx->tx_sema));
}
/*****************************************************************************
* �� �� ��  : hsuart_adapt_timer_init
* ��������  : 
* �������  : uart_ctx ������
* �������  : NA
* �� �� ֵ  : �ɹ�/ʧ��bcmsdh_register_oob_intr
*****************************************************************************/
int hsuart_adapt_timer_init(hsuart_ctx *uart_ctx)
{
    struct softtimer_list* adapt_timer = &g_hsuart_ctrl.adapt_timer;

    adapt_timer->func = hsuart_selfadapt_timer_cb;
    adapt_timer->para = (u32)&m2m_hsuart_ctx;
    adapt_timer->timeout = HSUART_SELFADAPT_TIMEOUT;
    adapt_timer->wake_type = SOFTTIMER_NOWAKE;

    if (OK != bsp_softtimer_create(adapt_timer))
    {
        hsuart_error("adapt_timer create fail\n");
        return ERROR;
    }

    return OK;
}
#endif/*MBB_HSUART*/
#ifdef CONFIG_PM
static int dialup_suspend(struct device *dev)
{
	dialup_ctx *uart_ctx = &g_dialup_ctx;
	uart_port *uart_dev = &uart_ctx->port;
	
	if(g_hsuart_ctrl.ACSHELL_FLAG == DIALUP_FALSE)
	{	
		if(!bsp_edma_channel_is_idle(EDMA_CH_HSUART_TX))
		{
			dialup_error("hsuart tx edma busy\n");
			return ERROR;
		}
		g_dma_ctrl.resume_flag = DIALUP_FALSE;
		bsp_edma_channel_stop(g_dma_ctrl.channel_id);
		bsp_edma_channel_stop(EDMA_CH_HSUART_TX);
		disable_irq(gpio_to_irq(g_dialup_ctx.gpio.dtr));
		disable_irq((unsigned int)uart_dev->irq);
	}
	return OK;
}
extern s32 hsuart_acshell_baud(uart_port *port);

static s32 dialup_hsuart_resume(struct device *dev)
{
	struct clk *uartclk = NULL;
	dialup_ctx *uart_ctx = &g_dialup_ctx;
	uart_port *port = &uart_ctx->port;

	uartclk = clk_get(NULL, "uart1_clk");
	if(uartclk == NULL)
	{
		dialup_error("get clk fail\n");
		return ERROR;
	}
	/* coverity[check_return] */
	clk_enable(uartclk);
	/* coverity[unchecked_value] */
	if(g_hsuart_ctrl.ACSHELL_FLAG == DIALUP_FALSE)
	{
		uart_set_termios(uart_ctx);
		enable_irq(gpio_to_irq(g_dialup_ctx.gpio.dtr));
		enable_irq((unsigned int)port->irq);
		g_dma_ctrl.resume_flag = DIALUP_TRUE;

		if(g_dma_ctrl.dma_run_flag == DIALUP_TRUE)
		{
			if (bsp_edma_channel_async_start(g_dma_ctrl.channel_id, (u32)(port->phy_addr_base + UART01x_DR), g_dma_ctrl.pMemNode_startaddr, uart_ctx->read_node.node_size))
	    	{
	        	dialup_error("dma_channel failed! ret_id = %d\n", g_dma_ctrl.channel_id);
	        	return ERROR;
	    	}
			(void)writel(UART011_RTIM,  (volatile void *)(port->vir_addr_base+ UART011_IMSC));
		}
	}
	return OK;
}
static const struct dev_pm_ops dialup_pm_ops ={
	.suspend = dialup_suspend,
	.resume = dialup_hsuart_resume,
};
#define BALONG_DEV_PM_OPS (&dialup_pm_ops)
#else
#define BALONG_DEV_PM_OPS NULL
#endif

#define DRIVER_NAME "dialup_device"
static struct platform_driver dialup_driver = {
	.probe = NULL,
	.remove = NULL,
	.driver = {
		.name = DRIVER_NAME,
		.owner  = THIS_MODULE,
		.pm     = BALONG_DEV_PM_OPS,
	},
};

static struct platform_device dialup_device =
{
    .name = DRIVER_NAME,
    .id       = -1,
    .num_resources = 0,
};
/*****************************************************************************
* �� �� ��  : hsuart_drv_init
* ��������  : uart �ײ��ʼ������
* �������  : uart_ctx:uart������
* �������  : NA
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
s32 dialup_drv_init(dialup_ctx *uart_ctx)
{
	int ret = 0;
    uart_port *uap = NULL;

    uap = &uart_ctx->port;
    spin_lock_init(&uap->lock_irq);
	
	memset(&uart_ctx->cbs, 0, sizeof(hsuart_udi_cbs));
    memset(&uart_ctx->stat, 0, sizeof(hsuart_stat));
	memset(&uart_ctx->feature, 0, sizeof(hsuart_feature));

	//uart_ctx->open = DIALUP_TRUE;
#if ( FEATURE_ON == MBB_HSUART )
#if ( FEATURE_ON == MBB_FACTORY )
    uart_ctx->baud_adapt = DIALUP_FALSE;
#else
    uart_ctx->baud_adapt = DIALUP_TRUE;
#endif/*MBB_FACTORY*/
#else
    uart_ctx->baud_adapt = DIALUP_FALSE;
#endif/*MBB_HSUART*/
    uart_ctx->stage = DETECT_STAGE_1;
    uart_ctx->rts = DIALUP_FALSE;
#if ( FEATURE_ON == MBB_HSUART )
#if ( FEATURE_ON == MBB_FACTORY )
    uart_ctx->feature.baud = DEFAULT_BAUD;
#else
    uart_ctx->feature.baud = 0;
#endif/*MBB_FACTORY*/
#else
    uart_ctx->feature.baud = DEFAULT_BAUD;
#endif/*MBB_HSUART*/
    uart_ctx->feature.wlen = UART01x_LCRH_WLEN_8;

    osl_sem_init(SEM_EMPTY ,&(uart_ctx->tx_reqsem));
    osl_sem_init(SEM_EMPTY ,&(uart_ctx->tx_endsem));
    osl_sem_init(SEM_EMPTY ,&g_dma_ctrl.edma_rx_sem);

	ret = dialup_buf_init(uart_ctx);
	ret |= uart_set_termios(uart_ctx);
	ret |= dialup_gpio_init(uart_ctx);
	ret |= dialup_timer_init(uart_ctx);
	if(ret != OK)
	{
		 return ERROR;
	}
	
	if(OK != request_irq((unsigned int)uap->irq, (irq_handler_t)uart_int,0,"dialup", uap))
	{
	   	dialup_error("request irq failed\n");
	    return ERROR;
	}
	disable_irq((unsigned int)uap->irq);
	writel(UART011_RTIM, (volatile void *)(uap->vir_addr_base+ UART011_IMSC));

#if ( FEATURE_ON ==MBB_HSUART )
    if (OK != hsuart_adapt_timer_init(uart_ctx))
    {
        hsuart_error("hsuart adapt timer init err\n");
    }
#endif/*MBB_HSUART*/
    return OK;
}

static UDI_DRV_INTEFACE_TABLE dialup_udi = {
    .udi_open_cb = (UDI_OPEN_CB_T)dialup_udi_open,
    .udi_close_cb = (UDI_CLOSE_CB_T)dialup_udi_close,
    .udi_write_cb = (UDI_WRITE_CB_T)dialup_write_sync,
    .udi_read_cb = (UDI_READ_CB_T)NULL,
    .udi_ioctl_cb = (UDI_IOCTL_CB_T)dialup_udi_ioctl,
};
/*****************************************************************************
* �� �� ��  : get_info_from_dts
* ��������  : UART UDI ��ʼ���ӿ�
* �������  : NA
* �������  : NA
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
int get_info_from_dts(void)
{
	struct device_node *node = NULL;
	unsigned int reg_data[2]={0,0};
	
	node = of_find_compatible_node(NULL, NULL, "hisilicon,dialup_uart");
	if (!node)
	{
		dialup_error("dts node hsuart app not found!\n");
		return ERROR;
	}
	
	if(of_property_read_u32(node, "clock", &g_dialup_ctx.port.clock))
	{
		dialup_error("read reg from dts is failed!\n");
		return ERROR;
	}
	if(of_property_read_u32_array(node, "reg", (u32 *)&reg_data,2))
	{
		dialup_error("read reg from dts is failed!\n");
		return ERROR;
	}

	if(of_property_read_u32_array(node, "gpio", (u32 *)&g_dialup_ctx.gpio, 4))
	{
		dialup_error("read gpio dts is failed!\n");
		return ERROR;
	}

	g_dialup_ctx.port.phy_addr_base = reg_data[0];
	g_dialup_ctx.port.reg_size = reg_data[1];
	g_dialup_ctx.port.irq = (u32)irq_of_parse_and_map(node, 0);
	if(g_dialup_ctx.port.irq == 0)
	{
		dialup_error("read hsuart irq fail\n");
		return ERROR;
	}
	g_dialup_ctx.port.vir_addr_base = of_iomap(node, 0);
	if(NULL == g_dialup_ctx.port.vir_addr_base)
	{
		dialup_error("vir addr get fail\n");
		return ERROR;
	}
	
	return OK;
}

int dialup_init(void)
{
	s32 ret = ERROR;
	struct clk *uartclk = NULL;
    DRV_DIALUP_HSUART_STRU 	dialup_nv_str;
		
	memset(&dialup_nv_str, 0, sizeof(DRV_DIALUP_HSUART_STRU));

	ret = bsp_nvm_read(NV_ID_DRV_DIALUP_ACSHELL, (u8 *)&dialup_nv_str, sizeof(DRV_DIALUP_HSUART_STRU));
	if (ret != OK)
	{
	   	dialup_error("read %d nv err\n",NV_ID_DRV_DIALUP_ACSHELL);
	   	dialup_nv_str.DialupEnableCFG = 0;
	} 

	if(1 == dialup_nv_str.DialupEnableCFG)
	{	
		if(dialup_nv_str.DialupACShellCFG == UART_A_SHELL || dialup_nv_str.DialupACShellCFG == UART_C_SHELL)
		{
			dialup_nv_str.DialupACShellCFG = 0;
			ret = bsp_nvm_write(NV_ID_DRV_DIALUP_ACSHELL, (u8 *)&dialup_nv_str, sizeof(DRV_DIALUP_HSUART_STRU));
			if (ret != OK)
   			{
        		dialup_error("dialup acshell init fail\n");
        		return ERROR;
    		}
		}
		else
		{
			if(OK != get_info_from_dts())
			{
				dialup_error("hsuart get dts err\n");
				return ERROR;
			}
				
			uartclk = clk_get(NULL, "uart1_clk");
			if(IS_ERR(uartclk))
			{
				dialup_error("get clk fail\n");
				return ERROR;
			}
			clk_prepare(uartclk);
			clk_enable(uartclk);
			
			ret = platform_device_register(&dialup_device);
			if(ret)
			{
				dialup_error("Platform device register failed\n");
				clk_disable(uartclk);
		        return ret;
			}
			ret = platform_driver_register(&dialup_driver);
			if (ret)
			{
				dialup_error("Platform driver register failed\n");
				platform_device_unregister(&dialup_device);
				clk_disable(uartclk);
				return ret;
			}
			
		    if(OK != dialup_drv_init(&g_dialup_ctx))
		    {
				goto init_fail;
		    }
			if(ERROR == osl_task_init("dialup recv",HSUART_RX_TASK_PRI,HSUART_RX_TASK_SIZE,(void *)dialup_rx_thread,
				(void *)&g_dialup_ctx, &g_hsuart_ctrl.rx_task_id))
			{
			  	dialup_error("uart rx thread create failed!\n");
				goto init_fail;
			}
			if(ERROR == osl_task_init("dialup send",HSUART_TX_TASK_PRI,HSUART_TX_TASK_SIZE,(void *)dialup_tx_thread,
				(void *)&g_dialup_ctx, &g_hsuart_ctrl.tx_task_id))
			{
			   	dialup_error("tx thread create failed\n");
				kthread_stop((struct task_struct *)g_hsuart_ctrl.rx_task_id);
				goto init_fail;
			}

			/* UDI��ĳ�ʼ�� */
	    	if(ERROR == BSP_UDI_SetCapability(g_dialup_ctx.udi_device_id, 0))
			{
			   	dialup_error("dialup setCapability err\n");
				goto udi_fail;
			}
	    	if(ERROR == BSP_UDI_SetInterfaceTable(g_dialup_ctx.udi_device_id, &dialup_udi))
			{
			   	dialup_error("dialup setInterfaceTable err\n");
				goto udi_fail;
			}
			g_dma_ctrl.resume_flag = DIALUP_TRUE;
			g_dialup_ctx.init = DIALUP_TRUE;
#if (FEATURE_ON == MBB_MODULE_PM)
            hsuart_pm_init();
#endif
		}
		dialup_error("init end\n");
	}
	return OK;
	
udi_fail:
	kthread_stop((struct task_struct *)g_hsuart_ctrl.rx_task_id);
	kthread_stop((struct task_struct *)g_hsuart_ctrl.tx_task_id);

init_fail:
	platform_device_unregister(&dialup_device);
	platform_driver_unregister(&dialup_driver);
	clk_disable(uartclk);
	return ERROR;
}

module_init(dialup_init);

/*****************************************************************************
 		HSUART ��ά�ɲ�ӿ�
*****************************************************************************/
void hsuart_set_loglevel(u32 temp)
{
	bsp_mod_level_set(BSP_MODU_HSUART , temp);
}

void hsuart_set_gpio(u32 gpio,u32 value)
{
	gpio_set_value(gpio,value);
}
#if ( FEATURE_ON == MBB_HSUART )
#ifndef BAUD_BUF_LEN
#define BAUD_BUF_LEN 10
#endif
unsigned int get_hsuart_baud(unsigned char* pbuf)
{
    unsigned int hsuartbaud = 0;
    char sztemp[BAUD_BUF_LEN];

    if(NULL == pbuf)
    {
        return 0;
    }
    
    hsuart_ctx *uart_ctx = &m2m_hsuart_ctx;/*lint !e578*/

    hsuartbaud = (unsigned int)uart_ctx->feature.baud;

    hsuart_error("get_hsuart_baud  hsuartbaud = <%d> \r\n", hsuartbaud);

    memset(sztemp, 0, BAUD_BUF_LEN);

    snprintf(sztemp, BAUD_BUF_LEN, " %d", hsuartbaud);

    memcpy(pbuf, sztemp, BAUD_BUF_LEN);

    return (unsigned int)strlen(pbuf);
}
#endif/*MBB_HSUART*/

void hsuart_read_reg(u32 offset)
{
    u32 status;	
    status = readl(g_dialup_ctx.port.vir_addr_base + offset);
    dialup_error("offset:0x%x   value:0x%x !\n", offset, status);
}

void hsuart_write_reg(u32 offset, u32 value)
{
    (void)writel(value, g_dialup_ctx.port.vir_addr_base + offset);   
    dialup_error("write offset:0x%x   value:0x%x !\n", offset, value);
}

void show_dialup_info(u32 sel)
{
    dialup_ctx *uart_ctx = &g_dialup_ctx;
    u32 idx = 0;
    unsigned long flags = 0;
    
    if (uart_ctx->read_node.pcur_pos)
    {
    printk("current memnode valid size: %d \n", uart_ctx->read_node.pcur_pos->valid_size);
    }
	printk("pcur_pos:                   %x \n", (u32)uart_ctx->read_node.pcur_pos);
    printk("baud adapt status:          %d \n", uart_ctx->baud_adapt);
    printk("total_bytes:                %d \r\n", uart_ctx->read_node.total_bytes);   
    printk("current detect stage:       %d \n", uart_ctx->stage);
	printk("============hsuart reg status=========\r\n");
    printk("current baud:               %d \n", uart_ctx->stat.cur_baud);
    printk("current wlen:               %d \n", uart_ctx->feature.wlen);
    printk("current stp2:               %d \n", uart_ctx->feature.stp2);
    printk("current eps:                %d \n", uart_ctx->feature.eps);
    
    printk("not free write data cnt:    %d \n", uart_ctx->stat.write_not_free_cnt);
    printk("lost data cnt:              %d \n", uart_ctx->stat.lost_data_cnt);
    printk("synchronization write cnt: 	%d \n", uart_ctx->stat.syn_write_cnt);
    printk("alloc skb fail:             %d \n", uart_ctx->stat.alloc_skb_fail);
    printk("alloc skb cnt:              %d \n", uart_ctx->stat.alloc_skb_cnt);
	printk("free  skb cnt:              %d \n",uart_ctx->stat.skb_free_cnt);

    if (uart_ctx->init)
    {
        spin_lock_irqsave(&uart_ctx->read_node.list_lock, flags);
		printk("=============read node info=============\r\n");
   		printk("read free node num:         %d \n", uart_ctx->read_node.free_node_cnt);
        printk("read done node num:         %d \n", uart_ctx->read_node.done_cnt);
        printk("read node num:              %d \n", uart_ctx->read_node.node_num);
        printk("read node size:             %d \n", uart_ctx->read_node.node_size);
        printk("read done cnt:              %d \n", uart_ctx->read_node.done_cnt);
		if(sel == 1)
		{
	        for (idx = 0; idx < uart_ctx->read_node.node_num; idx++)
	        {
	        	printk("node state:                 %d \n", uart_ctx->read_node.pmem_start[idx].state);
	            printk("node size:                  %d \n", uart_ctx->read_node.pmem_start[idx].valid_size);
	        }
		}
		spin_unlock_irqrestore(&uart_ctx->read_node.list_lock, flags); 
        
        spin_lock_irqsave(&uart_ctx->write_node.list_lock, flags);
		printk("=============write node info============\r\n");
        printk("write free node num:        %d \n", uart_ctx->write_node.free_node_cnt);
        printk("write done node num:        %d \n", uart_ctx->write_node.done_cnt);
        printk("write node num:             %d \n", uart_ctx->write_node.node_num);
        printk("write node size:            %d \n", uart_ctx->write_node.node_size);
        printk("write done cnt:             %d \n", uart_ctx->write_node.done_cnt);
		printk("water cur bytes:            %d \n", uart_ctx->write_node.cur_bytes);
		printk("water free node cnt:        %d \n", uart_ctx->write_node.free_node_cnt);
		
		if(sel == 1)
		{
	        for (idx = 0; idx < uart_ctx->write_node.node_num; idx++)
	        {
	        	printk("node state:                 %d \n", uart_ctx->write_node.pmem_start[idx].state);
	            printk("node size:                  %d \r\n", uart_ctx->write_node.pmem_start[idx].valid_size);
	        }
		}
        spin_unlock_irqrestore(&uart_ctx->write_node.list_lock, flags);
    }
}

