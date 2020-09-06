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

/*lint -save -e537*/
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <osl_bio.h>
#include <osl_types.h>
#include <bsp_om.h>
#include <bsp_pmu.h>
#include "pmic_irq.h"

#define ALARM_ENABLE   (1)
#define ALARM_DISABLED (0)
#define PMU_BASE_ADDR  (bsp_pmic_get_base_addr())
#define RTC_REG_MASK   (0xff)

#define rtc_log(fmt,...) (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_RTC, "[rtc]:[func: %s],[line: %d]"fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__))

struct pmu_alm_info
{
    int irq;
    int id;
    u32 base;
    struct rtc_device *rtc_dev;
};

struct pmu_rtc_info
{
    u32 rtc_curr; /* curr register*/
    u32 rtc_load; /* load register*/
    u32 rtc_ctrl; /* ctrl register*/
    u32 rst_addr; /* 软复位寄存器地址 */
    u32 rst_data; /* 软复位寄存器配置值 */
    u32 clk_addr; /* 时钟使能及校准地址 */
    u32 clk_data; /* 时钟使能及校准配置值 */
    u32 alm_num;
    u32 init_flag;
    spinlock_t lock;
    struct pmu_alm_info* alm_info;
};

struct pmu_rtc_info g_pmu_rtc;

static unsigned pmu_read_reg(unsigned addr)
{
    unsigned value = 0;

    value = readl((void*)(PMU_BASE_ADDR + (addr << 2)));

    return value;
}

static void pmu_write_reg(unsigned addr, unsigned value)
{
    writel(value, (void*)(PMU_BASE_ADDR + (addr << 2)));

    return;
}

static unsigned pmu_rtc_read(unsigned addr)
{
    unsigned val0  = 0;
    unsigned val1  = 0;
    unsigned val2  = 0;
    unsigned val3  = 0;
    unsigned value = 0;

    val0 = pmu_read_reg(addr + 0);
    val1 = pmu_read_reg(addr + 1);
    val2 = pmu_read_reg(addr + 2);
    val3 = pmu_read_reg(addr + 3);

    value = val0 | (val1 << 8) | (val2 << 16) | (val3 << 24);

    return value;
}

static void pmu_rtc_write(unsigned addr, unsigned value)
{
    unsigned val0  = 0;
    unsigned val1  = 0;
    unsigned val2  = 0;
    unsigned val3  = 0;

    val0 = ((value) >> 0)  & RTC_REG_MASK;
    val1 = ((value) >> 8)  & RTC_REG_MASK;
    val2 = ((value) >> 16) & RTC_REG_MASK;
    val3 = ((value) >> 24) & RTC_REG_MASK;

    pmu_write_reg(addr + 0, val0);
    pmu_write_reg(addr + 1, val1);
    pmu_write_reg(addr + 2, val2);
    pmu_write_reg(addr + 3, val3);

    return;
}

u32 mdrv_rtc_get_value(void)
{
    unsigned long rtc_flag = 0;
    u32           value    = 0;

    if(!!PMU_BASE_ADDR && g_pmu_rtc.init_flag)
    {
        spin_lock_irqsave(&(g_pmu_rtc.lock), rtc_flag);
        value = pmu_rtc_read(g_pmu_rtc.rtc_curr);
        spin_unlock_irqrestore(&(g_pmu_rtc.lock), rtc_flag);
    }

    return value;
}
EXPORT_SYMBOL(mdrv_rtc_get_value);

/**
 * pmu_rtc_read_time-read the RTC time
 * @dev:	the rtc device.
 * @tm:		the RTC time pointer which point to the RTC time
 *
 */
int pmu_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
    u32 value = mdrv_rtc_get_value();

    rtc_time_to_tm((unsigned long)value, tm);

    dev_dbg(dev, "%s: %4d-%02d-%02d %02d:%02d:%02d\n", "readtime",
            1900 + tm->tm_year, tm->tm_mon, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);

    return BSP_OK;
}

/**
 * pmu_rtc_set_time-set the RTC time
 * @dev:	the RTC device.
 * @tm:		the RTC time which will be set
 *
 */
