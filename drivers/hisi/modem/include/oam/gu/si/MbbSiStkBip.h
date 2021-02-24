




#ifndef __MBB_SI_STK_BIP_H__
#define __MBB_SI_STK_BIP_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "TafTypeDef.h"
#include "sitypedef.h"
#include "vos.h"
#include "Taf_MmiStrParse.h"

#ifdef  __cplusplus
#if  __cplusplus
extern "C"{
#endif
#endif

#if (FEATURE_ON == MBB_WPG_COMMON)

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*******************************************************************************
  3 枚举定义
*******************************************************************************/
#if (FEATURE_ON == MBB_FEATURE_BIP)

#define     BIP_TEST_SUPPORT_MARK               0x10
#define     BIP_TEST_SUPPORT(stBipFeatureNV)    (0 != (stBipFeatureNV.ucReserved[0] & BIP_TEST_SUPPORT_MARK))

typedef enum {

  GSTK_NO_SPECIFIC_CAUSE_CAN_BE_GIVEN                             = 0x00, /* No specific cause */
    /**< No specific cause. */
  GSTK_SCREEN_IS_BUSY                                             = 0x01,   /* Screen busy */
    /**< Screen is busy. */
  GSTK_ACTION_NOT_ALLOWED                                         = 0x01,   /* Action not allowed */
    /**< Action is not allowed. */
  GSTK_BEARER_UNAVAILABLE                                         = 0x01,   /* Bearer unavailable */
    /**< Bearer is unavailable. */
  GSTK_CH_UNAVAILABLE                                             = 0x01,   /* Channel unavailable */
    /**< Channel is unavailable. */

  GSTK_REQUEST_CHANGED                                            = 0x02,   /* Request been changed */
    /**< Request has been changed. */
  GSTK_ME_CURRENTLY_BUSY_ON_CALL                                  = 0x02,   /* busy on a call */
    /**< Mobile equipment is currently busy on a call. */
  GSTK_BROWSER_UNAVAILABLE                                        = 0x02,   /* Browser unavailable */
    /**< Browser is unavailable. */
  GSTK_CH_CLOSED                                                  = 0x02,   /* Channel closed */
    /**< Channel is closed. */

  GSTK_ME_UNABLE_TO_READ_PROVISIONING_DATA                        = 0x03,   /* unable read data */
    /**< Mobile equipment is unable to read the provisioning data. */
  GSTK_ME_CURRENTLY_BUSY_ON_SS_TRANSACTION                        = 0x03,   /* busy */
    /**< Mobile equipment is currently busy with a supplementary services
         transaction. */
  GSTK_CH_ID_INVALID                                              = 0x03,   /* Channel ID invalid */
    /**< Channel ID is invalid. */

  GSTK_NO_SERVICE_AVAILABLE                                       = 0x04,   /* No service */
    /**< No service is available. */
  GSTK_BUFFER_SIZE_UNAVAILABLE                                    = 0x04,   /* Buffer size unavailable */
    /**< Buffer size is unavailable. */

  GSTK_ACCESS_CONTROL_CLASS_BAR                                   = 0x05,   /* barred */
    /**< Access Control Class is barred on the serving network. */
  GSTK_SECURITY_ERROR_UNSUCCESSFUL_AUTH                           = 0x05,   /* Security error */
    /**< Security error occurred. The authorization attempt was unsuccessful. */

  GSTK_RADIO_RESOURCE_NOT_GRANTED                                 = 0x06,   /* not granted */
    /**< Radio resource was not granted. */
  GSTK_SIM_ME_INTERFACE_TRANSPORT_LV_UNAVAILABLE                  = 0x06,   /* transport level is unavailable */
    /**< SIM/ME interface transport level is unavailable. */

  GSTK_NOT_IN_SPEECH_CALL                                         = 0x07,   /* not in speech call */
    /**< Not in a speech call. */

  GSTK_ME_CURRENTLY_BUSY_ON_USSD_TRANSACTION                      = 0x08,   /* busy */
    /**< Mobile equipment is currently busy on an unstructured supplementary
         service data transaction. */

  GSTK_ME_CURRENTLY_BUSY_ON_SEND_DTMF_COMMAND                     = 0x09,   /* busy now */
    /**< Mobile equipment is currently busy sending the Dual-tone
         Multifrequency command. */

  GSTK_NO_USIM_ACTIVE                                             = 0x0A,   /* module not active */
    /**< No universal subscriber identity module is active. */

  GSTK_ADDITIONAL_INFO_CODE_MAX                                   = 0xFFFF  /* max val */
    /**< Maximum value of the addition information code. */
}bip_additional_info_code_enum_type;
/*****************************************************************************
 结构名    : MN_APP_STK_AT_CNF_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : STK给AT上报的消息结构
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                       ulMsgId;
    SI_UINT32                           Datalen;
    SI_UINT8                             Data[8];    /* 变长的data数据区 */
} MN_APP_STK_AT_EVENT_STRU;
#endif/*FEATURE_ON == MBB_FEATURE_BIP*/


/*****************************************************************************
  4 回调函数数据结构声明
*****************************************************************************/

/*****************************************************************************
  5 函数声明
*****************************************************************************/
#if (FEATURE_ON == MBB_FEATURE_BIP)
extern VOS_VOID  BIP_Initialize(VOS_VOID);
extern void SI_STK_BipEventProc(VOS_VOID *pSTKReqMsg);
extern VOS_UINT32 SI_BIP_CmdTimeoutProc(SI_STK_DATA_INFO_STRU * pstUsatCmdDecode);
/*Bip 5个主动命令处理声明*/
extern VOS_UINT32 SI_STK_OpenChannelProc(SI_STK_DATA_INFO_STRU *pCmdData);
extern VOS_UINT32 SI_STK_CloseChannelProc(SI_STK_DATA_INFO_STRU *pCmdData);
extern VOS_UINT32 SI_STK_ReceivedDataProc(SI_STK_DATA_INFO_STRU *pCmdData);
extern VOS_UINT32 SI_STK_SendDataProc(SI_STK_DATA_INFO_STRU *pCmdData);
extern VOS_UINT32 SI_STK_GetChannelStatusProc(SI_STK_DATA_INFO_STRU *pCmdData);
#endif /*#if (FEATURE_ON == MBB_FEATURE_BIP)*/


#endif/*FEATURE_ON == MBB_WPG_COMMON*/

#if ((TAF_OS_VER == TAF_WIN32) || (TAF_OS_VER == TAF_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of SiAppSim.h */








