/**
  ******************************************************************************
  * @file    libndefuri.c
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to manage NDEF file that represent URI.
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
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "libndefuri.h"


#define M24SR_ACTION_COMPLETED                          0x9000
#define NDEF_ACTION_COMPLETED               M24SR_ACTION_COMPLETED


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
extern uint8_t NDEF_Buffer [NDEF_MAX_SIZE];

/** @defgroup libURI_Private_Functions
  * @{
  */
static void NDEF_Parse_WellKnowType(sRecordInfo *pRecordStruct, sURI_Info* pURI );

/**
  * @brief  This fonction read the URI information and store data in a structure
    * @param    pRecordStruct : Pointer on the record structure
    * @param    pURI : pointer on the structure to fill
  * @retval NONE 
  */
static void NDEF_Parse_WellKnowType(sRecordInfo *pRecordStruct, sURI_Info* pURI )
{
    u32 PayloadSize;
    u8 Offset = 0;
    u8* pPayload;
    
    pPayload = (u8*)(pRecordStruct->PayloadBufferAdd);
        
    switch( *pPayload)
    {
#if (FEATURE_ON == MBB_NFC)
        case URI_ID_0x00:
        pURI->protocol[0] = 0;
        Offset = 0;
        break;
#endif
        case URI_ID_0x01:
            memcpy( pURI->protocol, URI_ID_0x01_STRING, strlen(URI_ID_0x01_STRING));
          Offset = strlen(URI_ID_0x01_STRING);
            break;
        
        case URI_ID_0x02:
            memcpy( pURI->protocol, URI_ID_0x02_STRING, strlen(URI_ID_0x02_STRING));
          Offset = strlen(URI_ID_0x02_STRING);
            break;
        
        case URI_ID_0x03:
            memcpy( pURI->protocol, URI_ID_0x03_STRING, strlen(URI_ID_0x03_STRING));
          Offset = strlen(URI_ID_0x03_STRING);
            break;
        
        case URI_ID_0x04:
            memcpy( pURI->protocol, URI_ID_0x04_STRING, strlen(URI_ID_0x04_STRING));
          Offset = strlen(URI_ID_0x04_STRING);
            break;
        
        case URI_ID_0x05:
            memcpy( pURI->protocol, URI_ID_0x05_STRING, strlen(URI_ID_0x05_STRING));
          Offset = strlen(URI_ID_0x05_STRING);
            break;
        
        case URI_ID_0x06:
            memcpy( pURI->protocol, URI_ID_0x06_STRING, strlen(URI_ID_0x06_STRING));
          Offset = strlen(URI_ID_0x06_STRING);
            break;
        
        case URI_ID_0x07:
            memcpy( pURI->protocol, URI_ID_0x07_STRING, strlen(URI_ID_0x07_STRING));
          Offset = strlen(URI_ID_0x07_STRING);
            break;
        
        case URI_ID_0x08:
            memcpy( pURI->protocol, URI_ID_0x08_STRING, strlen(URI_ID_0x08_STRING));
          Offset = strlen(URI_ID_0x08_STRING);
            break;
        
        case URI_ID_0x09:
            memcpy( pURI->protocol, URI_ID_0x09_STRING, strlen(URI_ID_0x09_STRING));
          Offset = strlen(URI_ID_0x09_STRING);
            break;
        
        case URI_ID_0x0A:
            memcpy( pURI->protocol, URI_ID_0x0A_STRING, strlen(URI_ID_0x0A_STRING));
          Offset = strlen(URI_ID_0x0A_STRING);
            break;
        
        case URI_ID_0x0B:
            memcpy( pURI->protocol, URI_ID_0x0B_STRING, strlen(URI_ID_0x0B_STRING));
          Offset = strlen(URI_ID_0x0B_STRING);
            break;
        
        case URI_ID_0x0C:
            memcpy( pURI->protocol, URI_ID_0x0C_STRING, strlen(URI_ID_0x0C_STRING));
          Offset = strlen(URI_ID_0x0C_STRING);
            break;
        
        case URI_ID_0x0D:
            memcpy( pURI->protocol, URI_ID_0x0D_STRING, strlen(URI_ID_0x0D_STRING));
          Offset = strlen(URI_ID_0x0D_STRING);
            break;
        
        case URI_ID_0x0E:
            memcpy( pURI->protocol, URI_ID_0x0E_STRING, strlen(URI_ID_0x0E_STRING));
          Offset = strlen(URI_ID_0x0E_STRING);
            break;
        
        case URI_ID_0x0F:
            memcpy( pURI->protocol, URI_ID_0x0F_STRING, strlen(URI_ID_0x0F_STRING));
          Offset = strlen(URI_ID_0x0F_STRING);
            break;
        
        case URI_ID_0x10:
            memcpy( pURI->protocol, URI_ID_0x10_STRING, strlen(URI_ID_0x10_STRING));
          Offset = strlen(URI_ID_0x10_STRING);
            break;
        
        case URI_ID_0x11:
            memcpy( pURI->protocol, URI_ID_0x11_STRING, strlen(URI_ID_0x11_STRING));
          Offset = strlen(URI_ID_0x11_STRING);
            break;
        
        case URI_ID_0x12:
            memcpy( pURI->protocol, URI_ID_0x12_STRING, strlen(URI_ID_0x12_STRING));
          Offset = strlen(URI_ID_0x12_STRING);
            break;
        
        case URI_ID_0x13:
            memcpy( pURI->protocol, URI_ID_0x13_STRING, strlen(URI_ID_0x13_STRING));
          Offset = strlen(URI_ID_0x13_STRING);
            break;
        
        case URI_ID_0x14:
            memcpy( pURI->protocol, URI_ID_0x14_STRING, strlen(URI_ID_0x14_STRING));
          Offset = strlen(URI_ID_0x14_STRING);
            break;
        
        case URI_ID_0x15:
            memcpy( pURI->protocol, URI_ID_0x15_STRING, strlen(URI_ID_0x15_STRING));
          Offset = strlen(URI_ID_0x15_STRING);
            break;
        
        case URI_ID_0x16:
            memcpy( pURI->protocol, URI_ID_0x16_STRING, strlen(URI_ID_0x16_STRING));
          Offset = strlen(URI_ID_0x16_STRING);
            break;
        
        case URI_ID_0x17:
            memcpy( pURI->protocol, URI_ID_0x17_STRING, strlen(URI_ID_0x17_STRING));
          Offset = strlen(URI_ID_0x17_STRING);
            break;
        
        case URI_ID_0x18:
            memcpy( pURI->protocol, URI_ID_0x18_STRING, strlen(URI_ID_0x18_STRING));
          Offset = strlen(URI_ID_0x18_STRING);
            break;
        
        case URI_ID_0x19:
            memcpy( pURI->protocol, URI_ID_0x19_STRING, strlen(URI_ID_0x19_STRING));
          Offset = strlen(URI_ID_0x19_STRING);
            break;
        
        case URI_ID_0x1A:
            memcpy( pURI->protocol, URI_ID_0x1A_STRING, strlen(URI_ID_0x1A_STRING));
          Offset = strlen(URI_ID_0x1A_STRING);
            break;
        
        case URI_ID_0x1B:
            memcpy( pURI->protocol, URI_ID_0x1B_STRING, strlen(URI_ID_0x1B_STRING));
          Offset = strlen(URI_ID_0x1B_STRING);
            break;
        
        case URI_ID_0x1C:
            memcpy( pURI->protocol, URI_ID_0x1C_STRING, strlen(URI_ID_0x1C_STRING));
          Offset = strlen(URI_ID_0x1C_STRING);
            break;
        
        case URI_ID_0x1D:
            memcpy( pURI->protocol, URI_ID_0x1D_STRING, strlen(URI_ID_0x1D_STRING));
          Offset = strlen(URI_ID_0x1D_STRING);
            break;
        
        case URI_ID_0x1E:
            memcpy( pURI->protocol, URI_ID_0x1E_STRING, strlen(URI_ID_0x1E_STRING));
          Offset = strlen(URI_ID_0x1E_STRING);
            break;
        
        case URI_ID_0x1F:
            memcpy( pURI->protocol, URI_ID_0x1F_STRING, strlen(URI_ID_0x1F_STRING));
          Offset = strlen(URI_ID_0x1F_STRING);
            break;
        
        case URI_ID_0x20:
            memcpy( pURI->protocol, URI_ID_0x20_STRING, strlen(URI_ID_0x20_STRING));
          Offset = strlen(URI_ID_0x20_STRING);
            break;
        
        case URI_ID_0x21:
            memcpy( pURI->protocol, URI_ID_0x21_STRING, strlen(URI_ID_0x21_STRING));
          Offset = strlen(URI_ID_0x21_STRING);
            break;
        
        case URI_ID_0x22:
            memcpy( pURI->protocol, URI_ID_0x22_STRING, strlen(URI_ID_0x22_STRING));
          Offset = strlen(URI_ID_0x22_STRING);
            break;
        
        case URI_ID_0x23:
            memcpy( pURI->protocol, URI_ID_0x23_STRING, strlen(URI_ID_0x23_STRING));
          Offset = strlen(URI_ID_0x23_STRING);
            break;
                
        default:
            /* Should not happened */
            break;
    }
    /* add end of string charactere */
    pURI->protocol[Offset] = '\0';
    
    pPayload++; /* go after well know byte */
        
    PayloadSize = ((u32)(pRecordStruct->PayloadLength3)<<24) | ((u32)(pRecordStruct->PayloadLength2)<<16) |
                                ((u32)(pRecordStruct->PayloadLength1)<<8)  | pRecordStruct->PayloadLength0;
    
    PayloadSize = PayloadSize -1; /* remove well know byte */
    
    memcpy( pURI->URI_Message, pPayload, PayloadSize);
    /* add end of string charactere */
    pURI->URI_Message[PayloadSize] = '\0';
        
}

