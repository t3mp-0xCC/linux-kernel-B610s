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
  
#ifndef _WLAN_AT_H_
#define _WLAN_AT_H_

#include "wlan_at_api.h"
#include "wlan_utils.h"

/* 以下为各个芯片按需实现的接口 */
typedef struct 
{
    int32 (*WlanATSetWifiEnable)(WLAN_AT_WIENABLE_TYPE onoff);
    WLAN_AT_WIENABLE_TYPE (*WlanATGetWifiEnable)(void);

    int32 (*WlanATSetWifiMode)(WLAN_AT_WIMODE_TYPE mode);
    int32 (*WlanATGetWifiMode)(WLAN_AT_BUFFER_STRU *strBuf);
    int32 (*WlanATGetWifiModeSupport)(WLAN_AT_BUFFER_STRU *strBuf);

    int32 (*WlanATSetWifiBand)(WLAN_AT_WIBAND_TYPE band);
    int32 (*WlanATGetWifiBand)(WLAN_AT_BUFFER_STRU *strBuf);
    int32 (*WlanATGetWifiBandSupport)(WLAN_AT_BUFFER_STRU *strBuf);

    int32 (*WlanATSetWifiFreq)(WLAN_AT_WIFREQ_STRU *pFreq);
    int32 (*WlanATGetWifiFreq)(WLAN_AT_WIFREQ_STRU *pFreq);

    int32 (*WlanATSetWifiDataRate)(uint32 rate);
    uint32 (*WlanATGetWifiDataRate)(void);

    int32 (*WlanATSetWifiPOW)(int32 power_dBm_percent);
    int32 (*WlanATGetWifiPOW)(void);

    int32 (*WlanATSetWifiTX)(WLAN_AT_FEATURE_TYPE onoff);
    WLAN_AT_FEATURE_TYPE (*WlanATGetWifiTX)(void);

    int32 (*WlanATSetWifiRX)(WLAN_AT_WIRX_STRU *params);
    int32 (*WlanATGetWifiRX)(WLAN_AT_WIRX_STRU *params);

    int32 (*WlanATSetWifiRPCKG)(int32 flag);
    int32 (*WlanATGetWifiRPCKG)(WLAN_AT_WIRPCKG_STRU *params);

    int32 (*WlanATGetWifiInfo)(WLAN_AT_WIINFO_STRU *params);
    
    WLAN_AT_WIPLATFORM_TYPE (*WlanATGetWifiPlatform)(void);

    int32 (*WlanATGetTSELRF)(void); 
    int32 (*WlanATSetTSELRF)(uint32 group);
    int32 (*WlanATGetTSELRFSupport)(WLAN_AT_BUFFER_STRU *strBuf);

    int32 (*WlanATSetWifiParange)(WLAN_AT_WiPARANGE_TYPE pa_type);
    WLAN_AT_WiPARANGE_TYPE (*WlanATGetWifiParange)(void);

    int32 (*WlanATGetWifiParangeSupport)(WLAN_AT_BUFFER_STRU *strBuf);

    int32 (*WlanATGetWifiCalTemp)(WLAN_AT_WICALTEMP_STRU *params);
    int32 (*WlanATSetWifiCalTemp)(WLAN_AT_WICALTEMP_STRU *params);

    int32 (*WlanATSetWifiCalData)(WLAN_AT_WICALDATA_STRU *params);
    int32 (*WlanATGetWifiCalData)(WLAN_AT_WICALDATA_STRU *params);

    int32 (*WlanATSetWifiCal)(WLAN_AT_FEATURE_TYPE onoff);
    WLAN_AT_FEATURE_TYPE (*WlanATGetWifiCal)(void);
    WLAN_AT_FEATURE_TYPE (*WlanATGetWifiCalSupport)(void);

    int32 (*WlanATSetWifiCalFreq)(WLAN_AT_WICALFREQ_STRU *params);
    int32 (*WlanATGetWifiCalFreq)(WLAN_AT_WICALFREQ_STRU *params);

    int32 (*WlanATSetWifiCalPOW)(WLAN_AT_WICALPOW_STRU *params);
    int32 (*WlanATGetWifiCalPOW)(WLAN_AT_WICALPOW_STRU *params);

    /*wifi射频参数查询设置接口*/
    int32 (*WlanATGetWifi2GPavars)(WLAN_AT_BUFFER_STRU *params);
    int32 (*WlanATSetWifi2GPavars)(WLAN_AT_PAVARS2G_STRU *params);
    int32 (*WlanATGetWifi5GPavars)(WLAN_AT_BUFFER_STRU *params);
    int32 (*WlanATSetWifi5GPavars)(WLAN_AT_PAVARS5G_STRU *params);
    int32 (*WlanATGetInitWifi2GPavars)(WLAN_AT_BUFFER_STRU *params);
    int32 (*WlanATGetInitWifi5GPavars)(WLAN_AT_BUFFER_STRU *params);
}WLAN_CHIP_OPS;

/*芯片当前切换*/
void wlan_at_switch_chip(int chip_type);
/*芯片注册*/
WLAN_AT_RETURN_TYPE wlan_at_reg_chip(int32 chip_type, WLAN_CHIP_OPS *chip_ops);
/*芯片去注册*/
WLAN_AT_RETURN_TYPE wlan_at_unreg_chip(int32 chip_type);
#endif


