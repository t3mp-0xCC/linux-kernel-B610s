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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/pwm.h>
#include <asm/div64.h>
#include <osl_math64.h>
#include <soc_memmap.h>
#include <hi_pwm.h>
#include <mach/irqs.h>//modem
#include "pwm_balong.h"
#ifdef CONFIG_OF
#include <linux/of_address.h>
#include <linux/of.h>
#endif


/*
 * period_ns = 10^9 * PV/ PWM_CLK_RATE
 * duty_ns   = period_ns * (DC / PV)= 10^9 * DC / PWM_CLK_RATE
 */
int balong_pwm_config(struct pwm_chip *chip,
					  struct pwm_device *pwm,
					  int duty_ns, int period_ns)
{
    unsigned long long c;
    unsigned long pv, dc;
    int disable_flag = 0;
    struct balong_pwm_device *balong_pwm;

    if (chip == NULL || pwm == NULL || duty_ns < 0 || period_ns <= 0 || duty_ns > period_ns)
        return -EINVAL;

    balong_pwm = dev_get_drvdata(chip->dev);
    c = clk_get_rate(balong_pwm->clk);
    c = c * period_ns;
    c = div_u64(c, 1000000000);
    //do_div(c, 1000000000);
    pv = c;

    if (pv > 1024)
        return -EINVAL;

    dc = pv * duty_ns / period_ns;

    /* NOTE: the clock to PWM has to be enabled first
     * before writing to the registers
     */
    clk_enable(balong_pwm->clk);
    if(!(pwm->hwpwm))
    {
        __raw_writel(dc, balong_pwm->mmio_base + HI_PWM_OUTA_WIDE_OFFSET);
        __raw_writel(pv, balong_pwm->mmio_base + HI_PWM_OUTA_DIV_OFFSET);
    }
    else
    {
        __raw_writel(dc, balong_pwm->mmio_base + HI_PWM_OUTB_WIDE_OFFSET);
        __raw_writel(pv, balong_pwm->mmio_base + HI_PWM_OUTB_DIV_OFFSET);
    }

    /*只有两个通道都不使用的时候，才能关闭pwm时钟*/
    if(!readl(balong_pwm->mmio_base + HI_PWM_OUTA_EN_OFFSET) && !readl(balong_pwm->mmio_base + HI_PWM_OUTB_EN_OFFSET))
        disable_flag = 1;
    if(disable_flag)
        clk_disable(balong_pwm->clk);

    return 0;
}

int balong_pwm_enable(struct pwm_chip *chip,
					  struct pwm_device *pwm)
{
    struct balong_pwm_device *balong_pwm;
    
    int rc = 0;
    
    balong_pwm = dev_get_drvdata(chip->dev);
    if (!balong_pwm->clk_enabled) {
        rc = clk_enable(balong_pwm->clk);
        if (!rc)
            balong_pwm->clk_enabled = 1;
    }
    if(!(pwm->hwpwm))
        writel(1, balong_pwm->mmio_base + HI_PWM_OUTA_EN_OFFSET);

    else
        writel(1, balong_pwm->mmio_base + HI_PWM_OUTB_EN_OFFSET);

    balong_pwm->running = 1;
    return rc;
}

void balong_pwm_disable(struct pwm_chip *chip,
					   struct pwm_device *pwm)
{
    struct balong_pwm_device *balong_pwm;
    int disable_flag = 0;

    balong_pwm = dev_get_drvdata(chip->dev);
    if(!(pwm->hwpwm))
        writel(0, balong_pwm->mmio_base + HI_PWM_OUTA_EN_OFFSET);
    else
        writel(0, balong_pwm->mmio_base + HI_PWM_OUTB_EN_OFFSET);

    /*只有两个通道都不使用的时候，才能关闭pwm时钟*/
    if(!readl(balong_pwm->mmio_base + HI_PWM_OUTA_EN_OFFSET) && !readl(balong_pwm->mmio_base + HI_PWM_OUTB_EN_OFFSET))
        disable_flag = 1;

    if ((balong_pwm->clk_enabled)&&(disable_flag)) {
        clk_disable(balong_pwm->clk);
        balong_pwm->clk_enabled = 0;
    }
    balong_pwm->running = 0;
}

int balong_pwm_request(struct pwm_chip *chip,
					   struct pwm_device *pwm)
{
    struct balong_pwm_device *b_pwm;
	int ret;

	if (pwm->hwpwm > chip->npwm)
		return -EINVAL;

	b_pwm = kzalloc(sizeof(*b_pwm), GFP_KERNEL);
	if (!b_pwm)
		return -ENOMEM;

	pwm_set_chip_data(pwm, b_pwm);

	return 0;
}

static void balong_pwm_free(struct pwm_chip *chip, struct pwm_device *pwm)
{
	struct balong_pwm_device *b_pwm = pwm_get_chip_data(pwm);

	if (b_pwm) {
		kfree(b_pwm);
	}
}

static struct pwm_ops balong_pwm_ops = {
	.request = balong_pwm_request,
	.free = balong_pwm_free,
	.config = balong_pwm_config,
	.enable = balong_pwm_enable,
	.disable = balong_pwm_disable,
	.owner = THIS_MODULE,
};

