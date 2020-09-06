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

/*lint --e{537,438}*/
/* Warning 537: (Warning -- Repeated include  */
/* Warning 438: (Warning -- Last value assigned  not used) */
/*lint ***--e{537,713,732,737,701,438,830}*/
#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/wakelock.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <mach/board.h>
#include <asm/outercache.h>
#include <linux/irqchip/arm-gic.h>
#include <osl_types.h>
#include <osl_bio.h>
#include <soc_interrupts_m3.h>
#include <drv_nv_def.h>
#include <drv_nv_id.h>
#include <mdrv_pm.h>
#include <bsp_nvim.h>
#include "bsp_om.h"
#include <bsp_hardtimer.h>
#include "bsp_utrace.h"
#include <bsp_icc.h>
#include <bsp_pm.h>
#include <bsp_s_memory.h>
#include <bsp_dump.h>
#include <bsp_ap_dump.h>
#include <bsp_sysctrl.h>
#include <bsp_pm_om.h>
#include "pm.h"
extern void* pm_stamp_addr  ;
extern void* pm_stamp_addr_phy  ;
extern void* pm_sddr_virt_addr  ;
extern void* pm_sddr_phy_addr  ;
extern void* pm_mem_acore_base_addr;
extern void* pm_mem_acore_base_addr_phy;
extern void* pm_stamp_start_addr;
extern void* pm_stamp_start_addr_phy;

extern void* pm_stamp_pwrup_code_begin  ;
extern void* pm_stamp_pwrup_code_begin_phy  ;
extern void* pm_stamp_rstr_mmureg_begin  ;
extern void* pm_stamp_rstr_mmureg_begin_phy  ;

extern void* pm_store_ctrl_reg_base  ;
extern void* pm_stamp_rstr_mmureg_end  ;
extern void* pm_stamp_rstr_coreg_begin  ;
extern void* pm_stamp_rstr_coreg_end  ;
extern void* pm_stamp_sleep_asm_enter  ;
extern void* pm_stamp_bak_coreg_begin  ;
extern void* pm_stamp_bak_coreg_end  ;
extern void* pm_stamp_bak_mmureg_begin  ;
extern void* pm_stamp_bak_mmureg_end_phy  ;

extern void* pm_stamp_before_send_ipc_phy  ;
extern unsigned long pm_a2m_ipc_addr  ;
extern void* pm_stamp_after_send_ipc_phy  ;

extern void* pm_stamp_after_wfi_nop_phy  ;
extern unsigned int pm_virt_phy_for_idmap  ;
extern void* pm_bak_mmu_reg_addr_phy;
extern void* pm_store_ctrl_reg_base_phy;
extern void* pm_bak_arm_co_reg_addr;
extern void* pm_bak_arm_reg_addr;
extern void* pm_store_remap_addr1;
extern void* pm_stamp_secos_begin_phy;
extern unsigned int pm_a2m_ipc_bit;
extern unsigned int pm_dump_sddr_v2p_delta;
//u32 pm_hi_app_gic_base_addr_distributor=0;
u32 gic_base_addr = 0;
ST_PWC_SWITCH_STRU g_nv_pwc_switch = {0}; // NV_ID_DRV_NV_PWC_SWITCH
DRV_NV_PM_TYPE  g_nv_pm_config  = {0};

/*lint -save -e550*/ /* 550: 函数内变量没有访问，仅仅只有赋值，自增，自减等运算.但为了程序可读性需要这种变量 */


s32 pm_check_irq_pending_status(void)
{
    return 0;
}
static int balong_pm_valid_state(suspend_state_t state)
{
	pm_printk(BSP_LOG_LEVEL_INFO,">>>>>>>enter valid state  %d<<<<<<<\n", state);
	switch (state){
	case PM_SUSPEND_ON:
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:
		return 1;
	default:
		return 0;
	}
}


/*lint -save -e34*/

struct pm_info pm_ctrl_info =
{
    .pm_enter_count = 0,
    .pm_enter_asm_count = 0,
    .pm_enter_wfi_count = 0,
    .pm_out_count = 0,
    .suspend_start_time = 0,
    .suspend_end_time = 0,
    .resume_start_time = 0,
    .resume_end_time = 0,
    .stamp_addr = 0,//DUMP_EXT_OM_DRX_ACORE_ADDR,  /* pclint Non-constant Initializer*/
    .gic_flag = 1,
    .sleep_flag = 1,
    .deepsleep_nv = 0,
};
/*lint -restore*/
static inline void pm_om_record(void){
     pm_ctrl_info.suspend_start_time = readl(STAMP_PM_ENTER_START);
     pm_ctrl_info.suspend_end_time = readl(STAMP_AFTER_SEND_IPC);
     pm_ctrl_info.resume_start_time = readl(STAMP_PWRUP_CODE_BEGIN);
     pm_ctrl_info.resume_end_time = readl(STAMP_PM_ENTER_END);
    (void)bsp_pm_log_type((u32)PM_OM_PMA,(u32)PM_LOG_NORMAL_RECORD,(u32)sizeof(struct pm_info ),(void*)&pm_ctrl_info);
}
 int balong_pm_enter(suspend_state_t state)
{
	unsigned long flags = 0;
	u32 phy_addrs = 0,virt_addrs = 0;
	local_irq_save(flags);
    print_stamp(STAMP_PM_ENTER_START);

    pm_ctrl_info.pm_enter_count++;
    if(pm_ctrl_info.sleep_flag)
    {
        if (0 == pm_check_irq_pending_status()) /*中断状态查询*/
        {
        	gic_cpu_if_down();
            print_stamp(STAMP_AFTER_DISABLE_GIC);
	     /*提前做虚实地址映射，将深睡的那段代码的虚拟地址映射成跟实际地址相同*/
	    phy_addrs = (u32)pm_create_idmap();
	    virt_addrs = (u32)__phys_to_virt(phy_addrs);/*lint !e737*//* [false alarm]:屏蔽Fortify错误*/
	    writel(virt_addrs, STORE_REMAP_ADDR1+PM_MEM_ACORE_BASE_ADDR);

            outer_flush_all();
		    outer_disable();

            print_stamp(STAMP_AFTER_BAK_GIC);

            pm_ctrl_info.pm_enter_asm_count++;

            bsp_utrace_suspend();

            pm_asm_cpu_go_sleep();
            print_stamp(STAMP_SLEEP_ASM_OUT);

            bsp_utrace_resume();

            print_stamp(STAMP_AFTER_RSTR_GIC);

            outer_resume();

            print_stamp(STAMP_AFTER_ENABLE_GIC);

            /*debug: 如果此flag为偶数，则pm流程只走一次*/
            pm_ctrl_info.sleep_flag = pm_ctrl_info.sleep_flag&1;
        }
    }
    else
    {
        pm_ctrl_info.pm_enter_wfi_count++;
        WFI();
    }
    pm_ctrl_info.pm_out_count++;
    print_stamp(STAMP_PM_ENTER_END);
    pm_om_record();
	local_irq_restore(flags);
	return 0;
}

static const struct platform_suspend_ops balong_pm_ops = {
	.enter = balong_pm_enter,
	.valid = balong_pm_valid_state,
};


extern u32 pm_asm_gic_flag; /* debug, 1:gic suspend ,0:gic not suspend,and ipc not send */
void pm_wakeup_init(void);
extern void*   bsp_get_stamp_addr(void);
extern void*   bsp_get_stamp_addr_phy(void);
extern phys_addr_t bsp_get_ipc_base_addr(void);
extern u32 bsp_dump_get_virt_base_addr(void);
extern u32 bsp_dump_get_phy_base_addr(void);
#define WAKESRC_MAX_NUM 10
struct pm_wake_src_inttrupt_info_s{
	u32 index;
	u32 interrupt;
	char *name;
}pm_wake_src_inttrupt_info[WAKESRC_MAX_NUM];

