/**
  ******************************************************************************
  * @file    lib_M24SR.c
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to manage M24SR in a NFC forum context.
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
#include "libm24sr.h"
#if (FEATURE_OFF == MBB_NFC)
#include <unistd.h>
#else
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/kernel.h>
#endif
/** @addtogroup M24SR_Driver
  * @{
  * @brief  <b>This folder contains the driver layer of M24SR family (M24SR64, M24SR16, M24SR04, M24SR02)</b> 
  */

/** @addtogroup lib_M24SR
  * @{
    *   @brief  This is the library to interface with the M24SR dynamic tag.
    *         This layer simplify the use of the M24SR driver by sequencing 
    *         some commands.
  */

uint8_t I2CPassword[16]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#if (FEATURE_ON == MBB_NFC)
#define M24R_PASS_WORD_LEN 16
/*默认的读写密码都是16位0x00*/
uint8_t ReadPassword[M24R_PASS_WORD_LEN]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t WritePassword[M24R_PASS_WORD_LEN]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
/*新密码为最后一位为0x01*/
uint8_t WriteNewPasswd[M24R_PASS_WORD_LEN]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
#endif
static u32 NDEFSessionOpenID=NDEF_SESSION_CLOSED;

/* Init NDEF_FileID with bad value in case Init failed */
static u16 NDEF_FileID = 0xDEAD;

/** @defgroup libM24SR_Private_Functions
  * @{
  */

/**
  * @}
  */


/** @defgroup libM24SR_Public_Functions
  * @{
  */ 

/**
  * @brief  This fonction initialize the M24SR
    * @param    CCBuffer : pointer on the buffer to store CC file
    * @param    size : number of byte of data to read
  * @retval SUCCESS : Initalization done
    * @retval ERROR : Not able to Initialize. 
  */
u16 M24SR_Initialization ( u8* CCBuffer, u8 size )
{
    u16 status = ERROR;
#if (FEATURE_ON == MBB_NFC)
    uint8_t PasswordTmp[M24R_PASS_WORD_LEN]={0x00};
#else
    u16 timeout = 1000; /* wait 1sec */
#endif
    /* Perform HW initialization */
    M24SR_Init();

#if (FEATURE_OFF == MBB_NFC)
    /* Read CC file */
    status = M24SR_GetSession();
    while( status != M24SR_ACTION_COMPLETED && timeout)
    {
        usleep(1000);
        status = M24SR_GetSession();
        timeout--;
    }
#else
    status = M24SR_KillSession();
#endif
    if (status != M24SR_ACTION_COMPLETED)
        return ERROR;
    /*===================================*/
    /* Select the NFC type 4 application */ 
    /*===================================*/
    errorchk( M24SR_SelectApplication() );
        
    /*==================*/      
    /* select a CC file */ 
    /*==================*/
    errorchk (M24SR_SelectCCfile() );
    /* read the first 15 bytes of the CC file */
    if( M24SR_ReadData ( 0x0000 , 0x0F , CCBuffer ) == M24SR_ACTION_COMPLETED)
    {           
        NDEF_FileID = (uint16_t) ((CCBuffer[0x09]<<8) | CCBuffer[0x0A]);
        errorchk( M24SR_Deselect () );
#if (FEATURE_ON == MBB_NFC)
        if(SUCCESS == M24SR_OpenNDEFSession(NDEF_FileID,ASK_FOR_SESSION))
        {
             /*确保写密码正确，如果已更改过，确认一遍新密码*/
             if( SUCCESS != M24SR_EnableReadOnly(WritePassword))
             {
                 memcpy(PasswordTmp,WritePassword,M24R_PASS_WORD_LEN);
                 memcpy(WritePassword,WriteNewPasswd,M24R_PASS_WORD_LEN);
                 memcpy(WriteNewPasswd,PasswordTmp,M24R_PASS_WORD_LEN);
                 if( SUCCESS != M24SR_EnableReadOnly(WritePassword))
                 {
                     printk("M24SR_EnableReadOnly failed!\n");
                 }
             }
             M24SR_CloseNDEFSession(NDEF_FileID);
        }
#endif

        return SUCCESS;
    }
    else
        errorchk( M24SR_Deselect () );
        
Error:
#if (FEATURE_ON == MBB_NFC)
        printk(KERN_ERR "M24SR_Initialization error!\n");
#endif
        return ERROR;
        
}

/**
  * @brief  This fonction retrieve the NDEF file ID of NDEF file present in M24SR
    * @param    NDEF_fileID : To store NDEF ID
  * @retval SUCCESS : File ID read
    * @retval ERROR : Not able to read file ID. 
  */
u16 M24SR_GetNDEFFileId ( u16 *NDEF_fileID )
{
    if( NDEF_FileID != 0xDEAD)
    {
        *NDEF_fileID = NDEF_FileID;
        return SUCCESS;
    }
    else
    {
        return ERROR;
    }
}


/**
  * @brief  This fonction configure the M24SR to access NDEF message by I2C
    * @param    NDEF_fileID : NDEF identification to select NDEF in M24SR
    * @param  Priority: 2 options: check if M24SR available to open session (no RF session on going)
  *                                                          Kill RF session and open I2C sesssion.
  * @retval SUCCESS : Session is opened
    * @retval ERROR : Not able to open session. 
  */
u16 M24SR_OpenNDEFSession ( u16 NDEF_fileID, u16 Priority )
{
    u16 status = ERROR;
#if (FEATURE_OFF == MBB_NFC)
    u16 timeout = 1000;
#endif  
    if(NDEFSessionOpenID == NDEF_SESSION_CLOSED)
    {
#if (FEATURE_OFF == MBB_NFC)
        if( Priority == TAKE_SESSION)
        {
            status = M24SR_KillSession();
        }
        else
        {
            status = M24SR_GetSession();
            while( status != M24SR_ACTION_COMPLETED && timeout)
            {
                usleep(1000);
                status = M24SR_GetSession();
                timeout--;
            }
        }
#else
        status = M24SR_KillSession();
#endif
        if (status != M24SR_ACTION_COMPLETED)
            return ERROR;
    
        /*===================================*/
        /* Select the NFC type 4 application */ 
        /*===================================*/
        errorchk( M24SR_SelectApplication() );
        
        /*====================*/
        /* select NDEF file   */
        /*====================*/
        errorchk( M24SR_SelectNDEFfile(NDEF_fileID) );
    
        NDEFSessionOpenID = (u32)(NDEF_fileID);

        return SUCCESS;
    }
    else if(NDEFSessionOpenID == NDEF_fileID)
    {
        /* Session already Open not an issue caller can perform access in NDEF file */
        return SUCCESS;
    }
        
Error:
        return ERROR;   
}

/**
  * @brief  This fonction close the NDEF Session.
    * @param    NDEF_fileID : NDEF identification to select NDEF in M24SR
  * @retval SUCCESS : Session is closed
    * @retval ERROR : Not able to close session. 
  */
u16 M24SR_CloseNDEFSession ( u16 NDEF_fileID )
{
    u16 status = ERROR;
    
    if(NDEFSessionOpenID == (u32)(NDEF_fileID))
    {
        errorchk( M24SR_Deselect () );
        NDEFSessionOpenID = NDEF_SESSION_CLOSED;
    
        return SUCCESS;
    }
    else if(NDEFSessionOpenID == NDEF_SESSION_CLOSED)
    {
        /* Not an error as session is already closed */
        return SUCCESS;
    }
        
Error:
        return ERROR;   
}

/**
  * @brief  This fonction read the data stored in M24SR at defined offset
    * @param    Offset : Offset in the NDEF file in M24SR
    * @param    DataSize : Number of byte to read
    * @param    pData : pointer on buffer to store read data
    * @retval Status (SW1&SW2) : Status of the operation. 
  */
u16 M24SR_ReadData ( u16 Offset , u16 DataSize , u8* pData)
{
    u16 status;
    
    if( DataSize > M24SR_READ_MAX_NBBYTE)
    {   
        do
        {
            status = M24SR_ReadBinary ( Offset, M24SR_READ_MAX_NBBYTE , pData);
            Offset += M24SR_READ_MAX_NBBYTE;
            pData += M24SR_READ_MAX_NBBYTE;
            DataSize -= M24SR_READ_MAX_NBBYTE;
        }while( DataSize > M24SR_READ_MAX_NBBYTE && status == M24SR_ACTION_COMPLETED);
        if( status == M24SR_ACTION_COMPLETED && DataSize)
            status = M24SR_ReadBinary ( Offset, (u8)(DataSize) , pData);
    }
    else
        status = M24SR_ReadBinary ( Offset, (u8)(DataSize) , pData);
    
    return status;
}

