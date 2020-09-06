include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
EXTRA_CFLAGS += -Isound/soc/balong/common/

obj-$(CONFIG_SND_MODULE_PCM) += snd_module_pcm.o


snd_module_pcm-objs +=  \
						../common/sio.o \
						../common/snd_platform.o \
						snd_module_platform.o \
						snd_module_dai.o \
						snd_module_machine.o \
						snd_dummy_codec.o \
						snd_module_init.o


