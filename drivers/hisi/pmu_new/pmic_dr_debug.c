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
#include <pmic_dr.h>

extern struct pmic_dr_info g_pmic_drinfo;
/*****************************************************************************
 �� �� ��  : pmic_dr_set_mode
 ��������  : pmic DRģʽ����
            (������˸)
 �������  : dr_id:����Դ��ţ�mode:ģʽ
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��ע˵��  :
*****************************************************************************/
int pmic_dr_set_mode(int dr_id, dr_mode_e mode)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
	unsigned long dr_flag = 0;

    /* ������Ч�Լ�� */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    /*lint --e{746,718}*/
    spin_lock_irqsave(&dr_ctrl->dr_lock, dr_flag);

    switch(mode){
    case PMU_DRS_MODE_FLA_FLASH:

        pmu_print_info("dr_id[%d] will be set fla-flash mode\n", dr_id);
        /*������Ϊ��˸ģʽ*/
        pmic_reg_read(dr_ctrl->dr_reg.mode_sel_reg, &regval);
        regval |= ((u32)0x1 << dr_ctrl->dr_reg.mode_sel_bit);
        pmic_reg_write(dr_ctrl->dr_reg.mode_sel_reg, regval);

        /*��������ģʽ*/
        pmic_reg_read(dr_ctrl->dr_fla.mode_reg, &regval);
        regval |= ((u32)0x1 << dr_ctrl->dr_fla.mode_bit);
        pmic_reg_write(dr_ctrl->dr_fla.mode_reg, regval);
        break;
    case PMU_DRS_MODE_FLA_LIGHT:
        pmic_print_info("dr_id[%d] will be set fla-light mode\n", dr_id);

        /*������Ϊ��˸ģʽ*/
        pmic_reg_read(dr_ctrl->dr_reg.mode_sel_reg, &regval);
        regval |= ((u32)0x1 << dr_ctrl->dr_reg.mode_sel_bit);
        pmic_reg_write(dr_ctrl->dr_reg.mode_sel_reg, regval);

        /*�ر�����ģʽ*/
        pmic_reg_read(dr_ctrl->dr_fla.mode_reg, &regval);
        regval &= ~((u32)0x1 << dr_ctrl->dr_fla.mode_bit);
        pmic_reg_write(dr_ctrl->dr_fla.mode_reg, regval);
        break;
    case PMU_DRS_MODE_BRE:
        pmic_print_info("dr_id[%d] will be set normal bre mode\n", dr_id);

        pmic_reg_read(dr_ctrl->dr_reg.mode_sel_reg, &regval);
        regval &= ~((u32)0x1 << dr_ctrl->dr_reg.mode_sel_bit);
        pmic_reg_write(dr_ctrl->dr_reg.mode_sel_reg, regval);
        break;
    case PMU_DRS_MODE_BRE_FLASH:
        pmic_print_info("dr_id[%d] will be set bre-flash mode\n", dr_id);
        /*������Ϊ����ģʽ*/
        pmic_reg_read(dr_ctrl->dr_reg.mode_sel_reg, &regval);
        regval &= ~((u32)0x1 << dr_ctrl->dr_reg.mode_sel_bit);
        pmic_reg_write(dr_ctrl->dr_reg.mode_sel_reg, regval);
        /*������Ϊ������˸ģʽ*/
        if(DR_INVAILD_ADDR == dr_ctrl->dr_bre.mode_reg)/*dr3/4/5û�и�ģʽ*/
        {
            pmu_print_error("dr_id[%d] cann't support bre-flash mode,will be set normal bre mode \n", dr_id);
            iret = BSP_PMU_ERROR;
        }
        else
        {
            pmic_reg_read(dr_ctrl->dr_bre.mode_reg, &regval);
            regval |= ((u32)0x1 << dr_ctrl->dr_bre.mode_bit);
            pmic_reg_write(dr_ctrl->dr_bre.mode_reg, regval);
        }
        break;

    case PMU_DRS_MODE_BRE_LIGHT:
        pmic_print_info("dr_id[%d] will be set bre-light mode\n", dr_id);
        /*������Ϊ����ģʽ*/
        pmic_reg_read(dr_ctrl->dr_reg.mode_sel_reg, &regval);
        regval &= ~((u32)0x1 << dr_ctrl->dr_reg.mode_sel_bit);
        pmic_reg_write(dr_ctrl->dr_reg.mode_sel_reg, regval);;
        /*������Ϊ��������ģʽ*/
        if(DR_INVAILD_ADDR == dr_ctrl->dr_bre.mode_reg)/*dr3/4/5û�и�ģʽ*/
        {
            pmic_print_error("dr_id[%d] cann't support breath-light mode,will be set normal bre mode \n", dr_id);
            iret = BSP_PMU_ERROR;
        }
        else
        {
            pmic_reg_read(dr_ctrl->dr_bre.mode_reg, &regval);
            regval &= ~((u32)0x1 << dr_ctrl->dr_bre.mode_bit);
            pmic_reg_write(dr_ctrl->dr_bre.mode_reg, regval);
        }
        break;

    default:
        pmic_print_error("cann't support the mode\n");
        iret = BSP_PMU_ERROR;
        break;
    }

    spin_unlock_irqrestore(&dr_ctrl->dr_lock, dr_flag);

    return iret;
}

