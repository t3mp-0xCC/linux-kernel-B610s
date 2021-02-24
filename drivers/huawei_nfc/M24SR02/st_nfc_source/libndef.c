/**
  ******************************************************************************
  * @file    lib_NDEF.c
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to manage NDEF file, to parse and identify them.
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
/**********************���ⵥ�޸ļ�¼******************************************
��    ��              �޸���         ���ⵥ��           �޸�����
******************************************************************************/
/* Includes ------------------------------------------------------------------*/

#include <mbb_config.h>

#if (FEATURE_ON == MBB_NFC)
#include <linux/string.h>
#include <linux/kernel.h>
#else
#include "string.h"
#endif
#include "libndef.h"
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

/** @defgroup libNDEF_Private_Functions
  * @{
  */



static u16 NDEF_IsNDEFPresent ( void );
static u16 NDEF_ParseRecordHeader ( sRecordInfo *pRecordStruct );
static void NDEF_ParseWellKnownType ( sRecordInfo *pRecordStruct );
static void NDEF_ParseMediaType ( sRecordInfo *pRecordStruct );
static void NDEF_ParseForumExternalType ( sRecordInfo *pRecordStruct );
static void NDEF_ParseURI(sRecordInfo *pRecordStruct);
static void NDEF_ParseSP(sRecordInfo *pRecordStruct);
static u16 NDEF_IdentifySPRecord ( sRecordInfo *pRecordStruct, u8* pPayload );

/* In case of smart Poster composed with different record, 3 records supported so far */
sRecordInfo SPRecordStruct1, SPRecordStruct2, SPRecordStruct3;
u32 SPRecordStructAdd[SP_MAX_RECORD];
// = {(u32)&SPRecordStruct1, (u32)&SPRecordStruct2, (u32)&SPRecordStruct3 };

/**
  * @brief  This fonction check that the tag contain a NDEF message
    * @param    None :
  * @retval SUCCESS : There is a NDEF file stored in tag
    * @retval ERROR : No NDEF in the tag. 
  */
static u16 NDEF_IsNDEFPresent ( void )
{
    u16 FileSize;
    u8 uM24SR_NDEFHeader [0x2] = {0};
    
    /* Check NDEF existence */
    ReadData ( NDEF_SIZE_OFFSET , 2 , uM24SR_NDEFHeader);
    FileSize = (u16) ((uM24SR_NDEFHeader[0x00]<<8) | uM24SR_NDEFHeader[0x01]);
    
    if( FileSize != 0 )
        return SUCCESS;
    else
        return ERROR;

}

/**
  * @brief  This fonction identify the type of record
    * @param    pRecordStruct : pointer on the record structure to fill
    * @param    pPayload : pointer on the payload
    * @retval Status : Status of the operation.
  */
