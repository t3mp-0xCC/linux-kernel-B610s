/**
  ******************************************************************************
  * @file    lib_TagType4.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to manage TagType4.
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
#ifndef __LIB_TAGTYPE4_H
#define __LIB_TAGTYPE4_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "libndefuri.h"
#include "libndefsms.h"
#include "libndefemail.h"
#include "libndefvcard.h"
#include "libndefgeo.h"
#include "libndefmyapp.h"
#include "libndefaar.h"
#include "libndefwifi.h"
#if (FEATURE_ON == MBB_NFC)
#include "libndeftext.h"
#endif
typedef struct
{
    u16 NumberCCByte;
    u8 Version;
    u16 MaxReadByte;
    u16 MaxWriteByte;
    u8 TField;
    u8 LField;
    u16 FileID;
    u16 NDEFFileMaxSize;
    u8 ReadAccess;
    u8 WriteAccess;
}sCCFileInfo;

typedef enum 
{
    UNKNOWN_TAG = 0,
  VCARD_TAG,
    URI_TAG,
    SMS_TAG,
    GEO_TAG,
    EMAIL_TAG,
    TEXT_TAG,
    BT_TAG,
    WIFI_TAG
} Tag_TypeDef;
#if (FEATURE_ON == MBB_NFC)
typedef struct
{
    sWifiTokenInfo WifiToken;
    sURI_Info URLToken;
}sWifiUrlToken;
#endif
u16 TT4_Init(void);
u16 TT4_ReadNDEF(u8 *pNDEF);
u16 TT4_WriteNDEF(u8 *pNDEF);
u16 TT4_ReadURI(sURI_Info *pURI);
u16 TT4_WriteURI(sURI_Info *pURI);
#if (FEATURE_OFF == MBB_NFC)
u16 TT4_ReadSMS(sSMSInfo *pSMS);
u16 TT4_WriteSMS(sSMSInfo *pSMS);
u16 TT4_ReadEmail(sEmailInfo *pEmailStruct);
u16 TT4_WriteEmail(sEmailInfo *pEmailStruct);
u16 TT4_ReadVcard(sVcardInfo *pVcard);
u16 TT4_WriteVcard(sVcardInfo *pVcard);
u16 TT4_ReadGeo(sGeoInfo *pGeo);
u16 TT4_WriteGeo(sGeoInfo *pGeo);
u16 TT4_ReadMyApp(sMyAppInfo *pMyAppStruct);
u16 TT4_WriteMyApp(sMyAppInfo *pMyAppStruct);
u16 TT4_AddAAR(sAARInfo *pAAR);
#endif
u16 TT4_ReadWifiToken(sWifiTokenInfo *pWifiToken);
u16 TT4_WriteWifiToken(sWifiTokenInfo *pWifiToken);
#if (FEATURE_ON == MBB_NFC) 
u16 TT4_ReadWifiURLToken(sWifiUrlToken *pToken);
u16 TT4_WriteWifiURLToken(sWifiUrlToken *pToken);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __LIB_TAGTYPE4_H */


/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
