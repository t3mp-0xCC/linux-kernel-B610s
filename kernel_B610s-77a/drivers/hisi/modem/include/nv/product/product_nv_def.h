

#ifndef __PRODUCT_NV_DEF_H__
#define __PRODUCT_NV_DEF_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define MAX_CHECK_NUM       1000

#ifndef BSP_S8
typedef signed char BSP_S8;
#endif
#ifndef BSP_U8
typedef unsigned char BSP_U8;
#endif
#ifndef BSP_S16
typedef signed short BSP_S16;
#endif
#ifndef BSP_U16
typedef unsigned short BSP_U16;
#endif
#ifndef BSP_S32
typedef signed int BSP_S32;
#endif
#ifndef BSP_U32
typedef unsigned int BSP_U32;
#endif
#ifndef BSP_S64
typedef signed long long BSP_S64;
#endif
#ifndef BSP_U64
typedef unsigned long long BSP_U64;
#endif

#ifndef VOS_UINT32
typedef unsigned int    VOS_UINT32;
#endif
#ifndef VOS_UINT16
typedef unsigned short  VOS_UINT16;
#endif
#ifndef VOS_UINT8
typedef unsigned char   VOS_UINT8;
#endif
#ifndef VOS_INT32
typedef signed int    VOS_INT32;
#endif
#ifndef VOS_INT16
typedef signed short  VOS_INT16;
#endif
#ifndef VOS_INT8
typedef signed char   VOS_INT8;
#endif

#define WEB_STR_LEN 36

#define MMA_HUAWEI_PLMN_MODIFY_MAX_SIZE                   (128)
#define TAF_NVIM_MAX_APN_LOCK_LEN                        (99)
#define TAF_NVIM_MAX_APN_LOCK_STR_LEN                    (TAF_NVIM_MAX_APN_LOCK_LEN + 1)
#define TAF_NVIM_HWLOCK_LEN                              (32)

/*请参考结构体样例*/
typedef struct
{
    int reserved;  /*note */
}PRODUCT_MODULE_STRU;

typedef struct
{
    VOS_UINT32 uwGuCrcResult;                      /* GU主卡NV的校验结果 */
    VOS_UINT32 uwTlCrcResult;                      /* TL主卡NV的校验结果 */
    VOS_UINT32 uwGuM2CrcResult;                    /* GU副卡NV的校验结果 */
}NV_CRC_CHECK_RESULT_STRU;

typedef struct
{
    VOS_UINT16 uhwValidCount;                       /* 指示有效数据的个数 */
    VOS_UINT16 uhwRsv;                              /* 保留位 */
    VOS_UINT16 auhwNeedCheckID[MAX_CHECK_NUM];      /* 保存需要检查的NV_ID值 */
}NV_CHECK_ITEM_STRU;

/*NV50336结构定义*/
typedef struct
{
    VOS_UINT8 SKU_TYPE;  /*合法取值 30、34 30：YMobile;34:SBM*/
    VOS_UINT8 RESERVED;  /*预留*/
}NV_ITEM_SKU_STRU;
/*NV50364电池参数数据结构**/
typedef struct
{
    VOS_UINT8 no_battery_powerup_enable;        /*非工厂模式下电池部在位开机使能标志*/
    VOS_UINT8 exception_poweroff_poweron_enable;/*异常关机后，下次插入电源进入开机模式使能标志*/
    VOS_UINT8 low_battery_poweroff_disable;     /*低电关机禁止标志*/
    VOS_UINT8 reserved;                         /*保留*/
}POWERUP_MODE_TYPE;

/*充电过程中温度参数NV 50385结构定义*/
typedef struct
{
    VOS_UINT32    ulChargeIsEnable;                    //充电温保护使能    
    VOS_INT32     overTempchgStopThreshold;           //充电高温保护门限
    VOS_INT32     subTempChgLimitCurrentThreshold;    //高温充电进入门限
    VOS_INT32     lowTempChgStopThreshold;            //充电低温保护门限
    VOS_INT32     overTempChgResumeThreshold;         //充电高温恢复温度门限
    VOS_INT32     lowTempChgResumeThreshold;          //充电低温恢复温度门限
    VOS_UINT32     chgTempProtectCheckTimes;           //充电停充轮询次数
    VOS_UINT32     chgTempResumeCheckTimes;            //充电复充轮询次数
    VOS_INT32     exitWarmChgToNormalChgThreshold;    //由高温充电恢复到常温充电温度门限
    VOS_INT32     reserved2;                          //预留    
}CHG_SHUTOFF_TEMP_PROTECT_NV_TYPE;

/*充电过程中温度参数NV50386结构定义*/
typedef struct
{
    VOS_INT32         battVoltPowerOnThreshold;           //开机电压门限
    VOS_INT32         battVoltPowerOffThreshold;          //关机电压门限
    VOS_INT32         battOverVoltProtectThreshold;       //平滑充电过压保护门限(平滑值)
    VOS_INT32         battOverVoltProtectOneThreshold;    //单次充电过压保护门限(单次值)
    VOS_INT32         battChgTempMaintThreshold;          //区分高温停充和正常停充的判断门限
    VOS_INT32         battChgRechargeThreshold;           //充电二次复充门限
    VOS_INT32         VbatLevelLow_MAX;                   //低电上限门限
    VOS_INT32         VbatLevel0_MAX;                     //0格电压上限门限
    VOS_INT32         VbatLevel1_MAX;                     //1格电压上限门限
    VOS_INT32         VbatLevel2_MAX;                     //2格电压上限门限
    VOS_INT32         VbatLevel3_MAX;                     //3格电压上限门限 
    VOS_INT32         reserved1;                          //预留
    VOS_INT32         reserved2;                          //预留
}CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE;

#define  MMC_MAX_LEN       (5)
typedef struct
{
    VOS_UINT32                   aulRoamMCCList[MMC_MAX_LEN];/* 存储非漫游的国家码 */
}NAS_MMC_NVIM_ROAM_MCC_CMP_STRU;


typedef struct
{
    VOS_UINT8                           ucRoamMncExactlyCmpFlg;
    VOS_UINT8                           aucRsv[3];                         /* 保留*/
} NAS_MMC_NVIM_ROAM_MNC_EXACTLY_CMP_STRU;

typedef struct 
{       
    VOS_UINT8     FileSysActiveProtectEnable;   /* 文件系统主动保护使能 */     
    VOS_UINT8     BadImgResumFromOnlineEnable;   /* 镜像损坏后从Online分区恢复使能*/     
    VOS_UINT8     BootNotOperaFileSysEnable;      /*系统启动过程不对文件系统操作使能*/     
    VOS_UINT8     FastOffFailTrueShutdownEnable;    /*假关机失败进入真关机功能使能*/      
    VOS_UINT8     SoftPowOffReliableEnable;          /*软件关机可靠性功能使能*/      
    VOS_UINT8     ZoneWriteProtectEnable;          /*分区写越界保护使能*/       
    VOS_UINT8     BadZoneReWriteEnable;            /* Flash病危块回写功能使能*/       
    VOS_UINT8     BootCanFromkernelBEnable;  /*主镜像破坏后可以从备份镜像启动使能*/
    VOS_UINT8     OnlineChangeNotUpgradEnable; /*Online分区变化不进行在线升级使能*/
    /*预留*/
    VOS_UINT8     BadZoneScanEnable;   /* Flash病危块扫描功能使能*/
    VOS_UINT8     reserved2;
    VOS_UINT8     reserved3;
    VOS_UINT8     reserved4;
    VOS_UINT8     reserved5;
    VOS_UINT8     reserved6;
    VOS_UINT8     MmcReliabilityEnable;            /*协议MMC可靠性保护使能开关*/
    VOS_UINT32    MmcReliabilityBitmap;     /* 通信协议协议MMC可靠性保护功能掩码 */
    VOS_UINT32    DangerTime;                   /*频繁上下电危险期经验值 ，单位为秒*/       
    VOS_UINT32    WaitTime;           /*应用通知到底软假关机后，定时器值单位为秒*/
    /*预留*/
    VOS_INT32    reserved7; 
    VOS_INT32    reserved8; 
    VOS_INT32    reserved9; 
    VOS_INT32    reserved10; 
    VOS_INT32    reserved11;       
}SOFT_RELIABLE_CFG_STRU;


typedef struct
{
    VOS_UINT8 webSite[WEB_STR_LEN];
}WEB_SITE_STRU;

typedef struct
{
    VOS_UINT8 wpsPin[WEB_STR_LEN];
}WPS_PIN_STRU;

typedef struct
{
    VOS_UINT8 userName[WEB_STR_LEN];
}WEB_USER_NAME_STRU;

#define CUSTOM_MCC_LIST_NUM_MAX                 (30)
typedef struct
{
    VOS_UINT8                           ucNvimActiveFlg;
    VOS_UINT8                           ucCustomMccNum;
    VOS_UINT8                           aucReserve[2];    
    VOS_UINT32                          aulCustommMccList[CUSTOM_MCC_LIST_NUM_MAX];  /* 允许漫游的国家码列表 */    
}NAS_MMC_NVIM_HPLMN_SEARCH_REGARDLESS_MCC_SUPPORT_STRU_EX;


