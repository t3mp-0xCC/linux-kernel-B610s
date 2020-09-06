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
#include "product_config.h"

#if (FEATURE_ON == MBB_FEATURE_BIP)

#include "bip_client_to_server_communication.h"

#include "mbb_drv_bip.h"

#define BIP_DEV_PRINTF(fmt...)
#ifndef BIP_DEV_PRINTF
#define BIP_DEV_PRINTF(fmt...)      printk(fmt)
#endif
/****************************************************************************
 外部函数原型说明     
 
*****************************************************************************/
extern int device_event_report(void *data, int len);

/****************************************************************************
 内部函数原型说明
 
*****************************************************************************/

/****************************************************************************
 全局变量申明                         
 
*****************************************************************************/
BIP_Command_Event_STRU  g_BipDeviceStru;
BIP_Command_Event_STRU  g_BipClientEvent;

/*lint -e19 -e24 -e43 -e63 -e64 -e65 -e133  */
/*创建BIP字符设备*/
static const struct file_operations bip_device_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = BipDeviceIoctl,
    .open   = BipDeviceOpen,
    .release = BipDeviceRelease,
};

/*BIP字符设备绑定*/
static struct miscdevice bip_miscdev = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "bip_dev",
    .fops = &bip_device_fops
};

/*lint +e19 +e24 +e43 +e63 +e64 +e65 +e133  */


static VOSM_INT32 BipDeviceOpen(struct inode *inode, struct file *file)
{
    if (NULL != inode && NULL != file)
    {
       ;
    }
    return 0;
}


static VOSM_INT32 BipDeviceRelease(struct inode *inode, struct file *file)
{
    if (NULL != inode && NULL != file)
    {
        ;
    }

    return 0;
}


VOSM_VOID BipDeviceShowInfo (BIP_Command_Event_STRU *Data)
{
    VOSM_UINT32 message_tpye = 0;

    message_tpye = Data->CmdType;
    BIP_DEV_PRINTF("BIP TEST: ChannelNum:%d. message_tpye=%d\n", Data->ChannelNum, message_tpye);

    switch(message_tpye)
    {
        /*lint -e30 -e515*/
        case BIP_OPEN_CHANNEL_REQ:
        { 
            BIP_DEV_PRINTF("BIP TEST:BIP_OPEN_CHANNEL_REQ,BearerDesp=%d\n", Data->CmdEventStru.open_channel_req.BearerDesp);
            BIP_DEV_PRINTF("BIP TEST:BIP_OPEN_CHANNEL_REQ,IPAddrType=%d,ApnLen=%d,BufferSize=%d,PortNum=%d\n",
                Data->CmdEventStru.open_channel_req.IPAddrType, 
                Data->CmdEventStru.open_channel_req.ApnLen,
                Data->CmdEventStru.open_channel_req.BufferSize,
                Data->CmdEventStru.open_channel_req.PortNum);

            break;
        }

        case BIP_CLOSE_CHANNEL_REQ:
        {
            BIP_DEV_PRINTF("BIP TEST:BIP_CLOSE_CHANNEL_REQ\n");
            break;
        }

        case BIP_RECEIVE_DATA_REQ:
        {
            BIP_DEV_PRINTF("BIP TEST:BIP_RECEIVE_DATA_REQ ReqDataLen:%d\n",
                            Data->CmdEventStru.receive_data_req.ReqDataLen);
            break;
        }

        case BIP_SEND_DATA_REQ:
        {   
            BIP_DEV_PRINTF("BIP TEST:BIP_SEND_DATA_REQ,SendDataMode=%d,ChannelDataLen=%d\n",
                            Data->CmdEventStru.send_data_req.SendDataMode,
                            Data->CmdEventStru.send_data_req.ChannleDataLen);
            break;
        }

        case BIP_GET_CHANNEL_STATUS_REQ:
        {        
            BIP_DEV_PRINTF("BIP TEST:BIP_GET_CHANNEL_STATUS_REQ\n");
            break;
        }

        /*lint +e30 +e515*/

        default:
            break;
    }
}


