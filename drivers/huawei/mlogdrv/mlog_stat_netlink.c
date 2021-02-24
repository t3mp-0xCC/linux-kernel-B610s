/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/socket.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/mlog_lib.h>


//#define NETLINK_STAT_EVENT  29    /*choose one netlink number no used*/

typedef struct
{
    struct sock*            dev_sock;  /*netlink socket*/
    unsigned int            user_pid;   /*user task pid*/
} stat_msg_ctrl_t ;

#if defined(FEATURE_HUAWEI_MLOG_SWITCH)
extern int mlog_is_disable(void);
#endif/*FEATURE_HUAWEI_MLOG_SWITCH*/

unsigned int g_partition_shared;
stat_msg_ctrl_t stat_msg_ctrl;
void  mlog_stat_netlink_init(void);
void kernel_receive(struct sk_buff* __skb) ;
#define INIT_MAGIC 0x1122aa55
unsigned int g_mlog_server_inited = 0; 

static struct netlink_kernel_cfg mlog_cfg = {
    .groups = 0,
    .input = kernel_receive,
};

int mlog_server_is_initd(void)
{
    int ret = 0;
    if(INIT_MAGIC == g_mlog_server_inited)
    {
        ret = 1;
    }
    return ret;
}



void  mlog_stat_netlink_init(void)
{
    struct sock* netlinkfd = NULL;
    /*kernel stat info netlink */
    netlinkfd = netlink_kernel_create(&init_net, NETLINK_STAT_EVENT, &mlog_cfg);
    if (!netlinkfd)
    {
        printk(KERN_ERR "can not create a netlink socket\n");
        return;
    }
    stat_msg_ctrl.dev_sock =  netlinkfd ;
}
/*****************************************************************************
��������  : kernel_receive
��������  :
�������  :
�������  :
�� �� ֵ  : No
�޸���ʷ  :
*****************************************************************************/
void kernel_receive(struct sk_buff* __skb)

{
    struct sk_buff* skb;
    struct nlmsghdr* nlh = NULL;

    skb = skb_get(__skb);
    if (skb->len >= sizeof(struct nlmsghdr))
    {
        nlh = (struct nlmsghdr*)skb->data;
        if ((nlh->nlmsg_len >= sizeof(struct nlmsghdr)) && (__skb->len >= nlh->nlmsg_len))
        {
            stat_msg_ctrl.user_pid = nlh->nlmsg_pid;
            memcpy(&g_partition_shared, (char*)NLMSG_DATA(nlmsg_hdr(__skb)), sizeof(int));
            printk(KERN_INFO"[kernel space] data receive from user_pid %d are 0x%x\n", stat_msg_ctrl.user_pid, g_partition_shared);
        }
        if ( 0 != stat_msg_ctrl.user_pid )
        {
            g_mlog_server_inited = INIT_MAGIC;
        }

    }
    else
    {
        printk(KERN_INFO"[kernel space] data receive from user are:%s\n", (char*)NLMSG_DATA(nlmsg_hdr(__skb)));
    }

    kfree_skb(skb);

}


/*****************************************************************************
��������  : mlog_stat_item_send
��������  :
�������  :
�������  :
�� �� ֵ  : No
�޸���ʷ  :
*****************************************************************************/
int mlog_stat_item_send(mlog_server_stat_info* stat_info)
{
    int size;
    struct sk_buff* skb;
    struct nlmsghdr* nlh;
    int retval;
    if (NULL ==  stat_info)
    {
        printk("stat_info is NULL\n");
        return -1;
    }

#if defined(FEATURE_HUAWEI_MLOG_SWITCH)
    if (mlog_is_disable())
    {
        return;
    }
#endif

    size = NLMSG_SPACE(sizeof (mlog_server_stat_info));
    skb = alloc_skb(size, GFP_ATOMIC);
    if(NULL == skb)
    {
        printk(KERN_ERR "[kernel space] skb alloc_skb is failed.\n");
        return -1;
    }
    nlh = nlmsg_put(skb, 0, 0, 0, size - sizeof(struct nlmsghdr), 0);
    memcpy(NLMSG_DATA(nlh), stat_info, sizeof(mlog_server_stat_info));     /*message filled*/

    NETLINK_CB(skb).dst_group = 0;

    retval = netlink_unicast(stat_msg_ctrl.dev_sock, skb, stat_msg_ctrl.user_pid, MSG_DONTWAIT);
    if (retval < 0 )
    {
        printk(KERN_ERR "[kernel space] netlink_unicast failed: %d\n", retval);
        return -1;
    }

    return 0;
}
