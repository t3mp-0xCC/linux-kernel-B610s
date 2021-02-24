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

#ifndef	__WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "osl_types.h"
#include "osl_spinlock.h"
#include "bsp_om.h"
#include "bsp_wp.h"

enum
{
    WATCHPOINT_UNALLOC = 0,
    WATCHPOINT_ALLOCED = 1,
};

typedef struct
{
    u32         alloc;
    bool        status_chg;
    wp_cfg_t    cfg;
}wp_info_t;

typedef struct
{
    wp_info_t * wp_info;
    u32         wp_cnt;
    bool        init_flag;
    spinlock_t  lock;
    u32         wp_hit_cnt;
}wp_ctrl_t;

typedef struct
{
    u32             addr;
    u32             bp_index;
    watchpoint_ctrl ctrl;
}wp_step_ctrl_t;

typedef struct
{
    u32     dscr;
    u32     wvr_reg[16];
    u32     wcr_reg[16];
}wp_reg_back_t;

#define wp_print(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_FATAL, BSP_MODU_WATCHPOINT, "[watchpoint]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))

s32 bsp_wp_register(wp_cfg_t * cfg);
s32 bsp_wp_unregister(s32 wp_id);
s32 bsp_wp_enable(s32 wp_id);
s32 bsp_wp_disable(s32 wp_id);
void wp_debug(void);
void wp_show(void);
u32 read_wp_reg(int n);
void wp_clr_hdbg(void);


#endif //__WATCHPOINT_H__

