/**
  ******************************************************************************
  * @file    lib_NDEF_Wifi.c
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
/* Includes ------------------------------------------------------------------*/
#include "libndefwifi.h"
#if (FEATURE_ON == MBB_NFC)
#include <linux/string.h>
#include <linux/kernel.h>
#else
#include "string.h"
#endif
#define ATTRIBUTE_ID_SSID_LSB                                               0X10
#define ATTRIBUTE_ID_SSID_MSB                                               0X45


#define ATTRIBUTE_ID_NETWORK_LSB                                            0X10
#define ATTRIBUTE_ID_NETWORK_MSB                                            0X27
#if (FEATURE_ON == MBB_NFC)
#define ATTRIBUTE_ID_AUTHENTICATION_LSB                         0x10
#define ATTRIBUTE_ID_AUTHENTICATION_MSB                        0x03

#define AUTHENTICATION_TYPE_OPEN               "OPEN" 
#define AUTHENTICATION_TYPE_WPAPSK            "WPAPSK"
#define AUTHENTICATION_TYPE_SHARED            "SHARED"
#define AUTHENTICATION_TYPE_WPA                "WPA"
#define AUTHENTICATION_TYPE_WPA2              "WPA2"
#define AUTHENTICATION_TYPE_WPA2PSK           "WPA2PSK"

enum 
{
    OPEN = 0x01,
    WPAPSK = 0x02,
    SHARED = 0x04,
    WPA = 0x08,
    WPA2 = 0x10,
    WPA2PSK = 0x20,
};
#endif
/**
 * @brief  This buffer contains the data send/received by TAG
 */
extern uint8_t NDEF_Buffer [NDEF_MAX_SIZE];

/** @defgroup libWifiToken_Private_Functions
  * @{
  */

static void NDEF_FillWifiTokenStruct( uint8_t* pPayload, uint32_t PayloadSize, sWifiTokenInfo *pWifiTokenStruct);
static void NDEF_Read_WifiToken ( sRecordInfo *pRecordStruct, sWifiTokenInfo *pWifiTokenStruct );
#if (FEATURE_ON == MBB_NFC)
static uint8_t NDEF_SelectWiFiAuthenticationType(sWifiTokenInfo *pWifiTokenStruct);
static void NDEF_ParseWiFiAuthenticationType(uint8_t nType,sWifiTokenInfo *pWifiTokenStruct);
#endif
/**
  * @brief  This function fill WifiToken structure with information of NDEF message
    * @param    pPayload : pointer on the payload data of the NDEF message
    * @param    PayloadSize : number of data in the payload
    * @param    pWifiTokenStruct : pointer on the structure to fill
  * @retval NONE 
  */
static void NDEF_FillWifiTokenStruct( uint8_t* pPayload, uint32_t PayloadSize, sWifiTokenInfo *pWifiTokenStruct)
{
    uint8_t* pLastByteAdd,data1,data2,*temp,*temp_br ;
        uint16_t SSIDLen,NetWorkKeyLen;
        uint8_t *dbg,dbg1;
    
    pLastByteAdd = (uint8_t*)(pPayload + PayloadSize);
        pPayload--;     
        
        while(pPayload++ != pLastByteAdd)
        {
          uint8_t attribute = *pPayload;
          temp_br = pPayload;
          switch(attribute)
          {
            
          case ATTRIBUTE_ID_SSID_LSB:
            temp = pPayload;
            dbg = temp;
            dbg1 = *++dbg;
            if(dbg1 == ATTRIBUTE_ID_SSID_MSB )
            {
              data1 = *++dbg;
              data2 = *++dbg;
              SSIDLen = data1;
              SSIDLen = SSIDLen << 8;
              SSIDLen |= data2;
              pPayload += 4;
              memcpy( pWifiTokenStruct->NetworkSSID, pPayload, SSIDLen);
              /* add end of string charactere */
              pWifiTokenStruct->NetworkSSID[SSIDLen] = '\0';    
              pPayload += SSIDLen - 1;
            }
            else if(dbg1 == ATTRIBUTE_ID_NETWORK_MSB )
            {
              data1 = *++dbg;
              data2 = *++dbg;
              NetWorkKeyLen = data1;
              NetWorkKeyLen = NetWorkKeyLen << 8;
              NetWorkKeyLen |= data2;
              pPayload += 4;
              memcpy( pWifiTokenStruct->NetworkKey, pPayload, NetWorkKeyLen);
              /* add end of string charactere */
              pWifiTokenStruct->NetworkKey[NetWorkKeyLen] = '\0';
              pPayload += NetWorkKeyLen -1;
            }
#if (FEATURE_ON == MBB_NFC)
            else if(dbg1 == ATTRIBUTE_ID_AUTHENTICATION_MSB )
            {
              dbg += 4;
              data1 = *dbg;
              NDEF_ParseWiFiAuthenticationType(data1,pWifiTokenStruct);
              pPayload += 6;
            }
#endif
            else
            {
              pPayload = temp_br;
            }
            
          break; 
           
          default :
            ;
          }
        }
        
}

