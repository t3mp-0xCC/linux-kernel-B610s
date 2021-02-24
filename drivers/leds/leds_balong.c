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


/*lint --e{527,529,533,537,752}*/
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <hi_dr.h>
#include <mdrv_leds.h>
#include "bsp_leds.h"
#include "bsp_regulator.h"
#include "bsp_dr.h"
#include "bsp_nvim.h"
#include "bsp_icc.h"
#include "bsp_softtimer.h"
#include "leds_balong.h"

#include <product_config.h>

#include <linux/gpio.h>


static DEFINE_MUTEX(g_led_kernelset_lock);    /*led内核控制接口自旋锁*/
#define long_time    50000    /*dr常亮寄存器值*/
#define current    3000    /*单位uA，电流设为3mA*/

extern int g_dr_brightness[5];     /*当前的亮度标识*/
struct regulator *dr_regulator[5];    /*定义regulator全局变量*/

static int g_set_flag = 0;    /*接口调用标识位*/
static int g_ledready_flag = 0;    /*节点就绪标识位*/

/*lint -e553*/
#define en_NV_Item_HUAWEI_PCCW_HS_HSPA_BLUE 50032
static int g_leds_balong_init = 0; /* 当前驱动是否初始化完毕 */
/*lint +e553*/
struct softtimer_list led_softtimer;                /* soft timer */
struct nv_led g_nv_led;                             /* store nv */
DRV_MODULE_TEST_STRU g_nv_module_ctrl;
LED_CONTROL_NV_STRU g_led_state_str_lte[LED_LIGHT_STATE_MAX][LED_CONFIG_MAX_LTE] = LED_OM_CONFIG; /* 不同的状态对应的闪灯方案，功能与led nv一样 */

/*lint -e553*/
LED_CONTROL_NV_STRU g_led_HongKong_StateStr[LED_LIGHT_STATE_MAX][LED_CONFIG_MAX_LTE] = LED_HONGKONG_CONFIG;
LED_CONTROL_NV_STRU g_led_Russia_StateStr[LED_LIGHT_STATE_MAX][LED_CONFIG_MAX_LTE] = LED_RUSSIA_CONFIG;
LED_CONTROL_NV_STRU g_led_Vodafone_StateStr[LED_LIGHT_STATE_MAX][LED_CONFIG_MAX_LTE] = LED_VODAFONE_CONFIG;
LED_CONTROL_NV_STRU g_led_ChinaUnion_StateStr[LED_LIGHT_STATE_MAX][LED_CONFIG_MAX_LTE] = LED_CHINAUNION_CONFIG;
LED_CONTROL_NV_STRU g_led_FranceSFR_StateStr[LED_LIGHT_STATE_MAX][LED_CONFIG_MAX_LTE] = LED_FRANCE_CONFIG;
LED_CONTROL_NV_STRU g_led_IndiaBSNL_StateStr[LED_LIGHT_STATE_MAX][LED_CONFIG_MAX_LTE] = LED_INDIA_CONFIG;
LED_CONTROL_NV_STRU g_led_HolandKPN_StateStr[LED_LIGHT_STATE_MAX][LED_CONFIG_MAX_LTE] = LED_HOLAND_CONFIG;
LED_CONTROL_NV_STRU g_led_SoftBank_StateStr[LED_LIGHT_STATE_MAX][LED_CONFIG_MAX_LTE] = LED_OM_CONFIG;
LED_CONTROL_NV_STRU g_led_Emobile_StateStr[LED_LIGHT_STATE_MAX][LED_CONFIG_MAX_LTE] = LED_OM_CONFIG;


/*lint +e553*/
unsigned int led_debug_level;                       /* for LED_TRACE level */

extern struct led_tled_arg g_arg;

EXPORT_SYMBOL_GPL(g_led_state_str_lte);
EXPORT_SYMBOL_GPL(g_nv_led);
EXPORT_SYMBOL_GPL(led_debug_level);

static int balong_led_probe(struct platform_device *dev);
//static int balong_led_remove(struct platform_device *pdev);
int register_led_by_nr(struct balong_led_device *led, int nr, struct led_platform_data *pdata);

/* LED resources */
struct led_info balong_led[] = {
    [0] = {
        .name = LED1_NAME,
    },
    [1] = {
        .name = LED2_NAME,
    },
    [2] = {
        .name = LED3_NAME,
    },
    [3] = {
        .name = LED4_NAME,
    },
    [4] = {
        .name = LED5_NAME,
    }
};

/* LED platform data */
 struct led_platform_data led_plat_data = {
    .num_leds   = ARRAY_SIZE(balong_led),
    .leds       = (struct led_info *)balong_led,
};

/* LED platform device */
 struct platform_device balong_led_dev = {
    .name	= "balong_led",
	.id	    = -1,
	.dev 	= {
		.platform_data	= &led_plat_data,
	}
};
EXPORT_SYMBOL_GPL(balong_led_dev);

unsigned long us2ms(unsigned long us)
{
    return us / 1000;
}

