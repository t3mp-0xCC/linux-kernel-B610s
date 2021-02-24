
#ifndef __DRV_FASTONOFF_H__
#define __DRV_FASTONOFF_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <product_config.h>

#if (FEATURE_ON == MBB_FAST_ON_OFF)
#include <linux/notifier.h>
#include <linux/ioctl.h>
/**************************************************************************
  �������ṹ���� 
**************************************************************************/
typedef enum
{

    FASTONOFF_MODE_CLR = 0,       /*��������״̬*/
    FASTONOFF_MODE_SET = 1,       /*�ٹػ�״̬  */
    FASTONOFF_MODE_MAX = 2,       /*�Ƿ�ֵ*/
}FASTONOFF_MODE;

#define FAST_ON_OFF_CMD 'F'
#define FASTONOFF_FAST_OFF_MODE  _IOWR(FAST_ON_OFF_CMD, 1, unsigned long)
#define FASTONOFF_FAST_ON_MODE   _IOWR(FAST_ON_OFF_CMD, 0, unsigned long)

/* �ٹػ�֪ͨ����ע�ᵽ�����ĺ��������ڽ��롢�˳��ٹػ�ʱ������ */
extern struct blocking_notifier_head g_fast_on_off_notifier_list;

/**************************************************************************
  �������� 
**************************************************************************/

/********************************************************
*������	  : fastOnOffGetFastOnOffMode
*�������� : ��ȡ���ٿ��ػ���־λ�������ж��Ƿ����ٹػ�״̬
*������� : ��
*������� : ��
*����ֵ	  : ִ�гɹ����ؼٹػ�״̬��־��ʧ�ܷ���-1
*�޸���ʷ :
*	       2013-5-15 �쳬 ��������
********************************************************/
extern FASTONOFF_MODE fastOnOffGetFastOnOffMode(void);

#endif /* #if (FEATURE_ON == MBB_FAST_ON_OFF) */

#ifdef __cplusplus
}
#endif

#endif

