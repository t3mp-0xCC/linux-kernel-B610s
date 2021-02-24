#
# Makefile for ddr test drivers
#
include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

ifeq ($(strip $(MBB_HWTEST)),FEATURE_ON)
obj-m		+= ddrtest.o
ddrtest-objs := ddr_tester.o \
		ddr_alg.o \
		ddr_cache.o 
endif

