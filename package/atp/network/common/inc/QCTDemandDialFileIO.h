
#ifndef __RNICDEMANDDIALFILEIO_H__
#define __RNICDEMANDDIALFILEIO_H__

#include <linux/namei.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kallsyms.h>
#include <asm/uaccess.h>
#include <asm-generic/errno-base.h>
/*****************************************************************************
  1 ���������Լ��궨��
*****************************************************************************/
typedef void                   VOS_VOID;


typedef unsigned char          VOS_UINT8;

typedef signed char            VOS_INT8;

typedef char                   VOS_CHAR;           /* 8 bit ASCII */

typedef unsigned char          VOS_UCHAR;


typedef unsigned short         VOS_UINT16;

typedef signed   short         VOS_INT16;

typedef signed   short         VOS_WCHAR;          /* 16 bit Unicode */


typedef unsigned long          VOS_UINT32;

typedef signed   long          VOS_INT32;

typedef unsigned long          VOS_BOOL;

#define VOS_OK  0
#define VOS_ERR 1
#define VOS_NULL_PTR   NULL
#define ONDEMAND_CONNECT_FLUX_CHECK_TIME   1
#define DEFULT_RNIC_DISCONNECT_TIME   30

typedef enum
{
    ONDEMAND_DISABLE,                   /*�ǰ���*/
    ONDEMAND_DISCONN,                   /*��ǰ���ųɹ�����Ҫ�����·��ļ��ʱ���ضϿ����¼�*/                                                    
    ONDEMAND_CONNECT,                   /*��ǰ����״̬Ϊ�Ͽ�����Ҫ�����������������*/
}RNIC_ONDEMAND_TYPE_ENUM;


enum RNIC_DEMAND_DIAL_INFO_MSG_ID_ENUM
{
    /* AT����RNIC����Ϣö�� */
    ID_RNIC_DIAL_MODE_INFO              = 0x0001,               /* ����ģʽ��Ϣ*/
    ID_RNIC_IDLE_TIMEOUT_INFO           = 0x0002,               /* ����ʱ����Ϣ */
    ID_RNIC_EVENT_REPORT_INFO           = 0x0003,               /* �Ƿ��ϱ��¼� */
};
typedef VOS_UINT32 RNIC_DEMAND_DIAL_INFO_MSG_ID_ENUM_UINT32;


enum RNIC_DAIL_EVENT_TYPE_ENUM
{
    RNIC_DAIL_EVENT_UP                  = 0x0600,            /*��Ҫ��������*/
    RNIC_DAIL_EVENT_DOWN                        ,            /*��Ҫ�Ͽ����� */

    RNIC_DAIL_EVENT_TYPE_BUTT
};
typedef VOS_UINT32 RNIC_DAIL_EVENT_TYPE_ENUM_UINT32;


enum AT_RNIC_DIAL_MODE_ENUM/*��ö��ֵ���иĶ���ͬʱ�޸�xt_REDIRECT.c�еĴ�ö��ֵ*/
{
    AT_RNIC_DIAL_MODE_MANUAL,                              /*Manual dial mode*/
    AT_RNIC_DIAL_MODE_DEMAND_CONNECT,                      /*Demand dial mode*/
    AT_RNIC_DIAL_MODE_DEMAND_DISCONNECT,                   /*Demand dial mode*/
    AT_RNIC_DIAL_MODE_BUTT
};
typedef VOS_UINT32 AT_RNIC_DIAL_MODE_ENUM_UINT32;


enum RNIC_DIAL_EVENT_REPORT_FLAG_ENUM
{
    RNIC_FORBID_EVENT_REPORT            = 0x0000,                 /*����Ӧ���ϱ�*/
    RNIC_ALLOW_EVENT_REPORT             = 0X0001,                /*�����Ӧ���ϱ�*/

    RNIC_DIAL_EVENT_REPORT_FLAG_BUTT
};
typedef VOS_UINT32 RNIC_DIAL_EVENT_REPORT_FLAG_ENUM_UINT32;


