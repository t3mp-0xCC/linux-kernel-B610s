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



#include <linux/module.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "wlu_at.h"
#include "wlan_if.h"


/*******************************************************************************
    博通wl命令所需要的结构体定义
*******************************************************************************/
typedef uint16 chanspec_t;

typedef struct wlc_ssid {
    uint32      SSID_len;
    unsigned char       SSID[DOT11_MAX_SSID_LEN];
} wlc_ssid_t;

typedef struct wl_bss_info {
    uint32        version;        /* version field */
    uint32        length;            /* byte length of data in this record,
                                     * starting at version and including IEs
                                     */
    struct ether_addr BSSID;
    uint16        beacon_period;        /* units are Kusec */
    uint16        capability;        /* Capability information */
    uint8        SSID_len;
    uint8        SSID[32];
    struct {
        uint32    count;            /* # rates in this set */
        uint8    rates[16];        /* rates in 500kbps units w/hi bit set if basic */
    } rateset;                /* supported rates */
    chanspec_t    chanspec;        /* chanspec for bss */
    uint16        atim_window;        /* units are Kusec */
    uint8        dtim_period;        /* DTIM period */
    int16        RSSI;            /* receive signal strength (in dBm) */
    int8        phy_noise;        /* noise (in dBm) */

    uint8        n_cap;            /* BSS is 802.11N Capable */
    uint32        nbss_cap;        /* 802.11N+AC BSS Capabilities */
    uint8        ctl_ch;            /* 802.11N BSS control channel number */
    uint8        padding1[3];        /* explicit struct alignment padding */
    uint16        vht_rxmcsmap;    /* VHT rx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
    uint16        vht_txmcsmap;    /* VHT tx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
    uint8        flags;            /* flags */
    uint8        vht_cap;        /* BSS is vht capable */
    uint8        reserved[2];        /* Reserved for expansion of BSS properties */
    uint8        basic_mcs[MCSSET_LEN];    /* 802.11N BSS required MCS set */

    uint16        ie_offset;        /* offset at which IEs start, from beginning */
    uint32        ie_length;        /* byte length of Information Elements */
    int16        SNR;            /* average SNR of during frame reception */
    /* Add new fields here */
    /* variable length Information Elements */
} wl_bss_info_t;

typedef struct wlc_rev_info {
    uint32        vendorid;    /* PCI vendor id */
    uint32        deviceid;    /* device id of chip */
    uint32        radiorev;    /* radio revision */
    uint32        chiprev;    /* chip revision */
    uint32        corerev;    /* core revision */
    uint32        boardid;    /* board identifier (usu. PCI sub-device id) */
    uint32        boardvendor;    /* board vendor (usu. PCI sub-vendor id) */
    uint32        boardrev;    /* board revision */
    uint32        driverrev;    /* driver version */
    uint32        ucoderev;    /* microcode version */
    uint32        bus;        /* bus type */
    uint32        chipnum;    /* chip number */
    uint32        phytype;    /* phy type */
    uint32        phyrev;        /* phy revision */
    uint32        anarev;        /* anacore rev */
    uint32        chippkg;    /* chip package info */
    uint32        nvramrev;    /* nvram revision number */
} wlc_rev_info_t;

typedef struct wl_pkteng {
    uint32 flags;
    uint32 delay;            /* Inter-packet delay */
    uint32 nframes;            /* Number of frames */
    uint32 length;            /* Packet length */
    uint8  seqno;            /* Enable/disable sequence no. */
    struct ether_addr dest;        /* Destination address */
    struct ether_addr src;        /* Source address */
} wl_pkteng_t;

