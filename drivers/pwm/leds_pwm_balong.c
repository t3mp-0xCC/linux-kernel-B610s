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
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/leds.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/leds_pwm.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <drv_nv_def.h>
#include <drv_nv_id.h>
#include <bsp_nvim.h>
#ifdef CONFIG_OF
#include <linux/of_address.h>
#include <linux/of.h>
#endif

unsigned int g_pwm_led_nv;

static struct led_pwm balong_leds[] = {
    [0] = {
        .name       = "pwm_led0",
        .pwm_id     = 0,
        .max_brightness = 1023,
        .pwm_period_ns  = 4877,/*53333*/
    },
    [1] = {
        .name       = "pwm_led1",
        .pwm_id     = 1,
        .max_brightness = 1023,
        .pwm_period_ns  = 53333,/*53333*/
    },
};
static struct led_pwm_platform_data balong_leds_pwm_data = {
    .num_leds   = ARRAY_SIZE(balong_leds),
    .leds       = balong_leds,
};
static struct platform_device balong_led_pwm_device = {
    .name       = "leds_pwm",
    .dev        = {
        //.parent = &balong_device_pwm1.dev,
        .platform_data = &balong_leds_pwm_data,
    },
    .id =-1,
};

static struct pwm_lookup led_pwm_lookup[] = {
    PWM_LOOKUP("", 0, "leds_pwm", "pwm_led0"),
    PWM_LOOKUP("", 1, "leds_pwm", "pwm_led1"),
};

static int pwm_led_nv_init(void)
{
    DRV_MODULE_TEST_STRU support;
    unsigned int ret;

    ret = bsp_nvm_read(NV_ID_DRV_TEST_SUPPORT, (u8*)&support, sizeof(DRV_MODULE_TEST_STRU));
    if(ret){
        support.pwm = 0;
        printk("lcd read nv fail!\n");
    }

    g_pwm_led_nv = support.pwm;

    return (int)ret;

}

int __init balong_led_pwm_init(void)
{
    int ret = 0;
    struct platform_device *pwm_pdev = NULL;
    struct device_node *dev_node = NULL;

        /* 读取nv，如果nv中对应bit为0，则不支持lcd，不注册，直接返回 */
    if (pwm_led_nv_init() || !g_pwm_led_nv)
    {
        printk("led pwm is not support.\n");
        return ret;
    }
    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,pwm_balong_app");
    if(NULL == dev_node)
    {
        printk("pwm device node not found!\n");
        return -1;
    }
    pwm_pdev = of_find_device_by_node(dev_node);
    if(NULL == pwm_pdev)
    {
        printk("failed to get platform device!\n");
        return -1;
    }
    led_pwm_lookup[0].provider = pwm_pdev->name;
    led_pwm_lookup[1].provider = pwm_pdev->name;

    pwm_add_table(led_pwm_lookup, ARRAY_SIZE(led_pwm_lookup));
    ret = platform_device_register(&balong_led_pwm_device);
    if(ret)
        printk(KERN_ERR "led-pwm:platform_device_register err! \n");
    return ret;
}
module_init(balong_led_pwm_init);

static void __exit balong_led_pwm_exit(void)
{
    //platform_driver_unregister(&led_pwm_driver);
}
module_exit(balong_led_pwm_exit);

MODULE_AUTHOR("Luotao Fu <l.fu@pengutronix.de>");
MODULE_DESCRIPTION("PWM LED driver for PXA");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:leds-pwm");
