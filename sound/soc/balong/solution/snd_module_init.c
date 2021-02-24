/*
 *  sound/soc/balong/snd_module_init.c
 *
 *  Copyright (c) 2009 Huawei Technology Co., Ltd
 *  Author: Huawei Technology Co., Ltd
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */
 

#include <linux/module.h>
#include <sound/soc.h>
#include <product_config.h>
#include "snd_init.h"
#include "bsp_version.h"

#define SND_DEV_NUM 5

#if (FEATURE_ON == MBB_FEATURE_AGEING_TEST)
extern void AT_RegisterPCMATTestEnable(void *Handle);
static int PCMATTestEnable(void);
static int snd_run_cmd(const char *pcmd);
#endif

void snd_dev_release(struct device *dev)
{
    return;
}

static const struct platform_driver snd_balong_drv[SND_DEV_NUM]=
{
    [0]={
        .probe  = snd_pcm_dev_probe,
        .remove = snd_pcm_dev_remove,
        .driver = {
            .name  = "pcm_dai",
            .owner = THIS_MODULE,
        }
    },
    [1]={
        .probe  = snd_platform_probe,
        .remove = snd_platform_remove,
        .driver = {
            .name  = "snd_plat",
            .owner = THIS_MODULE,
        }
    },
   [2]={
        .probe  = snd_module_platform_probe,
        .remove = snd_module_platform_remove,
        .driver = {
            .name  = "snd_module_plat",
            .owner = THIS_MODULE,
        }
    },
    [3]={
        .probe  = snd_codec_probe,
        .remove = snd_codec_remove,
        .driver = {
            .name  = "dummy_codec",
            .owner = THIS_MODULE,
        }
    },
    [4]={
        .probe  = snd_module_machine_probe,
        .remove = snd_module_machine_remove,
        .driver = {
            .name  = "module_machine",
            .owner = THIS_MODULE,
        }
    },

};

static const struct platform_device snd_balong_dev[SND_DEV_NUM]=
{
    [0]={
        .name = "pcm_dai",
        .id   = -1,
        .dev  ={
            .release = snd_dev_release,
        }
    },
    [1]={
        .name = "snd_plat",
        .id   = -1,
        .dev  ={
            .release = snd_dev_release,
        }
    },
    [2]={
        .name = "snd_module_plat",
        .id   = -1,
        .dev  ={
            .release = snd_dev_release,
        }
    },
    [3]={
        .name = "dummy_codec",
        .id   = -1,
        .dev  ={
            .release = snd_dev_release,
        }
    },
    [4]={
        .name = "module_machine",
        .id   = -1,
        .dev  ={
            .release = snd_dev_release,
        }
    },
};

#if (FEATURE_ON == MBB_FEATURE_AGEING_TEST)
/*****************************************************************************
 Prototype    : snd_run_cmd
 Description  : 用来执行应用态回环进程
 Input        : 要执行的进程名
 Output       : None
 Return Value :
 Calls        :
*****************************************************************************/

int snd_run_cmd(const char *pcmd)
{
    int  ret = 0;
    char *envp[] = {"HOME=/", "PATH=/app/bin:/system/bin:/sbin:/bin:/usr/sbin:/usr/bin", NULL};
    char **argv = NULL;

    if(NULL == pcmd)
    {
        return -1 ;
    }

    argv = argv_split(0, pcmd, &ret);

    printk(KERN_ERR "ret =%d\n", ret);

    printk(KERN_ERR "argv[0]=%s\n", argv[0]);

    if(NULL == argv)
    {
        return -1;
    }

    if(ret <= 0)
    {
        argv_free(argv);
        printk(KERN_ERR "params = %d.\n", ret);
        return -1;
    }

    ret = call_usermodehelper(argv[0], argv, envp, (int)UMH_WAIT_PROC);
    argv_free(argv);
    printk(KERN_ERR "ret = %d.\n", ret);
    ret |= ret >> 8;  /*返回值用的高8位来表示的*/
    printk(KERN_ERR "ret = %d, run cmd:%s\n", (char)ret, pcmd);

    return ret;
}


static int PCMATTestEnable(void)
{
    int ret = 0;
    ret = snd_run_cmd("/system/bin/pcm_loopback_test");
    if (0 != ret)
    {
        ret = -1;
    }
    return ret;
}
#endif



static struct platform_device snd_dev[SND_DEV_NUM];
static struct platform_driver snd_drv[SND_DEV_NUM];

int snd_balong_init(void)
{
    int ret = 0;
    int i   = 0;
    SOLUTION_PRODUCT_TYPE type = PRODUCT_TYPE_INVALID;

    type = bsp_get_solution_type();
    if (PRODUCT_TYPE_CE == type)
    {
        return 0;
    }
    memcpy(snd_dev, snd_balong_dev, sizeof(snd_balong_dev));
    memcpy(snd_drv, snd_balong_drv, sizeof(snd_balong_drv));

    for(i = 0;i < SND_DEV_NUM;i++)
    {
        ret = platform_device_register(&snd_dev[i]);
        if(ret)
        {
            printk(KERN_ERR "pcm failed to register platform device!\n");
            goto dev_fail;
        }

        ret = platform_driver_register(&snd_drv[i]);
        if(ret)
        {
            printk(KERN_ERR "pcm balong failed to register platform driver!\n");
            goto drv_fail;
        }
    }
#if (FEATURE_ON == MBB_FEATURE_AGEING_TEST)
    AT_RegisterPCMATTestEnable((void *)PCMATTestEnable);
#endif

    printk(KERN_ERR "sound card init ok!\n");

    return 0;

drv_fail:
    platform_device_unregister(&snd_dev[i]);

dev_fail:
    while (--i >= 0)
    {
        platform_driver_unregister(&snd_drv[i]);
        platform_device_unregister(&snd_dev[i]);
    };
    printk(KERN_ERR "sound machine init fail!\n");
    return ret;
}

void snd_balong_exit(void)
{
    int i = SND_DEV_NUM;
    while (--i >= 0)
    {
        platform_driver_unregister(&snd_drv[i]);
        platform_device_unregister(&snd_dev[i]);
    };
    printk(KERN_ERR "sound balong exit ok!\n");
}

module_init(snd_balong_init);
module_exit(snd_balong_exit);


MODULE_AUTHOR("Huawei Technology Co., Ltd");
MODULE_DESCRIPTION("Balong ASoC Driver");
MODULE_LICENSE("GPL");

