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

#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/syscalls.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/wakelock.h>

#include "bsp_pmu.h"
#include "bsp_version.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (FEATURE_ON == MBB_FEATURE_GPS)

/******************************************************************************
  1 ģ��˽�� (�ꡢö�١��ṹ�塢�Զ�����������) ������:
******************************************************************************/
#define GPS_PRINT(level, fmt, ...) (printk(level"[*GPS*] %s:%d: "fmt"\n", \
    __func__, __LINE__, ##__VA_ARGS__))

#define GPS_VOLTAGE_UV          (1800 * 1000)       //GPS��Դ1.8V
#define GPS_DISABLE_PIN         GPIO_0_4
#define LABEL_GPS_DISABLE       "gps_disable"
#define PM_GPS_MSG_TIME         (1000)
extern void ATNL_StopGPSCB( void );

/******************************************************************************
  2 ȫ�ֱ���������:
******************************************************************************/
static struct regulator *gps_vcc = NULL;
static struct wake_lock  gps_lock;
static struct wake_lock  gps_msg_lock;

#define GPS_CMD_BASE 0X88
    /*GPS��Դ����*/
#define GPS_POWER_CTRL_CMD                _IOWR(GPS_CMD_BASE, 0, unsigned long)
    /*GPS 32kʱ�ӿ���*/
#define GPS_CLOCK_CTRL_CMD                _IOWR(GPS_CMD_BASE, 1, unsigned long)
    /*GPS��ѯDisable�ܽ�״̬*/
#define GPS_DISABLE_QUY_CMD               _IOWR(GPS_CMD_BASE, 2, unsigned long)
    /*GPS��HWCUST���ƿ���*/
#define GPS_HWCUST_PIN_SET_CMD            _IOWR(GPS_CMD_BASE, 3, unsigned long)
    /*GPS��HWCUST״̬��ѯ*/
#define GPS_HWCUST_PIN_QUY_CMD            _IOWR(GPS_CMD_BASE, 4, unsigned long)
    /*GPS��Դ����ͶƱ����*/ 
#define GPS_LOCK_CTRL_CMD                 _IOWR(GPS_CMD_BASE, 5, unsigned long)

typedef enum
{
    GPS_POWER_OFF = 0,
    GPS_POWER_ON
}GPS_POWER_STATE;

typedef enum
{
    GPS_32K_CLK_OFF = 0,
    GPS_32K_CLK_ON
}GPS_32K_CLK_STATE;

typedef enum
{
    GPS_PM_UNLOCK = 0,
    GPS_PM_LOCK
}GPS_LOCK_STATE;

int gps_pmu_power_switch(GPS_POWER_STATE enPara)
{
    int ret = 0;

    if(GPS_POWER_OFF == enPara)
    {
        ret = regulator_enable(gps_vcc);
    }
    else if(GPS_POWER_ON == enPara)
    {
        ret = regulator_disable(gps_vcc);
    }

    if (ret)
    {
        GPS_PRINT(KERN_ERR, "ERROR: regulator_enable GPS_VCC failed, ret %d \n", ret);
    }

    return ret;

}

int gps_32k_clk_switch(GPS_POWER_STATE enPara)
{
    int ret = 0;
    if(GPS_32K_CLK_OFF == enPara)
    {
        ret = bsp_pmu_32k_clk_disable(PMU_32K_CLK_B);
    }
    else if(GPS_32K_CLK_ON == enPara)
    {
        ret = bsp_pmu_32k_clk_enable(PMU_32K_CLK_B);
    }

    if (ret)
    {
        GPS_PRINT(KERN_ERR, "ERROR: enable gps 32k clock failed, ret %d \n",ret);
        return -1;
    }

    GPS_PRINT(KERN_INFO, "gps_adapter_init successful out\n");

    return 0;

}

int gps_disable_pin_level_qury(unsigned int *enPara)
{
    unsigned int uclevel = 0;
    uclevel = !!gpio_get_value(GPS_DISABLE_PIN);
    GPS_PRINT(KERN_ERR, "gps disable pin level is %d \n",uclevel);	

    if(copy_to_user(enPara, &uclevel, sizeof(unsigned int)))
    {
        GPS_PRINT(KERN_ERR, "copy data to user OK **\n ");
    }

    return 0;
}

int gps_hwcust_pin_level_qury(unsigned int *enPara)
{


    return 0;
}

void gps_pm_lock_ctrl(GPS_LOCK_STATE enPara)
{
    if(GPS_PM_UNLOCK == enPara)
    {
        wake_unlock(&gps_lock);
    }
    else if(GPS_PM_LOCK == enPara)
    {
        wake_lock(&gps_lock);
    }
    else
    {
        GPS_PRINT(KERN_ERR, "para error !!! \n ");
    }
}