static inline int bsp_get_pm_dts_value(void){
	u32 ret = 0,pm_boot_offset=0,pm_remap_offset=0,sysctrl_enum = 0,id=0;
    void* sys_baseaddr = NULL;
    struct device_node *node = NULL,*child_node = NULL;
	int ret_value = 0,i=0;
	node = of_find_compatible_node(NULL, NULL,"hisilicon,app_a9_boot_addr");
	if(!node)
	{
		pm_printk(BSP_LOG_LEVEL_ERROR,"app_a9_boot_addr of_find_compatible_node failed.\r\n");
		return -1;
	}

	if(of_property_read_u32(node, "boot_offset_sysctrl", &sysctrl_enum)!=0){
		pm_printk(BSP_LOG_LEVEL_ERROR,"boot_offset_sysctrl not find.\n");
		return -1;
	}

	sys_baseaddr=bsp_sysctrl_addr_byindex((BSP_SYSCTRL_INDEX)sysctrl_enum);/*获取系统控制器基地址*/
	if(!sys_baseaddr){
		pm_printk(BSP_LOG_LEVEL_ERROR,"sys_baseaddr get failed\n");
		return -1;
	}
	if(of_property_read_u32(node, "boot_offset", &pm_boot_offset)==0){
			writel(pm_mem_acore_base_addr_phy,sys_baseaddr+pm_boot_offset);
	}
	else{
		pm_printk(BSP_LOG_LEVEL_ERROR,"pm_boot_offset not find.\r\n");
		return -1;
	}
	if(of_property_read_u32(node, "remap_size_offset", &pm_remap_offset)==0){
			ret = readl(sys_baseaddr+pm_remap_offset);
			writel(A9_BOOT_ALIGN_16K|ret,sys_baseaddr+pm_remap_offset);
	}
	else{
		pm_printk(BSP_LOG_LEVEL_ERROR,"pm_remap_offset not find.\r\n");
		return -1;
	}
	node = of_find_compatible_node(NULL, NULL,"hisilicon,wakesrc_int");
	if(!node)
	{
		pm_printk(BSP_LOG_LEVEL_ERROR,"wakesrc_int of_find_compatible_node failed. pm init failed\n");
		return -1;
	}
	for_each_child_of_node(node, child_node){
		ret_value = of_property_read_u32(child_node, "index", &id) ;
		pm_wake_src_inttrupt_info[i].index = id;
		ret_value |= of_property_read_u32(child_node, "interrupt", &pm_wake_src_inttrupt_info[i].interrupt) ;
		ret_value |= of_property_read_string(child_node,"wakesource_name",(const char**)&pm_wake_src_inttrupt_info[i].name);
		i++;
		if(ret_value||i>=WAKESRC_MAX_NUM){
			pm_printk(BSP_LOG_LEVEL_ERROR,"wakesrc_int child read failed,i=%d\n",i);
			return -1;
		}
	}
	return 0;
}

