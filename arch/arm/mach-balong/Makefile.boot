#include $(BALONG_TOPDIR)/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

   zreladdr-$(CONFIG_ARCH_BALONG)	:= CFG_PRODUCT_CFG_KERNEL_ENTRY
params_phys-$(CONFIG_ARCH_BALONG)	:= $(CFG_PRODUCT_KERNEL_PARAMS_PHYS)
initrd_phys-$(CONFIG_ARCH_BALONG)	:= $(CFG_DDR_ACORE_ADDR)