int pmu_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
    u32           seconds          = 0;
    int           ret           = 0;
    u32           alm_time    = 0;
    int           alm_enabled = 0;
    u32           alm_id      = 0;
    u32           curr_time     = 0; /*当前的时间值*/
    unsigned long rtc_flag      = 0;

    ret = rtc_valid_tm(tm);
    if (0 != ret)
    {
        rtc_log("rtc_valid_tm error, ret = %d\n", ret);
        return BSP_ERROR;
    }

    ret = rtc_tm_to_time(tm, (unsigned long *)&seconds);
    if (ret != 0)
    {
        rtc_log("rtc_tm_to_time error, ret = %d\n", ret);
        return BSP_ERROR;
    }

    spin_lock_irqsave(&(g_pmu_rtc.lock), rtc_flag);
    curr_time = pmu_rtc_read(g_pmu_rtc.rtc_curr);

    for(alm_id = 0; alm_id < g_pmu_rtc.alm_num; alm_id++)
    {
        alm_time = pmu_rtc_read(g_pmu_rtc.alm_info[alm_id].base);

        /*只有在设置了闹钟的时候才进行下面的重新设置闹钟时间动作*/
        /*查询是否使能了该闹钟*/
        alm_enabled = pmic_irq_is_masked(g_pmu_rtc.alm_info[alm_id].irq);
        if ((alm_enabled) && (curr_time < alm_time))
        {
            /* disable interrupts */
            pmic_irq_mask(g_pmu_rtc.alm_info[alm_id].irq);
            alm_time = seconds + (alm_time - curr_time);
            pmu_rtc_write(g_pmu_rtc.alm_info[alm_id].base, alm_time);
            /* enable alarm interrupts */
            pmic_irq_unmask(g_pmu_rtc.alm_info[alm_id].irq);
        }
    }

    pmu_rtc_write(g_pmu_rtc.rtc_load, seconds);
    spin_unlock_irqrestore(&(g_pmu_rtc.lock), rtc_flag);

    return BSP_OK;
}

/**
 * pmu_rtc_alarm_irq_enable-enable or disenable RTC alarm irq
 * @dev:	the RTC device.
 * @enabled:		1: enable the RTC alrm irq
 *					0: disenable the RTC alarm irq
 *
 */
int pmu_rtc_alarm_irq_enable(struct device *dev, unsigned int enabled)
{
    struct platform_device *rtc_dev = NULL;
    u32                     alm_id  = 0;

    rtc_dev = to_platform_device(dev);
    alm_id  = rtc_dev->id;

    /* if interupt enabled,unmask */
    /* if interupt unenabled,mask */
    if (ALARM_DISABLED == enabled)
    {
        pmic_irq_mask(g_pmu_rtc.alm_info[alm_id].irq);
    }
    else
    {
        pmic_irq_unmask(g_pmu_rtc.alm_info[alm_id].irq);
    }

    return BSP_OK;
}

/**
 * pmu_rtc_read_alarm-read rtc alarm
 */
int pmu_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
    u32                     alm_time = 0;
    struct pmu_alm_info*    alm_info = NULL;
    unsigned long           rtc_flag = 0;
    struct platform_device *rtc_dev  = to_platform_device(dev);
    u32                     alm_id   = rtc_dev->id;

    alm_info = &g_pmu_rtc.alm_info[alm_id];

    spin_lock_irqsave(&(g_pmu_rtc.lock), rtc_flag);
    alm_time = pmu_rtc_read(alm_info->base);    
    spin_unlock_irqrestore(&(g_pmu_rtc.lock), rtc_flag);

    rtc_time_to_tm(alm_time, &(alarm->time));
    alarm->pending = 0;
    if(pmic_irq_is_masked(alm_info->irq))
    {
        alarm->enabled = ALARM_DISABLED;
    }
    else
    {
        alarm->enabled = ALARM_ENABLE;
    }

    return rtc_valid_tm(&alarm->time);
}

/**
 * pmu_rtc_set_alarm-set rtc alarm
 */
