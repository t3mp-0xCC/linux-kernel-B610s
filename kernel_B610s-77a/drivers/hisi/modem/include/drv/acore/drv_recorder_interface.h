#ifndef __ACORE_RECORD_PLAY_DRV_INTERFACE_H__
#define __ACORE_RECORD_PLAY_DRV_INTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum drv_hifi_cmd_type_e
{
    ID_DRV_HIFI_CMD_NOT_VALID               = 0, /*无效命令*/
    ID_DRV_START_RECORD_REQ                 = 1, /*请求hifi，开始录音*/
    ID_DRV_STOP_RECORD_REQ                  = 2, /*请求hifi，结束录音*/
    ID_DRV_START_PLAY_REQ                   = 3, /*请求hifi，开始远程播放*/
    ID_DRV_STOP_PLAY_REQ                    = 4, /*请求hifi，结束远程播放*/
    ID_DRV_SET_BUFFER_REQ                   = 5, /*通知hifi播放、录音的数据存放地址*/
    ID_DRV_HIFI_CMD_MAX                     = 9, /*max msg id*/
}ALSA_HIFI_VOICE_MSG_ID_ENUM;
typedef unsigned int ALSA_HIFI_VOICE_MSG_ID_ENUM_UINT32;

typedef enum hifi_drv_cmd_type_e
{
    ID_HIFI_DRV_NOT_VALID                      = 0, /*无效命令*/
    ID_DRV_RECORD_START_CNF                    = 1, /*通知DRV, hifi已经准备好数据录音*/
    ID_DRV_START_PLAY_CNF                      = 2, /*通知DRV, hifi已经准备好传送数据*/
    ID_HIFI_DRV_BUFFER_DATA_READY              = 3,  /*通知DRV,hifi已经在缓冲中填充好数据*/
    ID_DRV_HIFI_RECORD_ERR_NOTIFY              = 4, /*通知DRV, hifi发生临时错误*/
    ID_HIFI_DRV_CALL_END_NOTIFY                = 5, /*通知DRV, 通话结束*/
    ID_DRV_HIFI_PLAY_ERR_NOTIFY                = 6, /*通知DRV, hifi发生临时错误*/
    ID_DRV_STOP_RECORD_CNF                     = 7, /*通知DRV, hifi stop record回复*/
    ID_DRV_STOP_PLAY_CNF                       = 8, /*通知DRV, hifi stop record回复*/
    ID_DRV_HIFI_DRV_MAX                        = 10 /*max msg id*/
}HIFI_ALSA_VOICE_MSG_ID_ENUM;
typedef unsigned int HIFI_ALSA_VOICE_MSG_ID_ENUM_UINT32;


typedef enum drv_data_type_e
{
    DATA_TYPE_REMOTE_TTS_PCM = 0,
    DATA_TYPE_REMOTE_DTMF111 = 1,
    DATA_TYPE_MAX            = 9, /*max msg id*/
}ALSA_VOICE_DATA_TYPE_ENUM;
typedef unsigned int ALSA_VOICE_DATA_TYPE_ENUM_UINT32;

/* 请求开始结构体定义：*/
struct DRV_VOICE_START_REQ{

    unsigned int ulSampeRate;
    unsigned int uwBufAddr;     /*phy address */
    unsigned int uwBufSize;  
};

struct DRV_VOICE_START_CNF{

    unsigned int ulResult;  /*0 success, others error code */
};
/* 开始请求数据传输定义：*/
struct DRV_VOICE_DATA_REQ{
    unsigned int uwMode;        /*0--remote play, 1--record in call*/
    unsigned int uwBufAddr;     /*phy address */
    unsigned int uwBufSize;  
};

/* HIFI对地址里的数据处理完后的回复定义：*/
struct DRV_VOICE_DATA_CNF{
    unsigned int uwMode;        /*0--remote play, 1--record in call*/ 
};

/* 临时性错误或是不可恢复的错误主动上报结构体定义： */
struct DRV_VOICE_DATA_ERR_IND{

    unsigned int ulErrorCode;
};
/*
 Acore与SOUND_DRV_ALSA接口，SOUND_DRV_ALSA通过这些接口与HIFI交互
*/

/*控制命令接口*/
int HiFiAgent_SendVoiceMsgReq(ALSA_HIFI_VOICE_MSG_ID_ENUM_UINT32 cmd, void *data, unsigned int len);

/*驱动控制接口*/
int sound_drv_alsa_ioctrl (HIFI_ALSA_VOICE_MSG_ID_ENUM_UINT32 cmd, void *data, unsigned int len);


#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif


#endif











