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

#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/thread_info.h>
#include <linux/syslog.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/kdebug.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <asm/string.h>
#include <asm/traps.h>
#include "product_config.h"
#include <linux/syscalls.h>
#include <asm/thread_notify.h>
#include "osl_types.h"
#include "osl_io.h"
#include "osl_bio.h"
#include "osl_malloc.h"
#include "bsp_ipc.h"
#include "bsp_memmap.h"
#include "bsp_wdt.h"
#include "bsp_icc.h"
#include "bsp_onoff.h"
#include "bsp_nvim.h"
#include "bsp_softtimer.h"
#include "bsp_version.h"
#include "bsp_sram.h"
#include "bsp_coresight.h"
#include "bsp_reset.h"
#include "drv_nv_def.h"
#include "mdrv_om.h"
#include <gunas_errno.h>
#include <linux/hisi/rdr_pub.h>
#include "bsp_om.h"
#include "ap_dump_base.h"
#include "bsp_ap_dump.h"
#include "ap_dump_mem.h"
#include "ap_dump_queue.h"
#include "drv_nv_def.h"
#include "bsp_rfile.h"

ap_dump_base_info_s        *g_dump_base_info = BSP_NULL;
ap_dump_task_info_s        *g_ap_dump_task_info = BSP_NULL;
ap_dump_filed_addr_s        g_ap_fileds_addr;
dump_ctrl_s                 g_ap_dump_ctrl;
ap_dump_flag_s              g_ap_dump_flags ;
rdr_exc_info_s              g_ap_rdr_exc_info;
ap_dump_task_name_ctrl_s    g_ap_dump_task_name_ctrl;
ap_dump_sys_switch_ctrl_s   g_ap_dump_sys_switch_ctrl;

extern void show_mem(unsigned int filter);

/*¸Ãº¯ÊýÔÚÄÚºË¶¨Òå*/
extern int dump_print_all(char * buf, int size, bool clear);

extern void unwind_backtrace(struct pt_regs *regs, struct task_struct *tsk);



  /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_save_base_info
* ¹¦ÄÜÃèÊö  : ±£´æ»ù±¾ÐÅÏ¢
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_save_base_info(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length)
{

    g_dump_base_info->modId = mod_id;
    g_dump_base_info->arg1 = arg1;
    g_dump_base_info->arg2 = arg2;
    g_dump_base_info->arg3 = (u32)data;
    g_dump_base_info->arg3_length = length;

    if(in_interrupt())
    {
        g_dump_base_info->reboot_task = (u32)(-1);
        memset(g_dump_base_info->taskName, 0, sizeof(g_dump_base_info->taskName));
        g_dump_base_info->reboot_int = g_dump_base_info->current_int;
        g_dump_base_info->reboot_context = DUMP_CTX_INT;

    }
    else
    {
        g_dump_base_info->reboot_task = g_dump_base_info->current_task;
        if(NULL != (void*)g_dump_base_info->reboot_task)
        {
            memcpy(g_dump_base_info->taskName, ((struct task_struct *)(g_dump_base_info->reboot_task))->comm, 16);
        }

        g_dump_base_info->reboot_int = (u32)(-1);
        g_dump_base_info->reboot_context = DUMP_CTX_TASK;

    }

    ap_dump_fetal("save base info finish\n");

    return;
}

  /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_stack_hook
* ¹¦ÄÜÃèÊö  : ¹´È¡ÈÎÎñµÄÕ»ÐÅÏ¢
*
* ÊäÈë²ÎÊý  : where
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_stack_hook(unsigned long where)
{

    ap_dump_task_info_s *task = (ap_dump_task_info_s *)g_ap_dump_ctrl.current_task_info;

    if( (DUMP_KERNEL_TASK_INFO_STACK_SIZE - task->offset) > (strlen((void*)where)+12) )
    {
        /* coverity[secure_coding] */
        task->offset += sprintf((char *)task->dump_stack+task->offset, "[%08lx]%pS\n", (unsigned long)where, (void *)where);
    }

    return;
}

 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_show_stack
* ¹¦ÄÜÃèÊö  : ´òÓ¡µ÷ÓÃÕ»£¬ÓÃÓÚ·ÇarmÒì³£
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_show_stack(void)
{
    if (!(AP_DUMP_REASON_ARM == g_dump_base_info->arg1))
    {
        ap_dump_error("###########show mem and current task stack start##############!\n");

        show_mem(0);

        show_stack(current, NULL);

        ap_dump_error("###########show mem and current task stack end################!\n");
    }
}

 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_save_arm_regs
