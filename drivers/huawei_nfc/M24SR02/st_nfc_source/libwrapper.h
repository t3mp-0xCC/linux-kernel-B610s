/**
  ******************************************************************************
  * @file    lib_wrapper.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to have upper layer independent from HW
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************  
  */
/**********************问题单修改记录******************************************
日    期              修改人         问题单号           修改内容
******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LIB_WRAPPER_H
#define __LIB_WRAPPER_H

/* Includes ------------------------------------------------------------------*/
#include "libm24sr.h"


/* The maximum size of a NDEF will be 64kBits with M24SR64 */
/* if smaller memory used update this define to save space */
#define NDEF_MAX_SIZE                               M24SR02_NDEF_MAX_SIZE

#define NDEF_ACTION_COMPLETED                       M24SR_ACTION_COMPLETED


/* Wrapper to have upper layer independent from HW */
#define TagT4Init                                       M24SR_Initialization

#define GetNDEFFileId                               M24SR_GetNDEFFileId
#define OpenNDEFSession                         M24SR_OpenNDEFSession   
#define CloseNDEFSession                        M24SR_CloseNDEFSession

#define ReadData                                        M24SR_ReadData  
#define ForceReadData                               M24SR_ForceReadData
#define WriteData                                       M24SR_WriteData                         
        
#define EnableReadPassword                  M24SR_EnableReadPassword
#define DisableReadPassword             M24SR_DisableReadPassword
#define EnableWritePassword                 M24SR_EnableWritePassword
#define DisableWritePassword                M24SR_DisableWritePassword
#define DisableAllPassword                  M24SR_DisableAllPassword

#define EnableReadOnly                          M24SR_EnableReadOnly
#define DisableReadOnly                         M24SR_DisableReadOnly
#define EnableWriteOnly                         M24SR_EnableWriteOnly
#define DisableWriteOnly                        M24SR_DisableWriteOnly
#define GPO_Config                                  M24SR_ManageGPO

#endif /* __LIB_WRAPPER_H */


/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
