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

#include <product_config.h>

#include <osl_types.h>
#include <osl_malloc.h>

#include <linux/hisi/rdr_pub.h>
#include <linux/notifier.h>
#include <linux/mm.h>
#include <linux/spinlock.h>
#include <linux/rtc.h>
#include <linux/suspend.h>
#include <linux/atomic.h>

#include "bsp_slice.h"
#include "bsp_onoff.h"

#include "rdr_exc.h"
#include "rdr_debug.h"
#include "rdr_core.h"
#include "rdr_comm.h"
#include "rdr_area.h"
#include "rdr_bootcheck.h"
#include "rdr_module.h"



struct rdr_core_ctrl_info_s g_rdr_core_ctrl;

/*
 * func name: rdr_register_exception_type
 * func args:
 *   struct rdr_exception_info_s* s_e_type
 *
 * return value		e_modid
 *	== 0 error
 *	>0 success
 */
u32 rdr_register_exception(struct rdr_exception_info_s* exc_info)
{
    if(NULL == exc_info)
    {
        rdr_err("register invalid parameter!\n");
        show_stack(current, NULL);
        return 0;
    }
    if(g_rdr_core_ctrl.initFlag == false)
    {
        rdr_err("not initial!\n");
        return 0;
    }
    if(rdr_register_module_exception(exc_info->e_from_core, exc_info))
    {
        rdr_err("register err! 0x%llx\n",exc_info->e_from_core);
        return 0;
    }

    return exc_info->e_modid_end;
}
EXPORT_SYMBOL(rdr_register_exception);
/*
 * func name: rdr_register_module_ops
 * func args:
 *   u32 core_id,       core id;
 *      .
 *   struct rdr_module_ops_pub* ops;
 *   struct rdr_register_module_result* retinfo;
 * return value		e_modid
 *	< 0 error
 *	>=0 success
 */
int rdr_register_module_ops(
        u64 coreid,
        struct rdr_module_ops_pub* ops,
        struct rdr_register_module_result* retinfo
		)
{
    if((coreid >= RDR_MAX)||(NULL == ops) ||(NULL == retinfo))
    {
        rdr_err("invalid core id or parameters!~ 0x%llx 0x%p 0x%p\n",coreid,ops,retinfo);
        return RDR_ERROR;
    }

    if(rdr_register_module_callback_ops(coreid,ops))
    {
        rdr_err("register module callback err!~ 0x%llx\n",coreid);
        return RDR_ERROR;
    }

    if(rdr_get_module_areainfo(coreid,retinfo))
    {
        rdr_err("get module areainfo err!~ 0x%llx\n",coreid);
        return RDR_ERROR;
    }

    return RDR_OK;
}
EXPORT_SYMBOL(rdr_register_module_ops);

/*
 * func name: rdr_system_error
 * func args:
 *   u32 modid,
 *   u32 arg1,
 *   u32 arg2
 *   return void
 */
void rdr_system_error(u32 modid, u32 arg1, u32 arg2)
{
	struct rdr_syserr_param_s *     syserr_info = NULL;
	struct rdr_exception_info_s *   exc_info = NULL;
	struct list_head                *p,*n;
	struct rdr_syserr_param_s *     cur = NULL;
    unsigned long syserr_flags;
    char* moudle_name = NULL;

	if (in_atomic() || irqs_disabled() || in_irq())
    {
        rdr_err("in_atomic or irqs_disabled or in_irq!\n");
	}
    rdr_err("===============================================\n");
    rdr_err("[%s] enter system_error,modid:0x%x\n",\
        (moudle_name=rdr_get_module_name(modid))?moudle_name:"unknown",modid);
    rdr_err("===============================================\n");
    show_stack(current, NULL);
    show_mem(0);


    if(g_rdr_core_ctrl.initFlag == false)
    {
        rdr_err("not initial!\n");
        return;
    }
    syserr_info = (struct rdr_syserr_param_s*)kmalloc(sizeof(struct rdr_syserr_param_s), GFP_ATOMIC);
    if(NULL == syserr_info)
    {
        rdr_err("kmalloc failed  :0x%x!~\n",modid);
        return;
    }

    exc_info = rdr_get_exception_info(modid);
    spin_lock_irqsave(&g_rdr_core_ctrl.spinlock, syserr_flags);
    if(NULL == exc_info)/*未找到异常类型则为未定义处理，直接添加到链表尾*/
    {
        spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock, syserr_flags);
        kfree(syserr_info);
        rdr_err("get exc info by modid err :0x%x!~\n",modid);
        return;
    }
    else if(exc_info->e_reentrant == RDR_REENTRANT_DISALLOW)/*不允许同一个modeid重入，则遍历链表，查询是否有重复modeid*/
    {
        list_for_each_safe(p, n, &g_rdr_core_ctrl.syserr_list)
        {
            cur = (struct rdr_syserr_param_s*)list_entry(p, struct rdr_syserr_param_s, syserr_list);
            if (cur->modid == syserr_info->modid)
            {
                spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock, syserr_flags);
                rdr_err("modid[0x%x] is in processing,do no allow process twice!\n",modid);
                kfree(syserr_info);
                return;
            }
        }

    }

    syserr_info->modid = modid;
    syserr_info->arg1  = arg1;
    syserr_info->arg2  = arg2;

    list_add_tail(&syserr_info->syserr_list, &g_rdr_core_ctrl.syserr_list);
	spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock, syserr_flags);

    if(RDR_DEBUG_SW(RDR_DEBUG_EXCEPTION_PROCESS_SW))
        return;
    osl_sem_up(&g_rdr_core_ctrl.rdr_sem);
    return;
}
EXPORT_SYMBOL(rdr_system_error);
/*
 * func name: rdr_get_highprio_exc
 * func args:
 *   return u32
 */
