/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


 
#ifndef _WLU_AT_
#define _WLU_AT_

#ifndef _UINT32_DEFINED
typedef  unsigned long int  uint32;      /* Unsigned 32 bit value */
#define _UINT32_DEFINED
#endif

#ifndef _UINT16_DEFINED
typedef  unsigned short     uint16;      /* Unsigned 16 bit value */
#define _UINT16_DEFINED
#endif

#ifndef _UINT8_DEFINED
typedef  unsigned char      uint8;       /* Unsigned 8  bit value */
#define _UINT8_DEFINED
#endif

#ifndef _INT32_DEFINED
typedef  signed long int    int32;       /* Signed 32 bit value */
#define _INT32_DEFINED
#endif

#ifndef _INT16_DEFINED
typedef  signed short       int16;       /* Signed 16 bit value */
#define _INT16_DEFINED
#endif

#ifndef _INT8_DEFINED
typedef  char        int8;        /* Signed 8  bit value */
#define _INT8_DEFINED
#endif

#define MAC_BYTE_MAX 6
/*******************************************************************************
    buf长度定义
*******************************************************************************/

#define WLC_CNTRY_BUF_SZ    4
#define    ETHER_ADDR_LEN        6
#define WLC_IOCTL_SMLEN 256
#define WLC_IOCTL_MEDLEN        1536
#define    WLC_IOCTL_MAXLEN 8192
#define SSID_FMT_BUF_LEN (4*32+1)

/*******************************************************************************
    博通wl错误码
*******************************************************************************/

#define BCME_OK                0    
#define BCME_ERROR            -1    
#define BCME_BADARG            -2    
#define BCME_USAGE_ERROR                -44    


/*******************************************************************************
    博通宏定义:每条命令的命令关键字,来源于博通wl命令的wlioctl_defs.h文件中
*******************************************************************************/

#define WLC_UP                    2
#define WLC_DOWN                3
#define WLC_GET_BSSID                23
#define WLC_GET_SSID                25
#define WLC_SET_CHANNEL                30
#define WLC_SET_PM                86
#define WLC_FREQ_ACCURACY            92    /* diag */
#define WLC_SET_RADIOREG            97
#define WLC_GET_REVINFO                98
#define WLC_SET_SCANSUPPRESS            116
#define WLC_GET_BSS_INFO            136
#define WLC_GET_BANDLIST            140
#define WLC_GET_BAND                141
#define WLC_SET_BAND                142
#define WLC_OUT                    163
#define WLC_GET_VAR                262
#define WLC_SET_VAR                263

/*******************************************************************************
    博通wl命令所需要的宏
*******************************************************************************/

#define WL_RSPEC_ENCODE_RATE    0x00000000      /* Legacy rate is stored in RSPEC_RATE_MASK */
#define WL_RSPEC_ENCODE_HT      0x01000000      /* HT MCS is stored in RSPEC_RATE_MASK */
#define WL_RSPEC_ENCODE_VHT     0x02000000      /* VHT MCS and Nss is stored in RSPEC_RATE_MASK */
#define WL_RSPEC_VHT_NSS_SHIFT  4               /* VHT Nss value shift */
#define    WLC_BAND_AUTO        0    /* auto-select */
#define    WLC_BAND_5G        1    /* 5 Ghz */
#define    WLC_BAND_2G        2    /* 2.4 Ghz */
#define    WLC_BAND_ALL        3    /* all bands */
#define    WLC_PHY_TYPE_HT        7
#define    CHAIN_1X1_1    1      /*1X1模式下的第一个天线*/
#define    CHAIN_1X1_2    2      /*1X1模式下的第二个天线*/
#define    CHAIN_2X2    3        /*2X2模式下的参数设置*/
#define    NUMBER_2     2         /*数字2*/
#define    LOOP_COUNT   3         /*OTP检查循环次数*/
#define    PKTENG_TX    1
#define    PKTENG_RX    2

#define htod64(i) (uint64)(i)
#define htod32(i) (uint32)(i)
#define htod16(i) (uint16)(i)
#define dtoh64(i) (uint64)(i)
#define dtoh32(i) (uint32)(i)
#define dtoh16(i) (uint16)(i)
#define CH_MAX_2G_CHANNEL        14    
#define WL_CHANSPEC_BAND_2G        0x0000
#define WL_CHANSPEC_BAND_5G        0xc000
#define WL_CHANSPEC_BW_20        0x1000
#define WL_CHANSPEC_BW_40        0x1800
#define WL_CHANSPEC_BW_80        0x2000
#define WL_CHANSPEC_BW_160        0x2800
#define WL_CHANSPEC_BW_8080        0x3000
#define CH_10MHZ_APART            2
#define    MAXCHANNEL        224    

#define LOWER_20_SB(channel)        (((channel) > CH_10MHZ_APART) ? \
                    ((channel) - CH_10MHZ_APART) : 0)
#define UPPER_20_SB(channel)        (((channel) < (MAXCHANNEL - CH_10MHZ_APART)) ? \
                    ((channel) + CH_10MHZ_APART) : 0)
