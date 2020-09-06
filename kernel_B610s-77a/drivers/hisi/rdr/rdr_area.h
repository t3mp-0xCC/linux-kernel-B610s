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

 #ifndef __RDR_AREA_H__
 #define __RDR_AREA_H__

#include <product_config.h>
#include <osl_types.h>
#include <mntn_interface.h>
#if !defined(__FASTBOOT__)
#include <linux/hisi/rdr_pub.h>
#else
#include "rdr_pub.h"
#endif

#define RDR_BASEINFO_ADDR           (MNTN_BASEINFO_ADDR)
#define RDR_BASEINFO_SIZE           (MNTN_BASEINFO_SIZE)

#define RDR_AREA_AP_ADDR            (MNTN_AREA_AP_ADDR)
#define RDR_AREA_AP_SIZE            (MNTN_AREA_AP_SIZE)     /*192k*/

#define RDR_AREA_TEEOS_ADDR         (MNTN_AREA_TEEOS_ADDR)
#define RDR_AREA_TEEOS_SIZE         (MNTN_AREA_TEEOS_SIZE)     /*128k*/

#define RDR_AREA_LPM3_ADDR          (MNTN_AREA_LPM3_ADDR)
#define RDR_AREA_LPM3_SIZE          (MNTN_AREA_LPM3_SIZE)      /*32k*/

#define RDR_AREA_MDMAP_ADDR         (MNTN_AREA_MDMAP_ADDR)
#define RDR_AREA_MDMAP_SIZE         (MNTN_AREA_MDMAP_SIZE)

#define RDR_AREA_CP_ADDR            (MNTN_AREA_CP_ADDR)
#define RDR_AREA_CP_SIZE            (MNTN_AREA_CP_SIZE)



typedef enum
{
    RDR_SAVE_FILE_NORMAL = 0x0,
    RDR_SAVE_FILE_NEED   = 0xAABBCCDD,
    RDR_SAVE_FILE_END    = 0x5A5A5A5A
}rdr_save_flag_t;

typedef enum
{
    RDR_START_POWER_ON = 0,
    RDR_START_REBOOT   = 0x5A5A1111,
    RDR_START_EXCH     = 0x5A5A2222
}rdr_start_flag_e;


#define RDR_TOP_HEAD_MAGIC          (0x44656164)
struct rdr_top_head_s {
	u32 magic;
	u32 version;
	u32 area_number;
    u32 reserve;
    u8 build_time[32];
    u8 product_name[32];
    u8 product_version[32];
};
struct rdr_base_info_s {
	u32 modid;
	u32 arg1;
	u32 arg2;
	u32 e_core;
    u32 e_type;
    u32 start_flag;
    u32 savefile_flag;
    u32 reboot_flag;
	u8	e_module[16];
	u8	e_desc[48];

    u8 datetime[24];
};

struct rdr_area_s {
	u32 offset; /* offset from area, unit is bytes(1 bytes) */
	u32 length; /* unit is bytes */
};

struct rdr_struct_s {
	struct rdr_top_head_s top_head;
    struct rdr_base_info_s base_info;
	struct rdr_area_s area_info[MNTN_MAX];
	u8 padding2[RDR_BASEINFO_SIZE
                - sizeof(struct rdr_top_head_s)
                - sizeof(struct rdr_area_s)*MNTN_MAX
                - sizeof(struct rdr_base_info_s)];
};

#if !defined(__FASTBOOT__)
struct rdr_area_mntn_addr_info_s
{
    void*       vaddr;
    void*       paddr;
    u32         len;
};


struct rdr_area_ctrl_info_s
{
    bool                    initState;

    void*                   phy_base_addr;
    struct rdr_struct_s*    virt_base_addr;
    u32                     base_len;

    void*                   phy_backup_addr;
    struct rdr_struct_s*    virt_backup_addr;
    u32                     backup_len;

    struct rdr_struct_s*    boot_info;
};


u32 rdr_area_init(void);
void * rdr_vmap(phys_addr_t paddr, size_t size);
void rdr_umap(void* vaddr);
u32 rdr_get_area_info(u64 coreid,struct rdr_area_mntn_addr_info_s* area_info);
void rdr_get_mntn_addr_info(struct rdr_area_mntn_addr_info_s* area_info);
void rdr_get_backup_mntn_addr_info(struct rdr_area_mntn_addr_info_s* area_info);
void rdr_fill_syserr_para(u32 modid,u32 arg1,u32 arg2);
void rdr_fill_exc_info(struct rdr_exception_info_s* exc_info,char* date);
void rdr_fill_reboot_done(void);
void rdr_fill_save_done(void);
void rdr_init_baseinfo(void);
#endif


#endif /*__RDR_AREA_H__*/

