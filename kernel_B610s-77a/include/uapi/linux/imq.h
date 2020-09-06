#ifndef _IMQ_H
#define _IMQ_H

#define IMQ_MAX_DEVS     16
#define QOS_DEFAULT_MARK  0x100000

#ifdef CONFIG_DOWN_RATE_CONTROL
	#define QOS_DOWNDEFAULT_MARK  0x400000
	#define IS_ETN_LAN(name)  ((('e' == name[0])&&('3' == name[5])) || ('w' == name[0])||(('e' == name[0]) && ('0' == name[3])&& ('\0' == name[4])))
	//#define IS_ETN_LAN(name)  ((('e' == name[0])&&('3' == name[5])))
	#define IS_DOWN_WIFI(name)  ('w' == name[0])
#endif

#define QOS_DEV_IMQ4      0x4
#define IS_DOWN_WAN(name)  ('e' == name[0])

#define QOS_DEV_IMQ0      0x0
#define QOS_DEV_IMQ1      0x1
#define QOS_DEV_IMQ2      0x2
#define QOS_DEV_IMQ3      0x3
#define QOS_DEV_IMQ5      0x5
#define QOS_HIGHEST_PRI                  0x100
#define QOS_HIGH_PRI                     0x200
#define QOS_DEFAULT_PRI                  0x000
#define QOS_PRI_MASK                     0x00000F00
#define IS_ETN_WAN(name)  ((('e' == name[0]) && ('0' == name[3])&& ('\0' == name[4]))||(('e' == name[0]) && ('_' == name[3])&& ('x' == name[4]) &&('\0' == name[5])))

/* IFMASK (16 device indexes, 0 to 15) and flag(s) fit in 5 bits */
#define IMQ_F_BITS	5

#define IMQ_F_IFMASK	0x0f
#define IMQ_F_ENQUEUE	0x10
extern int qos_enable;
extern int downqos_enable;
extern int smartqos_enable;
extern int imq_nf_queue(struct sk_buff *skb);
//#define IMQ_MAX_DEVS	(IMQ_F_IFMASK + 1)

#endif /* _IMQ_H */

