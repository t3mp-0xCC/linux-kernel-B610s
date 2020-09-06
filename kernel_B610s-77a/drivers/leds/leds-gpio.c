/*
 * LEDs driver for GPIOs
 *
 * Copyright (C) 2007 8D Technologies inc.
 * Raphael Assenat <raph@8d.com>
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
 
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/leds.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/pinctrl/consumer.h>
#include <product_config.h>
#include <bsp_wlan.h>
#include <linux/err.h>
#include <linux/wakelock.h>
static struct wake_lock huawei_led_wake_lock; /*lint !e86*/
#define LED_DELAY_TIME  (20 * HZ)    /*应对led节点初始化提前，改为20秒*/
extern struct workqueue_struct *led_workqueue_brightness;    /*亮灭控制队列*/

struct gpio_led_data {
	struct led_classdev cdev;
	unsigned gpio;
	struct work_struct work;
	u8 new_level;
	u8 can_sleep;
	u8 active_low;
	u8 blinking;
	int (*platform_gpio_blink_set)(unsigned gpio, int state,
			unsigned long *delay_on, unsigned long *delay_off);
};

static void gpio_led_work(struct work_struct *work)
{
	struct gpio_led_data	*led_dat =
		container_of(work, struct gpio_led_data, work);
    if (GPIO_NULL != led_dat->gpio)                  /*lint !e10*/
    {
        if (led_dat->blinking) {
            led_dat->platform_gpio_blink_set(led_dat->gpio,
                             led_dat->new_level,
                             NULL, NULL);
            led_dat->blinking = 0;
        } else
        gpio_set_value(led_dat->gpio,  led_dat->new_level);
    }
    else
    {
        /*wlan_set_led_flag(led_dat->new_level);*/     /*lint !e628*/
        /* 专门给特殊的灯预留的通道，可以在设备树中配置成NULL，在这里走特殊处理 */
    }
}

static void gpio_led_set(struct led_classdev *led_cdev,
	enum led_brightness value)
{
	struct gpio_led_data *led_dat =
		container_of(led_cdev, struct gpio_led_data, cdev);
	int level;
	if (value == LED_OFF)
		level = 0;
	else
		level = 1;

	if (led_dat->active_low)
		level = !level;

	/* Setting GPIOs with I2C/etc requires a task context, and we don't
	 * seem to have a reliable way to know if we're already in one; so
	 * let's just assume the worst.
	 */
    if (GPIO_NULL != led_dat->gpio)             /*lint !e10*/
    {
        if (1) {
        led_dat->new_level = level;
        queue_work(led_workqueue_brightness, &(led_dat->work));
	} else {
		if (led_dat->blinking) {
			led_dat->platform_gpio_blink_set(led_dat->gpio, level,
							 NULL, NULL);
			led_dat->blinking = 0;
		} else
			gpio_set_value(led_dat->gpio, level);
	}
    }
    else
    { 
        led_dat->new_level = level;
        schedule_work(&led_dat->work);            /*lint !e534*/
    }
}

static int gpio_blink_set(struct led_classdev *led_cdev,
	unsigned long *delay_on, unsigned long *delay_off)
{
	struct gpio_led_data *led_dat =
		container_of(led_cdev, struct gpio_led_data, cdev);

	led_dat->blinking = 1;
	return led_dat->platform_gpio_blink_set(led_dat->gpio, GPIO_LED_BLINK,
						delay_on, delay_off);
}

