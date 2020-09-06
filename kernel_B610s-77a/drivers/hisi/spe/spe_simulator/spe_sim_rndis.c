/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * spe_sim_td.c -- spe simulator TD process
 *
 */
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>
#include <linux/kernel.h>
#include <linux/list.h>

#include "spe_sim.h"
#include "spe_sim_priv.h"
#include <linux/unaligned/le_struct.h>
#include <linux/if_ether.h>

#include <linux/device.h>
#include <linux/crc32.h>
#include <linux/kthread.h>
#include <linux/usb/cdc.h>
#include <net/ip.h>
#include "spe_sim_reg.h"

//wangzhongshun told me 
#define SPE_GNET_MAX_TRANSFER                   (0x4000) /* 16k */
#define SPE_GNET_IN_MAXSIZE_PER_TRANSFER        (SPE_GNET_MAX_TRANSFER)
#define SPE_GNET_PKT_LEN_WATERLINE              (2*1024)/* roy added for test 2014/05/07 */
#define SPE_GNET_PKT_NUM_WATERLINE              (64)

#define RNDIS_GNET_ALIGN_LEN					(cpu_to_le32(4))
#define REMOTE_NDIS_PACKET_MSG					0x00000001U



typedef unsigned __le32;
typedef unsigned short __le16;

struct rndis_packet_msg_type
{
	__le32	MessageType;
	__le32	MessageLength;
	__le32	DataOffset;
	__le32	DataLength;
	__le32	OOBDataOffset;
	__le32	OOBDataLength;
	__le32	NumOOBDataElements;
	__le32	PerPacketInfoOffset;
	__le32	PerPacketInfoLength;
	__le32	VcHandle;			/*resered1 in new rndis protocol--roy*/
	__le32	Reserved;
} __attribute__ ((packed));

#define RNDIS_PACKET_HEADER_LEN  sizeof(struct rndis_packet_msg_type)


//return 1 ok
//return 0 error
int spe_sim_rndis_rm_hdr(spe_sim_port_t* port, char* buf,unsigned int *msg_len)
{
	//unsigned int head;
	unsigned int payload_offset;
	/* tmp points to a struct rndis_packet_msg_type */
	__le32 *tmp = (void *)buf;
	//head = (unsigned int)(buf);

	SPE_SIM_INFO("%s: 100: enter\n", __FUNCTION__);
#if 0
	print_hex_dump(KERN_ERR, "HEAD:", DUMP_PREFIX_ADDRESS, 16, 1,
			       buf, 32, true);
	print_hex_dump(KERN_ERR, "HEAD:", DUMP_PREFIX_ADDRESS, 16, 1,
			       buf+32, 32, true);
	print_hex_dump(KERN_ERR, "HEAD:", DUMP_PREFIX_ADDRESS, 16, 1,
			       buf+64, 32, true);
#endif
	/* MessageType, MessageLength */
	if (cpu_to_le32(REMOTE_NDIS_PACKET_MSG)
			//!= get_unaligned(tmp++)) {  //can not find header file
			!= get_unaligned_le32(tmp++)) {
			//just care messagetype = 0x01
		return -EINVAL;
	}

    /* Whether padded or not, we rely on the messagelength to position the next rndis packet
        Normally, Microsoft did NOT pad the packets
    */
    *msg_len = le32_to_cpu(*tmp);
	
	SPE_SIM_INFO("%s: 200: MessageType, MessageLength=%d\n", __FUNCTION__,(*msg_len));

	tmp++;

	/* DataOffset, DataLength */
	payload_offset = get_unaligned_le32(tmp++) + 8;
	
	buf += payload_offset; 

	port->src_ptr = buf;
	port->src_len = get_unaligned_le32(tmp++); //payload length

	SPE_SIM_INFO("%s: 300: skb_queue_tail ok\n", __FUNCTION__);

	return 1;
}



