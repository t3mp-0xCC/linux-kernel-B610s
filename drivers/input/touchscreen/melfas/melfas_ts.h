/*********************************************************************
 *
 * Melfas MCS6000 Touchscreen Controller Driver
 *
 *********************************************************************/

/*********************************************************************
 * include/linux/melfas_ts.h - platform data structure for MCS Series sensor
 *
 * Copyright (C) 2010 Melfas, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *********************************************************************/

#ifndef _LINUX_MELFAS_TS_H
#define _LINUX_MELFAS_TS_H

#define MELFAS_TS_NAME "melfas-ts"

#include <linux/gpio.h>

#define MELFAS_TOUCH_ADDR       (0x23)/*i2c slave address*/
#define MELFAS_POWER_UP_TIME    (10)/*TP power up need 20ms*/
#define MELFAS_TP_UNRESET_TIME    (130)/*TP power up need >= 120ms */



struct melfas_touch_platform_data {
    unsigned gpio_en;
    unsigned gpio_int;
    unsigned gpio_reset;
    unsigned gpio_sck;
    unsigned gpio_sda;
    int (*gpio_config)(unsigned interrupt_gpio, bool configure);

};


#endif /* _LINUX_MELFAS_TS_H */
