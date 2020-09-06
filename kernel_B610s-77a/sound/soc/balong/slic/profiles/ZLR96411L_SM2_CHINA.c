/*
 * ZLR96621L_SM2_CHINA.c --
 *
 * This file contains profile data in byte format
 *
 * Project Info --
 *   File:   C:\Microsemi\VoicePath API-II P2.22.0\coefficient_sets\VE960\ZLR96621_ABS\ZLR96621L_SM2_FULL.vpw
 *   Type:   Design for ZLR96621L SM2 Line Module Featuring the Le9662, Lite Coefficients
 *   Date:   Friday, June 20, 2014 17:04:28
 *   Device: VE960 Le9662_A
 *
 *   This file was generated with Profile Wizard Version: P2.5.0
 *
 * Project Comments --
 *  ----------------------------------------------------------------------------------------------------------------------------------
 *  Profile Wizard Coefficient Revision 2.9 Release Notes:
 *  ----------------------------------------------------------------------------------------------------------------------------------
 *  I. General:
 *  1. This release includes support for the following 44 countries:
 *  Argentina (AR), Austria (AT), Australia (AU), Belgium (BE), Brazil (BR), Bulgaria (BG), Canada (CA), Switzerland (CH),
 *  Chile (CL), China (CN), Czech Republic (CZ), Germany (DE), Denmark (DK), Ecuador (EC), Spain (ES), Finland (FI),
 *  France (FR), UK (GB), Greece (GR), Hong Kong SAR China (HK), Hungary (HU), Indonesia (ID), Ireland (IE), Israel (IL),
 *  India (IN), Iceland (IS), Italy (IT), Japan (JP), S. Korea (KR), Mexico (MX), Malaysia (MY), Netherlands (NL),
 *  Norway (NO), New Zealand (NZ), Poland (PL), Portugal (PT), Russian Federation (RU), Sweden (SE), Singapore (SG),
 *  Thailand (TH), Turkey (TK), Taiwan (TW), USA (US), and South Africa (ZA).
 *  2. The coefficients in this and all releases are provided for use only with the Microsemi VoicePath API-II (VP-API-II). Please refer 
 *  to the terms and conditions for licensing the software regarding terms and conditions of usage. These profiles are provided for 
 *  reference only with no guarantee whatsoever by Microsemi Corporation.
 *  3. This release is for the ZLR96621L SM2 Line Module based on the Le9662.
 *  
 *  II. Device Profile:
 *  1. The default settings for the Device Profiles are:
 *         PCLK = 2048 kHz
 *         PCM Transmit Edge = Positive
 *         Transmit Time Slot = 6 (for ZSI)
 *         Receive Time Slot = 0
 *         Interrupt Mode = Open Drain
 *         Switching Regulator Y Control Mode = Single
 *         Switching Regulator Y Voltage = 27V or 30V
 *         Switching Regulator Z Control Mode = Single
 *         Switching Regulator Z Voltage = 81V or 90V
 *         IO21 Pin Mode = Digital
 *         IO22 Pin Mode = Analog Voltage Sense
 *      
 *  2. The settings may be changed by the user as necessary.  Please refer to the VE960 and VP-API-II documentation for information 
 *  about the supported settings.
 *  
 *  II. DC Profiles:
 *  1. The DC_FXS_VE960_ABS100V_DEF Profile is the default unless a country specific profile is selected. Example DC profile settings are
 *  provided for China, ETSI and the USA.
 *  
 *  III. AC Profiles:
 *  1. FXS Coefficients assume -6dBr RX (Output from chipset) and 0dB TX relative gain levels.
 *  2. Supported countries not individually listed should use the default 600R profile AC_FXS_RF14_600R_DEF.
 *  4. AC FXS Coefficients assume the use of two 7 ohm series resistors or PTCs. Customers using other PTC resistance values (such as 
 *  25ohms or 50 ohms) should not use these AC coefficients and can request alternate ones from Microsemi.
 *  5. This release includes both Narrowband and Wideband coefficients. Note that the VE960 Series devices support per channel Narrowband or
 *  Wideband audio selection.
 *  
 *  IV. Ring Profiles:
 *  1. RING_VE960_ABS100V_DEF is the default ringing profile and should be used for all countries which do not have a listed ringing profile.  
 *  The default ringing profile is set for a sine wave ringing with an amplitude of 50Vrms (70.7Vpk)with no DC bias and a frequency of 25 Hz 
 *  generated by fixed (non-tracking) supply.
 *  2. Most ringing profiles on the list are sinusoidal with an amplitude of 50Vrms with no DC bias generated by fixed (non-tracking) supply. 
 *  3. The ringing definitions may be changed based on the requirements of the target market as long as the total amplitude (AC + DC 
 *  components) does not exceed the limits set forth in the Le9662 data sheet.
 *  
 *  V. Tone Profiles:
 *  1. These profiles are available only in the full version of the VP-API-II.
 *  
 *  VI. Cadence Profiles:
 *  1. These profiles are available only in the full version of the VP-API-II.
 *  
 *  VII. Caller ID Profiles:
 *  1. These profiles are available only in the full version of the VP-API-II.
 *  
 *  VIII. Metering Profiles:
 *  1. These profiles are available only in the full version of the VP-API-II.
 *  
 *  (c) Copyright 2013 Microsemi Corporation. All rights reserved.
 *  
 *  -----------------------------------------------------------------------------------------------------------------------------------------------------
 */
 

