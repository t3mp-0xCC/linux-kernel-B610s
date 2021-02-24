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
#ifndef __AP_DUMP_MEM_H__
#define __AP_DUMP_MEM_H__

#ifndef __ASSEMBLY__
#include "bsp_dump.h"
#include "osl_types.h"
#include "osl_list.h"
#include "osl_spinlock.h"
#endif
#include "bsp_memmap.h"
#include "bsp_s_memory.h"
#include "mntn_interface.h"

#define DUMP_FIELD_MAX_NUM 64
/* AP area */
#define DUMP_AREA_KERNEL_ADDR         (MNTN_AREA_AP_ADDR)
#define DUMP_AREA_KERNEL_SIZE         (MNTN_AREA_AP_SIZE)
#define DUMP_AREA_KERNEL_HEAD_ADDR    (MNTN_AREA_AP_ADDR)
#define DUMP_AREA_KERNEL_HEAD_SIZE    (0x20)
#define DUMP_AREA_KERNEL_MAP_ADDR     (DUMP_AREA_KERNEL_HEAD_ADDR+DUMP_AREA_KERNEL_HEAD_SIZE)
#define DUMP_AREA_KERNEL_MAP_SIZE     (32*DUMP_FIELD_MAX_NUM)
#define DUMP_AREA_KERNEL_STATIC_ADDR  (DUMP_AREA_KERNEL_MAP_ADDR+DUMP_AREA_KERNEL_MAP_SIZE)
#define DUMP_AREA_KERNEL_STATIC_SIZE  (0)
#define DUMP_AREA_KERNEL_DYNAMIC_ADDR (DUMP_AREA_KERNEL_STATIC_ADDR+DUMP_AREA_KERNEL_STATIC_SIZE)
#define DUMP_AREA_KERNEL_DYNAMIC_SIZE (MNTN_AREA_AP_SIZE-DUMP_AREA_KERNEL_HEAD_SIZE-DUMP_AREA_KERNEL_MAP_SIZE-DUMP_AREA_KERNEL_STATIC_SIZE)

/* field magic num */
#define DUMP_FIELD_MAGIC_NUM    (0x6C7D9F8E)
#define DUMP_AREA_MAGICNUM  0x4e656464

/* dump memory map */
typedef struct ap_dump_mem_ctrl_s
{
    void*  ap_dump_virt_addr;
	u32    ap_dump_phy_addr;
	u32    ap_dump_mem_size;
}ap_dump_mem_ctrl_t;


/* area head  */
typedef struct _dump_area_head_s
{
    u32 magic_num;
    u32 field_num;
    u8  name[8];
    u8  version[16]; /* area format version */
}ap_dump_area_head_t;

/* field map */
typedef struct _dump_field_map_s
{
    u32 field_id;
    u32 offset_addr;
    u32 length;
    u16 version;
    u16 status;
    u8  field_name[16];
}ap_dump_field_map_t;

/* area */
typedef struct _dump_area_s
{
    ap_dump_area_head_t  area_head;
    ap_dump_field_map_t  fields[DUMP_FIELD_MAX_NUM];
    u8                data[4];
}ap_dump_area_t;

/* one field can have some hook function, called by dump framework */
typedef struct _dump_field_hook_t
{
    struct list_head hook_list;
    dump_hook        pfunc;
    char             name[16];
}ap_dump_field_hook_t;
/* dump memory infortion in this cpu */
typedef struct __dump_info_s
{
    u32                   cur_field_num;                  /* current cpu registered field number   */
    ap_dump_field_hook_t  cur_hook_list;                  /* current cpu registered field hook list*/
    u8 *                  cur_dynamic_free_addr;          /* current cpu dynamic area base address */
    u32                   cur_dynamic_free_size;          /* current cpu dynamic area free size */
    ap_dump_area_t*       p_cur_area;                     /* current cpu area */
    spinlock_t            lock;                           /* lock for protecting dump */
}ap_dump_manager_t;

enum
{
    DUMP_FIELD_UNUSED = 0,
    DUMP_FIELD_USED   = 1,
};



u8 * bsp_ap_dump_get_field_map(u32 field_id);
u8 * bsp_ap_dump_get_field_addr(u32 field_id);
u8 * bsp_ap_dump_register_field(u32 field_id, char * name, void * virt_addr, void * phy_addr, u32 length, u16 version);
void ap_dump_save_all_fields(void);


#endif