u32 rdr_get_highprio_exc(struct rdr_syserr_param_s * syserr_parm,struct rdr_exception_info_s* exc_info)
{
	struct list_head *cur = NULL;
    struct list_head *next = NULL;

	struct rdr_syserr_param_s *cur_syserr = NULL;
	struct rdr_syserr_param_s *p_syserr_info = NULL;

    struct rdr_exception_info_s *cur_exc = NULL;
	struct rdr_exception_info_s *p_exce_info = NULL;
    unsigned long exc_flags;
    u32 e_priority = RDR_PPRI_MAX;

	spin_lock_irqsave(&g_rdr_core_ctrl.spinlock,exc_flags);

	list_for_each_safe(cur, next, &g_rdr_core_ctrl.syserr_list) {
		cur_syserr  = list_entry(cur, struct rdr_syserr_param_s, syserr_list);
		cur_exc     = rdr_get_exception_info(cur_syserr->modid);
        if(NULL == cur_exc){
            rdr_err("Invalid modeid :0x%x\n",cur_syserr->modid);
            list_del(cur);
            continue;
        }
        if(cur_exc->e_process_priority >= RDR_PPRI_MAX){
            rdr_err("Invalid process priority :0x%x,modeid :0x%x,core[%s]\n",cur_exc->e_process_priority,cur_syserr->modid,rdr_get_core_name(cur_exc->e_from_core));
            list_del(cur);
            continue;
        }
        if(cur_exc->e_process_priority < e_priority){
            p_syserr_info = cur_syserr;
            p_exce_info   = cur_exc;

            e_priority    = cur_exc->e_process_priority;
        }
	}

	if (p_syserr_info == NULL || p_exce_info == NULL) {
		spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock,exc_flags);
		return RDR_ERROR;
	}
    memcpy(syserr_parm,p_syserr_info,sizeof(*p_syserr_info));
    memcpy(exc_info,p_exce_info,sizeof(*p_exce_info));

    /*删除异常参数节点*/
	list_del(&p_syserr_info->syserr_list);
	spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock,exc_flags);
    kfree(p_syserr_info);

    return RDR_OK;
}

/*
 * func name: rdr_dump_done
 * func args:
 *   u32 modid
 *   u64 coreid
 * return
 *	 NULL.
 * callback func.
 */
void rdr_dump_done(u32 modid, u64 coreid)
{
    unsigned long exc_flags;
    rdr_err("[%s],modid :0x%x dump done!~\n",rdr_get_core_name(coreid),modid);

    if(g_rdr_core_ctrl.current_modeid != modid)
    {
        rdr_err("not this time mode id,current[0x%x],[0x%x]\n",g_rdr_core_ctrl.current_modeid,modid);
        return;
    }

    spin_lock_irqsave(&g_rdr_core_ctrl.spinlock, exc_flags);
    if(g_rdr_core_ctrl.dump_notify_core & coreid)
    {
        g_rdr_core_ctrl.dump_notify_core &= ~coreid;
        spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock, exc_flags);
        return;
    }
    spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock, exc_flags);
    rdr_err("invalid coreid to dump done! 0x%llx\n",coreid);
    return;
}

