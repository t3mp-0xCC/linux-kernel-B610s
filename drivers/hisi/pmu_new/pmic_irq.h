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

#ifndef __HI6551_IRQ_H
#define __HI6551_IRQ_H

#include <osl_types.h>
#include <linux/workqueue.h>
#include "osl_spinlock.h"

#define INT_LVL_PMIC   31

typedef unsigned long pmic_irqflags_t;
typedef void (*pmicfuncptr)(void *);

struct pmic_irq_reg{
    u32 irqarrays;/*arrys of irq*/
    u32 ocparrays;
    u32 *irq;
    u32 *ocp_irq;
};

struct pmic_irq_special{
    u32 sim_arry;
    u32 sim0_bit;
    u32 sim1_bit;
    u32 sim0_ldo;
    u32 sim1_ldo;

    u32 key_arry;
    u32 key_pending;
    u32 keyup_irq;
    u32 keydown_irq;
};

/*中断处理回调函数的结构体*/
struct pmic_irq_handle{
    unsigned irq_num;
    pmicfuncptr   routine;
    void *  data;
    unsigned cnt;
};
struct pmic_irq_mask{
    u32 maskarrys;
    u32 *mask; /*mask irq deaulft*/
};
struct pmic_irq{
    unsigned irq;
    unsigned cnts;
    unsigned irq_nums;/*num of irq*/
    struct pmic_irq_mask irq_mask;
    struct resource *res;
    spinlock_t      irq_lock;
    struct workqueue_struct *irq_wq;
    struct work_struct irq_wk;
    struct pmic_irq_reg irq_reg;
    struct pmic_irq_handle *irq_handler;
    struct pmic_irq_special irq_special;
};

/*错误码*/
#define PMIC_IRQ_OK       0
#define PMIC_IRQ_ERROR    -1
/*函数声明*/
void pmic_irq_mask(unsigned int irq);
void pmic_irq_unmask(unsigned int irq);
int pmic_irq_is_masked(unsigned int irq);
int pmic_irq_callback_register(unsigned int irq, pmicfuncptr routine, void *data);
#endif
