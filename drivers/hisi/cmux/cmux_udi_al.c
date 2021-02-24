/******************************************************************************
  Copyright ?, 1988-2014, Huawei Tech. Co., Ltd.
  File name:   cmux_udi_al.c
  Author: shiyanan      Version:  1.0      Date: 20140805
  Description:    cmux lib 适配Balong V3R3平台的UDI机制而写的适配文件。
  该文件的目的在于，向cmux的上层如AT模块提供数据交互
  的接口。
  Others:         NONE
******************************************************************************/

/******************************************************************************
头文件声明
******************************************************************************/
#include <linux/spinlock.h>
#include <linux/kernel.h>       /*kmalloc,printk*/
#include "cmux_udi_al.h"
#include "cmux_api_ifc.h"
#include "cmux_tgt_osal.h"
#include "mdrv_udi.h"
#include "../dialup_hsuart/dialup_hsuart.h"

#if ( FEATURE_ON == MBB_HSUART_CMUX )
/******************************************************************************
变量声明
******************************************************************************/
#ifndef TRUE
#define TRUE 1
#endif
    
#ifndef FALSE
#define FALSE 0
#endif

extern struct cmuxlib * gCmux;
static cmux_ctx cmux_ctx_arr[MAX_CMUX_VPORT_NO] = 
{
    {
        .udi_device_id = UDI_CMUX_AT_ID,/*lint !e65*/
        .DlcIndex = CMUX_UDI_DLC_AT,
    },
    {
        .udi_device_id = UDI_CMUX_MDM_ID,/*lint !e65*/
        .DlcIndex = CMUX_UDI_DLC_MDM,
    },
    {
        .udi_device_id = UDI_CMUX_EXT_ID,/*lint !e65*/
        .DlcIndex = CMUX_UDI_DLC_EXT,
    },
    {
        .udi_device_id = UDI_CMUX_GPS_ID,/*lint !e65*/
        .DlcIndex = CMUX_UDI_DLC_GPS,
    },
#if (FEATURE_ON == MBB_WPG_ECALL)
    {
        .udi_device_id = UDI_CMUX_ECL_ID,/*lint !e65*/
        .DlcIndex = CMUX_UDI_DLC_ECL,
    }
#endif/*MBB_WPG_ECALL*/
};

static UDI_DRV_INTEFACE_TABLE cmux_udi = {
    .udi_open_cb = (UDI_OPEN_CB_T)cmux_udi_open,
    .udi_close_cb = (UDI_CLOSE_CB_T)cmux_udi_close,
    .udi_ioctl_cb = (UDI_IOCTL_CB_T)cmux_udi_ioctl,
};

cmux_close_cb_t close_port_cb;

unsigned int cmux_pluscount = 0;
static const char cmux_switch_cmd[] = "+++";

/******************************************************************************
函数实现
******************************************************************************/
extern int AT_CMUX_CheckRawDataMode(unsigned char priIndex);

/*******************************************************************************
* 函 数 名  : cmux_switch_stage_two
* 功能描述  : CMUX下模式切换第二阶段
* 输入参数  : arg 传参
* 输出参数  : NA
* 返 回 值  : NA
*******************************************************************************/
void cmux_switch_stage_two(unsigned int arg)
{
    cmux_ctx *ctx = (cmux_ctx *)arg;
    ctx->stage = CMUX_STAGE_2;
}

/*******************************************************************************
* 函 数 名  : cmux_switch_stage_three
* 功能描述  : CMUX下模式切换第三阶段
* 输入参数  : arg 传参
* 输出参数  : NA
* 返 回 值  : NA
*******************************************************************************/
void cmux_switch_stage_three(unsigned int arg)
{
    cmux_ctx *ctx = (cmux_ctx *)arg;
    
    ctx->stage = CMUX_STAGE_1;
    if (ctx->cbs.switch_mode_cb)
    {
        ctx->cbs.switch_mode_cb(ctx->DlcIndex);
    }
    else
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "switch_mode_cb is NULL!\r\n");
    }

    bsp_softtimer_add(&(ctx->cmux_state2_timer));
}

/*******************************************************************************
* 函 数 名  : cmux_switch_stage_goback
* 功能描述  : 进入 DETECT_STAGE_1 并起定时器switch_timer  --> switch_stage_two进入 DETECT_STAGE_2
* 输入参数  : arg 传参
* 输出参数  : NA
* 返 回 值  : NA
*******************************************************************************/
void cmux_switch_stage_goback(cmux_ctx *ctx)
{
    /*设置当前处于命令模式切换第一阶段，重新等1S空闲*/
    ctx->stage = CMUX_STAGE_1;
    bsp_softtimer_add(&(ctx->cmux_state2_timer));
}

/*******************************************************************************
* 函 数 名  : cmux_switch_stage_plus
* 功能描述  : CMUX下模式切换plus阶段
* 输入参数  : arg 传参
* 输出参数  : NA
* 返 回 值  : NA
*******************************************************************************/
void cmux_switch_stage_plus(unsigned int arg)
{
    cmux_ctx *ctx = (cmux_ctx *)arg;
    cmux_pluscount = 0;
    cmux_switch_stage_goback(ctx);
}

