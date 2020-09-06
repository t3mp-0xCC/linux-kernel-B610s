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

/*lint --e{537 } */
#include <linux/cpufreq.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/sched.h>	/* set_cpus_allowed() */
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_device.h>
#include <linux/of_address.h>

#include <osl_types.h>
#include <osl_sem.h>
#include <osl_thread.h>
#include <osl_irq.h>
#include <osl_malloc.h>
#include <bsp_hardtimer.h>
#include <bsp_icc.h>
#include <drv_comm.h>

#include <cpufreq_balong.h>

#ifdef __cplusplus
extern "C"
{
#endif
/*lint --e{24,34,35,43,50,63,64,110,78, 728,745,752, 958,808,438,550 } */
/******************************test for cpufreq start***********************************/
extern struct dbs_tuners dbs_tuners_ins;
extern int g_cpufreq_lock_status_flag;
extern int g_icc_run_flag;
extern int cpufreq_dfs_get_profile(void);

/*设置调频阈值 不用配置的项输入负值*/
void cpufreq_exc_change_limit(u32 sample, u32 up_cpuloadlimit, u32 down_cpuloadlimit, u32 down_times, u32 up_times)
{
	unsigned long flags = 0;
	local_irq_save(flags);
	if (up_cpuloadlimit <= down_cpuloadlimit)
	{
		cpufreq_err("ERROE: up_cpuloadlimit <= down_cpuloadlimit")	;
		goto out;
	}
	dbs_tuners_ins.sampling_rate = sample;

	dbs_tuners_ins.up_threshold = up_cpuloadlimit;
	
	dbs_tuners_ins.down_threshold = down_cpuloadlimit;

	dbs_tuners_ins.down_threshold_times = down_times;

	dbs_tuners_ins.up_threshold_times = up_times;
out:
	local_irq_restore(flags);
}


/*
 * g_dfs_flag控制是否计算占用
 * g_icc_flag_run控制是否调用icc通知CCORE
 */
void cpufreq_debug_stop(int flag, int flag1)
{
	g_cpufreq_lock_status_flag = flag;
 	g_icc_run_flag = flag1;
}


/*1 tick 3000000*/
static unsigned int busy_time = 10;
static int idle_time = 10;
static unsigned long start_time = 0;
void cpufreq_debug_cpuload(int busytime, int idletime)
{
	if (busytime)
	busy_time = (unsigned int)busytime;
	if (idletime)
	idle_time = idletime;
}
void cpufreq_debug_task(void)
{/*lint --e{732, 716 } */
	while (1)
	{
		start_time = bsp_get_elapse_ms();
		while (bsp_get_elapse_ms()-start_time <= busy_time)
		{
			;
		}
		msleep(idle_time);
		if (!idle_time)
		{
			break;
		}
	}
}
void cpufreq_debug_switch_load_auto(void)
{/*lint --e{716 } */
	int which_load_bound = 0;
	/* coverity[INFINITE_LOOP] */
	/* coverity[no_escape] */
	while(1)
	{
		/*每30s改变cpu占用，测试长时间调频是否正常*/
		(void)osl_task_delay(3000);
		switch(which_load_bound)
		{
			case 0:
				cpufreq_debug_cpuload(40, 1);//90
				break;
			case 1:
				cpufreq_debug_cpuload(10,1);//50
				break;
			case 2:
				cpufreq_debug_cpuload(13,1);//8-11
				break;
			default:
				break;
		}
		which_load_bound = (which_load_bound + 1) % 3;
	}
}
void cpufreq_debug_start_cpuload_task(void)
{
	OSL_TASK_ID task_id = 0;
	osl_task_init("pmu_test", 16, 0x1000 ,(void *)cpufreq_debug_task, NULL, &task_id);
}

u32 cpufreq_ddr_num = 1024;
u32 cpufreq_debug_ddr_size = 1024;
u32 cpufreq_debug_ddr_pro = 0;
u32 cpufreq_debug_ddr_flag = 1;
u32 cpufreq_debug_ddr_tick = 1;
void cpufreq_debug_change_ddr_size(u32 size, u32 tick)
{
    cpufreq_ddr_num = size;
    cpufreq_debug_ddr_tick = tick;
}


void cpufreq_debug_ddr_stress_task(void)
{
    u32 *ddt_test_addr = NULL;
    cpufreq_debug_ddr_size = cpufreq_ddr_num * cpufreq_ddr_num;

    ddt_test_addr = (u32 *)osl_malloc(cpufreq_debug_ddr_size);
    if (NULL == ddt_test_addr)
    {
        cpufreq_err("malloc failed\n");
        return;
    }
    while (cpufreq_debug_ddr_flag)
    {
        cpufreq_debug_ddr_pro = cpufreq_dfs_get_profile();
        memset(ddt_test_addr, 0x14141414, cpufreq_debug_ddr_size);
        if (memcmp(ddt_test_addr, ddt_test_addr + cpufreq_ddr_num, cpufreq_ddr_num))
        {
            cpufreq_err("ddr memcmp error have cache flush,cur pro:%d\n", cpufreq_debug_ddr_pro);
        }
        memset(ddt_test_addr, 0x28282828, cpufreq_debug_ddr_size);
        if (memcmp(ddt_test_addr, ddt_test_addr + cpufreq_ddr_num, cpufreq_ddr_num))
        {
            cpufreq_err("ddr memcmp error no cache flush,cur pro:%d\n", cpufreq_debug_ddr_pro);
        }
        (void)osl_task_delay(cpufreq_debug_ddr_tick);
    }
    osl_free((void *)ddt_test_addr);
}

void cpufreq_debug_ddr_task(void)
{
    OSL_TASK_ID ret = 0;
	(void)osl_task_init("testddr", 33, 4096, (OSL_TASK_FUNC)cpufreq_debug_ddr_stress_task, 0, &ret); /*lint !e64 !e119 */
}

void cpufreq_debug_start_stress_task(void)
{
	OSL_TASK_ID ret = 0;
	(void)osl_task_init("cpufreq_test_task", 2, 4096, (OSL_TASK_FUNC)cpufreq_debug_task, 0, &ret); /*lint !e64 !e119 */
	(void)osl_task_init("cpufreq_stress_test_task", 2, 4096, (OSL_TASK_FUNC)cpufreq_debug_switch_load_auto, 0, &ret); /*lint !e64 !e119 */
}

/******************************test for cpufreq end***********************************/


#ifdef __cplusplus
}
#endif


