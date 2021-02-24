#include "pm.h"
extern u32 pm_asm_gic_flag; 
extern void* pm_stamp_start_addr;
extern struct pm_info pm_ctrl_info;
void set_pm_debug_flag(u32 gic_flag,u32 sleep_flag)
{
    pm_ctrl_info.gic_flag= gic_flag;
    pm_asm_gic_flag = pm_ctrl_info.gic_flag;
    pm_ctrl_info.sleep_flag= sleep_flag;
}
void pm_set_trace_level(u32 level)
{
    bsp_mod_level_set(BSP_MODU_PM, level);
    pm_printk(BSP_LOG_LEVEL_ERROR,"bsp_mod_level_set(BSP_MODU_PM=%d, %d)\n",BSP_MODU_PM,level);
}

void pm_print_debug_info(void)
{
    pm_printk(BSP_LOG_LEVEL_ERROR,"============= pm_ctrl_info ============\n");
    pm_printk(BSP_LOG_LEVEL_ERROR," pm_enter_count      %d\n", pm_ctrl_info.pm_enter_count);
    pm_printk(BSP_LOG_LEVEL_ERROR," pm_enter_asm_count  %d\n", pm_ctrl_info.pm_enter_asm_count);
    pm_printk(BSP_LOG_LEVEL_ERROR," pm_enter_wfi_count  %d\n", pm_ctrl_info.pm_enter_wfi_count);
    pm_printk(BSP_LOG_LEVEL_ERROR," pm_out_count        %d\n", pm_ctrl_info.pm_out_count);
    pm_printk(BSP_LOG_LEVEL_ERROR," stamp_addr 0x%x\n", pm_ctrl_info.stamp_addr);
    pm_printk(BSP_LOG_LEVEL_ERROR," gic_flag        %d\n", pm_ctrl_info.gic_flag);
    pm_printk(BSP_LOG_LEVEL_ERROR," sleep_flag      %d\n", pm_ctrl_info.sleep_flag);
    pm_printk(BSP_LOG_LEVEL_ERROR," deepsleep_nv    %d\n", pm_ctrl_info.deepsleep_nv);
	pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_START_ADDR =0x%x \n",STAMP_START_ADDR);
    //pm_printk(BSP_LOG_LEVEL_ERROR,"  %d\n", pm_ctrl_info.);
}

void pm_debug_clear_stamp(void)
{
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_START_ADDR =0x%x \n",STAMP_START_ADDR);
/*start form pm_suspend()*/
    *(u32*)STAMP_PM_SUSPEND_START = 0;
    *(u32*)STAMP_AFTER_DPM_SUSPEND = 0;
    *(u32*)STAMP_AFTER_UART_SUSPEND = 0;

/* pm enter */
    *(u32*)STAMP_PM_ENTER_START = 0;
    *(u32*)STAMP_AFTER_DISABLE_GIC = 0;
    *(u32*)STAMP_AFTER_BAK_GIC = 0;

/*in sleep ASM power down*/
    *(u32*)STAMP_SLEEP_ASM_ENTER = 0;
    *(u32*)STAMP_BAK_COREG_BEGIN = 0;
    *(u32*)STAMP_BAK_COREG_END = 0;
    *(u32*)STAMP_BAK_MMUREG_BEGIN = 0;
    *(u32*)STAMP_BAK_MMUREG_END = 0;
    *(u32*)STAMP_BEFORE_SEND_IPC = 0;
    *(u32*)STAMP_AFTER_SEND_IPC = 0;
/* after wfi, should not be stamped */
    *(u32*)STAMP_AFTER_WFI_NOP = 0;

/*in sleep ASM power up*/
    *(u32*)STAMP_PWRUP_CODE_BEGIN = 0;
    *(u32*)STAMP_RSTR_MMUREG_BEGIN = 0;
    *(u32*)STAMP_RSTR_MMUREG_END = 0;
    *(u32*)STAMP_RSTR_COREG_BEGIN = 0;
    *(u32*)STAMP_RSTR_COREG_END = 0;

/* out from sleep ASM,in pm enter */
    *(u32*)STAMP_SLEEP_ASM_OUT = 0;
    *(u32*)STAMP_AFTER_ENABLE_GIC = 0;
    *(u32*)STAMP_AFTER_RSTR_GIC = 0;
    *(u32*)STAMP_PM_ENTER_END = 0;

/* out from pm enter, in pm suspend*/
    *(u32*)STAMP_AFTER_UART_RESUME = 0;
    *(u32*)STAMP_AFTER_DPM_RESUME = 0;
}


