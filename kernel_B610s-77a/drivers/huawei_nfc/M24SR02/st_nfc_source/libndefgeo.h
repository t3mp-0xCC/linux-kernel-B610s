/**
  ******************************************************************************
  * @file    lib_NDEF_Geo.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to manage Geolocation NDEF file.
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
#ifndef __LIB_NDEF_GEO_H
#define __LIB_NDEF_GEO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "libndef.h"
     
     
typedef struct 
{
    char Latitude[20];
    char Longitude[20];
    char Information[100];
}sGeoInfo;

#if (FEATURE_OFF == MBB_NFC)
u16 NDEF_ReadGeo(sRecordInfo *pRecordStruct, sGeoInfo *pGeoStruct){return ERROR;}
u16 NDEF_WriteGeo( sGeoInfo *pGeoStruct ){return ERROR;}
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIB_NDEF_GEO_H */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
