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

/*===========================================================================

                        linux ϵͳͷ�ļ�

===========================================================================*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/moduleparam.h>
#include <net/sock.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>

#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>  /* ����android��׼���߽ӿ� */
#endif

#include <linux/gpio.h>
#include <bsp_wlan.h> /* ����wlan_set_led_flag������ */

#if defined(BSP_CONFIG_BOARD_E5)
#ifdef FEATURE_HUAWEI_MBB_BCM4356
#include <hi_gpio.h>
#include <linux/pci.h>
#endif
#endif

#include <linux/netlink.h>
/*===========================================================================

                        ����ƽ̨��ͷ�ļ�

===========================================================================*/

#include "bsp_softtimer.h"    
#include "wlan_if.h"
#include "wlan_drv_if.h"
#include "wlan_utils.h"

#define HW_VOLTE
#include <ImsParm.h>
#include <product_nv_id.h>
#include <bsp_nvim.h>
#include <product_nv_def.h>
#include "wlan_at_api.h"

/*===========================================================================

                        �������ݽṹ����(linuxϵͳ��)
===========================================================================*/
#define  WIFI_AT_DEVICE             "wifi_at_dev"
#define  WIFI_DELAY_FOR_NL_SEND     msecs_to_jiffies(1)

#define  WIFI_SHUTDOWN_SH           "/system/bin/%s/exe/wifi_poweroff_%u.sh"
#define  WIFI_STARTUP_SH            "/system/bin/%s/exe/wifi_poweron_%u.sh"
#define  WIFI_STARTUP_VSDB_SH       "/system/bin/%s/exe/wifi_poweron_%u_vsdb.sh"

#define  WIFI_MAX_EVENT_QUEUE_LEN       (20)   /* ������󳤶� */
#define  WIFI_USER_ID_MAX               (0xFFFFFFFF) /* netlink�û����̳�ʼֵ */
#define  WIFI_WL_ARGS_MAX               (3)  /* wl������������� */
#define  WIFI_MICROSECONDS              (1000) /* 1��= 1000���� */
#define  WIFI_COMMON_LEN                (64)
#define  CHIP_STUS_CHECK_INTERVAL       (5 * HZ)  /* WiFiоƬ״̬���ʱ�� */
#define  CHIP_STUS_CHECK_TIMES          (3)  /* WiFiоƬ״̬������ */
#define  USER_WIFI_FIRMWARE_DATA_DEAD_EVENT  (32)  /* ����ͨ·���������ͨ·�쳣�¼� */


#define ERROR         (-1)
#define OK            0
#if defined(BSP_CONFIG_BOARD_E5) 
#ifdef FEATURE_HUAWEI_MBB_BCM4356
#define PCIE_BALONG_PCIE0_CLASS  0x0604
#define PCIE_4356_CLASS 0x0280
#define WiFi_POWER_GPIO_NAME     "wifi_power"

void * g_pktid_map_handle_memalloc = NULL;
unsigned int wifi_gpio_addr = GPIO_2_1;
int g_wifi_power_state = 0;


int g_dhd_spe_alloc_port[2] = {0,0};

#endif
#endif
typedef enum 
{
    USER_WIFI_TMR_AP  = 0,  /* ��ʱ���¼���ap��ʱ */
    USER_WIFI_TMR_STA = 1,  /* ��ʱ���¼���sta��ʱ */
    USER_WIFI_TMR_MAX,
}WIFI_USER_TMR_ID;

/* WiFi����BB�ܽ� */
typedef enum _WLAN_PLATFORM_GPIO_TYPE
{
    WLAN_GPIO_WIFI_WAKEUP_BB = WIFI_WAKEUP_BB_GPIO,  /* WiFi����BB(����) */ /*lint !e30 */
}WLAN_PLATFORM_GPIO_TYPE;

/* �û����¼�����10����wifi��ʱ */
typedef struct _wifi_user_event
{
    unsigned int eventId;
    unsigned int eventVal;
} WIFI_USER_EVENT;

/* �¼��ڵ� */
typedef struct _wifi_event_node 
{
    struct list_head list;
    int len;
    char data[0];
}WIFI_EVENT_NODE_STRU;


/* linux ��Ϣ���� */
typedef struct _wifi_msg_ctrl
{
    struct sock   *dev_sock;  //netlink socket;
    unsigned int  user_pid;   //user task pid
    spinlock_t queue_lock;  /*lint !e43*/
    struct delayed_work event_work;        
    struct list_head event_queue;
    unsigned int queue_len;
} WIFI_MSG_CTRL_STRU;

static struct mutex wlan_ioc_mutex;
static struct mutex wlan_pwr_mutex;

WLAN_STATUS_STU wlan_status = {0};    /* ������¼5��ʱ���ڴ����״̬ */
static WLAN_STATUS_STU wlan_count = {0};    /* ������¼����Ĵ��� */
spinlock_t wlan_status_lock;

/* ��Ϣ���������� */
static WIFI_EVENT_NODE_STRU *wifi_event_dequeue(WIFI_MSG_CTRL_STRU *pCtrl);
static WIFI_EVENT_NODE_STRU *wifi_event_node_alloc(WIFI_MSG_CTRL_STRU *pCtrl,void * data, int len);
static void wifi_event_enqueue(WIFI_MSG_CTRL_STRU *pCtrl,WIFI_EVENT_NODE_STRU *pNode);
static void wifi_event_node_free(WIFI_EVENT_NODE_STRU *pNode);
static void wifi_event_send(struct work_struct *work);

/* Ӧ�ò������������ */
static ssize_t wifi_at_device_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count); /*lint !e1075 !e31 !e10*/
static void wifi_netlink_chunnel_uninit(void);
int wifi_event_send_tasklet_mode(void *data, int len);
void wlan_sendmsg_to_userspace(int event_code);

#if defined(BSP_CONFIG_BOARD_E5)
#ifdef FEATURE_HUAWEI_MBB_BCM4356
#ifdef CONFIG_BALONG_PCIE_PM
extern int balong_pcie_pm_sys_suspend(void);
extern void balong_pcie_pm_sys_resume(void);
#endif
extern int pci_save_state(struct pci_dev *dev);
extern void pci_restore_state(struct pci_dev *dev);
extern struct pci_dev *
pci_get_class(unsigned int class, struct pci_dev *from);
#endif
#endif

#if defined(BSP_CONFIG_BOARD_E5_DC04)
#if (MBB_CHG_EXTCHG == FEATURE_ON)
static uint8_t pcie_is_suspend_due_to_extchg = 0;
extern uint8_t chg_stm_get_extchg_online_st(void);
#endif
#endif

#define WIFI_MSG_CTRL_INIT(pCtrl)  \
{\
    INIT_LIST_HEAD(&(pCtrl->event_queue));    \
    pCtrl->queue_len = 0; \
    struct netlink_kernel_cfg config_cfg = {0}; \
    config_cfg.groups = 0; \
    config_cfg.input = (void*)wifi_event_input; \
    config_cfg.cb_mutex = NULL; \
    INIT_DELAYED_WORK(&(pCtrl->event_work), wifi_event_send); \
    spin_lock_init(&(pCtrl->queue_lock)); \
    pCtrl->dev_sock = netlink_kernel_create(&init_net, \
    NETLINK_USER_WIFI_EVENT,&config_cfg); \
    pCtrl->user_pid = (unsigned int)(WIFI_USER_ID_MAX); \
}

#define WIFI_MSG_CTRL_UNINIT(pCtrl)  \
{\
    cancel_delayed_work_sync(&(pCtrl->event_work)); \
    if (pCtrl->dev_sock) \
    {\
        netlink_kernel_release(pCtrl->dev_sock); \
    }\
    pCtrl->user_pid = (unsigned int)(WIFI_USER_ID_MAX); \
}

#define WIFI_EVENT_PATH_IS_READY(pCtrl)  (pCtrl->user_pid != (unsigned int)WIFI_USER_ID_MAX)


DEVICE_ATTR(wifi_at_dev, 0666, NULL, wifi_at_device_store); /*lint !e65 !e133 !e10*/
DECLARE_TASKLET(wifi_send_event_name, wifi_event_send,0); /*lint !e64  !e628*/


/* �豸�ڵ���� */

static struct platform_device *wifi_platform_device = NULL;
static WIFI_MSG_CTRL_STRU  *wifi_user_msg_block = NULL;

static struct attribute *wifi_dev_attributes[] = {
    &dev_attr_wifi_at_dev.attr,
    NULL
};

static const struct attribute_group wifi_dev_group = {
    .attrs = wifi_dev_attributes,
};

typedef enum 
{
    TIMER_INACTIVE = 0,   /* ɾ����״̬ */
    TIMER_ACTIVE  = 1,     /* ����״̬����ӵ�list��״̬ */
}timer_status;

typedef struct 
{
    struct softtimer_list timer_list;
    timer_status   timer_stus;
}wifi_timer;

typedef struct _wlan_pid
{
    int pid;
    const struct pid *p_pid;
}wlan_pid_t;


