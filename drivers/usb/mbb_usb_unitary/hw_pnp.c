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

#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/usb/gadget.h>
#include <linux/delay.h>
#include <linux/list.h>
 
#include <linux/usb/usb_interface_external.h>
#include "android.h"
#include "hw_pnp.h"
#include "hw_pnp_adapt.h"
#include "hw_pnp_api.h"
#include "usb_nv_get.h"
#include "f_mass_storage_api.h"
#include "usb_fast_on_off.h"
#include "usb_event.h"
#include "usb_smem.h"

#define MBB_USB_SWITCH_DELAY_MS 100/*延迟切换时间，单位ms*/

/*the string descriptor of device */
static char manufacturer_string[MAX_STRING_DESC_NUM];
static char product_string[MAX_STRING_DESC_NUM];
static char serial_string[MAX_STRING_DESC_NUM];
static char MSOS_descriptor_string[] =
{
    USB_NUM_18, /* sizeof(ms_os_string) */
    USB_DT_STRING,
    /* Signature field: "MSFT100" */
    'M', 0, 'S', 0, 'F', 0, 'T', 0, '1', 0, '0', 0, '0', 0,
    /* vendor code */
    MS_HUAWEI_VENDOR_CODE,
    /* padding */
    0
};

static struct usb_device_descriptor device_desc =
{
    .bLength              = sizeof(device_desc),
    .bDescriptorType      = USB_DT_DEVICE,
    .bcdUSB               = __constant_cpu_to_le16(0x0200),
    .bDeviceClass         = USB_CLASS_PER_INTERFACE,
    .idVendor             = __constant_cpu_to_le16(VENDOR_ID_HUAWEI),
    .idProduct            = __constant_cpu_to_le16(CDROM_SCENE_PID),
#ifdef MBB_USB_UNITARY_Q
    .bcdDevice            = __constant_cpu_to_le16(USB_BCDDEVICE_QC),
#else
    .bcdDevice            = __constant_cpu_to_le16(USB_BCDDEVICE_BALONG),
#endif
    .bDeviceProtocol      = 0x00,
    .bDeviceSubClass      = 0x00,
    .bNumConfigurations   = 1,
};

static struct usb_otg_descriptor otg_descriptor =
{
    .bLength         = sizeof otg_descriptor,
    .bDescriptorType = USB_DT_OTG,
    .bmAttributes    = USB_OTG_SRP | USB_OTG_HNP,
    .bcdOTG          = __constant_cpu_to_le16(0x0200),
};

static const struct usb_descriptor_header* otg_desc[] =
{
    (struct usb_descriptor_header*)& otg_descriptor,
    NULL,
};

/* String Table */
static struct usb_string strings_dev[] =
{
    [STRING_MANUFACTURER_IDX].s = manufacturer_string,
    [STRING_PRODUCT_IDX].s = product_string,
    [STRING_SERIAL_IDX].s = serial_string,
    {}
};

static struct usb_gadget_strings stringtab_dev =
{
    .language = 0x0409, /* en-us */
    .strings  = strings_dev,
};

static struct usb_gadget_strings* dev_strings[] =
{
    &stringtab_dev,
    NULL,
};

/*****************************************************************************
内部接口 begin{
*****************************************************************************/
USB_VOID pnp_probe(USB_VOID);
USB_VOID pnp_remove(USB_VOID);

USB_VOID pnp_switch_normal_work_mode(USB_VOID);
USB_VOID pnp_switch_mbim_debug_mode(USB_VOID);
USB_VOID pnp_switch_rndis_debug_mode(USB_VOID);
USB_VOID pnp_switch_rndis_project_mode(USB_VOID);
USB_VOID pnp_switch_autorun_port(USB_VOID);
USB_VOID pnp_switch_charge_only_port(USB_VOID);
USB_VOID pnp_usb_queue_delay_work(struct delayed_work* dwork, unsigned long delay);
hw_usb_mode* pnp_static_port_get_mode(USB_UCHAR   index );

usb_enum_state huawei_get_usb_enum_state( USB_VOID );

static USB_VOID pnp_free_android_config(struct android_dev* dev,
                                        struct android_configuration* conf);
USB_INT pnp_huawei_vendor_setup(struct usb_gadget* gadget,
                                const struct usb_ctrlrequest* ctrl);

pnp_dynamic_mode* pnp_d_port_get_mode(USB_UCHAR port_index);
USB_VOID pnp_notify_to_switch(USB_VOID);
USB_VOID pnp_switch_func(struct work_struct* w);

USB_VOID pnp_d_port_add_info(
    pnp_dynamic_mode* port_mode, USB_CHAR* new_name, USB_UINT8 prot_procotol);
USB_VOID pnp_config_dev(USB_VOID);
USB_VOID pnp_d_port_get_info_from_NV(USB_INT portindex);
USB_VOID pnp_d_port_clear_mode(USB_UCHAR port_index);
USB_CHAR* pnp_d_port_case_port_name(USB_CHAR* case_name);
USB_VOID pnp_d_port_mode_default(pnp_dynamic_mode* dynamic_port_mode);
USB_UINT8 pnp_d_port_get_port_protocol(USB_CHAR* fd_name);
USB_VOID pnp_set_ctl_app_flag(USB_INT flag);

static int android_bind(struct usb_composite_dev* cdev);
static int android_setup(struct usb_gadget* gadget, const struct usb_ctrlrequest* c);
static USB_INT android_usb_unbind(struct usb_composite_dev* cdev);
static void android_disconnect(struct usb_composite_dev* cdev);
static void android_suspend(struct usb_composite_dev* cdev);
static void android_resume(struct usb_composite_dev* cdev);
int Check_EnablePortName( USB_CHAR* PsBuffer, USB_ULONG* Length  );
USB_CHAR* pnp_get_serial_num(USB_VOID);
USB_INT pnp_unique_serial_num_flag(USB_VOID);
USB_VOID pnp_dump(USB_VOID);
USB_VOID pnp_notify_to_switch(USB_VOID);
USB_VOID pnp_switch_ndis_on_off(USB_VOID);



/*****************************************************************************
}  内部接口 end
*****************************************************************************/

static struct usb_composite_driver pnp_usb_driver =
{
    .name       = "android_usb",
    .dev        = &device_desc,
    .strings    = dev_strings,
    .bind       = android_bind,
    .setup      = android_setup,
    .unbind     = android_usb_unbind,
    .disconnect = android_disconnect,
    .resume     = android_resume,
    .suspend    = android_suspend,
    .max_speed  = USB_SPEED_HIGH,
};

/*MBIM name管理*/
USB_PCHAR config_funcs_win8[CONFIG_TWO_MAX_PORT_NUM] = {USB_MBIM_FUNC_NAME, NULL};

static struct delayed_work  pnp_switch_work;

static struct delayed_work  pnp_notify_rewind_after_work;

static struct delayed_work  pnp_remove_work;

int setmode_state = SETMODE_UNKNOWN_STATE;
struct delayed_work setmode_work;

static huawei_dynamic_info_st  g_pnp_info  = {0};

static int functions_store_enable = 0;

pnp_dynamic_mode g_pnp_dynamic_port_mode_info[] =
{
    {
        DYNAMIC_REWIND_BEFORE_MODE,
        {NULL},
        {USB_IF_PROTOCOL_VOID},
        CDROM_SCENE_PID
    },
    {
        DYNAMIC_REWIND_AFTER_MODE,
        {NULL},
        {USB_IF_PROTOCOL_VOID},
        CDROM_SCENE_PID
    },
    {
        DYNAMIC_DIRECT_PORT_MODE,
        {NULL},
        {USB_IF_PROTOCOL_VOID},
        DYNAMIC_PID
    },
    {
        DYNAMIC_DEBUG_MODE,
        {NULL},
        {USB_IF_PROTOCOL_VOID},
        DYNAMIC_DEBUG_PID
    },
    {
        HILINK_PORT_MODEM_MODE,
        {"modem", "pcui"},
        {USB_IF_PROTOCOL_MODEM, USB_IF_PROTOCOL_PCUI},
        DYNAMIC_PID
    },
    {
        HILINK_PORT_MODEM_DEBUG_MODE,
        {"modem", "pcui", "diag", "adb"},
        {USB_IF_PROTOCOL_MODEM, USB_IF_PROTOCOL_PCUI, USB_IF_PROTOCOL_DIAG, USB_IF_PROTOCOL_VOID},
        DYNAMIC_PID
    },
};

/*****************************************************************************
android 设备驱动功能 begin{
*****************************************************************************/

/*****************************************************************
Parameters    :  dev
                 c
Return        :
Description   :  将端口形态绑定到设备
*****************************************************************/
static USB_INT android_bind_enabled_functions(struct android_dev* dev,
        struct usb_configuration* c)
{
    struct android_usb_function* f;
    struct android_configuration* conf = NULL;
    USB_INT ret = -1;
    
    if ( NULL == dev || NULL == c)
    {
        DBG_E(MBB_PNP, "dev /%p OR c/%p NULL !\n",dev,c );
        return ret;
    }
    
    conf = container_of(c, struct android_configuration, usb_config);
    
    list_for_each_entry(f, &conf->enabled_functions, enabled_list)
    {
        if( NULL != f->bind_config )
        {
            DBG_I(MBB_PNP, "bind '%-10s'\n", f->name);
            ret = f->bind_config(f, c);
            if (ret)
            {
                DBG_E(MBB_PNP, "conf %d: %s failed\n",
                      conf->usb_config.bConfigurationValue, f->name);
                return ret;
            }
        }
        else
        {
            DBG_I(MBB_PNP, " 'f_%s'->bind_config  NULL\n", f->name);
        }
        ret = 0;
    }
    return ret;
}

/*****************************************************************
Parameters    :  dev
                 c
Return        :
Description   :  将端口形态从设备上去绑定
*****************************************************************/
static void
android_unbind_enabled_functions(struct android_dev* dev,
                                 struct usb_configuration* c)
{
    struct android_usb_function *f, *f_next;
    struct android_configuration* conf = NULL;
    
    if ( NULL == dev || NULL == c)
    {
        DBG_E(MBB_PNP, "dev /%p OR c/%p NULL !\n",dev,c );
        return ;
    }
    conf = container_of(c, struct android_configuration, usb_config);
#ifdef  MBB_USB_RESERVED
    list_for_each_entry(f, &conf->enabled_functions, enabled_list)
    {
        DBG_T(MBB_PNP, "unbind android_usb_func '%-10s'\n", f->name);
        if (f->unbind_config)
        {
            f->unbind_config(f, c);
        }
    }
#endif

    list_for_each_entry_safe(f, f_next, &conf->enabled_functions, enabled_list)
    {
        if (f->unbind_config)
        {
            DBG_T(MBB_PNP, "unbind android_usb_func '%-10s'\n", f->name);
            f->unbind_config(f, c);
        }
        list_del(&f->enabled_list);
        kfree(f);
    }
}

/*****************************************************************
Parameters    :  c
Return        :
Description   :  设备绑定config
*****************************************************************/
static int android_bind_config(struct usb_configuration* c)
{
    struct android_dev* dev = android_get_android_dev();
    int ret = -1;

    ret = android_bind_enabled_functions(dev, c);

    if (ret)
    { return ret; }

    return 0;
}

/*****************************************************************
Parameters    :  c
Return        :
Description   :  设备去绑定config
*****************************************************************/
static void android_unbind_config(struct usb_configuration* c)
{
    struct android_dev* dev = android_get_android_dev();

    android_unbind_enabled_functions(dev, c);
}

/*****************************************************************
Parameters    :  dev
Return        :
Description   :  启动设备
*****************************************************************/
static int android_enable(struct android_dev* dev)
{
    struct usb_composite_dev* cdev = dev->cdev;
    struct android_configuration* conf;
    int err = -1;

    if (WARN_ON(!dev->disable_depth))
    { return err; }

    if (dev->disable_depth == 1)
    {
        DBG_I(MBB_PNP, "start android_enable\n");
        dev->disable_depth = 0;
        list_for_each_entry(conf, &dev->android_configs, list_item)
        {
            err = usb_add_config(cdev, &conf->usb_config,
                                 android_bind_config);

            if (err < 0)
            {
                DBG_E(MBB_PNP, "usb_add_config failed : err: %d\n", err);
                return err;
            }
        }
    }

    return err;
}

/*****************************************************************
Parameters    :  dev
Return        :
Description   :  关闭设备
*****************************************************************/
void android_disable(struct android_dev* dev)
{
    struct usb_composite_dev* cdev = dev->cdev;
    struct android_configuration* conf;
    struct list_head* curr_conf = &dev->android_configs;

    if (dev->disable_depth == 0)
    {
        dev->disable_depth = 1;
        DBG_I(MBB_PNP, "start disable process\n");

        usb_gadget_disconnect(cdev->gadget);
        /* Cancel pending control requests */
        usb_ep_dequeue(cdev->gadget->ep0, cdev->req);

        list_for_each_entry(conf, &dev->android_configs, list_item)
        {
            DBG_I(MBB_PNP, "remove config/%p!\n", &conf->usb_config);
            /*防止重复删除*/
            if(LIST_POISON1 != conf->usb_config.list.next)
            {
                usb_remove_config(cdev, &conf->usb_config);
            }
        }

        /* Free uneeded configurations if exists */
        while (!list_empty(curr_conf))
        {
            conf = list_entry(curr_conf->next,
                              struct android_configuration, list_item);
            pnp_free_android_config(dev, conf);
        }
        dev->enabled = MBB_USB_FALSE;
    }
}

/*****************************************************************
Parameters    :  functions
                 cdev
Return        :
Description   :  初始化设备支持的funcs
*****************************************************************/
static int android_init_functions(struct android_usb_function** functions,
                                  struct usb_composite_dev* cdev)
{
    struct android_dev* dev = android_get_android_dev();
    struct android_usb_function* f;
    struct device_attribute** attrs;
    struct device_attribute* attr;
    struct class* dev_class = android_get_android_class();
    int err = -1;
    int index = 1; /* index 0 is for android0 device */

    if ( NULL == dev || NULL == dev_class)
    {
        DBG_E(MBB_PNP, "android_get_android_dev %p! dev_class%p\n", dev, dev_class);
        return err;
    }

    for (; (f = *functions++); index++)
    {
        f->dev_name = kasprintf(GFP_KERNEL, "f_%s", f->name);
        f->android_dev = NULL;

        if (!f->dev_name)
        {
            err = (-ENOMEM);
            goto err_out;
        }
        DBG_I(MBB_PNP, "create device %s\n", f->name);
        f->dev = device_create(dev_class, dev->dev,
                               MKDEV(0, index), f, f->dev_name);
        if (IS_ERR(f->dev))
        {
            DBG_E(MBB_PNP, "Failed to create dev %s\n", f->dev_name);
            err = PTR_ERR(f->dev);
            f->dev = NULL;
            goto err_create;
        }

        if (f->init)
        {
            err = f->init(f, cdev);

            if (err)
            {
                DBG_E(MBB_PNP, "Failed to init %s\n", f->name);
                goto err_init;
            }
        }

        attrs = f->attributes;
        DBG_I(MBB_PNP, "create attrs %s\n", f->name);
        if (attrs)
        {
            while ((attr = *attrs++) && !err)
            {
                err = device_create_file(f->dev, attr);
            }
        }

        if (err)
        {
            DBG_E(MBB_PNP, "Failed to create function %s attributes\n", f->name);
            goto err_attrs;
        }
    }

    return 0;

err_attrs:

    for (attr = *(attrs -= USB_NUM_2); attrs != f->attributes; attr = *(attrs--))
    { device_remove_file(f->dev, attr); }

    if (f->cleanup)
    { f->cleanup(f); }

err_init:
    device_destroy(dev_class, f->dev->devt);
err_create:
    f->dev = NULL;
    kfree(f->dev_name);
err_out:
    android_cleanup_functions(dev->functions);
    return err;
}

/*****************************************************************
Parameters    :  dev
                 c
                 name
                 procotol
Return        :
Description   :  将要打开的设备功能挂在设备功能链中
*****************************************************************/
static USB_INT android_enable_function(struct android_dev* dev,
                                       struct android_configuration* c,
                                       USB_CHAR* name,
                                       USB_UINT8 procotol)
{
    struct android_usb_function** functions = NULL;
    struct android_usb_function* f = NULL;
    struct android_usb_function* f_tmp = NULL;

    if ( NULL == c || NULL == name)
    {
        DBG_E(MBB_PNP, "dev/%p ,android_configuration/ %p ,Failed to enable function '%s'\n",
            dev,c, name);
        return (-EINVAL);
    }
    
    functions = dev->functions;
    
    while ((f = *functions++))
    {
        if (!strcmp(name, f->name))
        {
            list_for_each_entry(f_tmp, &c->enabled_functions, enabled_list)
            {
                if (!strcmp(name, f_tmp->name))
                {
                    DBG_E(MBB_PNP, "name:%s**f_tmp->name:%s has been enabled\n",
                          name, f_tmp->name);
                    return 0;
                }
            }
            f_tmp = NULL;
            f_tmp = (struct android_usb_function *)kzalloc(sizeof(struct android_usb_function),GFP_KERNEL);
            if (!f_tmp)
            {
                DBG_E(MBB_PNP, "Malloc function name:%s failed\n",
                          name);
                return -ENOMEM;
            }
            memcpy(f_tmp, f, sizeof(struct android_usb_function));
            INIT_LIST_HEAD(&(f_tmp->enabled_list));
            f_tmp->property = procotol;
            list_add_tail(&f_tmp->enabled_list, &c->enabled_functions);
            DBG_T(MBB_PNP, "enable '%s' port \n", f_tmp->name);
            return 0;
        }
    }
    
    DBG_E(MBB_PNP, "can not find %s in support functions\n", name);
    return (-EINVAL);
}

/*-------------------------------------------------------------------------*/
/* /sys/class/android_usb/android%d/ interface */
/*-------------------------------------------------------------------------*/