typedef struct {
    uint16    version;    /* see definition of WL_CNT_T_VERSION */
    uint16    length;        /* length of entire structure */

    /* transmit stat counters */
    uint32    txframe;    /* tx data frames */
    uint32    txbyte;        /* tx data bytes */
    uint32    txretrans;    /* tx mac retransmits */
    uint32    txerror;    /* tx data errors (derived: sum of others) */
    uint32    txctl;        /* tx management frames */
    uint32    txprshort;    /* tx short preamble frames */
    uint32    txserr;        /* tx status errors */
    uint32    txnobuf;    /* tx out of buffers errors */
    uint32    txnoassoc;    /* tx discard because we're not associated */
    uint32    txrunt;        /* tx runt frames */
    uint32    txchit;        /* tx header cache hit (fastpath) */
    uint32    txcmiss;    /* tx header cache miss (slowpath) */

    /* transmit chip error counters */
    uint32    txuflo;        /* tx fifo underflows */
    uint32    txphyerr;    /* tx phy errors (indicated in tx status) */
    uint32    txphycrs;    /* PR8861/8963 counter */

    /* receive stat counters */
    uint32    rxframe;    /* rx data frames */
    uint32    rxbyte;        /* rx data bytes */
    uint32    rxerror;    /* rx data errors (derived: sum of others) */
    uint32    rxctl;        /* rx management frames */
    uint32    rxnobuf;    /* rx out of buffers errors */
    uint32    rxnondata;    /* rx non data frames in the data channel errors */
    uint32    rxbadds;    /* rx bad DS errors */
    uint32    rxbadcm;    /* rx bad control or management frames */
    uint32    rxfragerr;    /* rx fragmentation errors */
    uint32    rxrunt;        /* rx runt frames */
    uint32    rxgiant;    /* rx giant frames */
    uint32    rxnoscb;    /* rx no scb error */
    uint32    rxbadproto;    /* rx invalid frames */
    uint32    rxbadsrcmac;    /* rx frames with Invalid Src Mac */
    uint32    rxbadda;    /* rx frames tossed for invalid da */
    uint32    rxfilter;    /* rx frames filtered out */

    /* receive chip error counters */
    uint32    rxoflo;        /* rx fifo overflow errors */
    uint32    rxuflo[NFIFO];    /* rx dma descriptor underflow errors */

    uint32    d11cnt_txrts_off;    /* d11cnt txrts value when reset d11cnt */
    uint32    d11cnt_rxcrc_off;    /* d11cnt rxcrc value when reset d11cnt */
    uint32    d11cnt_txnocts_off;    /* d11cnt txnocts value when reset d11cnt */

    /* misc counters */
    uint32    dmade;        /* tx/rx dma descriptor errors */
    uint32    dmada;        /* tx/rx dma data errors */
    uint32    dmape;        /* tx/rx dma descriptor protocol errors */
    uint32    reset;        /* reset count */
    uint32    tbtt;        /* cnts the TBTT int's */
    uint32    txdmawar;    /* # occurrences of PR15420 workaround */
    uint32    pkt_callback_reg_fail;    /* callbacks register failure */

    /* MAC counters: 32-bit version of d11.h's macstat_t */
    uint32    txallfrm;    /* total number of frames sent, incl. Data, ACK, RTS, CTS,
                 * Control Management (includes retransmissions)
                 */
    uint32    txrtsfrm;    /* number of RTS sent out by the MAC */
    uint32    txctsfrm;    /* number of CTS sent out by the MAC */
    uint32    txackfrm;    /* number of ACK frames sent out */
    uint32    txdnlfrm;    /* Not used */
    uint32    txbcnfrm;    /* beacons transmitted */
    uint32    txfunfl[6];    /* per-fifo tx underflows */
    uint32    rxtoolate;    /* receive too late */
    uint32  txfbw;        /* transmit at fallback bw (dynamic bw) */
    uint32    txtplunfl;    /* Template underflows (mac was too slow to transmit ACK/CTS
                 * or BCN)
                 */
    uint32    txphyerror;    /* Transmit phy error, type of error is reported in tx-status for
                 * driver enqueued frames
                 */
    uint32    rxfrmtoolong;    /* Received frame longer than legal limit (2346 bytes) */
    uint32    rxfrmtooshrt;    /* Received frame did not contain enough bytes for its frame type */
    uint32    rxinvmachdr;    /* Either the protocol version != 0 or frame type not
                 * data/control/management
                 */
    uint32    rxbadfcs;    /* number of frames for which the CRC check failed in the MAC */
    uint32    rxbadplcp;    /* parity check of the PLCP header failed */
    uint32    rxcrsglitch;    /* PHY was able to correlate the preamble but not the header */
    uint32    rxstrt;        /* Number of received frames with a good PLCP
                 * (i.e. passing parity check)
                 */
    uint32    rxdfrmucastmbss; /* Number of received DATA frames with good FCS and matching RA */
    uint32    rxmfrmucastmbss; /* number of received mgmt frames with good FCS and matching RA */
    uint32    rxcfrmucast;    /* number of received CNTRL frames with good FCS and matching RA */
    uint32    rxrtsucast;    /* number of unicast RTS addressed to the MAC (good FCS) */
    uint32    rxctsucast;    /* number of unicast CTS addressed to the MAC (good FCS) */
    uint32    rxackucast;    /* number of ucast ACKS received (good FCS) */
    uint32    rxdfrmocast;    /* number of received DATA frames (good FCS and not matching RA) */
    uint32    rxmfrmocast;    /* number of received MGMT frames (good FCS and not matching RA) */
    uint32    rxcfrmocast;    /* number of received CNTRL frame (good FCS and not matching RA) */
    uint32    rxrtsocast;    /* number of received RTS not addressed to the MAC */
    uint32    rxctsocast;    /* number of received CTS not addressed to the MAC */
    uint32    rxdfrmmcast;    /* number of RX Data multicast frames received by the MAC */
    uint32    rxmfrmmcast;    /* number of RX Management multicast frames received by the MAC */
    uint32    rxcfrmmcast;    /* number of RX Control multicast frames received by the MAC
                 * (unlikely to see these)
                 */
    uint32    rxbeaconmbss;    /* beacons received from member of BSS */
    uint32    rxdfrmucastobss; /* number of unicast frames addressed to the MAC from
                  * other BSS (WDS FRAME)
                  */
    uint32    rxbeaconobss;    /* beacons received from other BSS */
    uint32    rxrsptmout;    /* Number of response timeouts for transmitted frames
                 * expecting a response
                 */
    uint32    bcntxcancl;    /* transmit beacons canceled due to receipt of beacon (IBSS) */
    uint32    rxf0ovfl;    /* Number of receive fifo 0 overflows */
    uint32    rxf1ovfl;    /* Number of receive fifo 1 overflows (obsolete) */
    uint32    rxf2ovfl;    /* Number of receive fifo 2 overflows (obsolete) */
    uint32    txsfovfl;    /* Number of transmit status fifo overflows (obsolete) */
    uint32    pmqovfl;    /* Number of PMQ overflows */
    uint32    rxcgprqfrm;    /* Number of received Probe requests that made it into
                 * the PRQ fifo
                 */
    uint32    rxcgprsqovfl;    /* Rx Probe Request Que overflow in the AP */
    uint32    txcgprsfail;    /* Tx Probe Response Fail. AP sent probe response but did
                 * not get ACK
                 */
    uint32    txcgprssuc;    /* Tx Probe Response Success (ACK was received) */
    uint32    prs_timeout;    /* Number of probe requests that were dropped from the PRQ
                 * fifo because a probe response could not be sent out within
                 * the time limit defined in M_PRS_MAXTIME
                 */
    uint32    rxnack;        /* obsolete */
    uint32    frmscons;    /* obsolete */
    uint32  txnack;        /* obsolete */
    uint32    rxback;        /* blockack rxcnt */
    uint32    txback;        /* blockack txcnt */

    /* 802.11 MIB counters, pp. 614 of 802.11 reaff doc. */
    uint32    txfrag;        /* dot11TransmittedFragmentCount */
    uint32    txmulti;    /* dot11MulticastTransmittedFrameCount */
    uint32    txfail;        /* dot11FailedCount */
    uint32    txretry;    /* dot11RetryCount */
    uint32    txretrie;    /* dot11MultipleRetryCount */
    uint32    rxdup;        /* dot11FrameduplicateCount */
    uint32    txrts;        /* dot11RTSSuccessCount */
    uint32    txnocts;    /* dot11RTSFailureCount */
    uint32    txnoack;    /* dot11ACKFailureCount */
    uint32    rxfrag;        /* dot11ReceivedFragmentCount */
    uint32    rxmulti;    /* dot11MulticastReceivedFrameCount */
    uint32    rxcrc;        /* dot11FCSErrorCount */
    uint32    txfrmsnt;    /* dot11TransmittedFrameCount (bogus MIB?) */
    uint32    rxundec;    /* dot11WEPUndecryptableCount */

    /* WPA2 counters (see rxundec for DecryptFailureCount) */
    uint32    tkipmicfaill;    /* TKIPLocalMICFailures */
    uint32    tkipcntrmsr;    /* TKIPCounterMeasuresInvoked */
    uint32    tkipreplay;    /* TKIPReplays */
    uint32    ccmpfmterr;    /* CCMPFormatErrors */
    uint32    ccmpreplay;    /* CCMPReplays */
    uint32    ccmpundec;    /* CCMPDecryptErrors */
    uint32    fourwayfail;    /* FourWayHandshakeFailures */
    uint32    wepundec;    /* dot11WEPUndecryptableCount */
    uint32    wepicverr;    /* dot11WEPICVErrorCount */
    uint32    decsuccess;    /* DecryptSuccessCount */
    uint32    tkipicverr;    /* TKIPICVErrorCount */
    uint32    wepexcluded;    /* dot11WEPExcludedCount */

    uint32    txchanrej;    /* Tx frames suppressed due to channel rejection */
    uint32    psmwds;        /* Count PSM watchdogs */
    uint32    phywatchdog;    /* Count Phy watchdogs (triggered by ucode) */

    /* MBSS counters, AP only */
    uint32    prq_entries_handled;    /* PRQ entries read in */
    uint32    prq_undirected_entries;    /*    which were bcast bss & ssid */
    uint32    prq_bad_entries;    /*    which could not be translated to info */
    uint32    atim_suppress_count;    /* TX suppressions on ATIM fifo */
    uint32    bcn_template_not_ready;    /* Template marked in use on send bcn ... */
    uint32    bcn_template_not_ready_done; /* ...but "DMA done" interrupt rcvd */
    uint32    late_tbtt_dpc;    /* TBTT DPC did not happen in time */

    /* per-rate receive stat counters */
    uint32  rx1mbps;    /* packets rx at 1Mbps */
    uint32  rx2mbps;    /* packets rx at 2Mbps */
    uint32  rx5mbps5;    /* packets rx at 5.5Mbps */
    uint32  rx6mbps;    /* packets rx at 6Mbps */
    uint32  rx9mbps;    /* packets rx at 9Mbps */
    uint32  rx11mbps;    /* packets rx at 11Mbps */
    uint32  rx12mbps;    /* packets rx at 12Mbps */
    uint32  rx18mbps;    /* packets rx at 18Mbps */
    uint32  rx24mbps;    /* packets rx at 24Mbps */
    uint32  rx36mbps;    /* packets rx at 36Mbps */
    uint32  rx48mbps;    /* packets rx at 48Mbps */
    uint32  rx54mbps;    /* packets rx at 54Mbps */
    uint32  rx108mbps;    /* packets rx at 108mbps */
    uint32  rx162mbps;    /* packets rx at 162mbps */
    uint32  rx216mbps;    /* packets rx at 216 mbps */
    uint32  rx270mbps;    /* packets rx at 270 mbps */
    uint32  rx324mbps;    /* packets rx at 324 mbps */
    uint32  rx378mbps;    /* packets rx at 378 mbps */
    uint32  rx432mbps;    /* packets rx at 432 mbps */
    uint32  rx486mbps;    /* packets rx at 486 mbps */
    uint32  rx540mbps;    /* packets rx at 540 mbps */

    /* pkteng rx frame stats */
    uint32    pktengrxducast; /* unicast frames rxed by the pkteng code */
    uint32    pktengrxdmcast; /* multicast frames rxed by the pkteng code */

    uint32    rfdisable;    /* count of radio disables */
    uint32    bphy_rxcrsglitch;    /* PHY count of bphy glitches */
    uint32  bphy_badplcp;

    uint32    txexptime;    /* Tx frames suppressed due to timer expiration */

    uint32    txmpdu_sgi;    /* count for sgi transmit */
    uint32    rxmpdu_sgi;    /* count for sgi received */
    uint32    txmpdu_stbc;    /* count for stbc transmit */
    uint32    rxmpdu_stbc;    /* count for stbc received */

    uint32    rxundec_mcst;    /* dot11WEPUndecryptableCount */

    /* WPA2 counters (see rxundec for DecryptFailureCount) */
    uint32    tkipmicfaill_mcst;    /* TKIPLocalMICFailures */
    uint32    tkipcntrmsr_mcst;    /* TKIPCounterMeasuresInvoked */
    uint32    tkipreplay_mcst;    /* TKIPReplays */
    uint32    ccmpfmterr_mcst;    /* CCMPFormatErrors */
    uint32    ccmpreplay_mcst;    /* CCMPReplays */
    uint32    ccmpundec_mcst;    /* CCMPDecryptErrors */
    uint32    fourwayfail_mcst;    /* FourWayHandshakeFailures */
    uint32    wepundec_mcst;    /* dot11WEPUndecryptableCount */
    uint32    wepicverr_mcst;    /* dot11WEPICVErrorCount */
    uint32    decsuccess_mcst;    /* DecryptSuccessCount */
    uint32    tkipicverr_mcst;    /* TKIPICVErrorCount */
    uint32    wepexcluded_mcst;    /* dot11WEPExcludedCount */

    uint32    dma_hang;    /* count for dma hang */
    uint32    reinit;        /* count for reinit */

    uint32  pstatxucast;    /* count of ucast frames xmitted on all psta assoc */
    uint32  pstatxnoassoc;    /* count of txnoassoc frames xmitted on all psta assoc */
    uint32  pstarxucast;    /* count of ucast frames received on all psta assoc */
    uint32  pstarxbcmc;    /* count of bcmc frames received on all psta */
    uint32  pstatxbcmc;    /* count of bcmc frames transmitted on all psta */

    uint32  cso_passthrough; /* hw cso required but passthrough */
    uint32    cso_normal;    /* hw cso hdr for normal process */
    uint32    chained;    /* number of frames chained */
    uint32    chainedsz1;    /* number of chain size 1 frames */
    uint32    unchained;    /* number of frames not chained */
    uint32    maxchainsz;    /* max chain size so far */
    uint32    currchainsz;    /* current chain size */
    uint32    rxdrop20s;    /* drop secondary cnt */
    uint32    pciereset;    /* Secondary Bus Reset issued by driver */
    uint32    cfgrestore;    /* configspace restore by driver */
    uint32    reinitreason[NREINITREASONCOUNT]; /* reinitreason counters; 0: Unknown reason */
} wl_cnt_t;

