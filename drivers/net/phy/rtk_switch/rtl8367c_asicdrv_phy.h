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
#ifndef _RTL8367C_ASICDRV_PHY_H_
#define _RTL8367C_ASICDRV_PHY_H_

#include <rtl8367c_asicdrv.h>

#define RTL8367C_PHY_INTERNALNOMAX	    0x4
#define RTL8367C_PHY_REGNOMAX		    0x1F
#define RTL8367C_PHY_EXTERNALMAX	    0x7

#define	RTL8367C_PHY_BASE   	        0x2000
#define	RTL8367C_PHY_EXT_BASE   	    0xA000

#define	RTL8367C_PHY_OFFSET	            5
#define	RTL8367C_PHY_EXT_OFFSET  	    9

#define	RTL8367C_PHY_PAGE_ADDRESS       31


extern ret_t rtl8367c_setAsicPHYReg(rtk_uint32 phyNo, rtk_uint32 phyAddr, rtk_uint32 regData );
extern ret_t rtl8367c_getAsicPHYReg(rtk_uint32 phyNo, rtk_uint32 phyAddr, rtk_uint32* pRegData );
extern ret_t rtl8367c_setAsicPHYOCPReg(rtk_uint32 phyNo, rtk_uint32 ocpAddr, rtk_uint32 ocpData );
extern ret_t rtl8367c_getAsicPHYOCPReg(rtk_uint32 phyNo, rtk_uint32 ocpAddr, rtk_uint32 *pRegData );

#endif /*#ifndef _RTL8367C_ASICDRV_PHY_H_*/

