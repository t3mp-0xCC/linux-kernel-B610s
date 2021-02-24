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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include "bsp_nvim.h"
#include "bsp_pmu.h"
#include "led_dr.h"

#include <linux/err.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include "bsp_leds.h"
#include "bsp_regulator.h"
#include "bsp_icc.h"
#include "bsp_softtimer.h"
#include <../leds/leds_balong.h>
#include "product_config.h"
#include <linux/of_platform.h>
/*******************************类型定义区**********************************/
/* DR相关 */

typedef enum
{
    BREATH_DR01 = 1,
    BREATH_DR02 ,
    BREATH_DR03 ,
    BREATH_DR04 ,
    BREATH_DR05 ,
    BREATH_DR_BOTTOM
}breathled_id_e;

/* 呼吸灯使能开关 */
typedef enum
{
    BREATH_LED_BREATH    = 0,  /* LED灯呼吸模式打开 */
    BREATH_LED_TIMER     = 1,  /* LED灯闪烁模式打开 */
    BREATH_LED_NONE      = 2,  /* LED灯关闭 */
    BREATH_LED_BOTTOM, 
}breath_led_ctrl;

typedef struct
{
    unsigned char led_enable;        /* LED灯使能开关 */
    unsigned char led_dr;            /* LED灯所使用的DR */
    unsigned char led_mode;          /* LED灯模式标志 */
    unsigned char led_reserve;        /* LED灯模式标志 */
    unsigned int  full_on;           /* LED灯呼吸模式稳定亮的持续时间 */
    unsigned int  full_off;          /* LED灯呼吸模式稳定暗的持续时间 */
    unsigned int  fade_on;           /* LED灯呼吸模式从暗到亮的持续时间 */
    unsigned int  fade_off;          /* LED灯呼吸模式从亮到暗的持续时间 */
    unsigned int  delay_on;          /* LED灯闪烁模式亮的时间 */
    unsigned int  delay_period;     /* LED灯闪烁模式的闪烁周期时间*/
    unsigned int  full_long_on;     /* LED灯长亮的持续时间 */
    unsigned int  full_long_off;    /* LED灯长暗的持续时间 */
    unsigned int  brightness;       /* LED灯亮度电流值 */
}NV_LED_PARA_STRU;

/* 属性 */
typedef enum
{
    BRT_ATTR_FULL_ON = 1,    /* LED灯稳定亮持续时间属性 */
    BRT_ATTR_FULL_OFF,       /* LED灯稳定灭持续时间属性 */
    BRT_ATTR_FADE_ON,        /* LED灯由暗变亮持续时间属性 */
    BRT_ATTR_FADE_OFF,       /* LED灯由亮变暗持续时间属性 */
    BRT_ATTR_DELAY_ON,       /* LED灯闪烁亮持续时间属性 */
    BRT_ATTR_DELAY_OFF,      /* LED灯闪烁灭持续时间属性 */
    BRT_ATTR_BRIGHTNESS,     /* LED灯闪亮度值属性 */
    BRT_ATTR_B0TTOM
}breathled_attr_e;
/*******************************宏定义区**********************************/

#define ZERO_TIME  0

/* 呼吸灯使能 */
#define BREATH_LED_ENABLE   1   /* 呼吸灯使能 */
#define BREATH_LED_DISABLE  0   /* 呼吸灯非使能 */

/* 呼吸灯模式 */
#define LED_MODE_BREATH     0   /* 呼吸灯呼吸模式 */

#define LED_MODE_BLINK      1   /* 呼吸灯闪烁模式 */

#define NV_ID_LED_PARA  50455

#define ATTR_INFO_LEN 20
#define CONTAINER_LEN 200
#define LED_MAX   32           /*最大可增加的led 节点数*/

/* 呼吸灯各属性名称 */
#define BREATH_TRIGGER_BREATH     "breath"
#define BREATH_TRIGGER_TIMER      "timer"
#define BREATH_TRIGGER_NONE       "none"

