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

#ifndef __SPE_SIM_PRIV_H
#define __SPE_SIM_PRIV_H

#include "spe_sim_debug.h"
#include "spe_sim.h"
#include "spe_sim_reg.h"
#include <linux/slab.h>
#include <linux/dma-mapping.h>


/* according the spe driver to set the macro */
#define SPE_SIM_DESC_ADDR_TYPE        spe_sim_mem_coherent
#define SPE_SIM_TD_ADDR_TYPE        spe_sim_mem_slub
#define SPE_SIM_RD_ADDR_TYPE        spe_sim_mem_slub
#define SPE_SIM_EVENT_ADDR_TYPE     spe_sim_mem_coherent
#define SPE_SIM_HASH_ADDR_TYPE      spe_sim_mem_coherent
#define SPE_SIM_DATA_FLUSH_CACHE    0
#define SPE_SIM_USE_PHY_ADDR        0
#define SPE_SIM_ENABLE_AGING        1

/* some default value */
#define SPE_SIM_MAX_PORTS       8
#define SPE_IRQ_NUM 159
#define SPE_SIM_MAX_PACK_LEN        2048
#define SPE_SIM_DFT_WARP_TIMEOUT    1
#define MAC_ADDR_LEN       6


/*return 0 while is NOT bridge */
#define SPE_SIM_IS_BR(port) ((port) >= SPE_SIM_MAX_PORTS)

typedef enum tag_spe_sim_mem_type {
    spe_sim_mem_slub            = 0,
    spe_sim_mem_coherent        = 1,
    spe_sim_mem_reserv
}spe_sim_mem_type_t;


/*
 *Structure of a 48-bit Ethernet address
 */
struct ether_addr {
	unsigned char octet[MAC_ADDR_LEN];
} ;

/* spe port status, pointer to port status register */
typedef struct tag_spe_sim_port_status_reg {
    unsigned int *port_status;
}spe_sim_port_status_reg_t;


/* spe port statistics */
typedef struct tag_spe_sim_port_stats {
    unsigned int stat_td_ok;

    unsigned int stat_rd_ok;
}spe_sim_port_stats_t;

typedef struct tag_spe_sim_ncm_dec_ctx {
    int index_pos;                      /* ncm dec position */
    int dg_len;                         /* ncm dec packet len */
    int ndp_len;                        /* ncm ndp len left */
    void* cur_pos;                      /* ncm cur buffer pos */
    int is_ntb32;                       /* ntb32 format */
}spe_sim_ncm_dec_ctx_t;

typedef struct tag_spe_sim_rndis_dec_ctx {
	unsigned int	last_packet_pos;
}spe_sim_rndis_dec_ctx_t;

/* spe td proc contex */
typedef struct tag_spe_sim_td_ctx {
    union {                             /* ncm/rndis dec ctx */
		spe_sim_ncm_dec_ctx_t ncm_ctx;
		spe_sim_rndis_dec_ctx_t rndis_ctx;
	};

    unsigned int td_num;                /* td descriptor number */
    char* base;                         /* td descriptor base address */
    int r_pos;                          /* td read ptr */
    int w_pos;                          /* td write ptr */
    //spinlock_t list_lock;
}spe_sim_td_ctx_t;

typedef struct dg_node {
    unsigned int max_len;               /* max lenght of NTH+NDP_PAD+NDP_HDR+PAD */
    unsigned int dg_cnt;                /* counter of datagram  */
    unsigned int last_dg_pos;           /* last datagram's position from nth header */
    unsigned int max_num;               /* max number of datagram */
}dg_node_t;

typedef struct tag_spe_sim_ncm_enc_ctx {
    unsigned int    sum_size;           /* ncm sum size */
    unsigned        pkt_len_waterline;  /* packet lenght waterline */
    void*           ncm_pos;            /* ncm_pos = dst_ptr + sum_size */
    int             is_ntb32;           /* ntb32 format */
    unsigned        fixed_in_len;       /* maxsize of ncm block */
    unsigned short* ndp_len_pos;       /* ndp's real length */
    struct dg_node  dg_ctx;             /* datagram context*/
}spe_sim_ncm_enc_ctx_t;


