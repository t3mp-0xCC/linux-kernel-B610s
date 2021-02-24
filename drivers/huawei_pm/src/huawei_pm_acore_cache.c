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



#include "huawei_pm.h"
#include <linux/slab.h>
#include <linux/netlink.h>

/*The buffer for store responses*/
/*It's only accessed in this file,
other modules can't visit it directly*/

/*电源管理A核LOG打印开关，默认关闭*/
unsigned int g_PmAcorePrintOnOff = PM_MSG_PRINT_OFF;

huawei_pm_rsp_proc_type huawei_pm_acore_data_buf[HUAWEI_PM_RSP_MAX_CACHE_SIZE];

/*The cache queue for store responses*/
/*It's only accessed in this file,
other modules can't visit it directly*/
huawei_pm_rsp_cache_type huawei_pm_acore_data_cache = 
{
    0,
    0,
    huawei_pm_acore_data_buf
};
huawei_pm_rsp_info_type huawei_pm_acore_rsp_info_table[] = 
{   /*      Msg_id                       RMT_WK_type         CURC_type        cache wether repeat  */
    {HUAWEI_PM_RSP_ID_VOICE,            RMT_WK_SRC_VOICE,  HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_SMS,              RMT_WK_SRC_SMS,    HUAWEI_PM_RSP_GROUP_NONE,    TRUE},
    {HUAWEI_PM_RSP_ID_TCPIP_DATA,       RMT_WK_SRC_DATA,   HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_ASYN_CMD,         RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_RESULT_CODE,      RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_MODE,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_CURC01,  FALSE},
    {HUAWEI_PM_RSP_ID_UR_RSSI,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_CURC01,  FALSE},
    {HUAWEI_PM_RSP_ID_UR_CSNR,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_CURC01,  FALSE},
    {HUAWEI_PM_RSP_ID_UR_SRVST,         RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_REG,           RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_SIMST,         RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_NWTIME,        RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_ACTIVEBAND,    RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_CURC01,  FALSE},
    {HUAWEI_PM_RSP_ID_UR_ANLEVEL,       RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_CURC01,  FALSE},
    {HUAWEI_PM_RSP_ID_UR_LOCCHD,        RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_SIMFILEREFRESH, RMT_WK_SRC_UR,    HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_SMMEMFULL,     RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_POSITION,      RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_GPSBASIC,      RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_WNINV,         RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE}, 
    {HUAWEI_PM_RSP_ID_UR_POSEND,        RMT_WK_SRC_GPS,    HUAWEI_PM_RSP_GROUP_NONE,    TRUE},
    {HUAWEI_PM_RSP_ID_UR_WPDCP,         RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_WPDDL,         RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_WPDOP,         RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_CTZV,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_BOOT,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_CURC01,  FALSE},
    {HUAWEI_PM_RSP_ID_UR_NDISEND,       RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_DSFLOWRPT,     RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_CURC01,  FALSE},
    {HUAWEI_PM_RSP_ID_UR_EARST,         RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_CURC01,  FALSE},
    {HUAWEI_PM_RSP_ID_UR_ORIG,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_CONF,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_CONN,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_CEND,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_RFSWITCH,      RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_STIN,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_CUSD,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_SMS,           RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    TRUE},
    {HUAWEI_PM_RSP_ID_UR_RSSILVL,       RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_CURC01,  FALSE},
    {HUAWEI_PM_RSP_ID_UR_HRSSILVL,      RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_CURC01,  FALSE},
    {HUAWEI_PM_RSP_ID_UR_HDRRSSI,       RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_CURC01,  FALSE},
    {HUAWEI_PM_RSP_ID_UR_CRSSI,         RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_CURC01,  FALSE},
    {HUAWEI_PM_RSP_ID_UR_OTACMSG,       RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_DSDORMANT,     RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_IPDATA,        RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    TRUE},
    {HUAWEI_PM_RSP_ID_UR_THERM,         RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_XDSTATUS,      RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},        
    {HUAWEI_PM_RSP_ID_UR_CLIP,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE}, 
    {HUAWEI_PM_RSP_ID_UR_CCWA,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE}, 
    {HUAWEI_PM_RSP_ID_UR_CSSI,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE}, 
    {HUAWEI_PM_RSP_ID_UR_CSSU,          RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},   
    {HUAWEI_PM_RSP_ID_UR_IPSTATE,       RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE}, 
    {HUAWEI_PM_RSP_ID_UR_CUSATP,        RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_CUSATEND,      RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_ECLSTAT,       RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_ECCLIST,       RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},     
    {HUAWEI_PM_RSP_ID_UR_FOTASTATE,     RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_NDISSTAT,      RMT_WK_SRC_UR,     HUAWEI_PM_RSP_GROUP_NONE,    FALSE},
    {HUAWEI_PM_RSP_ID_UR_GPS,           RMT_WK_SRC_GPS,    HUAWEI_PM_RSP_GROUP_NONE,    TRUE},
};

