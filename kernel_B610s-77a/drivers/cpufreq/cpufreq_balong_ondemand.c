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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/jiffies.h>
#include <linux/kernel_stat.h>
#include <linux/mutex.h>
#include <linux/hrtimer.h>
#include <linux/tick.h>
#include <linux/ktime.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/percpu.h>
#include <osl_thread.h>
#include <osl_irq.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <bsp_hardtimer.h>
#include <bsp_icc.h>
#include <bsp_nvim.h>
#include <drv_nv_id.h>
#include <drv_nv_def.h>
#include <drv_comm.h>
#include "cpufreq_balong.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*lint --e{24,34,35,43,50,63,64,110,78, 728,745,752, 958,808 } */
#define TRANSITION_LATENCY_LIMIT		(10 * 1000 * 1000)
extern int cpufreq_dfs_get_profile(void);

struct dbs_tuners dbs_tuners_ins = {0};
static DEFINE_PER_CPU(struct cpu_dbs_info_s, g_acpu_dbs_info);/*lint !e49 !e601 !e808 !e762 !e830 */
static DEFINE_MUTEX(dbs_mutex); /*lint !e651 !e120 !e156 !e121 !e133 */
static DEFINE_MUTEX(info_mutex);/*lint !e651 */
/*读取NV*/
ST_PWC_DFS_STRU g_stDfsSwitch={0};

static u32 dbs_enable = 0;	/* number of CPUs using this policy */
/*当前profile*/
extern s32 g_cur_profile;
/*当前占用率*/
extern u32 g_ulACpuload;
/*cpufreq 锁定标志 1:未锁，0: 锁定(锁定后不会主动调频)*/
extern int g_cpufreq_lock_status_flag;
/*主动调频是否发送icc消息标志 1:可发送*/
int g_icc_run_flag = 1;

extern struct cpufreq_msg debug_msg;

extern int cpufreq_icc_send(struct cpufreq_msg *msg);
/*******************************************************/

/*lint -save -e438*/
/*lint --e{550}*/
unsigned int cpufreq_calccpu_cpuload(void)
{
	u32 idle_time = 0;
	u32 wall_time = 0;
	unsigned int cpu_load = 0;
	cputime64_t cur_wall_time = 0;
	cputime64_t cur_idle_time = 0;
	struct cpu_dbs_info_s *dbs_info;
	dbs_info = &per_cpu(g_acpu_dbs_info, 0); 
	
	cur_idle_time = get_cpu_idle_time(0, &cur_wall_time, 0);
	idle_time = (u32)(cur_idle_time - dbs_info->prev_cpu_idle);
	wall_time = (u32)(cur_wall_time - dbs_info->prev_cpu_wall);

	cpu_load = (100 * (wall_time - idle_time) / wall_time);

	return cpu_load;
}
/*lint -restore +e438*/
/*
 * 该接口负责cpu负载检测，
 * 并根据预设阈值判决是否需要向M3请求调频
 */
void cpufreq_update_frequency(void)
{
	u32 cpuload = 0;
	int cur_profile = 0;
	struct cpufreq_msg task_msg = {CPUFREQ_ADJUST_FREQ, CPUFREQ_ACORE, 0, BALONG_FREQ_MAX};
	cpuload = (u32)cpufreq_calccpu_cpuload();
	cur_profile = (int)cpufreq_dfs_get_profile();
	if (cpuload > dbs_tuners_ins.up_threshold)
   {
		task_msg.content = DFS_PROFILE_UP_TARGET;
	}
    else if (cpuload < dbs_tuners_ins.down_threshold)
    {
		task_msg.profile = (cur_profile != BALONG_FREQ_MIN) ? (cur_profile - 1) : (BALONG_FREQ_MIN);
    	task_msg.content = DFS_PROFILE_DOWN;
    }
    else
    {
		return;
    }
    cpufreq_icc_send(&task_msg);
}
/*****************************************************************************
* 函 数 名  : cpufreq_set_boost
*
* 功能描述  : 给touch和按键用作升频使用
*             
*
* 输入参数  : boost_val  0：需要升频  1：不需要升频
*            duratiom_endtime 延迟一段时间后，开始升频
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
#if (FEATURE_ON == MBB_CPU_FREQ_BOOST)
/*给touch用作升频使用*/
void cpufreq_set_boost(u32 boost_val, u64 duratiom_endtime)
{
    struct cpu_dbs_info_s *dbs_info;
    mutex_lock(&info_mutex);
    dbs_info = &per_cpu(g_acpu_dbs_info, 0);
    dbs_info->cpu_boost_value = boost_val;
    dbs_info->cpu_boost_duraion_time = ktime_to_us(ktime_get()) + duratiom_endtime;
    mutex_unlock(&info_mutex);
}
#endif
/*
 *计算CPU占用率及算出将要调整的频率值
 */
