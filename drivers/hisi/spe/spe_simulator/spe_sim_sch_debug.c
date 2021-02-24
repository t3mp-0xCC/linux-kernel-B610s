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

#include "spe_sim_sch.c"

/*port 0:cpu port*/

/*port 1:(like gmac or usb connect pc address are for pc)
	under bridge 10 
	ip:192.168.1.10
	port:65533
	mac:0x00,0x1B,0x78,0x40,0x08,0x1F
	dst of dl
*/
/*port 2:(WAN&rnic, it has two port :2 and 9)
	under bridge 9 
	ip:202.118.80.200
	port:
	mac:0xAE,0xAE,0xAE,0xAE,0xAE,0xAE
*/
/*port 9:(WAN, the same thing to port 2, add this so spe can see rnic as a bridge)
	bridge
	ip:202.118.80.200
	port:65000
*/

/*port 10:(LAN, has no mac)
	bridge
	ip:192.168.1.1
	port:NO
	mac:0x00,0x1B,0x78,0x40,0x08,0x1E
*/
/*remote server
	ip:202.118.80.1
	port:60000
	mac:0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
*/

#define TD_RD_SIZE 64

extern void spe_set_td(spe_sim_ctx_t *ctx);
extern void spe_set_rd(spe_sim_ctx_t *ctx);

static unsigned char g_local_dev_mac[MAC_ADDR_LEN]={0x00,0x1B,0x78,0x40,0x08,0x1F};
static unsigned char g_local_lan_mac[MAC_ADDR_LEN]={0x00,0x1B,0x78,0x40,0x08,0x1E};
static unsigned char g_local_wan_mac[MAC_ADDR_LEN]={0xAE,0xAE,0xAE,0xAE,0xAE,0xAE};
static unsigned char g_remote_sever_mac[MAC_ADDR_LEN]={0xAF,0xAF,0xAF,0xAF,0xAF,0xAF};

void spe_sim_sch_test_init(void)
{
    char* temp=NULL;
    spe_sim_ctx.ports[0].is_enable = 1;
    spe_sim_ctx.ports[0].tocken_set = 1;
    spe_sim_ctx.ports[0].port_num = 0;
    spe_sim_ctx.ports[0].enc_type = spe_sim_enc_cpu;
    spe_sim_ctx.ports[0].td_ctx.r_pos=0;
    spe_sim_ctx.ports[0].td_ctx.w_pos=0;
    spe_sim_ctx.ports[0].td_ctx.td_num=TD_RD_SIZE;
    spe_sim_ctx.ports[0].rd_ctx.r_pos=0;
    spe_sim_ctx.ports[0].rd_ctx.w_pos=0;
    spe_sim_ctx.ports[0].rd_ctx.rd_num=TD_RD_SIZE;
    spe_sim_ctx.ports[0].ctx=(void*)(&spe_sim_ctx);
    spe_sim_ctx.ports_idx[0] = &spe_sim_ctx.ports[0];



    spe_sim_ctx.ports[1].is_enable = 1;
    spe_sim_ctx.ports[1].tocken_set = 2;
    spe_sim_ctx.ports[1].port_num = 1;
    memcpy(spe_sim_ctx.ports[1].port_mac, g_local_dev_mac, 6);
    spe_sim_ctx.ports[1].enc_type = spe_sim_enc_none;

    temp=kzalloc(TD_RD_SIZE*sizeof(struct transaction_descriptor), GFP_KERNEL);
    spe_sim_ctx.ports[1].td_ctx.base=temp;
    spe_sim_ctx.ports[1].td_ctx.r_pos=0;
    spe_sim_ctx.ports[1].td_ctx.w_pos=0;
    spe_sim_ctx.ports[1].td_ctx.td_num=TD_RD_SIZE;

    temp=kzalloc(TD_RD_SIZE*sizeof(struct received_descriptor), GFP_KERNEL);
    spe_sim_ctx.ports[1].rd_ctx.base=temp;
    spe_sim_ctx.ports[1].rd_ctx.r_pos=0;
    spe_sim_ctx.ports[1].rd_ctx.w_pos=0;
    spe_sim_ctx.ports[1].rd_ctx.rd_num=TD_RD_SIZE;
    spe_sim_ctx.ports[1].ctx=(void*)(&spe_sim_ctx);
    spe_sim_ctx.ports_idx[1] = &spe_sim_ctx.ports[1];



    spe_sim_ctx.ports[2].is_enable = 1;
    spe_sim_ctx.ports[2].tocken_set = 3;
    spe_sim_ctx.ports[2].port_num = 2;
    memcpy(spe_sim_ctx.ports[2].port_mac, g_local_wan_mac, 6);
    spe_sim_ctx.ports[2].enc_type = spe_sim_enc_none;

    temp=kzalloc(TD_RD_SIZE*sizeof(struct transaction_descriptor), GFP_KERNEL);
    spe_sim_ctx.ports[2].td_ctx.base=temp;
    spe_sim_ctx.ports[2].td_ctx.r_pos=0;
    spe_sim_ctx.ports[2].td_ctx.w_pos=0;
    spe_sim_ctx.ports[2].td_ctx.td_num=TD_RD_SIZE;

    temp=kzalloc(TD_RD_SIZE*sizeof(struct received_descriptor), GFP_KERNEL);	
    spe_sim_ctx.ports[2].rd_ctx.base=temp;
    spe_sim_ctx.ports[2].rd_ctx.r_pos=0;
    spe_sim_ctx.ports[2].rd_ctx.w_pos=0;
    spe_sim_ctx.ports[2].rd_ctx.rd_num=TD_RD_SIZE;
    spe_sim_ctx.ports[2].ctx=(void*)(&spe_sim_ctx);
    spe_sim_ctx.ports_idx[2] = &spe_sim_ctx.ports[2];



    spe_sim_ctx.cpu_port = &spe_sim_ctx.ports[0];
    spe_sim_ctx.usr_field_num=0;
    spe_sim_rescan_enable_port();
    spe_set_rd(&spe_sim_ctx);

}

