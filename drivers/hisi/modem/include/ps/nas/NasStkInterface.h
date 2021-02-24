
#ifndef __NAS_STK_INTERFACE_H__
#define __NAS_STK_INTERFACE_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "MnCallApi.h"
#include "TafAppMma.h"
#include "TafAppSsa.h"
#include "MnMsgApi.h"
#include "MnClient.h"



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)


/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define NAS_STK_CALL_DISC_CAUSE_LEN     (3)
#define NAS_STK_CURC_RPT_CFG_MAX_SIZE   (8)


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

/*****************************************************************************
 �ṹ����: NAS_STK_MSG_ID_ENUM
 �ṹ˵��: NAS��STK֮���ԭ��
*****************************************************************************/
enum NAS_STK_MSG_ID_ENUM
{
    /* ��STK_SS_REGISTERSS_REQ��TAF_STK_CS_STATE_IND��Ϣ���ṹֻ�Ǵ�TafOamInterface.h���Ƴ���������δ�޸� */
    /* STK  -> SSA */
    STK_SS_REGISTERSS_REQ               = 0x02,                                 /* _H2ASN_MsgChoice MN_APP_SS_REGISTER_REQ_STRU */
    STK_SS_ERASESS_REQ                  = 0x03,                                 /* _H2ASN_MsgChoice MN_APP_SS_ERASE_REQ_STRU */
    STK_SS_ACTIVATESS_REQ               = 0x04,                                 /* _H2ASN_MsgChoice MN_APP_SS_ACTIVATE_REQ_STRU */
    STK_SS_DEACTIVATESS_REQ             = 0x05,                                 /* _H2ASN_MsgChoice MN_APP_SS_DEACTIVATE_REQ_STRU */
    STK_SS_INTERROGATESS_REQ            = 0x06,                                 /* _H2ASN_MsgChoice MN_APP_SS_INTERROGATE_REQ_STRU */
    STK_SS_REGPWD_REQ                   = 0x07,                                 /* _H2ASN_MsgChoice MN_APP_SS_REGPWD_REQ_STRU */

    STK_SS_USSD_REQ                     = 0x09,                                 /* _H2ASN_MsgChoice MN_APP_SS_USSD_REQ_STRU */

    /* STK  -> TAF(MSG) */
    STK_MSG_SEND_REQ                    = 0x0a,                                 /* _H2ASN_MsgChoice MN_APP_MSG_SEND_REQ_STRU */

    /* STK  -> TAF(CALL) */
    STK_CALL_START_DTMF_REQ             = 0x0b,                                 /* _H2ASN_MsgChoice MN_APP_CALL_START_DTMF_REQ_STRU */
    STK_CALL_STOP_DTMF_REQ              = 0x0c,                                 /* _H2ASN_MsgChoice MN_APP_CALL_STOP_DTMF_REQ_STRU */

    STK_CALL_CALLORIG_REQ               = 0x0d,                                 /* _H2ASN_MsgChoice MN_APP_CALL_CALLORIG_REQ_STRU */
    STK_CALL_SUPS_CMD_REQ               = 0x0e,                                 /* _H2ASN_MsgChoice MN_APP_CALL_SUPS_REQ_STRU */

    /* TAF -> STK */
    TAF_STK_CMD_CNF                     = 0x10,                                 /* _H2ASN_MsgChoice MN_APP_CMD_CNF_STRU */

    /* TAF(SSA)-> STK */
    TAF_STK_SS_EVENT_IND                = 0x13,                                 /* _H2ASN_MsgChoice MN_APP_SS_EVENT_IND_STRU */

    /* TAF(SMS)-> STK */
    TAF_STK_MSG_EVENT_IND               = 0x14,                                 /* _H2ASN_MsgChoice MN_APP_MSG_EVENT_IND_STRU */

    /* TAF(CALL)-> STK */
    TAF_STK_CS_CALLINFO_EVENT_IND       = 0x15,                                 /* _H2ASN_MsgChoice MN_APP_CS_CALLINFO_MSG_STRU */

    /* TAF-> STK */
    TAF_STK_CURC_RPT_CFG_INFO_IND       = 0x16,                                 /* _H2ASN_MsgChoice TAF_STK_CURC_RPT_CFG_INFO_MSG_STRU */

    /* TAF-> STK */
    TAF_STK_CS_STATE_IND                = 0x17,                                 /* _H2ASN_MsgChoice TAF_STK_CS_SERVICE_IND */

    /*------------------------------------------------------------------
                        NAS --->STK EVENT DOWNLOAD
     *-----------------------------------------------------------------*/