/*****************************************************************************
* 函 数 名  : cmux_switch_mode_cmd_detect
* 功能描述  : CMUX下模式切换命令检测函数
* 输入参数  : ctx 上下文 pMemNode 数据节点指针
* 输出参数  : NA
* 返 回 值  : 成功/失败
*****************************************************************************/
int cmux_switch_mode_cmd_detect(cmux_ctx *ctx, char *buff, int length)
{
    int result = 0;
    /*入参合法性检查*/
    if(NULL == buff)
    {
        return result;
    }
    
    if (CMUX_STAGE_2 == ctx->stage)
    {
        /*判断是否+或者++*/
        if(((1 == length) || (2 == length))
            && (!memcmp((void*)cmux_switch_cmd, (void*)buff, length)))
        {
            ctx->stage = CMUX_STAGE_PLUS;
            cmux_pluscount = length;
            
            /*起定时器，900ms之后，不再处于等待+号输入状态*/
            bsp_softtimer_add(&(ctx->cmux_stateplus_timer));
        }
        /*判断是否+++*/
        else if((3 == length) 
            && (!memcmp((void*)cmux_switch_cmd, (void*)buff, length)))
        {
            /*起1S钟的超时SoftTimer，（在UART接收数据中断中，判断删除该SoftTimer，若该SoftTimer能触发超时中断处理，则表示后1S无数据下发）*/
            /*设置当前处于命令模式切换第三阶段*/
            bsp_softtimer_add(&(ctx->cmux_state3_timer));
            
            /*保持非UART透传模式下原代码逻辑，
            相当于在非透传模式下"+++"会被丢弃，只修改透传模式下的逻辑*/
            result = AT_CMUX_CheckRawDataMode(ctx->DlcIndex);
        }
        else
        {
            cmux_switch_stage_goback(ctx);
        }
    }
    else if(CMUX_STAGE_PLUS == ctx->stage)
    {
        cmux_pluscount = cmux_pluscount + length;
        
        /*判断是否+或者++ 且g_pluscount == 3*/
        if(((1 == length) || (2 == length)) && (3 == cmux_pluscount)
            && (!memcmp((void*)cmux_switch_cmd, (void*)buff, length)))
        {
            cmux_pluscount = 0;
            
            /*起1S钟的超时SoftTimer，（在UART接收数据中断中，判断删除该SoftTimer，若该SoftTimer能触发超时中断处理，则表示后1S无数据下发）*/
            /*设置当前处于命令模式切换第三阶段*/
            bsp_softtimer_add(&(ctx->cmux_state3_timer));
        }
        /*判断是否第2个+号*/
        else if(((1 == length) || (2 == length)) && (2 == cmux_pluscount)
            && (!memcmp((void*)cmux_switch_cmd, (void*)buff, length)))
        {
            /*刷新定时器，再等900ms*/
            bsp_softtimer_add(&(ctx->cmux_stateplus_timer));
        }
        else
        {
            cmux_switch_stage_goback(ctx);
        }
        
    }
    
    return result;
}

/*****************************************************************************
* 函 数 名  : cmux_mode_switch_config
* 功能描述  : CMUX下模式切换设置
* 输入参数  : ctx 上下文 s32Arg 配置参数
* 输出参数  :
* 返 回 值  :NA
*****************************************************************************/
void cmux_mode_switch_config(cmux_ctx* ctx, void* s32Arg)
{
    if (NULL == s32Arg)
    {
        bsp_softtimer_delete(&(ctx->cmux_state3_timer));	
        bsp_softtimer_delete(&(ctx->cmux_state2_timer));
        bsp_softtimer_delete(&(ctx->cmux_stateplus_timer));
        ctx->stage = NO_CMUX_STAGE;
        ctx->cbs.switch_mode_cb= NULL;
    }
    else
    {
        ctx->stage = CMUX_STAGE_1;
        ctx->cbs.switch_mode_cb = (cmux_switch_mode_cb_t)s32Arg;
        bsp_softtimer_add(&(ctx->cmux_state2_timer));
    }
}

/*****************************************************************************
* 函 数 名  : cmux_timer_init
* 功能描述  : CMUX下的+++功能的timer初始化
* 输入参数  : ctx 上下文
* 输出参数  : NA
* 返 回 值  : PASS/FAIL
*****************************************************************************/
int cmux_timer_init(cmux_ctx *ctx)
{
    struct softtimer_list *statethree_timer = &(ctx->cmux_state3_timer);
    statethree_timer->func = cmux_switch_stage_three;
    statethree_timer->para = (unsigned int)ctx;
    statethree_timer->timeout = CMUX_SW_TIMER_LENGTH;
    statethree_timer->wake_type = SOFTTIMER_NOWAKE;

    struct softtimer_list *statetwo_timer = &(ctx->cmux_state2_timer);
    statetwo_timer->func = cmux_switch_stage_two;
    statetwo_timer->para = (unsigned int)ctx;
    statetwo_timer->timeout = CMUX_SW_TIMER_LENGTH;
    statetwo_timer->wake_type = SOFTTIMER_NOWAKE;

    struct softtimer_list *stateplus_timer = &(ctx->cmux_stateplus_timer);
    stateplus_timer->func = cmux_switch_stage_plus;
    stateplus_timer->para = (unsigned int)ctx;
    stateplus_timer->timeout = CMUX_SW_TIMER_LENGTH;
    stateplus_timer->wake_type = SOFTTIMER_NOWAKE;

    if(OK != bsp_softtimer_create(statethree_timer))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "statethree_timer create fail!\r\n");
    }

    if(OK != bsp_softtimer_create(statetwo_timer))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "statetwo_timer create fail!\r\n");
    }

    if(OK != bsp_softtimer_create(stateplus_timer))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "stateplus_timer create fail!\r\n");
    }
    
    return PASS;
}

