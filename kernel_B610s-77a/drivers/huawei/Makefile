-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

ifeq ($(strip $(MBB_MLOG)),FEATURE_ON)
obj-y	+= mlogdrv/
endif
ifeq ($(strip $(MBB_DLOAD)),FEATURE_ON)
obj-y     += dload_com/
endif

obj-$(CONFIG_FLIGHT_MODE)  += huawei_flight_mode/

ifeq ($(strip $(MBB_FEATURE_GPS)), FEATURE_ON)
obj-y     += gps/
endif
ifeq ($(strip $(MBB_FEATURE_BODYSAR)),FEATURE_ON)
obj-y     += bodysar/  
endif
ifeq ($(strip $(MBB_CLIENT_CODEC)),FEATURE_ON)
obj-y	+= codecdrv/
endif
ifeq ($(strip $(CFG_BSP_CONFIG_BOARD_SOLUTION)),YES)
obj-y	+= simlockdrv/
endif

ifeq ($(strip $(MBB_AGING_TEST)),FEATURE_ON)
obj-y	+= aging_test/
endif
