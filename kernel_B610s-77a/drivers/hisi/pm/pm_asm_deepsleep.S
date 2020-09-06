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

#include <asm/pgtable.h>

#include <bsp_ipc.h>
#include "pm.h"

_asm_function_called_from_c(pm_asm_boot_code_begin)
    /*LDR r0,=PM_MEM_V2P(PM_MEM_ACORE_BASE_ADDR+0x8)*/
    LDR r0,pm_mem_acore_base_addr_phy
    //LDR r0,[r0]
    ADD r0,r0,#12
    MOV pc,r0
    _asm_debug_pm_stamp_v2p(pm_stamp_pwrup_code_begin_phy)
    MCR p15,0,r0,c7,c5,0            @ invalidate Icache to PoU (this core)
    MCR p15,0,r0,c7,c5,6            @ invalidate branch predictor (this core)
    ISB

    /* disable L1 D-caches */
    MRC p15, 0, r1, c1, c0, 0       @ Read control register into r1
    BIC r1, r1, #(0x4)              @ Clear L1 D-cache enable
    MCR p15, 0, r1, c1, c0, 0       @ Write control register

    /* disable and clear I-cache */
    MRC p15 , 0, r2, c1, c0, 0      @ Read control register
    BIC r2, r2, #(0x1000)           @ Disable I-cache
    MCR p15 , 0, r2, c1, c0, 0      @ Write control register
    ISB

    /* endian */
    MRS r3, spsr
    LDR r2,=0xFFFFFDFF
    AND r3,r3,r2
    MSR spsr_c, r3

/************************************************************************
;* FUNCTION
;*                 pm_asm_restore_mmu_register
;* DESCRIPTION
;*                 Swich to Normal.
;* INPUTS
;*                 NONE
;* OUTPUTS
;*                 NONE
;*************************************************************************/
_asm_function_called_from_c(pm_asm_restore_mmu_register)

    _asm_debug_pm_stamp_v2p(pm_stamp_rstr_mmureg_begin_phy)

    mov    r0, #0
    mcr    p15, 0, r0, c7, c5, 0    @ invalidate I cache
    mcr    p15, 2, r0, c0, c0, 0
    mrc    p15, 1, r0, c0, c0, 0

    ldr    r1, =0x7fff
    and    r2, r1, r0, lsr #13

    ldr    r1, =0x3ff

    and    r3, r1, r0, lsr #3       @ NumWays - 1
    add    r2, r2, #1               @ NumSets

    and    r0, r0, #0x7
    add    r0, r0, #4               @ SetShift

    clz    r1, r3                   @ WayShift
    add    r4, r3, #1               @ NumWays
1:  sub    r2, r2, #1               @ NumSets--
    mov    r3, r4                   @ Temp = NumWays
2:  subs   r3, r3, #1               @ Temp--
    mov    r5, r3, lsl r1
    mov    r6, r2, lsl r0
    orr    r5, r5, r6               @ Reg = (Temp<<WayShift)|(NumSets<<SetShift)
    mcr    p15, 0, r5, c7, c6, 2
    bgt    2b
    cmp    r2, #0
    bgt    1b
    dsb
    isb

