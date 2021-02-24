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


#ifndef __RDR_MODULE_H__
#define __RDR_MODULE_H__

#include <product_config.h>
#include <osl_types.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/hisi/rdr_pub.h>

struct rdr_module_info_s
{
    bool        initState;
    u64   coreid;
    char  core[8];
    pfn_dump    dump_callback;
    pfn_reset   reset_callback;

    void*       addr;           /*子系统可维可测物理内存地址*/
    u32         len;            /*子系统可维可测物理内存长度*/
    u32         nve;            /*子系统可维可测相关其他配置*/

    const u32   modeid_start;
    const u32   modeid_end;

    spinlock_t  spinlock;
    struct list_head excList;
};

void rdr_callback(struct rdr_exception_info_s *exc_info, u32 modeid,char *path);
u32 rdr_register_module_exception(u64 coreid,struct rdr_exception_info_s* e);
struct rdr_exception_info_s* rdr_get_exception_info(u32 modeid);

u32 rdr_get_module_areainfo(u64 coreid,struct rdr_register_module_result * areainfo);
u32 rdr_register_module_callback_ops(u64 coreid,struct rdr_module_ops_pub* ops);
u32 rdr_get_module_ops(u64 coreid,struct rdr_module_ops_pub* ops);

char* rdr_get_module_name(u32 modeid);
u64 rdr_get_module_id(u32 modeid);
void rdr_print_one_exc(struct rdr_exception_info_s *e);
void rdr_show_module_info(u32 arg);
void rdr_show_exception_info(u32 core);
u32 rdr_module_init(void);
#endif



