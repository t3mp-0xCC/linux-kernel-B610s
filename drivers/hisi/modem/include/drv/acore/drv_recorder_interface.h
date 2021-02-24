#ifndef __ACORE_RECORD_PLAY_DRV_INTERFACE_H__
#define __ACORE_RECORD_PLAY_DRV_INTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum drv_hifi_cmd_type_e
{
    ID_DRV_HIFI_CMD_NOT_VALID               = 0, /*��Ч����*/
    ID_DRV_START_RECORD_REQ                 = 1, /*����hifi����ʼ¼��*/
    ID_DRV_STOP_RECORD_REQ                  = 2, /*����hifi������¼��*/
    ID_DRV_START_PLAY_REQ                   = 3, /*����hifi����ʼԶ�̲���*/
    ID_DRV_STOP_PLAY_REQ                    = 4, /*����hifi������Զ�̲���*/
    ID_DRV_SET_BUFFER_REQ                   = 5, /*֪ͨhifi���š�¼�������ݴ�ŵ�ַ*/
    ID_DRV_HIFI_CMD_MAX                     = 9, /*max msg id*/
}ALSA_HIFI_VOICE_MSG_ID_ENUM;
typedef unsigned int ALSA_HIFI_VOICE_MSG_ID_ENUM_UINT32;

typedef enum hifi_drv_cmd_type_e
{
    ID_HIFI_DRV_NOT_VALID                      = 0, /*��Ч����*/
    ID_DRV_RECORD_START_CNF                    = 1, /*֪ͨDRV, hifi�Ѿ�׼��������¼��*/
    ID_DRV_START_PLAY_CNF                      = 2, /*֪ͨDRV, hifi�Ѿ�׼���ô�������*/
    ID_HIFI_DRV_BUFFER_DATA_READY              = 3,  /*֪ͨDRV,hifi�Ѿ��ڻ�������������*/
    ID_DRV_HIFI_RECORD_ERR_NOTIFY              = 4, /*֪ͨDRV, hifi������ʱ����*/
    ID_HIFI_DRV_CALL_END_NOTIFY                = 5, /*֪ͨDRV, ͨ������*/
    ID_DRV_HIFI_PLAY_ERR_NOTIFY                = 6, /*֪ͨDRV, hifi������ʱ����*/
    ID_DRV_STOP_RECORD_CNF                     = 7, /*֪ͨDRV, hifi stop record�ظ�*/
    ID_DRV_STOP_PLAY_CNF                       = 8, /*֪ͨDRV, hifi stop record�ظ�*/
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

/* ����ʼ�ṹ�嶨�壺*/
struct DRV_VOICE_START_REQ{

    unsigned int ulSampeRate;
    unsigned int uwBufAddr;     /*phy address */
    unsigned int uwBufSize;  
};

struct DRV_VOICE_START_CNF{

    unsigned int ulResult;  /*0 success, others error code */
};
/* ��ʼ�������ݴ��䶨�壺*/
struct DRV_VOICE_DATA_REQ{
    unsigned int uwMode;        /*0--remote play, 1--record in call*/
    unsigned int uwBufAddr;     /*phy address */
    unsigned int uwBufSize;  
};

/* HIFI�Ե�ַ������ݴ������Ļظ����壺*/
struct DRV_VOICE_DATA_CNF{
    unsigned int uwMode;        /*0--remote play, 1--record in call*/ 
};

/* ��ʱ�Դ�����ǲ��ɻָ��Ĵ��������ϱ��ṹ�嶨�壺 */
struct DRV_VOICE_DATA_ERR_IND{

    unsigned int ulErrorCode;
};
/*
 Acore��SOUND_DRV_ALSA�ӿڣ�SOUND_DRV_ALSAͨ����Щ�ӿ���HIFI����
*/

/*��������ӿ�*/
int HiFiAgent_SendVoiceMsgReq(ALSA_HIFI_VOICE_MSG_ID_ENUM_UINT32 cmd, void *data, unsigned int len);

/*�������ƽӿ�*/
int sound_drv_alsa_ioctrl (HIFI_ALSA_VOICE_MSG_ID_ENUM_UINT32 cmd, void *data, unsigned int len);


#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif


#endif