@@@@@@@@@@@@@@@@@@@@@
@ Restore MMU Regist
@@@@@@@@@@@@@@@@@@@@@
     /*LDR    r0, =PM_MEM_V2P(BAK_MMU_REG_ADDR)  @ Restore MMU Regist*/
    LDR  r0,pm_bak_mmu_reg_addr_phy
    //LDR  r0,[r0]
    LDR  r0,[r0]

    LDMDB    r0!, {r3 - r7}
    MCR p15, 0, r3, c12, c0, 0       @ VBAR
    MCR p15, 0, r4, c13, c0, 1       @ CONTEXTIDR
    MCR p15, 0, r5, c13, c0, 2       @ TPIDRURW
    MCR p15, 0, r6, c13, c0, 3       @ TPIDRURO
    MCR p15, 0, r7, c13, c0, 4       @ TPIDRPRW

    LDMDB    r0!, {r4 - r7}
    MCR p15, 0, r4, c3, c0, 0        @ DACR
    MCR p15, 0, r5, c7, c4, 0        @ PAR
    MCR p15, 0, r6, c10, c2, 0       @ PRRR
    MCR p15, 0, r7, c10, c2, 1       @ NMRR

    @ resume TTB register
    LDMDB    r0!, {r5 - r7}
    MCR p15, 0, r5, c2, c0, 0        @ TTBR0
    MCR p15, 0, r6, c2, c0, 1        @ TTBR1
    MCR p15, 0, r7, c2, c0, 2        @ TTBCR

   /* LDR r0,=PM_MEM_V2P(STORE_CTRL_REG_BASE+0x4)*/
    LDR  r0,pm_store_ctrl_reg_base_phy
    //LDR  r0,[r0]
    ADD  r0,r0,#0x4
    @current mode registers reload
    LDMIA    r0!, {r1-r9, r11}

    @R0  = save memory address
    @R1  = control register value
    @R2  = domain access register value
    @R3  = identity first level descriptor address
    @R4  = direct mapping first level descriptor
    @R5  = identity mapping first level descriptor
    @R6  = TTB register value
    @R7  = PID register value
    @R8  = disabmmu physical address
    @R9  = boundary address

    @write domain access for getting the domain access right
    LDR    r10,=(DM_ACC_UNCTRL)
    MCR    p15, 0, r10, c3, c0, 0

    @resume TLB register
    MCR    p15, 0, r6, c2, c0, 0

    @resume PID register
    MCR    p15, 0, r7, c13, c0, 0

    @clean, flush, invalidate the cache
    MOV    r7, #0

    @invalidate Icache
    MCR    p15, 0, r7, c7, c5, 0

    @drain WB
    MCR  p15, 0, r7, c7, c10, 4

    @invalidate I & D TLBs
    MCR  p15, 0, r7, c8, c7, 0

    DSB
    ISB

    @resume control register,
    @if mmu configed enable then it will be enabled
    MCR  p15, 0, r1, c1, c0, 0

    DSB
    ISB

    LDR r0,pm_store_ctrl_reg_base
    //LDR r0,[r0]
    @current mode registers reload
    LDMIA    r0!, {sp}

    @load virt boundary address to pc
    MOV     pc, r9

boundary:

    @resume the direct mapping first level descriptor
    STR     r4, [r3]

    @resume domain access register
    MCR     p15, 0, r2, c3, c0, 0

    _asm_debug_pm_stamp(pm_stamp_rstr_mmureg_end)