#include "ZLR96411L_SM2_CHINA.h"


/************** Device Profile **************/
#if (FEATURE_ON == MBB_SLIC)
/* Device Configuration Data - Le9662 100V ABS (-81V/-27V) */
/* B612��B715��slicоƬ�����ӷ�ʽ����B618��ͬ��slic�������Լ̳� */
#if defined(BSP_CONFIG_BOARD_CPE_B618) || defined(BSP_CONFIG_BOARD_CPE_B612) || defined(BSP_CONFIG_BOARD_CPE_B715)
VpProfileDataType ZLR96411L_ABS81V_DEVICE[] =
{
/* Profile Header ---------------------------------------------------------- */
   0x0D, 0xFF, 0x00,       /* Device Profile for Le9641 device */
   0x28,                   /* Profile length = 40 + 4 = 44 bytes */
   0x04,                   /* Profile format version = 4 */
   0x14,                   /* MPI section length = 20 bytes */
/* Raw MPI Section --------------------------------------------------------- */
   0x46, 0x0A,             /* PCLK = 8.192 MHz; INTM = CMOS-compatible */
   0x44, 0x46,             /* PCM Clock Slot = 6 TX / 0 RX; XE = Pos. */
   0x5E, 0x14, 0x00,       /* Device Mode Register */
   0xF6, 0x95, 0x00, 0x68, /* Switching Regulator Timing Params */
         0x30, 0x94, 0x30,
   0xE4, 0x44, 0x92, 0x0A, /* Switching Regulator Params */
   0xE6, 0x60,             /* Switching Regulator Control */
/* Formatted Parameters Section -------------------------------------------- */
   0x00,                   /* IO21 Mode: Digital */
                           /* IO22 Mode: Digital */
   0xA0,                   /* Dial pulse correction: 10 ms */
                           /* Switcher Configuration =
                              BuckBoost 47uH 1.5A miSLIC Power Limited (12 V in, 100 V out) */
   0x00, 0x00,             /* Over-current event threshold = Off */
  0x01,                   /* Leading edge blanking window = 81 ns */
   0x30, 0x14, 0x30, 0x14, /* FreeRun Timing Parameters */
         0x30, 0x14,
   0xFF,
   0x95, 0x00,             /* Low-Power Timing Parameters */
   0x62, 0x62,             /* Switcher Limit Voltages 98 V, 98 V */
   0x04,                   /* Charge pump disabled */
                           /* Charge Pump Overload Protection: Disabled */
   0x3C                    /* Switcher Input 12 V */

};
#else /*-BSP_CONFIG_BOARD_CPE_B618*/
VpProfileDataType ZLR96411L_ABS81V_DEVICE[] =
{
/* Profile Header ---------------------------------------------------------- */
   0x0D, 0xFF, 0x00,       /* Device Profile for Le9661 device */
   0x28,                   /* Profile length = 40 + 4 = 44 bytes */
   0x04,                   /* Profile format version = 4 */
   0x14,                   /* MPI section length = 20 bytes */
/* Raw MPI Section --------------------------------------------------------- */
   0x46, 0x8A,             /* PCLK = 8.192 MHz; INTM = Open Drain */
   0x44, 0x46,             /* PCM Clock Slot = 6 TX / 0 RX; XE = Neg. */
   0x5E, 0x14, 0x00,       /* Device Mode Register */
   0xF6, 0x95, 0x00, 0x58, /* Switching Regulator Timing Params */
         0x30, 0x58, 0x30,
   0xE4, 0x04, 0x8F, 0x0A, /* Switching Regulator Params */
   0xE6, 0x60,             /* Switching Regulator Control */
/* Formatted Parameters Section -------------------------------------------- */
   0x00,                   /* IO21 Mode: Digital */
                           /* IO22 Mode: Digital */
   0xA0,                   /* Dial pulse correction: 10 ms */
                           /* Switcher Configuration =
                              BuckBoost 47uH 1.5A miSLIC Power Limited (12 V in, 100 V out) */
   0x00, 0x00,             /* Over-current event threshold = Off */
   0x01,                   /* Leading edge blanking window = 81 ns */
   0x30, 0x14, 0x30, 0x14, /* FreeRun Timing Parameters */
         0x30, 0x14,
   0xFF,
   0x95, 0x00,             /* Low-Power Timing Parameters */
   0x62, 0x62,             /* Switcher Limit Voltages 98 V, 98 V */
   0x04,                   /* Charge pump disabled */
                           /* Charge Pump Overload Protection: Disabled */
   0x3C                    /* Switcher Input 12 V */
};
#endif /*!BSP_CONFIG_BOARD_CPE_B618*/

/************** DC Profile **************/

