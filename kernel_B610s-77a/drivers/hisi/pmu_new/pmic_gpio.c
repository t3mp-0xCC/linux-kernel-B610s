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

#include <osl_spinlock.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <bsp_pmu.h>
#include <bsp_pmu_gpio.h>

 PMU_GPIO_INFO_S      g_pmu_gpio_info;

void pmic_gpio_reg_write(u32 addr, u32 value)
{
    writel(value,(void*)(g_pmu_gpio_info.base + (addr << 2)));
}

void  pmic_gpio_reg_read(u32 addr, u32 *pValue)
{
    *pValue = readl((void*)(g_pmu_gpio_info.base + (addr << 2)));
}

void pmic_gpio_reg_show(u32 addr)
{
    pgpio_print_error("addr : 0x%x, value : 0x%x\n", addr, readl((void*)(g_pmu_gpio_info.base + (addr << 2))) );
}
/*****************************************************************************
 函 数  : bsp_pmu_gpio_direction_output
 功 能  : 配置pmu gpio 输出的电平
 输 入  : gpio编号,输出的电平值
 输 出  : 无
 返 回  : 配置成功或失败

*****************************************************************************/
int bsp_pmu_gpio_direction_output(unsigned int gpio_num, unsigned int value)
{
    int ret = 0;
    unsigned long flag = 0;

    /*check para is valid*/
    if(gpio_num >= g_pmu_gpio_info.num){
        pgpio_print_error("pmu gpio direction output para error, gpio_num is %d\n",gpio_num);
        return  -1;
    }

    if(!bsp_pmic_ioshare_status_get(GPIO_FUNC0 + gpio_num))
    {
        pgpio_print_error("pmu gpio num:%d, mux is not suport!\n");
        return -1;
    }

    /*lock*/
    spin_lock_irqsave(&g_pmu_gpio_info.lock, flag);

    /*set direction output*/
    pmic_gpio_reg_write(g_pmu_gpio_info.node[gpio_num].dir_reg, PMIC_GPIO_OUTPUT);

    /*set data regiter*/
    pmic_gpio_reg_write(g_pmu_gpio_info.node[gpio_num].data_reg, (unsigned int) !!value);

    /*unlock*/
    spin_unlock_irqrestore(&g_pmu_gpio_info.lock, flag);

    return ret;
}

/*****************************************************************************
 函 数  : bsp_pmu_gpio_direction_input
 功 能  : 配置pmu gpio 方向为输入
 输 入  : gpio编号
 输 出  : 无
 返 回  : 配置成功或失败

*****************************************************************************/
int bsp_pmu_gpio_direction_input(unsigned int gpio_num)
{
    int ret = 0;
    unsigned long flag = 0;

    /*check para is valid*/
    if(gpio_num >= g_pmu_gpio_info.num){
        pgpio_print_error("pmu gpio direction output para error, gpio_num is %d\n",gpio_num);
        return  -1;
    }

    if(!bsp_pmic_ioshare_status_get(GPIO_FUNC0 + gpio_num))
    {
        pgpio_print_error("pmu gpio num:%d, mux is not suport!\n");
        return -1;
    }

    /*lock*/
    spin_lock_irqsave(&g_pmu_gpio_info.lock, flag);

    /*set direction input*/
    pmic_gpio_reg_write(g_pmu_gpio_info.node[gpio_num].dir_reg, PMIC_GPIO_INPUT);

    /*unlock*/
    spin_unlock_irqrestore(&g_pmu_gpio_info.lock, flag);

    return ret;
}

/*****************************************************************************
 函 数  : bsp_pmu_gpio_set_value
 功 能  : 配置pmu gpio 电平
 输 入  : gpio编号
 输 出  : 无
 返 回  : 配置成功或失败

*****************************************************************************/
int bsp_pmu_gpio_set_value(unsigned int gpio_num, unsigned int value)
{
    int ret = 0;
    unsigned long flag = 0;

    /*check para is valid*/
    if(gpio_num >= g_pmu_gpio_info.num){
        pgpio_print_error("pmu gpio direction output para error, gpio_num is %d\n", gpio_num);
        return  -1;
    }

    if(!bsp_pmic_ioshare_status_get(GPIO_FUNC0 + gpio_num))
    {
        pgpio_print_error("pmu gpio num:%d, mux is not suport!\n");
        return -1;
    }
    /*lock*/
    spin_lock_irqsave(&g_pmu_gpio_info.lock, flag);

    /*set data regiter*/
    pmic_gpio_reg_write(g_pmu_gpio_info.node[gpio_num].data_reg, (unsigned int) !!value);

    /*unlock*/
    spin_unlock_irqrestore(&g_pmu_gpio_info.lock, flag);

    return ret;
}