* ¹¦ÄÜÃèÊö  : ±£´æµ±Ç°ÈÎÎñµÄ¼Ä´æÆ÷ÐÅÏ¢
*
* ÊäÈë²ÎÊý  : addr ±£´æµØÖ·
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
__inline__ void ap_dump_save_arm_regs(u32 addr)
{
    asm volatile(
        "str r0, [r0,#0x00]\n"
        "str r1, [r0,#0x04]\n"
        "str r2, [r0,#0x08]\n"
        "str r3, [r0,#0x0C]\n"
        "str r4, [r0,#0x10]\n"
        "str r5, [r0,#0x14]\n"
        "str r6, [r0,#0x18]\n"
        "str r7, [r0,#0x1C]\n"
        "str r8, [r0,#0x20]\n"
        "str r9, [r0,#0x24]\n"
        "str r10, [r0,#0x28]\n"
        "str r11, [r0,#0x2C]\n"
        "str r12, [r0,#0x30]\n"
        "str r14, [r0,#0x38]\n"
        "push {r1}\n"
        "str r13, [r0,#0x34]\n"
        "mov r1, pc\n"
        "str r1, [r0,#0x3C]\n"
        "mrs r1, cpsr\n"
        "str r1, [r0,#0x40]\n"
        "pop {r1}\n"
    );
}
 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_save_log
* ¹¦ÄÜÃèÊö  : ±£´æaºË´®¿ÚµÄlog
*
*
* ÊäÈë²ÎÊý  : addr µ±Ç°¼Ä´æÆ÷±£´æµÄµØÖ·
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_save_log(void)
{
    /* MBBÇÖÈëÄÚºËÐÞ¸Äprintk.c£¬PHONE dump²»±£´æ´òÓ¡Êý¾Ý£¬ÓÉRDR±£´æ */
    if(g_ap_fileds_addr.print_log_addr != NULL)
    {
        dump_print_all((char*)(g_ap_fileds_addr.print_log_addr), DUMP_KERNEL_PRINT_SIZE, false);
    }

    ap_dump_fetal("ap dump save print log finish\n");

}
 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_save_exc_task
* ¹¦ÄÜÃèÊö  : ±£´æÒì³£ÈÎÎñÐÅÏ¢
*             Ä¿Ç°Ö»±£´æstack, ±£´æÈ«²¿8KÊý¾Ý
*
* ÊäÈë²ÎÊý  : addr µ±Ç°¼Ä´æÆ÷±£´æµÄµØÖ·
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_save_exc_task(u32 addr)
{
    struct task_struct *task = get_current();

    ap_dump_save_arm_regs(addr);

    g_dump_base_info->current_task = (u32)task;

    if(g_ap_fileds_addr.task_stack_addr != NULL)
    {
        memcpy((void * )g_ap_fileds_addr.task_stack_addr , (const void * )task->stack, (size_t )THREAD_SIZE);
    }

    ap_dump_fetal("ap_dump_save_exc_task finish\n");

    return;
}

 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_save_all_task
* ¹¦ÄÜÃèÊö  : ±£´æËùÓÐÈÎÎñµÄtcbÐÅÏ¢
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_save_all_task(void)
{
    struct task_struct  *pTid = NULL;
    struct thread_info  *pThread = NULL;
    ap_dump_task_info_s *task_info = NULL;
    int ulTaskNum = 0x00;
    if(g_ap_dump_task_info == NULL)
    {
        ap_dump_fetal("g_ap_dump_task_info is NULL\n");
        return;
    }

    for_each_process(pTid)
    {
        if(ulTaskNum >=  DUMP_LINUX_TASK_NUM_MAX)
        {
            return;
        }

        pThread = (struct thread_info*)pTid->stack;
        task_info = &g_ap_dump_task_info[ulTaskNum];
        task_info->pid = PID_PPID_GET(pTid);
        task_info->entry = (u32)BSP_NULL;                        // linuxÔÝ²»Ö§³Ö
        task_info->status = pTid->state;
        task_info->policy = pTid->policy;
        task_info->priority = pTid->prio;
        task_info->stack_base = (uintptr_t)((uintptr_t)pTid->stack + THREAD_SIZE);
        task_info->stack_end = (uintptr_t)end_of_stack(pTid);
        task_info->stack_high = 0;                          // linuxÔÝ²»Ö§³Ö
        /* coverity[buffer_size_warning] */
        strncpy((char *)task_info->name, pTid->comm, 16);
        task_info->regs[0] = 0;                             // Ç°ËÄ¸öÍ¨ÓÃ¼Ä´æÆ÷ÎÞÒâÒå
        task_info->regs[1] = 0;
        task_info->regs[2] = 0;
        task_info->regs[3] = 0;
        memcpy(&task_info->regs[4], &pThread->cpu_context, 12*sizeof(u32));
        task_info->offset = 0;

        g_ap_dump_ctrl.current_task_info = (uintptr_t)task_info;

        g_ap_dump_flags.stack_trace_flag = true;

        unwind_backtrace(NULL, pTid);
        g_ap_dump_flags.stack_trace_flag = false;

        ulTaskNum++;

    }

    ap_dump_fetal("dump save all task finish\n");

    return;
}


 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_save_usr_data