void pm_debug_print_stamp(void)
{
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_START_ADDR =0x%x \n",STAMP_START_ADDR);
    /*start form pm_suspend()*/
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_PM_SUSPEND_START     =0x%x \n",  *(u32*)STAMP_PM_SUSPEND_START);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_AFTER_DPM_SUSPEND    =0x%x \n",  *(u32*)STAMP_AFTER_DPM_SUSPEND);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_AFTER_UART_SUSPEND   =0x%x \n",  *(u32*)STAMP_AFTER_UART_SUSPEND);

    /* pm enter */
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_PM_ENTER_START       =0x%x \n",  *(u32*)STAMP_PM_ENTER_START);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_AFTER_DISABLE_GIC    =0x%x \n",  *(u32*)STAMP_AFTER_DISABLE_GIC);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_AFTER_BAK_GIC        =0x%x \n",  *(u32*)STAMP_AFTER_BAK_GIC);

    /*in sleep ASM power down*/
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_SLEEP_ASM_ENTER      =0x%x \n",  *(u32*)STAMP_SLEEP_ASM_ENTER);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_BAK_COREG_BEGIN      =0x%x \n",  *(u32*)STAMP_BAK_COREG_BEGIN);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_BAK_COREG_END        =0x%x \n",  *(u32*)STAMP_BAK_COREG_END);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_BAK_MMUREG_BEGIN     =0x%x \n",  *(u32*)STAMP_BAK_MMUREG_BEGIN);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_BAK_MMUREG_END       =0x%x \n",  *(u32*)STAMP_BAK_MMUREG_END);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_BEFORE_SEND_IPC      =0x%x \n",  *(u32*)STAMP_BEFORE_SEND_IPC);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_AFTER_SEND_IPC       =0x%x \n",  *(u32*)STAMP_AFTER_SEND_IPC);
    /* after wfi, should not be stamped */
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_AFTER_WFI_NOP        =0x%x \n",  *(u32*)STAMP_AFTER_WFI_NOP);
    /*in sleep ASM power up*/
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_PWRUP_CODE_BEGIN     =0x%x \n",  *(u32*)STAMP_PWRUP_CODE_BEGIN);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_RSTR_MMUREG_BEGIN    =0x%x \n",  *(u32*)STAMP_RSTR_MMUREG_BEGIN);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_RSTR_MMUREG_END      =0x%x \n",  *(u32*)STAMP_RSTR_MMUREG_END);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_RSTR_COREG_BEGIN     =0x%x \n",  *(u32*)STAMP_RSTR_COREG_BEGIN);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_RSTR_COREG_END       =0x%x \n",  *(u32*)STAMP_RSTR_COREG_END);

    /* out from sleep ASM,in pm enter */
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_SLEEP_ASM_OUT        =0x%x \n",  *(u32*)STAMP_SLEEP_ASM_OUT);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_AFTER_RSTR_GIC       =0x%x \n",  *(u32*)STAMP_AFTER_RSTR_GIC);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_AFTER_ENABLE_GIC     =0x%x \n",  *(u32*)STAMP_AFTER_ENABLE_GIC);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_PM_ENTER_END         =0x%x \n",  *(u32*)STAMP_PM_ENTER_END);

    /* out from pm enter, in pm suspend*/
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_AFTER_UART_RESUME    =0x%x \n",  *(u32*)STAMP_AFTER_UART_RESUME);
    pm_printk(BSP_LOG_LEVEL_ERROR,"STAMP_AFTER_DPM_RESUME     =0x%x \n",  *(u32*)STAMP_AFTER_DPM_RESUME);
}

