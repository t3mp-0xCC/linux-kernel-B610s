/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <linux/usb/bsp_usb.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/export.h>
#include <linux/delay.h>
#include <linux/fcntl.h>
#include <linux/namei.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/wakelock.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/usb/phy.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/io.h>

#include <asm/mach/irq.h>

#include "mdrv_usb.h"
#include "drv_nv_id.h"
#include "drv_nv_def.h"
#include "bsp_nvim.h"
#include "bsp_pmu.h"

#include "bsp_dump.h"
#include "bsp_ap_dump.h"

#include "usb_vendor.h"


#ifdef GPIO_0_17
#define USBID_GPIO_NO   (GPIO_0_17)
#else
#define USBID_GPIO_NO   (17)
#endif

//#define USB_FPGA_HOST_CHARGER_BASE 0x9004E000

usb_dbg_info_t g_usb_dbg_info = {{0x0},0,1,0};
//static unsigned g_usb_enum_done_cur = 0;
//static unsigned g_usb_enum_dis_cur = 0;
#ifdef CONFIG_USB_OTG_USBID_BYGPIO
extern int otg_set_usbid(int usb_id);
#else
#define otg_set_usbid
#endif
extern int bsp_usb_is_support_usbid_bygpio(void);
extern int bsp_usb_is_support_vbus_bycharger(void);
extern int usb_prealloc_eth_rx_mem(void);
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
extern int eth_adp_for_spe_init(void);
#endif

/*
 * usb adapter for charger
 */
typedef struct usb_notifer_ctx
{
    int charger_type;
    int usb_status;
    int usb_old_status;
    int usb_hotplub_state;
    int usb_start_inboot;
    int usbid_gpio_no;
    int usbid_gpio_state;
    int cdev_name_pos;
    char* last_cdev_name[USB_CDEV_NAME_MAX];
    unsigned stat_usb_insert;
    unsigned stat_usb_insert_proc;
    unsigned stat_usb_insert_proc_end;
    unsigned stat_usb_enum_done;
    unsigned stat_usb_enum_done_proc;
    unsigned stat_usb_enum_done_proc_end;
    unsigned stat_usb_remove;
    unsigned stat_usb_remove_proc;
    unsigned stat_usb_remove_proc_end;
    unsigned stat_usb_disable;
    unsigned stat_usb_disable_proc;
    unsigned stat_usb_disable_proc_end;
    unsigned stat_usb_no_need_notify;
    unsigned stat_reg_pmu_cb_fail;
    unsigned stat_usb_id_init_fail;
    unsigned stat_usb_perip_insert;
    unsigned stat_usb_perip_insert_proc;
    unsigned stat_usb_perip_insert_proc_end;
    unsigned stat_usb_perip_remove;
    unsigned stat_usb_perip_remove_proc;
    unsigned stat_usb_perip_remove_proc_end;
    unsigned stat_usb_poweroff_fail;
    unsigned stat_usb_poweron_fail;
    unsigned stat_wait_cdev_created;

    struct workqueue_struct *usb_notify_wq;
    struct delayed_work usb_notify_wk;
}usb_notifer_ctx_t;

BLOCKING_NOTIFIER_HEAD(usb_balong_notifier_list);/*lint !e34 !e110 !e156 !e651 !e43*/
static usb_notifer_ctx_t g_usb_notifier;

/*
 * usb enum done management
 */

static usb_enum_stat_t g_usb_devices_enum_stat[USB_ENABLE_CB_MAX];
static unsigned g_usb_dev_enum_done_num;
static unsigned g_usb_dev_setup_num;
//static spinlock_t g_usb_dev_enum_lock;


/*
 * usb enum done management implement
 */
void bsp_usb_init_enum_stat(void)
{
    //spin_lock_init(&g_usb_dev_enum_lock);
    g_usb_dev_enum_done_num = 0;
    g_usb_dev_setup_num = 0;
    memset(g_usb_devices_enum_stat, 0, sizeof(g_usb_devices_enum_stat));
}

/*This will be check after usb enum done. */
void bsp_usb_add_setup_dev_fdname(unsigned intf_id, char* fd_name)
{
    if (g_usb_dev_setup_num >= USB_ENABLE_CB_MAX) {
        printk("%s error, setup_num:%d, USB_ENABLE_CB_MAX:%d\n",
               __FUNCTION__, g_usb_dev_setup_num, USB_ENABLE_CB_MAX);
        return;
    }

    g_usb_devices_enum_stat[g_usb_dev_setup_num].usb_intf_id = intf_id;
    g_usb_devices_enum_stat[g_usb_dev_setup_num].fd_name = fd_name;
    g_usb_dev_setup_num++;
}

void bsp_usb_del_setup_dev(unsigned intf_id)
{
    int i;

    if (g_usb_dev_setup_num <= 0) {
        return;
    }

    for (i = 0; i < USB_ENABLE_CB_MAX; i++) {
        if (g_usb_devices_enum_stat[i].usb_intf_id == intf_id) {
            g_usb_devices_enum_stat[i].usb_intf_id = 0xFFFF;
            g_usb_devices_enum_stat[i].fd_name = NULL;
            g_usb_dev_setup_num--;
            return;
        }
    }
    return;
}


int bsp_usb_is_all_enum(void)
{
    return (g_usb_dev_setup_num != 0 && g_usb_dev_setup_num == g_usb_dev_enum_done_num);
}

int bsp_usb_is_all_disable(void)
{
    return (g_usb_dev_setup_num != 0 && 0 == g_usb_dev_enum_done_num);
}

