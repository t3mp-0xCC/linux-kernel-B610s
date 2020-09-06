 

/*lint -save -e537*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/pm.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/input/key_balong.h>
#if(FEATURE_ON == MBB_KEY_BOOST)
#include <asm-generic/cputime_jiffies.h>
#endif

#include "linux/wakelock.h"
#include "bsp_om.h"
#include "bsp_pmu.h"

#if(FEATURE_ON == MBB_KEY_BOOST)
/*add for touch boost*/
#include "../../cpufreq/cpufreq_balong.h"
#include "bsp_cpufreq.h"
#endif

/*lint -restore */
#include <linux/mlog_lib.h>

#if (FEATURE_ON == MBB_FAST_ON_OFF)
#include <drv_fastOnOff.h>
#endif
/* GPIO配置统一放到mbb_adapt.h里面 */

#if ( FEATURE_ON == MBB_KEY )
/* 事件上报延时锁，默认100ms，假关机后变为3秒 */
int WAKE_LOCK_TIME = 100;
#else
/*延时锁为100ms*/
#define WAKE_LOCK_TIME    (100)
#endif

#define  key_print_error(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_KEY, "[key]: <%s> <%d> "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define  key_print_info(fmt, ...)     (bsp_trace(BSP_LOG_LEVEL_ERROR,  BSP_MODU_KEY, "[key]: "fmt, ##__VA_ARGS__))
#if(FEATURE_ON == MBB_KEY_BOOST)
#define  TOUCH_BOOST_DURATION         3000000
#endif
typedef enum 
{
    RELEASE = 0,
    PRESS = 1,
    BUTT
}KEY_EVENT_TYPE;

/*lint -save -e958*/
struct gpio_button_data {
    const struct gpio_keys_button *button;
    struct wake_lock *key_wake_lock;
    struct input_dev *input;
    struct timer_list timer;
    unsigned int timer_debounce;    /* in msecs */
    unsigned int irq;
    spinlock_t lock;
    int disabled;
    int key_pressed;
#if ( FEATURE_ON == MBB_KEY )
    int is_suspend;   /* GPIO控制的按键是否进入休眠 */
    int event_type;   /* 按键是否在休眠中未唤醒前被按下  */
#endif
};
/*lint -restore*/

struct balong_gpio_key {
    struct input_dev *input;
    struct wake_lock key_wake_lock;
    
    unsigned int suspended;
    unsigned int event_type;
    
    unsigned int n_buttons;
    struct gpio_button_data data[0];/*lint !e43*/
};

struct balong_gpio_key *g_suspend_gpio_key = NULL;
#if (FEATURE_ON == MBB_FAST_ON_OFF)
/* 假关机通知链按键模块处理函数 */
static int key_fastonoff_event(struct notifier_block *nb, unsigned long event, void *v);

/* 假关机时通知按键模块用的数据结构 */
struct notifier_block key_fastonoff_notifier = {
    .notifier_call = key_fastonoff_event,
};

/******************************************************************************
*  Function:  key_fastonoff_event
*  Description: SD卡升级组合键检测函数，检测到组合键被按下时触发定时器进一步检测
*  Called by: power_key_handle ，gpio_keys_gpio_report_event
*  Input:     int key:按键键值, int status:按键状态
*  Output:    None
*  Return:    None
*  Note  :      
*  History:  2013-07-18 初版作成  
********************************************************************************/
static int key_fastonoff_event(struct notifier_block *nb, unsigned long event, void *v)
{
	switch(event)
	{  
		case FASTONOFF_MODE_CLR:  
			printk(KERN_ERR "[KEY] got the chain event: FASTONOFF_MODE_CLR\n");
            /* 为保证假关机充电时也能点亮屏幕，和应用商议此处不做按键屏蔽*/
            WAKE_LOCK_TIME = 100; /* 假开机后调整为100ms */
			break;  
		case FASTONOFF_MODE_SET:  
			printk(KERN_ERR "[KEY] got the chain event: FASTONOFF_MODE_SET\n");
            /* 为保证假关机充电时也能点亮屏幕，和应用商议此处不做按键屏蔽*/
            WAKE_LOCK_TIME = 3000; /* 假关机后延时为3000ms */
			break;
		default:  
			printk(KERN_ERR "[KEY] got the chain event: event=%lu\n",event);
			break;  
	}

	return NOTIFY_DONE;
}
#endif

void report_power_key_up(void *data)
{
    struct balong_gpio_key *gpio_key = (struct balong_gpio_key*)data;

    input_event(gpio_key->input, EV_KEY, KEY_POWER, 0);
#if(FEATURE_ON == MBB_KEY_BOOST)
    cpufreq_set_boost(0, TOUCH_BOOST_DURATION);
#endif
    input_sync(gpio_key->input);
    
    wake_lock_timeout(&(gpio_key->key_wake_lock), (long)msecs_to_jiffies(WAKE_LOCK_TIME));
    key_print_info("power key is %s. \n", "release");   

    mlog_print(MLOG_KEY, mlog_lv_info, "Power key released.\n");
}

void report_power_key_down(void *data)
{
#if(FEATURE_ON == MBB_KEY_BOOST)
    int ret = 0;
#endif
    struct balong_gpio_key *gpio_key = (struct balong_gpio_key*)data;

    if(1 == gpio_key->suspended)
    {
        gpio_key->event_type = PRESS;
    }
    else
    {
        input_event(gpio_key->input, EV_KEY, KEY_POWER, 1);
#if(FEATURE_ON == MBB_KEY_BOOST)
        /*add for touch boost*/
        cpufreq_set_boost(1, 0);
        ret =  cpufreq_dfs_set_profile(BALONG_FREQ_MAX);
        if(0 != ret)
        {
            printk(KERN_ERR "cpufreq_dfs_set_profile is err\n");
        }
#endif
        input_sync(gpio_key->input);
        
        wake_lock_timeout(&(gpio_key->key_wake_lock), (long)msecs_to_jiffies(WAKE_LOCK_TIME));
        key_print_info("power key is %s. \n", "press");

        mlog_print(MLOG_KEY, mlog_lv_info, "Power key pressed.\n");
    }
}

void report_power_key_down_for_suspend()
{
    report_power_key_down(g_suspend_gpio_key);
    msleep(100);
}

void report_power_key_up_for_suspend()
{
    report_power_key_up(g_suspend_gpio_key);
}

static int balong_gpio_key_open(struct input_dev *dev)
{
    return 0;
}

static void balong_gpio_key_close(struct input_dev *dev)
{
    return;
}

static void gpio_keys_gpio_report_event(struct gpio_button_data *bdata)
{
    const struct gpio_keys_button *button = bdata->button;
    struct input_dev *input = bdata->input;
    unsigned int type = EV_KEY;
#if ( FEATURE_ON == MBB_KEY )
    /* 如果按键在单板休眠中被按下，则先记录按下标志，等唤醒后再上报事件 */
	if((1 == bdata->is_suspend) && (1 == bdata->key_pressed))
    {
        bdata->event_type = PRESS;
		return;
    }
#endif

    input_event(input, type, button->code, bdata->key_pressed);

    input_sync(input);

    wake_lock_timeout(bdata->key_wake_lock, (long)msecs_to_jiffies(WAKE_LOCK_TIME));    
    key_print_info("%s is %s. \n", bdata->button->desc, bdata->key_pressed ? "press":"release");

    mlog_print(MLOG_KEY, mlog_lv_info, "%s %s. \n", bdata->button->desc, 
               bdata->key_pressed ? "pressed":"released");

}

static void gpio_keys_gpio_timer(unsigned long _data)
{
    struct gpio_button_data *bdata = (struct gpio_button_data *)_data;
    unsigned long flags;

    spin_lock_irqsave(&bdata->lock, flags);
    
    if((!gpio_get_value((unsigned)(bdata->button->gpio))) && bdata->key_pressed)
    {
        gpio_keys_gpio_report_event(bdata);

    }
    else
    {
        bdata->key_pressed = 0;        
    }
    spin_unlock_irqrestore(&bdata->lock, flags);

}

