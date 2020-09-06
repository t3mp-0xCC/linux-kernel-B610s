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

/*遗留问题:dst port的获取cpu port number的获取*/
#include <linux/jiffies.h>
#include <linux/etherdevice.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/jhash.h>
#include <net/ipv6.h>
#include <net/ip.h>
#include <linux/netfilter/nf_conntrack_tuple_common.h>
#include "spe_sim.h"
#include "spe_sim_priv.h"

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

#if 0
struct spe_mac_fw_entry {
    u32 mac_hi;                     /* mac in big endian */
    u32 mac_lo;                     /* mac in big endian */
    u16 vid:12;                     /* vlan id in big endian */
    u16 port_br_id:4;               /* br id this entry belongs to */
    u16 port_no:4;                  /* port no */
    u16 reserve:12;
};
#endif


union spe_sim_nf_inet_addr {
	__u32		all[4];
	__be32		ip;
	__be32		ip6[4];
	struct in_addr	in;
	struct in6_addr	in6;
};

/* The manipulable part of the tuple. */
struct spe_sim_nf_conntrack_man {
	union spe_sim_nf_inet_addr u3;
	union nf_conntrack_man_proto u;
	/* Layer 3 protocol */
	u_int16_t l3num;
};

/* This contains the information to distinguish a connection. */
typedef struct spe_sim_nf_conntrack_tuple {
	struct spe_sim_nf_conntrack_man src;

	/* These are the parts of the tuple which are fixed. */
	struct {
		union spe_sim_nf_inet_addr u3;
		union {
			/* Add other protocols here. */
			__be16 all;

			struct {
				__be16 port;
			} tcp;
			struct {
				__be16 port;
			} udp;
			struct {
				u_int8_t type, code;
			} icmp;
			struct {
				__be16 port;
			} dccp;
			struct {
				__be16 port;
			} sctp;
			struct {
				__be16 key;
			} gre;
		} u;

		/* The protocol. */
		u_int8_t protonum;

		/* The direction (for tuplehash) */
		u_int8_t dir;
	} dst;
} spe_sim_conn_tuple_t;


/*
l3num: PF_INET6 PF_INET
*protonum = iph->protocol
ipv6_get_l4proto
*/
//typedef struct nf_conntrack_tuple spe_sim_conn_tuple_t;


struct spe_sim_nat {
	unsigned int ip;
	unsigned short port;
}__attribute__ ((packed));

#define SPE_SIM_IP_ENTRY_IPV4       (0x0)

struct spe_sim_ip_fw_entry {
    struct list_head  list;         /* list ptr */
    spe_sim_conn_tuple_t tuple;     /* tuple to track the stream */
    unsigned int age;               /* timestamp used to expire the entry */

    unsigned short vid;             /* spe vid */
    unsigned short port_index;      /* port index registered to spe */

    struct spe_sim_nat nat;         /* manip data for SNAT, DNAT */
    char action;                    /* nat action. 0:src nat 1:dst nat 2:not nat */

    char iptype:1;
    char is_static:1;
    char reserv:5;
    char valid:1;
    struct  ether_addr dhost;       /* Destination MAC address */
    struct  ether_addr shost;       /* Source MAC address */
    long    dead_timestamp;         /* timestamp when entry set to invalid */
};


/*mac entry mangement list*/
struct spe_mac_fw_entry_list{
	struct spe_sim_mac_fw_entry entry;
	struct list_head list;
	unsigned long aging_timer;
};

struct spe_sim_mac_forwarding{
	struct tag_spe_sim_port *cpu_port;
	struct spe_mac_fw_entry_list head;
	spinlock_t spe_mac_fw_list_lock;
	unsigned long aging_time;

	unsigned int stat_add_entry;
	unsigned int stat_del_entry;
};

#define SPE_SIM_GET_HASH_HEAD(hash) \
    (struct spe_sim_ip_fw_entry*)(ip_fw_ctx.entry_table + (hash) * ip_fw_ctx.hash_width)

#define GET_NEXT_HASH_NODE(cur) \
    (struct spe_sim_ip_fw_entry*)\
        (((cur)->list.next == NULL) ? NULL : \
            (list_entry(spe_sim_phy_to_virt((unsigned int)(cur)->list.next,\
            SPE_SIM_HASH_ADDR_TYPE), typeof(*cur), list)))


struct spe_sim_ip_forwarding{
    unsigned int hash_rnd;              /* hash random value set by reg */
    unsigned int hash_num;              /* hash header num */
    unsigned int hash_width;            /* each hash node width */
    u16 zone;                           /* zone, set by reg */
    spe_sim_conn_tuple_t cur_tuple;     /* tuple info from cur packet */
	unsigned long aging_time;
    char* entry_table;
};

static struct spe_sim_mac_forwarding mac_fw_ctx;
static struct spe_sim_ip_forwarding ip_fw_ctx;

/* problem: */
/* phy address / td entry usr field */

void spe_sim_reg_set_cpu_port(struct tag_spe_sim_port* cpu_port)
{
    mac_fw_ctx.cpu_port = cpu_port;
}

void spe_sim_mac_entry_process_init(void)
{
	INIT_LIST_HEAD(&mac_fw_ctx.head.list);
	spin_lock_init(&mac_fw_ctx.spe_mac_fw_list_lock);
}

void spe_sim_set_mac_age_time(unsigned int age_time)
{
    mac_fw_ctx.aging_time = age_time;
}

void spe_sim_set_ip_age_time(unsigned int age_time)
{
    ip_fw_ctx.aging_time = age_time;
}

/* mac filter, ret port: redirect port, ret NULL: go ahead, ret -1: drop */
static inline spe_sim_port_t* spe_sim_mac_filter(spe_sim_port_t* port,
                                   char* buffer, unsigned int size)
{
    return NULL;
}
static void spe_sim_ip_entry_process(spe_sim_port_t* src_port_info,
	char* buffer, unsigned int size);
static inline spe_sim_port_t* spe_sim_ipfwd_get_dst(spe_sim_port_t* port,
	struct spe_sim_ip_fw_entry* ip_entry);