/*******************************************************************************
    全局变量定义
*******************************************************************************/

static char smbuf[WLC_IOCTL_SMLEN] = {0};
static char long_buf[WLC_IOCTL_MAXLEN] = {0};
/*WiFi数据包统计信息*/
WLAN_AT_PACK_REP_ST g_wifi_packet_report = {0};
WLAN_AT_PACK_REP_ST g_wifi_packet_new_rep = {0};/*实时信息*/
static at_ioctl_entry_reg_t g_at_ioctl_entry = {0};

/*******************************************************************************
    提供给驱动的注册接口
*******************************************************************************/

int reg_at_ioctl_entry(ioctl_entry_t ioctl_entry, struct net_device  *arg)
{
    if ((NULL == ioctl_entry) || (NULL == arg))
    {
        WLAN_TRACE_ERROR("reg_at_ioctl_entry null point error! ioctl:%p,arg:%p\n",
            ioctl_entry, arg);
        return BCME_ERROR;
    }
    memset(&g_at_ioctl_entry, 0, sizeof(at_ioctl_entry_reg_t));
    g_at_ioctl_entry.ioctl_entry = ioctl_entry;
    g_at_ioctl_entry.arg = arg;
    return 0;
}
EXPORT_SYMBOL(reg_at_ioctl_entry);

void unreg_at_ioctl_entry(void)
{
    memset(&g_at_ioctl_entry, 0, sizeof(at_ioctl_entry_reg_t));
    WLAN_TRACE_ERROR("unreg_at_ioctl_entry \n");
}
EXPORT_SYMBOL(unreg_at_ioctl_entry);

/*******************************************************************************
        通用接口函数实现
********************************************************************************/
static int wl_ioctl_at(int cmd, char *buf, uint32 len, uint8 set)
{
    wl_ioctl_t ioc;
    int ret;

    ioc.cmd = cmd;
    ioc.buf = buf;
    ioc.len = len;
    ioc.set = set;
    if (g_at_ioctl_entry.ioctl_entry)
    {
        ret = g_at_ioctl_entry.ioctl_entry(g_at_ioctl_entry.arg, &ioc);
    }
    else
    {
        WLAN_TRACE_ERROR("ioctl entry not register,error\n");
        return BCME_ERROR;
    }
    return ret;
}
static int wlu_set_at(int cmd, char *buf, uint32 len)
{
    return wl_ioctl_at(cmd, buf, len, 1);
}
static int wlu_get_at(int cmd, char *buf, uint32 len)
{
    return wl_ioctl_at(cmd, buf, len, 0);
}