/* Ŀǰ�õ���������ʱ�� */
static wifi_timer wlan_timer[USER_WIFI_TMR_MAX];

/* WiFiоƬ״̬��ⶨʱ�� */
struct timer_list wlan_stus_detect_timer;


/*��ƷID��Ӧֵ*/
int g_product_index = 0;
EXPORT_SYMBOL(g_product_index);

/* �͹������ */
#ifdef CONFIG_HAS_WAKELOCK
/* wakelock ���� */
static struct wake_lock wlan_wakelock;
#define  WIFI_WAKELOCK_NAME         "wlan_if"
#define  WIFI_WAKE_TIME   5          /* �¼����ͺ���ʱ5s */

/* ������ͶƱ������RealtekоƬ */
struct wake_lock g_wlan_driver_lock;
#endif

#ifdef CONFIG_USB_GADGET_SNPS_DWC_OTG
extern void dwc_otg_wifi_pwctrl_down(void);
#endif
/* for sdio interface */
#ifdef CONFIG_MMC
//extern int mshci_wifi_power_on(void);
//extern int mshci_wifi_power_off(void);
#endif
#define MAX_RETRY  10
static int wlan_pwroff_status = 0; /* Ĭ��wl����ͨ��������˭��˭�ڳ�ʼ��ʱ��رգ���ɺ�� */
static int wlan_ioctl_status = 0; /* ioctl ͨ·�Ƿ񷵻�,0��ʾӦ�ò��wl������� */

wlan_pid_t g_wlan_msg_monitor = {-1, NULL}; /* msg_monitor ���̵� pid */



/*===========================================================================

                         ��������оƬƽ̨�����ݽṹ���塢�ⲿ����
===========================================================================*/


/*===========================================================================

                        ����ʵ�ֲ���

===========================================================================*/
/*****************************************************************************
 ��������  : void kernel_sendmsg_to_userspace(int event_code)
 ��������  : wlan driver send msg to userspace
 �������  : event_code
 �������  : NA
 �� �� ֵ  : NA
*****************************************************************************/
void wlan_sendmsg_to_userspace(int event_code)
{
    int ret = -1;
    int size = 0;
    DEVICE_EVENT *event = NULL;
    char buff[sizeof(DEVICE_EVENT) + sizeof(int)];

    if(event_code < WLAN_WOW_EVENT || event_code >= WLAN_EVENT_MAX)
    {
        PLAT_WLAN_INFO("[%s] error event_code = %d\n",__FUNCTION__,event_code);
        return;
    }

    PLAT_WLAN_INFO("[%s] enter event_code = %d\n",__FUNCTION__,event_code);

    event = (DEVICE_EVENT *)buff;
    size =  sizeof(buff);
    event->device_id = DEVICE_ID_WLAN;      /* �豸ID */
    event->event_code = event_code ;    /* �¼����� */
    event->len = sizeof(int);
    /* �¼��ϱ� */
    ret = device_event_report(event, size);
    PLAT_WLAN_INFO("[%s]eventid=%d, eventcode=%d, len=%d\n",__FUNCTION__,
        event->device_id, event->event_code, event->len);

    if (ret) 
    {
        PLAT_WLAN_ERR("[%s] fail!ret = %d\n",__FUNCTION__,ret);
    }
}
EXPORT_SYMBOL(wlan_sendmsg_to_userspace);
/*****************************************************************************
 ��������  : void wlan_set_led_flag(int flag)
 ��������  : ����led�Ƶ�״̬��Ϊ����v7ƽ̨�ṩ�ĵ�ƽӿڣ�������bsp_wlan.h��
 �������  : flag: 0:����; 1:����
 �������  : NA
 �� �� ֵ  : BOOT
*****************************************************************************/
void wlan_set_led_flag(int flag)
{
    if (0 == flag) /* 1:���� */
    {
        (void)wlan_run_cmd("/system/bin/bcm43217/exe/wl gpioout 0x8 0x8");
    }
    else if (1 == flag) /* 1:���� */
    {
        (void)wlan_run_cmd("/system/bin/bcm43217/exe/wl gpioout 0x8 0x0");
    }
    else
    {
        SYS_WLAN_INFO("undefined flag = %d", flag);
    }
}
EXPORT_SYMBOL(wlan_set_led_flag);

/*****************************************************************************
 ��������  : wlan_nv_read_wiinfo
 ��������  : ��ȡװ��AT^WiInfo��NV����
 �������  : type: wiinfo������
 �������  : plen: ָ�򻺴泤�ȵ�ָ��
 �� �� ֵ  : NV��������
*****************************************************************************/
void *wlan_nv_read_wiinfo(int type, unsigned *plen)
{
    char *buf = NULL;
    unsigned short len = 0, nv_id = 0;

    switch (type)
    {
        case AT_WIINFO_CHANNEL:    /*֧�ֵ��ŵ���*/
        case AT_WIINFO_FREQ:       /*֧�ֵ�Ƶ��*/
        {
            nv_id = NV_ID_AT_GET_CHANWIINFO;
            len = sizeof(WLAN_AT_WIINFO_CHN_BUF_NVIM_STRU);
            break;
        }
        case AT_WIINFO_POWER:       /*֧�ֵ�Ŀ�깦��*/
        {
            nv_id = NV_ID_GET_PWRWIINFO;
            len = sizeof(WLAN_AT_WIINFO_POW_BUF_NVIM_STRU);
            break;
        }
        default:
        {
            PLAT_WLAN_ERR("Error type = %d", type);
            return (NULL);
        }
    }
    /* ����nv���� */
    buf = (char *)kmalloc(len, GFP_ATOMIC);
    if (NULL == buf)
    {
        PLAT_WLAN_ERR("Error, malloc Failed!");
        return (NULL);
    }
    memset(buf, 0x00, len);
    /* ��ȡnv���� */
    if (NV_OK != NV_Read(nv_id, buf, len))
    {
        kfree(buf);
        buf = NULL;
        PLAT_WLAN_ERR("Error, nv read Failed! nv_id = %d, len = %d", nv_id, len);
        return (NULL);
    }

    if (NULL != plen)
    {
        *plen = len;
    }
    return buf;
}

/*****************************************************************************
 ��������  : int wlan_get_poweroff_status()
 ��������  : ��ȡ��ǰ�Ĺػ�״̬
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0:�����ڹػ�״̬
             1:���ڹػ�
*****************************************************************************/
int wlan_get_poweroff_status()
{
    return wlan_pwroff_status;
}
EXPORT_SYMBOL(wlan_get_poweroff_status);

/*****************************************************************************
 ��������  : int wlan_set_poweroff_status(int poweroff_status)
 ��������  : ���ùػ�״̬
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
*****************************************************************************/
int wlan_set_poweroff_status(int poweroff_status)
{
    mutex_lock(&wlan_pwr_mutex);
    wlan_pwroff_status = poweroff_status;
    PLAT_WLAN_INFO("poweroff_status = %d",poweroff_status);
    mutex_unlock(&wlan_pwr_mutex);
    return 0;
}
EXPORT_SYMBOL(wlan_set_poweroff_status);

/*****************************************************************************
 ��������  : int wlan_get_ioctl_status()
 ��������  : ��ȡ��ǰ��ioctlͨ��״̬
 �������  : NA
 �������  : NA
 �� �� ֵ  : 0:�����ڹػ�״̬
             1:���ڹػ�
*****************************************************************************/
int wlan_get_ioctl_status() 
{
    PLAT_WLAN_INFO("wlan_ioctl_status = %d",wlan_ioctl_status);

    return wlan_ioctl_status;
}
EXPORT_SYMBOL(wlan_get_ioctl_status);

/*****************************************************************************
 ��������  : int wlan_count_ioctl_status()
 ��������  : ��1���߼�1��ħ����0xff����wlan_ioctl_status = 0 
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
*****************************************************************************/
int wlan_count_ioctl_status(int ioctl_step) 
{
    mutex_lock(&wlan_ioc_mutex);
    
    if (0xff == ioctl_step)
    {
        wlan_ioctl_status = 0;
    }
    else 
    {
        wlan_ioctl_status += (ioctl_step);
    }
    mutex_unlock(&wlan_ioc_mutex);
    return 0;
}
EXPORT_SYMBOL(wlan_count_ioctl_status);


