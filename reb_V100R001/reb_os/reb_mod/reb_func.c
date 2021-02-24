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


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "../reb_config.h"
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/export.h>
#include <linux/notifier.h>
#include "../os_adapter/reb_os_adapter.h"
#include "reb_func.h"
#if ( YES == Reb_Platform_9x25)
#include "../platform_adapter/9x25/reb_9x25_platform.h"
#endif
#if ( YES == Reb_Platform_V7R2)
#include "../platform_adapter/v7r2/reb_v7r2_platform.h"
#include <drv_fastOnOff.h>
#endif
/*----------------------------------------------*
 * �궨��                                       *
*----------------------------------------------*/
/*4.0V~4.2V֮�����̫��,��4.0V��ʼ����*/
#define VOLT_INSPECT_UPPER_LIMIT (4000)

/*���ڼٹػ��͵�ػ�����10��Сʱ*/
#define UPPER_LIMIT_TIME (10 * 3600 * 1000)

/*�ٹػ��¼���ص�ѹ��������*/
#define VOLT_INSPECT_MAX_TIME (10)

/*��ص͵��ѹֵ*/
#define LOW_POWER_BAT_LVL (3550)

/*�ٹػ��¼���ص�ѹ����С�̶�:56mV,�Զ�Ӧ3000mAh���
3.7V-3.6V֮���50mA*/
#define VOLT_INSPECT_VOLT_DELTA_UNIT (56)

/*�ٹػ��¼���ص�ѹ��׼������� 3000mAh*/
#define VOLT_INSPECT_BASE_VOLT_CAPABILITY (3000)
/*----------------------------------------------*
 * �ڲ�����˵��                                 *
 *----------------------------------------------*/
typedef struct __volt_and_elec
{
    int volt_upside; /*��ص�*/
    int elect;       /*��ǰ��ĵ�ص㵽��һ�����
                      ��ص���Ҫ�ĵĵ���,��λmAh*/
}volt_and_elec;

typedef enum
{
    INVALID_VOLT = -1,       /*��Ч��ص�ѹ,�������*/
    BAT_OVER_USE = 0,        /*��ع���,��Ҫ�ػ�*/
    CONTINUE_INSPECT = 1,    /*�������*/
    STOP_INSPECT = 2,        /*ֹͣ���*/
}BAT_VOLT_STA;

/*----------------------------------------------*
 * �ⲿ����ԭ������                             *
 *----------------------------------------------*/
#if (FEATURE_ON == MBB_FAST_ON_OFF)
#if (MBB_CHARGE == FEATURE_ON)
extern int chg_get_batt_volt_value(void);
extern unsigned int chg_is_bat_only(void);
extern int get_low_bat_level(void);
#endif
#endif

extern reb_ret_val is_dload_mod(void);
/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

#if (FEATURE_ON == MBB_FAST_ON_OFF)
#if (MBB_CHARGE == FEATURE_ON)
int reb_fast_onoff_event_deal(struct notifier_block *nb, unsigned long val, void *data);
void reb_for_fast_onoff_stop(void);
#endif
#endif
/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
extern struct blocking_notifier_head g_fast_on_off_notifier_list;
#if ( YES == Reb_Platform_V7R2 )
extern reb_hard_timer_callback g_hard_timer_call_func;
#endif
/*kernel �׶ζ�ȡ��NV50442��ֵ*/
SOFT_RELIABLE_CFG_STRU g_kernel_soft_reliable_info = {0};
/*�ػ�ͬ���ļ�ϵͳǰ��ֹͣд��־*/
static bool g_stop_write = FALSE;

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
#if (FEATURE_ON == MBB_FAST_ON_OFF)
#if (MBB_CHARGE == FEATURE_ON)
/*�ٹػ�ʱ�ĵ�ص�ѹ,��Ϊ���ڼٹػ�ǰ��͵�����,�ñ���
����Ϊ�ٹػ�20���,��ص�ѹ�ȶ�ʱ��ֵ,���ж��Ƿ�56mv���ŵĻ�׼*/
static int volt_of_fast_off = 0;