int balong_pwm_probe(struct platform_device *pdev)
{
    struct balong_pwm_device *pwm = NULL;
    struct resource *r;
    unsigned char i;
    int ret = 0;
    static int id=0;
    
    pwm = devm_kzalloc(&pdev->dev, sizeof(*pwm), GFP_KERNEL);
    if (!pwm) {
		dev_err(&pdev->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}
    
    pwm->chip.dev = &pdev->dev;
	pwm->chip.ops = &balong_pwm_ops;
	pwm->chip.base = (id * CHANNEL_NUM);
	pwm->chip.npwm = CHANNEL_NUM;

    if(id == 0)
    {
        pwm->clk = clk_get(NULL, "pwm0_clk");
    }
    else
    {
        pwm->clk = clk_get(NULL, "pwm1_clk");
    }

    if (IS_ERR(pwm->clk)) {
        ret = PTR_ERR(pwm->clk);
        goto err_free;
    }

    ret = clk_prepare(pwm->clk);
    if (ret) {
		printk("Clock prepare failed\n");
		goto err_free;
	}

    ret = pwmchip_add(&pwm->chip);
	if (ret < 0) {
		dev_err(&pdev->dev, "pwmchip_add() failed: %d\n", ret);
		goto err_free_chip;
	}
    
    pwm->clk_enabled = 0;
    pwm->pdev = pdev;

    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    
    if (r == NULL) {
        dev_err(&pdev->dev, "no memory resource defined\n");
        ret = -ENODEV;
        goto err_free_clk;
    }

    r = request_mem_region(r->start, resource_size(r), pdev->name);
    
    if (r == NULL) {
        dev_err(&pdev->dev, "failed to request memory resource\n");
        ret = -EBUSY;
        goto err_free_clk;
    }
    pwm->mmio_base = ioremap(r->start, resource_size(r));
    if (pwm->mmio_base == NULL) {
        dev_err(&pdev->dev, "failed to ioremap() registers\n");
        ret = -ENODEV;
        goto err_free_mem;
    }

    platform_set_drvdata(pdev, pwm);

    dev_err(&pdev->dev, "balong_pwm_probe ok! \n");
    id++;
    return 0;
    
err_free_mem:
    release_mem_region(r->start, resource_size(r));
err_free_clk:
    clk_put(pwm->clk);
err_free_chip:
    pwmchip_remove(&pwm->chip);
err_free:
    kfree(pwm);

    return ret;
}
int balong_pwm_remove(struct platform_device *pdev)
{
    struct balong_pwm_device *pwm;
    struct resource *r;

    pwm = platform_get_drvdata(pdev);
    if (pwm == NULL)
        return -ENODEV;

    pwmchip_remove(&pwm->chip);

    iounmap(pwm->mmio_base);

    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    release_mem_region(r->start, resource_size(r));

    clk_put(pwm->clk);
    kfree(pwm);
    return 0;
}
#if 0
static struct resource balong_resource_pwm0[] = {
    [0] = {
        .start  = HI_PWM0_REGBASE_ADDR,
        .end    = HI_PWM0_REGBASE_ADDR + 0x30,
        .flags  = IORESOURCE_MEM,
    },
};


struct platform_device balong_device_pwm0 = {
    .name       = "balong-pwm",
    .id     = 0,
    .resource   = balong_resource_pwm0,
    .num_resources  = ARRAY_SIZE(balong_resource_pwm0),
};


static struct resource balong_resource_pwm1[] = {
    [0] = {
        .start  = HI_PWM1_REGBASE_ADDR,
        .end    = HI_PWM1_REGBASE_ADDR + 0x30,
        .flags  = IORESOURCE_MEM,
    },
};


struct platform_device balong_device_pwm1 = {
    .name       = "balong-pwm",
    .id     = 1,
    .resource   = balong_resource_pwm1,
    .num_resources  = ARRAY_SIZE(balong_resource_pwm1),
};
#endif

#ifdef CONFIG_OF
static const struct of_device_id balong_pwm_of_match[] = {
	{ .compatible = "hisilicon,pwm_balong_app", },
	{},
};
MODULE_DEVICE_TABLE(of, balong_pwm_of_match);
#endif

static struct platform_driver balong_pwm_driver = {
    .driver     = {
        .name   = "balong-pwm",
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(balong_pwm_of_match),
    },
    .probe      = balong_pwm_probe,
    .remove     = balong_pwm_remove,
};
int balong_pwm_init(void)
{
    int ret;
    
#if 0
    ret = platform_device_register(&balong_device_pwm0);
    if(ret)
        printk(KERN_ERR "pwm0:platform_device_register err! \n");
    ret = platform_device_register(&balong_device_pwm1);
    if(ret)
        printk(KERN_ERR "pwm1:platform_device_register err! \n");
#endif
    ret = platform_driver_register(&balong_pwm_driver);
    if (ret)
        printk(KERN_ERR "%s: failed to add pwm driver\n", __func__);

    return ret;
}
arch_initcall(balong_pwm_init);

static void __exit balong_pwm_exit(void)
{
    platform_driver_unregister(&balong_pwm_driver);
}
module_exit(balong_pwm_exit);
