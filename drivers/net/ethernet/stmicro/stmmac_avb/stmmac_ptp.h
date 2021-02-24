/******************************************************************************
  PTP Header file

  Copyright (C) 2013  Vayavya Labs Pvt Ltd

 * 2016-2-18 - Modifed code to adapt Synopsys DesignWare Cores Ethernet 
 * Quality-of-Service (DWC_ether_qos) core, 4.10a. 
 * liufangyuan <liufangyuan2@huawei.com>
 * Copyright (C) Huawei Technologies Co., Ltd.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Author: Rayagond Kokatanur <rayagond@vayavyalabs.com>
******************************************************************************/

#ifndef __STMMAC_PTP_H__
#define __STMMAC_PTP_H__

#define STMMAC_SYSCLOCK 62500000

/* IEEE 1588 PTP register offsets */
#define PTP_TCR		0x0700	/* Timestamp Control Reg */
#define PTP_SSIR	0x0704	/* Sub-Second Increment Reg */
#define PTP_STSR	0x0708	/* System Time Seconds Regr */
#define PTP_STSUR	0x0710	/* System Time Seconds Update Reg */
#define PTP_TAR		0x0718	/* Timestamp Addend Reg */
#define PTP_TTSR	0x071C	/* Target Time Seconds Reg */
#define PTP_TTNSR	0x0720	/* Target Time Nanoseconds Reg */
#define	PTP_STHWSR	0x0724	/* System Time - Higher Word Seconds Reg */
#define PTP_TSR		0x0728	/* Timestamp Status */

/* MAC timestamp related registers */
#define GMAC_TIMESTAMP_CONTROL                 0x0B00	/* Timestamp Control Reg */
#define GMAC_SUB_SECOND_INCREMENT              0x0B04	/* Sub-Second Increment Reg */
#define GMAC_SYSTEM_TIME_SECONDS               0x0B08	/* System Time Seconds Regr */
#define GMAC_SYSTEM_TIME_NANOSECONDS           0x0B0C	/* System Time Nanoseconds Reg */
#define GMAC_SYSTEM_TIME_SECONDS_UPDATE        0x0B10	/* System Time Seconds Update Reg */
#define GMAC_SYSTEM_TIME_NANOSECONDS_UPDATE    0x0B14	/* System Time Nanoseconds Update Reg */
#define GMAC_TIMESTAMP_ADDEND                  0x0B18	/* Timestamp Addend Reg */
#define GMAC_SYSTEM_TIME_HIGHER_WORD_SECONDS   0x0B1C	/* System Time - Higher Word Seconds Reg */
#define GMAC_TIMESTAMP_STATUS                  0x0B20 	/* Timestamp Status */
#define GMAC_TXTIMESTAMP_STATUS_NANOSECONDS    0x0B30 
#define GMAC_TXTIMESTAMP_STATUS_SECONDS        0x0B34 
#define GMAC_AUXILIARY_CONTROL                 0x0B40 
#define GMAC_AUXILIARY_TIMESTAMP_NANOSECONDS   0x0B48 
#define GMAC_AUXILIARY_TIMESTAMP_SECONDS       0x0B4C 
#define GMAC_TIMESTAMP_INGRESS_ASYM_CORR       0x0B50 
#define GMAC_TIMESTAMP_EGRESS_ASYM_CORR        0x0B54 
#define GMAC_TIMESTAMP_INGRESS_CORR_NANOSECOND 0x0B58 
#define GMAC_TIMESTAMP_EGRESS_CORR_NANOSECOND  0x0B5C 
#define GMAC_PPS_CONTROL                       0X0B70 
#define GMAC_PPS0_TARGET_TIME_SECONDS          0X0B80 
#define GMAC_PPS0_TARGET_TIME_NANOSECONDS      0X0B84 
#define GMAC_PPS0_INTERVAL                     0X0B88 
#define GMAC_PPS0_WIDTH                        0X0B8C 
#define GMAC_PPS1_TARGET_TIME_SECONDS          0X0B90 
#define GMAC_PPS1_TARGET_TIME_NANOSECONDS      0X0B94 
#define GMAC_PPS1_INTERVAL                     0X0B98 
#define GMAC_PPS1_WIDTH                        0X0B9C 
#define GMAC_PPS2_TARGET_TIME_SECONDS          0X0BA0 
#define GMAC_PPS2_TARGET_TIME_NANOSECONDS      0X0BA4 
#define GMAC_PPS2_INTERVAL                     0X0BA8 
#define GMAC_PPS2_WIDTH                        0X0BAC 
#define GMAC_PPS3_TARGET_TIME_SECONDS          0X0BB0 
#define GMAC_PPS3_TARGET_TIME_NANOSECONDS      0X0BB4 
#define GMAC_PPS3_INTERVAL                     0X0BB8 
#define GMAC_PPS3_WIDTH                        0X0BBC 
#define GMAC_PTO_CONTROL                       0X0BC0 
#define GMAC_SOURCE_PORT_IDENTITY0             0X0BC4 
#define GMAC_SOURCE_PORT_IDENTITY1             0X0BC8 
#define GMAC_SOURCE_PORT_IDENTITY2             0X0BCC 
#define GMAC_LOG_MESSAGE_INTERVAL              0X0BD0 

