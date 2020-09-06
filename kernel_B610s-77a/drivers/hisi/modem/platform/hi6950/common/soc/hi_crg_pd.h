#ifndef __HI_PD_CRG_H__
#define __HI_PD_CRG_H__ 
#ifndef HI_SET_GET
#define HI_SET_GET(a0,a1,a2,a3,a4) 
#endif
#define HI_PD_CRG_CLKEN1_OFFSET (0x0)
#define HI_PD_CRG_CLKDIS1_OFFSET (0x4)
#define HI_PD_CRG_CLKSTAT1_OFFSET (0x8)
#define HI_PD_CRG_CLKCGSTAT1_OFFSET (0xC)
#define HI_PD_CRG_CLKEN2_OFFSET (0x10)
#define HI_PD_CRG_CLKDIS2_OFFSET (0x14)
#define HI_PD_CRG_CLKSTAT2_OFFSET (0x18)
#define HI_PD_CRG_CLKCGSTAT2_OFFSET (0x1C)
#define HI_PD_CRG_CLKEN3_OFFSET (0x20)
#define HI_PD_CRG_CLKDIS3_OFFSET (0x24)
#define HI_PD_CRG_CLKSTAT3_OFFSET (0x28)
#define HI_PD_CRG_CLKCGSTAT3_OFFSET (0x2C)
#define HI_PD_CRG_CLKEN4_OFFSET (0x30)
#define HI_PD_CRG_CLKDIS4_OFFSET (0x34)
#define HI_PD_CRG_CLKSTAT4_OFFSET (0x38)
#define HI_PD_CRG_CLKCGSTAT4_OFFSET (0x2C)
#define HI_PD_CRG_SRSTEN1_OFFSET (0x60)
#define HI_PD_CRG_SRSTDIS1_OFFSET (0x64)
#define HI_PD_CRG_SRSTSTAT1_OFFSET (0x68)
#define HI_PD_CRG_SRSTEN2_OFFSET (0x6C)
#define HI_PD_CRG_SRSTDIS2_OFFSET (0x70)
#define HI_PD_CRG_SRSTSTAT2_OFFSET (0x74)
#define HI_PD_CRG_SRSTEN3_OFFSET (0x78)
#define HI_PD_CRG_SRSTDIS3_OFFSET (0x7C)
#define HI_PD_CRG_SRSTSTAT3_OFFSET (0x80)
#define HI_PD_CRG_SRSTEN4_OFFSET (0x84)
#define HI_PD_CRG_SRSTDIS4_OFFSET (0x88)
#define HI_PD_CRG_SRSTSTAT4_OFFSET (0x8C)
#define HI_PD_CRG_CLKDIV1_OFFSET (0x100)
#define HI_PD_CRG_CLKDIV2_OFFSET (0x104)
#define HI_PD_CRG_CLKDIV3_OFFSET (0x108)
#define HI_PD_CRG_CLKDIV4_OFFSET (0x10C)
#define HI_PD_CRG_MMC_DLY_CTRL_OFFSET (0x114)
#define HI_PLLLOCK_CTRL_OFFSET (0x118)
#define HI_PD_CRG_SIO_CFG_OFFSET (0x11C)
#define HI_PD_CRG_CLK_SEL1_OFFSET (0x140)
#define HI_CRG_DFS1_CTRL1_OFFSET (0x200)
#define HI_CRG_DFS1_CTRL2_OFFSET (0x204)
#define HI_CRG_DFS1_CTRL3_OFFSET (0x208)
#define HI_CRG_DFS2_CTRL1_OFFSET (0x20C)
#define HI_CRG_DFS2_CTRL2_OFFSET (0x210)
#define HI_CRG_DFS2_CTRL3_OFFSET (0x214)
#define HI_CRG_DFS3_CTRL1_OFFSET (0x218)
#define HI_CRG_DFS3_CTRL2_OFFSET (0x21C)
#define HI_CRG_DFS3_CTRL3_OFFSET (0x220)
#define HI_CRG_DFS5_CTRL1_OFFSET (0x230)
#define HI_CRG_DFS5_CTRL2_OFFSET (0x234)
#define HI_CRG_DFS5_CTRL3_OFFSET (0x238)
#define HI_CRG_DFS6_CTRL1_OFFSET (0x23C)
#define HI_CRG_DFS6_CTRL2_OFFSET (0x240)
#define HI_CRG_DFS6_CTRL3_OFFSET (0x244)
#define HI_PD_CRG_CLKSW_STAT1_OFFSET (0x260)
#ifndef __ASSEMBLY__
typedef union
{
    struct
    {
        unsigned int reserved_8 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_5 : 1;
        unsigned int amon_soc_clk_en : 1;
        unsigned int reserved_4 : 1;
        unsigned int spe_clk_en : 1;
        unsigned int psam_clk_en : 1;
        unsigned int ipf_clk_en : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_2 : 1;
        unsigned int socp_clk_en : 1;
        unsigned int hkadcssi_clk_en : 1;
        unsigned int pmussi0_clk_en : 1;
        unsigned int pmussi1_clk_en : 1;
        unsigned int spi0_clk_en : 1;
        unsigned int rsracc_clk_en : 1;
        unsigned int nandc_clk_en : 1;
        unsigned int hsuart_clk_en : 1;
        unsigned int emi_clk_en : 1;
        unsigned int edmac_clk_en : 1;
        unsigned int ios_pd_clk_en : 1;
        unsigned int pwm1_clk_en : 1;
        unsigned int pwm0_clk_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int ipcm_clk_en : 1;
        unsigned int spi1_clk_en : 1;
        unsigned int i2c1_clk_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int i2c0_clk_en : 1;
        unsigned int gpio3_clk_en : 1;
        unsigned int rsa_clk_en : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKEN1_T;
typedef union
{
    struct
    {
        unsigned int reserved_9 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_6 : 1;
        unsigned int amon_soc_clk_dis : 1;
        unsigned int reserved_5 : 1;
        unsigned int spe_clk_dis : 1;
        unsigned int psam_clk_dis : 1;
        unsigned int ipf_clk_dis : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_3 : 1;
        unsigned int socp_clk_dis : 1;
        unsigned int hkadcssi_clk_dis : 1;
        unsigned int pmussi0_clk_dis : 1;
        unsigned int pmussi1_clk_dis : 1;
        unsigned int spi0_clk_dis : 1;
        unsigned int rsracc_clk_dis : 1;
        unsigned int nandc_clk_dis : 1;
        unsigned int hsuart_clk_dis : 1;
        unsigned int emi_clk_dis : 1;
        unsigned int edmac_clk_dis : 1;
        unsigned int reserved_2 : 1;
        unsigned int pwm1_clk_dis : 1;
        unsigned int pwm0_clk_dis : 1;
        unsigned int reserved_1 : 1;
        unsigned int ipcm_clk_dis : 1;
        unsigned int spi1_clk_dis : 1;
        unsigned int i2c1_clk_dis : 1;
        unsigned int reserved_0 : 1;
        unsigned int i2c0_clk_dis : 1;
        unsigned int gpio3_clk_dis : 1;
        unsigned int rsa_clk_dis : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKDIS1_T;
typedef union
{
    struct
    {
        unsigned int reserved_9 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_6 : 1;
        unsigned int amon_soc_clk_status : 1;
        unsigned int reserved_5 : 1;
        unsigned int spe_clk_status : 1;
        unsigned int psam_clk_status : 1;
        unsigned int ipf_clk_status : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_3 : 1;
        unsigned int socp_clk_status : 1;
        unsigned int hkadcssi_clk_status : 1;
        unsigned int pmussi0_clk_status : 1;
        unsigned int pmussi1_clk_status : 1;
        unsigned int spi0_clk_status : 1;
        unsigned int rsracc_clk_dis : 1;
        unsigned int nandc_clk_status : 1;
        unsigned int hsuart_clk_status : 1;
        unsigned int emi_clk_status : 1;
        unsigned int edmac_clk_status : 1;
        unsigned int reserved_2 : 1;
        unsigned int pwm1_clk_status : 1;
        unsigned int pwm0_clk_status : 1;
        unsigned int reserved_1 : 1;
        unsigned int ipcm_clk_status : 1;
        unsigned int spi1_clk_status : 1;
        unsigned int i2c1_clk_status : 1;
        unsigned int reserved_0 : 1;
        unsigned int i2c0_clk_status : 1;
        unsigned int gpio3_clk_status : 1;
        unsigned int rsa_clk_status : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKSTAT1_T;
typedef union
{
    struct
    {
        unsigned int reserved_9 : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_6 : 1;
        unsigned int amon_soc_clkcg_status : 1;
        unsigned int reserved_5 : 1;
        unsigned int spe_clkcg_status : 1;
        unsigned int psam_clkcg_status : 1;
        unsigned int ipf_clkcg_status : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_3 : 1;
        unsigned int socp_clkcg_status : 1;
        unsigned int hkadcssi_clkcg_status : 1;
        unsigned int pmussi0_clkcg_status : 1;
        unsigned int pmussi1_clkcg_status : 1;
        unsigned int spi0_clkcg_status : 1;
        unsigned int rsracc_clkcg_status : 1;
        unsigned int nandc_clkcg_status : 1;
        unsigned int hsuart_clkcg_status : 1;
        unsigned int emi_clkcg_status : 1;
        unsigned int edmac_clkcg_status : 1;
        unsigned int reserved_2 : 1;
        unsigned int pwm1_clkcg_status : 1;
        unsigned int pwm0_clkcg_status : 1;
        unsigned int reserved_1 : 1;
        unsigned int ipcm_clkcg_status : 1;
        unsigned int spi1_clkcg_status : 1;
        unsigned int i2c1_clkcg_status : 1;
        unsigned int reserved_0 : 1;
        unsigned int i2c0_clkcg_status : 1;
        unsigned int gpio3_clkcg_status : 1;
        unsigned int rsa_clkcg_status : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKCGSTAT1_T;
typedef union
{
    struct
    {
        unsigned int reserved_5 : 1;
        unsigned int uart2_clk_en : 1;
        unsigned int gpio2_clk_en : 1;
        unsigned int cs_dbg_clk_en : 1;
        unsigned int acpu_mbist_clk_en : 1;
        unsigned int reserved_4 : 1;
        unsigned int mddrc_cfg_clk_en : 1;
        unsigned int mddrc_clk_en : 1;
        unsigned int ddrphy_cfg_clk_en : 1;
        unsigned int ddrphy_ddr_clk_en : 1;
        unsigned int reserved_3 : 1;
        unsigned int cs_trace_clk_en : 1;
        unsigned int soc_peri_usb_clk_en : 1;
        unsigned int usbotg_clk_en : 1;
        unsigned int reserved_2 : 1;
        unsigned int pcie1_ctrl_clk_en : 2;
        unsigned int mmc0_clk_en : 1;
        unsigned int mmc1_clk_en : 1;
        unsigned int mmc0_phase_clk_en : 1;
        unsigned int mmc1_phase_clk_en : 1;
        unsigned int mmc0_refclk_en : 1;
        unsigned int mmc1_refclk_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int sdcc_clk_en : 1;
        unsigned int hpm_clk_en : 1;
        unsigned int sysctrl_pcie_clk_en : 1;
        unsigned int pcie0_ctrl_clk_en : 2;
        unsigned int pcie0_phy_clk_en : 1;
        unsigned int pcie1_phy_clk_en : 1;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKEN2_T;
typedef union
{
    struct
    {
        unsigned int reserved_5 : 1;
        unsigned int uart2_clk_dis : 1;
        unsigned int gpio2_clk_dis : 1;
        unsigned int cs_dbg_clk_dis : 1;
        unsigned int acpu_mbist_clk_dis : 1;
        unsigned int reserved_4 : 1;
        unsigned int mddrc_cfg_clk_dis : 1;
        unsigned int mddrc_clk_dis : 1;
        unsigned int ddrphy_clk_dis : 1;
        unsigned int ddrphy_ddr_clk_dis : 1;
        unsigned int reserved_3 : 1;
        unsigned int cs_trace_clk_dis : 1;
        unsigned int soc_usb_pd_clk_dis : 1;
        unsigned int usbotg_clk_dis : 1;
        unsigned int reserved_2 : 1;
        unsigned int pcie1_ctrl_clk_dis : 2;
        unsigned int mmc0_clk_dis : 1;
        unsigned int mmc1_clk_dis : 1;
        unsigned int mmc0_phase_clk_dis : 1;
        unsigned int mmc1_phase_clk_dis : 1;
        unsigned int mmc0_refclk_dis : 1;
        unsigned int mmc1_refclk_dis : 1;
        unsigned int reserved_1 : 1;
        unsigned int sdcc_clk_dis : 1;
        unsigned int hpm_clk_dis : 1;
        unsigned int sysctrl_pcie_clk_dis : 1;
        unsigned int pcie0_ctrl_clk_dis : 2;
        unsigned int pcie0_phy_clk_dis : 1;
        unsigned int pcie1_phy_clk_dis : 1;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKDIS2_T;
typedef union
{
    struct
    {
        unsigned int reserved_5 : 1;
        unsigned int uart2_clk_status : 1;
        unsigned int gpio2_clk_status : 1;
        unsigned int cs_dbg_clk_status : 1;
        unsigned int acpu_mbist_clk_status : 1;
        unsigned int reserved_4 : 1;
        unsigned int mddrc_cfg_clk_dis : 1;
        unsigned int mddrc_clk_status : 1;
        unsigned int ddrphy_clk_status : 1;
        unsigned int ddrphy_ddr_clk_status : 1;
        unsigned int axi_mem_clk_status : 1;
        unsigned int cs_trace_clk_status : 1;
        unsigned int soc_usb_pd_clk_status : 1;
        unsigned int usbotg_clk_status : 1;
        unsigned int reserved_3 : 1;
        unsigned int pcie1_ctrl_clk_status : 2;
        unsigned int mmc0_clk_status : 1;
        unsigned int mmc1_clk_status : 1;
        unsigned int mmc0_phase_clk_status : 1;
        unsigned int mmc1_phase_clk_status : 1;
        unsigned int mmc0_refclk_status : 1;
        unsigned int mmc1_refclk_status : 1;
        unsigned int reserved_2 : 1;
        unsigned int sdcc_clk_status : 1;
        unsigned int hpm_clk_status : 1;
        unsigned int reserved_1 : 1;
        unsigned int pcie0_ctrl_clk_status : 2;
        unsigned int pcie0_phy_clk_status : 1;
        unsigned int pcie1_phy_clk_status : 1;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKSTAT2_T;
typedef union
{
    struct
    {
        unsigned int uart3_clkcg_status : 1;
        unsigned int uart2_clkcg_status : 1;
        unsigned int gpio2_clkcg_status : 1;
        unsigned int cs_clkcg_status : 1;
        unsigned int acpu_mbist_clkcg_status : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_4 : 1;
        unsigned int mddrc_clkcg_status : 1;
        unsigned int ddrphy_clkcg_status : 1;
        unsigned int ddrphy_ddr_clkcg_status : 1;
        unsigned int axi_mem_clkcg_status : 1;
        unsigned int cs_trace_clkcg_status : 1;
        unsigned int reserved_3 : 1;
        unsigned int usbotg_clkcg_status : 1;
        unsigned int hdlc_clkcg_status : 1;
        unsigned int pcie1_ctrl_clkcg_status : 2;
        unsigned int mmc0_clkcg_status : 1;
        unsigned int mmc1_clkcg_status : 1;
        unsigned int mmc0_phase_clkcg_status : 1;
        unsigned int mmc1_phase_clkcg_status : 1;
        unsigned int mmc0_refclkcg_status : 1;
        unsigned int mmc1_refclkcg_status : 1;
        unsigned int reserved_2 : 1;
        unsigned int sdcc_clkcg_status : 1;
        unsigned int hpm_clkcg_status : 1;
        unsigned int reserved_1 : 1;
        unsigned int pcie0_ctrl_clkcg_status : 2;
        unsigned int pcie0_phy_clkcg_status : 1;
        unsigned int pcie1_phy_clkcg_status : 1;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKCGSTAT2_T;
typedef union
{
    struct
    {
        unsigned int reserved_3 : 1;
        unsigned int uart0_clk_en : 1;
        unsigned int reserved_2 : 2;
        unsigned int gpio4_clk_en : 1;
        unsigned int gpio5_clk_en : 1;
        unsigned int gpio6_clk_en : 1;
        unsigned int gpio7_clk_en : 1;
        unsigned int gpio8_clk_en : 1;
        unsigned int gpio9_clk_en : 1;
        unsigned int gpio10_clk_en : 1;
        unsigned int gpio11_clk_en : 1;
        unsigned int gpio12_clk_en : 1;
        unsigned int hifi_atb_clk_en : 1;
        unsigned int zsi_clk_en : 1;
        unsigned int reserved_1 : 8;
        unsigned int acpu_dbg_clk_en : 1;
        unsigned int acpu_atb_clk_en : 1;
        unsigned int acpu_acp_clk_en : 1;
        unsigned int appa9_clk_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int hifi_vic_clk_en : 1;
        unsigned int lte_sio_clk_en : 1;
        unsigned int hifi_dbg_clk_en : 1;
        unsigned int hifi_clk_en : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKEN3_T;
typedef union
{
    struct
    {
        unsigned int reserved_3 : 1;
        unsigned int uart0_clk_dis : 1;
        unsigned int reserved_2 : 2;
        unsigned int gpio4_clk_dis : 1;
        unsigned int gpio5_clk_dis : 1;
        unsigned int gpio6_clk_dis : 1;
        unsigned int gpio7_clk_dis : 1;
        unsigned int gpio8_clk_dis : 1;
        unsigned int gpio9_clk_dis : 1;
        unsigned int gpio10_clk_dis : 1;
        unsigned int gpio11_clk_dis : 1;
        unsigned int gpio12_clk_dis : 1;
        unsigned int hifi_atb_clk_dis : 1;
        unsigned int zsi_clk_dis : 1;
        unsigned int reserved_1 : 8;
        unsigned int acpu_dbg_clk_dis : 1;
        unsigned int acpu_atb_clk_dis : 1;
        unsigned int acpu_acp_clk_dis : 1;
        unsigned int appa9_clk_dis : 1;
        unsigned int reserved_0 : 1;
        unsigned int hifi_vic_clk_dis : 1;
        unsigned int lte_sio_clk_dis : 1;
        unsigned int hifi_dbg_clk_dis : 1;
        unsigned int hifi_clk_dis : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKDIS3_T;
typedef union
{
    struct
    {
        unsigned int reserved_3 : 1;
        unsigned int uart0_clk_status : 1;
        unsigned int reserved_2 : 2;
        unsigned int gpio4_clk_status : 1;
        unsigned int gpio5_clk_status : 1;
        unsigned int gpio6_clk_status : 1;
        unsigned int gpio7_clk_status : 1;
        unsigned int gpio8_clk_status : 1;
        unsigned int gpio9_clk_status : 1;
        unsigned int gpio10_clk_status : 1;
        unsigned int gpio11_clk_status : 1;
        unsigned int gpio12_clk_status : 1;
        unsigned int hifi_atb_clk_status : 1;
        unsigned int zsi_clk_status : 1;
        unsigned int reserved_1 : 8;
        unsigned int acpu_dbg_clk_status : 1;
        unsigned int acpu_atb_clk_status : 1;
        unsigned int acpu_acp_clk_status : 1;
        unsigned int appa9_clk_status : 1;
        unsigned int reserved_0 : 1;
        unsigned int hifi_vic_clk_status : 1;
        unsigned int lte_sio_clk_status : 1;
        unsigned int hifi_dbg_clk_status : 1;
        unsigned int hifi_clk_status : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKSTAT3_T;
typedef union
{
    struct
    {
        unsigned int reserved_3 : 1;
        unsigned int uart0_clkcg_status : 1;
        unsigned int reserved_2 : 2;
        unsigned int gpio4_clkcg_status : 1;
        unsigned int gpio5_clkcg_status : 1;
        unsigned int gpio6_clkcg_status : 1;
        unsigned int gpio7_clkcg_status : 1;
        unsigned int gpio8_clkcg_status : 1;
        unsigned int gpio9_clkcg_status : 1;
        unsigned int gpio10_clkcg_status : 1;
        unsigned int gpio11_clkcg_status : 1;
        unsigned int gpio12_clkcg_status : 1;
        unsigned int hifi_atb_clkcg_status : 1;
        unsigned int zsi_clkcg_status : 1;
        unsigned int reserved_1 : 8;
        unsigned int acpu_dbg_clkcg_status : 1;
        unsigned int acpu_atb_clkcg_status : 1;
        unsigned int acpu_acp_clkcg_status : 1;
        unsigned int appa9_clkcg_status : 1;
        unsigned int reserved_0 : 1;
        unsigned int hifi_vic_clkcg_status : 1;
        unsigned int lte_sio_clkcg_status : 1;
        unsigned int hifi_dbg_clkcg_status : 1;
        unsigned int hifi_clkcg_status : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKCGSTAT3_T;
typedef union
{
    struct
    {
        unsigned int a9_refclk_en : 4;
        unsigned int hifi_refclk_en : 4;
        unsigned int reserved_4 : 2;
        unsigned int hdlc_clk_en : 1;
        unsigned int reserved_3 : 4;
        unsigned int uart1_clk_en : 1;
        unsigned int reserved_2 : 1;
        unsigned int gmac_refclk_en : 1;
        unsigned int gmac_rx_clk_en : 1;
        unsigned int gmac_tx_clk_en : 1;
        unsigned int gmac_gtx_clk_en : 1;
        unsigned int gmac_switch_clk_en : 1;
        unsigned int gmac_aclk_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int dsp_refclk_en : 4;
        unsigned int reserved_0 : 4;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKEN4_T;
typedef union
{
    struct
    {
        unsigned int a9_refclk_dis : 4;
        unsigned int hifi_refclk_dis : 4;
        unsigned int reserved_3 : 2;
        unsigned int hdlc_clk_dis : 1;
        unsigned int reserved_2 : 4;
        unsigned int uart1_clk_dis : 1;
        unsigned int reserved_1 : 2;
        unsigned int gmac_rx_clk_dis : 1;
        unsigned int gmac_tx_clk_dis : 1;
        unsigned int gmac_gtx_clk_dis : 1;
        unsigned int gmac_switch_clk_dis : 1;
        unsigned int gmac_aclk_dis : 1;
        unsigned int dsp_refclk_dis : 5;
        unsigned int reserved_0 : 4;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKDIS4_T;
typedef union
{
    struct
    {
        unsigned int a9_refclk_status : 4;
        unsigned int hifi_refclk_status : 4;
        unsigned int reserved_3 : 2;
        unsigned int hdlc_clk_status : 1;
        unsigned int reserved_2 : 4;
        unsigned int uart1_clk_status : 1;
        unsigned int reserved_1 : 2;
        unsigned int gmac_rx_clk_status : 1;
        unsigned int gmac_tx_clk_status : 1;
        unsigned int gmac_gtx_clk_status : 1;
        unsigned int gmac_switch_clk_status : 1;
        unsigned int gmac_aclk_status : 1;
        unsigned int dsp_refclk_status : 5;
        unsigned int reserved_0 : 4;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKSTAT4_T;
typedef union
{
    struct
    {
        unsigned int reserved_8 : 4;
        unsigned int reserved_7 : 4;
        unsigned int reserved_6 : 2;
        unsigned int hdlc_clkcg_status : 1;
        unsigned int reserved_5 : 4;
        unsigned int uart1_clkcg_status : 1;
        unsigned int reserved_4 : 2;
        unsigned int gmac_rx_clkcg_status : 1;
        unsigned int gmac_tx_clkcg_status : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_2 : 1;
        unsigned int gmac_aclk_status : 1;
        unsigned int reserved_1 : 5;
        unsigned int reserved_0 : 4;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKCGSTAT4_T;
typedef union
{
    struct
    {
        unsigned int appa9_core_srst_en : 1;
        unsigned int appa9_dbg_srst_en : 1;
        unsigned int appa9_peri_srst_en : 1;
        unsigned int appa9_scu_srst_en : 1;
        unsigned int appa9_ptm_srst_en : 1;
        unsigned int appa9_wd_srst_en : 1;
        unsigned int appa9_l2c_srst_en : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_12 : 4;
        unsigned int hpm_srst_en : 1;
        unsigned int reserved_11 : 1;
        unsigned int appa9_srst_en : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_9 : 1;
        unsigned int mddrc_srst_en : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_6 : 1;
        unsigned int hdlc_srst_en : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_4 : 1;
        unsigned int edmac_srst_en : 1;
        unsigned int emi_srst_en : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int amon_soc_srst_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int rsa_srst_en : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_SRSTEN1_T;
typedef union
{
    struct
    {
        unsigned int appa9_core_srst_dis : 1;
        unsigned int appa9_dbg_srst_dis : 1;
        unsigned int appa9_peri_srst_dis : 1;
        unsigned int appa9_scu_srst_dis : 1;
        unsigned int appa9_ptm_srst_dis : 1;
        unsigned int appa9_wd_srst_dis : 1;
        unsigned int appa9_l2c_srst_dis : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_12 : 4;
        unsigned int hpm_srst_dis : 1;
        unsigned int reserved_11 : 1;
        unsigned int appa9_srst_dis : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_9 : 1;
        unsigned int mddrc_srst_dis : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_6 : 1;
        unsigned int hdlc_srst_dis : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_4 : 1;
        unsigned int edmac_srst_dis : 1;
        unsigned int emi_srst_dis : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int amon_soc_srst_dis : 1;
        unsigned int reserved_0 : 1;
        unsigned int rsa_srst_dis : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_SRSTDIS1_T;
typedef union
{
    struct
    {
        unsigned int appa9_core_srst_status : 1;
        unsigned int appa9_dbg_srst_status : 1;
        unsigned int appa9_peri_srst_status : 1;
        unsigned int appa9_scu_srst_status : 1;
        unsigned int appa9_ptm_srst_status : 1;
        unsigned int appa9_wd_srst_status : 1;
        unsigned int appa9_l2c_srst_status : 1;
        unsigned int reserved_13 : 1;
        unsigned int reserved_12 : 4;
        unsigned int hpm_srst_status : 1;
        unsigned int reserved_11 : 1;
        unsigned int appa9_srst_status : 1;
        unsigned int reserved_10 : 1;
        unsigned int reserved_9 : 1;
        unsigned int mddrc_srst_status : 1;
        unsigned int reserved_8 : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_6 : 1;
        unsigned int hdlc_srst_status : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_4 : 1;
        unsigned int edmac_srst_status : 1;
        unsigned int emi_srst_status : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int amon_soc_srst_status : 1;
        unsigned int reserved_0 : 1;
        unsigned int rsa_srst_status : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_SRSTSTAT1_T;
typedef union
{
    struct
    {
        unsigned int reserved_10 : 5;
        unsigned int reserved_9 : 1;
        unsigned int mmc0_srst_en : 1;
        unsigned int mmc1_srst_en : 1;
        unsigned int sdcc_srst_en : 1;
        unsigned int hsuart_srst_en : 1;
        unsigned int sio_srst_en : 1;
        unsigned int gmac_srst_en : 1;
        unsigned int pcie1_ctrl_srst_en : 1;
        unsigned int pcie1_phy_srst_en : 1;
        unsigned int pcie0_ctrl_srst_en : 1;
        unsigned int pcie0_phy_srst_en : 1;
        unsigned int reserved_8 : 2;
        unsigned int hifi_core_srst_en : 1;
        unsigned int hifi_pd_srst_en : 1;
        unsigned int hifi_dbg_srst_en : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int cs_srst_en : 1;
        unsigned int reserved_0 : 3;
    } bits;
    unsigned int u32;
}HI_PD_CRG_SRSTEN2_T;
typedef union
{
    struct
    {
        unsigned int reserved_11 : 5;
        unsigned int reserved_10 : 1;
        unsigned int mmc0_srst_dis : 1;
        unsigned int mmc1_srst_dis : 1;
        unsigned int sdcc_srst_dis : 1;
        unsigned int hsuart_srst_dis : 1;
        unsigned int sio_srst_dis : 1;
        unsigned int gmac_srst_dis : 1;
        unsigned int pcie1_ctrl_srst_dis : 1;
        unsigned int pcie1_phy_srst_dis : 1;
        unsigned int pcie_ctrl_srst_dis : 1;
        unsigned int pcie_phy_srst_dis : 1;
        unsigned int reserved_9 : 1;
        unsigned int reserved_8 : 1;
        unsigned int hifi_core_srst_dis : 1;
        unsigned int hifi_pd_srst_dis : 1;
        unsigned int hifi_dbg_srst_dis : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int cs_srst_dis : 1;
        unsigned int reserved_0 : 3;
    } bits;
    unsigned int u32;
}HI_PD_CRG_SRSTDIS2_T;
typedef union
{
    struct
    {
        unsigned int reserved_10 : 5;
        unsigned int reserved_9 : 1;
        unsigned int mmc0_srst_status : 1;
        unsigned int mmc1_srst_status : 1;
        unsigned int sdcc_srst_status : 1;
        unsigned int hsuart_srst_status : 1;
        unsigned int sio_srst_status : 1;
        unsigned int gmac_srst_status : 1;
        unsigned int pcie1_ctrl_srst_status : 1;
        unsigned int pcie1_phy_srst_status : 1;
        unsigned int pcie0_ctrl_srst_status : 1;
        unsigned int pcie0_phy_srst_status : 1;
        unsigned int reserved_8 : 2;
        unsigned int hificore_srst_status : 1;
        unsigned int hifi_pd_srst_status : 1;
        unsigned int hifi_dbg_srst_status : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_6 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int cs_srst_status : 1;
        unsigned int reserved_0 : 3;
    } bits;
    unsigned int u32;
}HI_PD_CRG_SRSTSTAT2_T;
typedef union
{
    struct
    {
        unsigned int gpio2_srst_en : 1;
        unsigned int gpio3_srst_en : 1;
        unsigned int gpio4_srst_en : 1;
        unsigned int gpio5_srst_en : 1;
        unsigned int gpio6_srst_en : 1;
        unsigned int gpio7_srst_en : 1;
        unsigned int gpio8_srst_en : 1;
        unsigned int gpio9_srst_en : 1;
        unsigned int gpio10_srst_en : 1;
        unsigned int gpio11_srst_en : 1;
        unsigned int gpio12_srst_en : 1;
        unsigned int i2c0_srst_en : 1;
        unsigned int i2c1_srst_en : 1;
        unsigned int spi0_srst_en : 1;
        unsigned int spi1_srst_en : 1;
        unsigned int uart0_srst_en : 1;
        unsigned int uart1_srst_en : 1;
        unsigned int uart2_srst_en : 1;
        unsigned int hkadcssi_srst_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int pmussi0_srst_en : 1;
        unsigned int pwm1_srst_en : 1;
        unsigned int pwm0_srst_en : 1;
        unsigned int rsracc_srst_en : 1;
        unsigned int psam_srst_en : 1;
        unsigned int ipf_srst_en : 1;
        unsigned int socp_srst_en : 1;
        unsigned int spe_srst_en : 1;
        unsigned int nandc_srst_en : 1;
        unsigned int reserved_0 : 3;
    } bits;
    unsigned int u32;
}HI_PD_CRG_SRSTEN3_T;
typedef union
{
    struct
    {
        unsigned int gpio2_srst_dis : 1;
        unsigned int gpio3_srst_dis : 1;
        unsigned int gpio4_srst_dis : 1;
        unsigned int gpio5_srst_dis : 1;
        unsigned int gpio6_srst_dis : 1;
        unsigned int gpio7_srst_dis : 1;
        unsigned int gpio8_srst_dis : 1;
        unsigned int gpio9_srst_dis : 1;
        unsigned int gpio10_srst_dis : 1;
        unsigned int gpio11_srst_dis : 1;
        unsigned int gpio12_srst_dis : 1;
        unsigned int i2c0_srst_dis : 1;
        unsigned int i2c1_srst_dis : 1;
        unsigned int spi0_srst_dis : 1;
        unsigned int spi1_srst_dis : 1;
        unsigned int uart0_srst_dis : 1;
        unsigned int uart1_srst_dis : 1;
        unsigned int uart2_srst_dis : 1;
        unsigned int hkadcssi_srst_dis : 1;
        unsigned int reserved_1 : 1;
        unsigned int pmussi0_srst_dis : 1;
        unsigned int pwm1_srst_dis : 1;
        unsigned int pwm0_srst_dis : 1;
        unsigned int rsracc_srst_dis : 1;
        unsigned int psam_srst_dis : 1;
        unsigned int ipf_srst_dis : 1;
        unsigned int socp_srst_dis : 1;
        unsigned int spe_srst_dis : 1;
        unsigned int nandc_srst_dis : 1;
        unsigned int reserved_0 : 3;
    } bits;
    unsigned int u32;
}HI_PD_CRG_SRSTDIS3_T;
typedef union
{
    struct
    {
        unsigned int gpio2_srst_status : 1;
        unsigned int gpio3_srst_status : 1;
        unsigned int gpio4_srst_status : 1;
        unsigned int gpio5_srst_status : 1;
        unsigned int gpio6_srst_status : 1;
        unsigned int gpio7_srst_status : 1;
        unsigned int gpio8_srst_status : 1;
        unsigned int gpio9_srst_status : 1;
        unsigned int gpio10_srst_status : 1;
        unsigned int gpio11_srst_status : 1;
        unsigned int gpio12_srst_status : 1;
        unsigned int i2c0_srst_status : 1;
        unsigned int i2c1_srst_status : 1;
        unsigned int spi0_srst_status : 1;
        unsigned int spi1_srst_status : 1;
        unsigned int uart0_srst_status : 1;
        unsigned int uart1_srst_status : 1;
        unsigned int uart2_srst_status : 1;
        unsigned int hkadcssi_srst_status : 1;
        unsigned int reserved_1 : 1;
        unsigned int pmussi0_srst_status : 1;
        unsigned int pwm1_srst_status : 1;
        unsigned int pwm0_srst_status : 1;
        unsigned int rsracc_srst_status : 1;
        unsigned int psam_srst_status : 1;
        unsigned int ipf_srst_status : 1;
        unsigned int socp_srst_status : 1;
        unsigned int spe_srst_status : 1;
        unsigned int nandc_srst_status : 1;
        unsigned int reserved_0 : 3;
    } bits;
    unsigned int u32;
}HI_PD_CRG_SRSTSTAT3_T;
typedef union
{
    struct
    {
        unsigned int reserved : 32;
    } bits;
    unsigned int u32;
}HI_PD_CRG_SRSTEN4_T;
typedef union
{
    struct
    {
        unsigned int reserved : 32;
    } bits;
    unsigned int u32;
}HI_PD_CRG_SRSTDIS4_T;
typedef union
{
    struct
    {
        unsigned int reserved : 32;
    } bits;
    unsigned int u32;
}HI_PD_CRG_SRSTSTAT4_T;
typedef union
{
    struct
    {
        unsigned int pcm_bclk_div : 16;
        unsigned int pcm_sync_div : 12;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKDIV1_T;
typedef union
{
    struct
    {
        unsigned int acpu_clk_div : 4;
        unsigned int ccpu_clk_div : 4;
        unsigned int slow_clk_div : 4;
        unsigned int fast_clk_div : 4;
        unsigned int reserved_3 : 8;
        unsigned int reserved_2 : 1;
        unsigned int reserved_1 : 5;
        unsigned int apb_freqmode : 1;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKDIV2_T;
typedef union
{
    struct
    {
        unsigned int mmc0_cclk_div : 5;
        unsigned int mmc0_refclk_sel : 2;
        unsigned int reserved_3 : 1;
        unsigned int mmc0_phase_clk_div : 3;
        unsigned int reserved_2 : 1;
        unsigned int mmc1_cclk_div : 5;
        unsigned int mmc1_refclk_sel : 2;
        unsigned int reserved_1 : 1;
        unsigned int mmc1_phase_clk_div : 3;
        unsigned int reserved_0 : 1;
        unsigned int hpm_clk_div : 4;
        unsigned int spe_clk_div : 4;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKDIV3_T;
typedef union
{
    struct
    {
        unsigned int hifi_clk_div_num : 4;
        unsigned int hifi_freqmode : 2;
        unsigned int reserved_1 : 2;
        unsigned int hifi_refclk_sel : 4;
        unsigned int lp_hifi_clk_div_num : 4;
        unsigned int lp_hifi_div_num_enable : 1;
        unsigned int reserved_0 : 15;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKDIV4_T;
typedef union
{
    struct
    {
        unsigned int mmc0_drv_sel : 7;
        unsigned int mmc0_clk_bypass : 1;
        unsigned int mmc0_sample_sel : 7;
        unsigned int mmc0_sample_ctrl : 1;
        unsigned int mmc1_drv_sel : 7;
        unsigned int mmc1_clk_bypass : 1;
        unsigned int mmc1_sample_sel : 7;
        unsigned int mmc1_sample_ctrl : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_MMC_DLY_CTRL_T;
typedef union
{
    struct
    {
        unsigned int reserved : 32;
    } bits;
    unsigned int u32;
}HI_PLLLOCK_CTRL_T;
typedef union
{
    struct
    {
        unsigned int sio_sync_cfg_cnt : 12;
        unsigned int reserved : 20;
    } bits;
    unsigned int u32;
}HI_PD_CRG_SIO_CFG_T;
typedef union
{
    struct
    {
        unsigned int dsp_refclk_sw_req : 4;
        unsigned int reserved_8 : 2;
        unsigned int sio_clk_sel : 1;
        unsigned int reserved_7 : 1;
        unsigned int a9_pllclk_sw_req : 4;
        unsigned int reserved_6 : 1;
        unsigned int reserved_5 : 1;
        unsigned int reserved_4 : 7;
        unsigned int gmac_rx_clk_sel : 1;
        unsigned int gmac_tx_clk_sel : 1;
        unsigned int gmac_gtx_clk_sel : 1;
        unsigned int reserved_3 : 4;
        unsigned int reserved_2 : 1;
        unsigned int reserved_1 : 1;
        unsigned int sio_refclk_sel : 1;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLK_SEL1_T;
typedef union
{
    struct
    {
        unsigned int dfs_frac : 24;
        unsigned int reserved : 8;
    } bits;
    unsigned int u32;
}HI_CRG_DFS1_CTRL1_T;
typedef union
{
    struct
    {
        unsigned int dfs_intprog : 12;
        unsigned int dfs_refdiv : 6;
        unsigned int reserved_2 : 2;
        unsigned int dfs_postdiv2 : 3;
        unsigned int reserved_1 : 1;
        unsigned int dfs_postdiv1 : 3;
        unsigned int reserved_0 : 5;
    } bits;
    unsigned int u32;
}HI_CRG_DFS1_CTRL2_T;
typedef union
{
    struct
    {
        unsigned int pd : 1;
        unsigned int reserved_1 : 1;
        unsigned int dsmpd : 1;
        unsigned int phase4_pd : 1;
        unsigned int postdivpd : 1;
        unsigned int vcopd : 1;
        unsigned int dacpd : 1;
        unsigned int bypass : 1;
        unsigned int pll_ctrl : 1;
        unsigned int postfout_clk_en : 1;
        unsigned int fout1_clk_en : 1;
        unsigned int fout2_clk_en : 1;
        unsigned int fout3_clk_en : 1;
        unsigned int fout4_clk_en : 1;
        unsigned int reserved_0 : 17;
        unsigned int lock : 1;
    } bits;
    unsigned int u32;
}HI_CRG_DFS1_CTRL3_T;
typedef union
{
    struct
    {
        unsigned int dfs_frac : 24;
        unsigned int reserved : 8;
    } bits;
    unsigned int u32;
}HI_CRG_DFS2_CTRL1_T;
typedef union
{
    struct
    {
        unsigned int dfs_intprog : 12;
        unsigned int dfs_refdiv : 6;
        unsigned int reserved_2 : 2;
        unsigned int dfs_postdiv2 : 3;
        unsigned int reserved_1 : 1;
        unsigned int dfs_postdiv1 : 3;
        unsigned int reserved_0 : 5;
    } bits;
    unsigned int u32;
}HI_CRG_DFS2_CTRL2_T;
typedef union
{
    struct
    {
        unsigned int pd : 1;
        unsigned int reserved_2 : 1;
        unsigned int dsmpd : 1;
        unsigned int phase4_pd : 1;
        unsigned int postdivpd : 1;
        unsigned int vcopd : 1;
        unsigned int dacpd : 1;
        unsigned int bypass : 1;
        unsigned int reserved_1 : 1;
        unsigned int postfout_clk_en : 1;
        unsigned int fout1_clk_en : 1;
        unsigned int fout2_clk_en : 1;
        unsigned int fout3_clk_en : 1;
        unsigned int fout4_clk_en : 1;
        unsigned int reserved_0 : 17;
        unsigned int lock : 1;
    } bits;
    unsigned int u32;
}HI_CRG_DFS2_CTRL3_T;
typedef union
{
    struct
    {
        unsigned int dfs_frac : 24;
        unsigned int reserved : 8;
    } bits;
    unsigned int u32;
}HI_CRG_DFS3_CTRL1_T;
typedef union
{
    struct
    {
        unsigned int dfs_intprog : 12;
        unsigned int dfs_refdiv : 6;
        unsigned int reserved_2 : 2;
        unsigned int dfs_postdiv2 : 3;
        unsigned int reserved_1 : 1;
        unsigned int dfs_postdiv1 : 3;
        unsigned int reserved_0 : 5;
    } bits;
    unsigned int u32;
}HI_CRG_DFS3_CTRL2_T;
typedef union
{
    struct
    {
        unsigned int pd : 1;
        unsigned int reserved_2 : 1;
        unsigned int dsmpd : 1;
        unsigned int phase4_pd : 1;
        unsigned int postdivpd : 1;
        unsigned int vcopd : 1;
        unsigned int dacpd : 1;
        unsigned int bypass : 1;
        unsigned int reserved_1 : 1;
        unsigned int postfout_clk_en : 1;
        unsigned int fout1_clk_en : 1;
        unsigned int fout2_clk_en : 1;
        unsigned int fout3_clk_en : 1;
        unsigned int fout4_clk_en : 1;
        unsigned int reserved_0 : 17;
        unsigned int lock : 1;
    } bits;
    unsigned int u32;
}HI_CRG_DFS3_CTRL3_T;
typedef union
{
    struct
    {
        unsigned int dfs_frac : 24;
        unsigned int reserved : 8;
    } bits;
    unsigned int u32;
}HI_CRG_DFS5_CTRL1_T;
typedef union
{
    struct
    {
        unsigned int dfs_intprog : 12;
        unsigned int dfs_refdiv : 6;
        unsigned int reserved_2 : 2;
        unsigned int dfs_postdiv2 : 3;
        unsigned int reserved_1 : 1;
        unsigned int dfs_postdiv1 : 3;
        unsigned int reserved_0 : 5;
    } bits;
    unsigned int u32;
}HI_CRG_DFS5_CTRL2_T;
typedef union
{
    struct
    {
        unsigned int pd : 1;
        unsigned int reserved_1 : 1;
        unsigned int dsmpd : 1;
        unsigned int phase4_pd : 1;
        unsigned int postdivpd : 1;
        unsigned int vcopd : 1;
        unsigned int dacpd : 1;
        unsigned int bypass : 1;
        unsigned int pll_ctrl : 1;
        unsigned int postfout_clk_en : 1;
        unsigned int fout1_clk_en : 1;
        unsigned int fout2_clk_en : 1;
        unsigned int fout3_clk_en : 1;
        unsigned int fout4_clk_en : 1;
        unsigned int reserved_0 : 17;
        unsigned int lock : 1;
    } bits;
    unsigned int u32;
}HI_CRG_DFS5_CTRL3_T;
typedef union
{
    struct
    {
        unsigned int dfs_frac : 24;
        unsigned int reserved : 8;
    } bits;
    unsigned int u32;
}HI_CRG_DFS6_CTRL1_T;
typedef union
{
    struct
    {
        unsigned int dfs_intprog : 12;
        unsigned int dfs_refdiv : 6;
        unsigned int reserved_2 : 2;
        unsigned int dfs_postdiv2 : 3;
        unsigned int reserved_1 : 1;
        unsigned int dfs_postdiv1 : 3;
        unsigned int reserved_0 : 5;
    } bits;
    unsigned int u32;
}HI_CRG_DFS6_CTRL2_T;
typedef union
{
    struct
    {
        unsigned int pd : 1;
        unsigned int reserved_2 : 1;
        unsigned int dsmpd : 1;
        unsigned int phase4_pd : 1;
        unsigned int postdivpd : 1;
        unsigned int vcopd : 1;
        unsigned int dacpd : 1;
        unsigned int bypass : 1;
        unsigned int reserved_1 : 1;
        unsigned int postfout_clk_en : 1;
        unsigned int fout1_clk_en : 1;
        unsigned int fout2_clk_en : 1;
        unsigned int fout3_clk_en : 1;
        unsigned int fout4_clk_en : 1;
        unsigned int reserved_0 : 17;
        unsigned int lock : 1;
    } bits;
    unsigned int u32;
}HI_CRG_DFS6_CTRL3_T;
typedef union
{
    struct
    {
        unsigned int reserved_4 : 2;
        unsigned int reserved_3 : 2;
        unsigned int reserved_2 : 2;
        unsigned int reserved_1 : 2;
        unsigned int hifi_refclk_sw_ack : 4;
        unsigned int dsp_refclk_sw_ack : 4;
        unsigned int a9_pllclk_sw_ack : 4;
        unsigned int reserved_0 : 12;
    } bits;
    unsigned int u32;
}HI_PD_CRG_CLKSW_STAT1_T;
HI_SET_GET(hi_pd_crg_clken1_reserved_8,reserved_8,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_reserved_7,reserved_7,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_reserved_6,reserved_6,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_reserved_5,reserved_5,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_amon_soc_clk_en,amon_soc_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_reserved_4,reserved_4,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_spe_clk_en,spe_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_psam_clk_en,psam_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_ipf_clk_en,ipf_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_reserved_3,reserved_3,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_reserved_2,reserved_2,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_socp_clk_en,socp_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_hkadcssi_clk_en,hkadcssi_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_pmussi0_clk_en,pmussi0_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_pmussi1_clk_en,pmussi1_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_spi0_clk_en,spi0_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_rsracc_clk_en,rsracc_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_nandc_clk_en,nandc_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_hsuart_clk_en,hsuart_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_emi_clk_en,emi_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_edmac_clk_en,edmac_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_ios_pd_clk_en,ios_pd_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_pwm1_clk_en,pwm1_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_pwm0_clk_en,pwm0_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_reserved_1,reserved_1,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_ipcm_clk_en,ipcm_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_spi1_clk_en,spi1_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_i2c1_clk_en,i2c1_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_reserved_0,reserved_0,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_i2c0_clk_en,i2c0_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_gpio3_clk_en,gpio3_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clken1_rsa_clk_en,rsa_clk_en,HI_PD_CRG_CLKEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_reserved_9,reserved_9,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_reserved_8,reserved_8,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_reserved_7,reserved_7,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_reserved_6,reserved_6,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_amon_soc_clk_dis,amon_soc_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_reserved_5,reserved_5,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_spe_clk_dis,spe_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_psam_clk_dis,psam_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_ipf_clk_dis,ipf_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_reserved_4,reserved_4,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_reserved_3,reserved_3,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_socp_clk_dis,socp_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_hkadcssi_clk_dis,hkadcssi_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_pmussi0_clk_dis,pmussi0_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_pmussi1_clk_dis,pmussi1_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_spi0_clk_dis,spi0_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_rsracc_clk_dis,rsracc_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_nandc_clk_dis,nandc_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_hsuart_clk_dis,hsuart_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_emi_clk_dis,emi_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_edmac_clk_dis,edmac_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_reserved_2,reserved_2,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_pwm1_clk_dis,pwm1_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_pwm0_clk_dis,pwm0_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_reserved_1,reserved_1,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_ipcm_clk_dis,ipcm_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_spi1_clk_dis,spi1_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_i2c1_clk_dis,i2c1_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_reserved_0,reserved_0,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_i2c0_clk_dis,i2c0_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_gpio3_clk_dis,gpio3_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis1_rsa_clk_dis,rsa_clk_dis,HI_PD_CRG_CLKDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_reserved_9,reserved_9,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_reserved_8,reserved_8,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_reserved_7,reserved_7,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_reserved_6,reserved_6,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_amon_soc_clk_status,amon_soc_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_reserved_5,reserved_5,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_spe_clk_status,spe_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_psam_clk_status,psam_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_ipf_clk_status,ipf_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_reserved_4,reserved_4,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_reserved_3,reserved_3,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_socp_clk_status,socp_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_hkadcssi_clk_status,hkadcssi_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_pmussi0_clk_status,pmussi0_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_pmussi1_clk_status,pmussi1_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_spi0_clk_status,spi0_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_rsracc_clk_dis,rsracc_clk_dis,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_nandc_clk_status,nandc_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_hsuart_clk_status,hsuart_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_emi_clk_status,emi_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_edmac_clk_status,edmac_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_reserved_2,reserved_2,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_pwm1_clk_status,pwm1_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_pwm0_clk_status,pwm0_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_reserved_1,reserved_1,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_ipcm_clk_status,ipcm_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_spi1_clk_status,spi1_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_i2c1_clk_status,i2c1_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_reserved_0,reserved_0,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_i2c0_clk_status,i2c0_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_gpio3_clk_status,gpio3_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat1_rsa_clk_status,rsa_clk_status,HI_PD_CRG_CLKSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_reserved_9,reserved_9,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_reserved_8,reserved_8,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_reserved_7,reserved_7,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_reserved_6,reserved_6,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_amon_soc_clkcg_status,amon_soc_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_reserved_5,reserved_5,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_spe_clkcg_status,spe_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_psam_clkcg_status,psam_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_ipf_clkcg_status,ipf_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_reserved_4,reserved_4,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_reserved_3,reserved_3,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_socp_clkcg_status,socp_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_hkadcssi_clkcg_status,hkadcssi_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_pmussi0_clkcg_status,pmussi0_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_pmussi1_clkcg_status,pmussi1_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_spi0_clkcg_status,spi0_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_rsracc_clkcg_status,rsracc_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_nandc_clkcg_status,nandc_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_hsuart_clkcg_status,hsuart_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_emi_clkcg_status,emi_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_edmac_clkcg_status,edmac_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_reserved_2,reserved_2,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_pwm1_clkcg_status,pwm1_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_pwm0_clkcg_status,pwm0_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_reserved_1,reserved_1,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_ipcm_clkcg_status,ipcm_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_spi1_clkcg_status,spi1_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_i2c1_clkcg_status,i2c1_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_reserved_0,reserved_0,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_i2c0_clkcg_status,i2c0_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_gpio3_clkcg_status,gpio3_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat1_rsa_clkcg_status,rsa_clkcg_status,HI_PD_CRG_CLKCGSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_reserved_5,reserved_5,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_uart2_clk_en,uart2_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_gpio2_clk_en,gpio2_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_cs_dbg_clk_en,cs_dbg_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_acpu_mbist_clk_en,acpu_mbist_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_reserved_4,reserved_4,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_mddrc_cfg_clk_en,mddrc_cfg_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_mddrc_clk_en,mddrc_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_ddrphy_cfg_clk_en,ddrphy_cfg_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_ddrphy_ddr_clk_en,ddrphy_ddr_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_reserved_3,reserved_3,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_cs_trace_clk_en,cs_trace_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_soc_peri_usb_clk_en,soc_peri_usb_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_usbotg_clk_en,usbotg_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_reserved_2,reserved_2,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_pcie1_ctrl_clk_en,pcie1_ctrl_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_mmc0_clk_en,mmc0_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_mmc1_clk_en,mmc1_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_mmc0_phase_clk_en,mmc0_phase_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_mmc1_phase_clk_en,mmc1_phase_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_mmc0_refclk_en,mmc0_refclk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_mmc1_refclk_en,mmc1_refclk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_reserved_1,reserved_1,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_sdcc_clk_en,sdcc_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_hpm_clk_en,hpm_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_sysctrl_pcie_clk_en,sysctrl_pcie_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_pcie0_ctrl_clk_en,pcie0_ctrl_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_pcie0_phy_clk_en,pcie0_phy_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_pcie1_phy_clk_en,pcie1_phy_clk_en,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clken2_reserved_0,reserved_0,HI_PD_CRG_CLKEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_reserved_5,reserved_5,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_uart2_clk_dis,uart2_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_gpio2_clk_dis,gpio2_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_cs_dbg_clk_dis,cs_dbg_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_acpu_mbist_clk_dis,acpu_mbist_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_reserved_4,reserved_4,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_mddrc_cfg_clk_dis,mddrc_cfg_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_mddrc_clk_dis,mddrc_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_ddrphy_clk_dis,ddrphy_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_ddrphy_ddr_clk_dis,ddrphy_ddr_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_reserved_3,reserved_3,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_cs_trace_clk_dis,cs_trace_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_soc_usb_pd_clk_dis,soc_usb_pd_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_usbotg_clk_dis,usbotg_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_reserved_2,reserved_2,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_pcie1_ctrl_clk_dis,pcie1_ctrl_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_mmc0_clk_dis,mmc0_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_mmc1_clk_dis,mmc1_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_mmc0_phase_clk_dis,mmc0_phase_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_mmc1_phase_clk_dis,mmc1_phase_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_mmc0_refclk_dis,mmc0_refclk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_mmc1_refclk_dis,mmc1_refclk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_reserved_1,reserved_1,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_sdcc_clk_dis,sdcc_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_hpm_clk_dis,hpm_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_sysctrl_pcie_clk_dis,sysctrl_pcie_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_pcie0_ctrl_clk_dis,pcie0_ctrl_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_pcie0_phy_clk_dis,pcie0_phy_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_pcie1_phy_clk_dis,pcie1_phy_clk_dis,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis2_reserved_0,reserved_0,HI_PD_CRG_CLKDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_reserved_5,reserved_5,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_uart2_clk_status,uart2_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_gpio2_clk_status,gpio2_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_cs_dbg_clk_status,cs_dbg_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_acpu_mbist_clk_status,acpu_mbist_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_reserved_4,reserved_4,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_mddrc_cfg_clk_dis,mddrc_cfg_clk_dis,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_mddrc_clk_status,mddrc_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_ddrphy_clk_status,ddrphy_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_ddrphy_ddr_clk_status,ddrphy_ddr_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_axi_mem_clk_status,axi_mem_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_cs_trace_clk_status,cs_trace_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_soc_usb_pd_clk_status,soc_usb_pd_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_usbotg_clk_status,usbotg_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_reserved_3,reserved_3,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_pcie1_ctrl_clk_status,pcie1_ctrl_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_mmc0_clk_status,mmc0_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_mmc1_clk_status,mmc1_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_mmc0_phase_clk_status,mmc0_phase_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_mmc1_phase_clk_status,mmc1_phase_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_mmc0_refclk_status,mmc0_refclk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_mmc1_refclk_status,mmc1_refclk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_reserved_2,reserved_2,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_sdcc_clk_status,sdcc_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_hpm_clk_status,hpm_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_reserved_1,reserved_1,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_pcie0_ctrl_clk_status,pcie0_ctrl_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_pcie0_phy_clk_status,pcie0_phy_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_pcie1_phy_clk_status,pcie1_phy_clk_status,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat2_reserved_0,reserved_0,HI_PD_CRG_CLKSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_uart3_clkcg_status,uart3_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_uart2_clkcg_status,uart2_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_gpio2_clkcg_status,gpio2_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_cs_clkcg_status,cs_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_acpu_mbist_clkcg_status,acpu_mbist_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_reserved_5,reserved_5,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_reserved_4,reserved_4,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_mddrc_clkcg_status,mddrc_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_ddrphy_clkcg_status,ddrphy_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_ddrphy_ddr_clkcg_status,ddrphy_ddr_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_axi_mem_clkcg_status,axi_mem_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_cs_trace_clkcg_status,cs_trace_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_reserved_3,reserved_3,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_usbotg_clkcg_status,usbotg_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_hdlc_clkcg_status,hdlc_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_pcie1_ctrl_clkcg_status,pcie1_ctrl_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_mmc0_clkcg_status,mmc0_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_mmc1_clkcg_status,mmc1_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_mmc0_phase_clkcg_status,mmc0_phase_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_mmc1_phase_clkcg_status,mmc1_phase_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_mmc0_refclkcg_status,mmc0_refclkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_mmc1_refclkcg_status,mmc1_refclkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_reserved_2,reserved_2,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_sdcc_clkcg_status,sdcc_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_hpm_clkcg_status,hpm_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_reserved_1,reserved_1,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_pcie0_ctrl_clkcg_status,pcie0_ctrl_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_pcie0_phy_clkcg_status,pcie0_phy_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_pcie1_phy_clkcg_status,pcie1_phy_clkcg_status,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat2_reserved_0,reserved_0,HI_PD_CRG_CLKCGSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_reserved_3,reserved_3,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_uart0_clk_en,uart0_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_reserved_2,reserved_2,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_gpio4_clk_en,gpio4_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_gpio5_clk_en,gpio5_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_gpio6_clk_en,gpio6_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_gpio7_clk_en,gpio7_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_gpio8_clk_en,gpio8_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_gpio9_clk_en,gpio9_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_gpio10_clk_en,gpio10_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_gpio11_clk_en,gpio11_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_gpio12_clk_en,gpio12_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_hifi_atb_clk_en,hifi_atb_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_zsi_clk_en,zsi_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_reserved_1,reserved_1,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_acpu_dbg_clk_en,acpu_dbg_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_acpu_atb_clk_en,acpu_atb_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_acpu_acp_clk_en,acpu_acp_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_appa9_clk_en,appa9_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_reserved_0,reserved_0,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_hifi_vic_clk_en,hifi_vic_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_lte_sio_clk_en,lte_sio_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_hifi_dbg_clk_en,hifi_dbg_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clken3_hifi_clk_en,hifi_clk_en,HI_PD_CRG_CLKEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_reserved_3,reserved_3,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_uart0_clk_dis,uart0_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_reserved_2,reserved_2,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_gpio4_clk_dis,gpio4_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_gpio5_clk_dis,gpio5_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_gpio6_clk_dis,gpio6_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_gpio7_clk_dis,gpio7_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_gpio8_clk_dis,gpio8_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_gpio9_clk_dis,gpio9_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_gpio10_clk_dis,gpio10_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_gpio11_clk_dis,gpio11_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_gpio12_clk_dis,gpio12_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_hifi_atb_clk_dis,hifi_atb_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_zsi_clk_dis,zsi_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_reserved_1,reserved_1,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_acpu_dbg_clk_dis,acpu_dbg_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_acpu_atb_clk_dis,acpu_atb_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_acpu_acp_clk_dis,acpu_acp_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_appa9_clk_dis,appa9_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_reserved_0,reserved_0,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_hifi_vic_clk_dis,hifi_vic_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_lte_sio_clk_dis,lte_sio_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_hifi_dbg_clk_dis,hifi_dbg_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis3_hifi_clk_dis,hifi_clk_dis,HI_PD_CRG_CLKDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_reserved_3,reserved_3,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_uart0_clk_status,uart0_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_reserved_2,reserved_2,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_gpio4_clk_status,gpio4_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_gpio5_clk_status,gpio5_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_gpio6_clk_status,gpio6_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_gpio7_clk_status,gpio7_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_gpio8_clk_status,gpio8_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_gpio9_clk_status,gpio9_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_gpio10_clk_status,gpio10_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_gpio11_clk_status,gpio11_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_gpio12_clk_status,gpio12_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_hifi_atb_clk_status,hifi_atb_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_zsi_clk_status,zsi_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_reserved_1,reserved_1,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_acpu_dbg_clk_status,acpu_dbg_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_acpu_atb_clk_status,acpu_atb_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_acpu_acp_clk_status,acpu_acp_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_appa9_clk_status,appa9_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_reserved_0,reserved_0,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_hifi_vic_clk_status,hifi_vic_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_lte_sio_clk_status,lte_sio_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_hifi_dbg_clk_status,hifi_dbg_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat3_hifi_clk_status,hifi_clk_status,HI_PD_CRG_CLKSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_reserved_3,reserved_3,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_uart0_clkcg_status,uart0_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_reserved_2,reserved_2,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_gpio4_clkcg_status,gpio4_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_gpio5_clkcg_status,gpio5_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_gpio6_clkcg_status,gpio6_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_gpio7_clkcg_status,gpio7_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_gpio8_clkcg_status,gpio8_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_gpio9_clkcg_status,gpio9_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_gpio10_clkcg_status,gpio10_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_gpio11_clkcg_status,gpio11_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_gpio12_clkcg_status,gpio12_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_hifi_atb_clkcg_status,hifi_atb_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_zsi_clkcg_status,zsi_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_reserved_1,reserved_1,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_acpu_dbg_clkcg_status,acpu_dbg_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_acpu_atb_clkcg_status,acpu_atb_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_acpu_acp_clkcg_status,acpu_acp_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_appa9_clkcg_status,appa9_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_reserved_0,reserved_0,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_hifi_vic_clkcg_status,hifi_vic_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_lte_sio_clkcg_status,lte_sio_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_hifi_dbg_clkcg_status,hifi_dbg_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat3_hifi_clkcg_status,hifi_clkcg_status,HI_PD_CRG_CLKCGSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_a9_refclk_en,a9_refclk_en,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_hifi_refclk_en,hifi_refclk_en,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_reserved_4,reserved_4,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_hdlc_clk_en,hdlc_clk_en,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_reserved_3,reserved_3,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_uart1_clk_en,uart1_clk_en,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_reserved_2,reserved_2,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_gmac_refclk_en,gmac_refclk_en,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_gmac_rx_clk_en,gmac_rx_clk_en,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_gmac_tx_clk_en,gmac_tx_clk_en,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_gmac_gtx_clk_en,gmac_gtx_clk_en,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_gmac_switch_clk_en,gmac_switch_clk_en,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_gmac_aclk_en,gmac_aclk_en,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_reserved_1,reserved_1,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_dsp_refclk_en,dsp_refclk_en,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clken4_reserved_0,reserved_0,HI_PD_CRG_CLKEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKEN4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_a9_refclk_dis,a9_refclk_dis,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_hifi_refclk_dis,hifi_refclk_dis,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_reserved_3,reserved_3,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_hdlc_clk_dis,hdlc_clk_dis,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_reserved_2,reserved_2,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_uart1_clk_dis,uart1_clk_dis,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_reserved_1,reserved_1,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_gmac_rx_clk_dis,gmac_rx_clk_dis,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_gmac_tx_clk_dis,gmac_tx_clk_dis,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_gmac_gtx_clk_dis,gmac_gtx_clk_dis,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_gmac_switch_clk_dis,gmac_switch_clk_dis,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_gmac_aclk_dis,gmac_aclk_dis,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_dsp_refclk_dis,dsp_refclk_dis,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdis4_reserved_0,reserved_0,HI_PD_CRG_CLKDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_a9_refclk_status,a9_refclk_status,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_hifi_refclk_status,hifi_refclk_status,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_reserved_3,reserved_3,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_hdlc_clk_status,hdlc_clk_status,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_reserved_2,reserved_2,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_uart1_clk_status,uart1_clk_status,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_reserved_1,reserved_1,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_gmac_rx_clk_status,gmac_rx_clk_status,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_gmac_tx_clk_status,gmac_tx_clk_status,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_gmac_gtx_clk_status,gmac_gtx_clk_status,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_gmac_switch_clk_status,gmac_switch_clk_status,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_gmac_aclk_status,gmac_aclk_status,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_dsp_refclk_status,dsp_refclk_status,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkstat4_reserved_0,reserved_0,HI_PD_CRG_CLKSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_reserved_8,reserved_8,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_reserved_7,reserved_7,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_reserved_6,reserved_6,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_hdlc_clkcg_status,hdlc_clkcg_status,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_reserved_5,reserved_5,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_uart1_clkcg_status,uart1_clkcg_status,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_reserved_4,reserved_4,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_gmac_rx_clkcg_status,gmac_rx_clkcg_status,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_gmac_tx_clkcg_status,gmac_tx_clkcg_status,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_reserved_3,reserved_3,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_reserved_2,reserved_2,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_gmac_aclk_status,gmac_aclk_status,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_reserved_1,reserved_1,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkcgstat4_reserved_0,reserved_0,HI_PD_CRG_CLKCGSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKCGSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_appa9_core_srst_en,appa9_core_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_appa9_dbg_srst_en,appa9_dbg_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_appa9_peri_srst_en,appa9_peri_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_appa9_scu_srst_en,appa9_scu_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_appa9_ptm_srst_en,appa9_ptm_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_appa9_wd_srst_en,appa9_wd_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_appa9_l2c_srst_en,appa9_l2c_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_13,reserved_13,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_12,reserved_12,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_hpm_srst_en,hpm_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_11,reserved_11,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_appa9_srst_en,appa9_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_10,reserved_10,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_9,reserved_9,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_mddrc_srst_en,mddrc_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_8,reserved_8,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_7,reserved_7,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_6,reserved_6,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_hdlc_srst_en,hdlc_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_5,reserved_5,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_4,reserved_4,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_edmac_srst_en,edmac_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_emi_srst_en,emi_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_3,reserved_3,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_2,reserved_2,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_1,reserved_1,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_amon_soc_srst_en,amon_soc_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_reserved_0,reserved_0,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten1_rsa_srst_en,rsa_srst_en,HI_PD_CRG_SRSTEN1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_appa9_core_srst_dis,appa9_core_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_appa9_dbg_srst_dis,appa9_dbg_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_appa9_peri_srst_dis,appa9_peri_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_appa9_scu_srst_dis,appa9_scu_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_appa9_ptm_srst_dis,appa9_ptm_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_appa9_wd_srst_dis,appa9_wd_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_appa9_l2c_srst_dis,appa9_l2c_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_13,reserved_13,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_12,reserved_12,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_hpm_srst_dis,hpm_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_11,reserved_11,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_appa9_srst_dis,appa9_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_10,reserved_10,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_9,reserved_9,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_mddrc_srst_dis,mddrc_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_8,reserved_8,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_7,reserved_7,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_6,reserved_6,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_hdlc_srst_dis,hdlc_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_5,reserved_5,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_4,reserved_4,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_edmac_srst_dis,edmac_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_emi_srst_dis,emi_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_3,reserved_3,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_2,reserved_2,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_1,reserved_1,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_amon_soc_srst_dis,amon_soc_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_reserved_0,reserved_0,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis1_rsa_srst_dis,rsa_srst_dis,HI_PD_CRG_SRSTDIS1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_appa9_core_srst_status,appa9_core_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_appa9_dbg_srst_status,appa9_dbg_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_appa9_peri_srst_status,appa9_peri_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_appa9_scu_srst_status,appa9_scu_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_appa9_ptm_srst_status,appa9_ptm_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_appa9_wd_srst_status,appa9_wd_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_appa9_l2c_srst_status,appa9_l2c_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_13,reserved_13,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_12,reserved_12,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_hpm_srst_status,hpm_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_11,reserved_11,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_appa9_srst_status,appa9_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_10,reserved_10,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_9,reserved_9,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_mddrc_srst_status,mddrc_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_8,reserved_8,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_7,reserved_7,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_6,reserved_6,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_hdlc_srst_status,hdlc_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_5,reserved_5,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_4,reserved_4,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_edmac_srst_status,edmac_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_emi_srst_status,emi_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_3,reserved_3,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_2,reserved_2,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_1,reserved_1,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_amon_soc_srst_status,amon_soc_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_reserved_0,reserved_0,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srststat1_rsa_srst_status,rsa_srst_status,HI_PD_CRG_SRSTSTAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT1_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_reserved_10,reserved_10,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_reserved_9,reserved_9,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_mmc0_srst_en,mmc0_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_mmc1_srst_en,mmc1_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_sdcc_srst_en,sdcc_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_hsuart_srst_en,hsuart_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_sio_srst_en,sio_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_gmac_srst_en,gmac_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_pcie1_ctrl_srst_en,pcie1_ctrl_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_pcie1_phy_srst_en,pcie1_phy_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_pcie0_ctrl_srst_en,pcie0_ctrl_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_pcie0_phy_srst_en,pcie0_phy_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_reserved_8,reserved_8,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_hifi_core_srst_en,hifi_core_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_hifi_pd_srst_en,hifi_pd_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_hifi_dbg_srst_en,hifi_dbg_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_reserved_7,reserved_7,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_reserved_6,reserved_6,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_reserved_5,reserved_5,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_reserved_4,reserved_4,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_reserved_3,reserved_3,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_reserved_2,reserved_2,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_reserved_1,reserved_1,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_cs_srst_en,cs_srst_en,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten2_reserved_0,reserved_0,HI_PD_CRG_SRSTEN2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_reserved_11,reserved_11,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_reserved_10,reserved_10,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_mmc0_srst_dis,mmc0_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_mmc1_srst_dis,mmc1_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_sdcc_srst_dis,sdcc_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_hsuart_srst_dis,hsuart_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_sio_srst_dis,sio_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_gmac_srst_dis,gmac_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_pcie1_ctrl_srst_dis,pcie1_ctrl_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_pcie1_phy_srst_dis,pcie1_phy_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_pcie_ctrl_srst_dis,pcie_ctrl_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_pcie_phy_srst_dis,pcie_phy_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_reserved_9,reserved_9,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_reserved_8,reserved_8,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_hifi_core_srst_dis,hifi_core_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_hifi_pd_srst_dis,hifi_pd_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_hifi_dbg_srst_dis,hifi_dbg_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_reserved_7,reserved_7,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_reserved_6,reserved_6,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_reserved_5,reserved_5,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_reserved_4,reserved_4,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_reserved_3,reserved_3,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_reserved_2,reserved_2,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_reserved_1,reserved_1,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_cs_srst_dis,cs_srst_dis,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis2_reserved_0,reserved_0,HI_PD_CRG_SRSTDIS2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_reserved_10,reserved_10,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_reserved_9,reserved_9,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_mmc0_srst_status,mmc0_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_mmc1_srst_status,mmc1_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_sdcc_srst_status,sdcc_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_hsuart_srst_status,hsuart_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_sio_srst_status,sio_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_gmac_srst_status,gmac_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_pcie1_ctrl_srst_status,pcie1_ctrl_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_pcie1_phy_srst_status,pcie1_phy_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_pcie0_ctrl_srst_status,pcie0_ctrl_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_pcie0_phy_srst_status,pcie0_phy_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_reserved_8,reserved_8,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_hificore_srst_status,hificore_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_hifi_pd_srst_status,hifi_pd_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_hifi_dbg_srst_status,hifi_dbg_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_reserved_7,reserved_7,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_reserved_6,reserved_6,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_reserved_5,reserved_5,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_reserved_4,reserved_4,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_reserved_3,reserved_3,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_reserved_2,reserved_2,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_reserved_1,reserved_1,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_cs_srst_status,cs_srst_status,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srststat2_reserved_0,reserved_0,HI_PD_CRG_SRSTSTAT2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT2_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_gpio2_srst_en,gpio2_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_gpio3_srst_en,gpio3_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_gpio4_srst_en,gpio4_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_gpio5_srst_en,gpio5_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_gpio6_srst_en,gpio6_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_gpio7_srst_en,gpio7_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_gpio8_srst_en,gpio8_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_gpio9_srst_en,gpio9_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_gpio10_srst_en,gpio10_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_gpio11_srst_en,gpio11_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_gpio12_srst_en,gpio12_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_i2c0_srst_en,i2c0_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_i2c1_srst_en,i2c1_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_spi0_srst_en,spi0_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_spi1_srst_en,spi1_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_uart0_srst_en,uart0_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_uart1_srst_en,uart1_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_uart2_srst_en,uart2_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_hkadcssi_srst_en,hkadcssi_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_reserved_1,reserved_1,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_pmussi0_srst_en,pmussi0_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_pwm1_srst_en,pwm1_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_pwm0_srst_en,pwm0_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_rsracc_srst_en,rsracc_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_psam_srst_en,psam_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_ipf_srst_en,ipf_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_socp_srst_en,socp_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_spe_srst_en,spe_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_nandc_srst_en,nandc_srst_en,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten3_reserved_0,reserved_0,HI_PD_CRG_SRSTEN3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_gpio2_srst_dis,gpio2_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_gpio3_srst_dis,gpio3_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_gpio4_srst_dis,gpio4_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_gpio5_srst_dis,gpio5_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_gpio6_srst_dis,gpio6_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_gpio7_srst_dis,gpio7_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_gpio8_srst_dis,gpio8_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_gpio9_srst_dis,gpio9_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_gpio10_srst_dis,gpio10_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_gpio11_srst_dis,gpio11_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_gpio12_srst_dis,gpio12_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_i2c0_srst_dis,i2c0_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_i2c1_srst_dis,i2c1_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_spi0_srst_dis,spi0_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_spi1_srst_dis,spi1_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_uart0_srst_dis,uart0_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_uart1_srst_dis,uart1_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_uart2_srst_dis,uart2_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_hkadcssi_srst_dis,hkadcssi_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_reserved_1,reserved_1,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_pmussi0_srst_dis,pmussi0_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_pwm1_srst_dis,pwm1_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_pwm0_srst_dis,pwm0_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_rsracc_srst_dis,rsracc_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_psam_srst_dis,psam_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_ipf_srst_dis,ipf_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_socp_srst_dis,socp_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_spe_srst_dis,spe_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_nandc_srst_dis,nandc_srst_dis,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis3_reserved_0,reserved_0,HI_PD_CRG_SRSTDIS3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_gpio2_srst_status,gpio2_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_gpio3_srst_status,gpio3_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_gpio4_srst_status,gpio4_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_gpio5_srst_status,gpio5_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_gpio6_srst_status,gpio6_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_gpio7_srst_status,gpio7_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_gpio8_srst_status,gpio8_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_gpio9_srst_status,gpio9_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_gpio10_srst_status,gpio10_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_gpio11_srst_status,gpio11_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_gpio12_srst_status,gpio12_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_i2c0_srst_status,i2c0_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_i2c1_srst_status,i2c1_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_spi0_srst_status,spi0_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_spi1_srst_status,spi1_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_uart0_srst_status,uart0_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_uart1_srst_status,uart1_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_uart2_srst_status,uart2_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_hkadcssi_srst_status,hkadcssi_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_reserved_1,reserved_1,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_pmussi0_srst_status,pmussi0_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_pwm1_srst_status,pwm1_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_pwm0_srst_status,pwm0_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_rsracc_srst_status,rsracc_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_psam_srst_status,psam_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_ipf_srst_status,ipf_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_socp_srst_status,socp_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_spe_srst_status,spe_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_nandc_srst_status,nandc_srst_status,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srststat3_reserved_0,reserved_0,HI_PD_CRG_SRSTSTAT3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT3_OFFSET)
HI_SET_GET(hi_pd_crg_srsten4_reserved,reserved,HI_PD_CRG_SRSTEN4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTEN4_OFFSET)
HI_SET_GET(hi_pd_crg_srstdis4_reserved,reserved,HI_PD_CRG_SRSTDIS4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTDIS4_OFFSET)
HI_SET_GET(hi_pd_crg_srststat4_reserved,reserved,HI_PD_CRG_SRSTSTAT4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SRSTSTAT4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv1_pcm_bclk_div,pcm_bclk_div,HI_PD_CRG_CLKDIV1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv1_pcm_sync_div,pcm_sync_div,HI_PD_CRG_CLKDIV1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv1_reserved,reserved,HI_PD_CRG_CLKDIV1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV1_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv2_acpu_clk_div,acpu_clk_div,HI_PD_CRG_CLKDIV2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv2_ccpu_clk_div,ccpu_clk_div,HI_PD_CRG_CLKDIV2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv2_slow_clk_div,slow_clk_div,HI_PD_CRG_CLKDIV2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv2_fast_clk_div,fast_clk_div,HI_PD_CRG_CLKDIV2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv2_reserved_3,reserved_3,HI_PD_CRG_CLKDIV2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv2_reserved_2,reserved_2,HI_PD_CRG_CLKDIV2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv2_reserved_1,reserved_1,HI_PD_CRG_CLKDIV2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv2_apb_freqmode,apb_freqmode,HI_PD_CRG_CLKDIV2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv2_reserved_0,reserved_0,HI_PD_CRG_CLKDIV2_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV2_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv3_mmc0_cclk_div,mmc0_cclk_div,HI_PD_CRG_CLKDIV3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv3_mmc0_refclk_sel,mmc0_refclk_sel,HI_PD_CRG_CLKDIV3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv3_reserved_3,reserved_3,HI_PD_CRG_CLKDIV3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv3_mmc0_phase_clk_div,mmc0_phase_clk_div,HI_PD_CRG_CLKDIV3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv3_reserved_2,reserved_2,HI_PD_CRG_CLKDIV3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv3_mmc1_cclk_div,mmc1_cclk_div,HI_PD_CRG_CLKDIV3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv3_mmc1_refclk_sel,mmc1_refclk_sel,HI_PD_CRG_CLKDIV3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv3_reserved_1,reserved_1,HI_PD_CRG_CLKDIV3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv3_mmc1_phase_clk_div,mmc1_phase_clk_div,HI_PD_CRG_CLKDIV3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv3_reserved_0,reserved_0,HI_PD_CRG_CLKDIV3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv3_hpm_clk_div,hpm_clk_div,HI_PD_CRG_CLKDIV3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv3_spe_clk_div,spe_clk_div,HI_PD_CRG_CLKDIV3_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV3_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv4_hifi_clk_div_num,hifi_clk_div_num,HI_PD_CRG_CLKDIV4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv4_hifi_freqmode,hifi_freqmode,HI_PD_CRG_CLKDIV4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv4_reserved_1,reserved_1,HI_PD_CRG_CLKDIV4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv4_hifi_refclk_sel,hifi_refclk_sel,HI_PD_CRG_CLKDIV4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv4_lp_hifi_clk_div_num,lp_hifi_clk_div_num,HI_PD_CRG_CLKDIV4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv4_lp_hifi_div_num_enable,lp_hifi_div_num_enable,HI_PD_CRG_CLKDIV4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV4_OFFSET)
HI_SET_GET(hi_pd_crg_clkdiv4_reserved_0,reserved_0,HI_PD_CRG_CLKDIV4_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKDIV4_OFFSET)
HI_SET_GET(hi_pd_crg_mmc_dly_ctrl_mmc0_drv_sel,mmc0_drv_sel,HI_PD_CRG_MMC_DLY_CTRL_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_MMC_DLY_CTRL_OFFSET)
HI_SET_GET(hi_pd_crg_mmc_dly_ctrl_mmc0_clk_bypass,mmc0_clk_bypass,HI_PD_CRG_MMC_DLY_CTRL_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_MMC_DLY_CTRL_OFFSET)
HI_SET_GET(hi_pd_crg_mmc_dly_ctrl_mmc0_sample_sel,mmc0_sample_sel,HI_PD_CRG_MMC_DLY_CTRL_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_MMC_DLY_CTRL_OFFSET)
HI_SET_GET(hi_pd_crg_mmc_dly_ctrl_mmc0_sample_ctrl,mmc0_sample_ctrl,HI_PD_CRG_MMC_DLY_CTRL_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_MMC_DLY_CTRL_OFFSET)
HI_SET_GET(hi_pd_crg_mmc_dly_ctrl_mmc1_drv_sel,mmc1_drv_sel,HI_PD_CRG_MMC_DLY_CTRL_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_MMC_DLY_CTRL_OFFSET)
HI_SET_GET(hi_pd_crg_mmc_dly_ctrl_mmc1_clk_bypass,mmc1_clk_bypass,HI_PD_CRG_MMC_DLY_CTRL_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_MMC_DLY_CTRL_OFFSET)
HI_SET_GET(hi_pd_crg_mmc_dly_ctrl_mmc1_sample_sel,mmc1_sample_sel,HI_PD_CRG_MMC_DLY_CTRL_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_MMC_DLY_CTRL_OFFSET)
HI_SET_GET(hi_pd_crg_mmc_dly_ctrl_mmc1_sample_ctrl,mmc1_sample_ctrl,HI_PD_CRG_MMC_DLY_CTRL_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_MMC_DLY_CTRL_OFFSET)
HI_SET_GET(hi_plllock_ctrl_reserved,reserved,HI_PLLLOCK_CTRL_T,HI_PD_CRG_BASE_ADDR, HI_PLLLOCK_CTRL_OFFSET)
HI_SET_GET(hi_pd_crg_sio_cfg_sio_sync_cfg_cnt,sio_sync_cfg_cnt,HI_PD_CRG_SIO_CFG_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SIO_CFG_OFFSET)
HI_SET_GET(hi_pd_crg_sio_cfg_reserved,reserved,HI_PD_CRG_SIO_CFG_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_SIO_CFG_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_dsp_refclk_sw_req,dsp_refclk_sw_req,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_reserved_8,reserved_8,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_sio_clk_sel,sio_clk_sel,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_reserved_7,reserved_7,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_a9_pllclk_sw_req,a9_pllclk_sw_req,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_reserved_6,reserved_6,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_reserved_5,reserved_5,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_reserved_4,reserved_4,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_gmac_rx_clk_sel,gmac_rx_clk_sel,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_gmac_tx_clk_sel,gmac_tx_clk_sel,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_gmac_gtx_clk_sel,gmac_gtx_clk_sel,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_reserved_3,reserved_3,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_reserved_2,reserved_2,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_reserved_1,reserved_1,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_sio_refclk_sel,sio_refclk_sel,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_pd_crg_clk_sel1_reserved_0,reserved_0,HI_PD_CRG_CLK_SEL1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLK_SEL1_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl1_dfs_frac,dfs_frac,HI_CRG_DFS1_CTRL1_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL1_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl1_reserved,reserved,HI_CRG_DFS1_CTRL1_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL1_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl2_dfs_intprog,dfs_intprog,HI_CRG_DFS1_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl2_dfs_refdiv,dfs_refdiv,HI_CRG_DFS1_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl2_reserved_2,reserved_2,HI_CRG_DFS1_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl2_dfs_postdiv2,dfs_postdiv2,HI_CRG_DFS1_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl2_reserved_1,reserved_1,HI_CRG_DFS1_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl2_dfs_postdiv1,dfs_postdiv1,HI_CRG_DFS1_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl2_reserved_0,reserved_0,HI_CRG_DFS1_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_pd,pd,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_reserved_1,reserved_1,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_dsmpd,dsmpd,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_phase4_pd,phase4_pd,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_postdivpd,postdivpd,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_vcopd,vcopd,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_dacpd,dacpd,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_bypass,bypass,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_pll_ctrl,pll_ctrl,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_postfout_clk_en,postfout_clk_en,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_fout1_clk_en,fout1_clk_en,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_fout2_clk_en,fout2_clk_en,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_fout3_clk_en,fout3_clk_en,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_fout4_clk_en,fout4_clk_en,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_reserved_0,reserved_0,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs1_ctrl3_lock,lock,HI_CRG_DFS1_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS1_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl1_dfs_frac,dfs_frac,HI_CRG_DFS2_CTRL1_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL1_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl1_reserved,reserved,HI_CRG_DFS2_CTRL1_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL1_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl2_dfs_intprog,dfs_intprog,HI_CRG_DFS2_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl2_dfs_refdiv,dfs_refdiv,HI_CRG_DFS2_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl2_reserved_2,reserved_2,HI_CRG_DFS2_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl2_dfs_postdiv2,dfs_postdiv2,HI_CRG_DFS2_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl2_reserved_1,reserved_1,HI_CRG_DFS2_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl2_dfs_postdiv1,dfs_postdiv1,HI_CRG_DFS2_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl2_reserved_0,reserved_0,HI_CRG_DFS2_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_pd,pd,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_reserved_2,reserved_2,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_dsmpd,dsmpd,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_phase4_pd,phase4_pd,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_postdivpd,postdivpd,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_vcopd,vcopd,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_dacpd,dacpd,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_bypass,bypass,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_reserved_1,reserved_1,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_postfout_clk_en,postfout_clk_en,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_fout1_clk_en,fout1_clk_en,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_fout2_clk_en,fout2_clk_en,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_fout3_clk_en,fout3_clk_en,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_fout4_clk_en,fout4_clk_en,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_reserved_0,reserved_0,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs2_ctrl3_lock,lock,HI_CRG_DFS2_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS2_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl1_dfs_frac,dfs_frac,HI_CRG_DFS3_CTRL1_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL1_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl1_reserved,reserved,HI_CRG_DFS3_CTRL1_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL1_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl2_dfs_intprog,dfs_intprog,HI_CRG_DFS3_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl2_dfs_refdiv,dfs_refdiv,HI_CRG_DFS3_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl2_reserved_2,reserved_2,HI_CRG_DFS3_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl2_dfs_postdiv2,dfs_postdiv2,HI_CRG_DFS3_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl2_reserved_1,reserved_1,HI_CRG_DFS3_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl2_dfs_postdiv1,dfs_postdiv1,HI_CRG_DFS3_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl2_reserved_0,reserved_0,HI_CRG_DFS3_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_pd,pd,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_reserved_2,reserved_2,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_dsmpd,dsmpd,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_phase4_pd,phase4_pd,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_postdivpd,postdivpd,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_vcopd,vcopd,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_dacpd,dacpd,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_bypass,bypass,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_reserved_1,reserved_1,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_postfout_clk_en,postfout_clk_en,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_fout1_clk_en,fout1_clk_en,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_fout2_clk_en,fout2_clk_en,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_fout3_clk_en,fout3_clk_en,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_fout4_clk_en,fout4_clk_en,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_reserved_0,reserved_0,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs3_ctrl3_lock,lock,HI_CRG_DFS3_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS3_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl1_dfs_frac,dfs_frac,HI_CRG_DFS5_CTRL1_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL1_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl1_reserved,reserved,HI_CRG_DFS5_CTRL1_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL1_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl2_dfs_intprog,dfs_intprog,HI_CRG_DFS5_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl2_dfs_refdiv,dfs_refdiv,HI_CRG_DFS5_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl2_reserved_2,reserved_2,HI_CRG_DFS5_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl2_dfs_postdiv2,dfs_postdiv2,HI_CRG_DFS5_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl2_reserved_1,reserved_1,HI_CRG_DFS5_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl2_dfs_postdiv1,dfs_postdiv1,HI_CRG_DFS5_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl2_reserved_0,reserved_0,HI_CRG_DFS5_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_pd,pd,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_reserved_1,reserved_1,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_dsmpd,dsmpd,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_phase4_pd,phase4_pd,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_postdivpd,postdivpd,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_vcopd,vcopd,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_dacpd,dacpd,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_bypass,bypass,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_pll_ctrl,pll_ctrl,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_postfout_clk_en,postfout_clk_en,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_fout1_clk_en,fout1_clk_en,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_fout2_clk_en,fout2_clk_en,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_fout3_clk_en,fout3_clk_en,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_fout4_clk_en,fout4_clk_en,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_reserved_0,reserved_0,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs5_ctrl3_lock,lock,HI_CRG_DFS5_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS5_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl1_dfs_frac,dfs_frac,HI_CRG_DFS6_CTRL1_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL1_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl1_reserved,reserved,HI_CRG_DFS6_CTRL1_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL1_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl2_dfs_intprog,dfs_intprog,HI_CRG_DFS6_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl2_dfs_refdiv,dfs_refdiv,HI_CRG_DFS6_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl2_reserved_2,reserved_2,HI_CRG_DFS6_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl2_dfs_postdiv2,dfs_postdiv2,HI_CRG_DFS6_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl2_reserved_1,reserved_1,HI_CRG_DFS6_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl2_dfs_postdiv1,dfs_postdiv1,HI_CRG_DFS6_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl2_reserved_0,reserved_0,HI_CRG_DFS6_CTRL2_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL2_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_pd,pd,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_reserved_2,reserved_2,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_dsmpd,dsmpd,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_phase4_pd,phase4_pd,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_postdivpd,postdivpd,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_vcopd,vcopd,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_dacpd,dacpd,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_bypass,bypass,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_reserved_1,reserved_1,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_postfout_clk_en,postfout_clk_en,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_fout1_clk_en,fout1_clk_en,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_fout2_clk_en,fout2_clk_en,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_fout3_clk_en,fout3_clk_en,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_fout4_clk_en,fout4_clk_en,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_reserved_0,reserved_0,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_crg_dfs6_ctrl3_lock,lock,HI_CRG_DFS6_CTRL3_T,HI_PD_CRG_BASE_ADDR, HI_CRG_DFS6_CTRL3_OFFSET)
HI_SET_GET(hi_pd_crg_clksw_stat1_reserved_4,reserved_4,HI_PD_CRG_CLKSW_STAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSW_STAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clksw_stat1_reserved_3,reserved_3,HI_PD_CRG_CLKSW_STAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSW_STAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clksw_stat1_reserved_2,reserved_2,HI_PD_CRG_CLKSW_STAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSW_STAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clksw_stat1_reserved_1,reserved_1,HI_PD_CRG_CLKSW_STAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSW_STAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clksw_stat1_hifi_refclk_sw_ack,hifi_refclk_sw_ack,HI_PD_CRG_CLKSW_STAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSW_STAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clksw_stat1_dsp_refclk_sw_ack,dsp_refclk_sw_ack,HI_PD_CRG_CLKSW_STAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSW_STAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clksw_stat1_a9_pllclk_sw_ack,a9_pllclk_sw_ack,HI_PD_CRG_CLKSW_STAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSW_STAT1_OFFSET)
HI_SET_GET(hi_pd_crg_clksw_stat1_reserved_0,reserved_0,HI_PD_CRG_CLKSW_STAT1_T,HI_PD_CRG_BASE_ADDR, HI_PD_CRG_CLKSW_STAT1_OFFSET)
#endif
#endif