#define DISABLE_RPLMN_ACT_ON       (0)
#define DISABLE_RPLMN_ACT_OFF      (1)
typedef struct
{
    VOS_UINT8                       ucEnableFlag;/*0: Disable RPLMN ACT 特性关闭,1: Disable RPLMN特性打开*/
    VOS_UINT8                       aucReserve[3];
}NAS_MCC_NVIM_RPLMN_DISABLE_ACT_CFG_STRU;

/*****************************************************************************
 结构名    : GID1_TYPE_STRU
 结构说明  : 用于定制GID1锁卡功能
 结构 ID   : 50083
*****************************************************************************/
typedef struct
{
    VOS_UINT8   bGID1Enable;                 /*GID1锁卡使能位*/
    VOS_UINT8   ucLeagalValue;               /*合法GID1值*/
}GID1_TYPE_STRU;

/*****************************************************************************
 结构名    : NAS_NV_Vodafone_CPBS
 结构说明  : 德国 Vodafone CPBS定制
 结构 ID   : 50429
*****************************************************************************/
typedef struct
{
   VOS_UINT16 Vodafone_CPBS;                 /*定制使能位*/
   VOS_UINT16 Reserved;
}NAS_NV_Vodafone_CPBS;

/*****************************************************************************
 结构名    : TAF_NV_CSIM_CUSTOMIZED
 结构说明  : AT+CSIM定制
 结构 ID   : 50189
*****************************************************************************/
typedef struct 
{
     VOS_UINT8  ucNvActiveFlag;     /* NV激活标志字段*/
     VOS_UINT8  ucReserved[3];         /* 保留字段*/
} TAF_NV_CSIM_CUSTOMIZED;


typedef struct
{
   VOS_UINT8    ucNvActiveFlag;
   VOS_UINT8    ucReserved[3];            /*保留字段*/
   VOS_UINT32   ulBandGroup1Low32Bit;
   VOS_UINT32   ulBandGroup1High32Bit;
   VOS_UINT32   ulBandGroup2Low32Bit;
   VOS_UINT32   ulBandGroup2High32Bit;
   VOS_UINT32   ulReserved[4];            /*保留字段，用于后续扩展*/
}NAS_NV_CUSTOMIZED_BAND_GROUP;


typedef struct
{
    VOS_UINT8 ucNvimActiveFlg;
    VOS_UINT8 aucReserved[3];                        /*保留字段*/
}NV_ITEM_START_MANUAL_TO_AUTO_ST;


#define MMA_SPECIAL_HPLMN_ITEM_SIZE     (4)      /* 单个PLMN空间大小*/
#define MMA_SPECIAL_HPLMN_LIST_SIZE     (30)     /* 最多可以存储30个PLMN*/

typedef struct
{
     VOS_UINT8            aucHPlmnId[MMA_SPECIAL_HPLMN_ITEM_SIZE];    /*4字节数组表示一个HPLMN*/
}MMA_SPEC_HPLMN_TYPE;

typedef struct
{
     VOS_UINT32           ulHPlmnCount;                               /*HPLMN个数*/
     MMA_SPEC_HPLMN_TYPE  astHPlmnGroup[MMA_SPECIAL_HPLMN_LIST_SIZE]; /*HPLMN列表*/
}TAF_MMA_SPEC_PLMN_NOT_ROAM_ST;


#define MMA_HUAWEI_PLMN_MODIFY_MAX_SIZE                   (128)

typedef struct
{
    VOS_UINT8 aucRcvPlmnData[MMA_HUAWEI_PLMN_MODIFY_MAX_SIZE];
}NAS_MMA_HUAWEI_PLMN_MODIFY_STRU;


typedef struct
{
    VOS_UINT8 ucCollected;
    VOS_UINT8 aucReserved;
}SALES_AGENT_RECORD_STRU;
/*****************************************************************************
 结构名    : NV_HUAWEI_DYNAMIC_VID_STRU
 结构说明  : 用于定制动态VID
 结构 ID   : 50394
*****************************************************************************/
typedef struct
{
    VOS_UINT16 nv_status;
    VOS_UINT16 huawei_dynamic_vid;
    
}NV_HUAWEI_DYNAMIC_VID_STRU;
/*****************************************************************************
 结构名    : NV_HUAWEI_DYNAMIC_BOOT_PID_STRU
 结构说明  : 用于定制动态升级模式PID
 结构 ID   : 50395
*****************************************************************************/
typedef struct
{
    VOS_UINT16 nv_status;
    VOS_UINT16 huawei_dynamic_boot_pid;
    
}NV_HUAWEI_DYNAMIC_BOOT_PID_STRU;
/*****************************************************************************
 结构名    : NV_HUAWEI_DYNAMIC_NORMAL_PID_STRU
 结构说明  : 用于定制动态正常模式PID
 结构 ID   : 50396
*****************************************************************************/
typedef struct
{
    VOS_UINT16 nv_status;
    VOS_UINT16 huawei_dynamic_normal_pid;
    
}NV_HUAWEI_DYNAMIC_NORMAL_PID_STRU;
/*****************************************************************************
 结构名    : NV_HUAWEI_DYNAMIC_BOOT_PID_STRU
 结构说明  : 用于定制动态调试模式PID
 结构 ID   : 50397
*****************************************************************************/
typedef struct
{
    VOS_UINT16 nv_status;
    VOS_UINT16 huawei_dynamic_debug_pid;
    
}NV_HUAWEI_DYNAMIC_DEBUG_PID_STRU;

typedef struct
{
     VOS_UINT8  ucNvActiveFlag;     /* NV激活标志字段*/
     VOS_UINT8  ucReserved[3];         /* 保留字段*/
}HPLMN_Within_EPLMN_NotRoam;

/*****************************************************************************
 结构名    : SI_PIH_IMSI_TIMER_CONFIG
 结构说明  : MMA保存IMSI读取周期的NV结构
 结构 ID    : 50426
*****************************************************************************/
typedef struct
{
    VOS_UINT8  ucNvActiveFlag;     /* NV激活标志字段*/
    VOS_UINT8  ucReserved;         /* 保留字段*/
    VOS_UINT16  usTimerLen;        /* 读取IMSI周期(单位为秒)*/
}SI_PIH_IMSI_TIMER_CONFIG;
/*****************************************************************************
 结构名    : TIM_CPIN_STRU
 结构说明  : AT+CPIN?定制
 结构 ID   : 50450
*****************************************************************************/
typedef struct 
{
     VOS_UINT8  ucNvActiveFlag;     /* NV激活标志字段*/
     VOS_UINT8  ucReserved[3];         /* 保留字段*/
} TIM_CPIN_STRU;

typedef struct
{
    VOS_UINT8   led_enable;        /* LED灯使能开关 */
    VOS_UINT8   led_dr;            /* LED灯所使用的DR */
    VOS_UINT8   led_mode;          /* LED灯模式标志 */
    VOS_UINT8   led_reserve;       /* 保留字段 */
    VOS_UINT32  full_on;           /* LED灯呼吸模式稳定亮的持续时间 */
    VOS_UINT32  full_off;          /* LED灯呼吸模式稳定暗的持续时间 */
    VOS_UINT32  fade_on;           /* LED灯呼吸模式从暗到亮的持续时间 */
    VOS_UINT32  fade_off;          /* LED灯呼吸模式从亮到暗的持续时间 */
    VOS_UINT32  delay_on;          /* LED灯闪烁模式亮的时间 */
    VOS_UINT32  delay_period;     /* LED灯闪烁模式的闪烁周期时间*/
    VOS_UINT32  full_long_on;     /* LED灯长亮的持续时间 */
    VOS_UINT32  full_long_off;    /* LED灯长暗的持续时间 */
    VOS_UINT32  brightness;       /* LED灯亮度电流值 */
}NV_LED_PARA_STRU;

#if (FEATURE_ON == MBB_COMMON)
#define NV_LED_FLICKER_DEFAULT_TIME                            (1)
#define NV_LED_FLICKER_MAX_TIME                                (100)
#define NV_LED_FLICKER_MAX_NUM                                 (2)
#define NV_GU_LED_SERVICE_STATE_NUM                            (12)
#define NV_LTE_LED_SERVICE_STATE_NUM                           (3)
#define NV_LED_SERVICE_STATE_NUM                               (15)
#endif

/*****************************************************************************
 结构名    : LED_CONTROL_NV_STRU
 结构说明  : LED_CONTROL_NV结构
*****************************************************************************/
typedef struct
{
    BSP_U8   ucLedColor;      /*三色灯颜色，对应LED_COLOR的值*/
    BSP_U8   ucTimeLength;    /*该配置持续的时间长度，单位100ms*/
}LED_CONTROL_NV_STRU;


typedef struct
{
    LED_CONTROL_NV_STRU   stLED[4];                                                 /* 一个闪灯周期内，至多两次闪烁 */
}M2M_LED_CONTROL_STRU;


