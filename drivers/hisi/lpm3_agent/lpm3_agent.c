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


#include "lpm3_agent.h"
#include "bsp_ap_dump.h"



struct rdr_exception_info_s g_lpm_exc_info[] =
{
    {
        .e_modid            = HISI_MNTN_EXC_LPM3_MOD_ID,
        .e_modid_end        = HISI_MNTN_EXC_LPM3_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask  = RDR_LPM3,
        .e_from_core        = RDR_LPM3,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = LPM3_S_EXCEPTION,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "LPM3",
        .e_desc             = "lpm3 exc wait ap reset system",
    },
    {
        .e_modid            = HISI_MNTN_EXC_LPM3_WDT_MOD_ID,
        .e_modid_end        = HISI_MNTN_EXC_LPM3_WDT_MOD_ID,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority  = RDR_REBOOT_WAIT,
        .e_notify_core_mask = RDR_AP | RDR_CP | RDR_LPM3,
        .e_reset_core_mask  = RDR_LPM3,
        .e_from_core        = RDR_LPM3,
        .e_reentrant        = RDR_REENTRANT_DISALLOW,
        .e_exce_type        = LPM3_S_WDT_TIMEOUT,
        .e_upload_flag      = RDR_UPLOAD_YES,
        .e_from_module      = "LPM3 WDT",
        .e_desc             = "lpm3 wdt exc",
    },
};

struct lpm3_rdr_exc_info g_rdr_lpm3_exc_info;
struct lpm3_agent_ctrl_info g_lpm3_ctrl;

void lpm3_dump_callback(u32 modid, u32 etype, u64 coreid, char* logpath, pfn_cb_dump_done fndone)
{
    /*若不是M3发生的异常则发送ipc中断通知m3*/
    if(!g_lpm3_ctrl.is_lpm3exc)
    {
	    bsp_ipc_int_send(IPC_CORE_MCORE, IPC_MCU_INT_SRC_DUMP);
    }

    if(!g_lpm3_ctrl.ulInitstate)
    {
        lpm3_err("lpm3 agent init fail!\n");
        return;
    }

    g_rdr_lpm3_exc_info.coreid=coreid;
    g_rdr_lpm3_exc_info.dump_done=fndone;
    g_rdr_lpm3_exc_info.modid=modid;

    osl_sem_up(&g_lpm3_ctrl.task_sem);
    lpm3_fetal("sem up!\n");
}

void lpm3_dump_reset(u32 modid, u32 etype, u64 coreid)
{
    lpm3_fetal("m3 dump save finished, wait ap reset!\n");
    return;
}

int lpm3_dump_task(void *data)
{
    u32 time_start;
    u32 time_out;
    u32 save_flag;
    dump_area_t* virt_addr;

    /*coverity[no_escape] */
    while(1)
    {

        osl_sem_down(&g_lpm3_ctrl.task_sem);

        lpm3_fetal("sem down!\n");

        /*循环检查LPM3是否保存log完成*/
        time_start = bsp_get_elapse_ms();
        time_out = 2000;

        do{
             virt_addr = (dump_area_t*)g_lpm3_ctrl.virt_addr;
             save_flag = readl((u32)virt_addr->area_head.version);
             if(DUMP_SAVE_SUCCESS == save_flag)
             {
                 lpm3_fetal("m3 dump save finished\n");
                 break;
             }

             if(time_out <= (bsp_get_elapse_ms()-time_start))
             {
                 lpm3_fetal("dump save max time out\n");
                 break;
             }
             msleep(10);
        }while(1);
        /* 通知RDR m3数据保存完成 */
        if(g_rdr_lpm3_exc_info.dump_done)
        {
            lpm3_fetal("notify rdr m3 dump save finished\n");
            g_rdr_lpm3_exc_info.dump_done(g_rdr_lpm3_exc_info.modid,g_rdr_lpm3_exc_info.coreid);
        }
    }
}