static VOSM_LONG BipDeviceIoctl(struct file *file, VOSM_UINT32  cmd, VOSM_ULONG arg)
{        
    VOSM_LONG ret = VOSM_OK;
    
    if (NULL == file)
    {  
        BIP_DEV_PRINTF(" balong_bip_ioctl receive fail,file is null \n");
        return VOSM_ERROR;
    }

    switch (cmd)
    {
        /*往BIP字符设备，写入内容*/
        case OTA_TO_UICC:
            ret = (VOSM_LONG)copy_from_user((VOSM_VOID*)&g_BipClientEvent, (VOSM_VOID*)arg, sizeof(g_BipClientEvent));   /*lVOSM_INT32 !e420 */
            if ( 0 == ret )
            {
                ret = (VOSM_LONG)BipDeviceReceiveDataFromBipClient((BIP_Command_Event_STRU *)&g_BipClientEvent);
            }
            break;
        
        /*读取BIP字符设备的内容*/
        case UICC_TO_OTA:
            ret = (long)copy_to_user((VOSM_VOID*)arg, (VOSM_VOID*)&g_BipDeviceStru, sizeof(g_BipDeviceStru)); /*lint !e420 */
            break;   
        
        default:
            break;
    }

    if (VOSM_OK != ret)
    {
        BIP_DEV_PRINTF("balong_bip_ioctl event=%d error:%ld\n", cmd, ret);
    }

    return ret;
}


VOSM_INT32 BipDeviceInit(VOSM_VOID)
{
    VOSM_INT32 ret  = 0;

    ret = misc_register(&bip_miscdev);

    if (VOSM_OK != ret)
    {
        BIP_DEV_PRINTF("BIP init, misc_register failed.s32Ret=%d \n",ret);  
    }

    memset(&g_BipDeviceStru, 0x00, sizeof(g_BipDeviceStru));
    memset(&g_BipClientEvent, 0x00, sizeof(g_BipClientEvent));
    
    return ret;
}


VOSM_INT32 BipDeviceReceiveDataFromBipClient( BIP_Command_Event_STRU *pDst)
{
    VOSM_INT32 ret = VOSM_OK;
    VOSM_UINT32 message_tpye = 0;

    if (NULL == pDst)
    {
        return VOSM_ERROR;
    }
    BIP_DEV_PRINTF("BipDeviceReceiveDataFromBipClient()\n MessageTpye=%d", pDst->CmdType);

    message_tpye = pDst->CmdType;

    switch(message_tpye)
    {  
        case BIP_OPEN_CHANNEL_RSP:
        { 
            if (0 != pDst->ResultValue)
            {
                (VOSM_VOID)BipDeviceNotifyDataToBipClient(pDst, UICC_COMPLETE);
            }
        }
        break;
        case BIP_CLOSE_CHANNEL_RSP:
        {
            if (0 == pDst->ResultValue)
            {
                (VOSM_VOID)BipDeviceNotifyDataToBipClient(pDst, UICC_COMPLETE);
            }
        }
        break;
        default:
            break;
    }
    
    ret = BipDeviceSendDataToModem(pDst);
    
    return ret; 
}


VOSM_INT32 BipDeviceSendDataToModem( BIP_Command_Event_STRU *stru)
{
    VOSM_INT32 ret = VOSM_OK;
    VOSM_UINT16 len = (VOSM_UINT16)sizeof(BIP_Command_Event_STRU);
    
    if (NULL == stru)
    {
        return VOSM_ERROR;
    }

    ret = SI_STK_DataSendBipEvent( len, (VOSM_UINT8*)stru);
    
    if (VOSM_OK != ret)
    {
        BIP_DEV_PRINTF("------BipDeviceSendDataToModem fail! ret=%d\n", ret);
        return VOSM_ERROR;
    }
    
    return ret;
}


