/*

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.


   Copyright (C) 2006-2007 - Motorola
   Copyright (c) 2008-2010, The Linux Foundation. All rights reserved.

   Date         Author           Comment
   -----------  --------------   --------------------------------
   2006-Apr-28    Motorola     The kernel module for running the Bluetooth(R)
                 Sleep-Mode Protocol from the Host side
   2006-Sep-08  Motorola         Added workqueue for handling sleep work.
   2007-Jan-24  Motorola         Added mbm_handle_ioi() call to ISR.
   2015-July-21 Huawei          modify the driver and added netlink_report_systerm call to ISR

*/
#include <linux/module.h>    /* kernel module definitions */
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/param.h>
#include <linux/bitops.h>
#include <linux/termios.h>
#include <mach/gpio.h>
#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h> /* event notifications */

#include <asm/gpio.h>
#include <linux/of_gpio.h>
#include <linux/device.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/of_platform.h>
#include <linux/spinlock.h>
#include <linux/netlink.h>
#include "linux/wakelock.h"



#define BT_SLEEP_DBG
#ifndef BT_SLEEP_DBG
#define BT_DBG(fmt, arg...)
#endif
/*
 * Defines
 */
#define VERSION        "1.1"
#define PROC_DIR    "bluetooth/sleep"

#define BT_LPM_SUPPORT_BTLD 1
#define BT_ERR_RET  (-1)
#define BLUE_WAKE_LOCK_TIME 2000
#define BLUE_TIME_LOCK_TIME 300
#define DEVICE_ID_GPIO_INTER 18
#define GPIO_INTER_BT_WAKE 1
#define DWH_BT_DEBUG 1
#ifdef DWH_BT_DEBUG
#define bt_printk(fmt, arg...)  printk("%s: " fmt "\n" , __func__ , ## arg)
#define btdb_printk(fmt, arg...)  printk( fmt, ## arg)
#else
#define bt_printk(fmt, arg...)  
#define btdb_printk(fmt, arg...)  
#endif

/* 1 second timeout */
#define TX_TIMER_INTERVAL    1

/* 100ms timeout */
#define BLUE_WAKE_TIMER_INTERVAL  100

/* state variable names and bit positions */
#define BT_PROTO    0x01
#define BT_TXDATA    0x02
#define BT_ASLEEP    0x04

struct bluesleep_info {
    unsigned host_wake;
    unsigned ext_wake;
    unsigned host_wake_irq;
    struct uart_port *uport;
};

unsigned g_bt_wakeup_bb = 0;  /*设置为输入脚也设置为中断脚，等待BT芯片的唤醒*/
unsigned g_bb_wakeup_bt = 0;

/* work function */
static void bluesleep_sleep_work(struct work_struct *work);

/* work queue */
DECLARE_DELAYED_WORK(sleep_workqueue, bluesleep_sleep_work);/*lint !e133 !e43 !e64 !e65 */

/* Macros for handling sleep work */
#define bluesleep_rx_busy()     schedule_delayed_work(&sleep_workqueue, 0)
#define bluesleep_tx_busy()     schedule_delayed_work(&sleep_workqueue, 0)
#define bluesleep_rx_idle()     schedule_delayed_work(&sleep_workqueue, 0)
#define bluesleep_tx_idle()     schedule_delayed_work(&sleep_workqueue, 0)

/* global pointer to a single hci device. */
//static struct hci_dev *bluesleep_hdev;

static struct bluesleep_info *bsi;

/* module usage */
static atomic_t open_count = ATOMIC_INIT(1);


/*
 * Global variables
 */

/** Global state flags */
static unsigned long flags;



/** Tasklet to respond to change in hostwake line */
static struct tasklet_struct hostwake_task;

/** Transmission timer */
static struct timer_list tx_timer;

/** Lock for state transitions */
static spinlock_t rw_lock; /*lint !e86 */


static int ext_wake_state;   //表示ext_wake的状态
static int wakeup_state;

static struct wake_lock bluetooth_wake_lock;/*lint !e86 */
struct proc_dir_entry *bluetooth_dir, *sleep_dir;

