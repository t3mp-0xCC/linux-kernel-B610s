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
/* Includes ------------------------------------------------------------------*/

#include <mbb_config.h>

#if (FEATURE_ON == MBB_NFC)
#include <linux/string.h>
#include <linux/kernel.h>
#else
#include "string.h"
#endif
#include "libndeftext.h"

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
extern uint8_t NDEF_Buffer [];

/**
  * @}
  */

/** @defgroup libEmail_Public_Functions
  * @{
  * @brief  This file is used to manage Email (stored or loaded in tag)
  */ 

/**
  * @brief  This fonction write the text in the TAG
    * @param    text : text to write
  * @retval RESULTOK : NDEF file data written in the tag
    * @retval ERRORCODE_GENERIC : not able to store NDEF in tag
  */
uint16_t NDEF_WriteText(char *text)
{
    uint16_t status = ERROR;
    uint32_t textSize, Offset = 0;
            
    NDEF_Buffer[0] = 0;
    NDEF_Buffer[1] = 0;
    Offset = FIRST_RECORD_OFFSET;
    
    /* TEXT : 1+en+message */
    textSize = 3+strlen(text);
        
    /* TEXT header */
    NDEF_Buffer[Offset] = 0xD1;
    if (textSize < 256) NDEF_Buffer[Offset] |= 0x10;                                         // Set the SR bit
    Offset++;
    
    NDEF_Buffer[Offset++] = TEXT_TYPE_STRING_LENGTH;
    if (textSize > 255)
    {
        NDEF_Buffer[Offset++] = (textSize & 0xFF000000)>>24;
        NDEF_Buffer[Offset++] = (textSize & 0x00FF0000)>>16;
        NDEF_Buffer[Offset++] = (textSize & 0x0000FF00)>>8;
        NDEF_Buffer[Offset++] = (textSize & 0x000000FF);
    }
    else
    {
        NDEF_Buffer[Offset++] = (uint8_t)textSize;
    }
    memcpy(&NDEF_Buffer[Offset], TEXT_TYPE_STRING, TEXT_TYPE_STRING_LENGTH);
    Offset+=TEXT_TYPE_STRING_LENGTH;
    
    /* TEXT payload */
    NDEF_Buffer[Offset++] = ISO_ENGLISH_CODE_STRING_LENGTH;
    memcpy(&NDEF_Buffer[Offset], ISO_ENGLISH_CODE_STRING, ISO_ENGLISH_CODE_STRING_LENGTH);
    Offset+=ISO_ENGLISH_CODE_STRING_LENGTH;
    
    memcpy( &NDEF_Buffer[Offset], text, strlen(text));
    Offset += strlen(text);
    
    Offset -= 2; /* Must not count the 2 byte that represent the NDEF size */
    NDEF_Buffer[0] = (Offset & 0xFF00)>>8;
    NDEF_Buffer[1] = (Offset & 0x00FF);
    
    status = WriteData ( 0x00 , Offset+2 , NDEF_Buffer);

    return status;
}


/**
  * @brief  This fonction read the text in the TAG
  * @param  text : text to write
  * @retval RESULTOK : NDEF file data written in the tag
  * @retval ERRORCODE_GENERIC : not able to store NDEF in tag
  */

uint16_t NDEF_ReadText(sRecordInfo *pRecordStruct, char *text)
{
#if (FEATURE_OFF == MBB_NFC)
	uint16_t status = ERROR;
      uint32_t PayloadSize, ndefSize = 0;
#else
	uint32_t PayloadSize=0;
#endif
    if( pRecordStruct->NDEF_Type == TEXT_TYPE )
    {
        PayloadSize = ((u32)(pRecordStruct->PayloadLength3)<<24) | ((u32)(pRecordStruct->PayloadLength2)<<16) |
                                        ((u32)(pRecordStruct->PayloadLength1)<<8)  | pRecordStruct->PayloadLength0;

		// Actually ISO_ENGLISH_CODE_STRING_LENGTH supported - so hardcoded in the offset
		// To be Done : replace hard coded value with real value
		memcpy( text, (u8*)(pRecordStruct->PayloadBufferAdd)+ISO_ENGLISH_CODE_STRING_LENGTH+TEXT_TYPE_STRING_LENGTH, PayloadSize-(ISO_ENGLISH_CODE_STRING_LENGTH+TEXT_TYPE_STRING_LENGTH));
	}
	else
	{
		return ERROR;
	}

    return SUCCESS;
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

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/


