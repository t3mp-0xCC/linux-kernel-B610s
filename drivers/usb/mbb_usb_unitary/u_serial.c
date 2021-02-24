/*
 * u_serial.c - utilities for USB gadget "serial port"/TTY support
 *
 * Copyright (C) 2003 Al Borchers (alborchers@steinerpoint.com)
 * Copyright (C) 2008 David Brownell
 * Copyright (C) 2008 by Nokia Corporation
 *
 * This code also borrows from usbserial.c, which is
 * Copyright (C) 1999 - 2002 Greg Kroah-Hartman (greg@kroah.com)
 * Copyright (C) 2000 Peter Berger (pberger@brimson.com)
 * Copyright (C) 2000 Al Borchers (alborchers@steinerpoint.com)
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
/*
 * 2015-05-19 - changed for huawei device Huawei Technologies. The contribution by Huawei Technologies is under GPLv2
 */
 

/* #define VERBOSE_DEBUG */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/console.h>


#include "usb_platform_comm.h"
#include "usb_nv_get.h"
#include "u_serial.h"
#include "f_acm.h"
#include "mbb_usb_adp.h"
#include "usb_debug.h"
#include "hw_pnp_api.h"


/*
 * This component encapsulates the TTY layer glue needed to provide basic
 * "serial port" functionality through the USB gadget stack.  Each such
 * port is exposed through a /dev/ttyGS* node.
 *
 * After initialization (gserial_setup), these TTY port devices stay
 * available until they are removed (gserial_cleanup).  Each one may be
 * connected to a USB function (gserial_connect), or disconnected (with
 * gserial_disconnect) when the USB host issues a config change event.
 * Data can only flow when the port is connected to the host.
 *
 * A given TTY port can be made available in multiple configurations.
 * For example, each one might expose a ttyGS0 node which provides a
 * login application.  In one case that might use CDC ACM interface 0,
 * while another configuration might use interface 3 for that.  The
 * work to handle that (including descriptor management) is not part
 * of this component.
 *
 * Configurations may expose more than one TTY port.  For example, if
 * ttyGS0 provides login service, then ttyGS1 might provide dialer access
 * for a telephone or fax link.  And ttyGS2 might be something that just
 * needs a simple byte stream interface for some messaging protocol that
 * is managed in userspace ... OBEX, PTP, and MTP have been mentioned.
 */

#define PREFIX    "tty_comm"

/*
 * gserial is the lifecycle interface, used by USB functions
 * gs_port is the I/O nexus, used by the tty driver
 * tty_struct links to the tty/filesystem framework
 *
 * gserial <---> gs_port ... links will be null when the USB link is
 * inactive; managed by gserial_{connect,disconnect}().  each gserial
 * instance can wrap its own USB control protocol.
 *    gserial->ioport == usb_ep->driver_data ... gs_port
 *    gs_port->port_usb ... gserial
 *
 * gs_port <---> tty_struct ... links will be null when the TTY file
 * isn't opened; managed by gs_open()/gs_close()
 *    gserial->port_tty ... tty_struct
 *    tty_struct->driver_data ... gserial
 */

/* RX and TX queues can buffer QUEUE_SIZE packets before they hit the
 * next layer of buffering.  For TX that's a circular buffer; for RX
 * consider it a NOP.  A third layer is provided by the TTY code.
 */
#define QUEUE_SIZE            16
#define WRITE_BUF_SIZE        8192        /* TX only */
#define READ_BUF_SIZE       1536

/* circular buffer */
struct gs_buf
{
    USB_UINT        buf_size;
    USB_PCHAR            buf_buf;
    USB_PCHAR            buf_get;
    USB_PCHAR            buf_put;
};

/*
 * The port structure holds info for each port, one for each minor number
 * (and thus for each /dev/ node).
 */
struct gs_port
{
    struct tty_port    port;
    spinlock_t        port_lock;    /* guard port_* access */

    struct gserial*        port_usb;
    struct tty_struct*    port_tty;

    USB_UINT        open_count;
    bool            openclose;    /* open/close in progress */
    USB_UINT8            port_num;

    wait_queue_head_t    close_wait;    /* wait for last close */

    struct list_head    read_pool;
    USB_INT read_started;
    USB_INT read_allocated;
    struct list_head    read_queue;
    USB_UINT        n_read;
    struct tasklet_struct    push;

    struct list_head    write_pool;
    USB_INT write_started;
    USB_INT write_allocated;
    struct gs_buf        port_write_buf;
    wait_queue_head_t    drain_wait;    /* wait while writes drain */

    /* REVISIT this state ... */
    struct usb_cdc_line_coding port_line_coding;    /* 8-N-1 etc */
	USB_UINT line_state_on;
	USB_UINT is_suspend;

    USB_UINT stat_write_no_mem;
    USB_UINT stat_con_write_no_room;
    USB_UINT stat_write_not_conn;
    USB_UINT stat_con_write_no_mem;
    USB_UINT stat_con_write_not_conn;
    USB_UINT stat_port_is_connect;
    USB_UINT stat_cannt_push;
    USB_UINT stat_con_data_max;
    USB_PCHAR in_name;
    USB_PCHAR out_name;

    USB_UINT stat_insert_tty;
    USB_UINT stat_insert_tty_push;

};

/* increase N_PORTS if you need more */
//#define N_PORTS        4
static struct portmaster
{
    struct mutex    lock;            /* protect open/close */
    struct gs_port*    port;
} ports[ACM_TTY_COUNT];
static USB_UINT    n_ports;

#define GS_CLOSE_TIMEOUT        15        /* seconds */

static struct tty_driver* gs_console_tty_driver = NULL;

#ifdef VERBOSE_DEBUG
#define pr_vdebug(fmt, arg...) \
    //pr_debug(fmt, ##arg)
#else
#define pr_vdebug(fmt, arg...) \
    //({ if (0) pr_debug(fmt, ##arg); })
#endif

static USB_INT gs_acm_is_console_enable(USB_VOID);


/*-------------------------------------------------------------------------*/

/* Circular Buffer */

/*
 * gs_buf_alloc
 *
 * Allocate a circular buffer and all associated memory.
 */
static USB_INT gs_buf_alloc(struct gs_buf* gb, USB_UINT size)
{
    gb->buf_buf = kmalloc(size, GFP_KERNEL);
    if (gb->buf_buf == NULL)
    { return -ENOMEM; }

    gb->buf_size = size;
    gb->buf_put = gb->buf_buf;
    gb->buf_get = gb->buf_buf;

    return 0;
}

/*
 * gs_buf_free
 *
 * Free the buffer and all associated memory.
 */
static USB_VOID gs_buf_free(struct gs_buf* gb)
{
    if (gb->buf_buf)
    {
        kfree(gb->buf_buf);
        gb->buf_buf = NULL;
    }
}

/*
 * gs_buf_clear
 *
 * Clear out all data in the circular buffer.
 */
static USB_VOID gs_buf_clear(struct gs_buf* gb)
{
    gb->buf_get = gb->buf_put;
    /* equivalent to a get of all data available */
}

/*
 * gs_buf_data_avail
 *
 * Return the number of bytes of data written into the circular
 * buffer.
 */
static USB_UINT gs_buf_data_avail(struct gs_buf* gb)
{
    return (gb->buf_size + gb->buf_put - gb->buf_get) % gb->buf_size;
}

/*
 * gs_buf_space_avail
 *
 * Return the number of bytes of space available in the circular
 * buffer.
 */
static USB_UINT gs_buf_space_avail(struct gs_buf* gb)
{
    return (gb->buf_size + gb->buf_get - gb->buf_put - 1) % gb->buf_size;
}

/*
 * gs_buf_room_avail
 *
 * Return the number of bytes of space available in the circular
 * buffer. If buf overflow,reset buffer
 *
 */
