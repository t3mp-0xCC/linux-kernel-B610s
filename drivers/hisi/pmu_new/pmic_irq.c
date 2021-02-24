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
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/bitops.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>
#include <linux/slab.h>
#include <linux/irqnr.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include "osl_spinlock.h"
#include <osl_malloc.h>
#include <soc_interrupts.h>

#include <bsp_vic.h>

#include "pmic_inner.h"
#include "pmic_volt.h"
#include "pmic_irq.h"

struct pmic_irq *g_pmic_irqinfo;/*中断全局结构体*/

static __inline__ void pmic_irq_enable(void)
{
    enable_irq(g_pmic_irqinfo->irq);
}
static __inline__ void pmic_irq_disable(void)
{
    disable_irq_nosync(g_pmic_irqinfo->irq);
}

/*查看当前中断状态*/
void pmic_irq_mask(unsigned int irq)
{
    pmic_irqflags_t irq_flags = 0;
    u32 reg_addr,reg_bit,reg_temp = 0;

    if (irq > g_pmic_irqinfo->irq_nums) {
        pmic_print_error("pmic irq is error,please check irq=%d\n", (int)irq);
        return;
    }

    reg_addr = g_pmic_irqinfo->irq_reg.irq[irq >> 3];
    reg_bit = irq & 0x7;

    spin_lock_irqsave(&g_pmic_irqinfo->irq_lock,irq_flags);
    pmic_reg_read(reg_addr,&reg_temp);
    reg_temp |= ((u32)1 << reg_bit);
    pmic_reg_write(reg_addr,reg_temp);
    spin_unlock_irqrestore(&g_pmic_irqinfo->irq_lock,irq_flags);
}
void pmic_irq_unmask(unsigned int irq)
{
    pmic_irqflags_t irq_flags = 0;
    u32 reg_addr,reg_bit,reg_temp = 0;

    if (irq > g_pmic_irqinfo->irq_nums) {
        pmic_print_error("pmic irq is error,please check irq=%d\n", (int)irq);
        return;
    }

    reg_addr = g_pmic_irqinfo->irq_reg.irq[irq >> 3];
    reg_bit = irq & 0x7;

    spin_lock_irqsave(&g_pmic_irqinfo->irq_lock,irq_flags);
    pmic_reg_read(reg_addr,&reg_temp);
    reg_temp &= ~((u32)1 << reg_bit);
    pmic_reg_write(reg_addr,reg_temp);
    spin_unlock_irqrestore(&g_pmic_irqinfo->irq_lock,irq_flags);
}
int pmic_irq_is_masked(unsigned int irq)
{
    pmic_irqflags_t irq_flags = 0;
    u32 reg_addr,reg_bit,reg_temp = 0;

    if (irq > g_pmic_irqinfo->irq_nums) {
        pmic_print_error("pmic irq is error,please check irq=%d\n", (int)irq);
        return PMIC_IRQ_ERROR;
    }

    reg_addr = g_pmic_irqinfo->irq_reg.irq[irq >> 3];
    reg_bit = irq & 0x7;

    spin_lock_irqsave(&g_pmic_irqinfo->irq_lock,irq_flags);
    pmic_reg_read(reg_addr,&reg_temp);
    spin_unlock_irqrestore(&g_pmic_irqinfo->irq_lock,irq_flags);

    return  !!(reg_temp & ((u32)1 << reg_bit));
}


int pmic_irq_callback_register(unsigned int irq, pmicfuncptr routine, void *data)
{
    if (irq >= g_pmic_irqinfo->irq_nums) {
        pmic_print_error("pmic irq is error,please check irq=%d\n", (int)irq);
        return  PMIC_IRQ_ERROR;
    }

    g_pmic_irqinfo->irq_handler[irq].irq_num = irq;
    g_pmic_irqinfo->irq_handler[irq].routine = routine;
    g_pmic_irqinfo->irq_handler[irq].data = data;

    return  PMIC_IRQ_OK;
}