    ID_NAS_STK_MT_CALL_EVENT            = 0x18,                                 /* _H2ASN_MsgChoice NAS_STK_MT_CALL_EVENT_INFO_STRU */
    ID_NAS_STK_CALL_CONNECT_EVENT       = 0x19,                                 /* _H2ASN_MsgChoice NAS_STK_CALL_CONNECT_EVENT_STRU */
    ID_NAS_STK_CALL_DISC_EVENT          = 0x1a,                                 /* _H2ASN_MsgChoice NAS_STK_CALL_DISC_EVENT_INFO_STRU */
    ID_NAS_STK_LOC_STATUS_EVENT         = 0x1b,                                 /* _H2ASN_MsgChoice NAS_STK_LOC_STATUS_EVENT_STRU */
    ID_NAS_STK_NET_SRCH_MODE_CHG_EVENT  = 0x1c,                                 /* _H2ASN_MsgChoice NAS_STK_SRCHMODE_CHG_EVENT_STRU */
    ID_NAS_STK_NETWORK_REJECTION_EVENT  = 0x1d,                                 /* _H2ASN_MsgChoice NAS_STK_NETWORK_REJECTION_EVENT_STRU*/

    ID_NAS_STK_PS_CALLINFO_EVENT        = 0x1e,                                 /* _H2ASN_MsgChoice NAS_STK_PS_CALLINFO_EVENT_STRU */

    /*------------------------------------------------------------------
                        STK����״̬ά����Ϣ
     *-----------------------------------------------------------------*/
    ID_NAS_STK_LOCATION_INFO_IND         = 0x100,                               /* _H2ASN_MsgChoice NAS_STK_LOCATION_INFO_IND_STRU */
    ID_MMA_STK_1X_SERVICE_INFO_IND       = 0x101,                               /* _H2ASN_MsgChoice MMA_STK_SERVICE_INFO_IND_STRU */
    ID_NAS_STK_BUTT                      = 0xffffffff
};
typedef VOS_UINT32 NAS_STK_MSG_ID_ENUM_UINT32;


enum NAS_STK_UTRAN_MODE_ENUM
{
    NAS_STK_UTRAN_MODE_FDD,                                               /* UTRANģʽΪWCDMA */
    NAS_STK_UTRAN_MODE_TDD,                                               /* UTRANģʽΪTDSCDMA */
    NAS_STK_UTRAN_MODE_BUTT
};
typedef VOS_UINT8 NAS_STK_UTRAN_MODE_ENUM_UINT8;


enum NAS_STK_SERVICE_STATUS_ENUM
{
    NAS_STK_SERVICE_STATUS_NORMAL_SERVICE           = 0,                        /* �������� */
    NAS_STK_SERVICE_STATUS_LIMITED_SERVICE          = 1,                        /* ���Ʒ��� */
    NAS_STK_SERVICE_STATUS_LIMITED_SERVICE_REGION   = 2,                        /* ������������� */
    NAS_STK_SERVICE_STATUS_NO_IMSI                  = 3,                        /* ��imsi */
    NAS_STK_SERVICE_STATUS_NO_SERVICE               = 4,                        /* �޷��� */
    NAS_STK_SERVICE_STATUS_DEEP_SLEEP               = 6,
    NAS_STK_SERVICE_STATUS_BUTT
};
typedef VOS_UINT8 NAS_STK_SERVICE_STATUS_ENUM_UINT8;


/*****************************************************************************
 �ṹ����: NAS_STK_NETWORK_REJECT_TYPE_ENUM
 �ṹ˵��: ֪ͨSTK������Reject����
*****************************************************************************/
enum NAS_STK_NETWORK_REJECT_TYPE_ENUM
{
    NAS_STK_ATTACH_REJECT,
    NAS_STK_RAU_REJECT,
    NAS_STK_LAU_REJECT,
    NAS_STK_TAU_REJECT,
    NAS_STK_NETWORK_REJECT_TYPE_BUTT
};
typedef VOS_UINT8 NAS_STK_NETWORK_REJECT_TYPE_ENUM_UINT8;

/*****************************************************************************
 �ṹ����: NAS_STK_UPDATE_TYPE_ENUM
 �ṹ˵��: ֪ͨSTK��Update Type
    �ο�Э��31111 8.92  Update/Attach Type����STK�����ӳ���ϵ:
    NAS_STK_LUT_NORMAL_LU                                        '00' = "Normal Location Updating" in the case of a LOCATION UPDATING REQUEST message;
    NAS_STK_LUT_PERIODIC_UPDATING                                '01' = "Periodic Updating" in the case of a LOCATION UPDATING REQUEST message;
    NAS_STK_LUT_IMSI_ATTACH                                      '02' = "IMSI Attach" in the case of a LOCATION UPDATING REQUEST message;
    NAS_STK_ATTACH_NORMAL & PS_STK_ATTACH_NORMAL_CS_TRANS        '03' = "GPRS Attach" in the case of a GPRS ATTACH REQUEST message;
    NAS_STK_ATTACH_COMBINED & PS_STK_ATTACH_WHILE_IMSI_ATTACHED  '04' = "Combined GPRS/IMSI Attach" in the case of a GPRS ATTACH REQUEST message;
    NAS_STK_RAU_NORMAL & PS_STK_RAU_NORMAL_CS_TRANS & PS_STK_RAU_NORMAL_CS_UPDATED
                                                                '05' = "RA Updating" in the case of a ROUTING AREA UPDATE REQUEST message;
    NAS_STK_RAU_COMBINED                                         '06' = "Combined RA/LA Updating" in the case of a ROUTING AREA UPDATE REQUEST message;
    NAS_STK_RAU_WITH_IMSI_ATTACH                                 '07' = "Combined RA/LA Updating with IMSI Attach" in the case of a ROUTING AREA UPDATE REQUEST message;
    NAS_STK_RAU_PERIODIC_UPDATING                                '08' = "Periodic Updating" in the case of a ROUTING AREA UPDATE REQUEST message
                '09' = "EPS Attach" in the case of an EMM ATTACH REQUEST message
                '0A' = "Combined EPS/IMSI Attach" in the case of an EMM ATTACH REQUEST message
                '0B' = "TA updating " in the case of an EMM TRACKING AREA UPDATE REQUEST message
                '0C' = "Combined TA/LA updating" in the case of an EMM TRACKING AREA UPDATE REQUEST message
                '0D' = "Combined TA/LA updating with IMSI attach" in the case of an EMM TRACKING AREA UPDATE REQUEST message
                '0E' = "Periodic updating" in the case of an EMM TRACKING AREA UPDATE REQUEST message


*****************************************************************************/
enum NAS_STK_UPDATE_TYPE_ENUM
{
    NAS_STK_LUT_NORMAL_LU                   = 0,
    NAS_STK_LUT_PERIODIC_UPDATING           = 1,
    NAS_STK_LUT_IMSI_ATTACH                 = 2,

    NAS_STK_ATTACH_COMBINED                 = 0x10,                             /* combined attach                          */
    NAS_STK_ATTACH_WHILE_IMSI_ATTACHED      = 0x11,                             /* GPRS attach while IMSI attached          */
    NAS_STK_ATTACH_NORMAL                   = 0x12,                             /* normal attach                            */
    NAS_STK_ATTACH_NORMAL_CS_TRANS          = 0x13,                             /* CSͨ���е�normal attach(A+I)             */

    NAS_STK_RAU_COMBINED                    = 0x20,                             /* combined RAU                             */
    NAS_STK_RAU_WITH_IMSI_ATTACH            = 0x21,                             /* combined RAU with IMSI attach            */
    NAS_STK_RAU_NORMAL                      = 0x22,                             /* normal RAU                               */
    NAS_STK_RAU_NORMAL_CS_TRANS             = 0x23,                             /* CSͨ���е�normal RAU(A+I)                */
    NAS_STK_RAU_NORMAL_CS_UPDATED           = 0x24,
    NAS_STK_RAU_PERIODIC_UPDATING           = 0x25,

    NAS_STK_UPDATE_TYPE_ENUM_BUTT
};
typedef VOS_UINT8 NAS_STK_UPDATE_TYPE_ENUM_UINT8;



enum TAF_CS_SERVICE_ENUM
{
    TAF_CS_OFF              = 0,
    TAF_CS_ON,
    TAF_CS_SERVICE_BUTT
};
typedef VOS_UINT32 TAF_CS_SERVICE_ENUM_UINT32;





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
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
}NAS_STK_EVENT_MSG_HEADER_STRU;



typedef struct
{
    TAF_PLMN_ID_STRU                    stCurPlmnID;
    VOS_UINT16                          usLac;
    VOS_UINT8                           aucRsv[2];
}NAS_STK_LAC_INFO_STRU;


typedef struct
{
    TAF_PLMN_ID_STRU                    stCurPlmnID;
    VOS_UINT16                          usLac;
    VOS_UINT8                           ucRac;
    VOS_UINT8                           ucRsv;
}NAS_STK_RAU_INFO_STRU;


typedef struct
{
    TAF_PLMN_ID_STRU                    stCurPlmnID;
    VOS_UINT16                          usTac;
    VOS_UINT8                           aucRsv[2];
}NAS_STK_TAU_INFO_STRU;


typedef struct
{
    TAF_MMA_RAT_TYPE_ENUM_UINT8              enRat;
    NAS_STK_UPDATE_TYPE_ENUM_UINT8           enUpdateAttachType;
    VOS_UINT8                                ucCauseCode;

    NAS_STK_NETWORK_REJECT_TYPE_ENUM_UINT8    enRejType;
    union
    {
        NAS_STK_LAC_INFO_STRU            stLacInfo;
        NAS_STK_RAU_INFO_STRU            stRauInfo;
        NAS_STK_TAU_INFO_STRU            stTauInfo;
    }uRejectInfo;
}NAS_STK_NETWORK_REJECTION_EVENT_INFO_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                              ulMsgName;
    NAS_STK_NETWORK_REJECTION_EVENT_INFO_STRU               stNetworkRejectionEvent;
}NAS_STK_NETWORK_REJECTION_EVENT_STRU;