/*
 * Local functions
 */

#ifndef BT_LPM_SUPPORT_BTLD
static void hsuart_power(int on)
{
    btdb_printk(" %s enter on=%d.\n", __FUNCTION__, on);
    if (on) 
    {
       msm_hs_request_clock_on(bsi->uport);
       msm_hs_set_mctrl(bsi->uport, TIOCM_RTS);
    } 
    else 
    {
       msm_hs_set_mctrl(bsi->uport, 0);
       msm_hs_request_clock_off(bsi->uport);
    }
}
#else
static void    hsuart_power(int on) {}
#endif

/**
 * @return 1 if the Host can go to sleep, 0 otherwise.
 */
static inline int bluesleep_can_sleep(void)
{
    btdb_printk(" %s enter.\n", __FUNCTION__);
#if BT_LPM_SUPPORT_BTLD
    if(0 == wakeup_state)
        {
        wakeup_state = 1;
        return 0;
        }
    bt_printk(" ext_wake =%d, host_wake= %d.\n", 
                ext_wake_state, gpio_get_value(bsi->host_wake));

    return ext_wake_state &&
        gpio_get_value(bsi->host_wake);

#else
    
    /* check if MSM_WAKE_BT_GPIO and BT_WAKE_MSM_GPIO are both deasserted */
    
    bt_printk(" ext_wake =%d, host_wake= %d, uport is %s.\n", 
                ext_wake_state , gpio_get_value(bsi->host_wake), 
                (bsi->uport != NULL)?"not NULL":"NULL");

    return ext_wake_state &&
        gpio_get_value(bsi->host_wake) &&
        (bsi->uport != NULL);
#endif
}
/******************************************************************************
  函数名称  : netlink_report_systerm
  功能描述  : 上报唤醒中断事件给应用
  输入参数  : void
  调用函数  : 
  被调函数  : 
  输出参数  : NA
  返 回 值  :void
  其它说明  ：
******************************************************************************/
void netlink_report_systerm(void)
{
    int ret = -1;
    DEVICE_EVENT event;/*lint !e10 !e522 */
    btdb_printk("netlink_report_systerm enter...\n");
    /*lint -e10 -e63*/
    event.device_id = DEVICE_ID_GPIO_INTER;
    event.event_code = GPIO_INTER_BT_WAKE;
    event.len = 0;
    /*lint +e10 +e63*/
    
    ret = device_event_report(&event, sizeof(event));
    if (-1 == ret) 
        {
        printk("bluetooth event report fail\n");
        }
}

void bluesleep_sleep_wakeup(void)
{
    btdb_printk(" %s enter.\n", __FUNCTION__);
    if (test_bit(BT_ASLEEP, &flags)) {
        bt_printk("waking up...");
        wake_lock_timeout(&bluetooth_wake_lock, 
                  (long)msecs_to_jiffies(BLUE_WAKE_LOCK_TIME));
        netlink_report_systerm();
        /* Start the timer */
        mod_timer(&tx_timer, jiffies + (long)msecs_to_jiffies(BLUE_WAKE_TIMER_INTERVAL));
        gpio_set_value(bsi->ext_wake, 0);
        ext_wake_state = 0;
        clear_bit(BT_ASLEEP, &flags);
        /*Activating UART */
        hsuart_power(1);
    }

    else {
    /*Tx idle, Rx    busy, we must also make    host_wake asserted,    that is    low
    * 1 means bt chip can sleep, in bluesleep.c
    */
    /* Here we depend on the status of gpio ext_wake, for stability    */
        if (1 == ext_wake_state) {
            bt_printk("bluesleep_sleep_wakeup wakeup bt chip\n");
            gpio_set_value(bsi->ext_wake, 0);
            ext_wake_state = 0;
            mod_timer(&tx_timer, jiffies + (long)msecs_to_jiffies(BLUE_WAKE_TIMER_INTERVAL));
        }
        else{
            bt_printk("bluesleep_sleep_wakeup is not real wakeup \n");
            }
    }

}

