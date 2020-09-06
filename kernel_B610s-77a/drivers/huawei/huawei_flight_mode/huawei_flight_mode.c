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



#include "huawei_flight_mode.h"

/*硬件防抖的定时器*/
RFSWITCH_T_CTRL_S g_wdis_ctx = {0};
/*切换射频状态时的睡眠锁*/
struct wake_lock rfswitch_pin_lock;  
/*飞行模式状态全局变量，由软硬件开关共同决定*/
unsigned int g_rfswitch_state =0;
/*温度保护状态全局变量*/
bool g_therm_rf_off = FALSE;
/*飞行模式切换触发类型全局变量*/
rfswitch_trig_type g_rfswitch_trig  = RFSWITCH_NONE_TRIG;


rfswtich_op_func rf_switch_op_end_func[RFSWITCH_NONE_TRIG+1]=
{
    RfSwitch_Swswtich_op_end,  /*RFSWITCH_SW_TRIG*/
    RfSwitch_Hwswtich_op_end,   /*RFSWITCH_HW_TRIG*/
    RfSwitch_Init_op_end,    /*RFSWITCH_INIT_TRIG*/
    NULL     /*RFSWITCH_NONE_TRIG*/
};
/*****************************************************************************
函 数 名  : RfSwitch_SW_Set
功能描述  : 飞行模式软件开关设置
输入参数  : swstate软件开关状态
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void RfSwitch_SW_Set(unsigned int swstate)
{
    FLIGHT_MODE_STRU  swswitchPara;     
    swswitchPara.huawei_flight_mode = swstate;
    (void)bsp_nvm_write(NV_ID_FLIGHT_MODE,
                                     (unsigned char *)&(swswitchPara),
                                     sizeof(swswitchPara));
}
/*****************************************************************************
函 数 名  : RfSwitch_SW_Get
功能描述  : 飞行模式软件开关查询
输入参数  : 无
输出参数  : 无
返 回 值  : 0:软件开关打开，关闭射频
                       1:软件开关关闭，打开射频
*****************************************************************************/
unsigned int RfSwitch_SW_Get(void)
{
    unsigned int ret = 0;
    FLIGHT_MODE_STRU  swswitchPara;
    memset(&swswitchPara,0,sizeof(FLIGHT_MODE_STRU));
    ret = bsp_nvm_read(NV_ID_FLIGHT_MODE,
                                      (unsigned char *)&(swswitchPara),
                                      sizeof(swswitchPara));
    return swswitchPara.huawei_flight_mode;
}
/*****************************************************************************
函 数 名  : RfSwitch_HW_Get
功能描述  : 获取飞行模式开关GPIO状态a核调用
输入参数  : 无
输出参数  : 无
返 回 值  : 0:硬件开关打开飞行模式，关闭射频
                       1:硬件开关关闭飞行模式，打开射频
*****************************************************************************/