/*****************************************************************************
 �ṹ��    : NAS_STK_MT_CALL_EVENT_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : CALL����STKģ���MT CALL EVENT��Ϣ��Ϣ�ֶνṹ
             stMtCallEvent MT CALL EVENT��Ϣ�ֶ�
             �ο�Э��10223 7.5.1.2 Structure of ENVELOPE (EVENT DOWNLOAD - MT call)
                     11.14 11.1.2  Structure of ENVELOPE (EVENT DOWNLOAD - MT call)
             ucTi                           Transaction identifier
             stCallerNumber                 Address������ucNumLenΪ0��ʾû�����к���
             stSubCallNumber                Subaddress������IsExistΪ0��ʾû�����к����ӵ�ַ
                                                            LastOctOffset��ʾ���к����ӵ�ַ����
                                                            Octet3���к����ӵ�ַ�ĵ�һ���ֽ�
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucTi;
    VOS_UINT8                           aucReserved1[3];
    MN_CALL_BCD_NUM_STRU                stCallerNumber;
    MN_CALL_SUBADDR_STRU                stCallerSubNumber;
}NAS_STK_MT_CALL_EVENT_INFO_STRU;

/*****************************************************************************
 �ṹ��    : NAS_STK_MT_CALL_EVENT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : CALL����STKģ���MT CALL EVENT��Ϣ�ṹ
             stMtCallEvent MT CALL EVENT��Ϣ�ֶ�
             �ο�Э��10223 7.5.1.2 Structure of ENVELOPE (EVENT DOWNLOAD - MT call)
                      11.14 11.1.2 Structure of ENVELOPE (EVENT DOWNLOAD - MT call)
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    NAS_STK_MT_CALL_EVENT_INFO_STRU     stMtCallEventInfo;
} NAS_STK_MT_CALL_EVENT_STRU;

/*****************************************************************************
 �ṹ��    : NAS_STK_CALL_CONNECT_EVENT_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : CALL����STKģ���CALL CONNECT EVENT���������Ϣ��Ϣ�ֶνṹ
             MN_CALL_DIR_ENUM_U8 enCallDir ָʾ����ͨ���ĺ����Ƿ�UE����ĺ���
             stCallConnectEvent  CALL CONNECT EVENT��Ϣ�ֶ�
             �ο�Э��10223 7.5.2.2 Structure of ENVELOPE (EVENT DOWNLOAD - call connected)
                     11.14 11.2.2  Structure of ENVELOPE (EVENT DOWNLOAD - call connected)
             ucTi                           Transaction identifier
             enCallDir                      ���з���
                                            in the case of connecting at the near end (an MT call), the terminal
                                            shall set the device identities to:
                                            - source: terminal;
                                            - destination: UICC.
                                            in the case of connecting at the far end (an MO call), the terminal
                                            shall set the device identities to:
                                            - source: network;
                                            - destination: UICC.
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucTi;
    MN_CALL_DIR_ENUM_U8                 enCallDir;
    VOS_UINT8                           aucRsv[2];
}NAS_STK_CALL_CONNECT_EVENT_INFO_STRU;

/*****************************************************************************
 �ṹ��    : NAS_STK_CALL_CONNECT_EVENT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : CALL����STKģ���CALL CONNECT EVENT���������Ϣ�ṹ
             stCallConnectEvent  CALL CONNECT EVENT��Ϣ�ֶ�
             �ο�Э��10223 7.5.2.2 Structure of ENVELOPE (EVENT DOWNLOAD - call connected)
                     11.14 11.2.2  Structure of ENVELOPE (EVENT DOWNLOAD - call connected)
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                           ulMsgId;
    NAS_STK_CALL_CONNECT_EVENT_INFO_STRU stCallConnectEventInfo;
} NAS_STK_CALL_CONNECT_EVENT_STRU;

/*****************************************************************************
 �ṹ��    : NAS_STK_CALL_DISC_CAUSE_STRU
 Э����  : CALL����STKģ���CALL CONNECT EVENT��Ϣ�ͷ�ԭ��ֵ�ṹ
 ASN.1���� :
 �ṹ˵��  :
             ucExistFlg         ָʾ�Ƿ����ԭ��ֵ�ֶΣ���ΪҪ֧���ֶδ��ڵ�û��ԭ��ֵ�������
                                ucExistFlgΪVOS_TRUE��ucLenΪ0��ʾaucCause��Ч
             ucLen              ָʾԭ��ֵ�ֶγ���
             �ο�Э��10223 7.5.3 Call disconnected event
                     11.14 11.3  Call disconnected event
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucExistFlg;
    VOS_UINT8                           ucLength;
    VOS_UINT8                           aucCause[NAS_STK_CALL_DISC_CAUSE_LEN];
    VOS_UINT8                           aucReserved1[3];
}NAS_STK_CALL_DISC_CAUSE_STRU;

/*****************************************************************************
 �ṹ��    : NAS_STK_CALL_DISC_EVENT_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : CALL����STKģ���CALL CONNECT EVENT��Ϣ��Ϣ�ֶνṹ
             ucLocalDiscFlg      ָʾ�Ƿ񱾵��ͷ�
             stCallDiscEvent     CALL DISCONNECTED EVENT��Ϣ�ֶ�
             �ο�Э��10223 7.5.3 Call disconnected event
                     11.14 11.3  Call disconnected event
            ucTi                           Transaction identifier
            ucNearEndFlg                   �����ͷű�־��
                                            in the case of "near end" disconnection, the terminal shall set the
                                            device identities to:
                                            - source: terminal;
                                            - destination: UICC.
                                            in the case of connecting at the far end (an MO call), the terminal
                                            shall set the device identities to:
                                            - source: network;
                                            - destination: UICC.
            stCause                        �ͷ�ԭ��ֵ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucTi;
    VOS_UINT8                           ucLocalDiscFlg;
    VOS_UINT8                           aucReserved1[2];
    NAS_STK_CALL_DISC_CAUSE_STRU        stCause;
}NAS_STK_CALL_DISC_EVENT_INFO_STRU;

/*****************************************************************************
 �ṹ��    : NAS_STK_CALL_DISC_EVENT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : CALL����STKģ���CALL CONNECT EVENT��Ϣ�ṹ
             ucLocalDiscFlg      ָʾ�Ƿ񱾵��ͷ�
             stCallDiscEvent     CALL DISCONNECTED EVENT��Ϣ�ֶ�
             �ο�Э��10223 7.5.3 Call disconnected event
                     11.14 11.3  Call disconnected event
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                           ulMsgId;
    NAS_STK_CALL_DISC_EVENT_INFO_STRU    stCallDiscEventInfo;
} NAS_STK_CALL_DISC_EVENT_STRU;

/*****************************************************************************
 �ṹ��    : NAS_STK_LOC_STATUS_EVENT_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : CALL����STKģ���LOCATION STATUS EVENT��Ϣ��Ϣ�ֶεĽṹ
             ucCsServStatus           CS�����״̬
                    0           ��������
                    1           ���Ʒ���
                    2           ������������� ��ӦSTK�����Ʒ���
                    3           ��imsi   ��ӦSTK���޷���
                    4           �޷���
                    6           SLEEP    ��ӦSTK���޷���
             enRat                    ���뼼������      �ο�Э��10223 8.61 Access technology
                    0           TAF_MMA_RAT_GSM              '00' = GSM
                    1           TAF_MMA_RAT_WCDMA            '03' = UTRAN
                    2           TAF_MMA_RAT_LTE              '08' = E-UTRAN
             stPlmn                   UEפ����PLMN
             usLac                    UEפ����λ����
             ulCellId                 UEפ����С����32BITת������ֽ������aucCellId[3]��aucCellId[2]����enRat ��ΪTAF_PH_RAT_GSMʱ��Ч
                   BIT 0 - 7            8-15            16 - 23         24 - 31
                      aucCellId[1]    aucCellId[0]    aucCellId[3]    aucCellId[2]

             �ο�Э��
             10223 7.5.4.2 Structure of ENVELOPE (EVENT DOWNLOAD - Location status)
             11.14 11.4.2  Structure of ENVELOPE (EVENT DOWNLOAD - Location status)
*****************************************************************************/
typedef struct
{
    TAF_PLMN_ID_STRU                  stPlmn;
    TAF_MMA_RAT_TYPE_ENUM_UINT8       enRat;
    NAS_STK_SERVICE_STATUS_ENUM_UINT8 enServiceStatus;
    VOS_UINT16                        usLac;
    VOS_UINT32                        ulCellId;
}NAS_STK_LOC_STATUS_EVENT_INFO_STRU;