#define WL_CHANSPEC_CTL_SB_LLL        0x0000
#define WL_CHANSPEC_CTL_SB_LLU        0x0100
#define WL_CHANSPEC_CTL_SB_NONE        WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_CTL_SB_SHIFT    8
#define WL_CHANSPEC_CHAN1_SHIFT        0
#define WL_CHANSPEC_CHAN2_SHIFT        4
#define DOT11_MAX_SSID_LEN        32    /* d11 max ssid length */
#define MCSSET_LEN    16    /* 16-bits per 8-bit set to give 128-bits bitmap of MCS Index */
#define UNIT_MW        1 /* UNIT_MW */
#define UNIT_QDBM    2 /* UNIT_QDBM */
#define UNIT_DBM    3 /* UNIT_DBM */
#define WL_TXPWR_OVERRIDE    (1U<<31)
#define WL_PKTENG_PER_TX_START            0x01
#define WL_PKTENG_PER_TX_STOP            0x02
#define WL_PKTENG_PER_RX_START            0x04
#define WL_PKTENG_PER_RX_STOP            0x08
#define    NFIFO            6    /* # tx/rx fifopairs */
#define NREINITREASONCOUNT    8
#define RADIO_CORE_SYN                           (0x0 << 12)
#define RADIO_CORE_TX0                           (0x2 << 12)
#define RADIO_CORE_TX1                           (0x3 << 12)
#define RADIO_CORE_RX0                           (0x6 << 12)
#define RADIO_CORE_RX1                           (0x7 << 12)
#define WLC_PHY_TYPE_AC        11
#define RADIO_2069_CORE_CR0                      (0x0 << 9)
#define RADIO_2069_CORE_CR1                      (0x1 << 9)
#define RADIO_2069_CORE_CR2                      (0x2 << 9)
#define RADIO_2069_CORE_ALL                      (0x3 << 9)
#define RADIO_2069_CORE_PLL                      (0x4 << 9)
#define RADIO_CORE_CR0                           (0x0 << 10)
#define RADIO_CORE_CR1                           (0x1 << 10)
#define RADIO_CORE_CR2                           (0x2 << 10)
#define RADIO_CORE_ALL                           (0x3 << 10)

/* Maximum srom: 12 Kilobits == 1536 bytes */
#define SROM_MAX        1536
#define SROM_MAXW        384
#define VARS_MAX        4096

#define WLC_CIS_DEFAULT 0    /* built-in default */
#define WLC_CIS_SROM 1       /* source is sprom */
#define WLC_CIS_OTP  2       /* source is otp */


/*******************************************************************************
            通用数据结构定义
*******************************************************************************/

struct ether_addr
{
    unsigned char octet[MAC_BYTE_MAX];
};

typedef struct wl_ioctl
{
    uint cmd;    /* common ioctl definition */
    void *buf;    /* pointer to user buffer */
    uint len;    /* length of user buffer */
    uint8 set;        /* 1=set IOCTL; 0=query IOCTL */
    uint used;    /* bytes read or written (optional) */
    uint needed;    /* bytes needed (optional) */
} wl_ioctl_t;

typedef int *(*ioctl_entry_t)(void *, wl_ioctl_t *);

typedef struct at_ioctl_entry_reg
{
    ioctl_entry_t ioctl_entry ;
    void *arg;
}at_ioctl_entry_reg_t;

typedef struct tagWlanATPacketREP
{
    unsigned int TotalRxPkts;
    unsigned int BadRxPkts;
    unsigned int UcastRxPkts;
    unsigned int McastRxPkts;
}WLAN_AT_PACK_REP_ST;

#define CISH_FLAG_PCIECIS    (1 << 15)    /* write CIS format bit for PCIe CIS */

typedef struct {
    uint16  source;     /* cis source */
    uint16  flags;      /* flags */
    uint32  byteoff;    /* byte offset */
    uint32  nbytes;     /* number of bytes */
    /* data follows here */
} cis_rw_t;

/*******************************************************************************
            提供给驱动的注册接口
*******************************************************************************/

int reg_at_ioctl_entry(ioctl_entry_t ioctl_entry, struct net_device  *arg);
void unreg_at_ioctl_entry(void);

/*******************************************************************************
            wl外部命令接口声明--对AT模块接口
*******************************************************************************/
int wl_ver(void);
int wl_down(void);
int wl_up(void);
int wl_scansuppress(int scansuppress);
int wl_country(char *country);
int wl_band(uint32 band);
int wl_channel(int channel);
int wl_chanspec(uint32 channel, uint32 bw, char ul, uint32 ch1, uint32 ch2);
int wl_status(void);
int wl_rate(int rate);
int wl_nrate(int mcs);
int wl_5g_rate(int mcs, int Nss);
int wl_txpwr1(int pwr);
int wl_fqacurcy(int freq);
int wl_out();
int wl_phy_txpwrctrl(int pwrctl);
int wl_phy_txpwrindex(uint32 arg1, uint32 arg2);
int wl_pkteng_stop(int tx_or_rx);
int wl_pkteng_start(struct ether_addr *dest, int tx_or_rx, uint32 delay, uint32 length, uint32 nframes);
int wl_mimo_bw_cap(int cap);
int wl_mimo_txbw(int txbw);
int wl_phy_forcecal(int phy_forcecal);
int wl_phy_watchdog(int watchdog);
int wl_PM(int PM);
int wl_counters();
int wl_radioreg(uint32 pos, uint32 value, char *name);
int wl_txchain(int chain);
int wl_rxchain(int chain);
int wl_mpc(int mpc);
int wlu_ciswrite(int if_p_param, char *filepath);
int wlu_cisdump(unsigned char* out_buf, int *out_len, int* if_write);

#endif