static irqreturn_t gpio_keys_gpio_isr(int irq, void *dev_id)
{
    struct gpio_button_data *bdata = (struct gpio_button_data *)dev_id;
    unsigned long flags;

    spin_lock_irqsave(&bdata->lock, flags);

    if(!gpio_get_value((unsigned)(bdata->button->gpio)))
    {
        bdata->key_pressed = 1;
    }
    else
    {
        bdata->key_pressed = 0;
        gpio_keys_gpio_report_event(bdata);
    }

    if (bdata->timer_debounce && bdata->key_pressed)
    {
        mod_timer(&bdata->timer, jiffies + msecs_to_jiffies(bdata->timer_debounce));/*考虑fiffies是否会溢出*/
    }

    spin_unlock_irqrestore(&bdata->lock, flags);

    return IRQ_HANDLED;
}

static void gpio_remove_key(struct gpio_button_data *bdata)
{
    free_irq(bdata->irq, bdata);
    
    if (bdata->timer_debounce)
        del_timer_sync(&bdata->timer);
    
    if (gpio_is_valid(bdata->button->gpio))
        gpio_free((unsigned)(bdata->button->gpio));
}

static int __init gpio_keys_setup_key(struct platform_device *pdev,
                     struct input_dev *input,
                     struct gpio_button_data *bdata,
                     const struct gpio_keys_button *button)
{
    const char *desc = button->desc ? button->desc : "balong_gpio_key";
    struct device *dev = &pdev->dev;
    int error;

    bdata->input = input;
    bdata->button = button;
    spin_lock_init(&bdata->lock);

    error = gpio_request((unsigned)(button->gpio), desc);
    if (error < 0) {
        dev_err(dev, "Failed to request GPIO %d, error %d\n",
            button->gpio, error);
        goto err_request_gpio;
    }

    gpio_direction_input((unsigned)(button->gpio));
    
    bdata->timer_debounce = (unsigned int)(button->debounce_interval);
    
    
    if(gpio_get_value((unsigned)(button->gpio)))
    {
        bdata->key_pressed = 0;
    }
    else
    {
        bdata->key_pressed = 1;
    }
    setup_timer(&bdata->timer, gpio_keys_gpio_timer, (unsigned long)bdata);

    error = request_irq((unsigned int)gpio_to_irq((unsigned)(button->gpio)), \
                         gpio_keys_gpio_isr,\
                         IRQF_NO_SUSPEND | IRQF_SHARED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, \
                         bdata->button->desc, bdata);
    if (error) {
        dev_err(&pdev->dev, "Failed to request press interupt handler!\n");
        goto err_request_irq;
    }

    return 0;

err_request_irq:
    gpio_free((unsigned)(button->gpio));

err_request_gpio:
    
    return error;
}

/*lint -save -e830 -e438*/

#ifdef CONFIG_PM

static int balong_gpio_key_prepare(struct device *dev)
{
    return GPIO_OK;
}

static void balong_gpio_key_complete(struct device *dev)
{
#if ( FEATURE_ON == MBB_KEY )
    int i = 0;
#endif
    struct platform_device *pdev = to_platform_device(dev);
    struct balong_gpio_key *gpio_key = platform_get_drvdata(pdev);

    if(NULL == gpio_key){
        key_print_error("gpio_key is null pointer.\n");
        return ;
    }    
#if ( FEATURE_ON == MBB_KEY )
    /* 对GPIO键的处理 */
    for(i = 0; i < gpio_key->n_buttons; i++)
    {
    	gpio_key->data[i].is_suspend = 0;
		if(PRESS == gpio_key->data[i].event_type)
		{
		    gpio_key->data[i].event_type = BUTT;
		    gpio_key->data[i].key_pressed = 1;
			gpio_keys_gpio_report_event(&(gpio_key->data[i]));
            /* 如果按键已经释放，则这里补报一个释放事件 */
			if(gpio_get_value((unsigned)(gpio_key->data[i].button->gpio)))
			{
			    gpio_key->data[i].key_pressed = 0;
				gpio_keys_gpio_report_event(&(gpio_key->data[i]));
			}
		}
    }
    /* 对POWER键的处理 */
#endif
    gpio_key->suspended = 0;

    if(PRESS == gpio_key->event_type)
    {
        /* keyboard event report to app */
        report_power_key_down(gpio_key);
        
        gpio_key->event_type = BUTT;

        /*如果Power键已经释放，则这里补报一个释放事件*/
        if (0 == bsp_pmu_key_state_get())
        {
            report_power_key_up(gpio_key);
        }
    }
    
    return ;
}

static int balong_gpio_key_suspend(struct device *dev)
{
#if ( FEATURE_ON == MBB_KEY )
    int i = 0;
#endif
    struct platform_device *pdev = to_platform_device(dev);
    struct balong_gpio_key *gpio_key = platform_get_drvdata(pdev);

    if(NULL == gpio_key){
        key_print_error("gpio_key is null pointer.\n");
        return -1;
    }
#if ( FEATURE_ON == MBB_KEY )
    for(i = 0; i < gpio_key->n_buttons; i++)
    {
    	gpio_key->data[i].is_suspend = 1;
    }
#endif
    gpio_key->suspended = 1;
    return 0;
}

static int balong_gpio_key_resume(struct device *dev)
{
   
    return 0;
}

static const struct dev_pm_ops balong_gpio_key_dev_pm_ops ={
    .suspend  = balong_gpio_key_suspend,
    .resume   = balong_gpio_key_resume,
    .prepare  = balong_gpio_key_prepare,
    .complete = balong_gpio_key_complete,
};

#define BALONG_DEV_PM_OPS (&balong_gpio_key_dev_pm_ops)

#else

#define BALONG_DEV_PM_OPS NULL

#endif


static struct gpio_keys_platform_data *
gpio_keys_get_devtree_pdata(struct platform_device *pdev)
{
    struct device_node *node, *pp;
    struct gpio_keys_platform_data *pdata;
    struct gpio_keys_button *button;
    int error;
    int nbuttons;
    int i;    
    int gpio = 0;
    int active_low = 0;
    unsigned int rep = 0;

    node = pdev->dev.of_node;
    if (!node) {
        error = -ENODEV;
        goto err_out;
    }

    nbuttons = of_get_child_count(node);
    if (nbuttons == 0) {
        error = -ENODEV;
        goto err_out;
    }

    pdata = kzalloc(sizeof(*pdata) + nbuttons * (sizeof *button),
            GFP_KERNEL);
    if (!pdata) {
        error = -ENOMEM;
        goto err_out;
    }

    pdata->buttons = (struct gpio_keys_button *)(pdata + 1);
    pdata->nbuttons = nbuttons;

    error = of_property_read_u32(node, "autorepeat", &rep);
    if(error)
         dev_err(&pdev->dev,"Failed to get autorepeat error: %d\n",error);
    pdata->rep = !!rep;

    i = 0;
    for_each_child_of_node(node, pp) {

        if (of_property_read_u32(pp, "gpio-num", &gpio)) {
            pdata->nbuttons--;
            dev_warn(&pdev->dev, "Found button without gpios\n");
            continue;
        }

        if (of_property_read_u32(pp, "active-low", &active_low)) {
            error = gpio;
            if (error != -EPROBE_DEFER)
                dev_err(&pdev->dev,
                    "Failed to get gpio flags, error: %d\n",
                    error);
            goto err_free_pdata;
        }

        button = &pdata->buttons[i++];

        button->gpio = gpio;
        button->active_low = active_low;

        if (of_property_read_u32(pp, "linux,code", &button->code)) {
            dev_err(&pdev->dev, "Button without keycode: 0x%x\n",
                button->gpio);
            error = -EINVAL;
            goto err_free_pdata;
        }

        button->desc = of_get_property(pp, "label", NULL);

        if (of_property_read_u32(pp, "linux,input-type", &button->type))
            button->type = EV_KEY;

        button->wakeup = !!of_get_property(pp, "gpio-key,wakeup", NULL);

        if (of_property_read_u32(pp, "debounce-interval",
                     &button->debounce_interval))
            button->debounce_interval = 20;
    }

    if (pdata->nbuttons == 0) {
        error = -EINVAL;
        goto err_free_pdata;
    }

    return pdata;

err_free_pdata:
    kfree(pdata);
err_out:
    return ERR_PTR(error);
}

static struct of_device_id gpio_keys_of_match[] = {
    { .compatible = "balong_gpio_key", },
    { },
};
MODULE_DEVICE_TABLE(of, gpio_keys_of_match);


static int __init balong_gpio_key_probe(struct platform_device* pdev)
{
    struct balong_gpio_key *gpio_key = NULL;
    struct input_dev *input = NULL;
    struct gpio_keys_platform_data *pdata = NULL;
    int err =0;
    int i = 0;
    
    key_print_info("balong key driver probes start!\n");
    
    if (NULL == pdev) {
        key_print_error("parameter error!\n");
        err = -EINVAL;
        return err;
    }

    //pdata = pdev->dev.platform_data; /*获取key平台数据*/
    pdata = gpio_keys_get_devtree_pdata(pdev);
    if(NULL == pdata){
        dev_err(&pdev->dev,"Failed to get no platform data!\n");
        return -EINVAL;
    }

    gpio_key = kzalloc(sizeof(struct balong_gpio_key) +
            (unsigned int)(pdata->nbuttons) * sizeof(struct gpio_button_data), GFP_KERNEL);
    if (!gpio_key) {/*内存申请的大小打印*/
        dev_err(&pdev->dev, "Failed to allocate struct balong_gpio_key!\n");
        err = -ENOMEM;
        return err;
    }
    
    g_suspend_gpio_key = gpio_key;

    gpio_key->suspended = 0;

    gpio_key->event_type = BUTT;
#if ( FEATURE_ON == MBB_KEY )
    gpio_key->n_buttons = pdata->nbuttons; /* 初始化GPIO按键个数 */
#endif
    input = input_allocate_device();
    if (!input) {
        dev_err(&pdev->dev, "Failed to allocate struct input_dev!\n");
        err = -ENOMEM;
        goto err_alloc_input_device;
    }

    input->name = pdev->name;
    input->id.bustype = BUS_HOST;
    input->dev.parent = &pdev->dev;
    input_set_drvdata(input, gpio_key);
    set_bit(EV_KEY, input->evbit);
    set_bit(EV_SYN, input->evbit);
    set_bit(KEY_MENU, input->keybit);
    set_bit(KEY_F24, input->keybit);
    set_bit(KEY_POWER, input->keybit);
    set_bit(KEY_WLAN, input->keybit);
    set_bit(KEY_F23, input->keybit);
    
    input->open = balong_gpio_key_open;
    input->close = balong_gpio_key_close;

    gpio_key->input = input;
    wake_lock_init(&(gpio_key->key_wake_lock), WAKE_LOCK_SUSPEND, "balong KEY");

    for (i = 0; i < pdata->nbuttons; i++) {
        const struct gpio_keys_button *button = &pdata->buttons[i];
        struct gpio_button_data *bdata = &gpio_key->data[i];
        bdata->key_wake_lock = &(gpio_key->key_wake_lock);
#if ( FEATURE_ON == MBB_KEY )
        bdata->event_type = BUTT;
        bdata->is_suspend = 0;
#endif
        err = gpio_keys_setup_key(pdev, input, bdata, button);
        if (err)
            goto err_gpio_key;
    }

    err = input_register_device(gpio_key->input);
    if (err) {
        dev_err(&pdev->dev, "Failed to register input device!\n");
        goto err_register_device;
    }

    /* get current state of buttons that are connected to GPIOs */
    for (i = 0; i < pdata->nbuttons; i++) {
        struct gpio_button_data *bdata = &gpio_key->data[i];
        if (gpio_is_valid(bdata->button->gpio))
            gpio_keys_gpio_report_event(bdata);
    }


#if (FEATURE_ON == MBB_POWER_KEY)
    /*挂载PMU中断处理函数*/
    if(0 != bsp_pmu_irq_callback_register(PMU_INT_POWER_KEY_20MS_PRESS, report_power_key_down, gpio_key))
    {
        dev_err(&pdev->dev, "Failed to register pmu down irq!\n");
        goto err_register_device;
    }

    if(0 != bsp_pmu_irq_callback_register(PMU_INT_POWER_KEY_20MS_RELEASE, report_power_key_up, gpio_key))
    {
        dev_err(&pdev->dev, "Failed to register pmu up irq!\n");
        goto err_register_device;
    }
    
    /*power键初始状态上报Input事件*/
    if(bsp_pmu_key_state_get())
    {
        report_power_key_down(gpio_key);
    }
    else
    {
        report_power_key_up(gpio_key);
    }
#endif
    
    device_init_wakeup(&pdev->dev, (bool)1);
    platform_set_drvdata(pdev, gpio_key);


    key_print_info("balong gpio key driver probes end!\n");

    return 0;

err_register_device:
    input_free_device(input);

err_gpio_key:
    while (--i >= 0)
        gpio_remove_key(&gpio_key->data[i]);

err_alloc_input_device:
    kfree(gpio_key);
    gpio_key = NULL;
    key_print_error("balong gpio key probe failed! ret = %d.\n", err);
    return err;
}
/*lint -restore*/

