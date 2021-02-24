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
    DEVICE_ID_ETH          =   20,          //��Ϊ������LAN��Ϣ�ϱ�
    /* ��Ϊ����������أ���Ҫ��atpv2����һ�¡�atpv2 �ӿ��ĵ��й涨 DEVICE_ID_TRAFFIC Ϊ28 */
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
	USB_SAMBA_PRINT_ATTACH = 11, /* Samba ��ӡ�������¼� */
	USB_SAMBA_PRINT_DETACH = 12, /* Samba ��ӡ���γ��¼� */	
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
    CRADLE_INVAILD = 0,                        	/*��Чֵ*/
    CRADLE_INSERT = 1,                         	/*���߲����¼�*/
    CRADLE_REMOVE = 2,                         	/*���߰γ��¼�*/
    CRADLE_PPPOE_UP = 3,                       	/*���ΪPPPoEģʽʱ�ϱ����¼�*/
    CRADLE_DYNAMICIP_UP = 4,                   	/*���Ϊ��̬IPģʽʱ�ϱ����¼�*/
    CRADLE_STATICIP_UP = 5,                    	/*���Ϊ��̬IPģʽʱ�ϱ����¼�*/
    CRADLE_PPPOE_DYNAMICIP_FAIL = 6,   			/*�û�����ΪPPPOE + ��̬IPģʽʱ�����ʧ���ϱ����¼�*/
    CRADLE_LAN_UP = 7,                          /*���ΪLANģʽʱ�ϱ����¼�*/
    CRADLE_MUTI_CAST = 8,             			/*��⵽�鲥����֪ͨӦ��*/
    CRADLE_POWEROFF_EN = 9,                     /*����eth phy�µ�*/
    CRADLE_POWEROFF_DIS = 10,                   /*��ֹeth phy�µ�*/
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
    /* ��ʱ��ʱ�䵽�¼��ϱ� */
    DEVICE_TIMEROU_F = 0, 
    /*��ʱ���ϱ�����¼���*/
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


/* �� �� �� �� ID */
typedef enum
{
    NL_DEVICE_ID_NULL_ID = 0,     /* ��id�����ڳ�ʼ��id */ 
    NL_DEVICE_ID_ATNLPROXT = 1,         /* DEVICE_ID_ATNLPROXT id */
    NL_DEVICE_ID_MAX_ID           /* �����߽籣�� */
}NL_DEVICE_ID;

/*�� ������ */
typedef enum _NL_MSG_EVENT
{
    NL_EVENT_MIN = -1,      /* �¼��߽�ֵ */
    NL_CONNECT_TO_KERNEL = 0,  /*�����ں�*/  
    NL_NORMALRESPONSE = 1,       /*������AT�ظ�*/
    NL_EVENT_MAX            /* �¼��߽�ֵ */
}NL_MSG_EVENT;

#define NL_MAX_AT_LEN 64
#define NL_AT_PARA_MAX_LEN                         (560)  /* +CGLA�������ַ����������Ϊ532 */
#define NL_AT_MAX_PARA_NUMBER                      (16)

typedef struct
{
    unsigned int ulParaValue;
    unsigned char  aucPara[NL_AT_PARA_MAX_LEN + 1];     /* �������ý������Ĳ����ַ��� */
    unsigned short usParaLen;                        /* ������ʶ�����ַ������� */
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
        /*�ܵ���PCUI�ڵȲο�AT_CLIENT_ID_ENUMö�ٱ������󲿷�ҵ�񲻹��Ĵ��ֶ�*/
        int m_uindex;
        int m_magic; 
        /*ħ���֣����������ã�1�ڴ�AT�����У����д�ħ���ֵ�ATS������
                                                                   ��Ϣ������ͨ������ͨ��
                                                                   2.�����״̬���ڵȴ�����ͬħ���ֵĻظ�*/
        /*���ã���ѯ�����ԣ��ж�*/
        int m_type;
        /*�ο��ں˵�AT_CMD_OPT_TYPE���壬Ϊ���������������Ƿ��=��*/
        unsigned char m_opt_type;
        /*�ο�MAX_MATCH_SIZE 64*/
        char m_atcmd[NL_MAX_AT_LEN];
        /*��������*/
        int m_para_count; 
        /*����*/
        NL_AT_PARSE_PARA_TYPE_STRU m_para_list[NL_AT_MAX_PARA_NUMBER];
}NLTRANSFER;

typedef enum _AT_NL_PROTOCOL_TYPE
{
    /*֪ͨ�ں˴�AT�ϱ�Ϊ���ض��˿��ϱ��������ϱ����˿���m_uindex��ָ��*/
    AT_NL_PROTOCOL_FIX_UNSOLIDREPORT = 1,
    /*֪ͨ�ں˴�UART�ϵ������ϱ����޸�NV 9202 ��ӦλΪ 4*/
    AT_NL_PROTOCOL_OPEN_UART_UNSOLIDREPORT,
    /*֪ͨ�ں˹ر�UART�ϵ������ϱ����޸�NV 9202 ��ӦλΪ 0*/
    AT_NL_PROTOCOL_CLOSE_UART_UNSOLIDREPORT,
    /*֪ͨ�ں˸���GPS״̬Ϊ����״̬*/
    AT_NL_PROTOCOL_GPS_RUNNING_UNSOLIDREPORT,
    /*֪ͨ�ں˸���GPS״̬Ϊ�ر�״̬*/
    AT_NL_PROTOCOL_GPS_STOP_UNSOLIDREPORT
}AT_NL_PROTOCOL_TYPE;


/*4 * sizeof(int)�������ĸ������ĳ��Ƚṹ����ˣ�Ҫ���������*/
#define FIX_ATNL_RESPONSE_LEN (6 * sizeof(int))
#define MAX_ATNL_RESPONSE_LEN  (FIX_NL_DATA_LENGTH - FIX_ATNL_RESPONSE_LEN - sizeof(NL_DEVICE_EVENT))
typedef struct _NLRESPONSE
{
        /*�ܵ���PCUI�ڵȲο�AT_CLIENT_ID_ENUMö�ٱ������󲿷�ҵ�񲻹��Ĵ��ֶ�*/
        int m_uindex;
        /*����ֶ���ATproxy US�Զ����*/
        int m_magic;
        
        /*Ӧ�ò���ں�֮���н�������ͨ������ֶα������壬ʹ��AT_NL_PROTOCOL_TYPE��ֵ*/
        int m_protocol;
        
        /*��Ϣ��ʶ����Դ����ģ���ж��òμ�HUAWEI_PM_RSP_ID_UR_IPDATA*/
        int m_id;
        /*�����ں˵�AT���أ����m_length == 0�����ֵ������*/
        int  m_retenum;
        /*���������m_buffer�е���Ч���ݳ���*/
        int m_length;
        /*response����*/
        char m_buffer[MAX_ATNL_RESPONSE_LEN];
}NLRESPONSE;
#endif

#endif

