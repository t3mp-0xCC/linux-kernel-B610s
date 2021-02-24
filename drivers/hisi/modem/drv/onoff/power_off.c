/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

 
/*lint --e{537} */
#include <hi_gpio.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/rtc.h>

#include <product_config.h>
#include <osl_list.h>
#include <osl_types.h>
#include <osl_spinlock.h>
#include <mdrv_chg.h>
#include <mdrv_sysboot.h>
#include <bsp_pmu.h>
#include <power_com.h>
#include "power_exchange.h"
#include <reb_func.h>
#include <bsp_sram.h>
#include <bsp_reset.h>

#include <bsp_onoff.h>
#include <bsp_sysctrl.h>
#include "mdrv_chg.h"

#include <bsp_icc.h>
#include <hi_gpio.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include "mbb_leds.h"
#include <linux/mlog_lib.h>
#include "bsp_dump.h"



struct bsp_onoff_callback {
    struct list_head node;
    void (*fn)(void);
};

static LIST_HEAD(list_callback);
static DEFINE_SPINLOCK(list_spinlock); //lint !e43 !e64 !e120



#define POWER_OFF_MONOTER_TIMEROUT      (5000)
LOCAL_1 void power_off_set_flag_and_reboot( power_off_reboot_flag enRebootFlag );
static int power_off_timeout_isr( void );
#define EXCH_RESET_LOG_PATH     "/modem_log/poweroff.txt"

typedef struct
{
    DRV_SHUTDOWN_REASON_E      reason;
    struct rtc_time            time;
	struct softtimer_list      off_timer;
}power_off_ctrl_s;

extern BATT_LEVEL_E chg_get_batt_level(void);

power_off_ctrl_s power_off_ctrl = {DRV_SHUTDOWN_BUTT, {0,0,0,0,0,0,0,0,0}};

/* stub */
BATT_LEVEL_E chg_get_batt_level(void)
{
    return BATT_LEVEL_4;
}

void power_off_clear_dump(void)
{
    unsigned long flags;
    struct bsp_onoff_callback *hook = NULL;

    spin_lock_irqsave(&list_spinlock, flags);

    list_for_each_entry(hook, &list_callback, node)
    if ((hook) && (hook->fn))
    {
        hook->fn();
    }
    spin_unlock_irqrestore(&list_spinlock, flags);
}

/*****************************************************************************
 �� �� ��  : void power_off_close_bb(void)
 ��������  : �رջ�����Դ
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void power_off_close_bb(void)
{
    int volt_id = PMIC_HI6421_VOLT_ID_MIN;
    s32 ret = 0;

    for(volt_id = PMIC_HI6421_VOLT_ID_MIN; volt_id <= PMIC_HI6421_VOLT_ID_MAX; volt_id++)
    {
        if(    (PMIC_HI6421_ID_BUCK0 != volt_id)
            && (PMIC_HI6421_ID_BUCK1 != volt_id)
            && (PMIC_HI6421_ID_BUCK3 != volt_id)
            && (PMIC_HI6421_ID_LDO0 != volt_id)
            && (PMIC_HI6421_ID_LDO2 != volt_id)
            && (PMIC_HI6421_ID_LDO26 != volt_id)
          )
        {
            ret = pmic_volt_disable(volt_id);
            if (BSP_PMU_OK != ret)
            {
                printk("power_off_close_bb disable %d failed, ret=%d \n", 
                        volt_id, ret);
            }
        }
    }
}

/*****************************************************************************
 �� �� ��  : power_off_close_peripheral
 ��������  : ��ػ����ر����裬Ŀǰ�ǹر�����ָʾ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void power_off_close_peripheral(void)
{
    if (0 == gpio_request(ETH_PWR_ON_GPIO, "Power off"))
    {
        (void)gpio_direction_output(ETH_PWR_ON_GPIO,  0);
    }
    else
    {
        printk("power_off_close_peripheral request ETH_PWR_ON_GPIO failed!\n");
    }

    led_kernel_status_set(POWER_GREEN_LED,      LEDS_OFF);
    led_kernel_status_set(WIFI_WHITE_LED,       LEDS_OFF);
    led_kernel_status_set(LAN_WHITE_LED,        LEDS_OFF);
    led_kernel_status_set(MODE_RED_LED,          LEDS_OFF);
    led_kernel_status_set(MODE_GREEN_LED,        LEDS_OFF);
    led_kernel_status_set(MODE_BLUE_LED,         LEDS_OFF);
    led_kernel_status_set(STATUS_GREEN_LED,     LEDS_OFF);
    led_kernel_status_set(SIGNAL1_RED_LED,      LEDS_OFF);
    led_kernel_status_set(SIGNAL1_WHITE_LED,     LEDS_OFF);
    led_kernel_status_set(SIGNAL2_WHITE_LED,     LEDS_OFF);
    led_kernel_status_set(SIGNAL3_WHITE_LED,     LEDS_OFF);
}

void power_off_notify_mcore(void)
{
#define ICC_CHANNEL_NUM_MASK        16
#define ICC_CHANNEL_ACORE_NOTIFY_MCORE \
        ((ICC_CHN_MCORE_ACORE << ICC_CHANNEL_NUM_MASK) | MCORE_ACORE_FUNC_POWEROFF_NOTIFY)

    int ret = 0;
    u32 data = 0;
    u32 data_len = sizeof(data);

    ret = bsp_icc_send(ICC_CPU_MCU, ICC_CHANNEL_ACORE_NOTIFY_MCORE, &data, data_len);
    if(data_len != (u32)ret)
    {
        printk(KERN_ALERT "--->power_off_notify_mcore send icc error..\n");        
    }

    printk(KERN_ALERT "--->power_off_notify_mcore send icc ok..\n");  
}

static inline void sysboot_set(void* __virt_addr, u32 __bit_offset, u32 __bit_width, u32 __bit_value)
{
    u32 __value = 0;

    __value = readl(__virt_addr);
    __value &= ~(((1 << __bit_width) - 1) << __bit_offset);
    __value |= (__bit_value << __bit_offset);
    writel(__value, (void*)__virt_addr);
}

/*****************************************************************************
 �� �� ��  : power_off_down_operation
 ��������  : pmu hold ϵͳ�µ�
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
LOCAL_1 void power_off_down_operation( void )
{
    int ret = 0;
    u32 dts_info[4];
    struct device_node *dev_node = NULL;

    bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_ONOFF, "system power down!\n");
        /* ������������� */
        power_reboot_cmd_set(POWER_REBOOT_CMD_BUTT);
        power_off_clear_dump();
        power_on_wdt_cnt_set();

    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,sysboot_balong");
    if (dev_node) {
    } else {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_ONOFF, "fail to find sysboot dts node\n");
        goto dead_loop;
    }

    ret = of_property_read_u32_array(dev_node, "power_down", &dts_info[0], sizeof(dts_info)/sizeof(dts_info[0]));
    if (!ret) {
        power_off_close_bb();
        power_off_close_peripheral();
        power_off_notify_mcore();
    } else {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_ONOFF, "fail to read sysboot dts node, ret = %d\n", ret);
        goto dead_loop;
    }

dead_loop:
    /* coverity[no_escape] */
    for(;;) ;
}

/*****************************************************************************
 �� �� ��  : power_off_reboot_operation
 ��������  : pmu rst ϵͳ����
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
LOCAL_1 void power_off_reboot_operation( void )
{
    int ret = 0;
    u32 dts_info[4];
    struct device_node *dev_node = NULL;
    bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_ONOFF, "system reboot!\n");

/* p532_FPGA��λ��Ҫͬʱ��λfpga. */

    /* ��������������Ϊ������ */
    power_reboot_cmd_set(POWER_REBOOT_CMD_WARMRESET);


    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,sysboot_balong");
    if (dev_node) {
    } else {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_ONOFF, "fail to find sysboot dts node\n");
        goto dead_loop;
    }

    ret = of_property_read_u32_array(dev_node, "reboot", &dts_info[0], sizeof(dts_info)/sizeof(dts_info[0]));
    if (!ret) {
        sysboot_set(bsp_sysctrl_addr_get((void*)dts_info[0]), dts_info[1], dts_info[2], dts_info[3]);
    } else {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_ONOFF, "fail to read sysboot dts node, ret = %d\n", ret);
        goto dead_loop;
    }

