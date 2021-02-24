

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

/*��ο��ṹ������*/
typedef struct
{
    int reserved;  /*note */
}PRODUCT_MODULE_STRU;

typedef struct
{
    VOS_UINT32 uwGuCrcResult;                      /* GU����NV��У���� */
    VOS_UINT32 uwTlCrcResult;                      /* TL����NV��У���� */
    VOS_UINT32 uwGuM2CrcResult;                    /* GU����NV��У���� */
}NV_CRC_CHECK_RESULT_STRU;

typedef struct
{
    VOS_UINT16 uhwValidCount;                       /* ָʾ��Ч���ݵĸ��� */
    VOS_UINT16 uhwRsv;                              /* ����λ */
    VOS_UINT16 auhwNeedCheckID[MAX_CHECK_NUM];      /* ������Ҫ����NV_IDֵ */
}NV_CHECK_ITEM_STRU;

/*NV50336�ṹ����*/
typedef struct
{
    VOS_UINT8 SKU_TYPE;  /*�Ϸ�ȡֵ 30��34 30��YMobile;34:SBM*/
    VOS_UINT8 RESERVED;  /*Ԥ��*/
}NV_ITEM_SKU_STRU;
/*NV50364��ز������ݽṹ**/
typedef struct
{
    VOS_UINT8 no_battery_powerup_enable;        /*�ǹ���ģʽ�µ�ز���λ����ʹ�ܱ�־*/
    VOS_UINT8 exception_poweroff_poweron_enable;/*�쳣�ػ����´β����Դ���뿪��ģʽʹ�ܱ�־*/
    VOS_UINT8 low_battery_poweroff_disable;     /*�͵�ػ���ֹ��־*/
    VOS_UINT8 reserved;                         /*����*/
}POWERUP_MODE_TYPE;

/*���������¶Ȳ���NV 50385�ṹ����*/
typedef struct
{
    VOS_UINT32    ulChargeIsEnable;                    //����±���ʹ��    
    VOS_INT32     overTempchgStopThreshold;           //�����±�������
    VOS_INT32     subTempChgLimitCurrentThreshold;    //���³���������
    VOS_INT32     lowTempChgStopThreshold;            //�����±�������
    VOS_INT32     overTempChgResumeThreshold;         //�����»ָ��¶�����
    VOS_INT32     lowTempChgResumeThreshold;          //�����»ָ��¶�����
    VOS_UINT32     chgTempProtectCheckTimes;           //���ͣ����ѯ����
    VOS_UINT32     chgTempResumeCheckTimes;            //��縴����ѯ����
    VOS_INT32     exitWarmChgToNormalChgThreshold;    //�ɸ��³��ָ������³���¶�����
    VOS_INT32     reserved2;                          //Ԥ��    
}CHG_SHUTOFF_TEMP_PROTECT_NV_TYPE;

/*���������¶Ȳ���NV50386�ṹ����*/
typedef struct
{
    VOS_INT32         battVoltPowerOnThreshold;           //������ѹ����
    VOS_INT32         battVoltPowerOffThreshold;          //�ػ���ѹ����
    VOS_INT32         battOverVoltProtectThreshold;       //ƽ������ѹ��������(ƽ��ֵ)
    VOS_INT32         battOverVoltProtectOneThreshold;    //���γ���ѹ��������(����ֵ)
    VOS_INT32         battChgTempMaintThreshold;          //���ָ���ͣ�������ͣ����ж�����
    VOS_INT32         battChgRechargeThreshold;           //�����θ�������
    VOS_INT32         VbatLevelLow_MAX;                   //�͵���������
    VOS_INT32         VbatLevel0_MAX;                     //0���ѹ��������
    VOS_INT32         VbatLevel1_MAX;                     //1���ѹ��������
    VOS_INT32         VbatLevel2_MAX;                     //2���ѹ��������
    VOS_INT32         VbatLevel3_MAX;                     //3���ѹ�������� 
    VOS_INT32         reserved1;                          //Ԥ��
    VOS_INT32         reserved2;                          //Ԥ��
}CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE;

#define  MMC_MAX_LEN       (5)
typedef struct
{
    VOS_UINT32                   aulRoamMCCList[MMC_MAX_LEN];/* �洢�����εĹ����� */
}NAS_MMC_NVIM_ROAM_MCC_CMP_STRU;


typedef struct
{
    VOS_UINT8                           ucRoamMncExactlyCmpFlg;
    VOS_UINT8                           aucRsv[3];                         /* ����*/
} NAS_MMC_NVIM_ROAM_MNC_EXACTLY_CMP_STRU;

typedef struct 
{       
    VOS_UINT8     FileSysActiveProtectEnable;   /* �ļ�ϵͳ��������ʹ�� */     
    VOS_UINT8     BadImgResumFromOnlineEnable;   /* �����𻵺��Online�����ָ�ʹ��*/     
    VOS_UINT8     BootNotOperaFileSysEnable;      /*ϵͳ�������̲����ļ�ϵͳ����ʹ��*/     
    VOS_UINT8     FastOffFailTrueShutdownEnable;    /*�ٹػ�ʧ�ܽ�����ػ�����ʹ��*/      
    VOS_UINT8     SoftPowOffReliableEnable;          /*����ػ��ɿ��Թ���ʹ��*/      
    VOS_UINT8     ZoneWriteProtectEnable;          /*����дԽ�籣��ʹ��*/       
    VOS_UINT8     BadZoneReWriteEnable;            /* Flash��Σ���д����ʹ��*/       
    VOS_UINT8     BootCanFromkernelBEnable;  /*�������ƻ�����Դӱ��ݾ�������ʹ��*/
    VOS_UINT8     OnlineChangeNotUpgradEnable; /*Online�����仯��������������ʹ��*/
    /*Ԥ��*/
    VOS_UINT8     BadZoneScanEnable;   /* Flash��Σ��ɨ�蹦��ʹ��*/
    VOS_UINT8     reserved2;
    VOS_UINT8     reserved3;
    VOS_UINT8     reserved4;
    VOS_UINT8     reserved5;
    VOS_UINT8     reserved6;
    VOS_UINT8     MmcReliabilityEnable;            /*Э��MMC�ɿ��Ա���ʹ�ܿ���*/
    VOS_UINT32    MmcReliabilityBitmap;     /* ͨ��Э��Э��MMC�ɿ��Ա����������� */
    VOS_UINT32    DangerTime;                   /*Ƶ�����µ�Σ���ھ���ֵ ����λΪ��*/       
    VOS_UINT32    WaitTime;           /*Ӧ��֪ͨ������ٹػ��󣬶�ʱ��ֵ��λΪ��*/
    /*Ԥ��*/
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
    VOS_UINT32                          aulCustommMccList[CUSTOM_MCC_LIST_NUM_MAX];  /* �������εĹ������б� */    
}NAS_MMC_NVIM_HPLMN_SEARCH_REGARDLESS_MCC_SUPPORT_STRU_EX;


#define DISABLE_RPLMN_ACT_ON       (0)
#define DISABLE_RPLMN_ACT_OFF      (1)
typedef struct
{
    VOS_UINT8                       ucEnableFlag;/*0: Disable RPLMN ACT ���Թر�,1: Disable RPLMN���Դ�*/
    VOS_UINT8                       aucReserve[3];
}NAS_MCC_NVIM_RPLMN_DISABLE_ACT_CFG_STRU;