/*****************************************************************
Function: wifi_tmd
Description:
    ʹ�ò�ͨоƬ�ģ����Թ��øýӿ�
    �ýӿ����ṩwl���߻�ȡ/����WiFi оƬ�Ĺ�ģ
Input:
    tcmd wl�����ַ���
Output:
    N/A
Return:
    0: execute succes
    1: execute failed
*******************************************************************/
int wifi_tcmd(char *tcmd_str)
{
    int ret = ERROR;
    char *envp[WIFI_WL_ARGS_MAX];        
    char **argv = NULL;
    char *temp_argv_0 = NULL;

    /*output tcmd_str*/
    if(NULL == tcmd_str)
    {
        return ERROR;
    }

    WLAN_TRACE_INFO("%s: tcmd_str : %s\n",  __FUNCTION__, tcmd_str);

    argv = argv_split(0, tcmd_str, &ret);

    if (NULL == argv)
    {
        return ret;
    }
    
    if(ret <= 1)
    {
        WLAN_TRACE_ERROR("%s: error, ret <= 1, ret =%d \n",__FUNCTION__, ret);
        argv_free(argv);
        return ERROR;
    }
    else
    {
        temp_argv_0 = argv[0];
        argv[0] = BRCM_WL_TOOL;
    }

    envp[0] = "HOME=/";
    envp[1] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";
    envp[WIFI_WL_ARGS_MAX - 1] = NULL;

    WLAN_TRACE_INFO("%s: end\n",  __FUNCTION__);

    ret = call_usermodehelper(argv[0],argv,envp,1);

    WLAN_TRACE_INFO("%s: success to call_usermodehelper, ret = %d\n",__FUNCTION__, ret);
    argv[0] = temp_argv_0;
    argv_free(argv);
    return ret;
}
/*****************************************************************
Function: wifi_set_product_id
Description:
    ͨ��sh�ű������ں˴��ݲ�Ʒid�ţ�����RSSI������
Input:
    product_id
Output:
    N/A
Return:
*******************************************************************/
void wifi_set_product_id(int product_id)
{
    g_product_index = product_id;
    return;
}
EXPORT_SYMBOL(wifi_set_product_id);
/*****************************************************************
Function: fw_recover_event_send
Description:
    WiFi �쳣����user�ռ䷢������wifi�¼���
Input:
    N/A
Output:
    N/A
Return:
*******************************************************************/
void fw_recover_event_send(void)
{
    if (1 == wlan_get_poweroff_status())
    {
        PLAT_WLAN_ERR("%s:pwroff_status = 1,wifi restarting!\n", __FUNCTION__);
        return;
    }
    else
    {
        wlan_set_poweroff_status(1);
        WLAN_EVENT_REPORT fw_recover = WLAN_FW_ABNORMAL_EVENT;
        PLAT_WLAN_ERR("%s:fw recover event send\n", __FUNCTION__);
        wlan_sendmsg_to_userspace(fw_recover);
        return;
    }

}

EXPORT_SYMBOL(fw_recover_event_send);
/*****************************************************************
Function: wlan_stus_timer_function
Description:
    WiFi״̬��⺯���Ķ�ʱ���Ļص�������
Input:
    N/A
Output:
    N/A
Return:
*******************************************************************/

static void wlan_stus_timer_function()
{
    int i = 0;
    unsigned long flgs = 0;
    int reset_flag = 0;
    WIFI_USER_EVENT event= {0};
    unsigned int *stus = &wlan_status;
    unsigned int *cnt = &wlan_count;
    int stulen = sizeof(WLAN_STATUS_STU) / sizeof(unsigned int);
    spin_lock_irqsave(&wlan_status_lock, flgs);
    /* ���´�����ѭ�����ṹ��ĳ�Ա��������Ա��������Ϊunsigned int���͡�
       ���5���ڳ�Ա������status״̬����0��count����һ�� */
    for(i = 0;i < stulen;i++)
    {
        if(0 < *(stus + i))
        {
            (*(cnt + i))++;
// *(stus + i) = 0;
        }
        else
        {
            *(cnt + i) = 0;
        }
        if(CHIP_STUS_CHECK_TIMES <= *(cnt + i))
        {
            reset_flag = 1;
            break;
        }
        //PLAT_WLAN_INFO("wlan_count:%d is %d",i,*(cnt + i));
    }
    spin_unlock_irqrestore(&wlan_status_lock, flgs);

    if(reset_flag)
    {   
        memset(&wlan_count, 0x00, sizeof(WLAN_STATUS_STU));
        fw_recover_event_send();
        PLAT_WLAN_ERR("FW error,WiFi chip will restart!!!");
    }   
    /* ѭ��������ʱ�� */
    mod_timer(&wlan_stus_detect_timer, jiffies + CHIP_STUS_CHECK_INTERVAL);
    return;
}

/*****************************************************************
Function: wlan_stus_timer_init
Description:
    WiFi״̬��⺯���Ķ�ʱ���ĳ�ʼ��������
Input:
    N/A
Output:
    N/A
Return:
*******************************************************************/

static void wlan_stus_timer_start()
{
    init_timer(&wlan_stus_detect_timer);
    wlan_stus_detect_timer.function = wlan_stus_timer_function;  
    wlan_stus_detect_timer.data = NULL;
    wlan_stus_detect_timer.expires = CHIP_STUS_CHECK_INTERVAL;
    mod_timer(&wlan_stus_detect_timer, jiffies + CHIP_STUS_CHECK_INTERVAL);
    PLAT_WLAN_INFO("wlan_stus_timer_started!!!!");
    return;
}

/***********************************************************************************
 Function:         wifi_timer_delete
 Description:      ɾ����timer
 Calls:
 Input:      USER_WIFI_TIMEID_MAX:ɾ�����еģ������ֵ��ɾ��ָ����
 Output:            NA
 Return:            NULL or node
                  
 ************************************************************************************/
static void wifi_timer_delete(WIFI_USER_TMR_ID timer_id )
{
    if(  USER_WIFI_TMR_MAX <= timer_id )
    {
        PLAT_WLAN_INFO(" timer id error.id = %d",timer_id);
        return;
    }
    
    PLAT_WLAN_INFO(" enter, wlan_timer[%d].timer_stus = %d",timer_id,
                               wlan_timer[timer_id].timer_stus);
       
    if( TIMER_INACTIVE == wlan_timer[timer_id].timer_stus )
    {
        PLAT_WLAN_INFO(" timer TIMER_INACTIVE,id = %d",timer_id);
        return;
    }
    (void)bsp_softtimer_delete(&wlan_timer[timer_id].timer_list);

    (void)bsp_softtimer_free(&wlan_timer[timer_id].timer_list);

    wlan_timer[timer_id].timer_stus = TIMER_INACTIVE;
    PLAT_WLAN_INFO("exit, timer delete success,wlan_timer[%d].timer_stus = %d",
                             timer_id,wlan_timer[timer_id].timer_stus);
}

/***********************************************************************************
 Function:          wifi_timer_callback
 Description:      ��ʱ��ʱִ�к���
 Calls:
 Input:              data: 
 Output:            NA
 Return:            NA                  
 ************************************************************************************/
static irqreturn_t wifi_timer_callback(unsigned int data)
{
    WIFI_USER_EVENT event;
    WIFI_USER_TMR_ID timer_id = (WIFI_USER_TMR_ID)data;
    
    PLAT_WLAN_INFO("enter,timer_id = %d",timer_id);
    
#ifdef CONFIG_HAS_WAKELOCK
    wake_lock_timeout(&wlan_wakelock, WIFI_WAKE_TIME*HZ); /*extend 5s*/
#endif

    if(  USER_WIFI_TMR_MAX <= timer_id )
    {
        PLAT_WLAN_INFO("param error  %d",timer_id);
        return IRQ_NONE;
    }    
    wifi_timer_delete(timer_id);

    event.eventId = USER_WIFI_TIMEOUT_EVENT;
    event.eventVal = timer_id;
    (void)wifi_event_send_tasklet_mode((void *)&event,sizeof(WIFI_USER_EVENT));
    PLAT_WLAN_INFO("event.eventId = %d, event.eventVal= %d",event.eventId, event.eventVal);    

    return IRQ_NONE;
}


/***********************************************************************************
 Function:          wifi_start_timer
 Description:     ����һ����ʱ��
 Calls:
 Input:              seconds: time out value������0ʱ����ʾɾ����ʱ��
 Output:            NA
 Return:            0/-1                  
 ************************************************************************************/
static int wifi_timer_start(unsigned int seconds,WIFI_USER_TMR_ID timer_id)
{
    s32 ret = 0;
    if(  USER_WIFI_TMR_MAX <= timer_id )
    {
        PLAT_WLAN_INFO(" timeid is over range,time id = %d",timer_id);
        return ERROR;
    }
    PLAT_WLAN_INFO("set timer to %d seconds,time id = %d",seconds,timer_id);

    /* һ��timerֻ��һ��ʵ��,�ٴ�����ʱ����֮ǰ��ɾ����,���¼�ʱ */
    wifi_timer_delete(timer_id);
    
    if ( seconds > 0)
    {
        /* ����softtimer_list */
        wlan_timer[timer_id].timer_list.func = wifi_timer_callback; /*lint !e64*/
        wlan_timer[timer_id].timer_list.para = timer_id;   /* ��id��Ϊ���¼����� */
        wlan_timer[timer_id].timer_list.timeout = seconds * WIFI_MICROSECONDS;
        wlan_timer[timer_id].timer_list.wake_type = SOFTTIMER_WAKE;

        ret = bsp_softtimer_create(&(wlan_timer[timer_id].timer_list));     
        if( ret )
        {
            PLAT_WLAN_INFO(" timer creat fail");
            return ERROR;
        }
        bsp_softtimer_add(&(wlan_timer[timer_id].timer_list));
        wlan_timer[timer_id].timer_stus = TIMER_ACTIVE;
        PLAT_WLAN_INFO("timer add success, time = %d seconds,wlan_timer[%d].timer_stus = %d",
                      seconds,timer_id,wlan_timer[timer_id].timer_stus);
    }
    return OK;
}

