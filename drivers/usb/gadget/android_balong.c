/*
 * Gadget Driver for Android
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2016. Dongyue Chen  <foss@huawei.com>
 * 2013-02-13 - Fix coverity and fority in function:
 *		dwc3_reomve	
 * 	Dongyue Chen & Yong Jing <foss@huawei.com>
 *
 * 2013-11-06 - clean warning. Zhongshun Wang <foss@huawei.com>
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *         Benoit Goby <benoit@android.com>
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
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/platform_device.h>

#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <linux/usb/bsp_usb.h>

#include "gadget_chips.h"
#include "usb_vendor.h"

#ifndef CONFIG_USB_BALONG_UDC
#include "f_fs.c"
#include "f_audio_source.c"
#include "f_accessory.c"
#include "f_mtp.c"
#endif

#include "f_adb_balong.c"


#define USB_ETH_RNDIS y
#ifdef CONFIG_USB_FSG
#include "f_mass_storage_balong.c"
#endif
#ifdef CONFIG_BALONG_RNDIS
#include "f_rndis_balong.c"
#include "rndis_balong.c"
#endif
#ifdef CONFIG_BALONG_ECM
#include "f_ecm_balong.c"
#endif
#ifdef CONFIG_BALONG_NCM
#include "ncm_balong.c"
#include "f_ncm_balong.c"
#endif
#include "u_ether_balong.c"

#include "u_serial_balong.h"

MODULE_AUTHOR("Mike Lockwood");
MODULE_DESCRIPTION("Android Composite USB Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static const char longname[] = "Gadget Android";

/* Default vendor and product IDs, overridden by userspace */
#define VENDOR_ID		0x18D1
#define PRODUCT_ID		0x0001

struct android_usb_function {
	char *name;
	void *config;

	struct device *dev;
	char *dev_name;
	struct device_attribute **attributes;

	/* for android_dev.enabled_functions */
	struct list_head enabled_list;

	/* Optional: initialization during gadget bind */
	int (*init)(struct android_usb_function *, struct usb_composite_dev *);
	/* Optional: cleanup during gadget unbind */
	void (*cleanup)(struct android_usb_function *);
	/* Optional: called when the function is added the list of
	 *		enabled functions */
	void (*enable)(struct android_usb_function *);
	/* Optional: called when it is removed */
	void (*disable)(struct android_usb_function *);

	int (*bind_config)(struct android_usb_function *,
			   struct usb_configuration *);

	/* Optional: called when the configuration is removed */
	void (*unbind_config)(struct android_usb_function *,
			      struct usb_configuration *);
	/* Optional: handle ctrl requests before the device is configured */
	int (*ctrlrequest)(struct android_usb_function *,
					struct usb_composite_dev *,
					const struct usb_ctrlrequest *);
};

struct android_dev {
	struct android_usb_function **functions;
	struct list_head enabled_functions;
	struct usb_composite_dev *cdev;
	struct device *dev;

	bool enabled;
	int disable_depth;
	struct mutex mutex;
	bool connected;
	bool sw_connected;
	struct work_struct work;
	char ffs_aliases[256];
};

static struct class *android_class;
static struct android_dev *_android_dev;
static int android_bind_config(struct usb_configuration *c);
static void android_unbind_config(struct usb_configuration *c);

/* string IDs are assigned dynamically */
#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

static char manufacturer_string[256];
static char product_string[256];
static char serial_string[256];

/* String Table */
static struct usb_string strings_dev[] = {
	[STRING_MANUFACTURER_IDX].s = manufacturer_string,
	[STRING_PRODUCT_IDX].s = product_string,
	[STRING_SERIAL_IDX].s = serial_string,
	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

static struct usb_device_descriptor device_desc = {
	.bLength              = sizeof(device_desc),
	.bDescriptorType      = USB_DT_DEVICE,
	.bcdUSB               = __constant_cpu_to_le16(0x0300),
	.bDeviceClass         = USB_CLASS_PER_INTERFACE,
	.idVendor             = __constant_cpu_to_le16(VENDOR_ID),
	.idProduct            = __constant_cpu_to_le16(PRODUCT_ID),
	.bcdDevice            = __constant_cpu_to_le16(0xffff),
	.bNumConfigurations   = 1,
};

static struct usb_configuration android_config_driver = {
	.label		= "android",
	.unbind		= android_unbind_config,
	.bConfigurationValue = 1,
	.bmAttributes	= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	//.MaxPower	= 500, /* 500ma */
};

static void android_work(struct work_struct *data)
{
	struct android_dev *dev = container_of(data, struct android_dev, work);
	struct usb_composite_dev *cdev = dev->cdev;
	char *disconnected[2] = { "USB_STATE=DISCONNECTED", NULL };
	char *connected[2]    = { "USB_STATE=CONNECTED", NULL };
	char *configured[2]   = { "USB_STATE=CONFIGURED", NULL };
	char **uevent_envp = NULL;
	unsigned long flags;

	spin_lock_irqsave(&cdev->lock, flags);
	if (cdev->config)
		uevent_envp = configured;
	else if (dev->connected != dev->sw_connected)
		uevent_envp = dev->connected ? connected : disconnected;
	dev->sw_connected = dev->connected;
	spin_unlock_irqrestore(&cdev->lock, flags);

	if (uevent_envp) {
		kobject_uevent_env(&dev->dev->kobj, KOBJ_CHANGE, uevent_envp);
		pr_info("%s: sent uevent %s\n", __func__, uevent_envp[0]);
	} else {
		pr_info("%s: did not send uevent (%d %d %p)\n", __func__,
			 dev->connected, dev->sw_connected, cdev->config);
	}
}

static void android_enable(struct android_dev *dev)
{
	struct usb_composite_dev *cdev = dev->cdev;

	if (WARN_ON(!dev->disable_depth))
		return;

	if (--dev->disable_depth == 0) {
		usb_add_config(cdev, &android_config_driver,
					android_bind_config);
		cdev->gadget->functions_ready = 1;
		usb_gadget_connect(cdev->gadget);
	}
}

static void android_disable(struct android_dev *dev)
{
	struct usb_composite_dev *cdev = dev->cdev;

	if (dev->disable_depth++ == 0) {
		cdev->gadget->functions_ready = 0;
		usb_gadget_disconnect(cdev->gadget);
		/* Cancel pending control requests */
		usb_ep_dequeue(cdev->gadget->ep0, cdev->req);
		usb_remove_config(cdev, &android_config_driver);		
	}
}

#define MAX_SCRIPT_BUF_SIZE 256
#define USB_ANDROID0_ID_VENDOR "12d1"
#define USB_ANDROID0_ACM_INSTANCES "0x12,0x14,0x13,0x03,0x0A,0x01,0x06"
#define USB_ANDROID0_FUNCTIONS "gnet,acm,mass_storage,adb"
#define USB_ANDROID0_ENABLE "1"

enum script_type{
	android0_idVendor = 0,
	android0_idProduct,
	android0_bcdDevice,
	android0_bDeviceClass,
	android0_bDeviceSubClass,
	android0_bDeviceProtocol,
	android0_iManufacturer,
	android0_iProduct,
	android0_iSerial,
	gnet_ethaddr,
	gnet_vendorID,
	gnet_manufacturer,
	gnet_mode,
	gnet_wceis,
	acm_instances,
	mass_storage_inquiry,
	/*if new type needed to be added, add before this line*/
	android0_functions,
	android0_enable,
	usb_script_type_buttom
};

struct android_usb_script_backup{
	char *buf;
	char *per_buf;
	size_t size;
	size_t per_size;
};

struct android_usb_script_context{
	unsigned int usb_setup_status;
	struct mutex mutex;
	struct android_usb_script_backup script_arrary[usb_script_type_buttom];
};

struct android_usb_script_context usb_script_ctx = {0};

static void usb_script_backup(enum script_type dev_type, const char *buf, size_t size);
void usb_script_backup_finish(enum script_type dev_type, u32 result);


#ifndef CONFIG_USB_BALONG_UDC

/*-------------------------------------------------------------------------*/
/* Supported functions initialization */

struct functionfs_config {
	bool opened;
	bool enabled;
	struct ffs_data *data;
};

static int ffs_function_init(struct android_usb_function *f,
			     struct usb_composite_dev *cdev)
{
	f->config = kzalloc(sizeof(struct functionfs_config), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;

	return functionfs_init();
}

static void ffs_function_cleanup(struct android_usb_function *f)
{
	functionfs_cleanup();
	kfree(f->config);
}

static void ffs_function_enable(struct android_usb_function *f)
{
	struct android_dev *dev = _android_dev;
	struct functionfs_config *config = f->config;

	config->enabled = true;

	/* Disable the gadget until the function is ready */
	if (!config->opened)
		android_disable(dev);
}

static void ffs_function_disable(struct android_usb_function *f)
{
	struct android_dev *dev = _android_dev;
	struct functionfs_config *config = f->config;

	config->enabled = false;

	/* Balance the disable that was called in closed_callback */
	if (!config->opened)
		android_enable(dev);
}

static int ffs_function_bind_config(struct android_usb_function *f,
				    struct usb_configuration *c)
{
	struct functionfs_config *config = f->config;
	return functionfs_bind_config(c->cdev, c, config->data);
}

static ssize_t
ffs_aliases_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct android_dev *dev = _android_dev;
	int ret;

