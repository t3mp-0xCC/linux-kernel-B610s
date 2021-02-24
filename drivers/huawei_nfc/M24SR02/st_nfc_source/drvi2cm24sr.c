/**
  ******************************************************************************
  * @file    drv_I2C_M24SR.c
  * @author  MMY Application Team
  * @version V4.5.0
  * @date    08-May-2013
  * @brief   This file provides a set of functions needed to manage the I2C of
    *                  the M24SR device.
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MMY-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
/**********************问题单修改记录******************************************
日    期              修改人         问题单号           修改内容
******************************************************************************/
/* Includes ------------------------------------------------------------------*/

#include <mbb_config.h>

#if (FEATURE_ON == MBB_NFC)
#include <linux/module.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include "drvi2cm24sr.h"
#include <linux/delay.h>

#include "mdrv_nfc.h"
#else
#include <fcntl.h>
#include <stdio.h>
#include <linux/i2c-dev.h>

#include "sttype.h"
#include "drvi2cm24sr.h"
#include "unistd.h"
#include "stdlib.h"

#include "hwconfig.h"
#endif

#if (FEATURE_ON == MBB_NFC)
#define NFC_BUFFER_MAX 256
/*****************************************************************************
 函 数 名  : hw_drv_i2c_read_buf
 功能描述  : NFC芯片I2C读
 输入参数  : u16 us_dev_addr     设备地址 
             u16 us_reg_addr      寄存器地址
             u32 ui_reg_addr_len  寄存器长度
             uint8_t *puc_data    存储数据的buffer
             u32 ui_data_len      读取的数据大小
 输出参数  :  puc_data 存储数据的buffer
 返 回 值  :   M24SR_STATUS_SUCCESS 读取成功
                         M24SR_ERROR_PARAMETER 参数错误
 调用函数  : 
 被调函数  : 
*****************************************************************************/
u32 hw_drv_i2c_read_buf(u16 us_dev_addr,
                           u16 us_reg_addr, u32 ui_reg_addr_len,
                           uint8_t *puc_data, u32 ui_data_len)
{
    u32 ret = 0;
    char temp_buff[NFC_BUFFER_MAX] = {0};
    struct i2c_client *huawei_nfc_client = NULL;

    if(NULL == puc_data)
    {
        printk(KERN_ERR "%s (%d) : Invalid Params !\n",__func__,__LINE__);
        return M24SR_ERROR_PARAMETER;
    }
    
    if(ui_data_len > NFC_BUFFER_MAX || 0 == ui_data_len)
    {
        printk(KERN_ERR "%s (%d) :  hw_drv_i2c_read_buf data beyond the max limit!\n",
                    __func__,__LINE__);
        return M24SR_ERROR_PARAMETER;
    }
    huawei_nfc_client = huawei_nfc_get_handle();
    if(NULL == huawei_nfc_client)
    {
        printk(KERN_ERR "%s (%d) :  huawei_nfc_client is NULL! \n",__func__,__LINE__);
        return M24SR_ERROR_PARAMETER;
    }
    ret = i2c_master_recv(huawei_nfc_client,temp_buff,ui_data_len);

    if (ui_data_len == ret) 
    {
        memcpy(puc_data,temp_buff,ui_data_len); 
        return M24SR_STATUS_SUCCESS;
    }
    else 
    {
        printk(KERN_ERR "%s (%d) :  hw_drv_i2c_read_buf failed (%d)!\n",
                __func__,__LINE__,ret);
        return ret;
    }
}
/*****************************************************************************
 函 数 名  : hw_drv_i2c_write_buf
 功能描述  : NFC芯片I2C写
 输入参数  : u16 us_dev_addr     设备地址 
             u16 us_reg_addr      寄存器地址
             u32 ui_reg_addr_len  寄存器长度
             uint8_t *puc_data    写入数据的buffer
             u32 ui_data_len      写入的数据大小
 输出参数  :  NA
 返 回 值  :   M24SR_STATUS_SUCCESS 写成功
                         M24SR_ERROR_PARAMETER 参数错误
 调用函数  : 
 被调函数  : 
*****************************************************************************/
int hw_drv_i2c_write_buf(u16 us_dev_addr,
                           u16 us_reg_addr, u32 ui_reg_addr_len,
                           uc8* puc_data, u32 ui_data_len)
{
    int ret = 0;
    char temp_buff[NFC_BUFFER_MAX] = {0};
    struct i2c_client *huawei_nfc_client = NULL;

    if(NULL == puc_data)
    {
        printk(KERN_ERR "%s (%d) : Invalid Params !\n",__func__,__LINE__);
        return M24SR_ERROR_PARAMETER;
    }
    
    if(ui_data_len > NFC_BUFFER_MAX || 0 == ui_data_len)
    {
        printk(KERN_ERR "%s (%d) :  hw_drv_i2c_write_buf data beyond the max limit!\n",
                __func__,__LINE__);
        return M24SR_ERROR_PARAMETER;
    }
    memcpy(temp_buff,puc_data,ui_data_len);

    huawei_nfc_client = huawei_nfc_get_handle();
    if(NULL == huawei_nfc_client)
    {
        printk(KERN_ERR "%s (%d) :  huawei_nfc_client is NULL! \n",__func__,__LINE__);
        return M24SR_ERROR_PARAMETER;
    }

    ret = i2c_master_send(huawei_nfc_client,temp_buff,ui_data_len);

    if (ui_data_len == ret) {
        return M24SR_STATUS_SUCCESS;
    }
    else 
    {
        printk(KERN_ERR "%s (%d) :  hw_drv_i2c_write_buf failed (%d)!\n",
                __func__,__LINE__,ret);
        return ret;
    }
}

