 /*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __BSP_USB_H__
#define __BSP_USB_H__

#ifdef __cplusplus
extern "C" { /* allow C++ to use these headers */
#endif /* __cplusplus */


#include "osl_types.h"

#ifdef __KERNEL__
#include "mdrv_usb.h"
#endif

typedef enum
{
    USB_ENUM_DONE_CB_MSP = 0,
    USB_ENUM_DONE_CB_PS,
    USB_ENUM_DONE_CB_BSP,
    USB_ENUM_DONE_CB_BOTTEM
}USB_ENUM_DONE_CB_E;

#define USB_ENABLE_CB_MAX 32
#define USB_CDEV_NAME_MAX 64

/* usb dbg module */
#define USB_DBG_NV  (NV_ID_DRV_USB_DBG)

#define USB_DBGMODU_GADGET  BIT(0)
#define USB_DBGMODU_DWC3    BIT(1)
#define USB_DBGMODU_CORE    BIT(2)
#define USB_DBGMODU_VENDOR  BIT(3)
#define USB_DBGMODU_OTG     BIT(4)
#define USB_DBGMODU_XHCI    BIT(5)
#define USB_DBGMODU_STORAGE BIT(6)

/*
 * charger type define
 */
#define USB_CHARGER_TYPE_HUAWEI         1
#define USB_CHARGER_TYPE_NOT_HUAWEI     2
#define USB_CHARGER_TYPE_INVALID        0

/*usb pc driver*/

typedef enum usb_pc_driver
{
    JUNGO_DRIVER,
    HUAWEI_PC_DRIVER,
    HUAWEI_MODULE_DRIVER
} usb_pc_driver_t;

typedef enum usb_u1u2_enable
{
    USB_U1U2_DISABLE,
    USB_U1U2_ENABLE_WITH_WORKAROUND,
    USB_U1U2_ENABLE_WITHOUT_WORKAROUND
} usb_u1u2_enable_t;

/*0=pmu detect; 
1=no detect(for fpga); 
2=car module vbus detect, disconnect detect by chip, connect detect by gpio 
*/
typedef enum usb_vbus_detect_mode
{
    USB_PMU_DETECT,
    USB_NO_DETECT,
    USB_TBOX_DETECT
} usb_vbus_detect_mode_t;


typedef struct
{
    u32 dbg_module;   /* usb调试信息控制 */
}usb_dbg_nv_t;

typedef struct
{
    /* usb nv structure ... */
    usb_dbg_nv_t dbg_info;

    u32 nv_key;
    u32 use_nv;

    /* stat counter */
    u32 stat_nv_read_fail;
} usb_dbg_info_t;

extern usb_dbg_info_t g_usb_dbg_info;

#define USB_DBG_GADGET(d, fmt, args...)\
do {\
    if (g_usb_dbg_info.dbg_info.dbg_module & USB_DBGMODU_GADGET){\
        dev_dbg(d, fmt, ## args);\
    }\
} while (0)

#define USB_DBG_CORE(d, fmt, args...)\
do {\
    if (g_usb_dbg_info.dbg_info.dbg_module & USB_DBGMODU_CORE){\
        dev_dbg(d, fmt, ## args);\
    }\
} while (0)

#define USB_DBG_DWC3(d, fmt, args...)\
do {\
    if (g_usb_dbg_info.dbg_info.dbg_module & USB_DBGMODU_DWC3){\
        dev_dbg(d, fmt, ## args);\
    }\
} while (0)

#define USB_DBG_VENDOR(fmt, args...)\
do {\
    if (g_usb_dbg_info.dbg_info.dbg_module & USB_DBGMODU_VENDOR){\
        printk(fmt, ## args);\
    }\
} while (0)

#define USB_DBG_OTG(d, fmt, args...) \
do{\
    if (g_usb_dbg_info.dbg_info.dbg_module & USB_DBGMODU_OTG)\
        dev_dbg((d)->dev, "%s(): " fmt , __func__, ## args);\
}while(0)

#define USB_DBG_XHCI(xhci, fmt, args...) \
do {\
    if (XHCI_DEBUG && (g_usb_dbg_info.dbg_info.dbg_module & USB_DBGMODU_XHCI)) \
        dev_dbg(xhci_to_hcd(xhci)->self.controller , fmt , ## args); \
} while (0)

#define USB_DBG_STORAGE(x...) printk(KERN_DEBUG USB_STORAGE x )

typedef void (*usb_enum_done_cb_t)(void);


void bsp_usb_dbg_init(void);

int dwc3_phy_auto_powerdown(int enable);

int bsp_usb_netmode(void);

int bsp_usb_is_enable_u1u2_workaround(void);

int bsp_usb_is_ncm_bypass_mode(void);

int bsp_usb_is_vbus_connect(void);

int bsp_usb_vbus_detect_mode(void);



#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif /* __cplusplus */

#endif    /* End of __BSP_USB_H__ */
