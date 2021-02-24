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



#ifndef __SCINTERFACE_H__
#define __SCINTERFACE_H__


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)


/*****************************************************************************
  1 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define   SC_MAX_SUPPORT_CATEGORY   (3)   /* ֧�ֵ���������CATEGORY�������Ŀǰֻ֧������:network/network subset/SP */

#define   SC_MAX_CODE_LEN           (4)   /* �Ŷε���󳤶�(����Ϊ3��������Ϊ4����SPΪ4��ȡ����) */

#define   SC_MAX_CODE_NUM           (20)  /* ���������Ŷ������� */

#define   SC_AUTH_PUB_KEY_LEN       (520) /* ��Ȩ��Կ���� */

#define   SC_SIGNATURE_LEN          (32)  /* ǩ������ */

#define   SC_RSA_ENCRYPT_LEN        (128) /* ����RSA���ܺ�����ĳ��� */

#define   SC_CRYPTO_PWD_LEN         (16)  /* ��������ĳ��� */

#define   SC_PERS_PWD_LEN           (16)  /* CK/UK/DK��������ĳ��� */

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

enum SC_ERROR_CODE_ENUM
{
    SC_ERROR_CODE_NO_ERROR = 0,                     /* �����ɹ� */
    SC_ERROR_CODE_OPEN_FILE_FAIL,                   /* ���ļ�ʧ�� */
    SC_ERROR_CODE_READ_FILE_FAIL,                   /* ��ȡ�ļ�ʧ�� */
    SC_ERROR_CODE_WRITE_FILE_FAIL,                  /* д���ļ�ʧ�� */
    SC_ERROR_CODE_VERIFY_SIGNATURE_FAIL,            /* ǩ��У��ʧ�� */
    SC_ERROR_CODE_DK_INCORRECT,                     /* DK���벻��ȷ */
    SC_ERROR_CODE_UNLOCK_KEY_INCORRECT,             /* CK/UK���벻��ȷ */
    SC_ERROR_CODE_UNLOCK_STATUS_ABNORMAL,           /* ����/UNBLOCKʱ��״̬����ȷ */
    SC_ERROR_CODE_RSA_ENCRYPT_FAIL,                 /* RSA����ʧ�� */
    SC_ERROR_CODE_RSA_DECRYPT_FAIL,                 /* RSA����ʧ�� */
    SC_ERROR_CODE_VERIFY_PUB_KEY_SIGNATURE_FAIL,    /* ��Ȩ��Կ��ժҪǩ��У��ʧ�� */
    SC_ERROR_CODE_GET_RAND_NUMBER_FAIL,             /* ��ȡ�����ʧ��(crypto_rand) */
    SC_ERROR_CODE_IDENTIFY_FAIL,                    /* ���߼�Ȩʧ�� */
    SC_ERROR_CODE_IDENTIFY_NOT_FINISH,              /* ��δ��ɲ��߼�Ȩ */
    SC_ERROR_CODE_LOCK_CODE_INVALID,                /* �����Ŷα���ȡֵ����ȷ */
    SC_ERROR_CODE_CREATE_KEY_FAIL,                  /* ��Կ����ʧ��(create_crypto_key) */
    SC_ERROR_CODE_GENERATE_HASH_FAIL,               /* ����HASHʧ��(crypto_hash) */
    SC_ERROR_CODE_AES_ECB_ENCRYPT_FAIL,             /* AES ECB�㷨����ʧ��(crypto_encrypt) */
    SC_ERROR_CODE_WRITE_HUK_FAIL,                   /* HUKд����� */
    SC_ERROR_CODE_ALLOC_MEM_FAIL,                   /* �����ڴ�ʧ�� */
    SC_ERROR_CODE_PARA_FAIL,                        /* ����������� */
    SC_ERROR_CODE_WRITE_FILE_IMEI_FAIL,             /* IMEI����д���ļ����� */
    SC_ERROR_CODE_CMP_IMEI_FAIL,                    /* �Ƚ�IMEI�ַ���ʧ�� */
    SC_ERROR_CODE_MODEM_ID_FAIL,                    /* ModemID ���� */
    SC_ERROR_CODE_NV_READ_FAIL,                     /* NV��ʧ�� */
    SC_ERROR_CODE_NV_WRITE_FAIL,                    /* NVдʧ�� */
    SC_ERROR_CODE_SCCONTENT_WRITE_FAIL,             /* SC�ļ�д�뱸����ʧ�� */
    SC_ERROR_CODE_SCBACKUP_READ_FAIL,               /* ��ȡSC������ʧ�� */
    SC_ERROR_CODE_MAGNUM_CMP_FAIL,                  /* �Ƚ�SC���������ʧ�� */
    SC_ERROR_CODE_SCFILE_RESTORE_FAIL,              /* SC�ļ�д��ʹ����ʧ�� */
    SC_ERROR_CODE_SC_NO_NEED_RESTORE,               /* SC�ļ�����Ҫ�ָ� */
    SC_ERROR_CODE_OTHER_ERROR,                      /* �������� */

    SC_ERROR_CODE_BUTT
};
typedef VOS_UINT32  SC_ERROR_CODE_ENUM_UINT32;


enum SC_PERSONALIZATION_CATEGORY_ENUM
{
    SC_PERSONALIZATION_CATEGORY_NETWORK                     = 0x00, /* category:���� */
    SC_PERSONALIZATION_CATEGORY_NETWORK_SUBSET              = 0x01, /* category:������ */
    SC_PERSONALIZATION_CATEGORY_SERVICE_PROVIDER            = 0x02, /* category:��SP */
    SC_PERSONALIZATION_CATEGORY_CORPORATE                   = 0x03, /* category:������ */
    SC_PERSONALIZATION_CATEGORY_SIM_USIM                    = 0x04, /* category:��(U)SIM�� */

    SC_PERSONALIZATION_CATEGORY_BUTT
};
typedef VOS_UINT8 SC_PERSONALIZATION_CATEGORY_ENUM_UINT8;



enum SC_PERSONALIZATION_INDICATOR_ENUM
{
    SC_PERSONALIZATION_INDICATOR_INACTIVE                   = 0x00, /* δ���� */
    SC_PERSONALIZATION_INDICATOR_ACTIVE                     = 0x01, /* �Ѽ��� */

    SC_PERSONALIZATION_INDICATOR_BUTT
};
typedef VOS_UINT8 SC_PERSONALIZATION_INDICATOR_ENUM_UINT8;



enum SC_PERSONALIZATION_STATUS_ENUM
{
    SC_PERSONALIZATION_STATUS_READY     = 0x00, /* �Ѿ����� */
    SC_PERSONALIZATION_STATUS_PIN       = 0x01, /* δ����״̬������������� */
    SC_PERSONALIZATION_STATUS_PUK       = 0x02, /* Block״̬��������UnBlock�� */

    SC_PERSONALIZATION_STATUS_BUTT
};
typedef VOS_UINT8 SC_PERSONALIZATION_STATUS_ENUM_UINT8;



enum SC_VERIFY_SIMLOCK_RESULT_ENUM
{
    SC_VERIFY_SIMLOCK_RESULT_READY            = 0x00, /* READY: MT is not pending for any simlock */
    SC_VERIFY_SIMLOCK_RESULT_PH_NET_PIN       = 0x01, /* PH-NET PIN: MT is waiting network personalization password to be given */
    SC_VERIFY_SIMLOCK_RESULT_PH_NET_PUK       = 0x02, /* PH-NET PUK: MT is waiting network personalization unblocking password to be given */
    SC_VERIFY_SIMLOCK_RESULT_PH_NETSUB_PIN    = 0x03, /* PH-NETSUB PIN: MT is waiting network subset personalization password to be given */
    SC_VERIFY_SIMLOCK_RESULT_PH_NETSUB_PUK    = 0x04, /* PH-NETSUB PUK: MT is waiting network subset personalization unblocking password to be given */
    SC_VERIFY_SIMLOCK_RESULT_PH_SP_PIN        = 0x05, /* PH-SP PIN: MT is waiting service provider personalization password to be given */
    SC_VERIFY_SIMLOCK_RESULT_PH_SP_PUK        = 0x06, /* PH-SP PUK: MT is waiting service provider personalization unblocking password to be given */

    SC_VERIFY_SIMLOCK_RESULT_BUTT
};
typedef VOS_UINT8 SC_VERIFY_SIMLOCK_RESULT_ENUM_UINT8;



enum SC_PORT_STATUS_ENUM
{
    SC_PORT_STATUS_OFF                  = 0x00, /* �˿ڹر� */
    SC_PORT_STATUS_ON                   = 0x01, /* �˿ڴ� */

    SC_PORT_STATUS_BUTT
};
typedef VOS_UINT32 SC_PORT_STATUS_ENUM_UINT32;



enum SC_DH_KEY_TYPE_ENUM
{
    SC_DH_KEY_SERVER_PUBLIC_KEY         = 0x00,             /* ��������Կ */
    SC_DH_KEY_MODEM_PUBLIC_KEY          = 0x01,             /* MODEM�๫Կ */
    SC_DH_KEY_MODEM_PRIVATE_KEY         = 0x02,             /* MODEM��˽Կ */

    SC_DH_KEY_BUTT
};
typedef VOS_UINT32 SC_DH_KEY_TYPE_ENUM_UINT32;



/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/

typedef struct
{
   VOS_UINT8                            aucPhLockCodeBegin[SC_MAX_CODE_LEN];
   VOS_UINT8                            aucPhLockCodeEnd[SC_MAX_CODE_LEN];
}SC_PH_LOCK_CODE_STRU;



typedef struct
{
    SC_PERSONALIZATION_CATEGORY_ENUM_UINT8                  enCategory;                  /* ����������category��� */
    SC_PERSONALIZATION_INDICATOR_ENUM_UINT8                 enIndicator;                 /* ���������ļ���ָʾ */
    SC_PERSONALIZATION_STATUS_ENUM_UINT8                    enStatus;                    /* ���������ĵ�ǰ״̬ */
    VOS_UINT8                                               ucMaxUnlockTimes;            /* �������������������� */
    VOS_UINT8                                               ucRemainUnlockTimes;         /* ����������ʣ��������� */
    VOS_UINT8                                               aucReserved[2];              /* �����ֽڣ��������ֽڶ��� */
    VOS_UINT8                                               ucGroupNum;                  /* �Ŷθ�����һ��begin/end��һ���Ŷ� */
    SC_PH_LOCK_CODE_STRU                                    astLockCode[SC_MAX_CODE_NUM];/* ���������ĺŶ�����*/
}SC_SIMLOCK_CATEGORY_STRU;


typedef struct
{
    SC_PERSONALIZATION_CATEGORY_ENUM_UINT8                  enCategory;                  /* ����������category��� */
    SC_PERSONALIZATION_INDICATOR_ENUM_UINT8                 enIndicator;                 /* ���������ļ���ָʾ */
    VOS_UINT8                                               ucMaxUnlockTimes;            /* �������������������� */
    VOS_UINT8                                               ucGroupNum;                  /* �Ŷθ�����һ��begin/end��һ���Ŷ� */
    SC_PH_LOCK_CODE_STRU                                    astLockCode[SC_MAX_CODE_NUM];/* ���������ĺŶ�����*/
    /* astLockCodeժҪ���ģ����ɹ��̣�aucSimlockRsaData����astLockCode�Ⱦ���MD5�㷨����ժҪ��
       Ȼ��ʹ��USB dog�е�˽Կ���м��ܵ�У�����ݡ� ��simlock code�Ϸ��Լ���ʱ����ʹ��AK-File��
       �Ĺ�Կ��aucSimlockRsaData���н��ܣ�Ȼ����astLockCodeʹ��MD5���ɵ�ժҪ���бȽϡ�*/
    VOS_UINT8                                               aucSimlockRsaData[SC_RSA_ENCRYPT_LEN];
    /* �����Ľ������룬��16λ��PIN��ʹ��USB dog�е�˽Կ���м��ܵ����� */
    VOS_UINT8                                               aucLockRsaPin[SC_RSA_ENCRYPT_LEN];
    /* ������UnBlock���룬��16λ��PUK��ʹ��USB dog�е�˽Կ���м��ܵ����� */
    VOS_UINT8                                               aucLockRsaPuk[SC_RSA_ENCRYPT_LEN];
}SC_WRITE_SIMLOCK_DATA_STRU;




typedef struct
{
    VOS_UINT8       aucAuthPubKey[SC_AUTH_PUB_KEY_LEN];   /* ��Ȩ��Կ */
    VOS_UINT8       aucSignature[SC_SIGNATURE_LEN];       /* ��Ȩ��Կ��ժҪ��AES-ECB�㷨(��ԿΪSSK)�������ɵ�ժҪǩ�� */
}SC_SET_FAC_AUTH_PUB_KEY_STRU;



typedef struct
{
    /* SC��ȡ����������ù�Կ����RSA���ܺ������ */
    VOS_UINT8       aucIdentifyStartRsp[SC_RSA_ENCRYPT_LEN];
}SC_IDENTIFY_START_RESPONSE_STRU;



typedef struct
{
    /* ��̨ͨ��˽Կ��<identify_start_response>�������������������˽Կ���������������<identify_end_request>��
       ͨ������^IDENTIFYEND����<identify_end_request>���͸�Э��ջ��Э��ջʹ�ø����ݺͲ��߼�Ȩ��Կ��ԭ�������
       ����ԭ�������ԭ�����һ�����Ȩͨ�������򣬼�Ȩʧ�� */
    VOS_UINT8       aucIdentifyEndReq[SC_RSA_ENCRYPT_LEN];
}SC_IDENTIFY_END_REQUEST_STRU;



typedef struct
{
    /* AT����^IDENTIFYEND�еĲ���<onoff>��ֵ */
    SC_PORT_STATUS_ENUM_UINT32          enStatus;

    /* AT����^IDENTIFYEND�еĲ���<port_passwd>��ֵ��<port_passwd>Ϊ16λ���루���볤��
       Ϊ�̶���16����0��-��9���ַ�������RSA�������ɵ�128�ֽ����� */
    VOS_UINT8                           aucRsaPwd[SC_RSA_ENCRYPT_LEN];
}SC_SET_PORT_ATTRIBUTE_STRU;


typedef struct
{
    VOS_UINT8                           aucCKCryptoPwd[SC_CRYPTO_PWD_LEN]; /* CK���� */
    VOS_UINT8                           aucUKCryptoPwd[SC_CRYPTO_PWD_LEN]; /* UK���� */
}SC_CK_CATEGORY_STRU;




typedef struct
{
    /* Ŀǰ֧��3��category���ṹ�����鰴��network->network subset->SP��˳������ */
    SC_SIMLOCK_CATEGORY_STRU            astSimlockCategory[SC_MAX_SUPPORT_CATEGORY];
}SC_PI_FILE_STRU;


typedef struct
{
    /* Ŀǰ֧��3��category���ṹ�����鰴��network->network subset->SP��˳������ */
    SC_CK_CATEGORY_STRU                 astCKCategory[SC_MAX_SUPPORT_CATEGORY];
}SC_CK_FILE_STRU;


typedef struct
{
    SC_PORT_STATUS_ENUM_UINT32          enPortStatus;       /* �˿���״̬������ */
    VOS_UINT8                           aucDKCryptoPwd[SC_CRYPTO_PWD_LEN]; /* DK���� */
}SC_DK_FILE_STRU;



typedef struct
{
    VOS_UINT8                           aucAuthPubKey[SC_AUTH_PUB_KEY_LEN];    /* AK���� */
}SC_AK_FILE_STRU;



typedef struct
{
    VOS_UINT8                           aucSignature[SC_SIGNATURE_LEN];    /* ǩ���ļ����� */
}SC_SIGN_FILE_STRU;


typedef SC_PI_FILE_STRU SC_SIMLOCK_INFO_STRU;

/*****************************************************************************
  8 UNION����
*****************************************************************************/

/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/

extern SC_ERROR_CODE_ENUM_UINT32 SC_PERS_GetSimlockInfo(SC_SIMLOCK_INFO_STRU *pstSimlockInfo);



extern SC_ERROR_CODE_ENUM_UINT32 SC_PERS_WriteSimLockData(SC_WRITE_SIMLOCK_DATA_STRU *pstWriteSimLockData);


extern SC_ERROR_CODE_ENUM_UINT32 SC_PERS_VerifySimLock(
    SC_VERIFY_SIMLOCK_RESULT_ENUM_UINT8 *penVerifySimlockRslt,
    MODEM_ID_ENUM_UINT16                enModemID
);


extern SC_ERROR_CODE_ENUM_UINT32 SC_PERS_VerifyIMEI(MODEM_ID_ENUM_UINT16 enModemID);


SC_ERROR_CODE_ENUM_UINT32 SC_PERS_NoVerifyNvReadImei(
    MODEM_ID_ENUM_UINT16                enModemID,
    VOS_UINT16                          usID,
    VOS_VOID                           *pItem,
    VOS_UINT32                          ulLength
);


SC_ERROR_CODE_ENUM_UINT32 SC_PERS_VerifyNvReadImei(
    MODEM_ID_ENUM_UINT16                enModemID,
    VOS_UINT16                          usID,
    VOS_VOID                           *pItem,
    VOS_UINT32                          ulLength
);


extern SC_ERROR_CODE_ENUM_UINT32 SC_PERS_UnLock(
    VOS_UINT8                          *pucPwd,
    VOS_UINT8                           ucLen,
    SC_VERIFY_SIMLOCK_RESULT_ENUM_UINT8 *penVerifySimlockRslt,
    MODEM_ID_ENUM_UINT16                enModemID
);


extern SC_ERROR_CODE_ENUM_UINT32 SC_FAC_SetFacAuthPubKey(SC_SET_FAC_AUTH_PUB_KEY_STRU *pstSetFacAuthPubKey);



extern SC_ERROR_CODE_ENUM_UINT32 SC_FAC_StartIdentify(SC_IDENTIFY_START_RESPONSE_STRU *pstIdentifyStartRsp);



extern SC_ERROR_CODE_ENUM_UINT32 SC_FAC_EndIdentify(SC_IDENTIFY_END_REQUEST_STRU *pstIdentifyEndReq);



extern SC_ERROR_CODE_ENUM_UINT32 SC_FAC_RsaEncrypt(
    VOS_UINT8   *pucRawData,
    VOS_UINT32   ulLen,
    VOS_UINT8   *pucCipherData,
    VOS_UINT32  *pulCipherLen
);



extern SC_ERROR_CODE_ENUM_UINT32 SC_FAC_RsaDecrypt(
    VOS_UINT8  *pucCipherData,
    VOS_UINT32  ulCipherLen,
    VOS_UINT8  *pucRawData,
    VOS_UINT32 *pulLen
);


extern SC_ERROR_CODE_ENUM_UINT32 SC_FAC_WriteHUK(VOS_UINT8 *pucHuk, VOS_UINT32 ulLen);


extern SC_ERROR_CODE_ENUM_UINT32 SC_FAC_WriteIMEI(MODEM_ID_ENUM_UINT16    enModemID,
                                                  VOS_UINT8              *pucCipherData,
                                                  VOS_UINT32              ulCipherLen);


extern SC_ERROR_CODE_ENUM_UINT32 SC_FAC_SetPortAttrib(SC_SET_PORT_ATTRIBUTE_STRU *pstSetPortAttrib);



extern SC_ERROR_CODE_ENUM_UINT32 SC_FAC_GetPortAttrib(SC_PORT_STATUS_ENUM_UINT32   *penStatus);


extern SC_ERROR_CODE_ENUM_UINT32 SC_FAC_VerifyPortPassword(VOS_UINT8 *pucPwd, VOS_UINT8 ucLen);


extern VOS_UINT32 SC_APSEC_ParseSecPacket(
    VOS_UINT32                         *pulSPLen,
    VOS_UINT8                          *pucSecPacket );


extern VOS_UINT32 SC_PERS_SimlockUnlock(
    SC_PERSONALIZATION_CATEGORY_ENUM_UINT8  enCategory,
    VOS_UINT8                              *pucPwd );

#if (FEATURE_ON == FEATURE_VSIM)

SC_ERROR_CODE_ENUM_UINT32 SC_FAC_SetDhKey(
    MODEM_ID_ENUM_UINT16                enModemID,
    SC_DH_KEY_TYPE_ENUM_UINT32          enKeyType,
    VOS_UINT32                          ulKeyLen,
    VOS_UINT8                          *pucKey
);
#endif


VOS_UINT32 SC_COMM_Backup(VOS_VOID);

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