* ¹¦ÄÜÃèÊö  : ±£´æsystem_error´«ÈëµÄÓÃ»§Êý¾Ý
*
* ÊäÈë²ÎÊý  : char *data  ÓÃ»§Êý¾ÝµØÖ·
*             u32 length  ÓÃ»§Êý¾Ý³¤¶È
*
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_save_usr_data(char *data, u32 length)
{
    s32 len = 0;

    ap_dump_field_map_t* pfield = NULL;

    if ((NULL != data) && (length))
    {
        pfield = (ap_dump_field_map_t*)bsp_ap_dump_get_field_map(DUMP_KERNEL_USER_DATA);
        len = (length > DUMP_KERNEL_USER_DATA_SIZE) ? DUMP_KERNEL_USER_DATA_SIZE : length;

        if(g_ap_fileds_addr.usr_data_addr != NULL)
        {
             memcpy((void *)g_ap_fileds_addr.usr_data_addr, (const void * )data, (size_t)len); /* [false alarm]:ÆÁ±ÎFortify´íÎó */
        }

        if(pfield)
        {
            pfield->length = len;
        }
        ap_dump_fetal("dump save usr data finish\n");
    }
    else
    {
        ap_dump_fetal("error no need save usr data\n");
    }
    return;
}





/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_save_etb
* ¹¦ÄÜÃèÊö  : ±£´æap etbÐÅÏ¢
*
* ÊäÈë²ÎÊý  : dir_name ÎÄ¼þ±£´æµÄÄ¿Â¼
*             cfg ¹¦ÄÜÅäÖÃÏî
*
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_save_etb(char * dir_name)
{
    char file_name[AP_DUMP_FILE_NAME_LENGTH] = {0};
    u8 * data = NULL;
    DUMP_FILE_CFG_STRU            dump_file_cfg = {{0}};

    if(BSP_OK != bsp_nvm_read(NV_ID_DRV_DUMP_FILE, (u8 *)&dump_file_cfg, sizeof(DUMP_FILE_CFG_STRU)))
    {
        ap_dump_fetal("read nv fail\n");
        return;
    }

    if(dump_file_cfg.file_list.file_bits.ap_etb == 1)
    {
        memset(file_name, 0, sizeof(file_name));

        snprintf(file_name, sizeof(file_name), "%sap_etb.bin", dir_name);

        data = (u8 *)bsp_ap_dump_get_field_addr(DUMP_KERNEL_UTRACE);
        if(data != NULL)
        {
            ap_dump_save_file(file_name, data, 0x2400);

            ap_dump_fetal("[dump]: save %s finished\n", file_name);
        }
    }
    else
    {
            ap_dump_fetal("noneed save etb\n");

    }
}

/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_save_files
* ¹¦ÄÜÃèÊö  : ap ±£´æ log ÎÄ¼þ
*
* ÊäÈë²ÎÊý  : log_path ÎÄ¼þ±£´æµÄÄ¿Â¼
*             nve_value ¹¦ÄÜÅäÖÃÏî
*
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 ap_dump_save_files(char * log_path, u32 nve_value)
{
    ap_dump_fetal("ap_dump_save_files[%d]: enter!\n", __LINE__);

    ap_dump_save_etb(log_path);

    ap_dump_fetal("ap_dump_save_files[%d]: exit!\n", __LINE__);


    return BSP_OK;
}


/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_wait_for_reboot
* ¹¦ÄÜÃèÊö  : ÏµÍ³½øÈëËÀÑ­»·Ä£Ê½
*
* ÊäÈë²ÎÊý  :
*
*
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
__inline__ void ap_dump_wait_for_reboot(void)
{
    /* coverity[no_escape] */
    do{
    }while(1);
}

/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_save_and_reboot
* ¹¦ÄÜÃèÊö  : ´¥·¢ap ±£´ælogÈÎÎñ²¢ÇÒµÈ´ý¸´Î»
*
* ÊäÈë²ÎÊý  :
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_save_and_reboot(void)
{
    g_ap_dump_ctrl.dump_task_job = DUMP_TASK_JOB_SAVE_REBOOT;

    up(&g_ap_dump_ctrl.sem_dump_task);

    ap_dump_fetal("up g_ap_dump_ctrl.sem_dump_task \n");

    return;
}


/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_task_name_init
* ¹¦ÄÜÃèÊö  : ±£´æÏµÍ³ÈÎÎñÃû³õÊ¼»¯
*
* ÊäÈë²ÎÊý  :
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 ap_dump_task_name_init(void)
{
    g_ap_dump_task_name_ctrl.task_name_addr = bsp_ap_dump_register_field(DUMP_KERNEL_TASK_NAME,     "TASK_NAME", NULL,NULL,DUMP_KERNEL_TASK_NAME_SIZE,0);

    if(g_ap_dump_task_name_ctrl.task_name_addr == NULL)
    {
        ap_dump_fetal("alloc DUMP_AP_ALLTASK fail\n");
        return BSP_ERROR;
    }

    queue_init((om_queue_t *)(g_ap_dump_task_name_ctrl.task_name_addr), (DUMP_KERNEL_TASK_NAME_SIZE - 0x10) / 0x4);

    ap_dump_start_task_name_timer(60000);

    ap_dump_fetal("ap_dump_task_name_init finish\n");

    return BSP_OK;
}

