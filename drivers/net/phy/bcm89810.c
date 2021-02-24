#include <linux/phy.h>

/* LRE Register Map */
#define LRE_CONTROL			0x00
#define LRE_STATUS			0x01
#define LRE_PHYID_M			0x02
#define LRE_PHYID_L			0x03
#define LRE_EXTENDED_STATUS	0x0f

/* Auxiliary Register Descriptions*/
#define AU_BR_PHY_EXCTL		0x10
#define AU_BR_PHY_EXSTAT	0x11
#define AU_BR_RX_ERRCNT		0x12
#define AU_BR_FCS_CNT		0x13
#define AU_BR_RX_NOTOK_CNT	0x14
#define AU_EX_REG			0x15
#define AU_EX_REG_ACCESS	0x17
#define AU_SHADOW_ACCESS	0x18
#define AU_INTERRUPT_STAT	0x1a
#define AU_INTERRUPT_MASK	0x1b
#define AU_SP_ACCESS		0x1c
#define AU_TEST_REG			0x1e

/* Shadow value */
#define SPARE_CTRL1			0x02
#define CLOCK_ALIGN_CTRL	0x03
#define SPARE_CTRL2			0x04
#define LED_STATUS			0x08
#define LED_CTRL			0x09
#define LED_SELECTOR1		0x0d
#define LED_SELECTOR2		0x0e
#define LED_GPIO			0x0f
#define EXTERNAL_CTRL		0x0b

/* Expansion Registers */
#define EX_PACKET_CNT		0x00
#define EX_MUL_LED_SEL		0x04
#define EX_MUL_LED_FLASH	0x05
#define EX_SYNC_ETH_CNTL	0x0e
#define EX_SOFT_RST			0x70
#define EX_MISC_CNTL_A		0x90
#define EX_MISC_CNTL_B		0x91
#define EX_MISC_CNTL_C		0x92

/* Top_Level Expansion Registers */
#define IEEE_1588_SLICE		0x00
#define IEEE_1588_EVENT_T	0x01
#define IEEE_1588_EVENT_R	0x02
#define IEEE_1588_SOPTS_CAP	0x03
#define IEEE_1588_TS_OFFSET	0x05
#define IEEE_1588_TIMECODE0	0x07
#define IEEE_1588_TIMECODE1	0x08
#define IEEE_1588_TIMECODE2	0x09
#define IEEE_1588_TIMECODE3	0x0a
#define IEEE_1588_INT_MASK	0x11
#define IEEE_1588_CNTL		0x12
#define TL_VLAN1_ITPID		0x15
#define TL_VLAN2_OTPID		0x16
#define TL_VLAN2_OTPID2		0x17
#define TL_NSE_NCO1			0x25
#define TL_NSE_NCO21		0x26
#define TL_NSE_NCO22		0x27
#define TL_NSE_NCO23		0x28
#define TL_SPARE			0x34
#define TL_SOP_TS1			0x50
#define TL_SOP_TS2			0x51
#define TL_SOP_TS3			0x52
#define TL_SOP_TS_INFO1		0x53
#define TL_SOP_TS_INFO2		0x54
#define TL_HEARTBEAT_TS1	0x55
#define TL_HEARTBEAT_TS2	0x56
#define TL_HEARTBEAT_TS3	0x57
#define TL_PKT_1588_CNT_RST	0x5b
#define TL_LINKDELAY1		0x5c
#define TL_LINKDELAY2		0x5d

struct lre_control_s
{
	unsigned short reset	:1;
	unsigned short shorternal_loopback	:1;
	unsigned short reserved	:2;
	unsigned short power_down	:1;
	unsigned short isolate	:1;
	unsigned short speed_selection	:4;
	unsigned short pair_selection	:2;
	unsigned short master_slave_selection	:1;
	unsigned short reserved1	:3;
};

struct lre_status_s
{
	unsigned short reserved	:2;
	unsigned short m100_one_pair_capable	:1;
	unsigned short reserved1	:4;
	unsigned short extended_status	:1;
	unsigned short reserved2	:1;
	unsigned short management_frames_preamble_supperssion	:1;
	unsigned short reserved3	:1;
	unsigned short support_ieee_802_3_phy	:1;
	unsigned short reserved4	:1;
	unsigned short link_status	:1;
	unsigned short receive_jabber_detect	:1;
	unsigned short extended_cap	:1;
};