/*****************************************************************************
 ��������  : static int wifi_powerup_rtl_8192()
 ��������  : RTL8192 ���� wifi
 �������  : NA
 �������  : NA
 �� �� ֵ  : �����������
*****************************************************************************/
static int wifi_powerup_rtl_8192(void)
{
    int ret = 0;
   
#ifdef CONFIG_MMC
    //ret = mshci_wifi_power_on();
    if (0 != ret)
    {
        PLAT_WLAN_ERR("wifi power up fail");
    }
#endif

    return ret;
}

/*****************************************************************************
 ��������  : static int wifi_powerup_bcm_43241()
 ��������  : BCM43241 ���� wifi
 �������  : NA
 �������  : NA
 �� �� ֵ  : �����������
*****************************************************************************/
static int wifi_powerup_bcm_43241(unsigned int chip_name, char *chip_supplier)
{
    int ret = 0;
    char wifi_run_sh[WIFI_COMMON_LEN] = {NULL};    /*ִ�е�SH�ű�*/

    wlan_set_poweroff_status(0);/* 1 ��ʾ���ڹػ���ioctlͨ·�رգ�0 ���� */
    
    snprintf(wifi_run_sh, WIFI_COMMON_LEN, WIFI_STARTUP_SH, chip_supplier, chip_name);
    PLAT_WLAN_INFO("wifi_at_dev:wifi_run_sh is %s!", wifi_run_sh);
    ret = wlan_run_cmd(wifi_run_sh);
    if(0 != ret)
    {
        PLAT_WLAN_ERR("ret = %d, run cmd:%s", (int)ret,  wifi_run_sh);
    }

    return ret;
}

/*****************************************************************************
��������  : static int wifi_powerup()
��������  : wifi�ϵ�
�������  : NA
�������  : NA
�� �� ֵ  : �����������
*****************************************************************************/
static int wifi_powerup(unsigned int chip_name, char *chip_supplier)
{
    int ret = 0;
        
    switch(chip_name)
    {
        case WIFI_CHIP_NAME_8189:
        case WIFI_CHIP_NAME_8192:
            ret = wifi_powerup_rtl_8192();
            break;
        case WIFI_CHIP_NAME_43241:
            ret = wifi_powerup_bcm_43241(chip_name, chip_supplier);
            break;
        default:
        {
            PLAT_WLAN_ERR("no process of this chip:%d ", chip_name);    
            break;
        }
    }

    return ret;
}

/*****************************************************************************
��������  : static int wifi_shutdown_rtl_8192()
��������  : RTL8192  wifi �µ�
�������  : NA
�������  : NA
�� �� ֵ  : �����������
*****************************************************************************/
static int wifi_shutdown_rtl_8192(void)
{
    int ret = 0;
    
#ifdef CONFIG_MMC
    //ret = mshci_wifi_power_off();
    if (0 != ret)
    {
        PLAT_WLAN_ERR("wifi power down fail");
    }
#endif

    return ret;
}

/*****************************************************************************
��������  : static int wifi_shutdown_rtl_8192()
��������  : RTL8192  wifi �µ�
�������  : NA
�������  : NA
�� �� ֵ  : �����������
*****************************************************************************/
static int wifi_shutdown_bcm_43241(unsigned int chip_name, char *chip_supplier)
{
    int ret = 0;
    int index = 0;
    char wifi_run_sh[WIFI_COMMON_LEN] = {NULL};    /*ִ�е�SH�ű�*/

    wlan_set_poweroff_status(1);/* 1 ��ʾ���ڹػ���ioctlͨ·�رգ�0 ���� */

    snprintf(wifi_run_sh, WIFI_COMMON_LEN, WIFI_SHUTDOWN_SH, chip_supplier, chip_name);
    PLAT_WLAN_INFO("wifi_at_dev:wifi_run_sh is %s!", wifi_run_sh);
    
    for(;index < MAX_RETRY;index++)
    {
        int ioctl_status = wlan_get_ioctl_status();
        PLAT_WLAN_ERR("ioctl_status = %d ",ioctl_status);
        if (0 == ioctl_status)
        {
         break;
        }
        msleep(500); 
    }

    PLAT_WLAN_ERR("index = %d ", index);
    /* �ȴ��������ʱ5s��ioctl����δ֪��ִ���µ���� */
#ifdef CONFIG_USB_GADGET_SNPS_DWC_OTG
    /* call hisc power off */
    dwc_otg_wifi_pwctrl_down();
#endif
    ret = wlan_run_cmd(wifi_run_sh);
    if(0 != ret)
    {
        PLAT_WLAN_ERR("ret = %d, run cmd:%s", (int)ret,  wifi_run_sh);
    }

    return ret;
}

/*****************************************************************************
��������  : static int wifi_shutdown()
��������  : wifi �µ�
�������  : NA
�������  : NA
�� �� ֵ  : �����������
*****************************************************************************/
static int wifi_shutdown(unsigned int chip_name, char *chip_supplier)
{
    int ret = 0;
 
    switch(chip_name)
    {
        case WIFI_CHIP_NAME_8189:
        case WIFI_CHIP_NAME_8192:
             ret = wifi_shutdown_rtl_8192();
             break;
        case WIFI_CHIP_NAME_43241:
            ret = wifi_shutdown_bcm_43241(chip_name, chip_supplier);
            break;
        default:
        {
            PLAT_WLAN_ERR("no process of this chip:%d ", chip_name);
            break;
        }
    }  
    
    return ret;
}

 /***********************************************************************************
 Function:          wifi_at_device_store
 Description:      �������Ӧ�ò㴫�ݵ������ַ�
 Calls:
 Input:              pCtrl: control block
                Data: date buffer
                len: data len
 Output:            NA
 Return:            NULL or node
                  
 ************************************************************************************/
static ssize_t wifi_at_device_store (struct device *dev,struct device_attribute *attr,const char *buf, size_t count) /*lint !e31 !e10*/
{
    unsigned int ap_seconds = 0;
    int cmd = 0;
    int index = 0;
    int tmp_val = 0;

    if ((NULL == dev) || (NULL == attr) || (NULL == buf))
    {
        PLAT_WLAN_INFO("[%s]: wifi_at_dev: invalid para!\n", __FUNCTION__);
        return count;
    }
    PLAT_WLAN_INFO("[%s]: receive msg from user is %s\n", __FUNCTION__, buf);
    
    if(sscanf(buf,"%d",&cmd) != 1)
    {
        PLAT_WLAN_ERR("[%s]: wifi_at_dev: %s is error!\n", __FUNCTION__,buf);
        return count;
    }
    switch(cmd)
    {
        case WIFI_AP_SHUTDOWN_TIMEOUT_SET:        /* AP �Զ��رռ�ʱ */
        {
            (void)sscanf(buf,WIFI_AP_SHUTDOWN_TIMEOUT_SET_STR,&ap_seconds) ;
            if(0 != wifi_timer_start(ap_seconds,USER_WIFI_TMR_AP))
            {
                PLAT_WLAN_ERR("[%s]: wifi_at_dev: failed to execute program wifi_ap_start_timer! \n", __FUNCTION__); 
            }
            break; 
        }
        case WIFI_STA_SCAN_TIMEOUT_SET:  
        {
            (void)sscanf(buf,WIFI_TIMER_SET_CMD_STR, &ap_seconds);
            if(0 != wifi_timer_start(ap_seconds,USER_WIFI_TMR_STA))
            {
                PLAT_WLAN_ERR("[%s]: failed to execute wifi_start_timer!\n", __FUNCTION__);
            }
            break;
        }
        case WIFI_STARTUP_SET:
        {
            PLAT_WLAN_INFO("WIFI_STARTUP_SET\n");
            break;
        }
        case WIFI_SHUTDOWN_SET:
        {
            wlan_set_poweroff_status(1);/* 1 ��ʾ���ڹػ���ioctlͨ·�رգ�0 ���� */
            for(;index < MAX_RETRY;index++)
            {
                int ioctl_status = wlan_get_ioctl_status();
                PLAT_WLAN_ERR("[%s]: ioctl_status = %d \n",__FUNCTION__,ioctl_status);
                if (0 == ioctl_status)
                {
                    break;
                }
                msleep(500); 
            }
            PLAT_WLAN_ERR("[%s]: index = %d \n",__FUNCTION__,index);


            break;
        }
        case WIFI_MONITOR_PID_SET:
        {
            (void)sscanf(buf,WIFI_MONITOR_PID_SET_STR,&g_wlan_msg_monitor.pid) ;
            
            rcu_read_lock();
            g_wlan_msg_monitor.p_pid = get_pid(find_vpid(g_wlan_msg_monitor.pid));    
            rcu_read_unlock();
            PLAT_WLAN_INFO("wifi MONITOR_PID = %d, p = %p", g_wlan_msg_monitor.pid, g_wlan_msg_monitor.p_pid);
            break;
        }
        case WIFI_LOG_LEVEL_SET:
        {
            int tmp_val = 0;
            (void)sscanf(buf,WIFI_LOG_LEVEL_SET_STR,&tmp_val) ;           
            (void)wlan_set_log_flag(tmp_val);
            break;
        }   
        default:
        {
            PLAT_WLAN_INFO("[%s]: wifi_at_dev: %s is not support!\n",__FUNCTION__, buf);
            break;

        }
    }
    return count;
}