typedef struct
{
    BSP_U8                              ucLedMode;                                  /* 闪灯模式 */
    BSP_U8                              unresolved[3];                              /* 保留 */
    M2M_LED_CONTROL_STRU                stLedStr[NV_GU_LED_SERVICE_STATE_NUM];      /* 闪灯配置 */
}NV_LED_SET_PARA_STRU;


typedef struct
{
    M2M_LED_CONTROL_STRU                stLedStr[NV_LTE_LED_SERVICE_STATE_NUM];     /* 闪灯配置 */
}NV_LED_SET_PARA_STRU_EXPAND;


typedef struct
{
    BSP_U8                              ucLedMode;                                  /* 闪灯模式 */
    BSP_U8                              unresolved[3];                              /* 保留 */
    M2M_LED_CONTROL_STRU                stLedStr[15];         /* 闪灯配置 */
}NV_LED_SET_PARA_STRU_COMBINED;

/*****************************************************************************
 结构名    : SMS_AUTO_REG_STRU
 结构说明  : 短信自注册定制
 结构 ID   : 50449
*****************************************************************************/
#define MAX_IMSI_LEN 15
typedef struct 
{
     VOS_UINT8  ucNvActiveFlag;                               /*NV激活标志字段*/
     VOS_UINT8  ucSmsRegFlag;                                 /*注册标志位*/
     VOS_UINT8  ucSmsRegImsi[MAX_IMSI_LEN];               /*已注册IMSI号*/
     VOS_UINT8  ucReserved[3];                                /*保留字段*/
} SMS_AUTO_REG_STRU;

/*****************************************************************************
 结构名    : TATA_DEVICELOCK_STRU
 结构说明  : 设备锁NV的结构
 结构 ID   : 50432
*****************************************************************************/
#define MAX_DLCK_ENCODE_LEN_MBB 32
typedef struct
{
    VOS_UINT32 ulStatus;                           /*设备锁激活状态*/
    VOS_UINT8   aucLockCode[MAX_DLCK_ENCODE_LEN_MBB];    /*设备锁的密码*/
}TATA_DEVICELOCK_STRU;

/*****************************************************************************
 结构名    : NAS_NV_HPLMN_FIRST_UMTS_TO_LTE
 结构说明  : YOTA定制，从3G到LTE时优先搜索EHPLMN
 结构 ID   : 50432
*****************************************************************************/
typedef struct
{
    VOS_UINT8 ucNvActiveFlag; /* 是否激活 YOTA 定制功能 */
    VOS_UINT8 aucReserved[3]; /* 保留 */
}NAS_NV_HPLMN_FIRST_UMTS_TO_LTE;


typedef struct
{
    VOS_UINT8    ucNvActiveFlag;   /* 是否激活 EE 运营商显示定制功能 */
    VOS_UINT8    aucReserved[3];   /* 保留 */
}NAS_NV_EE_OPERATOR_NAME_DISPLAY;


#define MODE_LIST_MAX_LEN    (6)
#define MODE_LIST_MAX_NUM    (10)

typedef struct
{
    VOS_UINT8    ucListItemNum;
    VOS_UINT8    ucRestrict;
    VOS_UINT8    aucModeList[MODE_LIST_MAX_NUM][MODE_LIST_MAX_LEN];
}NAS_NV_SYSCFGEX_MODE_LIST;


/* 锁APN设置 */
typedef struct
{
    VOS_UINT8   ucStatus;       /* 1: NV有效标志位，0：无效 */
    VOS_UINT8   ucApnLength;
    VOS_UINT8   aucApn[TAF_NVIM_MAX_APN_LOCK_STR_LEN];       /* APN从该数组的第一个字节开始写入，并且以'\0'作为结尾 */
    VOS_UINT8   aucRsv[2];
}APS_APN_LOCK_STRU;

/*****************************************************************************
 结构名    : NV_WINBLUE_PROFILE_STRU
 结构说明  : Windows8.1 特性控制NV
 结构 ID   : 50424
*****************************************************************************/
typedef struct
{
    VOS_UINT8  InterfaceName[32];
    VOS_UINT8  MBIMEnable;
    VOS_UINT8  CdRom;
    VOS_UINT8  TCard;
    VOS_UINT8  MaxPDPSession;
    VOS_UINT16 IPV4MTU;
    VOS_UINT16 IPV6MTU;
    VOS_UINT32 Reserved1;
    VOS_UINT32 Reserved2;
}NV_WINBLUE_PROFILE_STRU;

/*****************************************************************************
 结构名    : NV_USB_CDC_NET_SPEED_STRU
 结构说明  : 获取网络速度默认值的NV
 结构 ID   : 50456
*****************************************************************************/
typedef struct
{
    VOS_UINT32  nv_status;
    VOS_UINT32  net_speed;  
    VOS_UINT32  reserve1;
    VOS_UINT32  reserve2;
    VOS_UINT32  reserve3;
}NV_USB_CDC_NET_SPEED_STRU;

/*****************************************************************************
 结构名    : NV_DEVNUMW_TIME_TYPE
 结构说明  : AT^DEVNUMW限制客户修改imei的次数
 结构 ID   : 50431
*****************************************************************************/
typedef struct
{
    VOS_UINT32                 times;      /*客户可更改IMEI的次数，1：一次，0：不可更改 */
}NV_DEVNUMW_TIME_TYPE;



#define MAX_PORT_NUMBER 17
#define MAX_PORT_NUMBER_AND_ALIGN 19
typedef struct
{
    VOS_UINT32 nv_status;
    VOS_UINT8 first_config_port[MAX_PORT_NUMBER];
    VOS_UINT8 second_config_port[MAX_PORT_NUMBER];
    VOS_UINT8 third_config_port[MAX_PORT_NUMBER];
    VOS_UINT8 reserved_1[MAX_PORT_NUMBER];
    VOS_UINT8 reserved_2[MAX_PORT_NUMBER];
    VOS_UINT8 reserved_3[MAX_PORT_NUMBER_AND_ALIGN];  
}NV_HUAWEI_MULTI_C0NFIG_USB_PORT_STRU;
typedef struct 
{
    VOS_UINT32 nv_status;
    VOS_UINT32 setmode_flag;
} NV_HUAWEI_DEBUG_MODE_FLAG_STRU;

/*****************************************************************************
 结构名    : ss_coul_nv_info
 结构说明  : 库仑计NV
 结构 ID   : 50462
*****************************************************************************/

#define COUL_MAX_TEMP_LEN 10
#define COUL_RESERVED_LEN 4
typedef struct
{
    VOS_INT32 charge_cycles;
    VOS_INT32 r_pcb; // uohm
    VOS_INT32 v_offset_a;
    VOS_INT32 v_offset_b;
    VOS_INT32 c_offset_a;
    VOS_INT32 c_offset_b;
    VOS_INT16 temp[COUL_MAX_TEMP_LEN];
    VOS_INT16 real_fcc[COUL_MAX_TEMP_LEN];
    VOS_INT16 calc_ocv_reg_v;
    VOS_INT16 calc_ocv_reg_c;
    VOS_INT16 hkadc_batt_temp;
    VOS_INT16 hkadc_batt_id_voltage;
    VOS_INT32 start_cc;
    VOS_INT32 ocv_temp;
    VOS_INT32 limit_fcc;
    VOS_INT32 reserved[COUL_RESERVED_LEN];
}ss_coul_nv_info;

/*****************************************************************************
 结构名    : NV_PLATFORM_CATEGORY_SET_STRU
 结构说明  : 设置单板CAT等级
 结构 ID   : 50458
*****************************************************************************/
typedef struct
{
    VOS_UINT32   nv_status;
    VOS_UINT32   cat_value;
    VOS_UINT32   reserve1;
    VOS_UINT32   reserve2;
    VOS_UINT32   reserve3;
}NV_PLATFORM_CATEGORY_SET_STRU;

/*****************************************************************************
 函数名称  : WLAN_AT_WIINFO_CHANNELS_NVIM_STRU
 功能描述  : 查询WiFi的信道和功率结构体 
 结构 ID   :      50468，50469
*****************************************************************************/
#define WLAN_AT_WIINFO_CHN_BUF_SIZE (128) /* channels占用NV缓存大小 */
#define WLAN_AT_WIINFO_POW_BUF_SIZE (64)  /* power占用NV缓存大小 */

/* 新的AT^WiInfo CHANNELS的NV结构，只限定BUF上限，具体数据根据产品决定 */
typedef struct 
{
    VOS_UINT8 buf[WLAN_AT_WIINFO_CHN_BUF_SIZE];
}WLAN_AT_WIINFO_CHN_BUF_NVIM_STRU;
/* 新的AT^WiInfo POWER的NV结构，只限定BUF上限，具体数据根据产品决定 */
typedef struct 
{
    VOS_UINT8 buf[WLAN_AT_WIINFO_POW_BUF_SIZE];
}WLAN_AT_WIINFO_POW_BUF_NVIM_STRU;