unsigned int cpufreq_calccpu_result(u32 *nextfreq)
{
	u32 max_load_cpu = 0;
	u32 cur_freq = 0;
	struct cpufreq_policy *policy;
	u32 idle_time = 0, wall_time = 0;
	cputime64_t cur_wall_time = 0;
	cputime64_t cur_idle_time = 0;
	struct cpu_dbs_info_s *dbs_info;
	dbs_info = &per_cpu(g_acpu_dbs_info, 0);
	policy = dbs_info->cur_policy;

	cur_idle_time = get_cpu_idle_time(0, &cur_wall_time, 0);
#if (FEATURE_ON == MBB_CPU_FREQ_BOOST)
    bool boosted = 0;
#endif

	idle_time = (u32)(cur_idle_time - dbs_info->prev_cpu_idle);
	wall_time = (u32)(cur_wall_time - dbs_info->prev_cpu_wall);

	dbs_info->prev_cpu_idle = cur_idle_time;
	dbs_info->prev_cpu_wall = cur_wall_time;

	/*获取cpu占用率*/
	max_load_cpu = 100 * (wall_time - idle_time) / wall_time;
	g_ulACpuload = max_load_cpu;
	cpufreq_debug("cpu load:%d\n", max_load_cpu);
	cur_freq = __cpufreq_driver_getavg(policy, 0);
	/* Check for frequency increase or decrease*/
#if (FEATURE_ON == MBB_CPU_FREQ_BOOST)
    /*add for touch boost*/
    boosted = dbs_info->cpu_boost_value || cur_wall_time < dbs_info->cpu_boost_duraion_time; 
    if (max_load_cpu > dbs_tuners_ins.up_threshold  || boosted)
#else
    if (max_load_cpu > dbs_tuners_ins.up_threshold)
#endif
	{
		dbs_info->cpu_up_time++;
		if (dbs_tuners_ins.up_threshold_times == dbs_info->cpu_up_time)
		{
			dbs_info->cpu_down_time = 0;
			dbs_info->cpu_up_time = 0;
			*nextfreq = policy->max;
			return CPUFREQ_RELATION_H;
		}
		return DFS_PROFILE_NOCHANGE;
	}
 	if (max_load_cpu < dbs_tuners_ins.down_threshold)
 	{
		dbs_info->cpu_down_time++;
		if (dbs_tuners_ins.down_threshold_times == dbs_info->cpu_down_time)
		{
			dbs_info->cpu_down_time = 0;
			dbs_info->cpu_up_time = 0;
			if (0 == max_load_cpu)
			{
				max_load_cpu = 1;
			}
			
			*nextfreq = (max_load_cpu * cur_freq)/(dbs_tuners_ins.down_threshold);
			return CPUFREQ_RELATION_L;
		}
		return DFS_PROFILE_NOCHANGE;
	}
	*nextfreq = 0;
	dbs_info->cpu_down_time = 0;
	dbs_info->cpu_up_time = 0;
	return DFS_PROFILE_NOCHANGE;
}


/*
 *	变为两个函数，一个计算占用率，并返回要之后要如何操作
 *	一个根据返回完成接下来的操作
 */
