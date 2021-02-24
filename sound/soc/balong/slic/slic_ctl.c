/*
 * slic_ctl.c  --  LE9662 ALSA Soc codec driver
 *
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
/*lint -save -e21 -e64 -e734 -e737 -e958*/
 
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/of_device.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include "osl_common.h"
#include "bsp_softtimer.h"
#include "vp_api.h"
#include "slic_profile.h"
#include "spi.h"
#include "snd_param.h"
#include "slic_ctl.h"
#ifdef CONFIG_SLIC_AMR_WB
#include "sio.h"
#endif
/****************************globle vars**************************************/

#define ONE_SECOND              1000000                /* 1s = 1000000us */
#define DTMF_ONTIME_THRESHOLD   60000                  /* DTMF should keep 60ms above*/

VpDevCtxType          DevCtx;                          /* Create generic device context */
VpLineCtxType         LineCtx[Line_Num];               /* Create generic line context */


struct slic_private*    slic_priv = NULL;
struct softtimer_list   slic_softtimer;

 /*表示是否需要开启hifi插除dtmf功能*/                                      
static int   ulHifiEnable = 0;
static int   no_send_flag = 0;
static int   is_init_ok   = -1;

static int     country  = -EINVAL;
module_param(country, int, S_IRUGO);
MODULE_PARM_DESC(country, "country type pass from app");

/*
 *MBB产品默认是9641芯片，不需要上层传递参数
static int slic_ver_param  = 9641;
module_param(slic_ver_param, int, S_IRUGO);
MODULE_PARM_DESC(slic_ver_param, "slic version pass from app");
 */

/**********************************Structure****************************************/
u8 *vpStatusString[] = {
    "VP_STATUS_SUCCESS",
    "VP_STATUS_FAILURE",
    "VP_STATUS_FUNC_NOT_SUPPORTED",
    "VP_STATUS_INVALID_ARG",
    "VP_STATUS_MAILBOX_BUSY",
    "VP_STATUS_ERR_VTD_CODE",
    "VP_STATUS_OPTION_NOT_SUPPORTED",
    "VP_STATUS_ERR_VERIFY",
    "VP_STATUS_DEVICE_BUSY",
    "VP_STATUS_MAILBOX_EMPTY",
    "VP_STATUS_ERR_MAILBOX_DATA",
    "VP_STATUS_ERR_HBI",
    "VP_STATUS_ERR_IMAGE",
    "VP_STATUS_IN_CRTCL_SECTN",
    "VP_STATUS_DEV_NOT_INITIALIZED",
    "VP_STATUS_ERR_PROFILE",
    "VP_STATUS_CUSTOM_TERM_NOT_CFG",
    "VP_STATUS_DEDICATED_PINS",
    "VP_STATUS_INVALID_LINE",
    "VP_STATUS_LINE_NOT_CONFIG",
    "VP_STATUS_ERR_SPI",
    "VP_STATUS_INPUT_PARAM_OOR",
    "VP_STATUS_MAX_NUM_TYPES"
};

const u16 slic_keycodes[KEY_CODE_NUM] = {
    SLIC_0_K, SLIC_1_K, SLIC_2_K, SLIC_3_K,
    SLIC_4_K, SLIC_5_K, SLIC_6_K, SLIC_7_K,
    SLIC_8_K, SLIC_9_K, SLIC_A_K, SLIC_B_K,
    SLIC_C_K, SLIC_D_K,
    SLIC_STAR_K,
    SLIC_POUND_K,
    SLIC_HANGON_K,
    SLIC_HANGUP_K,
    SLIC_RKEY_K
};

#if (FEATURE_ON == MBB_SLIC)
const struct slic_customize_ex_parm_type slic_customize_ex_parm[] =
{
    {0}, /*0 REGION_Belgium*/
    {0}, /*1 REGION_Brazil*/
    {0}, /*2 REGION_Chile*/
    
    /*3 China */
    {
        'D',     /* CID DTMF first signal*/       
        {0,700}  /* min flash time & max flash time */
    },

    {0}, /*4 REGION_Denmark*/
    {0}, /*5 REGION_ETSI*/
    {0}, /*6 REGION_Finland*/
    {0}, /*7 REGION_France*/

    /*8 Germany */
    {
        'D',     /* CID DTMF first signal*/       
        {300,800}  /* min flash time & max flash time */
    },

    {0}, /*9 REGION_Hungary*/
    {0}, /*10 REGION_Italy*/
    {0}, /*11 REGION_Japan*/
    {0}, /*12 REGION_Netherlands*/
    {0}, /*13 REGION_UnitedStates*/
    {0}, /*14 REGION_Spain*/
    {0}, /*15 REGION_Sweden*/
    {0}, /*16 REGION_Switzerland*/
    {0}, /*17 REGION_TR57*/
    {0}, /*18 REGION_GreatBritain*/
    {0}, /*19 REGION_Singapore*/
    {0}, /*20 REGION_Russia*/
    
    /*21 Poland */
    {
        'D',     /* CID DTMF first signal*/       
        {0,750}  /* min flash time & max flash time */
    },
    
    /*22 REGION_Australia*/
    {
        'D',     /* CID DTMF first signal*/       
        {0,750}  /* min flash time & max flash time */
    },

    /*23 XX country 适配新的国家请在此处增加 */
    
};

struct slic_profile_type slic_profile[] =
{    
    {0}, /*0 REGION_Belgium*/
    {0}, /*1 REGION_Brazil*/
    {0}, /*2 REGION_Chile*/

    /*3 REGION_China */
    {
         /*DC Profile*/ 
         DC_FXS_MISLIC_ABS100V_DEF,

         /* Narrow band AC profile & Wide band AC profile*/
         AC_FXS_RF14_CN,
         AC_FXS_RF14_WB_CN,
         
         /* Caller ID profile for CID_DTMF, CID1_FSK, CID2_FSK*/
         CID_TYPE_DTMF_CN,
         CID_TYPE1_CN,
         CID_TYPE2_CN,

         /* Tone profile*/
         {
             /* 铃声 Ring profile & Ring Cadence profile*/
             RING_VE960_ABS100V_DEF, CR_FSK_CN,CR_DTMF_CN,
             /* 拨号音 Dial Tone profile & Dial Tone Cadence profile*/
             T_DL_CN, CT_DL_CN,
             /* 回铃音 Ringback Tone profile & Ringback Tone Cadence profile*/
             T_RB_CN, CT_RB_CN,
             /*忙音 Busy Tone profile & Busy Tone Cadence profile*/ 
             T_BS_CN, CT_BS_CN,
             /* 呼叫等待音 Call waiting Tone profile & Call waiting Tone Cadence profile*/
             T_CW_CN, CT_CW_CN,
             /* 阻塞音 Congestion Tone profile & Congestion Tone Cadence profile*/
             T_CG_CN, CT_CG_CN,
             /* 嚎鸣音 Alarm Tone profile & Alarm Tone Cadence profile*/
             T_ALARM_CN, CT_ALARM_CN, 
             /* 业务启用音 Service Enable Tone profile & Service Enable Tone Cadence profile*/
             T_SVCEN_CN, CT_SVCEN_CN,
             /* 业务禁用音 Service Disable Tone profile & Service Disable Tone Cadence profile*/
             T_SVCDIS_CN, CT_SVCDIS_CN,
             /* 错误音 Fault Tone profile & Fault Tone Candence profile*/
             T_FAULT_CN, CT_FAULT_CN,             
             T_DL_VM_CN,CT_DL_VM_CN
         }
    },

    {0}, /*4 REGION_Denmark*/
    {0}, /*5 REGION_ETSI*/
    {0}, /*6 REGION_Finland*/
    {0}, /*7 REGION_France*/

    /*8 REGION_Germany */
    {
         /* DC Profile*/
         DC_FXS_MISLIC_ABS100V_DEF,

         /* Narrow band AC profile & Wide band AC profile*/
         AC_FXS_RF14_DE,
         AC_FXS_RF14_WB_DE,
         
         /* Caller ID profile for CID_DTMF, CID1_FSK, CID2_FSK*/
         CID_TYPE_DTMF_CN,
         CID_TYPE1_DE,
         CID_TYPE2_DE,

         /* Tone profile*/
         {
             /* 铃声 Ring profile & Ring Cadence profile*/
             RING_VE960_ABS100V_DEF, CR_DE,CR_DTMF_CN,  /*DTMF的格式先默认用中国的，如有特殊定制，请修改成对应国家的*/
             /* 拨号音 Dial Tone profile & Dial Tone Cadence profile*/
             T_DL_DE, CT_DL_DE,
             /* 回铃音 Ringback Tone profile & Ringback Tone Cadence profile*/
             T_RB_DE, CT_RB_DE,
             /* 忙音 Busy Tone profile & Busy Tone Cadence profile*/
             T_BS_DE, CT_BS_DE,
             /* 呼叫等待音 Call waiting Tone profile & Call waiting Tone Cadence profile*/
             T_CW_DE, CT_CW_DE,
             /* 阻塞音 Congestion Tone profile & Congestion Tone Cadence profile*/
             T_CG_DE, CT_CG_DE,
             /* 嚎鸣音 Alarm Tone profile & Alarm Tone Cadence profile*/
             T_ALARM_DE, CT_ALARM_DE, 
             /* 业务启用音 Service Enable Tone profile & Service Enable Tone Cadence profile*/
             T_SVCEN_DE, CT_SVCEN_DE,
             /* 业务禁用音 Service Disable Tone profile & Service Disable Tone Cadence profile*/
             T_SVCDIS_DE, CT_SVCDIS_DE,
             /* 错误音 Fault Tone profile & Fault Tone Candence profile*/
             T_BS_DE, CT_BS_DE,             
             T_DL_VM_CN,CT_DL_VM_CN
         }
    },

    {0}, /*9 REGION_Hungary*/
    {0}, /*10 REGION_Italy*/
    {0}, /*11 REGION_Japan*/
    {0}, /*12 REGION_Netherlands*/
    {0}, /*13 REGION_UnitedStates*/
    {0}, /*14 REGION_Spain*/
    {0}, /*15 REGION_Sweden*/
    {0}, /*16 REGION_Switzerland*/
    {0}, /*17 REGION_TR57*/
    {0}, /*18 REGION_GreatBritain*/
    {0}, /*19 REGION_Singapore*/
    {0}, /*20 REGION_Russia*/

    /*21 Poland */
    {
         /* DC Profile*/
         DC_FXS_MISLIC_ABS100V_DEF,

         /* Narrow band AC profile & Wide band AC profile*/
         AC_FXS_RF14_ETSI,
         AC_FXS_RF14_WB_ETSI,
         
         /* Caller ID profile for CID_DTMF, CID1_FSK, CID2_FSK*/
         CID_TYPE_DTMF_CN,
         CID_TYPE1_DE,
         CID_TYPE2_DE,

         /* Tone profile*/
         {
             /* 铃声 Ring profile & Ring Cadence profile*/
             RING_VE960_ABS100V_DEF, CR_DE,CR_DTMF_CN, /*DTMF的格式先默认用中国的，如有特殊定制，请修改成对应国家的*/
             /* 拨号音 Dial Tone profile & Dial Tone Cadence profile*/
             T_DL_PL, CT_DL_PL,
             /* 回铃音 Ringback Tone profile & Ringback Tone Cadence profile*/
             T_RG_PL, CT_RB_PL,
             /* 忙音 Busy Tone profile & Busy Tone Cadence profile*/
             T_BS_PL, CT_BS_PL,
             /* 呼叫等待音 Call waiting Tone profile & Call waiting Tone Cadence profile*/
             T_CW_PL, CT_CW_PL,
             /* 阻塞音 Congestion Tone profile & Congestion Tone Cadence profile*/
             T_CG_PL, CT_CG_PL,
             /* 嚎鸣音 Alarm Tone profile & Alarm Tone Cadence profile*/
             T_ALARM_DE, CT_ALARM_DE, 
             /* 业务启用音 Service Enable Tone profile & Service Enable Tone Cadence profile*/
             T_SVCEN_DE, CT_SVCEN_DE,
             /* 业务禁用音 Service Disable Tone profile & Service Disable Tone Cadence profile*/
             T_SVCDIS_DE, CT_SVCDIS_DE,
             /* 错误音 Fault Tone profile & Fault Tone Candence profile*/
             T_BS_PL, CT_BS_PL,             
             T_DL_VM_CN,CT_DL_VM_CN
         }
    },
    
     /*22 REGION_Australia*/
    {
         /* DC Profile*/
         DC_FXS_MISLIC_ABS100V_DEF,

         /* Narrow band AC profile & Wide band AC profile*/
         AC_FXS_RF14_AU,
         AC_FXS_RF14_WB_AU,
         
         /* Caller ID profile for CID_DTMF, CID1_FSK, CID2_FSK*/
         CID_TYPE_DTMF_CN,
         CID_TYPE1_AU,
         CID_TYPE2_AU,

        /* Tone profile*/
         {
             /* 铃声 Ring profile & Ring Cadence profile*/
             RING_VE960_ABS100V_DEF, CR_AU,CR_DTMF_CN, /*DTMF的格式先默认用中国的，如有特殊定制，请修改成对应国家的*/
             /* 拨号音 Dial Tone profile & Dial Tone Cadence profile*/
             T_DL_AU, CT_DL_AU,
             /* 回铃音 Ringback Tone profile & Ringback Tone Cadence profile*/
             T_RB_AU, CT_RB_AU,
             /* 忙音 Busy Tone profile & Busy Tone Cadence profile*/
             T_BS_AU, CT_BS_AU,
             /* 呼叫等待音 Call waiting Tone profile & Call waiting Tone Cadence profile*/
             T_CW_AU, CT_CW_AU,
             /* 阻塞音 Congestion Tone profile & Congestion Tone Cadence profile*/
             T_CG_AU, CT_CG_AU,
             /* 嚎鸣音 Alarm Tone profile & Alarm Tone Cadence profile*/
             T_ALARM_DE, CT_ALARM_DE, 
             /* 业务启用音 Service Enable Tone profile & Service Enable Tone Cadence profile*/
             T_SVCEN_DE, CT_SVCEN_DE,
             /* 业务禁用音 Service Disable Tone profile & Service Disable Tone Cadence profile*/
             T_SVCDIS_DE, CT_SVCDIS_DE,
             /* 错误音 Fault Tone profile & Fault Tone Candence profile*/
             T_BS_AU, CT_BS_AU,             
             T_DL_VM_CN,CT_DL_VM_CN
         }
    },

    /*23 XX country 适配新的国家请在此处增加 */
};

#else  /*MBB_SLIC == FEATURE_OFF*/
const struct slic_customize_ex_parm_type slic_customize_ex_parm[1] =
{
    /*China --这里只列举中国，完整配置请产品线适配*/
    {'D',{0,700}}

};

struct slic_profile_type slic9662_profile = {
     ZLR9662_DC_FXS_MISLIC_ABS100V_DEF,ZLR9662_AC_FXS_RF14_600R_DEF,
     ZLR9662_AC_FXS_RF14_WB_CN,ZLR9662_CID_TYPE_DTMF_CN,
     ZLR9662_CID_TYPE1_CN,ZLR9662_CID_TYPE2_CN,
     {
     ZLR9662_RING_VE960_ABS100V_DEF,ZLR9662_CR_CN,ZLR9662_T_DL_CN,
     ZLR9662_CT_DL_CN,ZLR9662_T_RB_CN,ZLR9662_CT_RB_CN,ZLR9662_T_BS_CN,ZLR9662_CT_BS_CN,
     ZLR9662_T_CW_CN,ZLR9662_CT_CW_CN,ZLR9662_T_CG_CN,ZLR9662_CT_CG_CN,ZLR9662_T_ALARM_CN,
     ZLR9662_CT_ALARM_CN, ZLR9662_T_SVCEN_CN, ZLR9662_CT_SVCEN_CN,
     ZLR9662_T_SVCDIS_CN, ZLR9662_CT_SVCDIS_CN
     }
};

struct slic_profile_type slic9641_profile = {
     ZLR9641_DC_FXS_MISLIC_ABS100V_DEF,ZLR9641_AC_FXS_RF14_CN,
     ZLR9641_AC_FXS_RF14_WB_CN,ZLR9641_CID_TYPE_DTMF_CN,
     ZLR9641_CID_TYPE1_CN,ZLR9641_CID_TYPE2_CN,
     {
     ZLR9641_RING_VE960_ABS100V_DEF,ZLR9641_CR_CN,ZLR9641_T_DL_CN,
     ZLR9641_CT_DL_CN,ZLR9641_T_RB_CN,ZLR9641_CT_RB_CN,ZLR9641_T_BS_CN,ZLR9641_CT_BS_CN,
     ZLR9641_T_CW_CN,ZLR9641_CT_CW_CN,ZLR9641_T_CG_CN,ZLR9641_CT_CG_CN,ZLR9641_T_ALARM_CN,
     ZLR9641_CT_ALARM_CN, ZLR9641_T_SVCEN_CN, ZLR9641_CT_SVCEN_CN,
     ZLR9641_T_SVCDIS_CN, ZLR9641_CT_SVCDIS_CN
     }
};

/*China --这里只列举中国，完整配置请产品线适配*/
struct slic_profile_type slic_profile[1];

#endif


/************************************Function******************************************/