typedef struct
{
    VOS_UINT32      ulDSLastLinkTime;                       /*DS最近一次连接时间*/
    VOS_UINT32      ulDSTotalSendFluxLow;                   /*DS累计发送流量低四个字节*/
    VOS_UINT32      ulDSTotalSendFluxHigh;                  /*DS累计发送流量高四个字节*/
    VOS_UINT32      ulDSTotalLinkTime;                      /*DS累计连接时间*/
    VOS_UINT32      ulDSTotalReceiveFluxLow;                /*DS累计接收流量低四个字节*/
    VOS_UINT32      ulDSTotalReceiveFluxHigh;               /*DS累计接收流量高四个字节*/

} TAF_APS_DSFLOW_NV_STRU_EX;

typedef struct
{
    TAF_APS_DSFLOW_NV_STRU_EX    DsflowNvWan[5]; /*根据目前的使用情况，及NV结构，支持最多保存5个WAN流量*/
} TAF_APS_DSFLOW_NV_STRU_EXT;

/*****************************************************************************
 结构名    : NV_HUAWEI_DYNAMIC_NAME
 结构说明  : 用于运营商定制的在PC上显示CDROM和SD 卡的字符串
 结构 ID   : 50108
*****************************************************************************/
#define DYNAMIC_CD_NAME_CHAR_NUM (28)
#define reserved_num_32  (32)
typedef struct
{
    VOS_UINT32 nv_status;
    VOS_UINT8 huawei_cdrom_dynamic_name[DYNAMIC_CD_NAME_CHAR_NUM];
    VOS_UINT8 huawei_sd_dynamic_name[DYNAMIC_CD_NAME_CHAR_NUM];
    VOS_UINT8 reserved[reserved_num_32];
}HUAWEI_DYNAMIC_NAME_STRU;


/*****************************************************************************
 结构名    : NV_HUAWEI_DYNAMIC_INFO_NAME
 结构说明  : 此NV项用于运营商定制的在PC上显示product manufacturer和configuration的字符串。
 结构 ID   : 50109
*****************************************************************************/
#define DYNAMIC_INFO_NAME_CHAR_NUM (40)
#define reserved_num_8  (8)
typedef struct
{
    VOS_UINT32 nv_status;
    VOS_UINT8 huawei_product_dynamic_name[DYNAMIC_INFO_NAME_CHAR_NUM];
    VOS_UINT8 huawei_manufacturer_dynamic_name[DYNAMIC_INFO_NAME_CHAR_NUM];
    VOS_UINT8 huawei_configuration_dynamic_name[DYNAMIC_INFO_NAME_CHAR_NUM];
    VOS_UINT8 reserved[reserved_num_8];
}HUAWEI_DYNAMIC_INFO_NAME_STRU;

/*****************************************************************************
 结构名    : NV_USB_PRIVATE_INFO
 结构说明  : 此NV项用于USBlog控制等
 结构 ID   : 50498
*****************************************************************************/
/*50498*/
#define reserved_num_16  (16)
#define reserved_num_10  (10)
typedef struct 
{
    VOS_UINT32 nv_status;
    VOS_UINT8  debug_mode[reserved_num_16];
    VOS_UINT32 reserve[reserved_num_10];
} HUAWEI_USB_PRIVATE_INFO;


/*各参数默认值80,107,120,200,200,15,0,1,2,9,9,9,0,0,0,0,3,3,0,9000*/
/*THERMAL_ACTION_ITEM_T_UINT8：0:正常；1:告警状态；2:飞行模式;3:降功耗；4:关分集；5:降流速；6:关PA；7:网络模式切换；8:关机状态*/
typedef struct 
{
    VOS_UINT16    Threshold1;                          /*门限1所对应的温度值*/
    VOS_UINT16    Threshold2;                          /*门限2所对应的温度值*/
    VOS_UINT16    Threshold3;                          /*门限3所对应的温度值*/
    VOS_UINT16    Threshold4;                          /*门限4所对应的温度值*/
    VOS_UINT16    Threshold5;                          /*门限5所对应的温度值*/

    VOS_UINT16    ThresholdDelay;                      /*各门限恢复时温度滞后量即迟滞温度*/

    VOS_INT8      Level0;                              /*不超过门限1时所执行的操作，取值范围参考THERMAL_ACTION_ITEM_T----1~9*/
    VOS_INT8      Level1;                              /*超过门限1时所执行的操作*/
    VOS_INT8      Level2;                              /*超过门限2时所执行的操作*/
    VOS_INT8      Level3;                              /*超过门限3时所执行的操作*/
    VOS_INT8      Level4;                              /*超过门限4时所执行的操作*/
    VOS_INT8      Level5;                              /*超过门限5时所执行的操作*/

    VOS_INT8      UnsolicitedLevel;                    /*通过AT主动打印信息的级别*/
    VOS_INT8      LogLevel;                            /*LOG文件保存信息的级别*/
    VOS_INT8      CurrentLogFileNum;                   /*当前正常使用的LOG文件ID，内部使用*/

    VOS_INT8      RxPowerDisableCloseDiversity;        /*当接受功率小于某指定值时，不允许关闭分集*/

    VOS_INT8      TempOverCount;                       /*温度超过门限TempOverCount次后执行相应操作*/
    VOS_INT8      TempResumeCount;                     /*温度低于恢复门限TempResumeCount次后执行相应操作*/

    VOS_INT32     MAXTxPower;                          /*限制PA的最大发射功率*/

    VOS_INT32     ControlPeriod;                       /*温度检查周期unit ms*/
}THERMAL_CONFIG_NV_T;


typedef struct 
{ 
    /* GPS使能标志 */
    VOS_UINT32 gpsEnable;
}NV_HUAWEI_GPS_I;


typedef struct
{
    VOS_INT32 m_NumFixes;
    VOS_INT32 m_TimeBetweenFixes;
    VOS_INT32 m_bAccuracy;
    VOS_INT16 m_bSessionType;
    VOS_INT8  m_bOperationMode;
    VOS_INT8  m_performance;
}GPS_USER_SETTING_STRU;


typedef struct
{
    VOS_INT32 iGpsEnable;
    VOS_INT32 iSetFactorySetting;
    VOS_INT32 iColdStart;
    VOS_INT32 iEventMask;
    VOS_INT32 iNmeaSentenceMask;
    VOS_INT32 iGpsLock;
    VOS_INT32 iOpMode;
    VOS_INT32 iFixSessionType;
    VOS_INT32 iTimeBetweenFixes;
    VOS_INT32 iAccuracyThreshold;
    VOS_INT32 iFixTimeout;
    VOS_INT32 iLocationPrivacy;
    VOS_INT32 iMapServiceID;
    VOS_INT32 iMetricUnit;
    VOS_INT32 iAccuracy;
    VOS_INT32 iPerformance;
    VOS_INT32 iEULA;
    VOS_INT32 iEnableAutoDownload;
    VOS_INT32 iAutoDownloadPeriod;
}GPS_SETTING_STRU;

/*****************************************************************************
 结构名    : CPE_TELNET_SWITCH_NVIM_STRU
 结构说明  : 此NV项用于控制CPE产品telnet功能是否打开。
 结构 ID   : 50501
*****************************************************************************/
typedef struct
{
    VOS_UINT8 nv_telnet_switch;
    VOS_UINT8 reserved;
}CPE_TELNET_SWITCH_NVIM_STRU;
typedef struct
{
    VOS_UINT8 InterfaceName[32];     /* 接口名称 */
    VOS_UINT8  MBIMEnable;            /* 是否启用MBIM,取值范围 [0-1] */
    VOS_UINT8  CdRom;                 /* 启用MBIM后，是否同时上报光盘,取值范围 [0-1] */
    VOS_UINT8  TCard;                 /* 启用MBIM后，是否同时上报T卡,取值范围 [0-1] */
    VOS_UINT8  MaxPDPSession;         /* 多PDP支持的最大PDP数量,取值范围 [1-8] */
    VOS_UINT16 IPV4MTU;               /* IPV4 MTU大小,取值范围 [296-1500] */
    VOS_UINT16 IPV6MTU;               /* IPV6 MTU大小,取值范围 [1280-65535] */
    VOS_UINT32 Reserved1;             /* 保留 */
    VOS_UINT32 Reserved2;             /* 保留 */
}NAS_WINBLUE_PROFILE_TYPE_STRU;


typedef  struct
{
    VOS_UINT8 ucNvActiveFlag;    /*是否激活BIP功能*/
    VOS_UINT8 ucRoamingEnable;  /*漫游时是否允许进行BIP业务*/
    VOS_UINT8 ucReserved[2];    /*对齐保留*/
}NV_BIP_FEATURE_STRU;


typedef struct
{
    VOS_UINT8    nv_lock30[TAF_NVIM_HWLOCK_LEN];
    VOS_UINT8    nv_lock21[TAF_NVIM_HWLOCK_LEN];/*simlock 2.1 未使用*/
    VOS_UINT8    nv_lock20[TAF_NVIM_HWLOCK_LEN];/*simlock 2.0 未使用*/
    VOS_UINT8    reserved[TAF_NVIM_HWLOCK_LEN];
}NV_AUHT_OEMLOCK_STWICH_SRTU;