/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_save_task_name
* ¹¦ÄÜÃèÊö  : ±£´æËùÓÐÈÎÎñµÄÈÎÎñÃû
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_save_task_name(void)
{
    struct task_struct *pTid = NULL;
    int ulTaskNum = 0x00;
    u32 pid_ppid = 0;
    void* task_name_addr = 0;
    char idle_task_name[12] = {"swapper"};

    task_name_addr = g_ap_dump_task_name_ctrl.task_name_addr;
    if(NULL == task_name_addr )
    {
        ap_dump_fetal("g_ap_dump_task_name_ctrl.task_name_addr is NULL\n");

        return;
    }
    /* ÈÎÎñÇÐ»»ÕýÔÚ¼ÇÂ¼£¬Ö±½Ó·µ»Ø */
    if(g_ap_dump_task_name_ctrl.task_name_save_flag == true)
    {
        return;
    }

    /* ¿ªÊ¼¼ÇÂ¼ */
    g_ap_dump_task_name_ctrl.task_name_save_flag = true;

    for_each_process(pTid)
    {
        if(ulTaskNum >=  DUMP_LINUX_TASK_NUM_MAX)
        {
            break;
        }

        pid_ppid = PID_PPID_GET(pTid);

        queue_loop_enter((om_queue_t *)task_name_addr, pid_ppid);
        queue_loop_enter((om_queue_t *)task_name_addr, *((int *)(pTid->comm)));
        queue_loop_enter((om_queue_t *)task_name_addr, *((int *)((pTid->comm)+4)));
        queue_loop_enter((om_queue_t *)task_name_addr, *((int *)((pTid->comm)+8)));

        ulTaskNum++;
    }

    queue_loop_enter((om_queue_t *)task_name_addr, 0);
    queue_loop_enter((om_queue_t *)task_name_addr, *((int *)(idle_task_name)));
    queue_loop_enter((om_queue_t *)task_name_addr, *((int *)(idle_task_name+4)));
    queue_loop_enter((om_queue_t *)task_name_addr, *((int *)(idle_task_name+8)));

    g_ap_dump_task_name_ctrl.task_name_save_flag = false;

}


 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_stack_trace_print
* ¹¦ÄÜÃèÊö  : »ñÈ¡ÈÎÎñÕ»ÐÅÏ¢
*
*
* ÊäÈë²ÎÊý  :
*
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
int ap_dump_stack_trace_print(unsigned long where)
{
    if(g_ap_dump_flags.stack_trace_flag == true)
    {
        ap_dump_stack_hook(where);
        return BSP_OK;
    }

    return BSP_ERROR;
}

 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_task_switch_hook
* ¹¦ÄÜÃèÊö  : ½«ÒªÇÐ»»µÄÈÎÎñIDºÍÊ±¼äÈë¶Ó
*
* ÊäÈë²ÎÊý  : old_tcb ¾ÉµÄtcbÐÅÏ¢
*             new_tcb ÐÂµÄtcbÐÅÏ¢
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
/*lint -save -e123 -e530 -e701 -e713 -e718 -e732 -e737 -e746*/
void ap_dump_task_switch_hook(void *old_tcb, void *new_tcb)
{
    u32 pid_ppid = 0;
    void* addr = NULL;
    unsigned long lock_flag;
    void* task_name_addr = NULL;


    if(BSP_FALSE == g_ap_dump_sys_switch_ctrl.task_switch_start)
    {
        return;
    }

    addr = g_ap_dump_sys_switch_ctrl.task_switch_addr;
    if(NULL == addr)
    {
        return;
    }

    pid_ppid = PID_PPID_GET(new_tcb);
    spin_lock_irqsave(&g_ap_dump_sys_switch_ctrl.task_switch_lock, lock_flag);
    queue_loop_enter((om_queue_t *)addr, (u32)pid_ppid);
    queue_loop_enter((om_queue_t *)addr, bsp_get_slice_value());
    spin_unlock_irqrestore(&g_ap_dump_sys_switch_ctrl.task_switch_lock, lock_flag);

    g_dump_base_info->current_task = ((u32)new_tcb);

    /* ¶¨Ê±Æ÷³¬Ê±£¬ÕýÔÚ¼ÇÂ¼ÈÎÎñÃû£¬ÈÎÎñÇÐ»»²»×ö¼ÇÂ¼ */
    if(g_ap_dump_task_name_ctrl.task_name_save_flag == true)
    {
        return;
    }

    /* ¿ªÊ¼¼ÇÂ¼ */
    g_ap_dump_task_name_ctrl.task_name_save_flag = true;

    /* ÒòÎªkthreaddÅÉÉú³öÀ´µÄÈÎÎñ£¬µÚÒ»´ÎÔËÐÐÊ±£¬ÈÎÎñÃû¶¼½Ðkthreadd£¬ËùÒÔÈÎÎñµÚ¶þ´Î½øÈëÊ±£¬²Å¼ÇÂ¼ */
    /* dump_magic×Ö¶Î£¬Ç¶ÈëÊ½ÐÞ¸ÄÄÚºË */
    if(((struct task_struct*)new_tcb)->dump_magic == (int)new_tcb)
    {
        g_ap_dump_task_name_ctrl.task_name_save_flag = false;
        return;
    }
    else if(((struct task_struct*)new_tcb)->dump_magic == (int)new_tcb + 1)
    {
        task_name_addr = g_ap_dump_task_name_ctrl.task_name_addr;
        if(NULL == task_name_addr )
        {
            return;
        }

        queue_loop_enter((om_queue_t *)task_name_addr, pid_ppid);
        queue_loop_enter((om_queue_t *)task_name_addr, *((u32 *)(((struct task_struct *)(new_tcb))->comm)));
        queue_loop_enter((om_queue_t *)task_name_addr, *((u32 *)((((struct task_struct *)new_tcb)->comm)+4)));
        queue_loop_enter((om_queue_t *)task_name_addr, *((u32 *)((((struct task_struct *)new_tcb)->comm)+8)));
        ((struct task_struct*)new_tcb)->dump_magic = (int)new_tcb;
    }
    else
    {
        ((struct task_struct*)new_tcb)->dump_magic = (int)new_tcb + 1;
    }

    g_ap_dump_task_name_ctrl.task_name_save_flag = false;

}

 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_task_switch_callback
