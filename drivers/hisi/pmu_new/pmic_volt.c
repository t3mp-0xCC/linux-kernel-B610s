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

/*lint --e{537,958}*/
#include <of.h>
#include <product_config.h>
#include <osl_bio.h>
#include <osl_thread.h>
#include <osl_sem.h>
#include <osl_spinlock.h>
#include <osl_malloc.h>
#include <soc_memmap.h>
#include <bsp_shared_ddr.h>
#include <bsp_pmu.h>
#include <drv_nv_id.h>
#include <drv_nv_def.h>
#include <bsp_version.h>
#include <bsp_nvim.h>
#include <bsp_ipc.h>
#include <bsp_regulator.h>
#include <bsp_pm_om.h>

#include <pmu_balong.h>
#include <pmic_inner.h>
#include <pmic_volt.h>

struct pmic_volt_info g_pmic_voltinfo;
/*****************************************************************************
* �� �� ��  : pmic_volt_para_check
*
* ��������  : ����ѹԴ�Ƿ�Ϸ�
*
* �������  : @volt_id�������ĵ�ѹԴ
*
* �������  : ��
*
* �� �� ֵ  : BSP_PMU_OK:�Ϸ�;   BSP_PMU_ERROR:���Ϸ�
*****************************************************************************/
static __inline__ int pmic_volt_para_check(int volt_id)
{
    /* ��Ч�Լ�� */
    if ((g_pmic_voltinfo.volt_max< volt_id) || (g_pmic_voltinfo.volt_min> volt_id))
    {
        pmu_print_error("PMIC doesn't have volt %d!\n",volt_id);
        pmu_print_error("the volt you can use in this PMIC is from %d to %d,please check!\n", g_pmic_voltinfo.volt_min, g_pmic_voltinfo.volt_max);
        return BSP_PMU_ERROR;
    }

    return BSP_PMU_OK;
}

/*****************************************************************************
* �� �� ��  : pmic_volt_table_check
*
* ��������  : ����ѹԴ���Ա��Ƿ����
*
* �������  : @pmic_volt_table ��ѹԴ���Ա��ַ
*
* �������  : ��
*
* �� �� ֵ  : BSP_PMU_OK:����;   BSP_PMU_ERROR:������
*****************************************************************************/
static __inline__ int pmic_volt_table_check(void)
{
    if((SHM_PMU_VOLTTABLE_MAGIC_START_DATA == g_pmic_voltinfo.magic_start) &&
       (SHM_PMU_VOLTTABLE_MAGIC_END_DATA == g_pmic_voltinfo.magic_end))
    {
        return BSP_PMU_OK;
    }

    return BSP_PMU_ERROR;
}

/*****************************************************************************
 �� �� ��  : bsp_pmic_volt_enable
 ��������  : ʹ�ܵ�Դvolt_id
 �������  : @volt_id ��Դid
 �������  : ��
 �� �� ֵ  : BSP_PMU_OK: �ɹ��� else:ʧ��
*****************************************************************************/
int pmic_volt_enable(int volt_id)
{
    struct pmic_volt_ctrl* volt_ctrl = NULL;
    int iret = BSP_PMU_OK;
	unsigned long volt_flag = 0;
	
    /* ������Ч�Լ�� */
    iret = pmic_volt_para_check(volt_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    /* ��ѹ���Ա�ȫ�Լ�� */
    iret = pmic_volt_table_check();
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_VOLTTABLE_ERROR;
    }

    volt_ctrl = (struct pmic_volt_ctrl*)(&g_pmic_voltinfo.volt_ctrl[volt_id]);
    /* keep a distance of off_on_delay from last time disabled */

    /*�ʺ˼���+������*/
    spin_lock_irqsave(&volt_ctrl->volt_lock, volt_flag);
    /* cannot enable more than one regulator at one time */
    /* set enable register */
    pmic_reg_write_mask(volt_ctrl->volt_reg.en_reg,(0x1 << volt_ctrl->volt_reg.en_bit),(0x1 << volt_ctrl->volt_reg.en_bit));
    spin_unlock_irqrestore(&volt_ctrl->volt_lock, volt_flag);

    return iret;

}