extern dsat_curc_status huawei_pm_get_curc_status(void);
extern boolean huawei_pm_wakeup_remote_host(rmt_wk_src_type rmt_wk_src);
extern boolean huawei_pm_get_usb_suspended_status(void);
extern void huawei_pm_request_wakeup_pin(void);
extern void huawei_pm_update_wakeup_channel_status(void);
extern int huawei_pm_dispatch_uspace_data(void *buf, int32 buf_len, int32 port_idx);
extern void huawei_pm_remote_wakeup_timer(void);
extern void huawei_pm_hsuart_rts_gpio_shake(void);


unsigned int AcoreLogOnOffSet(unsigned int print_onoff)
{
    g_PmAcorePrintOnOff = print_onoff;
    return g_PmAcorePrintOnOff;
}


unsigned int AcoreLogOnOffGet(void)
{
    return g_PmAcorePrintOnOff;
}
/******************************************************************************
Function:       huawei_pm_rsp_find_id_acore
Description:   
Input:           huawei_pm_rsp_id_type send_id
Output:         None
Return:          None
Others:          None
******************************************************************************/
static uint32 huawei_pm_rsp_find_id_acore(huawei_pm_rsp_id_type send_id)
{
    uint32 i = 0;

    /*ARR_SIZE(huawei_pm_acore_rsp_info_table) is the size 
      of huawei_pm_acore_rsp_info_table*/
    for ( i = 0; 
            i < sizeof(huawei_pm_acore_rsp_info_table) / sizeof(huawei_pm_acore_rsp_info_table[0]);
            i++)
    {
        if (send_id == huawei_pm_acore_rsp_info_table[i].id)
        {
            return i;
        }
    }

    return HUAWEI_PM_RSP_ERROR_ID;
}