/* ip filter, ret 0: go ahead, ret -1: drop */
static inline int spe_sim_ip_filter(spe_sim_port_t* port,
                                   char* buffer, unsigned int size)
{
    return 0;
}
/*
int spe_sim_dump_mac_fw_entry(void)
{
	struct spe_mac_fw_entry_list *f;
	struct list_head *mac_fw_list_head = &mac_fw_ctx.head.list;
	void *dump_addr = (void *)spe_sim_reg_get_entry_dump(NULL);
	if(NULL == dump_addr){
		SPE_SIM_ERR("mac dump_addr NULL!\n");
		return -ENXIO;
	}
	list_for_each_entry(f, mac_fw_list_head, list){
		memcpy(dump_addr, &f->entry, sizeof(struct spe_sim_mac_fw_entry));
		dump_addr += sizeof(struct spe_sim_mac_fw_entry);
	}
	return 0;

}*/
void spe_sim_print_mac_addr(char * mac)
{
	int i;
	for(i=0; i<6; i++){
		SPE_SIM_INFO("  %2x  ",mac[i]);
	}
	SPE_SIM_INFO(" \n");

}

/*mac entry add function*/
static int spe_sim_add_mac_fw_entry(struct spe_sim_mac_fw_entry *current_entry)
{
	struct spe_mac_fw_entry_list *new_mac_entry;
	int i;

	mac_fw_ctx.stat_add_entry++;
	new_mac_entry = kmalloc(sizeof(struct spe_mac_fw_entry_list), GFP_ATOMIC);
	SPE_SIM_INFO("new_entry_addr is 0x%x \n", (unsigned int)new_mac_entry);

	if(NULL == new_mac_entry)
	{
		SPE_SIM_ERR("Fail to malloc new mac entry.\n");
		return -ENXIO;
	}
	new_mac_entry->entry.br_portno = current_entry->br_portno;
	new_mac_entry->entry.port_index = current_entry->port_index;
	new_mac_entry->aging_timer = jiffies;
	for(i=0; i<MAC_ADDR_LEN; i++)
	{
		new_mac_entry->entry.mac[i] = current_entry->mac[i];
	}
	spin_lock(&mac_fw_ctx.spe_mac_fw_list_lock);
	list_add(&new_mac_entry->list, &mac_fw_ctx.head.list);
	spin_unlock(&mac_fw_ctx.spe_mac_fw_list_lock);
	spe_sim_reg_set_entry_result(0);
	return 0;
}


/*mac entry delete function*/

static int spe_sim_del_mac_fw_entry(struct spe_sim_mac_fw_entry *current_entry)
{
	struct spe_mac_fw_entry_list *f;
	struct list_head *list_head = &mac_fw_ctx.head.list;
	//unsigned int i;

    mac_fw_ctx.stat_del_entry++;
	spin_lock(&mac_fw_ctx.spe_mac_fw_list_lock);
	list_for_each_entry(f, list_head, list){
		if(f->entry.br_portno != current_entry->br_portno){
			continue;
		}
		if(f->entry.port_index != current_entry->port_index){
			continue;
		}
		/*equal return 0;goto del process*/
		if(compare_ether_addr(f->entry.mac, current_entry->mac)){
			continue;
		}
		list_del(&f->list);
		spin_unlock(&mac_fw_ctx.spe_mac_fw_list_lock);
		SPE_SIM_DBG("del_entry_addr is 0x%x \n", (unsigned int)f);

		kfree(f);
		spe_sim_reg_set_entry_result(0);
		return 0;
	}
	spin_unlock(&mac_fw_ctx.spe_mac_fw_list_lock);
	spe_sim_reg_set_entry_result(-ENODATA);
	return -ENODATA;
}


/*mac entry aging function*/
static inline int spe_sim_age_mac_fw_entry(struct spe_mac_fw_entry_list *current_entry)
{
     /* static entry can't timeout */
    if (current_entry->entry.is_static) {
        return 0;
    }

	if(time_before(jiffies,current_entry->aging_timer + 60*HZ)){
		return 0;
	}
    return 1;
}

static inline int spe_sim_age_proc_end(spe_sim_port_t* port, unsigned long* age)
{
    if (port->last_err < 0) {
        port->last_err = 0;
    }
    else {
        *age = jiffies;
    }
    return 0;
}

/*mac entry traversal function, check weather a mac entry excise or not, if there is a mac entry, return port num*/
static int spe_sim_get_mac_fw_entry(char *mac_addr, struct spe_mac_fw_entry_list **current_entry)
{
	struct spe_mac_fw_entry_list *f;
	struct list_head *mac_fw_list_head = &mac_fw_ctx.head.list;
	/*Internal function, noneed to check input*/
	spin_lock(&mac_fw_ctx.spe_mac_fw_list_lock);//// del ?
	list_for_each_entry(f, mac_fw_list_head, list){
		/*if unequal, return 1, continue search*/
		if(compare_ether_addr(f->entry.mac, mac_addr)){
			continue;
		}
		spin_unlock(&mac_fw_ctx.spe_mac_fw_list_lock);
		*current_entry = f;
		return 0;
	}

	spin_unlock(&mac_fw_ctx.spe_mac_fw_list_lock);
	return -ENODATA;
}

/* need buffer/size param ? */
static int spe_sim_send_pkt_to_cpu(spe_sim_port_t *src_port_info, char *buffer, unsigned int size, int update_only)
{
	src_port_info->modify.modify_mask = 0;
	src_port_info->src_len = size;
	src_port_info->src_ptr = buffer;
	SPE_SIM_TRACE("Entry:Enter spe_sim_send_pkt_to_cpu \n");

	if(NULL == mac_fw_ctx.cpu_port){
		SPE_SIM_ERR("Entry:Port config error!Can not find pc port \n");
	}
	src_port_info->debug_info.stat_to_cpu_proc++;
	spe_sim_rd_process(src_port_info, mac_fw_ctx.cpu_port, &src_port_info->modify, update_only);
	return 0;
}

