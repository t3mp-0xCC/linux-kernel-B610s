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

#define    HI_SIO_VERSION_OFFSET                             (0x3C) /* 版本寄存器，用来记录SIO的版本号和进行SIO的自测试。 */
#define    HI_SIO_MODE_OFFSET                                (0x40) /* 模式寄存器，用来对SIO工作的基本模式进行选择：(I)主模式下，CRG送时钟和同步信号给CODEC，同时给SIO。(I)从模式下，时钟和同步信号由外部CODEC送给SIO。I2S或PCM的主从模式选择由系统控制寄存器SC_PERCTRL6设定。 */
#define    HI_SIO_INTSTATUS_OFFSET                           (0x44) /* SIO的中断状态指示寄存器。对于接收中断，当接收FIFO的数据深度大于FIFO阈值时，会一直把高电平锁存到中断状态寄存器中，一直产生中断（即使CPU清一次中断，但中断状态寄存器会在下一个时钟周期再次置位）。因此，建议CPU的处理步骤为：(S)向SIO_CT_CLR[intr_en]写1，关闭全局中断使能。(S)读中断状态寄存器SIO_INTSTATUS。(S)根据中断源进行相应处理。(S)向SIO_INTCLR的相应位写1，清除中断。(S)向写寄存器SIO_CT_SET[intr_en]写1，打开全局中断使能。(SE)发送中断的产生方式与接收中断产生方式相同，因此对于发送中断的处理，建议也采用相同的方式。本寄存器是原始中断状态寄存器。在相应中断位屏蔽的情况下，中断条件满足时，对应中断状态位仍然会置位，但不会触发中断。 */
#define    HI_SIO_INTCLR_OFFSET                              (0x48) /* 中断清除寄存器，可以按位清除。Bit值为0表示不清除中断，为1表示清除中断。 */
#define    HI_SIO_I2S_LEFT_XD_OFFSET                         (0x4C) /* I2S模式下的左声道数据发送寄存器。向寄存器写数据时，有效数据需放在寄存器的低bit区域。例如，数据宽度为8bit时，bit[7:0]为有效数据，bit[31:8]为无效数据；数据宽度为16bit时，bit[15:0]为有效数据，bit[31:16]为无效数据。超出有效数据宽度的bit位由SIO模块自动置0。 */
#define    HI_SIO_I2S_RIGHT_XD_OFFSET                        (0x50) /* I2S模式下的右声道数据发送寄存器。PCM数据发送寄存器与I2S右声道数据发送寄存器是复用的。向寄存器写数据时，有效数据需放在寄存器的低bit区域。例如，数据宽度为8bit时，bit[7:0]为有效数据，bit[31:8]为无效数据；数据宽度为16bit时，bit[15:0]为有效数据，bit[31:16]为无效数据。超出有效数据宽度的bit位由SIO模块自动置0。 */
#define    HI_SIO_PCM_XD_OFFSET                              (0x50) /* PCM模式下的数据发送寄存器。PCM数据发送寄存器与I2S右声道数据发送寄存器是复用的。向寄存器写有效数据时，有效数据需放在寄存器的低bit区域。例如，8bit宽度时，bit[7:0]为有效数据，bit[31:8]为无效数据。16bit宽度时，bit[15:0]为有效数据，bit[31:16]为无效数据。超出有效数据宽度的bit位由SIO模块自动置0。 */
#define    HI_SIO_I2S_LEFT_RD_OFFSET                         (0x54) /* I2S左声道数据接收寄存器。SIO模块把接收到的有效数据放在寄存器的低bit区域。例如，数据宽度为8bit时，bit[7:0]为有效数据，bit[31:8]为无效数据；数据宽度为16bit时，bit[15:0]为有效数据，bit[31:16]为无效数据。超出有效数据宽度的bit位由SIO模块自动置0。 */
#define    HI_SIO_I2S_RIGHT_RD_OFFSET                        (0x58) /* I2S右声道数据接收寄存器。SIO模块把接收到的有效数据放在寄存器的低bit区域。例如，数据宽度为8bit时，bit[7:0]为有效数据，bit[31:8]为无效数据；数据宽度为16bit时，bit[15:0]为有效数据，bit[31:16]为无效数据。超出有效数据宽度的bit位由SIO模块自动置0。 */
#define    HI_SIO_PCM_RD_OFFSET                              (0x58) /* PCM接收数据寄存器。它与I2S右声道接收寄存器是复用的。SIO模块把接收到的有效数据放在寄存器的低bit区域。例如，数据宽度为8bit时，bit[7:0]为有效数据，bit[31:8]为无效数据；数据宽度为16bit时，bit[15:0]为有效数据，bit[31:16]为无效数据。超出有效数据宽度的bit位由SIO模块自动置0。 */
#define    HI_SIO_CT_SET_OFFSET                              (0x5C) /* 设置寄存器。为了能够方便的对SIO控制寄存器进行位操作，在SIO中，为SIO控制寄存器设置了2个地址：0x05C（SIO_CT_SET）和0x060（SIO_CT_CLR），其中：0x05C为设置寄存器地址，当向0x05C寄存器中相应位写入1时，对应位被设为1，写0无效；该寄存器属性为读写。 */
#define    HI_SIO_CT_CLR_OFFSET                              (0x60) /* 清除寄存器。为了能够方便的对SIO控制寄存器进行位操作，在SIO中，为SIO控制寄存器设置了2个地址：0x05C（SIO_CT_SET）和0x060（SIO_CT_CLR），其中：0x060为清除寄存器地址，当向寄存器中相应位写入1时，对应位被清除，写0无效。该寄存器属性为只写。 */
#define    HI_SIO_RX_STA_OFFSET                              (0x68) /* SIO接收状态寄存器。 */
#define    HI_SIO_TX_STA_OFFSET                              (0x6C) /* SIO发送状态寄存器。 */
#define    HI_SIO_DATA_WIDTH_SET_OFFSET                      (0x78) /* I2S/PCM模式下的数据宽度配置寄存器。 */
#define    HI_SIO_I2S_START_POS_OFFSET                       (0x7C) /* I2S左右声道起始位置配置控制寄存器。在I2S模式下，左右声道数据操作地址合并使能后，控制起始访问是从左声道开始还是从右声道开始。 */
#define    HI_I2S_POS_FLAG_OFFSET                            (0x80) /* I2S左右声道操作当前位置状态寄存器。在I2S模式下，左右声道数据操作地址合并使能后，指示下一次访问寄存器是从左声道开始还是从右声道开始。 */
#define    HI_SIO_SIGNED_EXT_OFFSET                          (0x84) /* 高位数据符号扩展使能寄存器。该标志只对接收数据有效，对发送数据无效。PCM模式和I2S模式下接收到的数据都支持符号扩展。在接收有效数据位宽为8/16/18/20/24 bit时，如果该标志使能，把接收到的数据转换为32bit数据时，把32bit数据的高位无效比特设置为接收数据最高有效bit对应的值，然后再写入接收FIFO。以16bit位宽为例：if (data_rx[15] == 1 ) data_rx[31:16]=0xffff;elsedata_rx[31:16]=0x0000; */
#define    HI_SIO_I2S_POS_MERGE_EN_OFFSET                    (0x88) /* I2S模式左右声道数据操作地址合并使能寄存器。在I2S模式下，用DMA方式读写SIO的FIFO数据时，因为左右声道数据地址不同，需要CPU不断配置DMA操作的地址，导致CPU效率低。为了提高CPU的效率，提供左右声道数据的读写的统一地址使能控制。使能情况下，读左右声道数据统一使用SIO_I2S_DUAL_RX_CHN寄存器，写左右声道数据统一使用SIO_I2S_DUAL_TX_CHN寄存器。 */
#define    HI_SIO_INTMASK_OFFSET                             (0x8C) /* 中断屏蔽寄存器。 */
#define    HI_SIO_I2S_DUAL_RX_CHN_OFFSET                     (0xA0) /* 读取接收数据寄存器。I2S左右声道操作地址合并使能后，读取接收数据。 */
#define    HI_SIO_I2S_DUAL_TX_CHN_OFFSET                     (0xC0) /* 写发送数据寄存器。I2S左右声道操作地址合并使能后，写发送数据。 */

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

    /* 打开SIO工作时钟门控 */
    sio_clock_enable();

    /*写SIO_INTR_CLR 0xffff复位SIO、禁止传输、禁止中断*/
    writel(0xffff, sio_data->base_addr + HI_SIO_CT_CLR_OFFSET);

    /*屏蔽所有SIO中断*/
    writel(0xffffffff, sio_data->base_addr + HI_SIO_INTMASK_OFFSET);

    /*清除所有SIO中断*/
    writel(0x3f, sio_data->base_addr + HI_SIO_INTMASK_OFFSET);

    /*解复位、设置水线Tx-8(0.5)、RX-8(0.5)、使能中断*/
    writel(0x8088, sio_data->base_addr + HI_SIO_CT_SET_OFFSET);

    /*配置为发送、接收16bit位宽*/
    writel(0x9, sio_data->base_addr + HI_SIO_DATA_WIDTH_SET_OFFSET);

    /*配置符号扩展禁止*/
    writel(0, sio_data->base_addr + HI_SIO_SIGNED_EXT_OFFSET);

    if(SND_PCM_MODE == snd_data_mode_get())
    {
        /*配置为PCM模式*/
        writel(0x1, sio_data->base_addr + HI_SIO_MODE_OFFSET);
        /*使能SIO接收、发送*/
        writel(0x3000, sio_data->base_addr + HI_SIO_CT_SET_OFFSET);
    }
    else
    {
        /*配置为I2S模式*/
        writel(0x0, sio_data->base_addr + HI_SIO_MODE_OFFSET);

        /*左右通道合并使能*/
        writel(0x1, sio_data->base_addr + HI_SIO_I2S_POS_MERGE_EN_OFFSET);

        /*从左通道开始访问*/
        writel(0x0, sio_data->base_addr + HI_SIO_I2S_START_POS_OFFSET);

        /*使能SIO接收、发送*/
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
    /*配置为PCM模式*/
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
    
    /*写SIO_INTR_CLR 0xffff复位SIO、禁止传输、禁止中断*/
    writel(0xffff, sio_data->base_addr + HI_SIO_CT_CLR_OFFSET);

    /*屏蔽所有SIO中断*/
    writel(0xffffffff, sio_data->base_addr + HI_SIO_INTMASK_OFFSET);

    /*清除所有SIO中断*/
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

    /* 该寄存器位于CRG中，与huangxianke确认clk里面没有类似功能，故自行配置 */
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

    /*配置为MASTER或SLAVE模式*/
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

    /*配置为SIO或ZSI模式*/
    snd_sc_write((void*)sio_data->sc[SIO_SC_SIOZSI].addr, sio_data->sc[SIO_SC_SIOZSI].start, sio_data->sc[SIO_SC_SIOZSI].end, mode);

}




