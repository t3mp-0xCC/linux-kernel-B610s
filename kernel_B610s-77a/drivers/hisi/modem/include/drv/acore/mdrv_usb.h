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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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



#ifndef _MDRV_USB_H_
#define _MDRV_USB_H_


#include "product_config.h"

#ifdef _cplusplus
extern "C"
{
#endif

#ifdef __KERNEL__
#include <linux/notifier.h>
#endif

/* IOCTL CMD 定义 */
#define ACM_IOCTL_SET_WRITE_CB      0x7F001000
#define ACM_IOCTL_SET_READ_CB       0x7F001001
#define ACM_IOCTL_SET_EVT_CB        0x7F001002
#define ACM_IOCTL_SET_FREE_CB       0x7F001003

#define ACM_IOCTL_WRITE_ASYNC       0x7F001010
#define ACM_IOCTL_GET_RD_BUFF       0x7F001011
#define ACM_IOCTL_RETURN_BUFF       0x7F001012
#define ACM_IOCTL_RELLOC_READ_BUFF  0x7F001013
#define ACM_IOCTL_SEND_BUFF_CAN_DMA 0x7F001014

#define ACM_IOCTL_IS_IMPORT_DONE    0x7F001020
#define ACM_IOCTL_WRITE_DO_COPY     0x7F001021

#define ACM_MODEM_IOCTL_SET_MSC_READ_CB 0x7F001030
#define ACM_MODEM_IOCTL_MSC_WRITE_CMD   0x7F001031
#define ACM_MODEM_IOCTL_SET_REL_IND_CB  0x7F001032

/* 异步数据收发结构 */
typedef struct tagACM_WR_ASYNC_INFO
{
    char *pVirAddr;
    char *pPhyAddr;
    unsigned int u32Size;
    void* pDrvPriv;
}ACM_WR_ASYNC_INFO;

/* ACM设备事件类型 */
typedef enum tagACM_EVT_E
{
    ACM_EVT_DEV_SUSPEND = 0,        /* 设备不可以进行读写(主要用于事件回调函数的状态) */
    ACM_EVT_DEV_READY = 1,          /* 设备可以进行读写(主要用于事件回调函数的状态) */
    ACM_EVT_DEV_BOTTOM
}ACM_EVT_E;

typedef enum tagACM_IOCTL_FLOW_CONTROL_E
{
    ACM_IOCTL_FLOW_CONTROL_DISABLE = 0,      /* resume receiving data from ACM port */
    ACM_IOCTL_FLOW_CONTROL_ENABLE      /* stop receiving data from ACM port */
}ACM_IOCTL_FLOW_CONTROL_E;

/* 读buffer信息 */
typedef struct tagACM_READ_BUFF_INFO
{
    unsigned int u32BuffSize;
    unsigned int u32BuffNum;
}ACM_READ_BUFF_INFO;


typedef void (*ACM_WRITE_DONE_CB_T)(char *pVirAddr, char *pPhyAddr, int size);
typedef void (*ACM_READ_DONE_CB_T)(void);
typedef void (*ACM_EVENT_CB_T)(ACM_EVT_E evt);
typedef void (*ACM_FREE_CB_T)(char* buf);
typedef void (*ACM_MODEM_REL_IND_CB_T)(unsigned int bEnable);

typedef void (*USB_UDI_ENABLE_CB_T)(void);
typedef void (*USB_UDI_DISABLE_CB_T)(void);


/* 设备枚举最大端口个数 */
#define DYNAMIC_PID_MAX_PORT_NUM        17
#define USB_NUM_19    19
typedef struct 
{
    unsigned long nv_status;
    unsigned char first_config_port[DYNAMIC_PID_MAX_PORT_NUM];
    unsigned char second_config_port[DYNAMIC_PID_MAX_PORT_NUM];
    unsigned char third_config_port[DYNAMIC_PID_MAX_PORT_NUM];
    unsigned char reserved_1[DYNAMIC_PID_MAX_PORT_NUM];
    unsigned char reserved_2[DYNAMIC_PID_MAX_PORT_NUM];
    unsigned char reserved_3[USB_NUM_19];
} DRV_HUAWEI_MULTI_CONFIG_PORT_TYPE;

#if (FEATURE_ON == MBB_USB_SOLUTION)
/*****************************************************************************
函数名：   usb_port_config_get
功能描述:  查询config对应的端口值
输入参数： 
输出参数： 
返回值：   0:    查询成功
           其他：查询失败
*****************************************************************************/
unsigned char* usb_port_config_get(unsigned char index);
#define DRV_USB_GET_CONFIG(index) usb_port_config_get(index)
/*****************************************************************************
函数名：   BSP_USB_PortTypeQuery
功能描述:  查询当前的设备枚举的端口形态值
输入参数： stDynamicPidType  端口形态
输出参数： stDynamicPidType  端口形态
返回值：   0:    查询成功
           其他：查询失败
*****************************************************************************/
unsigned int BSP_USB_PortTypeQuery(DRV_HUAWEI_MULTI_CONFIG_PORT_TYPE *pstDynamicPidType);
#define DRV_SET_PORT_QUIRY(a)   BSP_USB_PortTypeQuery(a)
#endif


/*****************************************************************************
 *  函 数 名  : mdrv_usb_reg_enablecb
 *  功能描述  : 协议栈注册USB使能通知回调函数
 *  输入参数  : pFunc: USB使能回调函数指针
 *
 *  输出参数  : 无
 *  返 回 值  :  0	       操作成功。
 *                           其他	操作失败。
 *
 ******************************************************************************/
unsigned int mdrv_usb_reg_enablecb(USB_UDI_ENABLE_CB_T pFunc);

/*****************************************************************************
 *  函 数 名  : mdrv_usb_reg_disablecb
 *  功能描述  : 协议栈注册USB去使能通知回调函数
 *  输入参数  : pFunc: USB去使能回调函数指针
 *
 *  输出参数  : 无
 *  返 回 值  :  0	       操作成功。
 *                           其他	操作失败。
 *
 ******************************************************************************/
unsigned int mdrv_usb_reg_disablecb(USB_UDI_DISABLE_CB_T pFunc);

#define USB_CAPABILITY_THREE BSP_USB_CapabilityThree
#define DRV_GET_USB_SPEED(a)   BSP_USB_GetSpeed(a)
#if (FEATURE_ON == MBB_USB)
#if (FEATURE_ON == MBB_USB_TYPEC)
#define DRV_GET_TYPEC_STATUS   usbc_get_cc_status
/*****************************************************************************
函数名：   usbc_get_cc_status
功能描述:  获取当前CC 插入状态
输入参数：无
返回值：   0:    未插入，1，CC1插入，2 CC2插入，3，ERROR
*****************************************************************************/
int usbc_get_cc_status(void);
#endif
/*****************************************************************************
函数名：   BSP_USB_GetSpeed
功能描述:  提供给上层查询设备USB速率接口
输入参数： char *buf
返回值：   0:    查询成功
           其他：查询失败
*****************************************************************************/
int BSP_USB_GetSpeed(unsigned char * buf);
/*****************************************************************************
函数名：   BSP_USB_CapabilityThree
功能描述:  查询设备USB工作模式接口
输入参数： char *
返回值：   
*****************************************************************************/
int BSP_USB_CapabilityThree(void);

/*****************************************************************************
 函 数 名  : BSP_USB_GetPortMode
 功能描述  : 获取端口形态模式，网关对接需求
 输入参数  : 。
 输出参数  :
 返 回 值  :
*****************************************************************************/
extern unsigned char BSP_USB_GetPortMode(char*PsBuffer, unsigned long*Length );
#define DRV_GET_PORT_MODE(PsBuffer, Length)    BSP_USB_GetPortMode(PsBuffer,Length)
/*****************************************************************************
 函 数 名  : BSP_USB_GetDiagModeValue
 功能描述  : 获得设备类型。
 输入参数  : 无。
 输出参数  : ucDialmode:  0 - 使用Modem拨号; 1 - 使用NDIS拨号; 2 - Modem和NDIS共存
              ucCdcSpec:   0 - Modem/NDIS都符合CDC规范; 1 - Modem符合CDC规范;
                           2 - NDIS符合CDC规范;         3 - Modem/NDIS都符合CDC规范
 返 回 值  : VOS_OK/VOS_ERR
*****************************************************************************/
extern int BSP_USB_GetDiagModeValue(unsigned char *pucDialmode,
                             unsigned char *pucCdcSpec);

#define DRV_GET_DIAG_MODE_VALUE(pucDialmode, pucCdcSpec)    \
                             BSP_USB_GetDiagModeValue(pucDialmode, pucCdcSpec)

/*****************************************************************************
函数名：   BSP_USB_MBIMSetMode
功能描述:  提供给上层切换MBIM端口工作模式的接口
输入参数： switch_idex   需要切换的端口形态
输出参数:  无
返回值：   无
*****************************************************************************/
void BSP_USB_MBIMSetMode(int switch_idex);
#define DRV_USB_MBIM_SET_MODE(switch_idex) BSP_USB_MBIMSetMode(switch_idex)

/*****************************************************************************
函数名：   BSP_USB_MBIMGetMode
功能描述:  提供给上层获得MBIM端口工作模式的接口
输入参数： 无
输出参数:  无
返回值：   无
*****************************************************************************/           
unsigned int BSP_USB_MBIMGetMode(void);
#define DRV_USB_MBIM_GET_MODE() BSP_USB_MBIMGetMode() 

#else
/*****************************************************************************
函数名：   BSP_USB_GetSpeed
功能描述:  提供给上层查询设备USB速率接口
输入参数： char *buf
返回值：   0:    查询成功
           其他：查询失败
*****************************************************************************/
static INLINE int BSP_USB_GetSpeed(unsigned char * buf)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return ERROR;
}
/*****************************************************************************
函数名：   BSP_USB_CapabilityThree
功能描述:  查询设备USB工作模式接口
输入参数： char *
返回值：   
*****************************************************************************/
static INLINE int BSP_USB_CapabilityThree(void)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return ERROR;
}