static int spe_sim_mac_entry_process(spe_sim_port_t *src_port_info, char *buffer, unsigned int size)
{
	char *dst_mac_addr;
	char *src_mac_addr;
	struct spe_mac_fw_entry_list 	*current_src_entry = NULL;
	struct spe_mac_fw_entry_list 	*current_dst_entry = NULL;
	spe_sim_port_t *dst_port_info;
	int i;
	SPE_SIM_TRACE("mac fw:Enter \n");

	if((NULL == src_port_info)||(NULL == buffer)){
		SPE_SIM_ERR("mac fw:buffer & src_port_info NULL! \n");
		return -ENXIO;
	}

	SPE_SIM_TRACE("portno: %d\n", src_port_info->port_num);
	src_port_info->debug_info.stat_mac_entry_proc++;

	/*get dst/src mac addr*/
	dst_mac_addr = buffer;
	src_mac_addr = buffer + MAC_ADDR_LEN;

	if(!spe_sim_reg_if_port_under_bridge(src_port_info)){/*check if the src mac is added to bridge 0=not under bridge,1=under bridge */

		/*src mac not under bridge, no need to mac fowarding*/
		SPE_SIM_TRACE("mac fw:Current dst does not under bridge, port = %u. \n",src_port_info->port_num);
		if(!compare_ether_addr(dst_mac_addr, src_port_info->port_mac)){/*check if the dst mac is the mac of current src port*/
			SPE_SIM_TRACE("mac fw:Send pkt to ip fw, dst mac == src mac . \n");
			spe_sim_ip_entry_process(src_port_info, buffer, size);/*sent pkt to spe ip module*/
			return 0;
		}
		/*sent pkt to cpu port*/
		SPE_SIM_TRACE("mac fw:Send pkt cpu, dst mac != src mac . \n");
		return spe_sim_send_pkt_to_cpu(src_port_info, buffer, size, 0);
	}
	else
	{

//		print_hex_dump(KERN_ERR, "mac fw:data", DUMP_PREFIX_ADDRESS,\
//                       16, 1, buffer, size, true);

		SPE_SIM_INFO("mac fw: dst mac is  ");
		spe_sim_print_mac_addr(dst_mac_addr);
		SPE_SIM_INFO("mac fw: src mac is  ");
		spe_sim_print_mac_addr(src_mac_addr);

		if(spe_sim_get_mac_fw_entry(dst_mac_addr, &current_dst_entry)){/*fail in get dst mac*/

			/*sent pkt to cpu port*/
			SPE_SIM_TRACE("mac fw:Send pkt to cpu, dst mac entry not found. \n");
			return spe_sim_send_pkt_to_cpu(src_port_info, buffer, size, 0);
		}
		if(spe_sim_get_mac_fw_entry(src_mac_addr, &current_src_entry)){/*fail in get src mac*/

			/*sent pkt to cpu port*/
			SPE_SIM_TRACE("mac fw:Send pkt to cpu, src mac entry not found. \n");
			return spe_sim_send_pkt_to_cpu(src_port_info, buffer, size, 0);
		}
		if(current_src_entry->entry.port_index != src_port_info->port_num){/*src mac and src port not match*/

			/*sent pkt to cpu port*/
			SPE_SIM_TRACE("mac fw:Send pkt to cpu, src mac & port not match. \n");
			return spe_sim_send_pkt_to_cpu(src_port_info, buffer, size, 0);
		}

	}
	if(current_dst_entry->entry.port_index >= SPE_SIM_MAX_PORTS){/*check the dst mac is host mac or not ut 10*/
		/*dst mac is host mac, send to ip fw process*/
		SPE_SIM_TRACE("mac fw:Send pkt to ip fw, dst mac != src mac . \n");
		spe_sim_ip_entry_process(src_port_info, buffer, size);
		return 0;
	}
	else
	{
		SPE_SIM_TRACE("mac fw:enter dst src mac port detcet \n");
		if(current_dst_entry->entry.br_portno == current_src_entry->entry.br_portno){/*Did src mac and dst mac balongs to the same bridge?*/
			/*src mac and dst mac balongs to the same bridge, send pkt to dst port*/
			src_port_info->modify.modify_mask = 0;
			if(0 != spe_sim_get_port_info(current_dst_entry->entry.port_index , &dst_port_info)){
			SPE_SIM_ERR("mac fw:Fail to get port info while try to do mac forwarding\n");
			return -ENXIO;
			}

			/*call aging sub process*/
			#if SPE_SIM_ENABLE_AGING

			if (spe_sim_age_mac_fw_entry(current_src_entry)) {
				SPE_SIM_TRACE("mac fw:Enter mac entry age process\n");
				spe_sim_send_pkt_to_cpu(src_port_info, buffer, size, 1);
				spe_sim_age_proc_end(src_port_info, &current_src_entry->aging_timer);
			}
			#endif
			/*Send pkt to port by mac fw*/
			spe_sim_rd_process(src_port_info, dst_port_info, &src_port_info->modify, 0);
			return 0;
		}
		else
		{
			/*src mac and dst mac balongs to different bridge, send pkt to cpu*/
			SPE_SIM_TRACE("mac fw:src & dst mac balongs to different bridge, send pkt to cpu \n");
			return spe_sim_send_pkt_to_cpu(src_port_info, buffer, size, 0);
		}
	}
}

void sep_sim_entry_set_hash_params(unsigned int base, unsigned int rnd,
                    unsigned short zone, unsigned int width, unsigned int num)
{
    //if (!base || !num)
        //SPE_SIM_ERR("hash entry:%s set hash table fail !!\n");
    SPE_SIM_INFO("entry hash:%s base 0x%x rnd 0x%x zone 0x%x width 0x%x num 0x%x \n",
		__func__, base, rnd, zone, width, num);
    ip_fw_ctx.entry_table = (char*)base;
    ip_fw_ctx.hash_rnd = rnd;//0
    ip_fw_ctx.zone = zone;//参数1
    ip_fw_ctx.hash_width = width;
    ip_fw_ctx.hash_num = num;//个数1
}

/* eth type check, ret 1: to cpu port, ret 0: go ahead, ret -1: drop */
static inline int spe_sim_entry_eth_check(spe_sim_port_t* port,
                                   char* buffer, unsigned int size)
{
    const struct ethhdr* eth = (struct ethhdr*)buffer;

    if (likely(eth->h_proto == htons(ETH_P_IP))) {
        port->cur_is_ipv4 = 1;
        SPE_SIM_TRACE("ip_hdr check:spe_sim_entry_eth_check pkt is ipv4 \n");
        return 0;
    }
    else if (eth->h_proto == htons(ETH_P_IPV6)) {
        SPE_SIM_TRACE("ip_hdr check:spe_sim_entry_eth_check pkt is ipv6 \n");
        port->cur_is_ipv4 = 0;
        return 0;
    }
#ifdef CONFIG_ETH_BYPASS_MODE
    else if (eth->h_proto == htons(ETH_P_ARP)) {
        SPE_SIM_TRACE("ip_hdr check:spe_sim_entry_eth_check pkt is arp \n");
        return 1;
    }
#endif
    SPE_SIM_TRACE("ip_hdr check:spe_sim_entry_eth_check pkt is other pkt \n");

    /* if the eth_type not support trans to cpu port */
    return 1;
}