/******************************************************************************
Function:       cmux_get_ctx
Description:    获取cmux的ctx，根据index的值。
Input:          unsigned int index
Output:         NONE
Return:         cmux_ctx
Others:         NONE
******************************************************************************/
static cmux_ctx* cmux_get_ctx(unsigned int index)
{
    int loop = 0;
    for ( loop = 0; loop < MAX_CMUX_VPORT_NO; loop++ )
    {
        if ( index == cmux_ctx_arr[loop].DlcIndex )
        {
            break;
        }
    }
    if ( loop >= MAX_CMUX_VPORT_NO )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "CANNOT find ctx with index %d\n", index);
        return NULL;
    }
    return &cmux_ctx_arr[loop];
}

/******************************************************************************
Function:      cmux_init_list
Description:   初始化对应的cmux list
Input:         cmux_trans_info *trans_info
Output:        NONE
Return:        int
Others:        NONE
******************************************************************************/
int cmux_init_list(cmux_trans_info *trans_info)
{
    char* pMemHead = NULL;
    struct list_head *pListNode = NULL;
    unsigned int idx = 0;
    int ret = PASS;
    unsigned long flags = 0;

    if ( NULL == trans_info )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "trans_info is NULL\n");
        ret = FAIL;
        return ret;
    }
    
    spin_lock_irqsave(&trans_info->list_lock, flags);/*lint !e18 !e830*/
    /* 初始化链表 */
    INIT_LIST_HEAD(&(trans_info->free_list));
    INIT_LIST_HEAD(&(trans_info->done_list));
    
    /* 初始化读信息 */
    pMemHead = cmux_alloc(trans_info->node_num * sizeof(cmux_mem_node));
    if (pMemHead)
    {
        trans_info->pmem_start = (cmux_mem_node*)pMemHead;
        trans_info->pcur_pos = NULL;
        trans_info->done_cnt = 0;
        trans_info->total_bytes = 0;

        /* 初始化时,所有节点都在FreeList */
        for (idx = 0; idx < trans_info->node_num; idx++)
        {
            pListNode = &(trans_info->pmem_start[idx].list_node);
            list_add_tail(pListNode, &(trans_info->free_list));
            trans_info->pmem_start[idx].state = CMUX_MEM_IN_FREELIST;
            trans_info->pmem_start[idx].valid_size = 0;
            trans_info->pmem_start[idx].mem_buf = NULL;
        }
    }
    else
    {
        ret = FAIL;
    }
    spin_unlock_irqrestore(&trans_info->list_lock, flags);

    return ret;
}

/******************************************************************************
Function:       cmux_buf_init
Description:   初始化对应的cmux buffer
Input:          unsigned int index
Output:        NONE
Return:        cmux_ctx
Others:        NONE
******************************************************************************/
int cmux_buf_init(cmux_ctx *ctx)
{
    int ret = PASS;

    if ( NULL == ctx )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_ctx is NULL\n");
        ret = FAIL;
        return ret;
    }

    /*lint -e506*/
    /* 初始化读写初始值 */
    (void)memset((void *)&(ctx->read_info), 0, sizeof(cmux_trans_info));
    (void)memset((void *)&(ctx->write_info), 0, sizeof(cmux_trans_info));
    /*lint +e506*/
    /*lint -e133 -e43*/
    spin_lock_init(&(ctx->read_info.list_lock));
    spin_lock_init(&(ctx->write_info.list_lock));
    /*lint +e133 +e43*/
    ctx->read_info.node_num = CMUX_READ_BUF_NUM;
    ctx->read_info.node_size = CMUX_READ_BUF_SIZE;
    ctx->write_info.node_num = CMUX_WRITE_BUF_NUM;
    ctx->write_info.node_size = 0;
    if (!cmux_init_list(&(ctx->read_info)) || !cmux_init_list(&(ctx->write_info)))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_init_list error\n");
        ret = FAIL;
    }
   
    return ret;
}

