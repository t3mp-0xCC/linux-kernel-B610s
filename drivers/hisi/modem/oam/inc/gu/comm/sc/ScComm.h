/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */



#ifndef __SCCOMM_H__
#define __SCCOMM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "ScInterface.h"
#include "UsimPsInterface.h"
#include "msp_diag_comm.h"
/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define   SC_KEY_LEN                (32)                                        /* ��Կ�ĳ��� */

#define   SC_HASH_LEN               (32)                                        /* HASH���������� */

#define   SC_FILE_PATH_LEN          (128)

#define   SC_SSK_STRING             "Secure Storage Key"                        /* ����SSK���ַ��� */

#define   SC_SSK_STRLEN             (VOS_StrLen(SC_SSK_STRING))                 /* ����SSK���ַ������� */

#define   SC_DSSK_STRING            "Debug port protect Secure Storage Key"     /* ����DSSK���ַ��� */

#define   SC_DSSK_STRLEN            (VOS_StrLen(SC_DSSK_STRING))                /* ����DSSK���ַ������� */

#define   SC_IPK_STRING             "Integrity Protection Key"                  /* ����IPK���ַ��� */

#define   SC_IPK_STRLEN             (VOS_StrLen(SC_IPK_STRING))                 /* ����IPK���ַ������� */

#define   SC_ASSK_STRING            "AT Secure Storage Key"                     /* ����ASSK���ַ��� */

#define   SC_ASSK_STRLEN            (VOS_StrLen(SC_ASSK_STRING))                /* ����ASSK���ַ������� */

#define   SC_SATK_STRING            "Secure AT Key"                             /* ����SATK���ַ��� */

#define   SC_SATK_STRLEN            (VOS_StrLen(SC_SATK_STRING))                /* ����SATK���ַ������� */

#define   SC_FILE_EXIST_MAGIC       (0x37ab65cd)                                /* SC�����ļ����ڱ�� */

#define   SC_FILE_PACKET_MAGIC      (0xdeacb589)                                /* SC���������ڱ�� */

#define   SC_LOG_MAX_LEN            (512)

#define   SC_LOG_FILE_MAX_SIZE      (SC_LOG_MAX_LEN * 100)    /* the max size of log file. */

#define   SC_FILE_SEEK_END          (2)

#define   SC_FILE_SEEK_SET          (0)

#define   SC_FILE_EXIST_OK          (0)

#if ((OSA_CPU_CCPU == VOS_OSA_CPU))
#define SC_GEN_LOG_MODULE(Level) (/*lint -e1011 -e778*/DIAG_GEN_LOG_MODULE(VOS_GetModemIDFromPid(MAPS_PB_PID), DIAG_MODE_COMM, Level)/*lint +e1011 +e778*/)

#define   SC_NORMAL_LOG(string)                     (VOS_VOID)DIAG_LogReport(SC_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), MAPS_PB_PID, __FILE__, __LINE__, "%s", string);\
                                                    SC_Printf("%s.\r\n", string)

#define   SC_NORMAL_ASCII_LOG(string)               (VOS_VOID)DIAG_LogReport(SC_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), MAPS_PB_PID, __FILE__, __LINE__, "%02x ", string);\
                                                    SC_Printf("%02x ", string)

#define   SC_INFO1_LOG(string, para1)               (VOS_VOID)DIAG_LogReport(SC_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL),MAPS_PB_PID, __FILE__, __LINE__, "%s,%d", string, para1);\
                                                    SC_Printf("%s %d.\r\n", string, para1)

#define   SC_NORMAL1_LOG(string, para1)             (VOS_VOID)DIAG_LogReport(SC_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL),MAPS_PB_PID, __FILE__, __LINE__, "%s,%d", string, para1);\
                                                    SC_Printf("%s %d.\r\n", string, para1)

#define   SC_WARNING1_LOG(string, para1)            (VOS_VOID)DIAG_LogReport(SC_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL),MAPS_PB_PID, __FILE__, __LINE__, "%s,%d", string, para1);\
                                                    SC_Printf("%s %d.\r\n", string, para1)

