-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

obj-$(CONFIG_MTD_NAND_BALONG_NANDC)     +=ptable_common.o ptable_product.o
ifeq ($(strip $(MBB_DLOAD_SOLUTION)),FEATURE_ON)
obj-$(CONFIG_MTD_NAND_BALONG_NANDC)     += ptable_product_ce.o
endif