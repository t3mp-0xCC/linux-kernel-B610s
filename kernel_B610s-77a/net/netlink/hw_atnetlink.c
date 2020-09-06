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

/* 
 *
 *  Netlink adapter layer in kernel.
 *  Device drviers report event to user space through API of this layer.
 *
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <asm/uaccess.h>
#include <net/sock.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#if (FEATURE_ON == MBB_ATNLPROXY)
struct workqueue_struct *at_netlink_wq = NULL; //事件上报用工作队列

struct sock *at_device_event_sock;
u32 at_us_pid = (unsigned int)(-1);

struct delayed_work at_device_event_send_work;

#define AT_DELAY_FOR_NL_SEND msecs_to_jiffies(1)

// Device event buffer, to avoid blocking by socket
struct list_head at_device_event_queue;
spinlock_t at_queue_lock;

struct at_device_event_node {
    struct list_head list;
    int len;
    char data[0];
}at_device_event_node;

/* Limit the event buffer queue length,
 * Otherwise, this queue may consuming out all of memory
 */
int at_event_queue_len = 0;
#define AT_MAX_EVENT_QUEUE_LEN    (300)

/* Device callback
 * Used to dispatch message from userspace
 */
device_event_dispatch at_device_event_handler[NL_DEVICE_ID_MAX_ID] = {0};

/**
 *
 * Register call back
 *
 */
int at_device_event_handler_register(NL_DEVICE_ID id, device_event_dispatch dispatcher)
{
    if (((int)id < 0) || (id >= NL_DEVICE_ID_MAX_ID) || (NULL == dispatcher)) {
        return -1;
    }

    at_device_event_handler[id] = dispatcher;

    return 0;
}

EXPORT_SYMBOL_GPL(at_device_event_handler_register);

/**
 *
 * Construct a buffer node
 *
 */
static struct at_device_event_node *at_device_event_node_alloc(void * data, int len)
{
    struct at_device_event_node *node;
    /*同步北研修改*/
    unsigned long flags;

    spin_lock_irqsave(&at_queue_lock, flags);
    if (at_event_queue_len >= AT_MAX_EVENT_QUEUE_LEN)
    {
        spin_unlock_irqrestore(&at_queue_lock, flags);
        return NULL;
    }
    spin_unlock_irqrestore(&at_queue_lock, flags);
    
    node = (struct at_device_event_node *)kmalloc((len + sizeof(struct at_device_event_node)), GFP_ATOMIC);
    if (NULL == node) {
        return NULL;
    }

    node->len = len;
    memcpy(node->data, data, (unsigned int)len);
    /*同步北研修改*/
    spin_lock_irqsave(&at_queue_lock, flags);
    at_event_queue_len++;
    spin_unlock_irqrestore(&at_queue_lock, flags);

    return node;
}

/**
 *
 * Destruct a buffer node
 *
 */
static inline void at_device_event_node_free(struct at_device_event_node *node)
{

    at_event_queue_len--;

    kfree(node);
}

/**
 *
 * Put a device event into event buffer
 *
 */
static inline void at_device_event_enqueue(struct at_device_event_node *node)
{
    unsigned long flags;

    spin_lock_irqsave(&at_queue_lock, flags);
    list_add_tail(&node->list, &at_device_event_queue);
    spin_unlock_irqrestore(&at_queue_lock, flags);
}

/**
 *
 * Get a device event from event buffer
 * Notice: list should have at least one entry!
 *
 */
static inline struct at_device_event_node *at_device_event_dequeue(void)
{
    struct at_device_event_node *node = NULL;
    unsigned long flags;

    spin_lock_irqsave(&at_queue_lock, flags);
    node = list_first_entry(&at_device_event_queue, struct at_device_event_node, list);
    if (NULL == node)
    {
        spin_unlock_irqrestore(&at_queue_lock, flags);    
        return NULL;
    }

    list_del(&node->list);
    spin_unlock_irqrestore(&at_queue_lock, flags);

    return node;
}

/**
 *
 * Check whether the device event buffer queue is empty
 *
 */
static inline bool at_device_event_queue_empty(void)
{
    unsigned long flags;
    bool ret;

    spin_lock_irqsave(&at_queue_lock, flags);
    ret = list_empty(&at_device_event_queue);
    spin_unlock_irqrestore(&at_queue_lock, flags);

    return ret;
}

/**
 *
 * Process the message sent from user space 
 *
 */
static void at_device_event_input(struct sk_buff *__skb)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    NL_DEVICE_EVENT *msg_body;
    device_event_dispatch dispatcher;

    skb = skb_get(__skb);
    if (skb->len < NLMSG_SPACE(0))
    {
        return;
    }

    nlh = nlmsg_hdr(skb);

    if ((unsigned int)-1 == at_us_pid) {
        // Store the user space daemon pid
        at_us_pid = nlh->nlmsg_pid;

        // Send device events, reported before daemon start
        /*将工作任务加入到工作队列中*/
        queue_delayed_work(at_netlink_wq, &at_device_event_send_work, AT_DELAY_FOR_NL_SEND);
    }

    // Dispatch message to other device driver ...
    msg_body = NLMSG_DATA(nlh);

    if ((msg_body->device_id < 0) || (msg_body->device_id >= NL_DEVICE_ID_MAX_ID)) {
        return;
    }

    dispatcher = at_device_event_handler[msg_body->device_id];
    if (likely(NULL != dispatcher))
    {
        dispatcher(msg_body, msg_body->len + sizeof(NL_DEVICE_EVENT));
    }
    else
    {
        printk(KERN_ERR "at_device_event_input: event handler is NOT registered\n");
    }

    kfree_skb(skb);

    return;
}

