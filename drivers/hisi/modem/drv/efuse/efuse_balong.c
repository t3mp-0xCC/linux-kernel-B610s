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

/*lint -save -e537*/
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "product_config.h"
#include "osl_bio.h"
#include "osl_sem.h"
#include "osl_thread.h"
#include "osl_wait.h"
#include "osl_spinlock.h"
#include "soc_clk.h"
#include "bsp_memmap.h"
#include "hi_base.h"
#include "hi_efuse.h"

#include "bsp_icc.h"
#include "bsp_hardtimer.h"
#include "bsp_om.h"
#include "bsp_efuse.h"


/*lint -restore*/


#define TIMEMAXDELAY   (0x1000000)       /*����ӳ�ʱ��*/

static struct spinlock efuse_lock;
static void* efuse_base_addr = 0;

typedef unsigned int (*punptr)(void);
/*********************************************************************************************
Function:           test_bit_value
Description:        ���ԼĴ����е�ĳλ���Ƿ����ָ����ֵ
Calls:              ��
Data Accessed:      ��
Data Updated:       ��
Input:              addr��  �Ĵ�����ַ
                    bitMask����Ҫ����λ��
                    bitValue��ָ����ֵ
Output:             ��
Return:             �ɹ��򷵻�TRUE�� ʧ���򷵻�FALSE
Others:             ��
*********************************************************************************************/
int test_bit_value(punptr pfun,  u32 bitValue)
{
    u32 time;                       /*��ʱʱ��*/

    for(time = 0; time < TIMEMAXDELAY; time++)
    {
        if(bitValue == (*pfun)())
        {
            return EFUSE_OK;
        }
    }

    return EFUSE_ERROR;
}
int hi_efuse_get_reg(unsigned int *value,unsigned int offset,unsigned int lowbit,unsigned int highbit)
{
    unsigned int mask = 0;
    unsigned int temp = 0;

    if(!efuse_base_addr)
    {
        efuse_print_error("efuse_base_addr may be not initial!\n,pls check!\n");
        return EFUSE_ERROR;
    }
    if(highbit > 31  || (highbit < lowbit) )
    {
        efuse_print_error("param error ,lowbit:%d,highbit:%d\n",lowbit,highbit);
        return EFUSE_ERROR;
    }

    temp   = readl(efuse_base_addr + offset);
    mask   = ((1 << (highbit - lowbit + 1) )-1) << lowbit;
    *value = (temp & mask)>>lowbit;

    return EFUSE_OK;
}