	mutex_lock(&dev->mutex);
	ret = snprintf(buf, MAX_SCRIPT_BUF_SIZE, "%s\n", dev->ffs_aliases);
	mutex_unlock(&dev->mutex);

	return ret;
}

static ssize_t
ffs_aliases_store(struct device *pdev, struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct android_dev *dev = _android_dev;
	char buff[256];

	mutex_lock(&dev->mutex);

	if (dev->enabled) {
		mutex_unlock(&dev->mutex);
		return -EBUSY;
	}

	strlcpy(buff, buf, sizeof(buff));
	strlcpy(dev->ffs_aliases, strim(buff), sizeof(dev->ffs_aliases));

	mutex_unlock(&dev->mutex);

	return size;
}

static DEVICE_ATTR(aliases, S_IRUGO | S_IWUSR, ffs_aliases_show,
					       ffs_aliases_store);
static struct device_attribute *ffs_function_attributes[] = {
	&dev_attr_aliases,
	NULL
};

static struct android_usb_function ffs_function = {
	.name		= "ffs",
	.init		= ffs_function_init,
	.enable		= ffs_function_enable,
	.disable	= ffs_function_disable,
	.cleanup	= ffs_function_cleanup,
	.bind_config	= ffs_function_bind_config,
	.attributes	= ffs_function_attributes,
};

static int functionfs_ready_callback(struct ffs_data *ffs)
{
	struct android_dev *dev = _android_dev;
	struct functionfs_config *config = ffs_function.config;
	int ret = 0;

	mutex_lock(&dev->mutex);

	ret = functionfs_bind(ffs, dev->cdev);
	if (ret)
		goto err;

	config->data = ffs;
	config->opened = true;

	if (config->enabled)
		android_enable(dev);

err:
	mutex_unlock(&dev->mutex);
	return ret;
}

static void functionfs_closed_callback(struct ffs_data *ffs)
{
	struct android_dev *dev = _android_dev;
	struct functionfs_config *config = ffs_function.config;

	mutex_lock(&dev->mutex);

	if (config->enabled)
		android_disable(dev);

	config->opened = false;
	config->data = NULL;

	functionfs_unbind(ffs);

	mutex_unlock(&dev->mutex);
}

static void *functionfs_acquire_dev_callback(const char *dev_name)
{
	return 0;
}

static void functionfs_release_dev_callback(struct ffs_data *ffs_data)
{
}
#endif
/*
static struct android_usb_function hotplug_function = {
	.name		= "hotplug",
};
*/
#define CONFIG_BALONG_ADB
#ifdef CONFIG_BALONG_ADB
struct adb_data {
	bool opened;
	bool enabled;
};

static int
adb_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	f->config = kzalloc(sizeof(struct adb_data), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;

	return adb_setup();
}

static void adb_function_cleanup(struct android_usb_function *f)
{
	adb_cleanup();
	kfree(f->config);
}

static int
adb_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	return adb_bind_config(c);
}
#if 0
static void adb_android_function_enable(struct android_usb_function *f)
{
	struct android_dev *dev = _android_dev;
	struct adb_data *data = f->config;

	data->enabled = true;

	/* Disable the gadget until adbd is ready */

	if (!data->opened)
		android_disable(dev);

}

static void adb_android_function_disable(struct android_usb_function *f)
{
	struct android_dev *dev = _android_dev;
	struct adb_data *data = f->config;

	data->enabled = false;

	/* Balance the disable that was called in closed_callback */
	if (!data->opened)
		android_enable(dev);
}
#endif
static struct android_usb_function adb_function = {
	.name		= "adb",
//	.enable		= adb_android_function_enable,
//	.disable	= adb_android_function_disable,
	.init		= adb_function_init,
	.cleanup	= adb_function_cleanup,
	.bind_config	= adb_function_bind_config,
};
#if 0
static void adb_ready_callback(void)
{
	struct android_dev *dev = _android_dev;
	struct adb_data *data = adb_function.config;

	mutex_lock(&dev->mutex);

	data->opened = true;

	if (data->enabled)
		android_enable(dev);

	mutex_unlock(&dev->mutex);
}

static void adb_closed_callback(void)
{
	struct android_dev *dev = _android_dev;
	struct adb_data *data = adb_function.config;

	mutex_lock(&dev->mutex);

	data->opened = false;

	if (data->enabled)
		android_disable(dev);

	mutex_unlock(&dev->mutex);
}
#endif
#endif //CONFIG_BALONG_ADB

#define MAX_ACM_INSTANCES 9

#if ACM_CDEV_USED_COUNT >= MAX_ACM_INSTANCES
    #error "ACM_CDEV_USED_COUNT and MAX_ACM_INSTANCES error"
#endif

#if ACM_TTY_USED_COUNT >= MAX_ACM_INSTANCES
    #error "ACM_TTY_USED_COUNT and MAX_ACM_INSTANCES error"
#endif

#if ACM_MDM_USED_COUNT >= MAX_ACM_INSTANCES
    #error "ACM_MDM_COUNT and MAX_ACM_INSTANCES error"
#endif

struct acm_function_config {
	int instances;
	int instances_on;
	int is_enable[MAX_ACM_INSTANCES];
	struct usb_function *f_acm[MAX_ACM_INSTANCES];
	struct usb_function_instance *f_acm_inst[MAX_ACM_INSTANCES];
};

struct acm_config_info {
    char* f_name;
    int count;
};

static struct acm_config_info g_acm_config_info[acm_class_max] = {
    {"acm_cdev",    ACM_CDEV_USED_COUNT},
    {"acm_tty",     ACM_TTY_USED_COUNT},
    {"acm_modem",   ACM_MDM_USED_COUNT}
};

extern struct acm_name_type_tbl g_acm_cdev_type_table[ACM_CDEV_COUNT];
extern struct acm_name_type_tbl g_acm_tty_type_table[ACM_TTY_COUNT];
extern struct acm_name_type_tbl g_acm_mdm_type_table[ACM_MDM_COUNT];

