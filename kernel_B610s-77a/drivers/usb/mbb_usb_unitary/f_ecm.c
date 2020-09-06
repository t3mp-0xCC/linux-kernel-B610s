/*
 * f_ecm.c -- USB CDC Ethernet (ECM) link function driver
 *
 * Copyright (C) 2003-2005,2008 David Brownell
 * Copyright (C) 2008 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
/*
 * 2015-05-19 - changed for huawei device Huawei Technologies. The contribution by Huawei Technologies is under GPLv2
 */


/* #define VERBOSE_DEBUG */

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/etherdevice.h>
#include <linux/kthread.h>
#include <linux/usb/cdc.h>
#include <net/ip.h>

#include "usb_platform_comm.h"
#include "ecm_balong.h"
#include "bsp_dump.h"
#include "f_ecm_mbb.h"
#include "adapt/hw_net_dev.h"
#include "hw_pnp.h"
#include "usb_event.h"
#include "u_ether.h"
#include "android.h"
#include "hw_pnp_api.h"

/*
 * This function is a "CDC Ethernet Networking Control Model" (CDC ECM)
 * Ethernet link.  The data transfer model is simple (packets sent and
 * received over bulk endpoints using normal short packet termination),
 * and the control model exposes various data and optional notifications.
 *
 * ECM is well standardized and (except for Microsoft) supported by most
 * operating systems with USB host support.  It's the preferred interop
 * solution for Ethernet over USB, at least for firmware based solutions.
 * (Hardware solutions tend to be more minimalist.)  A newer and simpler
 * "Ethernet Emulation Model" (CDC EEM) hasn't yet caught on.
 *
 * Note that ECM requires the use of "alternate settings" for its data
 * interface.  This means that the set_alt() method has real work to do,
 * and also means that a get_alt() method is required.
 */

#define DISCONNECT_TIME_DELAY               0

/* Recored the interface name read from nv 50xxx */
#define ECM_INTERFACE_NAME_LEN_MAX     64
static USB_CHAR gECMInterfaceName[ECM_INTERFACE_NAME_LEN_MAX + 1] = {0}; 
static struct f_ecm *the_ecm[GNET_MAX_NUM] = {0};
static USB_UINT32 gecm_link_idx = 0;
//static USB_UINT32 n_gnet_ports = GNET_USED_NUM;

static USB_BOOL g_ecmforce_bypass_mode = false;

typedef struct
{
    struct list_head    list;
    USB_UINT8            *buf;
    USB_UINT32            length;
    USB_INT32           send;
} ecm_resp_t;

typedef struct
{
    USB_VOID            (*resp_avail)(USB_VOID *v);
    USB_VOID            *v;
    spinlock_t          resp_lock;
    struct list_head    resp_queue;
    struct semaphore    sem;
    struct timer_list   timer;
} ecm_params_t;


enum ecm_notify_state {
    ECM_NOTIFY_NONE,        /* don't notify */
    ECM_NOTIFY_CONNECT,        /* issue CONNECT next */
    ECM_NOTIFY_SPEED,        /* issue SPEED_CHANGE next */
    ECM_NOTIFY_SET_HOST_TIMEOUT,
    ECM_NOTIFY_IPV6_DNS,
    ECM_NOTIFY_FLOW_CONTROL,
    ECM_NOTIFY_RESPONSE_AVAI,  /* response available notify */
};

typedef enum _ecm_work_mode{
    ECM_WORK_MODE_NET_BRIDGE,     /* 网桥模式，双接口，用于hilink对接路由*/
    ECM_WORK_MODE_BYPASS,         /* 旁路ECM ,双接口，对接协议栈*/    
    ECM_WORK_MODE_INVALID = 0xFFFFFFFF,
}ecm_work_mode;

/* 检查ECM是否处于BYPASS模式(非网桥模式) */
#define VERIFY_ECM_BYPASS(_ecm)     (ECM_WORK_MODE_BYPASS == _ecm->work_mode)


struct f_ecm {
    struct gether            port;
    USB_UINT8                ctrl_id, data_id;

    USB_CHAR                 ethaddr[14];

    struct usb_ep           *notify;
    struct usb_request      *notify_req;
    struct usb_request      *notify_req_rec;
    USB_UINT8                notify_state;
    /*
     * for notification, it is accessed from both
     * callback and ethernet open/close
     */
    bool                    is_open;
    USB_UINT32              curr_alt;
    
    /* FIXME is_open needs some irq-ish locking
     * ... possibly the same as port.ioport
     */

    spinlock_t              lock;
    bool                    is_single_interface;
    ecm_work_mode           work_mode;
    struct delayed_work      notify_work;
    USB_UINT8               queued_notify_state;
    USB_UINT8               connect_notify_blocked;
    struct delayed_work     disconnect_work;
    ecm_params_t            params;
    ecm_vendor_ctx_t        vendor_ctx;
    USB_BOOL                EncapRspBlocked;

};

extern netdev_tx_t eth_vendor_start_xmit(struct sk_buff *skb,
                    struct net_device *net);
extern USB_INT32 ecm_vendor_submit_skb(USB_VOID *app_ctx, struct sk_buff *skb);


USB_VOID ecm_data_disconnect (struct work_struct* w)
{
    struct f_ecm *ecm = container_of(w, struct f_ecm, disconnect_work.work);
    ecm_app_ctx_t *app_ctx = NULL;
    /*编程规范covertiy 认为此判断不会进，没有意义*/
    if (NULL == ecm)
    {
        DBG_E(MBB_ECM, "ecm is empty\n");
        return;
    }
    app_ctx = (ecm_app_ctx_t *)ecm->vendor_ctx.vendor_priv;

    if(app_ctx
    && app_ctx->bDataConnect
    && app_ctx->cbs.ndis_stat_change_cb)
    {
        app_ctx->cbs.ndis_stat_change_cb(ECM_IOCTL_STUS_BREAK, NULL);
    }
}


USB_BOOL ecm_get_bypass_state(USB_VOID)
{
    USB_BOOL bRet = false;
    usb_pnp_api_hanlder *pnp_api_handler = pnp_adp_get_api_handler();
    USB_UINT8 pc_type = OS_FLAG_INVALID;
    
    if(pnp_api_handler->pnp_system_type_get_cb)
    {
        pc_type = pnp_api_handler->pnp_system_type_get_cb();
    }
    
    bRet = (GATEWAY_OS_FLAG == pc_type);  /* 在网关下上报bypass模式网口 */

    if (g_ecmforce_bypass_mode)
    {
        bRet = true;
    }

#if defined(USB_SOLUTION)
    bRet = true;
#endif

    DBG_T(MBB_PNP, "ecm bypass state = %u, is force=%u\n", bRet, g_ecmforce_bypass_mode);
    return bRet;
}

USB_INT ecm_set_bypass_state(USB_INT mode)
{
    g_ecmforce_bypass_mode = (0 != mode);
    return mode;
}


static inline struct f_ecm *func_to_ecm(struct usb_function *f)
{
    return container_of(f, struct f_ecm, port.func);
}

/* peak (theoretical) bulk transfer rate in bits-per-second */
static inline USB_UINT32 ecm_bitrate(struct usb_gadget *g)
{
    USB_UINT32 speed = eth_get_net_speed();
    if (0 < speed)
    {
        return speed;
    }

    return 300 * 1000 * 1000;   /* V7R2的理论峰值速率为300M */
}

/*-------------------------------------------------------------------------*/

/*
 * Include the status endpoint if we can, even though it's optional.
 *
 * Use wMaxPacketSize big enough to fit CDC_NOTIFY_SPEED_CHANGE in one
 * packet, to simplify cancellation; and a big transfer interval, to
 * waste less bandwidth.
 *
 * Some drivers (like Linux 2.4 cdc-ether!) "need" it to exist even
 * if they ignore the connect/disconnect notifications that real aether
 * can provide.  More advanced cdc configurations might want to support
 * encapsulated commands (vendor-specific, using control-OUT).
 */

#define LOG2_STATUS_INTERVAL_MSEC    5    /* 1 << 5 == 32 msec */
#define ECM_STATUS_BYTECOUNT        16    /* 8 byte header + data */


/* interface descriptor: */

static struct usb_interface_assoc_descriptor ecm_iad_descriptor = {
    .bLength =        sizeof ecm_iad_descriptor,
    .bDescriptorType =    USB_DT_INTERFACE_ASSOCIATION,