void bsp_usb_set_enum_stat(unsigned intf_id, int enum_stat)
{
    unsigned int i;
    usb_enum_stat_t *find_dev = NULL;

    if (enum_stat) {
        /* if all dev is already enum, do nothing */
        if (bsp_usb_is_all_enum()) {
            return;
        }

        for (i = 0; i < g_usb_dev_setup_num; i++) {
            if (g_usb_devices_enum_stat[i].usb_intf_id == intf_id) {
                find_dev = &g_usb_devices_enum_stat[i];
            }
        }
        if (find_dev) {
            /* if the dev is already enum, do nothing */
            if (find_dev->is_enum) {
                return;
            }
            find_dev->is_enum = enum_stat;
            g_usb_dev_enum_done_num++;

            /* after change stat, if all dev enum done, notify callback */
            if (bsp_usb_is_all_enum()) {
                bsp_usb_status_change(USB_BALONG_ENUM_DONE);
            }
        }
    }
    else {
        if (bsp_usb_is_all_disable()) {
            return;
        }

        for (i = 0; i < g_usb_dev_setup_num; i++) {
            if (g_usb_devices_enum_stat[i].usb_intf_id == intf_id) {
                find_dev = &g_usb_devices_enum_stat[i];
            }
        }
        if (find_dev) {
            /* if the dev is already disable, do nothing */
            if (!find_dev->is_enum) {
                return;
            }
            find_dev->is_enum = enum_stat;

            /* g_usb_dev_enum_done_num is always > 0,
             * we protect it in check bsp_usb_is_all_disable
             */
            if (g_usb_dev_enum_done_num > 0) {
                g_usb_dev_enum_done_num--;
            }

            /* if the version is not support pmu detect
             * and all the device is disable, we assume that the usb is remove,
             * so notify disable callback, tell the other modules
             * else, we use the pmu remove detect.
             */
            if (!(g_usb_notifier.usb_status == USB_BALONG_DEVICE_REMOVE ||
                  g_usb_notifier.usb_status == USB_BALONG_DEVICE_DISABLE)
                && bsp_usb_is_all_disable()) {
                bsp_usb_status_change(USB_BALONG_DEVICE_DISABLE);
            }
        }
    }

    return;
}

void bsp_usb_enum_info_dump(void)
{
    unsigned int i;

    printk("balong usb is enum done:%d\n", bsp_usb_is_all_enum());
    printk("setup_num:%d, enum_done_num:%d\n",
        g_usb_dev_setup_num, g_usb_dev_enum_done_num);
    printk("device enum info details:\n\n");
    for (i = 0; i < g_usb_dev_setup_num; i++) {
        printk("enum dev:%d\n", i);
        printk("usb_intf_id:%d\n", g_usb_devices_enum_stat[i].usb_intf_id);
        printk("is_enum:%d\n", g_usb_devices_enum_stat[i].is_enum);
        printk("fd_name:%s\n\n", g_usb_devices_enum_stat[i].fd_name);
    }
}


/*
 * usb charger adapter implement
 */
void bsp_usb_status_change(int status)
{
    unsigned long timeout = 0;

    USB_DBG_VENDOR("%s:status %d\n",__FUNCTION__,status);

    if (USB_BALONG_ENUM_DONE == status) {
        g_usb_notifier.stat_usb_enum_done++;
    }
    else if (USB_BALONG_DEVICE_INSERT == status) {
        g_usb_notifier.stat_usb_insert++;
    }
    else if (USB_BALONG_DEVICE_REMOVE == status) {
        g_usb_notifier.stat_usb_remove++;
    }
    else if (USB_BALONG_DEVICE_DISABLE == status) {
        g_usb_notifier.stat_usb_disable++;
    }
    else if (USB_BALONG_PERIP_INSERT == status) {
        g_usb_notifier.stat_usb_perip_insert++;
    }
    else if (USB_BALONG_PERIP_REMOVE == status) {
        g_usb_notifier.stat_usb_perip_remove++;
    }
    else {
        printk("%s: error status:%d\n", __FUNCTION__, status);
    }

    g_usb_notifier.usb_status = status;

    queue_delayed_work(g_usb_notifier.usb_notify_wq,
        &g_usb_notifier.usb_notify_wk, timeout);
}
EXPORT_SYMBOL_GPL(bsp_usb_status_change);

static struct wake_lock g_dwc_wakelock;

void bsp_usb_unlock_wakelock(void)
{
	wake_unlock(&g_dwc_wakelock);
}

void bsp_usb_lock_wakelock(void)
{
	wake_lock(&g_dwc_wakelock);
}

bool bsp_usb_host_state_get(void)
{
#ifdef CONFIG_USB_OTG_USBID_BYGPIO
    usb_notifer_ctx_t *usb_notifier = &g_usb_notifier;
    unsigned int value;

    value = gpio_get_value(usb_notifier->usbid_gpio_no);
    return (0 == value);
#else
    return 0;
#endif
}

static int bsp_usb_state_get(int is_otg)
{
	printk("[USB DBG]otg status %d \n",is_otg);
	if(is_otg){
		return bsp_usb_host_state_get();
	}else{
		if(USB_PMU_DETECT == bsp_usb_vbus_detect_mode()){
			return bsp_pmu_usb_state_get();
		}
		else if(USB_TBOX_DETECT == bsp_usb_vbus_detect_mode()){
			//fixme:add what to do.
			return 0;
		}
		else{
			return 1;
		}
	}
}
int bsp_usb_notifier_status_get(void)
{
    return g_usb_notifier.usb_old_status;
}

#ifdef CONFIG_USB_OTG_USBID_BYGPIO
static irqreturn_t bsp_usb_id_irq(int irq, void *data)
{
    usb_notifer_ctx_t *usb_notifier = (usb_notifer_ctx_t *)data;

    disable_irq_nosync(gpio_to_irq(usb_notifier->usbid_gpio_no));
	return IRQ_WAKE_THREAD;
}

static irqreturn_t bsp_usb_id_thread(int irq, void *data)
{
    usb_notifer_ctx_t *usb_notifier = (usb_notifer_ctx_t *)data;
    unsigned int value = 0;
    unsigned int changed = 0;

    value = gpio_get_value(usb_notifier->usbid_gpio_no);
    changed = (unsigned int)(value != usb_notifier->usbid_gpio_state);

    usb_notifier->usbid_gpio_state = value;
//    gpio_int_unmask_set(usb_notifier->usbid_gpio_no);
    enable_irq(gpio_to_irq(usb_notifier->usbid_gpio_no));

    if(changed){
        USB_DBG_VENDOR("%s:gpio value %d,%s\n",__FUNCTION__,value,changed?"changed":"no change");
        bsp_usb_status_change(value?USB_BALONG_PERIP_REMOVE:USB_BALONG_PERIP_INSERT);
    }

    return IRQ_HANDLED;
}
#endif