static int wlu_iovar_set_at(char *valuename, char *value, uint32 valuelen)
{
    char smbuf[WLC_IOCTL_SMLEN*2];
    uint32 buflen = WLC_IOCTL_SMLEN*2;
    uint32 val_len = strlen(valuename) + 1;
    if (buflen < val_len + sizeof(int))
    {
        return BCME_ERROR;
    }
    memset(smbuf, 0, buflen);
    memcpy(&smbuf[val_len], value, valuelen);
    snprintf(smbuf, val_len, "%s", valuename);
    return wlu_set_at(WLC_SET_VAR, smbuf, buflen);
}

static int wl_var_setint_at(char *valuename, int *value)
{
    return wlu_iovar_set_at(valuename, value, sizeof(int));
}

static int wl_var_getint_at(char *valuename, int *value)
{
    int ret = 0;
    char smbuf[WLC_IOCTL_SMLEN];
    uint32 val_len = strlen(valuename) + 1;
    if (WLC_IOCTL_SMLEN < val_len)
    {
        WLAN_TRACE_ERROR("%s,buf too short\n",__FUNCTION__);
        return BCME_ERROR;
    }
    memset(smbuf, 0, WLC_IOCTL_SMLEN);
    snprintf(smbuf, val_len, "%s", valuename);
    ret = wlu_get_at(WLC_GET_VAR, smbuf, WLC_IOCTL_SMLEN);
    memcpy(value, smbuf, sizeof(int));
    return ret;
}
static int wl_varint_at(char *valuename, int *value, bool set)
{
    if (set)
        return wl_var_setint_at(valuename, value);
    else
        return wl_var_getint_at(valuename, value);
}



/*******************************************************************************
        wl命令实现接口，提供给AT模块
*******************************************************************************/

/*****************************************************************************
 函数名称  : int wl_ver(void)
 功能描述  : 查询wl ver的值，检查控制通路是否OK
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_ver(void)
{
    int ret = 0;
    memset(smbuf, 0, WLC_IOCTL_SMLEN);
    strncpy(smbuf, "ver", WLC_IOCTL_SMLEN);
    ret = wlu_get_at(WLC_GET_VAR, smbuf, WLC_IOCTL_SMLEN);
    return ret;
}

/*****************************************************************************
 函数名称  : int down(void)
 功能描述  : 关闭WiFi射频,wl down
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_down(void)
{
    int ret = 0;
    ret = wlu_set_at(WLC_DOWN, NULL, 0);
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_ver(void)
 功能描述  : 打开WiFi射频，wl up
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_up(void)
{
    int ret = 0;
    ret = wlu_set_at(WLC_UP, NULL, 0);
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_scansuppress(int scansuppress)
 功能描述  : 设置int wl_scansuppress的值
 输入参数  : scansuppress
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_scansuppress(int scansuppress)
{
    int value = scansuppress;
    return wlu_set_at(WLC_SET_SCANSUPPRESS, &value, sizeof(int));
}

/*****************************************************************************
 函数名称  : int wl_country(char *country)
 功能描述  : 设置国家码
 输入参数  : country
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_country(char *country)
{
    int ret = 0;
    if (NULL == country)
    {
        WLAN_TRACE_ERROR("wl_country null point error!\n");
        return BCME_ERROR;
    }
    char value[WLC_CNTRY_BUF_SZ] = {0};
    memset(value, 0, sizeof(WLC_CNTRY_BUF_SZ));
    memcpy(value, country, WLC_CNTRY_BUF_SZ);
    ret = wlu_iovar_set_at("country", value, WLC_CNTRY_BUF_SZ);
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_band(uint32 band)
 功能描述  : 设置WiFi的band，比如wl band b
 输入参数  : band    取值范围是WLC_BAND_2G    WLC_BAND_5G
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_band(uint32 band)
{
    int ret = 0;
    uint32 value = band;
    ret = wlu_set_at(WLC_SET_BAND, &value, sizeof(uint32));
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_channel(int channel)
 功能描述  : 设置WiFi信道
 输入参数  : channel
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_channel(int channel)
{
    int ret = 0;
    int value = channel;
    ret = wlu_set_at(WLC_SET_CHANNEL, &value, sizeof(int));
    return ret;
}

static const uint8 wf_5g_40m_chans[] =
{38, 46, 54, 62, 102, 110, 118, 126, 134, 142, 151, 159};    //5G 40M带宽下的信道号
#define WF_NUM_5G_40M_CHANS \
    (sizeof(wf_5g_40m_chans)/sizeof(uint8))

static const uint8 wf_5g_80m_chans[] =
{42, 58, 106, 122, 138, 155};    //5G 80M带宽下的信道号
#define WF_NUM_5G_80M_CHANS \
    (sizeof(wf_5g_80m_chans)/sizeof(uint8))

static const uint8 wf_5g_160m_chans[] =
{50, 114};    //5G 160M带宽下的信道号
#define WF_NUM_5G_160M_CHANS \
    (sizeof(wf_5g_160m_chans)/sizeof(uint8))

static uint8
center_chan_to_edge(uint bw)
{
    return (uint8)(((bw - 20) / 2) / 5);
}


static uint8
channel_low_edge(uint center_ch, uint bw)
{
    return (uint8)(center_ch - center_chan_to_edge(bw));
}


static int
channel_to_sb(uint center_ch, uint ctl_ch, uint bw)
{
    uint lowest = channel_low_edge(center_ch, bw);
    uint sb;

    if ((ctl_ch - lowest) % 4) {
        
        return BCME_ERROR;
    }

    sb = ((ctl_ch - lowest) / 4);

    
    if (sb >= (bw / 20)) {
        
        return BCME_ERROR;
    }

    return sb;
}

static int
channel_80mhz_to_id(uint ch)
{
    uint i;
    for (i = 0; i < WF_NUM_5G_80M_CHANS; i ++) {
        if (ch == wf_5g_80m_chans[i])
            return i;
    }

    return BCME_ERROR;
}

/*****************************************************************************
 函数名称  : chanspec_t wf_chspec_parse(uint32 ctl_ch, uint32 bw, char ul,\
                 uint32 ch1, uint32 ch2)
 功能描述  : 解析chanspec后面的参数
 输入参数  : ctl_ch    控制信道
             bw    带宽，取值范围是20、40、80、160、8080
             ul    40M带宽时主信道偏移，取值范围是u、l
             ch1、ch2    8080M带宽时两个信道的值
 输出参数  : NA
 返 回 值  : chanspec_t
 其他说明  : 
*****************************************************************************/

