#ifndef __HI_PWM_H__
#define __HI_PWM_H__ 
#ifndef HI_SET_GET
#define HI_SET_GET(a0,a1,a2,a3,a4) 
#endif
#define HI_PWM_OUTA_EN_OFFSET (0x0)
#define HI_PWM_OUTB_EN_OFFSET (0x4)
#define HI_PWM_OUTA_DIV_OFFSET (0x8)
#define HI_PWM_OUTB_DIV_OFFSET (0xC)
#define HI_PWM_OUTA_WIDE_OFFSET (0x10)
#define HI_PWM_OUTB_WIDE_OFFSET (0x14)
#define HI_PWM_OUTA_WARN_OFFSET (0x18)
#define HI_PWM_OUTB_WARN_OFFSET (0x1C)
#define HI_PWM_UNLOCK (0x1acce551)
#define HI_PWM_LOCK_OFFSET (0x0)
#define HI_PWM_CTL_OFFSET (0x4)
#define HI_PWM_CFG_OFFSET (0x8)
#define HI_PWM_PR0_OFFSET (0x100)
#define HI_PWM_PR1_OFFSET (0x104)
#define HI_PWM_C0_MR_OFFSET (0x300)
#define HI_PWM_C1_MR_OFFSET (0x400)
#define HI_PWM_C0_MR0_OFFSET (0x304)
#define HI_PWM_C1_MR0_OFFSET (0x404)
typedef union
{
    struct
    {
        unsigned int OUTA_EN : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_PWM_OUTA_EN_T;
typedef union
{
    struct
    {
        unsigned int OUTB_EN : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_PWM_OUTB_EN_T;
typedef union
{
    struct
    {
        unsigned int OUTA_DIV : 11;
        unsigned int reserved : 21;
    } bits;
    unsigned int u32;
}HI_PWM_OUTA_DIV_T;
typedef union
{
    struct
    {
        unsigned int OUTB_DIV : 11;
        unsigned int reserved : 21;
    } bits;
    unsigned int u32;
}HI_PWM_OUTB_DIV_T;
typedef union
{
    struct
    {
        unsigned int OUTA_WIDE : 11;
        unsigned int reserved : 21;
    } bits;
    unsigned int u32;
}HI_PWM_OUTA_WIDE_T;
typedef union
{
    struct
    {
        unsigned int OUTB_WIDE : 11;
        unsigned int reserved : 21;
    } bits;
    unsigned int u32;
}HI_PWM_OUTB_WIDE_T;
typedef union
{
    struct
    {
        unsigned int OUTA_WARN : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_PWM_OUTA_WARN_T;
typedef union
{
    struct
    {
        unsigned int OUTB_WARN : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_PWM_OUTB_WARN_T;
HI_SET_GET(hi_pwm_outa_en_outa_en,outa_en,HI_PWM_OUTA_EN_T,HI_PWM_BASE_ADDR, HI_PWM_OUTA_EN_OFFSET)
HI_SET_GET(hi_pwm_outa_en_reserved,reserved,HI_PWM_OUTA_EN_T,HI_PWM_BASE_ADDR, HI_PWM_OUTA_EN_OFFSET)
HI_SET_GET(hi_pwm_outb_en_outb_en,outb_en,HI_PWM_OUTB_EN_T,HI_PWM_BASE_ADDR, HI_PWM_OUTB_EN_OFFSET)
HI_SET_GET(hi_pwm_outb_en_reserved,reserved,HI_PWM_OUTB_EN_T,HI_PWM_BASE_ADDR, HI_PWM_OUTB_EN_OFFSET)
HI_SET_GET(hi_pwm_outa_div_outa_div,outa_div,HI_PWM_OUTA_DIV_T,HI_PWM_BASE_ADDR, HI_PWM_OUTA_DIV_OFFSET)
HI_SET_GET(hi_pwm_outa_div_reserved,reserved,HI_PWM_OUTA_DIV_T,HI_PWM_BASE_ADDR, HI_PWM_OUTA_DIV_OFFSET)
HI_SET_GET(hi_pwm_outb_div_outb_div,outb_div,HI_PWM_OUTB_DIV_T,HI_PWM_BASE_ADDR, HI_PWM_OUTB_DIV_OFFSET)
HI_SET_GET(hi_pwm_outb_div_reserved,reserved,HI_PWM_OUTB_DIV_T,HI_PWM_BASE_ADDR, HI_PWM_OUTB_DIV_OFFSET)
HI_SET_GET(hi_pwm_outa_wide_outa_wide,outa_wide,HI_PWM_OUTA_WIDE_T,HI_PWM_BASE_ADDR, HI_PWM_OUTA_WIDE_OFFSET)
HI_SET_GET(hi_pwm_outa_wide_reserved,reserved,HI_PWM_OUTA_WIDE_T,HI_PWM_BASE_ADDR, HI_PWM_OUTA_WIDE_OFFSET)
HI_SET_GET(hi_pwm_outb_wide_outb_wide,outb_wide,HI_PWM_OUTB_WIDE_T,HI_PWM_BASE_ADDR, HI_PWM_OUTB_WIDE_OFFSET)
HI_SET_GET(hi_pwm_outb_wide_reserved,reserved,HI_PWM_OUTB_WIDE_T,HI_PWM_BASE_ADDR, HI_PWM_OUTB_WIDE_OFFSET)
HI_SET_GET(hi_pwm_outa_warn_outa_warn,outa_warn,HI_PWM_OUTA_WARN_T,HI_PWM_BASE_ADDR, HI_PWM_OUTA_WARN_OFFSET)
HI_SET_GET(hi_pwm_outa_warn_reserved,reserved,HI_PWM_OUTA_WARN_T,HI_PWM_BASE_ADDR, HI_PWM_OUTA_WARN_OFFSET)
HI_SET_GET(hi_pwm_outb_warn_outb_warn,outb_warn,HI_PWM_OUTB_WARN_T,HI_PWM_BASE_ADDR, HI_PWM_OUTB_WARN_OFFSET)
HI_SET_GET(hi_pwm_outb_warn_reserved,reserved,HI_PWM_OUTB_WARN_T,HI_PWM_BASE_ADDR, HI_PWM_OUTB_WARN_OFFSET)
#endif