void slic_create_fsk_buff(struct slic_ioctl* slic_ctl)
{
    u32   date_length = 0;
    u32   cid_length  = 0;
    u32   name_length = 0;
    u32   mdmf        = FALSE;
    u32   Index       = 2;
    u8*   cid_buff    = slic_priv->cid_buff;


    /*计算传入参数中时间、号码和姓名字符的长度，单位:byte*/
    if( slic_ctl->date_time)
    {
        date_length = strlen(slic_ctl->date_time );
    }

    if( slic_ctl->cid_num)
    {
        cid_length = strlen(slic_ctl->cid_num );
    }

    if( slic_ctl->name)
    {
        name_length = strlen(slic_ctl->name );
    }

    /*如果包含姓名，则是复合数据格式:mdmf*/
    if( 0 != name_length )
    {
        mdmf = TRUE;
    }

    /* 填充日期和时间参数 */
    if( TRUE == mdmf )
    {
        cid_buff[Index++] = CLI_PARAM_DATETIME; /* 参数类型 */
        cid_buff[Index++] = (u8)date_length;        /* 参数长度 */
    }

    /*拷贝日期时间*/
    strncpy((s8*)&cid_buff[Index],(s8*)slic_ctl->date_time,date_length);
    Index += date_length;

    /* 如果不允许发送号码 */
    if( TRUE == slic_ctl->secret )
    {
        if( TRUE == mdmf )
        {
            cid_buff[Index++] = CLI_PARAM_NO_NUMBER;
            cid_buff[Index++] = 0x1;
        }
        cid_buff[Index++] = 'P';       /*隐藏号码时发送'P'*/
    }
    else
    {
        if( TRUE == mdmf )
        {
            cid_buff[Index++] = CLI_PARAM_NUMBER;
            cid_buff[Index++] = (u8)cid_length;
        }
        /*拷贝电话号码*/
        strncpy((s8*)&cid_buff[Index],(s8*)slic_ctl->cid_num,cid_length);
        Index += cid_length;
    }

    /* 如果是符合数据格式，拷贝姓名 */
    if( TRUE == mdmf )
    {
        cid_buff[Index++] = CLI_PARAM_NAME;
        cid_buff[Index++] = (u8)name_length;
        strncpy((s8*)&cid_buff[Index],(s8*)slic_ctl->name,name_length);
        Index += name_length;
    }

    /* 拷贝完成加上结束符 */
    cid_buff[Index] = '\0';

    /* 在数据前加上消息类型数据格式 */
    if( TRUE == mdmf )
    {
        cid_buff[0] = CLI_MDMF;
    }
    else
    {
        cid_buff[0] = CLI_SDMF;
    }
    cid_buff[1] = (u8)(Index - 2);
    slic_priv->cid_length = (u8)Index;

}

void slic_create_dtmf_buff(struct slic_ioctl* slic_ctl)
{
    u32  cid_length = 0;
    u8*  cid_buff   = slic_priv->cid_buff;

    if( slic_ctl->cid_num )
    {
        cid_length = strlen(slic_ctl->cid_num);
    }

    /*DTMF制式下不显示号码或者号码为空*/
    if( TRUE == slic_ctl->secret || 0 == cid_length )
    {
        slic_priv->cid_length = 0;
        return;
    }
    /*添加定制号码前缀*/
    cid_buff[0] = slic_customize_ex_parm[country].callid_first_signal;

    /*如果第一个号码是'+' 号*/
    if( '+' == slic_ctl->cid_num[0])
    {
        slic_ctl->cid_num[0] = 0x30;
    }
    /*拷贝电话号码*/
    strncpy((s8*)&cid_buff[1],(s8*)slic_ctl->cid_num,cid_length);
    cid_buff[1+cid_length] = 'C';
    cid_buff[2+cid_length] = '\0';

    slic_priv->cid_length = (u8)(cid_length + 2);
    return;
}

void slic_get_callerid(struct slic_ioctl* slic_ctl)
{

    memset(slic_priv->cid_buff,0x0,MAX_USER_BUFFER);

    if(CID_DTMF == slic_ctl->mode)
    {
        slic_create_dtmf_buff(slic_ctl);
    }
    else if(CID_FSK == slic_ctl->mode)
    {

        slic_create_fsk_buff(slic_ctl);
    }
    slic_priv->pos = 0;

}

VpStatusType slic_set_mode(u8 mode)
{
    VpStatusType status = VP_STATUS_SUCCESS;

    if(CID_DTMF == mode)
    {
        status = VpInitRing(&LineCtx[0],slic_profile[country].ToneProf.RingDTMFCandProf,\
                   slic_profile[country].CIDDtmfprof);
        printk(KERN_ERR"CID mode:DTMF\n");
    }
    else if(CID_FSK == mode)
    {
        status = VpInitRing(&LineCtx[0],slic_profile[country].ToneProf.RingFSKCandProf,\
                   slic_profile[country].CIDFsk1Prof);
        printk(KERN_ERR"CID mode:FSK\n");
    }
    else
    {
        printk(KERN_ERR"err mode,mode:%d\n",mode);
        return -EINVAL;
    }

    if(VP_STATUS_SUCCESS != status)
    {
        printk(KERN_ERR"SLIC_CID_MODE failed,status:%s",vpStatusString[status]);

    }

    return status;
}


VpStatusType slic_sound_start(struct slic_ioctl* slic_ctl)
{
    VpStatusType status = VP_STATUS_SUCCESS;
    u8 tone = slic_ctl->tone;

    switch(tone)
    {
        case SLIC_RING:
        {
            status = slic_set_mode(slic_ctl->mode);
            if(VP_STATUS_SUCCESS != status)
            {
                return status;
            }

            slic_get_callerid(slic_ctl);

            if( MAX_ONE_TRANSIZE >= slic_priv->cid_length )
            {
                status = VpInitCid(&LineCtx[0],slic_priv->cid_length,&slic_priv->cid_buff[0]);
                slic_priv->pos = slic_priv->cid_length;
            }
            else
            {
                status = VpInitCid(&LineCtx[0],MAX_ONE_TRANSIZE,&slic_priv->cid_buff[0]);
                slic_priv->pos = MAX_ONE_TRANSIZE;

            }
            if(VP_STATUS_SUCCESS != status)
            {
                printk(KERN_ERR"vpinitcid failed,tone=%d,status:%s\n",tone,vpStatusString[status]);
                return status;
            }

            status = VpSetLineState(&LineCtx[0],VP_LINE_RINGING);
            break;
        }

        case SLIC_DIAL_TONE:
        {
            status = VpSetLineTone(&LineCtx[0],slic_profile[country].ToneProf.DailToneProf,\
                      NULL,NULL);
            break;
        }

        case SLIC_RINGBACK_TONE:
        {
            status = VpSetLineTone(&LineCtx[0],slic_profile[country].ToneProf.RBackToneProf,\
                      slic_profile[country].ToneProf.RBackCandProf,NULL);
            break;
        }

        case SLIC_BUSY_TONE:
        {
            status = VpSetLineTone(&LineCtx[0],slic_profile[country].ToneProf.BusyToneProf,\
                      slic_profile[country].ToneProf.BusyCandProf,NULL);
            break;
        }
        
        /* voicemail 放音 case */
        case SLIC_VM_SHUTTER_TONE: 
        {
            status = VpSetLineTone(&LineCtx[0],slic_profile[country].ToneProf.VoiceMailToneProf,\
                          slic_profile[country].ToneProf.VoiceMailToneCandProf,NULL);
        printk("SLIC_VM_SHUTTER_TONE status:%d\n", status);
            break;
        }

        case SLIC_CALL_WAITING_TONE:
        {
            status = VpSetLineTone(&LineCtx[0],slic_profile[country].ToneProf.CWaitToneProf,\
                      slic_profile[country].ToneProf.CWaitCandProf,NULL);
            break;
        }

        case SLIC_CONGESTION_TONE:
        {
            status = VpSetLineTone(&LineCtx[0],slic_profile[country].ToneProf.CongeToneProf,\
                      slic_profile[country].ToneProf.CongeToneCandProf,NULL);
            break;
        }
        case SLIC_ALARM_TONE:
        {
            status = VpSetLineTone(&LineCtx[0],slic_profile[country].ToneProf.alarmToneProf,\
                      slic_profile[country].ToneProf.alarmToneCandProf,NULL);
            break;
        }       
        case SLic_SVC_ENABLE_TONE:
        {
            status = VpSetLineTone(&LineCtx[0],slic_profile[country].ToneProf.SvcEnableToneProf,\
                      slic_profile[country].ToneProf.SvcEnableCandToneProf,NULL);
            break;
        }
        case SLic_SVC_DISABLE_TONE:
        {
            status = VpSetLineTone(&LineCtx[0],slic_profile[country].ToneProf.SvcDisableToneProf,\
                      slic_profile[country].ToneProf.SvcDisableCandToneProf,NULL);
            break;
        }

        case SLIC_FAULT_TONE:
        {
            status = VpSetLineTone(&LineCtx[0],slic_profile[country].ToneProf.faultToneProf,\
                      slic_profile[country].ToneProf.faultToneCandProf,NULL);
            break;
        }

        default:
            printk(KERN_ERR"Error: invalid tone type[%d]\n",tone);
            break;
    }

    if(VP_STATUS_SUCCESS != status)
    {
        printk(KERN_ERR"slic sound start failed,tone=%d,status:%s\n",tone,vpStatusString[status]);
    }
    else
    {
        printk(KERN_ERR"slic sound start playing,tone=%d,status:%s\n",tone,vpStatusString[status]);   
    }

    return status;    
}


