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

extern u8 g_bodysar_gpio_state; /*bodysar��gpio���ص�ƽֵ��1�ߵ�ƽ�ر�bodysar, 0�͵�ƽ��bodysarͬ�⽵����*/
extern u8 g_bodysar_sim_state; /*����bodysar����=2ʱ������sim��ʱ��ֵΪ1ͬ�⽵����*/
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

    /*�жϴ�������*/
    if (g_BodySAR_ctx.gpio_value)
    {
        gpio_int_trigger_set(BodySAR_N, IRQ_TYPE_LEVEL_LOW);
    }
    else
    {
        gpio_int_trigger_set(BodySAR_N, IRQ_TYPE_LEVEL_HIGH);
    }

    /*��ʱ��ƽ���ϴ�һ��������Ϊ��ë�̣���������*/
    if (g_BodySAR_ctx.gpio_value == g_BodySAR_ctx.gpio_value_old)
    {
        printk(KERN_INFO"%s(): two read gpio value is equal,return\n",__FUNCTION__);
        (void)printk(KERN_ERR"BodySAR_on_off not call func\n");
        return;
    }
    else //��Ч�жϣ���������
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
    /*�ж��Ƿ�Ϊbodysar���ж�*/
    if(!gpio_int_state_get(BodySAR_N))
    {
        printk(KERN_ERR"%s():L%d:gpio_int_state_get error.\n",__FUNCTION__,__LINE__);
        return IRQ_NONE;
    }
    gpio_int_mask_set(BodySAR_N);

   
    //��ʱ��,�������δ����ʱ��ʱ������ɾ��
    (void)bsp_softtimer_delete(&g_BodySAR_ctx.irq_timer);

    /*��timerǰ�ӳ�ϵͳ ���ߣ���ֹģ������timerʧЧ��ʱ��Ҫ����timer��ʱʱ��*/
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
    
    /*����GPIOΪ����*/
    gpio_direction_input(BodySAR_N);
    
    gpio_int_mask_set(BodySAR_N);  
    /*��ʼ����жϱ�־λ*/
    gpio_int_state_clear(BodySAR_N); 

    /*����GPIO�����жϴ�����ʽ*/
    g_BodySAR_ctx.gpio_value = gpio_get_value(BodySAR_N);
    g_bodysar_gpio_state = g_BodySAR_ctx.gpio_value;

    if (g_BodySAR_ctx.gpio_value)
    {
         /*Ĭ��״̬�������账�� */
        gpio_int_trigger_set(BodySAR_N, IRQ_TYPE_LEVEL_LOW);
        //gpio_int_trigger_set(BodySAR_N, IRQ_TYPE_EDGE_FALLING);
        g_BodySAR_ctx.gpio_value_old = g_BodySAR_ctx.gpio_value;
    }
    else
    {
        gpio_int_trigger_set(BodySAR_N, IRQ_TYPE_LEVEL_HIGH);
        //gpio_int_trigger_set(BodySAR_N, IRQ_TYPE_EDGE_RISING);
        g_BodySAR_ctx.gpio_value_old = ~g_BodySAR_ctx.gpio_value;
        //ͨ����ʱ������body sar on �ӿ�
        bsp_softtimer_add(&g_BodySAR_ctx.irq_timer);
    }

    /*����GPIOΪ�жϷ�ʽ*/
    gpio_set_function(BodySAR_N, GPIO_INTERRUPT);

    /*�����жϺŶ�Ӧ�Ļص�����*/
    irq_id = gpio_to_irq(BodySAR_N);
    ret = request_irq(irq_id, gpio_BodySAR_irq, IRQF_SHARED,"BodySAR_N", (void *)&g_BodySAR_ctx);
    if (ret != 0) 
    {
        printk(KERN_ERR"failed at register gpio-oob irq\n");
        gpio_free(BodySAR_N);
        return ERROR;
    }

    /*��ʼ����жϱ�־λ*/
    gpio_int_state_clear(BodySAR_N);
    /*ʹ��GPIO�ж�*/
    gpio_int_unmask_set(BodySAR_N);

    /*��ʼ��ϵͳ ��*/
    wake_lock_init(&bodysar_pin_lock, WAKE_LOCK_SUSPEND, "bodysar_pin");

    return OK;
}

void  sarreductoin_nv_update(void)
{
    /*lte  sarreduction nvˢ��*/
    (void)update_lte_sarreduction_nv_by_bodysar_nv();
    
    /*wcdma  sarreduction nvˢ��*/
    (void)update_wcdma_sarreduction_nv_by_bodysar_nv();
    
    /*gsm  sarreduction nvˢ��*/
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
        /*����ģʽ������nvģ�������*/
        printk("entry update not init bodysar gpio !\n");
        return ERROR;  
    }
#endif /*MBB_DLOAD*/

    /* ��CEģ��֧�ָù��� */
    module_type = mdrv_ver_get_solution_type();
    if (MDRV_VER_TYPE_CE != module_type)
    {
        printk(KERN_ERR"%s(): This board type not CE. \n",__FUNCTION__);
        return ERROR;
    }

    /*sarreduction �ײ㹦�ʿ���NV ˢ��*/
    sarreductoin_nv_update();

    /*bodysarӲ���жϳ�ʼ��*/
    if (OK != BodySAR_switch_gpio_init())
    {
        return ERROR;
    }
    return OK;
}

#if (FEATURE_ON == MBB_MULTI_CARRIER)
/*****************************************************************************
 �� �� ��  : bodysar_PlmnListCheck
 ��������  : ��鵱ǰ�����sim����plmn�Ƿ���bodysar��plmn-list��
 �������  : char *sim_plmn  ��sim���л�õ�plmn
 �������  : ��
 �� �� ֵ  : TRUE:ƥ��ɹ�
                        FALSE:ƥ��ʧ��
 ���ú���  :
 ��������  :
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

    /*���һ���ļ��Ƿ����*/
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
    /*���ļ��ж�����*/
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
    /*����xml�ļ��������ļ����Ƿ����sim_plmn*/
    parse_res = huawei_mtc_check_sim_plmn(data_start,data_end,sim_plmn);

    kfree(data_start);
    data_start = NULL;
    return parse_res;
}
/*****************************************************************************
 �� �� ��  : bodysar_powr_limit_by_plmnlist_check
 ��������  : ��鵱ǰ�����sim����plmn�Ƿ���bodysar��plmn-list,
                            ����ڣ����н�����
 �������  : char *sim_plmn  ��sim���л�õ�plmn
 �������  : ��
 �� �� ֵ  : TRUE:ƥ��ɹ�
                         FALSE:ƥ��ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
void bodysar_powr_limit_by_plmnlist_check(char *sim_plmn)
{
    if (TRUE == bodysar_PlmnListCheck(sim_plmn))
    {
        g_bodysar_sim_state = 1; /*��ǰ��plmnƥ������plmnlist,Ҫ������*/
    }
    else
    {
        g_bodysar_sim_state = 0; /*��ǰ��plmnû��ƥ����plmnlist,��������*/
    }
    
    printk(KERN_ERR"wk:g_bodysar_sim_state=%d\r\n",g_bodysar_sim_state);

    /*���ʿ���*/
    (void)bodysar_power_control(0);
}
#endif
module_init(BodySAR_init);
