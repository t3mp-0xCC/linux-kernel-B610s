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
 * spe_sim_reg.h - spe register interface
 *
 */
#include "hi_spe_sim.h"

#ifndef __SPE_SIM_REG_H
#define __SPE_SIM_REG_H

#define SPE_EVENT_BUFFER_MAX_SIZE   1024
#define SPE_SIM_TD_RD_OFFSET_SIZE    0x20
#define SPE_SIM_PORT_CTRL_OFFSET_SIZE    0x40
#define SPE_SIM_PORT_SIZE           0x1000

typedef enum spe_event_type
{
    SPE_RD_EMPTY=0x0,
    SPE_RD_FULL=0x1,
    SPE_TD_EMPTY=0x2,
    SPE_TD_FULL=0x3,
    SPE_RD_COMPLETE=0x4,
    SPE_TD_COMPLETE=0x5,
}spe_sim_event_type_t;

/*
[2..0] 表项类型。
3'b000：添加MAC过滤表；
3'b001：添加MAC转发表；
3'b010：添加IPV4过滤表；
3'b011：添加IPV6过滤表；
3'b100：删除MAC过滤表；
3'b101：删除MAC转发表；
3'b110：删除IPV4过滤表；
3'b011：删除IPV6过滤表；
*/
typedef enum tag_spe_sim_entry_ctrl {
    spe_sim_add_mac_filter      = 0,
    spe_sim_add_mac_fwd         = 1,
    spe_sim_add_ipv4_filter     = 2,
    spe_sim_add_ipv6_filter     = 3,
    spe_sim_del_mac_filter      = 4,
    spe_sim_del_mac_fwd         = 5,
    spe_sim_del_ipv4_filter     = 6,
    spe_sim_del_ipv6_filter     = 7
}spe_sim_entry_ctrl_t;

typedef enum tag_spe_sim_entry_action {
    spe_sim_read_entry          = 1,
    spe_sim_add_entry           = 2,
    spe_sim_del_entry           = 3
}spe_sim_entry_action_t;

typedef enum tag_spe_sim_entry_type {
    spe_sim_entry_mac_filter    = 0,
    spe_sim_entry_ip_filter     = 1,
    spe_sim_entry_mac_forward   = 2,
    spe_sim_entry_ip_forward    = 3
}spe_sim_entry_type_t;

typedef enum tag_spe_sim_enc_type {
    spe_sim_enc_none            = 0,
	spe_sim_enc_ncm16           = 1,
    spe_sim_enc_ncm             = 2,
    spe_sim_enc_rndis           = 3,
    spe_sim_enc_wifi            = 4,
    spe_sim_enc_ipf             = 5,
    spe_sim_enc_cpu             = 6
}spe_sim_enc_type_t;

typedef enum tag_spe_sim_port_type {
    spe_sim_port_norm           = 0,
    spe_sim_port_in_br          = 1,
    spe_sim_port_mix            = 2,
    spe_sim_port_reserv         = 3
}spe_sim_port_type;


/*REGISTERS OF PORTS*/
/* spe_sim_reg_get_port_num 15*/
#define SPE_SIM_PORT_NUM(n)             ((HI_SPE_CHX_PORTNUM_0_OFFSET + (n * SPE_SIM_TD_RD_OFFSET_SIZE)) & MASK_OFFSET)/*old HI_SPE_TDQ_CH_NUM_0_OFFSET */
/* spe_sim_reg_get_property  14*/
#define SPE_SIM_PORT_DEF(n)             ((HI_SPE_PORTX_PROPERTY_0_OFFSET + (n * SPE_SIM_PORT_CTRL_OFFSET_SIZE)) & MASK_OFFSET)// old HI_SPE_CH_PROPERTY_0_OFFSET
/* spe_sim_reg_get_port_mac 16*/
#define SPE_SIM_PORT_MAC_ADR_H(n)       ((HI_SPE_PORTX_MAC_ADDR_H_0_OFFSET + (n * SPE_SIM_PORT_CTRL_OFFSET_SIZE)) & MASK_OFFSET)//HI_SPE_CH_MAC_ADDR_H_0_OFFSET
/* spe_sim_reg_get_port_mac 17*/
#define SPE_SIM_PORT_MAC_ADR_L(n)       ((HI_SPE_PORTX_MAC_ADDR_L_0_OFFSET + (n * SPE_SIM_PORT_CTRL_OFFSET_SIZE)) & MASK_OFFSET)//old HI_SPE_CH_MAC_ADDR_L_0_OFFSET

