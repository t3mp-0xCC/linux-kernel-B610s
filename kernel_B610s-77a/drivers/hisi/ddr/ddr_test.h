
#ifndef __DDR_TEST__
#define __DDR_TEST__

enum ddr_mmu_type{
    ddr_test_cacheable      = 0,
    ddr_test_nocacheable    = 1,
};

enum ddr_access_type{
    ddr_test_read   = 0,
    ddr_test_write  = 1,
};


#ifndef OK
#define OK 0
#endif
#ifndef ERROR
#define ERROR (-1)
#endif

#define DEFAULT_SIZE  0x100


enum ddr_test_type{
    ddr_test_type_reserved = 0,
    ddr_test_read_cache,
    ddr_test_read_nocache,
    ddr_test_write_cache,
    ddr_test_write_nocache,
    ddr_test_dma_copy

};


#endif

