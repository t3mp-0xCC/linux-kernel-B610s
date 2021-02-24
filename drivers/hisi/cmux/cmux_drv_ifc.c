/******************************************************************************
  Copyright , 1988-2014, Huawei Tech. Co., Ltd.
  File name:      cmux_drv_ifc.c
  Author: shiyanan / zhangtian     Version:  1.00      Date:  20140804
  Description:   This file supplies the interface of cmux and HSUART driver.
  Others:         None
******************************************************************************/


/******************************************************************************
ͷ�ļ�����
******************************************************************************/
#include "cmux_framer.h"
#include "cmux_api_ifc.h"
#include "cmux_tgt_osal.h"
#include "cmux_drv_ifc.h"
#include "mdrv_hsuart.h"

#if ( FEATURE_ON == MBB_HSUART_CMUX )
/******************************************************************************
��������
******************************************************************************/
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*���CMUX��handle*/
UDI_HANDLE gCmuxUdiHandle = UDI_INVALID_HANDLE;/*lint !e129*/

/*���cmȫ�ֱ���*/
struct cmuxlib * gCmux = NULL;

struct delayed_work at_cmux_expiry_timer;

extern cmux_close_cb_t close_port_cb;
unsigned char gCmuxClosePort = FALSE;
static unsigned long cmux_baud[] = {9600, 19200, 38400, 57600, 115200, 230400,
    460800, 921600, 1000000, 1152000, 1500000, 2000000, 2500000, 3000000};


/******************************************************************************
����ʵ��
******************************************************************************/

/******************************************************************************
Function:       SetCmuxClosePort
Description:    ����gCmuxClosePortȫ�ֱ���ֵ(����balongƽ̨)
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
Description:    CMUXģ���ʼ��������CMUXģʽ(����balongƽ̨)
Input:          *CmuxInfo :AT+CMUX��������Я���Ĳ���
Output:         None
Return:         None
Others:         None
******************************************************************************/
int CMUX_InitPort(cmux_info_type *CmuxInfo)
{
    struct cmux_callback cmux_cb;/*lint !e565*/
    unsigned char response[] = "\r\nOK\r\n";
    int ret = PASS;

    /*��ʼ��cmuxlib�����Ѳ���cm�浽ȫ�ֱ���g_cmux����*/
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
    
    /*���ûص����������ڷ�������*/
    ret = Cmux_register_callback(gCmux, &cmux_cb);
    if ( FAIL == ret )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "register callback error\n");
        //CMUX_MLOG_RECORD("CMUX_InitPort, register callback error\n");
        return 0;
    }
        
    /*��CMUX��HSUART�˿�*/
    CMUX_OpenDevice();
    
    /*����AT+CMUX����Ӧ:ok*/
    (void)CMUX_HSUART_SendDlData(response, strlen(response) + 1);/*lint !e64*/

    /*��ʼ��AT+CMUX��ʱ��ʱ��*/
    CMUX_init_timer(&at_cmux_expiry_timer, CMUX_at_timer_expiry_cb);

    /*������ʱ����10����δ�յ�SABM 0֡���Զ�������ͨģʽ*/
    CMUX_timer_begin(&at_cmux_expiry_timer, AT_CMUX_EXPIRY_TIME);

    /*��¼MLOG�¼���CMUX����*/
    //CMUX_MLOG_RECORD("CMUX_InitPort, CMUX OPEN\n");
    return 1;
}

