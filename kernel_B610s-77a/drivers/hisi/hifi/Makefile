-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

ifeq ($(strip $(CFG_CONFIG_HIFI)),YES)
obj-y	+= bsp_hifi.o
else
obj-n	+= bsp_hifi.o
endif
