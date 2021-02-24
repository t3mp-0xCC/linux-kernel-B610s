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
#ifndef NANDC_CTRL_H
#define NANDC_CTRL_H

#define    NANDC_REG_CON_OFFSET                                 (0x0) /* NANDC���üĴ����� */
#define    NANDC_REG_PWIDTH_OFFSET                              (0x4) /* ��д���������üĴ����� */
#define    NANDC_REG_OPIDLE_OFFSET                              (0x8) /* ����������üĴ����� */
#define    NANDC_REG_CMD_OFFSET                                 (0xC) /* ���������üĴ����� */
#define    NANDC_REG_ADDRL_OFFSET                               (0x10) /* ��λ��ַ���üĴ����� */
#define    NANDC_REG_ADDRH_OFFSET                               (0x14) /* ��λ��ַ���üĴ����� */
#define    NANDC_REG_DATA_NUM_OFFSET                            (0x18) /* ��д������Ŀ���üĴ����� */
#define    NANDC_REG_OP_OFFSET                                  (0x1C) /* �����Ĵ����� */
#define    NANDC_REG_STATUS_OFFSET                              (0x20) /* ״̬�Ĵ����� */
#define    NANDC_REG_INTEN_OFFSET                               (0x24) /* �ж�ʹ�ܼĴ����� */
#define    NANDC_REG_INTS_OFFSET                                (0x28) /* �ж�״̬�Ĵ����� */
#define    NANDC_REG_INTCLR_OFFSET                              (0x2C) /* �ж�����Ĵ����� */


#define    NANDC_REG_BADDR_D_OFFSET                             (0x64) /* �����������Ļ���ַ�Ĵ����� */
#define    NANDC_REG_BADDR_OOB_OFFSET                           (0x68) /* ����OOB���Ļ���ַ�Ĵ����� */
#define    NANDC_REG_DMA_LEN_OFFSET                             (0x6C) /* ���ͳ��ȼĴ����� */
#define    NANDC_REG_OP_PARA_OFFSET                             (0x70) /* ���������Ĵ����� */

#define    NANDC_REG_BOOT_SET_OFFSET                            (0x94) /* boot�����趨�Ĵ����� */
#define    NANDC_REG_LP_CTRL_OFFSET                              (0x9C) /* nandc �ĵ͹��Ŀ��ƼĴ����� */
#define    NANDC_REG_ERR_NUM0_BUF0_OFFSET                       (0xA0) /* nandflash ��һ��buffer����ͳ�ƼĴ���0 */
#define    NANDC_REG_ERR_NUM1_BUF0_OFFSET                       (0xA4) /* nandflash ��һ��buffer����ͳ�ƼĴ���1 */
#define    NANDC_REG_RB_MODE_OFFSET                              (0xB0) /* nandflash ��ready_busyģʽ�Ĵ����� */
#define    NANDC_REG_BADDR_D1_OFFSET                            (0xB4) /* �����������Ļ���ַ�Ĵ���1�� */
#define    NANDC_REG_BADDR_D2_OFFSET                            (0xB8) /* �����������Ļ���ַ�Ĵ���2�� */
#define    NANDC_REG_BADDR_D3_OFFSET                            (0xBC) /* �����������Ļ���ַ�Ĵ���3�� */
#define    NANDC_REG_BOOT_CFG_OFFSET                            (0xC4) /* nandflash ��boot���üĴ��� */
#define    NANDC_REG_OOB_SEL_OFFSET                             (0xC8) /* 16bit ECC��OOB������ѡ��Ĵ��� */

#define STU_SET(element)                            element =


/*��bitλ��������*/
#define NANDC_REG_GETBIT32(addr, pos, bits)     \
                 ((*(volatile u32 *)(addr)  >> (pos)) & \
                 (((u32)1 << (bits)) - 1))

/*��bitλ��д���ݵĲ���*/
#define NANDC_REG_SETBIT32(addr, pos, bits, val)       \
                (*(volatile u32 *)(addr) = (*(volatile u32 *)(addr) & \
                (~((((u32)1 << (bits)) - 1) << (pos)))) \
                 | ((u32)((val) & (((u32)1 << (bits)) - 1)) << (pos)))


/*д��ص�bitλ��ֵ,������bitλ��ֵΪ0*/
#define NANDC_REG_WRITEBIT32(addr, pos, bits, val)       \
                (*(volatile u32 *)(addr) =  ((u32)((val) & (((u32)1 << (bits)) - 1)) << (pos)))
/*�����д���bitλ�ļĴ���ʱҲ��Ѳ���صļĴ�����0��*/
#define NANDC_WRITE_REG_BITS(bitfunc, value)     \
                    NANDC_REG_WRITEBIT32((u32)host->regbase + ((bitfunc).reg_offset),        \
                                                            (bitfunc).bitset.offset ,        \
                                                            (bitfunc).bitset.length,         \
                                                            value);
/*���������������ӿ�*/
#define NANDC_GET_REG_BITS(bitfunc)     \
                    NANDC_REG_GETBIT32((u32)host->regbase + ((bitfunc).reg_offset),          \
                                                            (bitfunc).bitset.offset ,        \
                                                            (bitfunc).bitset.length)



#define NANDC_SET_REG_BITS(bitfunc, value)                                                  \
                    NANDC_REG_SETBIT32((u32)host->regbase + ((bitfunc).reg_offset),        \
                                                            (bitfunc).bitset.offset,        \
                                                            (bitfunc).bitset.length,         \
                                                            value);


