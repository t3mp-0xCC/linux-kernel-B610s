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
#ifndef _RTL8367C_ASICDRV_UNKNOWNMULTICAST_H_
#define _RTL8367C_ASICDRV_UNKNOWNMULTICAST_H_

#include <rtl8367c_asicdrv.h>

enum L2_UNKOWN_MULTICAST_BEHAVE
{
    L2_UNKOWN_MULTICAST_FLOODING = 0,
    L2_UNKOWN_MULTICAST_DROP,
    L2_UNKOWN_MULTICAST_TRAP,
    L2_UNKOWN_MULTICAST_DROP_EXCLUDE_RMA,
    L2_UNKOWN_MULTICAST_END
};

enum L3_UNKOWN_MULTICAST_BEHAVE
{
    L3_UNKOWN_MULTICAST_FLOODING = 0,
    L3_UNKOWN_MULTICAST_DROP,
    L3_UNKOWN_MULTICAST_TRAP,
    L3_UNKOWN_MULTICAST_ROUTER,
    L3_UNKOWN_MULTICAST_END
};

enum MULTICASTTYPE{
	MULTICAST_TYPE_IPV4 = 0,
	MULTICAST_TYPE_IPV6,
	MULTICAST_TYPE_L2,
	MULTICAST_TYPE_END
};

extern ret_t rtl8367c_setAsicUnknownL2MulticastBehavior(rtk_uint32 port, rtk_uint32 behave);
extern ret_t rtl8367c_getAsicUnknownL2MulticastBehavior(rtk_uint32 port, rtk_uint32 *pBehave);
extern ret_t rtl8367c_setAsicUnknownIPv4MulticastBehavior(rtk_uint32 port, rtk_uint32 behave);
extern ret_t rtl8367c_getAsicUnknownIPv4MulticastBehavior(rtk_uint32 port, rtk_uint32 *pBehave);
extern ret_t rtl8367c_setAsicUnknownIPv6MulticastBehavior(rtk_uint32 port, rtk_uint32 behave);
extern ret_t rtl8367c_getAsicUnknownIPv6MulticastBehavior(rtk_uint32 port, rtk_uint32 *pBehave);
extern ret_t rtl8367c_setAsicUnknownMulticastTrapPriority(rtk_uint32 priority);
extern ret_t rtl8367c_getAsicUnknownMulticastTrapPriority(rtk_uint32 *pPriority);

#endif /*_RTL8367C_ASICDRV_UNKNOWNMULTICAST_H_*/


