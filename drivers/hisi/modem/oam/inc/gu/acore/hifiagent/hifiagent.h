#ifndef __HIFIAGENT_HEADER_H__
#define __HIFIAGENT_HEADER_H__

#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
typedef struct {
    VOS_UINT16      usMessageId;
    VOS_VOID (*pfAct)(MsgBlock *);
}RECORDER_VOICE_RT_MSG_PROSESS_ST;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 ulEventId;
    VOS_UINT32 aucData[1];      /*�ɱ䳤�ȵ�����*/
}HIFIAGENT_MSG_HOOK_ST;

typedef enum {
    DIR_HIFI_TO_DRV = 0,        /*��Ϣ����ΪHIFI �� Driver*/
    DIR_DRV_TO_HIFI = 1,        /*��Ϣ����ΪDriver �� HIFI*/
}DRV_HIFI_DIR_ENUM;
typedef VOS_UINT16  DRV_HIFI_DIR_ENUM_UINT16;

typedef struct {
    DRV_HIFI_DIR_ENUM_UINT16        usDirection; /*HIFI to DRV or DRV to HIFI*/
    VOS_UINT16      usHifiMessageId;
    VOS_UINT32      usDrvMessageId;
    VOS_UINT32      ulMessageStructSize;        /*��̬��Ϣ�Ľṹ���С*/
}RECORDER_DRV_MSG_TRANSLATE_ST;

#define OM_APP_PLAY_RECORDER_EVENT          (0x30)

typedef enum{
    PLAY_ALLOC_BUFFER_FAIL = 0,
    RECORD_ALLOC_BUFFER_FAIL,
    ALSA_DRV_MESSAGE_SIZE_ERROR,
    ALSA_DRV_CTRL_MSG_ID_IND,
    ALSA_DRV_CTRL_DATA_IND,
    ALSA_DRV_CTRL_RESULT_IND,
    OM_EVENT_ERROR_LINE_IND            = 0x8000,        /*error line ind mark event*/
}AP_PLAY_RECORDER_OM_EVENT_ENUM;
typedef VOS_UINT16  AP_PLAY_RECORDER_OM_EVENT_ENUM_UINT16;

extern VOS_VOID HifiAgent_VoiceSendOMEvent(
                AP_PLAY_RECORDER_OM_EVENT_ENUM_UINT16 enID,
                VOS_VOID *pucData,
                VOS_UINT32 ulLen);

extern VOS_UINT32 OM_AcpuTraceMsgHook(VOS_VOID *pMsg);

#endif