static int spe_sim_check_l4_packet(spe_sim_port_t* port,
            char* buffer, unsigned int nhoff, unsigned int dataoff)
{
    struct iphdr *iph = (struct iphdr *)(buffer + nhoff);
    struct ipv6hdr *ipv6h = (struct ipv6hdr *)(buffer + nhoff);
    struct tcphdr *tcp = (struct tcphdr *)(buffer + dataoff);
    int is_ipv6 = 0;

    if (iph->version == 6)
    {
        is_ipv6 = 1;
        SPE_SIM_TRACE("check l4:is ipv6\n");
    }

    SPE_SIM_TRACE("check l4:enter spe_sim_check_l4_packet\n");
    if ((!is_ipv6 && (iph->protocol == IPPROTO_TCP))
        || (is_ipv6 && (ipv6h->nexthdr == NEXTHDR_TCP)))
        {
        SPE_SIM_TRACE("check l4:l4 is tcp \n");
        /* tcp syn/rst to cpu */
        if (tcp->syn || tcp->rst || tcp->fin) {
            SPE_SIM_TRACE("check l4:tcp syn||rst enable\n");
            return 1;
        }
        else {
            port->cur_l4_proto = IPPROTO_TCP;
            return 0;
        }
    }
    /* udp to spe port */
    else if ((!is_ipv6 && (iph->protocol == IPPROTO_UDP))
        || (is_ipv6 && (ipv6h->nexthdr == NEXTHDR_UDP))) {
        port->cur_l4_proto = IPPROTO_UDP;
        SPE_SIM_TRACE("check l4:l4 is udp \n");
        return 0;
    }
    /* others to cpu */
    else {
        SPE_SIM_TRACE("check l4:l4 is something other than tcp & udp \n");
        return 1;
    }
    return 0;
}

/* refer to ip_rcv and ipv4_get_l4proto */
static inline int spe_sim_ipv4_check(spe_sim_port_t* port, char* buffer,
    unsigned int size, unsigned int nhoff,
    unsigned int *dataoff, unsigned char *protonum)
{
    struct iphdr *iph = (struct iphdr *)(buffer + nhoff);

    unsigned int total_len;
    unsigned int ip_len = size - nhoff;

    SPE_SIM_INFO("ipv4_check:bufferaddr: 0x%x size:%u nhoff:%u \n",buffer,size,nhoff);
    if (iph->ihl < 5 || iph->version != 4)
        return -1;

    total_len = ntohs(iph->tot_len);
    if (ip_len < total_len || total_len < (iph->ihl << 2)){

        SPE_SIM_INFO("ipv4_check:ip len check fail ip_len =%u, total_len =%u, iph->ihl = %u \n", ip_len, total_len, iph->ihl);
        return -1;
    }
    SPE_SIM_INFO("ipv4_check:ip len check success ip_len =%u, total_len =%u, iph->ihl = %u \n", ip_len, total_len, iph->ihl);
    *dataoff = nhoff + (iph->ihl << 2);
    *protonum = iph->protocol;

    /* check data offset */
    if (*dataoff > size) {
        SPE_SIM_TRACE("ipv4_check:data off check fail \n");
        return -1;
    }

    if ((!iph->frag_off || (iph->frag_off == htons(IP_DF))) && /* fragment check: don't frag */
        iph->ttl > 1) {

        return spe_sim_check_l4_packet(port, buffer, nhoff, *dataoff);
    }
    /* other packet to cpu port */
    else {
        SPE_SIM_TRACE("ipv4_check:ip frag check fail \n");

        return 1;
    }

    return 0;
}

static int spe_sim_ipv6_ext_hdr(u8 nexthdr)
{
    /*
     * find out if nexthdr is an extension header or a protocol
     */
    return  (nexthdr == NEXTHDR_HOP)   ||
            (nexthdr == NEXTHDR_ROUTING)   ||
            (nexthdr == NEXTHDR_FRAGMENT)  ||
            (nexthdr == NEXTHDR_AUTH)  ||
            (nexthdr == NEXTHDR_NONE)  ||
            (nexthdr == NEXTHDR_DEST);
}

static int spe_sim_ipv6_skip_exthdr(char* buffer, unsigned int size, int start,
                  u8 *nexthdrp, int len)
{
    u8 nexthdr = *nexthdrp;

    while (spe_sim_ipv6_ext_hdr(nexthdr)) {
        struct ipv6_opt_hdr hdr;
        int hdrlen;

        if (len < (int)sizeof(struct ipv6_opt_hdr))
            return -1;
        if (nexthdr == NEXTHDR_NONE)
            break;
        if (nexthdr == NEXTHDR_FRAGMENT)
            break;
        if (start > (int)size - sizeof(hdr))
            BUG();
        else
            memcpy(&hdr, buffer + start, sizeof(hdr));
        if (nexthdr == NEXTHDR_AUTH)
            hdrlen = (hdr.hdrlen+2)<<2;
        else
            hdrlen = ipv6_optlen(&hdr); /* refer to ipv6.h */

        nexthdr = hdr.nexthdr;
        len -= hdrlen;
        start += hdrlen;
    }

    *nexthdrp = nexthdr;
    return start;
}

