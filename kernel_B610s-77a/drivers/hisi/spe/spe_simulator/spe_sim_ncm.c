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
 * spe_sim_ncm.c -- spe simulator ncm wrap/unwrap
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


#define USB_CDC_NCM_NDP16_CRC_SIGN      0x314D434E /* NCM1 */
#define USB_CDC_NCM_NDP16_NOCRC_SIGN    0x304D434E /* NCM0 */
#define USB_CDC_NCM_NDP32_CRC_SIGN      0x316D636E /* ncm1 */
#define USB_CDC_NCM_NDP32_NOCRC_SIGN    0x306D636E /* ncm0 */

#define USB_CDC_NCM_NTH16_SIGN          0x484D434E /* NCMH */
#define USB_CDC_NCM_NTH32_SIGN          0x686D636E /* ncmh */

#define SPE_SIM_NCM_WSEQUENCE           0x00       /* First wsequence */
#define SPE_SIM_NCM_NTB_RESERVE         0x4000     /* reserved buf */
#define ROOM_FOR_NTB_MAX_SIZE           (2048)
#define NCM_MAX_NUM                     0x40
#define FORMATS_SUPPORTED	(USB_CDC_NCM_NTB16_SUPPORTED |	\
				 USB_CDC_NCM_NTB32_SUPPORTED)
#define NTB_DEFAULT_IN_SIZE	USB_CDC_NCM_NTB_MIN_IN_SIZE
#define NTB_OUT_SIZE		16384


#define GNET_MAX_TRANSFER                   (0x4000) /* 16k */
#define GNET_OUT_MAXSIZE_PER_TRANSFER       (2*1024) /* 2k */
#define GNET_IN_MAXSIZE_PER_TRANSFER        (GNET_MAX_TRANSFER)
#define USB_CDC_NCM_NTB_IN_MAXSIZE          (128*1024)
#define USB_CDC_NCM_NDP_IN_DIVISOR          (4)
#define USB_CDC_NCM_NDP_IN_REMAINDER        (2)
#define USB_CDC_NCM_NDP_IN_ALIGNMENT        (4)

#define USB_CDC_NCM_NDP_OUT_DIVISOR         (4)
#define USB_CDC_NCM_NDP_OUT_REMAINDER       (2)
#define USB_CDC_NCM_NDP_OUT_ALIGNMENT       (4)

typedef unsigned __le32;
typedef unsigned short __le16;

struct ndp_parser_opts {
    unsigned    nth_sign;
    unsigned    ndp_sign;
    unsigned    nth_size;
    unsigned    ndp_size;
    unsigned    ndplen_align;
    /* sizes in u16 units */
    unsigned    dgram_item_len; /* index or length */
    unsigned    block_length;
    unsigned    fp_index;
    unsigned    reserved1;
    unsigned    reserved2;
    unsigned    next_fp_index;
};
struct usb_cdc_ncm_nth_parameters {
    __le16 wLength;
    __le16 bmNthFormatsSupported;
    __le32 dwNtbInMaxSize;
    __le16 wNdpInDivisor;
    __le16 wNdpInPayloadRemainder;
    __le16 wNdpInAlignment;
    __le16 wPadding1;
    __le32 dwNtbOutMaxSize;
    __le16 wNdpOutDivisor;
    __le16 wNdpOutPayloadRemainder;
    __le16 wNdpOutAlignment;
    __le16 wNtbOutMaxDatagrams;
} __attribute__ ((packed));

static struct usb_cdc_ncm_ntb_parameters ntb_parameters = {
	.wLength = sizeof ntb_parameters,
	.bmNtbFormatsSupported = cpu_to_le16(FORMATS_SUPPORTED),
	.dwNtbInMaxSize = cpu_to_le32(GNET_IN_MAXSIZE_PER_TRANSFER),
	.wNdpInDivisor = cpu_to_le16(USB_CDC_NCM_NDP_IN_DIVISOR),
	.wNdpInPayloadRemainder = cpu_to_le16(USB_CDC_NCM_NDP_IN_REMAINDER),
	.wNdpInAlignment = cpu_to_le16(USB_CDC_NCM_NDP_IN_ALIGNMENT),

	.dwNtbOutMaxSize = cpu_to_le32(GNET_OUT_MAXSIZE_PER_TRANSFER),
	.wNdpOutDivisor = cpu_to_le16(USB_CDC_NCM_NDP_OUT_DIVISOR),
	.wNdpOutPayloadRemainder = cpu_to_le16(USB_CDC_NCM_NDP_OUT_REMAINDER),
	.wNdpOutAlignment = cpu_to_le16(USB_CDC_NCM_NDP_OUT_ALIGNMENT),
};