void balong_dbs_check_cpu(void)
{
	u32 result = 2;
	u32 nextfreq = 0;
	struct cpu_dbs_info_s *dbs_info = &per_cpu(g_acpu_dbs_info, 0);
	struct cpufreq_policy *policy;

	policy = dbs_info->cur_policy;

	if (!g_cpufreq_lock_status_flag)
	{
		return;
	}
	result = cpufreq_calccpu_result(&nextfreq);
	cpufreq_debug("result:%d\n", result);
	if (result != DFS_PROFILE_NOCHANGE)
	{
		
		if (g_icc_run_flag)
		{
			__cpufreq_driver_target(policy, nextfreq, result);
		}
	}
	
}


/***********************************************************
*调频任务
***********************************************************/
void balong_do_dbs_timer(struct work_struct *work)
{
	struct cpu_dbs_info_s *dbs_info = &per_cpu(g_acpu_dbs_info, 0);
	s32 cpu = 0;
	unsigned long delay = 0;
	if (dbs_info == NULL){
		cpufreq_err("dbs_info error\n");
		return;
	}
	cpu = dbs_info->cpu;
	mutex_lock(&info_mutex);
	/*检查CPU占用率，调频*/
	balong_dbs_check_cpu();

	delay = usecs_to_jiffies(dbs_tuners_ins.sampling_rate);

	schedule_delayed_work_on(cpu, &dbs_info->work, delay);
	mutex_unlock(&info_mutex);
}

/***********************************************************
*调频任务初始化
***********************************************************/
static inline void dbs_timer_init(struct cpu_dbs_info_s *dbs_info)
{
	/* We want all CPUs to do sampling nearly on same jiffy */
	unsigned long delay = usecs_to_jiffies(dbs_tuners_ins.sampling_rate);
	if (NULL == dbs_info){
		cpufreq_err("!!!!!!dbs_timer_init!!!!!!error\n");
		return;
	}

	INIT_DEFERRABLE_WORK_ONSTACK(&(dbs_info->work), balong_do_dbs_timer);/*lint !e613*/
	schedule_delayed_work_on(dbs_info->cpu, &(dbs_info->work), delay);/*lint !e613*/
}

static inline void dbs_timer_exit(struct cpu_dbs_info_s *dbs_info)
{
	cancel_delayed_work_sync(&dbs_info->work);
}

static s32 cpufreq_governor_dbs(struct cpufreq_policy *policy, u32 event)
{
	s32 cpu = (s32)policy->cpu;
	struct cpu_dbs_info_s *dbs_info = NULL;
	u32 retValue = 0;
	ST_PWC_SWITCH_STRU cpufreq_control_nv = {0} ;
	/*cpu 信息*/
	dbs_info = &per_cpu(g_acpu_dbs_info, (u32)cpu);
	/*lint --e{744 } */
	switch (event) {
	case CPUFREQ_GOV_START:
		cpufreq_debug("CPUFREQ_GOV_START\n");
		mutex_lock(&dbs_mutex);

		dbs_enable++;
		
		if (1 == dbs_enable)
		{
			/*cpu 信息初始化  函数??idle_time*/
			dbs_info->prev_cpu_idle = get_cpu_idle_time(0,
							&dbs_info->prev_cpu_wall, 0);
			dbs_info->cur_policy = policy;
			dbs_info->cpu = cpu;
			dbs_info->freq_table = cpufreq_frequency_get_table((u32)cpu);
			dbs_info->cpu_down_time = 0;
			dbs_info->cpu_up_time = 0;
		
			retValue = bsp_nvm_read(NV_ID_DRV_NV_PWC_SWITCH,(u8*)&cpufreq_control_nv,sizeof(ST_PWC_SWITCH_STRU));
			if (NV_OK == retValue)
			{
				g_cpufreq_lock_status_flag = cpufreq_control_nv.dfs;
			}
			else
			{
				cpufreq_err("read nv failed %d\n", retValue);
			}

			retValue = bsp_nvm_read(NV_ID_DRV_NV_DFS_SWITCH,(u8*)&g_stDfsSwitch,sizeof(ST_PWC_DFS_STRU));
		    if (NV_OK != retValue)
		    {
		    	cpufreq_err("read nv failed use default value\n");
				g_stDfsSwitch.AcpuDownLimit = 20;
				g_stDfsSwitch.AcpuDownNum = 3;
				g_stDfsSwitch.AcpuUpLimit = 80;
				g_stDfsSwitch.AcpuUpNum = 1;
				g_stDfsSwitch.DFSTimerLen = 400;
		    }
		    
			dbs_tuners_ins.up_threshold = g_stDfsSwitch.AcpuUpLimit;
			dbs_tuners_ins.down_threshold = g_stDfsSwitch.AcpuDownLimit;
			dbs_tuners_ins.down_threshold_times = g_stDfsSwitch.AcpuDownNum;
			dbs_tuners_ins.up_threshold_times = g_stDfsSwitch.AcpuUpNum;
			dbs_tuners_ins.sampling_rate = g_stDfsSwitch.DFSTimerLen * 10000; /*unit:us*/
			/*
			 * Start the timerschedule work, when this governor
			 * is used for first time
			 */
			dbs_timer_init(dbs_info);
		}
		mutex_unlock(&dbs_mutex);
		break;

	case CPUFREQ_GOV_STOP:
		cpufreq_info("exit balong ondemand\n");
		mutex_lock(&dbs_mutex);
		dbs_enable--;
		dbs_timer_exit(dbs_info);
		mutex_unlock(&dbs_mutex);
		break;

	case CPUFREQ_GOV_LIMITS:
		if (policy->max < dbs_info->cur_policy->cur)
			__cpufreq_driver_target(dbs_info->cur_policy,
				policy->max, CPUFREQ_RELATION_H);
		else if (policy->min > dbs_info->cur_policy->cur)
			__cpufreq_driver_target(dbs_info->cur_policy,
				policy->min, CPUFREQ_RELATION_L);

		break;
	}
	return 0;
}

