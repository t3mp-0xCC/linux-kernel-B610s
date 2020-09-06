/* Copyright (c) 2009-2010, 2013 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
/*
 * Bluetooth Power Switch Module
 * controls power to external Bluetooth device
 * with interface to power management device
    Date         Author           Comment
   -----------  --------------   --------------------------------
   2015-08-28    huawei     modify the driver
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/rfkill.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/bluetooth-power.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include "bsp_pmu.h"

#define BT_PWR_DBG(fmt, arg...)  pr_debug("%s: " fmt "\n" , __func__ , ## arg)
#define BT_PWR_INFO(fmt, arg...) pr_info("%s: " fmt "\n" , __func__ , ## arg)
#define BT_PWR_ERR(fmt, arg...)  pr_err("%s: " fmt "\n" , __func__ , ## arg)
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))


#define  DWH_BT_DEBUG 1
#ifdef DWH_BT_DEBUG
#define bt_printk(fmt, arg...)  printk("%s: " fmt "\n" , __func__ , ## arg)
#define btdb_printk(fmt, arg...)  printk( fmt, ## arg)
#else
#define bt_printk(fmt, arg...)  
#define btdb_printk(fmt, arg...)  
#endif



static struct bluetooth_power_platform_data *bt_power_pdata;
static struct platform_device *btpdev;
static bool previous;

unsigned g_bt_regon_gpio = 0;

static int bt_configure_gpios(int on)
{
    int rc = 0;
    int bt_reset_gpio = bt_power_pdata->bt_gpio_sys_rst;
    BT_PWR_DBG("%s  bt_gpio= %d on: %d", __func__, bt_reset_gpio, on);
    if (on) {
        rc = gpio_request(bt_reset_gpio, "bt_sys_rst_n");
        if (rc) {
            BT_PWR_ERR("unable to request gpio %d (%d)\n",
                    bt_reset_gpio, rc);
            return rc;
        }
        rc = gpio_direction_output(bt_reset_gpio,1);
        gpio_set_value(bt_reset_gpio, 1);
        if (rc) {
            BT_PWR_ERR("Unable to set direction\n");
            return rc;
        }

        msleep(100);

    } else {
        gpio_set_value(bt_reset_gpio, 0);

        msleep(100);
    }
    return rc;
}

static int bluetooth_power(int on)
{
    int rc = 0;

    BT_PWR_DBG("on: %d", on);
    if (on) 
    {

        if (bt_power_pdata->bt_gpio_sys_rst) 
        {
            rc = bt_configure_gpios(on);
            if (rc < 0) 
            {
                BT_PWR_ERR("bt_power gpio config failed");
                goto gpio_fail;
            }           
        }
        else 
        {
            btdb_printk(" bluetooth_power bt_power gpio not set.\n");
        }
    } 
    else 
    {
        bt_configure_gpios(on);
gpio_fail:
        if (bt_power_pdata->bt_gpio_sys_rst)
        {
            gpio_free(bt_power_pdata->bt_gpio_sys_rst);
        }

    }
    btdb_printk(" %s exit.\n", __FUNCTION__);

    return rc;
}

static int bluetooth_toggle_radio(void *data, bool blocked)
{
    int ret = 0;
    int (*power_control)(int enable);
    btdb_printk(" %s enter.\n", __FUNCTION__);
    power_control =
        ((struct bluetooth_power_platform_data *)data)->bt_power_setup;

    if (previous != blocked)
    {
        ret = (*power_control)(!blocked);
    }
    if (!ret)
    {
        previous = blocked;
    }
    btdb_printk(" %s exit.\n", __FUNCTION__);
    return ret;
}

static const struct rfkill_ops bluetooth_power_rfkill_ops = {
    .set_block = bluetooth_toggle_radio,
};

static int bluetooth_power_rfkill_probe(struct platform_device *pdev)
{
    struct rfkill *rfkill;
    int ret;
    btdb_printk(" %s  enter\n",__FUNCTION__);
    rfkill = rfkill_alloc("bt_power", &pdev->dev, RFKILL_TYPE_BLUETOOTH,
                  &bluetooth_power_rfkill_ops,
                  pdev->dev.platform_data);

    if (!rfkill) {
        dev_err(&pdev->dev, "rfkill allocate failed\n");
         btdb_printk("%s: rfkill allocate failed!!!\n",__func__);
        return -ENOMEM;
    }

    /* force Bluetooth off during init to allow for user control */
    rfkill_init_sw_state(rfkill, 1);
    previous = 1;

    ret = rfkill_register(rfkill);
    if (ret) {
        dev_err(&pdev->dev, "rfkill register failed=%d\n", ret);
        btdb_printk("%s: rfkill register failed==%d!!!\n",__func__,ret);
        rfkill_destroy(rfkill);
        return ret;
    }

    platform_set_drvdata(pdev, rfkill);
    btdb_printk(" %s  EXIT\n",__FUNCTION__);
    return 0;
}

