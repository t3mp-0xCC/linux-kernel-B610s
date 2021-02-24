/**
  ******************************************************************************
  * @file    lib_NDEF_WifiToken.h
  * @author  IPD System Lab Team
  * @version V1.0.0
  * @date    7-October-2015
  * @brief   This file help to manage NDEF file that represent Wifi Token
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
#ifndef __LIB_NDEF_WIFIT_H
#define __LIB_NDEF_WIFIT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "libndef.h"
     
/** @addtogroup NFC_libraries
  * @{
  */


/** @addtogroup lib_NDEF
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/**
  * @brief  WifiToken structure, to store Network SSID,Authentication Type,
   Encryption Type and Network Key
  */         
#if (FEATURE_ON == MBB_NFC)
#define NFC_MAX_SSID_SIZE        32
#define NFC_MAX_PWD_SIZE        64
#define NFC_MAX_SEC_SIZE          8
#define NFC_MAX_ENCRY_SIZE     64
typedef struct 
{
    char NetworkSSID[NFC_MAX_SSID_SIZE + 1];
    char AuthenticationType[NFC_MAX_SEC_SIZE + 1];
    char EncryptionType[NFC_MAX_ENCRY_SIZE + 1];
    char NetworkKey[NFC_MAX_PWD_SIZE + 1];
}sWifiTokenInfo;
#else
typedef struct 
{
    char NetworkSSID[32];
    char AuthenticationType[6];
    char EncryptionType[6];
    char NetworkKey[32];
}sWifiTokenInfo;
#endif
         
uint16_t NDEF_ReadWifiToken(sRecordInfo *pRecordStruct, sWifiTokenInfo *pWifiTokenStruct);
uint16_t NDEF_WriteWifiToken( sWifiTokenInfo *pWifiTokenStruct );
#if (FEATURE_ON == MBB_NFC)
uint16_t NDEF_PackageWifiToken ( sWifiTokenInfo *pWifiTokenStruct, uint8_t *pData,uint16_t *pDataSize);
#endif
/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif
     
#endif /* __LIB_NDEF_SMS_H */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
