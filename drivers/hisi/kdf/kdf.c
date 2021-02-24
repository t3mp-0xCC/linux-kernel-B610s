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

#include <linux/gfp.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/dma-mapping.h>
#include <asm/io.h>
#include "kdf.h"

/*lint --e{550} */

int kdf_start_channel(void);
void kdf_move_bdq_w_pos(void);
void kdf_move_rdq_r_pos(void);

static struct kdf_s ch4_info = {0, 0, 0, 0, 0, 0, BD_RD_NUM, 0, 0, 0, 0, 0, NULL};

/*****************************************************************************
* �� �� ��  : kdf_get_base_addr
*
* ��������  : API for kdf test code get kdf base address
*
* �������  : ��
* �������  : ��
* �� �� ֵ  : ��
*****************************************************************************/
unsigned long kdf_get_base_addr(void)
{
	if (!ch4_info.reg_virt_addr)
		return (unsigned long)CIPHER_NULL_PTR;
	else
		return (unsigned long)ch4_info.reg_virt_addr;
}
EXPORT_SYMBOL(kdf_get_base_addr);

/*****************************************************************************
* �� �� ��  : kdf_reg_init
*
* ��������  : A��KDF�Ĵ������ã����ڼ���HASHֵ����C������֮ǰʹ��
*
* �������  : ��
* �������  : ��
* �� �� ֵ  : ��
*****************************************************************************/
void kdf_reg_init(void)
{
	u32 reg_val = 0;
	int i = 0;

	/*��λch4*/
	writel(0x7, (void *)(ch4_info.reg_virt_addr + HI_CH4_SOFTRESET_OFFSET));
	i = 1000;
	do
	{
		reg_val = readl((void *)(ch4_info.reg_virt_addr + HI_CH4_SOFTRESET_OFFSET));
		i--;
	}while((0 != (reg_val & 0x7)) && (i > 0));
	if(i <= 0)
	{
		return;
	}

	/*����BD�Ĵ���*/
	writel(ch4_info.bdq_real_addr, (void *)(ch4_info.reg_virt_addr + HI_CH4_BDQ_BADDR_OFFSET));
	writel(BD_RD_NUM - 1, (void *)(ch4_info.reg_virt_addr + HI_CH4_BDQ_SIZE_OFFSET));

	/*����RD�Ĵ���*/
	writel(ch4_info.rdq_real_addr, (void *)(ch4_info.reg_virt_addr + HI_CH4_RDQ_BADDR_OFFSET));
	writel(BD_RD_NUM - 1, (void *)(ch4_info.reg_virt_addr + HI_CH4_RDQ_SIZE_OFFSET));

    /*ʹ��KDFͨ��*/
	writel(0x1, (void *)(ch4_info.reg_virt_addr + HI_CH4_EN_OFFSET));

    /* ����KDFͨ��config�Ĵ���*/
    reg_val= CHN_IV_SEL| (CHN_RDQ_CTRL << CHN_RDQCTRL_BIT)| KDF_CHN_USRFIELD_LEN << CHN_USRFIELDLEN_BIT;/* [false alarm]:�� */
	writel(reg_val, (void *)(ch4_info.reg_virt_addr + HI_CH4_CONFIG_OFFSET));

    /* KeyRam�е���������Ϊ0��ֻʹ����0������λ��*/
    for(i = 0; i < KDF_SHA_KEY_LENGTH; i += 4)
    {
        writel(0, (volatile void *)(ch4_info.reg_virt_addr + HI_KEY_RAM_OFFSET + KEY_INDEX * CIPHER_KEY_LEN + i));/*lint !e737*/
    }

}

/*****************************************************************************
* �� �� ��  : kdf_init
*
* ��������  : A��KDF���пռ������ʼ�������ڼ���HASHֵ����C������֮ǰʹ��
*
* �������  : ��
* �������  : ��
* �� �� ֵ  : 0    : ��ȷ
*             ���� : ����
*****************************************************************************/
int kdf_init(void)
{
	/*����RDQ��BDQ�ռ�*/
	ch4_info.buffer_virt_addr =
		(u32)dma_alloc_coherent(NULL, RDQ_SIZE + BDQ_SIZE + KDF_SHA_S_LENGTH_MAX + CIPHER_ALIGN_LEN, &(ch4_info.buffer_real_addr), GFP_KERNEL);
	if(!ch4_info.buffer_virt_addr || !ch4_info.buffer_real_addr)
	{
		dma_free_coherent(NULL, RDQ_SIZE + BDQ_SIZE + KDF_SHA_S_LENGTH_MAX + CIPHER_ALIGN_LEN, (void*)ch4_info.buffer_virt_addr, ch4_info.buffer_real_addr);
		return CIPHER_NULL_PTR;
	}

	ch4_info.bdq_virt_addr = (ch4_info.buffer_virt_addr + CIPHER_ALIGN_LEN) & ~CIPHER_ALIGN_LEN;
	ch4_info.bdq_real_addr = (ch4_info.buffer_real_addr + CIPHER_ALIGN_LEN) & ~CIPHER_ALIGN_LEN;
	ch4_info.rdq_virt_addr = ch4_info.bdq_virt_addr + BDQ_SIZE;
	ch4_info.rdq_real_addr = ch4_info.bdq_real_addr + BDQ_SIZE;
	ch4_info.sha_s_buff_virt_addr = ch4_info.rdq_virt_addr + RDQ_SIZE;
	ch4_info.sha_s_buff_real_addr = ch4_info.rdq_real_addr + RDQ_SIZE;
	ch4_info.cipher_clk = clk_get(NULL, "cipher_clk");
	if(IS_ERR(ch4_info.cipher_clk))
	{
		return CIPHER_NULL_PTR;
	}

	memset((void*)ch4_info.bdq_virt_addr, 0, BDQ_SIZE);
	memset((void*)ch4_info.rdq_virt_addr, 0, RDQ_SIZE);
	memset((void*)ch4_info.sha_s_buff_virt_addr, 0, KDF_SHA_S_LENGTH_MAX);

    return CIPHER_SUCCESS;
}

