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



#ifndef _BODYSAR_H_
#define _BODYSAR_H_
#ifdef  __cplusplus
    extern "C"{
#endif
#include <linux/gpio.h>
#include "bsp_softtimer.h"

/*****************************************************************************
 函 数 名  : bodysar_powr_limit_by_plmnlist_check
 功能描述  : 检查当前插入的sim卡的plmn是否在bodysar的plmn-list,
                            如果在，进行降功率
 输入参数  : char *sim_plmn  从sim卡中获得的plmn
 输出参数  : 无
 返 回 值  : TRUE:匹配成功
                         FALSE:匹配失败
 调用函数  :
 被调函数  :
*****************************************************************************/
extern void bodysar_powr_limit_by_plmnlist_check(char *sim_plmn);

#ifndef OK
#define OK                          (0)
#endif
#ifndef ERROR
#define ERROR                       (-1)
#endif
#ifndef TRUE
#define	 TRUE	(1)
#endif
#ifndef FALSE
#define	 FALSE	(0)
#endif

#define BodySAR_N                   GPIO_0_2
#define BodySAR_N_TIMER_LENGTH      (200)

#define MTC_BODYSAR_PLMN_LIST_NAME   ("/online/mtc/bodysar_plmn_list.bin")

typedef struct tagBODYSAR_CTRL_S
{
    uint         gpio_value;
    uint         gpio_value_old;
    struct softtimer_list irq_timer;
//     BodySAR_onoff_func
}BODYSART_CTRL_S;


#ifdef  __cplusplus
    }
#endif

#endif /*_BODYSAR_H_*/