static USB_UINT gs_buf_room_avail(struct gs_buf* gb)
{
   USB_UINT len  = 0;
   len = ( gb->buf_size + gb->buf_get - gb->buf_put - 1) % gb->buf_size;
   /*if the space overflow  , reset log buffer*/
   if( 0 == len)
   {
         gb->buf_put = gb->buf_buf;
         gb->buf_get = gb->buf_buf;
         len = gb->buf_size-1;
   }
   return len ;
}

/*
 * gs_buf_put
 *
 * Copy data data from a user buffer and put it into the circular buffer.
 * Restrict to the amount of space available.
 *
 * Return the number of bytes copied.
 */
static USB_UINT
gs_buf_put(struct gs_buf* gb, const USB_PCHAR buf, USB_UINT count)
{
    USB_UINT len;

    len  = gs_buf_space_avail(gb);
    if (count > len)
    { count = len; }

    if (count == 0)
    { return 0; }

    len = gb->buf_buf + gb->buf_size - gb->buf_put;
    if (count > len)
    {
        memcpy(gb->buf_put, buf, len);
        memcpy(gb->buf_buf, buf + len, count - len);
        gb->buf_put = gb->buf_buf + count - len;
    }
    else
    {
        memcpy(gb->buf_put, buf, count);
        if (count < len)
        { 
            gb->buf_put += count; 
        }
        else /* count == len */
        {
            gb->buf_put = gb->buf_buf; 
        }
    }

    return count;
}

/*
 * gs_buf_get
 *
 * Get data from the circular buffer and copy to the given buffer.
 * Restrict to the amount of data available.
 *
 * Return the number of bytes copied.
 */
static USB_UINT
gs_buf_get(struct gs_buf* gb, USB_PCHAR buf, USB_UINT count)
{
    USB_UINT len;

    len = gs_buf_data_avail(gb);
    if (count > len)
    { count = len; }

    if (count == 0)
    { return 0; }

    len = gb->buf_buf + gb->buf_size - gb->buf_get;
    if (count > len)
    {
        memcpy(buf, gb->buf_get, len);
        memcpy(buf + len, gb->buf_buf, count - len);
        gb->buf_get = gb->buf_buf + count - len;
    }
    else
    {
        memcpy(buf, gb->buf_get, count);
        if (count < len)
        {
            gb->buf_get += count; 
        }
        else /* count == len */
        {
            gb->buf_get = gb->buf_buf; 
        }
    }

    return count;
}

/*-------------------------------------------------------------------------*/

/* I/O glue between TTY (upper) and USB function (lower) driver layers */

/*
 * gs_alloc_req
 *
 * Allocate a usb_request and its buffer.  Returns a pointer to the
 * usb_request or NULL if there is an error.
 */
struct usb_request*
gs_alloc_req(struct usb_ep* ep, USB_UINT len, gfp_t kmalloc_flags)
{
    struct usb_request* req;

    req = usb_ep_alloc_request(ep, kmalloc_flags);

    if (req != NULL)
    {
        req->length = len;
        /* if len is 0, alloc the empty req */
        if (0 == len)
        {
            req->buf = NULL;
            req->dma = (dma_addr_t)NULL;
        }
        else
        {
            req->buf = kmalloc(len, kmalloc_flags);
            if (req->buf == NULL)
            {
                usb_ep_free_request(ep, req);
                return NULL;
            }
        }
    }

    return req;
}

/*
 * gs_free_req
 *
 * Free a usb_request and its buffer.
 */
USB_VOID gs_free_req(struct usb_ep* ep, struct usb_request* req)
{
    if (req->buf)
    {
        kfree(req->buf);
    }
    usb_ep_free_request(ep, req);
}

/*
 * gs_send_packet
 *
 * If there is data to send, a packet is built in the given
 * buffer and the size is returned.  If there is no data to
 * send, 0 is returned.
 *
 * Called with port_lock held.
 */
static USB_UINT
gs_send_packet(struct gs_port* port, USB_PCHAR packet, USB_UINT size)
{
    USB_UINT len;

    len = gs_buf_data_avail(&port->port_write_buf);
    if (len < size)
    {
        size = len;
    }
    if (size != 0)
    {
        size = gs_buf_get(&port->port_write_buf, packet, size); 
    }
    return size;
}

/*
 * gs_start_tx
 *
 * This function finds available write requests, calls
 * gs_send_packet to fill these packets with data, and
 * continues until either there are no more write requests
 * available or no more data to send.  This function is
 * run whenever data arrives or write requests are available.
 *
 * Context: caller owns port_lock; port_usb is non-null.
 */
static USB_INT gs_start_tx(struct gs_port* port)
/*
__releases(&port->port_lock)
__acquires(&port->port_lock)
*/
{
    struct list_head*    pool = &port->write_pool;
    struct usb_ep*        in = port->port_usb->in;
    USB_INT            status = 0;
    bool            do_tty_wake = false;

    while (!list_empty(pool))
    {
        struct usb_request*    req;
        USB_INT            len;

        if (port->write_started >= QUEUE_SIZE)
        { break; }

        req = list_entry(pool->next, struct usb_request, list);
        len = gs_send_packet(port, req->buf, in->maxpacket);
        if (len == 0)
        {
            wake_up_interruptible(&port->drain_wait);
            break;
        }
        do_tty_wake = true;

        req->length = len;
        list_del(&req->list);
        req->zero = (len % in->maxpacket) ? (0) : (1);
        DBG_I(MBB_ACM,"%d: tx len=%d, 0x%02x 0x%02x 0x%02x ...\n",
                  port->port_num, len, *((u8*)req->buf),
                  *((u8*)req->buf + 1), *((u8*)req->buf + 2));
        /* Drop lock while we call out of driver; completions
         * could be issued while we do so.  Disconnection may
         * happen too; maybe immediately before we queue this!
         *
         * NOTE that we may keep sending data for a while after
         * the TTY closed (dev->ioport->port_tty is NULL).
         */
        spin_unlock(&port->port_lock);
        status = usb_ep_queue(in, req, GFP_ATOMIC);
        spin_lock(&port->port_lock);

        if (status)
        {
            DBG_I(MBB_ACM,"%s: %s %s err %d\n",
                      __func__, "queue", in->name, status);
            list_add(&req->list, pool);
            break;
        }

        port->write_started++;

        /* abort immediately after disconnect */
        if (!port->port_usb)
        { break; }
    }

    if (do_tty_wake && port->port_tty)
    { tty_wakeup(port->port_tty); }
    return status;
}

/*
 * Context: caller owns port_lock, and port_usb is set
 */
static USB_INT gs_start_rx(struct gs_port* port)
/*
__releases(&port->port_lock)
__acquires(&port->port_lock)
*/
{
    struct list_head*    pool = &port->read_pool;
    struct usb_ep*        out = NULL;

    if ((NULL != port) && (NULL != port->port_usb))
    {
        out = port->port_usb->out;
    }

    if (NULL == out)
    {
        BUG_ON(1);
        return -1;
    }

    while (!list_empty(pool))
    {
        struct usb_request*    req;
        USB_INT            status;
        //struct tty_struct*    tty;

        /* no more rx if closed */
        //tty = port->port_tty;

        /* if not open tty, still prepare rx req */
        /*
        if (!tty)
            break;
        */

        if (port->read_started >= QUEUE_SIZE)
        { break; }

        req = list_entry(pool->next, struct usb_request, list);
        list_del(&req->list);
        /* set it in alloc reqs */
        /* req->length = out->maxpacket; */

        /* drop lock while we call out; the controller driver
         * may need to call us back (e.g. for disconnect)
         */
        spin_unlock(&port->port_lock);
        status = usb_ep_queue(out, req, GFP_ATOMIC);
        spin_lock(&port->port_lock);

        if (status)
        {
            DBG_I(MBB_ACM,"%s: %s %s err %d\n",
                      __func__, "queue", out->name, status);
            list_add(&req->list, pool);
            break;
        }
        port->read_started++;

        /* abort immediately after disconnect */
        if (!port->port_usb)
        {
            break; 
        }
    }
    return port->read_started;
}