u32 rdr_notify_module_dump(u32 modid,struct rdr_exception_info_s* exc_info,char* path)
{
    u64 dump_core_mask = exc_info->e_notify_core_mask;
    struct rdr_module_ops_pub ops;
    int i;
    unsigned long exc_flags;

    if(RDR_DEBUG_SW(RDR_DEBUG_DUMP_SW))
        return RDR_OK;


    /*MBB产品异常需要通知所有子系统*/
    dump_core_mask = RDR_AP|RDR_CP|RDR_LPM3|RDR_TEEOS|RDR_MDMAP;

    spin_lock_irqsave(&g_rdr_core_ctrl.spinlock, exc_flags);
    g_rdr_core_ctrl.dump_notify_core |= dump_core_mask;
    g_rdr_core_ctrl.current_modeid    = modid;
    spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock, exc_flags);

    /*first callback exc core or module*/
    if(dump_core_mask & exc_info->e_from_core)
    {
        memset(&ops,0,sizeof(ops));
        if(rdr_get_module_ops(exc_info->e_from_core,&ops))
        {
            rdr_err("get module callback ops err!\n");
            return RDR_ERROR;
        }
        if(ops.ops_dump)
        {
            ops.ops_dump(modid,exc_info->e_exce_type,exc_info->e_from_core, path,rdr_dump_done);
        }
        else
        {
            spin_lock_irqsave(&g_rdr_core_ctrl.spinlock, exc_flags);
            g_rdr_core_ctrl.dump_notify_core &= ~(exc_info->e_from_core);/*该子系统未注册dump回调，不需要继续等待*/
            spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock, exc_flags);
        }
        dump_core_mask &= ~(exc_info->e_from_core);
    }

    for(i = 0;i < MNTN_MAX;i++)
    {
        memset(&ops,0,sizeof(ops));
        if(dump_core_mask & RDR_CORE(i))
        {
            if(rdr_get_module_ops(RDR_CORE(i),&ops))
            {
                rdr_err("get module callback ops err! 0x%x\n",RDR_CORE(i));
                dump_core_mask &= ~(RDR_CORE(i));
                spin_lock_irqsave(&g_rdr_core_ctrl.spinlock, exc_flags);
                g_rdr_core_ctrl.dump_notify_core &= ~(RDR_CORE(i));/*未通知该子系统，则不需要继续等待*/
                spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock, exc_flags);
                continue;
            }
            if(ops.ops_dump)
            {
                ops.ops_dump(modid,exc_info->e_exce_type,RDR_CORE(i), path,rdr_dump_done);
            }
            else
            {
                spin_lock_irqsave(&g_rdr_core_ctrl.spinlock, exc_flags);
                g_rdr_core_ctrl.dump_notify_core &= ~(RDR_CORE(i));/*该子系统未注册dump回调，不需要继续等待*/
                spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock, exc_flags);
            }
            dump_core_mask &= ~(RDR_CORE(i));
        }
    }

    if(dump_core_mask)
    {
        rdr_err("notify other core mask is err! 0x%llx\n",exc_info->e_notify_core_mask);
    }

    rdr_callback(exc_info,modid,path);

    return RDR_OK;
}
u32 rdr_notify_module_reset(u32 modid,struct rdr_exception_info_s* exc_info)
{
    struct rdr_module_ops_pub ops;

    if(RDR_DEBUG_SW(RDR_DEBUG_RESET_SW))
        return RDR_OK;

    memset(&ops,0,sizeof(ops));
    if(rdr_get_module_ops(RDR_AP,&ops))
    {
        rdr_err("get module callback ops err! 0x%x\n",RDR_AP);
    }
    if(ops.ops_reset)
    {
        ops.ops_reset(modid,exc_info->e_exce_type, exc_info->e_from_core);
    }

    return RDR_OK;
}

void rdr_wait_module_dump_done(void)
{
    int time = g_rdr_core_ctrl.wait_max_time;
    RDR_WAIT_DONE(g_rdr_core_ctrl.dump_notify_core, time);
    if(time <= 0)
    {
        rdr_err("wait module dump done time out %d s\n",g_rdr_core_ctrl.wait_max_time);
    }
}

/*
 * func name: rdr_entry_exc_process
 * func args:
 *   return u32
 */
