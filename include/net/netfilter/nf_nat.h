#ifndef _NF_NAT_H
#define _NF_NAT_H
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/nf_nat.h>
#include <net/netfilter/nf_conntrack_tuple.h>

#ifdef CONFIG_ATP_ROUTE
#define IP_NAT_MANIP_SRC  NF_NAT_MANIP_SRC
#define IP_NAT_MANIP_DST  NF_NAT_MANIP_DST
#endif
enum nf_nat_manip_type {
	NF_NAT_MANIP_SRC,
	NF_NAT_MANIP_DST
};

/* SRC manip occurs POST_ROUTING or LOCAL_IN */
#define HOOK2MANIP(hooknum) ((hooknum) != NF_INET_POST_ROUTING && \
			     (hooknum) != NF_INET_LOCAL_IN)

#ifdef CONFIG_ATP_ROUTE
#define IP_NAT_RANGE_MAP_IPS 1
#define IP_NAT_RANGE_PROTO_SPECIFIED 2
#define IP_NAT_RANGE_PROTO_RANDOM 4
#define IP_NAT_RANGE_PERSISTENT 8
#endif
/* NAT sequence number modifications */
struct nf_nat_seq {
	/* position of the last TCP sequence number modification (if any) */
	u_int32_t correction_pos;

	/* sequence number offset before and after last modification */
	int16_t offset_before, offset_after;
};

#ifdef CONFIG_ATP_ROUTE

/* For backwards compat: don't use in modern code. */
struct nf_nat_multi_range_compat {
	unsigned int rangesize; /* Must be 1. */

	/* hangs off end. */
	struct nf_nat_range range[1];
};
#endif
#include <linux/list.h>
#include <linux/netfilter/nf_conntrack_pptp.h>
#include <net/netfilter/nf_conntrack_extend.h>

/* per conntrack: nat application helper private data */
union nf_conntrack_nat_help {
	/* insert nat helper private data here */
#if defined(CONFIG_NF_NAT_PPTP) || defined(CONFIG_NF_NAT_PPTP_MODULE)
	struct nf_nat_pptp nat_pptp_info;
#endif
};

struct nf_conn;

/* The structure embedded in the conntrack structure. */
struct nf_conn_nat {
	struct hlist_node bysource;
	struct nf_nat_seq seq[IP_CT_DIR_MAX];
	struct nf_conn *ct;
	union nf_conntrack_nat_help help;
#if defined(CONFIG_IP_NF_TARGET_MASQUERADE) || \
    defined(CONFIG_IP_NF_TARGET_MASQUERADE_MODULE) || \
    defined(CONFIG_IP6_NF_TARGET_MASQUERADE) || \
    defined(CONFIG_IP6_NF_TARGET_MASQUERADE_MODULE)
	int masq_index;
#endif
};

/* Set up the info structure to map into this range. */
extern unsigned int nf_nat_setup_info(struct nf_conn *ct,
				      const struct nf_nat_range *range,
				      enum nf_nat_manip_type maniptype);

/* Is this tuple already taken? (not by us)*/
extern int nf_nat_used_tuple(const struct nf_conntrack_tuple *tuple,
			     const struct nf_conn *ignored_conntrack);

static inline struct nf_conn_nat *nfct_nat(const struct nf_conn *ct)
{
#if defined(CONFIG_NF_NAT) || defined(CONFIG_NF_NAT_MODULE)
	return nf_ct_ext_find(ct, NF_CT_EXT_NAT);
#else
	return NULL;
#endif
}

static inline bool nf_nat_oif_changed(unsigned int hooknum,
				      enum ip_conntrack_info ctinfo,
				      struct nf_conn_nat *nat,
				      const struct net_device *out)
{
#if IS_ENABLED(CONFIG_IP_NF_TARGET_MASQUERADE) || \
    IS_ENABLED(CONFIG_IP6_NF_TARGET_MASQUERADE)
	return nat->masq_index && hooknum == NF_INET_POST_ROUTING &&
	       CTINFO2DIR(ctinfo) == IP_CT_DIR_ORIGINAL &&
	       nat->masq_index != out->ifindex;
#else
	return false;
#endif
}

#endif