#if (FEATURE_ON == MBB_FACTORY)
/* DC FXS Defaults - Use for all countries unless country file exists - 23mA Current Feed */
VpProfileDataType DC_FXS_MISLIC_ABS100V_DEF[] =
{
/* Profile Header ---------------------------------------------------------- */
   0x0D, 0x01, 0x00,       /* DC Profile for Le9661 device */
   0x0C,                   /* Profile length = 12 + 4 = 16 bytes */
   0x02,                   /* Profile format version = 2 */
   0x03,                   /* MPI section length = 3 bytes */
/* Raw MPI Section --------------------------------------------------------- */
   0xC6, 0x92, 0x22,       /* DC Feed Parameters: ir_overhead = 100 ohm; */
                           /* VOC = 48 V; LI = 50 ohm; VAS = 8.78 V; ILA = 20 mA */
                           /* Maximum Voice Amplitude = 2.93 V */
/* Formatted Parameters Section -------------------------------------------- */
   0x1F,                   /* Switch Hook Threshold = 14 mA */
                           /* Ground-Key Threshold = 18 mA */
   0x84,                   /* Switch Hook Debounce = 8 ms */
                           /* Ground-Key Debounce = 16 ms */
   0x58,                   /* Low-Power Switch Hook Hysteresis = 2 V */
                           /* Ground-Key Hysteresis = 6 mA */
                           /* Switch Hook Hysteresis = 2 mA */
   0x40,                   /* Low-Power Switch Hook Threshold = 18 V */
   0x03,                   /* Floor Voltage = -20 V */
   0x00,                   /* R_OSP = 0 ohms */
   0x07                    /* R_ISP = 7 ohms */
};

#else /*-MBB_FACTORY*/
/* China DC FXS Parameters - 20mA Current Feed VBL=-20V */
VpProfileDataType DC_FXS_MISLIC_ABS100V_DEF[] =
{
/* Profile Header ---------------------------------------------------------- */
   0x0D, 0x01, 0x00,       /* DC Profile for Le9661 device */
   0x0C,                   /* Profile length = 12 + 4 = 16 bytes */
   0x02,                   /* Profile format version = 2 */
   0x03,                   /* MPI section length = 3 bytes */
/* Raw MPI Section --------------------------------------------------------- */
   0xC6, 0x92, 0x22,       /* DC Feed Parameters: ir_overhead = 100 ohm; */
                           /* VOC = 48 V; LI = 50 ohm; VAS = 8.78 V; ILA = 25 mA */
                           /* Maximum Voice Amplitude = 2.93 V */
/* Formatted Parameters Section -------------------------------------------- */
   0x1F,                   /* Switch Hook Threshold = 14 mA */
                           /* Ground-Key Threshold = 18 mA */
   0x84,                   /* Switch Hook Debounce = 8 ms */
                           /* Ground-Key Debounce = 16 ms */
   0x58,                   /* Low-Power Switch Hook Hysteresis = 2 V */
                           /* Ground-Key Hysteresis = 6 mA */
                           /* Switch Hook Hysteresis = 2 mA */
   0x40,                   /* Low-Power Switch Hook Threshold = 18 V */
   0x03,                   /* Floor Voltage = -20 V */
   0x00,                   /* R_OSP = 0 ohms */
   0x07                    /* R_ISP = 7 ohms */
};

#endif /*!MBB_FACTORY*/

#else /*-MBB_SLIC*/
/* Device Configuration Data - Le9662 100V ABS (-81V/-27V) */
VpProfileDataType ZLR96411L_ABS81V_DEVICE[] =
{
/* Profile Header ---------------------------------------------------------- */
   0x0C, 0xFF, 0x00,       /* Device Profile for Le9662_A device */
   0x2C,                   /* Profile length = 44 + 4 = 48 bytes */
   0x05,                   /* Profile format version = 5 */
   0x14,                   /* MPI section length = 20 bytes */
/* Raw MPI Section --------------------------------------------------------- */
#if (ZSI_PCLK_8M == ZSI_PCLK_MODE)
   0x46, 0x8A,             /* PCLK = 8.192 MHz; INTM = Open Drain */
#elif (ZSI_PCLK_2M == ZSI_PCLK_MODE)
   0x46, 0x82,             /* PCLK = 2.048 MHz; INTM = Open Drain */   
#endif
   0x44, 0x46,             /* PCM Clock Slot = 6 TX / 0 RX; XE = Neg. */
   0x5E, 0x14, 0x04,       /* Device Mode Register */
   0xF6, 0x66, 0x00, 0x4C, /* Switching Regulator Timing Params */
         0x30, 0x64, 0x30,
   0xE4, 0x05, 0xA0, 0x00, /* Switching Regulator Params */
   0xE6, 0x00,             /* Switching Regulator Control */
/* Formatted Parameters Section -------------------------------------------- */
   0x20,                   /* IO21 Mode: Digital */
                           /* IO22 Mode: Analog Voltage Sense */
   0x21,                   /* Dial pulse correction: 2 ms */
                           /* Switcher Configuration = ABS (12 V in, 100 V out) */
   0x00, 0x00,             /* Over-current shutdown count = off */
   0x21,                   /* Leading edge blanking window = 81 ns */
                           /* ABS supply config: 1 = Single; 2 = Slave */
   0x45, 0x00, 0x44, 0x30, /* Free-Run Timing Parameters */
         0x64, 0x30,
   0x1B,                   /* Switcher 1 Voltage = 27 V */
   0x51,                   /* Switcher 2 Voltage = 81 V */
   0xFF,
   0x66, 0x00,             /* Low-Power Timing Parameters */
   0x20, 0x6E,             /* Switcher Limit Voltages 32 V, 110 V */
   0x03,                   /* Charge pump enabled */
                           /* Charge Pump Overload Protection: Switcher Cycle Skip */
   0x3C,                   /* Switcher Input 12 V */
   0x00, 0x00              /* VBH Offset 0 V, VBH Overhead 0 V */
};

/************** DC Profile **************/

/* DC FXS Defaults - Use for all countries unless country file exists - 23mA Current Feed */
VpProfileDataType DC_FXS_MISLIC_ABS100V_DEF[] =
{
/* Profile Header ---------------------------------------------------------- */
   0x0C, 0x01, 0x00,       /* DC Profile for Le9662_A device */
   0x0B,                   /* Profile length = 11 + 4 = 15 bytes */
   0x02,                   /* Profile format version = 2 */
   0x03,                   /* MPI section length = 3 bytes */
/* Raw MPI Section --------------------------------------------------------- */
   0xC6, 0x91, 0xE5,       /* DC Feed Parameters: ir_overhead = 100 ohm; */
                           /* VOC = 48 V; LI = 50 ohm; ILA = 23 mA */
                           /* Battery Switch Offset Voltage = 4.02 V */
/* Formatted Parameters Section -------------------------------------------- */
   0x1C,                   /* Switch Hook Threshold = 11 mA */
                           /* Ground-Key Threshold = 18 mA */
   0x86,                   /* Switch Hook Debounce = 12 ms */
                           /* Ground-Key Debounce = 16 ms */
   0x58,                   /* Low-Power Switch Hook Hysteresis = 2 V */
                           /* Ground-Key Hysteresis = 6 mA */
                           /* Switch Hook Hysteresis = 2 mA */
   0x80,                   /* Low-Power Switch Hook Threshold = 22 V */
   0x00,                   /* R_OSP = 0 ohms */
   0x07                    /* R_ISP = 7 ohms */
};

#endif /*!MBB_SLIC*/
/************** AC Profile **************/

/* AC FXS RF14 600R Normal Coefficients (Default)  */
VpProfileDataType AC_FXS_RF14_600R_DEF[] =
{
  /* AC Profile */
 0xA4, 0x00, 0xF4, 0x4C, 0x01, 0x49, 0xCA, 0xF5, 0x98, 0xAA, 0x7B, 0xAB,
 0x2C, 0xA3, 0x25, 0xA5, 0x24, 0xB2, 0x3D, 0x9A, 0x2A, 0xAA, 0xA6, 0x9F,
 0x01, 0x8A, 0x1D, 0x01, 0xA3, 0xA0, 0x2E, 0xB2, 0xB2, 0xBA, 0xAC, 0xA2,
 0xA6, 0xCB, 0x3B, 0x45, 0x88, 0x2A, 0x20, 0x3C, 0xBC, 0x4E, 0xA6, 0x2B,
 0xA5, 0x2B, 0x3E, 0xBA, 0x8F, 0x82, 0xA8, 0x71, 0x80, 0xA9, 0xF0, 0x50,
 0x00, 0x86, 0x2A, 0x42, 0xA1, 0xCB, 0x1B, 0xA3, 0xA8, 0xFB, 0x87, 0xAA,
 0xFB, 0x9F, 0xA9, 0xF0, 0x96, 0x2E, 0x01, 0x00
};

/* AC FXS RF14 China 200R+680R//100nF Normal Coefficients */
VpProfileDataType AC_FXS_RF14_CN[] =
{
  /* AC Profile */
 0xB9, 0x00, 0x1F, 0x4C, 0x01, 0x49, 0xCA, 0xE9, 0x98, 0x5C, 0xB1, 0xAA,
 0x23, 0x43, 0xEB, 0x32, 0xAA, 0x23, 0xB3, 0x9A, 0x98, 0x94, 0x97, 0x9F,
 0x01, 0x8A, 0x2D, 0x01, 0x22, 0x30, 0x6A, 0x59, 0x2A, 0xCB, 0xB4, 0xB2,
 0xBF, 0x3B, 0xA3, 0x25, 0x88, 0x43, 0xC0, 0x3A, 0x45, 0x2B, 0xBD, 0x4C,
 0x2E, 0x4C, 0x25, 0x42, 0xAD, 0x82, 0xA8, 0x71, 0x80, 0xA9, 0xF0, 0x50,
 0x00, 0x86, 0xA4, 0x5A, 0x32, 0xB5, 0x12, 0xA1, 0xA2, 0x3A, 0xA4, 0xB5,
 0x64, 0x97, 0xAA, 0xF0, 0x96, 0x2E, 0x01, 0x00
};