#define INIT_NDP16_OPTS {                   \
        .nth_sign = USB_CDC_NCM_NTH16_SIGN,     \
        .ndp_sign = USB_CDC_NCM_NDP16_NOCRC_SIGN,   \
        .nth_size = sizeof(struct usb_cdc_ncm_nth16),   \
        .ndp_size = sizeof(struct usb_cdc_ncm_ndp16),   \
        .ndplen_align = 4,              \
        .dgram_item_len = 1,                \
        .block_length = 1,              \
        .fp_index = 1,                  \
        .reserved1 = 0,                 \
        .reserved2 = 0,                 \
        .next_fp_index = 1,             \
    }


#define INIT_NDP32_OPTS {                   \
        .nth_sign = USB_CDC_NCM_NTH32_SIGN,     \
        .ndp_sign = USB_CDC_NCM_NDP32_NOCRC_SIGN,   \
        .nth_size = sizeof(struct usb_cdc_ncm_nth32),   \
        .ndp_size = sizeof(struct usb_cdc_ncm_ndp32),   \
        .ndplen_align = 8,              \
        .dgram_item_len = 2,                \
        .block_length = 2,              \
        .fp_index = 2,                  \
        .reserved1 = 1,                 \
        .reserved2 = 2,                 \
        .next_fp_index = 2,             \
    }

static struct ndp_parser_opts ndp16_opts = INIT_NDP16_OPTS;
static struct ndp_parser_opts ndp32_opts = INIT_NDP32_OPTS;


static inline void put_ncm(__le16 **p, unsigned size, unsigned val)
{
    switch (size) {
    case 1:
        put_unaligned_le16((u16)val, *p);
        break;
    case 2:
        put_unaligned_le32((u32)val, *p);

        break;
    default:
        BUG();
    }

    *p += size;
}


static inline unsigned get_ncm(__le16 **p, unsigned size)
{
    unsigned tmp;

    switch (size) {
    case 1:
        tmp = get_unaligned_le16(*p);
        break;
    case 2:
        tmp = get_unaligned_le32(*p);
        break;
    default:
        BUG();
    }

    *p += size;
    return tmp;
}