/*�ٹػ�ʱ��ʱ�Ĵ���*/
static unsigned int volt_inspect_cnt = 0;

/*�ٹػ�ʱ��ص�ѹ���ֵ*/
static int volt_inspect_record[VOLT_INSPECT_MAX_TIME + 1] = {0};


/*����ٹػ��¼�֪ͨ���ص��ṹ��*/
struct notifier_block fast_on_event_notifier = {
    .notifier_call = reb_fast_onoff_event_deal,
};

/*���ŵĵ�ѹ��С�̶����Ʒ���������Ӧ*/
unsigned int bat_overuse_delt
    = ( VOLT_INSPECT_BASE_VOLT_CAPABILITY * VOLT_INSPECT_VOLT_DELTA_UNIT )
    / ( MBB_CHG_BAT_CAPABILITY ); 
#endif
#endif
/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
/*3000mAh���,��ѹ��������ձ�,ȡ3���ŵ����ߵ���Сֵ*/
/*����������Ϊ��ÿ0.1V�̶��ڵ������ѹʱ�������ԵĹ�ϵ*/
volt_and_elec volt_arry[] =
{
    { 4100, 180 },  /*4.1V-4.0V֮��(����)��200mAh����*/
    { 4000, 360 },  /*4.0V-3.9V֮��(����)��360mAh����*/
    { 3900, 400 },  /*3.9V-3.8V֮��(����)��400mAh����*/
    { 3800, 680 },  /*3.8V-3.7V֮��(����)��680mAh����*/
    { 3700, 900 },  /*3.7V-3.6V֮��(����)��900mAh����*/
    { 3600, 280 },  /*3.6V-3.5V֮��(����)��280mAh����*/
};

/*************************************************************
  ��������  : reb_reliability_nv_cfg_kernel_init
  ��������  : ��ȡnv 50442 ��ֵ����ȡ�ɿ���������Ϣ
  �������  : ��
  �������  : ��
  �� �� ֵ  :��
*************************************************************/
int reb_reliability_nv_cfg_kernel_init(void)
{
    return reb_nv_kernel_read();
}
EXPORT_SYMBOL(reb_reliability_nv_cfg_kernel_init);


#if (FEATURE_ON == MBB_FAST_ON_OFF)
#if (MBB_CHARGE == FEATURE_ON)
/*****************************************************************************
 �� �� ��  : reset_bat_inspect_para
 ��������  : ����ز�������
 �������  : none
 �������  : none
 �� �� ֵ  : none
 ע������  : 1\ÿ�ν���ٹػ� 2\��ػ�֮ǰ 3\��ص�ѹ����4.1V����
*****************************************************************************/
void reset_bat_inspect_para(void)
{
    reb_msg( REB_MSG_INFO, "REB_DFT:Fast on/off reset all inspect para\r\n");

    /*���õ�ѹ������*/
    volt_inspect_cnt = 0;

    /*���ùػ�ʱ�̵�ص�ѹ*/
    volt_of_fast_off = 0;

    /*���õ�ص�ѹ��¼*/
    memset( volt_inspect_record, 0, sizeof(volt_inspect_record) );
}