u32 rdr_entry_exc_process(struct rdr_syserr_param_s * syserr_parm,struct rdr_exception_info_s* exc_info)
{
    char date[24];
    char  dstpath[128];
    struct rdr_area_mntn_addr_info_s area_info;


    memset(date,0,sizeof(date));
    rdr_get_date_timestamp(date,sizeof(date));

    rdr_init_baseinfo();
    (void)rdr_fill_syserr_para(syserr_parm->modid,syserr_parm->arg1,syserr_parm->arg2);
    (void)rdr_fill_exc_info(exc_info,date);

    /*打印异常信息*/
    printk(KERN_ERR"<<<<%s>>>>\n",date);
    rdr_print_one_exc(exc_info);

    atomic_set(&g_rdr_core_ctrl.save_state, 1);
    if(rdr_create_exc_dir(RDR_EXC_DUMP_PATH,dstpath,sizeof(dstpath),date))
    {
        rdr_err("create exc dir fail!\n");
        atomic_set(&g_rdr_core_ctrl.save_state, 0);
        return RDR_ERROR;
    }

    /*notify other core dump*/
    (void)rdr_notify_module_dump(syserr_parm->modid,exc_info,dstpath);

    /*wait other core process done*/
    rdr_wait_module_dump_done();

    /*get mntn addr info*/
    memset(&area_info,0,sizeof(area_info));
    rdr_get_mntn_addr_info(&area_info);


    /*save rdr.bin*/
    if((area_info.vaddr)&&(area_info.len))
    {
        strcat(dstpath,RDR_BIN_NAME);
        if(rdr_save_file(dstpath,area_info.vaddr,area_info.len,RDR_ERROR,false)){
        }
        else
        {
            rdr_fill_save_done();/*设置log保存完成标志*/
        }
    }
    atomic_set(&g_rdr_core_ctrl.save_state, 0);

    /*notify other core reset*/
    (void)rdr_notify_module_reset(syserr_parm->modid,exc_info);

    return RDR_OK;
}

s32 rdr_syserr_task(void* arg)
{
    struct rdr_syserr_param_s syserr_parm;
    struct rdr_exception_info_s exc_info;
    unsigned long exc_flags;

    while(!kthread_should_stop())
    {
        osl_sem_down(&g_rdr_core_ctrl.rdr_sem);

        /*链表如果为空则返回*/
        spin_lock_irqsave(&g_rdr_core_ctrl.spinlock, exc_flags);
        if(list_empty(&g_rdr_core_ctrl.syserr_list)){
            spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock, exc_flags);
            continue;
        }
        spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock, exc_flags);

        rdr_err("============wait for fs ready start =============\n");
        while(rdr_wait_partition("/modem_log/lost+found", 10) != 0);
        rdr_err("============wait for fs ready e n d =============\n");

        memset(&syserr_parm,0,sizeof(syserr_parm));
        memset(&exc_info,0,sizeof(exc_info));

        wake_lock(&g_rdr_core_ctrl.rdr_wakelock);

        /*获取优先级最高的异常先处理*/
        if(rdr_get_highprio_exc(&syserr_parm,&exc_info))
        {
            wake_unlock(&g_rdr_core_ctrl.rdr_wakelock);
            continue;
        }

        if(rdr_entry_exc_process(&syserr_parm,&exc_info))
        {
            rdr_err("exc process err! modid:0x%x,[%s],[%s]\n",\
                syserr_parm.modid,rdr_get_exception_type(exc_info.e_exce_type),exc_info.e_from_module);
        }
        wake_unlock(&g_rdr_core_ctrl.rdr_wakelock);
        osl_sem_up(&g_rdr_core_ctrl.rdr_sem);
        continue;
    }
    return 0;
}

/*
 * func name: rdr_fetal_system_error
 * func args:
 *   u32 modid,
 *   u32 arg1,
 *   u32 arg2
 *   return void
 */
void rdr_fetal_system_error(u32 modid, u32 arg1, u32 arg2)
{
    char date[24];
    struct rdr_exception_info_s *   exc_info = NULL;
    char* moudle_name = NULL;
    u32 slice;

	if (in_atomic() || irqs_disabled() || in_irq())
    {
        rdr_err("in_atomic or irqs_disabled or in_irq!\n");
	}
    rdr_err("===============================================\n");
    rdr_err("[%s] enter system_error,modid:0x%x\n",\
        (moudle_name=rdr_get_module_name(modid))?moudle_name:"unknown",modid);
    rdr_err("===============================================\n");

	preempt_disable();
	show_stack(current, NULL);
    show_mem(0);

    rdr_init_baseinfo();
    (void)rdr_fill_syserr_para(modid,arg1,arg2);

    memset(date,0,sizeof(date));
    rdr_get_date_timestamp(date,sizeof(date));

    exc_info = rdr_get_exception_info(modid);
    if(NULL == exc_info)
    {
        rdr_err("invalid mode id:0x%x~\n",modid);
        /*无效modeid 立刻重启*/
        bsp_drv_power_reboot_direct();
        return;
    }
    (void)rdr_fill_exc_info(exc_info,date);

    (void)rdr_notify_module_dump(modid,exc_info,RDR_EXC_DUMP_PATH);

    slice = bsp_get_elapse_ms();
    while(g_rdr_core_ctrl.dump_notify_core)
    {
        if(5000 < (bsp_get_elapse_ms()-slice))/*超过5s直接跳出*/
            break;
    }

    (void)rdr_notify_module_reset(modid,exc_info);
}
EXPORT_SYMBOL(rdr_fetal_system_error);