/************************************************************************
;* FUNCTION
;*                 pm_asm_restore_arm_co_register
;* DESCRIPTION
;*                 Swich to Normal.
;* INPUTS
;*                 NONE
;* OUTPUTS
;*                 NONE
;*************************************************************************/
_asm_function_called_from_c(pm_asm_restore_arm_co_register)

    _asm_debug_pm_stamp(pm_stamp_rstr_coreg_begin)

    LDR    r0, pm_bak_arm_co_reg_addr      @ Restore Arm Regist
   // LDR    r0,[r0]
    LDR    r0,[r0]

    ISB
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ Resume co-cpu register
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    LDMDB    r0!, {r1-r5}
    MCR    p15, 2, r1, c0, c0, 0
    ISB
    MCR    p15, 0, r2, c1, c0, 1
    ISB
    MCR    p15, 0, r3, c1, c0, 0
    ISB
    MCR    p15, 0, r4, c1, c0, 2
    ISB
    MCR    p15, 0, r5, c2, c0, 0


    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ Resume cpu mode-register
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ resume fiq mode registers
    @ enter fiq mode, no interrupts
    MRS     r2, cpsr
    BIC     r2, r2, #(MODE_MASK)
    ORR     r2, r2, #(MODE_FIQ32 | I_BIT | F_BIT)
    MSR     cpsr_c, r2
    LDMDB    r0!, {r1, r8-r14}
    MSR     spsr_cxsf, r1

    @ resume irq mode registers
    @ enter irq mode, no interrupts
    MRS     r2, cpsr
    BIC     r2, r2, #(MODE_MASK)
    ORR     r2, r2, #(MODE_IRQ32 | I_BIT | F_BIT)
    MSR     cpsr_c, r2
    LDMDB    r0!, {r1, r13, r14}
    MSR     spsr_cxsf, r1

    @ resume undefine mode registers
    @ enter undefine mode, no interrupts
    MRS     r2, cpsr
    BIC     r2, r2, #(MODE_MASK)
    ORR     r2, r2, #(MODE_UNDEF32 | I_BIT | F_BIT)
    MSR     cpsr_c, r2
    LDMDB    r0!, {r1, r13, r14}
    MSR     spsr_cxsf, r1

    @ resume abort mode registers
    @ enter abort mode, no interrupts
    MRS     r2, cpsr
    BIC     r2, r2, #(MODE_MASK)
    ORR     r2, r2, #(MODE_ABORT32 | I_BIT | F_BIT)
    MSR     cpsr_c, r2
    LDMDB    r0!, {r1, r13, r14}
    MSR     spsr_cxsf ,r1

    @ resume system mode registers
    @ enter system mode, no interrupts
    MRS     r2, cpsr
    BIC     r2, r2, #(MODE_MASK)
    ORR     r2, r2, #(MODE_SYSTEM32 | I_BIT | F_BIT)
    MSR     cpsr_c, r2
    LDMDB    r0!, {r13, r14}

    @ resume svc mode registers
    @ enter svc mode, no interrupts
    MRS     r2, cpsr
    BIC     r2, r2, #(MODE_MASK)
    ORR     r2, r2, #(MODE_SVC32 | I_BIT | F_BIT)
    MSR     cpsr_c, r2
    LDMDB    r0!, {r1-r2}
    NOP
    MSR     cpsr_cxsf, r2
    MSR     spsr_cxsf, r1

    _asm_debug_pm_stamp(pm_stamp_rstr_coreg_end)

_asm_function_called_from_c(pm_asm_restore_arm_reg)
    LDR    r0, pm_bak_arm_reg_addr
    //LDR    r0,[r0]
    LDR    r0,[r0]
    LDMDB   r0!, {r1-r13,pc}

/**********************END **********************************************/

/************************************************************************
;* FUNCTION
;*               pm_asm_cpu_go_sleep
;* DESCRIPTION
;*               entry of sleep, called in pm.c.
;* INPUTS
;*               NONE
;* OUTPUTS
;*               NONE
;*************************************************************************/
_asm_function_called_from_c(pm_asm_cpu_go_sleep)

    @ save armReg r0-r14 for powerdown and resume

    LDR    r0, pm_bak_arm_reg_addr
    //LDR   r0,[r0]
    ADD    r0,r0,#0x04
    STMIA  r0!, {r1-r13,r14}
    LDR    r2, pm_bak_arm_reg_addr
    //LDR   r2,[r2]
    STR    r0,[r2]

    _asm_debug_pm_stamp(pm_stamp_sleep_asm_enter)

