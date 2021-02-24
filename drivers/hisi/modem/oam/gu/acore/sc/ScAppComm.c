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


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "mdrv.h"
#include "ScInterface.h"
#include "ScCommCtx.h"



#define    THIS_FILE_ID        PS_FILE_ID_SC_APP_COMM_C

#define SC_IMEI_LOG_FILE_PATH   "/modem_log/OAM_IMEI_ACORE_Log.bin"


VOS_VOID SC_COMM_CloseAllFile(FILE *fp[SC_SECRET_FILE_TYPE_BUTT*2])
{
    VOS_UINT32                          i;

    for (i=0;i<SC_SECRET_FILE_TYPE_BUTT*2;i++)
    {
        if (VOS_NULL_PTR != fp[i])
        {
             /*lint -e534*/
             mdrv_file_close(fp[i]);
             /*lint +e534*/
        }
        else
        {
          continue;
        }
    }
    return;
}


VOS_UINT32 SC_COMM_Backup(VOS_VOID)
{
    FILE                               *fp[SC_SECRET_FILE_TYPE_BUTT*2] = {VOS_NULL_PTR};
    SC_BACKUP_FILE_INFO_STRU           *pstFileInfo;
    SC_BACKUP_FILE_INFO_STRU           *pstFileInfoTemp;
    SC_COMM_GLOBAL_STRU                *pstCommGlobal;
    VOS_INT32                           lReadSize;
    VOS_UINT32                          i;

    pstFileInfoTemp = (SC_BACKUP_FILE_INFO_STRU*)VOS_MemAlloc(ACPU_PID_PAM_OM, DYNAMIC_MEM_PT,
                                                             sizeof(SC_BACKUP_FILE_INFO_STRU));
    if(VOS_NULL_PTR == pstFileInfoTemp)
    {
        SC_Printf("SC_COMM_Backup: alloc mem1 fail!\r\n");
        return SC_ERROR_CODE_ALLOC_MEM_FAIL;
    }

    /*lint -e534*/
    VOS_MemSet((VOS_CHAR*)pstFileInfoTemp, 0, sizeof(SC_BACKUP_FILE_INFO_STRU));
    /*lint +e534*/

    pstFileInfoTemp->ulBackMagicMUM = SC_FILE_PACKET_MAGIC;
    pstFileInfoTemp->ulTotaleSize   = (sizeof(SC_BACKUP_FILE_INFO_STRU) - 4*sizeof(VOS_CHAR));

    pstCommGlobal = SC_CTX_GetCommGlobalVarAddr();

    /* ��SC�ļ� */
    for (i=0; i<SC_SECRET_FILE_TYPE_BUTT; i++)
    {
        fp[i] = mdrv_file_open(pstCommGlobal->acSecretFileName[i], "rb");

        if (VOS_NULL_PTR != fp[i])
        {
            pstFileInfoTemp->astSCEachFileInfo[i].ulFileMagicMUM = SC_FILE_EXIST_MAGIC;
            /*lint -e534*/
            VOS_StrNCpy(pstFileInfoTemp->astSCEachFileInfo[i].acFilePath,
                        pstCommGlobal->acSecretFileName[i], SC_FILE_PATH_LEN -1);
            /*lint +e534*/
        }
    }

    /* ��ǩ���ļ� */
    for (i=0; i<SC_SECRET_FILE_TYPE_BUTT; i++)
    {
        fp[i+SC_SECRET_FILE_TYPE_BUTT] = mdrv_file_open(pstCommGlobal->acSignFileName[i], "rb");

        if (VOS_NULL_PTR != fp[i+SC_SECRET_FILE_TYPE_BUTT])
        {
            pstFileInfoTemp->astSCEachFileInfo[i+SC_SECRET_FILE_TYPE_BUTT].ulFileMagicMUM = SC_FILE_EXIST_MAGIC;
            /*lint -e534*/
            VOS_StrNCpy(pstFileInfoTemp->astSCEachFileInfo[i+SC_SECRET_FILE_TYPE_BUTT].acFilePath,
                        pstCommGlobal->acSignFileName[i], SC_FILE_PATH_LEN-1);
            /*lint +e534*/
        }
    }

    /* ���������ļ���С */
    for (i=0; i<SC_SECRET_FILE_TYPE_BUTT*2; i++)
    {
        if (SC_FILE_EXIST_MAGIC != pstFileInfoTemp->astSCEachFileInfo[i].ulFileMagicMUM)
        {
            continue;
        }

        (VOS_VOID)mdrv_file_seek(fp[i], 0, SC_FILE_SEEK_END);
        pstFileInfoTemp->astSCEachFileInfo[i].ulFileLen    = (VOS_UINT32)mdrv_file_tell(fp[i]);
        pstFileInfoTemp->astSCEachFileInfo[i].ulFileOffset = pstFileInfoTemp->ulTotaleSize;
        pstFileInfoTemp->ulTotaleSize += pstFileInfoTemp->astSCEachFileInfo[i].ulFileLen;
        (VOS_VOID)mdrv_file_seek(fp[i], 0, SC_FILE_SEEK_SET);
    }

    pstFileInfo =(SC_BACKUP_FILE_INFO_STRU*)VOS_MemAlloc(ACPU_PID_PAM_OM,
                                                         DYNAMIC_MEM_PT,
                                                         pstFileInfoTemp->ulTotaleSize);
    if (VOS_NULL_PTR == pstFileInfo)
    {
        /*lint -e534*/
        VOS_MemFree(ACPU_PID_PAM_OM, pstFileInfoTemp);
        /*lint +e534*/
        SC_COMM_CloseAllFile(fp);

        SC_Printf("SC_COMM_Backup: alloc mem2 fail!\r\n");

        return SC_ERROR_CODE_ALLOC_MEM_FAIL;
    }

    /* д���ļ�ͷ��Ϣ */
    /*lint -e534*/
    VOS_MemCpy((VOS_CHAR*)pstFileInfo,
               (VOS_CHAR*)pstFileInfoTemp,
               sizeof(SC_BACKUP_FILE_INFO_STRU) - 4*sizeof(VOS_CHAR));

    VOS_MemFree(ACPU_PID_PAM_OM, pstFileInfoTemp);
    /*lint +e534*/

    for (i=0;i<SC_SECRET_FILE_TYPE_BUTT*2; i++)
    {
        if (SC_FILE_EXIST_MAGIC != pstFileInfo->astSCEachFileInfo[i].ulFileMagicMUM)
        {
            continue;
        }

        lReadSize = mdrv_file_read((VOS_CHAR*)pstFileInfo + pstFileInfo->astSCEachFileInfo[i].ulFileOffset,
                                  sizeof(VOS_CHAR),
                                  pstFileInfo->astSCEachFileInfo[i].ulFileLen,
                                  fp[i]);

        if(lReadSize != pstFileInfo->astSCEachFileInfo[i].ulFileLen)
        {
            /* ��ʧ�ܣ��ͷž�����ڴ�*/
            SC_COMM_CloseAllFile(fp);
            /*lint -e534*/
            VOS_MemFree(ACPU_PID_PAM_OM, pstFileInfo);
            /*lint +e534*/

            SC_Printf("SC_COMM_Backup: mdrv_file_read fail!\r\n");

            return SC_ERROR_CODE_READ_FILE_FAIL;
        }

    }

    /* �ļ���д��ϣ��رվ�� */
    SC_COMM_CloseAllFile(fp);

    /* д�����SC������ */
    if (SC_ERROR_CODE_NO_ERROR != mdrv_misc_scbackup_ext_write((VOS_UINT8*)pstFileInfo, pstFileInfo->ulTotaleSize))
    {
        /*lint -e534*/
        VOS_MemFree(ACPU_PID_PAM_OM, pstFileInfo);
        /*lint +e534*/
        SC_Printf("SC_COMM_Backup: scbackup_ext_write fail!\r\n");

        return SC_ERROR_CODE_SCCONTENT_WRITE_FAIL;
    }

    /*lint -e534*/
    VOS_MemFree(ACPU_PID_PAM_OM, pstFileInfo);
    /*lint +e534*/

    return SC_ERROR_CODE_NO_ERROR;
}


VOS_BOOL SC_COMM_RestoreCheck(VOS_VOID)
{
    SC_COMM_GLOBAL_STRU                *pstCommGlobal;
    VOS_UINT32                          ulCheckStatus = 0;

    /* ˵��: ֻ�йؼ��ļ��������ڲ�ִ�лָ��������ؼ��ļ��ݶ� CK-FILE PI-FILE
              �и����ļ���ʧ����λ */

    pstCommGlobal = SC_CTX_GetCommGlobalVarAddr();

    if (VOS_OK != mdrv_file_access(pstCommGlobal->acSecretFileName[SC_SECRET_FILE_TYPE_CK], SC_FILE_EXIST_OK))
    {
        ulCheckStatus++;
    }

    if (VOS_OK != mdrv_file_access(pstCommGlobal->acSecretFileName[SC_SECRET_FILE_TYPE_PI], SC_FILE_EXIST_OK))
    {
        ulCheckStatus++;
    }

    if (2 == ulCheckStatus)
    {
        /* �ؼ��ļ��������ڣ���ִ�лָ� */
        SC_Printf("SC_COMM_RestoreCheck: sc need restore!!\r\n");

        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_UINT32 SC_COMM_WriteScFile(SC_BACKUP_FILE_INFO_STRU     *pstFileInfo)
{
    FILE                               *fp;
    VOS_INT32                           lWriteSize;
    VOS_UINT32                          i;

    for (i=0; i<(SC_SECRET_FILE_TYPE_BUTT * 2); i++)
    {
         /* �ж�ħ�������Ƿ���ȷ */
        if (SC_FILE_EXIST_MAGIC != pstFileInfo->astSCEachFileInfo[i].ulFileMagicMUM)
        {
            continue;
        }

        /* д�ļ� */
        fp = mdrv_file_open(pstFileInfo->astSCEachFileInfo[i].acFilePath, "wb+");

        if (VOS_NULL_PTR == fp)
        {
            SC_Printf("SC_COMM_WriteScFile: sc create file fail!");

            return SC_ERROR_CODE_OPEN_FILE_FAIL;
        }

        lWriteSize = mdrv_file_write_sync((VOS_CHAR*)pstFileInfo + pstFileInfo->astSCEachFileInfo[i].ulFileOffset,
                                     sizeof(VOS_CHAR),
                                     pstFileInfo->astSCEachFileInfo[i].ulFileLen,
                                     fp);

        (VOS_VOID)mdrv_file_close(fp);

        if (lWriteSize != pstFileInfo->astSCEachFileInfo[i].ulFileLen)
        {
            SC_Printf("SC_COMM_WriteScFile: sc write file fail!");

            return SC_ERROR_CODE_WRITE_FILE_FAIL;
        }
    }

    return SC_ERROR_CODE_NO_ERROR;
}


VOS_UINT32 SC_COMM_Restore(VOS_VOID)
{
    SC_BACKUP_FILE_INFO_STRU           *pstFileInfoTemp;
    SC_BACKUP_FILE_INFO_STRU           *pstFileInfo;

    if (VOS_FALSE == SC_COMM_RestoreCheck())
    {
        return SC_ERROR_CODE_SC_NO_NEED_RESTORE;
    }

    pstFileInfoTemp = (SC_BACKUP_FILE_INFO_STRU*)VOS_MemAlloc(ACPU_PID_PAM_OM,
                                                               DYNAMIC_MEM_PT,
                                                               sizeof(SC_BACKUP_FILE_INFO_STRU));
    if (VOS_NULL_PTR == pstFileInfoTemp)
    {
        SC_Printf("SC_COMM_Restore: alloc mem1 fail!\r\n");

        return SC_ERROR_CODE_ALLOC_MEM_FAIL;
    }

    if (VOS_OK != mdrv_misc_scbackup_ext_read((VOS_UINT8*)pstFileInfoTemp, sizeof(SC_BACKUP_FILE_INFO_STRU)))
    {
        (VOS_VOID)VOS_MemFree(ACPU_PID_PAM_OM, pstFileInfoTemp);

        SC_Printf("SC_COMM_Restore: sc backup read fail 1!\r\n");

        return SC_ERROR_CODE_SCBACKUP_READ_FAIL;
    }

    /* �ж�ħ�������Ƿ���ȷ */
    if (SC_FILE_PACKET_MAGIC != pstFileInfoTemp->ulBackMagicMUM)
    {
        (VOS_VOID)VOS_MemFree(ACPU_PID_PAM_OM, pstFileInfoTemp);

        SC_Printf("SC_COMM_Restore: sc backup magic number error!\r\n");

        return SC_ERROR_CODE_MAGNUM_CMP_FAIL;
    }

    /* �ж�Ŀ���ļ����Ƿ���� */
    if (SC_ERROR_CODE_NO_ERROR != mdrv_file_access("/mnvm2:0/SC", SC_FILE_EXIST_OK))
    {
        (VOS_VOID)mdrv_file_mkdir("/mnvm2:0/SC");
    }

    if (SC_ERROR_CODE_NO_ERROR != mdrv_file_access("/mnvm2:0/SC/Pers", SC_FILE_EXIST_OK))
    {
        (VOS_VOID)mdrv_file_mkdir("/mnvm2:0/SC/Pers");
    }

    /* ���ݳ��������ڴ� */
    pstFileInfo = (SC_BACKUP_FILE_INFO_STRU*)VOS_MemAlloc(ACPU_PID_PAM_OM,
                                                          DYNAMIC_MEM_PT,
                                                          pstFileInfoTemp->ulTotaleSize);
    if (VOS_NULL_PTR == pstFileInfo)
    {
        (VOS_VOID)VOS_MemFree(ACPU_PID_PAM_OM, pstFileInfoTemp);

        SC_Printf("SC_COMM_Restore: alloc mem2 fail!\r\n");

        return SC_ERROR_CODE_ALLOC_MEM_FAIL;
    }

    /* ��ȡȫ������ */
    if (VOS_OK != mdrv_misc_scbackup_ext_read((VOS_UINT8*)pstFileInfo, pstFileInfoTemp->ulTotaleSize))
    {
        (VOS_VOID)VOS_MemFree(ACPU_PID_PAM_OM, pstFileInfoTemp);
        (VOS_VOID)VOS_MemFree(ACPU_PID_PAM_OM, pstFileInfo);

        SC_Printf("SC_COMM_Restore: sc backup read fail 2!\r\n");

        return SC_ERROR_CODE_SCBACKUP_READ_FAIL;
    }

    (VOS_VOID)VOS_MemFree(ACPU_PID_PAM_OM, pstFileInfoTemp);

    if (VOS_OK != SC_COMM_WriteScFile(pstFileInfo))
    {
        (VOS_VOID)VOS_MemFree(ACPU_PID_PAM_OM, pstFileInfo);

        return SC_ERROR_CODE_SCFILE_RESTORE_FAIL;
    }

    (VOS_VOID)VOS_MemFree(ACPU_PID_PAM_OM, pstFileInfo);

    return SC_ERROR_CODE_NO_ERROR;
}

/*****************************************************************************
�� �� ��  :SC_Printf
��������  :Log��ӡ,�ɱ����
*****************************************************************************/
VOS_VOID SC_Printf(VOS_CHAR *pcData)
{
    FILE                               *fp;
    VOS_UINT32                          ulRetLen;
    VOS_UINT32                          ulFileSize;

    ulRetLen = VOS_StrLen(pcData);

    if ((SC_LOG_MAX_LEN < ulRetLen) || (0 == ulRetLen))
    {
        return ;
    }

    fp = mdrv_file_open(SC_IMEI_LOG_FILE_PATH, "a");

    if (VOS_NULL_PTR == fp)
    {
        return ;
    }

    if (VOS_OK != mdrv_file_seek(fp, 0, SC_FILE_SEEK_END))
    {
        return;
    }

    ulFileSize = (VOS_UINT32)mdrv_file_tell(fp);

    /* If the log file is too large, we need empty it. */
    if (ulFileSize > SC_LOG_FILE_MAX_SIZE)
    {
        /*lint -e534*/
        mdrv_file_close(fp);
        /*lint +e534*/
        fp = mdrv_file_open(SC_IMEI_LOG_FILE_PATH, "w");

        if (VOS_NULL_PTR == fp)
        {
            return ;
        }
    }

    /*lint -e534*/
    (VOS_VOID)mdrv_file_write(pcData, ulRetLen, 1, fp);

    (VOS_VOID)mdrv_file_close(fp);
    /*lint +e534*/

    return ;
}


