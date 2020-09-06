#ifndef __HI_SYSSC_INTER_H__
#define __HI_SYSSC_INTER_H__ 
#include "hi_base.h"
#include "hi_syssc.h"
#include "hi_sc_pd.h"
static __inline__ void hi_syssc_wdt_reboot(void)
{
    set_hi_sc_ao_ctrl1_pmu_rstout_n(0);
}
static __inline__ void hi_syssc_wdt_enble(void)
{
}
static __inline__ void hi_syssc_amon_cpufast_start(void)
{
#if 0
    if(get_hi_sc_peri_ctrl0_amon_cpufast_monitor_start())
    {
        set_hi_sc_peri_ctrl0_amon_cpufast_monitor_start(0);
    }
    set_hi_sc_peri_ctrl0_amon_cpufast_monitor_start(1);
#endif
}
static __inline__ void hi_syssc_amon_soc_start(void)
{
#if 0
    if(get_hi_sc_peri_ctrl0_amon_soc_monitor_start())
    {
        set_hi_sc_peri_ctrl0_amon_soc_monitor_start(0);
    }
    set_hi_sc_peri_ctrl0_amon_soc_monitor_start(1);
#endif
}
static __inline__ void hi_syssc_amon_cpufast_stop(void)
{
}
static __inline__ void hi_syssc_amon_soc_stop(void)
{
}
static __inline__ unsigned int get_efuse_tsensor_trim_value(void)
{
 return get_hi_sc_ao_stat14_efuse_tsensor_trim();
}
static __inline__ void hi_syssc_ebi_mode_normal(void)
{
    set_hi_sc_peri_ctrl15_ebi_normal_mode(1);
}
static __inline__ void hi_syssc_pmu_reboot(void)
{
    set_hi_sc_ao_ctrl1_pmu_rstout_n(0);
}
static __inline__ void hi_syssc_usb_ref_ssp(unsigned int enable)
{
    set_hi_sc_ao_ctrl13_ref_ssp_en(!!enable);
}
static __inline__ void hi_syssc_usb_powerdown_hsp(unsigned int enable)
{
    set_hi_sc_ao_ctrl13_test_powerdown_hsp(!!enable);
}
static __inline__ void hi_syssc_usb_powerdown_ssp(unsigned int enable)
{
    set_hi_sc_ao_ctrl13_test_powerdown_ssp(!!enable);
}
static __inline__ void hi_syssc_usb_vbusvldext(unsigned int enable)
{
    set_hi_sc_ao_ctrl13_vbusvldext(!!enable);
    set_hi_sc_ao_ctrl13_vbusvldextsel(!!enable);
}
static __inline__ void hi_syssc_usb_override(unsigned int enable)
{
    set_hi_sc_ao_ctrl18_usb3_pmu_powerpresent_override_en(!!enable);
    set_hi_sc_ao_ctrl18_usb3_pmu_avalid_override_en(!!enable);
    set_hi_sc_ao_ctrl18_usb3_pmu_bvalid_override_en(!!enable);
    set_hi_sc_ao_ctrl18_usb3_pmu_vbusvalid_override_en(!!enable);
    set_hi_sc_ao_ctrl18_usb3_pmu_powerpresent_override(!!enable);
    set_hi_sc_ao_ctrl18_usb3_pmu_avalid_override(!!enable);
    set_hi_sc_ao_ctrl18_usb3_pmu_bvalid_override(!!enable);
    set_hi_sc_ao_ctrl18_usb3_pmu_vbusvalid_override(!!enable);
}
static __inline__ void hi_syssc_usb_txpreempamptune(unsigned int txpreempamptune)
{
    set_hi_sc_ao_ctrl15_txpreempamptune(txpreempamptune);
}
static __inline__ void hi_syssc_usb_iddig(unsigned int value)
{
    set_hi_sc_ao_ctrl18_usb3_pmu_iddig_override(value);
}
static __inline__ void hi_syssc_usb_iddig_en(unsigned int value)
{
    set_hi_sc_ao_ctrl18_usb3_pmu_iddig_override_en(value?1:0);
}
static __inline__ void hi_syssc_usb_phy3_init(void)
{
    set_hi_sc_ao_ctrl16_los_level4_3(0x1);
    set_hi_sc_ao_ctrl16_pcs_tx_deemph_3p5db(0x15);
    set_hi_sc_ao_ctrl16_pcs_tx_deemph_6db(0x20);
    set_hi_sc_ao_ctrl16_pcs_tx_swing_full(0x71);
    set_hi_sc_ao_ctrl16_lane0_tx_term_offset(0x0);
    set_hi_sc_ao_ctrl16_tx_vboost_lvl(0x7);
    set_hi_sc_ao_ctrl14_ssc_en(0);
    set_hi_sc_ao_ctrl14_usb3phy_idpullup(1);
    set_hi_sc_ao_ctrl15_los_bias(3);
}
static __inline__ void hi_syssc_pmu_hold_down(void)
{
 set_hi_sc_ao_ctrl1_pmu_hold(0);
}
static __inline__ void hi_syssc_dt_testmode_switch(unsigned int value)
{
 set_hi_sc_peri_ctrl16_func_ddr_testmode(value?1:0);
}
static __inline__ void edma_ch4_set_axi_mst_min(void)
{
    return;
}
static __inline__ void dsp_set_bbe16_run(void)
{
}
static __inline__ void dsp_set_bbe16_stop(void)
{
}
static __inline__ void hifi_run(void)
{
}
static __inline__ void hifi_stop(void)
{
}
#define edma_dw_ssi0_dma_sel(edma_id) 
#define edma_dw_ssi1_dma_sel(edma_id) 
#define edma_sio_dma_sel(edma_id) 
#define edma_hs_uart_dma_sel(edma_id) 
#define edma_uart0_dma_sel(edma_id) 
#define edma_uart1_dma_sel(edma_id) 
#define edma_uart2_dma_sel(edma_id) 
#define edma_sci0_dma_sel(edma_id) 
#define edma_sci1_dma_sel(edma_id) 
#define edma_bbp_dbg_dma_sel(edma_id) 
#define edma_bbp_grif_dma_sel(edma_id) 
#define edma_amon_soc_dma_sel(edma_id) 
#define edma_amon_fast_dma_sel(edma_id) 
static __inline__ void hi_syssc_set_appa9_boot_addr(unsigned int boot_addr __attribute__((unused)))
{
}
static __inline__ void hi_syssc_set_mdma9_boot_addr(unsigned int boot_addr __attribute__((unused)))
{
}
#endif