unsigned int RfSwitch_HW_Get(void)
{
    int value = 1;
    value = gpio_get_value(W_DISABLE_PIN);
    if ( 0 == value )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
/*****************************************************************************
函 数 名  : Rfswitch_Change_Mode_To_State
功能描述  : 根据设置RF的mode获得RFSWITCH的state
输入参数  : current_mode 设置的模式
输出参数  : 无
返 回 值  : 0:软件进入飞行模式，关闭射频
                      1:软件退出飞行模式，打开射频
*****************************************************************************/
unsigned int
Rfswitch_Change_Mode_To_State
(
unsigned int current_mode
)
{
    unsigned int temp_state = RF_ON;
    if (1 ==current_mode)
    {
        temp_state = RF_ON;
    }
    else
    {
        temp_state = RF_OFF;
    }
    return temp_state;
}
/*****************************************************************************
函 数 名  : RfSwitch_State_Set
功能描述  : 设置飞行模式状态
输入参数  : rf_state 射频模式
输出参数  : 无
g_rfswitch_state :  0表示飞行模式打开，射频关闭
                          1表示飞行模式关闭，射频打开
返 回 值  : 无
*****************************************************************************/
void 
RfSwitch_State_Set(unsigned int rf_state)
{
    g_rfswitch_state = rf_state;
}
/*****************************************************************************
函 数 名  : RfSwitch_State_Get
功能描述  : 查询飞行模式状态
输入参数  : 无
输出参数  : 无
返 回 值  : g_rfswitch_state :  0表示飞行模式打开，射频关闭
                          1表示飞行模式关闭，射频打开
*****************************************************************************/
unsigned int 
RfSwitch_State_Get(void)
{
    return g_rfswitch_state;
}
/*****************************************************************************
函 数 名  : RfSwitch_Trig_Type_Set
功能描述  : 设置飞行模式触发方式
输入参数  : RFSWITCH_SW_TRIG = 0,
    RFSWITCH_HW_TRIG,
    RFSWITCH_INIT_TRIG,
    RFSWITCH_NONE_TRIG
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void 
RfSwitch_Trig_Type_Set(rfswitch_trig_type trig_type)
{
    g_rfswitch_trig = trig_type;
}
/*****************************************************************************
函 数 名  : RfSwitch_Trig_Type_Get
功能描述  : 查询飞行模式触发方式
输入参数  : 无
输出参数  : 无
返 回 值  :     RFSWITCH_SW_TRIG = 0,
    RFSWITCH_HW_TRIG,
    RFSWITCH_INIT_TRIG,
    RFSWITCH_NONE_TRIG
*****************************************************************************/
rfswitch_trig_type 
RfSwitch_Trig_Type_Get(void)
{
    return g_rfswitch_trig;
}
/*****************************************************************************
函 数 名  : RfSwitch_Swswtich_op_end
功能描述  : 软件飞行模式处理函数
输入参数  :sw_op_mode 软件操作模式
输出参数  : 无
返 回 值  : 无
*****************************************************************************/

void 
RfSwitch_Swswtich_op_end(unsigned int sw_op_mode)
{
    unsigned int sw_state = RF_ON;
    unsigned int hw_state = RF_ON;
    unsigned int rf_state = RF_ON;
    /*获取真实的软硬件开关状态，
        软件开关状态由入参决定，
        硬件开关状态由GPIO管脚决定*/
    sw_state = Rfswitch_Change_Mode_To_State(sw_op_mode);
    hw_state = RfSwitch_HW_Get();
    rf_state = (sw_state & hw_state) & RF_ON;
    /*设置飞行模式标记*/
    RfSwitch_State_Set(rf_state);
    /*设置软件飞行模式开关NV*/
    RfSwitch_SW_Set(sw_state); 
    /*主动上报*/
    RfSwitch_State_Report(sw_state,hw_state);
#if  (FEATURE_ON == MBB_MODULE_THERMAL_PROTECT)
    /*通知温度保护模块，当前飞行模式的状态*/
    RfSwitch_State_Info_Therm(rf_state);
#endif

    RfSwitch_Trig_Type_Set(RFSWITCH_NONE_TRIG);
}
/*****************************************************************************
函 数 名  : RfSwitch_Hwswtich_op_end
功能描述  : 硬件飞行模式切换成功后的后续处理，用来主动上报
输入参数  : 未用
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void 
RfSwitch_Hwswtich_op_end(unsigned int hw_op_mode)
{
    unsigned int sw_state = RF_ON;
    unsigned int hw_state = RF_ON; 
    unsigned int rf_state = RF_ON; 
    /*获取真实的软硬件开关状态，
        软件开关状态由NV决定，
        硬件开关状态由GPIO决定*/
    hw_state = RfSwitch_HW_Get();
    sw_state = RfSwitch_SW_Get(); 
    rf_state = (sw_state & hw_state) & RF_ON;
    /*设置飞行模式标记*/
    RfSwitch_State_Set(rf_state);
    /*主动上报*/	
    RfSwitch_State_Report(sw_state,hw_state); 
    RfSwitch_Trig_Type_Set(RFSWITCH_NONE_TRIG);
}
/*****************************************************************************
函 数 名  : RfSwitch_Init_op_end
功能描述  : 飞行模式初始化的后续处理
输入参数  :未用
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void 
RfSwitch_Init_op_end(unsigned int init_op_mode)
{
    unsigned int sw_state = RF_ON; 	
    unsigned int hw_state = RF_ON; 
    unsigned int rf_state = RF_ON; 
    /*获取真实的软硬件开关状态，
        软件开关状态由NV决定，
        硬件开关状态由GPIO管脚获取*/	
    sw_state = RfSwitch_SW_Get();
    hw_state = RfSwitch_HW_Get();
    rf_state = (sw_state & hw_state) & RF_ON;
    /*设置飞行模式标记*/	
    RfSwitch_State_Set(rf_state);		
    RfSwitch_Trig_Type_Set(RFSWITCH_NONE_TRIG);	
}
/*****************************************************************************
函 数 名  : RfSwitch_Hwswitch_handle
功能描述  : 硬件飞行模式处理函数
输入参数  :current_mode 当前模式
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
static irqreturn_t RfSwitch_Hwswitch_handle(int irq, void *dev_id)
{
    /*判断是否为W-DISABLE的中断*/
    if(!gpio_int_state_get(W_DISABLE_PIN))
    {
        printk(KERN_ERR"%s():L%d:gpio_int_state_get error.\n",__FUNCTION__,__LINE__);
        return IRQ_NONE;
    }
    /*锁中断*/
    gpio_int_mask_set(W_DISABLE_PIN);

    //起定时器,如果存在未到超时定时器，先删除
    (void)bsp_softtimer_delete(&g_wdis_ctx.irq_timer);

    /*起timer前延迟系统 休眠，防止模块休眠timer失效，时间要大于timer定时时间*/
    RfSwitch_Trig_Type_Set(RFSWITCH_HW_TRIG);
    wake_lock_timeout(&rfswitch_pin_lock, (long)msecs_to_jiffies(WDIS_N_TIMER_LENGTH+2000)); /*lint !e526 !e628 !e516*/
    bsp_softtimer_add(&g_wdis_ctx.irq_timer);
   
    return IRQ_HANDLED;
}
/*****************************************************************************
函 数 名  : RfSwitch_wdis_timer_handler
功能描述  :飞行模式硬件开关定时器回调函数，
                           用来发消息给rfswtich_task切换射频
输入参数  : 无
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void RfSwitch_wdis_timer_handler(unsigned int param)
{
    unsigned int sw_state = RF_ON;
    unsigned int hw_state = RF_ON;
    unsigned int rf_state = RF_ON;
    rfswitch_trig_type rfswitch_trig = RfSwitch_Trig_Type_Get();
    /*获得软件飞行模式状态*/
    sw_state = RfSwitch_SW_Get();
    /*获得硬件飞行模式状态*/
    hw_state = RfSwitch_HW_Get();
    printk("rfswitch hwstate %d, swstate is %d\r\n",hw_state,sw_state);
    if(hw_state)
    {
        gpio_int_trigger_set(W_DISABLE_PIN , IRQ_TYPE_LEVEL_LOW);
    }
    else
    {
        gpio_int_trigger_set(W_DISABLE_PIN , IRQ_TYPE_LEVEL_HIGH); 
    }
    gpio_int_state_clear(W_DISABLE_PIN);
    gpio_int_unmask_set(W_DISABLE_PIN);
    /*此时读取的值与上次状态一样则不处理，防抖处理*/
    if  (RFSWITCH_HW_TRIG == rfswitch_trig)
    {
        if (hw_state == g_wdis_ctx.hw_state)
        {
            printk("two read gpio value is the same,first is %d,second is %d\n",g_wdis_ctx.hw_state,hw_state);
            RfSwitch_Trig_Type_Set(RFSWITCH_NONE_TRIG);
            return ;
        }
    }
    g_wdis_ctx.hw_state = hw_state;
    /*设置RF状态*/
    rf_state = (sw_state & hw_state) & RF_ON;
    /*切换状态*/	
    RfSwitch_change(rf_state);  
}
/*****************************************************************************
函 数 名  : RfSwitch_timer_init
功能描述  : 飞行模式定时器初始化
输入参数  : 无
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
signed int RfSwitch_timer_init(void)
{
    signed int res = ERROR;
    struct softtimer_list *irq_timer = &g_wdis_ctx.irq_timer;
    /*irq_timer用来硬件开关的处理*/	
    irq_timer->func = RfSwitch_wdis_timer_handler;
    irq_timer->para = (u32)&g_wdis_ctx;
    irq_timer->timeout = WDIS_N_TIMER_LENGTH;
    irq_timer->wake_type = SOFTTIMER_WAKE;
    res = bsp_softtimer_create(irq_timer);

    if(OK != res)
    {
        printk(KERN_ERR"statethree_timer create fail\n");
        return ERROR;
    }
    return OK;
}