struct lre_extended_status_s
{
	unsigned short reserved	:6;
	unsigned short local_receive_status	:1;
	unsigned short remote_receive_status	:1;
	unsigned short idle_error_count	:8;
};

struct broadr_reach_control_s
{
	unsigned short reserved	:2;
	unsigned short transmit_disable	:1;
	unsigned short shorterrupt_disable	:1;
	unsigned short force_shorterrupt	:1;
	unsigned short reserved1	:5;
	unsigned short enable_led_traffic_mode	:1;
	unsigned short force_leds_on	:1;
	unsigned short force_leds_off	:1;
	unsigned short reserved2	:2;
	unsigned short pcs_transmit_fifo_elasticity	:1;
};

struct broadr_reach_status_s
{
	unsigned short auto_negotiation_base_page_selector_field_mismatch	:1;
	unsigned short reserved	:1;
	unsigned short mdi_crossover_state	:1;
	unsigned short shorterrupt_status	:1;
	unsigned short remote_receiver_status	:1;
	unsigned short locked	:1;
	unsigned short link_status	:1;
	unsigned short crc_error_detected	:1;
	unsigned short carrier_error_detected	:1;
	unsigned short carrier_extension_error_detected	:1;
	unsigned short bad_ssd_detected	:1;
	unsigned short bad_esd_detected	:1;
	unsigned short receive_error_detected	:1;
	unsigned short transmit_error_detected	:1;
	unsigned short lock_error_detected	:1;
	unsigned short reserved1	:1;
};

struct broad_reach_false_carrier_sense_counter_s
{
	unsigned short reserved	:8;
	unsigned short false_carrier_sense_counter	:8;
};

struct board_reach_receiver_not_ok_counter_s
{
	unsigned short local_receiver_not_ok_counter	:8;
	unsigned short remote_receiver_not_ok_counter	:8;
};

#define EXPANSION_SELECTED		0x0f
#define TOPLEVEL_SELECTED		0x0d
#define EXPANSION_NOTSELECTED	0x00

struct expansion_register_access_s
{
	unsigned short reserved	:4;
	unsigned short expansion_register_select	:4;
	unsigned short expansion_register_address	:8;
};


#define SHADOWS_ACCESS		0x00
#define SHADOWS_AUX_CTRL	0x01
#define SHADOWS_PWR_CTRL	0x02
#define SHADOWS_MISC_TEST	0x04
#define SHADOWS_MISC_CTRL	0x0f

struct shadows_read_s
{
	unsigned short	enable	:1;
	unsigned short	yyy	:3;
	unsigned short	dont_care	:9;
	unsigned short	misc_reg	:3;
};

struct shadows_write_s
{
	unsigned short	desired_value	:13;
	unsigned short	yyy	:3;
};

struct auxiliary_control_s
{
	unsigned short	external_loopback	:1;
	unsigned short	extended_packet_length	:1;
	unsigned short	reserved	:3;
	unsigned short	transmit_mode	:1;
	unsigned short	reserved1	:7;
	unsigned short	shadow_reg_select	:3;
};

struct power_mii_control_s
{
	unsigned short	reserved	:10;
	unsigned short	super_isolate	:1;
	unsigned short	reserved1	:2;
	unsigned short	shadow_reg_select	:3;
};

struct misc_test_s
{
	unsigned short	line_side_loopback_enable	:1;
	unsigned short	reserved	:3;
	unsigned short	line_side_loop_back_tristate	:1;
	unsigned short	reserved1	:8;
	unsigned short	shadow_reg_select	:3;
};

struct misc_control_s
{
	unsigned short	write_enable	:1;
	unsigned short	read_selector	:3;
	unsigned short	packet_conter_mode	:1;
	unsigned short	bypass_wirespeed_timer	:1;
	unsigned short	reserved	:1;
	unsigned short	rgmii_rx_skew	:1;
	unsigned short	rgmii_enable	:1;
	unsigned short	reserved1	:4;
	unsigned short	shadow_reg_select	:3;
};

struct inerrupt_status_s
{
	unsigned short	energy_detect_change	:1;
	unsigned short	illegal_pair_swap	:1;
	unsigned short	reserved	:1;
	unsigned short	exceeded_high_counter_threshold	:1;
	unsigned short	exceeded_low_counter_threshold	:1;
	unsigned short	reserved1	:4;
	unsigned short	scrambler_sync_error	:1;
	unsigned short	remote_receiver_status_change	:1;
	unsigned short	local_receiver_status_change	:1;
	unsigned short	duplex_mode_change	:1;
	unsigned short	link_speed_change	:1;
	unsigned short	link_status_change	:1;
	unsigned short	crc_error	:1;
};