typedef struct tag_spe_sim_rndis_enc_ctx {
	unsigned int	dg_num_cnt;			/*rndis data gram number count*/
    unsigned int	sum_size;			/*rndis sum size*/
	unsigned int	last_packet_pos;	/*last rndis small packet positon, used by dst->prt*/
}spe_sim_rndis_enc_ctx_t;

/* spe rd proc contex */
typedef struct tag_spe_sim_rd_ctx {
    struct timer_list   timer;          /* assemble packet timeout */
    union {                             /* ncm/rndis enc ctx */
		spe_sim_ncm_enc_ctx_t ncm_ctx;
		spe_sim_rndis_enc_ctx_t rndis_ctx;
	};
    unsigned int rd_num;                /* rd descriptor number */
    char* base;   /* rd descriptor base address */
    int r_pos;                          /* rd read ptr */
    int w_pos;                          /* rd write ptr */
    int wrapping;                       /* wrap can stop */
    int update_only;
    int wrap_end;
    void* src;
}spe_sim_rd_ctx_t;

/* spe proc contex */
typedef enum tag_spe_sim_proc_stat {
    /* normal status */
    spe_sim_proc_idle            = 0,
    spe_sim_proc_td              = 1,
    spe_sim_proc_entry           = 2,
    spe_sim_proc_rd              = 3

}spe_sim_proc_stat_t;

typedef enum tag_spe_sim_proc_result {
    /* error number */
    spe_sim_proc_no_rd           = -1,

    /* normal status */
    spe_sim_proc_success         = 0

}spe_sim_proc_result_t;

/* spe simulator ip forward modify info */
typedef struct tag_spe_sim_ip_fwd_modify {
    /*
     modify mask flag marks the modified field:
     bit0: 1: modify src ip/port,   0: modify dst ip/port
     bit1: 1: modify dest mac,      0: do nothing
     bit2: 1: modify src mac,       0: do nothing
     bit3: 1: modify ip and port,   0: do nothing
    */
    unsigned int modify_mask;
    struct  ether_addr dest;	    /* destination eth addr	*/
    struct  ether_addr source;	    /* source ether addr */
    unsigned int ip;                /* ip address, ipv4 only */
    unsigned short port;            /* tcp port, ipv4 only */
    int l4_proto;
}spe_sim_ip_fwd_modify_t;

/* spe simulator port define */
typedef struct tag_spe_sim_port {
    unsigned int port_num;              /* spe port number */
    void* ctx;                          /* pointer to spe_sim_ctx */
    spe_sim_proc_stat_t proc_stat;      /* port process status */
    spe_sim_proc_result_t last_err;     /* record last error */
    int is_enable;                      /* port enable */
    int redo_td;                        /* flag: do the same td again */
    int last_packet;                    /* the same as push_en */
    char* src_ptr;                      /* src packet ptr */
    unsigned int src_len;               /* src packet len */
    char* dst_ptr;                      /* dst packet ptr */
    unsigned int dst_len;               /* dst packet len */
    int cur_is_ipv4;                    /* set by entry */
    int cur_l4_proto;                   /* set by entry */
    int unwrap_result;                  /* 0: unwrap done, 1: unwrap next */
    spe_sim_port_type port_type;        /* port type */
    spe_sim_enc_type_t enc_type;        /* packet enc type */
    spe_sim_td_ctx_t td_ctx;            /* td process ctx */
    spe_sim_rd_ctx_t rd_ctx;            /* rd process ctx */
    spinlock_t port_lock;               /* port spin lock */
    int tocken;
    int tocken_set;

    spe_sim_port_status_reg_t status;   /* port status in status reg */
    spe_sim_port_stats_t stat;

    spe_sim_ip_fwd_modify_t modify;

    spe_sim_port_debug_t debug_info;
    char port_mac[MAC_ADDR_LEN];
}spe_sim_port_t;

