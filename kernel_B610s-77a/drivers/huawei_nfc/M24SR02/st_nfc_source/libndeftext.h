/**
  ******************************************************************************
  * @file    lib_NDEF_Text.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to manage Text NDEF file.
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
#ifndef __LIB_NDEF_TEXT_H
#define __LIB_NDEF_TEXT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "libndef.h"

uint16_t NDEF_WriteText(char *text);
#if (FEATURE_ON == MBB_NFC)
uint16_t NDEF_ReadText(sRecordInfo *pRecordStruct, char *text);
#endif
#ifdef __cplusplus
}
#endif
     
#endif /* __LIB_NDEF_TEXT_H */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