    /* .bFirstInterface =    DYNAMIC, */
    .bInterfaceCount =    2,    /* control + data */
    .bFunctionClass =    USB_CLASS_COMM,
    .bFunctionSubClass =    USB_CDC_SUBCLASS_ETHERNET,
    .bFunctionProtocol =    USB_CDC_PROTO_NONE,
    /* .iFunction =        DYNAMIC */
};


static struct usb_interface_descriptor ecm_control_intf = {
    .bLength =        sizeof ecm_control_intf,
    .bDescriptorType =    USB_DT_INTERFACE,

    /* .bInterfaceNumber = DYNAMIC */
    /* status endpoint is optional; this could be patched later */
    .bAlternateSetting =    0,
    .bNumEndpoints =    1,
    .bInterfaceClass =    USB_CLASS_COMM,
    .bInterfaceSubClass =    USB_CDC_SUBCLASS_ETHERNET,
    .bInterfaceProtocol =    USB_CDC_PROTO_NONE,
    /* .iInterface = DYNAMIC */
};

static struct usb_cdc_header_desc ecm_header_desc = {
    .bLength =        sizeof ecm_header_desc,
    .bDescriptorType =    USB_DT_CS_INTERFACE,
    .bDescriptorSubType =    USB_CDC_HEADER_TYPE,

    .bcdCDC =        cpu_to_le16(0x0110),
};

static struct usb_cdc_union_desc ecm_union_desc = {
    .bLength =        sizeof(ecm_union_desc),
    .bDescriptorType =    USB_DT_CS_INTERFACE,
    .bDescriptorSubType =    USB_CDC_UNION_TYPE,
    /* .bMasterInterface0 =    DYNAMIC */
    /* .bSlaveInterface0 =    DYNAMIC */
};

static struct usb_cdc_ether_desc ecm_desc = {
    .bLength =        sizeof ecm_desc,
    .bDescriptorType =    USB_DT_CS_INTERFACE,
    .bDescriptorSubType =    USB_CDC_ETHERNET_TYPE,

    /* this descriptor actually adds value, surprise! */
    /* .iMACAddress = DYNAMIC */
    .bmEthernetStatistics =    cpu_to_le32(0), /* no statistics */
    .wMaxSegmentSize =    cpu_to_le16(ETH_FRAME_LEN),
    .wNumberMCFilters =    cpu_to_le16(0),
    .bNumberPowerFilters =    0,
};

/* the default data interface has no endpoints ... */

static struct usb_interface_descriptor ecm_data_nop_intf = {
    .bLength =        sizeof ecm_data_nop_intf,
    .bDescriptorType =    USB_DT_INTERFACE,

    .bInterfaceNumber =    1,
    .bAlternateSetting =    0,
    .bNumEndpoints =    0,
    .bInterfaceClass =    USB_CLASS_CDC_DATA,
    .bInterfaceSubClass =    USB_CDC_SUBCLASS_ETHERNET,
    .bInterfaceProtocol =    0,
    /* .iInterface = DYNAMIC */
};

/* ... but the "real" data interface has two bulk endpoints */

static struct usb_interface_descriptor ecm_data_intf = {
    .bLength =        sizeof ecm_data_intf,
    .bDescriptorType =    USB_DT_INTERFACE,

    .bInterfaceNumber =    1,
    .bAlternateSetting =    0,
    .bNumEndpoints =    2,
    .bInterfaceClass =    USB_CLASS_CDC_DATA,
    .bInterfaceSubClass =    USB_CDC_SUBCLASS_ETHERNET,
    .bInterfaceProtocol =    0,
    /* .iInterface = DYNAMIC */
};

/* full speed support: */

static struct usb_endpoint_descriptor fs_ecm_notify_desc = {
    .bLength =        USB_DT_ENDPOINT_SIZE,
    .bDescriptorType =    USB_DT_ENDPOINT,

    .bEndpointAddress =    USB_DIR_IN,
    .bmAttributes =        USB_ENDPOINT_XFER_INT,
    .wMaxPacketSize =    cpu_to_le16(ECM_STATUS_BYTECOUNT),
    .bInterval =        1 << LOG2_STATUS_INTERVAL_MSEC,
};

static struct usb_endpoint_descriptor fs_ecm_in_desc = {
    .bLength =        USB_DT_ENDPOINT_SIZE,
    .bDescriptorType =    USB_DT_ENDPOINT,