/**
  * @brief  This fonction read the WifiToken and store data in a structure
    * @param    pRecordStruct : Pointer on the record structure
    * @param    pWifiTokenStruct : pointer on the structure to fill
  * @retval NONE 
  */
static void NDEF_Read_WifiToken ( sRecordInfo *pRecordStruct, sWifiTokenInfo *pWifiTokenStruct )
{
    uint8_t* pPayload;
    uint32_t PayloadSize;
    
    PayloadSize = ((uint32_t)(pRecordStruct->PayloadLength3)<<24) | ((uint32_t)(pRecordStruct->PayloadLength2)<<16) |
    ((uint32_t)(pRecordStruct->PayloadLength1)<<8)  | pRecordStruct->PayloadLength0;
    
    /* Read record header */
    pPayload = (uint8_t*)(pRecordStruct->PayloadBufferAdd);
    if( pRecordStruct->NDEF_Type == URI_WIFITOKEN_TYPE)
        NDEF_FillWifiTokenStruct(pPayload , PayloadSize, pWifiTokenStruct);
    
}

/**
  * @}
  */

/** @defgroup libWifiToken_Public_Functions
  * @{
  * @brief  This file is used to manage WifiToken (stored or loaded in tag)
  */ 

/**
  * @brief  This fonction read NDEF and retrieve WifiToken information if any
    * @param    pRecordStruct : Pointer on the record structure
    * @param    pWifiTokenStruct : pointer on the structure to fill 
  * @retval SUCCESS : WifiToken information from NDEF have been retrieve
    * @retval ERROR : Not able to retrieve WifiToken information
  */
uint16_t NDEF_ReadWifiToken(sRecordInfo *pRecordStruct, sWifiTokenInfo *pWifiTokenStruct)
{
    uint16_t status = ERROR;
    uint16_t FileId=0;

    if( pRecordStruct->NDEF_Type == URI_WIFITOKEN_TYPE )
    {   
        NDEF_Read_WifiToken(pRecordStruct, pWifiTokenStruct );
        status = SUCCESS;
    }
    else{
#if (FEATURE_ON == MBB_NFC)
          printk(KERN_ERR "NDEF_ReadWifiToken Error!\n");
#endif
    }
    
    CloseNDEFSession(FileId);
    
    return status;
}


/**
  * @brief  This fonction write the NDEF file with the WifiToken data given in the structure
    * @param    pWifiTokenStruct : pointer on structure that contain the WifiToken information
  * @retval SUCCESS : the function is succesful
    * @retval ERROR : Not able to store NDEF file inside tag.
  */