typedef struct
{
    VOS_UINT8    nv_lock30[TAF_NVIM_HWLOCK_LEN];
    VOS_UINT8    nv_lock21[TAF_NVIM_HWLOCK_LEN];
    VOS_UINT8    nv_lock20[TAF_NVIM_HWLOCK_LEN];
    VOS_UINT8    reserved[TAF_NVIM_HWLOCK_LEN];
}NV_AUHT_SIMLOCK_STWICH_STRU;

typedef  struct
{
    VOS_UINT8 ucSimRefreshIndFlag;    /*SIM卡RESET上报使能*/
    VOS_UINT8 ucReserv;             /* C31之后的基线此位不用*/
    VOS_UINT8 ucReserv1;             /*对齐保留*/
    VOS_UINT8 ucReserv2;            /*对齐保留*/
}NV_SIM_CUST_FEATURE_STRU;


#define MMA_ROMA_WHITELIST_NV_SIZE                   (127)

typedef struct
{
    VOS_UINT8 ucNvActiveFlag;           /*是否激活漫游白名单，1表示激活，0不激活*/
    VOS_UINT8 aucRoamWhiteListData[MMA_ROMA_WHITELIST_NV_SIZE]; /* NV数据 */
} NV_ROAM_WHITELIST_DATA;

/*****************************************************************************
 结构名    : nv_mac_num
 结构说明  :定义每一种类型对应的设备MAC地址个数
 结构 ID   : 50517
*****************************************************************************/
typedef struct
{
    VOS_UINT8 lanmac_num;        /* LAN_MAC/Cradle MAC个数*/
    VOS_UINT8 wifimac_num;       /* WIFI_MAC 个数*/
    VOS_UINT8 btmac_num;        /* BLUETOOTH_MAC个数*/
    VOS_UINT8 usbmac_num;       /* USB_MAC个数*/
    VOS_UINT8 reserve[4];          /*预留字段*/
} NV_MAC_NUM_STRU;


typedef struct
{
    VOS_UINT32 ulIsEnable;                  /*二级降速和软关机恢复功能的使能开关:0,1，依赖于NV 40中的使能开启*/
    VOS_INT32  lLimitSpeedThreshold;        /*二级限速恢复到正常态的温度门限值（modem产品，温度为SIM卡温度；E5产品，温度为电池NTC温度)*/
    VOS_INT32  ulTempOverCount;             /*二级限速的告警门限检测次数*/
    VOS_INT32  ulSoftoffResumeThreshold;    /*软关机恢复到一级限速的温度门限值，超出告警门限的检测次数同海思设置NV 40*/
} USIM_SECOND_TEMP_PROTECT_NV_TYPE;

/*****************************************************************************
 结构名    : SPY_TEMP_PROTECT_SAR_NV_STRU
 结构说明  : 此NV项用于控制温度保护降SAR方案
 结构 ID   : 50520
*****************************************************************************/
typedef struct
{
    VOS_UINT32  ulIsEnable;
    VOS_INT32   lReduceSARThreshold;
    VOS_INT32   lResumeSARThreshold;
    VOS_UINT32  ulTempOverCount;
    VOS_UINT32  ulTempResumeCount;
    VOS_INT32   lReserved_1;
} SPY_TEMP_PROTECT_SAR_NV_STRU;

typedef struct
{
    VOS_UINT8  ucEtype;
    VOS_UINT8  ucVtype;
    VOS_UINT8  ucQtype;
    VOS_UINT8  ucXtype;
    VOS_UINT8  ucAmpCpara;
    VOS_UINT8  ucAmpDpara;
    VOS_UINT8  ucAmpSpara;
    VOS_UINT8  ucS0type;
    VOS_UINT8  ucS7type;
    VOS_UINT8  ucS10type;
    VOS_UINT8  ucIFCpara[2];/*Reserved 2 byte for +IFC*/
    VOS_UINT8  ucICFpara[2];/*Reserved 2 byte for +ICF*/
    VOS_UINT8  ucAligned[2];/*Reserved 2 byte for 4BYTE aligned*/
}ATCMD_AMPW_STORE_STRUCT;


typedef struct
{
    VOS_UINT8  ucEtype;
    VOS_UINT8  ucVtype;
    VOS_UINT8  ucQtype;
    VOS_UINT8  ucXtype;
    VOS_UINT8  ucAmpCpara;
    VOS_UINT8  ucAmpDpara;
    VOS_UINT8  ucAmpSpara;
    VOS_UINT8  ucS0type;
    VOS_UINT8  ucS7type;
    VOS_UINT8  ucS10type;
    VOS_UINT8  ucIFCpara[2];/*Reserved 2 byte for +IFC*/
    VOS_UINT8  ucICFpara[2];/*Reserved 2 byte for +ICF*/
    VOS_UINT8  ucAligned[2];/*Reserved 2 byte for 4BYTE aligned*/
    VOS_UINT32 uiIPRpara;/*Reserved 2 byte for +IPR*/
}ATCMD_AMPW_ADD_IPR_STORE_STRUCT;
/*****************************************************************************
 结构名    : MTA_BODY_SAR_LT_PARA_STRU
 结构说明  : 此NV项用于保存at^bodysarlte的功率设置
 结构 ID   : 50542
*****************************************************************************/
#define MTA_BODY_SAR_LBAND_MAX_NUM                          (20)

typedef struct
{
    VOS_UINT16          enBand;                     /* LTE频段 */
    VOS_INT16                           sPower;                     /* 功率门限值 */
}MTA_BODY_SAR_L_PARA;

typedef struct
{
    VOS_UINT16                          usLBandNum;                                 /* LTE的Band个数 */
    VOS_UINT16                          ausReserved1[1];                            /* 保留位 */
    MTA_BODY_SAR_L_PARA            astLBandPara[MTA_BODY_SAR_LBAND_MAX_NUM];   /* LTE功率门限值 */
}MTA_BODY_SAR_L_PARA_STRU;

/*****************************************************************************
 结构名    : NV_BODYSAR_FLAG_STRU
 结构说明  : 此NV项用于控制整机降SAR方案
 结构 ID   : 50541
*****************************************************************************/
typedef struct
{
    VOS_UINT8 bodysar_flag;
    VOS_UINT8 ucReserved[3];   /*保留字段*/
} NV_BODYSAR_FLAG_STRU;

/*****************************************************************************
 结构名    : remote_wakeup_info_type
 结构说明  : remote_wakeup_info_type结构 ID=50299
*****************************************************************************/
typedef struct remote_wakeup_info_struct/*FEATURE_HUWEI_PM*/
{
    VOS_UINT8 remote_wakeup_ctrl;            /*Configure switch*/
    VOS_UINT8 remote_wakeup_channel;         /*Configure channel*/
    VOS_UINT16 remote_wakeup_source;         /*Configure source*/
}remote_wakeup_info_type;

typedef struct
{
    VOS_UINT32 sleepcfg_para;
    VOS_UINT32 sleepcfg_value;
}sleepcfg_type;

typedef struct 
{
    sleepcfg_type huawei_sleep_cfg[10];
}nv_huawei_sleepcfg_type;
/*****************************************************************************
  结构名    : NV_AUTHFALLBACK_FAATURE_STRU
  结构说明  : NV_AUTHFALLBACK_FAATURE_STRU 结构
*****************************************************************************/
typedef struct
{
    VOS_UINT8       ucNvActiveFlag;    /*是否开启鉴权回退功能*/
    VOS_UINT8       ucAuthType;               /*鉴权类型*/
    VOS_UINT8       ucReserved[30];            /*对齐保留*/
}NV_HUAWEI_AUTHFALLBACK_FEATURE_STRU;

/*****************************************************************************
  结构名    : NV_AUTHFALLBACK_FAATURE_STRU
  结构说明  : NV_AUTHFALLBACK_FAATURE_STRU 结构
*****************************************************************************/
typedef struct
{
    VOS_UINT8       ucNvActiveFlag;          /*是否开启运营商鉴权定制功能*/
    VOS_UINT8       ucNvDocomoActiveFlag;    /*是否开启DOCOMO鉴权定制功能*/
    VOS_UINT8       ucReserved[30];           /*对齐保留*/
}NV_HUAWEI_CUSTOMAUTH_FEATURE_STRU;


#define NV_MAX_FOTA_DOMAIN_NAME_LENGTH                  (128)
#define NV_MAX_FOTA_PPP_APN_LENGTH                            (100)
#define NV_MAX_FOTA_PPP_PASSWORD_LENGTH                 (64)
#define NV_MAX_FOTA_PPP_USER_ID_LENGTH                     (64)


typedef struct { 
  VOS_UINT8       apn[NV_MAX_FOTA_PPP_APN_LENGTH];
}NV_HUAWEI_FOTA_PPP_APN_STRU;


typedef struct { 
  VOS_UINT8        password[NV_MAX_FOTA_PPP_PASSWORD_LENGTH];
}NV_HUAWEI_FOTA_PPP_PASSWORD_STRU;