/******************************************************************************
Function:       cmux_udi_open
Description:   打开cmux udi虚拟设备
Input:          UDI_OPEN_PARAM *param, UDI_HANDLE handle
Output:        NONE
Return:        signed int
Others:        NONE
******************************************************************************/
signed int cmux_udi_open(UDI_OPEN_PARAM_S *param, UDI_HANDLE handle)
{/*lint !e101*/
    cmux_ctx *ctx;

    if ( NULL == param )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "param is NULL\n");
        return -1;
    }

    ctx = cmux_get_ctx((unsigned int)param->pPrivate);

    if ( NULL != ctx)
    {
        ctx->open = TRUE;

        (void)BSP_UDI_SetPrivate(param->devid, (void*)ctx);
    }
    else
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "CANNOT find ctx of %d\n",
            (unsigned int)param->pPrivate);
        return -1;
    }
    return 0;
}
/******************************************************************************
Function:       cmux_udi_close
Description:   关闭cmux udi虚拟设备
Input:          void *DevCtx
Output:        NONE
Return:        signed int
Others:        NONE
******************************************************************************/
signed int cmux_udi_close(void *DevCtx)
{
    cmux_ctx *pCmuxDevCtx = (cmux_ctx *)DevCtx;
    if ( NULL == pCmuxDevCtx )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "DevCtx is NULL!\n");
        return -1;
    }
    pCmuxDevCtx->open = FALSE;

    return 0;
}
/******************************************************************************
Function:       cmux_write_async
Description:   cmux异步写接口
Input:          cmux_ctx *ctx, at_cmux_wr_async_info *pParam
Output:        NONE
Return:        signed int
Others:        NONE
******************************************************************************/
signed int cmux_write_async(cmux_ctx *ctx, at_cmux_wr_async_info *pParam)
{
    unsigned char *buff, *temp_buff;
    unsigned int len, temp_len, send_len, total_len;
    unsigned char dlc;
    int ret = 0;
    
    if ( ( NULL == ctx ) || ( NULL == pParam ) )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR,"InputParam err with ctx = %d pParam = %d\n", 
            ctx, pParam);
        return -1;
    }
    
    if ( NULL != gCmux )
    {
        buff = cmux_get_skb(pParam->pVirAddr);
        len = pParam->u32Size;
        dlc = ctx->DlcIndex;

        if ( NULL == buff )
        {
            CMUX_PRINT(CMUX_DEBUG_ERR,"cmux_get_skb return NULL!\n");
            return -1;
        }

        CMUX_PRINT(CMUX_DEBUG_INFO, "cmux_hex_dump!\r\n");
        cmux_hex_dump(CMUX_DEBUG_INFO, buff, len);
        
        temp_len = len;
        temp_buff = buff;
        total_len = 0;

        while ( 0 != temp_len )
        {
            if ( temp_len > MAX_SERIAL_PKT_LEN )
            {
                send_len = MAX_SERIAL_PKT_LEN;
            }
            else
            {
                send_len = temp_len;
            }
            ret = Cmux_send_dlc_data(gCmux, temp_buff, send_len, dlc, 
                CMUX_FRAGMENT_DATA | CMUX_FLUSH_DATA, 0, 0);
            if (PASS != ret )
            {
                CMUX_PRINT(CMUX_DEBUG_WARN, 
                    "Cmux_send_dlc_data failed with len = %d\n", send_len);
                return -1;
            }
            temp_len -= send_len;
            total_len += send_len;
            temp_buff = buff + total_len;
        }

        if ( ctx->cbs.free_cb )
        {
            ctx->cbs.free_cb((char *)pParam->pVirAddr);
        }
        return 0;
    }
    else 
    {
        CMUX_PRINT(CMUX_DEBUG_INFO,"CMUX function is not on!\n");
        return -1;
    }
}
/******************************************************************************
Function:       cmux_modem_write_signal
Description:    cmux modem状态异步写接口
Input:          cmux_ctx *ctx, cmux_modem_msc_stru *pParam
Output:         NONE
Return:         signed int
Others:         NONE
******************************************************************************/
extern unsigned int CMUX_HSUART_WriteMscCmd(cmux_modem_msc_stru *pstDceMsc);
signed int cmux_modem_write_signal(cmux_ctx *ctx, cmux_modem_msc_stru *pParam)
{
    cmux_modem_msc_stru *msc_stru;
    unsigned char dlc;
    struct cmuxlib *cm = (struct cmuxlib *) gCmux;
    int ret = PASS;
    int is_send_cmd = FALSE;

    if (NULL == cm )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR,"CMUX function is not on!\n");
        return -1;
    }
    if ( ( NULL == ctx ) || ( NULL == pParam ) )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR,"InputParam err with ctx = %d pParam = %d\n", 
            ctx, pParam);
        return -1;
    }
    msc_stru = (struct cmux_modem_msc_stru *)pParam;/*lint !e565*/
    dlc = ctx->DlcIndex;

    /* 设置硬件管脚 */
    ret = CMUX_HSUART_WriteMscCmd(msc_stru);
    if (FAIL == ret)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR,"HSUART Write Msc Cmd failed!\n");
    }

    if (NULL == cm->dlc_db [dlc])
    {
        CMUX_PRINT(CMUX_DEBUG_ERR,"DLC is not established!\n");
        return -1;
    }

    /*如果是RING管脚变化，DLC置为DLC0*/
    if (1 == msc_stru->OP_Ri)
    {
        cm->dlc_db [dlc]->msc.dlci = 0;
    }
    else
    {
        cm->dlc_db [dlc]->msc.dlci = dlc;
    }

    /* 当DCD信号有改变，为高电平时，msc的dv位置1，反之置0 */
    CMUX_PRINT(CMUX_DEBUG_INFO,"DCD change is %d!\n", msc_stru->OP_Dcd);
    if (1 == msc_stru->OP_Dcd)
    {
        if ( cm->dlc_db [dlc]->msc.dv != msc_stru->ucDcd )
        {
            if (1 == msc_stru->ucDcd)
            {
                cm->dlc_db [dlc]->msc.dv = 1;
            }
            else
            {
                cm->dlc_db [dlc]->msc.dv = 0;
            }
            is_send_cmd = TRUE;
        }
    }

    /* 当DSR信号有改变，为高电平时，msc的rtc位置1，反之置0 */
    CMUX_PRINT(CMUX_DEBUG_INFO,"DSR change is %d!\n", msc_stru->OP_Dsr);
    if (1 == msc_stru->OP_Dsr)
    {
        if ( cm->dlc_db [dlc]->msc.rtc != msc_stru->ucDsr )
        {
            if (1 == msc_stru->ucDsr)
            {
                cm->dlc_db [dlc]->msc.rtc = 1;
            }
            else
            {
                cm->dlc_db [dlc]->msc.rtc = 0;
            }
            is_send_cmd = TRUE;
        }
    }

    /* 当RING信号有改变，为高电平时，msc的ic位置1，反之置0 */
    CMUX_PRINT(CMUX_DEBUG_INFO,"RI change is %d!\n", msc_stru->OP_Ri);
    if (1 == msc_stru->OP_Ri)
    {
        if (1 == msc_stru->ucRi)
        {
            cm->dlc_db [dlc]->msc.ic = 1;
            is_send_cmd = TRUE;
        }
        else
        {
            /* RING变低电平时，无需上报 */
            cm->dlc_db [dlc]->msc.ic = 0;
        }
    }
    
    /* 当CTS信号有改变，需要发送FCON/FCOFF命令*/
    CMUX_PRINT(CMUX_DEBUG_INFO,"CTS change is %d!\n", msc_stru->OP_Cts);
    if (1 == msc_stru->OP_Cts)
    {
        if ( cm->flow_ctrl != msc_stru->ucCts )
        {
            if (1 == msc_stru->ucCts)
            {
                CMUX_PRINT(CMUX_DEBUG_INFO,"going to send FCON\n");
                ret = Cmux_send_fcon_fcoff(cm, 0, FCON_CMD);
                cm->flow_ctrl = 1;
            }
            else
            {
                CMUX_PRINT(CMUX_DEBUG_INFO,"going to send FCOFF\n");
                ret = Cmux_send_fcon_fcoff(cm, 0, FCOFF_CMD);
                cm->flow_ctrl = 0;
            }
        }
    }
    /* 发送msc命令上报管脚状态 */
    if (TRUE == is_send_cmd)
    {
        ret = Cmux_send_msc (cm, dlc, &(cm->dlc_db [dlc]->msc));
        if (FAIL == ret)
        {
            CMUX_PRINT(CMUX_DEBUG_ERR,"MSC is sent failed!\n");
        }
    }

    return 0;
}
/******************************************************************************
Function:       cmux_add_mem_to_freelist
Description:   将使用过的memnode重新放入freelist里面。
Input:         
Output:        NONE
Return:        signed int
Others:        NONE
******************************************************************************/
void cmux_add_mem_to_freelist
(
    cmux_trans_info *trans_info, 
    cmux_mem_node* pMemNode
)
{
    unsigned long flags;
    
    spin_lock_irqsave(&trans_info->list_lock, flags);
    
    list_add_tail(&pMemNode->list_node, &(trans_info->free_list));
    /*去初始化pMemNode成员*/
    pMemNode->state = CMUX_MEM_IN_FREELIST;
    pMemNode->valid_size = 0;
    pMemNode->mem_buf = NULL;
    
    spin_unlock_irqrestore(&trans_info->list_lock, flags);
}
/******************************************************************************
Function:       cmux_get_mem_from_freelist
Description:   从freelist里面找到第一个可用的memnode。
Input:          cmux_trans_info *trans_info
Output:        NONE
Return:        cmux_mem_node*
Others:        NONE
******************************************************************************/
cmux_mem_node* cmux_get_mem_from_freelist(cmux_trans_info *trans_info)
{
    struct list_head *pListNode;
    cmux_mem_node* pMemNode;
    unsigned long flags = 0;
    
    spin_lock_irqsave(&trans_info->list_lock, flags);
    
    if (list_empty(&(trans_info->free_list)))
    {
        pMemNode = NULL;
    }
    else
    {
        pListNode = trans_info->free_list.next;
        pMemNode = list_entry(pListNode, cmux_mem_node, list_node);
        
        list_del_init(pListNode);
        pMemNode->state = CMUX_MEM_NOT_IN_LIST;
    }
    
    spin_unlock_irqrestore(&trans_info->list_lock, flags);
    
    return pMemNode;
}