/*****************************************************************************
 �� �� ��  : bsp_hi6551_dr_get_mode
 ��������  : PMIC HI6551��Դģʽ��ѯ
            (֧����˸�ͺ���)
            (DR1/2֧����ͨ��˸������������������˸( �߾�����˸ )
             DR3/4/5ֻ֧����˸�ͺ���)
 �������  : dr_id:Ҫ��ѯ�ĵ���Դ���
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��ע˵��  : Acore�ṩ�ú���ʵ��
*****************************************************************************/
dr_mode_e pmic_dr_get_mode(int dr_id)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
	unsigned long dr_flag = 0;

    /* ������Ч�Լ�� */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
        return PMU_DRS_MODE_BUTTOM;

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    /*lint --e{746,718}*/
    spin_lock_irqsave(&dr_ctrl->dr_lock, dr_flag);
    /*���ж�����˸ģʽ���Ǻ���ģʽ*/
    pmic_reg_read(dr_ctrl->dr_reg.mode_sel_reg, &regval);
    if(regval & ((u32)0x1 << dr_ctrl->dr_reg.mode_sel_bit))
    {
        /*�ж�����˸����������˸����*/
        pmic_reg_read(dr_ctrl->dr_fla.mode_reg, &regval);
        if(regval & ((u32)0x1 << dr_ctrl->dr_fla.mode_bit))
        {
            pmic_print_info("dr_id[%d] is in fla-flash mode\n", dr_id);
            return PMU_DRS_MODE_FLA_FLASH;/*��˸�����;���ģʽ*/
        }
        else
        {
            pmic_print_info("dr_id[%d] is in fla-light mode\n", dr_id);
            return PMU_DRS_MODE_FLA_LIGHT;/*��˸�����������ģʽ*/
        }
    }
    else
    {
        /*if(PMIC_HI6551_DR03 <= dr_id),DR3,4,5�����ֺ��������ͺ�����˸*/
        if(DR_INVAILD_ADDR == dr_ctrl->dr_bre.mode_reg)
        {
           pmic_print_info("dr_id[%d] is in normal bre mode\n", dr_id);
           return PMU_DRS_MODE_BRE;/*����ģʽ*/
        }
        else
        {
            /*�ж��Ǻ����������Ǻ�����˸*/
            pmic_reg_read(dr_ctrl->dr_bre.mode_reg, &regval);
            if(regval & ((u32)0x1 << dr_ctrl->dr_bre.mode_bit))
            {
                pmic_print_info("dr_id[%d] is in breath-flash mode\n", dr_id);
                return PMU_DRS_MODE_BRE_FLASH;/*������˸�߾���ģʽ*/
            }
            else
            {
                pmic_print_info("dr_id[%d] is in breath-light mode\n", dr_id);
                return PMU_DRS_MODE_BRE_LIGHT;/*��������ģʽ*/
            }
        }
    }
	spin_unlock_irqrestore(&dr_ctrl->dr_lock, dr_flag);
}

