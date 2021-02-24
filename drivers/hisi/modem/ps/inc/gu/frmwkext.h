/*******************************************************************************
  Copyright   : 2005-2007, Huawei Tech. Co., Ltd.
  File name   :
  Version     : V100R001
  Date        : 2005-04-19
  Description :
  History     :
  1. Date:2005-04-19
     Author: w29695
     Modification:Create
*******************************************************************************/

#if !defined(_FRMWK_EXT_H_)
#define _FRMWK_EXT_H_



#include "PsCommonDef.h"


typedef VOS_SEM      MUTEX_ID;    /* �������ı�ʶ���Ͷ��� */
typedef VOS_SEM      SEMAPHORE_ID;      /* �ź����ı�ʶ���Ͷ��� */
typedef VOS_UINT32   THREAD_ID;    /* �̵߳ı�ʶ���Ͷ��� */
typedef VOS_PID      MSGQUE_ID;                         /* ��Ϣ���еı�ʶ���Ͷ��� */


/* ��ʾ�϶�/�񶨵�ȫ�ֺ� */
#define    WUEPS_TRUE               0
#define    WUEPS_FALSE              1

#define    WUEPS_OK                 0
#define    WUEPS_NG                 1

/* ��ʾ��Ч��ȫ�ֺ� */
#define    WUEPS_INVALID            (-1)

/* ȡ���������ֵ/��Сֵ��ȫ�ֺ� */
#define    WUEPS_MAX(a,b)           (((a)>(b)) ? (a) : (b))
#define    WUEPS_MIN(a,b)           (((a)<=(b))? (a) : (b))

/* ASSERT���� */
#define    WUEPS_ASSERT( exp )      if( (exp) == 0 ){VOS_ASSERT(exp);}

#ifndef PS_RAND_MAX
#define PS_RAND_MAX 0x7fff
#endif

#define WUEPS_MUTEX_WAIT_TIME          20

#if(VOS_WIN32 == VOS_OS_VER)
#define WUEPS_SEMA_WAIT_TIME           30
#endif

/* WuepsLock����ֵ���� */
#define WUEPS_LOCK_SUCCESS  (VOS_INT32)0                     /* �����ɹ� */
#define WUEPS_LOCK_FAILURE  (VOS_INT32)((VOS_INT)-1)             /* ����ʧ�� */

/* WuepsUnlock����ֵ���� */
#define WUEPS_UNLOCK_SUCCESS    (VOS_INT32)0                 /* �����ɹ� */
#define WUEPS_UNLOCK_FAILURE    (VOS_INT32)((VOS_INT)-1)         /* ����ʧ�� */

/* WuepsMutexCreate����ֵ���� */
#define WUEPS_MUTEX_CREATE_SUCCESS  (VOS_INT32)0             /* �����ɹ� */
#define WUEPS_MUTEX_CREATE_FAILURE  ((VOS_INT32)-1)          /* ����ʧ�� */

/* WuepsMutexDestroy����ֵ���� */
#define WUEPS_MUTEX_DESTROY_SUCCESS     (VOS_INT32)0         /* ɾ���ɹ� */
#define WUEPS_MUTEX_DESTROY_FAILURE     (VOS_INT32)((VOS_INT)-1) /* ɾ��ʧ�� */

/* WuepsSemaphoreLock����ֵ���� */
#define WUEPS_SEMAPHORE_LOCK_SUCCESS  (VOS_INT32)0               /* �����ɹ� */
#define WUEPS_SEMAPHORE_LOCK_FAILURE  ((VOS_INT32)-1)            /* ����ʧ�� */

/* WuepsSemaphoreUnlock����ֵ���� */
#define WUEPS_SEMAPHORE_UNLOCK_SUCCESS    (VOS_INT32)0           /* �����ɹ� */
#define WUEPS_SEMAPHORE_UNLOCK_FAILURE    (VOS_INT32)((int)-1)   /* ����ʧ�� */

/* WuepsSemaphoreCreate����ֵ���� */
#define WUEPS_SEMAPHORE_CREATE_SUCCESS  (VOS_INT32)0             /* �����ɹ� */
#define WUEPS_SEMAPHORE_CREATE_FAILURE  (VOS_INT32)(-1)            /* ����ʧ�� */

/* WuepsSemaphoreDestroy����ֵ���� */
#define WUEPS_SEMAPHORE_DESTROY_SUCCESS     (VOS_INT32)0         /* ���ٳɹ� */
#define WUEPS_SEMAPHORE_DESTROY_FAILURE     (VOS_INT32)((int)-1) /* ����ʧ�� */

/* ulWaitOptionֵ���� */
#define WUEPS_SEMAPHORE_WAIT          ((VOS_INT32)-1)    /* �ȴ�     */
#define WUEPS_SEMAPHORE_NO_WAIT       (VOS_INT32)0      /* ���ȴ�   */

#define WUEPS_TIMER_TICK  10

extern VOS_INT32 WuepsLock(MUTEX_ID *pMutexId);
extern VOS_INT32 WuepsUnlock(MUTEX_ID *pMutexId);
extern VOS_INT32 WuepsSemaphoreCreate(SEMAPHORE_ID *pSemaphoreId,
                          const VOS_CHAR   *pcName,VOS_UINT32 ulInitCount);
extern VOS_INT32 WuepsSemaphoreDestroy(SEMAPHORE_ID *pSemaphoreId);
extern VOS_INT32 WuepsSemaphoreLock(SEMAPHORE_ID *pSemaphoreId,VOS_UINT32 ulWaitOption);
extern VOS_INT32 WuepsSemaphoreUnlock(SEMAPHORE_ID *pSemaphoreId);
extern VOS_INT32 WuepsMutexCreate(MUTEX_ID *pMutexId);
extern VOS_INT32 WuepsMutexDestroy(MUTEX_ID *pMutexId);



#define WUEPS_MAX_PID_NUM    40

/***********************************************
 *
 * FrameWork�����ķ���ֵ�Ķ���
 *
 **********************************************/
#define WUEPS_FRAMEWORK_SUCCESS (VOS_INT32)0         /* FrameWork�����ĳɹ���־  */
#define WUEPS_FRAMEWORK_FAILURE ((VOS_INT32)-1)      /* FrameWork������ʧ�ܱ�־  */


/***********************************************
 *
 * ȫ�ֺ������ⲿ����
 *
 **********************************************/

#endif  /* !defined(_FRMWK_EXT_H_) */

/***************************** End Of File ***********************************/
