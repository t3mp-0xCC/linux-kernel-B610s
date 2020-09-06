




#ifndef __HUAWEI_PM_AT_CACHE_H
#define __HUAWEI_PM_AT_CACHE_H

#include "product_config.h"

#if (FEATURE_ON == MBB_MODULE_PM) 
/*----------------------------------------------*
 * 简单数据类型定义说明                         *
 *----------------------------------------------*/
#ifndef boolean
typedef unsigned char      boolean;
#endif

#ifndef uint64
typedef unsigned long long uint64;
#endif

#ifndef _UINT32_DEFINED
typedef  unsigned int      uint32;      /* Unsigned 32 bit value */
#define _UINT32_DEFINED
#endif

#ifndef _UINT16_DEFINED
typedef  unsigned short    uint16;      /* Unsigned 16 bit value */
#define _UINT16_DEFINED
#endif

#ifndef _UINT8_DEFINED
typedef  unsigned char      uint8;       /* Unsigned 8  bit value */
#define _UINT8_DEFINED
#endif

#ifndef _INT32_DEFINED
typedef  signed int         int32;       /* Signed 32 bit value */
#define _INT32_DEFINED
#endif

#ifndef _INT16_DEFINED
typedef  signed short       int16;       /* Signed 16 bit value */
#define _INT16_DEFINED
#endif

#ifndef _INT8_DEFINED
typedef  signed char        int8;        /* Signed 8  bit value */
#define _INT8_DEFINED
#endif
/*record the sleep vote state*/
extern int logMsg
(
    char *fmt, 
    int arg1, 
    int arg2,
    int arg3,
    int arg4,
    int arg5,
    int arg6
);

/* 主动上报的大类中包含的小类的最大值 */
/*curc控制的最大主动上报AT命令个数*//*Max buffer numbers for rsp cache*/
#define HUAWEI_PM_RSP_MAX_CACHE_SIZE (128)  
/*ID error flag*/
#define HUAWEI_PM_RSP_ERROR_ID      (0xFFFFFF)   

/*Cache buffer len*/
#define HUAWEI_PM_RSP_CACHE_LEN(pool) ((pool.tail    \
                                 - pool.head         \
                                 + HUAWEI_PM_RSP_MAX_CACHE_SIZE)  \
                                 % HUAWEI_PM_RSP_MAX_CACHE_SIZE)
                                 
/*Next cache index*/
#define HUAWEI_PM_RSP_CACHE_NEXT(index) (((index) + 1)  \
                             % HUAWEI_PM_RSP_MAX_CACHE_SIZE) 
                                  
/*Cache buffer full*/
#define HUAWEI_PM_RSP_CACHE_FULL(pool)       \
                (HUAWEI_PM_RSP_CACHE_NEXT(pool.tail)  \
                                  == pool.head) 

/*Cache buffer empty*/
#define HUAWEI_PM_RSP_CACHE_EMPTY(pool)    \
                (pool.head == pool.tail)   

/*Define the wakeup switch*/
typedef enum
{
   REMOTE_WAKEUP_OFF,       /*Close*/
   REMOTE_WAKEUP_ON,        /*Open*/
}remote_switch_type;

/*Define the Wakeup Channel*/
typedef enum
{
    RMT_WK_CHN_PIN = 0x01,   /*WAKEUP_OUT PIN channel*/
    RMT_WK_CHN_USB = 0x02,   /*USB channel*/
    RMT_WK_CHN_MAX = 0x03
}rmt_wk_chl_type;

/*Define the wakeup source*/
typedef enum
{
    RMT_WK_SRC_VOICE = 0x01,   /*Voice*/
    RMT_WK_SRC_SMS   = 0x02,   /*SMS*/
    RMT_WK_SRC_DATA  = 0x04,   /*IP packets*/
    RMT_WK_SRC_UR    = 0x08,   /*Unsolicited Report*/
    RMT_WK_SRC_GPS   = 0x10,   /*GPS data*/
    RMT_WK_SRC_MAX   = 0x1F
}rmt_wk_src_type;

/*Define the route type of the response*/
typedef enum
{
    HUAWEI_PM_RSP_ROUTE_STORE,      /*Store into cache*/ 
    HUAWEI_PM_RSP_ROUTE_DISCARD,    /*Discard the response*/ 
    HUAWEI_PM_RSP_ROUTE_TRANSFER,   /*Send to host anyway,if USB suspend,then wake up host*/
    HUAWEI_PM_RSP_ROUTE_MAX,
}huawei_pm_rsp_route_type;

/*Curc control status*/
typedef enum
{
    DSAT_UR_DISABLE = 0,         /*curc = 0*/
    DSAT_UR_ENABLE = 1,          /*curc = 1*/
    DSAT_UR_CLASS_CONTROL = 2,   /*curc = 2*/
}dsat_curc_status;