/*
 * RX tasklet takes data out of the RX queue and hands it up to the TTY
 * layer until it refuses to take any more data (or is throttled back).
 * Then it issues reads for any further data.
 *
 * If the RX queue becomes full enough that no usb_request is queued,
 * the OUT endpoint may begin NAKing as soon as its FIFO fills up.
 * So QUEUE_SIZE packets plus however many the FIFO holds (usually two)
 * can be buffered before the TTY layer's buffers (currently 64 KB).
 */
static void gs_rx_push(USB_ULONG _port)
{
    struct gs_port*        port = (void*)_port;
    struct tty_struct*    tty;
    struct list_head*    queue = &port->read_queue;
    bool            disconnect = false;
    bool            do_push = false;

    /* hand any queued data to the tty */
    spin_lock_irq(&port->port_lock);

    tty = port->port_tty;
    while (!list_empty(queue))
    {
        struct usb_request*    req;

        req = list_first_entry(queue, struct usb_request, list);

        /* discard data if tty was closed */
        if (!tty)
        { goto recycle; }

        /* leave data queued if tty was rx throttled */
        if (test_bit(TTY_THROTTLED, &tty->flags))
        {
            break;
        }

        switch (req->status)
        {
            case -ESHUTDOWN:
                disconnect = true;
                DBG_I(MBB_ACM,"%d: shutdown\n", port->port_num);
                break;

            default:
                /* presumably a transient fault */
                DBG_W(MBB_ACM,"%d: unexpected RX status %d\n",
                           port->port_num, req->status);
                /* FALLTHROUGH */
            case 0:
                /* normal completion */
                break;
        }

        /* push data to (open) tty */
        if (req->actual)
        {
            USB_PCHAR        packet = req->buf;
            USB_UINT    size = req->actual;
            USB_UINT    n;
            USB_INT        count;

            /* we may have pushed part of this packet already... */
            n = port->n_read;
            if (n)
            {
                packet += n;
                size -= n;
            }

            count = tty_insert_flip_string(&port->port, packet, size);
            port->stat_insert_tty += count;
            if (count)
            { 
                do_push = true; 
            }

            if (count != size)
            {
                /* stop pushing; TTY layer can't handle more */
                port->n_read += count;
                port->stat_cannt_push++;
                DBG_I(MBB_ACM,"%d: rx block %d/%d\n",
                          port->port_num,
                          count, req->actual);
                break;
            }
            port->n_read = 0;
        }
    recycle:
        list_move(&req->list, &port->read_pool);
        port->read_started--;
    }

    /* Push from tty to ldisc; without low_latency set this is handled by
     * a workqueue, so we won't get callbacks and can hold port_lock
     */
    if (tty && do_push)
    {
        port->stat_insert_tty_push++;
        tty_flip_buffer_push(&port->port); 
    }


    /* We want our data queue to become empty ASAP, keeping data
     * in the tty and ldisc (not here).  If we couldn't push any
     * this time around, there may be trouble unless there's an
     * implicit tty_unthrottle() call on its way...
     *
     * REVISIT we should probably add a timer to keep the tasklet
     * from starving ... but it's not clear that case ever happens.
     */
    if (!list_empty(queue) && tty)
    {
        if (!test_bit(TTY_THROTTLED, &tty->flags))
        {
            if (do_push)
            { 
                tasklet_schedule(&port->push); 
            }
            else
            {
                DBG_W(MBB_ACM,"%d: RX not scheduled?\n",port->port_num);
            }
                
        }
    }

    /* If we're still connected, refill the USB RX queue. */
    if (!disconnect && port->port_usb)
    {
        gs_start_rx(port);
    }

    spin_unlock_irq(&port->port_lock);
}

static USB_VOID gs_read_complete(struct usb_ep* ep, struct usb_request* req)
{
    struct gs_port*    port = ep->driver_data;

    /* Queue all received data until the tty layer is ready for it. */
    spin_lock(&port->port_lock);

    /* we start rx in connect, so if usr not open the dev, drop the rx data */
    if (!port->open_count)
    {
        list_add_tail(&req->list, &port->read_pool);
        port->read_started--;
    }
    else
    {
        list_add_tail(&req->list, &port->read_queue);
    }
    tasklet_schedule(&port->push);
    spin_unlock(&port->port_lock);
}

static USB_VOID gs_write_complete(struct usb_ep* ep, struct usb_request* req)
{
    struct gs_port*    port = ep->driver_data;

    spin_lock(&port->port_lock);
    list_add(&req->list, &port->write_pool);
    port->write_started--;

    switch (req->status)
    {
        default:
            /* presumably a transient fault */

            DBG_W(MBB_ACM,"%s: unexpected %s status %d\n",
                       __func__, ep->name, req->status);
            /* FALL THROUGH */
        case 0:
            /* normal completion */
            gs_start_tx(port);
            break;

        case -ESHUTDOWN:
            /* disconnect */
            DBG_I(MBB_ACM,"%s: %s shutdown\n", __func__, ep->name);
            break;
    }

    spin_unlock(&port->port_lock);
}

static USB_VOID gs_free_requests(struct usb_ep* ep, struct list_head* head,USB_INT* allocated)
{
    struct usb_request*    req;

    while (!list_empty(head))
    {
        req = list_entry(head->next, struct usb_request, list);
        list_del(&req->list);
        gs_free_req(ep, req);
        if (allocated)
        {
            (*allocated)--; 
        }
    }
}

static USB_INT gs_alloc_requests(struct usb_ep* ep, struct list_head* head,
            USB_VOID (*fn)(struct usb_ep*, struct usb_request*), USB_INT* allocated, bool is_in)
{
    USB_INT            i;
    struct usb_request*    req;
    USB_INT n = allocated ? QUEUE_SIZE - *allocated : QUEUE_SIZE;
    USB_UINT alloc_size = is_in ? ep->maxpacket : READ_BUF_SIZE;

    /* Pre-allocate up to QUEUE_SIZE transfers, but if we can't
     * do quite that many this time, don't fail ... we just won't
     * be as speedy as we might otherwise be.
     */
    for (i = 0; i < n; i++)
    {
        /* for lrz bug: if push size is not correct, lrz can't work */
        /*req = gs_alloc_req(ep, ep->maxpacket, GFP_ATOMIC);*/
        req = gs_alloc_req(ep, alloc_size, GFP_ATOMIC);
        if (!req)
        { return list_empty(head) ? -ENOMEM : 0; }
        req->complete = fn;
        list_add_tail(&req->list, head);
        if (allocated)
        { (*allocated)++; }
    }
    return 0;
}


/**
 * gs_start_io - start USB I/O streams
 * @dev: encapsulates endpoints to use
 * Context: holding port_lock; port_tty and port_usb are non-null
 *
 * We only start I/O when something is connected to both sides of
 * this port.  If nothing is listening on the host side, we may
 * be pointlessly filling up our TX buffers and FIFO.
 */
static USB_INT gs_start_io(struct gs_port* port)
{
    struct list_head*    head = &port->read_pool;
    struct usb_ep*        ep = port->port_usb->out;
    USB_INT            status;
    USB_UINT        started;

    /* Allocate RX and TX I/O buffers.  We can't easily do this much
     * earlier (with GFP_KERNEL) because the requests are coupled to
     * endpoints, as are the packet sizes we'll be using.  Different
     * configurations may use different endpoints with a given port;
     * and high speed vs full speed changes packet sizes too.
     */
    status = gs_alloc_requests(ep, head, gs_read_complete,
                               &port->read_allocated, false);
    if (status)
    { return status; }

    status = gs_alloc_requests(port->port_usb->in, &port->write_pool,
                               gs_write_complete, &port->write_allocated, true);
    if (status)
    {
        gs_free_requests(ep, head, &port->read_allocated);
        return status;
    }

    /* queue read requests */
    port->n_read = 0;
    started = gs_start_rx(port);

    /* unblock any pending writes into our circular buffer */
    if (started)
    {
        if (port->port_tty)
        { tty_wakeup(port->port_tty); }
    }
    else
    {
        gs_free_requests(ep, head, &port->read_allocated);
        gs_free_requests(port->port_usb->in, &port->write_pool,
                         &port->write_allocated);
        status = -EIO;
    }

    return status;
}

