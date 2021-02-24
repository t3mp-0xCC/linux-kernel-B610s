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

#ifndef __SPE_REG_H__
#define __SPE_REG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*__cplusplus*/

#define SPE_REG_BASE                        (0x91004000)
#define SPE_REG_SIZE                        (0x4000)
#define SPE_IRQ                             (215)
#define SPE_CH_PRT_INTERVAL                    (0x20)
#define SPE_CH_CTRL_INTERVAL                    (0x40)

#define SPE_MAC_TAB_NUMBER          (128)
#define SPE_MAC_TAB_WORD_NO         (4)
#define SPE_IPV4_TAB_NUMBER         (16)
#define SPE_IPV4_TAB_WORD_NO        (6)
#define SPE_IPV6_TAB_NUMBER         (8)
#define SPE_IPV6_TAB_WORD_NO        (12)
#define SPE_TAB_ENTRY_INTERVAL      (4)

#define SPE_ENTRY_NUM               (704)
#define SPE_MAC_ENTRY_NUM           (512)
#define SPE_IPV4_ENTRY_NUM          (96)

#define SPE_TAB_L4_PORTNUM_CFG      (4)

#ifndef CONFIG_BALONG_SPE_SIM
#include "hi_spe.h"
#else
#include <../spe_simulator/hi_spe_sim.h>
#endif

#define SPE_CH_PROPERTY_OFFSET(x)           (HI_SPE_PORTX_PROPERTY_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))

/* desc queue */
#define SPE_TDQ_BADDR_OFFSET(x)             (HI_SPE_TDQX_BADDR_0_OFFSET+SPE_CH_PRT_INTERVAL*(x))
#define SPE_TDQ_LEN_OFFSET(x)               (HI_SPE_TDQX_LEN_0_OFFSET+SPE_CH_PRT_INTERVAL*(x))
#define SPE_TDQ_PTR_OFFSET(x)               (HI_SPE_TDQX_PTR_0_OFFSET+SPE_CH_PRT_INTERVAL*(x))
#define SPE_TDQ_PRI_OFFSET(x)               (HI_SPE_TDQX_PRI_0_OFFSET+SPE_CH_PRT_INTERVAL*(x))

#define SPE_PORT_NUM_OFFSET(x)            (HI_SPE_CHX_PORTNUM_0_OFFSET+SPE_CH_PRT_INTERVAL*(x))

#define SPE_RDQ_BADDR_OFFSET(x)             (HI_SPE_RDQX_BADDR_0_OFFSET+SPE_CH_PRT_INTERVAL*(x))
#define SPE_RDQ_LEN_OFFSET(x)               (HI_SPE_RDQX_LEN_0_OFFSET+SPE_CH_PRT_INTERVAL*(x))
#define SPE_RDQ_PTR_OFFSET(x)               (HI_SPE_RDQX_PTR_0_OFFSET+SPE_CH_PRT_INTERVAL*(x))

/* event buf */
#define SPE_EVENT_BUF_PTR_OFFSET(x)         (HI_SPE_EVENT_BUFF0_PTR_OFFSET + 0x8*(x))
#define SPE_EVENT_BUF_ADDR_OFFSET(x)        (HI_SPE_EVENT_BUFF0_ADDR_OFFSET + 0x8*(x))
#define SPE_EVENT_BUFF_LEN_OFFSET           (HI_SPE_EVENT_BUFF_LEN_OFFSET)
#define SPE_INT_INTERVAL_OFFSET(x)          (HI_SPE_INT0_INTERVAL_OFFSET + 0x4*(x))

#define SPE_EVENT_BUF_MASK_OFFSET(x)        (HI_SPE_EVENT_BUFF0_MASK_OFFSET+0x4*(x))
#define SPE_EVENTBUF_PRO_CNT_OFFSET(x)     (HI_SPE_EVENTBUF0_PRO_CNT_OFFSET+0x8*(x))
#define SPE_EVENTBUF_RPT_CNT_OFFSET(x)     (HI_SPE_EVENTBUF0_RPT_CNT_OFFSET+0x8*(x))

#define SPE_EVENT_BUFF_RD_ERR_OFFSET(x)       (HI_SPE_EVENT_BUFF0_RD_ERR_OFFSET+0x4*(x))


/* flow limit */
#define SPE_CH_UDP_LIMIT_TIME_OFFSET(x)     (HI_SPE_PORT_LMTTIME_OFFSET)/*HI_SPE_CH_UDP_LIMIT_TIME_0_OFFSET+0x30*(x)*/
#define SPE_CH_UDP_LIMIT_CNT_OFFSET(x)      (HI_SPE_PORTX_UDP_LMTNUM_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))
#define SPE_CH_RATE_LIMIT_TIME_OFFSET(x)    (HI_SPE_PORT_LMTTIME_OFFSET)/*HI_SPE_CH_RATE_LIMIT_TIME_0_OFFSET+0x30*(x)*/
#define SPE_CH_RATE_LIMIT_BYTE_OFFSET(x)    (HI_SPE_PORTX_LMTBYTE_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))

