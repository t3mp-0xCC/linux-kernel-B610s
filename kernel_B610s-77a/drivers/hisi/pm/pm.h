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


#ifndef __PM_H__
#define __PM_H__
#ifndef __ASSEMBLY__
#include <osl_secos.h>
#include <bsp_sram.h>
#include <bsp_memmap.h>
#include <bsp_shared_ddr.h>
#include <bsp_hardtimer.h>

//#include <bsp_dump_drv.h>

#define PM_ERROR    (-1)
#define PM_OK       (0)
#define PM_TEST_WSRC
#define pm_printk(level, fmt, ...)      (bsp_trace(level, BSP_MODU_PM, "[pm]:<%s> <%d> "fmt, __FUNCTION__,__LINE__, ##__VA_ARGS__))
#endif
#define MAPI_CPU_SUSPEND     0x80100001

#define _asm_function_called_from_c(a) \
    .globl a ;\
    .code  32 ;\
    .balign 4 ;\
a:

	
#define	_asm_variable_(a) \
		.global a ;\
		a: \
		.long 0

#ifndef __ASSEMBLY__
extern int pm_create_idmap(void);
extern void pm_asm_boot_code_begin(void);
extern void pm_asm_cpu_go_sleep(void);
extern void pm_asm_sleep_end(void);
static inline void WFI(void)
{
	asm volatile ( "dsb;" );
    asm volatile ( "isb;");
	asm volatile ( "wfi;");
	asm volatile ( "nop;");
	asm volatile ( "nop;");
}
struct pm_info
{
    u32 pm_enter_count;
    u32 pm_enter_asm_count;
    u32 pm_enter_wfi_count;
    u32 pm_out_count;
    u32 suspend_start_time;
    u32 suspend_end_time;
    u32 resume_start_time;
    u32 resume_end_time;
    void* stamp_addr;
    u32 gic_flag;   /* 若不禁GIC 直接从wfi出来，测试整个流程。 在汇编中读取，要放到结构体第一位 */
    u32 sleep_flag; /* 可以走 suspend;0 - 不进深睡；奇数 - 重复进入; 偶数 - 只进一次 ;*/
    u32 deepsleep_nv;
};


/*********  pm start **********/
/* a9热启动地址控制。*/
#define A9_BOOT_ALIGN_64K  0
#define A9_BOOT_ALIGN_32K  1
#define A9_BOOT_ALIGN_16K  2
#define A9_BOOT_ALIGN_8K   3

/* 动态设置唤醒源 发送  icc */
typedef enum tagPmType
{
	PM_TYEP_ACORE_SLEEP = 0,
	PM_TYEP_ACORE_WAKE = 1,
	PM_TYEP_CCORE_SLEEP = 2,
	PM_TYEP_CCORE_WAKE = 3,
	PM_TYEP_ASET_WSRC = 4,
	PM_TYEP_CSET_WSRC = 5,
	PM_TYEP_AUNSET_WSRC = 6,
	PM_TYEP_CUNSET_WSRC = 7,
	PM_TYEP_BUTTOM = 8
}T_PM_TYPE;
typedef struct
{
	T_PM_TYPE      type;
	u32            addr;
}T_PM_MAIL;

#endif

/*
#define PM_MEM_V2P(addr_offset,rx)   \
	LDR  r0,=(pm_mem_acore_base_addr);\
	LDR  r0,[r0];\
	LDR  r1,=pm_sddr_virt_addr;\
	LDR  r1,[r1];\
	SUB  r0,r0,r1;\
	LDR  r1,=pm_sddr_phy_addr;\
	LDR  r1,[r1];\
	ADD  r0,r0,r1;\
	MOV  rx,r0

#define PM_MEM_V2P(addr_offset,rx)   \
		LDR  rx,=(pm_virt_phy_for_sddr_v2p_delta);\
		LDR  rx,[rx];\
		ADD  rx,rx,#(addr_offset)
*/

