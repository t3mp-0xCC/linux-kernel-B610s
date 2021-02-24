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
#ifndef __ATP_KCMSMAIN_MSGTYPES_H__
#define __ATP_KCMSMAIN_MSGTYPES_H__

#ifndef ATP_MSG_CATEGORY_DEF
#define ATP_MSG_CATEGORY_DEF(x)          (((x) & 0xFFF) << 8)
#endif

enum ATP_KCMSMAIN_MSGTYPES_EN
{
    ATP_MSG_KCMSMAIN_START = ATP_MSG_CATEGORY_DEF(165),
    ATP_MSG_MONITOR_EVT_RESET,      /*�ָ��������ð���*/
    ATP_MSG_MONITOR_EVT_REBOOT,      /*�������ذ���*/    
    ATP_MSG_MONITOR_EVT_DSL_UP,         /*dsl line0 link up msg*/
    ATP_MSG_MONITOR_EVT_DSL_DOWN,       /*dsl line0 link down msg*/
    ATP_MSG_MONITOR_EVT_DSL_TRAINING,   /*dsl line0 training msg*/
    ATP_MSG_MONITOR_EVT_DSL_INIT,       /*dsl line0 ini msg*/
    ATP_MSG_MONITOR_EVT_LAN_ETH,
    ATP_MSG_MONITOR_EVT_ETH_MULTICAST_UPGRADE,
    ATP_MSG_MONITOR_EVT_ETH_CRADLE,    /*��̫������*/
    ATP_MSG_MONITOR_EVT_DATACARD_START,       
    ATP_MSG_MONITOR_EVT_DATACARD_STOP, 
    ATP_MSG_MONITOR_EVT_WLAN,          /*WLAN����*/
    ATP_MSG_MONITOR_EVT_WPS,           /*WPS����*/
    ATP_MSG_MONITOR_EVT_HILINK_OKC_ENABLE,     
    ATP_MSG_MONITOR_EVT_HILINK_OKC_STA_ENABLE,
    ATP_MSG_MONITOR_EVT_HILINK_OKC_LEDA,     
    ATP_MSG_MONITOR_EVT_HILINK_OKC_LEDB,    
    ATP_MSG_MONITOR_EVT_HILINK_OKC_LEDOFF, 
    ATP_MSG_MONITOR_EVT_WLAN_5G,       /*WLAN 5G����*/
    ATP_MSG_MONITOR_EVT_WPS_5G,        /*WPS 5G����*/
    ATP_MSG_MONITOR_EVT_DECT_PP,    /*DECT���밴��*/
    ATP_MSG_MONITOR_EVT_DECT_SEARCH,    /*DECT���Ұ���*/
    ATP_MSG_MONITOR_EVT_LOG,
    ATP_MSG_MONITOR_EVT_IPTV_START,    
    ATP_MSG_MONITOR_EVT_IPTV_STOP,  
    ATP_MSG_MONITOR_EVT_RTSP_START,    
    ATP_MSG_MONITOR_EVT_RTSP_STOP,         
    ATP_MSG_MONITOR_EVT_IPP_START,       
    ATP_MSG_MONITOR_EVT_IPP_STOP,   
    ATP_MSG_MONITOR_EVT_DYING_GASP,  /*Dying Gasp�����¼�*/ 	
    ATP_MSG_MONITOR_EVT_USBSTORAGE_PLUGIN,
    ATP_MSG_MONITOR_EVT_USBSTORAGE_PLUGOUT,
    ATP_MSG_MONITOR_EVT_WLANRF,      /*���߲�����д�¼�*/
    ATP_MONITOR_EVT_SERVLED_STOP,    /*Service LED stop*/
    ATP_MSG_MONITOR_EVT_WLAN_BAND_CHG,	   /*WLAN 2.4 G/5G �л�����*/
    ATP_MSG_MONITOR_EVT_WLAN_MODE_CHG,	   /*WLAN ģʽ �л�����*/
    ATP_MSG_MONITOR_EVT_WPS_SUCCESS,       /* WPS �ɹ�֮���ϱ� */
    ATP_MSG_MONITOR_EVT_LAN_DEVICE_UP,     /*ipv6��LAN���豸���ӵ����ط���DAD����*/
    ATP_MSG_MONITOR_EVT_MIDWARE_BUTTONPRESS, /*WPS/WLAN�����¼��ϱ����м��*/
    ATP_MSG_MONITOR_EVT_DSL1_UP,         /*dsl line1 link up msg*/
    ATP_MSG_MONITOR_EVT_DSL1_DOWN,       /*dsl line1 link down msg*/
    ATP_MSG_MONITOR_EVT_DSL1_TRAINING,   /*dsl line1 training msg*/
    ATP_MSG_MONITOR_EVT_DSL1_INIT,       /*dsl line1 ini msg*/
    ATP_MSG_MONITOR_EVT_USBERROR_HANDLE, /*USB������*/
	ATP_MSG_MONITOR_EVT_WM_START, /* ����ģ�鿪�� */
	ATP_MSG_MONITOR_EVT_WM_STOP,  /* ����ģ��Ͽ� */
    ATP_MSG_MONITOR_EVT_RCV_PADT,
    ATP_MSG_MONITOR_EVT_WIFISAFEKEY_ON,       /*wifisafekey on*/
    ATP_MSG_MONITOR_EVT_WIFISAFEKEY_OFF,      /*wifisafekey off*/
    ATP_MSG_MONITOR_EVT_WIFIPOWERSAVE_ON,     /*wifipowersave led on*/
    ATP_MSG_MONITOR_EVT_WIFIPOWERSAVE_OFF,    /*wifipowersave led off*/    
    ATP_MSG_MONITOR_EVT_WIFI_FBT_NOTIFY, /* WIFI FBT notify */
    ATP_MSG_MONITOR_EVT_WIFI_MULTICAST_NOTIFY, /* WIFI multicast notify */
    ATP_MSG_MONITOR_EVT_WIFI_STAMAC_NOTIFY,
    ATP_MSG_MONITOR_EVT_COREDUMP_FINISH, /* ����coredump�ļ� */
    ATP_MSG_MONITOR_EVT_WLAN_LOG,               /*��¼WLAN LOG*/
    ATP_MSG_MONITOR_EVT_USB_RNDIS_UP,    /*USB����*/
    ATP_MSG_MONITOR_EVT_USB_RNDIS_DOWN,  /*USB����*/
    ATP_MSG_MONITOR_EVT_3GPP_ONDEMAND_CONNECT,  /*�����ϱ����貦��*/
    ATP_MSG_MONITOR_EVT_3GPP_ONDEMAND_DISCONN,  /*�����ϱ�����Ͽ�*/
    ATP_MSG_MONITOR_EVT_HOTA_UPG,               /*ϵͳһ����������*/
    ATP_MSG_MONITOR_EVT_STANDBY,         /*ϵͳ���߻��ѿ���*/