int hi_efuse_set_reg(unsigned int value,unsigned int offset,unsigned int lowbit,unsigned highbit)
{
    void* reg    = efuse_base_addr + offset;
    unsigned int temp   = 0;
    unsigned int mask   = 0;

    if(!efuse_base_addr){
        efuse_print_error("efuse_base_addr may be not initial!\n,pls check!\n");
        return EFUSE_ERROR;
    }
    if(highbit > 31  || (highbit < lowbit) )
    {
        efuse_print_error("param error ,lowbit:%d,highbit:%d\n",lowbit,highbit);
        return EFUSE_ERROR;
    }

    temp   = readl(reg);
    mask   = ((1 << (highbit - lowbit + 1)) -1) << lowbit;

    value  = (temp & (~mask)) | ((value << lowbit) & mask);
    writel(value  ,reg);

    return EFUSE_OK;

}
/*****************************************************************************
* �� ��   : hi_efuse_get_pd_status
* �� ��   : ��ȡefuse power-down״̬
* �� ��   : ��
* �� ��   : ��
* �� ��   : -1��ʾ��ȡʧ�ܣ�0Ϊpd��1Ϊ����̬
* ˵ ��   :
*****************************************************************************/
unsigned int hi_efuse_get_pd_status(void)
{
    unsigned int val = 0;
    int          ret = 0;

    ret = hi_efuse_get_reg(&val, HI_EFUSEC_STATUS_OFFSET, HI_EFUSE_PD_STAT_BIT, HI_EFUSE_PD_STAT_BIT);
    if(ret){
        efuse_print_error("get efuse pd status faild\n");
    }

    return val;
}
/*****************************************************************************
* �� ��   : hi_efusec_set_pd_en
* �� ��   : ʹ��efuse power-down״̬
* �� ��   : ��
* �� ��   : ��
* �� ��   : -1��ʾ��ȡʧ�ܣ�0Ϊpd,11Ϊ����̬
* ˵ ��   :
*****************************************************************************/
void hi_efusec_set_pd_en(void)
{
    unsigned int ret    = 0;

    ret = hi_efuse_set_reg(0, HI_EFUSEC_CFG_OFFSET, HI_EFUSE_PD_EN_BIT, HI_EFUSE_PD_EN_BIT);
    if(ret){
        efuse_print_error("get efuse pd status faild\n");
    }

    return ;
}
/*****************************************************************************
* �� ��   : hi_efusec_set_pd_en
* �� ��   : ѡ���ź�Ϊapb����efuse
* �� ��   : ��
* �� ��   : ��
* �� ��   :
* ˵ ��   :
*****************************************************************************/
void hi_efusec_cfg_aib_sel(void)
{
    unsigned int ret    = 0;

    ret = hi_efuse_set_reg(1, HI_EFUSEC_CFG_OFFSET, HI_EFUSE_AIB_SEL_BIT, HI_EFUSE_AIB_SEL_BIT);
    if(ret){
        efuse_print_error("Set efuse AIB_sel faild\n");
    }

    return ;

}
/*****************************************************************************
* �� ��   : hi_efusec_set_pd_en
* �� ��   : ����efuse group
* �� ��   : ��
* �� ��   : ��
* �� ��   :
* ˵ ��   :
*****************************************************************************/
void hi_efusec_set_group(unsigned int val)
{

    unsigned int ret    = 0;

    ret = hi_efuse_set_reg(val, HI_EFUSE_GROUP_OFFSET, HI_EFUSE_GROUP_LBIT, HI_EFUSE_GROUP_HBIT);
    if(ret){
        efuse_print_error("Set efuse group faild\n");
    }

    return ;
}

void hi_efusec_set_read_en(void)
{
    unsigned int ret    = 0;

    ret = hi_efuse_set_reg(1, HI_EFUSEC_CFG_OFFSET, HI_EFUSE_RD_EN_BIT, HI_EFUSE_RD_EN_BIT);
    if(ret){
        efuse_print_error("Set efuse read enable faild\n");
    }

    return ;

}
unsigned int hi_efusec_get_read_status(void)
{
    unsigned int val = 0;
    int          ret = 0;

    ret = hi_efuse_get_reg(&val, HI_EFUSEC_STATUS_OFFSET, HI_EFUSE_RD_STAT_BIT, HI_EFUSE_RD_STAT_BIT);
    if(ret){
        efuse_print_error("get efuse read status faild\n");
    }

    return val;

}

unsigned int hi_efusec_get_data(void)
{
    unsigned int val = 0;
    int          ret = 0;

    ret = hi_efuse_get_reg(&val, HI_EFUSEC_DATA_OFFSET, HI_EFUSE_DATA_LBIT, HI_EFUSE_DATA_HBIT);
    if(ret){
        efuse_print_error("get efuse data faild\n");
    }

    return val;

}

unsigned int hi_efusec_get_pgm_disable_flag(void)
{
    unsigned int val = 0;
    int          ret = 0;

    ret = hi_efuse_get_reg(&val, HI_HW_CFG_OFFSET, HI_EFUSE_DISFLAG_BIT, HI_EFUSE_DISFLAG_BIT);
    if(ret){
        efuse_print_error("get efuse disable flag faild\n");
    }

    return val;

}

void hi_efusec_set_count(unsigned val)
{
    unsigned int ret    = 0;

    ret = hi_efuse_set_reg(val, HI_EFUSEC_COUNT_OFFSET, HI_EFUSE_COUNT_LBIT, HI_EFUSE_COUNT_HBIT);
    if(ret){
        efuse_print_error("Set efuse count faild\n");
    }

    return ;

}

