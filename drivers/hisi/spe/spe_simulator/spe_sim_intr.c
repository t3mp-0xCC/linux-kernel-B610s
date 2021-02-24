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

#include "spe_sim.h"
#include "spe_sim_priv.h"
#include "bsp_hardtimer.h"
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/module.h>

/*timer time out time, by us*/
#define SPE_TIME_OUT_INIT 1000000
#define SPE_TIME_OUT_FIRST_EVENT 1000
#define SPE_HDT_100US_VALUE 1920
#define SPE_HDT_10US_VALUE 192
#define SPE_TIMER_ID USB_TIMER_ID
#define SPE_GET_BUFFER_DEPTH(wptr, rptr, size) ((wptr>=rptr)?(wptr-rptr):(size -rptr+wptr))
#define SPE_HDR_US_TO_SLICE(time) (time*SPE_HDT_10US_VALUE/10)
#define SPE_HDR_SLICE_TO_US(slice) (slice*100/SPE_HDT_100US_VALUE)

typedef enum spe_event_buffer_status
{
	SPE_EVENT_BUFFER_EMPTY,
	SPE_EVENT_BUFFER_USED,
}spe_event_buffer_status_t;

typedef enum spe_timer_status
{
	TIMER_ON,
	TIMER_OFF,
}spe_timer_status_t;

struct spe_sim_intr
{
	unsigned int *event_buffer;
	unsigned int event_buffer_size;
	unsigned int event_buffer_wprt;
	unsigned int event_buffer_rprt;
	unsigned int timer_status;
	unsigned int time_out;/*by us*/
	unsigned int last_time;
	spinlock_t spe_event_buffer_lock;
	void *idev;
	irqreturn_t (*spe_intr_cb_func)(int irq, void *dev);
	struct bsp_hardtimer_control spe_timer_ctrl;
};

struct spe_sim_intr_debug
{
	unsigned int event_buffer_full;
	unsigned int intr_suppress;
	unsigned int intr_cnt;
};

static struct spe_sim_intr spe_intr; 
static struct spe_sim_intr_debug spe_intr_debug; 
struct platform_device spe_sim_device = {
	.name = "spe_sim_dev",
};


/* write event into event buffer,maybe event buffer don't need this close to the real thing?*/
static void write_event(enum spe_event_type event_type, unsigned int port,
								unsigned int buffer) 
{
	spe_intr.event_buffer[buffer] = (event_type<<4)|port; 
}

/*this func suppose to be call by reg after driver set the event number it processed*/
void spe_dec_current_event_num(unsigned int event_num)
{
	unsigned long spe_intr_spinlock_flag = 0;
	unsigned int spe_event_buffer_depth;
	unsigned int spe_event_buffer_wptr;
	unsigned int spe_event_buffer_rptr;
	unsigned int spe_event_buffer_size;

	spin_lock_irqsave(&spe_intr.spe_event_buffer_lock, spe_intr_spinlock_flag);
	spe_event_buffer_wptr = spe_intr.event_buffer_wprt;
	spe_event_buffer_rptr = spe_intr.event_buffer_rprt;
	spe_event_buffer_size = spe_intr.event_buffer_size;

	/*updata buffer depth and rptr, do I need to add protecting for it?*/
	spe_event_buffer_rptr = (spe_event_buffer_rptr+event_num)%spe_event_buffer_size;
	
	spe_event_buffer_depth = SPE_GET_BUFFER_DEPTH(spe_event_buffer_wptr,
		spe_event_buffer_rptr, spe_event_buffer_size);
	
	spe_sim_reg_set_event_depth(spe_event_buffer_depth);
	spe_intr.event_buffer_wprt = spe_event_buffer_wptr;
	spe_intr.event_buffer_rprt = spe_event_buffer_rptr;
	
	spin_unlock_irqrestore(&spe_intr.spe_event_buffer_lock,
		spe_intr_spinlock_flag);

	
}

