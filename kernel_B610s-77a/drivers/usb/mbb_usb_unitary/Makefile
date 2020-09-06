#
# USB peripheral controller drivers
#
ccflags-$(CONFIG_USB_GADGET_DEBUG) := -DDEBUG
ccflags-$(CONFIG_USB_GADGET_DEBUG) += -DVERBOSE_DEBUG

obj-$(CONFIG_USB_GADGET)	+= udc-core.o


#
# USB gadget drivers
#
-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

obj-$(CONFIG_USB_GADGET)		+= adapt/
KBUILD_CPPFLAGS +=	-Idrivers/usb/mbb_usb_unitary/adapt/include\
					-Idrivers/usb/mbb_usb_unitary\
                    -Idrivers/mntn/include			

ifeq ($(CONFIG_NCM_MBIM_SUPPORT),y)
KBUILD_CPPFLAGS +=	-Idrivers/usb/mbb_usb_unitary/mbim/inc
obj-y				+= mbim/
endif
				


g_balong_udc-y		:= mbb_usb_pnp.o \
                       usb_vendor.o\
		       usb_dfx.o\
                       usb_debug.o \
                       usb_notify.o \
                       ncm_balong.o \
                       ecm_balong.o \
                       usb_nv_get.o \
                       usb_platform_comm.o \
                       f_acm.o \
                       u_cdev.o \
                       u_modem.o \
                       u_serial.o \
                       u_ether.o \
                       f_ncm.o \
                       f_rndis.o \
                       rndis.o \
                       f_ecm.o \
                       mbb_usb_adp.o \
                       u_usb_ctrl_sd.o \
                       hisi_adp_usb.o \
                       hisi_adp_acm.o\
		       usb_workqueue.o\
                       usb_depend.o\
		       usb_hotplug_adp.o\
		       usb_otg_dev_adp.o\
                       usb_fast_on_off.o\
                       usb_typec_driver.o
ifeq ($(strip $(MBB_USB_CHARGE)), FEATURE_ON)
obj-y	+= usb_charger_manager.o
endif					   


ifeq ($(CONFIG_ENABLE_TEST_CODE), y)
ifeq ($(CONFIG_BALONG_NCM), y)
g_balong_udc-y       += ncm_balong_test.o
endif
endif


obj-$(CONFIG_USB_BALONG_UDC)	+= g_balong_udc.o

#
# USB unitary drivers
#
NAME :=usb_unitary_driver
obj-$(CONFIG_HUAWEI_USB_COMPILER_OPTION) += usb_unitary_driver.o
$(NAME)-objs :=usb_unitary_module.o usb_hotplug.o  \
               hw_pnp.o f_mass_storage.o
