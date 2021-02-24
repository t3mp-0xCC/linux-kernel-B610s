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

#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <huawei_bodysar.h>
#include "mdrv_version.h"
#include "bsp_sram.h"
#include "bsp_rfile.h"
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

extern u8 g_bodysar_gpio_state; /*bodysar的gpio开关电平值，1高电平关闭bodysar, 0低电平打开bodysar同意降功率*/
extern u8 g_bodysar_sim_state; /*整机bodysar开关=2时，北美sim卡时该值为1同意降功率*/
extern u32 bodysar_power_control(u8 ucIndex);
extern u32 update_lte_sarreduction_nv_by_bodysar_nv(void);
extern u32 update_wcdma_sarreduction_nv_by_bodysar_nv(void);
extern u32 update_gsm_sarreduction_nv_by_bodysar_nv(void);
extern bool huawei_mtc_check_sim_plmn(char *data_start, char * data_end,char* sim_plmn);

BODYSART_CTRL_S g_BodySAR_ctx = {0}; 
struct wake_lock bodysar_pin_lock;  

void BodySAR_on_off(void)
{
    g_BodySAR_ctx.gpio_value = gpio_get_value(BodySAR_N);

    /*中断触发配置*/
    if (g_BodySAR_ctx.gpio_value)
    {
        gpio_int_trigger_set(BodySAR_N, IRQ_TYPE_LEVEL_LOW);
    }
    else
    {
        gpio_int_trigger_set(BodySAR_N, IRQ_TYPE_LEVEL_HIGH);
    }

    /*此时电平和上次一样，则认为是毛刺，防抖处理*/
    if (g_BodySAR_ctx.gpio_value == g_BodySAR_ctx.gpio_value_old)
    {
        printk(KERN_INFO"%s(): two read gpio value is equal,return\n",__FUNCTION__);
        (void)printk(KERN_ERR"BodySAR_on_off not call func\n");
        return;
    }
    else //有效中断，正常处理
    {
        (void)printk(KERN_ERR"BodySAR_on_off call func(%d)\n",g_BodySAR_ctx.gpio_value);
        g_bodysar_gpio_state = g_BodySAR_ctx.gpio_value;
        (void)bodysar_power_control(0);
    }

    g_BodySAR_ctx.gpio_value_old = g_BodySAR_ctx.gpio_value;

    gpio_int_state_clear(BodySAR_N);
    gpio_int_unmask_set(BodySAR_N);

    return;
}



s32 BodySAR_timer_init(void)
{
    struct softtimer_list *irq_timer = &g_BodySAR_ctx.irq_timer;
    irq_timer->func = BodySAR_on_off;
    irq_timer->para = (u32)&g_BodySAR_ctx;
    irq_timer->timeout = BodySAR_N_TIMER_LENGTH;
    irq_timer->wake_type = SOFTTIMER_WAKE;

    if(OK != bsp_softtimer_create(irq_timer))
    {
        printk(KERN_ERR"statethree_timer create fail\n");
        return ERROR;
    }
    return OK;
}


irqreturn_t gpio_BodySAR_irq(int irq, void *ctx)
{
    /*判断是否为bodysar的中断*/
    if(!gpio_int_state_get(BodySAR_N))
    {
        printk(KERN_ERR"%s():L%d:gpio_int_state_get error.\n",__FUNCTION__,__LINE__);
        return IRQ_NONE;
    }
    gpio_int_mask_set(BodySAR_N);

   
    //起定时器,如果存在未到超时定时器，先删除
    (void)bsp_softtimer_delete(&g_BodySAR_ctx.irq_timer);

    /*起timer前延迟系统 休眠，防止模块休眠timer失效，时间要大于timer定时时间*/
    wake_lock_timeout(&bodysar_pin_lock, (long)msecs_to_jiffies(BodySAR_N_TIMER_LENGTH+300)); /*lint !e526 !e628 !e516*/
    bsp_softtimer_add(&g_BodySAR_ctx.irq_timer);
   
    return IRQ_HANDLED;
}