* ¹¦ÄÜÃèÊö  : ÏµÍ³ÈÎÎñÇÐ»»µÄnotify»Øµ÷
*
* ÊäÈë²ÎÊý  :
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
int ap_dump_task_switch_callback(struct notifier_block *nb, unsigned long action, void *data)
{
    struct thread_info *thread = data;

    if (action != THREAD_NOTIFY_SWITCH)
    {
        return NOTIFY_DONE;
    }

    ap_dump_task_switch_hook(NULL, thread->task);

    return NOTIFY_OK;
}

/* dump task switch notifier */
static struct notifier_block ap_dump_task_switch_notifier =
{
    .notifier_call = ap_dump_task_switch_callback,
    .priority      = 0,
};


 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_int_enter
* ¹¦ÄÜÃèÊö  : ÖÐ¶Ï½øÈë¹
*             ¸Ãº¯ÊýÔÚÄÚºËÖÐÊ¹ÓÃ
*
* ÊäÈë²ÎÊý  : u32 dir ·½Ïò
*             u32 irq_num ÖÐ¶ÏºÅ
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_int_enter(u32 dir, u32 irq_num)
{
    if(g_ap_dump_sys_switch_ctrl.int_switch_hook != NULL)
    {
        g_ap_dump_sys_switch_ctrl.int_switch_hook(dir, irq_num);
    }
}


 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_int_exit
* ¹¦ÄÜÃèÊö  : ÖÐ¶Ï½øÈë¹
*             ¸Ãº¯ÊýÔÚÄÚºËÖÐÊ¹ÓÃ
*
* ÊäÈë²ÎÊý  : u32 dir ·½Ïò
*             u32 irq_num ÖÐ¶ÏºÅ
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_int_exit(u32 dir, u32 irq_num)
{
    if(g_ap_dump_sys_switch_ctrl.int_switch_hook != NULL)
    {
        g_ap_dump_sys_switch_ctrl.int_switch_hook(dir, irq_num);
    }
}

 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_int_switch_hook_add
* ¹¦ÄÜÃèÊö  : ×¢²áÖÐ¶ÏÇÐ»»¹³×Ó
*
* ÊäÈë²ÎÊý  : hook_func ÖÐ¶ÏÇÐ»»º¯Êý
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_int_switch_hook_add(dump_int_hook hook_func)
{
    g_ap_dump_sys_switch_ctrl.int_switch_hook = hook_func;
}


 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_int_switch_hook_delete
* ¹¦ÄÜÃèÊö  : É¾³ýÖÐ¶ÏÇÐ»»¹³×Ó
*
* ÊäÈë²ÎÊý  :
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_int_switch_hook_delete(void)
{
    g_ap_dump_sys_switch_ctrl.int_switch_hook = NULL;
}

 /*****************************************************************************
* º¯ Êý Ãû  : ap_dump_int_switch_hook
* ¹¦ÄÜÃèÊö  : ÖÐ¶ÏÇÐ»»¹³×Óº¯Êý£¬½«´¥·¢µÄÖÐ¶ÏIDºÍÊ±¼äÈë¶Ó
*
* ÊäÈë²ÎÊý  : dir   ·½Ïò:½øÈëÖÐ¶Ï»òÍË³öÖÐ¶Ï
*             newVec ÐÂµÄÖÐ¶Ï
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_int_switch_hook(u32 dir, u32 newVec)
{
    void* addr = NULL;
    unsigned long lock_flag = 0;

    if(BSP_FALSE == g_ap_dump_sys_switch_ctrl.int_switch_start)
    {
        return;
    }
    addr = g_ap_dump_sys_switch_ctrl.task_switch_addr;
    if(NULL == addr)
    {
        return;
    }

    spin_lock_irqsave(&g_ap_dump_sys_switch_ctrl.task_switch_lock, lock_flag);

    if (0 == dir)/*IN*/
    {
        queue_loop_enter((om_queue_t *)addr, (((u32)DUMP_INT_IN_FLAG<<16)|newVec));
    }
    else/*EXIT*/
    {
        queue_loop_enter((om_queue_t *)addr, (((u32)DUMP_INT_EXIT_FLAG<<16)|newVec));
    }

    queue_loop_enter((om_queue_t *)addr, bsp_get_slice_value());

    spin_unlock_irqrestore(&g_ap_dump_sys_switch_ctrl.task_switch_lock, lock_flag);

    g_dump_base_info->current_int = newVec;

    return;
}

