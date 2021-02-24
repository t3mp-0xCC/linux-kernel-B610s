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

#ifndef __SPE_DESC_H__
#define __SPE_DESC_H__

struct spe_td_trb {
    u32 buf_addr;
    u32 skb_addr;
    u16 eth_type;
    u16 push_en:1;
    u16 irq_en:1;
    u16 timestamp_en:1;
    u16 reserved:13;
    u16 reserved1;
    u16 pkt_len;
    u32 result;
};

struct spe_td_desc {
    struct spe_td_trb trb;
    u32 usr_field1;
    u32 usr_field2;
};

struct spe_rd_trb {
    u32 buf_addr;
    u32 skb_addr;
    u16 sport_num:4;
    u16 reserve1:12;
    u16 irq_en:1;
    u16 reserve2:11;
    u16 updata_only:1;
    u16 reserve3:3;
    u16 vid:12;
    u16 dport_num:4;
    u16 pkt_len;
    u32 result;
};

struct spe_rd_desc {
    struct spe_rd_trb trb;
    u32 usr_field1;
    u32 usr_field2;
    //dma_addr_t dma;
};

/* desc number occupied by hardware */
static inline int spe_rd_busyslots(int portno)
{
    struct spe *spe = &spe_balong;
    struct spe_port_ctrl *ctrl = &spe->ports[portno].ctrl;
    int ret;

    ret = (ctrl->rd_free >= ctrl->rd_busy)?(ctrl->rd_free - ctrl->rd_busy):
            (ctrl->rd_depth - ctrl->rd_busy + ctrl->rd_free);

    return ret;
}

/* desc number can be used by software */
static inline int spe_rd_freeslots(int portno)
{
    struct spe *spe = &spe_balong;
    struct spe_port_ctrl *ctrl = &spe->ports[portno].ctrl;
    int ret;

    /* here 1 for distinguish full and empty */
    ret = ctrl->rd_depth - 1 - spe_rd_busyslots(portno);
    return ret;
}

/* no rd for software */
static inline bool spe_rd_full(int portno)
{
    return (0 == spe_rd_freeslots(portno));
}

/* all rd for software */
static inline bool spe_rd_empty(int portno)
{
    return (0 == spe_rd_busyslots(portno));
}

/* desc number occupied by hardware */
static inline int spe_td_busyslots(int portno)
{
    struct spe *spe = &spe_balong;
    struct spe_port_ctrl *ctrl = &spe->ports[portno].ctrl;
    int ret;

    ret = (ctrl->td_free >= ctrl->td_busy)?(ctrl->td_free - ctrl->td_busy):
            (ctrl->td_depth - ctrl->td_busy + ctrl->td_free);
    return ret;
}

/* desc number can be used by software */
static inline int spe_td_freeslots(int portno)
{
    struct spe *spe = &spe_balong;
    struct spe_port_ctrl *ctrl = &spe->ports[portno].ctrl;
    int ret;

    /* 1 for distinguish full and empty */
    ret = ctrl->td_depth - 1 - spe_td_busyslots(portno);
    return ret;
}

/* no rd for software */
static inline bool spe_td_full(int portno)
{
    return (0 == spe_td_freeslots(portno));
}

/* all rd for software */
static inline bool spe_td_empty(int portno)
{
    return (0 == spe_td_busyslots(portno));
}

int spe_rd_recycle(struct spe *spe, u32 portno);
void spe_rd_rewind(struct spe *spe, u32 portno);


#endif /*__SPE_DESC_H__ */