/* spe_sim_reg_get_td_addr */
#define SPE_SIM_TD_ADDR(n)              ((HI_SPE_TDQX_BADDR_0_OFFSET + (n * SPE_SIM_TD_RD_OFFSET_SIZE)) & MASK_OFFSET)//HI_SPE_TDQ_BADDR_0_OFFSET
/* spe_sim_reg_get_td_size */
#define SPE_SIM_TD_DEPT(n)              ((HI_SPE_TDQX_LEN_0_OFFSET + (n * SPE_SIM_TD_RD_OFFSET_SIZE)) & MASK_OFFSET)//HI_SPE_TDQ_LEN_0_OFFSET
/* spe_sim_reg_get_td_wptr / spe_sim_reg_set_td_rptr */
#define SPE_SIM_TD_WR_PTR(n)            ((HI_SPE_TDQX_PTR_0_OFFSET + (n * SPE_SIM_TD_RD_OFFSET_SIZE)) & MASK_OFFSET)//HI_SPE_TDQ_PTR_0_OFFSET
/* spe_sim_reg_get_pri */
#define SPE_SIM_TD_PRI(n)               ((HI_SPE_TDQX_PRI_0_OFFSET + (n * SPE_SIM_TD_RD_OFFSET_SIZE)) & MASK_OFFSET)//HI_SPE_TDQ_PRI_0_OFFSET

/* spe_sim_reg_get_rd_addr */
#define SPE_SIM_RD_ADDR(n)              ((HI_SPE_RDQX_BADDR_0_OFFSET + (n * SPE_SIM_TD_RD_OFFSET_SIZE)) & MASK_OFFSET)//HI_SPE_RDQ_BADDR_0_OFFSET
/* spe_sim_reg_get_rd_size */
#define SPE_SIM_RD_DEPT(n)              ((HI_SPE_RDQX_LEN_0_OFFSET + (n * SPE_SIM_TD_RD_OFFSET_SIZE)) & MASK_OFFSET)//HI_SPE_RDQ_LEN_0_OFFSET
/* spe_sim_reg_get_rd_wptr / spe_sim_reg_set_rd_rptr */
#define SPE_SIM_RD_WR_PTR(n)            ((HI_SPE_RDQX_PTR_0_OFFSET + (n * SPE_SIM_TD_RD_OFFSET_SIZE)) & MASK_OFFSET)//HI_SPE_RDQ_PTR_0_OFFSET

/* ncm/rndis config reg */
#define SPE_SIM_NCM_TMOUT               ((HI_SPE_PACK_MAX_TIME_OFFSET) & MASK_OFFSET)
/* spe_sim_reg_get_ncm_param */
#define SPE_SIM_NCM_SP_CTR              ((HI_SPE_PACK_CTRL_OFFSET) & MASK_OFFSET)
/*spe_sim_reg_get_ncm_align_parametor*/
#define SPE_PACK_ADDR_CTRL              ((HI_SPE_PACK_ADDR_CTRL_OFFSET) & MASK_OFFSET)
/*spe_sim_reg_get_ncm_max_size*/
#define SPE_UNPACK_CTRL                 ((HI_SPE_UNPACK_CTRL_OFFSET) & MASK_OFFSET)

#define SPE_EVENT_BUFF0_PTR             ((HI_SPE_EVENT_BUFF0_PTR_OFFSET) & MASK_OFFSET)



/*REGISTERS RELATED WITH ENTRIES*/
/* table operation */
#define SPE_SIM_ENTRY_OPRT              ((HI_SPE_TAB_CTRL_OFFSET) & MASK_OFFSET)
/* spe_sim_reg_set_entry_result */
#define SPE_SIM_ENTRY_RESULT            ((HI_SPE_TAB_ACT_RESULT_OFFSET) & MASK_OFFSET)