/**
 * @brief@  main sleep work handling function which update the flags
 * and activate and deactivate UART ,check FIFO.
 */
static void bluesleep_sleep_work(struct work_struct *work)
{
    btdb_printk(" %s enter.\n", __FUNCTION__);
    if (bluesleep_can_sleep()) {
        /* already asleep, this is an error case */
        if (test_bit(BT_ASLEEP, &flags)) {
            btdb_printk("already asleep");
            return;
        }
#ifndef BT_LPM_SUPPORT_BTLD
        if (msm_hs_tx_empty(bsi->uport)) {
            btdb_printk("going to sleep...");
            set_bit(BT_ASLEEP, &flags);
            /*Deactivating UART */
            hsuart_power(0);
        }
        else {
          mod_timer(&tx_timer, jiffies + (long)msecs_to_jiffies(BLUE_WAKE_TIMER_INTERVAL));
            return;
        }
#else
        btdb_printk("bt going to sleep...\n");
        set_bit(BT_ASLEEP, &flags);
#endif
    } else {
        bluesleep_sleep_wakeup();
    }
}

/**
 * A tasklet function that runs in tasklet context and reads the value
 * of the HOST_WAKE GPIO pin and further defer the work.
 * @param data Not used.
 */
static void bluesleep_hostwake_task(unsigned long data)
{

    btdb_printk(" %s enter.\n", __FUNCTION__);
    spin_lock(&rw_lock);
    wakeup_state = 0;
    if (gpio_get_value(bsi->host_wake))
    {
        bluesleep_rx_busy();
    }
    else
    {
        bluesleep_rx_idle();
    }
    spin_unlock(&rw_lock);
}

/**
 * Handles proper timer action when outgoing data is delivered to the
 * HCI line discipline. Sets BT_TXDATA.
 */
static void bluesleep_outgoing_data(void)
{
    unsigned long irq_flags = 0;
    btdb_printk(" %s enter.\n", __FUNCTION__);
    spin_lock_irqsave(&rw_lock, irq_flags);/*lint !e628 !e160 !e666 !e534 !e530 !e830 !e10 !e550 !e522 */

    /* log data passing by */
    set_bit(BT_TXDATA, &flags);

    /* if the tx side is sleeping... */
    if (ext_wake_state) {
        btdb_printk("tx was sleeping");
        bluesleep_sleep_wakeup();
    }

    spin_unlock_irqrestore(&rw_lock, irq_flags);/*lint !e530 */
}


/**
 * Handles transmission timer expiration.
 * @param data Not used.
 */
static void bluesleep_tx_timer_expire(unsigned long data)
{
    unsigned long irq_flags = 0;

    spin_lock_irqsave(&rw_lock, irq_flags);/*lint !e628 !e160 !e666 !e534 !e530 !e830 !e10 !e550 !e522 */
    btdb_printk(" %s enter.\n", __FUNCTION__);
    BT_DBG("Tx timer expired");

    /* were we silent during the last timeout? */
    if (!test_bit(BT_TXDATA, &flags)) {
        BT_DBG("Tx has been idle");
        gpio_set_value(bsi->ext_wake, 1);
        ext_wake_state = 1;
        bluesleep_tx_idle();
    } else {
        BT_DBG("Tx data during last period");
        mod_timer(&tx_timer, jiffies + (long)msecs_to_jiffies(BLUE_WAKE_TIMER_INTERVAL));
    }

    /* clear the incoming data flag */
    clear_bit(BT_TXDATA, &flags);

    spin_unlock_irqrestore(&rw_lock, irq_flags);/*lint !e530 */
}


/**
 * Schedules a tasklet to run when receiving an interrupt on the
 * <code>HOST_WAKE</code> GPIO pin.
 * @param irq Not used.
 * @param dev_id Not used.
 */
static irqreturn_t bluesleep_hostwake_isr(int irq, void *dev_id)
{

    /* schedule a tasklet to handle the change in the host wake line */
    tasklet_schedule(&hostwake_task);
    return IRQ_HANDLED;
}

