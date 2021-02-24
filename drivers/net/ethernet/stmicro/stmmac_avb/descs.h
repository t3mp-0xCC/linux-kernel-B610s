/*******************************************************************************
  Header File to describe the DMA descriptors.
  Enhanced descriptors have been in case of DWMAC1000 Cores.

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

  Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
*******************************************************************************/

#ifndef __DESCS_H__
#define __DESCS_H__

/* All RX descriptors are prepared by the software and given to the DMA as "Normal" Descriptors
 * with the content as receive normal descriptor (Read Format )
 */

struct tx_nrd_desc {

	/* TDES0 */
	u32 buf1_phy_addr;

	/* TDES1 */
	u32 buf2_phy_addr;

	/* TDES2 */
	u32 buf1_len:14;
	u32 vlan_ins_rplc:2;		//VLAN Tag Insertion or Replacement
	u32 buf2_len:14;
	u32 time_stamp_enable:1;
	u32 ioc:1;

	/* TDES3 */
	u32 pkt_len:15;
	u32 rsvd1:1;
	u32 csum_ins_ctl:2;
	u32 tcp_seg_en:1;
	u32 slot_num:4;
	u32 sa_ins:3;
	u32 crc_pad:2;
	u32 last_desc:1;
	u32 first_desc:1;
	u32 ctx_type:1;
	u32 own:1;
};	

struct rx_nrd_desc {

	/* RDES0 */
	u32 buf1_phy_addr;
	
	/* RDES1
	 * In 64-bit addressing mode, this field contains the most-significant  
	 * 32 bits of the Buffer 1 Address Pointer. Otherwise, this field is reserved.
	*/
	u32 rsvd1;

	/* RDES2 */
	u32 buf2_phy_addr;

	/* RDES3 */
	u32 rsvd2:24;
	u32 buf1_addr_valid:1;
	u32 buf2_addr_valid:1;
	u32 rsvd3:4;
	u32 ioc:1;
	u32 own:1;
};

struct tx_nwb_desc {

	/* TDES0 */
	u32 tx_pkt_ts_low;

	/* TDES1 */
	u32 tx_pkt_ts_high;

	/* TDES2 */
	u32 rsvd1;

	/* TDES3 */
	u32 iphdr_err:1;
	u32 deferred:1;
	u32 underflow_err:1;
	u32 excessive_deferral:1;
	u32 collision_count:4;
	u32 excessive_collision:1;
	u32 late_collision:1;
	u32 no_carrier:1;
	u32 loc:1;
	u32 payload_csum_err:1;
	u32 pkt_flushed:1;
	u32 jabber_timeout:1;
	u32 err_sum:1;
	u32 rsvd2:1;
	u32 time_stamp_status:1;
	u32 rsvd3:10;
	u32 last_desc:1;
	u32 first_desc:1;
	u32 ctx_type:1;
	u32 own:1;
};

struct rd_nwb_desc {
				
	/* RDES0 */
	u32 outer_vlan_tag:16;
	u32 inner_valn_tag:16;
	
	/* RDES1 */
	u32 payload_type:3;
	u32 ip_hdr_err:1;
	u32 ipv4_hdr_dtt:1;
	u32 ipv6_hdr_dtt:1;
	u32 ip_csum_bypass:1;
	u32 ip_payload_err:1;
	u32 ptp_msg_type:4;
	u32 ptp_pkt_type:1;
	u32 ptp_ver:1;
	u32 ts_avail:1;
	u32 ts_dropped:1;
	u32 opc:16;

	/* RDES2 */
	u32 l3l4_hdr_len:10;
	u32 arp_nr:1;		//This bit is reserved when the Enable IPv4 ARP Offload option is not selected.
	u32 rsvd1:4;
	u32 vlan_filter_stat:1;
	u32 sa_filter_fail:1;
	u32 da_filter_fail:1;
	u32 hash_flt_stats:1;
	u32 mac_addr_match:8; //MAC Address Match or Hash Value, depend on hash_flt_stats
	u32 l3_flt_match:1;
	u32 l4_flt_match:1;
	u32 l3l4_fltnum_match:3;
	
	/* RDES3 */
	u32 pkt_len:15;
	u32 err_sum:1;
	u32 len_type_filed:3;
	u32 dribble_err:1;
	u32 receive_err:1;
	u32 overflow_err:1;
	u32 rw_timeout:1;	//receive watchdog timeout
	u32 giant_pkt:1;
	u32 crc_err:1;
	u32 rdes0_stat_valid:1;
	u32 rdes1_stat_valid:1;
	u32 rdes2_stat_valid:1;
	u32 last_desc:1;
	u32 first_desc:1;
	u32 rx_ctx_desc:1;
	u32 own:1;
};

struct tx_ctx_desc {

	/* TDES0 */
	u32 tx_pkt_ts_low;

	/* TDES1 */
	u32 tx_pkt_ts_high;

	/* TDES2 */
	u32 max_seg_size:14;
	u32 rsvd1:2;
	u32 inner_vlan_tag:16;

	/* TDES3 */
	u32 vlan_tag:16;
	u32 vlan_tag_valid:1;
	u32 inner_vt_valid:1;
	u32 inner_vt_ins_rep:2;
	u32 rsvd2:3;
	u32 ctx_desc_err:1;
	u32 rsvd3:2;
	u32 tcmmsv:1;	//One-Step Timestamp Correction Input or MSS Valid
	u32 ostc:1;		//One-Step Timestamp Correction Enable
	u32 rsvd4:2;
	u32 ctx_type:1;
	u32 own:1;
};

struct rx_ctx_desc {

	/* RDES0 */
	u32 rx_ts_low;

	/* RDES1 */
	u32 rx_ts_high;

	/* RDES2 */
	u32 rsvd1;

	/* RDES3 */
	u32 rsvd2:30;
	u32 rx_ctx_desc:1;
	u32 own:1;
};

union dma_desc {
	union {
		/* Receive normal descriptor with read format */
		struct rx_nrd_desc nrd;

		/* Receive normal descriptor with write-back format */
		struct rd_nwb_desc nwb;	

		/* Receive context descriptor */
		struct rx_ctx_desc ctx;
	} rx_desc;
	
	union {
		/* transmit normal descriptor with read format */
		struct tx_nrd_desc nrd;		
		
		/* transmit normal descriptor with write-back format */
		struct tx_nwb_desc nwb;
		
		/* transmit context descriptor */
		struct tx_ctx_desc ctx;
	} tx_desc;
};

/* Transmit checksum insertion control */
enum tdes_csum_insertion {
	cic_disabled = 0,	/* Checksum Insertion Control */
	cic_only_ip = 1,	/* Only IP header */
	/* IP header but pseudoheader is not calculated */
	cic_no_pseudoheader = 2,
	cic_full = 3,		/* IP header and pseudoheader */
};

#endif /* __DESCS_H__ */