void pmic_irq_wk_handler(void)
{
    u32 pending = 0;
    u8 i , offset = 0;
    unsigned long ppending = 0;
    struct pmic_irq_special *irq_special = &g_pmic_irqinfo->irq_special;
    struct pmic_irq_reg *irq_reg = &g_pmic_irqinfo->irq_reg;
    u32 key_pending = irq_special->key_pending;
    u32 keyup_irq = irq_special->keyup_irq;
    u32 keydown_irq = irq_special->keydown_irq;

    for (i = 0; i < irq_reg->irqarrays; i++)
    {
        pmic_reg_read(irq_reg->irq[i],&pending);
        if (pending != 0)
        {
            pmic_print_info("pending[%d]=0x%x\n\r", i, pending);
        }

        if (i == irq_special->sim_arry)
        {
            if(pending & (0x1 << irq_special->sim0_bit))
                pmic_volt_disable(irq_special->sim0_ldo);
            if(pending & (0x1 << irq_special->sim1_bit))
                pmic_volt_disable(irq_special->sim1_ldo);
        }

        /*clear int*/
        pmic_reg_write(irq_reg->irq[i] , pending);
        /* 按键中断press和release中断同时发生，先处理press中断，这样更符合逻辑 */
        if ((i == irq_special->key_arry)&&
            ((pending & key_pending) == key_pending))
        {
            g_pmic_irqinfo->irq_handler[keydown_irq].cnt++;
            g_pmic_irqinfo->irq_handler[keyup_irq].cnt++;
            if(g_pmic_irqinfo->irq_handler[keydown_irq].routine)
                g_pmic_irqinfo->irq_handler[keydown_irq].routine(g_pmic_irqinfo->irq_handler[keydown_irq].data);
            if(g_pmic_irqinfo->irq_handler[keyup_irq].routine)
                g_pmic_irqinfo->irq_handler[keyup_irq].routine(g_pmic_irqinfo->irq_handler[keyup_irq].data);
            pending &= (~irq_special->key_pending);
        }
        if (pending)
        {
            ppending = (unsigned long)pending;
            for_each_set_bit(offset, &ppending, 8)
            {
                g_pmic_irqinfo->irq_handler[offset+i*8].cnt++;
                if(g_pmic_irqinfo->irq_handler[offset+i*8].routine)
                    g_pmic_irqinfo->irq_handler[offset+i*8].routine(g_pmic_irqinfo->irq_handler[offset+i*8].data);
            }
			pmic_print_info("reg:0x%x,data:0x%x\n",irq_reg->irq[i], pending);
        }	
    }
    pmic_print_info("**********pmic_irq_wk_handler**********\n");
    pmic_irq_enable();
}


irqreturn_t pmic_irq_handler(int irq, void *data)
{
    struct pmic_irq *pmic_irq = (struct pmic_irq*)data;

    /*关闭中断,中断处理任务完成后打开*/
    pmic_irq->cnts++;
    pmic_irq_disable();
    pmic_print_info("**********pmic_irq_handler**********\n");
    queue_work(pmic_irq->irq_wq, &pmic_irq->irq_wk);

    return IRQ_HANDLED;
}

