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

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include "spe_sysfs.h"
#include "spe.h"
#include "spe_dbg.h"
#include "spe_entry.h"
#include "spe_port.h"


struct spe_cmd_handler_s spe_cmd_handler;
static unsigned int spe_blackwhite_mode = 0;
static unsigned int spe_blackwhite = 0;
static u32 spe_eth_set_min_len = 0;
static unsigned int spe_param_action = 0;

static u32 spe_udp_lim_portno;
static u32 spe_udp_rate;
static u32 spe_port_lim_portno;
static u32 spe_port_rate;
static u32 spe_port_lim_time;
static u32 spe_l4_protocol;
static u16 spe_l4_port_num;

unsigned int onoff = 0;

extern struct spe spe_balong;

struct spe_filter_param mac_filter_array[] = 
{
	{"mac", filter_mac_addr},
	{"action", filter_action},
	{"reaction", filter_mac_reaction},
	{"redirect", filter_mac_redirect},
};

struct spe_filter_param mac_forward_array[] = 
{
	{"action", forward_action},
};

struct spe_filter_param ip_filter_array[] = 
{
	{"action", filter_action},
	{"filter_mask", filter_ip_mask},	
	{"src_port_low", filter_src_port_low},	
	{"src_port_high", filter_src_port_high},
	{"dst_port_low", filter_dest_port_low},	
	{"dst_port_high", filter_dest_port_high},

	{"src_addr", filter_ip_src_addr},
	{"src_addr_mask", filter_ip_src_addr_mask},	
	{"dst_addr", filter_ip_dest_addr},
	{"dst_addr_mask", filter_ip_dest_addr_mask},	
	{"protocol", filter_ipv4_protocol},
	{"tos", filter_tos},	
};

struct spe_filter_param ipv6_filter_array[] = 
{
	{"action", filter_action},
	{"filter_mask", filter_ip_mask},
	{"src_port_low", filter_src_port_low},	
	{"src_port_high", filter_src_port_high},
	{"dst_port_low", filter_dest_port_low},	
	{"dst_port_high", filter_dest_port_high},

	{"src_addr", filter_ipv6_src_addr},
	{"src_addr_mask", filter_ipv6_src_addr_mask},	
	{"dst_addr", filter_ipv6_dest_addr},
	{"dst_addr_mask", filter_ipv6_dest_addr_mask},	
	{"protocol", filter_ipv6_protocol},
	{"tc", filter_ipv6_tc},
	{"next_hdr", filter_ipv6_next_hdr},
	{"flow_label", filter_ipv6_flow_label},
};

struct spe_filter_param spe_black_white[] = 
{
	{"mode", spe_black_white_mode_set},
    {"black_white", spe_black_white_set},
    {"action", set_action},
};

struct spe_filter_param spe_set_eth_minlen_array[] = 
{
	{"value", spe_eth_min_len_value_set},
    {"action", set_action},
};

struct spe_filter_param udp_lmtnum_array[]=
{
    {"portno", spe_udp_lim_portno_set},
    {"udp_rate", udp_rate_set},
    {"action", set_action},
};

struct spe_filter_param lmt_byte_array[]=
{
    {"portno", spe_port_lim_portno_set},
    {"byte_rate", port_rate_set},
    {"action", set_action},
};

struct spe_filter_param lmt_time_array[]=
{
    {"time", spe_port_lim_time_set},
    {"action", set_action},
};

struct spe_filter_param spe_dynamic_switch[] = {
	{"switch", spe_switch},
    {"action", set_action},
};

struct spe_filter_param spe_l4_portnum_array[] = {
	{"protocol", spe_l4_protocol_get},
	{"portnum", spe_l4_portnum_get},
    {"action", filter_action},
};

//variables define

int last_filter_type = 0;

//function declaration



/**********************************************************
function name:  spe_mac_validate

description: do some basic mac check
mac format  01:02:03:04:05:06

input:
	
output:
	
**********************************************************/
int mac_entry_set(struct mac_forward_entry *entry, int action)
{
	struct mac_forward_handler_s mac_forward_handler;

	memset(&mac_forward_handler, 0, sizeof(mac_forward_handler));


	memcpy(&mac_forward_handler.mac_forward, entry, sizeof(struct mac_forward_entry));
	
	mac_forward_handler.action = action;
	mac_forward_handler.type = SPE_MAC_FORWARD_TABLE;
	
	return exe_spe_cmd();

}



/**********************************************************
function name:  spe_mac_validate

description: do some basic mac check
mac format  01:02:03:04:05:06

input:
	
output:
	
**********************************************************/
int spe_mac_validate(void)
{
	return SPE_OK;
}