/**
 *
 * Send message to user space 
 *
 */

extern int netlink_unicast(struct sock *ssk, struct sk_buff *skb,
            u32 pid, int nonblock);
static void at_device_event_send(struct work_struct *w)
{
    struct at_device_event_node *node = NULL;
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    int size;
    int ret;

    while (!at_device_event_queue_empty())
    {
        printk("at_device_event_send: queue is NOT empty\n");

        // Get one entry from event buffer queue
        node = at_device_event_dequeue();
        if (NULL == node)
        {
            break;
        }

        /*
         * node->len: size of (node->data), is event message length
         * size: size of (skb) 
         * size = sizeof(*nlh) + align pad + node->len + aligh pad
         * 
         * alloc skb here
         * But, NOT release skb here
         *  
         */
        size = NLMSG_SPACE(node->len);
        skb = alloc_skb((unsigned int)size, GFP_ATOMIC);
        if (NULL == skb)
        {
            printk(KERN_ALERT "at_device_event_send: alloc skb failed\n");
            return;
        }

        // Use "size - sizeof(*nlh)" here (incluing align pads)
        nlh = nlmsg_put(skb, 0, 0, 0, size - sizeof(*nlh), 0);

        NETLINK_CB(skb).portid = 0;
        NETLINK_CB(skb).dst_group = 0;
        memcpy (NLMSG_DATA(nlh), node->data, (unsigned int)node->len);

        if(node->len > 0)
        {
        }

        // Release this event entry
        at_device_event_node_free(node);

        // Send message to user space
        ret = netlink_unicast(at_device_event_sock, skb, at_us_pid, 0);
        if (ret < 0) {
        /*netlink_unicast发送失败时会释放内存，外面无需释放*/
        //kfree_skb(skb);
            return;
        }
    }
}

/**
 *
 * Device driver use this interface to report an event 
 * NOTICE: This interface may be called in interrupt or in process context
 *
 */
int at_device_event_report(void *data, int len) 
{
    struct at_device_event_node *node = NULL;

    //int myvalue = 100;

    if (unlikely(NULL == at_device_event_sock)) {
        return -1;
    }

    node = at_device_event_node_alloc(data, len);
    if (NULL == node) {
        return  -1;
    }

    at_device_event_enqueue(node);

    /*
     * If device driver report event before daemon was started,
     * Only put event into buffer queue.
     */
    if (likely(-1 != at_us_pid))
    {
        /*将工作任务加入到工作队列中*/
        queue_delayed_work(at_netlink_wq, &at_device_event_send_work, AT_DELAY_FOR_NL_SEND);
    }

    return 0;
}

EXPORT_SYMBOL_GPL(at_device_event_report);

// Testing code, to be deleted ...
#if 0

static int test_handler(void *data, int len)
{
    DEVICE_EVENT *event = (DEVICE_EVENT *)data;

    printk("*** test_handler: device_id = %d, event_code = %d, event_len = %d, data = %s\n", event->device_id, event->event_code, event->len, event->data);

    // Loop-back for APP test
    device_event_report(data, len);

    return 0;
}

#endif

static int __init at_device_event_init(void)
{

    struct netlink_kernel_cfg cfg = {
        .groups = 32,
        .input = at_device_event_input,
    };
    /*创建netlink用工作队列*/
    at_netlink_wq = create_workqueue("AT_NET_LINK_WQ");
    if( NULL == at_netlink_wq )
    {  
        printk(KERN_ALERT "kernel create NET_LINK_WQ workqueue failed.\n");
        return -EIO;
    }

    INIT_LIST_HEAD(&at_device_event_queue);
    INIT_DELAYED_WORK(&at_device_event_send_work, at_device_event_send);
    spin_lock_init(&at_queue_lock);
    
    // Create a socket
    at_device_event_sock = netlink_kernel_create(&init_net, NETLINK_ATNLPROXY, &cfg);
    if (!at_device_event_sock)
    {
        printk(KERN_ALERT "kernel create netlink failed.\n");
        return -EIO;
    }

    return 0;
}

static void __exit at_device_event_exit(void)
{
    cancel_delayed_work_sync(&at_device_event_send_work);

    /*清空工作队列并释放资源*/
    flush_workqueue(at_netlink_wq);
    destroy_workqueue(at_netlink_wq);
    at_netlink_wq = NULL;

    netlink_kernel_release(at_device_event_sock);
}

subsys_initcall(at_device_event_init);
module_exit(at_device_event_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dedong.yang");
MODULE_DESCRIPTION("Netlink module for kernel<->userspace messge.");

#endif
