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

#ifndef __HI_SPE_MPI_H__
#define __HI_SPE_MPI_H__


#define SPE_SIM_REG_OFFSET_RIGHT_SHIFT        	24
#define MASK_OFFSET		            (0xffffffff >> (32-SPE_SIM_REG_OFFSET_RIGHT_SHIFT))
#define SPE_SIM_REG_READ_EX         (0x1 << (32-SPE_SIM_REG_OFFSET_RIGHT_SHIFT-2))
#define SPE_SIM_REG_WRITE_EX        (0x2 << (32-SPE_SIM_REG_OFFSET_RIGHT_SHIFT-2))
#define SPE_SIM_REG_WR_OFFSET       (0x3 << (32-SPE_SIM_REG_OFFSET_RIGHT_SHIFT-2))
#define SPE_SIM_REG_WR_MASK         (~SPE_SIM_REG_WR_OFFSET)

#define SPE_SIM_REG_MAKE_WRITE_EX(n) \
    ((SPE_SIM_REG_WRITE_EX | (n))<<SPE_SIM_REG_OFFSET_RIGHT_SHIFT)

#define SPE_SIM_REG_MAKE_READ_EX(n) \
    ((SPE_SIM_REG_READ_EX | (n))<<SPE_SIM_REG_OFFSET_RIGHT_SHIFT)

#define SPE_SIM_REG_MAKE_WR_EX(n) \
        ((SPE_SIM_REG_WR_OFFSET | (n))<<SPE_SIM_REG_OFFSET_RIGHT_SHIFT)


/*
 * Project: HI
 * Module : SPE_MPI
 */

#ifndef HI_SET_GET
#define HI_SET_GET(a0,a1,a2,a3,a4)
#endif

/********************************************************************************/
/*    SPE_MPI �Ĵ���ƫ�ƶ��壨��Ŀ��_ģ����_�Ĵ�����_OFFSET)        */
/********************************************************************************/
#define    HI_SPE_EN_OFFSET                                  (0x0 | SPE_SIM_REG_MAKE_WRITE_EX(2)) /* SPE����������ʹ�ܼĴ��� */
#define    HI_SPE_USRFIELD_CTRL_OFFSET                       (0x4 | SPE_SIM_REG_MAKE_WRITE_EX(1)) /* SPEȫ�ֿ��ƼĴ��� */
#define    HI_SPE_BLACK_WHITE_OFFSET                         (0xC) /* �ڰ�����ָʾ�Ĵ��� */
#define    HI_SPE_PORT_LMTTIME_OFFSET                        (0x10) /* �˿�������UDPС���������ʱ�����üĴ��� */
#define    HI_SPE_ETH_MINLEN_OFFSET                          (0x14) /* ������̫���ж����� */
#define    HI_SPE_MODE_OFFSET                                (0x18) /* STICKģʽָʾ�Ĵ��� */
#define    HI_SPE_DBGEN_OFFSET                               (0x1C) /* ��Ȩģʽ�Ĵ��� */
#define    HI_SPE_GLB_DMAC_ADDR_L_OFFSET                     (0x20 | SPE_SIM_REG_MAKE_WRITE_EX(25)) /* SPEȫ��Ŀ��MAC��λ��ַ���üĴ��� */
#define    HI_SPE_GLB_DMAC_ADDR_H_OFFSET                     (0x24 | SPE_SIM_REG_MAKE_WRITE_EX(26)) /* SPEȫ��Ŀ��MAC��λ��ַ���üĴ��� */
#define    HI_SPE_GLB_SMAC_ADDR_L_OFFSET                     (0x28 | SPE_SIM_REG_MAKE_WRITE_EX(27)) /* SPEȫ��ԴMAC��λ��ַ���üĴ��� */
#define    HI_SPE_GLB_SMAC_ADDR_H_OFFSET                     (0x2C | SPE_SIM_REG_MAKE_WRITE_EX(28)) /* SPEȫ��ԴMAC��λ��ַ���üĴ��� */
#define    HI_SPE_RD_BURSTLEN_OFFSET                         (0x30) /* SPE����burst���ȼĴ��� */
#define    HI_SPE_L4_PORTNUM_CFG_0_OFFSET                    (0x40) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_1_OFFSET                    (0x44) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_2_OFFSET                    (0x48) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_3_OFFSET                    (0x4C) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_4_OFFSET                    (0x50) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_5_OFFSET                    (0x54) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_6_OFFSET                    (0x58) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_7_OFFSET                    (0x5C) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_8_OFFSET                    (0x60) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_9_OFFSET                    (0x64) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_10_OFFSET                   (0x68) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_11_OFFSET                   (0x6C) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_12_OFFSET                   (0x70) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_13_OFFSET                   (0x74) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_14_OFFSET                   (0x78) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_L4_PORTNUM_CFG_15_OFFSET                   (0x7C) /* TCP/UDP�˿�ƥ����˼Ĵ����� */
#define    HI_SPE_CNT_CLR_OFFSET                             (0x80) /* ͳ������Ĵ����� */
#define    HI_SPE_VER_OFFSET                                 (0x9C) /* SPE�汾�żĴ��� */
#define    HI_SPE_TIME_OFFSET                                (0xA0 | SPE_SIM_REG_MAKE_READ_EX(13)) /* SPE������ʱ���ϱ��Ĵ��� */
#define    HI_SPE_IDLE_OFFSET                                (0xA4) /* SPE���б�־ */
#define    HI_SPE_OST_CTRL_OFFSET                            (0x100) /* SPE������outstanding���ƼĴ��� */
#define    HI_SPE_BURST_CTRL_OFFSET                          (0x104) /* SPE������burst���ƼĴ��� */
#define    HI_SPE_WCH_ID_OFFSET                              (0x108) /* дͨ��ID�Ĵ��� */
#define    HI_SPE_RCH_ID_OFFSET                              (0x10C) /* ��ͨ��ID�Ĵ��� */
#define    HI_SPE_WCH_PRI_OFFSET                             (0x110) /* дͨ�����ȼ��Ĵ��� */
#define    HI_SPE_RCH_PRI_OFFSET                             (0x114) /* ��ͨ�����ȼ��Ĵ��� */
#define    HI_SPE_TDQX_BADDR_0_OFFSET                        (0x200 | SPE_SIM_REG_MAKE_WRITE_EX(18)) /* TD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_BADDR_1_OFFSET                        (0x220 | SPE_SIM_REG_MAKE_WRITE_EX(18)) /* TD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_BADDR_2_OFFSET                        (0x240 | SPE_SIM_REG_MAKE_WRITE_EX(18)) /* TD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_BADDR_3_OFFSET                        (0x260 | SPE_SIM_REG_MAKE_WRITE_EX(18)) /* TD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_BADDR_4_OFFSET                        (0x280 | SPE_SIM_REG_MAKE_WRITE_EX(18)) /* TD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_BADDR_5_OFFSET                        (0x2A0 | SPE_SIM_REG_MAKE_WRITE_EX(18)) /* TD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_BADDR_6_OFFSET                        (0x2C0 | SPE_SIM_REG_MAKE_WRITE_EX(18)) /* TD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_BADDR_7_OFFSET                        (0x2E0 | SPE_SIM_REG_MAKE_WRITE_EX(18)) /* TD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_LEN_0_OFFSET                          (0x204 | SPE_SIM_REG_MAKE_WRITE_EX(19)) /* TD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_LEN_1_OFFSET                          (0x224 | SPE_SIM_REG_MAKE_WRITE_EX(19)) /* TD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_LEN_2_OFFSET                          (0x244 | SPE_SIM_REG_MAKE_WRITE_EX(19)) /* TD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_LEN_3_OFFSET                          (0x264 | SPE_SIM_REG_MAKE_WRITE_EX(19)) /* TD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_LEN_4_OFFSET                          (0x284 | SPE_SIM_REG_MAKE_WRITE_EX(19)) /* TD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_LEN_5_OFFSET                          (0x2A4 | SPE_SIM_REG_MAKE_WRITE_EX(19)) /* TD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_LEN_6_OFFSET                          (0x2C4 | SPE_SIM_REG_MAKE_WRITE_EX(19)) /* TD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_LEN_7_OFFSET                          (0x2E4 | SPE_SIM_REG_MAKE_WRITE_EX(19)) /* TD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PTR_0_OFFSET                          (0x208 | SPE_SIM_REG_MAKE_WRITE_EX(20)) /* SPE TD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PTR_1_OFFSET                          (0x228 | SPE_SIM_REG_MAKE_WRITE_EX(20)) /* SPE TD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PTR_2_OFFSET                          (0x248 | SPE_SIM_REG_MAKE_WRITE_EX(20)) /* SPE TD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PTR_3_OFFSET                          (0x268 | SPE_SIM_REG_MAKE_WRITE_EX(20)) /* SPE TD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PTR_4_OFFSET                          (0x288 | SPE_SIM_REG_MAKE_WRITE_EX(20)) /* SPE TD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PTR_5_OFFSET                          (0x2A8 | SPE_SIM_REG_MAKE_WRITE_EX(20)) /* SPE TD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PTR_6_OFFSET                          (0x2C8 | SPE_SIM_REG_MAKE_WRITE_EX(20)) /* SPE TD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PTR_7_OFFSET                          (0x2E8 | SPE_SIM_REG_MAKE_WRITE_EX(20)) /* SPE TD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_BADDR_0_OFFSET                        (0x20C | SPE_SIM_REG_MAKE_WRITE_EX(22)) /* RD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_BADDR_1_OFFSET                        (0x22C | SPE_SIM_REG_MAKE_WRITE_EX(22)) /* RD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_BADDR_2_OFFSET                        (0x24C | SPE_SIM_REG_MAKE_WRITE_EX(22)) /* RD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_BADDR_3_OFFSET                        (0x26C | SPE_SIM_REG_MAKE_WRITE_EX(22)) /* RD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_BADDR_4_OFFSET                        (0x28C | SPE_SIM_REG_MAKE_WRITE_EX(22)) /* RD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_BADDR_5_OFFSET                        (0x2AC | SPE_SIM_REG_MAKE_WRITE_EX(22)) /* RD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_BADDR_6_OFFSET                        (0x2CC | SPE_SIM_REG_MAKE_WRITE_EX(22)) /* RD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_BADDR_7_OFFSET                        (0x2EC | SPE_SIM_REG_MAKE_WRITE_EX(22)) /* RD������ʼ��ַ��m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_LEN_0_OFFSET                          (0x210 | SPE_SIM_REG_MAKE_WRITE_EX(23)) /* RD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_LEN_1_OFFSET                          (0x230 | SPE_SIM_REG_MAKE_WRITE_EX(23)) /* RD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_LEN_2_OFFSET                          (0x250 | SPE_SIM_REG_MAKE_WRITE_EX(23)) /* RD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_LEN_3_OFFSET                          (0x270 | SPE_SIM_REG_MAKE_WRITE_EX(23)) /* RD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_LEN_4_OFFSET                          (0x290 | SPE_SIM_REG_MAKE_WRITE_EX(23)) /* RD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_LEN_5_OFFSET                          (0x2B0 | SPE_SIM_REG_MAKE_WRITE_EX(23)) /* RD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_LEN_6_OFFSET                          (0x2D0 | SPE_SIM_REG_MAKE_WRITE_EX(23)) /* RD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_LEN_7_OFFSET                          (0x2F0 | SPE_SIM_REG_MAKE_WRITE_EX(23)) /* RD������ȣ�m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_PTR_0_OFFSET                          (0x214 | SPE_SIM_REG_MAKE_WRITE_EX(24)) /* SPE RD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_PTR_1_OFFSET                          (0x234 | SPE_SIM_REG_MAKE_WRITE_EX(24)) /* SPE RD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_PTR_2_OFFSET                          (0x254 | SPE_SIM_REG_MAKE_WRITE_EX(24)) /* SPE RD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_PTR_3_OFFSET                          (0x274 | SPE_SIM_REG_MAKE_WRITE_EX(24)) /* SPE RD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_PTR_4_OFFSET                          (0x294 | SPE_SIM_REG_MAKE_WRITE_EX(24)) /* SPE RD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_PTR_5_OFFSET                          (0x2B4 | SPE_SIM_REG_MAKE_WRITE_EX(24)) /* SPE RD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_PTR_6_OFFSET                          (0x2D4 | SPE_SIM_REG_MAKE_WRITE_EX(24)) /* SPE RD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_RDQX_PTR_7_OFFSET                          (0x2F4 | SPE_SIM_REG_MAKE_WRITE_EX(24)) /* SPE RD����ָ�룬m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PRI_0_OFFSET                          (0x218 | SPE_SIM_REG_MAKE_WRITE_EX(21)) /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PRI_1_OFFSET                          (0x238 | SPE_SIM_REG_MAKE_WRITE_EX(21)) /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PRI_2_OFFSET                          (0x258 | SPE_SIM_REG_MAKE_WRITE_EX(21)) /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PRI_3_OFFSET                          (0x278 | SPE_SIM_REG_MAKE_WRITE_EX(21)) /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PRI_4_OFFSET                          (0x298 | SPE_SIM_REG_MAKE_WRITE_EX(21)) /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PRI_5_OFFSET                          (0x2B8 | SPE_SIM_REG_MAKE_WRITE_EX(21)) /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PRI_6_OFFSET                          (0x2D8 | SPE_SIM_REG_MAKE_WRITE_EX(21)) /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_TDQX_PRI_7_OFFSET                          (0x2F8 | SPE_SIM_REG_MAKE_WRITE_EX(21)) /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_CHX_PORTNUM_0_OFFSET                       (0x21C | SPE_SIM_REG_MAKE_WRITE_EX(15)) /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_CHX_PORTNUM_1_OFFSET                       (0x23C | SPE_SIM_REG_MAKE_WRITE_EX(15)) /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_CHX_PORTNUM_2_OFFSET                       (0x25C | SPE_SIM_REG_MAKE_WRITE_EX(15)) /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_CHX_PORTNUM_3_OFFSET                       (0x27C | SPE_SIM_REG_MAKE_WRITE_EX(15)) /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_CHX_PORTNUM_4_OFFSET                       (0x29C | SPE_SIM_REG_MAKE_WRITE_EX(15)) /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_CHX_PORTNUM_5_OFFSET                       (0x2BC | SPE_SIM_REG_MAKE_WRITE_EX(15)) /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_CHX_PORTNUM_6_OFFSET                       (0x2DC | SPE_SIM_REG_MAKE_WRITE_EX(15)) /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_CHX_PORTNUM_7_OFFSET                       (0x2FC | SPE_SIM_REG_MAKE_WRITE_EX(15)) /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */
#define    HI_SPE_PORTX_UDP_LMTNUM_0_OFFSET                  (0x300) /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_UDP_LMTNUM_1_OFFSET                  (0x340) /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_UDP_LMTNUM_2_OFFSET                  (0x380) /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_UDP_LMTNUM_3_OFFSET                  (0x3C0) /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_UDP_LMTNUM_4_OFFSET                  (0x400) /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_UDP_LMTNUM_5_OFFSET                  (0x440) /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_UDP_LMTNUM_6_OFFSET                  (0x480) /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_UDP_LMTNUM_7_OFFSET                  (0x4C0) /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_LMTBYTE_0_OFFSET                     (0x304) /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_LMTBYTE_1_OFFSET                     (0x344) /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_LMTBYTE_2_OFFSET                     (0x384) /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_LMTBYTE_3_OFFSET                     (0x3C4) /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_LMTBYTE_4_OFFSET                     (0x404) /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_LMTBYTE_5_OFFSET                     (0x444) /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_LMTBYTE_6_OFFSET                     (0x484) /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_LMTBYTE_7_OFFSET                     (0x4C4) /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_PROPERTY_0_OFFSET                    (0x308 | SPE_SIM_REG_MAKE_WRITE_EX(14)) /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */
#define    HI_SPE_PORTX_PROPERTY_1_OFFSET                    (0x348 | SPE_SIM_REG_MAKE_WRITE_EX(14)) /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */
#define    HI_SPE_PORTX_PROPERTY_2_OFFSET                    (0x388 | SPE_SIM_REG_MAKE_WRITE_EX(14)) /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */
#define    HI_SPE_PORTX_PROPERTY_3_OFFSET                    (0x3C8 | SPE_SIM_REG_MAKE_WRITE_EX(14)) /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */
#define    HI_SPE_PORTX_PROPERTY_4_OFFSET                    (0x408 | SPE_SIM_REG_MAKE_WRITE_EX(14)) /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */
#define    HI_SPE_PORTX_PROPERTY_5_OFFSET                    (0x448 | SPE_SIM_REG_MAKE_WRITE_EX(14)) /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */
#define    HI_SPE_PORTX_PROPERTY_6_OFFSET                    (0x488 | SPE_SIM_REG_MAKE_WRITE_EX(14)) /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */
#define    HI_SPE_PORTX_PROPERTY_7_OFFSET                    (0x4C8 | SPE_SIM_REG_MAKE_WRITE_EX(14)) /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_L_0_OFFSET                  (0x30C | SPE_SIM_REG_MAKE_WRITE_EX(17)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_L_1_OFFSET                  (0x34C | SPE_SIM_REG_MAKE_WRITE_EX(17)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_L_2_OFFSET                  (0x38C | SPE_SIM_REG_MAKE_WRITE_EX(17)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_L_3_OFFSET                  (0x3CC | SPE_SIM_REG_MAKE_WRITE_EX(17)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_L_4_OFFSET                  (0x40C | SPE_SIM_REG_MAKE_WRITE_EX(17)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_L_5_OFFSET                  (0x44C | SPE_SIM_REG_MAKE_WRITE_EX(17)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_L_6_OFFSET                  (0x48C | SPE_SIM_REG_MAKE_WRITE_EX(17)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_L_7_OFFSET                  (0x4CC | SPE_SIM_REG_MAKE_WRITE_EX(17)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_H_0_OFFSET                  (0x310 | SPE_SIM_REG_MAKE_WRITE_EX(16)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_H_1_OFFSET                  (0x350 | SPE_SIM_REG_MAKE_WRITE_EX(16)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_H_2_OFFSET                  (0x390 | SPE_SIM_REG_MAKE_WRITE_EX(16)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_H_3_OFFSET                  (0x3D0 | SPE_SIM_REG_MAKE_WRITE_EX(16)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_H_4_OFFSET                  (0x410 | SPE_SIM_REG_MAKE_WRITE_EX(16)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_H_5_OFFSET                  (0x450 | SPE_SIM_REG_MAKE_WRITE_EX(16)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_H_6_OFFSET                  (0x490 | SPE_SIM_REG_MAKE_WRITE_EX(16)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_MAC_ADDR_H_7_OFFSET                  (0x4D0 | SPE_SIM_REG_MAKE_WRITE_EX(16)) /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */
#define    HI_SPE_PORTX_IPV6_ADDR_1ST_0_OFFSET               (0x314) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_1ST_1_OFFSET               (0x354) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_1ST_2_OFFSET               (0x394) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_1ST_3_OFFSET               (0x3D4) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_1ST_4_OFFSET               (0x414) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_1ST_5_OFFSET               (0x454) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_1ST_6_OFFSET               (0x494) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_1ST_7_OFFSET               (0x4D4) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_2ND_0_OFFSET               (0x318) /* �˿�IPV6��ַ�ε�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_2ND_1_OFFSET               (0x358) /* �˿�IPV6��ַ�ε�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_2ND_2_OFFSET               (0x398) /* �˿�IPV6��ַ�ε�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_2ND_3_OFFSET               (0x3D8) /* �˿�IPV6��ַ�ε�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_2ND_4_OFFSET               (0x418) /* �˿�IPV6��ַ�ε�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_2ND_5_OFFSET               (0x458) /* �˿�IPV6��ַ�ε�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_2ND_6_OFFSET               (0x498) /* �˿�IPV6��ַ�ε�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_2ND_7_OFFSET               (0x4D8) /* �˿�IPV6��ַ�ε�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_3TH_0_OFFSET               (0x31C) /* �˿�IPV6��ַ�θ�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_3TH_1_OFFSET               (0x35C) /* �˿�IPV6��ַ�θ�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_3TH_2_OFFSET               (0x39C) /* �˿�IPV6��ַ�θ�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_3TH_3_OFFSET               (0x3DC) /* �˿�IPV6��ַ�θ�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_3TH_4_OFFSET               (0x41C) /* �˿�IPV6��ַ�θ�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_3TH_5_OFFSET               (0x45C) /* �˿�IPV6��ַ�θ�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_3TH_6_OFFSET               (0x49C) /* �˿�IPV6��ַ�θ�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_3TH_7_OFFSET               (0x4DC) /* �˿�IPV6��ַ�θ�32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_4TH_0_OFFSET               (0x320) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_4TH_1_OFFSET               (0x360) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_4TH_2_OFFSET               (0x3A0) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_4TH_3_OFFSET               (0x3E0) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_4TH_4_OFFSET               (0x420) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_4TH_5_OFFSET               (0x460) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_4TH_6_OFFSET               (0x4A0) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_ADDR_4TH_7_OFFSET               (0x4E0) /* �˿�IPV6��ַ���32λ */
#define    HI_SPE_PORTX_IPV6_MASK_0_OFFSET                   (0x324) /* �˿�IPV6��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV6_MASK_1_OFFSET                   (0x364) /* �˿�IPV6��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV6_MASK_2_OFFSET                   (0x3A4) /* �˿�IPV6��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV6_MASK_3_OFFSET                   (0x3E4) /* �˿�IPV6��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV6_MASK_4_OFFSET                   (0x424) /* �˿�IPV6��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV6_MASK_5_OFFSET                   (0x464) /* �˿�IPV6��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV6_MASK_6_OFFSET                   (0x4A4) /* �˿�IPV6��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV6_MASK_7_OFFSET                   (0x4E4) /* �˿�IPV6��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV4_ADDR_0_OFFSET                   (0x328) /* �˿�IPV4��ַ */
#define    HI_SPE_PORTX_IPV4_ADDR_1_OFFSET                   (0x368) /* �˿�IPV4��ַ */
#define    HI_SPE_PORTX_IPV4_ADDR_2_OFFSET                   (0x3A8) /* �˿�IPV4��ַ */
#define    HI_SPE_PORTX_IPV4_ADDR_3_OFFSET                   (0x3E8) /* �˿�IPV4��ַ */
#define    HI_SPE_PORTX_IPV4_ADDR_4_OFFSET                   (0x428) /* �˿�IPV4��ַ */
#define    HI_SPE_PORTX_IPV4_ADDR_5_OFFSET                   (0x468) /* �˿�IPV4��ַ */
#define    HI_SPE_PORTX_IPV4_ADDR_6_OFFSET                   (0x4A8) /* �˿�IPV4��ַ */
#define    HI_SPE_PORTX_IPV4_ADDR_7_OFFSET                   (0x4E8) /* �˿�IPV4��ַ */
#define    HI_SPE_PORTX_IPV4_MASK_0_OFFSET                   (0x32C) /* �˿�IPV4��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV4_MASK_1_OFFSET                   (0x36C) /* �˿�IPV4��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV4_MASK_2_OFFSET                   (0x3AC) /* �˿�IPV4��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV4_MASK_3_OFFSET                   (0x3EC) /* �˿�IPV4��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV4_MASK_4_OFFSET                   (0x42C) /* �˿�IPV4��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV4_MASK_5_OFFSET                   (0x46C) /* �˿�IPV4��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV4_MASK_6_OFFSET                   (0x4AC) /* �˿�IPV4��ַMASK�Ĵ��� */
#define    HI_SPE_PORTX_IPV4_MASK_7_OFFSET                   (0x4EC) /* �˿�IPV4��ַMASK�Ĵ��� */
#define    HI_SPE_TAB_CTRL_OFFSET                            (0x500 | SPE_SIM_REG_MAKE_WRITE_EX(8)) /* SPE��������Ĵ��� */
#define    HI_SPE_TAB_ACT_RESULT_OFFSET                      (0x504) /* SPE���������� */
#define    HI_SPE_TAB_CONTENT_0_OFFSET                       (0x508) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_1_OFFSET                       (0x50C) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_2_OFFSET                       (0x510) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_3_OFFSET                       (0x514) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_4_OFFSET                       (0x518) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_5_OFFSET                       (0x51C) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_6_OFFSET                       (0x520) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_7_OFFSET                       (0x524) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_8_OFFSET                       (0x528) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_9_OFFSET                       (0x52C) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_10_OFFSET                      (0x530) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_11_OFFSET                      (0x534) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_12_OFFSET                      (0x538) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_13_OFFSET                      (0x53C) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_TAB_CONTENT_14_OFFSET                      (0x540) /* SPE�������ݼĴ�����һ��15�� */
#define    HI_SPE_HASH_BADDR_OFFSET                          (0x580 | SPE_SIM_REG_MAKE_WRITE_EX(4)) /* HASH�������ַ */
#define    HI_SPE_HASH_ZONE_OFFSET                           (0x584 | SPE_SIM_REG_MAKE_WRITE_EX(5)) /* ���Ӹ����������ڹ�ϣ���� */
#define    HI_SPE_HASH_RAND_OFFSET                           (0x588 | SPE_SIM_REG_MAKE_WRITE_EX(6)) /* ���Ӹ��ٹ�ϣ����� */
#define    HI_SPE_HASH_L3_PROTO_OFFSET                       (0x58C) /* �����ַ�� */
#define    HI_SPE_HASH_WIDTH_OFFSET                          (0x590 | SPE_SIM_REG_MAKE_WRITE_EX(7)) /* HASH��������ȼĴ����� */
#define    HI_SPE_HASH_DEPTH_OFFSET                          (0x594 | SPE_SIM_REG_MAKE_WRITE_EX(12)) /* HASHͰ�����üĴ��� */
#define    HI_SPE_AGING_TIME_OFFSET                          (0x5A0 | SPE_SIM_REG_MAKE_WRITE_EX(11)) /* �����ϻ�ʱ��Ĵ��� */
#define    HI_SPE_PACK_MAX_TIME_OFFSET                       (0x600) /* USB���ʱ�����ƼĴ��� */
#define    HI_SPE_PACK_CTRL_OFFSET                           (0x604 | SPE_SIM_REG_MAKE_WRITE_EX(3)) /* USB������ƼĴ��� */
#define    HI_SPE_PACK_ADDR_CTRL_OFFSET                      (0x608 | SPE_SIM_REG_MAKE_WRITE_EX(29)) /* NCM�������Ĵ��� */
#define    HI_SPE_PACK_REMAIN_LEN_OFFSET                     (0x60C) /* USB�������Ԥ���ռ�Ĵ��� */
#define    HI_SPE_UNPACK_CTRL_OFFSET                         (0x610 | SPE_SIM_REG_MAKE_WRITE_EX(30)) /* NCM��װ��󳤶�Լ���Ĵ��� */
#define    HI_SPE_UNPACK_MAX_LEN_OFFSET                      (0x614) /* USB��װ��󳤶����üĴ���������Լ��TD�е�pkt_len�ֶΡ� */
#define    HI_SPE_EVENT_BUFF0_PTR_OFFSET                     (0x620 | SPE_SIM_REG_MAKE_READ_EX(31)) /* eventbuffer0��дָ�� */
#define    HI_SPE_EVENT_BUFF0_ADDR_OFFSET                    (0x624) /* eventbuffer0��ʼ��ַ��ƫ�Ƶ�ַ */
#define    HI_SPE_EVENT_BUFF1_PTR_OFFSET                     (0x628) /* eventbuffer1��дָ�� */
#define    HI_SPE_EVENT_BUFF1_ADDR_OFFSET                    (0x62C) /* eventbuffer1��ʼ��ַ��ƫ�Ƶ�ַ */
#define    HI_SPE_EVENT_BUFF_LEN_OFFSET                      (0x630) /* eventbuffer������üĴ��� */
#define    HI_SPE_INT0_INTERVAL_OFFSET                       (0x634 | SPE_SIM_REG_MAKE_WRITE_EX(9)) /* eventbuffer0�жϼ��ʱ�� */
#define    HI_SPE_INT1_INTERVAL_OFFSET                       (0x638) /* eventbuffer1�жϼ��ʱ�� */
#define    HI_SPE_EVENT_BUFF0_MASK_OFFSET                    (0x63C) /* eventbuffer0�ж�MASK���ƼĴ��� */
#define    HI_SPE_EVENT_BUFF1_MASK_OFFSET                    (0x640) /* eventbuffer1�ж�MASK���ƼĴ��� */
#define    HI_SPE_EVENTBUF0_PRO_CNT_OFFSET                   (0x644 | SPE_SIM_REG_MAKE_WRITE_EX(10)) /* �������EVENTBUFFER0��Чevent�¼����� */
#define    HI_SPE_EVENTBUF0_RPT_CNT_OFFSET                   (0x648) /* оƬ�ϱ�EVENTBUFFER0��Чevent�¼����� */
#define    HI_SPE_EVENTBUF1_PRO_CNT_OFFSET                   (0x64C) /* �������EVENTBUFFER1��Чevent�¼����� */
#define    HI_SPE_EVENTBUF1_RPT_CNT_OFFSET                   (0x650) /* оƬ�ϱ�EVENTBUFFER1��Чevent�¼����� */
#define    HI_SPE_INT_STATE_OFFSET                           (0x654) /* �ж�״̬�������ж������ĸ�eventbuffer */
#define    HI_SPE_EVENT_BUFF_FULL_OFFSET                     (0x658) /* eventbuffer��״̬�ϱ� */
#define    HI_SPE_EVENT_BUFF0_RD_ERR_OFFSET                  (0x65C) /* �����eventbuffer0��� */
#define    HI_SPE_EVENT_BUFF1_RD_ERR_OFFSET                  (0x660) /* �����eventbuffer1��� */
#define    HI_SPE_WRR_DEBUG_OFFSET                           (0x700) /* spe_wrrģ����ԼĴ��� */
#define    HI_SPE_UNPACK_DEBUG_OFFSET                        (0x710) /* spe_unpackģ����ԼĴ��� */
#define    HI_SPE_ENG_DEBUG_OFFSET                           (0x720) /* spe_engģ����ԼĴ��� */
#define    HI_SPE_PACK_DEBUG_OFFSET                          (0x730) /* spe_packģ����ԼĴ��� */
#define    HI_SPE_DONE_DEBUG_OFFSET                          (0x740) /* spe_doneģ����ԼĴ��� */
#define    HI_SPE_IF_DEBUG_OFFSET                            (0x750) /* spe_ifģ����ԼĴ��� */
#define    HI_SPE_LP_RD_BLOCK_CNT_OFFSET                     (0x780) /* �˿�0~3�������кŶ˿ڣ�RD����ͳ�ƼĴ��� */
#define    HI_SPE_HP_RD_BLOCK_CNT_OFFSET                     (0x784) /* �˿�4~7�������кŶ˿ڣ�RD����ͳ�ƼĴ��� */
#define    HI_SPE_PACK_OVERTIME_CNT_OFFSET                   (0x788) /* USB�����ʱͳ�ƼĴ��� */
#define    HI_SPE_PACK_PUSH_CNT_OFFSET                       (0x78C) /* USB���PUSHͳ�ƼĴ��� */
#define    HI_SPE_DEPACK_ERR_CNT_OFFSET                      (0x790) /* USB��װ��ʽ��������ͳ�ƼĴ��� */