dead_loop:
    /* coverity[no_escape] */
    for(;;) ;
}

/*****************************************************************************
 �� �� ��  : is_power_key_pressed
 ��������  : �ػ�ʱ�����Ƿ���
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
LOCAL_1 bool is_power_key_pressed( void )
{
    return bsp_pmu_key_state_get();
}

/*****************************************************************************
 �� �� ��  : is_power_off_charge_in
 ��������  : �ػ�ʱ������Ƿ����
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
LOCAL_1 bool is_power_off_charge_in( void )
{
    bool pmu_usb_state = false;
    /*ģ���Ʒusb������*/
	pmu_usb_state = bsp_pmu_usb_state_get();
    printk(KERN_INFO "[ ON OFF] pmu_usb_state is %d.\n", pmu_usb_state);

    return pmu_usb_state;
}
/*****************************************************************************
 �� �� ��  : power_off_operation
 ��������  : ִ�йػ�����
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
LOCAL_1 void power_off_operation( void )
{
    /* ��ػ������󡢰����ɿ�ǰ���Ȱ�ָʾ�ƹرա������û�һֱ���� power������ 8s������ PMU�����¼� */
    power_off_close_peripheral();

    /* �������ɿ�����£�������ػ� */
    while( is_power_key_pressed() )
    {
        /* avoid the watchdog reset while polling the power key */
        /* TO DO*/
    }

    pr_dbg("\r\n power_off_operation : the power key is release\r\n" );

    /* shutdown the system. */
    power_off_down_operation();
}

/*****************************************************************************
 �� �� ��  : power_off_set_flag_and_reboot
 ��������  : ִ�йػ�����
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
LOCAL_1 void power_off_set_flag_and_reboot( power_off_reboot_flag enRebootFlag )
{
    /*lint --e{*} */
    unsigned long irq_flags = 0;
    struct bsp_onoff_callback *hook = NULL;

    spin_lock_irqsave(&list_spinlock, irq_flags);
    /*coverity[var_deref_op] */
    list_for_each_entry(hook, &list_callback, node)
        /*coverity[var_compare_op] */
        if ((hook) && (hook->fn))
            hook->fn();
    spin_unlock_irqrestore(&list_spinlock, irq_flags);

    power_on_reboot_flag_set( enRebootFlag );
    power_off_reboot_operation();
}

/*****************************************************************************
 �� �� ��  : power_off_battery_error_handler
 ��������  : ��ش���ػ�����
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
LOCAL_1  void power_off_battery_error_handler( void )
{
    if( is_power_off_charge_in() )
    {
        power_off_set_flag_and_reboot( POWER_OFF_REASON_BAD_BATTERY );
    }
    else
    {
        power_off_operation();
    }
}

/*****************************************************************************
 �� �� ��  : power_off_battery_low_handler
 ��������  : ��ص͵紦��
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
LOCAL_1 void power_off_battery_low_handler( void )
{
    if( is_power_off_charge_in() )
    {
        power_off_set_flag_and_reboot( POWER_OFF_REASON_LOW_BATTERY );
    }
    else
    {
        power_off_operation();
    }
}

/*****************************************************************************
 �� �� ��  : power_off_temp_protect_handler
 ��������  : �¶ȱ�������
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
LOCAL_1 void power_off_temp_protect_handler( void )
{
    if( is_power_off_charge_in() )
    {
        /* The charge is in, can't power off, restart and show the warning */
        power_off_set_flag_and_reboot(POWER_OFF_REASON_OVER_TEMP);
    }
    else
    {
        /* No warning message, power off directly */
        power_off_operation();
    }
}

/*****************************************************************************
 �� �� ��  : power_off_power_key_handler
 ��������  : �����ػ�����, �ڹػ����ģʽ�°�����������������ģʽ
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
LOCAL_1 void power_off_power_key_handler( void )
{
    if( DRV_START_MODE_CHARGING == bsp_start_mode_get() )
    {
        /* reboot and enter normal mode; */
        power_off_set_flag_and_reboot( POWER_OFF_REASON_NORMAL );
    }
    else /* normal mode */
    {
        /* if charge is in, will power on again and enter charge mode. */
        power_off_operation();
    }
}