VpStatusType slic_sound_stop(u8 tone)
{
    VpStatusType status = VP_STATUS_SUCCESS;   
    VpLineStateType currentState;     
    
    if (SLIC_RING == tone)
    {     
        if( VpGetLineState(&LineCtx[0], &currentState) != VP_STATUS_SUCCESS ) 
        {
            printk(KERN_ERR"Line State ERROR: %s %d", __FUNCTION__, __LINE__);
            return VP_STATUS_FAILURE;    
        } 
        if(VP_LINE_RINGING == currentState)
        {
            status = VpSetLineState(&LineCtx[0],VP_LINE_STANDBY); 
            if(VP_STATUS_SUCCESS != status)
            {
                printk(KERN_ERR"slic sound stop failed,tone:%d,status:%s\n", tone, vpStatusString[status]);
            }
        }
        /*如果slic本身就不在振铃状态，就直接返回默认值成功*/
        return status;
    }
    
    status = VpSetLineTone(&LineCtx[0],NULL,NULL,NULL);
    if(VP_STATUS_SUCCESS != status)
    {
        printk(KERN_ERR"slic sound stop failed,tone:%d,status:%s\n", tone, vpStatusString[status]);
    }    
    return status; 
}    


VpStatusType slic_cid_in_talk(struct slic_ioctl* slic_ctl)
{
    VpStatusType status = VP_STATUS_SUCCESS;

    slic_get_callerid(slic_ctl);

    if(MAX_ONE_TRANSIZE >= slic_priv->cid_length)
    {
        if( CID_FSK == slic_ctl->mode )
        {
            status = VpSendCid(&LineCtx[0],slic_priv->cid_length,slic_profile[country].CIDFsk2Prof, \
                      (u8*)&slic_priv->cid_buff[0]);
            slic_priv->pos = slic_priv->cid_length;
        }
    }
    else
    {
        if( CID_FSK == slic_ctl->mode )
        {
            status = VpSendCid(&LineCtx[0],MAX_ONE_TRANSIZE,slic_profile[country].CIDFsk2Prof, \
                      &slic_priv->cid_buff[0]);
            slic_priv->pos = MAX_ONE_TRANSIZE;
        }
    }

    if(VP_STATUS_SUCCESS != status)
    {
        printk(KERN_ERR"SLIC_CID_IN_TALK failed,status:%s",vpStatusString[status]);
    }
    return status;

}


static void slic_dtmfkey_report(uint32 code)
{
    input_report_key(slic_priv->input_dev, code, 1);
    input_sync(slic_priv->input_dev);
    input_report_key(slic_priv->input_dev, code, 0);
    input_sync(slic_priv->input_dev);
}

void slic_dtmf_digit_report(VpDigitType digit)
{
    slic_key_code keycode = SLIC_NONE_K;

    switch(digit)
    {
        case VP_DIG_NONE:
            keycode = SLIC_NONE_K;
            break;
        case VP_DIG_ZERO:
            keycode = SLIC_0_K;
            printk(KERN_ERR"report_0\n");
            break;
        case VP_DIG_1:
            keycode = SLIC_1_K;
            printk(KERN_ERR"report_1\n");
            break;
        case VP_DIG_2:
            keycode = SLIC_2_K;
            printk(KERN_ERR"report_2\n");
            break;
        case VP_DIG_3:
            keycode = SLIC_3_K;
            printk(KERN_ERR"report_3\n");
            break;
        case VP_DIG_4:
            keycode = SLIC_4_K;
            printk(KERN_ERR"report_4\n");
            break;
        case VP_DIG_5:
            keycode = SLIC_5_K;
            printk(KERN_ERR"report_5\n");
            break;
        case VP_DIG_6:
            keycode = SLIC_6_K;
            printk(KERN_ERR"report_6\n");
            break;
        case VP_DIG_7:
            keycode = SLIC_7_K;
            printk(KERN_ERR"report_7\n");
            break;
        case VP_DIG_8:
            keycode = SLIC_8_K;
            printk(KERN_ERR"report_8\n");
            break;
        case VP_DIG_9:
            keycode = SLIC_9_K;
            printk(KERN_ERR"report_9\n");
            break;
        case VP_DIG_ASTER:
            keycode = SLIC_STAR_K;
            printk(KERN_ERR"report_*\n");
            break;
        case VP_DIG_POUND:
            keycode = SLIC_POUND_K;
            printk(KERN_ERR"report_#\n");
            break;
        case VP_DIG_A:
            keycode = SLIC_A_K;
            printk(KERN_ERR"report_A\n");
            break;
        case VP_DIG_B:
            keycode = SLIC_B_K;
            printk(KERN_ERR"report_B\n");
            break;
        case VP_DIG_C:
            keycode = SLIC_C_K;
            printk(KERN_ERR"report_C\n");
            break;
        case VP_DIG_D:
            keycode = SLIC_D_K;
            printk(KERN_ERR"report_D\n");
            break;
        default:
            break;
    }
    if(SLIC_NONE_K != keycode)
    {
        slic_dtmfkey_report(keycode);
    }
}


VpStatusType slic_config_line(void)
{
    u32          linenum = 0;
    u32          rmin    = slic_customize_ex_parm[country].rkey_time.rkey_time_min;
    u32          rmax    = slic_customize_ex_parm[country].rkey_time.rkey_time_max;
    VpStatusType status  = VP_STATUS_SUCCESS;

    VpOptionTimeslotType     timeSlots[Line_Num];
    VpOptionPcmTxRxCntrlType PcmMode   = VP_OPTION_PCM_BOTH;
    VpOptionCodecType        CodeMode  = SND_SAMPLE_RATE_8K == snd_sample_rate_get() ? VP_OPTION_LINEAR : VP_OPTION_LINEAR_WIDEBAND;
    VpOptionPulseModeType    pulseMode = VP_OPTION_PULSE_DECODE_ON;
    VpOptionDtmfModeType     dtmfMode;
    VpOptionPulseType        pulseType;

#ifdef SLIC_LOOPBACK_DEBUG
    VpOptionLoopbackType     loop = VP_OPTION_LB_TIMESLOT;
#endif
    pulseType.breakMin      = 33 * 8;  /* 33 ms  */
    pulseType.breakMax      = 100 * 8; /* 100ms */
    pulseType.makeMin       = 17 * 8;  /* 17ms   */
    pulseType.makeMax       = 75 * 8;  /* 75ms   */
    pulseType.interDigitMin = 250 * 8; /* 250ms */

    /* 此处应该是rkey_time_min，但是必须满足
       breakMin<breakMax<flashMin,取它们的较大值*/
    pulseType.flashMin = (u16)((rmin > 101) ? rmin : 101) * 8;
    pulseType.flashMax = (u16)rmax * 8;

    dtmfMode.dtmfControlMode = VP_OPTION_DTMF_DECODE_OFF;
    dtmfMode.direction       = VP_DIRECTION_US;

    status = VpSetOption(VP_NULL, &DevCtx, VP_OPTION_ID_DTMF_MODE, &dtmfMode);
    if (status != VP_STATUS_SUCCESS)
    {
        printk(KERN_ERR"ERROR: unable to set dtmf mode %s\n", vpStatusString[status]);
        return status;
    }

    status =  VpSetOption(VP_NULL, &DevCtx, VP_OPTION_ID_PULSE_MODE, &pulseMode);
    if (status != VP_STATUS_SUCCESS)
    {
        printk(KERN_ERR"ERROR: unable to set option pulse mode %s\n", vpStatusString[status]);
        return status;
    }

    status = VpSetOption(VP_NULL, &DevCtx, VP_DEVICE_OPTION_ID_PULSE, &pulseType);
    if (status != VP_STATUS_SUCCESS)
    {
        printk(KERN_ERR"ERROR: unable to set time slots %s\n", vpStatusString[status]);
        return status;
    }


    status = VpSetOption(VP_NULL, &DevCtx, VP_OPTION_ID_CODEC, &CodeMode);
    if (status != VP_STATUS_SUCCESS)
    {
        printk(KERN_ERR"ERROR: unable to codec mode %s\n", vpStatusString[status]);
        return status;
    }

    status =  VpSetOption(VP_NULL, &DevCtx, VP_OPTION_ID_PCM_TXRX_CNTRL, &PcmMode);
    if (status != VP_STATUS_SUCCESS)
    {
        printk(KERN_ERR"ERROR: unable to set option pcm txrx ctrl %s\n", vpStatusString[status]);
        return status;
    }

    /*if(2 == Line_Num)
    {
        timeSlots[0].tx = 0;
        timeSlots[0].rx = 0;
        timeSlots[1].tx = 2;
        timeSlots[1].rx = 2;
    }
    else */
    
    if(1 == Line_Num)
    {
        timeSlots[0].tx = 0;
        timeSlots[0].rx = 0;
    }

    for(linenum = 0; linenum < Line_Num; linenum++)
    {

        status = VpSetOption(&LineCtx[linenum], VP_NULL, VP_OPTION_ID_TIMESLOT, &timeSlots[linenum]);
        if (status != VP_STATUS_SUCCESS)
        {
            printk(KERN_ERR"ERROR: unable to set time slots %s\n", vpStatusString[status]);
            return status;
        }
    }

#ifdef SLIC_LOOPBACK_DEBUG
    status = VpSetOption(&LineCtx[0], VP_NULL, VP_OPTION_ID_LOOPBACK, &loop);
    if (status != VP_STATUS_SUCCESS)
    {
        printk(KERN_ERR"ERROR: unable to set loopback %s\n", vpStatusString[status]);
        return status;
    }

    status = VpSetLineState(&LineCtx[0], VP_LINE_TALK);
    if(VP_STATUS_SUCCESS != status)
    {
        printk(KERN_ERR"[%s,line:%d]set line status failed:%s\n",  \
               __FUNCTION__,__LINE__,vpStatusString[status]);
    }
#endif
    
    printk(KERN_INFO"Line config success!\n");
    return status;

}


