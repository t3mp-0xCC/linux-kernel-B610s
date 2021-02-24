/******************************************************************************
 */
/*
 */
/*                  ��Ȩ���� (C), 2001-2011, ��Ϊ�������޹�˾
 */
/*
 */
/******************************************************************************
 */
/*  �� �� ��   : at_lte_eventreport_mbb.h
 */
/*  �� �� ��   : V1.0
 */

/*  ��������   : 2014-06-09
 */
/*  ��������   : ��Ʒ�����м��޸�at�����ļ�
 */
/*  �����б�   :
 */
/*  �޸���ʷ   :
 */
/*  1.��    �� : 2011-03-10
 */

/*    �޸����� : �����ļ�
 */
/*
 */
/******************************************************************************
 */
/*************************************************************************************
*************************************************************************************/



#ifndef _MBB_MPDP_COMM_H__
#define _MBB_MPDP_COMM_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "TafTypeDef.h"
#include "TafAppMma.h"
#include "TafApsApi.h"
#include "mbb_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if(FEATURE_ON == MBB_WPG_DIAL)
enum AT_CH_DATA_CHANNEL_ENUM
{
    AT_CH_DATA_CHANNEL_ID_1             = 1,
    AT_CH_DATA_CHANNEL_ID_2,
    AT_CH_DATA_CHANNEL_ID_3,
    AT_CH_DATA_CHANNEL_ID_4,
    AT_CH_DATA_CHANNEL_ID_5,
    AT_CH_DATA_CHANNEL_ID_6,
    AT_CH_DATA_CHANNEL_ID_7,
    AT_CH_DATA_CHANNEL_ID_8,
    AT_CH_DATA_CHANNEL_BUTT
};
typedef VOS_UINT32 AT_CH_DATA_CHANNEL_ENUM_UINT32;


enum NDIS_RM_NET_ID_ENUM
{
    NDIS_RM_NET_ID_0,                        /*����1*/
    NDIS_RM_NET_ID_1,                        /*����2*/
    NDIS_RM_NET_ID_2,                        /*����3*/
    NDIS_RM_NET_ID_3,                        /*����4*/
    NDIS_RM_NET_ID_4,                        /*����5*/
    NDIS_RM_NET_ID_5,                        /*����6*/
    NDIS_RM_NET_ID_6,                        /*����7*/
    NDIS_RM_NET_ID_7,                        /*����8*/
    NDIS_RM_NET_ID_BUTT
};
typedef VOS_UINT8 NDIS_RM_NET_ID_ENUM_UINT8;



typedef struct
{
    AT_CH_DATA_CHANNEL_ENUM_UINT32      enChdataValue;
    NDIS_RM_NET_ID_ENUM_UINT8           enNdisRmNetId;
    VOS_UINT8                           aucReserved[3];
}AT_CHDATA_NDIS_RMNET_ID_STRU;

/*****************************************************************************
 �� �� ��  : AT_DeRegNdisFCPoint
 ��������  : ȥע��NDIS�˿����ص㡣
 �������  : VOS_UINT8                           ucRabId
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 AT_DeRegNdisFCPointEx(VOS_UINT8 ucRabId,
    VOS_UINT16                enModemId,
    VOS_UINT8                   enFcId);

/*ͨ���ŵ�ID��ȡ��RmnetId*/
VOS_UINT32 AT_PS_GetNdisRmNetIdFromChDataValue(VOS_UINT8 ucIndex,
    VOS_UINT32      enDataChannelId,
    VOS_UINT8          *penNdisRmNetId);

/*���ųɹ�֮�󣬼��������Ĵ���*/
VOS_VOID  AT_PS_SndNdisPdpActInd( VOS_UINT8 ucCid,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent,
    VOS_UINT8             enPdpType);
/*���ųɹ�֮��ȥ���������Ĵ���*/
VOS_VOID AT_PS_SndNdisPdpDeactInd(VOS_UINT8 ucCid,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent,
    VOS_UINT8             enPdpType);
/*���ųɹ�֮��ע�����صĴ���*/
VOS_VOID AT_PS_RegNdisFCPoint(VOS_UINT8 ucCid, TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent);
/*���ųɹ�֮��ȥע�����صĴ���*/
VOS_VOID AT_PS_DeRegNdisFCPoint(VOS_UINT8 ucCid, TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent);

extern VOS_UINT8 AT_GetDefaultFcID(VOS_UINT8 ucUserType, VOS_UINT32 ulRmNetId);
extern VOS_VOID AT_PS_UpdateUserDialDnsInfo(VOS_UINT8 ucIndex, VOS_VOID* stUsrDialParam);
extern VOS_INT AT_UsbCtrlBrkReqCBMpdp(VOS_VOID);
extern VOS_VOID AT_OpenUsbNdisMpdp(VOS_VOID* stParam);
extern VOS_VOID AT_CloseUsbNdisMpdp(VOS_VOID);
extern VOS_UINT8 AT_GetUsbNetNum(VOS_VOID);
extern VOS_VOID AT_IncreaseNumWhenAct(VOS_VOID);
extern VOS_VOID AT_DecreaseNumWhenDeact(VOS_VOID);
extern VOS_UINT8 AT_PS_FindDialCid(VOS_UINT16 ucIndex, const VOS_UINT8 ucCid);
extern VOS_UINT32 AT_PS_ProcDialCmdMpdp(VOS_UINT32* ulResult, VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_SendNdisIPv4PdnInfoCfgReqEx(TAF_UINT16 usClientId,
    VOS_VOID *pstIPv4DhcpParam, VOS_UINT32 ulHandle);
#endif/*FEATURE_ON == MBB_WPG_DIAL*/


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif


