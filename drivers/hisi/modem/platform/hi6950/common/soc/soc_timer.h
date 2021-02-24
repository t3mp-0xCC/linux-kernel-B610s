#ifndef __SOC_TIMER_H__
#define __SOC_TIMER_H__ 
#include <hi_base.h>
#include <hi_timer.h>
#include <hi_syscrg.h>
#include <hi_bbp_systime.h>
#include <bsp_memmap.h>
#include <soc_interrupts.h>
#include <soc_clk.h>
#define UDELAY_TIMER_CLK HI_TIMER_CLK
#define CCORE_SYS_TIMER_CLK HI_TIMER_CLK
#define CCORE_SYS_TIMER_BASE_ADDR HI_TIMER_12_REGBASE_ADDR_VIRT
#define CCORE_SYS_TIMER_INT_LVL INT_LVL_TIMER12
#define FASTBOOT_TIMER_BASE 0x20000000
#define FASTBOOT_TIMER_CURRENTVALUE 0x614
#define UDELAY_TIMER_ADDR 0x80220000
#define CLK_DEF_TC_INTENABLE (1<<5)
#define CLK_DEF_TC_INTDISABLE (0<<5)
#define CLK_DEF_TC_PERIODIC (1<<6)
#define CLK_DEF_TC_FREERUN 0
#define CLK_DEF_TC_ENABLE (1<<7 )
#define CLK_DEF_TC_DISABLE 0
#define CLK_DEF_TC_COUNT32BIT (1<<1)
#define CLK_DEF_TC_COUNT16BIT 0
#define CLK_DEF_ENABLE (CLK_DEF_TC_PERIODIC | CLK_DEF_TC_INTENABLE | CLK_DEF_TC_ENABLE|CLK_DEF_TC_COUNT32BIT)
#define CLK_DEF_DISABLE (CLK_DEF_TC_PERIODIC | CLK_DEF_TC_INTDISABLE | CLK_DEF_TC_DISABLE|CLK_DEF_TC_COUNT32BIT)
#ifndef __ASSEMBLY__
#if defined(__KERNEL__)
static __inline__ void soc_cache_sync(void)
{
 mb();
}
#elif defined(__OS_VXWORKS__)||defined(__OS_RTOSCK__)
extern void osL2CacheSync(void);
static __inline__ void SOC_DWB(void)
{
    asm volatile ( "dsb;" );
}
static __inline__ void soc_cache_sync(void)
{
    SOC_DWB();
#ifdef CONFIG_BALONG_L2CACHE
    osL2CacheSync();
#endif
}
#elif defined(__CMSIS_RTOS)||defined(__FASTBOOT__)
static __inline__ void soc_cache_sync(void)
{}
#endif
static inline void soc_writel(unsigned val, unsigned addr)
{
    soc_cache_sync();
    (*(volatile unsigned *) (addr)) = (val);
}
static inline unsigned soc_readl(unsigned addr)
{
    return (*(volatile unsigned *) (addr));
}
static inline void __set_timer_rate(void)
{
 unsigned int ret = 0;
 soc_writel(0x7000,HI_AO_CRG_REG_BASE_ADDR+0x4);
 soc_writel(0x5400,HI_AO_CRG_REG_BASE_ADDR+0X45c);
 soc_writel(0x7000,HI_AO_CRG_REG_BASE_ADDR+0x0);
 soc_writel(0x2,HI_MODEM_SC_BASE_ADDR+0x14);
 ret = soc_readl(HI_MODEM_SC_BASE_ADDR+0X47c);
 soc_writel(ret|0x4,HI_MODEM_SC_BASE_ADDR+0X47c);
 soc_writel(0x2,HI_MODEM_SC_BASE_ADDR+0x10);
}
static inline void systimer_int_clear(unsigned addr)
{
 soc_writel(0x1,addr+0xc);
}
static inline unsigned int systimer_check_enable_success(void)
{
 return 0;
}
static inline void init_timer_stamp(void)
{
 soc_writel(0x1,UDELAY_TIMER_ADDR+0x20);
 soc_writel(0x101,UDELAY_TIMER_ADDR);
}
static inline void acore_clocksource_enable_and_mask_int(unsigned addr)
{
 soc_writel(CLK_DEF_ENABLE&(~ CLK_DEF_TC_INTENABLE),addr+0x8);
}
#endif
#define TIMER_ARM_FEATURE 
#define platform_set_timer_rate() do{}while(0)
#define ARM_ODD_LOAD_OFFSET 0x20
#define ARM_ODD_VALUE_OFFSET 0x24
#define ARM_ODD_CTRL_OFFSET 0x28
#define ARM_ODD_INTCLR_OFFSET 0x2C
#define ARM_ODD_INTRIS_OFFSET 0x30
#define ARM_ODD_INTMIS_OFFSET 0x34
#define ARM_ODD_BGLOAD_OFFSET 0x38
#define ARM_EVEN_LOAD_OFFSET 0x0
#define ARM_EVEN_VALUE_OFFSET 0x4
#define ARM_EVEN_CTRL_OFFSET 0x8
#define ARM_EVEN_INTCLR_OFFSET 0xC
#define ARM_EVEN_INTRIS_OFFSET 0x10
#define ARM_EVEN_INTMIS_OFFSET 0x14
#define ARM_EVEN_BGLOAD_OFFSET 0x18
#endif