/*===========================================================================
FUNCTION 
    gps_disablepin_event_isr
DESCRIPTION
    The interrupt handler for GPS_DISABLE interrupt.
DEPENDENCIES
    None
RETURN VALUE
    None
SIDE EFFECTS
    None
===========================================================================*/
static irqreturn_t gps_disablepin_event_isr(void)
{
    int ucdata = 0;
    unsigned int uclevel = 0;
    
    ucdata = gpio_int_state_get(GPS_DISABLE_PIN);

    if (!ucdata)
    {
        GPS_PRINT(KERN_ERR, " gpio_int_state_get error !\n");
        return IRQ_NONE;
    }
    gpio_int_state_clear(GPS_DISABLE_PIN);  

    uclevel = gpio_get_value(GPS_DISABLE_PIN);

    if ( uclevel )
    {
        GPS_PRINT(KERN_ERR, " high level irq happen !\n");
        wake_lock_timeout(&gps_msg_lock, (long)msecs_to_jiffies(PM_GPS_MSG_TIME));
        gpio_int_trigger_set( GPS_DISABLE_PIN, IRQ_TYPE_EDGE_FALLING);
        ATNL_StopGPSCB();
    }
    else
    {
        GPS_PRINT(KERN_ERR, " low level irq happen !\n");
        gpio_int_trigger_set(GPS_DISABLE_PIN,IRQ_TYPE_EDGE_RISING);
    }

    return IRQ_HANDLED;
        
}


/******************************************************************************
Function:       huawei_pm_wakeupin_gpio_init
Description:   Initialize the GPS_DISABLE gpio
Input:           None
Output:         None
Return:         int, 0 = success, -1 = failed
Others:         None
******************************************************************************/
int gps_disable_gpio_init(void)
{
    int ret = 0;
    int level = 0;
    unsigned int int_num;

    ret = gpio_request(GPS_DISABLE_PIN, LABEL_GPS_DISABLE);
    if(ret)
    {
        GPS_PRINT(KERN_ERR, "request GPS_DISABLE_PIN fail !!! \n");
        goto fail1;
    }

    /*����GPIOΪ����*/
    gpio_direction_input(GPS_DISABLE_PIN);
    gpio_int_mask_set(GPS_DISABLE_PIN);

    /*����GPIO�����жϴ�����ʽ*/
    level = gpio_get_value(GPS_DISABLE_PIN);
    if(level)
    {
        gpio_int_trigger_set( GPS_DISABLE_PIN, IRQ_TYPE_EDGE_FALLING);
    }
    else
    {
        gpio_int_trigger_set(GPS_DISABLE_PIN,IRQ_TYPE_EDGE_RISING);
    }

    gpio_set_function(GPS_DISABLE_PIN,GPIO_INTERRUPT);
    int_num = gpio_to_irq(GPS_DISABLE_PIN);
    /*�����жϺŶ�Ӧ�Ļص�����*/
    ret = request_irq(int_num, \
            (irq_handler_t) gps_disablepin_event_isr, IRQF_SHARED, LABEL_GPS_DISABLE, "gps_disable");
    if (ret) 
    {
        GPS_PRINT(KERN_ERR," request GPS_DISABLE irq fail !!! \n");
        goto fail2;
    }
    
    /*ʹ��GPIO�ж�*/
    gpio_int_unmask_set(GPS_DISABLE_PIN);

    /*��ʼ����жϱ�־λ*/
    gpio_int_state_clear(GPS_DISABLE_PIN);

    return 0;

fail2:
    gpio_free(GPS_DISABLE_PIN);
fail1:
    return -1;
}


/********************************************************
*�� �� ��   : gpsControlIoctl
*��������: GPS����ioctl���������ڴ����û�̬�����·���ָ��
*�������: 
*�������: ��
*�� �� ֵ   : ִ�гɹ�����0��ʧ�ܷ��ط�0ֵ
********************************************************/
long gpsControlIoctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    if(NULL == file )
    {
        GPS_PRINT(KERN_ERR, "file is NULL!\r\n");
        return -1;
    }

    GPS_PRINT(KERN_ERR, " cmd=0x%x, data = 0x%x !\r\n",cmd,arg);

    switch(cmd)
    {
        case GPS_POWER_CTRL_CMD:
        {
            /*gps��Դ����*/
            ret = gps_pmu_power_switch((GPS_POWER_STATE)arg);
            break;
        }
        case GPS_CLOCK_CTRL_CMD:
        {
            /*gps��32Kʱ�ӿ���*/
            ret = gps_32k_clk_switch((GPS_32K_CLK_STATE)arg);
            break;
        }
        case GPS_DISABLE_QUY_CMD:
            /*gps disable pin�ŵ�ƽ״̬��ѯ*/
            ret = gps_disable_pin_level_qury((unsigned int *)arg);
            break;
        case GPS_HWCUST_PIN_SET_CMD:
            /*gps disable pin�ŵ�ƽ״̬��ѯ*/
            //ret = gpsDisablePinLevelQury((unsigned int *)arg);
            break;
        case GPS_HWCUST_PIN_QUY_CMD:
            /*gps disable pin�ŵ�ƽ״̬��ѯ*/
            //ret = gpsDisablePinLevelQury((unsigned int *)arg);
            break;
        case GPS_LOCK_CTRL_CMD:
            /*gps�����ƽӿ�*/
            gps_pm_lock_ctrl((GPS_LOCK_STATE)arg);
            break;
        default:
            /*������֧�ָ�����*/
            return -ENOTTY;
    }
    
    return ret;
}

