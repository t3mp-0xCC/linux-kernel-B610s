

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"

#ifndef __COMINTERFACE_H__
#define __COMINTERFACE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

enum RRMM_RAT_TYPE_ENUM
{
    RRMM_RAT_TYPE_GSM = 0,                           /* GSM���뼼�� */
    RRMM_RAT_TYPE_WCDMA,                             /* WCDMA���뼼�� */
    RRMM_RAT_TYPE_LTE,                               /* LTE���뼼�� */
    /*Modify   for CL multimode 2014-01-09 begin*/
    RRMM_RAT_TYPE_CDMA1X,                            /* 1x���뼼��*/
    RRMM_RAT_TYPE_HRPD,                              /* HRPD���뼼��*/
    /*Modify   for CL multimode 2014-01-09 end*/
    RRMM_RAT_TYPE_BUTT
};
typedef VOS_UINT8 RRMM_RAT_TYPE_ENUM_UINT8;

enum RRMM_RAT_PRIO_ENUM
{
    RRMM_RAT_PRIO_NULL                = 0,               /* �ý��뼼��������    */
    RRMM_RAT_PRIO_LOW                 = 1,               /* ���ȼ��ϵ�   */
    RRMM_RAT_PRIO_MIDDLE              = 2,               /* ���ȼ��м�   */
    RRMM_RAT_PRIO_HIGH                = 3,               /* ���ȼ��ϸ�   */
    RRMM_RAT_PRIO_BUTT
};
typedef VOS_UINT8  RRMM_RAT_PRIO_ENUM_UINT8;


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
/* 7.1 GAS��WAS������Ϣ */

typedef struct
{
    VOS_UINT32                          BandGsm450      :1;
    VOS_UINT32                          BandGsm480      :1;
    VOS_UINT32                          BandGsm850      :1;
    VOS_UINT32                          BandGsmP900     :1;
    VOS_UINT32                          BandGsmR900     :1;
    VOS_UINT32                          BandGsmE900     :1;
    VOS_UINT32                          BandGsm1800     :1;
    VOS_UINT32                          BandGsm1900     :1;
    VOS_UINT32                          BandGsm700      :1;
    VOS_UINT32                          BandSpare23     :23;
}GSM_BAND_STRU;

typedef struct
{
    VOS_UINT32                          BandWCDMA_I_2100      :1;
    VOS_UINT32                          BandWCDMA_II_1900     :1;
    VOS_UINT32                          BandWCDMA_III_1800    :1;
    VOS_UINT32                          BandWCDMA_IV_1700     :1;
    VOS_UINT32                          BandWCDMA_V_850       :1;   
    VOS_UINT32                          BandWCDMA_VI_800      :1;
    VOS_UINT32                          BandWCDMA_VII_2600    :1;
    VOS_UINT32                          BandWCDMA_VIII_900    :1;
    VOS_UINT32                          BandWCDMA_IX_J1700    :1;
    VOS_UINT32                          BandSpareBand10       :1;
    VOS_UINT32                          BandWCDMA_XI_1500     :1;
    VOS_UINT32                          BandSpare7            :7;
    VOS_UINT32                          BandWCDMA_XIX_850     :1;
    VOS_UINT32                          BandSpare13           :13;
}WCDMA_BAND_STRU;


typedef union
{
    VOS_UINT32                          ulBand;
    
    GSM_BAND_STRU                       stBitBand;
}GSM_BAND_SET_UN;

typedef union
{
    VOS_UINT32                          ulBand;
    
    WCDMA_BAND_STRU                     stBitBand;
}WCDMA_BAND_SET_UN;

/*******************************************************************************
 �ṹ��    : LTE_BAND_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : LTE��Ƶ�ζ���,aulLteBand[0]�����32,aulLteBand[1]�����32��Band,
            ���� �û�����Band1,Band3,��aulLteBand[0]=0x05,aulLteBand[1]=0x00
*******************************************************************************/
typedef struct
{
    VOS_UINT32                          aulLteBand[2];
}LTE_BAND_STRU;



typedef struct
{
    VOS_UINT32                          ulWcdmaBand;
}NAS_RRC_WCDMA_BAND_STRU;


typedef struct
{
    VOS_UINT32                          ulGsmBand;
}NAS_RRC_GSM_BAND_STRU;


/* NetScan Add Begin */
/*******************************************************************************
 �ṹ��    : NAS_RRC_TDSCDMA_BAND_STRU
 �ṹ˵��  : TD-SCDMAƵ�ζ���
             aulTdsBand[0]:
                           TDS_COMM_MTC_FREQ_BAND_A         = 0x01,     Ƶ�㷶Χ: 9504~9596  10054~10121
                           TDS_COMM_MTC_FREQ_BAND_E         = 0x10,     Ƶ�㷶Χ: 11504~11996
                           TDS_COMM_MTC_FREQ_BAND_F         = 0x20,     Ƶ�㷶Χ: 9404~9596
                           �μ�TDS_COMM_MTC_FREQ_BAND_LIST_ENUM_UINT8����
             aulTdsBand[1]:��ʵ�ʺ��壬��LTE�ṹ����
             ���� �û�����FƵ�κ�EƵ��,��aulTdsBand[0]=0x06,aulTdsBand[1]=0x00
*******************************************************************************/
typedef struct
{
    VOS_UINT32                          aulTdsBand[2];
}NAS_RRC_TDSCDMA_BAND_STRU;
/* NetScan Add End */

/*******************************************************************************
 * Name        : CDMA_BAND_STRU
 *
 * Description : CDMA��Ƶ��
 *******************************************************************************/
typedef struct
{
    VOS_UINT32                          ulCdmaBand;
}CDMA_BAND_STRU;


typedef struct
{
    LTE_BAND_STRU                       stLteBand;

    NAS_RRC_WCDMA_BAND_STRU             stWcdmaBand;
    NAS_RRC_GSM_BAND_STRU               stGsmBand;
    /* NetScan Add Begin */
    NAS_RRC_TDSCDMA_BAND_STRU           stTdsBand;
    /* NetScan Add Edn */
    CDMA_BAND_STRU                      stCdmaBand; /* CDMAƵ�ν���MMC֪ͨLTEʱ��Ч��GU�಻��ע */
}NAS_RRC_USER_SET_FREQ_BAND_STRU;


/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


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

#endif /* end of ComInterface.h */