#define BREATH_TRIGGER            "trigger"
#define BREATH_FULL_ON            "full_on"
#define BREATH_FULL_OFF           "full_off"
#define BREATH_FADE_ON            "fade_on"
#define BREATH_FADE_OFF           "fade_off"
#define BREATH_DELAY_ON           "delay_on"
#define BREATH_DELAY_OFF          "delay_off"
#define BREATH_BRIGHTNESS         "brightness"
/* pmic_dr.dtsi 中定义了dr点亮时间数组，最长时间是 50秒，即长亮 */
#define LED_LONG_TIME             (50000000)    /*长亮、长暗寄存器固定配置值*/

PRIVATE char path_container[CONTAINER_LEN];
PRIVATE const char* container_format   = "/sys/devices/platform/balong_led/leds/Balong_dr%d/%s";
int  g_dr_name = 0;   
int g_dr_brightness[5] = {-1, -1, -1, -1, -1};     /*当前的亮度标识*/
extern struct workqueue_struct *led_workqueue_brightness;    /*亮灭控制队列*/

/*******************************函数声明区**********************************/
extern int led_dr_regulator_enable(int led_id);

int breathled_controller(breathled_state led_state);

static DEFINE_MUTEX(led_dr_mutex); /* LED操作锁 */

/*******************************函数实现区**********************************/


PRIVATE int assemble_container_dr(const char * cmd_obj)
{   
    int len = 0;

    if(!cmd_obj)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] arguments is NULL\n", __FUNCTION__);
        return BR_ERROR;
    }
    
    /* 置空容器 */
    (void)memset(path_container,0x0, CONTAINER_LEN);/*lint !e160 !e506*/

    /* 填充容器 */
    len = snprintf(path_container, CONTAINER_LEN, container_format, g_dr_name, cmd_obj);
    if(len < 0)
    {
        printk(KERN_ERR "[*Breath-Led*] %s: assemble %s failed.\n", __func__, cmd_obj);
        return BR_ERROR;
    }

    return BR_OK;
}

PRIVATE int set_led_dr_attr(breathled_attr_e br_attr, unsigned long attr_value)
{
    struct file *attr_handle = NULL;
    int ret = BR_ERROR;
    int len = 0;
   
    char value_buf[ATTR_INFO_LEN];
    
    char attr_buf[ATTR_INFO_LEN];
    mm_segment_t old_fs = get_fs();
    /*lint -e160 -e506*/
    (void)memset(value_buf,0x0, sizeof(value_buf));
    (void)memset(attr_buf,0x0, sizeof(attr_buf));
    /*lint +e160 +e506*/
    switch(br_attr)
    {
        case BRT_ATTR_FULL_ON:
         
            strncpy(attr_buf, BREATH_FULL_ON, sizeof(BREATH_FULL_ON));
            break;
        case BRT_ATTR_FULL_OFF:
          
            strncpy(attr_buf, BREATH_FULL_OFF, sizeof(BREATH_FULL_OFF));
            break;
        case BRT_ATTR_FADE_ON:
          
            strncpy(attr_buf, BREATH_FADE_ON, sizeof(BREATH_FADE_ON));
            break;
        case BRT_ATTR_FADE_OFF:
            
            strncpy(attr_buf, BREATH_FADE_OFF, sizeof(BREATH_FADE_OFF));
            break;
        case BRT_ATTR_DELAY_ON:
           
            strncpy(attr_buf, BREATH_DELAY_ON, sizeof(BREATH_DELAY_ON));
            break;
        case BRT_ATTR_DELAY_OFF:
           
            strncpy(attr_buf, BREATH_DELAY_OFF, sizeof(BREATH_DELAY_OFF));
            break;
        case BRT_ATTR_BRIGHTNESS:
            
            strncpy(attr_buf, BREATH_BRIGHTNESS, sizeof(BREATH_BRIGHTNESS));
            break;
        default:
           
            printk(KERN_ERR "%s: BREATH ATTR error.\n", __func__);
            return BR_ERROR;
    }


    ret = assemble_container_dr(attr_buf);
    if(BR_ERROR == ret)
    {
        return BR_ERROR;
    }

    len = snprintf(value_buf, sizeof(value_buf), "%ld", attr_value);
    if(len < 0) 
    {
        printk(KERN_ERR "[*Breath-Led*] %s: fill value buffer failded.\n", __func__);
        set_fs(old_fs);
        return BR_ERROR;
    }

    set_fs(KERNEL_DS);

    attr_handle = filp_open(path_container, O_RDWR, S_IRWXU);
    if(IS_ERR(attr_handle))
    {
        printk(KERN_ERR "[*Breath-Led*] %s: sys_open \"%s\" failed.\n", __func__,path_container);
        set_fs(old_fs);
        return BR_ERROR;
    }

    len = attr_handle->f_op->write(attr_handle, value_buf, len, &attr_handle->f_pos);
    if(len < 0) 
    {
        printk(KERN_ERR "[*Breath-Led*] %s: sys_write \"%s\" failed.\n", __func__,path_container);
        (void)filp_close(attr_handle, NULL);
        set_fs(old_fs);
        return BR_ERROR;
    }
    
    (void)filp_close(attr_handle, NULL);
    set_fs(old_fs);
    
    return BR_OK;
}