/* refer to ipv6_rcv and ipv6_get_l4proto */
static int spe_sim_ipv6_check(spe_sim_port_t* port, char* buffer,
    unsigned int size, unsigned int nhoff,
    unsigned int *dataoff, unsigned char *protonum)
{
    const struct ipv6hdr *hdr = (struct ipv6hdr *)(buffer + nhoff);
    unsigned int extoff = nhoff + sizeof(struct ipv6hdr);
    unsigned char pnum;
    unsigned int pkt_len;
    unsigned int ip_len = size - nhoff;
    int protoff;

    if (hdr->version != 6)
        return -1;

    pkt_len = ntohs(hdr->payload_len);

    /* pkt_len may be zero if Jumbo payload option is present */
	if (pkt_len || hdr->nexthdr != NEXTHDR_HOP) {
		if (pkt_len + sizeof(struct ipv6hdr) > ip_len) {
			return -1;
		}
	}

    /* hop packet trans to cpu port */
	if (hdr->nexthdr == NEXTHDR_HOP) {
		return 1;
	}

    pnum = *(buffer + nhoff + offsetof(struct ipv6hdr, nexthdr));
    protoff = spe_sim_ipv6_skip_exthdr(buffer, size, extoff, &pnum, size - extoff);
    /*
     * (protoff == skb->len) mean that the packet doesn't have no data
     * except of IPv6 & ext headers. but it's tracked anyway. - YK
     */
    if ((protoff < 0) || (protoff > size)) {
        return -1;
    }

    *dataoff = protoff;
    *protonum = pnum;

    return spe_sim_check_l4_packet(port, buffer, nhoff, *dataoff);
}

static inline void spe_sim_ipv4_pkt_to_tuple(char* buffer,
    unsigned int nhoff, spe_sim_conn_tuple_t *tuple)
{
    const __be32 *ap;

    ap = (__be32 *)(buffer + nhoff + offsetof(struct iphdr, saddr));
    tuple->src.u3.ip = ap[0];
    tuple->dst.u3.ip = ap[1];

    return;
}

static inline void spe_sim_ipv6_pkt_to_tuple(char* buffer, unsigned int nhoff,
                  spe_sim_conn_tuple_t *tuple)
{
    const unsigned int *ap;

    ap = (unsigned int *)(buffer + nhoff + offsetof(struct ipv6hdr, saddr));
    memcpy(tuple->src.u3.ip6, ap, sizeof(tuple->src.u3.ip6));
    memcpy(tuple->dst.u3.ip6, ap + 4, sizeof(tuple->dst.u3.ip6));

    return;
}

static inline void spe_sim_tcp_pkt_to_tuple(char* buffer, unsigned int dataoff,
                 spe_sim_conn_tuple_t *tuple)
{
    const struct tcphdr *hp;

    hp = (struct tcphdr *)(buffer + dataoff);

    /*
    tcp and udp has the same pos in tuple, so we just use tcp.port as l4 port

    for example:
    tuple->src.u.udp.port = hp->source;
    tuple->dst.u.udp.port = hp->dest;
    */
    tuple->src.u.tcp.port = hp->source;
    tuple->dst.u.tcp.port = hp->dest;

    return;
}

static inline void spe_sim_udp_pkt_to_tuple(char* buffer, unsigned int dataoff,
                 spe_sim_conn_tuple_t *tuple)
{
    const struct udphdr *hp;

    hp = (struct udphdr *)(buffer + dataoff);
    tuple->src.u.udp.port = hp->source;
    tuple->dst.u.udp.port = hp->dest;

    return;
}

static inline void spe_sim_get_tuple(spe_sim_port_t* port, char* buffer,
    unsigned int size, unsigned int nhoff,
    unsigned int dataoff, unsigned char protonum)
{
    spe_sim_conn_tuple_t *cur_tuple = &ip_fw_ctx.cur_tuple;
    const struct ethhdr* eth = (struct ethhdr*)buffer;
    //char* l3_buf = buffer + nhoff;

    memset(cur_tuple, 0, sizeof(*cur_tuple));

    if (likely(port->cur_is_ipv4)) {
        //struct iphdr *iph = (struct iphdr *)l3_buf;
        cur_tuple->src.l3num = PF_INET;
        spe_sim_ipv4_pkt_to_tuple(buffer, nhoff, cur_tuple);
    }
    else if (eth->h_proto == htons(ETH_P_IPV6)) {
        cur_tuple->src.l3num = PF_INET6;
        spe_sim_ipv6_pkt_to_tuple(buffer, nhoff, cur_tuple);
    }
    cur_tuple->dst.protonum = (u_int8_t)protonum;
    cur_tuple->dst.dir = IP_CT_DIR_ORIGINAL;
    if(IPPROTO_TCP == port->cur_l4_proto){
        SPE_SIM_TRACE("ip fw get tuple:Get tuple success, pkt is  tcp. \n");
        spe_sim_tcp_pkt_to_tuple(buffer, dataoff, cur_tuple);
    }
    else if(IPPROTO_UDP == port->cur_l4_proto){
        SPE_SIM_TRACE("ip fw get tuple:Get tuple success, pkt is udp. \n");
        spe_sim_udp_pkt_to_tuple(buffer, dataoff, cur_tuple);
    }
    else{
        SPE_SIM_TRACE("ip fw get tuple:Get tuple fail, pkt is nether tcp nor udp. \n");
    }
}

/* tcp/ip check, ret 1: to cpu port, ret 0: go ahead, ret -1: drop */
static inline int spe_sim_entry_tcpip_check(spe_sim_port_t* port,
                                  char* buffer, unsigned int size)
{
    //struct ethhdr* eth = (struct ethhdr*)buffer;
    unsigned int dataoff = 0;
    unsigned char protonum = 0;
    int ret;

    /* ipv4 */
    if (likely(port->cur_is_ipv4)) {
        SPE_SIM_TRACE("ip fw tcpip check:Get tuple success, pkt is ipv4. \n");
        ret = spe_sim_ipv4_check(port, buffer, size,
                    ETH_HLEN, &dataoff, &protonum);
        if(ret){
            port->debug_info.ipv4_check_fail_drop++;
			return ret;
        }
        /* prepare tuple for hash */
        spe_sim_get_tuple(port, buffer, size, ETH_HLEN, dataoff, protonum);
    }
    /* ipv6 */
    else {

        //print_hex_dump(KERN_ERR, "ipv6_data_check:   ", DUMP_PREFIX_ADDRESS, \
	     //               16, 1, buffer, size, true);
        ret = spe_sim_ipv6_check(port, buffer, size,
                    ETH_HLEN, &dataoff, &protonum);
        if(ret){
            port->debug_info.ipv6_check_fail_drop++;
			return ret;
        }
        /* prepare tuple for hash */
        spe_sim_get_tuple(port, buffer, size, ETH_HLEN, dataoff, protonum);
    }

    return ret;
}