/*****************************************************************************
 函 数 名  : BSP_USB_GetPortMode
 功能描述  : 获取端口形态模式，网关对接需求，打桩。
 输入参数  : 。
 输出参数  :
 返 回 值  :
*****************************************************************************/
static inline unsigned char DRV_GET_PORT_MODE(char*PsBuffer, unsigned int*Length)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return 0;
}

static inline int DRV_GET_DIAG_MODE_VALUE(unsigned char *pucDialmode,
                                          unsigned char *pucCdcSpec)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return 0;
}

static inline void DRV_USB_MBIM_SET_MODE(int switch_idex)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return 0;
}

static inline unsigned int DRV_USB_MBIM_GET_MODE(void)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return 0;
}


#endif


#if (FEATURE_ON == MBB_USB)
typedef enum tagHW_USB_CB_E
{
    HWUSBCB_PM_REMOTE_WAKEUP_CTRL = 0,
    HWUSBCB_MAX , //必须放在最后
}HW_USB_CB_E;

/*远程唤醒开关控制函数指针类型*/
typedef void (*Pm_Remote_Wakeup_Ctrl_Func)(unsigned int);

typedef void* HW_USB_CB_FUNCPTR;
void mdrv_hw_usb_register_cb(HW_USB_CB_E type, HW_USB_CB_FUNCPTR func);

#endif /*#if (FEATURE_ON == MBB_USB)*/
#ifdef _cplusplus
}
#endif
#endif