void hi_efusec_set_pgm_count(unsigned val)
{
    unsigned int ret    = 0;

    ret = hi_efuse_set_reg(val, HI_PGM_COUNT_OFFSET, HI_EFUSE_PGM_COUNT_LBIT, HI_EFUSE_PGM_COUNT_HBIT);
    if(ret){
        efuse_print_error("Set efuse pgm count faild\n");
    }

    return ;

}

void hi_efusec_set_pre_pg(unsigned val)
{
    unsigned int ret    = 0;

    ret = hi_efuse_set_reg(val, HI_EFUSEC_CFG_OFFSET, HI_EFUSE_PRE_PG_BIT, HI_EFUSE_PRE_PG_BIT);
    if(ret){
        efuse_print_error("Set efuse pre pg faild\n");
    }

    return ;

}

void hi_efusec_set_pg_en(unsigned val)
{
    unsigned int ret    = 0;

    ret = hi_efuse_set_reg(val, HI_EFUSEC_CFG_OFFSET, HI_EFUSE_PGEN_BIT, HI_EFUSE_PGEN_BIT);
    if(ret){
        efuse_print_error("Set efuse pg en faild\n");
    }

    return ;

}

/*��ѯ�Ƿ�ʹ����λ */
unsigned int hi_efusec_get_pgenb_status(void)
{
    unsigned int val = 0;
    int          ret = 0;

    ret = hi_efuse_get_reg(&val, HI_EFUSEC_STATUS_OFFSET, HI_EFUSE_PGENB_STAT_BIT, HI_EFUSE_PGENB_STAT_BIT);
    if(ret){
        efuse_print_error("get efuse pgenb status faild\n");
    }

    return val;

}

void hi_efusec_set_pg_value(unsigned int val)
{
    unsigned int ret    = 0;

    ret = hi_efuse_set_reg(val, HI_PG_VALUE_OFFSET, HI_EFUSE_PG_VALUE_LBIT, HI_EFUSE_PG_VALUE_HBIT);
    if(ret){
        efuse_print_error("Set efuse pg value faild\n");
    }

    return ;

}

unsigned int hi_efusec_get_pg_status(void)
{
    unsigned int val = 0;
    int          ret = 0;

    ret = hi_efuse_get_reg(&val, HI_EFUSEC_STATUS_OFFSET, HI_EFUSE_PG_STAT_BIT, HI_EFUSE_PG_STAT_BIT);
    if(ret){
        efuse_print_error("get efuse pg status faild\n");
    }

    return val;

}

/*lint -save -e718 -e746*/
/*****************************************************************************
* �� �� ��  : bsp_efuse_read
*
* ��������  : �����ȡEFUSE�е�����
*
* �������  : group  ��ʼgroup
*                   num  ���鳤��(word��,������16)
* �������  : pBuf ��EFUSE�е�����
*
* �� �� ֵ  :
*
* ����˵��  :
*
*****************************************************************************/
int bsp_efuse_read(u32* pBuf, const u32 group, const u32 num)
{
    u32* pSh = 0;
    u32 cnt = 0;
    int ret = EFUSE_ERROR;

    unsigned long flags = 0;
    struct clk *efuse_clk = NULL;

    if((0 == num) || (group + num > EFUSE_MAX_SIZE) || (NULL == pBuf))
    {
        efuse_print_error(" parameter error, group=%d, num=%d, pBuf=0x%x.\n", group, num, pBuf);
        return ret;
    }

    efuse_clk = (struct clk *)clk_get(NULL,"efuse_clk");
    if(IS_ERR(efuse_clk))
    {
        efuse_print_error("efuse clk cannot get, 0x%x.\n", efuse_clk);
        return ret;
    }

    ret = clk_prepare(efuse_clk);
    if (ret) {
        efuse_print_error("efuse clk prepare error, 0x%x.\n", (u32)ret);
        goto clk_prepare_error;
    }

    if(0 != clk_enable(efuse_clk))
    {
        efuse_print_error("efuse clk enable is fail, 0x%x.\n", efuse_clk);
        goto clk_enable_error;
    }

    spin_lock_irqsave(&efuse_lock, flags);

    /*�ж��Ƿ���power_down״̬*/
    if(hi_efuse_get_pd_status())
    {
       /*�˳�powerd_down״̬*/
        hi_efusec_set_pd_en();

        /* �ȴ�powerd_down�˳���־���óɹ�������ʱ���ش��� */
        if(EFUSE_OK != test_bit_value(hi_efuse_get_pd_status, 0))
        {
            efuse_print_error(" powerd_down disable is fail .\n");
            goto efuse_rd_error;
        }
    }

    /*ѡ���ź�Ϊapb����efuse*/
    hi_efusec_cfg_aib_sel();

    pSh = pBuf;

    /* ѭ����ȡefuseֵ */
    for(cnt = 0; cnt < num; cnt++)
    {
        /* ���ö�ȡ��ַ */
        hi_efusec_set_group( group+cnt);

        /* ʹ�ܶ� */
        hi_efusec_set_read_en();

        /* �ȴ������ */
        if(EFUSE_OK != test_bit_value(hi_efusec_get_read_status, 1))
        {
            efuse_print_error(" read finish is fail .\n");
            goto efuse_rd_error;
        }
        /* ��ȡ���� */
        *pSh = hi_efusec_get_data();
        pSh++;

    }

    ret = EFUSE_OK;

efuse_rd_error:
    spin_unlock_irqrestore(&efuse_lock, flags);
    clk_disable(efuse_clk);

clk_enable_error:
    clk_unprepare(efuse_clk);

clk_prepare_error:
    clk_put(efuse_clk);

    return ret;
}
/*lint -restore*/

