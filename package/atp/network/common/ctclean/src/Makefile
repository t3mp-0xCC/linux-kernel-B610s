#
# Makefile for pptp fast forward.
#
EXTRA_CFLAGS += -I$(LINUX_KERNEL)/net/netlink -I$(KERNEL_ATP_INC) -I$(KERNEL_PACKAGE_ATP)/ssp

obj-$(CONFIG_ATP_COMMON_CT_CLEAN) := atp_common_ct_clean.o atp_common_ct_block.o

clean:
	rm -f core *.o *.a *.s