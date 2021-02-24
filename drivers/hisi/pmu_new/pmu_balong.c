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

#include <bsp_regulator.h>
#include <bsp_pmu.h>
#include "pmu_stub.h"
#include "pmu_balong.h"
#include "pmic_inner.h"
#include "pmic_ioshare.h"

struct pmu_adp_ops g_pmu_adpops = {
    .clk_32k_enable = dummy_32k_clk_handler,
    .clk_32k_disable = dummy_32k_clk_handler,
    .clk_32k_is_enabled = dummy_32k_clk_handler,
    .clk_xo_enable = dummy_32k_clk_handler,
    .clk_xo_disable = dummy_32k_clk_handler,
    .clk_xo_is_enabled = dummy_32k_clk_handler,
    .usb_state_get = dummy_get_part_state,
    .key_state_get = dummy_get_part_state,
    .irq_mask = dummy_irq_mask_handler,
    .irq_unmask = dummy_irq_mask_handler,
    .irq_is_masked = dummy_irq_state_handler,
    .irq_callback_register = dummy_irq_callback_register,
    .sim_debtime_set = dummy_sim_debtime_set,
    .ldo22_res_enable = dummy_no_para_handler,
    .ldo22_res_disable = dummy_no_para_handler,
    .version_get = dummy_version_get,
    .sim_upres_disable = dummy_sim_upres_disable,
    .ocp_register = dummy_ocp_register,
    .ioshare_status_get = dummy_one_para_handler,
};

u32 bsp_pmic_get_base_addr(void)
{
    return pmic_get_base_addr();
}
void bsp_pmic_reg_write(u32 addr, u32 value)
{
    pmic_reg_write(addr, value);
}
/*****************************************************************************
* �� �� ��  : pmu_reg_read
*
* ��������  : ��pmuоƬ�Ĵ����Ķ�����
*
* �������  : u16 addr����д��ļĴ�����ַ
*             u8 u8Data������������
*
* �������  : ��
*
* �� �� ֵ  : ��
*****************************************************************************/
void bsp_pmic_reg_read(u32 addr, u32 *pValue)
{
    pmic_reg_read(addr, pValue);
}
/*****************************************************************************
* �� �� ��  : pmu_reg_write_mask
*
* ��������  : ��pmuоƬ�Ĵ�����ĳЩbit��λ
*
* �������  : u16 addr����д��ļĴ�����ַ
*             u8 u8Data������������
*
* �������  : ��
*
* �� �� ֵ  : ��
*****************************************************************************/
void bsp_pmic_reg_write_mask(u32 addr, u32 value, u32 mask)
{
    pmic_reg_write_mask(addr, value, mask);
}
int bsp_pmic_reg_show(u32 addr)
{
    return pmic_reg_show(addr);
}

int pmu_adpops_register(struct pmu_adp_ops *pmu_ops)
{
    if (NULL == pmu_ops)
    {
        pmu_print_error("argc is NULL check it\n");
        return BSP_ERROR;
    }
    g_pmu_adpops = *pmu_ops;

    return BSP_OK;
}

/*****************************************************************************
 �� �� ��  : bsp_pmu_32k_clk_enable
 ��������  : ����pmu��32kʱ��
 �������  : clk_id:32kʱ�ӱ��
 �������  : ��
 �� �� ֵ  : �����ɹ���ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
int bsp_pmu_32k_clk_enable(pmu_clk_e clk_id)
{
    return  g_pmu_adpops.clk_32k_enable(clk_id);
}
/*****************************************************************************
 �� �� ��  : bsp_pmu_32k_clk_disable
 ��������  : �ر�pmu��32kʱ��
 �������  : clk_id:32kʱ�ӱ��
 �������  : ��
 �� �� ֵ  : �رճɹ���ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
int bsp_pmu_32k_clk_disable(pmu_clk_e clk_id)
{
    return  g_pmu_adpops.clk_32k_disable(clk_id);
}
/*****************************************************************************
 �� �� ��  : bsp_pmu_32k_clk_is_enabled
 ��������  : ��ѯpmu��32kʱ���Ƿ���
 �������  : clk_id:32kʱ�ӱ��
 �������  : ��
 �� �� ֵ  : ������ر�
 ���ú���  :
 ��������  :
*****************************************************************************/
int bsp_pmu_32k_clk_is_enabled(pmu_clk_e clk_id)
{
    return  g_pmu_adpops.clk_32k_is_enabled(clk_id);
}
/*****************************************************************************
 �� �� ��  : bsp_pmu_xo_clk_enable
 ��������  : ����pmu��xoʱ��
 �������  : xo
 �������  : ��
 �� �� ֵ  : �����ɹ���ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
int bsp_pmu_xo_clk_enable(int clk_id)
{
    return  g_pmu_adpops.clk_xo_enable(clk_id);
}
/*****************************************************************************
 �� �� ��  : bsp_pmu_xo_clk_disable
 ��������  : �ر�pmu��xoʱ��
 �������  :
 �������  : ��
 �� �� ֵ  : �رճɹ���ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
int bsp_pmu_xo_clk_disable(int clk_id)
{
    return  g_pmu_adpops.clk_xo_disable(clk_id);
}
/*****************************************************************************
 �� �� ��  : bsp_pmu_xo_clk_is_enabled
 ��������  : ��ѯpmu��xoʱ���Ƿ���
 �������  : clk_id:xoʱ�ӱ��
 �������  : ��
 �� �� ֵ  : ������ر�
 ���ú���  :
 ��������  :
*****************************************************************************/
int bsp_pmu_xo_clk_is_enabled(int clk_id)
{
    return  g_pmu_adpops.clk_xo_is_enabled(clk_id);
}