/************************************************************************
;* FUNCTION
;*               pm_asm_bak_arm_co_register
;* DESCRIPTION
;*               bak arm and co cpu register to sram0.
;* INPUTS
;*               NONE
;* OUTPUTS
;*               NONE
;*************************************************************************/
_asm_function_called_from_c(pm_asm_bak_arm_co_register)

    _asm_debug_pm_stamp(pm_stamp_bak_coreg_begin)

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ save armReg in all mode
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    /* save arm register in DSRAM0 */
    LDR    r0, pm_bak_arm_co_reg_addr
    //LDR   r0,[r0]
    ADD    r0,r0,#0x04

    MRS     r2, cpsr
    BIC     r2, r2, #(MODE_MASK)
    ORR     r2, r2, #(MODE_SVC32 | I_BIT | F_BIT)
    MSR     cpsr_c, r2
    MRS     r1, spsr
    MRS     r2, cpsr
    STMIA   r0!, {r1-r2}

    /*save system mode registers*/
    /*enter system mode, no interrupts*/
    MRS     r2, cpsr
    BIC     r2, r2, #(MODE_MASK)
    ORR     r2, r2, #(MODE_SYSTEM32 | I_BIT | F_BIT)
    MSR     cpsr_c, r2
    STMIA   r0!, {r13, r14}

    /*save abort mode registers*/
    /*enter abort mode, no interrupts*/
    MRS     r2, cpsr
    BIC     r2, r2, #(MODE_MASK)
    ORR     r2, r2, #(MODE_ABORT32 | I_BIT | F_BIT)
    MSR     cpsr_c, r2
    MRS     r1,spsr
    STMIA   r0!, {r1, r13, r14}

    /*save undefine mode registers*/
    /*enter undefine mode, no interrupts*/
    MRS     r2, cpsr
    BIC     r2, r2, #(MODE_MASK)
    ORR     r2, r2, #(MODE_UNDEF32 | I_BIT | F_BIT)
    MSR     cpsr_c, r2
    MRS     r1,spsr
    STMIA   r0!, {r1, r13, r14}

    /*save irq mode registers*/
    /*enter irq mode, no interrupts*/
    MRS     r2, cpsr
    BIC     r2, r2, #(MODE_MASK)
    ORR     r2, r2, #(MODE_IRQ32 | I_BIT | F_BIT)
    MSR     cpsr_c, r2
    MRS     r1,spsr
    STMIA   r0!, {r1, r13, r14}

    /*save fiq mode registers*/
    /*enter fiq mode, no interrupts*/
    MRS     r2, cpsr
    BIC     r2, r2, #(MODE_MASK)
    ORR     r2, r2, #(MODE_FIQ32 | I_BIT | F_BIT)
    MSR     cpsr_c, r2
    MRS     r1,spsr
    STMIA   r0!, {r1, r8-r14}

    /*back to SVC mode, no interrupts*/
    MRS     r2, cpsr
    BIC     r2, r2, #(MODE_MASK)
    ORR     r2, r2, #(MODE_SVC32 | I_BIT | F_BIT)
    MSR     cpsr_c, r2


    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ save co-cpu register
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    MRC    p15, 2, r1, c0, c0, 0
    MRC    p15, 0, r2, c1, c0, 1
    MRC    p15, 0, r3, c1, c0, 0
    MRC    p15, 0, r4, c1, c0, 2
    MRC    p15, 0, r5, c2, c0, 0
    STMIA   r0!, {r1-r5}

    /*save current addr in */
    /*LDR    r2, =BAK_ARM_CO_REG_ADDR*/
    LDR    r2, pm_bak_arm_co_reg_addr
    //LDR   r2,[r2]
    STR    r0,[r2]

    _asm_debug_pm_stamp(pm_stamp_bak_coreg_end)

/************************************************************************
;* FUNCTION
;*               pm_asm_bak_mmu_register
;* DESCRIPTION
;*               bak gic register.
;* INPUTS
;*               NONE
;* OUTPUTS
;*               NONE
;*************************************************************************/

_asm_function_called_from_c(pm_asm_bak_mmu_register)

  /*  _asm_debug_pm_stamp(STAMP_BAK_MMUREG_BEGIN)*/
	_asm_debug_pm_stamp(pm_stamp_bak_mmureg_begin)

