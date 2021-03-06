#ifndef __HI_HKADC_H__
#define __HI_HKADC_H__ 
#ifdef __cplusplus
extern "C" {
#endif
#define HKADC_TWO_INSTRUCTION_DELAY 0xC00037
#define BSP_HKADC_OPTIMIZED 
enum HKADC_CHANNEL_ID{
    HKADC_CHANNEL_MIN = 0,
    HKADC_CHANNEL_0 = 0,
    HKADC_CHANNEL_1,
    HKADC_CHANNEL_2,
    HKADC_CHANNEL_3,
    HKADC_CHANNEL_4,
    HKADC_CHANNEL_5,
    HKADC_CHANNEL_6,
    HKADC_CHANNEL_7,
    HKADC_CHANNEL_8,
    HKADC_CHANNEL_9,
    HKADC_CHANNEL_10,
    HKADC_CHANNEL_11,
    HKADC_CHANNEL_12,
    HKADC_CHANNEL_13,
    HKADC_CHANNEL_MAX = 13,
};
typedef enum _ehard_ware_version
{
    HW_VERSION_BIG_LOW = HKADC_CHANNEL_4,
    HW_VERSION_BIG_HIGH= HKADC_CHANNEL_5,
    HW_VERSION_PRODUCT = HKADC_CHANNEL_8,
    HW_VERSION_UDP = HKADC_CHANNEL_13
}hard_ware_version;
#ifdef __cplusplus
}
#endif
#endif