/* AC FXS RF14 China 200R+680R//100nF Wideband Coefficients */
VpProfileDataType AC_FXS_RF14_WB_CN[] =
{
  /* AC Profile */
 0xEF, 0x00, 0x46, 0x4C, 0x01, 0x49, 0xCA, 0xDA, 0x98, 0xB3, 0xB1, 0x23,
 0x32, 0x64, 0x97, 0xAA, 0x2B, 0x4A, 0xA4, 0x9A, 0x01, 0x90, 0x01, 0x01,
 0x90, 0x8A, 0x2E, 0x01, 0xCD, 0x10, 0x2B, 0x29, 0x22, 0x2A, 0xA2, 0x33,
 0x4B, 0x4C, 0xBD, 0xF6, 0x88, 0x2A, 0xC0, 0x52, 0x97, 0xF2, 0x2E, 0xAA,
 0x4E, 0xAC, 0xB5, 0x5B, 0xBE, 0x82, 0xA8, 0x71, 0x80, 0xA9, 0xF0, 0x50,
 0x00, 0x86, 0x2A, 0x5A, 0xA2, 0xBA, 0x1B, 0x22, 0xCA, 0x24, 0xC3, 0x43,
 0x42, 0xA4, 0x4B, 0x50, 0x96, 0x3C, 0x41, 0x00
};

/************** Ringing Profile **************/
#if (FEATURE_ON == MBB_SLIC)
/* China Ringing 25Hz 55Vrms Fixed, AC Trip */
VpProfileDataType RING_VE960_ABS100V_DEF[] =
{
 /* Profile Header ---------------------------------------------------------- */
    0x0D, 0x04, 0x00,       /* Ringing Profile for Le9661 device */
    0x12,                   /* Profile length = 18 + 4 = 22 bytes */
    0x01,                   /* Profile format version = 1 */
    0x0C,                   /* MPI section length = 12 bytes */
 /* Raw MPI Section --------------------------------------------------------- */
    0xC0, 0x08, 0x00, 0x00, /* Ringing Generator Parameters: */
          0x00, 0x44, 0x40, /* 24.9 Hz Sine; 1.41 CF; 78.00 Vpk; 0.00 V bias */
          0xAA, 0x00, 0x00, /* Ring trip cycles = 1; RTHALFCYC = 0 */
          0x00, 0x00,
 /* Formatted Parameters Section -------------------------------------------- */
    0xA6,                   /* Ring Trip Method = AC; Threshold = 19.0 mA */
    0x00,                   /* Ringing Current Limit = 50 mA */
    0x4F,                   /* Fixed; Max Ringing Supply = 80 V */
    0x00                    /* Balanced; Ring Cadence Control Disabled */

};
#else
/* Default Ringing 25Hz 50Vrms Fixed, AC Trip - Use for all countries unless country profile exists */
VpProfileDataType RING_VE960_ABS100V_DEF[] =
{
/* Profile Header ---------------------------------------------------------- */
   0x0C, 0x04, 0x00,       /* Ringing Profile for Le9662_A device */
   0x12,                   /* Profile length = 18 + 4 = 22 bytes */
   0x01,                   /* Profile format version = 1 */
   0x0C,                   /* MPI section length = 12 bytes */
/* Raw MPI Section --------------------------------------------------------- */
   0xC0, 0x08, 0x00, 0x00, /* Ringing Generator Parameters: */
         0x00, 0x44, 0x3A, /* 24.9 Hz Sine; 1.41 CF; 70.70 Vpk; 0.00 V bias */
         0x9D, 0x00, 0x00, /* Ring trip cycles = 1; RTHALFCYC = 0 */
         0x00, 0x00,
/* Formatted Parameters Section -------------------------------------------- */
   0xAA,                   /* Ring Trip Method = AC; Threshold = 21.0 mA */
   0x02,                   /* Ringing Current Limit = 54 mA */
   0x4E,                   /* Fixed; Max Ringing Supply = 75 V */
   0x00                    /* Balanced; Ring Cadence Control Disabled */
};
#endif
/************** Tone Profile **************/

/* China Dial Tone  */
VpProfileDataType T_DL_CN[] =
{
/* Sig gen A = 450.07 Hz at -4.00 dBm0 */
/* Profile Header ---------------------------------------------------------- */
   0x0C, 0x02, 0x00,       /* Tone Profile for Le9641_A device */
   0x0F,                   /* Profile length = 15 + 4 = 19 bytes */
   0x02,                   /* Profile format version = 2 */
   0x0C,                   /* MPI section length = 12 bytes */
/* Raw MPI Section --------------------------------------------------------- */
   0xD2, 0x00, 0x00, 0x00, /* Signal Generator A, B, Bias Parameters */
         0x04, 0xCD, 0x38,
         0x43, 0x00, 0x00,
         0x0E, 0x22,
/* Formatted Parameters Section -------------------------------------------- */
   0x01                    /* FM_EN = 0; WB = 0; Active Generators: A */
};

/* China Ringback Tone  */
VpProfileDataType T_RB_CN[] =
{
/* Sig gen A = 450.07 Hz at -10.00 dBm0 */
/* Profile Header ---------------------------------------------------------- */
   0x0C, 0x02, 0x00,       /* Tone Profile for Le9641_A device */
   0x0F,                   /* Profile length = 15 + 4 = 19 bytes */
   0x02,                   /* Profile format version = 2 */
   0x0C,                   /* MPI section length = 12 bytes */
/* Raw MPI Section --------------------------------------------------------- */
   0xD2, 0x00, 0x00, 0x00, /* Signal Generator A, B, Bias Parameters */
         0x04, 0xCD, 0x1C,
         0x33, 0x00, 0x00,
         0x0E, 0x22,
/* Formatted Parameters Section -------------------------------------------- */
   0x01                    /* FM_EN = 0; WB = 0; Active Generators: A */

};


