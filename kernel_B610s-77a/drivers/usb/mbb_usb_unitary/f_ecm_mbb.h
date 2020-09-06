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


#ifndef _MBB_ECM_H
#define _MBB_ECM_H

#include "usb_platform_comm.h"
#include "u_ether.h"
#include "bsp_ncm.h"

/* ECM连接速度结构,ECM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF命令字对应参数结构体*/
typedef NCM_IOCTL_CONNECTION_SPEED_S ECM_IOCTL_CONNECTION_SPEED_S;

typedef struct ecm_vendor_ctx
{
    USB_BOOL connect;
    USB_BOOL flow_control;
    ECM_IOCTL_CONNECTION_SPEED_S speed;
    struct completion notify_done;  /* notify completion */
    USB_BOOL tx_task_run;               /* tx task run */
    struct task_struct *tx_task;    /* tx task */
    struct completion tx_task_stop; /* notify tx task stoped */
    struct sk_buff_head tx_frames;  /* hold tx frames from vendor */
    struct completion tx_kick;      /* kick tx task */
    USB_VOID *ecm_priv;                 /* hold f_ecm */
    USB_VOID *vendor_priv;              /* hold app_ctx */

    /* stat info */
    USB_ULONG stat_rx_total;
    USB_ULONG stat_rx_einval;
    USB_ULONG stat_tx_total;
    USB_ULONG stat_tx_cancel;
    USB_ULONG stat_tx_xmit;
    USB_ULONG stat_tx_xmit_fail;
    USB_ULONG stat_tx_post;
    USB_ULONG stat_tx_drop;
    USB_ULONG stat_tx_nochl;
    USB_ULONG stat_tx_nodev;
    USB_ULONG stat_notify_timeout;
}ecm_vendor_ctx_t;

USB_VOID ecm_free_ntbnode(ntb_node_t *ntb_node);
ntb_node_t *ecm_alloc_ntbnode(USB_UINT32 size, gfp_t gfp_msk);
USB_UINT32 ecm_get_ntbnode(tx_ctx_t *ctx, USB_UINT32 size, gfp_t gfp_msk);
USB_VOID ecm_put_ntbnode(tx_ctx_t *ctx);
USB_BOOL ecm_get_bypass_state(USB_VOID);

USB_VOID usb_ecm_init(USB_VOID);

#endif