static inline void pm_init_var_in_asm(void){
	 pm_stamp_addr = bsp_get_stamp_addr();
     pm_stamp_addr_phy = (void*)bsp_get_stamp_addr_phy();
     pm_sddr_virt_addr =(void*)g_mem_ctrl.sddr_virt_addr;
     pm_sddr_phy_addr =(void*)g_mem_ctrl.sddr_phy_addr;
     pm_mem_acore_base_addr = (void*)SHM_BASE_ADDR+SHM_OFFSET_APPA9_PM_BOOT;
     pm_mem_acore_base_addr_phy = (void*)SHD_DDR_V2P(SHM_BASE_ADDR+SHM_OFFSET_APPA9_PM_BOOT);
     pm_stamp_start_addr =bsp_ap_dump_register_field(DUMP_KERNEL_DRX, "DRX_ACORE", NULL, NULL, 0x2000, 0);
     pm_stamp_start_addr_phy = bsp_ap_dump_get_field_phy_addr(DUMP_KERNEL_DRX);
     pm_dump_sddr_v2p_delta = pm_stamp_start_addr - pm_stamp_start_addr_phy;
     pm_stamp_pwrup_code_begin = STAMP_PWRUP_CODE_BEGIN;
     pm_stamp_pwrup_code_begin_phy = pm_stamp_pwrup_code_begin - pm_dump_sddr_v2p_delta;
     //pm_stamp_rstr_mmureg_begin = STAMP_RSTR_MMUREG_BEGIN;
      pm_stamp_rstr_mmureg_begin_phy = STAMP_RSTR_MMUREG_BEGIN - pm_dump_sddr_v2p_delta;
     pm_bak_mmu_reg_addr_phy = pm_mem_acore_base_addr_phy+BAK_MMU_REG_ADDR_OFFSET;
     pm_store_ctrl_reg_base = STORE_CTRL_REG_BASE+pm_mem_acore_base_addr;
     pm_store_ctrl_reg_base_phy = STORE_CTRL_REG_BASE+pm_mem_acore_base_addr_phy;
     pm_bak_arm_co_reg_addr = BAK_ARM_CO_REG_ADDR_OFFSET+pm_mem_acore_base_addr;
     pm_bak_arm_reg_addr = BAK_ARM_REG_ADDR_OFFSET+pm_mem_acore_base_addr;
     pm_stamp_rstr_mmureg_end = STAMP_RSTR_MMUREG_END;
     pm_stamp_rstr_coreg_begin = STAMP_RSTR_COREG_BEGIN;
     pm_stamp_rstr_coreg_end = STAMP_RSTR_COREG_END;
     pm_stamp_sleep_asm_enter = STAMP_SLEEP_ASM_ENTER;
     pm_stamp_bak_coreg_begin = STAMP_BAK_COREG_BEGIN;
     pm_stamp_bak_coreg_end = STAMP_BAK_COREG_END;
     pm_stamp_bak_mmureg_begin = STAMP_BAK_MMUREG_BEGIN;
     pm_store_remap_addr1 = STORE_REMAP_ADDR1+pm_mem_acore_base_addr;
    // pm_stamp_bak_mmureg_end = STAMP_BAK_MMUREG_END;
     pm_stamp_bak_mmureg_end_phy = STAMP_BAK_MMUREG_END -pm_dump_sddr_v2p_delta ;
     //pm_stamp_before_send_ipc = STAMP_BEFORE_SEND_IPC;
     pm_stamp_before_send_ipc_phy = STAMP_BEFORE_SEND_IPC -pm_dump_sddr_v2p_delta  ;
     pm_a2m_ipc_addr = (unsigned long)bsp_get_ipc_base_addr()+IPC_CPU_RAW_INT_M3;
     pm_stamp_after_send_ipc_phy = STAMP_AFTER_SEND_IPC -pm_dump_sddr_v2p_delta ;
     pm_stamp_after_wfi_nop_phy = STAMP_AFTER_WFI_NOP -pm_dump_sddr_v2p_delta;
     pm_virt_phy_for_idmap = (u32)PM_VIRT_PHY_FOR_IDMAP;
     pm_ctrl_info.stamp_addr = pm_stamp_start_addr;
     pm_a2m_ipc_bit =(0x1<<IPC_MCU_INT_SRC_ACPU_PD);
}
static int __init balong_pm_init(void)
{
    u32 ret=0;
    NV_ID_DRV_ENUM nv_id;
    u32 pm_code_begin = 0,pm_code_size = 0;
     pm_init_var_in_asm();
     writel(0x5555AAAA,STAMP_START_ADDR);
    nv_id = NV_ID_DRV_NV_PWC_SWITCH;
    ret = bsp_nvm_read(nv_id, (u8 *)(&g_nv_pwc_switch), sizeof(ST_PWC_SWITCH_STRU));
    if(ret!=0)
    {
        pm_printk(BSP_LOG_LEVEL_ERROR,"bsp_nvm_read FAILED id=0x%x,ret=%d \n",nv_id,ret);
        return (int)ret;
    }
    nv_id = NV_ID_DRV_PM;
    ret = bsp_nvm_read(nv_id, (u8 *)(&g_nv_pm_config), sizeof(DRV_NV_PM_TYPE));
    if(ret!=0)
    {
        pm_printk(BSP_LOG_LEVEL_ERROR,"bsp_nvm_read FAILED id=0x%x,ret=%d \n",nv_id,ret);
        return (int)ret;
    }
    pm_ctrl_info.deepsleep_nv = g_nv_pwc_switch.deepsleep;
    pm_asm_gic_flag = pm_ctrl_info.gic_flag; /* flag for pm_asm_sleep.s */
    pm_code_begin = (u32)pm_asm_boot_code_begin;
    pm_code_size = (u32)pm_asm_sleep_end-pm_code_begin;

    /*FOR creat id map */
    writel(0, STORE_REMAP_ADDR1+PM_MEM_ACORE_BASE_ADDR);
    writel(0, STORE_REMAP_ADDR2+PM_MEM_ACORE_BASE_ADDR);
    writel(0, STORE_REMAP_ADDR3+PM_MEM_ACORE_BASE_ADDR);

	/* copy the pm_asm_sleep code to PM MEMORY */
	memcpy((void*)PM_MEM_ACORE_BASE_ADDR, (void*)pm_asm_boot_code_begin, pm_code_size+PM_ASM_CODE_COPY_OFFSET);
	if(bsp_get_pm_dts_value()){
		pm_printk(BSP_LOG_LEVEL_ERROR," acore pm init failed \n");
		return -1;
	}

	writel(0x5555AAAA,STAMP_AFTER_WFI_NOP);
	/* NV control , system will not suspend if ops is not set */
    if(0==pm_ctrl_info.deepsleep_nv)
    {
        pm_printk(BSP_LOG_LEVEL_ERROR," nv closed;core pm ops not register \n");
        return -1;
    }
    suspend_set_ops(&balong_pm_ops);
	pm_printk(BSP_LOG_LEVEL_ERROR," pm init ok \n");
	return 0;
}

