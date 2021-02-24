#ifndef CMUX_DRV_IFC_H
#define CMUX_DRV_IFC_H

/******************************************************************************
  Copyright ?, 1988-2014, Huawei Tech. Co., Ltd.
  File name:      cmux_drv_ifc.c
  Author: shiyanan / zhangtian     Version:  1.00      Date:  20140804
  Description:   This file supplies the interface of cmux and HSUART driver.
  Others:         None
******************************************************************************/


#include "mdrv_udi.h"
#include "cmux_api_ifc.h"
#include "cmux_udi_al.h"

#if ( FEATURE_ON == MBB_HSUART_CMUX )
/*----------------------------------------------*
 * ���������Ͷ���˵��                         *
 *----------------------------------------------*/

#define CMUX_SUCCESS               0
#define CMUX_FAILURE               1

/*AT+CMUX���ʱ�ָ�����ģʽʱ��: 10000 ms*/
#define AT_CMUX_EXPIRY_TIME        9500   //��0.5�뻺��ʱ��

typedef enum
{
  CMUX_MODE_MIN = 0,
  CMUX_MODE_BASIC = CMUX_MODE_MIN,
  CMUX_MODE_ADVANCED = 1,
}cmux_mode_enum_type;

/** Frame type definitions based on 3GPP 27.010
*/

typedef enum
{
  CMUX_SUBSET_MIN = 0,
  CMUX_SUBSET_UIH = CMUX_SUBSET_MIN,
  CMUX_SUBSET_UI = 1,
  CMUX_SUBSET_I = 2,
}cmux_subset_enum_type;

/** Port speed definitions based on 3GPP 27.010
*/

typedef enum
{
  CMUX_PHY_PORT_SPEED_INVALID = 0,
  CMUX_PHY_PORT_SPEED_1 = 1,    /* 9,600 bit/s */
  CMUX_PHY_PORT_SPEED_2 = 2,    /* 19,200 bit/s */
  CMUX_PHY_PORT_SPEED_3 = 3,    /* 38,400 bit/s */
  CMUX_PHY_PORT_SPEED_4 = 4,    /* 57,600 bit/s */
  CMUX_PHY_PORT_SPEED_5 = 5,    /* 115,200 bit/s */
  CMUX_PHY_PORT_SPEED_6 = 6,    /* 230,400 bit/s */
  CMUX_PHY_PORT_SPEED_7 = 7,    /* 460,800 bit/s */
  CMUX_PHY_PORT_SPEED_8 = 8,    /* 921,600 bit/s */
  CMUX_PHY_PORT_SPEED_9 = 9,    /* 1,000,000 bit/s */
  CMUX_PHY_PORT_SPEED_10 = 10,  /* 1,152,000 bit/s */
  CMUX_PHY_PORT_SPEED_11 = 11,  /* 1,500,000 bit/s */
  CMUX_PHY_PORT_SPEED_12 = 12,  /* 2,000,000 bit/s */
  CMUX_PHY_PORT_SPEED_13 = 13,  /* 2,500,000 bit/s */
  CMUX_PHY_PORT_SPEED_14 = 14,  /* 3,000,000 bit/s */
}cmux_port_speed_enum_type;

/** Window size definitions based on 3GPP 27.010, applicable only for
    Advanced mode. */

typedef enum
{
  CMUX_WINDOW_SIZE_INVALID = 0,
  CMUX_WINDOW_SIZE_1 = 1,
  CMUX_WINDOW_SIZE_2 = 2,
  CMUX_WINDOW_SIZE_3 = 3,
  CMUX_WINDOW_SIZE_4 = 4,
  CMUX_WINDOW_SIZE_5 = 5,
  CMUX_WINDOW_SIZE_6 = 6,
  CMUX_WINDOW_SIZE_7 = 7,
}cmux_window_size_enum_type;

typedef struct tagCMUX_UART_WR_ASYNC_INFO
{
    char* pBuffer;
    unsigned int u32Size;
    void* pDrvPriv;
}cmux_hsuart_wr_async_info;

/** Range of size in bytes in Information field as per 3GPP 27.010 */
#define CMUX_MIN_FRAME_N1                      31
#define CMUX_MAX_FRAME_N1                      1540
/** Range of number of re-transmissions as per 3GPP 27.010*/
#define CMUX_MAX_FRAME_N2                      10
/** The values of T1, T2 should extend to ms*/
#define T1_T2_FACTOR     (10)

