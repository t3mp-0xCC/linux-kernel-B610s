
#ifndef __MBB_CLIENT_CODEC_H__
#define __MBB_CLIENT_CODEC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "product_config.h"

#if ( FEATURE_ON == MBB_CLIENT_CODEC )
//#define NAU8814_REGISTER_COUNT (0x50)
#define NAU8814_CODEC_ADDR    0x1A
struct nau8814 {
    int (*read_dev)(void *io_data, void *read_data);
    int (*write_dev)(void *io_data, void *write_data);
    void *io_data;
};
typedef enum CODEC_COMMAND_ID
{
    CODEC_POW_CMD,
    CODEC_SETREG_CMD,
    CODEC_GETREG_CMD,
}CODEC_COMMAND_ID_E;
//寄存器设置类型
typedef struct REGISTER_SETTING
{
    unsigned char reg_addr;
    unsigned short data;
}REGISTER_SETTING_T;
//CODEC操作信息
typedef struct CODEC_INFO
{
    CODEC_COMMAND_ID_E      cmd_id;
    unsigned char    codec_type;
    REGISTER_SETTING_T *codec_data;
}CODEC_INFO_T;
/******************************************************************************
Function:    nau8814_proc_cmd
Description:interface function which is exposed to AT module to config codec by i2c 
Input:        CODEC_INFO_T *codec_cmd
Output:      NONE
Return:        int, 0 for success
Others:       NONE
******************************************************************************/
int nau8814_proc_cmd(CODEC_INFO_T *codec_cmd);
#endif
#endif
