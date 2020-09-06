#ifndef __TEE_RDR_REGISTER_H__
#define __TEE_RDR_REGISTER_H__

#define BB_PRINT_PN(args...)    printk(KERN_WARNING args);
#define BB_PRINT_ERR(args...)   printk(KERN_ERR args);
#define BB_PRINT_DBG(args...)   printk(KERN_ERR args);
#define BB_PRINT_START(args...) \
    printk(KERN_ERR ">>>>>enter %s: %.4d. \n", __func__, __LINE__);
#define BB_PRINT_END(args...)   \
    printk(KERN_ERR "<<<<<exit  %s: %.4d. \n", __func__, __LINE__);

int tee_rdr_init(void);

#endif
