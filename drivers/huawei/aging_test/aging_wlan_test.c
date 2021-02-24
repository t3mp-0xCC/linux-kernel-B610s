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


#include "aging_wlan_test.h"
#include "wlan_at.h"
#include "wlan_at_api.h"
#include "wlan_utils.h"

#define AGING_WLAN_2G_TEST  0
#define AGING_WLAN_5G_TEST  1

#define ANTENNA_1 0
#define ANTENNA_2 1

#define AGING_WLAN_2G_ANTENNA1   0
#define AGING_WLAN_2G_ANTENNA2   1
#define AGING_WLAN_5G_ANTENNA1   2
#define AGING_WLAN_5G_ANTENNA2   3

#define WLAN_AGING_WAIT_SYSTEM_SETUP_TIME 15000

#define WLAN_AGING_TEST_MINUTES     6
#define WLAN_AGING_ALL_TEST_MINUTES 60

#define WLAN_AGING_TEST_CIRLE 6

typedef enum
{
    WLAN_2G_11A = 0,
    WLAN_2G_11B,
    WLAN_2G_11G,
    WLAN_2G_11N,
    WLAN_2G_11AC,
    WLAN_5G_11A,
    WLAN_5G_11B,
    WLAN_5G_11G,
    WLAN_5G_11N,
    WLAN_5G_11AC,
    WLAN_MODE_MAX
}WIFI_CATMODE_IDX;

static unsigned char g_atcmd_tselrf[AGING_ATCMD_LEN] = {0};
static unsigned char g_atcmd_wimode[AGING_ATCMD_LEN] = {0};
static unsigned char g_atcmd_freq[AGING_ATCMD_LEN] = {0};

static aging_atcmd_info atcmd_2G_info[] =
{
    {AG_WLAN_ATCMD_TSELRF,   g_atcmd_tselrf},
    {AG_WLAN_ATCMD_MODE,     g_atcmd_wimode},
    {AG_WLAN_ATCMD_BAND,     "AT^WIBAND=1\r\n"},
    {AG_WLAN_ATCMD_FREQ,     g_atcmd_freq},
    {AG_WLAN_ATCMD_DATARATE, "AT^WIDATARATE=2700\r\n"},
    {AG_WLAN_ATCMD_POW,      "AT^WIPOW=60000\r\n"},
    {AG_WLAN_ATCMD_TX,       "AT^WITX=1\r\n"},
};

static aging_atcmd_info atcmd_5G_info[] =
{
    {AG_WLAN_ATCMD_TSELRF,   g_atcmd_tselrf},
    {AG_WLAN_ATCMD_MODE,     g_atcmd_wimode},
    {AG_WLAN_ATCMD_BAND,     "AT^WIBAND=2\r\n"},
    {AG_WLAN_ATCMD_FREQ,     g_atcmd_freq},
    {AG_WLAN_ATCMD_DATARATE, "AT^WIDATARATE=35100\r\n"},
    {AG_WLAN_ATCMD_POW,      "AT^WIPOW=60000\r\n"},
    {AG_WLAN_ATCMD_TX,       "AT^WITX=1\r\n"},
};

#define AGING_WLAN_STOP_TX  "AT^WITX=0\r\n"

#define AGING_WLAN_STOP_DUALBAND_TX "AT^WIENABLE=0\r\n"

static unsigned int g_stop_all = 0;
    
/*工作队列结构变量*/
static aging_work_struct wlan_aging_work = {0};
static unsigned int wlan_init_flag = 0;  /*wlan测试初始化标志*/
static unsigned int wlan_exit_flag = 0;  /*wlan测试结束标志*/
static struct workqueue_struct * aging_wlan_test_workqueue = NULL;
static DEFINE_MUTEX(aging_wlan_mutex); /* wlan测试资源互斥锁 */

