
#include "android.h"
#include "hw_pnp_api.h"
#include <linux/usb/composite.h>

static int adb_setup(void);
static void adb_cleanup(void);
static int adb_bind_config(struct usb_configuration* c);

static int
adb_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	return adb_setup();
}

static void adb_function_cleanup(struct android_usb_function *f)
{
	adb_cleanup();
}

static int
adb_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	return adb_bind_config(c);
}

static struct android_usb_function adb_function = {
	.name		= "adb",
	.init		= adb_function_init,
	.cleanup	= adb_function_cleanup,
	.bind_config	= adb_function_bind_config,
};

void usb_adb_init(void)
{
    pr_info("%s enter\n", __FUNCTION__);
    pnp_register_usb_support_function(&adb_function);
}