#define PM_STAMP_ADDR       (pm_stamp_addr)
#define PM_STAMP_ADDR_PHY   (pm_stamp_addr_phy)

#define DEBUG_PM_STAMP
#ifdef DEBUG_PM_STAMP
    #define _asm_debug_pm_stamp(macro_addr)\
        LDR    r0, =(macro_addr) ;\
        LDR    r0,[r0];\
        LDR    r1, =(pm_stamp_addr) ;\
        LDR    r1,[r1];\
        LDR    r2, [r1] ;\
        STR    r2, [r0]
    #define _asm_debug_pm_stamp_v2p(macro_addr)\
        LDR    r0,(macro_addr);\
        LDR    r1, pm_stamp_addr_phy ;\
        LDR    r2, [r1] ;\
        STR    r2, [r0]

	
#else
    #define _asm_debug_pm_stamp(macro_addr)
    #define _asm_debug_pm_stamp_v2p(macro_addr)
#endif




/* boot addr after WFI & powerdown */

#define PM_MEM_ACORE_BASE_ADDR          pm_mem_acore_base_addr
#define PM_ASM_CODE_COPY_OFFSET 0x200  /* data-section in asm*/

#define	PM_VIRT_PHY_FOR_IDMAP				(pm_sddr_virt_addr-pm_sddr_phy_addr) /*PM MEM (virt_addr-phy_addr) */

#define DSARM0_BASE_ADDR_OFFSET               0x1000 //(PM_MEM_ACORE_BASE_ADDR + 0x1000)
#define DSRAM0_DEEPSLEEP_ADDR_OFFSET          (DSARM0_BASE_ADDR_OFFSET+0x200) //(DSARM0_BASE_ADDR + 0x200)

/* in asm save regs(arm,arm-corp,mmu), when powerup restore them */
/*  save r0-r14 */
#define BAK_ARM_REG_ADDR_OFFSET        DSRAM0_DEEPSLEEP_ADDR_OFFSET//(DSRAM0_DEEPSLEEP_ADDR + 0x0)  // (14+1) = 0x38
#define BAK_ARM_REG_SIZE        (0x50)
/* store cor_reg */
#define BAK_ARM_CO_REG_ADDR_OFFSET     (BAK_ARM_REG_ADDR_OFFSET + BAK_ARM_REG_SIZE)  //(21+1) = 0x58
#define BAK_ARM_CO_REG_SIZE     (0xD0)
/* store mmu */
#define BAK_MMU_REG_ADDR_OFFSET        (BAK_ARM_CO_REG_ADDR_OFFSET + BAK_ARM_CO_REG_SIZE)  // (12+1) = 0x34
#define BAK_MMU_REG_SIZE        (0x50)

/* MMU  idmap */
#define STORE_REMAP_ADDR1           (DSRAM0_DEEPSLEEP_ADDR_OFFSET + 0x794)
#define STORE_REMAP_ADDR2           (DSRAM0_DEEPSLEEP_ADDR_OFFSET + 0x798)
#define STORE_REMAP_ADDR3           (DSRAM0_DEEPSLEEP_ADDR_OFFSET + 0x79C)
#define STORE_REMAP_ADDR4           (DSRAM0_DEEPSLEEP_ADDR_OFFSET + 0x7A0)
#define STORE_CTRL_REG_BASE          (DSRAM0_DEEPSLEEP_ADDR_OFFSET + 0x800)

/* STAMP */
#undef STAMP_START_ADDR
#define STAMP_START_ADDR            pm_stamp_start_addr/*(DSRAM0_DEEPSLEEP_ADDR+0x700)*/
/*start form pm_suspend()*/
#undef STAMP_PM_SUSPEND_START
#define STAMP_PM_SUSPEND_START      (4+STAMP_START_ADDR)
#undef STAMP_AFTER_DPM_SUSPEND
#define STAMP_AFTER_DPM_SUSPEND     (4+STAMP_PM_SUSPEND_START)  /* dpm_suspend may fail and goto resume without being stamped.*/
#undef STAMP_AFTER_UART_SUSPEND
#define STAMP_AFTER_UART_SUSPEND    (4+STAMP_AFTER_DPM_SUSPEND)
/* pm enter */
#define STAMP_PM_ENTER_START        (4+STAMP_AFTER_UART_SUSPEND)
#define STAMP_AFTER_DISABLE_GIC     (4+STAMP_PM_ENTER_START)
#define STAMP_AFTER_BAK_GIC         (4+STAMP_AFTER_DISABLE_GIC)

