#ifndef __HI6551_H__
#define __HI6551_H__ 
#define HI6551_STATUS1_OFFSET (0x1)
#define HI6551_STATUS2_OFFSET (0x2)
#define HI6551_IRQ1_OFFSET (0x3)
#define HI6551_IRQ2_OFFSET (0x4)
#define HI6551_IRQ3_OFFSET (0x5)
#define HI6551_IRQ4_OFFSET (0x6)
#define HI6551_COUL_IRQ_OFFSET (0x7)
#define HI6551_IRQ1_MASK_OFFSET (0x8)
#define HI6551_IRQ2_MASK_OFFSET (0x9)
#define HI6551_IRQ3_MASK_OFFSET (0xA)
#define HI6551_IRQ4_MASK_OFFSET (0xB)
#define HI6551_COUL_IRQ_MASK_OFFSET (0xC)
#define HI6551_SCP_RECORD1_OFFSET (0xD)
#define HI6551_OCP_RECORD1_OFFSET (0xE)
#define HI6551_OCP_RECORD2_OFFSET (0xF)
#define HI6551_OCP_RECORD3_OFFSET (0x10)
#define HI6551_OCP_RECORD4_OFFSET (0x11)
#define HI6551_OCP_RECORD5_OFFSET (0x12)
#define HI6551_OCP_RECORD6_OFFSET (0x13)
#define HI6551_OCP_RECORD7_OFFSET (0x14)
#define HI6551_NP_IRQ1_RECORD_OFFSET (0x15)
#define HI6551_NP_IRQ2_RECORD_OFFSET (0x16)
#define HI6551_NP_IRQ3_RECORD_OFFSET (0x17)
#define HI6551_NP_SCP_RECORD1_OFFSET (0x18)
#define HI6551_NP_OCP_RECORD1_OFFSET (0x19)
#define HI6551_NP_OCP_RECORD2_OFFSET (0x1A)
#define HI6551_NP_OCP_RECORD3_OFFSET (0x1B)
#define HI6551_NP_OCP_RECORD4_OFFSET (0x1C)
#define HI6551_NP_OCP_RECORD5_OFFSET (0x1D)
#define HI6551_NP_OCP_RECORD6_OFFSET (0x1E)
#define HI6551_NP_OCP_RECORD7_OFFSET (0x1F)
#define HI6551_OCP_DEB_OFFSET (0x20)
#define HI6551_ONOFF8_OFFSET (0x21)
#define HI6551_OCP_SCP_MOD_CTRL1_OFFSET (0x22)
#define HI6551_OCP_SCP_MOD_CTRL2_OFFSET (0x23)
#define HI6551_OCP_MOD_CTRL3_OFFSET (0x24)
#define HI6551_OCP_MOD_CTRL4_OFFSET (0x25)
#define HI6551_OCP_MOD_CTRL5_OFFSET (0x26)
#define HI6551_OCP_MOD_CTRL6_OFFSET (0x27)
#define HI6551_OCP_MOD_CTRL7_OFFSET (0x28)
#define HI6551_OCP_MOD_CTRL8_OFFSET (0x29)
#define HI6551_OCP_MOD_CTRL9_OFFSET (0x2A)
#define HI6551_OCP_MOD_CTRL10_OFFSET (0x2B)
#define HI6551_OCP_MOD_CTRL11_OFFSET (0x2C)
#define HI6551_ENABLE1_OFFSET (0x2D)
#define HI6551_DISABLE1_OFFSET (0x2E)
#define HI6551_ONOFF_STATUS1_OFFSET (0x2F)
#define HI6551_ENABLE2_OFFSET (0x30)
#define HI6551_DISABLE2_OFFSET (0x31)
#define HI6551_ONOFF_STATUS2_OFFSET (0x32)
#define HI6551_ENABLE3_OFFSET (0x33)
#define HI6551_DISABLE3_OFFSET (0x34)
#define HI6551_ONOFF_STATUS3_OFFSET (0x35)
#define HI6551_ENABLE4_OFFSET (0x36)
#define HI6551_DISABLE4_OFFSET (0x37)
#define HI6551_ONOFF_STATUS4_OFFSET (0x38)
#define HI6551_ENABLE5_OFFSET (0x39)
#define HI6551_DISABLE5_OFFSET (0x3A)
#define HI6551_ONOFF_STATUS5_OFFSET (0x3B)
#define HI6551_ENABLE6_OFFSET (0x3C)
#define HI6551_DISABLE6_OFFSET (0x3D)
#define HI6551_ONOFF_STATUS6_OFFSET (0x3E)
#define HI6551_ENABLE7_OFFSET (0x3F)
#define HI6551_SIMCARD_EN_OFFSET (0x40)
#define HI6551_SIM0_CFG_OFFSET (0x41)
#define HI6551_SIM1_CFG_OFFSET (0x42)
#define HI6551_SIM_DEB_OFFSET (0x43)
#define HI6551_ECO_MOD_CFG1_OFFSET (0x44)
#define HI6551_ECO_MOD_CFG2_OFFSET (0x45)
#define HI6551_ECO_MOD_CFG3_OFFSET (0x46)
#define HI6551_ECO_MOD_CFG4_OFFSET (0x47)
#define HI6551_ENB3_ECO_MARK_OFFSET (0x48)
#define HI6551_BUCK01_REG1_ADJ_OFFSET (0x49)
#define HI6551_BUCK01_REG2_ADJ_OFFSET (0x4A)
#define HI6551_BUCK01_REG3_ADJ_OFFSET (0x4B)
#define HI6551_BUCK01_REG4_ADJ_OFFSET (0x4C)
#define HI6551_BUCK01_REG5_ADJ_OFFSET (0x4D)
#define HI6551_BUCK01_REG6_ADJ_OFFSET (0x4E)
#define HI6551_BUCK01_REG7_ADJ_OFFSET (0x4F)
#define HI6551_BUCK01_REG8_ADJ_OFFSET (0x50)
#define HI6551_BUCK01_REG9_ADJ_OFFSET (0x51)
#define HI6551_BUCK01_REG10_ADJ_OFFSET (0x52)
#define HI6551_BUCK01_REG11_ADJ_OFFSET (0x53)
#define HI6551_BUCK01_REG12_ADJ_OFFSET (0x54)
#define HI6551_BUCK01_REG13_ADJ_OFFSET (0x55)
#define HI6551_BUCK01_REG14_ADJ_OFFSET (0x56)
#define HI6551_BUCK01_REG15_ADJ_OFFSET (0x57)
#define HI6551_BUCK01_REG16_ADJ_OFFSET (0x58)
#define HI6551_BUCK01_REG17_ADJ_OFFSET (0x59)
#define HI6551_BUCK2_REG1_ADJ_OFFSET (0x5A)
#define HI6551_BUCK2_REG2_ADJ_OFFSET (0x5B)
#define HI6551_BUCK2_REG3_ADJ_OFFSET (0x5C)
#define HI6551_BUCK2_REG4_ADJ_OFFSET (0x5D)
#define HI6551_BUCK2_REG5_ADJ_OFFSET (0x5E)
#define HI6551_BUCK2_REG6_ADJ_OFFSET (0x5F)
#define HI6551_BUCK2_REG7_ADJ_OFFSET (0x60)
#define HI6551_BUCK2_REG8_ADJ_OFFSET (0x61)
#define HI6551_BUCK2_REG9_ADJ_OFFSET (0x62)
#define HI6551_BUCK2_REG10_ADJ_OFFSET (0x63)
#define HI6551_BUCK2_REG11_ADJ_OFFSET (0x64)
#define HI6551_BUCK2_REG12_ADJ_OFFSET (0x65)
#define HI6551_BUCK3_REG1_ADJ_OFFSET (0x66)
#define HI6551_BUCK3_REG2_ADJ_OFFSET (0x67)
#define HI6551_BUCK3_REG3_ADJ_OFFSET (0x68)
#define HI6551_BUCK3_REG4_ADJ_OFFSET (0x69)
#define HI6551_BUCK3_REG5_ADJ_OFFSET (0x6A)
#define HI6551_BUCK3_REG6_ADJ_OFFSET (0x6B)
#define HI6551_BUCK3_REG7_ADJ_OFFSET (0x6C)
#define HI6551_BUCK3_REG8_ADJ_OFFSET (0x6D)
#define HI6551_BUCK3_REG9_ADJ_OFFSET (0x6E)
#define HI6551_BUCK3_REG10_ADJ_OFFSET (0x6F)
#define HI6551_BUCK3_REG11_ADJ_OFFSET (0x70)
#define HI6551_BUCK3_REG12_ADJ_OFFSET (0x71)
#define HI6551_BUCK4_REG1_ADJ_OFFSET (0x72)
#define HI6551_BUCK4_REG2_ADJ_OFFSET (0x73)
#define HI6551_BUCK4_REG3_ADJ_OFFSET (0x74)
#define HI6551_BUCK4_REG4_ADJ_OFFSET (0x75)
#define HI6551_BUCK4_REG5_ADJ_OFFSET (0x76)
#define HI6551_BUCK4_REG6_ADJ_OFFSET (0x77)
#define HI6551_BUCK4_REG7_ADJ_OFFSET (0x78)
#define HI6551_BUCK4_REG8_ADJ_OFFSET (0x79)
#define HI6551_BUCK4_REG9_ADJ_OFFSET (0x7A)
#define HI6551_BUCK4_REG10_ADJ_OFFSET (0x7B)
#define HI6551_BUCK4_REG12_ADJ_OFFSET (0x7C)
#define HI6551_BUCK5_REG1_ADJ_OFFSET (0x7D)
#define HI6551_BUCK5_REG2_ADJ_OFFSET (0x7E)
#define HI6551_BUCK5_REG3_ADJ_OFFSET (0x7F)
#define HI6551_BUCK5_REG4_ADJ_OFFSET (0x80)
#define HI6551_BUCK5_REG5_ADJ_OFFSET (0x81)
#define HI6551_BUCK5_REG6_ADJ_OFFSET (0x82)
#define HI6551_BUCK5_REG7_ADJ_OFFSET (0x83)
#define HI6551_BUCK5_REG8_ADJ_OFFSET (0x84)
#define HI6551_BUCK5_REG9_ADJ_OFFSET (0x85)
#define HI6551_BUCK5_REG10_ADJ_OFFSET (0x86)
#define HI6551_BUCK5_REG12_ADJ_OFFSET (0x87)
#define HI6551_BUCK5_REG13_ADJ_OFFSET (0x88)
#define HI6551_BUCK5_REG14_ADJ_OFFSET (0x89)
#define HI6551_BUCK6_REG1_ADJ_OFFSET (0x8A)
#define HI6551_BUCK6_REG2_ADJ_OFFSET (0x8B)
#define HI6551_BUCK6_REG3_ADJ_OFFSET (0x8C)
#define HI6551_BUCK6_REG4_ADJ_OFFSET (0x8D)
#define HI6551_BUCK6_REG5_ADJ_OFFSET (0x8E)
#define HI6551_BUCK6_REG6_ADJ_OFFSET (0x8F)
#define HI6551_BUCK6_REG7_ADJ_OFFSET (0x90)
#define HI6551_BUCK6_REG8_ADJ_OFFSET (0x91)
#define HI6551_BUCK6_REG9_ADJ_OFFSET (0x92)
#define HI6551_BUCK6_REG10_ADJ_OFFSET (0x93)
#define HI6551_BUCK6_REG11_ADJ_OFFSET (0x94)
#define HI6551_BUCK6_REG12_ADJ_OFFSET (0x95)
#define HI6551_CHG_PUMP2_ADJ_OFFSET (0x96)
#define HI6551_VSET_BUCK01_ADJ_OFFSET (0x97)
#define HI6551_VSET_BUCK2_ADJ_OFFSET (0x98)
#define HI6551_VSET_BUCK3_ADJ_OFFSET (0x99)
#define HI6551_VSET_BUCK4_ADJ_OFFSET (0x9A)
#define HI6551_VSET_BUCK5_ADJ_OFFSET (0x9B)
#define HI6551_VSET_BUCK6_ADJ_OFFSET (0x9C)
#define HI6551_LDO1_REG_ADJ_OFFSET (0x9D)
#define HI6551_LDO2_REG_ADJ_OFFSET (0x9E)
#define HI6551_LDO3_REG_ADJ_OFFSET (0x9F)
#define HI6551_LDO4_REG_ADJ_OFFSET (0xA0)
#define HI6551_LDO5_REG_ADJ_OFFSET (0xA1)
#define HI6551_LDO6_REG_ADJ_OFFSET (0xA2)
#define HI6551_LDO7_REG_ADJ_OFFSET (0xA3)
#define HI6551_LDO8_REG_ADJ_OFFSET (0xA4)
#define HI6551_LDO9_REG_ADJ_OFFSET (0xA5)
#define HI6551_LDO10_REG_ADJ_OFFSET (0xA6)
#define HI6551_LDO11_REG_ADJ_OFFSET (0xA7)
#define HI6551_LDO12_REG_ADJ_OFFSET (0xA8)
#define HI6551_LDO13_REG_ADJ_OFFSET (0xA9)
#define HI6551_LDO14_REG_ADJ_OFFSET (0xAA)
#define HI6551_LDO15_REG_ADJ_OFFSET (0xAB)
#define HI6551_LDO16_REG_ADJ_OFFSET (0xAC)
#define HI6551_LDO17_REG_ADJ_OFFSET (0xAD)
#define HI6551_LDO18_REG_ADJ_OFFSET (0xAE)
#define HI6551_LDO19_REG_ADJ_OFFSET (0xAF)
#define HI6551_LDO20_REG_ADJ_OFFSET (0xB0)
#define HI6551_LDO21_REG_ADJ_OFFSET (0xB1)
#define HI6551_LDO22_REG_ADJ_OFFSET (0xB2)
#define HI6551_LDO23_REG_ADJ_OFFSET (0xB3)
#define HI6551_LDO24_REG_ADJ_OFFSET (0xB4)
#define HI6551_PMUA_REG_ADJ_OFFSET (0xB5)
#define HI6551_LVS_ADJ1_OFFSET (0xB6)
#define HI6551_LVS_ADJ2_OFFSET (0xB7)
#define HI6551_LVS_ADJ3_OFFSET (0xB8)
#define HI6551_BOOST_ADJ0_OFFSET (0xB9)
#define HI6551_BOOST_ADJ1_OFFSET (0xBA)
#define HI6551_BOOST_ADJ2_OFFSET (0xBB)
#define HI6551_BOOST_ADJ3_OFFSET (0xBC)
#define HI6551_BOOST_ADJ4_OFFSET (0xBD)
#define HI6551_BOOST_ADJ5_OFFSET (0xBE)
#define HI6551_BOOST_ADJ6_OFFSET (0xBF)
#define HI6551_BOOST_ADJ7_OFFSET (0xC0)
#define HI6551_BOOST_ADJ8_OFFSET (0xC1)
#define HI6551_BOOST_ADJ9_OFFSET (0xC2)
#define HI6551_BOOST_ADJ10_OFFSET (0xC3)
#define HI6551_BOOST_ADJ11_OFFSET (0xC4)
#define HI6551_CLASSD_ADJ0_OFFSET (0xC5)
#define HI6551_CLASSD_ADJ1_OFFSET (0xC6)
#define HI6551_CLASSD_ADJ2_OFFSET (0xC7)
#define HI6551_BANDGAP_THSD_ADJ1_OFFSET (0xC8)
#define HI6551_BANDGAP_THSD_ADJ2_OFFSET (0xC9)
#define HI6551_DR_FLA_CTRL1_OFFSET (0xCA)
#define HI6551_DR_FLA_CTRL2_OFFSET (0xCB)
#define HI6551_FLASH_PERIOD_OFFSET (0xCC)
#define HI6551_FLASH_ON_OFFSET (0xCD)
#define HI6551_DR_MODE_SEL_OFFSET (0xCE)
#define HI6551_DR_BRE_CTRL_OFFSET (0xCF)
#define HI6551_DR1_TIM_CONF0_OFFSET (0xD0)
#define HI6551_DR1_TIM_CONF1_OFFSET (0xD1)
#define HI6551_DR1_ISET_OFFSET (0xD2)
#define HI6551_DR2_TIM_CONF0_OFFSET (0xD3)
#define HI6551_DR2_TIM_CONF1_OFFSET (0xD4)
#define HI6551_DR2_ISET_OFFSET (0xD5)
#define HI6551_DR_LED_CTRL_OFFSET (0xD6)
#define HI6551_DR_OUT_CTRL_OFFSET (0xD7)
#define HI6551_DR3_ISET_OFFSET (0xD8)
#define HI6551_DR3_START_DEL_OFFSET (0xD9)
#define HI6551_DR3_TIM_CONF0_OFFSET (0xDA)
#define HI6551_DR3_TIM_CONF1_OFFSET (0xDB)
#define HI6551_DR4_ISET_OFFSET (0xDC)
#define HI6551_DR4_START_DEL_OFFSET (0xDD)
#define HI6551_DR4_TIM_CONF0_OFFSET (0xDE)
#define HI6551_DR4_TIM_CONF1_OFFSET (0xDF)
#define HI6551_DR5_ISET_OFFSET (0xE0)
#define HI6551_DR5_START_DEL_OFFSET (0xE1)
#define HI6551_DR5_TIM_CONF0_OFFSET (0xE2)
#define HI6551_DR5_TIM_CONF1_OFFSET (0xE3)
#define HI6551_OTP_CTRL1_OFFSET (0xE4)
#define HI6551_OTP_CTRL2_OFFSET (0xE5)
#define HI6551_OTP_PDIN_OFFSET (0xE6)
#define HI6551_OTP_PDOB0_OFFSET (0xE7)
#define HI6551_OTP_PDOB1_OFFSET (0xE8)
#define HI6551_OTP_PDOB2_OFFSET (0xE9)
#define HI6551_OTP_PDOB3_OFFSET (0xEA)
#define HI6551_RTCCR_A0_OFFSET (0xEB)
#define HI6551_RTCCR_A1_OFFSET (0xEC)
#define HI6551_RTCCR_A2_OFFSET (0xED)
#define HI6551_RTCCR_A3_OFFSET (0xEE)
#define HI6551_RTCLR_A0_OFFSET (0xEF)
#define HI6551_RTCLR_A1_OFFSET (0xF0)
#define HI6551_RTCLR_A2_OFFSET (0xF1)
#define HI6551_RTCLR_A3_OFFSET (0xF2)
#define HI6551_RTCCTRL_A_OFFSET (0xF3)
#define HI6551_RTCMR_A_A0_OFFSET (0xF4)
#define HI6551_RTCMR_A_A1_OFFSET (0xF5)
#define HI6551_RTCMR_A_A2_OFFSET (0xF6)
#define HI6551_RTCMR_A_A3_OFFSET (0xF7)
#define HI6551_RTCMR_A_B0_OFFSET (0xF8)
#define HI6551_RTCMR_A_B1_OFFSET (0xF9)
#define HI6551_RTCMR_A_B2_OFFSET (0xFA)
#define HI6551_RTCMR_A_B3_OFFSET (0xFB)
#define HI6551_RTCMR_A_C0_OFFSET (0xFC)
#define HI6551_RTCMR_A_C1_OFFSET (0xFD)
#define HI6551_RTCMR_A_C2_OFFSET (0xFE)
#define HI6551_RTCMR_A_C3_OFFSET (0xFF)
#define HI6551_RTCMR_A_D0_OFFSET (0x100)
#define HI6551_RTCMR_A_D1_OFFSET (0x101)
#define HI6551_RTCMR_A_D2_OFFSET (0x102)
#define HI6551_RTCMR_A_D3_OFFSET (0x103)
#define HI6551_RTCMR_A_E0_OFFSET (0x104)
#define HI6551_RTCMR_A_E1_OFFSET (0x105)
#define HI6551_RTCMR_A_E2_OFFSET (0x106)
#define HI6551_RTCMR_A_E3_OFFSET (0x107)
#define HI6551_RTCMR_A_F0_OFFSET (0x108)
#define HI6551_RTCMR_A_F1_OFFSET (0x109)
#define HI6551_RTCMR_A_F2_OFFSET (0x10A)
#define HI6551_RTCMR_A_F3_OFFSET (0x10B)
#define HI6551_RTCCR_NA0_OFFSET (0x10C)
#define HI6551_RTCCR_NA1_OFFSET (0x10D)
#define HI6551_RTCCR_NA2_OFFSET (0x10E)
#define HI6551_RTCCR_NA3_OFFSET (0x10F)
#define HI6551_RTCLR_NA0_OFFSET (0x110)
#define HI6551_RTCLR_NA1_OFFSET (0x111)
#define HI6551_RTCLR_NA2_OFFSET (0x112)
#define HI6551_RTCLR_NA3_OFFSET (0x113)
#define HI6551_RTCCTRL_NA_OFFSET (0x114)
#define HI6551_VERSION_OFFSET (0x115)
#define HI6551_RESERVED0_OFFSET (0x116)
#define HI6551_RESERVED1_OFFSET (0x117)
#define HI6551_RESERVED2_OFFSET (0x118)
#define HI6551_RESERVED3_OFFSET (0x119)
#define HI6551_RESERVED4_OFFSET (0x11A)
#define HI6551_HTOL_MODE_OFFSET (0x11B)
#define HI6551_DAC_CTRL_OFFSET (0x11C)
#define HI6551_CHIP_SOFT_RST_OFFSET (0x11D)
#define HI6551_NP_REG_ADJ_OFFSET (0x11E)
#define HI6551_NP_REG_CHG_OFFSET (0x11F)
#define HI6551_NP_RSVED1_OFFSET (0x120)
#define HI6551_NP_RSVED2_OFFSET (0x121)
#define HI6551_NP_RSVED3_OFFSET (0x122)
#define HI6551_NP_RSVED4_OFFSET (0x123)
#define HI6551_RTC_ADJ1_OFFSET (0x124)
#define HI6551_RTC_ADJ2_OFFSET (0x125)
#define HI6551_CLJ_CTRL_REG_OFFSET (0x133)
#define HI6551_ECO_REFALSH_TIME_OFFSET (0x134)
#define HI6551_CL_OUT0_OFFSET (0x135)
#define HI6551_CL_OUT1_OFFSET (0x136)
#define HI6551_CL_OUT2_OFFSET (0x137)
#define HI6551_CL_OUT3_OFFSET (0x138)
#define HI6551_CL_IN0_OFFSET (0x139)
#define HI6551_CL_IN1_OFFSET (0x13A)
#define HI6551_CL_IN2_OFFSET (0x13B)
#define HI6551_CL_IN3_OFFSET (0x13C)
#define HI6551_CHG_TIMER0_OFFSET (0x13D)
#define HI6551_CHG_TIMER1_OFFSET (0x13E)
#define HI6551_CHG_TIMER2_OFFSET (0x13F)
#define HI6551_CHG_TIMER3_OFFSET (0x140)
#define HI6551_LOAD_TIMER0_OFFSET (0x141)
#define HI6551_LOAD_TIMER1_OFFSET (0x142)
#define HI6551_LOAD_TIMER2_OFFSET (0x143)
#define HI6551_LOAD_TIMER3_OFFSET (0x144)
#define HI6551_OFF_TIMER0_OFFSET (0x145)
#define HI6551_OFF_TIMER1_OFFSET (0x146)
#define HI6551_OFF_TIMER2_OFFSET (0x147)
#define HI6551_OFF_TIMER3_OFFSET (0x148)
#define HI6551_CL_INT0_OFFSET (0x149)
#define HI6551_CL_INT1_OFFSET (0x14A)
#define HI6551_CL_INT2_OFFSET (0x14B)
#define HI6551_CL_INT3_OFFSET (0x14C)
#define HI6551_V_INT0_OFFSET (0x14D)
#define HI6551_V_INT1_OFFSET (0x14E)
#define HI6551_OFFSET_CURRENT0_OFFSET (0x14F)
#define HI6551_OFFSET_CURRENT1_OFFSET (0x150)
#define HI6551_OFFSET_VOLTAGE0_OFFSET (0x151)
#define HI6551_OFFSET_VOLTAGE1_OFFSET (0x152)
#define HI6551_OCV_DATA1_OFFSET (0x153)
#define HI6551_OCV_DATA2_OFFSET (0x154)
#define HI6551_V_OUT0_PRE0_OFFSET (0x155)
#define HI6551_V_OUT1_PRE0_OFFSET (0x156)
#define HI6551_V_OUT0_PRE1_OFFSET (0x157)
#define HI6551_V_OUT1_PRE1_OFFSET (0x158)
#define HI6551_V_OUT0_PRE2_OFFSET (0x159)
#define HI6551_V_OUT1_PRE2_OFFSET (0x15A)
#define HI6551_V_OUT0_PRE3_OFFSET (0x15B)
#define HI6551_V_OUT1_PRE3_OFFSET (0x15C)
#define HI6551_V_OUT0_PRE4_OFFSET (0x15D)
#define HI6551_V_OUT1_PRE4_OFFSET (0x15E)
#define HI6551_V_OUT0_PRE5_OFFSET (0x15F)
#define HI6551_V_OUT1_PRE5_OFFSET (0x160)
#define HI6551_V_OUT0_PRE6_OFFSET (0x161)
#define HI6551_V_OUT1_PRE6_OFFSET (0x162)
#define HI6551_V_OUT0_PRE7_OFFSET (0x163)
#define HI6551_V_OUT1_PRE7_OFFSET (0x164)
#define HI6551_V_OUT0_PRE8_OFFSET (0x165)
#define HI6551_V_OUT1_PRE8_OFFSET (0x166)
#define HI6551_V_OUT0_PRE9_OFFSET (0x167)
#define HI6551_V_OUT1_PRE9_OFFSET (0x168)
#define HI6551_V_OUT0_PRE10_OFFSET (0x169)
#define HI6551_V_OUT1_PRE10_OFFSET (0x16A)
#define HI6551_V_OUT0_PRE11_OFFSET (0x16B)
#define HI6551_V_OUT1_PRE11_OFFSET (0x16C)
#define HI6551_V_OUT0_PRE12_OFFSET (0x16D)
#define HI6551_V_OUT1_PRE12_OFFSET (0x16E)
#define HI6551_V_OUT0_PRE13_OFFSET (0x16F)
#define HI6551_V_OUT1_PRE13_OFFSET (0x170)
#define HI6551_V_OUT0_PRE14_OFFSET (0x171)
#define HI6551_V_OUT1_PRE14_OFFSET (0x172)
#define HI6551_V_OUT0_PRE15_OFFSET (0x173)
#define HI6551_V_OUT1_PRE15_OFFSET (0x174)
#define HI6551_V_OUT0_PRE16_OFFSET (0x175)
#define HI6551_V_OUT1_PRE16_OFFSET (0x176)
#define HI6551_V_OUT0_PRE17_OFFSET (0x177)
#define HI6551_V_OUT1_PRE17_OFFSET (0x178)
#define HI6551_V_OUT0_PRE18_OFFSET (0x179)
#define HI6551_V_OUT1_PRE18_OFFSET (0x17A)
#define HI6551_V_OUT0_PRE19_OFFSET (0x17B)
#define HI6551_V_OUT1_PRE19_OFFSET (0x17C)
#define HI6551_CURRENT0_PRE0_OFFSET (0x17D)
#define HI6551_CURRENT1_PRE0_OFFSET (0x17E)
#define HI6551_CURRENT0_PRE1_OFFSET (0x17F)
#define HI6551_CURRENT1_PRE1_OFFSET (0x180)
#define HI6551_CURRENT0_PRE2_OFFSET (0x181)
#define HI6551_CURRENT1_PRE2_OFFSET (0x182)
#define HI6551_CURRENT0_PRE3_OFFSET (0x183)
#define HI6551_CURRENT1_PRE3_OFFSET (0x184)
#define HI6551_CURRENT0_PRE4_OFFSET (0x185)
#define HI6551_CURRENT1_PRE4_OFFSET (0x186)
#define HI6551_CURRENT0_PRE5_OFFSET (0x187)
#define HI6551_CURRENT1_PRE5_OFFSET (0x188)
#define HI6551_CURRENT0_PRE6_OFFSET (0x189)
#define HI6551_CURRENT1_PRE6_OFFSET (0x18A)
#define HI6551_CURRENT0_PRE7_OFFSET (0x18B)
#define HI6551_CURRENT1_PRE7_OFFSET (0x18C)
#define HI6551_CURRENT0_PRE8_OFFSET (0x18D)
#define HI6551_CURRENT1_PRE8_OFFSET (0x18E)
#define HI6551_CURRENT0_PRE9_OFFSET (0x18F)
#define HI6551_CURRENT1_PRE9_OFFSET (0x190)
#define HI6551_CURRENT0_PRE10_OFFSET (0x191)
#define HI6551_CURRENT1_PRE10_OFFSET (0x192)
#define HI6551_CURRENT0_PRE11_OFFSET (0x193)
#define HI6551_CURRENT1_PRE11_OFFSET (0x194)
#define HI6551_CURRENT0_PRE12_OFFSET (0x195)
#define HI6551_CURRENT1_PRE12_OFFSET (0x196)
#define HI6551_CURRENT0_PRE13_OFFSET (0x197)
#define HI6551_CURRENT1_PRE13_OFFSET (0x198)
#define HI6551_CURRENT0_PRE14_OFFSET (0x199)
#define HI6551_CURRENT1_PRE14_OFFSET (0x19A)
#define HI6551_CURRENT0_PRE15_OFFSET (0x19B)
#define HI6551_CURRENT1_PRE15_OFFSET (0x19C)
#define HI6551_CURRENT0_PRE16_OFFSET (0x19D)
#define HI6551_CURRENT1_PRE16_OFFSET (0x19E)
#define HI6551_CURRENT0_PRE17_OFFSET (0x19F)
#define HI6551_CURRENT1_PRE17_OFFSET (0x1A0)
#define HI6551_CURRENT0_PRE18_OFFSET (0x1A1)
#define HI6551_CURRENT1_PRE18_OFFSET (0x1A2)
#define HI6551_CURRENT0_PRE19_OFFSET (0x1A3)
#define HI6551_CURRENT1_PRE19_OFFSET (0x1A4)
#define HI6551_CLJ_DEBUG_OFFSET (0x1A5)
#define HI6551_STATE_TEST_OFFSET (0x1A6)
#define HI6551_CLJ_RESERVED1_OFFSET (0x1A7)
#define HI6551_CLJ_RESERVED2_OFFSET (0x1A8)
#define HI6551_CLJ_RESERVED3_OFFSET (0x1A9)
#define HI6551_CLJ_RESERVED4_OFFSET (0x1AA)
#define HI6551_CLJ_RESERVED5_OFFSET (0x1AB)
#define HI6551_CLJ_RESERVED6_OFFSET (0x1AC)
#define HI6551_CLJ_RESERVED7_OFFSET (0x1AD)
#define HI6551_CLJ_RESERVED8_OFFSET (0x1AE)
typedef union
{
    struct
    {
        unsigned int otmp_d1r : 1;
        unsigned int otmp150_d1r : 1;
        unsigned int osc_state : 1;
        unsigned int vsys_uv_d2f : 1;
        unsigned int vsys_6p0_d200ur : 1;
        unsigned int pwron_d20r : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_STATUS1_T;
typedef union
{
    struct
    {
        unsigned int alarm_on_a : 1;
        unsigned int alarm_on_b : 1;
        unsigned int alarm_on_c : 1;
        unsigned int alarm_on_d : 1;
        unsigned int alarm_on_e : 1;
        unsigned int vbus_det_0p9_d3r : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_STATUS2_T;
typedef union
{
    struct
    {
        unsigned int otmp_d1r : 1;
        unsigned int vsys_2p5_f : 1;
        unsigned int vsys_uv_d2f : 1;
        unsigned int vsys_6p0_d200ur : 1;
        unsigned int pwron_d4sr : 1;
        unsigned int pwron_d20f : 1;
        unsigned int pwron_d20r : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_IRQ1_T;
typedef union
{
    struct
    {
        unsigned int ocp_scp_r : 1;
        unsigned int vbus_det_1p375_d90ur : 1;
        unsigned int vbus_det_0p9_d3f : 1;
        unsigned int vbus_det_0p9_d3r : 1;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_IRQ2_T;
typedef union
{
    struct
    {
        unsigned int alarm_on_a : 1;
        unsigned int alarm_on_b : 1;
        unsigned int alarm_on_c : 1;
        unsigned int alarm_on_d : 1;
        unsigned int alarm_on_e : 1;
        unsigned int alarm_on_f : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_IRQ3_T;
typedef union
{
    struct
    {
        unsigned int sim0_hpd_out_f : 1;
        unsigned int sim0_hpd_out_r : 1;
        unsigned int sim0_hpd_in_f : 1;
        unsigned int sim0_hpd_in_r : 1;
        unsigned int sim1_hpd_out_f : 1;
        unsigned int sim1_hpd_out_r : 1;
        unsigned int sim1_hpd_in_f : 1;
        unsigned int sim1_hpd_in_r : 1;
    } bits;
    unsigned int u32;
}HI6551_IRQ4_T;
typedef union
{
    struct
    {
        unsigned int cl_int_i : 1;
        unsigned int cl_out_i : 1;
        unsigned int cl_in_i : 1;
        unsigned int chg_timer_i : 1;
        unsigned int load_timer_i : 1;
        unsigned int vbat_int_i : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_COUL_IRQ_T;
typedef union
{
    struct
    {
        unsigned int otmp_d1r_mk : 1;
        unsigned int vsys_2p5_f_mk : 1;
        unsigned int vsys_uv_d2f_mk : 1;
        unsigned int vsys_6p0_d200ur_mk : 1;
        unsigned int pwron_d4sr_mk : 1;
        unsigned int pwron_d20f_mk : 1;
        unsigned int pwron_d20r_mk : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_IRQ1_MASK_T;
typedef union
{
    struct
    {
        unsigned int ocp_scp_r_mk : 1;
        unsigned int vbus_det_1p375_mk : 1;
        unsigned int vbus_det_0p9_d3f_mk : 1;
        unsigned int vbus_det_0p9_d3r_mk : 1;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_IRQ2_MASK_T;
typedef union
{
    struct
    {
        unsigned int alarm_on_a_mk : 1;
        unsigned int alarm_on_b_mk : 1;
        unsigned int alarm_on_c_mk : 1;
        unsigned int alarm_on_d_mk : 1;
        unsigned int alarm_on_e_mk : 1;
        unsigned int alarm_on_f_mk : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_IRQ3_MASK_T;
typedef union
{
    struct
    {
        unsigned int sim0_hpd_out_f_mk : 1;
        unsigned int sim0_hpd_out_r_mk : 1;
        unsigned int sim0_hpd_in_f_mk : 1;
        unsigned int sim0_hpd_in_r_mk : 1;
        unsigned int sim1_hpd_out_f_mk : 1;
        unsigned int sim1_hpd_out_r_mk : 1;
        unsigned int sim1_hpd_in_f_mk : 1;
        unsigned int sim1_in_hpd_r_mk : 1;
    } bits;
    unsigned int u32;
}HI6551_IRQ4_MASK_T;
typedef union
{
    struct
    {
        unsigned int cl_int_i_mk : 1;
        unsigned int cl_out_i_mk : 1;
        unsigned int cl_in_i_mk : 1;
        unsigned int chg_timer_i_mk : 1;
        unsigned int load_timer_i_mk : 1;
        unsigned int vbat_int_i_mk : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_COUL_IRQ_MASK_T;
typedef union
{
    struct
    {
        unsigned int scp_buck0_1 : 1;
        unsigned int scp_buck2 : 1;
        unsigned int scp_buck3 : 1;
        unsigned int scp_buck4 : 1;
        unsigned int scp_buck5 : 1;
        unsigned int scp_buck6 : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_SCP_RECORD1_T;
typedef union
{
    struct
    {
        unsigned int ocp_buck0_1 : 1;
        unsigned int ocp_buck2 : 1;
        unsigned int ocp_buck3 : 1;
        unsigned int ocp_buck4 : 1;
        unsigned int ocp_buck5 : 1;
        unsigned int ocp_buck6 : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_OCP_RECORD1_T;
typedef union
{
    struct
    {
        unsigned int ocp_ldo1 : 1;
        unsigned int ocp_ldo2 : 1;
        unsigned int ocp_ldo3 : 1;
        unsigned int ocp_ldo4 : 1;
        unsigned int ocp_ldo5 : 1;
        unsigned int ocp_ldo6 : 1;
        unsigned int ocp_ldo7 : 1;
        unsigned int ocp_ldo8 : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_RECORD2_T;
typedef union
{
    struct
    {
        unsigned int ocp_ldo9 : 1;
        unsigned int ocp_ldo10 : 1;
        unsigned int ocp_ldo11 : 1;
        unsigned int ocp_ldo12 : 1;
        unsigned int ocp_ldo13 : 1;
        unsigned int ocp_ldo14 : 1;
        unsigned int ocp_ldo15 : 1;
        unsigned int ocp_ldo16 : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_RECORD3_T;
typedef union
{
    struct
    {
        unsigned int ocp_ldo17 : 1;
        unsigned int ocp_ldo18 : 1;
        unsigned int ocp_ldo19 : 1;
        unsigned int ocp_ldo20 : 1;
        unsigned int ocp_ldo21 : 1;
        unsigned int ocp_ldo22 : 1;
        unsigned int ocp_ldo23 : 1;
        unsigned int ocp_ldo24 : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_RECORD4_T;
typedef union
{
    struct
    {
        unsigned int ocp_lvs2 : 1;
        unsigned int ocp_lvs3 : 1;
        unsigned int ocp_lvs4 : 1;
        unsigned int ocp_lvs5 : 1;
        unsigned int ocp_lvs7 : 1;
        unsigned int ocp_lvs6 : 1;
        unsigned int ocp_lvs8 : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_RECORD5_T;
typedef union
{
    struct
    {
        unsigned int ocp_lvs9 : 1;
        unsigned int ocp_lvs10 : 1;
        unsigned int reserved : 6;
    } bits;
    unsigned int u32;
}HI6551_OCP_RECORD6_T;
typedef union
{
    struct
    {
        unsigned int ocp_boost : 1;
        unsigned int scp_boost : 1;
        unsigned int ovp_boost : 1;
        unsigned int uvp_boost : 1;
        unsigned int ocp_classd : 1;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_OCP_RECORD7_T;
typedef union
{
    struct
    {
        unsigned int np_otmp_d1r : 1;
        unsigned int np_hreset_n_f : 1;
        unsigned int np_vsys_2p5_f : 1;
        unsigned int np_vsys_uv_d2f : 1;
        unsigned int np_vsys_6p0_d200ur : 1;
        unsigned int np_pwron_d4sr : 1;
        unsigned int np_pwron_d20f : 1;
        unsigned int np_pwron_d20r : 1;
    } bits;
    unsigned int u32;
}HI6551_NP_IRQ1_RECORD_T;
typedef union
{
    struct
    {
        unsigned int np_vbus_det_0p9_d150r : 1;
        unsigned int np_vbus_det_1p375 : 1;
        unsigned int np_vbus_det_0p9_d3f : 1;
        unsigned int np_vbus_det_0p9_d3r : 1;
        unsigned int np_pwron_restart : 1;
        unsigned int np_pwron_dnsr : 1;
        unsigned int np_pwron_d500r : 1;
        unsigned int np_otmp150_d1r : 1;
    } bits;
    unsigned int u32;
}HI6551_NP_IRQ2_RECORD_T;
typedef union
{
    struct
    {
        unsigned int np_alarm_on_a : 1;
        unsigned int np_alarm_on_b : 1;
        unsigned int np_alarm_on_c : 1;
        unsigned int np_alarm_on_d : 1;
        unsigned int np_alarm_on_e : 1;
        unsigned int np_alarm_on_f : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_NP_IRQ3_RECORD_T;
typedef union
{
    struct
    {
        unsigned int np_scp_buck0_1 : 1;
        unsigned int np_scp_buck2 : 1;
        unsigned int np_scp_buck3 : 1;
        unsigned int np_scp_buck4 : 1;
        unsigned int np_scp_buck5 : 1;
        unsigned int np_scp_buck6 : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_NP_SCP_RECORD1_T;
typedef union
{
    struct
    {
        unsigned int np_ocp_buck0 : 1;
        unsigned int np_ocp_buck1 : 1;
        unsigned int np_ocp_buck0_1 : 1;
        unsigned int np_ocp_buck2 : 1;
        unsigned int np_ocp_buck3 : 1;
        unsigned int np_ocp_buck4 : 1;
        unsigned int np_ocp_buck5 : 1;
        unsigned int np_ocp_buck6 : 1;
    } bits;
    unsigned int u32;
}HI6551_NP_OCP_RECORD1_T;
typedef union
{
    struct
    {
        unsigned int np_ocp_ldo1 : 1;
        unsigned int np_ocp_ldo2 : 1;
        unsigned int np_ocp_ldo3 : 1;
        unsigned int np_ocp_ldo4 : 1;
        unsigned int np_ocp_ldo5 : 1;
        unsigned int np_ocp_ldo6 : 1;
        unsigned int np_ocp_ldo7 : 1;
        unsigned int np_ocp_ldo8 : 1;
    } bits;
    unsigned int u32;
}HI6551_NP_OCP_RECORD2_T;
typedef union
{
    struct
    {
        unsigned int np_ocp_ldo9 : 1;
        unsigned int np_ocp_ldo10 : 1;
        unsigned int np_ocp_ldo11 : 1;
        unsigned int np_ocp_ldo12 : 1;
        unsigned int np_ocp_ldo13 : 1;
        unsigned int np_ocp_ldo14 : 1;
        unsigned int np_ocp_ldo15 : 1;
        unsigned int np_ocp_ldo16 : 1;
    } bits;
    unsigned int u32;
}HI6551_NP_OCP_RECORD3_T;
typedef union
{
    struct
    {
        unsigned int np_ocp_ldo17 : 1;
        unsigned int np_ocp_ldo18 : 1;
        unsigned int np_ocp_ldo19 : 1;
        unsigned int np_ocp_ldo20 : 1;
        unsigned int np_ocp_ldo21 : 1;
        unsigned int np_ocp_ldo22 : 1;
        unsigned int np_ocp_ldo23 : 1;
        unsigned int np_ocp_ldo24 : 1;
    } bits;
    unsigned int u32;
}HI6551_NP_OCP_RECORD4_T;
typedef union
{
    struct
    {
        unsigned int np_ocp_lvs2 : 1;
        unsigned int np_ocp_lvs3 : 1;
        unsigned int np_ocp_lvs4 : 1;
        unsigned int np_ocp_lvs5 : 1;
        unsigned int np_ocp_lvs7 : 1;
        unsigned int np_ocp_lvs6 : 1;
        unsigned int np_ocp_lvs8 : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_NP_OCP_RECORD5_T;
typedef union
{
    struct
    {
        unsigned int np_ocp_lvs9 : 1;
        unsigned int np_ocp_lvs10 : 1;
        unsigned int reserved : 6;
    } bits;
    unsigned int u32;
}HI6551_NP_OCP_RECORD6_T;
typedef union
{
    struct
    {
        unsigned int np_ocp_boost : 1;
        unsigned int np_scp_boost : 1;
        unsigned int np_ovp_boost : 1;
        unsigned int np_uvp_boost : 1;
        unsigned int np_ocp_classd : 1;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_NP_OCP_RECORD7_T;
typedef union
{
    struct
    {
        unsigned int ocp_ldo_deb_sel : 2;
        unsigned int en_ldo_ocp_deb : 1;
        unsigned int ocp_buck_deb_sel : 2;
        unsigned int en_buck_ocp_deb : 1;
        unsigned int en_buck_scp_deb : 1;
        unsigned int en_bst_ocp_deb : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_DEB_T;
typedef union
{
    struct
    {
        unsigned int vbus_det_0p9_ctrl : 1;
        unsigned int vsys_2p5_ctrl : 1;
        unsigned int vsys_uv_ctrl : 1;
        unsigned int vsys_6p0_ctrl : 1;
        unsigned int otmp150_ctrl : 1;
        unsigned int en_32kc : 1;
        unsigned int en_32kb : 1;
        unsigned int en_32ka : 1;
    } bits;
    unsigned int u32;
}HI6551_ONOFF8_T;
typedef union
{
    struct
    {
        unsigned int ocp_scp_ctrl_buck0_1 : 1;
        unsigned int ocp_scp_off_buck0_1 : 1;
        unsigned int ocp_scp_ctrl_buck2 : 1;
        unsigned int ocp_scp_off_buck2 : 1;
        unsigned int ocp_scp_ctrl_buck3 : 1;
        unsigned int ocp_scp_off_buck3 : 1;
        unsigned int ocp_scp_ctrl_buck4 : 1;
        unsigned int ocp_scp_off_buck4 : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_SCP_MOD_CTRL1_T;
typedef union
{
    struct
    {
        unsigned int ocp_scp_ctrl_buck5 : 1;
        unsigned int ocp_scp_off_buck5 : 1;
        unsigned int ocp_scp_ctrl_buck6 : 1;
        unsigned int ocp_scp_off_buck6 : 1;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_OCP_SCP_MOD_CTRL2_T;
typedef union
{
    struct
    {
        unsigned int ocp_ctrl_ldo1 : 1;
        unsigned int ocp_off_ldo1 : 1;
        unsigned int ocp_ctrl_ldo2 : 1;
        unsigned int ocp_off_ldo2 : 1;
        unsigned int ocp_ctrl_ldo3 : 1;
        unsigned int ocp_off_ldo3 : 1;
        unsigned int ocp_ctrl_ldo4 : 1;
        unsigned int ocp_off_ldo4 : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_MOD_CTRL3_T;
typedef union
{
    struct
    {
        unsigned int ocp_ctrl_ldo5 : 1;
        unsigned int ocp_off_ldo5 : 1;
        unsigned int ocp_ctrl_ldo6 : 1;
        unsigned int ocp_off_ldo6 : 1;
        unsigned int ocp_ctrl_ldo7 : 1;
        unsigned int ocp_off_ldo7 : 1;
        unsigned int ocp_ctrl_ldo8 : 1;
        unsigned int ocp_off_ldo8 : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_MOD_CTRL4_T;
typedef union
{
    struct
    {
        unsigned int ocp_ctrl_ldo9 : 1;
        unsigned int ocp_off_ldo9 : 1;
        unsigned int ocp_ctrl_ldo10 : 1;
        unsigned int ocp_off_ldo10 : 1;
        unsigned int ocp_ctrl_ldo11 : 1;
        unsigned int ocp_off_ldo11 : 1;
        unsigned int ocp_ctrl_ldo12 : 1;
        unsigned int ocp_off_ldo12 : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_MOD_CTRL5_T;
typedef union
{
    struct
    {
        unsigned int ocp_ctrl_ldo13 : 1;
        unsigned int ocp_off_ldo13 : 1;
        unsigned int ocp_ctrl_ldo14 : 1;
        unsigned int ocp_off_ldo14 : 1;
        unsigned int ocp_ctrl_ldo15 : 1;
        unsigned int ocp_off_ldo15 : 1;
        unsigned int ocp_ctrl_ldo16 : 1;
        unsigned int ocp_off_ldo16 : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_MOD_CTRL6_T;
typedef union
{
    struct
    {
        unsigned int ocp_ctrl_ldo17 : 1;
        unsigned int ocp_off_ldo17 : 1;
        unsigned int ocp_ctrl_ldo18 : 1;
        unsigned int ocp_off_ldo18 : 1;
        unsigned int ocp_ctrl_ldo19 : 1;
        unsigned int ocp_off_ldo19 : 1;
        unsigned int ocp_ctrl_ldo20 : 1;
        unsigned int ocp_off_ldo20 : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_MOD_CTRL7_T;
typedef union
{
    struct
    {
        unsigned int ocp_ctrl_ldo21 : 1;
        unsigned int ocp_off_ldo21 : 1;
        unsigned int ocp_ctrl_ldo22 : 1;
        unsigned int ocp_off_ldo22 : 1;
        unsigned int ocp_ctrl_ldo23 : 1;
        unsigned int ocp_off_ldo23 : 1;
        unsigned int ocp_ctrl_ldo24 : 1;
        unsigned int ocp_off_ldo24 : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_MOD_CTRL8_T;
typedef union
{
    struct
    {
        unsigned int ocp_ctrl_lvs2 : 1;
        unsigned int ocp_off_lvs2 : 1;
        unsigned int ocp_ctrl_lvs3 : 1;
        unsigned int ocp_off_lvs3 : 1;
        unsigned int ocp_ctrl_lvs4 : 1;
        unsigned int ocp_off_lvs4 : 1;
        unsigned int ocp_ctrl_lvs5 : 1;
        unsigned int ocp_off_lvs5 : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_MOD_CTRL9_T;
typedef union
{
    struct
    {
        unsigned int ocp_ctrl_lvs7 : 1;
        unsigned int ocp_off_lvs7 : 1;
        unsigned int ocp_ctrl_lvs6 : 1;
        unsigned int ocp_off_lvs6 : 1;
        unsigned int ocp_ctrl_lvs8 : 1;
        unsigned int ocp_off_lvs8 : 1;
        unsigned int ocp_ctrl_lvs9 : 1;
        unsigned int ocp_off_lvs9 : 1;
    } bits;
    unsigned int u32;
}HI6551_OCP_MOD_CTRL10_T;
typedef union
{
    struct
    {
        unsigned int ocp_ctrl_lvs10 : 1;
        unsigned int ocp_off_lvs10 : 1;
        unsigned int ocp_ctrl_bst : 1;
        unsigned int ocp_off_bst : 1;
        unsigned int ocp_ctrl_classd : 1;
        unsigned int ocp_off_classd : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_OCP_MOD_CTRL11_T;
typedef union
{
    struct
    {
        unsigned int en_buck0_int : 1;
        unsigned int en_buck1_int : 1;
        unsigned int en_buck2_int : 1;
        unsigned int en_buck3_int : 1;
        unsigned int en_buck4_int : 1;
        unsigned int en_buck5_int : 1;
        unsigned int en_buck6_int : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_ENABLE1_T;
typedef union
{
    struct
    {
        unsigned int dis_buck0_int : 1;
        unsigned int dis_buck1_int : 1;
        unsigned int dis_buck2_int : 1;
        unsigned int dis_buck3_int : 1;
        unsigned int dis_buck4_int : 1;
        unsigned int dis_buck5_int : 1;
        unsigned int dis_buck6_int : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_DISABLE1_T;
typedef union
{
    struct
    {
        unsigned int st_buck0_int : 1;
        unsigned int st_buck1_int : 1;
        unsigned int st_buck2_int : 1;
        unsigned int st_buck3_int : 1;
        unsigned int st_buck4_int : 1;
        unsigned int st_buck5_int : 1;
        unsigned int st_buck6_int : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_ONOFF_STATUS1_T;
typedef union
{
    struct
    {
        unsigned int en_ldo1_int : 1;
        unsigned int en_ldo2_int : 1;
        unsigned int en_ldo3_buck_int : 1;
        unsigned int en_ldo4_int : 1;
        unsigned int en_ldo5_int : 1;
        unsigned int en_ldo6_int : 1;
        unsigned int en_ldo7_int : 1;
        unsigned int en_ldo8_int : 1;
    } bits;
    unsigned int u32;
}HI6551_ENABLE2_T;
typedef union
{
    struct
    {
        unsigned int dis_ldo1_int : 1;
        unsigned int dis_ldo2_int : 1;
        unsigned int dis_ldo3_buck_int : 1;
        unsigned int dis_ldo4_int : 1;
        unsigned int dis_ldo5_int : 1;
        unsigned int dis_ldo6_int : 1;
        unsigned int dis_ldo7_int : 1;
        unsigned int dis_ldo8_int : 1;
    } bits;
    unsigned int u32;
}HI6551_DISABLE2_T;
typedef union
{
    struct
    {
        unsigned int st_ldo1_int : 1;
        unsigned int st_ldo2_int : 1;
        unsigned int st_ldo3_buck_int : 1;
        unsigned int st_ldo4_int : 1;
        unsigned int st_ldo5_int : 1;
        unsigned int st_ldo6_int : 1;
        unsigned int st_ldo7_int : 1;
        unsigned int st_ldo8_int : 1;
    } bits;
    unsigned int u32;
}HI6551_ONOFF_STATUS2_T;
typedef union
{
    struct
    {
        unsigned int en_ldo10_int : 1;
        unsigned int en_ldo12_int : 1;
        unsigned int en_ldo13_int : 1;
        unsigned int en_ldo14_int : 1;
        unsigned int en_ldo15_int : 1;
        unsigned int en_ldo16_int : 1;
        unsigned int en_ldo17_int : 1;
        unsigned int en_ldo18_int : 1;
    } bits;
    unsigned int u32;
}HI6551_ENABLE3_T;
typedef union
{
    struct
    {
        unsigned int dis_ldo10_int : 1;
        unsigned int dis_ldo12_int : 1;
        unsigned int dis_ldo13_int : 1;
        unsigned int dis_ldo14_int : 1;
        unsigned int dis_ldo15_int : 1;
        unsigned int dis_ldo16_int : 1;
        unsigned int dis_ldo17_int : 1;
        unsigned int dis_ldo18_int : 1;
    } bits;
    unsigned int u32;
}HI6551_DISABLE3_T;
typedef union
{
    struct
    {
        unsigned int st_ldo10_int : 1;
        unsigned int st_ldo12_int : 1;
        unsigned int st_ldo13_int : 1;
        unsigned int st_ldo14_int : 1;
        unsigned int st_ldo15_int : 1;
        unsigned int st_ldo16_int : 1;
        unsigned int st_ldo17_int : 1;
        unsigned int st_ldo18_int : 1;
    } bits;
    unsigned int u32;
}HI6551_ONOFF_STATUS3_T;
typedef union
{
    struct
    {
        unsigned int en_ldo19_int : 1;
        unsigned int en_ldo20_int : 1;
        unsigned int en_ldo21_int : 1;
        unsigned int en_ldo22_int : 1;
        unsigned int en_ldo23_int : 1;
        unsigned int en_ldo24_int : 1;
        unsigned int en_ldo3_batt_int : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_ENABLE4_T;
typedef union
{
    struct
    {
        unsigned int dis_ldo19_int : 1;
        unsigned int dis_ldo20_int : 1;
        unsigned int dis_ldo21_int : 1;
        unsigned int dis_ldo22_int : 1;
        unsigned int dis_ldo23_int : 1;
        unsigned int dis_ldo24_int : 1;
        unsigned int dis_ldo3_batt_int : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_DISABLE4_T;
typedef union
{
    struct
    {
        unsigned int st_ldo19_int : 1;
        unsigned int st_ldo20_int : 1;
        unsigned int st_ldo21_int : 1;
        unsigned int st_ldo22_int : 1;
        unsigned int st_ldo23_int : 1;
        unsigned int st_ldo24_int : 1;
        unsigned int st_ldo3_batt_int : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_ONOFF_STATUS4_T;
typedef union
{
    struct
    {
        unsigned int en_lvs2_int : 1;
        unsigned int en_lvs3_int : 1;
        unsigned int en_lvs4_int : 1;
        unsigned int en_lvs5_int : 1;
        unsigned int en_lvs7_int : 1;
        unsigned int en_lvs6_int : 1;
        unsigned int en_lvs8_int : 1;
        unsigned int en_lvs9_int : 1;
    } bits;
    unsigned int u32;
}HI6551_ENABLE5_T;
typedef union
{
    struct
    {
        unsigned int dis_lvs2_int : 1;
        unsigned int dis_lvs3_int : 1;
        unsigned int dis_lvs4_int : 1;
        unsigned int dis_lvs5_int : 1;
        unsigned int dis_lvs7_int : 1;
        unsigned int dis_lvs6_int : 1;
        unsigned int dis_lvs8_int : 1;
        unsigned int dis_lvs9_int : 1;
    } bits;
    unsigned int u32;
}HI6551_DISABLE5_T;
typedef union
{
    struct
    {
        unsigned int st_lvs2_int : 1;
        unsigned int st_lvs3_int : 1;
        unsigned int st_lvs4_int : 1;
        unsigned int st_lvs5_int : 1;
        unsigned int st_lvs7_int : 1;
        unsigned int st_lvs6_int : 1;
        unsigned int st_lvs8_int : 1;
        unsigned int st_lvs9_int : 1;
    } bits;
    unsigned int u32;
}HI6551_ONOFF_STATUS5_T;
typedef union
{
    struct
    {
        unsigned int en_lvs10_int : 1;
        unsigned int reserved : 7;
    } bits;
    unsigned int u32;
}HI6551_ENABLE6_T;
typedef union
{
    struct
    {
        unsigned int dis_lvs10_int : 1;
        unsigned int reserved : 7;
    } bits;
    unsigned int u32;
}HI6551_DISABLE6_T;
typedef union
{
    struct
    {
        unsigned int st_lvs10_int : 1;
        unsigned int reserved : 7;
    } bits;
    unsigned int u32;
}HI6551_ONOFF_STATUS6_T;
typedef union
{
    struct
    {
        unsigned int en_cp2_int : 1;
        unsigned int cp2_always_on_int : 1;
        unsigned int reserved : 6;
    } bits;
    unsigned int u32;
}HI6551_ENABLE7_T;
typedef union
{
    struct
    {
        unsigned int en_ldo9_int : 1;
        unsigned int sim0_en_int : 1;
        unsigned int en_ldo11_int : 1;
        unsigned int sim1_en_int : 1;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_SIMCARD_EN_T;
typedef union
{
    struct
    {
        unsigned int simcard0_resv : 2;
        unsigned int simcard0_bbrp : 1;
        unsigned int simcard0_simrp : 1;
        unsigned int simcard0_200ns_en : 1;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_SIM0_CFG_T;
typedef union
{
    struct
    {
        unsigned int simcard1_resv : 2;
        unsigned int simcard1_bbrp : 1;
        unsigned int simcard1_simrp : 1;
        unsigned int simcard1_200ns_en : 1;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_SIM1_CFG_T;
typedef union
{
    struct
    {
        unsigned int simcard_deb_sel : 5;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_SIM_DEB_T;
typedef union
{
    struct
    {
        unsigned int en_eco_buck3_int : 1;
        unsigned int en_eco_buck4_int : 1;
        unsigned int en_eco_buck5_int : 1;
        unsigned int en_eco_buck6_int : 1;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_ECO_MOD_CFG1_T;
typedef union
{
    struct
    {
        unsigned int en_eco_ldo3_int : 1;
        unsigned int en_eco_ldo7_int : 1;
        unsigned int en_eco_ldo9_int : 1;
        unsigned int en_eco_ldo10_int : 1;
        unsigned int en_eco_ldo11_int : 1;
        unsigned int en_eco_ldo12_int : 1;
        unsigned int en_eco_ldo22_int : 1;
        unsigned int en_eco_ldo24_int : 1;
    } bits;
    unsigned int u32;
}HI6551_ECO_MOD_CFG2_T;
typedef union
{
    struct
    {
        unsigned int force_eco_buck3_int : 1;
        unsigned int force_eco_buck4_int : 1;
        unsigned int force_eco_buck5_int : 1;
        unsigned int force_eco_buck6_int : 1;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_ECO_MOD_CFG3_T;
typedef union
{
    struct
    {
        unsigned int force_eco_ldo3_int : 1;
        unsigned int force_eco_ldo7_int : 1;
        unsigned int force_eco_ldo9_int : 1;
        unsigned int force_eco_ldo10_int : 1;
        unsigned int force_eco_ldo11_int : 1;
        unsigned int force_eco_ldo12_int : 1;
        unsigned int force_eco_ldo22_int : 1;
        unsigned int force_eco_ldo24_int : 1;
    } bits;
    unsigned int u32;
}HI6551_ECO_MOD_CFG4_T;
typedef union
{
    struct
    {
        unsigned int eco_pwrsel : 2;
        unsigned int peri_en_ctrl_ldo12 : 1;
        unsigned int peri_en_ctrl_lvs9 : 1;
        unsigned int peri_en_ctrl_ldo8 : 1;
        unsigned int peri_en_ctrl_buck3 : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_ENB3_ECO_MARK_T;
typedef union
{
    struct
    {
        unsigned int buck01_osc_d : 4;
        unsigned int buck0_pfm_vth_sel : 1;
        unsigned int buck0_sc_sel : 2;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG1_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck01_mod_sel : 2;
        unsigned int buck0_bt_sel : 1;
        unsigned int buck0_pd_sel : 1;
        unsigned int buck0_burst_drv_ctr : 1;
        unsigned int buck01_ocp_mod_sel : 1;
        unsigned int buck01_ocp_clamp_sel : 1;
        unsigned int buck01_ocp_shied_sel : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG2_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck0_vc_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck01_auto_pfm_ctr : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG3_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck0_isc_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck0_ipk_sel : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG4_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck01_dt_sel : 2;
        unsigned int buck01_pg_dt_sel : 1;
        unsigned int buck01_ng_dt_sel : 1;
        unsigned int buck01_sft_sel : 1;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG5_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck01_pg_p_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck01_pg_n_sel : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG6_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck01_ng_p_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck01_ng_n_sel : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG7_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck0_ccm_sel : 2;
        unsigned int buck1_ccm_sel : 2;
        unsigned int buck0_ccc_sel : 2;
        unsigned int buck0_cs_sel : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG8_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck0_dmd_sel : 2;
        unsigned int buck01_ton_off : 1;
        unsigned int buck01_pfm_sleep : 1;
        unsigned int buck01_ton_on : 2;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG9_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck01_new_dmd_sel : 5;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG10_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck0_ocp_sel : 4;
        unsigned int buck01_ocp_en : 1;
        unsigned int buck01_dmd_en : 1;
        unsigned int buck01_ea_sel : 1;
        unsigned int buck01_clk_sel : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG11_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck0_sleep_depth_adj : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck0_ea_comp_cap : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG12_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck01_slope_gen : 3;
        unsigned int buck01_pdn_lx_det : 1;
        unsigned int buck01_sleep_dmd : 1;
        unsigned int buck01_dmd_shield_n : 1;
        unsigned int buck01_ocp_delay_sel : 1;
        unsigned int buck01_dmd_clamp : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG13_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck01_slope_dc : 2;
        unsigned int buck01_unsleep : 1;
        unsigned int buck01_dmd_float : 1;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG14_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck0_comp_adj1 : 8;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG15_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck01_reserved1 : 8;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG16_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck01_reserved0 : 8;
    } bits;
    unsigned int u32;
}HI6551_BUCK01_REG17_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck2_adj_rlx : 4;
        unsigned int buck2_adj_clx : 4;
    } bits;
    unsigned int u32;
}HI6551_BUCK2_REG1_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck2_new_dmd_sel : 5;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_BUCK2_REG2_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck2_shield_i : 2;
        unsigned int buck2_en_regop_clamp : 1;
        unsigned int buck2_dmd_clamp : 1;
        unsigned int buck2_ocp_delay_sel : 1;
        unsigned int buck2_dmd_shield_n : 1;
        unsigned int buck2_sleep_dmd : 1;
        unsigned int buck2_pdn_lx_det : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK2_REG3_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck2_ocp_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck2_dmd_sel : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK2_REG4_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck2_ng_dt_sel : 1;
        unsigned int buck2_pg_dt_sel : 1;
        unsigned int buck2_sft_sel : 1;
        unsigned int buck2_sleep : 1;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_BUCK2_REG5_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck2_pg_n_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck2_pg_p_sel : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK2_REG6_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck2_ng_n_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck2_ng_p_sel : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK2_REG7_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck2_dbias : 2;
        unsigned int buck2_ocp_d : 1;
        unsigned int reserved_1 : 1;
        unsigned int buck2_ton : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK2_REG8_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck2_nmos_off : 1;
        unsigned int buck2_reg_c : 1;
        unsigned int buck2_short_pdp : 1;
        unsigned int buck2_reg_ss_d : 1;
        unsigned int buck2_dt_sel : 2;
        unsigned int buck2_regop_c : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK2_REG9_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck2_reg_dr : 3;
        unsigned int buck2_en_reg : 1;
        unsigned int buck2_ocp_clamp_sel : 1;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_BUCK2_REG10_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck2_reg_idr : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck2_reg_r : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK2_REG11_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck2_reserve : 8;
    } bits;
    unsigned int u32;
}HI6551_BUCK2_REG12_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck3_adj_rlx : 4;
        unsigned int buck3_adj_clx : 4;
    } bits;
    unsigned int u32;
}HI6551_BUCK3_REG1_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck3_new_dmd_sel : 5;
        unsigned int buck3_ocp_sel : 2;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK3_REG2_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck3_dmd_sel : 2;
        unsigned int buck3_en_regop_clamp : 1;
        unsigned int buck3_dmd_clamp : 1;
        unsigned int buck3_ocp_delay_sel : 1;
        unsigned int buck3_dmd_shield_n : 1;
        unsigned int buck3_sleep_dmd : 1;
        unsigned int buck3_pdn_lx_det : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK3_REG3_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck3_ng_dt_sel : 1;
        unsigned int buck3_pg_dt_sel : 1;
        unsigned int buck3_sft_sel : 1;
        unsigned int reserved_1 : 1;
        unsigned int buck3_shield_i : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK3_REG4_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck3_pg_n_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck3_pg_p_sel : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK3_REG5_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck3_ng_n_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck3_ng_p_sel : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK3_REG6_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck3_dbias : 2;
        unsigned int buck3_ocp_d : 1;
        unsigned int reserved_1 : 1;
        unsigned int buck3_ton : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK3_REG7_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck3_nmos_off : 1;
        unsigned int buck3_reg_c : 1;
        unsigned int buck3_short_pdp : 1;
        unsigned int buck3_reg_ss_d : 1;
        unsigned int buck3_dt_sel : 2;
        unsigned int buck3_regop_c : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK3_REG8_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck3_reg_dr : 3;
        unsigned int buck3_en_reg : 1;
        unsigned int buck3_ocp_clamp_sel : 1;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_BUCK3_REG9_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck3_reg_idr : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck3_reg_r : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK3_REG10_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck3_mos_sel : 2;
        unsigned int reserved : 6;
    } bits;
    unsigned int u32;
}HI6551_BUCK3_REG11_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck3_reserve : 8;
    } bits;
    unsigned int u32;
}HI6551_BUCK3_REG12_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck4_adj_rlx : 4;
        unsigned int buck4_adj_clx : 4;
    } bits;
    unsigned int u32;
}HI6551_BUCK4_REG1_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck4_new_dmd_sel : 5;
        unsigned int buck4_ocp_sel : 2;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK4_REG2_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck4_dmd_sel : 2;
        unsigned int buck4_en_regop_clamp : 1;
        unsigned int buck4_dmd_clamp : 1;
        unsigned int buck4_ocp_delay_sel : 1;
        unsigned int buck4_dmd_shield_n : 1;
        unsigned int buck4_sleep_dmd : 1;
        unsigned int buck4_pdn_lx_det : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK4_REG3_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck4_ng_dt_sel : 1;
        unsigned int buck4_pg_dt_sel : 1;
        unsigned int buck4_sft_sel : 1;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_BUCK4_REG4_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck4_pg_n_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck4_pg_p_sel : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK4_REG5_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck4_ng_n_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck4_ng_p_sel : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK4_REG6_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck4_dbias : 2;
        unsigned int buck4_ocp_d : 1;
        unsigned int reserved_1 : 1;
        unsigned int buck4_ton : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK4_REG7_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck4_nmos_off : 1;
        unsigned int buck4_reg_c : 1;
        unsigned int buck4_short_pdp : 1;
        unsigned int buck4_reg_ss_d : 1;
        unsigned int buck4_dt_sel : 2;
        unsigned int buck4_regop_c : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK4_REG8_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck4_reg_dr : 3;
        unsigned int buck4_en_reg : 1;
        unsigned int buck4_ocp_clamp_sel : 1;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_BUCK4_REG9_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck4_reg_idr : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck4_reg_r : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK4_REG10_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck4_reserve : 8;
    } bits;
    unsigned int u32;
}HI6551_BUCK4_REG12_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_adj_rlx : 4;
        unsigned int buck5_adj_clx : 4;
    } bits;
    unsigned int u32;
}HI6551_BUCK5_REG1_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_new_dmd_sel : 5;
        unsigned int buck5_ocp_sel : 2;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK5_REG2_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_dmd_sel : 2;
        unsigned int buck5_en_regop_clamp : 1;
        unsigned int buck5_dmd_clamp : 1;
        unsigned int buck5_ocp_delay_sel : 1;
        unsigned int buck5_dmd_shield_n : 1;
        unsigned int buck5_sleep_dmd : 1;
        unsigned int buck5_pdn_lx_det : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK5_REG3_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_ng_dt_sel : 1;
        unsigned int buck5_pg_dt_sel : 1;
        unsigned int buck5_sft_sel : 1;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_BUCK5_REG4_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_pg_n_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck5_pg_p_sel : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK5_REG5_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_ng_n_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck5_ng_p_sel : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK5_REG6_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_dbias : 2;
        unsigned int buck5_ocp_d : 1;
        unsigned int reserved_1 : 1;
        unsigned int buck5_ton : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK5_REG7_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_nmos_off : 1;
        unsigned int buck5_reg_c : 1;
        unsigned int buck5_short_pdp : 1;
        unsigned int buck5_reg_ss_d : 1;
        unsigned int buck5_dt_sel : 2;
        unsigned int buck5_regop_c : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK5_REG8_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_reg_dr : 3;
        unsigned int buck5_en_reg : 1;
        unsigned int buck5_ocp_clamp_sel : 1;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_BUCK5_REG9_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_reg_idr : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck5_reg_r : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK5_REG10_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_reserve0 : 8;
    } bits;
    unsigned int u32;
}HI6551_BUCK5_REG12_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_reserve1 : 8;
    } bits;
    unsigned int u32;
}HI6551_BUCK5_REG13_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_reserve2 : 8;
    } bits;
    unsigned int u32;
}HI6551_BUCK5_REG14_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck6_adj_rlx : 4;
        unsigned int buck6_adj_clx : 4;
    } bits;
    unsigned int u32;
}HI6551_BUCK6_REG1_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck6_new_dmd_sel : 5;
        unsigned int buck6_ocp_sel : 2;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK6_REG2_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck6_dmd_sel : 2;
        unsigned int buck6_en_regop_clamp : 1;
        unsigned int buck6_dmd_clamp : 1;
        unsigned int buck6_ocp_delay_sel : 1;
        unsigned int buck6_dmd_shield_n : 1;
        unsigned int buck6_sleep_dmd : 1;
        unsigned int buck6_pdn_lx_det : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK6_REG3_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck6_ng_dt_sel : 1;
        unsigned int buck6_pg_dt_sel : 1;
        unsigned int buck6_sft_sel : 1;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_BUCK6_REG4_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck6_pg_n_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck6_pg_p_sel : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK6_REG5_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck6_ng_n_sel : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck6_ng_p_sel : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK6_REG6_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck6_dbias : 2;
        unsigned int buck6_ocp_d : 1;
        unsigned int reserved_1 : 1;
        unsigned int buck6_ton : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK6_REG7_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck6_nmos_off : 1;
        unsigned int buck6_reg_c : 1;
        unsigned int buck6_short_pdp : 1;
        unsigned int buck6_reg_ss_d : 1;
        unsigned int buck6_dt_sel : 2;
        unsigned int buck6_regop_c : 1;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_BUCK6_REG8_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck6_reg_dr : 3;
        unsigned int buck6_en_reg : 1;
        unsigned int buck6_ocp_clamp_sel : 1;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_BUCK6_REG9_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck6_reg_idr : 2;
        unsigned int reserved_1 : 2;
        unsigned int buck6_reg_r : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BUCK6_REG10_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck6_mos_sel : 2;
        unsigned int reserved : 6;
    } bits;
    unsigned int u32;
}HI6551_BUCK6_REG11_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck6_reserve : 8;
    } bits;
    unsigned int u32;
}HI6551_BUCK6_REG12_ADJ_T;
typedef union
{
    struct
    {
        unsigned int cp2_vout_sel : 2;
        unsigned int reserved : 6;
    } bits;
    unsigned int u32;
}HI6551_CHG_PUMP2_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck0_vck : 6;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_VSET_BUCK01_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck2_dv : 8;
    } bits;
    unsigned int u32;
}HI6551_VSET_BUCK2_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck3_dv : 4;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_VSET_BUCK3_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck4_dv : 4;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_VSET_BUCK4_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck5_dv : 8;
    } bits;
    unsigned int u32;
}HI6551_VSET_BUCK5_ADJ_T;
typedef union
{
    struct
    {
        unsigned int buck6_dv : 4;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_VSET_BUCK6_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo1 : 3;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_LDO1_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo2 : 3;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_LDO2_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo3 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo3 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO3_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo4 : 3;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_LDO4_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo5 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo5 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO5_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo6 : 3;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_LDO6_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo7 : 3;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_LDO7_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo8 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo8 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO8_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo9 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo9 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO9_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo10 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo10 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO10_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo11 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo11 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO11_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo12 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo12 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO12_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo13 : 3;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_LDO13_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo14 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo14 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO14_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo15 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo15 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO15_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo16 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo16 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO16_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo17 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo17 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO17_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo18 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo18 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO18_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo19 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo19 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO19_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo20 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo20 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO20_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo21 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo21 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO21_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo22 : 3;
        unsigned int en_sink_ldo22 : 1;
        unsigned int rset_sink_ldo22 : 2;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_LDO22_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo23 : 3;
        unsigned int reserved_1 : 1;
        unsigned int vrset_ldo23 : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_LDO23_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_ldo24 : 3;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_LDO24_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int vset_pmua : 3;
        unsigned int eco_pmua : 1;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_PMUA_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int sstset_lvs5 : 2;
        unsigned int sstset_lvs4 : 2;
        unsigned int sstset_lvs3 : 2;
        unsigned int sstset_lvs2 : 2;
    } bits;
    unsigned int u32;
}HI6551_LVS_ADJ1_T;
typedef union
{
    struct
    {
        unsigned int sstset_lvs9 : 2;
        unsigned int sstset_lvs8 : 2;
        unsigned int sstset_lvs7 : 2;
        unsigned int sstset_lvs6 : 2;
    } bits;
    unsigned int u32;
}HI6551_LVS_ADJ2_T;
typedef union
{
    struct
    {
        unsigned int sstset_lvs10 : 2;
        unsigned int reserved : 6;
    } bits;
    unsigned int u32;
}HI6551_LVS_ADJ3_T;
typedef union
{
    struct
    {
        unsigned int en_bst_int : 1;
        unsigned int bst_pm_th : 1;
        unsigned int bst_pm_cut : 1;
        unsigned int bst_reserved0 : 5;
    } bits;
    unsigned int u32;
}HI6551_BOOST_ADJ0_T;
typedef union
{
    struct
    {
        unsigned int bst_sel_scp : 1;
        unsigned int bst_sel_pd : 1;
        unsigned int bst_en_uvp : 1;
        unsigned int bst_en_scp : 1;
        unsigned int bst_en_ovp : 1;
        unsigned int bst_en_ocp : 1;
        unsigned int bst_reserved1 : 2;
    } bits;
    unsigned int u32;
}HI6551_BOOST_ADJ1_T;
typedef union
{
    struct
    {
        unsigned int bst_drv_mode : 1;
        unsigned int bst_loop_mode : 1;
        unsigned int bst_en_pfm : 1;
        unsigned int bst_en_nring : 1;
        unsigned int bst_en_clp_os : 1;
        unsigned int bst_en_slop : 1;
        unsigned int bst_en_dmd : 1;
        unsigned int bst_reserved2 : 1;
    } bits;
    unsigned int u32;
}HI6551_BOOST_ADJ2_T;
typedef union
{
    struct
    {
        unsigned int bst_clp : 3;
        unsigned int bst_itail : 1;
        unsigned int bst_gm : 3;
        unsigned int bst_reserved3 : 1;
    } bits;
    unsigned int u32;
}HI6551_BOOST_ADJ3_T;
typedef union
{
    struct
    {
        unsigned int bst_ccom2 : 2;
        unsigned int bst_ccom1 : 2;
        unsigned int bst_rcom : 2;
        unsigned int bst_reserved4 : 2;
    } bits;
    unsigned int u32;
}HI6551_BOOST_ADJ4_T;
typedef union
{
    struct
    {
        unsigned int bst_slop : 2;
        unsigned int bst_ri : 2;
        unsigned int bst_reserved5 : 4;
    } bits;
    unsigned int u32;
}HI6551_BOOST_ADJ5_T;
typedef union
{
    struct
    {
        unsigned int bst_nsn : 2;
        unsigned int bst_nsp : 2;
        unsigned int bst_psn : 2;
        unsigned int bst_psp : 2;
    } bits;
    unsigned int u32;
}HI6551_BOOST_ADJ6_T;
typedef union
{
    struct
    {
        unsigned int bst_dt_nm : 2;
        unsigned int bst_dt_pm : 2;
        unsigned int bst_lxde : 2;
        unsigned int bst_reserved6 : 2;
    } bits;
    unsigned int u32;
}HI6551_BOOST_ADJ7_T;
typedef union
{
    struct
    {
        unsigned int bst_ckmin : 2;
        unsigned int bst_osc : 4;
        unsigned int bst_reserved7 : 2;
    } bits;
    unsigned int u32;
}HI6551_BOOST_ADJ8_T;
typedef union
{
    struct
    {
        unsigned int vo : 3;
        unsigned int sel_ocp : 2;
        unsigned int bst_start : 3;
    } bits;
    unsigned int u32;
}HI6551_BOOST_ADJ9_T;
typedef union
{
    struct
    {
        unsigned int bst_dmd_ofs : 4;
        unsigned int bst_reserved10 : 4;
    } bits;
    unsigned int u32;
}HI6551_BOOST_ADJ10_T;
typedef union
{
    struct
    {
        unsigned int bst_v2clp : 2;
        unsigned int bst_v2div : 2;
        unsigned int bst_v2ramp : 2;
        unsigned int bst_reserved11 : 2;
    } bits;
    unsigned int u32;
}HI6551_BOOST_ADJ11_T;
typedef union
{
    struct
    {
        unsigned int classd_mute_sel : 1;
        unsigned int classd_mute : 1;
        unsigned int classd_gain : 2;
        unsigned int en_classd_int : 1;
        unsigned int classd_i_ocp : 2;
        unsigned int reserved : 1;
    } bits;
    unsigned int u32;
}HI6551_CLASSD_ADJ0_T;
typedef union
{
    struct
    {
        unsigned int classd_n_sel : 2;
        unsigned int classd_p_sel : 2;
        unsigned int classd_i_ramp : 2;
        unsigned int classd_i_pump : 2;
    } bits;
    unsigned int u32;
}HI6551_CLASSD_ADJ1_T;
typedef union
{
    struct
    {
        unsigned int classd_ocp_bps : 1;
        unsigned int classd_fx_bps : 1;
        unsigned int classd_dt_sel : 1;
        unsigned int classd_pls_byp : 1;
        unsigned int classd_reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_CLASSD_ADJ2_T;
typedef union
{
    struct
    {
        unsigned int bg_test : 4;
        unsigned int bg_sleep : 1;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_BANDGAP_THSD_ADJ1_T;
typedef union
{
    struct
    {
        unsigned int en_tmp_int : 2;
        unsigned int reserved_1 : 2;
        unsigned int thsd_set_tmp : 2;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_BANDGAP_THSD_ADJ2_T;
typedef union
{
    struct
    {
        unsigned int en_dr3_int : 1;
        unsigned int dr3_mode : 1;
        unsigned int en_dr4_int : 1;
        unsigned int dr4_mode : 1;
        unsigned int en_dr5_int : 1;
        unsigned int dr5_mode : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_DR_FLA_CTRL1_T;
typedef union
{
    struct
    {
        unsigned int en_dr1_int : 1;
        unsigned int dr1_mode : 1;
        unsigned int reserved_1 : 2;
        unsigned int en_dr2_int : 1;
        unsigned int dr2_mode : 1;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_DR_FLA_CTRL2_T;
typedef union
{
    struct
    {
        unsigned int flash_period : 8;
    } bits;
    unsigned int u32;
}HI6551_FLASH_PERIOD_T;
typedef union
{
    struct
    {
        unsigned int flash_on : 8;
    } bits;
    unsigned int u32;
}HI6551_FLASH_ON_T;
typedef union
{
    struct
    {
        unsigned int dr1_mode_sel : 1;
        unsigned int dr2_mode_sel : 1;
        unsigned int dr3_mode_sel : 1;
        unsigned int dr4_mode_sel : 1;
        unsigned int dr5_mode_sel : 1;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_DR_MODE_SEL_T;
typedef union
{
    struct
    {
        unsigned int dr1_en : 1;
        unsigned int dr1_flash_en : 1;
        unsigned int reserved_1 : 2;
        unsigned int dr2_en : 1;
        unsigned int dr2_flash_en : 1;
        unsigned int reserved_0 : 2;
    } bits;
    unsigned int u32;
}HI6551_DR_BRE_CTRL_T;
typedef union
{
    struct
    {
        unsigned int dr1_t_off : 3;
        unsigned int reserved_1 : 1;
        unsigned int dr1_t_on : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_DR1_TIM_CONF0_T;
typedef union
{
    struct
    {
        unsigned int dr1_t_rise : 3;
        unsigned int reserved_1 : 1;
        unsigned int dr1_t_fall : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_DR1_TIM_CONF1_T;
typedef union
{
    struct
    {
        unsigned int iset_dr1 : 3;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_DR1_ISET_T;
typedef union
{
    struct
    {
        unsigned int dr2_t_off : 3;
        unsigned int reserved_1 : 1;
        unsigned int dr2_t_on : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_DR2_TIM_CONF0_T;
typedef union
{
    struct
    {
        unsigned int dr2_t_rise : 3;
        unsigned int reserved_1 : 1;
        unsigned int dr2_t_fall : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_DR2_TIM_CONF1_T;
typedef union
{
    struct
    {
        unsigned int iset_dr2 : 3;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_DR2_ISET_T;
typedef union
{
    struct
    {
        unsigned int dr3_en : 1;
        unsigned int dr4_en : 1;
        unsigned int dr5_en : 1;
        unsigned int dr_eco_en : 1;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_DR_LED_CTRL_T;
typedef union
{
    struct
    {
        unsigned int dr3_out_ctrl : 2;
        unsigned int dr4_out_ctrl : 2;
        unsigned int dr5_out_ctrl : 2;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_DR_OUT_CTRL_T;
typedef union
{
    struct
    {
        unsigned int iset_dr3 : 3;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_DR3_ISET_T;
typedef union
{
    struct
    {
        unsigned int dr3_start_delay : 8;
    } bits;
    unsigned int u32;
}HI6551_DR3_START_DEL_T;
typedef union
{
    struct
    {
        unsigned int dr3_t_off : 4;
        unsigned int dr3_t_on : 4;
    } bits;
    unsigned int u32;
}HI6551_DR3_TIM_CONF0_T;
typedef union
{
    struct
    {
        unsigned int dr3_t_rise : 3;
        unsigned int reserved_1 : 1;
        unsigned int dr3_t_fall : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_DR3_TIM_CONF1_T;
typedef union
{
    struct
    {
        unsigned int iset_dr4 : 3;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_DR4_ISET_T;
typedef union
{
    struct
    {
        unsigned int dr4_start_delay : 8;
    } bits;
    unsigned int u32;
}HI6551_DR4_START_DEL_T;
typedef union
{
    struct
    {
        unsigned int dr4_t_off : 4;
        unsigned int dr4_t_on : 4;
    } bits;
    unsigned int u32;
}HI6551_DR4_TIM_CONF0_T;
typedef union
{
    struct
    {
        unsigned int dr4_t_rise : 3;
        unsigned int reserved_1 : 1;
        unsigned int dr4_t_fall : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_DR4_TIM_CONF1_T;
typedef union
{
    struct
    {
        unsigned int iset_dr5 : 3;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_DR5_ISET_T;
typedef union
{
    struct
    {
        unsigned int dr5_start_delay : 8;
    } bits;
    unsigned int u32;
}HI6551_DR5_START_DEL_T;
typedef union
{
    struct
    {
        unsigned int dr5_t_off : 4;
        unsigned int dr5_t_on : 4;
    } bits;
    unsigned int u32;
}HI6551_DR5_TIM_CONF0_T;
typedef union
{
    struct
    {
        unsigned int dr5_t_rise : 3;
        unsigned int reserved_1 : 1;
        unsigned int dr5_t_fall : 3;
        unsigned int reserved_0 : 1;
    } bits;
    unsigned int u32;
}HI6551_DR5_TIM_CONF1_T;
typedef union
{
    struct
    {
        unsigned int otp_pwe_int : 1;
        unsigned int otp_pwe_pulse : 1;
        unsigned int otp_write_mask : 1;
        unsigned int otp_por_int : 1;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_OTP_CTRL1_T;
typedef union
{
    struct
    {
        unsigned int otp_pa : 2;
        unsigned int otp_ptm : 2;
        unsigned int otp_pprog : 1;
        unsigned int reserved : 3;
    } bits;
    unsigned int u32;
}HI6551_OTP_CTRL2_T;
typedef union
{
    struct
    {
        unsigned int otp_pdin : 8;
    } bits;
    unsigned int u32;
}HI6551_OTP_PDIN_T;
typedef union
{
    struct
    {
        unsigned int otp_pdob0 : 8;
    } bits;
    unsigned int u32;
}HI6551_OTP_PDOB0_T;
typedef union
{
    struct
    {
        unsigned int otp_pdob1 : 8;
    } bits;
    unsigned int u32;
}HI6551_OTP_PDOB1_T;
typedef union
{
    struct
    {
        unsigned int otp_pdob2 : 8;
    } bits;
    unsigned int u32;
}HI6551_OTP_PDOB2_T;
typedef union
{
    struct
    {
        unsigned int otp_pdob3 : 8;
    } bits;
    unsigned int u32;
}HI6551_OTP_PDOB3_T;
typedef union
{
    struct
    {
        unsigned int rtccr_a0 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCCR_A0_T;
typedef union
{
    struct
    {
        unsigned int rtccr_a1 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCCR_A1_T;
typedef union
{
    struct
    {
        unsigned int rtccr_a2 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCCR_A2_T;
typedef union
{
    struct
    {
        unsigned int rtccr_a3 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCCR_A3_T;
typedef union
{
    struct
    {
        unsigned int rtclr_a0 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCLR_A0_T;
typedef union
{
    struct
    {
        unsigned int rtclr_a1 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCLR_A1_T;
typedef union
{
    struct
    {
        unsigned int rtclr_a2 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCLR_A2_T;
typedef union
{
    struct
    {
        unsigned int rtclr_a3 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCLR_A3_T;
typedef union
{
    struct
    {
        unsigned int rtccr_a : 1;
        unsigned int reserved : 7;
    } bits;
    unsigned int u32;
}HI6551_RTCCTRL_A_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_a0 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_A0_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_a1 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_A1_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_a2 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_A2_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_a3 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_A3_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_b0 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_B0_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_b1 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_B1_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_b2 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_B2_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_b3 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_B3_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_c0 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_C0_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_c1 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_C1_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_c2 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_C2_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_c3 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_C3_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_d0 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_D0_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_d1 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_D1_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_d2 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_D2_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_d3 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_D3_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_e0 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_E0_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_e1 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_E1_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_e2 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_E2_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_e3 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_E3_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_f0 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_F0_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_f1 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_F1_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_f2 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_F2_T;
typedef union
{
    struct
    {
        unsigned int rtcmr_a_f3 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCMR_A_F3_T;
typedef union
{
    struct
    {
        unsigned int rtccr_na0 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCCR_NA0_T;
typedef union
{
    struct
    {
        unsigned int rtccr_na1 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCCR_NA1_T;
typedef union
{
    struct
    {
        unsigned int rtccr_na2 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCCR_NA2_T;
typedef union
{
    struct
    {
        unsigned int rtccr_na3 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCCR_NA3_T;
typedef union
{
    struct
    {
        unsigned int rtclr_na0 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCLR_NA0_T;
typedef union
{
    struct
    {
        unsigned int rtclr_na1 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCLR_NA1_T;
typedef union
{
    struct
    {
        unsigned int rtclr_na2 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCLR_NA2_T;
typedef union
{
    struct
    {
        unsigned int rtclr_na3 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTCLR_NA3_T;
typedef union
{
    struct
    {
        unsigned int rtccr_na : 1;
        unsigned int reserved : 7;
    } bits;
    unsigned int u32;
}HI6551_RTCCTRL_NA_T;
typedef union
{
    struct
    {
        unsigned int version : 8;
    } bits;
    unsigned int u32;
}HI6551_VERSION_T;
typedef union
{
    struct
    {
        unsigned int reserved0 : 8;
    } bits;
    unsigned int u32;
}HI6551_RESERVED0_T;
typedef union
{
    struct
    {
        unsigned int reserved1 : 8;
    } bits;
    unsigned int u32;
}HI6551_RESERVED1_T;
typedef union
{
    struct
    {
        unsigned int reserved2 : 8;
    } bits;
    unsigned int u32;
}HI6551_RESERVED2_T;
typedef union
{
    struct
    {
        unsigned int reserved3 : 8;
    } bits;
    unsigned int u32;
}HI6551_RESERVED3_T;
typedef union
{
    struct
    {
        unsigned int reserved4 : 8;
    } bits;
    unsigned int u32;
}HI6551_RESERVED4_T;
typedef union
{
    struct
    {
        unsigned int htol_mode : 4;
        unsigned int reserved : 4;
    } bits;
    unsigned int u32;
}HI6551_HTOL_MODE_T;
typedef union
{
    struct
    {
        unsigned int dac_on_sel : 1;
        unsigned int aux_ibias_cfg : 2;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_DAC_CTRL_T;
typedef union
{
    struct
    {
        unsigned int soft_rst : 8;
    } bits;
    unsigned int u32;
}HI6551_CHIP_SOFT_RST_T;
typedef union
{
    struct
    {
        unsigned int pdns_sel : 3;
        unsigned int pdns_mk : 1;
        unsigned int vsys_uv_adj : 1;
        unsigned int nopwr_rc_off : 1;
        unsigned int reserved : 2;
    } bits;
    unsigned int u32;
}HI6551_NP_REG_ADJ_T;
typedef union
{
    struct
    {
        unsigned int nopwr_vcoinsl : 2;
        unsigned int nopwr_en_backup_chg : 1;
        unsigned int reserved : 5;
    } bits;
    unsigned int u32;
}HI6551_NP_REG_CHG_T;
typedef union
{
    struct
    {
        unsigned int no_pwr_resved1 : 8;
    } bits;
    unsigned int u32;
}HI6551_NP_RSVED1_T;
typedef union
{
    struct
    {
        unsigned int no_pwr_resved2 : 8;
    } bits;
    unsigned int u32;
}HI6551_NP_RSVED2_T;
typedef union
{
    struct
    {
        unsigned int no_pwr_resved3 : 8;
    } bits;
    unsigned int u32;
}HI6551_NP_RSVED3_T;
typedef union
{
    struct
    {
        unsigned int no_pwr_resved4 : 8;
    } bits;
    unsigned int u32;
}HI6551_NP_RSVED4_T;
typedef union
{
    struct
    {
        unsigned int rtc_clk_step_adj1 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTC_ADJ1_T;
typedef union
{
    struct
    {
        unsigned int rtc_clk_step_adj2 : 8;
    } bits;
    unsigned int u32;
}HI6551_RTC_ADJ2_T;
typedef union
{
    struct
    {
        unsigned int eco_ctrl : 3;
        unsigned int reflash_value_ctrl : 1;
        unsigned int eco_filter_time : 2;
        unsigned int calibration_ctrl : 1;
        unsigned int coul_ctrl_onoff_reg : 1;
    } bits;
    unsigned int u32;
}HI6551_CLJ_CTRL_REG_T;
typedef union
{
    struct
    {
        unsigned int eco_reflash_time : 8;
    } bits;
    unsigned int u32;
}HI6551_ECO_REFALSH_TIME_T;
typedef union
{
    struct
    {
        unsigned int cl_out0 : 8;
    } bits;
    unsigned int u32;
}HI6551_CL_OUT0_T;
typedef union
{
    struct
    {
        unsigned int cl_out1 : 8;
    } bits;
    unsigned int u32;
}HI6551_CL_OUT1_T;
typedef union
{
    struct
    {
        unsigned int cl_out2 : 8;
    } bits;
    unsigned int u32;
}HI6551_CL_OUT2_T;
typedef union
{
    struct
    {
        unsigned int cl_out3 : 8;
    } bits;
    unsigned int u32;
}HI6551_CL_OUT3_T;
typedef union
{
    struct
    {
        unsigned int cl_in0 : 8;
    } bits;
    unsigned int u32;
}HI6551_CL_IN0_T;
typedef union
{
    struct
    {
        unsigned int cl_in1 : 8;
    } bits;
    unsigned int u32;
}HI6551_CL_IN1_T;
typedef union
{
    struct
    {
        unsigned int cl_in2 : 8;
    } bits;
    unsigned int u32;
}HI6551_CL_IN2_T;
typedef union
{
    struct
    {
        unsigned int cl_in3 : 8;
    } bits;
    unsigned int u32;
}HI6551_CL_IN3_T;
typedef union
{
    struct
    {
        unsigned int chg_timer0 : 8;
    } bits;
    unsigned int u32;
}HI6551_CHG_TIMER0_T;
typedef union
{
    struct
    {
        unsigned int chg_timer1 : 8;
    } bits;
    unsigned int u32;
}HI6551_CHG_TIMER1_T;
typedef union
{
    struct
    {
        unsigned int chg_timer2 : 8;
    } bits;
    unsigned int u32;
}HI6551_CHG_TIMER2_T;
typedef union
{
    struct
    {
        unsigned int chg_timer3 : 8;
    } bits;
    unsigned int u32;
}HI6551_CHG_TIMER3_T;
typedef union
{
    struct
    {
        unsigned int load_timer0 : 8;
    } bits;
    unsigned int u32;
}HI6551_LOAD_TIMER0_T;
typedef union
{
    struct
    {
        unsigned int load_timer1 : 8;
    } bits;
    unsigned int u32;
}HI6551_LOAD_TIMER1_T;
typedef union
{
    struct
    {
        unsigned int load_timer2 : 8;
    } bits;
    unsigned int u32;
}HI6551_LOAD_TIMER2_T;
typedef union
{
    struct
    {
        unsigned int load_timer3 : 8;
    } bits;
    unsigned int u32;
}HI6551_LOAD_TIMER3_T;
typedef union
{
    struct
    {
        unsigned int off_timer0 : 8;
    } bits;
    unsigned int u32;
}HI6551_OFF_TIMER0_T;
typedef union
{
    struct
    {
        unsigned int off_timer1 : 8;
    } bits;
    unsigned int u32;
}HI6551_OFF_TIMER1_T;
typedef union
{
    struct
    {
        unsigned int off_timer2 : 8;
    } bits;
    unsigned int u32;
}HI6551_OFF_TIMER2_T;
typedef union
{
    struct
    {
        unsigned int off_timer3 : 8;
    } bits;
    unsigned int u32;
}HI6551_OFF_TIMER3_T;
typedef union
{
    struct
    {
        unsigned int cl_int0 : 8;
    } bits;
    unsigned int u32;
}HI6551_CL_INT0_T;
typedef union
{
    struct
    {
        unsigned int cl_int1 : 8;
    } bits;
    unsigned int u32;
}HI6551_CL_INT1_T;
typedef union
{
    struct
    {
        unsigned int cl_int2 : 8;
    } bits;
    unsigned int u32;
}HI6551_CL_INT2_T;
typedef union
{
    struct
    {
        unsigned int cl_int3 : 8;
    } bits;
    unsigned int u32;
}HI6551_CL_INT3_T;
typedef union
{
    struct
    {
        unsigned int v_int0 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_INT0_T;
typedef union
{
    struct
    {
        unsigned int v_int1 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_INT1_T;
typedef union
{
    struct
    {
        unsigned int offset_current0 : 8;
    } bits;
    unsigned int u32;
}HI6551_OFFSET_CURRENT0_T;
typedef union
{
    struct
    {
        unsigned int offset_current1 : 8;
    } bits;
    unsigned int u32;
}HI6551_OFFSET_CURRENT1_T;
typedef union
{
    struct
    {
        unsigned int offset_voltage0 : 8;
    } bits;
    unsigned int u32;
}HI6551_OFFSET_VOLTAGE0_T;
typedef union
{
    struct
    {
        unsigned int offset_voltage1 : 8;
    } bits;
    unsigned int u32;
}HI6551_OFFSET_VOLTAGE1_T;
typedef union
{
    struct
    {
        unsigned int ocv_data0 : 8;
    } bits;
    unsigned int u32;
}HI6551_OCV_DATA1_T;
typedef union
{
    struct
    {
        unsigned int ocv_data1 : 8;
    } bits;
    unsigned int u32;
}HI6551_OCV_DATA2_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre0 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE0_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre0 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE0_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre1 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE1_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre1 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE1_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre2 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE2_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre2 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE2_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre3 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE3_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre3 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE3_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre4 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE4_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre4 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE4_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre5 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE5_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre5 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE5_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre6 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE6_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre6 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE6_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre7 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE7_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre7 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE7_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre8 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE8_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre8 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE8_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre9 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE9_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre9 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE9_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre10 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE10_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre10 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE10_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre11 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE11_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre11 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE11_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre12 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE12_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre12 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE12_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre13 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE13_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre13 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE13_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre14 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE14_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre14 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE14_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre15 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE15_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre15 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE15_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre16 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE16_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre16 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE16_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre17 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE17_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre17 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE17_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre18 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE18_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre18 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE18_T;
typedef union
{
    struct
    {
        unsigned int v_out0_pre19 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT0_PRE19_T;
typedef union
{
    struct
    {
        unsigned int v_out1_pre19 : 8;
    } bits;
    unsigned int u32;
}HI6551_V_OUT1_PRE19_T;
typedef union
{
    struct
    {
        unsigned int current0_pre0 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE0_T;
typedef union
{
    struct
    {
        unsigned int current1_pre0 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE0_T;
typedef union
{
    struct
    {
        unsigned int current0_pre1 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE1_T;
typedef union
{
    struct
    {
        unsigned int current1_pre1 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE1_T;
typedef union
{
    struct
    {
        unsigned int current0_pre2 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE2_T;
typedef union
{
    struct
    {
        unsigned int current1_pre2 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE2_T;
typedef union
{
    struct
    {
        unsigned int current0_pre3 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE3_T;
typedef union
{
    struct
    {
        unsigned int current1_pre3 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE3_T;
typedef union
{
    struct
    {
        unsigned int current0_pre4 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE4_T;
typedef union
{
    struct
    {
        unsigned int current1_pre4 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE4_T;
typedef union
{
    struct
    {
        unsigned int current0_pre5 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE5_T;
typedef union
{
    struct
    {
        unsigned int current1_pre5 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE5_T;
typedef union
{
    struct
    {
        unsigned int current0_pre6 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE6_T;
typedef union
{
    struct
    {
        unsigned int current1_pre6 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE6_T;
typedef union
{
    struct
    {
        unsigned int current0_pre7 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE7_T;
typedef union
{
    struct
    {
        unsigned int current1_pre7 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE7_T;
typedef union
{
    struct
    {
        unsigned int current0_pre8 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE8_T;
typedef union
{
    struct
    {
        unsigned int current1_pre8 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE8_T;
typedef union
{
    struct
    {
        unsigned int current0_pre9 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE9_T;
typedef union
{
    struct
    {
        unsigned int current1_pre9 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE9_T;
typedef union
{
    struct
    {
        unsigned int current0_pre10 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE10_T;
typedef union
{
    struct
    {
        unsigned int current1_pre10 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE10_T;
typedef union
{
    struct
    {
        unsigned int current0_pre11 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE11_T;
typedef union
{
    struct
    {
        unsigned int current1_pre11 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE11_T;
typedef union
{
    struct
    {
        unsigned int current0_pre12 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE12_T;
typedef union
{
    struct
    {
        unsigned int current1_pre12 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE12_T;
typedef union
{
    struct
    {
        unsigned int current0_pre13 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE13_T;
typedef union
{
    struct
    {
        unsigned int current1_pre13 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE13_T;
typedef union
{
    struct
    {
        unsigned int current0_pre14 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE14_T;
typedef union
{
    struct
    {
        unsigned int current1_pre14 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE14_T;
typedef union
{
    struct
    {
        unsigned int current0_pre15 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE15_T;
typedef union
{
    struct
    {
        unsigned int current1_pre15 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE15_T;
typedef union
{
    struct
    {
        unsigned int current0_pre16 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE16_T;
typedef union
{
    struct
    {
        unsigned int current1_pre16 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE16_T;
typedef union
{
    struct
    {
        unsigned int current0_pre17 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE17_T;
typedef union
{
    struct
    {
        unsigned int current1_pre17 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE17_T;
typedef union
{
    struct
    {
        unsigned int current0_pre18 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE18_T;
typedef union
{
    struct
    {
        unsigned int current1_pre18 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE18_T;
typedef union
{
    struct
    {
        unsigned int current0_pre19 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT0_PRE19_T;
typedef union
{
    struct
    {
        unsigned int current1_pre19 : 8;
    } bits;
    unsigned int u32;
}HI6551_CURRENT1_PRE19_T;
typedef union
{
    struct
    {
        unsigned int cic_clk_inv_i : 1;
        unsigned int cic_clk_inv_v : 1;
        unsigned int adc_ana_v_output : 1;
        unsigned int adc_ana_i_output : 1;
        unsigned int cali_en_i : 1;
        unsigned int cali_en_i_force : 1;
        unsigned int cali_en_v_force : 1;
        unsigned int cali_en_v : 1;
    } bits;
    unsigned int u32;
}HI6551_CLJ_DEBUG_T;
typedef union
{
    struct
    {
        unsigned int state_test : 8;
    } bits;
    unsigned int u32;
}HI6551_STATE_TEST_T;
typedef union
{
    struct
    {
        unsigned int reg_data_clr : 1;
        unsigned int clj_rw_inf1 : 7;
    } bits;
    unsigned int u32;
}HI6551_CLJ_RESERVED1_T;
typedef union
{
    struct
    {
        unsigned int clj_rw_inf2 : 8;
    } bits;
    unsigned int u32;
}HI6551_CLJ_RESERVED2_T;
typedef union
{
    struct
    {
        unsigned int clj_rw_inf3 : 8;
    } bits;
    unsigned int u32;
}HI6551_CLJ_RESERVED3_T;
typedef union
{
    struct
    {
        unsigned int clj_rw_inf4 : 8;
    } bits;
    unsigned int u32;
}HI6551_CLJ_RESERVED4_T;
typedef union
{
    struct
    {
        unsigned int i_reserve_1 : 8;
    } bits;
    unsigned int u32;
}HI6551_CLJ_RESERVED5_T;
typedef union
{
    struct
    {
        unsigned int i_reserve_2 : 8;
    } bits;
    unsigned int u32;
}HI6551_CLJ_RESERVED6_T;
typedef union
{
    struct
    {
        unsigned int v_reserve_1 : 8;
    } bits;
    unsigned int u32;
}HI6551_CLJ_RESERVED7_T;
typedef union
{
    struct
    {
        unsigned int v_reserve_2 : 8;
    } bits;
    unsigned int u32;
}HI6551_CLJ_RESERVED8_T;
#endif
