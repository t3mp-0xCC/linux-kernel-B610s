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

/*Ӳ�������Ķ�ʱ��*/
RFSWITCH_T_CTRL_S g_wdis_ctx = {0};
/*�л���Ƶ״̬ʱ��˯����*/
struct wake_lock rfswitch_pin_lock;  
/*����ģʽ״̬ȫ�ֱ���������Ӳ�����ع�ͬ����*/
unsigned int g_rfswitch_state =0;
/*�¶ȱ���״̬ȫ�ֱ���*/
bool g_therm_rf_off = FALSE;
/*����ģʽ�л���������ȫ�ֱ���*/
rfswitch_trig_type g_rfswitch_trig  = RFSWITCH_NONE_TRIG;


rfswtich_op_func rf_switch_op_end_func[RFSWITCH_NONE_TRIG+1]=
{
    RfSwitch_Swswtich_op_end,  /*RFSWITCH_SW_TRIG*/
    RfSwitch_Hwswtich_op_end,   /*RFSWITCH_HW_TRIG*/
    RfSwitch_Init_op_end,    /*RFSWITCH_INIT_TRIG*/
    NULL     /*RFSWITCH_NONE_TRIG*/
};
/*****************************************************************************
�� �� ��  : RfSwitch_SW_Set
��������  : ����ģʽ�����������
�������  : swstate�������״̬
�������  : ��
�� �� ֵ  : ��
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
�� �� ��  : RfSwitch_SW_Get
��������  : ����ģʽ������ز�ѯ
�������  : ��
�������  : ��
�� �� ֵ  : 0:������ش򿪣��ر���Ƶ
                       1:������عرգ�����Ƶ
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
�� �� ��  : RfSwitch_HW_Get
��������  : ��ȡ����ģʽ����GPIO״̬a�˵���
�������  : ��
�������  : ��
�� �� ֵ  : 0:Ӳ�����ش򿪷���ģʽ���ر���Ƶ
                       1:Ӳ�����عرշ���ģʽ������Ƶ
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
�� �� ��  : Rfswitch_Change_Mode_To_State
��������  : ��������RF��mode���RFSWITCH��state
�������  : current_mode ���õ�ģʽ
�������  : ��
�� �� ֵ  : 0:����������ģʽ���ر���Ƶ
                      1:����˳�����ģʽ������Ƶ
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
�� �� ��  : RfSwitch_State_Set
��������  : ���÷���ģʽ״̬
�������  : rf_state ��Ƶģʽ
�������  : ��
g_rfswitch_state :  0��ʾ����ģʽ�򿪣���Ƶ�ر�
                          1��ʾ����ģʽ�رգ���Ƶ��
�� �� ֵ  : ��
*****************************************************************************/
void 
RfSwitch_State_Set(unsigned int rf_state)
{
    g_rfswitch_state = rf_state;
}
/*****************************************************************************
�� �� ��  : RfSwitch_State_Get
��������  : ��ѯ����ģʽ״̬
�������  : ��
�������  : ��
�� �� ֵ  : g_rfswitch_state :  0��ʾ����ģʽ�򿪣���Ƶ�ر�
                          1��ʾ����ģʽ�رգ���Ƶ��
*****************************************************************************/
unsigned int 
RfSwitch_State_Get(void)
{
    return g_rfswitch_state;
}
/*****************************************************************************
�� �� ��  : RfSwitch_Trig_Type_Set
��������  : ���÷���ģʽ������ʽ
�������  : RFSWITCH_SW_TRIG = 0,
    RFSWITCH_HW_TRIG,
    RFSWITCH_INIT_TRIG,
    RFSWITCH_NONE_TRIG
�������  : ��
�� �� ֵ  : ��
*****************************************************************************/
void 
RfSwitch_Trig_Type_Set(rfswitch_trig_type trig_type)
{
    g_rfswitch_trig = trig_type;
}
/*****************************************************************************
�� �� ��  : RfSwitch_Trig_Type_Get
��������  : ��ѯ����ģʽ������ʽ
�������  : ��
�������  : ��
�� �� ֵ  :     RFSWITCH_SW_TRIG = 0,
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
�� �� ��  : RfSwitch_Swswtich_op_end
��������  : �������ģʽ������
�������  :sw_op_mode �������ģʽ
�������  : ��
�� �� ֵ  : ��
*****************************************************************************/