/*-------------------------------------------------------------------------*/

/* TTY Driver */

/*
 * gs_open sets up the link between a gs_port and its associated TTY.
 * That link is broken *only* by TTY close(), and all driver methods
 * know that.
 */
static USB_INT gs_open(struct tty_struct* tty, struct file* file)
{
    USB_INT        port_num = tty->index;
    struct gs_port*    port;
    USB_INT        status;

    do
    {
        mutex_lock(&ports[port_num].lock);
        port = ports[port_num].port;
        if (!port)
        { 
            mutex_unlock(&ports[port_num].lock);
            return -ENODEV;
        }
        else
        {
            spin_lock_irq(&port->port_lock);

            /* already open?  Great. */
            if (port->open_count)
            {
                status = 0;
                port->open_count++;

                /* currently opening/closing? wait ... */
            }
            else if (port->openclose)
            {
                status = -EBUSY;

                /* ... else we do the work */
            }
            else
            {
                status = -EAGAIN;
                port->openclose = true;
            }
            spin_unlock_irq(&port->port_lock);
        }
        mutex_unlock(&ports[port_num].lock);

        switch (status)
        {
            default:
                /* fully handled */
                return status;
            case -EAGAIN:
                /* must do the work */
                break;
            case -EBUSY:
                /* wait for EAGAIN task to finish */
                msleep(1);
                /* REVISIT could have a waitchannel here, if
                 * concurrent open performance is important
                 */
                break;
        }
    }
    while (status != -EAGAIN);

    /* Do the "real open" */
    spin_lock_irq(&port->port_lock);

    /* allocate circular buffer on first open */
    if (port->port_write_buf.buf_buf == NULL)
    {

        spin_unlock_irq(&port->port_lock);
        status = gs_buf_alloc(&port->port_write_buf, WRITE_BUF_SIZE);
        spin_lock_irq(&port->port_lock);

        if (status)
        {
            DBG_I(MBB_ACM,"gs_open: ttyGS%d (%p,%p) no buffer\n",
                      port->port_num, tty, file);
            port->openclose = false;
            goto exit_unlock_port;
        }
    }

    /* REVISIT if REMOVED (ports[].port NULL), abort the open
     * to let rmmod work faster (but this way isn't wrong).
     */

    /* REVISIT maybe wait for "carrier detect" */

    tty->driver_data = port;
    port->port.tty = tty;
    port->port_tty = tty;
    port->port.count = 1;

    port->open_count = 1;
    port->openclose = false;

    /* if connected, start the I/O stream */
    if (port->port_usb)
    {
#if ACM_TTY_SUPPORT_NOTIFY
        struct gserial*    gser = port->port_usb;
        DBG_I(MBB_ACM,"gs_open: start ttyGS%d\n", port->port_num);
        if (gser->connect)
        { gser->connect(gser); }
#endif

        gs_start_io(port);
    }

    DBG_I(MBB_ACM,"gs_open: ttyGS%d (%p,%p)\n", port->port_num, tty, file);
    status = 0;

exit_unlock_port:
    spin_unlock_irq(&port->port_lock);
    return status;
}

static USB_INT gs_writes_finished(struct gs_port* p)
{
    USB_INT cond;

    /* return true on disconnect or empty buffer */
    spin_lock_irq(&p->port_lock);
    cond = (p->port_usb == NULL) || !gs_buf_data_avail(&p->port_write_buf);
    spin_unlock_irq(&p->port_lock);

    return cond;
}

static USB_VOID gs_close(struct tty_struct* tty, struct file* file)
{
    struct gs_port* port = tty->driver_data;
    struct gserial*    gser;

    spin_lock_irq(&port->port_lock);

    if (port->open_count != 1)
    {
        if (port->open_count == 0)
        { WARN_ON(1); }
        else
        { --port->open_count; }
        goto exit;
    }
    DBG_I(MBB_ACM,"gs_close: ttyGS%d (%p,%p) ...\n", port->port_num, tty, file);
    /* mark port as closing but in use; we can drop port lock
     * and sleep if necessary
     */
    port->openclose = true;
    port->open_count = 0;

    gser = port->port_usb;

#if ACM_TTY_SUPPORT_NOTIFY
    if (gser && gser->disconnect)
    { gser->disconnect(gser); }
#endif

    /* wait for circular write buffer to drain, disconnect, or at
     * most GS_CLOSE_TIMEOUT seconds; then discard the rest
     */
    if (gs_buf_data_avail(&port->port_write_buf) > 0 && gser)
    {
        spin_unlock_irq(&port->port_lock);
        wait_event_interruptible_timeout(port->drain_wait,
                                         gs_writes_finished(port),
                                         GS_CLOSE_TIMEOUT * HZ);
        spin_lock_irq(&port->port_lock);
        gser = port->port_usb;
    }

    /* Iff we're disconnected, there can be no I/O in flight so it's
     * ok to free the circular buffer; else just scrub it.  And don't
     * let the push tasklet fire again until we're re-opened.
     */
    if (gser == NULL)
    { gs_buf_free(&port->port_write_buf); }
    else
    { gs_buf_clear(&port->port_write_buf); }

    tty->driver_data = NULL;
    port->port_tty = NULL;
    port->port.tty = NULL;

    port->openclose = false;
    DBG_I(MBB_ACM,"gs_close: ttyGS%d (%p,%p) done!\n",
              port->port_num, tty, file);

    wake_up_interruptible(&port->close_wait);
exit:
    spin_unlock_irq(&port->port_lock);
}

static USB_INT gs_write(struct tty_struct* tty, const USB_UCHAR* buf, USB_INT count)
{
    struct gs_port*    port = tty->driver_data;
    USB_ULONG    flags;
    //USB_INT        status;
    
    DBG_I(MBB_ACM,"gs_write: ttyGS%d (%p) writing %d bytes\n",
              port->port_num, tty, count);
    spin_lock_irqsave(&port->port_lock, flags);
    if (unlikely(!port->open_count))
    {
        spin_unlock_irqrestore(&port->port_lock, flags);
        return -EINVAL;
    }
#ifdef USB_SOLUTION
	if (unlikely(!port->port_usb))
#else
	if (unlikely(!port->port_usb || port->is_suspend))
#endif
    {
	    port->stat_write_not_conn++;
	    spin_unlock_irqrestore(&port->port_lock, flags);
        return -ESHUTDOWN;
    }
    /*
     * we don't have enough room for some data
     * may be the host don't start read, so drop the data.
     * otherwise the printed thread may pending forever.
     */
    if (!port->line_state_on && list_empty(&port->write_pool))
    {
        port->stat_write_no_mem++;
        spin_unlock_irqrestore(&port->port_lock, flags);
        return -ENOMEM;
    }
    if (count)
    { count = gs_buf_put(&port->port_write_buf, buf, count); }
    /* treat count == 0 as flush_chars() */
    if (port->port_usb)
    { 
        (USB_VOID)gs_start_tx(port); 
    }
    spin_unlock_irqrestore(&port->port_lock, flags);

    return count;
}

static USB_INT gs_put_char(struct tty_struct* tty, USB_UINT8 ch)
{
    struct gs_port*    port = tty->driver_data;
    USB_ULONG    flags;
    USB_INT        status;

    DBG_I(MBB_ACM,"gs_put_char: (%d,%p) char=0x%x, called from %p\n",
              port->port_num, tty, ch, __builtin_return_address(0));

    spin_lock_irqsave(&port->port_lock, flags);
    status = gs_buf_put(&port->port_write_buf, &ch, 1);
    spin_unlock_irqrestore(&port->port_lock, flags);

    return status;
}

static USB_VOID gs_flush_chars(struct tty_struct* tty)
{
    struct gs_port*    port = tty->driver_data;
    USB_ULONG    flags;

    DBG_I(MBB_ACM,"gs_flush_chars: (%d,%p)\n", port->port_num, tty);
    
    spin_lock_irqsave(&port->port_lock, flags);
    if (port->port_usb)
    { gs_start_tx(port); }
    spin_unlock_irqrestore(&port->port_lock, flags);
}

