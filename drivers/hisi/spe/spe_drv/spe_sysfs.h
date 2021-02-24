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
	 
#ifndef SPE_SYSFS_H
#define SPE_SYSFS_H


//#include

//macro define



#define SPE_OK 		0
#define SPE_FAIL 	1

#define SPE_DEST_IP	1
#define SPE_SRC_IP	2


#define  SPE_IP_FILTER_TABLE 	1
#define  SPE_IPV6_FILTER_TABLE 	2
#define  SPE_MAC_FILTER_TABLE 	3
#define  SPE_MAC_FORWARD_TABLE  4
#define  SPE_BLACK_WHITE        5
#define  SPE_ETH_MIN_LEN 		6
#define  SPE_UDP_LMTNUM_TABLE   7
#define  SPE_LMTBYTE_TABLE      8
#define  SPE_LMT_TIME_TABLE     9
#define  SPE_DYNAMIC_SWITCH		10
#define  SPE_L4_PORTNUM			11
	
//ACTION defines
#define  SPE_ENTRY_ADD		1
#define  SPE_ENTRY_DELETE	2
#define  SPE_ENTRY_DUMP		3
#define  SPE_PARAM_SET      4
#define  SPE_PARAM_GET      5

#define HISI_SPE_DEBUG

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif


//structure define

struct spe_filter_param 
{
	const char *name;
	int (*setup_func)(char *);
	int flag;  //just reserved now
};

struct mac_filter_entry
{
	char mac[6];
	char reserved;
	unsigned action:4;	
	unsigned redirect:4;
};

struct mac_forward_entry
{
	char mac[6];
	u16	 port_index;
};


typedef struct ipv4_contex {
	unsigned int tos:8;
	unsigned int protocol:8;
	unsigned int src_addr;
	unsigned int src_addr_mask;
	unsigned int dst_addr;
	unsigned int dst_addr_mask;
}ipv4_ctx_t;

typedef struct ipv6_contex {
	unsigned int tc:8;
	unsigned int next_hdr:8;
	unsigned int flow_label;
	unsigned int src_addr[4];
	unsigned int src_addr_mask;
	unsigned int dst_addr[4];
	unsigned int dst_addr_mask;	
	unsigned int protocol:8;
}ipv6_ctx_t;

typedef struct ip_filter_entry {
	unsigned int filter_mask;
	unsigned int src_port_low:16;
	unsigned int src_port_high:16;
	unsigned int dst_port_low:16;
	unsigned int dst_port_high:16;
	unsigned int reserved:15;
	unsigned int iptype:1;
	union {
	ipv4_ctx_t	  spe_ipv4;
	ipv6_ctx_t	  spe_ipv6;
	}u;
	
}ip_filter_entry_t;


struct spe_cmd_handler_s
{

	int action;
	int type;		
	int num_funcs;
	
	union 
	{
		struct mac_filter_entry mac_content;
		struct ip_filter_entry ip_content;
	}u;
	
	struct spe_filter_param *filter_parser_ops;

};

struct mac_forward_handler_s
{
	int action;
	int type;		
	struct mac_forward_entry mac_forward;
};

//variables define

//function declaration

int mac_filter_dump(void);
int ip_filter_dump(void);

int filter_cmd_parser(char *args);

int exe_spe_cmd(void);

int filter_mac_addr(char *buf);
int filter_action(char *buf);
int filter_mac_reaction(char *buf);
int filter_mac_redirect(char *buf);

int forward_action(char *buf);
int filter_ip_mask(char *buf);
int filter_src_port_low(char * buf);
int filter_src_port_high(char * buf);
int filter_dest_port_low(char * buf);

int filter_dest_port_high(char * buf);

int filter_ip_src_addr(char *buf);
int filter_ip_dest_addr(char * buf);
int filter_ip_src_addr_mask(char *buf);
int filter_ip_dest_addr_mask(char * buf);
int filter_ipv4_protocol(char * buf);
int filter_tos(char * buf);
int filter_ipv6_src_addr(char *buf);
int filter_ipv6_dest_addr(char * buf);
int filter_ipv6_src_addr_mask(char *buf);
int filter_ipv6_dest_addr_mask(char * buf);
int filter_ipv6_protocol(char * buf);
int filter_ipv6_tc(char * buf);

int filter_ipv6_next_hdr(char * buf);
int filter_ipv6_flow_label(char * buf);
void filter_cmd_help(char *args);

int spe_black_white_mode_set(char *buf);
int spe_black_white_set(char *buf);
int spe_eth_min_len_value_set(char *buf);
int spe_udp_lim_portno_set(char *buf);
int udp_rate_set(char *buf);
int spe_port_lim_portno_set(char *buf);
int port_rate_set(char *buf);
int spe_port_lim_time_set(char *buf);

int spe_switch(char * buf);
int set_action(char *buf);

int spe_l4_protocol_get(char * buf);
int spe_l4_portnum_get(char * buf);

#endif

/****************************************************************************/
//                                                                          
//  End of file.                                                        
//                                                            
/****************************************************************************/
