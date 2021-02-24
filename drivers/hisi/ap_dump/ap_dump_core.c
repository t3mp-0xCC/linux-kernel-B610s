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

/*lint -save -e537*/
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
#include "bsp_icc.h"
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
#include "ap_dump_mem.h"
#include "bsp_ap_dump.h"
#include "ap_dump_base.h"
#include "ap_dump_panic.h"
#include "ap_dump_queue.h"
#include "bsp_onoff.h"
#include "bsp_slice.h"
#include "drv_nv_id.h"
/*lint -restore*/
extern rdr_exc_info_s          g_ap_rdr_exc_info;
extern ap_dump_base_info_s        *g_dump_base_info;

ap_dump_core_ctrl_s            g_ap_dump_core_ctrl;

/* RDR异常类型定义 */
struct rdr_exception_info_s g_ap_exc_info[] = {
    {
        .e_modid            = RDR_AP_DUMP_ARM_RESET_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_ARM_RESET_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm exc reset",
    },
    {
        .e_modid            = RDR_AP_DUMP_ARM_UNDEF_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_ARM_UNDEF_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm exc undef",
    },
    {
        .e_modid            = RDR_AP_DUMP_ARM_PREFETCH_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_ARM_PREFETCH_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm exc prefectch",
    },
    {
        .e_modid            = RDR_AP_DUMP_ARM_DATA_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_ARM_DATA_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm exc databort",
    },
    {
        .e_modid            = RDR_AP_DUMP_ARM_FIQ_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_ARM_FIQ_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm exc fiq",
    },
    {
        .e_modid            = RDR_AP_DUMP_ARM_IRQ_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_ARM_IRQ_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm exc irq",
    },
    {
        .e_modid            = RDR_AP_DUMP_NORMAL_EXC_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_NORMAL_EXC_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_SFTRESET,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap normal soft reset",
    },
    {
        .e_modid            = RDR_AP_DUMP_PANIC_IN_INT_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_PANIC_IN_INT_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_EXC_PANIC_INT,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP",
        .e_desc             = "ap arm painc in int",
    },
    {
        .e_modid            = RDR_AP_DUMP_AP_WDT_MOD_ID,
        .e_modid_end        = RDR_AP_DUMP_AP_WDT_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3 ,
        .e_reset_core_mask  = RDR_AP,
        .e_from_core        = RDR_AP,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = AP_S_WDT_TIMEOUT,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "AP WDT",
        .e_desc             = "ap wdt exc",
    },
};



/*****************************************************************************
* 函 数 名  : ap_system_error
* 功能描述  : ap 复位接口
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_system_error(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length)
{
    u32 rdr_mod_id = RDR_AP_DUMP_NORMAL_EXC_MOD_ID;

    if(g_ap_dump_core_ctrl.exc_flag == true)
    {
        ap_dump_fetal("exception has happened can not deal new exception\n");
        return;
    }
    else
    {
        g_ap_dump_core_ctrl.exc_flag = true;
    }

    ap_dump_fetal("[0x%x]================ acore enter system error! ================\n", bsp_get_slice_value());
    ap_dump_fetal("mod_id=0x%x arg1=0x%x arg2=0x%x data=0x%p len=0x%x\n", mod_id, arg1, arg2, data, length);


    if (DUMP_INIT_FLAG != g_ap_dump_core_ctrl.init_flag)
    {
        ap_dump_fetal("Ap Dump not init,exit system_error process\n");
        return;
    }

    rdr_mod_id = ap_dump_match_rdr_id(mod_id);

    ap_dump_save_exc_task((uintptr_t)(&(g_dump_base_info->regSet[0])));

    bsp_coresight_disable();

    ap_dump_trace_stop();

    ap_dump_save_base_info(mod_id,arg1,arg2,data,length);

    ap_dump_save_usr_data(data, length);

    /*ap_dump_show_stack();*/

    ap_dump_save_all_task();

    ap_dump_stop_task_name_timer();

    ap_dump_save_task_name();

    if(ap_dump_is_panic() == true)
    {
        ap_dump_fetal("exception happen in interrupt go enter fatal process\n");

        rdr_fetal_system_error(RDR_AP_DUMP_PANIC_IN_INT_MOD_ID, arg1, arg2);

        return;
    }

    rdr_system_error(rdr_mod_id, arg1, arg2);

    return;
}