/* China Busy Tone  */
VpProfileDataType T_BS_CN[] =
{
/* Sig gen A = 450.07 Hz at -12.00 dBm0 */
/* Profile Header ---------------------------------------------------------- */
   0x0C, 0x02, 0x00,       /* Tone Profile for Le9641_A device */
   0x0F,                   /* Profile length = 15 + 4 = 19 bytes */
   0x02,                   /* Profile format version = 2 */
   0x0C,                   /* MPI section length = 12 bytes */
/* Raw MPI Section --------------------------------------------------------- */
   0xD2, 0x00, 0x00, 0x00, /* Signal Generator A, B, Bias Parameters */
         0x04, 0xCD, 0x16,
         0x66, 0x00, 0x00,
         0x0E, 0x22,
/* Formatted Parameters Section -------------------------------------------- */
   0x01                    /* FM_EN = 0; WB = 0; Active Generators: A */
};

/* China Call Waiting Tone */
VpProfileDataType T_CW_CN[] =
{
/* Sig gen A = 450.07 Hz at -7.00 dBm0 */
/* Profile Header ---------------------------------------------------------- */
   0x0C, 0x02, 0x00,       /* Tone Profile for Le9662_A device */
   0x0F,                   /* Profile length = 15 + 4 = 19 bytes */
   0x02,                   /* Profile format version = 2 */
   0x0C,                   /* MPI section length = 12 bytes */
/* Raw MPI Section --------------------------------------------------------- */
   0xD2, 0x00, 0x00, 0x00, /* Signal Generator A, B, Bias Parameters */
         0x04, 0xCD, 0x27,
         0xD5, 0x00, 0x00,
         0x13, 0xF7,
/* Formatted Parameters Section -------------------------------------------- */
   0x01                    /* FM_EN = 0; WB = 0; Active Generators: A */
};

/* China Congestion Tone */
VpProfileDataType T_CG_CN[] =
{
/* Sig gen A = 450.07 Hz at -15.00 dBm0 */
/* Profile Header ---------------------------------------------------------- */
   0x0C, 0x02, 0x00,       /* Tone Profile for Le9641_A device */
   0x0F,                   /* Profile length = 15 + 4 = 19 bytes */
   0x02,                   /* Profile format version = 2 */
   0x0C,                   /* MPI section length = 12 bytes */
/* Raw MPI Section --------------------------------------------------------- */
   0xD2, 0x00, 0x00, 0x00, /* Signal Generator A, B, Bias Parameters */
         0x04, 0xCD, 0x0F,
         0xDC, 0x00, 0x00,
         0x0E, 0x22,
/* Formatted Parameters Section -------------------------------------------- */
   0x01                    /* FM_EN = 0; WB = 0; Active Generators: A */
};

/* China alarm tone */
VpProfileDataType T_ALARM_CN[] =
{
   0x0C, 0x02, 0x00, 0x18, 0x02, 0x15, 0xD2, 0x00, 0x00, 0x00, 0x0E, 0xEF, 
   0x2C, 0xB0, 0x15, 0xF9, 0x2C, 0xB0, 0xD4, 0x1A, 0x22, 0x2C, 0xB0, 0x1B, 
   0xBC, 0x2C, 0xB0, 0x0F
};

/* China Fault tone */
VpProfileDataType T_FAULT_CN[] =
{
/* Sig gen A = 450.07 Hz at -12.00 dBm0 */
/* Profile Header ---------------------------------------------------------- */
   0x0C, 0x02, 0x00,       /* Tone Profile for Le9662_A device */
   0x0F,                   /* Profile length = 15 + 4 = 19 bytes */
   0x02,                   /* Profile format version = 2 */
   0x0C,                   /* MPI section length = 12 bytes */
/* Raw MPI Section --------------------------------------------------------- */
   0xD2, 0x00, 0x00, 0x00, /* Signal Generator A, B, Bias Parameters */
         0x04, 0xCD, 0x16,
         0x66, 0x00, 0x00,
         0x13, 0xF7,
/* Formatted Parameters Section -------------------------------------------- */
   0x01                    /* FM_EN = 0; WB = 0; Active Generators: A */
};

VpProfileDataType T_SVCEN_CN[] =
{
   0x0D, 0x02, 0x00, 0x0F, 0x02, 0x0C, 0xD2, 0x00, 0x00, 0x00, 0x04, 0x89, 
   0x27, 0xD4, 0x00, 0x00, 0x00, 0x00, 0x01
};

VpProfileDataType T_SVCDIS_CN[] =
{
   0x0D, 0x02, 0x00, 0x0F, 0x02, 0x0C, 0xD2, 0x00, 0x00, 0x00, 0x04, 0x89, 
   0x27, 0xD4, 0x00, 0x00, 0x00, 0x00, 0x01
};

VpProfileDataType T_DL_VM_CN[] =
{
    /* Sig gen A = 399.90 Hz at -4.00 dBm0 */
    /* Sig gen B = 425.17 Hz at -10.00 dBm0 */
    /* Profile Header ---------------------------------------------------------- */
       0x0D, 0x02, 0x00,       /* Tone Profile for Le9641 device */
       0x0F,                   /* Profile length = 15 + 4 = 19 bytes */
       0x02,                   /* Profile format version = 2 */
       0x0C,                   /* MPI section length = 12 bytes */
    /* Raw MPI Section --------------------------------------------------------- */
       0xD2, 0x00, 0x00, 0x00, /* Signal Generator A, B, Bias Parameters */
             0x04, 0x44, 0x38,
             0x43, 0x04, 0x89,
             0x1C, 0x33,
    /* Formatted Parameters Section -------------------------------------------- */
       0x03                    /* FM_EN = 0; WB = 0; Active Generators: A B */
};

/************** Cadence Profile **************/

/* China Ringing Signal Cadence */
VpProfileDataType CR_FSK_CN[] =
{
  /* Cadence Profile */
 0x00, 0x08, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x1A, 0x01, 0x07, 0x20, 0xC8,
 0x01, 0x05, 0x20, 0x28, 0x02, 0x01, 0x22, 0xF8, 0x01, 0x07, 0x20, 0xC8,
 0x01, 0x00, 0x20, 0x28, 0x01, 0x05, 0x22, 0xF8, 0x46, 0x00
};

/* China DTMF Ringing Signal Cadence: PR_DTMF CID_cadence ringing */
VpProfileDataType CR_DTMF_CN[] =
{
  /* Cadence Profile */
 0x00, 0x08, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x16, 0x01, 0x0D, 0x20, 0xC8,
 0x02, 0x01, 0x22, 0x58, 0x01, 0x07, 0x20, 0xC8, 0x01, 0x00, 0x20, 0x28,
 0x01, 0x05, 0x22, 0xF8, 0x44, 0x00
};

/* China Dial Tone Cadence */
VpProfileDataType CT_DL_CN[] =
{
  /* Cadence Profile */
 0x00, 0x03, 0x00, 0x08, 0x00, 0x00, 0x00, 0x04, 0x00, 0x8F, 0x20, 0x00
};


/* China Ringback Tone Cadence */
VpProfileDataType CT_RB_CN[] =
{
  /* Cadence Profile */
  0x00, 0x03, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x81, 0x20, 0xC8, 
  0x00, 0x80, 0x23, 0x20, 0x40, 0x00
};

/* China Busy Tone Cadence */
VpProfileDataType CT_BS_CN[] =
{
  /* Cadence Profile */
  0x00, 0x03, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x81, 0x20, 0x46, 
  0x00, 0x80, 0x20, 0x46, 0x40, 0x00
};
/* China Call Waiting Cadence */
VpProfileDataType CT_CW_CN[] =
{
  /* Cadence Profile */
  0x00, 0x03, 0x00, 0x16, 0x00, 0x00, 0x00, 0x12, 0x00, 0x8F, 0x20, 0x28,
  0x00, 0x80, 0x20, 0x28, 0x00, 0x8F, 0x20, 0x28, 0x00, 0x80, 0x23, 0x20,
  0x40, 0x00
};

/* China Congestion Cadence */
VpProfileDataType CT_CG_CN[] =
{
  /* Cadence Profile */
 0x00, 0x03, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x8F, 0x20, 0x8C,
 0x00, 0x80, 0x20, 0x8C, 0x40, 0x00
};

/* China alarm Cadence */
VpProfileDataType CT_ALARM_CN[] =
{
  0x00, 0x03, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x8F, 0x20, 0x14, 
  0x00, 0x80, 0x20, 0x14, 0x40, 0x00
};

/* China fault Cadence */
VpProfileDataType CT_FAULT_CN[] =
{
  0x00, 0x03, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x8F, 0x20, 0x96,
  0x00, 0x80, 0x20, 0x96, 0x40, 0x00
};

VpProfileDataType CT_SVCEN_CN[] =
{
   0x00, 0x03, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x81, 0x20, 0x8C, 
   0x00, 0x80, 0x20, 0x28   
};

VpProfileDataType CT_SVCDIS_CN[] =
{
   0x00, 0x03, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x81, 0x20, 0x18, 
   0x00, 0x80, 0x20, 0x18, 0x40, 0x01   
};

