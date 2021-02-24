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




#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/netlink.h>
#include <asm/uaccess.h>
#include <net/sock.h>
#if (FEATURE_ON == MBB_ATNLPROXY)
void output_nl_device_event(NL_DEVICE_EVENT *nlevent)
{
    
}



/*-------------------------------------------------
函数原型： static void atproxy_receive_from_app(struct sk_buff *__skb)；
描    述： 		此函数处理userspace下发的消息
输    入： 		封装了userspace消息体的sk_buff
输    出： 		无
返 回 值： 	无
--------------------------------------------------*/
extern int atnlproxy_register_to_kernel(void* buff, int ilength);
extern int   atnlproxy_dispatch_uspace_data(void* buffer, int ilength);
extern void aaa();
static int atproxy_receive_from_app(void *buffer, int length)
{
    char *atresponse = NULL;
    void* pVoid = NULL;

    NL_DEVICE_EVENT *pEvent = (NL_DEVICE_EVENT*)buffer;
    if(pEvent == NULL)
    {
        return -1;
    }

    output_nl_device_event(pEvent);

    //atresponse =  (char *)buffer + sizeof(NL_DEVICE_EVENT);
       
    /*连接内核的netlink并注册AT*/
    if(NL_CONNECT_TO_KERNEL == pEvent->event_code)
    {
        pVoid = (void*)pEvent->data;  
        atnlproxy_register_to_kernel(pVoid, pEvent->len);
    }
    /*所有的用户态的AT上报和AT应答都在这里处理*/
    else if(NL_NORMALRESPONSE == pEvent->event_code)
    {
        atnlproxy_dispatch_uspace_data((void *)pEvent->data, pEvent->len);
    }
    else
    {
    }

    return 0;

}


extern int at_device_event_handler_register(NL_DEVICE_ID id, device_event_dispatch dispatcher);
static void atproxy_kernel_exit(void)
{
    int ret;
    /* unregister event dispatcher first */
    ret = at_device_event_handler_register(NL_DEVICE_ID_ATNLPROXT, NULL);
    if (ret != 0)
    {
        printk(KERN_ERR"atproxy_kernel_exit:unregister event handler failed\r\n");
    }

    return;
}

/*-------------------------------------------------
Func:	atproxy_kernel_init
Desc:	init syswatch module
Para:	NA
return:	SYSWATCH_OK when init success
		SYSWATCH_ERROR when failed to alloc memory
--------------------------------------------------*/
static int  atproxy_kernel_init(void)
{

    /* register callback function to netlink module */
    if (0 != at_device_event_handler_register(NL_DEVICE_ID_ATNLPROXT, atproxy_receive_from_app))
    {
        printk(KERN_ERR"atproxy_kernel_init:register event handler failed\r\n");
        return -1;
    }

    return 0;
}

//EXPORT_SYMBOL(process_exit_monitor);

module_init(atproxy_kernel_init);
module_exit(atproxy_kernel_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("dedong.yang");
MODULE_DESCRIPTION("process monitor----report event when specified processes exit");
#endif
