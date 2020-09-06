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

#ifndef __DEV_SMARTSTAR_COUL_H__
#define __DEV_SMARTSTAR_COUL_H__

#define IRQ_CL_INT "cl_int"
#define IRQ_CL_OUT_D_INT "cl_out_D_int"
#define IRQ_CL_IN_D_INT "cl_in_D_int"
#define IRQ_CHG_TIMER_D_INT "chg_timer_D_int"
#define IRQ_LOAD_TIMER_D_INT "load_timer_D_int"
#define IRQ_VBAT_INT "vbat_int"

#define IRQ_MAX 4

/*Hi6421内置库仑计中断编号，请参见寄存器 0x14B。*/
typedef enum  
{
    COUL_INT_ID = 0,   /*cl_out>cl_int时上报中断*/
    COUL_OUT_ID,       /*cl_out计数到81.25%上报中断*/
    COUL_IN_ID,        /*cl_in计数到81.25%上报中断*/
    COUL_VBAT_INT_ID,  /*vbat电压<设定的vbat_int值*/
    COUL_INT_BUTT = IRQ_MAX
}coul_int_idx;

#endif /*__DEV_SMARTSTAR_COUL_H__*/