void 
RfSwitch_Swswtich_op_end(unsigned int sw_op_mode)
{
    unsigned int sw_state = RF_ON;
    unsigned int hw_state = RF_ON;
    unsigned int rf_state = RF_ON;
    /*��ȡ��ʵ����Ӳ������״̬��
        �������״̬����ξ�����
        Ӳ������״̬��GPIO�ܽž���*/
    sw_state = Rfswitch_Change_Mode_To_State(sw_op_mode);
    hw_state = RfSwitch_HW_Get();
    rf_state = (sw_state & hw_state) & RF_ON;
    /*���÷���ģʽ���*/
    RfSwitch_State_Set(rf_state);
    /*�����������ģʽ����NV*/
    RfSwitch_SW_Set(sw_state); 
    /*�����ϱ�*/
    RfSwitch_State_Report(sw_state,hw_state);
#if  (FEATURE_ON == MBB_MODULE_THERMAL_PROTECT)
    /*֪ͨ�¶ȱ���ģ�飬��ǰ����ģʽ��״̬*/
    RfSwitch_State_Info_Therm(rf_state);
#endif

    RfSwitch_Trig_Type_Set(RFSWITCH_NONE_TRIG);
}
/*****************************************************************************
�� �� ��  : RfSwitch_Hwswtich_op_end
��������  : Ӳ������ģʽ�л��ɹ���ĺ����������������ϱ�
�������  : δ��
�������  : ��
�� �� ֵ  : ��
*****************************************************************************/
void 
RfSwitch_Hwswtich_op_end(unsigned int hw_op_mode)
{
    unsigned int sw_state = RF_ON;
    unsigned int hw_state = RF_ON; 
    unsigned int rf_state = RF_ON; 
    /*��ȡ��ʵ����Ӳ������״̬��
        �������״̬��NV������
        Ӳ������״̬��GPIO����*/
    hw_state = RfSwitch_HW_Get();
    sw_state = RfSwitch_SW_Get(); 
    rf_state = (sw_state & hw_state) & RF_ON;
    /*���÷���ģʽ���*/
    RfSwitch_State_Set(rf_state);
    /*�����ϱ�*/	
    RfSwitch_State_Report(sw_state,hw_state); 
    RfSwitch_Trig_Type_Set(RFSWITCH_NONE_TRIG);
}
/*****************************************************************************
�� �� ��  : RfSwitch_Init_op_end
��������  : ����ģʽ��ʼ���ĺ�������
�������  :δ��
�������  : ��
�� �� ֵ  : ��
*****************************************************************************/
void 
RfSwitch_Init_op_end(unsigned int init_op_mode)
{
    unsigned int sw_state = RF_ON; 	
    unsigned int hw_state = RF_ON; 
    unsigned int rf_state = RF_ON; 
    /*��ȡ��ʵ����Ӳ������״̬��
        �������״̬��NV������
        Ӳ������״̬��GPIO�ܽŻ�ȡ*/	
    sw_state = RfSwitch_SW_Get();
    hw_state = RfSwitch_HW_Get();
    rf_state = (sw_state & hw_state) & RF_ON;
    /*���÷���ģʽ���*/	
    RfSwitch_State_Set(rf_state);		
    RfSwitch_Trig_Type_Set(RFSWITCH_NONE_TRIG);	
}
/*****************************************************************************
�� �� ��  : RfSwitch_Hwswitch_handle
��������  : Ӳ������ģʽ������
�������  :current_mode ��ǰģʽ
�������  : ��
�� �� ֵ  : ��
*****************************************************************************/
static irqreturn_t RfSwitch_Hwswitch_handle(int irq, void *dev_id)
{
    /*�ж��Ƿ�ΪW-DISABLE���ж�*/
    if(!gpio_int_state_get(W_DISABLE_PIN))
    {
        printk(KERN_ERR"%s():L%d:gpio_int_state_get error.\n",__FUNCTION__,__LINE__);
        return IRQ_NONE;
    }
    /*���ж�*/
    gpio_int_mask_set(W_DISABLE_PIN);

    //��ʱ��,�������δ����ʱ��ʱ������ɾ��
    (void)bsp_softtimer_delete(&g_wdis_ctx.irq_timer);

    /*��timerǰ�ӳ�ϵͳ ���ߣ���ֹģ������timerʧЧ��ʱ��Ҫ����timer��ʱʱ��*/
    RfSwitch_Trig_Type_Set(RFSWITCH_HW_TRIG);
    wake_lock_timeout(&rfswitch_pin_lock, (long)msecs_to_jiffies(WDIS_N_TIMER_LENGTH+2000)); /*lint !e526 !e628 !e516*/
    bsp_softtimer_add(&g_wdis_ctx.irq_timer);
   
    return IRQ_HANDLED;
}
/*****************************************************************************
�� �� ��  : RfSwitch_wdis_timer_handler
��������  :����ģʽӲ�����ض�ʱ���ص�������
                           ��������Ϣ��rfswtich_task�л���Ƶ
�������  : ��
�������  : ��
�� �� ֵ  : ��
*****************************************************************************/
void RfSwitch_wdis_timer_handler(unsigned int param)
{
    unsigned int sw_state = RF_ON;
    unsigned int hw_state = RF_ON;
    unsigned int rf_state = RF_ON;
    rfswitch_trig_type rfswitch_trig = RfSwitch_Trig_Type_Get();
    /*����������ģʽ״̬*/
    sw_state = RfSwitch_SW_Get();
    /*���Ӳ������ģʽ״̬*/
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
    /*��ʱ��ȡ��ֵ���ϴ�״̬һ���򲻴�����������*/
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
    /*����RF״̬*/
    rf_state = (sw_state & hw_state) & RF_ON;
    /*�л�״̬*/	
    RfSwitch_change(rf_state);  
}
/*****************************************************************************
�� �� ��  : RfSwitch_timer_init
��������  : ����ģʽ��ʱ����ʼ��
�������  : ��
�������  : ��
�� �� ֵ  : ��
*****************************************************************************/
signed int RfSwitch_timer_init(void)
{
    signed int res = ERROR;
    struct softtimer_list *irq_timer = &g_wdis_ctx.irq_timer;
    /*irq_timer����Ӳ�����صĴ���*/	
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
�� �� ��  : RfSwitch_gpio_init
��������  : ����ģʽӲ�����س�ʼ��
�������  : ��
�������  : ��
�� �� ֵ  : ��
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
 �� �� ��  : RfSwitch_icc_init_cb
 ��������  : C�˳�ʼ������A�˷�icc��Ϣ�Ļص�����
 �������  : NULL
 �������  : ��
 �� �� ֵ  : 0:�ɹ�  -1:ʧ��
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
    /*�¶ȱ���Ҫ������Ƶ������ֻ��Ҫ�޸ı�Ǽ���*/
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
    /*���Ӳ������ģʽ״̬*/
    g_wdis_ctx.hw_state = RfSwitch_HW_Get();
    /*����������ģʽ״̬*/
    g_wdis_ctx.sw_state = RfSwitch_SW_Get();
    RfSwitch_Trig_Type_Set(RFSWITCH_INIT_TRIG);
    if (RF_OFF == g_wdis_ctx.sw_state)
    {
        /*����ϵ��ʼ��������������ģʽ����ֱ���л�״̬*/    
        RfSwitch_change(RF_OFF);  
        return 0;
    }

    if (RF_ON == g_wdis_ctx.hw_state)	
    {
        /*����ϵ��ʼ��δ�������ģʽ����ֱ�ӷ���*/    
        RfSwitch_State_Set(RF_ON);	
        RfSwitch_Trig_Type_Set(RFSWITCH_NONE_TRIG);
        return 0;
    }
    //��ʱ��,�������δ����ʱ��ʱ������ɾ��
    (void)bsp_softtimer_delete(&g_wdis_ctx.irq_timer);

    /*��timerǰ�ӳ�ϵͳ ���ߣ���ֹģ������timerʧЧ��ʱ��Ҫ����timer��ʱʱ��*/
    wake_lock_timeout(&rfswitch_pin_lock, (long)msecs_to_jiffies(WDIS_N_TIMER_LENGTH+300)); /*lint !e526 !e628 !e516*/
    bsp_softtimer_add(&g_wdis_ctx.irq_timer);
    return 0;
}


/*****************************************************************************
�� �� ��  : RfSwitch_Init
��������  : ����ģʽ��ʼ������
�������  : ��
�������  : ��
�� �� ֵ  : ��
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

    /* ��CEģ��ų�ʼ����ģ�� */
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
    /*��ʼ��ϵͳ ��*/
    wake_lock_init(&rfswitch_pin_lock, WAKE_LOCK_SUSPEND, WDIS_DRIVER_NAME);
    /*�趨����ģʽ�ܽ��ж�״̬*/
    RfSwitch_gpio_init();
    /*ע��icc�¼���ΪC�˳�ʼ��׼��*/
    rt = bsp_icc_event_register(channel_id, (read_cb_func)RfSwitch_icc_cb, NULL, NULL, NULL);
    if(rt != 0)
    {
        printk("RfSwitch_Init bsp_icc_event_register fail\n");
    }
    return 0 ;
}
module_init(RfSwitch_Init);