static int __init pmic_irq_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    int index;
    u32 regvalue;
    int  ret = 0;
    u32 *reg_temp;
    u32 irq_data_width = 0;

    g_pmic_irqinfo = kzalloc(sizeof(struct pmic_irq), GFP_KERNEL);
    if (g_pmic_irqinfo == NULL) {
        pmic_print_error("pmic irq kzalloc is failed,please check!\n");
        return -ENOMEM;
    }

    /* 内存映射，获得中断*/
	/* coverity[overflow_const] */
    g_pmic_irqinfo->irq = (u32)irq_of_parse_and_map(np, 0);
    spin_lock_init(&g_pmic_irqinfo->irq_lock);/*中断只在Acore实现，多core*/

    /*read dts*/
    ret = of_property_read_u32(np, "irq_data_width", &irq_data_width);
    ret |= of_property_read_u32_index(np,"arrys",0,&(g_pmic_irqinfo->irq_reg.irqarrays));
    ret |= of_property_read_u32_index(np,"arrys",1,&(g_pmic_irqinfo->irq_mask.maskarrys));
    ret |= of_property_read_u32_index(np,"arrys",2,&(g_pmic_irqinfo->irq_reg.ocparrays));
    if(ret)
    {
       pmic_print_error("read arrays from dts failed,err id %d!\n",ret);
       return ret;
    }
    g_pmic_irqinfo->irq_nums = irq_data_width * g_pmic_irqinfo->irq_reg.irqarrays;
    
    /*malloc*/
    g_pmic_irqinfo->irq_reg.irq = kzalloc(g_pmic_irqinfo->irq_reg.irqarrays*sizeof(u32), GFP_KERNEL);
    if(g_pmic_irqinfo->irq_reg.irq == NULL)
    {
        pmic_print_error("pmic irq reg kzalloc is failed,please check!\n");
    	goto out_irqinfo;
    }
    
    g_pmic_irqinfo->irq_mask.mask = kzalloc(g_pmic_irqinfo->irq_mask.maskarrys*sizeof(u32), GFP_KERNEL);
    if(g_pmic_irqinfo->irq_mask.mask == NULL)
    {
        pmic_print_error("pmic irq reg kzalloc is failed,please check!\n");
        goto out_irq;
    }
    
    g_pmic_irqinfo->irq_reg.ocp_irq = kzalloc(g_pmic_irqinfo->irq_reg.ocparrays*sizeof(u32), GFP_KERNEL);
    if(g_pmic_irqinfo->irq_reg.ocp_irq ==NULL)
    {
        pmic_print_error("pmic irq reg kzalloc is failed,please check!\n");
        goto out_mask;
    }
    
    ret = of_property_read_u32_array(np, "irq_reg", &(g_pmic_irqinfo->irq_reg.irq[0]),g_pmic_irqinfo->irq_reg.irqarrays);
    ret |= of_property_read_u32_array(np, "irq_mask_reg", &(g_pmic_irqinfo->irq_mask.mask[0]),g_pmic_irqinfo->irq_mask.maskarrys);
    ret |= of_property_read_u32_array(np, "ocp_irq_reg", &(g_pmic_irqinfo->irq_reg.ocp_irq[0]),g_pmic_irqinfo->irq_reg.ocparrays);
    ret |= of_property_read_u32_array(np, "irq_special", &(g_pmic_irqinfo->irq_special.sim_arry),sizeof(struct pmic_irq_special)/sizeof(u32));

    if(ret)
    {
       pmic_print_error("read irq nums or reg from dts failed,err id %d!\n",ret);
       return ret;
    }
    /*malloc*/
    g_pmic_irqinfo->irq_handler = kzalloc(g_pmic_irqinfo->irq_nums*sizeof(struct pmic_irq_handle), GFP_KERNEL);
    if(g_pmic_irqinfo->irq_handler == NULL)
    {
        pmic_print_error("pmic irq handler kzalloc is failed,please check!\n");
        goto out_ocpirq;
    }

    /*初始化工作队列*/
    g_pmic_irqinfo->irq_wq = create_singlethread_workqueue("bsp_pmic_irq");
    INIT_WORK(&g_pmic_irqinfo->irq_wk, (void *)pmic_irq_wk_handler);

    /*清除所有中断寄存器*/
    for(index = 0; index < g_pmic_irqinfo->irq_reg.irqarrays; index++)
    {
        pmic_reg_read(g_pmic_irqinfo->irq_reg.irq[index],&regvalue);
        pmic_reg_write(g_pmic_irqinfo->irq_reg.irq[index], regvalue);
    }
    for(index = 0; index < g_pmic_irqinfo->irq_reg.ocparrays; index++)
    {
        pmic_reg_read(g_pmic_irqinfo->irq_reg.ocp_irq[index],&regvalue);
        pmic_reg_write(g_pmic_irqinfo->irq_reg.ocp_irq[index], regvalue);
    }

    reg_temp = kzalloc(g_pmic_irqinfo->irq_mask.maskarrys*sizeof(u32), GFP_KERNEL);
	if(reg_temp == NULL)
	{
	    pmic_print_error("reg_temp for irq mask kzalloc is failed,please check!\n");
        goto out_irqhandler;
	}
    /*mask所有中断*/
    for(index = 0; index < g_pmic_irqinfo->irq_mask.maskarrys; index++)
    {
        pmic_reg_read(g_pmic_irqinfo->irq_mask.mask[index], &reg_temp[index]);
        pmic_reg_write(g_pmic_irqinfo->irq_mask.mask[index], 0xff);
    }
        /* 中断注册 */
    /*connect and enable IRQ*/
    ret = request_irq((g_pmic_irqinfo->irq), pmic_irq_handler, IRQF_DISABLED,
            "pmic_irq", g_pmic_irqinfo);
    if (ret < 0) {
        pmic_print_error("unable to request PMIC pmic IRQ!\n");
        goto out;
    } 

    /*unmask所有中断*/
    for(index = 0; index < g_pmic_irqinfo->irq_mask.maskarrys; index++)
    {
        pmic_reg_write(g_pmic_irqinfo->irq_mask.mask[index], reg_temp[index]);
    }

    /*set pmic_irq_data in order to get at last*/
    platform_set_drvdata(pdev, g_pmic_irqinfo);
    pmic_print_error("pmic_irq_probe ok !\n");
	kfree(reg_temp);

    return 0;