int pmu_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
    s32                     ret       = 0;
    unsigned long           alm_time  = 0;
    u32                     curr_time = 0;
    struct pmu_alm_info*    alm_info  = NULL;
    unsigned long           rtc_flag  = 0;
    struct platform_device *rtc_dev   = to_platform_device(dev);
    u32                     alm_id    = rtc_dev->id;

    alm_info = &g_pmu_rtc.alm_info[alm_id];

    pmic_irq_mask(alm_info->irq);

    ret = rtc_valid_tm(&alarm->time);
    if (0 != ret)
    {
        rtc_log("rtc_valid_tm error, ret = %d.\n", ret);
        return BSP_ERROR;
    }

    ret = rtc_tm_to_time(&alarm->time, &alm_time);
    if (0 != ret)
    {
        rtc_log("rtc_tm_to_time error, ret = %d.\n", ret);
        return BSP_ERROR;
    }

    spin_lock_irqsave(&(g_pmu_rtc.lock), rtc_flag);
    curr_time = pmu_rtc_read(g_pmu_rtc.rtc_curr);
    if(curr_time <= alm_time)
    {
        pmu_rtc_write(alm_info->base, alm_time);
    }
    else
    {
        rtc_log("pmu_rtc_set_alarm error, curr_time <= alm_time\n");
    }
    spin_unlock_irqrestore(&(g_pmu_rtc.lock), rtc_flag);

    pmu_rtc_alarm_irq_enable(dev, alarm->enabled);

    return BSP_OK;
}

/**
 * pmu_rtc_alarm_handler-the rtc irq handler function
 */
void pmu_rtc_alarm_handler(void *data)
{
    u32 events = 0;
    struct pmu_alm_info *alm_info = NULL;

    events   = RTC_IRQF | RTC_AF;
    alm_info = (struct pmu_alm_info *)data;
    rtc_update_irq(alm_info->rtc_dev, 1, events);
    pmic_irq_mask(alm_info->irq);

    rtc_log("=========alram is handled==========\n");

    return;
}

/*RTC操作函数数据结构*/
/*lint -save -e527*/
static const struct rtc_class_ops pmu_rtc_ops =
{
    /*lint -restore*/
    .read_time	= pmu_rtc_read_time,
    .set_time	= pmu_rtc_set_time,
    .read_alarm	= pmu_rtc_read_alarm,
    .set_alarm	= pmu_rtc_set_alarm,
    .alarm_irq_enable = pmu_rtc_alarm_irq_enable,
};

/*lint -save -e563*/
int pmu_rtc_probe(struct platform_device *pdev)
{
    struct device        *dev      = &pdev->dev;
    struct device_node   *np       = dev->of_node;
    struct rtc_device    *rtc_dev  = NULL;
    struct pmu_alm_info  *alm_info = NULL;
    int                   ret      = 0;



    /*read dts*/
    ret = of_property_read_u32(np, "id_index", (u32 *)&pdev->id);
    if(ret)
    {
        rtc_log("of_property_read_u32 id_index error, ret = %d.\n", ret);
        goto error_ret;
    }

    alm_info = &g_pmu_rtc.alm_info[pdev->id];

    alm_info->id = pdev->id;

    ret = of_property_read_u32(np, "base", &alm_info->base);
    if(ret)
    {
        rtc_log("of_property_read_u32 base error, ret = %d.\n", ret);
        goto error_ret;
    }
    ret = of_property_read_u32_index(np, "interrupts", 0, &alm_info->irq);
    if (ret || (alm_info->irq < 0))
    {
        rtc_log("of_property_read_u32_index interrupts error, ret = %d.\n", ret);
        ret = -ENODEV;
        goto error_ret;
    }
    device_init_wakeup(&pdev->dev, true);

    rtc_dev = rtc_device_register(pdev->name, &pdev->dev, &pmu_rtc_ops, THIS_MODULE);
    if (IS_ERR(rtc_dev))
    {
        ret = PTR_ERR(rtc_dev);
        rtc_log("rtc_device_register error, ret = %d!\n",ret);
        goto error_ret;
    }

    platform_set_drvdata(pdev, rtc_dev);

    alm_info->rtc_dev = rtc_dev;

    /* coverity[noescape] */
    ret = pmic_irq_callback_register((unsigned int)alm_info->irq,
        pmu_rtc_alarm_handler, alm_info);
    if (ret != 0)
    {
        rtc_log("rtc request_irq fail\n");
        goto unreg_ret;
    }

    rtc_log("pmic RTC[%d] init OK!\n", pdev->id);

    return BSP_OK;

unreg_ret:
    rtc_device_unregister(rtc_dev);

error_ret:
    return BSP_ERROR;
}

