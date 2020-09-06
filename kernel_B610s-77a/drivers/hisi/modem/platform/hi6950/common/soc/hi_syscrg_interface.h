#ifndef __HI_SYSCRG_INTER_H__
#define __HI_SYSCRG_INTER_H__ 
#include "hi_base.h"
#include "hi_syscrg.h"
static __inline__ void hi_syscrg_wdt_clk_enble(void)
{
}
static __inline__ void hi_syscrg_spi0_clk_enable(void)
{
}
static __inline__ void hi_syscrg_spi1_clk_enable(void)
{
}
static __inline__ void hi_syscrg_amon_cpufast_clk_enable(void)
{
}
static __inline__ void hi_syscrg_amon_soc_clk_enable(void)
{
}
static __inline__ void hi_syscrg_amon_cpufast_clk_disable(void)
{
}
static __inline__ void hi_syscrg_amon_soc_clk_disable(void)
{
}
static __inline__ unsigned int hi_syscrg_amon_cpufast_get_clk_status(void)
{
    return 0;
}
static __inline__ unsigned int hi_syscrg_amon_soc_get_clk_status(void)
{
    return 0;
}
static __inline__ void hi_syscrg_amon_cpufast_soft_reset(void)
{
}
static __inline__ void hi_syscrg_amon_soc_soft_reset(void)
{
}
static __inline__ void hi_syscrg_amon_cpufast_rls_reset(void)
{
}
static __inline__ void hi_syscrg_amon_soc_rls_reset(void)
{
}
#ifdef __FASTBOOT__
static __inline__ void hi_syscrg_usb_bc_clk_on(void)
{
    set_hi_ao_crg_clken1_bc_clk_en(1);
}
static __inline__ void hi_syscrg_usb_bc_clk_off(void)
{
   set_hi_ao_crg_clkdis1_bc_clk_dis(1);
}
static __inline__ void hi_syscrg_usb_bc_release(void)
{
 set_hi_ao_crg_srstdis1_bcctrl_srst_dis(1);
}
static __inline__ void hi_syscrg_usb_release(void)
{
    set_hi_pd_crg_clken2_usbotg_clk_en(1);
    set_hi_ao_crg_srstdis1_usbphy_srst_dis(1);
    set_hi_ao_crg_srstdis1_usbotg_ctrl_srst_dis(1);
    set_hi_ao_crg_srstdis1_usbctrl_vaux_srs_dis(1);
    set_hi_ao_crg_srstdis1_usbctrl_vcc_srst_dis(1);
}
static __inline__ void hi_syscrg_usb_reset(void)
{
 set_hi_ao_crg_srsten1_usbctrl_vcc_srst_en(1);
 set_hi_ao_crg_srsten1_usbctrl_vaux_srs_en(1);
 set_hi_ao_crg_srsten1_usbotg_ctrl_srst_en(1);
 set_hi_ao_crg_srsten1_usbphy_srst_en(1);
 set_hi_pd_crg_clkdis2_usbotg_clk_dis(1);
}
#endif
static __inline__ void hi_syscrg_efuse_clk_enble(void)
{
 set_hi_ao_crg_clken1_efuse_clk_en(1);
}
static __inline__ void hi_syscrg_efuse_clk_disable(void)
{
 set_hi_ao_crg_clkdis1_efuse_clk_dis(1);
}
static __inline__ void hi_syscrg_nandc_clk_enable(void)
{
}
static __inline__ void hi_crg_gpio1_clk_en(void)
{
}
static __inline__ void hi_crg_gpio2_clk_en(void)
{
}
static __inline__ void hi_crg_gpio3_clk_en(void)
{
}
static __inline__ void hi_crg_gpio1_clk_dis(void)
{
}
static __inline__ void hi_crg_gpio2_clk_dis(void)
{
}
static __inline__ void hi_crg_gpio3_clk_dis(void)
{
}
static __inline__ unsigned int hi_crg_gpio1_clk_stat(void)
{
 return 0;
}
static __inline__ unsigned int hi_crg_gpio2_clk_stat(void)
{
 return 0;
}
static __inline__ unsigned int hi_crg_gpio3_clk_stat(void)
{
 return 0;
}
static __inline__ void hi_syscrg_uart0_set_clk_val(unsigned int val __attribute__((unused)))
{
}
static __inline__ void hi_syscrg_uart0_set_clk_disable(void)
{
}
static __inline__ void hi_syscrg_uart0_set_clk_enable(void)
{
}
static __inline__ void hi_syscrg_dt_set_clk_enable(void)
{
}
static __inline__ void dsp_bbe_clock_enable(void)
{
    return;
}
static __inline__ void dsp_bbe_clock_disable(void)
{
    return;
}
static __inline__ void dsp_bbe_unreset(void)
{
    return;
}
static __inline__ void dsp_bbe_reset(void)
{
    return;
}
static __inline__ void dsp_clock_enable(void)
{
}
static __inline__ void dsp_clock_disable(void)
{
}
static __inline__ void dsp_unreset(void)
{
    return;
}
static __inline__ void dsp_reset(void)
{
    return;
}
static __inline__ int dsp_pll_status(void)
{
    return 1;
}
static __inline__ void dsp_pll_enable(void)
{
}
static __inline__ void dsp_pll_disable(void)
{
}
static __inline__ void dsp_bbe_refclk_enable(void)
{
    return;
}
static __inline__ void dsp_bbe_refclk_disable(void)
{
    return;
}
static __inline__ void dsp_bbe_chose_dsp_pll(void)
{
    return;
}
static __inline__ void dsp_bbe_chose_ccpu_pll(void)
{
    return;
}
static __inline__ unsigned int get_hi_crg_clkstat1(void)
{ return 0;}
static __inline__ unsigned int get_hi_crg_clkstat2(void)
{ return 0;}
static __inline__ unsigned int get_hi_crg_clkstat3(void)
{ return 0;}
static __inline__ unsigned int get_hi_crg_clkstat4(void)
{ return 0;}
static __inline__ unsigned int get_hi_crg_clkstat5(void)
{ return 0;}
static __inline__ void hifi_clock_enable(void)
{
}
static __inline__ void hifi_clock_disable(void)
{
}
static __inline__ void hifi_unreset(void)
{
}
static __inline__ void hifi_reset(void)
{
}
#endif