/**
 * Starts the Sleep-Mode Protocol on the Host.
 * @return On success, 0. On error, -1, and <code>errno</code> is set
 * appropriately.
 */
static int bluesleep_start(void)
{
    int retval;
    unsigned long irq_flags = 0;
    btdb_printk(" %s enter.\n", __FUNCTION__);
    spin_lock_irqsave(&rw_lock, irq_flags);/*lint !e628 !e160 !e666 !e534 !e530 !e830 !e10 !e550 !e522 */

    if (test_bit(BT_PROTO, &flags)) {
        spin_unlock_irqrestore(&rw_lock, irq_flags);/*lint !e530 */
        return 0;
    }

    spin_unlock_irqrestore(&rw_lock, irq_flags);/*lint !e530 */

    if (!atomic_dec_and_test(&open_count)) {
        atomic_inc(&open_count);
        return -EBUSY;
    }
    wake_lock_timeout(&bluetooth_wake_lock, 
                  (long)msecs_to_jiffies(BLUE_TIME_LOCK_TIME));
    /* start the timer */
    mod_timer(&tx_timer, jiffies + (long)msecs_to_jiffies(BLUE_WAKE_TIMER_INTERVAL));

    /* assert BT_WAKE */
    gpio_set_value(bsi->ext_wake, 0);
    ext_wake_state = 0;

    retval = request_irq((unsigned int)gpio_to_irq(g_bt_wakeup_bb), bluesleep_hostwake_isr, 
                    IRQF_NO_SUSPEND | IRQF_SHARED | IRQ_TYPE_EDGE_FALLING, "bluetooth hostwake", bsi);
    if (retval  < 0) {
        BT_ERR("Couldn't acquire BT_HOST_WAKE IRQ");
        goto fail;
    }

    set_bit(BT_PROTO, &flags);
    return 0;
fail:
    del_timer(&tx_timer);
    atomic_inc(&open_count);
    return retval;
}

/**
 * Stops the Sleep-Mode Protocol on the Host.
 */
static void bluesleep_stop(void)
{
    unsigned long irq_flags = 0;
    btdb_printk(" %s enter.\n", __FUNCTION__);
    spin_lock_irqsave(&rw_lock, irq_flags);/*lint !e628 !e160 !e666 !e534 !e530 !e830 !e10 !e550 !e522 */

    if (!test_bit(BT_PROTO, &flags)) {
        spin_unlock_irqrestore(&rw_lock, irq_flags);/*lint !e530 */
        return;
    }

    /* assert BT_WAKE */
    gpio_set_value(bsi->ext_wake, 0);
    ext_wake_state = 0;
    del_timer(&tx_timer);
    clear_bit(BT_PROTO, &flags);

    if (test_bit(BT_ASLEEP, &flags)) {
        clear_bit(BT_ASLEEP, &flags);
        hsuart_power(1);
    }

    atomic_inc(&open_count);

    spin_unlock_irqrestore(&rw_lock, irq_flags);/*lint !e530 */
    free_irq(bsi->host_wake_irq, bsi);

}
/**
 * Read the <code>BT_WAKE</code> GPIO pin value via the proc interface.
 * When this function returns, <code>BT_WAKE</code> will contain a 1 if the
 * pin is high, 0 otherwise.
 * @param file not used.
 * @param buf for writing data.
 * @param count the number of bytes to be written.
 * @param ppos offset of the file.
 * @return  On success, the number of bytes written.
 */

static ssize_t bluepower_read_proc_btwake(struct file *file, char __user *buf,size_t count, loff_t *ppos)
{
    unsigned int btwake;
    int len = 0;
    
    btdb_printk(" %s enter.\n", __FUNCTION__);
    
    btwake = ext_wake_state;/*获取btwake的值*/

    btdb_printk(" the btwake value is %d .\n", btwake);
    len = sizeof(btwake);   
    if(*ppos > len )
    {
        return  0;
    }
    if(count > len - *ppos)
    {
        count = len - *ppos;
    }

    if(copy_to_user(buf, &btwake, count))
    {
         btdb_printk(" %s fail.\n", __FUNCTION__);
    }
    
    *ppos += count;
    
    return count;
}