    /*netlink module*/
    ATP_KCMSMAIN_NETLINK_ROUTE,    /*Route Balance Module*/
    /* ��Ҫ������ֵ���Զ�����Ϊ42297�����û�̬�¼�id��һ�� */
    ATP_KCMSMAIN_NETLINK_CT_CLEAN = 42304, /*Conntrack Clean Module */
    ATP_KCMSMAIN_NETLINK_KLOG,      /*Kernel Log Module*/
    ATP_MSG_MONITOR_EVT_CIRCLE,     /* Circle���¼�*/
    ATP_MSG_MONITOR_EVT_ONE_STA_ASSOC,	   /* 1��STA����SSID��Ϣ */
    ATP_MSG_MONITOR_EVT_ALL_STA_DISASSOC,  /* ����STAȥ����SSID��Ϣ */

    /*** ������netlink�¼�ID����ŵ�����棬����Ӱ��MBB��Ʒnetlink�¼������̱�� ***/
    ATP_MSG_MONITOR_EVT_NEW_DEVICE_ONLINE, /*�µ��豸��fdb�д����˱���*/
    ATP_KCMSMAIN_NETLINK_USB_DEVICE_DEL,  /*ɾ����Ҫж�ص�USB�豸*/
    ATP_MSG_WIFI_CHANNEL_NOTIFY, /* WIFI channel notify */
    ATP_MSG_MONITOR_EVT_HILINK_OKC_DEVIN,      /* ���豸���� */
    ATP_MSG_MONITOR_EVT_HILINK_OKC_DEVOUT,     /* ���豸�˳�*/
    ATP_MSG_MONITOR_EVT_HILINK_OKC_DEVUPDATE,  /* ���豸����*/
    ATP_MSG_MONITOR_EVT_HILINK_OKC_DEVCLEAN,   /* ���豸���*/
	ATP_MSG_MONITOR_EVT_GPIO_TIMER,   /* gpioӲ����ʱ��*/

