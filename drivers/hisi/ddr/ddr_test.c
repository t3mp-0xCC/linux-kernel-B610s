
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <product_config.h>
#include <osl_bio.h>
#include <osl_malloc.h>
#include <bsp_hardtimer.h>
#include <bsp_edma.h>
#include <bsp_memmap.h>
#include <bsp_om.h>
#include <bsp_ddr_test.h>

#include "ddr_test.h"


#define read_bytes  sizeof(u32)


static inline void _read_test( void* start_addr, unsigned int size, unsigned int cnt)
{
    unsigned int i;
    void *cur_addr, *end_addr;
    end_addr = start_addr + size;
    for(i=0; i<cnt; i++){
        for(cur_addr = start_addr; cur_addr<end_addr; ){
            (void)readl_relaxed(cur_addr);
            cur_addr += read_bytes;
        }
    }
}

static inline void _write_test( void* start_addr, unsigned int size, unsigned int cnt)
{
    unsigned int i;
    void *cur_addr, *end_addr;
    end_addr = start_addr + size;
    for(i=0; i<cnt; i++){
        for(cur_addr = start_addr; cur_addr<end_addr; ){
            writel_relaxed(0, cur_addr);
            cur_addr += read_bytes;
        }
    }
}

void ddr_test_cpu(enum ddr_access_type access_type, enum ddr_mmu_type mmu_type, unsigned size, unsigned cnt)
{
    void* start_addr = 0;
    u32 slice_value[2] = {0,0};
    u32 slice_delta;
    unsigned long flags;
    dma_addr_t dummy;
    switch (mmu_type){
        case ddr_test_cacheable :
            start_addr = osl_malloc(size);
            break;
            
        case ddr_test_nocacheable:
            start_addr = dma_alloc_coherent(NULL, size, &dummy, GFP_DMA|__GFP_WAIT);;
            break;
            
        default:
            ddr_test_print("mmu_type=%d error. cacheable = %d nocacheable = %d \n",mmu_type,ddr_test_cacheable, ddr_test_nocacheable);
            return;
    }

    if(!start_addr){
        ddr_test_print("alloc failed\n");
        return;
    }
    ddr_test_print("interrupt locked when test is running\n");

    local_irq_save(flags);

    slice_value[0] = bsp_get_slice_value();

    switch (access_type){
        case ddr_test_read :
            _read_test( start_addr, size, cnt);
            break;
            
        case ddr_test_write:
            _write_test( start_addr, size, cnt);
            break;
            
        default:
            ddr_test_print("access_type=%d error. read = %d write = %d \n",access_type,ddr_test_read, ddr_test_write);
            
            local_irq_restore(flags);
            return;
    }
    
    slice_value[1] = bsp_get_slice_value();

    local_irq_restore(flags);
    slice_delta = get_timer_slice_delta(slice_value[0], slice_value[1]);
    ddr_test_print("test size=%d Bytes for %d times\n", size, cnt );
    ddr_test_print("test begin:0x%x, end:0x%x, delta:0x%x = %dus\n", slice_value[0], slice_value[1]\
        , slice_delta, slice_delta*1000000/32768 );
    
    switch (mmu_type){
        case ddr_test_cacheable :
            osl_free( start_addr );
            break;
            
        case ddr_test_nocacheable:
            dma_free_coherent(NULL, size, start_addr, dummy );
            break;
            
        default:
            /* unreachable */
            return;
    }
}

u32 ddr_test_cpu_args[][2]=
{
    {ddr_test_type_reserved},
    {ddr_test_read , ddr_test_cacheable   },
    {ddr_test_read , ddr_test_nocacheable },
    {ddr_test_write, ddr_test_cacheable   },
    {ddr_test_write, ddr_test_nocacheable },
};

unsigned ddr_test_size = DEFAULT_SIZE;

void ddr_test_set(unsigned size)
{
    ddr_test_size = size;
}


void ddr_test(enum ddr_test_type test_type, unsigned cnt)
{
    if(0 == cnt){
        ddr_test_print("cnt=0 is invaliad \n");
        return;
    }
    if((test_type>ddr_test_type_reserved) && (test_type<ddr_test_dma_copy)){
        ddr_test_cpu(ddr_test_cpu_args[test_type][0], ddr_test_cpu_args[test_type][1], ddr_test_size, cnt);
    }else{
        ddr_test_print("test_type=%d not surpported \n", test_type);
    }
}