/* add for bluedroid begin ,add btwrite proc entry*/
static int bluesleep_ap_wakeup_bt(struct file *file, const    char *buffer,
                    unsigned long count, void *data)
{
    char buf;
    unsigned long irq_flags = 0;
    if (count < 1 )
    {    
        return -EINVAL;
    }

    if (copy_from_user(&buf, buffer, sizeof(buf)))
    {
        return -EFAULT;
    }

    btdb_printk(" %s enter buf=%x.\n", __FUNCTION__, buf);
    
    if (buf == '0') {
        spin_lock_irqsave(&rw_lock, irq_flags);/*lint !e628 !e160 !e666 !e534 !e530 !e830 !e10 !e550 !e522 */
        clear_bit(BT_TXDATA, &flags);
        spin_unlock_irqrestore(&rw_lock, irq_flags);/*lint !e530 */
    } else
            {
            bluesleep_outgoing_data();
            }

    return count;
}

/*add for bluedroid end,add btwrite proc entry*/
/**
 * Write the <code>BT_WAKE</code> GPIO pin value via the proc interface.
 * @param file Not used.
 * @param buffer The buffer to read from.
 * @param count The number of bytes to be written.
 * @param data Not used.
 * @return On success, the number of bytes written. On error, -1, and
 * <code>errno</code> is set appropriately.
 */
static int bluepower_write_proc_btwake(struct file *file, const char *buffer,
                    unsigned long count, void *data)
{
    char *buf;

    if (count < 1)
    {
        return -EINVAL;
    }
    btdb_printk(" %s enter.\n", __FUNCTION__);
    buf = kmalloc(count, GFP_KERNEL);
    if (!buf)
    {
        return -ENOMEM;
    }

    if (copy_from_user(buf, buffer, count)) {
        kfree(buf);
        return -EFAULT;
    }

    if (buf[0] == '0') {
        gpio_set_value(bsi->ext_wake, 0);
        ext_wake_state = 0;
    } else if (buf[0] == '1') {
        gpio_set_value(bsi->ext_wake, 1);
        ext_wake_state = 1;
    } else {
        kfree(buf);
        return -EINVAL;
    }

    kfree(buf);
    return count;
}

/**
 * Read the <code>BT_HOST_WAKE</code> GPIO pin value via the proc interface.
 * When this function returns, <code>BT_HOST_WAKE</code> will contain a 1 if the
 * pin is high, 0 otherwise.
 * @param file not used.
 * @param buf for writing data.
 * @param count the number of bytes to be written.
 * @param ppos offset of the file.
 * @return  On success, the number of bytes written.
 */

static ssize_t bluepower_read_proc_hostwake(struct file *file, char __user *buf,size_t count, loff_t *ppos)
{
    unsigned int hostwake;
    int len = 0;

    btdb_printk(" %s enter.\n", __FUNCTION__);

    hostwake = gpio_get_value(bsi->host_wake);/*获取hostwake的值*/

    btdb_printk(" the hostwake value is %d .\n", hostwake);
    len = sizeof(hostwake);   
    if(*ppos > len )
    {
        return  0;
    }
    if(count > len - *ppos)
    {
        count = len - *ppos;
    }

    if(copy_to_user(buf, &hostwake, count))
    {
         btdb_printk(" %s fail.\n", __FUNCTION__);
    }

    *ppos += count;

    return count;
}


/**
 * Read low-power status of the Host via the proc interface.
 * When this function returns, <code>asleep</code> will contain a 1 if the
 * pin is high, 0 otherwise.
 * @param file not used.
 * @param buf for writing data.
 * @param count the number of bytes to be written.
 * @param ppos offset of the file.
 * @return  On success, the number of bytes written.
 */