/******************************************************************
*函数名   : aging_wlan_execute_mode
*函数功能 : 开启某个频段的wlan 老化测试
*输入参数 : aging_test_priv *priv -- 老化测试设备结构指针
            unsigned short mode -- 老化测试频段
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
******************************************************************/
static int aging_wlan_execute_mode(aging_wlan_work_data *wlan_work_data,
    AGING_WIFI_CONFIG *wifi_config)
{
    int i = 0;
    int ret = 0;
    int phymode = 0;
    int mode = 0;
    int atlist_num = 0;
    unsigned int antenna = 0;
    aging_atcmd_info *pstATcmd_info = NULL;
    
    if (NULL == wlan_work_data
     || NULL == wifi_config)
    {
        aging_print_error("Input para error\n");
        return -1;
    }

    if (wifi_config->mode >= WLAN_2G_11A 
     && wifi_config->mode <= WLAN_2G_11AC)
    {
        mode = AGING_WLAN_2G_TEST;
        if (ANTENNA_1 == wlan_work_data->antenna)
        {
            antenna = AGING_WLAN_2G_ANTENNA1;
        }
        else
        {
            antenna = AGING_WLAN_2G_ANTENNA2;
        }
    }
    else if (wifi_config->mode >= WLAN_5G_11A 
    && wifi_config->mode <= WLAN_5G_11AC)
    {
        mode = AGING_WLAN_5G_TEST;
        if (ANTENNA_1 == wlan_work_data->antenna)
        {
            antenna = AGING_WLAN_5G_ANTENNA1;
        }
        else
        {
            antenna = AGING_WLAN_5G_ANTENNA2;
        }
    }
    else
    {
        aging_print_error("Error WiFi mode unknown: %d\n", wifi_config->mode);
        return -1;
    }

    if (AGING_WLAN_2G_TEST == mode)
    {
        atlist_num = (unsigned int)(sizeof(atcmd_2G_info) / sizeof(atcmd_2G_info[0]));
        pstATcmd_info = &(atcmd_2G_info[0]);
    }
    else
    {
        atlist_num = (unsigned int)(sizeof(atcmd_5G_info) / sizeof(atcmd_5G_info[0]));
        pstATcmd_info = &(atcmd_5G_info[0]);
    }
    
    /*顺序执行AT命令开始老化测试*/
    for(i = 0; i < atlist_num; i++)
    {
        if((pstATcmd_info[i].atcmd != (unsigned int)i)
        || (NULL == pstATcmd_info[i].atname))
        {
            aging_print_error("wlan atcmd not in order or atname not exist, %d.\n", i);
            return -1;
        }
    
        if (AG_WLAN_ATCMD_TSELRF == pstATcmd_info[i].atcmd)
        {
            (void)memset((void *)(pstATcmd_info[i].atname), 0, AGING_ATCMD_LEN);
            
            (void)snprintf(pstATcmd_info[i].atname, AGING_ATCMD_LEN,
                              "AT^TSELRF=7,%d\r\n", antenna);
        }

        if (AG_WLAN_ATCMD_MODE == pstATcmd_info[i].atcmd)
        {
            (void)memset((void *)(pstATcmd_info[i].atname), 0, AGING_ATCMD_LEN);
            switch(wifi_config->mode)
            {
                case WLAN_2G_11B:
                {
                    phymode = AT_WIMODE_80211b;
                    break;
                }
                case WLAN_2G_11G:
                {
                    phymode = AT_WIMODE_80211g;
                    break;
                }
                case WLAN_2G_11N:
                {
                    phymode = AT_WIMODE_80211n;
                    break;
                }
                case WLAN_5G_11A:
                {
                    phymode = AT_WIMODE_80211a;
                    break;
                }
                case WLAN_5G_11N:
                {
                    phymode = AT_WIMODE_80211n;
                    break;
                }
                case WLAN_5G_11AC:
                {
                    phymode = AT_WIMODE_80211ac;
                    break;
                }
                case WLAN_2G_11A:
                case WLAN_2G_11AC:
                case WLAN_5G_11B:
                case WLAN_5G_11G:
                default:
                {
                    aging_print_error("wlan atcmd wifimode not supported %d.\n", wifi_config->mode);
                    return -1;
                }
            }

            (void)snprintf(pstATcmd_info[i].atname, AGING_ATCMD_LEN,
                              "AT^WIMODE=%d\r\n", phymode);
        }

        if (AG_WLAN_ATCMD_FREQ == pstATcmd_info[i].atcmd)
        {
            (void)memset((void *)(pstATcmd_info[i].atname), 0, AGING_ATCMD_LEN);
            
            (void)snprintf(pstATcmd_info[i].atname, AGING_ATCMD_LEN,
                              "AT^WIFREQ=%d\r\n", wifi_config->frequency);
        }

        aging_print_error("wlan aging to execute command: %s\n", pstATcmd_info[i].atname);
        ret = At_CmdStreamPreProc(0, pstATcmd_info[i].atname,
                    strlen(pstATcmd_info[i].atname));
        if(0 != ret)
        {
            aging_print_error("wlan at index %d Cmd %s send failed.\n", i, pstATcmd_info[i].atname);
            return -1;
        }
        msleep(AGING_ATCMD_DELAY_TIME);
    }  

    return 0;
}

/******************************************************************
*函数名   : aging_wlan_test_open
*函数功能 : 开启某个频段的wlan 老化测试
*输入参数 : aging_test_priv *priv -- 老化测试设备结构指针
            wlan_work_data -- 老化测试数据
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
******************************************************************/
static int aging_wlan_test_open(struct aging_test_priv *priv, 
    aging_wlan_work_data *wlan_work_data)
{
     int index = 0;
     AGING_WIFI_CONFIG wifi_config;
     
     if((NULL == priv) 
     || (NULL == wlan_work_data))
     {
         aging_print_error("Input para error\n");
         return -1;
     }

     if (wlan_work_data->work_index >= 
        priv->aging_test_nv.wifi_parameter.mode_num)
     {
         wlan_work_data->work_index = 0; //重头开始
     }

     index = wlan_work_data->work_index;
             
     wifi_config = priv->aging_test_nv.wifi_parameter.wifi_parametr[index];

     //2G的2根天线以及5G的2根天线轮流测试
     if (wlan_work_data->work_index % 2 == 0)
     {
         wlan_work_data->antenna= (wlan_work_data->antenna+1)%2;
     }
     
     wlan_work_data->work_index++; //切换到下一个index

     return aging_wlan_execute_mode(wlan_work_data, &wifi_config);
}

/******************************************************************
*函数名   : aging_wlan_test_close
*函数功能 : 关闭wifi老化测试
*输入参数 : 无
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
******************************************************************/
static int aging_wlan_test_close(void)
{
    int ret = -1;

    /*发送AT命令关闭射频发射*/
    if (!g_stop_all)
    {
        ret = At_CmdStreamPreProc(0, AGING_WLAN_STOP_TX, strlen(AGING_WLAN_STOP_TX));
        if(0 != ret)
        {
            aging_print_error("close aging wlan test failed.\n");
            ret = -1;
        }
    }
    else 
    {
        /*双频模式下需要切换一次天线将两个频段都干掉*/
        ret = At_CmdStreamPreProc(0, AGING_WLAN_STOP_DUALBAND_TX, strlen(AGING_WLAN_STOP_DUALBAND_TX));
        if(0 != ret)
        {
            aging_print_error("close aging wlan test failed.\n");
            ret = -1;
        }
    }
    
    return ret;
}

/********************************************************
*函数名   : aging_wlan_test_workfun
*函数功能 : wlan 老化测试work函数
*输入参数 : void *p，即:struct platform_device *pdev
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
********************************************************/
static void aging_wlan_test_workfun(struct work_struct *work)
{
    int ret = -1;
    aging_work_struct *pa_work = NULL;
    aging_wlan_work_data *wlan_work_data = NULL;
    unsigned long ldelay_time = 0;
    
    if(NULL == work)
    {
        aging_print_error("Input para error.\n");
        return;
    }

    pa_work = to_aging_work(work);
    if((NULL == pa_work) || (NULL == pa_work->priv)
        || (NULL == pa_work->work_data))
    {
        aging_print_error("aging work struct get failed.\n");
        return;
    }
    wlan_work_data = (aging_wlan_work_data *)(pa_work->work_data);

    mutex_lock(&aging_wlan_mutex);
    if(0 != wlan_exit_flag)
    {
        /*已退出不设置状态*/
        aging_print_error("wlan aging work has exited, %d.\n", wlan_exit_flag);
        mutex_unlock(&aging_wlan_mutex);
        return;
    }
    /*根据状态选择处理*/
    switch(pa_work->work_state)
    {
        case AG_WORK_START:
        case AG_WORK_RUN:
        {
            /*开始测试*/
            aging_print_error("wlan aging start test\n");

            ret = aging_wlan_test_open(pa_work->priv, wlan_work_data);
            if(0 != ret)
            {
                aging_print_error("wlan aging test open error\n");
                goto WORK_FAIL;
            }

            //双PA同时测试的场景，2.4G和5G同时发射功率
            if (wlan_work_data->dualband)
            {
                ret = aging_wlan_test_open(pa_work->priv, wlan_work_data);
                if(0 != ret)
                {
                    aging_print_error("wlan aging test open error\n");
                    goto WORK_FAIL;
                }
            }

            /*开启后重新设置定时,等待时间为工作时间*/
            ldelay_time = (unsigned long)(wlan_work_data->on_time * AGING_MIN_TO_MSEC);
            pa_work->work_state = AG_WORK_REST; /*下一个阶段休息*/
         
            /*重新加入work*/
            (void)queue_delayed_work(aging_wlan_test_workqueue,
                  &(pa_work->d_work), msecs_to_jiffies(ldelay_time));
            break; 
        }
        case AG_WORK_REST:
        {
            /*开始休息*/
            aging_print_error("wlan aging start sleep\n");

            ret = aging_wlan_test_close();
            if(0 != ret)
            {
                aging_print_error("aging test close error\n");                
                goto WORK_FAIL;
            }
            
            /*关闭后重新设置定时,等待时间为休息时间*/
            ldelay_time = (unsigned long)(wlan_work_data->off_time * AGING_MIN_TO_MSEC);
            pa_work->work_state = AG_WORK_RUN;  
            
            (void)queue_delayed_work(aging_wlan_test_workqueue,
                &(pa_work->d_work), msecs_to_jiffies(ldelay_time));
            break;
        }
        default:
        {
            /*其他均不做处理*/
            aging_print_error("default case = %d, do nothing.\n", pa_work->work_state);
            break;
        }
    }
    mutex_unlock(&aging_wlan_mutex);

    return;

WORK_FAIL:
    /*失败设置fail状态并退出*/
    mutex_unlock(&aging_wlan_mutex);
    AGING_STATUS_SET_FAIL(WLAN_TEST_MODULE, pa_work->priv->aging_total_status);
    return;
}

/********************************************************
*函数名   : aging_wlan_test_thread
*函数功能 : wlan 老化测试线程
*输入参数 : void *p，即:struct platform_device *pdev
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
*           2015-8-30 罗琦 初版作成
********************************************************/
int aging_wlan_test_thread(void *p)
{
    struct aging_test_priv *priv = NULL;
    aging_wlan_work_data *wlan_work_data = NULL;
    if((NULL == p) || (0 == wlan_init_flag))
    {
        aging_print_error("p is NULL or wlan test not init, %d!!\n", wlan_init_flag);
        return 0;
    }

    priv = platform_get_drvdata((struct platform_device *)p);
    wlan_work_data = (aging_wlan_work_data *)(wlan_aging_work.work_data);
    if((NULL == priv) || (NULL == wlan_work_data))
    {
        aging_print_error("priv or wlan data is NULL!!\n");
        return -1;
    }

    /* 初次上电等待15秒等系统起来 */
    aging_print_error("aging wlan test start and wlan wait for system initialized.\n");    
    msleep((unsigned int)(WLAN_AGING_WAIT_SYSTEM_SETUP_TIME));

    /*共享资源需要保护*/
    mutex_lock(&aging_wlan_mutex);
    if(0 != wlan_exit_flag)
    {
        /*已退出不设置状态*/
        aging_print_error("wlan aging work has exited, %d.\n", wlan_exit_flag);
        mutex_unlock(&aging_wlan_mutex);
        return 0;
    }
    wlan_work_data->work_index = 0;
    wlan_aging_work.work_state = AG_WORK_START;
    (void)queue_delayed_work(aging_wlan_test_workqueue,
                &(wlan_aging_work.d_work), (unsigned long)AG_WORK_START);
    mutex_unlock(&aging_wlan_mutex);

    return 0;
}


