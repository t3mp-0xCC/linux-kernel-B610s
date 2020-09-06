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

#include <linux/module.h>
#include <linux/rtc.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/amba/bus.h>
#include <linux/io.h>
#include <linux/bcd.h>
#include <linux/delay.h>
#include <linux/slab.h>


#include "linux/of.h"
#include "linux/of_address.h"
#include "linux/of_irq.h"

#define PL031_ERR_OK                (0)
#define PL031_ERR_NONODE            (-1)
#define PL031_ERR_NODEVICE          (-2)
#define PL031_ERR_NODAMBAEVICE      (-3)
#define PL031_ERR_INVALIDFUNC       (-4)
#define PL031_ERR_INVALIDPARA       (-5)

#define PL031_ERR_ERROR             (-20)

#define ALARM_DISABLE              (0)
#define ALARM_ENABLE               (1)
#define RTC_BASE_YEAR              1900

#define	RTC_DR		0x00	/* Data read register */
#define	RTC_MR		0x04	/* Match register */
#define	RTC_LR		0x08	/* Data load register */
#define	RTC_CR		0x0c	/* Control register */
#define	RTC_IMSC	0x10	/* Interrupt mask and set register */
#define	RTC_RIS		0x14	/* Raw interrupt status register */
#define	RTC_MIS		0x18	/* Masked interrupt status register */
#define	RTC_ICR		0x1c	/* Interrupt clear register */
#define RTC_BIT_AI	(1 << 0) /* Alarm interrupt bit */

struct pl031_vendor_data {
	struct rtc_class_ops ops;
	bool clockwatch;
	bool st_weekday;
	unsigned long irqflags;
};

extern int pl031_interrupt_count;
extern void *pl031_addr;
extern struct device *pl031_dev;
extern struct pl031_vendor_data arm_pl031; 
extern void *get_pl031_base(void);
extern struct device *get_pl031_dev(void);

int pl031_readtime_testcase0(void)
{
    struct rtc_time tm;
    struct device *dev = get_pl031_dev();
    
    if (!dev)
        return PL031_ERR_INVALIDPARA;

    arm_pl031.ops.read_time(dev, &tm);
    printk(" %4d-%02d-%02d %02d:%02d:%02d\n",
                RTC_BASE_YEAR + tm.tm_year, tm.tm_mon, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec);
    return PL031_ERR_OK;
}

int pl031_settime_test_case0(void)
{
    struct rtc_time tm = {
        .tm_year = 2014-1900,
        .tm_mon = 8,
        .tm_mday = 17,
        .tm_hour = 20,
        .tm_min = 30,
        .tm_sec = 17
    };
    
    struct device *dev = get_pl031_dev();
    if (!dev)
        return PL031_ERR_INVALIDPARA;
    arm_pl031.ops.set_time(dev, &tm);
    
    printk("[%s]2014.08.17 20:30:17 be set.invoke pl031_readtime_testcase0 to read the time\n",
                            __FUNCTION__);
    return 0;
}

int pl031_readalarm_testcase0(void)
{
    struct rtc_wkalrm alarm;
    struct device *dev = get_pl031_dev();
    if (!dev)
        return PL031_ERR_INVALIDPARA;
        
    arm_pl031.ops.read_alarm(dev,&alarm);
    printk("alarm.time %4d-%02d-%02d %02d:%02d:%02d, pending %d, enabled %d\n",
                                RTC_BASE_YEAR + alarm.time.tm_year, alarm.time.tm_mon, alarm.time.tm_mday,
                                alarm.time.tm_hour, alarm.time.tm_min, alarm.time.tm_sec,alarm.pending,alarm.enabled);
   return PL031_ERR_OK;
}

int pl031_setalarm_testcase0(void)
{
    struct rtc_time tm = {
        .tm_year = 2014-1900,
        .tm_mon = 8,
        .tm_mday = 17,
        .tm_hour = 20,
        .tm_min = 30,
        .tm_sec = 17
    };
    
    struct rtc_wkalrm alarm = {
        .time.tm_year = 2014-1900,
        .time.tm_mon = 8,
        .time.tm_mday = 17,
        .time.tm_hour = 20,
        .time.tm_min = 31,
        .time.tm_sec = 00,
        .enabled = 1,        
    };
    
    struct device *dev = get_pl031_dev();
    if (!dev)
        return PL031_ERR_INVALIDPARA;
        
    arm_pl031.ops.set_time(dev,&tm);
    arm_pl031.ops.set_alarm(dev,&alarm);
   
    return 0;
}

int get_pl031_interrupt_count(void)
{
    return pl031_interrupt_count;
}

int get_pl031_all_regs(void)
{
    int dr = 0;
    int mr = 0;
    int lr = 0;
    int cr = 0;
    int imsc = 0;
    int ris = 0;
    int mis = 0;
    int icr = 0;
    void *addr = get_pl031_base();
    if (addr == NULL)
        return -1;
    dr = readl(addr + RTC_DR);
    mr = readl(addr + RTC_MR);
    lr = readl(addr + RTC_LR);
    cr = readl(addr + RTC_CR);
    imsc = readl(addr + RTC_IMSC);
    ris = readl(addr + RTC_RIS);
    mis = readl(addr + RTC_MIS);
    icr = readl(addr + RTC_ICR);

    printk("dr %d\n",dr);
    printk("mr %d\n",mr);
    printk("lr %d\n",lr);
    printk("cr %d\n",cr);
    printk("imsc %d\n",imsc);
    printk("ris %d\n",ris);
    printk("mis %d\n",mis);
    printk("icr %d\n",icr);
    return 0;
}