#define   SC_ERROR_LOG(string)                      (VOS_VOID)DIAG_LogReport(SC_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), MAPS_PB_PID, __FILE__, __LINE__, "%s", string);\
                                                    SC_Printf("%s \r\n", string)

#define   SC_ERROR1_LOG(string, para1)              (VOS_VOID)DIAG_LogReport(SC_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL),MAPS_PB_PID, __FILE__, __LINE__, "%s,%d", string, para1);\
                                                    SC_Printf("%s %d.\r\n", string, para1)

#define   SC_ERROR2_LOG(string, para1, para2)       (VOS_VOID)DIAG_LogReport(SC_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL),MAPS_PB_PID, __FILE__, __LINE__, "%s,%d,%d", string, para1, para2);\
                                                    SC_Printf("%s %d, %d.\r\n", string, para1, para2)
#else
#define   SC_ERROR_LOG(string)                      SC_Printf(string)
#endif

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

enum SC_KEY_TYPE_ENUM
{
    SC_KEY_TYPE_SSK                     = 0x00,             /* SSK */
    SC_KEY_TYPE_DSSK                    = 0x01,             /* DSSK */
    SC_KEY_TYPE_IPK                     = 0x02,             /* IPK */
    SC_KEY_TYPE_BUTT
};
typedef VOS_UINT8 SC_KEY_TYPE_ENUM_UINT8;


enum SC_CRYPTO_PASSWORD_TYPE_ENUM
{
    SC_CRYPTO_PASSWORD_TYPE_CK          = 0x00,             /* CK������ */
    SC_CRYPTO_PASSWORD_TYPE_DK          = 0x01,             /* DK������ */
    SC_CRYPTO_PASSWORD_TYPE_BUTT
};
typedef VOS_UINT8 SC_CRYPTO_PASSWORD_TYPE_ENUM_UINT8;


enum SC_SECRET_FILE_TYPE_ENUM
{
    SC_SECRET_FILE_TYPE_CK              = 0x00,             /* CK-FILE */
    SC_SECRET_FILE_TYPE_DK              = 0x01,             /* DK-FILE */
    SC_SECRET_FILE_TYPE_AK              = 0x02,             /* AK-FILE */
    SC_SECRET_FILE_TYPE_PI              = 0x03,             /* PI-FILE */
#if  ( FEATURE_MULTI_MODEM == FEATURE_ON )
#if (3 == MULTI_MODEM_NUMBER)
    SC_SECRET_FILE_TYPE_IMEI_I0         = 0x04,             /* IMEI-FILE ��0 */
    SC_SECRET_FILE_TYPE_IMEI_I1         = 0x05,             /* IMEI-FILE ��1 */
    SC_SECRET_FILE_TYPE_IMEI_I2         = 0x06,             /* IMEI-FILE ��2 */
#else
    SC_SECRET_FILE_TYPE_IMEI_I0         = 0x04,             /* IMEI-FILE ��0 */
    SC_SECRET_FILE_TYPE_IMEI_I1         = 0x05,             /* IMEI-FILE ��1 */
#endif
#else
    SC_SECRET_FILE_TYPE_IMEI_I0         = 0x04,             /* IMEI-FILE ��0 */
#endif

    SC_SECRET_FILE_TYPE_BUTT
};
typedef VOS_UINT8 SC_SECRET_FILE_TYPE_ENUM_UINT8;

/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/
/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/
/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/
/*****************************************************************************
  7 STRUCT����
*****************************************************************************/


typedef struct
{
    VOS_UINT32                          ulFileMagicMUM;
    VOS_UINT32                          ulFileLen;
    VOS_UINT32                          ulFileOffset;
    VOS_CHAR                            acFilePath[SC_FILE_PATH_LEN];
}SC_BACKUP_EACH_FILE_INFO_STRU;


typedef struct
{
    VOS_UINT32                         ulBackMagicMUM;
    VOS_UINT32                         ulTotaleSize;
    SC_BACKUP_EACH_FILE_INFO_STRU      astSCEachFileInfo[SC_SECRET_FILE_TYPE_BUTT*2];
    VOS_CHAR                           aucFileData[4];
}SC_BACKUP_FILE_INFO_STRU;

