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

/*
 * spe_sim_priv.h - private header for spe simulator
 *
 */

#include "spe_sim_priv.h"
#include <linux/jiffies.h>
#include <linux/netfilter/nf_conntrack_tuple_common.h>


void spe_sim_reg_set_td_rptr(spe_sim_port_t* port)
{
	printk("port %u td_rptr set to %d", port->port_num, port->td_ctx.r_pos);

}

void spe_sim_reg_set_rd_rptr(spe_sim_port_t* port)
{
	printk("port %u rd_rptr set to %d", port->port_num, port->rd_ctx.r_pos);
}
/*ÉèÖÃÈ«¾Ö×´Ì¬¼Ä´æÆ÷*/
void spe_sim_reg_set_status(spe_sim_ctx_t* ctx, int status)
{

}
void spe_sim_reg_init(spe_sim_ctx_t* ctx)
{
}

void spe_sim_reg_set_entry_result(int result){}
#define CURRENT_EVENT_BUFFER_SIZE 512
/* get event buffer info from register */
unsigned int spe_sim_reg_get_event_addr(spe_sim_ctx_t* ctx)
{
	void * event_buffer = NULL;
	/*max event buffer size is u32*1024,use half*/
	event_buffer = kmalloc(4*CURRENT_EVENT_BUFFER_SIZE, GFP_ATOMIC);
	if(NULL == event_buffer){
		return 0;
	}
	memset(event_buffer, 0, 4*CURRENT_EVENT_BUFFER_SIZE);
	return (unsigned int)event_buffer;
}

unsigned int spe_sim_reg_get_event_size(spe_sim_ctx_t* ctx)
{
	return CURRENT_EVENT_BUFFER_SIZE;
}
unsigned int spe_sim_reg_set_event_depth(unsigned int value)
{
	printk("event_buffer_depth is %u \n", value);
	return 0;
}

unsigned int spe_sim_reg_get_event_timeout(spe_sim_ctx_t* ctx)
{
	return 1000;
}

unsigned int spe_sim_reg_get_event_en(char evnet){return 0;}

unsigned int spe_sim_reg_get_mac_age_time(spe_sim_ctx_t* ctx)
{
	/*return aging timer reg value*/
	return 10*HZ;
}

unsigned int spe_sim_reg_get_ip_age_time(spe_sim_ctx_t* ctx){return 0;}/*return aging timer reg value*/

struct spe_sim_mac_fw_entry {
	char mac[MAC_ADDR_LEN];         /* mac address */
	u16 br_portno:4;                /* bridge number this entry belongs*/
	u16 reserve:12;
	u32 port_index;                 /* port index */
};

struct spe_sim_mac_fw_entry mac_dump_buffer[256];/*no mac entry overflow but it's not possible for that  many mac entry*/

unsigned int spe_sim_reg_get_entry_dump(spe_sim_ctx_t* ctx)
{
	return (unsigned int)mac_dump_buffer;
}/*return dump reg value(32bit)*/

int spe_sim_reg_if_port_under_bridge(spe_sim_port_t *port)/*return 1=not under bridge,0=under bridge*/
{
	/*port 0 is cpu, port 1&2 under bridge, port 3,4 not under bridge
	there is no possible for port 0 use as a src or dst port
	port 1,2 return 0; port 3,4 return 1*/
	if((port->port_num == 1)||(port->port_num == 2)){
		return 0;
	}
	else{
		return 1;
	}
}
struct tag_spe_sim_port * dst_port_stub[16]={NULL};
#if 0
/*this func needs to rebuild*/
int spe_sim_get_port_info(unsigned int port_num, spe_sim_port_t** port)
{
	struct tag_spe_sim_port *temp_cpu_port = dst_port_stub[port_num];
	/*Force 0 to be cpu port*/
	if(NULL != temp_cpu_port){
		printk("no need to malloc cpu port temp_cpu_port  is 0x%x \n",(unsigned int)temp_cpu_port);
		*port = temp_cpu_port;
		return 0;
	}
	
	temp_cpu_port = kmalloc(sizeof(struct tag_spe_sim_port), GFP_ATOMIC);
	if(NULL == temp_cpu_port){
		printk("fail to malloc cpu port temp_cpu_port  is 0x%x \n",(unsigned int)temp_cpu_port);
		return -ENOMEM;
	}
	temp_cpu_port->port_num = port_num;
	*port = temp_cpu_port;
	printk("malloc cpu port success cpu_port  is 0x%x \n",(unsigned int)port);

	dst_port_stub[port_num] = temp_cpu_port;

	return 0;
}
#endif
