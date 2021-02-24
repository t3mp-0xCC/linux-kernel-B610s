/************************************************************************
  Copyright   : 2005-2007, Huawei Tech. Co., Ltd.
  File name   : CstNasInterface.h
  Author      : x44545
  Version     : V100R002
  Date        : 2006-08-01
  Description : ��ͷ�ļ�������CST���ϲ�NAS(����:CCA,AT��DCģ��)֮��Ľӿ�
  History     :
  1. Date     : 2006-08-01
     Author   : x44545
     Modification: Create
************************************************************************/

#ifndef _CST_NAS_INTERFACE_H_
#define _CST_NAS_INTERFACE_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "product_config.h"

/*****************************************************************************
  2 �궨��
*****************************************************************************/


/*******************************************************************************
  3 ö�ٶ���
*******************************************************************************/
/*================*/ /*CST��NAS֮���ԭ��*/
enum CST_TAF_MSG_TYPE_ENUM
{
    ID_CALL_CST_SETUP_REQ               = 0xC102,/* _H2ASN_MsgChoice CALL_CST_SETUP_REQ_STRU */
    ID_CALL_CST_CHANGE_REQ              = 0xC104,/* _H2ASN_MsgChoice CALL_CST_CHANGE_REQ_STRU */
    ID_CALL_CST_REL_REQ                 = 0xC108,/* _H2ASN_MsgChoice CALL_CST_REL_REQ_STRU */
    ID_CST_CALL_SETUP_CNF               = 0xC101,/* _H2ASN_MsgChoice CST_CALL_SETUP_CNF_STRU */
    ID_CST_CALL_CHANGE_CNF              = 0xC103,/* _H2ASN_MsgChoice CST_CALL_CHANGE_CNF_STRU */
    ID_CST_CALL_ERROR_IND               = 0xC105,/* _H2ASN_MsgChoice CST_CALL_ERROR_IND_STRU */
    ID_TRACE_AT_CST_DATA_REQ            = 0xC10e,/* _H2ASN_MsgChoice CST_TRACE_AT_CST_DATA_REQ_STRU */
    ID_TRACE_CST_AT_DATA_IND            = 0xC10f,/* _H2ASN_MsgChoice CST_TRACE_CST_AT_DATA_IND_STRU */
    ID_TRACE_CST_SET_FLOW_CTRL          = 0xC109,/* _H2ASN_MsgChoice CST_TRACE_SET_FLOW_CTRL_STRU */
    ID_TRACE_CST_CLEAR_FLOW_CTRL        = 0xC10A,/* _H2ASN_MsgChoice CST_TRACE_CLEAR_FLOW_CTRL_STRU */

    ID_CST_TAF_MSG_TYPE_BUTT     = 0xFFFF
};
typedef VOS_UINT16 CST_TAF_MSG_TYPE_ENUM_UINT16;


/*================*/ /*CST�ϱ���CCA�Ĵ������*/
enum CST_ERR_IND_CAUSE_ENUM
{
    CST_SETUP_ERROR             = 0,       /*����ʧ��ָʾ*/
    CST_CHANGE_ERROR            = 1,       /*�޸�ʧ��ָʾ*/
    CST_UNSYNC_ERROR            = 2,       /*ʧ��ָʾ */
    CST_RLP_TRANS_ERROR         = 3,       /*RLP����ʧ��ָʾ*/
    CST_IWF_DISC                = 4,        /*RLP�յ�IWF��DISCָʾ*/
    CST_ERR_IND_CAUSE_BUTT
};

typedef VOS_UINT16 CST_ERR_IND_CAUSE_ENUM_UINT16;

/*================*/ /* �������� */
enum CST_RATE_TYPE_ENUM
{
    CST_RATE_VALUE_300 = 1,  /* 300 bit/s */
    CST_RATE_VALUE_1K2,      /* 1.2kbit/s */
    CST_RATE_VALUE_2K4,      /* 2.4kbit/s */
    CST_RATE_VALUE_4K8,      /* 4.8kbit/s */
    CST_RATE_VALUE_9K6,      /* 9.6kbit/s */
    CST_RATE_VALUE_12K,      /* 12kbit/s */
    CST_RATE_VALUE_14K4,     /* 14.4kbit/s */
    CST_RATE_VALUE_19K2,     /* 19.2kbit/s */
    CST_RATE_VALUE_28K8,     /* 28.8kbit/s */
    CST_RATE_VALUE_31K2,     /* 31.2kbit/s */
    CST_RATE_VALUE_32K,      /* 32kbit/s */
    CST_RATE_VALUE_33K6,     /* 33.6kbit/s */
    CST_RATE_VALUE_38K4,     /* 38.4kbit/s */
    CST_RATE_VALUE_48K,      /* 48kbit/s */
    CST_RATE_VALUE_56K,      /* 56kbit/s */
    CST_RATE_VALUE_64K,      /* 64kbit/s */
    CST_RATE_BUTT
};

