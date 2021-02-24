/************************************************************************
  Copyright   : 2005-2007, Huawei Tech. Co., Ltd.
  File name   : Nasrrcinterface.h
  Author      : ---
  Version     : V200R001
  Date        : 2005-08-17
  Description : ��ͷ�ļ������� AS �ṩ�� NAS �ӿ����ݽṹ�Լ�����
  History     :
  1. Date:2005-08-17
     Author: ---
     Modification:Create
************************************************************************/

#ifndef _WUEPS_CONFIG_H_
#define _WUEPS_CONFIG_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/* �Ѵ�ӡͨ��OM�����PSTAS�Ŀ��ƿ��� */
#if 1
#define PS_OM_DUMP_SWITCH 1
#endif

/******************************��Ʒ�������������Կ���******************************/
#define  WUEPS_RNC_SWICTH_ON        0
#define  WUEPS_CMU200_SWICTH_ON     1
#define  WUEPS_CRTUW_SWICTH_ON      2
/*20070324 add   begin */
// #define  WUEPS_RRM_SWICTH_ON        3
/*20070324 add   end*/

/*Ĭ�ϱ��뿪��ΪRNC*/
#define  WUEPS_PRODUCT_SWITCH       WUEPS_RNC_SWICTH_ON

/*2006-08-21 added   begin*/
/* �ȴ���־���� */
#define WUEPS_MSG_WAIT      ((VOS_UINT32)(-1))       /* �ȴ�     */
#define WUEPS_MSG_NO_WAIT   (VOS_UINT32)0            /* ���ȴ�   */
/*2006-08-21 added   end*/

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif

