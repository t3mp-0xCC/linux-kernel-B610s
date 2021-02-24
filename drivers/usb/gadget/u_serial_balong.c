/*
 * u_serial.c - utilities for USB gadget "serial port"/TTY support
 *
 * Copyright (C) 2003 Al Borchers (alborchers@steinerpoint.com)
 * Copyright (C) 2008 David Brownell
 * Copyright (C) 2008 by Nokia Corporation
 * Copyright (C) 2013 by Huawei Technologies Co., Ltd.
 * 2013-06-07 - Adopt&bugfix for hisi balong platform
 * Zhongshun Wang, Yong Jing, Xinli Wang, Dongyue Chen, Changdong Fan<foss@huawei.com>
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
#include <linux/module.h>
#include <linux/console.h>
#include <linux/usb/bsp_usb.h>

#include "usb_vendor.h"
#include "u_serial_balong.h"

struct console gs_console;

/*
 * This component encapsulates the TTY layer glue needed to provide basic
 * "serial port" functionality through the USB gadget stack.  Each such
 * port is exposed through a /dev/ttyGS* node.
 *
 * After this module has been loaded, the individual TTY port can be requested
 * (gserial_alloc_line()) and it will stay available until they are removed
 * (gserial_free_line()). Each one may be connected to a USB function
 * (gserial_connect), or disconnected (with gserial_disconnect) when the USB
 * host issues a config change event. Data can only flow when the port is
 * connected to the host.
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

#define PREFIX	"tty_comm"

/*
 * gserial is the lifecycle interface, used by USB functions
 * gs_port is the I/O nexus, used by the tty driver
 * tty_struct links to the tty/filesystem framework
 *
 * gserial <---> gs_port ... links will be null when the USB link is
 * inactive; managed by gserial_{connect,disconnect}().  each gserial
 * instance can wrap its own USB control protocol.
 *	gserial->ioport == usb_ep->driver_data ... gs_port
 *	gs_port->port_usb ... gserial
 *
 * gs_port <---> tty_struct ... links will be null when the TTY file
 * isn't opened; managed by gs_open()/gs_close()
 *	gserial->port_tty ... tty_struct
 *	tty_struct->driver_data ... gserial
 */

/* RX and TX queues can buffer QUEUE_SIZE packets before they hit the
 * next layer of buffering.  For TX that's a circular buffer; for RX
 * consider it a NOP.  A third layer is provided by the TTY code.
 */
#define QUEUE_SIZE		    16
#define WRITE_BUF_SIZE		8192		/* TX only */
#define READ_BUF_SIZE       2048
#define WRITE_LEFT_MIN      128

/* circular buffer */
struct gs_buf {
	unsigned		buf_size;
	char			*buf_buf;
	char			*buf_get;
	char			*buf_put;
};

/*
 * The port structure holds info for each port, one for each minor number
 * (and thus for each /dev/ node).
 */
struct gs_port {
	int offset;
	struct tty_port		port;
	spinlock_t		port_lock;	/* guard port_* access */

	struct gserial		*port_usb;
	struct list_head	read_pool;
	struct list_head	read_queue;
	struct tasklet_struct	push;
	
	struct list_head	write_pool;
	struct gs_buf		port_write_buf;
	wait_queue_head_t	drain_wait; /* wait while writes drain */
	struct usb_cdc_line_coding port_line_coding;	/* 8-N-1 etc */

	bool			openclose;	/* open/close in progress */
	u8			port_num;
	
	int read_started;
	int read_allocated;
	
	unsigned		n_read;
	
	int write_started;
	int write_allocated;	

	/* REVISIT this state ... */
	
	unsigned line_state_on;
	unsigned is_suspend;

	unsigned stat_write_no_mem;
	unsigned stat_con_write_no_room;//10
	unsigned stat_write_not_conn;
	unsigned stat_con_write_no_mem;
	unsigned stat_con_write_not_conn;
	unsigned stat_port_is_connect;
	unsigned stat_cannt_push;
	unsigned stat_con_data_max;
    unsigned stat_wait_port_close;
    unsigned stat_port_closed;
	char* in_name;
	char* out_name;
};

static struct portmaster {
	struct mutex	lock;			/* protect open/close */
	struct gs_port	*port;
} ports[ACM_TTY_COUNT];

#define GS_CLOSE_TIMEOUT		15		/* seconds */

extern struct acm_name_type_tbl g_acm_tty_type_table[ACM_TTY_COUNT];

#ifdef VERBOSE_DEBUG
#ifndef pr_vdebug
#define pr_vdebug(fmt, arg...) \
	//pr_debug(fmt, ##arg)
#endif /* pr_vdebug */
#else
#ifndef pr_vdebug
#define pr_vdebug(fmt, arg...) \
	//({ if (0) pr_debug(fmt, ##arg); })
#endif /* pr_vdebug */
#endif

static int gs_acm_is_console_enable(void);

/*-------------------------------------------------------------------------*/

/* Circular Buffer */

/*
 * gs_buf_alloc
 *
 * Allocate a circular buffer and all associated memory.
 */