static u32 __spe_sim_hash_bucket(u32 hash, unsigned int size)
{
	return ((u64)hash * size) >> 32;
}

/* please refer to kernel/net/netfilter/nf_conntrack_core.c */
static u32 spe_sim_hash_raw(const spe_sim_conn_tuple_t *tuple, u16 zone)
{
	unsigned int n;

	/* The direction must be ignored, so we hash everything up to the
	 * destination ports (which is a multiple of 4) and treat the last
	 * three bytes manually.
	 */
	//printk("hash_rnd:%d\n", ip_fw_ctx.hash_rnd);
	n = (sizeof(tuple->src) + sizeof(tuple->dst.u3)) / sizeof(u32);
	return jhash2((u32 *)tuple, n, zone ^ ip_fw_ctx.hash_rnd ^
		      (((__force __u16)tuple->dst.u.all << 16) |
		      tuple->dst.protonum));
}

/* get the hash idx according to tuple */
static u32 spe_sim_get_hash(const spe_sim_conn_tuple_t *tuple)
{
	return __spe_sim_hash_bucket
            (spe_sim_hash_raw(tuple, ip_fw_ctx.zone), ip_fw_ctx.hash_num);
}

static inline int spe_sim_hash_empty(struct spe_sim_ip_fw_entry *entry)
{
    //return (entry->list.prev == NULL && entry->list.next == NULL);
    return (int)(!entry->valid);
}

/* ip entry don't need add / del / dump function */
#if 0
static int spe_sim_add_ip_fw_entry(struct spe_sim_ip_fw_entry *entry)
{
}

static int spe_sim_del_ip_fw_entry(struct spe_sim_ip_fw_entry *current_entry)
{
}

static int spe_sim_dump_ip_fw_entry(void)
{
}
#endif

static inline int __spe_sim_addr_cmp(const union spe_sim_nf_inet_addr *a1,
				   const union spe_sim_nf_inet_addr *a2)
{
	return a1->all[0] == a2->all[0] &&
	       a1->all[1] == a2->all[1] &&
	       a1->all[2] == a2->all[2] &&
	       a1->all[3] == a2->all[3];
}

static inline bool __spe_sim_tuple_src_equal(const spe_sim_conn_tuple_t *t1,
					   const spe_sim_conn_tuple_t *t2)
{
	return (__spe_sim_addr_cmp(&t1->src.u3, &t2->src.u3) &&
		t1->src.u.all == t2->src.u.all &&
		t1->src.l3num == t2->src.l3num);
}

static inline bool __spe_sim_tuple_dst_equal(const spe_sim_conn_tuple_t *t1,
					   const spe_sim_conn_tuple_t *t2)
{
	return (__spe_sim_addr_cmp(&t1->dst.u3, &t2->dst.u3) &&
		t1->dst.u.all == t2->dst.u.all &&
		t1->dst.protonum == t2->dst.protonum);
}

static inline bool spe_sim_tuple_equal(const spe_sim_conn_tuple_t *t1,
				     const spe_sim_conn_tuple_t *t2)
{
	return __spe_sim_tuple_src_equal(t1, t2) &&
	       __spe_sim_tuple_dst_equal(t1, t2);
}

extern ipv6_tuple_show(struct nf_conntrack_tuple *tuple);

/* cur packet has been parsed to cur_tuple */
int spe_sim_get_ip_fw_entry(spe_sim_port_t* port,
                                   struct spe_sim_ip_fw_entry** get_entry)
{
    unsigned int hash;
    struct spe_sim_ip_fw_entry *head = (struct spe_sim_ip_fw_entry*)(ip_fw_ctx.entry_table);
    struct spe_sim_ip_fw_entry *cur;
    spe_sim_conn_tuple_t *cur_tuple = &ip_fw_ctx.cur_tuple;
	int i = 0;
    if (!ip_fw_ctx.entry_table) {
        printk(KERN_EMERG
            "ip tables base address is NULL!, all packet trans to cpu port\n");
        return -ENODATA;
    }
	SPE_SIM_TRACE("%s:enter\n", __func__);
	if (spe_msg_level & SPE_SIM_MSG_DBG) {
		//tuple_show((struct nf_conntrack_tuple *)cur_tuple);
	}
	SPE_SIM_TRACE("iptable\n");
	if (spe_msg_level & SPE_SIM_MSG_DBG) {
		//iptable_show();
	}
    if (spe_msg_level & SPE_SIM_MSG_TUPLE) {
        //printk("tuple_show->%s:%d**************************\n", __func__, __LINE__);
    }
    hash = spe_sim_get_hash(cur_tuple);
    if (spe_msg_level & SPE_SIM_MSG_TUPLE) {
        //tuple_show(cur_tuple);
        //ipv6_tuple_show(cur_tuple);
        printk("hash:%d\n", hash);
        printk("hsize:%d,hzone:%d\n", ip_fw_ctx.hash_num, ip_fw_ctx.zone);
        //printk("tuple_show->%s:%d**************************\n", __func__, __LINE__);
    }

    /* get the hash header */
    head = SPE_SIM_GET_HASH_HEAD(hash);
    cur = head;
    do{
		if (spe_msg_level & SPE_SIM_MSG_DBG) {
			//tuple_show(&cur->tuple);
            //ipv6_tuple_show(&cur->tuple);
		}
        if (cur->valid && spe_sim_tuple_equal(&cur->tuple, cur_tuple)) {
            *get_entry = cur;
            goto find_entry;
        }
        cur = GET_NEXT_HASH_NODE(cur);

    }while(cur != head && cur != NULL);
	SPE_SIM_TRACE("not find entry from hash table\n");
    return -ENODATA;

find_entry:
    if (((cur->iptype == SPE_SIM_IP_ENTRY_IPV4) && !port->cur_is_ipv4)
        && ((cur->iptype != SPE_SIM_IP_ENTRY_IPV4) && port->cur_is_ipv4) ) {
        printk(KERN_EMERG"some errors in hash tables, attr.ipv4/ipv6 not match\n");
        return -ENODATA;
    }
	SPE_SIM_TRACE("find entry from hash table\n");

    return 0;
}

