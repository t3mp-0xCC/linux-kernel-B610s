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
#include <linux/clk-provider.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/clocksource.h>
#include <linux/irqchip.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/hardware/cache-l2x0.h>
#include <mach/board.h>
#include <bsp_memmap.h>

#include "bsp_onoff.h"

static const struct of_dev_auxdata balong_auxdata_lookup[] __initconst = {
	OF_DEV_AUXDATA("arm,primecell", 0, NULL, NULL),
	{}
	};

static void __init hisi_balong_timer_init(void)
{
#ifdef CONFIG_COMMON_CLK
	of_clk_init(NULL);
#endif
	clocksource_of_init();
}

static void __init balong_init(void)
{
    int ret = 0;
    ret = l2x0_of_init(L2_AUX_VAL, L2_AUX_MASK);

    of_platform_populate(NULL, of_default_bus_match_table, balong_auxdata_lookup, NULL);

#if defined(CONFIG_BALONG_ONOFF)
	pm_power_off = balong_power_off;
#endif
}

static void __init balong_reserve(void)
{
}
void __init balong_init_early(void)
{
}

void __init balong_init_late(void)
{
}

void balong_restart(char mode, const char *cmd)
{
#if defined(CONFIG_BALONG_ONOFF)
    balong_power_restart(mode, cmd);
#endif

}

static struct map_desc balong_iodesc[] __initdata = {

	/* DTS无法做到内核动态映射 */
#if (!defined CONFIG_ARM_APPENDED_DTB) && (defined DDR_ACORE_DTS_ADDR)
	{
		.virtual	= HI_IO_ADDRESS(DDR_ACORE_DTS_ADDR),
		.pfn		= __phys_to_pfn(DDR_ACORE_DTS_ADDR),
		.length		= DDR_ACORE_DTS_SIZE,
		.type		= MT_DEVICE,
	},
#endif

};
static void __init balong_map_io(void)
{
    iotable_init(balong_iodesc, ARRAY_SIZE(balong_iodesc));
}

static char const *p532_dt_compat[] __initdata = {
	"hisilicon,p532",
	NULL
};

DT_MACHINE_START(P532_DT, "P532 (Flattened Device Tree)")
	.dt_compat	  = p532_dt_compat,
	.reserve      = balong_reserve,
	.map_io		  = balong_map_io,
	.init_early	  = balong_init_early,
	.init_irq	  = irqchip_init,
	.init_time	  = hisi_balong_timer_init,
	.init_machine = balong_init,
	.init_late	  = balong_init_late,
	.restart      = balong_restart,
MACHINE_END

static char const *v711_dt_compat[] __initdata = {
	"hisilicon,v711",
	NULL
};

DT_MACHINE_START(V711_DT, "V711 (Flattened Device Tree)")
	.dt_compat	  = v711_dt_compat,
	.reserve      = balong_reserve,
	.map_io		  = balong_map_io,
	.init_early	  = balong_init_early,
	.init_irq	  = irqchip_init,
	.init_time	  = hisi_balong_timer_init,
	.init_machine = balong_init,
	.init_late	  = balong_init_late,
	.restart      = balong_restart,
MACHINE_END

static char const *hi6950_dt_compat[] __initdata = {
	"hisilicon,hi6950",
	NULL
};

DT_MACHINE_START(HI6950_DT, "HI6950 (Flattened Device Tree)")
	.dt_compat	  = hi6950_dt_compat,
	.reserve      = balong_reserve,
	.map_io		  = balong_map_io,
	.init_early	  = balong_init_early,
	.init_irq	  = irqchip_init,
	.init_time	  = hisi_balong_timer_init,
	.init_machine = balong_init,
	.init_late	  = balong_init_late,
	.restart      = balong_restart,
MACHINE_END

static char const *hi6932_dt_compat[] __initdata = {
	"hisilicon,hi6932",
	NULL
};

DT_MACHINE_START(HI6932_DT, "HI6932 (Flattened Device Tree)")
	.dt_compat	  = hi6932_dt_compat,
	.reserve      = balong_reserve,
	.map_io		  = balong_map_io,
	.init_early	  = balong_init_early,
	.init_irq	  = irqchip_init,
	.init_time	  = hisi_balong_timer_init,
	.init_machine = balong_init,
	.init_late	  = balong_init_late,
	.restart      = balong_restart,
MACHINE_END