uint16_t NDEF_WriteWifiToken ( sWifiTokenInfo *pWifiTokenStruct )
{
    uint16_t status = ERROR;
        uint8_t* pPayload,initStage = 0;
        uint16_t DataSize;
        uint32_t PayloadSize,SSIDSize,SSIDKeySize;
        uint8_t configToken1[CONFIG_TOKEN_1] = {0x10,0x4A, /* Attribute ID : Version*/
                                    0x00,0x01, /* Attribute ID Length*/
                                    0x10,    /* Version 1.0*/
                                    0x10,0x0E,  /* Attribute ID Credential*/  
                                    0x00,0x43, /* Attribute ID Length*/
                                    0x10,0x26, /* Attribute ID : Network Index*/
                                    0x00,0x01, /* Attribute Length*/
                                    0x01,  /* Length*/
                                    0x10,0x45, /* Attribute ID :SSID*/
                            
        };
        
      //Fille SSID length + SSID between configToken1 and configToken3       
        uint8_t configToken3[CONFIG_TOKEN_3] = {0x10,0x03, /* Attribute ID :Authentication Type*/
                                    0x00,0x02, /* Attribute Length*/
                                    0x00,0x01,  /* Attribute Type : WPA2-Personal*/
                                    0x10,0x0F,  /* Attribute ID  : Encryption Type*/
                                    0x00,0x02,  /* Attribute Length*/
                                    0x00,0x02, /* Encryption Type : AES*/
                                    0x10,0x27};  /* Attribute ID  : Network Key */
        
              
     //Fill SSID KEY Length and SSID Key between configToken3 and configToken5
        
        uint8_t configToken5[CONFIG_TOKEN_5] = {0x10,0x20, /* Attribute ID  : MAC Address */
                                    0x00,0x06, /* Attribute Length*/
                                    0, /*MAC-ADDRESS*/
                                    0, /*MAC-ADDRESS*/
                                    0, /*MAC-ADDRESS*/
                                    0, /*MAC-ADDRESS*/
                                    0, /*MAC-ADDRESS*/
                                    0, /*MAC-ADDRESS*/
                                    0x10,0x49, /* Attribute ID  : Vendor Extension */
                                    0x00,0x06, /* Attribute Length*/
                                    0x00,0x37,0x2A, /* Vendor ID:WFA*/
                                    0x02, /* Subelement ID:Network Key Shareable*/
                                    0x01, /* Subelement Length*/
                                    0x01, /*Network Key Shareable : TRUE*/
                                    0x10,0x49, /* Attribute ID  : Vendor Extension */
                                    0x00,0x06,/* Attribute Length*/
                                    0x00,0x37,0x2A,/* Vendor ID:WFA*/
                                    0x00, /* Subelement ID:Version2*/
                                    0x01, /* Subelement Length:1*/
                                    0x20 /* Version2*/
        };
               

    NDEF_Buffer[0] = 0x00;
    NDEF_Buffer[1] = 0x00;
    
    /* fill Wifi record header */
    NDEF_Buffer[FIRST_RECORD_OFFSET] = 0xD2;   /* Record Flag */
    NDEF_Buffer[FIRST_RECORD_OFFSET+1] = WIFITOKEN_TYPE_STRING_LENGTH;
    NDEF_Buffer[FIRST_RECORD_OFFSET+2] = 76; /* needs to be autocalculated - done at the end */
  
    memcpy(&NDEF_Buffer[FIRST_RECORD_OFFSET+3], WIFITOKEN_TYPE_STRING, WIFITOKEN_TYPE_STRING_LENGTH);
    
    pPayload = &NDEF_Buffer[FIRST_RECORD_OFFSET+3+WIFITOKEN_TYPE_STRING_LENGTH];
    PayloadSize = 0;
    
    
        for(initStage=0;initStage<CONFIG_TOKEN_1;initStage++)
        {
          *pPayload =configToken1[initStage];
          pPayload++;
        }
        
        //Fill SSID length and SSID value
        SSIDSize = strlen(pWifiTokenStruct->NetworkSSID);
        *pPayload = 0x00; pPayload++;
        *pPayload = SSIDSize & 0x000000FF; pPayload++;
#if (FEATURE_ON == MBB_NFC)
        strncpy((char*)pPayload,pWifiTokenStruct->NetworkSSID,SSIDSize);
#else
        strcpy((char*)pPayload,pWifiTokenStruct->NetworkSSID);
#endif
        pPayload = pPayload + strlen(pWifiTokenStruct->NetworkSSID);
 
        for(initStage=0;initStage<CONFIG_TOKEN_3;initStage++)
        {
          *pPayload =configToken3[initStage];
          pPayload++;
        }
        
      //Fill SSIDKey length and SSIDKey value
        SSIDKeySize = strlen(pWifiTokenStruct->NetworkKey);
        *pPayload = 0x00; pPayload++;
        *pPayload = SSIDKeySize & 0x000000FF; pPayload++;
#if (FEATURE_ON == MBB_NFC)
        strncpy((char*)pPayload,pWifiTokenStruct->NetworkKey,SSIDKeySize);
#else
        strcpy((char*)pPayload,pWifiTokenStruct->NetworkKey);
#endif
        pPayload = pPayload + strlen(pWifiTokenStruct->NetworkKey);
                
        for(initStage=0;initStage<CONFIG_TOKEN_5;initStage++)
        {
          *pPayload =configToken5[initStage];
          pPayload++;
        }

    PayloadSize += CONFIG_TOKEN_1 + CONFIG_TOKEN_3 + CONFIG_TOKEN_5 + SSIDSize + SSIDKeySize;
        
    NDEF_Buffer[FIRST_RECORD_OFFSET+2] = (PayloadSize & 0x000000FF);
    
    DataSize = PayloadSize + 5 + WIFITOKEN_TYPE_STRING_LENGTH;
    
    /* Write NDEF */
    status = WriteData ( 0x00 , DataSize , NDEF_Buffer);

    /* Write NDEF size to complete*/
    if( status == NDEF_ACTION_COMPLETED)
    {
        DataSize -= 2; /* Must not count the 2 byte that represent the NDEF size */
        NDEF_Buffer[0] = (DataSize & 0xFF00)>>8;
        NDEF_Buffer[1] = (DataSize & 0x00FF);
    
        status = WriteData ( 0x00 , 2 , NDEF_Buffer);
    }
    
    if( status == NDEF_ACTION_COMPLETED)
        return SUCCESS;
    else
        return ERROR;
}
#if (FEATURE_ON == MBB_NFC)
/**
  * @brief  This fonction config wifi authentication type 
    * @param    pWifiTokenStruct : pointer on structure that contain the WifiToken information
  * @retval SUCCESS : the function is succesful
    * @retval ERROR : Not able to store NDEF file inside tag.
  */