static struct notifier_block rdr_suspend_notifier;
static int rdr_suspend_nb(struct notifier_block *this,
			 unsigned long event, void *ptr)
{
	switch (event) {
	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:
		rdr_err("resume +\n");
		atomic_set(&g_rdr_core_ctrl.pm_state, 0);
		rdr_err("resume -\n");
		break;

	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:
		rdr_err("suspend +\n");
		atomic_set(&g_rdr_core_ctrl.pm_state, 1);
		while (1) {
			if (atomic_read(&g_rdr_core_ctrl.save_state))
				msleep(1000);
			else
				break;
		}
		rdr_err("suspend -\n");
		break;
	default:
		return NOTIFY_DONE;
	}
	return NOTIFY_OK;
}

void rdr_onoff_notifier(void)
{
    atomic_set(&g_rdr_core_ctrl.reboot_state,1);
    rdr_fill_reboot_done();
}
EXPORT_SYMBOL(rdr_onoff_notifier);

void rdr_register_notifier(void)
{
    /*pm notifier*/
	/* Register to get PM events */
	rdr_suspend_notifier.notifier_call = rdr_suspend_nb;
	rdr_suspend_notifier.priority = -1;
	if (register_pm_notifier(&rdr_suspend_notifier))
    {
		rdr_err("Failed to register for PM events\n");
	}
    /*reboot notifier*/
    bsp_reboot_callback_register(rdr_onoff_notifier);
    return;
}

/*
 * func name: rdr_core_init
 * func args:
 *   return u32
 */
u32 rdr_core_init(void)
{
    if(g_rdr_core_ctrl.initFlag == true)
    {
        return RDR_OK;
    }
    osl_sem_init(0, &g_rdr_core_ctrl.rdr_sem);
    INIT_LIST_HEAD(&g_rdr_core_ctrl.syserr_list);
    wake_lock_init(&g_rdr_core_ctrl.rdr_wakelock,WAKE_LOCK_SUSPEND,"rdr_wakelock");
    spin_lock_init(&g_rdr_core_ctrl.spinlock);
    g_rdr_core_ctrl.wait_max_time = 1000;/*最大等待时间100s*/

    rdr_register_notifier();

    if(osl_task_init("rdr_syserr", 99,0x2000,(OSL_TASK_FUNC)rdr_syserr_task,NULL,&g_rdr_core_ctrl.task_id))
    {
        rdr_err("create rdr_syserr task err!\n");
        return RDR_ERROR;
    }

    rdr_debug_register_debug_info("Show all occure exception info! ",rdr_core_show_exception_info,0);

    g_rdr_core_ctrl.initFlag = true;
    rdr_err("ok!\n");
    return RDR_OK;
}

void rdr_core_show_exception_info(u32 arg)
{
    struct list_head *p,*n;
    struct rdr_syserr_param_s* cur;
    struct rdr_exception_info_s* exc_info;
    unsigned long syserr_flags;
    char* module_name = NULL;

    spin_lock_irqsave(&g_rdr_core_ctrl.spinlock, syserr_flags);
    list_for_each_safe(p, n, &g_rdr_core_ctrl.syserr_list)
    {
        cur = (struct rdr_syserr_param_s*)list_entry(p,struct rdr_syserr_param_s, syserr_list);
        exc_info = rdr_get_exception_info(cur->modid);
        printk(KERN_ERR"[%s] enter system error!\n",(module_name=rdr_get_module_name(cur->modid))?module_name:"unknown");
        rdr_print_one_exc(exc_info);
    }
    spin_unlock_irqrestore(&g_rdr_core_ctrl.spinlock, syserr_flags);
}

static s32 __init rdr_init(void)
{
    if(rdr_area_init())
    {
        rdr_err("rdr area init err!\n");
    }
    /*rdr module init*/
    if(rdr_module_init())
    {
        rdr_err("rdr comm init err!\n");
    }

    /*rdr core init*/
    if(rdr_core_init())
    {
        rdr_err("rdr core init err!\n");
    }

    /*rdr bootcheck*/
    if(rdr_bootcheck_init())
    {
        rdr_err("rdr bootcheck init err!\n");
    }
    return RDR_OK;
}


static void __exit rdr_exit(void)
{
}

core_initcall_sync(rdr_init);
module_exit(rdr_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Balong Drv_I/Msp Team");
MODULE_DESCRIPTION("Mntn mananger Module");


