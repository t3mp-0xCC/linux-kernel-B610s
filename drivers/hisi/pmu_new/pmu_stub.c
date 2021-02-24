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

#include <bsp_pmu.h>
#include "pmu_balong.h"
#include "pmu_stub.h"


/*****************************************************************************
 �� �� ��  : dummy_32k_clk_enable
 ��������  : �������رա���ѯpmu��32kʱ�Ӵ�׮����
 �������  : clk_id:32kʱ�ӱ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
int dummy_32k_clk_handler(int clk_id)
{
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}
/*****************************************************************************
 �� �� ��  : dummy_irq_handler
 ��������  : mask,unmask pmu�а�����ģ���irq��׮����
 �������  : clk_id:32kʱ�ӱ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
void dummy_irq_mask_handler(unsigned int irq)
{
    pmu_print_error("no pmic macro defined,use dummy!");
}
/*****************************************************************************
 �� �� ��  : dummy_irq_handler
 ��������  : ��ѯpmu�а���ģ���ж��Ƿ����δ�׮����
 �������  : clk_id:32kʱ�ӱ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
int dummy_irq_state_handler(unsigned int irq)
{
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}
/*****************************************************************************
 �� �� ��  : bsp_pmu_irq_callback_register
 ��������  : ע���жϴ���ص�����Ĭ�ϴ�׮����
 �������  : irq
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  : ������pmu�ڲ����ж�ģ��
*****************************************************************************/
int dummy_irq_callback_register(unsigned int irq,pmufuncptr routine,void *data)
{
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}
/*****************************************************************************
 �� �� ��  : dummy_get_boot_state
 ��������  :ϵͳ����ʱ���pmu�Ĵ���״̬��Ĭ�ϴ�׮����
 �������  : void
 �������  : reset.log
 �� �� ֵ  : pmu�����ok
 ���ú���  :
 ��������  :ϵͳ��ά�ɲ�
*****************************************************************************/
int dummy_get_boot_state(void)
{
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}
/*****************************************************************************
 �� �� ��  : dummy_get_part_state
 ��������  :ϵͳ����ʱ���pmu��usb\��������״̬��Ĭ�ϴ�׮����
 �������  : void
 �������  : reset.log
 �� �� ֵ  : pmu�����ok
 ���ú���  :
 ��������  :��ŵ硢���ػ�
*****************************************************************************/
bool dummy_get_part_state(void)
{
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}
/*****************************************************************************
 �� �� ��  : dummy_sim_debtime_set
 ��������  : ����SIM���ж�ȥ��ʱ��Ĵ�׮����
 �������  : para:����
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
int dummy_sim_debtime_set(u32 para)
{
    pmu_print_info("para is %d",para);
    pmu_print_error("no pmic macro defined,use dummy!");
    return  BSP_PMU_NO_PMIC;
}

void dummy_no_para_handler(void)
{
    pmu_print_info("no pmic macro defined,use dummy!");
}

char* dummy_version_get(void)
{
    pmu_print_info("no pmic macro defined,use dummy!\n");
    return NULL;
}

/*****************************************************************************
 �� �� ��  : dummy_info_get_handler
 ��������  : ��ѯPMU�汾�ŵĴ�׮����
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
u32 dummy_info_get_handler(void)
{
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}

int dummy_sim_upres_disable(u32 para)
{
    pmu_print_info("para is %d",para);
    pmu_print_error("no pmic macro defined,use dummy!");
    return  BSP_PMU_NO_PMIC;
}

int dummy_ocp_register(int volt_id,PMU_OCP_FUNCPTR func)
{
    pmu_print_info("no pmic macro defined,use dummy!\n");
    return 0;
}

int dummy_one_para_handler(u32 para)
{
    pmu_print_info("para is %d",para);
    pmu_print_error("no pmic macro defined,use dummy!");
    return  0;
}

