# balong spe module
obj-$(CONFIG_BALONG_SPE) 		+= spe_objs.o

spe_objs-y				:= spe.o adp_spe.o spe_balong.o spe_desc.o spe_entry.o spe_port.o spe_cpuport.o spe_dbg.o spe_sysfs.o spe_sysfs_main.o
ifneq ($(CONFIG_DEBUG_FS),)
spe_objs-y				+= spe_debugfs.o
endif

$(info [wi-fi]: CHIP1 = $(MBB_WIFI_CHIP1); CHIP2 = $(MBB_WIFI_CHIP2))
ifneq ($(MBB_WIFI_CHIP1),FEATURE_OFF)
    EXTRA_CFLAGS += -DFEATURE_HUAWEI_MBB_$(shell echo '$(MBB_WIFI_CHIP1)' | tr '[a-z]' '[A-Z]')
endif

