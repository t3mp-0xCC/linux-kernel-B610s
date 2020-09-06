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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS"
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

#ifndef __SPE_INTERFACE_H__
#define __SPE_INTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*__cplusplus*/

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
#include <linux/skbuff.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <linux/netdevice.h>
#include <linux/types.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_tuple.h>

#define SPE_PORT_NUM   (8)
#define SPE_PORT_MAX   (32)

#define SPE_DFT_SKB_LEN             (1800)
#define SPE_INVALID_PORT(x)         ((x)>=SPE_PORT_MAX)
#define SPE_BR_PORT(x)              (((x)&0xf)>=SPE_PORT_NUM)
#define SPE_NORM_PORT(x)             (((x)&0xf)<SPE_PORT_NUM)

/* error code */
#define SPE_ERR_MODU                (0x1000)
#define SPE_ERR_BASE                (SPE_ERR_MODU<<16)

#define SPE_ERR_TDFULL              -(SPE_ERR_BASE + 1)
#define SPE_ERR_RDFULL              -(SPE_ERR_BASE + 2)
#define SPE_ERR_NOCHAN              -(SPE_ERR_BASE + 3)
#define SPE_ERR_ENTRYOP             -(SPE_ERR_BASE + 4)
#define SPE_ERR_PORT_DISABLED       -(SPE_ERR_BASE + 5)
#define SPE_ERR_PORT_BUSY           -(SPE_ERR_BASE + 6)
#define SPE_ERR_PORT_ENABLED        -(SPE_ERR_BASE + 7)

#define	ETHER_ADDR_LEN		6
struct ether_addr {
	uint8_t octet[ETHER_ADDR_LEN];
} ;

typedef enum spe_l3_type{
    spe_l3_ipv4,
    spe_l3_ipv6,
    spe_l3_bottom
}spe_l3_type_t;

enum spe_mode{
    spe_mode_normal,
    spe_mode_stick,
    spe_mode_bottom
};

enum spe_enc_type{
    spe_enc_none,
    spe_enc_ncm_ntb16,
    spe_enc_ncm_ntb32,
    spe_enc_rndis,
    spe_enc_wifi,
    spe_enc_ipf,
    spe_enc_cpu,
    spe_enc_bottom
};

enum spe_attach_brg_type{
    spe_attach_brg_brif,
    spe_attach_brg_normal,
    spe_attach_brg_mix,
    spe_attach_brg_br,
    spe_attach_brg_bottom
};

enum spe_port_ioctl_cmd{
    spe_port_ioctl_set_attach_brg  = 0,
    spe_port_ioctl_set_mac,
    spe_port_ioctl_set_ipv4,
    spe_port_ioctl_set_ipv6,
    spe_port_ioctl_set_udp_time,    /*unit:ms*/
    spe_port_ioctl_set_udp_cnt,
    spe_port_ioctl_set_rate_time,   /*unit:ms*/
    spe_port_ioctl_set_rate_byte,
    spe_port_ioctl_cmd_bottom
};

enum spe_rd_fw{
    rd_fw_mac = 0,
    rd_fw_eth,
    rd_fw_first_mac,
    rd_fw_iph,
    rd_fw_ip_filter,
    rd_fw_tcp_or_udp,
    rd_fw_hash_calculate,
    rd_fw_hash_read,
    rd_fw_hash_entry_mach,
    rd_fw_second_mac,
    rd_fw_end,
    rd_fw_bottom
 };

enum spe_td_fw{
    td_fw_mac = 0,
    td_fw_eth,
    td_fw_first_mac,
    td_fw_iph,
    td_fw_ip_filter,
    td_fw_tcp_or_udp,
    td_fw_hash_calculate,
    td_fw_hash_read,
    td_fw_hash_entry_mach,
    td_fw_second_mac,
    td_fw_end,
    td_fw_bottom
 };

enum spe_td_result{
    td_result_updata_only = 0,
    td_result_discard,
    td_result_normal,
    td_result_wrap_or_lenth_wrong,
    td_result_bottom
};
enum spe_td_reason{
    td_reason_undiscard = 0,
    td_reason_wrap_wrong,
    td_reason_length_wrong,
    td_reason_sport_disable,
    td_reason_mac_filt,
    td_reason_ipv4_csum,
    td_reason_ttl,
    td_reason_ip_filt,
    td_reason_udp_rate_limit,
    td_reason_mac_fw_entry_wrong,
    td_reason_dport_disable,
    td_reason_protocol_differ,
    td_reason_rd_port_null,
    td_reason_bottom
};

enum spe_td_indicate{
    td_udp = 1,
    td_tcp = 2,
    td_ipv6 = 4,
    td_ipv4 = 8,
    td_sport_vlan = 16,
    td_indicate_bottom
};
enum spe_td_ncm{
    td_ncm_success,
    td_ncm_part_discard,
    td_ncm_all_discard,
    td_ncm_bottom
};
enum spe_td_wrap{
    td_wrap_success,
    td_port_disable,
    td_abnormity,

#ifdef CONFIG_BALONG_RNDIS
    td_rndis_first_msg_type,
    td_rndis_msg_length_oversize,
    td_rndis_eth_over_pkt_len,
    td_rndis_eth_length_wrong,
#endif
#ifdef CONFIG_BALONG_NCM
    td_ncm_nth_flag_wrong,
    td_ncm_nth_length_wrong,
    td_ncm_block_length_wrong,
    td_ncm_ndp_flag_wrong,
    td_ncm_ndp_length_wrong,
    td_ncm_datagram_wrong,
    td_ncm_eth_length_wrong,
#endif
    td_wrap_bottom
};

