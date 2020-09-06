/**
  ******************************************************************************
  * @file    drvi2cm24sr.h
  * @author  MMY Application Team
  * @version V4.5.0
  * @date    08-May-2013
  * @brief   This file provides a set of functions needed to manage the I2C of
                         the M24SR device.
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_I2CM24SR_H
#define __DRV_I2CM24SR_H

#include <mbb_config.h>

#if (FEATURE_ON == MBB_NFC)
#include <linux/kernel.h>
#endif
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

     
/* Flags ---------------------------------------------------------------------*/
#define I2C_GPO_SYNCHRO_ALLOWED /* allow tu use GPO as I2C synchronization */ 
     
/* macro function ------------------------------------------------------------*/     
 
#ifndef errchk
#define errchk(fCall) if (status = (fCall), status != M24SR_STATUS_SUCCESS) \
    {goto Error;} else
#endif

/*!< constant Unsigned integer types  */
typedef const unsigned char     uc8;
typedef const unsigned short    uc16;
//typedef const unsigned long     uc32;

    
/* M24SR GPIO mapping -------------------------------------------------------------------------*/
#define M24SR_SDA_PIN                                                       GPIO_Pin_11
#define M24SR_SDA_PIN_SOURCE                                            GPIO_PinSource11
#define M24SR_SDA_PIN_PORT                                              GPIOB

#define M24SR_SCL_PIN                                                       GPIO_Pin_10
#define M24SR_SCL_PIN_SOURCE                                            GPIO_PinSource10
#define M24SR_SCL_PIN_PORT                                              GPIOB
    
#define M24SR_GPO_PIN                                                       GPIO_Pin_6
#define M24SR_GPO_PIN_PORT                                              GPIOA

#define M24SR_RFDIS_PIN                                                     GPIO_Pin_7
#define M24SR_RFDIS_PIN_PORT                                            GPIOA

#if defined STM32F40_41xxx
#define M24SR_GPIO_BLOCK_CLK                                            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB)
#else
#define M24SR_GPIO_BLOCK_CLK                                            (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB)
#endif

/*  I2C config  ------------------------------------------------------------------------------*/
#define M24SR_I2C                                          I2C2
#define M24SR_I2C_BLOCK_CLK                                              RCC_APB1Periph_I2C2
#define M24SR_I2C_CLOCKSPEED                                             400000

#define M24SR_I2C_TIMEOUT       (0x3FFFF) /*!< I2C Time out */
#define M24SR_I2C_POLLING       0x0FFF /* Nb attempt of the polling */
#define M24SR_ADDR              0xAC   /*!< M24SR address */

    
/* error code ---------------------------------------------------------------------------------*/
#define M24SR_ERRORCODE_FILEOVERFLOW                            0x6280
#define M24SR_ERRORCODE_ENDOFFILE                                   0x6282
#define M24SR_ERRORCODE_PASSWORDREQUIRED                    0x63C0
#define M24SR_ERRORCODE_PASSWORDINCORRECT2RETRY     0x63C2
#define M24SR_ERRORCODE_PASSWORDINCORRECT1RETRY     0x63C1
#define M24SR_ERRORCODE_RFSESSIONKILLED                     0x6500
#define M24SR_ERRORCODE_UNSUCCESSFULUPDATING            0x6581
#define M24SR_ERRORCODE_WRONGHLENGTH                            0x6700
#define M24SR_ERRORCODE_COMMANDINCORRECT                    0x6981
#define M24SR_ERRORCODE_SECURITYSTATUS                      0x6982
#define M24SR_ERRORCODE_REFERENCEDATANOTUSABLE      0x6984
#define M24SR_ERRORCODE_INCORRECTPARAMETER              0x6A80
#define M24SR_ERRORCODE_FILENOTFOUND                            0x6A82
#define M24SR_ERRORCODE_FILEOVERFLOWLC                      0x6A84
#define M24SR_ERRORCODE_INCORRECTP1P2                           0x6A86
#define M24SR_ERRORCODE_INSNOTSUPPORTED                     0x6D00
#define M24SR_ERRORCODE_CLASSNOTSUPPORTED                   0x6E00
#define M24SR_ERRORCODE_DAFAULT                                     0x6F00
    
/* Status and error code -----------------------------------------------------*/     
#define M24SR_STATUS_SUCCESS                                    0x0000
#define M24SR_ERROR_DEFAULT                                     0x0010
#define M24SR_ERROR_I2CTIMEOUT                              0x0011
#define M24SR_ERROR_CRC                                             0x0012
#define M24SR_ERROR_NACK                                            0x0013
#define M24SR_ERROR_PARAMETER                                   0x0014 
#define M24SR_ERROR_NBATEMPT                                    0x0015 
#define M24SR_ERROR_NOACKNOWLEDGE                           0x0016


    
/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will 
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define M24SR_FLAG_TIMEOUT         ((uint16_t)0xF000)
#define M24SR_LONG_TIMEOUT         ((uint16_t)( M24SR_FLAG_TIMEOUT))    
     
/*  mask    ------------------------------------------------------------------------------------*/
typedef enum
{
    M24SR_WAITINGTIME_UNKNOWN = 0,
    M24SR_WAITINGTIME_POLLING,
    M24SR_WAITINGTIME_TIMEOUT,
    M24SR_WAITINGTIME_GPO
}M24SR_WAITINGTIME_MGMT; 

/*  public function --------------------------------------------------------------------------*/

void   M24SR_I2CInit                    ( void );
void     M24SR_SetI2CSynchroMode        ( uc8 mode );
int8_t M24SR_SendI2Ccommand             ( uc8 NbByte , uc8 *pBuffer );
int8_t M24SR_IsAnswerReady              ( void );
int8_t M24SR_PollI2C                    ( void );
int8_t M24SR_ReceiveI2Cresponse         ( uint8_t NbByte , uint8_t *pBuffer );
int8_t M24SR_ReceiveI2Cbuffer           ( uint8_t NbByte , uint8_t *pBuffer );

void M24SR_GPOInt(void);
void M24SR_RFConfig_Hard( uc8 OnOffChoice);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
