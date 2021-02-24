/*lint -save -e19 -e123 -e537 -e713*/
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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS"
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
#include <linux/usb/bsp_usb.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/module.h>
#include "usb_vendor.h"

typedef struct
{
    usb_enum_done_cb_t enum_done_cbs[USB_ENUM_DONE_CB_BOTTEM];
    USB_UDI_ENABLE_CB_T udi_enable_cb[USB_ENABLE_CB_MAX];
    USB_UDI_DISABLE_CB_T udi_disable_cb[USB_ENABLE_CB_MAX];
}USB_CTX_S;


static unsigned g_usb_enum_done_cur = 0;
static unsigned g_usb_enum_dis_cur = 0;

static struct notifier_block gs_adp_usb_nb;
static struct notifier_block *gs_adp_usb_nb_ptr = NULL;
static int g_usb_enum_done_notify_complete = 0;
static int g_usb_disable_notify_complete = 0;
static USB_CTX_S g_usb_ctx = {{0},{0},{0}};

USB_UDI_ENABLE_CB_T gs_usb_notifier = 0;


/********************************************************
函数说明：协议栈注册USB使能通知回调函数
函数功能:
输入参数：pFunc: USB使能回调函数指针
输出参数：无
输出参数：无
返回值  ：0：成功
          1：失败
********************************************************/
unsigned int BSP_USB_RegUdiEnableCB(USB_UDI_ENABLE_CB_T pFunc)
{
    if (g_usb_enum_done_cur >= USB_ENABLE_CB_MAX)
    {
        printk("mdrv_usb_reg_enablecb error:0x%x", (unsigned)pFunc);
        return (unsigned int)(-1);
    }

    g_usb_ctx.udi_enable_cb[g_usb_enum_done_cur] = pFunc;
    g_usb_enum_done_cur++;

	if (g_usb_enum_done_notify_complete)
    {
    	if (pFunc){
			gs_usb_notifier = pFunc;
        	pFunc();
    	}
    }
	gs_usb_notifier = 0;
    return 0;
}

/********************************************************
函数说明：协议栈注册USB去使能通知回调函数
函数功能:
输入参数：pFunc: USB去使能回调函数指针
输出参数：无
输出参数：无
返回值  ：0：成功
          1：失败
********************************************************/
unsigned int BSP_USB_RegUdiDisableCB(USB_UDI_DISABLE_CB_T pFunc)
{
    if (g_usb_enum_dis_cur >= USB_ENABLE_CB_MAX)
    {
        printk("mdrv_usb_reg_disablecb error:0x%x", (unsigned)pFunc);
        return (unsigned int)(-1);
    }

    g_usb_ctx.udi_disable_cb[g_usb_enum_dis_cur] = pFunc;
    g_usb_enum_dis_cur++;

    return 0;
}

void gs_usb_notifier_cb_show(void)
{
	if (gs_usb_notifier){
		printk("gs_usb_notifier : %pS \n", gs_usb_notifier);
	}else{
		printk("gs_usb_notifier : NULL \n");
	}		
}
EXPORT_SYMBOL(gs_usb_notifier_cb_show);

static int gs_usb_adp_notifier_cb(struct notifier_block *nb,
            unsigned long event, void *priv)
{
    int loop;

    switch (event) {

    case USB_BALONG_DEVICE_INSERT:
        g_usb_disable_notify_complete = 0;
        break;
    case USB_BALONG_ENUM_DONE:
        /* enum done */
        g_usb_disable_notify_complete = 0;
        if (!g_usb_enum_done_notify_complete) {
            for(loop = 0; loop < USB_ENUM_DONE_CB_BOTTEM; loop++)
            {
				if(g_usb_ctx.enum_done_cbs[loop]){
					gs_usb_notifier = g_usb_ctx.enum_done_cbs[loop];
                    g_usb_ctx.enum_done_cbs[loop]();
				}
				
            }

            for(loop = 0; loop < USB_ENABLE_CB_MAX; loop++)
            {
                if(g_usb_ctx.udi_enable_cb[loop]){
					gs_usb_notifier = g_usb_ctx.udi_enable_cb[loop];
                    g_usb_ctx.udi_enable_cb[loop]();
                }
            }
        }
        g_usb_enum_done_notify_complete = 1;
		gs_usb_notifier = 0;
        break;
    case USB_BALONG_DEVICE_DISABLE:
    case USB_BALONG_DEVICE_REMOVE:
        /* notify other cb */
        g_usb_enum_done_notify_complete = 0;
        if (!g_usb_disable_notify_complete) {
            for(loop = 0; loop < USB_ENABLE_CB_MAX; loop++)
            {
                if(g_usb_ctx.udi_disable_cb[loop])
                    g_usb_ctx.udi_disable_cb[loop]();
            }
            g_usb_disable_notify_complete = 1;
        }
        break;
    default:
        break;
    }
    return 0;
}

#ifdef CONFIG_USB_SUPPORT
int __init adp_usb_init(void)
{
    /* we just regist once, and don't unregist any more */
    if (!gs_adp_usb_nb_ptr) {
        gs_adp_usb_nb_ptr = &gs_adp_usb_nb;
        gs_adp_usb_nb.priority = USB_NOTIF_PRIO_ADP;
        gs_adp_usb_nb.notifier_call = gs_usb_adp_notifier_cb;
        bsp_usb_register_notify(gs_adp_usb_nb_ptr);
    }
    return 0;
}
module_init(adp_usb_init);
#endif
/*lint -restore*/