static ssize_t bluesleep_read_proc_asleep(struct file *file, char __user *buf,size_t count, loff_t *ppos)
{
    unsigned int asleep;
    int len = 0;

    btdb_printk(" %s enter.\n", __FUNCTION__);

    asleep = test_bit(BT_ASLEEP, &flags) ? 1 : 0;/*获取asleep的值*/

    btdb_printk(" the asleep value is %d .\n", asleep);
    len = sizeof(asleep);   
    if(*ppos > len )
    {
        return  0;
    }
    if(count > len - *ppos)
    {
        count = len - *ppos;
    }

    if(copy_to_user(buf, &asleep, count))
    {
         btdb_printk(" %s fail.\n", __FUNCTION__);
    }

    *ppos += count;

    return count;
}

/**
 * Read the low-power protocol being used by the Host via the proc interface.
 * When this function returns, <code>proto</code> will contain a 1  if the Host
 * is using the Sleep Mode Protocol, 0 otherwise.
 * @param file not used.
 * @param buf for writing data.
 * @param count the number of bytes to be written.
 * @param ppos offset of the file.
 * @return  On success, the number of bytes written.
 */

static ssize_t bluesleep_read_proc_proto(struct file *file, char __user *buf,size_t count, loff_t *ppos)
{
    unsigned int proto;
    int len = 0;

    btdb_printk(" %s enter.\n", __FUNCTION__);

    proto = test_bit(BT_PROTO, &flags) ? 1 : 0;/*获取proto的值*/

    btdb_printk(" the proto value is %d .\n", proto);
    len = sizeof(proto);   
    if(*ppos > len )
    {
        return  0;
    }
    if(count > len - *ppos)
    {
        count = len - *ppos;
    }

    if(copy_to_user(buf, &proto, count))
    {
         btdb_printk(" %s fail.\n", __FUNCTION__);
    }

    *ppos += count;

    return count;
}
/**
 * Modify the low-power protocol used by the Host via the proc interface.
 * @param file Not used.
 * @param buffer The buffer to read from.
 * @param count The number of bytes to be written.
 * @param data Not used.
 * @return On success, the number of bytes written. On error, -1, and
 * <code>errno</code> is set appropriately.
 */
static int bluesleep_write_proc_proto(struct file *file, const char *buffer,
                    unsigned long count, void *data)
{
    char proto;
    btdb_printk(" %s enter.\n", __FUNCTION__);

    if (count < 1)
    {
        return -EINVAL;
    }

    if (copy_from_user(&proto, buffer, 1))
    {
        return -EFAULT;
    }

    if (proto == '0')
    {
        bluesleep_stop();
    }
    else
    {
        bluesleep_start();
    }

    /* claim that we wrote everything */
    return count;
}


/******************************************************************************
  函数名称  : bluesleep_gpio_get
  功能描述  : 获取设备树中的gpio管脚            
  输入参数  : 
  调用函数  : 
  被调函数  : 
  输出参数  : NA
  返 回 值  :    NA             
  其它说明  ：
******************************************************************************/

static int bluesleep_gpio_get(struct platform_device *pdev)
{
    int ret = 0;
    struct device_node *np = pdev->dev.of_node;
    btdb_printk(" %s enter.\n", __FUNCTION__);

    ret = of_property_read_u32_index(np, "bt_wakeup_bb_gpio", 0, &g_bt_wakeup_bb);
    if(0 != ret)
    {
        BT_ERR("find bluetooth bt_wakeup_bb_gpio fail\n");
        return BT_ERR_RET;

    }
    ret = of_property_read_u32_index(np, "bb_wakeup_bt_gpio", 0, &g_bb_wakeup_bt);
    if(0 != ret)
    {
        BT_ERR("find bluetooth bt_wakeup_bb_gpio fail\n");
        return BT_ERR_RET;
    }
   
    return 0;
}

static const struct file_operations btwake_operations = {
    .read = bluepower_read_proc_btwake,
    .write = bluepower_write_proc_btwake,
};

static const struct file_operations proto_operations = {
    .read = bluesleep_read_proc_proto,
    .write = bluesleep_write_proc_proto,
};