static uint8_t NDEF_SelectWiFiAuthenticationType(sWifiTokenInfo *pWifiTokenStruct)
{
        if(NULL == pWifiTokenStruct)
        {
            printk(KERN_ERR "NDEF_SelectWiFiAuthenticationType: Ivalid params!\n");
            return ERROR;
        }

        if ( 0 == strncmp(AUTHENTICATION_TYPE_OPEN,
                   pWifiTokenStruct->AuthenticationType,
                   strlen(AUTHENTICATION_TYPE_OPEN)))
        {
              return OPEN;
        }
        else if (0 == strncmp(AUTHENTICATION_TYPE_WPAPSK,
                   pWifiTokenStruct->AuthenticationType,
                   strlen(AUTHENTICATION_TYPE_WPAPSK)))
        {
             return WPAPSK;
        }
        else if (0 == strncmp(AUTHENTICATION_TYPE_SHARED,
                   pWifiTokenStruct->AuthenticationType,
                   strlen(AUTHENTICATION_TYPE_SHARED)))
        {
             return SHARED;
        }
        else if (0 == strncmp(AUTHENTICATION_TYPE_WPA,
                   pWifiTokenStruct->AuthenticationType,
                   strlen(AUTHENTICATION_TYPE_WPA)))
        {
             return WPA;
        }
        else if (0 == strncmp(AUTHENTICATION_TYPE_WPA2,
                   pWifiTokenStruct->AuthenticationType,
                   strlen(AUTHENTICATION_TYPE_WPA2)))
        {
             return WPA2;
        }
        else if (0 == strncmp(AUTHENTICATION_TYPE_WPA2PSK,
                   pWifiTokenStruct->AuthenticationType,
                   strlen(AUTHENTICATION_TYPE_WPA2PSK)))
        {
             return WPA2PSK;
        }
        else
            return WPA2PSK;

}
/**
  * @brief  This fonction parse wifi authentication type 
    * @param    pWifiTokenStruct : pointer on structure that contain the WifiToken information
  * @retval SUCCESS : the function is succesful
    * @retval ERROR : Not able to store NDEF file inside tag.
  */