/* find name and type by prot_id in table_template */
int acm_find_type_info(char** get_name,
        enum acm_class_type* get_type, int*ret_idx, int prot_id)
{
    int i;
    int acm_type;
    struct acm_name_type_tbl* acm_type_table[(int)acm_class_max] = {
        g_acm_cdev_type_table,
        g_acm_tty_type_table,
        g_acm_mdm_type_table
    };

    for (acm_type = 0; acm_type < (int)acm_class_max; acm_type++) {
        for (i = 0; i < g_acm_config_info[acm_type].count; i++) {
            /* find type */
            if ((int)acm_type_table[acm_type][i].type == prot_id) {
                *get_name = acm_type_table[acm_type][i].name;
                *get_type = acm_type;
                *ret_idx = i;
                return 1;
            }
        }
    }
    return 0;
}

static void acm_function_cleanup(struct android_usb_function *f);
static void acm_function_unbind_config(struct android_usb_function *f,
				       struct usb_configuration *c);
static int
acm_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	int i;
	int acm_type;
	int ret;
	struct acm_function_config *config;

	config = kzalloc(sizeof(struct acm_function_config)*acm_class_max, GFP_KERNEL);
	if (!config)
		return -ENOMEM;
	f->config = config;

    for (acm_type = 0; acm_type < (int)acm_class_max; acm_type++) {

    	for (i = 0; i < g_acm_config_info[acm_type].count; i++) {
    		config->f_acm_inst[i] =
    		    usb_get_function_instance(g_acm_config_info[acm_type].f_name);
    		if (IS_ERR(config->f_acm_inst[i])) {
    			ret = PTR_ERR(config->f_acm_inst[i]);
    			goto err_usb_acm;
    		}
    		config->f_acm[i] = usb_get_function(config->f_acm_inst[i]);
    		if (IS_ERR(config->f_acm[i])) {
    			ret = PTR_ERR(config->f_acm[i]);
    			goto err_usb_acm;
    		}
    	}
    	/* goto next type instance */
    	config++;
	}
	return 0;

err_usb_acm:
	acm_function_cleanup(f);
#if 0
err_usb_get_function_instance:
	while (i-- > 0) {
		usb_put_function(config->f_acm[i]);
err_usb_get_function:
		usb_put_function_instance(config->f_acm_inst[i]);
	}
#endif
	return ret;
}

static void acm_function_cleanup(struct android_usb_function *f)
{
	int i;
	int acm_type;
	struct acm_function_config *config = f->config;

    for (acm_type = 0; acm_type < (int)acm_class_max; acm_type++) {
    	for (i = 0; i < g_acm_config_info[acm_type].count; i++) {
    	    if (config->f_acm[i]) {
    		    usb_put_function(config->f_acm[i]);
    		    config->f_acm[i] = NULL;
    		}
    		if (config->f_acm_inst[i]) {
    		    usb_put_function_instance(config->f_acm_inst[i]);
    		    config->f_acm_inst[i] = NULL;
    		}
    	}

    	/* goto next type instance */
    	config++;
	}
	kfree(f->config);
	f->config = NULL;
}

static int
acm_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	int i;
	int ret = 0;
	int acm_type;
	struct acm_function_config *config = f->config;

     for (acm_type = 0; acm_type < (int)acm_class_max; acm_type++) {

        for (i = 0; i < config->instances; i++) {
            if (config->is_enable[i]) {
            	ret = usb_add_function(c, config->f_acm[i]);
            	if (ret) {
            		pr_err("Could not bind acm%u config\n", i);
            		goto err_usb_add_function;
            	}
            	config->instances_on++;
        	}
        }

        /* goto next type instance */
    	config++;
	}

	return 0;

err_usb_add_function:
    acm_function_unbind_config(f, c);
#if 0
	while (i-- > 0)
		usb_remove_function(c, config->f_acm[i]);
#endif
	return ret;
}

static void acm_function_unbind_config(struct android_usb_function *f,
				       struct usb_configuration *c)
{
	int i;
	int acm_type;
	struct acm_function_config *config = f->config;

    for (acm_type = 0; acm_type < (int)acm_class_max; acm_type++) {
    	for (i = 0; i < config->instances_on; i++) {
    		//usb_remove_function(c, config->f_acm[i]);
    	}
    	config->instances_on = 0;
    	memset((void*)config->is_enable, 0, sizeof(int)*MAX_ACM_INSTANCES);

    	/* goto next type instance */
    	config++;
	}
}

static ssize_t acm_instances_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct acm_function_config *config = f->config;
	int acm_type;
	ssize_t ret = 0;

	for (acm_type = 0; acm_type < (int)acm_class_max; acm_type++) {
	    ret += snprintf(buf, MAX_SCRIPT_BUF_SIZE, "%d\n", config->instances);
	    /* goto next type instance */
    	config++;
	}
	return ret;
}

static ssize_t acm_instances_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct acm_function_config *config = f->config;
	struct acm_function_config *cur_config;
	char spe_dev[] = " ,;";
	char* cmd_str_find;
	char* cmd_str = (char *)buf;
	int cmd_len;
	int value[acm_class_max] = {0};
	int prot_id;
	int acm_type;
	int find_idx;
	enum acm_class_type class_type;
	char* dev_name;

	usb_script_backup(acm_instances, buf, size);

    cmd_len = strlen(cmd_str);
    while((int)(cmd_str - buf) < cmd_len) {

        cmd_str_find = strsep(&cmd_str, spe_dev);
        if (!cmd_str_find || kstrtou32(cmd_str_find, 0, &prot_id) < 0) {
            break;
        }

        /* if find the prot_id table, and mark it enable */
        if (acm_find_type_info(&dev_name, &class_type, &find_idx, prot_id)) {
            pr_err("acm parse cmd, prot_id is:0x%x, dev_name:%s, find_idx:%d, class_type:%s\n",
                prot_id, dev_name, find_idx, g_acm_config_info[class_type].f_name);
            cur_config = config + (int)class_type;
            cur_config->is_enable[find_idx] = 1;
            value[class_type]++;
        }
	}

    /* calc instance value in types */
	for (acm_type = 0; acm_type < (int)acm_class_max; acm_type++) {

	    if (value[acm_type] > g_acm_config_info[acm_type].count)
            value[acm_type] = g_acm_config_info[acm_type].count;

	    config->instances = value[acm_type];
    	/* goto next type instance */
    	config++;
	}
	
	usb_script_backup_finish(acm_instances, true);
	return size;
}

static DEVICE_ATTR(instances, S_IRUGO | S_IWUSR, acm_instances_show,
						 acm_instances_store);
static struct device_attribute *acm_function_attributes[] = {
	&dev_attr_instances,
	NULL
};

static struct android_usb_function acm_function = {
	.name		= "acm",
	.init		= acm_function_init,
	.cleanup	= acm_function_cleanup,
	.bind_config	= acm_function_bind_config,
	.unbind_config	= acm_function_unbind_config,
	.attributes	= acm_function_attributes,
};

#ifndef CONFIG_USB_BALONG_UDC

static int
mtp_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	return mtp_setup();
}

static void mtp_function_cleanup(struct android_usb_function *f)
{
	mtp_cleanup();
}

static int
mtp_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	return mtp_bind_config(c, false);
}

static int
ptp_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	/* nothing to do - initialization is handled by mtp_function_init */
	return 0;
}

static void ptp_function_cleanup(struct android_usb_function *f)
{
	/* nothing to do - cleanup is handled by mtp_function_cleanup */
}

static int
ptp_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	return mtp_bind_config(c, true);
}

static int mtp_function_ctrlrequest(struct android_usb_function *f,
					struct usb_composite_dev *cdev,
					const struct usb_ctrlrequest *c)
{
	return mtp_ctrlrequest(cdev, c);
}