static inline int spe_sim_age_ip_fw_entry(struct spe_sim_ip_fw_entry *entry)
{
    /* static entry can't timeout */
    if (entry->is_static) {
        return 0;
    }

	if(time_before(jiffies, entry->age + ip_fw_ctx.aging_time)) {
		return 0;
	}
    return 1;
}

static inline void spe_sim_nat(spe_sim_port_t* port,
                               struct spe_sim_ip_fw_entry *entry)
{
    if (entry->action == 0) {
		//SPE_SIM_ERR("snat\n");
        port->modify.modify_mask |= SPE_SIM_MODIFY_SRC |
                                    (SPE_SIM_MODIFY_IP_PORT|SPE_SIM_MODIFY_SMAC);
        port->modify.ip = entry->nat.ip;
        port->modify.port = entry->nat.port;
    }
    else if (entry->action == 1) {
		//SPE_SIM_ERR("dnat\n");

        port->modify.modify_mask |= (SPE_SIM_MODIFY_IP_PORT|SPE_SIM_MODIFY_SMAC);
        port->modify.ip = entry->nat.ip;
        port->modify.port = entry->nat.port;
    }
    else {
		SPE_SIM_ERR("do nothing\n");
        port->modify.modify_mask &= ~(SPE_SIM_MODIFY_IP_PORT);
    }
    port->modify.l4_proto = port->cur_l4_proto;

    if (port->modify.modify_mask) {
        port->debug_info.stat_do_nat++;
    }
}

static inline void spe_sim_replace_mac(spe_sim_port_t* port,
                                       struct spe_sim_ip_fw_entry *entry)
{
    port->modify.modify_mask |=
        (SPE_SIM_MODIFY_DMAC|SPE_SIM_MODIFY_SMAC);
    memcpy(&port->modify.dest, &entry->dhost,
           sizeof(struct ether_addr));
    memcpy(&port->modify.source, &entry->shost,
           sizeof(struct ether_addr));
	SPE_SIM_INFO("replace mac\n");
}

static inline spe_sim_port_t*
spe_sim_ipfwd_get_dst(spe_sim_port_t* port, struct spe_sim_ip_fw_entry* ip_entry)
{
    struct spe_mac_fw_entry_list* mac_entry;
    spe_sim_port_t* dst;

    if (SPE_SIM_IS_BR(ip_entry->port_index)) {
        /* find out port by ip_entry->dhost */
        if (spe_sim_get_mac_fw_entry(ip_entry->dhost.octet, &mac_entry)) {
            SPE_SIM_TRACE("ip fw get dst:Get mac entry fail. \n");
            dst = NULL;
        }
        else {
            if (mac_entry->entry.br_portno == ip_entry->port_index) {
                spe_sim_get_port_info(mac_entry->entry.port_index, &dst);
            }
            else {
				dst = NULL;
				SPE_SIM_TRACE("ip fw get dst:mac entry port num != ip_entry->port_index. \n");

            }
        }
    }
    else {
        spe_sim_get_port_info(ip_entry->port_index, &dst);
        SPE_SIM_TRACE("ip fw get dst:dst is not bridge. \n");
    }
    SPE_SIM_TRACE("ip fw get dst:spe_sim_ipfwd_get_dst return dst is 0x%x\n",dst);
    return dst;
}

static void spe_sim_ip_entry_process(spe_sim_port_t* src_port_info, char* buffer, unsigned int size)
{
    int ret;
    struct spe_sim_ip_fw_entry* ip_entry;
    spe_sim_port_t* dst = NULL;
    SPE_SIM_TRACE("ip fw:Enter spe_sim_ip_entry_process \n");

    src_port_info->debug_info.stat_ip_entry_proc++;

	if (spe_msg_level & SPE_SIM_MSG_DUMP) {
		print_hex_dump(KERN_ERR, "process: ", DUMP_PREFIX_ADDRESS, \
			16, 1, buffer, min(64, size), false);
		printk("***************\n");
	}

    ret = spe_sim_ip_filter(src_port_info, buffer, size);
    if (ret) {
        src_port_info->debug_info.ip_filter_drop++;
        goto ip_entry_ret;
    }
    SPE_SIM_TRACE("ip fw:spe_sim_ip_filter done\n");
    ret = spe_sim_entry_tcpip_check(src_port_info, buffer, size);
    if (ret) {
        goto ip_entry_ret;
    }
    SPE_SIM_TRACE("ip fw:spe_sim_entry_tcpip_check done\n");
    if (spe_sim_get_ip_fw_entry(src_port_info, &ip_entry)) {
        spe_sim_send_pkt_to_cpu(src_port_info, buffer, size, 0);
        return;
    }
    SPE_SIM_TRACE("ip fw:spe_sim_get_ip_fw_entry done\n");
    /* ip entry timeout */
#if SPE_SIM_ENABLE_AGING
    if (spe_sim_age_ip_fw_entry(ip_entry)) {
        spe_sim_send_pkt_to_cpu(src_port_info, buffer, size, 1);
        spe_sim_age_proc_end(src_port_info, (unsigned long*)&ip_entry->age);
    }
#endif
    dst = spe_sim_ipfwd_get_dst(src_port_info, ip_entry);
    if (src_port_info->cur_is_ipv4) {
        /* nat proc */
        spe_sim_nat(src_port_info, ip_entry);
    }

    /* mac replace */
    spe_sim_replace_mac(src_port_info, ip_entry);

    if (dst)
        spe_sim_rd_process(src_port_info, dst, &src_port_info->modify, 0);
    else
        spe_sim_send_pkt_to_cpu(src_port_info, buffer, size, 0);

    return;

ip_entry_ret:
    if (ret < 0) {
        src_port_info->debug_info.total_pkt_drop++;
        spe_sim_td_complete(src_port_info, 0x2, NULL);
    }
    else {
        spe_sim_send_pkt_to_cpu(src_port_info, buffer, size, 0);
    }
    return;
}

#ifdef CONFIG_ETH_BYPASS_MODE
spe_sim_port_t* spe_sim_stick_find_dst_port(spe_sim_ctx_t* ctx,
                spe_sim_port_t* src_port)
{
    int i;
	spe_sim_enc_type_t src_enc_type = src_port->enc_type;
	spe_sim_enc_type_t dst_enc_type;

    /* find the different port */
    for (i = 0; i < ctx->ports_en_num; i++) {
		dst_enc_type = ctx->ports_en[i]->enc_type;
        if ((spe_sim_enc_ncm == src_enc_type && spe_sim_enc_ipf == dst_enc_type) ||
			(spe_sim_enc_ipf == src_enc_type && spe_sim_enc_ncm == dst_enc_type)) {
            return ctx->ports_en[i];
        }
    }
    return NULL;
}

