-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

ifeq ($(MBB_CHG_BQ24196),FEATURE_ON)
obj-y	+=	bq24196/
endif

ifeq ($(MBB_CHG_BQ27510),FEATURE_ON)
obj-y	+=	bq27510/
endif

ifeq ($(MBB_CHG_COULOMETER),FEATURE_ON)
#for coul
obj-y	+=	coul/
endif

ifeq ($(MBB_CHG_BQ25892),FEATURE_ON)
obj-y	+=	fsa9688/
obj-y   +=  bq25892/
endif