/*****************************************************************************
 函 数  : bsp_pmu_gpio_get_value
 功 能  : 获取pmu gpio 电平
 输 入  : gpio编号
 输 出  : 无
 返 回  : 配置成功或失败

*****************************************************************************/
int bsp_pmu_gpio_get_value(unsigned int gpio_num)
{
    int ret = 0;
    u32 value = 0;
    unsigned long flag = 0;

    /*check para is valid*/
    if(gpio_num >= g_pmu_gpio_info.num){
        pgpio_print_error("pmu gpio direction output para error, gpio_num is %d\n",gpio_num);
        ret = -1;
        return ret;
    }

    if(!bsp_pmic_ioshare_status_get(GPIO_FUNC0 + gpio_num))
    {
        pgpio_print_error("pmu gpio num:%d, mux is not suport!\n");
        return -1;
    }

    /*lock*/
    spin_lock_irqsave(&g_pmu_gpio_info.lock, flag);

    /*get data regiter*/
    pmic_gpio_reg_read(g_pmu_gpio_info.node[gpio_num].data_reg,  &value);

    /*unlock*/
    spin_unlock_irqrestore(&g_pmu_gpio_info.lock, flag);

    return (int)!!value;
}

static void pmu_gpio_set_irq_trigger_type(int gpio, unsigned long flags)
{
    u32 gpioiev = 0;
    u32 gpiois  = 0;
    u32 gpioibe = 0;
    int   offset = gpio % 8;
    unsigned long flag = 0;

    /*check para is valid*/
    if(gpio >= g_pmu_gpio_info.num){
        pgpio_print_error("pmu gpio direction output para error, gpio_num is %d\n", g_pmu_gpio_info.num);
        return ;
    }

    /*lock*/
    spin_lock_irqsave(&g_pmu_gpio_info.lock, flag);

    pmic_gpio_reg_read(g_pmu_gpio_info.iev_reg, &gpioiev);
    pmic_gpio_reg_read(g_pmu_gpio_info.is_reg,  &gpiois);
    pmic_gpio_reg_read(g_pmu_gpio_info.ibe_reg, &gpioibe);

    if (flags & (IRQ_TYPE_LEVEL_HIGH | IRQ_TYPE_LEVEL_LOW)) {
        gpiois |= (u32)1 << offset;
        if (flags & IRQ_TYPE_LEVEL_HIGH)
            gpioiev |= (u32)1 << offset;
        else
            gpioiev &= ~((u32)1 << offset);
    } else
        gpiois &= ~((u32)1 << offset);
    pmic_gpio_reg_write(g_pmu_gpio_info.is_reg, gpiois);

    if ((flags & IRQ_TYPE_EDGE_BOTH) == IRQ_TYPE_EDGE_BOTH)
        gpioibe |= (u32)1 << offset;
    else {
        gpioibe &= ~((u32)1 << offset);
        if (flags & IRQ_TYPE_EDGE_RISING)
            gpioiev |= (u32)1 << offset;
        else if (flags & IRQ_TYPE_EDGE_FALLING)
            gpioiev &= ~((u32)1 << offset);
    }
    pmic_gpio_reg_write(g_pmu_gpio_info.ibe_reg, gpioibe);

    pmic_gpio_reg_write(g_pmu_gpio_info.iev_reg, gpioiev);

    /*unlock*/
    spin_unlock_irqrestore(&g_pmu_gpio_info.lock, flag);


    return ;
}