void spe_sim_stick_process(spe_sim_port_t* src_port, char* buffer, unsigned int size)
{
    spe_sim_ctx_t* ctx = (spe_sim_ctx_t*)src_port->ctx;
    spe_sim_port_t* dst_port;

    src_port->modify.modify_mask = 0;
	src_port->src_len = size;
	src_port->src_ptr = buffer;

    dst_port = spe_sim_stick_find_dst_port(ctx, src_port);
    if (NULL == dst_port) {
        SPE_SIM_ERR("Entry:Port config error!Can not find dst port in stick mode\n");
		src_port->debug_info.total_pkt_drop++;
        spe_sim_td_complete(src_port, 0x2, NULL);
        return;
    }
	spe_sim_rd_process(src_port, dst_port, &src_port->modify, 0);
	return;
}
#endif

void spe_sim_entry_process(spe_sim_port_t* port, char* buffer, unsigned int size)
{
    int ret;

    port->debug_info.stat_entry_proc++;

    /* clear the modify_mask in start */
    port->modify.modify_mask = 0;
    if (NULL != spe_sim_mac_filter(port, buffer, size)) {
        SPE_SIM_TRACE("entry:get mac filter fail go to cpu \n");
        goto entry_ret;
    }

    ret = spe_sim_entry_eth_check(port, buffer, size);
    if (ret) {
        SPE_SIM_TRACE("entry:spe_sim_entry_eth_check fail go to cpu \n");
        goto entry_ret;
    }

#ifdef CONFIG_ETH_BYPASS_MODE
    return spe_sim_stick_process(port, buffer, size);
#endif

    spe_sim_mac_entry_process(port, buffer, size);

    return;

entry_ret:
    if (ret < 0) {
        port->debug_info.total_pkt_drop++;
        spe_sim_td_complete(port, 0x2, NULL);
    }
    else {
        spe_sim_send_pkt_to_cpu(port, buffer, size, 0);
    }
    return;
}
/*
void spe_sim_rescan_port_by_num(spe_sim_ctx_t* ctx, int port_num)
{
    int i;

    if (unlikely(port_num >= SPE_SIM_MAX_PORTS)) {
        printk(KERN_EMERG"%s, error port num:%d\n", __func__, port_num);
        return;
    }
    for (i = 0; i < SPE_SIM_MAX_PORTS; i++) {
        if (ctx->ports[i].port_num == port_num) {
            ctx->ports_idx[port_num] = &ctx->ports[i];
        }
    }
}
*/

/* read / add / delete a table entry */
void spe_sim_entry_operate(spe_sim_ctx_t* ctx, spe_sim_entry_action_t action,
                           spe_sim_entry_type_t type, char* entry)
{

    struct spe_mac_fw_entry *spe_entry = (struct spe_mac_fw_entry *)entry;
    struct spe_sim_mac_fw_entry spe_sim_entry;
    struct spe_mac_addr_heads* mac_addr;

    spe_sim_entry.br_portno = spe_entry->port_br_id;
    spe_sim_entry.port_index = spe_entry->port_no;
    spe_sim_entry.is_static = spe_entry->is_static;

    mac_addr = &spe_entry->mac_hi;
    spe_sim_entry.mac[0] = mac_addr->mac[3];
    spe_sim_entry.mac[1] = mac_addr->mac[2];
    spe_sim_entry.mac[2] = mac_addr->mac[1];
    spe_sim_entry.mac[3] = mac_addr->mac[0];
    mac_addr = &spe_entry->mac_lo;
    spe_sim_entry.mac[4] = mac_addr->mac[1];
    spe_sim_entry.mac[5] = mac_addr->mac[0];

    SPE_SIM_TRACE("entry: %s enter  \n",__func__);

    /* we just process mac fw entry */
    if (spe_sim_entry_mac_forward == type) {
        if (spe_sim_add_entry == action) {
            spe_sim_add_mac_fw_entry(&spe_sim_entry);
        }
        else if (spe_sim_del_entry == action) {
            spe_sim_del_mac_fw_entry(&spe_sim_entry);
        }
    }

}

void spe_sim_mac_fw_ctx_show()
{
	SPE_SIM_ERR("mac_fw_ctx.cpu_port		0x%x\n", (unsigned int)mac_fw_ctx.cpu_port);
	//SPE_SIM_INFO("mac_fw_ctx.cpu_port point is %x\n", mac_fw_ctx.cpu_port);
	//SPE_SIM_INFO("mac_fw_ctx.cpu_port point is %x\n", mac_fw_ctx.cpu_port);
	SPE_SIM_ERR("mac_fw_ctx.aging_time		%u\n", mac_fw_ctx.aging_time);

	SPE_SIM_ERR("mac_fw_ctx.stat_add_entry  %u\n", mac_fw_ctx.stat_add_entry);
	SPE_SIM_ERR("mac_fw_ctx.stat_del_entry  %u\n", mac_fw_ctx.stat_del_entry);
}

void spe_sim_ip_fw_ctx_show()
{
	SPE_SIM_ERR("ip_fw_ctx.hash_rnd		%u\n", ip_fw_ctx.hash_rnd);
	SPE_SIM_ERR("ip_fw_ctx.hash_num		%u\n", ip_fw_ctx.hash_num);
	SPE_SIM_ERR("ip_fw_ctx.hash_width		%u\n", ip_fw_ctx.hash_width);
	SPE_SIM_ERR("ip_fw_ctx.zone			%u\n", ip_fw_ctx.zone);
	//SPE_SIM_INFO("ip_fw_ctx.cur_tuple point is %d\n", ip_fw_ctx.cur_tuple);
	SPE_SIM_ERR("ip_fw_ctx.aging_time		%u\n", ip_fw_ctx.aging_time);
	SPE_SIM_ERR("ip_fw_ctx.entry_table		0x%x\n", (unsigned int)ip_fw_ctx.entry_table);
}


module_init(spe_sim_mac_entry_process_init);