out:
    kfree(reg_temp);
out_irqhandler:
    kfree(g_pmic_irqinfo->irq_handler);
out_ocpirq:
    kfree(g_pmic_irqinfo->irq_reg.ocp_irq);
out_mask:            
    kfree(g_pmic_irqinfo->irq_mask.mask);
out_irq:
    kfree(g_pmic_irqinfo->irq_reg.irq);
out_irqinfo:   
    kfree(g_pmic_irqinfo);
    return ret;

}
static  int pmic_irq_remove(struct platform_device *pdev)
{
    struct pmic_irq_data *pmic_irq_data_pt;
    /*get pmic_irq_data*/
    pmic_irq_data_pt = platform_get_drvdata(pdev);
    if (NULL == pmic_irq_data_pt) {
        pr_err("%s %d platform_get_drvdata NULL\n", __func__, __LINE__);
        return -1;
    }

    /*release memory*/
    kfree(pmic_irq_data_pt);

    return 0;
}

static const struct of_device_id pmic_irq_of_match[] = {
    { .compatible = "hisilicon,pmu_pmic_app"},
    {},
};
MODULE_DEVICE_TABLE(of, pmic_irq_of_match);

static struct platform_driver pmic_irq_driver = {
    .probe = pmic_irq_probe,
    .remove = pmic_irq_remove,
    .driver     = {
        .name           = "pmic-irq",
        .of_match_table = of_match_ptr(pmic_irq_of_match),
    },
};
static int __init pmic_irq_init(void)
{
    int ret = 0;

    ret = platform_driver_register(&pmic_irq_driver);

    if (ret)
        printk("Failed to register pmic_irq driver or device: %d\n", ret);

    return ret;
}

static void __exit pmic_irq_exit(void)
{
    platform_driver_unregister(&pmic_irq_driver);
}
module_exit(pmic_irq_exit);
subsys_initcall(pmic_irq_init);
MODULE_LICENSE("GPL");

EXPORT_SYMBOL_GPL(g_pmic_irqinfo);
EXPORT_SYMBOL_GPL(pmic_irq_callback_register);
EXPORT_SYMBOL_GPL(pmic_irq_handler);

