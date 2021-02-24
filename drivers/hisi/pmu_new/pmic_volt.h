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

#ifndef _PMU_PMIC_H_
#define _PMU_PMIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <osl_list.h>
#include <osl_spinlock.h>
#include <hi_pmu.h>
#include <bsp_pmu.h>

typedef unsigned long pmic_voltflags_t;

struct pmic_reg_cfg
{
    u16 addr;
    u16 value;
};
struct pmic_volt_reg
{
    u32 en_reg;
    u32 en_bit;
    u32 dis_reg;
    u32 dis_bit;
    u32 state_reg;
    u32 state_bit;
    u32 voltage_reg;
    u32 voltage_mask;
};
enum pmic_volt_enum{
    VOLT_TABLE,
    VOLT_LINEAR,
    VOLT_CONST,
};

struct pmic_volt_ocp
{
    struct list_head   list;
    PMU_OCP_FUNCPTR func;
};
struct pmic_volt_ctrl
{
    u32 id;
    const char *name;
    u32 voltage_flag;/*0:table;1:linear;others:consts*/
    u32 voltage_nums;
    u32 voltage_const;/*const voltage value*/
    u32 voltage_base;/*linear voltage base*/
    u32 voltage_step;/*linear voltage step*/
    const u32 *voltage_table;
    struct pmic_volt_reg volt_reg;
    spinlock_t volt_lock;
    u32 off_on_delay;/*struct timeval last_off_time;*/
    struct list_head ocp_list;
};
struct pmic_volt_info
{
    u32 magic_start;
    int volt_min;
    int volt_max;
    struct pmic_volt_ctrl *volt_ctrl;
    u32 magic_end;
};

void pmic_volt_init(void);
void pmic_volt_dts_init(void);

/*test/debug use*/
int pmic_volt_enable(int volt_id);
int pmic_volt_disable(int volt_id);
int pmic_volt_is_enabled(int volt_id);
int pmic_volt_get_voltage(int volt_id);
int pmic_volt_set_voltage(int volt_id, int min_uV, int max_uV,unsigned *selector);
int pmic_volt_list_voltage(int volt_id, unsigned selector);


#ifdef __cplusplus
}
#endif

#endif /* end #define _PMU_TEST_H_*/