/******************************************************************************
Function:       huawei_pm_acore_data_store
Description:   
Input:           VOS_UINT32 index, VOS_PID Pid, VOS_VOID *pMsg
Output:         None
Return:          None
Others:          None
******************************************************************************/
static void huawei_pm_acore_data_store
(
    uint32 index,
    void *pBuf,
    int32 length,
    int32 port_idx
)
{
    uint32 i = 0;
    /*The current pos*/
    uint32 pos = 0;     
    /*Used to free old memory*/
    huawei_pm_rsp_proc_type *tmp_ptr = NULL;
    void *tmp_buf = NULL;

    tmp_buf = kzalloc(length, GFP_KERNEL);
    if(NULL == tmp_buf)
    {
        return;
    }
    memcpy(tmp_buf, pBuf, length);
    
    if (FALSE == huawei_pm_acore_rsp_info_table[index].duplicate)
    {
        /*The current pos*/
        pos = huawei_pm_acore_data_cache.head; 
    
        /*Check id whether exist*/
        for (i = 0; i < HUAWEI_PM_RSP_CACHE_LEN(huawei_pm_acore_data_cache); i++)
        {
            if ((huawei_pm_acore_data_cache.buf_array[pos].id)
                    == huawei_pm_acore_rsp_info_table[index].id)
            {
                /*Find,We overlay this position*/
                /*Free old memory*/
                tmp_ptr = &huawei_pm_acore_data_cache.buf_array[pos];
                /*TO DO : Free the memory!*/
                kfree(tmp_ptr->pBuf);
                //VOS_FreeMsg( tmp_ptr->pid, tmp_ptr->pMsg );
                /*Update the response*/
                huawei_pm_acore_data_cache.buf_array[pos].pBuf = tmp_buf;
                huawei_pm_acore_data_cache.buf_array[pos].pBuf_len = length;
                huawei_pm_acore_data_cache.buf_array[pos].port_idx = port_idx;
                return;
            }
            else
            {
                /*The next response*/
                pos = HUAWEI_PM_RSP_CACHE_NEXT(pos);                
            }
        }    
    }

    /*The default store position*/
    pos = huawei_pm_acore_data_cache.tail;
    
    /*Cache full,overlay the oldest*/
    if (HUAWEI_PM_RSP_CACHE_FULL(huawei_pm_acore_data_cache))
    {
        /*Free old memory*/
        tmp_ptr 
          = &huawei_pm_acore_data_cache.buf_array[huawei_pm_acore_data_cache.head];
        /*TO DO : Free the memory!*/
        //VOS_FreeMsg( tmp_ptr->pid, tmp_ptr->pMsg );
        kfree(tmp_ptr->pBuf);
        tmp_ptr->pBuf = NULL;
        tmp_ptr->pBuf_len = 0;
        huawei_pm_acore_data_cache.head 
                = HUAWEI_PM_RSP_CACHE_NEXT(huawei_pm_acore_data_cache.head);
    }

    /*Store the response*/
    huawei_pm_acore_data_cache.buf_array[pos].pBuf = tmp_buf;
    huawei_pm_acore_data_cache.buf_array[pos].pBuf_len = length;
    huawei_pm_acore_data_cache.buf_array[pos].port_idx = port_idx;
    /*Update tail index*/
    huawei_pm_acore_data_cache.tail 
        = HUAWEI_PM_RSP_CACHE_NEXT(huawei_pm_acore_data_cache.tail);
    
}

/******************************************************************************
  函数名称  : huawei_pm_usb_is_present()
  功能描述  : 判断usb是否在位
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值      : 1-待机状态 0-非待机状态,                
********************************************************************************/
boolean huawei_pm_usb_is_present(void) 
{
    BSP_U32 usb_status = 0;
    pm_status_s *pm_st = (pm_status_s *)(SRAM_MODULE_PM_ADDR);

    usb_status = pm_st->usb_status;

    if(( USB_SUSPEND_STATUS != usb_status ) && ( USB_RESUME_STATUS !=  usb_status ))
    {
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff,"usb is not present !\n");
        return FALSE;
    }
    else
    {
        PRINT_ACORE_LOG(g_PmAcorePrintOnOff,"usb is present !\n");
        return TRUE;
    }
    
}

/******************************************************************************
  函数名称  : huawei_pm_remote_check_And_handle_acore()
  功能描述  : 判断是否需要通过PIN脚或USB唤醒主机。
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值      : 无
********************************************************************************/
void huawei_pm_remote_check_And_handle_acore
(
    unsigned char en_wakeup_host,
    unsigned char *store
)
{
    unsigned char pin_channel = FALSE;
    unsigned char usb_channel = FALSE;
    
    /*USB在位且非suspend情况下，不用远程唤醒*/
    if((TRUE == huawei_pm_usb_is_present()) \
        && (FALSE == huawei_pm_get_usb_suspended_status()))
    {
        /*非suspend模式下，不用输出1s高电平*/
        return;
    }
    
    /*如果唤醒源不允许唤醒主机，不处理*/
    if(FALSE == en_wakeup_host)
    {
        return;
    }
    
    pin_channel = huawei_pm_get_wakeup_channel_status(RMT_WK_CHN_PIN);
    usb_channel = huawei_pm_get_wakeup_channel_status(RMT_WK_CHN_USB);
    
    if ( TRUE == usb_channel )
    {
        huawei_pm_update_wakeup_channel_status();
    }
    
    if( FALSE == huawei_pm_get_pin_suspended_status() )
    {
        return;
    }
    
    /*允许作为远程唤醒源，且支持PIN方式远程唤醒*/
    if ( TRUE == pin_channel )
    {
        huawei_pm_request_wakeup_pin();
        /*模块和上位机握手,起超时定时器*/
        huawei_pm_remote_wakeup_timer();
    }

    /*只有PIN唤醒源，消息需要先缓存然后等待握手后发出*/
    if ( (TRUE == pin_channel) && ( FALSE == usb_channel ) )
    {
        *store = TRUE;
    }
    else
    {
        *store = FALSE;
    }
    return;
}

