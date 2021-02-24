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

#ifndef __PMIC_EXC_H
#define __PMIC_EXC_H

#include <linux/workqueue.h>
#include <bsp_om.h>

typedef unsigned long pmic_excflags_t;

struct ocp_info
{
    u32 ocp_addr;
    u32 ocp_data;
    u32* ocp_id;
    char** ocp_name;
};
struct record_info
{
    u32 record_addr;
    u32 record_data;
    u32 inacceptable;/*flag this record exc is acceptable or not;1:inaccept;0:accept*/
    char** record_name;
};
struct pmic_record{
    u32 record_num;
    struct record_info* record_info;
};
struct pmic_ocp{
    u32 ocp_irq;
    u32 ocp_num;
    struct ocp_info* ocp_info;
};
struct pmic_otp{
    u32 otp_irq;
    u32 otp_base;
    u32 otp_mask;
    u32 otp_value;
    u32 otp_offset;
};
struct pmic_uvp{
    u32 uvp_irq;
    u32 uvp_base;
    u32 uvp_mask;
    u32 uvp_value;
    u32 uvp_offset;
};
struct pmic_pro{
    u32 ocp_reon;/* 过流的电源可以重新打开 */
    u32 ocp_rst;/* 过流的电源需要关闭 */
    u32 otp_rst; /* 过温后系统重启 */
    u32 otp_off_num;/* 发生过温后需要关闭非核心电源个数 */
    u32* otp_off_arry;/* 发生过温后需要关闭非核心电源列表 */
};

struct pmic_exc{
    spinlock_t      lock;
    u32 data_width;
    struct workqueue_struct *om_wq;
    struct delayed_work om_wk;
    struct pmic_ocp ocp;
    struct pmic_otp otp;
    struct pmic_uvp uvp;
    struct pmic_pro pro;
    struct pmic_record record;
};

int pmic_ocp_register(int volt_id,PMU_OCP_FUNCPTR func);
#endif