/*****************************************************************************
 �� �� ��  : bsp_pmic_dr_enable
 ��������  : ʹ�ܵ�Դdr_id
 �������  : @dr_id ��Դid
 �������  : ��
 �� �� ֵ  : BSP_PMU_OK: �ɹ��� else:ʧ��
*****************************************************************************/
int pmic_dr_enable(int dr_id)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
	unsigned long dr_flag = 0;

    /* ������Ч�Լ�� */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    if(PMU_DRS_MODE_FLA_FLASH == pmic_dr_get_mode(dr_id) || PMU_DRS_MODE_FLA_LIGHT == pmic_dr_get_mode(dr_id))/*FLA MODE*/
    {
        spin_lock_irqsave(&dr_ctrl->dr_lock, dr_flag);
        pmic_reg_read(dr_ctrl->dr_fla.onoff_reg, &regval);
        regval |= ((u32)0x1 << dr_ctrl->dr_fla.onoff_bit);
        pmic_reg_write(dr_ctrl->dr_fla.onoff_reg, regval);
        /*dr3/4/5��˸ģʽ�¿�������Ҫ����bre���ؼĴ���*/
        spin_unlock_irqrestore(&dr_ctrl->dr_lock, dr_flag);
    }
    else/*BRE MODE*/
    {
        spin_lock_irqsave(&dr_ctrl->dr_lock, dr_flag);
        pmic_reg_read(dr_ctrl->dr_bre.onoff_reg, &regval);
        regval |= ((u32)0x1 << dr_ctrl->dr_bre.onoff_bit);
        pmic_reg_write(dr_ctrl->dr_bre.onoff_reg, regval);
        spin_unlock_irqrestore(&dr_ctrl->dr_lock, dr_flag);
    }

    return iret;
}

/*****************************************************************************
 �� �� ��  : bsp_pmic_dr_disable
 ��������  : ��ֹ��Դdr_id
 �������  : @dr_id ��Դid
 �������  : ��
 �� �� ֵ  : BSP_PMU_OK: �ɹ��� else:ʧ��
*****************************************************************************/
int pmic_dr_disable(int dr_id)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
	unsigned long dr_flag = 0;

    /* ������Ч�Լ�� */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    if(PMU_DRS_MODE_FLA_FLASH == pmic_dr_get_mode(dr_id) || PMU_DRS_MODE_FLA_LIGHT == pmic_dr_get_mode(dr_id))/*FLA MODE*/
    {
        spin_lock_irqsave(&dr_ctrl->dr_lock, dr_flag);
        pmic_reg_read(dr_ctrl->dr_fla.onoff_reg, &regval);
        regval &= ~((u32)0x1 << dr_ctrl->dr_fla.onoff_bit);
        pmic_reg_write(dr_ctrl->dr_fla.onoff_reg, regval);
        /*dr3/4/5��˸ģʽ�¿�������Ҫ����bre���ؼĴ���*/
        spin_unlock_irqrestore(&dr_ctrl->dr_lock, dr_flag);
    }
    else/*BRE MODE*/
    {
        spin_lock_irqsave(&dr_ctrl->dr_lock, dr_flag);
        pmic_reg_read(dr_ctrl->dr_bre.onoff_reg, &regval);
        regval &= ~((u32)0x1 << dr_ctrl->dr_bre.onoff_bit);
        pmic_reg_write(dr_ctrl->dr_bre.onoff_reg, regval);
        spin_unlock_irqrestore(&dr_ctrl->dr_lock, dr_flag);
    }

    return iret;
}

