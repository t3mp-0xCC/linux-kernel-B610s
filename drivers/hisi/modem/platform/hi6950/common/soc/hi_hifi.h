#ifndef __HI_HIFI_H__
#define __HI_HIFI_H__ 
#include <osl_bio.h>
#include <bsp_memmap.h>
#ifdef __cplusplus
extern "C" {
#endif
#define HIFI_STRESS_MAGIC 0x5555aaaa
#define HIFI_TCM_START_ADDR 0x7fe00000
#define HIFI_TCM_END_ADDR 0x7fe11000
#define HIFI_DATA_TCM_ADDR 0x8fe00000
#define HIFI_TOTAL_TCM_SIZE 0x11000
#define HIFI_TCM_BASE_ADDR (HI_HIFI2DMEM0_BASE_ADDR)
#ifndef __KERNEL__
static inline void hifi_sc_set(u32 addr, u32 start, u32 end, u32 val)
{
 unsigned int tmp = readl(addr);
 tmp &= (~(((1 << (end - start + 1)) - 1) << start));
 tmp |= (val << start);
 writel(tmp, addr);
}
static inline unsigned int hifi_sc_get(u32 addr, u32 start, u32 end)
{
 unsigned int tmp = readl(addr);
    return ((tmp >> start) & ((1 << (end - start + 1)) - 1));
}
static inline void hifi_run(void)
{
    hifi_sc_set(0x90000414, 0x1, 0x1, 0x0);
}
static inline void hifi_stop(void)
{
 hifi_sc_set(0x90000414, 0x1, 0x1, 0x1);
}
static inline void hifi_power_on(void)
{
}
static inline void hifi_iso_dis(void)
{
}
static inline void hifi_iso_en(void)
{
}
static inline void hifi_power_off(void)
{
}
static inline void hifi_clock_enable(void)
{
    hifi_sc_set(0x90000020, 31, 31, 0x1);
    hifi_sc_set(0x90000020, 30, 30, 0x1);
}
static inline void hifi_clock_disable(void)
{
    hifi_sc_set(0x90000024, 31, 31, 0x1);
    hifi_sc_set(0x90000024, 30, 30, 0x1);
}
static inline void hifi_unreset(void)
{
    hifi_sc_set(0x90000070, 18, 18, 0x1);
    hifi_sc_set(0x90000070, 19, 19, 0x1);
    hifi_sc_set(0x90000070, 20, 20, 0x1);
}
static inline void hifi_reset(void)
{
    hifi_sc_set(0x9000006c, 18, 18, 0x1);
    hifi_sc_set(0x9000006c, 19, 19, 0x1);
    hifi_sc_set(0x9000006c, 20, 20, 0x1);
}
#endif
#ifdef __cplusplus
}
#endif
#endif