static u16 NDEF_IdentifySPRecord ( sRecordInfo *pRecordStruct, u8* pPayload )
{
    u16 status = ERROR;
    u16 SizeOfRecordHeader, TypeNbByte, PayloadLengthField, IDLengthField, IDNbByte;
        
    /* Is ID length field present */
    if( (*pPayload)&IL_Mask)
    {
        IDLengthField = ID_LENGTH_FIELD;
    }
    else
    {
        IDLengthField = 0;
    }
    
    /* it's a SR */
    if( (*pPayload)&SR_Mask)
    {
        TypeNbByte = pPayload[1];
        PayloadLengthField = 1;
        if( IDLengthField == ID_LENGTH_FIELD)
            IDNbByte = pPayload[3];

        else
            IDNbByte = 0;

    }
    else
    {
        TypeNbByte = pPayload[1];
        PayloadLengthField = 4;
        if( IDLengthField == ID_LENGTH_FIELD)
             IDNbByte = pPayload[6];

        else
            IDNbByte = 0;
    }
        
    SizeOfRecordHeader = RECORD_FLAG_FIELD + TYPE_LENGTH_FIELD + PayloadLengthField + IDLengthField + TypeNbByte + IDNbByte;
    
    /* it's a SR */
    if( pPayload[0]&SR_Mask)
    {
        pRecordStruct->RecordFlags = pPayload[0];
        pRecordStruct->TypeLength = TypeNbByte;
        pRecordStruct->PayloadLength3 = 0;
        pRecordStruct->PayloadLength2 = 0;
        pRecordStruct->PayloadLength1 = 0;
        pRecordStruct->PayloadLength0 = pPayload[2];
        pRecordStruct->IDLength = IDNbByte;
        memcpy(pRecordStruct->Type, &pPayload[3+IDNbByte] , TypeNbByte);
        memcpy(pRecordStruct->ID, &pPayload[3+IDNbByte+TypeNbByte] , IDNbByte);
        pRecordStruct->PayloadOffset = SizeOfRecordHeader;
    }
    else
    {
        pRecordStruct->RecordFlags = pPayload[0];
        pRecordStruct->TypeLength = TypeNbByte;
        pRecordStruct->PayloadLength3 = pPayload[2];
        pRecordStruct->PayloadLength2 = pPayload[3];
        pRecordStruct->PayloadLength1 = pPayload[4];
        pRecordStruct->PayloadLength0 = pPayload[5];
        pRecordStruct->IDLength = IDNbByte;
        memcpy(pRecordStruct->Type, &pPayload[6+IDNbByte] , TypeNbByte);
        memcpy(pRecordStruct->ID, &pPayload[6+IDNbByte+TypeNbByte] , IDNbByte);
        pRecordStruct->PayloadOffset = SizeOfRecordHeader;
    }
    
    pRecordStruct->PayloadBufferAdd = (u32)(pPayload+SizeOfRecordHeader);
    
    status = NDEF_ParseRecordHeader(pRecordStruct);
    
    return status;
}

/**
  * @brief  This fonction parse the record header and dispatch regarding TNF value
    * @param    pRecordStruct : pointer on the record structure to fill
  * @retval SUCCESS : record identified and structure filled
    * @retval ERROR : Not supported 
  */
static u16 NDEF_ParseRecordHeader ( sRecordInfo *pRecordStruct )
{ 
    u16 status = SUCCESS;
    
    switch( (pRecordStruct->RecordFlags&TNF_Mask))
    {
        case TNF_WellKnown:
            NDEF_ParseWellKnownType ( pRecordStruct );
            break;          
        
        case TNF_MediaType:
            NDEF_ParseMediaType ( pRecordStruct );
            break;
        
        case TNF_NFCForumExternal:
            NDEF_ParseForumExternalType ( pRecordStruct);
            break;
        
        default:
            /* currently not supported or unknown*/
            pRecordStruct->NDEF_Type = UNKNOWN_TYPE;
         status = ERROR;
    }
    return status;
}   

/**
  * @brief  This fonction parse the Well Known type record
    * @param    pRecordStruct : pointer on the record structure to fill
  * @retval None 
  */
static void NDEF_ParseWellKnownType ( sRecordInfo *pRecordStruct )
{
    u8* pPayload;
    
    pPayload = (u8*)(pRecordStruct->PayloadBufferAdd);
    if( !memcmp( &(pRecordStruct->Type), SMART_POSTER_TYPE_STRING, pRecordStruct->TypeLength))
    { 
        /* special case where   we have to parse others records */
        pRecordStruct->NDEF_Type = SMARTPOSTER_TYPE;
        NDEF_ParseSP(pRecordStruct);
    }

    else if( !memcmp( &(pRecordStruct->Type), URI_TYPE_STRING, pRecordStruct->TypeLength))
    { 
        /* it's an URI Type check if it's an URL or SMS or ... */
        /* check identifier */
        if( *pPayload == URI_ID_0x00) 
        {   
            NDEF_ParseURI(pRecordStruct);
        }
        else if ( *pPayload > URI_ID_0x00 && *pPayload < URI_RFU ) 
        {
            /* email special case  */
            if ( *pPayload == (u8) URI_ID_0x06)
            {
                pRecordStruct->NDEF_Type = URI_EMAIL_TYPE;
            }
            else
            {
                pRecordStruct->NDEF_Type = WELL_KNOWN_ABRIDGED_URI_TYPE;
            }
        }
        else
        {   
            pRecordStruct->NDEF_Type = UNKNOWN_TYPE;
        }
    }       
    
    else if( !memcmp( &(pRecordStruct->Type), TEXT_TYPE_STRING, pRecordStruct->TypeLength))
    {
        pRecordStruct->NDEF_Type = TEXT_TYPE;
    }       
    else
    pRecordStruct->NDEF_Type = UNKNOWN_TYPE;

}