chanspec_t wf_chspec_parse(uint32 ctl_ch, uint32 bw, char ul, uint32 ch1, uint32 ch2)
{
    chanspec_t chspec;
    uint chspec_ch, chspec_band, chspec_bw, chspec_sb;
    int i;
    chspec_sb = 0;
    chspec_ch =0;
    const uint8 *center_ch = NULL;
    int num_ch = 0;
    int sb = -1;
    int ch1_id = 0, ch2_id = 0;
    chspec_band = ((ctl_ch <= CH_MAX_2G_CHANNEL) ?
        WL_CHANSPEC_BAND_2G : WL_CHANSPEC_BAND_5G);
    chspec_bw = bw;
    switch(bw)
    {
        case WL_CHANSPEC_BW_20:
            chspec_ch = ctl_ch;
            chspec_sb = WL_CHANSPEC_CTL_SB_NONE;
            break;

        case WL_CHANSPEC_BW_40:
            if ('l' == ul)
            {
                chspec_ch = UPPER_20_SB(ctl_ch);
                chspec_sb = WL_CHANSPEC_CTL_SB_LLL;
            }
            else if ('u' == ul)
            {
                chspec_ch = LOWER_20_SB(ctl_ch);
                chspec_sb = WL_CHANSPEC_CTL_SB_LLU;
            }
            else//如果既不是l也不是u，则是一个20M带宽的信道
            {
                chspec_bw = WL_CHANSPEC_BW_20;
                chspec_ch = ctl_ch;
                chspec_sb = WL_CHANSPEC_CTL_SB_NONE;
            }
            break;
        case WL_CHANSPEC_BW_80:
            center_ch = wf_5g_80m_chans;
            num_ch = WF_NUM_5G_80M_CHANS;
            for (i = 0; i < num_ch; i ++)
            {
                sb = channel_to_sb(center_ch[i], ctl_ch, 80);//80M带宽
                if (sb >= 0)
                {
                    chspec_ch = center_ch[i];
                    chspec_sb = sb << WL_CHANSPEC_CTL_SB_SHIFT;
                    break;
                }
            }
            if (sb < 0)
            {
                return 0;
            }
            break;
        case WL_CHANSPEC_BW_160:
            center_ch = wf_5g_160m_chans;
            num_ch = WF_NUM_5G_160M_CHANS;
            for (i = 0; i < num_ch; i ++)
            {
                sb = channel_to_sb(center_ch[i], ctl_ch, 160);//160M带宽
                if (sb >= 0)
                {
                    chspec_ch = center_ch[i];
                    chspec_sb = sb << WL_CHANSPEC_CTL_SB_SHIFT;
                    break;
                }
            }
            if (sb < 0)
            {
                return 0;
            }
            break;
        case WL_CHANSPEC_BW_8080:
            ch1_id = channel_80mhz_to_id(ch1);
            ch2_id = channel_80mhz_to_id(ch2);
            if (ch1 >= ch2 || ch1_id < 0 || ch2_id < 0)
                return 0;

        
            chspec_ch = (((uint16)ch1_id << WL_CHANSPEC_CHAN1_SHIFT) |
                ((uint16)ch2_id << WL_CHANSPEC_CHAN2_SHIFT));

            sb = channel_to_sb(ch1, ctl_ch, 80);//80M带宽
            if (sb < 0) {
            
                sb = channel_to_sb(ch2, ctl_ch, 80);//80M带宽
                if (sb < 0) {
                
                    return 0;
                }
            
                sb += 4;   //博通原始算法
            }
            chspec_sb = sb << WL_CHANSPEC_CTL_SB_SHIFT;
            break;

        default:
            return 0;
    }

    chspec = (chspec_ch | chspec_band | chspec_bw | chspec_sb);
    WLAN_TRACE_ERROR("parse_chanspec:%x\n",chspec);
    return chspec;
}

/*****************************************************************************
 函数名称  : int wl_chanspec(uint32 channel, uint32 bw, char ul,\
                 uint32 ch1, uint32 ch2)
 功能描述  : 设置WiFi信道
 输入参数  : channel    控制信道
             bw    带宽
             ul    40M带宽时控制信道的偏移
             ch1、ch2    8080M带宽时两个信道的值
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_chanspec(uint32 channel, uint32 bw, char ul, uint32 ch1, uint32 ch2)
{
    int ret = 0;
    chanspec_t chanspec = wf_chspec_parse(channel, bw, ul, ch1, ch2);
    uint32 val = htod32((uint32)chanspec);
    ret = wl_var_setint_at("chanspec", &val);
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_status(void)
 功能描述  : 获取WiFi状态，相当于wl status,查询结果不返回给调用函数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_status(void)
{
    int ret = 0;
    struct ether_addr bssid;
    wlc_ssid_t ssid;
    char ssidbuf[SSID_FMT_BUF_LEN];
    wl_bss_info_t *bi;

    if ((ret = wlu_get_at(WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN)) == 0)
    {
        /* The adapter is associated. */
        memset(long_buf, 0, WLC_IOCTL_MAXLEN);
        *(uint32*)long_buf = htod32(WLC_IOCTL_MAXLEN);
        if ((ret = wlu_get_at(WLC_GET_BSS_INFO, long_buf, WLC_IOCTL_MAXLEN)) < 0)
        {
            return ret;
        }
    }
    else
    {
        if ((ret = wlu_get_at(WLC_GET_SSID, &ssid, sizeof(wlc_ssid_t))) < 0)
        {
            WLAN_TRACE_ERROR("WLC_GET_SSID error:%d\n", ret);
            return ret;
        }
    }

    return ret;
}

/*****************************************************************************
 函数名称  : int wl_rate(int rate)
 功能描述  : 设置WiFi的速率，a、b、g模式采用这个函数设置
 输入参数  : rate，原本的rateX2的值
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_rate(int rate)
{
    const char* fn_name = "wl_rate_mrate";
    int ret;
    int val;
    int band;
    int list[3];
    char aname[sizeof("5g_mrate") + 1];
    char bgname[sizeof("2g_mrate") + 1];
    char *name;

    snprintf(aname, (sizeof("5g_mrate") + 1), "5g_%s", "rate");
    snprintf(bgname, (sizeof("2g_mrate") + 1), "2g_%s", "rate");
    rate = rate * 2 /100;//入参的rate值是没有除以100和X2的操作的，需要在这里执行

    if ((ret = wlu_get_at(WLC_GET_BAND, &band, sizeof(uint))) < 0)
        return ret;
    band = dtoh32(band);

    if ((ret = wlu_get_at(WLC_GET_BANDLIST, list, sizeof(list))) < 0)
        return ret;
    list[0] = dtoh32(list[0]);
    list[1] = dtoh32(list[1]);
    list[2] = dtoh32(list[2]);

    if (!list[0])
        return BCME_ERROR;
    else if (list[0] > 2)    //博通原始算法
        list[0] = 2;     //博通原始算法

    switch (band)
    {
        case WLC_BAND_AUTO :
        if (list[0] > 1)
        {
            return BCME_BADARG;
        }
        else if (list[1] == WLC_BAND_5G)
            name = (char *)aname;
        else if (list[1] == WLC_BAND_2G)
            name = (char *)bgname;
        else
            return BCME_ERROR;
        break;

        case WLC_BAND_5G :
            name = (char *)aname;
            break;

        case WLC_BAND_2G :
            name = (char *)bgname;
            break;

        default :
            return BCME_ERROR;
    }
    val = WL_RSPEC_ENCODE_RATE | rate;
    val = htod32(val);
    ret = wl_var_setint_at(name, &val);

    return ret;
}

/*****************************************************************************
 函数名称  : int wl_rate(int rate)
 功能描述  : 设置WiFi的速率，n模式采用这个函数设置
 输入参数  : mcs
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 只支持AT模块用到的-m参数，输入参数mcs几位-m的值
             不支持auto
*****************************************************************************/

