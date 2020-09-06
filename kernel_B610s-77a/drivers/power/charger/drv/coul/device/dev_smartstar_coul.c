/*
 * arch/arm/mach-hi6620/dev_smartstar_coul.c
 *
 * Copyright (C) 2013 Hisilicon Co. Ltd.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#include <linux/platform_device.h>
#include <linux/io.h>

#include <linux/slab.h>
#include <linux/string.h>
#include <dev_smartstar_coul.h>

#include <bsp_pmu.h>

#define SMARTSTAR_COUL_ENABLE "gas_gauge/ss_coul_enable"

static struct resource  smartstar_coul_resource[] = {

    [0] = {
            .name  = "cl_int",
            .start = COUL_INT_ID,
            .end   = COUL_INT_ID,
            .flags = IORESOURCE_IRQ,
    },
    [1] = {
            .name  = "cl_out_D_int",
            .start = COUL_OUT_ID,
            .end   = COUL_OUT_ID,
            .flags = IORESOURCE_IRQ,
    },
    [2] = {
            .name  = "cl_in_D_int",
            .start = COUL_IN_ID,
            .end   = COUL_IN_ID,
            .flags = IORESOURCE_IRQ,
    },
    [3] = {
            .name  = "vbat_int",
            .start = COUL_VBAT_INT_ID,
            .end   = COUL_VBAT_INT_ID,
            .flags = IORESOURCE_IRQ,
    },

};


static struct platform_device smartstar_coul_device = {
    .name                   = "hisi_smartstar_coul",
    .id                             = -1,
    .resource                       = (struct resource	*)&smartstar_coul_resource,
    .num_resources  = ARRAY_SIZE(smartstar_coul_resource),
};


static int __init smartstar_coul_dev_init(void)
{
    int ret = 0, enable = 0;

    ret = platform_device_register(&smartstar_coul_device);
    return ret;
}

arch_initcall(smartstar_coul_dev_init);