/**
  * @brief  This fonction parse the Media type record
    * @param    pRecordStruct : pointer on the record structure to fill
  * @retval None 
  */
#if (FEATURE_ON == MBB_NFC)
static void NDEF_ParseMediaType ( sRecordInfo *pRecordStruct )
{
    if(NULL == pRecordStruct)
    {
        printk(KERN_ERR "%s (%d) : Invalid Params !\n",__func__,__LINE__);
        return;
    }
    
    if( !memcmp( &(pRecordStruct->Type), 
                        WIFITOKEN_TYPE_STRING, pRecordStruct->TypeLength))
    { 
        pRecordStruct->NDEF_Type = URI_WIFITOKEN_TYPE; 
    }
    else if( !memcmp( &(pRecordStruct->Type), 
                        VCARD_TYPE_STRING, pRecordStruct->TypeLength))
    { 
        pRecordStruct->NDEF_Type = VCARD_TYPE; 
    }
    else if( !memcmp( &(pRecordStruct->Type), 
                        XVCARD_TYPE_STRING, pRecordStruct->TypeLength))
    { 
        pRecordStruct->NDEF_Type = VCARD_TYPE; 
    }
    else
    { 
        pRecordStruct->NDEF_Type = UNKNOWN_TYPE; 
    }
}
#else
static void NDEF_ParseMediaType ( sRecordInfo *pRecordStruct )
{   
    if( !memcmp( &(pRecordStruct->Type), VCARD_TYPE_STRING, pRecordStruct->TypeLength))
        pRecordStruct->NDEF_Type = VCARD_TYPE;
    else if( !memcmp( &(pRecordStruct->Type), XVCARD_TYPE_STRING, pRecordStruct->TypeLength))
        pRecordStruct->NDEF_Type = VCARD_TYPE;
    else
     pRecordStruct->NDEF_Type = UNKNOWN_TYPE;
}
#endif


/**
  * @brief  This fonction parse the Forum External type record
    * @param    pRecordStruct : pointer on the record structure to fill
  * @retval None 
  */
static void NDEF_ParseForumExternalType ( sRecordInfo *pRecordStruct )
{
    if( !memcmp( &(pRecordStruct->Type), M24SR_DISCOVERY_APP_STRING, pRecordStruct->TypeLength))
        pRecordStruct->NDEF_Type = M24SR_DISCOVERY_APP_TYPE;
    else
        pRecordStruct->NDEF_Type = UNKNOWN_TYPE;
}

/**
  * @brief  This fonction parse the URI type record
    * @param    pRecordStruct : pointer on the record structure to fill
  * @retval None 
  */
static void NDEF_ParseURI(sRecordInfo *pRecordStruct)
{
    u8* pPayload;
    
    pPayload = (u8*)(pRecordStruct->PayloadBufferAdd);
    pPayload ++; /* to skip URI identifier first URI payload byte */
    
    if( !memcmp( pPayload, SMS_TYPE_STRING, strlen(SMS_TYPE_STRING)) )
    { 
        pRecordStruct->NDEF_Type = URI_SMS_TYPE;
    }
    else if( !memcmp( pPayload, GEO_TYPE_STRING, strlen(GEO_TYPE_STRING)) )
    { 
        pRecordStruct->NDEF_Type = URI_GEO_TYPE;
    }
    else
        pRecordStruct->NDEF_Type = UNKNOWN_TYPE;
    
}

/**
  * @brief  This fonction parse the Smart Poster 
    * @param    pRecordStruct : pointer on the record structure to fill
  * @retval None 
  */