/*****************************************************************************
  8 UNION����
*****************************************************************************/
/*****************************************************************************
  9 OTHERS����
*****************************************************************************/
/*****************************************************************************
  10 ��������
*****************************************************************************/
#if ((OSA_CPU_CCPU == VOS_OSA_CPU))
extern SC_ERROR_CODE_ENUM_UINT32 SC_COMM_GenerateKey(
    VOS_CHAR                           *pcData,
    VOS_UINT32                          ulLen,
    VOS_UINT8                          *pucKey,
    VOS_UINT32                          ulKeyLen
);

extern SC_ERROR_CODE_ENUM_UINT32 SC_COMM_ReadSCFileAndCmpSign(
    SC_SECRET_FILE_TYPE_ENUM_UINT8      enFileType,
    VOS_UINT8                          *pucContent,
    VOS_UINT32                          ulContentLen
);

extern SC_ERROR_CODE_ENUM_UINT32 SC_COMM_WriteSecretFile(
    SC_SECRET_FILE_TYPE_ENUM_UINT8      enFileType,
    VOS_UINT8                          *pucContent,
    VOS_UINT32                          ulContentLen
);

extern SC_ERROR_CODE_ENUM_UINT32 SC_COMM_GenerateCryptoPwd(
    SC_CRYPTO_PASSWORD_TYPE_ENUM_UINT8  enPwdType,
    VOS_UINT8                          *pucPwd,
    VOS_UINT8                           ucPwdLen,
    VOS_UINT8                          *pucCryptoPwd,
    VOS_UINT32                          ulCryptoPwdLen
);

extern SC_ERROR_CODE_ENUM_UINT32 SC_COMM_RsaEncrypt(
    VOS_UINT8                          *pucRawData,
    VOS_UINT32                          ulLen,
    VOS_UINT8                          *pucCipherData,
    VOS_UINT32                         *pulCipherLen
);

extern SC_ERROR_CODE_ENUM_UINT32 SC_COMM_RsaDecrypt(
    VOS_UINT8                          *pucPwd,
    VOS_UINT32                          ulCipherLen,
    VOS_UINT8                          *pucPubContent,
    VOS_UINT32                         *pulPubLen
);


SC_ERROR_CODE_ENUM_UINT32 SC_COMM_GenerateSignCode(
    VOS_UINT8                          *pucContent,
    VOS_UINT32                          ulContentLen,
    VOS_UINT8                          *pucSignCode,
    VOS_UINT32                          ulSignCodeLen
);


extern SC_ERROR_CODE_ENUM_UINT32 SC_COMM_WriteFile(
    VOS_CHAR                           *pcFilePath,
    VOS_UINT8                          *pucContent,
    VOS_UINT32                          ulContentLen
);


extern SC_ERROR_CODE_ENUM_UINT32 SC_COMM_ReadFile(
    VOS_CHAR                           *pcFilePath,
    VOS_UINT8                          *pucContent,
    VOS_UINT32                          ulContentLen
);


extern VOS_UINT32 SC_COMM_GetUsimmCachedFile(
    VOS_CHAR                           *pucFilePath,
    VOS_UINT32                         *pulDataLen,
    VOS_UINT8                         **ppucData,
    USIMM_CARDAPP_ENUM_UINT32           enAppType,
    MODEM_ID_ENUM_UINT16                enModemID);


extern VOS_UINT32 SC_COMM_IsUsimServiceAvailable(
    UICC_SERVICES_TYPE_ENUM_UINT32  enService,
    MODEM_ID_ENUM_UINT16            enModemID);
#endif

#if ((OSA_CPU_ACPU == VOS_OSA_CPU))


VOS_UINT32 SC_COMM_Restore(VOS_VOID);

extern VOS_VOID SC_Printf(VOS_CHAR *pcData);

#else
/*lint -e960 */
extern VOS_VOID SC_Printf(const VOS_CHAR *pcformat, ...);
/*lint +e960 */
#endif

#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif
