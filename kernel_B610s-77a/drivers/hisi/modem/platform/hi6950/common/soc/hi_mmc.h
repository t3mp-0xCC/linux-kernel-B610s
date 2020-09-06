#ifndef __HI_MMC_H__
#define __HI_MMC_H__ 
#include "product_config.h"
#define HI6950_MMC 
#define MMC_CLOCK_SOURCE (800000000)
#if (FEATURE_ON == MBB_COMMON)
#define MMC0_CAPS (MMC_CAP_4_BIT_DATA | MMC_CAP_SDIO_IRQ \
                          | MMC_CAP_ERASE | MMC_CAP_SD_HIGHSPEED\
                          | MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 \
                          | MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_SDR104)
#else
#define MMC0_CAPS (MMC_CAP_4_BIT_DATA | MMC_CAP_SDIO_IRQ \
                          | MMC_CAP_NONREMOVABLE | MMC_CAP_SD_HIGHSPEED\
                          | MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 \
                          | MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_SDR104)
#endif
#define MMC1_CAPS (MMC_CAP_DRIVER_TYPE_A | MMC_CAP_4_BIT_DATA \
                          | MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED \
                          | MMC_CAP_UHS_SDR104)
#define MMC_BIU_ENUM_CLOCK (20000000)
#endif