/**************************************************
 	MAC Filter1
[0:31] MAC地址寄存器，MAC地址前4个字节

 	MAC Filter2
[16:31]MAC 地址后2个字节
[4:7]：重定向端口号。
[0:3]action：1、添加 2、删除 3、查询



**************************************************/
int spe_sysfs_cmd_type(char *buf)
{
	memset(&spe_cmd_handler, 0, sizeof(spe_cmd_handler));
	
	if(0 == strcmp(buf, "ipv4"))
	{
		spe_cmd_handler.type = SPE_IP_FILTER_TABLE;
		spe_cmd_handler.filter_parser_ops = ip_filter_array;		
		spe_cmd_handler.num_funcs = ARRAY_SIZE(ip_filter_array);
		spe_cmd_handler.u.ip_content.iptype = 0;
        printk("ipv4 filter:\n");
	}
	else if(0 == strcmp(buf, "ipv6"))
	{
		spe_cmd_handler.type = SPE_IPV6_FILTER_TABLE;				
		spe_cmd_handler.filter_parser_ops = ipv6_filter_array;		
		spe_cmd_handler.num_funcs = ARRAY_SIZE(ipv6_filter_array);
		spe_cmd_handler.u.ip_content.iptype = 1;
        printk("ipv6 filter:\n");
	}
	else if(0 == strcmp(buf, "mac_filter"))
	{
		spe_cmd_handler.type = SPE_MAC_FILTER_TABLE;
		spe_cmd_handler.filter_parser_ops = mac_filter_array;
		spe_cmd_handler.num_funcs = ARRAY_SIZE(mac_filter_array);
        printk("mac filter:\n");
	}
    else if(0 == strcmp(buf, "mac_forward"))
	{
		spe_cmd_handler.type = SPE_MAC_FORWARD_TABLE;
		spe_cmd_handler.filter_parser_ops = mac_filter_array;
		spe_cmd_handler.num_funcs = ARRAY_SIZE(mac_filter_array);
        printk("mac forward:\n");
	}
    else if(0 == strcmp(buf, "filter_type"))
	{
		spe_cmd_handler.type = SPE_BLACK_WHITE;
		spe_cmd_handler.filter_parser_ops = spe_black_white;
		spe_cmd_handler.num_funcs = ARRAY_SIZE(spe_black_white);
        printk("filter type:\n");
	}
    else if(0 == strcmp(buf, "udp_limit"))
    {
        spe_cmd_handler.type = SPE_UDP_LMTNUM_TABLE;
        spe_cmd_handler.filter_parser_ops = udp_lmtnum_array;
		spe_cmd_handler.num_funcs = ARRAY_SIZE(udp_lmtnum_array);
        printk("udp limit:\n");
    }   
    else if(0 == strcmp(buf, "byte_limit"))
    {
        spe_cmd_handler.type = SPE_LMTBYTE_TABLE;
        spe_cmd_handler.filter_parser_ops = lmt_byte_array;
		spe_cmd_handler.num_funcs = ARRAY_SIZE(lmt_byte_array);
        printk("byte limit:\n");
    } 
    else if(0 == strcmp(buf, "limit_time"))
    {
        spe_cmd_handler.type = SPE_LMT_TIME_TABLE;
        spe_cmd_handler.filter_parser_ops = lmt_time_array;
		spe_cmd_handler.num_funcs = ARRAY_SIZE(lmt_time_array);
        printk("limit time:\n");
    }  
	else if(0 == strcmp(buf, "min_len"))
	{
		spe_cmd_handler.type = SPE_ETH_MIN_LEN;
		spe_cmd_handler.filter_parser_ops = spe_set_eth_minlen_array;
		spe_cmd_handler.num_funcs = ARRAY_SIZE(spe_set_eth_minlen_array);
        printk("min len:\n");
	}
	else if(0 == strcmp(buf, "switch"))
	{
		spe_cmd_handler.type = SPE_DYNAMIC_SWITCH;
		spe_cmd_handler.filter_parser_ops = spe_dynamic_switch;
		spe_cmd_handler.num_funcs = ARRAY_SIZE(spe_dynamic_switch);
        printk("dynamic switch mode:\n");
	}
	else if(0 == strcmp(buf, "l4_portnum"))
	{
		spe_cmd_handler.type = SPE_L4_PORTNUM;
		spe_cmd_handler.filter_parser_ops = spe_l4_portnum_array;
		spe_cmd_handler.num_funcs = ARRAY_SIZE(spe_l4_portnum_array);
        printk("l4 port number:\n");
	}
	else
	{
	    SPE_ERR("error parameter : %s\n", buf);
		return SPE_FAIL;
	}

	return SPE_OK;	
}

int filter_mac_addr(char *buf)
{
	int mac1[6];
	char mac[6];

	(void)sscanf(buf, "%2x:%2x:%2x:%2x:%2x:%2x", &mac1[0], &mac1[1],&mac1[2],&mac1[3],&mac1[4],&mac1[5]);

    mac[0] = mac1[0];
    mac[1] = mac1[1];
    mac[2] = mac1[2];
    mac[3] = mac1[3];
    mac[4] = mac1[4];
    mac[5] = mac1[5];

    printk("mac: %pM\n", mac);

	if(spe_mac_validate() != SPE_OK)
	{
		
		SPE_ERR("%s: 100: mac validate fail\n", __FUNCTION__);
		return SPE_FAIL;
	}

	memcpy(spe_cmd_handler.u.mac_content.mac, mac, sizeof(mac));

	return SPE_OK;	
}

int filter_action(char *buf)
{
	if(0 == strcmp(buf, "add"))
	{
		spe_cmd_handler.action = SPE_ENTRY_ADD;
	}
	else if(0 == strcmp(buf, "delete"))
	{
		spe_cmd_handler.action = SPE_ENTRY_DELETE;		
	}
	else if(0 == strcmp(buf, "dump"))
	{
		spe_cmd_handler.action = SPE_ENTRY_DUMP;		
	}
	else
	{
		SPE_ERR("action %s is not supported !\n", buf);
		return SPE_FAIL;
	}
    
	printk("action = %s\n", buf);
	return SPE_OK;
}

int forward_action(char *buf)
{
    if(0 == strcmp(buf, "dump"))
	{
		spe_cmd_handler.action = SPE_ENTRY_DUMP;		
	}
	else
	{
		SPE_ERR("action %s is not supported !\n", buf);
		return SPE_FAIL;
	}
    
	printk("action = %s\n", buf);
	return SPE_OK;
}

int filter_mac_reaction(char *buf)
{
	unsigned  redir_action;
	(void)sscanf(buf, "%4u", &redir_action);	
	spe_cmd_handler.u.mac_content.action = redir_action;
    printk("redir acriont = %d\n", redir_action);
	return SPE_OK;    
}

int filter_mac_redirect(char *buf)
{
	unsigned port_index = 255;
	
	(void)sscanf(buf, "%3d", &port_index);	
	if(port_index > SPE_PORT_NUM)
	{
	    SPE_ERR("redir port can't be %d\n", port_index);
		return SPE_FAIL;		
	}
	//should check whether the port is registered later!
	
	spe_cmd_handler.u.mac_content.redirect = port_index;
    printk("redir port = %d\n", port_index);
	return SPE_OK;
}

int filter_ip_mask(char *buf) 
{
	int mask;
	
	(void)sscanf(buf, "%4x", &mask);
	spe_cmd_handler.u.ip_content.filter_mask = (unsigned int)mask;
    printk("mask = 0x%x\n", mask);

	return SPE_OK;
}