int led_dr_trigger_hold(void)
{
    struct file *attr_handle = NULL;
    int ret = BR_ERROR;
    int len = -1;
    mm_segment_t old_fs = get_fs();

    ret = assemble_container_dr(BREATH_TRIGGER);
    if(BR_ERROR == ret)
    {
        return BR_ERROR;
    }

    set_fs(KERNEL_DS);

     attr_handle = filp_open(path_container, O_RDWR, S_IRWXU);
    if(IS_ERR(attr_handle))
    {
        printk(KERN_ERR "[*Breath-Led*] %s: sys_open \"%s\" failed.\n", __func__,path_container);
        set_fs(old_fs);
        return BR_ERROR;
    }
    len = attr_handle->f_op->write(attr_handle, BREATH_TRIGGER_BREATH, 
                                                sizeof(BREATH_TRIGGER_BREATH), &attr_handle->f_pos);           
    if(len < 0) 
    {
        printk(KERN_ERR "[*Breath-Led*] %s: sys_write \"%s\" failed.\n", __func__,path_container);
        (void)filp_close(attr_handle, NULL);
        set_fs(old_fs);
        return BR_ERROR;
    }
    
    (void)filp_close(attr_handle, NULL);
    set_fs(old_fs);
    
    return BR_OK;
}

int led_dr_breath_set(struct led_classdev *led_cdev, unsigned long *full_on, unsigned long *full_off, unsigned long *fade_on, unsigned long *fade_off)
{   
    int ret = BR_ERROR;
    struct balong_led_device    *led_dat = NULL;
    /* argument check */
    if((!led_cdev) || (!full_on) || (!full_off) || (!fade_on) || (!fade_off))
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] arguments is NULL\n", __FUNCTION__);
        return BR_ERROR;
    }

    led_dat = container_of(led_cdev, struct balong_led_device, cdev);
    if(!led_dat)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] arguments is NULL\n", __FUNCTION__);
        return BR_ERROR;
    }

    mutex_lock(&led_dr_mutex);
    led_cdev->breath_full_on = *full_on;
    led_cdev->breath_full_off = *full_off;
    led_cdev->breath_fade_on = *fade_on;
    led_cdev->breath_fade_off = *fade_off;
    
    g_dr_name = led_dat->dr;
    
    g_dr_brightness[g_dr_name - 1] = -1;        /*亮度值复位*/
    
    ret = led_dr_trigger_hold();
    if(BR_ERROR == ret)
    {
        goto out;
    }
    /*应用层下发的时间参数是毫秒级的，而底层定义的参数是微秒级的，在用户态参数传入的时候需要转换*/
    ret = set_led_dr_attr(BRT_ATTR_FULL_ON, (led_cdev->breath_full_on)*1000);
    if(BR_ERROR == ret)
    {
        goto out;
    }
    ret = set_led_dr_attr(BRT_ATTR_FULL_OFF, (led_cdev->breath_full_off)*1000);
    if(BR_ERROR == ret)
    {
        goto out;
    }
    ret = set_led_dr_attr(BRT_ATTR_FADE_ON, (led_cdev->breath_fade_on)*1000);
    if(BR_ERROR == ret)
    {
        goto out;
    }
    ret = set_led_dr_attr(BRT_ATTR_FADE_OFF, (led_cdev->breath_fade_off)*1000);
    if(BR_ERROR == ret)
    {
        goto out;
    }
    
     /*时间赋值完毕进入亮度设置*/
    if ((led_cdev->breath_full_on)&&(led_cdev->breath_full_off)&&(led_cdev->breath_fade_on)&&(led_cdev->breath_fade_off))
    {
        if ((0 == led_cdev->brightness) || (1 == led_cdev->brightness))
        {               
            ret = set_led_dr_attr(BRT_ATTR_BRIGHTNESS, led_dat->default_brightness);
        }
        else 
        {
            ret = set_led_dr_attr(BRT_ATTR_BRIGHTNESS, led_cdev->brightness);
        }    

        if(BR_ERROR == ret)
        {
            goto out;
        }
    }

out:
    mutex_unlock(&led_dr_mutex);
    return BR_OK;
}


static void led_dr_work(struct work_struct *work)
{
    int ret = BR_ERROR;
    unsigned long time_local = 0;
    int brightness = 0;
    
    struct balong_led_device	*led_dat = NULL;
    struct led_classdev *led_cdev = NULL;
	
    mutex_lock(&led_dr_mutex);
    led_dat = container_of(work, struct balong_led_device, work);
    led_cdev = &(led_dat->cdev);
    brightness = led_dat->brightness;

    if(!led_cdev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] arguments is NULL\n", __FUNCTION__);
        goto out;
    }
    
    g_dr_name = led_dat->dr;
    
    if (brightness >= LED_FULL)
    {
        brightness = LED_FULL;
    }
    
    if(1 == brightness)
    {
        brightness = led_dat->default_brightness; /* 如果用户态传1则设置为默认亮度 */
    }
        
    if(g_dr_brightness[g_dr_name - 1] == brightness)    /*重复设置亮度*/
    {
        goto out;
    }
    g_dr_brightness[g_dr_name - 1] = brightness;    /*亮度值更新*/

    ret = led_dr_trigger_hold();
    if(BR_ERROR == ret)
    {
        goto out;
    }
    /*设置灯的状态为长灭*/
    if (LED_OFF == brightness)
    {
        /* 无需操作，把亮度改了就可以 */
    }
    /*设置灯的状态为长亮*/
    else
    {
        ret = set_led_dr_attr(BRT_ATTR_FULL_ON, LED_LONG_TIME);
        if(BR_ERROR == ret)
        {
            printk(KERN_ERR "[*Breath-Led*] %s: set_led_dr_attr(BRT_ATTR_FULL_ON, LED_LONG_TIME) failded.\n", __func__);
            goto out;
        }
        ret = set_led_dr_attr(BRT_ATTR_FULL_OFF, time_local);
        if(BR_ERROR == ret)
        {
            printk(KERN_ERR "[*Breath-Led*] %s: set_led_dr_attr(BRT_ATTR_FULL_OFF, time_local) failded.\n", __func__);
            goto out;
        }
    }
    ret = set_led_dr_attr(BRT_ATTR_FADE_ON, time_local);
    if(BR_ERROR == ret)
    {
        goto out;
    }
    ret = set_led_dr_attr(BRT_ATTR_FADE_OFF, time_local);
    if(BR_ERROR == ret)
    {
        goto out;
    }
    
    ret = set_led_dr_attr(BRT_ATTR_BRIGHTNESS, brightness);
    if(BR_ERROR == ret)
    {
        goto out;
    }

out:
    mutex_unlock(&led_dr_mutex);
}


void led_dr_brightness_set(struct led_classdev *led_cdev, enum led_brightness brightness)
{
    struct balong_led_device    *led_dat =
        container_of(led_cdev, struct balong_led_device, cdev);
    led_dat->brightness = brightness;
    queue_work(led_workqueue_brightness, &(led_dat->work));
}

enum led_brightness led_dr_brightness_get(struct led_classdev *led_cdev)
{
    if(!led_cdev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] arguments is NULL\n", __FUNCTION__);
        return (enum led_brightness)BR_ERROR;  
    }
    
    return (enum led_brightness)led_cdev->brightness;   
}

int led_dr_led_blink_set(struct led_classdev *led_cdev, unsigned long *delay_on, unsigned long *delay_off)
{
    return BR_OK;
}


/* 保存从设备树中读取到的数据  */
struct dr_led {
	const char *name;               /* 灯的名字 */
    const char *default_trigger;    /* 默认的触发器 */
    unsigned    default_state;      /* 开机时默认的状态 */
    unsigned    default_brightness; /* 默认的亮度 */
    unsigned    dr;                 /* 灯所连接的DR */
};

struct dr_leds_priv {
	int num_leds;
	struct balong_led_device leds[];
};

static struct dr_leds_priv *g_led_dr_priv = NULL;

/************************************************************************
 *函数原型 ： int dr_led_init_status(void)
 *描述     ： 检测dr控制led初始化状态
 *输入     ： NA
 *输出     ： NA
 *返回值   ： 初始化成功 0；初始化失败-1
*************************************************************************/
int dr_led_init_status(void)
{
    int ret = 0;
    if(NULL == g_led_dr_priv)
    {
        printk(KERN_ERR "[leds-dr] led driver is not init!\n");
        ret = -1;
    }
    return ret;
}

int dr_led_name2dr(char *led_name)
{
    int i;

    if ( NULL == led_name )
    {
        printk(KERN_ERR "[leds-dr] name is NULL or led driver is not init!\n");
        return LEDS_ERROR;
    }
    else if (NULL == g_led_dr_priv)
    {
        printk(KERN_ERR "[leds-dr] led driver is not init!\n");
        return LEDS_NOT_READY;
    }
    else
    {
        /*根据name  查找对应控制的led*/
        for (i = 0; i < g_led_dr_priv->num_leds; i++)    
        {
            if (!strncmp(g_led_dr_priv->leds[i].cdev.name , led_name , strlen(g_led_dr_priv->leds[i].cdev.name) ))
            {
                printk(KERN_INFO "[leds-dr] Find led named %s in dts!\n", led_name);
                return g_led_dr_priv->leds[i].dr;
            }
        }
    }

    printk(KERN_INFO "[leds-dr] Do not find dr led named %s in dts!\n", led_name);
    return LEDS_ERROR;
}

int dr_led_get_default_brightness(int numb)
{
    if (NULL == g_led_dr_priv)
    {
        printk(KERN_ERR "[leds-dr] led driver is not init!\n");
        return LEDS_NOT_READY;
    }
    else if((0 <= numb) && (numb < g_led_dr_priv->num_leds))
    {
        return g_led_dr_priv->leds[numb].default_brightness;
    }
    
    return LEDS_ERROR;
}

static void delete_dr_led(struct balong_led_device *led)
{
	led_classdev_unregister(&led->cdev);
	cancel_work_sync(&led->work);
    kfree(led->pdata);
}

static inline int sizeof_dr_leds_priv(int num_leds)
{
	return sizeof(struct dr_leds_priv) +
		(sizeof(struct balong_led_device) * num_leds);
}

static int create_dr_led(const struct dr_led *template,
	struct balong_led_device *led_dat, struct device *parent)
{
	int ret, state;

    led_dat->pdata =(struct balong_led_platdata*)kzalloc(sizeof(struct balong_led_platdata), GFP_KERNEL);
    if(!led_dat->pdata)
    {
        printk(KERN_ERR "[leds-dr] %s get pdata buffer failed\n", __FUNCTION__);
        return -1;
    }

	led_dat->dr = template->dr;
	led_dat->cdev.name = template->name;
	led_dat->cdev.default_trigger = template->default_trigger;

	led_dat->cdev.blink_set             = led_dr_led_blink_set;
	led_dat->cdev.brightness_set        = led_dr_brightness_set;
    led_dat->pdata->led_breath_set      = led_dr_breath_set;
    led_dat->cdev.brightness_get        = led_dr_brightness_get;

	state = (LEDS_GPIO_DEFSTATE_ON == template->default_state);