/*****************************************************************************
 �� �� ��  : power_off_update_handler
 ��������  : �����ػ�������λ����recoveryģʽ
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
LOCAL_1 void power_off_update_handler( void )
{
    /* reboot and enter update mode */
    power_off_set_flag_and_reboot( POWER_OFF_REASON_UPDATE );
}

/*****************************************************************************
 �� �� ��  : power_off_charge_remove_handler
 ��������  : ������Ƴ�����
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
LOCAL_1 void power_off_charge_remove_handler( void )
{
    if( DRV_START_MODE_CHARGING == bsp_start_mode_get() )
    {
        power_off_set_flag_and_reboot( POWER_OFF_REASON_RM_CHARGE );
        power_off_operation();  /* �ػ����ʱ���γ������������ֱ���µ� */
    }
    else
    {
        /* Normal mode, nothing need to do. */
    }
}

/*****************************************************************************
 �� �� ��  : power_off_reset_handler
 ��������  : ϵͳ��λ������ֱ�Ӹ�λ�����ü��charger״̬
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
void power_off_reset_handler( void )
{
    /* Needn't check charge status */
    power_off_set_flag_and_reboot( POWER_OFF_REASON_NORMAL );
}

/*****************************************************************************
 �� �� ��  : power_off_reset_handler
 ��������  : ϵͳ��λ������ֱ�Ӹ�λ�����ü��charger״̬
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
LOCAL_1 void power_off_excecute( DRV_SHUTDOWN_REASON_E enShutdownReason )
{
	unsigned long flags = 0;
    reb_file_sync();

	local_irq_save(flags);

    switch( enShutdownReason )
    {
    case DRV_SHUTDOWN_RESET:
        power_off_set_flag_and_reboot( POWER_OFF_REASON_NORMAL );
        break;
    case DRV_SHUTDOWN_BATTERY_ERROR:
        power_off_battery_error_handler();
        break;
    case DRV_SHUTDOWN_LOW_BATTERY:
        power_off_battery_low_handler();
        break;
    case DRV_SHUTDOWN_TEMPERATURE_PROTECT:
        power_off_temp_protect_handler();
        break;
    case DRV_SHUTDOWN_POWER_KEY:
        power_off_power_key_handler();
        break;
    case DRV_SHUTDOWN_UPDATE:
        power_off_update_handler();
        break;
    case DRV_SHUTDOWN_CHARGE_REMOVE:
        power_off_charge_remove_handler( );
        break;
    default:
        power_off_operation();
        break;
    }

    /* Woulnd't reach here ! */
    local_irq_restore( flags );
}

