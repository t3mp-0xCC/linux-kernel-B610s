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
 * POSSIBILITY OF SUCH DAMAGE.*/

#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/of.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/dirent.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include "drv_nv_def.h"
#include "drv_nv_id.h"
#include "ap_dump_queue.h"
#include "bsp_rfile.h"
#include "bsp_nvim.h"
#include "bsp_ap_dump.h"
#include "bsp_slice.h"


void queue_init(om_queue_t *Q, u32 elementNum)
{
    Q->maxNum = elementNum;
    Q->front = 0;
    Q->rear = 0;
    Q->num = 0;
    memset((void *)Q->data, 0, (size_t )(elementNum*sizeof(u32)));
}

s32 queue_enter(om_queue_t *Q, u32 element)
{
    if (Q->num == Q->maxNum)
    {
        return -1;
    }

    Q->data[Q->rear] = element;
    Q->rear = (Q->rear+1) % Q->maxNum;
    Q->num++;

    return 0;
}

s32 queue_outer(om_queue_t *Q, u32 *element)
{
    if (0 == Q->num)
    {
        return -1;
    }

    *element = Q->data[Q->front];
    Q->front = (Q->front+1) % Q->maxNum;
    Q->num--;

    return 0;
}

int queue_del_pre(om_queue_t *Q, u32 del_num)
{
    if (del_num > Q->num)
    {
        return -1;
    }
    Q->front = (Q->front+del_num) % Q->maxNum;
    Q->num   = Q->num - del_num;

    return 0;
}


s32 queue_read_last(om_queue_t *Q, u32 *element)
{
    if (0 == Q->num)
    {
        return -1;
    }

    *element = Q->data[((Q->rear+Q->maxNum)-1) % Q->maxNum];

    return 0;
}


s32 queue_loop_enter(om_queue_t *Q, u32 element)
{
    if (Q->num < Q->maxNum)
    {
        return queue_enter(Q, element);
    }
    else
    {
        Q->data[Q->rear] = element;
        Q->rear = (Q->rear+1) % Q->maxNum;
        Q->front = (Q->front+1) % Q->maxNum;
    }

    return 0;
}

u32 queue_get_number(om_queue_t *Q)
{
    return Q->num;
}



