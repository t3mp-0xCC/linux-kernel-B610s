#
# Makefile for pmic hi6421 coul drivers.
#
-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

ifeq ($(strip $(CFG_CONFIG_PMU_NEW)),YES)
obj-y += pmic_comm.o
obj-y += pmic_init.o
obj-y += pmic_dts.o
obj-y += pmic_volt.o
obj-y += pmic_irq.o
obj-y += pmic_exc.o
obj-y += pmic_exc_debug.o
obj-y += pmic_rtc.o
obj-y += pmic_dr.o
obj-y += pmu_debug.o
obj-y += pmic_clk.o
obj-y += pmic_ioshare.o
endif

ifeq ($(strip $(CFG_CONFIG_COUL)), YES)
obj-y += pmic_coul.o
endif

ifeq ($(strip $(CFG_CONFIG_PMU_GPIO)), YES)
obj-y += pmic_gpio.o
endif

obj-y += pmu_balong.o
obj-y += pmu_stub.o