/* China VOICE MAIL Dial Tone Cadence */
VpProfileDataType CT_DL_VM_CN[] =
{
      /* Cadence Profile */
    0x00, 0x03, 0x00, 0x66, 0x00, 0x00, 0x00, 0x62, 0x00, 0x81, 0x20, 0x14,
    0x00, 0x80, 0x20, 0x14, 0x00, 0x81, 0x20, 0x14, 0x00, 0x80, 0x20, 0x14,
    0x00, 0x81, 0x20, 0x14, 0x00, 0x80, 0x20, 0x14, 0x00, 0x81, 0x20, 0x14,
    0x00, 0x80, 0x20, 0x14, 0x00, 0x81, 0x20, 0x14, 0x00, 0x80, 0x20, 0x14,
    0x00, 0x81, 0x20, 0x14, 0x00, 0x80, 0x20, 0x14, 0x00, 0x81, 0x20, 0x14,
    0x00, 0x80, 0x20, 0x14, 0x00, 0x81, 0x20, 0x14, 0x00, 0x80, 0x20, 0x14,
    0x00, 0x81, 0x20, 0x14, 0x00, 0x80, 0x20, 0x14, 0x00, 0x81, 0x20, 0x14,
    0x00, 0x80, 0x20, 0x14, 0x00, 0x81, 0x20, 0x14, 0x00, 0x80, 0x20, 0x14,
    0x00, 0x81, 0x20, 0x14, 0x00, 0x80, 0x20, 0x14, 0x00, 0x82
};

/************** Caller ID Profile **************/

/* CHINA Caller ID (Type 1 - On-Hook) - ETSI DTMF */
VpProfileDataType CID_TYPE_DTMF_CN[] =
{
  /* Space=2200Hz, Mark=1200Hz, Amp=-10.00dBm */
  /* Caller ID Profile */
  0x00, 0x05, 0x00, 0x14, 0x00, 0x00,
  0x09, 0xD4, /* MPI Length and Command */
  0x17, 0x77, 0x1C, 0x32, 0x0C, 0xCC, 0x1C, 0x32, /* MPI Data */
  0x00, 0x00, /* Checksum Part of Message Data */
  0x00, 0x04, /* Length of Elements Data */
  0x00, 0x0E, /* Message Data (DTMF Format) */
  0x00, 0x0D  /* End of Transmission */
};

/* China Caller ID (Type 1 - On-Hook) - Telcordia FSK */
VpProfileDataType CID_TYPE1_CN[] =
{
  /* Space=2200Hz, Mark=1200Hz, Amp=-7.50dBm */
  /* Caller ID Profile */
  0x00, 0x05, 0x00, 0x20, 0x00, 0x00,
  0x09, 0xD4, /* MPI Length and Command */
  0x17, 0x77, 0x25, 0x99, 0x0C, 0xCC, 0x25, 0x99, /* MPI Data */
  0x00, 0x01, /* Checksum Computed by device/API */
  0x00, 0x10, /* Length of Elements Data */
  0x00, 0x06, 0x01, 0x2C, /* Silence Interval for 300ms */
  0x00, 0x09, 0x00, 0xFA, /* Channel Seizure for 250ms */
  0x00, 0x0A, 0x00, 0x96, /* Mark Signal for 150ms */
  0x00, 0x0B, /* Message Data (FSK Format) */
  0x00, 0x0D  /* End of Transmission */
};

/* China Caller ID (Type 2 - Off-Hook) - Telcordia FSK */
VpProfileDataType CID_TYPE2_CN[] =
{
  /* Space=2200Hz, Mark=1200Hz, Amp=-7.50dBm */
  /* Caller ID Profile */
  0x00, 0x05, 0x00, 0x54, 0x00, 0x00,
  0x09, 0xD4, /* MPI Length and Command */
  0x17, 0x77, 0x25, 0x99, 0x0C, 0xCC, 0x25, 0x99, /* MPI Data */
  0x00, 0x01, /* Checksum Computed by device/API */
  0x00, 0x44, /* Length of Elements Data */
  0x00, 0x02, /* Mute the far end */
  0x00, 0x06, 0x00, 0x05, /* Silence Interval for 5ms */
  0x00, 0x04, /* Alert Tone */
  /* China Call Waiting Tone */
  /* 450.07 Hz, -7.00 dBm0, 0.00 Hz, -100.00 dBm0 */
  0x09, 0xD4, 0x04, 0xCD, 0x27, 0xD5, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x05, 0x01, 0x90, /* Alert Tone Part 2 - Tone+Cadence = 400ms */
  0x00, 0x06, 0x00, 0x64, /* Silence Interval for 100ms */
  0x00, 0x04, /* Alert Tone */
  /* Caller ID Alert Tone (CAS) - Used in Many Country Caller ID Profiles */
  /* 2129.88 Hz, -8.00 dBm0, 2750.24 Hz, -8.00 dBm0 */
  0x09, 0xD4, 0x16, 0xB8, 0x23, 0x80, 0x1D, 0x56, 0x23, 0x80,
  0x00, 0x05, 0x00, 0x50, /* Alert Tone Part 2 - Tone+Cadence = 80ms */
  0x00, 0x08, 0x00, 0xA0, 0x00, 0xD0, 0x00, 0x00, /* Detect Tone A | D, Timeout = 160ms */
  0x00, 0x06, 0x00, 0x32, /* Silence Interval for 50ms */
  0x00, 0x0A, 0x00, 0x41, /* Mark Signal for 65ms */
  0x00, 0x0B, /* Message Data (FSK Format) */
  0x00, 0x06, 0x00, 0x05, /* Silence Interval for 5ms */
  0x00, 0x03, /* Unmute the far end */
  0x00, 0x0D  /* End of Transmission */
};

/************** Metering_Profile **************/

/* end of file ZLR96411L_SM2_CHINA.c */