static USB_INT gs_write_room(struct tty_struct* tty)
{
    struct gs_port*    port = tty->driver_data;
    USB_ULONG    flags;
    USB_INT        room = WRITE_BUF_SIZE - 1;

    spin_lock_irqsave(&port->port_lock, flags);
    if (port->port_usb)
    { room = gs_buf_room_avail(&port->port_write_buf); }
    spin_unlock_irqrestore(&port->port_lock, flags);

    DBG_I(MBB_ACM,"gs_write_room: (%d,%p) room=%d\n",
              port->port_num, tty, room);
    return room;
}

static USB_INT gs_chars_in_buffer(struct tty_struct* tty)
{
    struct gs_port*    port = tty->driver_data;
    USB_ULONG    flags;
    USB_INT        chars = 0;

    spin_lock_irqsave(&port->port_lock, flags);
    chars = gs_buf_data_avail(&port->port_write_buf);
    spin_unlock_irqrestore(&port->port_lock, flags);

    DBG_I(MBB_ACM,"gs_chars_in_buffer: (%d,%p) chars=%d\n",
              port->port_num, tty, chars);
    return chars;
}

/* undo side effects of setting TTY_THROTTLED */
static USB_VOID gs_unthrottle(struct tty_struct* tty)
{
    struct gs_port*        port = tty->driver_data;
    USB_ULONG        flags;

    spin_lock_irqsave(&port->port_lock, flags);
    if (port->port_usb)
    {
        /* Kickstart read queue processing.  We don't do xon/xoff,
         * rts/cts, or other handshaking with the host, but if the
         * read queue backs up enough we'll be NAKing OUT packets.
         */
        tasklet_schedule(&port->push);
        DBG_I(MBB_ACM,"%d: unthrottle\n", port->port_num);
    }
    spin_unlock_irqrestore(&port->port_lock, flags);
}

static USB_INT gs_break_ctl(struct tty_struct* tty, USB_INT duration)
{
    struct gs_port*    port = tty->driver_data;
    USB_INT        status = 0;
    struct gserial*    gser;

    DBG_I(MBB_ACM,"gs_break_ctl: ttyGS%d, send break (%d) \n",
              port->port_num, duration);
    spin_lock_irq(&port->port_lock);
    gser = port->port_usb;
    if (gser && gser->send_break)
    { status = gser->send_break(gser, duration); }
    spin_unlock_irq(&port->port_lock);

    return status;
}

static const struct tty_operations gs_tty_ops =
{
    .open =            gs_open,
    .close =        gs_close,
    .write =        gs_write,
    .put_char =        gs_put_char,
    .flush_chars =        gs_flush_chars,
    .write_room =        gs_write_room,
    .chars_in_buffer =    gs_chars_in_buffer,
    .unthrottle =        gs_unthrottle,
    .break_ctl =        gs_break_ctl,
};

/*-------------------------------------------------------------------------*/

static struct tty_driver* gs_tty_driver;

static USB_INT
gs_port_alloc(USB_UINT port_num, struct usb_cdc_line_coding* coding)
{
    struct gs_port*    port;

    port = kzalloc(sizeof(struct gs_port), GFP_KERNEL);
    if (port == NULL)
    { return -ENOMEM; }

    tty_port_init(&port->port);
    spin_lock_init(&port->port_lock);
    init_waitqueue_head(&port->close_wait);
    init_waitqueue_head(&port->drain_wait);

    tasklet_init(&port->push, gs_rx_push, (USB_ULONG) port);

    INIT_LIST_HEAD(&port->read_pool);
    INIT_LIST_HEAD(&port->read_queue);
    INIT_LIST_HEAD(&port->write_pool);

    port->port_num = port_num;
    port->port_line_coding = *coding;

    ports[port_num].port = port;

    return 0;
}

/**
 * gserial_setup - initialize TTY driver for one or more ports
 * @g: gadget to associate with these ports
 * @count: how many ports to support
 * Context: may sleep
 *
 * The TTY stack needs to know in advance how many devices it should
 * plan to manage.  Use this call to set up the ports you will be
 * exporting through USB.  Later, connect them to functions based
 * on what configuration is activated by the USB host; and disconnect
 * them as appropriate.
 *
 * An example would be a two-configuration device in which both
 * configurations expose port 0, but through different functions.
 * One configuration could even expose port 1 while the other
 * one doesn't.
 *
 * Returns negative errno or zero.
 */
USB_INT gserial_setup(struct usb_gadget* g, USB_UINT count)
{
    USB_UINT            i;
    struct usb_cdc_line_coding    coding;
    USB_INT                status = 0;
    struct device*    tty_dev;

    if (count == 0 || count > ACM_TTY_COUNT)
    { return -EINVAL; }

    /* count:0 is init in the bsp_usb_console_init */
    if (count > 1)
    {
        gs_tty_driver = alloc_tty_driver(count - 1);
        if (!gs_tty_driver)
        { return -ENOMEM; }

        gs_tty_driver->driver_name = "g_serial";
        gs_tty_driver->name = PREFIX;

        /* uses dynamically assigned dev_t values */

        gs_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
        gs_tty_driver->subtype = SERIAL_TYPE_NORMAL;
        gs_tty_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
        gs_tty_driver->init_termios = tty_std_termios;

        /* 9600-8-N-1 ... matches defaults expected by "usbser.sys" on
         * MS-Windows.  Otherwise, most of these flags shouldn't affect
         * anything unless we were to actually hook up to a serial line.
         */
        gs_tty_driver->init_termios.c_cflag =
            B9600 | CS8 | CREAD | HUPCL | CLOCAL;
        gs_tty_driver->init_termios.c_ispeed = 9600;
        gs_tty_driver->init_termios.c_ospeed = 9600;
        gs_tty_driver->num = 1;

        coding.dwDTERate = cpu_to_le32(9600);
        coding.bCharFormat = 8;
        coding.bParityType = USB_CDC_NO_PARITY;
        coding.bDataBits = USB_CDC_1_STOP_BITS;

        tty_set_operations(gs_tty_driver, &gs_tty_ops);

        /* export the driver ... */
        status = tty_register_driver(gs_tty_driver);
        if (status)
        {
            DBG_E(MBB_ACM,"%s: cannot register, err %d\n",
                   __func__, status);
            goto fail;
        }
    }

    /* make devices be openable */
    for (i = 1; i < count; i++)
    {
        mutex_init(&ports[i].lock);
        status = gs_port_alloc(i, &coding);
        if (status)
        {
            count = i;
            goto fail;
        }
    }
    n_ports = count;

    /* 1. reg tty device for console tty */
    //tty_dev = tty_register_device(gs_console_tty_driver, 0, &g->dev);
    /* ... and sysfs class devices, so mdev/udev make /dev/ttyGS* */
    //tty_dev = tty_port_register_device(&ports[0].port->port, gs_console_tty_driver, 0, &g->dev);
    //if (IS_ERR(tty_dev))
    //{
    //    DBG_W(MBB_ACM,"%s: no classdev for port %d, err %ld\n",__func__, 0, PTR_ERR(tty_dev));
    //}


    /* 2. reg other tty ... */
    /* ... and sysfs class devices, so mdev/udev make /dev/ttyGS* */
    for (i = 1; i < count; i++)
    {
        //struct device    *tty_dev;

        /* i-1 means index start from 0 */
        //tty_dev = tty_register_device(gs_tty_driver, i - 1, &g->dev);
        tty_dev = tty_port_register_device(&ports[i].port->port, gs_tty_driver, i - 1, &g->dev);
        if (IS_ERR(tty_dev))
        {
            DBG_W(MBB_ACM,"%s: no classdev for port %d, err %ld\n",__func__,
                       i, PTR_ERR(tty_dev));
        }

    }

    DBG_I(MBB_ACM,"%s: registered %d ttyGS* device%s\n", __func__,
              count, (count == 1) ? "" : "s");
    

    return status;
fail:
    /* never free console tty port */
    while (count-- > 1)
    { kfree(ports[count].port); }
    put_tty_driver(gs_tty_driver);
    gs_tty_driver = NULL;
    return status;
}