#ifndef __ASSEMBLY__

/********************************************************************************/
/*    SPE_MPI �Ĵ������壨��Ŀ��_ģ����_�Ĵ�����_T)        */
/********************************************************************************/
typedef union
{
    struct
    {
        unsigned int    spe_en                     : 1; /* [0..0] SPE����������ʹ�ܡ�1'b0����ʹ�ܣ�1'b1��ʹ�ܡ� */
        unsigned int    reserved                   : 31; /* [31..1] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_EN_T;    /* SPE����������ʹ�ܼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_usrfield_ctrl          : 3; /* [2..0] TD/RD ��user field˽������չ��������WORDΪ��λ�� */
        unsigned int    reserved                   : 29; /* [31..3] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_USRFIELD_CTRL_T;    /* SPEȫ�ֿ��ƼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_mac_black_white        : 1; /* [0..0] MAC���˱�����������ԡ�1'b0����������1'b1���������� */
        unsigned int    spe_ip_black_white         : 1; /* [1..1] IP���˱�����������ԡ�1'b0����������1'b1���������� */
        unsigned int    reserved                   : 30; /* [31..2] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_BLACK_WHITE_T;    /* �ڰ�����ָʾ�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_udp_lmttime       : 8; /* [7..0] NA */
        unsigned int    spe_port_lmttime           : 8; /* [15..8] �˿���������ʱ�����üĴ������ڸ�ʱ���ڣ��Ա��˿ڽ��յ���̫���ֽ����������ơ�WRR����ǰ�����б𣬶˿ڳɹ����Ⱥ󣬾��������������ƣ�Ҳ��Ҫ����ǰ���ȶ˿ڴ�����ɡ����Ϊ0����ʾ�������������ơ���32.768kHzʱ�����ڵ�2^5����0.976ms��Ϊ��λ������UDP��������ʱ�䣬�ڸ�ʱ���ڣ��Ա��˿ڽ��յ�UDP���������������������Ϊ0����ʾ������UDP���������ơ���32.768kHzʱ�����ڵ�2^5����0.976ms��Ϊ��λ�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORT_LMTTIME_T;    /* �˿�������UDPС���������ʱ�����üĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_eth_minlen             : 11; /* [10..0] ������̫���ж����ޣ����ֽ�Ϊ��λ�� */
        unsigned int    reserved                   : 21; /* [31..11] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_ETH_MINLEN_T;    /* ������̫���ж����� */

typedef union
{
    struct
    {
        unsigned int    spe_stick_sportnum         : 3; /* [2..0] STICKģʽ�µ�Դ�˿ںżĴ����� */
        unsigned int    reserved_2                 : 1; /* [3..3] ������ */
        unsigned int    spe_stick_dportnum         : 3; /* [6..4] STICKģʽ�µ�Ŀ�Ķ˿ںżĴ����� */
        unsigned int    reserved_1                 : 1; /* [7..7] ������ */
        unsigned int    spe_mode                   : 1; /* [8..8] STICKģʽָʾ�Ĵ�����1'b0����STICKģʽ��1'b1��STICKģʽ�� */
        unsigned int    reserved_0                 : 23; /* [31..9] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_MODE_T;    /* STICKģʽָʾ�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_dbgen                  : 1; /* [0..0] ��Ȩģʽ�Ĵ�����1'b0����ͨģʽ��1'b1����Ȩģʽ */
        unsigned int    reserved                   : 31; /* [31..1] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_DBGEN_T;    /* ��Ȩģʽ�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_glb_dmac_addr_l        : 32; /* [31..0] SPEȫ��Ŀ��MAC��λ��ַ���üĴ�����ΪIPF�������MACʱʹ�á� */
    } bits;
    unsigned int    u32;
}HI_SPE_GLB_DMAC_ADDR_L_T;    /* SPEȫ��Ŀ��MAC��λ��ַ���üĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_glb_dmac_addr_h        : 16; /* [15..0] SPEȫ��Ŀ��MAC��λ��ַ���üĴ�����ΪIPF�������MACʱʹ�á� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_GLB_DMAC_ADDR_H_T;    /* SPEȫ��Ŀ��MAC��λ��ַ���üĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_glb_smac_addr_l        : 32; /* [31..0] SPEȫ��ԴMAC��λ��ַ���üĴ�����ΪIPF�������MACʱʹ�á� */
    } bits;
    unsigned int    u32;
}HI_SPE_GLB_SMAC_ADDR_L_T;    /* SPEȫ��ԴMAC��λ��ַ���üĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_glb_smac_addr_h        : 16; /* [15..0] SPEȫ��ԴMAC�͸ߵ�ַ���üĴ�����ΪIPF�������MACʱʹ�á� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_GLB_SMAC_ADDR_H_T;    /* SPEȫ��ԴMAC��λ��ַ���üĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_rd_burstlen            : 4; /* [3..0] ����̫���Ĳ�����ȣ���64bitΪ��λ��spe_rd_burstlen+1Ϊʵ�ʳ��ȡ� */
        unsigned int    reserved                   : 28; /* [31..4] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_RD_BURSTLEN_T;    /* SPE����burst���ȼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_0_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_1_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_2_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_3_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_4_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_5_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_6_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_7_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_8_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_9_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_10_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_11_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_12_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_13_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_14_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_l4_portnum_cfg         : 18; /* [17..0] bit[17]��UDPָʾ��1'b1����Ч��1'b0����Ч��Bit[16]��TCPָʾ��1'b1����Ч��1'b0����Ч��Bit[15:0]��ƥ��˿ںš� */
        unsigned int    reserved                   : 14; /* [31..18] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_L4_PORTNUM_CFG_15_T;    /* TCP/UDP�˿�ƥ����˼Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_cnt_clr                : 1; /* [0..0] ͳ������Ĵ�����1'b0����ʹ�����㹦�ܣ�1'b1��ʹ�����㹦�ܣ� */
        unsigned int    reserved                   : 31; /* [31..1] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_CNT_CLR_T;    /* ͳ������Ĵ����� */