/***********************************************************************************
 Function:          wifi_dev_register
 Description:      sysfs�ļ�ע��
 Calls:
 Input:              pCtrl: control block
                Data: date buffer
                len: data len
 Output:            NA
 Return:            NULL or node
                  
************************************************************************************/
static int wifi_dev_register(void) /*lint !e10*/
{
    int ret = ERROR;

    wifi_platform_device = platform_device_register_simple(WIFI_AT_DEVICE,-1, NULL, 0);

    if( NULL == wifi_platform_device )
    {
        PLAT_WLAN_INFO("create wifi_platform_device failed.");
        return ERROR;
    }

    ret = sysfs_create_group(&wifi_platform_device->dev.kobj, &wifi_dev_group);
    if (OK != ret)
    {
        PLAT_WLAN_INFO(" create sdio sys filesystem node failed.");
        return ERROR;
    }

    return OK;
}

 /*****************************************************************************
  ��������  : WLAN_RETURN_TYPE wifi_at_device_unregister(void)
  ��������  : �ͷ�wifi_platform_device�豸ռ����Դ
  �������  : NA
  �������  : NA
  �� �� ֵ  : WLAN_RETURN_TYPE
 *****************************************************************************/
 static WLAN_RETURN_TYPE wifi_at_device_unregister(void)
 {   
     ASSERT_NULL_POINTER(wifi_platform_device, WLAN_SUCCESS);
     sysfs_remove_group(&wifi_platform_device->dev.kobj, &wifi_dev_group);
     platform_device_unregister(wifi_platform_device);
     wifi_platform_device = NULL;
     return WLAN_SUCCESS;
 }

 /***********************************************************************************
 Function:          wifi_event_node_alloc
 Description:      Construct a buffer node
 Calls:
 Input:              pCtrl: control block
                Data: date buffer
                len: data len
 Output:            NA
 Return:            NULL or node
                  
 ************************************************************************************/
static WIFI_EVENT_NODE_STRU *wifi_event_node_alloc(WIFI_MSG_CTRL_STRU *pCtrl,void * data, int len)
{
    WIFI_EVENT_NODE_STRU *pNode = NULL;

    PLAT_WLAN_INFO("enter");

    if ( WIFI_MAX_EVENT_QUEUE_LEN < pCtrl->queue_len)
    {
        PLAT_WLAN_INFO(" max queue len is reached! ");
        return NULL;
    }
    
    
    pNode = (WIFI_EVENT_NODE_STRU *)kmalloc(((unsigned int)len + sizeof(WIFI_EVENT_NODE_STRU)), GFP_ATOMIC); /*lint !e628*/
    if (NULL == pNode) 
    {
        PLAT_WLAN_INFO("kmalloc failed! ");
        return NULL;
    }

    pNode->len = len;
    memcpy(pNode->data, data, (unsigned int)len);
    
    return pNode;
}

/***********************************************************************************
Function:           wifi_event_node_free
Description:       free the node
Calls:
Input:             pNote
Output:            NA
Return:            NA
                 
************************************************************************************/
static inline void wifi_event_node_free(WIFI_EVENT_NODE_STRU *pNode)
{
    PLAT_WLAN_INFO("enter");
    if (NULL != pNode)
    {
        kfree(pNode);
    }
}

/***********************************************************************************
Function:           wifi_event_enqueue
Description:       inset the note into queue
Calls:
Input:             pCtrl: control block
             pNote: wifi event note
Output:            NA
Return:            NA
                 
************************************************************************************/

static inline void wifi_event_enqueue(WIFI_MSG_CTRL_STRU *pCtrl,WIFI_EVENT_NODE_STRU *pNode)
{
    unsigned long flags = 0;

    PLAT_WLAN_INFO("enter");

    spin_lock_irqsave(&(pCtrl->queue_lock), flags); /*lint !e628 !e160 !e666 !e534 !e530 !e830 !e10 !e550 !e522 */
    list_add_tail(&pNode->list, &(pCtrl->event_queue));
    pCtrl->queue_len++;
    spin_unlock_irqrestore(&(pCtrl->queue_lock), flags); /*lint !e530 */
}

/***********************************************************************************
Function:           wifi_event_dequeue
Description:      dequeue a note for queue
Calls:
Input:             pCtrl: control block
Output:            NA
Return:            event node or NULL
                 
************************************************************************************/
static inline WIFI_EVENT_NODE_STRU *wifi_event_dequeue(WIFI_MSG_CTRL_STRU *pCtrl) /*lint !e533*/
{
    WIFI_EVENT_NODE_STRU *pNode = NULL;
    unsigned long flags = 0;

    PLAT_WLAN_INFO("enter");

    spin_lock_irqsave(&(pCtrl->queue_lock), flags);/*lint !e666 !e160 !e534 !e530 !e830 !e10 !e550 !e522 */
    pNode = list_first_entry(&(pCtrl->event_queue), WIFI_EVENT_NODE_STRU, list); /*lint !e515 !e533 !e160 !e102 !e15 !e10 !e42*/
    if (NULL == pNode)
    {
        spin_unlock_irqrestore(&(pCtrl->queue_lock), flags);/*lint !e530 */
        return NULL;
    }
    
    list_del(&pNode->list);    /*lint !e413*/
    pCtrl->queue_len--;    
    spin_unlock_irqrestore(&(pCtrl->queue_lock), flags); /*lint !e530 */
    
    return pNode;/*lint !e533 */
}/*lint !e529 */


/***********************************************************************************
Function:          wifi_eventq_is_empty
Description:      check the event queue weather is empty
Calls:
Input:              control block
Output:            NA
Return:            TRUE : empty
               FALSE: not empty
              
************************************************************************************/
static inline int wifi_eventq_is_empty(WIFI_MSG_CTRL_STRU *pCtrl) /*lint !e10*/
{   /*lint !e10*/
    unsigned long flags = 0;/*lint !e578*/
    int ret = 0;
    
    spin_lock_irqsave(&(pCtrl->queue_lock), flags);    /*lint !e666 !e160 !e534 !e530 !e830 !e10 !e550 !e522 !e119 */
    ret = list_empty(&(pCtrl->event_queue));
    spin_unlock_irqrestore(&(pCtrl->queue_lock),flags);/*lint !e530 */
    return ret;  /*lint !e64 */
}

/***********************************************************************************
 Function:          wifi_event_input
 Description:      use to proces msg from APP
 Calls:
 Input:         pCtrl: control block
                _skb: date
 Output:            NA
 Return:            NA                  
 ************************************************************************************/
static void wifi_event_input(struct sk_buff *__skb)/*lint !e527*/
{
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlh = NULL;
    WIFI_MSG_CTRL_STRU *pCtrl = NULL; /*lint !e578*/

    PLAT_WLAN_INFO("enter");

    if (unlikely (NULL == (pCtrl = wifi_user_msg_block)))/*lint !e730 !e628*/
    {
        PLAT_WLAN_INFO("pCtrl is NULL!");
        return;
    }

    skb = skb_get(__skb);
    if (skb->len < NLMSG_SPACE(0))
    {
        PLAT_WLAN_INFO("skb->len isn't enough!");    
        return;
    }

    nlh = nlmsg_hdr(skb);

    /* check nlh */
    if ( NULL == nlh)
    {
        PLAT_WLAN_INFO("nlh is NULL!");
        return;
    }

    if (!WIFI_EVENT_PATH_IS_READY(pCtrl))
    {
        PLAT_WLAN_INFO("record user_pid = 0x%u!",nlh->nlmsg_pid);
        // Store the user space daemon pid
        pCtrl->user_pid = nlh->nlmsg_pid;
        
        // Send device events, reported before daemon start
        schedule_delayed_work(&(pCtrl->event_work),WIFI_DELAY_FOR_NL_SEND);    /*lint !e534*/
    }
}


/***********************************************************************************
 Function:          wifi_event_send
 Description:      send event to user space
 Calls:
 Input:             workqueue
 Output:            NA
 Return:            NA
                  
 ************************************************************************************/