void ddr_test_help(void)
{
    ddr_test_print("Warning: Cpu may be locked for seconds, please keep wdt closed \n");
    ddr_test_print("         Dfs have effects on the result of test \n");
    ddr_test_print("ddr_test cpu read/write ddr, ddr_test_size=%d \n", ddr_test_size );
    ddr_test_print("ecall ddr_test_set(size) to change it \n\n");
    
    ddr_test_print("ddr_test 1 +times: cpu read  cacheable   ddr \n");
    ddr_test_print("ddr_test 2 +times: cpu read  nocacheable ddr \n");
    ddr_test_print("ddr_test 3 +times: cpu write cacheable   ddr \n");
    ddr_test_print("ddr_test 4 +times: cpu write nocacheable ddr \n");
    ddr_test_print(" e.g. \"ddr_test 1 40000\" \n\n");

}

#define DDR_TEST_ADDR     ((DDR_SOCP_ADDR))
#define ddr_test_trace(fmt, ...) (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_EDMA, "[edma]:<%s> <%d> "fmt, __FUNCTION__,__LINE__, ##__VA_ARGS__))

/* bit[26:14] Row
 * bit[13:11] Bank
 * bit[10:02] Col
 */

void* g_ddr_test_virt_addr = NULL;


static void * get_address(u32 bank, u32 row, u32 col)
{
    u32 value = 0;

    bank &= 0x07;
    row &= 0x1FFF;
    col &= 0x1FF;

    value = (u32)g_ddr_test_virt_addr + ((bank << 11) | (row << 14 ) | (col << 2));

    return (void*)value;
}


int ddr_test_rbc_01(void)
{
    int ret = 0;

    u32 bank, row, col;

    void * address1,*address2,*address3,*address4,*temp_addr;
    u32 value1,value2,value3,value4;

    for (row = 0; row <= 3; row++){
        for (col = 0; col <= 0x1FF; col += 4){
            for (bank = 0; bank <= 7; bank++){
                address1 = get_address(bank, row, col);
                address2 = get_address(bank, row, col+1);
                address3 = get_address(bank, row, col+2);
                address4 = get_address(bank, row, col+3);
                temp_addr = get_address(7,3,0x1FF) + 4;

                writel(0x0, address1);
                writel(0xFFFFFFFF, address2);
                writel(0x0, address3);
                writel(0xFFFFFFFF, address4);

                writel(0xFFFFFFFF, temp_addr);
                writew(0xFFFF, address1);
                writel(0x0, temp_addr);
                writew(0x0, address2);
                writel(0xFFFFFFFF, temp_addr);
                writew(0xFFFF, address3);
                writel(0x0, temp_addr);
                writew(0x0, address4);

                value1 = readl(address1);
                value2 = readl(address2);
                value3 = readl(address3);
                value4 = readl(address4);

                if ((value1 != 0x0000FFFF) || (value2 != 0xFFFF0000) ||
                    (value3 != 0x0000FFFF) || (value4 != 0xFFFF0000)){
                    ret = -1;
                    return ret;
                }

                writel(0x0, temp_addr);
                writew(0xFFFF, address1+2);
                writel(0xFFFFFFFF, temp_addr);
                writew(0x0, address2+2);
                writel(0x0, temp_addr);
                writew(0xFFFF, address3+2);
                writel(0xFFFFFFFF, temp_addr);
                writew(0x0, address4+2);

                value1 = readl(address1);
                value2 = readl(address2);
                value3 = readl(address3);
                value4 = readl(address4);

                if ((value1 != 0xFFFFFFFF) || (value2 != 0x0) ||
                    (value3 != 0xFFFFFFFF) || (value4 != 0x0)){
                    ret = -1;
                    return ret;
                }
            }
        }
    }

    return ret;
}

