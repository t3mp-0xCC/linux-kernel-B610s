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


#ifndef _SCI_BALONG_H_
#define _SCI_BALONG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_pmu.h"
#include "bsp_om.h"

#include <product_config.h>

/* add if you need */
#define BALONG_SIM_NUM  2

/* use raise int only */
#define INT_LVL_SIM0_PMU_IN_RAISE       (PMU_INT_SIM0_IN_RAISE)
#define INT_LVL_SIM0_PMU_IN_FALL        (PMU_INT_SIM0_IN_FALL)
#define INT_LVL_SIM0_PMU_OUT_RAISE      (PMU_INT_SIM0_OUT_RAISE)
#define INT_LVL_SIM0_PMU_OUT_FALL       (PMU_INT_SIM0_OUT_FALL)


#define INT_LVL_SIM1_PMU_IN      (PMU_INT_SIM1_IN_RAISE)
#define INT_LVL_SIM1_PMU_OUT     (PMU_INT_SIM1_OUT_FALL)

/* sim detect pin */
#define SIM0_GPIO_DETECT         (GPIO_0_5)  /* yangzhi modified 2014-07-17 */

#define DRIVER_NAME_SIM			"sim_io"
#define DETECT_NAME_SIM 		"sim_detect"

#define PMU_HPD_DEBOUNCE_TIME    (400)


#ifndef OK
#define OK      (0)
#endif

#ifndef ERROR
#define ERROR   (-1)
#endif

#ifdef __KERNEL__
#define SCI_PRINT(fmt, ...)   (bsp_trace(BSP_LOG_LEVEL_FATAL,   BSP_MODU_SCI, "[sci]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#endif

typedef enum
{
    SIM_PMU_HPD_FALL              = 0,
    SIM_PMU_HPD_LOW               = 1,
    SIM_PMU_HPD_RAISE             = 2,
    SIM_PMU_HPD_HIGH              = 3,
    SIM_PMU_HPD_BUTT
} sci_pmu_hpd_status;

typedef enum
{
    SIM_CARD_STATUS_OUT          = 0,
    SIM_CARD_STATUS_IN_POSITION  = 1,
    SIM_CARD_STATUS_IN           = 2,
    SIM_CARD_STATUS_LEAVE        = 3,
    SIM_CARD_STATUS_BUTT
} sci_status_enum;


    

typedef enum
{
    SIM_CARD_DETECT_LOW = 0,
    SIM_CARD_DETECT_HIGH  = 1
} sci_detect_level_enum;


typedef struct {
	bool sci_init_flag;
	sci_status_enum  sci0_card_satus;
    sci_pmu_hpd_status sci0_pmu_hpd_status;
	u32  sci0_detect_level;
    
	sci_status_enum  sci1_card_satus;
    sci_pmu_hpd_status sci1_pmu_hpd_status;
	u32  sci1_detect_level;
} bsp_sci_st;


typedef struct sim_cfg
{
    u32 sim_hpd_low;
    u32 sim_hpd_high;
    u32 sim_hpd_fall;
    u32 sim_hpd_raise;
    u32 sim_gpio;
}sim_hotplug_cfg;

#ifdef __cplusplus
}
#endif

#endif /* _VIC_BALONG_H_ */