/*****************************************************************************
函 数 名  : RfSwitch_gpio_init
功能描述  : 飞行模式硬件开关初始化
输入参数  : 无
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
void RfSwitch_gpio_init(void)
{
    int value = 0;

    if(gpio_request(W_DISABLE_PIN, WDIS_DRIVER_NAME))
    {
        printk("gpio_request W_DISABLE_PIN  fail \n");
        return;
    }
    gpio_direction_input(W_DISABLE_PIN);
    gpio_int_mask_set(W_DISABLE_PIN);

    value = RfSwitch_HW_Get();
    if (value)
    {
        gpio_int_trigger_set(W_DISABLE_PIN, IRQ_TYPE_LEVEL_LOW);
    }
    else
    {
        gpio_int_trigger_set(W_DISABLE_PIN, IRQ_TYPE_LEVEL_HIGH);
    }
    gpio_set_function(W_DISABLE_PIN, GPIO_INTERRUPT);
    if(request_irq(gpio_to_irq(W_DISABLE_PIN), RfSwitch_Hwswitch_handle, IRQF_SHARED, WDIS_DRIVER_NAME, &value))
    {
        printk("RfSwitch_gpio_init request_irq  fail \n");
        gpio_free(W_DISABLE_PIN);
        return;		
    }
    gpio_int_state_clear(W_DISABLE_PIN);
    gpio_int_unmask_set(W_DISABLE_PIN);

}
/*****************************************************************************
 函 数 名  : RfSwitch_icc_init_cb
 功能描述  : C核初始化后向A核发icc消息的回调函数
 输入参数  : NULL
 输出参数  : 无
 返 回 值  : 0:成功  -1:失败
*****************************************************************************/
signed int RfSwitch_icc_cb(void)
{
    int read_len = 0;
    unsigned int rf_init = (unsigned int)RFSWITCH_RF_NONE;
    unsigned int channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_RF_INIT;
    read_len = bsp_icc_read(channel_id, (unsigned char*)&rf_init, sizeof(rf_init));
    if ( sizeof(rf_init) != read_len )
    {
        printk("RfSwitch_icc_cb bsp_icc_read fail\r\n");
        return -1;
    }
    /*温度保护要开关射频，这里只需要修改标记即可*/
    if (RFSWITCH_RF_OFF == (rfswitch_ccore_op_type)rf_init)
    {
        g_therm_rf_off = TRUE;
        return 0;
    }
    else if (RFSWITCH_RF_ON == (rfswitch_ccore_op_type)rf_init)
    {
        g_therm_rf_off = FALSE;
        return 0;
    }
    else if(RFSWITCH_RF_NONE == (rfswitch_ccore_op_type)rf_init)
    {
        printk("RfSwitch_icc_cb rf_init is RFSWITCH_RF_NONE\r\n");
        return -1;
    }
    /*获得硬件飞行模式状态*/
    g_wdis_ctx.hw_state = RfSwitch_HW_Get();
    /*获得软件飞行模式状态*/
    g_wdis_ctx.sw_state = RfSwitch_SW_Get();
    RfSwitch_Trig_Type_Set(RFSWITCH_INIT_TRIG);
    if (RF_OFF == g_wdis_ctx.sw_state)
    {
        /*如果上电初始化是软件进入飞行模式，则直接切换状态*/    
        RfSwitch_change(RF_OFF);  
        return 0;
    }

    if (RF_ON == g_wdis_ctx.hw_state)	
    {
        /*如果上电初始化未进入飞行模式，则直接返回*/    
        RfSwitch_State_Set(RF_ON);	
        RfSwitch_Trig_Type_Set(RFSWITCH_NONE_TRIG);
        return 0;
    }
    //起定时器,如果存在未到超时定时器，先删除
    (void)bsp_softtimer_delete(&g_wdis_ctx.irq_timer);

    /*起timer前延迟系统 休眠，防止模块休眠timer失效，时间要大于timer定时时间*/
    wake_lock_timeout(&rfswitch_pin_lock, (long)msecs_to_jiffies(WDIS_N_TIMER_LENGTH+300)); /*lint !e526 !e628 !e516*/
    bsp_softtimer_add(&g_wdis_ctx.irq_timer);
    return 0;
}