static struct android_usb_function mtp_function = {
	.name		= "mtp",
	.init		= mtp_function_init,
	.cleanup	= mtp_function_cleanup,
	.bind_config	= mtp_function_bind_config,
	.ctrlrequest	= mtp_function_ctrlrequest,
};

/* PTP function is same as MTP with slightly different interface descriptor */
static struct android_usb_function ptp_function = {
	.name		= "ptp",
	.init		= ptp_function_init,
	.cleanup	= ptp_function_cleanup,
	.bind_config	= ptp_function_bind_config,
};
#endif

#define GNET_MODE_NCM               (0)
#define GNET_MODE_RNDIS             (1)
#define GNET_MODE_ECM               (2)
#define GNET_MODE_NCM_16            (3)
#define GNET_MODE_FORCE_NCM_16      (4)

struct gnet_function_config {
	u8      ethaddr[GNET_MAX_NUM][ETH_ALEN];
	u32     vendorID;
    u32     ports;
    u32     mode;
	char	manufacturer[256];

    /* "Wireless" RNDIS; auto-detected by Windows */
	bool	wceis;
	struct eth_dev *dev[GNET_MAX_NUM];
};

static int
gnet_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
    struct gnet_function_config *gnet;

	f->config = kzalloc(sizeof(struct gnet_function_config), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;

    gnet = (struct gnet_function_config *)f->config;
    gnet->ports = GNET_USED_NUM;
#if defined(CONFIG_BALONG_NCM)
    gnet->mode = GNET_MODE_NCM;
#elif defined(CONFIG_BALONG_RNDIS)
    gnet->mode = GNET_MODE_RNDIS;
#elif defined(CONFIG_BALONG_ECM)
    gnet->mode = GNET_MODE_ECM;
#endif

	return 0;
}

static void gnet_function_cleanup(struct android_usb_function *f)
{
	kfree(f->config);
	f->config = NULL;
}

int gnet_bind_config(struct android_usb_function *f, struct usb_configuration *c,
        u8 ethaddr[ETH_ALEN],struct eth_dev *dev,
        u32 mode, u32 vendorID, const char *manufacturer)
{
    int ret;

    switch(mode){
#ifdef CONFIG_BALONG_NCM        
        case GNET_MODE_NCM:
            ret = ncm_bind_config(c, ethaddr, dev, GNET_NTB32_FORMAT);
            break;
        case GNET_MODE_NCM_16:
            ret = ncm_bind_config(c, ethaddr, dev, GNET_NTB16_FORMAT);
            break;
        case GNET_MODE_FORCE_NCM_16:
            ret = ncm_bind_config(c, ethaddr, dev, GNET_NTB16_FORCE_FORMAT);
            break;
#endif
#ifdef CONFIG_BALONG_RNDIS
        case GNET_MODE_RNDIS:
            ret = rndis_bind_config_vendor(c, ethaddr, vendorID, manufacturer, dev);
            break;
#endif
#ifdef CONFIG_BALONG_ECM
        case GNET_MODE_ECM:
            ret = ecm_bind_config(c, ethaddr, dev);
            break;
#endif            
        default:
            ret = -1;
            break;
    }

    return ret;
}

static int
gnet_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	int ret;
	struct eth_dev *dev;
	struct gnet_function_config *gnet = f->config;
    unsigned int i;

	if (!gnet) {
		pr_err("%s: gnet_pdata\n", __func__);
		return -1;
	}

    for(i=0; i<gnet->ports; i++){
    	dev = gether_setup_name(c->cdev->gadget, gnet->ethaddr[i], "usb");
    	if (IS_ERR(dev)) {
    		ret = PTR_ERR(dev);
    		pr_err("%s: gether_setup_name port %d failed\n", __func__, i);
    		goto fail_clean_eth;
    	}
    	gnet->dev[i] = dev;

		ret = gnet_bind_config(f, c, gnet->ethaddr[i], dev,
            gnet->mode, gnet->vendorID, gnet->manufacturer);
		if(ret){
			pr_err("%s: gnet_bind_config port %d fail in %d mode!\n", __func__, i, gnet->mode);
			goto fail_clean_eth;
		}
	}

    return 0;

fail_clean_eth:
    for(i=0; i<gnet->ports; i++){
        if(gnet->dev[i]){
            gether_cleanup(gnet->dev[i]);
            gnet->dev[i] = NULL;
        }
    }

	return ret;
}

static void gnet_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
    unsigned int i;
	struct gnet_function_config *gnet = f->config;

    for(i=0; i<gnet->ports; i++){
        if(gnet->dev[i]){
            gether_cleanup(gnet->dev[i]);
            gnet->dev[i] = NULL;
        }
    }
}

static ssize_t gnet_manufacturer_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct gnet_function_config *config = f->config;
	return snprintf(buf, MAX_SCRIPT_BUF_SIZE, "%s\n", config->manufacturer);
}

static ssize_t gnet_manufacturer_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct gnet_function_config *config = f->config;

	usb_script_backup(gnet_manufacturer, buf, size);

	if (size >= sizeof(config->manufacturer)){
		usb_script_backup_finish(gnet_manufacturer, false);
		return -EINVAL;
	}
	if (sscanf(buf, "%s", config->manufacturer) == 1){
		usb_script_backup_finish(gnet_manufacturer, true);
		return size;
	}
	usb_script_backup_finish(gnet_manufacturer, false);
	return -1;
}

static DEVICE_ATTR(manufacturer, S_IRUGO | S_IWUSR, gnet_manufacturer_show,
						    gnet_manufacturer_store);

static ssize_t gnet_wceis_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct gnet_function_config *config = f->config;
	return snprintf(buf, MAX_SCRIPT_BUF_SIZE, "%d\n", config->wceis);
}

static ssize_t gnet_wceis_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct gnet_function_config *config = f->config;
	int value;
	
	usb_script_backup(gnet_wceis, buf, size);

	if (sscanf(buf, "%d", &value) == 1) {
		config->wceis = value;
		usb_script_backup_finish(gnet_wceis, true);
		return size;
	}
	usb_script_backup_finish(gnet_wceis, false);
	return -EINVAL;
}

static DEVICE_ATTR(wceis, S_IRUGO | S_IWUSR, gnet_wceis_show,
					     gnet_wceis_store);

static ssize_t gnet_ethaddr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct gnet_function_config *gnet = f->config;
	unsigned int i;
	
	for(i = 0; i < GNET_MAX_NUM; i++)
	{
		if(gnet->dev[i]){
			snprintf(buf, MAX_SCRIPT_BUF_SIZE, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
				gnet->ethaddr[i][0], gnet->ethaddr[i][1], gnet->ethaddr[i][2],
				gnet->ethaddr[i][3], gnet->ethaddr[i][4], gnet->ethaddr[i][5]);
		}
	}
	return sizeof(gnet->ethaddr);

}

static ssize_t gnet_ethaddr_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct gnet_function_config *gnet = f->config;
	unsigned int i;
	int ret;
	usb_script_backup(gnet_ethaddr, buf, size);

	for(i = 0; i < GNET_MAX_NUM; i++)
	{
		if(!gnet->dev[i])
			continue;
		printk("Please input mac address of gnet[%d]", i);
		ret = sscanf(buf, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
		    &gnet->ethaddr[i][0], &gnet->ethaddr[i][1], &gnet->ethaddr[i][2],
		    &gnet->ethaddr[i][3], &gnet->ethaddr[i][4], &gnet->ethaddr[i][5]);
		if (ETH_ALEN != ret){
			usb_script_backup_finish(gnet_ethaddr, false);
			return -EINVAL;
		}
	}
	usb_script_backup_finish(gnet_ethaddr, true);
	return size;

}

static DEVICE_ATTR(ethaddr, S_IRUGO | S_IWUSR, gnet_ethaddr_show,
					       gnet_ethaddr_store);

