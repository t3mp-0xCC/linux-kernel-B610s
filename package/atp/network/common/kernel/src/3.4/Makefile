#
# Makefile for atp protocol common module.
#

obj-$(CONFIG_ATP_IP_NF_TARGET_ALLCONENAT) += ipt_ALLMASQUERADE.o
obj-$(CONFIG_ATP_NF_CONNTRACK_RTSP) += nf_nat_rtsp.o  
obj-$(CONFIG_ATP_IP_NF_TARGET_PORTTRIGER) += ipt_PORT_TRIGGERING.o
obj-$(CONFIG_ATP_NF_CONNTRACK_RTSP) += nf_conntrack_rtsp.o

clean:
	rm -f core *.o *.a *.s