/******************************************************************************
Function:       CMUX_ClosePort
Description:    �˳�CMUXģʽ���ָ�ATģʽ(����balongƽ̨)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_ClosePort(void)
{
    /*�ر�CMUX��HSUART��ͨ��*/
    CMUX_CloseDevice();

    /*�ر�CMUX��AT��ͨ��,�ָ�AT��HSUART��ͨ��*/
    if (NULL != close_port_cb)
    {
        close_port_cb();
    }

    /* �ر�ͨ�����ͷ�pending_frame */
    if ( NULL != gCmux )
    {
        if ( NULL != gCmux->cmux_if.pending_frame )
        {
            cmux_free((unsigned char *)gCmux->cmux_if.pending_frame);
        }
        
        cmux_free((unsigned char *)gCmux);
        gCmux = NULL;
    }
    
    /*��¼MLOG�¼���CMUX�ر�*/
    //CMUX_MLOG_RECORD("CMUX_ClosePort, CMUX CLOSE\n");
    
    return;
}
/******************************************************************************
Function:       CMUX_HSUART_UlDataReadCB
Description:    ��ȡHSUART����������buffer���͵�CMUX lib����(����balongƽ̨)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_HSUART_UlDataReadCB(void)
{
    /*�������ָ������ݳ���*/
    char *pucData = NULL;
    unsigned char *pData = NULL;
    unsigned int ulLen = 0;
    
    //������buffer����ȡ����
    if (PASS == CMUX_HSUART_GetUlDataBuff(&pucData, &ulLen))
    {
        /* �����豸��ǰģʽ���ַ��������� */
        pData = cmux_get_skb((unsigned char *)pucData);
        if ( NULL == pData )
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "pData is NULL\n");
        }
        (void)Cmux_Input_frame_to_CMUX (gCmux, (unsigned char *)pData, ulLen);

        /*���ݴ����꣬�ͷ�*/
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
Description:    ����IOCTL������buffer��ȡ����(����balongƽ̨)
Input:          None
Output:         None
Return:         Pass :�ɹ�
                Fail :ʧ��
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

    /* ���UDI�����Ч�� */
    lUdiHandle = gCmuxUdiHandle;
    if (UDI_INVALID_HANDLE == lUdiHandle)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Invalid UDI handle!\n");
        return FAIL;
    }

    /* ��ȡ������������BUFFER */
    stCtlParam.pBuffer  = NULL;
    stCtlParam.u32Size  = 0;
    stCtlParam.pDrvPriv = NULL;

    lResult = mdrv_udi_ioctl(lUdiHandle, UART_IOCTL_GET_RD_BUFF, &stCtlParam);
    if (CMUX_SUCCESS != lResult)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Get buffer failed!\n");
        return FAIL;
    }

    /* ������Ч�Լ�� */
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
Description:    ���������ڴ��ͷ�(����balongƽ̨)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_HSUART_FreeDlDataBuff(char *pucData)
{
    /*�ͷ��ڴ�ĺ���*/
    if (NULL != pucData)
    {
        cmux_free_skb((unsigned char *)pucData);
    }

    return;
}

/******************************************************************************
Function:       CMUX_HSUART_FreeUlDataBuff
Description:    ���������ڴ��ͷ�(����balongƽ̨)
Input:          *pucData :��Ҫ�ͷŵ�����
Output:         None
Return:         Pass :�ɹ�
                Fail :ʧ��
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_FreeUlDataBuff(char *pucData)
{
    cmux_hsuart_wr_async_info           stCtlParam;
    UDI_HANDLE                          lUdiHandle;
    signed int                          lResult;

    /* ���UDI�����Ч�� */
    lUdiHandle = gCmuxUdiHandle;
    if (UDI_INVALID_HANDLE == lUdiHandle)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Invalid UDI handle!\n");
        return FAIL;
    }

    /* ��д���ͷŵ��ڴ��ַ */
    stCtlParam.pBuffer  = pucData;
    stCtlParam.u32Size  = 0;
    stCtlParam.pDrvPriv = NULL;

    /* ����ִ���ͷ��ڴ���� */
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
Description:    ��CMUX��HSUART�豸,ע��ص�����(����balongƽ̨)
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

    /* ��Device�����ID */
    lUdiHandle = mdrv_udi_open(&stParam);
    if (UDI_INVALID_HANDLE != lUdiHandle)
    {
        /* ע��UART�豸�������ݽ��ջص� */
        if (CMUX_SUCCESS != mdrv_udi_ioctl(lUdiHandle, 
            UART_IOCTL_SET_READ_CB, CMUX_HSUART_UlDataReadCB))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Register Read Callback Error!\n");
        }
        
        /* ע��UART���������ڴ��ͷŽӿ� */
        if (CMUX_SUCCESS != mdrv_udi_ioctl(lUdiHandle, 
            UART_IOCTL_SET_FREE_CB, CMUX_HSUART_FreeDlDataBuff))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Register Free Callback Error!\n");
        }
        
        /* ע��ܽ��ź�֪ͨ�ص� */
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
Description:    �ر�CMUX��HSUART�Ľӿ�(����balongƽ̨)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_CloseDevice(void)
{
    UDI_HANDLE                          lUdiHandle;

    /* �ͷ���ػص� */
    lUdiHandle = gCmuxUdiHandle;
    if (UDI_INVALID_HANDLE != lUdiHandle)
    {
        /* ע��UART�豸�������ݽ��ջص� */
        if (CMUX_SUCCESS != mdrv_udi_ioctl(lUdiHandle, 
            UART_IOCTL_SET_READ_CB, NULL))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Free Read Callback Error!\n");
        }
        
        /* ע��UART���������ڴ��ͷŽӿ� */
        if (CMUX_SUCCESS != mdrv_udi_ioctl(lUdiHandle, 
            UART_IOCTL_SET_FREE_CB, NULL))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Free Free Callback Error!\n");
        }
        
        /* ע��ܽ��ź�֪ͨ�ص� */
        if (CMUX_SUCCESS != mdrv_udi_ioctl(lUdiHandle, 
            UART_IOCTL_SET_MSC_READ_CB, NULL))
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Free Msc Read Callback Error!\n");
        }
    }
    
    /* �ر�Device */
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
Description:    ��cmux�����ݷ��͸���������(����balongƽ̨)
Input:          None
Output:         None
Return:         Pass :�ɹ�
                Fail :ʧ��
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_WriteDataAsync(char *pucData)
{
    cmux_hsuart_wr_async_info           stCtlParam;
    UDI_HANDLE                          lUdiHandle;
    signed int                          lResult;

    /* ���UDI�����Ч�� */
    lUdiHandle = gCmuxUdiHandle;
    if (UDI_INVALID_HANDLE == lUdiHandle)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Invalid UDI handle!\r\n");
        CMUX_HSUART_FreeDlDataBuff(pucData);
        return FAIL;
    }

    /* ��д�������ڴ��ַ */
    stCtlParam.pBuffer  = pucData;
    stCtlParam.u32Size  = 0;
    stCtlParam.pDrvPriv = NULL;

    /* �첽��ʽд�� */
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
Description:    CMUX lib��HSUART�������ݽӿ�(����balongƽ̨)
Input:          None
Output:         None
Return:         Pass :�ɹ�
                Fail :ʧ��
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_SendDlData(
    unsigned char                          *pucData,
    unsigned short                          usLen
)
{
    unsigned char                      *pcPutData = NULL;
    unsigned int                        ulResult;

    /* ����ռ� */
    pcPutData = cmux_alloc_skb((int)usLen);
    if (NULL == pcPutData)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "pcPutData null error!\r\n");
        return FAIL;
    }

    /* �������� */
    cmux_put_skb(pcPutData, pucData, usLen);

    /* �첽дHSUART�豸, д�ɹ����ڴ��ɵ������ͷ� */
    ulResult = CMUX_HSUART_WriteDataAsync((char *)pcPutData);

    return ulResult;
}