static ssize_t remote_wakeup_show(struct device* pdev,
                                  struct device_attribute* attr, char* buf)
{
    struct android_dev* dev = dev_get_drvdata(pdev);
    struct android_configuration* conf;

    /*
     * Show the wakeup attribute of the first configuration,
     * since all configurations have the same wakeup attribute
     */
    if (dev->configs_num == 0)
    { return 0; }

    conf = list_entry(dev->android_configs.next,
                      struct android_configuration,
                      list_item);

    return snprintf(buf, PAGE_SIZE, "%d\n",
                    !!(conf->usb_config.bmAttributes &
                       USB_CONFIG_ATT_WAKEUP));
}

static ssize_t remote_wakeup_store(struct device* pdev,
            struct device_attribute* attr, const char* buff, size_t size)
{
    struct android_dev* dev = dev_get_drvdata(pdev);
    struct android_configuration* conf;
    int enable = 0;
    /*成功写入参数1*/
    if (sscanf(buff, "%d", &enable) > 0)
    {
        pr_debug("android_usb: %s remote wakeup\n",
                 enable ? "enabling" : "disabling");

        list_for_each_entry(conf, &dev->android_configs, list_item)
        {
            if (enable)
                conf->usb_config.bmAttributes |=
                    USB_CONFIG_ATT_WAKEUP;
            else
                conf->usb_config.bmAttributes &=
                    ~USB_CONFIG_ATT_WAKEUP;
        }
        return size;

    }
    else
    {
        return 0;
    }

}

static ssize_t
functions_store_enable_show(struct device* pdev, struct device_attribute* attr, char* buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", functions_store_enable);
}

static ssize_t
functions_store_enable_store(struct device* pdev, struct device_attribute* attr,
                             const char* buff, size_t size)
{
    /*code cc */
    if (1 == sscanf(buff, "%d", &functions_store_enable))
    {
        return size;
    }
    else
    {
        return 0;
    }
}

static ssize_t
functions_show(struct device* pdev, struct device_attribute* attr, char* buf)
{
    struct android_dev* dev = dev_get_drvdata(pdev);
    struct android_configuration* conf;
    struct android_usb_function* f;
    char* buff = buf;

    mutex_lock(&dev->mutex);

    list_for_each_entry(conf, &dev->android_configs, list_item)
    {
        if (buff != buf)
        { *(buff - 1) = ':'; }

        list_for_each_entry(f, &conf->enabled_functions, enabled_list)
        buff += snprintf(buff, PAGE_SIZE, "%s,", f->name);
    }

    mutex_unlock(&dev->mutex);

    if (buff != buf)
    { *(buff - 1) = '\n'; }

    return buff - buf;
}

static ssize_t
functions_store(struct device* pdev, struct device_attribute* attr,
                const char* buff, size_t size)
{
    struct android_dev* dev = dev_get_drvdata(pdev);
    char* conf_str;
    char* stor_name;
    char* case_name;
    char buf[USB_NUM_256], *b;
    USB_INT port_protocol = 0;
    pnp_dynamic_mode* dynamic_port_mode = NULL;

    if ( MBB_USB_OFF == functions_store_enable)
    {
        DBG_T(MBB_PNP, "can not store functions \n");
        return -1;
    }
    mutex_lock(&dev->mutex);

    strlcpy(buf, buff, sizeof(buf));
    b = strim(buf);
    
    pnp_d_port_clear_mode(DYNAMIC_DEBUG_MODE);
    dynamic_port_mode = pnp_d_port_get_mode(DYNAMIC_DEBUG_MODE);


    while (b)
    {
        conf_str = strsep(&b, ":");

        if (!conf_str)
        { continue; }

        while (conf_str)
        {
            DBG_T(MBB_PNP, "conf_str = %s\n", conf_str);
            stor_name = strsep(&conf_str, ",");
            case_name = pnp_d_port_case_port_name(stor_name);
            DBG_T(MBB_PNP, "case_name = %s\n", case_name);
            if ( NULL != case_name)
            {
                port_protocol = pnp_d_port_get_port_protocol(case_name);
                if (port_protocol > USB_IF_PROTOCOL_VOID)
                {
                    pnp_d_port_add_info(dynamic_port_mode,
                                               case_name, port_protocol);
                }
                else
                {
                    pnp_d_port_add_info(dynamic_port_mode,
                                               case_name, USB_IF_PROTOCOL_VOID);
                }
            }
        }
    }    
    g_pnp_info.portModeIndex = DYNAMIC_DEBUG_MODE;
    pnp_notify_to_switch();

    DBG_T(MBB_PNP, "success\n");
    mutex_unlock(&dev->mutex);

    return size;
}

static ssize_t enable_show(struct device* pdev, struct device_attribute* attr,
                           char* buf)
{
    struct android_dev* dev = dev_get_drvdata(pdev);

    return snprintf(buf, PAGE_SIZE, "%d\n", dev->enabled);
}


static ssize_t pm_qos_show(struct device* pdev,
                           struct device_attribute* attr, char* buf)
{
    struct android_dev* dev = dev_get_drvdata(pdev);

    return snprintf(buf, PAGE_SIZE, "%s\n", dev->pm_qos);
}

static ssize_t pm_qos_store(struct device* pdev,
                            struct device_attribute* attr,
                            const char* buff, size_t size)
{
    struct android_dev* dev = dev_get_drvdata(pdev);

    strlcpy(dev->pm_qos, buff, sizeof(dev->pm_qos));

    return size;
}


static ssize_t state_show(struct device* pdev, struct device_attribute* attr,
                          char* buf)
{
    struct android_dev* dev = dev_get_drvdata(pdev);
    struct usb_composite_dev* cdev = dev->cdev;
    char* state = "DISCONNECTED";
    unsigned long flags;

    if (!cdev)
    { goto out; }

    spin_lock_irqsave(&cdev->lock, flags);

    if (cdev->config)
    { state = "CONFIGURED"; }
    else if (dev->connected)
    { state = "CONNECTED"; }

    spin_unlock_irqrestore(&cdev->lock, flags);
out:
    return snprintf(buf, PAGE_SIZE, "%s\n", state);
}

USB_VOID pnp_set_setmode_state(int state)
{
    setmode_state = state;
}
USB_VOID pnp_switch_ndis_on_off(USB_VOID)
{
    DBG_I(MBB_PNP, "%s +++usb_monitor_web_event+++\n", __func__);

    /*重新读取NV项*/
    usb_nv_get();
    /*根据新NV重新枚举端口*/
    pnp_notify_to_switch();

    return;
}

static void setmode_work_func(struct work_struct* work)
{
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    DBG_I(MBB_PNP, "setmode_work_func  !\n");
    switch (setmode_state)
    {
        case APP_SETMODE_NORMAL_WORK:
            pnp_switch_autorun_port();
            break;
        case APP_SETMODE_RNDIS_DEBUG:
            /*static ports switch*/
            pnp_switch_rndis_debug_mode();
            break;

        case APP_SETMODE_RNDIS_DLOAD:
            /*static ports switch*/
            pnp_switch_rndis_project_mode();
            break;
        case APP_SETMODE_MBIM_DEBUG:
            /*static ports switch*/
            if ( MBB_NV_ENABLE == usb_nv_info->winblue_profile.MBIMEnable)
            {
                pnp_switch_mbim_debug_mode();
            }
            else
            {
                DBG_T(MBB_PNP, "do not support MBIM\n");
                pnp_switch_autorun_port();
            }
            break;
        case APP_SETMODE_CHARGE_ONLY:
            /*static ports switch*/
            pnp_switch_charge_only_port();
            break;
        case APP_SETMODE_NDIS_SWITCH:
            /* ndis switch on or off */
            pnp_switch_ndis_on_off();
            break;
        default:
            DBG_T(MBB_PNP, "mode do not support %d\n", setmode_state);
            /*dynamic ports switch*/
            pnp_switch_autorun_port();
            break;
    }
}

static ssize_t setmode_show(struct device* pdev, struct device_attribute* attr,
                            char* buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", setmode_state);
}

static ssize_t
setmode_store(struct device* pdev, struct device_attribute* attr,
              const char* buff, size_t size)
{
    struct android_dev* dev = android_get_android_dev();
    struct usb_composite_dev* cdev = NULL;
    int app_setmode = SETMODE_UNKNOWN_STATE;
    
    if ( NULL == dev)
    {
        DBG_E(MBB_PNP, "android_get_android_dev NULL!\n");
        return -EIO;
    }
    
    cdev = dev->cdev;
    
    if (1 == sscanf(buff, "%4d", &app_setmode))
    {
        DBG_T(MBB_PNP, "app_setmode : %d\n", app_setmode);
        if (app_setmode == setmode_state)
        {
            DBG_T(MBB_PNP, "setmode don't changed\n");
            return size;
        }
        /* 此处会引起Code Checker警告，但是此处代码完全符合C语言语法，编程规范*/
        pnp_set_setmode_state(app_setmode);
        /*先拉低D+，*/
        usb_gadget_disconnect(cdev->gadget);
        DBG_I(MBB_PNP, "setmode_store %d!\n", setmode_state);
        pnp_usb_queue_delay_work(&setmode_work, 1000 * HZ / 1000);
        return size;
    }
    else
    {
        DBG_E(MBB_PNP, "setmode read failed\n");
        return -ENOMEM;
    }

}

static ssize_t usb_private_show(struct device* pdev, struct device_attribute* attr,
                                char* buf)
{
    usb_nv_dump();
    pnp_dump();
    return 0;
}

static ssize_t
usb_module_log_show(struct device* pdev, struct device_attribute* attr,
                    char* buf)
{
    usb_log_level_get();
    return 0;
}
static ssize_t
usb_module_log_store(struct device* pdev, struct device_attribute* attr,
                     const char* buff, size_t size)
{
    char* change;
    USB_INT module = MBB_PNP;
    USB_INT lever = 1;
    char buf[USB_NUM_256], *b;

    strlcpy(buf, buff, sizeof(buf));
    b = strim(buf);

    if (NULL != b)
    {
        change = strsep(&b, ",");
        if (NULL != change)
        {
            if (1 != sscanf(change, "%2d", &module))
            {
                DBG_E(MBB_PNP, "store module err\n");
                return -EINVAL;
            }
        }
        else
        {
            DBG_E(MBB_PNP, "echo module err\n");
            return -EINVAL;
        }
        change = strsep(&b, ",");
        if (NULL != change)
        {
            if (1 != sscanf(change, "%2d", &lever))
            {
                DBG_E(MBB_PNP, "store lever err\n");
                return -EINVAL;
            }
        }
        else
        {
            DBG_E(MBB_PNP, "echo lever err\n");
            return -EINVAL;
        }
        if ((MBB_LAST <= module) || (0 > module))
        {
            DBG_E(MBB_PNP, "module is wrong\n");
            return -EINVAL;
        }
        /*lever用于在usb_debug.c的lev_name数组中按下标取值，需要限制大小*/
        if ((U_ALL < lever) || (0 > lever))
        {
            DBG_E(MBB_PNP, "lever is wrong\n");
            return -EINVAL;
        }

        usb_log_level_set(module , lever);
        DBG_SET_LEVEL(module, lever);
        DBG_T(MBB_PNP, "success\n");
        return size;
    }

    return -EINVAL;
}

static ssize_t dialmode_show(struct device* pdev, struct device_attribute* attr,
                             char* buf)
{
    USB_UINT PsBuffer = 0;
    USB_UINT CdcBuffer = 0;

    if (0 != Query_USB_PortType(&PsBuffer , &CdcBuffer))
    {
        DBG_E(MBB_PNP, "dialmode_show err !\n");
        return 0;
    }
    return snprintf(buf, PAGE_SIZE, "%d,%d\n", PsBuffer, CdcBuffer);
}

static ssize_t getportmode_show(struct device* pdev, struct device_attribute* attr,
                                char* buf)
{
    USB_CHAR PsBuffer[USB_NUM_256];
    USB_CHAR mode_type[USB_NUM_256];
    USB_ULONG len = 0;
    memset(PsBuffer , 0 , sizeof(PsBuffer) - 1);
    memset(mode_type , 0 , sizeof(mode_type) - 1);
    get_support_mode_list_str(mode_type);
    if (1 == Check_EnablePortName(PsBuffer, &len))
    {
        DBG_E(MBB_PNP, "getportmode_show err !\n");
        return 0;
    }
    DBG_T(MBB_PNP, "mode_type + PsBuffer = %s%s\n", mode_type, PsBuffer);
    return snprintf(buf, PAGE_SIZE, "%s:%s%s", mode_type, CHIP_MANUFACTURER_NAME, PsBuffer);
}

static ssize_t usb_net_state_show(struct device* pdev,
                                  struct device_attribute* attr, char* buf)
{
    USB_INT drv_mode = NET_STATE_NOT_READY;
    if( NULL == buf)
    {
        return 0;
    }
    /*RNDIS,ECM,NCM 提供给路由单板开机时查询网卡设备是否准备好来决定是否挂桥*/
    drv_mode = pnp_get_net_drv_state();
    DBG_T(MBB_PNP, "usb_net_state %d !\n", drv_mode);
    return snprintf(buf, PAGE_SIZE, "%d\n", drv_mode);
}

#define DESCRIPTOR_ATTR(field, format_string)               \
    static ssize_t                              \
    field ## _show(struct device *dev, struct device_attribute *attr,   \
                   char *buf)                       \
    {                                   \
        return snprintf(buf, PAGE_SIZE,                 \
                        format_string, device_desc.field);      \
    }                                   \
    static ssize_t                              \
    field ## _store(struct device *dev, struct device_attribute *attr,  \
                    const char *buf, size_t size)               \
    {                                   \
        int value;                          \
        if (sscanf(buf, format_string, &value) == 1) {          \
            device_desc.field = value;              \
            return size;                        \
        }                               \
        return -1;                          \
    }                                   \
    static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);

#define DESCRIPTOR_STRING_ATTR(field, buffer)               \
    static ssize_t                              \
    field ## _show(struct device *dev, struct device_attribute *attr,   \
                   char *buf)                       \
    {                                   \
        return snprintf(buf, PAGE_SIZE, "%s", buffer);          \
    }                                   \
    static ssize_t                              \
    field ## _store(struct device *dev, struct device_attribute *attr,  \
                    const char *buf, size_t size)               \
    {                                   \
        if (size >= sizeof(buffer))                 \
            return -EINVAL;                     \
        strlcpy(buffer, buf, sizeof(buffer));               \
        strim(buffer);                          \
        return size;                            \
    }                                   \
    static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);


DESCRIPTOR_ATTR(idVendor, "%04x\n")
DESCRIPTOR_ATTR(idProduct, "%04x\n")
DESCRIPTOR_ATTR(bcdDevice, "%04x\n")
DESCRIPTOR_ATTR(bDeviceClass, "%d\n")
DESCRIPTOR_ATTR(bDeviceSubClass, "%d\n")
DESCRIPTOR_ATTR(bDeviceProtocol, "%d\n")
DESCRIPTOR_STRING_ATTR(iManufacturer, manufacturer_string)
DESCRIPTOR_STRING_ATTR(iProduct, product_string)
DESCRIPTOR_STRING_ATTR(iSerial, serial_string)

static DEVICE_ATTR(functions_store_enable, S_IRUGO | S_IWUSR, functions_store_enable_show,
                   functions_store_enable_store);
static DEVICE_ATTR(functions, S_IRUGO | S_IWUSR, functions_show,
                   functions_store);
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, enable_show, enable_store);
static DEVICE_ATTR(pm_qos, S_IRUGO | S_IWUSR,
                   pm_qos_show, pm_qos_store);
static DEVICE_ATTR(state, S_IRUGO, state_show, NULL);
static DEVICE_ATTR(remote_wakeup, S_IRUGO | S_IWUSR,
                   remote_wakeup_show, remote_wakeup_store);

static DEVICE_ATTR(setmode, S_IRUGO | S_IWUSR, setmode_show, setmode_store);
static DEVICE_ATTR(usb_private, S_IRUGO, usb_private_show, NULL);
static DEVICE_ATTR(usb_module_log, S_IRUGO | S_IWUSR, usb_module_log_show, usb_module_log_store);
static DEVICE_ATTR(dialmode, S_IRUGO , dialmode_show, NULL);
static DEVICE_ATTR(getportmode, S_IRUGO , getportmode_show, NULL);
static DEVICE_ATTR(usb_net_state, S_IRUGO , usb_net_state_show, NULL);

static struct device_attribute* android_usb_attributes[] =
{
    &dev_attr_idVendor,
    &dev_attr_idProduct,
    &dev_attr_bcdDevice,
    &dev_attr_bDeviceClass,
    &dev_attr_bDeviceSubClass,
    &dev_attr_bDeviceProtocol,
    &dev_attr_iManufacturer,
    &dev_attr_iProduct,
    &dev_attr_iSerial,
    &dev_attr_functions,
    &dev_attr_enable,
    &dev_attr_pm_qos,
    &dev_attr_state,
    &dev_attr_remote_wakeup,
    &dev_attr_setmode,
    &dev_attr_usb_private,
    &dev_attr_usb_module_log,
    &dev_attr_functions_store_enable,
    &dev_attr_dialmode,
    &dev_attr_getportmode,
    &dev_attr_usb_net_state,
    NULL
};

/*****************************************************************
Parameters    :  dev
                 usb_core_id
Return        :
Description   :  创建单板AP侧的设备节点
*****************************************************************/
int android_create_device(struct android_dev* dev, u8 usb_core_id)
{
    struct device_attribute** attrs = android_usb_attributes;
    struct device_attribute* attr;
    struct class* dev_class = android_get_android_class();
    char device_node_name[ANDROID_DEVICE_NODE_NAME_LENGTH];
    int err = -1;

    /*
     * The primary usb core should always have usb_core_id=0, since
     * Android user space is currently interested in android0 events.
     */
    snprintf(device_node_name, ANDROID_DEVICE_NODE_NAME_LENGTH,
             "android%d", usb_core_id);

    if ( NULL == dev_class)
    {
        DBG_E(MBB_PNP, "dev_class NULL!\n");
        return err;
    }
    dev->dev = device_create(dev_class, NULL,
                             MKDEV(0, 0), NULL, device_node_name);

    if (IS_ERR(dev->dev))
    { return PTR_ERR(dev->dev); }

    dev_set_drvdata(dev->dev, dev);

    while ((attr = *attrs++))
    {
        err = device_create_file(dev->dev, attr);

        if (err)
        {
            device_destroy(dev_class, dev->dev->devt);
            return err;
        }
    }

    return 0;
}