/*****************************************************************************
 �� �� ��  : bsp_pmic_dr_is_enabled
 ��������  : ��ѯĳ·��ѹԴ�Ƿ�����
 �������  : dr_id:��ѹԴid��
 �������  : ��
 �� �� ֵ  : 0:δ������else:����
*****************************************************************************/
int pmic_dr_is_enabled(int dr_id)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
	unsigned long dr_flag = 0;

    /* ������Ч�Լ�� */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    if(PMU_DRS_MODE_FLA_FLASH == pmic_dr_get_mode(dr_id) || PMU_DRS_MODE_FLA_LIGHT == pmic_dr_get_mode(dr_id))/*FLA MODE*/
    {
        spin_lock_irqsave(&dr_ctrl->dr_lock, dr_flag);
        pmic_reg_read(dr_ctrl->dr_fla.onoff_reg, &regval);
        /*dr3/4/5��˸ģʽ�¿�������Ҫ����bre���ؼĴ���*/
        spin_unlock_irqrestore(&dr_ctrl->dr_lock, dr_flag);

        return (regval & ((u32)0x1 << dr_ctrl->dr_fla.onoff_bit));
    }
    else/*BRE MODE*/
    {
        spin_lock_irqsave(&dr_ctrl->dr_lock, dr_flag);
        pmic_reg_read(dr_ctrl->dr_bre.onoff_reg, &regval);
        spin_unlock_irqrestore(&dr_ctrl->dr_lock, dr_flag);

        return (regval & ((u32)0x1 << dr_ctrl->dr_bre.onoff_bit));
    }

    return iret;
}

/*****************************************************************************
 �� �� ��  : pmic_dr_get_current
 ��������  : ��ȡ��ѹԴdr_id�ĵ�ѹֵ
 �������  : dr_id:��ѹԴid��
 �������  : ��
 �� �� ֵ  : ��ѹֵ
*****************************************************************************/
int pmic_dr_get_current(int dr_id)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;
    u32 vsel = 0;
    int current_uA = 0;
	unsigned long dr_flag = 0;

    /* ������Ч�Լ�� */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = &g_pmic_drinfo.dr_ctrl[dr_id];

    spin_lock_irqsave(&dr_ctrl->dr_lock, dr_flag);
    pmic_reg_read(dr_ctrl->dr_reg.current_reg, &regval);
    spin_unlock_irqrestore(&dr_ctrl->dr_lock, dr_flag);
    /* ��ȡ������λֵ */
    vsel = (regval & dr_ctrl->dr_reg.current_mask);
    current_uA = dr_ctrl->current_table[vsel];

    pmu_print_info("dr_id %d's current is  %d uV!\n", dr_id, current_uA);

    return current_uA;
}

/*****************************************************************************
 �� �� ��  : bsp_pmic_dr_set_current
 ��������  : ���õ�ѹԴdr_id�ĵ�ѹֵ������Ϊ[min_uV, max_uV]�����ֵ���ɣ����������û�кϷ�ֵ��ʧ�ܷ���
 �������  : dr_id: Ҫ���õĵ�Դ���
             min_uV: ��С�Ϸ���ѹֵ
             max_uV: ���Ϸ���ѹֵ
 �������  : @selector: ʵ�����õ�ѹֵ�ĵ�λ
 �� �� ֵ  : BSP_PMU_OK: �ɹ��� else:ʧ��
*****************************************************************************/
int pmic_dr_set_current_debug(int dr_id, int uA)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 vsel = 0;
    s32 current_uA = 0;
	unsigned long dr_flag = 0;

    /* ������Ч�Լ�� */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = &g_pmic_drinfo.dr_ctrl[dr_id];

    for(vsel = 0; vsel < dr_ctrl->current_nums; vsel++)
    {
        if (((u32)uA <= dr_ctrl->current_table[vsel]) && ((u32)uA >= dr_ctrl->current_table[vsel]))
        {
            break;
        }
    }
    if((vsel == dr_ctrl->current_nums))
    {
        pmu_print_error("dr_id %d cann't support current between %d and %d uA!\n",dr_id,uA,uA);
        iret = BSP_PMU_ERROR;
        goto out;
    }
    current_uA = dr_ctrl->current_table[vsel];

    spin_lock_irqsave(&dr_ctrl->dr_lock, dr_flag);
    pmic_reg_write_mask(dr_ctrl->dr_reg.current_reg, vsel, dr_ctrl->dr_reg.current_mask);
    spin_unlock_irqrestore(&dr_ctrl->dr_lock, dr_flag);

    pmu_print_info("current is set at %d uA!\n", current_uA);

