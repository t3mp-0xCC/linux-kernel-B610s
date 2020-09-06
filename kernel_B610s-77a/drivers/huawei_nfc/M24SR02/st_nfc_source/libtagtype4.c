/**
  ******************************************************************************
  * @file    lib_TagType4.c
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
/* Includes ------------------------------------------------------------------*/

#include <mbb_config.h>

#if (FEATURE_ON == MBB_NFC)
#include <linux/kernel.h>
#include <linux/string.h>
#endif
#include "libtagtype4.h"
#define DATA_MAX_SIZE 246
#define NETWORKKEY_LENGTH (uint16_t)                    14
#define SSID_LENGTH (uint16_t)                           8

/*NDEF Wi-Fi Record related Data*/
  uint8_t ssidData[SSID_LENGTH]={0x48,0x6F,0x6D,0x65,0x57,0x4C,0x41,0x4E};

  uint8_t networkKey[NETWORKKEY_LENGTH]={0x4D,0x79,0x50,0x72,0x65,0x53,0x68,0x61,
                                         0x72,0x65,0x64,0x4B,0x65,0x79};
/** @addtogroup NFC_libraries
 *  @{
 *  @brief  <b>This is the library used to manage the content of the TAG (data)
 *          But also the specific feature of the tag, for instance
 *          password, gpo... </b>
 */


/** @addtogroup libNFC_FORUM
  * @{
    *   @brief  This part of the library manage data which follow NFC forum organisation.
  */


/**
 * @brief  This buffer contains the data send/received by TAG
 */
uint8_t NDEF_Buffer [NDEF_MAX_SIZE];
#if (FEATURE_ON == MBB_NFC)
/**
 * @brief  This buffer contains the wifi data send/received by TAG
 */
uint8_t NDEF_Wifi_Buffer [NDEF_MAX_SIZE];

/**
 * @brief  This buffer contains the URL data send/received by TAG
 */
uint8_t NDEF_URL_Buffer [NDEF_MAX_SIZE];
#endif
/**
 * @brief  This structure contains the data of the CC file
 */
sCCFileInfo CCFileStruct;

/**
 * @brief  This structure contains the information encapsuled in the record header
 *               with few more for SW purpose
 */
sRecordInfo RecordStruct;

/** @defgroup libTT4_Private_Functions
  * @{
  */

/**
  * @}
  */


/** @defgroup libTT4_Public_Functions
  * @{
    *   @brief  This file is used to access tag type 4.
  */ 

/**
  * @brief  This fonction initialize Tag Type 4
    * @param    None 
  * @retval SUCCESS : Initialization done
  */
u16 TT4_Init (void)
{
    u16 status = SUCCESS;
    u8 CCBuffer[15] = {0};
    sCCFileInfo *pCCFile;
    
    pCCFile = &CCFileStruct;

    status = TagT4Init( CCBuffer, sizeof(CCBuffer));
    
    pCCFile->NumberCCByte = (uint16_t) ((CCBuffer[0x00]<<8) | CCBuffer[0x01]);
    pCCFile->Version = CCBuffer[0x02];
    pCCFile->MaxReadByte = (uint16_t) ((CCBuffer[0x03]<<8) | CCBuffer[0x04]);
    pCCFile->MaxWriteByte = (uint16_t) ((CCBuffer[0x05]<<8) | CCBuffer[0x06]);
    pCCFile->TField = CCBuffer[0x07];
    pCCFile->LField = CCBuffer[0x08];
    pCCFile->FileID = (uint16_t) ((CCBuffer[0x09]<<8) | CCBuffer[0x0A]);
    pCCFile->NDEFFileMaxSize = (uint16_t) ((CCBuffer[0x0B]<<8) | CCBuffer[0x0C]);
    pCCFile->ReadAccess = CCBuffer[0x0D];
    pCCFile->WriteAccess = CCBuffer[0x0E];
    return status;
}   

/**
  * @brief  This fonction read NDEF file
    * @param    pNDEF : pointer on buffer to fill with NDEF data
  * @retval SUCCESS : NDEF file data have been retrieve
    * @retval ERROR : Not able to get NDEF file data
  */
