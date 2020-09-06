/**
  ******************************************************************************
  * @file    lib_NDEF_URI.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to manage URI NDEF file.
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
#ifndef __LIB_NDEF_URI_H
#define __LIB_NDEF_URI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "libndef.h"
#if (FEATURE_OFF == MBB_NFC)
#include "string.h"
#endif

#if (FEATURE_ON == MBB_NFC)
#define NFC_URL_PROTOCOL_MAX_SIZE 32
#define NFC_URL_MESSAGE_MAX_SIZE 64
#define NFC_URL_INFO_MAX_SIZE 32
typedef struct 
{
    char protocol[NFC_URL_PROTOCOL_MAX_SIZE + 1];
    char URI_Message[NFC_URL_MESSAGE_MAX_SIZE + 1];
    char Information[NFC_URL_INFO_MAX_SIZE + 1];
}sURI_Info;
#else
typedef struct 
{
    char protocol[80];
    char URI_Message[400];
    char Information[400];
}sURI_Info;
#endif
u16 NDEF_ReadURI(sRecordInfo *pRecordStruct, sURI_Info *pURI);
u16 NDEF_WriteURI(sURI_Info *pURI);
#if (FEATURE_ON == MBB_NFC)
u16 NDEF_PackageWriteURI ( sURI_Info *pURI, u8* pData,u16 *pDataSize);
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIB_NDEF_URI_H */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