static void wifi_event_send(struct work_struct *work)
{
    WIFI_MSG_CTRL_STRU *pCtrl = NULL; /*lint !e578*/
    WIFI_EVENT_NODE_STRU *pNode = NULL; /*lint !e578*/
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlh = NULL;    
    int msgSize = 0;
    int ret = 0;

    PLAT_WLAN_INFO("enter");
    
    /*restore the contrl block*/        
    pCtrl = wifi_user_msg_block;

    while (!wifi_eventq_is_empty(pCtrl))
    {
        pNode = wifi_event_dequeue(pCtrl);
        if (NULL == pNode)
        {
            PLAT_WLAN_INFO("dequeue a null node");
            break;
        }

        PLAT_WLAN_INFO("dequeue a node");

        /*
         * node->len: size of (node->data), is event message length
         * size: size of (skb) 
         * size = sizeof(*nlh) + align pad + node->len + aligh pad
         * 
         * alloc skb here
         * But, NOT release skb here
         * 
         */
        msgSize = NLMSG_SPACE(pNode->len);
        skb = alloc_skb((unsigned int)msgSize, GFP_ATOMIC);
        if (NULL == skb)
        {
            PLAT_WLAN_INFO("alloc skb failed");
            return;
        }

        // Use "size - sizeof(*nlh)" here (incluing align pads)
        nlh = nlmsg_put(skb, 0, 0, 0, (unsigned int)msgSize - sizeof(*nlh), 0);

        NETLINK_CB(skb).portid = 0;        /*lint !e545 */
        NETLINK_CB(skb).dst_group = 0;  /*lint !e545 */
        memcpy(NLMSG_DATA(nlh), pNode->data, (unsigned int)pNode->len);

        // Release this event entry
        wifi_event_node_free(pNode);

        // Send message to user space
        ret = netlink_unicast(pCtrl->dev_sock, skb, pCtrl->user_pid, 0);
        if (ret < 0) 
        {
            PLAT_WLAN_INFO("msg send failed!!!");
            return;
        }
        
        PLAT_WLAN_INFO("msg send over");    
    }
}

/***********************************************************************************
 Function:          wifi_event_send_tasklet_mode
 Description:      report a message  to user space
 Calls:
 Input:             data: data buffer
               len: date len
 Output:            NA
 Return:            OK or ERROR
 ************************************************************************************/
int wifi_event_send_tasklet_mode(void *data, int len) /*lint !e529!e629*/
{   /*lint !e629*/
    WIFI_MSG_CTRL_STRU *pCtrl = NULL;    /*lint !e578*/
    WIFI_EVENT_NODE_STRU *pNode = NULL; /*lint !e578*/

    PLAT_WLAN_INFO("enter");

    /*restore the contrl block*/        
    pCtrl = wifi_user_msg_block;

    /*check paramate*/
    if (unlikely(NULL == pCtrl) || (!WIFI_EVENT_PATH_IS_READY(pCtrl))) /*lint !e730 */
    {
        PLAT_WLAN_INFO("msg path isn't ready");
        return ERROR;
    }

    pNode = wifi_event_node_alloc(pCtrl,data, len);
    if (unlikely(NULL == pNode)) /*lint !e730 */
    {
        PLAT_WLAN_INFO("alloc node failed");
        return  ERROR;
    }
    
    wifi_event_enqueue(pCtrl,pNode);

    /* Only put event into buffer queue */
    tasklet_schedule(&wifi_send_event_name);    

    return OK;        
}

/*****************************************************************************
 ��������  : WLAN_RETURN_TYPE wlan_event_report(WIFI_USER_EVENT *event) 
 ��������  : �ϱ�netlink�¼�
 �������  : event:�ϱ���Ϣ
 �������  : NA
 �� �� ֵ  : WLAN_RETURN_TYPE
*****************************************************************************/
WLAN_RETURN_TYPE wlan_event_report(WLAN_USER_EVENT *event) /*lint -e529 */
{
    WIFI_MSG_CTRL_STRU *pCtrl = wifi_user_msg_block;
    WIFI_EVENT_NODE_STRU *pNode = NULL; /*lint !e578*/
    
    ASSERT_NULL_POINTER(pCtrl, WLAN_POINTER_NULL);
    ASSERT_NULL_POINTER(event, WLAN_POINTER_NULL);
    PLAT_WLAN_INFO("Enter, eventId = %d, eventVal = %d",
                    (int)event->eventId, (int)event->eventVal);
   
    pNode = wifi_event_node_alloc(pCtrl,(void *)event, sizeof(*event));
    if (unlikely(NULL == pNode)) /*lint !e730 */
    {
        PLAT_WLAN_INFO("alloc node failed");
        return  ERROR;
    }
    
    wifi_event_enqueue(pCtrl,pNode);

    /* Only put event into buffer queue */
    tasklet_schedule(&wifi_send_event_name); 
    return WLAN_SUCCESS;
}
EXPORT_SYMBOL(wlan_event_report);

/*****************************************************************************
 ��������  : int wifi_netlink_chunnel_init(void)
 ��������  : netlink client��ʼ��
             initialization a netlink for wifi pass event to user APP
 �������  : NA
 �������  : NA
 �� �� ֵ  : WLAN_RETURN_TYPE
*****************************************************************************/
static int wifi_netlink_chunnel_init(void)
{
    WIFI_MSG_CTRL_STRU *pCtrl = NULL; /*lint !e578*/
    
    PLAT_WLAN_INFO("enter");
    pCtrl = (WIFI_MSG_CTRL_STRU*) kmalloc(sizeof(WIFI_MSG_CTRL_STRU), GFP_ATOMIC);
    if (NULL == pCtrl)
    {
        PLAT_WLAN_INFO("kmalloc failed");
        return ERROR;
    }

    /*init netlink and other resource*/
    WIFI_MSG_CTRL_INIT(pCtrl);  /*lint !e69 !e534 !e133 !e43 !e611*/

    if (NULL == pCtrl->dev_sock)
    {
        /*if failed, release the resource*/
        WIFI_MSG_CTRL_UNINIT(pCtrl);  /*lint !e534*/
        kfree(pCtrl);        
        PLAT_WLAN_INFO("WIFI_MSG_CTRL_INIT failed!");    
        return ERROR;
    }

    /*record the contrl block*/        
    wifi_user_msg_block = pCtrl;
    PLAT_WLAN_INFO("exit");
    return OK;
}

/*****************************************************************************
 ��������  : void wifi_netlink_chunnel_uninit(void)
 ��������  : netlink client��Դ�ͷ�
 �������  : NA
 �������  : NA
 �� �� ֵ  : NA
*****************************************************************************/
static void wifi_netlink_chunnel_uninit(void)
{
    WIFI_MSG_CTRL_STRU *pCtrl = NULL; /*lint !e578*/

    PLAT_WLAN_INFO("enter");

    /*restore the contrl block*/        
    pCtrl = wifi_user_msg_block;

    if (pCtrl)
    {
        WIFI_MSG_CTRL_UNINIT(pCtrl); /*lint !e534*/
        kfree(pCtrl);
        wifi_user_msg_block = NULL;        
    }
    PLAT_WLAN_INFO("exit");
}

 /*****************************************************************************
  ��������  : WLAN_RETURN_TYPE wlan_signal_report(WiFi_SIGNAL_EVENT_ENUM signal)
  ��������  : ��Ӧ�ò㷢���ź�
  �������  : NA
  �������  : NA
  �� �� ֵ  : �����ź��Ƿ�ɹ�
 *****************************************************************************/
 WLAN_RETURN_TYPE wlan_signal_report(WLAN_SIGNAL_EVENT_ENUM signal) /*lint -e529 */
 {
     int ret = 0;
     ASSERT_NULL_POINTER(g_wlan_msg_monitor.p_pid, WLAN_POINTER_NULL);
     
     ret = kill_pid((struct pid *)g_wlan_msg_monitor.p_pid, signal, 1);
     
     PLAT_WLAN_INFO("kill signal = %d, ret = %d, pid = %d", signal, ret, g_wlan_msg_monitor.pid);
     return ((0 == ret) ? WLAN_SUCCESS : WLAN_EFFECT_FAILURE);
 }
 EXPORT_SYMBOL(wlan_signal_report);

