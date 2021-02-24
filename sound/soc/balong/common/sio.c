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
/*lint --e{537}*/
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include "osl_bio.h"
#include "sio.h"
#include "snd_param.h"
#include "bsp_sysctrl.h"

#define    HI_SIO_VERSION_OFFSET                             (0x3C) /* �汾�Ĵ�����������¼SIO�İ汾�źͽ���SIO���Բ��ԡ� */
#define    HI_SIO_MODE_OFFSET                                (0x40) /* ģʽ�Ĵ�����������SIO�����Ļ���ģʽ����ѡ��(I)��ģʽ�£�CRG��ʱ�Ӻ�ͬ���źŸ�CODEC��ͬʱ��SIO��(I)��ģʽ�£�ʱ�Ӻ�ͬ���ź����ⲿCODEC�͸�SIO��I2S��PCM������ģʽѡ����ϵͳ���ƼĴ���SC_PERCTRL6�趨�� */
#define    HI_SIO_INTSTATUS_OFFSET                           (0x44) /* SIO���ж�״ָ̬ʾ�Ĵ��������ڽ����жϣ�������FIFO��������ȴ���FIFO��ֵʱ����һֱ�Ѹߵ�ƽ���浽�ж�״̬�Ĵ����У�һֱ�����жϣ���ʹCPU��һ���жϣ����ж�״̬�Ĵ���������һ��ʱ�������ٴ���λ������ˣ�����CPU�Ĵ�����Ϊ��(S)��SIO_CT_CLR[intr_en]д1���ر�ȫ���ж�ʹ�ܡ�(S)���ж�״̬�Ĵ���SIO_INTSTATUS��(S)�����ж�Դ������Ӧ����(S)��SIO_INTCLR����Ӧλд1������жϡ�(S)��д�Ĵ���SIO_CT_SET[intr_en]д1����ȫ���ж�ʹ�ܡ�(SE)�����жϵĲ�����ʽ������жϲ�����ʽ��ͬ����˶��ڷ����жϵĴ�������Ҳ������ͬ�ķ�ʽ�����Ĵ�����ԭʼ�ж�״̬�Ĵ���������Ӧ�ж�λ���ε�����£��ж���������ʱ����Ӧ�ж�״̬λ��Ȼ����λ�������ᴥ���жϡ� */
#define    HI_SIO_INTCLR_OFFSET                              (0x48) /* �ж�����Ĵ��������԰�λ�����BitֵΪ0��ʾ������жϣ�Ϊ1��ʾ����жϡ� */
#define    HI_SIO_I2S_LEFT_XD_OFFSET                         (0x4C) /* I2Sģʽ�µ����������ݷ��ͼĴ�������Ĵ���д����ʱ����Ч��������ڼĴ����ĵ�bit�������磬���ݿ��Ϊ8bitʱ��bit[7:0]Ϊ��Ч���ݣ�bit[31:8]Ϊ��Ч���ݣ����ݿ��Ϊ16bitʱ��bit[15:0]Ϊ��Ч���ݣ�bit[31:16]Ϊ��Ч���ݡ�������Ч���ݿ�ȵ�bitλ��SIOģ���Զ���0�� */
#define    HI_SIO_I2S_RIGHT_XD_OFFSET                        (0x50) /* I2Sģʽ�µ����������ݷ��ͼĴ�����PCM���ݷ��ͼĴ�����I2S���������ݷ��ͼĴ����Ǹ��õġ���Ĵ���д����ʱ����Ч��������ڼĴ����ĵ�bit�������磬���ݿ��Ϊ8bitʱ��bit[7:0]Ϊ��Ч���ݣ�bit[31:8]Ϊ��Ч���ݣ����ݿ��Ϊ16bitʱ��bit[15:0]Ϊ��Ч���ݣ�bit[31:16]Ϊ��Ч���ݡ�������Ч���ݿ�ȵ�bitλ��SIOģ���Զ���0�� */
#define    HI_SIO_PCM_XD_OFFSET                              (0x50) /* PCMģʽ�µ����ݷ��ͼĴ�����PCM���ݷ��ͼĴ�����I2S���������ݷ��ͼĴ����Ǹ��õġ���Ĵ���д��Ч����ʱ����Ч��������ڼĴ����ĵ�bit�������磬8bit���ʱ��bit[7:0]Ϊ��Ч���ݣ�bit[31:8]Ϊ��Ч���ݡ�16bit���ʱ��bit[15:0]Ϊ��Ч���ݣ�bit[31:16]Ϊ��Ч���ݡ�������Ч���ݿ�ȵ�bitλ��SIOģ���Զ���0�� */
#define    HI_SIO_I2S_LEFT_RD_OFFSET                         (0x54) /* I2S���������ݽ��ռĴ�����SIOģ��ѽ��յ�����Ч���ݷ��ڼĴ����ĵ�bit�������磬���ݿ��Ϊ8bitʱ��bit[7:0]Ϊ��Ч���ݣ�bit[31:8]Ϊ��Ч���ݣ����ݿ��Ϊ16bitʱ��bit[15:0]Ϊ��Ч���ݣ�bit[31:16]Ϊ��Ч���ݡ�������Ч���ݿ�ȵ�bitλ��SIOģ���Զ���0�� */
#define    HI_SIO_I2S_RIGHT_RD_OFFSET                        (0x58) /* I2S���������ݽ��ռĴ�����SIOģ��ѽ��յ�����Ч���ݷ��ڼĴ����ĵ�bit�������磬���ݿ��Ϊ8bitʱ��bit[7:0]Ϊ��Ч���ݣ�bit[31:8]Ϊ��Ч���ݣ����ݿ��Ϊ16bitʱ��bit[15:0]Ϊ��Ч���ݣ�bit[31:16]Ϊ��Ч���ݡ�������Ч���ݿ�ȵ�bitλ��SIOģ���Զ���0�� */
#define    HI_SIO_PCM_RD_OFFSET                              (0x58) /* PCM�������ݼĴ���������I2S���������ռĴ����Ǹ��õġ�SIOģ��ѽ��յ�����Ч���ݷ��ڼĴ����ĵ�bit�������磬���ݿ��Ϊ8bitʱ��bit[7:0]Ϊ��Ч���ݣ�bit[31:8]Ϊ��Ч���ݣ����ݿ��Ϊ16bitʱ��bit[15:0]Ϊ��Ч���ݣ�bit[31:16]Ϊ��Ч���ݡ�������Ч���ݿ�ȵ�bitλ��SIOģ���Զ���0�� */
#define    HI_SIO_CT_SET_OFFSET                              (0x5C) /* ���üĴ�����Ϊ���ܹ�����Ķ�SIO���ƼĴ�������λ��������SIO�У�ΪSIO���ƼĴ���������2����ַ��0x05C��SIO_CT_SET����0x060��SIO_CT_CLR�������У�0x05CΪ���üĴ�����ַ������0x05C�Ĵ�������Ӧλд��1ʱ����Ӧλ����Ϊ1��д0��Ч���üĴ�������Ϊ��д�� */
#define    HI_SIO_CT_CLR_OFFSET                              (0x60) /* ����Ĵ�����Ϊ���ܹ�����Ķ�SIO���ƼĴ�������λ��������SIO�У�ΪSIO���ƼĴ���������2����ַ��0x05C��SIO_CT_SET����0x060��SIO_CT_CLR�������У�0x060Ϊ����Ĵ�����ַ������Ĵ�������Ӧλд��1ʱ����Ӧλ�������д0��Ч���üĴ�������Ϊֻд�� */
#define    HI_SIO_RX_STA_OFFSET                              (0x68) /* SIO����״̬�Ĵ����� */
#define    HI_SIO_TX_STA_OFFSET                              (0x6C) /* SIO����״̬�Ĵ����� */
#define    HI_SIO_DATA_WIDTH_SET_OFFSET                      (0x78) /* I2S/PCMģʽ�µ����ݿ�����üĴ����� */
#define    HI_SIO_I2S_START_POS_OFFSET                       (0x7C) /* I2S����������ʼλ�����ÿ��ƼĴ�������I2Sģʽ�£������������ݲ�����ַ�ϲ�ʹ�ܺ󣬿�����ʼ�����Ǵ���������ʼ���Ǵ���������ʼ�� */
#define    HI_I2S_POS_FLAG_OFFSET                            (0x80) /* I2S��������������ǰλ��״̬�Ĵ�������I2Sģʽ�£������������ݲ�����ַ�ϲ�ʹ�ܺ�ָʾ��һ�η��ʼĴ����Ǵ���������ʼ���Ǵ���������ʼ�� */
#define    HI_SIO_SIGNED_EXT_OFFSET                          (0x84) /* ��λ���ݷ�����չʹ�ܼĴ������ñ�־ֻ�Խ���������Ч���Է���������Ч��PCMģʽ��I2Sģʽ�½��յ������ݶ�֧�ַ�����չ���ڽ�����Ч����λ��Ϊ8/16/18/20/24 bitʱ������ñ�־ʹ�ܣ��ѽ��յ�������ת��Ϊ32bit����ʱ����32bit���ݵĸ�λ��Ч��������Ϊ�������������Чbit��Ӧ��ֵ��Ȼ����д�����FIFO����16bitλ��Ϊ����if (data_rx[15] == 1 ) data_rx[31:16]=0xffff;elsedata_rx[31:16]=0x0000; */
#define    HI_SIO_I2S_POS_MERGE_EN_OFFSET                    (0x88) /* I2Sģʽ�����������ݲ�����ַ�ϲ�ʹ�ܼĴ�������I2Sģʽ�£���DMA��ʽ��дSIO��FIFO����ʱ����Ϊ�����������ݵ�ַ��ͬ����ҪCPU��������DMA�����ĵ�ַ������CPUЧ�ʵ͡�Ϊ�����CPU��Ч�ʣ��ṩ�����������ݵĶ�д��ͳһ��ַʹ�ܿ��ơ�ʹ������£���������������ͳһʹ��SIO_I2S_DUAL_RX_CHN�Ĵ�����д������������ͳһʹ��SIO_I2S_DUAL_TX_CHN�Ĵ����� */
#define    HI_SIO_INTMASK_OFFSET                             (0x8C) /* �ж����μĴ����� */
#define    HI_SIO_I2S_DUAL_RX_CHN_OFFSET                     (0xA0) /* ��ȡ�������ݼĴ�����I2S��������������ַ�ϲ�ʹ�ܺ󣬶�ȡ�������ݡ� */
#define    HI_SIO_I2S_DUAL_TX_CHN_OFFSET                     (0xC0) /* д�������ݼĴ�����I2S��������������ַ�ϲ�ʹ�ܺ�д�������ݡ� */

