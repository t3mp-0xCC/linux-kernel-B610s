#ifndef __SOC_CLK_H
#define __SOC_CLK_H 
#include <product_config.h>
#ifdef __cplusplus
extern "C" {
#endif
#define HI_TIMER_STAMP_CLK (32*1024)
#define HI_TCXO_CLK (32*1024)
#define HI_UART_CLK (48000000)
#define HI_HSUART_CLK (48000000)
#define HI_TIMER_CLK (19200000)
#define HI_TIMER_TCXO_CLK TCXO_CLK
#define HI_TIMER_STAMP_CLK (32*1024)
#define HI_SCI0_CLK (60000000)
#define HI_SCI1_CLK (60000000)
#define HI_APB_CLK (25000000)
#define CFG_M3_CLK_CPU (36000000)
#ifdef __cplusplus
}
#endif
#endif