static USB_INT gs_closed(struct gs_port* port)
{
    USB_INT cond;

    spin_lock_irq(&port->port_lock);
    cond = (port->open_count == 0) && !port->openclose;
    spin_unlock_irq(&port->port_lock);
    return cond;
}

/**
 * gserial_cleanup - remove TTY-over-USB driver and devices
 * Context: may sleep
 *
 * This is called to free all resources allocated by @gserial_setup().
 * Accordingly, it may need to wait until some open /dev/ files have
 * closed.
 *
 * The caller must have issued @gserial_disconnect() for any ports
 * that had previously been connected, so that there is never any
 * I/O pending when it's called.
 */
USB_VOID gserial_cleanup(USB_VOID)
{
    USB_UINT    i;
    struct gs_port*    port;

    /* don't need to free console tty - port0 */
    if (gs_console_tty_driver)
    { tty_unregister_device(gs_console_tty_driver, 0); }

    if (!gs_tty_driver)
    { return; }

    /* start sysfs and /dev/ttyGS* node removal */
    for (i = 1; i < n_ports; i++)
    { tty_unregister_device(gs_tty_driver, i - 1); }

    for (i = 1; i < n_ports; i++)
    {
        /* prevent new opens */
        mutex_lock(&ports[i].lock);
        port = ports[i].port;
        ports[i].port = NULL;
        mutex_unlock(&ports[i].lock);

        tasklet_kill(&port->push);

        /* wait for old opens to finish */
        wait_event_interruptible(port->close_wait, gs_closed(port));

        WARN_ON(port->port_usb != NULL);
        tty_port_destroy(&port->port);
        kfree(port);
    }
    n_ports = 0;

    tty_unregister_driver(gs_tty_driver);
    put_tty_driver(gs_tty_driver);
    gs_tty_driver = NULL;
    DBG_I(MBB_ACM,"%s: cleaned up ttyGS* support\n", __func__);
}

/**
 * gserial_connect - notify TTY I/O glue that USB link is active
 * @gser: the function, set up with endpoints and descriptors
 * @port_num: which port is active
 * Context: any (usually from irq)
 *
 * This is called activate endpoints and let the TTY layer know that
 * the connection is active ... not unlike "carrier detect".  It won't
 * necessarily start I/O queues; unless the TTY is held open by any
 * task, there would be no point.  However, the endpoints will be
 * activated so the USB host can perform I/O, subject to basic USB
 * hardware flow control.
 *
 * Caller needs to have set up the endpoints and USB function in @dev
 * before calling this, as well as the appropriate (speed-specific)
 * endpoint descriptors, and also have set up the TTY driver by calling
 * @gserial_setup().
 *
 * Returns negative errno or zero.
 * On success, ep->driver_data will be overwritten.
 */
USB_INT gserial_connect(struct gserial* gser, USB_UINT8 port_num)
{
    struct gs_port*    port;
    USB_ULONG    flags;
    USB_INT        status;

    /* port 0 is console tty dirver */
    if (0 == port_num)
    {
        if (!gs_console_tty_driver)
        { return -ENXIO; }
    }
    else
    {
        if (!gs_tty_driver || port_num >= n_ports)
        { return -ENXIO; }
    }

    /* we "know" gserial_cleanup() hasn't been called */
    port = ports[port_num].port;

    /* mask the not ready interrupt for usb netcard class function driver */
    gser->out->enable_xfer_in_progress = 1;

    /* activate the endpoints */
    status = usb_ep_enable(gser->in);
    if (status < 0)
    { return status; }
    gser->in->driver_data = port;

    status = usb_ep_enable(gser->out);
    if (status < 0)
    { goto fail_out; }
    gser->out->driver_data = port;

    /* then tell the tty glue that I/O can work */
    spin_lock_irqsave(&port->port_lock, flags);
    gser->ioport = (void*)port;
    port->port_usb = gser;

    /* REVISIT unclear how best to handle this state...
     * we don't really couple it with the Linux TTY.
     */
    gser->port_line_coding = port->port_line_coding;

    /* REVISIT if waiting on "carrier detect", signal. */


    /* if it's already open, start I/O ... and notify the serial
     * protocol about open/close status (connect/disconnect).
     */
    gs_start_io(port);/* usb rx fifo is shared, so must submit rx req at any time */
#if ACM_TTY_SUPPORT_NOTIFY
    if (port->open_count)
    {
        DBG_I(MBB_ACM,"gserial_connect: start ttyGS%d\n", port->port_num);
        //gs_start_io(port);
        if (gser->connect)
        { gser->connect(gser); }
    }
    else
    {
        if (gser->disconnect)
        { gser->disconnect(gser); }
    }
#endif

    spin_unlock_irqrestore(&port->port_lock, flags);

    port->in_name = (USB_PCHAR)gser->in->name;
    port->out_name = (USB_PCHAR)gser->out->name;
    port->stat_port_is_connect = 1;
    return status;

fail_out:
    usb_ep_disable(gser->in);
    gser->in->driver_data = NULL;
    port->stat_port_is_connect = 0;
    return status;
}

/**
 * gserial_disconnect - notify TTY I/O glue that USB link is inactive
 * @gser: the function, on which gserial_connect() was called
 * Context: any (usually from irq)
 *
 * This is called to deactivate endpoints and let the TTY layer know
 * that the connection went inactive ... not unlike "hangup".
 *
 * On return, the state is as if gserial_connect() had never been called;
 * there is no active USB I/O on these endpoints.
 */
USB_VOID gserial_disconnect(struct gserial* gser)
{
    struct gs_port*    port = gser->ioport;
    USB_ULONG    flags;

    if (!port)
    { return; }

    /* tell the TTY glue not to do I/O here any more */
    spin_lock_irqsave(&port->port_lock, flags);

    /* REVISIT as above: how best to track this? */
    port->port_line_coding = gser->port_line_coding;

    port->port_usb = NULL;
    gser->ioport = NULL;
    if (port->open_count > 0 || port->openclose)
    {
        wake_up_interruptible(&port->drain_wait);
        //if (port->port_tty)
        //{ tty_hangup(port->port_tty); }
    }
    spin_unlock_irqrestore(&port->port_lock, flags);

    /* disable endpoints, aborting down any active I/O */
    usb_ep_disable(gser->out);
    gser->out->driver_data = NULL;

    usb_ep_disable(gser->in);
    gser->in->driver_data = NULL;

    /* finally, free any unused/unusable I/O buffers */
    spin_lock_irqsave(&port->port_lock, flags);
    if (port->open_count == 0 && !port->openclose)
    { gs_buf_free(&port->port_write_buf); }
    gs_free_requests(gser->out, &port->read_pool, NULL);
    gs_free_requests(gser->out, &port->read_queue, NULL);
    gs_free_requests(gser->in, &port->write_pool, NULL);

    port->read_allocated = 0;
    port->read_started = 0;
    port->write_allocated = 0;
    port->write_started = 0;
    spin_unlock_irqrestore(&port->port_lock, flags);

	port->stat_port_is_connect = 0;

    port->is_suspend = 0;
}

USB_INT gserial_line_state(struct gserial *gser, USB_UINT state)
{
    struct gs_port	*port = gser->ioport;

    if (!port) {
        return -ESHUTDOWN;
    }
	port->line_state_on = (state & U_ACM_CTRL_DTR);
	return 0;
}

USB_INT gserial_suspend(struct gserial *gser)
{
    struct gs_port	*port = gser->ioport;

    if (!port) {
        return -ESHUTDOWN;
    }
    port->is_suspend = 1;
    return 0;
}


