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

#ifndef __SPE_H__
#define __SPE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*__cplusplus*/

#include <linux/version.h>
#include <linux/neighbour.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/list.h>
#include <linux/rtnetlink.h>
#include <linux/route.h>
#include <linux/dmapool.h>
#include <linux/etherdevice.h>
#include "linux/module.h"

#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/ip6_route.h>
#include <net/route.h>

#include <linux/spe/spe_interface.h>

#include "spe_errcode.h"
#include "spe_io.h"
#include "spe_reg.h"

#define SPE_MODULE_NAME	"spe"

#define SPE_FLAG_ENABLE             BIT(0)
#define SPE_FLAG_BR_ISOLATION       BIT(1)

#define SPE_SKB_PUSH                BIT(0)
#define SPE_SKB_UPDATE_ONLY         BIT(1)

/* now, the clock in SPE time calc is 32KHz,
 * if asic switch to 19.2MHz, use 192MHZ macro
 */
#define SPE_192MHZ_TO_US(cnt)       ((cnt)*10 / 192) /* 19.2MHz -> us */
#define SPE_32KHZ_TO_US(cnt)        ((cnt)*305/10) /* 32KHz -> us */
#define SPE_US_TO_192MHZ(us)        ((us)*192/10)    /* us -> 19.2MHz */
/* be care, param: us, can't less than 32 */
#define SPE_US_TO_32KHZ(us)         ((us)*10/305)  /* us -> 32KHz */

#define SPE_US_TO_TIMERCNT(us)      SPE_US_TO_32KHZ(us)
#define SPE_TIMERCNT_TO_US(cnt)     SPE_32KHZ_TO_US(cnt)

/* SPE EVENT MACRO */
#define SPE_EVENT_BUFFERS_USED      (2)
#define SPE_EVENT_BUFFERS_MAX       (2)
#define SPE_EVENT_BUFFERS_SIZE	    PAGE_SIZE

/* event0+event1 max is 1024,
 * event0 num is : SPE_EVENT_BUFFER_DFT_NUM
 * event1 num is : SPE_EVENT_BUFFER_TOTAL_NUM - SPE_EVENT_BUFFER_DFT_NUM
 */
#define SPE_EVENT_BUFFER_TOTAL_NUM  (1024)

#if (SPE_EVENT_BUFFERS_USED == 1)
#define SPE_EVENT_BUFFER_DFT_NUM	    (1024)
#elif (SPE_EVENT_BUFFERS_USED == 2)
#define SPE_EVENT_BUFFER_DFT_NUM	    (512)
#else
#error "SPE_EVENT_BUFFERS_USED specify error"
#endif

#define SPE_EVENT_DISABLE   (0x3F)
#define SPE_EVENT_COMP_ONLY   (0x1B)
#define SPE_EVENT_EMPTY_FULL   (0x24)

#define SPE_EVENT_BUFFER_NUM_MASK   (0x7FF)
#define SPE_EVENT_BUFFER_NUM_BITS   (0xFFFF)
#define SPE_EVENT_BUFFER_NUM_BIT_CNT (16)
#define SPE_EVENT_STAT_MAX          (16)

#define SPE_DFT_PORT_PRIO           (2)
#define SPE_PORT_PRIO_MASK          (0xFF)

#define SPE_MIN_PKT_SIZE        (20)

#define SPE_GEVNTCOUNT_MASK	        0xffff

#define EVENT_RD_EMPTY		0x0
#define EVENT_RD_FULL		0x1
#define EVENT_TD_EMPTY		0x2
#define EVENT_TD_FULL		0x3
#define EVENT_RD_COMPLETE	0x4
#define EVENT_TD_COMPLETE	0x5

#define BUFFER_0_EVENT	0x0
#define BUFFER_1_EVENT	0x1

/* SPE PORT MACRO */
#define SPE_PORT_EVT_EN_RD          BIT(0)
#define SPE_PORT_EVT_EN_TD          BIT(1)
#define SPE_PORT_EVT_EN_DEFAULT     (SPE_PORT_EVT_EN_RD | SPE_PORT_EVT_EN_TD)
#define SPE_BR_PORT(x)              (((x)&0xf)>=SPE_PORT_NUM)

#define SPE_PORT_ENABLED		    BIT(0)

#define spe_is_ipv6(skb)            (skb->protocol == htons(ETH_P_IPV6))