/* entry content used for: mac filter/mac fwd/ip filter */
#define SPE_SIM_ENTRY_TAB_CONTENT       ((HI_SPE_TAB_CONTENT_0_OFFSET) & MASK_OFFSET)//HI_SPE_TAB_1ST_WORD_OFFSET

/* hash param */
/* spe_sim_reg_get_hash_param */
#define SPE_SIM_HASH_BASE_ADDR          ((HI_SPE_HASH_BADDR_OFFSET) & MASK_OFFSET)
/* spe_sim_reg_get_hash_param */
#define SPE_SIM_HASH_ZONE               ((HI_SPE_HASH_ZONE_OFFSET) & MASK_OFFSET)
/* spe_sim_reg_get_hash_param */
#define SPE_SIM_HASH_RAND               ((HI_SPE_HASH_RAND_OFFSET) & MASK_OFFSET)
/* spe_sim_reg_get_hash_param */
#define SPE_SIM_HASH_SIZE               ((HI_SPE_HASH_WIDTH_OFFSET) & MASK_OFFSET)
/* spe_sim_reg_get_hash_param */
#define SPE_SIM_HASH_DEPTH               ((HI_SPE_HASH_DEPTH_OFFSET) & MASK_OFFSET)

/* spe_sim_reg_get_mac\ip_age_time */
#define SPE_SIM_AG_TMOUT            ((HI_SPE_AGING_TIME_OFFSET) & MASK_OFFSET)

/* spe_sim_reg_set_time */
#define SPE_SIM_REG_TIMER               ((HI_SPE_TIME_OFFSET) & MASK_OFFSET)

/* GLOBAL REGISTERS*/
/* spe_sim_reg_get_globle_ctl */
#define SPE_SIM_GLB_CFG_REG             ((HI_SPE_USRFIELD_CTRL_OFFSET) & MASK_OFFSET)//HI_SPE_GLB_CTL_OFFSET
/* spe_sim_reg_set_status */
#define SPE_SIM_GLB_STAT                ((HI_SPE_IDLE_OFFSET) & MASK_OFFSET)
/* spe_sim_reg_global_enable */
#define SPE_SIM_GLB_ENABLE              ((HI_SPE_EN_OFFSET) & MASK_OFFSET)

/* REGISTERS RELATED WITH ENVENT BUFFER  */
/* spe_sim_reg_set_event_addr */
#define SPE_SIM_EVENT_ADDR              ((HI_SPE_EVENT_BUFF0_ADDR_OFFSET) & MASK_OFFSET) //HI_SPE_EVENT_BUF_BADDR_OFFSET

#define SPE_SIM_EVENT_PTR              ((HI_SPE_EVENT_BUFF0_PTR_OFFSET) & MASK_OFFSET) 
/* spe_sim_reg_get_event_en / spe_sim_reg_get_event_size */
#define SPE_SIM_EVENT_BUF_LEN          ((HI_SPE_EVENT_BUFF_LEN_OFFSET) & MASK_OFFSET)//HI_SPE_EVENT_BUF_CTRL_OFFSET
/* spe_sim_reg_set_event_depth / spe_sim_reg_get_event_cnt */
#define SPE_SIM_EVENT_COUNT             ((HI_SPE_EVENTBUF0_RPT_CNT_OFFSET) & MASK_OFFSET)//number of event need to be processed

#define SPE_SIM_EVENT_PROCESSED             ((HI_SPE_EVENTBUF0_PRO_CNT_OFFSET) & MASK_OFFSET)//number of event software processed

#define SPE_SIM_EVENT_MASK             ((HI_SPE_EVENT_BUFF0_MASK_OFFSET) & MASK_OFFSET)//number of event software processed

/* spe_sim_reg_get_intr_timeout */
#define SPE_SIM_INT_MDRT_TMOUT          ((HI_SPE_INT0_INTERVAL_OFFSET) & MASK_OFFSET)//HI_SPE_INT_INTERVAL_OFFSET

