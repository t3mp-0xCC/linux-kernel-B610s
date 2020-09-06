/**
  ******************************************************************************
  * @file    libndef.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to manage NDEF file.
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
#ifndef __LIB_NDEF_H
#define __LIB_NDEF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* include file which match the HW configuration */

#include <mbb_config.h>

#if (FEATURE_ON == MBB_NFC)
#include <linux/kernel.h>
#endif
#include "libwrapper.h"
     
#ifndef errorchk
#define errorchk(fCall) if (status = (fCall), status != M24SR_ACTION_COMPLETED) \
    {goto Error;} else
#endif
    

#define NDEF_SIZE_OFFSET                    0    
#define FIRST_RECORD_OFFSET             2   
#define CONFIG_TOKEN_1              16
#define CONFIG_TOKEN_3              14
#define CONFIG_TOKEN_5              30



#define RECORD_FLAG_FIELD                   1    
#define TYPE_LENGTH_FIELD                   1
#define ID_LENGTH_FIELD                     1    


#define MB_Mask                                     ((u8)(0x80))
#define ME_Mask                                     ((u8)(0x40))
#define CF_Mask                                     ((u8)(0x20))
#define SR_Mask                                     ((u8)(0x10))
#define IL_Mask                                     ((u8)(0x08))
#define TNF_Mask                                    ((u8)(0x07))

#define TNF_Empty                                   0x00
#define TNF_WellKnown                           0x01
#define TNF_MediaType                           0x02
#define TNF_AbsoluteURI                     0x03
#define TNF_NFCForumExternal            0x04
#define TNF_Unknown                         0x05
#define TNF_Unchanged                           0x06
#define TNF_Reserved                            0x07

#define SP_MAX_RECORD                           3

#define AAR_TYPE_STRING                                                 "android.com:pkg"   
#define AAR_TYPE_STRING_LENGTH                                  15

#define M24SR_DISCOVERY_APP_STRING                          "st.com:m24sr_discovery_democtrl"    
#define M24SR_DISCOVERY_APP_STRING_LENGTH               31  

#define VCARD_TYPE_STRING                                               "text/vcard"    
#define VCARD_TYPE_STRING_LENGTH                                10   

#define XVCARD_TYPE_STRING                                          "text/x-vCard"   
#define XVCARD_TYPE_STRING_LENGTH                               12

#define SMART_POSTER_TYPE_STRING                                "Sp"
#define SMART_POSTER_TYPE_STRING_LENGTH                 2
     
#define URI_TYPE_STRING                                                 "U"
#define URI_TYPE_STRING_LENGTH                                  1
     
#define SMS_TYPE_STRING                                                 "sms:"  
#define SMS_TYPE_STRING_LENGTH                                  4

#define WIFITOKEN_TYPE_STRING                                                   "application/vnd.wfa.wsc"   
#define WIFITOKEN_TYPE_STRING_LENGTH                                    23


#define WIFITOKEN_TYPE_STRING_1                                                 ".E"    
#define WIFITOKEN_TYPE_STRING_LENGTH_1                                  2


#define GEO_TYPE_STRING                                                 "geo:"  
#define GEO_TYPE_STRING_LENGTH                                  4

#define URI_LATITUDE_END                                                ","
#define URI_LATITUDE_END_LENGTH                                 1

#define EMAIL_TYPE_STRING                               "mailto:"
#define EMAIL_TYPE_STRING_LENGTH                                7

#define URI_FIRST_DATA_END                                          "?"
#define URI_FIRST_DATA_END_LENGTH                               1

#define SUBJECT_BEGIN_STRING                                        "subject="
#define SUBJECT_BEGIN_STRING_LENGTH                         8    

#define MESSAGE_BEGIN_STRING                                        "body="  
#define MESSAGE_BEGIN_STRING_LENGTH                         5    

#define URI_SECOND_DATA_END                                         "&"
#define URI_SECOND_DATA_END_LENGTH                          1

#define TEXT_TYPE_STRING                                                "T"
#define TEXT_TYPE_STRING_LENGTH                                 1

#define ISO_ENGLISH_CODE_STRING                                 "en"


#define URI_ID_0x00                             0x00
#define URI_ID_0x01                             0x01
#define URI_ID_0x02                             0x02
#define URI_ID_0x03                             0x03
#define URI_ID_0x04                             0x04
#define URI_ID_0x05                             0x05
#define URI_ID_0x06                             0x06
#define URI_ID_0x07                             0x07
#define URI_ID_0x08                             0x08
#define URI_ID_0x09                             0x09
#define URI_ID_0x0A                             0x0A
#define URI_ID_0x0B                             0x0B
#define URI_ID_0x0C                             0x0C
#define URI_ID_0x0D                             0x0D
#define URI_ID_0x0E                             0x0E
#define URI_ID_0x0F                             0x0F
#define URI_ID_0x10                             0x10
#define URI_ID_0x11                             0x11
#define URI_ID_0x12                             0x12
#define URI_ID_0x13                             0x13
#define URI_ID_0x14                             0x14
#define URI_ID_0x15                             0x15
#define URI_ID_0x16                             0x16
#define URI_ID_0x17                             0x17
#define URI_ID_0x18                             0x18
#define URI_ID_0x19                             0x19
#define URI_ID_0x1A                             0x1A
#define URI_ID_0x1B                             0x1B
#define URI_ID_0x1C                             0x1C
#define URI_ID_0x1D                             0x1D
#define URI_ID_0x1E                             0x1E
#define URI_ID_0x1F                             0x1F
#define URI_ID_0x20                             0x20
#define URI_ID_0x21                             0x21
#define URI_ID_0x22                             0x22
#define URI_ID_0x23                             0x23
#define URI_RFU                                     0x24