int filter_ip_addr(char *buf, int type)
{
    unsigned int ip[4]={0};
	
	(void)sscanf(buf, "%4d.%4d.%4d.%4d", &ip[0], &ip[1],&ip[2],&ip[3]);
        
	if(type == SPE_DEST_IP)	{
		//dst_addr
		printk("dip:%d,%d,%d,%d\n",ip[0], ip[1],ip[2],ip[3]);
		spe_cmd_handler.u.ip_content.u.spe_ipv4.dst_addr = (ip[0]<<24)|(ip[1]<<16)|(ip[2]<<8)|ip[3];
	}
	else if(type  == SPE_SRC_IP){
        //src_addr
		spe_cmd_handler.u.ip_content.u.spe_ipv4.src_addr = (ip[0]<<24)|(ip[1]<<16)|(ip[2]<<8)|ip[3];        
		printk("sip:%d,%d,%d,%d\n",ip[0], ip[1],ip[2],ip[3]);
	}
	else{
        SPE_ERR("%s: type = %d", __FUNCTION__, type);
		return SPE_FAIL;
	}

	return SPE_OK;
}

int filter_ip_src_addr(char *buf)
{
	return filter_ip_addr(buf, SPE_SRC_IP);
}

int filter_ip_dest_addr(char * buf)
{
	return filter_ip_addr(buf, SPE_DEST_IP);
}

/**********************************************************
function name: 

description: 
		mask = 0-32,  don't use 255.255.255.xxx
input:
	
output:
	
**********************************************************/
int filter_ip_addr_mask(char *buf, int type)
{
	unsigned int mask;
	
	(void)sscanf(buf, "%4d", &mask);
	if(mask > 32){
		SPE_ERR("%s: mask = %d", __FUNCTION__, mask);
		return SPE_FAIL;
	}
	
	if(type == SPE_DEST_IP){
		//dst_mask
		printk("dmask:%d \n", mask);
		spe_cmd_handler.u.ip_content.u.spe_ipv4.dst_addr_mask = mask;
	}
	else if(type == SPE_SRC_IP){
        //src_mask
        printk("smask:%d \n", mask);
		spe_cmd_handler.u.ip_content.u.spe_ipv4.src_addr_mask = mask;
	}
	else{
		SPE_ERR("%s: type = %d\n", __FUNCTION__, type);
        return SPE_FAIL;
	}

	return SPE_OK;
}

int filter_ip_src_addr_mask(char *buf)
{
	return  filter_ip_addr_mask(buf, SPE_SRC_IP);
}


int filter_ip_dest_addr_mask(char * buf)
{
	return  filter_ip_addr_mask(buf, SPE_DEST_IP);
}


int filter_ipv4_protocol(char * buf)
{

	if(0 == strcmp(buf, "tcp")){
		spe_cmd_handler.u.ip_content.u.spe_ipv4.protocol = SPE_TCP;
	}
	else if(0 == strcmp(buf, "udp")){
		spe_cmd_handler.u.ip_content.u.spe_ipv4.protocol = SPE_UDP; 	
	}
	else{
		SPE_ERR("protocol %s error ,and protocol should be tcp or udp\n",buf);
		return SPE_FAIL;
	}

    printk("protocol = %s\n", buf);
	return SPE_OK;
}

int filter_tos(char * buf)
{
	unsigned int tos = 255;
	(void)sscanf(buf, "%1d", &tos);	

	if( tos > 8 ){
	    SPE_ERR("tos = %d ,out of rage!\n", tos);
		return SPE_FAIL;		
	}

	spe_cmd_handler.u.ip_content.u.spe_ipv4.tos = tos;
    printk("tos = %d\n", tos);
	return SPE_OK;
}

int filter_src_port_low(char * buf)
{
	unsigned int scr_port_low;
	(void)sscanf(buf, "%5d", &scr_port_low);	
	spe_cmd_handler.u.ip_content.src_port_low = scr_port_low;
    printk("scr port low is %d\n", scr_port_low);
	return SPE_OK;
}

int filter_src_port_high(char * buf)
{
	unsigned int scr_port_high;
	(void)sscanf(buf, "%5d", &scr_port_high);	
	spe_cmd_handler.u.ip_content.src_port_high= scr_port_high;
    printk("scr port high is %d\n", scr_port_high);
	return SPE_OK;
}

int filter_dest_port_low(char * buf)
{
	unsigned int dst_port_low;
	(void)sscanf(buf, "%5d", &dst_port_low);	
	spe_cmd_handler.u.ip_content.dst_port_low = dst_port_low;
    printk("dst port low is %d\n", dst_port_low);
	return SPE_OK;
}

int filter_dest_port_high(char * buf)
{
	unsigned int dst_port_high;
	(void)sscanf(buf, "%5d", &dst_port_high);	
	spe_cmd_handler.u.ip_content.dst_port_high = dst_port_high;
    printk("dst port high is %d\n", dst_port_high);
	return SPE_OK;
}

int spe_black_white_mode_set(char *buf)
{
    if(0 == strcmp(buf, "mac")){
        spe_blackwhite_mode = 0;
    }else if(0 == strcmp(buf, "ip")){
        spe_blackwhite_mode = 1;
    }else{
        SPE_ERR("paramate error! mode can't be %s\n", buf);
        return SPE_FAIL;
    }
    printk("change %s black/white\n", buf);
    return SPE_OK;
}

int spe_black_white_set(char *buf)
{
    if(0 == strcmp(buf, "black")){
        spe_blackwhite = 1;
    }else if(0 == strcmp(buf, "white")){
        spe_blackwhite = 0;
    }else{
        SPE_ERR("paramate error! please input black or white\n");
        return SPE_FAIL;
    }
    printk("change %s black/white\n", buf);
    return SPE_OK;
}


int set_action(char *buf)
{
    if(0 == strcmp(buf, "set")){
        spe_param_action = SPE_PARAM_SET;
    }else if(0 == strcmp(buf, "get")){
        spe_param_action = SPE_PARAM_GET;
    }else{
        SPE_ERR("paramate error! action should be set or get");
        return SPE_FAIL;
    }

    printk("action = %s\n", buf);
    return SPE_OK;
}

