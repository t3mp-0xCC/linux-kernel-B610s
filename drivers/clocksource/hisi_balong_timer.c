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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <asm/delay.h>

#include <mach/timer.h>
#include <soc_timer.h>

static void __iomem *clksrc_base = NULL;
static unsigned int clksrc_freq = 0;
static struct delay_timer balong_delay_timer;

static cycle_t balong_timer_read(struct clocksource *cs)
{
    return ~readl(clksrc_base + TIMER_VALUE);
}

static struct clocksource clocksource_balong = {
	.name		= "balong clk cs",
	.rating		= 200,
	.read		= balong_timer_read,
	.mask		= CLOCKSOURCE_MASK(32),
	.shift		= 20,
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

void __init balong_clocksource_init(void __iomem *base, unsigned int freq)
{
	struct clocksource *cs = &clocksource_balong;

	clksrc_base = base;
    clksrc_freq = freq;

	/* setup timer 9 as free-running clocksource */
	writel(0, clksrc_base + TIMER_CTRL);
	writel(0xffffffff, clksrc_base + TIMER_LOAD);
	acore_clocksource_enable_and_mask_int((unsigned)clksrc_base);

    clocksource_register_hz(cs, freq);
}

static void __iomem *clkevt_base = NULL;
static unsigned int clkevt_freq = 0;
/*
 * IRQ handler for the timer
 */
static irqreturn_t balong_timer_interrupt(int irq, void *dev_id)
{
    struct clock_event_device *evt = dev_id;
    unsigned long ctrl;

    /* clear the interrupt */
	systimer_int_clear((unsigned)clkevt_base);
	if (CLOCK_EVT_MODE_ONESHOT == evt->mode)
	{
		ctrl = readl(clkevt_base + TIMER_CTRL);
		ctrl &= ~(TIMER_CTRL_ENABLE);/* [false alarm]:Îó±¨ */
		writel(ctrl, clkevt_base + TIMER_CTRL);
	}

	evt->event_handler(evt);

	return IRQ_HANDLED;
}

static void balong_timer_set_mode(enum clock_event_mode mode,
	struct clock_event_device *evt)
{
	unsigned long ctrl = 0;

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		writel(clkevt_freq/HZ, clkevt_base + TIMER_LOAD);
		ctrl |= CLK_DEF_ENABLE;/* [false alarm]:Îó±¨ */
		break;

	case CLOCK_EVT_MODE_ONESHOT:
		/* period set, and timer enabled in 'next_event' hook */
		ctrl = 0;/* [false alarm]:Îó±¨ */
		break;

	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		break;
	}

	writel(ctrl, clkevt_base + TIMER_CTRL);
}

static int balong_timer_set_next_event(unsigned long next,
	struct clock_event_device *evt)
{
	writel(next, clkevt_base + TIMER_LOAD);
	writel(CLK_DEF_ENABLE, clkevt_base + TIMER_CTRL);

	return 0;
}

static struct clock_event_device balong_clockevent = {
	.name				= "balong clk evt",
	.shift				= 32,
	.features       	= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode			= balong_timer_set_mode,
	.set_next_event		= balong_timer_set_next_event,
	.rating				= 300,
	.cpumask			= cpu_all_mask,
};

static struct irqaction balong_timer_irq = {
	.name		= "balong system timer",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= balong_timer_interrupt,
	.dev_id		= &balong_clockevent,
};

void __init balong_clockevent_init(void __iomem *base, unsigned int timer_irq, unsigned int freq)
{
	struct clock_event_device *evt = &balong_clockevent;

	clkevt_base = base;
    clkevt_freq = freq;

	evt->irq = timer_irq;
	setup_irq(timer_irq, &balong_timer_irq);
    clockevents_config_and_register(evt, freq, 0xf, 0xffffffff);
}

static unsigned long balong_delay_timer_read_counter(void)
{
	return ~readl(clksrc_base + TIMER_VALUE);
}

static void __init hisi_ce_init(struct device_node *np)
{
   // struct device_node *np = NULL;
    void __iomem *base = NULL;
	unsigned int irq = 0;
    unsigned int freq = 0;

	base = of_iomap(np, 0);
	if (!base)
    {
		pr_err("Failed to map event base\n");
		return;
	}

	irq = irq_of_parse_and_map(np, 0);
	if (irq <= 0)
    {
		pr_err("Can't get irq\n");
		return;
	}

    if (of_property_read_u32(np, "clock-frequency", &freq))
    {
		pr_err("Unknown frequency\n");
		return;
	}
    printk(KERN_INFO "clockevent base = %x, irq = %d, freq = %d\n", (unsigned int)base, irq, freq);
    balong_clockevent_init(base, irq, freq);
}

static void __init hisi_cs_init(struct device_node *np)
{
    //struct device_node *np = NULL;
	void __iomem *base = NULL;
    unsigned int freq = 0;

	base = of_iomap(np, 0);
	if (!base)
    {
		pr_err("Failed to map event base\n");
		return;
	}

    if (of_property_read_u32(np, "clock-frequency", &freq))
    {
		pr_err("Unknown frequency\n");
		return;
	}
    printk(KERN_INFO "clocksource base = %x, freq = %d\n", (unsigned int)base, freq);
    balong_clocksource_init(base, freq);

    balong_delay_timer.read_current_timer = balong_delay_timer_read_counter;
	balong_delay_timer.freq = freq;
	register_current_timer_delay(&balong_delay_timer);
}

CLOCKSOURCE_OF_DECLARE(hisi_ce, "hisi,ce-timer", hisi_ce_init);
CLOCKSOURCE_OF_DECLARE(hisi_cs, "hisi,cs-timer", hisi_cs_init);



