obj-$(CONFIG_NEW_STMMAC_ETH) += stmmac.o
stmmac-$(CONFIG_NEW_STMMAC_PLATFORM) += stmmac_platform.o
stmmac-$(CONFIG_NEW_STMMAC_PCI) += stmmac_pci.o
stmmac-$(CONFIG_NEW_GMAC_TEST) += gmac_test.o
ifeq ($(strip $(CONFIG_BALONG_AVB)), y)
stmmac-objs:= stmmac_balong.o stmmac_main_balong.o stmmac_ethtool.o stmmac_mdio.o ring_mode.o	\
	      dwmac_lib.o dwmac1000_core.o norm_desc.o dwmac1000_mtl.o dwmac1000_dma.o \
	      mmc_core.o stmmac_hwtstamp.o stmmac_ptp.o stmmac_cbs.o $(stmmac-y)
else
stmmac-objs:= stmmac_main.o stmmac_ethtool.o stmmac_mdio.o ring_mode.o	\
	      chain_mode.o dwmac_lib.o dwmac1000_core.o  dwmac1000_dma.o \
	      dwmac100_core.o dwmac100_dma.o enh_desc.o  norm_desc.o \
	      mmc_core.o stmmac_hwtstamp.o stmmac_ptp.o $(stmmac-y)
endif
