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

#ifndef __SPE_ENTRY_H__
#define __SPE_ENTRY_H__

#include "spe.h"

#define SPE_IPFW_HTABLE_SIZE        (512)
#define SPE_IPFW_HTABLE_ZONE        (0)
#define SPE_IPFW_HTABLE_RAND        (0)
#define SPE_IPFW_L3PROTO_IPV4       (2)
#define SPE_IPFW_L3PROTO_IPV6       (10)

#define BIT(nr)			(1UL << (nr))
#define IS_MAC_FW_ENTRY (BIT(17))
#define IS_MAC_FL_ENTRY (BIT(16))
#define IS_IP_FL_ENTRY (BIT(31))

enum spe_tab_list_type{
    spe_tab_list_mac_filt = 0,
    spe_tab_list_ipv4_filt,
    spe_tab_list_ipv6_filt,
    spe_tab_list_bottom
};

enum spe_tab_ctrl_type{
    spe_tab_ctrl_add_mac_filt = 0,
    spe_tab_ctrl_add_mac_fw,
    spe_tab_ctrl_add_ipv4_filt,
    spe_tab_ctrl_add_ipv6_filt,
    spe_tab_ctrl_del_mac_filt,
    spe_tab_ctrl_del_mac_fw,
    spe_tab_ctrl_del_ipv4_filt,
    spe_tab_ctrl_del_ipv6_filt,
    spe_tab_ctrl_bottom
};

struct spe_mac_fw_entry {
    u32 mac_hi;                     /* mac in big endian */
    u32 mac_lo;                     /* mac in big endian */
    u16 vid:12;                     /* vlan id in big endian */
    u16 reserve0:4;
    u8 port_br_id:4;               /* br id this entry belongs to */
    u8 port_no:4;                  /* port no */
    u8 reserve1;
    u16 timestamp;                  /* mac fw timestamp,software no care. */
    u16 is_static;                  /* is static entry */
};

struct spe_mac_fw_entry_ext {
    struct list_head list;
    struct spe_mac_fw_entry ent;
};

struct spe_mac_filt_entry {
    u32 mac_hi;                     /* mac in big endian */
    u16 mac_lo;                     /* mac in big endian */
    u16 reserve;
    u8 redir_act:1;                 /* 0:no redir 1:redir */
    u8 reserve1:7;
    u8 redir_port:4;                /* redir port */
    u8 reserve2:4;
};

struct spe_ipv4_filt_entry {
    u32 sip;                        /* source ip in big endian */
    u32 sip_mask;
    u32 dip;                        /* dest ip in big endian */
    u32 dip_mask;
    u16 sport_min;                  /* source min port in big endian */
    u16 sport_max;                  /* source max port in big endian */
    u16 dport_min;                  /* dest min port in big endian */
    u16 dport_max;                  /* dest max port in big endian */
    u8 protocol;
    u8 tos;
    u8 filt_mask;
    u8 reserve;
};

struct spe_ipv6_filt_entry {
    u32 sip[4];                     /* source ip in big endian */
    u32 sip_mask;
    u32 dip[4];                     /* dest ip in big endian */
    u32 dip_mask;
    u16 sport_min;                  /* source min port in big endian */
    u16 sport_max;                  /* source max port in big endian */
    u16 dport_min;                  /* dest min port in big endian */
    u16 dport_max;                  /* dest max port in big endian */
    u8 protocol;
    u8 tos;
    u16 reserve1;
    u32 flow_label:20;              /* flow label in big endian */
    u32 reserve2:12;
    u8 filt_mask;
    u8 valid:1;
    u8 reserve3:7;
    u16 reserve4;
};

struct spe_ipv4_filter_entry_in_sram {
	u32 sip_mask:8;
    u32 dip_mask:8;
    u32 protocol:8;
    u32 tos:8;
    u32 filter_mask:8;
    u32 reserve:23;
    u32 valid:1;
    char sip[4];
    char dip[4];
    u16 sport_min;
    u16 sport_max;
    u16 dport_min;
    u16 dport_max;
};

struct spe_ipv6_filter_entry_in_sram {
	u32 sip_mask:8;
    u32 dip_mask:8;
    u32 next_hdr:8;
    u32 tc:8;
    u32 filter_mask:8;
    u32 flow_label:20;
    u32 reserve:3;
    u32 valid:1;
    u16 sip[8];
    u16 dip[8];
    u16 sport_min;
    u16 sport_max;
    u16 dport_min;
    u16 dport_max;
};

void spe_entry_set_black_white(enum spe_tab_list_type list_type, unsigned int black_white);
void spe_entry_get_black_white(void);
int spe_mac_filt_add(struct spe_mac_filt_entry *ent);
int spe_mac_filt_del(struct spe_mac_filt_entry *ent);
int spe_ipv4_filt_add(struct spe_ipv4_filt_entry *ent);
int spe_ipv4_filt_del(struct spe_ipv4_filt_entry *ent);
int spe_ipv6_filt_add(struct spe_ipv6_filt_entry *ent);
int spe_ipv6_filt_del(struct spe_ipv6_filt_entry *ent);
void spe_ip_fw_htab_set(struct spe *spe);
void spe_transfer_pause(void);
void spe_transfer_restart(void);
int spe_mac_fw_add(const char *mac, uint16_t vid, u32 br_speno, u32 src_speno, u32 is_static);
int spe_mac_fw_del(char *mac, uint16_t vid, u32 br_speno, u32 src_speno);
void spe_macfw_entry_dump(void);
void spe_macfl_entry_dump(void);
void spe_ipv4_filter_dump(void);
void spe_ipv6_filter_dump(void);


#endif /*__SPE_ENTRY_H__*/