out:
    return iret;
}

/*****************************************************************************
 �� �� ��  : bsp_pmic_dr_list_current
 ��������  : ��ȡ��ѹԴdr_id��λΪselector�ĵ�ѹֵ
 �������  : dr_id: Ҫ���õĵ�Դ���
             selector: ��ѹ��λ
 �� �� ֵ  : BSP_PMU_OK: �ɹ��� else:ʧ��
*****************************************************************************/
int pmic_dr_list_current(int dr_id, unsigned selector)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    int current_uA = 0;

    /* ������Ч�Լ�� */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = &g_pmic_drinfo.dr_ctrl[dr_id];

    /* ��鵵λ��Ч�� */
    if (selector >= dr_ctrl->current_nums)
    {
        pmu_print_error("selector is %d,not exist,-EINVAL,please input new\n", selector);
        return BSP_PMU_ERROR;
    }

    current_uA = dr_ctrl->current_table[selector];

    pmu_print_info("dr %d selector %d is %d uV!!\n",dr_id,selector,current_uA);

    return current_uA;
}

struct regulator_id_ops pmic_dr_ops = {
		.list_voltage = pmic_dr_list_current,
        .set_current = pmic_dr_set_current,
        .get_current_limit = pmic_dr_get_current,
//        .is_enabled = pmic_dr_is_enabled,
        .enable = pmic_dr_enable,
        .disable = pmic_dr_disable,
};
void pmic_dr_register_ops(void)
{
    int ret = 0;
    ret = regulator_pmic_ops_register(&pmic_dr_ops, "pmic_dr");
    if (ret)
    {
        pmu_print_error("register pmic_dr ops failed,err id: %d!\n",ret);
    }
}
/*****************************************************************************
 �� �� ��  : hi6551_dr_fla_time_get
 ��������  : ��ȡdr����˸����ʱ��͵���ʱ��
 �������  : dr_fla_time_st:��˸ʱ������ṹ��;��λ:us
 �������  : ��
 �� �� ֵ  : ���óɹ�����ʧ��
 ���ú���  :
 ��ע˵��  : ��Ӧ�Ĵ���ֻ������ʱ��Ĺ��ܣ�����Ҫ������
             ��˸����:reg_value*31.25ms;����ʱ��:reg_value*7.8125ms
*****************************************************************************/
int pmic_dr_fla_time_get(int dr_id, DR_FLA_TIME* dr_fla_time_st)
{
    u32 period = 0;/*��˸���ڼĴ�������ֵ*/
    u32 on_time = 0;/*����ʱ������ֵ*/
    unsigned period_time_us = 0;
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;

    /* ������Ч�Լ�� */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    /*feriodʱ���ȡ*/
    pmic_reg_read(dr_ctrl->dr_fla.period_reg,&period);
    period_time_us = period_time_us * dr_ctrl->dr_fla.period_step;
    pmic_print_info("dr fla feriod is %d us!!\n",period_time_us);

    /*��˸����ʱ���ȡ*/
    pmic_reg_read(dr_ctrl->dr_fla.light_reg,&on_time);
    dr_fla_time_st->fla_on_us = on_time * dr_ctrl->dr_fla.light_step;
    pmic_print_info("dr fla on_time is %d us!!\n",dr_fla_time_st->fla_on_us);

    /*��˸����ʱ���ȡ*/
    dr_fla_time_st->fla_off_us = period_time_us - dr_fla_time_st->fla_on_us;
    pmic_print_info("dr fla off_time is %d us!!\n",dr_fla_time_st->fla_off_us);

    return BSP_DR_OK;
}
/*****************************************************************************
 �� �� ��  : hi6551_dr_fla_time_set
 ��������  : ����dr����˸����ʱ��͵���ʱ��
 �������  : dr_fla_time_st:��˸ʱ������ṹ��;��λ:us
 �������  : ��
 �� �� ֵ  : ���óɹ�����ʧ��
 ���ú���  :
 ��ע˵��  : ��Ӧ�Ĵ���ֻ������ʱ��Ĺ��ܣ�����Ҫ������
             ��˸����:reg_value*31.25ms;����ʱ��:reg_value*7.8125ms
*****************************************************************************/
int pmic_dr_fla_time_set(int dr_id, DR_FLA_TIME *dr_fla_time_st)
{
    u32 feriod = 0;/*��˸���ڼĴ�������ֵ*/
    u32 on_time = 0;/*����ʱ������ֵ*/
    DR_FLA_TIME dr_fla_time_current={0,0};
    unsigned int fla_on_time,fla_off_time,feriod_time;
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;

    if(!dr_fla_time_st)
    {
        return BSP_DR_ERROR;
    }

    /* ������Ч�Լ�� */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    /*��ȡ��ǰ��˸��������ֵ*/
    pmic_dr_fla_time_get(dr_id, &dr_fla_time_current);

    if(DR_VALUE_INVALIED != dr_fla_time_st->fla_on_us)
        fla_on_time = dr_fla_time_st->fla_on_us;
    else
        fla_on_time = dr_fla_time_current.fla_on_us;

    if(DR_VALUE_INVALIED != dr_fla_time_st->fla_off_us)
        fla_off_time = dr_fla_time_st->fla_off_us;
    else
        fla_off_time = dr_fla_time_current.fla_off_us;

    feriod_time = fla_on_time + fla_off_time;

    /*feriodʱ������*/
    feriod = (feriod_time / dr_ctrl->dr_fla.period_step);
    pmic_reg_write(dr_ctrl->dr_fla.period_reg,feriod);

    /*��˸����ʱ������*/
    on_time = (fla_on_time / dr_ctrl->dr_fla.light_step);
    pmic_reg_write(dr_ctrl->dr_fla.light_reg,on_time);

    return BSP_DR_OK;
}
/*****************************************************************************
 �� �� ��  : hi6551_dr_bre_time_set
 ��������  : ����dr�ĺ���ʱ��(��������������������������ʱ��)
 �������  : dr_id:Ҫ���õĵ���Դ���;dr_bre_time_st:����ʱ������ṹ��;
            û���ҵ����õ�ʱ�䣬������Ϊ����
 �������  : ��
 �� �� ֵ  : ���óɹ�����ʧ��
 ���ú���  :
 ��ע˵��  : ��Ӧ�Ĵ���ֻ������ʱ��Ĺ��ܣ�����Ҫ������
*****************************************************************************/
int pmic_dr_bre_time_set(int dr_id, DR_BRE_TIME *dr_bre_time_st)
{
    unsigned int bre_on_time,bre_off_time,bre_rise_time,bre_fall_time = 0;
    u32 bre_on,bre_off,bre_rise,bre_fall = 0;/*���õļĴ�����Ӧֵ*/
    u8 i = 0;
    bool valid = 0;
    int iret = BSP_PMU_OK;
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    u32 regval = 0;

    if(!dr_bre_time_st)
    {
        return BSP_DR_ERROR;
    }

    /* ������Ч�Լ�� */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    bre_on_time = dr_bre_time_st->bre_on_ms;
    bre_off_time = dr_bre_time_st->bre_off_ms;
    bre_rise_time = dr_bre_time_st->bre_rise_ms;
    bre_fall_time = dr_bre_time_st->bre_fall_ms;

    /*��������õ�ֵ*/
    if(bre_on_time != DR_VALUE_INVALIED)
    {
        for(i = 0; i < dr_ctrl->dr_bre.onofftime_num; i++ )
        {
            if(bre_on_time == dr_ctrl->onofftime_table[i])
            {
                valid = 1;
                break;
            }
        }
        if(valid)
        {
            valid = 0;

            if(i == dr_ctrl->dr_bre.onofftime_num - 1)
                bre_on = ALWAYS_ON_OFF;/*����*/
            else
                bre_on = i;

            pmic_reg_write_mask(dr_ctrl->dr_bre.onofftime_reg, (bre_on << dr_ctrl->dr_bre.ontime_offset), dr_ctrl->dr_bre.ontime_mask << dr_ctrl->dr_bre.ontime_offset);
        }
        else
        {
            pmic_print_info("can not support bre_on_time you want! \n");
            iret = BSP_DR_ERROR;
        }
    }

    if(bre_off_time != DR_VALUE_INVALIED)
    {
        for(i = 0; i < dr_ctrl->dr_bre.onofftime_num; i++ )
        {
            if(bre_off_time == dr_ctrl->onofftime_table[i])
            {
                valid = 1;
                break;
            }
        }
        if(valid)
        {
            valid = 0;

            if(i == dr_ctrl->dr_bre.onofftime_num - 1)
                bre_off = ALWAYS_ON_OFF;/*����*/
            else
                bre_off = i;

            pmic_reg_write_mask(dr_ctrl->dr_bre.onofftime_reg, (bre_off << dr_ctrl->dr_bre.offtime_offset), dr_ctrl->dr_bre.offtime_mask << dr_ctrl->dr_bre.offtime_offset);
        }
        else
        {
            pmic_print_info("can not support bre_off_time you want! \n");
            iret = BSP_DR_ERROR;
        }
    }

    if(bre_rise_time != DR_VALUE_INVALIED)
    {
        for(i = 0; i < dr_ctrl->dr_bre.risefalltime_num; i++ )
        {
            if(bre_rise_time == dr_ctrl->risefalltime_table[i])
            {
                valid = 1;
                break;
            }
        }
        if(valid)
        {
            valid = 0;
            bre_rise = i;
            pmic_reg_write_mask(dr_ctrl->dr_bre.risefalltime_reg, (bre_rise << dr_ctrl->dr_bre.risetime_offset), dr_ctrl->dr_bre.risetime_mask << dr_ctrl->dr_bre.risetime_offset);
        }
        else
        {
            pmic_print_info("can not support bre_rise_time you want! \n");
            iret = BSP_DR_ERROR;
        }
    }

    if(bre_fall_time != DR_VALUE_INVALIED)
    {
        for(i = 0; i < dr_ctrl->dr_bre.risefalltime_num; i++ )
        {
            if(bre_fall_time == dr_ctrl->risefalltime_table[i])
            {
                valid = 1;
                break;
            }
        }
        if(valid)
        {
            bre_fall = i;
            pmic_reg_write_mask(dr_ctrl->dr_bre.risefalltime_reg, (bre_rise << dr_ctrl->dr_bre.falltime_offset) , dr_ctrl->dr_bre.falltime_mask << dr_ctrl->dr_bre.falltime_offset);
        }
        else
        {
            pmic_print_info("can not support bre_fall_time you want! \n");
            iret = BSP_DR_ERROR;
        }
    }

    return iret;
}
/*****************************************************************************
 �� �� ��  : hi6551_dr_bre_time_list
 ��������  : ����dr�ĺ���ʱ��(��������������������������ʱ��)����λ��ʱ��ֵ
 �������  : dr_id:Ҫ���õĵ���Դ���;bre_time_enum:��Ҫ��ѯ��ʱ������;
             selector��ʱ�䵵λ
 �������  : ��
 �� �� ֵ  : ��λʱ��
 ���ú���  :
 ��ע˵��  :
*****************************************************************************/
unsigned pmic_dr_bre_time_list(dr_id_e dr_id, dr_bre_time_e bre_time_enum ,unsigned selector)
{
    unsigned dr_bre_time = 0;
    unsigned int bre_on_time,bre_off_time,bre_rise_time,bre_fall_time = 0;
    u32 bre_on,bre_off,bre_rise,bre_fall = 0;/*���õļĴ�����Ӧֵ*/
    u8 i = 0;
    bool valid = 0;
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;

    /* ������Ч�Լ�� */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);

    switch(bre_time_enum)
    {
        case PMU_DRS_BRE_ON_MS:
        case PMU_DRS_BRE_OFF_MS:
            if(selector > dr_ctrl->dr_bre.onofftime_num)
            {
                pmic_print_error("dr_id[%d] bre_onoff_time don't support selector %d!\n",dr_id,selector);
                pmic_print_error("max selector is %d!\n",dr_ctrl->dr_bre.onofftime_num);
                return BSP_PMU_PARA_ERROR;
            }
            else
                dr_bre_time = dr_ctrl->onofftime_table[selector];
            break;
        case PMU_DRS_BRE_RISE_MS:
        case PMU_DRS_BRE_FALL_MS:
            if(selector > dr_ctrl->dr_bre.risefalltime_num)
            {
                pmic_print_error("dr_id[%d] bre_risefall_time don't support selector %d!\n",dr_id,selector);
                pmic_print_error("max selector is %d!\n",dr_ctrl->dr_bre.risefalltime_num);
                return BSP_PMU_PARA_ERROR;
            }
            else
                dr_bre_time = dr_ctrl->risefalltime_table[selector];
            break;
        default:
            pmu_print_info("don't support this bre time!\n");
            return BSP_PMU_PARA_ERROR;
    }
    pmu_print_info("dr_id[%d],enum %d 's selector %d time is %dms!\n",dr_id,bre_time_enum,selector,dr_bre_time);

    return dr_bre_time;
}