typedef enum
{
    PM_MSG_RMT_CHANNEL_SET = 0, /*C->A更新通道状态*/
    PM_MSG_WAKEUP_PIN_SET,      /*C->A输出远程唤醒pin信号*/
    PM_MSG_WAKEUP_TIMER_SET,    /*C->Atimer*/
    PM_MSG_FLUSH_CACHE,         /*A->C发送缓存消息*/ 
    PM_MSG_USB_S34_IN,          /*A->C通知USB进入S3/S4阶段*/ 
    PM_MSG_USB_S34_OUT,         /*A->C通知USB退出S3/S4阶段*/ 
}MODULE_PM_MSG_ID;

typedef struct
{
    unsigned int usb_status;
    unsigned int pin_status;
}pm_status_s;


/*Curc control struct*/
typedef struct
{
    dsat_curc_status ctrl_status;                       /*Curc status*/
}dsat_curc_cfg_type;

/*Level control status*/
typedef enum
{
    PM_MSG_PRINT_OFF = 0,
    PM_MSG_PRINT_ON,
    PM_MSG_PRINT_MAX
}PM_MSG_ON_OFF;


/*Define response IDs*/
typedef enum
{
    HUAWEI_PM_RSP_ID_UR_NONE = -1,

    HUAWEI_PM_RSP_ID_VOICE,           /*Voice class,such as:RING&CRING*/
    
    HUAWEI_PM_RSP_ID_SMS,             /*SMS class,such as:+CMT&+CMTI&+CBM&CDS&CDSI*/
    
    HUAWEI_PM_RSP_ID_TCPIP_DATA,      /*IP packets class*/
    
    HUAWEI_PM_RSP_ID_ASYN_CMD,        /*Asynchronous command*/
    
    HUAWEI_PM_RSP_ID_RESULT_CODE,     /*Final result code,such as:CONNECT&BUSY&NO CARRIER*/

    HUAWEI_PM_RSP_ID_UR_MODE,         /*^MODE*/
    HUAWEI_PM_RSP_ID_UR_RSSI,         /*^RSSI*/ 
    HUAWEI_PM_RSP_ID_UR_CSNR,         /*^CSNR*/
    HUAWEI_PM_RSP_ID_UR_SRVST,        /*^SRVST */
     
    /*4*/
    HUAWEI_PM_RSP_ID_UR_REG,          /*+CREG &+CGREG */
    HUAWEI_PM_RSP_ID_UR_SIMST,        /*^SIMST */
    HUAWEI_PM_RSP_ID_UR_NWTIME,       /*^NWTIME*/
    HUAWEI_PM_RSP_ID_UR_ACTIVEBAND,    /*^ACTIVEBAND*/
   
    /*8*/
    HUAWEI_PM_RSP_ID_UR_ANLEVEL,        /*^ANLEVEL*/
    HUAWEI_PM_RSP_ID_UR_LOCCHD,         /*^LOCCHD*/
    HUAWEI_PM_RSP_ID_UR_SIMFILEREFRESH, /*^SIMFILEREFRESH AT&T feature*/
    HUAWEI_PM_RSP_ID_UR_SMMEMFULL,      /*^SMEMFULL*/

    /*12*/
    HUAWEI_PM_RSP_ID_UR_POSITION,      /*^POSITION*/
    HUAWEI_PM_RSP_ID_UR_GPSBASIC,      /*^TIMESETRULT/^DATASETRULT/^DATAVALIDITY*/
    HUAWEI_PM_RSP_ID_UR_WNINV,         /*^WNINV*/ 
    HUAWEI_PM_RSP_ID_UR_POSEND,        /*^POSEND*/

    /*16*/
    HUAWEI_PM_RSP_ID_UR_WPDCP,       /*^WPDCP*/
    HUAWEI_PM_RSP_ID_UR_WPDDL,       /*^WPDDL*/
    HUAWEI_PM_RSP_ID_UR_WPDOP,       /*^WPDOP*/
    HUAWEI_PM_RSP_ID_UR_CTZV,        /*+CTZV*/

    /*20*/
    HUAWEI_PM_RSP_ID_UR_NDISEND,     /*^NDISEND*/
    HUAWEI_PM_RSP_ID_UR_BOOT,        /*^BOOT */
    HUAWEI_PM_RSP_ID_UR_DSFLOWRPT,   /*^DSFLOWRPT*/
    HUAWEI_PM_RSP_ID_UR_EARST,       /*^EARST*/

    /*24*/
    HUAWEI_PM_RSP_ID_UR_ORIG,        /*^ORIG*/
    HUAWEI_PM_RSP_ID_UR_CONF,        /*^CONF*/
    HUAWEI_PM_RSP_ID_UR_CONN,        /*^CONN*/
    HUAWEI_PM_RSP_ID_UR_CEND,        /*^CEND*/

    /*28*/
    HUAWEI_PM_RSP_ID_UR_RFSWITCH,    /*^RFSWITCH*/
    HUAWEI_PM_RSP_ID_UR_STIN,        /*^STIN*/ 
    HUAWEI_PM_RSP_ID_UR_CUSD,        /*+CUSD*/
    HUAWEI_PM_RSP_ID_UR_SMS,         /*CMT,CMTI,CDS,CDSI,CBM,Convert from HUAWEI_PM_RSP_ID_SMS*/

    /*32*/
    HUAWEI_PM_RSP_ID_UR_RSSILVL,     /*^RSSILVL*/
    HUAWEI_PM_RSP_ID_UR_HRSSILVL,    /*^HRSSILVL*/
    HUAWEI_PM_RSP_ID_UR_HDRRSSI,     /*^HDRRSSI*/
    HUAWEI_PM_RSP_ID_UR_CRSSI,       /*^CRSSI*/
    
    /*36*/
    HUAWEI_PM_RSP_ID_UR_OTACMSG,     /*^OTACMSG*/
    HUAWEI_PM_RSP_ID_UR_DSDORMANT,   /*^DSDORMANT*/
    HUAWEI_PM_RSP_ID_UR_IPDATA,      /*^IPDATA*/
    HUAWEI_PM_RSP_ID_UR_THERM,       /*^THERM*/

    /*40*/
    HUAWEI_PM_RSP_ID_UR_XDSTATUS,    /*^XDSTATUS*/
    HUAWEI_PM_RSP_ID_UR_CLIP,        /*+CLIP*/
    HUAWEI_PM_RSP_ID_UR_CCWA,        /*+CCWA*/
    HUAWEI_PM_RSP_ID_UR_CSSI,        /*+CSSI*/

    /*44*/
    HUAWEI_PM_RSP_ID_UR_CSSU,        /*CSSU*/
    HUAWEI_PM_RSP_ID_UR_IPSTATE,     /*^IPSTATE*/
    HUAWEI_PM_RSP_ID_UR_CUSATP,      /*+CUSATP*/
    HUAWEI_PM_RSP_ID_UR_CUSATEND,    /*+CUSATEND*/

    /* 48 */
    HUAWEI_PM_RSP_ID_UR_PLACEHOLD,   /* MU609T 不使用这个位,保留 */
    HUAWEI_PM_RSP_ID_UR_ECLSTAT,     /* ^ECLSTAT */

    HUAWEI_PM_RSP_ID_UR_ECCLIST,
    HUAWEI_PM_RSP_ID_UR_FOTASTATE,
    HUAWEI_PM_RSP_ID_UR_NDISSTAT,    /*^NDISSTAT*/
    HUAWEI_PM_RSP_ID_UR_GPS,
    HUAWEI_PM_RSP_ID_UR_MAX,
}huawei_pm_rsp_id_type;

