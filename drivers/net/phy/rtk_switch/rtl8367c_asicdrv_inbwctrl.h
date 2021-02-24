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
#ifndef _RTL8367C_ASICDRV_INBWCTRL_H_
#define _RTL8367C_ASICDRV_INBWCTRL_H_

#include <rtl8367c_asicdrv.h>

extern ret_t rtl8367c_setAsicPortIngressBandwidth(rtk_uint32 port, rtk_uint32 bandwidth, rtk_uint32 preifg, rtk_uint32 enableFC);
extern ret_t rtl8367c_getAsicPortIngressBandwidth(rtk_uint32 port, rtk_uint32* pBandwidth, rtk_uint32* pPreifg, rtk_uint32* pEnableFC );
extern ret_t rtl8367c_setAsicPortIngressBandwidthBypass(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicPortIngressBandwidthBypass(rtk_uint32* pEnabled);


#endif /*_RTL8367C_ASICDRV_INBWCTRL_H_*/

