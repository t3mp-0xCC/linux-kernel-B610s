#
# Makefile for atp bridge common module.
#
EXTRA_CFLAGS += -I$(LINUX_KERNEL)/net/bridge -I$(KERNEL_ATP_INC)

obj-y  				:= atp_network_bridge.o

atp_network_bridge-objs += atp_bridge_ioctl.o

atp_network_bridge-$(CONFIG_ATP_BRIDGE_PORT_RELAY) += atp_bridge_port_relay.o

clean:
	rm -f core *.o *.a *.s