typedef union
{
    struct
    {
        unsigned int    spe_ver                    : 32; /* [31..0] SPE�汾�żĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_VER_T;    /* SPE�汾�żĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_time                   : 16; /* [15..0] SPE������ʱ���ϱ��Ĵ�������32.768kHzʱ�����ڵ�2^10����31.2ms��Ϊ��λ�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TIME_T;    /* SPE������ʱ���ϱ��Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_idle                   : 1; /* [0..0] ���ж��пգ���SPE���У��ߵ�ƽ��Ч�� */
        unsigned int    reserved                   : 31; /* [31..1] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_IDLE_T;    /* SPE���б�־ */

typedef union
{
    struct
    {
        unsigned int    spe_gm_rd_osrc_lmt         : 2; /* [1..0] ��ͨ��outstanding��ȡ� */
        unsigned int    spe_gm_wr_osrc_lmt         : 2; /* [3..2] дͨ��outstanding��ȡ� */
        unsigned int    spe_rd_ost_en              : 1; /* [4..4] ��ͨ��outstandingʹ�ܡ�1'b0����ʹ�ܣ�1'b1��ʹ�ܡ� */
        unsigned int    spe_wr_ost_en              : 1; /* [5..5] дͨ��outstandingʹ�ܡ�1'b0����ʹ�ܣ�1'b1��ʹ�ܡ� */
        unsigned int    reserved                   : 26; /* [31..6] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_OST_CTRL_T;    /* SPE������outstanding���ƼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_gm_enburst_undef       : 1; /* [0..0] ���̶��������ָʾ��1'b0����Ч��1'b1����Ч�� */
        unsigned int    spe_gm_enburst4            : 1; /* [1..1] ��������Ƿ�̶�Ϊ4��trsnsferָʾ��1'b0�����̶���1'b1���̶��� */
        unsigned int    spe_gm_enburst8            : 1; /* [2..2] ��������Ƿ�̶�Ϊ8��transferָʾ��1'b0�����̶���1'b1���̶��� */
        unsigned int    spe_gm_enburst16           : 1; /* [3..3] ��������Ƿ�̶�Ϊ16��transferָʾ��1'b0�����̶���1'b1���̶��� */
        unsigned int    reserved                   : 28; /* [31..4] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_BURST_CTRL_T;    /* SPE������burst���ƼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_wch0_id                : 3; /* [2..0] дͨ��0 ID�� */
        unsigned int    spe_wch1_id                : 3; /* [5..3] дͨ��1 ID�� */
        unsigned int    spe_wch2_id                : 3; /* [8..6] дͨ��2 ID�� */
        unsigned int    spe_wch3_id                : 3; /* [11..9] дͨ��3 ID�� */
        unsigned int    reserved                   : 20; /* [31..12] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_WCH_ID_T;    /* дͨ��ID�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_rch0_id                : 3; /* [2..0] ��ͨ��0 ID�� */
        unsigned int    spe_rch1_id                : 3; /* [5..3] ��ͨ��1 ID�� */
        unsigned int    spe_rch2_id                : 3; /* [8..6] ��ͨ��2 ID�� */
        unsigned int    spe_rch3_id                : 3; /* [11..9] ��ͨ��3 ID�� */
        unsigned int    spe_rch4_id                : 3; /* [14..12] ��ͨ��4 ID�� */
        unsigned int    reserved                   : 17; /* [31..15] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_RCH_ID_T;    /* ��ͨ��ID�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_wch0_pri               : 3; /* [2..0] дͨ��0���ȼ��� */
        unsigned int    spe_wch1_pri               : 3; /* [5..3] дͨ��2���ȼ��� */
        unsigned int    spe_wch2_pri               : 3; /* [8..6] дͨ��2���ȼ��� */
        unsigned int    spe_wch3_pri               : 3; /* [11..9] дͨ��3���ȼ��� */
        unsigned int    reserved                   : 20; /* [31..12] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_WCH_PRI_T;    /* дͨ�����ȼ��Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_rch0_pri               : 3; /* [2..0] ��ͨ��0���ȼ��� */
        unsigned int    spe_rch1_pri               : 3; /* [5..3] ��ͨ��1���ȼ��� */
        unsigned int    spe_rch2_pri               : 3; /* [8..6] ��ͨ��2���ȼ��� */
        unsigned int    spe_rch3_pri               : 3; /* [11..9] ��ͨ��3���ȼ��� */
        unsigned int    spe_rch4_pri               : 3; /* [14..12] ��ͨ��4���ȼ��� */
        unsigned int    reserved                   : 17; /* [31..15] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_RCH_PRI_T;    /* ��ͨ�����ȼ��Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_baddr              : 32; /* [31..0] TD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_BADDR_0_T;    /* TD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_baddr              : 32; /* [31..0] TD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_BADDR_1_T;    /* TD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_baddr              : 32; /* [31..0] TD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_BADDR_2_T;    /* TD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_baddr              : 32; /* [31..0] TD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_BADDR_3_T;    /* TD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_baddr              : 32; /* [31..0] TD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_BADDR_4_T;    /* TD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_baddr              : 32; /* [31..0] TD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_BADDR_5_T;    /* TD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_baddr              : 32; /* [31..0] TD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_BADDR_6_T;    /* TD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_baddr              : 32; /* [31..0] TD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_BADDR_7_T;    /* TD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_len                : 16; /* [15..0] TD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_LEN_0_T;    /* TD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_len                : 16; /* [15..0] TD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_LEN_1_T;    /* TD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_len                : 16; /* [15..0] TD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_LEN_2_T;    /* TD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_len                : 16; /* [15..0] TD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_LEN_3_T;    /* TD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_len                : 16; /* [15..0] TD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_LEN_4_T;    /* TD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_len                : 16; /* [15..0] TD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_LEN_5_T;    /* TD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_len                : 16; /* [15..0] TD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_LEN_6_T;    /* TD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_len                : 16; /* [15..0] TD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_LEN_7_T;    /* TD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_wptr               : 16; /* [15..0] TD����дָ�룬������TD�������ά���� */
        unsigned int    spe_tdq_rptr               : 16; /* [31..16] TD���ж�ָ�룬������TD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PTR_0_T;    /* SPE TD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_wptr               : 16; /* [15..0] TD����дָ�룬������TD�������ά���� */
        unsigned int    spe_tdq_rptr               : 16; /* [31..16] TD���ж�ָ�룬������TD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PTR_1_T;    /* SPE TD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_wptr               : 16; /* [15..0] TD����дָ�룬������TD�������ά���� */
        unsigned int    spe_tdq_rptr               : 16; /* [31..16] TD���ж�ָ�룬������TD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PTR_2_T;    /* SPE TD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_wptr               : 16; /* [15..0] TD����дָ�룬������TD�������ά���� */
        unsigned int    spe_tdq_rptr               : 16; /* [31..16] TD���ж�ָ�룬������TD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PTR_3_T;    /* SPE TD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_wptr               : 16; /* [15..0] TD����дָ�룬������TD�������ά���� */
        unsigned int    spe_tdq_rptr               : 16; /* [31..16] TD���ж�ָ�룬������TD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PTR_4_T;    /* SPE TD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_wptr               : 16; /* [15..0] TD����дָ�룬������TD�������ά���� */
        unsigned int    spe_tdq_rptr               : 16; /* [31..16] TD���ж�ָ�룬������TD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PTR_5_T;    /* SPE TD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_wptr               : 16; /* [15..0] TD����дָ�룬������TD�������ά���� */
        unsigned int    spe_tdq_rptr               : 16; /* [31..16] TD���ж�ָ�룬������TD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PTR_6_T;    /* SPE TD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_wptr               : 16; /* [15..0] TD����дָ�룬������TD�������ά���� */
        unsigned int    spe_tdq_rptr               : 16; /* [31..16] TD���ж�ָ�룬������TD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PTR_7_T;    /* SPE TD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_baddr              : 32; /* [31..0] RD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_BADDR_0_T;    /* RD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_baddr              : 32; /* [31..0] RD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_BADDR_1_T;    /* RD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_baddr              : 32; /* [31..0] RD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_BADDR_2_T;    /* RD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_baddr              : 32; /* [31..0] RD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_BADDR_3_T;    /* RD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_baddr              : 32; /* [31..0] RD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_BADDR_4_T;    /* RD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_baddr              : 32; /* [31..0] RD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_BADDR_5_T;    /* RD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_baddr              : 32; /* [31..0] RD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_BADDR_6_T;    /* RD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_baddr              : 32; /* [31..0] RD������ʼ��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_BADDR_7_T;    /* RD������ʼ��ַ��m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_len                : 16; /* [15..0] RD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_LEN_0_T;    /* RD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_len                : 16; /* [15..0] RD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_LEN_1_T;    /* RD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_len                : 16; /* [15..0] RD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_LEN_2_T;    /* RD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_len                : 16; /* [15..0] RD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_LEN_3_T;    /* RD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_len                : 16; /* [15..0] RD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_LEN_4_T;    /* RD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_len                : 16; /* [15..0] RD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_LEN_5_T;    /* RD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_len                : 16; /* [15..0] RD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_LEN_6_T;    /* RD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_len                : 16; /* [15..0] RD������ȣ�������TD�� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_LEN_7_T;    /* RD������ȣ�m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_wptr               : 16; /* [15..0] RD����дָ�룬������RD�������ά���� */
        unsigned int    spe_rdq_rptr               : 16; /* [31..16] RD���ж�ָ�룬������RD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_PTR_0_T;    /* SPE RD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_wptr               : 16; /* [15..0] RD����дָ�룬������RD�������ά���� */
        unsigned int    spe_rdq_rptr               : 16; /* [31..16] RD���ж�ָ�룬������RD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_PTR_1_T;    /* SPE RD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_wptr               : 16; /* [15..0] RD����дָ�룬������RD�������ά���� */
        unsigned int    spe_rdq_rptr               : 16; /* [31..16] RD���ж�ָ�룬������RD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_PTR_2_T;    /* SPE RD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_wptr               : 16; /* [15..0] RD����дָ�룬������RD�������ά���� */
        unsigned int    spe_rdq_rptr               : 16; /* [31..16] RD���ж�ָ�룬������RD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_PTR_3_T;    /* SPE RD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_wptr               : 16; /* [15..0] RD����дָ�룬������RD�������ά���� */
        unsigned int    spe_rdq_rptr               : 16; /* [31..16] RD���ж�ָ�룬������RD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_PTR_4_T;    /* SPE RD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_wptr               : 16; /* [15..0] RD����дָ�룬������RD�������ά���� */
        unsigned int    spe_rdq_rptr               : 16; /* [31..16] RD���ж�ָ�룬������RD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_PTR_5_T;    /* SPE RD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_wptr               : 16; /* [15..0] RD����дָ�룬������RD�������ά���� */
        unsigned int    spe_rdq_rptr               : 16; /* [31..16] RD���ж�ָ�룬������RD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_PTR_6_T;    /* SPE RD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_rdq_wptr               : 16; /* [15..0] RD����дָ�룬������RD�������ά���� */
        unsigned int    spe_rdq_rptr               : 16; /* [31..16] RD���ж�ָ�룬������RD����оƬά���� */
    } bits;
    unsigned int    u32;
}HI_SPE_RDQX_PTR_7_T;    /* SPE RD����ָ�룬m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_pri                : 8; /* [7..0] ͨ��Ȩ�ؼĴ�����WRR����ʱʹ�ã���TDΪ���ȡ� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PRI_0_T;    /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_pri                : 8; /* [7..0] ͨ��Ȩ�ؼĴ�����WRR����ʱʹ�ã���TDΪ���ȡ� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PRI_1_T;    /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_pri                : 8; /* [7..0] ͨ��Ȩ�ؼĴ�����WRR����ʱʹ�ã���TDΪ���ȡ� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PRI_2_T;    /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_pri                : 8; /* [7..0] ͨ��Ȩ�ؼĴ�����WRR����ʱʹ�ã���TDΪ���ȡ� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PRI_3_T;    /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_pri                : 8; /* [7..0] ͨ��Ȩ�ؼĴ�����WRR����ʱʹ�ã���TDΪ���ȡ� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PRI_4_T;    /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_pri                : 8; /* [7..0] ͨ��Ȩ�ؼĴ�����WRR����ʱʹ�ã���TDΪ���ȡ� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PRI_5_T;    /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_pri                : 8; /* [7..0] ͨ��Ȩ�ؼĴ�����WRR����ʱʹ�ã���TDΪ���ȡ� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PRI_6_T;    /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_tdq_pri                : 8; /* [7..0] ͨ��Ȩ�ؼĴ�����WRR����ʱʹ�ã���TDΪ���ȡ� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TDQX_PRI_7_T;    /* ͨ��Ȩ�ؼĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_ch_portnum             : 3; /* [2..0] SPE���ж�Ӧ�Ķ˿ںš� */
        unsigned int    reserved                   : 29; /* [31..3] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_CHX_PORTNUM_0_T;    /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_ch_portnum             : 3; /* [2..0] SPE���ж�Ӧ�Ķ˿ںš� */
        unsigned int    reserved                   : 29; /* [31..3] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_CHX_PORTNUM_1_T;    /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_ch_portnum             : 3; /* [2..0] SPE���ж�Ӧ�Ķ˿ںš� */
        unsigned int    reserved                   : 29; /* [31..3] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_CHX_PORTNUM_2_T;    /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_ch_portnum             : 3; /* [2..0] SPE���ж�Ӧ�Ķ˿ںš� */
        unsigned int    reserved                   : 29; /* [31..3] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_CHX_PORTNUM_3_T;    /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_ch_portnum             : 3; /* [2..0] SPE���ж�Ӧ�Ķ˿ںš� */
        unsigned int    reserved                   : 29; /* [31..3] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_CHX_PORTNUM_4_T;    /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_ch_portnum             : 3; /* [2..0] SPE���ж�Ӧ�Ķ˿ںš� */
        unsigned int    reserved                   : 29; /* [31..3] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_CHX_PORTNUM_5_T;    /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_ch_portnum             : 3; /* [2..0] SPE���ж�Ӧ�Ķ˿ںš� */
        unsigned int    reserved                   : 29; /* [31..3] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_CHX_PORTNUM_6_T;    /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_ch_portnum             : 3; /* [2..0] SPE���ж�Ӧ�Ķ˿ںš� */
        unsigned int    reserved                   : 29; /* [31..3] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_CHX_PORTNUM_7_T;    /* SPE���а󶨵Ķ˿ںżĴ�����m������кţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_udp_lmtnum        : 32; /* [31..0] ����ʱ���ڣ����˿ڽ���UDP���������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_UDP_LMTNUM_0_T;    /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_udp_lmtnum        : 32; /* [31..0] ����ʱ���ڣ����˿ڽ���UDP���������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_UDP_LMTNUM_1_T;    /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_udp_lmtnum        : 32; /* [31..0] ����ʱ���ڣ����˿ڽ���UDP���������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_UDP_LMTNUM_2_T;    /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_udp_lmtnum        : 32; /* [31..0] ����ʱ���ڣ����˿ڽ���UDP���������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_UDP_LMTNUM_3_T;    /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_udp_lmtnum        : 32; /* [31..0] ����ʱ���ڣ����˿ڽ���UDP���������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_UDP_LMTNUM_4_T;    /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_udp_lmtnum        : 32; /* [31..0] ����ʱ���ڣ����˿ڽ���UDP���������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_UDP_LMTNUM_5_T;    /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_udp_lmtnum        : 32; /* [31..0] ����ʱ���ڣ����˿ڽ���UDP���������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_UDP_LMTNUM_6_T;    /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_udp_lmtnum        : 32; /* [31..0] ����ʱ���ڣ����˿ڽ���UDP���������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_UDP_LMTNUM_7_T;    /* �˿�UDPС����������������üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_lmtbyte           : 32; /* [31..0] ����ʱ���ڣ����˿ڽ�����̫���ֽ������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_LMTBYTE_0_T;    /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_lmtbyte           : 32; /* [31..0] ����ʱ���ڣ����˿ڽ�����̫���ֽ������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_LMTBYTE_1_T;    /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_lmtbyte           : 32; /* [31..0] ����ʱ���ڣ����˿ڽ�����̫���ֽ������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_LMTBYTE_2_T;    /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_lmtbyte           : 32; /* [31..0] ����ʱ���ڣ����˿ڽ�����̫���ֽ������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_LMTBYTE_3_T;    /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_lmtbyte           : 32; /* [31..0] ����ʱ���ڣ����˿ڽ�����̫���ֽ������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_LMTBYTE_4_T;    /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_lmtbyte           : 32; /* [31..0] ����ʱ���ڣ����˿ڽ�����̫���ֽ������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_LMTBYTE_5_T;    /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_lmtbyte           : 32; /* [31..0] ����ʱ���ڣ����˿ڽ�����̫���ֽ������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_LMTBYTE_6_T;    /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_lmtbyte           : 32; /* [31..0] ����ʱ���ڣ����˿ڽ�����̫���ֽ������ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_LMTBYTE_7_T;    /* �˿����������ֽ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_attach_brg        : 2; /* [1..0] �˿ڸ������ԼĴ�����2'd0�����˿��������¹Ҷ˿ڣ�2'd1�����˿�����ͨ�˿ڣ�2'd2�����˿�Ϊ���Ӷ˿ڣ�2'd3�������� */
        unsigned int    spe_port_enc_type          : 4; /* [5..2] �˿ڱ��ĸ�ʽ��4'd0���޷����4'd1��NCM_NTH16�����4'd2��NCM_NTH32�����4'd3��RNDIS�����4'd4��WIFI���ݣ�4'd5��IPF��4'd6��CPU����˿ڣ������������� */
        unsigned int    spe_port_pad_en            : 1; /* [6..6] padding���ʹ�ܡ�1'b0����ʹ�ܣ�1'b1��ʹ�ܡ� */
        unsigned int    reserved                   : 24; /* [30..7] ������ */
        unsigned int    spe_port_en                : 1; /* [31..31] �˿�ʹ�ܼĴ�����1'b0���˿ڲ�ʹ�ܣ�1'b1���˿�ʹ�ܡ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_PROPERTY_0_T;    /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_attach_brg        : 2; /* [1..0] �˿ڸ������ԼĴ�����2'd0�����˿��������¹Ҷ˿ڣ�2'd1�����˿�����ͨ�˿ڣ�2'd2�����˿�Ϊ���Ӷ˿ڣ�2'd3�������� */
        unsigned int    spe_port_enc_type          : 4; /* [5..2] �˿ڱ��ĸ�ʽ��4'd0���޷����4'd1��NCM_NTH16�����4'd2��NCM_NTH32�����4'd3��RNDIS�����4'd4��WIFI���ݣ�4'd5��IPF��4'd6��CPU����˿ڣ������������� */
        unsigned int    spe_port_pad_en            : 1; /* [6..6] padding���ʹ�ܡ�1'b0����ʹ�ܣ�1'b1��ʹ�ܡ� */
        unsigned int    reserved                   : 24; /* [30..7] ������ */
        unsigned int    spe_port_en                : 1; /* [31..31] �˿�ʹ�ܼĴ�����1'b0���˿ڲ�ʹ�ܣ�1'b1���˿�ʹ�ܡ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_PROPERTY_1_T;    /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_attach_brg        : 2; /* [1..0] �˿ڸ������ԼĴ�����2'd0�����˿��������¹Ҷ˿ڣ�2'd1�����˿�����ͨ�˿ڣ�2'd2�����˿�Ϊ���Ӷ˿ڣ�2'd3�������� */
        unsigned int    spe_port_enc_type          : 4; /* [5..2] �˿ڱ��ĸ�ʽ��4'd0���޷����4'd1��NCM_NTH16�����4'd2��NCM_NTH32�����4'd3��RNDIS�����4'd4��WIFI���ݣ�4'd5��IPF��4'd6��CPU����˿ڣ������������� */
        unsigned int    spe_port_pad_en            : 1; /* [6..6] padding���ʹ�ܡ�1'b0����ʹ�ܣ�1'b1��ʹ�ܡ� */
        unsigned int    reserved                   : 24; /* [30..7] ������ */
        unsigned int    spe_port_en                : 1; /* [31..31] �˿�ʹ�ܼĴ�����1'b0���˿ڲ�ʹ�ܣ�1'b1���˿�ʹ�ܡ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_PROPERTY_2_T;    /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_attach_brg        : 2; /* [1..0] �˿ڸ������ԼĴ�����2'd0�����˿��������¹Ҷ˿ڣ�2'd1�����˿�����ͨ�˿ڣ�2'd2�����˿�Ϊ���Ӷ˿ڣ�2'd3�������� */
        unsigned int    spe_port_enc_type          : 4; /* [5..2] �˿ڱ��ĸ�ʽ��4'd0���޷����4'd1��NCM_NTH16�����4'd2��NCM_NTH32�����4'd3��RNDIS�����4'd4��WIFI���ݣ�4'd5��IPF��4'd6��CPU����˿ڣ������������� */
        unsigned int    spe_port_pad_en            : 1; /* [6..6] padding���ʹ�ܡ�1'b0����ʹ�ܣ�1'b1��ʹ�ܡ� */
        unsigned int    reserved                   : 24; /* [30..7] ������ */
        unsigned int    spe_port_en                : 1; /* [31..31] �˿�ʹ�ܼĴ�����1'b0���˿ڲ�ʹ�ܣ�1'b1���˿�ʹ�ܡ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_PROPERTY_3_T;    /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_attach_brg        : 2; /* [1..0] �˿ڸ������ԼĴ�����2'd0�����˿��������¹Ҷ˿ڣ�2'd1�����˿�����ͨ�˿ڣ�2'd2�����˿�Ϊ���Ӷ˿ڣ�2'd3�������� */
        unsigned int    spe_port_enc_type          : 4; /* [5..2] �˿ڱ��ĸ�ʽ��4'd0���޷����4'd1��NCM_NTH16�����4'd2��NCM_NTH32�����4'd3��RNDIS�����4'd4��WIFI���ݣ�4'd5��IPF��4'd6��CPU����˿ڣ������������� */
        unsigned int    spe_port_pad_en            : 1; /* [6..6] padding���ʹ�ܡ�1'b0����ʹ�ܣ�1'b1��ʹ�ܡ� */
        unsigned int    reserved                   : 24; /* [30..7] ������ */
        unsigned int    spe_port_en                : 1; /* [31..31] �˿�ʹ�ܼĴ�����1'b0���˿ڲ�ʹ�ܣ�1'b1���˿�ʹ�ܡ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_PROPERTY_4_T;    /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_attach_brg        : 2; /* [1..0] �˿ڸ������ԼĴ�����2'd0�����˿��������¹Ҷ˿ڣ�2'd1�����˿�����ͨ�˿ڣ�2'd2�����˿�Ϊ���Ӷ˿ڣ�2'd3�������� */
        unsigned int    spe_port_enc_type          : 4; /* [5..2] �˿ڱ��ĸ�ʽ��4'd0���޷����4'd1��NCM_NTH16�����4'd2��NCM_NTH32�����4'd3��RNDIS�����4'd4��WIFI���ݣ�4'd5��IPF��4'd6��CPU����˿ڣ������������� */
        unsigned int    spe_port_pad_en            : 1; /* [6..6] padding���ʹ�ܡ�1'b0����ʹ�ܣ�1'b1��ʹ�ܡ� */
        unsigned int    reserved                   : 24; /* [30..7] ������ */
        unsigned int    spe_port_en                : 1; /* [31..31] �˿�ʹ�ܼĴ�����1'b0���˿ڲ�ʹ�ܣ�1'b1���˿�ʹ�ܡ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_PROPERTY_5_T;    /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_attach_brg        : 2; /* [1..0] �˿ڸ������ԼĴ�����2'd0�����˿��������¹Ҷ˿ڣ�2'd1�����˿�����ͨ�˿ڣ�2'd2�����˿�Ϊ���Ӷ˿ڣ�2'd3�������� */
        unsigned int    spe_port_enc_type          : 4; /* [5..2] �˿ڱ��ĸ�ʽ��4'd0���޷����4'd1��NCM_NTH16�����4'd2��NCM_NTH32�����4'd3��RNDIS�����4'd4��WIFI���ݣ�4'd5��IPF��4'd6��CPU����˿ڣ������������� */
        unsigned int    spe_port_pad_en            : 1; /* [6..6] padding���ʹ�ܡ�1'b0����ʹ�ܣ�1'b1��ʹ�ܡ� */
        unsigned int    reserved                   : 24; /* [30..7] ������ */
        unsigned int    spe_port_en                : 1; /* [31..31] �˿�ʹ�ܼĴ�����1'b0���˿ڲ�ʹ�ܣ�1'b1���˿�ʹ�ܡ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_PROPERTY_6_T;    /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_attach_brg        : 2; /* [1..0] �˿ڸ������ԼĴ�����2'd0�����˿��������¹Ҷ˿ڣ�2'd1�����˿�����ͨ�˿ڣ�2'd2�����˿�Ϊ���Ӷ˿ڣ�2'd3�������� */
        unsigned int    spe_port_enc_type          : 4; /* [5..2] �˿ڱ��ĸ�ʽ��4'd0���޷����4'd1��NCM_NTH16�����4'd2��NCM_NTH32�����4'd3��RNDIS�����4'd4��WIFI���ݣ�4'd5��IPF��4'd6��CPU����˿ڣ������������� */
        unsigned int    spe_port_pad_en            : 1; /* [6..6] padding���ʹ�ܡ�1'b0����ʹ�ܣ�1'b1��ʹ�ܡ� */
        unsigned int    reserved                   : 24; /* [30..7] ������ */
        unsigned int    spe_port_en                : 1; /* [31..31] �˿�ʹ�ܼĴ�����1'b0���˿ڲ�ʹ�ܣ�1'b1���˿�ʹ�ܡ� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_PROPERTY_7_T;    /* �˿����ԼĴ�����n����ͨ���ţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_l        : 32; /* [31..0] �˿�MAC��λ��ַ���üĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_L_0_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_l        : 32; /* [31..0] �˿�MAC��λ��ַ���üĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_L_1_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_l        : 32; /* [31..0] �˿�MAC��λ��ַ���üĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_L_2_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_l        : 32; /* [31..0] �˿�MAC��λ��ַ���üĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_L_3_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_l        : 32; /* [31..0] �˿�MAC��λ��ַ���üĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_L_4_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_l        : 32; /* [31..0] �˿�MAC��λ��ַ���üĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_L_5_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_l        : 32; /* [31..0] �˿�MAC��λ��ַ���üĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_L_6_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_l        : 32; /* [31..0] �˿�MAC��λ��ַ���üĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_L_7_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_h        : 16; /* [15..0] �˿�MAC��λ��ַ���üĴ����� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_H_0_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_h        : 16; /* [15..0] �˿�MAC��λ��ַ���üĴ����� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_H_1_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_h        : 16; /* [15..0] �˿�MAC��λ��ַ���üĴ����� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_H_2_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_h        : 16; /* [15..0] �˿�MAC��λ��ַ���üĴ����� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_H_3_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_h        : 16; /* [15..0] �˿�MAC��λ��ַ���üĴ����� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_H_4_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_h        : 16; /* [15..0] �˿�MAC��λ��ַ���üĴ����� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_H_5_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_h        : 16; /* [15..0] �˿�MAC��λ��ַ���üĴ����� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_H_6_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_mac_addr_h        : 16; /* [15..0] �˿�MAC��λ��ַ���üĴ����� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_MAC_ADDR_H_7_T;    /* �˿�MAC��λ��ַ���üĴ�����n����˿ںţ���Χ0~7 */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_1st     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_1ST_0_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_1st     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_1ST_1_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_1st     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_1ST_2_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_1st     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_1ST_3_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_1st     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_1ST_4_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_1st     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_1ST_5_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_1st     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_1ST_6_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_1st     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_1ST_7_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_2nd     : 32; /* [31..0] �˿�IPV6��ַ�ε�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_2ND_0_T;    /* �˿�IPV6��ַ�ε�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_2nd     : 32; /* [31..0] �˿�IPV6��ַ�ε�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_2ND_1_T;    /* �˿�IPV6��ַ�ε�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_2nd     : 32; /* [31..0] �˿�IPV6��ַ�ε�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_2ND_2_T;    /* �˿�IPV6��ַ�ε�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_2nd     : 32; /* [31..0] �˿�IPV6��ַ�ε�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_2ND_3_T;    /* �˿�IPV6��ַ�ε�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_2nd     : 32; /* [31..0] �˿�IPV6��ַ�ε�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_2ND_4_T;    /* �˿�IPV6��ַ�ε�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_2nd     : 32; /* [31..0] �˿�IPV6��ַ�ε�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_2ND_5_T;    /* �˿�IPV6��ַ�ε�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_2nd     : 32; /* [31..0] �˿�IPV6��ַ�ε�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_2ND_6_T;    /* �˿�IPV6��ַ�ε�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_2nd     : 32; /* [31..0] �˿�IPV6��ַ�ε�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_2ND_7_T;    /* �˿�IPV6��ַ�ε�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_3th     : 32; /* [31..0] �˿�IPV6��ַ�θ�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_3TH_0_T;    /* �˿�IPV6��ַ�θ�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_3th     : 32; /* [31..0] �˿�IPV6��ַ�θ�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_3TH_1_T;    /* �˿�IPV6��ַ�θ�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_3th     : 32; /* [31..0] �˿�IPV6��ַ�θ�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_3TH_2_T;    /* �˿�IPV6��ַ�θ�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_3th     : 32; /* [31..0] �˿�IPV6��ַ�θ�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_3TH_3_T;    /* �˿�IPV6��ַ�θ�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_3th     : 32; /* [31..0] �˿�IPV6��ַ�θ�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_3TH_4_T;    /* �˿�IPV6��ַ�θ�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_3th     : 32; /* [31..0] �˿�IPV6��ַ�θ�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_3TH_5_T;    /* �˿�IPV6��ַ�θ�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_3th     : 32; /* [31..0] �˿�IPV6��ַ�θ�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_3TH_6_T;    /* �˿�IPV6��ַ�θ�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_3th     : 32; /* [31..0] �˿�IPV6��ַ�θ�32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_3TH_7_T;    /* �˿�IPV6��ַ�θ�32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_4th     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_4TH_0_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_4th     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_4TH_1_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_4th     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_4TH_2_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_4th     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_4TH_3_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_4th     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_4TH_4_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_4th     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_4TH_5_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_4th     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_4TH_6_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_addr_4th     : 32; /* [31..0] �˿�IPV6��ַ���32λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_ADDR_4TH_7_T;    /* �˿�IPV6��ַ���32λ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_mask         : 8; /* [7..0] �˿�IPV6��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_MASK_0_T;    /* �˿�IPV6��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_mask         : 8; /* [7..0] �˿�IPV6��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_MASK_1_T;    /* �˿�IPV6��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_mask         : 8; /* [7..0] �˿�IPV6��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_MASK_2_T;    /* �˿�IPV6��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_mask         : 8; /* [7..0] �˿�IPV6��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_MASK_3_T;    /* �˿�IPV6��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_mask         : 8; /* [7..0] �˿�IPV6��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_MASK_4_T;    /* �˿�IPV6��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_mask         : 8; /* [7..0] �˿�IPV6��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_MASK_5_T;    /* �˿�IPV6��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_mask         : 8; /* [7..0] �˿�IPV6��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_MASK_6_T;    /* �˿�IPV6��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv6_mask         : 8; /* [7..0] �˿�IPV6��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 24; /* [31..8] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV6_MASK_7_T;    /* �˿�IPV6��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_addr         : 32; /* [31..0] �˿�IPV4��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_ADDR_0_T;    /* �˿�IPV4��ַ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_addr         : 32; /* [31..0] �˿�IPV4��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_ADDR_1_T;    /* �˿�IPV4��ַ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_addr         : 32; /* [31..0] �˿�IPV4��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_ADDR_2_T;    /* �˿�IPV4��ַ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_addr         : 32; /* [31..0] �˿�IPV4��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_ADDR_3_T;    /* �˿�IPV4��ַ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_addr         : 32; /* [31..0] �˿�IPV4��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_ADDR_4_T;    /* �˿�IPV4��ַ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_addr         : 32; /* [31..0] �˿�IPV4��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_ADDR_5_T;    /* �˿�IPV4��ַ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_addr         : 32; /* [31..0] �˿�IPV4��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_ADDR_6_T;    /* �˿�IPV4��ַ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_addr         : 32; /* [31..0] �˿�IPV4��ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_ADDR_7_T;    /* �˿�IPV4��ַ */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_mask         : 6; /* [5..0] �˿�IPV4��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 26; /* [31..6] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_MASK_0_T;    /* �˿�IPV4��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_mask         : 6; /* [5..0] �˿�IPV4��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 26; /* [31..6] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_MASK_1_T;    /* �˿�IPV4��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_mask         : 6; /* [5..0] �˿�IPV4��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 26; /* [31..6] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_MASK_2_T;    /* �˿�IPV4��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_mask         : 6; /* [5..0] �˿�IPV4��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 26; /* [31..6] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_MASK_3_T;    /* �˿�IPV4��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_mask         : 6; /* [5..0] �˿�IPV4��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 26; /* [31..6] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_MASK_4_T;    /* �˿�IPV4��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_mask         : 6; /* [5..0] �˿�IPV4��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 26; /* [31..6] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_MASK_5_T;    /* �˿�IPV4��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_mask         : 6; /* [5..0] �˿�IPV4��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 26; /* [31..6] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_MASK_6_T;    /* �˿�IPV4��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_port_ipv4_mask         : 6; /* [5..0] �˿�IPV4��ַMASK��1�ĸ����� */
        unsigned int    reserved                   : 26; /* [31..6] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PORTX_IPV4_MASK_7_T;    /* �˿�IPV4��ַMASK�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_ctrl               : 3; /* [2..0] ����������͡�3'b000�����MAC���˱�3'b001�����MACת����3'b010�����IPV4���˱�3'b011�����IPV6���˱�3'b100��ɾ��MAC���˱�3'b101��ɾ��MACת����3'b110��ɾ��IPV4���˱�3'b111��ɾ��IPV6���˱� */
        unsigned int    reserved                   : 29; /* [31..3] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CTRL_T;    /* SPE��������Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_done               : 1; /* [0..0] 1'b0��δ��ɱ��������1'b1����ɱ�������� */
        unsigned int    spe_tab_success            : 1; /* [1..1] 1'b0������δ�����ɹ���1'b1����������ɹ��� */
        unsigned int    reserved                   : 30; /* [31..2] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_ACT_RESULT_T;    /* SPE���������� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_0_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_1_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_2_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_3_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_4_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_5_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_6_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_7_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_8_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_9_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_10_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_11_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_12_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_13_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_tab_content            : 32; /* [31..0] ����˵���������ṹ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_TAB_CONTENT_14_T;    /* SPE�������ݼĴ�����һ��15�� */

typedef union
{
    struct
    {
        unsigned int    spe_hash_baddr             : 32; /* [31..0] HASH�������ַ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_HASH_BADDR_T;    /* HASH�������ַ */

typedef union
{
    struct
    {
        unsigned int    spe_hash_zone              : 32; /* [31..0] ���Ӹ����������ڹ�ϣ���㡣 */
    } bits;
    unsigned int    u32;
}HI_SPE_HASH_ZONE_T;    /* ���Ӹ����������ڹ�ϣ���� */

typedef union
{
    struct
    {
        unsigned int    spe_hash_rand              : 32; /* [31..0] ���Ӹ��ٹ�ϣ������� */
    } bits;
    unsigned int    u32;
}HI_SPE_HASH_RAND_T;    /* ���Ӹ��ٹ�ϣ����� */

typedef union
{
    struct
    {
        unsigned int    spe_ipv4_hash_l3_proto     : 16; /* [15..0] �����ַ�壬IPV4ʱ��Ϊ2�� */
        unsigned int    spe_ipv6_hash_l3_proto     : 16; /* [31..16] �����ַ�壬IPV6ʱ��Ϊ10�� */
    } bits;
    unsigned int    u32;
}HI_SPE_HASH_L3_PROTO_T;    /* �����ַ�� */

typedef union
{
    struct
    {
        unsigned int    spe_hash_width             : 5; /* [4..0] HASH��������ȼĴ�������wordΪ��λ������֧�ֵı�����Ϊ31��word */
        unsigned int    reserved                   : 27; /* [31..5] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_HASH_WIDTH_T;    /* HASH��������ȼĴ����� */

typedef union
{
    struct
    {
        unsigned int    spe_hash_depth             : 16; /* [15..0] HASHͰ��Ĵ���,��λ��HASH�ڵ������ */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_HASH_DEPTH_T;    /* HASHͰ�����üĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_ip_aging_time          : 16; /* [15..0] IPת�����ϻ�ʱ��Ĵ�������32.768kHzʱ�����ڵ�2^10����31.2ms��Ϊ��λ�� */
        unsigned int    spe_mac_aging_time         : 16; /* [31..16] MACת�����ϻ�ʱ��Ĵ�������32.768kHzʱ�����ڵ�2^10����31.2ms��Ϊ��λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_AGING_TIME_T;    /* �����ϻ�ʱ��Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_pack_max_time          : 16; /* [15..0] USB���ʱ�����ƼĴ�������32.768kHzʱ�����ڣ�30.5us��Ϊ��λ���ڵȴ���������У������ʱ��ﵽ������ֵʱ������δ�ﵽ���õ�������������������ɴ��������� */
        unsigned int    reserved                   : 16; /* [31..16] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PACK_MAX_TIME_T;    /* USB���ʱ�����ƼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_pack_max_pkt_cnt       : 6; /* [5..0] USB�����̫���������ƼĴ������԰�Ϊ��λ���ڵȴ���������У��������̫�������ﵽ������ֵʱ������δ�ﵽ���õ�������������������ɴ��������� */
        unsigned int    reserved_1                 : 2; /* [7..6] ������ */
        unsigned int    spe_pack_max_len           : 17; /* [24..8] USB����������ƼĴ�������byteΪ��λ���ڵȴ���������У���������ȴﵽ������ֵʱ������δ�ﵽ���õ�������������������ɴ�����������󳤶�Ϊ64K byte�� */
        unsigned int    reserved_0                 : 7; /* [31..25] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PACK_CTRL_T;    /* USB������ƼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_pack_remainder         : 16; /* [15..0] NCM������빫ʽ�е���������ֵ�� */
        unsigned int    spe_pack_divisor           : 16; /* [31..16] NCM������빫ʽ�еĳ�������ֵ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_PACK_ADDR_CTRL_T;    /* NCM�������Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_pack_remain_len        : 12; /* [11..0] USB�������Ԥ���ռ�Ĵ�������byteΪ��λ���ڵȴ���������У���������ȴﵽspe_pack_max_len-spe_pack_remain_len�����ɴ�����������󳤶�Ϊ4K-1 byte�� */
        unsigned int    reserved                   : 20; /* [31..12] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_PACK_REMAIN_LEN_T;    /* USB�������Ԥ���ռ�Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_unpack_ctrl            : 15; /* [14..0] USB����������ƼĴ�������byteΪ��λ�����Ϊ16K byte */
        unsigned int    reserved                   : 17; /* [31..15] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_UNPACK_CTRL_T;    /* NCM��װ��󳤶�Լ���Ĵ��� */

typedef union
{
    struct
    {
        unsigned int    spe_rndis_max_len          : 16; /* [15..0] RNDIS��װ����󳤶ȼĴ���������Լ��TD�е�pkt_len�ֶΡ� */
        unsigned int    spe_ncm_max_len            : 16; /* [31..16] NCM��װ����󳤶ȼĴ���������Լ��TD�е�pkt_len�ֶΡ� */
    } bits;
    unsigned int    u32;
}HI_SPE_UNPACK_MAX_LEN_T;    /* USB��װ��󳤶����üĴ���������Լ��TD�е�pkt_len�ֶΡ� */

typedef union
{
    struct
    {
        unsigned int    spe_event_buff0_wptr       : 10; /* [9..0] eventbuffer0дָ�롣��оƬά������Ӧ������ϵͳ�еĵ�ַΪ��SPE��ϵͳ�еĻ���ַ+eventbuffer0����ʼƫ�Ƶ�ַ+eventbuffer0дָ��*4 */
        unsigned int    reserved_1                 : 6; /* [15..10] ������ */
        unsigned int    spe_event_buff0_rptr       : 10; /* [25..16] eventbuffer0��ָ�롣��оƬά������Ӧ������ϵͳ�еĵ�ַΪ��SPE��ϵͳ�еĻ���ַ+eventbuffer0����ʼƫ�Ƶ�ַ+eventbuffer0��ָ��*4 */
        unsigned int    reserved_0                 : 6; /* [31..26] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENT_BUFF0_PTR_T;    /* eventbuffer0��дָ�� */

typedef union
{
    struct
    {
        unsigned int    spe_event_buff0_addr       : 32; /* [31..0] eventbuffer0��ʼ��ַ��ƫ�Ƶ�ַ��ƫ�����������ϵͳ�ı�����˵�ġ�����SPE��ϵͳ�еĻ���ַ��0x9002_1000���˼Ĵ�����ֵΪ0x1000����ôeventbuffer0����ʼ��ַΪ0x9002_20OO�� */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENT_BUFF0_ADDR_T;    /* eventbuffer0��ʼ��ַ��ƫ�Ƶ�ַ */

typedef union
{
    struct
    {
        unsigned int    spe_event_buff1_wptr       : 10; /* [9..0] eventbuffer1дָ�롣��оƬά������Ӧ������ϵͳ�еĵ�ַΪ��SPE��ϵͳ�еĻ���ַ+eventbuffer1����ʼƫ�Ƶ�ַ+eventbuffer1дָ��*4 */
        unsigned int    reserved_1                 : 6; /* [15..10] ������ */
        unsigned int    spe_event_buff1_rptr       : 10; /* [25..16] eventbuffer1��ָ�롣��оƬά������Ӧ������ϵͳ�еĵ�ַΪ��SPE��ϵͳ�еĻ���ַ+eventbuffer1����ʼƫ�Ƶ�ַ+eventbuffer1��ָ��*4 */
        unsigned int    reserved_0                 : 6; /* [31..26] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENT_BUFF1_PTR_T;    /* eventbuffer1��дָ�� */

typedef union
{
    struct
    {
        unsigned int    spe_event_buff1_addr       : 32; /* [31..0] eventbuffer1��ʼ��ַ��ƫ�Ƶ�ַ��ƫ�����������ϵͳ�ı�����˵�ġ�����SPE��ϵͳ�еĻ���ַ��0x9002_1000���˼Ĵ�����ֵΪ0x1100����ôeventbuffer1����ʼ��ַΪ0x9002_21OO�� */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENT_BUFF1_ADDR_T;    /* eventbuffer1��ʼ��ַ��ƫ�Ƶ�ַ */

typedef union
{
    struct
    {
        unsigned int    spe_event_buff0_len        : 11; /* [10..0] eventbuffer0������ã����ֵΪ1024����eventbuff0��eventbuffer1����ȺͲ�����1024�������Ⱥͳ�����1024����buffer0��ʵ��������ȣ�buffer1�����Ϊ1024��buffer0����ȣ� */
        unsigned int    reserved_1                 : 5; /* [15..11] ������ */
        unsigned int    spe_event_buff1_len        : 11; /* [26..16] eventbuffer1������ã����ֵΪ1024����eventbuff0��eventbuffer1����ȺͲ�����1024�������Ⱥͳ�����1024����buffer0��ʵ��������ȣ�buffer1�����Ϊ1024��buffer0����ȣ� */
        unsigned int    reserved_0                 : 5; /* [31..27] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENT_BUFF_LEN_T;    /* eventbuffer������üĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_int0_interval          : 32; /* [31..0] eventbuffer0�������ж���Сʱ��������32.768kHzʱ�����ڣ�30.5us��Ϊ��λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_INT0_INTERVAL_T;    /* eventbuffer0�жϼ��ʱ�� */

typedef union
{
    struct
    {
        unsigned int    spe_int1_interval          : 32; /* [31..0] eventbuffer1�������ж���Сʱ��������32.768kHzʱ�����ڣ�30.5us��Ϊ��λ�� */
    } bits;
    unsigned int    u32;
}HI_SPE_INT1_INTERVAL_T;    /* eventbuffer1�жϼ��ʱ�� */

typedef union
{
    struct
    {
        unsigned int    rdq_empty_int0_mask        : 1; /* [0..0] RDQ���ж�mask��1'b0��ʹ�ܣ�1'b1�����Ρ� */
        unsigned int    rdq_full_int0_mask         : 1; /* [1..1] RDQ���ж�mask��1'b0��ʹ�ܣ�1'b1�����Ρ� */
        unsigned int    rd_done_int0_mask          : 1; /* [2..2] RD����ж�mask��1'b0��ʹ�ܣ�1'b1�����Ρ� */
        unsigned int    tdq_empty_int0_mask        : 1; /* [3..3] TDQ���ж�mask��1'b0��ʹ�ܣ�1'b1�����Ρ� */
        unsigned int    tdq_full_int0_mask         : 1; /* [4..4] TDQ���ж�mask��1'b0��ʹ�ܣ�1'b1�����Ρ� */
        unsigned int    td_done_int0_mask          : 1; /* [5..5] TD����ж�mask��1'b0��ʹ�ܣ�1'b1�����Ρ� */
        unsigned int    reserved                   : 26; /* [31..6] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENT_BUFF0_MASK_T;    /* eventbuffer0�ж�MASK���ƼĴ��� */

typedef union
{
    struct
    {
        unsigned int    rdq_empty_int1_mask        : 1; /* [0..0] RDQ���ж�mask��1'b0��ʹ�ܣ�1'b1�����Ρ� */
        unsigned int    rdq_full_int1_mask         : 1; /* [1..1] RDQ���ж�mask��1'b0��ʹ�ܣ�1'b1�����Ρ� */
        unsigned int    rd_done_int1_mask          : 1; /* [2..2] RD����ж�mask��1'b0��ʹ�ܣ�1'b1�����Ρ� */
        unsigned int    tdq_empty_int1_mask        : 1; /* [3..3] TDQ���ж�mask��1'b0��ʹ�ܣ�1'b1�����Ρ� */
        unsigned int    tdq_full_int1_mask         : 1; /* [4..4] TDQ���ж�mask��1'b0��ʹ�ܣ�1'b1�����Ρ� */
        unsigned int    td_done_int1_mask          : 1; /* [5..5] TD����ж�mask��1'b0��ʹ�ܣ�1'b1�����Ρ� */
        unsigned int    reserved                   : 26; /* [31..6] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENT_BUFF1_MASK_T;    /* eventbuffer1�ж�MASK���ƼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_event_buf0_pro_cnt     : 11; /* [10..0] �������eventbuffer0����Чevent�¼������� */
        unsigned int    reserved                   : 21; /* [31..11] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENTBUF0_PRO_CNT_T;    /* �������EVENTBUFFER0��Чevent�¼����� */

typedef union
{
    struct
    {
        unsigned int    spe_event_buf0_rpt_cnt     : 11; /* [10..0] оƬ�ϱ�eventbuffer0����Чevent�¼������� */
        unsigned int    reserved                   : 21; /* [31..11] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENTBUF0_RPT_CNT_T;    /* оƬ�ϱ�EVENTBUFFER0��Чevent�¼����� */

typedef union
{
    struct
    {
        unsigned int    spe_event_buf1_pro_cnt     : 11; /* [10..0] �������eventbuffer1����Чevent�¼������� */
        unsigned int    reserved                   : 21; /* [31..11] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENTBUF1_PRO_CNT_T;    /* �������EVENTBUFFER1��Чevent�¼����� */

typedef union
{
    struct
    {
        unsigned int    spe_event_buf1_rpt_cnt     : 11; /* [10..0] оƬ�ϱ�eventbuffer1����Чevent�¼������� */
        unsigned int    reserved                   : 21; /* [31..11] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENTBUF1_RPT_CNT_T;    /* оƬ�ϱ�EVENTBUFFER1��Чevent�¼����� */

typedef union
{
    struct
    {
        unsigned int    spe_event_buff0_int_state  : 1; /* [0..0] �ж�����eventbuffer0��1'b0:��1'b1:�ǡ� */
        unsigned int    spe_event_buff1_int_state  : 1; /* [1..1] �ж�����eventbuffer1��1'b0:��1'b1:�ǡ� */
        unsigned int    reserved                   : 30; /* [31..2] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_INT_STATE_T;    /* �ж�״̬�������ж������ĸ�eventbuffer */

typedef union
{
    struct
    {
        unsigned int    spe_event_buff0_full       : 1; /* [0..0] eventbuffer0��������1'b0:������1'b1:���� */
        unsigned int    spe_event_buff1_full       : 1; /* [1..1] eventbuffer1��������1'b0:������1'b1:���� */
        unsigned int    reserved                   : 30; /* [31..2] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENT_BUFF_FULL_T;    /* eventbuffer��״̬�ϱ� */

typedef union
{
    struct
    {
        unsigned int    spe_event_buf0_rd_err      : 1; /* [0..0] �����eventbuffer0�����1'b0:û�����1'b1:����� */
        unsigned int    reserved                   : 31; /* [31..1] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENT_BUFF0_RD_ERR_T;    /* �����eventbuffer0��� */

typedef union
{
    struct
    {
        unsigned int    spe_event_buf1_rd_err      : 1; /* [0..0] �����eventbuffer1�����1'b0:û�����1'b1:����� */
        unsigned int    reserved                   : 31; /* [31..1] ������ */
    } bits;
    unsigned int    u32;
}HI_SPE_EVENT_BUFF1_RD_ERR_T;    /* �����eventbuffer1��� */

typedef union
{
    struct
    {
        unsigned int    spe_wrr_debug              : 32; /* [31..0] spe_wrrģ����ԼĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_WRR_DEBUG_T;    /* spe_wrrģ����ԼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_unpack_debug           : 32; /* [31..0] spe_unpackģ����ԼĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_UNPACK_DEBUG_T;    /* spe_unpackģ����ԼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_eng_debug              : 32; /* [31..0] spe_engģ����ԼĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_ENG_DEBUG_T;    /* spe_engģ����ԼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_pack_debug             : 32; /* [31..0] spe_packģ����ԼĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_PACK_DEBUG_T;    /* spe_packģ����ԼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_done_debug             : 32; /* [31..0] spe_doneģ����ԼĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_DONE_DEBUG_T;    /* spe_doneģ����ԼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_if_debug               : 32; /* [31..0] spe_ifģ����ԼĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_IF_DEBUG_T;    /* spe_ifģ����ԼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_lp_rd_block_cnt        : 32; /* [31..0] �����ж˿�RD����ͳ�ƼĴ�����Bit[7:0]��Ӧ0�˿�ͳ�ƣ�Bit[15:8]��Ӧ1�˿�ͳ�ƣ�Bit[23:16]��Ӧ2�˿�ͳ�ƣ�Bit[31:24]��Ӧ3�˿�ͳ�ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_LP_RD_BLOCK_CNT_T;    /* �˿�0~3�������кŶ˿ڣ�RD����ͳ�ƼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_hp_rd_block_cnt        : 32; /* [31..0] �����ж˿�RD����ͳ�ƼĴ�����Bit[7:0]��Ӧ4�˿�ͳ�ƣ�Bit[15:8]��Ӧ5�˿�ͳ�ƣ�Bit[23:16]��Ӧ6�˿�ͳ�ƣ�Bit[31:24]��Ӧ7�˿�ͳ�ơ� */
    } bits;
    unsigned int    u32;
}HI_SPE_HP_RD_BLOCK_CNT_T;    /* �˿�4~7�������кŶ˿ڣ�RD����ͳ�ƼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_pack_overtime_cnt      : 32; /* [31..0] USB�����ʱͳ�ƼĴ��� */
    } bits;
    unsigned int    u32;
}HI_SPE_PACK_OVERTIME_CNT_T;    /* USB�����ʱͳ�ƼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_pack_push_cnt          : 32; /* [31..0] USB���PUSHͳ�ƼĴ��� */
    } bits;
    unsigned int    u32;
}HI_SPE_PACK_PUSH_CNT_T;    /* USB���PUSHͳ�ƼĴ��� */

typedef union
{
    struct
    {
        unsigned int    spe_depack_err_cnt         : 32; /* [31..0] USB��װ��ʽ��������ͳ�ƼĴ����� */
    } bits;
    unsigned int    u32;
}HI_SPE_DEPACK_ERR_CNT_T;    /* USB��װ��ʽ��������ͳ�ƼĴ��� */


/********************************************************************************/
/*    SPE_MPI ��������Ŀ��_ģ����_�Ĵ�����_��Ա��_set)        */
/********************************************************************************/
HI_SET_GET(hi_spe_en_spe_en,spe_en,HI_SPE_EN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EN_OFFSET)
HI_SET_GET(hi_spe_en_reserved,reserved,HI_SPE_EN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EN_OFFSET)
HI_SET_GET(hi_spe_usrfield_ctrl_spe_usrfield_ctrl,spe_usrfield_ctrl,HI_SPE_USRFIELD_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_USRFIELD_CTRL_OFFSET)
HI_SET_GET(hi_spe_usrfield_ctrl_reserved,reserved,HI_SPE_USRFIELD_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_USRFIELD_CTRL_OFFSET)
HI_SET_GET(hi_spe_black_white_spe_mac_black_white,spe_mac_black_white,HI_SPE_BLACK_WHITE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_BLACK_WHITE_OFFSET)
HI_SET_GET(hi_spe_black_white_spe_ip_black_white,spe_ip_black_white,HI_SPE_BLACK_WHITE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_BLACK_WHITE_OFFSET)
HI_SET_GET(hi_spe_black_white_reserved,reserved,HI_SPE_BLACK_WHITE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_BLACK_WHITE_OFFSET)
HI_SET_GET(hi_spe_port_lmttime_spe_port_udp_lmttime,spe_port_udp_lmttime,HI_SPE_PORT_LMTTIME_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORT_LMTTIME_OFFSET)
HI_SET_GET(hi_spe_port_lmttime_spe_port_lmttime,spe_port_lmttime,HI_SPE_PORT_LMTTIME_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORT_LMTTIME_OFFSET)
HI_SET_GET(hi_spe_port_lmttime_reserved,reserved,HI_SPE_PORT_LMTTIME_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORT_LMTTIME_OFFSET)
HI_SET_GET(hi_spe_eth_minlen_spe_eth_minlen,spe_eth_minlen,HI_SPE_ETH_MINLEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_ETH_MINLEN_OFFSET)
HI_SET_GET(hi_spe_eth_minlen_reserved,reserved,HI_SPE_ETH_MINLEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_ETH_MINLEN_OFFSET)
HI_SET_GET(hi_spe_mode_spe_stick_sportnum,spe_stick_sportnum,HI_SPE_MODE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_MODE_OFFSET)
HI_SET_GET(hi_spe_mode_reserved_2,reserved_2,HI_SPE_MODE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_MODE_OFFSET)
HI_SET_GET(hi_spe_mode_spe_stick_dportnum,spe_stick_dportnum,HI_SPE_MODE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_MODE_OFFSET)
HI_SET_GET(hi_spe_mode_reserved_1,reserved_1,HI_SPE_MODE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_MODE_OFFSET)
HI_SET_GET(hi_spe_mode_spe_mode,spe_mode,HI_SPE_MODE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_MODE_OFFSET)
HI_SET_GET(hi_spe_mode_reserved_0,reserved_0,HI_SPE_MODE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_MODE_OFFSET)
HI_SET_GET(hi_spe_dbgen_spe_dbgen,spe_dbgen,HI_SPE_DBGEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_DBGEN_OFFSET)
HI_SET_GET(hi_spe_dbgen_reserved,reserved,HI_SPE_DBGEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_DBGEN_OFFSET)
HI_SET_GET(hi_spe_glb_dmac_addr_l_spe_glb_dmac_addr_l,spe_glb_dmac_addr_l,HI_SPE_GLB_DMAC_ADDR_L_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_GLB_DMAC_ADDR_L_OFFSET)
HI_SET_GET(hi_spe_glb_dmac_addr_h_spe_glb_dmac_addr_h,spe_glb_dmac_addr_h,HI_SPE_GLB_DMAC_ADDR_H_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_GLB_DMAC_ADDR_H_OFFSET)
HI_SET_GET(hi_spe_glb_dmac_addr_h_reserved,reserved,HI_SPE_GLB_DMAC_ADDR_H_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_GLB_DMAC_ADDR_H_OFFSET)
HI_SET_GET(hi_spe_glb_smac_addr_l_spe_glb_smac_addr_l,spe_glb_smac_addr_l,HI_SPE_GLB_SMAC_ADDR_L_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_GLB_SMAC_ADDR_L_OFFSET)
HI_SET_GET(hi_spe_glb_smac_addr_h_spe_glb_smac_addr_h,spe_glb_smac_addr_h,HI_SPE_GLB_SMAC_ADDR_H_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_GLB_SMAC_ADDR_H_OFFSET)
HI_SET_GET(hi_spe_glb_smac_addr_h_reserved,reserved,HI_SPE_GLB_SMAC_ADDR_H_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_GLB_SMAC_ADDR_H_OFFSET)
HI_SET_GET(hi_spe_rd_burstlen_spe_rd_burstlen,spe_rd_burstlen,HI_SPE_RD_BURSTLEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RD_BURSTLEN_OFFSET)
HI_SET_GET(hi_spe_rd_burstlen_reserved,reserved,HI_SPE_RD_BURSTLEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RD_BURSTLEN_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_0_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_0_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_0_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_0_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_1_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_1_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_1_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_1_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_2_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_2_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_2_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_2_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_3_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_3_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_3_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_3_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_4_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_4_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_4_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_4_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_5_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_5_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_5_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_5_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_6_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_6_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_6_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_6_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_7_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_7_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_7_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_7_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_8_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_8_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_8_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_8_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_8_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_8_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_9_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_9_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_9_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_9_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_9_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_9_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_10_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_10_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_10_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_10_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_10_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_10_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_11_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_11_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_11_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_11_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_11_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_11_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_12_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_12_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_12_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_12_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_12_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_12_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_13_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_13_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_13_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_13_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_13_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_13_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_14_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_14_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_14_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_14_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_14_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_14_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_15_spe_l4_portnum_cfg,spe_l4_portnum_cfg,HI_SPE_L4_PORTNUM_CFG_15_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_15_OFFSET)
HI_SET_GET(hi_spe_l4_portnum_cfg_15_reserved,reserved,HI_SPE_L4_PORTNUM_CFG_15_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_L4_PORTNUM_CFG_15_OFFSET)
HI_SET_GET(hi_spe_cnt_clr_spe_cnt_clr,spe_cnt_clr,HI_SPE_CNT_CLR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CNT_CLR_OFFSET)
HI_SET_GET(hi_spe_cnt_clr_reserved,reserved,HI_SPE_CNT_CLR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CNT_CLR_OFFSET)
HI_SET_GET(hi_spe_ver_spe_ver,spe_ver,HI_SPE_VER_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_VER_OFFSET)
HI_SET_GET(hi_spe_time_spe_time,spe_time,HI_SPE_TIME_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TIME_OFFSET)
HI_SET_GET(hi_spe_time_reserved,reserved,HI_SPE_TIME_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TIME_OFFSET)
HI_SET_GET(hi_spe_idle_spe_idle,spe_idle,HI_SPE_IDLE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_IDLE_OFFSET)
HI_SET_GET(hi_spe_idle_reserved,reserved,HI_SPE_IDLE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_IDLE_OFFSET)
HI_SET_GET(hi_spe_ost_ctrl_spe_gm_rd_osrc_lmt,spe_gm_rd_osrc_lmt,HI_SPE_OST_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_OST_CTRL_OFFSET)
HI_SET_GET(hi_spe_ost_ctrl_spe_gm_wr_osrc_lmt,spe_gm_wr_osrc_lmt,HI_SPE_OST_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_OST_CTRL_OFFSET)
HI_SET_GET(hi_spe_ost_ctrl_spe_rd_ost_en,spe_rd_ost_en,HI_SPE_OST_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_OST_CTRL_OFFSET)
HI_SET_GET(hi_spe_ost_ctrl_spe_wr_ost_en,spe_wr_ost_en,HI_SPE_OST_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_OST_CTRL_OFFSET)
HI_SET_GET(hi_spe_ost_ctrl_reserved,reserved,HI_SPE_OST_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_OST_CTRL_OFFSET)
HI_SET_GET(hi_spe_burst_ctrl_spe_gm_enburst_undef,spe_gm_enburst_undef,HI_SPE_BURST_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_BURST_CTRL_OFFSET)
HI_SET_GET(hi_spe_burst_ctrl_spe_gm_enburst4,spe_gm_enburst4,HI_SPE_BURST_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_BURST_CTRL_OFFSET)
HI_SET_GET(hi_spe_burst_ctrl_spe_gm_enburst8,spe_gm_enburst8,HI_SPE_BURST_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_BURST_CTRL_OFFSET)
HI_SET_GET(hi_spe_burst_ctrl_spe_gm_enburst16,spe_gm_enburst16,HI_SPE_BURST_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_BURST_CTRL_OFFSET)
HI_SET_GET(hi_spe_burst_ctrl_reserved,reserved,HI_SPE_BURST_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_BURST_CTRL_OFFSET)
HI_SET_GET(hi_spe_wch_id_spe_wch0_id,spe_wch0_id,HI_SPE_WCH_ID_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_WCH_ID_OFFSET)
HI_SET_GET(hi_spe_wch_id_spe_wch1_id,spe_wch1_id,HI_SPE_WCH_ID_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_WCH_ID_OFFSET)
HI_SET_GET(hi_spe_wch_id_spe_wch2_id,spe_wch2_id,HI_SPE_WCH_ID_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_WCH_ID_OFFSET)
HI_SET_GET(hi_spe_wch_id_spe_wch3_id,spe_wch3_id,HI_SPE_WCH_ID_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_WCH_ID_OFFSET)
HI_SET_GET(hi_spe_wch_id_reserved,reserved,HI_SPE_WCH_ID_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_WCH_ID_OFFSET)
HI_SET_GET(hi_spe_rch_id_spe_rch0_id,spe_rch0_id,HI_SPE_RCH_ID_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RCH_ID_OFFSET)
HI_SET_GET(hi_spe_rch_id_spe_rch1_id,spe_rch1_id,HI_SPE_RCH_ID_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RCH_ID_OFFSET)
HI_SET_GET(hi_spe_rch_id_spe_rch2_id,spe_rch2_id,HI_SPE_RCH_ID_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RCH_ID_OFFSET)
HI_SET_GET(hi_spe_rch_id_spe_rch3_id,spe_rch3_id,HI_SPE_RCH_ID_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RCH_ID_OFFSET)
HI_SET_GET(hi_spe_rch_id_spe_rch4_id,spe_rch4_id,HI_SPE_RCH_ID_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RCH_ID_OFFSET)
HI_SET_GET(hi_spe_rch_id_reserved,reserved,HI_SPE_RCH_ID_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RCH_ID_OFFSET)
HI_SET_GET(hi_spe_wch_pri_spe_wch0_pri,spe_wch0_pri,HI_SPE_WCH_PRI_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_WCH_PRI_OFFSET)
HI_SET_GET(hi_spe_wch_pri_spe_wch1_pri,spe_wch1_pri,HI_SPE_WCH_PRI_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_WCH_PRI_OFFSET)
HI_SET_GET(hi_spe_wch_pri_spe_wch2_pri,spe_wch2_pri,HI_SPE_WCH_PRI_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_WCH_PRI_OFFSET)
HI_SET_GET(hi_spe_wch_pri_spe_wch3_pri,spe_wch3_pri,HI_SPE_WCH_PRI_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_WCH_PRI_OFFSET)
HI_SET_GET(hi_spe_wch_pri_reserved,reserved,HI_SPE_WCH_PRI_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_WCH_PRI_OFFSET)
HI_SET_GET(hi_spe_rch_pri_spe_rch0_pri,spe_rch0_pri,HI_SPE_RCH_PRI_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RCH_PRI_OFFSET)
HI_SET_GET(hi_spe_rch_pri_spe_rch1_pri,spe_rch1_pri,HI_SPE_RCH_PRI_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RCH_PRI_OFFSET)
HI_SET_GET(hi_spe_rch_pri_spe_rch2_pri,spe_rch2_pri,HI_SPE_RCH_PRI_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RCH_PRI_OFFSET)
HI_SET_GET(hi_spe_rch_pri_spe_rch3_pri,spe_rch3_pri,HI_SPE_RCH_PRI_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RCH_PRI_OFFSET)
HI_SET_GET(hi_spe_rch_pri_spe_rch4_pri,spe_rch4_pri,HI_SPE_RCH_PRI_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RCH_PRI_OFFSET)
HI_SET_GET(hi_spe_rch_pri_reserved,reserved,HI_SPE_RCH_PRI_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RCH_PRI_OFFSET)
HI_SET_GET(hi_spe_tdqx_baddr_0_spe_tdq_baddr,spe_tdq_baddr,HI_SPE_TDQX_BADDR_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_BADDR_0_OFFSET)
HI_SET_GET(hi_spe_tdqx_baddr_1_spe_tdq_baddr,spe_tdq_baddr,HI_SPE_TDQX_BADDR_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_BADDR_1_OFFSET)
HI_SET_GET(hi_spe_tdqx_baddr_2_spe_tdq_baddr,spe_tdq_baddr,HI_SPE_TDQX_BADDR_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_BADDR_2_OFFSET)
HI_SET_GET(hi_spe_tdqx_baddr_3_spe_tdq_baddr,spe_tdq_baddr,HI_SPE_TDQX_BADDR_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_BADDR_3_OFFSET)
HI_SET_GET(hi_spe_tdqx_baddr_4_spe_tdq_baddr,spe_tdq_baddr,HI_SPE_TDQX_BADDR_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_BADDR_4_OFFSET)
HI_SET_GET(hi_spe_tdqx_baddr_5_spe_tdq_baddr,spe_tdq_baddr,HI_SPE_TDQX_BADDR_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_BADDR_5_OFFSET)
HI_SET_GET(hi_spe_tdqx_baddr_6_spe_tdq_baddr,spe_tdq_baddr,HI_SPE_TDQX_BADDR_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_BADDR_6_OFFSET)
HI_SET_GET(hi_spe_tdqx_baddr_7_spe_tdq_baddr,spe_tdq_baddr,HI_SPE_TDQX_BADDR_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_BADDR_7_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_0_spe_tdq_len,spe_tdq_len,HI_SPE_TDQX_LEN_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_0_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_0_reserved,reserved,HI_SPE_TDQX_LEN_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_0_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_1_spe_tdq_len,spe_tdq_len,HI_SPE_TDQX_LEN_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_1_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_1_reserved,reserved,HI_SPE_TDQX_LEN_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_1_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_2_spe_tdq_len,spe_tdq_len,HI_SPE_TDQX_LEN_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_2_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_2_reserved,reserved,HI_SPE_TDQX_LEN_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_2_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_3_spe_tdq_len,spe_tdq_len,HI_SPE_TDQX_LEN_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_3_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_3_reserved,reserved,HI_SPE_TDQX_LEN_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_3_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_4_spe_tdq_len,spe_tdq_len,HI_SPE_TDQX_LEN_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_4_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_4_reserved,reserved,HI_SPE_TDQX_LEN_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_4_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_5_spe_tdq_len,spe_tdq_len,HI_SPE_TDQX_LEN_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_5_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_5_reserved,reserved,HI_SPE_TDQX_LEN_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_5_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_6_spe_tdq_len,spe_tdq_len,HI_SPE_TDQX_LEN_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_6_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_6_reserved,reserved,HI_SPE_TDQX_LEN_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_6_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_7_spe_tdq_len,spe_tdq_len,HI_SPE_TDQX_LEN_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_7_OFFSET)
HI_SET_GET(hi_spe_tdqx_len_7_reserved,reserved,HI_SPE_TDQX_LEN_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_LEN_7_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_0_spe_tdq_wptr,spe_tdq_wptr,HI_SPE_TDQX_PTR_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_0_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_0_spe_tdq_rptr,spe_tdq_rptr,HI_SPE_TDQX_PTR_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_0_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_1_spe_tdq_wptr,spe_tdq_wptr,HI_SPE_TDQX_PTR_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_1_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_1_spe_tdq_rptr,spe_tdq_rptr,HI_SPE_TDQX_PTR_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_1_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_2_spe_tdq_wptr,spe_tdq_wptr,HI_SPE_TDQX_PTR_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_2_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_2_spe_tdq_rptr,spe_tdq_rptr,HI_SPE_TDQX_PTR_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_2_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_3_spe_tdq_wptr,spe_tdq_wptr,HI_SPE_TDQX_PTR_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_3_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_3_spe_tdq_rptr,spe_tdq_rptr,HI_SPE_TDQX_PTR_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_3_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_4_spe_tdq_wptr,spe_tdq_wptr,HI_SPE_TDQX_PTR_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_4_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_4_spe_tdq_rptr,spe_tdq_rptr,HI_SPE_TDQX_PTR_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_4_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_5_spe_tdq_wptr,spe_tdq_wptr,HI_SPE_TDQX_PTR_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_5_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_5_spe_tdq_rptr,spe_tdq_rptr,HI_SPE_TDQX_PTR_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_5_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_6_spe_tdq_wptr,spe_tdq_wptr,HI_SPE_TDQX_PTR_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_6_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_6_spe_tdq_rptr,spe_tdq_rptr,HI_SPE_TDQX_PTR_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_6_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_7_spe_tdq_wptr,spe_tdq_wptr,HI_SPE_TDQX_PTR_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_7_OFFSET)
HI_SET_GET(hi_spe_tdqx_ptr_7_spe_tdq_rptr,spe_tdq_rptr,HI_SPE_TDQX_PTR_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PTR_7_OFFSET)
HI_SET_GET(hi_spe_rdqx_baddr_0_spe_rdq_baddr,spe_rdq_baddr,HI_SPE_RDQX_BADDR_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_BADDR_0_OFFSET)
HI_SET_GET(hi_spe_rdqx_baddr_1_spe_rdq_baddr,spe_rdq_baddr,HI_SPE_RDQX_BADDR_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_BADDR_1_OFFSET)
HI_SET_GET(hi_spe_rdqx_baddr_2_spe_rdq_baddr,spe_rdq_baddr,HI_SPE_RDQX_BADDR_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_BADDR_2_OFFSET)
HI_SET_GET(hi_spe_rdqx_baddr_3_spe_rdq_baddr,spe_rdq_baddr,HI_SPE_RDQX_BADDR_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_BADDR_3_OFFSET)
HI_SET_GET(hi_spe_rdqx_baddr_4_spe_rdq_baddr,spe_rdq_baddr,HI_SPE_RDQX_BADDR_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_BADDR_4_OFFSET)
HI_SET_GET(hi_spe_rdqx_baddr_5_spe_rdq_baddr,spe_rdq_baddr,HI_SPE_RDQX_BADDR_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_BADDR_5_OFFSET)
HI_SET_GET(hi_spe_rdqx_baddr_6_spe_rdq_baddr,spe_rdq_baddr,HI_SPE_RDQX_BADDR_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_BADDR_6_OFFSET)
HI_SET_GET(hi_spe_rdqx_baddr_7_spe_rdq_baddr,spe_rdq_baddr,HI_SPE_RDQX_BADDR_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_BADDR_7_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_0_spe_rdq_len,spe_rdq_len,HI_SPE_RDQX_LEN_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_0_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_0_reserved,reserved,HI_SPE_RDQX_LEN_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_0_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_1_spe_rdq_len,spe_rdq_len,HI_SPE_RDQX_LEN_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_1_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_1_reserved,reserved,HI_SPE_RDQX_LEN_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_1_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_2_spe_rdq_len,spe_rdq_len,HI_SPE_RDQX_LEN_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_2_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_2_reserved,reserved,HI_SPE_RDQX_LEN_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_2_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_3_spe_rdq_len,spe_rdq_len,HI_SPE_RDQX_LEN_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_3_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_3_reserved,reserved,HI_SPE_RDQX_LEN_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_3_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_4_spe_rdq_len,spe_rdq_len,HI_SPE_RDQX_LEN_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_4_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_4_reserved,reserved,HI_SPE_RDQX_LEN_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_4_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_5_spe_rdq_len,spe_rdq_len,HI_SPE_RDQX_LEN_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_5_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_5_reserved,reserved,HI_SPE_RDQX_LEN_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_5_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_6_spe_rdq_len,spe_rdq_len,HI_SPE_RDQX_LEN_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_6_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_6_reserved,reserved,HI_SPE_RDQX_LEN_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_6_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_7_spe_rdq_len,spe_rdq_len,HI_SPE_RDQX_LEN_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_7_OFFSET)
HI_SET_GET(hi_spe_rdqx_len_7_reserved,reserved,HI_SPE_RDQX_LEN_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_LEN_7_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_0_spe_rdq_wptr,spe_rdq_wptr,HI_SPE_RDQX_PTR_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_0_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_0_spe_rdq_rptr,spe_rdq_rptr,HI_SPE_RDQX_PTR_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_0_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_1_spe_rdq_wptr,spe_rdq_wptr,HI_SPE_RDQX_PTR_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_1_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_1_spe_rdq_rptr,spe_rdq_rptr,HI_SPE_RDQX_PTR_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_1_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_2_spe_rdq_wptr,spe_rdq_wptr,HI_SPE_RDQX_PTR_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_2_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_2_spe_rdq_rptr,spe_rdq_rptr,HI_SPE_RDQX_PTR_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_2_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_3_spe_rdq_wptr,spe_rdq_wptr,HI_SPE_RDQX_PTR_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_3_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_3_spe_rdq_rptr,spe_rdq_rptr,HI_SPE_RDQX_PTR_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_3_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_4_spe_rdq_wptr,spe_rdq_wptr,HI_SPE_RDQX_PTR_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_4_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_4_spe_rdq_rptr,spe_rdq_rptr,HI_SPE_RDQX_PTR_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_4_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_5_spe_rdq_wptr,spe_rdq_wptr,HI_SPE_RDQX_PTR_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_5_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_5_spe_rdq_rptr,spe_rdq_rptr,HI_SPE_RDQX_PTR_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_5_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_6_spe_rdq_wptr,spe_rdq_wptr,HI_SPE_RDQX_PTR_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_6_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_6_spe_rdq_rptr,spe_rdq_rptr,HI_SPE_RDQX_PTR_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_6_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_7_spe_rdq_wptr,spe_rdq_wptr,HI_SPE_RDQX_PTR_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_7_OFFSET)
HI_SET_GET(hi_spe_rdqx_ptr_7_spe_rdq_rptr,spe_rdq_rptr,HI_SPE_RDQX_PTR_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_RDQX_PTR_7_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_0_spe_tdq_pri,spe_tdq_pri,HI_SPE_TDQX_PRI_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_0_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_0_reserved,reserved,HI_SPE_TDQX_PRI_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_0_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_1_spe_tdq_pri,spe_tdq_pri,HI_SPE_TDQX_PRI_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_1_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_1_reserved,reserved,HI_SPE_TDQX_PRI_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_1_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_2_spe_tdq_pri,spe_tdq_pri,HI_SPE_TDQX_PRI_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_2_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_2_reserved,reserved,HI_SPE_TDQX_PRI_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_2_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_3_spe_tdq_pri,spe_tdq_pri,HI_SPE_TDQX_PRI_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_3_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_3_reserved,reserved,HI_SPE_TDQX_PRI_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_3_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_4_spe_tdq_pri,spe_tdq_pri,HI_SPE_TDQX_PRI_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_4_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_4_reserved,reserved,HI_SPE_TDQX_PRI_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_4_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_5_spe_tdq_pri,spe_tdq_pri,HI_SPE_TDQX_PRI_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_5_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_5_reserved,reserved,HI_SPE_TDQX_PRI_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_5_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_6_spe_tdq_pri,spe_tdq_pri,HI_SPE_TDQX_PRI_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_6_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_6_reserved,reserved,HI_SPE_TDQX_PRI_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_6_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_7_spe_tdq_pri,spe_tdq_pri,HI_SPE_TDQX_PRI_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_7_OFFSET)
HI_SET_GET(hi_spe_tdqx_pri_7_reserved,reserved,HI_SPE_TDQX_PRI_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TDQX_PRI_7_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_0_spe_ch_portnum,spe_ch_portnum,HI_SPE_CHX_PORTNUM_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_0_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_0_reserved,reserved,HI_SPE_CHX_PORTNUM_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_0_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_1_spe_ch_portnum,spe_ch_portnum,HI_SPE_CHX_PORTNUM_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_1_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_1_reserved,reserved,HI_SPE_CHX_PORTNUM_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_1_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_2_spe_ch_portnum,spe_ch_portnum,HI_SPE_CHX_PORTNUM_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_2_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_2_reserved,reserved,HI_SPE_CHX_PORTNUM_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_2_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_3_spe_ch_portnum,spe_ch_portnum,HI_SPE_CHX_PORTNUM_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_3_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_3_reserved,reserved,HI_SPE_CHX_PORTNUM_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_3_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_4_spe_ch_portnum,spe_ch_portnum,HI_SPE_CHX_PORTNUM_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_4_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_4_reserved,reserved,HI_SPE_CHX_PORTNUM_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_4_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_5_spe_ch_portnum,spe_ch_portnum,HI_SPE_CHX_PORTNUM_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_5_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_5_reserved,reserved,HI_SPE_CHX_PORTNUM_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_5_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_6_spe_ch_portnum,spe_ch_portnum,HI_SPE_CHX_PORTNUM_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_6_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_6_reserved,reserved,HI_SPE_CHX_PORTNUM_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_6_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_7_spe_ch_portnum,spe_ch_portnum,HI_SPE_CHX_PORTNUM_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_7_OFFSET)
HI_SET_GET(hi_spe_chx_portnum_7_reserved,reserved,HI_SPE_CHX_PORTNUM_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_CHX_PORTNUM_7_OFFSET)
HI_SET_GET(hi_spe_portx_udp_lmtnum_0_spe_port_udp_lmtnum,spe_port_udp_lmtnum,HI_SPE_PORTX_UDP_LMTNUM_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_UDP_LMTNUM_0_OFFSET)
HI_SET_GET(hi_spe_portx_udp_lmtnum_1_spe_port_udp_lmtnum,spe_port_udp_lmtnum,HI_SPE_PORTX_UDP_LMTNUM_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_UDP_LMTNUM_1_OFFSET)
HI_SET_GET(hi_spe_portx_udp_lmtnum_2_spe_port_udp_lmtnum,spe_port_udp_lmtnum,HI_SPE_PORTX_UDP_LMTNUM_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_UDP_LMTNUM_2_OFFSET)
HI_SET_GET(hi_spe_portx_udp_lmtnum_3_spe_port_udp_lmtnum,spe_port_udp_lmtnum,HI_SPE_PORTX_UDP_LMTNUM_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_UDP_LMTNUM_3_OFFSET)
HI_SET_GET(hi_spe_portx_udp_lmtnum_4_spe_port_udp_lmtnum,spe_port_udp_lmtnum,HI_SPE_PORTX_UDP_LMTNUM_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_UDP_LMTNUM_4_OFFSET)
HI_SET_GET(hi_spe_portx_udp_lmtnum_5_spe_port_udp_lmtnum,spe_port_udp_lmtnum,HI_SPE_PORTX_UDP_LMTNUM_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_UDP_LMTNUM_5_OFFSET)
HI_SET_GET(hi_spe_portx_udp_lmtnum_6_spe_port_udp_lmtnum,spe_port_udp_lmtnum,HI_SPE_PORTX_UDP_LMTNUM_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_UDP_LMTNUM_6_OFFSET)
HI_SET_GET(hi_spe_portx_udp_lmtnum_7_spe_port_udp_lmtnum,spe_port_udp_lmtnum,HI_SPE_PORTX_UDP_LMTNUM_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_UDP_LMTNUM_7_OFFSET)
HI_SET_GET(hi_spe_portx_lmtbyte_0_spe_port_lmtbyte,spe_port_lmtbyte,HI_SPE_PORTX_LMTBYTE_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_LMTBYTE_0_OFFSET)
HI_SET_GET(hi_spe_portx_lmtbyte_1_spe_port_lmtbyte,spe_port_lmtbyte,HI_SPE_PORTX_LMTBYTE_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_LMTBYTE_1_OFFSET)
HI_SET_GET(hi_spe_portx_lmtbyte_2_spe_port_lmtbyte,spe_port_lmtbyte,HI_SPE_PORTX_LMTBYTE_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_LMTBYTE_2_OFFSET)
HI_SET_GET(hi_spe_portx_lmtbyte_3_spe_port_lmtbyte,spe_port_lmtbyte,HI_SPE_PORTX_LMTBYTE_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_LMTBYTE_3_OFFSET)
HI_SET_GET(hi_spe_portx_lmtbyte_4_spe_port_lmtbyte,spe_port_lmtbyte,HI_SPE_PORTX_LMTBYTE_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_LMTBYTE_4_OFFSET)
HI_SET_GET(hi_spe_portx_lmtbyte_5_spe_port_lmtbyte,spe_port_lmtbyte,HI_SPE_PORTX_LMTBYTE_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_LMTBYTE_5_OFFSET)
HI_SET_GET(hi_spe_portx_lmtbyte_6_spe_port_lmtbyte,spe_port_lmtbyte,HI_SPE_PORTX_LMTBYTE_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_LMTBYTE_6_OFFSET)
HI_SET_GET(hi_spe_portx_lmtbyte_7_spe_port_lmtbyte,spe_port_lmtbyte,HI_SPE_PORTX_LMTBYTE_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_LMTBYTE_7_OFFSET)
HI_SET_GET(hi_spe_portx_property_0_spe_port_attach_brg,spe_port_attach_brg,HI_SPE_PORTX_PROPERTY_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_0_OFFSET)
HI_SET_GET(hi_spe_portx_property_0_spe_port_enc_type,spe_port_enc_type,HI_SPE_PORTX_PROPERTY_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_0_OFFSET)
HI_SET_GET(hi_spe_portx_property_0_spe_port_pad_en,spe_port_pad_en,HI_SPE_PORTX_PROPERTY_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_0_OFFSET)
HI_SET_GET(hi_spe_portx_property_0_reserved,reserved,HI_SPE_PORTX_PROPERTY_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_0_OFFSET)
HI_SET_GET(hi_spe_portx_property_0_spe_port_en,spe_port_en,HI_SPE_PORTX_PROPERTY_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_0_OFFSET)
HI_SET_GET(hi_spe_portx_property_1_spe_port_attach_brg,spe_port_attach_brg,HI_SPE_PORTX_PROPERTY_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_1_OFFSET)
HI_SET_GET(hi_spe_portx_property_1_spe_port_enc_type,spe_port_enc_type,HI_SPE_PORTX_PROPERTY_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_1_OFFSET)
HI_SET_GET(hi_spe_portx_property_1_spe_port_pad_en,spe_port_pad_en,HI_SPE_PORTX_PROPERTY_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_1_OFFSET)
HI_SET_GET(hi_spe_portx_property_1_reserved,reserved,HI_SPE_PORTX_PROPERTY_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_1_OFFSET)
HI_SET_GET(hi_spe_portx_property_1_spe_port_en,spe_port_en,HI_SPE_PORTX_PROPERTY_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_1_OFFSET)
HI_SET_GET(hi_spe_portx_property_2_spe_port_attach_brg,spe_port_attach_brg,HI_SPE_PORTX_PROPERTY_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_2_OFFSET)
HI_SET_GET(hi_spe_portx_property_2_spe_port_enc_type,spe_port_enc_type,HI_SPE_PORTX_PROPERTY_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_2_OFFSET)
HI_SET_GET(hi_spe_portx_property_2_spe_port_pad_en,spe_port_pad_en,HI_SPE_PORTX_PROPERTY_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_2_OFFSET)
HI_SET_GET(hi_spe_portx_property_2_reserved,reserved,HI_SPE_PORTX_PROPERTY_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_2_OFFSET)
HI_SET_GET(hi_spe_portx_property_2_spe_port_en,spe_port_en,HI_SPE_PORTX_PROPERTY_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_2_OFFSET)
HI_SET_GET(hi_spe_portx_property_3_spe_port_attach_brg,spe_port_attach_brg,HI_SPE_PORTX_PROPERTY_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_3_OFFSET)
HI_SET_GET(hi_spe_portx_property_3_spe_port_enc_type,spe_port_enc_type,HI_SPE_PORTX_PROPERTY_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_3_OFFSET)
HI_SET_GET(hi_spe_portx_property_3_spe_port_pad_en,spe_port_pad_en,HI_SPE_PORTX_PROPERTY_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_3_OFFSET)
HI_SET_GET(hi_spe_portx_property_3_reserved,reserved,HI_SPE_PORTX_PROPERTY_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_3_OFFSET)
HI_SET_GET(hi_spe_portx_property_3_spe_port_en,spe_port_en,HI_SPE_PORTX_PROPERTY_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_3_OFFSET)
HI_SET_GET(hi_spe_portx_property_4_spe_port_attach_brg,spe_port_attach_brg,HI_SPE_PORTX_PROPERTY_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_4_OFFSET)
HI_SET_GET(hi_spe_portx_property_4_spe_port_enc_type,spe_port_enc_type,HI_SPE_PORTX_PROPERTY_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_4_OFFSET)
HI_SET_GET(hi_spe_portx_property_4_spe_port_pad_en,spe_port_pad_en,HI_SPE_PORTX_PROPERTY_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_4_OFFSET)
HI_SET_GET(hi_spe_portx_property_4_reserved,reserved,HI_SPE_PORTX_PROPERTY_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_4_OFFSET)
HI_SET_GET(hi_spe_portx_property_4_spe_port_en,spe_port_en,HI_SPE_PORTX_PROPERTY_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_4_OFFSET)
HI_SET_GET(hi_spe_portx_property_5_spe_port_attach_brg,spe_port_attach_brg,HI_SPE_PORTX_PROPERTY_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_5_OFFSET)
HI_SET_GET(hi_spe_portx_property_5_spe_port_enc_type,spe_port_enc_type,HI_SPE_PORTX_PROPERTY_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_5_OFFSET)
HI_SET_GET(hi_spe_portx_property_5_spe_port_pad_en,spe_port_pad_en,HI_SPE_PORTX_PROPERTY_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_5_OFFSET)
HI_SET_GET(hi_spe_portx_property_5_reserved,reserved,HI_SPE_PORTX_PROPERTY_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_5_OFFSET)
HI_SET_GET(hi_spe_portx_property_5_spe_port_en,spe_port_en,HI_SPE_PORTX_PROPERTY_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_5_OFFSET)
HI_SET_GET(hi_spe_portx_property_6_spe_port_attach_brg,spe_port_attach_brg,HI_SPE_PORTX_PROPERTY_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_6_OFFSET)
HI_SET_GET(hi_spe_portx_property_6_spe_port_enc_type,spe_port_enc_type,HI_SPE_PORTX_PROPERTY_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_6_OFFSET)
HI_SET_GET(hi_spe_portx_property_6_spe_port_pad_en,spe_port_pad_en,HI_SPE_PORTX_PROPERTY_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_6_OFFSET)
HI_SET_GET(hi_spe_portx_property_6_reserved,reserved,HI_SPE_PORTX_PROPERTY_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_6_OFFSET)
HI_SET_GET(hi_spe_portx_property_6_spe_port_en,spe_port_en,HI_SPE_PORTX_PROPERTY_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_6_OFFSET)
HI_SET_GET(hi_spe_portx_property_7_spe_port_attach_brg,spe_port_attach_brg,HI_SPE_PORTX_PROPERTY_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_7_OFFSET)
HI_SET_GET(hi_spe_portx_property_7_spe_port_enc_type,spe_port_enc_type,HI_SPE_PORTX_PROPERTY_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_7_OFFSET)
HI_SET_GET(hi_spe_portx_property_7_spe_port_pad_en,spe_port_pad_en,HI_SPE_PORTX_PROPERTY_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_7_OFFSET)
HI_SET_GET(hi_spe_portx_property_7_reserved,reserved,HI_SPE_PORTX_PROPERTY_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_7_OFFSET)
HI_SET_GET(hi_spe_portx_property_7_spe_port_en,spe_port_en,HI_SPE_PORTX_PROPERTY_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_PROPERTY_7_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_l_0_spe_port_mac_addr_l,spe_port_mac_addr_l,HI_SPE_PORTX_MAC_ADDR_L_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_L_0_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_l_1_spe_port_mac_addr_l,spe_port_mac_addr_l,HI_SPE_PORTX_MAC_ADDR_L_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_L_1_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_l_2_spe_port_mac_addr_l,spe_port_mac_addr_l,HI_SPE_PORTX_MAC_ADDR_L_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_L_2_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_l_3_spe_port_mac_addr_l,spe_port_mac_addr_l,HI_SPE_PORTX_MAC_ADDR_L_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_L_3_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_l_4_spe_port_mac_addr_l,spe_port_mac_addr_l,HI_SPE_PORTX_MAC_ADDR_L_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_L_4_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_l_5_spe_port_mac_addr_l,spe_port_mac_addr_l,HI_SPE_PORTX_MAC_ADDR_L_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_L_5_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_l_6_spe_port_mac_addr_l,spe_port_mac_addr_l,HI_SPE_PORTX_MAC_ADDR_L_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_L_6_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_l_7_spe_port_mac_addr_l,spe_port_mac_addr_l,HI_SPE_PORTX_MAC_ADDR_L_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_L_7_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_0_spe_port_mac_addr_h,spe_port_mac_addr_h,HI_SPE_PORTX_MAC_ADDR_H_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_0_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_0_reserved,reserved,HI_SPE_PORTX_MAC_ADDR_H_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_0_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_1_spe_port_mac_addr_h,spe_port_mac_addr_h,HI_SPE_PORTX_MAC_ADDR_H_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_1_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_1_reserved,reserved,HI_SPE_PORTX_MAC_ADDR_H_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_1_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_2_spe_port_mac_addr_h,spe_port_mac_addr_h,HI_SPE_PORTX_MAC_ADDR_H_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_2_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_2_reserved,reserved,HI_SPE_PORTX_MAC_ADDR_H_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_2_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_3_spe_port_mac_addr_h,spe_port_mac_addr_h,HI_SPE_PORTX_MAC_ADDR_H_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_3_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_3_reserved,reserved,HI_SPE_PORTX_MAC_ADDR_H_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_3_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_4_spe_port_mac_addr_h,spe_port_mac_addr_h,HI_SPE_PORTX_MAC_ADDR_H_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_4_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_4_reserved,reserved,HI_SPE_PORTX_MAC_ADDR_H_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_4_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_5_spe_port_mac_addr_h,spe_port_mac_addr_h,HI_SPE_PORTX_MAC_ADDR_H_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_5_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_5_reserved,reserved,HI_SPE_PORTX_MAC_ADDR_H_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_5_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_6_spe_port_mac_addr_h,spe_port_mac_addr_h,HI_SPE_PORTX_MAC_ADDR_H_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_6_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_6_reserved,reserved,HI_SPE_PORTX_MAC_ADDR_H_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_6_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_7_spe_port_mac_addr_h,spe_port_mac_addr_h,HI_SPE_PORTX_MAC_ADDR_H_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_7_OFFSET)
HI_SET_GET(hi_spe_portx_mac_addr_h_7_reserved,reserved,HI_SPE_PORTX_MAC_ADDR_H_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_MAC_ADDR_H_7_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_1st_0_spe_port_ipv6_addr_1st,spe_port_ipv6_addr_1st,HI_SPE_PORTX_IPV6_ADDR_1ST_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_1ST_0_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_1st_1_spe_port_ipv6_addr_1st,spe_port_ipv6_addr_1st,HI_SPE_PORTX_IPV6_ADDR_1ST_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_1ST_1_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_1st_2_spe_port_ipv6_addr_1st,spe_port_ipv6_addr_1st,HI_SPE_PORTX_IPV6_ADDR_1ST_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_1ST_2_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_1st_3_spe_port_ipv6_addr_1st,spe_port_ipv6_addr_1st,HI_SPE_PORTX_IPV6_ADDR_1ST_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_1ST_3_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_1st_4_spe_port_ipv6_addr_1st,spe_port_ipv6_addr_1st,HI_SPE_PORTX_IPV6_ADDR_1ST_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_1ST_4_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_1st_5_spe_port_ipv6_addr_1st,spe_port_ipv6_addr_1st,HI_SPE_PORTX_IPV6_ADDR_1ST_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_1ST_5_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_1st_6_spe_port_ipv6_addr_1st,spe_port_ipv6_addr_1st,HI_SPE_PORTX_IPV6_ADDR_1ST_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_1ST_6_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_1st_7_spe_port_ipv6_addr_1st,spe_port_ipv6_addr_1st,HI_SPE_PORTX_IPV6_ADDR_1ST_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_1ST_7_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_2nd_0_spe_port_ipv6_addr_2nd,spe_port_ipv6_addr_2nd,HI_SPE_PORTX_IPV6_ADDR_2ND_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_2ND_0_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_2nd_1_spe_port_ipv6_addr_2nd,spe_port_ipv6_addr_2nd,HI_SPE_PORTX_IPV6_ADDR_2ND_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_2ND_1_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_2nd_2_spe_port_ipv6_addr_2nd,spe_port_ipv6_addr_2nd,HI_SPE_PORTX_IPV6_ADDR_2ND_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_2ND_2_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_2nd_3_spe_port_ipv6_addr_2nd,spe_port_ipv6_addr_2nd,HI_SPE_PORTX_IPV6_ADDR_2ND_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_2ND_3_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_2nd_4_spe_port_ipv6_addr_2nd,spe_port_ipv6_addr_2nd,HI_SPE_PORTX_IPV6_ADDR_2ND_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_2ND_4_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_2nd_5_spe_port_ipv6_addr_2nd,spe_port_ipv6_addr_2nd,HI_SPE_PORTX_IPV6_ADDR_2ND_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_2ND_5_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_2nd_6_spe_port_ipv6_addr_2nd,spe_port_ipv6_addr_2nd,HI_SPE_PORTX_IPV6_ADDR_2ND_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_2ND_6_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_2nd_7_spe_port_ipv6_addr_2nd,spe_port_ipv6_addr_2nd,HI_SPE_PORTX_IPV6_ADDR_2ND_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_2ND_7_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_3th_0_spe_port_ipv6_addr_3th,spe_port_ipv6_addr_3th,HI_SPE_PORTX_IPV6_ADDR_3TH_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_3TH_0_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_3th_1_spe_port_ipv6_addr_3th,spe_port_ipv6_addr_3th,HI_SPE_PORTX_IPV6_ADDR_3TH_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_3TH_1_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_3th_2_spe_port_ipv6_addr_3th,spe_port_ipv6_addr_3th,HI_SPE_PORTX_IPV6_ADDR_3TH_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_3TH_2_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_3th_3_spe_port_ipv6_addr_3th,spe_port_ipv6_addr_3th,HI_SPE_PORTX_IPV6_ADDR_3TH_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_3TH_3_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_3th_4_spe_port_ipv6_addr_3th,spe_port_ipv6_addr_3th,HI_SPE_PORTX_IPV6_ADDR_3TH_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_3TH_4_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_3th_5_spe_port_ipv6_addr_3th,spe_port_ipv6_addr_3th,HI_SPE_PORTX_IPV6_ADDR_3TH_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_3TH_5_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_3th_6_spe_port_ipv6_addr_3th,spe_port_ipv6_addr_3th,HI_SPE_PORTX_IPV6_ADDR_3TH_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_3TH_6_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_3th_7_spe_port_ipv6_addr_3th,spe_port_ipv6_addr_3th,HI_SPE_PORTX_IPV6_ADDR_3TH_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_3TH_7_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_4th_0_spe_port_ipv6_addr_4th,spe_port_ipv6_addr_4th,HI_SPE_PORTX_IPV6_ADDR_4TH_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_4TH_0_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_4th_1_spe_port_ipv6_addr_4th,spe_port_ipv6_addr_4th,HI_SPE_PORTX_IPV6_ADDR_4TH_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_4TH_1_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_4th_2_spe_port_ipv6_addr_4th,spe_port_ipv6_addr_4th,HI_SPE_PORTX_IPV6_ADDR_4TH_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_4TH_2_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_4th_3_spe_port_ipv6_addr_4th,spe_port_ipv6_addr_4th,HI_SPE_PORTX_IPV6_ADDR_4TH_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_4TH_3_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_4th_4_spe_port_ipv6_addr_4th,spe_port_ipv6_addr_4th,HI_SPE_PORTX_IPV6_ADDR_4TH_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_4TH_4_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_4th_5_spe_port_ipv6_addr_4th,spe_port_ipv6_addr_4th,HI_SPE_PORTX_IPV6_ADDR_4TH_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_4TH_5_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_4th_6_spe_port_ipv6_addr_4th,spe_port_ipv6_addr_4th,HI_SPE_PORTX_IPV6_ADDR_4TH_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_4TH_6_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_addr_4th_7_spe_port_ipv6_addr_4th,spe_port_ipv6_addr_4th,HI_SPE_PORTX_IPV6_ADDR_4TH_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_ADDR_4TH_7_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_0_spe_port_ipv6_mask,spe_port_ipv6_mask,HI_SPE_PORTX_IPV6_MASK_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_0_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_0_reserved,reserved,HI_SPE_PORTX_IPV6_MASK_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_0_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_1_spe_port_ipv6_mask,spe_port_ipv6_mask,HI_SPE_PORTX_IPV6_MASK_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_1_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_1_reserved,reserved,HI_SPE_PORTX_IPV6_MASK_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_1_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_2_spe_port_ipv6_mask,spe_port_ipv6_mask,HI_SPE_PORTX_IPV6_MASK_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_2_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_2_reserved,reserved,HI_SPE_PORTX_IPV6_MASK_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_2_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_3_spe_port_ipv6_mask,spe_port_ipv6_mask,HI_SPE_PORTX_IPV6_MASK_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_3_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_3_reserved,reserved,HI_SPE_PORTX_IPV6_MASK_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_3_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_4_spe_port_ipv6_mask,spe_port_ipv6_mask,HI_SPE_PORTX_IPV6_MASK_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_4_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_4_reserved,reserved,HI_SPE_PORTX_IPV6_MASK_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_4_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_5_spe_port_ipv6_mask,spe_port_ipv6_mask,HI_SPE_PORTX_IPV6_MASK_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_5_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_5_reserved,reserved,HI_SPE_PORTX_IPV6_MASK_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_5_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_6_spe_port_ipv6_mask,spe_port_ipv6_mask,HI_SPE_PORTX_IPV6_MASK_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_6_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_6_reserved,reserved,HI_SPE_PORTX_IPV6_MASK_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_6_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_7_spe_port_ipv6_mask,spe_port_ipv6_mask,HI_SPE_PORTX_IPV6_MASK_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_7_OFFSET)
HI_SET_GET(hi_spe_portx_ipv6_mask_7_reserved,reserved,HI_SPE_PORTX_IPV6_MASK_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV6_MASK_7_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_addr_0_spe_port_ipv4_addr,spe_port_ipv4_addr,HI_SPE_PORTX_IPV4_ADDR_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_ADDR_0_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_addr_1_spe_port_ipv4_addr,spe_port_ipv4_addr,HI_SPE_PORTX_IPV4_ADDR_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_ADDR_1_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_addr_2_spe_port_ipv4_addr,spe_port_ipv4_addr,HI_SPE_PORTX_IPV4_ADDR_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_ADDR_2_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_addr_3_spe_port_ipv4_addr,spe_port_ipv4_addr,HI_SPE_PORTX_IPV4_ADDR_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_ADDR_3_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_addr_4_spe_port_ipv4_addr,spe_port_ipv4_addr,HI_SPE_PORTX_IPV4_ADDR_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_ADDR_4_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_addr_5_spe_port_ipv4_addr,spe_port_ipv4_addr,HI_SPE_PORTX_IPV4_ADDR_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_ADDR_5_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_addr_6_spe_port_ipv4_addr,spe_port_ipv4_addr,HI_SPE_PORTX_IPV4_ADDR_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_ADDR_6_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_addr_7_spe_port_ipv4_addr,spe_port_ipv4_addr,HI_SPE_PORTX_IPV4_ADDR_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_ADDR_7_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_0_spe_port_ipv4_mask,spe_port_ipv4_mask,HI_SPE_PORTX_IPV4_MASK_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_0_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_0_reserved,reserved,HI_SPE_PORTX_IPV4_MASK_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_0_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_1_spe_port_ipv4_mask,spe_port_ipv4_mask,HI_SPE_PORTX_IPV4_MASK_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_1_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_1_reserved,reserved,HI_SPE_PORTX_IPV4_MASK_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_1_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_2_spe_port_ipv4_mask,spe_port_ipv4_mask,HI_SPE_PORTX_IPV4_MASK_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_2_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_2_reserved,reserved,HI_SPE_PORTX_IPV4_MASK_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_2_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_3_spe_port_ipv4_mask,spe_port_ipv4_mask,HI_SPE_PORTX_IPV4_MASK_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_3_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_3_reserved,reserved,HI_SPE_PORTX_IPV4_MASK_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_3_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_4_spe_port_ipv4_mask,spe_port_ipv4_mask,HI_SPE_PORTX_IPV4_MASK_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_4_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_4_reserved,reserved,HI_SPE_PORTX_IPV4_MASK_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_4_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_5_spe_port_ipv4_mask,spe_port_ipv4_mask,HI_SPE_PORTX_IPV4_MASK_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_5_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_5_reserved,reserved,HI_SPE_PORTX_IPV4_MASK_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_5_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_6_spe_port_ipv4_mask,spe_port_ipv4_mask,HI_SPE_PORTX_IPV4_MASK_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_6_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_6_reserved,reserved,HI_SPE_PORTX_IPV4_MASK_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_6_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_7_spe_port_ipv4_mask,spe_port_ipv4_mask,HI_SPE_PORTX_IPV4_MASK_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_7_OFFSET)
HI_SET_GET(hi_spe_portx_ipv4_mask_7_reserved,reserved,HI_SPE_PORTX_IPV4_MASK_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PORTX_IPV4_MASK_7_OFFSET)
HI_SET_GET(hi_spe_tab_ctrl_spe_tab_ctrl,spe_tab_ctrl,HI_SPE_TAB_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CTRL_OFFSET)
HI_SET_GET(hi_spe_tab_ctrl_reserved,reserved,HI_SPE_TAB_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CTRL_OFFSET)
HI_SET_GET(hi_spe_tab_act_result_spe_tab_done,spe_tab_done,HI_SPE_TAB_ACT_RESULT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_ACT_RESULT_OFFSET)
HI_SET_GET(hi_spe_tab_act_result_spe_tab_success,spe_tab_success,HI_SPE_TAB_ACT_RESULT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_ACT_RESULT_OFFSET)
HI_SET_GET(hi_spe_tab_act_result_reserved,reserved,HI_SPE_TAB_ACT_RESULT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_ACT_RESULT_OFFSET)
HI_SET_GET(hi_spe_tab_content_0_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_0_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_0_OFFSET)
HI_SET_GET(hi_spe_tab_content_1_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_1_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_1_OFFSET)
HI_SET_GET(hi_spe_tab_content_2_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_2_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_2_OFFSET)
HI_SET_GET(hi_spe_tab_content_3_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_3_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_3_OFFSET)
HI_SET_GET(hi_spe_tab_content_4_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_4_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_4_OFFSET)
HI_SET_GET(hi_spe_tab_content_5_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_5_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_5_OFFSET)
HI_SET_GET(hi_spe_tab_content_6_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_6_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_6_OFFSET)
HI_SET_GET(hi_spe_tab_content_7_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_7_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_7_OFFSET)
HI_SET_GET(hi_spe_tab_content_8_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_8_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_8_OFFSET)
HI_SET_GET(hi_spe_tab_content_9_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_9_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_9_OFFSET)
HI_SET_GET(hi_spe_tab_content_10_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_10_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_10_OFFSET)
HI_SET_GET(hi_spe_tab_content_11_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_11_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_11_OFFSET)
HI_SET_GET(hi_spe_tab_content_12_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_12_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_12_OFFSET)
HI_SET_GET(hi_spe_tab_content_13_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_13_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_13_OFFSET)
HI_SET_GET(hi_spe_tab_content_14_spe_tab_content,spe_tab_content,HI_SPE_TAB_CONTENT_14_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_TAB_CONTENT_14_OFFSET)
HI_SET_GET(hi_spe_hash_baddr_spe_hash_baddr,spe_hash_baddr,HI_SPE_HASH_BADDR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_HASH_BADDR_OFFSET)
HI_SET_GET(hi_spe_hash_zone_spe_hash_zone,spe_hash_zone,HI_SPE_HASH_ZONE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_HASH_ZONE_OFFSET)
HI_SET_GET(hi_spe_hash_rand_spe_hash_rand,spe_hash_rand,HI_SPE_HASH_RAND_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_HASH_RAND_OFFSET)
HI_SET_GET(hi_spe_hash_l3_proto_spe_ipv4_hash_l3_proto,spe_ipv4_hash_l3_proto,HI_SPE_HASH_L3_PROTO_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_HASH_L3_PROTO_OFFSET)
HI_SET_GET(hi_spe_hash_l3_proto_spe_ipv6_hash_l3_proto,spe_ipv6_hash_l3_proto,HI_SPE_HASH_L3_PROTO_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_HASH_L3_PROTO_OFFSET)
HI_SET_GET(hi_spe_hash_width_spe_hash_width,spe_hash_width,HI_SPE_HASH_WIDTH_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_HASH_WIDTH_OFFSET)
HI_SET_GET(hi_spe_hash_width_reserved,reserved,HI_SPE_HASH_WIDTH_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_HASH_WIDTH_OFFSET)
HI_SET_GET(hi_spe_hash_depth_spe_hash_depth,spe_hash_depth,HI_SPE_HASH_DEPTH_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_HASH_DEPTH_OFFSET)
HI_SET_GET(hi_spe_hash_depth_reserved,reserved,HI_SPE_HASH_DEPTH_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_HASH_DEPTH_OFFSET)
HI_SET_GET(hi_spe_aging_time_spe_ip_aging_time,spe_ip_aging_time,HI_SPE_AGING_TIME_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_AGING_TIME_OFFSET)
HI_SET_GET(hi_spe_aging_time_spe_mac_aging_time,spe_mac_aging_time,HI_SPE_AGING_TIME_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_AGING_TIME_OFFSET)
HI_SET_GET(hi_spe_pack_max_time_spe_pack_max_time,spe_pack_max_time,HI_SPE_PACK_MAX_TIME_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PACK_MAX_TIME_OFFSET)
HI_SET_GET(hi_spe_pack_max_time_reserved,reserved,HI_SPE_PACK_MAX_TIME_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PACK_MAX_TIME_OFFSET)
HI_SET_GET(hi_spe_pack_ctrl_spe_pack_max_pkt_cnt,spe_pack_max_pkt_cnt,HI_SPE_PACK_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PACK_CTRL_OFFSET)
HI_SET_GET(hi_spe_pack_ctrl_reserved_1,reserved_1,HI_SPE_PACK_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PACK_CTRL_OFFSET)
HI_SET_GET(hi_spe_pack_ctrl_spe_pack_max_len,spe_pack_max_len,HI_SPE_PACK_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PACK_CTRL_OFFSET)
HI_SET_GET(hi_spe_pack_ctrl_reserved_0,reserved_0,HI_SPE_PACK_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PACK_CTRL_OFFSET)
HI_SET_GET(hi_spe_pack_addr_ctrl_spe_pack_remainder,spe_pack_remainder,HI_SPE_PACK_ADDR_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PACK_ADDR_CTRL_OFFSET)
HI_SET_GET(hi_spe_pack_addr_ctrl_spe_pack_divisor,spe_pack_divisor,HI_SPE_PACK_ADDR_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PACK_ADDR_CTRL_OFFSET)
HI_SET_GET(hi_spe_pack_remain_len_spe_pack_remain_len,spe_pack_remain_len,HI_SPE_PACK_REMAIN_LEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PACK_REMAIN_LEN_OFFSET)
HI_SET_GET(hi_spe_pack_remain_len_reserved,reserved,HI_SPE_PACK_REMAIN_LEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PACK_REMAIN_LEN_OFFSET)
HI_SET_GET(hi_spe_unpack_ctrl_spe_unpack_ctrl,spe_unpack_ctrl,HI_SPE_UNPACK_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_UNPACK_CTRL_OFFSET)
HI_SET_GET(hi_spe_unpack_ctrl_reserved,reserved,HI_SPE_UNPACK_CTRL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_UNPACK_CTRL_OFFSET)
HI_SET_GET(hi_spe_unpack_max_len_spe_rndis_max_len,spe_rndis_max_len,HI_SPE_UNPACK_MAX_LEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_UNPACK_MAX_LEN_OFFSET)
HI_SET_GET(hi_spe_unpack_max_len_spe_ncm_max_len,spe_ncm_max_len,HI_SPE_UNPACK_MAX_LEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_UNPACK_MAX_LEN_OFFSET)
HI_SET_GET(hi_spe_event_buff0_ptr_spe_event_buff0_wptr,spe_event_buff0_wptr,HI_SPE_EVENT_BUFF0_PTR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_PTR_OFFSET)
HI_SET_GET(hi_spe_event_buff0_ptr_reserved_1,reserved_1,HI_SPE_EVENT_BUFF0_PTR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_PTR_OFFSET)
HI_SET_GET(hi_spe_event_buff0_ptr_spe_event_buff0_rptr,spe_event_buff0_rptr,HI_SPE_EVENT_BUFF0_PTR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_PTR_OFFSET)
HI_SET_GET(hi_spe_event_buff0_ptr_reserved_0,reserved_0,HI_SPE_EVENT_BUFF0_PTR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_PTR_OFFSET)
HI_SET_GET(hi_spe_event_buff0_addr_spe_event_buff0_addr,spe_event_buff0_addr,HI_SPE_EVENT_BUFF0_ADDR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_ADDR_OFFSET)
HI_SET_GET(hi_spe_event_buff1_ptr_spe_event_buff1_wptr,spe_event_buff1_wptr,HI_SPE_EVENT_BUFF1_PTR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_PTR_OFFSET)
HI_SET_GET(hi_spe_event_buff1_ptr_reserved_1,reserved_1,HI_SPE_EVENT_BUFF1_PTR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_PTR_OFFSET)
HI_SET_GET(hi_spe_event_buff1_ptr_spe_event_buff1_rptr,spe_event_buff1_rptr,HI_SPE_EVENT_BUFF1_PTR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_PTR_OFFSET)
HI_SET_GET(hi_spe_event_buff1_ptr_reserved_0,reserved_0,HI_SPE_EVENT_BUFF1_PTR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_PTR_OFFSET)
HI_SET_GET(hi_spe_event_buff1_addr_spe_event_buff1_addr,spe_event_buff1_addr,HI_SPE_EVENT_BUFF1_ADDR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_ADDR_OFFSET)
HI_SET_GET(hi_spe_event_buff_len_spe_event_buff0_len,spe_event_buff0_len,HI_SPE_EVENT_BUFF_LEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF_LEN_OFFSET)
HI_SET_GET(hi_spe_event_buff_len_reserved_1,reserved_1,HI_SPE_EVENT_BUFF_LEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF_LEN_OFFSET)
HI_SET_GET(hi_spe_event_buff_len_spe_event_buff1_len,spe_event_buff1_len,HI_SPE_EVENT_BUFF_LEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF_LEN_OFFSET)
HI_SET_GET(hi_spe_event_buff_len_reserved_0,reserved_0,HI_SPE_EVENT_BUFF_LEN_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF_LEN_OFFSET)
HI_SET_GET(hi_spe_int0_interval_spe_int0_interval,spe_int0_interval,HI_SPE_INT0_INTERVAL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_INT0_INTERVAL_OFFSET)
HI_SET_GET(hi_spe_int1_interval_spe_int1_interval,spe_int1_interval,HI_SPE_INT1_INTERVAL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_INT1_INTERVAL_OFFSET)
HI_SET_GET(hi_spe_event_buff0_mask_rdq_empty_int0_mask,rdq_empty_int0_mask,HI_SPE_EVENT_BUFF0_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_MASK_OFFSET)
HI_SET_GET(hi_spe_event_buff0_mask_rdq_full_int0_mask,rdq_full_int0_mask,HI_SPE_EVENT_BUFF0_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_MASK_OFFSET)
HI_SET_GET(hi_spe_event_buff0_mask_rd_done_int0_mask,rd_done_int0_mask,HI_SPE_EVENT_BUFF0_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_MASK_OFFSET)
HI_SET_GET(hi_spe_event_buff0_mask_tdq_empty_int0_mask,tdq_empty_int0_mask,HI_SPE_EVENT_BUFF0_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_MASK_OFFSET)
HI_SET_GET(hi_spe_event_buff0_mask_tdq_full_int0_mask,tdq_full_int0_mask,HI_SPE_EVENT_BUFF0_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_MASK_OFFSET)
HI_SET_GET(hi_spe_event_buff0_mask_td_done_int0_mask,td_done_int0_mask,HI_SPE_EVENT_BUFF0_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_MASK_OFFSET)
HI_SET_GET(hi_spe_event_buff0_mask_reserved,reserved,HI_SPE_EVENT_BUFF0_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_MASK_OFFSET)
HI_SET_GET(hi_spe_event_buff1_mask_rdq_empty_int1_mask,rdq_empty_int1_mask,HI_SPE_EVENT_BUFF1_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_MASK_OFFSET)
HI_SET_GET(hi_spe_event_buff1_mask_rdq_full_int1_mask,rdq_full_int1_mask,HI_SPE_EVENT_BUFF1_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_MASK_OFFSET)
HI_SET_GET(hi_spe_event_buff1_mask_rd_done_int1_mask,rd_done_int1_mask,HI_SPE_EVENT_BUFF1_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_MASK_OFFSET)
HI_SET_GET(hi_spe_event_buff1_mask_tdq_empty_int1_mask,tdq_empty_int1_mask,HI_SPE_EVENT_BUFF1_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_MASK_OFFSET)
HI_SET_GET(hi_spe_event_buff1_mask_tdq_full_int1_mask,tdq_full_int1_mask,HI_SPE_EVENT_BUFF1_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_MASK_OFFSET)
HI_SET_GET(hi_spe_event_buff1_mask_td_done_int1_mask,td_done_int1_mask,HI_SPE_EVENT_BUFF1_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_MASK_OFFSET)
HI_SET_GET(hi_spe_event_buff1_mask_reserved,reserved,HI_SPE_EVENT_BUFF1_MASK_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_MASK_OFFSET)
HI_SET_GET(hi_spe_eventbuf0_pro_cnt_spe_event_buf0_pro_cnt,spe_event_buf0_pro_cnt,HI_SPE_EVENTBUF0_PRO_CNT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENTBUF0_PRO_CNT_OFFSET)
HI_SET_GET(hi_spe_eventbuf0_pro_cnt_reserved,reserved,HI_SPE_EVENTBUF0_PRO_CNT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENTBUF0_PRO_CNT_OFFSET)
HI_SET_GET(hi_spe_eventbuf0_rpt_cnt_spe_event_buf0_rpt_cnt,spe_event_buf0_rpt_cnt,HI_SPE_EVENTBUF0_RPT_CNT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENTBUF0_RPT_CNT_OFFSET)
HI_SET_GET(hi_spe_eventbuf0_rpt_cnt_reserved,reserved,HI_SPE_EVENTBUF0_RPT_CNT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENTBUF0_RPT_CNT_OFFSET)
HI_SET_GET(hi_spe_eventbuf1_pro_cnt_spe_event_buf1_pro_cnt,spe_event_buf1_pro_cnt,HI_SPE_EVENTBUF1_PRO_CNT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENTBUF1_PRO_CNT_OFFSET)
HI_SET_GET(hi_spe_eventbuf1_pro_cnt_reserved,reserved,HI_SPE_EVENTBUF1_PRO_CNT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENTBUF1_PRO_CNT_OFFSET)
HI_SET_GET(hi_spe_eventbuf1_rpt_cnt_spe_event_buf1_rpt_cnt,spe_event_buf1_rpt_cnt,HI_SPE_EVENTBUF1_RPT_CNT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENTBUF1_RPT_CNT_OFFSET)
HI_SET_GET(hi_spe_eventbuf1_rpt_cnt_reserved,reserved,HI_SPE_EVENTBUF1_RPT_CNT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENTBUF1_RPT_CNT_OFFSET)
HI_SET_GET(hi_spe_int_state_spe_event_buff0_int_state,spe_event_buff0_int_state,HI_SPE_INT_STATE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_INT_STATE_OFFSET)
HI_SET_GET(hi_spe_int_state_spe_event_buff1_int_state,spe_event_buff1_int_state,HI_SPE_INT_STATE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_INT_STATE_OFFSET)
HI_SET_GET(hi_spe_int_state_reserved,reserved,HI_SPE_INT_STATE_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_INT_STATE_OFFSET)
HI_SET_GET(hi_spe_event_buff_full_spe_event_buff0_full,spe_event_buff0_full,HI_SPE_EVENT_BUFF_FULL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF_FULL_OFFSET)
HI_SET_GET(hi_spe_event_buff_full_spe_event_buff1_full,spe_event_buff1_full,HI_SPE_EVENT_BUFF_FULL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF_FULL_OFFSET)
HI_SET_GET(hi_spe_event_buff_full_reserved,reserved,HI_SPE_EVENT_BUFF_FULL_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF_FULL_OFFSET)
HI_SET_GET(hi_spe_event_buff0_rd_err_spe_event_buf0_rd_err,spe_event_buf0_rd_err,HI_SPE_EVENT_BUFF0_RD_ERR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_RD_ERR_OFFSET)
HI_SET_GET(hi_spe_event_buff0_rd_err_reserved,reserved,HI_SPE_EVENT_BUFF0_RD_ERR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF0_RD_ERR_OFFSET)
HI_SET_GET(hi_spe_event_buff1_rd_err_spe_event_buf1_rd_err,spe_event_buf1_rd_err,HI_SPE_EVENT_BUFF1_RD_ERR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_RD_ERR_OFFSET)
HI_SET_GET(hi_spe_event_buff1_rd_err_reserved,reserved,HI_SPE_EVENT_BUFF1_RD_ERR_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_EVENT_BUFF1_RD_ERR_OFFSET)
HI_SET_GET(hi_spe_wrr_debug_spe_wrr_debug,spe_wrr_debug,HI_SPE_WRR_DEBUG_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_WRR_DEBUG_OFFSET)
HI_SET_GET(hi_spe_unpack_debug_spe_unpack_debug,spe_unpack_debug,HI_SPE_UNPACK_DEBUG_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_UNPACK_DEBUG_OFFSET)
HI_SET_GET(hi_spe_eng_debug_spe_eng_debug,spe_eng_debug,HI_SPE_ENG_DEBUG_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_ENG_DEBUG_OFFSET)
HI_SET_GET(hi_spe_pack_debug_spe_pack_debug,spe_pack_debug,HI_SPE_PACK_DEBUG_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PACK_DEBUG_OFFSET)
HI_SET_GET(hi_spe_done_debug_spe_done_debug,spe_done_debug,HI_SPE_DONE_DEBUG_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_DONE_DEBUG_OFFSET)
HI_SET_GET(hi_spe_if_debug_spe_if_debug,spe_if_debug,HI_SPE_IF_DEBUG_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_IF_DEBUG_OFFSET)
HI_SET_GET(hi_spe_lp_rd_block_cnt_spe_lp_rd_block_cnt,spe_lp_rd_block_cnt,HI_SPE_LP_RD_BLOCK_CNT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_LP_RD_BLOCK_CNT_OFFSET)
HI_SET_GET(hi_spe_hp_rd_block_cnt_spe_hp_rd_block_cnt,spe_hp_rd_block_cnt,HI_SPE_HP_RD_BLOCK_CNT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_HP_RD_BLOCK_CNT_OFFSET)
HI_SET_GET(hi_spe_pack_overtime_cnt_spe_pack_overtime_cnt,spe_pack_overtime_cnt,HI_SPE_PACK_OVERTIME_CNT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PACK_OVERTIME_CNT_OFFSET)
HI_SET_GET(hi_spe_pack_push_cnt_spe_pack_push_cnt,spe_pack_push_cnt,HI_SPE_PACK_PUSH_CNT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_PACK_PUSH_CNT_OFFSET)
HI_SET_GET(hi_spe_depack_err_cnt_spe_depack_err_cnt,spe_depack_err_cnt,HI_SPE_DEPACK_ERR_CNT_T,HI_SPE_MPI_BASE_ADDR, HI_SPE_DEPACK_ERR_CNT_OFFSET)

#endif

#endif // __HI_SPE_MPI_H__

