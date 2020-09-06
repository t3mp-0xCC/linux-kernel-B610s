#ifndef __HI_SPI_H__
#define __HI_SPI_H__ 
#ifdef __cplusplus
extern "C" {
#endif
#include "hi_base.h"
#include "hi_syssc.h"
#include <bsp_sysctrl.h>
#define SPI_LCD_NUM 0
#define SPI_LCD_CS 0
#define SPI_SFLASH_NUM 1
#define SPI_SFLASH_CS 0
#define HI_SPI_CTRLR0_OFFSET (0x0)
#define HI_SPI_CTRLR1_OFFSET (0x4)
#define HI_SPI_SSIENR_OFFSET (0x8)
#define HI_SPI_MWCR_OFFSET (0xC)
#define HI_SPI_SER_OFFSET (0x10)
#define HI_SPI_BAUDR_OFFSET (0x14)
#define HI_SPI_TXFTLR_OFFSET (0x18)
#define HI_SPI_RXFTLR_OFFSET (0x1C)
#define HI_SPI_TXFLR_OFFSET (0x20)
#define HI_SPI_RXFLR_OFFSET (0x24)
#define HI_SPI_SR_OFFSET (0x28)
#define HI_SPI_IMR_OFFSET (0x2C)
#define HI_SPI_ISR_OFFSET (0x30)
#define HI_SPI_RISR_OFFSET (0x34)
#define HI_SPI_TXOICR_OFFSET (0x38)
#define HI_SPI_RXOICR_OFFSET (0x3C)
#define HI_SPI_RXUICR_OFFSET (0x40)
#define HI_SPI_MSTICR_OFFSET (0x44)
#define HI_SPI_ICR_OFFSET (0x48)
#define HI_SPI_DMACR_OFFSET (0x4C)
#define HI_SPI_DMATDLR_OFFSET (0x50)
#define HI_SPI_DMARDLR_OFFSET (0x54)
#define HI_SPI_IDR_OFFSET (0x58)
#define HI_SPI_SSI_COMP_VERSION_OFFSET (0x5C)
#define HI_SPI_DR_OFFSET (0x60)
#define HI_SPI_RX_SAMPLE_DLY_OFFSET (0xF0)
typedef union
{
    struct
    {
        unsigned int DFS : 4;
        unsigned int FRF : 2;
        unsigned int SCPH : 1;
        unsigned int SCPOL : 1;
        unsigned int TMOD : 2;
        unsigned int SLV_OE : 1;
        unsigned int SRL : 1;
        unsigned int CFS : 4;
        unsigned int reserved : 16;
    } bits;
    unsigned int u32;
}HI_SPI_CTRLR0_T;
typedef union
{
    struct
    {
        unsigned int NDF : 16;
        unsigned int reserved : 16;
    } bits;
    unsigned int u32;
}HI_SPI_CTRLR1_T;
typedef union
{
    struct
    {
        unsigned int SSI_EN : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_SPI_SSIENR_T;
typedef union
{
    struct
    {
        unsigned int MWMOD : 1;
        unsigned int MDD : 1;
        unsigned int MHS : 1;
        unsigned int reserved_1 : 1;
        unsigned int mw_toggle_cyc : 4;
        unsigned int reserved_0 : 24;
    } bits;
    unsigned int u32;
}HI_SPI_MWCR_T;
typedef union
{
    struct
    {
        unsigned int SER : 4;
        unsigned int reserved : 28;
    } bits;
    unsigned int u32;
}HI_SPI_SER_T;
typedef union
{
    struct
    {
        unsigned int SCKDV : 16;
        unsigned int reserved : 16;
    } bits;
    unsigned int u32;
}HI_SPI_BAUDR_T;
typedef union
{
    struct
    {
        unsigned int TFT : 8;
        unsigned int reserved : 24;
    } bits;
    unsigned int u32;
}HI_SPI_TXFTLR_T;
typedef union
{
    struct
    {
        unsigned int RFT : 3;
        unsigned int reserved_1 : 5;
        unsigned int reserved_0 : 24;
    } bits;
    unsigned int u32;
}HI_SPI_RXFTLR_T;
typedef union
{
    struct
    {
        unsigned int TXTFL : 9;
        unsigned int reserved : 23;
    } bits;
    unsigned int u32;
}HI_SPI_TXFLR_T;
typedef union
{
    struct
    {
        unsigned int RXTFL : 4;
        unsigned int reserved : 28;
    } bits;
    unsigned int u32;
}HI_SPI_RXFLR_T;
typedef union
{
    struct
    {
        unsigned int BUSY : 1;
        unsigned int TFNF : 1;
        unsigned int TFE : 1;
        unsigned int RFNE : 1;
        unsigned int RFF : 1;
        unsigned int TXE : 1;
        unsigned int DCOL : 1;
        unsigned int reserved : 25;
    } bits;
    unsigned int u32;
}HI_SPI_SR_T;
typedef union
{
    struct
    {
        unsigned int TXEIM : 1;
        unsigned int TXOIM : 1;
        unsigned int RXUIM : 1;
        unsigned int RXOIM : 1;
        unsigned int RXFIM : 1;
        unsigned int MSTIM : 1;
        unsigned int reserved : 26;
    } bits;
    unsigned int u32;
}HI_SPI_IMR_T;
typedef union
{
    struct
    {
        unsigned int TXEIS : 1;
        unsigned int TXOIS : 1;
        unsigned int RXUIS : 1;
        unsigned int RXOIS : 1;
        unsigned int RXFIS : 1;
        unsigned int MSTIS : 1;
        unsigned int reserved : 26;
    } bits;
    unsigned int u32;
}HI_SPI_ISR_T;
typedef union
{
    struct
    {
        unsigned int TXEIR : 1;
        unsigned int TXOIR : 1;
        unsigned int RXUIR : 1;
        unsigned int RXOIR : 1;
        unsigned int RXFIR : 1;
        unsigned int MSTIR : 1;
        unsigned int reserved : 26;
    } bits;
    unsigned int u32;
}HI_SPI_RISR_T;
typedef union
{
    struct
    {
        unsigned int TXOICR : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_SPI_TXOICR_T;
typedef union
{
    struct
    {
        unsigned int RXOICR : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_SPI_RXOICR_T;
typedef union
{
    struct
    {
        unsigned int RXUICR : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_SPI_RXUICR_T;
typedef union
{
    struct
    {
        unsigned int MSTICR : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_SPI_MSTICR_T;
typedef union
{
    struct
    {
        unsigned int ICR : 1;
        unsigned int reserved : 31;
    } bits;
    unsigned int u32;
}HI_SPI_ICR_T;
typedef union
{
    struct
    {
        unsigned int RDMAE : 1;
        unsigned int TDMAE : 1;
        unsigned int PMI0_IND : 30;
    } bits;
    unsigned int u32;
}HI_SPI_DMACR_T;
typedef union
{
    struct
    {
        unsigned int DMATDL : 3;
        unsigned int reserved_1 : 5;
        unsigned int reserved_0 : 24;
    } bits;
    unsigned int u32;
}HI_SPI_DMATDLR_T;
typedef union
{
    struct
    {
        unsigned int DMARDL : 3;
        unsigned int reserved_1 : 5;
        unsigned int reserved_0 : 24;
    } bits;
    unsigned int u32;
}HI_SPI_DMARDLR_T;
typedef union
{
    struct
    {
        unsigned int IDCODE : 32;
    } bits;
    unsigned int u32;
}HI_SPI_IDR_T;
typedef union
{
    struct
    {
        unsigned int SSI_COMP_VERSION : 32;
    } bits;
    unsigned int u32;
}HI_SPI_SSI_COMP_VERSION_T;
typedef union
{
    struct
    {
        unsigned int DR : 16;
        unsigned int reserved : 16;
    } bits;
    unsigned int u32;
}HI_SPI_DR_T;
typedef union
{
    struct
    {
        unsigned int RSD : 8;
        unsigned int reserved : 24;
    } bits;
    unsigned int u32;
}HI_SPI_RX_SAMPLE_DLY_T;
#define hi_spi_three_wire_set(spi_no) do \
{\
    unsigned int reg;\
                \
    reg = readl(HI_PD_CRG_BASE_ADDR+HI_SC_TOP_CTRL6_OFFSET);\
    if(spi_no)\
        reg &= ~0x100;\
    else\
        reg &= ~0x80;\
    writel(reg,HI_PD_CRG_BASE_ADDR+HI_SC_TOP_CTRL6_OFFSET);\
}while(0)
#define hi_spi0_clk_enable() writel(0x1 << 15, HI_PD_CRG_BASE_ADDR + HI_PD_CRG_CLKEN1_OFFSET)
#define hi_spi0_clk_disable() writel(0x1 << 15, HI_PD_CRG_BASE_ADDR + HI_PD_CRG_CLKDIS1_OFFSET)
#define hi_spi1_clk_enable() writel(0x1 << 26, HI_PD_CRG_BASE_ADDR + HI_PD_CRG_CLKEN1_OFFSET)
#define hi_spi1_clk_disable() writel(0x1 << 26, HI_PD_CRG_BASE_ADDR + HI_PD_CRG_CLKDIS1_OFFSET)
#ifdef __cplusplus
}
#endif
#endif
