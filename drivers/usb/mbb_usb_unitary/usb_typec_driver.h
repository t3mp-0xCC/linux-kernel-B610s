/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _USB_TYPEC_DRIVER_H
#define _USB_TYPEC_DRIVER_H
typedef enum __hw_usbc_type
{
    TYPEC_UNATTACH = -1,
    TYPEC_UFP = 0,
    TYPEC_DFP,
    TYPEC_DRP,
    TYPEC_ACC,
    TYPEC_DEBUG,
    TYPEC_MAX,
} USBC_TYPE;
int usbc_i2c_hotplug(int action);
int usbc_check_host_mode(void);

int usbc_try_sink_attach(void);
void usbc_try_sink_init();
int usbc_get_irq_flag();
#endif