-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
# as a module(.ko) or build into kernel(.o)

ifeq ($(strip $(CFG_CONFIG_AUDIO)),y)
EXTRA_CFLAGS += -Isound/soc/balong/common

obj-y += snd_soc_audio.o
else
ifeq ($(strip $(CFG_CONFIG_AUDIO)),m)
EXTRA_CFLAGS += -fno-pic
EXTRA_CFLAGS += -Isound/soc/balong/common
obj-m += snd_soc_audio.o
else
obj-n += snd_soc_audio.o
endif
endif

# soc common
snd_soc_audio-objs +=  \
						../common/sio.o \
						../common/snd_param.o \
						../common/snd_platform.o \
						../common/snd_pcm.o \
						snd_audio_machine.o \
						snd_audio_init.o

all:
	$(MAKE) -C $(KERNEL_OBJ_DIR) M=$(AUDIO_PATH) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	rm -rf   *.o *.ko .*cmd *.mod.c *.order .tmp_versions Module.symvers