/******************************************************************************
  函数名称  : huawei_pm_process_acore_data_ex()
  功能描述  : 判断是否(curc = 2)控制的主动上报命令
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值      : HUAWEI_PM_RSP_ROUTE_STORE,                
                            : HUAWEI_PM_RSP_ROUTE_DISCARD,            
                            : HUAWEI_PM_RSP_ROUTE_TRANSFER,           
                            : HUAWEI_PM_RSP_ROUTE_MAX,
********************************************************************************/
huawei_pm_rsp_route_type huawei_pm_process_acore_data_ex(huawei_pm_rsp_id_type send_id) 
{
    /*The index in huawei_pm_acore_rsp_info_table*/
    uint32 idx = 0;
    /*Whether or not wake host*/
    boolean wakeup_host = FALSE;                         
    /*The wakeup source type of response*/
    rmt_wk_src_type wake_source = RMT_WK_SRC_MAX;   
    /*Current curc status*/
    dsat_curc_status curc_val = DSAT_UR_ENABLE;   
    boolean usb_store = FALSE;  
    /*Check params validity*/
    if ((HUAWEI_PM_RSP_ID_UR_NONE >= send_id) 
          || (HUAWEI_PM_RSP_ID_UR_MAX <= send_id))
    {
        return HUAWEI_PM_RSP_ROUTE_TRANSFER;
    }

    if ( ( HUAWEI_PM_RSP_ID_RESULT_CODE == send_id ) || \
    ( HUAWEI_PM_RSP_ID_ASYN_CMD == send_id ) )
    {
        huawei_pm_remote_check_And_handle_acore(TRUE, &usb_store);
        if ( TRUE == usb_store)
        {
            return HUAWEI_PM_RSP_ROUTE_STORE;
        }
        return HUAWEI_PM_RSP_ROUTE_TRANSFER;
    }
    
    /*Get index*/
    idx = huawei_pm_rsp_find_id_acore(send_id);
    if (HUAWEI_PM_RSP_ERROR_ID == idx)
    {
        return HUAWEI_PM_RSP_ROUTE_DISCARD;
    }

    /*Get wakeup source*/
    wake_source = huawei_pm_acore_rsp_info_table[idx].wake_source;
    /*Get curc status*/
    curc_val = huawei_pm_get_curc_status();
    /*Wakup host check*/
    wakeup_host = huawei_pm_wakeup_remote_host(wake_source);

    PRINT_ACORE_LOG(g_PmAcorePrintOnOff,"#wake_source = %d\n"
        "#curc_val = %d\n#wakeup_host = %d\n", wake_source, curc_val, wakeup_host);
    PRINT_ACORE_LOG(g_PmAcorePrintOnOff,"#send_id = %d#\n", send_id);
    if (DSAT_UR_DISABLE == curc_val) /*curc=0*/
    {
        /*受curc=0/1控制的命令*/
        if (HUAWEI_PM_RSP_GROUP_CURC01 
            == (HUAWEI_PM_RSP_GROUP_CURC01 
            & huawei_pm_acore_rsp_info_table[idx].group))
        {
            return HUAWEI_PM_RSP_ROUTE_DISCARD;           
        }
    }

    /*不受curc=0/1控制的命令，直接transfer，同时需要判断是否需要远程唤醒PIN信号*/
    huawei_pm_remote_check_And_handle_acore(wakeup_host, &usb_store);
    if( FALSE == huawei_pm_get_usb_suspended_status() )
    {
        return HUAWEI_PM_RSP_ROUTE_TRANSFER;
    }
    else if( FALSE == huawei_pm_get_pin_suspended_status() )
    {
        return HUAWEI_PM_RSP_ROUTE_TRANSFER;
    }
    else
    {
        if ( FALSE == wakeup_host )
        {
            if ( HUAWEI_PM_RSP_ID_SMS == send_id )
            {
                return HUAWEI_PM_RSP_ROUTE_STORE;
            }
            return HUAWEI_PM_RSP_ROUTE_DISCARD ;
        }
        else
        {
            if (( TRUE == usb_store ) && ( HUAWEI_PM_RSP_ID_TCPIP_DATA != send_id ))
            {
                return HUAWEI_PM_RSP_ROUTE_STORE;
            }

            return HUAWEI_PM_RSP_ROUTE_TRANSFER;
        }
    }
}