/** Default size in bytes in Information field per 3GPP 27.010, basic mode */
#define CMUX_BASIC_MODE_DEFAULT_FRAME_N1       31
/** Default size in bytes in Information field per 3GPP 27.010, advanced mode */
#define CMUX_ADVANCED_MODE_DEFAULT_FRAME_N1    63
/** Max Number of re-tranmission as per 3GPP 27.010 */
#define CMUX_DEFAULT_MAX_TX_N2                 3
/** Default value of acknowledgement timer in ms as per 3GPP 27.010 */
#define CMUX_DEFAULT_CMD_TIMER_T1              100
/** Default value of DLCI0 response timer in ms as per 3GPP 27.010 */
#define CMUX_DEFAULT_DLCI0_TIMER_T2            900
/** Default value of response timer as per 3GPP 27.010 */
#define CMUX_DEFAULT_TIMER_T3                  10
/** Default value of window size as per 3GPP 27.010 */
#define CMUX_DEFAULT_WINDOW_SIZE_K             2

/******************************************************************************
Function:       SetCmuxClosePort
Description:    ����gCmuxClosePortȫ�ֱ���ֵ(����balongƽ̨)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void SetCmuxClosePort(unsigned char value);

/******************************************************************************
Function:       CMUX_InitPort
Description:    CMUXģ���ʼ��������CMUXģʽ(����balongƽ̨)
Input:          *CmuxInfo :AT+CMUX��������Я���Ĳ���
Output:         None
Return:         None
Others:         None
******************************************************************************/
int CMUX_InitPort(cmux_info_type *CmuxInfo);

/******************************************************************************
Function:       CMUX_ClosePort
Description:    �˳�CMUXģʽ���ָ�ATģʽ(����balongƽ̨)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_ClosePort(void);

/******************************************************************************
Function:       CMUX_HSUART_UlDataReadCB
Description:    ��ȡHSUART����������buffer���͵�CMUX lib����(����balongƽ̨)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_HSUART_UlDataReadCB(void);

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
);

/******************************************************************************
Function:       CMUX_HSUART_FreeDlDataBuff
Description:    ���������ڴ��ͷ�(����balongƽ̨)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_HSUART_FreeDlDataBuff(char *pucData);

/******************************************************************************
Function:       CMUX_HSUART_FreeUlDataBuff
Description:    ���������ڴ��ͷ�(����balongƽ̨)
Input:          *pucData :��Ҫ�ͷŵ�����
Output:         None
Return:         Pass :�ɹ�
                Fail :ʧ��
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_FreeUlDataBuff(char *pucData);

/******************************************************************************
Function:       CMUX_OpenDevice
Description:    ��CMUX��HSUART�豸,ע��ص�����(����balongƽ̨)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_OpenDevice(void);

/******************************************************************************
Function:       CMUX_CloseDevice
Description:    �ر�CMUX��HSUART�Ľӿ�(����balongƽ̨)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_CloseDevice(void);

/******************************************************************************
Function:       CMUX_HSUART_WriteDataAsync
Description:    ��cmux�����ݷ��͸���������(����balongƽ̨)
Input:          None
Output:         None
Return:         Pass :�ɹ�
                Fail :ʧ��
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_WriteDataAsync(char *pucData);

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
    unsigned short                          usLen);

/******************************************************************************
Function:       CMUX_HSUART_MscReadCB
Description:    HSUARTӲ���ܽ��źŶ��ص�
Input:          pstDceMsc - �ܽ��ź���Ϣ
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_HSUART_MscReadCB(void *pParam);

/******************************************************************************
Function:       CMUX_timer_expiry_cb
Description:    AT+CMUX��ʱ��Ļص�����(����balongƽ̨)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_at_timer_expiry_cb(struct work_struct *w);

/******************************************************************************
Function:       CMUX_HSUART_setPowerStatus
Description:    CMUX lib����HSUART˯��ͶƱ�ӿ�(����balongƽ̨)
Input:          None
Output:         None
Return:         Pass :�ɹ�
                Fail :ʧ��
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_setPowerStatus(void);

/******************************************************************************
Function:       CMUX_BaudToNum
Description:   �жϲ������Ƿ���CMUX֧�ַ�Χ
Input:          enBaudRate
Output:         unsigned int ���ز���������Ӧ��CMUX NUMֵ,�Ƿ��Ʒ���0
Return:         0,1,2,3,4,5,6,7,8
Others:         None
******************************************************************************/
unsigned int CMUX_BaudToNum(unsigned long enBaudRate);

#endif /* MBB_HSUART_CMUX */
#endif /* CMUX_DRV_IFC_H */