/*this func went thought each event buffer and run current event beening recorded*/
static int spe_start_event(void)
{
	void *device = spe_intr.idev;
    irqreturn_t ret;

    spe_intr_debug.intr_cnt++;

	if(NULL == spe_intr.spe_intr_cb_func){
		SPE_SIM_ERR("intr:spe_td_complete NULL! \n");
		return -ENXIO;
	}
	
	bsp_hardtimer_int_clear(SPE_TIMER_ID);/*Clean intr*/
	bsp_hardtimer_disable(SPE_TIMER_ID);
	spe_intr.last_time = bsp_get_slice_value_hrt();
	
	SPE_SIM_TRACE("intr:spe_start_event, call intr cb \n");
	ret=spe_intr.spe_intr_cb_func(SPE_IRQ_NUM, device); 
	spe_intr.timer_status = TIMER_OFF;
	return ret;
	
}

/*registe irq func for spe*/
int spe_request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
	    const char *name, void *dev)
{	
	int result = 0;
#ifndef CONFIG_BALONG_SPE_SIMULATOR
	result = request_irq(irq, handler, 0, "spe_balong", NULL);
	if(0 != result){
		SPE_SIM_ERR("intr:spe fail request irq fail! \n");
		return result;
	}
#else
	spe_intr.spe_intr_cb_func = handler;
	spe_intr.idev = dev;
#endif
	return result;
}
EXPORT_SYMBOL_GPL(spe_request_irq);

void spe_free_irq(unsigned int irq, void* dev_id)
{
#ifndef CONFIG_BALONG_SPE_SIMULATOR
	free_irq(irq, dev_id);
#endif
	return;
}
EXPORT_SYMBOL_GPL(spe_free_irq);


void spe_sim_intr_set_event_timeout(unsigned int timeout)
{
    spe_intr.time_out = timeout;
}

void spe_sim_intr_re_init(void)
{
    unsigned int i;
    struct spe_sim_intr *spe_intr_addr = &spe_intr;

    spe_intr_addr->event_buffer_rprt = 0;
	spe_intr_addr->event_buffer_wprt = 0;

	for(i=0; i< SPE_EVENT_BUFFER_MAX_SIZE;i++){
		spe_intr_addr->event_buffer[i] = 0;
	}
	return;
}

void spe_sim_intr_get_event_ptr(unsigned int* rptr, unsigned int* wptr)
{
    struct spe_sim_intr *spe_intr_addr = &spe_intr;

    *rptr = spe_intr_addr->event_buffer_rprt;
    *wptr = spe_intr_addr->event_buffer_wprt;
    return;
}

/* init event buffer and start a hardtime for intr */
void spe_sim_intr_init(void)
{
	unsigned int i;
	int result;
	dma_addr_t dma_addr;
	struct spe_sim_intr *spe_intr_addr = &spe_intr; 

	int ret;
	ret = platform_device_register(&spe_sim_device);
	if(ret){
		SPE_SIM_ERR("intr:spe_sim_start_intr, input pointer NULL!\n");
		return;
	}

	spe_intr_debug.event_buffer_full = 0;
	spe_intr_debug.intr_suppress = 0;

	spe_intr_addr->event_buffer_size = SPE_EVENT_BUFFER_MAX_SIZE;
	SPE_SIM_INFO("intr:event buffer size 0x%x. \n", spe_intr_addr->event_buffer_size);

	spe_intr_addr->event_buffer = dma_alloc_coherent(
		NULL, spe_intr_addr->event_buffer_size*4, &dma_addr, GFP_KERNEL);
	printk("new_reg: event buffer addr 0x%x",spe_intr_addr->event_buffer);
	if (IS_ERR(spe_intr_addr->event_buffer)) {
		SPE_SIM_ERR("can't allocate event buffer\n");
		return;
	}

	spe_sim_reg_set_event_addr(spe_intr_addr->event_buffer);
	SPE_SIM_INFO("intr:event buffer head addr 0x%x. \n", spe_intr_addr->event_buffer);

	spe_intr_addr->event_buffer_rprt = 0;
	spe_intr_addr->event_buffer_wprt = 0;
	
	for(i=0; i< SPE_EVENT_BUFFER_MAX_SIZE;i++){
		spe_intr_addr->event_buffer[i] = 0;
	}

	spin_lock_init(&spe_intr_addr->spe_event_buffer_lock);

	spe_intr_addr->timer_status = TIMER_OFF;
	spe_intr_addr->last_time =  bsp_get_slice_value_hrt(); // 确认下可以调用的时刻 65225
	//spe_intr.time_out= spe_sim_reg_get_event_timeout(NULL);
	
	spe_intr_addr->spe_timer_ctrl.timerId = SPE_TIMER_ID;
	spe_intr_addr->spe_timer_ctrl.mode = TIMER_ONCE_COUNT;
	spe_intr_addr->spe_timer_ctrl.timeout = SPE_TIME_OUT_INIT;
	spe_intr_addr->spe_timer_ctrl.func = spe_start_event;
	spe_intr_addr->spe_timer_ctrl.para = NULL;
	spe_intr_addr->spe_timer_ctrl.unit = TIMER_UNIT_US;/*Use micro second as unit*/
	result = bsp_hardtimer_start(&spe_intr_addr->spe_timer_ctrl); 
	if(OK != result){
		SPE_SIM_ERR("SPE init hardtimer fail \n");
	}
	bsp_hardtimer_disable(SPE_TIMER_ID);
	bsp_hardtimer_int_unmask(SPE_TIMER_ID);
}