static ssize_t gnet_vendorID_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct gnet_function_config *config = f->config;
	return snprintf(buf, MAX_SCRIPT_BUF_SIZE, "%04x\n", config->vendorID);
}

static ssize_t gnet_vendorID_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct gnet_function_config *config = f->config;
	int value;

	usb_script_backup(gnet_vendorID, buf, size);

	if (sscanf(buf, "%04x", &value) == 1) {
		config->vendorID = value;
		usb_script_backup_finish(gnet_vendorID, true);
		return size;
	}
	usb_script_backup_finish(gnet_vendorID, false);
	return -EINVAL;
}

static DEVICE_ATTR(vendorID, S_IRUGO | S_IWUSR, gnet_vendorID_show,
						gnet_vendorID_store);

static ssize_t gnet_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct gnet_function_config *config = f->config;
	return snprintf(buf, MAX_SCRIPT_BUF_SIZE, "%d\n", config->mode);
}

static ssize_t gnet_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct gnet_function_config *config = f->config;
	int value;
	
	usb_script_backup(gnet_mode, buf, size);
	if (sscanf(buf, "%d", &value) == 1) {
		config->mode = value;
		usb_script_backup_finish(gnet_mode, true);
		return size;
	}
	usb_script_backup_finish(gnet_mode, false);
	return -EINVAL;
}

static DEVICE_ATTR(gnetmode, S_IRUGO | S_IWUSR, gnet_mode_show,
					     gnet_mode_store);

static struct device_attribute *gnet_function_attributes[] = {
	&dev_attr_manufacturer,
	&dev_attr_wceis,
	&dev_attr_ethaddr,
	&dev_attr_vendorID,
	&dev_attr_gnetmode,
	NULL
};

static struct android_usb_function gnet_function = {
	.name		= "gnet",
	.init		= gnet_function_init,
	.cleanup	= gnet_function_cleanup,
	.bind_config	= gnet_function_bind_config,
	.unbind_config	= gnet_function_unbind_config,
	.attributes	= gnet_function_attributes,
};

#ifdef CONFIG_USB_FSG

struct mass_storage_function_config {
	struct fsg_config fsg;
	struct fsg_common *common;
};

static int mass_storage_function_init(struct android_usb_function *f,
					struct usb_composite_dev *cdev)
{
	struct mass_storage_function_config *config;
	struct fsg_common *common;
	int err;

	config = kzalloc(sizeof(struct mass_storage_function_config),
								GFP_KERNEL);
	if (!config)
		return -ENOMEM;

	config->fsg.nluns = 2;
	config->fsg.luns[1].cdrom = 1;
	config->fsg.luns[0].removable = 1;/*for sdcard*/
	config->fsg.luns[1].removable = 1;
	config->fsg.can_stall = 1;
	common = fsg_common_init(NULL, cdev, &config->fsg);
	if (IS_ERR(common)) {
		kfree(config);
		return PTR_ERR(common);
	}

	err = sysfs_create_link(&f->dev->kobj,
				&common->luns[0].dev.kobj,
				"lun0");
	err |= sysfs_create_link(&f->dev->kobj,
				&common->luns[1].dev.kobj,
				"lun1");

	if (err) {
		kfree(config);
		return err;
	}

	config->common = common;
	f->config = config;
	return 0;
}

static void mass_storage_function_cleanup(struct android_usb_function *f)
{
	struct mass_storage_function_config *config = NULL;

	if (f->config) {
		config = f->config;
		if (config->common)
		    fsg_common_put(config->common);
	}

	kfree(f->config);
	f->config = NULL;
}

static int mass_storage_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct mass_storage_function_config *config = f->config;
	return fsg_bind_config(c->cdev, c, config->common);
}

static ssize_t mass_storage_inquiry_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct mass_storage_function_config *config = f->config;
	return snprintf(buf, MAX_SCRIPT_BUF_SIZE, "%s\n", config->common->inquiry_string);
}

static ssize_t mass_storage_inquiry_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct mass_storage_function_config *config = f->config;

	usb_script_backup(mass_storage_inquiry, buf, size);

	if (size >= sizeof(config->common->inquiry_string)){
		usb_script_backup_finish(mass_storage_inquiry, false);
		return -EINVAL;
	}
	if (sscanf(buf, "%s", config->common->inquiry_string) != 1){
		usb_script_backup_finish(mass_storage_inquiry, false);
		return -EINVAL;
	}
	usb_script_backup_finish(mass_storage_inquiry, true);
	return size;
}

static DEVICE_ATTR(inquiry_string, S_IRUGO | S_IWUSR,
					mass_storage_inquiry_show,
					mass_storage_inquiry_store);

static struct device_attribute *mass_storage_function_attributes[] = {
	&dev_attr_inquiry_string,
	NULL
};

static struct android_usb_function mass_storage_function = {
	.name		= "mass_storage",
	.init		= mass_storage_function_init,
	.cleanup	= mass_storage_function_cleanup,
	.bind_config	= mass_storage_function_bind_config,
	.attributes	= mass_storage_function_attributes,
};
#endif

#ifndef CONFIG_USB_BALONG_UDC

static int accessory_function_init(struct android_usb_function *f,
					struct usb_composite_dev *cdev)
{
	return acc_setup();
}

static void accessory_function_cleanup(struct android_usb_function *f)
{
	acc_cleanup();
}

static int accessory_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	return acc_bind_config(c);
}

static int accessory_function_ctrlrequest(struct android_usb_function *f,
						struct usb_composite_dev *cdev,
						const struct usb_ctrlrequest *c)
{
	return acc_ctrlrequest(cdev, c);
}

static struct android_usb_function accessory_function = {
	.name		= "accessory",
	.init		= accessory_function_init,
	.cleanup	= accessory_function_cleanup,
	.bind_config	= accessory_function_bind_config,
	.ctrlrequest	= accessory_function_ctrlrequest,
};

static int audio_source_function_init(struct android_usb_function *f,
			struct usb_composite_dev *cdev)
{
	struct audio_source_config *config;

	config = kzalloc(sizeof(struct audio_source_config), GFP_KERNEL);
	if (!config)
		return -ENOMEM;
	config->card = -1;
	config->device = -1;
	f->config = config;
	return 0;
}

static void audio_source_function_cleanup(struct android_usb_function *f)
{
	kfree(f->config);
}

static int audio_source_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct audio_source_config *config = f->config;

	return audio_source_bind_config(c, config);
}

static void audio_source_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct audio_source_config *config = f->config;

	config->card = -1;
	config->device = -1;
}

static ssize_t audio_source_pcm_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct audio_source_config *config = f->config;

	/* print PCM card and device numbers */
	return snprintf(buf, MAX_SCRIPT_BUF_SIZE, "%d %d\n", config->card, config->device);
}

static DEVICE_ATTR(pcm, S_IRUGO, audio_source_pcm_show, NULL);

static struct device_attribute *audio_source_function_attributes[] = {
	&dev_attr_pcm,
	NULL
};

static struct android_usb_function audio_source_function = {
	.name		= "audio_source",
	.init		= audio_source_function_init,
	.cleanup	= audio_source_function_cleanup,
	.bind_config	= audio_source_function_bind_config,
	.unbind_config	= audio_source_function_unbind_config,
	.attributes	= audio_source_function_attributes,
};
#endif

static struct android_usb_function *supported_functions[] = {
    &gnet_function,
	&acm_function,
#ifdef CONFIG_USB_FSG
	&mass_storage_function,	
#endif
	&adb_function,
//	&hotplug_function,
	NULL
};