static const struct file_operations gpsCtrlFops = {
    .owner         = THIS_MODULE,
    .unlocked_ioctl = gpsControlIoctl,
};

static struct miscdevice gpsmiscdevice = {
    .minor    = MISC_DYNAMIC_MINOR,
    .name    = "gps_control",
    .fops    = &gpsCtrlFops
};
/*end add for debug*/

static int __init gps_adapter_init(void)
{
    int ret = 0;
    SOLUTION_PRODUCT_TYPE product_type = PRODUCT_TYPE_INVALID;

    GPS_PRINT(KERN_INFO, "gps_adapter_init IN\n");

    product_type = bsp_get_solution_type();
    if (PRODUCT_TYPE_CE != product_type)
    {
        GPS_PRINT(KERN_INFO, "gps_adapter_init OUT, !CE product\n");
        return ret;
    }

    /* ����gps�������ĳ�ʼ��*/
    wake_lock_init(&gps_lock, WAKE_LOCK_SUSPEND, "pm_gps");
    wake_lock_init(&gps_msg_lock, WAKE_LOCK_SUSPEND, "pm_gps_msg");

    /*1. ����GPS��Դ*/
    gps_vcc = regulator_get(NULL, "LCD-vcc");
    if (IS_ERR(gps_vcc))
    {
        GPS_PRINT(KERN_ERR, "ERROR: GPS_VCC regulator_get error:%d!/n", 
        (int)PTR_ERR(gps_vcc));
        return -1;
    }

    ret = regulator_set_voltage(gps_vcc, GPS_VOLTAGE_UV, GPS_VOLTAGE_UV);
    if (ret)
    {
        GPS_PRINT(KERN_ERR, "ERROR: regulator_set_voltage failed, ret = %d\n", ret);
        goto Error5;
    }

    ret = regulator_enable(gps_vcc);
    if (ret)
    {
        GPS_PRINT(KERN_ERR, "ERROR: regulator_enable GPS_VCC failed, ret %d \n", ret);
        goto Error5;
    }

    /*gps standby GPIO����*/
    /*����GPIO_2_25��ΪGPS standby �ܽţ��������ʼΪ�͵�ƽ*/
    ret = gpio_request(GPS_GPIO, "gpsstandby");
    if (ret < 0)
    {
        GPS_PRINT(KERN_ERR, "ERROR: request gps standy gpio failed, ret %d \n", ret);
        goto Error4;
    }
    ret = gpio_export(GPS_GPIO, true);
    if (ret < 0)
    {
        GPS_PRINT(KERN_ERR, "ERROR: export gps standy gpio failed, ret %d \n", ret);
        goto Error3;
    }
 
    gpio_direction_output(GPS_GPIO, 1);

    /*2. ���32Kʱ��*/
    ret = bsp_pmu_32k_clk_enable(PMU_32K_CLK_B);
    if (0 > ret)
    {
        GPS_PRINT(KERN_ERR, "ERROR: enable gps 32k clock failed, ret %d \n", ret);
        goto Error3;
    }

    /*3. ע������豸*/
    ret = misc_register(&gpsmiscdevice);
    if (0 > ret)
    {
        GPS_PRINT(KERN_ERR,"ERROR: gpsmiscdevice register failed !!! \r\n");
        goto Error2;
    }

    ret = gps_disable_gpio_init();
    if (ret)
    {
        GPS_PRINT(KERN_ERR,"ERROR: gps_disable_gpio_init failed !!! \r\n");
        goto Error1;
    }

    GPS_PRINT(KERN_ERR, "gps_adapter_init successful out\n");

    return 0;

Error1:
    misc_deregister(&gpsmiscdevice);

Error2:
    bsp_pmu_32k_clk_disable(PMU_32K_CLK_B);

Error3:
    gpio_free(GPS_GPIO);

Error4:
    regulator_disable(gps_vcc);

Error5:
    regulator_put(gps_vcc);

    return ret;
}

static void __exit gps_adapter_exit(void)
{
    misc_deregister(&gpsmiscdevice);
    bsp_pmu_32k_clk_disable(PMU_32K_CLK_B);
    regulator_disable(gps_vcc);
    regulator_put(gps_vcc);
}

module_init(gps_adapter_init);
module_exit(gps_adapter_exit);

#endif

#ifdef __cplusplus
}
#endif