#define GMAC_STNSUR_ADDSUB_SHIFT 31

/* GMAC PTP defines */
#define GMAC_SSINC	0x002b0000	/* Sub-second Increment Value:0x2B, PTP clock is 50 MHz*/
#define GMAC_SNSINC	0x00000000	/* Sub-nanosecond Increment Value:0 */
#define SSINC_BIT	0x00FF0000	/* SSINC bit field */
#define SNSINC_BIT	0x0000FF00	/* SNSINC bit field */

/* GMAC TCR defines */
#define GMAC_TCR_TSENA		0x00000001 /* Timestamp Enable */
#define GMAC_TCR_TSCFUPDT	0x00000002 /* Timestamp Fine/Coarse Update */
#define GMAC_TCR_TSINIT		0x00000004 /* Timestamp Initialize */
#define GMAC_TCR_TSUPDT		0x00000008 /* Timestamp Update */
/* Timestamp Interrupt Trigger Enable */
#define GMAC_TCR_TSTRIG		0x00000010
#define GMAC_TCR_TSADDREG	0x00000020 /* Addend Reg Update */
#define PTP_TCR_TSENALL		0x00000100 /* Enable Timestamp for All Frames */
/* Timestamp Digital or Binary Rollover Control */
#define GMAC_TCR_TSCTRLSSR	0x00000200

/* Enable PTP packet Processing for Version 2 Format */
#define PTP_TCR_TSVER2ENA	0x00000400
/* Enable Processing of PTP over Ethernet Frames */
#define PTP_TCR_TSIPENA		0x00000800
/* Enable Processing of PTP Frames Sent over IPv6-UDP */
#define PTP_TCR_TSIPV6ENA	0x00001000
/* Enable Processing of PTP Frames Sent over IPv4-UDP */
#define PTP_TCR_TSIPV4ENA	0x00002000
/* Enable Timestamp Snapshot for Event Messages */
#define PTP_TCR_TSEVNTENA	0x00004000
/* Enable Snapshot for Messages Relevant to Master */
#define PTP_TCR_TSMSTRENA	0x00008000
/* Select PTP packets for Taking Snapshots */
#define PTP_TCR_SNAPTYPSEL_1	0x00010000
/* Enable MAC address for PTP Frame Filtering */
#define PTP_TCR_TSENMACADDR	0x00040000

/* GMAC_TIMESTAMP_CONTROL defines */
#define GMAC_TSC_TSENA		0x00000001
#define GMAC_TSC_TSCFUPDT	0x00000000		//coarse method
#define GMAC_TSC_TSINIT		0x00000004
#define GMAC_TSC_TSVER2ENA	0x00000400
#define GMAC_TSC_TSIPENA	0x00000800
#define	GMAC_TSC_TSMSTRENA	0x00008000
#define	GMAC_TSC_TSEVNTENA	0x00004000
#define GMAC_TSC_SNAPTYPSEL	0x00000000

#endif /* __STMMAC_PTP_H__ */
