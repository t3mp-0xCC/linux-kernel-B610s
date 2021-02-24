#ifndef __HI_PCIE_INTERFACE_H__
#define __HI_PCIE_INTERFACE_H__ 
#ifdef __cplusplus
extern "C" {
#endif
#define BALONG_PCI_INNER_CLK 
#define BALONG_PCIE_MAX_DMA_CHANNEL_NUM (8)
static __inline__ u32 hi_pcie_get(void* __virt_addr, u32 __bit_offset, u32 __bit_width)
{
    u32 __value = 0;
    __value = readl(__virt_addr);
    __value >>= __bit_offset;
    __value &= ((1 << __bit_width) - 1);
    return __value;
}
static __inline__ void hi_pcie_set(void* __virt_addr, u32 __bit_offset, u32 __bit_width, u32 __bit_value)
{
    u32 __value = 0;
    __value = readl(__virt_addr);
    __value &= ~(((1 << __bit_width) - 1) << __bit_offset);
    __value |= (__bit_value << __bit_offset);
    writel(__value, (void*)__virt_addr);
}
static __inline__ void hi_pcie_dbi_enable(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC28, 0, 22, 0x200000);
        hi_pcie_set(__virt_addr + 0xC2C, 0, 22, 0x200000);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xC88, 0, 22, 0x200000);
        hi_pcie_set(__virt_addr + 0xC8C, 0, 22, 0x200000);
    }
}
static __inline__ void hi_pcie_dbi_disable(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC28, 0, 22, 0);
        hi_pcie_set(__virt_addr + 0xC2C, 0, 22, 0);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xC88, 0, 22, 0);
        hi_pcie_set(__virt_addr + 0xC8C, 0, 22, 0);
    }
}
static __inline__ void hi_pcie_set_work_mode(void* __virt_addr, u32 __pcie_id, u32 __pcie_work_mode)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC28, 28, 4, __pcie_work_mode);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xC88, 28, 4, __pcie_work_mode);
    }
}
static __inline__ void hi_pcie_ltssm_enable(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC44, 11, 1, 1);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xCA4, 11, 1, 1);
    }
}
static __inline__ void hi_pcie_ltssm_disable(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC44, 11, 1, 0);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xCA4, 11, 1, 0);
    }
}
static __inline__ u32 hi_pcie_get_link_status(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
        return hi_pcie_get(__virt_addr + 0xE3C, 0, 32);
    else
        return hi_pcie_get(__virt_addr + 0xE64, 0, 32);
}
static __inline__ u32 hi_pcie_is_linkup(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
        return (hi_pcie_get(__virt_addr + 0xE38, 5, 1) & hi_pcie_get(__virt_addr + 0xE38, 15, 1));
    else
        return (hi_pcie_get(__virt_addr + 0xE60, 5, 1) & hi_pcie_get(__virt_addr + 0xE60, 15, 1));
}
static __inline__ void hi_pcie_linkdown_interrupt_clear(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC54, 11, 1, 1);
        hi_pcie_set(__virt_addr + 0xC54, 12, 1, 1);
        hi_pcie_set(__virt_addr + 0xC54, 11, 1, 0);
        hi_pcie_set(__virt_addr + 0xC54, 12, 1, 0);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xCB4, 11, 1, 1);
        hi_pcie_set(__virt_addr + 0xCB4, 12, 1, 1);
        hi_pcie_set(__virt_addr + 0xCB4, 11, 1, 0);
        hi_pcie_set(__virt_addr + 0xCB4, 12, 1, 0);
    }
}
static __inline__ void hi_soc_peri_usb_deassert_reset(void* __virt_addr, u32 __pcie_id)
{
}
static __inline__ void hi_pcie_phy_deassert_reset(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC18, 22, 1, 0);
        hi_pcie_set(__virt_addr + 0xC1C, 16, 1, 0);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xC78, 22, 1, 0);
        hi_pcie_set(__virt_addr + 0xC7C, 16, 1, 0);
    }
}
static __inline__ u32 hi_pcie_is_under_reset(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        return hi_pcie_get(__virt_addr + 0xE8C, 19, 1);
    }
    else
    {
        return hi_pcie_get(__virt_addr + 0xE88, 19, 1);
    }
}
static __inline__ void hi_pcie_phy_init(void* __virt_addr, u32 __pcie_id)
{
}
static __inline__ void hi_pcie_set_apps_pm_xmt_turnoff(void* __virt_addr, u32 __pcie_id, u32 __pcie_value)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC44, 8, 1, __pcie_value);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xCA4, 8, 1, __pcie_value);
    }
}
static __inline__ void hi_pcie_set_app_ready_entr_l23(void* __virt_addr, u32 __pcie_id, u32 __pcie_value)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC44, 2, 1, __pcie_value);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xCA4, 2, 1, __pcie_value);
    }
}
static __inline__ void hi_pcie_set_apps_pm_xmt_pme(void* __virt_addr, u32 __pcie_id, u32 __pcie_value)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC44, 9, 1, __pcie_value);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xCA4, 9, 1, __pcie_value);
    }
}
static __inline__ u32 hi_pcie_get_radm_pm_to_ack_reg(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        return hi_pcie_get(__virt_addr + 0xE3C, 16, 1);
    }
    else
    {
        return hi_pcie_get(__virt_addr + 0xE64, 16, 1);
    }
}
static __inline__ void hi_pcie_inner_clk_enable(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC1C, 8, 1, 0);
        hi_pcie_set(__virt_addr + 0xC1C, 9, 1, 1);
        hi_pcie_set(__virt_addr + 0xC14, 13, 1, 0);
        hi_pcie_set(__virt_addr + 0xC14, 4, 1, 1);
        hi_pcie_set(__virt_addr + 0xC14, 8, 1, 0);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xC7C, 8, 1, 0);
        hi_pcie_set(__virt_addr + 0xC7C, 9, 1, 1);
        hi_pcie_set(__virt_addr + 0xC14, 15, 1, 0);
        hi_pcie_set(__virt_addr + 0xC14, 6, 1, 1);
        hi_pcie_set(__virt_addr + 0xC14, 10, 1, 0);
    }
}
static __inline__ void hi_pcie_output_clk_enable(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC14, 16, 1, 1);
        hi_pcie_set(__virt_addr + 0xC14, 17, 1, 1);
        hi_pcie_set(__virt_addr + 0xC14, 12, 1, 0);
        hi_pcie_set(__virt_addr + 0xC14, 5, 1, 1);
        hi_pcie_set(__virt_addr + 0xC14, 9, 1, 0);
        hi_pcie_set(__virt_addr + 0xC10, 30, 1, 1);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xC14, 18, 1, 1);
        hi_pcie_set(__virt_addr + 0xC14, 19, 1, 1);
        hi_pcie_set(__virt_addr + 0xC14, 14, 1, 0);
        hi_pcie_set(__virt_addr + 0xC14, 7, 1, 1);
        hi_pcie_set(__virt_addr + 0xC14, 11, 1, 0);
    }
}
static __inline__ void hi_pcie_l1ss_auto_gate_enable(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC14, 13, 1, 1);
        hi_pcie_set(__virt_addr + 0xC14, 12, 1, 1);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xC14, 15, 1, 1);
        hi_pcie_set(__virt_addr + 0xC14, 14, 1, 1);
    }
}
static __inline__ void hi_pcie_l1ss_auto_gate_disable(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC14, 13, 1, 0);
        hi_pcie_set(__virt_addr + 0xC14, 12, 1, 0);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xC14, 15, 1, 0);
        hi_pcie_set(__virt_addr + 0xC14, 14, 1, 0);
    }
}
static __inline__ void hi_pcie_app_clk_req_n(void* __virt_addr, u32 __pcie_id, u32 __request)
{
    if (0 == __pcie_id)
    {
        if (__request)
        {
            hi_pcie_set(__virt_addr + 0xC2C, 23, 1, 0);
            hi_pcie_set(__virt_addr + 0xC44, 3, 1, 1);
        }
        else
        {
            hi_pcie_set(__virt_addr + 0xC2C, 23, 1, 1);
            hi_pcie_set(__virt_addr + 0xC44, 3, 1, 0);
        }
    }
    else
    {
        if (__request)
        {
            hi_pcie_set(__virt_addr + 0xC8C, 23, 1, 0);
            hi_pcie_set(__virt_addr + 0xCA4, 3, 1, 1);
        }
        else
        {
            hi_pcie_set(__virt_addr + 0xC8C, 23, 1, 1);
            hi_pcie_set(__virt_addr + 0xCA4, 3, 1, 0);
        }
    }
}
static __inline__ void hi_pcie_trigger_int_msi(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC14, 0, 2, 3);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xC14, 2, 2, 3);
    }
}
static __inline__ void hi_pcie_clear_int_msi(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC14, 0, 2, 0);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xC14, 2, 2, 0);
    }
}
static __inline__ void hi_pcie_assert_perst(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC58, 0, 1, 0);
        hi_pcie_set(__virt_addr + 0xC58, 1, 1, 1);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xCB8, 0, 1, 0);
        hi_pcie_set(__virt_addr + 0xCB8, 1, 1, 1);
    }
}
static __inline__ void hi_pcie_deassert_perst(void* __virt_addr, u32 __pcie_id)
{
    if (0 == __pcie_id)
    {
        hi_pcie_set(__virt_addr + 0xC58, 0, 1, 1);
        hi_pcie_set(__virt_addr + 0xC58, 1, 1, 1);
    }
    else
    {
        hi_pcie_set(__virt_addr + 0xCB8, 0, 1, 1);
        hi_pcie_set(__virt_addr + 0xCB8, 1, 1, 1);
    }
}
#ifdef __cplusplus
}
#endif
#endif