/*lint -save -e413 -e613*/
/************************************************************************
 * Name         : brightness_to_limit
 * Function     : brightness to limit
 * Arguments
 *      input   : @led_id         - led id
 *              : @led_brightness - brightness
 *      output  : null
 *
 * Return       : limit
 * Decription   : 把最大电流到最小电流划分为(LED_FULL + 1)个间隔，据此将led_brightness转换为limit,
 *                再根据DR可接受的8个电流档位，选择最接近的一个档位。
 ************************************************************************/
 int brightness_to_limit(int led_id, int led_brightness)
{
    unsigned i;
    int limit, limit_min, limit_max, limit_bigger, limit_smaller;

    /* compute limit */
    if(LED_OFF == led_brightness)
    {
        return 0;
    }
    else
    {
        limit_min = bsp_dr_list_current(led_id, 0);
        limit_max = bsp_dr_list_current(led_id, DR_CUR_NUMS - 1);

        limit = led_brightness * limit_max / LED_FULL; //DEBUG
        LED_TRACE(LED_DEBUG_LEVEL(INFO),"[%s] led id %d, brightness %d, limit min %d, limit max %d, limit %d\n",
            __FUNCTION__, led_id, led_brightness, limit_min,limit_max, limit);
    }

    /* find nearest limit level to use */
    for(i = 0; i < DR_CUR_NUMS; i++)
    {
        limit_bigger = bsp_dr_list_current(led_id, i);
        if(limit < limit_bigger)
        {
            break;
        }
    }
    limit_smaller = (0 == i) ? 0 : bsp_dr_list_current(led_id, i - 1);
    LED_TRACE(LED_DEBUG_LEVEL(INFO),"[%s] limit smaller %d, limit bigger %d\n", __FUNCTION__, limit_smaller, limit_bigger);

    /* find nearest limit level, if limit is just in the middle of limit_bigger and limit_smaller, chose limit_bigger */
    limit = ((limit - limit_smaller) < (limit_bigger - limit)) ? limit_smaller : limit_bigger;
    LED_TRACE(LED_DEBUG_LEVEL(INFO),"[%s] limit level %d\n", __FUNCTION__, limit);

    return limit;
}


 int name_to_id(struct led_classdev *led_cdev)
{
    /* reject null */
    if(!led_cdev || !led_cdev->name)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] arguments is NULL\n", __FUNCTION__);
        return LED_ERROR;
    }

    LED_TRACE(LED_DEBUG_LEVEL(INFO),"[%s] led name %s\n", __FUNCTION__, led_cdev->name);

    if(!strcmp(led_cdev->name, LED1_NAME))
    {
        return PMIC_DR01;
    }
    else if(!strcmp(led_cdev->name, LED2_NAME))
    {
        return PMIC_DR02;
    }
    else if(!strcmp(led_cdev->name, LED3_NAME))
    {
        return PMIC_DR03;
    }
    else if(!strcmp(led_cdev->name, LED4_NAME))
    {
        return PMIC_DR04;
    }
    else
    {
        return PMIC_DR05;
    }
}

/************************************************************************
 * Name         : set_bre_time_valid
 * Function     : set breath time aviled
 * Arguments
 *      input   : @led_id   - led id
 *                @bre_time_enum - 要设置的参数类型
 *                @p_breathtime_ms - 要设置的参数值
 *      output  : @p_breathtime_ms - 要设置的参数值
 *
 * Return       : default breath on time(ms)
 * Decription   : 默认值此处参考Hi6551用户手册对应寄存器的默认值
 ************************************************************************/
int set_bre_time_valid(int led_id, dr_bre_time_e bre_time_enum, unsigned long *p_breathtime_ms)
{
    unsigned long value_num, selector;
    unsigned long temp_bigger = 0, temp_smaller, breath_time;
    int ret = LED_ERROR;

    /* 检查参数 */
    if(!p_breathtime_ms)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] arguments is NULL\n", __FUNCTION__);
        goto ERRO;
    }

    value_num = bsp_dr_bre_time_selectors_get(led_id, bre_time_enum);
    if(BSP_DR_PARA_ERROR == value_num)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]%d get %d value num failed\n", __FUNCTION__, led_id, bre_time_enum);
        goto ERRO;
    }

    /* 选择可用的档位值 */
    breath_time = *p_breathtime_ms;
    for(selector = 0; selector < value_num; selector++)
    {
        temp_bigger = bsp_dr_bre_time_list(led_id, bre_time_enum, selector);
        if(BSP_DR_PARA_ERROR == temp_bigger)
        {
            LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]%d get breath time failed\n", __FUNCTION__, led_id);
            goto ERRO;
        }
        if(temp_bigger >= breath_time)
        {
            LED_TRACE(LED_DEBUG_LEVEL(INFO),"[%s]%d bigger breath time %d\n", __FUNCTION__, led_id, temp_bigger);
            break;
        }
    }

    if(value_num == selector)   /* 传入的值超过最大档位，就使用最大档位 */
    {
        LED_TRACE(LED_DEBUG_LEVEL(WARNING),"[%s]%d breath time is larger than max, breath_time %d, max value %d\n",
            __FUNCTION__, led_id, breath_time, temp_bigger);
        *p_breathtime_ms = temp_bigger;
    }
    else if(0 == selector)      /* 传入的值小于最小档位，就使用最小档位 */
    {
        LED_TRACE(LED_DEBUG_LEVEL(WARNING),"[%s]%d breath time is samller than min, breath_time %d, min value %d\n",
            __FUNCTION__, led_id, breath_time, temp_bigger);
        *p_breathtime_ms = temp_bigger;
    }
    else                        /* 传入的值介于最大档位和最小档位之间，选择最近的档位。如果恰位于两档正中间，使用较小档位 */
    {
        temp_smaller = bsp_dr_bre_time_list(led_id, bre_time_enum, selector - 1);
        if(BSP_DR_PARA_ERROR == temp_smaller)
        {
            LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]%d get breath time failed\n", __FUNCTION__, led_id);
            goto ERRO;
        }
        *p_breathtime_ms = ((temp_bigger - breath_time) > (breath_time - temp_smaller) ? temp_smaller : temp_bigger);
        LED_TRACE(LED_DEBUG_LEVEL(INFO),"[%s]%d breath time bigger %d, smaller %d\n",
            __FUNCTION__, led_id, temp_bigger, temp_smaller);
        LED_TRACE(LED_DEBUG_LEVEL(INFO),"[%s]%d breath time %d, choice %d\n",
            __FUNCTION__, led_id, breath_time, *p_breathtime_ms);
    }

    return LED_OK;
ERRO:
    return ret;
}

/************************************************************************
 * Name         : set_bre_mode
 * Function     : set breath mode
 * Arguments
 *      input   : @led_id   - led id
 *
 * Return       : 0 - success; else - failed
 * Decription   : DR1/2和DR3/4/5的呼吸模式配置不一样
 ************************************************************************/
int set_bre_mode(int led_id)
{
    int ret = LED_ERROR;
    
    switch(led_id)
    {        
        case PMIC_DR01:
        case PMIC_DR02:
            ret = bsp_dr_set_mode(led_id, PMU_DRS_MODE_BRE_FLASH);
            break;
        case PMIC_DR03:
        case PMIC_DR04:
        case PMIC_DR05:
            ret = bsp_dr_set_mode(led_id, PMU_DRS_MODE_BRE);
            break;
        default:
            break;
    };
    return ret;
}
/************************************************************************
 * Name         : balong_brightness_set
 * Function     : set brightness
 * Arguments
 *      input   : @led_cdev - which led
 *                @brightness - brightness, should between 0~255
 *      output  : null
 *
 * Return       : null
 * Decription   : 这个函数不能阻塞，可考虑使用任务队列
 ************************************************************************/
void balong_led_brightness_set(struct led_classdev *led_cdev, enum led_brightness brightness)
{
    struct balong_led_device *led_dev = NULL;
    int led_id, limit, ret;

    if(0 == g_nv_module_ctrl.leds)
        return;
    
    led_dev = container_of(led_cdev, struct balong_led_device, cdev);
    
    /* argument check */
    if((!led_cdev) || (!led_dev) || (!led_dev->pdata))
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] arguments is NULL\n", __FUNCTION__);
        return;
    }

    LED_TRACE(LED_DEBUG_LEVEL(INFO),"[%s] %s, brightness %d\n", __FUNCTION__, led_cdev->name, brightness);

    mutex_lock(&led_dev->pdata->mlock);

    /* brightness can't be too big or too small */
    if(brightness >= LED_FULL)
    {
        brightness = LED_FULL;
    }

    /* get id of led */
    led_id = name_to_id(led_cdev);

    /* save brightness */
    led_cdev->brightness = brightness;

    /* set limit */
    limit = brightness_to_limit(led_id, brightness);
    if(0 == limit)
    {
        /* if enabled, disable */
        if(regulator_is_enabled(led_dev->pdata->pregulator))
        {
            ret = regulator_disable(led_dev->pdata->pregulator);
            if(ret)
            {
                LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] %s regulator_disable failed, ret = %d\n",
                    __FUNCTION__, led_cdev->name, ret);
                goto EXIT;
            }
        }
        goto EXIT;
    }
    regulator_set_current_limit(led_dev->pdata->pregulator, limit, limit);

    LED_TRACE(LED_DEBUG_LEVEL(INFO),"[%s] %s set brightness %d limit %d\n",
        __FUNCTION__, led_cdev->name, brightness, limit);

    /* enable led */
    if(!regulator_is_enabled(led_dev->pdata->pregulator))
    {
        ret = regulator_enable(led_dev->pdata->pregulator);
        if(ret)
        {
            LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] %s regulator_enable failed, ret = %d\n",
                __FUNCTION__, led_cdev->name, ret);
            goto EXIT;
        }
    }

EXIT:
    mutex_unlock(&led_dev->pdata->mlock);
    return;
}

/************************************************************************
 * Name         : brightness_get
 * Function     : get brightness
 * Arguments
 *      input   : @led_cdev - which led
 *      output  : null
 *
 * Return       : brightness
 * Decription   : null
 ************************************************************************/
/*lint -save -e82, -e110, -e533*/
enum led_brightness balong_led_brightness_get(struct led_classdev *led_cdev)
{/*lint !e64*/

    if(0 == g_nv_module_ctrl.leds)
        return (enum led_brightness)LED_OK;

    /* argument check */
    if(!led_cdev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] arguments is NULL\n", __FUNCTION__);
        return (enum led_brightness)LED_ERROR;
    }

    return (enum led_brightness)led_cdev->brightness;
}
/*lint -restore*/
/************************************************************************
 * Name         : balong_blink_set
 * Function     : set blink
 * Arguments
 *      input   : @led_cdev - which led
 *              : @delay_on - delay time when led is on
 *              : @delay_off - delay time when led is off
 *      output  : null
 *
 * Return       : 0 - success
 *              : else - error
 * Decription   : null
 ************************************************************************/
