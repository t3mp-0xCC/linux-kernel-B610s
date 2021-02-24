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



#ifndef __MDRV_ACORE_MBB_CHANNEL_H__
#define __MDRV_ACORE_MBB_CHANNEL_H__

#define NETLINK_HW_LOGCAT   29
#include <product_config.h>

#if (FEATURE_ON == MBB_COMMON)
#define TIMER_ID    15
#endif /*#if (FEATURE_ON == MBB_COMMON)*/

typedef enum _DEVICE_ID
{	
#if (FEATURE_ON == MBB_COMMON)
    DEVICE_ID_NULL_ID = 0,    /*NULL_ID, as initialize value.*/ 
    DEVICE_ID_USB = 1,  // USB device = 1
    DEVICE_ID_KEY,      // Keyboard device = 2 
    DEVICE_ID_BATTERY,  // Battery device = 3
    DEVICE_ID_CHARGER,  // Charger device = 4
    DEVICE_ID_SD,       // SD card device =  5
    DEVICE_ID_PROCESS,  // Used for monitored process = 6
    DEVICE_ID_SCREEN,   // Screen device = 7
    DEVICE_ID_WLAN, // WLAN device = 8
    DEVICE_ID_OM,   // OM module = 9
    DEVICE_ID_TEMP, // Temperature = 10
#endif /*#if (FEATURE_ON == MBB_COMMON)*/
	DEVICE_ID_WAN = 11,       // WAN module = 11 
#if (FEATURE_ON == MBB_COMMON)
    DEVICE_ID_SIM,  //SIM device = 12
    DEVICE_ID_UPDATEONLINE,  //Used for update online ;Add fansaihua
    DEVICE_ID_TIMER =  TIMER_ID, 
#if (FEATURE_ON == MBB_FEATURE_ETH)
    DEVICE_ID_CRADLE    =   16, 
    DEVICE_ID_ETH          =   20,          //作为多网口LAN消息上报
    /* 作为网口流量监控，需要与atpv2保持一致。atpv2 接口文档中规定 DEVICE_ID_TRAFFIC 为28 */
    DEVICE_ID_TRAFFIC = 28,
#endif
#if ( FEATURE_ON == MBB_HSUART_AT )
    DEVICE_ID_HSUART = 26, /*Hsuart device*/
#endif/*MBB_HSUART_AT*/
#endif /*#if (FEATURE_ON == MBB_COMMON)*/	
    DEVICE_ID_MAX	
} DEVICE_ID;

// Define the device event structure (driver => APP) 
typedef struct _DEVICE_EVENT	
{
	DEVICE_ID device_id;
	int event_code;
	int len;
	char data[0];
} DEVICE_EVENT;

#if (FEATURE_ON == MBB_COMMON)
typedef enum _USB_EVENT
{
	USB_ATTACH = 0,
	USB_REMOVE,
	USB_ENABLE,
	USB_DISABLE,
	USB_INIT,
	USB_HALT,
	USB_RESET,
	USB_SUSPEND,
	USB_RESUME,
#if (FEATURE_ON == MBB_USB)
	USB_SAMBA_PRINT_ATTACH = 11, /* Samba 打印机插入事件 */
	USB_SAMBA_PRINT_DETACH = 12, /* Samba 打印机拔出事件 */	
#endif
	USB_EVENT_MAX
} USB_EVENT;

typedef enum _SD_EVENT
{
	SD_ATTATCH = 0,
	SD_REMOVE,
#if (FEATURE_ON == MBB_USB)
	U_DISK_ATTATCH,     /* Plug U-Disk in to OTG-USB */
	U_DISK_REMOVE,      /* Unplug U-Disk from OTG-USB */
#endif
	SD_EVENT_MAX
} SD_EVENT;
typedef enum _CRADLE_EVENT
{
    CRADLE_INVAILD = 0,                        	/*无效值*/
    CRADLE_INSERT = 1,                         	/*网线插入事件*/
    CRADLE_REMOVE = 2,                         	/*网线拔出事件*/
    CRADLE_PPPOE_UP = 3,                       	/*检测为PPPoE模式时上报该事件*/
    CRADLE_DYNAMICIP_UP = 4,                   	/*检测为动态IP模式时上报该事件*/
    CRADLE_STATICIP_UP = 5,                    	/*检测为静态IP模式时上报该事件*/
    CRADLE_PPPOE_DYNAMICIP_FAIL = 6,   			/*用户设置为PPPOE + 动态IP模式时，检测失败上报该事件*/
    CRADLE_LAN_UP = 7,                          /*检测为LAN模式时上报该事件*/
    CRADLE_MUTI_CAST = 8,             			/*检测到组播升级通知应用*/
    CRADLE_POWEROFF_EN = 9,                     /*允许eth phy下电*/
    CRADLE_POWEROFF_DIS = 10,                   /*禁止eth phy下电*/
    CRADLE_MAX = 0xFF
}CRADLE_EVENT;

typedef enum _ETH_TRAFFIC_EVENT
{
    ETH_TRAFFIC_INVAILD,
    ETH_TRAFFIC_BLINK,
    ETH_TRAFFIC_ON,  
    ETH_TRAFFIC_OFF,  
    ETH_TRAFFIC_MAX,  
}ETH_TRAFFIC_EVENT;

typedef enum _PROCESS_EVENT
{
	PROCESS_ADD_MONITORED = 0,
	PROCESS_RM_MONITORED,
	PROCESS_REPORT_EXIT,
	PROCESS_EVENT_MAX
} PROCESS_EVENT;

