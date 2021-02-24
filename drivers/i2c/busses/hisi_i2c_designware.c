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

/*lint --e{124,530,537,550,718,732,737,746,830} */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/clk.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/of_i2c.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_address.h>
#include <bsp_i2c.h>
#include "i2c-designware-core.h"

#define DW_IC_COMP_PARAM_1    0xf4

#define  i2c_error(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_I2C, "[I2C]: <%s> "fmt"\n", __FUNCTION__, ##__VA_ARGS__))


static struct i2c_algorithm hs_i2c_dw_algo = {
    .master_xfer    = i2c_dw_xfer,
    .functionality    = i2c_dw_func,
};

struct dw_i2c_dev *g_i2c_d[2];

static u32 hs_i2c_dw_get_clk_rate_khz(struct dw_i2c_dev *dev)
{
    return clk_get_rate(dev->clk)/1000;
}
#if 0
static void hs_i2c_dw_reset_controller(struct dw_i2c_dev *dev)
{
    #if 0
    struct hs_i2c_priv_data *priv = dev->priv;
    u32 val = 0, timeout = 10;

    writel(BIT(priv->reset_bit), peri_cfg + priv->reset_enable_off);
    do {
        val = readl(peri_cfg + priv->reset_status_off);
        val &= BIT(priv->reset_bit);
        udelay(1);
    } while (!val && timeout--);

    timeout = 10;

    writel(BIT(priv->reset_bit), peri_cfg + priv->reset_disable_off);
    do {
        val = readl(peri_cfg + priv->reset_status_off);
        val &= BIT(priv->reset_bit);
        udelay(1);
    } while (!val && timeout--);
    printk("hs_i2c_dw_reset_controller is null\n");
    #endif
    return;
}
#endif

int hs_dw_i2c_set_speed(enum i2c_master i2c,int speed)
{
    int ret = 0;
    ret = i2c_dw_set_speed(g_i2c_d[i2c], speed);
    if(ret)
    {
        i2c_error("set speed failed.\n");
    }
    return ret;
}
static int hs_dw_i2c_probe(struct platform_device *pdev)
{
    struct dw_i2c_dev *dw_i2c_dev;
    struct i2c_adapter *adap;
    struct resource *iores;
    const char *i2c_clk_array[2]={"i2c0_clk","i2c1_clk"};
    int r;
    static int id = 0;

    dw_i2c_dev = devm_kzalloc(&pdev->dev, sizeof(struct dw_i2c_dev), GFP_KERNEL);
    if (!dw_i2c_dev) {
        dev_err(&pdev->dev, "mem alloc failed for dw_i2c_dev data\n");
        return -ENOMEM;
    }

    g_i2c_d[id] = dw_i2c_dev;

    /* NOTE: driver uses the static register mapping */
    iores = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!iores)
        return -EINVAL;

    dw_i2c_dev->base = devm_request_and_ioremap(&pdev->dev, iores);
    if (!dw_i2c_dev->base)
        return -EADDRNOTAVAIL;

    //d->platform_dev = &pdev->dev;
    r = of_property_read_u32(pdev->dev.of_node, "id", &id);
    if(r) {
        i2c_error("failed to get id!\n");
        return  -1;
    }

    dw_i2c_dev->clk = clk_get(NULL, i2c_clk_array[id]);
    if (IS_ERR(dw_i2c_dev->clk))
        return -ENODEV;

    dw_i2c_dev->get_clk_rate_khz = hs_i2c_dw_get_clk_rate_khz;

    r = clk_prepare_enable(dw_i2c_dev->clk);
    if(r) {
        dev_warn(&pdev->dev,"Unable to enable clock!\n");
        return  -EINVAL;
    }

    init_completion(&dw_i2c_dev->cmd_complete);
    mutex_init(&dw_i2c_dev->lock);
    dw_i2c_dev->dev = get_device(&pdev->dev);

    dw_i2c_dev->functionality =
        I2C_FUNC_I2C |
        I2C_FUNC_SMBUS_BYTE |
        I2C_FUNC_SMBUS_BYTE_DATA |
        I2C_FUNC_SMBUS_WORD_DATA |
        I2C_FUNC_SMBUS_I2C_BLOCK;

    dw_i2c_dev->master_cfg =  DW_IC_CON_MASTER | DW_IC_CON_SLAVE_DISABLE |
                     DW_IC_CON_RESTART_EN | DW_IC_CON_SPEED_FAST;


    {
        u32 param1 = readl(dw_i2c_dev->base + DW_IC_COMP_PARAM_1);

        dw_i2c_dev->tx_fifo_depth = ((param1 >> 16) & 0xff) + 1;
        dw_i2c_dev->rx_fifo_depth = ((param1 >> 8)  & 0xff) + 1;
        dev_info(&pdev->dev, "tx_fifo_depth: %d, rx_fifo_depth: %d\n",
                 dw_i2c_dev->tx_fifo_depth, dw_i2c_dev->rx_fifo_depth);
    }

    r = i2c_dw_init(dw_i2c_dev);
    if (r)
        goto err;

    i2c_dw_disable_int(dw_i2c_dev);

    dw_i2c_dev->irq = platform_get_irq(pdev, 0);
    if (dw_i2c_dev->irq < 0) {
        dev_err(&pdev->dev, "no irq resource?\n");
        return dw_i2c_dev->irq; /* -ENXIO */
    }

    r = devm_request_irq(&pdev->dev, dw_i2c_dev->irq,
                         i2c_dw_isr, IRQF_DISABLED, pdev->name, dw_i2c_dev);
    if (r) {
        dev_err(&pdev->dev, "failure requesting irq %i\n", dw_i2c_dev->irq);
        return -EINVAL;
    }

    adap = &dw_i2c_dev->adapter;
    i2c_set_adapdata(adap, dw_i2c_dev);
    adap->owner = THIS_MODULE;
    adap->class = I2C_CLASS_HWMON;
    strlcpy(adap->name, "Synopsys DesignWare I2C adapter",
            sizeof(adap->name));
    adap->algo = &hs_i2c_dw_algo;
    adap->dev.parent = &pdev->dev;
    adap->dev.of_node = pdev->dev.of_node;

    adap->nr = id;
    r = i2c_add_numbered_adapter(adap);
    if (r) {
        dev_err(&pdev->dev, "failure adding adapter\n");
        goto err;
    }
    of_i2c_register_devices(adap);
    platform_set_drvdata(pdev, dw_i2c_dev);

    clk_disable_unprepare(dw_i2c_dev->clk);
    id++;
    return 0;