u16 TT4_ReadNDEF(u8 *pNDEF)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    
    pCCFile = &CCFileStruct;
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        status = NDEF_ReadNDEF(pNDEF);
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}

/**
  * @brief  This fonction write NDEF file
    * @param    pNDEF : pointer on buffer that contain the NDEF data
  * @retval SUCCESS : NDEF file data have been stored
    * @retval ERROR : Not able to store NDEF file
  */
u16 TT4_WriteNDEF(u8 *pNDEF)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    
    pCCFile = &CCFileStruct;
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        status = NDEF_WriteNDEF( pNDEF);
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}
/**
  * @brief  This fonction read NDEF file if NDEF is identified as URI
    * @param    pURI : pointer on URI structure to fill with read data
  * @retval SUCCESS : URI structure has been updated
    * @retval ERROR : Not able to fill URI structure
  */
u16 TT4_ReadURI(sURI_Info *pURI)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    sRecordInfo *pRecordStruct;
    
    pCCFile = &CCFileStruct;
    pRecordStruct = &RecordStruct;
    
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        if(NDEF_IdentifyNDEF( pRecordStruct, NDEF_Buffer) == SUCCESS)
        {
            status = NDEF_ReadURI(pRecordStruct, pURI);
        }
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}

/**
  * @brief  This fonction write NDEF file from data given in the URI structure
    * @param    pURI : pointer on URI structure to prepare NDEF
  * @retval SUCCESS : NDEF URI stored
    * @retval ERROR : Not able to store NDEF URI
  */
u16 TT4_WriteURI(sURI_Info *pURI)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    
    pCCFile = &CCFileStruct;
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        status = NDEF_WriteURI ( pURI );
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}


/**
  * @brief  This fonction read NDEF file if NDEF is identified as Text
  * @param  pText : pointer on text buffer to fill with read data
  * @retval SUCCESS : text buffer has been updated
  * @retval ERROR : Not able to fill text buffer
  */
u16 TT4_ReadText(char *pText)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    sRecordInfo *pRecordStruct;

    pCCFile = &CCFileStruct;
    pRecordStruct = &RecordStruct;


    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        if(NDEF_IdentifyNDEF( pRecordStruct, NDEF_Buffer) == SUCCESS)
        {
            status = NDEF_ReadText(pRecordStruct, pText);
        }
        CloseNDEFSession(pCCFile->FileID);
    }

    return status;
}

/**
  * @brief  This fonction write NDEF file from data given in text buffer
  * @param  pText : pointer on text buffer to prepare NDEF
  * @retval SUCCESS : NDEF Text stored
  * @retval ERROR : Not able to store NDEF Text
  */
u16 TT4_WriteText(char *pText)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;

    pCCFile = &CCFileStruct;

    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        status = NDEF_WriteText ( pText );
        CloseNDEFSession(pCCFile->FileID);
    }

    return status;
}
#if (FEATURE_OFF == MBB_NFC)
/**
  * @brief  This fonction read NDEF file if NDEF is identified as SMS
    * @param    pSMS : pointer on SMS structure to fill with read data
  * @retval SUCCESS : SMS structure has been updated
    * @retval ERROR : Not able to fill MS structure
  */
u16 TT4_ReadSMS(sSMSInfo *pSMS)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    sRecordInfo *pRecordStruct;
    
    pCCFile = &CCFileStruct;
    pRecordStruct = &RecordStruct;
        
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {   
        if(NDEF_IdentifyNDEF( pRecordStruct, NDEF_Buffer) == SUCCESS)
        {
            status = NDEF_ReadSMS(pRecordStruct, pSMS);
        }
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}

/**
  * @brief  This fonction write NDEF file from data given in the SMS structure
    * @param    pSMS : pointer on SMS structure to prepare NDEF
  * @retval SUCCESS : NDEF SMS stored
    * @retval ERROR : Not able to store NDEF SMS
  */
u16 TT4_WriteSMS(sSMSInfo *pSMS)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    
    pCCFile = &CCFileStruct;
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        status = NDEF_WriteSMS ( pSMS );
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}

/**
  * @brief  This fonction read NDEF file if NDEF is identified as eMail
    * @param    pEmailStruct : pointer on eMail structure to fill with read data
  * @retval SUCCESS : eMail structure has been updated
    * @retval ERROR : Not able to fill eMail structure
  */
