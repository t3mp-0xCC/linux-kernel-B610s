

#ifndef __PRODUCT_NV_ID_H__
#define __PRODUCT_NV_ID_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*
 *  NV ID 的添加按从小到大排列
 */

typedef enum
{
    NV_ID_CRC_CHECK_RESULT = 0xc350,
    NV_ID_GU_CHECK_ITEM    = 0xc351,
    NV_ID_TL_CHECK_ITEM    = 0xc352,
    NV_ID_GU_M2_CHECK_ITEM = 0xc353,

    NV_ID_FLIGHT_MODE	  = 0xC37B,    /*50043*/
    NV_HUAWEI_GPS_ENABLE              = 0xC394,    /*50068*/     
    NV_Item_GPS_USER_SETTING          = 0xC399,    /*50073*/     
    NV_ID_GID1_LOCK                   = 0xC3A3,    /*50083*/
    NV_ID_DYNAMIC_NAME                = 0xc3bc,     /*50108*/
    NV_ID_DYNAMIC_INFO_NAME           = 0xc3bd,     /*50109*/
    NV_ID_SPECIAL_PLMN_NOT_ROAM       = 0xC3B7,    /*50103*/
    NV_Item_HUAWEI_PLMN_MODIFY        = 0xC3C3,    /*50115*/
    NV_ID_CSIM_CUSTOMIZATION          = 0xC40D,    /*50189*/
    NV_ID_HUAWEI_SYSCFGEX_MODE_LIST   = 0xC416,    /*50198*/
    en_NV_Item_THERMAL_CONFIG         = 0xC44F,    /*50255*/
    NV_Item_GPS_SETTING               = 0xC468,    /*50280*/
    en_NV_Item_WAKEUP_CFG_FLAG        = 0xC47B,    /* 50299  */
#if (FEATURE_ON == MBB_FEATURE_SKU)
    NV_ITEM_SKU_TYPE                  = 0xC4A0,    /*50336*/
#endif
    NV_CALL_SPECIAL_NUM_I             = 0xc4b8,       /* 50360  */
    NV_TEST_POWERUP_MODE_CONTROL_FLAG = 0xC4BC,   /*50364*/
    NV_ID_HUAWEI_ANTENNA_TUNER_GSM    = 0xC4C9,    /* 50377 */
    NV_ID_HUAWEI_ANTENNA_TUNER_WCDMA  = 0xC4CA,    /* 50378 */
    NV_ID_HUAWEI_ANTENNA_TUNER_CDMA   = 0xC4CB,    /* 50379 */
    NV_ID_HUAWEI_ANTENNA_TUNER_LTE    = 0xC4CC,    /* 50380 */
    NV_CHG_SHUTOFF_TEMP_PROTECT_I     = 0xC4D1,   /*50385*/
    NV_CHG_SHUTOFF_VOLT_PROTECT_I     = 0xC4D2,   /*50386*/
    NV_HUAWEI_DYNAMIC_VID             = 0xC4DA,   /*50394*/
    NV_HUAWEI_DYNAMIC_BOOT_PID        = 0XC4DB,   /*50395*/
    NV_HUAWEI_DYNAMIC_NORMAL_PID      = 0XC4DC,   /*50396*/
    NV_HUAWEI_DYNAMIC_DEBUG_PID       = 0XC4DD,   /*50397*/
    NV_FOTA_PPP_APN = 0xC4E0,     /* 50400 */
    NV_FOTA_PPP_PASSWORD = 0xC4E1,   /* 50401 */
    NV_FOTA_PPP_USER_ID = 0xC4E2,    /* 50402 */
    NV_FOTA_PPP_AUTHTYPE = 0xC4E3,   /* 50403 */
    NV_FOTA_SERVER_IP = 0xC4E4,      /* 50404 */
    NV_FOTA_SERVER_PORT = 0xC4E5,    /* 50405 */
    NV_FOTA_DETECT_MODE = 0xC4E6,    /* 50406 */
    NV_FOTA_DETECT_COUNT = 0xC4E7,   /* 50407 */
    NV_FOTA_NWTIME = 0xC4E8,      /* 50408 */
    NV_FOTA_TIMER = 0xC4E9,       /* 50409 */
    NV_HUAWEI_MULTI_IMAGE_I  = 0xC4EC,  /*50412*/
    NV_HUAWEI_MULTI_CARRIER_AUTO_SWITCH_I  = 0xC4ED,  /*50413*/
    NV_ID_CUSTOMIZED_BAND_GROUP       = 0xC4FB,   /*50427*/
    NV_ID_SOFT_RELIABLE_CFG  = 0xC50A,
    NV_ID_HPLMN_FIRST_UMTS_TO_LTE     = 0xC515,   /*50453*/
    NV_ID_EE_OPERATOR_NAME_DISPLAY    = 0xC516,   /*50454*/
    NV_ID_TTS_CFG_I                   = 0xC521,   /*50465*/
    NV_ID_WEB_SITE                    = 0xCB84,
    NV_ID_WPS_PIN                     = 0xCB8D,
    NV_ID_WEB_USER_NAME               = 0xCB9C,
    NV_ID_WEB_ADMIN_PASSWORD_NEW_I = 0xC4F2,   /*50418*/
#if (FEATURE_ON == MBB_FEATURE_DCM_MNC_COMPARE)
    en_NV_Item_Roam_MNC_ExactlyCmp_Config = 0xC4F4,  /*50420*/
#endif/*FEATURE_ON == MBB_FEATURE_DCM_MNC_COMPARE*/

    NV_ID_HPLMN_SEARCH_REGARDLESS_MCC_SUPPORT_EX = 0xC4F6,/* 50422 */
    NV_ID_DSFLOW_REPORT_MUTI_WAN = 0xC4EA,/*50410*/
    NV_ID_WINBLUE_PROFILE             = 0xC4F8, /* 50424 */
    NV_ITEM_RPLMN_ACT_DISABLE_CFG = 0xC4F9,  /*50425*/
    NV_ID_PIH_IMSI_TIMER              = 0xC4FA,   /*50426*/
    NV_START_MANUAL_TO_AUTO = 0xC4FC, /*50428 */
    NV_ID_VODAFONE_CPBS               = 0xC4FD,   /*50429*/
#if (FEATURE_ON == MBB_THERMAL_PROTECT)
    NV_LIMIT_SPEED_SECOND_TEMP_PROTECT = 0xC4FE,    /*50430*/
#endif
    en_NV_DEVNUMW_TIME_I             = 0xC4FF ,   /*50431*/
    NV_ID_TATA_DEVICE_LOCK            = 0xC500,   /*50432*/
    NV_Item_HPLMNWithinEPLMNNotRoam = 0XC501,/*50433 */
    NV_Item_APN_LOCK = 0xC508,/*50440*/
    NV_ID_ATCMD_AMPW_STORE = 0xC509, /* 50441*/
    NV_FOTA_DETECT_INTERVAL = 0xC50F, /* 50447 */
    NV_ID_SMS_AUTO_REG                = 0xC511,   /*50449*/
    NV_ID_TME_CPIN                    = 0xC512,   /*50450*/
    NV_ID_LED_PARA                  = 0xC517,/* 50455  */
    NV_ID_USB_CDC_NET_SPEED           = 0xC518, /* 50456 */
    NV_ID_USB_MULTI_CONFIG_PORT           = 0xC519, /* 50457 */
    NV_M2M_LED_CONTROL_EX               = 0xC51D,   /* 50461 */
    NV_HUAWEI_COUL_INFO_I             = 0xC51E,  /*50462*/
    NV_ID_HUAWEI_ANTENNA_TUNER_TDSCDMA = 0xC520, /* 50464 */
    NV_ID_PLATFORM_CATEGORY_SET     = 0xC51A,   /* 50458 */
    en_NV_Item_SLEEP_CFG_FLAG       = 0xC523,   /* 50467 */
    NV_ID_AT_GET_CHANWIINFO = 0xC524,/*50468*/
    NV_ID_GET_PWRWIINFO = 0xC525,/*50469*/
    NV_Item_FOTA_SMS_CONFIG = 0xC526,    /* 50470 */
    NV_Item_FOTA_TIME_STAMP = 0xC527,  /* 50471 */
    NV_Item_FOTA_RSA_PUB_MOD = 0xC528,  /* 50472 */ 
    NV_Item_FOTA_RSA_PUB_EXP = 0xC529,	/* 50473 */ 
    NV_FOTA_SMS_FLAG = 0xC531,            /* 50481 */
#if (FEATURE_ON == MBB_WPG_CELLROAM)
    Nas_Mmc_Nvim_Roam_Mcc_Cmp        = 0xC533, /*50483*/
#endif /*FEATURE_ON == MBB_WPG_CELLROAM*/
    NV_HUAWEI_PWRCFG_SWITCH = 0xC53A,    /* 50490 */
    NV_HUAWEI_PWRCFG_GSM = 0xC53B,    /* 50491 */
    NV_HUAWEI_PWRCFG_WCDMA = 0xC53C,  /* 50492 */
    NV_HUAWEI_PWRCFG_LTE = 0xC53E,  /* 50494 */
    NV_HUAWEI_PWRCFG_TDSCDMA = 0xC53F,  /* 50495 */
    NV_ID_HUAWEI_CUST_NVID_NV = 0xC540, /* 50496 */
    NV_HUAWEI_MLOG_SWITCH    = 0xC541, /*50497*/
    NV_USB_PRIVATE_INFO    = 0xC542, /*50498*/
    NV_TELNET_SWITCH_I             = 0xC545,  /*50501*/
    NV_HUAWEI_OEMLOCK_I   = 0xC546 ,  /*50502*/
    NV_HUAWEI_SIMLOCK_I   = 0xC547 ,  /*50503*/
    NV_ID_ATCMD_AMPW_ADD_IPR_STORE = 0xc549,  /*50505*/
    NV_ID_SIM_CUSM_FEATURE = 0xC54E,/*50510*/
    NV_HUAWEI_ROAM_WHITELIST_I         = 0xC550, /*50512*/
    NV_HUAWEI_ROAM_WHITELIST_EXTEND1_I = 0xC551, /*50513*/
    NV_DRV_MAC_NUM                  = 0xC555, /*50517*/
    NV_TEMP_PROTECT_SAR_REDUCTION      = 0xC558 ,  /*50520*/
    NV_ID_USB_TETHERING = 0xC55C,/*50524*/
    en_NV_Item_AUTHFALLBACK_FEATURE = 50525,  /* 50525 */      
    en_NV_Item_CUSTOMAUTH_FEATURE = 50526,    /* 50526 */      
    en_NV_HUAWEI_SIM_SWITCH = 50529,  /*50529*/    
    NV_ID_USB_DEBUG_MODE_FLAG           = 0xC56A, /* 50538 */
    NV_ID_HUAWEI_CUST_NVID_RESTORE = 0xC56C, /* 50540 */
    NV_ID_BODYSAR_CFG                =0xC56D,
    NV_ID_BODYSAR_L_PARA         =0xC56E,
    NV_HUAWEI_S3S4_REMOTE_WAKEUP_ENABLE = 0xC571,
    NV_ID_HUAWEI_MBB_FEATURE         = 0xC572, /* 50546 */
#if ((FEATURE_ON ==  MBB_FEATURE_SBM_APN_MATCH) || (FEATURE_ON == MBB_WPG_KDDI_APN_MATCH))
    NV_ID_SBM_APN_FLAG                      = 0xC57A, /*50554*/
#endif/*FEATURE_ON ==  MBB_FEATURE_SBM_APN_MATCH*/
    NV_ID_HUAWEI_AGING_TEST          = 0xC581, /* 50561 */
    NV_SB_SERIAL_NUM                     = 0xC582,  /*50562*/
    NV_ID_RF_ANTEN_DETECT                = 0xC589,  /*50569*/
    NV_ID_LTE_ANT_INFO                   = 0xC58E,  /*50574*/
    NV_ID_USB_SECURITY                   = 0xC591,/*50577*/
#if(FEATURE_ON == MBB_WPG_PCM)
    NV_ID_Item_CMIC_Volume        = 0xC5A1,   /*50593*/
#endif /* FEATURE_ON == MBB_WPG_PCM */
    NV_ID_WIFI_2G_RFCAL    = 0xC5A9,       /*50601*/
    NV_ID_WIFI_5G_RFCAL    = 0xC5AA,       /*50602*/
#if (FEATURE_ON == MBB_THERMAL_PROTECT_EXT)
    NV_ID_TEMP_PROTECT_SECOND_SAMPLE = 0xC5B7,    /*50615*/
#endif

#if (FEATURE_ON == MBB_WPG_CELLLOCK_CPE)
    en_NV_Item_CELL_LOCK_STATUS_NV = 0xC5BC,	 /*50620*/		 
    en_NV_Item_CELL_LOCK_GSM_CELL_ID_INFO = 0xC5BD,
    en_NV_Item_CELL_LOCK_WCDMA_CELL_ID_INFO = 0xC5BE,
    en_NV_Item_CELL_LOCK_LTE_CELL_ID_INFO = 0xC5BF,
    en_NV_Item_CELL_LOCK_DIAL_TIME = 0xC5C0,
    en_NV_Item_CELL_LOCK_CALL_NUM = 0xC5C1,
    en_NV_Item_CELL_LOCK_ALLOW_CALL_NUM_LIST = 0xC5C2,
#endif	
    NV_ID_PRODUCT_END      = 0xcb1f
}NV_ID_PRODUCT_ENUM;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