/*****************************************************************************
函 数 名  : RfSwitch_Init
功能描述  : 飞行模式初始化函数
输入参数  : 无
输出参数  : 无
返 回 值  : 无
*****************************************************************************/
static int __init  RfSwitch_Init(void)
{
    int rt=-1;
    MDRV_VER_SOLUTION_TYPE module_type = MDRV_VER_TYPE_M2M;
    unsigned int channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_RF_INIT;
#if (FEATURE_ON == MBB_DLOAD)
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        return rt;
    }
    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        return rt;
    }
#endif /*MBB_DLOAD*/

    /* 仅CE模块才初始化该模块 */
    module_type = mdrv_ver_get_solution_type();
    if (MDRV_VER_TYPE_CE != module_type)
    {
        printk("RfSwitch module will not init!");
        return rt;
    }

    if (RfSwitch_timer_init())
    {
        printk(KERN_ERR"%s(): timer init fail. \n",__FUNCTION__);
        return rt;
    }
    /*初始化系统 锁*/
    wake_lock_init(&rfswitch_pin_lock, WAKE_LOCK_SUSPEND, WDIS_DRIVER_NAME);
    /*设定飞行模式管脚中断状态*/
    RfSwitch_gpio_init();
    /*注册icc事件，为C核初始化准备*/
    rt = bsp_icc_event_register(channel_id, (read_cb_func)RfSwitch_icc_cb, NULL, NULL, NULL);
    if(rt != 0)
    {
        printk("RfSwitch_Init bsp_icc_event_register fail\n");
    }
    return 0 ;
}
module_init(RfSwitch_Init);