int spe_sim_ncm_unwrap(spe_sim_port_t* port, char* buf, unsigned int len)
{
    spe_sim_ncm_dec_ctx_t* ncm_ctx = &port->td_ctx.ncm_ctx;
    __le16      *tmp = (void *)buf;
    unsigned    index;
    unsigned    dg_len;
    unsigned    ndp_len;
    int         ret = 1;
    unsigned    crc_len = 0;

	struct ndp_parser_opts *opts;
	if(buf< 0xb0000000){
		SPE_SIM_ERR("ncm: %s input buf is 0x%x! \n", __func__, buf);
		spe_sim_print_port(port->port_num);
		BUG_ON(buf< 0xb0000000);
	}
	ncm_ctx->is_ntb32 = 1;

	opts = ncm_ctx->is_ntb32 ? (&ndp32_opts) : (&ndp16_opts);

    /* first enter ... */
    if (0 == ncm_ctx->index_pos) {
        tmp = (void *)buf;
        SPE_SIM_INFO("ncm: %s addr is 0x%x \n", __func__, tmp);

        /* dwSignature */
        if (get_unaligned_le32(tmp) != opts->nth_sign) {
			
			print_hex_dump(KERN_ERR, "HEAD:", DUMP_PREFIX_ADDRESS, 16, 1,
			       buf, 32, true);
            printk(KERN_EMERG"%s: Wrong NTH SIGN\n", __func__);
            goto err;
        }
        tmp += 2;
        /* wHeaderLength */
        if (get_unaligned_le16(tmp++) != opts->nth_size) {
            printk(KERN_EMERG"%s: Wrong NTB headersize\n", __func__);
            goto err;
        }
        tmp++; /* skip wSequence */
        /* (d)wBlockLength */
        get_ncm(&tmp, opts->block_length);
        index = get_ncm(&tmp, opts->fp_index);
        /* NCM 3.2 */
        if (((index % 4) != 0) && (index < opts->nth_size)) {
            printk(KERN_EMERG"%s: Bad index: %x\n", __func__, index);
            goto err;
        }

        /* walk through NDP */
        tmp = ((void *)buf) + index;
        if (get_unaligned_le32(tmp) != opts->ndp_sign) {
            printk(KERN_EMERG"%s: Wrong NDP SIGN\n", __func__);
            goto err;
        }
        tmp += 2;
        ndp_len = get_unaligned_le16(tmp++);
        /*
         * NCM 3.3.1
         * entry is 2 items
         * item size is 16/32 bits, opts->dgram_item_len * 2 bytes
         * minimal: struct usb_cdc_ncm_ndpX + normal entry + zero entry
         */

        if ((ndp_len < opts->ndp_size + 2 * 2 * (opts->dgram_item_len * 2))
            || (ndp_len % opts->ndplen_align != 0)) {
            printk(KERN_EMERG"%s: Bad NDP length: %x\n", __func__, ndp_len);
            goto err;
        }

        tmp += opts->reserved1;
        tmp += opts->next_fp_index; /* skip reserved (d)wNextFpIndex */
        tmp += opts->reserved2;

        ndp_len -= opts->ndp_size;
        ncm_ctx->index_pos = get_ncm(&tmp, opts->dgram_item_len);
        ncm_ctx->dg_len = get_ncm(&tmp, opts->dgram_item_len);
        ncm_ctx->ndp_len = ndp_len;
        ncm_ctx->cur_pos = tmp;
    }

    /* here is while proc in f_ncm */
    index = ncm_ctx->index_pos;
    dg_len = ncm_ctx->dg_len;
    if (dg_len < 14 + crc_len
        || index >= len
        || index + dg_len > len
        || dg_len > ETH_FRAME_LEN + crc_len){ /* ethernet header + crc */

        printk(KERN_EMERG"%s: Bad NDP length: %x\n", __func__, dg_len);
        goto err;
    }


    ncm_ctx->index_pos =
        get_ncm((__le16 **)&ncm_ctx->cur_pos, opts->dgram_item_len);
    ncm_ctx->dg_len =
        get_ncm((__le16 **)&ncm_ctx->cur_pos, opts->dgram_item_len);

    port->src_ptr = buf + index;
    port->src_len = dg_len - crc_len;

    ncm_ctx->ndp_len -= 2 * (opts->dgram_item_len * 2);


    /* parse to the end of packet */
    if (ncm_ctx->index_pos == 0 || ncm_ctx->dg_len == 0 ||
        (ncm_ctx->ndp_len <= 2 * (opts->dgram_item_len * 2))) {

        /* ensure the index_pos set to 0 */
        ncm_ctx->index_pos = 0;
        ncm_ctx->cur_pos = NULL;
        ret = 0;
    }

    return ret;

err:
    /* 0: stop unwrap the cur packet */
    SPE_SIM_TRACE("ncm unwarp err!. \n");
    return -EFAULT;
}

