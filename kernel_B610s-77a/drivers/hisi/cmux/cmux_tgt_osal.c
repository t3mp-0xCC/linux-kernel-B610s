/******************************************************************************
  Copyright , 1988-2014, Huawei Tech. Co., Ltd.
  File name:      cmux_tgt_osal.c
  Author: HUAWEI DRIVER DEV GROUP     Version:  1.00      Date:  20140804
  Description:   
  Others:         None
******************************************************************************/

/******************************************************************************
头文件声明
******************************************************************************/

#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include <linux/mutex.h>
#include <linux/skbuff.h>
extern struct mutex g_port_mutex_phy;
extern unsigned int mutex_flag_phy;
extern struct mutex g_port_mutex_send;
extern unsigned int mutex_flag_send;
extern struct mutex g_port_mutex_recv;
extern unsigned int mutex_flag_recv;
extern struct mutex g_port_mutex_recvdata;
extern unsigned int mutex_flag_recvdata;
#endif

#include "cmux_framer.h"
#include "cmux_api_ifc.h"
#include "cmux_tgt_osal.h"
#include "cmux_drv_ifc.h"

#if ( FEATURE_ON == MBB_HSUART_CMUX )
/******************************************************************************
变量声明
******************************************************************************/

/* Set the currently active DLC*/
unsigned char gcmux_active_dstask_dlc      = 0;
/*Set the currently active DLC*/
unsigned char gcmux_last_active_dstask_dlc = 0;
unsigned char gcmux_active_ppp_pstask_dlc  = 0;

struct cmux_callback callbk;

#define FEATURE_MEM_DEBUG

extern unsigned char gcmux_active_ppp_pstask_dlc;

static struct delayed_work cmuxtask_timer;

cmux_osal_timer_type cmux_tmr_container;

int timer_init = 0;

int dlc_tmr = -1;

int cmux_dbg_level = CMUX_DEBUG_ERR;/*lint !e18*/

int (*cmux_rxed_timer_expiry_g)(CMUXLIB *cm, unsigned char dlc);
#define CMUX_HEAP_SIZE 10 * 1024
#define THIS_FILE_ID      9999

unsigned int cmux_alloc_count = 0;
unsigned int cmux_free_count  = 0;

unsigned int cmux_mem_usage = 0;
/******************************************************************************
函数实现
******************************************************************************/

/******************************************************************************
Function:       CMUX_init_timer
Description:    初始化超时定时器，注册回调(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_init_timer
(
    struct delayed_work *cmux_timer, 
    void (*timer_expiry_cb)(struct work_struct *w)
)
{
    INIT_DELAYED_WORK(cmux_timer, timer_expiry_cb);
}

/******************************************************************************
Function:       CMUX_timer_begin
Description:    启动超时定时器(适配balong平台)
Input:          int time_val；单位是毫秒
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_timer_begin(struct delayed_work *cmux_timer, int time_val)
{
    schedule_delayed_work(cmux_timer, msecs_to_jiffies(time_val));
}

/******************************************************************************
Function:       CMUX_timer_end
Description:    关闭超时定时器(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_timer_end(struct delayed_work *cmux_timer)
{
    cancel_delayed_work_sync(cmux_timer);
}

static void cmuxtask_timer_cb(struct work_struct *w)
{
    cmux_osal_timer_type *con_ptr =
    container_of(to_delayed_work(w), cmux_osal_timer_type, timer);/*lint !e102 !e42*/
    
    if (( NULL != con_ptr->timer_cb ) && ( NULL != con_ptr->cm ))
    {
        (*con_ptr->timer_cb)(con_ptr->cm, con_ptr->dlc_tmr);
    }
    return;
}

/* Memory Related Porting */
// Following piece of code gets enabled only for target side
int cmux_start_timer (CMUXLIB *cm, unsigned char dlc, struct cmux_timer *ct, 
int (*cmux_rxed_timer_expiry)(CMUXLIB *cm, unsigned char dlc))/*lint !e78*/
{/*lint !e527*/
    enum fr_type ftype = 0;

    CMUX_PRINT(CMUX_DEBUG_INFO,"should start timer\n");
    if ( ( NULL == cm ) || ( NULL == ct ) )
    {
        return FAIL;
    }
    if ((dlc == 0) && (cm->dlc_db [0] != NULL) && (cm->dlc_db [0]->cmd_list))
    {
        ftype = cm->dlc_db [0]->cmd_list->fr->f_type;
    }
    if ((ftype == CLD_CMD) || ( ftype == FCON_CMD ) || ( ftype == FCOFF_CMD ))
    {
        int timeout_cm = ct->res_timer_t2;
        cmux_tmr_container.timer_cb = cmux_rxed_timer_expiry;
        cmux_tmr_container.dlc_tmr = (int) dlc;
        cmux_tmr_container.cm = cm;
        if (!timer_init)
        {
            INIT_DELAYED_WORK(&cmux_tmr_container.timer, cmuxtask_timer_cb);
            timer_init = 1;
        }
        CMUX_PRINT(CMUX_DEBUG_INFO,"going to start timer\n");
        schedule_delayed_work(&cmux_tmr_container.timer, msecs_to_jiffies(timeout_cm));
        return 0;
    }

    if (cm->dlc_db [dlc] == NULL)
    {
        return FAIL;
    }
    cm->dlc_db [dlc]->dlc = dlc;/*lint !e63*/
    ct->retransmit_n2     = 0;/*lint !e63*/
    return 0;
} /* cmux_start_timer */