int spe_eth_min_len_value_set(char *buf)
{
	(void)sscanf(buf, "%4d", &spe_eth_set_min_len);
	SPE_TRACE("input value is %d\n",spe_eth_set_min_len);

    //spe_eth_minlen bit 0-10bits
	if(spe_eth_set_min_len >= 2048 || spe_eth_set_min_len <= 16){
        SPE_ERR("out of range(17-2047):input value %d\n",spe_eth_set_min_len);
		return SPE_FAIL;
	}
    
	printk("set eth min len %d\n",spe_eth_set_min_len);
	return SPE_OK;
}

int spe_udp_lim_portno_set(char *buf)
{
	(void)sscanf(buf, "%4d", &spe_udp_lim_portno);
    if(spe_udp_lim_portno >= SPE_PORT_NUM){
        SPE_ERR("out of range:port num can't be %d\n", spe_udp_lim_portno);
        return SPE_FAIL;
    }    

    printk("set %d port udp limite\n", spe_udp_lim_portno);
    return SPE_OK;
}

int udp_rate_set (char *buf)
{
    (void)sscanf(buf, "0x%8x", &spe_udp_rate);
    if(spe_udp_rate > SPE_PORT_UDP_LIM_MAX){
        SPE_ERR("out of range:udp limit can't be 0x%x\n", spe_udp_rate);
        return SPE_FAIL;
    }

    printk("set udp limite:0x%x \n", spe_udp_rate);
    return SPE_OK;
}

int spe_port_lim_portno_set(char *buf)
{
    (void)sscanf(buf, "%4d", &spe_port_lim_portno);
    if(spe_port_lim_portno >= SPE_PORT_NUM){
        SPE_ERR("out of range:port num can't be %d\n", spe_port_lim_portno);
        return SPE_FAIL;
    }

    printk("set %d port limite\n", spe_port_lim_portno);
    return SPE_OK;
}

int port_rate_set (char *buf)
{
    (void)sscanf(buf, "0x%8x", &spe_port_rate);
    if(spe_port_rate > SPE_PORT_BYTE_LIM_MAX){
        SPE_ERR("out of range:byte limit can't be 0x%x\n", spe_port_rate);
        return SPE_FAIL;
    }

    printk("set byte limite:0x%x\n", spe_port_rate);
    return SPE_OK;
}

int spe_port_lim_time_set(char *buf)
{
	(void)sscanf(buf, "0x%4x", &spe_port_lim_time);
    if(spe_port_lim_time > SPE_PORT_LIM_TIME_MAX){
        SPE_ERR("out of range:byte limit can't be 0x%x\n", spe_port_lim_time);
        return SPE_FAIL;
    }

    printk("set limite time 0x%x\n", spe_port_lim_time);
    return SPE_OK;
}

int spe_switch(char *buf){
	if(0 == strcmp(buf, "on")){
	    onoff = 0;
    }else if(0 == strcmp(buf, "off")){
	    onoff = 1;
    }else{
        SPE_ERR("parameter error! switch should be on or off\n");
        return SPE_FAIL;
    }
    
    printk("set switch %s\n", buf);
	return SPE_OK;
}

int spe_l4_protocol_get(char * buf)
{

    if(0 == strcmp(buf, "tcp")){
        spe_l4_protocol = SPE_TCP;
    }
    else if(0 == strcmp(buf, "udp")){
        spe_l4_protocol = SPE_UDP;
    }
    else{
        SPE_ERR("protocol %s error ,and protocol should be tcp or udp\n",buf);
        return SPE_FAIL;
    }

    printk("protocol = %s\n", buf);
    return SPE_OK;
}

int spe_l4_portnum_get(char * buf)
{
	int tmp;
    (void)sscanf(buf, "%5d", &tmp);
	spe_l4_port_num = (u16)tmp;
    printk("set l4 portnum : %d\n", spe_l4_port_num);
    return SPE_OK;
}

int char_to_num(char c)
{
    if (('0'<=c) && ('9'>=c))
    {return c-'0';}
    else if (('a'<=c) && ('f'>=c))
    {return c-'a'+10;}
    else if (('A'<=c) && ('F'>=c))
    {return c-'A'+10;}
    else
    {return -1;}
}

void ipv6_resolution(char *ipv6_l,unsigned int *ipv6_m,int *m,int *n)
{
    int *l = m;
    int i = 0;
    while('\0' != *ipv6_l)
    {
        if (':' == *ipv6_l)
        {
            l=n;
            ipv6_l++;
            continue;
        }
        while((':' != *ipv6_l)&&('\0' != *ipv6_l))
        {
            ipv6_m[i] = ipv6_m[i]*16 +char_to_num(*ipv6_l);
            ipv6_l++;
            continue;
        }
        if ('\0' == *ipv6_l)
        {
            (*l)++;
            return;
        }
        ipv6_l++;
        (*l)++;
        i++;
    }
    return;
}
void ipv6_resort(unsigned int *ipv6_m,unsigned int *ipv6,int *m,int *n)
{
    int d = 8-*m-*n;
    int i = 0;
    SPE_TRACE("m=%d,n=%d\n",*m,*n);
    for (i=0; i<*m; i++)
    {    ipv6[i] = ipv6_m[i];  }
    for (i=0; i<d; i++)
    {    ipv6[(*m)+i] = 0;  }
    for (i=0; i<*n; i++)
    {    ipv6[(*m)+d+i] = ipv6_m[(*m)+i];  }
    
    return;
}