static int android_init_functions(struct android_usb_function **functions,
				  struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;
	struct android_usb_function *f;
	struct device_attribute **attrs;
	struct device_attribute *attr;
	int err = 0;
	int index = 1;

	for (; (f = *functions++); index++) {
		f->dev_name = kasprintf(GFP_KERNEL, "f_%s", f->name);
		f->dev = device_create(android_class, dev->dev,
				MKDEV(0, index), f, f->dev_name);
		if (IS_ERR(f->dev)) {
			pr_err("%s: Failed to create dev %s", __func__,
							f->dev_name);
			err = PTR_ERR(f->dev);
			goto err_create;
		}

		if (f->init) {
			err = f->init(f, cdev);
			if (err) {
				pr_err("%s: Failed to init %s", __func__,
								f->name);
				goto err_out;
			}
		}

		attrs = f->attributes;
		if (attrs) {
			while ((attr = *attrs++) && !err)
				err = device_create_file(f->dev, attr);
		}
		if (err) {
			pr_err("%s: Failed to create function %s attributes",
					__func__, f->name);
			goto err_out;
		}
	}
	return 0;

err_out:
	device_destroy(android_class, f->dev->devt);
err_create:
	kfree(f->dev_name);
	return err;
}

static void android_cleanup_functions(struct android_usb_function **functions)
{
	struct android_usb_function *f;

	while (*functions) {
		f = *functions++;

		if (f->dev) {
			device_destroy(android_class, f->dev->devt);
			kfree(f->dev_name);
		}

		if (f->cleanup)
			f->cleanup(f);
	}
}

static int
android_bind_enabled_functions(struct android_dev *dev,
			       struct usb_configuration *c)
{
	struct android_usb_function *f;
	int ret;

	list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
		ret = f->bind_config(f, c);
		if (ret) {
			pr_err("%s: %s failed", __func__, f->name);
			return ret;
		}
	}
	return 0;
}

static void
android_unbind_enabled_functions(struct android_dev *dev,
			       struct usb_configuration *c)
{
	struct android_usb_function *f;

	list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
		if (f->unbind_config)
			f->unbind_config(f, c);
	}
}

static int android_enable_function(struct android_dev *dev, char *name)
{
	struct android_usb_function **functions = dev->functions;
	struct android_usb_function *f;

	while ((f = *functions++)) {
		if (!strcmp(name, f->name)) {
			list_add_tail(&f->enabled_list,
						&dev->enabled_functions);
			return 0;
		}
	}
	return -EINVAL;
}

/*-------------------------------------------------------------------------*/
/* /sys/class/android_usb/android%d/ interface */

static ssize_t
functions_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct android_usb_function *f;
	char *buff = buf;

	mutex_lock(&dev->mutex);

	list_for_each_entry(f, &dev->enabled_functions, enabled_list)
		buff += snprintf(buff, MAX_SCRIPT_BUF_SIZE, "%s,", f->name);

	mutex_unlock(&dev->mutex);

	if (buff != buf)
		*(buff-1) = '\n';
	return buff - buf;
}

static ssize_t
functions_store(struct device *pdev, struct device_attribute *attr,
			       const char *buff, size_t size)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	char *name;
	char buf[256], *b;
	char aliases[256], *a;
	int err;
	int is_ffs;
	int ffs_enabled = 0;

	mutex_lock(&dev->mutex);
	usb_script_backup(android0_functions, buff, size);

	if (dev->enabled) {
		usb_script_backup_finish(android0_functions, false);
		mutex_unlock(&dev->mutex);
		return -EBUSY;
	}

	INIT_LIST_HEAD(&dev->enabled_functions);

	strlcpy(buf, buff, sizeof(buf));
	b = strim(buf);
	while (b) {
		name = strsep(&b, ",");
		if (!name)
			continue;
		
		is_ffs = 0;
		strlcpy(aliases, dev->ffs_aliases, sizeof(aliases));

		a = aliases;

		while (a) {
			char *alias = strsep(&a, ",");
			if (alias && !strcmp(name, alias)) {
				is_ffs = 1;
				break;
			}
		}

		if (is_ffs) {
			if (ffs_enabled)
				continue;
			err = android_enable_function(dev, "ffs");
			if (err)
				pr_err("android_usb: Cannot enable ffs (%d) \n",
									err);
			else
				ffs_enabled = 1;
			continue;
		}

		err = android_enable_function(dev, name);
		if (err)
			pr_err("android_usb: Cannot enable '%s' (%d)  \n",
							   name, err);
	}
	usb_script_backup_finish(android0_functions, true);
	mutex_unlock(&dev->mutex);

	return size;
}

static ssize_t enable_show(struct device *pdev, struct device_attribute *attr,
			   char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	return snprintf(buf, MAX_SCRIPT_BUF_SIZE, "%d\n", dev->enabled);
}

static ssize_t enable_store(struct device *pdev, struct device_attribute *attr,
			    const char *buff, size_t size)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct usb_composite_dev *cdev = dev->cdev;
	struct android_usb_function *f;
	int enabled = 0;
	
	usb_script_backup(android0_enable, buff, size);

	if (!cdev){
		usb_script_backup_finish(android0_enable, false);
		return -ENODEV;
	}
	mutex_lock(&dev->mutex);
	sscanf(buff, "%d", &enabled);
	if (enabled && !dev->enabled) {
		/*
		 * Update values in composite driver's copy of
		 * device descriptor.
		 */
		cdev->desc.idVendor = device_desc.idVendor;
		cdev->desc.idProduct = device_desc.idProduct;
		cdev->desc.bcdDevice = device_desc.bcdDevice;
		cdev->desc.bDeviceClass = device_desc.bDeviceClass;
		cdev->desc.bDeviceSubClass = device_desc.bDeviceSubClass;
		cdev->desc.bDeviceProtocol = device_desc.bDeviceProtocol;
		list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
			if (f->enable)
				f->enable(f);
		}
		android_enable(dev);
		dev->enabled = true;
	} else if (!enabled && dev->enabled) {
		android_disable(dev);
		list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
			if (f->disable)
				f->disable(f);
		}
		dev->enabled = false;
	} else {
		pr_err("android_usb: already %s\n",
				dev->enabled ? "enabled" : "disabled");
	}

	usb_script_backup_finish(android0_enable, true);
	mutex_unlock(&dev->mutex);
	return size;
}

static ssize_t state_show(struct device *pdev, struct device_attribute *attr,
			   char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct usb_composite_dev *cdev = dev->cdev;
	char *state = "DISCONNECTED";
	unsigned long flags;

	if (!cdev)
		goto out;

	spin_lock_irqsave(&cdev->lock, flags);
	if (cdev->config)
		state = "CONFIGURED";
	else if (dev->connected)
		state = "CONNECTED";
	spin_unlock_irqrestore(&cdev->lock, flags);
out:
	return snprintf(buf, MAX_SCRIPT_BUF_SIZE, "%s\n", state);
}