#define HI_SPE_SIM_GLB_SMAC_ADDR_L_OFFSET ((HI_SPE_GLB_SMAC_ADDR_L_OFFSET) & MASK_OFFSET)
#define HI_SPE_SIM_GLB_SMAC_ADDR_H_OFFSET ((HI_SPE_GLB_SMAC_ADDR_H_OFFSET) & MASK_OFFSET)
#define HI_SPE_SIM_GLB_DMAC_ADDR_L_OFFSET ((HI_SPE_GLB_DMAC_ADDR_L_OFFSET) & MASK_OFFSET)
#define HI_SPE_SIM_GLB_DMAC_ADDR_H_OFFSET ((HI_SPE_GLB_DMAC_ADDR_H_OFFSET) & MASK_OFFSET)



/* DEFINITION OF TRANSACTION DESCRIPTOR */
typedef struct td_attribute{
    unsigned int eth_type:16;               /* eth_type, used for IPF port type */
    unsigned int push_en:1;
    unsigned int irq_enable:1;
    unsigned int timstamp_enable:1;
    unsigned int reserv0:1;
    unsigned int reserv1:12;
}td_attr_t;

typedef struct transaction_descriptor{
    unsigned int buf_addr_low;
    unsigned int skb_addr;
    struct td_attribute attribute;
    unsigned int reserv0:16;
    unsigned int pkt_len:16;
    unsigned int result;
    //unsigned int vlan_id:12;
    //unsigned int port_num:4;

    //unsigned int timestamp_usr_field1;
    //unsigned int timestamp_usr_field2;
}tran_desc_t;

/* DEFINITION OF RECEIVED DESCRIPTOR */
typedef struct rd_attribute{
    unsigned int source_port_num:4;
    unsigned int reserv0:12;

    /* attri */
    unsigned int irq_enable:1;
    unsigned int reserv1:3;

    unsigned int reserv2:8;

    /* pkt_type */
    unsigned int update_only:1;
    unsigned int reserv3:1;

    unsigned int reserv4:2;
}rd_attr_t;

typedef struct received_descriptor{
    unsigned int buf_addr_low;
    //unsigned int buf_addr_high;
    unsigned int skb_addr;
    struct rd_attribute attribute;
    unsigned int vlan_id:12;
    unsigned int dport_num:4;
    unsigned int pkt_len:16;
    unsigned int result;
    //unsigned int timestamp_usr_field1;
    //unsigned int timestamp_usr_field2;
}reveived_desc_t;

#if 0

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
    unsigned int flow_lable;
    unsigned int src_addr[4];
    unsigned int src_addr_mask[4];
    unsigned int dst_addr[4];
    unsigned int dst_addr_mask[4];
}ipv6_ctx_t;

typedef struct event_buffer{
    unsigned int port_number:4;
    unsigned int reserved1:12;
    unsigned int td_completed:1;
    unsigned int rd_completed:1;
    unsigned int reserved2:14;
}event_buf_t;

typedef struct ip_filter_entry {
    unsigned int filter_mask;
    unsigned int src_port_low:16;
    unsigned int src_port_high:16;
    unsigned int dst_port_low:16;
    unsigned int dst_port_high:16;
    unsigned int reserved:15;
    unsigned int iptype:1;
    union {
    struct ipv4_contex    spe_ipv4;
    struct ipv6_contex    spe_ipv6;
    };
}ipfilter_ent_t;

struct spe_ip_entry {
    struct  spe_ip_entry *next;         /*链表指针*/
    struct  spe_conn_tuple  tuple;      /* Tuple to track the stream */
    char    action;
    char    type;                       /*1:本entry是ipv4, 0:本entry是ipv4*/
    char    reserved1[1];                /*补充tuple4字节对齐的空洞*/
    uint32  age;                        /* timestamp used to expire the entry */
    uint32  port_index;                  /*port index registered to spe*/

    struct  spe_nat     nat;            /* Manip data for SNAT, DNAT */
    char    reserved2[2];                /*补充tuple 4字节对齐的空洞*/

    struct  ether_addr dhost;           /* Destination MAC address */
    char    reserved3[2];                /*ether_addr 4字节对齐的空洞*/

    char    ether_addr shost;           /* Source MAC address */
    char    reserved4[2];                /*ether_addr 4字节对齐的空洞*/

    long    dead_timestamp;             /*entry被设置为无效的时间戳*/
};
#endif

#endif /* __SPE_SIM_REG_H */