int kdf_bd_configure(KDF_BDCONFIG_INFO_S *stKeyCfgInfo)
{
    KDF_BD_SCPT_S *p_w_addr = (KDF_BD_SCPT_S *)(ch4_info.bdq_virt_addr);
	u32 reg_val_bdq_ptr;

	reg_val_bdq_ptr = readl((void *)(ch4_info.reg_virt_addr + HI_CH4_BDQ_PTR_OFFSET));
	reg_val_bdq_ptr = reg_val_bdq_ptr & 0x3FF;
	p_w_addr += reg_val_bdq_ptr;

    p_w_addr->u32KdfConfig= 0                                           |
		                    (stKeyCfgInfo->enShaKeySource & 0x3) << 0x2 |
                            (stKeyCfgInfo->enShaSSource & 0x1)   << 0x4 |
                            (stKeyCfgInfo->u32ShaKeyIndex & 0xf) << 0x7 |
                            (stKeyCfgInfo->u32ShaSIndex & 0xf)   << 0xb |
                            (stKeyCfgInfo->u32Length & 0x1ff)    << 0xf |
                            (stKeyCfgInfo->u32DestIndex & 0xf)   << 0x18;

    p_w_addr->u32Address = (u32)(stKeyCfgInfo->pAddress);

    return CIPHER_SUCCESS;
}

void kdf_move_rdq_r_pos(void)
{
	u32 reg_val_rdq_rwptr;
	u32 reg_val_rdq_size;

	reg_val_rdq_rwptr = readl((void *)(ch4_info.reg_virt_addr + HI_CH4_RDQ_PTR_OFFSET));
	reg_val_rdq_size  = readl((void *)(ch4_info.reg_virt_addr + HI_CH4_RDQ_SIZE_OFFSET));
	reg_val_rdq_rwptr = (reg_val_rdq_rwptr & 0x3FF0000);
	reg_val_rdq_rwptr = reg_val_rdq_rwptr >> 16;
	reg_val_rdq_rwptr += 1;
	reg_val_rdq_size++;
	reg_val_rdq_rwptr = reg_val_rdq_rwptr % reg_val_rdq_size;
	/*reg_val_rdq_rwptr = ((reg_val_rdq_rwptr & 0x3FF0000) >> 16 + 1) % (1 + reg_val_rdq_size);*/
	reg_val_rdq_rwptr = reg_val_rdq_rwptr << 16;/* [false alarm]:�� */
	writel(reg_val_rdq_rwptr, (void *)(ch4_info.reg_virt_addr + HI_CH4_RDQ_PTR_OFFSET));
}

void kdf_move_bdq_w_pos(void)
{
	u32 reg_val_bdq_rwptr;
	u32 reg_val_bdq_size;

	reg_val_bdq_rwptr = readl((void *)(ch4_info.reg_virt_addr + HI_CH4_BDQ_PTR_OFFSET));
	reg_val_bdq_size  = readl((void *)(ch4_info.reg_virt_addr + HI_CH4_BDQ_SIZE_OFFSET));
	reg_val_bdq_rwptr = reg_val_bdq_rwptr & 0x3FF;
	reg_val_bdq_rwptr += 1;
	reg_val_bdq_size++;
	reg_val_bdq_rwptr = reg_val_bdq_rwptr % reg_val_bdq_size;/* [false alarm]:�� */
	/*reg_val_bdq_rwptr = (reg_val_bdq_rwptr & 0x3FF + 1) % (1 + reg_val_bdq_size);*/
	writel(reg_val_bdq_rwptr, (void *)(ch4_info.reg_virt_addr + HI_CH4_BDQ_PTR_OFFSET));
}

