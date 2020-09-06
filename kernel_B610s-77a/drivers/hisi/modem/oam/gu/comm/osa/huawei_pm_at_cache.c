


#include "mbb_config.h"
#if (FEATURE_ON == MBB_MODULE_PM)

#define THIS_FILE_ID (1234)
#include "vos_Id.h"
#include "v_iddef.h"
#include "v_typdef.h"
#include "v_msg.h"

#include "PsTypeDef.h"  
//for set_at_id_to_index
#include "TafAppMma.h"
#include "MnCallApi.h"
#include "MnCallApi.h" 
#include "MnMsgApi.h" 
#include "siappstk.h" 
#include "TafApsApi.h" 
#include "AtOamInterface.h"
#include "AtMnInterface.h"                     
#include "SysNvId.h"
#include "OmApi.h"
#include "bsp_icc.h"
#include "bsp_sram.h"
#include "mdrv_nvim_comm.h"
#include "MnClient.h"
#include "product_nv_def.h"
#include "product_nv_id.h"
#include "huawei_pm_at_cache.h"
#include "ApsL4aInterface.h"
#include "gen_msg.h"
#include "TafAppMma.h"
#include "MnCallApi.h"
#include "MnMsgApi.h"
#include <osl_spinlock.h>


/*phone的消息id和pm msg_id列表结构体*/
typedef struct pm_phone_node_
{
    TAF_PHONE_EVENT taf_id;         
    huawei_pm_rsp_id_type  rsp_id;  
}pm_phone_node;

/*cs call的消息id和pm msg_id列表结构体*/
typedef struct cs_call_node_
{
    MN_CALL_EVENT_ENUM_U32 taf_id;         
    huawei_pm_rsp_id_type  rsp_id;  
}cs_call_node;

/*mn msg的消息id和pm msg_id列表结构体*/
typedef struct mn_msg_node_
{
    MN_MSG_EVENT_ENUM_U32 taf_id;         
    huawei_pm_rsp_id_type  rsp_id;  
}mn_msg_node;

/*L4A msg的消息id和pm msg_id列表结构体*/
typedef struct l4a_msg_node_
{
    APS_L4A_MSG_ID_ENUM_UINT32  taf_id;         
    huawei_pm_rsp_id_type      rsp_id;  
}l4a_msg_node;


unsigned int  g_AtCacDeqTaskId = 0;
/*电源管理C核LOG打印开关，默认关闭*/
unsigned int  g_PmCcorePrintOnOff = PM_MSG_PRINT_OFF;

/*The buffer for store responses*/
/*It's only accessed in this file,
other modules can't visit it directly*/
huawei_pm_rsp_proc_type huawei_pm_rsp_buf[HUAWEI_PM_RSP_MAX_CACHE_SIZE];

static pm_phone_node pm_phone_table[] = 
{
    { TAF_PH_EVT_MODE_CHANGE_IND,     HUAWEI_PM_RSP_ID_UR_MODE },    /* ^MODE [28] */
    { TAF_PH_EVT_RSSI_CHANGE_IND,     HUAWEI_PM_RSP_ID_UR_RSSI },    /* ^RSSI [16] */
    { TAF_PH_EVT_SERVICE_STATUS_IND,  HUAWEI_PM_RSP_ID_UR_SRVST },   /* ^SRVST[9] */
    { TAF_PH_EVT_SYSTEM_INFO_IND,     HUAWEI_PM_RSP_ID_UR_REG },     /* +CREG/+CGREG [19] */
    { TAF_PH_EVT_USIM_INFO_IND,       HUAWEI_PM_RSP_ID_UR_SIMST },   /* ^SIMST [22] */
    { TAF_PH_EVT_MM_INFO_IND,         HUAWEI_PM_RSP_ID_UR_CTZV },    /* +CTZV [27] */
    { TAF_PH_EVT_PLMN_LIST_CNF,       HUAWEI_PM_RSP_ID_RESULT_CODE },
#if (FEATURE_ON == MBB_FLIGHT_MODE)	
    { TAF_PH_EVT_RFSWITCH_CNF,        HUAWEI_PM_RSP_ID_UR_RFSWITCH},  /*^RFSWTICH*/
#endif
};

static cs_call_node pm_cs_call_table[] = 
{
    { MN_CALL_EVT_ORIG,                 HUAWEI_PM_RSP_ID_UR_ORIG },     /* ^ORIG [0] */
    { MN_CALL_EVT_CALL_PROC,            HUAWEI_PM_RSP_ID_UR_CONF},      /* ^CONF [1] */
    { MN_CALL_EVT_CONNECT,              HUAWEI_PM_RSP_ID_UR_CONN},      /* ^CONN [3] */
    { MN_CALL_EVT_RELEASED,             HUAWEI_PM_RSP_ID_UR_CEND},      /* ^CEND [4] */
    { MN_CALL_EVT_INCOMING,             HUAWEI_PM_RSP_ID_VOICE},        /*^CRING[51], 新增*/
    { MN_CALL_EVT_ECCLIST_UPDATE_IND,   HUAWEI_PM_RSP_ID_UR_ECCLIST},   /* ^ECCLIST  */
};

static mn_msg_node pm_mn_msg_table[] = 
{
    { MN_MSG_EVT_STORAGE_EXCEED, HUAWEI_PM_RSP_ID_UR_SMMEMFULL },/* ^SMMEMFULL [7] */
    /*  +CDS/+CMT/+CDSI/+CMTI/+CBM [31] */
    { MN_MSG_EVT_DELIVER_CBM,    HUAWEI_PM_RSP_ID_SMS },
    { MN_MSG_EVT_DELIVER,        HUAWEI_PM_RSP_ID_SMS },
    { MN_MSG_EVT_SUBMIT_RPT,     HUAWEI_PM_RSP_ID_RESULT_CODE },
    { MN_MSG_EVT_MSG_SENT,       HUAWEI_PM_RSP_ID_RESULT_CODE },
};

static l4a_msg_node pm_l4a_msg_table[] = 
{
    { ID_MSG_L4A_RSSI_IND,     HUAWEI_PM_RSP_ID_UR_RSSI },    /* ^RSSI  */
    { ID_MSG_L4A_HCSQ_IND,     HUAWEI_PM_RSP_ID_UR_RSSI },    /* ^HCSQ 控制方法和RSSI是一样的	*/
    { ID_MSG_L4A_ANLEVEL_IND,  HUAWEI_PM_RSP_ID_UR_RSSI },    /* ^ANLEVEL 控制方法和RSSI是一样的*/
};

#define pm_phone_table_len   (sizeof(pm_phone_table) / sizeof(pm_phone_node))
#define pm_csCall_table_len  (sizeof(pm_cs_call_table) / sizeof(cs_call_node))
#define pm_mnMsg_table_len   (sizeof(pm_mn_msg_table) / sizeof(mn_msg_node))
#define pm_l4a_table_len     (sizeof(pm_l4a_msg_table) / sizeof(l4a_msg_node))

/*The cache queue for store responses*/
/*It's only accessed in this file,
other modules can't visit it directly*/
huawei_pm_rsp_cache_type huawei_pm_rsp_cache = 
{
    0,
    0,
    huawei_pm_rsp_buf
};

dsat_curc_cfg_type g_curc_ctrl_cfg = 
{
    DSAT_UR_ENABLE
};

