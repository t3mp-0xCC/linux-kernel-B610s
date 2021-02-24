/*
* Copyright (c) 1988-2014 by Huawei Technologies Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/
 
#ifndef SLIC_CTL_H
#define SLIC_CTL_H

#include <osl_sem.h>
#include <sound/soc.h>
#include "vp_api_types.h"
#include "product_config.h"

#define MAX_ONE_TRANSIZE (32)
#define MAX_DATA_SIZE    (80)
#define KEY_CODE_NUM     (19)
#define KEY_VAULE        (-1800)
#define Line_Num         (1 )            /* slicоƬͨ����*/
#define ZSI_4WIRE        (0x0E00)        /* SPI data on a 4 wire interface muxed with PCM data */

#define SLIC_TASK_PRIO   (98)            /* slic ��ѵ��������ȼ����ں����Ϊ99*/
#define SLIC_FAST_TIME   (10)            /* ժ����Ŀ���ѵʱ�䣬10ms*/

/*�����ṩ����dtmf����dBmֵ*/
#define SHIFT_OFFSET      (8)          /*ȡֵ�ĸ�8λ*/
#define MULT_VALUE        (25)         /*����*/
#define BASE_VALUE        (5375)       /*��������*/

#define GET_dBm_VALUE(x)  (((x >> SHIFT_OFFSET) & 0x00ff) * MULT_VALUE - BASE_VALUE)

/*���Ҷ�Ӧ�ı���*/
typedef enum tagREGION_CODING_E
{
    REGION_Belgium     =   0,          
    REGION_Brazil      =   1,
    REGION_Chile       =   2,             
    REGION_China       =   3,
    REGION_Denmark     =   4,           
    REGION_ETSI        =   5,
    REGION_Finland     =   6,           
    REGION_France      =   7,
    REGION_Germany     =   8,           
    REGION_Hungary     =   9,
    REGION_Italy       =   10,            
    REGION_Japan       =   11,
    REGION_Netherlands =   12,      
    REGION_UnitedStates=   13,
    REGION_Spain       =   14,            
    REGION_Sweden      =   15,
    REGION_Switzerland =   16,      
    REGION_TR57        =   17,
    REGION_GreatBritain=   18,    
    REGION_Singapore   =   19,
    REGION_Russia      =   20,
    REGION_Poland      =   21,
    REGION_Australia   =   22,

    //�����������µĹ�����.
    
    REGION_COUNT,
}REGION_CODING_E;
/*mbb ʹ��REGION_CODING_E����ʹ������ö��*/
typedef enum
{
    COUNTRY_CN = 0,      /*�й�(China)*/
    COUNTRY_COUNT
}Country_Type;

/* Tone enum type ������Ӧ�ò㱣��һ�� */
typedef enum{
    SLIC_NONE_TONE = 0,                     /* none */
    SLIC_RING,                              /* Ring */
    SLIC_RING_FOR_MONITOR,                  /* Ring for houtai Monitor */
    SLIC_KEEP_RING,                         /* Keep Ring */
    SLIC_DIAL_TONE,                         /* Dial tone */
    SLIC_BUSY_TONE,                         /* Busy tone */
    SLIC_ALARM_TONE,                        /* Alarm tone */
    SLIC_TIP_TONE,                          /* Succeed/fail tone */
    SLIC_FAULT_TONE,                        /* Fault tone */
    SLIC_CALL_WAITING_TONE,                 /* call waiting tone */
    SLIC_RINGBACK_TONE,                     /* ring back tone */
    SLIC_CONFIRMATION_TONE,                 /* Confirmation tone */
    SLIC_VM_SHUTTER_TONE,                   /* Voice Mail Stutter Tone */
    SLIC_CALL_FWD_RMD_TONE,                 /* Call forward reminder tone */
    SLic_SVC_ENABLE_TONE,                   /* service enable tone*/
    SLic_SVC_DISABLE_TONE,                  /* sevice disable tone*/
    SLIC_SUCCEED_TONE,
    SLIC_FAIL_TONE,   
    SLIC_CONGESTION_TONE,                   /* congestion tone */

} slic_tone_enum_type;