/******************************************************************************
Function:       CMUX_HSUART_setPowerStatus
Description:    CMUX lib����HSUART˯��ͶƱ�ӿ�(����balongƽ̨)
Input:          None
Output:         None
Return:         Pass :�ɹ�
                Fail :ʧ��
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_setPowerStatus(void)
{
    signed int                          lResult;
    UDI_HANDLE                          lUdiHandle;

    /* ���UDI�����Ч�� */
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
Description:    CMUX lib��HSUART�ܽ��ź�д�ӿ�(����balongƽ̨)
Input:          ucIndex    - �˿�����
                pstDceMsc  - �ܽ��źŽṹ(�ɵ����߱�֤�ǿ�)
Output:         None
Return:         PASS - �ɹ�
                FAIL - ʧ��
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_WriteMscCmd(cmux_modem_msc_stru *pstDceMsc)
{
    UDI_HANDLE                          lUdiHandle;
    signed int                          lResult;

    /* ���UDI�����Ч�� */
    lUdiHandle = gCmuxUdiHandle;
    if (UDI_INVALID_HANDLE == lUdiHandle)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "Invalid UDI handle!\r\n");
        return FAIL;
    }

    /* д�ܽ��ź� */
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
Description:    HSUARTӲ���ܽ��źŶ��ص�
Input:          pstDceMsc - �ܽ��ź���Ϣ
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_HSUART_MscReadCB(void *pParam)
{
    unsigned char DlcIndex;
    cmux_modem_msc_stru* pstDceMsc;
    
    /* ��μ�� */
    if (NULL == pParam)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "pstDceMsc is NULL!\r\n");
        return;
    }
    pstDceMsc = (cmux_modem_msc_stru *)pParam;
    
    /* Ĭ����DLC 0ͨ������ */
    DlcIndex = CMUX_UDI_DLC_CTL;

    /* ��AT�෢��msc״̬��Ϣ */
    CMUX_PRINT(CMUX_DEBUG_INFO, "DlcIndex is %d, OP_Dtr is %d, ucDtr is %d!\r\n", 
        DlcIndex, pstDceMsc->OP_Dtr, pstDceMsc->ucDtr);
    cmux_send_msc_status(DlcIndex, pstDceMsc);

    return;
}

/******************************************************************************
Function:       CMUX_timer_expiry_cb
Description:    AT+CMUX��ʱ��Ļص�����(����balongƽ̨)
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
Description:    �жϲ������Ƿ���CMUX֧�ַ�Χ
Input:          enBaudRate
Output:         unsigned int ���ز���������Ӧ��CMUX NUMֵ,�Ƿ��Ʒ���0
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