/*****************************************************************
Parameters    :  dev
Return        :
Description   :  销毁单板AP侧的设备节点
*****************************************************************/
void android_destroy_device(struct android_dev* dev)
{
    struct device_attribute** attrs = android_usb_attributes;
    struct device_attribute* attr;
    struct class* dev_class = android_get_android_class();

    while ((attr = *attrs++))
    { device_remove_file(dev->dev, attr); }

    if ( NULL != dev_class)
    {
        device_destroy(dev_class, dev->dev->devt);
    }
    else
    {
        DBG_E(MBB_PNP, "dev_class NULL!\n");
    }
}

/*****************************************************************
Parameters    :  cdev
Return        :
Description   :  设备去绑定
*****************************************************************/
static USB_INT android_usb_unbind(struct usb_composite_dev* cdev)
{
    struct android_dev* dev = android_get_android_dev();
    int ret = -1;

    if ( NULL == dev)
    {
        DBG_E(MBB_PNP, "android_get_android_dev NULL!\n");
        return ret;
    }

    manufacturer_string[0] = '\0';
    product_string[0] = '\0';
    serial_string[0] = '0';
    cancel_work_sync(&dev->work);
    android_cleanup_functions(dev->functions);
    dev->cdev = NULL;
    return 0;
}

/*****************************************************************
Parameters    :  cdev
Return        :
Description   :  设备绑定,由底层驱动调用
*****************************************************************/
static int android_bind(struct usb_composite_dev* cdev)
{
    struct android_dev* dev = android_get_android_dev();
    struct usb_gadget*  gadget = NULL;
    struct android_configuration* conf;
    int id;
    int ret = -1;
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    USB_CHAR* serialum = NULL;

    USB_INT unique_serial_num_flag = 0;

    if ( NULL == dev || NULL == cdev)
    {
        DBG_E(MBB_PNP, "dev/%p OR  cdev/%p NULL!\n", dev, cdev);
        return ret;
    }

    gadget = cdev->gadget;
    pnp_usb_init_enum_stat();
    dev->cdev = cdev;

    /* Save the default handler */
    if (NULL != cdev->req && NULL != cdev->req->complete)
    {
        dev->setup_complete = cdev->req->complete;
    }
    else
    {
        DBG_E(MBB_PNP, "cdev->req/%p OR  cdev->req->complete NULL!\n", cdev->req);
        return ret;
    }

    /*
     * Start disconnected. Userspace will connect the gadget once
     * it is done configuring the functions.
     */
    usb_gadget_disconnect(gadget);

    /* Init the supported functions only once, on the first android_dev */
    ret = android_init_functions(dev->functions, cdev);

    if (ret)
    {
        DBG_E(MBB_PNP, "android_init_functions return err %d\n", ret);
        return ret;
    }

    /* Allocate string descriptor numbers ... note that string
     * contents can be overridden by the composite_dev glue.
     */
    id = usb_string_id(cdev);

    if (id < 0)
    {
        DBG_E(MBB_PNP, "usb_string_id STRING_MANUFACTURER_IDX return err %d\n", id);
        return id;
    }

    strings_dev[STRING_MANUFACTURER_IDX].id = id;
    device_desc.iManufacturer = id;

    id = usb_string_id(cdev);

    if (id < 0)
    {
        DBG_E(MBB_PNP, "usb_string_id STRING_PRODUCT_IDX return err %d\n", id);
        return id;
    }

    strings_dev[STRING_PRODUCT_IDX].id = id;
    device_desc.iProduct = id;

    /* Default strings - should be updated by userspace */
    memset(manufacturer_string,0,sizeof(manufacturer_string));
    memset(product_string,0,sizeof(product_string));
    memset(serial_string,0,sizeof(serial_string));

    strncpy(manufacturer_string,
            usb_nv_info->port_dynamic_name.huawei_manufacturer_dynamic_name,
            DYNAMIC_INFO_NAME_CHAR_NUM - 1);

    strncpy(product_string,
            usb_nv_info->port_dynamic_name.huawei_product_dynamic_name,
            DYNAMIC_INFO_NAME_CHAR_NUM - 1);
    /*判断是否唯一SN的标志*/
    unique_serial_num_flag = pnp_unique_serial_num_flag();

    /*如果是唯一SN从NV中读取SN*/
    if (SERIAL_NUM_FLAG_UNIQUE == unique_serial_num_flag)
    {
        DBG_T(MBB_PNP, "unique_serial_num_flag\n");
        serialum = pnp_get_serial_num();
    }
    else/*默认SN*/
    {
        serialum = SERIAL_NUM_DEFAULT;
    }

    strncpy(serial_string, serialum, sizeof(serial_string) - 1);

    id = usb_string_id(cdev);

    if (id < 0)
    {
        DBG_E(MBB_PNP, "usb_string_id STRING_SERIAL_IDX return err %d\n", id);
        return id;
    }

    strings_dev[STRING_SERIAL_IDX].id = id;
    device_desc.iSerialNumber = id;

    if (gadget_is_otg(cdev->gadget))
    {
        list_for_each_entry(conf, &dev->android_configs, list_item)
        conf->usb_config.descriptors = otg_desc;
    }

    return 0;
}

/*****************************************************************
Parameters    :  gadget
                 c
Return        :
Description   :  ep0的请求处理
*****************************************************************/
static int
android_setup(struct usb_gadget* gadget, const struct usb_ctrlrequest* c)
{
    struct usb_composite_dev*   cdev = NULL;
    struct android_dev* dev = android_get_android_dev();
    struct usb_request*     req = NULL;
    struct android_usb_function*    f;
    struct android_configuration*   conf;
    int value = -EOPNOTSUPP;
    unsigned long flags;
    bool do_work = false;
    int ret = -1;

    if ( NULL == dev  || NULL == gadget)
    {
        DBG_E(MBB_PNP, "dev/%p OR gadget /%p NULL!\n", dev, gadget);
        return ret;
    }
    cdev = get_gadget_data(gadget);

    if (NULL == cdev)
    {
        DBG_E(MBB_PNP, "cdev NULL!\n");
        return ret;
    }

    if ( NULL == cdev->req \
         || NULL == dev->setup_complete || NULL == gadget->ep0)
    {
        DBG_E(MBB_PNP, "cdev->req/%p OR  dev->setup_complete "
              "/%p OR gadget->ep0/%p NULL!\n", cdev->req, dev->setup_complete, gadget->ep0);
        return ret;
    }
    
    req = cdev->req;
    req->complete = dev->setup_complete;
    req->zero = 0;
    req->length = 0;
    gadget->ep0->driver_data = cdev;

    list_for_each_entry(conf, &dev->android_configs, list_item)
    list_for_each_entry(f,
                        &conf->enabled_functions,
                        enabled_list)
    {
        if (f->ctrlrequest)
        {
            value = f->ctrlrequest(f, cdev, c);

            if (value >= 0)
            { break; }
        }
    }

    if (value < 0 || (USB_GADGET_DELAYED_STATUS == value) )
    { value = pnp_huawei_vendor_setup(gadget, c); }

    spin_lock_irqsave(&cdev->lock, flags);

    if (!dev->connected)
    {
        dev->connected = 1;
        do_work = true;
    }
    else if ((c->bRequest == USB_REQ_SET_CONFIGURATION) &&
        (NULL != cdev->config))
    {
        do_work = true;
    }

    spin_unlock_irqrestore(&cdev->lock, flags);

    if (do_work)
    {
        schedule_work(&dev->work);
    }

    return value;
}

/*****************************************************************
Parameters    :  cdev
Return        :
Description   :  设备disconnect，由底层掉用。
*****************************************************************/
static void android_disconnect(struct usb_composite_dev* cdev)
{
    struct android_dev* dev = android_get_android_dev();

    if ( NULL == dev)
    {
        DBG_E(MBB_PNP, "android_get_android_dev NULL!\n");
        return ;
    }

    /*do other disconnect if necessary*/
    //pnp_accessory_disconnect();

    dev->connected = 0;
    schedule_work(&dev->work);
}

/*****************************************************************
Parameters    :  cdev
Return        :
Description   :  设备suspend，由底层掉用。
*****************************************************************/
static void android_suspend(struct usb_composite_dev* cdev)
{
    usb_enum_state state = USB_ENUM_NONE;
    unsigned long flags;
    struct android_dev* dev = android_get_android_dev();

    DBG_I(MBB_PNP, " android_suspend \n");
    if ( NULL == dev)
    {
        DBG_E(MBB_PNP, "android_get_android_dev NULL!\n");
        return ;
    }
    state = huawei_get_usb_enum_state();
    if ( USB_ENUM_DONE == state )
    {
        usb_notify_syswatch(EVENT_DEVICE_USB, USB_SUSPEND_EVENT);
    }
    spin_lock_irqsave(&cdev->lock, flags);

    if (!dev->suspended)
    {
        dev->suspended = 1;
        schedule_work(&dev->work);
    }

    spin_unlock_irqrestore(&cdev->lock, flags);
}

/*****************************************************************
Parameters    :  cdev
Return        :
Description   :  设备resume，由底层掉用。
*****************************************************************/
static void android_resume(struct usb_composite_dev* cdev)
{
    usb_enum_state state = USB_ENUM_NONE;
    unsigned long flags;
    struct android_dev* dev = android_get_android_dev();
    DBG_I(MBB_PNP, " android_resume \n");

    if ( NULL == dev)
    {
        DBG_E(MBB_PNP, "android_get_android_dev NULL!\n");
        return;
    }
    state = huawei_get_usb_enum_state();
    if ( USB_ENUM_DONE == state )
    {
        usb_notify_syswatch(EVENT_DEVICE_USB, USB_RESUME_EVENT);
    }
    spin_lock_irqsave(&cdev->lock, flags);

    if (dev->suspended)
    {
        dev->suspended = 0;
        schedule_work(&dev->work);
    }

    spin_unlock_irqrestore(&cdev->lock, flags);

}