/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_trace_stop
* ¹¦ÄÜÃèÊö  : Í£Ö¹¼ÇÂ¼trace
*
* ÊäÈë²ÎÊý  : log_path ÎÄ¼þ±£´æµÄÄ¿Â¼
*             nve_value ¹¦ÄÜÅäÖÃÏî
*
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_trace_stop(void)
{
    ap_dump_int_switch_hook_delete();

    g_ap_dump_sys_switch_ctrl.task_switch_start = BSP_FALSE;

    g_ap_dump_sys_switch_ctrl.int_switch_start = BSP_FALSE;

    ap_dump_fetal("ap_dump_trace_stop success\n ");

    return;
}

/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_register_hook
* ¹¦ÄÜÃèÊö  : ×¢²áÏµÍ³¹³×Óº¯Êý
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 ap_dump_register_hook(void)
{
    s32 ret = BSP_OK;
    NV_DUMP_STRU            dump_cfg = {{0}};

    g_ap_dump_sys_switch_ctrl.task_switch_addr = bsp_ap_dump_register_field(DUMP_KERNEL_TASK_SWITCH, "TASK_SWITCH", NULL,NULL,DUMP_KERNEL_TASK_SWITCH_SIZE,0);

    if(g_ap_dump_sys_switch_ctrl.task_switch_addr == NULL)
    {
        ap_dump_fetal("alloc DUMP_AP_TASK_SWITCH fail\n");
        return BSP_ERROR;
    }

    queue_init((om_queue_t *)(g_ap_dump_sys_switch_ctrl.task_switch_addr), (DUMP_KERNEL_TASK_SWITCH_SIZE - 0x10) / 0x4);

    ret = bsp_nvm_read(NV_ID_DRV_DUMP, (u8*)&dump_cfg, sizeof(NV_DUMP_STRU));
    if(ret != BSP_OK)
    {
        ap_dump_fetal("read cfg nv fail register hook fail\n ");
    }

    /* register task switch notifier */
    if(1 == dump_cfg.dump_cfg.Bits.taskSwitch)
    {
        g_ap_dump_sys_switch_ctrl.task_switch_start = BSP_TRUE;

        thread_register_notifier(&ap_dump_task_switch_notifier);

        ap_dump_fetal("task switch hook register success\n ");
    }

    if(1 == dump_cfg.dump_cfg.Bits.intSwitch)
    {
        g_ap_dump_sys_switch_ctrl.int_switch_start = BSP_TRUE;

        ap_dump_int_switch_hook_add((dump_int_hook)ap_dump_int_switch_hook);

        ap_dump_fetal("int switch hook register success\n ");
    }
    return BSP_OK;
}


/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_save_task
* ¹¦ÄÜÃèÊö  : dump_saveÈÎÎñµÄÈë¿Úº¯Êý
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
int ap_dump_save_task(void *data)
{
    /* coverity[no_escape] */
    while(1)
    {
        down(&g_ap_dump_ctrl.sem_dump_task);

        ap_dump_fetal("down g_ap_dump_ctrl.sem_dump_task\n");

        if(DUMP_TASK_JOB_SAVE_REBOOT == (g_ap_dump_ctrl.dump_task_job & DUMP_TASK_JOB_SAVE_REBOOT))
        {

            ap_dump_save_files(g_ap_rdr_exc_info.log_path, g_ap_rdr_exc_info.soc_rst.nve);

            ap_dump_fetal("dump save finished\n");

            if(g_ap_rdr_exc_info.dump_done != BSP_NULL)
            {
                g_ap_rdr_exc_info.dump_done(g_ap_rdr_exc_info.modid, g_ap_rdr_exc_info.coreid);

                ap_dump_fetal("ap dump save files finish,notify rdr success\n");

            }
        }

        g_ap_dump_ctrl.dump_task_job = 0;

        return BSP_OK;
    }

    return BSP_OK;
}

/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_save_task_init
* ¹¦ÄÜÃèÊö  : ´´½¨±£´ælogµÄÈÎÎñ
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 ap_dump_save_task_init(void)
{
    struct task_struct * pid = NULL;
    struct sched_param   param = {0,};

    sema_init(&g_ap_dump_ctrl.sem_dump_task, 0);

    pid = (struct task_struct *)kthread_run(ap_dump_save_task, 0, "dump_ap_save");
    if (IS_ERR((void*)pid))
    {
        ap_dump_error("bsp_dump_save_backup[%d]: create kthread task failed! ret=%p\n", __LINE__, pid);
        return BSP_ERROR;
    }

    g_ap_dump_ctrl.dump_task_id = (uintptr_t)pid;

    param.sched_priority = 97;
    if (BSP_OK != sched_setscheduler(pid, SCHED_FIFO, &param))
    {
        ap_dump_error("bsp_dump_save_backup[%d]: sched_setscheduler failed!\n", __LINE__);
        return BSP_ERROR;
    }

    g_ap_dump_ctrl.dump_task_job = 0;

    ap_dump_fetal("ap_dump_save_task_init finish\n");

    return BSP_OK;
}