/**
  * @brief  This fonction read the data stored in M24SR at defined offset without NDEF concideration
    * @param    Offset : Offset in the NDEF file in M24SR
    * @param    DataSize : Number of byte to read
    * @param    pData : pointer on buffer to store read data
    * @retval Status (SW1&SW2) : Status of the operation.  
  */
u16 M24SR_ForceReadData ( u16 Offset , u16 DataSize , u8* pData)
{
    u16 status;
    
    if( DataSize > M24SR_READ_MAX_NBBYTE)
    {   
        do
        {
            status = M24SR_STReadBinary ( Offset, M24SR_READ_MAX_NBBYTE , pData);
            Offset += M24SR_READ_MAX_NBBYTE;
            pData += M24SR_READ_MAX_NBBYTE;
            DataSize -= M24SR_READ_MAX_NBBYTE;
        }while( DataSize > M24SR_READ_MAX_NBBYTE && status == M24SR_ACTION_COMPLETED);
        if( status == M24SR_ACTION_COMPLETED && DataSize)
            status = M24SR_STReadBinary ( Offset, (u8)(DataSize) , pData);
    }
    else
        status = M24SR_STReadBinary ( Offset, (u8)(DataSize) , pData);
    
    return status;
}

/**
  * @brief  This fonction write data in M24SR at defined offset
    * @param    Offset : Offset in the NDEF file in M24SR
    * @param    DataSize : Number of byte to read
    * @param    pData : pointer on buffer to copy in M24SR
    * @retval Status (SW1&SW2) : Status of the operation.  
  */
u16 M24SR_WriteData ( u16 Offset , u16 DataSize , u8* pData)
{
    u16 status;
    
    if( DataSize > M24SR_WRITE_MAX_NBBYTE)
    {   
        do
        {
            status = M24SR_UpdateBinary ( Offset, M24SR_WRITE_MAX_NBBYTE , pData);
            Offset += M24SR_WRITE_MAX_NBBYTE;
            pData += M24SR_WRITE_MAX_NBBYTE;
            DataSize -= M24SR_WRITE_MAX_NBBYTE;
        }while( DataSize > M24SR_WRITE_MAX_NBBYTE && status == M24SR_ACTION_COMPLETED);
        if( status == M24SR_ACTION_COMPLETED && DataSize)
            status = M24SR_UpdateBinary ( Offset, (u8)(DataSize) , pData);
    }
    else
        status = M24SR_UpdateBinary ( Offset, (u8)(DataSize) , pData);
    
    return status;
}

/**
  * @brief  This fonction activate the need of a password for next read access
    * @param    pCurrentWritePassword : Write password is needed to have the right to enable Read Password
    * @param    pNewPassword : The password that will be requiered for next read access
  * @retval SUCCESS : Read password is activated
    * @retval ERROR : operation does not complete  
  */
u16 M24SR_EnableReadPassword( u8* pCurrentWritePassword, u8* pNewPassword)
{
    u16 status = SUCCESS;

    if(M24SR_Verify( WRITE_PWD ,0x10 ,pCurrentWritePassword ) == M24SR_PWD_CORRECT)
    {               
        /* Set new password */
        M24SR_ChangeReferenceData ( READ_PWD, pNewPassword );
        M24SR_EnableVerificationRequirement( READ_PWD );
        status = SUCCESS;
    }
    else
    {
        /* M24SR already lock but password not known */
        status = ERROR;
    }
            
    return status;
}   

/**
  * @brief  This fonction desactivate the need of a password for next read access
    * @param    pCurrentWritePassword : Write password is needed to have the right to disable Read Password
  * @retval SUCCESS : Read password is desactivated
    * @retval ERROR : operation does not complete  
  */