/*in sleep ASM power down*/
#undef STAMP_SLEEP_ASM_ENTER
#define STAMP_SLEEP_ASM_ENTER       (4+STAMP_AFTER_BAK_GIC)
#define STAMP_BAK_COREG_BEGIN       (4+STAMP_SLEEP_ASM_ENTER)
#define STAMP_BAK_COREG_END         (4+STAMP_BAK_COREG_BEGIN)
#define STAMP_BAK_MMUREG_BEGIN      (4+STAMP_BAK_COREG_END)
#define STAMP_BAK_MMUREG_END        (4+STAMP_BAK_MMUREG_BEGIN)
#ifdef CONFIG_TRUSTZONE
#define STAMP_SECOS_SUSPEND_BEGIN   (4+STAMP_BAK_MMUREG_END)
#define STAMP_SECOS_BEFORE_GIC_SUS   (4+STAMP_SECOS_SUSPEND_BEGIN) 
#define STAMP_SECOS_AFTER_GIC_SUS   (4+STAMP_SECOS_BEFORE_GIC_SUS) 
#define STAMP_SECOS_AFTER_EDMA_SUS  (4+STAMP_SECOS_AFTER_GIC_SUS)  
#define STAMP_SECOS_AFTER_L2_FLUSH  (4+STAMP_SECOS_AFTER_EDMA_SUS) 
#define STAMP_BEFORE_SEND_IPC       (4+STAMP_SECOS_AFTER_L2_FLUSH)
#define STAMP_AFTER_SEND_IPC        (4+STAMP_BEFORE_SEND_IPC)
#define STAMP_AFTER_WFI_NOP         (4+STAMP_AFTER_SEND_IPC)   /* after wfi, should not be stamped */
#define STAMP_SECOS_CODE_RES_BEGIN		(4+STAMP_AFTER_WFI_NOP)
#define STAMP_SECOS_BEFORE_DPM_RES		(4+STAMP_SECOS_CODE_RES_BEGIN)
#define STAMP_SECOS_AFTER_L1_ENABLE		(4+STAMP_SECOS_BEFORE_DPM_RES)
#define STAMP_SECOS_AFTER_SERIAL_RES		(4+STAMP_SECOS_AFTER_L1_ENABLE)
#define STAMP_SECOS_AFTER_DX_RES			(4+STAMP_SECOS_AFTER_SERIAL_RES)
#define STAMP_SECOS_AFTER_TIMER_RES		(4+STAMP_SECOS_AFTER_DX_RES) 
#define STAMP_SECOS_AFTER_EDMA_RES		(4+STAMP_SECOS_AFTER_TIMER_RES)
#define STAMP_SECOS_AFTER_GIC_RES		(4+STAMP_SECOS_AFTER_EDMA_RES) 
#define STAMP_PWRUP_CODE_BEGIN      (4+STAMP_SECOS_AFTER_GIC_RES)
#else
#define STAMP_BEFORE_SEND_IPC       (4+STAMP_BAK_MMUREG_END)
#define STAMP_AFTER_SEND_IPC        (4+STAMP_BEFORE_SEND_IPC)
#define STAMP_AFTER_WFI_NOP         (4+STAMP_AFTER_SEND_IPC)   /* after wfi, should not be stamped */
#define STAMP_PWRUP_CODE_BEGIN      (4+STAMP_AFTER_WFI_NOP)
#endif

/*in sleep ASM power up*/