typedef struct { 
  VOS_UINT8         user_name[NV_MAX_FOTA_PPP_USER_ID_LENGTH];
}NV_HUAWEI_FOTA_PPP_USER_ID_STRU;


typedef struct { 
  VOS_UINT32          auth_type;
}NV_HUAWEI_FOTA_PPP_AUTH_TYPE_STRU;


typedef struct { 
  VOS_UINT8       domain_name[NV_MAX_FOTA_DOMAIN_NAME_LENGTH];
}NV_HUAWEI_FOTA_SERVER_DOMAIN_NAME_STRU;


typedef struct { 
  VOS_UINT32      port;
}NV_HUAWEI_FOTA_SERVER_PORT_STRU;


typedef struct
{
    VOS_UINT8     detect_mode;
    VOS_UINT8     download_mode;
    VOS_UINT8     update_mode;
    VOS_UINT8     en_resume;
    VOS_UINT32   period;
}NV_HUAWEI_FOTA_MODE_TYPE_STRU;


typedef struct 
{
    VOS_UINT32   detect_count;
}NV_HUAWEI_FOTA_DETECT_COUNT_STRU;


typedef struct 
{
    VOS_UINT32   year;
    VOS_UINT32   month;
    VOS_UINT32   day;
    VOS_UINT32   hour;
    VOS_UINT32   minute;
    VOS_UINT32   second;
}NV_HUAWEI_FOTA_NWTIME_STRU;


typedef struct 
{
    VOS_UINT32   timer_len;
}NV_HUAWEI_FOTA_TIMER_STRU;


typedef struct { 
  VOS_UINT32       detect_interval;        /* 合法取值: 0 ～65535 , 单位为分钟*/
}NV_HUAWEI_FOTA_DETECT_INTERVAL_STRU;


typedef struct { 
  VOS_UINT32       fotasms_flag;        /* 合法取值: 0 ～1  */
}NV_HUAWEI_FOTA_SMS_FLAG_STRU;


typedef struct
{
    BSP_U8                           aucSimSwitchFlag;
    BSP_U8                           aucReserved[3];
}NV_Sim_Switch_STRU;

/* 支持可信任号码的最大长度 */
#define NV_TRUST_NUM_SIZE 26
/* 支持可信任号码的最大个数 */
#define TRUSTNUM_MAX       20
typedef struct
{
    VOS_UINT8   index;
    VOS_UINT8   ucReserved;            /*对齐保留*/
    VOS_UINT8   trust_number[NV_TRUST_NUM_SIZE];
}nv_trustnum_type;

/*****************************************************************************
 结构名    : nv_smsnum_type
 结构说明  : 用于存储短信白名单
 结构 ID   : 50360
*****************************************************************************/
typedef struct
{
    nv_trustnum_type trust_sms_num_nv_store[TRUSTNUM_MAX];
}trustnum_record_type;
/*****************************************************************************
 结构名    : ANTENNA_TUNER_NV_TYPE
 结构说明  : 此NV项用于记录各制式的各个频段调谐天线逻辑值
 结构 ID   : 50377/50378/50379/50380/50464
*****************************************************************************/
#define ANT_TUNER_MAX_BAND 64
typedef struct
{
    VOS_UINT8 is_setted;
    VOS_UINT8 ant_tuner_gpio_setting;
} ANTENNA_TUNER_SETTING_INFO;

typedef struct
{
    ANTENNA_TUNER_SETTING_INFO antenna_tuner_setting[ANT_TUNER_MAX_BAND];
} ANTENNA_TUNER_NV_TYPE;
/*****************************************************************************
 结构名    : MULTI_IMAGE_TYPE_STRU
 结构说明  : 用于存储oem ver.s
                            ackage_carrier_type,update_mode,manual_swtich_flag都不使用
 结构 ID   : 50412
*****************************************************************************/
typedef  struct 
{
VOS_UINT8 package_carrier_type;
VOS_UINT8 update_mode;
VOS_UINT8 manual_swtich_flag;
VOS_UINT8 vendor_ver;
VOS_UINT8 vendor_config_file_ver;
VOS_UINT8 reserved[123];
}MULTI_IMAGE_TYPE_STRU;

/*****************************************************************************
 结构名    : MTCSWT_AUTO_MANUL_STATUS_STRU
 结构说明  : 用于存储自动、手动切换状态
 结构 ID   : 50413
*****************************************************************************/
typedef struct
{
    VOS_UINT32  auto_status;     /*自动切换标记*/
    VOS_UINT32 carrier_adapter_switch;     /* 随卡匹配开关标记 */
    VOS_UINT32 plmn;/* 强制锁定的sim卡的plmn */
}MTCSWT_AUTO_MANUL_STATUS_STRU;
/*****************************************************************************
 结构名    : FLIGHT_MODE_STRU
 结构说明  : 用于存储飞行模式软件开关状态
 结构 ID   : 50043
*****************************************************************************/
typedef struct
{
    VOS_UINT32	  huawei_flight_mode;  /*0表示开启飞行模式，关闭射频*/
}FLIGHT_MODE_STRU;

/*****************************************************************************
 结构名    : NV_HUAWEI_PWRCFG_SWITCH
 结构说明  : 最大发射功率配置开关状态，0:关闭1:打开
 结构 ID   : 50490
*****************************************************************************/
typedef struct
{
    VOS_UINT8 pwrcfg_switch;
    VOS_UINT8 reserved [7];
} PWRCFG_SWITCH_NV_TYPE;

/*****************************************************************************
 结构名    : PWRCFG_WCDMA_NV_TYPE
 结构说明  : 用来记录GSM频段的最大发射功率值，reserved[8]保存
             用户AT命令设置的GSM最大发射功率值。
 结构 ID   : 50491
*****************************************************************************/
typedef struct
{
    VOS_UINT16 gsm_power[8];
    VOS_UINT16 reserved[8];
} PWRCFG_GSM_NV_TYPE;

/*****************************************************************************
 结构名    : PWRCFG_WCDMA_NV_TYPE
 结构说明  : 用来记录WCDMA频段的最大发射功率值，wcdma_power[16]保存
             底层WCDMA最大发射功率值，reserved[8]保存用户AT命令设置
             的WCDMA最大发射功率值。
 结构 ID   : 50492
*****************************************************************************/
typedef struct
{
    VOS_UINT16 wcdma_power[16];
    VOS_UINT16 reserved[8];
} PWRCFG_WCDMA_NV_TYPE;

/*****************************************************************************
 结构名    : PWRCFG_LTE_NV_TYPE
 结构说明  : 用来记录LTE频段的最大发射功率值，前面32个成员保存
             底层NV_ID_BODYSAR_L_PARA的值，后面32个成员保存用户AT命令设置
             的LTE最大发射功率值。
 结构 ID   : 50494
*****************************************************************************/
typedef struct
{
    VOS_UINT16 lte_power[64];
} PWRCFG_LTE_NV_TYPE;

/*****************************************************************************
 结构名    : PWRCFG_TDSCDMA_NV_TYPE
 结构说明  : 用来记录TDS频段的最大发射功率值，dscdma_power[6]保存
             底层TDS最大发射功率值，reserved[8]保存用户AT命令设置
             的TDS最大发射功率值。
 结构 ID   : 50495
*****************************************************************************/
typedef struct
{
    VOS_UINT16 tdscdma_power[6];
    VOS_UINT16 reserved[8];
} PWRCFG_TDSCDMA_NV_TYPE;

/*****************************************************************************
 结构名    : NV_TTS_CFG_TYPE
 结构说明  : 用来记录支持的TTS配置情况，开关，支持的语音等；
 结构 ID   : 50465
*****************************************************************************/
typedef struct
{
    VOS_UINT8  tts_flag;
    VOS_UINT8  tts_lang;
    VOS_UINT16 reserved;
} NV_TTS_CFG_TYPE;

#define HUAWEI_CUST_NV_NUM 128
/*****************************************************************************
 结构名    : NV_HUAWEI_CUST_NVID_NV_TYPE
 结构说明  : 用来记录用户NV列表中的NV是否被修改过，总长度128个字节。
             每个字节表示一个用户NV是否被用户设置，最多可支持128个用户NV。
 结构 ID   : 50496
*****************************************************************************/
typedef struct
{
    VOS_UINT8 huawei_cust_nvid_nv[HUAWEI_CUST_NV_NUM];
} NV_HUAWEI_CUST_NVID_NV_TYPE;

/*****************************************************************************
 结构名    : MLOG_SWITCH_NV_STRU
 结构说明  : nv50497 用来控制mobile logger功能是否开启
              0 -- 关闭；1 -- 开启
 结构 ID   : 50497
*****************************************************************************/
typedef struct
{
    BSP_U8     mlog_switch;  /*是否打开mobile logger功能*/
    BSP_U8     reserved[3];  /*扩展字节,用以4字节对齐*/ 
}MLOG_SWITCH_NV_STRU;