/*****************************************************************************
 �� �� ��  : bsp_pmic_volt_disable
 ��������  : ��ֹ��Դvolt_id
 �������  : @volt_id ��Դid
 �������  : ��
 �� �� ֵ  : BSP_PMU_OK: �ɹ��� else:ʧ��
*****************************************************************************/
int pmic_volt_disable(int volt_id)
{
    struct pmic_volt_ctrl* volt_ctrl = NULL;
    int iret = BSP_PMU_OK;
	unsigned long volt_flag = 0;

    /* ������Ч�Լ�� */
    iret = pmic_volt_para_check(volt_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    /* ��ѹ���Ա�ȫ�Լ�� */
    iret = pmic_volt_table_check();
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_VOLTTABLE_ERROR;
    }

    volt_ctrl = (struct pmic_volt_ctrl*)(&g_pmic_voltinfo.volt_ctrl[volt_id]);
    /* keep a distance of off_on_delay from last time disabled */

    /*�ʺ˼���+������*/
    spin_lock_irqsave(&volt_ctrl->volt_lock, volt_flag);
    /* set disable register */
    pmic_reg_write_mask(volt_ctrl->volt_reg.en_reg,(0x0 << volt_ctrl->volt_reg.en_bit),(0x1 << volt_ctrl->volt_reg.en_bit));
    spin_unlock_irqrestore(&volt_ctrl->volt_lock, volt_flag);

    return BSP_PMU_OK;

}

/*****************************************************************************
 �� �� ��  : bsp_pmic_volt_is_enabled
 ��������  : ��ѯĳ·��ѹԴ�Ƿ�����
 �������  : volt_id:��ѹԴid��
 �������  : ��
 �� �� ֵ  : 0:δ������else:����
*****************************************************************************/
int pmic_volt_is_enabled(int volt_id)
{
    struct pmic_volt_ctrl* volt_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
	unsigned long volt_flag = 0;

    /* ������Ч�Լ�� */
    iret = pmic_volt_para_check(volt_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    /* ��ѹ���Ա�ȫ�Լ�� */
    iret = pmic_volt_table_check();
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_VOLTTABLE_ERROR;
    }

    volt_ctrl = (struct pmic_volt_ctrl*)(&g_pmic_voltinfo.volt_ctrl[volt_id]);
    /* keep a distance of off_on_delay from last time disabled */

    spin_lock_irqsave(&volt_ctrl->volt_lock, volt_flag);
    pmic_reg_read(volt_ctrl->volt_reg.state_reg,&regval);
    spin_unlock_irqrestore(&volt_ctrl->volt_lock, volt_flag);

    /* ����״̬bitΪ1ʱ����ʾ��ʹ�� */
    return (regval & (0x1 << volt_ctrl->volt_reg.state_bit));
}

/*****************************************************************************
 �� �� ��  : bsp_pmic_volt_get_voltage
 ��������  : ��ȡ��ѹԴvolt_id�ĵ�ѹֵ
 �������  : volt_id:��ѹԴid��
 �������  : ��
 �� �� ֵ  : ��ѹֵ
*****************************************************************************/
int pmic_volt_get_voltage(int volt_id)
{
    struct pmic_volt_ctrl* volt_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
    u32 vsel = 0;
    int voltage = 0;
	unsigned long volt_flag = 0;

    /* ������Ч�Լ�� */
    iret = pmic_volt_para_check(volt_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    /* ��ѹ���Ա�ȫ�Լ�� */
    iret = pmic_volt_table_check();
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_VOLTTABLE_ERROR;
    }

    volt_ctrl = &g_pmic_voltinfo.volt_ctrl[volt_id];

    if(volt_ctrl->voltage_flag == VOLT_TABLE)/*���ǵ�λ�����ѯ*/
    {
        spin_lock_irqsave(&volt_ctrl->volt_lock, volt_flag);
        pmic_reg_read(volt_ctrl->volt_reg.voltage_reg, &regval);
        spin_unlock_irqrestore(&volt_ctrl->volt_lock, volt_flag);
        /* ��ȡ��ѹ��λֵ */
        vsel = (regval & volt_ctrl->volt_reg.voltage_mask);
        voltage = volt_ctrl->voltage_table[vsel];
    }
    else if(volt_ctrl->voltage_flag == VOLT_LINEAR)/*�������ԣ������*/
    {
        spin_lock_irqsave(&volt_ctrl->volt_lock, volt_flag);
        pmic_reg_read(volt_ctrl->volt_reg.voltage_reg, &regval);
        spin_unlock_irqrestore(&volt_ctrl->volt_lock, volt_flag);
        /* ��ȡ��ѹ��λֵ */
        vsel = (regval & volt_ctrl->volt_reg.voltage_mask);
        voltage = volt_ctrl->voltage_base + vsel * volt_ctrl->voltage_step;
    }
    else /*���Ǻ㶨��ѹ�����ܸı䣬���غ㶨*/
    {
        voltage = volt_ctrl->voltage_const;
    }

    pmu_print_info("volt_id %d's voltage is  %d uV!\n", volt_id, voltage);

    return voltage;
}