typedef struct{
    VpProfileDataType* RingProf;
    VpProfileDataType* RingFSKCandProf;
    VpProfileDataType* RingDTMFCandProf;
    VpProfileDataType* DailToneProf;
    VpProfileDataType* DailCandProf;
    VpProfileDataType* RBackToneProf;        /*RingBack Tone*/
    VpProfileDataType* RBackCandProf;
    VpProfileDataType* BusyToneProf;
    VpProfileDataType* BusyCandProf;
    VpProfileDataType* CWaitToneProf;        /*CallWaiting Tone*/
    VpProfileDataType* CWaitCandProf;
    VpProfileDataType* CongeToneProf;         /*congestion Tone*/
    VpProfileDataType* CongeToneCandProf;
    VpProfileDataType* alarmToneProf;         
    VpProfileDataType* alarmToneCandProf;    
    VpProfileDataType* SvcEnableToneProf;  
    VpProfileDataType* SvcEnableCandToneProf;    
    VpProfileDataType* SvcDisableToneProf;
    VpProfileDataType* SvcDisableCandToneProf;
    VpProfileDataType* faultToneProf;         
    VpProfileDataType* faultToneCandProf;    
    VpProfileDataType* VoiceMailToneProf;     /*voice mail Tone*/        
    VpProfileDataType* VoiceMailToneCandProf;    	

}tone_profile_type;

struct slic_profile_type{
    VpProfileDataType*      DCProf;
    VpProfileDataType*      ACNbProf;          /*Narrow band  AC Profile*/
    VpProfileDataType*      ACWbProf;          /*Wide band AC profile*/
    VpProfileDataType*      CIDDtmfprof;
    VpProfileDataType*      CIDFsk1Prof;       /*Type_I  CallerID*/
    VpProfileDataType*      CIDFsk2Prof;       /*Type_II CallerID*/
    tone_profile_type       ToneProf;
};

typedef enum{
    CID_NONE = 0,
    CID_DTMF,
    CID_FSK,
    CID_MAX
} slic_cid_mode;



struct slic_rkey_time{
    uint32 rkey_time_min;
    uint32 rkey_time_max;
};


/* ����������չ���� */
struct slic_customize_ex_parm_type{
    uint8 callid_first_signal; 
    struct slic_rkey_time rkey_time;
};


/* �����㣬DTMF��ֵ���壬Ӧ�ò�ҲӦ��˱���һ�� */
typedef enum {
    SLIC_NONE_K = 0x00,
    /**< -- Deprecated. */
    SLIC_POUND_K = 0x23,
    /**< -- # key, ASCII #. */
    SLIC_STAR_K = 0x2A,
    /**< -- * key, ASCII *. */
    SLIC_0_K = 0x30,
    /**< -- 0 key, ASCII 0. */
    SLIC_1_K = 0x31,
    /**< -- 1 key, ASCII 1. */
    SLIC_2_K = 0x32,
    /**< -- 2 key, ASCII 2. */
    SLIC_3_K = 0x33,
    /**< -- 3 key, ASCII 3. */
    SLIC_4_K = 0x34,
    /**< -- 4 key, ASCII 4. */
    SLIC_5_K = 0x35,
    /**< -- 5 key, ASCII 5. */
    SLIC_6_K = 0x36,
    /**< -- 6 key, ASCII 6. */
    SLIC_7_K = 0x37,
    /**< -- 7 key, ASCII 7. */
    SLIC_8_K = 0x38,
    /**< -- 8 key, ASCII 8. */
    SLIC_9_K = 0x39,
    /**< -- 9 key, ASCII 9. */
    SLIC_A_K = 0x41,
    /**< -- A key, ASCII A. */
    SLIC_B_K = 0x42,
    /**< -- B key, ASCII B. */
    SLIC_C_K = 0x43,
    /**< -- C key, ASCII C. */
    SLIC_D_K = 0x44,
    /**< -- D key, ASCII D. */
    SLIC_RKEY_K = 0xF7, /*R��*/
    SLIC_HANGUP_K = 0xF8, /*ժ����*/
    SLIC_HANGON_K = 0xF9,  /*�һ���*/
    SLIC_RELEASE_K = 0xFF,
} slic_key_code;




/*
 * MAX_USER_BUFFER is the size of the user's CLI buffer. The API's CLI buffer
 * will be MAX_USER_BUF * 10 / 6 + 2.
 */