/*lint -restore*/
static int  pmu_rtc_remove(struct platform_device *pdev)
{
    struct rtc_device *rtc_dev = platform_get_drvdata(pdev);
    if(!rtc_dev)
    {
        return BSP_ERROR;
    }

    pmic_irq_mask(g_pmu_rtc.alm_info[pdev->id].irq);

    rtc_device_unregister(rtc_dev);

    rtc_dev = NULL;

    return BSP_OK;
}

static const struct of_device_id pmu_rtc_of_match[] =
{
    { .compatible = "hisilicon,pmic_rtc_alarm0",},
    {},
};
MODULE_DEVICE_TABLE(of, pmu_rtc_of_match);

struct platform_driver pmu_rtc_driver =
{

    .driver = {
        .name = "pmu_rtc",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(pmu_rtc_of_match),
    },
    .probe	= pmu_rtc_probe,
    .remove = pmu_rtc_remove,

};

int pmu_get_rtc_dts(void)
{
    int                 ret      = 0;
    struct device_node *dev_node = NULL;

    dev_node = of_find_compatible_node(NULL, NULL, "arm,rtc_pmu");
    if (!dev_node)
    {
        rtc_log("of_find_compatible_node arm,rtc_pmu error\n");
        return -ENODEV;
    }

    ret |= of_property_read_u32_index(dev_node, "rtc_reg", 0, &g_pmu_rtc.rtc_curr);
    ret |= of_property_read_u32_index(dev_node, "rtc_reg", 1, &g_pmu_rtc.rtc_load);
    ret |= of_property_read_u32_index(dev_node, "rtc_reg", 2, &g_pmu_rtc.rtc_ctrl);
    ret |= of_property_read_u32_index(dev_node, "rst_reg", 0, &g_pmu_rtc.rst_addr);
    ret |= of_property_read_u32_index(dev_node, "rst_reg", 1, &g_pmu_rtc.rst_data);
    ret |= of_property_read_u32_index(dev_node, "clk_reg", 0, &g_pmu_rtc.clk_addr);
    ret |= of_property_read_u32_index(dev_node, "clk_reg", 1, &g_pmu_rtc.clk_data);
    ret |= of_property_read_u32(dev_node, "alarm_num", &g_pmu_rtc.alm_num);
    if(ret)
    {
        rtc_log("of_property_read_u32(_index) error, ret = %d.\n", ret);
        return BSP_ERROR;
    }

    return BSP_OK;
}

void pmu_rtc_prepare(void)
{
    /* 开时钟处理 */
    if(0XFFF != g_pmu_rtc.clk_addr)
    {
        pmu_write_reg(g_pmu_rtc.clk_addr, g_pmu_rtc.clk_data);
    }

    /* 解复位处理 */
    if(0XFFF != g_pmu_rtc.rst_addr)
    {
        pmu_write_reg(g_pmu_rtc.rst_addr, g_pmu_rtc.rst_data);
    }
}

int pmu_rtc_init(void)
{
    int ret = 0;

    g_pmu_rtc.init_flag = 0;

    ret = pmu_get_rtc_dts();
    if(ret)
    {
        rtc_log("pmu_get_rtc_dts error.\n");
        return BSP_ERROR;
    }

    pmu_rtc_prepare();

    g_pmu_rtc.alm_info = (struct pmu_alm_info *)kzalloc(
        g_pmu_rtc.alm_num * sizeof(struct pmu_alm_info), GFP_KERNEL);
    if(!g_pmu_rtc.alm_info)
    {
        rtc_log("kzalloc error.\n");
        return -ENOMEM;
    }

    spin_lock_init(&(g_pmu_rtc.lock));

    if(platform_driver_register(&pmu_rtc_driver))
    {
        kfree(g_pmu_rtc.alm_info);
        return BSP_ERROR;
    }

    g_pmu_rtc.init_flag = 1;

    return BSP_OK;
}

module_init(pmu_rtc_init);

MODULE_AUTHOR("balong modem bsp team");
MODULE_DESCRIPTION("RTC Device Driver on PMU");

/*lint -restore +e19*/
