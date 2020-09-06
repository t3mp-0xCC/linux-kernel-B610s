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
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/netlink.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/netlink.h>

#include "mbb_drv_bip.h"

/*BIP Buffer 最大长度*/
#define BIP_MAX_BUFFER_LENGTH               1500

/*BIP APN 最小长度*/
#define BIP_MIN_NETWORK_ACCESS_NAME_LENGTH  1

/*BIP APN 最大长度*/
#define BIP_MAX_NETWORK_ACCESS_NAME_LENGTH  99

/*BIP IP地址 最大长度*/
#define BIP_MAX_IPADDRESS_LENGTH            16

/*BIP 拨号用户名 最大长度*/
#define BIP_MAX_DIAL_USERNAME_LEN           255

/*BIP 拨号密码 最大长度*/
#define BIP_MAX_DIAL_PASSWORD_LEN           255

/*BIP 协议定义 Channel Data最大长度*/
#define BIP_MAX_CHANNEL_DATA_LENGTH         255

/*定义BIP Client和BIP设备 之间 通信的设备ID*/
#define DEVICE_ID_BIP                       17

typedef enum
{
    BIP_NTO_CREATE_SOCKET =  1,
    BIP_TO_CREATE_SOCKET =  2,
    BIP_HAVE_CREATED_SOCKET = 3,
    BIP_CREATE_SOCKET_STATE_BUTT
} BIP_SOCKET_STATE_EN;


/*BIP Client和 BIP设备通信 消息枚举*/   
typedef enum
{ 
    UICC_INFORM = -1,/*通知BIP Client 取数据*/
    OTA_TO_UICC = 0, /*BIP Client 写入 BIP字符设备 数据*/
    UICC_TO_OTA = 1, /*BIP Client 读取 BIP字符设备 数据*/
    UICC_COMPLETE = 0xff /*BIP业务完成*/
}BIP_EVENT;


#define BIP_SOCKET_CREATE_NOK   0
#define BIP_SOCKET_CREATE_OK     1

#ifndef MBB_BASIC_TYPE_DEF
#define MBB_BASIC_TYPE_DEF
typedef void VOSM_VOID;
typedef char VOSM_INT8;
typedef unsigned char VOSM_UINT8;
typedef short VOSM_INT16;
typedef unsigned short VOSM_UINT16;
typedef int VOSM_INT32;
typedef unsigned int VOSM_UINT32;
typedef long VOSM_LONG;
typedef unsigned long VOSM_ULONG;

#define VOSM_OK                 (0)
#define VOSM_ERROR          (-1)
#endif

/*PACKET DATA Structure to find BIP Data */ 
typedef struct
{  
    VOSM_UINT32               BipServerAddr;
    VOSM_UINT16               BipServerPort;
    VOSM_UINT16               BipClientPort;
    VOSM_UINT8                 BipSocketStatus;
}BIP_PACKET_HEADER;
/**************************************************************************
  函数声明
**************************************************************************/


static VOSM_INT32 BipDeviceOpen(struct inode *inode, struct file *file);


static VOSM_INT32 BipDeviceRelease(struct inode *inode, struct file *file);


static VOSM_LONG BipDeviceIoctl(struct file *file, VOSM_UINT32  cmd, VOSM_ULONG arg);



VOSM_INT32 BipDeviceInit(VOSM_VOID);


VOSM_VOID __init BipClientRegisterFuncInit(VOSM_VOID);


VOSM_INT32 BipDeviceReceiveDataFromBipClient( BIP_Command_Event_STRU *pDst);


VOSM_INT32 BipDeviceSendDataToModem( BIP_Command_Event_STRU *stru);


VOSM_INT32 BipDeviceReceiveDataFromModem(VOSM_VOID *pMsgBody, VOSM_UINT32 u32Len);


VOSM_INT32 BipDeviceNotifyDataToBipClient( BIP_Command_Event_STRU *pDst, BIP_EVENT event_code);


VOSM_VOID BipDeviceShowInfo (BIP_Command_Event_STRU *Data);

#if (FEATURE_ON == MBB_FEATURE_BIP_TEST)


VOSM_VOID BipClientTestOpenChannel(VOSM_UINT8 ResultValue);


VOSM_VOID BipClientTestCloseChannel(VOSM_VOID);


VOSM_VOID BipClientTestSendData(VOSM_UINT8 dataLen);


VOSM_VOID BipClientTestReceiveData( VOSM_UINT8 dataLen, VOSM_UINT8 LeftDataLen );


VOSM_VOID BipClientTestGetChannelStatus(VOSM_UINT8 ChannelStatus);


VOSM_VOID BipClientTestGetChannelStatusFail(VOSM_VOID);


VOSM_VOID BipClientTestChannelStatusEvent(VOSM_UINT8 ChannelStatus);
#endif /*(FEATURE_ON == MBB_FEATURE_BIP_TEST)*/