int filter_ipv6_addr(char *buf, int type)
{
	unsigned int ipv6[8] = {0};
    unsigned int ipv6_m[8] = {0};
    char ipv6_l[50] = {0};
    int m = 0;
    int n = 0;
    int *m1 = &m;
    int *n1 = &n;
	(void)sscanf(buf,"%50s", ipv6_l);
    SPE_TRACE("%s\n",ipv6_l);
    ipv6_resolution(ipv6_l,ipv6_m,m1,n1);
    SPE_TRACE("ipv6_m  %x:%x:%x:%x:%x:%x:%x:%x\n",ipv6_m[0],ipv6_m[1],ipv6_m[2],ipv6_m[3],ipv6_m[4],ipv6_m[5],ipv6_m[6],ipv6_m[7]);
    ipv6_resort(ipv6_m,ipv6,m1,n1);
   
	if(type == SPE_DEST_IP)
	{
		//dst_addr
		spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr[0] =ipv6[0]<<16 |ipv6[1];
        spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr[1] =ipv6[2]<<16 |ipv6[3];
        spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr[2] =ipv6[4]<<16 |ipv6[5];
        spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr[3] =ipv6[6]<<16 |ipv6[7];        
        printk("dipv6: %pI6\n",spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr);

	}
	else if(type  == SPE_SRC_IP)
	{
		spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr[0]=ipv6[0]<<16 | ipv6[1];
        spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr[1]=ipv6[2]<<16 | ipv6[3];
        spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr[2]=ipv6[4]<<16 | ipv6[5];
        spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr[3]=ipv6[6]<<16 | ipv6[7];
        
        printk("sipv6: %pI6\n",spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr);
	}
	else
	{
		SPE_TRACE("type = %d", type);
		return SPE_FAIL;
	}

	return SPE_OK;

    
}

int filter_ipv6_src_addr(char *buf)
{
	return filter_ipv6_addr(buf, SPE_SRC_IP);
}


int filter_ipv6_dest_addr(char * buf)
{
	return filter_ipv6_addr(buf, SPE_DEST_IP);
}


/**********************************************************
function name: 

description: 
		mask = 0-32,  don't use 255.255.255.xxx
input:
	
output:
	
**********************************************************/

int filter_ipv6_addr_mask(char *buf, int type)
{
	unsigned int mask;
	
	(void)sscanf(buf, "%4d", &mask);

	if(mask > 128)  
	{
		SPE_ERR("out of range, mask shoule less then 128");
		return SPE_FAIL;
	}
	
	if(type == 1)
	{
		//dst_addr
		printk("dmask:%d \n", mask);
		spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr_mask = mask;

	}
	else if(type  == 2)
	{
	    printk("smask:%d \n", mask);
		spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr_mask = mask;
	}
	else
	{
		SPE_ERR("type = %d error \n",type);
	}

	return SPE_OK;

}
int filter_ipv6_src_addr_mask(char *buf)
{
	return filter_ipv6_addr_mask(buf, SPE_SRC_IP);
}


int filter_ipv6_dest_addr_mask(char * buf)
{
	return filter_ipv6_addr_mask(buf, SPE_DEST_IP);
}


int filter_ipv6_protocol(char * buf)
{

	if(0 == strcmp(buf, "tcp"))
	{
		spe_cmd_handler.u.ip_content.u.spe_ipv6.protocol = SPE_TCP;
	}
	else if(0 == strcmp(buf, "udp"))
	{
		spe_cmd_handler.u.ip_content.u.spe_ipv6.protocol = SPE_UDP; 	
	}
	else
	{
		SPE_ERR("protocol %s error ,and protocol should be tcp or udp\n",buf);
		return SPE_FAIL;
	}

	return SPE_OK;

}



int filter_ipv6_tc(char * buf)
{
	unsigned int tc = 255;
	(void)sscanf(buf, "%1d", &tc);	

	if(tc >= 8)
	{
	    SPE_ERR("tc = %s out of range\n",buf);
		return SPE_FAIL;		
	}

	spe_cmd_handler.u.ip_content.u.spe_ipv6.tc = tc;
	return SPE_OK;

}

int filter_ipv6_next_hdr(char * buf)
{
	unsigned int hdr;
	(void)sscanf(buf, "%4d", &hdr);	

	spe_cmd_handler.u.ip_content.u.spe_ipv6.next_hdr = hdr;
	return SPE_OK;

}

int filter_ipv6_flow_label(char * buf)
{
	unsigned int label;
	(void)sscanf(buf, "%4d", &label);	

	spe_cmd_handler.u.ip_content.u.spe_ipv6.flow_label = label;
	return SPE_OK;

}

extern bool parameq(const char *a, const char *b);

/*****************************************************
cmd line parse routine begins here
*****************************************************/
static int spe_parse_one(char *param, char *val)
{
	int i;
	int err;

    SPE_TRACE("%s:param=%s,strlen(param)=%d,val=%s\n",__FUNCTION__,param,strlen(param),val);

	if (!val)
	{
		return -EINVAL;
	}

	/* Find parameter */
    SPE_TRACE("num:%d\n",spe_cmd_handler.num_funcs);
	for (i = 0; i < spe_cmd_handler.num_funcs; i++) 
	{
        
        if (parameq(param, spe_cmd_handler.filter_parser_ops[i].name)) 
		{
			/* No one handled NULL, so do it here. */
            SPE_TRACE("name:%s\n",spe_cmd_handler.filter_parser_ops[i].name);
			err = spe_cmd_handler.filter_parser_ops[i].setup_func(val);
			return err;
		}
	}

	SPE_ERR("unknown argument: %s\n", param);	
	return SPE_FAIL;
	
}



static char *find_next_arg(char *args, char **param, char **val)
{
	unsigned int i, equals = 0;
	char *next;


	for (i = 0; args[i]; i++) 
	{
		if (isspace(args[i]))
		{
			break;
		}
		if (equals == 0) 
		{
			if (args[i] == '=')
			{
				equals = i;
			}
				
		}
	}

	*param = args;
	if (!equals)
	{
		*val = NULL;
	}
	else 
	{
		args[equals] = '\0';
		*val = args + equals + 1;
	}

	if (args[i]) 
	{
		args[i] = '\0';
		next = args + i + 1;
	}
	else
	{
		next = args + i;
	}
		

	/* Chew up trailing spaces. */
	return skip_spaces(next);
}