/******************************************************************************
Function:       cmux_get_read_node
Description:   将CMUX的数据保存在donelist里面等待AT获取。
Input:          char *buff, int length
Output:        NONE
Return:        int
Others:        NONE
******************************************************************************/
cmux_mem_node* cmux_get_read_node(cmux_ctx *ctx)
{
    cmux_mem_node* pMemNode;
    
    pMemNode = cmux_get_mem_from_freelist(&(ctx->read_info));
    
    if (pMemNode)
    {
        if (!pMemNode->mem_buf)
        {
            pMemNode->mem_buf = cmux_alloc_skb(ctx->read_info.node_size);
            if ( NULL == pMemNode->mem_buf )
            {
                cmux_add_mem_to_freelist(&(ctx->read_info), pMemNode);
                pMemNode = NULL;
                ctx->stat.alloc_mem_fail++;
            }
        }
        ctx->read_info.pcur_pos = pMemNode;
    }
    
    return pMemNode;
}

/******************************************************************************
Function:       cmux_add_mem_to_donelist
Description:   将CMUX的数据保存在donelist里面等待AT获取。
Input:          char *buff, int length
Output:        NONE
Return:        int
Others:        NONE
******************************************************************************/
void cmux_add_mem_to_donelist
(
    cmux_trans_info *trans_info, 
    cmux_mem_node* pMemNode
)
{
    unsigned long flags = 0;
    /*入参合法性检查*/
    spin_lock_irqsave(&trans_info->list_lock, flags);

    list_add_tail(&pMemNode->list_node, &(trans_info->done_list));
    trans_info->done_cnt++;
    pMemNode->state = CMUX_MEM_IN_DONELIST;
    
    spin_unlock_irqrestore(&trans_info->list_lock, flags);
}