/*****************************************************************************
 �� �� ��  : bsp_pmic_volt_set_voltage
 ��������  : ���õ�ѹԴvolt_id�ĵ�ѹֵ������Ϊ[min_uV, max_uV]�����ֵ���ɣ����������û�кϷ�ֵ��ʧ�ܷ���
 �������  : volt_id: Ҫ���õĵ�Դ���
             min_uV: ��С�Ϸ���ѹֵ
             max_uV: ���Ϸ���ѹֵ
 �������  : @selector: ʵ�����õ�ѹֵ�ĵ�λ
 �� �� ֵ  : BSP_PMU_OK: �ɹ��� else:ʧ��
*****************************************************************************/
int pmic_volt_set_voltage(int volt_id, int min_uV, int max_uV,unsigned *selector)
{
    struct pmic_volt_ctrl* volt_ctrl = NULL;
    pmic_voltflags_t flags = 0;
    int iret = BSP_PMU_OK;
    u32 vsel = 0;
    s32 voltage = 0;
	unsigned long volt_flag = 0;

    /* ������Ч�Լ�� */
    iret = pmic_volt_para_check(volt_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    /* ��ѹ���Ա�ȫ�Լ�� */
    iret = pmic_volt_table_check();
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_VOLTTABLE_ERROR;
    }

    volt_ctrl = &g_pmic_voltinfo.volt_ctrl[volt_id];
   
    if(volt_ctrl->voltage_flag == 0)/*���ǵ�λ�����ѯ*/
    {


        for(vsel = 0; vsel < volt_ctrl->voltage_nums; vsel++)
        {
            if (((u32)min_uV <= volt_ctrl->voltage_table[vsel]) && ((u32)max_uV >= volt_ctrl->voltage_table[vsel]))
            {
                break;
            }
        }
        if((vsel == volt_ctrl->voltage_nums))
        {
            pmu_print_error("volt_id %d cann't support voltage between %d and %d uV!\n",volt_id,min_uV,max_uV);
            return BSP_PMU_ERROR;
        }
        *selector = vsel;
        voltage = volt_ctrl->voltage_table[vsel];

        spin_lock_irqsave(&volt_ctrl->volt_lock, volt_flag);
        pmic_reg_write_mask(volt_ctrl->volt_reg.voltage_reg, vsel, volt_ctrl->volt_reg.voltage_mask);
        spin_unlock_irqrestore(&volt_ctrl->volt_lock, volt_flag);

        goto out;
    }
    else if(volt_ctrl->voltage_flag == 1)/*�������ԣ������*/
    {
        for(vsel = 0; vsel < volt_ctrl->voltage_nums; vsel++)
        {
            if(((u32)min_uV <= (volt_ctrl->voltage_base + vsel * volt_ctrl->voltage_step)) && ((u32)max_uV >= (volt_ctrl->voltage_base + vsel * volt_ctrl->voltage_step)))
            {
                break;
            }
        }
        if(vsel == volt_ctrl->voltage_nums)
        {
            pmu_print_error("volt_id %d cann't support voltage between %d and %d uV!\n",volt_id,min_uV,max_uV);
            return BSP_PMU_ERROR;
        }
        *selector = vsel;
        voltage = volt_ctrl->voltage_base + vsel * volt_ctrl->voltage_step;

        spin_lock_irqsave(&volt_ctrl->volt_lock,flags);
        pmic_reg_write_mask(volt_ctrl->volt_reg.voltage_reg, vsel, volt_ctrl->volt_reg.voltage_mask);
        spin_unlock_irqrestore(&volt_ctrl->volt_lock,flags);

        goto out;
    }
    else /*���Ǻ㶨��ѹ�����ܸı�*/
    {
        if(((u32)min_uV <= volt_ctrl->voltage_const)&&((u32)max_uV >= volt_ctrl->voltage_const))
        {
            voltage = volt_ctrl->voltage_const;
            goto out;
        }
        else
        {
            pmu_print_error("volt_id %d's voltage can not be set!\n", volt_id);
            return BSP_PMU_ERROR;
        }
    }

out:
    pmu_print_info("voltage is set at %d uV!\n", voltage);
    return BSP_PMU_OK;

}