#ifdef REB_OS_DEBUG
/*****************************************************************************
 �� �� ��  : show_bat_inspect_val
 ��������  : ��ʾ��¼�ĵ�ص�ѹ
 �������  : none
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void show_bat_inspect_val(void)
{
    unsigned int idx = 0;
    if ( bat_overuse_delt )
    {
        reb_msg( REB_MSG_DEBUG, "REB_DFT:bat_overuse_delt is %umv\r\n",
            bat_overuse_delt );
    }
    
    if ( 0 == volt_inspect_cnt )
    {
        reb_msg( REB_MSG_DEBUG, "REB_DFT:No volt inspect record\r\n");
        return;
    }

    reb_msg( REB_MSG_DEBUG, "REB_DFT:Total %u inspect record\r\n", volt_inspect_cnt );

    for ( idx = 0; idx < volt_inspect_cnt; idx++ )
    {
        reb_msg( REB_MSG_DEBUG, "REB_DFT:The %dth inspect record is %dmV\r\n",\
            idx, volt_inspect_record[idx] );
    
    }
}
#endif
/*****************************************************************************
 �� �� ��  : is_current_normal
 ��������  : �鿴�����Ƿ�����
 �������  : none
 �������  : none
 �� �� ֵ  : BAT_VOLT_STA-��ǰ���״̬
 ע������  : 1- ���ڵ�����������(���Сʱ��10mv����),�´ζ�ʱ��ʱ��Ҫ�ӱ���������
             Ҫ����ö�ʱ
             2- ���ڵ������Ŀ��(���Զ�ȷ������,��������ػ�)
*****************************************************************************/
BAT_VOLT_STA is_current_normal(void)
{
    /*��ȡ��ǰ��ص�ѹ*/
    int cur_volt = chg_get_batt_volt_value();

    int low_power_batt_volt = get_low_bat_level();

    /*��ص�ѹ����4.0Vʱ,�������һ�ε�ѹֵ,����Ϊ0*/
    static int last_volt = 0;

    /*���û�д��ڼٹػ�,ֱ�ӷ��ص��OK*/
    if ( FASTONOFF_MODE_SET != fastOnOffGetFastOnOffMode() )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT:Fast on/off Not in fast off mode\r\n");
        return STOP_INSPECT;
    }

    /*���ڼ���ص�ѹ�ķ���:���ܴ��ڼٹػ�ǰ���,����ٹػ����ѹ����������*/
    if ( cur_volt > volt_of_fast_off )
    {
        /*���ܵ�ѹ��׼*/
        reb_msg( REB_MSG_ERR, "REB_DFT:Fast on/off Invalid bat volt: %d\r\n",
            cur_volt );
        /*����3.6V�����б���,�Ա�����ٹػ�3.55V������ͻ*/
        if ( ( low_power_batt_volt + bat_overuse_delt ) >= cur_volt ) 
        {
            reb_msg( REB_MSG_ERR, "REB_DFT:Fast on/off lower than %dmv, stop...\r\n",
                ( low_power_batt_volt + bat_overuse_delt ) );
            return STOP_INSPECT;
        }
        /*�˴�volt_of_fast_offҲ�ᱻ����*/
        reset_bat_inspect_para();
        volt_of_fast_off = cur_volt; /*ˢ��volt_of_fast_off*/
        return INVALID_VOLT;
    }
    reb_msg( REB_MSG_ERR, "REB_DFT:Fast on/off cur bat volt: %d, cnt: %ust\r\n",
        cur_volt,  ( volt_inspect_cnt + 1 ) );

    /*��¼���βɼ�ֵ(0~9)*/
    if ( volt_inspect_cnt < VOLT_INSPECT_MAX_TIME )
    {
        volt_inspect_record[volt_inspect_cnt] = cur_volt;
    }

    /*��ص�ѹ����4.0V����,�ȵ�4.0V*/
    if ( cur_volt > VOLT_INSPECT_UPPER_LIMIT )
    {
        /*��¼�ôε�ѹ*/
        last_volt = cur_volt;
        reb_msg( REB_MSG_INFO, "REB_DFT:Fast on/off higner than 4.0V wait...\r\n" );
    }
    else
    {
        /*��ص�ѹ����4.0V����,���������*/
        if ( last_volt > VOLT_INSPECT_UPPER_LIMIT )
        {
            /*�˴�volt_of_fast_offҲ�ᱻ����*/
            reset_bat_inspect_para();
            last_volt = 0;
            volt_of_fast_off = cur_volt;
            return INVALID_VOLT;
        }
        
        /*(0~9)10��֮��,����10��*/
        if ( volt_inspect_cnt < VOLT_INSPECT_MAX_TIME )
        {
            /*��ص�ѹ���ĳ���56mv*/
            if ( ( volt_of_fast_off - cur_volt ) >= bat_overuse_delt )
            {
                /*(0~9)10��֮�ڵ�ѹ���ͳ���bat_overuse_delt,׼���ػ�*/
                reb_msg( REB_MSG_ERR, "REB_DFT:Fast on/off Delt > %umv, go to power down\r\n",
                    bat_overuse_delt );
                return BAT_OVER_USE;
            }
        }
    }

    if ( ( volt_inspect_cnt + 1 ) >= VOLT_INSPECT_MAX_TIME )
    {
        /*�������(0~9)10�ζ�û�г���56mV,ֹͣ���*/
        reb_msg( REB_MSG_INFO, "REB_DFT:Fast on/off stop inspect\r\n");
        return STOP_INSPECT;
    }
    else
    {
        reb_msg( REB_MSG_DEBUG, "REB_DFT:Fast on/off continue inspect\r\n");
        return CONTINUE_INSPECT;
    }
}
/*****************************************************************************
 �� �� ��  : reb_for_fast_onoff_start
 ��������  : ������ʱ,���Ƿ����͹���,���û��,�������ػ�
 �������  : none
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void reb_for_fast_onoff_start(void)
{
    /*��ȡ�ٹػ�ʱ�̵ĵ�ص�ѹ*/
    volt_of_fast_off = chg_get_batt_volt_value();
    reb_msg( REB_MSG_INFO, "REB_DFT:Fast on/off current bat volt %umv\r\n",
        volt_of_fast_off );
    
    /*���üٹػ��µ�ؼ�����*/
    reset_bat_inspect_para();
