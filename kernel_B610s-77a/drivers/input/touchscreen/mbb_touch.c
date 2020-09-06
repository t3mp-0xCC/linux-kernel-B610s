/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <product_config.h>
#include <linux/module.h>
#include <linux/init.h>
#include "bsp_sram.h"
#include "mbb_touch.h"

static int __init mbb_touch_driver_init(void)
{
    int ret = -1;
    huawei_smem_info *smem_data = NULL;

    /* 升级模式不启动touch驱动 */
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;

    if (NULL == smem_data)
    {
        printk(KERN_ERR "nandc_mtd_dload_proc_deal:get smem_data error\n");
   
        return 0;
    }
   
    if(smem_data->smem_dload_flag == SMEM_DLOAD_FLAG_NUM)
    {
         printk(KERN_ERR "Need not start up TP in update process\n");
        return 0;
    }

    /* 依次尝试启动各厂家驱动，如果成功则不继续尝试，
       注: 各厂家驱动中，若初始化失败需释放已申请的资源，确保不影响其他厂家 */
#if (FEATURE_ON == MBB_TOUCHSCREEN_MSTAR)
    ret = touch_mstar_init();
    if( 0 == ret )
    {
        printk(KERN_ERR "Mstar TP init success!\n");
        return ret;
    }
#endif
#if (FEATURE_ON == MBB_TOUCHSCREEN_FOCAL)
    ret = touch_fts_init();
    if( 0 == ret )
    {
        printk(KERN_ERR "Focal  TP init success!\n");
        return ret;
    }
#endif
#if (FEATURE_ON == MBB_TOUCHSCREEN_MELFAS)
    ret = touch_melfas_init();
    if( 0 == ret )
    {
        printk(KERN_ERR "Melfas TP init success!\n");
        return ret;
    }
#endif

    return ret;
}

static void __exit mbb_touch_driver_exit(void)
{
#if (FEATURE_ON == MBB_TOUCHSCREEN_MSTAR)
    touch_mstar_exit();
#endif
#if (FEATURE_ON == MBB_TOUCHSCREEN_FOCAL)
    touch_fts_exit();
#endif
#if (FEATURE_ON == MBB_TOUCHSCREEN_MELFAS)
    touch_melfas_exit();
#endif
    printk(KERN_ERR "exit mbb touch driver.\n");
}

module_init(mbb_touch_driver_init);
module_exit(mbb_touch_driver_exit);
MODULE_AUTHOR("MBB.Huawei Device");
MODULE_DESCRIPTION("Mbb Touch Driver");
MODULE_LICENSE("GPL");