// 65225
// 锁的使用只在保护全局变量 wptr 和 rptr 就行了, 合理的处理方法如下:
// 进入函数时加锁, 将 wptr 和 rptr 读取到局部变量里, 开锁
// 后面读取和 ++ wptr 和 rptr 只针对 局部变量
// 处理完后加锁, 将 wptr 写入全局变量

/* write event buffer and generate an interrupt */
void spe_sim_start_intr(spe_sim_port_t * port, char event)
{
	unsigned long spe_intr_spinlock_flag = 0; // 不用初始化 65225
	unsigned int spe_event_buffer_wptr;
	unsigned int spe_event_buffer_rptr;
	unsigned int spe_event_buffer_size;
	unsigned int spe_event_buffer_depth;
	unsigned int spe_current_event_buffer;
	unsigned int spe_current_time;
	unsigned int spe_time_out;/*by us*/
	unsigned int spe_timer_status;
	unsigned int spe_timer_slice_delta;
	enum spe_event_buffer_status event_buffer_status;

	SPE_SIM_TRACE("intr:spe_sim_start_intr,entry!\n");
	if(NULL == port)
	{
		SPE_SIM_ERR("intr:spe_sim_start_intr, input pointer NULL!\n");
		return;
	}
	/*check intr enable*/

	if(!spe_sim_reg_get_event_en(event)){// 用接口 后面相同 w65225 
		SPE_SIM_ERR("intr:spe_sim_start_intr, intr masked!\n");
		return;
	}

	spin_lock_irqsave(&spe_intr.spe_event_buffer_lock, spe_intr_spinlock_flag);
	spe_event_buffer_wptr = spe_intr.event_buffer_wprt;
	spe_event_buffer_rptr = spe_intr.event_buffer_rprt;
		
	spe_event_buffer_size = spe_intr.event_buffer_size;
	spe_current_event_buffer = spe_event_buffer_wptr;
	spe_timer_status = spe_intr.timer_status; // 这个后面忘更新了 w65225
	spe_time_out = spe_intr.time_out;
		
	/*Normally move wprt, but if event buffer already full, discard event and return*/
	if(spe_event_buffer_rptr != (spe_event_buffer_wptr+1)%spe_event_buffer_size){
		spe_event_buffer_wptr = (spe_event_buffer_wptr+1)%spe_event_buffer_size;
	}
	else
	{
        // 加可谓可测计数 65225
		spe_intr_debug.event_buffer_full++;
		return;
	}

	write_event(event, port->port_num, spe_current_event_buffer);

	/*Updata event buffer size*/
	if(spe_event_buffer_rptr == spe_event_buffer_wptr){
		event_buffer_status = SPE_EVENT_BUFFER_EMPTY;
	}
	else{
		event_buffer_status = SPE_EVENT_BUFFER_USED;
	}

	if(TIMER_ON == spe_timer_status){
		/*timer on, do nothing*/
		spe_intr_debug.intr_suppress++;
        
	}
	else{
		spe_current_time = bsp_get_slice_value_hrt();
		spe_timer_slice_delta = get_timer_slice_delta(spe_intr.last_time, 
				spe_current_time);
		spe_timer_status = TIMER_ON;
		spe_intr.timer_status = spe_timer_status;
		SPE_SIM_INFO("intr:timer slice delta = %u long/short thr = %u \n" ,spe_timer_slice_delta, SPE_HDR_US_TO_SLICE(spe_time_out));

		if(spe_timer_slice_delta > SPE_HDR_US_TO_SLICE(spe_time_out)){// 寄存器中断压制时间怎么没用 65225
			
			/*set a short hard timer*/
			bsp_hardtimer_disable(SPE_TIMER_ID);
			bsp_hardtimer_load_value(SPE_TIMER_ID, SPE_TIME_OUT_FIRST_EVENT);
			bsp_hardtimer_enable(SPE_TIMER_ID);
			SPE_SIM_TRACE("intr:short timer start running!\n");
		}
		else{
			/*set a long hard timer*/
			//spe_timer_slice_delta = SPE_HDR_SLICE_TO_US(spe_time_out)
			//	- SPE_HDR_SLICE_TO_US(spe_timer_slice_delta);/*switch from slice to us*/
			spe_timer_slice_delta = SPE_HDR_US_TO_SLICE(spe_time_out)
			    - spe_timer_slice_delta;
			bsp_hardtimer_disable(SPE_TIMER_ID);
			bsp_hardtimer_load_value(SPE_TIMER_ID, spe_timer_slice_delta);
			bsp_hardtimer_enable(SPE_TIMER_ID);
			SPE_SIM_TRACE("ong timer start running!timer:%u\n", spe_timer_slice_delta);

		}
	}
	
	spe_intr.event_buffer_wprt = spe_event_buffer_wptr;
	spe_intr.event_buffer_rprt = spe_event_buffer_rptr;
	spe_event_buffer_depth = SPE_GET_BUFFER_DEPTH(spe_event_buffer_wptr,
	spe_event_buffer_rptr, spe_event_buffer_size);
	
	spe_sim_reg_set_event_depth(spe_event_buffer_depth);

	spin_unlock_irqrestore(&spe_intr.spe_event_buffer_lock, spe_intr_spinlock_flag);
}