mmuHandler:

    @read control register to r1
    MRC    p15, 0, r1, c1, c0, 0
    @R1 = control register value

    @read domain access register
    MRC    p15, 0, r2, c3, c0, 0
    @R2 = original domain access register value

    @write domain access to get the domain access right
    LDR r3,=(DM_ACC_UNCTRL)
    MCR    p15, 0, r3, c3, c0, 0

    @If MMU is not enable,jump to storecode
    TST    r1, #CTRL_MMU_BIT
    @TST    r1, #0x1
    BEQ    pm_asm_bak_aarm_register_finished

    STMFD   sp!, {r1-r2}
    @R3 = identity mapping first level descriptor virtual address
    /*LDR    r2, =STORE_REMAP_ADDR1*/
    LDR    r2, pm_store_remap_addr1
    //LDR   r2,[r2]
    LDR    r3,[r2]

    /*LDR    r2, =STORE_REMAP_ADDR3*/
    LDR    r2,pm_store_remap_addr1
    //LDR    r2,[r2]
    ADD    r2,r2,#0x8
    LDR    r5,[r2]

    /*LDR    r2, =STORE_REMAP_ADDR4*/
    LDR    r2,pm_store_remap_addr1
   // LDR    r2,[r2]
    ADD    r2,r2,#0xC
    LDR    r8,[r2]

    LDMFD    sp!, {r1-r2}

    LDR    r4, [r3]
    @R4 = direct mapping first level descriptor

    @change the descriptor value to build the identity mapping
    STR    r5, [r3]

    @save a virtual address for resume
    ADRL   r9, boundary
    @resume process virtual to physical boundary address

    MOV    r6, r1
    LDR    r7, =CTRL_MMU_CACHE
    BIC    r6, r6, r7
    @r6 = control register config value(disable cache&mmu)

    STMFD    sp!, {r1-r11}

    /* cache clear */
    LDR r1,flush
    BLX r1
    B label_end1

#if 1
flush:
    .word v7_flush_dcache_all
#endif
label_end1:
    DSB

    LDMFD    sp!, {r1-r11}

    @clean, flush, invalidate the cache
    MOV   r7, #0

    @invalidate Icache
    MCR   p15, 0, r7, c7, c5, 0

    @drain WB
    MCR  p15, 0, r7, c7, c10, 4

    @invalidate I & D TLBs
    MCR  p15, 0, r7, c8, c7, 0

    @jump to PA
    MOV    pc, r8