int balong_led_blink_set(struct led_classdev *led_cdev, unsigned long *delay_on, unsigned long *delay_off)
{
    int led_id, ret;
    unsigned long local_delayon, local_delayoff;
    DR_FLA_TIME dr_fla_time;
    struct balong_led_device *led_dev = NULL;

    if(0 == g_nv_module_ctrl.leds)
        return LED_OK;

    led_dev = container_of(led_cdev, struct balong_led_device, cdev);

    /* argument check */
    if((!led_cdev) || (!led_dev) || (!led_dev->pdata))
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] arguments is NULL\n", __FUNCTION__);
        return LED_ERROR;
    }

    mutex_lock(&led_dev->pdata->mlock);

    /* get id of led */
    led_id = name_to_id(led_cdev);

    /* if null, set LED_VALUE_INVALID */
    if(!delay_on)
    {
        local_delayon = LED_VALUE_INVALID;
        delay_on = &local_delayon;
    }

    if(!delay_off)
    {
        local_delayoff = LED_VALUE_INVALID;
        delay_off = &local_delayoff;
    }
    
    LED_TRACE(LED_DEBUG_LEVEL(INFO),"[%s]%s blink feriod %d, blink on %d\n", 
        __FUNCTION__, led_cdev->name, *delay_on + *delay_off, *delay_on);

    /* set blink */
    dr_fla_time.fla_off_us = *delay_off;
    dr_fla_time.fla_on_us = *delay_on;
    ret = bsp_dr_fla_time_set(led_id, &dr_fla_time);
    if(ret)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]%s ERROR: set flash time failed, ret = %d.\n", 
            __FUNCTION__, led_cdev->name, ret);
        goto EXIT;
    }
    
    /* set blink mode */
    ret = bsp_dr_set_mode(led_id, PMU_DRS_MODE_FLA_FLASH);      
    if(ret)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]%s ERROR: set flash mode failed, ret = %d.\n", 
            __FUNCTION__, led_cdev->name, ret);
        goto EXIT;
    }    

    mutex_unlock(&led_dev->pdata->mlock);
    return LED_OK;
    
EXIT:
    mutex_unlock(&led_dev->pdata->mlock);
    return LED_ERROR;
}

/************************************************************************
 * Name         : balong_led_breath_set
 * Function     : set
 * Arguments
 *      input   : @led_cdev - which led
 *              : @full_on - delay time(us) when led is on
 *              : @full_off - delay time(us) when led is off
 *              : @fade_on - delay time(us) when led is breathing from off to on
 *              : @fade_off - delay time(us) when led is breathing form on to off
 *      output  : null
 *
 * Return       : 0 - success
 *              : else - error
 * Decription   : null
 ************************************************************************/
int balong_led_breath_set(struct led_classdev *led_cdev, unsigned long *full_on, unsigned long *full_off, unsigned long *fade_on, unsigned long *fade_off)
{/*lint !e64*/
    DR_BRE_TIME dr_bre_time;
    int led_id, ret;
    unsigned long local_fullon, local_fulloff, local_breon, local_breoff;
    struct balong_led_device *led_dev = NULL;

    if(0 == g_nv_module_ctrl.leds)
        return LED_OK;

    led_dev = container_of(led_cdev, struct balong_led_device, cdev);
    
    /* argument check */
    if((!led_cdev) || (!led_dev) || (!led_dev->pdata))
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] arguments is NULL.\n", __FUNCTION__);
        return LED_ERROR;
    }

    mutex_lock(&led_dev->pdata->mlock);

    /* get id of led */
    led_id = name_to_id(led_cdev);

    /* if null, set default value */
    if(!full_on)
    {
        local_fullon = LED_VALUE_INVALID;
        full_on = &local_fullon;
    }
    else
    {
        local_fullon = us2ms(*full_on);
        ret = set_bre_time_valid(led_id, PMU_DRS_BRE_ON_MS, &local_fullon);
        if(ret)
        {
            LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]%s ERROR: set breath time valid failed, ret = %d.\n", 
                __FUNCTION__, led_cdev->name, ret);
            goto EXIT;
        }
    }

    if(!full_off)
    {
        local_fulloff = LED_VALUE_INVALID;
        full_off = &local_fulloff;
    }
    else
    {
        local_fulloff = us2ms(*full_off);
        ret = set_bre_time_valid(led_id, PMU_DRS_BRE_OFF_MS, &local_fulloff);
        if(ret)
        {
            LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]%s ERROR: set breath time valid failed, ret = %d.\n", 
                __FUNCTION__, led_cdev->name, ret);
            goto EXIT;
        }
    }
    
    if(!fade_on)
    {
        local_breon = LED_VALUE_INVALID;
        fade_on = &local_breon;
    }
    else
    {
        local_breon = us2ms(*fade_on);
        ret = set_bre_time_valid(led_id, PMU_DRS_BRE_RISE_MS, &local_breon);
        if(ret)
        {
            LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]%s ERROR: set breath time valid failed, ret = %d.\n", 
                __FUNCTION__, led_cdev->name, ret);
            goto EXIT;
        }
    }
    
    if(!fade_off)
    {
        local_breoff = LED_VALUE_INVALID;
        fade_off = &local_breoff;
    }
    else
    {
        local_breoff = us2ms(*fade_off);
        ret = set_bre_time_valid(led_id, PMU_DRS_BRE_FALL_MS, &local_breoff);
        if(ret)
        {
            LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]%s ERROR: set breath time valid failed, ret = %d.\n", 
                __FUNCTION__, led_cdev->name, ret);
            goto EXIT;
        }
    }

    /* set time */
    dr_bre_time.bre_fall_ms = local_breoff;
    dr_bre_time.bre_off_ms = local_fulloff;
    dr_bre_time.bre_on_ms = local_fullon;
    dr_bre_time.bre_rise_ms = local_breon;
    
    LED_TRACE(LED_DEBUG_LEVEL(INFO),"[%s]%d full_on %d, full_off %d, fade_on %d, fade_off %d\n",
        __FUNCTION__, led_id, dr_bre_time.bre_on_ms, dr_bre_time.bre_off_ms, dr_bre_time.bre_rise_ms, dr_bre_time.bre_fall_ms);
    
    ret = bsp_dr_bre_time_set(led_id, &dr_bre_time);
    if(ret)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]%s ERROR: set breath time failed, ret = %d.\n", 
            __FUNCTION__, led_cdev->name, ret);
        goto EXIT;
    }

    /* set breath mode */
    ret = set_bre_mode(led_id);
    if(ret)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]%s ERROR: set breath mode, ret = %d.\n", 
            __FUNCTION__, led_cdev->name, ret);
        goto EXIT;
    }

    mutex_unlock(&led_dev->pdata->mlock);
    return LED_OK;
    