int kdf_start_channel(void)
{
    u32 reg_val;
    KDF_RD_SCPT_S *pCurRDAddr = NULL;
	s32 delay_loops = 20000;/*ѭ��20000�Σ�ÿ���ӳ�1us�����ӳ�20000us*/

	/* �ƶ��Ĵ���дָ�� */
	kdf_move_bdq_w_pos();

	do
    {
		reg_val = readl((void *)(ch4_info.reg_virt_addr + HI_CH4_EN_OFFSET));
        delay_loops = delay_loops - 1;
		/*delay(1);˯1us*/
    }
    while((reg_val & 0x80000000) && (delay_loops > 0));
	if(delay_loops <= 0)
	{
		return CIPHER_TIME_OUT;
	}

    /* ��ȡ��ǰ��RD������*/ 
	reg_val = readl((volatile void *)(ch4_info.reg_virt_addr + HI_CH4_RDQ_PTR_OFFSET));
	reg_val = (reg_val & 0x3FF0000) >> 16;
    pCurRDAddr = (KDF_RD_SCPT_S *)(ch4_info.rdq_virt_addr) + reg_val;
    /* ���RD��Чλ*/
    pCurRDAddr->u32KdfConfig &= (~0x80000000);
    /* �ж��Ƿ��������Լ�����*/
    if(CIPHER_STAT_CHECK_ERR == ((pCurRDAddr->u32KdfConfig >> 29) & 0x3))
    {
        return CIPHER_CHECK_ERROR;
    }
	/* �ƶ��Ĵ����е�RD����ַ*/
	kdf_move_rdq_r_pos();

    return CIPHER_SUCCESS;
}

int kdf_hash_make(void * sha_s_addr, u32 sha_s_len)
{
    KDF_BDCONFIG_INFO_S stBDCfgInfo;
	u32 i = 0;

    stBDCfgInfo.enShaKeySource  = SHA_KEY_SOURCE_KEYRAM;           /*sha_key����ȥkey ram��ȡ*/ 
    stBDCfgInfo.u32ShaKeyIndex  = KEY_INDEX;                       /*sha_s��key ram�е�λ��*/
    stBDCfgInfo.u32ShaSIndex    = 0;                               /*sha_s������Դ��DDR������λ������ν*/
    stBDCfgInfo.u32Length       = sha_s_len - 1;                   /*Ӧ�ü�1���߼����1*/
    stBDCfgInfo.u32DestIndex    = KEY_INDEX;                       /*���ɵ�HASH����̶�����key ram�ĵ�15��λ��*/
    stBDCfgInfo.pAddress        = (void*)(ch4_info.sha_s_buff_real_addr);   /*sha_s�ڴ��ַ*/
	stBDCfgInfo.enShaSSource    = SHA_S_SOURCE_DDR;                /*sha_s������Դ��DDR*/

	for(i = 0; i < sha_s_len; i++)
	{
		*((u8 *)(ch4_info.sha_s_buff_virt_addr) + i) = *((u8*)sha_s_addr + i);
	}
    kdf_bd_configure(&stBDCfgInfo);
    return kdf_start_channel();
}
EXPORT_SYMBOL(kdf_hash_make);

int kdf_hash_setkey(void * key_addr)
{
	int i = 0;
    u32 val = 0;
	
	if(!key_addr)
	{
		return CIPHER_NULL_PTR;
	}
	for(i = 0; i < CIPHER_KEY_LEN; i += 4)
	{
        val = (u32)readl((volatile void *)((char*)key_addr + i));
		writel(val, (volatile void *)(ch4_info.reg_virt_addr + HI_KEY_RAM_OFFSET + KEY_INDEX * CIPHER_KEY_LEN + i));/*lint !e737*/
	}
	return 0;
}

int kdf_result_hash_get(void * out_hash)
{
	int i = 0;
    u32 val = 0;
	
	if(!out_hash)
		return CIPHER_NULL_PTR;

    for(i = 0; i < CIPHER_KEY_LEN; i += 4)
    {
        val = readl((volatile void *)(ch4_info.reg_virt_addr + HI_KEY_RAM_OFFSET + KEY_INDEX * CIPHER_KEY_LEN + i));/*lint !e737*/
        writel(val, (volatile void *)((char*)out_hash + i));
    }
	return 0;
}

int kdf_hash_init(void)
{
	static const char * dts_node_name = "hisilicon,cipher";
	struct device_node * dts_node = NULL;

	dts_node = of_find_compatible_node(NULL, NULL, dts_node_name);
	if(!dts_node)
		return -1;
	ch4_info.reg_virt_addr = (u32)of_iomap(dts_node, 0);
	if(!ch4_info.reg_virt_addr)
		return -1;
	of_property_read_u32_array(dts_node, "clkrate", &ch4_info.clk_rate, 1);
	
	if(kdf_init())
		return -1;
	if(clk_set_rate(ch4_info.cipher_clk, ch4_info.clk_rate))
		return -1;
	if(clk_prepare(ch4_info.cipher_clk))
		return -1;
	if(clk_enable(ch4_info.cipher_clk))
	{
		return -1;
	}
	kdf_reg_init();
	return 0;
}
EXPORT_SYMBOL(kdf_hash_init);

void kdf_hash_exit(void)
{
	clk_disable(ch4_info.cipher_clk);
	dma_free_coherent(NULL,RDQ_SIZE + BDQ_SIZE + KDF_SHA_S_LENGTH_MAX + CIPHER_ALIGN_LEN,(void*)ch4_info.buffer_virt_addr,ch4_info.buffer_real_addr);
}

EXPORT_SYMBOL(kdf_hash_exit);