err:
    clk_disable_unprepare(dw_i2c_dev->clk);
    //devm_clk_put(&pdev->dev, d->clk);
    dw_i2c_dev->clk = NULL;
    put_device(&pdev->dev);
    return r;
}

static int hs_dw_i2c_remove(struct platform_device *pdev)
{
    struct dw_i2c_dev *dw_i2c_dev = NULL;

    dw_i2c_dev = (struct dw_i2c_dev *)platform_get_drvdata(pdev);
    if(dw_i2c_dev == NULL)
    {
        i2c_error("failed to get drvdata.\n");
        return -1;
    }
    platform_set_drvdata(pdev, NULL);
    if(&dw_i2c_dev->adapter)
    {
        i2c_del_adapter(&dw_i2c_dev->adapter);
    }
    put_device(&pdev->dev);
    clk_disable_unprepare(dw_i2c_dev->clk);
    clk_put(dw_i2c_dev->clk);
    dw_i2c_dev->clk = NULL;
    i2c_dw_disable(dw_i2c_dev);

    return 0;
}

#ifdef CONFIG_PM
static int hs_dw_i2c_suspend(struct device *dev)
{
    struct platform_device *pdev = NULL;
    struct dw_i2c_dev *i_dev = NULL;

    pdev = to_platform_device(dev);
    if(NULL == pdev)
    {
        i2c_error("to platform device failed.\n");
        return -1;
    }

    dev_info(&pdev->dev, "%s: suspend +\n", __func__);
    i_dev = platform_get_drvdata(pdev);
    if(NULL == i_dev)
    {
        i2c_error("platform get drvdata failed.\n");
        return -1;
    }
    if (!mutex_trylock(&i_dev->lock)) {
        dev_info(&pdev->dev, "%s: mutex_trylock.\n", __func__);
        return -EAGAIN;
    }

    dev_info(&pdev->dev, "%s: suspend -\n", __func__);
    return 0;
}