static int bsp_usb_id_register(void)
{
#ifdef CONFIG_USB_OTG_USBID_BYGPIO
    usb_notifer_ctx_t *usb_notifier  = &g_usb_notifier;
    int ret;
    int gpio = USBID_GPIO_NO;

    usb_notifier->usbid_gpio_no = USBID_GPIO_NO;

    ret = gpio_request((unsigned)(gpio), "usb_otg");
    if (ret) {
        printk( "Failed to request GPIO %d, error %d\n", gpio, ret);
    }

    ret = gpio_get_value(gpio);
    if (ret)
    {
        usb_notifier->usbid_gpio_state = 1;
    }
    else
    {
        usb_notifier->usbid_gpio_state = 0;
    }

    ret = request_threaded_irq(gpio_to_irq(gpio), bsp_usb_id_irq, bsp_usb_id_thread,
                IRQF_SHARED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "usbid_task", usb_notifier);
    if (ret) {
        return ret;
    }

#endif

    return 0;
}

static void bsp_usb_id_unregister(void)
{
#ifdef CONFIG_USB_OTG_USBID_BYGPIO
    int gpio = USBID_GPIO_NO;
    usb_notifer_ctx_t *usb_notifier  = &g_usb_notifier;

    free_irq(gpio_to_irq(gpio), usb_notifier);
#endif

    return ;
}

static void bsp_usb_pmu_insert_detect(void* ctx)
{
    /* when otg-host supply vbus,pmu intr is obmit */
    if(bsp_usb_is_support_vbus_bycharger()){
        if(USB_BALONG_PERIP_INSERT == bsp_usb_notifier_status_get()){
            return ;
        }
    }

    /* if peripheral is inserted in otg mode,pmu intr is unexpected. */
    if(bsp_usb_host_state_get())
    {
        printk("pmu insert event in otg host mode.\n");
        return ;
    }

    /* need to check whether usb is connected, as the PMU could
     * report the false event during rapid inserting & pluging
     */
    if (!bsp_pmu_usb_state_get()) {
       printk(KERN_WARNING "false insert event from PMU detected\n");
       return;
    }

    printk(KERN_DEBUG "DWC3 USB: pmu insert event received\n");
    bsp_usb_status_change(USB_BALONG_DEVICE_INSERT);
}

static void bsp_usb_pmu_remove_detect(void* ctx)
{
    /* when otg-host supply vbus,pmu intr is obmit */
    if(bsp_usb_is_support_vbus_bycharger()){
        if(USB_BALONG_PERIP_INSERT == bsp_usb_notifier_status_get()){
            return ;
        }
    }

    /* if peripheral is inserted in otg mode,pmu intr is unexpected. */
    if(bsp_usb_host_state_get())
    {
        printk("pmu remove event in otg host mode.\n");
        return ;
    }

    /* need to check whether usb is connected, as the PMU could
     * report the false event during rapid inserting & pluging
     */
    if (bsp_pmu_usb_state_get()) {
       printk(KERN_WARNING "false remove event from PMU detected");
       return;
    }

    printk(KERN_DEBUG "DWC3 USB: pmu remove event received\n");

    /* usb gadget driver will catch the remove event if USB_PMU_DETECT not defined */
    bsp_usb_status_change(USB_BALONG_DEVICE_REMOVE);
}

static void bsp_usb_pmu_detect_init(void)
{
    /* reg usb insert/remove detect callback */
    if (bsp_pmu_irq_callback_register(PMU_INT_USB_IN,
        bsp_usb_pmu_insert_detect, &g_usb_notifier)) {
        g_usb_notifier.stat_reg_pmu_cb_fail++;
    }
    if (bsp_pmu_irq_callback_register(PMU_INT_USB_OUT,
        bsp_usb_pmu_remove_detect, &g_usb_notifier)) {
        g_usb_notifier.stat_reg_pmu_cb_fail++;
    }
}

static int bsp_usb_detect_init(void)
{
    wake_lock_init(&g_dwc_wakelock, WAKE_LOCK_SUSPEND, "dwc3-wakelock");

	/*otg id line register.*/
    if(bsp_usb_id_register()){
        g_usb_notifier.stat_usb_id_init_fail++;
        bsp_usb_id_unregister();
    }

#if IS_ENABLED(CONFIG_USB_DWC3_HOST)
    USB_DBG_VENDOR("usb is on host mode\n");
    return USB_BALONG_PERIP_INSERT;
#endif

    /* check whether the usb is connected in boot time */
    if (bsp_usb_host_state_get()) {
        USB_DBG_VENDOR("usb host is connect in boot, pmu detect disabled\n");
        return USB_BALONG_PERIP_INSERT;
    }

    if(USB_PMU_DETECT == bsp_usb_vbus_detect_mode()){
        bsp_usb_pmu_detect_init();
        if(bsp_pmu_usb_state_get()){
            USB_DBG_VENDOR("usb device is connect in boot, pmu detect enabled\n");
            return USB_BALONG_DEVICE_INSERT;
        }

        USB_DBG_VENDOR("usb is not connect in boot\n");
        return USB_BALONG_DEVICE_REMOVE;
    }
    else if(USB_TBOX_DETECT == bsp_usb_vbus_detect_mode()){
        //fixme:add needed moves.
        return USB_BALONG_DEVICE_INSERT;
    }
    else{
        USB_DBG_VENDOR("usb is always init in boot\n");
        return USB_BALONG_DEVICE_INSERT;
    }
}