int exe_spe_mac_filter_cmd(void)
{
    int ret = SPE_OK;
    struct spe_mac_filt_entry mac_filter;
    
    SPE_TRACE("exe_spe_mac_filter_cmd  enter\n");
    switch(spe_cmd_handler.action){
    case SPE_ENTRY_DUMP:
		ret = mac_filter_dump();
        break;
	//write reg and wait
	case SPE_ENTRY_DELETE:
        memcpy(&mac_filter.mac_hi, spe_cmd_handler.u.mac_content.mac, sizeof(char)*4);
        memcpy(&mac_filter.mac_lo, spe_cmd_handler.u.mac_content.mac+4, sizeof(char)*2);
		mac_filter.redir_act = spe_cmd_handler.u.mac_content.action;
		mac_filter.redir_port = spe_cmd_handler.u.mac_content.redirect;

		ret = spe_mac_filt_del(&mac_filter);        
		break;
	case SPE_ENTRY_ADD:
        memcpy(&mac_filter.mac_hi, spe_cmd_handler.u.mac_content.mac, sizeof(char)*4);
        memcpy(&mac_filter.mac_lo, spe_cmd_handler.u.mac_content.mac+4, sizeof(char)*2);
                
		mac_filter.redir_act = spe_cmd_handler.u.mac_content.action;
		mac_filter.redir_port = spe_cmd_handler.u.mac_content.redirect;
        
		ret = spe_mac_filt_add(&mac_filter);
		break;
	default:
        break;
    }
    if(ret){
        SPE_ERR("exe spe mac filter cmd error!\n");
    }
    return ret;
}


int exe_spe_mac_forward_cmd(void)
{
	//write reg and wait
	return 0;
}


int exe_spe_ip_filter_cmd(void)
{
    int ret = 0;

    switch(spe_cmd_handler.action){
	case SPE_ENTRY_DUMP:
		ret = ip_filter_dump();
        break;
	case SPE_ENTRY_DELETE:
        if (spe_cmd_handler.type == SPE_IP_FILTER_TABLE){
            struct spe_ipv4_filt_entry ipv4_filter;
            ipv4_filter.filt_mask= spe_cmd_handler.u.ip_content.filter_mask;
    		ipv4_filter.dport_max= cpu_to_be16(spe_cmd_handler.u.ip_content.dst_port_high);
    		ipv4_filter.dport_min= cpu_to_be16(spe_cmd_handler.u.ip_content.dst_port_low);
    		ipv4_filter.sport_max= cpu_to_be16(spe_cmd_handler.u.ip_content.src_port_high);
    		ipv4_filter.sport_min= cpu_to_be16(spe_cmd_handler.u.ip_content.src_port_low);
            
            ipv4_filter.sip= spe_cmd_handler.u.ip_content.u.spe_ipv4.src_addr;
            ipv4_filter.sip = cpu_to_be32(ipv4_filter.sip);
            ipv4_filter.sip_mask= spe_cmd_handler.u.ip_content.u.spe_ipv4.src_addr_mask;
            ipv4_filter.dip= spe_cmd_handler.u.ip_content.u.spe_ipv4.dst_addr;            
            ipv4_filter.dip = cpu_to_be32(ipv4_filter.dip);
            ipv4_filter.dip_mask= spe_cmd_handler.u.ip_content.u.spe_ipv4.dst_addr_mask;
            ipv4_filter.tos= spe_cmd_handler.u.ip_content.u.spe_ipv4.tos;
            ipv4_filter.protocol= spe_cmd_handler.u.ip_content.u.spe_ipv4.protocol;

            ret = spe_ipv4_filt_del(&ipv4_filter);    
        }
        if (spe_cmd_handler.type == SPE_IPV6_FILTER_TABLE){
            struct spe_ipv6_filt_entry ipv6_filter;
            ipv6_filter.filt_mask = spe_cmd_handler.u.ip_content.filter_mask;
    		ipv6_filter.dport_max= cpu_to_be16(spe_cmd_handler.u.ip_content.dst_port_high);
    		ipv6_filter.dport_min= cpu_to_be16(spe_cmd_handler.u.ip_content.dst_port_low);
    		ipv6_filter.sport_max= cpu_to_be16(spe_cmd_handler.u.ip_content.src_port_high);
    		ipv6_filter.sport_min= cpu_to_be16(spe_cmd_handler.u.ip_content.src_port_low);
            
            ipv6_filter.sip[0] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr[0]);
            ipv6_filter.sip[1] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr[1]);
            ipv6_filter.sip[2] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr[2]);
            ipv6_filter.sip[3] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr[3]);
    		ipv6_filter.sip_mask= spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr_mask;

            ipv6_filter.dip[0] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr[0]);
            ipv6_filter.dip[1] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr[1]);
            ipv6_filter.dip[2] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr[2]);
            ipv6_filter.dip[3] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr[3]);
			ipv6_filter.dip_mask= spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr_mask;

            ipv6_filter.tos= spe_cmd_handler.u.ip_content.u.spe_ipv6.tc;
            ipv6_filter.flow_label = spe_cmd_handler.u.ip_content.u.spe_ipv6.flow_label;
            ipv6_filter.protocol= spe_cmd_handler.u.ip_content.u.spe_ipv6.protocol;

            ipv6_filter.protocol= spe_cmd_handler.u.ip_content.u.spe_ipv6.protocol;

            ret = spe_ipv6_filt_del(&ipv6_filter);   
        }
        break;
    case SPE_ENTRY_ADD:
		if (spe_cmd_handler.type == SPE_IP_FILTER_TABLE){
            struct spe_ipv4_filt_entry ipv4_filter;
            ipv4_filter.filt_mask= spe_cmd_handler.u.ip_content.filter_mask;
    		ipv4_filter.dport_max= cpu_to_be16(spe_cmd_handler.u.ip_content.dst_port_high);
    		ipv4_filter.dport_min= cpu_to_be16(spe_cmd_handler.u.ip_content.dst_port_low);
    		ipv4_filter.sport_max= cpu_to_be16(spe_cmd_handler.u.ip_content.src_port_high);
    		ipv4_filter.sport_min= cpu_to_be16(spe_cmd_handler.u.ip_content.src_port_low);
            
            ipv4_filter.sip= spe_cmd_handler.u.ip_content.u.spe_ipv4.src_addr;
            ipv4_filter.sip = cpu_to_be32(ipv4_filter.sip);
            ipv4_filter.sip_mask= spe_cmd_handler.u.ip_content.u.spe_ipv4.src_addr_mask;
            ipv4_filter.dip= spe_cmd_handler.u.ip_content.u.spe_ipv4.dst_addr;            
            ipv4_filter.dip = cpu_to_be32(ipv4_filter.dip);
            ipv4_filter.dip_mask= spe_cmd_handler.u.ip_content.u.spe_ipv4.dst_addr_mask;

            ipv4_filter.tos= spe_cmd_handler.u.ip_content.u.spe_ipv4.tos;
            ipv4_filter.protocol= spe_cmd_handler.u.ip_content.u.spe_ipv4.protocol;

            ret = spe_ipv4_filt_add(&ipv4_filter);
		}

		if (spe_cmd_handler.type == SPE_IPV6_FILTER_TABLE){
            struct spe_ipv6_filt_entry ipv6_filter;
            u32 label;
            ipv6_filter.filt_mask = spe_cmd_handler.u.ip_content.filter_mask;
    		ipv6_filter.dport_max= cpu_to_be16(spe_cmd_handler.u.ip_content.dst_port_high);
    		ipv6_filter.dport_min= cpu_to_be16(spe_cmd_handler.u.ip_content.dst_port_low);
    		ipv6_filter.sport_max= cpu_to_be16(spe_cmd_handler.u.ip_content.src_port_high);
    		ipv6_filter.sport_min= cpu_to_be16(spe_cmd_handler.u.ip_content.src_port_low);
            ipv6_filter.sip[0] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr[0]);
            ipv6_filter.sip[1] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr[1]);
            ipv6_filter.sip[2] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr[2]);
            ipv6_filter.sip[3] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr[3]);
            ipv6_filter.sip_mask= spe_cmd_handler.u.ip_content.u.spe_ipv6.src_addr_mask;

            ipv6_filter.dip[0] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr[0]);
            ipv6_filter.dip[1] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr[1]);
            ipv6_filter.dip[2] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr[2]);
            ipv6_filter.dip[3] = cpu_to_be32(spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr[3]);
			ipv6_filter.dip_mask= spe_cmd_handler.u.ip_content.u.spe_ipv6.dst_addr_mask;

            ipv6_filter.tos= spe_cmd_handler.u.ip_content.u.spe_ipv6.tc;
            label = spe_cmd_handler.u.ip_content.u.spe_ipv6.flow_label<<8;
            ipv6_filter.flow_label = cpu_to_be32(label);
            ipv6_filter.protocol= spe_cmd_handler.u.ip_content.u.spe_ipv6.protocol;

            ret = spe_ipv6_filt_add(&ipv6_filter);
        }
        break;

    default:
        break;
    }
    if(ret){        
        SPE_ERR("exe spe ip cmd error!\n");
    }

	return ret;
}