static int __init balong_gpio_key_remove(struct platform_device* pdev)
{
    unsigned int i = 0;
    struct input_dev *input = NULL;
    
    struct balong_gpio_key *gpio_key = platform_get_drvdata(pdev);
    if(NULL == gpio_key)
    {
        key_print_error("platform_get_drvdata is fail.\n");
        return -1;
    }
    
    input = gpio_key->input;

    device_init_wakeup(&pdev->dev, (bool)0);

    for (i = 0; i < gpio_key->n_buttons; i++)
        gpio_remove_key(&gpio_key->data[i]);

    input_unregister_device(input);

    /*
     * If we had no platform_data, we allocated buttons dynamically, and
     * must free them here. ddata->data[0].button is the pointer to the
     * beginning of the allocated array.
     */
    if (!pdev->dev.platform_data)
        kfree(gpio_key->data[0].button);

    kfree(gpio_key);

    return 0;
}
static struct platform_driver balong_gpio_key_driver = {
    .probe        = balong_gpio_key_probe,
    .remove        = balong_gpio_key_remove,
    .driver        = {
        .name    = "balong_gpio_key",
        .owner    = THIS_MODULE,
        .pm    = BALONG_DEV_PM_OPS,
        .of_match_table = of_match_ptr(gpio_keys_of_match),
    }
};

static int __init balong_gpio_key_init(void)
{
    int ret = 0;
    key_print_info("balong gpio key init!\n");
  
    ret = platform_driver_register(&balong_gpio_key_driver);
    if (ret < 0)
    {
        key_print_info("platform_driver_register failed!\n");
        return ret;            
    }

#if (FEATURE_ON == MBB_FAST_ON_OFF)
	ret = blocking_notifier_chain_register(&g_fast_on_off_notifier_list, &key_fastonoff_notifier);
	if(0 > ret)
	{
	    key_print_info( "failed to register blocking_notifier_chain!\n");
		platform_driver_unregister(&balong_gpio_key_driver);
        return ret;
	}
#endif

}

static void __exit balong_gpio_key_exit(void)
{
    key_print_info("balong gpio key exit!\n");
    
    platform_driver_unregister(&balong_gpio_key_driver);
	
#if (FEATURE_ON == MBB_FAST_ON_OFF)
	blocking_notifier_chain_unregister(&g_fast_on_off_notifier_list, &key_fastonoff_notifier);
#endif

}

module_init(balong_gpio_key_init);
module_exit(balong_gpio_key_exit);
MODULE_AUTHOR("Hisilicon balong Driver Group");
MODULE_DESCRIPTION("balong keypad platform driver");
MODULE_LICENSE("GPL");