/*****************************************************************************
 函 数  : bsp_pmu_gpio_request_irq
 功 能  : 挂接pmu gpio 中断
 输 入  : 
                 gpio_num: gpio编号
                 handler   : 回调
 输 出  : 无
 返 回  : 配置成功或失败

*****************************************************************************/
int bsp_pmu_gpio_request_irq(unsigned int gpio_num,  pmufuncptr handler, unsigned long flags,  const char *name, void *bdata)
{
    int ret = 0;

    /*check para is valid*/
    if(gpio_num >= g_pmu_gpio_info.num){
        pgpio_print_error("pmu gpio direction output para error, gpio_num is %d\n",gpio_num);
        ret = -1;
        return ret;
    }

    if(!bsp_pmic_ioshare_status_get(GPIO_FUNC0 + gpio_num))
    {
        pgpio_print_error("pmu gpio num:%d, mux is not suport!\n");
        return -1;
    }

    g_pmu_gpio_info.node[gpio_num].irq_handler = handler;
    g_pmu_gpio_info.node[gpio_num].name         = name;

    /*set irq trigger type*/
    pmu_gpio_set_irq_trigger_type(gpio_num, flags);

    /*regist pmu handler*/
    ret = bsp_pmu_irq_callback_register(g_pmu_gpio_info.node[gpio_num].irq_num, handler, bdata);

    return ret;
}

/*****************************************************************************
 函 数  : bsp_pmu_gpio_irq_mask
 功 能  : pmu gpio 中断屏蔽
 输 入  : pmu gpio 管脚编号
 输 出  : 无
 返 回  : 配置成功或失败

*****************************************************************************/
int bsp_pmu_gpio_irq_mask(unsigned int gpio_num)
{
    int ret= 0;
    u32 value = 0;
    unsigned long flag = 0;

    /*check para is valid*/
    if(gpio_num >= g_pmu_gpio_info.num){
        pgpio_print_error("pmu gpio direction output para error, gpio_num is %d\n",gpio_num);
        ret = -1;
        return ret;
    }

    if(!bsp_pmic_ioshare_status_get(GPIO_FUNC0 + gpio_num))
    {
        pgpio_print_error("pmu gpio num:%d, mux is not suport!\n");
        return -1;
    }

     /*lock*/
    spin_lock_irqsave(&g_pmu_gpio_info.lock, flag);

    pmic_gpio_reg_read(g_pmu_gpio_info.irq_mask_reg, &value);
    value |= 1 << gpio_num;
    pmic_gpio_reg_write(g_pmu_gpio_info.irq_mask_reg, value);

    /*unlock*/
    spin_unlock_irqrestore(&g_pmu_gpio_info.lock, flag);

    return 0;
}

/*****************************************************************************
 函 数  : bsp_pmu_gpio_irq_mask
 功 能  : pmu gpio 中断去屏蔽
 输 入  : pmu gpio 管脚编号
 输 出  : 无
 返 回  : 配置成功或失败

*****************************************************************************/
int bsp_pmu_gpio_irq_unmask(unsigned int gpio_num)
{
    int ret= 0;
    u32 value = 0;
    unsigned long flag = 0;

    /*check para is valid*/
    if(gpio_num >= g_pmu_gpio_info.num){
        pgpio_print_error("pmu gpio direction output para error, gpio_num is %d\n",gpio_num);
        ret = -1;
        return ret;
    }


    if(!bsp_pmic_ioshare_status_get(GPIO_FUNC0 + gpio_num))
    {
        pgpio_print_error("pmu gpio num:%d, mux is not suport!\n");
        return -1;
    }

     /*lock*/
    spin_lock_irqsave(&g_pmu_gpio_info.lock, flag);

    pmic_gpio_reg_read(g_pmu_gpio_info.irq_mask_reg, &value);
    value &= ~(1 << gpio_num);
    pmic_gpio_reg_write(g_pmu_gpio_info.irq_mask_reg, value);

    /*unlock*/
    spin_unlock_irqrestore(&g_pmu_gpio_info.lock, flag);

    return 0;

}

