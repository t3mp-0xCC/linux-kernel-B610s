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

#ifndef _PMU_BALONG_H_
#define _PMU_BALONG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <bsp_pmu.h>
#include <bsp_icc.h>
#include <bsp_om.h>

#define  pmu_print_error(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_PMU, "[pmu]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define  pmu_print_info(fmt, ...)     (bsp_trace(BSP_LOG_LEVEL_INFO, BSP_MODU_PMU, "[pmu]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))

struct pmu_adp_ops{

    int (*clk_32k_enable)(int clk_id);
    int (*clk_32k_disable)(int clk_id);
    int (*clk_32k_is_enabled)(int clk_id);
    int (*clk_xo_enable)(int clk_id);
    int (*clk_xo_disable)(int clk_id);
    int (*clk_xo_is_enabled)(int clk_id);
    bool (*usb_state_get)(void);
    bool (*key_state_get)(void);
    void (*irq_mask)(unsigned int irq);
    void (*irq_unmask)(unsigned int irq);
    int (*irq_is_masked)(unsigned int irq);
    int (*irq_callback_register)(unsigned int irq,pmufuncptr routine,void *data);
    int (*sim_debtime_set)(u32 uctime);
    void (*ldo22_res_enable)(void);
    void (*ldo22_res_disable)(void);
    //unsigned int (*irq_inner_id_get)(pmu_int_mod_e mod);
    char* (*version_get)(void);
    int (*sim_upres_disable)(u32 sim_id);
    int (*ocp_register)(int volt_id,PMU_OCP_FUNCPTR func);
    int (*ioshare_status_get)(pmic_ioshare_e id);
};
/*º¯ÊýÉùÃ÷*/
int pmu_adpops_register(struct pmu_adp_ops *pmic_ops);

void pmic_log_level_set_debug(u32 level);
u32 pmic_log_level_get_debug(void);
void pmic_addr_debug(void);
void pmic_voltage_table_debug(void);
void pmic_volt_id_debug(void);
void pmic_volt_state_debug(void);
void pmic_volt_set_debug(int volt_id, int voltage);
void pmic_get_shm_mem_debug(void);
void pmu_help(void);
void pmic_volt_list_show(int volt_id);
void pmic_otp_threshold_set(void);
void pmic_uvp_threshold_set(void);
void pmic_ocp_handle(void* para);
void pmic_otp_handle(void* para);

#ifdef __cplusplus
}
#endif

#endif /* end #define _PMU_TEST_H_*/