void slic_init_set_masks(void)
{
    VpOptionEventMaskType eventMask;
    VpStatusType status = VP_STATUS_SUCCESS;

    /* mask everything */
    memset(&eventMask, 0xFF, sizeof(VpOptionEventMaskType));

    /* unmask only the events the application is interested in */
    eventMask.faults    = VP_EVCAT_FAULT_UNMASK_ALL;
    eventMask.signaling = VP_EVCAT_SIGNALING_UNMASK_ALL;
    eventMask.response  = VP_EVCAT_RESPONSE_UNMASK_ALL;
    eventMask.test      = VP_EVCAT_TEST_MASK_ALL;
    eventMask.process   = VP_EVCAT_PROCESS_UNMASK_ALL;

    /* inform the API of the mask */
    status = VpSetOption(VP_NULL, &DevCtx, VP_OPTION_ID_EVENT_MASK, &eventMask);
    if (status != VP_STATUS_SUCCESS)
    {
        printk(KERN_ERR"ERROR: initsetmask fail: %s\n", vpStatusString[status]);
    }
    return;
}

void fault_event_handle(VpLineCtxType *pLineCtx,VpEventType *pEvent)
{
    u16 eventid   = 0;
    u16 eventdata = 0;
    u32 linenum   = 0;
    VpStatusType status = VP_STATUS_SUCCESS;

    eventid = pEvent->eventId;
    switch(eventid)
    {
        case VP_DEV_EVID_BAT_FLT:
        case VP_DEV_EVID_CLK_FLT:
        {
             eventdata = (pEvent->eventData & 0x01);
             if( 0x01 == eventdata )
             {
                printk(KERN_ERR"Error (%s) fault detected\n",(eventid == VP_DEV_EVID_BAT_FLT ? "bat":"clk"));

             }
             else
             {
#ifdef CONFIG_SLIC_AMR_WB
                /*宽窄带切换时，会有一次时钟错误，待恢复时，重新初始化dtmf检测*/
                VpOptionDtmfModeType     dtmfmode;
                dtmfmode.dtmfControlMode = VP_OPTION_DTMF_DECODE_OFF;
                status = VpSetOption(VP_NULL, &DevCtx, VP_OPTION_ID_DTMF_MODE, &dtmfmode);
                if (VP_STATUS_SUCCESS != status)
                {
                    printk(KERN_ERR"ERROR: unable to set DTMF_DECODE_OFF %s\n", vpStatusString[status]);
                    return status;
                }

                dtmfmode.dtmfControlMode = VP_OPTION_DTMF_DECODE_ON;
                dtmfmode.direction = VP_DIRECTION_US;
                status = VpSetOption(&LineCtx[0], NULL, VP_OPTION_ID_DTMF_MODE, &dtmfmode);
                if (status != VP_STATUS_SUCCESS)
                {
                    printk(KERN_ERR"ERROR: unable to set DTMF_DECODE_ON :%s\n", vpStatusString[status]);
                    return status;
                }
#endif
                printk(KERN_ERR"INFO (%s) fault recovered\n",(eventid == VP_DEV_EVID_BAT_FLT ? "bat":"clk"));

             }
             break;
        }
        case VP_LINE_EVID_DC_FLT:
        case VP_LINE_EVID_AC_FLT:
        case VP_LINE_EVID_THERM_FLT:
         {
            eventdata = (pEvent->eventData & 0x01);
            if(0x01 == eventdata)
            {
                printk(KERN_ERR"Error line critical fault happen,eventid=%d,channel=%d\n",pEvent->eventId,pEvent->channelId);
            }
            else
            {
                printk(KERN_ERR"Error line critical fault recover,eventid=%d,channel=%d\n",pEvent->eventId,pEvent->channelId);
                status = VpSetLineState(pLineCtx, VP_LINE_STANDBY);
                if( VP_STATUS_SUCCESS != status )
                {
                    printk(KERN_ERR"Error set linestate on linenum %d: %s\n",linenum,vpStatusString[status]);
                }
            }
            break;
         }
        case VP_DEV_EVID_EVQ_OFL_FLT:
        {
            printk(KERN_ERR"Error FIFO overflow\n");
            break;
        }
        case VP_LINE_EVID_GND_FLT:
        {
            printk(KERN_ERR"Error Ground Key Fault\n");
            break;
        }
        default:
            break;

    }
}

void response_event_handle(VpLineCtxType *pLineCtx,VpEventType *pEvent)
{
    u32  linenum = 0;
    VpStatusType status = VP_STATUS_SUCCESS;
    u16 eventid = pEvent->eventId;

    switch(eventid)
    {
        case VP_DEV_EVID_DEV_INIT_CMP:
         {
            printk(KERN_INFO"DEV_INIT_CMP ok!\n");

            /*step4--unmask events needed*/
            slic_init_set_masks();

            /*step5--set lines to standby and calibrate them*/
            for(linenum = 0;linenum < Line_Num;linenum++)
            {
                status = VpSetLineState(&LineCtx[linenum], VP_LINE_STANDBY);
                if( VP_STATUS_SUCCESS != status )
                {
                    printk(KERN_ERR"[%s]set the linenum(%d) state failed!state=%s\n", \
                           __FUNCTION__,linenum,vpStatusString[status]);
                    return;
                }

                /*calibrate it and wait CAL_CMP event*/
                status = VpCalLine(&LineCtx[linenum]);
                if( VP_STATUS_SUCCESS != status )
                {
                    printk(KERN_ERR"[%s]cal the linenum(%d) failed,state=%s\n",  \
                           __FUNCTION__,linenum,vpStatusString[status]);
                    return;
                }
            }
            break;
         }
        case VP_EVID_CAL_CMP:
         {
            if((Line_Num -1) == pEvent->channelId)
            {
                status = slic_config_line();
                if(VP_STATUS_SUCCESS != status)
                {
                    printk(KERN_ERR"[%s]set the linenum(%d) state failed!state=%s\n",__FUNCTION__,linenum,vpStatusString[status]);
                    return;
                }
            }
            break;
         }

        default:
             break;
    }
}

