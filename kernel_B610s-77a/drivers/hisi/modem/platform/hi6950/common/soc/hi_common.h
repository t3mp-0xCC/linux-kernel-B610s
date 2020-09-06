#ifndef __HI_COMMON_H__
#define __HI_COMMON_H__ 
#include "bsp_memmap.h"
#include "hi_crg_pd.h"
#include "hi_crg_ao.h"
#include "hi_sc_pd.h"
static __inline__ void prepare_ccpu_a9(void)
{
    (*(volatile unsigned *) (HI_IO_ADDRESS(0x80200000 + 0x20))) = (0x8040);
    (*(volatile unsigned *) (HI_IO_ADDRESS(0x80200000 + 0x00))) = (0x28);
}
static __inline__ void start_ccpu_a9(unsigned int addr)
{
    (*(volatile unsigned *) (HI_IO_ADDRESS(0x80200000 + 0x418))) = (addr);
    (*(volatile unsigned *) (HI_IO_ADDRESS(0x80200000 + 0x24))) = (0x8040);
}
static __inline__ void hi_mcore_reset(void)
{
    (*(volatile unsigned *) (HI_IO_ADDRESS(HI_AO_CRG_REG_BASE_ADDR + HI_AO_CRG_SRSTEN1_OFFSET))) = (0x2);
}
static __inline__ void hi_mcore_active(void)
{
    (*(volatile unsigned *) (HI_IO_ADDRESS(HI_AO_CRG_REG_BASE_ADDR + HI_AO_CRG_SRSTDIS1_OFFSET))) = (0x2);
}
static __inline__ void hi_acore_set_entry_addr(int a)
{
    (*(volatile unsigned *) (HI_IO_ADDRESS(HI_SYSCTRL_PD_REG_BASE_ADDR + HI_SC_TOP_CTRL0_OFFSET))) = (a);
}
#endif