#define NANDC_OK                           0
#define NANDC_ERROR                        1
#define NANDC_E_NOMEM                      2
#define NANDC_E_PARAM                      3
#define NANDC_E_NOFIND                     4
#define NANDC_E_STATUS                     5
#define NANDC_E_TIMEOUT                    6
#define NANDC_E_INIT                       7
#define NANDC_E_READID                     8
#define NANDC_E_NULL                       9
#define NANDC_E_ERASE                      10
#define NANDC_E_ADDR_OVERSTEP              11
#define NANDC_E_READ                       12
#define NANDC_E_ECC                        13
#define NANDC_E_CRC                        14

#define NANDC_STATUS_INT_DONE		        (1)
#define NAND_CMD_NOP		                (0)
#define NANDC_RD_RESULT_TIMEOUT                 (100000)
#define NANDC_PRG_RESULT_TIMEOUT                (1000000)
#define NANDC_ERASE_RESULT_TIMEOUT              (10000000)
#define NANDC_RESULT_TIME_OUT                   (10000000)
#define NANDC_STATUS_OP_DONE		        (1)
#define NANDC_MAX_CHIPS                         (1)

#define NANDC_ECC_ERR_VALID                 (0x1)
#define NANDC_ECC_ERR_INVALID               (0x2)
#define NANDC_SLEEP_BUFFER_SIZE     (0x20)

#define INTMASK_ALL                (0x7ff)
#define INTDIS_ALL                 (0x000)
#define ECC_NONE                   (0x00)
#define OP_ERASE                      (0x6d)
#define OP_RESET                      (0x44)
#define OP_READ_ID                    (0x66)

#define NANDC_OPTION_ECC_FORBID             (0x0004)
#define NANDC_BUFSIZE_BASIC			        (8192)
#define OP_READ_PAGE                        (0x6e)
#define OP_WRITE_PAGE                       (0x7d)

#define NAND_SPEC_ALREADY_SAVED         0x5A5A5A5A

#define NANDC_BAD_FLAG_BYTE_SIZE                (4)
#define NANDC_ADDR_CYCLE_MASK                   (0x4)
#define NANDC_CHIP_DELAY                        (25)
#define NANDC_READID_SIZE	                    (8)
#define END_FLAG    0xFFFFFFFF
#define NANDC_NAME                             "NANDC_V6.00"

#define NANDC_TRACE_CTRL			    0x00000004
#define NANDC_TRACE_ALL		            0x0FFFFFFF
#define NANDC_TRACE_ERRO			    0x00000000
#define NANDC_TRACE_WARNING		        0x10000000
#define NANDC_TRACE_NORMAL		        0x20000000
#define NANDC_TRACE_LEVEL			    0xF0000000

extern u32 nandc_trace_mask;
extern unsigned int g_nand_cmd_status;
//#define NANDC_READ_RESULT_DELAY

#define NFCDBGLVL(LVL)       (NANDC_TRACE_CTRL|NANDC_TRACE_##LVL)
#define NANDC_TRACE(mask, p)    \
/*lint -save -e778*/    /*do*/{                                                                                        \
        if (((mask) & nandc_trace_mask & NANDC_TRACE_ALL) &&                                                       \
        ((s32)((mask) &  (NANDC_TRACE_LEVEL)) <= (s32)((nandc_trace_mask) & (NANDC_TRACE_LEVEL)))) \
        {                                                                                           \
            /* coverity[printf_arg_mismatch] */         \
            printk p;                                                                          \
        }                                                                                           \
    } /*while(0)*//*lint -restore */
/*�Ĵ�����bit����*/
struct nandc_bitset
{
    u32 offset;
    u32 length;
};

/*nandcģ���bit�Ĵ�������,���Ǳ�ʾ��һ���Ĵ����ļ���bitλ�����ֵ*/
struct nandc_bit_reg
{
	/*�Ĵ��������ַ��ƫ��*/
    u32 reg_offset ;
	/*�Ĵ�����bit����*/
    struct nandc_bitset  bitset;
};

/*
* ���ﶨ����������еļĴ�����������ϼ�����Щ��������������������ȡ��ִ�У�
* �ϼ��е�ÿ����λ����һ���Ĵ�����bitλ�������������еľ���������������ʼ��֮ǰ�ǲ��հ׵ģ�
* �ڳ�ʼ�������л���һϵ�в�����������ͨ���������Ϳ��ƶ���������ԣ�������ı�Ҫ��Ŀ
* ������䣬�����һ�ṹ���ݵĲ��컯���á�
*/
struct nandc_bit_cluster
{
    struct nandc_bit_reg cmd;
    struct nandc_bit_reg addr_low;
    struct nandc_bit_reg addr_high;

    struct nandc_bit_reg addr_cycle;
    struct nandc_bit_reg operate;

    struct nandc_bit_reg op_stauts;
    struct nandc_bit_reg int_stauts;
    struct nandc_bit_reg data_num;

    struct nandc_bit_reg ecc_type;
    struct nandc_bit_reg ecc_select;

    struct nandc_bit_reg int_enable;

    struct nandc_bit_reg op_mode;
    struct nandc_bit_reg page_size;
    struct nandc_bit_reg int_clr;

    struct nandc_bit_reg ecc_result;
    struct nandc_bit_reg nf_status;
    struct nandc_bit_reg errnum0buf0;
    struct nandc_bit_reg rb_status;
    struct nandc_bit_reg oob_len_sel;
};
struct nandc_host;
u32 nandc_ctrl_excute_cmd(struct nandc_host *host);
#endif