int exe_spe_cmd(void)
{
	int ret = 0;
	
	//write spe reg
	switch(spe_cmd_handler.type){
    case SPE_IP_FILTER_TABLE:
        if(spe_cmd_handler.action == SPE_ENTRY_DUMP){
            spe_ipv4_filter_dump();
            break;
        }else if (0 != spe_cmd_handler.u.ip_content.iptype){
            SPE_ERR("ipv4 filter set parameter error!\n");
            ret = -EINVAL;
        }        
        ret = exe_spe_ip_filter_cmd();
        break;
    case SPE_IPV6_FILTER_TABLE:
        if(spe_cmd_handler.action == SPE_ENTRY_DUMP){
            spe_ipv6_filter_dump();
            break;
        }else if (1 != spe_cmd_handler.u.ip_content.iptype){
            SPE_ERR("ipv6 filter set parameter error!\n");
            ret = -EINVAL;
        }        
        ret = exe_spe_ip_filter_cmd();
        break;
    case SPE_MAC_FILTER_TABLE:
        if(spe_cmd_handler.action == SPE_ENTRY_DUMP){
            spe_macfl_entry_dump();
            break;
        }
        ret = exe_spe_mac_filter_cmd();
        break;
    case SPE_MAC_FORWARD_TABLE:
        if(spe_cmd_handler.action == SPE_ENTRY_DUMP){
            spe_macfw_entry_dump();
            break;
        }
        break;
    case SPE_BLACK_WHITE:
        if(spe_param_action == SPE_PARAM_SET){
            spe_entry_set_black_white(spe_blackwhite_mode, spe_blackwhite);            
            SPE_TRACE("spe set black white \n");
        }else if(spe_param_action == SPE_PARAM_GET){
            spe_entry_get_black_white();
        }else{            
            SPE_ERR("black/white set parameter error!\n");
        }
        break;
    case SPE_ETH_MIN_LEN:
        if(spe_param_action == SPE_PARAM_SET){
            spe_writel(spe_balong.regs,HI_SPE_ETH_MINLEN_OFFSET,spe_eth_set_min_len);
             SPE_TRACE("\n register value is %d\n",spe_readl(spe_balong.regs, HI_SPE_ETH_MINLEN_OFFSET));
        }else if(spe_param_action == SPE_PARAM_GET){
            spe_eth_set_min_len = spe_readl(spe_balong.regs, HI_SPE_ETH_MINLEN_OFFSET);
            printk("eth min len is %d\n", spe_eth_set_min_len & 0x7FFF);
        }else{            
            SPE_ERR("eth_min_len set parameter error!\n");
        }
        break;
    case SPE_UDP_LMTNUM_TABLE:
        if(spe_param_action == SPE_PARAM_SET){
            spe_writel(spe_balong.regs, SPE_PORTX_UDP_RATE_LIMIT(spe_udp_lim_portno), spe_udp_rate);
            SPE_TRACE("udp limit:0x%x\n",spe_readl(spe_balong.regs, SPE_PORTX_UDP_RATE_LIMIT(spe_udp_lim_portno)));
        }else if(spe_param_action == SPE_PARAM_GET){
            spe_udp_rate = spe_readl(spe_balong.regs, SPE_PORTX_UDP_RATE_LIMIT(spe_udp_lim_portno));
            printk("port(%d) udp limit is 0x%x\n", spe_udp_lim_portno, spe_udp_rate);            
        }else{            
            SPE_ERR("udp limit num set parameter error!\n");
        }
        break;
    case SPE_LMTBYTE_TABLE:  
        if(spe_param_action == SPE_PARAM_SET){
            spe_writel(spe_balong.regs, SPE_PORTX_RATE_LIMIT(spe_port_lim_portno), spe_port_rate);
            SPE_TRACE("limit:0x%x\n",spe_readl(spe_balong.regs, SPE_PORTX_RATE_LIMIT(spe_port_lim_portno)));
        }else if(spe_param_action == SPE_PARAM_GET){
            spe_port_rate = spe_readl(spe_balong.regs, SPE_PORTX_RATE_LIMIT(spe_port_lim_portno));
            printk("port(%d) udp limit is 0x%x\n", spe_port_lim_portno, spe_port_rate);         
        }else{            
            SPE_ERR("byte limit set parameter error!\n");
        }
        break;
    case SPE_LMT_TIME_TABLE:
        if(spe_param_action == SPE_PARAM_SET){
            HI_SPE_PORT_LMTTIME_T lim_time;
            lim_time.u32 = spe_readl(spe_balong.regs, HI_SPE_PORT_LMTTIME_OFFSET);
            lim_time.bits.spe_port_lmttime = spe_port_lim_time;
            spe_writel(spe_balong.regs, HI_SPE_PORT_LMTTIME_OFFSET, lim_time.u32);
            SPE_TRACE("limit time:0x%x\n",spe_readl(spe_balong.regs, HI_SPE_PORT_LMTTIME_OFFSET));

        }else if(spe_param_action == SPE_PARAM_GET){
            HI_SPE_PORT_LMTTIME_T lim_time;
            lim_time.u32 = spe_readl(spe_balong.regs, HI_SPE_PORT_LMTTIME_OFFSET);
            printk("lim time :0x%x \n", lim_time.bits.spe_port_lmttime);            
        }else{            
            SPE_ERR("limit time set parameter error!\n");
        }
        break;
    case SPE_DYNAMIC_SWITCH:
        if(spe_param_action == SPE_PARAM_SET){
            if(onoff){
                spe_transfer_pause();
            }
            else{
                spe_transfer_restart(); 
            }
        }else if(spe_param_action == SPE_PARAM_GET){
            printk("dynamic swich is %s \n", onoff?"off":"on");            
        }else{            
            SPE_ERR("dynamic switch set parameter error!\n");
        }
        break;
    case SPE_L4_PORTNUM:
        if(spe_cmd_handler.action == SPE_ENTRY_ADD){
            ret = spe_port_l4portno_to_cpu_add(spe_l4_port_num, spe_l4_protocol);
        }else if(spe_cmd_handler.action == SPE_ENTRY_DELETE){
            ret = spe_port_l4portno_to_cpu_del(spe_l4_port_num, spe_l4_protocol);
        }else if(spe_cmd_handler.action == SPE_ENTRY_DUMP){
            spe_port_l4portno_to_cpu_dump();
        }else{            
            SPE_ERR("l4 portnum set parameter error!\n");
        }
        break;
    default:
        SPE_ERR("spe_cmd_handler.type = %d, error", spe_cmd_handler.type);
        ret = SPE_FAIL;
        break;
    }
    
    return ret;
}

