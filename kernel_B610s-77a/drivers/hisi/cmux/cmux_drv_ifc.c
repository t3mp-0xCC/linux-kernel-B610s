/******************************************************************************
  Copyright , 1988-2014, Huawei Tech. Co., Ltd.
  File name:      cmux_drv_ifc.c
  Author: shiyanan / zhangtian     Version:  1.00      Date:  20140804
  Description:   This file supplies the interface of cmux and HSUART driver.
  Others:         None
******************************************************************************/


/******************************************************************************
头文件声明
******************************************************************************/
#include "cmux_framer.h"
#include "cmux_api_ifc.h"
#include "cmux_tgt_osal.h"
#include "cmux_drv_ifc.h"
#include "mdrv_hsuart.h"

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

/*存放CMUX的handle*/
UDI_HANDLE gCmuxUdiHandle = UDI_INVALID_HANDLE;/*lint !e129*/

/*存放cm全局变量*/
struct cmuxlib * gCmux = NULL;

struct delayed_work at_cmux_expiry_timer;

extern cmux_close_cb_t close_port_cb;
unsigned char gCmuxClosePort = FALSE;
static unsigned long cmux_baud[] = {9600, 19200, 38400, 57600, 115200, 230400,
    460800, 921600, 1000000, 1152000, 1500000, 2000000, 2500000, 3000000};


/******************************************************************************
函数实现
******************************************************************************/

/******************************************************************************
Function:       SetCmuxClosePort
Description:    设置gCmuxClosePort全局变量值(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void SetCmuxClosePort(unsigned char value)
{
    gCmuxClosePort = value;
    return;
}

/******************************************************************************
Function:       CMUX_InitPort
Description:    CMUX模块初始化，开启CMUX模式(适配balong平台)
Input:          *CmuxInfo :AT+CMUX设置命令携带的参数
Output:         None
Return:         None
Others:         None
******************************************************************************/
int CMUX_InitPort(cmux_info_type *CmuxInfo)
{
    struct cmux_callback cmux_cb;/*lint !e565*/
    unsigned char response[] = "\r\nOK\r\n";
    int ret = PASS;

    /*初始化cmuxlib，并把参数cm存到全局变量g_cmux里面*/
    gCmux = Cmux_cmuxlib_init(CMUX_UEMODE, CmuxInfo);
    if ( NULL == gCmux )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "lib init error\n");
        //CMUX_MLOG_RECORD("CMUX_InitPort, lib init error\n");
        return 0;
    }

    cmux_cb.callback_func = cmux_callback_func;  
    cmux_cb.send_dl_data = CMUX_HSUART_SendDlData;/*lint !e64*/
    cmux_cb.set_power_cb = CMUX_HSUART_setPowerStatus;
    
    /*设置回调函数，用于发送数据*/
    ret = Cmux_register_callback(gCmux, &cmux_cb);
    if ( FAIL == ret )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "register callback error\n");
        //CMUX_MLOG_RECORD("CMUX_InitPort, register callback error\n");
        return 0;
    }
        
    /*打开CMUX与HSUART端口*/
    CMUX_OpenDevice();
    
    /*发送AT+CMUX的响应:ok*/
    (void)CMUX_HSUART_SendDlData(response, strlen(response) + 1);/*lint !e64*/

    /*初始化AT+CMUX超时定时器*/
    CMUX_init_timer(&at_cmux_expiry_timer, CMUX_at_timer_expiry_cb);

    /*启动定时器，10秒内未收到SABM 0帧，自动返回普通模式*/
    CMUX_timer_begin(&at_cmux_expiry_timer, AT_CMUX_EXPIRY_TIME);

    /*记录MLOG事件，CMUX开启*/
    //CMUX_MLOG_RECORD("CMUX_InitPort, CMUX OPEN\n");
    return 1;
}