static void bluetooth_power_rfkill_remove(struct platform_device *pdev)
{
    struct rfkill *rfkill;

    rfkill = platform_get_drvdata(pdev);
    if (rfkill)
    {
        rfkill_unregister(rfkill);
    }
    rfkill_destroy(rfkill);
    platform_set_drvdata(pdev, NULL);
}


static int bt_power_populate_dt_pinfo(struct platform_device *pdev)
{
    BT_PWR_DBG("");

    btdb_printk(" %s  enter\n",__FUNCTION__);
    if (!bt_power_pdata)
    {
        return -ENOMEM;
    }


    bt_power_pdata->bt_gpio_sys_rst = g_bt_regon_gpio;
            
    if (bt_power_pdata->bt_gpio_sys_rst < 0) {
        BT_PWR_ERR("bt-reset-gpio not provided in device tree");
        btdb_printk(" bt-host_wake-gpio not provided in device tree\n");
        return bt_power_pdata->bt_gpio_sys_rst;
    }
    
    bt_power_pdata->bt_power_setup = bluetooth_power;
    btdb_printk(" %s  EXIT\n",__FUNCTION__);
    return 0;
}

/******************************************************************************
  函数名称  : bluetooth_power_gpio_get
  功能描述  : 获取设备树中的蓝牙上电gpio管脚            
  输入参数  : 
  调用函数  : 
  被调函数  : 
  输出参数  : NA
  返 回 值  :    NA             
  其它说明  ：
******************************************************************************/

static int bluetooth_power_gpio_get(struct platform_device *pdev)
{
    int ret = 0;    
    struct device_node *np = pdev->dev.of_node;
    btdb_printk(" %s  enter\n",__FUNCTION__);

   ret = of_property_read_u32_index(np, "bt_reg_on_gpio", 0,&g_bt_regon_gpio); 
   if(0 != ret)
    {
        btdb_printk("find bluetooth bt_reg_on_gpio fail\n");
        return -1;
    }  
   
    return 0;
}

static int  bt_power_probe(struct platform_device *pdev)
{
    int ret = 0;
    btdb_printk(" %s  enter\n",__FUNCTION__);

    /*使能bt clk*/
    ret = bsp_pmu_32k_clk_enable(PMU_32K_CLK_A);
    if (ret)
    {
        btdb_printk( "enable bt 32k clock failed!\n");
        return ret;
    }
   /*获取bt regon gpio值*/
    ret = bluetooth_power_gpio_get(pdev);
    if(ret)
    {
        btdb_printk("bluetooth_power_gpio_get fail!\n");
        return ret;
    }

    bt_power_pdata =
        kzalloc(sizeof(struct bluetooth_power_platform_data),
            GFP_KERNEL);

    if (!bt_power_pdata) {
        BT_PWR_ERR("Failed to allocate memory");
        return -ENOMEM;
    }


    ret = bt_power_populate_dt_pinfo(pdev);
    if (ret < 0) {
        BT_PWR_ERR("Failed to populate device tree info");
        goto free_pdata;
    }
    pdev->dev.platform_data = bt_power_pdata;   

    if (bluetooth_power_rfkill_probe(pdev) < 0)
    {
        goto free_pdata;
    }

    btpdev = pdev;
    btdb_printk(" %s  exit\n",__FUNCTION__);
    return 0;

free_pdata:
    kfree(bt_power_pdata);
    bsp_pmu_32k_clk_disable(PMU_32K_CLK_A);
    return ret;
}

static int  bt_power_remove(struct platform_device *pdev)
{

    bluetooth_power_rfkill_remove(pdev);

    kfree(bt_power_pdata);

    return 0;
}
static const struct of_device_id of_gpio_bluepower_match[] = {
    { .compatible = "bluepower", },
    {},
};


static struct platform_driver bt_power_driver = {
    .probe = bt_power_probe,
    .remove = bt_power_remove,
    .driver = {
        .name = "bt_power",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(of_gpio_bluepower_match),
    },
};



MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION(" Bluetooth power control driver");
MODULE_VERSION("1.40");


module_platform_driver(bt_power_driver);