/**
  * @}
  */

/** @defgroup libURI_Public_Functions
  * @{
  * @brief  This file is used to manage URI (stored or loaded in tag)
  */ 

/**
  * @brief  This fonction read NDEF and retrieve URI information if any
    * @param    pRecordStruct : Pointer on the record structure
    * @param    pURI : pointer on the structure to fill 
  * @retval SUCCESS : URI information from NDEF have been retrieved
    * @retval ERROR : Not able to retrieve URI information
  */
u16 NDEF_ReadURI(sRecordInfo *pRecordStruct, sURI_Info *pURI)
{
    u16 status = ERROR;
    sRecordInfo *pSPRecordStruct;   
    u32 PayloadSize, RecordPosition;
    u8* pData;

    if( pRecordStruct->NDEF_Type == WELL_KNOWN_ABRIDGED_URI_TYPE )
    {       
        NDEF_Parse_WellKnowType(pRecordStruct, pURI );
        status = SUCCESS;
    }
    else if( pRecordStruct->NDEF_Type == SMARTPOSTER_TYPE)
    {
        for (RecordPosition = 0; RecordPosition<pRecordStruct->NbOfRecordInSPPayload; RecordPosition++)
        {
            pSPRecordStruct = (sRecordInfo *)(pRecordStruct->SPRecordStructAdd[RecordPosition]);
            if(pSPRecordStruct->NDEF_Type == WELL_KNOWN_ABRIDGED_URI_TYPE )
            {
                NDEF_Parse_WellKnowType(pSPRecordStruct, pURI );
                status = SUCCESS;
            }
            if(pSPRecordStruct->NDEF_Type == TEXT_TYPE )
            {
                PayloadSize = ((u32)(pSPRecordStruct->PayloadLength3)<<24) | ((u32)(pSPRecordStruct->PayloadLength2)<<16) |
                                        ((u32)(pSPRecordStruct->PayloadLength1)<<8)  | pSPRecordStruct->PayloadLength0;
                
                /* The instruction content the UTF-8 language code that is not used here */
                pData = (u8*)pSPRecordStruct->PayloadBufferAdd;
                pData += *pData+1;
                    
                memcpy(pURI->Information, pData, PayloadSize);
            }
        }
    }
    else 
    {
#if (FEATURE_ON == MBB_NFC)
        printk(KERN_ERR "NDEF_ReadURI Error!\n");
#endif
        status = ERROR;
    }
    
    return status;
}