	led_dat->cdev.brightness = state ? template->default_brightness : LED_OFF;
    led_dat->default_brightness = template->default_brightness;
	INIT_WORK(&led_dat->work, led_dr_work);

	ret = led_classdev_register(parent, &led_dat->cdev);
	if (ret < 0)
		return ret;

	return 0;
}

static struct dr_leds_priv *dr_leds_create_of(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node, *child;
	struct dr_leds_priv *priv;
	int count, ret;
    const char * state = NULL;
	/* count LEDs in this device, so we know how much to allocate */
	count = of_get_child_count(np);
	if (!count)
		return ERR_PTR(-ENODEV);

	priv = devm_kzalloc(&pdev->dev, sizeof_dr_leds_priv(count),
			GFP_KERNEL);
	if (!priv)
		return ERR_PTR(-ENOMEM);

	for_each_child_of_node(np, child) {
		struct dr_led led = {};

        led.name = of_get_property(child, "label", NULL) ? : child->name;
		led.default_trigger = of_get_property(child, "linux,default-trigger", NULL);
        state = of_get_property(child, "default-state", NULL);
        if (state) {
			if (!strncmp(state, "keep", strlen(state)))
				led.default_state = LEDS_GPIO_DEFSTATE_KEEP;
			else if (!strncmp(state, "on", strlen(state)))
				led.default_state = LEDS_GPIO_DEFSTATE_ON;
			else
				led.default_state = LEDS_GPIO_DEFSTATE_OFF;
		}
        ret = of_property_read_u32_index(child, "drs", 0, &(led.dr));
        ret |= of_property_read_u32_index(child, "linux,default-brightness", 0, &(led.default_brightness));
        if( 0 != ret)
        {
            printk(KERN_EMERG "[leds-dr] of_property_read_u32_index ERROR! ret=%d.\n", ret);
            led.dr = -ENOSYS;
        }

		ret = create_dr_led(&led, &priv->leds[priv->num_leds++],
				      &pdev->dev);
		if (ret < 0) {
			of_node_put(child);
			goto err;
		}
	}
    g_led_dr_priv = priv;
	return priv;

err:
	for (count = priv->num_leds - 2; count >= 0; count--)
		delete_dr_led(&priv->leds[count]);
	return ERR_PTR(-ENODEV);
}

static const struct of_device_id of_dr_leds_match[] = {
	{ .compatible = "dr-leds", },
	{},
};

    /*lint -e54 -e119 -e30 -e84 -e514 */
static int led_dr_probe(struct platform_device *dev)
{
    struct led_platform_data *pdata = dev->dev.platform_data;
    struct dr_leds_priv *priv = NULL;

    if (pdata && pdata->num_leds) 
    {
        /* 不使用设备树的方式 */
        printk(KERN_EMERG "[leds-dr] Now we use device tree to config LED!\n");
        return -1;
    }
    else
    {
        /* 从设备树中读取LED的配置 */
        priv = dr_leds_create_of(dev);
        if (IS_ERR(priv))
	    {
	        return PTR_ERR(priv);
        }
    }

    platform_set_drvdata(dev, priv);
    printk(KERN_ERR "[leds-dr] led_dr_probe OK!\n");
    return 0;
}

static int led_dr_remove(struct platform_device *dev)
{
	struct dr_leds_priv *priv = platform_get_drvdata(dev);
	int i;

	for (i = 0; i < priv->num_leds; i++)
		delete_dr_led(&priv->leds[i]);

	platform_set_drvdata(dev, NULL);
    return BR_OK;
}
    /*lint +e54 +e119 +e30 +e84 +e514 */

static struct platform_driver led_dr_drv = {
    .probe =led_dr_probe,
    .remove = led_dr_remove,
    .driver = {  
        .name  = "led_dr",  
        .owner = THIS_MODULE, 
        .of_match_table = of_match_ptr(of_dr_leds_match),
    },  
};

/*lint -e629*/
module_platform_driver(led_dr_drv);

/*lint +e629*/
MODULE_AUTHOR("MBB.Huawei Device");
MODULE_DESCRIPTION("Breath Led Driver");
MODULE_LICENSE("GPL");