    .bEndpointAddress =    USB_DIR_IN,
    .bmAttributes =        USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor fs_ecm_out_desc = {
    .bLength =        USB_DT_ENDPOINT_SIZE,
    .bDescriptorType =    USB_DT_ENDPOINT,

    .bEndpointAddress =    USB_DIR_OUT,
    .bmAttributes =        USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *ecm_fs_function[] = {
    /* CDC ECM control descriptors */
//#ifndef GNET_SINGLE_INTERFACE
//    (struct usb_descriptor_header *) &ecm_iad_descriptor,
//#endif
    (struct usb_descriptor_header *) &ecm_control_intf,
    (struct usb_descriptor_header *) &ecm_header_desc,
    (struct usb_descriptor_header *) &ecm_union_desc,
    (struct usb_descriptor_header *) &ecm_desc,

    /* NOTE: status endpoint might need to be removed */
    (struct usb_descriptor_header *) &fs_ecm_notify_desc,

    /* data interface, altsettings 0 and 1 */
//#ifndef GNET_SINGLE_INTERFACE
//    (struct usb_descriptor_header *) &ecm_data_nop_intf,
//#endif
    (struct usb_descriptor_header *) &ecm_data_intf,
    (struct usb_descriptor_header *) &fs_ecm_in_desc,
    (struct usb_descriptor_header *) &fs_ecm_out_desc,
//    (struct usb_descriptor_header *) &fs_ecm_notify_desc,
    NULL,
};

/* high speed support: */

static struct usb_endpoint_descriptor hs_ecm_notify_desc = {
    .bLength =        USB_DT_ENDPOINT_SIZE,
    .bDescriptorType =    USB_DT_ENDPOINT,

    .bEndpointAddress =    USB_DIR_IN,
    .bmAttributes =        USB_ENDPOINT_XFER_INT,
    .wMaxPacketSize =    cpu_to_le16(ECM_STATUS_BYTECOUNT),
    .bInterval =        LOG2_STATUS_INTERVAL_MSEC,
};

static struct usb_endpoint_descriptor hs_ecm_in_desc = {
    .bLength =        USB_DT_ENDPOINT_SIZE,
    .bDescriptorType =    USB_DT_ENDPOINT,

    .bEndpointAddress =    USB_DIR_IN,
    .bmAttributes =        USB_ENDPOINT_XFER_BULK,
    .wMaxPacketSize =    cpu_to_le16(512),
};

static struct usb_endpoint_descriptor hs_ecm_out_desc = {
    .bLength =        USB_DT_ENDPOINT_SIZE,
    .bDescriptorType =    USB_DT_ENDPOINT,

    .bEndpointAddress =    USB_DIR_OUT,
    .bmAttributes =        USB_ENDPOINT_XFER_BULK,
    .wMaxPacketSize =    cpu_to_le16(512),
};

static struct usb_descriptor_header *ecm_hs_function[] = {
    /* CDC ECM control descriptors */
//#ifndef GNET_SINGLE_INTERFACE
//    (struct usb_descriptor_header *) &ecm_iad_descriptor,
//#endif
    (struct usb_descriptor_header *) &ecm_control_intf,
    (struct usb_descriptor_header *) &ecm_header_desc,
    (struct usb_descriptor_header *) &ecm_union_desc,
    (struct usb_descriptor_header *) &ecm_desc,

    /* NOTE: status endpoint might need to be removed */
    (struct usb_descriptor_header *) &hs_ecm_notify_desc,

    /* data interface, altsettings 0 and 1 */
//#ifndef GNET_SINGLE_INTERFACE
//    (struct usb_descriptor_header *) &ecm_data_nop_intf,
//#endif
    (struct usb_descriptor_header *) &ecm_data_intf,
    (struct usb_descriptor_header *) &hs_ecm_in_desc,
    (struct usb_descriptor_header *) &hs_ecm_out_desc,
//    (struct usb_descriptor_header *) &hs_ecm_notify_desc,
    NULL,
};

/* super speed support: */

static struct usb_endpoint_descriptor ss_ecm_notify_desc = {
    .bLength =        USB_DT_ENDPOINT_SIZE,
    .bDescriptorType =    USB_DT_ENDPOINT,

    .bEndpointAddress =    USB_DIR_IN,
    .bmAttributes =        USB_ENDPOINT_XFER_INT,
    .wMaxPacketSize =    cpu_to_le16(ECM_STATUS_BYTECOUNT),
    .bInterval =        LOG2_STATUS_INTERVAL_MSEC,
};

static struct usb_ss_ep_comp_descriptor ss_ecm_intr_comp_desc = {
    .bLength =        sizeof ss_ecm_intr_comp_desc,
    .bDescriptorType =    USB_DT_SS_ENDPOINT_COMP,

    /* the following 3 values can be tweaked if necessary */
    /* .bMaxBurst =        0, */
    /* .bmAttributes =    0, */
    .wBytesPerInterval =    cpu_to_le16(ECM_STATUS_BYTECOUNT),
};

static struct usb_endpoint_descriptor ss_ecm_in_desc = {
    .bLength =        USB_DT_ENDPOINT_SIZE,
    .bDescriptorType =    USB_DT_ENDPOINT,

    .bEndpointAddress =    USB_DIR_IN,
    .bmAttributes =        USB_ENDPOINT_XFER_BULK,
    .wMaxPacketSize =    cpu_to_le16(1024),
};

static struct usb_endpoint_descriptor ss_ecm_out_desc = {
    .bLength =        USB_DT_ENDPOINT_SIZE,
    .bDescriptorType =    USB_DT_ENDPOINT,

    .bEndpointAddress =    USB_DIR_OUT,
    .bmAttributes =        USB_ENDPOINT_XFER_BULK,
    .wMaxPacketSize =    cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor ss_ecm_bulk_comp_desc = {
    .bLength =        sizeof ss_ecm_bulk_comp_desc,
    .bDescriptorType =    USB_DT_SS_ENDPOINT_COMP,

    /* the following 2 values can be tweaked if necessary */
    /* .bMaxBurst =        0, */
    /* .bmAttributes =    0, */
};

static struct usb_descriptor_header *ecm_ss_function[] = {
    /* CDC ECM control descriptors */
#ifndef GNET_SINGLE_INTERFACE
    (struct usb_descriptor_header *) &ecm_iad_descriptor,
#endif
    (struct usb_descriptor_header *) &ecm_control_intf,
    (struct usb_descriptor_header *) &ecm_header_desc,
    (struct usb_descriptor_header *) &ecm_union_desc,
    (struct usb_descriptor_header *) &ecm_desc,

    /* NOTE: status endpoint might need to be removed */
    (struct usb_descriptor_header *) &ss_ecm_notify_desc,
    (struct usb_descriptor_header *) &ss_ecm_intr_comp_desc,

    /* data interface, altsettings 0 and 1 */
#ifndef GNET_SINGLE_INTERFACE
    (struct usb_descriptor_header *) &ecm_data_nop_intf,
#endif
    (struct usb_descriptor_header *) &ecm_data_intf,
    (struct usb_descriptor_header *) &ss_ecm_in_desc,
    (struct usb_descriptor_header *) &ss_ecm_bulk_comp_desc,
    (struct usb_descriptor_header *) &ss_ecm_out_desc,
    (struct usb_descriptor_header *) &ss_ecm_bulk_comp_desc,
    (struct usb_descriptor_header *) &ss_ecm_notify_desc,
    (struct usb_descriptor_header *) &ss_ecm_intr_comp_desc,
    NULL,
};

/* string descriptors: */
#define ECM_CTRL_IDX        (0)
#define ECM_DATA_IDX        (1)
#define ECM_IAD_IDX         (2)
#define ECM_MAC_IDX_BASE    (3)
#define ECM_MAC0_IDX        (3)
#define ECM_MAC1_IDX        (4)
/*MAC address 第一字节为02表示usb网卡"021E101F0000"，
在Linux系统上，显示为usb0和usb1两个网卡设备；
在QNX系统上，显示为usbecm0和usbecm1两个网卡设备*/

static struct usb_string ecm_string_defs[] = {
    [ECM_CTRL_IDX].s = "CDC Ethernet Control Model (ECM)",
    [ECM_DATA_IDX].s = "CDC Ethernet Data",
    [ECM_IAD_IDX].s = "CDC ECM",
    [ECM_MAC0_IDX].s = "021E101F0000" /* DYNAMIC MAC address */,
    [ECM_MAC1_IDX].s = "021E101F0001" /* DYNAMIC MAC address */,
    {  } /* end of list */
};

static struct usb_gadget_strings ecm_string_table = {
    .language =        0x0409,    /* en-us */
    .strings =        ecm_string_defs,
};

static struct usb_gadget_strings *ecm_strings[] = {
    &ecm_string_table,
    NULL,
};

/*-------------------------------------------------------------------------*/

static USB_VOID ecm_do_notify(struct f_ecm *ecm)
{
    struct usb_request        *req = ecm->notify_req;
    struct usb_cdc_notification    *event;
    struct usb_composite_dev    *cdev = ecm->port.func.config->cdev;
    __le32                 *data;
    USB_INT32               status;
    struct eth_dev *dev = ecm->port.ioport;

    /*目前ECM 不需要获取at返回值，仅上报速率与连接状态 */
    if ((ECM_NOTIFY_CONNECT != ecm->notify_state)
        && (ECM_NOTIFY_SPEED != ecm->notify_state))
    {
        ecm->notify_state = ECM_NOTIFY_NONE;
        return;
    }
    
    /* notification already in flight? */
    if (!req)
    {
        return;
    }

    /* ecm not enable, reset the notify status */
    if (!dev)
    {
        ecm->notify_state = ECM_NOTIFY_NONE;
        return;
    }

    event = req->buf;
    switch (ecm->notify_state) {
    case ECM_NOTIFY_NONE:
        return;

    case ECM_NOTIFY_CONNECT:
        event->bNotificationType = USB_CDC_NOTIFY_NETWORK_CONNECTION;

        if (VERIFY_ECM_BYPASS(ecm))
        {
            event->wValue = cpu_to_le16(ecm->vendor_ctx.connect);
        }
        else
        {
            if (ecm->is_open)
            {
                event->wValue = cpu_to_le16(1);
            }
            else
            {
                event->wValue = cpu_to_le16(0);
            }
        }

        event->wLength = 0;
        req->length = sizeof *event;

        DBG_I(MBB_ECM, "notify connect %s\n",
                ecm->is_open ? "true" : "false");
        ecm->notify_state = ECM_NOTIFY_SPEED;
        break;

    case ECM_NOTIFY_SPEED:
        event->bNotificationType = USB_CDC_NOTIFY_SPEED_CHANGE;
        event->wValue = cpu_to_le16(0);
        event->wLength = cpu_to_le16(8);
        req->length = ECM_STATUS_BYTECOUNT;

        /* SPEED_CHANGE data is up/down speeds in bits/sec */
        data = req->buf + sizeof *event;

        if (VERIFY_ECM_BYPASS(ecm))
        {
            data[0] = cpu_to_le32(ecm->vendor_ctx.speed.u32DownBitRate);
            data[1] = cpu_to_le32(ecm->vendor_ctx.speed.u32UpBitRate);
            ecm->notify_state = ECM_NOTIFY_NONE;
            DBG_I(MBB_ECM, "notify speed [%d:%d]\n",
                ecm->vendor_ctx.speed.u32DownBitRate,
                ecm->vendor_ctx.speed.u32UpBitRate);
        }
        else
        {
            data[0] = cpu_to_le32(ecm_bitrate(cdev->gadget));
            data[1] = data[0];

            DBG_I(MBB_ECM, "notify speed %ld\n", ecm_bitrate(cdev->gadget));
        }

        break;

    }
    event->bmRequestType = 0xA1;
    event->wIndex = cpu_to_le16(ecm->ctrl_id);

    /* check whether the gadget is suspended.
       if so, try to wake up the host */
    if (unlikely(dev->is_suspend)) {
        if (gnet_wakeup_gadget(dev)) {
            /* wakeup failed, reset the notify state */
            ecm->notify_state = ECM_NOTIFY_NONE;
            return;
        }
    }

    ecm->notify_req = NULL;
    status = usb_ep_queue(ecm->notify, req, GFP_ATOMIC);
    if (status < 0) {
        ecm->notify_req = req;
        DBG_E(MBB_ECM, "notify --> %ld\n", status);
    }
}

static USB_VOID ecm_notify(struct f_ecm *ecm)
{
    /* NOTE on most versions of Linux, host side cdc-ethernet
     * won't listen for notifications until its netdevice opens.
     * The first notification then sits in the FIFO for a long
     * time, and the second one is queued.
     */
    ecm->notify_state = ECM_NOTIFY_CONNECT;
    ecm_do_notify(ecm);
}

static USB_VOID ecm_notify_work (struct work_struct* w)
{
    struct f_ecm *ecm = container_of(w, struct f_ecm, notify_work.work);

    DBG_E(MBB_ECM, "enter\n");
    
    if (NULL == ecm)
    {
        DBG_E(MBB_ECM, "ecm is empty\n");
        return;
    }

    ecm_notify(ecm);
}

static USB_VOID ecm_notify_complete(struct usb_ep *ep, struct usb_request *req)
{
    struct f_ecm            *ecm = req->context;
    struct usb_cdc_notification    *event = req->buf;

    switch (req->status) {
    case 0:
        /* no fault */
        break;
    case -ECONNRESET:
    case -ESHUTDOWN:
        ecm->notify_state = ECM_NOTIFY_NONE;
        break;
    default:
        DBG_I(MBB_ECM, "event %02x --> %d\n",
            event->bNotificationType, req->status);
        break;
    }
    ecm->notify_req = req;
    ecm_do_notify(ecm);
    if (VERIFY_ECM_BYPASS(ecm))
    {
        complete(&(ecm->vendor_ctx.notify_done));
    }

}

static USB_INT ecm_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
    struct f_ecm        *ecm = func_to_ecm(f);
    struct usb_composite_dev *cdev = f->config->cdev;
    struct usb_request    *req = cdev->req;
    USB_INT             value = -EOPNOTSUPP;
    USB_UINT16            w_index = le16_to_cpu(ctrl->wIndex);
    USB_UINT16            w_value = le16_to_cpu(ctrl->wValue);
    USB_UINT16            w_length = le16_to_cpu(ctrl->wLength);

    /* composite driver infrastructure handles everything except
     * CDC class messages; interface activation uses set_alt().
     */
    switch ((ctrl->bRequestType << 8) | ctrl->bRequest) {
    case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
            | USB_CDC_SET_ETHERNET_PACKET_FILTER:
        /* see 6.2.30: no data, wIndex = interface,
         * wValue = packet filter bitmap
         */
        if (w_length != 0 || w_index != ecm->ctrl_id)
            goto invalid;
        DBG_I(MBB_ECM, "packet filter %02x\n", w_value);
        /* REVISIT locking of cdc_filter.  This assumes the UDC
         * driver won't have a concurrent packet TX irq running on
         * another CPU; or that if it does, this write is atomic...
         */
        ecm->port.cdc_filter = w_value;
        value = 0;
        break;

    /* and optionally:
     * case USB_CDC_SEND_ENCAPSULATED_COMMAND:
     * case USB_CDC_GET_ENCAPSULATED_RESPONSE:
     * case USB_CDC_SET_ETHERNET_MULTICAST_FILTERS:
     * case USB_CDC_SET_ETHERNET_PM_PATTERN_FILTER:
     * case USB_CDC_GET_ETHERNET_PM_PATTERN_FILTER:
     * case USB_CDC_GET_ETHERNET_STATISTIC:
     */

    default:
invalid:
        DBG_E(MBB_ECM, "invalid control req%02x.%02x v%04x i%04x l%d\n",
            ctrl->bRequestType, ctrl->bRequest,
            w_value, w_index, w_length);
    }

    /* respond with data transfer or status phase? */
    if (value >= 0) {
        DBG_I(MBB_ECM, "ecm req%02x.%02x v%04x i%04x l%d\n",
            ctrl->bRequestType, ctrl->bRequest,
            w_value, w_index, w_length);
        req->zero = 0;
        req->length = value;
        value = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
        if (value < 0)
            DBG_E(MBB_ECM, "ecm req %02x.%02x response err %d\n",
                    ctrl->bRequestType, ctrl->bRequest,
                    value);
    }

    /* device either stalls (value < 0) or reports success */
    return value;
}

USB_INT32 ecm_vendor_notify(USB_VOID *app_ecm, USB_UINT32 cmd, USB_VOID *param)
{
    struct f_ecm *ecm = (struct f_ecm *)app_ecm;
    USB_INT32 ret = 0;
    USB_ULONG flags = 0;
    
    DBG_T(MBB_ECM, "cmd=%u\n", cmd);

    if ((NULL == ecm) || (!VERIFY_ECM_BYPASS(ecm)))
    {
        return -EIO;
    }
    
    switch (cmd)
    {
        case ECM_IOCTL_NETWORK_CONNECTION_NOTIF :
        {
            ecm->vendor_ctx.connect = (0 != (*(USB_UINT32 *)param));
            ecm->notify_state = ECM_NOTIFY_CONNECT;

            if (!ecm->notify_req && ecm->notify_req_rec)
            {
                DBG_T(MBB_ECM, "notify is busy, sending connect notify in work\n"); 
                usb_ep_dequeue(ecm->notify, ecm->notify_req_rec);
                ecm->queued_notify_state = ECM_NOTIFY_CONNECT;
                adp_usb_queue_delay_work(&(ecm->notify_work), 0);
                return 0;
            }

            break;
        }

        case ECM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF :
        {
            ECM_IOCTL_CONNECTION_SPEED_S *speed = (ECM_IOCTL_CONNECTION_SPEED_S *)param;
            ecm->vendor_ctx.speed.u32UpBitRate = speed->u32UpBitRate;
            ecm->vendor_ctx.speed.u32DownBitRate = speed->u32DownBitRate;
            ecm->notify_state = ECM_NOTIFY_SPEED;      
            break;
        }

        default :
        {
            DBG_E(MBB_ECM, "%s : invalid cmd %u\n", __FUNCTION__, cmd);
            return -EINVAL;
        }
    }

    spin_lock_irqsave(&ecm->lock, flags);
    ecm_do_notify(ecm);
    spin_unlock_irqrestore(&ecm->lock, flags);

    ret = wait_for_completion_interruptible_timeout(&ecm->vendor_ctx.notify_done, 1*HZ);
    if(0 >= ret)
    {
        DBG_E(MBB_ECM, "wait for notify_done failed: ret=%d, cmd=%u\n", ret, cmd);
        ecm->vendor_ctx.stat_notify_timeout++;
    }

    return 0;
}



USB_INT32 ecm_vendor_post_skb(USB_VOID *vendor_ecm, struct sk_buff *skb)
{
    struct f_ecm *ecm = (struct f_ecm *)vendor_ecm;
    struct eth_dev  *dev = NULL;
    ecm_app_ctx_t *app_ctx = NULL;
    netdev_tx_t  tx_ret = NETDEV_TX_OK;

    DBG_I(MBB_ECM, "ecm_vendor_post_skb():line %d:enter \n",__LINE__);

    if(!ecm)
        return -ENOMEM;

    if (!VERIFY_ECM_BYPASS(ecm))
    {
        return -EIO;
    }
    
    if(gnet_dbg_level & GNET_DBG_TX2TASK)
    {
        if(skb_queue_len(&ecm->vendor_ctx.tx_frames) >= 1000)
        {
            ecm->vendor_ctx.stat_tx_drop++;
            return -EIO;
        }

        skb_queue_tail(&ecm->vendor_ctx.tx_frames, skb);
        ecm->vendor_ctx.stat_tx_post++;
        complete(&ecm->vendor_ctx.tx_kick);

        return 0;
    }

    app_ctx = (ecm_app_ctx_t *)ecm->vendor_ctx.vendor_priv;
    dev = ecm->port.ioport;
    if(!dev)
    {
        ecm->vendor_ctx.stat_tx_nodev++;
        return -ENXIO;
    }
    ecm->vendor_ctx.stat_tx_post++;
    if(skb)
    {
        ecm->vendor_ctx.stat_tx_total++;
        if (true != app_ctx->bDataConnect)
        {
            ecm->vendor_ctx.stat_tx_cancel++;
            dev_kfree_skb_any(skb);
        }
        else
        {
            ecm->vendor_ctx.stat_tx_xmit++;
            tx_ret = eth_vendor_start_xmit(skb, dev->net);
            if (NETDEV_TX_OK != tx_ret)
            {
                ecm->vendor_ctx.stat_tx_xmit_fail++;
                return tx_ret;
            }
        }
    }

    return 0;
}
static USB_INT32 ecm_vendor_rx(struct gether *port, struct sk_buff *skb)
{
    struct f_ecm  *ecm = func_to_ecm(&port->func);
    DBG_I(MBB_ECM, "enter,port(%p)skb(%p)skb->data(%p),skb->len(%d)\n", port, skb,skb->data,skb->len);
//    usb_MemHexDump((unsigned char*)skb->data, (skb->len >= 128)?128:skb->len);
    if (!ecm) 
    {
        DBG_E(MBB_ECM,"ecm is null");
        return -ENOMEM;
    }

    if(!ecm->vendor_ctx.vendor_priv)
    {
        DBG_E(MBB_ECM,"ecm->vendor_ctx.vendor_priv is null");
        ecm->vendor_ctx.stat_rx_einval++;
        return -ENOMEM;
    }

    ecm->vendor_ctx.stat_rx_total++;
    
    return ecm_vendor_submit_skb((ecm_app_ctx_t *)ecm->vendor_ctx.vendor_priv, skb);
}

static USB_INT32 ecm_vendor_tx_thread(USB_VOID *param)
{
    struct f_ecm *ecm = (struct f_ecm *)param;
    struct eth_dev  *dev = NULL;
    struct sk_buff *skb = NULL;
    ecm_app_ctx_t *app_ctx = NULL;
    netdev_tx_t  tx_ret = NETDEV_TX_OK;

    GNET_TRACE(("ecm vendor tx thread enter.\n"));

    if (!ecm || !ecm->vendor_ctx.vendor_priv)
    {
        GNET_ERR(("ecm vendor tx thread abnormal quit!\n"));
        return -ENODEV;
    }

    app_ctx = (ecm_app_ctx_t *)ecm->vendor_ctx.vendor_priv;
    dev = ecm->port.ioport;
    if(!dev)
    {
        GNET_ERR(("no eth_dev,ecm vendor tx thread abnormal quit!\n"));
        return -ENODEV;
    }

    ecm->vendor_ctx.tx_task_run = true;
    while (ecm->vendor_ctx.tx_task_run)
    {
        wait_for_completion(&ecm->vendor_ctx.tx_kick);

        do{
            skb = skb_dequeue(&ecm->vendor_ctx.tx_frames);
            if(skb)
            {
                ecm->vendor_ctx.stat_tx_total++;
                if (true != app_ctx->bDataConnect)
                {
                    ecm->vendor_ctx.stat_tx_cancel++;
                    dev_kfree_skb_any(skb);
                }
                else
                {
                    ecm->vendor_ctx.stat_tx_xmit++;
                    tx_ret = eth_vendor_start_xmit(skb, dev->net);
                    if (NETDEV_TX_OK != tx_ret)
                    {
                        ecm->vendor_ctx.stat_tx_xmit_fail++;
                        GNET_ERR(("vendor xmit skb failed\n"));
                    }
                }
            }
        }while(skb);
    }

    complete(&ecm->vendor_ctx.tx_task_stop);
    GNET_TRACE(("ecm vendor tx thread exit.\n"));
    return 0;
}

static USB_INT ecm_vendor_tx_thread_init(struct f_ecm *ecm)
{
    DBG_I(MBB_ECM,"enter.\n");

    if (!ecm)
    {
        return -ENOMEM;
    }

    if (ecm->vendor_ctx.tx_task_run)
    {
        return 0;
    }

    //skb_queue_head_init(&ecm->vendor_ctx.tx_frames);
    //init_completion(&ecm->vendor_ctx.tx_kick);
    //init_completion(&ecm->vendor_ctx.tx_task_stop);
    init_completion(&ecm->vendor_ctx.notify_done);

    /* move tx_task_run flag in thread, if the thread is wakeup after set_alt 1
       if clean up before set_alt 1, thread_uninit will wait for ever */
    //ecm->vendor_ctx.tx_task_run = true;
/*
    ecm->vendor_ctx.tx_task = kthread_create(ecm_vendor_tx_thread, ecm,
                   "ecm_vendor_tx_thread");
    if (IS_ERR(ecm->vendor_ctx.tx_task)) {
        GNET_ERR(("create kthread ecm_tx_thread failed!\n"));
        ecm->vendor_ctx.tx_task_run = false;
        return (USB_INT32)PTR_ERR(ecm->vendor_ctx.tx_task);
    }
// */
    DBG_I(MBB_ECM,"line %d:leave\n",__LINE__);

    return 0;
}

static USB_VOID ecm_vendor_tx_thead_stop(struct f_ecm *ecm)
{
    if (ecm->vendor_ctx.tx_task) {
        kthread_stop(ecm->vendor_ctx.tx_task);
        ecm->vendor_ctx.tx_task = NULL;
    }
}

static USB_INT32 ecm_vendor_tx_thread_uninit(struct f_ecm *ecm)
{
    struct sk_buff *skb = NULL;

    if (!ecm)
    {
        return -ENOMEM;
    }

    if (!ecm->vendor_ctx.tx_task_run)
    {
        /* if ecm is disable, the thread will not wakeup,
         * stop it here.
         */
        ecm_vendor_tx_thead_stop(ecm);
        return 0;
    }

    ecm->vendor_ctx.tx_task_run = false;

    complete(&ecm->vendor_ctx.tx_kick);
    wait_for_completion(&ecm->vendor_ctx.tx_task_stop);

    do{
        skb = skb_dequeue(&ecm->vendor_ctx.tx_frames);
        if(skb)
        {
            kfree_skb(skb);
        }
    }while(skb);

    DBG_I(MBB_ECM, "ecm_vendor_tx_thread_uninit ok\n");
    return 0;
}

USB_VOID ecm_params_init(struct f_ecm *ecm)
{
    DBG_I(MBB_ECM, "%s:enter\n",__FUNCTION__);

    spin_lock_init(&(ecm->params.resp_lock));
    INIT_LIST_HEAD(&(ecm->params.resp_queue));
    sema_init(&(ecm->params.sem), 0);

    /* init timer */
//    init_timer(&ecm->params.timer);
//    ecm->params.timer.function = ecm_response_available_notify;
//    ecm->params.timer.data = (USB_ULONG)ecm;

    INIT_DELAYED_WORK(&ecm->disconnect_work, ecm_data_disconnect);
    INIT_DELAYED_WORK(&ecm->notify_work, ecm_notify_work);
    
    DBG_I(MBB_ECM, "%s:leave\n",__FUNCTION__);
}

USB_INT32 ecm_params_register(USB_VOID (*resp_avail)(USB_VOID *v), USB_VOID *v)
{
    struct f_ecm *ecm = (struct f_ecm *)v;

    DBG_I(MBB_ECM, "%s:\n", __func__);

    if (!resp_avail)
    {
        return -EINVAL;
    }
    ecm->params.resp_avail = resp_avail;
    ecm->params.v = v;

    return 0;
}

USB_VOID ecm_params_deregister(struct f_ecm *ecm)
{
    DBG_I(MBB_ECM, "%s:\n", __func__);

    ecm->params.resp_avail = NULL;
    ecm->params.v = NULL;
}
static USB_INT ecm_set_alt(struct usb_function *f, USB_UINT intf, USB_UINT alt)
{
    struct f_ecm        *ecm = func_to_ecm(f);
    struct usb_composite_dev *cdev = f->config->cdev;

    DBG_T(MBB_ECM, "ecm_set_alt: intf=%u, alt=%u, ctrl_id=%u, data_id=%u\n",
        intf, alt, ecm->ctrl_id, ecm->data_id);
        
    /* Control interface has only altsetting 0 */
#ifndef GNET_SINGLE_INTERFACE
    if (intf == ecm->ctrl_id) {
        if (alt != 0)
            goto fail;
#else
    if (intf == ecm->ctrl_id && alt == 0) {

#endif
        if (ecm->notify->driver_data) {
            DBG_I(MBB_ECM, "reset ecm control %ld\n", intf);
            usb_ep_disable(ecm->notify);
        }
        if (!(ecm->notify->desc)) {
            DBG_I(MBB_ECM, "init ecm ctrl %ld\n", intf);
            if (config_ep_by_speed(cdev->gadget, f, ecm->notify))
                goto fail;
        }
        (USB_VOID)usb_ep_enable(ecm->notify);
        ecm->notify->driver_data = ecm;

        ndis_set_ecm();
        bsp_usb_set_enum_stat(ecm->data_id, 1);

        if ((0 == alt) && ( 0 == (VERIFY_ECM_BYPASS(ecm))))
        {
            pnp_set_net_drv_state(NET_STATE_NOT_READY);
            usb_notify_syswatch(EVENT_DEVICE_USB, USB_DISABLE_EVENT);
        }
    /* Data interface has two altsettings, 0 and 1 */
    } else if (intf == ecm->data_id) {
        if (alt > 1)
            goto fail;

        if (ecm->port.in_ep->driver_data) {
            DBG_I(MBB_ECM, "reset ecm\n");
            gether_disconnect(&ecm->port);
        }

        if (!ecm->port.in_ep->desc ||
            !ecm->port.out_ep->desc) {
            DBG_I(MBB_ECM, "init ecm\n");
            if (config_ep_by_speed(cdev->gadget, f,
                           ecm->port.in_ep) ||
                config_ep_by_speed(cdev->gadget, f,
                           ecm->port.out_ep)) {
                ecm->port.in_ep->desc = NULL;
                ecm->port.out_ep->desc = NULL;
                goto fail;
            }
        }

        /* CDC Ethernet only sends data in non-default altsettings.
         * Changing altsettings resets filters, statistics, etc.
         */
        struct net_device    *net;

        /* Enable zlps by default for ECM conformance;
         * override for musb_hdrc (avoids txdma ovhead).
         */
        ecm->port.is_zlp_ok = !(gadget_is_musbhdrc(cdev->gadget));
        ecm->port.cdc_filter = DEFAULT_FILTER;
        DBG_I(MBB_ECM, "activate ecm\n");
        net = gether_connect(&ecm->port);
        if (IS_ERR(net))
            return PTR_ERR(net);

        if (!VERIFY_ECM_BYPASS(ecm))
        {
            pnp_set_net_drv_state(NET_STATE_ALREADY);
            usb_notify_syswatch(EVENT_DEVICE_USB, USB_ENABLE_EVENT);
            /*设置连接状态为连接*/
            USB_ETH_LinkStatSet( TRUE );
        }
                
        ecm->port.notify = ecm->notify;

        /* NOTE this can be a minor disagreement with the ECM spec,
         * which says speed notifications will "always" follow
         * connection notifications.  But we allow one connect to
         * follow another (if the first is in flight), and instead
         * just guarantee that a speed notification is always sent.
         */
        ecm_notify(ecm);
    } else
        goto fail;
  

    ecm->curr_alt = alt;

    return 0;
fail:
    ecm->curr_alt = 0;
    return -EINVAL;
}

/* Because the data interface supports multiple altsettings,
 * this ECM function *MUST* implement a get_alt() method.
 */
static USB_INT32 ecm_get_alt(struct usb_function *f, USB_UINT32 intf)
{
    struct f_ecm        *ecm = func_to_ecm(f);

    if (intf == ecm->ctrl_id)
        return 0;
    return ecm->port.in_ep->driver_data ? 1 : 0;
}

static USB_VOID ecm_disable(struct usb_function *f)
{
    struct f_ecm        *ecm = func_to_ecm(f);
    
    DBG_T(MBB_ECM, "%s\n", __func__);
    usb_notify_syswatch(EVENT_DEVICE_USB,USB_DISABLE_EVENT);

    if (VERIFY_ECM_BYPASS(ecm))
    {
        adp_usb_queue_delay_work(&(ecm->disconnect_work), DISCONNECT_TIME_DELAY);
    }

    if (ecm->port.in_ep->driver_data)
        gether_disconnect(&ecm->port);

    if (ecm->notify->driver_data) {
        usb_ep_disable(ecm->notify);
        ecm->notify->driver_data = NULL;
        ecm->notify->desc = NULL;
    }

    bsp_usb_set_enum_stat(ecm->data_id, 0);
}

static USB_VOID ecm_suspend(struct usb_function *f)
{
    struct eth_dev *dev = func_to_ethdev(f);
    struct f_ecm *ecm = func_to_ecm(f);

    DBG_T(MBB_ECM, "%s\n", __func__);
    
    if ((!ecm->curr_alt) && (!dev)) {
        DBG_I(MBB_ECM, "ecm function disabled, skip the ecm adapter suspend process\n");
        return;
    }

    eth_suspend(dev);
}

static USB_VOID ecm_resume(struct usb_function *f)
{
    struct eth_dev *dev = func_to_ethdev(f);
    struct f_ecm *ecm = func_to_ecm(f);
    usb_pnp_api_hanlder *pnp_api_handler = pnp_adp_get_api_handler();
    USB_UINT8 pc_type = OS_FLAG_INVALID;
    
    if(pnp_api_handler->pnp_system_type_get_cb)
    {
        pc_type = pnp_api_handler->pnp_system_type_get_cb();
    }

    DBG_T(MBB_ECM, "%s\n", __func__);
    
    if ((!ecm->curr_alt) && (!dev)) {
        DBG_I(MBB_ECM, "ecm function disabled, skip the ecm adapter suspend process\n");
        return;
    }

    eth_resume(dev);

    if (OS_TYPE_MAC == pc_type)
    {
        adp_usb_queue_delay_work(&(ecm->notify_work), 0);
    }
}

/*-------------------------------------------------------------------------*/

/*
 * Callbacks let us notify the host about connect/disconnect when the
 * net device is opened or closed.
 *
 * For testing, note that link states on this side include both opened
 * and closed variants of:
 *
 *   - disconnected/unconfigured
 *   - configured but inactive (data alt 0)
 *   - configured and active (data alt 1)
 *
 * Each needs to be tested with unplug, rmmod, SET_CONFIGURATION, and
 * SET_INTERFACE (altsetting).  Remember also that "configured" doesn't
 * imply the host is actually polling the notification endpoint, and
 * likewise that "active" doesn't imply it's actually using the data
 * endpoints for traffic.
 */

static USB_VOID ecm_open(struct gether *geth)
{
    struct f_ecm        *ecm = func_to_ecm(&geth->func);

    DBG_T(MBB_ECM, "%s\n", __func__);

    ecm->is_open = true;
    ecm_notify(ecm);
}

static USB_VOID ecm_close(struct gether *geth)
{
    struct f_ecm        *ecm = func_to_ecm(&geth->func);

    DBG_T(MBB_ECM, "%s\n", __func__);

    ecm->is_open = false;
    ecm_notify(ecm);
}

/*-------------------------------------------------------------------------*/

/* ethernet function driver setup/binding */

static USB_INT
ecm_bind(struct usb_configuration *c, struct usb_function *f)
{
    struct usb_composite_dev *cdev = c->cdev;
    struct f_ecm        *ecm = func_to_ecm(f);
    USB_INT           status;
    struct usb_ep        *ep;

    DBG_T(MBB_ECM, "%s\n", __func__);
    
    /* allocate instance-specific interface IDs */
    status = usb_interface_id(c, f);
    if (status < 0)
        goto fail;
    ecm->ctrl_id = status;
    ecm_iad_descriptor.bFirstInterface = status;

    ecm_control_intf.bInterfaceNumber = status;
    ecm_union_desc.bMasterInterface0 = status;

#ifndef GNET_SINGLE_INTERFACE
    status = usb_interface_id(c, f);
    if (status < 0)
        goto fail;
#endif
    ecm->data_id = status;

    bsp_usb_add_setup_dev((USB_UINT32)status);

    ecm_data_nop_intf.bInterfaceNumber = status;
    ecm_data_intf.bInterfaceNumber = status;
    ecm_union_desc.bSlaveInterface0 = status;

    status = -ENODEV;

    /* allocate instance-specific endpoints */
    ep = usb_ep_autoconfig(cdev->gadget, &fs_ecm_in_desc);
    if (!ep)
        goto fail;
    ecm->port.in_ep = ep;
    ep->driver_data = cdev;    /* claim */

    ep = usb_ep_autoconfig(cdev->gadget, &fs_ecm_out_desc);
    if (!ep)
        goto fail;
    ecm->port.out_ep = ep;
    ep->driver_data = cdev;    /* claim */

    /* NOTE:  a status/notification endpoint is *OPTIONAL* but we
     * don't treat it that way.  It's simpler, and some newer CDC
     * profiles (wireless handsets) no longer treat it as optional.
     */
    ep = usb_ep_autoconfig(cdev->gadget, &fs_ecm_notify_desc);
    if (!ep)
        goto fail;
    ecm->notify = ep;
    ep->driver_data = cdev;    /* claim */

    status = -ENOMEM;

    /* allocate notification request and buffer */
    ecm->notify_req = usb_ep_alloc_request(ep, GFP_KERNEL);
    if (!ecm->notify_req)
        goto fail;
    ecm->notify_req->buf = kmalloc(ECM_STATUS_BYTECOUNT, GFP_KERNEL);
    if (!ecm->notify_req->buf)
        goto fail;
    ecm->notify_req->context = ecm;
    ecm->notify_req->complete = ecm_notify_complete;

    /* copy descriptors, and track endpoint copies */
    f->fs_descriptors = usb_copy_descriptors(ecm_fs_function);
    if (!f->fs_descriptors)
        goto fail;

    /* support all relevant hardware speeds... we expect that when
     * hardware is dual speed, all bulk-capable endpoints work at
     * both speeds
     */
    if (gadget_is_dualspeed(c->cdev->gadget)) {
        hs_ecm_in_desc.bEndpointAddress =
                fs_ecm_in_desc.bEndpointAddress;
        hs_ecm_out_desc.bEndpointAddress =
                fs_ecm_out_desc.bEndpointAddress;
        hs_ecm_notify_desc.bEndpointAddress =
                fs_ecm_notify_desc.bEndpointAddress;

        /* copy descriptors, and track endpoint copies */
        f->hs_descriptors = usb_copy_descriptors(ecm_hs_function);
        if (!f->hs_descriptors)
            goto fail;
    }

    if (gadget_is_superspeed(c->cdev->gadget)) {
        ss_ecm_in_desc.bEndpointAddress =
                fs_ecm_in_desc.bEndpointAddress;
        ss_ecm_out_desc.bEndpointAddress =
                fs_ecm_out_desc.bEndpointAddress;
        ss_ecm_notify_desc.bEndpointAddress =
                fs_ecm_notify_desc.bEndpointAddress;

        /* copy descriptors, and track endpoint copies */
        f->ss_descriptors = usb_copy_descriptors(ecm_ss_function);
        if (!f->ss_descriptors)
            goto fail;
    }

    /* NOTE:  all that is done without knowing or caring about
     * the network link ... which is unavailable to this code
     * until we're activated via set_alt().
     */

    ecm->port.open = ecm_open;
    ecm->port.close = ecm_close;

    DBG_I(MBB_ECM, "CDC Ethernet: %s speed IN/%s OUT/%s NOTIFY/%s\n",
            gadget_is_superspeed(c->cdev->gadget) ? "super" :
            gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
            ecm->port.in_ep->name, ecm->port.out_ep->name,
            ecm->notify->name);
    if (VERIFY_ECM_BYPASS(ecm))
    {
        //(USB_VOID)ecm_params_register(ecm_response_available, (USB_VOID * )ecm);
    
        status = ecm_vendor_tx_thread_init(ecm);
        if (status)
        {
            DBG_E(MBB_ECM, "ecm vendor tx thread init failed.\n");
            goto fail;
        }
    }

    return 0;

fail:
    if (f->fs_descriptors)
        usb_free_descriptors(f->fs_descriptors);
    if (f->hs_descriptors)
        usb_free_descriptors(f->hs_descriptors);

    if (ecm->notify_req) {
        kfree(ecm->notify_req->buf);
        usb_ep_free_request(ecm->notify, ecm->notify_req);
    }

    /* we might as well release our claims on endpoints */
    if (ecm->notify)
        ecm->notify->driver_data = NULL;
    if (ecm->port.out_ep)
        ecm->port.out_ep->driver_data = NULL;
    if (ecm->port.in_ep)
        ecm->port.in_ep->driver_data = NULL;

    DBG_E(MBB_ECM, "%s: can't bind, err %d\n", f->name, status);

    return status;
}

static USB_VOID
ecm_unbind(struct usb_configuration *c, struct usb_function *f)
{
    struct f_ecm        *ecm = func_to_ecm(f);

    DBG_T(MBB_ECM, "%s\n", __func__);

    if (gadget_is_superspeed(c->cdev->gadget))
        usb_free_descriptors(f->ss_descriptors);
    if (gadget_is_dualspeed(c->cdev->gadget))
        usb_free_descriptors(f->hs_descriptors);
    usb_free_descriptors(f->fs_descriptors);

    kfree(ecm->notify_req->buf);
    usb_ep_free_request(ecm->notify, ecm->notify_req);

    if (!VERIFY_ECM_BYPASS(ecm))
    {
        ecm_string_defs[ECM_MAC0_IDX].s = NULL;
    }
    kfree(ecm);
}

#ifdef GNET_CLASS_TYPE_HUAWEI
static inline USB_VOID ecm_set_config_vendor(USB_VOID)
{
    usb_pnp_api_hanlder *pnp_api_handler = pnp_adp_get_api_handler();
    USB_UINT8 pc_type = OS_FLAG_INVALID;
    
    if(pnp_api_handler->pnp_system_type_get_cb)
    {
        pc_type = pnp_api_handler->pnp_system_type_get_cb();
    }
    
    ecm_control_intf.bInterfaceClass = 0xFF;
    ecm_control_intf.bInterfaceProtocol = 0x07;

    ecm_data_intf.bInterfaceClass = 0xFF;
    ecm_data_intf.bInterfaceProtocol = 0x07;
    ecm_data_intf.bNumEndpoints = 3;

    ecm_data_nop_intf.bInterfaceClass = 0xFF;
    ecm_data_nop_intf.bInterfaceProtocol = 0x07;

    if((LINUX_OS_FLAG == pc_type)
                || (GATEWAY_OS_FLAG == pc_type))
    {
        ecm_control_intf.bInterfaceSubClass = USB_SUBCLASS_LINUX;
        ecm_data_intf.bInterfaceSubClass = USB_SUBCLASS_LINUX;
        ecm_data_nop_intf.bInterfaceSubClass = USB_SUBCLASS_LINUX;
    }
    else
    {
        ecm_control_intf.bInterfaceSubClass = USB_SUBCLASS_CODE;
        ecm_data_intf.bInterfaceSubClass = USB_SUBCLASS_CODE;
        ecm_data_nop_intf.bInterfaceSubClass = USB_SUBCLASS_CODE;
    }
}
#endif

#if defined(CONFIG_GNET_PREALLOC_RX_MEM)
static USB_INT32 ecm_unwrap(struct gether *port,
              struct sk_buff *skb,
              struct sk_buff_head *list)
{
    struct sk_buff *skb2;

    skb2 = skb_clone(skb,GFP_ATOMIC);
    
    if(NULL == skb2)
    {
        return -1;
    }

    make_clone_skb_writable(skb2);

    skb_queue_tail(list, skb2);

    gnet_put_rx_skb_to_done(port->ioport,skb);

    return 0;
}
#endif

/**
 * ecm_bind_config - add CDC Ethernet network link to a configuration
 * @c: the configuration to support the network link
 * @ethaddr: a buffer in which the ethernet address of the host side
 *    side of the link was recorded
 * Context: single threaded during gadget setup
 *
 * Returns zero on success, else negative errno.
 *
 * Caller must have called @gether_setup().  Caller is also responsible
 * for calling @gether_cleanup() before module unload.
 */
USB_INT
ecm_bind_config(struct usb_configuration *c, USB_UINT8 ethaddr[ETH_ALEN],struct eth_dev *dev)
{
    struct f_ecm    *ecm;
    USB_INT       status;

    DBG_T(MBB_ECM, "%s\n", __func__);

    if (!can_support_ecm(c->cdev->gadget) || !ethaddr)
        return -EINVAL;

    /* maybe allocate device-global string IDs */
    if (ecm_string_defs[ECM_CTRL_IDX].id == 0) {

        /* control interface label */
        status = usb_string_id(c->cdev);
        if (status < 0)
            return status;
        ecm_string_defs[ECM_CTRL_IDX].id = status;
        ecm_control_intf.iInterface = status;

        /* data interface label */
        status = usb_string_id(c->cdev);
        if (status < 0)
            return status;
        ecm_string_defs[ECM_DATA_IDX].id = status;
        ecm_data_intf.iInterface = status;

        /* MAC address */
        status = usb_string_id(c->cdev);
        if (status < 0)
            return status;
        ecm_string_defs[ECM_MAC0_IDX].id = status;
        ecm_desc.iMACAddress = status;

        /* IAD label */
        status = usb_string_id(c->cdev);
        if (status < 0)
            return status;
        ecm_string_defs[ECM_IAD_IDX].id = status;
        ecm_iad_descriptor.iFunction = status;
    }
    else if(0 == ecm_string_defs[ECM_MAC1_IDX].id){
        /* control interface label */
        ecm_control_intf.iInterface = ecm_string_defs[ECM_CTRL_IDX].id;
        /* data interface label */
        ecm_data_intf.iInterface = ecm_string_defs[ECM_DATA_IDX].id;
        /* IAD label */
        ecm_iad_descriptor.iFunction = ecm_string_defs[ECM_IAD_IDX].id;
     
        /* MAC1 address */
        status = usb_string_id(c->cdev);
        if (status < 0)
        {
            return status;
        }
        ecm_string_defs[ECM_MAC1_IDX].id = status;
        ecm_desc.iMACAddress = status;
    }

#ifdef GNET_CLASS_TYPE_HUAWEI
    ecm_set_config_vendor();
#endif

    /* allocate and initialize one new instance */
    ecm = kzalloc(sizeof *ecm, GFP_KERNEL);
    if (!ecm)
        return -ENOMEM;

    if (ecm_get_bypass_state())
    {
        ecm->work_mode = ECM_WORK_MODE_BYPASS;
    }
    else
    {
        ecm->work_mode = ECM_WORK_MODE_NET_BRIDGE;
    }

    /* export host's Ethernet address in CDC format */
    snprintf(ecm->ethaddr, sizeof ecm->ethaddr,
        "%02X%02X%02X%02X%02X%02X",
        ethaddr[0], ethaddr[1], ethaddr[2],
        ethaddr[3], ethaddr[4], ethaddr[5]);
    /*路由模式mac地址用转入参数旁路模式使用默认值*/
    if (!VERIFY_ECM_BYPASS(ecm))
    {
        ecm_string_defs[ECM_MAC0_IDX].s = ecm->ethaddr;
    }

    ecm->port.cdc_filter = DEFAULT_FILTER;

    ecm->port.func.name = "cdc_ethernet";
    ecm->port.func.strings = ecm_strings;
    /* descriptors are per-instance copies */
    ecm->port.func.bind = ecm_bind;
    ecm->port.func.unbind = ecm_unbind;
    ecm->port.func.set_alt = ecm_set_alt;
    ecm->port.func.get_alt = ecm_get_alt;
    ecm->port.func.setup = ecm_setup;
    ecm->port.func.disable = ecm_disable;

    ecm->port.func.suspend = ecm_suspend;
    ecm->port.func.resume = ecm_resume;

#if defined(CONFIG_GNET_PREALLOC_RX_MEM)
    ecm->port.unwrap = ecm_unwrap;
#endif

    if (VERIFY_ECM_BYPASS(ecm))
    {
        ecm->port.handle_rx = ecm_vendor_rx;

        ecm_params_init(ecm);

        ecm->vendor_ctx.ecm_priv = ecm;
        status = ecm_vendor_init(&ecm->vendor_ctx);
        if (status)
        {
            kfree(ecm);
            return status;
        }
    }
    else
    {
        ecm->port.handle_rx = NULL;
    }

    ecm->port.fixed_in_len = GNET_ECM_IN_MAXSIZE_PER_TRANSFER;
    ecm->port.fixed_out_len = GNET_ECM_OUT_MAXSIZE_PER_TRANSFER;

    INIT_DELAYED_WORK(&ecm->notify_work, ecm_notify_work);
    
    status = usb_add_function(c, &ecm->port.func);
    if (status) {
        if (VERIFY_ECM_BYPASS(ecm))
        {
            ecm_vendor_uninit(&ecm->vendor_ctx);
        }
        else
        {
            ecm_string_defs[ECM_MAC0_IDX].s = NULL;
        }

        kfree(ecm);
        return status;
    }

    ecm->port.ioport = (struct eth_dev *)dev;

    ecm->port.port_no = gecm_link_idx;
    the_ecm[gecm_link_idx] = ecm;
    gecm_link_idx++;
    return status;
}

void f_ecm_dump(void)
{
    int idx;
    struct f_ecm *ecm;

    for (idx = 0; idx < GNET_MAX_NUM; idx++) {
        ecm = the_ecm[idx];
        if (ecm) {
            DBG_T(MBB_ECM, "notify_state:       %u\n", ecm->notify_state);
        }
    }
}


/*ECM*/
/* Code Checker 将单目运算符"-（负号）"
作为双目运算符"-（减号）"处理，此为Code Checker问题 */
static ssize_t ecm_manufacturer_show(struct device* dev, struct device_attribute* attr, USB_CHAR *buf)
{
    return -1;
}

static ssize_t ecm_manufacturer_store(struct device* dev, struct device_attribute* attr, const USB_CHAR *buf, size_t size)
{

    return -1;
}

static DEVICE_ATTR(ecm_manufacturer, S_IRUGO | S_IWUSR, ecm_manufacturer_show, ecm_manufacturer_store);


static ssize_t ecm_ethaddr_show(struct device* dev, struct device_attribute* attr, USB_CHAR *buf)
{
    return -EINVAL;
}

static ssize_t ecm_ethaddr_store(struct device* dev, struct device_attribute* attr, const char *buf, size_t size)
{

    return  - EINVAL;
}

static DEVICE_ATTR(ecm_ethaddr, S_IRUGO | S_IWUSR, ecm_ethaddr_show, ecm_ethaddr_store);

static ssize_t ecm_vendorID_show(struct device* dev, struct device_attribute* attr, USB_CHAR *buf)
{
    return -EINVAL;
}

static ssize_t ecm_vendorID_store(struct device* dev,
    struct device_attribute* attr, const USB_CHAR *buf, size_t size)
{

    return -EINVAL;
}

static DEVICE_ATTR(ecm_vendorID, S_IRUGO | S_IWUSR, ecm_vendorID_show, ecm_vendorID_store);

static struct device_attribute* ecm_function_attributes[] =
{
    &dev_attr_ecm_manufacturer,
    &dev_attr_ecm_ethaddr,
    &dev_attr_ecm_vendorID,
    NULL
};

struct ecm_function_config
{
    USB_UINT8      ethaddr[ETH_ALEN];
    USB_UINT32     vendorID;
    USB_CHAR       manufacturer[MANUFACTURER_BUFFER_SIZE];
};

static USB_INT ecm_function_init(struct android_usb_function* f, struct usb_composite_dev* cdev)
{
    f->config = kzalloc(sizeof(struct ecm_function_config), GFP_KERNEL);
    if (!f->config)
    {
        return -ENOMEM;
    }
    return 0;
}
static USB_VOID ecm_function_cleanup(struct android_usb_function* f)
{
    kfree(f->config);
    f->config = NULL;
}

static USB_INT ecm_function_bind_config(struct android_usb_function* f, struct usb_configuration* c)
{
    USB_INT ret;
    struct eth_dev *dev;
    /*这是网卡的物理定制，数字定义成宏没有间意义codecheck*/
    USB_UINT8 ethaddr[ETH_ALEN] = {0x00, 0x1e, 0x10, 0x1f, 0x00, 0x00};

    dev = gether_setup_name(c->cdev->gadget, ethaddr, "ecm");
    if (IS_ERR(dev)) {
        ret = PTR_ERR(dev);
        DBG_E(MBB_ECM, "%s: gether_setup failed\n", __func__);
        return ret;
    }
    return ecm_bind_config(c, ethaddr,dev);

}
static USB_VOID ecm_function_unbind_config(struct android_usb_function* f, struct usb_configuration* c)
{
    gether_cleanup();
}

static struct android_usb_function ecm_function =
{
    .name          = "ecm",
    .init          = ecm_function_init,
    .cleanup       = ecm_function_cleanup,
    .bind_config   = ecm_function_bind_config,
    .unbind_config = ecm_function_unbind_config,
    .attributes    = ecm_function_attributes,
};

USB_VOID usb_ecm_init(USB_VOID)
{
    DBG_I(MBB_ECM, "usb_ecm_init enter\n");
    pnp_register_usb_support_function(&ecm_function);
}