static int create_gpio_led(const struct gpio_led *template,
	struct gpio_led_data *led_dat, struct device *parent,
	int (*blink_set)(unsigned, int, unsigned long *, unsigned long *))
{
	int ret, state;

	led_dat->gpio = -1;
    if (GPIO_NULL != template->gpio)         /*lint !e10*/
    {
	/* skip leds that aren't available */
	if (!gpio_is_valid(template->gpio)) {
		dev_info(parent, "Skipping unavailable LED gpio %d (%s)\n",
				template->gpio, template->name);
		return 0;
	}

	ret = devm_gpio_request(parent, template->gpio, template->name);
	if (ret < 0)
		return ret;
    }
	led_dat->cdev.name = template->name;
	led_dat->cdev.default_trigger = template->default_trigger;
	led_dat->gpio = template->gpio;
    if (GPIO_NULL != template->gpio)         /*lint !e10*/
    {
	led_dat->can_sleep = gpio_cansleep(template->gpio);
    }
	led_dat->active_low = template->active_low;
	led_dat->blinking = 0;
	if (blink_set) {
		led_dat->platform_gpio_blink_set = blink_set;
		led_dat->cdev.blink_set = gpio_blink_set;
	}
	led_dat->cdev.brightness_set = gpio_led_set;
	if (template->default_state == LEDS_GPIO_DEFSTATE_KEEP)
		state = !!gpio_get_value_cansleep(led_dat->gpio) ^ led_dat->active_low;
	else
		state = (template->default_state == LEDS_GPIO_DEFSTATE_ON);
	led_dat->cdev.brightness = state ? LED_FULL : LED_OFF;
	if (!template->retain_state_suspended)
		led_dat->cdev.flags |= LED_CORE_SUSPENDRESUME;
    if (GPIO_NULL != template->gpio)           /*lint !e10*/
    {
	ret = gpio_direction_output(led_dat->gpio, led_dat->active_low ^ state);
	if (ret < 0)
		return ret;
    }
	INIT_WORK(&led_dat->work, gpio_led_work);

	ret = led_classdev_register(parent, &led_dat->cdev);
	if (ret < 0)
		return ret;

	return 0;
}

static void delete_gpio_led(struct gpio_led_data *led)
{
	if (!gpio_is_valid(led->gpio))
		return;
	led_classdev_unregister(&led->cdev);
	cancel_work_sync(&led->work);
}

struct gpio_leds_priv {
	int num_leds;
	struct gpio_led_data leds[];
};

static struct gpio_leds_priv *g_led_gpio_priv = NULL;

/************************************************************************
 *函数原型 ： int gpio_led_init_status(void)
 *描述     ： 检测led初始化状态
 *输入     ： NA
 *输出     ： NA
 *返回值   ： 初始化成功 0；初始化失败-1
*************************************************************************/
int gpio_led_init_status(void)
{
    int ret = 0;
    if(NULL == g_led_gpio_priv)
    {
        printk(KERN_ERR "[leds-gpio] led driver is not init!\n");
        ret = -1;
    }
    return ret;
}

int gpio_led_name2gpio(char *led_name)
{
    int i;
    if ( NULL == led_name )
    {
        printk(KERN_ERR "[leds-gpio] name is NULL!\n");
        return LEDS_ERROR;
    }
    else if (NULL == g_led_gpio_priv)
    {
        printk(KERN_ERR "[leds-gpio] led driver is not init!\n");
        return LEDS_NOT_READY;
    }
    else
    {
        /*根据name  查找对应控制的led*/
        for (i = 0; i < g_led_gpio_priv->num_leds; i++)    
        {
            if (!strncmp(g_led_gpio_priv->leds[i].cdev.name , led_name , strlen(g_led_gpio_priv->leds[i].cdev.name) ))
            {
                printk(KERN_INFO "[leds-gpio] Find led named %s in dts!\n", led_name);
                return (int)g_led_gpio_priv->leds[i].gpio;
            }
        }
    }
    printk(KERN_INFO "[leds-gpio] Do not find dr led named %s in dts!\n", led_name);
    return -1;
}

static inline int sizeof_gpio_leds_priv(int num_leds)
{
	return sizeof(struct gpio_leds_priv) +
		(sizeof(struct gpio_led_data) * num_leds);
}