#define SPE_TD_RESULT_SUCC(r)           (!((r) & 0x1))
#define SPE_TD_RESULT_UPDATE_ONLY(r)    (!((r) & 0x2))
#define SPE_TD_GET_TRANS_RESULT_IDX(r)  ((r) & 0x3)
#define SPE_TD_GET_DISC_REASON_IDX(r)   (((r) >> 2) & 0xf)
#define SPE_TD_GET_FW_PATH_IDX(r)       (((r) >> 6) & 0x7ff)
#define SPE_TD_GET_INDICATE_IDX(r)      (((r) >> 17) & 0x1f)
#define SPE_TD_GET_DISC_WRAP_IDX(r)     (((r) >> 22) & 0x3)
#define SPE_TD_GET_WRAP_RESULT_IDX(r)   (((r) >> 24) & 0xf)

#define SPE_RD_RESULT_SUCC(r)           (!((r) & 0x1))
#define SPE_RD_RESULT_UPDATE_ONLY(r)    (!((r) & 0x2))
#define SPE_RD_GET_TRANS_RESULT_IDX(r)  ((r) & 0x3)
#define SPE_RD_GET_DISC_REASON_IDX(r)   (((r) >> 2) & 0xf)
#define SPE_RD_GET_FW_PATH_IDX(r)       (((r) >> 6) & 0x7ff)
#define SPE_RD_GET_INDICATE_IDX(r)      (((r) >> 17) & 0x1f)
#define SPE_RD_GET_DISC_WRAP_IDX(r)     (((r) >> 22) & 0x3)
#define SPE_RD_GET_WRAP_RESULT_IDX(r)   (((r) >> 24) & 0xf)

#define SPE_PORT_UDP_LIM_MAX            (0xFFFFF)
#define SPE_PORT_BYTE_LIM_MAX           (0x1FFF807)
#define SPE_PORT_LIM_TIME_MAX            (0xFF)

/*l4 type*/
#define  SPE_TCP                        (6)  /* Transmission Control Protocol	  */
#define  SPE_UDP                        (17)  /* User Datagram Protocol 	  */

#ifdef CONFIG_BALONG_SPE_SIM
#define SPE_IP_FW_TIMEOUT           (10)        /* unit:jiffies */
#define SPE_MAC_FW_TIMEOUT          (100*HZ)    /* unit:jiffies */
#define SPE_INTR_TIMEOUT            (1000)      /* unit:us */
#define SPE_NCM_WRAP_TIMEOUT        (1)         /* unit:jiffies*/
#else
#define SPE_IP_FW_TIMEOUT           (SPE_US_TO_TIMERCNT(10*1000*1000)/1024) /* 10s, unit:32khz/(2^10) */
#define SPE_MAC_FW_TIMEOUT          (SPE_US_TO_TIMERCNT(100*1000*1000)/1024)/* 100s unit:32khz/(2^10) */
#define SPE_INTR_TIMEOUT            (SPE_US_TO_TIMERCNT(256))         /* 256us, unit:32khz */
#define SPE_NCM_WRAP_TIMEOUT        (SPE_US_TO_TIMERCNT(256))       /* 256us, unit:32khz */
#endif

#define SPE_WORK_DEF_DELAY          (2)         /* unit:tick */

struct spe_plat_data {
	void (*sysctrl_init)(void);
	void (*sysctrl_exit)(void);
};

union spe_event
{
    uint32_t raw;
    struct
    {
        uint8_t portno:4;
        uint8_t etype:4;
        uint8_t reserved0;
        uint16_t reserved1;
    }bits;
};

enum spe_desc_type{
    spe_desc_td = 0,
    spe_desc_rd,
    spe_desc_bottom
};

union spe_desc_queue_ptr{
    uint32_t raw;
    struct{
        uint16_t wptr;     /* write ptr, maintenance by sw */
        uint16_t rptr;     /* read ptr, maintenance by hw */
    }bits;
};

enum spe_enable_status{
    spe_status_disable = 0,
    spe_status_enable,
    spe_status_bottom
};

struct spe_event_buffer {
	void __iomem *buf;
	unsigned		length;
	unsigned int		lpos;
	//dma_addr_t		dma;
	struct spe  *spe;
};

struct spe_td_result_s {
	unsigned int result		:2;
	unsigned int reason		:4;
	unsigned int path		:11;
	unsigned int indicat	:5;
	unsigned int drop		:2;
	unsigned int error		:4;
	unsigned int rsv		:4;
};

