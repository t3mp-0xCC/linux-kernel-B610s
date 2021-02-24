/* 
 * Copyright (C) 2013 Realtek Semiconductor Corp.  All Rights Reserved.
 * 
 * www.realtek.com 
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
#ifndef _RTL8367C_ASICDRV_METER_H_
#define _RTL8367C_ASICDRV_METER_H_

#include <rtl8367c_asicdrv.h>


extern ret_t rtl8367c_setAsicShareMeter(rtk_uint32 index, rtk_uint32 rate, rtk_uint32 ifg);
extern ret_t rtl8367c_getAsicShareMeter(rtk_uint32 index, rtk_uint32 *pRate, rtk_uint32 *pIfg);
extern ret_t rtl8367c_setAsicShareMeterBucketSize(rtk_uint32 index, rtk_uint32 lbThreshold);
extern ret_t rtl8367c_getAsicShareMeterBucketSize(rtk_uint32 index, rtk_uint32 *pLbThreshold);
extern ret_t rtl8367c_setAsicShareMeterType(rtk_uint32 index, rtk_uint32 type);
extern ret_t rtl8367c_getAsicShareMeterType(rtk_uint32 index, rtk_uint32 *pType);
extern ret_t rtl8367c_setAsicMeterExceedStatus(rtk_uint32 index);
extern ret_t rtl8367c_getAsicMeterExceedStatus(rtk_uint32 index, rtk_uint32* pStatus);

#endif /*_RTL8367C_ASICDRV_FC_H_*/