void signal_event_handle(VpLineCtxType *pLineCtx,VpEventType *pEvent)
{
    u16 eventid   = 0;
    u16 eventdata = 0;
    signed long  dBm   = 0; 
    VpDigitType  digit = 0;
    VpStatusType status = VP_STATUS_SUCCESS;
    VpOptionDtmfModeType dtmfMode;

    static struct timeval dtmf_sense_make_time = {0};
    struct timeval dtmf_sense_break_time = {0};
    time_t       dtmf_delta_sec = 0;
    suseconds_t  dtmf_delta_usec = 0;

    eventid = pEvent->eventId;
    switch(eventid)
    {
        case VP_LINE_EVID_HOOK_OFF:   /*摘机事件*/
        {
            slic_priv->dtmf_enable = 1;
            
            dtmfMode.dtmfControlMode = VP_OPTION_DTMF_DECODE_ON;
            dtmfMode.direction = VP_DIRECTION_US;

            status = VpSetOption(pLineCtx, NULL, VP_OPTION_ID_DTMF_MODE, &dtmfMode);
            if (status != VP_STATUS_SUCCESS)
            {
                printk(KERN_ERR"ERROR: unable to set DTMF_DECODE_ON :%s\n", vpStatusString[status]);
            }

            status = VpSetLineState(pLineCtx, VP_LINE_TALK);
            if(VP_STATUS_SUCCESS != status)
            {
                printk(KERN_ERR"[%s,line:%d]set line status failed:%s\n",  \
                       __FUNCTION__,__LINE__,vpStatusString[status]);
            }

            slic_dtmfkey_report(SLIC_HANGUP_K);
            printk(KERN_ERR"report_hook off.\n");
            break;
        }
        case VP_LINE_EVID_HOOK_ON:    /*挂机事件*/
        {
            slic_priv->dtmf_enable = 0;
            
            status = VpSetLineState(pLineCtx, VP_LINE_STANDBY);
            if(VP_STATUS_SUCCESS != status)
            {
                printk(KERN_ERR"[%s,line:%d]ERROR:set line status failed:%s\n",__FUNCTION__, \
                       __LINE__,vpStatusString[status]);
            }

            dtmfMode.dtmfControlMode = VP_OPTION_DTMF_DECODE_OFF;
            
            status = VpSetOption(VP_NULL, &DevCtx, VP_OPTION_ID_DTMF_MODE, &dtmfMode);
            if (status != VP_STATUS_SUCCESS)
            {
                printk(KERN_ERR"ERROR: unable to set DTMF_DECODE_OFF %s\n", vpStatusString[status]);
            }

            slic_dtmfkey_report(SLIC_HANGON_K);
            printk(KERN_ERR"report_hook on.\n");
            break;
        }
        case VP_LINE_EVID_GKEY_DET:
        {
            printk(KERN_ERR"INFO:Ground Key detected,line[%d]\n",pEvent->lineId);
            break;
        }
        case VP_LINE_EVID_GKEY_REL:
        {
            status = VpSetLineState(pLineCtx, VP_LINE_STANDBY);
            if(VP_STATUS_SUCCESS != status)
            {
                printk(KERN_ERR"[%s,line:%d]ERROR:set line status failed:%s\n",__FUNCTION__, \
                       __LINE__,vpStatusString[status]);
            }
            printk(KERN_ERR"Info:Ground Key Released\n");
            break;
        }
        case VP_LINE_EVID_FLASH:  /*R_key 事件*/
        {
            slic_dtmfkey_report(SLIC_RKEY_K);
            printk(KERN_ERR"report_rkey\n");
            break;
        }
        case VP_LINE_EVID_DTMF_DIG:    /*DTMF按键事件*/
        {
            eventdata = pEvent->eventData & 0x0010;
            if( VP_DIG_SENSE_MAKE == eventdata )
            {                                 
                /*计算一下DTMF阈值 */
                dBm = GET_dBm_VALUE(pEvent->eventData);                
                if(dBm < KEY_VAULE)
                {
                    no_send_flag = 1;
                    break;
                }
                /*只有在HifiEnable时才通知hifi，马上会产生一个dtmf音，需要过滤 */
                if(ulHifiEnable)
                {
                    VpOptionPcmTxRxCntrlType PcmMode_RX_ONLY = VP_OPTION_PCM_RX_ONLY;
                    VpSetOption(pLineCtx, VP_NULL, VP_OPTION_ID_PCM_TXRX_CNTRL, &PcmMode_RX_ONLY);
                    send_mute_dtmf_to_med();
                }
                
                do_gettimeofday(&dtmf_sense_make_time);
            }
            else if( VP_DIG_SENSE_BREAK == eventdata )
            {                                      
                /*判断一下是否需要过滤掉 */
                if(1 == no_send_flag)
                {                       
                   no_send_flag = 0;
                   break;
                }
                /*把通道打开 */
                if(ulHifiEnable)
                {
                    VpOptionPcmTxRxCntrlType PcmMode_BOTH = VP_OPTION_PCM_BOTH;
                    VpSetOption(pLineCtx, VP_NULL, VP_OPTION_ID_PCM_TXRX_CNTRL, &PcmMode_BOTH);
                }

                do_gettimeofday(&dtmf_sense_break_time);

                dtmf_delta_sec = dtmf_sense_break_time.tv_sec - dtmf_sense_make_time.tv_sec;
                /* drop the dtmf if ontime is less than 60ms */
                if (1 == dtmf_delta_sec || 0 == dtmf_delta_sec)
                {
                    dtmf_delta_usec = dtmf_delta_sec*ONE_SECOND + dtmf_sense_break_time.tv_usec - dtmf_sense_make_time.tv_usec;

                    /* drop the dtmf if ontime is less than 60ms */
                    if (dtmf_delta_usec < DTMF_ONTIME_THRESHOLD)
                    {
                        printk(KERN_ERR"dtmf on time : %dus, not long enough,drop it!\n",dtmf_delta_usec);
                        break;
                    }
                }

                digit = (VpDigitType)(pEvent->eventData & 0x000F);
                slic_dtmf_digit_report(digit);
               
            }
            break;
        }
        default:
             break;
    }
}

void process_event_handle(VpLineCtxType *pLineCtx,VpEventType *pEvent)
{
    u16  eventid       = 0;
    u16  eventdata     = 0;
    u32  cidleftlength = 0;
    VpStatusType status = VP_STATUS_SUCCESS;


    eventid = pEvent->eventId;
    switch(eventid)
    {
        case VP_LINE_EVID_CID_DATA:
        {
            eventdata = pEvent->eventData;
            cidleftlength = slic_priv->cid_length - slic_priv->pos;

            if( VP_CID_DATA_NEED_MORE_DATA ==  eventdata )
            {
                if( 16 <= cidleftlength )
                {
                    status = VpContinueCid(pLineCtx,16,&slic_priv->cid_buff[slic_priv->pos]);
                    slic_priv->pos += 16;
                }
                else if( 0 < cidleftlength )
                {
                    status = VpContinueCid(pLineCtx,(u8)cidleftlength,&slic_priv->cid_buff[slic_priv->pos]);
                    slic_priv->pos = slic_priv->cid_length;
                }

                if (status != VP_STATUS_SUCCESS)
                {
                    printk(KERN_ERR"ERROR: NEED_MORE_DATA failed,status: %s\n", vpStatusString[status]);
                }
            }
            break;
        }
        default:
            break;
    }
}

int slic_event_handler(void* data)
{
    VpLineCtxType *pLineCtx = NULL;
    VpEventCategoryType eventCategory;
    VpEventType   Event;

    while(1)
    {
        osl_sem_down(&slic_priv->event_sem);
        
        /*如果是摘机状态，使能搬移dtmf数据*/
        if(1 == slic_priv->dtmf_enable)
        {
            (void)Vp886DtmfReadBuffer(&LineCtx[0]);
        }

        memset(&Event, 0, sizeof(VpEventType));
        if( VpGetEvent(&DevCtx, &Event) )
        {
            pLineCtx = Event.pLineCtx;
            eventCategory = Event.eventCategory;
            switch(eventCategory)
            {
                case VP_EVCAT_FAULT:
                    fault_event_handle(pLineCtx,&Event);
                    break;

                case VP_EVCAT_RESPONSE:
                    response_event_handle(pLineCtx,&Event);
                    break;

                case VP_EVCAT_SIGNALING:
                    signal_event_handle(pLineCtx,&Event);
                    break;

                case VP_EVCAT_PROCESS:
                    process_event_handle(pLineCtx,&Event);
                    break;

                default:
                     break;
            }
        }
    }
    return 0;
}


s32 slic_cmd_ctrl(u32 cmd, struct slic_ioctl *slic_ctl)
{
    s32  err = 0;

    if( SLIC_CTRL_COUNT <= cmd )
    {
        printk(KERN_ERR"[%d]invalid cmd\n",cmd);
        return -EINVAL;
    }

    switch( cmd )
    {
        case SLIC_SOUND_START:
            {
                err = slic_sound_start(slic_ctl);
                break;
            }

        case SLIC_SOUND_STOP:
            {
                err = slic_sound_stop(slic_ctl->tone);
                break;
            }

        case SLIC_CID_IN_TALK:
            {
                err = slic_cid_in_talk(slic_ctl);
                break;
            }
                                          
        case SLIC_DTMF_INOUT_BAND:
            {
                /*控制dtmf走带内带外，0为带内，1为带外；当走带外时，需要擦除带内音，
                  所以需要给hifi发送消息擦除带内音；voip时不涉及，由hellosoft处理*/
                ulHifiEnable = (slic_ctl->inout_band == 1);
                printk(KERN_ERR"voip status change detected by SLIC. status=%d\r\n", ulHifiEnable);
                break;
            }

        /*voip电话前设置下宽窄带*/
        case SLIC_VOIP_BAND_SWITCH:
            {
#ifdef CONFIG_SLIC_AMR_WB
                printk(KERN_ERR"voip set band : %d\r\n", slic_ctl->band_switch);
                err = slic_band_switch(slic_ctl->band_switch);
#endif
                break;
            }

        default:
            break;

    }

    return err;
}