struct spe_rd_result_s {
	unsigned int result		:2;
	unsigned int drop_rsn	:4;
	unsigned int path		:11;
	unsigned int indicat	:5;
	unsigned int done_rsn	:3;
	unsigned int rsv		:7;
};


/*TODO:split spe_port_ctrl to caller part and spe part */
struct spe_port_ctrl{
    uint32_t portno;
    HI_SPE_PORTX_PROPERTY_T property;
    uint32_t flags;
    struct net_device *net;
    struct spe_port_ops desc_ops;
    void *port_priv;
    spinlock_t lock;
    uint32_t stick_mode;

    /* td */
    uint32_t td_addr;
    uint32_t td_depth;
    uint32_t td_evt_gap;
    dma_addr_t td_dma;
    uint32_t td_busy;    /* filled by software last time */
    uint32_t td_free;    /* to be fill by software next time */
    spinlock_t td_lock;

    /* rd */
    uint32_t rd_addr;
    uint32_t rd_depth;
    uint32_t rd_evt_gap;
    dma_addr_t rd_dma;
    uint32_t rd_busy;    /* filled by software last time */
    uint32_t rd_free;    /* to be fill by software next time */
    spinlock_t rd_lock;

    /* rd skb pool */
    struct sk_buff_head	*rd_free_q;
    uint32_t rd_skb_num;
    uint32_t rd_skb_size;
    uint32_t rd_skb_used;
    uint32_t rd_skb_align;
	uint32_t rd_desc_filled;

    /* rate limit */
    uint32_t udp_limit_time;
    uint32_t udp_limit_cnt;
    uint32_t rate_limit_time;
    uint32_t rate_limit_byte;

	void (*wport_push)(void); // for wport, push ads to work
};

struct spe_port_stat{
    uint32_t netif_rx_drop;
    uint32_t td_result[td_result_bottom];
	uint32_t td_reason[td_reason_bottom];
	uint32_t td_fw[td_fw_bottom];
	uint32_t td_indicate[td_indicate_bottom];
	uint32_t td_ncm[td_ncm_bottom];
	uint32_t td_wrap[td_wrap_bottom];

    uint32_t rd_sport[SPE_PORT_NUM];
    uint32_t rd_result[rd_result_bottom];
	uint32_t rd_reason[rd_reason_bottom];
	uint32_t rd_fw[rd_fw_bottom];
	uint32_t rd_indicate[rd_indicate_bottom];
	uint32_t rd_wrap[rd_wrap_bottom];

    uint32_t rd_send_success;
    uint32_t rd_net_null;
    uint32_t rd_config;
    uint32_t rd_finished;
    uint32_t rd_finished_bytes;
    uint32_t rd_finsh_intr_complete;
    uint32_t rd_full;
    uint32_t rd_drop;
    uint32_t rd_skb_null;
    uint32_t rd_irq_en;
    uint32_t rd_upd_only;

    uint32_t td_config;
    uint32_t td_config_bytes;
    uint32_t td_port_disabled;
    uint32_t td_full;
    uint32_t td_push;
    uint32_t td_irq_en;
    uint32_t td_ts_en;
    uint32_t td_dma_null;
    uint32_t td_finsh_intr_complete;

    uint32_t cpu_kevent_ok;
    uint32_t cpu_kevent_cancel;
    uint32_t cpu_port_alloc_skb_fail;
    uint32_t cpu_port_rd_full;
    uint32_t cpu_dma_map_from_dev;

    uint32_t free_while_enabled;
    uint32_t free_busy;
	uint32_t disable_timeout;

    /* notice:the following can't be memset */
    uint32_t port_alloc_cnt;
    uint32_t port_free_cnt;
};

struct spe_port_ctx{
    struct spe_port_ctrl ctrl;
    struct spe_port_stat stat;
};

struct spe_mac_fw_ctx{
    struct list_head pending;
    struct list_head backups;
    struct kmem_cache *slab;
    spinlock_t lock;
};

struct spe_ip_fw_ctx{
    struct spe_ip_fw_entry *hbucket;
    struct spe_ip_fw_entry *hbucket_empty;
    dma_addr_t hbucket_dma;
    dma_addr_t hbucket_dma_empty;
    struct dma_pool *hslab;
    struct list_head free_list;
    spinlock_t free_lock;
    uint32_t free_cnt;
    uint32_t free_threhold;
    uint32_t deadtime;
    uint32_t hsize;
    uint32_t hwidth;
    uint32_t hzone;
    uint32_t hrand;
    spinlock_t lock;
};