/*****************************************************************************
 �ṹ��    : NAS_STK_LOC_STATUS_EVENT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : LOCATION STATUS EVENT��Ϣ�ṹ
             stLocStatusEvent LOCATION STATUS EVENT��Ϣ��Ϣ�ֶεĽṹ
             �ο�Э�� 10223 7.5.4 Location status event
                      11.14 11.4  Location status event
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                           ulMsgId;
    NAS_STK_LOC_STATUS_EVENT_INFO_STRU   stLocStatusEvent;
} NAS_STK_LOC_STATUS_EVENT_STRU;

/*****************************************************************************
 �ṹ��    : NAS_STK_ACCTECH_CHG_EVENT_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Access Technology Change Event��Ϣ�ṹ
             stAccTechChgEvent Access Technology Change Event��Ϣ��Ϣ�ֶεĽṹ
             �ο�Э�� 10223 7.5.12 Access Technology Change Event
                      11.14 ���漰
            ucExistFlg      �Ƿ������Ч�Ľ��뼼��
            enRat           UEפ������Ľ��뼼��
*****************************************************************************/
typedef struct
{
    TAF_MMA_RAT_TYPE_ENUM_UINT8         enRat;
    VOS_UINT8                           aucRsv[3];
}NAS_STK_ACCTECH_CHG_EVENT_INFO_STRU;