/*****************************************************************************
 �� �� ��  : pmic_dr_start_delay_set
 ��������  : ����dr��������ʱʱ��
 �������  : dr_id:Ҫ���õĵ���Դ���;delay_ms:������ʱ��ʱ�䣬��λ:ms
             ���÷�Χ:[0 , 32768]ms
 �������  : ��
 �� �� ֵ  : ���óɹ���ʧ��
 ���ú���  :
 ��ע˵��  : ��Ӧ�Ĵ���ֻ������ʱ��Ĺ��ܣ�����Ҫ��������ֻ��DR3/4/5�д˹���
*****************************************************************************/
int pmic_dr_start_delay_set(dr_id_e dr_id, unsigned delay_ms)
{
    struct pmic_dr_ctrl* dr_ctrl = NULL;
    int iret = BSP_PMU_OK;
    u32 regval = 0;

    /* ������Ч�Լ�� */
    iret = pmic_dr_para_check(dr_id);
    if(BSP_PMU_OK != iret)
    {
        return BSP_PMU_PARA_ERROR;
    }

    dr_ctrl = (struct pmic_dr_ctrl*)(&g_pmic_drinfo.dr_ctrl[dr_id]);
    /* ��Ч�Լ��*/
    if (PMU_INVAILD_DATA == dr_ctrl->dr_del.del_step)
    {
        pmic_print_error("This DR %d can not set start delay time!\n",dr_id);
        return BSP_PMU_PARA_ERROR;
    }
    if ((delay_ms < dr_ctrl->dr_del.del_minms)||(delay_ms > dr_ctrl->dr_del.del_maxms))
    {
        pmic_print_error("This dr start delay time can be set form %d to %d ms!\n",dr_ctrl->dr_del.del_minms,dr_ctrl->dr_del.del_maxms);
        return BSP_PMU_PARA_ERROR;
    }

    pmic_reg_write(dr_ctrl->dr_del.del_reg,(delay_ms/dr_ctrl->dr_del.del_step));

    return BSP_DR_OK;
}
/*****************************************************************************
 �� �� ��  : bsp_pmu_pmic_init
 ��������  : PMIC pmic PMUģ���ʼ��
 �������  : ��
 �������  : ��
*****************************************************************************/
void pmic_dr_init(void)
{
    /*read dts*/
    pmic_dr_dts_init();

    pmic_dr_register_ops();

    pmu_print_error("pmic dr init ok!\n");
}