/*Define the group type of the response*/
typedef enum
{
    HUAWEI_PM_RSP_GROUP_NONE = 0x00,     /*Don't control by curc*/
    HUAWEI_PM_RSP_GROUP_CURC01 = 0x01,   /*Control by curc=0/1*/
    HUAWEI_PM_RSP_GROUP_MAX,
}huawei_pm_rsp_group_type;


/*Define the property struct for all responses*/
typedef struct huawei_pm_rsp_info_struct
{
    huawei_pm_rsp_id_type id;           /*The ID for the response*/
    rmt_wk_src_type wake_source;        /*What wakup source the respone belong to*/
    uint16 group;                       /*What curc group the respone belong to*/
    boolean duplicate;                  /*The flag indicates whether dupliciate store when cache*/
}huawei_pm_rsp_info_type;

/*Define process struct for response*/
typedef struct huawei_pm_rsp_proc_struct
{
    huawei_pm_rsp_id_type id;        /*The ID for the response*/
    unsigned long pid;
    void *pMsg;
    void *pBuf;
    uint16 pBuf_len;
    int8 port_idx;
}huawei_pm_rsp_proc_type;

/*Define cache struct for response*/
/*The cache get/put responses through FIFO*/
typedef struct huawei_pm_bsp_msg_struct
{
    uint32 ModuleId;
    uint32 FuncId;
}huawei_pm_bsp_msg_type;


/*缓存队列结构体*/
typedef struct huawei_pm_rsp_cache_struct
{
    uint32 head;                           /*Head index of the cache buffer*/
    uint32 tail;                           /*Tail index,This position doesn't contain data*/
    huawei_pm_rsp_proc_type *buf_array;    /*Cache buffer memory*/
}huawei_pm_rsp_cache_type;

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
boolean huawei_pm_check_data_type(unsigned long Pid, void *pMsg);
/******************************************************************************
Function:       huawei_pm_update_at_cache_struct
Description:   set the value of g_curc_ctrl_cfg, called by at^curc
Input:           src_curc which is gained from at command from A core.
Output:         None
Return:         int : 0=success
Others:         None
******************************************************************************/
void huawei_pm_update_curc_para(dsat_curc_cfg_type *src_curc);

#endif/*MBB_MODULE_PM*/

#endif/*__HUAWEI_PM_AT_CACHE_H*/