/******************************************************************************
Function:       CMUX_ClosePort
Description:    退出CMUX模式，恢复AT模式(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_ClosePort(void)
{
    /*关闭CMUX与HSUART的通道*/
    CMUX_CloseDevice();

    /*关闭CMUX与AT的通道,恢复AT与HSUART的通道*/
    if (NULL != close_port_cb)
    {
        close_port_cb();
    }

    /* 关闭通道，释放pending_frame */
    if ( NULL != gCmux )
    {
        if ( NULL != gCmux->cmux_if.pending_frame )
        {
            cmux_free((unsigned char *)gCmux->cmux_if.pending_frame);
        }
        
        cmux_free((unsigned char *)gCmux);
        gCmux = NULL;
    }
    
    /*记录MLOG事件，CMUX关闭*/
    //CMUX_MLOG_RECORD("CMUX_ClosePort, CMUX CLOSE\n");
    
    return;
}
/******************************************************************************
Function:       CMUX_HSUART_UlDataReadCB
Description:    获取HSUART的上行数据buffer，送到CMUX lib处理(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_HSUART_UlDataReadCB(void)
{
    /*存放数据指针和数据长度*/
    char *pucData = NULL;
    unsigned char *pData = NULL;
    unsigned int ulLen = 0;
    
    //从上行buffer里面取数据
    if (PASS == CMUX_HSUART_GetUlDataBuff(&pucData, &ulLen))
    {
        /* 根据设备当前模式，分发上行数据 */
        pData = cmux_get_skb((unsigned char *)pucData);
        if ( NULL == pData )
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "pData is NULL\n");
        }
        (void)Cmux_Input_frame_to_CMUX (gCmux, (unsigned char *)pData, ulLen);

        /*数据传输完，释放*/
        (void)CMUX_HSUART_FreeUlDataBuff(pucData);

        if (TRUE == gCmuxClosePort)
        {
            gCmuxClosePort = FALSE;
            CMUX_ClosePort();
        }
    }

    return;
}

/******************************************************************************
Function:       CMUX_HSUART_GetUlDataBuff
Description:    调用IOCTL从上行buffer中取数据(适配balong平台)
Input:          None
Output:         None
Return:         Pass :成功
                Fail :失败
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_GetUlDataBuff(
    char                                **pucData,
    unsigned int                        *ulLen
)
{
    cmux_hsuart_wr_async_info           stCtlParam;
    UDI_HANDLE                          lUdiHandle;
    signed int                          lResult;

    /* 检查UDI句柄有效性 */
    lUdiHandle = gCmuxUdiHandle;
    if (UDI_INVALID_HANDLE == lUdiHandle)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Invalid UDI handle!\n");
        return FAIL;
    }

    /* 获取底软上行数据BUFFER */
    stCtlParam.pBuffer  = NULL;
    stCtlParam.u32Size  = 0;
    stCtlParam.pDrvPriv = NULL;

    lResult = mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_GET_RD_BUFF, &stCtlParam);
    if (CMUX_SUCCESS != lResult)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Get buffer failed!\n");
        return FAIL;
    }

    /* 数据有效性检查 */
    if ((NULL == stCtlParam.pBuffer) || (0 == stCtlParam.u32Size))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Data buffer error!\n");
        return FAIL;
    }

    *pucData = stCtlParam.pBuffer;
    *ulLen    = stCtlParam.u32Size;

    return PASS;
}

/******************************************************************************
Function:       CMUX_HSUART_FreeDlDataBuff
Description:    下行数据内存释放(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_HSUART_FreeDlDataBuff(char *pucData)
{
    /*释放内存的函数*/
    if (NULL != pucData)
    {
        cmux_free_skb((unsigned char *)pucData);
    }

    return;
}

/******************************************************************************
Function:       CMUX_HSUART_FreeUlDataBuff
Description:    上行数据内存释放(适配balong平台)
Input:          *pucData :需要释放的内容
Output:         None
Return:         Pass :成功
                Fail :失败
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_FreeUlDataBuff(char *pucData)
{
    cmux_hsuart_wr_async_info           stCtlParam;
    UDI_HANDLE                          lUdiHandle;
    signed int                          lResult;

    /* 检查UDI句柄有效性 */
    lUdiHandle = gCmuxUdiHandle;
    if (UDI_INVALID_HANDLE == lUdiHandle)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Invalid UDI handle!\n");
        return FAIL;
    }

    /* 填写待释放的内存地址 */
    stCtlParam.pBuffer  = pucData;
    stCtlParam.u32Size  = 0;
    stCtlParam.pDrvPriv = NULL;

    /* 底软执行释放内存操作 */
    lResult = mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_RETURN_BUFF, &stCtlParam);
    if (CMUX_SUCCESS != lResult)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Return buffer failed!\n");
        cmux_free((unsigned char *)pucData);
        return FAIL;
    }

    return PASS;
}