u16 M24SR_DisableReadPassword( u8* pCurrentWritePassword)
{
    u16 status = SUCCESS;

    if(M24SR_Verify( WRITE_PWD ,0x10 ,pCurrentWritePassword ) == M24SR_PWD_CORRECT)
    {               
        /* Set new password */
        M24SR_DisableVerificationRequirement( READ_PWD );
        status = SUCCESS;
    }
    else
    {
        /* M24SR already lock but password not known */
        status = ERROR;
    }
            
    return status;
}   

/**
  * @brief  This fonction activate the need of a password for next write access
    * @param    pCurrentWritePassword : Write password must be prensented to have the right to modify write Password
    * @param    pNewPassword : The password that will be requiered for next write access
  * @retval SUCCESS : Write password is activated
    * @retval ERROR : operation does not complete   
  */
u16 M24SR_EnableWritePassword( u8* pCurrentWritePassword, u8* pNewPassword)
{
    u16 status;

    /* check we have the good password */
    if (M24SR_Verify( WRITE_PWD ,0x10 ,pCurrentWritePassword )== M24SR_PWD_CORRECT)
    {
        /* Set new password */
        M24SR_ChangeReferenceData ( WRITE_PWD, pNewPassword );
        M24SR_EnableVerificationRequirement( WRITE_PWD );
        status = SUCCESS;           
    }
    else /* we don't have the good password */
    {               
        status = ERROR;
    }
    
    return status;
}   

/**
  * @brief  This fonction desactivate the need of a password for next write access
    * @param    pCurrentWritePassword : Write password must be prensented to have the right to disable it
  * @retval SUCCESS : Write password is desactivated
    * @retval ERROR : operation does not complete   
  */
u16 M24SR_DisableWritePassword( u8* pCurrentWritePassword)
{
    u16 status = SUCCESS;

    if(M24SR_Verify( WRITE_PWD ,0x10 ,pCurrentWritePassword ) == M24SR_PWD_CORRECT)
    {               
        M24SR_DisableVerificationRequirement( WRITE_PWD );
        status = SUCCESS;
    }
    else
    {
        /* M24SR already lock but password not known */
        status = ERROR;
    }
            
    return status;
}   

/**
  * @brief  This fonction desactivate the need of read and write password for next access
    * @param    pSuperUserPassword : I2C super user password to overwrite read and write password
  * @retval SUCCESS : M24SR access is now free (no password needed)
    * @retval ERROR : operation does not complete   
  */
u16 M24SR_DisableAllPassword( u8* pSuperUserPassword)
{
    u16 status = SUCCESS;

    if(M24SR_Verify( I2C_PWD ,0x10 ,pSuperUserPassword ) == M24SR_PWD_CORRECT)
    {               
        M24SR_DisablePermanentState( READ_PWD );
        M24SR_DisablePermanentState( WRITE_PWD );
    
        M24SR_DisableVerificationRequirement( READ_PWD );
        M24SR_DisableVerificationRequirement( WRITE_PWD );
    
        /* reset password */
        M24SR_ChangeReferenceData ( READ_PWD, pSuperUserPassword );
        M24SR_ChangeReferenceData ( WRITE_PWD, pSuperUserPassword );
        status = SUCCESS;
    }
    else
    {
        /* M24SR already lock but password not known */
        status = ERROR;
    }
            
    return status;
}

/**
  * @brief  This fonction enable read only mode
    * @param    pCurrentWritePassword : Write password is needed to have right to enable read only mode
  * @retval SUCCESS : M24SR access is now forbidden in write mode
    * @retval ERROR : operation does not complete   
  */
u16 M24SR_EnableReadOnly( u8* pCurrentWritePassword)
{
    u16 status = SUCCESS;

    if(M24SR_Verify( WRITE_PWD ,0x10 ,pCurrentWritePassword ) == M24SR_PWD_CORRECT)
    {               
        M24SR_EnablePermanentState( WRITE_PWD ); /* lock write to have read only */
        status = SUCCESS;
    }
    else
    {
        /* M24SR already lock but password not known */
        status = ERROR;
    }
            
    return status;
}   