static void NDEF_ParseWiFiAuthenticationType(uint8_t nType,
                                             sWifiTokenInfo *pWifiTokenStruct)
{
    uint16_t nLen = 0;
    if(NULL == pWifiTokenStruct)
    {
        printk(KERN_ERR "NDEF_ParseWiFiAuthenticationType: Ivalid params!\n");
        return;
    }
    switch(nType)
    {
        case OPEN:
            nLen = strlen(AUTHENTICATION_TYPE_OPEN);
            strncpy(pWifiTokenStruct->AuthenticationType,AUTHENTICATION_TYPE_OPEN,nLen);
            break;
        case WPAPSK:
            nLen = strlen(AUTHENTICATION_TYPE_WPAPSK);
            strncpy(pWifiTokenStruct->AuthenticationType,AUTHENTICATION_TYPE_WPAPSK,nLen);
            break;
        case SHARED:
            nLen = strlen(AUTHENTICATION_TYPE_SHARED);
            strncpy(pWifiTokenStruct->AuthenticationType,AUTHENTICATION_TYPE_SHARED,nLen);
            break;
        case WPA:
            nLen = strlen(AUTHENTICATION_TYPE_WPA);
            strncpy(pWifiTokenStruct->AuthenticationType,AUTHENTICATION_TYPE_WPA,nLen);
            break;
        case WPA2:
            nLen = strlen(AUTHENTICATION_TYPE_WPA2);
            strncpy(pWifiTokenStruct->AuthenticationType,AUTHENTICATION_TYPE_WPA2,nLen);
            break;
        case WPA2PSK:
            nLen = strlen(AUTHENTICATION_TYPE_WPA2PSK);
            strncpy(pWifiTokenStruct->AuthenticationType,
                    AUTHENTICATION_TYPE_WPA2PSK,
                    nLen);
            break;
        default:
            nLen = strlen(AUTHENTICATION_TYPE_WPA2PSK);
            strncpy(pWifiTokenStruct->AuthenticationType,
                    AUTHENTICATION_TYPE_WPA2PSK,
                    nLen);
            break;
    }
     pWifiTokenStruct->AuthenticationType[nLen] = '\0';

}
/**
  * @brief  This fonction write the NDEF file with the WifiToken data given in the structure
    * @param    pWifiTokenStruct : pointer on structure that contain the WifiToken information
  * @retval SUCCESS : the function is succesful
    * @retval ERROR : Not able to store NDEF file inside tag.
  */