USB_INT gserial_resume(struct gserial *gser)
{
    struct gs_port	*port = gser->ioport;
    USB_ULONG flags;

    if (!port) {
        return -ESHUTDOWN;
    }

    port->is_suspend = 0;
    spin_lock_irqsave(&port->port_lock, flags);
    gs_start_rx(port);
    spin_unlock_irqrestore(&port->port_lock, flags);
    return 0;
}


/* implement for usb console dev */
#define CONSOLE_BUFFER_SIZE 32*1024

struct usb_console_ctx_info {
    struct gs_buf buf_info;
    spinlock_t lock;
    struct delayed_work write_work;
};

static struct usb_console_ctx_info gs_console_ctx;

/*
 * convert '\n' -> "\r\n"
 */
static USB_INT gs_char_cpy(USB_PCHAR d, USB_PCHAR s, USB_UINT size,
                       USB_UINT max, USB_UINT* dst_size)
{
    USB_INT src_pos = 0;
    USB_INT dst_pos = 0;

    /* max - 2 : reserve the 2 rooms for last "\r\n" */
    while (src_pos < size && dst_pos < max - 2)
    {
        if (*(s + src_pos) == '\n')
        {
            *(d + dst_pos) = '\r';
            *(d + dst_pos + 1) = '\n';
            dst_pos += 2;
            src_pos++;
        }
        else
        {
            *(d + dst_pos) = *(s + src_pos);
            src_pos++;
            dst_pos++;
        }
    }

    *dst_size = dst_pos;
    return src_pos;
}

static USB_INT gs_usb_tty_write(struct gs_port* port,
                            const USB_CHAR *buf, USB_UINT count)
{
    struct list_head*    pool = &port->write_pool;
    struct usb_ep*        in;
    struct usb_request*    req;
    USB_ULONG flags;
    USB_UINT trans_size;
    USB_INT status;

    spin_lock_irqsave(&port->port_lock, flags);
    if (unlikely(!port->open_count))
    {
        status = -EINVAL;
        goto console_write_exit;
    }
#ifdef USB_SOLUTION
	if (unlikely(!port->port_usb)) 
#else
	if (unlikely(!port->port_usb || port->is_suspend)) 
#endif
    {
        status = -ESHUTDOWN;
        port->stat_con_write_not_conn++;
        goto console_write_exit;
    }
    if (unlikely(list_empty(pool)))
    {
        status = -ENOMEM;
        port->stat_con_write_no_mem++;
        goto console_write_exit;
    }
    in = port->port_usb->in;
    req = list_entry(pool->next, struct usb_request, list);
    list_del(&req->list);
    spin_unlock_irqrestore(&port->port_lock, flags);

    count = gs_char_cpy(req->buf, (USB_PCHAR)buf, count, in->maxpacket, &trans_size);
    req->length = trans_size;
    req->zero = (trans_size % in->maxpacket) ? (0) : (1);
    status = usb_ep_queue(in, req, GFP_ATOMIC);
    spin_lock_irqsave(&port->port_lock, flags);
    if (status)
    {
        list_add(&req->list, pool);
        goto console_write_exit;
    }
    port->write_started++;
    status = count;

console_write_exit:
    spin_unlock_irqrestore(&port->port_lock, flags);
    return status;
}

static USB_INT gs_console_write_to_usb(const USB_CHAR *buf, USB_UINT count)
{
    struct gs_port *port = ports[ACM_CONSOLE_IDX].port;

    if (count == 0 || NULL == port)
		return 0;

    while (count) {
		USB_INT ret;
        ret = gs_usb_tty_write(port, buf, count);
        if (ret < 0) {
            return ret;
        }
		buf += ret;
		count -= ret;
	}
	return 0;
}

static USB_VOID gs_console_write(struct console *co,
					const USB_CHAR *buf, USB_UINT count)
{
    struct gs_port *port = ports[ACM_CONSOLE_IDX].port;
    USB_ULONG flags;
    USB_INT no_data = 0;

    if (count == 0 || NULL == port)
		return;

    /* write the data to usb console buffer */
    spin_lock_irqsave(&gs_console_ctx.lock, flags);
    if (gs_buf_space_avail(&gs_console_ctx.buf_info) < count) {
        port->stat_con_write_no_room++;
        no_data = 1;
    }
    (void)gs_buf_put(&gs_console_ctx.buf_info, buf, count);
    spin_unlock_irqrestore(&gs_console_ctx.lock, flags);

    /* we can delay a little time to get the more msg */
    if (!no_data)
        schedule_delayed_work(&gs_console_ctx.write_work, 1);
    return;
}

static USB_VOID gs_console_write_work(struct work_struct *work)
{
    USB_ULONG flags;
    USB_UINT count;
    USB_UINT len;
    struct gs_buf *gb = &gs_console_ctx.buf_info;
    struct gs_port *port = ports[ACM_CONSOLE_IDX].port;

    if (NULL == port)
		return;

    spin_lock_irqsave(&gs_console_ctx.lock, flags);

	count = gs_buf_data_avail(gb);
	if (count == 0) {
		goto write_exit;
	}

	if (count > port->stat_con_data_max)
	    port->stat_con_data_max = count;

	len = gb->buf_buf + gb->buf_size - gb->buf_get;
	if (count > len) {
		(void)gs_console_write_to_usb(gb->buf_get, len);
		(void)gs_console_write_to_usb(gb->buf_buf, count - len);
		gb->buf_get = gb->buf_buf + count - len;
	} else {
		(void)gs_console_write_to_usb(gb->buf_get, count);
		if (count < len)
			gb->buf_get += count;
		else /* count == len */
			gb->buf_get = gb->buf_buf;
	}

write_exit:
	spin_unlock_irqrestore(&gs_console_ctx.lock, flags);
	return;
}

static USB_INT gs_console_setup(struct console* co, USB_CHAR* options)
{
    if (ports[ACM_CONSOLE_IDX].port)
    {
        if (gs_buf_alloc(&gs_console_ctx.buf_info, CONSOLE_BUFFER_SIZE)) {
            return -ENOMEM;
        }
        spin_lock_init(&gs_console_ctx.lock);
        INIT_DELAYED_WORK(&gs_console_ctx.write_work, gs_console_write_work);
    
        return 0;
    }

    return -ENODEV;
}

static struct tty_driver* gs_console_device(struct console* co, USB_INT* index)
{
    struct tty_driver** p = (struct tty_driver**)co->data;

    if (!*p)
    { return NULL; }

    *index = co->index;
    return *p;
}


static struct console gs_console =
{
    .name =        ACM_CONSOLE_NAME,
    .write =    gs_console_write,
    .device =    gs_console_device,
    .setup =    gs_console_setup,
    .flags =    CON_PRINTBUFFER,
    .index =    -1,
    .data =     &gs_console_tty_driver,
};

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  �ж�USB ����̨ʹ�ܽӿ�
*****************************************************************/
USB_INT gs_acm_is_console_enable(USB_VOID)
{
#ifdef USB_SECURITY
    if (1 == usb_port_enable("shell"))
    {
        DBG_I(MBB_ACM,"gs_acm_is_console_enable is enable\n");
        return 1;
    }
    else
    {
        DBG_I(MBB_ACM,"gs_acm_is_console_enable is disable\n");
        return 0;
    }
#else
    /*Ĭ��USB shell ���ܲ�����*/
    DBG_I(MBB_ACM,"gs_acm_is_console_enable is disable\n");
    return 0;
#endif
}