/******************************************************************************
  函数名称  : huawei_pm_process_acore_data()
  功能描述  : 判断是否(curc = 2)控制的主动上报命令
  输入参数  : 无
  被调函数  : 无
  输出参数  : 无
  返 回 值      : HUAWEI_PM_RSP_ROUTE_STORE,                
                            : HUAWEI_PM_RSP_ROUTE_DISCARD,            
                            : HUAWEI_PM_RSP_ROUTE_TRANSFER,           
                            : HUAWEI_PM_RSP_ROUTE_MAX,
********************************************************************************/
/*lint -e101*/
boolean huawei_pm_process_acore_data( NLRESPONSE *pRspBuf)
{
    huawei_pm_rsp_id_type send_id;
    huawei_pm_rsp_route_type route;
    /*The index in huawei_pm_acore_rsp_info_table*/
    uint32 index = 0;
    boolean result = FALSE;

    if( NULL == pRspBuf)
    {
        return TRUE;
    } 
    send_id = pRspBuf->m_id;

    route = huawei_pm_process_acore_data_ex(send_id);

    switch ( route )
    {
        case HUAWEI_PM_RSP_ROUTE_STORE:
        {
            index = huawei_pm_rsp_find_id_acore(send_id);

            if(AT_NL_PROTOCOL_FIX_UNSOLIDREPORT == pRspBuf->m_protocol)
            {
                huawei_pm_acore_data_store(index, (void *)pRspBuf->m_buffer, 
                    pRspBuf->m_length, pRspBuf->m_uindex);
            }
            else
            {
                huawei_pm_acore_data_store(index, (void *)pRspBuf->m_buffer, 
                    pRspBuf->m_length, -1);
            }

            result = FALSE;
            break;
        }
        case HUAWEI_PM_RSP_ROUTE_DISCARD:
        {
            result = FALSE;
            break;
        }
            
        case HUAWEI_PM_RSP_ROUTE_TRANSFER: 
        default:
        {
            result = TRUE;
            break;
        }
    }
    return result;
}
/*lint +e101*/
/******************************************************************************
Function:       huawei_pm_acore_data_cache_flush
Description:   
Input:           void
Output:         None
Return:          void
Others:          None
******************************************************************************/
static void huawei_pm_acore_data_cache_flush(void)
{
    uint32 i = 0;
    /*The current pos*/
    uint32 pos = huawei_pm_acore_data_cache.head;    
    /*Response params as follow*/
    void *msg_struct;
    int32 port_idx;
    int32 buf_length;
    
    
    for (i = 0; i < HUAWEI_PM_RSP_CACHE_LEN(huawei_pm_acore_data_cache); i++)
    {
        /*Get response params*/
        msg_struct
         = (void *)huawei_pm_acore_data_cache.buf_array[pos].pBuf;
        port_idx = huawei_pm_acore_data_cache.buf_array[pos].port_idx;
        buf_length = huawei_pm_acore_data_cache.buf_array[pos].pBuf_len;
        (void)huawei_pm_dispatch_uspace_data(msg_struct, buf_length, port_idx);
        kfree(msg_struct);
        msg_struct = NULL;
        huawei_pm_acore_data_cache.buf_array[pos].pBuf_len = 0;
        
        /*The next response*/
        pos = HUAWEI_PM_RSP_CACHE_NEXT(pos);        
    }

    /*Clear cache empty*/
    huawei_pm_acore_data_cache.head = huawei_pm_acore_data_cache.tail;

}

/******************************************************************************
Function:       huawei_pm_rsp_inform_flush_cache_acore
Description:   
Input:           None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void huawei_pm_rsp_inform_flush_cache_acore(void)
{
    huawei_pm_acore_data_cache_flush();
}