disableMmu:
    MOV   r7, #0
    @invalidate Icache
    MCR   p15, 0, r7, c7, c5, 0

    @drain WB
    MCR  p15, 0, r7, c7, c10, 4

    @invalidate I & D TLBs
    MCR  p15, 0, r7, c8, c7, 0

    @ write control register to disable the mmu. It is harmless because
    @ we have build the direct mapping
    MCR    p15, 0, r6, c1, c0, 0
    isb
    dsb
    NOP
    NOP
    NOP

    @current store data
    @R0  = save memory address pha
    @R1  = control register value
    @R2  = domain access register value
    @R3  = identity first level descriptor address
    @R4  = direct mapping first level descriptor
    @R5  = identity mapping first level descriptor
    @R8  = disabmmu physical address
    @R9  = boundary address
    @R11 = current mode sp register value
    @R12 = cpsr register value

    @read TTB register
    MRC    p15, 0, r6, c2, c0, 0
    @R6 = TTB register value

    @ read PID register
    MRC    p15, 0, r7, c13, c0, 0
    @R7 = PID register value

      /*LDR r0,=PM_MEM_V2P(STORE_CTRL_REG_BASE+0x4)*/
    LDR r0,pm_store_ctrl_reg_base_phy
    ADD  r0,r0,#0x4
    @save arm registers and mapping informations
    STMIA    r0!, {r1-r9, r11}

    /*LDR    r0, =PM_MEM_V2P(BAK_MMU_REG_ADDR)*/
    LDR    r0, pm_bak_mmu_reg_addr_phy
  //  LDR    r0,[r0]
    ADD    r0,r0,#0x04

    /* read TTB register*/
    MRC    p15, 0, r5, c2, c0, 0     @ TTBR0
    MRC    p15, 0, r6, c2, c0, 1     @ TTBR1
    MRC    p15, 0, r7, c2, c0, 2     @ TTBCR
    STMIA    r0!, {r5 - r7}

    MRC    p15, 0, r4, c3, c0, 0     @ DACR
    MRC    p15, 0, r5, c7, c4, 0     @ PAR
    MRC    p15, 0, r6, c10, c2, 0    @ PRRR
    MRC    p15, 0, r7, c10, c2, 1    @ NMRR
    STMIA    r0!, {r4 - r7}

    MRC    p15, 0, r3, c12, c0, 0    @ VBAR
    MRC    p15, 0, r4, c13, c0, 1    @ CONTEXTIDR
    MRC    p15, 0, r5, c13, c0, 2    @ TPIDRURW
    MRC    p15, 0, r6, c13, c0, 3    @ TPIDRURO
    MRC    p15, 0, r7, c13, c0, 4    @ TPIDRPRW
    STMIA    r0!, {r3 - r7}

    @ save current addr in
    /*LDR    r2, =PM_MEM_V2P(BAK_MMU_REG_ADDR)*/
    LDR    r2, pm_bak_mmu_reg_addr_phy
    STR    r0,[r2]

pm_asm_bak_aarm_register_finished:

    /*_asm_debug_pm_stamp_v2p(STAMP_BAK_MMUREG_END)*/
    _asm_debug_pm_stamp_v2p(pm_stamp_bak_mmureg_end_phy)
#ifndef CONFIG_TRUSTZONE
    /*_asm_debug_pm_stamp_v2p(STAMP_BEFORE_SEND_IPC)*/
    _asm_debug_pm_stamp_v2p(pm_stamp_before_send_ipc_phy)

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Send IPC to m3
@ pm_asm_gic_flag equal to pm_ctrl_info.gic_flag
@ and it is set in balong_pm_init & set_pm_debug_flag
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    /* send ipc or not, according to gic_flag */
    LDR     r0,pm_asm_gic_flag
    CMP     r0,#0
    BEQ     label_after_ipc

    /* send ipc int */
    LDR      r2,pm_a2m_ipc_addr
    /*LDR      r1,=PM_A2M_IPC_BIT*/
    LDR  r1,pm_a2m_ipc_bit
    STR      r1, [r2]
    B        label_after_ipc

label_after_ipc:

    _asm_debug_pm_stamp_v2p(pm_stamp_after_send_ipc_phy)
#else
     LDR r1,pm_mem_acore_base_addr_phy
     LDR r2,pm_stamp_secos_begin_phy
     .arch_extension sec
     LDR r0,=MAPI_CPU_SUSPEND
     smc    #0
#endif
    dsb
    isb
label_wfi:
    wfi
    B label_wfi
    NOP
    NOP
    NOP


/*should not go through this if power is down*/
    _asm_debug_pm_stamp_v2p(pm_stamp_after_wfi_nop_phy)