/*****************************************************************************
 �� �� ��  : power_on_mode_get
 ��������  : ��ȡ����ģʽ
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
unsigned int power_off_reason_get( void )
{
    power_off_reboot_flag reboot_reason = (power_off_reboot_flag)(power_off_ctrl.reason) ;
    DRV_SHUTDOWN_REASON_E rb = DRV_SHUTDOWN_BUTT;

    switch(reboot_reason)
    {
    case POWER_OFF_REASON_NORMAL:
        rb = DRV_SHUTDOWN_POWER_KEY;
        break;
    case POWER_OFF_REASON_BAD_BATTERY:
        rb = DRV_SHUTDOWN_BATTERY_ERROR;
        break;
    case POWER_OFF_REASON_LOW_BATTERY:
        rb = DRV_SHUTDOWN_LOW_BATTERY;
        break;
    case POWER_OFF_REASON_OVER_TEMP:
        rb = DRV_SHUTDOWN_TEMPERATURE_PROTECT;
        break;
    case POWER_OFF_REASON_RM_CHARGE:
        rb = DRV_SHUTDOWN_CHARGE_REMOVE;
        break;
    case POWER_OFF_REASON_UPDATE:
        rb = DRV_SHUTDOWN_UPDATE;
        break;
    case POWER_OFF_REASON_RESET:
        rb = DRV_SHUTDOWN_RESET;
        break;
    default:
        rb = DRV_SHUTDOWN_BUTT;
        break;
    }

    return rb;

}

/******************************************************************************
*  Function:  power_off_log_save
*  Description: save the power off log( reason and battery voltage ).
*  Input:
*         None
*  Output:
*         None
*  Return:
*         None
*  Note  :
********************************************************************************/
LOCAL_1 int power_off_log_save( void )
{
    long pos;
    unsigned int pf;
    mm_segment_t old_fs;
    struct rtc_time tm;
    struct timespec ts;
    int     rt;
    char    buf[128];
    char    *reboot_reason[] = {"NORMAL", "BAD BATTERY", "LOWBATTERY", "OVERTEMP", \
                                    "RM_CHARGER", "UPDATE", "REBOOT", "INVALID"};
    BATT_LEVEL_E                 battery_level = chg_get_batt_level();
    DRV_SHUTDOWN_REASON_E        rb;

    getnstimeofday(&ts);

    rtc_time_to_tm((unsigned long)ts.tv_sec, &tm);

    power_off_ctrl.time = tm;
    rb = power_off_ctrl.reason;

    pr_dbg("%4d-%02d-%02d %02d:%02d:%02d\n",tm.tm_year+1900, tm.tm_mon+1, \
        tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    if( DRV_SHUTDOWN_RESET == power_off_ctrl.reason )
    {
       mlog_print(MLOG_POWER, mlog_lv_info, "system will go to reboot\n" );
       mlog_set_statis_info("reboot_times",1); //normal reboot statistic   
    }
    else
    {
        mlog_print(MLOG_POWER, mlog_lv_info, "SHUTDOWN due to %s\n" , reboot_reason[rb]);
        mlog_set_statis_info("off_times",1);    
    }  
    sys_shutdown_mog_report();


    /* ��¼�ػ���Ϣ(ʱ�䡢�������ػ�ԭ��) */
    snprintf(buf, sizeof(buf) - 1, "system close reason(E5): %s, current battery voltage: %d, current time: %4d-%02d-%02d %02d:%02d:%02d\n", \
        reboot_reason[rb], battery_level, tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    old_fs = get_fs(); //lint !e63

    set_fs(KERNEL_DS);

    /*coverity[check_return] */
    pf = (unsigned int)sys_open(EXCH_RESET_LOG_PATH, O_RDWR | O_CREAT, 0666);
    /*coverity[unchecked_value] */
    if(IS_ERR((const void*)pf))
    {
        pr_dbg( "error occured happened when open file %s, exiting.\n", EXCH_RESET_LOG_PATH);
        return (int)pf;
    }

    /*coverity[unchecked_value] */
    pos = sys_lseek(pf, 0, SEEK_END);
    if(pos > EXCH_ONOFF_LOG_MAX){
        /* �ļ����� 16k,ɾ�����´� */
        sys_rmdir(EXCH_RESET_LOG_PATH);
        /*coverity[check_return] */
        pf = (unsigned int)sys_open(EXCH_RESET_LOG_PATH, O_RDWR | O_CREAT, 0666);
        /*coverity[unchecked_value] */
        if(IS_ERR((const void*)pf))
        {
            pr_dbg( "error occured happened when open file %s, exiting.\n", EXCH_RESET_LOG_PATH);
            return (int)pf;
        }
    }
    else{
        /*coverity[unchecked_value] */
        sys_lseek(pf, pos, SEEK_SET);
    }

    /*coverity[unchecked_value] */
    rt = sys_write(pf, (const char*)buf, strlen(buf));
    if(rt<0)
    {
        pr_dbg("error occured happened when write file %s, exiting.\n", EXCH_RESET_LOG_PATH);
        /*coverity[unchecked_value] */
        sys_close( pf );
        set_fs(old_fs);
        return rt;
    }

    pr_dbg(KERN_DEBUG "power off log save.");

    /*coverity[unchecked_value] */
    sys_close( pf );
    set_fs(old_fs);

    return rt;
}

/******************************************************************************
*  Function:  power_off_timeout_isr
*  Description: ���Ӧ��û��һ��ʱ���ڹػ����ײ�ֱ�ӹػ�
*  Input:
*         None
*  Output:
*         None
*  Return:
*         None
*  Note  :
********************************************************************************/
static int power_off_timeout_isr( void )
{
    pr_dbg("power_off_timeout_isr, reason :%d\n ",power_off_ctrl.reason);

    (void)bsp_softtimer_free(&power_off_ctrl.off_timer);

    power_off_excecute(power_off_ctrl.reason);

    return 0;
}

/******************************************************************************
*  Function:  drv_power_off
*  Description: ֱ�ӹػ��ӿڣ�����¼�ػ��¼�
*  Input:
*         None
*  Output:
*         None
*  Return:
*         None
*  Note  :
********************************************************************************/
LOCAL_1 void drv_power_off( void )
{
    /*
       If the shutdown reason has been set, do it.
       If not, it is invalid one and will shutdown
    */
    power_off_excecute(power_off_ctrl.reason);
}

void bsp_reboot_callback_register(void (*hook)(void))
{
    /*lint --e{*} */
    unsigned long flags = 0;

    struct bsp_onoff_callback *callback =
        (struct bsp_onoff_callback *)kmalloc(sizeof(struct bsp_onoff_callback), GFP_KERNEL);
    if (NULL == callback)
    {
        pr_dbg("fail to malloc struct bsp_onoff_callback \n");
        return;
    }

    callback->fn = hook;

    spin_lock_irqsave(&list_spinlock, flags);
    list_add(&callback->node, &list_callback);
    spin_unlock_irqrestore(&list_spinlock, flags);
    /*coverity[leaked_storage] */
}

/******************************************************************************
*  Function:  drv_shut_down
*  Description: start the power off process.
*  Input:
*         eReason : shutdown reason.
*  Output:
*         None
*  Return:
*         None
*  Note  : �ײ���ùػ��ӿڣ����ö�ʱ�����ϱ��¼���Ӧ�á�
*          ��ʱʱ����Ӧ�ò��ػ����ɵײ�ǿ�ƹػ���
********************************************************************************/
void drv_shut_down( DRV_SHUTDOWN_REASON_E enReason )
{
    static unsigned long drv_shut_down_set_flag = false;

	power_off_ctrl.off_timer.func = (softtimer_func)power_off_timeout_isr;
    if( DRV_SHUTDOWN_CHARGE_REMOVE == enReason )
    {
        if( DRV_START_MODE_NORMAL == bsp_start_mode_get() )
        {
            /* In normal mode, don't handle the charge remove message */
            return ;
        }
    }

    if( DRV_SHUTDOWN_BUTT == enReason )
    {
        return ; /* the shutdown been handled, ignore this call */
    }

    if( DRV_SHUTDOWN_RESET == enReason )
    {
        pr_dbg("system will go to reboot!! \n");
        bsp_drv_power_reboot();
        return;
    }

    /* save the reason and drv_power_off need it */
    if (false == drv_shut_down_set_flag)
    {
    power_off_ctrl.reason = enReason;

	power_off_ctrl.off_timer.para = (u32)0;
	power_off_ctrl.off_timer.timeout = POWER_OFF_MONOTER_TIMEROUT;
	power_off_ctrl.off_timer.wake_type = SOFTTIMER_WAKE;

    if (bsp_softtimer_create(&power_off_ctrl.off_timer))
    {
        pr_dbg("create softtimer failed \n");
    }

    bsp_softtimer_add(&power_off_ctrl.off_timer);

    /* ����ɵײ��ϱ��ػ��¼�����Ӧ��ִ�йػ��ĳ��� */
    /* TO DO */

    //power_off_log_save();
        drv_shut_down_set_flag = true;
    power_off_log_save();
    }
    else
    {
        return ;
    }
    /* Notify the monitor task */
}

/******************************************************************************
*  Function:  bsp_drv_power_off
*  Description: same as drv_power_off, the public API
*  Input:
*         None
*  Output:
*         None
*  Return:
*         None
*  Note  : Ӧ�õ��ùػ�ʱ���ã�at+cfun=8
********************************************************************************/
void bsp_drv_power_off( void )
{
    if(power_off_ctrl.reason == DRV_SHUTDOWN_BUTT)
    {
        /*Set shutdown reason to Power_key.*/
        power_off_ctrl.reason = DRV_SHUTDOWN_POWER_KEY;

        pr_dbg( "power off process triggered by a-cpu with power_key\n");
        //power_off_log_save();
    }
    power_off_log_save();
    power_on_wdt_cnt_set();
    drv_power_off();
}

/******************************************************************************
*  Function:  bsp_drv_power_reboot
*  Description: same as drv_power_off, the public API
*  Input:
*         None
*  Output:
*         None
*  Return:
*         None
*  Note  : Ӧ�õ�������ʱ���ã�at+cfun=6
********************************************************************************/
void bsp_drv_power_reboot( void )
{
    /*Set shutdown reason to reset.*/
    power_off_ctrl.reason = DRV_SHUTDOWN_RESET;

    pr_dbg( "power reboot process triggered \n");
    //power_off_log_save();

    reb_file_sync();
    power_on_wdt_cnt_set();

    power_off_set_flag_and_reboot((power_off_reboot_flag)(power_off_ctrl.reason));
}

void mdrv_sysboot_restart(void)
{
	bsp_om_save_reboot_log(__FUNCTION__, __builtin_return_address(0));
	bsp_drv_power_reboot();
}


/******************************************************************************
*  Function:  bsp_drv_power_reboot_direct
*  Description:
*  Input:
*         None
*  Output:
*         None
*  Return:
*         None
*  Note  : ֱ������
********************************************************************************/
void bsp_drv_power_reboot_direct( void )
{
    power_off_reboot_operation();
}



/******************************************************************************
*  Function:  balong_power_restart
*  Description: same as bsp_drv_power_reboot, ϵͳ����ʱʹ��
*  Input:
*         None
*  Output:
*         None
*  Return:
*         None
*  Note  :android_reboot(ANDROID_RB_RESTART, 0, 0);
********************************************************************************/
/*lint -save -e958 */
void balong_power_restart(char mode, const char *cmd)
{
	bsp_om_save_reboot_log(__FUNCTION__, __builtin_return_address(0));
    bsp_drv_power_reboot();
}
EXPORT_SYMBOL_GPL(balong_power_restart);
/*lint -restore */

/******************************************************************************
*  Function:  balong_power_off
*  Description: same as bsp_drv_power_off, ϵͳ����ʱʹ��
*  Input:
*         None
*  Output:
*         None
*  Return:
*         None
*  Note  :android_reboot(ANDROID_RB_POWER_OFF, 0, 0);
********************************************************************************/
void balong_power_off( void )
{
	bsp_om_save_reboot_log(__FUNCTION__, __builtin_return_address(0));
	bsp_drv_power_off();
}
EXPORT_SYMBOL_GPL(balong_power_off);

/******************************************************************************
*  Function:  debug����
*  Description: �ڲ�����ʱʹ��
*  Input:
*         None
*  Output:
*         None
*  Return:
*         None
*  Note  :
********************************************************************************/
void bsp_power_reboot_boot( void )
{
   power_reboot_cmd_set(POWER_REBOOT_CMD_BOOTLOADER);
   power_off_set_flag_and_reboot(POWER_OFF_REASON_RESET);
}

void bsp_power_reboot_recovery( void )
{
   power_reboot_cmd_set(POWER_REBOOT_CMD_RECOVERY);
   power_off_set_flag_and_reboot(POWER_OFF_REASON_RESET);
}

void bsp_power_reboot_warm( void )
{
   power_reboot_cmd_set(POWER_REBOOT_CMD_WARMRESET);
   power_off_set_flag_and_reboot(POWER_OFF_REASON_RESET);
}


void mdrv_sysboot_shutdown(void)
{
	bsp_om_save_reboot_log(__FUNCTION__, __builtin_return_address(0));
	drv_shut_down(DRV_SHUTDOWN_TEMPERATURE_PROTECT);
}