struct spe_cpuport_ctx{
    uint32_t portno;
    uint32_t threshold;
};

struct spe_ipfport_ctx{
    uint32_t portno;
    struct ether_addr src_mac;
    struct ether_addr dst_mac;
};

struct spe_dev_stat{
    uint32_t evt_stat[SPE_EVENT_STAT_MAX];
    uint32_t evt_0_stat;
    uint32_t evt_1_stat;
    uint32_t evt_td;
    uint32_t evt_td_complt[SPE_PORT_NUM];
    uint32_t evt_td_full[SPE_PORT_NUM];
    uint32_t evt_td_empty[SPE_PORT_NUM];
    uint32_t evt_td_errport;
    uint32_t evt_rd;
    uint32_t evt_rd_complt[SPE_PORT_NUM];
    uint32_t evt_rd_full[SPE_PORT_NUM];
    uint32_t evt_rd_empty[SPE_PORT_NUM];
    uint32_t evt_rd_errport;
    uint32_t evt_buf_rd_err;
    uint32_t evt_unknown;
    //uint32_t wait_ready;
    uint32_t wait_idle;
    uint32_t ipfw_del;
    uint32_t ipfw_del_enter;
    uint32_t ipfw_del_leave;
    uint32_t ipfw_del_nothing_leave;
    uint32_t ipfw_add;
    uint32_t ipfw_add_enter;
    uint32_t ipfw_add_leave;
    uint32_t disable_timeout;

};

struct spe_stick_ctrl{
    uint8_t src_port;
    uint8_t src_status;
    uint8_t dst_port;
    uint8_t dst_status;
};

#ifdef CONFIG_BALONG_SPE_DFS
struct spe_dfs_freq{
	unsigned int freq;
	unsigned int freq_min;
	unsigned int freq_max;
};
#endif

struct spe{
    char name[32];
    char compile[64];
    uint32_t version;
    uint32_t flags;
    uint32_t mask_flags;
    uint32_t spe_mode;
    int event_buf_left;
    void __iomem *regs;
    unsigned int irq;
    uint32_t desc_usr_field;
    uint32_t ipfw_usr_field;
    uint16_t ipfw_timeout;
    uint16_t macfw_timeout;
    uint32_t intr_timeout;
    uint32_t ncmwrap_timeout;
    unsigned long portmap[2];
    struct delayed_work	work;
    uint32_t work_delay;
    unsigned long		work_portmap;
#define	WORK_RENEW_CPUPORT_RD       (0)
#define	WORK_RECYCLE_RD_STAYIN      (1)     /* attention: reserve 1~1+SPE_PORT_NUM */
    unsigned long		todo;
    struct resource		*res;
    struct spe_port_ctx ports[SPE_PORT_NUM];
    struct spe_cpuport_ctx cpuport;
    struct spe_ipfport_ctx ipfport;
    struct spe_event_buffer **ev_buffs;
    uint32_t num_event_buffers;
    struct spe_ip_fw_ctx ipfw;
    struct spe_mac_fw_ctx macfw;
    struct spe_stick_ctrl stick_ctrl;
    spinlock_t port_alloc_lock;
	uint32_t rd_success_byte;

#ifdef CONFIG_BALONG_SPE_DFS
	struct spe_dfs_freq freq_grade_array[10];
	struct timer_list spe_dfs_timer;
	uint32_t rd_success_byte_last;
	int cur_dfs_grade;
	int last_dfs_grade;
	uint32_t spe_fre_grade_max;
    uint32_t byte;
    uint32_t last_byte;
    uint32_t start_byte_limit;
	uint32_t total_byte;
	uint32_t spe_rd_count;
	uint32_t spe_rd_count_last;
	uint32_t rd_count;
	int spe_dfs_time_interval;
#endif	

    struct spe_dev_stat stat;
    struct dentry *root;    /*for debugfs*/

    /* the flowing elements after dev won't be zeroed. */
    struct device *dev;

    uint32_t min_pkt_len;
    uint32_t msg_level;
    uint32_t dbg_level;
    uint32_t evt_mask[SPE_EVENT_BUFFERS_MAX];
    uint32_t *reg_bak;
    uint32_t *entry_bak;
    uint32_t porten_bak;
    uint32_t not_idle;
    uint32_t suspend_count;
    uint32_t resume_count;

	spinlock_t pm_lock;
};

struct spe_skb_cb {
    dma_addr_t dma;
};