/*****************************************************************************
 �ṹ��    : NAS_STK_SRCHMODE_CHG_EVENT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Access Technology Change Event��Ϣ�ṹ
             stNetSrchModeChgEvent Network Search Mode Change Event��Ϣ��Ϣ�ֶεĽṹ
             �ο�Э�� 10223 7.5.15 Network Search Mode Change Event
                      11.14 ���漰
             ucSrchMode            UE���õ�����ģʽ
                                   0   automatic selection mode
                                   1   manual selection mode
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucSrchMode; /* typedef TAF_UINT8 TAF_PH_PLMN_SEL_MODE;   */
}NAS_STK_SRCHMODE_CHG_EVENT_INFO_STRU;

/*****************************************************************************
 �ṹ��    : NAS_STK_SRCHMODE_CHG_EVENT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Access Technology Change Event��Ϣ�ṹ
             stNetSrchModeChgEvent Network Search Mode Change Event��Ϣ��Ϣ�ֶεĽṹ
             �ο�Э�� 10223 7.5.15 Network Search Mode Change Event
                      11.14 ���漰
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                           ulMsgId;
    NAS_STK_SRCHMODE_CHG_EVENT_INFO_STRU stSrchModeChgEvent;
} NAS_STK_SRCHMODE_CHG_EVENT_STRU;


typedef struct
{
    NAS_STK_SERVICE_STATUS_ENUM_UINT8    enCsServiceStatus;
    VOS_UINT8                            aucReserved1[3];
}NAS_STK_SERVICE_STATUS_INFO_STRU;


typedef struct
{
    TAF_PLMN_ID_STRU                    stPlmn;
    VOS_UINT16                          usLac;
    TAF_MMA_RAT_TYPE_ENUM_UINT8         enRat;
    NAS_STK_UTRAN_MODE_ENUM_UINT8       enUtranMode;
    VOS_UINT32                          ulCellId;
}NAS_STK_SYS_INFO_STRU;



typedef struct
{
    VOS_MSG_HEADER                                                              /* ��Ϣͷ */        /*_H2ASN_Skip*/
    VOS_UINT32                          ulMsgName;                              /* ��Ϣͷ */        /*_H2ASN_Skip*/
    NAS_STK_SYS_INFO_STRU               stSysInfo;
    NAS_STK_SERVICE_STATUS_INFO_STRU    stServiceStatus;
}NAS_STK_LOCATION_INFO_IND_STRU;

/*****************************************************************************
 �ṹ��    : STK_CALL_ADDR_STRU
 Э����  : �ο�Э��102.223 8.1
                     24008   10.5.4.7
 ASN.1���� : ucLen          ����aucAddr�е���Ч�ֽ�����
             ucAddrType     ��ַ����: TON and NPI��
             aucAddr        ���к���: Dialling number string��һ���ֽڰ���2������
 �ṹ˵��  : STK����CALL ����CS����е�����ṹ
*****************************************************************************/
#define STK_CALL_ADDR_MAX_LEN                               (40)
typedef struct
{
    VOS_UINT8                           ucAddrType;
    VOS_UINT8                           ucLen;
    VOS_UINT8                           aucAddr[STK_CALL_ADDR_MAX_LEN];
}STK_CALL_ADDR_STRU;

/*****************************************************************************
 �ṹ��    : STK_CALL_SUBADDR_STRU
 Э����  : �ο�Э��102.223 8.3
                     24008   10.5.4.8
 ASN.1���� : ucLen          ����aucSubAddr�е���Ч�ֽ�����
             aucSubAddr     �����ӵ�ַ: Subaddress
 �ṹ˵��  : STK����CALL ����CS����е�����ṹ
*****************************************************************************/
#define STK_CALL_SUBADDR_MAX_LEN                            (20)
typedef struct
{
    VOS_UINT8                           ucLen;
    VOS_UINT8                           aucReserved1[3];
    VOS_UINT8                           aucSubAddr[STK_CALL_SUBADDR_MAX_LEN];
}STK_CALL_SUBADDR_STRU;

/*****************************************************************************
 �ṹ��    : STK_CALL_BC_STRU
 Э����  : �ο�Э��102.223 8.4    Capability configuration parameters
                     24008   10.5.4.5
 ASN.1���� : ucLen          ����aucBc�е���Ч�ֽ�����
             aucBc          Capability configuration parameters
 �ṹ˵��  : STK����CALL ����CS����е�����ṹ
*****************************************************************************/
#define STK_CALL_BC_MAX_LEN                                  (14)
typedef struct
{
    VOS_UINT8                           ucLen;
    VOS_UINT8                           aucReserved1[1];
    VOS_UINT8                           aucBc[STK_CALL_BC_MAX_LEN];
}STK_CALL_BC_STRU;