/******************************************************************************
Function:       cmux_data_rx_complete
Description:   将CMUX的数据保存在donelist里面等待AT获取。
Input:          char *buff, int length
Output:        NONE
Return:        int
Others:        NONE
******************************************************************************/
int cmux_data_rx_complete(cmux_ctx *ctx, char *buff, int length)
{
    cmux_mem_node* pMemNode;
    int ret;
    
    if (strlen(cmux_switch_cmd) >= length)
    {
        ret = cmux_switch_mode_cmd_detect(ctx, buff, length);
        if (ret)
        {
            return FAIL;
        }
    }
    else
    {
        cmux_switch_stage_goback(ctx);
    }
    
    pMemNode = cmux_get_read_node(ctx);
    if ( NULL == pMemNode )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "CANNOT get buf from freelist\n");
        return FAIL;
    }

    cmux_put_skb(pMemNode->mem_buf, buff, length);
    pMemNode->valid_size = length;
    cmux_add_mem_to_donelist(&ctx->read_info, pMemNode);
    return PASS;
}

/******************************************************************************
Function:       cmux_callback_func
Description:   cmux向AT/PPP/GPS发送数据的接口
Input:         
Output:        NONE
Return:        signed int
Others:        NONE
******************************************************************************/
void cmux_callback_func(unsigned int cbtype, void* ptr, struct cmux_timer* ct)
{
    unsigned int DlcIndex;
    struct frame *  rxfr = NULL;
    char *pdata = NULL;
    int length = 0;
    cmux_ctx *ctx = NULL;

    CMUX_PRINT(CMUX_DEBUG_INFO, "rcved type %d \n", cbtype);
    if ( ( TE_SENT | RX_COMPLETE ) == cbtype )
    {
        rxfr = (struct frame *) ptr;
        if ( NULL == rxfr )
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "FRAME PTR is NULL\n");
            return;
        }
        pdata = (char *)rxfr->uih_data;
        if ( NULL == pdata )
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "pdata is NULL\n");
            return;
        }
        
        length = (int)rxfr->length;
        DlcIndex = rxfr->dlc;
        ctx = cmux_get_ctx(DlcIndex);
        if ( NULL == ctx )
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_ctx is NULL\n");
            return;
        }

        (void)bsp_softtimer_delete(&(ctx->cmux_state3_timer));
        (void)bsp_softtimer_delete(&(ctx->cmux_state2_timer));
        (void)bsp_softtimer_delete(&(ctx->cmux_stateplus_timer));

        if ( FAIL == cmux_data_rx_complete(ctx, pdata, length) )
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_data_rx_complete failed\n");
            return;
        }
        if ( ctx->cbs.read_cb )
        {
            CMUX_PRINT(CMUX_DEBUG_INFO, "read_cb DlcIndex %d\n", ctx->DlcIndex);
            ctx->cbs.read_cb(ctx->DlcIndex);
        }
        return;
    }
    else if ( DLC_ESTABLISH_COMPLETE == cbtype )
    {
        DlcIndex = (unsigned int) ptr;
        ctx = cmux_get_ctx(DlcIndex);
        
        if ( NULL == ctx )
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "DLC_ESTABLISH_COMPLETE ctx NULL %d\n", DlcIndex);
            return;
        }
        
        if (ctx->cbs.msc_mode_read_cb)
        {
            CMUX_PRINT(CMUX_DEBUG_INFO, "msc_mode_read_cb DlcIndex %d\n", ctx->DlcIndex);
            ctx->cbs.msc_mode_read_cb(ctx->DlcIndex);
        }
        return;
    }
    else if ( AT_STATE_CHANGE == cbtype )
    {
        struct snc_cmd *snc;
        unsigned char enable;
        
        snc = (struct snc_cmd *) ptr;
        DlcIndex = snc->dlci;
        enable = snc->s1;
        
        ctx = cmux_get_ctx(DlcIndex);
        if ( NULL == ctx )
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "AT_STATE_CHANGE ctx NULL %d\n", DlcIndex);
            return;
        }
        if (ctx->cbs.snc_mode_cb)
        {
            CMUX_PRINT(CMUX_DEBUG_INFO, 
                "snc_mode_cb DlcIndex=%d, enable=%d\n", ctx->DlcIndex, enable);
            ctx->cbs.snc_mode_cb(ctx->DlcIndex, enable);
        }
        return;
    }
    else
    {
        CMUX_PRINT(CMUX_DEBUG_INFO, "cbtype 0x%x non_support, return\n", cbtype);
        return;
    }
}