/*****************************************************************************
 ��������  : wlan_wakeup_irq_handle
 ��������  : WiFi����BB �жϻص�������
 �������  : irq: �жϺţ�dev_instance: �ж�ʵ��
 �������  : NA
 �� �� ֵ  : irqreturn_t
*****************************************************************************/
STATIC irq_handler_t g_wlan_wakeup_irq_handler = (irq_handler_t)NULL; /*lint !e10 */
STATIC irqreturn_t wlan_wakeup_irq_handle(int irq, void *dev_instance) /*lint !e629 */
{
    unsigned int ucData = 0;
    irqreturn_t irq_ret = IRQ_NONE; /*lint !e20 */

    PLAT_WLAN_INFO("Entry: get intr %d, dev_instance = %p", irq, dev_instance);

    if(!gpio_int_state_get(WLAN_GPIO_WIFI_WAKEUP_BB))
    {
        return IRQ_NONE;
    }

    gpio_int_mask_set(WLAN_GPIO_WIFI_WAKEUP_BB);
    gpio_int_state_clear(WLAN_GPIO_WIFI_WAKEUP_BB);
    
    if (NULL == g_wlan_wakeup_irq_handler)
    {
        PLAT_WLAN_ERR("intr handler is NULL");
    }
    else
    {
        irq_ret = g_wlan_wakeup_irq_handler(irq, dev_instance); /*lint !e64 */
    }

    gpio_int_unmask_set(WLAN_GPIO_WIFI_WAKEUP_BB);

    PLAT_WLAN_INFO("Exit: ret = %d", (int)irq_ret);
    return irq_ret;
}
/*****************************************************************************
 ��������  : wlan_request_wakeup_irq
 ��������  : WiFi����BB �ж�ע��ӿ�
 �������  : handler: �жϻص�������devname: �豸���ƣ�dev:�ж���Ӧ����
 �������  : NA
 �� �� ֵ  : int
*****************************************************************************/
unsigned int wlan_request_wakeup_irq(irq_handler_t handler, const char *devname, void *dev) /*lint !e10 !e578 !e101 */
{
    int ret = 0;
    unsigned int irq_id = 0;
    const unsigned long irq_flags = (IRQF_NO_SUSPEND | IRQF_SHARED);//(IRQF_TRIGGER_RISING);liuzixi

    ASSERT_NULL_POINTER(handler, 0);
    ASSERT_NULL_POINTER(devname, 0);

    ret = gpio_request(WLAN_GPIO_WIFI_WAKEUP_BB, "GPIO_WIFI_WAKE_BB");
    if (ret < 0)
    {
        PLAT_WLAN_ERR("ret = %d, Failed to reques GPIO_WIFI_WAKE_BB GPIO!", ret);
        return (0);
    }

    ret = gpio_direction_input(WLAN_GPIO_WIFI_WAKEUP_BB);
    if (ret < 0)
    {
        PLAT_WLAN_ERR("ret = %d, Failed to configure direction for GPIO", ret);
        gpio_free(WLAN_GPIO_WIFI_WAKEUP_BB);
        return (0);
    }

    irq_id = gpio_to_irq(WLAN_GPIO_WIFI_WAKEUP_BB);
    if ((int)irq_id < 0)
    {
        PLAT_WLAN_ERR("ret = %d, Unable to get g_dhd_wow_irq number for GPIO", (int)irq_id);
        gpio_free(WLAN_GPIO_WIFI_WAKEUP_BB);
        return (0);
    }
    gpio_int_mask_set(WLAN_GPIO_WIFI_WAKEUP_BB);
    gpio_int_trigger_set(WLAN_GPIO_WIFI_WAKEUP_BB, IRQF_TRIGGER_RISING);
    gpio_set_function(WLAN_GPIO_WIFI_WAKEUP_BB, GPIO_INTERRUPT);
    ret = request_irq(irq_id, wlan_wakeup_irq_handle, irq_flags, devname, dev); /*lint !e64 */
    if (0 != ret)
    {
        PLAT_WLAN_ERR("irq_id = %d, ret = %d, Unable to request_irq error", irq_id, ret);
        gpio_free(WLAN_GPIO_WIFI_WAKEUP_BB);
        return (0);
    }
    g_wlan_wakeup_irq_handler = handler;

    gpio_int_state_clear(WLAN_GPIO_WIFI_WAKEUP_BB);
    gpio_int_unmask_set(WLAN_GPIO_WIFI_WAKEUP_BB);

    PLAT_WLAN_INFO("Get WiFi WAKEUP BB(0x%04x) GPIO irq = %d Successfully", (int)irq_flags, irq_id);
    return irq_id;
}
EXPORT_SYMBOL(wlan_request_wakeup_irq);

/*****************************************************************************
 ��������  : wlan_free_irq
 ��������  : WiFi����BB �ж��ͷŽӿ�
 �������  : irq: �ж�id
 �������  : NA
 �� �� ֵ  : int
*****************************************************************************/
void wlan_free_irq(unsigned int irq, void *dev)
{
    g_wlan_wakeup_irq_handler = NULL; /*lint !e64 */
    gpio_int_mask_set(WLAN_GPIO_WIFI_WAKEUP_BB);
    free_irq(irq, dev);
    gpio_free(WLAN_GPIO_WIFI_WAKEUP_BB);
}
EXPORT_SYMBOL(wlan_free_irq);

#ifdef CONFIG_HAS_WAKELOCK
 /*****************************************************************************
  ��������  : wlan_reg_driver_lock
  ��������  : ע������ʹ�õĵ��廽����
  �������  : NA
  �������  : NA
  �� �� ֵ  : NA
  ����˵��  : 
 *****************************************************************************/
void wlan_reg_driver_lock(int reg)
{
    static int s_lock_reg = false;
    if (true == reg)
    {
        if (true != s_lock_reg)
        {
            s_lock_reg = true;
            memset(&g_wlan_driver_lock, 0, sizeof(g_wlan_driver_lock));
            wake_lock_init(&g_wlan_driver_lock, WAKE_LOCK_SUSPEND, "wifi_driver_lock");
        }
    }
    else
    {
        if (true == s_lock_reg)
        {
            s_lock_reg = false;
            if (wake_lock_active(&g_wlan_driver_lock))
            {
                wake_unlock(&g_wlan_driver_lock);
            }
            wake_lock_destroy(&g_wlan_driver_lock);
            memset(&g_wlan_driver_lock, 0, sizeof(g_wlan_driver_lock));
        }
    }
    PLAT_WLAN_INFO("Exit, wifi driver lock %s", (s_lock_reg ? "Register" : "Unregister"));
}
/*****************************************************************************
  ��������  : wlan_set_driver_lock
  ��������  : ������ʹ�õ���
  �������  : NA
  �������  : NA
  �� �� ֵ  : NA
  ����˵��  : 
*****************************************************************************/
void wlan_set_driver_lock(int locked)
{
    if (true == locked)
    {
        if (!wake_lock_active(&g_wlan_driver_lock))
        {
            wake_lock(&g_wlan_driver_lock);
        }
    }
    else
    {
        if (wake_lock_active(&g_wlan_driver_lock))
        {
            wake_unlock(&g_wlan_driver_lock);
        }
    }
    PLAT_WLAN_INFO("Exit, wifi driver lock %s", (locked ? "ON" : "OFF"));
}
EXPORT_SYMBOL(wlan_set_driver_lock);
#endif /* CONFIG_HAS_WAKELOCK */

 /*****************************************************************************
  ��������  : int wlan_if_init(void)
  ��������  : ����������ʼ��
  �������  : NA
  �������  : NA
  �� �� ֵ  : WLAN_RETURN_TYPE
 *****************************************************************************/
static int __init wlan_if_init(void)
{
    int ret = 0;
#if defined(BSP_CONFIG_BOARD_E5)
#ifdef FEATURE_HUAWEI_MBB_BCM4356
    g_pktid_map_handle_memalloc = kmalloc(PKTID_MAP_HANDLE_MEMALLOC_SIZE, GFP_ATOMIC);
    if (NULL == g_pktid_map_handle_memalloc)
    {
        printk("g_pktid_map_handle_memalloc alloc fail\n");
    }
    ret = gpio_request(wifi_gpio_addr, WiFi_POWER_GPIO_NAME);
    if(0 != ret) 
    {
        printk(KERN_ERR "%s: request gpio %s FAIL!\n", __FUNCTION__, WiFi_POWER_GPIO_NAME);
        return ret;
    }
#endif
#endif
    ret = wifi_dev_register();
    
    ret |= wifi_netlink_chunnel_init(); 

    /* init timer */
    memset(wlan_timer,0x0,sizeof(wlan_timer));
    
#ifdef CONFIG_HAS_WAKELOCK
    wake_lock_init(&wlan_wakelock,WAKE_LOCK_SUSPEND,WIFI_WAKELOCK_NAME);
    wlan_reg_driver_lock(true);
#endif

    spin_lock_init(&wlan_status_lock);
    wlan_stus_timer_start();
    
    mutex_init(&wlan_ioc_mutex);
    mutex_init(&wlan_pwr_mutex);

    return ret;
}

/*****************************************************************************
 ��������  : int wlan_if_exit(void)
 ��������  : ����������ʼ��
 �������  : NA
 �������  : NA
 �� �� ֵ  : WLAN_RETURN_TYPE
*****************************************************************************/
static void __exit wlan_if_exit(void)
{ 
#if defined(BSP_CONFIG_BOARD_E5)
#ifdef FEATURE_HUAWEI_MBB_BCM4356
    if(g_pktid_map_handle_memalloc)
    {
        kfree(g_pktid_map_handle_memalloc);
    }
#endif
#endif
    /* wifi at dev unregister */
    (void)wifi_at_device_unregister();
    
    /* netlink uninit */
    wifi_netlink_chunnel_uninit();

    /* del all timers */
    wifi_timer_delete(USER_WIFI_TMR_AP);
    wifi_timer_delete(USER_WIFI_TMR_STA);

    del_timer_sync(&wlan_stus_detect_timer);
    
#ifdef CONFIG_HAS_WAKELOCK
    wake_lock_destroy(&wlan_wakelock);
    wlan_reg_driver_lock(false);
#endif
}

