#
# Makefile for CoreSight drivers.
#
obj-y += coresight.o
obj-y += coresight_mgr.o
obj-y += of_coresight.o
obj-y += coresight-tmc.o coresight-tpiu.o coresight-etb.o coresight-funnel.o coresight-replicator.o
obj-y += coresight-etm.o 
obj-$(CONFIG_CORESIGHT_SOURCE_ETM) += coresight-etm-cp14.o


subdir-ccflags-y := -Werror