u16 TT4_ReadEmail (sEmailInfo *pEmailStruct)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    sRecordInfo *pRecordStruct;
    
    pCCFile = &CCFileStruct;
    pRecordStruct = &RecordStruct;
    
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        if(NDEF_IdentifyNDEF( pRecordStruct, NDEF_Buffer) == SUCCESS)
        {
            status = NDEF_ReadEmail(pRecordStruct, pEmailStruct);
        }
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;      
}

/**
  * @brief  This fonction write NDEF file from data given in the eMail structure
    * @param    pEmailStruct : pointer on eMail structure to prepare NDEF
  * @retval SUCCESS : NDEF eMail stored
    * @retval ERROR : Not able to store NDEF eMail
  */
u16 TT4_WriteEmail(sEmailInfo *pEmailStruct)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    
    pCCFile = &CCFileStruct;
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        status = NDEF_WriteEmail ( pEmailStruct );
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}

/**
  * @brief  This fonction read NDEF file if NDEF is identified as Vcard
    * @param    pVcard : pointer on Vcard structure to fill with read data
  * @retval SUCCESS : Vcard structure has been updated
    * @retval ERROR : Not able to fill Vcard structure
  */
u16 TT4_ReadVcard(sVcardInfo *pVcard)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    sRecordInfo *pRecordStruct;
    
    pCCFile = &CCFileStruct;
    pRecordStruct = &RecordStruct;
    
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {   
        if(NDEF_IdentifyNDEF( pRecordStruct, NDEF_Buffer) == SUCCESS)
        {
            status = NDEF_ReadVcard(pRecordStruct, pVcard);
        }
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}

/**
  * @brief  This fonction write NDEF file from data given in the Vcard structure
    * @param    pVcard : pointer on Vcard structure to prepare NDEF
  * @retval SUCCESS : NDEF Vcard stored
    * @retval ERROR : Not able to store NDEF Vcard
  */
u16 TT4_WriteVcard(sVcardInfo *pVcard)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    
    pCCFile = &CCFileStruct;
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {   
        status = NDEF_WriteVcard ( pVcard );
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}

/**
  * @brief  This fonction read NDEF file if NDEF is identified as geolocation information
    * @param    pGeo : pointer on geo structure to fill with read data
  * @retval SUCCESS : geo structure has been updated
    * @retval ERROR : Not able to fill geo structure
  */
u16 TT4_ReadGeo(sGeoInfo *pGeo)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    sRecordInfo *pRecordStruct;
    
    pCCFile = &CCFileStruct;
    pRecordStruct = &RecordStruct;
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        if(NDEF_IdentifyNDEF( pRecordStruct, NDEF_Buffer) == SUCCESS)
        {
            status = NDEF_ReadGeo(pRecordStruct, pGeo);
        }
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}

/**
  * @brief  This fonction write NDEF file from data given in the geo structure
    * @param    pGeo : pointer on geo structure to prepare NDEF
  * @retval SUCCESS : NDEF geo stored
    * @retval ERROR : Not able to store NDEF geo
  */
u16 TT4_WriteGeo(sGeoInfo *pGeo)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    
    pCCFile = &CCFileStruct;
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        status = NDEF_WriteGeo ( pGeo );
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}

/**
  * @brief  This fonction read NDEF file if NDEF is identified as expected private application
    * @param    pMyAppStruct : pointer on structure to fill with read data
  * @retval SUCCESS : structure has been updated
    * @retval ERROR : Not able to fill structure
  */
u16 TT4_ReadMyApp(sMyAppInfo *pMyAppStruct)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    sRecordInfo *pRecordStruct;
    
    pCCFile = &CCFileStruct;
    pRecordStruct = &RecordStruct;
    
    
    if(OpenNDEFSession(pCCFile->FileID, TAKE_SESSION) == SUCCESS)
    {
        if(NDEF_IdentifyNDEF( pRecordStruct, NDEF_Buffer) == SUCCESS)
        {
            status = NDEF_ReadMyApp(pRecordStruct, pMyAppStruct);
        }
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}