static void NDEF_ParseSP(sRecordInfo *pRecordStruct)
{
    u8* pPayload;
    u32 PayloadSize = 0;
    u32 SPPayloadSize = 0;
    u32 OffsetInSPPayload =0;
    u32 RecordPosition = 0;
    sRecordInfo *pSPRecordStruct;
    
    /* initialize variable with size of the payload and poiter on data */
    PayloadSize = ((u32)(pRecordStruct->PayloadLength3)<<24) | ((u32)(pRecordStruct->PayloadLength2)<<16) |
                                ((u32)(pRecordStruct->PayloadLength1)<<8)  | pRecordStruct->PayloadLength0;
    
    pPayload = (u8*)(pRecordStruct->PayloadBufferAdd);
    
    pSPRecordStruct = (sRecordInfo *)SPRecordStructAdd[0];
    
    /* Initailize the number of record find in the SP payload */
    pRecordStruct->NbOfRecordInSPPayload = 0;
    
    do
    {
        pSPRecordStruct = (sRecordInfo *)SPRecordStructAdd[RecordPosition];
        /* identify the 1st record in the SP payload */
        if( NDEF_IdentifySPRecord ( pSPRecordStruct, pPayload ))
        {
            /* store add of structure that will contain the other record information */
            pRecordStruct->NbOfRecordInSPPayload ++;
            pRecordStruct->SPRecordStructAdd[RecordPosition] = (u32) pSPRecordStruct;
            
            /* After SPRecord + First Record check if we are at the end of NDEF file */
            SPPayloadSize = ((u32)(pSPRecordStruct->PayloadLength3)<<24) | ((u32)(pSPRecordStruct->PayloadLength2)<<16) |
                                            ((u32)(pSPRecordStruct->PayloadLength1)<<8)  | pSPRecordStruct->PayloadLength0;
    
            OffsetInSPPayload += pSPRecordStruct->PayloadOffset + SPPayloadSize;
            pPayload += OffsetInSPPayload;
            RecordPosition++;       
        }
    }
    while(  OffsetInSPPayload < PayloadSize && RecordPosition<SP_MAX_RECORD); /* there is another record */
        
}

/**
  * @}
  */

/** @defgroup libNDEF_Public_Functions
  * @{
  */ 


/**
  * @brief  This fonction identify the NDEF message stored in tag
    * @param    pRecordStruct : Structure to fill with record information
    * @param    pNDEF : pointer on the NDEF message data
  * @retval SUCCESS : record struct filled
    * @retval ERROR : record struct not updated
  */
