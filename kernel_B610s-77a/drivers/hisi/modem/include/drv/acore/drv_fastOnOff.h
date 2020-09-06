
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
  变量、结构定义 
**************************************************************************/
typedef enum
{

    FASTONOFF_MODE_CLR = 0,       /*正常开机状态*/
    FASTONOFF_MODE_SET = 1,       /*假关机状态  */
    FASTONOFF_MODE_MAX = 2,       /*非法值*/
}FASTONOFF_MODE;

#define FAST_ON_OFF_CMD 'F'
#define FASTONOFF_FAST_OFF_MODE  _IOWR(FAST_ON_OFF_CMD, 1, unsigned long)
#define FASTONOFF_FAST_ON_MODE   _IOWR(FAST_ON_OFF_CMD, 0, unsigned long)

/* 假关机通知链，注册到此链的函数将会在进入、退出假关机时被调用 */
extern struct blocking_notifier_head g_fast_on_off_notifier_list;

/**************************************************************************
  函数定义 
**************************************************************************/

/********************************************************
*函数名	  : fastOnOffGetFastOnOffMode
*函数功能 : 获取快速开关机标志位，用于判断是否进入假关机状态
*输入参数 : 无
*输出参数 : 无
*返回值	  : 执行成功返回假关机状态标志，失败返回-1
*修改历史 :
*	       2013-5-15 徐超 初版作成
********************************************************/
extern FASTONOFF_MODE fastOnOffGetFastOnOffMode(void);

#endif /* #if (FEATURE_ON == MBB_FAST_ON_OFF) */

#ifdef __cplusplus
}
#endif

#endif

