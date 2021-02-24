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



#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/jiffies.h>
#include "linux/wakelock.h"
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include "mbb_anten.h"
#include "anten_detect.h"
#if (FEATURE_ON == MBB_RFFE_ANT_TO_MAIN_EXANT)
#include "bsp_icc.h"
#include "product_nv_id.h"
#include "product_nv_def.h"
#include "bsp_nvim.h"

struct anten_msg_stru
{
    unsigned int modem_id;
    unsigned int status;
};
#define MAIN_ANT_TYPE 0   /*0表示为主天线1表示为副天线*/
static NV_RF_ANTEN_DETECT_GPIO_STRU g_anten_stru = {0}; /*nv 50569的配置信息*/
static int g_main_exant_detect_state = 1; /*天线插入的状态，0表示低电平，1表示为高电平*/
#endif

#ifdef __cplusplus
extern "C" {
#endif
#define ant_print_err(fmt, ...)  (printk(KERN_ERR "[Antenna]%s: "fmt"\n", __func__, ##__VA_ARGS__))
#define ant_print_info(fmt, ...) (printk(KERN_INFO "[Antenna]%s: "fmt"\n", __func__, ##__VA_ARGS__))

static char  msg_container[MSG_CONTAINER_LEN] = {0};
static char* container_ant_msg = " current_antenna %d \n outside_antenna %d \n";

static ant_dev_info g_ant_info = {0};

/*调试接口，用于查看gpio的信息和当前状态，供手动调用*/
void show_ant_info(void)
{
    u32 idx = 0;
    u32 num = 0;
    ant_gpio_info*   ant_info = g_ant_info.ant_info;
    ant_switch_info* swt_info = NULL;

    printk(KERN_ERR "======total %d antenna======\n", g_ant_info.ant_num);
    for (idx = 0; idx < g_ant_info.ant_num; idx++)
    {
        printk(KERN_ERR "======show antenna %d info======\n", idx);
        printk(KERN_ERR "proc_name[%s], ant_type[%d],detect_name[%s],gpio_num[%d],insert_value[%d]\n",
                           ant_info->proc_name,
                           ant_info->ant_type,
                           ant_info->ant_detect.gpio_name,
                           ant_info->ant_detect.gpio_num, 
                           ant_info->ant_detect.insert_value);
        
        printk(KERN_ERR "antenna[%d] has %d switch gpio\n",idx, ant_info->switch_gpio_num);
        swt_info = ant_info->ant_switch;
        for (num = 0; num < ant_info->switch_gpio_num; num++)
        {
            printk(KERN_ERR "switch[%d]:gpio_name[%s],gpio_num[%d],inner_value[%d].\n",num,
                           swt_info->gpio_name,
                           swt_info->gpio_num,
                           swt_info->inner_value);
            swt_info += 1;
        }
        printk(KERN_ERR "current detect state is [%d]\n",ant_info->ant_detect.detect_state);       
        printk(KERN_ERR "current switch anten is [%d]\n", ant_info->switch_state);
        
        ant_info += 1;
    }

}

/*****************************************************************************
 函 数 名  : anten_switch_get
 功能描述  : 提供给AT模块获取天线切换状态的接口
 输入参数  : ant_type : 天线类型，0--主天线，1--辅天线
 输出参数  : 无
 返 回 值  : 当前天线的状态
 说    明  : 此接口只给at使用，应用获取状态通过节点操作
*****************************************************************************/
int anten_switch_get(u32 ant_type)
{
    ant_gpio_info* ant_info = NULL;
    
    if (g_ant_info.ant_num - 1 < ant_type)
    {
        ant_print_err("only support %d antenna,pls check.",g_ant_info.ant_num);
        return ANTE_ERR;
    }
    
    ant_info = &g_ant_info.ant_info[ant_type];
    if(NULL == ant_info)
    {
        ant_print_err("this cannot be enter,some err hanppened!!!");
        return ANTE_ERR;
    }

    return (ant_info->switch_state);
}

static int __anten_switch_set(u32 ant_type,u32 in_or_out)
{
    u32 index = 0;
    int value = 0;
    ant_gpio_info*   ant_info = NULL;
    ant_switch_info* swt_info = NULL;
    
    if (g_ant_info.ant_num - 1 < ant_type)
    {
        ant_print_err("only support %d antenna,pls check.",g_ant_info.ant_num);
        return ANTE_ERR;
    }

    if (SWITCH_TO_INNER_ANTEN != in_or_out && SWITCH_TO_OUTER_ANTEN != in_or_out)
    {
        ant_print_err("in_or_out[%u] para err,pls check.",in_or_out);
        return ANTE_ERR;
    }
    
    ant_info = &g_ant_info.ant_info[ant_type];

    /*如果需要设置的状态与当前状态一致，直接返回*/
    if (ant_info->switch_state == in_or_out)
    {
        ant_print_err("the state to set[%u] is equal to current state[%d].",
            in_or_out, ant_info->switch_state);
        return ANTE_OK;
    }

    ant_print_err("ant[%u] is going to set [%d].",ant_type, in_or_out);
    for (index = 0; index < ant_info->switch_gpio_num; index++)
    {
        swt_info = &ant_info->ant_switch[index];
        
        if (SWITCH_TO_INNER_ANTEN == in_or_out)
        {
            value = swt_info->inner_value;
        }
        else
        {
            value = !(swt_info->inner_value);
        }
        (void)gpio_direction_output(swt_info->gpio_num, value);
    }
    
    ant_info->switch_state = (in_or_out == SWITCH_TO_INNER_ANTEN ? \
                             CURRENT_IS_INNER_ANTEN : CURRENT_IS_OUTER_ANTEN);

    return 0;
}

/*****************************************************************************
 函 数 名  : anten_switch_set
 功能描述  : 提供给AT模块操作天线切换的接口
 输入参数  : ant_type : 天线类型，0--主天线，1--辅天线，2--所有天线
             in_or_out: 使用内置还是外部天线
 输出参数  : 无
 返 回 值  : 0--成功，其他--失败
 说    明  : 此接口只给at使用，应用切换天线通过节点操作
*****************************************************************************/
int anten_switch_set(u32 ant_type,u32 in_or_out)
{
    int ret = 0;
    u32 idx = 0;
    
    /*AT命令里可能会同时设置两个天线*/
    if (g_ant_info.ant_num == ant_type)
    {
        for (idx = 0; idx < g_ant_info.ant_num; idx++)
        {
            ret |= __anten_switch_set(idx,in_or_out);
        }
    }
    else
    {
        ret = __anten_switch_set(ant_type, in_or_out);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : anten_number_get
 功能描述  : 提供给AT模块获取天线数目的接口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 天线数目
 说    明  : 此接口只给at使用
*****************************************************************************/
int anten_number_get(void)
{
    return g_ant_info.ant_num;
}

/*****************************************************************************
 函 数 名  : anten_detect_get
 功能描述  : 提供给AT模块获取天线插入状态的接口
 输入参数  : ant_type : 天线类型，0--主天线，1--辅天线
 输出参数  : 无
 返 回 值  : 当前天线的状态( -1获取失败，0天线未插入，1天线已插入 )
 说    明  : 此接口只给at使用
*****************************************************************************/
int anten_detect_get(int ant_type)
{
    ant_gpio_info* ant_info = NULL;
    
    /* 如果天线数目非法，则返回错误 */
    if ((0 > ant_type) || (g_ant_info.ant_num - 1 < ant_type))
    {
        ant_print_err("only support %d antenna,pls check.",g_ant_info.ant_num);
        return ANTEN_DETECT_STATUS_ERR;
    }
    
    /* 如果指定天线不存在，则返回错误 */
    ant_info = &g_ant_info.ant_info[ant_type];
    if(NULL == ant_info)
    {
        ant_print_err("ant_info is null");
        return ANTEN_DETECT_STATUS_ERR;
    }

    if((ANTEN_DETECT_STATUS_NOT_INSERTED != ant_info->ant_detect.detect_state)
        && (ANTEN_DETECT_STATUS_INSERTED != ant_info->ant_detect.detect_state))
    {
        return ANTEN_DETECT_STATUS_ERR;
    }
    
    return (ant_info->ant_detect.detect_state);
}

/*****************************************************************************
 函 数 名  : ant_read
 功能描述  : 应用写节点操作接口
 输入参数  : file:  文件句柄，对应proc节点；
             buf:   应用态数据存放地址；
             count: 要读的长度;
             ppos:  当前操作的文件位置
 输出参数  : 无
 返 回 值  : 已读入长度
 说    明  : 
*****************************************************************************/
static ssize_t ant_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;
    int len = 0;
    int index = 0;
    ant_gpio_info* ant_info = g_ant_info.ant_info;

    (void)memset(msg_container, '\0', MSG_CONTAINER_LEN);
    
    /*参数检查*/
    if (NULL == file || NULL == buf || NULL == ppos || NULL == ant_info)
    {
        ant_print_err("para is null.");
        return  ANTE_ERR;
    }
    
    while( index < g_ant_info.ant_num)
    {
        if (0 == strncmp(file->f_path.dentry->d_iname, ant_info->proc_name, strlen(ant_info->proc_name)))
        {
            break;
        }
        else 
        {
            index++;
            ant_info += 1;
        }
    }

    if (index >= g_ant_info.ant_num)
    {
        ant_print_err("[%s]not supported.",file->f_path.dentry->d_iname);
        return  ANTE_ERR;
    } 
    
    /*应用调用读接口来获取状态值时清除poll 标志*/
    ant_info->ant_detect.poll_flag = 0;
    
    /*将当前检测和切换gpio的状态发给应用*/
    len = snprintf(msg_container, MSG_CONTAINER_LEN, container_ant_msg, 
            ant_info->switch_state, ant_info->ant_detect.detect_state);

    if(len < 0)
    {
        ant_print_err("fill msg failed.");
        return -EFAULT;
    }

    len = strnlen(msg_container, MSG_CONTAINER_LEN);    
    if(*ppos >= len )
    {
        return 0;
    }
    if(count > len)
    {
        count = len;
    }

    ret = copy_to_user(buf, msg_container, count);
    if(0 != ret)
    {
        ant_print_err("msg copy to user Failed!\n");
        return -EFAULT;
    }
    
    *ppos += count;
    
    return count;
}
/*****************************************************************************
 函 数 名  : ant_write
 功能描述  : 应用写节点操作接口
 输入参数  : file:  文件句柄，对应proc节点；
             buf:   应用态数据存放地址；
             length:要写的长度;
             ppos:  当前写的文件位置
 输出参数  : 无
 返 回 值  : 写入长度
 说    明  : 
*****************************************************************************/
static int ant_write(struct file *file, const char __user *buf, size_t length, loff_t *ppos)
{
    
    int index = 0;
    u32 state = 0;
    char recv_buf[BUFFER_LEN] = {0};
    
    ant_gpio_info* ant_info = g_ant_info.ant_info;
    
    /*参数检查*/
    if (NULL == file || NULL == buf || NULL == ppos || NULL == ant_info)
    {
        ant_print_err("para is null.");
        return  ANTE_ERR;
    }

    /*找到要操作的天线*/
    while( index < g_ant_info.ant_num)
    {
        if (0 == strncmp(file->f_path.dentry->d_iname, ant_info->proc_name, strlen(ant_info->proc_name)))
        {
            break;
        }
        else 
        {
            index++;
            ant_info += 1;
        }
    }

    if (index >= g_ant_info.ant_num)
    {
        ant_print_err("[%s]not supported.",file->f_path.dentry->d_iname);
        return  ANTE_ERR;
    } 
    
    /*应用只传0或者1下来，取1位就够了*/
    if(copy_from_user(recv_buf, (void*)buf, 1))
    {
        ant_print_err("[%s]copy from user data failed.", file->f_path.dentry->d_iname);
        return -EFAULT;
    }
    recv_buf[BUFFER_LEN - 1] = '\0';
    
    /*这里是ascii码，转为十进制数，合法性判断在set函数里校验*/
    state = recv_buf[0] - '0';

    if (0 != __anten_switch_set(index, state))
    {
        ant_print_err("set switch gpio err,name[%s],inout[%d]", file->f_path.dentry->d_iname, state);
        return -EFAULT; 
    }

    return length;
}

/*****************************************************************************
 函 数 名  : ant_poll
 功能描述  : 文件poll接口，当达到一定条件(有gpio插拔时)向应用通知事件
 输入参数  : file:文件句柄，对应proc节点；
             wait:没有事件发生时，挂载的等待队列表
 输出参数  : 无
 返 回 值  : 事件类型
 说    明  : 
*****************************************************************************/
static unsigned int ant_poll(struct file *file, poll_table *wait)
{ 
    int ret   = 0;
    int index = 0;
    ant_gpio_info* ant_info = g_ant_info.ant_info;
    
    /*参数检查*/
    if (NULL == file || NULL == wait || NULL == ant_info)
    {
        ant_print_err("file is null.");
        return  ANTE_ERR;
    }

    /*找到要操作的天线*/
    while( index < g_ant_info.ant_num)
    {
        if (0 == strncmp(file->f_path.dentry->d_iname, ant_info->proc_name, strlen(ant_info->proc_name)))
        {
            break;
        }
        else 
        {
            index++;
            ant_info += 1;
        }
    }

    if (index >= g_ant_info.ant_num)
    {
        ant_print_err("[%s]not supported.",file->f_path.dentry->d_iname);
        return  ANTE_ERR;
    } 

    poll_wait(file, &ant_info->ant_detect.wait, wait);

    if (ant_info->ant_detect.poll_flag) 
    {
        ret = POLLIN | POLLRDNORM;
    }
    
    return ret;
}

static const struct file_operations ant_operations = {
    .read  = ant_read,
    .write = ant_write,
    .poll  = ant_poll,
};

#if (FEATURE_ON == MBB_RFFE_ANT_TO_MAIN_EXANT)
/*****************************************************************************
 函 数 名  : ant_handle_work
 功能描述  : 判断如果主外置天线状态发生变化，将他的状态通过icc发送给c核处理
 输入参数  : detect:插入或拔出天线的配置信息
 输出参数  : 无
 返 回 值  : 无
 说    明  : 
*****************************************************************************/
static void ant_handle_work(ant_detect_info* detect)
{
    int len = 0;
    unsigned int chan_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_ANTEN;
    struct anten_msg_stru anten_msg  = {0};
    /*detect入参数在调用的时候已经做过判断这里不用再做入参判断*/

    /*nv 50569的使能位为true，并且是指定的nv主天线检测位*/
    if ((TRUE == g_anten_stru.enable) && (detect->gpio_num == g_anten_stru.anten[MAIN_ANT_TYPE]))
    {
        anten_msg.status = gpio_get_value((unsigned)(detect->gpio_num));
        anten_msg.modem_id = MAIN_ANT_TYPE;
        ant_print_err("main_anten_gpio = %d before status =%d\n",
            g_anten_stru.anten[MAIN_ANT_TYPE], g_main_exant_detect_state);

        if (anten_msg.status != g_main_exant_detect_state)
        {
            g_main_exant_detect_state = anten_msg.status;
            ant_print_err("modem id = 0x%x, status = %d.", anten_msg.modem_id, anten_msg.status);
            /*通过IFC将消息发出*/
            len = bsp_icc_send(ICC_CPU_MODEM, chan_id, (unsigned char*)&anten_msg, (u32)sizeof(struct anten_msg_stru));
            if (len != sizeof(struct anten_msg_stru))
            {
                ant_print_err("send len(%d) != expected len.\n", len);
            }
        }
    }
}
#endif

static void ant_timer_fn(unsigned long data)
{
    ant_detect_info* detect = (ant_detect_info*)data;
    unsigned long flags;

    if (NULL == detect)
    {
        ant_print_err("detect is null. ");
        return;
    }
    
    spin_lock_irqsave(&detect->ops_lock, flags);

#if (FEATURE_ON == MBB_RFFE_ANT_TO_MAIN_EXANT)
    ant_handle_work(detect);
#endif

    if (detect->insert_value == gpio_get_value((unsigned)(detect->gpio_num)))
    {
        detect->detect_state  = DETECT_IS_INSERT;

    }
    else
    {
        detect->detect_state = DETECT_IS_OUTSIDE;
    }
    detect->poll_flag = 1;
    wake_up_interruptible(&detect->wait);
     
    spin_unlock_irqrestore(&detect->ops_lock, flags);
    
    return;
}
static irqreturn_t ant_gpio_isr(int irq, void *data)
{
    ant_detect_info *detect = (ant_detect_info *)data;
    unsigned long flags;
    
    if (NULL == detect)
    {
        ant_print_err("detect is null. ");
        return ANTE_ERR;
    }
    
    spin_lock_irqsave(&detect->ops_lock, flags);

    /* 天线延时防抖定时启动*/
    (void)mod_timer(&detect->timer, jiffies + msecs_to_jiffies(TIME_DELAY));

    spin_unlock_irqrestore(&detect->ops_lock, flags);
    return IRQ_HANDLED;
}
/*****************************************************************************
 函 数 名  : detect_gpio_init
 功能描述  : 对天线gpio初始化配置，并默认选择内置天线
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 0--成功，-1--失败
 说    明  : 
*****************************************************************************/
static int detect_gpio_init(ant_detect_info* det_info)
{
    int ret = 0;

    if (NULL == det_info)
    {
        ant_print_err("det_info is null. ");
        return ANTE_ERR;
    }
    
    spin_lock_init(&det_info->ops_lock);
    
    ret = gpio_request(det_info->gpio_num, det_info->gpio_name);
    if(0 != ret) 
    {
        ant_print_err("request gpio %s FAIL!", det_info->gpio_name);
        return -EBUSY;
    }

    /*中断gpio需设置为输入模式*/
    gpio_direction_input(det_info->gpio_num);

    setup_timer(&det_info->timer, ant_timer_fn, (unsigned long)det_info);
    
    ret = request_irq((unsigned int)gpio_to_irq(det_info->gpio_num), ant_gpio_isr, 
                    IRQF_NO_SUSPEND | IRQF_SHARED| IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, 
                    det_info->gpio_name, det_info);
    if (0 != ret)
    {
        ant_print_err("fail to request gpio[%s] irq handler.", det_info->gpio_name);
        goto err_request_irq;
    }
    
    det_info->poll_flag = 0;
    init_waitqueue_head(&det_info->wait);

    /*初始化时记录下状态*/
    if(det_info->insert_value == gpio_get_value(det_info->gpio_num))
    {
        /*外置天线插入*/
        det_info->detect_state = DETECT_IS_INSERT;
    }
    else
    {
        /*外置天线未插入*/
        det_info->detect_state = DETECT_IS_OUTSIDE;
    }

    return 0;
    
err_request_irq:
    gpio_free((unsigned)(det_info->gpio_num));
    return ret;

}

/*****************************************************************************
 函 数 名  : antenna_gpio_init
 功能描述  : 对天线gpio初始化配置，并默认选择内置天线
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 0--成功，-1--失败
 说    明  : 
*****************************************************************************/
static int antenna_gpio_init(void)
{
    int error   = 0;
    u32 det_idx = 0,swit_idx = 0;
    ant_gpio_info*   pdata = NULL;
    ant_switch_info* ant_switch = NULL;
    
    pdata = g_ant_info.ant_info;
    for(det_idx = 0; det_idx < g_ant_info.ant_num; det_idx++)
    { 
        error = detect_gpio_init(&pdata->ant_detect);
        if (0 != error)
        {
            return error;
        }

        ant_switch = pdata->ant_switch;
        for(swit_idx = 0; swit_idx < pdata->switch_gpio_num; swit_idx++)
        {
            error = gpio_request(ant_switch->gpio_num, ant_switch->gpio_name);
            if(0 != error) 
            {
                ant_print_err("request gpio %s FAIL!", ant_switch->gpio_name);
                return -EBUSY;
            }
            /*默认配置为内置天线*/
            (void)gpio_direction_output(ant_switch->gpio_num, ant_switch->inner_value);
            
            ant_switch += 1;
        }
        /*保存下当前切换gpio的状态*/
        pdata->switch_state = CURRENT_IS_INNER_ANTEN;

        pdata += 1;
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : antenna_proc_init
 功能描述  : 从dts文件获取天线gpio的相关信息，填充到g_ant_info全局信息里
 输入参数  : dev:内核传入device结构体，从中获取dts节点信息
 输出参数  : 无
 返 回 值  : 0--成功，-1--失败
 说    明  : 
*****************************************************************************/
static int antenna_dts_init(struct device *dev)
{
    int ret   = 0;
    int count = 0;
    struct device_node* np = dev->of_node; 
    struct device_node* child    = NULL;       /* 子节点 */
    struct device_node* grdchild = NULL;       /* 孙子节点*/
    ant_gpio_info*      pdata    = NULL;
    ant_switch_info*    ant_switch = NULL;

    if(NULL == np)
    {
        ant_print_err("dts node is null.");
        return -ENODEV;
    }

    /* 获取子节点个数 */
    count = of_get_child_count(np);
    if (0 == count)
    {
        ant_print_err("of_get_child_count ERROR.");
        return -ENODEV;
    }
    g_ant_info.ant_num = count;

    /*用devm_kzalloc申请的内存，不用自己释放，在设备detach的时候会自动释放内存*/
    pdata = (ant_gpio_info*)devm_kzalloc(dev,sizeof(ant_gpio_info) * count, GFP_KERNEL);
    if(NULL == pdata)
    {
        ant_print_err("alloc memory ERROR.");
        return -ENOMEM; 
    }
    g_ant_info.ant_info = pdata;
    
    /* 循环遍历子节点 */
    for_each_child_of_node(np, child)
    {
        ret = of_property_read_string_index(child,"proc_name",0,&pdata->proc_name);
        if(0 != ret)
        {
            ant_print_err( "proc name get failed."); 
            return -EINVAL;
        }

        ret  = of_property_read_u32_index(child, "ant_type", 0, &pdata->ant_type);
        ret |= of_property_read_string_index(child,"detect_name", 0, &pdata->ant_detect.gpio_name);
        if (0 != ret) 
        {
            ant_print_err("ant detect name get failed.");
            return -EINVAL;
        }

        ret  = of_property_read_u32_index(child, "detect_gpio", 0, &pdata->ant_detect.gpio_num);
        ret |= of_property_read_u32_index(child, "detect_gpio", 1, &pdata->ant_detect.insert_value);
        if(0 != ret)
        {
            ant_print_err("ant detect level type info get failed.");
            return -EINVAL;
        }
            
        /*获取天线切换gpio的个数*/
        count = of_get_child_count(child);
        if (0 == count)
        {
            ant_print_err("switch gpio num is zero.");
            return -EINVAL;
        }
        pdata->switch_gpio_num = count;
       
        ant_switch = (ant_switch_info*)devm_kzalloc(dev, sizeof(ant_switch_info) * count, GFP_KERNEL);
        if (NULL == ant_switch)
        {
            ant_print_err("malloc ant_switch failed.");
            return -EINVAL;
        }
        pdata->ant_switch = ant_switch;
        
        /*遍历切换天线的信息*/
        for_each_child_of_node(child,grdchild)
        {
            ret = of_property_read_string_index(grdchild,"switch_name", 0,
                            &ant_switch->gpio_name);
            if (0 != ret) 
            {
                ant_print_err("ant switch name get failed.");
                return -EINVAL;
            }
            ret  = of_property_read_u32_index(grdchild, "switch_gpio", 0, 
                            &ant_switch->gpio_num);
            
            ret |= of_property_read_u32_index(grdchild, "inner_value", 0, 
                            &ant_switch->inner_value);
            
            if(0 != ret)
            {
                ant_print_err("get switch gpio para err,child[%s],grdchild[%s].",
                              child->name,grdchild->name);
                return -EINVAL;
            }

            ant_switch += 1;
        }
        pdata += 1;
    }
    return 0;
}

/*****************************************************************************
 函 数 名  : antenna_proc_init
 功能描述  : 根据获取到的天线组数，创建相应的proc节点，一组天线一个节点
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 0--成功，-1--失败
 说    明  : 
*****************************************************************************/
static int antenna_proc_init(void)
{
    u32 i = 0;

    ant_dev_info* p_ant_info = &g_ant_info;
    static struct proc_dir_entry *ant_entry = NULL;
    
    if(0 == p_ant_info->ant_num)
    {
        ant_print_err("ant num zero, ERROR");
        return ANTE_ERR;
    }

    for(i = 0; i < p_ant_info->ant_num; i++)
    {
        ant_entry = proc_create(p_ant_info->ant_info[i].proc_name, S_IRUSR, NULL, &ant_operations);
        if(NULL == ant_entry)
        {
            ant_print_err("can't create /proc/%s \n", p_ant_info->ant_info[i].proc_name);
            return - EFAULT;
        }
    }
    
    return 0;
}

#if (FEATURE_ON == MBB_RFFE_ANT_TO_MAIN_EXANT)
/*****************************************************************************
 函 数 名  : antenna_nv_gpio_init
 功能描述  : 获取指定nv50569的配置信息
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 0--成功，-1--失败
 说    明  : 
*****************************************************************************/
static int antenna_nv_gpio_init(void)
{
    int ret = 0;
    /*读取nv50569的值记录下来*/
    ret = bsp_nvm_read(NV_ID_RF_ANTEN_DETECT, (u8*)&g_anten_stru, sizeof(NV_RF_ANTEN_DETECT_GPIO_STRU));
    if (0 != ret)
    {
        ant_print_err("failed anten_gpio read NV=0x%x, ret = %d \n",NV_ID_RF_ANTEN_DETECT, ret);
    }
    return ret;
}
#endif

/*****************************************************************************
 函 数 名  : antenna_gpio_probe
 功能描述  : probe函数，用于初始化一系列参数
 输入参数  : pdev:内核传入参数
 输出参数  : 无
 返 回 值  : 0--成功，-1--失败
 说    明  : 
*****************************************************************************/
static int antenna_gpio_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct device *dev = &pdev->dev;
    
    if(NULL == dev)
    {
        ant_print_err("dev is null.");
        return ANTE_ERR;
    }

    /*初始化dts的数据，填充到全局结构体里*/
    ret = antenna_dts_init(dev);
    if(0 != ret)
    {
        return ANTE_ERR;
    }

    /*创建应用读写proc节点*/
    ret = antenna_proc_init();
    if(0 != ret)
    {
        return ANTE_ERR;
    }

#if (FEATURE_ON == MBB_RFFE_ANT_TO_MAIN_EXANT)
    /*获取指定nv50569的配置信息*/
    ret = antenna_nv_gpio_init();
    if (0 != ret)
    {
        return ANTE_ERR;
    }
#endif

    /*对天线进行相关配置和初始化*/
    ret = antenna_gpio_init();
    if(0 != ret)
    {
        return ANTE_ERR;
    }

    ant_print_err("init success.");
    return ANTE_OK;
}
/*****************************************************************************
 函 数 名  : antenna_gpio_remove
 功能描述  : 本模块退出时的操作
 输入参数  : pdev:内核传入参数
 输出参数  : 无
 返 回 值  : 0--成功，1--失败
 说    明  : 该模块没有退出操作，这里打桩
*****************************************************************************/
static int antenna_gpio_remove(struct platform_device *pdev)
{   
    ant_print_err("exit success\n");
    return ANTE_OK;
}

static struct of_device_id of_gpio_antenna_match[] = {
    {.compatible = "antenna-gpio" },
    {},
};

static struct platform_driver gpio_antenna_driver = {
    .probe  = antenna_gpio_probe,
    .remove = antenna_gpio_remove,
    .driver = {
        .name = "antenna",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(of_gpio_antenna_match),
    }
};

module_platform_driver(gpio_antenna_driver);

MODULE_AUTHOR("MBB.Huawei Device");
MODULE_DESCRIPTION("Gpio Antenna Driver");
MODULE_LICENSE("GPL");

#ifdef __cplusplus
}
#endif