#if ( YES == Reb_Platform_V7R2 )
    /*���ö�ʱ�ص�����*/
    g_hard_timer_call_func = reb_fast_onoff_inspect_callback;
#endif
    /*Ϊ��ֹ�ٹػ�ʱV���,20���Ժ�����״μ��,������volt_of_fast_off*/
    hard_timer_creat( FAST_ON_OFF_FST_CYCLE, REB_HARD_TIMER_OUT_EVENT );

    return;
}

/*****************************************************************************
 �� �� ��  : reb_for_fast_onoff_stop
 ��������  : ���ٿ���/(���ڹ�������)ֹͣ���ż��
 �������  : none
 �������  : none  
 �� �� ֵ  : none
*****************************************************************************/
void reb_for_fast_onoff_stop(void)
{
    /*���üٹػ��µ�ؼ�����*/
    reset_bat_inspect_para();
#if ( YES == Reb_Platform_V7R2 )
    g_hard_timer_call_func = NULL;
    hard_timer_delete();
#endif
#if ( YES == Reb_Platform_9x25 )
    reb_send_msg_to_main_task(REB_HARD_TIMER_STOP_EVENT);
#endif
    return;
}

/*****************************************************************************
 �� �� ��  : reb_fast_onoff_event_deal
 ��������  : �ɿ��Կ��ٿ��ػ��¼�����
 �������  : none
 �������  : none  
 �� �� ֵ  : none
*****************************************************************************/
int reb_fast_onoff_event_deal(struct notifier_block *nb, unsigned long val, void *data)
{
    int ret_val = 0;
    reb_msg( REB_MSG_ERR, "REB_DFT:reb_fast_onoff_event_deal %d\r\n", val );

    switch (val)
    {
        /*�˳��ٹػ�״̬*/
        case FASTONOFF_MODE_CLR:
            reb_for_fast_onoff_stop();
            break;
        /*����ٹػ�״̬*/
        case FASTONOFF_MODE_SET:
            /*���������*/
            reb_lock_sleep( 0 );
            if ( 0 == chg_is_bat_only() )
            {
                /*�ٹػ���粻���б���*/
                reb_msg( REB_MSG_ERR, "REB_DFT:Fast on/off Not Bat only!\r\n" );
            }
            else
            {
                reb_msg( REB_MSG_INFO, "REB_DFT:Fast on/off YES Bat only!\r\n" );
                reb_for_fast_onoff_start();
            }
            /*�������*/
            reb_unlock_sleep( 0 );
            break;
        default:
            ret_val = reb_err;
            break;
    }

    return ret_val;

}
/*****************************************************************************
 �� �� ��  : reb_fast_onoff_inspect_callback
 ��������  : Hard timer��ʱ�����ص�ѹ
 �������  : none
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void reb_fast_onoff_inspect_callback(void * para)
{
    BAT_VOLT_STA result = INVALID_VOLT;

    /*��������,��ֹ�������*/
    reb_lock_sleep( 0 );

    result = is_current_normal();