EXIT:
    mutex_unlock(&led_dev->pdata->mlock);
    return LED_ERROR;

}
/************************************************************************
 * Name         : register_led_by_nr
 * Function     : register led by number
 * Arguments
 *      input   : nr - LED id, from 0 to 4
 *      output  : null
 *
 * Return       : 0 - success
 *              : else - error
 * Decription   : null
 ************************************************************************/
int register_led_by_nr(struct balong_led_device *led, int nr, struct led_platform_data *pdata)
{/*lint !e64*/
    /*lint -save -e409*/ 
    led[nr].pdata->led_breath_set      = balong_led_breath_set;
	led[nr].cdev.brightness_set        = balong_led_brightness_set;
    led[nr].cdev.brightness_get        = balong_led_brightness_get;
    led[nr].cdev.blink_set             = balong_led_blink_set;
	led[nr].cdev.name                  = pdata->leds[nr].name;

    /* regist regulator device */
    led[nr].pdata->pregulator = regulator_get(NULL, led[nr].cdev.name);

    dr_regulator[nr] = led[nr].pdata->pregulator;    /*赋值regulator全局变量*/
    
    if (IS_ERR(led[nr].pdata->pregulator))
    {
        pr_err("[%s] led%d get regulator failed\n", __FUNCTION__, nr + 1);
        return LED_ERROR;
    }

    /*lint -save -e539*/ 
	led[nr].cdev.default_trigger       = pdata->leds[nr].default_trigger;
	led[nr].cdev.flags                 = 0;
    /*lint -restore*/  
    
    /* mutex lock init */
    mutex_init(&led[nr].pdata->mlock);
    return LED_OK;
    /*lint -restore*/  
}

/*lint -e534 -e516*/


/************************************************************************
 * Name         : balong_led_probe
 * Function     : probe when init
 * Arguments
 *      input   : @pdev - platform device
 *      output  : null
 *
 * Return       : 0 - success
 *              : else - error
 * Decription   : null
 ************************************************************************/
