

#ifndef __MBB_DRV_AT_EXTEND_PRIVATE_CMD_H__
#define __MBB_DRV_AT_EXTEND_PRIVATE_CMD_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "vos.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if(FEATURE_ON == MBB_COMMON)


/*****************************************************************************
  特性公共 函数申明
*****************************************************************************/
/* 注册函数，对外接口 */
VOS_UINT32 At_RegisterMbbDrvPrivateCmdTable(VOS_VOID);

#if (FEATURE_ON == MBB_FEATURE_BODYSAR)
VOS_UINT32 Mbb_AT_QryBodySarOnPara(VOS_UINT8 ucIndex);
#endif

#if  (FEATURE_ON == MBB_MODULE_THERMAL_PROTECT)
VOS_UINT32 Mbb_AT_QryChipTempPara(VOS_UINT8 ucIndex);
#endif

#if  (FEATURE_ON == MBB_FEATURE_OFFLINE_LOG)
VOS_UINT32 Mbb_AT_QryRsfrLogFs(VOS_CHAR* cSubName, VOS_CHAR* cName);
#endif

#endif/*#if(FEATURE_ON == MBB_COMMON)*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


#endif /* __MBB_DRV_AT_EXTEND_PRIVATE_CMD_H__ */
