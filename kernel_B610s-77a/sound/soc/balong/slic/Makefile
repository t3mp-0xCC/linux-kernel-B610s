-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
# as a module(.ko) or build into kernel(.o)
ifeq ($(strip $(BUILD_MODE)),factory)
ifeq ($(strip $(CFG_CONFIG_SLIC)),m)
	CFG_CONFIG_SLIC := y
endif
endif
ifeq ($(strip $(CFG_CONFIG_SLIC)),y)
# copy zarlink api to here
EXTRA_CFLAGS += -Isound/soc/balong/common -Isound/soc/balong/slic/zarlink_api/includes -Isound/soc/balong/slic/profiles -I$(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config
obj-y += snd_soc_balong.o
else
ifeq ($(strip $(CFG_CONFIG_SLIC)),m)
EXTRA_CFLAGS += -fno-pic
EXTRA_CFLAGS += -Isound/soc/balong/common -Isound/soc/balong/slic/zarlink_api/includes -Isound/soc/balong/slic/profiles -I$(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config
					
obj-m += snd_soc_balong.o
else
obj-n += snd_soc_balong.o
endif
endif

#profiles
snd_soc_balong-objs +=  profiles/ZLR96411L_SM2_CHINA.o \
			profiles/ZLR96411L_SM2_AUSTRALIA.o \
			profiles/ZLR96411L_SM2_GERMANY.o \
			profiles/ZLR96411L_SM2_POLAND.o
						
# soc common
snd_soc_balong-objs +=  ../common/spi.o \
						../common/sio.o \
						../common/snd_param.o \
						../common/snd_platform.o \
						../common/snd_pcm.o

# slic driver
snd_soc_balong-objs +=  vp_hal.o \
						zsi.o \
						slic_ctl.o \
						snd_slic.o \
						snd_machine.o \
						snd_init.o

# zsi irq test
#snd_soc_balong-objs +=  zsi_irq_test.o


# zarlink api
snd_soc_balong-objs +=  zarlink_api/common/vp_api_common.o \
                        zarlink_api/common/vp_api_config.o \
                        zarlink_api/common/vp_api_control.o \
                        zarlink_api/common/vp_api_cslac_seq.o \
                        zarlink_api/common/vp_api_init.o \
                        zarlink_api/common/vp_api_query.o \
                        zarlink_api/common/vp_api_test.o \
                        zarlink_api/common/vp_debug.o \
                        zarlink_api/common/vp_dtmf_detect.o \
                        zarlink_api/common/vp_pulse_decode.o \
                        zarlink_api/common/vp_timer_queue.o \
                        zarlink_api/common/sys_service.o \
                        zarlink_api/vp886_api/vp886_adaptive_ringing.o \
                        zarlink_api/vp886_api/vp886_calibration_common.o \
                        zarlink_api/vp886_api/vp886_common.o \
                        zarlink_api/vp886_api/vp886_control.o \
                        zarlink_api/vp886_api/vp886_events.o \
                        zarlink_api/vp886_api/vp886_init.o \
                        zarlink_api/vp886_api/vp886_query.o \
                        zarlink_api/vp886_api/vp886_seq.o \
                        zarlink_api/vp886_api/vp886_slac.o \
                        zarlink_api/vp886_api/vp886_timers.o

all:
	$(MAKE) -C $(KERNEL_OBJ_DIR) M=$(SLIC_PATH) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	rm -rf   *.o *.ko .*cmd *.mod.c *.order .tmp_versions Module.symvers