static int balong_led_probe(struct platform_device *dev)
{
    struct led_platform_data *pdata = dev->dev.platform_data;
	struct balong_led_device *led   = NULL;
	int ret = LED_OK;
    unsigned int i;

    ret = (s32)bsp_nvm_read(NV_ID_DRV_TEST_SUPPORT, (u8 *)&g_nv_module_ctrl, sizeof(DRV_MODULE_TEST_STRU));
    if(ret){
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"error: read nv failed, ret = %d, enable default\n", ret);
        g_nv_module_ctrl.leds = 1;
    }

    if(0 == g_nv_module_ctrl.leds)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] leds stub\n", __FUNCTION__);
        return LED_OK;
    }

	led = (struct balong_led_device *)kzalloc(sizeof(struct balong_led_device) * ARRAY_SIZE(balong_led), GFP_KERNEL);
    if(!led)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] get buffer failed\n", __FUNCTION__);
        goto ERRO;
    }    

    /* get pdata, register led and register classdev for every led device */
    for(i = (unsigned int)PMU_DR_MIN; i <= (unsigned int)PMU_DR_MAX; i++)
    {
        led[i].pdata = (struct balong_led_platdata *)kzalloc(sizeof(struct balong_led_platdata), GFP_KERNEL);
        if(!led[i].pdata)
        {
            LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] get pdata buffer failed\n", __FUNCTION__);
            goto ERRO;
        }

        /* regulator register failed is possible, as not all DRs are for LED */
        ret = register_led_by_nr(led, (int)i, pdata);
        if(!ret)
        {
        	ret = led_classdev_register(&dev->dev, &(led[i].cdev));
            if(ret)
            {
                LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]led%d sysfs register failed\n", __FUNCTION__, i + 1);
            }
        }

        /* coverity[unsigned_compare] */
        if((unsigned int)PMU_TLED_DR_MIN <= i)
        {
            /* set start delay */
            ret = bsp_dr_start_delay_set(i, 0);
            if(ret)
            {
                LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]set led%d start delay ERROR, ret = %d.\n", 
                    __FUNCTION__, i + 1, ret);
                goto ERRO;
            }        
        }
    }

    /* save "led" as private data */
    /*lint -save -e539*/ 
	platform_set_drvdata(dev, led);
    /*lint -restore*/ 

    /* timer create for three-color-led */
	led_softtimer.func = (softtimer_func)do_led_threecolor_flush;
	led_softtimer.para = 0;                              
	led_softtimer.timeout = 0;                        /* 定时长度，单位ms */
	led_softtimer.wake_type = SOFTTIMER_WAKE;
    
	if (bsp_softtimer_create(&led_softtimer))
	{
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s]ERROR: softtimer create failed\n", __FUNCTION__);
		goto ERRO;
	}  
    
    /* read NV and store for three-color-led*/    
    if(g_nv_led.g_already_read != LED_NV_ALREADY_READ)
    {
    	ret = (int)bsp_nvm_read(NV_ID_DRV_LED_CONTROL,(u8 *)g_nv_led.g_led_state_str_om, LED_LIGHT_STATE_MAX * LED_CONFIG_MAX_LTE * sizeof(LED_CONTROL_NV_STRU));
    	if(NV_OK != ret)
    	{
    		LED_TRACE(LED_DEBUG_LEVEL(ERROR), "[%s]WARNING: read nv failed, use default, ret = %d!\n", __FUNCTION__, ret);

            /* 读nv失败，采用默认值 */
            memcpy(g_nv_led.g_led_state_str_om, g_led_state_str_lte, 
                LED_LIGHT_STATE_MAX * LED_CONFIG_MAX_LTE * sizeof(LED_CONTROL_NV_STRU));
    	}
    }    

/*lint -e553*/
    /*从NV50032从获取闪灯的方案*/
    unsigned long ulLEDStatus = DEF;
    if(NV_OK != bsp_nvm_read(en_NV_Item_HUAWEI_PCCW_HS_HSPA_BLUE,&ulLEDStatus,sizeof(unsigned long)))
    {
        /*NV读取失败，则使用默认闪灯方案*/
        ulLEDStatus = DEF;
    }

    /*根据NV的取值，使用不同的闪灯配置方案*/
    switch(ulLEDStatus)
    {
        case(DEF):/*默认闪灯*/
        {
            break;
        }

        case(HONGKONG):/*香港PCCW*/
        {
            memcpy(g_nv_led.g_led_state_str_om,g_led_HongKong_StateStr,sizeof(g_led_state_str_lte));
            break;
        }

        case(RUSSIA):/*俄罗斯MTS*/
        {
            memcpy(g_nv_led.g_led_state_str_om,g_led_Russia_StateStr,sizeof(g_led_state_str_lte));
            break;
        }

        case(VODAFONE):/*Vodafone*/
        {
            memcpy(g_nv_led.g_led_state_str_om,g_led_Vodafone_StateStr,sizeof(g_led_state_str_lte));
            break;
        }

        case(CHINAUNION):/*中国联通*/
        {
            memcpy(g_nv_led.g_led_state_str_om,g_led_ChinaUnion_StateStr,sizeof(g_led_state_str_lte));
            break;
        }

        case(SFR):/*法国SFR*/
        {
            memcpy(g_nv_led.g_led_state_str_om,g_led_FranceSFR_StateStr,sizeof(g_led_state_str_lte));
            break;
        }

        case(BSNL):/*印度BSNL*/
        {
            memcpy(g_nv_led.g_led_state_str_om,g_led_IndiaBSNL_StateStr,sizeof(g_led_state_str_lte));
            break;
        }

        case(KPN):/*荷兰KPN*/
        {
            memcpy(g_nv_led.g_led_state_str_om,g_led_HolandKPN_StateStr,sizeof(g_led_state_str_lte));
            break;
        }

        case(SOFTBANK):/*日本软银*/
        {
            memcpy(g_nv_led.g_led_state_str_om,g_led_SoftBank_StateStr,sizeof(g_led_state_str_lte));
            break;
        }

        case(EMOBILE):/*日本EM*/
        {
            memcpy(g_nv_led.g_led_state_str_om,g_led_Emobile_StateStr,sizeof(g_led_state_str_lte));
            break;
        }

        default:
        {
            break;
        }
    }
/*lint +e553*/ 

    /* icc register for three-color-led*/
    ret = bsp_icc_event_register(LED_ICC_CHN_ID, led_threecolor_flush, NULL, NULL, NULL);
	if(ret != LED_OK)
	{
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] register icc event failed, ret = 0x%x\n", __FUNCTION__, ret);
        goto ERRO;
	}
    
    LED_TRACE(LED_DEBUG_LEVEL(INFO),"[%s]: succeed!!!!!register icc callback\n", __FUNCTION__);
    g_leds_balong_init = 1; /* 表示该驱动是否已经初始化 */
    return ret;
    