/*****************************************************************************
 函 数  : pmu_gpio_dts_init
 功 能  : pmu gpio dts 初始化
 输 入  : 
 输 出  : 无
 返 回  : 配置成功或失败

*****************************************************************************/
int pmu_gpio_dts_init(void)
{
    int ret = 0;
    int i = 0;
    struct device_node *dev_node   = NULL;
    struct device_node *child_node = NULL;
    const char         *name       ="hisilicon,pmic_gpio_app";

    /*get dts info*/
    dev_node = of_find_compatible_node(NULL, NULL, name);
    if(NULL == dev_node)
    {
        pgpio_print_error("pmu gpio device node not found!\n");
        return -1;
    }

    /*get GPIOIS register addr*/
    ret  = of_property_read_u32_index(dev_node, "is_reg", 0, &g_pmu_gpio_info.is_reg);
    if(ret){
        pgpio_print_error("pmu gpio is_reg read error!ret =%d\n", ret);
        return ret;
    }

   /*get GPIOIBE register addr*/
    ret  = of_property_read_u32_index(dev_node, "ibe_reg", 0, &g_pmu_gpio_info.ibe_reg);
    if(ret){
        pgpio_print_error("pmu gpio ibe_reg read error!ret =%d\n", ret);
        return ret;
    }

    /*get GPIOIEV register addr*/
    ret  = of_property_read_u32_index(dev_node, "iev_reg", 0, &g_pmu_gpio_info.iev_reg);
    if(ret){
        pgpio_print_error("pmu gpio iev_reg get error!ret =%d\n", ret);
        return ret;
    }

    /*get GPIO PIN num*/
    ret = of_property_read_u32_index(dev_node, "gpio_num", 0, &g_pmu_gpio_info.num);
    if(ret){
        pgpio_print_error("pmu gpio gpio num  error!ret =%d\n", ret);
        return ret;
    }

    /*malloc memory*/
    g_pmu_gpio_info.node = kmalloc(g_pmu_gpio_info.num * sizeof(PMU_GPIO_NODE_S), GFP_KERNEL);
    if(NULL == g_pmu_gpio_info.node)
    {
        pgpio_print_error("pmu gpio malloc memory fail, mem size is %d\n", g_pmu_gpio_info.num * sizeof(PMU_GPIO_NODE_S));
        return  -1;
    }

    /*get gpio node info*/
    for_each_child_of_node(dev_node, child_node)
    {
        /*get GPIODIR register addr*/
        ret = of_property_read_u32_index(child_node, "dir_reg", 0, &g_pmu_gpio_info.node[i].dir_reg);
        if(ret)
        {
            pgpio_print_error("get dir reg fail, i=%d,ret=%d\n", i, ret);
            goto out;
        }

        /*get GPIODATA register addr*/
        ret = of_property_read_u32_index(child_node, "data_reg", 0, &g_pmu_gpio_info.node[i].data_reg);
        if(ret)
        {
            pgpio_print_error("get data reg fail, i=%d,ret=%d\n", i, ret);
            goto out;
        }

        /*get irq num*/
        ret = of_property_read_u32_index(child_node, "irq_num", 0, &g_pmu_gpio_info.node[i].irq_num);
        if(ret)
        {
            pgpio_print_error("get irq_num fail, i=%d,ret=%d\n", i, ret);
            goto out;
        }
        i++;
    }

    return 0;
out: 
    kfree(g_pmu_gpio_info.node);
    return ret;

}

/*****************************************************************************
 函 数  : bsp_pmu_gpio_init
 功 能  : pmu gpio 初始化
 输 入  : 
 输 出  : 无
 返 回  : 配置成功或失败

*****************************************************************************/
int bsp_pmu_gpio_init(void)
{
    int ret = 0;
    /*lock init*/
    spin_lock_init(&g_pmu_gpio_info.lock);

    /*dts init*/
    ret = pmu_gpio_dts_init();
    if(ret){
        pgpio_print_error("pmu gpio get dts error!\n");
	 return ret;
    }

   g_pmu_gpio_info.base = bsp_pmic_get_base_addr();
   if(!g_pmu_gpio_info.base){
        pgpio_print_error("pmu gpio get base addr fail, base is 0x%x!\n", g_pmu_gpio_info.base);
        return -1;
   }
   
    return 0;
}

/*****************************************************************************
 函 数  : bsp_pmu_gpio_init
 功 能  : pmu gpio 初始化
 输 入  : 
 输 出  : 无
 返 回  : 配置成功或失败

*****************************************************************************/
void bsp_pmu_gpio_exit(void)
{
    return;
}

module_exit(bsp_pmu_gpio_exit);
module_init(bsp_pmu_gpio_init);