static int hs_dw_i2c_resume(struct device *dev)
{
    struct platform_device *pdev = NULL;
    struct dw_i2c_dev *i_dev = NULL;
    int ret = 0;

    pdev = to_platform_device(dev);
    if(pdev == NULL)
    {
        i2c_error("failed to get platform device.\n");
        return -1;
    }
    i_dev = (struct dw_i2c_dev *)platform_get_drvdata(pdev);
    if(i_dev == NULL)
    {
        i2c_error("failed to get drvdata.\n");
        return -1;
    }
    dev_info(&pdev->dev, "%s: resume +\n", __func__);

    ret = clk_prepare_enable(i_dev->clk);
    if(ret) {
        dev_err(&pdev->dev, "clk_prepare_enable failed!\n");
        return -EAGAIN;
    }
    //hs_i2c_dw_reset_controller(i_dev);
    i2c_dw_init(i_dev);
    i2c_dw_disable_int(i_dev);
    clk_disable_unprepare(i_dev->clk);

    mutex_unlock(&i_dev->lock);

    dev_info(&pdev->dev, "%s: resume -\n", __func__);
    return 0;
}
static void hs_dw_i2c_complete(struct device *dev)
{
    return ;
}
static int hs_dw_i2c_prepare(struct device *dev)
{
    return 0;
}
static const struct dev_pm_ops hs_dw_i2c_dev_pm_ops ={
    .prepare = hs_dw_i2c_prepare,
    .suspend_noirq = hs_dw_i2c_suspend,
    .resume_noirq = hs_dw_i2c_resume,
    .complete = hs_dw_i2c_complete
};

#define BALONG_DEV_PM_OPS (&hs_dw_i2c_dev_pm_ops)
#endif



#if 0
static struct resource designware_i2c0_resource[]={
    [0]={
    .start=HI_I2C0_REGBASE_ADDR,
    .end = HI_I2C0_REGBASE_ADDR+HI_I2C0_REG_SIZE-32,
    .flags=IORESOURCE_MEM,
        },
    [1]={
    .start=INT_LVL_I2C0,/*�жϺ�*/
    .end=INT_LVL_I2C0,
    .flags=IORESOURCE_IRQ,
    }
};

static struct resource designware_i2c1_resource[]={
    [0]={
    .start=HI_I2C1_REGBASE_ADDR,
    .end =HI_I2C1_REGBASE_ADDR+HI_I2C1_REG_SIZE,
    .flags=IORESOURCE_MEM,
    },
    [1]={
    .start=INT_LVL_I2C1,
    .end=INT_LVL_I2C1,
    .flags=IORESOURCE_IRQ,
    }
};
struct platform_device designware_device_i2c0={
    .name="designware_i2c0",
    .id = 0,
    .num_resources=ARRAY_SIZE(designware_i2c0_resource),
    .resource=designware_i2c0_resource,
};

struct platform_device designware_device_i2c1={
    .name="designware_i2c1",
    .id = 1,
    .num_resources=ARRAY_SIZE(designware_i2c1_resource),
    .resource=designware_i2c1_resource,
};
#endif
#ifdef CONFIG_OF
static const struct of_device_id hs_dw_i2c_of_match[] = {
    { .compatible = "hisilicon,designware-i2c", },
    {},
};
MODULE_DEVICE_TABLE(of, hs_dw_i2c_of_match);
#endif

static struct platform_driver designware_i2c_driver = {
    .probe        = hs_dw_i2c_probe,
    .remove        = hs_dw_i2c_remove,
    .driver        = {
        .owner    = THIS_MODULE,
        .name    = "designware_i2c",
        .of_match_table = of_match_ptr(hs_dw_i2c_of_match),
#ifdef CONFIG_PM
        .pm        = BALONG_DEV_PM_OPS
#endif
    },
};
#if 0

#endif
static int __init i2c_adapter_designware_init(void)
{
    int ret=0;
    ret= platform_driver_register(&designware_i2c_driver);
    return ret;
}
arch_initcall(i2c_adapter_designware_init);
MODULE_DESCRIPTION("HS Synopsys DesignWare I2C bus adapter");
MODULE_ALIAS("platform:i2c_designware-hs");
MODULE_LICENSE("GPL");