#define STAMP_RSTR_MMUREG_BEGIN     (4+STAMP_PWRUP_CODE_BEGIN)
#define STAMP_RSTR_MMUREG_END       (4+STAMP_RSTR_MMUREG_BEGIN)
#define STAMP_RSTR_COREG_BEGIN      (4+STAMP_RSTR_MMUREG_END)
#define STAMP_RSTR_COREG_END        (4+STAMP_RSTR_COREG_BEGIN)
/* out from sleep ASM,in pm enter */
#define STAMP_SLEEP_ASM_OUT         (4+STAMP_RSTR_COREG_END)
#define STAMP_AFTER_RSTR_GIC        (4+STAMP_SLEEP_ASM_OUT)
#undef STAMP_AFTER_ENABLE_GIC
#define STAMP_AFTER_ENABLE_GIC      (4+STAMP_AFTER_RSTR_GIC)
#define STAMP_PM_ENTER_END          (4+STAMP_AFTER_ENABLE_GIC)
/* out from pm enter, in pm suspend*/
#define STAMP_AFTER_UART_RESUME     (4+STAMP_PM_ENTER_END)
#undef STAMP_AFTER_DPM_RESUME
#define STAMP_AFTER_DPM_RESUME      (4+STAMP_AFTER_UART_RESUME) /* after dpm_resume, pm_suspend return */

#if 0
/*GIC基址*/
#define CARM_GIC_DIST_BASE              (pm_hi_app_gic_base_addr_distributor)

#define CARM_GIC_ICDISER                (CARM_GIC_DIST_BASE + 0x100)
#define CARM_GIC_ICDICER                (CARM_GIC_DIST_BASE + 0x180)
#define CARM_GIC_ICDISPR                (CARM_GIC_DIST_BASE + 0x300)

/*GIC*/
#define CARM_GIC_ICDABR_NUM             (7)
#define CARM_GIC_ICDISPR_OFFSET(x)      (CARM_GIC_ICDISPR  + ((x)*0x4))
#endif
#define DM_ACC_UNCTRL               (0xFFFFFFFF)

/* CPSR config bit . */;
#ifndef I_BIT
#define I_BIT       (1 << 7)
#endif
#ifndef F_BIT
#define F_BIT       (1 << 6)
#endif

/* mode bits */
#ifndef MODE_MASK
#define MODE_MASK       0x1F
#endif
/* masks for getting bits from PSR */
#ifndef MASK_MODE
#define MASK_MODE       0x0000003F
#endif
/* mode bits */
#define MODE_SYSTEM32   0x1F
#define MODE_UNDEF32    0x1B
#define MODE_ABORT32    0x17
#define MODE_SVC32      0x13
#define MODE_IRQ32      0x12
#define MODE_FIQ32      0x11
#define MODE_USER32     0x10



/* cpsr control bits */
#define CTRL_MMU_BIT            (0x01)
#define CTRL_ICACHE_BIT         (1<<12)
#define CTRL_DCACHE_BIT         (1<<2)
#define CTRL_CACHE_BIT          CTRL_DCACHE_BIT | CTRL_ICACHE_BIT
#define CTRL_MMU_CACHE          CTRL_MMU_BIT | CTRL_CACHE_BIT


/*ID map---*/
/*first level section address mask*/
#define SEC_ADDR_MASK           (0xFFF00000)     /*bit 31 ~ 20 mask*/

#define PMD_SECT_ARMA9        	PMD_TYPE_SECT | PMD_SECT_BUFFERABLE \
								| PMD_SECT_CACHEABLE|PMD_SECT_AP_WRITE| PMD_SECT_AP_READ

//#define PMD_SECT_ARMA9        	(PMD_TYPE_SECT | PMD_SECT_BUFFERABLE | PMD_SECT_CACHEABLE|PMD_SECT_AP_WRITE| PMD_SECT_AP_READ)
/*TTB register transfer bit0~13 MASK*/
#define TRANS_BASE_MASK         (0x3FFF)

#endif

