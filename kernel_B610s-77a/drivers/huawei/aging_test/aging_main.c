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


#include "aging_main.h"
#include "aging_rf_test.h"
#include "aging_led_test.h"
#include "aging_wlan_test.h"
#include "bsp_sram.h"

static int is_aging_module_inited = AGING_DRV_STATUS_ERROR; /* 模块初始化状态 */
static NV_AGING_TEST_CTRL_STRU aging_test_nv;  /* NV中读取的值 */
static struct task_struct *aging_main_test_thread = NULL;
static struct aging_test_config aging_config[] = 
{
    {NULL, "aging_led_test", aging_led_test_thread, aging_led_test_init, aging_led_test_exit}, /*LED*/
    {NULL, "aging_rf_test", aging_rf_test_thread, aging_rf_test_init, aging_rf_test_exit}, /*RF*/
    {NULL, "aging_wlan_test", aging_wlan_test_thread, aging_wlan_test_init, aging_wlan_test_exit},  /*WIFI*/
};

/********************************************************
*函数名   : is_in_update_mode_aging
*函数功能 : 获取当前是否是升级模式
*输入参数 : 无
*输出参数 : 无
*返回值   : TURE:是升级模式
            FALSE:不是升级模式
*修改历史 :
********************************************************/
int is_in_update_mode_aging(void)
{
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;

    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/********************************************************
*函数名   : aging_test_config_get
*函数功能 : 对外接口，查询老化测试各模块是否需要使能，在NV配置
*输入参数 : AGING_TEST_MODULE_E aging_test_module
*输出参数 : 无
*返回值   : 返回1表示使能，0表示不使能, 驱动没准备好返回-1
*修改历史 :
*           2015-7-21 徐超 初版作成
********************************************************/
int aging_test_config_get(AGING_TEST_MODULE_E aging_test_module)
{
    int ret = -1;
    if(AGING_DRV_STATUS_OK != is_aging_module_inited)
    {
        /* 驱动没初始化完毕，返回错误状态，由调用模块酌情处理 */
        return AGING_DRV_STATUS_ERROR;
    }
    
    aging_print_info("aging test enable status is %d.\n", aging_test_nv.aging_test_enable);

    switch (aging_test_module)
    {
        case CHARGE_TEST_MODULE:
            aging_print_info("charge_test_enable is %d.\n", aging_test_nv.charge_test_enable);
            ret = aging_test_nv.charge_test_enable;
            break;
        case LED_TEST_MODULE:
            aging_print_info("led_test_enable is %d.\n",  aging_test_nv.led_parameter.led_test_enable);
            ret = aging_test_nv.led_parameter.led_test_enable;
            break;
        case LCD_TEST_MODULE:
            aging_print_info("lcd_test_enable is %d.\n", aging_test_nv.lcd_test_enable);
            ret = aging_test_nv.lcd_test_enable;
            break;
        case WLAN_TEST_MODULE:
            aging_print_info("wifi_test_enable is %d.\n",  aging_test_nv.wifi_parameter.wifi_test_enable);
            ret = aging_test_nv.wifi_parameter.wifi_test_enable;
            break;
        case RF_TEST_MODULE:
            aging_print_error("rf_test_enable is %d.\n", aging_test_nv.rf_test.rf_test_enable);
            ret =  aging_test_nv.rf_test.rf_test_enable;
            break;
        case FLASH_TEST_MODULE:
            aging_print_info("flash_test_enable is %d.\n", aging_test_nv.flash_test_enable);
            ret = aging_test_nv.flash_test_enable;
            break;
        case CPU_TEST_MODULE:
            aging_print_info("cpu_test_enable is %d.\n", aging_test_nv.cpu_test_enable);
            ret = aging_test_nv.cpu_test_enable;
            break;
        default:
            aging_print_error("Error! aging_test_module is %d.\n", aging_test_module);
            break;
    }
    return ret;
}

/********************************************************
*函数名   : to_aging_work
*函数功能 : 返回aging_work_struct结构指针
              为以后扩展,此函数不加static声明
*输入参数 : struct work_struct *work -- work结构指针
*输出参数 : 无
*返回值   : aging_work_struct结构指针/NULL
*修改历史 :
********************************************************/
aging_work_struct *to_aging_work(struct work_struct *work)
{
    struct delayed_work *dwork = NULL;
    if(NULL == work)
    {
        aging_print_error("Input para error.\n");
        return NULL;
    }

    dwork = to_delayed_work(work);

    return (NULL == dwork)?NULL:(container_of(dwork, aging_work_struct, d_work));
}


static void aging_test_stop_status(aging_test_status status)
{
    /*当前仅led状态提示,后续可扩展增加*/
    aging_led_test_stop(status);
}


static int aging_test_thread_init(struct platform_device *pdev)
{
    int ret = -1;
    int i = 0;

    if(NULL == pdev)
    {
        aging_print_error("dev is NULL !!\n");
        return -EINVAL;
    }

    /*依次初始化各测试线程*/
    for(i = 0; i< ARRAYSIZE(aging_config); i++)
    {
        /*初始化各测试模块*/
        if(NULL != aging_config[i].init_func)
        {
            ret = aging_config[i].init_func(pdev);
            if(0 != ret)
            {
                aging_print_error("init index %d failed!\n", i);
                return -1;
            }   
            else if(NULL == aging_config[i].test_thread)
            {
                /*init成功后启动测试线程*/
                aging_config[i].test_thread = kthread_run(aging_config[i].thread_func, 
                    pdev, aging_config[i].test_thread_name);
                if(IS_ERR(aging_config[i].test_thread))
                {
                    aging_print_error("%s kthread_run error!\n", aging_config[i].test_thread_name);
                    return -1;
                }
            }
        }
        else
        {
            aging_print_error("init func not exist, index=%d!\n", i);
            return -1;
        }
    }

    return 0;
}


static int aging_test_thread_exit(struct platform_device *pdev)
{
    int ret = -1;
    int i = 0;

    if(NULL == pdev)
    {
        aging_print_error("dev is NULL !!\n");
        return -EINVAL;
    }

    /*调用各模块退出函数处理*/
    for(i = 0; i < ARRAYSIZE(aging_config); i++)
    {
        if(NULL != aging_config[i].exit_func)
        {
            ret = aging_config[i].exit_func(pdev);
            if(0 != ret)
            {
                aging_print_error("exit index %d failed!\n",i);
            }
        }
    }

    return 0;
}

/********************************************************
*函数名   : aging_main_thread_func
*函数功能 : 老化主线程初始化函数
*输入参数 : struct platform_device *pdev
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
*           2015-7-21 徐超 初版作成
********************************************************/
static int aging_main_thread_func(struct platform_device *pdev)
{
    int ret = -1;
    int i = 0;
    unsigned short desc_time = 0; /*递减时间*/
    struct aging_test_priv *priv = NULL;

    if(NULL == pdev)
    {
        aging_print_error("dev is NULL !!\n");
        return -EINVAL;
    }
    
    priv = platform_get_drvdata(pdev);
    if(NULL == priv)
    {
        aging_print_error("priv is NULL !!\n");
        return -EINVAL;
    }

    if((AGING_ENABLE != aging_test_nv.aging_test_enable)
        || (AGING_DISABLE == aging_test_nv.total_time))
    {
        aging_print_error("aging test disabled in nv!!\n");
        return 0;
    }

    /*初始化各测试线程*/
    ret = aging_test_thread_init(pdev);
    if(0 != ret)
    {
        aging_print_error("aging test thread init failed!\n");
        goto EXIT0;
    }

    /*主线程循环唤醒等待结束*/
    while(0 < aging_test_nv.total_time)
    {
        /*先判定各模块执行情况*/
        if(0 != priv->aging_total_status)
        {
            aging_print_error("some tests failed, 0x%x!\n", priv->aging_total_status);
            goto EXIT1;
        }
        /*定时唤醒后更新nv时间*/
        desc_time = (aging_test_nv.total_time > AGING_MAIN_WAIT_TIME) ? \
                    AGING_MAIN_WAIT_TIME : aging_test_nv.total_time;
        msleep(desc_time * AGING_MIN_TO_MSEC);
        /*等待唤醒后刷新nv值*/
        aging_test_nv.total_time -= desc_time;
        ret = (unsigned int)bsp_nvm_write(NV_ID_HUAWEI_AGING_TEST,
                    (u8*)&(aging_test_nv), sizeof(NV_AGING_TEST_CTRL_STRU));
        if(0 != ret)
        {
            aging_print_error("nv write failed!\n");
            goto EXIT1;
        }
    }


    /*成功完成测试*/
    aging_test_stop_status(AGING_TEST_PASS);
    (void)aging_test_thread_exit(pdev);
    return 0;

EXIT1:
    /*测试执行失败需要处理状态*/
    aging_test_stop_status(AGING_TEST_FAIL);
EXIT0:
    /*各线程退出*/
    (void)aging_test_thread_exit(pdev);
    return -1;
}


/********************************************************
*函数名   : aging_test_probe
*函数功能 : 老化驱动初始化函数
*输入参数 : struct platform_device *pdev
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
*           2015-7-21 徐超 初版作成
********************************************************/
static int aging_test_probe(struct platform_device *pdev)
{
    int ret = -1;
    struct aging_test_priv *priv = NULL;
    /*升级模式不起老化任务*/
    if( TRUE == is_in_update_mode_aging() )
    {
        return ret;
    }

    priv = devm_kzalloc(&pdev->dev, sizeof(struct aging_test_priv),
            GFP_KERNEL);
    if (NULL == priv || IS_ERR(priv))
    {
        aging_print_error("devm_kzalloc error!\n");
        return ERR_PTR(-ENOMEM);
    }
    
    ret = (ssize_t)bsp_nvm_read(NV_ID_HUAWEI_AGING_TEST, (u8*)&(aging_test_nv), sizeof(NV_AGING_TEST_CTRL_STRU));
    if(ret)
    {
        /* NV读取失败时本模块不启动，直接退出 */
        aging_print_error("nv read err!!\n");
        devm_kfree(&pdev->dev, priv);
        return ret;
    }

    memcpy(&(priv->aging_test_nv), &aging_test_nv, sizeof(NV_AGING_TEST_CTRL_STRU));
    priv->aging_total_status = 0; /*初始化状态均为PASS*/
    platform_set_drvdata(pdev, priv);

    /*启动main线程初始化其他测试项*/
    aging_main_test_thread = kthread_run(aging_main_thread_func, pdev, AGING_MAIN_THREAD_NAME);
    if(IS_ERR(aging_main_test_thread))
    {
        aging_print_error("%s kthread_run error!\n", AGING_MAIN_THREAD_NAME);
        devm_kfree(&pdev->dev, priv);
        return -1;
    }

    is_aging_module_inited = AGING_DRV_STATUS_OK;
    aging_print_info("aging_test_probe OK!\n");
    return 0;
}

static int aging_test_remove(struct platform_device *pdev)
{
    platform_set_drvdata(pdev, NULL);
    return 0;
}

static struct platform_driver aging_test_drv = {
    .probe = aging_test_probe,
    .remove = aging_test_remove,
    .driver = {
        .name  = "aging_test",
        .owner = THIS_MODULE, 
    },
};

static struct platform_device aging_test_dev = {
    .name           = "aging_test",
};

static int __init aging_test_init(void)
{
    int ret = 0;
    
    ret = platform_device_register(&aging_test_dev);
    if (ret < 0)
    {
        aging_print_error("platform_device_register [mbb aging test Module] failed.\r\n");
        return ret;
    }
    
    ret = platform_driver_register(&aging_test_drv);
    if (0 > ret)
    {
        platform_device_unregister(&aging_test_dev);
        aging_print_error("platform_driver_register [mbb aging test Module] failed.\r\n");
        return ret;
    }
    return ret;
}

static void __exit aging_test_exit(void)
{
    platform_driver_unregister(&aging_test_drv);

    platform_device_unregister(&aging_test_dev);
}

module_init(aging_test_init);
module_exit(aging_test_exit);

MODULE_AUTHOR("MBB.Huawei Device");
MODULE_DESCRIPTION("Aging Test Driver");
MODULE_LICENSE("GPL");