/*****************************************************************************
 �� �� ��  : bsp_pmic_volt_list_voltage
 ��������  : ��ȡ��ѹԴvolt_id��λΪselector�ĵ�ѹֵ
 �������  : volt_id: Ҫ���õĵ�Դ���
             selector: ��ѹ��λ
 �� �� ֵ  : BSP_PMU_OK: �ɹ��� else:ʧ��
*****************************************************************************/
int pmic_volt_list_voltage(int volt_id, unsigned selector)
{
    struct pmic_volt_ctrl* volt_ctrl = NULL;
    int iret = BSP_PMU_OK;
    int voltage = 0;

    /* ������Ч�Լ�� */
    iret = pmic_volt_para_check(volt_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    /* ��ѹ���Ա�ȫ�Լ�� */
    iret = pmic_volt_table_check();
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_VOLTTABLE_ERROR;
    }

    volt_ctrl = &g_pmic_voltinfo.volt_ctrl[volt_id];

    /* ��鵵λ��Ч�� */
    if (selector >= volt_ctrl->voltage_nums)
    {
        pmu_print_error("selector is %d,not exist,-EINVAL,please input new\n", selector);
        return BSP_PMU_ERROR;
    }

    if(volt_ctrl->voltage_flag == 0)/*���ǵ�λ�����ѯ*/
    {
        voltage = volt_ctrl->voltage_table[selector];
    }
    else if(volt_ctrl->voltage_flag == 1)/*�������ԣ������*/
    {
        voltage = volt_ctrl->voltage_base + selector * volt_ctrl->voltage_step;
    }
    else /*���Ǻ㶨��ѹ�����ܸı䣬���غ㶨*/
    {
        voltage = volt_ctrl->voltage_const;
    }


    pmu_print_info("volt %d selector %d is %d uV!!\n",volt_id,selector,voltage);

    return voltage;

}
/*****************************************************************************
 �� �� ��  : bsp_pmic_volt_list_show
 ��������  : ��ʾ��ǰ����·��Դ�����е�λֵ
 �������  : void
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void pmic_volt_list_show(int volt_id)
{
    struct pmic_volt_ctrl* volt_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 selector = 0;
    int voltage = 0;

    /* ������Ч�Լ�� */
    iret = pmic_volt_para_check(volt_id);
    if(BSP_PMU_OK != iret)
    {
        return;
    }

    /* ��ѹ���Ա�ȫ�Լ�� */
    iret = pmic_volt_table_check();
    if(BSP_PMU_OK != iret)
    {
        return;
    }

    volt_ctrl = &g_pmic_voltinfo.volt_ctrl[volt_id];

    for(selector = 0;selector < volt_ctrl->voltage_nums;selector++)
    {
        voltage = pmic_volt_list_voltage(volt_id,selector);
        pmu_print_error("volt %d selector %d is %d uV!!\n",volt_id,selector,voltage);
    }

    return;
}

struct regulator_id_ops pmic_volt_ops = {
        .list_voltage = pmic_volt_list_voltage,
        .set_voltage = pmic_volt_set_voltage,
        .get_voltage = pmic_volt_get_voltage,
//        .is_enabled = pmic_volt_is_enabled,
        .enable = pmic_volt_enable,
        .disable = pmic_volt_disable,
};
void pmic_register_ops(void)
{
    int ret = 0;
    ret = regulator_pmic_ops_register(&pmic_volt_ops, "pmic");
    if (ret)
    {
        pmu_print_error("register pmic_ops ops failed\n");
    }
}
void pmic_ocplist_init(void)
{
    int volt_id = 0;
    struct pmic_volt_ctrl* volt_ctrl = NULL;

    /*init ocp list*/
    for(volt_id = g_pmic_voltinfo.volt_min;volt_id <= g_pmic_voltinfo.volt_max;volt_id++)
    {
        volt_ctrl = (struct pmic_volt_ctrl*)(&g_pmic_voltinfo.volt_ctrl[volt_id]);
        INIT_LIST_HEAD((struct list_head *)&(volt_ctrl->ocp_list));
    }
}

/*****************************************************************************
 �� �� ��  : bsp_pmu_pmic_init
 ��������  : PMIC pmic PMUģ���ʼ��
 �������  : ��
 �������  : ��
*****************************************************************************/
void pmic_volt_init(void)
{    
    /*read dts*/
    pmic_volt_dts_init();

    g_pmic_voltinfo.magic_start = SHM_PMU_VOLTTABLE_MAGIC_START_DATA;
    g_pmic_voltinfo.magic_end = SHM_PMU_VOLTTABLE_MAGIC_END_DATA;

    pmic_register_ops();
    pmic_ocplist_init();

    pmu_print_error("pmic volt init ok!\n");
}

EXPORT_SYMBOL_GPL(g_pmic_voltinfo);
EXPORT_SYMBOL_GPL(pmic_volt_enable);
EXPORT_SYMBOL_GPL(pmic_volt_disable);
EXPORT_SYMBOL_GPL(pmic_volt_is_enabled);
EXPORT_SYMBOL_GPL(pmic_volt_set_voltage);
EXPORT_SYMBOL_GPL(pmic_volt_list_voltage);
EXPORT_SYMBOL_GPL(pmic_volt_get_voltage);
EXPORT_SYMBOL_GPL(pmic_volt_list_show);

