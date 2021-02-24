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

/*lint --e{438,451,537,550,651,661,737} */
/* 438 -- Last value assigned to variable not used
 * 451 -- repeatedly included but does not have a standard include guard
 * 537 -- Repeated include file
 * 550 -- Symbol not accessed
 * 651 -- Potentially confusing initializer
 * 661 -- Possible access of out-of-bounds pointer
 * 737 -- Loss of sign in promotion from int to unsigned int
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/io.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/types.h>
#include <linux/of_address.h>

#include <soc_onchiprom.h>
#include <product_config.h>


#include <hi_rsa_reg.h>

static void* g_rsa_base = NULL;

/* Computes a = b^c mod d.

   Lengths: a[dDigits], b[dDigits], c[cDigits], d[dDigits].
     Assumes d > 0, cDigits > 0, dDigits < MAX_NN_DIGITS_EX.
 */
void rsa_mod_expex(unsigned int *a, unsigned int *b, unsigned int *c, unsigned int cDigits, unsigned int *d, unsigned int dDigits, unsigned int *key_c)
{
    unsigned i = 0;
    unsigned value = 0;
    struct clk *rsa_clk;

    rsa_clk = clk_get(NULL, "rsa_clk");
    if (IS_ERR(rsa_clk)) {
        printk(KERN_ERR"fail to get sign verify tool clk\n");
        return;
    }

    (void)clk_prepare(rsa_clk);
    (void)clk_enable(rsa_clk);

    /* wait for not busy */
    do {
        value = readl(g_rsa_base + HI_RSA_BUSY_REG) & 0x01;
    } while (value);

    /* calc mode select */
    value = readl(g_rsa_base + HI_RSA_MULT_EXP_MODULE_REG);
    value &= 0xFFFFFFFE;
    writel(value, g_rsa_base + HI_RSA_MULT_EXP_MODULE_REG);

    /* key mode select */
    value = readl(g_rsa_base + HI_RSA_ROOTKEY_MODE_REG);
    value &= 0xFFFFFFFE;
    writel(value, g_rsa_base + HI_RSA_ROOTKEY_MODE_REG);

    for (i = 0; i < MAX_N_LENGTH; i++)
        writel(*(b+i), g_rsa_base + HI_RSA_OPERAND_M_REG + i * 4);

    for (i = 0; i < MAX_N_LENGTH; i++)
        writel(*(c+i), g_rsa_base + HI_RSA_OPERAND_E_REG + i * 4);

    for (i = 0; i < MAX_N_LENGTH; i++)
        writel(*(d+i), g_rsa_base + HI_RSA_OPERAND_N_REG + i * 4);

    for (i = 0; i < MAX_N_LENGTH; i++)
        writel(key_c[i], g_rsa_base + HI_RSA_OPERAND_C_REG + i * 4);

    writel(((dDigits + 32 - 1) >> 5) & 0x1F, g_rsa_base + HI_RSA_N_LENGTH_REG);
    writel((cDigits + 64 - 1) >> 6, g_rsa_base + HI_RSA_E_LENGTH_REG);

    writel(0x01, g_rsa_base + HI_RSA_ST_REG);

    do {
        value = readl(g_rsa_base + HI_RSA_BUSY_REG) & 0x01;
    } while (value);

    value = readl(g_rsa_base + HI_RSA_ERROR_REG) & 0x01;
    if (value) {
        printk(KERN_ERR"sign verify tool error\n");
        goto error_calc;
    }

    for (i = 0; i < MAX_N_LENGTH; i++)
        *(a+i) = readl(g_rsa_base + HI_RSA_RESULT_R_REG + i * 4);

error_calc:
    clk_disable(rsa_clk);
    clk_unprepare(rsa_clk);
    clk_put(rsa_clk);
}

static int __init rsa_init(void)
{
    int ret = 0;
    struct device_node *dev_node = NULL;
    struct clk *rsa_clk;

    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,rsa");
    if (!dev_node) {
        printk(KERN_ERR"fail to find sign verify tool dev node\n");
        return -1;
    }

    g_rsa_base = (void*)of_iomap(dev_node, 0);
    if (!g_rsa_base) {
        printk(KERN_ERR"fail to of_iomap sign verify tool dev node\n");
        return -1;
    }

    rsa_clk = clk_get(NULL, "bootrom_clk");
    if (IS_ERR(rsa_clk)) {
        printk(KERN_ERR"fail to get bootrom clk\n");
        return ret;
    }

    (void)clk_prepare(rsa_clk);
    (void)clk_enable(rsa_clk);
    clk_disable(rsa_clk);
    clk_unprepare(rsa_clk);
    clk_put(rsa_clk);

    rsa_clk = clk_get(NULL, "rsa_clk");
    if (IS_ERR(rsa_clk)) {
        printk(KERN_ERR"fail to get sign verify tool clk\n");
        return ret;
    }

    (void)clk_prepare(rsa_clk);
    (void)clk_enable(rsa_clk);
    clk_disable(rsa_clk);
    clk_unprepare(rsa_clk);
    clk_put(rsa_clk);

    printk(KERN_INFO"sign verify tool init OK\n");

    return ret;
}

subsys_initcall(rsa_init);