int BodySAR_switch_gpio_init(void)
{
    s32 ret;
    s32 irq_id = 0;
    u32 dev = 0;

    printk("%s(): enter\n",__FUNCTION__);

    if (BodySAR_timer_init())
    {
        printk(KERN_ERR"%s(): timer init fail. \n",__FUNCTION__);
        return ERROR;
    }

    if (gpio_request(BodySAR_N, "BodySAR_N"))
    {
        printk(KERN_ERR"BodySAR_N gpio=%d is busy.\n", BodySAR_N);
        return ERROR;
    }
    
    /*设置GPIO为输入*/
    gpio_direction_input(BodySAR_N);
    
    gpio_int_mask_set(BodySAR_N);  
    /*初始清除中断标志位*/
    gpio_int_state_clear(BodySAR_N); 

    /*设置GPIO输入中断触发方式*/
    g_BodySAR_ctx.gpio_value = gpio_get_value(BodySAR_N);
    g_bodysar_gpio_state = g_BodySAR_ctx.gpio_value;

    if (g_BodySAR_ctx.gpio_value)
    {
         /*默认状态开机无需处理 */
        gpio_int_trigger_set(BodySAR_N, IRQ_TYPE_LEVEL_LOW);
        //gpio_int_trigger_set(BodySAR_N, IRQ_TYPE_EDGE_FALLING);
        g_BodySAR_ctx.gpio_value_old = g_BodySAR_ctx.gpio_value;
    }
    else
    {
        gpio_int_trigger_set(BodySAR_N, IRQ_TYPE_LEVEL_HIGH);
        //gpio_int_trigger_set(BodySAR_N, IRQ_TYPE_EDGE_RISING);
        g_BodySAR_ctx.gpio_value_old = ~g_BodySAR_ctx.gpio_value;
        //通过定时器调用body sar on 接口
        bsp_softtimer_add(&g_BodySAR_ctx.irq_timer);
    }

    /*设置GPIO为中断方式*/
    gpio_set_function(BodySAR_N, GPIO_INTERRUPT);

    /*挂载中断号对应的回调函数*/
    irq_id = gpio_to_irq(BodySAR_N);
    ret = request_irq(irq_id, gpio_BodySAR_irq, IRQF_SHARED,"BodySAR_N", (void *)&g_BodySAR_ctx);
    if (ret != 0) 
    {
        printk(KERN_ERR"failed at register gpio-oob irq\n");
        gpio_free(BodySAR_N);
        return ERROR;
    }

    /*初始清除中断标志位*/
    gpio_int_state_clear(BodySAR_N);
    /*使能GPIO中断*/
    gpio_int_unmask_set(BodySAR_N);

    /*初始化系统 锁*/
    wake_lock_init(&bodysar_pin_lock, WAKE_LOCK_SUSPEND, "bodysar_pin");

    return OK;
}

void  sarreductoin_nv_update(void)
{
    /*lte  sarreduction nv刷新*/
    (void)update_lte_sarreduction_nv_by_bodysar_nv();
    
    /*wcdma  sarreduction nv刷新*/
    (void)update_wcdma_sarreduction_nv_by_bodysar_nv();
    
    /*gsm  sarreduction nv刷新*/
    (void)update_gsm_sarreduction_nv_by_bodysar_nv();
}
int BodySAR_init(void)
{
    MDRV_VER_SOLUTION_TYPE module_type = MDRV_VER_TYPE_INVALID;

#if (FEATURE_ON == MBB_DLOAD)
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        printk("nv_file_init: smem_data is NULL \n");
        return ERROR;  
    }

    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        /*升级模式，屏蔽nv模块的启动*/
        printk("entry update not init bodysar gpio !\n");
        return ERROR;  
    }