typedef VOS_UINT8 CST_RATE_TYPE_ENUM_UINT8;

/*================*/ /* �����ŵ����� */
enum CST_TCH_TYPE_ENUM
{
    CST_TCH_NULL    = 0,
    CST_TCH_F_24    = 1,                        /* ȫ����TCH_2.4 */
    CST_TCH_H_24,                               /* ������TCH_2.4 */
    CST_TCH_F_48,                               /* ȫ����TCH_4.8 */
    CST_TCH_H_48,                               /* ������TCH_4.8 */
    CST_TCH_F_96,                               /* ȫ����TCH_9.6 */
    CST_TCH_F_144,                              /* ȫ����TCH_14.4 */
    CST_TCH_BUTT
};

typedef VOS_UINT8 CST_TCH_TYPE_ENUM_UINT8;

/*================*/ /* ��żУ������ */
enum CST_PARITY_TYPE_ENUM
{
    CST_ODD_PARITY                  = 0,        /* ��У��   */
    CST_EVEN_PARITY                 = 2,        /* żУ��   */
    CST_NONE_PARITY                 = 3,        /* ��У��   */
    CST_FORC_0_PARITY               = 4,        /* ǿ�Ƽ�0 */
    CST_FORC_1_PARITY               = 5,        /* ǿ�Ƽ�1 */
    CST_PARITY_BUTT
};

typedef VOS_UINT8 CST_PARITY_TYPE_ENUM_UINT8;

/*================*/ /* ���ݵ�ͬ�첽���� */
enum CST_SYNC_TYPE_ENUM
{
    CST_SYNC_TYPE_SYNC              = 0,
    CST_SYNC_TYPE_ASYNC             = 1,
    CST_SYNC_TYPE_BUTT
};

typedef VOS_UINT8 CST_SYNC_TYPE_ENUM_UINT8;

/*================*/ /* ҵ���͸��/��͸������ */
enum CST_TRAN_TYPE_ENUM
{
    CST_TRAN_TYPE_TRANSPARENT       = 0,
    CST_TRAN_TYPE_NON_TRANSPARENT   = 1,
    CST_TRAN_TYPE_BUTT
};

typedef VOS_UINT8 CST_TRAN_TYPE_ENUM_UINT8;

/*================*/ /* ���������� */
enum CST_TRAN_MODE_ENUM
{
    CST_TRAN_MODE_GSM               = 0,
    CST_TRAN_MODE_UMTS              = 1,
    CST_TRAN_MODE_BUTT
};

typedef VOS_UINT8 CST_TRAN_MODE_ENUM_UINT8;

/*================*/ /* ���ݵı���λ���� */
enum CST_DATA_BIT_NUM_ENUM
{
    CST_DATA_BIT_NUM_7              = 0,
    CST_DATA_BIT_NUM_8              = 1,
    CST_DATA_BIT_NUM_BUTT
};

typedef VOS_UINT8 CST_DATA_BIT_NUM_ENUM_UINT8;

/*================*/ /* Stop����λ�ĸ��� */
enum CST_STOP_BIT_NUM_ENUM
{
    CST_STOP_BIT_NUM_1              = 0,
    CST_STOP_BIT_NUM_2              = 1,
    CST_STOP_BIT_NUM_BUTT
};

typedef VOS_UINT8 CST_STOP_BIT_NUM_ENUM_UINT8;

/*================*/ /* �Ƿ����ѹ�� */
enum CST_COMPRESS_ENUM
{
    CST_COMPRESS_NOT_ADOPT          = 0,        /* ������ѹ�� */
    CST_COMPRESS_ADOPT              = 1,        /* ����ѹ�� */
    CST_COMPRESS_BUTT
};

typedef VOS_UINT8 CST_COMPRESS_ENUM_UINT8;

/*================*/ /* ���з���ö�� */
enum CST_CALL_DIR_ENUM
{
    CST_CALL_DIR_MO             = 0,
    CST_CALL_DIR_MT             = 1,
    CST_CALL_DIR_BUTT
};

typedef VOS_UINT8 CST_CALL_DIR_ENUM_UINT8;

