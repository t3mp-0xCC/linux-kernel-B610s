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
#ifndef NANDC_MTD_H
#define NANDC_MTD_H
/*
* 说明: 驱动模块的主控实体描述符，包含所有当前控制对象信息和为此控制对象所定义的控制器信息。
*
*/
struct nandc_host
{
    struct nand_chip * chip;
    struct mtd_info  * mtd;
    struct clk  *clk;
    struct nandc_bit_cluster *cluster;
    u32 badoffset;

    u32 command;
    u32 chipselect;
    /*ecc0/非ecc0 */
    u32 options;
    /* 决定坏块管理使用page0和page1，还是last page */
    u32 bbt_options;

    u32 addr_cycle;
    u32 addr_physics[2];
    long unsigned int flash_addr;
    long unsigned int addr_logic;
    u32 length;                         /*for no ecc mode*/
    u8  flash_id[NANDC_READID_SIZE];
    u32 probwidth;
    void __iomem * regbase;       /* */
    void __iomem * bufbase;       /* */
    u32 regsize;
    u32 bufsize;
    /*如果此Flash支持ONFI的功能，那么在ONFI检测的过程中把ECCTYPE保存便于后面恢复*/
    u32 ecctype_onfi;
    u32 column;
    u32 offset;

#ifdef CONFIG_PM
    u32 *sleep_buffer;
#endif

};
#define NANDC_STATUS_SUSPEND    1
#define NANDC_STATUS_RESUME     0
#endif