#define URI_ID_0x01_STRING                              "http://www.\0"
#define URI_ID_0x02_STRING                              "https://www.\0"
#define URI_ID_0x03_STRING                              "http://\0"
#define URI_ID_0x04_STRING                              "https://\0"
#define URI_ID_0x05_STRING                              "tel:\0"
#define URI_ID_0x06_STRING                              "mailto:\0"
#define URI_ID_0x07_STRING                              "ftp://anonymous:anonymous@\0"
#define URI_ID_0x08_STRING                              "ftp://ftp.\0"
#define URI_ID_0x09_STRING                              "ftps://\0"
#define URI_ID_0x0A_STRING                              "sftp://\0"
#define URI_ID_0x0B_STRING                              "smb://\0"
#define URI_ID_0x0C_STRING                              "nfs://\0"
#define URI_ID_0x0D_STRING                              "ftp://\0"
#define URI_ID_0x0E_STRING                              "dav://\0"
#define URI_ID_0x0F_STRING                              "news:\0"
#define URI_ID_0x10_STRING                              "telnet://\0"
#define URI_ID_0x11_STRING                              "imap:\0"
#define URI_ID_0x12_STRING                              "rtsp://\0"
#define URI_ID_0x13_STRING                              "urn:\0"
#define URI_ID_0x14_STRING                              "pop:\0"
#define URI_ID_0x15_STRING                              "sip:\0"
#define URI_ID_0x16_STRING                              "sips:\0"
#define URI_ID_0x17_STRING                              "tftp:\0"
#define URI_ID_0x18_STRING                              "btspp://\0"
#define URI_ID_0x19_STRING                              "btl2cap://\0"
#define URI_ID_0x1A_STRING                              "btgoep://\0"
#define URI_ID_0x1B_STRING                              "tcpobex://\0"
#define URI_ID_0x1C_STRING                              "irdaobex://\0"
#define URI_ID_0x1D_STRING                              "file://\0"
#define URI_ID_0x1E_STRING                              "urn:epc:id:\0"
#define URI_ID_0x1F_STRING                              "urn:epc:tag\0"
#define URI_ID_0x20_STRING                              "urn:epc:pat:\0"
#define URI_ID_0x21_STRING                              "urn:epc:raw:\0"
#define URI_ID_0x22_STRING                              "urn:epc:\0"
#define URI_ID_0x23_STRING                              "urn:nfc:\0"

#define ISO_ENGLISH_CODE_STRING "en"
#define ISO_ENGLISH_CODE_STRING_LENGTH 2
#if (FEATURE_ON == MBB_NFC)
typedef struct
{
       u8 nID;
       u8 aString[32];
}URI_StringDef;
#endif
typedef enum
{
    UNKNOWN_TYPE = 0,
    VCARD_TYPE,
    WELL_KNOWN_ABRIDGED_URI_TYPE,
    URI_SMS_TYPE,
    URI_GEO_TYPE,
    URI_EMAIL_TYPE,
    SMARTPOSTER_TYPE,
    URL_TYPE,
    TEXT_TYPE,
    BT_TYPE,
    URI_WIFITOKEN_TYPE,
    /* list of "external type" known by this demo, other external type will be addressed as UNKNWON_TYPE */
    M24SR_DISCOVERY_APP_TYPE
} NDEF_TypeDef;



typedef struct
{
  u8 RecordFlags;
  u8 TypeLength;
    u8 PayloadLength3;
    u8 PayloadLength2;
    u8 PayloadLength1;
    u8 PayloadLength0;
    u8 IDLength;
    u8 Type[0xFF];
    u8 ID[0xFF];
    u16 PayloadOffset;
    u32 PayloadBufferAdd;    /* add where payload content has been stored */
    NDEF_TypeDef NDEF_Type;  /* to store identification ID for application */
    u32 SPRecordStructAdd[SP_MAX_RECORD]; /*in case of smart poster array to store add of other sRecordInfo struct */
    u8 NbOfRecordInSPPayload;
}sRecordInfo;   


u16 NDEF_IdentifyNDEF ( sRecordInfo *pRecordStruct, u8* pNDEF );
#if (FEATURE_ON == MBB_NFC)
u16 NDEF_IdentifyNDEFByOffset (  u16 Offset,sRecordInfo *pRecordStruct, u8* pNDEF );
#endif
u16 NDEF_ReadNDEF( u8 *pNDEF);
u16 NDEF_WriteNDEF( u8 *pNDEF);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __LIB_NDEF_H */


/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/