static void bsp_usb_insert_process(void)
{
    g_usb_notifier.stat_usb_insert_proc++;
    blocking_notifier_call_chain(&usb_balong_notifier_list,
        USB_BALONG_DEVICE_INSERT, (void*)&g_usb_notifier.charger_type);

#ifdef CONFIG_USB_OTG_USBID_BYGPIO
    otg_set_usbid(1);
#endif

    /* init the usb driver */
    (void)usb_balong_init();

    blocking_notifier_call_chain(&usb_balong_notifier_list,
        USB_BALONG_CHARGER_IDEN, (void*)&g_usb_notifier.charger_type);

    g_usb_notifier.stat_usb_insert_proc_end++;

    return;
}

extern int usb_dwc3_balong_drv_init(void);
extern void usb_dwc3_balong_drv_exit(void);
extern int usb_dwc3_platform_drv_init(void);
extern void usb_dwc3_platform_drv_exit(void);
extern int android_gadget_init(void);
extern void android_gadget_cleanup(void);
extern int dwc3_usb2phy_init(void);
extern void dwc3_usb2phy_exit(void);
extern int dwc3_usb3phy_init(void);
extern void dwc3_usb3phy_exit(void);
extern void sysctrl_set_rx_scope_lfps_en(void);

#ifdef CONFIG_USB_OTG
extern int dwc_otg_init(void);
extern void dwc_otg_exit(void);
#else
int dwc_otg_init(void)
{
    return 0;
}

void dwc_otg_exit(void)
{
    return ;
}
#endif

static int g_is_usb_balong_init = 0;
int usb_balong_init(void)
{
    int ret = 0;
    if (g_is_usb_balong_init) {
        printk("%s:balong usb is already init\n", __FUNCTION__);
        return -EPERM;
    }
	
    bsp_usb_lock_wakelock();

    ret = dwc3_usb2phy_init();
    if(ret){
        printk("%s:dwc3_usb2phy_init fail:%d\n", __FUNCTION__, ret);
        return ret;
    }

    ret = dwc3_usb3phy_init();
    if(ret){
        printk("%s:dwc3_usb3phy_init fail:%d\n", __FUNCTION__, ret);
        goto ret_usb2phy_exit;
    }

    ret = usb_dwc3_balong_drv_init();
    if (ret) {
        printk("%s:usb_dwc3_platform_dev_init fail:%d\n", __FUNCTION__, ret);
        goto ret_usb3phy_exit;
    }

	ret = dwc_otg_init();
    if (ret) {
        printk("%s:dwc_otg_init fail:%d\n", __FUNCTION__, ret);
        goto ret_exit;
    }

    ret = usb_dwc3_platform_drv_init();
    if (ret) {
        printk("%s:usb_dwc3_platform_drv_init fail:%d\n", __FUNCTION__, ret);
        goto ret_otg_exit;
    }
    if(USB_U1U2_ENABLE_WITH_WORKAROUND == bsp_usb_is_enable_u1u2_workaround()){
        
        sysctrl_set_rx_scope_lfps_en();
    }
    if (IS_ENABLED(CONFIG_USB_DWC3_GADGET)
        || IS_ENABLED(CONFIG_USB_DWC3_DUAL_ROLE)){
        ret = android_gadget_init();
        if (ret) {
            printk("%s:android usb init fail:%d\n", __FUNCTION__, ret);
            goto ret_dev_exit;
        }
    }

    g_is_usb_balong_init = 1;
    return 0;
	
ret_dev_exit:		
	usb_dwc3_platform_drv_exit();

ret_otg_exit:
	dwc_otg_exit();
	
ret_exit:
	usb_dwc3_balong_drv_exit();

ret_usb3phy_exit:
    dwc3_usb3phy_exit();
	
ret_usb2phy_exit:
    dwc3_usb2phy_exit();

    g_is_usb_balong_init = 0;

    return ret;
}

void usb_balong_exit(int is_otg)
{
    unsigned long flags;

    if (!g_is_usb_balong_init) {
        printk("%s: balong usb is not init\n", __FUNCTION__);
        return;
    }

    if (IS_ENABLED(CONFIG_USB_DWC3_GADGET)
        || IS_ENABLED(CONFIG_USB_DWC3_DUAL_ROLE)){
        android_gadget_cleanup();
    }

    usb_dwc3_platform_drv_exit();

    dwc_otg_exit();

    usb_dwc3_balong_drv_exit();

    dwc3_usb3phy_exit();

    dwc3_usb2phy_exit();
	
    g_is_usb_balong_init = 0;
    local_irq_save(flags);
    if(!bsp_usb_state_get(is_otg)){
        bsp_usb_unlock_wakelock();
    }
    local_irq_restore(flags);

    printk("%s ok\n", __FUNCTION__);

    return;
}

#ifdef USB_FPGA_HOST_CHARGER_BASE
void usb_vbus_charger_on(void)
{
	void* base;
	unsigned reg;

	base = ioremap(USB_FPGA_HOST_CHARGER_BASE, 4096);
	reg = readl(base + 0x10);
	reg |= 0x8;
	writel( reg, base + 0x10);

	iounmap(base);
}
#endif

static void bsp_usb_perip_insert_process(void)
{
    int status;

    g_usb_notifier.stat_usb_perip_insert_proc++;

    /* set usb notifier status in advance to evade unexpect pmu insert intr. */
    status = g_usb_notifier.usb_old_status;
    g_usb_notifier.usb_old_status = USB_BALONG_PERIP_INSERT;

#ifdef USB_FPGA_HOST_CHARGER_BASE
	if(IS_ENABLED(CONFIG_USB_DWC3_HOST) )
		usb_vbus_charger_on();
#endif

#ifdef CONFIG_USB_OTG_USBID_BYGPIO
        otg_set_usbid(0);
#endif

    /* init the usb driver */
    if(usb_balong_init()){
        g_usb_notifier.usb_old_status = status;
    }

    g_usb_notifier.stat_usb_perip_insert_proc_end++;
    return;
}