/*****************************************************************************
 结构名    : NV_HUAWEI_CUST_NVID_RESTORE_TYPE
 结构说明  : 用来记录用户NV列表中的NV是否需要升级后被恢复，总长度128个字节。
             每个字节表示一个用户NV是否需要恢复，最多可支持128个用户NV。
 结构 ID   : 50540
*****************************************************************************/
typedef struct
{
    VOS_UINT8 huawei_cust_nvid_restore[HUAWEI_CUST_NV_NUM];
} NV_HUAWEI_CUST_NVID_RESTORE_TYPE;

/*****************************************************************************
 结构名    : 1.1	NV_HUAWEI_S3S4_ REMOTE_WAKEUP_ENABLE
 结构说明  : 用来使能主机进入S3S4状态时，remote wakeup是否使能。
当取1时，表示使能，0表示不使能。
 结构 ID   : 50540
*****************************************************************************/
typedef  struct 
{
    VOS_UINT8 remote_wakeup_enable;
    VOS_UINT8 reserve[3];
} nv_huawei_s3s4_remote_wakeup_enable;


typedef struct 
{
    VOS_UINT8  ucSimSwitchEnable;           /*SimSwitch*/
    VOS_UINT8  ucAdaptiveClockingEnable;    /*AdaptiveClocking*/
    VOS_UINT8  ucNetMonitorEnable;          /*NetMonitor*/
    VOS_UINT8  ucNetScanEnable;             /*NetScan*/
    VOS_UINT8  ucJammingDetectionEnable;    /*JammingDetection*/
    VOS_UINT8  ucFreqLockEnable;            /*FreqLock*/
    VOS_UINT8  ucCellLockEnable;            /*CellLock*/
    VOS_UINT8  ucPDPStatusEnable;           /*PDPStatus主动上报*/
    VOS_UINT8  ucCPBREADYEnable;            /*CPBREADY主动上报*/
    VOS_UINT8  ucNWNAMEEnable;              /*NWNAME主动上报*/
    VOS_UINT8  ucMbbFastDormEnable;         /*MbbFastDorm*/
    VOS_UINT8  ucPcmModemLoopEnable;        /*PCM语音回环测试*/
}NV_HUAWEI_MBB_FEATURE_STRU;

/*****************************************************************************
 结构名    : NV_HUAWEI_WEB_ADMIN_STRU
 结构说明  : 用来保存web site password
 结构 ID   : 50418
*****************************************************************************/
#define ADMIN_NUM  16
typedef struct
{
    VOS_UINT8 web_admin[ADMIN_NUM];
} NV_HUAWEI_WEB_ADMIN_STRU;

/*****************************************************************************
 结构名    : NV_AGING_TEST_CTRL_STRU
 结构说明  : 用来保存老化测试参数
 结构 ID   : 
*****************************************************************************/
#define RF_BAND_MAX_NUM  20
#define WIFI_MODE_MAX_NUM  20
typedef struct 
{
    VOS_UINT16 led_test_enable;   /*LED  老化测试开关*/
    VOS_UINT16 on_time;           /*LED长亮时间*/
    VOS_UINT16 off_time;          /*LED长灭时间*/
    VOS_UINT16 reserve;
}AGING_LED_STRU;

typedef struct
{
    VOS_UINT16 mode;        /* wifi  模式+制式*/
    VOS_UINT16 frequency;   /* wifi  频点*/
}AGING_WIFI_CONFIG;

typedef struct 
{
    VOS_UINT16 wifi_test_enable; /*wifi老化测试开关*/
    VOS_UINT16 mode_num;        /*测试模式个数*/
    VOS_UINT16 on_time;         /*发射时间*/
    VOS_UINT16 off_time;        /*休息时间*/
    AGING_WIFI_CONFIG wifi_parametr[WIFI_MODE_MAX_NUM];
}AGING_WIFI_STRU;

typedef struct
{
    VOS_UINT16 mode;        /*RF制式取值1-4: 1--CDMA, 2--GSM, 3--WCDMA, 4--LTE*/
    VOS_UINT16 band;        /*RF频段*/
    VOS_UINT16 frequency;   /*RF 频点*/
    VOS_UINT16 reserve;
}AGING_RF_CONFIG;

typedef struct 
{
    VOS_UINT16 rf_test_enable;   /*lte 老化测试开关*/
    VOS_UINT16 band_num;          /*测试频段个数*/
    VOS_UINT16 on_time;          /*发射时间*/
    VOS_UINT16 off_time;         /*休息时间*/
    AGING_RF_CONFIG rf_parametr[RF_BAND_MAX_NUM];
}AGING_RF_STRU;

typedef struct
{
    VOS_UINT32 aging_test_enable:1;   /*老化测试总开关*/
    VOS_UINT32 charge_test_enable:1;  /*充放电老化测试开关*/
    VOS_UINT32 lcd_test_enable:1;     /*lcd 老化测试开关*/
    VOS_UINT32 flash_test_enable:1;   /*flash 老化测试开关*/
    VOS_UINT32 cpu_test_enable:1;     /*CPU 老化测试开关*/
    VOS_UINT32 reserve:27;            /*预留项*/
    VOS_UINT32 total_time;            /*测试总时间*/
    AGING_LED_STRU  led_parameter;    /*led 老化测试结构*/
    AGING_WIFI_STRU wifi_parameter;   /*wifi 老化测试结构*/
    AGING_RF_STRU  rf_test;           /*lte 老化测试结构*/
}NV_AGING_TEST_CTRL_STRU;
/*****************************************************************************
 结构名    : CPE_TELNET_SWITCH_NVIM_STRU
 结构说明  : 此NV项用于控制CPE产品telnet功能是否打开。
 结构 ID   : 50518
*****************************************************************************/
#define SB_SERIAL_NUM_NV_LEN (20)
typedef struct
{
    VOS_UINT8 serial_num[SB_SERIAL_NUM_NV_LEN];
}SB_SERIAL_NUM_STRU;


/*****************************************************************************
 结构名    :  NV_RF_ANTEN_DETECT_GPIO_STRU
 结构说明  :  射频天线插拔nv定义
              anten[]:记录检测射频天线是否插入的对应GPIO编号。
              切记：Modem 0天线的GPIO号必须配置到0~3元素，Modem 1天线GPIO号必须配置到4~7元素。
              若硬件未预留GPIO号，则配置为0xffff，默认RF天线未插入。
              目前P750s项目只支持Modem 0，不支持Modem 1。
 NV ID    :   50569
*****************************************************************************/
#define MODEM_NUM            (2) /* 支持的卡最多数目: MODEM 0,1 */
#define ANTEN_NUM_PER_MODEM  (4) /* 每个卡支持的最多主集射频天线的数目 */
#define ANTEN_MAX_NUM        (MODEM_NUM * ANTEN_NUM_PER_MODEM) /* modem 0和1支持的最大天线数目 */

typedef struct
{
    VOS_UINT32 enable;               //检测使能，1使能，0去使能
    VOS_UINT32 anten[ANTEN_MAX_NUM]; //检测对应主集上射频线是否在位的GPIO编号
    VOS_UINT32 reserved1;            //预留
} NV_RF_ANTEN_DETECT_GPIO_STRU;
/*****************************************************************************
 结构名    :  NV_SBM_APN_FLAG_STRU
 结构说明  : 软银APN自适配用来标记的FLAG
 NV ID    :   50554
*****************************************************************************/

typedef struct
{
    VOS_UINT8       ucApnFlag;       /*合法取值0或1*/
    VOS_UINT8       aucReserved[3];  /*预留*/
}NV_SBM_APN_FLAG_STRU;


#define MAX_BAND_NUM  (10)   /* 天线的最大band数*/
#define MAX_ANT_NUM   (8)    /* 最大天线数 */

/*每个天线的实际band总数及其主分集标识位*/
typedef struct
{
    VOS_UINT16   band_num   :4;              /*每个天线的实际band总数*/
    VOS_UINT16   band_id0_main_div_flag :1;  /*该天线上第0个band的主分集标识位*/
    VOS_UINT16   band_id1_main_div_flag :1;  /*该天线上第1个band的主分集标识位*/
    VOS_UINT16   band_id2_main_div_flag :1;  /*该天线上第2个band的主分集标识位*/
    VOS_UINT16   band_id3_main_div_flag :1;  /*该天线上第3个band的主分集标识位*/
    VOS_UINT16   band_id4_main_div_flag :1;  /*该天线上第4个band的主分集标识位*/
    VOS_UINT16   band_id5_main_div_flag :1;  /*该天线上第5个band的主分集标识位*/
    VOS_UINT16   band_id6_main_div_flag :1;  /*该天线上第6个band的主分集标识位*/
    VOS_UINT16   band_id7_main_div_flag :1;  /*该天线上第7个band的主分集标识位*/
    VOS_UINT16   band_id8_main_div_flag :1;  /*该天线上第8个band的主分集标识位*/
    VOS_UINT16   band_id9_main_div_flag :1;  /*该天线上第9个band的主分集标识位*/
    VOS_UINT16   reserved   :2;              /*扩展bit位，用以字节对齐*/
}BAND_NUM_MAIN_DIV_STRU;

