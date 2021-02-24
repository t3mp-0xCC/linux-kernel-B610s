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

#include "spe_sim_intr.c"

#define SPE_INTR_NUM 0

irqreturn_t spe_sim_ut_irq_handler(int spe_irq, void * spe_dev)
{
	unsigned int *event_buffer=NULL;
	unsigned int wptr=spe_intr.event_buffer_wprt;
	unsigned int rptr=spe_intr.event_buffer_rprt;
	unsigned int i = 0;
	printk("Spe_sim_ut_irq_handler called, event: \n");
	printk("Spe_timer suppress num %u \n", spe_intr_debug.intr_suppress);
	printk("Spe_intr wprt before %u \n", spe_intr.event_buffer_wprt);
	printk("Spe_intr rptr before %u \n", spe_intr.event_buffer_rprt);
	event_buffer = spe_intr.event_buffer;
	while(wptr != rptr)
	{
		i++;
		printk("current event is 0x%x \n", event_buffer[rptr]);

		rptr = (rptr++)%512;//CURRENT_EVENT_BUFFER_SIZE
	}
	
	printk("Spe_sim even num %u \n",i);
	spe_dec_current_event_num(i);
	printk("Spe_intr wprt after %u \n", spe_intr.event_buffer_wprt);
	printk("Spe_intr rptr after %u \n", spe_intr.event_buffer_rprt);
	
}

/*init ut entry test, init mac_fw_ctx and pkt stub*/
void spe_sim_intr_ut_init(void)
{
	unsigned long flags=0;
	spe_sim_intr_init();
	(void)spe_request_irq(SPE_INTR_NUM, spe_sim_ut_irq_handler, flags, "spe_sim", NULL);

}

/*set one event, call irq call back without intr_suppress incurse*/
void spe_sim_intr_ut_001(void)
{
	
	spe_sim_port_t port;
	port.port_num =11;
	spe_sim_start_intr(&port, SPE_TD_COMPLETE);
}


/*set ten event, call irq call back with intr_suppress incurse by 5*/
void spe_sim_intr_ut_002(void)
{
	spe_sim_port_t port[5];
	int i=0;
	for(i=0;i<5;i++){
		port[i].port_num =i;
		spe_sim_start_intr(&port[i], SPE_TD_COMPLETE);
	}
	for(i=0;i<5;i++){
		port[i].port_num =i;
		spe_sim_start_intr(&port[i], SPE_RD_COMPLETE);
	}

}

/*set 200 event, call irq call back with intr_suppress incurse by unknow*/
void spe_sim_intr_ut_003(void)
{
	

//	spe_sim_start_intr(spe_sim_port_t * port, unsigned int event);
}