void bsp_usb_set_last_cdev_name(char* cdev_name)
{
    if (cdev_name) {
	/* coverity[buffer_size_warning] */
        //strncpy(g_usb_notifier.last_cdev_name, cdev_name, USB_CDEV_NAME_MAX);
        g_usb_notifier.last_cdev_name[g_usb_notifier.cdev_name_pos] = cdev_name;
        g_usb_notifier.cdev_name_pos++;
    }
}

void bsp_usb_clear_last_cdev_name(void)
{
    g_usb_notifier.cdev_name_pos = 0;
}

static void bsp_usb_wait_cdev_created(void)
{
    mm_segment_t old_fs;
    int i = 0;
    char acm_dev_name[USB_CDEV_NAME_MAX];

    /* if there are no cdev, do nothing */
    if (g_usb_notifier.cdev_name_pos == 0) {
        return;
    }
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    while(i < g_usb_notifier.cdev_name_pos) {
        snprintf(acm_dev_name, USB_CDEV_NAME_MAX, "/dev/%s",
                g_usb_notifier.last_cdev_name[i]);
        while (sys_faccessat(AT_FDCWD, acm_dev_name, O_RDWR)) {
            g_usb_notifier.stat_wait_cdev_created++;
            msleep(100);
        }
        i++;
    }
    set_fs(old_fs);
}

static void bsp_usb_enum_done_process(void)
{
    //int loop;

    printk("balong usb enum done,it works well.\n");
    g_usb_notifier.stat_usb_enum_done_proc++;

    /*
     * wait for /dev/xxx created by udev
     */
    bsp_usb_wait_cdev_created();

    /* notify kernel notifier */
    blocking_notifier_call_chain(&usb_balong_notifier_list,
        USB_BALONG_ENUM_DONE, (void*)&g_usb_notifier.charger_type);

    g_usb_notifier.stat_usb_enum_done_proc_end++;
}

static void bsp_usb_remove_device_process(void)
{

    USB_DBG_VENDOR("balong usb remove.\n");
    bsp_usb_clear_last_cdev_name();
    g_usb_notifier.stat_usb_remove_proc++;

    /* notify kernel notifier,
     * we must call notifier list before disable callback,
     * there are something need to do before user
     */
    blocking_notifier_call_chain(&usb_balong_notifier_list,
        USB_BALONG_DEVICE_REMOVE, (void*)&g_usb_notifier.charger_type);


    /* exit the usb driver */
    usb_balong_exit(false);

    g_usb_notifier.charger_type = USB_CHARGER_TYPE_INVALID;
    g_usb_notifier.stat_usb_remove_proc_end++;
		
    return;
}

static void bsp_usb_remove_perip_process(void)
{
    printk("balong usb peripheral remove.\n");
    g_usb_notifier.stat_usb_perip_remove_proc++;

    blocking_notifier_call_chain(&usb_balong_notifier_list,
        USB_BALONG_PERIP_REMOVE, (void*)&g_usb_notifier.charger_type);

    /* exit the usb driver */
    usb_balong_exit(true);

    g_usb_notifier.stat_usb_perip_remove_proc_end++;
    return;
}

static void bsp_usb_disable_device_process(void)
{
    //int loop;

    USB_DBG_VENDOR("balong usb disable.\n");
    bsp_usb_clear_last_cdev_name();
    g_usb_notifier.stat_usb_disable_proc++;

    /* notify kernel notifier */
    blocking_notifier_call_chain(&usb_balong_notifier_list,
        USB_BALONG_DEVICE_DISABLE, (void*)&g_usb_notifier.charger_type);

    g_usb_notifier.charger_type = USB_CHARGER_TYPE_INVALID;
    g_usb_notifier.stat_usb_disable_proc_end++;
    //fix me. not sure if this is needed or not
    //wake_lock_destroy(&g_dwc_wakelock);
    if(bsp_usb_is_sys_err_on_disable()){
        system_error(0, 0, 0, 0, 0);
    }
    return;
}


/**
 * usb_register_notify - register a notifier callback whenever a usb change happens
 * @nb: pointer to the notifier block for the callback events.
 *
 * These changes are either USB devices or busses being added or removed.
 */
