/**
  ******************************************************************************
  * @file    lib_NDEF_MyApp.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file illustrate how to use M24SR with a proprietary protocol.
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
#ifndef __LIB_NDEF_MYAPP_H
#define __LIB_NDEF_MYAPP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "libndef.h"
     
#define BLINK_CONFIG                                        "<BLINK_CONFIG>"
#define BLINK_CONFIG_STRING_SIZE            14  
#define BLINK_CONFIG_END                                "<\\BLINK_CONFIG>"
#define BLINK_CONFIG_END_STRING_SIZE    15
     
#define LINE_DESCR                                          "<LINE_DESCR>"
#define LINE_DESCR_STRING_SIZE                  12
#define LINE_DESCR_END                                  "<\\LINE_DESCR>"
#define LINE_DESCR_END_STRING_SIZE          13
     
#define BLINKING_NONE                                       0x00
#define BLINKING_SLOW                                       0x01
#define BLINKING_MEDIUM                                 0x02
#define BLINKING_FAST                                       0x03
     

typedef struct 
{
    u8 LedConf[4];
    u8 Speed;
}sLedBlinkConfig;

typedef struct 
{
    u8 LineNb;
    u16 BackGroundColor;
    u16 FontColor;
    char String[20];
}sLineConfig;

typedef struct 
{
    sLedBlinkConfig LedBlinkConf;
    sLineConfig LineX[8];
}sMyAppInfo;

#if (FEATURE_OFF == MBB_NFC)
u16 NDEF_ReadMyApp(sRecordInfo *pRecordStruct, sMyAppInfo *pMyAppStruct){return ERROR;}
u16 NDEF_WriteMyApp(sMyAppInfo *pMyAppStruct){return ERROR;}
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIB_NDEF_MYAPP_H */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