/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_base_info_init
* ¹¦ÄÜÃèÊö  : ³õÊ¼»¯apµÄ»ù±¾ÐÅÏ¢
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 ap_dump_base_info_init(void)
{

    g_ap_fileds_addr.base_info_addr = bsp_ap_dump_register_field(DUMP_KERNEL_BASE_INFO, "BASE_INFO", NULL,NULL,DUMP_KERNEL_BASE_INFO_SIZE,0);

    if(g_ap_fileds_addr.base_info_addr == NULL)
    {
        ap_dump_fetal("register DUMP_KERNEL_BASE_INFO fail\n");
        return BSP_ERROR;
    }
    else
    {
        g_dump_base_info = (ap_dump_base_info_s*)g_ap_fileds_addr.base_info_addr;
    }

    memset(g_dump_base_info, 0, sizeof(ap_dump_base_info_s));

    g_dump_base_info->vec = DUMP_ARM_VEC_UNKNOW;

    /*coverity[buffer_size_warning] */
    (void)strncpy((char*)g_dump_base_info->version,(char*)bsp_version_get_firmware(),sizeof(g_dump_base_info->version));
    /*coverity[buffer_size_warning] */
    (void)strncpy((char*)g_dump_base_info->compile_time,(char*)bsp_version_get_build_date_time(),sizeof(g_dump_base_info->compile_time));


    ap_dump_fetal("ap_dump_base_info_init finish\n");

    return BSP_OK;
}

/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_task_info_init
* ¹¦ÄÜÃèÊö  : ³õÊ¼»¯apµÄ»ù±¾ÐÅÏ¢
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 ap_dump_task_info_init(void)
{
    u32 i = 0;

    g_ap_fileds_addr.task_tcb_addr = bsp_ap_dump_register_field(DUMP_KERNEL_ALLTASK_TCB, "ALLTASK_TCB", NULL,NULL,DUMP_KERNEL_ALLTASK_TCB_SIZE,0);

    if(g_ap_fileds_addr.task_tcb_addr == NULL)
    {
        ap_dump_fetal("register DUMP_AP_ALLTASK_TCB fail\n");
        return BSP_ERROR;
    }
    else
    {
        g_ap_dump_task_info = (ap_dump_task_info_s*)g_ap_fileds_addr.task_tcb_addr;
        memset((void*)g_ap_fileds_addr.task_tcb_addr, 0, DUMP_KERNEL_ALLTASK_TCB_SIZE);
    }

    for(i = 0; i < DUMP_LINUX_TASK_NUM_MAX; i++)
    {
        g_ap_dump_task_info[i].pid = 0xffffffff;
    }

    ap_dump_fetal("ap_dump_task_info_init finish\n");

    return BSP_OK;
}

/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_match_rdr_id
* ¹¦ÄÜÃèÊö  : Æ¥Åärdr
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
u32 ap_dump_match_rdr_id(u32 drv_mod_id)
{
    u32 rdr_mod_id = RDR_AP_DUMP_NORMAL_EXC_MOD_ID;

    if(drv_mod_id == DRV_ERRNO_DUMP_ARM_EXC)
    {
        switch(g_dump_base_info->vec)
        {
            case DUMP_ARM_VEC_RESET:
                rdr_mod_id = RDR_AP_DUMP_ARM_RESET_MOD_ID;
                break;
            case DUMP_ARM_VEC_UNDEF:
                rdr_mod_id = RDR_AP_DUMP_ARM_UNDEF_MOD_ID;
                break;
            case DUMP_ARM_VEC_SWI:
                rdr_mod_id = RDR_AP_DUMP_ARM_SWI_MOD_ID;
                break;
            case DUMP_ARM_VEC_PREFETCH :
                rdr_mod_id = RDR_AP_DUMP_ARM_PREFETCH_MOD_ID;
                break;
            case DUMP_ARM_VEC_DATA :
                rdr_mod_id = RDR_AP_DUMP_ARM_DATA_MOD_ID;
                break;
            case DUMP_ARM_VEC_IRQ :
                rdr_mod_id = RDR_AP_DUMP_ARM_IRQ_MOD_ID;
                break;
            case DUMP_ARM_VEC_FIQ :
                rdr_mod_id = RDR_AP_DUMP_ARM_FIQ_MOD_ID;
                break;
            default:
                rdr_mod_id = RDR_AP_DUMP_ARM_UNKNOW_MOD_ID;
                break;
        }

    }
    return rdr_mod_id;

}

