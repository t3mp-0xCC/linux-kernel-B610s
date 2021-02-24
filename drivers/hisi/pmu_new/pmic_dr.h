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

#ifndef _PMIC_DR_H_
#define _PMIC_DR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <osl_spinlock.h>
#include <bsp_dr.h>

#define ALWAYS_ON_OFF    0xf/*长亮或者长暗寄存器配置值*/


struct dr_fla
{
    u32 onoff_reg;
    u32 onoff_bit;
    u32 mode_reg;
    u32 mode_bit;

    u32 period_reg;
    u32 period_step;
    u32 light_reg;
    u32 light_step;
};
struct dr_bre
{
    u32 onoff_reg;
    u32 onoff_bit;
    u32 mode_reg;
    u32 mode_bit;

    u32 onofftime_reg;
    u32 onofftime_num;
    u32 ontime_mask;
    u32 ontime_offset;
    u32 offtime_mask;
    u32 offtime_offset;
    
    u32 risefalltime_reg;
    u32 risefalltime_num;
    u32 risetime_mask;
    u32 risetime_offset;
    u32 falltime_mask;
    u32 falltime_offset;
};
struct dr_reg
{
    u32 mode_sel_reg;
    u32 mode_sel_bit;
    u32 current_reg;
    u32 current_mask;
};
struct dr_del
{
    u32 del_reg;
    u32 del_step;
    u32 del_minms;
    u32 del_maxms;
};
struct pmic_dr_ctrl
{
    u32 id;
    const char *name;
    u32 current_nums;
    const u32 *current_table;
    struct dr_reg dr_reg;
    struct dr_fla dr_fla;
    struct dr_bre dr_bre;
    struct dr_del dr_del;/*start_del*/
    const u32 *onofftime_table;
    const u32 *risefalltime_table;
};
struct pmic_dr_info
{
    u32 dr_num;
    u32 dr_min;
    u32 dr_max;
    struct pmic_dr_ctrl *dr_ctrl;
    spinlock_t dr_lock;
};

struct pmu_dr_adp_ops{
    int (*dr_list_current)(int dr_id, unsigned selector);
    int (*dr_set_mode)(int dr_id, dr_mode_e mode);
    dr_mode_e (*dr_get_mode)(int dr_id);
    int (*dr_fla_time_set)(int dr_id, DR_FLA_TIME *dr_fla_time_st);
    int (*dr_fla_time_get)(int dr_id, DR_FLA_TIME *dr_fla_time_st);
    int (*dr_bre_time_set)(int dr_id, DR_BRE_TIME *dr_bre_time_st);
    unsigned (*dr_bre_time_list)(int dr_id, dr_bre_time_e bre_time_enum ,unsigned selector);
    unsigned (*dr_bre_time_selectors_get)(int dr_id, dr_bre_time_e bre_time_enum);
    int (*dr_start_delay_set)(int dr_id, unsigned delay_ms);
};

int pmic_dr_set_mode(int dr_id, dr_mode_e mode);
dr_mode_e pmic_dr_get_mode(int dr_id);
int pmic_dr_is_enabled(int dr_id);
int pmic_dr_get_current(int dr_id);
int pmic_dr_list_current(int dr_id, unsigned selector);
int pmic_dr_fla_time_get(int dr_id, DR_FLA_TIME* dr_fla_time_st);
int pmic_dr_fla_time_set(int dr_id, DR_FLA_TIME *dr_fla_time_st);
int pmic_dr_bre_time_set(int dr_id, DR_BRE_TIME *dr_bre_time_st);
unsigned pmic_dr_bre_time_list(int dr_id, dr_bre_time_e bre_time_enum ,unsigned selector);
unsigned pmic_dr_bre_time_selectors_get(int dr_id, dr_bre_time_e bre_time_enum);
int pmic_dr_start_delay_set(int dr_id, unsigned delay_ms);
void pmic_dr_dts_init(void);
void pmic_dr_init(void);

#ifdef __cplusplus
}
#endif

#endif 

