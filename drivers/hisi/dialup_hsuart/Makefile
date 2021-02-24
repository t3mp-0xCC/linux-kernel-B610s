-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
obj-$(CONFIG_DIALUP_UART)  	+=  dialup_hsuart.o

ifeq ($(strip $(CFG_CONFIG_M2M_HSUART_TEST)),YES) 	
obj-y += dialup_hsuart_test.o
endif

ifeq ($(strip $(CFG_CONFIG_MODULE_BUSSTRESS)),YES)
obj-y +=hsuart_balong_busstress.o
endif

