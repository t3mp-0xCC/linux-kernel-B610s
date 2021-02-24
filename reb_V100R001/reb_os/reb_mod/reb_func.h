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
 /*----------------------------------------------*
 * �궨��                        *
 *----------------------------------------------*/
 /*�ٹػ����ű�������Ĭ��60����+10��,�Ա�����ģ���*/
#define FAST_ON_OFF_CYCLE (3610 * 1000)

/*����ٹػ����״ε�ص�ѹ���*/
#define FAST_ON_OFF_FST_CYCLE (20 * 1000)

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
void reb_write_partition_beyond(const long long from, unsigned int* cnt_addr, 
    const char* part_name, unsigned int part_size );
void reb_sys_start_write_protect(char *name);
void reb_file_sync(void);
int reb_stop_write_file(void);
void reb_fast_onoff_inspect_callback(void * para);
/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
typedef struct __SOFT_RELIABLE_CFG_STRU
{       
    unsigned char     FileSysActiveProtectEnable;   /* �ļ�ϵͳ��������ʹ�� */     
    unsigned char     BadImgResumFromOnlineEnable;   /* �����𻵺��Online�����ָ�ʹ��*/     
    unsigned char     BootNotOperaFileSysEnable;      /*ϵͳ�������̲����ļ�ϵͳ����ʹ��*/     
    unsigned char     FastOffFailTrueShutdownEnable;    /*�ٹػ�ʧ�ܽ�����ػ�����ʹ��*/      
    unsigned char     SoftPowOffReliableEnable;          /*����ػ��ɿ��Թ���ʹ��*/      
    unsigned char     ZoneWriteProtectEnable;          /*����дԽ�籣��ʹ��*/       
    unsigned char     BadZoneReWriteEnable;            /* Flash��Σ���д����ʹ��*/       
    unsigned char     BootCanFromkernelBEnable;  /*�������ƻ�����Դӱ��ݾ�������ʹ��*/
    unsigned char     OnlineChangeNotUpgradEnable; /*Online�����仯��������������ʹ��*/
    /*Ԥ��*/
    unsigned char     BadZoneScanEnable;   /* Flash��Σ��ɨ�蹦��ʹ��*/    
    unsigned char     reserved2;
    unsigned char     reserved3;
    unsigned char     reserved4;
    unsigned char     reserved5;
    unsigned char     reserved6;
    unsigned char     MmcReliabilityEnable;            /*Э��MMC�ɿ��Ա���ʹ�ܿ���*/
    unsigned int      MmcReliabilityBitmap;     /* ͨ��Э��Э��MMC�ɿ��Ա����������� */
    unsigned int      DangerTime;                   /*Ƶ�����µ�Σ���ھ���ֵ ����λΪ��*/       
    unsigned int      WaitTime;           /*Ӧ��֪ͨ������ٹػ��󣬶�ʱ��ֵ��λΪ��*/
    /*Ԥ��*/
    int         reserved7; 
    int         reserved8; 
    int         reserved9; 
    int         reserved10; 
    int         reserved11;       
}SOFT_RELIABLE_CFG_STRU;
/*----------------------------------------------*
 * �ⲿ����                                     *
 *----------------------------------------------*/
extern SOFT_RELIABLE_CFG_STRU g_kernel_soft_reliable_info;