int filter_cmd_parser(char *args)
{
	char *param, *val;
	int cnt = 0;
	int ret = SPE_OK;
	char type_str[20];
		
	SPE_TRACE("Parsing ARGS: %s\n", args);
    
	last_filter_type = spe_cmd_handler.type; //just for debug

	//clean all field
	memset(&spe_cmd_handler, 0, sizeof(spe_cmd_handler));
	
	/* clean leading spaces */
	args = skip_spaces(args);

	//cmdline must start with  "type=xx"  xx : ip, ipv6, mac
	//get type first
	
	if(0 != strncmp(args, "type=", strlen("type=")))
	{
		filter_cmd_help(args);
		return SPE_FAIL;
	}

	memset(type_str, 0, sizeof(type_str));

	//need to limit size of type_str
	cnt = sscanf(args, "type=%15s", type_str);
	
	ret = spe_sysfs_cmd_type(type_str);

	if(ret == SPE_FAIL)
	{		
        SPE_TRACE("cmd: %s\n", args);
		SPE_TRACE("cmd: %s, fail\n", type_str);
	}

	args = args + strlen(type_str) + strlen("type=");	
	args = skip_spaces(args);

    SPE_TRACE("args=%s\n",args);
	while (*args) {
		int ret;        

		args = find_next_arg(args, &param, &val);
		ret = spe_parse_one(param, val);

		switch (ret) {
		case SPE_FAIL:
			SPE_ERR(KERN_ERR "%s: parameter error '%s'\n",
				   __FUNCTION__, val);
			return ret;

		case SPE_OK:
			break;
			
		default:
			SPE_ERR(KERN_ERR
				   "%s: '%s' invalid for parameter '%s'\n",
				   __FUNCTION__, val ?: "", param);
			return ret;
		}
	}

	return ret;
}


int mac_filter_dump(void)
{
	//set again for some special target, don't delete
	spe_cmd_handler.action = SPE_ENTRY_DUMP;
	//set spe reg

	//wait for dump ok
	
	//decode mac filter in specified memory area
	return SPE_OK;
	
}


int ip_filter_dump(void)
{
	
	//set again for some special target, don't delete
	spe_cmd_handler.action = SPE_ENTRY_DUMP;
	//set spe reg

	//wait for dump ok
	
	//decode ip filter in specified memory area
	return SPE_OK;	

}





void filter_cmd_help(char *args)
{
	if(args)
	{
		SPE_TRACE("cmd: %s\n", args);		
	}
	
	SPE_ERR("cmd should start with type=ip, type=ipv6 or type=mac\n");

	return;
}

/****************************************************************************/
//                                                                          
//  End of file.                                                        
//                                                            
/****************************************************************************/

