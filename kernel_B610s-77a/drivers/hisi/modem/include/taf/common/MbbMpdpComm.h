/******************************************************************************
 */
/*
 */
/*                  版权所有 (C), 2001-2011, 华为技术有限公司
 */
/*
 */
/******************************************************************************
 */
/*  文 件 名   : at_lte_eventreport_mbb.h
 */
/*  版 本 号   : V1.0
 */

/*  生成日期   : 2014-06-09
 */
/*  功能描述   : 产品线自研及修改at命令文件
 */
/*  函数列表   :
 */
/*  修改历史   :
 */
/*  1.日    期 : 2011-03-10
 */

/*    修改内容 : 创建文件
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
  1 其他头文件包含
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
    NDIS_RM_NET_ID_0,                        /*网卡1*/
    NDIS_RM_NET_ID_1,                        /*网卡2*/
    NDIS_RM_NET_ID_2,                        /*网卡3*/
    NDIS_RM_NET_ID_3,                        /*网卡4*/
    NDIS_RM_NET_ID_4,                        /*网卡5*/
    NDIS_RM_NET_ID_5,                        /*网卡6*/
    NDIS_RM_NET_ID_6,                        /*网卡7*/
    NDIS_RM_NET_ID_7,                        /*网卡8*/
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
 函 数 名  : AT_DeRegNdisFCPoint
 功能描述  : 去注册NDIS端口流控点。
 输入参数  : VOS_UINT8                           ucRabId
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_DeRegNdisFCPointEx(VOS_UINT8 ucRabId,
    VOS_UINT16                enModemId,
    VOS_UINT8                   enFcId);

/*通过信道ID获取到RmnetId*/
VOS_UINT32 AT_PS_GetNdisRmNetIdFromChDataValue(VOS_UINT8 ucIndex,
    VOS_UINT32      enDataChannelId,
    VOS_UINT8          *penNdisRmNetId);

/*拨号成功之后，激活网卡的处理*/
VOS_VOID  AT_PS_SndNdisPdpActInd( VOS_UINT8 ucCid,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent,
    VOS_UINT8             enPdpType);
/*拨号成功之后，去激活网卡的处理*/
VOS_VOID AT_PS_SndNdisPdpDeactInd(VOS_UINT8 ucCid,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent,
    VOS_UINT8             enPdpType);
/*拨号成功之后，注册流控的处理*/
VOS_VOID AT_PS_RegNdisFCPoint(VOS_UINT8 ucCid, TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU  *pstEvent);
/*拨号成功之后，去注册流控的处理*/
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