enum
{
    SIO_SC_MASTER = 0,
    SIO_SC_SIOZSI,
    SIO_CNT,
    SIO_PROP_MAX,
};

#define SIO_CLK_NUM 2

struct sio_sc_st
{
    unsigned char *addr;
    unsigned int start;
    unsigned int end;

};

struct sio_data_st
{
    unsigned int init;
    unsigned char *base_addr;
    unsigned int clk_enable;
    struct clk *clk;
    struct sio_sc_st sc[SIO_PROP_MAX];

};

static void snd_sc_write(void* addr, unsigned start, unsigned end, unsigned val)
{
    unsigned tmp = readl(addr);
    tmp &= (~(((1<<(end - start + 1))-1)<<start));
    tmp |= (val<<start);
    writel(tmp, addr);
}

/*static unsigned int snd_sc_read(void* addr, unsigned start, unsigned end)
{
    unsigned tmp = readl(addr);
    return ((tmp >> start)&((1<<(end - start + 1))-1));
}*/


void* sio_dts_para_get(void)
{
    int ret = 0;
    unsigned int i = 0;
    unsigned int para[3] = {0};

    unsigned char *addr = NULL;
    struct device_node *dev = NULL;
    const char name[20] = "hisilicon,sio_app";
    const char *prop[SIO_PROP_MAX] = {"master_mode","sio_zsi","sio_cnt"};

    static struct sio_data_st sio_data = {
        .init = 0,
        .clk_enable = 0,
        .base_addr = NULL,
    };

    if(1 == sio_data.init)
    {
        return &sio_data;
    }

    dev = of_find_compatible_node(NULL, NULL, name);
    if(NULL == dev)
    {
        printk(KERN_ERR "sio device node not found\n");
        return NULL;
    }

    addr = (unsigned char*)of_iomap(dev, 0);
    if (NULL == addr)
    {
        printk(KERN_ERR "sio remap addr fail\n");
        return NULL;
    }

    sio_data.base_addr = addr;

    for(i=0;i < SIO_PROP_MAX;i++)
    {
        ret = of_property_read_u32_array(dev, prop[i], (u32*)para, 3);
        if (ret < 0)
        {
            printk(KERN_ERR "sio %s read error, ret %d.\n", prop[i], ret);
            return NULL;
        }

        addr = bsp_sysctrl_addr_get((void*)para[0]);
        if(NULL == addr)
        {
            printk(KERN_ERR "sio %s sysctrl addr get fail.\n", prop[i]);
            return NULL;
        }

        sio_data.sc[i].addr   = addr;
        sio_data.sc[i].start  = para[1];
        sio_data.sc[i].end    = para[2];

        printk(KERN_INFO "sio addr 0x%x, para 0x%x 0x%x 0x%x.\n", (unsigned)sio_data.base_addr,
                    (unsigned)sio_data.sc[i].addr, sio_data.sc[i].start, sio_data.sc[i].end);
    }


    sio_data.clk = clk_get(NULL, "sio_clk");
    if (IS_ERR(sio_data.clk))
    {
        printk(KERN_ERR "sio_clk get fail.\n");
        return NULL;
    }


    sio_data.init = 1;

    return &sio_data;

}

void sio_init(void)
{
    struct sio_data_st *sio_data = sio_dts_para_get();
    if(NULL == sio_data)
    {
        printk(KERN_ERR "sio dts get fail.\n");
        return ;
    }

    /* ��SIO����ʱ���ſ� */
    sio_clock_enable();

    /*дSIO_INTR_CLR 0xffff��λSIO����ֹ���䡢��ֹ�ж�*/
    writel(0xffff, sio_data->base_addr + HI_SIO_CT_CLR_OFFSET);

    /*��������SIO�ж�*/
    writel(0xffffffff, sio_data->base_addr + HI_SIO_INTMASK_OFFSET);

    /*�������SIO�ж�*/
    writel(0x3f, sio_data->base_addr + HI_SIO_INTMASK_OFFSET);

    /*�⸴λ������ˮ��Tx-8(0.5)��RX-8(0.5)��ʹ���ж�*/
    writel(0x8088, sio_data->base_addr + HI_SIO_CT_SET_OFFSET);

    /*����Ϊ���͡�����16bitλ��*/
    writel(0x9, sio_data->base_addr + HI_SIO_DATA_WIDTH_SET_OFFSET);

    /*���÷�����չ��ֹ*/
    writel(0, sio_data->base_addr + HI_SIO_SIGNED_EXT_OFFSET);

    if(SND_PCM_MODE == snd_data_mode_get())
    {
        /*����ΪPCMģʽ*/
        writel(0x1, sio_data->base_addr + HI_SIO_MODE_OFFSET);
        /*ʹ��SIO���ա�����*/
        writel(0x3000, sio_data->base_addr + HI_SIO_CT_SET_OFFSET);
    }
    else
    {
        /*����ΪI2Sģʽ*/
        writel(0x0, sio_data->base_addr + HI_SIO_MODE_OFFSET);

        /*����ͨ���ϲ�ʹ��*/
        writel(0x1, sio_data->base_addr + HI_SIO_I2S_POS_MERGE_EN_OFFSET);

        /*����ͨ����ʼ����*/
        writel(0x0, sio_data->base_addr + HI_SIO_I2S_START_POS_OFFSET);

        /*ʹ��SIO���ա�����*/
        writel(0x3300, sio_data->base_addr + HI_SIO_CT_SET_OFFSET);
    }

}