/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_timer_handler
* ¹¦ÄÜÃèÊö  : ap¶¨Ê±Æ÷ÖÐ¶Ï´¦Àíº¯Êý
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_timer_handler(u32 param)
{
    ap_dump_save_task_name();

    bsp_softtimer_add(&g_ap_dump_task_name_ctrl.task_name_timer);
}


/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_start_timer
* ¹¦ÄÜÃèÊö  : ´´½¨ap dump±£´æÈÎÎñÃûµÄ¶¨Ê±Æ÷
*
* ÊäÈë²ÎÊý  :  time_out ³¬Ê±Ê±¼ä
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 ap_dump_start_task_name_timer(u32 time_out)
{
    s32 ret = 0;

    g_ap_dump_task_name_ctrl.task_name_timer.func = ap_dump_timer_handler;
    g_ap_dump_task_name_ctrl.task_name_timer.para = 0;
    g_ap_dump_task_name_ctrl.task_name_timer.timeout = time_out;
    g_ap_dump_task_name_ctrl.task_name_timer.wake_type = SOFTTIMER_NOWAKE;

    ret =  bsp_softtimer_create(&g_ap_dump_task_name_ctrl.task_name_timer);
    if(ret)
    {
        return BSP_ERROR;
    }

    bsp_softtimer_add(&g_ap_dump_task_name_ctrl.task_name_timer);

    return BSP_OK;
}


/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_stop_timer
* ¹¦ÄÜÃèÊö  : Í£Ö¹ap dump±£´æÈÎÎñÃûµÄ¶¨Ê±Æ÷
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_stop_task_name_timer(void)
{
    if(g_ap_dump_task_name_ctrl.task_name_timer.init_flags!=TIMER_INIT_FLAG)
    {
        ap_dump_fetal("timer is null\n");

        return;
    }

    (void)bsp_softtimer_delete(&g_ap_dump_task_name_ctrl.task_name_timer);

    (void)bsp_softtimer_free(&g_ap_dump_task_name_ctrl.task_name_timer);
}

/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_register_fileds
* ¹¦ÄÜÃèÊö  : ×¢²áfiledÐÅÏ¢
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_register_fileds(void)
{

    g_ap_fileds_addr.task_stack_addr = bsp_ap_dump_register_field(DUMP_KERNEL_TASK_STACK,  "TASK_STACK",  NULL,NULL,DUMP_KERNEL_TASK_STACK_SIZE,0);
    if(g_ap_fileds_addr.task_stack_addr == NULL)
    {
        ap_dump_fetal("alloc DUMP_AP_TASK_STACK fail\n");
    }

    g_ap_fileds_addr.usr_data_addr = bsp_ap_dump_register_field(DUMP_KERNEL_USER_DATA,   "USER_DATA", NULL,NULL,DUMP_KERNEL_USER_DATA_SIZE,0);
    if(g_ap_fileds_addr.usr_data_addr  == NULL)
    {
        ap_dump_fetal("alloc DUMP_AP_USER_DATA fail\n");
    }

    g_ap_fileds_addr.print_log_addr= bsp_ap_dump_register_field(DUMP_KERNEL_PRINT,   "PRINT", NULL,NULL,DUMP_KERNEL_PRINT_SIZE,0);
    if(g_ap_fileds_addr.print_log_addr  == NULL)
    {
        ap_dump_fetal("alloc DUMP_AP_USER_DATA fail\n");
    }
    ap_dump_fetal("ap_dump_register_fileds finish\n");
}


/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_set_vec
* ¹¦ÄÜÃèÊö  : ¸üÐÂ»ù±¾ÐÅÏ¢µÄÒì³£ÏòÁ¿
*
* ÊäÈë²ÎÊý  :
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_set_vec(u32 vec)
{
    g_dump_base_info->vec= vec;
}

/*****************************************************************************
* º¯ Êý Ãû  : ap_dump_save_file
* ¹¦ÄÜÃèÊö  : ±£´ælogÎÄ¼þ
*
* ÊäÈë²ÎÊý  : file_name ÎÄ¼þÃû
*             addr ÄÚÈÝµØÖ·
*             len ÄÚÈÝ³¤¶È
*
* Êä³ö²ÎÊý  :

* ·µ »Ø Öµ  :

*
* ÐÞ¸Ä¼ÇÂ¼  : 2016Äê1ÔÂ4ÈÕ17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_save_file(char * file_name, void * addr, u32 len)
{
    int ret;
    int fd;
    int bytes;

    fd = bsp_open(file_name, O_CREAT|O_RDWR|O_SYNC, 0660);
    if(fd < 0)
    {
        ap_dump_error("creat file %s failed\n", file_name);
        return;
    }

    bytes = bsp_write(fd, addr, len);
    if(bytes != len)
    {
        ap_dump_error("write data to %s failed, bytes %d, len %d\n", file_name, bytes, len);
        (void)bsp_close(fd);
        return;
    }


    ret = bsp_close(fd);
    if(0 != ret)
    {
        ap_dump_error("close file failed, ret = %d\n", ret);
        return;
    }
    return;
}