/*****************************************************************************
* 函 数 名  : bsp_ap_dump_trace_stop
* 功能描述  : 对外接口，用于停止记录trace信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void bsp_ap_dump_trace_stop(void)
{
    ap_dump_trace_stop();
}

/*****************************************************************************
* 函 数 名  : bsp_ap_dump_save_task_info
* 功能描述  : 对外接口，保存ap的所有的任务信息和任务名
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void bsp_ap_dump_save_all_task_info(void)
{
    ap_dump_save_exc_task((uintptr_t)(&(g_dump_base_info->regSet[0])));

    ap_dump_save_base_info(BSP_MODU_OTHER_CORE,0,0,NULL,0);

    ap_dump_save_all_task();

    ap_dump_stop_task_name_timer();

    ap_dump_save_task_name();
}


/*****************************************************************************
* 函 数 名  : ap_dump_callback
* 功能描述  : 注册给rdr的回调函数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_callback( u32 modid, u32 etype, u64 coreid,char* logpath, pfn_cb_dump_done fndone)
{

    ap_dump_fetal("enter dump callback, mod id:0x%x\n", modid);

    g_ap_rdr_exc_info.modid  = modid;
    g_ap_rdr_exc_info.coreid = coreid;
    g_ap_rdr_exc_info.dump_done = fndone;

    if((strlen(logpath) + strlen((char*)RDR_DUMP_FILE_AP_PATH)) >= RDR_DUMP_FILE_AP_LOG_PATH_LEN - 1)
    {
        ap_dump_fetal("log path is too long %s\n", logpath);
        return ;
    }
    memset(g_ap_rdr_exc_info.log_path,'\0',RDR_DUMP_FILE_AP_LOG_PATH_LEN);
    memcpy(g_ap_rdr_exc_info.log_path, logpath, strlen(logpath));
    memcpy((void*)(g_ap_rdr_exc_info.log_path + strlen(logpath)) , (void*)(RDR_DUMP_FILE_AP_PATH), strlen((void*)RDR_DUMP_FILE_AP_PATH));
    ap_dump_fetal("log path is %s\n", g_ap_rdr_exc_info.log_path);

    ap_dump_save_all_fields();

    ap_dump_save_log();

    ap_dump_save_and_reboot();

    return ;
}

/*****************************************************************************
* 函 数 名  : ap_dump_reset
* 功能描述  : 注册给rdr的复位函数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_reset(u32 modid, u32 etype, u64 coreid)
{

    s32 ret = BSP_OK;
    NV_DUMP_STRU            dump_cfg = {{0}};

    ap_dump_fetal("enter dump reset, mod id:0x%x\n", modid);

    ret = bsp_nvm_read(NV_ID_DRV_DUMP, (u8*)&dump_cfg, sizeof(NV_DUMP_STRU));

    if(ret != BSP_OK)
    {
        ap_dump_fetal("read nv error,not reset");

        return;
    }

    if(1 == dump_cfg.dump_cfg.Bits.sysErrReboot)
    {
        ap_dump_fetal("enter drv reboot process\n");

        ap_dump_save_log();

        bsp_drv_power_reboot_direct();
    }

}

/*****************************************************************************
* 函 数 名  : bsp_ap_dump_init
* 功能描述  : dump 模块初始化
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/

s32 __init bsp_ap_dump_init(void)
{

    struct rdr_module_ops_pub   soc_ops = {NULL,NULL};
    int i = 0;
    s32 ret = BSP_ERROR;

    ret = ap_dump_save_task_init();
    if(ret == BSP_ERROR)
    {
        ap_dump_fetal("ap_dump_save_task_init fail\n");
        return BSP_ERROR;
    }

    ret = ap_dump_base_info_init();
    if(ret == BSP_ERROR)
    {
        ap_dump_fetal("ap_dump_save_task_init fail\n");
        return BSP_ERROR;
    }

    ret = ap_dump_task_info_init();
    if(ret == BSP_ERROR)
    {
        ap_dump_fetal("ap_dump_task_info_init fail\n");
        return BSP_ERROR;
    }

    ret = ap_dump_task_name_init();
    if(ret == BSP_ERROR)
    {
        ap_dump_fetal("ap_dump_task_name_init fail\n");
        return BSP_ERROR;
    }

    ret = ap_dump_register_hook();
    if(ret == BSP_ERROR)
    {
        ap_dump_fetal("ap_dump_register_hook fail\n");
        return BSP_ERROR;
    }
    ap_dump_register_fileds();

    ap_dump_register_exc_notifier();

    g_ap_dump_core_ctrl.exc_flag = false;

    g_ap_dump_core_ctrl.init_flag = DUMP_INIT_FLAG;

    g_ap_rdr_exc_info.dump_done = BSP_NULL;

    for(i=0; i<sizeof(g_ap_exc_info)/sizeof(struct rdr_exception_info_s); i++)
    {
        if(rdr_register_exception(&g_ap_exc_info[i]) == 0)
        {
            ap_dump_fetal("dump init: rdr_register_exception 0x%x fail\n", g_ap_exc_info[i].e_modid);
            return BSP_ERROR;
        }
    }

    soc_ops.ops_dump  = (pfn_dump)ap_dump_callback;
    soc_ops.ops_reset = (pfn_reset)ap_dump_reset;

    if(rdr_register_module_ops(RDR_AP, &soc_ops, &g_ap_rdr_exc_info.soc_rst) != BSP_OK)
    {
        ap_dump_fetal("dump init: rdr_register_soc_ops fail\n");
        return BSP_ERROR;
    }

    ap_dump_fetal("dump init success!\n");

    return BSP_OK;
}
EXPORT_SYMBOL_GPL(ap_system_error);
arch_initcall_sync(bsp_ap_dump_init);