/* MAC ADDR */
#define SPE_CH_MAC_ADDR_L(x)                (HI_SPE_PORTX_MAC_ADDR_L_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))
#define SPE_CH_MAC_ADDR_H(x)                (HI_SPE_PORTX_MAC_ADDR_H_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))

/* IPV6 ADDR & MASK*/
#define SPE_CH_IPV6_ADDR_1ST(x)             (HI_SPE_PORTX_IPV6_ADDR_1ST_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))
#define SPE_CH_IPV6_ADDR_2ND(x)             (HI_SPE_PORTX_IPV6_ADDR_2ND_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))
#define SPE_CH_IPV6_ADDR_3RD(x)             (HI_SPE_PORTX_IPV6_ADDR_3TH_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))
#define SPE_CH_IPV6_ADDR_4TH(x)             (HI_SPE_PORTX_IPV6_ADDR_4TH_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))
#define SPE_CH_IPV6_MASK(x)             (HI_SPE_PORTX_IPV6_MASK_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))

/* IPV4 ADDR */
#define SPE_CH_IPV4_ADDR(x)                 (HI_SPE_PORTX_IPV4_ADDR_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))
#define SPE_CH_IPV4_MASK(x)                 (HI_SPE_PORTX_IPV4_MASK_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))

#define SPE_TAB_WORD_OFFSET(x)              (HI_SPE_TAB_CONTENT_0_OFFSET+4*(x))

#define SPE_PORTX_UDP_RATE_LIMIT(x)                (HI_SPE_PORTX_UDP_LMTNUM_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))
#define SPE_PORTX_RATE_LIMIT(x)                (HI_SPE_PORTX_LMTBYTE_0_OFFSET+SPE_CH_CTRL_INTERVAL*(x))

#define SPE_MAC_TAB_OFFSET(x)                   (HI_SPE_TAB_0_OFFSET+SPE_TAB_ENTRY_INTERVAL*(x))
#define SPE_IPV4_TAB_OFFSET(x)                  (SPE_MAC_TAB_OFFSET(SPE_MAC_ENTRY_NUM)+SPE_TAB_ENTRY_INTERVAL*(x))
#define SPE_IPV6_TAB_OFFSET(x)                  (SPE_IPV4_TAB_OFFSET(SPE_IPV4_ENTRY_NUM)+SPE_TAB_ENTRY_INTERVAL*(x))

/**/
#define SPE_L4_PORTNUM_CFG_OFFSET(x)        (HI_SPE_L4_PORTNUM_CFG_0_OFFSET+SPE_TAB_L4_PORTNUM_CFG*(x))

typedef union
{
    struct
    {
        unsigned int    spe_port_attach_brg        : 2; /* [1..0] 端口附属特性寄存器。2'd0：本端口是网桥下挂端口；2'd1：本端口是普通端口；2'd2：本端口为混杂端口；2'd3：保留。 */
        unsigned int    spe_port_enc_type          : 4; /* [5..2] 端口报文格式。4'd0：无封包；4'd1：NCM_NTH16封包；4'd2：NCM_NTH32封包；4'd3：RNDIS封包；4'd4：WIFI数据；4'd5：IPF；4'd6：CPU虚拟端口；其它：保留。 */
        unsigned int    spe_port_pad_en            : 1; /* [6..6] padding添加使能。1'b0：不使能；1'b1：使能。 */
        unsigned int    reserved                   : 24; /* [30..7] 保留。 */
        unsigned int    spe_port_en                : 1; /* [31..31] 端口使能寄存器。1'b0：端口不使能；1'b1：端口使能。 */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_PROPERTY_T; /*HI_SPE_CH_PROPERTY_T*/

typedef union
{
    struct
    {
        unsigned int    rdq_empty_int_mask        : 1; /* [0..0] RDQ空中断mask。1'b0：使能；1'b1：屏蔽。 */
        unsigned int    rdq_full_int_mask         : 1; /* [1..1] RDQ满中断mask。1'b0：使能；1'b1：屏蔽。 */
        unsigned int    rd_done_int_mask          : 1; /* [2..2] RD完成中断mask。1'b0：使能；1'b1：屏蔽。 */
        unsigned int    tdq_empty_int_mask        : 1; /* [3..3] TDQ空中断mask。1'b0：使能；1'b1：屏蔽。 */
        unsigned int    tdq_full_int_mask         : 1; /* [4..4] TDQ满中断mask。1'b0：使能；1'b1：屏蔽。 */
        unsigned int    td_done_int_mask          : 1; /* [5..5] TD完成中断mask。1'b0：使能；1'b1：屏蔽。 */
        unsigned int    reserved                  : 26; /* [31..6] 保留。 */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENT_BUFF_MASK_T;    /* eventbuffer0中断MASK控制寄存器 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__SPE_REG_H__*/
