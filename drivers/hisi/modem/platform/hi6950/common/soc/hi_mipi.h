#ifndef __HI_BBP_MIPI_H__
#define __HI_BBP_MIPI_H__ 
#ifndef HI_SET_GET_MIPI
#define HI_SET_GET_MIPI(a0,a1,a2,a3,a4) 
#endif
#define MIPI_MIPI0_RD_DATA_LOW_SOFT_OFFSET (0x0780)
#define MIPI_MIPI0_RD_DATA_HIGH_SOFT_OFFSET (0x0784)
#define MIPI_RD_END_FLAG_MIPI0_SOFT_OFFSET (0x0788)
#define MIPI_MIPI0_GRANT_DSP_OFFSET (0x078C)
#define MIPI_DSP_MIPI0_WDATA_LOW_OFFSET (0x0790)
#define MIPI_DSP_MIPI0_WDATA_HIGH_OFFSET (0x0794)
#define MIPI_DSP_MIPI0_EN_IMI_OFFSET (0x0798)
#define MIPI_DSP_MIPI0_CFG_IND_IMI_OFFSET (0x079C)
#define MIPI_DSP_MIPI0_RD_CLR_OFFSET (0x07A0)
typedef union
{
    struct
    {
        unsigned int mipi0_rd_data_low_soft : 32;
    } bits;
    unsigned int u32;
}BBP_MIPI_MIPI0_RD_DATA_LOW_SOFT_T;
typedef union
{
    struct
    {
        unsigned int mipi0_rd_data_high_soft : 32;
    } bits;
    unsigned int u32;
}BBP_MIPI_MIPI0_RD_DATA_HIGH_SOFT_T;
typedef union
{
    struct
    {
        unsigned int rd_end_flag_mipi0_soft : 1;
        unsigned int Reserved : 31;
    } bits;
    unsigned int u32;
}BBP_MIPI_RD_END_FLAG_MIPI0_SOFT_T;
typedef union
{
    struct
    {
        unsigned int mipi0_grant_dsp : 1;
        unsigned int Reserved : 31;
    } bits;
    unsigned int u32;
}BBP_MIPI_MIPI0_GRANT_DSP_T;
typedef union
{
    struct
    {
        unsigned int dsp_mipi0_wdata_low : 32;
    } bits;
    unsigned int u32;
}BBP_MIPI_DSP_MIPI0_WDATA_LOW_T;
typedef union
{
    struct
    {
        unsigned int dsp_mipi0_wdata_high : 32;
    } bits;
    unsigned int u32;
}BBP_MIPI_DSP_MIPI0_WDATA_HIGH_T;
typedef union
{
    struct
    {
        unsigned int dsp_mipi0_en_imi : 1;
        unsigned int Reserved : 31;
    } bits;
    unsigned int u32;
}BBP_MIPI_DSP_MIPI0_EN_IMI_T;
typedef union
{
    struct
    {
        unsigned int dsp_mipi0_cfg_ind_imi : 1;
        unsigned int Reserved : 31;
    } bits;
    unsigned int u32;
}BBP_MIPI_DSP_MIPI0_CFG_IND_IMI_T;
typedef union
{
    struct
    {
        unsigned int dsp_mipi0_rd_clr : 1;
        unsigned int Reserved : 31;
    } bits;
    unsigned int u32;
}BBP_MIPI_DSP_MIPI0_RD_CLR_T;
HI_SET_GET_MIPI(bbp_mipi_mipi0_rd_data_low_soft_mipi0_rd_data_low_soft,mipi0_rd_data_low_soft,BBP_MIPI_MIPI0_RD_DATA_LOW_SOFT_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_MIPI0_RD_DATA_LOW_SOFT_OFFSET)
HI_SET_GET_MIPI(bbp_mipi_mipi0_rd_data_high_soft_mipi0_rd_data_high_soft,mipi0_rd_data_high_soft,BBP_MIPI_MIPI0_RD_DATA_HIGH_SOFT_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_MIPI0_RD_DATA_HIGH_SOFT_OFFSET)
HI_SET_GET_MIPI(bbp_mipi_rd_end_flag_mipi0_soft_rd_end_flag_mipi0_soft,rd_end_flag_mipi0_soft,BBP_MIPI_RD_END_FLAG_MIPI0_SOFT_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_RD_END_FLAG_MIPI0_SOFT_OFFSET)
HI_SET_GET_MIPI(bbp_mipi_rd_end_flag_mipi0_soft_reserved,Reserved,BBP_MIPI_RD_END_FLAG_MIPI0_SOFT_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_RD_END_FLAG_MIPI0_SOFT_OFFSET)
HI_SET_GET_MIPI(bbp_mipi_mipi0_grant_dsp_mipi0_grant_dsp,mipi0_grant_dsp,BBP_MIPI_MIPI0_GRANT_DSP_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_MIPI0_GRANT_DSP_OFFSET)
HI_SET_GET_MIPI(bbp_mipi_mipi0_grant_dsp_reserved,Reserved,BBP_MIPI_MIPI0_GRANT_DSP_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_MIPI0_GRANT_DSP_OFFSET)
HI_SET_GET_MIPI(bbp_mipi_dsp_mipi0_wdata_low_dsp_mipi0_wdata_low,dsp_mipi0_wdata_low,BBP_MIPI_DSP_MIPI0_WDATA_LOW_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_DSP_MIPI0_WDATA_LOW_OFFSET)
HI_SET_GET_MIPI(bbp_mipi_dsp_mipi0_wdata_high_dsp_mipi0_wdata_high,dsp_mipi0_wdata_high,BBP_MIPI_DSP_MIPI0_WDATA_HIGH_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_DSP_MIPI0_WDATA_HIGH_OFFSET)
HI_SET_GET_MIPI(bbp_mipi_dsp_mipi0_en_imi_dsp_mipi0_en_imi,dsp_mipi0_en_imi,BBP_MIPI_DSP_MIPI0_EN_IMI_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_DSP_MIPI0_EN_IMI_OFFSET)
HI_SET_GET_MIPI(bbp_mipi_dsp_mipi0_en_imi_reserved,Reserved,BBP_MIPI_DSP_MIPI0_EN_IMI_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_DSP_MIPI0_EN_IMI_OFFSET)
HI_SET_GET_MIPI(bbp_mipi_dsp_mipi0_cfg_ind_imi_dsp_mipi0_cfg_ind_imi,dsp_mipi0_cfg_ind_imi,BBP_MIPI_DSP_MIPI0_CFG_IND_IMI_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_DSP_MIPI0_CFG_IND_IMI_OFFSET)
HI_SET_GET_MIPI(bbp_mipi_dsp_mipi0_cfg_ind_imi_reserved,Reserved,BBP_MIPI_DSP_MIPI0_CFG_IND_IMI_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_DSP_MIPI0_CFG_IND_IMI_OFFSET)
HI_SET_GET_MIPI(bbp_mipi_dsp_mipi0_rd_clr_dsp_mipi0_rd_clr,dsp_mipi0_rd_clr,BBP_MIPI_DSP_MIPI0_RD_CLR_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_DSP_MIPI0_RD_CLR_OFFSET)
HI_SET_GET_MIPI(bbp_mipi_dsp_mipi0_rd_clr_reserved,Reserved,BBP_MIPI_DSP_MIPI0_RD_CLR_T,BBP_MIPI_BASE_ADDR, BBP_MIPI_DSP_MIPI0_RD_CLR_OFFSET)
#endif