struct cpufreq_governor cpufreq_balong_ondemand = {
       .name                   = "balong_ondemand",
       .governor               = cpufreq_governor_dbs,
       .max_transition_latency = TRANSITION_LATENCY_LIMIT,/*遗留:功能见注释*/
       .owner                  = THIS_MODULE,
};

static s32 __init cpufreq_gov_dbs_init(void)
{
	return cpufreq_register_governor(&cpufreq_balong_ondemand);
}

static void __exit cpufreq_gov_dbs_exit(void)
{
	cpufreq_unregister_governor(&cpufreq_balong_ondemand);
}


fs_initcall(cpufreq_gov_dbs_init);
module_exit(cpufreq_gov_dbs_exit);
/******************************test for cpufreq start***********************************/

void cpufreq_print_debug(void)
{
	struct cpu_dbs_info_s *dbs_info;
	dbs_info = &per_cpu(g_acpu_dbs_info, 0);
	cpufreq_dfs_get_profile();
	cpufreq_err("cur    profile: %d\n", g_cur_profile);
	cpufreq_err("acore    load: %d\n", g_ulACpuload);
	cpufreq_err("up      times: %d\n", dbs_info->cpu_up_time);
	cpufreq_err("down  times: %d\n", dbs_info->cpu_down_time);
	cpufreq_err("up_threshold: %d\n", dbs_tuners_ins.up_threshold);
	cpufreq_err("up_threshold_times: %d\n", dbs_tuners_ins.up_threshold_times);
	cpufreq_err("down_threshold: %d\n", dbs_tuners_ins.down_threshold);
	cpufreq_err("down_threshold_times: %d\n", dbs_tuners_ins.down_threshold_times);
	cpufreq_err("sampling_rate: %d\n", dbs_tuners_ins.sampling_rate);
	cpufreq_err("last icc msg\n");
	cpufreq_err("icc msg_type: %d\n", debug_msg.msg_type);
	cpufreq_err("icc source: %d\n", debug_msg.source);
	cpufreq_err("icc content: %d\n", debug_msg.content);
	cpufreq_err("icc profile: %d\n", debug_msg.profile);
	cpufreq_err("cur max limit:%d\n", CPUFREQ_MAX_PROFILE_LIMIT);
	cpufreq_err("cur min limit:%d\n", CPUFREQ_MIN_PROFILE_LIMIT);
}

/******************************test for cpufreq end***********************************/
#ifdef __cplusplus
}
#endif