static const struct file_operations hostwake_operations = {
    .read = bluepower_read_proc_hostwake,
};
static const struct file_operations asleep_operations = {
    .read = bluesleep_read_proc_asleep,
};

 /******************************************************************************
  函数名称  : bluesleep_init
  功能描述  : Initializes the module            
  输入参数  : 
  调用函数  : 
  被调函数  : 
  输出参数  : NA
  返 回 值  :    return On success, 0. On error, -1, and <code>errno</code> is set appropriately.        
  其它说明  ：
******************************************************************************/
static int bluesleep_init(void)
{
    int retval = 0;
    struct proc_dir_entry *ent = NULL;
    btdb_printk(" %s enter.\n", __FUNCTION__);

    bluetooth_dir = proc_mkdir("bluetooth", NULL);
    if (bluetooth_dir == NULL) {
        BT_ERR("Unable to create /proc/bluetooth directory");
        return -ENOMEM;
    }

    sleep_dir = proc_mkdir("sleep", bluetooth_dir);
    if (sleep_dir == NULL) {
        BT_ERR("Unable to create /proc/%s directory", PROC_DIR);
        return -ENOMEM;
    }

    /* Creating read/write "btwake" entry */   
    ent = proc_create("btwake", 0777, sleep_dir, &btwake_operations);
    if (ent == NULL) {
        BT_ERR("Unable to create /proc/%s/btwake entry", PROC_DIR);
        retval = -ENOMEM;
        goto fail;
    }  

    /* read only proc entries */
    ent = proc_create("hostwake", 0, sleep_dir, &hostwake_operations);
    if(ent == NULL)
        {
        BT_ERR("Unable to create /proc/%s/hostwake entry", PROC_DIR);
        retval = -ENOMEM;
        goto fail;
    }

    /* read/write proc entries */  
    ent = proc_create("proto", 0777, sleep_dir, &proto_operations);
    if (ent == NULL) {
        BT_ERR("Unable to create /proc/%s/proto entry", PROC_DIR);
        retval = -ENOMEM;
        goto fail;
    }

    /* read only proc entries */
    ent = proc_create("asleep", 0, sleep_dir, &asleep_operations);
    if(ent == NULL)
        {
        BT_ERR("Unable to create /proc/%s/asleep entry", PROC_DIR);
        retval = -ENOMEM;
        goto fail;
    }

    flags = 0; /* clear all status bits */
    wakeup_state = 1;
    /* Initialize spinlock. */
    spin_lock_init(&rw_lock);/*lint !e133 !e43*/

    /* Initialize timer */
    init_timer(&tx_timer);
    tx_timer.function = bluesleep_tx_timer_expire;
    tx_timer.data = 0;

    /* initialize host wake tasklet */
    tasklet_init(&hostwake_task, bluesleep_hostwake_task, 0);

    /*系统保持唤醒初始化*/
    wake_lock_init(&bluetooth_wake_lock, WAKE_LOCK_SUSPEND, "bluetooth wake_lock");   
    
    return 0;

fail:
    remove_proc_entry("btwrite",sleep_dir);
    remove_proc_entry("asleep", sleep_dir);
    remove_proc_entry("proto", sleep_dir);
    remove_proc_entry("hostwake", sleep_dir);
    remove_proc_entry("btwake", sleep_dir);
    remove_proc_entry("sleep", bluetooth_dir);
    remove_proc_entry("bluetooth", 0);
    return retval;
}

/******************************************************************************
  函数名称  : bluesleep_probe
  功能描述  : 初始化hostwake 和ext_wake 管脚            
  输入参数  : 
  调用函数  : 
  被调函数  : 
  输出参数  : NA
  返 回 值  :    NA             
  其它说明  ：
******************************************************************************/