/*****************************************************************************
* �� �� ��  : bsp_efuse_write
*
* ��������  : ��дEfsue
*
* �������  : pBuf:����д��EFUSEֵ
*                 group,Efuse��ַƫ��
*                 len,��д����
* �������  :
*
* �� �� ֵ  :
*
* ����˵��  :
*
*****************************************************************************/
int bsp_efuse_write( u32 *pBuf, const u32 group, const u32 len )
{
    u32 i = 0;
    u32 u32Grp = 0;
    u32 *pu32Value = 0;

    int ret = EFUSE_ERROR;

    unsigned long flags = 0;
    struct clk *efuse_clk = NULL;
    struct regulator *efuse_regulator = NULL;

    if((0 == len) || (group + len > EFUSE_MAX_SIZE) || (NULL == pBuf))
    {
        efuse_print_error(" parameter error, group=%d, len=%d, pBuf=0x%x.\n", group, len, pBuf);
        return ret;
    }

    efuse_regulator = regulator_get(NULL, "EFUSE-vcc");
    if(NULL == efuse_regulator)
    {
        efuse_print_error("efuse regulator cannot get. \n");
        return ret;
    }

    efuse_clk = (struct clk *)clk_get(NULL,"efuse_clk");
    if(IS_ERR(efuse_clk))
    {
        efuse_print_error("efuse clk cannot get, 0x%x.\n", efuse_clk);
        goto clk_get_error;
    }

    ret = clk_prepare(efuse_clk);
    if (ret) {
        efuse_print_error("efuse clk prepare error, 0x%x.\n", (u32)ret);
        goto clk_prepare_error;
    }

    spin_lock_irqsave(&efuse_lock, flags);

    if(0 != regulator_enable(efuse_regulator))
    {
        efuse_print_error("efuse regulator enable is fail. \n");
        goto regulator_enable_error;
    }

    if(0 != clk_enable(efuse_clk))
    {
        efuse_print_error("efuse clk enable is fail, 0x%x.\n", efuse_clk);
        goto clk_enable_error;
    }

    udelay(1000);/*lint !e737*/

    /*�ж��Ƿ���power_down״̬*/
    if(hi_efuse_get_pd_status())
    {
       /*�˳�powerd_down״̬*/
        hi_efusec_set_pd_en();
        /* �ȴ�powerd_down�˳���־���óɹ�������ʱ���ش��� */
        if(EFUSE_OK != test_bit_value(hi_efuse_get_pd_status, 0))
        {
            efuse_print_error(" power_down disable is fail .\n");
            goto efuse_write_error;
        }
    }

    /* �ж��Ƿ�������д,Ϊ1��ֹ��д,ֱ���˳� */
    if(hi_efusec_get_pgm_disable_flag())
    {
        efuse_print_error(" burn write is disable .\n");
        goto efuse_write_error;
    }

    /*ѡ���ź�Ϊapb����efuse*/
    hi_efusec_cfg_aib_sel();

    /* ����ʱ��Ҫ�� */
    hi_efusec_set_count(EFUSE_COUNT_CFG);
    hi_efusec_set_pgm_count(PGM_COUNT_CFG);

    /* ʹ��Ԥ��д */
    hi_efusec_set_pre_pg(1);

    /*��ѯ�Ƿ�ʹ����λ */
    if(EFUSE_OK != test_bit_value(hi_efusec_get_pgenb_status, 1))
    {
        efuse_print_error(" pre write enable is fail .\n");
        goto efuse_write_error;
    }

    /* ѭ����д */
    u32Grp = group;
    pu32Value = pBuf;
    for (i = 0; i < len; i++)
    {
        /* ����group */
        hi_efusec_set_group(u32Grp);

        /* ����value */
        hi_efusec_set_pg_value(*pu32Value);

        /* ʹ����д */
        hi_efusec_set_pg_en(1);

        /* ��ѯ��д��� */
        if(EFUSE_OK != test_bit_value(hi_efusec_get_pg_status, 1))
        {
            efuse_print_error(" burn write finish is fail .\n");
            goto efuse_write_error;
        }

        /* ��д��һ�� */
        u32Grp++;
        pu32Value++;
    }

    /*ȥʹ��Ԥ��д*/
    hi_efusec_set_pre_pg(0);

    ret = EFUSE_OK;

efuse_write_error:
    clk_disable(efuse_clk);

clk_enable_error:
    (void)regulator_disable(efuse_regulator);
    clk_unprepare(efuse_clk);

regulator_enable_error:
    spin_unlock_irqrestore(&efuse_lock, flags);

clk_prepare_error:
    clk_put(efuse_clk);

clk_get_error:
    regulator_put(efuse_regulator);

    return ret;
}

