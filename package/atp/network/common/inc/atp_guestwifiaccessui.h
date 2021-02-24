#ifndef _ATP_GUESTWIFI_ACCESS_UI_H_
#define _ATP_GUESTWIFI_ACCESS_UI_H_

#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/atomic.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nf_conntrack_tuple_common.h>
#include <net/netfilter/nf_conntrack.h>

#define DEVNAME_LENGTH_MAX 16
#define GUESTWIFI_BUFF_LEN_MAX             64

struct guestwifiaccessui_info
{
    int          lEnable;/*0代表允许访问 1代表禁止访问*/
    char         dev[DEVNAME_LENGTH_MAX]; /*客人wifi设备名*/
    unsigned int ul_lan_addr;  
};
#endif