int slic_chip_init(void)
{
    Vp886DeviceObjectType *vp886Dev  = NULL;
    Vp886LineObjectType   *vp886Line = NULL;
    VpDeviceIdType deviceId   = ZSI_4WIRE;
    VpLineIdType   linenum    = 0;
    VpStatusType   status     = VP_STATUS_SUCCESS;
    VpDeviceType   deviceType = VP_DEV_887_SERIES;

    /*  
    if(9662 == slic_ver_param)
    {
        slic_profile = slic9662_profile;
        deviceType = VP_DEV_886_SERIES;
    }
    else
    {
        slic_profile = slic9641_profile;
        deviceType = VP_DEV_887_SERIES;
        
    }
    */

    vp886Dev = (Vp886DeviceObjectType *)kzalloc(sizeof(Vp886DeviceObjectType), GFP_KERNEL);
    if(NULL == vp886Dev)
    {
        printk(KERN_ERR "slic malloc vp886Dev failed.\n");
        return -1;
    }

    vp886Line = (Vp886LineObjectType *)kzalloc(sizeof(Vp886LineObjectType) * Line_Num, GFP_KERNEL);
    if(NULL == vp886Line)
    {
        printk(KERN_ERR "slic malloc vp886Line failed.\n");
        kfree(vp886Dev);
        return -1;
    }
    
    /*step1--make device object*/
    status = VpMakeDeviceObject(deviceType, deviceId, &DevCtx, vp886Dev);

    if( VP_STATUS_SUCCESS != status )
    {
        printk(KERN_ERR"Error making the device object: %s\n", vpStatusString[status]);
        goto out;
    }

    /*step2--make line object for each line*/
    for( linenum = 0;linenum < Line_Num;linenum++ )
    {
        status = VpMakeLineObject(VP_TERM_FXS_LOW_PWR, linenum, &LineCtx[linenum], \
                                  &vp886Line[linenum], &DevCtx);
        if( VP_STATUS_SUCCESS != status )
        {
            printk(KERN_ERR"Error calling VpMakeLineObject on linenum %d: %s\n",linenum, \
                   vpStatusString[status]);
            goto out;
        }
    }

    /*step3--init device and wait INIT_CMP event，这里统一用宽带的AC profile*/
    status = VpInitDevice(&DevCtx, ZLR96411L_ABS81V_DEVICE, slic_profile[country].ACWbProf, \
                           slic_profile[country].DCProf, slic_profile[country].ToneProf.RingProf, \
                           VP_PTABLE_NULL, VP_PTABLE_NULL);


    if( VP_STATUS_SUCCESS != status )
    {
        printk(KERN_ERR"Device not properly initialized: %s\n", vpStatusString[status]);
        goto out;
    }
    return 0;
out:
    kfree(vp886Dev);
    kfree(vp886Line);
    vp886Dev = NULL;
    vp886Line = NULL;
    return -1;
}

void slic_softtimer_handle(u32 para)
{
    
    /*释放信号量，让内核线程能往下走*/
    osl_sem_up(&slic_priv->event_sem);

    /*再次使能时钟中断*/
    bsp_softtimer_add(&slic_softtimer);
    return;
}

/*启动一个高优先级的任务来处理slic事件，任务的调度通过一个timer硬中断来触发*/
int slic_irq_and_task_init(void)
{
    struct softtimer_list *softtimer = &slic_softtimer;
    struct task_struct * pid = NULL;
    struct sched_param   param = {
        .sched_priority = SLIC_TASK_PRIO,
    };

    osl_sem_init(0,&slic_priv->event_sem);

    pid = (struct task_struct *)kthread_run(slic_event_handler, 0, "slic_event");
    if (NULL == pid)
    {
        printk(KERN_ERR"slic_irq_and_task_init create kthread failed!\n");
        return -1;
    }
    
    if (0 != sched_setscheduler(pid, SCHED_FIFO, &param))
    {
        printk(KERN_ERR"slic sched setscheduler failed!\n");
        return -1;
    }

    /*申请一个timer中断*/
    softtimer->func = slic_softtimer_handle;
    softtimer->para = 0;
    softtimer->timeout = SLIC_FAST_TIME;
    softtimer->wake_type = SOFTTIMER_WAKE;
    softtimer->emergency = TIMER_EMERGENCY_FLAG;

    if(0 != bsp_softtimer_create(softtimer))
    {
        printk(KERN_ERR"slic_irq_and_task_init:softtimer failed!\n");
        return -1;
    }

    bsp_softtimer_add(softtimer);

    return 0;
}

int slic_probe(struct snd_soc_codec *codec)
{
    s32 err = 0;
    u32 i   = 0;

    struct input_dev * input_dev = NULL;
  
    /*获取国家码，从insmod参数传入*/
    if (0 > country || country >= REGION_COUNT
        || !slic_profile[country].DCProf)
    {
        country = REGION_China;
    }

    printk(KERN_ERR"country = %d\n", country);

    if(NULL == codec)
    {
        printk(KERN_ERR"slic platform_device is null!\n");
        err = -EINVAL;
        goto  exit_soc_codec_null;
    }

    slic_priv = (struct slic_private*)kzalloc(sizeof(struct slic_private), GFP_KERNEL);
    if (!slic_priv)
    {
        printk(KERN_ERR"failed to allocate memory for slic_priv\n");
        err = -ENOMEM;
        goto exit_alloc_slic_private_failed;
    }

    slic_priv->slic_ctl = (struct slic_ioctl*)kzalloc(sizeof(struct slic_ioctl), GFP_KERNEL);
    if (!slic_priv->slic_ctl)
    {
        printk(KERN_ERR"failed to allocate memory for slic_ctl\n");
        err = -ENOMEM;
        goto exit_alloc_slic_ctl_failed;
    }

    input_dev = input_allocate_device();
    if (!input_dev)
    {
        printk(KERN_ERR"input device allocate failed!\n");
        err = -ENOMEM;
        goto err_alloc_input_device;
    }

    input_dev->name = codec->name;
    input_dev->id.bustype = BUS_HOST;
    input_dev->dev.parent = codec->dev;
    input_dev->keycode = slic_priv->keycodes;
    input_dev->keycodesize = sizeof(slic_priv->keycodes[0]);
    input_dev->keycodemax = ARRAY_SIZE(slic_keycodes);
    set_bit(EV_KEY, input_dev->evbit);
    for( i = 0; i < ARRAY_SIZE(slic_keycodes); i++ )
    {
        slic_priv->keycodes[i] = slic_keycodes[i];
        set_bit(slic_keycodes[i], input_dev->keybit);
    }

    slic_priv->input_dev = input_dev;

    err = input_register_device(slic_priv->input_dev);
    if (err) {
        printk(KERN_ERR"Failed to register input device!\n");
        goto err_register_input_device;
    }

    osl_sem_init(1,&slic_priv->ioctl_sem);
    osl_sem_init(1,&slic_priv->code_sem);
    osl_sem_init(1,&slic_priv->mpi_sem);

    /*slic芯片初始化*/
    if(0 != slic_chip_init())
    {
        printk(KERN_ERR"slic_chip_init failed!\n");
        goto err_chip_init;
    }

    err = slic_irq_and_task_init();
    if (err)
    {
        goto err_register_input_device;
    }

#if (FEATURE_ON == MBB_SLIC)
    AT_RegisterSlicATSetAudioEnable((void*)SlicATSetAudioEnable);
    AT_RegisterSlicATSelfTest((void*)is_slic_init_ok);
#ifdef CONFIG_SLIC_AMR_WB
    AT_RegisterSlicChanneSet((void*)slic_band_switch);
#endif
#endif
    printk(KERN_ERR"slic codec probe ok!\n");
    is_init_ok = 1;
    return 0;

err_chip_init:
err_register_input_device:
    input_unregister_device(slic_priv->input_dev);
    input_free_device(slic_priv->input_dev);
    slic_priv->input_dev = NULL;
err_alloc_input_device:
    kfree(slic_priv->slic_ctl);
    slic_priv->slic_ctl = NULL;
exit_alloc_slic_ctl_failed:
    kfree(slic_priv);
    slic_priv = NULL;
exit_alloc_slic_private_failed:
exit_soc_codec_null:
    is_init_ok = 0;
    return err;
}

