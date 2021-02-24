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

#include <product_config.h>
#include <osl_bio.h>
#include <osl_spinlock.h>
#include <of.h>
#include <bsp_version.h>
#include <pmic_inner.h>
#include <pmic_volt.h>
#include <pmic_dr.h>
#include <pmic_ioshare.h>
#include <pmic_comm.h>

static u32 pmic_base_addr = 0;
/*----------------------------------�����Ĵ��������ӿ�---------------------------------------*/
/*****************************************************************************
* �� �� ��  : pmic_get_base_addr
*
* ��������  : ����pmu����ַ
*
* �������  : ��
*
* �������  : ��
*
* �� �� ֵ  : pmu��ʼ��֮ǰΪ0����ʼ��֮����ӳ���Ļ�ַ
*****************************************************************************/

u32 pmic_get_base_addr(void)
{
    return pmic_base_addr;
}

void pmic_reg_write(u32 addr, u32 value)
{
    writel(value,(void*)(pmic_base_addr + (addr << 2)));
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
void  pmic_reg_read(u32 addr, u32 *pValue)
{
    *pValue = readl((void*)(pmic_base_addr + (addr << 2)));
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
void pmic_reg_write_mask(u32 addr, u32 value, u32 mask)
{
    u32 reg_tmp = 0;

    pmic_reg_read(addr, &reg_tmp);
    reg_tmp &= ~mask;
    reg_tmp |= value;
    pmic_reg_write(addr, reg_tmp);
}
int pmic_reg_show(u32 addr)
{
    u32 value = 0;

    pmic_reg_read(addr, &value);
    pmic_print_error("pmuRead addr 0x%x value is 0x%x!!\n",addr,value);
    return value;
}


/*****************************************************************************
 �� �� ��  : pmu_init
 ��������  : PMUģ���ʼ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��ע˵��  : fastboot����ɽ���ѹ���Ʊ�����ddr�У�
             ����nv�����趨��Ĭ�ϵ�ѹ��ecoģʽ��
             ������Ŀǰ��Ϊ��׮��
*****************************************************************************/
static int __init pmu_pmic_init(void)
{
    struct device_node *dev_node = NULL;
    void *pmussi_base = NULL;   

    dev_node = of_find_compatible_node(NULL,NULL,"hisilicon,pmu_pmic_app");
    if(!dev_node)
    {
        pmic_print_error("get pmic dts node failed!\n");
		return -1;
    }
    /* �ڴ�ӳ�䣬��û�ַ */
    pmussi_base = of_iomap(dev_node, 0);
    if (NULL == pmussi_base)
    {
        pmic_print_error("pmussi asic ioremap fail\n");
        return -1;
    }
    if(bsp_get_version_info()->board_type == BSP_BOARD_TYPE_SFT)
    {
        dev_node = of_find_compatible_node(NULL,NULL,"hisilicon,pmu_pmicsft_app");
        if(!dev_node)
        {
            pmic_print_error("get pmic sft dts node failed!\n");
        }
        /* �ڴ�ӳ�䣬��û�ַ */
        pmussi_base = of_iomap(dev_node, 0);
        if (NULL == pmussi_base)
        {
            pmic_print_error("pmussi sft ioremap fail\n");
            return -1;
        }
    }

    pmic_base_addr = (u32)pmussi_base;

    pmic_print_error("pmussi addr: 0x%x!\n",pmic_base_addr);

    /*init volt reference*/
    pmic_volt_init();
    /*init dr reference*/
    pmic_dr_init();
    /*init comm reference*/
    pmic_comm_init();
    return 0;
}
arch_initcall(pmu_pmic_init);
static void __exit pmu_pmic_exit(void)
{
}
module_exit(pmu_pmic_exit);