#ifdef REB_OS_DEBUG
    reb_msg( REB_MSG_DEBUG, "REB_DFT:Fast on/off %s return %d\r\n",
        __func__, result );
#endif

    switch (result)
    {
        case INVALID_VOLT:
            /*���ö�ʱ��*/
#if ( YES == Reb_Platform_V7R2 )
            hard_timer_reset( FAST_ON_OFF_CYCLE );
#endif
#if ( YES == Reb_Platform_9x25 )
           reb_send_msg_to_main_task(REB_HARD_TIMER_OUT_EVENT);
#endif
            break;
        case CONTINUE_INSPECT:  /*�������*/
            volt_inspect_cnt++;
#if ( YES == Reb_Platform_V7R2 )
            /*���ö�ʱ��*/
            hard_timer_reset( FAST_ON_OFF_CYCLE );
#endif
#if ( YES == Reb_Platform_9x25 )
           reb_send_msg_to_main_task(REB_HARD_TIMER_OUT_EVENT);
#endif
            break;

        case BAT_OVER_USE:
            /*���üٹػ��µ�ؼ�����*/
            reb_for_fast_onoff_stop();
            reb_send_msg_to_main_task(REB_FAST_OFF_BAT_OVERUSE);
            reb_delay_ms( 1000 );/*�ӳ�1��*/
            reb_for_power_off();
            break;
        case STOP_INSPECT:      /*Ҫֹͣ���*/
            /*���üٹػ��µ�ؼ�����*/
            reb_for_fast_onoff_stop();
            break;
        default:
            reb_msg( REB_MSG_ERR, "REB_DFT:Fast on/off is_current_normal ret err\r\n");
            break;
    }

    /*���������*/
    reb_unlock_sleep( 0 );
}

/*****************************************************************************
 �� �� ��  : reb_fast_off_inspect_init
 ��������  : ���ٹػ�����ص�ѹģ���ʼ��
 �������  : none
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void reb_fast_off_inspect_init(void)
{
    int ret_val = 0;

    /*�鿴�ٹػ�ʧ�ܽ�����ػ������Ƿ���*/
    if ( g_kernel_soft_reliable_info.FastOffFailTrueShutdownEnable )
    {
        /*ע����ٿ��ػ��¼�֪ͨ��*/
        ret_val = blocking_notifier_chain_register(&g_fast_on_off_notifier_list, &fast_on_event_notifier);
        if(ret_val)
        {
            reb_msg( REB_MSG_ERR, "REB_DFT:reb_fast_off_inspect_init fail\r\n");
        }
    }
}
#endif
#endif


