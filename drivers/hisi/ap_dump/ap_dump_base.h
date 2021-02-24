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
#ifndef __AP_DUMP_BASE_H__
#define __AP_DUMP_BASE_H__

#include "osl_types.h"
#include "bsp_softtimer.h"
#include <linux/hisi/rdr_pub.h>

/**************************************************************************
  宏定义
**************************************************************************/
/*APP段内存分配*/

#define DUMP_KERNEL_BASE_INFO_SIZE                 (0x180)
#define DUMP_KERNEL_TASK_SWITCH_SIZE               (0x10000)
#define DUMP_KERNEL_INTLOCK_SIZE                   (0x1000)
#define DUMP_KERNEL_TASK_STACK_SIZE                (0x2000)
#define DUMP_KERNEL_INT_STACK_SIZE                 (0x0)
#define DUMP_KERNEL_TASK_NAME_SIZE                 (0x800)
#define DUMP_KERNEL_ALLTASK_TCB_SIZE               (0x10000)
#define DUMP_KERNEL_PRINT_SIZE                     (0x4000)
#define DUMP_KERNEL_REGS_SIZE                      (0x3000)                                     // 待定
#define DUMP_KERNEL_CPUVIEW_SIZE                   (0)
#define DUMP_KERNEL_USER_DATA_SIZE                 (0x1000)
#define DUMP_KERNEL_RSV_SIZE                       (0)

#define DUMP_INIT_FLAG_PHASE1               (0x5B5B0000)
#define DUMP_INIT_FLAG_PHASE2               (0x5B5B0001)
#define DUMP_INIT_FLAG                      (0x5B5B0002)
#define DUMP_INIT_FLAG_WAIT                 (0x5C5C5C5C)

#define DUMP_TASK_JOB_RESET_LOG             (0x00000001)
#define DUMP_TASK_JOB_SAVE                  (0x00000002)
#define DUMP_TASK_JOB_INIT                  (0x00000004)
#define DUMP_TASK_JOB_REBOOT                (0x00000008)
#define DUMP_TASK_JOB_SAVE_INIT             (DUMP_TASK_JOB_SAVE | DUMP_TASK_JOB_INIT)
#define DUMP_TASK_JOB_SAVE_REBOOT           (DUMP_TASK_JOB_SAVE | DUMP_TASK_JOB_REBOOT)

#define RDR_DUMP_FILE_AP_LOG_PATH_LEN                200
#define RDR_DUMP_FILE_AP_PATH                        "ap_log/"

typedef struct
{
    u32     modid;
    u32     product_type;
    u64     coreid;
    char    log_path[RDR_DUMP_FILE_AP_LOG_PATH_LEN];
    pfn_cb_dump_done    dump_done;
    struct rdr_register_module_result soc_rst;
}rdr_exc_info_s;

/*  CPSR R16
  31 30  29  28   27  26    7   6   5   4    3    2    1    0
----------------------------------------------------------------
| N | Z | C | V | Q | RAZ | I | F | T | M4 | M3 | M2 | M1 | M0 |
---------------------------------------------------------------- */

/*  REG
R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11 R12 R13/SP R14/LR R15/PC R16/CPSR
*/

typedef struct
{
    u32 reboot_context;     /*0x00  */
    u32 reboot_task;        /*0x04  */
    u8  taskName[16];       /*0x08  */
    u32 reboot_int;         /*0x18  */

    u32 modId;              /*0x1c  */
    u32 arg1;               /*0x20  */
    u32 arg2;               /*0x24  */
    u32 arg3;               /*0x28  */
    u32 arg3_length;        /*0x2c  */

    u32 vec;                /*0x30  */
    u32 cpsr;               /*0x34  */
    u32 regSet[17];         /*0x38  */

    u32 current_task;       /*0x7c */
    u32 current_int;        /*0x80 */

    u32 cpu_usage;          /*0x84 */
    u32 mem_free;           /*0x88 */
    u32 axi_dlock_reg[3];   /*0x8C --- AXI总线挂死寄存器，0x624,0x628,0x658*/
    u32 int_stack_base;     /*0x98 */
    u32 int_stack_end;      /*0x9C */
    u8  version[32];        /*0xA0 */
    u8  compile_time[32];   /*0xB0 */
}ap_dump_base_info_s;


typedef struct __ap_dump_filed_addr
{
    void* base_info_addr;
    void* task_stack_addr;
    void* task_tcb_addr;
    void* usr_data_addr;
    void* print_log_addr;
}ap_dump_filed_addr_s;

typedef struct __ap_dump_save_task_name
{
   void*                   task_name_addr;
   struct softtimer_list   task_name_timer;
   u32                     task_name_save_flag;

}ap_dump_task_name_ctrl_s;

typedef void (*dump_int_hook)(u32, u32);

typedef struct __ap_dump_task_switch
{
   void*                   task_switch_addr;
   spinlock_t              task_switch_lock;
   u32                     task_switch_start;
   u32                     int_switch_start;
   dump_int_hook           int_switch_hook;

}ap_dump_sys_switch_ctrl_s;

typedef enum _dump_reboot_ctx_e
{
    DUMP_CTX_TASK        = 0x0,
    DUMP_CTX_INT         = 0x1
}dump_reboot_ctx_t;


s32 ap_dump_save_task_init(void);
u32 ap_dump_match_rdr_id(u32 drv_mod_id);
void ap_dump_save_exc_task(u32 addr);
void ap_dump_save_base_info(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length);
void ap_dump_save_usr_data(char *data, u32 length);
void ap_dump_show_stack(void);
void ap_dump_save_all_task(void);
void ap_dump_save_task_name(void);
s32 ap_dump_task_info_init(void);
void ap_dump_trace_stop(void);
void ap_dump_save_and_reboot(void);
void ap_dump_set_vec(u32 vec);
u32 ap_dump_get_init_status(void);
s32 ap_dump_base_info_init(void);
void ap_dump_queue_t_init(void);
s32 ap_dump_register_hook(void);
void ap_dump_init_done(void);
void ap_dump_ctrl_init(void);
s32 ap_dump_start_task_name_timer(u32 time_out);
void ap_dump_stop_task_name_timer(void);
void ap_dump_print_int(void);
s32 ap_dump_task_name_init(void);
void ap_dump_register_fileds(void);
void ap_dump_save_log(void);
void ap_dump_save_file(char * file_name, void * addr, u32 len);


#endif
