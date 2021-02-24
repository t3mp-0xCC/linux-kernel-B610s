#
# Makefile for atp protocol common module.
#
EXTRA_CFLAGS += -Inet/bridge -Ipackage/atp/network/common/inc

TMPKERNELVERSION:=$(VERSION).$(PATCHLEVEL)
ATP_KERNEL_DIR:= $(TMPKERNELVERSION)
#ATP_KERNEL_DIR := $(basename  $(KERNELVERSION))
#vpath %.c  $(ATP_KERNEL_DIR)

obj-y  				:= atp_network_common.o

atp_network_common-objs += atp_arp.o

ifeq ($(CONFIG_ATP_DROP_ARP),y)
atp_network_common-objs += atp_arp_drop.o
endif

atp_network_common-objs += atp_proc.o


obj-$(CONFIG_ATP_QCT_WAN_DEVICE) += qct_rmnet_wan.o
obj-$(CONFIG_ATP_QCT_ONDEMAND_DIAL) += QCTDemandDialFileIO.o
obj-$(CONFIG_ATP_BRIDGE_EBT_TIME) += ebt_time.o
obj-$(CONFIG_ATP_SMUX) += smux.o
obj-$(CONFIG_ATP_NETFILTER_XT_MATCH_DNS_FLT) += xt_dns_flt.o
obj-$(CONFIG_ATP_RMNET) += hw_rmnet.o
obj-$(CONFIG_ATP_ROUTE) += atphooks.o
obj-$(CONFIG_ATP_ROUTE) += atp_sipalg_sock.o
obj-$(CONFIG_ATP_ROUTE) += atp_ip_sock.o
obj-$(CONFIG_ATP_ROUTE) += atp_ipv6_sock_opt.o
obj-$(CONFIG_ATP_ROUTE) += atp_ipv6_skb_mark.o
obj-$(CONFIG_ATP_PRSITE) += atp_prsite.o
obj-$(CONFIG_ATP_GUESTWIFI_ACCESS_UI) += atp_ifaccess.o
obj-$(CONFIG_IMQ) += imq.o
obj-$(CONFIG_IMQ) += atp_qos.o
obj-$(CONFIG_BRIDGE_EBT_WMM_MARK) += ebt_wmm_mark.o 
obj-y += $(ATP_KERNEL_DIR)/
clean:
	rm -f core *.o *.a *.s