/**
  * @brief  This fonction write NDEF file from data given in the structure
    * @param    pMyAppStruct : pointer on structure to prepare NDEF
  * @retval SUCCESS : NDEF stored
    * @retval ERROR : Not able to store NDEF
  */
u16 TT4_WriteMyApp(sMyAppInfo *pMyAppStruct)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    
    pCCFile = &CCFileStruct;
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        status = NDEF_WriteMyApp ( pMyAppStruct );
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}
#endif

/**
  * @brief  This function read NDEF file if NDEF is identified as WifiToken
    * @param    pWifiToken : pointer on WifiToken structure to fill with read data
  * @retval SUCCESS : WifiToken structure has been updated
    * @retval ERROR : Not able to fill WifiToken structure
  */
u16 TT4_ReadWifiToken(sWifiTokenInfo *pWifiToken)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    sRecordInfo *pRecordStruct;
    
    pCCFile = &CCFileStruct;
    pRecordStruct = &RecordStruct;
  
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {   
        if(NDEF_IdentifyNDEF( pRecordStruct, NDEF_Buffer) == SUCCESS)
        {
            status = NDEF_ReadWifiToken(pRecordStruct, pWifiToken);
        }
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}

/**
  * @brief  This fonction write NDEF file from data given in the pWifiToken structure
    * @param    ppWifiToken : pointer on pWifiToken structure to prepare NDEF
  * @retval SUCCESS : NDEF pWifiToken stored
    * @retval ERROR : Not able to store NDEF pWifiToken
  */
u16 TT4_WriteWifiToken(sWifiTokenInfo *pWifiToken)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    
    pCCFile = &CCFileStruct;
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        status = NDEF_WriteWifiToken ( pWifiToken );
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}
#if (FEATURE_ON == MBB_NFC)
/**
  * @brief  This function read NDEF file if NDEF is identified as WifiToken
    * @param    pWifiToken : pointer on WifiToken structure to fill with read data
  * @retval SUCCESS : WifiToken structure has been updated
    * @retval ERROR : Not able to fill WifiToken structure
  */
u16 TT4_ReadWifiURLToken(sWifiUrlToken *pToken)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    sRecordInfo *pRecordStruct;

    sWifiTokenInfo *pWifiToken = NULL;
    sURI_Info *pURI = NULL;
    u32 nWifiDataSize = 0;
    u32 nWifiPayload = 0;

    if(NULL == pToken)
    {
        printk(KERN_ERR "%s (%d) : pToken is NULL !\n",__func__,__LINE__);
        return ERROR;
    }
    pCCFile = &CCFileStruct;
    pRecordStruct = &RecordStruct;

    pWifiToken = &(pToken->WifiToken);
    pURI = &(pToken->URLToken);
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {   
        memset(NDEF_Buffer,0,NDEF_MAX_SIZE);
        if(SUCCESS == NDEF_IdentifyNDEFByOffset(FIRST_RECORD_OFFSET,
                                                pRecordStruct, 
                                                NDEF_Buffer))
        {   
            status = NDEF_ReadWifiToken(pRecordStruct, pWifiToken);
            nWifiPayload = ((u32)(pRecordStruct->PayloadLength3)<<24) | ((u32)(pRecordStruct->PayloadLength2)<<16) |
                                ((u32)(pRecordStruct->PayloadLength1)<<8)  | pRecordStruct->PayloadLength0;
            nWifiDataSize = pRecordStruct->PayloadOffset + nWifiPayload;
            memset(NDEF_Buffer,0,NDEF_MAX_SIZE);
            if(SUCCESS == NDEF_IdentifyNDEFByOffset((FIRST_RECORD_OFFSET+nWifiDataSize),pRecordStruct, NDEF_Buffer))
            {
                status = NDEF_ReadURI(pRecordStruct, pURI);
            }
            else
            {
                printk(KERN_ERR "Identify URL Token failed !\n");
            }          
        }
        else
        {
            printk(KERN_ERR "Identify WiFi Token failed !\n");
        }
        CloseNDEFSession(pCCFile->FileID);
    }
    
    return status;
}

