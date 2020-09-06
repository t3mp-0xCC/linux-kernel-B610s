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
#include <osl_malloc.h>
#include <bsp_icc.h>

#include <bsp_nvim.h>
#include <drv_nv_id.h>
#include <drv_nv_def.h>
#include <cpufreq_balong.h>

#ifdef __cplusplus
extern "C"
{
#endif
int cpufreq_dfs_get_profile(void);
int g_cpufreq_max_profile = 5;

u32 g_cur_freq = 0;
u32 g_ulACpuload = 0;
s32 g_cur_profile = 0;
/*cpufreq 锁定标志 1:未锁，0: 锁定(锁定后不会主动调频)*/
int g_cpufreq_lock_status_flag = 0;
/*for debug*/
struct cpufreq_msg debug_msg = {0,0,0,0};

/*频率表*/
struct cpufreq_frequency_table *balong_clockrate_table = NULL;
struct cpufreq_query *balong_query_profile_table = NULL;
EXPORT_SYMBOL_GPL(balong_query_profile_table);
EXPORT_SYMBOL_GPL(g_cpufreq_lock_status_flag);
EXPORT_SYMBOL_GPL(g_cpufreq_max_profile);
struct cpu_dbs_info_netif_s
{
	u64 prev_cpu_idle;
	u64 prev_cpu_wall;
};
struct cpu_dbs_info_netif_s g_netif_dbs_info = {0};
static inline void cpufreq_pm_om_log(struct cpufreq_msg *msg)
{
	struct cpufreq_debug_msg debug_msg_log = {msg->msg_type, msg->source, msg->content, msg->profile, current->pid,0,0};
	debug_msg_log.cur_profile = cpufreq_dfs_get_profile();
	debug_msg_log.cur_load = g_ulACpuload;
	bsp_pm_log_type(PM_OM_CPUF, CPUFREQ_ICC_LOG, sizeof(struct cpufreq_debug_msg), &debug_msg_log);
}

unsigned int cpufreq_calccpu_load_netif(void)
{
    struct cpu_dbs_info_netif_s *dbs_info;
    u64 cur_wall_time = 0;
	u64 cur_idle_time = 0;
	u32 idle_time = 0, wall_time = 0;
	unsigned int load = 0;

    dbs_info = &g_netif_dbs_info;
    cur_idle_time = get_cpu_idle_time(0, &cur_wall_time, 0);

    idle_time = (u32)(cur_idle_time - dbs_info->prev_cpu_idle);
	wall_time = (u32)(cur_wall_time - dbs_info->prev_cpu_wall);

	dbs_info->prev_cpu_idle = cur_idle_time;
	dbs_info->prev_cpu_wall = cur_wall_time;

	load = (wall_time == 0) ?
	    0 : (unsigned int)(100 * (wall_time - idle_time) / wall_time);
	return load;

}

static int cpufreq_find_min_profile(unsigned int a9freq, unsigned int ddrfreq, unsigned int slowfreq)
{
	unsigned int target_profile = DC_RESV;
	int i = 0;
	
	for (i = BALONG_FREQ_MIN; i < DC_RESV; i++)
	{
		if ((balong_query_profile_table[i].cpu_frequency >= a9freq) && (balong_query_profile_table[i].ddr_frequency >= ddrfreq)
					&& (balong_query_profile_table[i].sbus_frequency >= slowfreq))
		{
			if (target_profile >= balong_query_profile_table[i].profile)
			{
				target_profile = balong_query_profile_table[i].profile;
			}
		}
	}
	if (DC_RESV == target_profile)
	{
		target_profile = BALONG_FREQ_MAX;
	}
	return (int)target_profile;
}

int cpufreq_icc_send(struct cpufreq_msg *msg);
/********************************************************************/
/*
 * 获取当前profile
 */
int cpufreq_dfs_get_profile(void)
{
	g_cur_profile = (s32)CPUFREQ_CUR_PROFILE;
	if ((g_cur_profile < BALONG_FREQ_MIN) || (g_cur_profile > BALONG_FREQ_MAX))
	{
		g_cur_profile = BALONG_FREQ_MAX;
		cpufreq_info("m3 cpufreq return right cur_profile value? %d\n", g_cur_profile);
	}
	return g_cur_profile;
}
EXPORT_SYMBOL_GPL(cpufreq_dfs_get_profile);
/*
 * 设置profile
 * success: return BSP_OK
 * fail:    return BSP_ERROR
 */
int cpufreq_dfs_set_profile(int profile)
{
	struct cpufreq_msg set_msg = {0,0,0,0};
	if ((profile < BALONG_FREQ_MIN) || (profile > BALONG_FREQ_MAX))
	{
		cpufreq_err("profile in right bound??%d\n", profile);
		return BSP_ERROR;
	}
	set_msg.msg_type = CPUFREQ_ADJUST_FREQ;
	set_msg.source = CPUFREQ_ACORE;
	if (cpufreq_dfs_get_profile() < profile)
	{
		set_msg.content = DFS_PROFILE_UP_TARGET;
	}
	else if (cpufreq_dfs_get_profile() > profile)
	{
		set_msg.content = DFS_PROFILE_DOWN_TARGET;
	}
	else
	{
		return BSP_OK;
	}
	set_msg.profile = (u32)profile;
	return cpufreq_icc_send(&set_msg);
}
EXPORT_SYMBOL_GPL(cpufreq_dfs_set_profile);
/*
 * 设置profile下限
 * success: return BSP_OK
 * fail:    return BSP_ERROR
 */
int cpufreq_dfs_set_baseprofile(int baseprofile)
{
	struct cpufreq_msg set_msg = {0,0,0,0};
	if ((baseprofile < BALONG_FREQ_MIN) || (baseprofile > BALONG_FREQ_MAX))
	{
		cpufreq_err("profile in right bound??%d\n", baseprofile);
		return BSP_ERROR;
	}
	set_msg.msg_type = CPUFREQ_ADJUST_FREQ;
	set_msg.source = CPUFREQ_ACORE;
	set_msg.content = DFS_PROFILE_DOWN_LIMIT;
	set_msg.profile = (u32)baseprofile;
	return cpufreq_icc_send(&set_msg);
}
EXPORT_SYMBOL_GPL(cpufreq_dfs_set_baseprofile);
/*
 * 锁定调频 DFS_PROFILE_LOCKFREQ=0锁定;DFS_PROFILE_LOCKFREQ=1解锁
 */
void cpufreq_dfs_lock(u32 lock)
{
	struct cpufreq_msg set_msg = {0,0,0,0};
	cpufreq_debug("cpufreq lock status is: %d\n", g_cpufreq_lock_status_flag);
	g_cpufreq_lock_status_flag = (s32)lock;

	set_msg.content = lock;
	
	set_msg.msg_type = CPUFREQ_LOCK_MCORE_ACTION;
	set_msg.source = CPUFREQ_ACORE;
	cpufreq_icc_send(&set_msg);
}
/*
 * 调试接口，设置频率 BSP_ERROR 设置失败;BSP_OK 设置成功
 *
 */
int cpufreq_dfs_target(int a9freq, int ddrfreq, int slowfreq)
{
	int target_profile = 0;
	target_profile = cpufreq_find_min_profile((u32)a9freq, (u32)ddrfreq, (u32)slowfreq);
	cpufreq_debug("prolfie : %d\n", target_profile);
	cpufreq_dfs_set_baseprofile(target_profile);
	return target_profile;
}
EXPORT_SYMBOL_GPL(cpufreq_dfs_target);
/*
 * 获取当前频率 BSP_ERROR 设置失败;BSP_OK 设置成功
 * 
 */
int cpufreq_dfs_current(int *a9freq, int *ddrfreq, int *slowfreq)
{
	int cur_profile = 0;
	int ret = BSP_OK;
	if ((a9freq != NULL) && (ddrfreq != NULL) && (slowfreq != NULL))
	{
		cur_profile = cpufreq_dfs_get_profile();
		*a9freq = (s32)balong_query_profile_table[cur_profile].cpu_frequency;
		*ddrfreq = (s32)balong_query_profile_table[cur_profile].ddr_frequency;
		*slowfreq = (s32)balong_query_profile_table[cur_profile].sbus_frequency;
	}
	else
	{
		cpufreq_err("argv is NULL,check it\n");
		ret = BSP_ERROR;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(cpufreq_dfs_current);

static void cpufreq_check_profile_limit(int *max_limit, int *min_limit)
{
	*max_limit = BALONG_FREQ_MAX;
	*min_limit = BALONG_FREQ_MIN;
	if ((CPUFREQ_MAX_PROFILE_LIMIT >= CPUFREQ_MIN_PROFILE_LIMIT)
	&& ((s32)CPUFREQ_MAX_PROFILE_LIMIT <= BALONG_FREQ_MAX)
	&& ((s32)CPUFREQ_MIN_PROFILE_LIMIT >= BALONG_FREQ_MIN))
	{
		*max_limit = (s32)CPUFREQ_MAX_PROFILE_LIMIT;
    	*min_limit = (s32)CPUFREQ_MIN_PROFILE_LIMIT;
	}
}

/*检查调频请求是否符合限制*/
static int cpufreq_check_msg(struct cpufreq_msg *msg)
{
	int ret = BSP_OK;
	int max_limit = 0;
	int min_limit = 0;
	int cur_profile = 0;
	cpufreq_check_profile_limit(&max_limit, &min_limit);
	cur_profile = cpufreq_dfs_get_profile();
	
	switch(msg->content)
	{
		case DFS_PROFILE_UP:
		case DFS_PROFILE_UP_TARGET:
			if (max_limit == cur_profile)
			{
				ret = BSP_ERROR;
			}
			if (msg->profile > (u32)max_limit)
			{
				msg->profile = (u32)max_limit;
			}
			break;
		case DFS_PROFILE_DOWN:
		case DFS_PROFILE_DOWN_TARGET:
			if (min_limit == cur_profile)
			{
				ret = BSP_ERROR;
			}
			if (msg->profile < (u32)min_limit)
			{
				msg->profile = (u32)min_limit;
			}
			/*判断本次请求是否和上次重复，重复则停止再次发送*/
			if ((1 == CPUFREQ_DOWN_FLAG(msg->source)) 
					&& (msg->profile == CPUFREQ_DOWN_PROFILE(msg->source)))
			{
				ret = BSP_ERROR;
			}
			break;
		case DFS_PROFILE_DOWN_LIMIT:
			if ((u32)min_limit == msg->profile)
			{
				ret =BSP_ERROR;
			}
			break;
		case DFS_PROFILE_UP_LIMIT:
			if ((u32)max_limit == msg->profile)
			{
				ret =BSP_ERROR;
			}
			break;
		default:
			break;
	}
	return ret;
}
int cpufreq_icc_send(struct cpufreq_msg *msg)
{
	u32 channel_id = ICC_CHN_MCORE_ACORE << 16 | MCU_ACORE_CPUFREQ;
	s32 ret = 0;
	u32 msg_len = sizeof(struct cpufreq_msg);
	if (!g_cpufreq_lock_status_flag)
	{
		return BSP_ERROR;
	}
	ret = cpufreq_check_msg(msg);
	if (BSP_ERROR == ret)
	{
		cpufreq_debug("soucre:%d, content:%d, profile:%d\n", msg->source, msg->content, msg->profile);
		return BSP_OK;
	}
	if (msg->msg_type != CPUFREQ_GET_FREQ_FROM_M)
	{
		debug_msg.msg_type = msg->msg_type;
		debug_msg.source = msg->source;
		debug_msg.content = msg->content;
		debug_msg.profile = msg->profile;
	}
	cpufreq_pm_om_log(msg);
	
	ret = bsp_icc_send(ICC_CPU_MCU, channel_id, (u8 *)msg, msg_len);

	if(ret != (s32)msg_len)
	{
		cpufreq_err("mcore return an ERROR please check m3 %d\n", ret);
		return BSP_ERROR;
	}
    return BSP_OK;
}

static s32 balong_cpu_freq_notifier(struct notifier_block *nb,
					unsigned long val, void *data)
{
	if (val == CPUFREQ_POSTCHANGE)
	{
		;//current_cpu_data.udelay_val = loops_per_jiffy;
	}
	else
	{
		;//for pclint
	}
	
	return 0;
}

static struct notifier_block balong_cpufreq_notifier_block = {
	.notifier_call = balong_cpu_freq_notifier,
};


/*
 *获取当前的频率
 *
 */
u32 balong_cpufreq_get(u32 cpu)
{
	int cur_profile = cpufreq_dfs_get_profile();
	return (u32)balong_clockrate_table[cur_profile].frequency;
}
u32 balong_cpufreq_getavg(struct cpufreq_policy *policy, u32 cpu)
{
	int cur_profile = cpufreq_dfs_get_profile();
	policy->cur = balong_clockrate_table[cur_profile].frequency;
	return 0;
}

/*
 * Here we notify other drivers of the proposed change and the final change.
 *
 *
 *根据relation做相应动作
 */
static s32 balong_cpufreq_target(struct cpufreq_policy *policy,
				     u32 target_freq,
				     u32 relation)
{
	u32 result = 2;
	u32 new_index = 0;
	int cur_profile = 0;
	struct cpufreq_msg task_msg = {0,0,0,0};
	
	(void)cpufreq_frequency_table_target(policy, balong_clockrate_table,
					   target_freq, relation, &new_index);
					   
	cpufreq_debug("target_freq %d new_index%d\n", target_freq, new_index);

	cur_profile = cpufreq_dfs_get_profile();
	
	if (new_index > cur_profile)
	{
		result = DFS_PROFILE_UP_TARGET;
	}
	else
	{
		result = DFS_PROFILE_DOWN_TARGET;
	}
	
	task_msg.msg_type = CPUFREQ_ADJUST_FREQ;
	task_msg.source = CPUFREQ_ACORE;
	task_msg.content = result;
	task_msg.profile = new_index;
	cpufreq_icc_send(&task_msg);
	
	policy->cur = balong_clockrate_table[cur_profile].frequency;
	g_cur_freq = policy->cur;
	return BSP_OK;
}


struct device_node *cpufreq_node = NULL;
static int cpufreq_table_init(void)
{
	int i = 0;
	unsigned int profile_count = 0;
	unsigned int *cpufreq_cpu_freq = NULL;
	unsigned int *cpufreq_ddr_freq = NULL;
	cpufreq_node = of_find_compatible_node(NULL, NULL, "cpufreq_balong_acore");
	if (!cpufreq_node)
	{
		cpufreq_err("cpufreq dts node not found!  %s\n");
		return ERROR;
	}
	of_property_read_u32_array(cpufreq_node, "cpufreq_freq_count", &profile_count, 1);
	g_cpufreq_max_profile = profile_count;
	balong_query_profile_table = (struct cpufreq_query *) osl_malloc(sizeof(struct cpufreq_query) * (g_cpufreq_max_profile + 1));
	if (NULL == balong_query_profile_table)
	{
		cpufreq_err("malloc profile table failed \n");
		return ERROR;
	}
	cpufreq_cpu_freq = (unsigned int *)osl_malloc(sizeof(unsigned int) * g_cpufreq_max_profile);
	cpufreq_ddr_freq = (unsigned int *)osl_malloc(sizeof(unsigned int) * g_cpufreq_max_profile);
	if ((NULL == cpufreq_cpu_freq) || (NULL == cpufreq_ddr_freq))
	{
		cpufreq_err("malloc freq failed\n");
		osl_free(balong_query_profile_table);
		return ERROR;
	}
	of_property_read_u32_array(cpufreq_node, "cpufreq_cpu_frequence", cpufreq_cpu_freq, g_cpufreq_max_profile);
	of_property_read_u32_array(cpufreq_node, "cpufreq_ddr_frequence", cpufreq_ddr_freq, g_cpufreq_max_profile);
	balong_clockrate_table = (struct cpufreq_frequency_table *) osl_malloc(sizeof(struct cpufreq_frequency_table) * (g_cpufreq_max_profile + 1));
	if (NULL == balong_clockrate_table)
	{
		cpufreq_err("malloc balong_clockrate_table table failed \n");
		osl_free(balong_query_profile_table);
		osl_free(cpufreq_cpu_freq);
		osl_free(cpufreq_ddr_freq);
		return ERROR;
	}
	for (i = BALONG_FREQ_MIN; i < DC_RESV; i++)
	{
		balong_query_profile_table[i].profile = i;
		balong_query_profile_table[i].cpu_frequency = cpufreq_cpu_freq[i];
		balong_query_profile_table[i].ddr_frequency = cpufreq_ddr_freq[i];
		balong_clockrate_table[i].index = balong_query_profile_table[i].profile;
		balong_clockrate_table[i].frequency = balong_query_profile_table[i].cpu_frequency;
	}
	balong_clockrate_table[DC_RESV].index = DC_RESV;
	balong_clockrate_table[DC_RESV].frequency = CPUFREQ_TABLE_END;
	osl_free(cpufreq_cpu_freq);
	osl_free(cpufreq_ddr_freq);
	return OK;
}
static s32 balong_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
	/*cpu_online 这里的作用是 ? */
	if (!cpu_online(policy->cpu))
		return -ENODEV;

	cpufreq_err("cpufreq: balong_cpufreq_cpu_init.\n");
	
	cpufreq_table_init();
	//policy->governor = &cpufreq_balong_ondemand;
	policy->max = policy->cpuinfo.max_freq = balong_query_profile_table[BALONG_FREQ_MAX].cpu_frequency;
	policy->min = policy->cpuinfo.min_freq = balong_query_profile_table[0].cpu_frequency;
	policy->cur = balong_query_profile_table[BALONG_FREQ_MAX].cpu_frequency;
	g_cur_freq = policy->cur;
	cpufreq_frequency_table_get_attr(&balong_clockrate_table[0],
					 policy->cpu);

	return cpufreq_frequency_table_cpuinfo(policy,
					    &balong_clockrate_table[0]);
}

static s32 balong_cpufreq_verify(struct cpufreq_policy *policy)
{
	return cpufreq_frequency_table_verify(policy, &balong_clockrate_table[0]);
}

static s32 balong_cpufreq_exit(struct cpufreq_policy *policy)
{
	//clk_put(cpuclk);
	return 0;
}

static struct freq_attr *balong_table_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static struct cpufreq_driver balong_cpufreq_driver = {
	.owner = THIS_MODULE,
	.name = "balong_cpufreq",
	.init = balong_cpufreq_cpu_init,
	.verify = balong_cpufreq_verify,
	.target = balong_cpufreq_target,
	.getavg = balong_cpufreq_getavg,
	.get = balong_cpufreq_get,
	.exit = balong_cpufreq_exit,
	.attr = balong_table_attr,
};

static struct platform_device_id platform_device_ids[] = {
	{
		.name = "balong_cpufreq",
	},
	{}
};

MODULE_DEVICE_TABLE(platform, platform_device_ids);

static struct platform_driver platform_driver = {
	.driver = {
		.name = "balong_cpufreq",
		.owner = THIS_MODULE,
	},
	.id_table = platform_device_ids,
};

s32 __init cpufreq_init(void)
{
	s32 ret;
	u32 retValue = 0;
	ST_PWC_SWITCH_STRU cpufreq_control_nv = {0} ;
	retValue = bsp_nvm_read(NV_ID_DRV_NV_PWC_SWITCH,(u8*)&cpufreq_control_nv,sizeof(ST_PWC_SWITCH_STRU));
	if (NV_OK == retValue)
	{
		g_cpufreq_lock_status_flag = cpufreq_control_nv.dfs;
	}
	else
	{
		g_cpufreq_lock_status_flag = 0;
		cpufreq_err("read nv failed %d\n", retValue);
	}


	/* Register platform stuff ?????*/
	ret = platform_driver_register(&platform_driver);
	if (ret)
		return ret;

	pr_info("cpufreq: balongv7r2 CPU frequency driver.\n");

	cpufreq_register_notifier(&balong_cpufreq_notifier_block,
				  CPUFREQ_TRANSITION_NOTIFIER);

	ret = cpufreq_register_driver(&balong_cpufreq_driver);
	cpufreq_err("cpufreq init ok\n");
	return ret;
}

static void __exit cpufreq_exit(void)
{

	cpufreq_unregister_driver(&balong_cpufreq_driver);
	cpufreq_unregister_notifier(&balong_cpufreq_notifier_block,
				    CPUFREQ_TRANSITION_NOTIFIER);

	platform_driver_unregister(&platform_driver);
}

module_init(cpufreq_init);
module_exit(cpufreq_exit);

//module_param(nowait, uint, 0644);

MODULE_AUTHOR("YQ ");
MODULE_DESCRIPTION("cpufreq driver for Balong");
MODULE_LICENSE("GPL");


/*test for send */
void cpufreq_debug_set_target(unsigned int msg_type, unsigned int source, unsigned int content, unsigned int profile)
{
	struct cpufreq_msg task_msg = {0,0,0,0};
	task_msg.msg_type = msg_type;
	task_msg.source = source;
	task_msg.content = content;
	task_msg.profile = profile;
	cpufreq_icc_send(&task_msg);
}
EXPORT_SYMBOL_GPL(cpufreq_debug_set_target);


#ifdef __cplusplus
}
#endif