/******************************************************************************
Function:       cmux_send_msc_status
Description:    cmux向AT侧发送msc状态
Input:          unsigned char ucIndex
                cmux_modem_msc_stru* pModemMsc
Output:         NONE
Return:         NONE
Others:         NONE
******************************************************************************/
void cmux_send_msc_status(unsigned char Index, cmux_modem_msc_stru* pModemMsc)
{
    cmux_ctx *ctx = NULL;

    if (NULL == pModemMsc)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "pModemMsc is NULL\n");
        return;
    }

    if (CMUX_UDI_DLC_CTL == Index)
    {
        /* 硬件DTR消息使用DLC 1来生成context承载 */
        ctx = cmux_get_ctx(CMUX_UDI_DLC_AT);
    }
    else
    {
        ctx = cmux_get_ctx(Index);
    }
    
    if (NULL == ctx )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_ctx is NULL\n");
        return;
    }

    /* 向AT侧上层发送设置管脚消息 */
    if (ctx->cbs.msc_read_cb)
    {
        ctx->cbs.msc_read_cb(Index, (void *)pModemMsc);
    }
    return;
}
/******************************************************************************
Function:       cmux_get_mem_from_donelist
Description:   返回cmux的donelist里面的buffer。
Input:         
Output:        NONE
Return:        cmux_mem_node*
Others:        NONE
******************************************************************************/
cmux_mem_node* cmux_get_mem_from_donelist(cmux_trans_info *trans_info)
{
    struct list_head *pListNode;
    cmux_mem_node* pMemNode;
    unsigned long flags;

    spin_lock_irqsave(&trans_info->list_lock, flags);
    
    if (list_empty(&(trans_info->done_list)))
    {
        pMemNode = NULL;
    }
    else
    {
        pListNode = trans_info->done_list.next;
        pMemNode = list_entry(pListNode, cmux_mem_node, list_node);
        list_del_init(pListNode);
        trans_info->done_cnt--;
        pMemNode->state = CMUX_MEM_NOT_IN_LIST;
    }
    
    spin_unlock_irqrestore(&trans_info->list_lock, flags);
    
    return pMemNode;
}
/******************************************************************************
Function:       cmux_get_read_buffer
Description:   返回cmux的donelist里面的buffer。
Input:         
Output:        NONE
Return:        signed int
Others:        NONE
******************************************************************************/
int cmux_get_read_buffer(cmux_ctx *ctx, at_cmux_wr_async_info *pWRInfo)
{
    cmux_mem_node* pMemNode = NULL;
    //unsigned long flags = 0;
    
    if (NULL == ctx || NULL == pWRInfo)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_ctx:0x%x   pWRInfo:0x%x.\n",
            (unsigned int)ctx, (unsigned int)pWRInfo);
        return -1;
    }

    /* 获取完成的 buffer节点 */
    pMemNode = cmux_get_mem_from_donelist(&(ctx->read_info));

    if (pMemNode)
    {
        /*数据所指向的数据*/
        pWRInfo->pVirAddr = (char *)pMemNode->mem_buf;
        /*数据的大小*/
        pWRInfo->u32Size = (unsigned int)pMemNode->valid_size;
        /*当前信息头指针*/
        pWRInfo->pDrvPriv = (void*)pMemNode->mem_buf;
        
        cmux_add_mem_to_freelist(&(ctx->read_info), pMemNode);

        pMemNode->mem_buf = cmux_alloc_skb(ctx->read_info.node_size);
    }
    else
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "no done mem node in list\n");
        return -1;
    }

    return 0;
}
/******************************************************************************
Function:       cmux_return_read_buffer
Description:   释放AT层使用完毕的内存。
Input:          void *DevCtx
Output:        NONE
Return:        signed int
Others:        NONE
******************************************************************************/
int cmux_return_read_buffer
(
    cmux_ctx* ctx, 
    at_cmux_wr_async_info* pWRInfo
)
{
    if (( NULL == ctx ) || ( NULL == pWRInfo ))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_ctx:0x%x   pWRInfo:0x%x.\n",
            (unsigned int)ctx, (unsigned int)pWRInfo);
        return -1;
    }

    cmux_free_skb((unsigned char *)pWRInfo->pVirAddr);

    return 0;
}