/* ret:0 wrap end, ret:1 need wrapping */
int spe_sim_ncm_wrap(spe_sim_port_t* port, char* buf, unsigned int len)
{
    spe_sim_ncm_enc_ctx_t* ncm_ctx = &port->rd_ctx.ncm_ctx;

    unsigned short *tmp;
    unsigned int    ncb_len = 0;
    unsigned int    ntb_len = 0;
    int             ndp_pad;
    unsigned int    pad;
    unsigned int    pkt_pad;
    unsigned int    flag_end = 1;
    void *          tmp_ptr;
    struct tag_spe_sim_ctx *sim_ctx = (struct tag_spe_sim_ctx *)port->ctx;
    //int div = le16_to_cpu(ntb_parameters.wNdpInDivisor);
    //int rem = le16_to_cpu(ntb_parameters.wNdpInPayloadRemainder);
    int div = sim_ctx->ncm_div;
	int rem = sim_ctx->ncm_rem;
    int ndp_align = le16_to_cpu(ntb_parameters.wNdpInAlignment);
    SPE_SIM_TRACE("rd warp:rd spe_sim_ncm_wrap entry \n");
	
    ncm_ctx->is_ntb32 = 1;
    ncm_ctx->dg_ctx.max_num = NCM_MAX_NUM;
    //ncm_ctx->fixed_in_len = le32_to_cpu(ntb_parameters.dwNtbInMaxSize);
	ncm_ctx->fixed_in_len = sim_ctx->ncm_max_size;

    struct ndp_parser_opts *opts =  ncm_ctx->is_ntb32 ?
                                    (&ndp32_opts) : (&ndp16_opts);

        /* the first entry */
    if (ncm_ctx->dg_ctx.dg_cnt == 0) {
        pad = 0;
        ncb_len += opts->nth_size;
        ndp_pad  = ALIGN(ncb_len, ndp_align) - ncb_len;
        ncb_len += ndp_pad;
        ncb_len += opts->ndp_size;
        ncm_ctx->pkt_len_waterline = ncm_ctx->fixed_in_len - ROOM_FOR_NTB_MAX_SIZE;

        /* datagram entry */
        ncb_len += 2 * 2 * opts->dgram_item_len;
        ncm_ctx->dg_ctx.last_dg_pos = ncb_len;
        ncb_len += ncm_ctx->dg_ctx.max_num * 2 * 2 * opts->dgram_item_len;

        pad = ALIGN(ncb_len, div) + rem - ncb_len;
        ncb_len += pad;
        ncm_ctx->dg_ctx.max_len = ncb_len;

        /* fill ntb header base info */
        tmp = (unsigned short *)port->dst_ptr;
        memset(tmp, 0, ncb_len);

        put_unaligned_le32(opts->nth_sign, tmp); /* dwSignature */
        tmp += 2;
        put_unaligned_le16(opts->nth_size, tmp++); /* wHeaderLength */
        tmp++;                                   /* skip wSequence */
        put_ncm(&tmp, opts->block_length, ncb_len + len);/* (d)wBlockLength */
        put_ncm(&tmp, opts->fp_index, opts->nth_size + ndp_pad);/*(d)wFpIndex */
        tmp = (__le16 *)((unsigned )tmp + ndp_pad);

        put_unaligned_le32(opts->ndp_sign, tmp); /* NDP dwSignature */
        tmp += 2;
        ncm_ctx->ndp_len_pos = tmp;
        tmp++;                                      /* skip dwLength */

        tmp += opts->reserved1;
        tmp += opts->next_fp_index;
        tmp += opts->reserved2;
        ncm_ctx->sum_size = ncb_len;
    }
    else {
        tmp_ptr = port->dst_ptr + ncm_ctx->dg_ctx.last_dg_pos;
        ncm_ctx->dg_ctx.last_dg_pos += 2 * 2 * opts->dgram_item_len;
        tmp = (unsigned short *)tmp_ptr;
    }      
        ncm_ctx->ncm_pos = port->dst_ptr + ncm_ctx->sum_size; 
        
        /* (d)wDatagramIndex[i-1] */
        put_ncm(&tmp, opts->dgram_item_len, ncm_ctx->sum_size);
        /* (d)wDatagramLength[i-1] */
        ncm_ctx->sum_size += len;
        ntb_len = ncm_ctx->sum_size;
        pkt_pad = ALIGN(ntb_len, div) + rem - ntb_len;
        ncm_ctx->sum_size += pkt_pad;
        put_ncm(&tmp, opts->dgram_item_len, len);
        /* (d)wDatagramIndex[i] and (d)wDatagramLength[i] already zeroed */
        ncm_ctx->dg_ctx.dg_cnt++;

        if (((ncm_ctx->sum_size) > ncm_ctx->pkt_len_waterline)||
             (ncm_ctx->dg_ctx.dg_cnt >= ncm_ctx->dg_ctx.max_num))
        {
            flag_end = 0;
        }
        memcpy(ncm_ctx->ncm_pos, buf, len);
        return flag_end;
}

int spe_sim_ncm_end_wrap(spe_sim_port_t* port)
{
    spe_sim_ncm_enc_ctx_t* ncm_ctx = &port->rd_ctx.ncm_ctx;
    unsigned short *tmp;
    unsigned ndp_rlen;
    struct ndp_parser_opts *opts =  ncm_ctx->is_ntb32 ?
                                    (&ndp32_opts) : (&ndp16_opts);

    tmp = (unsigned short *)port->dst_ptr;
    tmp += 4;
    put_ncm(&tmp, opts->block_length, ncm_ctx->sum_size);/* (d)wBlockLength */

    tmp = (unsigned short *)port->dst_ptr;
    tmp = (unsigned short *)((unsigned int)tmp + ncm_ctx->dg_ctx.last_dg_pos);
    ndp_rlen = opts->ndp_size + 2 * 2 * opts->dgram_item_len * (ncm_ctx->dg_ctx.dg_cnt + 1);
    put_unaligned_le16(ndp_rlen, ncm_ctx->ndp_len_pos);/*dwLength*/  
    memset(tmp, 0, 2 * 2 * opts->dgram_item_len);
    
    ncm_ctx->dg_ctx.dg_cnt = 0;
    port->dst_len = port->rd_ctx.ncm_ctx.sum_size;

    
    return 0;
}