u16 NDEF_IdentifyNDEF ( sRecordInfo *pRecordStruct, u8* pNDEF )
{
    u16 status = ERROR;
    u16 SizeOfRecordHeader, TypeNbByte, PayloadLengthField, IDLengthField, IDNbByte;
    u32 PayloadSize;
    
    /* check NDEF present */
    if(NDEF_IsNDEFPresent() != SUCCESS)
    {
        return ERROR;
    }
    
    /* Analyse record layout */
    ReadData ( FIRST_RECORD_OFFSET , 1 , pNDEF);
    
    /* Is ID length field present */
    if( (*pNDEF)&IL_Mask)
    {
        IDLengthField = ID_LENGTH_FIELD;
    }
    else
    {
        IDLengthField = 0;
    }
    
    /* it's a SR */
    if( (*pNDEF)&SR_Mask)
    {
        /* Analyse short record layout */
        ReadData ( FIRST_RECORD_OFFSET , 4 , pNDEF);
        TypeNbByte = pNDEF[1];
        PayloadLengthField = 1;
        if( IDLengthField == ID_LENGTH_FIELD)
            IDNbByte = pNDEF[3];
        else
            IDNbByte = 0;
    }
    else
    {
        /* Analyse normal record layout */
        ReadData ( FIRST_RECORD_OFFSET , 7 , pNDEF);
        TypeNbByte = pNDEF[1];
        PayloadLengthField = 4;
        if( IDLengthField == ID_LENGTH_FIELD)
            IDNbByte = pNDEF[6];
        else
            IDNbByte = 0;
    }
        
    SizeOfRecordHeader = RECORD_FLAG_FIELD + TYPE_LENGTH_FIELD + PayloadLengthField + IDLengthField + TypeNbByte + IDNbByte;
    
    /* Read record header */
    ReadData ( FIRST_RECORD_OFFSET , SizeOfRecordHeader , pNDEF);
    /* it's a SR */
    if( pNDEF[0]&SR_Mask)
    {
        pRecordStruct->RecordFlags = pNDEF[0];
        pRecordStruct->TypeLength = TypeNbByte;
        pRecordStruct->PayloadLength3 = 0;
        pRecordStruct->PayloadLength2 = 0;
        pRecordStruct->PayloadLength1 = 0;
        pRecordStruct->PayloadLength0 = pNDEF[2];
        pRecordStruct->IDLength = IDNbByte;
        memcpy(pRecordStruct->Type, &pNDEF[3+IDNbByte] , TypeNbByte);
        memcpy(pRecordStruct->ID, &pNDEF[3+IDNbByte+TypeNbByte] , IDNbByte);
        pRecordStruct->PayloadOffset = SizeOfRecordHeader;
    }
    else
    {
        pRecordStruct->RecordFlags = pNDEF[0];
        pRecordStruct->TypeLength = TypeNbByte;
        pRecordStruct->PayloadLength3 = pNDEF[2];
        pRecordStruct->PayloadLength2 = pNDEF[3];
        pRecordStruct->PayloadLength1 = pNDEF[4];
        pRecordStruct->PayloadLength0 = pNDEF[5];
        pRecordStruct->IDLength = IDNbByte;
        memcpy(pRecordStruct->Type, &pNDEF[6+IDNbByte] , TypeNbByte);
        memcpy(pRecordStruct->ID, &pNDEF[6+IDNbByte+TypeNbByte] , IDNbByte);
        pRecordStruct->PayloadOffset = SizeOfRecordHeader;
    }
        
    PayloadSize = ((u32)(pRecordStruct->PayloadLength3)<<24) | ((u32)(pRecordStruct->PayloadLength2)<<16) |
                                ((u32)(pRecordStruct->PayloadLength1)<<8)  | pRecordStruct->PayloadLength0;
    
    /* read Payload */
    status = ReadData ( (u16)((FIRST_RECORD_OFFSET) + pRecordStruct->PayloadOffset) , PayloadSize , pNDEF);
    
    if( status != NDEF_ACTION_COMPLETED)
        return status;
    else
        pRecordStruct->PayloadBufferAdd = (u32)(pNDEF);

    NDEF_ParseRecordHeader(pRecordStruct);
    
    return SUCCESS;
}   
#if (FEATURE_ON == MBB_NFC)
/**
  * @brief  This function identify the NDEF message stored in tag
    * @param    pRecordStruct : Structure to fill with record information
    * @param    pNDEF : pointer on the NDEF message data
  * @retval SUCCESS : record struct filled
    * @retval ERROR : record struct not updated
  */