struct spe_registers_stash{
    unsigned int spe_userfied_ctrl;
    unsigned int spe_black_write;
    unsigned int spe_port_lmttime;
    unsigned int spe_eth_minlen;
    unsigned int spe_mode;
    unsigned int spe_glb_dmac_addr_l;
    unsigned int spe_glb_dmac_addr_h;
    unsigned int spe_glb_smac_addr_l;
    unsigned int spe_glb_smac_addr_h;
    unsigned int spe_rd_burstlen;
    unsigned int spe_l4_portnum_cfg[16];
    unsigned int spe_tdqx_baddr[8];
    unsigned int spe_tdqx_len[8];
    unsigned int spe_rdqx_baddr[8];
    unsigned int spe_rdqx_len[8];
    unsigned int spe_rdqx_ptr[8];   //need to open deben
    unsigned int spe_tdqx_pri[8];
    unsigned int spe_chx_portnum[8];
    unsigned int spe_portx_udp_lmtnum[8];
    unsigned int spe_portx_lmtbyte[8];
    unsigned int spe_portx_property[8];
    unsigned int spe_portx_mac_addr_l[8];
    unsigned int spe_protx_mac_addr_h[8];
    unsigned int spe_portx_ipv6_addr_1s[8];
    unsigned int spe_portx_ipv6_addr_2n[8];
    unsigned int spe_portx_ipv6_addr_3t[8];
    unsigned int spe_portx_ipv6_addr_4t[8];
    unsigned int spe_portx_ipv6_mask[8];
    unsigned int spe_portx_ipv4_addr[8];
    unsigned int spe_portx_ipv4_mask[8];
    unsigned int spe_hash_baddr;
    unsigned int spe_hash_zone;
    unsigned int spe_hash_band;
    unsigned int spe_hash_l3_proto;
    unsigned int spe_hash_width;
    unsigned int spe_hash_depth;
    unsigned int spe_aging_time;
    unsigned int spe_pack_max_time;
    unsigned int spe_pack_ctrl;
    unsigned int spe_pack_addr_ctrl;
    unsigned int spe_pack_pemain_len;
    unsigned int spe_unpack_ctrl;
    unsigned int spe_unpack_max_len;
    unsigned int spe_event_buf_len;
    unsigned int spe_int0_interval;
    unsigned int spe_int1_interval;
    unsigned int spe_event_buff0_mask;
    unsigned int spe_event_buff1_mask;
};

static inline void spe_skb_set_updateonly(struct sk_buff *skb)
{
    skb->update_only = 1;
}

extern struct spe spe_balong;
extern void *spe_port_priv(int portno);
extern struct net_device *spe_port_netdev(int portno);
extern int spe_core_init(struct spe *spe);
extern int spe_core_exit(struct spe *spe);
extern int spe_enable(struct spe *spe);
extern int spe_disable(struct spe *spe);
extern void spe_dbgen_enable(void);
extern void spe_dbgen_disable(void);
extern int spe_cpuport_init(struct spe *spe);
extern int spe_cpuport_exit(struct spe *spe);
extern uint32_t spe_timestamp_get(void);

extern void spe_hook_register(void);
extern void spe_hook_unregister(void);
extern int spe_is_enable(void);
extern int spe_ip_fw_add(struct neighbour *n, struct sk_buff *skb);
extern void spe_ip_fw_del(struct nf_conn *ct);
bool spe_skb_is_updateonly(struct sk_buff *skb);
extern int spe_port_exist(int portno);
int spe_port_ioctl(int portno, int cmd, int param);
int spe_port_alloc(struct spe_port_attr *attr);
int spe_port_free(int portno);
int spe_port_is_enable(int portno);
int spe_port_enable(int portno);
int spe_port_disable(int portno);
void spe_port_set_property(struct net_device *net, struct spe_port_attr_alt *attr, int portno);

void *spe_port_priv(int portno);
struct net_device *spe_port_netdev(int portno);

int spe_rd_config(int portno, struct sk_buff *skb, dma_addr_t dma);
int spe_td_config(int portno, struct sk_buff *skb, dma_addr_t dma, spe_l3_type_t l3_type, u32 push);

dma_addr_t spe_get_skb_dma(struct sk_buff *skb);
void spe_set_skb_dma(struct sk_buff *skb, dma_addr_t dma);
void spe_set_ipf_mac(enum spe_mac_type mac_type, struct ether_addr * mac_addr);
int spe_set_bypass_ops(struct spe_port_ops *ops);
int spe_td_complete(struct spe *spe, union spe_event *event);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__SPE_H__*/