typedef enum _TIMER_EVENT
{ 
    /* 定时器时间到事件上报 */
    DEVICE_TIMEROU_F = 0, 
    /*定时器上报最大事件数*/
    DEVICE_TIMER_MAX
} TIMER_EVENT;

#endif /*#if (FEATURE_ON == MBB_COMMON)*/

// Define the device event handler (APP => driver) 
typedef int (*device_event_dispatch)(void *data, int len);

// Export interface to other module
#if (FEATURE_ON == MBB_COMMON)
extern int device_event_report(void *data, int len);
extern int device_event_handler_register(DEVICE_ID id, device_event_dispatch dispatcher);
#else
static inline int device_event_report(void *data, int len)
{
	return 0;
}
#endif
#if (FEATURE_ON == MBB_ATNLPROXY)
#ifndef FIX_NL_DATA_LENGTH
#define FIX_NL_DATA_LENGTH 10240
#endif


/* 事 件 设 备 ID */
typedef enum
{
    NL_DEVICE_ID_NULL_ID = 0,     /* 空id，用于初始化id */ 
    NL_DEVICE_ID_ATNLPROXT = 1,         /* DEVICE_ID_ATNLPROXT id */
    NL_DEVICE_ID_MAX_ID           /* 用做边界保护 */
}NL_DEVICE_ID;

/*事 件定义 */
typedef enum _NL_MSG_EVENT
{
    NL_EVENT_MIN = -1,      /* 事件边界值 */
    NL_CONNECT_TO_KERNEL = 0,  /*连接内核*/  
    NL_NORMALRESPONSE = 1,       /*正常的AT回复*/
    NL_EVENT_MAX            /* 事件边界值 */
}NL_MSG_EVENT;

#define NL_MAX_AT_LEN 64
#define NL_AT_PARA_MAX_LEN                         (560)  /* +CGLA命令中字符串长度最大为532 */
#define NL_AT_MAX_PARA_NUMBER                      (16)

typedef struct
{
    unsigned int ulParaValue;
    unsigned char  aucPara[NL_AT_PARA_MAX_LEN + 1];     /* 用来放置解析出的参数字符串 */
    unsigned short usParaLen;                        /* 用来标识参数字符串长度 */
}NL_AT_PARSE_PARA_TYPE_STRU;

typedef struct _NL_DEVICE_EVENT
{
    int device_id;
    int event_code;
    int len;
    char data[0];
}NL_DEVICE_EVENT;

typedef struct _NLTRANSFER
{
        /*管道如PCUI口等参考AT_CLIENT_ID_ENUM枚举变量，大部分业务不关心此字段*/
        int m_uindex;
        int m_magic; 
        /*魔术字，有两个作用：1在此AT处理中，带有此魔术字的ATS发下来
                                                                   消息，可以通过串化通道
                                                                   2.上面的状态机在等待有相同魔术字的回复*/
        /*设置，查询，测试，中断*/
        int m_type;
        /*参考内核的AT_CMD_OPT_TYPE定义，为了区分设置命令是否带=号*/
        unsigned char m_opt_type;
        /*参考MAX_MATCH_SIZE 64*/
        char m_atcmd[NL_MAX_AT_LEN];
        /*参数个数*/
        int m_para_count; 
        /*参数*/
        NL_AT_PARSE_PARA_TYPE_STRU m_para_list[NL_AT_MAX_PARA_NUMBER];
}NLTRANSFER;

typedef enum _AT_NL_PROTOCOL_TYPE
{
    /*通知内核此AT上报为往特定端口上报的主动上报，端口在m_uindex中指定*/
    AT_NL_PROTOCOL_FIX_UNSOLIDREPORT = 1,
    /*通知内核打开UART上的主动上报，修改NV 9202 相应位为 4*/
    AT_NL_PROTOCOL_OPEN_UART_UNSOLIDREPORT,
    /*通知内核关闭UART上的主动上报，修改NV 9202 相应位为 0*/
    AT_NL_PROTOCOL_CLOSE_UART_UNSOLIDREPORT,
    /*通知内核更新GPS状态为开启状态*/
    AT_NL_PROTOCOL_GPS_RUNNING_UNSOLIDREPORT,
    /*通知内核更新GPS状态为关闭状态*/
    AT_NL_PROTOCOL_GPS_STOP_UNSOLIDREPORT
}AT_NL_PROTOCOL_TYPE;


/*4 * sizeof(int)是上面四个整数的长度结构体变了，要改这个长度*/
#define FIX_ATNL_RESPONSE_LEN (6 * sizeof(int))
#define MAX_ATNL_RESPONSE_LEN  (FIX_NL_DATA_LENGTH - FIX_ATNL_RESPONSE_LEN - sizeof(NL_DEVICE_EVENT))
typedef struct _NLRESPONSE
{
        /*管道如PCUI口等参考AT_CLIENT_ID_ENUM枚举变量，大部分业务不关心此字段*/
        int m_uindex;
        /*这个字段由ATproxy US自动填充*/
        int m_magic;
        
        /*应用层和内核之间有交互，则通过这个字段标明含义，使用AT_NL_PROTOCOL_TYPE赋值*/
        int m_protocol;
        
        /*消息标识，电源管理模块判断用参见HUAWEI_PM_RSP_ID_UR_IPDATA*/
        int m_id;
        /*兼容内核的AT返回，如果m_length == 0，则此值有意义*/
        int  m_retenum;
        /*这个长度是m_buffer中的有效数据长度*/
        int m_length;
        /*response长度*/
        char m_buffer[MAX_ATNL_RESPONSE_LEN];
}NLRESPONSE;
#endif

#endif