/* Code to create from OpenFirmware platform devices */
static struct gpio_leds_priv *gpio_leds_create_of(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node, *child;
	struct gpio_leds_priv *priv;
	int count, ret;

	/* count LEDs in this device, so we know how much to allocate */
	count = of_get_child_count(np);
	if (!count)
		return ERR_PTR(-ENODEV);


	priv = devm_kzalloc(&pdev->dev, sizeof_gpio_leds_priv(count),
			GFP_KERNEL);
	if (!priv)
		return ERR_PTR(-ENOMEM);

	for_each_child_of_node(np, child) {
		struct gpio_led led = {};
		enum of_gpio_flags flags;
		const char *state;
        /* C60基线海思没有提供标准GPIO设备树，后续V7R5提供后使用内核标准机制 */
        ret = of_property_read_u32_index(child, "gpios", 0, &(led.gpio));
        ret |= of_property_read_u32_index(child, "gpios", 1, &(flags));
        if( 0 != ret)
        {
            printk(KERN_EMERG "[leds-gpio] of_property_read_u32_index ERROR! ret=%d.\n", ret);
            led.gpio = -ENOSYS;
        }
        printk(KERN_INFO "[leds-gpio] gpio=%d, active_low=%d.\n", led.gpio, flags);
		led.active_low = flags & OF_GPIO_ACTIVE_LOW;
		led.name = of_get_property(child, "label", NULL) ? : child->name;
		led.default_trigger =
			of_get_property(child, "linux,default-trigger", NULL);
		state = of_get_property(child, "default-state", NULL);
		if (state) {
			if (!strcmp(state, "keep"))
				led.default_state = LEDS_GPIO_DEFSTATE_KEEP;
			else if (!strcmp(state, "on"))
				led.default_state = LEDS_GPIO_DEFSTATE_ON;
			else
				led.default_state = LEDS_GPIO_DEFSTATE_OFF;
		}

		ret = create_gpio_led(&led, &priv->leds[priv->num_leds++],
				      &pdev->dev, NULL);
		if (ret < 0) {
			of_node_put(child);
			goto err;
		}
	}

	return priv;

err:
	for (count = priv->num_leds - 2; count >= 0; count--)
		delete_gpio_led(&priv->leds[count]);
	return ERR_PTR(-ENODEV);
}

static const struct of_device_id of_gpio_leds_match[] = {
	{ .compatible = "gpio-leds", },
	{},
};


static int gpio_led_probe(struct platform_device *pdev)
{
	struct gpio_led_platform_data *pdata = pdev->dev.platform_data;
	struct gpio_leds_priv *priv;
	struct pinctrl *pinctrl;
	int i, ret = 0;

	pinctrl = devm_pinctrl_get_select_default(&pdev->dev);
	if (IS_ERR(pinctrl))
		dev_warn(&pdev->dev,
			"pins are not configured from the driver\n");

	if (pdata && pdata->num_leds) {
		priv = devm_kzalloc(&pdev->dev,
				sizeof_gpio_leds_priv(pdata->num_leds),
					GFP_KERNEL);
		if (!priv)
			return -ENOMEM;

		priv->num_leds = pdata->num_leds;
		for (i = 0; i < priv->num_leds; i++) {
			ret = create_gpio_led(&pdata->leds[i],
					      &priv->leds[i],
					      &pdev->dev, pdata->gpio_blink_set);
			if (ret < 0) {
				/* On failure: unwind the led creations */
				for (i = i - 1; i >= 0; i--)
					delete_gpio_led(&priv->leds[i]);
				return ret;
			}
		}
	} else {
		priv = gpio_leds_create_of(pdev);
		if (IS_ERR(priv))
			return PTR_ERR(priv);
	}
    g_led_gpio_priv = priv;
    wake_lock_init(&huawei_led_wake_lock, WAKE_LOCK_SUSPEND,"huawei_led_wakelock");
    wake_lock_timeout(&huawei_led_wake_lock, LED_DELAY_TIME);

	platform_set_drvdata(pdev, priv);

	return 0;
}

static int gpio_led_remove(struct platform_device *pdev)
{
	struct gpio_leds_priv *priv = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < priv->num_leds; i++)
		delete_gpio_led(&priv->leds[i]);

	platform_set_drvdata(pdev, NULL);

    wake_lock_destroy(&huawei_led_wake_lock);
	return 0;
}

static struct platform_driver gpio_led_driver = {
	.probe		= gpio_led_probe,
	.remove		= gpio_led_remove,
	.driver		= {
		.name	= "leds-gpio",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(of_gpio_leds_match),
	},
};

module_platform_driver(gpio_led_driver);

MODULE_AUTHOR("Raphael Assenat <raph@8d.com>, Trent Piepho <tpiepho@freescale.com>");
MODULE_DESCRIPTION("GPIO LED driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:leds-gpio");
