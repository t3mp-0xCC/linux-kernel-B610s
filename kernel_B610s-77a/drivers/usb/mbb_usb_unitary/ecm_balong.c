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

#ifdef __cplusplus
extern "C" 
{ 
#endif /* __cplusplus */

#include <linux/types.h>
#include <linux/export.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include "usb_platform_comm.h"
#include "usb_debug.h"
#include "mdrv_udi.h"
#include "ecm_balong.h"
#include <linux/usb/bsp_usb.h>
#include "usb_vendor.h"
#include "f_ecm_mbb.h"

#define FNAME __FUNCTION__

static bool ecm_vendor_inited = false;
static USB_INT32 ecm_verndor_init_counts = 0;
bool ecm_bypass_udi = false;    /* just for test */
static ecm_app_ctx_t ecm_app_ctx_set[GUSB_MAX_CONFIG][GNET_MAX_NUM] = {{0}};
static ECM_PKT_INFO_S ecm_pkt_info_cur[GNET_MAX_NUM] = {{0}};  /* ugly requirement */
static ECM_PKT_INFO_S ecm_pkt_info_last[GNET_MAX_NUM] = {{0}}; /* ugly requirement */

#define ECM_AT_CMD_BUFFER_LENGTH      2048
static u8 gecmAtCmdBuf[ECM_AT_CMD_BUFFER_LENGTH] = {0};
static u8 gecmAtRspBuf[ECM_AT_CMD_BUFFER_LENGTH] = {0};

extern UDI_HANDLE BSP_UDI_NCM_DataChnToHandle(u32 u32Chn);
#define  BSP_UDI_ECM_DataChnToHandle(u32Chn) BSP_UDI_NCM_DataChnToHandle(u32Chn);

extern int ecm_vendor_post_skb(void *ncm_priv, struct sk_buff *skb);

/*****************************************************************************
* 函 数 名  : ecm_get_ctx
*
* 功能描述  : 根据设备ID获取上下文
*
* 输入参数  : u32 dev_id    :设备ID
* 输出参数  : 无
*
* 返 回 值  : 设备ID上下文
*
* 其它说明  : 无
*
*****************************************************************************/
static inline ecm_app_ctx_t *ecm_get_ctx(u32 dev_id)
{
    //DBG_I(MBB_ECM, "[%d]usb_get_curr_cfg_idx= %d\n",__LINE__,usb_get_curr_cfg_idx());
    u32 config = ((usb_get_curr_cfg_idx() > 0) ? usb_get_curr_cfg_idx() - 1 : 0);
    return (&ecm_app_ctx_set[config][dev_id]);
}

/*****************************************************************************
* 函 数 名  : ecm_get_id
*
* 功能描述  : 根据上下文结构获取设备ID
*
* 输入参数  : BSP_U32 u32ecmDevId  
* 输出参数  : 
*
* 返 回 值  : 
*
* 其它说明  : 
*
*****************************************************************************/
static inline s32 ecm_get_id(ecm_app_ctx_t *app_ctx)
{
    u32 i;/*lint !e578*/

    for (i = 0; i < GNET_USED_NUM; i++)
    {
        if (app_ctx == ecm_get_ctx(i))
        {
            return i;
        }
    }

    return -ENODEV;
}


/*****************************************************************************
* 函 数 名  : ecm_get_id
*
* 功能描述  : 获取一个可用的ecm app ctx
*
* 输入参数  : 无
* 输出参数  : 无
*
* 返 回 值  : >=0 ecm app ctx下标
*             -ENODEV
*
* 其它说明  : 无
*
*****************************************************************************/
static inline int ecm_get_idle(void)
{
    u32 i;/*lint !e578*/
    ecm_app_ctx_t *ecm_app_ctx = NULL;

    for (i = 0; i < GNET_USED_NUM; i++)
    {
        ecm_app_ctx = ecm_get_ctx(i);
        if (false == ecm_app_ctx->bUsed)
        {
            ecm_app_ctx->bUsed = true;
            ecm_app_ctx->ecm_id = i;
            return (int)i;
        }
    }

    return -ENODEV;
}

/*****************************************************************************
* 函 数 名  : ecm_set_idle
*
* 功能描述  : recycle a ecm app ctxt
*
* 输入参数  : ecm_id :ecm id
* 输出参数  : 无
*
* 返 回 值  : 0  :success
*             <0 :error
*
* 修改记录 : 
*
*****************************************************************************/
static inline int ecm_set_idle(u32 dev_id)
{
    ecm_app_ctx_t *ecm_app_ctx = NULL;

    if(dev_id >= GNET_USED_NUM)
    {
        return -EINVAL;
    }
    
    ecm_app_ctx = ecm_get_ctx(dev_id);

    ecm_app_ctx->bUsed = false;
    ecm_app_ctx->ecm_id = 0;
  
    return 0;
}

/*****************************************************************************
* 函 数 名  : get_ecm_mnet_flag
*
* 功能描述  : 获取是否初始化多个ecm 网卡
*
* 输入参数  : NA
* 输出参数  : NA
*
* 返 回 值  : 0  :false 1:true
*
* 修改记录 : 
*
*****************************************************************************/
static inline USB_UINT32 get_ecm_mnet_flag(void)
{
    return (USB_NUM_1 < ecm_verndor_init_counts)? true : false;
}

/*****************************************************************************
* 函 数 名  : ecm_at_resp_process
*
* 功能描述  : AT命令响应接口，同步接口
*
* 输入参数  : ecm_app_ctx_t *pecmCtx
*             void *response  
*             u32 size     
* 输出参数  : 
*
* 返 回 值  : 
*
* 其它说明  : 
*
*****************************************************************************/
s32 ecm_vendor_at_resp_process(ecm_app_ctx_t *ctx, void *response, u32 size)
{
    s32 ret = 0;
    
    ecm_vendor_ctx_t *vendor_ctx = ctx->vendor_ctx;

    if (ECM_DBG_MASK_CHECK(ECM_DBG_MASK_AT))
    {
        memset(gecmAtRspBuf, 0, ECM_AT_CMD_BUFFER_LENGTH);
        memcpy(gecmAtRspBuf, response, size);
        DBG_E(MBB_ECM, "L[%d]: size=%u, data=\"%s\"\n", __LINE__, size, gecmAtRspBuf);
    }
/*删除实际发送动作仅给协议返回处理完成    */

    if (ctx->cbs.usb_ndis_resp_status_func)
    {
        ctx->cbs.usb_ndis_resp_status_func(response, 0);
    }
    
    return 0;
}

/*****************************************************************************
* 函 数 名  : ecm_vendor_open
*
* 功能描述  : 打开ecm通道
*
* 输入参数  : ECM_DEV_TYPE_E dev_type  通道类型
*             
* 输出参数  : u32 *dev_id              设备id
*
* 返 回 值  : -EINVAL
*             -ENODEV
*             OK
*
* 修改记录  : 2014年12月10日   mahailong  creat
*****************************************************************************/
s32 ecm_vendor_open(ECM_DEV_TYPE_E dev_type, u32 dev_id)
{
    ecm_app_ctx_t *ecm_app_ctx = NULL;

    DBG_T(MBB_ECM, "%s: type=%u, id=%u\n", __FUNCTION__, dev_type, dev_id);

    if(!ecm_vendor_inited)
    {
        DBG_E(MBB_ECM, "%s line %d:module not init.\n",
            __FUNCTION__,__LINE__);
        return -ENODEV;
    }

    if((dev_id >= GNET_USED_NUM) 
        || ((ECM_DEV_DATA_TYPE != dev_type) && (ECM_DEV_CTRL_TYPE != dev_type)))
    {
        DBG_E(MBB_ECM, "L[%d]:type=%u, id=%u\n", __LINE__, dev_type, dev_id);
        return -EINVAL;
    }

    ecm_app_ctx = ecm_get_ctx(dev_id);
        
    if (false == ecm_app_ctx->chnl_open[dev_type]) 
    {
        ecm_app_ctx->chnl_open[dev_type] = true;
        return 0;
    }
  
    return -ENODEV;
}

/*****************************************************************************
* 函 数 名  : ecm_vendor_close
*
* 功能描述  : 关闭ecm设备通道
*
* 输入参数  : ECM_DEV_TYPE_E enDevType  设备类型
*             u32 u32ecmDevId  ECM设备ID
* 输出参数  : NA
*
* 返 回 值  : -EINVAL   : 参数非法
*             OK        : 成功
*
* 其它      : NA
*****************************************************************************/
s32 ecm_vendor_close(ECM_DEV_TYPE_E dev_type, u32 dev_id)
{
    ecm_app_ctx_t *ecm_app_ctx = NULL;

    DBG_T(MBB_ECM, "%s: type=%u, id=%u\n", __FUNCTION__, dev_type, dev_id);
    
    if(!ecm_vendor_inited)
    {
        return -ENODEV;
    }
    if (dev_id >= GNET_USED_NUM)
    {
        DBG_E(MBB_ECM, "%s: dev_id = %d, dev_type = %d\n", 
            FNAME, (s32)dev_id, (s32)dev_type);
        return -ENODEV;
    }
    
    if (dev_type != ECM_DEV_DATA_TYPE && dev_type != ECM_DEV_CTRL_TYPE)
    {
        DBG_E(MBB_ECM, "%s: dev_id = %d, dev_type = %d\n", 
            FNAME, (s32)dev_id, (s32)dev_type);
        return -EINVAL;
    }

    ecm_app_ctx = ecm_get_ctx(dev_id);

    ecm_app_ctx->chnl_open[dev_type] = false;

    return 0;
}

/*****************************************************************************
* 函 数 名  : ecm_write
*
* 功能描述  : 发送数据
*
* 输入参数  : u32 dev_id        : 设备ID
* 输出参数  : void *pkt_encap   : 包封装首地址
*
* 返 回 值  : 0
*             -EINVAL
*             -ENODEV
*
* 其它      : NA
*****************************************************************************/
s32 ecm_vendor_write(u32 dev_id, void *pkt_encap, void *net_priv)
{
    ecm_app_ctx_t *app_ctx = NULL;
    ecm_vendor_ctx_t *vendor_ctx = NULL;

    DBG_I(MBB_ECM, "line %d:enter dev_id(%u),pkt_encap(%p).\n",__LINE__,dev_id,pkt_encap);
//    WARN_ON(1);
    if(!ecm_vendor_inited)
    {
        DBG_E(MBB_ECM, "%s line %d:ecm vendor not init.\n",__FUNCTION__,__LINE__);
        return -ENODEV;
    }
    
    if(dev_id >= GNET_USED_NUM || !pkt_encap)
    {
        DBG_E(MBB_ECM, "%s line %d:invalid para(dev_id=%d,pkt_encap=%d).\n",
            __FUNCTION__,__LINE__,dev_id,(u32)pkt_encap);
        return -EINVAL;
    }

    app_ctx = ecm_get_ctx(dev_id);

    
    vendor_ctx = app_ctx->vendor_ctx;

    if(!app_ctx->chnl_open[ECM_DEV_DATA_TYPE] || !app_ctx->bDataConnect)
    {
            vendor_ctx->stat_tx_nochl++;

        return -ENODEV;
    }

    
    return ecm_vendor_post_skb(app_ctx->vendor_ctx->ecm_priv,(struct sk_buff *)pkt_encap);
}

/*****************************************************************************
* 函 数 名  : ecm_ioctl
*
* 功能描述  : 配置ecm设备属性
*
* 输入参数  : dev_id
*             cmd
*             param
* 输出参数  : NA
*
* 返 回 值  : -ENODEV
*
* 其它      : NA
*****************************************************************************/
s32 ecm_vendor_ioctl(u32 dev_id, ECM_IOCTL_CMD_TYPE_E cmd, void *param)
{    
    ecm_app_ctx_t *ctx = NULL;
    s32 ret = 0; 
    DBG_E(MBB_ECM, "line %d:enter.dev_id(%u),cmd(%x)\n", __LINE__,dev_id,cmd);
    if(!ecm_vendor_inited)
    {
        DBG_E(MBB_ECM, "%s line %d:module not init.\n", __FUNCTION__,__LINE__);
        return -ENODEV;
    }

    if (dev_id >= GNET_USED_NUM)
    {
        DBG_E(MBB_ECM, "%s line %d:dev_id %d overflow.\n",
            __FUNCTION__,__LINE__,dev_id);
        return -EINVAL;
    }

    ctx = ecm_get_ctx(dev_id);
    if(!ctx->chnl_open[ECM_DEV_DATA_TYPE] && !ctx->chnl_open[ECM_DEV_CTRL_TYPE])
    {
        DBG_E(MBB_ECM, "%s line %d:both channel not opened.\n",
            __FUNCTION__,__LINE__);
        return -ENODEV;
    }

    /* at_process/at_response divert to ctrl channel, other's divert to data channel. */
    if (!ctx->chnl_open[ECM_DEV_DATA_TYPE]
        && (cmd != ECM_IOCTL_REG_AT_PROCESS_FUNC)
        && (cmd != ECM_IOCTL_REG_NDIS_RESP_STATUS_FUNC)
        && (cmd != ECM_IOCTL_AT_RESPONSE))
    {
        DBG_E(MBB_ECM, "ctx->chnl_open[%d]\n",ctx->chnl_open[ECM_DEV_DATA_TYPE]);
        return -EIO;
    }
    
    if (!ctx->chnl_open[ECM_DEV_CTRL_TYPE]
        && (ECM_IOCTL_REG_AT_PROCESS_FUNC == cmd
            || ECM_IOCTL_AT_RESPONSE == cmd))
    {
        DBG_E(MBB_ECM, "ctx->chnl_open[%d]\n",ctx->chnl_open[ECM_DEV_CTRL_TYPE]);
        return -EIO;
    }
    
    switch (cmd)
    {
        case ECM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF:   /* notify speed to pc */
        {
            ECM_IOCTL_CONNECTION_SPEED_S *speed = (ECM_IOCTL_CONNECTION_SPEED_S *)param;

            if (true != ctx->bEnable || !param)
            {
                return -EIO;
            }
            
            /* PC端网卡显示速率为上下行最大值，如果上下行速率均小于1k，
               或者大于480M，则PC端速率会显示不准确*/
            if (((speed->u32UpBitRate < ECM_MIN_RATE) 
                && (speed->u32DownBitRate < ECM_MIN_RATE))
                || (speed->u32UpBitRate > ECM_MAX_RATE)
                || (speed->u32DownBitRate > ECM_MAX_RATE))
            {
                return -EIO;
            }

            //处理多实例网卡端口
            if (get_ecm_mnet_flag())
            {
                USB_UINT32 u32NdisCID = ((ECM_IOCTL_CONNECTION_SPEED_S *)param)->u32NdisCID;

                if ((0 == u32NdisCID) || (u32NdisCID > ecm_verndor_init_counts))
                {
                    DBG_E(MBB_ECM, "L[%d]:u32NdisCID=%d is error\n",__LINE__, u32NdisCID);
                    return -EIO;
                }
                ctx = ecm_get_ctx(u32NdisCID - USB_NUM_1);
            }
            
            /* notify the host the connection speed */
            ret = ecm_vendor_notify(ctx->vendor_ctx->ecm_priv, cmd, param);
            break;
        }

        case ECM_IOCTL_NETWORK_CONNECTION_NOTIF:
        {
            if (true != ctx->bEnable || !param)
            {
                return -EIO;
            }

            //处理多实例网卡端口
            USB_UINT32  u32NdisCID = ((ECM_IOCTL_CONNECTION_STATUS_S *)param)->u32NdisCID;
            if (get_ecm_mnet_flag())
            {
                if ((0 == u32NdisCID) || (u32NdisCID > ecm_verndor_init_counts))
                {
                    DBG_E(MBB_ECM, "L[%d]:u32NdisCID=%d is error\n",__LINE__, u32NdisCID);
                    return -EIO;
                }
                ctx = ecm_get_ctx(u32NdisCID - USB_NUM_1);
            }

            if (((*(BSP_U32 *)param) != ECM_IOCTL_CONNECTION_LINKDOWN) && ((*(BSP_U32 *)param) != ECM_IOCTL_CONNECTION_LINKUP))
            {
                return -EIO;
            }
            ctx->bDataConnect = *(BSP_U32 *)param;

            if (ctx->bDataConnect)
            {
                msleep(50);  /* 延时50ms，等待前一个中断消息(连接速率)上报完成 */
            }
                        
            /* notify the host the connection state */
            ret = ecm_vendor_notify(ctx->vendor_ctx->ecm_priv, cmd, param);

            break;
        }

        case ECM_IOCTL_FREE_BUFF:
        {
            if (!param)
            {
                return -EIO;
            }
            break;
        }
                
        case ECM_IOCTL_SET_ACCUMULATION_TIME:
        {
            if (true != ctx->bEnable)
            {
                return -EIO;
            }

            ctx->u32AccumulationTime = *(u32 *)param;
            break;
        }
                
        case ECM_IOCTL_SET_PKT_ENCAP_INFO:
        {
            ctx->pkt_encap_info[0] = (u32)(((ECM_PKT_ENCAP_INFO_S *)param)->s32BufOft);
            ctx->pkt_encap_info[1] = (u32)(((ECM_PKT_ENCAP_INFO_S *)param)->s32LenOft);
            ctx->pkt_encap_info[2] = (u32)(((ECM_PKT_ENCAP_INFO_S *)param)->s32NextOft);
            break;
        }

        case ECM_IOCTL_REG_UPLINK_RX_FUNC:
        {
            ctx->cbs.rx_cb = (USBUpLinkRxFunc)param;/*lint !e611*/
            if (get_ecm_mnet_flag())
            {
                ecm_get_ctx(USB_NUM_1)->cbs.rx_cb = (USBUpLinkRxFunc)param;
            }
            
            printk(KERN_ERR"ctx->cbs.rx_cb=(%p)",ctx->cbs.rx_cb);
            break;
        }
        
        case ECM_IOCTL_REG_FREE_PKT_FUNC:
        {
            ctx->cbs.pkt_encap_free_cb = (USBFreePktEncap)param;/*lint !e611*/
            break;
        }
        
        case ECM_IOCTL_GET_USED_MAX_BUFF_NUM:
        {
            *(u32*)param = (u32)(ECM_RCV_BUFFER_NUM - 1);
            break;
        }
        
        case ECM_IOCTL_GET_DEFAULT_TX_MIN_NUM:
        {
            *(u32*)param = MIN_ECM_TX;
            break;
        }
        
        case ECM_IOCTL_GET_DEFAULT_TX_TIMEOUT:
        {            
            *(u32*)param = ECM_TX_TIMEOUT;
            break;
        }
        
        case ECM_IOCTL_GET_DEFAULT_TX_MAX_SIZE:
        {            
            *(u32*)param = ECM_MAX_TX_TRANSFER_SIZE;
            break;
        }
        
        case ECM_IOCTL_SET_TX_MIN_NUM:
        {            
            break;
        }
        
        case ECM_IOCTL_SET_TX_TIMEOUT:
        {
            if (true != ctx->bEnable)
            {
                return -EIO;
            }

            break;
        }
        
        case ECM_IOCTL_SET_TX_MAX_SIZE:
        {
            break;
        }
        
        case ECM_IOCTL_GET_RX_BUF_SIZE:
        {
            *(u32 *)param = ECM_MAX_RX_TRANSFER_SIZE;
            break;
        }
        
        case ECM_IOCTL_FLOW_CTRL_NOTIF:
        {    
            if (true != ctx->bEnable)
            {
                return -EIO;
            }
            
            if (*(u32 *)param > (u32)ECM_IOCTL_FLOW_CTRL_DISABLE)
            {

                return -EINVAL;
            }
            
            ctx->u32FlowCtrlStus = *(u32 *)param;
            ret = ecm_vendor_notify(ctx->vendor_ctx->ecm_priv, cmd, param);
            break;
        }    
        
        case ECM_IOCTL_REG_AT_PROCESS_FUNC:
        {
            ctx->cbs.ndis_at_recv_cb = (USBNdisAtRecvFunc)param;/*lint !e611*/
            break;
        }
        
        case ECM_IOCTL_AT_RESPONSE:
        {
            if((NULL == ((ECM_AT_RSP_S *)param)->pu8AtAnswer)
                || (0 == ((ECM_AT_RSP_S *)param)->u32Length))
            {
                return -EINVAL;
            }

            ret = ecm_vendor_at_resp_process(ctx,
              ((ECM_AT_RSP_S *)param)->pu8AtAnswer,
              ((ECM_AT_RSP_S *)param)->u32Length);
            if(ret)
            {
                return -EIO;
            }
            break;
        }

        case ECM_IOCTL_REG_NDIS_RESP_STATUS_FUNC :
        {
            ctx->cbs.usb_ndis_resp_status_func = (USBNdisRespStatusCB)param;
            break;
        }
        
        case ECM_IOCTL_REG_CONNECT_STUS_CHG_FUNC:
        {
            ctx->cbs.ndis_stat_change_cb = (USBNdisStusChgFunc)param;/*lint !e611*/
            break;
        }

        case ECM_IOCTL_SET_PKT_STATICS:
        { 
            memcpy(&(ctx->pkt_stats), param, sizeof(ECM_IOCTL_PKT_STATISTICS_S));
            break;
        }
        
        case ECM_IOCTL_GET_FLOWCTRL_STATUS:
        {
            *(u32 *)param = ctx->u32FlowCtrlStus;
            break;
        }

        case ECM_IOCTL_GET_CUR_TX_MIN_NUM:
        {
            *(u32 *)param = ctx->u32TxCurMinNum;
            break;
        }
        
        case ECM_IOCTL_GET_CUR_TX_TIMEOUT:
        {
            *(u32 *)param = ctx->u32TxCurTimeout;
            break;
        }
        
        case ECM_IOCTL_IPV6_DNS_NOTIF:
        {
            if (true != ctx->bEnable)
            {
                return -EIO;
            }

            if ((NULL == ((ECM_AT_IPV6_DNS_NTF_S *)param)->pu8Ipv6DnsNtf)
                || (0 == ((ECM_AT_IPV6_DNS_NTF_S *)param)->u32Length))
            {
                DBG_E(MBB_ECM, "parm error pu8Ipv6DnsNtf = 0x%x,length:%d\n",
                    (u32)(((ECM_AT_IPV6_DNS_NTF_S *)param)->pu8Ipv6DnsNtf),
                      ((ECM_AT_IPV6_DNS_NTF_S *)param)->u32Length);
                return -EINVAL;
            }
            else if (BSP_ECM_IPV6_DNS_LEN >= ((ECM_AT_IPV6_DNS_NTF_S *)param)->u32Length)
            {
                ECM_AT_IPV6_DNS_NTF_S *ipv6_info = (ECM_AT_IPV6_DNS_NTF_S *)param;
                memcpy(ctx->au8Ipv6Dns, ipv6_info->pu8Ipv6DnsNtf, ipv6_info->u32Length);
                ctx->u32Ipv6DnsLen = ipv6_info->u32Length;
            }
            else
            {
                return -EINVAL;
            }
        
            break;
        }
         
        case ECM_IOCTL_SET_IPV6_DNS:
        {
            u8 * pu8Ipv6DnsInfo = ((ECM_IPV6DNS_S *)param)->pu8Ipv6DnsInfo;
            u32 u32Len = ((ECM_IPV6DNS_S *)param)->u32Length;
            if (true != ctx->bEnable)
            {
                DBG_E(MBB_ECM, "%s: the ECM device is disable\n",FNAME);
                return -EIO;        
            }
            
            if((NULL  == pu8Ipv6DnsInfo)
                || (BSP_ECM_IPV6_DNS_LEN != u32Len))
            {
                DBG_E(MBB_ECM, "parm error pu8Ipv6DnsInfo = 0x%x,at.length:%d\n", (u32)pu8Ipv6DnsInfo, u32Len);
                return -EINVAL;
            }
            
            memcpy(ctx->au8Ipv6Dns, pu8Ipv6DnsInfo, BSP_ECM_IPV6_DNS_LEN);
            ctx->u32Ipv6DnsLen = u32Len;            
            ret = ecm_vendor_notify(ctx->vendor_ctx->ecm_priv, cmd, NULL);
            break;
        }
        
        case ECM_IOCTL_CLEAR_IPV6_DNS:
        {
            (void)memset(ctx->au8Ipv6Dns, 0x0, BSP_ECM_IPV6_DNS_LEN);
            ctx->u32Ipv6DnsLen = 0;
            break;
        }
        
        case ECM_IOCTL_GET_ECM_STATUS:
        {
            *(u32 *)param = (u32)ctx->bEnable;
            break;
        }
        
        case ECM_IOCTL_SET_RX_MIN_NUM:
        {
            break;
        }
        
        case ECM_IOCTL_SET_RX_TIMEOUT:
        {
            break;
        }

        case ECM_IOCTL_GET_MBIM_FLAG :
        {
            *(u32 *)param = 0;
            break;
        }

        case ECM_IOCTL_GET_MNET_FLAG:
        {
            *(u32 *)param = get_ecm_mnet_flag();
            break;
        }
        
        default:
        {
            DBG_E(MBB_ECM, "%s: cmd [%d]param[0x%x] is invalid!\n", FNAME, (u32)cmd,(u32)param);
            return -EINVAL;
        }
              
    }

    return ret;
}

/*****************************************************************************
* 函 数 名  : ecm_vendor_init
*
* 功能描述  : ecm app初始化
*
* 输入参数  :       
*             void *app_ctx     
* 输出参数  : 
*
* 返 回 值  : 
*
* 其它说明  : 
*
*****************************************************************************/
s32 ecm_vendor_init(ecm_vendor_ctx_t *vendor_ctx)
{
    ecm_app_ctx_t *app_ctx = NULL;
    s32 dev_id = 0;

    DBG_T(MBB_ECM, "%s enter\n", __FUNCTION__);
    
    if(!vendor_ctx)
    {
        return -ENODEV;
    }

    if (false == ecm_vendor_inited)
    {
        (void)memset((void *)ecm_app_ctx_set, 0x0, sizeof(ecm_app_ctx_set));
        ecm_vendor_inited = true;
        ecm_verndor_init_counts = 0;
    }
    ecm_verndor_init_counts++;

    dev_id = ecm_get_idle();
    if (dev_id < 0)
    {
        DBG_E(MBB_ECM, "%s wrong dev id\n", __FUNCTION__);
        return -ENODEV;
    }

    DBG_T(MBB_ECM, "%s get idel dev id %d\n", __FUNCTION__, dev_id);
    
    app_ctx = ecm_get_ctx(dev_id);
    app_ctx->bEnable = true;
    app_ctx->u32AccumulationTime = ECM_PACKETS_ACCUMULATION_TIME;
    app_ctx->vendor_ctx = vendor_ctx;
    app_ctx->vendor_ctx->vendor_priv = app_ctx;
    if(!app_ctx->vendor_ctx)
    {
        DBG_E(MBB_ECM, "%s line %d:null app_ctx->vendor_ctx\n",__FUNCTION__,__LINE__);
    }

    return 0;
}

/*****************************************************************************
* 函 数 名  : ecm_vendor_uninit
*
* 功能描述  : ecm app去初始化
*
* 输入参数  :       
*             void *app_ctx     
* 输出参数  : 
*
* 返 回 值  : 
*
* 其它说明  : 
*
*****************************************************************************/
s32 ecm_vendor_uninit(ecm_vendor_ctx_t *vendor_ctx)
{
    ecm_app_ctx_t *app_ctx = (ecm_app_ctx_t *)vendor_ctx->vendor_priv;

    DBG_T(MBB_ECM, "%s enter\n", __FUNCTION__);
    
    if(!app_ctx || app_ctx->ecm_id >= GNET_USED_NUM)
    {
        return -EINVAL;
    }

    ecm_set_idle(app_ctx->ecm_id);
    memset(app_ctx, 0, sizeof(*app_ctx));

    ecm_vendor_inited = false;

    return 0;
}

int ecm_vendor_submit_skb(ecm_app_ctx_t *app_ctx, struct sk_buff *skb)
{
    UDI_HANDLE handle;
    s32 dev_id = 0;
    u32 pkt_len = 0;

    DBG_I(MBB_ECM, "enter,appctx(%p)skb(%p)\n", app_ctx, skb);
  

    if (!app_ctx || !app_ctx->bDataConnect || !app_ctx->cbs.rx_cb || !skb)
    {
        return -ENOMEM;
    }
    
    dev_id = ecm_get_id(app_ctx);
    if(0 > dev_id)
    {
        return -ENODEV;
    }

    DBG_I(MBB_ECM,"dev_id(%d)\n",dev_id);

    ECM_DBG_PRINT_RX("%s : dev_id=%d, open=%u\n", __FUNCTION__, dev_id, 
        (dev_id < GNET_MAX_NUM) ? ecm_get_ctx(dev_id)->chnl_open[ECM_DEV_DATA_TYPE]:0);

    if(!ecm_bypass_udi)
    {
        handle = BSP_UDI_ECM_DataChnToHandle((u32)dev_id);
    }
    else
    {
        handle = (u32)dev_id;
    }
        

    pkt_len = skb->len;

    if (pkt_len < ETH_HLEN || pkt_len > ETH_FRAME_LEN)
    {
        app_ctx->debug_info.u32RcvTooLongNum++;
        app_ctx->debug_info.u32RcvErrPktLen = pkt_len;
        return -EINVAL;
    }

    //printk(KERN_ERR"app_ctx->cbs.rx_cb=(%p)\n",app_ctx->cbs.rx_cb);
                
    app_ctx->debug_info.u32PsRcvPktNum++;
    app_ctx->cbs.rx_cb(handle, skb);

    return 0;
}

s32 ECM_debugPktGet(u32 dev_id, ECM_PKT_INFO_S *pkt_info)
{
    ecm_app_ctx_t *ecm_app_ctx = NULL;
    if(dev_id >= GNET_USED_NUM)
    {
        return -EINVAL;
    }
    
    (void)memset((void *)&ecm_pkt_info_cur[dev_id], 0, sizeof(ECM_PKT_INFO_S));
    ecm_app_ctx = ecm_get_ctx(dev_id);
    ecm_pkt_info_cur[dev_id].u32EcmSendPktNum = ecm_app_ctx->debug_info.u32ecmSendPktNum;
    ecm_pkt_info_cur[dev_id].u32PsRcvPktNum = ecm_app_ctx->debug_info.u32PsRcvPktNum;
    ecm_pkt_info_cur[dev_id].u32RcvUnusedNum = ecm_app_ctx->debug_info.u32RcvUnusedNum;

    ecm_pkt_info_cur[dev_id].u32EcmSendPktNum -= ecm_pkt_info_last[dev_id].u32EcmSendPktNum;
    ecm_pkt_info_cur[dev_id].u32PsRcvPktNum -= ecm_pkt_info_last[dev_id].u32PsRcvPktNum;
    ecm_pkt_info_cur[dev_id].u32RcvUnusedNum -= ecm_pkt_info_last[dev_id].u32RcvUnusedNum;

    *pkt_info = ecm_pkt_info_cur[dev_id];
    
    return 0;
}
EXPORT_SYMBOL(ECM_debugPktGet);

s32 ECM_debugPktClear(u32 dev_id)
{
    if(dev_id >= GNET_USED_NUM)
    {
        return -EINVAL;
    }
    
    ecm_pkt_info_last[dev_id].u32EcmSendPktNum += ecm_pkt_info_cur[dev_id].u32EcmSendPktNum;
    ecm_pkt_info_last[dev_id].u32PsRcvPktNum += ecm_pkt_info_cur[dev_id].u32PsRcvPktNum;
    ecm_pkt_info_last[dev_id].u32RcvUnusedNum += ecm_pkt_info_cur[dev_id].u32RcvUnusedNum;
    
    (void)memset((void *)&ecm_pkt_info_cur[dev_id], 0, sizeof(ECM_PKT_INFO_S));

    return 0;
}
EXPORT_SYMBOL(ECM_debugPktClear);

void ecm_vendor_help(void)
{    
    (void) printk("\r |*************************************|\n");    
    (void) printk("\r ecm_debuginfo_show : 查看各种包统计信息\n");    
    (void) printk("\r ecm_debugcb_show   : 查看回调函数信息\n");
    (void) printk("\r ecm_debugctx_Show  : 查看vendor上下文信息\n");
    (void) printk("\r ecm_vendorctx_show : 查看上下文信息\n");    
    (void) printk("\r |*************************************|\n");    
}

void ecm_debuginfo_show(void)
{
    u32 i;/*lint !e578*/
    ecm_app_ctx_t *ecm_app_ctx = NULL;
    
    for (i = 0; i < GNET_MAX_NUM;i++)
    {
        ecm_app_ctx = ecm_get_ctx(i);        
        (void)  printk("\r The ECM[%d]adpter info                :\n",(s32)i);
        (void)  printk("\r The ECM[%d]:收包函数进入次数          : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32RcvInNum);
        (void)  printk("\r The ECM[%d]:收包字节数                : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32RcvInSize);
        (void)  printk("\r The ECM[%d]:收包错误包计数            : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32RcvTooLongNum);
        (void)  printk("\r The ECM[%d]:收包错误包包长            : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32RcvErrPktLen);
        (void)  printk("\r The ECM[%d]:收包分配skb失败次数       : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32AllocSkbFailNum);
        (void)  printk("\r The ECM[%d]:收包函数进入异常退出次数  : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32RcvInErrNum);
        (void)  printk("\r The ECM[%d]:收包送到PS的包个数        : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32PsRcvPktNum);
        (void)  printk("\r The ECM[%d]:收包回调函数空而丢弃包个数: %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32RcvUnusedNum);
        (void)  printk("\r The ECM[%d]:发包函数进入次数          : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmSendInNum);    
        (void)  printk("\r The ECM[%d]:发包个数                  : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmSendPktNum);      
        (void)  printk("\r The ECM[%d]:发包全部无效次数          : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmSendNoPktNum);
        (void)  printk("\r The ECM[%d]:发包函数成功次数          : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmSendOkNum);
        (void)  printk("\r The ECM[%d]:发包申请不到内存退出次数  : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmSendNoMemNum);
        (void)  printk("\r The ECM[%d]:发包调用fd层write失败次数 : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmSendFailNum);
        (void)  printk("\r The ECM[%d]:发包通道没有打开退出次数  : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmSendDevNotOpenNum);
        (void)  printk("\r The ECM[%d]:发包设备没有使能退出次数  : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmSendDevNotEnableNum);
        (void)  printk("\r The ECM[%d]:发包网卡没有连接退出次数  : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmSendNetNotConnectNum);
        (void)  printk("\r The ECM[%d]:发包包长超过1514退出次数  : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmSendPktTooLargeNum);
        (void)  printk("\r The ECM[%d]:发包函数数据包无效次数    : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmSendPktInvalidNum);
        (void)  printk("\r The ECM[%d]:发包完成回调次数          : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmSendDoneNum);
        (void)  printk("\r The ECM[%d]:发包完成回调失败次数      : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmSendDoneFailNum);
        (void)  printk("\r The ECM[%d]:AT reply进入次数          : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmAtInNum);
        (void)  printk("\r The ECM[%d]:AT reply成功次数          : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmAtInOKNum);
        (void)  printk("\r The ECM[%d]:AT reply失败次数          : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmAtInFailNum);
        (void)  printk("\r The ECM[%d]:AT reply超时次数          : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmAtInTimeOutNum);
        (void)  printk("\r The ECM[%d]:AT reply完成次数          : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmAtDoneNum);
        (void)  printk("\r The ECM[%d]:AT request回调处理次数    : %d\n",(s32)i,(s32)ecm_app_ctx->debug_info.u32ecmAtCbNum);
    }    
}

void ecm_debugcb_show(void)
{
    u32 i;/*lint !e578*/
    ecm_app_ctx_t *ecm_app_ctx = NULL;
    
    for (i = 0; i < GNET_MAX_NUM; i++)
    {
        ecm_app_ctx = ecm_get_ctx(i);                 
        (void)  printk("\r The ECM[%d] callback func info            :\n",(s32)i);
        (void)  printk("\r The ECM[%d]:上行收包回调函数              :0x%x\n",(s32)i,(s32)ecm_app_ctx->cbs.rx_cb);
        (void)  printk("\r The ECM[%d]:下行buffer释放回调函数        :0x%x\n",(s32)i,(s32)ecm_app_ctx->cbs.pkt_encap_free_cb);
        (void)  printk("\r The ECM[%d]:AT命令处理回调                :0x%x\n",(s32)i,(s32)ecm_app_ctx->cbs.ndis_at_recv_cb);
        (void)  printk("\r The ECM[%d]:USB网卡状态变化处理回调函数   :0x%x\n",(s32)i,(s32)ecm_app_ctx->cbs.ndis_stat_change_cb);
    }
}

void ecm_debugctx_Show(void)
{
    u32 i;/*lint !e578*/
    ecm_app_ctx_t *ecm_app_ctx = NULL;
    
    for (i = 0; i < GNET_MAX_NUM; i++)
    {
        ecm_app_ctx = ecm_get_ctx(i);       
        (void)  printk("\r The ECM[%d] ctx info                      :\n",(s32)i);
        (void)  printk("\r The ECM[%d]:设备ID号                      :0x%x\n",(s32)i,(s32)ecm_app_ctx->ecm_id);
        (void)  printk("\r The ECM[%d]:设备是否被初始化              :0x%x\n",(s32)i,(s32)ecm_app_ctx->bUsed);
        (void)  printk("\r The ECM[%d]:设备数据通道是否被打开        :0x%x\n",(s32)i,(s32)ecm_app_ctx->chnl_open[ECM_DEV_DATA_TYPE]);
        (void)  printk("\r The ECM[%d]:设备控制通道是否被打开        :0x%x\n",(s32)i,(s32)ecm_app_ctx->chnl_open[ECM_DEV_CTRL_TYPE]);      
        (void)  printk("\r The ECM[%d]:设备是否使能                  :0x%x\n",(s32)i,(s32)ecm_app_ctx->bEnable);
        (void)  printk("\r The ECM[%d]:网卡是否连接                  :0x%x\n",(s32)i,(s32)ecm_app_ctx->bDataConnect); 
        (void)  printk("\r The ECM[%d]:AT发送返回值                  :0x%x\n",(s32)i,(s32)ecm_app_ctx->s32atResponseRet);
        (void)  printk("\r The ECM[%d]:回调函数结构                  :0x%x\n",(s32)i,(s32)&ecm_app_ctx->cbs);
        (void)  printk("\r The ECM[%d]:PS配置的包封装结构            :0x%x\n",(s32)i,(s32)ecm_app_ctx->pkt_encap_info);      
        (void)  printk("\r The ECM[%d]:流控状态                      :0x%x\n",(s32)i,(s32)ecm_app_ctx->u32FlowCtrlStus);        
        (void)  printk("\r The ECM[%d]:当前发包阈值                  :0x%x\n",(s32)i,(s32)ecm_app_ctx->u32TxCurMinNum);      
        (void)  printk("\r The ECM[%d]:当前发包超时时间              :0x%x\n",(s32)i,(s32)ecm_app_ctx->u32TxCurTimeout);        
        (void)  printk("\r     The ECM[%d]:PS配置的包封装结构buf偏移量   :0x%x\n",
            (s32)i,(s32)ecm_app_ctx->pkt_encap_info[0]); 
        (void)  printk("\r     The ECM[%d]:PS配置的包封装结构len偏移量   :0x%x\n",
            (s32)i,(s32)ecm_app_ctx->pkt_encap_info[1]); 
        (void)  printk("\r     The ECM[%d]:PS配置的包封装结构next偏移量  :0x%x\n",
            (s32)i,(s32)ecm_app_ctx->pkt_encap_info[2]); 
        (void)  printk("\r The ECM[%d]:debug信息结构                  :0x%x\n",(s32)i,(s32)&ecm_app_ctx->debug_info);
        (void)  printk("\r The ECM[%d]:PS配置的收发包统计             :0x%x\n",(s32)i,(s32)&ecm_app_ctx->pkt_stats); 
    }
}

void ecm_vendorctx_show(void)
{
    u32 i;/*lint !e578*/
    ecm_app_ctx_t *app_ctx;
    ecm_vendor_ctx_t *vendor_ctx;

    for (i = 0; i < GNET_MAX_NUM; i++)
    {
        app_ctx = ecm_get_ctx(i);
        vendor_ctx = app_ctx->vendor_ctx;

        if(!vendor_ctx)
        {
            continue;
        }
        
        printk("|-+ecm[%d] vendor ctx show  :\n",i);
        printk("| |--connect                :%u\n",vendor_ctx->connect);
        printk("| |--tx_task_run            :%u\n",vendor_ctx->tx_task_run);
        printk("| |--u32UpBitRate           :%u\n",vendor_ctx->speed.u32UpBitRate);
        printk("| |--u32DownBitRate         :%u\n",vendor_ctx->speed.u32DownBitRate);
        printk("| |--stat_rx_total          :%lu\n",vendor_ctx->stat_rx_total);
        printk("| |--stat_rx_einval         :%lu\n",vendor_ctx->stat_rx_einval);
        printk("| |--stat_tx_total          :%lu\n",vendor_ctx->stat_tx_total);
        printk("| |--stat_tx_cancel         :%lu\n",vendor_ctx->stat_tx_cancel);
        printk("| |--stat_tx_xmit           :%lu\n",vendor_ctx->stat_tx_xmit);
        printk("| |--stat_tx_xmit_fail      :%lu\n",vendor_ctx->stat_tx_xmit_fail);
        printk("| |--stat_tx_post           :%lu\n",vendor_ctx->stat_tx_post);
        printk("| |--stat_tx_drop           :%lu\n",vendor_ctx->stat_tx_drop);
        printk("| |--stat_tx_nochl          :%lu\n",vendor_ctx->stat_tx_nochl);
        printk("| |--stat_tx_nodev          :%lu\n",vendor_ctx->stat_tx_nodev);
        printk("| |--stat_notify_timeout    :%lu\n",vendor_ctx->stat_notify_timeout);
    }
}

s32 ecm_test_flow_control(u32 state)
{
    u32 ctl_state = state;
    return ecm_vendor_ioctl(0, ECM_IOCTL_FLOW_CTRL_NOTIF, &ctl_state);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