u16 NDEF_IdentifyNDEFByOffset (  u16 Offset,sRecordInfo *pRecordStruct, u8* pNDEF )
{
    u16 status = ERROR;
    u16 SizeOfRecordHeader, TypeNbByte, PayloadLengthField, IDLengthField, IDNbByte;
    u32 PayloadSize;

    if(NULL == pRecordStruct || NULL == pNDEF)
    {
        printk(KERN_ERR "%s (%d) : Invalid Params !\n",__func__,__LINE__);
        return ERROR;
    }
    /* check NDEF present */
    if(NDEF_IsNDEFPresent() != SUCCESS)
    {
        return ERROR;
    }
    
    /* Analyse record layout */
    ReadData ( Offset , 1 , pNDEF);
    
    /* Is ID length field present */
    if( (*pNDEF)&IL_Mask)
    {
        IDLengthField = ID_LENGTH_FIELD;
    }
    else
    {
        IDLengthField = 0;
    }
    
    /* it's a SR */
    if( (*pNDEF)&SR_Mask)
    {
        /* Analyse short record layout */
        ReadData ( Offset , 4 , pNDEF);
        TypeNbByte = pNDEF[1];
        PayloadLengthField = 1;
        if( IDLengthField == ID_LENGTH_FIELD)
        { 
            IDNbByte = pNDEF[3]; 
        }
        else
        { 
            IDNbByte = 0; 
        }
    }
    else
    {
        /* Analyse normal record layout */
        ReadData ( Offset , 7 , pNDEF);
        TypeNbByte = pNDEF[1];
        PayloadLengthField = 4;
        if( IDLengthField == ID_LENGTH_FIELD)
        { 
            IDNbByte = pNDEF[6]; 
        }
        else
        { 
            IDNbByte = 0; 
        }
    }
        
    SizeOfRecordHeader = RECORD_FLAG_FIELD + TYPE_LENGTH_FIELD
            + PayloadLengthField + IDLengthField + TypeNbByte + IDNbByte;
    
    /* Read record header */
    ReadData ( Offset , SizeOfRecordHeader , pNDEF);
    /* it's a SR */
    if( pNDEF[0]&SR_Mask)
    {
        pRecordStruct->RecordFlags = pNDEF[0];
        pRecordStruct->TypeLength = TypeNbByte;
        pRecordStruct->PayloadLength3 = 0;
        pRecordStruct->PayloadLength2 = 0;
        pRecordStruct->PayloadLength1 = 0;
        pRecordStruct->PayloadLength0 = pNDEF[2];
        pRecordStruct->IDLength = IDNbByte;
        memcpy(pRecordStruct->Type, &pNDEF[3+IDNbByte] , TypeNbByte);
        memcpy(pRecordStruct->ID, &pNDEF[3+IDNbByte+TypeNbByte] , IDNbByte);
        pRecordStruct->PayloadOffset = SizeOfRecordHeader;
    }
    else
    {
        pRecordStruct->RecordFlags = pNDEF[0];
        pRecordStruct->TypeLength = TypeNbByte;
        pRecordStruct->PayloadLength3 = pNDEF[2];
        pRecordStruct->PayloadLength2 = pNDEF[3];
        pRecordStruct->PayloadLength1 = pNDEF[4];
        pRecordStruct->PayloadLength0 = pNDEF[5];
        pRecordStruct->IDLength = IDNbByte;
        memcpy(pRecordStruct->Type, &pNDEF[6+IDNbByte] , TypeNbByte);
        memcpy(pRecordStruct->ID, &pNDEF[6+IDNbByte+TypeNbByte] , IDNbByte);
        pRecordStruct->PayloadOffset = SizeOfRecordHeader;
    }
        
    PayloadSize = ((u32)(pRecordStruct->PayloadLength3)<<24) 
                    | ((u32)(pRecordStruct->PayloadLength2)<<16) 
                    | ((u32)(pRecordStruct->PayloadLength1)<<8)  
                    | pRecordStruct->PayloadLength0;
    /* read Payload */
    status = ReadData ( (u16)((Offset) + pRecordStruct->PayloadOffset) , 
                        PayloadSize , 
                        pNDEF);
    
    if( status != NDEF_ACTION_COMPLETED)
    { return status; }
    else
    { pRecordStruct->PayloadBufferAdd = (u32)(pNDEF); }

    NDEF_ParseRecordHeader(pRecordStruct);
    
    return SUCCESS;
}   
#endif
/**
  * @brief  This fonction read the NDEF content of the TAG
    * @param    pNDEF : pointer on the buffer to store NDEF data
  * @retval SUCCESS : NDEF file data retrieve and store in the buffer
    * @retval ERROR : not able to read NDEF
  */
u16 NDEF_ReadNDEF( u8* pNDEF)
{
    u16 status = ERROR;
    u16 NDEF_Size = 0;
    
    status = ReadData( 0 , 2 , pNDEF);
    
    if( status == NDEF_ACTION_COMPLETED)
    {
        NDEF_Size = (u16) (*pNDEF << 8);
        NDEF_Size = NDEF_Size | (u16) (*++pNDEF );
       
        status = ReadData( 0 , NDEF_Size+2 , --pNDEF);
    }
    
    if( status == NDEF_ACTION_COMPLETED)
        return SUCCESS;
    else
        return ERROR;
    
}
/**
  * @brief  This fonction write the NDEF in the TAG
    * @param    pNDEF : pointer on the buffer containing the NDEF data
  * @retval SUCCESS : NDEF file data written in the tag
    * @retval ERROR : not able to store NDEF in tag
  */
u16 NDEF_WriteNDEF( u8 *pNDEF)
{
    u16 status = ERROR;
    u16 NDEF_Size = 0;
    
    NDEF_Size = (u16) (*pNDEF << 8);
    NDEF_Size = NDEF_Size | (u16) (*++pNDEF );

    status = WriteData( 0 , NDEF_Size+2 , --pNDEF);
    
    if( status == NDEF_ACTION_COMPLETED)
        return SUCCESS;
    else
        return ERROR;

}

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