/*每个天线上的band信息*/
typedef struct
{
    VOS_UINT8               band_idx[MAX_BAND_NUM];  /*每个BAND的band号，MAX_BAND_NUM 为10*/
    BAND_NUM_MAIN_DIV_STRU  band_num_main_div_flag;  /*该组实际BAND个数及其主分集标识位*/
}BAND_INFO_PER_ANT_STRU;

/*实际射频物理天线总数及每个天线的信息*/
typedef struct
{
    VOS_UINT8                ant_num;               /*实际射频物理天线总数*/
    VOS_UINT8                reserved[3];           /*扩展字节，用以4字节对齐*/
    BAND_INFO_PER_ANT_STRU   ant_info[MAX_ANT_NUM]; /*每个天线对应的信息 MAX_ANT_NUM为8*/
}LTE_ANT_INFO_STRU;


typedef struct
{
    VOS_UINT32 nv_status;    /*该nv是否激活*/
    VOS_UINT16 ucActiveFlag;/*是否开启usb tethering功能*/
    VOS_UINT16 ucReserved1; /*保留字段1*/
    VOS_UINT16 ucReserved2; /*保留字段2*/
    VOS_UINT16 ucReserved3; /*保留字段3*/
}HUAWEI_NV_USB_TETHERING;
/*50577*/
typedef struct
{
VOS_UINT8 nv_status;    /*该nv是否激活*/
VOS_UINT8 diag_enable;  /*diag端口是否使能*/
VOS_UINT8 shell_enable; /*shell端口是否使能*/
VOS_UINT8 adb_enable;   /*adb端口是否使能*/
VOS_UINT8 cbt_enable;   /*cbt端口是否使能*/
VOS_UINT8 ucReserved_0; /*保留字段*/
VOS_UINT8 ucReserved_1; /*保留字段*/
VOS_UINT8 ucReserved_2; /*保留字段*/
}HUAWEI_NV_USB_SECURITY_FLAG;


typedef struct
{ 
    VOS_INT16  sHandSetUpVolValue;      /*手持模式上行音量*/
    VOS_INT16  sHandsFreeUpVolValue;    /*免提模式上行音量*/
    VOS_INT16  sCarUpVolValue;          /*车载模式上行音量*/
    VOS_INT16  sHeadSetUpVolValue;      /*耳机模式上行音量*/
    VOS_INT16  sBlueToothUpVolValue;    /*蓝牙模式上行音量*/
    VOS_INT16  sPcVoiceUpVolValue;      /*PC语言模式上行音量*/
    VOS_INT16  sHeadPhoneUpVolValue;    /*不带耳机MIC的耳机通话模式上行音量*/
    VOS_INT16  ucReserved[5];           /*Reserved*/
} APP_VC_NV_CMIC_VOLUME_STRU;

typedef struct
{
     VOS_UINT16  wifi_2g_ant0[3];       /*2.4g的ant0的wifi校准参数*/
     VOS_UINT16  wifi_2g_ant1[3];       /*2.4g的ant1的wifi校准参数*/
}NV_WIFI_2G_RFCAL_STRU;

typedef struct
{
     VOS_UINT16  wifi_5g_ant0[12];
     VOS_UINT16  wifi_5g_ant1[12];
     VOS_UINT16  wifi_5g_ant2[12];
}NV_WIFI_5G_RFCAL_STRU;

typedef struct
{
    VOS_INT32 lEnterPowerOffThreshold;     /*进入下电模式的门限*/
    VOS_INT32 lEnterFlightModeThreshold;  /*进入飞行模式的温度门限*/
    VOS_INT32 lExitFlightModeThreshold;     /*退出飞行模式的温度门限*/
    VOS_INT32 lEnterSpeedLimit2Threshold; /*进入二级限速的温度门限*/
    VOS_INT32 lEnterSpeedLimit1Threshold; /*进入一级限速的温度门限*/
    VOS_INT32 lReturnNormalThreshold;       /*返回正常状态的温度门限*/
    VOS_INT32 lReserve[2];                            /*预留*/
} SPY_TEMP_PROTECT_SECOND_SAMPLE_THRESHOLD_STRU;

typedef struct
{
    VOS_UINT32 ulIsEnable; /*是否需要启动保护机制, 0为关闭，1为开启*/
    VOS_UINT32 ulLocation; /*使用哪里的温度作为温保的输入，该参数直接作为入参进入底软的函数,目前的取值范围是0到6 */
    SPY_TEMP_PROTECT_SECOND_SAMPLE_THRESHOLD_STRU threshold; /*各个温度保护状态温度门限值*/
} SPY_TEMP_PROTECT_SENCOND_SAMPLE_NV_STRU;

//#if (FEATURE_ON == MBB_WPG_CELLLOCK_CPE)
/*****************************************************************************
 结构名    : NV_Item_CELL_LOCK_STATUS_NV_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : NV_Item_CELL_LOCK_STATUS_NV_STRU  50552
*****************************************************************************/
typedef struct
{ 
    VOS_UINT8                               cell_lock_status;   /* 锁小区状态*/
    VOS_UINT8                               cell_max_num;       /* 合法小区个数上限，可由运营商定制*/
    VOS_UINT8                               cell_default_num;       /* 合法小区个数上限默认值*/
    VOS_UINT8                               reserved;                                           
}NV_Item_CELL_LOCK_STATUS_NV_STRU; 


/*****************************************************************************
 结构名    : NV_Item_CELL_LOCK_GSM_CELL_ID_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : en_NV_Item_CELL_LOCK_GSM_CELL_ID_INFO  50553
*****************************************************************************/
typedef struct
{ 
    VOS_UINT8                                 cell_lock_type;   /*是GSM\WCDMA\LTE的锁小区*/                                  
    VOS_UINT8                                 cell_actual_num;    /* 合法小区的实际数目 */
    VOS_UINT8                                 reserved[2]; 
    VOS_UINT8                                 cell_id_list[45][8];  /* 存储单板曾注册过的合法的CELL的CELL ID */                                             
}NV_Item_CELL_LOCK_GSM_CELL_ID_INFO_STRU;

/*****************************************************************************
 结构名    : NV_Item_CELL_LOCK_WCDMA_CELL_ID_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : en_NV_Item_CELL_LOCK_WCDMA_CELL_ID_INFO  50554
*****************************************************************************/
typedef struct
{ 
    VOS_UINT8                                 cell_lock_type;    /*是GSM\WCDMA\LTE的锁小区*/                                      
    VOS_UINT8                                 cell_actual_num;    /* 合法小区的实际数目 */
    VOS_UINT8                                 reserved[2]; 
    VOS_UINT8                                 cell_id_list[45][8];  /* 存储单板曾注册过的合法的CELL的CELL ID */                                             
}NV_Item_CELL_LOCK_WCDMA_CELL_ID_INFO_STRU; 

/*****************************************************************************
 结构名    : NV_Item_CELL_LOCK_LTE_CELL_ID_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : en_NV_Item_CELL_LOCK_LTE_CELL_ID_INFO  50555
*****************************************************************************/
typedef struct
{ 
    VOS_UINT8                                 cell_lock_type;    /*是GSM\WCDMA\LTE的锁小区*/                                    
    VOS_UINT8                                 cell_actual_num;    /* 合法小区的实际数目 */
    VOS_UINT8                                 reserved[2]; 
    VOS_UINT8                                 cell_id_list[45][8];  /* 存储单板曾注册过的合法的CELL的CELL ID */                                             
}NV_Item_CELL_LOCK_LTE_CELL_ID_INFO_STRU; 

/*****************************************************************************
 结构名    : NV_Item_CELL_LOCK_DIAL_TIME_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : en_NV_Item_CELL_LOCK_DIAL_TIME  50556 
*****************************************************************************/
typedef struct
{
    VOS_UINT32                       dial_time_single_max;  //单次拨号时间最大值
    VOS_UINT32                       dial_time_total_max;  //累计拨号时间最大值
    VOS_UINT32                       dial_time_total_actual; //实际拨号总时间
}NV_Item_CELL_LOCK_DIAL_TIME_STRU;

/*****************************************************************************
 结构名    : NV_Item_CELL_LOCK_CALL_NUM_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : en_NV_Item_CELL_LOCK_CALL_NUM 50557 
*****************************************************************************/
typedef struct
{
    VOS_UINT32                           call_num_max; //拨打电话最大次数
    VOS_UINT32                           call_num_actual; //拨打电话实际次数
}NV_Item_CELL_LOCK_CALL_NUM_STRU;

/*****************************************************************************
 结构名    : NV_Item_CELL_LOCK_ALLOW_CALL_NUM_LIST_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : en_NV_Item_CELL_LOCK_ALLOW_CALL_NUM_LIST  50558
*****************************************************************************/
typedef struct
{
    VOS_UINT8              permit_dial_num;    /* 非法小区允许呼出号码的实际个数。0表示除紧急呼叫以外不允许呼出任何号码 */  
    VOS_UINT8              permit_dial_number[5][21];     /* 非法小区允许呼出号码列表  */  
    VOS_UINT8              reserved[2];
}NV_Item_CELL_LOCK_ALLOW_CALL_NUM_LIST_STRU;

//#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