remote_wakeup_info_type gstHuaweiWakeupCfg = 
{
    REMOTE_WAKEUP_ON, 
    RMT_WK_CHN_PIN | RMT_WK_CHN_USB,
    RMT_WK_SRC_SMS | RMT_WK_SRC_VOICE | RMT_WK_SRC_DATA | RMT_WK_SRC_UR
};

huawei_pm_rsp_info_type huawei_pm_at_rsp_info_table[] = 
{   /*      Msg_id                        RMT_WK_type         CURC_type        cache wether repeat  */
    {HUAWEI_PM_RSP_ID_VOICE,            RMT_WK_SRC_VOICE,   HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_SMS,              RMT_WK_SRC_SMS,     HUAWEI_PM_RSP_GROUP_NONE,   TRUE},
    {HUAWEI_PM_RSP_ID_TCPIP_DATA,       RMT_WK_SRC_DATA,    HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_ASYN_CMD,         RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_RESULT_CODE,      RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_MODE,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_CURC01, FALSE},
    {HUAWEI_PM_RSP_ID_UR_RSSI,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_CURC01, FALSE},
    {HUAWEI_PM_RSP_ID_UR_CSNR,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_CURC01, FALSE},
    {HUAWEI_PM_RSP_ID_UR_SRVST,         RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_REG,           RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_SIMST,         RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_NWTIME,        RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_ACTIVEBAND,    RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_CURC01, FALSE},
    {HUAWEI_PM_RSP_ID_UR_ANLEVEL,       RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_CURC01, FALSE},
    {HUAWEI_PM_RSP_ID_UR_LOCCHD,        RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_SIMFILEREFRESH, RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_SMMEMFULL,     RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_POSITION,      RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_GPSBASIC,      RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_WNINV,         RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_POSEND,        RMT_WK_SRC_GPS,     HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_WPDCP,         RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_WPDDL,         RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_WPDOP,         RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_CTZV,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_BOOT,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_CURC01, FALSE},
    {HUAWEI_PM_RSP_ID_UR_NDISEND,       RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_DSFLOWRPT,     RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_CURC01, FALSE},
    {HUAWEI_PM_RSP_ID_UR_EARST,         RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_CURC01, FALSE},
    {HUAWEI_PM_RSP_ID_UR_ORIG,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_CONF,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_CONN,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_CEND,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_RFSWITCH,      RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_STIN,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_CUSD,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_SMS,           RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   TRUE},
    {HUAWEI_PM_RSP_ID_UR_RSSILVL,       RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_CURC01, FALSE},
    {HUAWEI_PM_RSP_ID_UR_HRSSILVL,      RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_CURC01, FALSE},
    {HUAWEI_PM_RSP_ID_UR_HDRRSSI,       RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_CURC01, FALSE},
    {HUAWEI_PM_RSP_ID_UR_CRSSI,         RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_CURC01, FALSE},
    {HUAWEI_PM_RSP_ID_UR_OTACMSG,       RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_DSDORMANT,     RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_IPDATA,        RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   TRUE},
    {HUAWEI_PM_RSP_ID_UR_THERM,         RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_XDSTATUS,      RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},        
    {HUAWEI_PM_RSP_ID_UR_CLIP,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE}, 
    {HUAWEI_PM_RSP_ID_UR_CCWA,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE}, 
    {HUAWEI_PM_RSP_ID_UR_CSSI,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE}, 
    {HUAWEI_PM_RSP_ID_UR_CSSU,          RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},   
    {HUAWEI_PM_RSP_ID_UR_IPSTATE,       RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE}, 
    {HUAWEI_PM_RSP_ID_UR_CUSATP,        RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_CUSATEND,      RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_ECLSTAT,       RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_ECCLIST,       RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},     
    {HUAWEI_PM_RSP_ID_UR_FOTASTATE,     RMT_WK_SRC_UR,      HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
    {HUAWEI_PM_RSP_ID_UR_GPS,           RMT_WK_SRC_GPS,     HUAWEI_PM_RSP_GROUP_NONE,   FALSE},
};
/*lint -e63*/

BSP_S32 BSP_huawei_pm_rsp_inform_flush_cache(void);

/*lint +e63*/
#if (FEATURE_ON == MBB_WPG_TRUST_SNUM)
extern VOS_BOOL sms_num_cmp_trust_num( VOS_VOID);
#endif/*FEATURE_ON == MBB_WPG_TRUST_SNUM*/
/**********************************************************************
函 数 名: pm_print_message
功能描述: PM LOG打印
输入参数: 格式化字符串及3个参数
输出参数: None
返 回 值: None
注意事项: 支持3个参数,不支持分级打印
***********************************************************************/
void pm_print_message( char *tring, int arg1, int arg2, int arg3 )
{
    vos_printf(tring, arg1, arg2, arg3); 
}

/**********************************************************************
函 数 名: pm_ccore_print_log
功能描述: C core LOG打印
输入参数: 格式化字符串及3个参数
输出参数: None
返 回 值    : None
注意事项: 支持3个参数,支持2个等级打印
***********************************************************************/
void pm_ccore_print_log( int print_onoff, char *tring, int arg1, int arg2, int arg3 )
{
    if ( g_PmCcorePrintOnOff == (unsigned int)print_onoff ) 
    { 
        pm_print_message(tring, arg1, arg2, arg3);
    } 
}


/******************************************************************************
  函数名称  : get_id_to_index_phone()
  功能描述  : 获取MN_CALLBACK_PHONE类消息对应的curc的位域
  输入参数  : evt_id - 消息的ID
  被调函数  : 无
  输出参数  : 无
  返 回 值  : 对应curc的位域
              VOS_ERROR - 异常
 ********************************************************************************/
static huawei_pm_rsp_id_type huawei_pm_get_id_to_index_phone(VOS_UINT8 evt_id)
{
    int loop = 0;
    for( ; loop < pm_phone_table_len ; loop++)
    {
        if(evt_id == pm_phone_table[loop].taf_id)
        {
            return pm_phone_table[loop].rsp_id;
        }
    }
    pm_ccore_print_log(PM_MSG_PRINT_ON, \
        "[module_pm]:get_id_to_index_phone: evt_idx error !!!\r\n", 0, 0, 0);

    return HUAWEI_PM_RSP_ID_UR_NONE;
    
}

/******************************************************************************
  函数名称  : get_id_to_index_cs_call()
  功能描述  : 获取MN_CALLBACK_CS_CALL类消息对应的curc的位域
  输入参数  : evt_id - 消息的ID
  被调函数  : 无
  输出参数  : 无
  返 回 值  : 对应curc的位域
              VOS_ERROR - 异常
 ********************************************************************************/
static huawei_pm_rsp_id_type huawei_pm_get_id_to_index_cs_call(MN_CALL_EVENT_ENUM_U32 evt_id)
{
    int loop = 0;
    for( ; loop < pm_csCall_table_len ; loop++)
    {
        if(evt_id == pm_cs_call_table[loop].taf_id)
        {
            return pm_cs_call_table[loop].rsp_id;
        }
    }
    pm_ccore_print_log(PM_MSG_PRINT_ON, \
        "[module_pm]:get_id_to_index_cs_call: evt_idx error !!!\r\n", 0, 0, 0);

    return HUAWEI_PM_RSP_ID_UR_NONE;
}

/******************************************************************************
  函数名称  : get_id_to_index_msg()
  功能描述  : 获取MN_CALLBACK_MSG类消息对应的curc的位域
  输入参数  : evt_id - 消息的ID
  被调函数  : 无
  输出参数  : 无
  返 回 值  : 对应curc的位域
              VOS_ERROR - 异常
 ********************************************************************************/
static huawei_pm_rsp_id_type huawei_pm_get_id_to_index_msg(MN_MSG_EVENT_ENUM_U32 evt_id)
{
    int loop = 0;
    for( ; loop < pm_mnMsg_table_len ; loop++)
    {
        if(evt_id == pm_mn_msg_table[loop].taf_id)
        {
            return pm_mn_msg_table[loop].rsp_id;
        }
    }

    pm_ccore_print_log(PM_MSG_PRINT_ON, \
        "[module_pm]:get_id_to_index_msg: evt_idx %d error !!!\r\n", evt_id, 0, 0);
    return HUAWEI_PM_RSP_ID_UR_NONE;    
}

/******************************************************************************
  函数名称  : get_id_to_index_msg()
  功能描述  : 获取MN_CALLBACK_MSG类消息对应的curc的位域
  输入参数  : evt_id - 消息的ID
  被调函数  : 无
  输出参数  : 无
  返 回 值  : 对应curc的位域
              VOS_ERROR - 异常
 ********************************************************************************/
static huawei_pm_rsp_id_type huawei_pm_get_id_to_index_l4a(APS_L4A_MSG_ID_ENUM_UINT32 evt_id)
{
    int loop = 0;
    for( ; loop < pm_l4a_table_len ; loop++)
    {
        if(evt_id == pm_l4a_msg_table[loop].taf_id)
        {
            pm_ccore_print_log(PM_MSG_PRINT_ON, \
                "[module_pm]:huawei_pm_get_id_to_index_l4a: evt_idx 0x%x  succes \r\n", evt_id, 0, 0);
            return pm_l4a_msg_table[loop].rsp_id;
        }
    }

    pm_ccore_print_log(PM_MSG_PRINT_ON, \
        "[module_pm]:huawei_pm_get_id_to_index_l4a: evt_idx 0x%x  error !!!\r\n", evt_id, 0, 0);
    return HUAWEI_PM_RSP_ID_UR_NONE;    
} 


/******************************************************************************
  函数名称  : get_id_to_index_stk()
  功能描述  : 获取STK ID到index的映射, 即在curc 中的位域
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值  : 对应curc的位域
********************************************************************************/
static huawei_pm_rsp_id_type huawei_pm_get_id_to_index_stk(VOS_VOID)
{
    /* STK的主动上报为:^STIN , 在curc 的29bit */
    return HUAWEI_PM_RSP_ID_UR_STIN;
}
/******************************************************************************
  函数名称  : get_id_to_index_ps_call()
  功能描述  : 获取ps call ID到index的映射，即在curc 中的位域
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值      : 对应curc的位域
 ********************************************************************************/
static huawei_pm_rsp_id_type huawei_pm_get_id_to_index_ps_call(VOS_VOID)
{
    /* ps call大类中的^DSFLOWRPT使用curc 控制，在curc 的22bit */
    return HUAWEI_PM_RSP_ID_UR_DSFLOWRPT;
}

/******************************************************************************
  函数名称  : huawei_pm_get_id_to_index_ss()
  功能描述  : 获取补充业务ss client ID到index的映射，即在curc 中的位域
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值  : 对应curc的位域
 ********************************************************************************/
static huawei_pm_rsp_id_type huawei_pm_get_id_to_index_ss(void)
{
    return HUAWEI_PM_RSP_ID_UR_CUSD;
}
spinlock_t usb_s34_lock = {0};
/*USB s3/s4的标记，FALSE表示进入S3/S4，TRUE表示退出*/
static unsigned char usb_s34_flag = FALSE;
unsigned char remote_wakeupctrl_bakeup = REMOTE_WAKEUP_ON;

/*************************************************************
* 函   数   名 : huawei_pm_usb_s34_state_entry_handle
* 功能描述  : USB进入s3/s4阶段的处理函数
* 输入参数  : NA
* 输出参数  :
* 返 回 值     : NA
***************************************************************/
static void huawei_pm_usb_s34_entry_handle(void)
{
    int ret = 0;
    int value = PM_MSG_USB_S34_IN;
    unsigned int icc_channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_PM;
	unsigned long flags = 0;

    spin_lock_irqsave(&usb_s34_lock,flags); 

    if(TRUE == usb_s34_flag)
    {
        spin_unlock_irqrestore(&usb_s34_lock,flags);
        return;
	}

    /*备份远程唤醒开关，并关A核闭远程唤醒*/
    remote_wakeupctrl_bakeup = gstHuaweiWakeupCfg.remote_wakeup_ctrl;
    gstHuaweiWakeupCfg.remote_wakeup_ctrl = REMOTE_WAKEUP_OFF;

    usb_s34_flag = TRUE;

    spin_unlock_irqrestore(&usb_s34_lock,flags);
}

/*************************************************************
* 函   数   名 : huawei_pm_usb_s34_state_exit_handle
* 功能描述  : USB退出s3/s4阶段的处理函数
* 输入参数  : NA
* 输出参数  :
* 返 回 值     : NA
***************************************************************/
static void huawei_pm_usb_s34_exit_handle(void)
{
    int ret = 0;
    int value = PM_MSG_USB_S34_OUT;
    unsigned int icc_channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_PM;
    unsigned long flags = 0;

    spin_lock_irqsave(&usb_s34_lock,flags); 

    if(FALSE == usb_s34_flag)
    {
        spin_unlock_irqrestore(&usb_s34_lock,flags);
        return;
    }
    /*恢复远程唤醒开关的原始状态值*/
    gstHuaweiWakeupCfg.remote_wakeup_ctrl = remote_wakeupctrl_bakeup;

    usb_s34_flag = FALSE;

    spin_unlock_irqrestore(&usb_s34_lock,flags);

}


/*****************************************************************************
 函 数 名  : bsp_pm_read_cb
 功能描述  : C核核间回调函数
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static BSP_S32 bsp_pm_read_cb( void )
{
    int ret = 0;
    int read_len = 0;
    int value;
    unsigned int channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_PM;

    read_len = bsp_icc_read(channel_id, (unsigned char*)&value, sizeof(value));
    if(read_len != (int)sizeof(value))
    {
        pm_ccore_print_log(PM_MSG_PRINT_ON,\
            "[module_pm]:bsp_pm_read_cb: read len(%x) != expected len(%x) !\n", 0, 0, 0);
        
        return -1;
    }

    pm_ccore_print_log(PM_MSG_PRINT_ON,\
        "[module_pm]:bsp_pm_read_cb: get value is %d !\n", value, 0, 0);
    switch(value)
    {
        case PM_MSG_FLUSH_CACHE:
            ret = BSP_huawei_pm_rsp_inform_flush_cache();
            break;
        case PM_MSG_USB_S34_IN:
            huawei_pm_usb_s34_entry_handle();
            break;
        case PM_MSG_USB_S34_OUT:
            huawei_pm_usb_s34_exit_handle();
            break;
        default:
            pm_ccore_print_log(g_PmCcorePrintOnOff,\
                "[module_pm]:bsp_pm_read_cb: invalid para !\n", 0, 0, 0);
            break;
	}

    return ret;
}
/******************************************************************************
  函数名称  : huawei_pm_at_cache_init()
  功能描述  : 初始化at cache
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值      : VOS_ERR - 初始化失败;
                                VOS_OK - 初始化成功.
********************************************************************************/
VOS_UINT32 huawei_pm_at_cache_init(void)
{
    static VOS_BOOL init_flag = FALSE;
    int ret = 0;
    unsigned int icc_channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_PM;
    remote_wakeup_info_type local_config = {0};

    pm_ccore_print_log(PM_MSG_PRINT_ON, "[module_pm]: Ccore module_pm init... \n", 0, 0, 0);
    
    if (FALSE == init_flag)
    {
        ret |= bsp_icc_event_register(icc_channel_id, \
            (read_cb_func)bsp_pm_read_cb, NULL, NULL, NULL);

        /*read nv item*/
        if (NV_OK == NV_Read(en_NV_Item_WAKEUP_CFG_FLAG, 
                                &local_config, sizeof(local_config)))
        {
            gstHuaweiWakeupCfg.remote_wakeup_channel = 
                local_config.remote_wakeup_channel;
            gstHuaweiWakeupCfg.remote_wakeup_ctrl = 
                local_config.remote_wakeup_ctrl;
            gstHuaweiWakeupCfg.remote_wakeup_source = 
                local_config.remote_wakeup_source;
        }

        init_flag = TRUE;
    }
    return VOS_OK;
}

/******************************************************************************
  函数名称  : huawei_pm_usb_is_sleep()
  功能描述  : 判断usb是否处于suspend状态
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值      : 1-待机状态 0-非待机状态,                
********************************************************************************/
int huawei_pm_usb_is_sleep(void) 
{
    BSP_U32 usb_status = 0;
    pm_status_s *pm_st = (pm_status_s *)(SRAM_MODULE_PM_ADDR);
    usb_status = pm_st->usb_status;

    if( USB_SUSPEND_STATUS ==  usb_status )
    {
        pm_ccore_print_log(PM_MSG_PRINT_ON,\
            "[module_pm]:huawei_pm_usb_is_sleep: usb suspend!\n", 0, 0, 0);
        return 1;
    }
    if( USB_RESUME_STATUS ==  usb_status )
    {
        pm_ccore_print_log(PM_MSG_PRINT_ON,\
            "[module_pm]:huawei_pm_usb_is_sleep: usb resumed!\n", 0, 0, 0);
        return 0;
    }
    pm_ccore_print_log(PM_MSG_PRINT_ON,\
        "[module_pm]:huawei_pm_usb_is_sleep: GET MEMORY_USB_STATUS_ADDR ERROR!\n", 0, 0, 0);
    return 1;
}

/******************************************************************************
  函数名称  : huawei_pm_pin_is_sleep()
  功能描述  : 通过判断PIN状态获取上位机是否睡眠
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值  : 1-待机状态 0-非待机状态,                
********************************************************************************/
int huawei_pm_pin_is_sleep(void) 
{
    BSP_U32 pin_status = 0;
    pm_status_s *pm_st = (pm_status_s *)(SRAM_MODULE_PM_ADDR);

    pin_status = pm_st->pin_status;

    if( PIN_SUSPEND_STATUS ==  pin_status )
    {
        pm_ccore_print_log(PM_MSG_PRINT_ON,
            "[module_pm]:huawei_pm_pin_is_sleep: pin suspend!\n", 0, 0, 0);
        return 1;
    }
    if( PIN_RESUME_STATUS ==  pin_status )
    {
        pm_ccore_print_log(PM_MSG_PRINT_ON,
            "[module_pm]:huawei_pm_pin_is_sleep: pin resumed!\n", 0, 0, 0);
        return 0;
    }
    pm_ccore_print_log(PM_MSG_PRINT_ON,
        "[module_pm]:huawei_pm_pin_is_sleep: get value fail !\n", 0, 0, 0);
    return 0;
}

/******************************************************************************
  函数名称  : huawei_pm_usb_is_present()
  功能描述  : 判断usb是否在位
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值  : 1-待机状态 0-非待机状态,                
********************************************************************************/
boolean huawei_pm_usb_is_present(void) 
{
    BSP_U32 usb_status = 0;
    pm_status_s *pm_st = (pm_status_s *)(SRAM_MODULE_PM_ADDR);

    usb_status = pm_st->usb_status;

    if(( USB_SUSPEND_STATUS != usb_status ) && ( USB_RESUME_STATUS !=  usb_status ))
    {
        pm_ccore_print_log(PM_MSG_PRINT_ON, "[module_pm]:usb is not present !\n", 0, 0, 0);
        return FALSE;
    }
    else
    {
        pm_ccore_print_log(PM_MSG_PRINT_ON, "[module_pm]:usb is present !\n", 0, 0, 0);
        return TRUE;
    }    
}

/******************************************************************************
Function    : huawei_pm_wake_source_check
Description :   
Input       : rmt_wk_src_type rmt_wk_src
Output      : None
Return      : boolean wether the src can wakeup host
Others      : None
******************************************************************************/
boolean huawei_pm_wake_source_check(rmt_wk_src_type rmt_wk_src)
{
    /*Check params validity*/
    if ((RMT_WK_SRC_VOICE > rmt_wk_src) || (RMT_WK_SRC_MAX < rmt_wk_src))
    {
        return FALSE;
    }
    /*Check wakeup source validity*/    
    if ((REMOTE_WAKEUP_OFF == gstHuaweiWakeupCfg.remote_wakeup_ctrl) \
            || (rmt_wk_src != (rmt_wk_src & gstHuaweiWakeupCfg.remote_wakeup_source)) \
            || ( 0 == gstHuaweiWakeupCfg.remote_wakeup_channel ))
    {
        return FALSE;
    }

    return TRUE;
}

/******************************************************************************
Function:       huawei_pm_wake_channel_check
Description:   used by c_core
Input:           rmt_wk_chl_type channel
Output:         None
Return:          boolean wether the channel can wakeup host
Others:          None
******************************************************************************/
boolean huawei_pm_wake_channel_check(rmt_wk_chl_type channel)
{
    if ((RMT_WK_CHN_PIN > channel) || (RMT_WK_CHN_MAX < channel))
    {
        return FALSE;
    }
    
    if (channel == (gstHuaweiWakeupCfg.remote_wakeup_channel & channel))
    {
        return TRUE;
    }
    
    return FALSE;    
}

/******************************************************************************
Function:       huawei_pm_get_curc_status
Description:   
Input:           huawei_pm_rsp_id_type send_id
Output:         None
Return:          dsat_curc_status
Others:          None
******************************************************************************/
dsat_curc_status huawei_pm_get_curc_status(void)
{
    return g_curc_ctrl_cfg.ctrl_status;
}

/******************************************************************************
Function:       huawei_pm_rsp_find_id
Description:   
Input:           huawei_pm_rsp_id_type send_id
Output:         None
Return:          None
Others:          None
******************************************************************************/
static uint32 huawei_pm_rsp_find_id(huawei_pm_rsp_id_type send_id)
{
    uint32 i = 0;

    /*ARR_SIZE(huawei_pm_at_rsp_info_table) is the size 
      of huawei_pm_at_rsp_info_table*/
    for ( i = 0; 
            i < sizeof(huawei_pm_at_rsp_info_table) / sizeof(huawei_pm_at_rsp_info_table[0]);
            i++)
    {
        if (send_id == huawei_pm_at_rsp_info_table[i].id)
        {
            return i;
        }
    }

    return HUAWEI_PM_RSP_ERROR_ID;
}

/******************************************************************************
Function:       huawei_pm_at_cache_store
Description:   
Input:           VOS_UINT32 index, VOS_PID Pid, VOS_VOID *pMsg
Output:         None
Return:          None
Others:          None
******************************************************************************/
static void huawei_pm_at_cache_store
(
    VOS_UINT32 idx,   
    VOS_PID Pid, 
    VOS_VOID *pMsg
)
{
    uint32 i = 0;
    /*The current pos*/
    uint32 pos = 0;     
    /*Used to free old memory*/
    huawei_pm_rsp_proc_type *tmp_ptr = NULL;
    
    if (FALSE == huawei_pm_at_rsp_info_table[idx].duplicate)
    {
        /*The current pos*/
        pos = huawei_pm_rsp_cache.head; 
    
        /*Check id whether exist*/
        for (i = 0; i < HUAWEI_PM_RSP_CACHE_LEN(huawei_pm_rsp_cache); i++)
        {
            if ((huawei_pm_rsp_cache.buf_array[pos].id)
                    == huawei_pm_at_rsp_info_table[idx].id)
            {
                /*Find,We overlay this position*/
                /*Free old memory*/
                tmp_ptr = &huawei_pm_rsp_cache.buf_array[pos];
                (void)VOS_FreeMsg( tmp_ptr->pid, tmp_ptr->pMsg );
                /*Update the response*/
                huawei_pm_rsp_cache.buf_array[pos].pid = Pid;
                huawei_pm_rsp_cache.buf_array[pos].pMsg = pMsg;
                return;
            }
            else
            {
                /*The next response*/
                pos = HUAWEI_PM_RSP_CACHE_NEXT(pos);                
            }
        }    
    }

    /*The default store position*/
    pos = huawei_pm_rsp_cache.tail;
    
    /*Cache full,overlay the oldest*/
    if (HUAWEI_PM_RSP_CACHE_FULL(huawei_pm_rsp_cache))
    {
        /*Free old memory*/
        tmp_ptr 
          = &huawei_pm_rsp_cache.buf_array[huawei_pm_rsp_cache.head];
        (void)VOS_FreeMsg( tmp_ptr->pid, tmp_ptr->pMsg );
        huawei_pm_rsp_cache.head 
                = HUAWEI_PM_RSP_CACHE_NEXT(huawei_pm_rsp_cache.head);
    }

    /*Store the response*/
    huawei_pm_rsp_cache.buf_array[pos].id = huawei_pm_at_rsp_info_table[idx].id;
    huawei_pm_rsp_cache.buf_array[pos].pid = Pid;
    huawei_pm_rsp_cache.buf_array[pos].pMsg = pMsg;
    /*Update tail index*/
    huawei_pm_rsp_cache.tail 
        = HUAWEI_PM_RSP_CACHE_NEXT(huawei_pm_rsp_cache.tail);
    
}

/******************************************************************************
Function:       huawei_pm_id_converse
Description:   converse the platform_format msg to huawei_pm_rsp_id_type
Input:           VOS_PID Pid, VOS_VOID *pMsg
Output:         None
Return:          huawei_pm_rsp_id_type
Others:          None
******************************************************************************/
huawei_pm_rsp_id_type huawei_pm_id_converse(VOS_PID Pid, VOS_VOID *pMsg)
{
    /*信息接收方的PID*/
    VOS_UINT32          ulPid;
    /*查看大类call/data/msg-MN_CALLBACK_CS_CALL等*/
    VOS_UINT32 msg_name = 0;
    /*查看事件ID如:ID MN_MSG_EVT_STORAGE_FULL等*/
    VOS_UINT32 msg_at_id_ps_call = 0;
    huawei_pm_rsp_id_type evt_id;
    /* 查看MN_CALLBACK_PHONE 大类的事件ID */
    VOS_UINT8      msg_at_id_phone = 0;
    /* 查看MN_CALLBACK_CS_CALL 大类的事件ID */
    MN_CALL_EVENT_ENUM_U32 msg_at_id_cs_call = 0; 
    /* 查看MN_CALLBACK_MSG 大类的事件ID */
    MN_MSG_EVENT_ENUM_U32 msg_at_id_msg = 0; 
    /* 查看STK_AT_EVENT_CNF大类的事件ID */
    VOS_UINT8 msg_at_id_stk = 0; 
    /* 查看LTE 主动上报的事件ID */
    VOS_UINT32 msg_at_l4_l4a_id = 0;

    MsgBlock                *pMsgCtrlBlk;
    TAF_PS_EVT_STRU         *pAppMsgCtrlBlk;
    MN_AT_IND_EVT_STRU      *pAtMsgCtrlBlk;
    MN_APP_STK_AT_CNF_STRU  *pSTKMsgCtrlBlk;
    MN_AT_IND_EVT_STRU      *pSTKVoiceCtrlBlk;

    if (NULL == pMsg)
    {
        pm_ccore_print_log(PM_MSG_PRINT_ON, \
            "[module_pm]:huawei_pm_id_converse:  pMsg is NULL  !!!\r\n", 0, 0, 0);
        return HUAWEI_PM_RSP_ID_UR_NONE;
    }

    pMsgCtrlBlk = (MsgBlock*)(pMsg);
    ulPid = pMsgCtrlBlk->ulReceiverPid;

    /*这个消息判断为NVBACKUP命令的回复消息*/
    if (( WUEPS_PID_DRV_AGENT == Pid ) && ( WUEPS_PID_AT == ulPid ))
    {
         pm_ccore_print_log(PM_MSG_PRINT_ON, \
             "[module_pm]:huawei_pm_id_converse: Pid = %d, ulPid = %d\r\n", Pid, ulPid, 0);
        return HUAWEI_PM_RSP_ID_RESULT_CODE;
    }

    if (( UEPS_PID_MTA == Pid ) && ( WUEPS_PID_AT == ulPid ))
    {
         pm_ccore_print_log(PM_MSG_PRINT_ON, 
             "huawei_pm_id_converse: get jamming ur, "
             "Pid = %d, ulPid = %d \n", Pid, ulPid, 0);
        return HUAWEI_PM_RSP_ID_RESULT_CODE;
    }
    
    /*TAF模块主动上报*/
    /*MN_CALLBACK_PS_CALL消息的结构体MN_APP_PS_EVT_STRU*/
    /*其他消息的结构体MN_AT_IND_EVT_STRU*/
    if ( ( WUEPS_PID_TAF == Pid ) && ( WUEPS_PID_AT == ulPid ) )
    {
        /*先默认是MN_CALLBACK_PS_CALL*/
        pAppMsgCtrlBlk = (TAF_PS_EVT_STRU*)(pMsg);
        msg_name = pAppMsgCtrlBlk->stHeader.ulMsgName ;
        if ( MN_CALLBACK_PS_CALL == msg_name )
        {
            pm_ccore_print_log(PM_MSG_PRINT_ON, \
                "[module_pm]:This is a ps call ur\r\n", 0, 0, 0);

            /*msg_at_id: PS Call细分的类,如:ID_EVT_TAF_PS_REPORT_DSFLOW_IND*/
            msg_at_id_ps_call = pAppMsgCtrlBlk->ulEvtId;

            /*PS Call中只有ID_EVT_TAF_PS_REPORT_DSFLOW_IND是curc中定义的主动上报*/
            if ( ID_EVT_TAF_PS_REPORT_DSFLOW_IND == msg_at_id_ps_call )
            {
                evt_id = huawei_pm_get_id_to_index_ps_call();
                pm_ccore_print_log(PM_MSG_PRINT_ON, \
                    "[module_pm]:This is ^DSFLOWRPT ur, the send_id is %d\r\n", evt_id, 0, 0);
                return evt_id;
            }
            else
            { 
                pm_ccore_print_log(PM_MSG_PRINT_ON, \
                    "[module_pm]:This is a ps result code: %d\r\n", msg_at_id_ps_call, 0, 0);
                return HUAWEI_PM_RSP_ID_RESULT_CODE;
            }
        }
        else
        {
            pAtMsgCtrlBlk = (MN_AT_IND_EVT_STRU*)(pMsg);
            if (MN_CALLBACK_PHONE == pAtMsgCtrlBlk->usMsgName)
            {
                msg_at_id_phone = *(VOS_UINT8*)(pAtMsgCtrlBlk->aucContent);
                pm_ccore_print_log(PM_MSG_PRINT_ON, \
                    "[module_pm]:This is phone ur, we want g_id_to_index_msg[%d]\r\n",
                     msg_at_id_phone, 0, 0);
                evt_id = huawei_pm_get_id_to_index_phone(msg_at_id_phone);
                pm_ccore_print_log(PM_MSG_PRINT_ON, \
                    "[module_pm]:This is a phone ur, the send_id is %d\r\n", evt_id, 0, 0);
                return evt_id;

            }
            else if (MN_CALLBACK_CS_CALL == pAtMsgCtrlBlk->usMsgName)
            {
                msg_at_id_cs_call = *(MN_CALL_EVENT_ENUM_U32*)(pAtMsgCtrlBlk->aucContent);
                pm_ccore_print_log(PM_MSG_PRINT_ON, \
                    "[module_pm]:This is cs call ur, we want g_id_to_index_msg[%d]\r\n", 
                    msg_at_id_cs_call, 0, 0);
                evt_id = huawei_pm_get_id_to_index_cs_call(msg_at_id_cs_call);
                pm_ccore_print_log(PM_MSG_PRINT_ON, \
                    "[module_pm]:This is cs call ur, the send_id is %d\r\n", evt_id, 0, 0);
                return evt_id;
            }
            else if (MN_CALLBACK_MSG == pAtMsgCtrlBlk->usMsgName)
            {
                msg_at_id_msg = *(MN_MSG_EVENT_ENUM_U32*)(pAtMsgCtrlBlk->aucContent);
                pm_ccore_print_log(PM_MSG_PRINT_ON, \
                    "[module_pm]:This is MSG ur, we want g_id_to_index_msg[%d]\r\n",
                     msg_at_id_msg, 0, 0);
                evt_id = huawei_pm_get_id_to_index_msg(msg_at_id_msg);
                pm_ccore_print_log(PM_MSG_PRINT_ON, \
                    "[module_pm]:This is MSG ur, the send_id is %d\r\n", evt_id, 0, 0);
                return evt_id;
            }
            else if ( MN_CALLBACK_SS == pAtMsgCtrlBlk->usMsgName )
            {
                MN_CLIENT_ID_T msg_ss_client_id;
                msg_ss_client_id = 
                    ((TAF_SS_CALL_INDEPENDENT_EVENT_STRU *)(pAtMsgCtrlBlk->aucContent))->ClientId; 
                pm_ccore_print_log(PM_MSG_PRINT_ON, \
                    "[module_pm]:This is a USSD message, client id = %x\n", msg_ss_client_id, 0, 0);

                evt_id = huawei_pm_get_id_to_index_ss();
                pm_ccore_print_log(PM_MSG_PRINT_ON, \
                    "[module_pm]:#This is USSD ur, the send_id is %d\r\n", evt_id, 0, 0);
                return evt_id;
            }
            else
            {
                pm_ccore_print_log(PM_MSG_PRINT_ON, \
                    "[module_pm]:This is other TAF cmd, the msg_id is %d\r\n", pAtMsgCtrlBlk->usMsgName, 0, 0);
            }
            return HUAWEI_PM_RSP_ID_UR_NONE;
        }
    }

    /*STK模块主动上报:MN_APP_STK_AT_CNF_STRU*/
    else if ( ( MAPS_STK_PID == Pid ) && ( WUEPS_PID_AT == ulPid ) )
    {
        pSTKMsgCtrlBlk = (MN_APP_STK_AT_CNF_STRU*)(pMsg);
        msg_at_id_stk = pSTKMsgCtrlBlk->stSTKAtCnf.STKCBEvent;
        if ( ( SI_STK_CMD_IND_EVENT == msg_at_id_stk ) || ( SI_STK_CMD_END_EVENT == msg_at_id_stk ) )
        {
            evt_id = huawei_pm_get_id_to_index_stk();
            pm_ccore_print_log(PM_MSG_PRINT_ON, \
                "[module_pm]:This is STK ur, the send_id is %d\r\n", evt_id, 0, 0);
            return evt_id;
        }
        else
        {
            pSTKVoiceCtrlBlk = (MN_AT_IND_EVT_STRU *)(pMsg);
            if( MN_CALLBACK_VOICE_CONTROL == pSTKVoiceCtrlBlk->usMsgName )
            {
                /*modified for coverity*/
                VOS_UINT8 msg_at_id_voice = *( VOS_UINT8* )(pSTKVoiceCtrlBlk->aucContent); 
                if (MN_CALL_EVT_INCOMING == msg_at_id_voice)
                {
                    pm_ccore_print_log(PM_MSG_PRINT_ON, \
                        "[module_pm]:This is RING ur, the send_id is 0\r\n", 0, 0, 0);
                    return HUAWEI_PM_RSP_ID_VOICE;
                }
                else
                {
                    pm_ccore_print_log(PM_MSG_PRINT_ON, \
                        "[module_pm]:This is other voice control, the msg_id is %d\r\n", msg_at_id_voice, 0, 0);
                }
            }
            else
            {
                pm_ccore_print_log(PM_MSG_PRINT_ON, \
                    "[module_pm]:This is other STK cmd, the msg_id is %d\r\n", msg_at_id_stk, 0, 0);
            }
        }
        return HUAWEI_PM_RSP_ID_UR_NONE;
    }
    /*UR form temper protect*/
    else if ( ( WUEPS_PID_SPY == Pid ) && ( WUEPS_PID_AT == ulPid ) )
    {
        TEMP_PROTECT_EVENT_AT_IND_STRU *pstSpyMsg;
        pstSpyMsg = (TEMP_PROTECT_EVENT_AT_IND_STRU *)(pMsg);
        if ( ID_TEMPPRT_STATUS_AT_EVENT_IND == pstSpyMsg->ulMsgId )
        {
            pm_ccore_print_log(PM_MSG_PRINT_ON, 
                "[module_pm]:This is temp_protect ur, the send_id is 44\r\n", 0, 0, 0);
            return HUAWEI_PM_RSP_ID_UR_THERM;
        }
        else
        {
            pm_ccore_print_log(PM_MSG_PRINT_ON, 
                "[module_pm]:This is other SPY, the msg_id is %d\r\n", pstSpyMsg->ulMsgId, 0, 0);
        }
    }
    else if ( ( MSP_L4_L4A_PID == Pid ) && ( WUEPS_PID_AT == ulPid ) )
    {
        msg_at_l4_l4a_id  = ((MSG_HEADER_STRU *)pMsg)->ulMsgName;
        evt_id = huawei_pm_get_id_to_index_l4a(msg_at_l4_l4a_id);
        pm_ccore_print_log(PM_MSG_PRINT_ON, \
            "[module_pm]:This is l4a msg, the send_id is %d\r\n", evt_id, 0, 0);
        return evt_id;
    }

    return HUAWEI_PM_RSP_ID_UR_NONE;
}

/******************************************************************************
Function:       huawei_pm_send_update_channel_sta
Description:   
Input:           
Output:         None
Return:         None
Others:         None
******************************************************************************/
void huawei_pm_send_update_channel_sta(void)
{

    int ret = 0;
    int value = PM_MSG_RMT_CHANNEL_SET;
    unsigned int channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_PM;

    if(sizeof(value) != (u32)(ret = bsp_icc_send(ICC_CPU_APP, \
                                channel_id, (unsigned char*)&value, sizeof(value))))
    {
        pm_ccore_print_log(PM_MSG_PRINT_ON, 
            "[module_pm]:send update_wakeup_channel msg to Acore fail !!! \r\n", 0, 0, 0);
        return ;
    }

    pm_ccore_print_log(PM_MSG_PRINT_ON, 
        "[module_pm]:send update_wakeup_channel msg to Acore success !!! \r\n", 0, 0, 0);

    return ;
}
/******************************************************************************
Function:       huawei_pm_send_remote_wakepin_req
Description:   
Input:           
Output:         None
Return:          None
Others:          None
******************************************************************************/
void huawei_pm_send_remote_wakepin_req(void)
{
    int ret = 0;
    int value = PM_MSG_WAKEUP_PIN_SET;
    unsigned int channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_PM;

    if(sizeof(value) != (u32)(ret = bsp_icc_send(ICC_CPU_APP, \
                        channel_id, (unsigned char*)&value, sizeof(value))))
    {
        pm_ccore_print_log(PM_MSG_PRINT_ON, 
            "[module_pm]:send remote_wakepin_req msg to Acore fail !!! \r\n", 0, 0, 0);
        return ;
    }

    pm_ccore_print_log(PM_MSG_PRINT_ON, 
        "[module_pm]:send remote_wakepin_req msg to Acore success !!! \r\n", 0, 0, 0);

    return ;
}

/******************************************************************************
Function    : BSP_huawei_pm_send_request_wakeup_timer
Description : 远程唤醒过程中，起缓存数据的超时定时器
Input   : None        
Output  : None
Return  : None
Others  : None
******************************************************************************/
void huawei_pm_send_cache_timer_req(void)
{
    int ret = 0;
    int value = PM_MSG_WAKEUP_TIMER_SET;
    unsigned int channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_PM;

    if(sizeof(value) != (u32)(ret = bsp_icc_send(ICC_CPU_APP, \
                                channel_id, (unsigned char*)&value, sizeof(value))))
    {
        pm_ccore_print_log(PM_MSG_PRINT_ON, 
            "[module_pm]:send cache_timer msg to Acore fail !!! \r\n", 0, 0, 0);
        return ;
    }

    pm_ccore_print_log(PM_MSG_PRINT_ON, 
        "[module_pm]:send cache_timer msg to Acore success !!! \r\n", 0, 0, 0);

    return ;
}

/******************************************************************************
  函数名称  : huawei_pm_remote_check_and_handle()
  功能描述  : 判断是否需要通过PIN脚或USB唤醒主机。
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值      : 无
********************************************************************************/
void huawei_pm_remote_check_and_handle
(
    unsigned char en_wakeup_host,
    unsigned char *store
)
{
    unsigned char pin_channel = FALSE;
    unsigned char usb_channel = FALSE;

    /*USB在位且非suspend情况下，不用远程唤醒*/
    if((TRUE == huawei_pm_usb_is_present())\
        && (FALSE == huawei_pm_usb_is_sleep()))
    {
        /*非suspend模式下，不用输出1s高电平*/
        return;
    }

    if(FALSE == en_wakeup_host)
    {
        return;
    }

    pin_channel = huawei_pm_wake_channel_check(RMT_WK_CHN_PIN);
    usb_channel = huawei_pm_wake_channel_check(RMT_WK_CHN_USB);

    if (TRUE == usb_channel)
    {
        huawei_pm_send_update_channel_sta();
    }

    if( FALSE == huawei_pm_pin_is_sleep() )
    {
        return;
    }
    
    /*允许作为远程唤醒源，且支持PIN方式远程唤醒*/
    if (TRUE == pin_channel)
    {
        huawei_pm_send_remote_wakepin_req();
        /*模块和上位机握手,缓存起超时定时器*/
        huawei_pm_send_cache_timer_req();
    }

    /*只有PIN唤醒源，消息需要先缓存然后等待握手后发出*/
    if ( (TRUE == pin_channel) && ( FALSE == usb_channel ) )
    {
        *store = TRUE;
    }
    else
    {
        *store = FALSE;
    }
    return;
}

/******************************************************************************
  函数名称  : huawei_pm_check_data_type()
  功能描述  : 判断是否(curc = 2)控制的主动上报命令
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值      : HUAWEI_PM_RSP_ROUTE_STORE,                
                            : HUAWEI_PM_RSP_ROUTE_DISCARD,            
                            : HUAWEI_PM_RSP_ROUTE_TRANSFER,           
                            : HUAWEI_PM_RSP_ROUTE_MAX,
********************************************************************************/
huawei_pm_rsp_route_type huawei_pm_check_data_type_ex
(
    huawei_pm_rsp_id_type send_id
) 
{
    /*The index in huawei_pm_acore_rsp_info_table*/
    uint32 idx = 0;
    /*Whether or not wake host*/
    boolean wakeup_host = FALSE; 
    boolean usb_store = FALSE; 
    /*The wakeup source type of response*/
    rmt_wk_src_type wake_source = RMT_WK_SRC_MAX; 
    /*Current curc status*/
    dsat_curc_status curc_val = DSAT_UR_ENABLE;   

    
    /*Check params validity*/
    if ((HUAWEI_PM_RSP_ID_UR_NONE >= send_id) 
          || (HUAWEI_PM_RSP_ID_UR_MAX <= send_id))
    {
        return HUAWEI_PM_RSP_ROUTE_TRANSFER;
    }

    if ( ( HUAWEI_PM_RSP_ID_RESULT_CODE == send_id ) || \
        ( HUAWEI_PM_RSP_ID_ASYN_CMD == send_id ) )
    {
        huawei_pm_remote_check_and_handle(TRUE, &usb_store);
        if ( TRUE == usb_store)
        {
            return HUAWEI_PM_RSP_ROUTE_STORE;
        }
        return HUAWEI_PM_RSP_ROUTE_TRANSFER;
    }

    /*Get index*/
    idx = huawei_pm_rsp_find_id(send_id);
    if (HUAWEI_PM_RSP_ERROR_ID == idx)
    {
        return HUAWEI_PM_RSP_ROUTE_DISCARD;
    }

    /*Get wakeup source*/
    wake_source = huawei_pm_at_rsp_info_table[idx].wake_source;
    /*Get curc status*/
    curc_val = huawei_pm_get_curc_status();
    /*Wakup host check*/
    wakeup_host = huawei_pm_wake_source_check(wake_source);

    pm_ccore_print_log(PM_MSG_PRINT_ON, "[module_pm]:send_id = %d\n", send_id, 0, 0);
    pm_ccore_print_log(PM_MSG_PRINT_ON, \
        "[module_pm]:wake_source = %d #curc_val = %d #wakeup_host = %d\n", \
        wake_source, curc_val, wakeup_host );

    if (DSAT_UR_DISABLE == curc_val) /*curc=0*/
    {
        /*受curc=0/1控制的命令*/
        if (HUAWEI_PM_RSP_GROUP_CURC01 
            == (HUAWEI_PM_RSP_GROUP_CURC01 
            & huawei_pm_at_rsp_info_table[idx].group))
        {
            return HUAWEI_PM_RSP_ROUTE_DISCARD;           
        }
    }
    
    #if (FEATURE_ON == MBB_WPG_TRUST_SNUM)
    if (( HUAWEI_PM_RSP_ID_SMS == send_id ) && ( TRUE == wakeup_host ))
    {
        /*不在白名单内的短信要缓存*/
        if (VOS_FALSE == sms_num_cmp_trust_num())
        {
            pm_ccore_print_log(PM_MSG_PRINT_ON, 
                "[module_pm]:this is not a trust SMS, set wakeup_host = 0\n", 0, 0, 0);
            wakeup_host = FALSE;
        }
    }
    #endif/*FEATURE_ON == MBB_WPG_TRUST_SNUM*/
    /*不受curc=0/1控制的命令，直接transfer，同时需要判断是否需要远程唤醒PIN信号*/
    huawei_pm_remote_check_and_handle(wakeup_host, &usb_store);

    /*attention: this modification is for when ^wakeupcfg set this source cannot wake host*/
    if( FALSE == huawei_pm_usb_is_sleep() )
    {
        return HUAWEI_PM_RSP_ROUTE_TRANSFER;
    }
    else if( FALSE == huawei_pm_pin_is_sleep() )
    {
        return HUAWEI_PM_RSP_ROUTE_TRANSFER;
    }
    else
    {
        if ( FALSE == wakeup_host )
        {
            if ( HUAWEI_PM_RSP_ID_SMS == send_id )
            {
                return HUAWEI_PM_RSP_ROUTE_STORE;
            }
            return HUAWEI_PM_RSP_ROUTE_DISCARD ;
        }
        else
        {
            if (( TRUE == usb_store ) && ( HUAWEI_PM_RSP_ID_TCPIP_DATA != send_id ))
            {
                return HUAWEI_PM_RSP_ROUTE_STORE;
            }
            return HUAWEI_PM_RSP_ROUTE_TRANSFER;
        }
    }
}

/******************************************************************************
  函数名称  : huawei_pm_check_data_type()
  功能描述  : 判断是否(curc = 2)控制的主动上报命令
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值      : HUAWEI_PM_RSP_ROUTE_STORE,                
                            : HUAWEI_PM_RSP_ROUTE_DISCARD,            
                            : HUAWEI_PM_RSP_ROUTE_TRANSFER,           
                            : HUAWEI_PM_RSP_ROUTE_MAX,
********************************************************************************/
boolean huawei_pm_check_data_type(unsigned long Pid, void *pMsg) 
{
    huawei_pm_rsp_id_type send_id;
    huawei_pm_rsp_route_type route;

    /*The index in huawei_pm_at_rsp_info_table*/
    uint32 idx = 0;
    boolean result = FALSE;

    send_id = huawei_pm_id_converse(Pid, pMsg);

    route = huawei_pm_check_data_type_ex(send_id);

    switch ( route )
    {
        case HUAWEI_PM_RSP_ROUTE_STORE:
        {
            idx = huawei_pm_rsp_find_id(send_id);
            huawei_pm_at_cache_store(idx, Pid, pMsg);
            result = FALSE;
            break;
        }
        case HUAWEI_PM_RSP_ROUTE_DISCARD:
        {
            /*消息确定丢弃后,需要释放消息占用的内存*/
            (void)VOS_FreeMsg( Pid, pMsg );
            result = FALSE;
            break;
        }
            
        case HUAWEI_PM_RSP_ROUTE_TRANSFER: 
        default:
        {
            result = TRUE;
            break;
        }
    }
    return result;
}
/******************************************************************************
Function:       huawei_pm_set_at_cache_struct
Description:   set the value of g_curc_ctrl_cfg, called by at^curc
Input:           src_curc which is gained from at command from A core.
Output:         None
Return:         int : 0=success
Others:         None
******************************************************************************/
void huawei_pm_update_wakeupcfg_struct(remote_wakeup_info_type *src_wakeupcfg)
{
    if ( NULL == src_wakeupcfg )
    {
        return;
    }     
    gstHuaweiWakeupCfg.remote_wakeup_ctrl = src_wakeupcfg->remote_wakeup_ctrl;
    gstHuaweiWakeupCfg.remote_wakeup_channel = src_wakeupcfg->remote_wakeup_channel;
    gstHuaweiWakeupCfg.remote_wakeup_source = src_wakeupcfg->remote_wakeup_source;
    return;
}

/******************************************************************************
Function:       huawei_pm_update_curc_para
Description:   set the value of g_curc_ctrl_cfg, called by at^curc
Input:           src_curc which is gained from at command from A core.
Output:         None
Return:         int : 0=success
Others:         None
******************************************************************************/
void huawei_pm_update_curc_para(dsat_curc_cfg_type *src_curc)
{
    int i = 0;

    if( NULL == src_curc )
    {
        return;
    }        
    g_curc_ctrl_cfg.ctrl_status =  src_curc->ctrl_status;
    
    return;
}
/******************************************************************************
Function:       huawei_pm_at_cache_flush
Description:   
Input:           void
Output:         None
Return:          void
Others:          None
******************************************************************************/
static void huawei_pm_at_cache_flush(void)
{
    uint32 i = 0;
    /*The current pos*/
    uint32 pos = huawei_pm_rsp_cache.head;    
    /*Response params as follow*/
    huawei_pm_rsp_proc_type  msg_struct;
    
    pm_ccore_print_log(PM_MSG_PRINT_ON, "[module_pm]: pm_at_cache_flush entered!\n", 0, 0, 0);

    for (i = 0; i < HUAWEI_PM_RSP_CACHE_LEN(huawei_pm_rsp_cache); i++)
    {
        /*Get response params*/
        msg_struct.pid
         = huawei_pm_rsp_cache.buf_array[pos].pid;
        msg_struct.pMsg
         = huawei_pm_rsp_cache.buf_array[pos].pMsg;
    
        /*Send response*/
        (void)V_PmCache_SendMsgByICC(msg_struct.pid, &(msg_struct.pMsg), VOS_FILE_ID, __LINE__ );
        /*The next response*/
        pos = HUAWEI_PM_RSP_CACHE_NEXT(pos);        
    }

    /*Clear cache empty*/
    huawei_pm_rsp_cache.head = huawei_pm_rsp_cache.tail;

}

/******************************************************************************
Function:       BSP_huawei_pm_rsp_inform_flush_cache
Description:   双核通信接口回调
Input:           None
Output:         None
Return:         uint32
Others:         None
******************************************************************************/
BSP_S32 BSP_huawei_pm_rsp_inform_flush_cache(void)
{
    pm_ccore_print_log(PM_MSG_PRINT_ON, 
        "[module_pm]: pm_rsp_inform_flush_cache entered!\n", 0, 0, 0);
    huawei_pm_at_cache_flush();
    return VOS_OK;
}
#endif/*MBB_MODULE_PM*/

