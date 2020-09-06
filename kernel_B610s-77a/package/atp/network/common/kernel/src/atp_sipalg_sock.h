
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/in.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <net/route.h>
#include <linux/list.h>

#define ALG_SIP_START_SUCCESS         1

#define SIPALG_DSTIP                    71

extern uint32_t g_ulSiproxdStatus;
extern uint32_t g_ulSipPortInfo;
extern unsigned char _ctype[];

extern unsigned int atp_sipalg_setsock(struct sk_buff* skb, int optname, char __user *optval);

extern unsigned int atp_sipalg_getsock(struct sk_buff* skb, int optname, char __user *optval);

extern void UpdateSipRouteByCallid(unsigned char *call_id, unsigned int call_id_len, __be32 daddr, __be16 dport, uint32_t proto, bool bAdd);