/*****************************************************************************
 �� �� ��  : bsp_pmu_version_get
 ��������  : ��ȡusb�Ƿ���״̬
 �������  : void
 �������  : ��
 �� �� ֵ  : usb�����γ�
 ���ú���  :
 ��������  : ���ػ�ģ��
*****************************************************************************/
bool bsp_pmu_usb_state_get(void)
{
    return  g_pmu_adpops.usb_state_get();
}
/*****************************************************************************
 �� �� ��  : bsp_pmu_key_state_get
 ��������  : ��ȡpower�����Ƿ���״̬
 �������  : void
 �������  : ��
 �� �� ֵ  : usb�����γ�
 ���ú���  :
 ��������  : ���ػ�ģ��
*****************************************************************************/
bool bsp_pmu_key_state_get(void)
{
    return  g_pmu_adpops.key_state_get();
}

/*****************************************************************************
 �� �� ��  : bsp_pmu_irq_mask
 ��������  : ע���жϴ���ص�����
 �������  : irq
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  : ������pmu�ڲ����ж�ģ��
*****************************************************************************/
void bsp_pmu_irq_mask(unsigned int irq)
{
    g_pmu_adpops.irq_mask(irq);
}
/*****************************************************************************
 �� �� ��  : bsp_pmu_irq_unmask
 ��������  : ע���жϴ���ص�����
 �������  : irq
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  : ������pmu�ڲ����ж�ģ��
*****************************************************************************/
void bsp_pmu_irq_unmask(unsigned int irq)
{
    g_pmu_adpops.irq_unmask(irq);
}
/*****************************************************************************
 �� �� ��  : bsp_pmu_irq_is_masked
 ��������  : ע���жϴ���ص�����
 �������  : irq
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  : ������pmu�ڲ����ж�ģ��
*****************************************************************************/
int bsp_pmu_irq_is_masked(unsigned int irq)
{
    return  g_pmu_adpops.irq_is_masked(irq);
}
/*****************************************************************************
 �� �� ��  : bsp_pmu_irq_callback_register
 ��������  : ע���жϴ���ص�����
 �������  : irq
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  : ������pmu�ڲ����ж�ģ��
*****************************************************************************/
int bsp_pmu_irq_callback_register(unsigned int irq,pmufuncptr routine,void *data)
{
    return  g_pmu_adpops.irq_callback_register(irq,routine,data);
}

/*****************************************************************************
 �� �� ��  : bsp_pmu_sim_debtime_set
 ��������  : ����sim��ȥ��ʱ��
 �������  : uctime:ȥ��ʱ��
 �������  : ��
 �� �� ֵ  : ���óɹ���ʧ��
 ���ú���  :
 ��ע˵��  :
*****************************************************************************/
int bsp_pmu_sim_debtime_set(u32 uctime)
{
   return  g_pmu_adpops.sim_debtime_set(uctime);
}

void bsp_pmu_ldo22_res_enable(void)
{
   g_pmu_adpops.ldo22_res_enable();
}
void bsp_pmu_ldo22_res_disable(void)
{
   g_pmu_adpops.ldo22_res_disable();
}

/*****************************************************************************
 �� �� ��  : bsp_pmu_version_get
 ��������  : ��ȡpmu�汾��
 �������  : void
 �������  : ��
 �� �� ֵ  : pmu�汾��
 ���ú���  :
 ��������  : ����hso��msp����
*****************************************************************************/
char* bsp_pmu_version_get(void)
{
    return g_pmu_adpops.version_get();
}
/*****************************************************************************
 �� �� ��  : bsp_pmu_version_get
 ��������  : ��ȡpmu�汾��
 �������  : void
 �������  : ��
 �� �� ֵ  : pmu�汾��
 ���ú���  :
 ��������  : ����hso��msp����
*****************************************************************************/
int bsp_pmu_sim_upres_disable(u32 sim_id)
{
    return g_pmu_adpops.sim_upres_disable(sim_id);
}

int bsp_pmu_ocp_register(int volt_id,PMU_OCP_FUNCPTR func)
{
    return g_pmu_adpops.ocp_register(volt_id,func);
}

int bsp_pmic_ioshare_status_get(pmic_ioshare_e id)
{
    return g_pmu_adpops.ioshare_status_get(id);
}

EXPORT_SYMBOL_GPL(bsp_pmic_get_base_addr);
EXPORT_SYMBOL_GPL(bsp_pmu_version_get);
EXPORT_SYMBOL_GPL(bsp_pmic_reg_show);
EXPORT_SYMBOL_GPL(bsp_pmic_reg_read);
EXPORT_SYMBOL_GPL(bsp_pmic_reg_write);
EXPORT_SYMBOL_GPL(bsp_pmic_reg_write_mask);
EXPORT_SYMBOL_GPL(bsp_pmu_32k_clk_is_enabled);
EXPORT_SYMBOL_GPL(bsp_pmu_32k_clk_disable);
EXPORT_SYMBOL_GPL(bsp_pmu_32k_clk_enable);
EXPORT_SYMBOL_GPL(bsp_pmu_xo_clk_is_enabled);
EXPORT_SYMBOL_GPL(bsp_pmu_xo_clk_disable);
EXPORT_SYMBOL_GPL(bsp_pmu_xo_clk_enable);
EXPORT_SYMBOL_GPL(bsp_pmu_usb_state_get);
EXPORT_SYMBOL_GPL(bsp_pmu_key_state_get);
EXPORT_SYMBOL_GPL(bsp_pmu_irq_is_masked);
EXPORT_SYMBOL_GPL(bsp_pmu_irq_unmask);
EXPORT_SYMBOL_GPL(bsp_pmu_irq_mask);
EXPORT_SYMBOL_GPL(bsp_pmic_ioshare_status_get);