int aging_wlan_test_init(void *p)
{
    struct aging_test_priv *priv = NULL;
    aging_wlan_work_data *wlan_work_data = NULL;
    if(NULL == p)
    {
        aging_print_error("p is NULL !!\n");
        return -EINVAL;
    }

    wlan_init_flag = 0;
    priv = platform_get_drvdata((struct platform_device *)p);
    if(NULL == priv)
    {
        aging_print_error("priv is NULL !!\n");
        return -EINVAL;
    }
    
    if(AGING_ENABLE != priv->aging_test_nv.wifi_parameter.wifi_test_enable)
    {
        aging_print_error("wlan test disabled in nv!!\n");
        return 0;
    }

    /*初始化work队列*/
    wlan_work_data = (aging_wlan_work_data *)kmalloc(sizeof(aging_wlan_work_data), GFP_KERNEL);
    if(NULL == wlan_work_data)
    {
        aging_print_error("malloc wlan data failed!!\n");
        return -1;
    }
    
    (void)memset(wlan_work_data, 0, sizeof(aging_wlan_work_data));
    
    wlan_work_data->wifi_band_num = priv->aging_test_nv.wifi_parameter.mode_num;
    wlan_work_data->on_time = priv->aging_test_nv.wifi_parameter.on_time;
    wlan_work_data->off_time = priv->aging_test_nv.wifi_parameter.off_time;
    wlan_work_data->antenna = ANTENNA_2;

    if (0 == priv->aging_test_nv.wifi_parameter.off_time) //若休息时间为0，则认为是双发模式
    {
        wlan_work_data->dualband = 1;
        g_stop_all = 1;
    }
    else
    {
        wlan_work_data->dualband = 0;
        g_stop_all = 0;
    }
    
    aging_wlan_test_workqueue = create_workqueue(WLAN_AGING_WORK_NAME);
    if(NULL == aging_wlan_test_workqueue)
    {
        aging_print_error("create workqueue failed.\n");
        kfree(wlan_work_data);
        wlan_work_data = NULL;
        return -1;
    }
    wlan_aging_work.work_data = (aging_wlan_work_data *)wlan_work_data;
    wlan_aging_work.priv = priv;
    INIT_DELAYED_WORK(&(wlan_aging_work.d_work), aging_wlan_test_workfun);

    wlan_init_flag = 1;

    aging_print_error("wlan aging test init.\n");
    return 0;
}


int aging_wlan_test_exit(void *p)
{
    int ret = -1;
    if((NULL == p) || (0 == wlan_init_flag))
    {
        /*此处不返回异常*/
        return 0;
    }

    mutex_lock(&aging_wlan_mutex);
    /*准备结束先设置标志*/
    wlan_exit_flag = 1;
    (void)cancel_delayed_work(&(wlan_aging_work.d_work));
    flush_workqueue(aging_wlan_test_workqueue);
    wlan_aging_work.work_state = AG_WORK_END;
    destroy_workqueue(aging_wlan_test_workqueue);

    /*释放申请的内存*/
    if(NULL != wlan_aging_work.work_data)
    {
        kfree(wlan_aging_work.work_data);
        wlan_aging_work.work_data = NULL;
    }

    /*关闭射频发射*/
    ret = aging_wlan_test_close();
    if(0 != ret)
    {
        aging_print_error("wlan aging test close error.\n");
    }
    mutex_unlock(&aging_wlan_mutex);

    aging_print_error("wlan aging test end.\n");
    return 0;

}