ERRO:
    for(i = 0; i < ARRAY_SIZE(balong_led); i++)
    {
        if(!led)
        {
            continue;
        }
    
        if(!IS_ERR(led[i].pdata->pregulator) && !(NULL == led[i].pdata->pregulator))
        {
            kfree(led[i].pdata->pregulator);
            led[i].pdata->pregulator = NULL;
        }
        if(!IS_ERR(led[i].pdata) && !(NULL == led[i].pdata))
        {
            kfree(led[i].pdata);
            led[i].pdata = NULL;
        }
    }
    if(led)
    {
        kfree(led);
        led = NULL;
    }
    LED_TRACE(LED_DEBUG_LEVEL(ALWAYS),"LED init failed\n");
    return LED_ERROR; /*lint !e438*/
}

/*****************************************************************************
 函 数 名  : drv_led_flash
 功能描述  : 三色灯设置，给上层提供的接口
 输入参数  : status：三色灯的状态
 输出参数  : 无。
 返 回 值  : 0:  操作成功；
             -1：操作失败。
*****************************************************************************/
int drv_led_flash(unsigned long state) 
{
    int ret = LED_ERROR;
	struct balong_led_device *led = NULL;
    led = (struct balong_led_device *)platform_get_drvdata(&balong_led_dev);
    if(!led)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ALWAYS),"get balong led device failed.\n");
        return LED_ERROR;
    }    

    g_arg.new_state = (unsigned char)state;
    g_arg.ctl = MNTN_LED_STATUS_FLUSH;    

    ret = do_led_threecolor_flush();
    if(ret)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "[%s]three color led flush failed, ret = %d!\n", __FUNCTION__, ret);
        return LED_ERROR;
    }

    return LED_OK;
}

/*************************************************************************/
/* for power manager */

/* LED platform driver */
 struct platform_driver balong_led_drv = {
	.probe		= balong_led_probe,
	//.remove		= balong_led_remove,
	.driver		= {
		.name		= "balong_led",
		.owner		= THIS_MODULE,
	    .bus        = &platform_bus_type,
	},
};

/************************************************************************
 * Name         : led_regulator_dr_enable
 * Function     :
 * Arguments
 *      input   : led_id:0~4
 *      output  : null
 *
 * Return       : 0 - ok
 *                else - fail
 * Decription   : enable dr regulator
 ************************************************************************/
int led_dr_regulator_enable(int led_id)
{
    int ret = LED_ERROR;
    struct balong_led_device *led = NULL;

    led_id = led_id - 1;
    if(0 > led_id || led_id > 4)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] led_id = %d, dr index error.\n", __FUNCTION__, led_id);
        return LED_ERROR;
    }
    
    led = (struct balong_led_device *)platform_get_drvdata(&balong_led_dev);
    if(!led)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ALWAYS),"get balong led device failed.\n");
        return LED_ERROR;
    }

    /* enable led */
    if(regulator_is_enabled(led[led_id].pdata->pregulator))
    {
        ret = regulator_enable(led[led_id].pdata->pregulator);
        if(ret)
        {
            LED_TRACE(LED_DEBUG_LEVEL(ERROR),"[%s] regulator_enable failed, ret = %d\n", __FUNCTION__, ret);
            return LED_ERROR;
        }
    }
    
    return LED_OK;
}
EXPORT_SYMBOL(led_dr_regulator_enable);

/*******************************************************************************
 * FUNC NAME:
 * balong_led_init() - register dirver and device for led driver
 *
 * PARAMETER:
 * none
 *
 * DESCRIPTION:
 * Linux standard driver initialization interface.
 *
 * CALL FUNC:
 *
 ********************************************************************************/
int __init bsp_led_init(void)
{
    int result;

    result = platform_driver_register(&balong_led_drv);
    if (result < 0)
    {
        return result;
    }

    result = platform_device_register(&balong_led_dev);
    if (result < 0)
    {
        platform_driver_unregister(&balong_led_drv);
        return result;
    }

    LED_TRACE(LED_DEBUG_LEVEL(ALWAYS),"LED init OK\n");

    return result;
}
/*****************************************************************************/

module_init(bsp_led_init);   /*lint !e19 */