/******************************************************************************
Function:       CMUX_OpenDevice
Description:    打开CMUX和HSUART设备,注册回调函数(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_OpenDevice(void)
{
    UDI_OPEN_PARAM_S                      stParam;
    UDI_HANDLE                          lUdiHandle;

    stParam.devid              = UDI_HSUART_0_ID;

    /* 打开Device，获得ID */
    lUdiHandle = mdrv_udi_open(&stParam);
    if (UDI_INVALID_HANDLE != lUdiHandle)
    {
        /* 注册UART设备上行数据接收回调 */
        if (CMUX_SUCCESS != mdrv_udi_ioctl(lUdiHandle, 
            UART_IOCTL_SET_READ_CB, CMUX_HSUART_UlDataReadCB))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Register Read Callback Error!\n");
        }
        
        /* 注册UART下行数据内存释放接口 */
        if (CMUX_SUCCESS != mdrv_udi_ioctl(lUdiHandle, 
            UART_IOCTL_SET_FREE_CB, CMUX_HSUART_FreeDlDataBuff))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Register Free Callback Error!\n");
        }
        
        /* 注册管脚信号通知回调 */
        if (CMUX_SUCCESS != mdrv_udi_ioctl(lUdiHandle, 
            UART_IOCTL_SET_MSC_READ_CB, CMUX_HSUART_MscReadCB))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Set Msc Read Callback Error!\n");
        }
    }
    
    gCmuxUdiHandle = lUdiHandle;

    return;
}

/******************************************************************************
Function:       CMUX_CloseDevice
Description:    关闭CMUX和HSUART的接口(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_CloseDevice(void)
{
    UDI_HANDLE                          lUdiHandle;

    /* 释放相关回调 */
    lUdiHandle = gCmuxUdiHandle;
    if (UDI_INVALID_HANDLE != lUdiHandle)
    {
        /* 注册UART设备上行数据接收回调 */
        if (CMUX_SUCCESS != mdrv_udi_ioctl(lUdiHandle, 
            UART_IOCTL_SET_READ_CB, NULL))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Free Read Callback Error!\n");
        }
        
        /* 注册UART下行数据内存释放接口 */
        if (CMUX_SUCCESS != mdrv_udi_ioctl(lUdiHandle, 
            UART_IOCTL_SET_FREE_CB, NULL))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Free Free Callback Error!\n");
        }
        
        /* 注册管脚信号通知回调 */
        if (CMUX_SUCCESS != mdrv_udi_ioctl(lUdiHandle, 
            UART_IOCTL_SET_MSC_READ_CB, NULL))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Free Msc Read Callback Error!\n");
        }
    }
    
    /* 关闭Device */
    if (UDI_INVALID_HANDLE != lUdiHandle)
    {
        if (CMUX_SUCCESS != mdrv_udi_close(lUdiHandle))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Close CMUX device failed!\r\n");
        }
        
    }
    
    return;
}

/******************************************************************************
Function:       CMUX_HSUART_WriteDataAsync
Description:    将cmux的数据发送给串口驱动(适配balong平台)
Input:          None
Output:         None
Return:         Pass :成功
                Fail :失败
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_WriteDataAsync(char *pucData)
{
    cmux_hsuart_wr_async_info           stCtlParam;
    UDI_HANDLE                          lUdiHandle;
    signed int                          lResult;

    /* 检查UDI句柄有效性 */
    lUdiHandle = gCmuxUdiHandle;
    if (UDI_INVALID_HANDLE == lUdiHandle)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Invalid UDI handle!\r\n");
        CMUX_HSUART_FreeDlDataBuff(pucData);
        return FAIL;
    }

    /* 待写入数据内存地址 */
    stCtlParam.pBuffer  = pucData;
    stCtlParam.u32Size  = 0;
    stCtlParam.pDrvPriv = NULL;

    /* 异步方式写数 */
    lResult = mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_WRITE_ASYNC, &stCtlParam);
    if (CMUX_SUCCESS != lResult)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Write failed!\r\n");
        CMUX_HSUART_FreeDlDataBuff(pucData);
        return FAIL;
    }

    return PASS;

}

