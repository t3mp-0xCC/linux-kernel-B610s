#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/thread_info.h>
#include <linux/syslog.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <asm/string.h>
#include <asm/traps.h>
#include "product_config.h"
#include "osl_types.h"
#include <drv_nv_def.h>
#include <drv_nv_id.h>
#include "bsp_ap_dump.h"
#include "bsp_memmap.h"
#include "osl_malloc.h"
#include "drv_nv_def.h"
#include "drv_comm.h"
#include "mdrv_om_common.h"


static char* g_ap_dump_st_buffer;
static int   g_ap_dump_st_len;

enum exchS
{
    RESET,
    UNDEF,
    SWI,
    PABT,
    DABT,
    DIV,
    LOOP
};

typedef void (*FUNC_VOID)(void);

int exchRESET(void* data)
{
    FUNC_VOID a = 0;/*lint !e611 !e740*/

    
    /* coverity[var_deref_op]*/
    a();

    return 0;
}

int exchUNDEF(void* data)
{
    int b = 0;
    FUNC_VOID a = (FUNC_VOID)(&b);/*lint !e611 !e740*/

    b = 0xffffffff;
    a();

    return 0;
}

int exchSWI(void* data)
{
    __asm("        SWI   0x1234   ");

    return 0;
}

int exchPABT(void* data)
{
    FUNC_VOID a = (FUNC_VOID)0xF0000000;

    a();

    return 0;
}

int exchDABT(void* data)
{
    int a;
    
    /* coverity[zero_deref] */
    a = *(int *)0x0; /* [false alarm]:fortify */       
    /* coverity[zero_deref] */
    *(int *)0x0 = 0x12345678;/* [false alarm]:fortify */

    return 0;
}

int exchLOOP(void* data)
{
    for (;;)
    {
        ;
    }
}

int exchDIV(void* a)
{
    int b = 4;
    if(!a || !((uintptr_t)a))
    {
        return -1;  
    }

    return b / (uintptr_t)a;

}

void exchTest(int type)
{
    switch (type)
    {
    case RESET:
        kthread_run(exchRESET, 0, "exchRESET");
        break;
    case UNDEF:
        kthread_run(exchUNDEF, 0, "exchUNDEF");
        break;
    case SWI:
        kthread_run(exchSWI, 0, "exchSWI");
        break;
    case PABT:
        kthread_run(exchPABT, 0, "exchPABT");
        break;
    case DABT:
        kthread_run(exchDABT, 0, "exchDABT");
        break;
    case DIV:
        kthread_run(exchDIV, 0, "exchDIV");
        break;
    case LOOP:
        {
            u32 pid;
            struct sched_param  param = {0,};
            pid = (u32)(uintptr_t)kthread_run(exchLOOP, 0, "exchLOOP");
        	if (IS_ERR((void*)(uintptr_t)pid))
        	{
        		printk("dump_init_phase2: create kthread task dump_save failed!\n");
        		return;
        	}
            
            /* coverity[unchecked_value] */
            sched_setscheduler((struct task_struct*)(uintptr_t)pid, SCHED_FIFO, &param);
        }
        break;
    default:
        break;
    }
}

/* 模式控制 EXC DUMP模式 */
int ap_dump_st_001(void)
{
    ap_system_error(0x72, 1, 2, 0, 0);
    
    return 0;
}


/* 模式控制 USB DUMP模式 */
int ap_dump_st_002(void)
{
	
    ap_system_error(0x72, 1, 2, 0, 0);
    return 0;
}

/* RESET错误 */
int ap_dump_st_003(void)
{
    exchRESET(NULL);
    return 0;
}


/* UNDEF错误 */
int ap_dump_st_004(void)
{
    exchUNDEF(NULL);
    return 0;
}


/* SWI错误 */
int ap_dump_st_005(void)
{
    exchTest(SWI);
    return 0;
}


/* Prefetch abort错误 */
int ap_dump_st_006(void)
{
    exchPABT(NULL);
    return 0;
}

/* Data abort错误 */
int ap_dump_st_007(void)
{
    exchDABT(NULL);
    return 0;
}

/* 除0错误 */
int ap_dump_st_008(void)
{
    exchDIV(0);
    return 0;
}


/* A核用户数据 */
int ap_dump_st_009(void)
{
    char *p;
    p = kmalloc(4096, GFP_KERNEL);

    memset(p,1,4096);
	
    ap_system_error(0x88,1,2,p,4096);

    return 0;
}

void ap_dump_st_hook(void)
{
    printk("ap_dump_st_hook enter \n");
}

/* A核扩展数据 */
int ap_dump_st_010(void)
{
    int ret;
	

    g_ap_dump_st_buffer = bsp_ap_dump_register_field(DUMP_KERNEL_CPUVIEW, "LLT", 0, 0, 1024, 0);
    if(BSP_NULL == g_ap_dump_st_buffer)
    {
        printk("get buffer failed.  test case 28 failed! \n");
        return BSP_ERROR;
    }
    g_ap_dump_st_len = 1024;
    
    printk("buffer=0x%p, len=0x%x\n", g_ap_dump_st_buffer, (s32)g_ap_dump_st_len);

    ret = bsp_ap_dump_register_hook("llt", (dump_hook)ap_dump_st_hook);
    if(BSP_ERROR == ret)
    {
        printk("register_hook failed.  test case 28 failed! \n");
        return BSP_ERROR;
    }
    ap_system_error(0,0,0,0,0);
    return 0;
}



void ap_dump_st_011(void)
{
    dump_handle handle;

    handle = bsp_ap_dump_register_hook("hook_test", ap_dump_st_hook);
    if(BSP_ERROR == handle)
    {
        printk("register_hook failed. test case 30 failed! \n");
        return;        
    }

    if(BSP_OK != bsp_ap_dump_unregister_hook(handle))
    {
        printk("unregister_hook failed. test case 30 failed! \n");
        return;       
    }
}