module_init(balong_pm_init);

/*
动态设置唤醒源
参数 wake_src 对应 drv_pm.h 中的 enum pm_wake_src
*/
#define ENABLE_WAKE_SRC     (PM_TYEP_ASET_WSRC)
#define DISABLE_WAKE_SRC    (PM_TYEP_AUNSET_WSRC)

void pm_set_wake_src(enum pm_wake_src wake_src, T_PM_TYPE pm_type)
{
    T_PM_MAIL pm_msg;
    u32 channel_id = (ICC_CHN_MCORE_ACORE << 16) | MCU_ACORE_WSRC;
	u32 ret = 0,i=0;
    u32 int_id = 0;
    u32 msglen = 0;
	if(wake_src>=PM_WAKE_SRC_BUT)
	{
		pm_printk(BSP_LOG_LEVEL_ERROR,"wake_src id error\n");
		return;
	}
	for(i=0;i<WAKESRC_MAX_NUM;i++)
	{
		if((u32)wake_src == pm_wake_src_inttrupt_info[i].index){
			int_id = pm_wake_src_inttrupt_info[i].interrupt;
			break;
		}
	}
    pm_msg.type = pm_type;
    pm_msg.addr = int_id;
	msglen = sizeof(T_PM_MAIL);

	ret = (u32)bsp_icc_send(ICC_CPU_MCU, channel_id, (u8*)(&pm_msg), msglen);
	if(ret != msglen)
	{
		pm_printk(BSP_LOG_LEVEL_ERROR,"PM icc_send_mcore error\n");
		return;
	}
    return;
}
void pm_enable_wake_src(enum pm_wake_src wake_src)
{
    pm_set_wake_src(wake_src,ENABLE_WAKE_SRC);
}

void pm_disable_wake_src(enum pm_wake_src wake_src)
{
    pm_set_wake_src(wake_src,DISABLE_WAKE_SRC);
}
/*lint -restore +e550*/