/*****************************************************************************
 �ṹ��    : GID1_TYPE_STRU
 �ṹ˵��  : ���ڶ���GID1��������
 �ṹ ID   : 50083
*****************************************************************************/
typedef struct
{
    VOS_UINT8   bGID1Enable;                 /*GID1����ʹ��λ*/
    VOS_UINT8   ucLeagalValue;               /*�Ϸ�GID1ֵ*/
}GID1_TYPE_STRU;

/*****************************************************************************
 �ṹ��    : NAS_NV_Vodafone_CPBS
 �ṹ˵��  : �¹� Vodafone CPBS����
 �ṹ ID   : 50429
*****************************************************************************/
typedef struct
{
   VOS_UINT16 Vodafone_CPBS;                 /*����ʹ��λ*/
   VOS_UINT16 Reserved;
}NAS_NV_Vodafone_CPBS;

/*****************************************************************************
 �ṹ��    : TAF_NV_CSIM_CUSTOMIZED
 �ṹ˵��  : AT+CSIM����
 �ṹ ID   : 50189
*****************************************************************************/
typedef struct 
{
     VOS_UINT8  ucNvActiveFlag;     /* NV�����־�ֶ�*/
     VOS_UINT8  ucReserved[3];         /* �����ֶ�*/
} TAF_NV_CSIM_CUSTOMIZED;


typedef struct
{
   VOS_UINT8    ucNvActiveFlag;
   VOS_UINT8    ucReserved[3];            /*�����ֶ�*/
   VOS_UINT32   ulBandGroup1Low32Bit;
   VOS_UINT32   ulBandGroup1High32Bit;
   VOS_UINT32   ulBandGroup2Low32Bit;
   VOS_UINT32   ulBandGroup2High32Bit;
   VOS_UINT32   ulReserved[4];            /*�����ֶΣ����ں�����չ*/
}NAS_NV_CUSTOMIZED_BAND_GROUP;


typedef struct
{
    VOS_UINT8 ucNvimActiveFlg;
    VOS_UINT8 aucReserved[3];                        /*�����ֶ�*/
}NV_ITEM_START_MANUAL_TO_AUTO_ST;


#define MMA_SPECIAL_HPLMN_ITEM_SIZE     (4)      /* ����PLMN�ռ��С*/
#define MMA_SPECIAL_HPLMN_LIST_SIZE     (30)     /* �����Դ洢30��PLMN*/

typedef struct
{
     VOS_UINT8            aucHPlmnId[MMA_SPECIAL_HPLMN_ITEM_SIZE];    /*4�ֽ������ʾһ��HPLMN*/
}MMA_SPEC_HPLMN_TYPE;