/******************************************************************************
Function:       cmux_udi_ioctl
Description:   处理cmux udi虚拟设备ioctl命令的函数。
Input:          void *DevCtx
Output:        NONE
Return:        signed int
Others:        NONE
******************************************************************************/
signed int cmux_udi_ioctl(void *pPrivate, unsigned int u32Cmd, void *pParam)
{
    cmux_ctx* ctx = (cmux_ctx*)pPrivate;
    signed int ret = 0;
    if ( NULL == ctx )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_ctx is NULL!\n");
        ret = -1;
        return ret;
    }
    
    if (!ctx->open)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_ctx is not open!\n");
        ret = -1;
        return ret;
    }
    
    switch(u32Cmd)
    {
        /* 下行异步写接口 */
        case CMUX_IOCTL_WRITE_ASYNC:
            {
                ret = cmux_write_async(ctx, (at_cmux_wr_async_info *)pParam);
            }
            break;
        /* NAS注册下行数据buf 释放函数 */
        case CMUX_IOCTL_SET_FREE_CB:
            {
                ctx->cbs.free_cb = (cmux_free_cb_t)pParam;
            }
            break;
        /* 收到数据后调用此注册接口通知上层接收 */
        case CMUX_IOCTL_SET_READ_CB:
            {
                ctx->cbs.read_cb = (cmux_read_cb_t)pParam;
            }
            break;
        /* 上层调用此接口获得上行buf 地址，上层在我们的read回调函数中来实现的 */
        case CMUX_IOCTL_GET_RD_BUFF:
            {               
                ret = cmux_get_read_buffer(ctx, (at_cmux_wr_async_info*)pParam);
            }
            break;
        /* 上层调用此接口释放上行buf */
        case CMUX_IOCTL_RETURN_BUFF:
            {
                ret = cmux_return_read_buffer(ctx, (at_cmux_wr_async_info*)pParam);
            }
            break;
        /* 上层调用此接口关闭多路复用模式 */
        case CMUX_IOCTL_CLOSE_PORT_CB:
            {
                close_port_cb = (cmux_close_cb_t)pParam;
            }
            break;
        /* 写modem管脚信号，参数为pMODEM_MSC_STRU */
        case CMUX_IOCTL_MSC_WRITE_CMD:
            {
                ret = cmux_modem_write_signal(ctx, (cmux_modem_msc_stru*)(pParam));
            }
            break;
        /* 注册管脚变化通知回调参数为pMODEM_MSC_STRU 指向本地全局 */
        case CMUX_IOCTL_SET_MSC_READ_CB:
            {
                ctx->cbs.msc_read_cb = (cmux_msc_read_cb_t)pParam;
            }
            break;
        /* 注册管脚模式查询回调 */
        case CMUX_IOCTL_MSC_MODE_READ_CB:
            {
                ctx->cbs.msc_mode_read_cb = (cmux_msc_mode_read_cb_t)pParam;
            }
            break;
        /* +++切换命令模式回调，设置NULL关闭该功能 */
        case CMUX_IOCTL_SWITCH_MODE_CB:
            {
                cmux_mode_switch_config(ctx, pParam);
            }
            break;
        case CMUX_IOCTL_SNC_CB:
            {
                ctx->cbs.snc_mode_cb = (cmux_snc_mode_cb_t)pParam;
            }
            break;      
        default:
            CMUX_PRINT(CMUX_DEBUG_ERR, "unknow s32Cmd: 0x%x.\n", u32Cmd);
            ret = -1;
            break;
    }

    return ret;
}

/******************************************************************************
Function:       cmux_udi_init
Description:    初始化cmux udi虚拟设备的函数。
Input:          void
Output:         NONE
Return:         Pass :成功
                Fail :失败
Others:         NONE
******************************************************************************/
int cmux_udi_init(void)
{
    int loop = 0;
    cmux_ctx *ctx = NULL;
    
    for ( loop = 0; loop < MAX_CMUX_VPORT_NO; loop++ )
    {
        ctx = &cmux_ctx_arr[loop];
        
        ctx->init = FALSE;
        
        /*UDI层的初始化*/
        (void)BSP_UDI_SetCapability(ctx->udi_device_id, 0);
        (void)BSP_UDI_SetInterfaceTable(ctx->udi_device_id, &cmux_udi);
        
        if ( PASS != cmux_buf_init(ctx))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_buf_init is error!\n");
            return FAIL;
        }

        if ( PASS != cmux_timer_init(ctx))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_timer_init is error!\n");
            return FAIL;
        }
        
        ctx->init = TRUE;
    }
    return PASS;
}
module_init(cmux_udi_init);
#endif/*MBB_HSUART_CMUX*/