	ATP_MSG_MONITOR_EVT_HILINK_OKC_LAUNCH_MCAST, /* HiLink OKC: �����鲥��Ϣ */
	ATP_MSG_MONITOR_EVT_HILINK_OKC_LAUNCH_PLC,   /* HiLink OKC: ����PLC��Ϣ */
	ATP_MSG_MONITOR_EVT_HILINK_OKC_LAUNCH_WPS,   /* HiLink OKC: ����WPS��Ϣ */
};


enum ATP_KCMSMAIN_NETLINK_MODULE_EN
{
    ATP_KCMSMAIN_NETLINK_NAT = 0xFF01,
    ATP_KCMSMAIN_NETLINK_DPI = 0xFF02,
    ATP_KCMSMAIN_NETLINK_HTTPMASQ = 0xFF04,
};

#define BR_FDB_DEVICE_ONLINE 0
#define BR_FDB_DEVICE_OFFLINE 1
#define ETH_ALEN 6

/*�ں˺��û�̬ͨ�ŵ�netlinkЭ������*/
/*�ں˺��û�̬ͨ�ŵ�netlinkЭ������*/
/*ATP ��ǰʹ��20 -21���Ѿ���3.4�ں˳�ͻ�����»���ATPʹ��22 - 26*/
#define NETLINK_SYSWATCH                25
#define NETLINK_ATP_CONSOLE             26 /* send console printk to userspace */

#define SYSWATCH_USERSPACE_GROUP    31
#define KERNEL_PID      0
#define NETLINK_MSG_CMD_LEN         64


#define KLOG_STR_LEN                    256
#define KLOG_IP_LEN                     64

#define ATP_ATTACKE_TYPE_OTHER      0
#define ATP_ATTACKE_TYPE_UDPDETECT  1
#define ATP_ATTACKE_TYPE_SMURF      2

/* �������豸�ϱ���Ϣ���ȶ��� */
#define HILINK_NEW_MAC_LEN                    (6)
#define HILINK_NEW_DEVICE_INFO_LEN            (7)
#define HILINK_NEW_DEVICE_VERSION_LEN         (1)
#define HILINK_NEW_DEVICE_MANUFACT_LEN        (3)
#define HILINK_NEW_DEVICE_TYPE_LEN            (3)
/* ��������SSID����Ҫ���޶� */
#define HILINK_NEW_DEVICE_SN_LEN              (3)
#define HILINK_NEW_DEVICE_CAP_LEN             (2)
#define HILINK_NEW_DEVICE_HILINKCERTID_LEN    (10)

/* �����������͸�WLANģ��ʹ�� */
/* ProbeIE + WPS ���������� */
#define HILINK_NEW_DEVICE_CONN_TYPE_WPS 		(0)
/* SSID + �鲥 ���������� */
#define HILINK_NEW_DEVICE_CONN_TYPE_MCAST 		(1)
/* SSID + PLC ���������� */
#define HILINK_NEW_DEVICE_CONN_TYPE_PLC 		(2)

typedef struct tagFW_KLOG_INFO_ST
{
    int             lType;
    char            acAttackSrc[KLOG_IP_LEN];
}FW_KLOG_INFO_ST;

typedef struct br_fdb_msg
{
    unsigned char macaddr[ETH_ALEN];
    int           msgtype;
}BR_FDB_MSG_ST;