/******************************************************************************
Function:       CMUX_HSUART_SendDlData
Description:    CMUX lib向HSUART发送数据接口(适配balong平台)
Input:          None
Output:         None
Return:         Pass :成功
                Fail :失败
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_SendDlData(
    unsigned char                          *pucData,
    unsigned short                          usLen
)
{
    unsigned char                      *pcPutData = NULL;
    unsigned int                        ulResult;

    /* 分配空间 */
    pcPutData = cmux_alloc_skb((int)usLen);
    if (NULL == pcPutData)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "pcPutData null error!\r\n");
        return FAIL;
    }

    /* 拷贝数据 */
    cmux_put_skb(pcPutData, pucData, usLen);

    /* 异步写HSUART设备, 写成功后内存由底软负责释放 */
    ulResult = CMUX_HSUART_WriteDataAsync((char *)pcPutData);

    return ulResult;
}

/******************************************************************************
Function:       CMUX_HSUART_setPowerStatus
Description:    CMUX lib设置HSUART睡眠投票接口(适配balong平台)
Input:          None
Output:         None
Return:         Pass :成功
                Fail :失败
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_setPowerStatus(void)
{
    signed int                          lResult;
    UDI_HANDLE                          lUdiHandle;

    /* 检查UDI句柄有效性 */
    lUdiHandle = gCmuxUdiHandle;
    if (UDI_INVALID_HANDLE == lUdiHandle)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Invalid UDI handle!\r\n");
        return FAIL;
    }
    
    lResult = mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_SLEEP_UNLOCK, NULL);
    if (CMUX_SUCCESS != lResult)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "set hsuart sleep failed!\r\n");
        return FAIL;
    }
    
    return PASS;
}

/******************************************************************************
Function:       CMUX_HSUART_WriteMscCmd
Description:    CMUX lib向HSUART管脚信号写接口(适配balong平台)
Input:          ucIndex    - 端口索引
                pstDceMsc  - 管脚信号结构(由调用者保证非空)
Output:         None
Return:         PASS - 成功
                FAIL - 失败
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_WriteMscCmd(cmux_modem_msc_stru *pstDceMsc)
{
    UDI_HANDLE                          lUdiHandle;
    signed int                          lResult;

    /* 检查UDI句柄有效性 */
    lUdiHandle = gCmuxUdiHandle;
    if (UDI_INVALID_HANDLE == lUdiHandle)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Invalid UDI handle!\r\n");
        return FAIL;
    }

    /* 写管脚信号 */
    lResult = mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_MSC_WRITE_CMD, pstDceMsc);
    if (CMUX_SUCCESS != lResult)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Write failed!\r\n");
        return FAIL;
    }

    return PASS;
}

/******************************************************************************
Function:       CMUX_HSUART_MscReadCB
Description:    HSUART硬件管脚信号读回调
Input:          pstDceMsc - 管脚信号信息
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_HSUART_MscReadCB(void *pParam)
{
    unsigned char DlcIndex;
    cmux_modem_msc_stru* pstDceMsc;
    
    /* 入参检查 */
    if (NULL == pParam)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "pstDceMsc is NULL!\r\n");
        return;
    }
    pstDceMsc = (cmux_modem_msc_stru *)pParam;
    
    /* 默认用DLC 0通道承载 */
    DlcIndex = CMUX_UDI_DLC_CTL;

    /* 往AT侧发送msc状态消息 */
    CMUX_PRINT(CMUX_DEBUG_INFO, "DlcIndex is %d, OP_Dtr is %d, ucDtr is %d!\r\n", 
        DlcIndex, pstDceMsc->OP_Dtr, pstDceMsc->ucDtr);
    cmux_send_msc_status(DlcIndex, pstDceMsc);

    return;
}

/******************************************************************************
Function:       CMUX_timer_expiry_cb
Description:    AT+CMUX超时后的回调函数(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_at_timer_expiry_cb(struct work_struct *w)
{
    CMUX_ClosePort();
}
/******************************************************************************
Function:       CMUX_BaudToNum
Description:    判断波特率是否在CMUX支持范围
Input:          enBaudRate
Output:         unsigned int 返回波特率所对应的CMUX NUM值,非法制返回0
Return:         0,1,2,3,4,5,6,7,8
Others:         None
******************************************************************************/
unsigned int CMUX_BaudToNum(unsigned long enBaudRate)
{
    unsigned  int index = CMUX_PHY_PORT_SPEED_INVALID;
    
    for(index = CMUX_PHY_PORT_SPEED_INVALID; 
        index < CMUX_PHY_PORT_SPEED_14; index++)
    {
        if(cmux_baud[index] == enBaudRate)
        {
            return (index + 1);
        }
    }
    
    return CMUX_PHY_PORT_SPEED_INVALID;
}
#endif /* MBB_HSUART_CMUX */