_asm_function_called_from_c(pm_create_idmap)

    STMFD   sp!, {r1-r8}

    @convert disableMmu VA to PA

    LDR r1, pm_mem_acore_base_addr
    //LDR r1,[r1]
    LDR r2,=pm_asm_boot_code_begin
    ADR r3,disableMmu
    SUB r3,r3,r2
    ADD r1,r1,r3
   /* LDR r6,=PM_VIRT_PHY_FOR_IDMAP*/
    LDR  r6,pm_virt_phy_for_idmap
    //LDR  r6,[r6]
    SUB  r5,r1,r6

    @R5 = disableMmu entry code physical address

    MOV      r8,  r5
    @R8 = disableMmu entry code physical address

    @creat identity mapping
    LDR      r6, =SEC_ADDR_MASK
    AND      r5, r5, r6
    LDR      r6, =PMD_SECT_ARMA9
    ORR      r5, r5, r6
    @R5 = identity mapping first level descriptor

    /*LDR    r1,=STORE_REMAP_ADDR3*/
    LDR    r1,pm_store_remap_addr1;
    //LDR    r1,[r1]
    ADD    r1,r1,#0x8
    STR    r5,[r1]

    /*LDR    r1, =STORE_REMAP_ADDR4*/
    LDR    r1,pm_store_remap_addr1;
    //LDR    r1,[r1]
    ADD    r1,r1,#0xC
    STR    r8,[r1]

    @read TTB registers
    MRC    p15, 0, r3, c2, c0, 0

    @get the first level descriptor
    LDR    r4, =TRANS_BASE_MASK
    BIC    r3, r3, r4
    MOV    r4, r8, LSR #0x14
    MOV    r4, r4, LSL #0x02
    ORR    r3, r3, r4
    @R3 = identity mapping first level descriptor physical address

    @get the first level descriptor VA
    MOV r0, r3

    LDMFD    sp!, {r1-r8}
    MOV pc, lr

		_asm_variable_( pm_stamp_addr )
		_asm_variable_( pm_stamp_addr_phy )
		_asm_variable_( pm_stamp_start_addr)
		_asm_variable_( pm_stamp_start_addr_phy)
		_asm_variable_( pm_stamp_pwrup_code_begin )
		_asm_variable_( pm_stamp_pwrup_code_begin_phy)
		_asm_variable_( pm_stamp_rstr_mmureg_begin )
		_asm_variable_( pm_stamp_rstr_mmureg_begin_phy )
		_asm_variable_( pm_stamp_rstr_mmureg_end )
		_asm_variable_( pm_stamp_rstr_coreg_begin ) 
		_asm_variable_( pm_stamp_rstr_coreg_end )
		_asm_variable_( pm_stamp_sleep_asm_enter )
		_asm_variable_( pm_stamp_bak_coreg_begin )	
		_asm_variable_( pm_stamp_bak_coreg_end )	
		_asm_variable_( pm_stamp_bak_mmureg_begin )
		_asm_variable_( pm_stamp_bak_mmureg_end_phy )
		_asm_variable_( pm_stamp_before_send_ipc_phy )
		_asm_variable_( pm_stamp_after_send_ipc_phy )
		_asm_variable_( pm_stamp_after_wfi_nop_phy )
		_asm_variable_( pm_dump_sddr_v2p_delta)
		_asm_variable_(pm_virt_phy_for_idmap )
		_asm_variable_(pm_bak_mmu_reg_addr_phy)
		_asm_variable_(pm_bak_arm_co_reg_addr)
		_asm_variable_(pm_bak_arm_reg_addr)
		_asm_variable_(pm_store_ctrl_reg_base )
		_asm_variable_(pm_store_ctrl_reg_base_phy)
		_asm_variable_(pm_store_remap_addr1)
		_asm_variable_( pm_sddr_virt_addr )
		_asm_variable_( pm_sddr_phy_addr )
		_asm_variable_( pm_mem_acore_base_addr )
		_asm_variable_( pm_mem_acore_base_addr_phy)
		
		
		_asm_variable_( pm_a2m_ipc_addr )
		_asm_variable_( pm_hi_uart1_regbase_addr_virt )
		_asm_variable_( pm_hi_uart1_regbase_addr )
		_asm_variable_( pm_a2m_ipc_bit)
		_asm_variable_( pm_stamp_secos_begin_phy)
		.globl pm_asm_gic_flag
		pm_asm_gic_flag:
			.word 0
_asm_function_called_from_c(pm_asm_sleep_end)
    B    pm_asm_sleep_end