/**
  * @brief  This fonction write the NDEF file with the URI data given in the structure
    * @param    pURI : pointer on structure that contain the URI information
  * @retval SUCCESS : the function is succesful
    * @retval ERROR : Not able to store NDEF file inside tag.
  */
u16 NDEF_WriteURI ( sURI_Info *pURI)
{
    u16 status = ERROR;
    u16 DataSize;
    u8* pPayload;

    
/* URI Record Header */
/************************************/  
/*  7 |  6 |  5 |  4 |  3 | 2  1  0 */
/*----------------------------------*/  
/* MB   ME   CF   SR   IL    TNF    */  /* <---- CF=0, IL=0 and SR=1 TNF=1 NFC Forum Well-known type*/
/*----------------------------------*/  
/*                  TYPE LENGTH                         */
/*----------------------------------*/
/*              PAYLOAD LENGTH 3                    */  /* <---- Not Used  */
/*----------------------------------*/
/*            PAYLOAD LENGTH 2                  */  /* <---- Not Used  */
/*----------------------------------*/
/*              PAYLOAD LENGTH 1                    */  /* <---- Not Used  */
/*----------------------------------*/  
/*              PAYLOAD LENGTH 0                    */  
/*----------------------------------*/
/*                  ID LENGTH                           */  /* <---- Not Used  */
/*----------------------------------*/
/*                          TYPE                                */
/*----------------------------------*/
/*                           ID                 */  /* <---- Not Used  */ 
/************************************/
    
    /* NDEF file must be written in 2 phases, first phase NDEF size is Null */
    NDEF_Buffer[NDEF_SIZE_OFFSET] = 0x00;
    NDEF_Buffer[NDEF_SIZE_OFFSET+1] = 0x00;
    
    /* fill URI record header */
    NDEF_Buffer[FIRST_RECORD_OFFSET] = 0xD1;   /* Record Flag */
    NDEF_Buffer[FIRST_RECORD_OFFSET+1] = URI_TYPE_STRING_LENGTH;
    NDEF_Buffer[FIRST_RECORD_OFFSET+2] = 0x00; /* Will be filled at the end when payload size is known */

    memcpy(&NDEF_Buffer[FIRST_RECORD_OFFSET+3], URI_TYPE_STRING, URI_TYPE_STRING_LENGTH);
    
    pPayload = &NDEF_Buffer[FIRST_RECORD_OFFSET+4];
        
    if(!memcmp( pURI->protocol, URI_ID_0x01_STRING, strlen(URI_ID_0x01_STRING)))
        *pPayload = URI_ID_0x01;
    else if(!memcmp( pURI->protocol, URI_ID_0x02_STRING, strlen(URI_ID_0x02_STRING)))
        *pPayload = URI_ID_0x02;
    else if(!memcmp( pURI->protocol, URI_ID_0x03_STRING, strlen(URI_ID_0x03_STRING)))
        *pPayload = URI_ID_0x03;
    else if(!memcmp( pURI->protocol, URI_ID_0x04_STRING, strlen(URI_ID_0x04_STRING)))
        *pPayload = URI_ID_0x04;
    else if(!memcmp( pURI->protocol, URI_ID_0x05_STRING, strlen(URI_ID_0x05_STRING)))
        *pPayload = URI_ID_0x05;
    else if(!memcmp( pURI->protocol, URI_ID_0x06_STRING, strlen(URI_ID_0x06_STRING)))
        *pPayload = URI_ID_0x06;
    else if(!memcmp( pURI->protocol, URI_ID_0x07_STRING, strlen(URI_ID_0x07_STRING)))
        *pPayload = URI_ID_0x07;
    else if(!memcmp( pURI->protocol, URI_ID_0x08_STRING, strlen(URI_ID_0x08_STRING)))
        *pPayload = URI_ID_0x08;
    else if(!memcmp( pURI->protocol, URI_ID_0x09_STRING, strlen(URI_ID_0x09_STRING)))
        *pPayload = URI_ID_0x09;
    else if(!memcmp( pURI->protocol, URI_ID_0x0A_STRING, strlen(URI_ID_0x0A_STRING)))
        *pPayload = URI_ID_0x0A;    
    else if(!memcmp( pURI->protocol, URI_ID_0x0B_STRING, strlen(URI_ID_0x0B_STRING)))
        *pPayload = URI_ID_0x0B;
    else if(!memcmp( pURI->protocol, URI_ID_0x0C_STRING, strlen(URI_ID_0x0C_STRING)))
        *pPayload = URI_ID_0x0C;
    else if(!memcmp( pURI->protocol, URI_ID_0x0D_STRING, strlen(URI_ID_0x0D_STRING)))
        *pPayload = URI_ID_0x0D;
    else if(!memcmp( pURI->protocol, URI_ID_0x0E_STRING, strlen(URI_ID_0x0E_STRING)))
        *pPayload = URI_ID_0x0E;
    else if(!memcmp( pURI->protocol, URI_ID_0x0F_STRING, strlen(URI_ID_0x0F_STRING)))
        *pPayload = URI_ID_0x0F;
    else if(!memcmp( pURI->protocol, URI_ID_0x10_STRING, strlen(URI_ID_0x10_STRING)))
        *pPayload = URI_ID_0x10;
    else if(!memcmp( pURI->protocol, URI_ID_0x11_STRING, strlen(URI_ID_0x11_STRING)))
        *pPayload = URI_ID_0x11;
    else if(!memcmp( pURI->protocol, URI_ID_0x12_STRING, strlen(URI_ID_0x12_STRING)))
        *pPayload = URI_ID_0x12;
    else if(!memcmp( pURI->protocol, URI_ID_0x13_STRING, strlen(URI_ID_0x13_STRING)))
        *pPayload = URI_ID_0x13;
    else if(!memcmp( pURI->protocol, URI_ID_0x14_STRING, strlen(URI_ID_0x14_STRING)))
        *pPayload = URI_ID_0x14;
    else if(!memcmp( pURI->protocol, URI_ID_0x15_STRING, strlen(URI_ID_0x15_STRING)))
        *pPayload = URI_ID_0x15;
    else if(!memcmp( pURI->protocol, URI_ID_0x16_STRING, strlen(URI_ID_0x16_STRING)))
        *pPayload = URI_ID_0x16;
    else if(!memcmp( pURI->protocol, URI_ID_0x17_STRING, strlen(URI_ID_0x17_STRING)))
        *pPayload = URI_ID_0x17;
    else if(!memcmp( pURI->protocol, URI_ID_0x18_STRING, strlen(URI_ID_0x18_STRING)))
        *pPayload = URI_ID_0x18;
    else if(!memcmp( pURI->protocol, URI_ID_0x19_STRING, strlen(URI_ID_0x19_STRING)))
        *pPayload = URI_ID_0x19;
    else if(!memcmp( pURI->protocol, URI_ID_0x1A_STRING, strlen(URI_ID_0x1A_STRING)))
        *pPayload = URI_ID_0x1A;
    else if(!memcmp( pURI->protocol, URI_ID_0x1B_STRING, strlen(URI_ID_0x1B_STRING)))
        *pPayload = URI_ID_0x1B;
    else if(!memcmp( pURI->protocol, URI_ID_0x1C_STRING, strlen(URI_ID_0x1C_STRING)))
        *pPayload = URI_ID_0x1C;
    else if(!memcmp( pURI->protocol, URI_ID_0x1D_STRING, strlen(URI_ID_0x1D_STRING)))
        *pPayload = URI_ID_0x1D;
    else if(!memcmp( pURI->protocol, URI_ID_0x1E_STRING, strlen(URI_ID_0x1E_STRING)))
        *pPayload = URI_ID_0x1E;
    else if(!memcmp( pURI->protocol, URI_ID_0x1F_STRING, strlen(URI_ID_0x1F_STRING)))
        *pPayload = URI_ID_0x1F;
    else if(!memcmp( pURI->protocol, URI_ID_0x20_STRING, strlen(URI_ID_0x20_STRING)))
        *pPayload = URI_ID_0x20;
    else if(!memcmp( pURI->protocol, URI_ID_0x21_STRING, strlen(URI_ID_0x21_STRING)))
        *pPayload = URI_ID_0x21;
    else if(!memcmp( pURI->protocol, URI_ID_0x22_STRING, strlen(URI_ID_0x22_STRING)))
        *pPayload = URI_ID_0x22;
    else if(!memcmp( pURI->protocol, URI_ID_0x23_STRING, strlen(URI_ID_0x23_STRING)))
        *pPayload = URI_ID_0x23;    
    else
    {
        /* not managed */
    }
    
    pPayload++;
    
    memcpy(pPayload, pURI->URI_Message, strlen(pURI->URI_Message));
    
    NDEF_Buffer[FIRST_RECORD_OFFSET+2] =  strlen(pURI->URI_Message) +1;
    
    DataSize = strlen(pURI->URI_Message) +7;
    
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
  * @brief  This fonction write the NDEF file with the URI data given in the structure
    * @param    pURI : pointer on structure that contain the URI information
  * @retval SUCCESS : the function is succesful
    * @retval ERROR : Not able to store NDEF file inside tag.
  */
u16 NDEF_PackageWriteURI ( sURI_Info *pURI, u8* pData,u16 *pDataSize)
{
    u16 DataSize;
    u8* pPayload;

    
/* URI Record Header */
/************************************/  
/*  7 |  6 |  5 |  4 |  3 | 2  1  0 */
/*----------------------------------*/  
/* MB   ME   CF   SR   IL    TNF    */  /* <---- CF=0, IL=0 and SR=1 TNF=1 NFC Forum Well-known type*/
/*----------------------------------*/  
/*                  TYPE LENGTH                         */
/*----------------------------------*/
/*              PAYLOAD LENGTH 3                    */  /* <---- Not Used  */
/*----------------------------------*/
/*            PAYLOAD LENGTH 2                  */  /* <---- Not Used  */
/*----------------------------------*/
/*              PAYLOAD LENGTH 1                    */  /* <---- Not Used  */
/*----------------------------------*/  
/*              PAYLOAD LENGTH 0                    */  
/*----------------------------------*/
/*                  ID LENGTH                           */  /* <---- Not Used  */
/*----------------------------------*/
/*                          TYPE                                */
/*----------------------------------*/
/*                           ID                 */  /* <---- Not Used  */ 
/************************************/
    if(NULL == pURI || NULL == pData || NULL == pDataSize)
    {
        printk(KERN_ERR "NDEF_PackageWriteURI: Ivalid params!\n");
        return ERROR;
    }
    /* NDEF file must be written in 2 phases, first phase NDEF size is Null */
    pData[NDEF_SIZE_OFFSET] = 0x00;
    pData[NDEF_SIZE_OFFSET+1] = 0x00;
    
    /* fill URI record header */
    /*URL的recode作为第二个包，也是最后一个，MB位为0，ME位为1*/
    pData[FIRST_RECORD_OFFSET] = 0x51;   /* Record Flag */
    pData[FIRST_RECORD_OFFSET+1] = URI_TYPE_STRING_LENGTH;
    pData[FIRST_RECORD_OFFSET+2] = 0x00; /* Will be filled at the end when payload size is known */

    memcpy(&pData[FIRST_RECORD_OFFSET+3], URI_TYPE_STRING, URI_TYPE_STRING_LENGTH);
    
    pPayload = &pData[FIRST_RECORD_OFFSET+4];
        
    if(!memcmp( pURI->protocol, URI_ID_0x01_STRING, strlen(URI_ID_0x01_STRING)))
    {
        *pPayload = URI_ID_0x01;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x02_STRING, strlen(URI_ID_0x02_STRING)))
    {
        *pPayload = URI_ID_0x02;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x03_STRING, strlen(URI_ID_0x03_STRING)))
    {
        *pPayload = URI_ID_0x03;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x04_STRING, strlen(URI_ID_0x04_STRING)))
    {
        *pPayload = URI_ID_0x04;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x05_STRING, strlen(URI_ID_0x05_STRING)))
    {   
        *pPayload = URI_ID_0x05;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x06_STRING, strlen(URI_ID_0x06_STRING)))
    {
        *pPayload = URI_ID_0x06;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x07_STRING, strlen(URI_ID_0x07_STRING)))
    {
        *pPayload = URI_ID_0x07;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x08_STRING, strlen(URI_ID_0x08_STRING)))
    {   
        *pPayload = URI_ID_0x08;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x09_STRING, strlen(URI_ID_0x09_STRING)))
    {
        *pPayload = URI_ID_0x09;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x0A_STRING, strlen(URI_ID_0x0A_STRING)))
    {   
        *pPayload = URI_ID_0x0A;    
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x0B_STRING, strlen(URI_ID_0x0B_STRING)))
    {
        *pPayload = URI_ID_0x0B;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x0C_STRING, strlen(URI_ID_0x0C_STRING)))
    {
        *pPayload = URI_ID_0x0C;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x0D_STRING, strlen(URI_ID_0x0D_STRING)))
    {
        *pPayload = URI_ID_0x0D;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x0E_STRING, strlen(URI_ID_0x0E_STRING)))
    {
        *pPayload = URI_ID_0x0E;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x0F_STRING, strlen(URI_ID_0x0F_STRING)))
    {
        *pPayload = URI_ID_0x0F;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x10_STRING, strlen(URI_ID_0x10_STRING)))
    {
        *pPayload = URI_ID_0x10;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x11_STRING, strlen(URI_ID_0x11_STRING)))
    {
        *pPayload = URI_ID_0x11;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x12_STRING, strlen(URI_ID_0x12_STRING)))
    {
        *pPayload = URI_ID_0x12;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x13_STRING, strlen(URI_ID_0x13_STRING)))
    {
        *pPayload = URI_ID_0x13;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x14_STRING, strlen(URI_ID_0x14_STRING)))
    {
        *pPayload = URI_ID_0x14;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x15_STRING, strlen(URI_ID_0x15_STRING)))
    {
        *pPayload = URI_ID_0x15;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x16_STRING, strlen(URI_ID_0x16_STRING)))
    {
        *pPayload = URI_ID_0x16;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x17_STRING, strlen(URI_ID_0x17_STRING)))
    {
        *pPayload = URI_ID_0x17;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x18_STRING, strlen(URI_ID_0x18_STRING)))
    {
        *pPayload = URI_ID_0x18;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x19_STRING, strlen(URI_ID_0x19_STRING)))
    {
        *pPayload = URI_ID_0x19;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x1A_STRING, strlen(URI_ID_0x1A_STRING)))
    {
        *pPayload = URI_ID_0x1A;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x1B_STRING, strlen(URI_ID_0x1B_STRING)))
    {
        *pPayload = URI_ID_0x1B;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x1C_STRING, strlen(URI_ID_0x1C_STRING)))
    {
        *pPayload = URI_ID_0x1C;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x1D_STRING, strlen(URI_ID_0x1D_STRING)))
    {
        *pPayload = URI_ID_0x1D;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x1E_STRING, strlen(URI_ID_0x1E_STRING)))
    {
        *pPayload = URI_ID_0x1E;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x1F_STRING, strlen(URI_ID_0x1F_STRING)))
    {
        *pPayload = URI_ID_0x1F;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x20_STRING, strlen(URI_ID_0x20_STRING)))
    {
        *pPayload = URI_ID_0x20;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x21_STRING, strlen(URI_ID_0x21_STRING)))
    {
        *pPayload = URI_ID_0x21;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x22_STRING, strlen(URI_ID_0x22_STRING)))
    {
        *pPayload = URI_ID_0x22;
    }
    else if(!memcmp( pURI->protocol, URI_ID_0x23_STRING, strlen(URI_ID_0x23_STRING)))
    {
        *pPayload = URI_ID_0x23; 
    }
    else
    {
        /* not managed */
    }
    
    pPayload++;
    
    memcpy(pPayload, pURI->URI_Message, strlen(pURI->URI_Message));

    pData[FIRST_RECORD_OFFSET+2] =  strlen(pURI->URI_Message) + 1; /* add well know byte */

    DataSize = strlen(pURI->URI_Message) +7;

    *pDataSize = DataSize;
    return SUCCESS;

}
#endif/*#if (FEATURE_ON == MBB_NFC)*/

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