void lpm3_int_handle(s32 param)
{
    u32 modid = 0;
    dump_base_info_t* base_ctrl;
    g_lpm3_ctrl.is_lpm3exc = true;

    base_ctrl = (dump_base_info_t*)bsp_dump_get_field_addr(DUMP_M3_BASE_INFO);
    if(NULL != base_ctrl)
    {
        modid = base_ctrl->modId;
    }

    /*M3异常的情况下需要去使能M3看门狗中断*/
    bsp_wdt_irq_disable(WDT_MCORE_ID);

    bsp_coresight_disable();
    /* 停止记录任务/中断切换 */
    bsp_ap_dump_trace_stop();
    bsp_ap_dump_save_all_task_info();

    lpm3_fetal("[0x%x] m3 ccore enter system error!\n", bsp_get_slice_value());

    if (DRV_ERRNO_DUMP_AP_WDT == modid)
    {
        rdr_system_error(RDR_AP_DUMP_AP_WDT_MOD_ID,0, 0);
    }
    else
    {
        rdr_system_error(HISI_MNTN_EXC_LPM3_MOD_ID,0, 0);
    }
}

void lpm3_wdt_hook(void)
{
	dump_base_info_t* base_ctrl;
    lpm3_fetal("[0x%x] m3 wdt exception!\n", bsp_get_slice_value());

    base_ctrl = (dump_base_info_t*)bsp_dump_get_field_addr(DUMP_M3_BASE_INFO);
    if(NULL != base_ctrl)
    {
        writel((u32)LPM3_WDT_EXC,(u32)(&(base_ctrl->modId)));
    }

    rdr_system_error(HISI_MNTN_EXC_LPM3_WDT_MOD_ID,0, 0);
    lpm3_fetal("lpm3 wdt hook: rdr handle finished!\n");
}


static s32 __init lpm3_agent_init(void)
{
    struct rdr_module_ops_pub           soc_ops;
    struct rdr_register_module_result   soc_rst;
    s32 ret = -1;
    u32 i = 0;


    /*回调函数注册*/
    memset(&soc_rst,0,sizeof(soc_rst));
    soc_ops.ops_dump = lpm3_dump_callback;
    soc_ops.ops_reset= lpm3_dump_reset;
    ret = rdr_register_module_ops(RDR_LPM3,&soc_ops,&soc_rst);
    if (ret)
    {
        lpm3_err("rdr_register_module_ops fail!\n");
	    return -1;
    }

    if((!soc_rst.log_addr)||(!soc_rst.log_len))
    {
        lpm3_err("get soc_rst.log_addr fail!\n");
        return -1;
    }

    g_lpm3_ctrl.phy_addr = soc_rst.log_addr;
    g_lpm3_ctrl.length   = soc_rst.log_len;
    g_lpm3_ctrl.virt_addr= (void*)ioremap_wc(soc_rst.log_addr,soc_rst.log_len);
    if(NULL == g_lpm3_ctrl.virt_addr)
    {
        lpm3_err("ioremap soc_rst.log_addr fail!\n");
        return -1;
    }

    /*创建任务*/
    osl_sem_init(0, &g_lpm3_ctrl.task_sem);
    osl_task_init("lpm3_dump",96, 0x1000, (OSL_TASK_FUNC)lpm3_dump_task,NULL,&g_lpm3_ctrl.task_id);

    /*register exception info*/
    for(i=0; i<sizeof(g_lpm_exc_info)/sizeof(struct rdr_exception_info_s); i++)
    {
        if(rdr_register_exception(&g_lpm_exc_info[i]) == 0)
        {
            lpm3_err("rdr_register_exception 0x%x fail!\n", g_lpm_exc_info[i].e_modid);
            return -1;
        }
        lpm3_fetal("rdr register lpm3 exception success!\n");

    }

    /*register ipc exc*/
    ret = bsp_ipc_int_connect(IPC_ACPU_INT_MCU_SRC_DUMP, (voidfuncptr)lpm3_int_handle, 0);
    if(ret)
    {
        lpm3_err("rdr_lpm3_init: bsp_ipc_int_connect fail!\n");
        return -1;
    }

    ret = bsp_ipc_int_enable(IPC_ACPU_INT_MCU_SRC_DUMP);
    if(ret)
    {
        lpm3_err("rdr_lpm3_init: bsp_ipc_int_enable failed!\n");
        return -1;
    }

    /*regitster lpm3 wdt exc */
    ret = bsp_wdt_register_hook(WDT_MCORE_ID, lpm3_wdt_hook);
    if(ret)
    {
        lpm3_err("request_irq m3_wdt_irq_handler failed!\n");
	    return -1;
    }

    g_lpm3_ctrl.ulInitstate = true;

    return 0;
}

static void __exit lpm3_agent_exit(void)
{
}

subsys_initcall(lpm3_agent_init);
module_exit(lpm3_agent_exit);