int  slic_remove(void)
{

    if (0 == bsp_softtimer_delete(&slic_softtimer))
    {
        (void)bsp_softtimer_free(&slic_softtimer);
    }

    input_unregister_device(slic_priv->input_dev);
    input_free_device(slic_priv->input_dev);
    slic_priv->input_dev = NULL;

    kfree(slic_priv->slic_ctl);
    slic_priv->slic_ctl = NULL;

    kfree(slic_priv);
    slic_priv = NULL;
    return 0;
}

void slic_reg_dump(void)
{
    VpRegisterDump(&DevCtx);
}

void slic_sound_start_test(u8 tone,u8 mode,u8 secret)
{
    u8 date_test_buf[]= {'0','6','2','2','1','4','3','2', '\0'};
    u8 num_test_buf[] = {'1','3','8','1','2','3','4','5','6','7','8','\0'};

    struct slic_ioctl* slic_ctl = slic_priv->slic_ctl;

    memset(slic_ctl,0x0,sizeof(struct slic_ioctl));
    memcpy(slic_ctl->date_time,date_test_buf,sizeof(date_test_buf));
    memcpy(slic_ctl->cid_num,num_test_buf,sizeof(num_test_buf));

    slic_ctl->tone   = tone;
    slic_ctl->mode   = mode;
    slic_ctl->secret = secret;

    slic_sound_start(slic_ctl);
}

/*宽窄带切换特性*/
#ifdef CONFIG_SLIC_AMR_WB
int slic_band_switch(int type)
{
    int clkrate = 0;
    VpStatusType status = VP_STATUS_SUCCESS;
    VpOptionPcmTxRxCntrlType pcmmode    = 0;
    VpOptionFsyncRateType    ratetype   = 0;
    VpOptionCodecType        codecmode  = 0;
    VpOptionDtmfModeType     dtmfmode = {0};

    clkrate = (BAND_SWITCH_TO_WIDE == type ? \
               SND_SAMPLE_RATE_16K : SND_SAMPLE_RATE_8K);
    printk(KERN_ERR "slic band switch to %s.\n",
                    type == BAND_SWITCH_TO_WIDE ? "wideband" : "norrowband");

    if(clkrate == snd_sample_rate_get())
    {
        printk(KERN_ERR "slic band to set is equal to now[%d],return\n",clkrate);
        return 0;
    }

    /*先关闭pcm通道*/
    pcmmode = VP_OPTION_PCM_OFF;
    status = VpSetOption(&LineCtx[0], VP_NULL, VP_OPTION_ID_PCM_TXRX_CNTRL, &pcmmode);
    if(VP_STATUS_SUCCESS != status)
    {
        printk(KERN_ERR "close pcm channel failed.\n");
        return status;
    }
   
    if(SND_SAMPLE_RATE_8K == clkrate)
    {
        ratetype  = VP_FSYNC_RATE_8KHZ;
        codecmode = VP_OPTION_LINEAR;
    }
    else if (SND_SAMPLE_RATE_16K == clkrate)
    {
        ratetype  = VP_FSYNC_RATE_16KHZ;
        codecmode = VP_OPTION_LINEAR_WIDEBAND;
    }

    /*时钟的切换本应在hifi里做，但是voip只需要窄带，所以这里也设置下时钟频率；*/
    sio_pcm_div(FS_CLK_8M, (clkrate == SND_SAMPLE_RATE_8K ? SYNC_CLK_8K : SYNC_CLK_16K));
    snd_sample_rate_set(clkrate);

    /*在进行切换时，会产生一个clk error事件，但是，
     *在下面这些步骤后，会产生clk recovery事件，slic恢复正常。*/
    /*设置时钟频率*/
    status = VpSetOption(VP_NULL, &DevCtx, VP_DEVICE_OPTION_ID_FSYNC_RATE, &ratetype);
    if(VP_STATUS_SUCCESS != status)
    {
        printk(KERN_ERR "set VP_DEVICE_OPTION_ID_FSYNC_RATE failed.\n");
        return status;
    }   
    
    /*设置语音编码配置*/
    status = VpSetOption(VP_NULL, &DevCtx, VP_OPTION_ID_CODEC, &codecmode);
    if (status != VP_STATUS_SUCCESS)
    {
        printk(KERN_ERR"set VP_OPTION_ID_CODEC failed:%s\n", vpStatusString[status]);
        return status;
    }

    /*重新打开关闭dtmf检测*/
    dtmfmode.dtmfControlMode = VP_OPTION_DTMF_DECODE_OFF;
    status = VpSetOption(VP_NULL, &DevCtx, VP_OPTION_ID_DTMF_MODE, &dtmfmode);
    if (VP_STATUS_SUCCESS != status)
    {
        printk(KERN_ERR"ERROR: unable to set DTMF_DECODE_OFF %s\n", vpStatusString[status]);
        return status;
    }

    dtmfmode.dtmfControlMode = VP_OPTION_DTMF_DECODE_ON;
    dtmfmode.direction = VP_DIRECTION_US;
    status = VpSetOption(&LineCtx[0], NULL, VP_OPTION_ID_DTMF_MODE, &dtmfmode);
    if (status != VP_STATUS_SUCCESS)
    {
        printk(KERN_ERR"ERROR: unable to set DTMF_DECODE_ON :%s\n", vpStatusString[status]);
        return status;
    }
    
    /*打开pcm通道*/
    pcmmode = VP_OPTION_PCM_BOTH;
    status = VpSetOption(&LineCtx[0], VP_NULL, VP_OPTION_ID_PCM_TXRX_CNTRL, &pcmmode);
    if(VP_STATUS_SUCCESS != status)
    {
        printk(KERN_ERR "open pcm channel failed.\n");
        return status;
    }

    return 0;
}
#endif

#if (FEATURE_ON == MBB_SLIC)
/*以下用于产品线产线测试使用，考虑只是烧片的时候才编译*/
/*****************************************************************************
 Prototype    : slic_run_cmd
 Description  : 产线测试使用，用来执行业务态回环进程
 Input        : 要执行的进程名
 Output       : None
 Return Value :
 Calls        :
*****************************************************************************/
int slic_run_cmd(const char *pcmd)
{
    int  ret = 0;
    char *envp[] = {"HOME=/", "PATH=/app/bin:/system/bin:/sbin:/bin:/usr/sbin:/usr/bin", NULL};
    char **argv = NULL;

    if(NULL == pcmd)
    {
        return -1;
    }

    argv = argv_split(0, pcmd, &ret); 
    if(NULL == argv)
    {
        printk("ERROR:argv is null.\n");
        return -1;
    }
    
    if(ret <= 0)
    {
        argv_free(argv);
        printk("ERROR:params=%d.", ret);
        return -1;
    }
    
    ret = call_usermodehelper(argv[0], argv, envp, (int)UMH_WAIT_PROC);
    argv_free(argv);
    printk("ret = %d, run cmd:%s\n", (int)ret,  pcmd);

    return ret;
}

/*****************************************************************************
 Prototype    : slic_loopback_thread
 Description  : 单独回环起一个线程
 Input        : enable
 Output       : None
 Return Value :
 Calls        :
*****************************************************************************/

int slic_loopback_thread(void* data)
{
    return slic_run_cmd("/app/bin/slic_pcm_loopback");
}

/*****************************************************************************
 Prototype    : SlicATSetAudioEnable
 Description  : 装备执行这个函数启动回环
 Input        : enable
 Output       : None
 Return Value :
 Calls        :
*****************************************************************************/
static int loopback_flag = 0;
static int ring_flag     = 0;

int SlicATSetAudioEnable(unsigned int channel, unsigned int type, unsigned int onoff)
{    
    /*目前只使用一个通道*/
    if (0 != channel)
    {
        printk(KERN_ERR"channel is not right,channel = %d!\n",channel);
        return -1;
    }

    switch(type)
    {
        case TYPE_RING_CMD:
        {
            if(ON_CMD == onoff)
            {
                /*不处于振铃状态*/
                if(0 == ring_flag)
                {
                    slic_sound_start_test(SLIC_RING,CID_FSK,0);
                    ring_flag = 1;
                } 
                return 0;
            }
            else if(OFF_CMD == onoff)
            {
               VpSetLineState(&LineCtx[0],VP_LINE_STANDBY); 
               ring_flag = 0;
            }
            else
            {
                printk(KERN_ERR"onoff is not right,onoff = %d!\n",onoff);
                return -1;
            }
            return 0;
        }
        case TYPE_LOOPBACK_CMD:
        {
            if(0 == loopback_flag)
            {
                kthread_run(slic_loopback_thread, NULL, "SLIC_LOOPBACK");
                loopback_flag = 1;
            }
            return 0;
        }
        default:
        {
            printk(KERN_ERR"type is not right,type = %d!\n",type);
            return -1;
        }
    }
}

int is_slic_init_ok(void)
{
    return is_init_ok;
}
#endif

/*lint -restore*/