void bsp_usb_register_notify(struct notifier_block *nb)
{
	blocking_notifier_chain_register(&usb_balong_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(bsp_usb_register_notify);

/**
 * usb_unregister_notify - unregister a notifier callback
 * @nb: pointer to the notifier block for the callback events.
 *
 * usb_register_notify() must have been previously called for this function
 * to work properly.
 */
void bsp_usb_unregister_notify(struct notifier_block *nb)
{
	blocking_notifier_chain_unregister(&usb_balong_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(bsp_usb_unregister_notify);

int bsp_usb_is_support_charger(void)
{
#if defined(CONFIG_USB_CHARGER_DETECT)
    return 1;
#else
    return 0;
#endif
}

int bsp_usb_is_support_vbus_bycharger(void)
{
#if defined(CONFIG_USB_OTG_VBUS_BYCHARGER)
    return 1;
#else
    return 0;
#endif
}

void bsp_usb_set_charger_type(int type)
{
    g_usb_notifier.charger_type = type;
    return;
}

int bsp_usb_get_charger_type(void)
{
    return g_usb_notifier.charger_type;
}

void bsp_usb_notifier_dump(void)
{
    int i;

    if (USB_CHARGER_TYPE_HUAWEI == g_usb_notifier.charger_type) {
        printk("balong usb CHARGER_TYPE: HuaWei     :) \n");
    }
    else if (USB_CHARGER_TYPE_NOT_HUAWEI  == g_usb_notifier.charger_type) {
        printk("balong usb CHARGER_TYPE: Not HuaWei :( \n");
    }
    else {
        printk("balong usb CHARGER_TYPE: Invalid    +_+ \n");
    }
    printk("usb_start_inboot:           %d\n", g_usb_notifier.usb_start_inboot);
    printk("stat_usb_insert:            %d\n", g_usb_notifier.stat_usb_insert);
    printk("stat_usb_insert_proc:       %d\n", g_usb_notifier.stat_usb_insert_proc);
    printk("stat_usb_insert_proc_end:   %d\n", g_usb_notifier.stat_usb_insert_proc_end);
    printk("stat_usb_enum_done:         %d\n", g_usb_notifier.stat_usb_enum_done);
    printk("stat_usb_enum_done_proc:    %d\n", g_usb_notifier.stat_usb_enum_done_proc);
    printk("stat_usb_enum_done_proc_end:%d\n", g_usb_notifier.stat_usb_enum_done_proc_end);
    printk("stat_usb_remove:            %d\n", g_usb_notifier.stat_usb_remove);
    printk("stat_usb_remove_proc:       %d\n", g_usb_notifier.stat_usb_remove_proc);
    printk("stat_usb_remove_proc_end:   %d\n", g_usb_notifier.stat_usb_remove_proc_end);
    printk("stat_usb_disable:           %d\n", g_usb_notifier.stat_usb_disable);
    printk("stat_usb_disable_proc:      %d\n", g_usb_notifier.stat_usb_disable_proc);
    printk("stat_usb_disable_proc_end:  %d\n", g_usb_notifier.stat_usb_disable_proc_end);

    printk("stat_usb_perip_insert:      %d\n", g_usb_notifier.stat_usb_perip_insert);
    printk("stat_usb_perip_insert_proc: %d\n", g_usb_notifier.stat_usb_perip_insert_proc);
    printk("stat_usb_perip_insert_proc_end: %d\n", g_usb_notifier.stat_usb_perip_insert_proc_end);
    printk("stat_usb_perip_remove:      %d\n", g_usb_notifier.stat_usb_perip_remove);
    printk("stat_usb_perip_remove_proc: %d\n", g_usb_notifier.stat_usb_perip_remove_proc);
    printk("stat_usb_perip_remove_proc_end: %d\n", g_usb_notifier.stat_usb_perip_remove_proc_end);

    printk("usb_status:                 %d\n", g_usb_notifier.usb_status);
    printk("usb_old_status:             %d\n", g_usb_notifier.usb_old_status);
    printk("usb_hotplub_state:          %d\n", g_usb_notifier.usb_hotplub_state);
    printk("stat_wait_cdev_created:     %d\n", g_usb_notifier.stat_wait_cdev_created);
    for (i = 0; i < g_usb_notifier.cdev_name_pos; i++) {
        printk("last_cdev_name[%d]:          %s\n", i, g_usb_notifier.last_cdev_name[i]);
    }
    printk("stat_reg_pmu_cb_fail:       %d\n", g_usb_notifier.stat_reg_pmu_cb_fail);
    printk("stat_usb_id_init_fail:      %d\n", g_usb_notifier.stat_usb_id_init_fail);
    printk("stat_usb_poweroff_fail:     %d\n", g_usb_notifier.stat_usb_poweroff_fail);
    printk("stat_usb_poweron_fail:      %d\n", g_usb_notifier.stat_usb_poweron_fail);
    printk("stat_usb_no_need_notify:    %d\n", g_usb_notifier.stat_usb_no_need_notify);
}

int usb_notifier_mem_dump(char* buffer, unsigned int buf_size)
{
    unsigned int need_size = sizeof(g_usb_notifier);
	char* cur = buffer;

    if (need_size + 8 > buf_size) {
        return -1;
    }
    *((int*)cur) = 1;
    cur += sizeof(int);
	*((int*)cur) = sizeof(g_usb_notifier);
	cur += sizeof(int);

    memcpy(cur, &g_usb_notifier, need_size);
    return (int)need_size + 8;
}

static void usb_notify_handler(struct work_struct *work)
{
    int cur_status = g_usb_notifier.usb_status;

	USB_DBG_VENDOR("%s:old_status %d,cur_status%d\n",
		__FUNCTION__,g_usb_notifier.usb_old_status,cur_status);

    if (g_usb_notifier.usb_old_status == cur_status) {
        g_usb_notifier.stat_usb_no_need_notify++;
        return;
    }

    USB_DBG_VENDOR("%s:task of %d start.\n",__FUNCTION__,cur_status);

    switch (cur_status) {
    case USB_BALONG_DEVICE_INSERT:
        if(USB_BALONG_HOTPLUG_INSERT
           == g_usb_notifier.usb_hotplub_state){
           USB_DBG_VENDOR("%s:task of %d cancel.\n",__FUNCTION__,cur_status);
           return ;
        }
        g_usb_notifier.usb_hotplub_state = USB_BALONG_HOTPLUG_INSERT;
        bsp_usb_insert_process();
        break;
    case USB_BALONG_PERIP_INSERT:
        bsp_usb_perip_insert_process();
        break;
    case USB_BALONG_ENUM_DONE:
        bsp_usb_enum_done_process();
        break;
    case USB_BALONG_DEVICE_REMOVE:
        if(USB_BALONG_HOTPLUG_REMOVE
           == g_usb_notifier.usb_hotplub_state){
           USB_DBG_VENDOR("%s:task of %d cancel.\n",__FUNCTION__,cur_status);
           return ;
        }
        g_usb_notifier.usb_hotplub_state = USB_BALONG_HOTPLUG_REMOVE;
        bsp_usb_remove_device_process();
        break;
    case USB_BALONG_PERIP_REMOVE:
        bsp_usb_remove_perip_process();
        break;
    case USB_BALONG_DEVICE_DISABLE:
        bsp_usb_disable_device_process();
        break;
    default:
        printk("%s, invalid status:%d\n",
            __FUNCTION__, cur_status);
        return;
    }

    USB_DBG_VENDOR("%s:task of %d end.\n",__FUNCTION__,cur_status);

    g_usb_notifier.usb_old_status = cur_status;
    return;
}

static int bsp_usb_notifier_init(void)
{
    /* init local ctx resource */
    g_usb_notifier.charger_type = USB_CHARGER_TYPE_INVALID;
    g_usb_notifier.usb_notify_wq = create_singlethread_workqueue("usb_notify");
    if (!g_usb_notifier.usb_notify_wq) {
        printk("%s: create_singlethread_workqueue fail\n", __FUNCTION__);
        return -ENOMEM;
    }
	INIT_DELAYED_WORK(&g_usb_notifier.usb_notify_wk, (void *)usb_notify_handler);
	return 0;
}

void bsp_usb_dbg_init(void)
{
    g_usb_dbg_info.nv_key = USB_DBG_NV;
    if(g_usb_dbg_info.use_nv){
        if (bsp_nvm_read(USB_DBG_NV,
            (u8*)&g_usb_dbg_info.dbg_info, sizeof(g_usb_dbg_info.dbg_info))) {
            g_usb_dbg_info.stat_nv_read_fail++;
        }
    }
}

void bsp_usb_dbg_dump(void)
{
    printk("usb dbg dump info       :\n");
    printk("usb dbg module          :0x%08x\n",g_usb_dbg_info.dbg_info.dbg_module);
    printk("usb dbg nv_key          :0x%08x\n",g_usb_dbg_info.nv_key);
    printk("usb nv read fail count  :%d\n",g_usb_dbg_info.stat_nv_read_fail);
}

void bsp_usb_dbg_set(u32 value)
{
    g_usb_dbg_info.dbg_info.dbg_module = value;
}

/* NV structure */
typedef NV_USB_FEATURE usb_feature_t;

typedef struct
{
    /* usb nv structure ... */
    usb_feature_t usb_feature;
    unsigned int   shell_lock;

    /* stat counter */
    unsigned int stat_nv_read_fail;
    unsigned int stat_nv_init;
} usb_nv_t;

static usb_nv_t g_usb_nv;

/*
 * usb adapter for NV
 */
#define USB_WWAN_FLAG 0x01
#define USB_BYPASS_MODE 0x02
#define USB_VBUS_CONNECT 0x04
#define USB_ERR_ON_REMNUM 0x08
#define USB_AUTO_PD_PHY 0x10

static inline void bsp_usb_nv_default_init(void)
{
    memset(&g_usb_nv, 0, sizeof(usb_nv_t));
}

int bsp_usb_is_support_wwan(void)
{
    return (int)(!!(g_usb_nv.usb_feature.flags&USB_WWAN_FLAG));
}
int bsp_usb_is_ncm_bypass_mode(void)
{
#ifdef CONFIG_BALONG_NCM
    return (int)(!(g_usb_nv.usb_feature.flags&USB_BYPASS_MODE));
#else
    /*Only ncm support bypass mode other usb net card can only use e5 mode*/
    return 0;
#endif
}

/*vbus connect to chip*/
int bsp_usb_is_vbus_connect(void)
{
    return (int)(!!(g_usb_nv.usb_feature.flags&USB_VBUS_CONNECT));
}

/*0=pmu detect; 1=no detect(for fpga); 2=car module vbus detect */
int bsp_usb_vbus_detect_mode(void)
{
    int vbus_detect_mode = (int)(g_usb_nv.usb_feature.detect_mode);
	
    if(bsp_usb_is_vbus_connect()){
        if(USB_PMU_DETECT == vbus_detect_mode){
            vbus_detect_mode = USB_NO_DETECT;
            printk("USB vbus detect NV setting error. \n");
        }
    }
    else{
        if(USB_TBOX_DETECT == vbus_detect_mode){
            vbus_detect_mode = USB_NO_DETECT;
            printk("USB vbus detect NV setting error. \n");
        }
    }
    return vbus_detect_mode;
}

int bsp_usb_is_support_phy_apd(void)
{
    return (int)(!!(g_usb_nv.usb_feature.flags&USB_AUTO_PD_PHY));
}

int bsp_usb_is_support_hibernation(void)
{
    return (int)(g_usb_nv.usb_feature.hibernation_support);
}

u16 bsp_usb_pc_driver_id_get(void)
{
    u16 pid = 0x1506;
    switch(g_usb_nv.usb_feature.pc_driver){
    case JUNGO_DRIVER:
    case HUAWEI_PC_DRIVER:
        pid = 0x1506;
        break;
    case HUAWEI_MODULE_DRIVER:
        pid = 0x15c1;
        break;
    default:
        break;
    }    
    return pid;
}

u8 bsp_usb_pc_driver_subclass_get(void)
{
    u8 subclass = 0x02;
    switch(g_usb_nv.usb_feature.pc_driver){
    case JUNGO_DRIVER:
        subclass = 0x02;
        break;
    case HUAWEI_PC_DRIVER:
        subclass = 0x03;
        break;
    case HUAWEI_MODULE_DRIVER:
        subclass = 0x06;
        break;
    default:
        break;
    }    
    return subclass;
}

int bsp_usb_is_sys_err_on_disable(void)
{
    return (int)(!!(g_usb_nv.usb_feature.flags&USB_ERR_ON_REMNUM));
}

/*0=disable u1 u2 
   1=enable u1 u2 with workaround
   2=enable u1 u2 without workaround 
*/
int bsp_usb_is_enable_u1u2_workaround(void)
{
    return (int)(g_usb_nv.usb_feature.enable_u1u2_workaround);
}
  
int bsp_usb_is_support_shell(void)
{
    int ret;

    if (g_usb_nv.shell_lock == 0) {
        ret = 1;
    } else {
        ret = 0;
    }
    return ret;
}

void bsp_usb_nv_dump(void)
{
    printk("usb read nv fail count: %d\n", g_usb_nv.stat_nv_read_fail);
    printk("features:        %x\n", g_usb_nv.usb_feature.flags);
    printk("is shell lock:          %d\n", g_usb_nv.shell_lock);
    printk("pc driver:              %d\n", g_usb_nv.usb_feature.pc_driver);
    return;
}

void bsp_usb_nv_init(void)
{
    if(g_usb_nv.stat_nv_init)
        return;
    
    /* init the default value */
    bsp_usb_nv_default_init();

    /*
     * read needed usb nv items from nv
     * ignore the nv_read return value,
     * if fail, use the default value
     */
    if (bsp_nvm_read(NV_ID_DRV_USB_FEATURE,
        (u8*)&g_usb_nv.usb_feature, sizeof(g_usb_nv.usb_feature))) {
        g_usb_nv.stat_nv_read_fail++;
    }
    if (bsp_nvm_read(NV_ID_DRV_AT_SHELL_OPNE,
        (u8*)&g_usb_nv.shell_lock, sizeof(g_usb_nv.shell_lock))) {
        g_usb_nv.stat_nv_read_fail++;
    }

    g_usb_nv.stat_nv_init++;
}
 /*  * usb adapter for NV END*/

#if 0
void usb_test_enable_cb(void)
{
    struct file* filp;

    filp = filp_open("/dev/ttyGS0", O_RDWR, 0);
    if (IS_ERR(filp)) {
        printk("open dev fail: %d\n", (u32)filp);
        return;
    }
    printk("open dev ok\n");
}
#endif

/*
 * for usb om
 */
extern int perf_caller_mem_dump(char* buffer, unsigned int buf_size);
extern int acm_cdev_mem_dump(char* buffer, unsigned int buf_size);
extern int acm_serial_mem_dump(char* buffer, unsigned int buf_size);
extern int ncm_eth_mem_dump(char* buffer, unsigned int buf_size);


typedef int(*usb_adp_dump_func_t)(char* buffer, unsigned int buf_size);

struct usb_dump_func_info {
    char* mod_name;
    usb_adp_dump_func_t dump_func;
};

struct usb_dump_info_ctx {
    char* dump_buffer;
    char* cur_pos;
    unsigned int buf_size;
    unsigned int left;
};


/* mod_name must be 4 bytes, we just copy 4 bytes to dump buffer */
struct usb_dump_func_info usb_dump_func_tb[] = {
//    {"perf", perf_caller_mem_dump},
    {"ntfy", usb_notifier_mem_dump},
    {"acmc", acm_cdev_mem_dump},
    {"acms", acm_serial_mem_dump},
    {"ncm_", ncm_eth_mem_dump},

    {"last", NULL}
};

#define USB_ADP_DUMP_FUNC_NUM \
    (sizeof(usb_dump_func_tb) / sizeof(struct usb_dump_func_info) - 1)

#define USB_ADP_GET_4BYTE_ALIGN(size) (((size) + 3) & (~0x3))

struct usb_dump_info_ctx usb_dump_ctx = {0};


void bsp_usb_adp_dump_hook(void)
{
    unsigned int i;
    int ret;
    char* cur_pos = usb_dump_ctx.dump_buffer;
    unsigned int left = usb_dump_ctx.buf_size;

    /* do nothing, if om init fail */
    if (0 == left || NULL == cur_pos)
        return;

    for (i = 0; i < USB_ADP_DUMP_FUNC_NUM; i++) {
        if (left <= 4) {
            break;
        }
        /* mark the module name, just copy 4 bytes to dump buffer */
        memcpy(cur_pos, usb_dump_func_tb[i].mod_name, 4);
        cur_pos += 4;
        left -= 4;

        if (usb_dump_func_tb[i].dump_func) {
            ret = usb_dump_func_tb[i].dump_func(cur_pos, left);
            /* may be no room left */
            if (ret < 0) {
                break;
            }

            /* different module recorder addr must be 4bytes align */
            ret = USB_ADP_GET_4BYTE_ALIGN(ret);
            cur_pos += ret;
            left -= ret;
        }
        /* NULL func is the last */
        else {
            break;
        }
    }

    usb_dump_ctx.cur_pos = cur_pos;
    usb_dump_ctx.left = left;
    return;
}

void bsp_usb_om_dump_init(void)
{
    /* reg the dump callback to om */
	if (-1 == bsp_ap_dump_register_hook("USB", (dump_hook)bsp_usb_adp_dump_hook)) {
	    goto err_ret;
	}

    usb_dump_ctx.dump_buffer = bsp_ap_dump_register_field(DUMP_KERNEL_USB, "USB", 0, 0, 0x2000, 0);
	if (NULL == usb_dump_ctx.dump_buffer) {
	    goto err_ret;
	}

    usb_dump_ctx.buf_size = 0x2000;

    return;

err_ret:
    usb_dump_ctx.dump_buffer = NULL;
    usb_dump_ctx.buf_size = 0;
    return;
}

void bsp_usb_om_dump_info(void)
{
    printk("buf_size:               %d\n", usb_dump_ctx.buf_size);
    printk("left:                   %d\n", usb_dump_ctx.left);
    printk("dump_buffer:            0x%x\n", (unsigned)usb_dump_ctx.dump_buffer);
    printk("cur_pos:                0x%x\n", (unsigned)usb_dump_ctx.cur_pos);
    return;
}
/*end of usb om*/

/* we don't need exit for adapter module */
int __init bsp_usb_adapter_init(void)
{
    int ret = 0;
    int insert_status = 0;

    ret = bsp_usb_notifier_init();
    if (ret) {
        return ret;
    }

    ret = usb_prealloc_eth_rx_mem();
    if(ret){
        printk("usb_prealloc_eth_rx_mem fail\n");

        return ret;
    }

    /* init usb needed nv */
    bsp_usb_nv_init();

#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
    if (0 > eth_adp_for_spe_init()) {
        printk("ncm spe adp init failed\n");
    }
    printk("ncm spe port alloc suscess\n");
#endif

    /* init om dump for usb */
    bsp_usb_om_dump_init();

    /* init detect */
    insert_status = bsp_usb_detect_init();

    /* if usb is connect, init the usb core */
    if (insert_status) {
        g_usb_notifier.usb_start_inboot = 1;
        bsp_usb_status_change(insert_status);
    }
    printk("%s line %d\n", __FUNCTION__, __LINE__);
    return ret;
}

module_init(bsp_usb_adapter_init);