/*================*/ /* ��������ö�� */
enum CST_FAX_CLASS_ENUM
{
    CST_FAX_CLASS_1_0           = 1,
    CST_FAX_CLASS_BUTT
};

typedef VOS_UINT8 CST_FAX_CLASS_ENUM_UINT8;

/*****************************************************************************
  4 STRUCT����
*****************************************************************************/
#pragma pack(4)

/*****************************************************************************
 �ṹ��    : CST_SETUP_PARAM_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :CALL_CST_SETUP_REQ_STRU�ṹ�еĲ����ṹ
*****************************************************************************/
typedef struct
{
    CST_TRAN_TYPE_ENUM_UINT8    enTranType;      /* ҵ��͸��/��͸����������*/
    CST_TRAN_MODE_ENUM_UINT8    enTranMode;      /* GSM����WCDMA*/
    VOS_UINT8                   ucRbId;          /* RbId,��������ģʽ��WCDMAʱ������*/
    VOS_UINT8                   ucRsv[3];
    CST_RATE_TYPE_ENUM_UINT8    enUsrRate;       /* �û�����*/
    CST_TCH_TYPE_ENUM_UINT8     enTchType;       /* �����ŵ����� */

    CST_PARITY_TYPE_ENUM_UINT8  enParityType;    /*��żУ������*/
    CST_SYNC_TYPE_ENUM_UINT8    enSyncType;      /*��������:0Ϊͬ������ 1Ϊ�첽����*/
    CST_DATA_BIT_NUM_ENUM_UINT8 enDataBitNum;    /*���ݵı���λ����*/
    CST_STOP_BIT_NUM_ENUM_UINT8 enStopBitNum;    /*Stop����λ�ĸ���*/

    CST_COMPRESS_ENUM_UINT8     enCompressAdopt; /*�Ƿ����ѹ��
                                                   1: ���� 2: �ܾ�;
                                                   �����ڷ�͸��ģʽ*/
    VOS_UINT8                   ucFaxCall;       /* �Ƿ���, VOS_YES:����ҵ��, VOS_NO:�Ǵ���ҵ��*/
    CST_CALL_DIR_ENUM_UINT8     enCallDir;       /* ���з��� */
    CST_FAX_CLASS_ENUM_UINT8    enFaxClass;      /* �������� */
}CST_SETUP_PARAM_STRU;

/*****************************************************************************
 �ṹ��    : CALL_CST_SETUP_REQ_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :ID_CALL_CST_SETUP_REQ��Ӧ����Ϣ�ṹ
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                         /* ��Ϣͷ */ /*_H2ASN_Skip*/
    VOS_UINT16                  usMsgType; /* ��Ϣ���� */ /*_H2ASN_Skip*/
    VOS_UINT16                  usTransId; /* �����ʶ */
    CST_SETUP_PARAM_STRU        stCSTPara;
}CALL_CST_SETUP_REQ_STRU;

/*****************************************************************************
 �ṹ��    : CST_ATRLP_PARAM_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :CALL_CST_CHANGE_REQ_STRU�еĲ����ṹ
*****************************************************************************/
typedef struct
{
    CST_TRAN_MODE_ENUM_UINT8    enTranMode;     /* GSM����WCDMA*/
    CST_TCH_TYPE_ENUM_UINT8     enTchType;      /* �ŵ�����*/
    VOS_UINT8                   aucRsv[2];      /* ����*/
}CST_CHANGE_PARAM_STRU;

/*****************************************************************************
 �ṹ��    : CALL_CST_CHANGE_REQ_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :ID_CALL_CST_CHANGE_REQ��Ӧ����Ϣ�ṹ
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                            /* ��Ϣͷ */ /*_H2ASN_Skip*/
    VOS_UINT16                  usMsgType;    /* ��Ϣ���� */ /*_H2ASN_Skip*/
    VOS_UINT16                  usTransId;
    CST_CHANGE_PARAM_STRU       stChangPara;
}CALL_CST_CHANGE_REQ_STRU;

/*****************************************************************************
 �ṹ��    : CALL_CST_REL_REQ_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :ID_CALL_CST_REL_REQ��Ӧ����Ϣ�ṹ
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                          /* ��Ϣͷ */ /*_H2ASN_Skip*/
    VOS_UINT16          usMsgType;          /* ��Ϣ���� */ /*_H2ASN_Skip*/
    VOS_UINT16          usTransId;
}CALL_CST_REL_REQ_STRU;

