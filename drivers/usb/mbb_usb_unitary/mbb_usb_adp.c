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

#include "usb_platform_comm.h"
#include "mbb_usb_adp.h"

/** ����USB_unitary ���������ⲿ�ӿ�**/
#ifdef USB_CHARGE
extern int battery_monitor_blocking_notifier_call_chain(
											unsigned long val, unsigned long v);
extern void chg_stm_set_chgr_type(chg_chgr_type_t chgr_type);											
#else

int battery_monitor_blocking_notifier_call_chain(
											unsigned long val, unsigned long v)
{
    return 0;
}
 void chg_stm_set_chgr_type(chg_chgr_type_t chgr_type)
 {
    return 0;
 }

#endif
extern void mbb_usb_register_notify(struct notifier_block *nb);
extern void mbb_usb_unregister_notify(struct notifier_block *nb);
extern struct console* bsp_get_uart_console(void);

/**ע���ⲿ�ӿں���**/

static usb_adp_ctx_t usb_adp_callbacks =
{
    .bsp_get_uart_console_cb = bsp_get_uart_console ,
    .battery_notifier_call_chain_cb = \
                    battery_monitor_blocking_notifier_call_chain,
    .stm_set_chg_type_cb = chg_stm_set_chgr_type,
    .usb_register_notify_cb = mbb_usb_register_notify,
    .usb_unregister_notify_cb = mbb_usb_unregister_notify
};

usb_adp_ctx_t* usb_get_adp_ctx(void)
{
    return &usb_adp_callbacks;
}
#ifdef USB_SD
extern int mmc_get_dev_idex(void);
extern int mmc_get_status(void);
#else
int mmc_get_dev_idex(void)
{
    return 0;
}
int mmc_get_status(void)
{
    return 0;
}
#endif

USB_INT usb_get_mmc_dev_idex(USB_VOID)
{
    return mmc_get_dev_idex();
}

USB_INT usb_get_mmc_status(USB_VOID)
{
    return mmc_get_status();
}