static acm_ctx_t* acm_ctx_ser = NULL;
/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  USB��ʼ��ʱ��
*****************************************************************/
USB_INT bsp_usb_console_init(USB_VOID)
{

    struct usb_cdc_line_coding    coding;
    USB_INT status = 0;
    struct device *tty_dev = NULL;

    DBG_I(MBB_ACM,"bsp_usb_console_init enter\n");
    /* alloc first tty driver for console */
    gs_console_tty_driver = alloc_tty_driver(1);
    if (!gs_console_tty_driver)
    { return -ENOMEM; }
    DBG_I(MBB_ACM,"bsp_usb_console_init 1111\n");
    gs_console_tty_driver->driver_name = "console_ser";
    
    gs_console_tty_driver->name = acm_ctx_ser->tty_name_type_var[ACM_CONSOLE_IDX].name;

    /* uses dynamically assigned dev_t values */

    gs_console_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
    gs_console_tty_driver->subtype = SERIAL_TYPE_NORMAL;
    gs_console_tty_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
    gs_console_tty_driver->init_termios = tty_std_termios;

    /* 9600-8-N-1 ... matches defaults expected by "usbser.sys" on
     * MS-Windows.  Otherwise, most of these flags shouldn't affect
     * anything unless we were to actually hook up to a serial line.
     */
    gs_console_tty_driver->init_termios.c_cflag =
        B9600 | CS8 | CREAD | HUPCL | CLOCAL;
    gs_console_tty_driver->init_termios.c_ispeed = 9600;
    gs_console_tty_driver->init_termios.c_ospeed = 9600;
    gs_console_tty_driver->num = 1;

    coding.dwDTERate = cpu_to_le32(9600);
    coding.bCharFormat = 8;
    coding.bParityType = USB_CDC_NO_PARITY;
    coding.bDataBits = USB_CDC_1_STOP_BITS;

    tty_set_operations(gs_console_tty_driver, &gs_tty_ops);

    /* export the driver ... */
    status = tty_register_driver(gs_console_tty_driver);
    if (status)
    {
        DBG_E(MBB_ACM,"%s: cannot register, err %d\n",
               __func__, status);
        goto console_init_fail;
    }

    mutex_init(&ports[0].lock);
    status = gs_port_alloc(0, &coding);
    if (status)
    {
        goto console_init_fail;
    }
    tty_dev = tty_port_register_device(&ports[0].port->port, gs_console_tty_driver, 0, NULL);
    if (IS_ERR(tty_dev))
    {
        DBG_W(MBB_ACM,"%s: no classdev for port %d, err %ld\n",__func__, 0, PTR_ERR(tty_dev));
    }
    if (gs_acm_is_console_enable())
    {
        /* reg from uart shell to usb shell */
        register_console(&gs_console);
        console_start(&gs_console);
    }

    return 0;

console_init_fail:
    put_tty_driver(gs_console_tty_driver);
    return status;
}

static USB_VOID acm_serial_dump_ep_info(struct gs_port* port)
{
    USB_PCHAR find;
    USB_UINT ep_num;

    if (port->stat_port_is_connect)
    {
        DBG_W(MBB_ACM,"in ep name:\t\t\t <%s>\n", port->in_name);
        find = strstr(port->in_name, "ep");
        if (find)
        {
            /* skip "ep" */
            find += 2;
            ep_num = simple_strtoul(find, NULL, 0);
            DBG_I(MBB_ACM,"in ep num:\t\t\t <%d>\n", ep_num * 2 + 1);
        }
        DBG_I(MBB_ACM,"out ep name:\t\t\t <%s>\n", port->out_name);
        find = strstr(port->out_name, "ep");
        if (find)
        {
            /* skip "ep" */
            find += 2;
            ep_num = simple_strtoul(find, NULL, 0);
            DBG_I(MBB_ACM,"out ep num:\t\t\t <%d>\n", ep_num * 2);
        }
    }
    else
    {
        DBG_E(MBB_ACM,"the acm dev is not connect\n");
    }
}


USB_INT acm_serial_dump(USB_INT idx)
{
    struct gs_port* port;

    if (idx >= ACM_TTY_USED_COUNT)
    {
        DBG_E(MBB_ACM,"serial port num:%d is not valid\n", idx);
        return -1;
    }
    port = ports[ACM_CONSOLE_IDX].port;
    if (NULL == port)
    {
        DBG_E(MBB_ACM,"serial port not alloc\n");
        return -1;
    }

    DBG_E(MBB_ACM,"serial port is connect:       %s\n", (port->port_usb) ? ("connect") : "disconnect");
    DBG_E(MBB_ACM,"console tty name:             %s\n", ACM_CONSOLE_NAME);
    acm_serial_dump_ep_info(port);
    DBG_E(MBB_ACM,"open_count:                   %d\n", port->open_count);
    DBG_E(MBB_ACM,"line_state_on:                %d\n", port->line_state_on);
    DBG_E(MBB_ACM,"is_suspend:                   %d\n", port->is_suspend);
    DBG_E(MBB_ACM,"port_write_buf buf_buf:       0x%x\n", (USB_UINT)port->port_write_buf.buf_buf);
    DBG_E(MBB_ACM,"port_write_buf buf_get:       0x%x\n", (USB_UINT)port->port_write_buf.buf_get);
    DBG_E(MBB_ACM,"port_write_buf buf_put:       0x%x\n", (USB_UINT)port->port_write_buf.buf_put);
    
    DBG_E(MBB_ACM,"port_write_buf buf_size:      %d\n", port->port_write_buf.buf_size);
    
    DBG_E(MBB_ACM,"read_started:                 %d\n", port->read_started);
    DBG_E(MBB_ACM,"n_read:                       %d\n", port->n_read);
    DBG_E(MBB_ACM,"read_allocated:               %d\n", port->read_allocated);
    DBG_E(MBB_ACM,"write_started:                %d\n", port->write_started);
    DBG_E(MBB_ACM,"write_allocated:              %d\n", port->write_allocated);
    DBG_E(MBB_ACM,"stat_con_write_not_conn:      %d\n", port->stat_con_write_not_conn);
    DBG_E(MBB_ACM,"stat_con_write_no_room:       %d\n", port->stat_con_write_no_room);
    DBG_E(MBB_ACM,"stat_con_write_no_mem:        %d\n", port->stat_con_write_no_mem);
    DBG_E(MBB_ACM,"stat_con_data_max:            %d\n", port->stat_con_data_max);
    DBG_E(MBB_ACM,"stat_write_not_conn:          %d\n", port->stat_write_not_conn);
    DBG_E(MBB_ACM,"stat_write_no_mem:            %d\n", port->stat_write_no_mem);
    DBG_E(MBB_ACM,"stat_cannt_push:              %d\n", port->stat_cannt_push);
    DBG_E(MBB_ACM,"stat_insert_tty:              %d\n", port->stat_insert_tty);
    DBG_E(MBB_ACM,"stat_insert_tty_push:         %d\n", port->stat_insert_tty_push);
    DBG_E(MBB_ACM,"port.count:                   %d\n", port->port.count);

    
    return 0;
}

USB_INT acm_serial_mem_dump(USB_PCHAR buffer, USB_UINT buf_size)
{
    USB_UINT need_size;
    struct gs_port*    port;
    USB_INT i;
    USB_PCHAR cur = buffer;

    /* no devs exist */
    if (0 == n_ports)
    {
        return 0;
    }

    need_size = n_ports * sizeof(struct gs_port);
    /* no room left */
    if (need_size > buf_size)
    {
        return -1;
    }

    for (i = 0; i < n_ports; i++)
    {
        port = ports[i].port;
        if (port)
        {
            memcpy(cur, port, sizeof(struct gs_port));
            cur += sizeof(struct gs_port);
        }
    }

    return (USB_INT)need_size;
}

/*****************************************************************
Parameters    :   None
Return        :
Description   :  USB serialģ���ʼ�����
*****************************************************************/
USB_INT usb_serial_init(USB_VOID)
{
    /*����ƽ̨*/
    USB_INT ret_val = 0;

    DBG_I(MBB_ACM,"%s:entry!\n",__func__);
#ifdef MBB_USB_UNITARY_B
    /*��ȡ��ǰconsole�ص�*/
#endif

    /*��ͨ9xN5ƽ̨�ں˲�֧�ֶ�NV����NVֵ��ƽ̨����*/
#ifdef MBB_USB_UNITARY_Q
    /*��ȡ��ǰconsole�ص�*/
    //usb_get_console_cb = xxx;
#endif
    acm_ctx_ser = acm_get_ctx();
    bsp_usb_console_init();

    return ret_val;
}