/*****************************************************************************
 �ṹ��    : CST_CALL_SETUP_CNF_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :CST���óɹ���,�ϱ���CCAȷ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                          /* ��Ϣͷ */ /*_H2ASN_Skip*/
    VOS_UINT16          usMsgType;          /* ��Ϣ���� */ /*_H2ASN_Skip*/
    VOS_UINT16          usTransId;
}CST_CALL_SETUP_CNF_STRU;

/*****************************************************************************
 �ṹ��    : CST_CALL_CHANGE_CNF_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :CST�޸����óɹ���,�ϱ���CCAȷ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                          /* ��Ϣͷ */ /*_H2ASN_Skip*/
    VOS_UINT16          usMsgType;          /* ��Ϣ���� */ /*_H2ASN_Skip*/
    VOS_UINT16          usTransId;
}CST_CALL_CHANGE_CNF_STRU;

/*****************************************************************************
 �ṹ��    : CST_CALL_ERROR_IND_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :CST����CCA�Ĵ���״ָ̬ʾ
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                              /* ��Ϣͷ */ /*_H2ASN_Skip*/
    VOS_UINT16                      usMsgType;  /* ��Ϣ���� */ /*_H2ASN_Skip*/
    VOS_UINT16                      usTransId;
    CST_ERR_IND_CAUSE_ENUM_UINT16   usErrId;    /* ����ָʾID,�����CST_ERR_IND_CAUSE_ENUM*/
    VOS_UINT8                       aucRsv[2];  /* ���� */
}CST_CALL_ERROR_IND_STRU;


/*****************************************************************************
 �ṹ��    : CST_TRACE_AT_CST_DATA_REQ_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :����ϢID_CST_TRACE_AT_CST_DATA_REQ�Ľṹ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                          /* ��Ϣͷ */ /*_H2ASN_Skip*/
    VOS_UINT16                      usMsgType;          /* ��Ϣ���� */ /*_H2ASN_Skip*/

    VOS_UINT16                      ulDataLen;          /* ���ݳ���, ��λ: BYTE */
    VOS_UINT8                       aucData[4];         /* ���ݲ��֣���ǰ���ֽ�һ��������������Ϣ�ڴ�ռ� */
    /******************************************************************************************************
         _H2ASN_Array2String
    ******************************************************************************************************/

} CST_TRACE_AT_CST_DATA_REQ_STRU;

/*****************************************************************************
 �ṹ��    : CST_TRACE_CST_AT_DATA_IND_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :����ϢID_CST_TRACE_CST_AT_DATA_IND�Ľṹ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                          /* ��Ϣͷ */ /*_H2ASN_Skip*/
    VOS_UINT16                     usMsgType;          /* ��Ϣ���� */ /*_H2ASN_Skip*/

    VOS_UINT16                     ulDataLen;           /* ���ݵĳ���, ��λ: BYTE*/
    VOS_UINT8                      aucData[4];          /* ���ݲ��֣���ǰ���ֽ�һ��������������Ϣ�ڴ�ռ� */
    /******************************************************************************************************
       _H2ASN_Array2String
    ******************************************************************************************************/

} CST_TRACE_CST_AT_DATA_IND_STRU;

/*****************************************************************************
 �ṹ��    : CST_TRACE_SET_FLOW_CTRL_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :����ϢID_TRACE_CST_SET_FLOW_CTRL�Ľṹ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                          /* ��Ϣͷ */ /*_H2ASN_Skip*/
    VOS_UINT16                     usMsgType;          /* ��Ϣ���� */ /*_H2ASN_Skip*/

    VOS_UINT16                     usBuffUsedLen;           /* ���ݵĳ���, ��λ: BYTE*/
} CST_TRACE_SET_FLOW_CTRL_STRU;

/*****************************************************************************
 �ṹ��    : CST_TRACE_CLEAR_FLOW_CTRL_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :����ϢID_TRACE_CST_CLEAR_FLOW_CTRL�Ľṹ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                          /* ��Ϣͷ */ /*_H2ASN_Skip*/
    VOS_UINT16                     usMsgType;          /* ��Ϣ���� */ /*_H2ASN_Skip*/
    VOS_UINT16                     usBuffUsedLen;           /* ���ݵĳ���, ��λ: BYTE*/

} CST_TRACE_CLEAR_FLOW_CTRL_STRU;


/*****************************************************************************
 �ṹ��    : CST_UL_MODEM_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :������״̬��Ϣ�ṹ, DCģ���ô˽ṹ���´���״̬��Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT8           ucCt105;        /* ����:MT->IWF, ���� SB */
    VOS_UINT8           ucCt1082;       /* ����:MT->IWF, ���� SA */
    VOS_UINT8           ucCt133;        /* ����:MT->IWF, ���� X */
    VOS_UINT8           ucBreakSig;     /* break�ź���Ч:1 , ��Ч:0 */
}CST_UL_MODEM_STRU;
/*****************************************************************************
 �ṹ��    : AT_DL_MODEM_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :����״̬��Ϣ,�ϱ���ATģ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8           ucCt107;        /* ����:IWF->MT, ���� SA */
    VOS_UINT8           ucCt109;        /* ����:IWF->MT, ���� SB */
    VOS_UINT8           ucCt106;        /* ����:IWF->MT, ���� X */
    VOS_UINT8           ucBreakSig;     /* break�ź���Ч:1 , ��Ч:0 */
}AT_DL_MODEM_STRU;
/*****************************************************************************
 �ṹ��    : CST_ATRLP_PARAM_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :
*****************************************************************************/
typedef struct
{
    VOS_UINT32          OP_Ver      : 1;    /* ucVer�Ƿ���Ч.1:��Ч,0:��Ч */
    VOS_UINT32          OP_KIM      : 1;    /* ucKIM�Ƿ���Ч.1:��Ч,0:��Ч */
    VOS_UINT32          OP_KMI      : 1;    /* ucKMI�Ƿ���Ч.1:��Ч,0:��Ч */
    VOS_UINT32          OP_TimerT1  : 1;    /* ucTimerT1�Ƿ���Ч.1:��Ч,0:��Ч */
    VOS_UINT32          OP_CountN2  : 1;    /* ucCountN2�Ƿ���Ч.1:��Ч,0:��Ч */
    VOS_UINT32          OP_TimerT4  : 1;    /* ucTimerT4�Ƿ���Ч.1:��Ч,0:��Ч */
    VOS_UINT8           ucVer;              /* RLP�İ汾: ��0,1,2���� */
    VOS_UINT8           ucKIM;              /* ���մ��ڵĴ�С��RLP�汾0��1ʱ����λ:1; RLP�汾2ʱ����λ:8 */
    VOS_UINT8           ucKMI;              /* ���ʹ��ڵĴ�С��RLP�汾0��1ʱ����λ:1; RLP�汾2ʱ����λ:8 */
    VOS_UINT8           ucTimerT1;          /* ��ʱ��T1�ĳ��ȣ���λ:10ms */
    VOS_UINT8           ucCountN2;          /* �ش��Ĵ��� */
    VOS_UINT8           ucTimerT4;          /* ��ʱ��T4�ĳ��ȣ���λ:10ms */
    VOS_UINT8           aucRsv[2];          /* ���� */
}CST_ATRLP_PARAM_STRU;


/*****************************************************************************
  H2ASN������Ϣ�ṹ����
*****************************************************************************/
typedef struct
{
    CST_TAF_MSG_TYPE_ENUM_UINT16        enMsgID;    /*_H2ASN_MsgChoice_Export CST_TAF_MSG_TYPE_ENUM_UINT16*/

    VOS_UINT8                           aucMsgBlock[2];
    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          CST_TAF_MSG_TYPE_ENUM_UINT16
    ****************************************************************************/
}CST_TAF_MSG_DATA;
/*_H2ASN_Length UINT32*/

typedef struct
{
    VOS_MSG_HEADER
    CST_TAF_MSG_DATA                    stMsgData;
}CstNasInterface_MSG;



#pragma pack(0)

/*****************************************************************************
  5 UNION����
*****************************************************************************/

/*****************************************************************************
  6 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  7 ��������
*****************************************************************************/
#if (FEATURE_ON == FEATURE_CSD)
extern VOS_UINT32  AT_CstDlDataInd(VOS_UINT8 *pDlData, VOS_UINT16 usDataLen,
    AT_DL_MODEM_STRU *pstModem);
#endif


extern VOS_UINT32 CST_SendCtrlInfVal(CST_UL_MODEM_STRU *pstCtrlInf);
extern VOS_VOID DC_CstActFlowCtrl(VOS_BOOL bActive);
extern VOS_UINT32 CST_SetRlpParam(CST_ATRLP_PARAM_STRU* pstParam);
extern VOS_UINT32 CST_QueryRlpParam(CST_ATRLP_PARAM_STRU* pstParam);

/*****************************************************************************
  8 OTHERS����
*****************************************************************************/


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* The end*/