static int gs_buf_alloc(struct gs_buf *gb, unsigned size)
{
	gb->buf_buf = kmalloc(size, GFP_KERNEL);
	if (gb->buf_buf == NULL)
		return -ENOMEM;

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
static void gs_buf_free(struct gs_buf *gb)
{
    if (gb->buf_buf) {
    	kfree(gb->buf_buf);
    	gb->buf_buf = NULL;
	}
}

/*
 * gs_buf_clear
 *
 * Clear out all data in the circular buffer.
 */
static void gs_buf_clear(struct gs_buf *gb)
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
static unsigned gs_buf_data_avail(struct gs_buf *gb)
{
	return (gb->buf_size + gb->buf_put - gb->buf_get) % gb->buf_size;
}

/*
 * gs_buf_space_avail
 *
 * Return the number of bytes of space available in the circular
 * buffer.
 */
static unsigned gs_buf_space_avail(struct gs_buf *gb, int line_state_on)
{
    int len;
	len = (gb->buf_size + gb->buf_get - gb->buf_put - 1) % gb->buf_size;

	/* if the left romm is not enough, and cosole is not connect,
	 * overwrite the old data
	 */
	if (!line_state_on && len < WRITE_LEFT_MIN) {
	    gb->buf_put = gb->buf_buf;
	    gb->buf_get = gb->buf_buf;
	    len = gb->buf_size - 1;
	}
	return len;
}

/*
 * gs_buf_put
 *
 * Copy data data from a user buffer and put it into the circular buffer.
 * Restrict to the amount of space available.
 *
 * Return the number of bytes copied.
 */
static unsigned
gs_buf_put(struct gs_buf *gb, const char *buf, unsigned count, int line_on)
{
	unsigned len;

	len  = gs_buf_space_avail(gb, line_on);
	if (count > len)
		count = len;

	if (count == 0)
		return 0;

	len = gb->buf_buf + gb->buf_size - gb->buf_put;
	if (count > len) {
		memcpy(gb->buf_put, buf, len);
		memcpy(gb->buf_buf, buf+len, count - len);
		gb->buf_put = gb->buf_buf + count - len;
	} else {
		memcpy(gb->buf_put, buf, count);
		if (count < len)
			gb->buf_put += count;
		else /* count == len */
			gb->buf_put = gb->buf_buf;
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
static unsigned
gs_buf_get(struct gs_buf *gb, char *buf, unsigned count)
{
	unsigned len;

	len = gs_buf_data_avail(gb);
	if (count > len)
		count = len;

	if (count == 0)
		return 0;

	len = gb->buf_buf + gb->buf_size - gb->buf_get;
	if (count > len) {
		memcpy(buf, gb->buf_get, len);
		memcpy(buf+len, gb->buf_buf, count - len);
		gb->buf_get = gb->buf_buf + count - len;
	} else {
		memcpy(buf, gb->buf_get, count);
		if (count < len)
			gb->buf_get += count;
		else /* count == len */
			gb->buf_get = gb->buf_buf;
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
struct usb_request *
gs_alloc_req(struct usb_ep *ep, unsigned len, gfp_t kmalloc_flags)
{
	struct usb_request *req;

	req = usb_ep_alloc_request(ep, kmalloc_flags);

	if (req != NULL) {
		req->length = len;
        /* if len is 0, alloc the empty req */
        if (0 == len) {
            req->buf = NULL;
            req->dma = (dma_addr_t)NULL;
        }
        else {
    		req->buf = kmalloc(len, kmalloc_flags);
    		if (req->buf == NULL) {
    			usb_ep_free_request(ep, req);
    			return NULL;
    	    }
        }
	}

	return req;
}
EXPORT_SYMBOL_GPL(gs_alloc_req);

/*
 * gs_free_req
 *
 * Free a usb_request and its buffer.
 */
void gs_free_req(struct usb_ep *ep, struct usb_request *req)
{
    if (req->buf)
	    kfree(req->buf);
	usb_ep_free_request(ep, req);
}
EXPORT_SYMBOL_GPL(gs_free_req);

/*
 * gs_send_packet
 *
 * If there is data to send, a packet is built in the given
 * buffer and the size is returned.  If there is no data to
 * send, 0 is returned.
 *
 * Called with port_lock held.
 */
static unsigned
gs_send_packet(struct gs_port *port, char *packet, unsigned size)
{
	unsigned len;

	len = gs_buf_data_avail(&port->port_write_buf);
	if (len < size)
		size = len;
	if (size != 0)
		size = gs_buf_get(&port->port_write_buf, packet, size);
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
static int gs_start_tx(struct gs_port *port)
/*
__releases(&port->port_lock)
__acquires(&port->port_lock)
*/
{
	/* coverity[deref_ptr] */
	struct list_head	*pool = &port->write_pool;
	struct usb_ep		*in = port->port_usb->in;
	int			status = 0;
	bool			do_tty_wake = false;

	while (!list_empty(pool)) {
		struct usb_request	*req;
		int			len;

		if (port->write_started >= QUEUE_SIZE)
			break;

		req = list_entry(pool->next, struct usb_request, list);
		len = gs_send_packet(port, req->buf, in->maxpacket);
		if (len == 0) {
			wake_up_interruptible(&port->drain_wait);
			break;
		}
		do_tty_wake = true;

		req->length = len;
		list_del(&req->list);
		req->zero = (len % in->maxpacket) ? (0) : (1);
		//req->zero = (gs_buf_data_avail(&port->port_write_buf) == 0);

		pr_vdebug(PREFIX "%d: tx len=%d, 0x%02x 0x%02x 0x%02x ...\n",
				port->port_num, len, *((u8 *)req->buf),
				*((u8 *)req->buf+1), *((u8 *)req->buf+2));

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

		if (status) {
			pr_vdebug("%s: %s %s err %d\n",
					__func__, "queue", in->name, status);
			list_add(&req->list, pool);
			break;
		}

		port->write_started++;

		/* abort immediately after disconnect */
		if (!port->port_usb)
			break;
	}

	if (do_tty_wake && port->port.tty)
		tty_wakeup(port->port.tty);
	return status;
}

/*
 * Context: caller owns port_lock, and port_usb is set
 */
static unsigned gs_start_rx(struct gs_port *port)
/*
__releases(&port->port_lock)
__acquires(&port->port_lock)
*/
{
	struct list_head	*pool = &port->read_pool;
	/* coverity[deref_ptr] */
	struct usb_ep		*out = port->port_usb->out;

	while (!list_empty(pool)) {
		struct usb_request	*req;
		int			status;
		/* struct tty_struct	*tty; */

		/* no more rx if closed */
		/*tty = port->port.tty;
		if (!tty)
			break;
		*/

		if (port->read_started >= QUEUE_SIZE)
			break;

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

		if (status) {
			pr_vdebug("%s: %s %s err %d\n",
					__func__, "queue", out->name, status);
			list_add(&req->list, pool);
			break;
		}
		port->read_started++;

		/* abort immediately after disconnect */
		if (!port->port_usb)
			break;
	}
	return port->read_started;/* [false alarm]:fortify disable */
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
static void gs_rx_push(unsigned long _port)
{
	struct gs_port		*port = (void *)_port;
	struct tty_struct	*tty;
	struct list_head	*queue = &port->read_queue;
	bool			disconnect = false;
	bool			do_push = false;

	/* hand any queued data to the tty */
	spin_lock_irq(&port->port_lock);
	tty = port->port.tty;
	while (!list_empty(queue)) {
		struct usb_request	*req;

		req = list_first_entry(queue, struct usb_request, list);

		/* leave data queued if tty was rx throttled */
		if (tty && test_bit(TTY_THROTTLED, &tty->flags))
			break;

		switch (req->status) {
		case -ESHUTDOWN:
			disconnect = true;
			pr_vdebug(PREFIX "%d: shutdown\n", port->port_num);
			break;

		default:
			/* presumably a transient fault */
			pr_warning(PREFIX "%d: unexpected RX status %d\n",
					port->port_num, req->status);
			/* FALLTHROUGH */
		case 0:
			/* normal completion */
			break;
		}

		/* push data to (open) tty */
		if (req->actual) {
			char		*packet = req->buf;
			unsigned	size = req->actual;
			unsigned	n;
			int		count;

			/* we may have pushed part of this packet already... */
			n = port->n_read;
			if (n) {
				packet += n;
				size -= n;
			}

			count = tty_insert_flip_string(&port->port, packet,
					size);
			if (count)
				do_push = true;
			if (count != size) {
				/* stop pushing; TTY layer can't handle more */
				port->stat_cannt_push++;
				port->n_read += count;
				pr_vdebug(PREFIX "%d: rx block %d/%d\n",
						port->port_num,
						count, req->actual);
				break;
			}
			port->n_read = 0;
		}

		list_move(&req->list, &port->read_pool);
		port->read_started--;
	}

	/* Push from tty to ldisc; without low_latency set this is handled by
	 * a workqueue, so we won't get callbacks and can hold port_lock
	 */
	if (do_push)
		tty_flip_buffer_push(&port->port);


	/* We want our data queue to become empty ASAP, keeping data
	 * in the tty and ldisc (not here).  If we couldn't push any
	 * this time around, there may be trouble unless there's an
	 * implicit tty_unthrottle() call on its way...
	 *
	 * REVISIT we should probably add a timer to keep the tasklet
	 * from starving ... but it's not clear that case ever happens.
	 */
	if (!list_empty(queue) && tty) {
		if (!test_bit(TTY_THROTTLED, &tty->flags)) {
			if (do_push)
				tasklet_schedule(&port->push);
			else
				pr_warning(PREFIX "%d: RX not scheduled?\n",
					port->port_num);
		}
	}

	/* If we're still connected, refill the USB RX queue. */
	if (!disconnect && port->port_usb) {
		gs_start_rx(port);
    }

	spin_unlock_irq(&port->port_lock);
}

static void gs_read_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct gs_port	*port = ep->driver_data;

	/* Queue all received data until the tty layer is ready for it. */
	spin_lock(&port->port_lock);

    /* we start rx in connect, so if usr not open the dev, drop the rx data */
	if (!port->port.count) {
	    list_add_tail(&req->list, &port->read_pool);
	    port->read_started--;
	}
	else {
    	list_add_tail(&req->list, &port->read_queue);
	}
	tasklet_schedule(&port->push);
	spin_unlock(&port->port_lock);
}

static void gs_write_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct gs_port	*port = ep->driver_data;

	spin_lock(&port->port_lock);
	list_add(&req->list, &port->write_pool);
	port->write_started--;

	switch (req->status) {
	default:
		/* presumably a transient fault */
		pr_warning("%s: unexpected %s status %d\n",
				__func__, ep->name, req->status);
		/* FALL THROUGH */
	case 0:
		/* normal completion */
		gs_start_tx(port);
		break;

	case -ESHUTDOWN:
		/* disconnect */
		pr_vdebug("%s: %s shutdown\n", __func__, ep->name);
		break;
	}

	spin_unlock(&port->port_lock);
}

static void gs_free_requests(struct usb_ep *ep, struct list_head *head,
							 int *allocated)
{
	struct usb_request	*req;

	while (!list_empty(head)) {
		req = list_entry(head->next, struct usb_request, list);
		list_del(&req->list);
		gs_free_req(ep, req);
		if (allocated)
			(*allocated)--;
	}
}

static int gs_alloc_requests(struct usb_ep *ep, struct list_head *head,
		void (*fn)(struct usb_ep *, struct usb_request *),
		int *allocated, bool is_in)
{
	int			i;
	struct usb_request	*req;
	int n = allocated ? QUEUE_SIZE - *allocated : QUEUE_SIZE;
	unsigned alloc_size = is_in ? ep->maxpacket : READ_BUF_SIZE;

	/* Pre-allocate up to QUEUE_SIZE transfers, but if we can't
	 * do quite that many this time, don't fail ... we just won't
	 * be as speedy as we might otherwise be.
	 */
	for (i = 0; i < n; i++) {
	    /* for lrz bug: if push size is not correct, lrz can't work */
		/*req = gs_alloc_req(ep, ep->maxpacket, GFP_ATOMIC);*/
		req = gs_alloc_req(ep, alloc_size, GFP_ATOMIC);
		if (!req)
			return list_empty(head) ? -ENOMEM : 0;
		req->complete = fn;
		list_add_tail(&req->list, head);
		if (allocated)
			(*allocated)++;
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
static int gs_start_io(struct gs_port *port)
{
	struct list_head	*head = &port->read_pool;
	struct usb_ep		*ep = port->port_usb->out;
	int			status;
	unsigned		started;

	/* Allocate RX and TX I/O buffers.  We can't easily do this much
	 * earlier (with GFP_KERNEL) because the requests are coupled to
	 * endpoints, as are the packet sizes we'll be using.  Different
	 * configurations may use different endpoints with a given port;
	 * and high speed vs full speed changes packet sizes too.
	 */
	status = gs_alloc_requests(ep, head, gs_read_complete,
		&port->read_allocated, false);
	if (status)
		return status;

	status = gs_alloc_requests(port->port_usb->in, &port->write_pool,
			gs_write_complete, &port->write_allocated, true);
	if (status) {
		gs_free_requests(ep, head, &port->read_allocated);
		return status;
	}

	/* queue read requests */
	port->n_read = 0;
	started = gs_start_rx(port);

	/* unblock any pending writes into our circular buffer */
	if (started) {
		if (port->port.tty)
			tty_wakeup(port->port.tty);
	} else {
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
static int gs_open(struct tty_struct *tty, struct file *file)
{
	int		port_num = tty->index;
	struct gs_port	*port;
	int		status;

	do {
		mutex_lock(&ports[port_num].lock);
		port = ports[port_num].port;
		if (!port) {
		    status = -ENODEV;
		}
		else {
			spin_lock_irq(&port->port_lock);

			/* already open?  Great. */
			if (port->port.count) {
				status = 0;
				port->port.count++;

			/* currently opening/closing? wait ... */
			} else if (port->openclose) {
				status = -EBUSY;

			/* ... else we do the work */
			} else {
				status = -EAGAIN;
				port->openclose = true;
			}
			spin_unlock_irq(&port->port_lock);
		}
		mutex_unlock(&ports[port_num].lock);

		switch (status) {
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
	} while (status != -EAGAIN);

	/* Do the "real open" */
	spin_lock_irq(&port->port_lock);

	/* allocate circular buffer on first open */
	if (port->port_write_buf.buf_buf == NULL) {

		spin_unlock_irq(&port->port_lock);
		status = gs_buf_alloc(&port->port_write_buf, WRITE_BUF_SIZE);
		spin_lock_irq(&port->port_lock);

		if (status) {
			pr_vdebug("gs_open: ttyGS%d (%p,%p) no buffer\n",
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

	port->port.count = 1;
	port->openclose = false;

	/* if connected, start the I/O stream */
	if (port->port_usb) {
#if ACM_TTY_SUPPORT_NOTIFY
		struct gserial	*gser = port->port_usb;
		pr_vdebug("gs_open: start ttyGS%d\n", port->port_num);

		if (gser->connect)
			gser->connect(gser);
#endif
        gs_start_io(port);
	}

	pr_vdebug("gs_open: ttyGS%d (%p,%p)\n", port->port_num, tty, file);

	status = 0;

exit_unlock_port:
	spin_unlock_irq(&port->port_lock);
	return status;
}

static int gs_writes_finished(struct gs_port *p)
{
	int cond;

	/* return true on disconnect or empty buffer */
	spin_lock_irq(&p->port_lock);
	cond = (p->port_usb == NULL) || !gs_buf_data_avail(&p->port_write_buf);
	spin_unlock_irq(&p->port_lock);

	return cond;
}

static void gs_close(struct tty_struct *tty, struct file *file)
{
	struct gs_port *port = tty->driver_data;
	struct gserial	*gser;

	spin_lock_irq(&port->port_lock);

	if (port->port.count != 1) {
		if (port->port.count == 0)
			WARN_ON(1);
		else
			--port->port.count;
		goto exit;
	}

	pr_vdebug("gs_close: ttyGS%d (%p,%p) ...\n", port->port_num, tty, file);

	/* mark port as closing but in use; we can drop port lock
	 * and sleep if necessary
	 */
	port->openclose = true;
	port->port.count = 0;

	gser = port->port_usb;

#if ACM_TTY_SUPPORT_NOTIFY
	if (gser && gser->disconnect)
		gser->disconnect(gser);
#endif

	/* wait for circular write buffer to drain, disconnect, or at
	 * most GS_CLOSE_TIMEOUT seconds; then discard the rest
	 */
	if (gs_buf_data_avail(&port->port_write_buf) > 0 && gser) {
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
		gs_buf_free(&port->port_write_buf);
	else
		gs_buf_clear(&port->port_write_buf);

	tty->driver_data = NULL;
	port->port.tty = NULL;

	port->openclose = false;

	pr_vdebug("gs_close: ttyGS%d (%p,%p) done!\n",
			port->port_num, tty, file);

	wake_up(&port->port.close_wait);
exit:
	spin_unlock_irq(&port->port_lock);
}

static int gs_write(struct tty_struct *tty, const unsigned char *buf, int count)
{
	struct gs_port	*port = tty->driver_data;
	unsigned long	flags;
	int		status;

	pr_vdebug("gs_write: ttyGS%d (%p) writing %d bytes\n",
			port->port_num, tty, count);

	spin_lock_irqsave(&port->port_lock, flags);
	if (unlikely(!port->port.count)) {
	    spin_unlock_irqrestore(&port->port_lock, flags);
        return -EINVAL;
    }
	/* for lpm remote wakeup support */
	if (unlikely(!port->port_usb/* || port->is_suspend*/)) {
	    port->stat_write_not_conn++;
	    spin_unlock_irqrestore(&port->port_lock, flags);
        return -ESHUTDOWN;
    }
	/*
	 * we don't have enough room for some data
	 * may be the host don't start read, so drop the data.
	 * otherwise the printed thread may pending forever.
	 */
	if (!port->line_state_on && list_empty(&port->write_pool)) {
	    port->stat_write_no_mem++;
	    spin_unlock_irqrestore(&port->port_lock, flags);
	    return -ENOMEM;
	}
	if (count)
		count = gs_buf_put(&port->port_write_buf, buf, count, port->line_state_on);
	/* treat count == 0 as flush_chars() */
	if (port->port_usb)
		status = gs_start_tx(port);/* [false alarm]:fortify disable */
	spin_unlock_irqrestore(&port->port_lock, flags);

	return count;
}

static int gs_put_char(struct tty_struct *tty, unsigned char ch)
{
	struct gs_port	*port = tty->driver_data;
	unsigned long	flags;
	int		status;

	pr_vdebug("gs_put_char: (%d,%p) char=0x%x, called from %pf\n",
		port->port_num, tty, ch, __builtin_return_address(0));

	spin_lock_irqsave(&port->port_lock, flags);
	status = gs_buf_put(&port->port_write_buf, &ch, 1, port->line_state_on);
	spin_unlock_irqrestore(&port->port_lock, flags);

	return status;
}

static void gs_flush_chars(struct tty_struct *tty)
{
	struct gs_port	*port = tty->driver_data;
	unsigned long	flags;

	pr_vdebug("gs_flush_chars: (%d,%p)\n", port->port_num, tty);

	spin_lock_irqsave(&port->port_lock, flags);
	if (port->port_usb)
		gs_start_tx(port);
	spin_unlock_irqrestore(&port->port_lock, flags);
}

static int gs_write_room(struct tty_struct *tty)
{
	struct gs_port	*port = tty->driver_data;
	unsigned long	flags;
	/* if usb not connect, return room available;
	   to avoid thread in sleep */
	int		room = WRITE_BUF_SIZE - 1;

	spin_lock_irqsave(&port->port_lock, flags);
	if (port->port_usb)
		room = gs_buf_space_avail(&port->port_write_buf, port->line_state_on);
	spin_unlock_irqrestore(&port->port_lock, flags);

	pr_vdebug("gs_write_room: (%d,%p) room=%d\n",
		port->port_num, tty, room);

	return room;
}

static int gs_chars_in_buffer(struct tty_struct *tty)
{
	struct gs_port	*port = tty->driver_data;
	unsigned long	flags;
	int		chars = 0;

	spin_lock_irqsave(&port->port_lock, flags);
	chars = gs_buf_data_avail(&port->port_write_buf);
	spin_unlock_irqrestore(&port->port_lock, flags);

	pr_vdebug("gs_chars_in_buffer: (%d,%p) chars=%d\n",
		port->port_num, tty, chars);

	return chars;
}

/* undo side effects of setting TTY_THROTTLED */
static void gs_unthrottle(struct tty_struct *tty)
{
	struct gs_port		*port = tty->driver_data;
	unsigned long		flags;

	spin_lock_irqsave(&port->port_lock, flags);
	if (port->port_usb) {
		/* Kickstart read queue processing.  We don't do xon/xoff,
		 * rts/cts, or other handshaking with the host, but if the
		 * read queue backs up enough we'll be NAKing OUT packets.
		 */
		tasklet_schedule(&port->push);
		pr_vdebug(PREFIX "%d: unthrottle\n", port->port_num);
	}
	spin_unlock_irqrestore(&port->port_lock, flags);
}

static int gs_break_ctl(struct tty_struct *tty, int duration)
{
	struct gs_port	*port = tty->driver_data;
	int		status = 0;
	struct gserial	*gser;

	pr_vdebug("gs_break_ctl: ttyGS%d, send break (%d) \n",
			port->port_num, duration);

	spin_lock_irq(&port->port_lock);
	gser = port->port_usb;
	if (gser && gser->send_break)
		status = gser->send_break(gser, duration);
	spin_unlock_irq(&port->port_lock);

	return status;
}

static const struct tty_operations gs_tty_ops = {
	.open =			gs_open,
	.close =		gs_close,
	.write =		gs_write,
	.put_char =		gs_put_char,
	.flush_chars =		gs_flush_chars,
	.write_room =		gs_write_room,
	.chars_in_buffer =	gs_chars_in_buffer,
	.unthrottle =		gs_unthrottle,
	.break_ctl =		gs_break_ctl,
};

/*-------------------------------------------------------------------------*/

static struct tty_driver *gs_tty_driver;

static int
gs_port_alloc(unsigned port_num, struct usb_cdc_line_coding *coding)
{
	struct gs_port	*port;
	int		ret = 0;

	mutex_lock(&ports[port_num].lock);
	if (ports[port_num].port) {
		ret = -EBUSY;
		goto out;
	}

	port = kzalloc(sizeof(struct gs_port), GFP_KERNEL);
	if (port == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	tty_port_init(&port->port);
	spin_lock_init(&port->port_lock);
	init_waitqueue_head(&port->drain_wait);

	tasklet_init(&port->push, gs_rx_push, (unsigned long) port);

	INIT_LIST_HEAD(&port->read_pool);
	INIT_LIST_HEAD(&port->read_queue);
	INIT_LIST_HEAD(&port->write_pool);

	port->port_num = port_num;
	port->port_line_coding = *coding;

	ports[port_num].port = port;
out:
	mutex_unlock(&ports[port_num].lock);
	return ret;
}

static int gs_closed(struct gs_port *port)
{
	int cond;

	spin_lock_irq(&port->port_lock);
	cond = (port->port.count == 0) && !port->openclose;
	spin_unlock_irq(&port->port_lock);
	return cond;
}

static void gserial_free_port(struct gs_port *port)
{
	tasklet_kill(&port->push);
	WARN_ON(port->port_usb != NULL);
	tty_port_destroy(&port->port);
	kfree(port);
}

void gserial_free_line(unsigned char port_num)
{
	struct gs_port	*port;

	port = ports[port_num].port;
	/* wait for old opens to finish */
    port->stat_wait_port_close++;    
	wait_event(port->port.close_wait, gs_closed(port));
    port->stat_port_closed++;	

	mutex_lock(&ports[port_num].lock);
	if (WARN_ON(!ports[port_num].port)) {
		mutex_unlock(&ports[port_num].lock);
		return;
	}
	ports[port_num].port = NULL;
	mutex_unlock(&ports[port_num].lock);

    gserial_free_port(port);
	tty_unregister_device(gs_tty_driver, port_num);
}
EXPORT_SYMBOL_GPL(gserial_free_line);

int gserial_alloc_line(unsigned char *line_num)
{
	struct usb_cdc_line_coding	coding;
	struct device			*tty_dev;
	int				ret = 0;
	int				port_num;

	coding.dwDTERate = cpu_to_le32(9600);
	coding.bCharFormat = 8;
	coding.bParityType = USB_CDC_NO_PARITY;
	coding.bDataBits = USB_CDC_1_STOP_BITS;

    /* find first port_num not used */
	for (port_num = 0; port_num < ACM_TTY_COUNT; port_num++) {
		ret = gs_port_alloc(port_num, &coding);
		if (ret == -EBUSY)
			continue;
		if (ret)
			return ret;
		break;
	}
	if (ret)
		return ret;

	/* ... and sysfs class devices, so mdev/udev make /dev/ttyGS* */
	tty_dev = tty_port_register_device(&ports[port_num].port->port,
		gs_tty_driver, port_num, NULL);

	if (IS_ERR(tty_dev)) {
		struct gs_port	*port;
		pr_err("%s: failed to register tty for port %d, err %ld\n",
				__func__, port_num, PTR_ERR(tty_dev));

		ret = PTR_ERR(tty_dev);
		port = ports[port_num].port;
		ports[port_num].port = NULL;
		gserial_free_port(port);
		goto err;
	}

    /*
	 * 1. port_num == 0, reg tty device for console tty
	 * 2. else reg other tty ...
	 */
	if (0 == port_num && gs_acm_is_console_enable()) {
        /* reg from uart shell to usb shell */
        #if 0
        uart_console = bsp_get_uart_console();
        if (uart_console) {
            unregister_console(uart_console);
        }
        #endif
        register_console(&gs_console);
        console_start(&gs_console);
    }

	*line_num = port_num;
err:
	return ret;
}
EXPORT_SYMBOL_GPL(gserial_alloc_line);

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
 * endpoint descriptors, and also have allocate @port_num by calling
 * @gserial_alloc_line().
 *
 * Returns negative errno or zero.
 * On success, ep->driver_data will be overwritten.
 */
int gserial_connect(struct gserial *gser, u8 port_num)
{
	struct gs_port	*port;
	unsigned long	flags;
	int		status;

	if (port_num >= ACM_TTY_COUNT)
		return -ENXIO;

	port = ports[port_num].port;
	if (!port) {
		pr_err("serial line %d not allocated.\n", port_num);
		return -EINVAL;
	}
	if (port->port_usb) {
		pr_err("serial line %d is in use.\n", port_num);
		return -EBUSY;
	}

    /* mask the not ready interrupt for usb netcard class function driver */
    gser->out->enable_xfer_in_progress = 1;

	/* activate the endpoints */
	status = usb_ep_enable(gser->in);
	if (status < 0)
		return status;
	gser->in->driver_data = port;

	status = usb_ep_enable(gser->out);
	if (status < 0)
		goto fail_out;
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
	if (port->port.count) {
		pr_vdebug("gserial_connect: start ttyGS%d\n", port->port_num);
		//gs_start_io(port);
		if (gser->connect)
			gser->connect(gser);
	} else {
		if (gser->disconnect)
			gser->disconnect(gser);
	}
#endif

	spin_unlock_irqrestore(&port->port_lock, flags);
    port->in_name = (char*)gser->in->name;
    port->out_name = (char*)gser->out->name;
    port->stat_port_is_connect = 1;
	return status;

fail_out:
	usb_ep_disable(gser->in);
	gser->in->driver_data = NULL;
	port->stat_port_is_connect = 0;
	return status;
}
EXPORT_SYMBOL_GPL(gserial_connect);
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
void gserial_disconnect(struct gserial *gser)
{
	struct gs_port	*port = gser->ioport;
	unsigned long	flags;

	if (!port)
		return;

	/* tell the TTY glue not to do I/O here any more */
	spin_lock_irqsave(&port->port_lock, flags);

	/* REVISIT as above: how best to track this? */
	port->port_line_coding = gser->port_line_coding;

	port->port_usb = NULL;
	gser->ioport = NULL;
	if (port->port.count > 0 || port->openclose) {
		wake_up_interruptible(&port->drain_wait);
		/*if (port->port.tty)
			tty_hangup(port->port.tty);*/
	}
	spin_unlock_irqrestore(&port->port_lock, flags);

	/* disable endpoints, aborting down any active I/O */
	usb_ep_disable(gser->out);
	gser->out->driver_data = NULL;

	usb_ep_disable(gser->in);
	gser->in->driver_data = NULL;

	/* finally, free any unused/unusable I/O buffers */
	spin_lock_irqsave(&port->port_lock, flags);
	if (port->port.count == 0 && !port->openclose)
		gs_buf_free(&port->port_write_buf);
	gs_free_requests(gser->out, &port->read_pool, NULL);
	gs_free_requests(gser->out, &port->read_queue, NULL);
	gs_free_requests(gser->in, &port->write_pool, NULL);

	port->read_allocated = port->read_started =
		port->write_allocated = port->write_started = 0;

	spin_unlock_irqrestore(&port->port_lock, flags);
	port->stat_port_is_connect = 0;
    port->is_suspend = 0;
}
EXPORT_SYMBOL_GPL(gserial_disconnect);

int gserial_line_state(struct gserial *gser, u32 state)
{
    struct gs_port	*port = gser->ioport;

    if (!port) {
        return -ESHUTDOWN;
    }
	port->line_state_on = (state & U_ACM_CTRL_DTR);
	return 0;
}

int gserial_suspend(struct gserial *gser)
{
    struct gs_port	*port = gser->ioport;

    if (!port) {
        return -ESHUTDOWN;
    }
    port->is_suspend = 1;
    return 0;
}


int gserial_resume(struct gserial *gser)
{
    struct gs_port	*port = gser->ioport;
    unsigned long flags;

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

extern struct console* bsp_get_uart_console(void);

/*
 * convert '\n' -> "\r\n"
 */
static int gs_char_cpy(char* d, char* s, unsigned size,
                       unsigned max, unsigned* dst_size)
{
    int src_pos = 0;
    int dst_pos = 0;

    /* max - 2 : reserve the 2 rooms for last "\r\n" */
    while(src_pos < (int)size && dst_pos < (int)max - 2) {
        if (*(s + src_pos) == '\n') {
            *(d + dst_pos) = '\r';
            *(d + dst_pos + 1) = '\n';
            dst_pos += 2;
            src_pos++;
        }
        else {
            *(d + dst_pos) = *(s + src_pos);
            src_pos++;
            dst_pos++;
        }
    }

    *dst_size = dst_pos;
    return src_pos;
}

static int gs_usb_tty_write(struct gs_port *port,
                        const char *buf, unsigned count)
{
    struct list_head	*pool = &port->write_pool;
	struct usb_ep		*in;
	struct usb_request	*req;
	unsigned long flags;
	unsigned trans_size;
	int status;

	spin_lock_irqsave(&port->port_lock, flags);
	if (unlikely(!port->port.count)) {
        status = -EINVAL;
        goto console_write_exit;
    }
	/* for lpm remote wakeup support */
	if (unlikely(!port->port_usb /*|| port->is_suspend*/)) {
        status = -ESHUTDOWN;
        port->stat_con_write_not_conn++;
        goto console_write_exit;
    }
    if (unlikely(list_empty(pool))) {
        status = -ENOMEM;
        port->stat_con_write_no_mem++;
        goto console_write_exit;
    }
    in = port->port_usb->in;
    req = list_entry(pool->next, struct usb_request, list);
    list_del(&req->list);
    spin_unlock_irqrestore(&port->port_lock, flags);

    count = gs_char_cpy(req->buf, (char*)buf, count, in->maxpacket, &trans_size);
    req->length = trans_size;
    req->zero = (trans_size % in->maxpacket) ? (0) : (1);
    status = usb_ep_queue(in, req, GFP_ATOMIC);
    spin_lock_irqsave(&port->port_lock, flags);
    if (status) {
		list_add(&req->list, pool);
		goto console_write_exit;
	}
	port->write_started++;
	status = count;

console_write_exit:
	spin_unlock_irqrestore(&port->port_lock, flags);
	return status;
}

static int gs_console_write_to_usb(const char *buf, unsigned count)
{
    struct gs_port *port = ports[ACM_CONSOLE_IDX].port;

    if (count == 0 || NULL == port)
		return 0;

    while (count) {
		int ret;
        ret = gs_usb_tty_write(port, buf, count);
        if (ret < 0) {
            return ret;
        }
		buf += ret;
		count -= ret;
	}
	return 0;
}

static void gs_console_write(struct console *co,
					const char *buf, unsigned count)
{
    struct gs_port *port = ports[ACM_CONSOLE_IDX].port;
    unsigned long flags;
    int no_data = 0;

    if (count == 0 || NULL == port)
		return;

    /* write the data to usb console buffer */
    spin_lock_irqsave(&gs_console_ctx.lock, flags);
    if (gs_buf_space_avail(&gs_console_ctx.buf_info,
        port->line_state_on) < count) {
        port->stat_con_write_no_room++;
        no_data = 1;
    }
    (void)gs_buf_put(&gs_console_ctx.buf_info, buf, count, port->line_state_on);
    spin_unlock_irqrestore(&gs_console_ctx.lock, flags);

    /* we can delay a little time to get the more msg */
    if (!no_data)
        schedule_delayed_work(&gs_console_ctx.write_work, 1);
    return;
}

static void gs_console_write_work(struct work_struct *work)
{
    unsigned long flags;
    unsigned count;
    unsigned len;
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

static int gs_console_setup(struct console *co, char *options)
{
    if (ports[ACM_CONSOLE_IDX].port) {
        if (gs_buf_alloc(&gs_console_ctx.buf_info, CONSOLE_BUFFER_SIZE)) {
            return -ENOMEM;
        }
        spin_lock_init(&gs_console_ctx.lock);
        INIT_DELAYED_WORK(&gs_console_ctx.write_work, gs_console_write_work);
        return 0;
    }

    return -ENODEV;
}

static struct tty_driver *gs_console_device(struct console *co, int *index)
{
	struct tty_driver **p = (struct tty_driver **)co->data;

	if (!*p)
		return NULL;

	*index = co->index;
	return *p;
}

struct console gs_console = {
	.name =		ACM_CONSOLE_NAME,
	.write =	gs_console_write,
	.device =	gs_console_device,
	.setup =	gs_console_setup,
	.flags =	CON_PRINTBUFFER,
	.index =	-1,
	.data = 	&gs_tty_driver,
};

static inline int gs_acm_is_console_enable(void)
{
    return bsp_usb_is_support_shell();
}

static void acm_serial_dump_ep_info(struct gs_port *port)
{
    char* find;
    unsigned ep_num;

    if (port->stat_port_is_connect) {
        pr_emerg("in ep name:\t\t\t <%s>\n", port->in_name);
        find = strstr(port->in_name, "ep");
        if (find) {
            /* skip "ep" */
            find += 2;
            ep_num = simple_strtoul(find, NULL, 0);
            pr_emerg("in ep num:\t\t\t <%d>\n", ep_num * 2 + 1);
        }
        pr_emerg("out ep name:\t\t\t <%s>\n", port->out_name);
        find = strstr(port->out_name, "ep");
        if (find) {
            /* skip "ep" */
            find += 2;
            ep_num = simple_strtoul(find, NULL, 0);
            pr_emerg("out ep num:\t\t\t <%d>\n", ep_num * 2);
        }
    }
    else {
        pr_emerg("the acm dev is not connect\n");
    }
}


int acm_serial_dump(int idx)
{
    struct gs_port *port;

    if (idx >= ACM_TTY_USED_COUNT) {
        pr_emerg("serial port num:%d is not valid\n", idx);
        return -1;
    }
    port = ports[ACM_CONSOLE_IDX].port;
    if (NULL == port) {
        pr_emerg("serial port not alloc\n");
        return -1;
    }

    pr_emerg("serial port is connect:       %s\n", (port->port_usb) ? ("connect") : ("disconnect"));
    pr_emerg("console tty name:             %s\n", ACM_CONSOLE_NAME);
    acm_serial_dump_ep_info(port);
    pr_emerg("port.count:                   %d\n", port->port.count);
    pr_emerg("line_state_on:                %d\n", port->line_state_on);
    pr_emerg("is_suspend:                   %d\n", port->is_suspend);
    pr_emerg("port_write_buf buf_buf:       0x%x\n", (unsigned)port->port_write_buf.buf_buf);
    pr_emerg("port_write_buf buf_get:       0x%x\n", (unsigned)port->port_write_buf.buf_get);
    pr_emerg("port_write_buf buf_put:       0x%x\n", (unsigned)port->port_write_buf.buf_put);
    pr_emerg("port_write_buf buf_size:      %d\n", port->port_write_buf.buf_size);
    pr_emerg("read_started:                 %d\n", port->read_started);
    pr_emerg("n_read:                       %d\n", port->n_read);
    pr_emerg("read_allocated:               %d\n", port->read_allocated);
    pr_emerg("write_started:                %d\n", port->write_started);
    pr_emerg("write_allocated:              %d\n", port->write_allocated);
    pr_emerg("stat_con_write_not_conn:      %d\n", port->stat_con_write_not_conn);
    pr_emerg("stat_con_write_no_room:       %d\n", port->stat_con_write_no_room);
    pr_emerg("stat_con_write_no_mem:        %d\n", port->stat_con_write_no_mem);
    pr_emerg("stat_con_data_max:            %d\n", port->stat_con_data_max);
    pr_emerg("stat_write_not_conn:          %d\n", port->stat_write_not_conn);
    pr_emerg("stat_write_no_mem:            %d\n", port->stat_write_no_mem);
    pr_emerg("stat_cannt_push:              %d\n", port->stat_cannt_push);
    pr_emerg("stat_wait_port_close          %d\n", port->stat_wait_port_close);
    pr_emerg("stat_port_closed              %d\n", port->stat_port_closed);

    return 0;
}

int acm_serial_mem_dump(char* buffer, unsigned int buf_size)
{
    unsigned int need_size;
    struct gs_port	*port;
    int i;
	int num = 0;
    char* cur = buffer;

    need_size = ACM_TTY_COUNT * sizeof(struct gs_port);
    /* no room left */
    if (need_size + 8> buf_size) {
        return -1;
    }
	
	cur += 2 * sizeof(int);
    for (i = 0; i < (int)ACM_TTY_COUNT; i++) {
        port = ports[i].port;
        if (port) {
			port->offset = DUMP_OFFSET_OF(struct gs_port, openclose);
            memcpy(cur, port, sizeof(struct gs_port));
            cur += sizeof(struct gs_port);
			num++;
        }
    }
	cur = buffer;
	*((int*)cur) = num;
	cur += sizeof(int);
	*((int*)cur) = sizeof(struct gs_port);
	cur += sizeof(int);
	need_size = num * sizeof(struct gs_port) + 8;
	return (int)need_size;
}

static int userial_init(void)
{
	unsigned			i;
	int				status;

	gs_tty_driver = alloc_tty_driver(MAX_U_SERIAL_PORTS);
	if (!gs_tty_driver)
		return -ENOMEM;

	gs_tty_driver->driver_name = "g_serial";
	gs_tty_driver->name = g_acm_tty_type_table[ACM_CONSOLE_IDX].name;
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

	tty_set_operations(gs_tty_driver, &gs_tty_ops);
	for (i = 0; i < MAX_U_SERIAL_PORTS; i++)
		mutex_init(&ports[i].lock);

	/* export the driver ... */
	status = tty_register_driver(gs_tty_driver);
	if (status) {
		pr_err("%s: cannot register, err %d\n",
				__func__, status);
		goto fail;
	}

	pr_debug("%s: registered %d ttyGS* device%s\n", __func__,
			MAX_U_SERIAL_PORTS,
			(MAX_U_SERIAL_PORTS == 1) ? "" : "s");

	return status;/* [false alarm]:fortify disable */

fail:
	put_tty_driver(gs_tty_driver);
	gs_tty_driver = NULL;
	return status;
}
module_init(userial_init);

static void userial_cleanup(void)
{
	tty_unregister_driver(gs_tty_driver);
	put_tty_driver(gs_tty_driver);
	gs_tty_driver = NULL;
}
module_exit(userial_cleanup);

MODULE_LICENSE("GPL");
