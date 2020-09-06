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
 * 简单数据类型定义说明                         *
 *----------------------------------------------*/

#define CMUX_SUCCESS               0
#define CMUX_FAILURE               1

/*AT+CMUX命令超时恢复正常模式时间: 10000 ms*/
#define AT_CMUX_EXPIRY_TIME        9500   //留0.5秒缓冲时间

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
Description:    设置gCmuxClosePort全局变量值(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void SetCmuxClosePort(unsigned char value);

/******************************************************************************
Function:       CMUX_InitPort
Description:    CMUX模块初始化，开启CMUX模式(适配balong平台)
Input:          *CmuxInfo :AT+CMUX设置命令携带的参数
Output:         None
Return:         None
Others:         None
******************************************************************************/
int CMUX_InitPort(cmux_info_type *CmuxInfo);

/******************************************************************************
Function:       CMUX_ClosePort
Description:    退出CMUX模式，恢复AT模式(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_ClosePort(void);

/******************************************************************************
Function:       CMUX_HSUART_UlDataReadCB
Description:    获取HSUART的上行数据buffer，送到CMUX lib处理(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_HSUART_UlDataReadCB(void);

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
);

/******************************************************************************
Function:       CMUX_HSUART_FreeDlDataBuff
Description:    下行数据内存释放(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_HSUART_FreeDlDataBuff(char *pucData);

/******************************************************************************
Function:       CMUX_HSUART_FreeUlDataBuff
Description:    上行数据内存释放(适配balong平台)
Input:          *pucData :需要释放的内容
Output:         None
Return:         Pass :成功
                Fail :失败
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_FreeUlDataBuff(char *pucData);

/******************************************************************************
Function:       CMUX_OpenDevice
Description:    打开CMUX和HSUART设备,注册回调函数(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_OpenDevice(void);

/******************************************************************************
Function:       CMUX_CloseDevice
Description:    关闭CMUX和HSUART的接口(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_CloseDevice(void);

/******************************************************************************
Function:       CMUX_HSUART_WriteDataAsync
Description:    将cmux的数据发送给串口驱动(适配balong平台)
Input:          None
Output:         None
Return:         Pass :成功
                Fail :失败
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_WriteDataAsync(char *pucData);

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
    unsigned short                          usLen);

/******************************************************************************
Function:       CMUX_HSUART_MscReadCB
Description:    HSUART硬件管脚信号读回调
Input:          pstDceMsc - 管脚信号信息
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_HSUART_MscReadCB(void *pParam);

/******************************************************************************
Function:       CMUX_timer_expiry_cb
Description:    AT+CMUX超时后的回调函数(适配balong平台)
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
void CMUX_at_timer_expiry_cb(struct work_struct *w);

/******************************************************************************
Function:       CMUX_HSUART_setPowerStatus
Description:    CMUX lib设置HSUART睡眠投票接口(适配balong平台)
Input:          None
Output:         None
Return:         Pass :成功
                Fail :失败
Others:         None
******************************************************************************/
unsigned int CMUX_HSUART_setPowerStatus(void);

/******************************************************************************
Function:       CMUX_BaudToNum
Description:   判断波特率是否在CMUX支持范围
Input:          enBaudRate
Output:         unsigned int 返回波特率所对应的CMUX NUM值,非法制返回0
Return:         0,1,2,3,4,5,6,7,8
Others:         None
******************************************************************************/
unsigned int CMUX_BaudToNum(unsigned long enBaudRate);

#endif /* MBB_HSUART_CMUX */
#endif /* CMUX_DRV_IFC_H */