#define DESCRIPTOR_ATTR(field, format_string)				\
static ssize_t								\
field ## _show(struct device *dev, struct device_attribute *attr,	\
		char *buf)						\
{									\
	return snprintf(buf, MAX_SCRIPT_BUF_SIZE, format_string, device_desc.field);		\
}									\
static ssize_t								\
field ## _store(struct device *dev, struct device_attribute *attr,	\
		const char *buf, size_t size)				\
{									\
	int value;							\
	usb_script_backup(android0_##field , buf, size);	\
	if (sscanf(buf, format_string, &value) == 1) {			\
		device_desc.field = value;				\
		usb_script_backup_finish(android0_##field , true); \
		return size;						\
	}								\
	usb_script_backup_finish(android0_##field , false); \
	return -1;							\
}									\
static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);

#define DESCRIPTOR_STRING_ATTR(field, buffer)				\
static ssize_t								\
field ## _show(struct device *dev, struct device_attribute *attr,	\
		char *buf)						\
{									\
	return snprintf(buf, MAX_SCRIPT_BUF_SIZE, "%s", buffer);				\
}									\
static ssize_t								\
field ## _store(struct device *dev, struct device_attribute *attr,	\
		const char *buf, size_t size)				\
{									\
	usb_script_backup(android0_##field , buf, size);	\
	if (size >= sizeof(buffer))	{				\
		usb_script_backup_finish(android0_##field , false); \
		return -EINVAL;						\
	}	\
	usb_script_backup_finish(android0_ ## field , true); \
	return strlcpy(buffer, buf, sizeof(buffer));		\
}									\
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

static DEVICE_ATTR(functions, S_IRUGO | S_IWUSR, functions_show,
						 functions_store);
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, enable_show, enable_store);
static DEVICE_ATTR(state, S_IRUGO, state_show, NULL);

static struct device_attribute *android_usb_attributes[] = {
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
	&dev_attr_state,
	NULL
};


void usb_script_init(void)
{
	unsigned int i;
	struct android_usb_script_context * ctx = &usb_script_ctx;

	ctx->usb_setup_status = 1;
	mutex_init(&ctx->mutex);
	for(i = 0; i < usb_script_type_buttom; i++){
		ctx->script_arrary[i].buf = NULL;
		ctx->script_arrary[i].size = 0;
	}
	/*Add the following code because init.usb.rc can not be called properly.
		So move the operation here and remove it from init.rc .
	*/
	usb_script_backup(android0_idVendor, USB_ANDROID0_ID_VENDOR, sizeof(USB_ANDROID0_ID_VENDOR));
	usb_script_backup_finish(gnet_vendorID, true);
	
	usb_script_backup(acm_instances, USB_ANDROID0_ACM_INSTANCES, sizeof(USB_ANDROID0_ACM_INSTANCES));
	usb_script_backup_finish(acm_instances, true);
	
	usb_script_backup(android0_functions, USB_ANDROID0_FUNCTIONS, sizeof(USB_ANDROID0_FUNCTIONS));
	usb_script_backup_finish(android0_functions, true);
	
	usb_script_backup(android0_enable, USB_ANDROID0_ENABLE, sizeof(USB_ANDROID0_ENABLE));
	usb_script_backup_finish(android0_enable, true);
}

static void usb_script_backup(enum script_type dev_type, const char *buf, size_t size)
{
	char * str_buf;
	struct android_usb_script_context * ctx = &usb_script_ctx;
	if(size >= (MAX_SCRIPT_BUF_SIZE-1)){
		pr_err("usb_script_backup fail, input string too large. \n ");
		return;
	}

	str_buf = kzalloc(size+1, GFP_KERNEL);
	if(!str_buf){
		pr_err("usb_script_backup fail, malloc mem fail. \n ");
		return;
	}
	strlcpy(str_buf, buf, size+1);
	ctx->script_arrary[dev_type].per_buf = ctx->script_arrary[dev_type].buf;
	ctx->script_arrary[dev_type].buf = str_buf;
	ctx->script_arrary[dev_type].per_size = ctx->script_arrary[dev_type].size;
	ctx->script_arrary[dev_type].size = size;
}

void usb_script_backup_finish(enum script_type dev_type, u32 result)
{
	struct android_usb_script_context * ctx = &usb_script_ctx;

	if(!result){
		/*fail*/
		kfree(ctx->script_arrary[dev_type].buf);
		ctx->script_arrary[dev_type].buf = ctx->script_arrary[dev_type].per_buf;
		ctx->script_arrary[dev_type].size = ctx->script_arrary[dev_type].per_size;
	}else{
		/*success*/
		kfree(ctx->script_arrary[dev_type].per_buf);
		ctx->script_arrary[dev_type].per_buf = NULL;
		ctx->script_arrary[dev_type].per_size = 0;
	}
}


struct android_usb_function* usb_script_get_func(char * name,
	struct android_usb_function **functions)
{
	struct android_usb_function **funcs = functions;
	struct android_usb_function *f = *functions;

	while(NULL != f){
		if(strncmp(f->dev_name, name, strnlen(name, 256))){
			//unmatch
			funcs++;
			f = *funcs;
		}else{
			//match
			return f;
		}
	}
	return NULL;

}

void usb_script_set(enum script_type type, char * buff, size_t size)
{
	struct android_dev *dev = _android_dev;
	struct android_usb_function **func = dev->functions;
	struct android_usb_function *cur_func = NULL;
	const char * str_buff = buff;
	
	switch(type){
	case android0_idVendor:
		idVendor_store(dev->dev, &dev_attr_idVendor, str_buff, size);
		break;
	case android0_idProduct:
		idProduct_store(dev->dev, &dev_attr_idProduct, str_buff, size);
		break;
	case android0_bcdDevice:
		bcdDevice_store(dev->dev, &dev_attr_bcdDevice, str_buff, size);
		break;
	case android0_bDeviceClass:
		bDeviceClass_store(dev->dev, &dev_attr_bDeviceClass, str_buff, size);
		break;
	case android0_bDeviceSubClass:
		bDeviceSubClass_store(dev->dev, &dev_attr_bDeviceSubClass, str_buff, size);
		break;
	case android0_bDeviceProtocol:
		bDeviceProtocol_store(dev->dev, &dev_attr_bDeviceProtocol, str_buff, size);
		break;
	case android0_iManufacturer:
		iManufacturer_store(dev->dev, &dev_attr_iManufacturer, str_buff, size);
		break;
	case android0_iProduct:
		iProduct_store(dev->dev, &dev_attr_iProduct, str_buff, size);
		break;
	case android0_iSerial:
		iSerial_store(dev->dev, &dev_attr_iSerial, str_buff, size);
		break;
	case android0_enable:
		enable_store(dev->dev, &dev_attr_enable, str_buff, size);
		break;
	case android0_functions:
		functions_store(dev->dev, &dev_attr_functions, str_buff, size);
		break;
	case acm_instances:
		cur_func = usb_script_get_func("f_acm", func);
		if(cur_func){
			acm_instances_store(cur_func->dev,&dev_attr_instances, str_buff, size);
		}else{
			pr_err("Restore fail! acm function device is NULL! \n");
		}
		break;
#ifdef CONFIG_USB_FSG
	case mass_storage_inquiry:
		cur_func = usb_script_get_func("f_mass_storage", func);
		if(cur_func){
			mass_storage_inquiry_store(cur_func->dev,&dev_attr_inquiry_string, str_buff, size);
		}else{
			pr_err("Restore fail! mass_storage function device is NULL! \n");
		}
		break;
#endif
	case gnet_ethaddr:
		cur_func = usb_script_get_func("f_gnet", func);
		if(cur_func){
			gnet_ethaddr_store(cur_func->dev,&dev_attr_ethaddr, str_buff, size);
		}else{
			pr_err("Restore fail! gnet_ethaddr function device is NULL! \n");
		}
		break;
	case gnet_vendorID:
		cur_func = usb_script_get_func("f_gnet", func);
		if(cur_func){
			gnet_vendorID_store(cur_func->dev,&dev_attr_vendorID, str_buff, size);
		}else{
			pr_err("Restore fail! gnet_vendorID function device is NULL! \n");
		}
		break;
	case gnet_manufacturer:
		cur_func = usb_script_get_func("f_gnet", func);
		if(cur_func){
			gnet_manufacturer_store(cur_func->dev,&dev_attr_manufacturer, str_buff, size);
		}else{
			pr_err("Restore fail! gnet_manufacturer function device is NULL! \n");
		}
		break;
	case gnet_mode:
		cur_func = usb_script_get_func("f_gnet", func);
		if(cur_func){
			gnet_mode_store(cur_func->dev, NULL, str_buff, size);
		}else{
			pr_err("Restore fail! gnet_mode function device is NULL! \n");
		}
		break;
	case gnet_wceis:
		cur_func = usb_script_get_func("f_gnet", func);
		if(cur_func){
			gnet_ethaddr_store(cur_func->dev,&dev_attr_wceis, str_buff, size);
		}else{
			pr_err("Restore fail! gnet_wceis function device is NULL! \n");
		}
		break;
	default:
		pr_err("false script type in usb_script_set \n");
		
	}
	
}

void usb_script_restore(void)
{
	char * scr_buf;
	size_t scr_size;
	char buffer[MAX_SCRIPT_BUF_SIZE];
	unsigned int i;
	struct android_usb_script_context * ctx = &usb_script_ctx;

	for(i = 0; i < usb_script_type_buttom; i++){
		scr_buf = ctx->script_arrary[i].buf;
		scr_size = ctx->script_arrary[i].size;
		if(scr_buf){
			strlcpy(buffer, scr_buf, scr_size+1);
			usb_script_set(i, buffer, scr_size);
		}
	}
}





/*-------------------------------------------------------------------------*/
/* Composite driver */

static int android_bind_config(struct usb_configuration *c)
{
	struct android_dev *dev = _android_dev;
	int ret = 0;

	ret = android_bind_enabled_functions(dev, c);
	if (ret)
		return ret;

	return 0;
}

static void android_unbind_config(struct usb_configuration *c)
{
	struct android_dev *dev = _android_dev;

	android_unbind_enabled_functions(dev, c);
}

static int android_bind(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;
	struct usb_gadget	*gadget = cdev->gadget;
	int			id, ret;

	/*
	 * Start disconnected. Userspace will connect the gadget once
	 * it is done configuring the functions.
	 */
	usb_gadget_disconnect(gadget);

	ret = android_init_functions(dev->functions, cdev);
	if (ret)
		return ret;

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */
	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_MANUFACTURER_IDX].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_PRODUCT_IDX].id = id;
	device_desc.iProduct = id;

	/* Default strings - should be updated by userspace */
	strncpy(manufacturer_string, "Android", sizeof(manufacturer_string)-1);
	strncpy(product_string, "Android", sizeof(product_string) - 1);
	strncpy(serial_string, "0123456789ABCDEF", sizeof(serial_string) - 1);

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_SERIAL_IDX].id = id;
	device_desc.iSerialNumber = id;

	usb_gadget_set_selfpowered(gadget);
	dev->cdev = cdev;

	return 0;
}