// this is the msg struct that being sent between the kernel and userspace
struct  generic_nl_msg
{
    unsigned int len; 		// the length of the "data" field
    unsigned int comm;  	// which kind of msg, referring to 'syswatch_msg_comm'
    unsigned char data[0]; 	// if the 'comm' needs to add more payload, put it here
};

typedef struct 
{
    unsigned int logType;
    unsigned int logLevel;
    unsigned int logNum;
}netlink_log_header;

/*�ں�֪ͨ�û�̬�����¼���Ϣ�ṹ��*/
typedef struct netlink_common_msg_st
{
    unsigned int eventType;
    unsigned int eventResult;
    unsigned int eventPortNum;
    char         acPortName[NETLINK_MSG_CMD_LEN];
}netlink_common_msg;

typedef enum FBT_NOTIFY_TYPE {
	FBT_LINK_WEAK_NOTIFY 		= 0,
    FBT_LINK_STRONG_NOTIFY = 1,
	FBT_STA_ONLINE_NOTIFY 	= 2,
	FBT_STA_OFFLINE_NOTIFY 	= 3, 
	FBT_STA_FOUND_NOTIFY 	= 4
} FBT_NOTIFY_E;

typedef struct  {
    unsigned int FbtWlanInstID;
    unsigned int  FbtChannel;
    unsigned int  FbtRSSI;
    FBT_NOTIFY_E  FbtNotifyType;
    char  FbtNotifyMac[20];
} FBT_NOTIFY_ST;

typedef struct  {
	char  ssid[36];
    char  StaMac[20];
} ASSOCI_NOTIFY_ST;

/* �������豸�ϱ���Ϣ���Ͷ��� */
typedef struct  {
    unsigned char  acDevMac[HILINK_NEW_MAC_LEN];                      /* ��ǰ�豸�����mac��ַ */    
    unsigned int ulRssi;                                              /* ��ǰ�����豸���ź�ǿ�� */   
	unsigned int connType;											  /* ��ǰ�����豸���������� */
    unsigned char acInfo[HILINK_NEW_DEVICE_INFO_LEN];                 /* ������ʽ��ͷ */  
    unsigned char acVersion[HILINK_NEW_DEVICE_VERSION_LEN];           /* ��ǰЭ��ԽӰ汾�� */  
    unsigned char acManu[HILINK_NEW_DEVICE_MANUFACT_LEN];             /* ��ǰ�豸����ID */  
    unsigned char acDevicetype[HILINK_NEW_DEVICE_TYPE_LEN];           /* ��ǰ�豸���� */ 
    unsigned char acDeviceSN[HILINK_NEW_DEVICE_SN_LEN];               /* ��ǰ�豸SN ����λ */ 
    unsigned char acDeviceCap[HILINK_NEW_DEVICE_CAP_LEN];               /* ��ǰ�豸������ */ 
    unsigned char acHiLinkCertID[HILINK_NEW_DEVICE_HILINKCERTID_LEN + 1];   /* ��ǰ�豸��HiLink��֤ID���䳤���12λ */ 
} HILINK_OKC_NOTIFY_ST;


/*kernel netlink handler*/
typedef int (* PFNetlinkMsgProc)(unsigned short usModuleId, void *pvData, unsigned int ulDataLen);

// Global variable to indicate whether the userspace netlink socket has created,
// and send the monitored processes' names to the kernel
extern struct sock *syswatch_nl_sock;

//extern int syswatch_nl_init(void);
extern int syswatch_nl_send(unsigned int type, unsigned char *buf, unsigned int len);

extern int syswatch_sendLog(unsigned int logType, unsigned int logLevel, unsigned int logNum, unsigned char *str);

extern void dad_skb_send_up(void *pskb, void *phdr);

extern int ATP_Netlink_SendToUserspace(unsigned short usModuleId, void *pvData, unsigned int ulDataLen);

extern int ATP_Netlink_Register(unsigned short ulModuleId, PFNetlinkMsgProc pfMsgProc);

extern int ATP_Netlink_Unregister(unsigned short ulModuleId);

#endif // End of __ATP_KCMSMAIN_MSGTYPES_H__