/*****************************************************************************
 �ṹ��    : MN_APP_SS_REGISTER_REQ_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : STK����SS Register SSҵ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT16                          usClientId;
    VOS_UINT8                           opID;
    VOS_UINT8                           aucReserved1[1];
    TAF_SS_REGISTERSS_REQ_STRU          stTafSsRegisterSsReq;
} MN_APP_SS_REGISTER_REQ_STRU;


/*****************************************************************************
 �ṹ��    : MN_APP_SS_REGPWD_RSP_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : STK����SS PWD������Ӧ��Ϣ
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT16                          usClientId;
    VOS_UINT8                           opID;
    VOS_UINT8                           aucReserved1[1];
    TAF_SS_GETPWD_RSP_STRU              stTafSsRegPwdRsp;
} MN_APP_SS_REGPWD_RSP_STRU;

/*****************************************************************************
 �ṹ��    : MN_APP_SS_ERASE_REQ_MSG_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : STK����SS Erase SSҵ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT16                          usClientId;
    VOS_UINT8                           opID;
    VOS_UINT8                           aucReserved1[1];
    TAF_SS_ERASESS_REQ_STRU             stTafSsEraseSsReq;
} MN_APP_SS_ERASE_REQ_STRU;

/*****************************************************************************
 �ṹ��    : MN_APP_SS_ACTIVATE_REQ_MSG_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : STK����SS ACTIVATE SSҵ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT16                          usClientId;
    VOS_UINT8                           opID;
    VOS_UINT8                           aucReserved1[1];
    TAF_SS_ACTIVATESS_REQ_STRU          stTafSsActivateSsReq;
} MN_APP_SS_ACTIVATE_REQ_STRU;

/*****************************************************************************
 �ṹ��    : MN_APP_SS_DEACTIVATE_REQ_MSG_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : STK����SS DEACTIVATE SSҵ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT16                          usClientId;
    VOS_UINT8                           opID;
    VOS_UINT8                           aucReserved1[1];
    TAF_SS_DEACTIVATESS_REQ_STRU        stTafSsDeActivateSsReq;
} MN_APP_SS_DEACTIVATE_REQ_STRU;

/*****************************************************************************
 �ṹ��    : MN_APP_SS_INTERROGATE_REQ_MSG_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : STK����SS INTERROGATE SSҵ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT16                          usClientId;
    VOS_UINT8                           opID;
    VOS_UINT8                           aucReserved1[1];
    TAF_SS_INTERROGATESS_REQ_STRU       stTafSsInterrogateSsReq;
} MN_APP_SS_INTERROGATE_REQ_STRU;


/*****************************************************************************
 �ṹ��    : MN_APP_SS_REGPWD_REQ_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : STK����SS REG PWD SSҵ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT16                          usClientId;
    VOS_UINT8                           opID;
    VOS_UINT8                           aucReserved1[1];
    TAF_SS_REGPWD_REQ_STRU              stTafSsRegPwdReq;
} MN_APP_SS_REGPWD_REQ_STRU;

/*****************************************************************************
 �ṹ��    : MN_APP_SS_USSD_REQ_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : STK����SS USSDҵ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT16                          usClientId;
    VOS_UINT8                           opID;
    VOS_UINT8                           aucReserved1[1];
    TAF_SS_PROCESS_USS_REQ_STRU         stTafSsUssdReq;
} MN_APP_SS_USSD_REQ_STRU;


/*****************************************************************************
 �ṹ��    : MN_APP_SS_REGISTER_REQ_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : STK����SMSҵ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT16                          usClientId;
    VOS_UINT8                           opID;
    VOS_UINT8                           aucReserved1[1];
    MN_MSG_SEND_PARM_STRU               stMsgSendReq;
} MN_APP_MSG_SEND_REQ_STRU;

/*****************************************************************************
 �ṹ��    : MN_APP_CALL_START_DTMF_REQ_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : STK����CALL ��ʼDTMFҵ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT16                          usClientId;
    VOS_UINT8                           opID;
    VOS_UINT8                           callID;
    TAF_CALL_DTMF_PARAM_STRU            stTafStartDtmfPara;
} MN_APP_CALL_START_DTMF_REQ_STRU;


/*****************************************************************************
 �ṹ��    : MN_APP_CALL_STOP_DTMF_REQ_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : STK����CALL ֹͣDTMFҵ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT16                          usClientId;
    VOS_UINT8                           opID;
    VOS_UINT8                           callID;
    VOS_UINT8                           aucData[4];
} MN_APP_CALL_STOP_DTMF_REQ_STRU;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT16                          usClientId;
    VOS_UINT8                           opID;
    VOS_UINT8                           callID;
    STK_CALL_ADDR_STRU                  stCalledAddr;
    STK_CALL_SUBADDR_STRU               stSubAddr;
    STK_CALL_BC_STRU                    stBc;
    MN_CALL_TYPE_ENUM_U8                enCallType;
    VOS_UINT8                           aucReserve[3];
    MN_CALL_EMERGENCY_CAT_STRU          stEmergencyCat;                         /* Emergency Catory */
} MN_APP_CALL_CALLORIG_REQ_STRU;

