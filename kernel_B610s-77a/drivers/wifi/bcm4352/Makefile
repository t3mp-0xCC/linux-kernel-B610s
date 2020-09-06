#
# Makefile for wifi_arp_cheap_protect
#

include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

INCLUDE_COMM_DIR_FLAG := \
    -I$(srctree)/net/bridge \

$(info [wi-fi]: CHIP1 = $(MBB_WIFI_CHIP1); CHIP2 = $(MBB_WIFI_CHIP2))

EXTRA_CFLAGS += -DFEATURE_HUAWEI_MBB_$(shell echo '$(MBB_WIFI_CHIP1)' | tr '[a-z]' '[A-Z]')

ifeq ($(MBB_WIFI_CHIP1),bcm4352)
    obj-y += wifi_arp_cheap_protect.o
	obj-y += wifi_spe_bcm4352.o
    $(info [wi-fi]: build wifi_arp_cheap_protect.c wifi_spe_bcm4352.c)
endif

$(info [wi-fi]: CHIP1 = $(MBB_WIFI_CHIP1); CHIP2 = $(MBB_WIFI_CHIP2))
ifeq ($(MBB_WIFI_CHIP1),bcm4360)
    obj-y += wifi_arp_cheap_protect.o
	obj-y += wifi_spe_bcm4352.o
    $(info [wi-fi]: build wifi_arp_cheap_protect.c wifi_spe_bcm4352.c)
endif

$(info [wi-fi]: CHIP1 = $(MBB_WIFI_CHIP1); CHIP2 = $(MBB_WIFI_CHIP2))
ifeq ($(MBB_WIFI_CHIP1),bcm43217)
    obj-y += wifi_arp_cheap_protect.o
	obj-y += wifi_spe_bcm4352.o
    $(info [wi-fi]: build wifi_arp_cheap_protect.c wifi_spe_bcm4352.c)
endif

$(info [wi-fi]: EXTRA_CFLAGS = $(EXTRA_CFLAGS))

EXTRA_CFLAGS += $(INCLUDE_COMM_DIR_FLAG) 
export EXTRA_CFLAGS