/*****************************************************************************
}  android 设备驱动功能 end
*****************************************************************************/
/*****************************************************************
Parameters    :  dwork
                 delay
Return        :
Description   :  pnp queue_delay_work调用归一化的 USB delay_work
*****************************************************************/
USB_VOID pnp_usb_queue_delay_work(struct delayed_work* dwork, unsigned long delay)
{
    adp_usb_queue_delay_work(dwork, delay);
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  查看设备是否处于升级状态
*****************************************************************/
USB_INT pnp_status_check_download(USB_VOID)
{
    USB_INT dload_flag 		= MBB_USB_FALSE;
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    hw_usb_mode* port_mode;


    dload_flag = pnp_get_dload_flag();
    /*SD升级或在线升级时上报不可用光盘*/
    /* 注: 与键升级不同若上报pcui会有黄色感叹号影响用户体验*/
    if ( PORT_NO_PCUI == dload_flag )
    {
        g_pnp_info.current_port_style   = download_port_style;
        g_pnp_info.portModeIndex        = POWER_OFF_MODE;
        DBG_T(MBB_PNP, "DLOAD NO_PCUI COMPLETE\n");
        return PNP_CHECK_DEVICE_STATUS_COMPLETE;
    }

    /*MBIM升级报1568，增加静态端口形态*/
    if (( PORT_DLOAD == dload_flag ) || ( PORT_NV_RES == dload_flag ))
    {
        g_pnp_info.current_port_style   = download_port_style;

        /*MBIM升级报PID1568*/
        if ( MBB_NV_ENABLE == usb_nv_info->winblue_profile.MBIMEnable )
        {
            DBG_I(MBB_PNP, "mbim dload\n");
            g_pnp_info.portModeIndex    = MBIM_DOWNLOAD_MODE;
        }
        else
        {
#ifdef USB_RNDIS
            DBG_I(MBB_PNP, "hilink dload\n");
            g_pnp_info.portModeIndex    = PCUI_DIAG_MODE;
#else
            DBG_I(MBB_PNP, "stick dload\n");
            g_pnp_info.portModeIndex    = DIAG_PCUI_MODE_EX;
#endif /* USB_RNDIS */
        }
        
        /*模块产品升级*/
        if (MBB_NV_ENABLE == usb_nv_info->multi_config_port_info.nv_status)
        {
            g_pnp_info.portModeIndex    = MBIM_DOWNLOAD_MODE;
            /*升级模式PID定制*/
            if (MBB_NV_ENABLE == usb_nv_info->boot_pid_info.nv_status)
            {
                port_mode = pnp_static_port_get_mode(g_pnp_info.portModeIndex);
                port_mode->PID = usb_nv_info->boot_pid_info.dynamic_boot_pid;
            }
        }

        DBG_T(MBB_PNP, " COMPLETE\n");
        return PNP_CHECK_DEVICE_STATUS_COMPLETE;
    }

    DBG_I(MBB_PNP, "return CONTINUE\n");
    return PNP_CHECK_DEVICE_STATUS_CONTINUE;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  查看设备是否处于关机充电模式
*****************************************************************/
USB_INT pnp_status_check_poweroff(USB_VOID)
{
    USB_INT power_off   = MBB_USB_NO;
    USB_INT fast_off    = MBB_USB_NO;
    USB_INT off_status    = MBB_USB_NO;
    struct android_dev* dev = android_get_android_dev();

    if ( NULL == dev)
    {
        DBG_E(MBB_PNP, "android_get_android_dev NULL!\n");
        return PNP_CHECK_DEVICE_STATUS_CONTINUE;
    }

    power_off   = usb_power_off_chg_stat();
    fast_off    = usb_fast_on_off_stat();

    if (MBB_USB_YES == power_off)
    {
        /*power off status enable USB port, do not ctl by app*/
        dev->irq_ctl_port = MBB_USB_ENABLE;
        off_status = MBB_USB_YES;
    }

    if ( MBB_USB_YES == fast_off)
    {
        off_status = MBB_USB_YES;
    }

    if ( MBB_USB_YES == off_status)
    {
        g_pnp_info.current_port_style   = charge_port_style;
        g_pnp_info.portModeIndex        = POWER_OFF_MODE;
        DBG_T(MBB_PNP, " COMPLETE\n");
        return PNP_CHECK_DEVICE_STATUS_COMPLETE;
    }

    DBG_I(MBB_PNP, "return CONTINUE\n");
    return PNP_CHECK_DEVICE_STATUS_CONTINUE;
}


/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  查看NO pnp状态
*****************************************************************/
USB_INT get_dynamic_port_nopnp_flag(USB_VOID)
{
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();

    if (USB_IF_PROTOCOL_NOPNP == usb_nv_info->dev_profile_info.first_port_style[0])
    {
        //NoPNP 过程只允许在windows 系统使用
        g_pnp_info.rewind_param.bPcType = WINDOWS_OS_FLAG;
        return MBB_USB_TRUE;
    }
    else
    {
        return MBB_USB_FALSE;
    }
}

/*****************************************************************
Parameters    :  OsType
Return        :
Description   :  RNDIS多端口选择
*****************************************************************/
USB_INT pnp_select_rndis_full_sence( USB_UINT8  OsType )
{
    USB_INT PortModeIndex   = INVALID_MODE;

    switch (OsType)
    {
        case OS_TYPE_WINDOWS:
            
#ifdef USB_SD
            PortModeIndex = RNDIS_DEBUG_MODE_SD;
#else
            PortModeIndex = RNDIS_DEBUG_MODE;
#endif
            break;

        case OS_TYPE_MAC:
        case OS_TYPE_LINUX:
            
#ifdef USB_SD
            PortModeIndex = ECM_DEBUG_MODE_SD;
#else
            PortModeIndex = ECM_DEBUG_MODE;
#endif
            break;

        default:
            break;
    }
    DBG_I(MBB_PNP, "PortModeIndex: %d\n", PortModeIndex);
    return PortModeIndex;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  NV 50091第一个字节为FF，
                 直接上报切换后的端口形态(非RNDIS形态)
                 或者多端口调试模式(RNDIS形态)
*****************************************************************/
USB_INT pnp_status_check_nopnp(USB_VOID)
{
    USB_INT nopnp_mode  =  MBB_USB_FALSE;
    USB_INT PortModeIndex   = INVALID_MODE;
    USB_INT ret = -1;
    nopnp_mode = get_dynamic_port_nopnp_flag();

    if ( MBB_USB_FALSE == nopnp_mode)
    {
        DBG_I(MBB_PNP, "return CONTINUE\n");
        return PNP_CHECK_DEVICE_STATUS_CONTINUE;
    }

#ifdef USB_RNDIS
    /*RNDIS产品，由于RNDIS必须在第一个位置，否则会导致windows蓝屏
     *为了防止出错，在代码中写死调试模式端口形态
     */
    PortModeIndex = pnp_select_rndis_full_sence(g_pnp_info.rewind_param.bPcType);

    if ( INVALID_MODE == PortModeIndex)
    {
        DBG_E(MBB_PNP, "PortModeIndex is INVALID\n");
        /*but we don't make an error */
        PortModeIndex = CDROM_MODE;
    }
#else
    g_pnp_info.current_port_style = rewind_port_style;
    PortModeIndex = DYNAMIC_REWIND_AFTER_MODE;
#endif

    /*烧片端口可配置功能*/
    ret = pnp_check_factory();
    if ( MBB_USB_YES == ret)
    {
        DBG_I(MBB_PNP, "factory mode\n");
	    g_pnp_info.current_port_style = factory_port_style;
	    PortModeIndex = DYNAMIC_REWIND_AFTER_MODE;
    }

    g_pnp_info.portModeIndex = PortModeIndex;
    g_pnp_info.is_mbim_enable = 0;
    pnp_set_ctl_app_flag(CTL_APP_START);

    DBG_T(MBB_PNP, " COMPLETE\n");
    return PNP_CHECK_DEVICE_STATUS_COMPLETE;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  默认的设备状态，pnp光盘模式
*****************************************************************/
USB_INT pnp_status_default_stat(USB_VOID)
{
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();

    /*50457 NV 可用时,认为是模块产品*/
    if (MBB_NV_ENABLE == usb_nv_info->multi_config_port_info.nv_status)
    {
        g_pnp_info.current_port_style = direct_port_style;

        g_pnp_info.portModeIndex = DYNAMIC_DIRECT_PORT_MODE;
    }
    else    /*否则认为是传统产品*/
    {
        g_pnp_info.current_port_style = pnp_port_style;

        g_pnp_info.portModeIndex = DYNAMIC_REWIND_BEFORE_MODE;
    }
    DBG_T(MBB_PNP, " COMPLETE\n");
    return PNP_CHECK_DEVICE_STATUS_COMPLETE;
}

/*按优先级check配置状态*/
USB_INT (* pnp_check_dev_status_funcs[])(USB_VOID) =
{
    pnp_status_check_download,
    pnp_status_check_factory,
    pnp_status_check_poweroff,
    pnp_status_check_charge_only,
    pnp_status_check_nopnp,
    pnp_status_default_stat,
};

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  根据设备状态选择端口形态
*****************************************************************/
USB_VOID pnp_status_select_port_index(USB_VOID)
{
    USB_INT i = 0;
    USB_INT ret = PNP_CHECK_DEVICE_STATUS_CONTINUE;

    while (NULL != pnp_check_dev_status_funcs[i])
    {
        ret = pnp_check_dev_status_funcs[i]();

        if (HUAWEI_CONFIG_DYNAMIC_PID_COMPLETE == ret )
        {
            /*符合端口配置的条件*/
            break;
        }

        i++;
    }

    DBG_I(MBB_PNP, "switch\n");
    pnp_notify_to_switch();
}

/*=============================================================
切换功能模块
==============================================================*/
/*****************************************************************
Parameters    :  dev
                 conf
Return        :
Description   :  释放一个android_configuration
*****************************************************************/
static USB_VOID pnp_free_android_config(struct android_dev* dev,
                                        struct android_configuration* conf)
{
    list_del(&conf->list_item);
    dev->configs_num--;
    kfree(conf);
    DBG_I(MBB_PNP, "return COMPLETE\n");
}

/*****************************************************************
Parameters    :  androiddev
Return        :
Description   :  申请一个新的android_configuration
*****************************************************************/
static struct android_configuration* pnp_alloc_android_config(struct android_dev* androiddev)
{
    struct android_configuration* config;

    config = kzalloc(sizeof(*config), GFP_KERNEL);

    if (!config)
    {
        DBG_E(MBB_PNP, "Failed to alloc memory for android conf\n");
        return ERR_PTR(-ENOMEM);
    }

    androiddev->configs_num++;
    config->usb_config.label = androiddev->name;
    config->usb_config.unbind = android_unbind_config;
    config->usb_config.bConfigurationValue = androiddev->configs_num;
    config->usb_config.cdev = androiddev->cdev;
    config->usb_config.bmAttributes |= USB_CONFIG_ATT_SELFPOWER;

    DBG_I(MBB_PNP, "config[%d]\n",androiddev->configs_num);
    
    /* MBIM强制需求REMOTE WAKEUP特性 */
    if (g_pnp_info.is_mbim_enable)
    {
        config->usb_config.bmAttributes &= ~USB_CONFIG_ATT_SELFPOWER;
        config->usb_config.bmAttributes |= USB_CONFIG_ATT_WAKEUP;
    }

    INIT_LIST_HEAD(&config->enabled_functions);

    list_add_tail(&config->list_item, &androiddev->android_configs);
    DBG_I(MBB_PNP, "return COMPLETE\n");
    return config;
}

/*****************************************************************
Parameters    :  androiddev
                 config
                 curr_conf
Return        :
Description   :  申请一个新的config，
                如果老config存在，说明上一次的释放出错。
*****************************************************************/
USB_VOID pnp_select_a_new_config(struct android_dev* androiddev,
                                 struct android_configuration** config, struct list_head* curr_conf)
{
    DBG_I(MBB_PNP, "\n");

    /* If the next not equal to the head, take it */
    if ((curr_conf->next) != &(androiddev->android_configs))
    {
        *config = list_entry(curr_conf->next,
                             struct android_configuration,
                             list_item);
        INIT_LIST_HEAD(&(*config)->enabled_functions);
        DBG_E(MBB_PNP, "it's an error,ConfigurationValue=%d,check if you free_android_config when switch\n",
              (*config)->usb_config.bConfigurationValue);
    }
    else
    {
        *config = pnp_alloc_android_config(androiddev);
    }
}

/*****************************************************************
Parameters    :  index
Return        :
Description   :  根据端口索引，获取静态端口形态信息
*****************************************************************/
hw_usb_mode* pnp_static_port_get_mode(USB_UCHAR   index )
{
    USB_INT i = 0 ;
    hw_usb_mode* port_mode = NULL;
    hw_usb_mode *support_usb_mode = pnp_get_huawei_usb_mode();

    for ( i = 0; i < MAX_PORT_MODE_NUM ; i++ )
    {
        if ( support_usb_mode[i].port_index == index )
        {
            port_mode = support_usb_mode + i;
            break;
        }
    }

    if (NULL == port_mode) /*如果失败，默认返回光盘端口形态*/
    {
        //如果代码走到这可能是config_static_port_index_for_scene 函数修改错误
        DBG_E(MBB_PNP, "get_hw_usb_static_mode index:%d error! return CDROM mode\n", index);
        WARN_ON(1);
        port_mode = &support_usb_mode[0];
        g_pnp_info.portModeIndex = CDROM_MODE;
    }

    return port_mode;
}

/*****************************************************************
Parameters    :  androiddev
                 portindex
Return        :
Description   :  绑定动态端口形态到设备config中
*****************************************************************/
USB_VOID pnp_bind_dynamic_port_mode(struct android_dev* androiddev, USB_INT portindex)
{
    struct android_configuration* config = NULL;
    struct list_head* curr_conf = &androiddev->android_configs;
    pnp_dynamic_mode* port_mode = NULL;
    USB_CHAR** port_group_buf = NULL;
    USB_UINT8* port_procotol = NULL;
    USB_INT multi_config_num = 0;
    USB_CHAR* name = NULL;
    USB_INT i = 0;
    USB_INT err = -1;
    DBG_I(MBB_PNP, "\n");
    port_mode = pnp_d_port_get_mode(portindex);
    port_group_buf = port_mode->mode_buf;
    port_procotol = port_mode->port_protocol;

    if (NULL != port_group_buf)
    {
        /**/
        pnp_select_a_new_config(androiddev, &config, curr_conf);

        curr_conf = curr_conf->next;

        for (i = 0; i < MAX_TOTAL_PORT_NUM; i++)
        {
            name = port_group_buf[i];

            if (NULL == name)
            {
                DBG_I(MBB_PNP, "port_group_buf is endd i = %d\n", i);
                break;
            }
            multi_config_num++;
            if (!strcmp(name, ":"))
            {
                multi_config_num++;
                DBG_I(MBB_PNP, "multi config %d \n", multi_config_num);
                /*previous config have enabled_functions*/
                if (config->enabled_functions.next != &config->enabled_functions )
                {
                    pnp_select_a_new_config(androiddev, &config, curr_conf);
                    curr_conf = curr_conf->next;
                }
                continue;
            }

            err = android_enable_function(androiddev, config, name, port_procotol[i]);

            if (err)
            {
                DBG_E(MBB_PNP, "android_usb: Cannot enable '%s' (%d)\n"
                      , name, err);
            }
        }

        /* Free uneeded configurations if exists */
        while (curr_conf->next != &androiddev->android_configs)
        {
            config = list_entry(curr_conf->next,
                                struct android_configuration, list_item);
            pnp_free_android_config(androiddev, config);
        }
    }
}

/*****************************************************************
Parameters    :  port_mode
Return        :
Description   :  设置静态端口形态的mass lun信息
*****************************************************************/
USB_VOID pnp_s_port_set_mass_lun_info( hw_usb_mode* port_mode )
{
    USB_CHAR** mode_buf = NULL;
    USB_INT i = 0;
    
    if( NULL == port_mode )
    {
        DBG_E(MBB_PNP, "port_mode NULL\n");
        return;
    }
    
    mode_buf = port_mode->mode_buf;
    
    DBG_I(MBB_PNP, "port_mode %d\n", port_mode->port_index);
    for (i = 0; i < DYNAMIC_PID_MAX_PORT_NUM; i++)
    {
        if ( NULL == mode_buf[i])
        {
            DBG_I(MBB_PNP, "port_mode %d mode_buf end with%d!\n",
                  port_mode->port_index, i);
            break;
        }

        if (!strcmp(mode_buf[i], "mass"))
        {
            if (port_mode->cdrom)
            {
                DBG_I(MBB_PNP, "port_mode index %d add CD to mass!\n",
                      port_mode->port_index);
                mass_function_add_lun("mass", "CD");
                break;
            }
            if (port_mode->sd)
            {
                DBG_I(MBB_PNP, "port_mode index %d add SD to mass!\n",
                      port_mode->port_index);
                mass_function_add_lun("mass", "SD");
                break;
            }
        }
    }
}

/*****************************************************************
Parameters    :  androiddev
                 portindex
Return        :
Description   :  绑定静态端口形态到设备config中
*****************************************************************/
USB_VOID pnp_bind_static_port_mode(struct android_dev* androiddev, USB_INT portindex)
{
    struct android_configuration* config = NULL;
    struct list_head* curr_conf = &androiddev->android_configs;
    USB_UINT8 port_protocol_num = USB_IF_PROTOCOL_VOID;
    pnp_dynamic_mode* port_mode = NULL;
    USB_CHAR** port_group_buf = NULL;
    USB_UINT8* port_procotol = NULL;
    USB_INT multi_config_num = 0;
    USB_CHAR* name = NULL;
    USB_INT i = 0;
    USB_INT err = -1;
    DBG_I(MBB_PNP, "\n");
    port_mode = pnp_static_port_get_mode(portindex);
    /*静态端口的mass多lun OR 多interface处理*/
    pnp_s_port_set_mass_lun_info(port_mode);

    port_group_buf = port_mode->mode_buf;

    if (NULL != port_group_buf)
    {
        /**/
        pnp_select_a_new_config(androiddev, &config, curr_conf);

        curr_conf = curr_conf->next;

        for (i = 0; i < DYNAMIC_PID_MAX_PORT_NUM; i++)
        {
            name = port_group_buf[i];

            if (NULL == name)
            {
                DBG_I(MBB_PNP, "port_group_buf is endd i = %d\n", i);
                break;
            }
            multi_config_num++;
            if (!strcmp(name, ":"))
            {
                multi_config_num++;
                DBG_I(MBB_PNP, "multi config %d \n", multi_config_num);
                /*previous config have enabled_functions*/
                if (config->enabled_functions.next != &config->enabled_functions )
                {
                    pnp_select_a_new_config(androiddev, &config, curr_conf);
                    curr_conf = curr_conf->next;
                }

                continue;
            }

            port_protocol_num = pnp_d_port_get_port_protocol(name);

            err = android_enable_function(androiddev, config, name, port_protocol_num);

            if (err)
            {
                DBG_E(MBB_PNP, "android_usb: Cannot enable '%s' (%d)\n",
                      name, err);
            }
        }

        /* Free uneeded configurations if exists */
        while (curr_conf->next != &androiddev->android_configs)
        {
            config = list_entry(curr_conf->next,
                                struct android_configuration, list_item);
            pnp_free_android_config(androiddev, config);
        }
    }
}

/*****************************************************************
Parameters    :  androiddev
Return        :
Description   :  绑定端口形态到设备config中
*****************************************************************/
USB_VOID pnp_bind_port_group2config(struct android_dev* androiddev)
{
    DBG_I(MBB_PNP, "\n");
    
    if( NULL == androiddev )
    {
        DBG_E(MBB_PNP, "androiddev NULL\n");
        return;
    }
    
    if (g_pnp_info.portModeIndex >= INVALID_MODE)
    {
        DBG_E(MBB_PNP, "PortModeIndex is INVALID\n");
        /*but we don't make an error */
        g_pnp_info.portModeIndex = CDROM_MODE;
    }
    /*动态端口*/
    if ( g_pnp_info.portModeIndex >= DYNAMIC_REWIND_BEFORE_MODE )
    {
        pnp_d_port_get_info_from_NV(g_pnp_info.portModeIndex);
        pnp_bind_dynamic_port_mode(androiddev,g_pnp_info.portModeIndex);
    }
    else/*静态端口*/
    {
        pnp_bind_static_port_mode(androiddev,g_pnp_info.portModeIndex);
    }
}

/*****************************************************************
Parameters    :  cdev
Return        :
Description   :  根据场景设置描述符中的iSerialNumber
*****************************************************************/
USB_VOID pnp_set_serialnumber_index(struct usb_composite_dev* cdev)
{
    USB_INT unique_serial_num_flag = 0;
    if( NULL == cdev)
    {
        DBG_E(MBB_PNP, "cdev NULL\n");
        return;
    }
    
    if (rewind_port_style == g_pnp_info.current_port_style)
    {
        /*切换后SN受NV控制*/
        if (0x01 == g_pnp_info.is_daynamic_sn )
        {
            cdev->desc.iSerialNumber = 0;
        }
        else
        {
            cdev->desc.iSerialNumber = strings_dev[STRING_SERIAL_IDX].id;
        }
    }
    else if (pnp_port_style == g_pnp_info.current_port_style || 
       direct_port_style == g_pnp_info.current_port_style )
    {
        cdev->desc.iSerialNumber = strings_dev[STRING_SERIAL_IDX].id;
    }
    else
    {
        /*download_port_style、factory_port_style无SN*/
        cdev->desc.iSerialNumber = 0;
    }
    
    unique_serial_num_flag = pnp_unique_serial_num_flag();
    /*如果是唯一sn设置sn IDX*/
    if (SERIAL_NUM_FLAG_UNIQUE == unique_serial_num_flag)
    {
        DBG_I(MBB_PNP, "SERIAL_NUM_FLAG_UNIQUE\n");
        cdev->desc.iSerialNumber = strings_dev[STRING_SERIAL_IDX].id;
    }
}

/*****************************************************************
Parameters    :  cdev
                 port_mode
Return        :
Description   :  配置静态端口形态时的描述符
*****************************************************************/
USB_VOID pnp_s_port_config_dev_desc(struct usb_composite_dev* cdev,
        hw_usb_mode* port_mode)
{
    if( NULL == cdev || NULL == port_mode)
    {
        DBG_E(MBB_PNP, "cdev\%p OR  port_mode \%p NULL\n", cdev, port_mode);
		return;
    }
    //dev info only need to config 4 points.
    pnp_set_serialnumber_index(cdev);
    cdev->desc.idProduct = port_mode->PID;
    cdev->desc.bDeviceClass = port_mode->bDeviceClass;
    cdev->desc.bDeviceProtocol = 0x00;
}

/*****************************************************************
Parameters    :  cdev
                 port_mode
Return        :
Description   :  配置动态端口形态时的描述符
*****************************************************************/
USB_VOID pnp_d_port_config_dev_desc(
    struct usb_composite_dev* cdev, pnp_dynamic_mode* port_mode)
{
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    
    if( NULL == cdev || NULL ==  port_mode)
    {
        DBG_E(MBB_PNP, "cdev /%p  OR port_mode/%p  NULL !!\n", cdev, port_mode);
        return;
    }
    
    cdev->desc.idProduct = port_mode->PID;
    pnp_set_serialnumber_index(cdev);

    /*配置切换前信息*/
    if (pnp_port_style == g_pnp_info.current_port_style)
    {
        cdev->desc.bDeviceClass = 0x00;

        if ( MBB_NV_ENABLE == usb_nv_info->winblue_profile.MBIMEnable)
        {
            if ( MBB_NV_DISABLED == usb_nv_info->pid_info.nv_status)
            {
                DBG_E(MBB_PNP, "MBIM enabled !50071 disabled!!\n");
                cdev->desc.idProduct = PRODUCT_ID_CONFIG2;
            }

            cdev->desc.bDeviceProtocol = VENDOR_PRO;
        }
        else
        {
            cdev->desc.bDeviceProtocol = 0x00;
        }
    }
    else/*配置切换后信息*/
    {
        cdev->desc.bDeviceClass = 0x00;

#ifdef USB_RNDIS
        if ((MAC_OS_FLAG == g_pnp_info.rewind_param.bPcType)
            || (LINUX_OS_FLAG == g_pnp_info.rewind_param.bPcType))
        {
            cdev->desc.bDeviceClass = USB_CLASS_COMM;
        }
#endif
        cdev->desc.bDeviceProtocol = 0x00;
    }
    DBG_I(MBB_PNP, "port_index%d pid %x\n", 
        port_mode->port_index,cdev->desc.idProduct);
    
}

/*****************************************************************
Parameters    :  androiddev
Return        :
Description   :  配置设备的描述符
*****************************************************************/
USB_VOID pnp_config_dev_desc_info(struct android_dev* androiddev)
{
    hw_usb_mode* static_port_mode = NULL;
    pnp_dynamic_mode* dynamic_port_mode = NULL;
    USB_INT PortModeIndex = g_pnp_info.portModeIndex;
    struct usb_composite_dev* cdev = NULL;
    
    if( NULL == androiddev )
    {
        DBG_E(MBB_PNP, "androiddev NULL\n");
        return;
    }
    cdev = androiddev->cdev;
    
    if ( PortModeIndex < DYNAMIC_REWIND_BEFORE_MODE )
    {
        static_port_mode = pnp_static_port_get_mode(PortModeIndex);
        pnp_s_port_config_dev_desc(cdev, static_port_mode);
    }
    else
    {
        dynamic_port_mode = pnp_d_port_get_mode(PortModeIndex);
        pnp_d_port_config_dev_desc(cdev, dynamic_port_mode);
    }
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  配置设备信息
*****************************************************************/
USB_VOID pnp_config_dev(USB_VOID)
{
    struct android_dev* dev = android_get_android_dev();
    DBG_I(MBB_PNP, "\n");
    
    if( NULL == dev )
    {
        DBG_E(MBB_PNP, "dev NULL\n");
        return;
    }
    
    pnp_bind_port_group2config(dev);

    pnp_config_dev_desc_info(dev);
}

USB_VOID pnp_switch_func(struct work_struct* w)
{
    struct android_dev* dev = NULL;
    struct usb_composite_dev* cdev = NULL;
    USB_INT err = -1;
    USB_INT  soft_delyay = 0;

    DBG_I(MBB_PNP, "\n");
    dev = android_get_android_dev();

    if ( NULL == dev)
    {
        DBG_E(MBB_PNP, "android_get_android_dev NULL!\n");
        return ;
    }

    if ( MBB_USB_ENABLE != dev->irq_ctl_port)
    {
        DBG_E(MBB_PNP, "irq_ctl_port is not enable!\n");
        return;
    }
    /*pnp_probe compete with the pnp_remove ,
      mutex the android_dev , and g_pnp_info */
    mutex_lock(&dev->mutex);

    cdev = dev->cdev;
    if ( NULL == cdev)
    {
        DBG_E(MBB_PNP, "dev->cdev NULL!\n");
        goto switch_exit ;
    }

    /*disconnect with PC first*/
    usb_gadget_disconnect(cdev->gadget);
    /* Cancel pending control requests */
    usb_ep_dequeue(cdev->gadget->ep0, cdev->req);

    /*if dev is enabled disabled it*/
    android_disable(dev);

    pnp_usb_init_enum_stat();
    pnp_set_setmode_state(SETMODE_UNKNOWN_STATE);
    pnp_config_dev();

    huawei_set_usb_enum_state(USB_ENUM_NONE);

    err = android_enable(dev);
    soft_delyay = pnp_get_soft_delay();
    /*让D+从拉低到拉高的时延符合USB3.0协议*/
    mdelay(soft_delyay);
    if (( 0 == dev->disable_depth ) && (!err))
    {
        /*the cdev maybe not the same with the start of pnp_switch_func
          because of maybe remove and reinsert.*/
        cdev = dev->cdev;
        err = pnp_usb_state_get();
        if ((0 != err) && (NULL != cdev))
        {
            usb_gadget_connect(cdev->gadget);
            dev->enabled = MBB_USB_TRUE;
        }
        else
        {
            DBG_T(MBB_PNP, "android_enable fail err=%d\n", err);
            android_disable(dev);
        }
    }
    else
    {
        DBG_T(MBB_PNP, "android_enable fail dev->disable_depth=%d,err=%d\n",
              dev->disable_depth, err);
        huawei_set_usb_enum_state(USB_ENUM_DONE);
        android_disable(dev);
    }
switch_exit:
    mutex_unlock(&dev->mutex);
    DBG_I(MBB_PNP, "pnp_switch_func success!\n");
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  pnp切换
*****************************************************************/
USB_VOID pnp_notify_to_switch(USB_VOID)
{
    pnp_usb_queue_delay_work(&pnp_switch_work, 0);
    pnp_mod_soft_reconnect_timer();
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  连接windows时，更新ECM为rndis
*****************************************************************/
USB_VOID pnp_hilink_up2rndis(USB_VOID)
{
    USB_INT i = 0;
    pnp_dynamic_mode* rewind_after_mode = NULL;

    DBG_I(MBB_PNP, "\n");
    rewind_after_mode = pnp_d_port_get_mode(DYNAMIC_REWIND_AFTER_MODE);
    if (NULL != rewind_after_mode)
    {
        for (i = 0; i < MAX_TOTAL_PORT_NUM; i++)
        {
            if ( NULL != rewind_after_mode->mode_buf[i] )
            {
                if (!strcmp(rewind_after_mode->mode_buf[i], "ecm"))
                {
                    rewind_after_mode->mode_buf[i] = "rndis";
                    break;
                }
            }
            else
            {
                break;
            }
        }

    }

}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  链接linux或者MAC时更新rndis为ECM
*****************************************************************/
USB_VOID pnp_hilink_up2ecm(USB_VOID)
{
    USB_INT i = 0;
    pnp_dynamic_mode* rewind_after_mode = NULL;

    DBG_I(MBB_PNP, "\n");
    rewind_after_mode = pnp_d_port_get_mode(DYNAMIC_REWIND_AFTER_MODE);
    if (NULL != rewind_after_mode)
    {
        for (i = 0; i < MAX_TOTAL_PORT_NUM; i++)
        {
            if ( NULL != rewind_after_mode->mode_buf[i] )
            {
                if (!strcmp(rewind_after_mode->mode_buf[i], "rndis"))
                {
                    rewind_after_mode->mode_buf[i] = "ecm";
                    break;
                }
            }
            else
            {
                break;
            }
        }

    }

}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  (高通 balong 都未使用该函数，评审时确认是否删除)
*****************************************************************/
USB_INT pnp_set_gateway_mode(USB_VOID)
{
    USB_INT ret = PNP_CHECK_MODE_CONTINUE;
    
    switch (g_pnp_info.rewind_param.bProFile)
    {
        case GATEWAT_MODEM_MODE:
            DBG_T(MBB_PNP, "gateway GATEWAT_MODEM_MODE\n");
            g_pnp_info.portModeIndex = MODEM_DIAG_PCUI_GATEWAY;
            ret = PNP_CHECK_MODE_COMPLETE;
            break;
        case GATEWAY_NDIS_MODE:
            DBG_T(MBB_PNP, "gateway GATEWAY_NDIS_MODE\n");
            g_pnp_info.portModeIndex = MODEM_DIAG_PCUI_NCM_GATEWAY;
            ret = PNP_CHECK_MODE_COMPLETE;
            break;
        default:
            DBG_E(MBB_PNP, "unknown gateway rewind command\n");
            break;
    }
    
    return ret;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  hilink产品判断hilink模式还是modem模式
*****************************************************************/
USB_INT pnp_hilink_mode_modem( USB_VOID )
{
    USB_INT ret = PNP_CHECK_MODE_CONTINUE;
#ifdef USB_HILINK_MODEM_SWITCH
    pnp_read_hilink_modem_flag();
#endif
    DBG_I(MBB_PNP, "\n");

    if ( HILINK_MODEM_MODE == g_pnp_info.hilink_mode )
    {
        g_pnp_info.portModeIndex = HILINK_PORT_MODEM_MODE;
        DBG_T(MBB_PNP, "HILINK_PORT_MODEM_MODE\n");
        ret = PNP_CHECK_MODE_COMPLETE;
    }
    else if (HILINK_MODEM_DEBUG_MODE == g_pnp_info.hilink_mode )
    {
        g_pnp_info.portModeIndex = HILINK_PORT_MODEM_DEBUG_MODE;
        DBG_T(MBB_PNP, "HILINK_PORT_MODEM_DEBUG_MODE\n");
        ret = PNP_CHECK_MODE_COMPLETE;
    }
    return ret;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  hilink产品切换到正常的工作模式
*****************************************************************/
USB_INT pnp_hilink_mode_work( USB_VOID )
{
    USB_INT ret = PNP_CHECK_MODE_CONTINUE;
    DBG_I(MBB_PNP, "\n");
    if (work_port_mode == g_pnp_info.rewind_param.bGreenMode)
    {
        if ((MAC_OS_FLAG == g_pnp_info.rewind_param.bPcType)
            || (LINUX_OS_FLAG == g_pnp_info.rewind_param.bPcType))
        {
            pnp_hilink_up2ecm();
        }
        else
        {
            pnp_hilink_up2rndis();
        }

        /*rewind with work mode let APP start*/
        pnp_set_ctl_app_flag(CTL_APP_START);

        g_pnp_info.portModeIndex = DYNAMIC_REWIND_AFTER_MODE;
        DBG_T(MBB_PNP, "rndis work mode\n");
        ret = PNP_CHECK_MODE_COMPLETE;
    }
    else
    {
        DBG_E(MBB_PNP, "PNP_CHECK_MODE_ERR unknown gateway rewind command\n");
        ret = PNP_CHECK_MODE_ERR;
    }
    return ret;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  hilink产品切换到升级模式(一般是产线环境)
*****************************************************************/
USB_INT pnp_hilink_mode_dload( USB_VOID )
{
    DBG_I(MBB_PNP, "\n");
    if ((WINDOWS_OS_FLAG == g_pnp_info.rewind_param.bPcType)
        && (product_port_mode == g_pnp_info.rewind_param.bGreenMode))
    {
        g_pnp_info.current_port_style = download_port_style;
        g_pnp_info.portModeIndex = PCUI_DIAG_MODE;
        DBG_T(MBB_PNP, "rndis dload mode\n");
        return PNP_CHECK_MODE_COMPLETE;
    }
    return PNP_CHECK_MODE_CONTINUE;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  是否是网关切换判断
*****************************************************************/
USB_INT pnp_rewind_gateway_mode(USB_VOID)
{
    USB_INT ret = PNP_CHECK_MODE_CONTINUE;
#ifdef USB_GATEWAY_MODE
    if (GATEWAY_OS_FLAG == g_pnp_info.rewind_param.bPcType)
    {
        switch (g_pnp_info.rewind_param.bProFile)
        {
            case GATEWAT_MODEM_MODE:
                DBG_T(MBB_PNP, "gateway GATEWAT_MODEM_MODE\n");
                g_pnp_info.portModeIndex = MODEM_DIAG_PCUI_GATEWAY;
                ret = PNP_CHECK_MODE_COMPLETE;
                break;
            case GATEWAY_NDIS_MODE:
                DBG_T(MBB_PNP, "gateway GATEWAY_NDIS_MODE\n");
                g_pnp_info.portModeIndex = MODEM_DIAG_PCUI_NCM_GATEWAY;
                ret = PNP_CHECK_MODE_COMPLETE;
                break;
            default:
                DBG_E(MBB_PNP, "PNP_CHECK_MODE_ERR unknown gateway rewind command\n");
                ret = PNP_CHECK_MODE_ERR;
                break;
        }
        DBG_I(MBB_PNP, "go gateway mode %d\n", g_pnp_info.portModeIndex);
    }
    else
    {
        DBG_I(MBB_PNP, "PNP_CHECK_MODE_CONTINUE\n");
    }
#endif
    return ret;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  hilink产品切换后模式选择
*****************************************************************/
USB_INT pnp_rewind_hilink_mode(USB_VOID)
{
    USB_INT ret = PNP_CHECK_MODE_CONTINUE;
    ret = pnp_hilink_mode_dload();
    if ( PNP_CHECK_MODE_CONTINUE != ret)
    {
        return ret;
    }
    ret = pnp_hilink_mode_modem();
    if ( PNP_CHECK_MODE_CONTINUE != ret)
    {
        return ret;
    }
    ret = pnp_hilink_mode_work();
    return ret;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  默认的切换后模式
*****************************************************************/
USB_INT pnp_rewind_default_mode(USB_VOID)
{
    g_pnp_info.portModeIndex = DYNAMIC_REWIND_AFTER_MODE;
    pnp_set_ctl_app_flag(CTL_APP_START);
    return PNP_CHECK_MODE_COMPLETE;/*default return COMPLETE*/
}

/*按优先级check切换后的端口形态*/
USB_INT (* pnp_check_mode_rewind[])(USB_VOID) =
{
    pnp_rewind_gateway_mode,
#ifdef USB_RNDIS
    pnp_rewind_hilink_mode,
#endif
    pnp_rewind_default_mode,
    NULL
};

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  选择切换后的工作模式
*****************************************************************/
USB_INT pnp_check_rewind_mode(USB_VOID)
{
    USB_INT i = 0;
    USB_INT ret = PNP_CHECK_MODE_CONTINUE;

    while (NULL != pnp_check_mode_rewind[i])
    {
        ret = pnp_check_mode_rewind[i]();
        /*选择切换模式*/
        if (PNP_CHECK_MODE_CONTINUE != ret )
        {
            break;
        }
        i++;
    }
    return ret;
}

/*****************************************************************
Parameters    :  w
Return        :
Description   :  完成从光盘模式切换的功能
*****************************************************************/
USB_VOID pnp_notify_rewind_after_func(struct work_struct* w)
{
    USB_INT err = MBB_USB_ERROR;
    DBG_I(MBB_PNP, "\n");
    err = pnp_check_rewind_mode();

    if ( PNP_CHECK_MODE_COMPLETE == err)
    {
        pnp_notify_to_switch();
    }
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  rewind方式切换的接口
*****************************************************************/
USB_VOID pnp_switch_rewind_after_mode(USB_VOID)
{
    /*先更新状态，再切换*/
    g_pnp_info.is_mbim_enable = 0;  /* rewind切换后没有mbim模式 */
    g_pnp_info.current_port_style = rewind_port_style;
    pnp_usb_queue_delay_work(&pnp_notify_rewind_after_work, MBB_USB_SWITCH_DELAY_MS);
}


/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  切换为mbim_debug模式的接口
*****************************************************************/
USB_VOID pnp_switch_mbim_debug_mode(USB_VOID)
{
    DBG_I(MBB_PNP, "\n");
    g_pnp_info.portModeIndex = MBIM_SWITCH_DEBUG_MODE;
    pnp_notify_to_switch();
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  切换为rndis_debug模式的接口
*****************************************************************/
USB_VOID pnp_switch_rndis_debug_mode(USB_VOID)
{
    DBG_I(MBB_PNP, "\n");
    g_pnp_info.portModeIndex =
        pnp_select_rndis_full_sence(g_pnp_info.rewind_param.bPcType);
    pnp_notify_to_switch();
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  切换为rndis 升级模式的接口
*****************************************************************/
USB_VOID pnp_switch_rndis_project_mode(USB_VOID)
{
    DBG_I(MBB_PNP, "\n");
    g_pnp_info.portModeIndex  = PCUI_DIAG_MODE;
    pnp_notify_to_switch();
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  调用此接口，切换到pnp默认状态
*****************************************************************/
USB_VOID pnp_switch_autorun_port(USB_VOID)
{
    DBG_I(MBB_PNP, "enter\n");
    pnp_probe();
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  切换到仅充电模式接口
*****************************************************************/
USB_VOID pnp_switch_charge_only_port(USB_VOID)
{
    DBG_I(MBB_PNP, "\n");
    g_pnp_info.current_port_style   = charge_port_style;
    /*仅充电报关机充电口，光盘可读*/
    g_pnp_info.portModeIndex   = POWER_OFF_MODE;
    pnp_notify_to_switch();
}

/*****************************************************************
Parameters    :  mode
Return        :
Description   :  mbim切换接口balong平台由NCM调用实现切换，高通平台不使用此接口
*****************************************************************/
USB_VOID pnp_switch_mbim_mode(USB_INT mode)
{
#ifdef CONFIG_NCM_MBIM_SUPPORT
    DBG_I(MBB_PNP, "\n");

    if (MBIM_SWITCH_DEBUG_MODE == mode)
    {
        pnp_switch_mbim_debug_mode();
    }
    else if (MBIM_SWITCH_NORMAL_MODE == mode)
    {
        pnp_switch_autorun_port();
    }
    else
    {
        DBG_E(MBB_PNP, "Invalid mode %u\n", mode);
    }
#else
    DBG_I(MBB_PNP, "no CONFIG_NCM_MBIM_SUPPORT %d\n", mode);
#endif
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   : (dynamic port )配置mbim端口
*****************************************************************/
USB_VOID pnp_d_port_config_mbim(USB_VOID)
{
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    USB_INT i = 0;
    USB_INT mass_enable = 0;

    DBG_I(MBB_PNP, "enter\n");

    if ( usb_nv_info->winblue_profile.CdRom)
    {
        DBG_I(MBB_PNP, "winblue_profile.CdRom enable\n");
        mass_function_add_lun("mass_two", "CD");
        mass_enable = 1;
    }

    if (usb_nv_info->winblue_profile.TCard)
    {
        DBG_I(MBB_PNP, "winblue_profile.TCard enable\n");
        mass_function_add_lun("mass_two", "SD");
        mass_enable = 1;
    }

    if (mass_enable)
    {
        for (i = 0; i < CONFIG_TWO_MAX_PORT_NUM; i++)
        {
            if (NULL != config_funcs_win8[i])
            {
                if (!strcmp(config_funcs_win8[i], "mass_two"))
                {
                    DBG_I(MBB_PNP, "config_win8 already have mass storage\n");
                    break;
                }
            }
            else
            {
                DBG_I(MBB_PNP, "config_win8 add mass storage\n");
                config_funcs_win8[i] = "mass_two";
                break;
            }
        }

        DBG_E(MBB_PNP, "config_win8 can not add mass\n");
        return;
    }

    DBG_I(MBB_PNP, "success\n");
}

/*****************************************************************
Parameters    :  port_mode
                 new_name
                 new_procotol
Return        :
Description   :  (dynamic port )向端口形态信息中添加
                 name和protocol信息
*****************************************************************/
USB_VOID pnp_d_port_add_info(pnp_dynamic_mode* port_mode,
                                    USB_CHAR* new_name, USB_UINT8 new_procotol)
{
    USB_INT i = 0;
    USB_CHAR** port_group_buf = NULL;
    USB_UINT8* port_procotol = NULL;

    if ( NULL != port_mode)
    {
        port_group_buf = port_mode->mode_buf;
        port_procotol = port_mode->port_protocol;
    }
    if ((NULL == port_group_buf) || (NULL == port_procotol))
    {
        DBG_E(MBB_PNP, "port_group_buf:%p ,port_procotol %p \n", port_group_buf, port_procotol);
        return;
    }
    for (i = 0; i < MAX_TOTAL_PORT_NUM; i++)
    {

        if ( NULL == port_group_buf[i])
        {
            port_group_buf[i] = new_name;
            port_procotol[i] =  new_procotol;

            DBG_I(MBB_PNP, "find %d to add new name:%s\n", i, new_name);
            return;
        }
    }
    
    DBG_E(MBB_PNP, "cannot add new name\n");

}

/*****************************************************************
Parameters    :  port_index
Return        :
Description   :  (dynamic port )清除该索引的端口信息
*****************************************************************/
USB_VOID pnp_d_port_clear_mode(USB_UCHAR port_index)
{
    USB_INT i = 0;

    for (i = 0; i < sizeof(g_pnp_dynamic_port_mode_info) / sizeof(pnp_dynamic_mode); i++)
    {
        if ( port_index == g_pnp_dynamic_port_mode_info[i].port_index)
        {
            DBG_I(MBB_PNP, "find port_index = %d in g_pnp_dynamic_port_mode_info[%d]\n",
                  port_index, i);
            pnp_d_port_mode_default(&(g_pnp_dynamic_port_mode_info[i]));
            return;
        }
    }
    DBG_E(MBB_PNP, "can not find port_index = %d\n", port_index);
}

/*****************************************************************
Parameters    :  port_index
Return        :
Description   :  (dynamic port )根据索引值，获取相应的动态端口信息
*****************************************************************/
pnp_dynamic_mode* pnp_d_port_get_mode(USB_UCHAR port_index)
{
    USB_INT i = 0;

    for (i = 0; i < sizeof(g_pnp_dynamic_port_mode_info) / sizeof(pnp_dynamic_mode); i++)
    {
        if ( port_index == g_pnp_dynamic_port_mode_info[i].port_index)
        {
            DBG_I(MBB_PNP, "find port_index = %d in g_pnp_dynamic_port_mode_info[%d]\n",
                  port_index, i);
            return &(g_pnp_dynamic_port_mode_info[i]);
        }
    }

    DBG_E(MBB_PNP, "can not find port_index = %d in g_pnp_dynamic_port_mode_info\n",
          port_index);
    return NULL;
}

/*****************************************************************
Parameters    :  case_name
Return        :
Description   :  (dynamic port 使用)
                 在设备信息中查询是否支持case_name对应的端口
*****************************************************************/
USB_CHAR* pnp_d_port_case_port_name(USB_CHAR* case_name)
{
    struct android_dev* dev = android_get_android_dev();
    struct android_usb_function** functions = NULL;
    struct android_usb_function* f;
    
    if( NULL == dev )
    {
        DBG_E(MBB_PNP, "dev NULL\n");
        return NULL;
    }
    
    functions = dev->functions;
    
    if (NULL == case_name)
    {
        return NULL;
    }

    while ((f = *functions++))
    {
        if (!strcmp(case_name, f->name))
        {
            DBG_I(MBB_PNP, "find name %s\n", f->name);
            return f->name;
        }
    }
    return NULL;
}

/*****************************************************************
Parameters    :  fd_name
Return        :
Description   :  (dynamic port 使用)获取fd_name对应的protocol信息
*****************************************************************/
USB_UINT8 pnp_d_port_get_port_protocol(USB_CHAR* fd_name)
{
    USB_UINT8 i = USB_IF_PROTOCOL_VOID;
    huawei_interface_info * interface_info =  pnp_get_huawei_interface_vector();
    if (NULL == fd_name)
    {
        DBG_E(MBB_PNP, "fd_name NULL\n");
        return USB_IF_PROTOCOL_VOID;
    }
    for (i = 0; i < MAX_INTERFACE_VECTOR_NUM; i++)
    {
        if ( NULL != interface_info[i].huawei_interface_name )
        {
            if (!strcmp(fd_name, interface_info[i].huawei_interface_name))
            {
                return interface_info[i].huawei_interface_protocol_num;
            }
        }
    }
    return USB_IF_PROTOCOL_VOID;
}

/*****************************************************************
Parameters    :  protocol_num
Return        :
Description   :  (dynamic port 使用)通过protocol获取FD名称
*****************************************************************/
USB_CHAR* pnp_d_port_get_port_name(USB_UINT8 port_index)
{
    USB_INT i = 0;
    huawei_interface_info * interface_info =  pnp_get_huawei_interface_vector();
    if (USB_IF_PROTOCOL_VOID == port_index)
    {
        DBG_E(MBB_PNP, "port_index NULL\n");
        return NULL;
    }

    for (i = 0; i < MAX_INTERFACE_VECTOR_NUM; i++)
    {
        if (port_index == interface_info[i].huawei_interface_protocol_num)
        {
            return interface_info[i].huawei_interface_name;
        }
    }

    return NULL;
}

/*****************************************************************
Parameters    :  dynamic_port_mode
Return        :
Description   :  (dynamic port 使用)动态端口形态信息初始化
*****************************************************************/
USB_VOID pnp_d_port_mode_default(pnp_dynamic_mode* dynamic_port_mode)
{
    USB_INT i = 0;

    if( NULL == dynamic_port_mode)
    {
        DBG_E(MBB_PNP, "dynamic_port_mode NULL\n");
        return;
    }
    
    for (i = 0; i < MAX_TOTAL_PORT_NUM; i++)
    {
        dynamic_port_mode->mode_buf[i] = NULL;
        dynamic_port_mode->port_protocol[i] = USB_IF_PROTOCOL_VOID;
    }
}

/*****************************************************************
Parameters    :  port_info
                 port_mode
Return        :
Description   :  根据protocol组合配置端口信息。
*****************************************************************/
USB_VOID pnp_complete_config_port_info(USB_UINT8* port_info, pnp_dynamic_mode* port_mode)
{
    USB_INT i;
    USB_INT rewind_before_mass_enable = 0;
    USB_UINT8 port_protocol = USB_IF_PROTOCOL_VOID;
    USB_CHAR* port_name = NULL;

    if (NULL == port_info || NULL == port_mode)
    {
        return;
    }

    for (i = 0; i < DYNAMIC_PID_MAX_PORT_NUM; i++)
    {
        port_protocol = port_info[i];

        if ( USB_IF_PROTOCOL_VOID == port_protocol)
        {
            DBG_I(MBB_PNP, "end with %d\n", i - 1);
            break;
        }

        switch (port_protocol)
        {
            case USB_IF_PROTOCOL_CDROM:
                mass_function_add_lun("mass", "CD");

                if ( 0 == rewind_before_mass_enable)
                {
                    DBG_I(MBB_PNP, "protocol %x name %s\n", port_protocol, port_name);
                    pnp_d_port_add_info(port_mode, "mass", port_protocol);
                }
                rewind_before_mass_enable = 1;
                break;

            case USB_IF_PROTOCOL_SDRAM:
                mass_function_add_lun("mass", "SD");

                if ( 0 == rewind_before_mass_enable)
                {
                    DBG_I(MBB_PNP, "protocol %x name %s\n", port_protocol, port_name);
                    pnp_d_port_add_info(port_mode, "mass", port_protocol);
                }
                rewind_before_mass_enable = 1;
                break;

            default:
                port_name = pnp_d_port_get_port_name(port_protocol);
                DBG_I(MBB_PNP, "protocol %x name %s\n", port_protocol, port_name);
                pnp_d_port_add_info(port_mode, port_name, port_protocol);
                break;
        }
    }

    return;
}

/*****************************************************************
Parameters    :  port_mode
Return        :
Description   :  (dynamic port 使用)在linux或这MAC系统上，将RNDIS更新为ECM
*****************************************************************/
USB_VOID pnp_d_port_up_rndis2ecm( pnp_dynamic_mode* port_mode)
{
    USB_CHAR** port_group_buf = NULL;
    USB_INT i = 0;
    USB_CHAR* port_name = NULL;

    if (NULL == port_mode)
    {
        DBG_E(MBB_PNP, "port_mode NULL \n");
        return;
    }

    for ( i = MAX_TOTAL_PORT_NUM -1 ; i  >= 0; i--)
    {
        if ( NULL != port_mode->mode_buf[i])
        {
            if(i < DYNAMIC_PID_MAX_PORT_NUM)
            {
               port_group_buf =&(port_mode->mode_buf[0]);
                DBG_I(MBB_PNP, "found mode_buf head %s\n",port_mode->mode_buf[i]);
            }
            else
            {
                port_group_buf = &(port_mode->mode_buf[i-DYNAMIC_PID_MAX_PORT_NUM]);
                DBG_I(MBB_PNP, "found port_group %d end name%s\n",
                    i-DYNAMIC_PID_MAX_PORT_NUM,port_mode->mode_buf[i]);
            }
            break;
        }
    }
	/*for coveriy*/
    if (NULL != port_group_buf)
    {
        for (i = DYNAMIC_PID_MAX_PORT_NUM - 1; i >= 0 ; i--)
        {
            port_name = port_group_buf[i];

            if (NULL == port_name)
            {
                DBG_I(MBB_PNP, "port_name NULL %d\n", i);
                continue;
            }

            if (!strcmp(port_name, ":"))
            {
                DBG_I(MBB_PNP, "next config with %d\n", i);
                continue;
            }

            if (!strcmp(port_name, "rndis"))
            {
                DBG_I(MBB_PNP, "up rndis to ecm in seat %d \n", i);
                port_group_buf[i] = "ecm";
                return;
            }
        }

    }

    DBG_I(MBB_PNP, "do not need to up \n");
}

/*****************************************************************
Parameters    :  port_mode
Return        :
Description   :  (dynamic port 使用)直接上报3config的产品形态，配置相应的端口信息
*****************************************************************/
USB_VOID pnp_d_port_direct_port(pnp_dynamic_mode* port_mode)
{
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    USB_UINT8 *port_protocol_info = NULL;
    USB_INT have_pre_config = MBB_USB_NO;
    USB_INT config_num = 0;
    
    port_mode->PID = usb_nv_info->dynamic_pid_info.dynamic_normal_pid;

    if ((MBB_NV_ENABLE == usb_nv_info->debug_mode.nv_status)
        && (NCM_MBIM_MODE_DEBUG == usb_nv_info->debug_mode.setmode_flag ))
    {
        if (MBB_NV_ENABLE == usb_nv_info->debug_pid_info.nv_status)
        {
            port_mode->PID = usb_nv_info->debug_pid_info.dynamic_debug_pid;
        }
        else
        {
            port_mode->PID = DYNAMIC_DEBUG_PID;
        }
    }

    DBG_I(MBB_PNP, "direct_port_mode  PID:0x%x\n", port_mode->PID);

    /*配置第一个config*/
    port_protocol_info = &(usb_nv_info->multi_config_port_info.first_config_port[0]);
    if( USB_IF_PROTOCOL_VOID != port_protocol_info[0])
    {
        pnp_complete_config_port_info(port_protocol_info, port_mode);
        config_num++;
        DBG_I(MBB_PNP, "add %d  config\n",config_num );
        have_pre_config = MBB_USB_YES;
    }
    else
    {
        DBG_I(MBB_PNP, " do not have protocol_info  \n");
    }

    
    /*配置第二个config linux 使用*/
    port_protocol_info = &(usb_nv_info->multi_config_port_info.second_config_port[0]);
    if( USB_IF_PROTOCOL_VOID != port_protocol_info[0])
    {
        if ( MBB_USB_YES == have_pre_config)
        {
            /*增加多config 分隔符*/
            pnp_d_port_add_info(port_mode, ":", USB_IF_PROTOCOL_VOID);
        }
        pnp_complete_config_port_info(port_protocol_info, port_mode);
        /*linux 使用的config 配置时配置为RNDIS端口*/
        /*，在绑定时需要更新为ECM                         */
        pnp_d_port_up_rndis2ecm(port_mode);
        config_num++;
        DBG_I(MBB_PNP, "add %d  config\n",config_num );
        have_pre_config = MBB_USB_YES;
    }
    else
    {
        DBG_I(MBB_PNP, " do not have protocol_info  \n");
    }
    
    /*配置第三个config*/
    port_protocol_info = &(usb_nv_info->multi_config_port_info.third_config_port[0]);
    if( USB_IF_PROTOCOL_VOID != port_protocol_info[0])
    {
        if ( MBB_USB_YES == have_pre_config)
        {
            /*增加多config 分隔符*/
            pnp_d_port_add_info(port_mode, ":", USB_IF_PROTOCOL_VOID);
        }
        g_pnp_info.is_mbim_enable = 1;
        pnp_complete_config_port_info(port_protocol_info, port_mode);
        config_num++;
        DBG_I(MBB_PNP, "add %d  config\n",config_num );
    }
    else
    {
        DBG_I(MBB_PNP, "do not have protocol_info\n");
    }
    
}

/*****************************************************************
Parameters    :  port_mode
Return        :
Description   :  (dynamic port 使用)配置切换前的端口形态信息
*****************************************************************/
USB_VOID pnp_d_port_rewind_before(pnp_dynamic_mode* port_mode)
{
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    USB_INT i = 0;
    USB_UINT8 *port_protocol_info = NULL;
    
    port_mode->PID = usb_nv_info->pid_info.cust_first_pid;
    DBG_I(MBB_PNP, "rewind_before_mode  PID:%x\n", port_mode->PID);

    /*配置第一个config*/
    port_protocol_info = &(usb_nv_info->dev_profile_info.first_port_style[0]);

    pnp_complete_config_port_info(port_protocol_info, port_mode);

    if ( MBB_NV_ENABLE == usb_nv_info->winblue_profile.MBIMEnable)
    {
        /*如果MBIM功能开启配置第二个config*/
        g_pnp_info.is_mbim_enable = 1;

        /*不通过procotol配置端口的功能不需要记录protocol*/
        pnp_d_port_add_info(port_mode, ":", USB_IF_PROTOCOL_VOID);

        pnp_d_port_config_mbim();

        for (i = 0; i < CONFIG_TWO_MAX_PORT_NUM; i++)
        {
            if (NULL != config_funcs_win8[i])
            {
                /*不通过procotol配置端口的功能不需要记录protocol*/
                pnp_d_port_add_info(
                    port_mode, config_funcs_win8[i], USB_IF_PROTOCOL_VOID);
            }
            else
            {
                break;
            }
        }
    }
}


/*****************************************************************
Parameters    :  port_mode
Return        :
Description   :  (dynamic port 使用)配置切换后的端口形态信息
*****************************************************************/
USB_VOID pnp_d_port_rewind_after(pnp_dynamic_mode* port_mode)
{
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    USB_INT i = 0;
    USB_UINT8 port_protocol = USB_IF_PROTOCOL_VOID;
    USB_CHAR* port_name = NULL;

    port_mode->PID = usb_nv_info->pid_info.cust_rewind_pid;
    DBG_I(MBB_PNP, "rewind_after_mode  PID:%x\n", port_mode->PID);

    for (i = 0; i < DYNAMIC_PID_MAX_PORT_NUM; i++)
    {
        port_protocol = usb_nv_info->dev_profile_info.rewind_port_style[i];

        DBG_I(MBB_PNP, "rewind_port_style  %d\n", port_protocol);

        if ( USB_IF_PROTOCOL_VOID == port_protocol)
        {
            DBG_I(MBB_PNP, "rewind_port_style end with %d\n", i);
            break;
        }

        port_name = pnp_d_port_get_port_name(port_protocol);
        pnp_d_port_add_info(port_mode, port_name, port_protocol);
    }

    /*config the eth dev according to the OS*/
    if ((MAC_OS_FLAG == g_pnp_info.rewind_param.bPcType)
        || (LINUX_OS_FLAG == g_pnp_info.rewind_param.bPcType))
    {
        pnp_hilink_up2ecm();
    }
    else
    {
        pnp_hilink_up2rndis();
    }
}

/*****************************************************************
Parameters    :  portindex
Return        :
Description   :  根据USB NV可配置项中的信息配置设备
*****************************************************************/
USB_VOID pnp_d_port_get_info_from_NV(USB_INT portindex)
{
    pnp_dynamic_mode* port_mode = NULL;

    DBG_I(MBB_PNP, "enter\n");

    /*check daynamic_sn NV */
    pnp_check_daynamic_sn_flag();

    mass_cleanup_alllun_info();

    port_mode = pnp_d_port_get_mode(portindex);
    if ( NULL == port_mode)
    {
        DBG_E(MBB_PNP, "port_mode NULL\n");
        return;
    }
    /*init the dynamic port mode before config it*/
    pnp_d_port_mode_default(port_mode);

    /*配置传统数据卡类产品切换后的端口组合*/
    if (DYNAMIC_REWIND_AFTER_MODE == portindex)
    {
        pnp_d_port_rewind_after(port_mode);
    }
    /*配置传统数据卡类产品切换前的端口组合*/
    else if (DYNAMIC_REWIND_BEFORE_MODE == portindex)
    {
        pnp_d_port_rewind_before(port_mode);
    }
    /*配置模块的端口组合*/
    else if (DYNAMIC_DIRECT_PORT_MODE == portindex)
    {
        pnp_d_port_direct_port(port_mode);
    }
    else
    {
        DBG_I(MBB_PNP, "port index %d do not config from NV\n", portindex);
    }
    DBG_I(MBB_PNP, "success\n");
}

/*****************************************************************
Parameters    :  file
                 buf
                 size
                 ppos
Return        :
Description   :  app启动前 读取控制app 是否驱动标志
*****************************************************************/
ssize_t pnp_ctl_app_start_flag_read( struct file* file, char __user* buf, size_t size, loff_t* ppos )
{
    ssize_t   len  = 0;
    unsigned long iRet = 0;
    USB_CHAR  mode_buf[APP_MODE_FLAG_LEN] = {0};

    if ((APP_MODE_FLAG_LEN > size) || (NULL == buf))
    {
        DBG_E(MBB_PNP, "Invalid param buf(%p) size(%u),*ppos(%lld).\n", buf,size,*ppos);
        return 0;
    }

    if (0 != *ppos)
    {
        DBG_I(MBB_PNP, "Invalid param buf(%p) size(%u),*ppos(%lld).\n", buf,size,*ppos);
        return 0;
    }

    memset( mode_buf, 0, APP_MODE_FLAG_LEN );
    snprintf( mode_buf, APP_MODE_FLAG_LEN, "%d", g_pnp_info.app_start_mode);
    len = strlen(mode_buf) + 1;

    iRet = copy_to_user( buf, mode_buf, len );

    if (0 != iRet)
    {
        DBG_E(MBB_PNP, "copy_to_user failed.\n");
        return 0;
    }

    DBG_I(MBB_PNP, "gAppStartMode=%u\n", g_pnp_info.app_start_mode);
    *ppos = len;
    return len;

}

/*****************************************************************
Parameters    :  file
                 buf
                 size
                 ppos
Return        :
Description   :  写入app是否启动标志，用来调试使用。驱动内部直接修改对应的标志
*****************************************************************/
ssize_t pnp_ctl_app_start_flag_write( struct file* file, const char __user* buf,
                                      size_t size, loff_t* ppos )
{
    unsigned long iRet = 0;
    USB_CHAR  mode_buf[APP_MODE_FLAG_LEN] = {0};

    memset( mode_buf, 0, APP_MODE_FLAG_LEN );

    iRet = copy_from_user( mode_buf, buf, 1 );

    if (iRet)
    {
        DBG_E(MBB_PNP, "copy_from_user failed err:%ld\n", iRet);
        return -EFAULT;
    }

    DBG_E(MBB_PNP, "%s\n", mode_buf);

    switch (mode_buf[APP_WRITE_DATA_SITE])
    {
        case CTL_APP_START_FALG  :
            pnp_set_ctl_app_flag(CTL_APP_START);
            break;

        case CTL_APP_DISABLE_FALG:
            pnp_set_ctl_app_flag(CTL_APP_DISABLE);
            break;

        case CTL_APP_WAIT_FALG   :
            pnp_set_ctl_app_flag(CTL_APP_WAIT);
            break;

        default:
            return -EFAULT;
    }

    return size;
}

static const struct file_operations app_start_mode_proc_fops =
{
    .owner      = THIS_MODULE,
    .read       = pnp_ctl_app_start_flag_read,
    .write      = pnp_ctl_app_start_flag_write,
};

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  控制app是否启动标志初始化
*****************************************************************/
USB_VOID usb_ctl_app_start_flag_init( USB_VOID )
{
    static struct proc_dir_entry* entry = NULL;

#if defined(USB_E5) || defined(USB_CPE) || defined(USB_WINGLE) || defined(USB_SOLUTION)
    pnp_set_ctl_app_flag(CTL_APP_START);
#else
    pnp_set_ctl_app_flag(CTL_APP_WAIT);
#endif

    if (NULL == entry)
    {
        entry = proc_create( "gAppStartMode", S_IRUGO | S_IWUSR, NULL, &app_start_mode_proc_fops );
        DBG_I(MBB_PNP, "app_start_flag_proc_init. gAppStartMode=%d, entry=%p \n", g_pnp_info.app_start_mode, entry);
    }
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  默认的pnp启动接口
*****************************************************************/
USB_VOID pnp_probe(USB_VOID)
{
    struct android_dev* dev = android_get_android_dev();
    struct usb_composite_dev* cdev = NULL;
    DBG_I(MBB_PNP, "enter\n");

    /*the HW irq early wait the android_dev create */
    if ( NULL == dev)
    {
        DBG_E(MBB_PNP, "android_get_android_dev NULL!\n");
        return ;
    }

    cdev = dev->cdev;
    /*pnp_probe compete with the pnp_remove ,
      mutex the android_dev , and g_pnp_info */
    mutex_lock(&dev->mutex);
    /*读取NV*/
    usb_nv_get();
    pnp_init_device_descriptor(cdev);
    pnp_status_select_port_index();
    mutex_unlock(&dev->mutex);
    DBG_I(MBB_PNP, "success\n");
}
EXPORT_SYMBOL_GPL(pnp_probe);


/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  pnp_info恢复初始状态
*****************************************************************/
USB_VOID pnp_info_set_default(USB_VOID)
{
    /*do not clean*/
    //g_pnp_info.app_start_mode
    /*do not clean */
    //g_pnp_info.portModeIndex;
    /*do not clean */
    //g_pnp_info.hilink_mode;

    g_pnp_info.current_port_style = pnp_port_style;

    g_pnp_info.is_daynamic_sn = MBB_USB_FALSE;

    g_pnp_info.is_mbim_enable = MBB_USB_FALSE;

    g_pnp_info.is_service_switch = MBB_USB_FALSE;

    memset(&(g_pnp_info.rewind_param), USB_NUM_0, sizeof(struct rewind_cmd_param));
}

/*****************************************************************
Parameters    :  w
Return        :
Description   :  pnp移除处理函数下半部
*****************************************************************/
USB_VOID pnp_remove_func(struct work_struct* w)
{
    struct android_dev* dev = android_get_android_dev();

    DBG_I(MBB_PNP, "\n");

    if ( NULL == dev)
    {
        DBG_E(MBB_PNP, "android_get_android_dev NULL!\n");
        return ;
    }

    if ( MBB_USB_ENABLE != dev->irq_ctl_port)
    {
        DBG_E(MBB_PNP, "irq_ctl_port is not enable!\n");
        return;
    }
    /*pnp_remove_func compete with the pnp_switch_func ,
      mutex the android_dev , and g_pnp_info */

    mutex_lock(&dev->mutex);
    android_disable(dev);

    pnp_usb_init_enum_stat();

    pnp_set_setmode_state(SETMODE_UNKNOWN_STATE);

    pnp_info_set_default();

    huawei_set_usb_enum_state(USB_ENUM_NONE);

    /*拔出时，将网卡驱动状态设置为NOT_READY*/
    pnp_set_net_drv_state(NET_STATE_NOT_READY);

    mutex_unlock(&dev->mutex);
    DBG_I(MBB_PNP, "success\n");
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  pnp移除接口
*****************************************************************/
USB_VOID pnp_remove(USB_VOID)
{
    struct android_dev* dev = android_get_android_dev();

    /*the HW irq early wait the android_dev create */
    if ( NULL == dev)
    {
        DBG_E(MBB_PNP, "android_get_android_dev NULL!\n");
        return ;
    }

    DBG_I(MBB_PNP, "\n");
    /*pnp_probe compete with the pnp_remove ,
      mutex the android_dev , and g_pnp_info */
    mutex_lock(&dev->mutex);
    /*异步调用*/
    pnp_usb_queue_delay_work(&pnp_remove_work, 0);
    mutex_unlock(&dev->mutex);

    DBG_I(MBB_PNP, "success\n");
}
EXPORT_SYMBOL_GPL(pnp_remove);

/*****************************************************************
Parameters    :  USB_VOID
Return        :    
Description   :  PNP移除函数，将此函数连接到其他模块可调起PNP。
                       用来同步调用 ，防止底层释放上层还没处理完成。
*****************************************************************/
USB_VOID pnp_remove_direct(USB_VOID)
{
    struct android_dev* dev = android_get_android_dev();

    /*the HW irq early wait the android_dev create */
    if ( NULL == dev)
    {
        DBG_E(MBB_PNP, "android_get_android_dev NULL!\n");
        return ;
    }

    DBG_I(MBB_PNP, "\n");
    pnp_remove_func(&pnp_remove_work);
}
EXPORT_SYMBOL_GPL(pnp_remove_direct);

/*=============================================================
PNP USB请求功能模块
==============================================================*/

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  查询当前配置的端口中是否支持rndis
                 用以控制回复部分MSOS命令
*****************************************************************/
USB_INT pnp_is_rndis_present(USB_VOID)
{
    struct android_dev* dev = android_get_android_dev();
    struct android_configuration* conf = NULL;
    struct android_usb_function* f;
    USB_INT ret = MBB_USB_FALSE;
    
    if ( NULL == dev)
    {
        DBG_E(MBB_PNP, "android_get_android_dev NULL!\n");
        return ret;
    }
    
    list_for_each_entry(conf, &dev->android_configs, list_item)
    {
        list_for_each_entry(f, &conf->enabled_functions, enabled_list)
        {
            if (!strcmp("rndis", f->name))
            {
                DBG_T(MBB_PNP, "enabled_function have %s\n", f->name);
                ret = MBB_USB_TRUE;
                break;
            }
        }
    }
    return ret;
}

/*微软文档中截取的containter_ID*/
static USB_UINT8 containter_ID[] =
{
    0x65, 0xe9, 0x36, 0x4d,
    0x25, 0xe3,
    0xce, 0x11,
    0xbf, 0xc1,
    0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18
};

/*****************************************************************
Parameters    :  msosctrl
                 buf
                 id
Return        :
Description   :  微软的msos命令处理函数
*****************************************************************/
static USB_INT  process_msos_request(const struct  usb_ctrlrequest* msosctrl,
                                     USB_PVOID buf,
                                     USB_INT id)
{
    ms_extend_compact_id_header header;
    ms_extend_container_id* pContainerID = NULL;
    ms_extend_compact_id_function Function;
    ms_extend_container_id containerID;
    USB_INT wLength = 0;
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();

    switch (msosctrl->wIndex)
    {
        case MS_EXTEND_COMPAT_ID:
            DBG_I(MBB_PNP, "--->MS_EXTEND_COMPAT_ID received!\n");
            memset(&header, 0x00, sizeof(ms_extend_compact_id_header));

            header.dwLength = sizeof(ms_extend_compact_id_header);
            /*Version = 1.00*/
            header.bcdVersion = 0x0100;
            header.wIndex = 0x0004;
            header.bCount = 1;
            memset(&Function, 0, sizeof(ms_extend_compact_id_function));
            Function.bFirstInterfaceNumber = 0;
            Function.bInterfaceCount = 0x01;
            memcpy(Function.compatibleID, "ALTRCFG", 7);
            Function.subCompatibleID[0] = ALTRCFG_1;

            if (MBB_NV_ENABLE == usb_nv_info->winblue_profile.MBIMEnable)
            {
                Function.subCompatibleID[0] = ALTRCFG_2;
            }
            
            /*模块配置端口的NV开启时，MBIM为config3*/
            if (MBB_NV_ENABLE == usb_nv_info->multi_config_port_info.nv_status)
            {
                Function.subCompatibleID[0] = ALTRCFG_3;
            }

            if (pnp_is_rndis_present())
            {
                DBG_I(MBB_PNP, "rndis is present!\n");
                memset(&Function.compatibleID, 0, 8);
                memcpy(Function.compatibleID, "RNDIS", 5);
                Function.subCompatibleID[0] = 0;
            }

            memcpy(buf + header.dwLength, &Function, sizeof(ms_extend_compact_id_function));
            header.dwLength += sizeof(ms_extend_compact_id_function);
            memcpy(buf, &header, sizeof(ms_extend_compact_id_header));
            wLength = header.dwLength;
            break;

        case MS_EXTEND_CONTAINER_ID:
            DBG_I(MBB_PNP, "--->MS_EXTEND_CONTAINER_ID received!\n");
            memset(&containerID, 0, sizeof(ms_extend_container_id));
            pContainerID = &containerID;
            memset((uint8_t*)pContainerID, 0x00, sizeof(ms_extend_container_id));
            pContainerID->dwLength = sizeof(ms_extend_container_id);
            pContainerID->bcdVersion = 0x0100;
            pContainerID->wIndex = 6;
            memcpy(pContainerID->containerID, containter_ID, sizeof(containter_ID));
            memcpy(buf, &containerID, sizeof(ms_extend_container_id));
            wLength = pContainerID->dwLength;
            break;

        default:
            break;
    }

    return wLength;
}

/*****************************************************************
Parameters    :  gadget
                 ctrl
Return        :
Description   :  厂商(huawei)自定义的ep0的命令
*****************************************************************/
USB_INT pnp_huawei_vendor_setup(struct usb_gadget* gadget, const struct usb_ctrlrequest* ctrl)
{
    struct usb_composite_dev*   cdev = get_gadget_data(gadget);
    struct usb_request*     req = NULL;
    usb_enum_state  enum_state   = USB_ENUM_NONE;
    USB_INT         value    = -EOPNOTSUPP;
    USB_UINT16      w_value  = 0;
    USB_UINT16      w_length = 0;

    /* partial re-init of the response message; the function or the
     * gadget might need to intercept e.g. a control-OUT completion
     * when we delegate to it.
     */
    if (NULL  == cdev || NULL  == ctrl || NULL  == gadget)
    {
        DBG_E(MBB_PNP, "cdev/%p OR ctrl/%p OR gadget/%p null, error!\n",cdev,ctrl,gadget);
        return -EOPNOTSUPP;
    }
    w_value  = le16_to_cpu(ctrl->wValue);
    w_length = le16_to_cpu(ctrl->wLength);
    req = cdev->req;
    req->zero = 0;
    req->length = 0;

    switch (ctrl->bRequest)
    {
        case USB_REQ_GET_DESCRIPTOR:
            if ( (USB_DT_STRING == (w_value >> 8))
                 && (MSOS_STRING_INDEX == (w_value & 0xFF)) )
            {
                DBG_T(MBB_PNP, "MSOS recive\n");
                w_length = le16_to_cpu(ctrl->wLength);
                value = (w_length < sizeof(MSOS_descriptor_string) ?
                         w_length : sizeof(MSOS_descriptor_string));
                memcpy(req->buf, MSOS_descriptor_string, value);
            }

            break;

        case MS_HUAWEI_VENDOR_CODE:
            if (USB_REQ_TYPE_MSOS_IN == ctrl->bRequestType )
            {
                DBG_T(MBB_PNP, "C0 DE recive\n");
                /*此处添加上报COMID*/
                enum_state = huawei_get_usb_enum_state();

                if ( USB_ENUM_DONE != enum_state || pnp_is_rndis_present())
                {
                    value = process_msos_request( ctrl, req->buf, w_value & USB_LOW_MASK);

                    if (value >= 0)
                    {
                        value = min(w_length, (USB_UINT16) value);
                    }
                }
            }

            break;

        case USB_REQ_GET_MSOS_CUSTOM: //windows service installed
            if (USB_REQ_TYPE_MSOS_IN == ctrl->bRequestType)
            {
                DBG_T(MBB_PNP, "C0 EE recive\n");
#ifndef USB_HILINK_MODEM_SWITCH
                if (pnp_port_style == g_pnp_info.current_port_style)
                {
                    g_pnp_info.rewind_param.bCmdReserved = USB_NUM_6;
                    g_pnp_info.rewind_param.bPcType      = WINDOWS_OS_FLAG;
                    g_pnp_info.rewind_param.bTimeOut     = 0x00;
                    g_pnp_info.rewind_param.bPID         = 0x00;
                    g_pnp_info.rewind_param.bNewPID      = 0x00;
                    g_pnp_info.rewind_param.bSupportCD   = 0x00;
                    g_pnp_info.rewind_param.bProFile     = 0x00;
                    g_pnp_info.rewind_param.bGreenMode   = 0x01;
                    g_pnp_info.is_service_switch         = MBB_USB_TRUE;
                }
#endif

                /*此处添加上报COMID*/
                enum_state = huawei_get_usb_enum_state();

                if ( USB_ENUM_DONE != enum_state || pnp_is_rndis_present())
                {
                    value = process_msos_request( ctrl, req->buf, w_value & USB_LOW_MASK);

                    if (value >= 0)
                    {
                        value = min(w_length, (USB_UINT16) value);
                    }
                }
            }
            break;

        case USB_REQ_GET_MSOS_CUSTOM_MAC://mac os service installed
            if ((USB_REQ_TYPE_MSOS_OUT == ctrl->bRequestType))
            {
                DBG_T(MBB_PNP, "40 A1 recive(MAC)\n");

                if (pnp_port_style == g_pnp_info.current_port_style)
                {
                    g_pnp_info.rewind_param.bCmdReserved = USB_NUM_6;
                    g_pnp_info.rewind_param.bPcType      = MAC_OS_FLAG;
                    g_pnp_info.rewind_param.bTimeOut     = 0x00;
                    g_pnp_info.rewind_param.bPID         = 0x00;
                    g_pnp_info.rewind_param.bNewPID      = 0x00;
                    g_pnp_info.rewind_param.bSupportCD   = 0x00;
                    g_pnp_info.rewind_param.bProFile     = 0x00;
                    g_pnp_info.rewind_param.bGreenMode   = 0x01;
                    pnp_switch_rewind_after_mode();
                }
                else
                {
                    DBG_T(MBB_PNP, "can not rewind\n");
                }
            }

            break;

        case USB_REQ_HUAWEI_EXT:
            if ((USB_REQ_TYPE_MSOS_IN == ctrl->bRequestType)
                && (pnp_port_style == g_pnp_info.current_port_style)
                && (0x0100 == (ctrl->wValue & USB_HIGH_MASK)))
            {
                USB_UCHAR os = ctrl->wValue & USB_LOW_MASK;
                DBG_T(MBB_PNP, "C0 9a recive\n");
                {
                    g_pnp_info.rewind_param.bCmdReserved = USB_NUM_6;
                    g_pnp_info.rewind_param.bPcType      = os;
                    g_pnp_info.rewind_param.bTimeOut     = 0x00;
                    g_pnp_info.rewind_param.bPID         = 0x00;
                    g_pnp_info.rewind_param.bNewPID      = ctrl->wIndex & USB_LOW_MASK;
                    g_pnp_info.rewind_param.bSupportCD   = 0x00;
                    g_pnp_info.rewind_param.bProFile     = 0x00;
#ifdef USB_RNDIS
                    g_pnp_info.rewind_param.bGreenMode   = 0x01;
#else
                    g_pnp_info.rewind_param.bGreenMode   = 0x00;
#endif
                    pnp_switch_rewind_after_mode();
                }
            }
            break;
            /*特殊切换命令，支持hilink、modem双模式的产品支持,和C0 EE互斥*/
        case USB_REQ_HILINK_MODEM_SWITCH:
            if (USB_REQ_TYPE_MSOS_IN == ctrl->bRequestType)
            {
                DBG_T(MBB_PNP, "C0 20 recive\n");
#ifdef USB_HILINK_MODEM_SWITCH
                if (pnp_port_style == g_pnp_info.current_port_style)
                {
                    g_pnp_info.rewind_param.bCmdReserved = USB_NUM_6;
                    g_pnp_info.rewind_param.bPcType      = WINDOWS_OS_FLAG;
                    g_pnp_info.rewind_param.bTimeOut     = 0x00;
                    g_pnp_info.rewind_param.bPID         = 0x00;
                    g_pnp_info.rewind_param.bNewPID      = ctrl->wIndex & USB_LOW_MASK;
                    g_pnp_info.rewind_param.bSupportCD   = 0x00;
                    g_pnp_info.rewind_param.bProFile     = 0x00;
                    g_pnp_info.rewind_param.bGreenMode   = 0x01;
                    g_pnp_info.is_service_switch         = MBB_USB_TRUE;
                }
#endif
                /*此处添加上报COMID*/
                enum_state = huawei_get_usb_enum_state();

                if ( USB_ENUM_DONE != enum_state || pnp_is_rndis_present())
                {
                    value = process_msos_request( ctrl, req->buf, w_value & USB_LOW_MASK);

                    if (value >= 0)
                    {
                        value = min(w_length, (USB_UINT16) value);
                    }
                }
            }
            break;

        default:
            break;
    }
    /* respond with data transfer before status phase? */
    if (value >= 0 && value != USB_GADGET_DELAYED_STATUS)
    {
        req->length = value;
        req->zero = value < w_length;
        value = usb_ep_queue(gadget->ep0, req, GFP_ATOMIC);

        if (value < 0)
        {
            DBG_I(MBB_PNP, "ep_queue --> %d\n", value);
            req->status = 0;
        }
    }

    /* device either (value < 0) or reports success */
    return value;
}


/*******************************************************************************
 *对外接口定义区
 *
 *接口定义必须进行接口说明
 ******************************************************************************/


void*   pnp_get_pnp_info(USB_VOID)
{
    return &g_pnp_info;
}


USB_INT  pnp_port_style_stat(USB_VOID)
{
    if (download_port_style == g_pnp_info.current_port_style
#if	(FEATURE_OFF == MBB_FACTORY)
        || factory_port_style == g_pnp_info.current_port_style
#endif        
	|| (GATEWAY_OS_FLAG == g_pnp_info.rewind_param.bPcType))
    {
        return MBB_USB_TRUE;
    }
    else
    {
        return MBB_USB_FALSE;
    }
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  查询是否静态端口形态的接口
*****************************************************************/
USB_INT  pnp_if_static_port_mode(USB_VOID)
{
    if(g_pnp_info.portModeIndex < CDROM_MODE 
        || g_pnp_info.portModeIndex >= INVALID_MODE)
    {
        DBG_I(MBB_PNP, "invalid mode\n");
        return  MBB_USB_ERROR;
    } 
    
    if ( g_pnp_info.portModeIndex < DYNAMIC_REWIND_BEFORE_MODE)
    {
        DBG_I(MBB_PNP, "static_port_mode\n");
        return MBB_USB_TRUE;
    }
    else
    {
        DBG_I(MBB_PNP, "dynamic_port_mode\n");
        return MBB_USB_FALSE;
    }
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  查询是否网关模式的接口
                 (该接口和pnp_system_type_get是否重复?)
*****************************************************************/
USB_INT  pnp_if_gateway_mode(USB_VOID)
{
    if ( GATEWAY_OS_FLAG == g_pnp_info.rewind_param.bPcType )
    {
        return MBB_USB_TRUE;
    }
    else
    {
        return MBB_USB_FALSE;
    }
}

/*****************************************************************
Parameters    :  USB_VOID
Return        : WINDOWS_OS_FLAG 0x00
            MAC_OS_FLAG     0x10
            LINUX_OS_FLAG   0x20
            GATEWAY_OS_FLAG 0x30
Description   :  查询当前连接的pc操作系统
*****************************************************************/
USB_INT pnp_system_type_get(USB_VOID)
{
    return g_pnp_info.rewind_param.bPcType;
}

/*****************************************************************
Parameters    :  pnp_style
Return        :  TRUE：动态，FALSE：静态
Description   : 判断端口形配置
*****************************************************************/
bool do_dynamic_port_config(USB_INT pnp_style)
{
    return  (pnp_style >= DYNAMIC_REWIND_BEFORE_MODE) ? MBB_USB_TRUE : MBB_USB_FALSE;
}

/*****************************************************************************
 函 数 名  :pnp_get_gateway_mode
 功能描述  :获取网关当前上报的端口模式
 返 回 值  :GATEWAY_NDIS_MODE 支持NDIS工作模式
            ；GATEWAT_MODEM_MODE:支持modem工作模式
*****************************************************************************/
/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  获取当前处于网关的何种模式
*****************************************************************/
USB_INT pnp_get_gateway_mode(USB_VOID)
{
    if ((GATEWAY_NDIS_MODE == g_pnp_info.rewind_param.bProFile)
        && (OS_TYPE_GATEWAY == g_pnp_info.rewind_param.bPcType))
    {
        return GATEWAY_NDIS_MODE;
    }
    return GATEWAT_MODEM_MODE;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  获取是否切换前，用以控制是否切换
*****************************************************************/
USB_INT pnp_is_rewind_before_mode(USB_VOID)
{
    USB_INT ret = MBB_USB_FALSE;
    if ( pnp_port_style == g_pnp_info.current_port_style )
    {
        ret = MBB_USB_TRUE;
    }

    return ret;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  获取是否切换前，用以控制是否mass mulit lun
*****************************************************************/
USB_INT pnp_is_multi_lun_mode(USB_VOID)
{
    USB_INT ret = MBB_USB_FALSE;
    if ( DYNAMIC_REWIND_BEFORE_MODE == g_pnp_info.portModeIndex )
    {
        ret = MBB_USB_TRUE;
    }
    return ret;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  当前是否是服务切换(hilink形态下的功能)
*****************************************************************/
USB_INT pnp_is_service_switch(USB_VOID)
{
    USB_INT ret = MBB_USB_FALSE;
    if ( MBB_USB_TRUE == g_pnp_info.is_service_switch )
    {
        ret = MBB_USB_TRUE;
    }

    return ret;
}

/*****************************************************************
Parameters    :  cmnd
Return        :
Description   :  mass设置给pnp切换命令的接口
*****************************************************************/
USB_VOID pnp_set_rewind_param(USB_UINT8* cmnd)
{
    memcpy(&(g_pnp_info.rewind_param), cmnd, MAX_REWIND_PARAM_NUM);
}

/*****************************************************************
Parameters    :  flag
Return        :
Description   :  设置app_start_mode标识，用以避免全局变量的使用
*****************************************************************/
USB_VOID pnp_set_ctl_app_flag(USB_INT flag)
{
    g_pnp_info.app_start_mode = flag;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  获取app_start_mode标识，用以避免全局变量的使用
*****************************************************************/
USB_INT pnp_get_ctl_app_flag(USB_VOID)
{
    return g_pnp_info.app_start_mode;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  判断光盘和sd卡是否有内容，可以打开的接口。
				关机充电等模式，光盘无内容
*****************************************************************/
USB_INT pnp_if_cdrom_can_open(USB_VOID)
{
    USB_INT ret = MBB_USB_FALSE;
    if ( rewind_port_style == g_pnp_info.current_port_style  
        || pnp_port_style == g_pnp_info.current_port_style  )
    {
        ret = MBB_USB_TRUE;
    }

    return ret;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  读取hilink产品，当前是hilink模式，还是modem模式
*****************************************************************/
USB_VOID pnp_read_hilink_modem_flag(USB_VOID)
{
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    USB_UINT8 mode_flag = HILINK_RNDIS_MODE;
    USB_UINT8 transf_mode = NV_HILINK_TYPE;
    if (NULL == usb_nv_info)
    {
        DBG_E(MBB_PNP, "usb_nv_info is NULL\n");
        return;
    }

    transf_mode = usb_nv_info->hilink_mode_flag.mode_flag;
    switch (transf_mode)
    {
        case NV_MODEM_DEBUG_TYPE:
            mode_flag = HILINK_MODEM_DEBUG_MODE;
            break;
        case NV_MODEM_TYPE:
            mode_flag = HILINK_MODEM_MODE;
            break;
        case NV_HILINK_TYPE:
            mode_flag = HILINK_RNDIS_MODE;
            break;
        default:
            DBG_I(MBB_PNP, "read hilink_modem_flag NV is a err value\n");
            mode_flag = HILINK_RNDIS_MODE;
            break;
    }
    DBG_I(MBB_PNP, "hilink_mode_flag %d\n", mode_flag);
    g_pnp_info.hilink_mode = mode_flag;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :    
Description   :  
*****************************************************************/

USB_INT pnp_unique_serial_num_flag(USB_VOID)
{
    USB_CHAR unique_serial_num_flag = 0;
#ifdef USB_UNIQUE_SERIAL_NUM
    USB_INT fd    = 0;
    mm_segment_t fs;

    fs = get_fs();
    set_fs(KERNEL_DS);

    fd = sys_open(UNIQUE_SERIAL_NUM_FLAG_PATH, O_RDONLY, 0);

    if (fd >= 0)
    {
        /*读取是否同步单板sn标志*/
        sys_read(fd, &unique_serial_num_flag, 1);
    }
    else
    {
        DBG_I(MBB_PNP, "no flag path\n");
    }

    sys_close(fd);
    set_fs(fs);
    /*标志转化为 0/1*/
    unique_serial_num_flag = !!unique_serial_num_flag;
#endif
    return unique_serial_num_flag;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :    
Description   :  
*****************************************************************/
USB_CHAR* pnp_get_serial_num(USB_VOID)
{
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();

    if (NULL != usb_nv_info)
    {
        /*从 NV中读取sn*/
        DBG_I(MBB_PNP, "aucSerialNumber %s\n", usb_nv_info->serial_num.aucSerialNumber);
        return usb_nv_info->serial_num.aucSerialNumber;
    }
    else
    {
        /*默认sn*/
        DBG_I(MBB_PNP, "aucSerialNumber %s\n", SERIAL_NUM_DEFAULT);
        return SERIAL_NUM_DEFAULT;
    }
}


/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  PNP注册外部回调。满足KO改造
*****************************************************************/
USB_VOID pnp_registe_cb(USB_VOID)
{
    usb_pnp_api_hanlder* pnp_api_handler = pnp_adp_get_api_handler();

    pnp_api_handler->pnp_if_cdrom_can_open_cb = pnp_if_cdrom_can_open;
    pnp_api_handler->pnp_set_rewind_param_cb = pnp_set_rewind_param;
    pnp_api_handler->pnp_switch_rewind_after_mode_cb = pnp_switch_rewind_after_mode;
    pnp_api_handler->pnp_is_service_switch_cb = pnp_is_service_switch;
    pnp_api_handler->pnp_is_rewind_before_mode_cb = pnp_is_rewind_before_mode;
    pnp_api_handler->pnp_get_gateway_mode_cb = pnp_get_gateway_mode;
    pnp_api_handler->pnp_system_type_get_cb = pnp_system_type_get;
    pnp_api_handler->pnp_if_static_port_mode_cb = pnp_if_static_port_mode;
    pnp_api_handler->pnp_port_style_stat_cb = pnp_port_style_stat;
    pnp_api_handler->pnp_get_pnp_info_cb = pnp_get_pnp_info;
    pnp_api_handler->pnp_probe_cb = pnp_probe;
    pnp_api_handler->pnp_remove_cb= pnp_remove;
    pnp_api_handler->pnp_remove_direct_cb = pnp_remove_direct;
    pnp_api_handler->pnp_switch_mbim_mode_cb = pnp_switch_mbim_mode;
    pnp_api_handler->pnp_switch_autorun_port_cb = pnp_switch_autorun_port;
    pnp_api_handler->android_disable_cb = android_disable;
    pnp_api_handler->android_create_device_cb = android_create_device;
    pnp_api_handler->android_destroy_device_cb = android_destroy_device;
    pnp_api_handler->pnp_is_multi_lun_mode_cb = pnp_is_multi_lun_mode;
    pnp_api_handler->pnp_set_ctl_app_flag_cb = pnp_set_ctl_app_flag;
    pnp_api_handler->pnp_get_ctl_app_flag_cb = pnp_get_ctl_app_flag;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  PNP外部回调去注册。满足KO改造
*****************************************************************/
USB_VOID pnp_unregiste_cb(USB_VOID)
{
    usb_pnp_api_hanlder* pnp_api_handler = pnp_adp_get_api_handler();

    memset(pnp_api_handler, 0, sizeof(usb_pnp_api_hanlder));
}

/*****************************************************************************
初始化和退出begin
*****************************************************************************/

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  PNP功能初始化。
*****************************************************************/
USB_VOID usb_pnp_init(USB_VOID)
{
    /*pnp_registe_cb 第一优先级初始化*/
    pnp_registe_cb();
    /*registe_usb_driver to "usb_composite_probe" 第二初始化*/
    android_registe_usb_driver(&pnp_usb_driver);
    /*usb_android_driver_init 第三初始化*/
    usb_android_driver_init();

    memset((USB_VOID*)&g_pnp_info, 0, sizeof(huawei_dynamic_info_st));
    usb_ctl_app_start_flag_init();
    INIT_DELAYED_WORK(&pnp_switch_work, pnp_switch_func);
    INIT_DELAYED_WORK(&pnp_notify_rewind_after_work, pnp_notify_rewind_after_func);
    INIT_DELAYED_WORK(&pnp_remove_work, pnp_remove_func);
    INIT_DELAYED_WORK(&setmode_work, setmode_work_func);
    usb_pnp_adp_init();
}

/*****************************************************************************
初始化和退出end
*****************************************************************************/

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  PNP可维可测函数，使用ecall 或者 usb_dfx直接应用态调用内核函数
*****************************************************************/
USB_VOID pnp_dump(USB_VOID)
{
    pnp_dynamic_mode* dynamic_port_mode = NULL;
    huawei_dynamic_info_st* pnp_info = pnp_get_pnp_info();
    USB_INT i = 0;
    /*output adp_dump*/
    pnp_adp_dump();
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    DBG_T(MBB_DEBUG, "|manufact_string---- = %s\n", manufacturer_string);
    DBG_T(MBB_DEBUG, "|product_string----- = %s\n", product_string);
    DBG_T(MBB_DEBUG, "|serial_string------ = %s\n", serial_string);
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    /*PNP端口信息*/
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    dynamic_port_mode = pnp_d_port_get_mode(DYNAMIC_REWIND_BEFORE_MODE);
    if (NULL != dynamic_port_mode)
    {
        DBG_T(MBB_DEBUG, "|rewind_before PID---------- = 0x%X\n", dynamic_port_mode->PID);
        DBG_T(MBB_DEBUG, "|rewind_before port_index--- = %d\n", dynamic_port_mode->port_index);

        for (i = 0; i < DYNAMIC_PID_MAX_PORT_NUM; i++)
        {
            if (dynamic_port_mode->mode_buf[i])
            {
                DBG_T(MBB_DEBUG, "|rewind_before_port----[%2d]- = %s \n",
                      i, dynamic_port_mode->mode_buf[i]);
                DBG_T(MBB_DEBUG, "|rewind_before_protocol[%2d]- = 0x%X \n",
                      i, dynamic_port_mode->port_protocol[i]);
            }
            else
            {
                break;
            }
        }
    }


    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");

    dynamic_port_mode = pnp_d_port_get_mode(DYNAMIC_REWIND_AFTER_MODE);

    if (NULL != dynamic_port_mode)
    {
        DBG_T(MBB_DEBUG, "|rewind_after PID---------- = 0x%X\n", dynamic_port_mode->PID);
        DBG_T(MBB_DEBUG, "|rewind_after port_index--- = %d\n", dynamic_port_mode->port_index);

        for (i = 0; i < DYNAMIC_PID_MAX_PORT_NUM; i++)
        {
            if (dynamic_port_mode->mode_buf[i])
            {
                DBG_T(MBB_DEBUG, "|rewind_after_port----[%2d]- = %s\n",
                      i, dynamic_port_mode->mode_buf[i]);
                DBG_T(MBB_DEBUG, "|rewind_after_protocol[%2d]- = 0x%X \n",
                      i, dynamic_port_mode->port_protocol[i]);
            }
            else
            {
                break;
            }
        }

    }

    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");

    /*PNP基本信息*/
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");

    if (NULL != pnp_info)
    {
        DBG_T(MBB_DEBUG, "|portModeIndex------ = %d\n", pnp_info->portModeIndex);
        DBG_T(MBB_DEBUG, "|rewind_sn_daynamic- = %d\n", pnp_info->is_daynamic_sn);
        DBG_T(MBB_DEBUG, "|current_port_style- = %d\n", pnp_info->current_port_style);
        DBG_T(MBB_DEBUG, "|is_service_switch-- = %d\n", pnp_info->is_service_switch);
        DBG_T(MBB_DEBUG, "|app_start_mode----- = %d\n", pnp_info->app_start_mode);
        DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
        DBG_T(MBB_DEBUG, "|bCmdReserved------- = %X\n", pnp_info->rewind_param.bCmdReserved);
        DBG_T(MBB_DEBUG, "|bPcType------------ = %X\n", pnp_info->rewind_param.bPcType);
        DBG_T(MBB_DEBUG, "|bTimeOut----------- = %X\n", pnp_info->rewind_param.bTimeOut);
        DBG_T(MBB_DEBUG, "|bPID--------------- = %X\n", pnp_info->rewind_param.bPID);
        DBG_T(MBB_DEBUG, "|bNewPID------------ = %X\n", pnp_info->rewind_param.bNewPID);
        DBG_T(MBB_DEBUG, "|bSupportCD--------- = %X\n", pnp_info->rewind_param.bSupportCD);
        DBG_T(MBB_DEBUG, "|bProFile----------- = %X\n", pnp_info->rewind_param.bProFile);
        DBG_T(MBB_DEBUG, "|bGreenMode--------- = %X\n", pnp_info->rewind_param.bGreenMode);
    }
}