struct interrupt_mask_s
{
	unsigned short	energy_detect_change	:1;
	unsigned short	illegal_pair_swap	:1;
	unsigned short	reserved	:1;
	unsigned short	exceeded_high_counter_threshold	:1;
	unsigned short	exceeded_low_counter_threshold	:1;
	unsigned short	reserved1	:4;
	unsigned short	scrambler_sync_error	:1;
	unsigned short	remote_receiver_status_change	:1;
	unsigned short	local_receiver_status_change	:1;
	unsigned short	duplex_mode_change	:1;
	unsigned short	link_speed_change	:1;
	unsigned short	link_status_change	:1;
	unsigned short	crc_error	:1;
};

struct spare_control1_s
{
	unsigned short	write_enable	:1;
	unsigned short	shadow_value	:5;
	unsigned short	reserved	:9;
	unsigned short	link_led_mode	:1;
};

struct clock_alignment_control_s
{
	unsigned short	write_enable	:1;
	unsigned short	shadow_value	:5;
	unsigned short	gtxclk_delay_enable	:1;
	unsigned short	reserved	:9;
};

struct spare_control2_s
{
	unsigned short	write_enable	:1;
	unsigned short	shadow_value	:5;
	unsigned short	reserved	:10;
};

struct led_status_s
{
	unsigned short	write_enable	:1;
	unsigned short	shadow_value	:5;
	unsigned short	reserved	:1;
	unsigned short	slave_indicator	:1;
	unsigned short	fdx_indicator	:1;
	unsigned short	intr_indicator	:1;
	unsigned short	reserved1	:1;
	unsigned short	linkspd_indicator	:2;
	unsigned short	transmit_indicator	:1;
	unsigned short	receive_indicator	:1;
	unsigned short	quality_indicator	:1;
};

struct led_control_s
{
	unsigned short	write_enable	:1;
	unsigned short	shadow_value	:5;
	unsigned short	reserved	:5;
	unsigned short	act_link_led_en	:1;
	unsigned short	act_led_en	:1;
	unsigned short	remote_fault_led_en	:1;
	unsigned short	link_uti_led_sel	:2;
};

struct external_control_s
{
	unsigned short	write_enable	:1;
	unsigned short	shadow_value	:5;
	unsigned short	reserved	:2;
	unsigned short	freeze_agc	:1;
	unsigned short	reserved1	:2;
	unsigned short	rgmii_pad_config	:2;
	unsigned short	lom_led	:1;
	unsigned short	reserved2	:2;
};

struct led_selector1_s
{
	unsigned short	write_enable	:1;
	unsigned short	shadow_value	:5;
	unsigned short	reserved	:2;
	unsigned short	led2_sel	:4;
	unsigned short	led1_sel	:4;
};

struct led_selector2_s
{
	unsigned short	write_enable	:1;
	unsigned short	shadow_value	:5;
	unsigned short	reserved	:2;
	unsigned short	led4_sel	:4;
	unsigned short	led3_sel	:4;
};

struct led_gpio_ctrl_stat_s
{
	unsigned short	write_enable	:1;
	unsigned short	shadow_value	:5;
	unsigned short	reserved	:2;
	unsigned short	led_status	:4;
	unsigned short	led_ctrl	:4;
};

struct test_reg1_s
{
	unsigned short	crc_error_cnt_sel	:1;
	unsigned short	reserved	:2;
	unsigned short	force_link	:1;
	unsigned short	reserved1	:12;
};

struct multicolor_led_selector_s
{
	unsigned short	reserved	:6;
	unsigned short	flash_now	:1;
	unsigned short	in_phase	:1;
	unsigned short	multicolor_led2	:4;
	unsigned short	multicolor_led1	:4;
};

struct multicolor_flash_s
{
	unsigned short	reserved	:4;
	unsigned short	alternating_rate	:6;
	unsigned short	flash_rate	:6;
};

struct multicolor_led_blink_s
{
	unsigned short	reserved	:10;
	unsigned short	blink_update_now	:1;
	unsigned short	blink_rate	:5;
};	

struct sync_eth_ctrl_s
{
	unsigned short	reserved	:4;
	unsigned short	mii_lite_en	:1;
	unsigned short	tx_sop_en	:1;
	unsigned short	tx_sop_sel	:2;
	unsigned short	rx_sop_en	:1;
	unsigned short	rx_sop_sel	:2;
	unsigned short	reserved1	:1;
	unsigned short	rx_er_mux	:1;
	unsigned short	reserved2	:3;
};