int dr_led_switch(int numb, int onoff)
{
    int ret = -1;
    DR_BRE_TIME dr_bre_time_st;
    int dr_brightness = 0;
    int dr_current = 0;
    
    if((0 != onoff) && (1 != onoff))
    {
        printk(KERN_ERR "[%s] input onoff = [%d] wrong. \n", __func__, onoff);
        return -1;
    }
    else if(LED_ID_MIN > numb || LED_ID_MAX < numb )
    {
        printk(KERN_ERR "[%s] input numb = [%d] wrong. \n", __func__, onoff);
        return -1;
    }
    
    g_dr_brightness[numb] = -1;        /*亮度值复位*/
    /* 设置呼吸模式 */
    ret = set_bre_mode(numb);
    if(ret)
    {
        printk(KERN_ERR "[%s] ERROR: set breath mode, ret = %d, dr = %d.\n", __FUNCTION__, ret, numb);
        return -1;
    }

    /* 设置呼吸灯时间 此处只设置常亮时间，灭由开关控制*/
    dr_bre_time_st.bre_fall_ms = 0;
    dr_bre_time_st.bre_off_ms = 0;
    dr_bre_time_st.bre_on_ms = (unsigned int)(long_time);
    dr_bre_time_st.bre_rise_ms = 0;

    ret = set_bre_time_valid(numb, PMU_DRS_BRE_FALL_MS, &dr_bre_time_st.bre_fall_ms);
    if(ret)
    {
        printk(KERN_ERR "[%s] ERROR: set bre_fall_ms  valid failed, ret = %d, dr = %d.\n", __FUNCTION__, ret, numb);
        return -1;
    }
    ret = set_bre_time_valid(numb, PMU_DRS_BRE_OFF_MS, &dr_bre_time_st.bre_off_ms);
    if(ret)
    {
        printk(KERN_ERR "[%s] ERROR: set bre_off_ms  valid failed, ret = %d, dr = %d.\n", __FUNCTION__, ret, numb);
        return -1;
    }
    ret = set_bre_time_valid(numb, PMU_DRS_BRE_ON_MS, &dr_bre_time_st.bre_on_ms);
    if(ret)
    {
        printk(KERN_ERR "[%s] ERROR: set bre_on_ms  valid failed, ret = %d, dr = %d.\n", __FUNCTION__, ret, numb);
        return -1;
    }
    ret = set_bre_time_valid(numb, PMU_DRS_BRE_RISE_MS, &dr_bre_time_st.bre_rise_ms);
    if(ret)
    {
        printk(KERN_ERR "[%s] ERROR: set bre_rise_ms  valid failed, ret = %d, dr = %d.\n", __FUNCTION__, ret, numb);
        return -1;
    }

    ret = bsp_dr_bre_time_set((dr_id_e)(numb), &dr_bre_time_st);
    if(ret)
    {
        printk(KERN_ERR "[%s] ERROR: set breath time failed, ret = %d, dr = %d.\n", __FUNCTION__, ret, numb);
        return -1;
    }

    /*关闭*/
    if(0 == onoff)
    {
        /* if enabled, disable */
        if(regulator_is_enabled(dr_regulator[numb]))
        {
            ret = regulator_disable(dr_regulator[numb]);
            if(ret)
            {
                printk(KERN_ERR "[%s] regulator_disable failed, ret = %d, dr = %d.\n", __FUNCTION__, ret, numb);
                return -1;
            }
        }
        
        return 0;
    }
    
    /*设置点亮电流*/
    dr_brightness = dr_led_get_default_brightness(numb);
    
    if(0 > dr_brightness)
    {
        dr_current = current;
    }
    else
    {
        dr_current = brightness_to_limit(numb, dr_brightness);
        if(0 > dr_current)
        {
            dr_current = current;
        }
    }
    
    regulator_set_current_limit(dr_regulator[numb], dr_current, dr_current);

    /* 点亮 */
    if(!regulator_is_enabled(dr_regulator[numb]))
    {
        ret = regulator_enable(dr_regulator[numb]);
        if(ret)
        {
            printk(KERN_ERR "[%s] regulator_enable failed, ret = %d, dr = %d\n", __FUNCTION__, ret, numb);
            return -1;
        }
    }

    return 0;
}


static void gpio_led_switch(int numb, int onoff)
{    
    (void)gpio_direction_output(numb, onoff);   
 
    return;
}

/************************************************************************
 *函数原型 ： int led_kernel_init_status(void)
 *描述     ： 判断df与gpio控制的LED是否初始化成功
 *输入     ： NA
 *输出     ： NA
 *返回值   ： 初始化成功 0；初始化失败-1
*************************************************************************/
int led_kernel_init_status(void)
{
    int ret = -1;
    if(0 == g_leds_balong_init)
    {
        return ret;
    }
    ret = gpio_led_init_status();
    if(ret)
    {
        return ret;
    }

    ret = dr_led_init_status();
    return ret;
}


int led_kernel_status_set(char* led_name, int onoff)
{
    int ret = -1;

    printk(KERN_INFO "[%s] input led_name is %s ,onoff = %d ,g_set_flag = %d. \n", __func__, led_name, onoff,g_set_flag);
    
    if(NULL == led_name)
    {
        printk(KERN_ERR "[%s] input led_name is NULL. \n",  __func__);
        return ret;
    }
    if((0 != onoff) && (1 != onoff))
    {
        printk(KERN_ERR "[%s] input onoff = [%d] wrong. \n", __func__, onoff);
        return ret;
    }
    
    mutex_lock(&g_led_kernelset_lock);

    ret = gpio_led_name2gpio(led_name);
    if(LEDS_NOT_READY == ret)
    {
        return ret;
    }
    else if(0 <= ret)
    {
        /*查找成功gpio控制调用*/
        gpio_led_switch( ret , onoff );
        ret = 0;
        goto out;  
    }
    else
    {
        /* 继续往下，什么都不做 */
    }

    ret = dr_led_name2dr(led_name);
    if(LEDS_NOT_READY == ret)
    {
        return ret;
    }
    else if(0 <= ret)
    {
        /*查找成功dr控制调用, DR ID从0开始，设备树中从1开始，需要转化*/
        dr_led_switch( ret - 1, onoff );      
        ret = 0;
        goto out;  
    }
    else
    {
        /* 继续往下，什么都不做 */
    }

    printk(KERN_ERR "[%s] input led_name no found. \n", __func__);  /*查找失败*/
out:
    mutex_unlock(&g_led_kernelset_lock);
    return  ret;
}



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hisilicon Tech.Co.,Ltd.<lusuo@hisilicon.com>");
MODULE_DESCRIPTION("BalongV700R200 Hisilicon LED driver");

