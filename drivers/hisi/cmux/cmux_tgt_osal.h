#ifndef CMUX_TGT_OSAL_H
#define CMUX_TGT_OSAL_H


#ifdef WIN32
#include <windows.h>
#else
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/mlog_lib.h>
#endif

#include "cmux_cmd.h"

#if ( FEATURE_ON == MBB_HSUART_CMUX )
extern struct cmux_callback callbk;

#define MUTEX_PHY_LOCK()   mutex_lock(&g_port_mutex_phy);
#define MUTEX_PHY_UNLOCK() mutex_unlock(&g_port_mutex_phy);

#define MUTEX_RECV_LOCK()   mutex_lock(&g_port_mutex_recv);
#define MUTEX_RECV_UNLOCK() mutex_unlock(&g_port_mutex_recv);

#define MUTEX_RECVDATA_LOCK()   mutex_lock(&g_port_mutex_recvdata);
#define MUTEX_RECVDATA_UNLOCK() mutex_unlock(&g_port_mutex_recvdata);

#define MUTEX_SEND_LOCK()   mutex_lock(&g_port_mutex_send);
#define MUTEX_SEND_UNLOCK() mutex_unlock(&g_port_mutex_send);

extern int cmux_dbg_level;

typedef enum{
    CMUX_DEBUG_INFO,
    CMUX_DEBUG_WARN,
    CMUX_DEBUG_ERR,
    CMUX_DEBUG_MAX
}cmux_debug_status;
typedef int (*timer_cb_t)(CMUXLIB *cm, unsigned char dlc);
typedef struct{
    CMUXLIB *cm;
    int dlc_tmr;
    struct delayed_work timer;
    timer_cb_t timer_cb;
}cmux_osal_timer_type;

#define CMUX_PRINT(level, fmt, arg0...) do{\
    if ( level >= cmux_dbg_level )\
    {\
        printk(KERN_ERR"%s,%d, "fmt"\n",__FUNCTION__,__LINE__,##arg0);\ 
    }\
}while(0)

#define CMUX_MLOG_RECORD(fmt, arg0...) do{\
    MLOG_PRINT_ERROR(MLOG_MODULE_CMUX, fmt, ##arg0);\
}while(0)

#ifdef WIN32
extern int cb_send_to_com( char* com_name, unsigned char* rawptr, unsigned short len);
#else
extern int cb_send_to_com( struct file * fp, unsigned char* rawptr, unsigned short len);
#endif

unsigned char * cmux_alloc (int size);
int cmux_free (unsigned char *);
unsigned char * cmux_alloc_skb(int size);
void cmux_put_skb(unsigned char *dest, unsigned char *src, unsigned int size);
unsigned char* cmux_get_skb(unsigned char *src);
void cmux_hex_dump(cmux_debug_status level, unsigned char *buf, unsigned int len);
void cmux_free_skb(unsigned char *buff);
void cmux_stop_timer (struct cmuxlib *, unsigned char);
int cmux_start_timer (struct cmuxlib *, unsigned char, struct cmux_timer *, 
    int (*cmux_rxed_timer_expiry)(struct cmuxlib *cm, unsigned char dlc));
/******************************************************************************
Function:       CMUX_init_timer
Description:    初始化超时定时器，注册回调(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_init_timer(struct delayed_work *cmux_timer, 
                                        void (*timer_expiry_cb)(struct work_struct *w));

/******************************************************************************
Function:       CMUX_timer_begin
Description:    启动超时定时器(适配balong平台)
Input:          int time_val；单位是毫秒
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_timer_begin(struct delayed_work *cmux_timer, int time_val);

/******************************************************************************
Function:       CMUX_timer_end
Description:    关闭超时定时器(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_timer_end(struct delayed_work *cmux_timer);

#ifndef NULL
#define NULL 0
#endif

#ifdef WIN32
#else
#define printf pr_err
#endif

#endif /* MBB_HSUART_CMUX */

#endif /* CMUX_TGT_OSAL_H */