void sio_pcm_mode_set(void)
{
    struct sio_data_st *sio_data = sio_dts_para_get();
    if(NULL == sio_data)
    {
        printk(KERN_ERR "sio dts get fail.\n");
        return ;
    }
    /*����ΪPCMģʽ*/
    writel(0x1, sio_data->base_addr + HI_SIO_MODE_OFFSET);

}

void sio_clock_enable(void)
{
    struct sio_data_st *sio_data = sio_dts_para_get();

    if(1 == sio_data->clk_enable)
        return;

    if(clk_prepare_enable(sio_data->clk))
    {
        printk(KERN_ERR "sio clk prepare enable fail.\n");
        return ;
    }

    sio_data->clk_enable = 1;
}

void sio_clock_disable(void)
{
    struct sio_data_st *sio_data = sio_dts_para_get();
    if(0 == sio_data->clk_enable)
        return;

    (void)clk_disable_unprepare(sio_data->clk);
    sio_data->clk_enable = 0;
}

void sio_reset(void)
{
    struct sio_data_st *sio_data = sio_dts_para_get();
    if(NULL == sio_data)
    {
        printk(KERN_ERR "sio dts get fail.\n");
        return ;
    }
    
    /*дSIO_INTR_CLR 0xffff��λSIO����ֹ���䡢��ֹ�ж�*/
    writel(0xffff, sio_data->base_addr + HI_SIO_CT_CLR_OFFSET);

    /*��������SIO�ж�*/
    writel(0xffffffff, sio_data->base_addr + HI_SIO_INTMASK_OFFSET);

    /*�������SIO�ж�*/
    writel(0x3f, sio_data->base_addr + HI_SIO_INTMASK_OFFSET);

}