/**
  * @brief  This fonction write NDEF file from data given in the pWifiToken structure
    * @param    ppWifiToken : pointer on pWifiToken structure to prepare NDEF
  * @retval SUCCESS : NDEF pWifiToken stored
    * @retval ERROR : Not able to store NDEF pWifiToken
  */
u16 TT4_WriteWifiURLToken(sWifiUrlToken *pToken)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    sWifiTokenInfo *pWifiToken = NULL;
    u16 nWifiDataSize = 0;
    u16 nURLDataSize = 0;
    u16 nWriteSize = 0;
    sURI_Info *pURI = NULL;
    uint8_t Password[16]={0};
    
    if(NULL == pToken)
    {
        printk(KERN_ERR "%s (%d) : pToken is NULL !\n",__func__,__LINE__);
        return ERROR;
    }
    pCCFile = &CCFileStruct;
    pWifiToken = &(pToken->WifiToken);
    pURI = &(pToken->URLToken);
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        M24SR_GetNewPasswd(I2C_PWD,Password);
        status = M24SR_Verify(I2C_PWD,0x10 ,Password);
        if (M24SR_PWD_CORRECT != status)
        {
            printk(KERN_ERR "%s (%d) : verify I2C passwd failed , status is %x!\n",
                        __func__,__LINE__,status);
            return ERROR;
        }
        memset(NDEF_Wifi_Buffer,0,NDEF_MAX_SIZE);
        memset(NDEF_URL_Buffer,0,NDEF_MAX_SIZE);
        memset(NDEF_Buffer,0,NDEF_MAX_SIZE);
        /*按规范打包WiFi信息*/
        NDEF_PackageWifiToken (pWifiToken,NDEF_Wifi_Buffer,&nWifiDataSize);
        /*按规范打包URL信息*/
        NDEF_PackageWriteURI (pURI ,NDEF_URL_Buffer,&nURLDataSize);
      
        memcpy(NDEF_Buffer,NDEF_Wifi_Buffer,(size_t)nWifiDataSize);
        /*减去表示整个URL NDEF长度的2Bytes*/
        memcpy(&(NDEF_Buffer[nWifiDataSize]),&(NDEF_URL_Buffer[2]),(size_t)(nURLDataSize-2));
        /* Must not count the 2 byte that represent the NDEF size. wifi+url = 4.*/
        nWriteSize = nWifiDataSize + nURLDataSize - 4;
        NDEF_Buffer[0] = (nWriteSize & 0xFF00)>>8;
        NDEF_Buffer[1] = (nWriteSize & 0x00FF);

        status = NDEF_WriteNDEF(NDEF_Buffer);
        CloseNDEFSession(pCCFile->FileID);
        if(SUCCESS != status)
        {
            printk(KERN_ERR "%s (%d): write NDEF failed !\n",__func__,__LINE__);
        }
    }
    else
    {
         printk(KERN_ERR "%s (%d): OpenNDEFSession failed !\n",__func__,__LINE__);
    }
    
    return status;
}

#endif
#if (FEATURE_OFF == MBB_NFC)
/**
  * @brief  This fonction add AAR (Android Application Record) in the tag
    * @param    pAAR : pointer on structure that contain AAR information
  * @retval SUCCESS : AAR added
    * @retval ERROR : Not able to add AAR
  */
u16 TT4_AddAAR(sAARInfo *pAAR)
{
    u16 status = ERROR;
    sCCFileInfo *pCCFile;
    
    pCCFile = &CCFileStruct;
    
    if(OpenNDEFSession(pCCFile->FileID, ASK_FOR_SESSION) == SUCCESS)
    {
        status = NDEF_AddAAR ( pAAR );
        CloseNDEFSession(pCCFile->FileID);
    }

    return status;
}
#endif
#if (FEATURE_ON == MBB_NFC)
EXPORT_SYMBOL(TT4_Init);
EXPORT_SYMBOL(TT4_ReadURI);
EXPORT_SYMBOL(TT4_WriteURI);
EXPORT_SYMBOL(TT4_ReadText);
EXPORT_SYMBOL(TT4_WriteText);
EXPORT_SYMBOL(TT4_ReadWifiToken);
EXPORT_SYMBOL(TT4_WriteWifiToken);
#endif
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/