uint16_t NDEF_PackageWifiToken ( sWifiTokenInfo *pWifiTokenStruct, 
                                 uint8_t *pData,
                                 uint16_t *pDataSize)
{
    uint8_t* pPayload = NULL;
    uint8_t initStage = 0;
    uint16_t DataSize = 0;
    uint32_t PayloadSize = 0;
    uint32_t SSIDSize = 0;
    uint32_t SSIDKeySize = 0;
    uint8_t configToken1[CONFIG_TOKEN_1] = {0x10,0x4A, /* Attribute ID : Version*/
                                0x00,0x01, /* Attribute ID Length*/
                                0x10,    /* Version 1.0*/
                                0x10,0x0E,  /* Attribute ID Credential*/  
                                0x00,0x43, /* Attribute ID Length*/
                                0x10,0x26, /* Attribute ID : Network Index*/
                                0x00,0x01, /* Attribute Length*/
                                0x01,  /* Length*/
                                0x10,0x45, /* Attribute ID :SSID*/
                        
    };
    
  //Fille SSID length + SSID between configToken1 and configToken3       
    uint8_t configToken3[CONFIG_TOKEN_3] = {0x10,0x03, /* Attribute ID :Authentication Type*/
                                0x00,0x02, /* Attribute Length*/
                                0x00,0x20,  /* Attribute Type : WPA2-Personal*/
                                0x10,0x0F,  /* Attribute ID  : Encryption Type*/
                                0x00,0x02,  /* Attribute Length*/
                                0x00,0x08, /* Encryption Type : AES*/
                                0x10,0x27};  /* Attribute ID  : Network Key */
    
          
 //Fill SSID KEY Length and SSID Key between configToken3 and configToken5
    
    uint8_t configToken5[CONFIG_TOKEN_5] = {0x10,0x20, /* Attribute ID  : MAC Address */
                                0x00,0x06, /* Attribute Length*/
                                0, /*MAC-ADDRESS*/
                                0, /*MAC-ADDRESS*/
                                0, /*MAC-ADDRESS*/
                                0, /*MAC-ADDRESS*/
                                0, /*MAC-ADDRESS*/
                                0, /*MAC-ADDRESS*/
                                0x10,0x49, /* Attribute ID  : Vendor Extension */
                                0x00,0x06, /* Attribute Length*/
                                0x00,0x37,0x2A, /* Vendor ID:WFA*/
                                0x02, /* Subelement ID:Network Key Shareable*/
                                0x01, /* Subelement Length*/
                                0x01, /*Network Key Shareable : TRUE*/
                                0x10,0x49, /* Attribute ID  : Vendor Extension */
                                0x00,0x06,/* Attribute Length*/
                                0x00,0x37,0x2A,/* Vendor ID:WFA*/
                                0x00, /* Subelement ID:Version2*/
                                0x01, /* Subelement Length:1*/
                                0x20 /* Version2*/
    };
               
    if(NULL == pWifiTokenStruct || NULL == pData || NULL == pDataSize)
    {
        printk(KERN_ERR "NDEF_PackageWifiToken: Ivalid params!\n");
        return ERROR;
    }
    pData[0] = 0x00;
    pData[1] = 0x00;
    
    /* fill Wifi record header */
    /*NDEF Message, WiFi的recode作为第一个包，MB位为1，ME位为0*/
    pData[FIRST_RECORD_OFFSET] = 0x92;   /* Record Flag */
    pData[FIRST_RECORD_OFFSET+1] = WIFITOKEN_TYPE_STRING_LENGTH;
    pData[FIRST_RECORD_OFFSET+2] = 76; /* needs to be autocalculated - done at the end */
  
    memcpy(&pData[FIRST_RECORD_OFFSET+3], 
                WIFITOKEN_TYPE_STRING, 
                WIFITOKEN_TYPE_STRING_LENGTH);
    
    pPayload = &pData[FIRST_RECORD_OFFSET+3+WIFITOKEN_TYPE_STRING_LENGTH];
    PayloadSize = 0;
    
    
    for(initStage=0;initStage<CONFIG_TOKEN_1;initStage++)
    {
      *pPayload =configToken1[initStage];
      pPayload++;
    }
    
    //Fill SSID length and SSID value
    SSIDSize = strlen(pWifiTokenStruct->NetworkSSID);
    *pPayload = 0x00; pPayload++;
    *pPayload = SSIDSize & 0x000000FF; pPayload++;
    
    strncpy((char*)pPayload,pWifiTokenStruct->NetworkSSID,SSIDSize);
    pPayload = pPayload + strlen(pWifiTokenStruct->NetworkSSID);

    /*Select wifi authentication type .*/
    configToken3[5] = NDEF_SelectWiFiAuthenticationType(pWifiTokenStruct);
    for(initStage = 0; initStage<CONFIG_TOKEN_3; initStage++)
    {
      *pPayload =configToken3[initStage];
      pPayload++;
    }
    
  //Fill SSIDKey length and SSIDKey value
    SSIDKeySize = strlen(pWifiTokenStruct->NetworkKey);
    *pPayload = 0x00; pPayload++;
    *pPayload = SSIDKeySize & 0x000000FF; pPayload++;
    
    strncpy((char*)pPayload,pWifiTokenStruct->NetworkKey,SSIDKeySize);
    pPayload = pPayload + strlen(pWifiTokenStruct->NetworkKey);
            
    for(initStage=0; initStage<CONFIG_TOKEN_5; initStage++)
    {
      *pPayload =configToken5[initStage];
      pPayload++;
    }
    /*4是存储SSID和密码的字符长度所占的字节*/
    PayloadSize += CONFIG_TOKEN_1 + CONFIG_TOKEN_3 
                            + CONFIG_TOKEN_5 + SSIDSize + SSIDKeySize + 4;
        
    pData[FIRST_RECORD_OFFSET+2] = (PayloadSize & 0x000000FF);
    
    DataSize = PayloadSize + 5 + WIFITOKEN_TYPE_STRING_LENGTH;

    *pDataSize = DataSize;
   
    return SUCCESS;
}
#endif /*#if (FEATURE_ON == MBB_NFC)*/
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