static int __init hi_efuse_init(void)
{
    void *addr = NULL;
    struct clk *efuse_clk = NULL;

    struct device_node *dev = NULL;
    const char name[] = "hisilicon,efuse_balong_app";

    dev = of_find_compatible_node(NULL, NULL, name);
    if(NULL == dev)
    {
        efuse_print_error("device node not found\n");
        return -1;
    }

    addr = of_iomap(dev, 0);
    if (0 == addr)
    {
        efuse_print_error("remap addr fail\n");
        return -1;
    }

    efuse_base_addr = addr;

    spin_lock_init(&efuse_lock);

    efuse_clk = (struct clk *)clk_get(NULL,"efuse_clk");
    if (IS_ERR(efuse_clk)) {
        efuse_print_error("efuse clk cannot get, 0x%x.\n", efuse_clk);
        return EFUSE_ERROR;
    }
    (void)clk_prepare(efuse_clk);
    (void)clk_enable(efuse_clk);
    clk_disable(efuse_clk);
    clk_unprepare(efuse_clk);
    clk_put(efuse_clk);

    efuse_print_info("efuse init ok.\n");

    return EFUSE_OK;
}


void bsp_efuse_show(void)
{
    unsigned int i = 0;
    unsigned int value = 0;

    for(i = 0;i < EFUSE_MAX_SIZE;i++)
    {
        if(OK == bsp_efuse_read(&value,i, 1))
        {
            efuse_print_info("efuse group%d value = 0x%x.\n ", i, value);
        }
        else
        {
            efuse_print_error("efuse group%d read fail.\n", i);
            return;
        }
    }
}

EXPORT_SYMBOL(bsp_efuse_show);
EXPORT_SYMBOL(bsp_efuse_read);
EXPORT_SYMBOL(bsp_efuse_write);

static void __exit hi_efuse_exit(void)
{
    /* do nothing */
}

module_init(hi_efuse_init);
module_exit(hi_efuse_exit);
MODULE_AUTHOR("Hisilicon Drive Group");
MODULE_DESCRIPTION("anten driver for the Hisilicon anten plug in/out");
MODULE_LICENSE("GPL");