static int __init bluesleep_probe(struct platform_device *pdev)
{
    int ret = 0;
    btdb_printk(" %s enter.\n", __FUNCTION__);
    /*创建设备节点*/
    ret = bluesleep_init();
    if(ret)
    {
        BT_ERR("bluesleep_init failed.\n");
        return BT_ERR_RET;
    }

    /*获取设备树中的gpio配置*/
    ret = bluesleep_gpio_get(pdev);
    if(ret)
    {
        BT_ERR("couldn't decode bluetooth device tree\n");
        return BT_ERR_RET;
    }

    bsi = kzalloc(sizeof(struct bluesleep_info), GFP_KERNEL);
    if (!bsi){
        BT_ERR("alloc memary for bluesleep_info failed!\n");
        return -ENOMEM;
    }
    bsi->host_wake = g_bt_wakeup_bb;
    bsi->ext_wake = g_bb_wakeup_bt;

    ret = gpio_request(bsi->host_wake, "bt-host_wake-gpio");
    if (ret)
    {
        BT_ERR("bluesleep_probe fail to request bt_host_wake gpio!!!!\n");
        ret = BT_ERR_RET;
        goto free_bsi;
    }
    
    ret = gpio_direction_input(bsi -> host_wake);
    if (ret){
        BT_ERR("bluesleep_probe bt_host_wake gpio set input failed.\n");
        ret = BT_ERR_RET;
        goto free_bt_host_wake;
    }

    bsi->host_wake_irq = gpio_to_irq(bsi -> host_wake);
    btdb_printk("init_bcm4356_host_wake_gpio  bluetooth host wake irq===%d\n",bsi->host_wake_irq);


    ret = gpio_request(bsi -> ext_wake, "bt-ext_wake-gpio");
    if (ret)
    {
        BT_ERR("bluesleep_probe fail to request bt_ext_wake gpio!!!!\n");
        ret = BT_ERR_RET;
        goto free_bt_host_wake;
    }
    /* assert bt wake */
    ret = gpio_direction_output(bsi -> ext_wake, 0);
    if (ret){
        BT_ERR("bluesleep_probe bt_ext_wake gpio set output failed.\n");
        ret = BT_ERR_RET;
        goto free_bt_ext_wake;
    }

    btdb_printk(" %s exit.\n", __FUNCTION__);

    return 0;

free_bt_ext_wake:
    gpio_free(bsi -> ext_wake);
free_bt_host_wake:
    gpio_free(bsi -> host_wake);
free_bsi:
    kfree(bsi);

    return ret;
}
/******************************************************************************
  函数名称  : bluesleep_remove
  功能描述  : 移除设备            
  输入参数  : 
  调用函数  : 
  被调函数  : 
  输出参数  : NA
  返 回 值  :    NA             
  其它说明  ：
******************************************************************************/

static int bluesleep_remove(struct platform_device *pdev)
{
    btdb_printk("%s enter.\n", __FUNCTION__);
    /* assert bt wake */
    gpio_set_value(bsi->ext_wake, 0);
    ext_wake_state = 0;
    if (test_bit(BT_PROTO, &flags)) {
        free_irq(bsi->host_wake_irq, NULL);
        del_timer(&tx_timer);
        if (test_bit(BT_ASLEEP, &flags))
        {
            hsuart_power(1);
        }
    }

    gpio_free(bsi->host_wake);
    gpio_free(bsi->ext_wake);
    kfree(bsi);

    remove_proc_entry("asleep", sleep_dir);
    remove_proc_entry("proto", sleep_dir);
    remove_proc_entry("hostwake", sleep_dir);
    remove_proc_entry("btwake", sleep_dir);
    remove_proc_entry("sleep", bluetooth_dir);
    remove_proc_entry("bluetooth", 0);
    return 0;
}

static const struct of_device_id of_gpio_bluesleep_match[] = {
    { .compatible = "bluetoothsleep", },
    {},
};

static struct platform_driver bluesleep_driver = {
    .probe  = bluesleep_probe,
    .remove = bluesleep_remove,
    .driver = {
        .name = "bluesleep",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(of_gpio_bluesleep_match),
    },
};

module_platform_driver(bluesleep_driver);

MODULE_DESCRIPTION("Bluetooth Sleep Mode Driver ver %s " VERSION);
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