typedef struct
{
    AT_RNIC_DIAL_MODE_ENUM_UINT32            enDialMode;         /* Dialģʽ */
    RNIC_DIAL_EVENT_REPORT_FLAG_ENUM_UINT32     enEventReportFlag;  /*�Ƿ��Ӧ���ϱ���ʶ*/
    struct timer_list flux_timer; //timer
    unsigned int disconnect_time; //minutes
    unsigned long last_net_time;  //last rx/tx time
    unsigned int is_dump_rmnet_skb;
	bool flow_monitor_flag;
    bool is_timer_init;
    bool is_flow;    
}RNIC_DIAL_MODE_STRU;

/*****************************************************************************
 �ṹ��    : RNIC_FLUX_TIMER
 �ṹ˵��  : ������ʱ�ϱ�

 �޸�����   : ���Ӷ�ʱ��,������ʱ��ʱ�ϱ����������¼���

*****************************************************************************/
typedef struct
{
  
    struct timer_list timer; //timer
    bool is_timer_init;
    
}RNIC_FLUX_TIMER;

/**********************************************************************************/
/******�궨��******/
#define RNIC_ONDEMAND_FILE_LEN          (4)

#define RNIC_IDLETIMEROUT_FILE_LEN      (16)

#define RNIC_EVENTFLAG_FILE_LEN         (4)

#define RNIC_VFILE_CRT_LEVEL            (0777)
#define SKB_BUF_LEN  48
#define LINE_LEN  16
#define RNIC_ERROR_LOG   printk
#define ACPU_PID_RNIC   KERN_ERR
#define RNIC_ERROR_LOG1         //null
#define vos_printf   printk
#define RNIC_NORMAL_LOG  printk
#define RNIC_WARNING_LOG printk
/*�ڴ��������궨��*/
#define PS_MEM_SET(pBuffer,ucData,Count)                    memset(pBuffer, ucData, Count)
#define PS_MIN(x, y)\
        (((x) < (y))? (x) : (y))


/*********************��������**************************/
ssize_t RNIC_WriteOnDemandFile(
    struct file                        *file,
    const char __user                  *buf,
    size_t                              len,
    loff_t                             *ppos
);
ssize_t RNIC_ReadOnDemandFile(
    struct file                        *file,
    char __user                        *buf,
    size_t                             len,
    loff_t                             *ppos
);
ssize_t RNIC_WriteIdleTimerOutFile(
    struct file                        *file,
    const char __user                  *buf,
    size_t                              len,
    loff_t                             *ppos
);
ssize_t RNIC_ReadIdleTimerOutFile(
    struct file                        *file,
    char __user                        *buf,
    size_t                             len,
    loff_t                             *ppos
);
ssize_t RNIC_WriteDialEventReportFile(
    struct file                        *file,
    const char __user                  *buf,
    size_t                              len,
    loff_t                             *ppos
);
ssize_t RNIC_ReadDialEventReportFile(
    struct file                        *file,
    char __user                        *buf,
    size_t                             len,
    loff_t                             *ppos
);
ssize_t RNIC_WriteLogSwitchFile(
    struct file                        *file,
    const char __user                  *buf,
    size_t                              len,
    loff_t                             *ppos
);
ssize_t RNIC_ReadLogSwitchFile(
    struct file                        *file,
    char __user                        *buf,
    size_t                             len, //fangxiaozhi for warning
    loff_t                             *ppos
);
ssize_t RNIC_WriteFluxFlagFile(
    struct file                        *file,
    const char __user                  *buf,
    size_t                              len,
    loff_t                             *ppos
);
ssize_t RNIC_ReadFluxFlagFile(
    struct file                        *file,
    char __user                        *buf,
    size_t                             len, //fangxiaozhi for warning
    loff_t                             *ppos
);
extern void send_rnic_disconnect_event_to_app(void);
VOS_UINT32 RNIC_InitIdleTimerOutFile(struct proc_dir_entry *pstParentFileDirEntry);

VOS_UINT32 RNIC_InitDemandDialFile(VOS_VOID);


extern RNIC_DIAL_MODE_STRU  g_DialMode;
extern RNIC_FLUX_TIMER  g_flux_timer;
extern void set_rnic_last_net_time(unsigned long time);
extern void send_rnic_connect_event_to_app(void);

#endif