/*****************************************************************************
 �ṹ��    : MN_APP_CALL_SUPS_REQ_STRU
 Э����  :
 ASN.1���� : clientId            OAM_CLIENT_ID_STK
             opId                Operation ID, ��ʶ���β�����TAF��͸����STKģ��
             stSupsPara
                enCallSupsCmd    MN_CALL_SUPS_CMD_REL_ALL_CALL   �ͷ����к��У�
                                 MN_CALL_SUPS_CMD_HOLD_ACT_ACPT_OTH HOLD����ACTIVE���У������ȴ��ĺ��У�
                                 MN_CALL_SUPS_CMD_REL_CALL_X �Ҷ�ָ�����У�
                callId           �Ҷ�ָ������ʱ��Ч�����������0
                stRedirectNum    enCallSupsCmdΪMN_CALL_SUPS_CMD_DEFLECT_CALLʱ��Ч
 �ṹ˵��  : STK����CALL ����CS�������ز���ҵ�������ṹ
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT16                          usClientId;
    VOS_UINT8                           opID;
    VOS_UINT8                           aucReserved1[1];
    MN_CALL_SUPS_PARAM_STRU             stSupsPara;
} MN_APP_CALL_SUPS_REQ_STRU;

/*****************************************************************************
 �ṹ��    : MN_APP_CMD_CNF_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : TAF����AT/STK��CMD CNF��Ϣ�ṹ
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    AT_CMD_CNF_EVENT                    stCmdCnfEvent;
} MN_APP_CMD_CNF_STRU;

/*****************************************************************************
 �ṹ��    : MN_APP_SS_EVENT_IND_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : TAF����AT/STK��SS�¼��ṹ
 enEventType�����ȷ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT32                          ulEventType;
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU  stSsEvent;
} MN_APP_SS_EVENT_IND_STRU;

/*****************************************************************************
 �ṹ��    : MN_APP_MSG_EVENT_IND_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : TAF����AT/STK��SMS�¼��ṹ
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    MN_MSG_EVENT_ENUM_U32               enEventType;
    MN_MSG_EVENT_INFO_STRU              stEvent;
} MN_APP_MSG_EVENT_IND_STRU;


/*****************************************************************************
 �ṹ��    : MN_APP_CS_CALLINFO_MSG_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : TAF����AT/STK��CS CALL�¼��ṹ
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    MN_CALL_EVENT_ENUM_U32              enEventType;
    MN_CALL_INFO_STRU                   stCsCallInfo;
} MN_APP_CS_CALLINFO_MSG_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_UINT8                           aucCurcRptCfg[NAS_STK_CURC_RPT_CFG_MAX_SIZE];
 }TAF_STK_CURC_RPT_CFG_INFO_MSG_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                                  ulMsgName;
    TAF_CS_SERVICE_ENUM_UINT32                  enCSState;
}TAF_STK_CS_SERVICE_IND;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                      ulMsgId;
    MN_CALL_EVENT_ENUM_U32          enEventType;
    VOS_UINT8                       aucEvent[4];
} NAS_CALL_STK_EVT_STRU;




typedef struct
{
    VOS_UINT32                      ulMcc;
    VOS_UINT16                      usImsi11_12;
    VOS_UINT16                      usSid;
    VOS_UINT16                      usNid;
    VOS_UINT16                      usBaseId;
    VOS_INT32                       lBaseLatitude;
    VOS_INT32                       lBaseLongtitude;
}MMA_STK_1X_SYS_INFO_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;
    MMA_STK_1X_SYS_INFO_STRU            stSysInfo;
    NAS_STK_SERVICE_STATUS_INFO_STRU    stServiceStatus;
}MMA_STK_SERVICE_INFO_IND_STRU;

/*****************************************************************************
 �ṹ��    : NAS_STK_PS_CALLINFO_EVENT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : TAF����STKģ���PS CALL INFO EVENT��Ϣ�ṹ,
             ������һ������ҵ��(����CDMA��LTE��Ĭ�ϳ���)��ʱ��ͷ��� TRUE
             ����������ҵ�񶼶Ͽ���ʱ���� FALSE
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;
    VOS_BOOL                            bPsCallExist;
} NAS_STK_PS_CALLINFO_EVENT_STRU;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/

/*****************************************************************************
  H2ASN������Ϣ�ṹ����
*****************************************************************************/
typedef struct
{
    NAS_STK_MSG_ID_ENUM_UINT32          enMsgId;            /* _H2ASN_MsgChoice_Export NAS_STK_MSG_ID_ENUM_UINT32 */
    VOS_UINT8                           aucMsg[4];
    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          NAS_STK_MSG_ID_ENUM_UINT32
    ****************************************************************************/
} NAS_STK_INTERFACE_MSG_DATA;
/* _H2ASN_Length UINT32 */

typedef struct
{
    VOS_MSG_HEADER
    NAS_STK_INTERFACE_MSG_DATA           stMsgData;
} NasStkInterface_MSG;


/*****************************************************************************
  10 ��������
*****************************************************************************/




#if (VOS_OS_VER == VOS_WIN32)
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