/**
  * @brief  This fonction disable read only mode
    * @param    pCurrentWritePassword : Write password is needed to have right to disable read only mode
  * @retval SUCCESS : M24SR write access is now allowed 
    * @retval ERROR : operation does not complete   
  */
u16 M24SR_DisableReadOnly( u8* pCurrentWritePassword)
{
    u16 status = SUCCESS;
                                                        
    if(M24SR_Verify( I2C_PWD ,0x10 ,I2CPassword ) == M24SR_PWD_CORRECT)
    {                   
        M24SR_DisablePermanentState( WRITE_PWD ); /* disable write protection to disable read only mode */
        M24SR_DisableVerificationRequirement( WRITE_PWD );
        status = SUCCESS;
    }
    else
    {
        /* we don't have the good I2C password nothing to do anymore */
        status = ERROR;
    }
            
    return status;
}   

/**
  * @brief  This fonction enable write only mode
    * @param    pCurrentWritePassword : Write password is needed to have right to enable write only mode
  * @retval SUCCESS : M24SR access is now forbidden in read mode
    * @retval ERROR : operation does not complete   
  */
u16 M24SR_EnableWriteOnly( u8* pCurrentWritePassword)
{
    u16 status = SUCCESS;

    if(M24SR_Verify( WRITE_PWD ,0x10 ,pCurrentWritePassword ) == M24SR_PWD_CORRECT)
    {               
        M24SR_EnablePermanentState( READ_PWD ); /* disable read access and keep write */
        status = SUCCESS;
    }
    else
    {
        /* M24SR already lock but password not known */
        status = ERROR;
    }
            
    return status;
}   

/**
  * @brief  This fonction disable write only mode
    * @param    pCurrentWritePassword : Write password is needed to have right to disable write only mode
  * @retval SUCCESS : M24SR read access is now allowed 
    * @retval ERROR : operation does not complete   
  */
u16 M24SR_DisableWriteOnly( u8* pCurrentWritePassword)
{
    u16 status = SUCCESS;

    if(M24SR_Verify( I2C_PWD ,0x10 ,I2CPassword ) == M24SR_PWD_CORRECT)
    {               
        M24SR_DisablePermanentState( READ_PWD ); /* disable write only -> enable write acces */
        M24SR_DisableVerificationRequirement( READ_PWD );
        status = SUCCESS;
    }
    else
    {
        /* M24SR already lock but password not known */
        status = ERROR;
    }
            
    return status;
}

/**
  * @brief  This function configure GPO purpose for RF session
    * @param    GPO_config: GPO configuration to set
    * @param    mode: select RF or I2C, GPO config to update
  * @retval Status : Status of the operation.
  */
u16 M24SR_ManageGPO( uc8 GPO_config, uc8 mode)
{
    u16 status;
    
    if( mode == RF_GPO)
    {
        status = M24SR_ManageRFGPO ( GPO_config );
    }
    else
    {
        status = M24SR_ManageI2CGPO ( GPO_config );
    }
    return status;
}
#if (FEATURE_ON == MBB_NFC)
 /**
  * @brief  This function sends the Verify command
    * @param    uPwdId : PasswordId ( 0x0001 : Read NDEF pwd or 0x0002 : Write NDEF pwd or 0x0003 : I2C pwd)
    * @param    pPasswd : pointer on the passwaord
    * @retval Status : NA.
*/
void M24SR_GetNewPasswd( uc16 uPwdId, uint8_t *pPasswd)
{
    if (uPwdId > I2C_PWD)
    {   
        printk(KERN_ERR "(%s) : pass word ID is invalid!\n",__func__);
        return;
    }
    if (NULL == pPasswd)
    {
        printk(KERN_ERR "(%s) : pPasswd is NULL !\n",__func__);
        return;
    }
    
    switch(uPwdId)
    {
        case READ_PWD:
            memcpy(pPasswd,ReadPassword,M24R_PASS_WORD_LEN);
        break;
        case WRITE_PWD:
            memcpy(pPasswd,WritePassword,M24R_PASS_WORD_LEN);
        break;
        case I2C_PWD:
            memcpy(pPasswd,I2CPassword,M24R_PASS_WORD_LEN);
        break;
        default:
        break;
     }
    
}

EXPORT_SYMBOL(M24SR_ManageGPO);
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


