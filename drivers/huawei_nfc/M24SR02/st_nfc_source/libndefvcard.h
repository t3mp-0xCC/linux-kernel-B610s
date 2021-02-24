/**
  ******************************************************************************
  * @file    lib_NDEF_Vcard.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to manage Vcard NDEF file.
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
#ifndef __LIB_NDEF_VCARD_H
#define __LIB_NDEF_VCARD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "libndef.h"

     
#define VCARD_VERSION_2_1                         "2.1" 
#define VCARD_VERSION_2_1_SIZE              3 

#define VCARD_VERSION_3_0                       "3.0"   
#define VCARD_VERSION_3_0_SIZE              3    
     

#define BEGIN                                                   "BEGIN:"    
#define VCARD                                                   "VCARD"  
#define VERSION                                             "VERSION:"
#define FIRSTNAME                                           "FN:"
#define HOME_TEL                                            "TEL;HOME:"  
#define WORK_TEL                                            "TEL;WORK:"  
#define CELL_TEL                                            "TEL;CELL:"  
#define HOME_EMAIL                                      "EMAIL;HOME:"
#define WORK_EMAIL                                      "EMAIL;WORK:"
#define HOME_ADDRESS                                    "ADR;HOME:"
#define WORK_ADDRESS                                    "ADR;WORK:"
#define TITLE                                                   "TITLE:"
#define ORG                                                     "ORG:"
#define END                                                     "END:"
#define JPEG                                                    "JPEG"  
     
#define LIMIT                                       "\r\n"

#define BEGIN_STRING_SIZE                           6    
#define VCARD_STRING_SIZE                           5
#define VERSION_STRING_SIZE                     8
#define FIRSTNAME_STRING_SIZE                   3
#define HOME_TEL_STRING_SIZE                    9    
#define WORK_TEL_STRING_SIZE                    9    
#define CELL_TEL_STRING_SIZE                    9    
#define HOME_EMAIL_STRING_SIZE              11
#define WORK_EMAIL_STRING_SIZE              11
#define HOME_ADDRESS_STRING_SIZE            9
#define WORK_ADDRESS_STRING_SIZE            9
#define TITLE_STRING_SIZE                           6
#define ORG_STRING_SIZE                             4
#define END_STRING_SIZE                             4
#define JPEG_STRING_SIZE                            4   
     
#define LIMIT_STRING_SIZE                           2
     
typedef struct 
{
    char Version [10];
    char FirstName[80];
    char Title[80];
    char Org[80];
    char HomeAddress[80];
    char WorkAddress[80];
    char HomeTel[40];
    char WorkTel[40];
    char CellTel[40];
    char HomeEmail[80];
    char WorkEmail[80];
}sVcardInfo;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#if (FEATURE_OFF == MBB_NFC)
u16 NDEF_ReadVcard ( sRecordInfo *pRecordStruct, sVcardInfo *pVcardStruct ){return ERROR;}
u16 NDEF_WriteVcard ( sVcardInfo *pVcardStruct ){return ERROR;}
#endif
#endif /* __LIB_NDEF_VCARD_H */


/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