#define MAX_USER_BUFFER 100
#define MAX_API_BUFFER (MAX_USER_BUFFER * 10 / 6 + 2)

/* ����FSK �������͵ĸ�ʽ */
#define CLI_SDMF            4       /*�����ݸ�ʽ������������*/    
#define CLI_MDMF            0x80     /*�������ݸ�ʽ��������*/
#define CLI_PARAM_DATETIME  1       /*����ʱ��*/
#define CLI_PARAM_NUMBER    2       /*���к���*/
#define CLI_PARAM_NO_NUMBER 4       /*�����к���*/
#define CLI_PARAM_NAME      7       /*��������*/
#define CLI_PARAM_NO_NAME   8       /*����������*/

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!��Ҫ!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  ===================�޸���������ʱһ��Ҫ��Ӧ��̬����,start============*/
#define MAX_DATE_LENGTH  (12)
#define MAX_CID_LENGTH   (32)
#define MAX_NAME_LENGTH  (20)

/*slic_ioctl�ṹ���ܴ�С���ܳ���512�ֽ�,�������������������ӵ��������
  ���ܱ�֤����ƽ̨�ļ�����*/
struct slic_ioctl{    
    uint8        date_time[MAX_DATE_LENGTH];     /* ����ʱ�� */
    uint8        cid_num[MAX_CID_LENGTH]; /* ������� */
    uint8        name[MAX_NAME_LENGTH]; 
    uint8        tone;              /* �������� ��slic_tone_enum_type��ʾ*/
    uint8        mode;              /*������ʾģʽ��ö��slic_cid_mode��ʾ*/
    uint8        secret;            /* �Ƿ�������� */
    uint8        inout_band;        /*����dtmf�ߴ��ڴ���*/
    uint8        band_switch;       /*��խ���л���voipģʽ��ͨ�������ã�
                                      cs��Э�������ϱ�ʱ���� */
};

typedef enum{
    SLIC_CTRL_NONE   = 0,
    SLIC_SOUND_START = 1,
    SLIC_SOUND_STOP  = 2,
    SLIC_CID_IN_TALK = 3,                  /*ͨ���У��������绰���뷢������*/
    SLIC_DTMF_INOUT_BAND  = 4,             /*����dtmf�ߴ��ڴ���*/
    SLIC_VOIP_BAND_SWITCH = 5,             /*voipͨ��ǰ�����¿�խ��*/
    SLIC_CTRL_COUNT,
}slic_ctl_type;

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!��Ҫ!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  ===================�޸���������ʱһ��Ҫ��Ӧ��̬����,end============*/

struct slic_private{
    struct input_dev         *input_dev;
    struct slic_ioctl        *slic_ctl;
    osl_sem_id                ioctl_sem;
    osl_sem_id                event_sem;
    osl_sem_id                code_sem;
    osl_sem_id                mpi_sem;
    u16                       keycodes[KEY_CODE_NUM];
    u8                        cid_buff[MAX_USER_BUFFER];
    u8                        cid_length;
    u8                        pos;
    u32                       dtmf_enable;
};

enum{
    TYPE_RING_CMD     = 0,    /*���ڲ�����������*/
    TYPE_LOOPBACK_CMD,        /*���ڲ����������ز�������*/
    TYPE_MAX
};

enum{
    OFF_CMD = 0,              /*�������*/
    ON_CMD,           /*�����*/
    ONONFF_MAX
};

extern struct slic_private* slic_priv;


int slic_cmd_ctrl(u32 cmd, struct slic_ioctl *slic_ctl);
int slic_probe(struct snd_soc_codec *codec);
int slic_remove(void);
void slic_sound_start_test(u8 tone,u8 mode,u8 secret);

extern void send_mute_dtmf_to_med(void);
extern void AT_RegisterSlicATSetAudioEnable(void *handle);

#ifdef CONFIG_SLIC_AMR_WB
#define BAND_SWITCH_TO_WIDE    (1)
#define BAND_SWITCH_TO_NARROW  (0)
int slic_band_switch(int type);
extern void AT_RegisterSlicChanneSet(void *handle);
#endif

int SlicATSetAudioEnable(unsigned int channel, unsigned int type, unsigned int onoff);
extern void AT_RegisterSlicATSelfTest(void *handle);
int is_slic_init_ok(void);
#endif

