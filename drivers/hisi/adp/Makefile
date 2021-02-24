#
# created by y000184236 for k3v3 fpga
#
-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
ifeq ($(MBB_USB),FEATURE_ON)
KBUILD_CPPFLAGS += -Idrivers/usb/mbb_usb_unitary
else
KBUILD_CPPFLAGS += -Idrivers/usb/gadget
endif
KBUILD_CPPFLAGS += -Idrivers/hisi/rtc

obj-y           += adp_ncm.o
obj-y           += adp_mbxhifi.o
#HSUART_M2M
ifeq ($(strip $(CFG_CONFIG_M2M_HSUART)),NO)
obj-y           += adp_dialup_hsuart.o
endif

