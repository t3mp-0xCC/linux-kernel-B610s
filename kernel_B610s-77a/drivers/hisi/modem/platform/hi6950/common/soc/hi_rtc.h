#ifndef __HI_RTC_H__
#define __HI_RTC_H__ 
#ifndef HI_SET_GET
#define HI_SET_GET(a0,a1,a2,a3,a4) 
#endif
#define HI_RTC_DR_OFFSET (0x0)
#define HI_RTC_MR_OFFSET (0x4)
#define HI_RTC_LR_OFFSET (0x8)
#define HI_RTC_CR_OFFSET (0xC)
#define HI_RTC_IMSC_OFFSET (0x10)
#define HI_RTCRIS_OFFSET (0x14)
#define HI_RTCMIS_OFFSET (0x18)
#define HI_RTCICR_OFFSET (0x1C)
#ifndef __ASSEMBLY__
typedef union
{
    struct
    {
        unsigned int rtcdr : 32;
    } bits;
    unsigned int u32;
}HI_RTC_DR_T;
typedef union
{
    struct
    {
        unsigned int rtcmr : 32;
    } bits;
    unsigned int u32;
}HI_RTC_MR_T;
typedef union
{
    struct
    {
        unsigned int rtclr : 32;
    } bits;
    unsigned int u32;
}HI_RTC_LR_T;
typedef union
{
    struct
    {
        unsigned int rtccr : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_RTC_CR_T;
typedef union
{
    struct
    {
        unsigned int rtcimsc : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_RTC_IMSC_T;
typedef union
{
    struct
    {
        unsigned int rtcris : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_RTCRIS_T;
typedef union
{
    struct
    {
        unsigned int rtcmis : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_RTCMIS_T;
typedef union
{
    struct
    {
        unsigned int rtcicr : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_RTCICR_T;
HI_SET_GET(hi_rtc_dr_rtcdr,rtcdr,HI_RTC_DR_T,HI_RTC_BASE_ADDR, HI_RTC_DR_OFFSET)
HI_SET_GET(hi_rtc_mr_rtcmr,rtcmr,HI_RTC_MR_T,HI_RTC_BASE_ADDR, HI_RTC_MR_OFFSET)
HI_SET_GET(hi_rtc_lr_rtclr,rtclr,HI_RTC_LR_T,HI_RTC_BASE_ADDR, HI_RTC_LR_OFFSET)
HI_SET_GET(hi_rtc_cr_rtccr,rtccr,HI_RTC_CR_T,HI_RTC_BASE_ADDR, HI_RTC_CR_OFFSET)
HI_SET_GET(hi_rtc_cr_reserved,reserved,HI_RTC_CR_T,HI_RTC_BASE_ADDR, HI_RTC_CR_OFFSET)
HI_SET_GET(hi_rtc_imsc_rtcimsc,rtcimsc,HI_RTC_IMSC_T,HI_RTC_BASE_ADDR, HI_RTC_IMSC_OFFSET)
HI_SET_GET(hi_rtc_imsc_reserved,reserved,HI_RTC_IMSC_T,HI_RTC_BASE_ADDR, HI_RTC_IMSC_OFFSET)
HI_SET_GET(hi_rtcris_rtcris,rtcris,HI_RTCRIS_T,HI_RTC_BASE_ADDR, HI_RTCRIS_OFFSET)
HI_SET_GET(hi_rtcris_reserved,reserved,HI_RTCRIS_T,HI_RTC_BASE_ADDR, HI_RTCRIS_OFFSET)
HI_SET_GET(hi_rtcmis_rtcmis,rtcmis,HI_RTCMIS_T,HI_RTC_BASE_ADDR, HI_RTCMIS_OFFSET)
HI_SET_GET(hi_rtcmis_reserved,reserved,HI_RTCMIS_T,HI_RTC_BASE_ADDR, HI_RTCMIS_OFFSET)
HI_SET_GET(hi_rtcicr_rtcicr,rtcicr,HI_RTCICR_T,HI_RTC_BASE_ADDR, HI_RTCICR_OFFSET)
HI_SET_GET(hi_rtcicr_reserved,reserved,HI_RTCICR_T,HI_RTC_BASE_ADDR, HI_RTCICR_OFFSET)
#endif
#endif