struct spe_mac_addr_heads
{
	char mac[4];
};

struct spe_sim_mac_fw_entry {
	char mac[MAC_ADDR_LEN];         /* mac address */
	unsigned short br_portno:4;                /* bridge number this entry belongs*/
	unsigned short reserve:12;
	unsigned int port_index;                 /* port index */
	unsigned int is_static;
};

/* spe simulator contecxt define */
typedef struct tag_spe_sim_ctx {
    spe_sim_port_t ports[SPE_SIM_MAX_PORTS];
    spe_sim_port_t* ports_en[SPE_SIM_MAX_PORTS];
    spe_sim_port_t* ports_idx[SPE_SIM_MAX_PORTS];
    int ports_en_num;
    unsigned int* reg_status;
    int wrap_timeout;
    spe_sim_port_t* cpu_port;
    spe_sim_port_t* ipf_port;
    int             is_ntb32;
    unsigned int    pack_fixed_in_len;
    unsigned int    pack_max_num;
    unsigned int    usr_field_num;
    char ipf_src_mac[MAC_ADDR_LEN];
    char ipf_dst_mac[MAC_ADDR_LEN];
	unsigned int    ncm_div;
	unsigned int    ncm_rem;
	unsigned int    ncm_max_size;

}spe_sim_ctx_t;

#define SPE_SIM_MODIFY_SRC      (0x1<<0)
#define SPE_SIM_MODIFY_DMAC     (0x1<<1)
#define SPE_SIM_MODIFY_SMAC     (0x1<<2)
#define SPE_SIM_MODIFY_IP_PORT  (0x1<<3)

/* may be problem in l2cache open */
static inline void spe_sim_flush_cache(void* virt, unsigned int size)
{
#if SPE_SIM_DATA_FLUSH_CACHE
    __dma_single_cpu_to_dev(virt, (size_t)size, DMA_TO_DEVICE);
#endif
}

static inline void spe_sim_invalid_cache(void* virt, unsigned int size)
{
#if SPE_SIM_DATA_FLUSH_CACHE
    __dma_single_dev_to_cpu(virt, (size_t)size, DMA_FROM_DEVICE);
#endif
}

static inline void* spe_sim_phy_to_virt(unsigned int phy, spe_sim_mem_type_t mem_type)
{
    void* virt;

#if SPE_SIM_USE_PHY_ADDR
    switch(mem_type) {
    case spe_sim_mem_slub:
        virt = phys_to_virt(phy);
        break;
    case spe_sim_mem_coherent:
        virt = dma_coherent_phy_to_virt((dma_addr_t)phy);
        break;
    default:
        return NULL;
    }
#else
    virt = (void*)phy;
#endif
    return virt;
}


/*
 * 1. spe simulator schedule process interfaces
 */
/* write register to trigger spe schedule */
void spe_sim_schedule_port(spe_sim_port_t* port);

/* read / add / delete a table entry */
int spe_sim_schedule_entry(spe_sim_ctx_t* ctx, spe_sim_entry_action_t action,
                           spe_sim_entry_type_t type, char* entry);

/* when enable one port call the function */
void spe_sim_rescan_enable_port(void);


/*
 * 2. spe simulator td process interfaces
 */
/* td process init */
void spe_sim_td_init(spe_sim_ctx_t* ctx);

/* start td process */
void spe_sim_td_process(spe_sim_port_t* port);

/* td complete, called by rd process */
void spe_sim_td_complete(spe_sim_port_t* port, int result, char* usr_field);

void spe_sim_recycle_all_td(spe_sim_port_t* port);

void spe_sim_recycle_all_rd(spe_sim_port_t* port);


/* whether there are work in the port */
int spe_sim_can_process_port(spe_sim_port_t* port);

int spe_sim_ncm_unwrap(spe_sim_port_t* port, char* buf, unsigned int len);

int spe_sim_rndis_unwrap(spe_sim_port_t* port, char* buf, unsigned int len);

/*
 * 3. spe simulator ip/mac entry process interfaces
 */
/* start mac/ip entry process */
void spe_sim_entry_process(spe_sim_port_t* port,
                           char* buffer, unsigned int size);

/* read / add / delete a table entry */
void spe_sim_entry_operate(spe_sim_ctx_t* ctx, spe_sim_entry_action_t action,
                           spe_sim_entry_type_t type, char* entry);

/* set result to reg, 0:success, others:fail */
void spe_sim_reg_set_entry_result(int result);

/* set hash param, width is bytes */
void sep_sim_entry_set_hash_params(unsigned int base, unsigned int rnd,
                    unsigned short zone, unsigned int width, unsigned int num);

/* call the function when set port regs */
void spe_sim_rescan_port_by_num(spe_sim_ctx_t* ctx, int port_num);

void spe_sim_set_mac_age_time(unsigned int age_time);

void spe_sim_set_ip_age_time(unsigned int age_time);



/*
 * 4. spe simulator rd process interfaces
 */
/* start rd process */
void spe_sim_rd_process(spe_sim_port_t* src, spe_sim_port_t* dst,
                        spe_sim_ip_fwd_modify_t* modify, int update_only);

int spe_sim_ncm_wrap(spe_sim_port_t* port, char* buf, unsigned int len);

int spe_sim_ncm_end_wrap(spe_sim_port_t* port);

int spe_sim_rndis_wrap(spe_sim_port_t* port, char* buf, unsigned int len);

void spe_sim_reg_rd_update(spe_sim_port_t* port);

int spe_sim_rndis_end_wrap(spe_sim_port_t* port);

/*
 * 5. spe simulator interrupt process interfaces
 */
/* write event buffer and generate an interrupt */
void spe_sim_start_intr(spe_sim_port_t* port, char event);

void spe_dec_current_event_num(unsigned int event_num);

void spe_sim_intr_set_event_timeout(unsigned int timeout);

void spe_sim_intr_init(void);

void spe_sim_intr_re_init(void);

void spe_sim_intr_get_event_ptr(unsigned int* rptr, unsigned int* wptr);




/*
 * 6. spe simulator reg process interfaces
 */
/* relate the status field to register */
unsigned int spe_sim_reg_reserved(spe_sim_ctx_t* ctx, int port_num);

void spe_sim_reg_set_status(spe_sim_ctx_t* ctx, int status);

void spe_sim_reg_set_td_rptr(spe_sim_port_t* port);

void spe_sim_reg_set_rd_rptr(spe_sim_port_t* port);

/* get event buffer info from register */
unsigned int spe_sim_reg_set_event_addr(unsigned buffer_addr);

unsigned int spe_sim_reg_get_event_size(spe_sim_ctx_t* ctx);

unsigned int spe_sim_reg_set_event_depth(unsigned int value);

unsigned int spe_sim_reg_get_event_en(char event);

/*return dump reg value(32bit)*/
unsigned int spe_sim_reg_get_entry_dump(spe_sim_ctx_t* ctx);

/*return 1=not under bridge,0=under bridge*/
int spe_sim_reg_if_port_under_bridge(spe_sim_port_t *port);

int spe_sim_get_port_info(unsigned int port_num, spe_sim_port_t** port);

void spe_sim_reg_set_cpu_port(struct tag_spe_sim_port* cpu_port);


/* */
void spe_sim_reg_init(spe_sim_ctx_t* ctx);
void spe_sim_rd_init(spe_sim_ctx_t* ctx);

void spe_sim_print_mac_addr(char * mac);
void spe_sim_print_port(unsigned int i);


#endif /* __SPE_SIM_PRIV_H */