#define ST_I2C_ADDR 0x56

#endif

/** @addtogroup M24SR_Driver
  * @{
  * @brief  <b>This folder contains the driver layer of M24SR family (M24SR64, M24SR16, M24SR04, M24SR02)</b> 
  */

/** @addtogroup M24SR_I2C
 *  @{
 *  @brief  This file includes the I2C driver used by M24SR family to communicate with the MCU.  
 */

/** @defgroup M24SR_I2C_Private_Functions
 *  @{
 */
#if (FEATURE_OFF == MBB_NFC)
static uint8_t                       uSynchroMode = M24SR_WAITINGTIME_POLLING;
static int filehandle;
#endif

/**
  * @brief  This functions polls the I2C interface
  * @retval M24SR_STATUS_SUCCESS : the function is succesful
    * @retval M24SR_ERROR_I2CTIMEOUT : The I2C timeout occured. 
    * @retval M24SR_ERROR_NACK : the M24SR doesn't acknowledge
    * @retval M24SR_ERROR_NBATEMPT  
  */

int8_t M24SR_PollI2C ( void )
{
#if (FEATURE_OFF == MBB_NFC)
    uint32_t  M24SR_Timeout = 0;
    uint16_t  NbAtempt = M24SR_I2C_POLLING,uSR1register = 0x0000;
#endif
    // TO BE DONE
    return M24SR_STATUS_SUCCESS;
}

/**
  * @}
  */


/** @defgroup M24SR_I2C_Public_Functions
  * @{
  */
#if (FEATURE_ON == MBB_NFC)
void M24SR_I2CDeInit(void)
{
}
#else
void M24SR_I2CDeInit(void)
{
    if(close(filehandle) < 0)
    {
        printf(" Error Can't close /dev/i2c-x%\n");
        exit(1);
    }
}
#endif
/**
  * @brief  This function initializes the M24SR_I2C interface
    * @retval None  
  */
#if (FEATURE_ON == MBB_NFC)
void M24SR_I2CInit ( void )
{
}
#else
void M24SR_I2CInit ( void )
{

    static char filename[20];
    static int adapter_nr = 1; /* probably dynamically determined - user can determine on which i2c Device ST M24sr is plugged*/
    int m24sraddr = 0b1010110;

    snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
    filehandle = open(filename,O_RDWR);
    if (filehandle < 0) {
        printf("Error : Can't open /dev/i2c-%d\n",adapter_nr);
        /* ERROR HANDLING; you can check errno to see what went wrong */
        exit(1);
        }

    // The I2C address of the ADC
    if (ioctl(filehandle, I2C_SLAVE, m24sraddr) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        M24SR_I2CDeInit();
    }

//close:
//  if(close(filehandle) < 0)
//  {
//      printf(" Error Can't close /dev/i2c-%d\n",adapter_nr);
//      exit(1);
//  }
}
#endif
/**
  * @brief  this functions configure I2C synchronization mode
    * @param  mode : 
  * @retval None
  */