typedef struct
{
     VOS_UINT32           ulHPlmnCount;                               /*HPLMN����*/
     MMA_SPEC_HPLMN_TYPE  astHPlmnGroup[MMA_SPECIAL_HPLMN_LIST_SIZE]; /*HPLMN�б�*/
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
 �ṹ��    : NV_HUAWEI_DYNAMIC_VID_STRU
 �ṹ˵��  : ���ڶ��ƶ�̬VID
 �ṹ ID   : 50394
*****************************************************************************/
typedef struct
{
    VOS_UINT16 nv_status;
    VOS_UINT16 huawei_dynamic_vid;
    
}NV_HUAWEI_DYNAMIC_VID_STRU;
/*****************************************************************************
 �ṹ��    : NV_HUAWEI_DYNAMIC_BOOT_PID_STRU
 �ṹ˵��  : ���ڶ��ƶ�̬����ģʽPID
 �ṹ ID   : 50395
*****************************************************************************/
typedef struct
{
    VOS_UINT16 nv_status;
    VOS_UINT16 huawei_dynamic_boot_pid;
    
}NV_HUAWEI_DYNAMIC_BOOT_PID_STRU;
/*****************************************************************************
 �ṹ��    : NV_HUAWEI_DYNAMIC_NORMAL_PID_STRU
 �ṹ˵��  : ���ڶ��ƶ�̬����ģʽPID
 �ṹ ID   : 50396
*****************************************************************************/
typedef struct
{
    VOS_UINT16 nv_status;
    VOS_UINT16 huawei_dynamic_normal_pid;
    
}NV_HUAWEI_DYNAMIC_NORMAL_PID_STRU;
/*****************************************************************************
 �ṹ��    : NV_HUAWEI_DYNAMIC_BOOT_PID_STRU
 �ṹ˵��  : ���ڶ��ƶ�̬����ģʽPID
 �ṹ ID   : 50397
*****************************************************************************/
typedef struct
{
    VOS_UINT16 nv_status;
    VOS_UINT16 huawei_dynamic_debug_pid;
    
}NV_HUAWEI_DYNAMIC_DEBUG_PID_STRU;

typedef struct
{
     VOS_UINT8  ucNvActiveFlag;     /* NV�����־�ֶ�*/
     VOS_UINT8  ucReserved[3];         /* �����ֶ�*/
}HPLMN_Within_EPLMN_NotRoam;

/*****************************************************************************
 �ṹ��    : SI_PIH_IMSI_TIMER_CONFIG
 �ṹ˵��  : MMA����IMSI��ȡ���ڵ�NV�ṹ
 �ṹ ID    : 50426
*****************************************************************************/
typedef struct
{
    VOS_UINT8  ucNvActiveFlag;     /* NV�����־�ֶ�*/
    VOS_UINT8  ucReserved;         /* �����ֶ�*/
    VOS_UINT16  usTimerLen;        /* ��ȡIMSI����(��λΪ��)*/
}SI_PIH_IMSI_TIMER_CONFIG;
/*****************************************************************************
 �ṹ��    : TIM_CPIN_STRU
 �ṹ˵��  : AT+CPIN?����
 �ṹ ID   : 50450
*****************************************************************************/
typedef struct 
{
     VOS_UINT8  ucNvActiveFlag;     /* NV�����־�ֶ�*/
     VOS_UINT8  ucReserved[3];         /* �����ֶ�*/
} TIM_CPIN_STRU;

typedef struct
{
    VOS_UINT8   led_enable;        /* LED��ʹ�ܿ��� */
    VOS_UINT8   led_dr;            /* LED����ʹ�õ�DR */
    VOS_UINT8   led_mode;          /* LED��ģʽ��־ */
    VOS_UINT8   led_reserve;       /* �����ֶ� */
    VOS_UINT32  full_on;           /* LED�ƺ���ģʽ�ȶ����ĳ���ʱ�� */
    VOS_UINT32  full_off;          /* LED�ƺ���ģʽ�ȶ����ĳ���ʱ�� */
    VOS_UINT32  fade_on;           /* LED�ƺ���ģʽ�Ӱ������ĳ���ʱ�� */
    VOS_UINT32  fade_off;          /* LED�ƺ���ģʽ���������ĳ���ʱ�� */
    VOS_UINT32  delay_on;          /* LED����˸ģʽ����ʱ�� */
    VOS_UINT32  delay_period;     /* LED����˸ģʽ����˸����ʱ��*/
    VOS_UINT32  full_long_on;     /* LED�Ƴ����ĳ���ʱ�� */
    VOS_UINT32  full_long_off;    /* LED�Ƴ����ĳ���ʱ�� */
    VOS_UINT32  brightness;       /* LED�����ȵ���ֵ */
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
 �ṹ��    : LED_CONTROL_NV_STRU
 �ṹ˵��  : LED_CONTROL_NV�ṹ
*****************************************************************************/
typedef struct
{
    BSP_U8   ucLedColor;      /*��ɫ����ɫ����ӦLED_COLOR��ֵ*/
    BSP_U8   ucTimeLength;    /*�����ó�����ʱ�䳤�ȣ���λ100ms*/
}LED_CONTROL_NV_STRU;


typedef struct
{
    LED_CONTROL_NV_STRU   stLED[4];                                                 /* һ�����������ڣ�����������˸ */
}M2M_LED_CONTROL_STRU;


typedef struct
{
    BSP_U8                              ucLedMode;                                  /* ����ģʽ */
    BSP_U8                              unresolved[3];                              /* ���� */
    M2M_LED_CONTROL_STRU                stLedStr[NV_GU_LED_SERVICE_STATE_NUM];      /* �������� */
}NV_LED_SET_PARA_STRU;


typedef struct
{
    M2M_LED_CONTROL_STRU                stLedStr[NV_LTE_LED_SERVICE_STATE_NUM];     /* �������� */
}NV_LED_SET_PARA_STRU_EXPAND;


typedef struct
{
    BSP_U8                              ucLedMode;                                  /* ����ģʽ */
    BSP_U8                              unresolved[3];                              /* ���� */
    M2M_LED_CONTROL_STRU                stLedStr[15];         /* �������� */
}NV_LED_SET_PARA_STRU_COMBINED;

/*****************************************************************************
 �ṹ��    : SMS_AUTO_REG_STRU
 �ṹ˵��  : ������ע�ᶨ��
 �ṹ ID   : 50449
*****************************************************************************/
#define MAX_IMSI_LEN 15
typedef struct 
{
     VOS_UINT8  ucNvActiveFlag;                               /*NV�����־�ֶ�*/
     VOS_UINT8  ucSmsRegFlag;                                 /*ע���־λ*/
     VOS_UINT8  ucSmsRegImsi[MAX_IMSI_LEN];               /*��ע��IMSI��*/
     VOS_UINT8  ucReserved[3];                                /*�����ֶ�*/
} SMS_AUTO_REG_STRU;

/*****************************************************************************
 �ṹ��    : TATA_DEVICELOCK_STRU
 �ṹ˵��  : �豸��NV�Ľṹ
 �ṹ ID   : 50432
*****************************************************************************/
#define MAX_DLCK_ENCODE_LEN_MBB 32
typedef struct
{
    VOS_UINT32 ulStatus;                           /*�豸������״̬*/
    VOS_UINT8   aucLockCode[MAX_DLCK_ENCODE_LEN_MBB];    /*�豸��������*/
}TATA_DEVICELOCK_STRU;

/*****************************************************************************
 �ṹ��    : NAS_NV_HPLMN_FIRST_UMTS_TO_LTE
 �ṹ˵��  : YOTA���ƣ���3G��LTEʱ��������EHPLMN
 �ṹ ID   : 50432
*****************************************************************************/
typedef struct
{
    VOS_UINT8 ucNvActiveFlag; /* �Ƿ񼤻� YOTA ���ƹ��� */
    VOS_UINT8 aucReserved[3]; /* ���� */
}NAS_NV_HPLMN_FIRST_UMTS_TO_LTE;


typedef struct
{
    VOS_UINT8    ucNvActiveFlag;   /* �Ƿ񼤻� EE ��Ӫ����ʾ���ƹ��� */
    VOS_UINT8    aucReserved[3];   /* ���� */
}NAS_NV_EE_OPERATOR_NAME_DISPLAY;


#define MODE_LIST_MAX_LEN    (6)
#define MODE_LIST_MAX_NUM    (10)

typedef struct
{
    VOS_UINT8    ucListItemNum;
    VOS_UINT8    ucRestrict;
    VOS_UINT8    aucModeList[MODE_LIST_MAX_NUM][MODE_LIST_MAX_LEN];
}NAS_NV_SYSCFGEX_MODE_LIST;


/* ��APN���� */
typedef struct
{
    VOS_UINT8   ucStatus;       /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8   ucApnLength;
    VOS_UINT8   aucApn[TAF_NVIM_MAX_APN_LOCK_STR_LEN];       /* APN�Ӹ�����ĵ�һ���ֽڿ�ʼд�룬������'\0'��Ϊ��β */
    VOS_UINT8   aucRsv[2];
}APS_APN_LOCK_STRU;

/*****************************************************************************
 �ṹ��    : NV_WINBLUE_PROFILE_STRU
 �ṹ˵��  : Windows8.1 ���Կ���NV
 �ṹ ID   : 50424
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
 �ṹ��    : NV_USB_CDC_NET_SPEED_STRU
 �ṹ˵��  : ��ȡ�����ٶ�Ĭ��ֵ��NV
 �ṹ ID   : 50456
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
 �ṹ��    : NV_DEVNUMW_TIME_TYPE
 �ṹ˵��  : AT^DEVNUMW���ƿͻ��޸�imei�Ĵ���
 �ṹ ID   : 50431
*****************************************************************************/
typedef struct
{
    VOS_UINT32                 times;      /*�ͻ��ɸ���IMEI�Ĵ�����1��һ�Σ�0�����ɸ��� */
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
 �ṹ��    : ss_coul_nv_info
 �ṹ˵��  : ���ؼ�NV
 �ṹ ID   : 50462
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
 �ṹ��    : NV_PLATFORM_CATEGORY_SET_STRU
 �ṹ˵��  : ���õ���CAT�ȼ�
 �ṹ ID   : 50458
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
 ��������  : WLAN_AT_WIINFO_CHANNELS_NVIM_STRU
 ��������  : ��ѯWiFi���ŵ��͹��ʽṹ�� 
 �ṹ ID   :      50468��50469
*****************************************************************************/
#define WLAN_AT_WIINFO_CHN_BUF_SIZE (128) /* channelsռ��NV�����С */
#define WLAN_AT_WIINFO_POW_BUF_SIZE (64)  /* powerռ��NV�����С */

/* �µ�AT^WiInfo CHANNELS��NV�ṹ��ֻ�޶�BUF���ޣ��������ݸ��ݲ�Ʒ���� */
typedef struct 
{
    VOS_UINT8 buf[WLAN_AT_WIINFO_CHN_BUF_SIZE];
}WLAN_AT_WIINFO_CHN_BUF_NVIM_STRU;
/* �µ�AT^WiInfo POWER��NV�ṹ��ֻ�޶�BUF���ޣ��������ݸ��ݲ�Ʒ���� */
typedef struct 
{
    VOS_UINT8 buf[WLAN_AT_WIINFO_POW_BUF_SIZE];
}WLAN_AT_WIINFO_POW_BUF_NVIM_STRU;


typedef struct
{
    VOS_UINT32      ulDSLastLinkTime;                       /*DS���һ������ʱ��*/
    VOS_UINT32      ulDSTotalSendFluxLow;                   /*DS�ۼƷ����������ĸ��ֽ�*/
    VOS_UINT32      ulDSTotalSendFluxHigh;                  /*DS�ۼƷ����������ĸ��ֽ�*/
    VOS_UINT32      ulDSTotalLinkTime;                      /*DS�ۼ�����ʱ��*/
    VOS_UINT32      ulDSTotalReceiveFluxLow;                /*DS�ۼƽ����������ĸ��ֽ�*/
    VOS_UINT32      ulDSTotalReceiveFluxHigh;               /*DS�ۼƽ����������ĸ��ֽ�*/

} TAF_APS_DSFLOW_NV_STRU_EX;

typedef struct
{
    TAF_APS_DSFLOW_NV_STRU_EX    DsflowNvWan[5]; /*����Ŀǰ��ʹ���������NV�ṹ��֧����ౣ��5��WAN����*/
} TAF_APS_DSFLOW_NV_STRU_EXT;

/*****************************************************************************
 �ṹ��    : NV_HUAWEI_DYNAMIC_NAME
 �ṹ˵��  : ������Ӫ�̶��Ƶ���PC����ʾCDROM��SD �����ַ���
 �ṹ ID   : 50108
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
 �ṹ��    : NV_HUAWEI_DYNAMIC_INFO_NAME
 �ṹ˵��  : ��NV��������Ӫ�̶��Ƶ���PC����ʾproduct manufacturer��configuration���ַ�����
 �ṹ ID   : 50109
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
 �ṹ��    : NV_USB_PRIVATE_INFO
 �ṹ˵��  : ��NV������USBlog���Ƶ�
 �ṹ ID   : 50498
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


/*������Ĭ��ֵ80,107,120,200,200,15,0,1,2,9,9,9,0,0,0,0,3,3,0,9000*/
/*THERMAL_ACTION_ITEM_T_UINT8��0:������1:�澯״̬��2:����ģʽ;3:�����ģ�4:�طּ���5:�����٣�6:��PA��7:����ģʽ�л���8:�ػ�״̬*/
typedef struct 
{
    VOS_UINT16    Threshold1;                          /*����1����Ӧ���¶�ֵ*/
    VOS_UINT16    Threshold2;                          /*����2����Ӧ���¶�ֵ*/
    VOS_UINT16    Threshold3;                          /*����3����Ӧ���¶�ֵ*/
    VOS_UINT16    Threshold4;                          /*����4����Ӧ���¶�ֵ*/
    VOS_UINT16    Threshold5;                          /*����5����Ӧ���¶�ֵ*/

    VOS_UINT16    ThresholdDelay;                      /*�����޻ָ�ʱ�¶��ͺ����������¶�*/

    VOS_INT8      Level0;                              /*����������1ʱ��ִ�еĲ�����ȡֵ��Χ�ο�THERMAL_ACTION_ITEM_T----1~9*/
    VOS_INT8      Level1;                              /*��������1ʱ��ִ�еĲ���*/
    VOS_INT8      Level2;                              /*��������2ʱ��ִ�еĲ���*/
    VOS_INT8      Level3;                              /*��������3ʱ��ִ�еĲ���*/
    VOS_INT8      Level4;                              /*��������4ʱ��ִ�еĲ���*/
    VOS_INT8      Level5;                              /*��������5ʱ��ִ�еĲ���*/

    VOS_INT8      UnsolicitedLevel;                    /*ͨ��AT������ӡ��Ϣ�ļ���*/
    VOS_INT8      LogLevel;                            /*LOG�ļ�������Ϣ�ļ���*/
    VOS_INT8      CurrentLogFileNum;                   /*��ǰ����ʹ�õ�LOG�ļ�ID���ڲ�ʹ��*/

    VOS_INT8      RxPowerDisableCloseDiversity;        /*�����ܹ���С��ĳָ��ֵʱ��������رշּ�*/

    VOS_INT8      TempOverCount;                       /*�¶ȳ�������TempOverCount�κ�ִ����Ӧ����*/
    VOS_INT8      TempResumeCount;                     /*�¶ȵ��ڻָ�����TempResumeCount�κ�ִ����Ӧ����*/

    VOS_INT32     MAXTxPower;                          /*����PA������书��*/

    VOS_INT32     ControlPeriod;                       /*�¶ȼ������unit ms*/
}THERMAL_CONFIG_NV_T;


typedef struct 
{ 
    /* GPSʹ�ܱ�־ */
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
 �ṹ��    : CPE_TELNET_SWITCH_NVIM_STRU
 �ṹ˵��  : ��NV�����ڿ���CPE��Ʒtelnet�����Ƿ�򿪡�
 �ṹ ID   : 50501
*****************************************************************************/
typedef struct
{
    VOS_UINT8 nv_telnet_switch;
    VOS_UINT8 reserved;
}CPE_TELNET_SWITCH_NVIM_STRU;
typedef struct
{
    VOS_UINT8 InterfaceName[32];     /* �ӿ����� */
    VOS_UINT8  MBIMEnable;            /* �Ƿ�����MBIM,ȡֵ��Χ [0-1] */
    VOS_UINT8  CdRom;                 /* ����MBIM���Ƿ�ͬʱ�ϱ�����,ȡֵ��Χ [0-1] */
    VOS_UINT8  TCard;                 /* ����MBIM���Ƿ�ͬʱ�ϱ�T��,ȡֵ��Χ [0-1] */
    VOS_UINT8  MaxPDPSession;         /* ��PDP֧�ֵ����PDP����,ȡֵ��Χ [1-8] */
    VOS_UINT16 IPV4MTU;               /* IPV4 MTU��С,ȡֵ��Χ [296-1500] */
    VOS_UINT16 IPV6MTU;               /* IPV6 MTU��С,ȡֵ��Χ [1280-65535] */
    VOS_UINT32 Reserved1;             /* ���� */
    VOS_UINT32 Reserved2;             /* ���� */
}NAS_WINBLUE_PROFILE_TYPE_STRU;


typedef  struct
{
    VOS_UINT8 ucNvActiveFlag;    /*�Ƿ񼤻�BIP����*/
    VOS_UINT8 ucRoamingEnable;  /*����ʱ�Ƿ��������BIPҵ��*/
    VOS_UINT8 ucReserved[2];    /*���뱣��*/
}NV_BIP_FEATURE_STRU;


typedef struct
{
    VOS_UINT8    nv_lock30[TAF_NVIM_HWLOCK_LEN];
    VOS_UINT8    nv_lock21[TAF_NVIM_HWLOCK_LEN];/*simlock 2.1 δʹ��*/
    VOS_UINT8    nv_lock20[TAF_NVIM_HWLOCK_LEN];/*simlock 2.0 δʹ��*/
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
    VOS_UINT8 ucSimRefreshIndFlag;    /*SIM��RESET�ϱ�ʹ��*/
    VOS_UINT8 ucReserv;             /* C31֮��Ļ��ߴ�λ����*/
    VOS_UINT8 ucReserv1;             /*���뱣��*/
    VOS_UINT8 ucReserv2;            /*���뱣��*/
}NV_SIM_CUST_FEATURE_STRU;


#define MMA_ROMA_WHITELIST_NV_SIZE                   (127)

typedef struct
{
    VOS_UINT8 ucNvActiveFlag;           /*�Ƿ񼤻����ΰ�������1��ʾ���0������*/
    VOS_UINT8 aucRoamWhiteListData[MMA_ROMA_WHITELIST_NV_SIZE]; /* NV���� */
} NV_ROAM_WHITELIST_DATA;

/*****************************************************************************
 �ṹ��    : nv_mac_num
 �ṹ˵��  :����ÿһ�����Ͷ�Ӧ���豸MAC��ַ����
 �ṹ ID   : 50517
*****************************************************************************/
typedef struct
{
    VOS_UINT8 lanmac_num;        /* LAN_MAC/Cradle MAC����*/
    VOS_UINT8 wifimac_num;       /* WIFI_MAC ����*/
    VOS_UINT8 btmac_num;        /* BLUETOOTH_MAC����*/
    VOS_UINT8 usbmac_num;       /* USB_MAC����*/
    VOS_UINT8 reserve[4];          /*Ԥ���ֶ�*/
} NV_MAC_NUM_STRU;


typedef struct
{
    VOS_UINT32 ulIsEnable;                  /*�������ٺ���ػ��ָ����ܵ�ʹ�ܿ���:0,1��������NV 40�е�ʹ�ܿ���*/
    VOS_INT32  lLimitSpeedThreshold;        /*�������ٻָ�������̬���¶�����ֵ��modem��Ʒ���¶�ΪSIM���¶ȣ�E5��Ʒ���¶�Ϊ���NTC�¶�)*/
    VOS_INT32  ulTempOverCount;             /*�������ٵĸ澯���޼�����*/
    VOS_INT32  ulSoftoffResumeThreshold;    /*��ػ��ָ���һ�����ٵ��¶�����ֵ�������澯���޵ļ�����ͬ��˼����NV 40*/
} USIM_SECOND_TEMP_PROTECT_NV_TYPE;

/*****************************************************************************
 �ṹ��    : SPY_TEMP_PROTECT_SAR_NV_STRU
 �ṹ˵��  : ��NV�����ڿ����¶ȱ�����SAR����
 �ṹ ID   : 50520
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
 �ṹ��    : MTA_BODY_SAR_LT_PARA_STRU
 �ṹ˵��  : ��NV�����ڱ���at^bodysarlte�Ĺ�������
 �ṹ ID   : 50542
*****************************************************************************/
#define MTA_BODY_SAR_LBAND_MAX_NUM                          (20)

typedef struct
{
    VOS_UINT16          enBand;                     /* LTEƵ�� */
    VOS_INT16                           sPower;                     /* ��������ֵ */
}MTA_BODY_SAR_L_PARA;

typedef struct
{
    VOS_UINT16                          usLBandNum;                                 /* LTE��Band���� */
    VOS_UINT16                          ausReserved1[1];                            /* ����λ */
    MTA_BODY_SAR_L_PARA            astLBandPara[MTA_BODY_SAR_LBAND_MAX_NUM];   /* LTE��������ֵ */
}MTA_BODY_SAR_L_PARA_STRU;

/*****************************************************************************
 �ṹ��    : NV_BODYSAR_FLAG_STRU
 �ṹ˵��  : ��NV�����ڿ���������SAR����
 �ṹ ID   : 50541
*****************************************************************************/
typedef struct
{
    VOS_UINT8 bodysar_flag;
    VOS_UINT8 ucReserved[3];   /*�����ֶ�*/
} NV_BODYSAR_FLAG_STRU;

/*****************************************************************************
 �ṹ��    : remote_wakeup_info_type
 �ṹ˵��  : remote_wakeup_info_type�ṹ ID=50299
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
  �ṹ��    : NV_AUTHFALLBACK_FAATURE_STRU
  �ṹ˵��  : NV_AUTHFALLBACK_FAATURE_STRU �ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT8       ucNvActiveFlag;    /*�Ƿ�����Ȩ���˹���*/
    VOS_UINT8       ucAuthType;               /*��Ȩ����*/
    VOS_UINT8       ucReserved[30];            /*���뱣��*/
}NV_HUAWEI_AUTHFALLBACK_FEATURE_STRU;

/*****************************************************************************
  �ṹ��    : NV_AUTHFALLBACK_FAATURE_STRU
  �ṹ˵��  : NV_AUTHFALLBACK_FAATURE_STRU �ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT8       ucNvActiveFlag;          /*�Ƿ�����Ӫ�̼�Ȩ���ƹ���*/
    VOS_UINT8       ucNvDocomoActiveFlag;    /*�Ƿ���DOCOMO��Ȩ���ƹ���*/
    VOS_UINT8       ucReserved[30];           /*���뱣��*/
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
  VOS_UINT32       detect_interval;        /* �Ϸ�ȡֵ: 0 ��65535 , ��λΪ����*/
}NV_HUAWEI_FOTA_DETECT_INTERVAL_STRU;


typedef struct { 
  VOS_UINT32       fotasms_flag;        /* �Ϸ�ȡֵ: 0 ��1  */
}NV_HUAWEI_FOTA_SMS_FLAG_STRU;


typedef struct
{
    BSP_U8                           aucSimSwitchFlag;
    BSP_U8                           aucReserved[3];
}NV_Sim_Switch_STRU;

/* ֧�ֿ����κ������󳤶� */
#define NV_TRUST_NUM_SIZE 26
/* ֧�ֿ����κ���������� */
#define TRUSTNUM_MAX       20
typedef struct
{
    VOS_UINT8   index;
    VOS_UINT8   ucReserved;            /*���뱣��*/
    VOS_UINT8   trust_number[NV_TRUST_NUM_SIZE];
}nv_trustnum_type;

/*****************************************************************************
 �ṹ��    : nv_smsnum_type
 �ṹ˵��  : ���ڴ洢���Ű�����
 �ṹ ID   : 50360
*****************************************************************************/
typedef struct
{
    nv_trustnum_type trust_sms_num_nv_store[TRUSTNUM_MAX];
}trustnum_record_type;
/*****************************************************************************
 �ṹ��    : ANTENNA_TUNER_NV_TYPE
 �ṹ˵��  : ��NV�����ڼ�¼����ʽ�ĸ���Ƶ�ε�г�����߼�ֵ
 �ṹ ID   : 50377/50378/50379/50380/50464
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
 �ṹ��    : MULTI_IMAGE_TYPE_STRU
 �ṹ˵��  : ���ڴ洢oem ver.s
                            ackage_carrier_type,update_mode,manual_swtich_flag����ʹ��
 �ṹ ID   : 50412
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
 �ṹ��    : MTCSWT_AUTO_MANUL_STATUS_STRU
 �ṹ˵��  : ���ڴ洢�Զ����ֶ��л�״̬
 �ṹ ID   : 50413
*****************************************************************************/
typedef struct
{
    VOS_UINT32  auto_status;     /*�Զ��л����*/
    VOS_UINT32 carrier_adapter_switch;     /* �濨ƥ�俪�ر�� */
    VOS_UINT32 plmn;/* ǿ��������sim����plmn */
}MTCSWT_AUTO_MANUL_STATUS_STRU;
/*****************************************************************************
 �ṹ��    : FLIGHT_MODE_STRU
 �ṹ˵��  : ���ڴ洢����ģʽ�������״̬
 �ṹ ID   : 50043
*****************************************************************************/
typedef struct
{
    VOS_UINT32	  huawei_flight_mode;  /*0��ʾ��������ģʽ���ر���Ƶ*/
}FLIGHT_MODE_STRU;

/*****************************************************************************
 �ṹ��    : NV_HUAWEI_PWRCFG_SWITCH
 �ṹ˵��  : ����书�����ÿ���״̬��0:�ر�1:��
 �ṹ ID   : 50490
*****************************************************************************/
typedef struct
{
    VOS_UINT8 pwrcfg_switch;
    VOS_UINT8 reserved [7];
} PWRCFG_SWITCH_NV_TYPE;

/*****************************************************************************
 �ṹ��    : PWRCFG_WCDMA_NV_TYPE
 �ṹ˵��  : ������¼GSMƵ�ε�����书��ֵ��reserved[8]����
             �û�AT�������õ�GSM����书��ֵ��
 �ṹ ID   : 50491
*****************************************************************************/
typedef struct
{
    VOS_UINT16 gsm_power[8];
    VOS_UINT16 reserved[8];
} PWRCFG_GSM_NV_TYPE;

/*****************************************************************************
 �ṹ��    : PWRCFG_WCDMA_NV_TYPE
 �ṹ˵��  : ������¼WCDMAƵ�ε�����书��ֵ��wcdma_power[16]����
             �ײ�WCDMA����书��ֵ��reserved[8]�����û�AT��������
             ��WCDMA����书��ֵ��
 �ṹ ID   : 50492
*****************************************************************************/
typedef struct
{
    VOS_UINT16 wcdma_power[16];
    VOS_UINT16 reserved[8];
} PWRCFG_WCDMA_NV_TYPE;

/*****************************************************************************
 �ṹ��    : PWRCFG_LTE_NV_TYPE
 �ṹ˵��  : ������¼LTEƵ�ε�����书��ֵ��ǰ��32����Ա����
             �ײ�NV_ID_BODYSAR_L_PARA��ֵ������32����Ա�����û�AT��������
             ��LTE����书��ֵ��
 �ṹ ID   : 50494
*****************************************************************************/
typedef struct
{
    VOS_UINT16 lte_power[64];
} PWRCFG_LTE_NV_TYPE;

/*****************************************************************************
 �ṹ��    : PWRCFG_TDSCDMA_NV_TYPE
 �ṹ˵��  : ������¼TDSƵ�ε�����书��ֵ��dscdma_power[6]����
             �ײ�TDS����书��ֵ��reserved[8]�����û�AT��������
             ��TDS����书��ֵ��
 �ṹ ID   : 50495
*****************************************************************************/
typedef struct
{
    VOS_UINT16 tdscdma_power[6];
    VOS_UINT16 reserved[8];
} PWRCFG_TDSCDMA_NV_TYPE;

/*****************************************************************************
 �ṹ��    : NV_TTS_CFG_TYPE
 �ṹ˵��  : ������¼֧�ֵ�TTS������������أ�֧�ֵ������ȣ�
 �ṹ ID   : 50465
*****************************************************************************/
typedef struct
{
    VOS_UINT8  tts_flag;
    VOS_UINT8  tts_lang;
    VOS_UINT16 reserved;
} NV_TTS_CFG_TYPE;

#define HUAWEI_CUST_NV_NUM 128
/*****************************************************************************
 �ṹ��    : NV_HUAWEI_CUST_NVID_NV_TYPE
 �ṹ˵��  : ������¼�û�NV�б��е�NV�Ƿ��޸Ĺ����ܳ���128���ֽڡ�
             ÿ���ֽڱ�ʾһ���û�NV�Ƿ��û����ã�����֧��128���û�NV��
 �ṹ ID   : 50496
*****************************************************************************/
typedef struct
{
    VOS_UINT8 huawei_cust_nvid_nv[HUAWEI_CUST_NV_NUM];
} NV_HUAWEI_CUST_NVID_NV_TYPE;

/*****************************************************************************
 �ṹ��    : MLOG_SWITCH_NV_STRU
 �ṹ˵��  : nv50497 ��������mobile logger�����Ƿ���
              0 -- �رգ�1 -- ����
 �ṹ ID   : 50497
*****************************************************************************/
typedef struct
{
    BSP_U8     mlog_switch;  /*�Ƿ��mobile logger����*/
    BSP_U8     reserved[3];  /*��չ�ֽ�,����4�ֽڶ���*/ 
}MLOG_SWITCH_NV_STRU;

/*****************************************************************************
 �ṹ��    : NV_HUAWEI_CUST_NVID_RESTORE_TYPE
 �ṹ˵��  : ������¼�û�NV�б��е�NV�Ƿ���Ҫ�����󱻻ָ����ܳ���128���ֽڡ�
             ÿ���ֽڱ�ʾһ���û�NV�Ƿ���Ҫ�ָ�������֧��128���û�NV��
 �ṹ ID   : 50540
*****************************************************************************/
typedef struct
{
    VOS_UINT8 huawei_cust_nvid_restore[HUAWEI_CUST_NV_NUM];
} NV_HUAWEI_CUST_NVID_RESTORE_TYPE;

/*****************************************************************************
 �ṹ��    : 1.1	NV_HUAWEI_S3S4_ REMOTE_WAKEUP_ENABLE
 �ṹ˵��  : ����ʹ����������S3S4״̬ʱ��remote wakeup�Ƿ�ʹ�ܡ�
��ȡ1ʱ����ʾʹ�ܣ�0��ʾ��ʹ�ܡ�
 �ṹ ID   : 50540
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
    VOS_UINT8  ucPDPStatusEnable;           /*PDPStatus�����ϱ�*/
    VOS_UINT8  ucCPBREADYEnable;            /*CPBREADY�����ϱ�*/
    VOS_UINT8  ucNWNAMEEnable;              /*NWNAME�����ϱ�*/
    VOS_UINT8  ucMbbFastDormEnable;         /*MbbFastDorm*/
    VOS_UINT8  ucPcmModemLoopEnable;        /*PCM�����ػ�����*/
}NV_HUAWEI_MBB_FEATURE_STRU;

/*****************************************************************************
 �ṹ��    : NV_HUAWEI_WEB_ADMIN_STRU
 �ṹ˵��  : ��������web site password
 �ṹ ID   : 50418
*****************************************************************************/
#define ADMIN_NUM  16
typedef struct
{
    VOS_UINT8 web_admin[ADMIN_NUM];
} NV_HUAWEI_WEB_ADMIN_STRU;

/*****************************************************************************
 �ṹ��    : NV_AGING_TEST_CTRL_STRU
 �ṹ˵��  : ���������ϻ����Բ���
 �ṹ ID   : 
*****************************************************************************/
#define RF_BAND_MAX_NUM  20
#define WIFI_MODE_MAX_NUM  20
typedef struct 
{
    VOS_UINT16 led_test_enable;   /*LED  �ϻ����Կ���*/
    VOS_UINT16 on_time;           /*LED����ʱ��*/
    VOS_UINT16 off_time;          /*LED����ʱ��*/
    VOS_UINT16 reserve;
}AGING_LED_STRU;

typedef struct
{
    VOS_UINT16 mode;        /* wifi  ģʽ+��ʽ*/
    VOS_UINT16 frequency;   /* wifi  Ƶ��*/
}AGING_WIFI_CONFIG;

typedef struct 
{
    VOS_UINT16 wifi_test_enable; /*wifi�ϻ����Կ���*/
    VOS_UINT16 mode_num;        /*����ģʽ����*/
    VOS_UINT16 on_time;         /*����ʱ��*/
    VOS_UINT16 off_time;        /*��Ϣʱ��*/
    AGING_WIFI_CONFIG wifi_parametr[WIFI_MODE_MAX_NUM];
}AGING_WIFI_STRU;

typedef struct
{
    VOS_UINT16 mode;        /*RF��ʽȡֵ1-4: 1--CDMA, 2--GSM, 3--WCDMA, 4--LTE*/
    VOS_UINT16 band;        /*RFƵ��*/
    VOS_UINT16 frequency;   /*RF Ƶ��*/
    VOS_UINT16 reserve;
}AGING_RF_CONFIG;

typedef struct 
{
    VOS_UINT16 rf_test_enable;   /*lte �ϻ����Կ���*/
    VOS_UINT16 band_num;          /*����Ƶ�θ���*/
    VOS_UINT16 on_time;          /*����ʱ��*/
    VOS_UINT16 off_time;         /*��Ϣʱ��*/
    AGING_RF_CONFIG rf_parametr[RF_BAND_MAX_NUM];
}AGING_RF_STRU;

typedef struct
{
    VOS_UINT32 aging_test_enable:1;   /*�ϻ������ܿ���*/
    VOS_UINT32 charge_test_enable:1;  /*��ŵ��ϻ����Կ���*/
    VOS_UINT32 lcd_test_enable:1;     /*lcd �ϻ����Կ���*/
    VOS_UINT32 flash_test_enable:1;   /*flash �ϻ����Կ���*/
    VOS_UINT32 cpu_test_enable:1;     /*CPU �ϻ����Կ���*/
    VOS_UINT32 reserve:27;            /*Ԥ����*/
    VOS_UINT32 total_time;            /*������ʱ��*/
    AGING_LED_STRU  led_parameter;    /*led �ϻ����Խṹ*/
    AGING_WIFI_STRU wifi_parameter;   /*wifi �ϻ����Խṹ*/
    AGING_RF_STRU  rf_test;           /*lte �ϻ����Խṹ*/
}NV_AGING_TEST_CTRL_STRU;
/*****************************************************************************
 �ṹ��    : CPE_TELNET_SWITCH_NVIM_STRU
 �ṹ˵��  : ��NV�����ڿ���CPE��Ʒtelnet�����Ƿ�򿪡�
 �ṹ ID   : 50518
*****************************************************************************/
#define SB_SERIAL_NUM_NV_LEN (20)
typedef struct
{
    VOS_UINT8 serial_num[SB_SERIAL_NUM_NV_LEN];
}SB_SERIAL_NUM_STRU;


/*****************************************************************************
 �ṹ��    :  NV_RF_ANTEN_DETECT_GPIO_STRU
 �ṹ˵��  :  ��Ƶ���߲��nv����
              anten[]:��¼�����Ƶ�����Ƿ����Ķ�ӦGPIO��š�
              �мǣ�Modem 0���ߵ�GPIO�ű������õ�0~3Ԫ�أ�Modem 1����GPIO�ű������õ�4~7Ԫ�ء�
              ��Ӳ��δԤ��GPIO�ţ�������Ϊ0xffff��Ĭ��RF����δ���롣
              ĿǰP750s��Ŀֻ֧��Modem 0����֧��Modem 1��
 NV ID    :   50569
*****************************************************************************/
#define MODEM_NUM            (2) /* ֧�ֵĿ������Ŀ: MODEM 0,1 */
#define ANTEN_NUM_PER_MODEM  (4) /* ÿ����֧�ֵ����������Ƶ���ߵ���Ŀ */
#define ANTEN_MAX_NUM        (MODEM_NUM * ANTEN_NUM_PER_MODEM) /* modem 0��1֧�ֵ����������Ŀ */

typedef struct
{
    VOS_UINT32 enable;               //���ʹ�ܣ�1ʹ�ܣ�0ȥʹ��
    VOS_UINT32 anten[ANTEN_MAX_NUM]; //����Ӧ��������Ƶ���Ƿ���λ��GPIO���
    VOS_UINT32 reserved1;            //Ԥ��
} NV_RF_ANTEN_DETECT_GPIO_STRU;
/*****************************************************************************
 �ṹ��    :  NV_SBM_APN_FLAG_STRU
 �ṹ˵��  : ����APN������������ǵ�FLAG
 NV ID    :   50554
*****************************************************************************/

typedef struct
{
    VOS_UINT8       ucApnFlag;       /*�Ϸ�ȡֵ0��1*/
    VOS_UINT8       aucReserved[3];  /*Ԥ��*/
}NV_SBM_APN_FLAG_STRU;


#define MAX_BAND_NUM  (10)   /* ���ߵ����band��*/
#define MAX_ANT_NUM   (8)    /* ��������� */

/*ÿ�����ߵ�ʵ��band�����������ּ���ʶλ*/
typedef struct
{
    VOS_UINT16   band_num   :4;              /*ÿ�����ߵ�ʵ��band����*/
    VOS_UINT16   band_id0_main_div_flag :1;  /*�������ϵ�0��band�����ּ���ʶλ*/
    VOS_UINT16   band_id1_main_div_flag :1;  /*�������ϵ�1��band�����ּ���ʶλ*/
    VOS_UINT16   band_id2_main_div_flag :1;  /*�������ϵ�2��band�����ּ���ʶλ*/
    VOS_UINT16   band_id3_main_div_flag :1;  /*�������ϵ�3��band�����ּ���ʶλ*/
    VOS_UINT16   band_id4_main_div_flag :1;  /*�������ϵ�4��band�����ּ���ʶλ*/
    VOS_UINT16   band_id5_main_div_flag :1;  /*�������ϵ�5��band�����ּ���ʶλ*/
    VOS_UINT16   band_id6_main_div_flag :1;  /*�������ϵ�6��band�����ּ���ʶλ*/
    VOS_UINT16   band_id7_main_div_flag :1;  /*�������ϵ�7��band�����ּ���ʶλ*/
    VOS_UINT16   band_id8_main_div_flag :1;  /*�������ϵ�8��band�����ּ���ʶλ*/
    VOS_UINT16   band_id9_main_div_flag :1;  /*�������ϵ�9��band�����ּ���ʶλ*/
    VOS_UINT16   reserved   :2;              /*��չbitλ�������ֽڶ���*/
}BAND_NUM_MAIN_DIV_STRU;

/*ÿ�������ϵ�band��Ϣ*/
typedef struct
{
    VOS_UINT8               band_idx[MAX_BAND_NUM];  /*ÿ��BAND��band�ţ�MAX_BAND_NUM Ϊ10*/
    BAND_NUM_MAIN_DIV_STRU  band_num_main_div_flag;  /*����ʵ��BAND�����������ּ���ʶλ*/
}BAND_INFO_PER_ANT_STRU;

/*ʵ����Ƶ��������������ÿ�����ߵ���Ϣ*/
typedef struct
{
    VOS_UINT8                ant_num;               /*ʵ����Ƶ������������*/
    VOS_UINT8                reserved[3];           /*��չ�ֽڣ�����4�ֽڶ���*/
    BAND_INFO_PER_ANT_STRU   ant_info[MAX_ANT_NUM]; /*ÿ�����߶�Ӧ����Ϣ MAX_ANT_NUMΪ8*/
}LTE_ANT_INFO_STRU;


typedef struct
{
    VOS_UINT32 nv_status;    /*��nv�Ƿ񼤻�*/
    VOS_UINT16 ucActiveFlag;/*�Ƿ���usb tethering����*/
    VOS_UINT16 ucReserved1; /*�����ֶ�1*/
    VOS_UINT16 ucReserved2; /*�����ֶ�2*/
    VOS_UINT16 ucReserved3; /*�����ֶ�3*/
}HUAWEI_NV_USB_TETHERING;
/*50577*/
typedef struct
{
VOS_UINT8 nv_status;    /*��nv�Ƿ񼤻�*/
VOS_UINT8 diag_enable;  /*diag�˿��Ƿ�ʹ��*/
VOS_UINT8 shell_enable; /*shell�˿��Ƿ�ʹ��*/
VOS_UINT8 adb_enable;   /*adb�˿��Ƿ�ʹ��*/
VOS_UINT8 cbt_enable;   /*cbt�˿��Ƿ�ʹ��*/
VOS_UINT8 ucReserved_0; /*�����ֶ�*/
VOS_UINT8 ucReserved_1; /*�����ֶ�*/
VOS_UINT8 ucReserved_2; /*�����ֶ�*/
}HUAWEI_NV_USB_SECURITY_FLAG;


typedef struct
{ 
    VOS_INT16  sHandSetUpVolValue;      /*�ֳ�ģʽ��������*/
    VOS_INT16  sHandsFreeUpVolValue;    /*����ģʽ��������*/
    VOS_INT16  sCarUpVolValue;          /*����ģʽ��������*/
    VOS_INT16  sHeadSetUpVolValue;      /*����ģʽ��������*/
    VOS_INT16  sBlueToothUpVolValue;    /*����ģʽ��������*/
    VOS_INT16  sPcVoiceUpVolValue;      /*PC����ģʽ��������*/
    VOS_INT16  sHeadPhoneUpVolValue;    /*��������MIC�Ķ���ͨ��ģʽ��������*/
    VOS_INT16  ucReserved[5];           /*Reserved*/
} APP_VC_NV_CMIC_VOLUME_STRU;

typedef struct
{
     VOS_UINT16  wifi_2g_ant0[3];       /*2.4g��ant0��wifiУ׼����*/
     VOS_UINT16  wifi_2g_ant1[3];       /*2.4g��ant1��wifiУ׼����*/
}NV_WIFI_2G_RFCAL_STRU;

typedef struct
{
     VOS_UINT16  wifi_5g_ant0[12];
     VOS_UINT16  wifi_5g_ant1[12];
     VOS_UINT16  wifi_5g_ant2[12];
}NV_WIFI_5G_RFCAL_STRU;

typedef struct
{
    VOS_INT32 lEnterPowerOffThreshold;     /*�����µ�ģʽ������*/
    VOS_INT32 lEnterFlightModeThreshold;  /*�������ģʽ���¶�����*/
    VOS_INT32 lExitFlightModeThreshold;     /*�˳�����ģʽ���¶�����*/
    VOS_INT32 lEnterSpeedLimit2Threshold; /*����������ٵ��¶�����*/
    VOS_INT32 lEnterSpeedLimit1Threshold; /*����һ�����ٵ��¶�����*/
    VOS_INT32 lReturnNormalThreshold;       /*��������״̬���¶�����*/
    VOS_INT32 lReserve[2];                            /*Ԥ��*/
} SPY_TEMP_PROTECT_SECOND_SAMPLE_THRESHOLD_STRU;

typedef struct
{
    VOS_UINT32 ulIsEnable; /*�Ƿ���Ҫ������������, 0Ϊ�رգ�1Ϊ����*/
    VOS_UINT32 ulLocation; /*ʹ��������¶���Ϊ�±������룬�ò���ֱ����Ϊ��ν������ĺ���,Ŀǰ��ȡֵ��Χ��0��6 */
    SPY_TEMP_PROTECT_SECOND_SAMPLE_THRESHOLD_STRU threshold; /*�����¶ȱ���״̬�¶�����ֵ*/
} SPY_TEMP_PROTECT_SENCOND_SAMPLE_NV_STRU;

//#if (FEATURE_ON == MBB_WPG_CELLLOCK_CPE)
/*****************************************************************************
 �ṹ��    : NV_Item_CELL_LOCK_STATUS_NV_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : NV_Item_CELL_LOCK_STATUS_NV_STRU  50552
*****************************************************************************/
typedef struct
{ 
    VOS_UINT8                               cell_lock_status;   /* ��С��״̬*/
    VOS_UINT8                               cell_max_num;       /* �Ϸ�С���������ޣ�������Ӫ�̶���*/
    VOS_UINT8                               cell_default_num;       /* �Ϸ�С����������Ĭ��ֵ*/
    VOS_UINT8                               reserved;                                           
}NV_Item_CELL_LOCK_STATUS_NV_STRU; 


/*****************************************************************************
 �ṹ��    : NV_Item_CELL_LOCK_GSM_CELL_ID_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : en_NV_Item_CELL_LOCK_GSM_CELL_ID_INFO  50553
*****************************************************************************/
typedef struct
{ 
    VOS_UINT8                                 cell_lock_type;   /*��GSM\WCDMA\LTE����С��*/                                  
    VOS_UINT8                                 cell_actual_num;    /* �Ϸ�С����ʵ����Ŀ */
    VOS_UINT8                                 reserved[2]; 
    VOS_UINT8                                 cell_id_list[45][8];  /* �洢������ע����ĺϷ���CELL��CELL ID */                                             
}NV_Item_CELL_LOCK_GSM_CELL_ID_INFO_STRU;

/*****************************************************************************
 �ṹ��    : NV_Item_CELL_LOCK_WCDMA_CELL_ID_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : en_NV_Item_CELL_LOCK_WCDMA_CELL_ID_INFO  50554
*****************************************************************************/
typedef struct
{ 
    VOS_UINT8                                 cell_lock_type;    /*��GSM\WCDMA\LTE����С��*/                                      
    VOS_UINT8                                 cell_actual_num;    /* �Ϸ�С����ʵ����Ŀ */
    VOS_UINT8                                 reserved[2]; 
    VOS_UINT8                                 cell_id_list[45][8];  /* �洢������ע����ĺϷ���CELL��CELL ID */                                             
}NV_Item_CELL_LOCK_WCDMA_CELL_ID_INFO_STRU; 

/*****************************************************************************
 �ṹ��    : NV_Item_CELL_LOCK_LTE_CELL_ID_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : en_NV_Item_CELL_LOCK_LTE_CELL_ID_INFO  50555
*****************************************************************************/
typedef struct
{ 
    VOS_UINT8                                 cell_lock_type;    /*��GSM\WCDMA\LTE����С��*/                                    
    VOS_UINT8                                 cell_actual_num;    /* �Ϸ�С����ʵ����Ŀ */
    VOS_UINT8                                 reserved[2]; 
    VOS_UINT8                                 cell_id_list[45][8];  /* �洢������ע����ĺϷ���CELL��CELL ID */                                             
}NV_Item_CELL_LOCK_LTE_CELL_ID_INFO_STRU; 

/*****************************************************************************
 �ṹ��    : NV_Item_CELL_LOCK_DIAL_TIME_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : en_NV_Item_CELL_LOCK_DIAL_TIME  50556 
*****************************************************************************/
typedef struct
{
    VOS_UINT32                       dial_time_single_max;  //���β���ʱ�����ֵ
    VOS_UINT32                       dial_time_total_max;  //�ۼƲ���ʱ�����ֵ
    VOS_UINT32                       dial_time_total_actual; //ʵ�ʲ�����ʱ��
}NV_Item_CELL_LOCK_DIAL_TIME_STRU;

/*****************************************************************************
 �ṹ��    : NV_Item_CELL_LOCK_CALL_NUM_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : en_NV_Item_CELL_LOCK_CALL_NUM 50557 
*****************************************************************************/
typedef struct
{
    VOS_UINT32                           call_num_max; //����绰������
    VOS_UINT32                           call_num_actual; //����绰ʵ�ʴ���
}NV_Item_CELL_LOCK_CALL_NUM_STRU;

/*****************************************************************************
 �ṹ��    : NV_Item_CELL_LOCK_ALLOW_CALL_NUM_LIST_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : en_NV_Item_CELL_LOCK_ALLOW_CALL_NUM_LIST  50558
*****************************************************************************/
typedef struct
{
    VOS_UINT8              permit_dial_num;    /* �Ƿ�С��������������ʵ�ʸ�����0��ʾ�������������ⲻ��������κκ��� */  
    VOS_UINT8              permit_dial_number[5][21];     /* �Ƿ�С��������������б�  */  
    VOS_UINT8              reserved[2];
}NV_Item_CELL_LOCK_ALLOW_CALL_NUM_LIST_STRU;

//#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