int ddr_test_rbc_02(void)
{
    int ret = 0;

    u32 bank, row, col;

    void *address1,*address2,*address3,*address4,*temp_addr;
    u32 value1,value2,value3,value4;

    for (row = 0; row <= 3; row++){
        for (col = 0; col <= 0x1FF; col += 4){
            for (bank = 0; bank <= 7; bank++){
                address1 = get_address(bank, row, col);
                address2 = get_address(bank, row, col+1);
                address3 = get_address(bank, row, col+2);
                address4 = get_address(bank, row, col+3);
                temp_addr = get_address(7,3,0x1FF) + 4;

                writel(0xFFFFFFFF, address1);
                writel(0x0, address2);
                writel(0xFFFFFFFF, address3);
                writel(0x0, address4);

                writel(0x0, temp_addr);
                writew(0x0, address1);
                writel(0xFFFFFFFF, temp_addr);
                writew(0xFFFF, address2);
                writel(0x0, temp_addr);
                writew(0x0, address3);
                writel(0xFFFFFFFF, temp_addr);
                writew(0xFFFF, address4);

                value1 = readl(address1);
                value2 = readl(address2);
                value3 = readl(address3);
                value4 = readl(address4);

                if ((value1 != 0xFFFF0000) || (value2 != 0x0000FFFF) ||
                    (value3 != 0xFFFF0000) || (value4 != 0x0000FFFF)){
                    ret = -1;
                    return ret;
                }

                writel(0xFFFFFFFF, temp_addr);
                writew(0x0, address1+2);
                writel(0x0, temp_addr);
                writew(0xFFFF, address2+2);
                writel(0xFFFFFFFF, temp_addr);
                writew(0x0, address3+2);
                writel(0x0, temp_addr);
                writew(0xFFFF, address4+2);

                value1 = readl(address1);
                value2 = readl(address2);
                value3 = readl(address3);
                value4 = readl(address4);

                if ((value1 != 0x0) || (value2 != 0xFFFFFFFF) ||
                    (value3 != 0x0) || (value4 != 0xFFFFFFFF)){
                    ret = -1;
                    return ret;
                }
            }
        }
    }

    return ret;
}

int ddr_test_rbc_03(void)
{
    int ret = 0;

    u32 bank, row, col, i;

    void *address1,*address2,*address3,*address4,*temp_addr;
    u32 value1,value2,value3,value4;

    for (row = 0; row <= 3; row++){
        for (col = 0; col <= 0x1FF; col += 4){
            for (bank = 0; bank <= 7; bank++){
                address1 = get_address(bank, row, col);
                address2 = get_address(bank, row, col+1);
                address3 = get_address(bank, row, col+2);
                address4 = get_address(bank, row, col+3);
                temp_addr = get_address(7,3,0x1FF) + 4;

                writel(0xFFFFFFFF, address1);
                writel(0x0, address2);
                writel(0xFFFFFFFF, address3);
                writel(0x0, address4);
                for (i=0;i<=3;i++){
                    writel(0x0, temp_addr);
                    writeb(0x0, address1+i);
                    writel(0xFFFFFFFF, temp_addr);
                    writeb(0xFF, address2+i);
                    writel(0x0, temp_addr);
                    writeb(0x0, address3+i);
                    writel(0xFFFFFFFF, temp_addr);
                    writeb(0xFF, address4+i);
                }
                value1 = readl(address1);
                value2 = readl(address2);
                value3 = readl(address3);
                value4 = readl(address4);

                if ((value1 != 0x0) || (value2 != 0xFFFFFFFF) ||
                    (value3 != 0x0) || (value4 != 0xFFFFFFFF)){
                    ret = -1;
                    return ret;
                }
            }
        }
    }
    return ret;
}

void ddr_test_rbc_test(void)
{
    int ret = 0;
    if(!g_ddr_test_virt_addr){
        g_ddr_test_virt_addr = ioremap(DDR_TEST_ADDR, 0x100000);
        if (!g_ddr_test_virt_addr){
            ret = -1;
            ddr_test_trace("fail to remap...\n");
        }
    }

    ddr_test_trace("Test Start...\n");

    ret |= ddr_test_rbc_01();
    ret |= ddr_test_rbc_02();
    ret |= ddr_test_rbc_03();

    if (!ret){
        ddr_test_trace("memcopy Test PASS!");
    }else{
        ddr_test_trace("memcopy Test ERROR!");
    }

}

static int ddr_test_rbc_init(void)
{
    if(get_ddr_test_ctrl(ddr_test_ctrl_boot)){
        ddr_test_rbc_test();
    }
    return 0;
}

subsys_initcall_sync(ddr_test_rbc_init);


