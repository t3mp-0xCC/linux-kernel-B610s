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
#ifndef _RTL8367C_ASICDRV_EEE_H_
#define _RTL8367C_ASICDRV_EEE_H_

#include <rtl8367c_asicdrv.h>

#define EEE_OCP_PHY_ADDR    (0xA5D0)

extern ret_t rtl8367c_setAsicEee100M(rtk_uint32 port, rtk_uint32 enable);
extern ret_t rtl8367c_getAsicEee100M(rtk_uint32 port, rtk_uint32 *enable);
extern ret_t rtl8367c_setAsicEeeGiga(rtk_uint32 port, rtk_uint32 enable);
extern ret_t rtl8367c_getAsicEeeGiga(rtk_uint32 port, rtk_uint32 *enable);


#endif /*_RTL8367C_ASICDRV_EEE_H_*/
