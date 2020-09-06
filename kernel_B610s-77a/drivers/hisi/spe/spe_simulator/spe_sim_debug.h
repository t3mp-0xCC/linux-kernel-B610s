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

/* spe msg system */
#define SPE_SIM_MSG_ERR			BIT(0)
#define SPE_SIM_MSG_TRACE		BIT(1)
#define SPE_SIM_MSG_INFO		BIT(2)
#define SPE_SIM_MSG_DBG			BIT(3)
#define SPE_SIM_MSG_DUMP		BIT(4)
#define SPE_SIM_MSG_TUPLE       BIT(5)

typedef struct spe_sim_port_debug
{
    int stat_td_proc;
    int stat_td_start_proc;
    int stat_td_complete;
    int stat_td_intr;

    int stat_rd_proc;
    int stat_rd_empty;
    int stat_rd_complete;
    int stat_rd_intr;

    int stat_entry_proc;
    int stat_mac_entry_proc;
    int stat_ip_entry_proc;

    int stat_to_cpu_proc;
    int stat_do_nat;

	/*pkt drop as src port*/
	unsigned int total_pkt_drop;
	unsigned int unwarp_fail_drop;
	unsigned int ncm_unwarp_fail_drop;
	unsigned int rndis_unwarp_fail_drop;
	unsigned int eth_check_fail_drop;
	unsigned int eth_filter_drop;
	unsigned int ipv4_check_fail_drop;
	unsigned int ipv6_check_fail_drop;
	unsigned int ip_filter_drop;
}spe_sim_port_debug_t;

extern unsigned int spe_msg_level;

#define SPE_SIM_ERR(fmt, ...)		\
	do {\
			printk(fmt, ##__VA_ARGS__);\
		} while (0)

#define SPE_SIM_INFO(fmt, ...)		\
	do {\
		if (spe_msg_level & SPE_SIM_MSG_INFO) \
			printk(fmt, ##__VA_ARGS__);\
		} while (0)

#define SPE_SIM_TRACE(fmt, ...)		\
	do {\
		if (spe_msg_level & SPE_SIM_MSG_TRACE) \
			printk(fmt, ##__VA_ARGS__);\
		} while (0)
		
#define SPE_SIM_DBG(fmt, ...)		\
	do {\
		if (spe_msg_level & SPE_SIM_MSG_DBG) \
			printk(fmt, ##__VA_ARGS__);\
		} while (0)
		
#define SPE_SIM_DUMP(fmt, ...)		\
	do {\
		if (spe_msg_level & SPE_SIM_MSG_DUMP) \
			printk("[%s: ]"fmt, __func__, ##__VA_ARGS__);\
		} while (0)
