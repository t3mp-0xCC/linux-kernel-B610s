#ifndef __MBB_ATCMDFILTER_H__
#define __MBB_ATCMDFILTER_H__

/*****************************************************************************
  1 ͷ�ļ�
*****************************************************************************/
#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif
#if (FEATURE_ON == MBB_WPG_COMMON)

/*****************************************************************************
  2 OTHERS����
*****************************************************************************/
extern VOS_UINT32 At_GetTheCustomVersion(VOS_VOID);

#if (FEATURE_ON == MBB_WPG_AT_CMD_FILTER)
/* һ������������ */
extern const VOS_CHAR * MbbAtCmdFilterGetTable(VOS_VOID);
extern VOS_UINT32 MbbAtCmdFilterGetTableCount(VOS_VOID);
extern VOS_UINT32 At_CheckUnlockCmdName(VOS_CHAR *pszCmdName, VOS_UINT8 ucIndex);
#endif /* FEATURE_ON == MBB_WPG_AT_CMD_FILTER */

#if (FEATURE_ON == MBB_WPG_AT_CMD_ALLOW)
/* �������������� */
extern const VOS_CHAR * MbbAtCmdAllowGetTable(VOS_VOID);
extern VOS_UINT32 MbbAtCmdAllowGetTableCount(VOS_VOID);
extern VOS_UINT32 At_CheckAllowCmdName(VOS_CHAR* pszCmdName, VOS_UINT8 ucIndex);
#endif /* FEATURE_ON == MBB_WPG_AT_CMD_ALLOW */
#endif /* FEATURE_ON == MBB_WPG_COMMON */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __MBB_ATCMDFILTER_H__ */