int wl_nrate(int mcs)
{
    int ret;
    int nrate = 0;

    nrate = WL_RSPEC_ENCODE_HT;    /* 11n HT */
    nrate |= mcs;
    ret = wl_var_setint_at("nrate", &nrate);
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_5g_rate(int mcs, int Nss)
 功能描述  : 设置WiFi的速率，ac模式采用这个函数设置
 输入参数  : mcs、Nss
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_5g_rate(int mcs, int Nss)
{
    int ret;
    uint32 rspec = 0;
    rspec = WL_RSPEC_ENCODE_VHT;    /* 11ac VHT */
    if (Nss == 0)
    {
        Nss = 1; /* default Nss = 1 if --ss option not given */
    }
    rspec |= (Nss << WL_RSPEC_VHT_NSS_SHIFT) | mcs;

    ret = wl_var_setint_at("5g_rate", &rspec);
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_txpwr1(int pwr)
 功能描述  : 设置WiFi的功率，单位是dbm
 输入参数  : pwr    功率值，就是wl txpwr1后面-o的参数值
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 输入参数-o表示pwr，-d表示单位是dbm，我们的代码中本来传入的就是dbm
*****************************************************************************/

int wl_txpwr1(int pwr)
{
    int ret, new_val = 0;
    const char *name = "qtxpower";
    new_val = pwr * 4;   //博通原始算法
    new_val |= WL_TXPWR_OVERRIDE;
    ret = wl_var_setint_at(name, &new_val);
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_fqacurcy(int freq)
 功能描述  : 设置WiFi工作的频率
 输入参数  : freq    功率值，就是wl fqacurcy的参数值
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_fqacurcy(int freq)
{
    int ret;
    int val;
    val = freq;
    ret = wlu_set_at(WLC_FREQ_ACCURACY, &val, sizeof(int));
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_out(void)
 功能描述  : 执行wl out
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_out(void)
{
    return wlu_set_at(WLC_OUT, NULL, 0);
}

/*****************************************************************************
 函数名称  : int wl_phy_txpwrctrl(int pwrctl)
 功能描述  : 执行wl phy_txpwrctrl
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_phy_txpwrctrl(int pwrctl)
{
    int val = pwrctl;
    return wl_var_setint_at("phy_txpwrctrl", &val);
}

/*****************************************************************************
 函数名称  : int wl_phy_txpwrindex(uint32 arg1, uint32 arg2)
 功能描述  : 执行wl phy_txpwrindex
 输入参数  : arg1、arg2
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 只支持两个参数的输入
*****************************************************************************/

int wl_phy_txpwrindex(uint32 arg1, uint32 arg2)
{
    int ret;
    uint32 txpwridx[4] = { 0 };
    int8 idx[4] = { 0 };
    uint argc;
    char *endptr;
    wlc_rev_info_t revinfo;
    uint32 phytype;
    txpwridx[0] = arg1;
    txpwridx[1] = arg2;
    memset(&revinfo, 0, sizeof(revinfo));
    if ((ret = wlu_get_at(WLC_GET_REVINFO, &revinfo, sizeof(revinfo))) < 0)
        return ret;

    phytype = dtoh32(revinfo.phytype);

    if (phytype == WLC_PHY_TYPE_HT)
    {
        WLAN_TRACE_ERROR("only support two args\n");
        return BCME_USAGE_ERROR;
    }

    ret = wlu_iovar_set_at("phy_txpwrindex", txpwridx, 4*sizeof(uint32));
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_pkteng_stop(int tx_or_rx)
 功能描述  : 停止收包或发包
 输入参数  : tx_or_rx,只支持PKTENG_RX和PKTENG_TX
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_pkteng_stop(int tx_or_rx)
{
    int ret;
    wl_pkteng_t pkteng;
    memset(&pkteng, 0, sizeof(pkteng));
    if (tx_or_rx == PKTENG_TX)
        pkteng.flags = WL_PKTENG_PER_TX_STOP;
    else if (tx_or_rx == PKTENG_RX)
        pkteng.flags = WL_PKTENG_PER_RX_STOP;
    else
        return BCME_USAGE_ERROR;
    pkteng.flags = htod32(pkteng.flags);
    ret = wlu_iovar_set_at("pkteng", &pkteng, sizeof(pkteng));
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_pkteng_start(struct ether_addr *dest, int tx_or_rx,
                 uint32 delay, uint32 length, uint32 nframes)
 功能描述  : 开始收包或发包
 输入参数  : tx_or_rx,只支持PKTENG_RX和PKTENG_TX
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_pkteng_start(struct ether_addr *dest, int tx_or_rx, uint32 delay, uint32 length, uint32 nframes)
{
    int ret;
    wl_pkteng_t pkteng;
    if (NULL == dest)
    {
        WLAN_TRACE_ERROR("%s, dest addr is null !\n", __FUNCTION__);
        return BCME_ERROR;
    }
    memset(&pkteng, 0, sizeof(pkteng));
    memcpy(&pkteng.dest, dest->octet, MAC_BYTE_MAX);
    if (tx_or_rx == PKTENG_TX)
    {
        pkteng.flags = WL_PKTENG_PER_TX_START;
        pkteng.delay = delay;
        pkteng.length = length;
        pkteng.nframes = nframes;
    }
    else if (tx_or_rx == PKTENG_RX)
    {
        pkteng.flags = WL_PKTENG_PER_RX_START;
        pkteng.nframes = nframes;
        pkteng.delay = delay;
    }
    else
        return BCME_USAGE_ERROR;
    pkteng.flags = htod32(pkteng.flags);
    pkteng.delay = htod32(pkteng.delay);
    pkteng.nframes = htod32(pkteng.nframes);
    pkteng.length = htod32(pkteng.length);
    ret = wlu_iovar_set_at("pkteng", &pkteng, sizeof(pkteng));
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_mimo_bw_cap(int cap)
 功能描述  : 设置mimo_bw_cap的值
 输入参数  : cap，mimo_bw_cap的值
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_mimo_bw_cap(int cap)
{
    int ret;
    int val = cap;
    ret = wl_var_setint_at("mimo_bw_cap", &val);
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_mimo_txbw(int txbw)
 功能描述  : 设置mimo_txbw的值
 输入参数  : txbw，mimo_txbw的值
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_mimo_txbw(int txbw)
{
    int ret;
    int val = txbw;
    ret = wl_var_setint_at("mimo_txbw", &val);
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_phy_forcecal(int phy_forcecal)
 功能描述  : 设置phy_forcecal的值
 输入参数  : phy_forcecal，phy_forcecal的值
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_phy_forcecal(int phy_forcecal)
{
    int ret;
    int val = phy_forcecal;
    ret = wl_var_setint_at("phy_forcecal", &val);
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_phy_watchdog(int watchdog)
 功能描述  : 设置phy_watchdog的值
 输入参数  : watchdog，phy_watchdog的值
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_phy_watchdog(int watchdog)
{
    int ret;
    int val = watchdog;
    ret = wl_var_setint_at("phy_watchdog", &val);
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_PM(int PM)
 功能描述  : 设置PM的值
 输入参数  : PM，PM的值
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_PM(int PM)
{
    int ret;
    int val = PM;
    ret = wlu_set_at(WLC_SET_PM, &val, sizeof(int));
    return ret;
}

/*****************************************************************************
 函数名称  : wlan_at_get_packet_report
 功能描述  : 获取wifi接收机接收包个数
 输入参数  : char * pValueStr
 输出参数  : NA             
 返 回 值  : uiRetPcktsNumBuf
 其他说明  : 
*****************************************************************************/
void wlan_at_get_packet_report(unsigned int total, unsigned int bad, unsigned int ucast, unsigned int mcast)
{
    WLAN_TRACE_INFO("%s,Enter [Total=%d,Bad=%d,Ucast=%d,Mcast=%d]\n", __FUNCTION__,total, bad, ucast, mcast);
    g_wifi_packet_new_rep.TotalRxPkts = total;
    g_wifi_packet_new_rep.BadRxPkts = bad;
    g_wifi_packet_new_rep.UcastRxPkts = ucast;
    g_wifi_packet_new_rep.McastRxPkts = mcast;
}

/*****************************************************************************
 函数名称  : int wl_counters(void)
 功能描述  : 查询WiFi的收发包数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_counters(void)
{
    int ret = 0;
    int len;
    wl_cnt_t *cnt = NULL;
    memset(long_buf, 0, WLC_IOCTL_MEDLEN);
    strncpy(long_buf, "counters", WLC_IOCTL_MAXLEN);

    len = strlen("counters") + 1;
    ret = wlu_get_at(WLC_GET_VAR, long_buf, WLC_IOCTL_MEDLEN);
    if (ret)
    {
        WLAN_TRACE_ERROR("get_counters failed:%d\n", ret);
        return ret;
    }
    cnt = (wl_cnt_t *)long_buf;
    wlan_at_get_packet_report(cnt->rxframe, cnt->rxerror, cnt->pktengrxducast, cnt->pktengrxdmcast);
    return 0;
}

/*****************************************************************************
 函数名称  : int wl_radioreg(uint32 pos, uint32 value, char *name)
 功能描述  : 设置radioreg 的值
 输入参数  : pos、value、name，分别对应wl_radioreg(0x8c, 0x490, "pll")调用里面的
             三个参数
 输出参数  : NA
 返 回 值  : 0 成功
             -1 失败
 其他说明  : 
*****************************************************************************/

int wl_radioreg(uint32 pos, uint32 value, char *name)
{
    int reg = pos;
    int ret;
    struct {
        int val;
        int band;
    } x;
    bool core_cmd;
    wlc_rev_info_t revinfo;
    uint32 phytype;

    x.val = 0;
    x.band = WLC_BAND_AUTO;
    core_cmd = 0;

    memset(&revinfo, 0, sizeof(revinfo));
    ret = wlu_get_at(WLC_GET_REVINFO, &revinfo, sizeof(revinfo));
    if (ret) {
        return ret;
    }
    phytype = dtoh32(revinfo.phytype);

    if (!strcmp(name, "a"))
        x.band = WLC_BAND_5G;
    else if (!strcmp(name, "b"))
        x.band = WLC_BAND_2G;
    else
    {
        core_cmd = 0;
        if (strcmp(name, "syn") == 0)
        {
            reg |= RADIO_CORE_SYN;
            core_cmd = 1;
        }
        else if (strcmp(name, "tx0") == 0)
        {
            reg |= RADIO_CORE_TX0;
            core_cmd = 1;
        }
        else if (strcmp(name, "tx1") == 0)
        {
            reg |= RADIO_CORE_TX1;
            core_cmd = 1;
        }
        else if (strcmp(name, "rx0") == 0)
        {
            reg |= RADIO_CORE_RX0;
            core_cmd = 1;
        }
        else if (strcmp(name, "rx1") == 0)
        {
            reg |= RADIO_CORE_RX1;
            core_cmd = 1;
        }
            /* For HTPHY/ACPHY, the 3rd argument can be
               the radio core
            */
        if (phytype == WLC_PHY_TYPE_AC)
        {
            if (strcmp(name, "cr0") == 0)
            {
                reg |= RADIO_2069_CORE_CR0;
                core_cmd = 1;
            }
            else if (strcmp(name, "cr1") == 0)
            {
                reg |= RADIO_2069_CORE_CR1;
                core_cmd = 1;
            }
            else if (strcmp(name, "cr2") == 0)
            {
                reg |= RADIO_2069_CORE_CR2;
                core_cmd = 1;
            }
            else if (strcmp(name, "pll") == 0)
            {
                reg |= RADIO_2069_CORE_PLL;
                core_cmd = 1;
            }
            else if (strcmp(name, "all") == 0)
            {
                reg |= RADIO_2069_CORE_ALL;
                core_cmd = 1;
            }
        }
        else
        {
            if (strcmp(name, "cr0") == 0)
            {
                reg |= RADIO_CORE_CR0;
                core_cmd = 1;
            }
            else if (strcmp(name, "cr1") == 0)
            {
                reg |= RADIO_CORE_CR1;
                core_cmd = 1;
            }
            else if (strcmp(name, "cr2") == 0)
            {
                reg |= RADIO_CORE_CR2;
                core_cmd = 1;
            }
            else if (strcmp(name, "all") == 0)
            {
                reg |= RADIO_CORE_ALL;
                core_cmd = 1;
            }
        }

        if (!core_cmd)
        {
            return BCME_USAGE_ERROR;
        }
        else
        {
            x.val = value;
        }
    }

    x.val = (x.val << 16) | (reg & 0xffff);
    x.band = htod32(x.band);
    x.val = htod32(x.val);
    ret = wlu_set_at(WLC_SET_RADIOREG, &x, sizeof(x));
    return (ret);
}

/*****************************************************************************
 函数名称  : int wl_txchain(int chain)
 功能描述  : 设置天线发送数量
 输入参数  : int chain
 输出参数  : NA             
 返 回 值  : ret
 其他说明  : 
*****************************************************************************/
int wl_txchain(int chain)
{
    int ret = 0;
    int value = chain;
    if (value)//set
    {
        ret = wl_varint_at("txchain", &value, 1);
    }
    else
    {
        ret = wl_varint_at("txchain", &value, 0);
        WLAN_TRACE_ERROR("wl_txchain, chain = %d\n",value);
    }
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_rxchain(int chain)
 功能描述  : 设置天线接收数量
 输入参数  : int chain
 输出参数  : NA             
 返 回 值  : ret
 其他说明  : 
*****************************************************************************/

int wl_rxchain(int chain)
{
    int ret = 0;
    int value = chain;
    if (value)//set
    {
        ret = wl_varint_at("rxchain", &value, 1);
    }
    else
    {
        ret = wl_varint_at("rxchain", &value, 0);
        WLAN_TRACE_ERROR("wl_rxchain, chain = %d\n",value);
    }
    return ret;
}

/*****************************************************************************
 函数名称  : int wl_mpc(int mpc)
 功能描述  : 设置mpc的值，相当于wl mpc
 输入参数  : mpc
 输出参数  : NA             
 返 回 值  : ret
 其他说明  : 
*****************************************************************************/

int wl_mpc(int mpc)
{
    int ret = 0;
    int value = mpc;
    ret = wl_varint_at("mpc", &value, 1);
    return ret;
}
/*****************************************************************************
 函数名称  : int wlu_ciswrite(int if_p_param, char *filepath)
 功能描述  : 通过命令设置OTP
 输入参数  : if_p_param:是否包含-p参数,0:不包含，1:包含。
             filepath:OTP文件路径
 输出参数  : NA             
 返 回 值  : ret
 其他说明  : 
*****************************************************************************/
int wlu_ciswrite(int if_p_param, char *filepath)
{
    char *bufp;
    struct file *fp = NULL;
    int  ret = 0;
    uint32 len = 0;
    mm_segment_t old_fs;

    cis_rw_t cish;
    char *cisp, *cisdata;
    
    if (!filepath)
        return BCME_USAGE_ERROR;

    WLAN_TRACE_INFO("wlu_ciswrite is enter!!!\n");

    memset((char*)&cish, 0, sizeof(cish));
    /* Grab and move past optional output file argument */
    if (if_p_param) {
        WLAN_TRACE_INFO("Writing in PCIe CIS format\n");

        cish.flags |= CISH_FLAG_PCIECIS;	/* write CIS format bit */
    }

    /* initialize buffer with iovar */
    bufp = long_buf;
    memset(long_buf, 0, WLC_IOCTL_MAXLEN);
    strncpy(bufp, "ciswrite", WLC_IOCTL_MAXLEN);
    bufp += strlen("ciswrite") + 1;
    cisp = bufp;
    cisdata = cisp + sizeof(cish);

    /* grab the filename arg */
    if (!(fp = filp_open(filepath, O_RDONLY, 0))) {
        WLAN_TRACE_ERROR("%s: No such file or directory\n", filepath);
        return BCME_BADARG;
    }
    
    old_fs = get_fs();
    set_fs(get_ds());
    
    len = fp->f_op->read(fp, cisdata, SROM_MAX + 1, &fp->f_pos);

    if (len <= 0) {
        WLAN_TRACE_ERROR("\nerror %d reading %s\n", ret, filepath);
        ret = BCME_ERROR;
        goto out;
    }

    if (len >= SROM_MAX) {
        WLAN_TRACE_ERROR("\nFile %s is too large\n", filepath);
        ret = BCME_ERROR;
        goto out;
    }

    /* Convert the endianess for non-zero fields */
    cish.flags = htod16(cish.flags);
    cish.nbytes = htod32(len); /* fill in length (offset is 0) */
    memcpy(cisp, (char*)&cish, sizeof(cish));

    WLAN_TRACE_INFO("len %d sizeof(cish) %d total %d\n", len, (int)sizeof(cish),
           (int)(len + sizeof(cish)));
    ret = wlu_set_at(WLC_SET_VAR, long_buf, (cisp - (char*)long_buf) + sizeof(cish) + len);
    if (ret < 0) {
        WLAN_TRACE_ERROR("ciswrite failed: %d\n", ret);
    }
out:
    set_fs(old_fs);
    (void)filp_close(fp, NULL);

    return ret;
}

/*****************************************************************************
 函数名称  : int wlu_cisdump(char* out_buf, int *out_len)
 功能描述  : OTP配置文件查询
 输入参数  : out_buf:输出的buf缓冲区，大小为8192字节
             out_len:输出的buf缓冲区的长度。
             if_write:芯片是否已经被写入OTP
 输出参数  : 返回芯片中的OTP内容             
 返 回 值  : 成功或者失败
 其他说明  : 
*****************************************************************************/
int wlu_cisdump(unsigned char* out_buf, int *out_len, int* if_write)
{
    char *bufp;
    int ret = 0;
    cis_rw_t cish;
    uint nbytes = 0;
    WLAN_TRACE_INFO("wlu_cisdump is enter!!\n");

    if(NULL == out_buf)
    {
        WLAN_TRACE_ERROR("out_buf is NULL!!\n");
        goto done;
    }

    /* Prepare the read info */
    memset((char*)&cish, 0, sizeof(cish));
    cish.nbytes = htod32(nbytes);

    /* set up the buffer and do the get */
    memset(out_buf, 0, WLC_IOCTL_MAXLEN);
    strncpy(out_buf, "cisdump", WLC_IOCTL_MAXLEN);
    bufp = out_buf + strlen("cisdump") + 1;
    memcpy(bufp, (char*)&cish, sizeof(cish));
    bufp += sizeof(cish);
    ret = wlu_get_at(WLC_GET_VAR, (void *)out_buf, (bufp - (char*)out_buf) + SROM_MAX);
    if (ret < 0) {
        WLAN_TRACE_ERROR("Failed cisdump request: %d\n", ret);
        goto done;
    }

    /* pull off the cis_rw_t */
    bufp = out_buf;
    memcpy((char*)&cish, bufp, sizeof(cish));
    cish.source = dtoh16(cish.source);
    cish.flags = dtoh16(cish.flags);
    cish.byteoff = dtoh32(cish.byteoff);
    cish.nbytes = dtoh32(cish.nbytes);

    /* move past to the data */
    bufp += sizeof(cish);

    WLAN_TRACE_INFO("Source: %d (%s)", cish.source,
           (cish.source == WLC_CIS_DEFAULT) ? "Built-in default" :
           (cish.source == WLC_CIS_SROM) ? "External SPROM" :
           (cish.source == WLC_CIS_OTP) ? "Internal OTP" : "Unknown?");
    if (!nbytes)
        WLAN_TRACE_INFO("\nMaximum length: %u bytes", cish.nbytes);
    *out_len = (int)cish.nbytes;
    if((int)cish.nbytes >= WLC_IOCTL_MAXLEN || (int)cish.nbytes <= 0)
        return BCME_ERROR;
    if(0xFF == (uint8)bufp[cish.nbytes - 1])  //如果最后一个字节是0xFF，说明OTP文件已经写入成功
    {
        WLAN_TRACE_INFO("OTP has writen to chip!!!\n");
        *if_write = 1;
    }
    else
    {
        WLAN_TRACE_INFO("OTP has not writen to chip,begin to write!!\n");
        *if_write = 0;
    }
done:
    return ret;
}


