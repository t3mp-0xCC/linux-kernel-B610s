#
#
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk 

ifeq ($(MBB_DLOAD),FEATURE_OFF)
EXTRA_CFLAGS += -Idrivers/tzdriver \
                -Idrivers/hisi/tzdriver \
		        -Iscripts/dtc/libfdt

obj-y		    += load_image.o
endif

ifeq ($(strip $(CONFIG_HISI_BALONG_MODEM)),y)
obj-y		    += loadm_phone.o
else
obj-y		    += loadm_mbb.o
endif

subdir-ccflags-y := -Werror
