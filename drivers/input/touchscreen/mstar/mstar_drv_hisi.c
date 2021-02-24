////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2014 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (??MStar Confidential Information??) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

/**
 *
 * @file    mstar_drv_hisi.c
 *
 * @brief   This file defines the interface of touch screen
 *
 *
 */
 
/*=============================================================*/
// INCLUDE FILE
/*=============================================================*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/kobject.h>
#include <asm/irq.h>
#include <asm/io.h>
#include "bsp_sram.h"
#include "mstar_drv_ic_fw_porting_layer.h"
#include "mstar_drv_platform_interface.h"
#include "mstar_drv_platform_porting_layer.h"
#ifdef CONFIG_ENABLE_REGULATOR_POWER_ON
#include <linux/regulator/consumer.h>
#endif //CONFIG_ENABLE_REGULATOR_POWER_ON

/*=============================================================*/
// CONSTANT VALUE DEFINITION
/*=============================================================*/

#define MSG_TP_IC_NAME "msg22s" //"msg21xxA" or "msg22xx" or "msg26xxM" /* Please define the mstar touch ic name based on the mutual-capacitive ic or self capacitive ic that you are using */

#define mstar_platform_data_length (3)
#define mstar_ldo_en_index (0)
#define mstar_int_en_index (1)
#define mstar_rst_en_index (2)

static int mstar_init_status = -1;

int MS_TS_MSG_IC_GPIO_RST = -1;
int MS_TS_MSG_IC_GPIO_INT = -1;
int MS_TS_MSG_IC_GPIO_PWC = -1;
/*=============================================================*/
// VARIABLE DEFINITION
/*=============================================================*/

struct i2c_client *g_I2cClient = NULL;

#ifdef CONFIG_ENABLE_REGULATOR_POWER_ON
struct regulator *g_ReguVdd = NULL;
#endif //CONFIG_ENABLE_REGULATOR_POWER_ON

/*=============================================================*/
// FUNCTION DEFINITION
/*=============================================================*/
/* probe function is used for matching and initializing input device */
static int __init touch_driver_probe(struct i2c_client *client,
        const struct i2c_device_id *id)
{
    int ret = -1;
    unsigned int reg_addr[mstar_platform_data_length] = {0};
    struct device_node *node = client->dev.of_node;
#ifdef CONFIG_ENABLE_REGULATOR_POWER_ON
    const char *vdd_name = "vdd";
#endif //CONFIG_ENABLE_REGULATOR_POWER_ON

    printk(KERN_ERR "***[mstar] %s ***\n", __FUNCTION__);

    if (client == NULL)
    {
        printk(KERN_ERR "[mstar]i2c client is NULL\n");
        return -1;
    }

    g_I2cClient = client;

    if(NULL == node )
    {
        printk(KERN_ERR "[mstar][%s:%d] device tree information missing\n", __func__,__LINE__);
        return - ENODEV;
    }
  
    ret = of_property_read_u32_array(node, "gpios", reg_addr, mstar_platform_data_length); 
    if(ret)
    {
        printk(KERN_ERR "[mstar][%s:%d] device tree information value get fail\n", __func__,__LINE__);
        return - ENODEV;
    }
    MS_TS_MSG_IC_GPIO_PWC   = reg_addr[mstar_ldo_en_index];
    MS_TS_MSG_IC_GPIO_INT   = reg_addr[mstar_int_en_index];
    MS_TS_MSG_IC_GPIO_RST   = reg_addr[mstar_rst_en_index];
  
#ifdef CONFIG_ENABLE_REGULATOR_POWER_ON
    g_ReguVdd = regulator_get(&g_I2cClient->dev, vdd_name);
#endif //CONFIG_ENABLE_REGULATOR_POWER_ON
    mstar_init_status = MsDrvInterfaceTouchDeviceProbe(g_I2cClient, id);
    return mstar_init_status;
}

/* remove function is triggered when the input device is removed from input sub-system */
static int touch_driver_remove(struct i2c_client *client)
{
    printk(KERN_ERR "[mstar]*** %s ***\n", __FUNCTION__);

    return MsDrvInterfaceTouchDeviceRemove(client);
}

/* The I2C device list is used for matching I2C device and I2C device driver. */
static const struct i2c_device_id touch_device_id[] =
{
    {MSG_TP_IC_NAME, 0},
    {}, /* should not omitted */ 
};

static struct of_device_id touch_match_table[] = {
    { .compatible = "mstar,msg22s",},
    {},
};
MODULE_DEVICE_TABLE(i2c, touch_device_id);

static struct i2c_driver touch_device_driver =
{
    .driver = {
        .name = MSG_TP_IC_NAME,
        .owner = THIS_MODULE,
        .of_match_table = touch_match_table,
    },
    .probe = touch_driver_probe,
    .remove = touch_driver_remove,
    .id_table = touch_device_id,
};

static int __init touch_driver_init(void)
{
    int ret;
    huawei_smem_info *smem_data = NULL;

    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;

    if (NULL == smem_data)
    {
        printk(KERN_ERR "[mstar] nandc_mtd_dload_proc_deal:get smem_data error\n");
   
        return 0;
    }
   
    if(smem_data->smem_dload_flag == SMEM_DLOAD_FLAG_NUM)
    {
         printk(KERN_ERR "[mstar] Need not start up TP in update process\n");
        return 0;
    }
    /* register driver */
    ret = i2c_add_driver(&touch_device_driver);
    if (ret < 0)
    {
        printk(KERN_ERR "[mstar] add touch device driver i2c driver failed.\n");
        return -ENODEV;
    }

    return ret;
}

static void __exit touch_driver_exit(void)
{
    printk(KERN_ERR "[mstar] remove touch device driver i2c driver.\n");

    i2c_del_driver(&touch_device_driver);
}

int touch_mstar_init(void)
{
    int ret = -1;
    ret = touch_driver_init();
    if( 0 > ret )
    {
        printk(KERN_ERR "[mstar] Mstar TP touch_driver_init fail!\n");
        return ret;
    }

    ret = mstar_init_status;
    return ret;
}

void touch_mstar_exit(void)
{
    mstar_init_status = -1;
    touch_driver_exit();
}

/*
module_init(touch_driver_init);
module_exit(touch_driver_exit);
*/
MODULE_LICENSE("GPL");