////return 1 continue
////return 0 end
int spe_sim_rndis_unwrap(spe_sim_port_t* port, char* buf, unsigned int len)
{
	int total_len;
	int ret;
	int single_len;
	spe_sim_rndis_dec_ctx_t *rndis_ctx = &port->td_ctx.rndis_ctx;
	total_len = len;
	ret = 1;
	
	SPE_SIM_INFO("%s: 100: enter,total_len=%d\n", __FUNCTION__,total_len);
	
	buf += rndis_ctx->last_packet_pos;
	total_len -= rndis_ctx->last_packet_pos;

	/* shold be larger than RNDIS Header Len for valid packets */
	if(total_len > RNDIS_PACKET_HEADER_LEN)
	{
		SPE_SIM_INFO("%s: 200: td len : %d\n", __FUNCTION__, len);

		ret = spe_sim_rndis_rm_hdr(port, buf, &single_len);
		if(ret < 1)
		{
			goto rndis_unwrap_array_err;
		}

		rndis_ctx->last_packet_pos += single_len;
	}
	else
	{
		//last packet, leave it
		rndis_ctx->last_packet_pos = 0;
		SPE_SIM_INFO("%s: 500: last packet\n", __FUNCTION__);
#if 0
		print_hex_dump(KERN_ERR, "HEAD:", DUMP_PREFIX_ADDRESS, 16, 1,
		       port->src_ptr, 32, true);
#endif
		SPE_SIM_INFO("port->src_len is %d\n", port->src_len);
		ret = 0;
	}

	return ret;

rndis_unwrap_array_err:
	SPE_SIM_INFO("%s: 900: rndis_unwrap_array_err\n", __FUNCTION__);
	
	return -EFAULT;
}



//return value 
//0 flag end
///to be continued
int spe_sim_rndis_wrap(spe_sim_port_t* port, char* buf, unsigned int len)
{
	__le32 pad_len;
	__le32 *tmp;
	spe_sim_rndis_enc_ctx_t *rndis_ctx = &port->rd_ctx.rndis_ctx;
	struct rndis_packet_msg_type *header;
	__le32 flag_end = 1;
	SPE_SIM_INFO("%s:enter\n", __FUNCTION__);

	/* take into account the RNDIS Header when counting the pad */
	pad_len = ALIGN(len + RNDIS_PACKET_HEADER_LEN, RNDIS_GNET_ALIGN_LEN) - len - RNDIS_PACKET_HEADER_LEN;
	SPE_SIM_INFO("%s:pad_len=%d\n", __FUNCTION__, pad_len);

	if(rndis_ctx->dg_num_cnt == 0)
	{
		tmp = (__le32 *)(port->dst_ptr);
	}
	else
	{
		tmp = (__le32 *)(port->dst_ptr)+ rndis_ctx->last_packet_pos;
	}
	/* add the rndis header */
	
	*tmp++ = cpu_to_le32(REMOTE_NDIS_PACKET_MSG);			//MessageType
	*tmp++ = cpu_to_le32(len + sizeof(*header)+ pad_len);	//MessageLength
	rndis_ctx->sum_size += cpu_to_le32(len + sizeof(*header)+ pad_len); 
	*tmp++ = cpu_to_le32(36);								//DataOffset 
    /* should deduce the pad_len */
    *tmp++ = cpu_to_le32(len);								//DataLength

	//according to rndis protocol, the 28bytes must be set to zero.
	memset(tmp,0,28);

	//jump rest of the hdr
	tmp += cpu_to_le32(7);

	//copy item datas to dst->src buffer
	memcpy(tmp,buf,len);
	
	rndis_ctx->dg_num_cnt++;	
	
/*fixme : maybe i should just define one variable here*/	
	rndis_ctx->last_packet_pos = rndis_ctx->sum_size;

	if (((rndis_ctx->sum_size) > (SPE_GNET_IN_MAXSIZE_PER_TRANSFER-SPE_GNET_PKT_LEN_WATERLINE))||
        (rndis_ctx->dg_num_cnt >= SPE_GNET_PKT_NUM_WATERLINE))
    {
        flag_end = 0;
    }

    return flag_end;
}

//clean some paraments
int spe_sim_rndis_end_wrap(spe_sim_port_t* port)
{
	//printk("--roy spe_sim_rndis_end_wrap\n");
	spe_sim_rndis_enc_ctx_t *rndis_ctx = &port->rd_ctx.rndis_ctx;
/*fixme: */

	port->dst_len = rndis_ctx->sum_size;
	rndis_ctx->dg_num_cnt = 0;
	rndis_ctx->last_packet_pos = 0;
	rndis_ctx->sum_size = 0;
	
    return 0;
}

