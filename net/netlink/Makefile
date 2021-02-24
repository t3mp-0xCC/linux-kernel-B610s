#
# Makefile for the netlink driver.
#

#/******************************************************************************************************
#     问题单号                       时间                   修改人                        说明

#*****************************************************************************************************/

-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
ifeq ($(strip $(MBB_COMMON)),FEATURE_ON)
obj-y  				:= af_netlink.o genetlink.o hw_netlink.o syswatch_ProMonitor.o hw_atnetlink.o hw_atnldispatch.o atp_hw_netlink.o
else
obj-y  				:= af_netlink.o genetlink.o
endif
obj-$(CONFIG_NETLINK_DIAG)	+= netlink_diag.o
obj-y += $(if $(CONFIG_ATP_CT_CLEAN), hw_ctclean.o)
netlink_diag-y			:= diag.o