/*******************************************************************
Wifi regulator power on and pcie host resume
********************************************************************/
#if defined(BSP_CONFIG_BOARD_E5)
#ifdef FEATURE_HUAWEI_MBB_BCM4356
void wifi_reg_on_pull_up(void)
{
    if  (!gpio_is_valid(wifi_gpio_addr))  {
        printk("%s:fail to get wifi gpio!\n",__FUNCTION__);
        return;
    }
    /*����Ѿ��Ϲ��磬��ֱ�ӷ���*/
    if (1 == gpio_get_value(wifi_gpio_addr))
    {
        printk("WL_REG_ON has been pulled up in wifi_reg_on_pull_up!!!\n");
        return;
    }
    mdelay(500);
    gpio_direction_output(wifi_gpio_addr,1); 
    mdelay(500);
}
void wifi_reg_on_pull_down(void)
{
    if  (!gpio_is_valid(wifi_gpio_addr))  {
        printk("%s:fail to get wifi gpio!\n",__FUNCTION__);
        return;
    }
    /*����Ѿ��¹��磬��ֱ�ӷ���*/
    if (0 == gpio_get_value(wifi_gpio_addr))
    {
        printk("WL_REG_ON has been pulled down in wifi_reg_on_pull_down!!!\n");
        return;
    }
    gpio_direction_output(wifi_gpio_addr,0); 

    mdelay(500);
}
void wifi_power_on_4356(void)
{
    struct pci_dev *h = NULL;
    struct pci_dev *d = NULL;
    spinlock_t power_onoff_lock;
    unsigned long flags = 0;
    g_wifi_power_state = 1;
    if  (!gpio_is_valid(wifi_gpio_addr))  {
        printk("%s:fail to get wifi gpio!\n",__FUNCTION__);
        return;
    }
    /*����Ѿ��Ϲ��磬��ֱ�ӷ���*/
    if (1 == gpio_get_value(wifi_gpio_addr))
    {
        printk("WL_REG_ON has been pulled up!!!\n");
        return;
    }
#ifdef CONFIG_BALONG_PCIE_PM
#if defined(BSP_CONFIG_BOARD_E5_DC04) && (MBB_CHG_EXTCHG == FEATURE_ON)
    if (likely(0 == pcie_is_suspend_due_to_extchg))
    {
        balong_pcie_pm_sys_suspend();
        mdelay(100);
    }
    else
    {
        pcie_is_suspend_due_to_extchg = 0;
    }
    balong_pcie_pm_sys_resume();
#else
    balong_pcie_pm_sys_suspend();
    mdelay(100);
    balong_pcie_pm_sys_resume();
#endif
#endif
    spin_lock_irqsave(&power_onoff_lock, flags);
    /*get host pcie���üĴ���*/
    h = pci_get_class(PCIE_BALONG_PCIE0_CLASS << 8,h);
    /*get device���üĴ���*/
    d = pci_get_class(PCIE_4356_CLASS << 8,d);
    if ( (NULL == h) || (NULL == d))
    {
        printk("please check pcie enumeration!!!\n");
        spin_unlock_irqrestore(&power_onoff_lock, flags);
        return;
    }
    /*restore host pcie���üĴ���*/
    pci_restore_state(h);
    /*restore device pcie���üĴ���*/
    pci_restore_state(d);
    spin_unlock_irqrestore(&power_onoff_lock, flags);
}

/*******************************************************************
Wifi regulator power off and pcie host suspend
********************************************************************/
void wifi_power_off_4356(void)
{
    struct pci_dev *d = NULL;
    struct pci_dev *h = NULL;
    spinlock_t power_onoff_lock;
    unsigned long flags = 0;
    g_wifi_power_state = 0;
    if  (!gpio_is_valid(wifi_gpio_addr))  {
        printk("%s:fail to get wifi gpio!\n",__FUNCTION__);
        return;
    }
    /*����¹��磬��ֱ�ӷ���*/
    if (0 == gpio_get_value(wifi_gpio_addr))
    {
        printk("WL_REG_ON has been pulled down!!!\n");
        return;
    }

    spin_lock_irqsave(&power_onoff_lock, flags);
    /*get host pcie���üĴ���*/
    h = pci_get_class(PCIE_BALONG_PCIE0_CLASS << 8,h);
    /*get device���üĴ���*/
    d = pci_get_class(PCIE_4356_CLASS << 8,d);
    if ( (NULL == h) || (NULL == d))
    {
        printk("please check pcie enumeration!!!\n");
        spin_unlock_irqrestore(&power_onoff_lock, flags);
        return;
    }
    /*save host pcie���üĴ���*/
    pci_save_state(h);
    /*save device pcie���üĴ���*/
    pci_save_state(d);
    spin_unlock_irqrestore(&power_onoff_lock, flags);

#if defined(BSP_CONFIG_BOARD_E5_DC04)
#if (FEATURE_ON == MBB_CHG_EXTCHG)
    /*�����ǰ�Ƕ�����ģʽ�ر�Wi-Fi����ͬʱSuspend PCIE��*/
    if (unlikely(1 == chg_stm_get_extchg_online_st()))
    {
        balong_pcie_pm_sys_suspend();
        pcie_is_suspend_due_to_extchg = 1;
    }
#endif
#endif
}

#if defined(BSP_CONFIG_BOARD_E5_DC04)
#if (FEATURE_ON == MBB_CHG_EXTCHG)
 /*****************************************************************************
  ��������  : pcie_resume_before_fastoff
  ��������  : Ϊ������ģ���װ�Ľӿڡ����ڶ�����ģ���ж������ΪWi-Fi�ر�
              �ֶ�������PCIE Suspend�����ڽ���ٹػ�ģʽǰ�ָ���
  �������  : NA
  �������  : NA
  �� �� ֵ  : NA
 *****************************************************************************/
void pcie_resume_before_fastoff()
{
    struct pci_dev *h = NULL;
    struct pci_dev *d = NULL;

    if (1 == pcie_is_suspend_due_to_extchg)
    {
#if defined(CONFIG_BALONG_PCIE_PM)
        balong_pcie_pm_sys_resume();
#endif
        /*get host pcie���üĴ���*/
        h = pci_get_class(PCIE_BALONG_PCIE0_CLASS << 8,h);
        /*get device���üĴ���*/
        d = pci_get_class(PCIE_4356_CLASS << 8,d);
        if ( (NULL == h) || (NULL == d))
        {
            printk(KERN_ERR "please check pcie enumeration!!!\n");
            return;
        }
        /*restore host pcie���üĴ���*/
        pci_restore_state(h);
        /*restore device pcie���üĴ���*/
        pci_restore_state(d);
        pcie_is_suspend_due_to_extchg = 0;
    }
}
#endif
#endif

/*****************************************************************************
 ��������  : void set_offload_connect_status(int connect_status)
 ��������  : set offload connect status
 �������  : connect_status
 �������  : NA
 �� �� ֵ  : void
*****************************************************************************/
int g_offload_connect_status = 0;//Ĭ�ϲ�����
void set_offload_connect_status(int connect_status)
{
    g_offload_connect_status = connect_status;
    PLAT_WLAN_ERR("set_offload_connect_status %d\n",g_offload_connect_status);
}
void get_offload_connect_status(void)
{
    PLAT_WLAN_ERR("get_offload_connect_status %d\n",g_offload_connect_status);
}
#endif
#endif
module_init(wlan_if_init); /*lint !e529*/
module_exit(wlan_if_exit); /*lint !e529*/


/*===========================================================================

                       ���������б�

===========================================================================*/

#if defined(BSP_CONFIG_BOARD_E5)
#ifdef FEATURE_HUAWEI_MBB_BCM4356
EXPORT_SYMBOL(wifi_power_on_4356);
EXPORT_SYMBOL(wifi_power_off_4356);
EXPORT_SYMBOL(wifi_reg_on_pull_down);
EXPORT_SYMBOL(wifi_reg_on_pull_up);
EXPORT_SYMBOL(g_pktid_map_handle_memalloc);
EXPORT_SYMBOL(g_offload_connect_status);
EXPORT_SYMBOL(set_offload_connect_status);
EXPORT_SYMBOL(get_offload_connect_status);

EXPORT_SYMBOL(g_dhd_spe_alloc_port);

#endif
#endif

EXPORT_SYMBOL(wifi_tcmd);
EXPORT_SYMBOL(wifi_event_send_tasklet_mode);

EXPORT_SYMBOL(wlan_status);

MODULE_AUTHOR("Hisilicon Drive Group");
MODULE_DESCRIPTION(WIFI_AT_DEVICE);
MODULE_LICENSE("GPL");