enum spe_rd_result{
    rd_result_updata_only = 0,
    rd_result_discard,
    rd_result_success,
    rd_result_wrap_or_length_wrong,
    rd_result_bottom
};

enum spe_rd_reason{
    rd_reason_undiscard = 0,
    rd_reason_rd_point_null = 15,
    rd_reason_bottom
};

enum spe_rd_indicate{
    rd_udp = 1,
    rd_tcp = 2,
    rd_ipv6 = 4,
    rd_ipv4 = 8,
    rd_sport_vlan = 16,
    rd_indicate_bottom
};
enum spe_rd_wrap{
	rd_wrong_format,
	rd_push_en,
	rd_wrap_num_oversize,
	rd_wrap_length_oversize,
	rd_timeout,
	rd_port_dis,
	rd_wrap_bottom
};

struct spe_port_ops{
    int (*finish_td)(int portno, struct sk_buff *skb, unsigned int flags);
    int (*finish_rd)(int portno, int src_portno, struct sk_buff *skb, dma_addr_t dma, unsigned int flags);
    int (*finish_rd_bypass)(int portno, int src_portno, struct sk_buff *skb);
};

struct spe_port_attr{
    enum spe_enc_type enc_type;             /* encap type */
    enum spe_attach_brg_type attach_brg;    /* port attached bridge type */
    uint32_t padding_enable;
    uint32_t td_depth;
    uint32_t rd_depth;
    uint32_t rd_skb_size;
    uint32_t rd_skb_num;
    uint32_t ncm_max_size;
    uint32_t ncm_align_parameter;
    uint32_t stick_mode;
    struct spe_port_ops desc_ops;
    struct net_device *net;
    struct ether_addr ipf_src_mac;
    struct ether_addr ipf_dst_mac;
    int prio;                               /* port priority, if it is 0, use the dft value */
    void *port_priv;
};

struct spe_port_attr_alt{
    enum spe_enc_type enc_type;             /* encap type */
    struct spe_port_ops desc_ops;
};

enum spe_mac_type{
    spe_mac_dst,
    spe_mac_src,
    spe_mac_bottom
};


struct spe_nat {
	u32	ip;
	u16	port;
}__attribute__ ((packed));

struct spe_ip_fw_entry {
    struct spe_ip_fw_entry *next;
    struct spe_ip_fw_entry *prev;
    struct nf_conntrack_tuple tuple;    /* tuple to track the stream */
    u32 timestamp;                      /* timestamp used to expire the entry */
    u16 vid;                            /* vid in big endian */
    u16 portno;                         /* spe port no */
    struct  spe_nat     nat;            /* manip data for SNAT, DNAT */
    char action;                        /* nat action. 0:src nat 1:dst nat 2/3:not nat */
    char iptype:1;                      /* 0-ipv4 1-ipv6 */
    char is_static:1;                      /* 0-dynamic 1-static */
    char reserve1:5;
    char valid:1;                       /* 0-invalid 1-valid */
    struct ether_addr dhost;            /* dest mac in big endian */
    struct ether_addr shost;            /* src mac in big endian */
    u32 dead_timestamp;                 /* timestamp when entry set to invalid */

    /* the following is usr field */
    dma_addr_t dma;
    struct list_head list;
    struct net_device *dst_net;
};

/*spe_mac_entry read from sram*/
struct spe_mac_entry_in_sram {
	long long action:1;
	long long rd_port:4;
	long long bid:4;
	long long vid:12;
	long long port:4;
	long long timestamp:16;
	long long is_static:1;
	long long reserve:22;
	char mac[6];
	char mac_fil_vld:1;
	char mac_fw_vld:1;
};

struct spe_hook_ops{
    int (* is_enable)(void);

    /* ip forward and mac forward interface */
    int (* ipfw_entry_add)(struct neighbour *n, struct sk_buff *skb);
    void (* ipfw_entry_del)(struct nf_conn *ct);
    int (* macfw_entry_add)(const char *mac, uint16_t vid, u32 br_speno, u32 src_speno, u32 is_static);
    int (* macfw_entry_del)(char *mac, uint16_t vid, u32 br_speno, u32 src_speno);
    void (* set_ipf_mac)(enum spe_mac_type mac_type, struct ether_addr * mac_addr);
    void (* set_port_property)(struct net_device *net, struct spe_port_attr_alt *attr, int portno);
    //int (* modify_cpuport_ops)(struct spe_port_ops *cpuport_ops);

    /* port interface */
    int (* port_alloc)(struct spe_port_attr *attr);
    int (* port_free)(int portno);
    int (* port_ioctl)(int portno, int cmd, int param);
    int (* port_is_enable)(int portno);
    int (* port_enable)(int portno);
    int (* port_disable)(int portno);
    struct net_device * (* port_netdev)(int portno);

    /* update only interface */
    bool (* is_updateonly)(struct sk_buff *skb);

    dma_addr_t (* get_skb_dma)(struct sk_buff *skb);
    void (* set_skb_dma)(struct sk_buff *skb, dma_addr_t dma);
    int (* td_config)(int portno, struct sk_buff *skb, dma_addr_t dma, spe_l3_type_t l3_type, u32 push);
    int (* rd_config)(int portno, struct sk_buff *skb, dma_addr_t dma);
    int (*mode)(void);
};

extern struct spe_hook_ops spe_hook;
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__SPE_INTERFACE_H__*/