static int android_usb_unbind(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;

	cancel_work_sync(&dev->work);
	android_cleanup_functions(dev->functions);
	return 0;
}

/* HACK: android needs to override setup for accessory to work */
static int (*composite_setup_func)(struct usb_gadget *gadget, const struct usb_ctrlrequest *c);

static int
android_setup(struct usb_gadget *gadget, const struct usb_ctrlrequest *c)
{
	struct android_dev		*dev = _android_dev;
	struct usb_composite_dev	*cdev = NULL;
	struct usb_request		*req = NULL;
	struct android_usb_function	*f;
	int value = -EOPNOTSUPP;
	unsigned long flags;
	
	cdev = get_gadget_data(gadget);
	if(NULL == cdev){
		printk("android_setup fail, no cdev \n");
		return -ENODEV;
	}
	
	req = cdev->req;
	if(NULL == req){
		printk("android_setup fail, no req \n");
		return -ENOMEM;
	}

	req->zero = 0;
	req->length = 0;
	gadget->ep0->driver_data = cdev;

	list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
		if (f->ctrlrequest) {
			value = f->ctrlrequest(f, cdev, c);
			if (value >= 0)
				break;
		}
	}

	/* Special case the accessory function.
	 * It needs to handle control requests before it is enabled.
	 */

#ifndef CONFIG_USB_BALONG_UDC
	if (value < 0)
		value = acc_ctrlrequest(cdev, c);
#endif

	if (value < 0)
		value = composite_setup_func(gadget, c);

	spin_lock_irqsave(&cdev->lock, flags);
	if (!dev->connected) {
		dev->connected = 1;
		schedule_work(&dev->work);
	} else if (c->bRequest == USB_REQ_SET_CONFIGURATION &&
						cdev->config) {
		schedule_work(&dev->work);
	}
	spin_unlock_irqrestore(&cdev->lock, flags);

	return value;
}

static void android_disconnect(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;

	/* accessory HID support can be active while the
	   accessory function is not actually enabled,
	   so we need to inform it when we are disconnected.
	 */
	//acc_disconnect();

	dev->connected = 0;
	schedule_work(&dev->work);
}

static struct usb_composite_driver android_usb_driver = {
	.name		= "android_usb",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.bind		= android_bind,
	.unbind		= android_usb_unbind,
	.disconnect	= android_disconnect,
	.max_speed	= USB_SPEED_SUPER,
};

static int android_create_device(struct android_dev *dev)
{
	struct device_attribute **attrs = android_usb_attributes;
	struct device_attribute *attr;
	int err;

	dev->dev = device_create(android_class, NULL,
					MKDEV(0, 0), NULL, "android0");
	if (IS_ERR(dev->dev))
		return PTR_ERR(dev->dev);

	dev_set_drvdata(dev->dev, dev);

	while ((attr = *attrs++)) {
		err = device_create_file(dev->dev, attr);
		if (err) {
			device_destroy(android_class, dev->dev->devt);
			return err;
		}
	}
	return 0;
}


static int init(void)
{
	struct android_dev *dev;
	int err;
	struct android_usb_script_context * ctx = &usb_script_ctx;

	android_class = class_create(THIS_MODULE, "android_usb");
	if (IS_ERR(android_class))
		return PTR_ERR(android_class);

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		err = -ENOMEM;
		goto err_dev;
	}

#ifdef CONFIG_GADGET_SUPPORT_REMOTE_WAKEUP
	if(USB_TBOX_DETECT != bsp_usb_vbus_detect_mode()){
		android_config_driver.bmAttributes |= USB_CONFIG_ATT_WAKEUP;
	}
#endif

	dev->disable_depth = 1;
	dev->functions = supported_functions;
	INIT_LIST_HEAD(&dev->enabled_functions);
	INIT_WORK(&dev->work, android_work);
	mutex_init(&dev->mutex);

	err = android_create_device(dev);
	if (err) {
		pr_err("%s: failed to create android device %d", __func__, err);
		goto err_create;
	}

	_android_dev = dev;

	err = usb_composite_probe(&android_usb_driver);
	if (err) {
		pr_err("%s: failed to probe driver %d", __func__, err);
		goto err_probe;
	}

	/* HACK: exchange composite's setup with ours */
	composite_setup_func = android_usb_driver.gadget_driver.setup;
	android_usb_driver.gadget_driver.setup = android_setup;

    bsp_usb_init_enum_stat();

    /*get the driver product id form nv*/
    device_desc.idProduct = __constant_cpu_to_le16(bsp_usb_pc_driver_id_get());

	if(unlikely(!ctx->usb_setup_status)){
		usb_script_init();
	}
	usb_script_restore();
	return 0;

err_probe:
	device_destroy(android_class, dev->dev->devt);
err_create:
	kfree(dev);
err_dev:
	class_destroy(android_class);
	return err;
}
//late_initcall(init);

int android_gadget_init(void)
{
    return init();
}
EXPORT_SYMBOL(android_gadget_init);

static void cleanup(void)
{
	usb_composite_unregister(&android_usb_driver);
	device_destroy(android_class, _android_dev->dev->devt);
	class_destroy(android_class);
	kfree(_android_dev);
	_android_dev = NULL;
}
//module_exit(cleanup);

void android_gadget_cleanup(void)
{
    return cleanup();
}
EXPORT_SYMBOL(android_gadget_cleanup);