struct soft_reset_s
{
	unsigned short reserved	:15;
	unsigned short soft_reset	:1;
};

struct lre_misc_control_a_s
{
	unsigned short	digital_HP_filter	:1;
	unsigned short	reserved	:13;
	unsigned short	force_link_ctrl	:1;
	unsigned short	broadr_reach_en	:1;
};

struct lre_misc_control_b_s
{
	unsigned short	digital_HPF	:1;
	unsigned short	reserved	:3;
	unsigned short	br_pga	:3;
	unsigned short	reserved1	:9;
};

struct lre_misc_control_c_s
{
	unsigned short	reserved	:3;
	unsigned short	edge_rate	:2;
	unsigned short	reserved1	:11;
};

struct reverse_mii_ctrl_s
{
	unsigned short	reserved	:13;
	unsigned short	restart_rx_fifo	:1;
	unsigned short	restart_tx_fifo	:1;
	unsigned short	rmii_en	:1;
};

struct tx_slice_en_s
{
	unsigned short	reserved	:14;
	unsigned short	rx_slice_en	:1;
	unsigned short	tx_slice_ne	:1;
};

struct tx_event_msg_mode_s
{
	unsigned short	reserved	:4;
	unsigned short	tx_mode2	:4;
	unsigned short	tx_mode1	:8;
};

struct rx_event_msg_mode_s
{
	unsigned short	reserved	:4;
	unsigned short	rx_mode2	:4;
	unsigned short	rx_mode1	:8;
};

struct sop_timestamp_cap_en_s
{
	unsigned short	reserved	:14;
	unsigned short	rx_cap_en	:1;
	unsigned short	tx_cap_en	:1;
};

struct ieee_1588_control_s
{
	unsigned short	rx_as_da_en	:1;
	unsigned short	rx_l2_da_en	:1;
	unsigned short	rx_l4_ip_ado_en	:1;
	unsigned short	rx_l4_ipv6_ado_en	:1;
	unsigned short	rx_as_en	:1;
	unsigned short	rx_l2_en	:1;
	unsigned short	rx_ipv4_udp_en	:1;
	unsigned short	rx_ipv6_udp_en	:1;
	unsigned short	tx_as_da_en	:1;
	unsigned short	tx_l2_da_en	:1;
	unsigned short	tx_l4_ip_ado_en	:1;
	unsigned short	tx_l4_ipv6_ado_en	:1;
	unsigned short	tx_as_en	:1;
	unsigned short	tx_l2_en	:1;
	unsigned short	tx_ipv4_udp_en	:1;
	unsigned short	tx_ipv6_udp_en	:1;
};

struct nse_nco_1_s
{
	unsigned short	reserved	:1;
	unsigned short	freq_mdio_sel	:1;
	unsigned short	reserved1	:2;
	unsigned short	local_time_msb	:12;
};

struct spare_reg_s
{
	unsigned short reserved	:14;
	unsigned short	clk125_output	:1;
	unsigned short	reserved1	:1;
};

struct packet_counter_rst
{
	unsigned short reserved	:14;
	unsigned short	rst_rx_cnt	:1;
	unsigned short	rst_tx_cnt	:1;
};


#define PHY_ID_BCM89810 0x03625cc2