void spe_sim_sch_start(void)
{
    spe_set_td(&spe_sim_ctx);

    spe_sim_schedule_port(NULL);
}

void check_td_ut_result(void)
{
	printk("td wptr: %d, td rptr: %d \n", spe_sim_ctx.ports[2].td_ctx.w_pos, spe_sim_ctx.ports[2].td_ctx.r_pos);
	printk("rd wptr: %d, rd rptr: %d \n", spe_sim_ctx.ports[1].rd_ctx.w_pos, spe_sim_ctx.ports[1].rd_ctx.r_pos);
}
void spe_sim_print_port(unsigned int i)
{
    printk("is_enable 0x%x \n",spe_sim_ctx.ports[i].is_enable);
    printk("tocken_set 0x%x \n",    spe_sim_ctx.ports[i].tocken_set);
    printk("port_num 0x%x \n",    spe_sim_ctx.ports[i].port_num);
    printk("port_type 0x%x \n",    spe_sim_ctx.ports[i].port_type);
	
    printk("enc_type 0x%x \n",    spe_sim_ctx.ports[i].enc_type);

    printk("td_ctx.base 0x%x \n",    spe_sim_ctx.ports[i].td_ctx.base);
    printk("td_ctx.r_pos 0x%x \n",    spe_sim_ctx.ports[i].td_ctx.r_pos);
    printk("td_ctx.w_pos 0x%x \n",    spe_sim_ctx.ports[i].td_ctx.w_pos);
    printk("td_ctx.td_num 0x%x \n",    spe_sim_ctx.ports[i].td_ctx.td_num);

    printk("rd_ctx.base 0x%x \n",    spe_sim_ctx.ports[i].rd_ctx.base);
    printk("rd_ctx.r_pos 0x%x \n",    spe_sim_ctx.ports[i].rd_ctx.r_pos);
    printk("rd_ctx.w_pos 0x%x \n",    spe_sim_ctx.ports[i].rd_ctx.w_pos);
    printk("rd_ctx.rd_num 0x%x \n",    spe_sim_ctx.ports[i].rd_ctx.rd_num);


    printk("ctx 0x%x \n",    spe_sim_ctx.ports[i].ctx);
    printk("ports_idx 0x%x \n",    spe_sim_ctx.ports_idx[i]);

}