#endif /*MBB_DLOAD*/

    /* 仅CE模块支持该功能 */
    module_type = mdrv_ver_get_solution_type();
    if (MDRV_VER_TYPE_CE != module_type)
    {
        printk(KERN_ERR"%s(): This board type not CE. \n",__FUNCTION__);
        return ERROR;
    }

    /*sarreduction 底层功率控制NV 刷新*/
    sarreductoin_nv_update();

    /*bodysar硬件中断初始化*/
    if (OK != BodySAR_switch_gpio_init())
    {
        return ERROR;
    }
    return OK;
}

#if (FEATURE_ON == MBB_MULTI_CARRIER)
/*****************************************************************************
 函 数 名  : bodysar_PlmnListCheck
 功能描述  : 检查当前插入的sim卡的plmn是否在bodysar的plmn-list里
 输入参数  : char *sim_plmn  从sim卡中获得的plmn
 输出参数  : 无
 返 回 值  : TRUE:匹配成功
                        FALSE:匹配失败
 调用函数  :
 被调函数  :
*****************************************************************************/
bool bodysar_PlmnListCheck (char *sim_plmn)
{
    int fd = -1;
    struct kstat  fd_stat;
    char *data_start=NULL;
    char *data_end=NULL;
    bool parse_res = FALSE;

    if (NULL == sim_plmn)
    {
        printk(KERN_ERR,"bodysar_PlmnListCheck param is error");
        return FALSE;
    }

    /*检查一下文件是否存在*/
    if( 0 != bsp_stat(MTC_BODYSAR_PLMN_LIST_NAME, &fd_stat))
    {
        printk(KERN_ERR,"bodysar_PlmnListCheck  does not exist \n");
        return FALSE;
    }
    fd = bsp_open(MTC_BODYSAR_PLMN_LIST_NAME, O_RDONLY , S_IRUSR | S_IWUSR);
    if (-1 == fd)
    {
        printk(KERN_ERR,"bodysar_PlmnListCheck  open fail \n");
        return FALSE;
    }

    data_start =(char *)kmalloc((size_t)fd_stat.size,GFP_KERNEL);
    if (NULL == data_start)
    {
        printk(KERN_ERR," mtc malloc  fail \n");
        (void)bsp_close(fd);
        return FALSE;
    }
    /*从文件中读数据*/
    if (fd_stat.size != bsp_read(fd, data_start, (u32)fd_stat.size))
    {
        (void)bsp_close(fd);
        kfree(data_start);
        data_start = NULL;
        printk(KERN_ERR," mtc bsp_read  fail \n");
        return FALSE;
    }
    (void)bsp_close(fd);
    data_end = 	data_start+fd_stat.size;
    /*解析xml文件，查找文件中是否包含sim_plmn*/
    parse_res = huawei_mtc_check_sim_plmn(data_start,data_end,sim_plmn);

    kfree(data_start);
    data_start = NULL;
    return parse_res;
}
/*****************************************************************************
 函 数 名  : bodysar_powr_limit_by_plmnlist_check
 功能描述  : 检查当前插入的sim卡的plmn是否在bodysar的plmn-list,
                            如果在，进行降功率
 输入参数  : char *sim_plmn  从sim卡中获得的plmn
 输出参数  : 无
 返 回 值  : TRUE:匹配成功
                         FALSE:匹配失败
 调用函数  :
 被调函数  :
*****************************************************************************/
void bodysar_powr_limit_by_plmnlist_check(char *sim_plmn)
{
    if (TRUE == bodysar_PlmnListCheck(sim_plmn))
    {
        g_bodysar_sim_state = 1; /*当前卡plmn匹配上了plmnlist,要降功率*/
    }
    else
    {
        g_bodysar_sim_state = 0; /*当前卡plmn没有匹配上plmnlist,不降功率*/
    }
    
    printk(KERN_ERR"wk:g_bodysar_sim_state=%d\r\n",g_bodysar_sim_state);

    /*功率控制*/
    (void)bodysar_power_control(0);
}
#endif
module_init(BodySAR_init);