static int bcm89810_config_init(struct phy_device *phydev)
{
	phy_write(phydev, LRE_CONTROL, 0x8000);
	phy_write(phydev, AU_EX_REG_ACCESS, 0x0f93);
	phy_write(phydev, AU_EX_REG, 0x107f);
	phy_write(phydev, AU_EX_REG_ACCESS, 0x0f90);
	phy_write(phydev, AU_EX_REG, 0x0001);
	phy_write(phydev, LRE_CONTROL, 0x3000);
	phy_write(phydev, LRE_CONTROL, 0x0200);
	phy_write(phydev, AU_SHADOW_ACCESS, 0x0c00);
	phy_write(phydev, AU_EX_REG_ACCESS, 0x0f90);
	phy_write(phydev, AU_EX_REG, 0x0);
	phy_write(phydev, LRE_CONTROL, 0x0100);
	phy_write(phydev, AU_EX_REG_ACCESS, 0x0001);
	phy_write(phydev, AU_EX_REG, 0x27);
	phy_write(phydev, AU_EX_REG_ACCESS, 0x000e);
	phy_write(phydev, AU_EX_REG, 0x9b52);
	phy_write(phydev, AU_EX_REG_ACCESS, 0x000f);
	phy_write(phydev, AU_EX_REG, 0xa04d);
	phy_write(phydev, AU_EX_REG_ACCESS, 0x0f90);
	phy_write(phydev, AU_EX_REG, 0x0001);
	phy_write(phydev, AU_EX_REG_ACCESS, 0x0f92);
	phy_write(phydev, AU_EX_REG, 0x9225);
	phy_write(phydev, AU_EX_REG_ACCESS, 0x000a);
	phy_write(phydev, AU_EX_REG, 0x0323);
	phy_write(phydev, AU_EX_REG_ACCESS, 0x0ffd);
	phy_write(phydev, AU_EX_REG, 0x1c3f);
	phy_write(phydev, AU_EX_REG_ACCESS, 0x0ffd);
	phy_write(phydev, AU_EX_REG, 0x1c3f);
	phy_write(phydev, AU_EX_REG_ACCESS, 0x0f99);
	phy_write(phydev, AU_EX_REG, 0x7180);
	phy_write(phydev, AU_EX_REG_ACCESS, 0x0f9a);
	phy_write(phydev, AU_EX_REG, 0x34c0);

	if(PHY_INTERFACE_MODE_RGMII == phydev->interface)
	{
		phy_write(phydev, AU_EX_REG_ACCESS, 0x0f0e);
		phy_write(phydev, AU_EX_REG, 0x0000);
		phy_write(phydev, AU_EX_REG_ACCESS, 0x0f9f);
		phy_write(phydev, AU_EX_REG, 0x0000);
		phy_write(phydev, AU_SHADOW_ACCESS, 0xf1e7);
	}
	else if(PHY_INTERFACE_MODE_MII == phydev->interface)
	{
		phy_write(phydev, AU_EX_REG_ACCESS, 0x0f0e);
		phy_write(phydev, AU_EX_REG, 0x0800);
		phy_write(phydev, AU_EX_REG_ACCESS, 0x0f9f);
		phy_write(phydev, AU_EX_REG, 0x0000);
		phy_write(phydev, AU_SHADOW_ACCESS, 0xf067);
	}

#ifdef MII_MASTER
	phy_write(phydev, LRE_CONTROL, 0x0208);
#else
	phy_write(phydev, LRE_CONTROL, 0x0200);
#endif

	phydev->autoneg = AUTONEG_DISABLE;
	phydev->speed = SPEED_100;
	phydev->duplex = DUPLEX_FULL;
	
	return 0;
}

int bcm89810_read_status(struct phy_device *phydev)
{
	unsigned short ret;
	struct lre_status_s *p = (struct lre_status_s *)&ret;

	// fake read to refresh the phy reg
	phy_read(phydev, LRE_STATUS);
	
	ret = phy_read(phydev, LRE_STATUS);

	if(p->link_status)
	{
		phydev->link = 1;
	}
	else
	{
		phydev->link = 0;
	}

	phydev->speed = SPEED_100;
	phydev->duplex = DUPLEX_FULL;

	phydev->pause = phydev->asym_pause = 0;

	return 0;
}
	
int bcm89810_config_aneg(struct phy_device* phydev)
{
	static int retry = 0;
	if(retry%2)
	{
		phy_write(phydev, LRE_CONTROL, 0x0200);
	}
	else
	{
		phy_write(phydev, LRE_CONTROL, 0x0208);
	}
	retry++;
	return 0;
}

struct phy_driver bcm89810_driver = 
{
	.phy_id		= PHY_ID_BCM89810,
	.phy_id_mask	= 0xfffffff0,
	.name		= "BCM89810",
	.config_init	= bcm89810_config_init,
	.read_status	= bcm89810_read_status,
	.config_aneg	= bcm89810_config_aneg, 
	.driver		= { .owner = THIS_MODULE },
};

static int __init bcm89810_init(void)
{
	return phy_driver_register(&bcm89810_driver);
}

static void __exit bcm89810_exit(void)
{
	phy_driver_unregister(&bcm89810_driver);
}

module_init(bcm89810_init);
module_exit(bcm89810_exit);

static struct mdio_device_id __maybe_unused bcm89810_tbl[] = {
	{ PHY_ID_BCM89810, 0xfffffff0 },
	{ }
};

MODULE_DEVICE_TABLE(mdio, bcm89810_tbl);



