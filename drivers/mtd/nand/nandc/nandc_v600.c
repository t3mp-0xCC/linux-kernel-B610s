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
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include "nandc_ctrl.h"
/*lint --e{3745,550,830,746}*/
/*寄存器的数据结构的定义*/
struct nandc_bit_cluster nandc6_bit_cluster =
{
    STU_SET(.cmd)
    {
        NANDC_REG_CMD_OFFSET,
        STU_SET(.bitset)
        {
            0, 32
        }

    },
    STU_SET(.addr_low)
    {
        NANDC_REG_ADDRL_OFFSET,
        STU_SET(.bitset)
        {
            0, 32
        }

    },
    STU_SET(.addr_high)
    {
        NANDC_REG_ADDRH_OFFSET,
        STU_SET(.bitset)
        {
            0, 32
        }

    },
    STU_SET(.addr_cycle)
    {
        NANDC_REG_OP_OFFSET,
        STU_SET(.bitset)
        {
            9, 3
        }

    },
    STU_SET(.operate)
    {
        NANDC_REG_OP_OFFSET,
        STU_SET(.bitset)
        {
            0, 7
        }

    },
    STU_SET(.op_stauts)
    {
        NANDC_REG_STATUS_OFFSET,
        STU_SET(.bitset)
        {
            0, 1
        }

    },
    STU_SET(.int_stauts)
    {
        NANDC_REG_INTS_OFFSET,
        STU_SET(.bitset)
        {
            0, 1
        }

    },
    STU_SET(.data_num)
    {
        NANDC_REG_DATA_NUM_OFFSET,
        STU_SET(.bitset)
        {
            0, 32
        }

    },
    STU_SET(.ecc_type)
    {
        NANDC_REG_CON_OFFSET,
        STU_SET(.bitset)
        {
            8, 4
        }

    },
    STU_SET(.ecc_select)
    {
        NANDC_REG_OP_PARA_OFFSET,
        STU_SET(.bitset)
        {
            0, 32
        }

    },
    STU_SET(.int_enable)
    {
        NANDC_REG_INTEN_OFFSET,
        STU_SET(.bitset)
        {
            0, 32
        }

    },
    STU_SET(.op_mode)
    {
        NANDC_REG_CON_OFFSET,
        STU_SET(.bitset)
        {
            0, 1
        }

    },
    STU_SET(.page_size)
    {
        NANDC_REG_CON_OFFSET,
        STU_SET(.bitset)
        {
            1, 3
        }

    },
    STU_SET(.int_clr)
    {
        NANDC_REG_INTCLR_OFFSET,
        STU_SET(.bitset)
        {
            0, 32
        }

    },
    STU_SET(.ecc_result)
    {
        NANDC_REG_INTS_OFFSET,
        STU_SET(.bitset)
        {
            5, 2
        }

    },
    STU_SET(.nf_status)
    {
        NANDC_REG_STATUS_OFFSET,
        STU_SET(.bitset)
        {
            8, 8
        }

    },
    STU_SET(.errnum0buf0)
    {
        NANDC_REG_ERR_NUM0_BUF0_OFFSET,
        STU_SET(.bitset)
        {
            0, 32
        }

    },
    STU_SET(.rb_status)
    {
        NANDC_REG_RB_MODE_OFFSET,
        STU_SET(.bitset)
        {
            0, 1
        }

    },
    STU_SET(.oob_len_sel)
    {
        NANDC_REG_OOB_SEL_OFFSET,
        STU_SET(.bitset)
        {
            0, 1
        }

    }
};