#if (FEATURE_ON == MBB_NFC)
void M24SR_SetI2CSynchroMode( uc8 mode )
{

}
#else
void M24SR_SetI2CSynchroMode( uc8 mode )
{
#ifdef I2C_GPO_SYNCHRO_ALLOWED
    uSynchroMode = mode;
#else
    if( mode == M24SR_WAITINGTIME_GPO)
    {
        uSynchroMode = M24SR_WAITINGTIME_POLLING;
    }
    else
    {
        uSynchroMode = mode;
    }
#endif /*  I2C_GPO_SYNCHRO_ALLOWED */
}

#endif
/**
  * @brief  This functions sends the command buffer 
    * @param  NbByte : Number of byte to send
  * @param  pBuffer : pointer to the buffer to send to the M24SR
  * @retval M24SR_STATUS_SUCCESS : the function is succesful
    * @retval M24SR_ERROR_I2CTIMEOUT : The I2C timeout occured. 
  */
#if (FEATURE_ON == MBB_NFC)
int8_t M24SR_SendI2Ccommand ( uc8 NbByte , uc8 *pBuffer )
{
    int  ui_ret            = 0;

    if(NULL == pBuffer)
    {
        printk(KERN_ERR "%s (%d) : Invalid Params !\n",__func__,__LINE__);
        return M24SR_ERROR_PARAMETER;
    }

    /*调用平台接口写数据*/    
    ui_ret = hw_drv_i2c_write_buf(ST_I2C_ADDR,
                                  0, 0,
                                  pBuffer, NbByte);

    /*M24SR 读写速率为106Kbps,WIFI Record的大小为111Bytes.*/ 
    if(NbByte < 100)
    {
        mdelay(10);
    }
    else
    {
        mdelay(100);/*延时100ms*/
    }
    if (M24SR_STATUS_SUCCESS != ui_ret)
    {
        printk(KERN_ERR "%s (%d) :  M24SR_SendI2Ccommand failed!\n",__func__,__LINE__);
        return ui_ret;
    } 
    else
    {
        return M24SR_STATUS_SUCCESS;
    }
}
#else
int8_t M24SR_SendI2Ccommand ( uc8 NbByte , uc8 *pBuffer )
{
    ssize_t byteSent = 0;

    while ((byteSent = write(filehandle, pBuffer, NbByte)) == -1)
    {
        usleep(10000); // Implement Fake polling - wait 10 ms
    }
    if( byteSent != NbByte)
    {
        return M24SR_ERROR_DEFAULT;
    }

    return M24SR_STATUS_SUCCESS;
}
#endif

/**
  * @brief  This functions reads a response of the M24SR device
  * @param  NbByte : Number of byte to read (shall be >= 5)
  * @param  pBuffer : Pointer to the buffer to send to the M24SR
  * @retval M24SR_STATUS_SUCCESS : The function is succesful
  * @retval M24SR_ERROR_I2CTIMEOUT : The I2C timeout occured.
  */
#if (FEATURE_ON == MBB_NFC)
int8_t M24SR_ReceiveI2Cresponse ( uint8_t NbByte , uint8_t *pBuffer )
{
    unsigned int  i            = 0;

    if(NULL == pBuffer)
    {
        printk(KERN_ERR "%s (%d) : Invalid Params !\n",__func__,__LINE__);
        return M24SR_ERROR_PARAMETER;
    }
    while (hw_drv_i2c_read_buf(ST_I2C_ADDR, 0, 0, pBuffer, NbByte))
    {
        mdelay(10);
        i++;

        if (i >= 10)
        {
            break;
        }
    }

    return M24SR_STATUS_SUCCESS;
}
#else
int8_t M24SR_ReceiveI2Cresponse ( uint8_t NbByte , uint8_t *pBuffer )
{

    ssize_t byteRead = 0;

    while ((byteRead=read(filehandle, pBuffer, NbByte))==-1)
    {
        usleep(10000); // Implement Fake polling - wait 10 ms
    }

    if( byteRead != NbByte)
    {
        return M24SR_ERROR_DEFAULT;
    }

    return M24SR_STATUS_SUCCESS;
}
#endif
/**
  * @brief  This function reads a number of byte through the I2C interface
  * @param  NbByte : Number of byte to read
  * @param  pBuffer : pointer to the buffer to send to the M24SR
  * @retval M24SR_STATUS_SUCCESS    : the function is succesful
  * @retval M24SR_ERROR_I2CTIMEOUT  : The I2C timeout occured.
  * @retval M24SR_ERROR_DEFAULT     : The I2C timeout occured.
  */