void sio_pcm_div(unsigned int p, unsigned int fs)
{
    unsigned int i = 0;
    struct clk *sio_clk = NULL;
    unsigned int div[] = {p,fs};
    struct sio_data_st *sio_data = NULL;
    const char *clk_name[SIO_CLK_NUM] = {"sio_bclk","sio_syncclk"};
    sio_data = sio_dts_para_get();
    if(NULL == sio_data)
    {
        printk(KERN_ERR "sio dts get fail.\n");
        return ;
    }

    sio_clk = clk_get(NULL, "sio_zsi_refclk");
    if (IS_ERR(sio_clk))
    {
        printk(KERN_ERR "%s get fail.\n", clk_name[i]);
        return;
    }

    if(clk_prepare_enable(sio_clk))
    {
        printk(KERN_ERR "%s prepare enable fail.\n", clk_name[i]);
        return;
    }
    for(i = 0; i < SIO_CLK_NUM; i++)
    {
        sio_clk = clk_get(NULL, clk_name[i]);
        if (IS_ERR(sio_clk))
        {
            printk(KERN_ERR "%s get fail.\n", clk_name[i]);
            return;
        }

        if (clk_set_rate(sio_clk, div[i]))
        {
            printk(KERN_ERR "%s set div(0x%x) fail.\n", clk_name[i], div[i]);
            return;
        }
    }

    /* �üĴ���λ��CRG�У���huangxiankeȷ��clk����û�����ƹ��ܣ����������� */
    if(SND_SAMPLE_RATE_WIDE_8K == snd_sample_rate_get())
    {
            snd_sc_write((void*)sio_data->sc[SIO_CNT].addr, sio_data->sc[SIO_CNT].start,
                        sio_data->sc[SIO_CNT].end, 513); /* pclk / fs / 2 + 1 */

    }

}


void sio_master_set(unsigned int mode)
{
    struct sio_data_st *sio_data = NULL;

    if(SND_SOC_MASTER != mode && SND_SOC_SLAVE != mode )
    {
        printk(KERN_ERR "sio mode error, mode =0x%x.\n", mode);
        return ;
    }

    sio_data = sio_dts_para_get();
    if(NULL == sio_data)
    {
        printk(KERN_ERR "sio dts get fail.\n");
        return ;
    }

    /*����ΪMASTER��SLAVEģʽ*/
    snd_sc_write((void*)sio_data->sc[SIO_SC_MASTER].addr, sio_data->sc[SIO_SC_MASTER].start,
                sio_data->sc[SIO_SC_MASTER].end, SND_SOC_MASTER == mode ? 1 : 0);

}


void sio_zsi_set(unsigned int mode)
{
    struct sio_data_st *sio_data = NULL;

    if(SIO_MODE != mode && ZSI_MODE != mode )
    {
        printk(KERN_ERR "sio mode error, mode =0x%x.\n", mode);
        return ;
    }

    sio_data = sio_dts_para_get();
    if(NULL == sio_data)
    {
        printk(KERN_ERR "sio dts get fail.\n");
        return ;
    }

    /*����ΪSIO��ZSIģʽ*/
    snd_sc_write((void*)sio_data->sc[SIO_SC_SIOZSI].addr, sio_data->sc[SIO_SC_SIOZSI].start, sio_data->sc[SIO_SC_SIOZSI].end, mode);

}