void spe_sim_intr_debug(void)
{
	printk("spe_intr.event_buffer addr 		0x%x\n",(unsigned int)spe_intr.event_buffer);
	printk("spe_intr.event_buffer_size 		%u\n",spe_intr.event_buffer_size);
	printk("spe_intr.event_buffer_wprt 		%u\n",spe_intr.event_buffer_wprt);
	printk("spe_intr.event_buffer_rprt 		%u\n",spe_intr.event_buffer_rprt);
	printk("spe_intr.timer_status 		%u\n",spe_intr.timer_status);
	printk("spe_intr.time_out 		%u\n",spe_intr.time_out);
	printk("spe_intr.last_time 		%u\n",spe_intr.last_time);

	printk("spe_intr.spe_timer_ctrl.timerId 		%u\n",spe_intr.spe_timer_ctrl.timerId);
	printk("spe_intr.spe_timer_ctrl.mode 		%u\n",spe_intr.spe_timer_ctrl.mode);
	printk("spe_intr.spe_timer_ctrl.timeout 		%u\n",spe_intr.spe_timer_ctrl.timeout);
	printk("spe_intr.spe_timer_ctrl.para 		%u\n",spe_intr.spe_timer_ctrl.para);
	printk("spe_intr.spe_timer_ctrl.unit 		%u\n",spe_intr.spe_timer_ctrl.unit);
	printk("spe_intr.spe_timer_ctrl.func 		0x%x\n",(unsigned int)spe_intr.spe_timer_ctrl.func);

	printk("spe_intr_debug.event_buffer_full 		%u\n",spe_intr_debug.event_buffer_full);
	printk("spe_intr_debug.intr_suppress 		%u\n",spe_intr_debug.intr_suppress);
	printk("spe_intr_debug.intr_cnt 		    %u\n",spe_intr_debug.intr_cnt);

}

void spe_sim_intr_set_timeout(unsigned int timer_len)
{
	spe_intr.time_out = timer_len;

}

//late_initcall(spe_sim_intr_init);

MODULE_LICENSE("GPL");