VOSM_INT32 BipDeviceReceiveDataFromModem(VOSM_VOID *pMsgBody, VOSM_UINT32 u32Len)
{
    BIP_Command_Event_STRU *bip_event_data = &g_BipDeviceStru;
    VOSM_INT32  ret = 0;

    if ((NULL == pMsgBody) || (u32Len > sizeof(BIP_Command_Event_STRU)))
    {
        BIP_DEV_PRINTF("------BSP_BIP_CLIENT_ReceiveArg is null \n-----");
        return VOSM_ERROR;
    }

    /*保存数据到g_BipDeviceStru，等待BIP Client Fetch数据*/
    if ( bip_event_data != (BIP_Command_Event_STRU *)pMsgBody )
    {
        memcpy((VOSM_VOID*)bip_event_data, pMsgBody, u32Len);
    }
    
    /*打印M核BIP消息内容*/
    BipDeviceShowInfo(bip_event_data);

    /*通知BIP Client有数据*/
    ret = BipDeviceNotifyDataToBipClient(bip_event_data, UICC_INFORM);

    return ret;
}

VOSM_VOID bip_event_receiver(VOSM_VOID *pMsgBody, VOSM_INT32 u32Len)
{
    BipDeviceReceiveDataFromModem(pMsgBody, u32Len);
}

EXPORT_SYMBOL(bip_event_receiver); /*lint !e19 !e323 */


VOSM_INT32 BipDeviceNotifyDataToBipClient( BIP_Command_Event_STRU *pDst, BIP_EVENT event_code)
{
    VOSM_UINT32 size = 0;
    VOSM_INT32  ret = 0;
    DEVICE_EVENT event; /*lint !e10 !e522 */

    if (NULL == pDst)
    {
        BIP_DEV_PRINTF("------BipDeviceNotifyDataToBipClient pDst is NULL! \n");
        return VOSM_ERROR;
    }
    
    /*封装NetLink结构，指定Event的DeviceId,EventCode,Length*/
    /*lint -e10 -e63*/
    event.device_id  = DEVICE_ID_BIP; 
    event.event_code = (VOSM_INT32)event_code;
    event.len = 0;
    size  = sizeof(DEVICE_EVENT);
    /*lint +e10 +e63*/

    /*通知BIP Client有数据，等待BIP Client Fetch数据*/
    ret = (VOSM_INT32)device_event_report((VOSM_VOID*)&event, (VOSM_INT32)size);

    if (VOSM_OK != ret)
    {
       BIP_DEV_PRINTF("BipDeviceNotifyDataToBipClient  Fail-----\n");
    }

    return ret;
}

/*lint -e10 -e528 */
/*模块初始化*/
late_initcall(BipDeviceInit);
/*lint +e10 +e528 */

#if (FEATURE_ON == MBB_FEATURE_BIP_TEST)

VOSM_VOID BipClientTestOpenChannel(VOSM_UINT8 ResultValue)
{
    BIP_Command_Event_STRU  bip_stru;
    memset((VOSM_VOID*)&bip_stru, 0, sizeof(BIP_Command_Event_STRU));
    
    bip_stru.CmdType = BIP_OPEN_CHANNEL_RSP;
    bip_stru.ResultValue = ResultValue;
    bip_stru.ChannelNum = 0 ;

    bip_stru.CmdEventStru.open_channel_rsp.BufferSize = BIP_MAX_BUFFER_LENGTH;
    
    bip_stru.CmdEventStru.open_channel_rsp.IPAddrType = 0x21;  /* ipv4 */
    bip_stru.CmdEventStru.open_channel_rsp.IPAddrLen = 4;       /* ipv4 */
    bip_stru.CmdEventStru.open_channel_rsp.IPAddr[0] = 81;      /* ipv4 */
    bip_stru.CmdEventStru.open_channel_rsp.IPAddr[1] = 23;      /* ipv4 */
    bip_stru.CmdEventStru.open_channel_rsp.IPAddr[2] = 12;      /* ipv4 */
    bip_stru.CmdEventStru.open_channel_rsp.IPAddr[3] = 21;      /* ipv4 */
    
    bip_stru.CmdEventStru.open_channel_rsp.ChannelStatus = CHANNEL_OPEN;
    
    BipDeviceReceiveDataFromBipClient(&bip_stru);
}