void cmux_stop_timer (CMUXLIB *cm, unsigned char dlc)/*lint !e527*/
{
    if (cm->dlc_db [dlc] == NULL)
    {
        return;
    }
    cm->dlc_db [dlc]->dlc = dlc;
    CMUX_PRINT(CMUX_DEBUG_INFO,"going to cancel timer\n");
    cancel_delayed_work_sync(&cmux_tmr_container.timer);
    
    return;
}

//used for allocating the buffer
unsigned char * cmux_alloc(int size)
{
    void *ptr = NULL;
    unsigned char *buff;

#ifdef WIN32
    ptr = (void*)malloc((unsigned int)size);
#else
    ptr = (void*)kmalloc((unsigned int)size, GFP_KERNEL);
#endif

    if (ptr == NULL) 
    {
        return FAIL;
    }
    memset((void *)ptr, 0, (unsigned int)size);/*lint !e506 !e522*/
    buff = (unsigned char*)ptr;
    return buff;
}

int cmux_free(unsigned char *buff)
{
    if (buff == NULL) 
    {
        return FAIL;
    }
#ifdef WIN32
    free(buff);
#else
    kfree(buff);
#endif
    return PASS;
}
/******************************************************************************
Function:      cmux_alloc_skb 
Description:  申请SKB内存(适配BALONG平台) 
Input:           size
Output:         None
Return:         SKB地址
Others:         None
******************************************************************************/
unsigned char * cmux_alloc_skb(int size)
{
    struct sk_buff *pSkb;
    pSkb = dev_alloc_skb(size);
    if ( NULL == pSkb )
    {
        return FAIL;
    }
    (void)memset((void *)pSkb->data, 0, size);/*lint !e506*/
    return (unsigned char *)pSkb;
}
/******************************************************************************
Function:      cmux_put_skb 
Description:  将*src中的长度为size的数据放入到dest中(适配BALONG平台) 
Input:           size
Output:         None
Return:         SKB地址
Others:         None
******************************************************************************/
void cmux_put_skb(unsigned char *dest, unsigned char *src, unsigned int size)
{
    struct sk_buff *pSkb = (struct sk_buff *)dest;
    
    if (( NULL == pSkb ) || ( NULL == src))
    {
        return;
    }
    /*修改SKB的长度*/
    skb_put(pSkb, size);
    /*将源数据拷贝入SKB*/
    memcpy(pSkb->data, src, size);
    
    return (unsigned char *)pSkb;
}
/******************************************************************************
Function:      cmux_get_skb 
Description:  将*src中的长度为size的数据放入到dest中(适配BALONG平台) 
Input:           size
Output:         None
Return:         SKB地址
Others:         None
******************************************************************************/
unsigned char* cmux_get_skb(unsigned char *src)
{
    struct sk_buff *pSkb = (struct sk_buff *)src;
    return (unsigned char *)pSkb->data;
}

/******************************************************************************
Function:      cmux_free_skb 
Description:  释放SKB内存(适配BALONG平台) 
Input:           size
Output:         None
Return:         SKB地址
Others:         None
******************************************************************************/
void cmux_free_skb(unsigned char *buff)
{
    if (NULL == buff)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "buff is NULL!!\n");
        return;
    }
    
    kfree_skb((struct sk_buff *)buff);
    return;
}

/******************************************************************************
Function:      cmux_debug_level 
Description:  设置CMUX log打印级别(适配BALONG平台) 
Input:           level
Output:         None
Return:         unsigned int
Others:         None
******************************************************************************/
unsigned int cmux_debug_level(unsigned int level)
{
    cmux_dbg_level = level;
    return cmux_dbg_level;
}
/******************************************************************************
Function:      cmux_hex_dump 
Description:  打印CMUX的raw数据的接口(适配BALONG平台) 
Input:           level
Output:         None
Return:         unsigned int
Others:         None
******************************************************************************/
void cmux_hex_dump(cmux_debug_status level, unsigned char *buf, unsigned int len)
{
    unsigned int idx;
    
    if (level < cmux_dbg_level)
    {
        return;
    }

    printk(KERN_ERR"cmux_hex_dump addr: %p, len = %d\n", buf, len);
    
    for (idx = 0; idx < len; idx++)
    {
        if (0 == (idx % 16))
        {
            printk(KERN_ERR"0x%04x : ", idx);
        }
        printk(KERN_ERR"%02x ", ((unsigned char)buf[idx]));
        if (15 == (idx % 16))
        {
            printk(KERN_ERR"\n");
        }
    }
    printk(KERN_ERR"\n");
}
#endif /* MBB_HSUART_CMUX */