/*****************************************************************************
 �� �� ��  : reb_write_partition_beyond
 ��������  : MTD flash�����Ƿ�дԽ��
 �������  : from-��ʼ��ַ;cnt_addr-Ҫд���ֽ�����(ջ)��ַ;
             part_name-��flash��������;part_size-��flash�����ߴ�
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void reb_write_partition_beyond(const long long from, unsigned int* cnt_addr,
                const char* part_name, unsigned int part_size )
{
    long long to = 0;

    if ( NULL == cnt_addr )
    {
        reb_msg( REB_MSG_ERR, "REB_DFT: cnt_addr is NULL \r\n" );
        return;
    }

    to = from + *cnt_addr;
    /*����NV���þ����Ƿ���*/
    if (g_kernel_soft_reliable_info.ZoneWriteProtectEnable)
    {
        if ( to > part_size )
        {
            *cnt_addr = 0;
            if ( part_name )
            {
                reb_msg( REB_MSG_ERR, 
                                    "REB_DFT: Write partition:%s ERROR\r\n", part_name );
            }
            reb_msg( REB_MSG_ERR, "REB_DFT: From:0x%08llX\r\n", from );
            reb_msg( REB_MSG_ERR, "REB_DFT: To  :0x%08llX\r\n", to  );
        }

        /*֪ͨ��������*/
        reb_send_msg_to_main_task( REB_PRT_BYND_EVENT );
    }
    else /*������������δ����*/
    {
        /*�����κδ���*/
        return;
    }
}
/*****************************************************************************
 �� �� ��  : reb_inspect_free_memory
 ��������  : ����ڴ��Ƿ��㹻
 �������  : from-��ʼ��ַ;cnt_addr-Ҫд���ֽ�����(ջ)��ַ;the_mtd-��flash������Ϣ
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void reb_inspect_free_memory(void)
{
    return;
}

/*****************************************************************************
 ��������  : void reb_sys_start_write_protect(void)
 ��������  : �����жϲ����������ļ�д��Ԥ������
 �������  : none
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void reb_sys_start_write_protect(char *name)
{
    static bool  write_protect = TRUE;
    unsigned long current_time = 0;
    reb_ret_val ret_val = reb_err;

    /*����ģʽ�ù��ܲ���Ч*/
    ret_val = is_dload_mod();
    if ( ( reb_err == ret_val ) || ( reb_true == ret_val ) )
    {
        /*���������ģʽ�ù��ܲ���Ч*/
        return;        
    }

    if(0 == reb_is_factory_mode())
    {
        return;
    }
    /*����NV���þ����Ƿ���*/
    if ( g_kernel_soft_reliable_info.BootNotOperaFileSysEnable)
    {
        if (FALSE == write_protect)
        {
            reb_msg( REB_MSG_DEBUG, 
                                 "Reb_DFT it should not start write protect function\n");
            return;
        }
        /*��ȡ��ǰ����ʱ��*/
        current_time = reb_get_current_systime();
        if (current_time < g_kernel_soft_reliable_info.DangerTime)
        {
            /*��ӡ������*/
            reb_msg( REB_MSG_ERR, "Reb_DFT write name %s\r\n", name);
        }
        else
        {
            write_protect = FALSE;
            reb_msg( REB_MSG_INFO, "Reb_DFT the write timer forbiden has passed\n");
        }

    }
    else /*�����ļ�д�������ܿ���*/
    {
        /*�����κδ���*/
        return;
    }
    return;
}
/*****************************************************************************
 ��������  :  void reb_file_sync(void)
 ��������  : ���ڹػ���������ʱ�ļ�ϵͳ��ͬ��
 �������  : none
 �������  : none
 �� �� ֵ  : none
*****************************************************************************/
void reb_file_sync(void)
{
    if(0 == reb_is_factory_mode())
    {
        return;
    }
    /*����NV���þ����Ƿ���*/
    if (g_kernel_soft_reliable_info.FileSysActiveProtectEnable)
    {
        g_stop_write = TRUE;
        reb_msg( REB_MSG_DEBUG, "Reb_DFT Syncing filesystems...\n");
        sys_sync();
        reb_msg( REB_MSG_DEBUG, "Reb_DFT done.\n");
        reb_delay_ms(500); /*�ӳ�500ms*/
    }
    return ;
}

/*****************************************************************************
 ��������  : bool reb_stop_write(void)
 ��������  : ��ȡ�Ƿ����дFLASH�ı�־
 �������  : None
 �������  : None
 �� �� ֵ  : g_stop_write
 �޸���ʷ  : 
             1. 2013-11-24 :  00206465 qiaoyichuan created
*****************************************************************************/
int reb_stop_write_file(void)
{
    return g_stop_write;
}