VOSM_VOID BipClientTestCloseChannel(VOSM_VOID)
{
    BIP_Command_Event_STRU bip_stru;
    memset((VOSM_VOID*)&bip_stru, 0, sizeof(BIP_Command_Event_STRU));
    
    BIP_DEV_PRINTF( " bip_client_test_close_channel \n");
    bip_stru.CmdType = BIP_CLOSE_CHANNEL_RSP;
    bip_stru.ResultValue = 0;
    
    BipDeviceReceiveDataFromBipClient(&bip_stru);
}


VOSM_VOID BipClientTestSendData(VOSM_UINT8 dataLen)
{    
    BIP_Command_Event_STRU bip_stru;
    memset((VOSM_VOID*)&bip_stru, 0, sizeof(BIP_Command_Event_STRU));
    
    bip_stru.CmdType = BIP_SEND_DATA_RSP;
    bip_stru.ResultValue = 0;
    
    bip_stru.CmdEventStru.send_data_rsp.IdleChannleDataLen = dataLen;
    
    BipDeviceReceiveDataFromBipClient(&bip_stru);
}


VOSM_VOID BipClientTestReceiveData( VOSM_UINT8 dataLen, VOSM_UINT8 LeftDataLen  )
{
    BIP_Command_Event_STRU bip_stru;
    VOSM_UINT8 i;

    memset((VOSM_VOID*)&bip_stru, 0, sizeof(BIP_Command_Event_STRU));
        
    bip_stru.CmdType = BIP_RECEIVE_DATA_RSP;
    bip_stru.ResultValue = 0;
    
    bip_stru.CmdEventStru.receive_data_rsp.DataLen = dataLen;
    bip_stru.CmdEventStru.receive_data_rsp.LeftDataLen = LeftDataLen;

    for ( i = 0; i < dataLen; i++ )
    {
        bip_stru.CmdEventStru.receive_data_rsp.ChannelData[i] = i;
    }
    
    BipDeviceReceiveDataFromBipClient(&bip_stru);
}


VOSM_VOID BipClientTestGetChannelStatus(VOSM_UINT8 ChannelStatus)
{
    BIP_Command_Event_STRU bip_stru;
    memset((VOSM_VOID*)&bip_stru,0,sizeof(BIP_Command_Event_STRU));
        
    bip_stru.CmdType = BIP_GET_CHANNEL_STATUS_RSP;
    bip_stru.ResultValue = 0;
    
    bip_stru.ChannelNum = 0 ;
    bip_stru.CmdEventStru.channel_status_rsp.ChannelStatus = ChannelStatus;
    
    BipDeviceReceiveDataFromBipClient(&bip_stru);
}


VOSM_VOID BipClientTestChannelStatusNotify(VOSM_UINT8 ChannelStatus)
{    
    BIP_Command_Event_STRU bip_stru;
    memset((VOSM_VOID*)&bip_stru, 0, sizeof(BIP_Command_Event_STRU));
        
    bip_stru.CmdType = BIP_CHANNEL_STATUS_NOTIFY;
    bip_stru.ResultValue = 0;
    bip_stru.ChannelNum = 0;
    
    bip_stru.CmdEventStru.channel_status_event.ChannelStatus = ChannelStatus;
    
    BipDeviceReceiveDataFromBipClient(&bip_stru);
}


VOSM_VOID BipClientTestFail(VOSM_UINT8 CmdType, VOSM_UINT8 ResultVal)
{    
    BIP_Command_Event_STRU bip_stru;
    memset((VOSM_VOID*)&bip_stru, 0, sizeof(BIP_Command_Event_STRU));
        
    bip_stru.CmdType = CmdType;
    bip_stru.ResultValue = ResultVal;
        
    BipDeviceReceiveDataFromBipClient(&bip_stru);
}

#endif /*#if (FEATURE_ON == MBB_FEATURE_BIP_TEST_KERNEL) */

#endif/*(FEATURE_ON == MBB_FEATURE_BIP)*/

