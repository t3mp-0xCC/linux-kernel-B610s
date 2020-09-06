/**
  ******************************************************************************
  * @file    lib_M24SR.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to manage Data inside M24SR.
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
#ifndef __LIB_M24SR_H
#define __LIB_M24SR_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <mbb_config.h>

/* Includes ------------------------------------------------------------------*/
#if (FEATURE_ON == MBB_NFC)
#include <linux/kernel.h>
#endif
#include "drv_m24sr.h"
     
/* ---------------------- M24SR properties -----------------------------------*/
#define M24SR02_NDEF_MAX_SIZE                               0x100
#define M24SR04_NDEF_MAX_SIZE                               0x200
#define M24SR16_NDEF_MAX_SIZE                               0x800
#define M24SR64_NDEF_MAX_SIZE                               0x2000   


#define ASK_FOR_SESSION                                         0x0000
#define TAKE_SESSION                                                0xFFFF   
     
/* M24SR buffer size is 0xF6 can be retrieve dynamicaly in CC file */
#define M24SR_READ_MAX_NBBYTE                               0xF6 
#define M24SR_WRITE_MAX_NBBYTE                          0xF6


#define NDEF_SESSION_CLOSED                                 0xDEADBEEF
     
#ifndef errorchk
#define errorchk(fCall) if (status = (fCall), status != M24SR_ACTION_COMPLETED) \
    {goto Error;} else
#endif

u16 M24SR_Initialization (u8* pCCBuffer, u8 size );
    
u16 M24SR_GetNDEFFileId ( u16 *NDEF_fileID );
u16 M24SR_OpenNDEFSession ( u16 NDEF_fileID, u16 Priority );
u16 M24SR_ReadData ( u16 Offset , u16 DataSize , u8* pData);
u16 M24SR_ForceReadData ( u16 Offset , u16 DataSize , u8* pData);
u16 M24SR_WriteData ( u16 Offset , u16 DataSize , u8* pData);
u16 M24SR_CloseNDEFSession ( u16 NDEF_fileID ); 

u16 M24SR_EnableReadPassword( u8* pCurrentWritePassword, u8* pNewPassword); 
u16 M24SR_DisableReadPassword( u8* pCurrentWritePassword ); 
u16 M24SR_EnableWritePassword( u8* pCurrentWritePassword, u8* pNewPassword);    
u16 M24SR_DisableWritePassword( u8* pCurrentWritePassword );
u16 M24SR_DisableAllPassword( u8* pSuperUserPassword);

u16 M24SR_EnableReadOnly( u8* pCurrentWritePassword);   
u16 M24SR_DisableReadOnly( u8* pCurrentWritePassword);  
u16 M24SR_EnableWriteOnly( u8* pCurrentWritePassword);  
u16 M24SR_DisableWriteOnly( u8* pCurrentWritePassword);

u16 M24SR_ManageGPO( uc8 GPO_config, uc8 mode); 
#if (FEATURE_ON == MBB_NFC)
void M24SR_GetNewPasswd( uc16 uPwdId, uint8_t *pPasswd);
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIB_M24SR_H */
 
/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