#if (FEATURE_ON == MBB_NFC)
int8_t M24SR_ReceiveI2Cbuffer ( uint8_t NbByte , uint8_t *pBuffer )
{
    unsigned int  ui_ret            = 0;

    if(NULL == pBuffer)
    {
        printk(KERN_ERR "%s (%d) : Invalid Params !\n",__func__,__LINE__);
        return M24SR_ERROR_PARAMETER;
    }
    
    ui_ret = hw_drv_i2c_read_buf(ST_I2C_ADDR,
                                 0, 0,
                                 pBuffer, NbByte);
/*M24SR 读写速率为106Kbps,WIFI Record的大小为111Bytes.*/
    mdelay(100);//延时100ms
    if (M24SR_STATUS_SUCCESS != ui_ret)
    {
        return ui_ret;
    } 
    else 
    {
        return M24SR_STATUS_SUCCESS;
    }
}
#else
int8_t M24SR_ReceiveI2Cbuffer ( uint8_t NbByte , uint8_t *pBuffer )
{
    if( read(filehandle, pBuffer, NbByte) != NbByte)
    {
        return M24SR_ERROR_DEFAULT;
    }
    return M24SR_STATUS_SUCCESS;
}
#endif
/**
  * @brief  This functions returns M24SR_STATUS_SUCCESS when a response is ready
  * @retval M24SR_STATUS_SUCCESS : a response of the M24LR is ready
  * @retval M24SR_ERROR_DEFAULT : the response of the M24LR is not ready
  */
#if (FEATURE_ON == MBB_NFC)
int8_t M24SR_IsAnswerReady ( void )
{
    return M24SR_STATUS_SUCCESS;
}
#else
int8_t M24SR_IsAnswerReady ( void )
{
    int8_t status;
    uint32_t retry = 0x3FFFF;
    uint8_t stable = 0;

    // GPO Event Management
    int fd;
    fd_set fds;
    char buffer[2];
    struct timeval  timeout;
    timeout.tv_usec = 80000;

    
        switch (uSynchroMode)
        {
            case M24SR_WAITINGTIME_POLLING :
                /* Nothing to do on Linux OS
                 * Polling is done on send I2C command
                 */
                return M24SR_STATUS_SUCCESS;

            case M24SR_WAITINGTIME_TIMEOUT :
                /* Deprecated Solution on system which can not get the satus of M24SR */
                // M24SR FWI=5 => (256*16/fc)*2^5=9.6ms but M24SR ask for extended time to program up to 246Bytes.
                usleep (80000);
                return M24SR_STATUS_SUCCESS;

            case M24SR_WAITINGTIME_GPO :
            {
                /* Here we check if I2C answer is ready, so if GPO of M24SR is low */
                /* Check the GPO state - if it has been configured accordingly */
                if ((fd= open(GPO_PIN,O_RDONLY)) < 0) {
                    exit(EXIT_FAILURE);
                }

                FD_ZERO(&fds);
                FD_SET(fd,&fds);
                if(select(fd+1,NULL,NULL,&fds,&timeout)<0){
                    perror("select");
                    break;
                }
                lseek(fd,0,SEEK_SET);
                // read GPO value - must be 0
                if (read(fd,& buffer,2) != 2)
                {
                    perror("read");
                    break;
                }
                // read GPO value - must be 0
                if (read(fd,& buffer,2) != 2)
                {
                    perror("read");
                    break;
                }
                // remove CR
                buffer[1]='/0';
                if(buffer[0]=='0') // GPO ACK received
                {
                    close(fd);
                    return M24SR_STATUS_SUCCESS;
                }
            }
                close(fd);
                return M24SR_ERROR_DEFAULT;
                break;
            default :
                return M24SR_ERROR_DEFAULT;
        }

        return M24SR_ERROR_DEFAULT;
}
#endif//#if (FEATURE_ON == MBB_NFC)
/**
  * @brief  This function enable or disable RF communication
    * @param    OnOffChoice: GPO configuration to set
  * @retval Status (SW1&SW2) : Status of the operation to complete.
  */
#if (FEATURE_ON == MBB_NFC)
void M24SR_RFConfig_Hard( uc8 OnOffChoice)
{

}
#else
void M24SR_RFConfig_Hard( uc8 OnOffChoice)
{
    int fd;
    if ((fd= open(RF_DISABLE_PIN,O_RDWR)) < 0) {
        exit(EXIT_FAILURE);
    }
    /* Disable RF */
    if ( OnOffChoice != 0 )
    {   
        write(fd,"1",2);
    }
    else
    {   
        write(fd,"0",2);
    }
    close(fd);
}
#endif
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/

