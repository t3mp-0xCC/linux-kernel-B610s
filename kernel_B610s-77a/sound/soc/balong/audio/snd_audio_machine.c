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


#include <linux/module.h>
#include <linux/string.h>
#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <asm/mach-types.h>
#include <asm/mach-types.h>
#include <../../codecs/wm8990.h>

#include "snd_param.h"
#include "snd_audio_machine.h"
#include "sio.h"

#define AUDIO_HEADPHONE_TEST
#define AUDIO_MCLK_FREG   19200000
#define AUDIO_SYSCLK_FREG 12288000

#define MACHINE_CODEC_NAME_LENGTH 20

static int balong_audio_late_init(struct snd_soc_pcm_runtime *rtd);

/* LE9662 BALONG DAI operations */
static struct snd_soc_ops balong_snd_ops = {
    .hw_params = NULL,
    //.startup = snd_balong_dai_startup,
};

char machine_codec_name[MACHINE_CODEC_NAME_LENGTH] = "wm8990.0-001a";

static struct snd_soc_dai_link snd_machine_dai[] = {
    [0]={ /* Sound Playback */
        .name = "balong audio",
        .stream_name = "playback_capture",
        .cpu_dai_name = "pcm_dai",
        .codec_dai_name = "wm8990-hifi",
        .platform_name = "snd_plat",
        .codec_name = machine_codec_name,/*"wm8990.0-001a"*/
        .init = balong_audio_late_init,
        .ops = &balong_snd_ops,
    },
};

int snd_machine_codec_name_set(char* name)
{
    unsigned int len = strlen(name);
    if(len + 1 > MACHINE_CODEC_NAME_LENGTH)
    {
        printk(KERN_ERR "%s length(%d) too long.\n", name, len);
        return -1;
    }

    (void)strncpy(machine_codec_name, name, len + 1);
    return 0;
}

int balong_audio_late_init(struct snd_soc_pcm_runtime *rtd)
{
    int ret = 0;
    unsigned int i = 0;
    unsigned int reg = 0;
    unsigned int volume = 0xf0;
    unsigned int filter = SND_SAMPLE_RATE_8K == snd_sample_rate_get() ? 0x3 : 0x1; // other 0x0

     unsigned int pll_in  = AUDIO_MCLK_FREG;     /* input MCLK=19.2MHz */
    unsigned int pll_out = AUDIO_SYSCLK_FREG*2; /* need MCLKDIV(2), and output SYSCLK=12.288MHz */

    unsigned int mode_flag = (SND_PCM_MODE == snd_data_mode_get() ? SND_SOC_DAIFMT_DSP_A:SND_SOC_DAIFMT_I2S);
    unsigned int blk_inv  = (SND_PCM_MODE == snd_data_mode_get() ? WM8990_AIF_BCLK_INV:0);
    unsigned int dacr_src  = (SND_PCM_MODE == snd_data_mode_get() ? 0 : 0x4000);

    unsigned int master_slave_flag = SND_SOC_DAIFMT_CBM_CFM;
    struct snd_soc_dai *codec_dai = rtd->codec_dai;

    static struct snd_pcm_hw_params params;
    memset(&params,0, sizeof(struct snd_pcm_hw_params));

    printk(KERN_INFO "snd audio codec 0x%x\n", (unsigned int)codec_dai->codec);

    if(SND_SOC_SLAVE == snd_master_slave_get())
    {
        /* audio配置为master模式 */
        master_slave_flag = SND_SOC_DAIFMT_CBM_CFM;

        /* 配置SIO为从模式 */
        sio_master_set(snd_master_slave_get());
        //sio_pcm_div(8192000, 32000);
    }
    else
    {
        /* audio配置为slave模式 */
        master_slave_flag = SND_SOC_DAIFMT_CBS_CFS;

        if(SND_PCM_MODE == snd_data_mode_get())
        {
            sio_pcm_div(8192000, SND_SAMPLE_RATE_8K == snd_sample_rate_get() ? 8000 : 16000);// pcm clk
        }
        else
        {
            sio_pcm_div(8192000, SND_SAMPLE_RATE_8K == snd_sample_rate_get() ? 16000 : 32000);// i2s clk
        }
        sio_clock_enable();

        /* 配置SIO为主模式 */
        sio_master_set(snd_master_slave_get());
    }

    snd_soc_update_bits(codec_dai->codec, WM8990_DAC_CTRL, WM8990_DAC_SB_FILT, 0x0100); // just for low sample

    snd_soc_update_bits(codec_dai->codec, WM8990_ADC_CTRL, WM8990_ADC_HPF_ENA,
                            0x0<<WM8990_ADC_HPF_ENA_BIT);

    snd_soc_update_bits(codec_dai->codec, WM8990_ADC_CTRL, WM8990_ADC_HPF_CUT_MASK<<WM8990_ADC_HPF_CUT_SHIFT,
                            filter<<WM8990_ADC_HPF_CUT_SHIFT);

    snd_soc_update_bits(codec_dai->codec, WM8990_ADC_CTRL, WM8990_ADC_HPF_ENA,
                            0x1<<WM8990_ADC_HPF_ENA_BIT);


    /*snd_soc_update_bits(codec_dai->codec, WM8990_DIGITAL_SIDE_TONE, WM8990_ADCL_DAC_SVOL_MASK<<WM8990_ADCL_DAC_SVOL_SHIFT,
        0x7<<WM8990_ADCL_DAC_SVOL_SHIFT);
    snd_soc_update_bits(codec_dai->codec, WM8990_DIGITAL_SIDE_TONE, WM8990_ADCR_DAC_SVOL_MASK<<WM8990_ADCR_DAC_SVOL_SHIFT,
        0x7<<WM8990_ADCR_DAC_SVOL_SHIFT);

    snd_soc_update_bits(codec_dai->codec, WM8990_DIGITAL_SIDE_TONE, WM8990_ADC_TO_DACL_MASK<<WM8990_ADC_TO_DACL_SHIFT,
        0x2<<WM8990_ADC_TO_DACL_SHIFT);
    snd_soc_update_bits(codec_dai->codec, WM8990_DIGITAL_SIDE_TONE, WM8990_ADC_TO_DACR_MASK<<WM8990_ADC_TO_DACR_SHIFT,
        0x1<<WM8990_ADC_TO_DACR_SHIFT);    */

    /* headphone output enable */
    snd_soc_update_bits(codec_dai->codec, WM8990_POWER_MANAGEMENT_1, WM8990_VREF_ENA |
                    WM8990_VMID_MODE_MASK | WM8990_MICBIAS_ENA | WM8990_ROUT_ENA |
                    WM8990_LOUT_ENA | WM8990_OUT4_ENA , 0x1713);

    /* ADCL ADCR and LIN12 enable */
    snd_soc_update_bits(codec_dai->codec, WM8990_POWER_MANAGEMENT_2, WM8990_ADCR_ENA |
                    WM8990_ADCL_ENA | WM8990_RIN12_ENA | WM8990_AINR_ENA |
                    WM8990_TSHUT_OPDIS | WM8990_TSHUT_ENA | WM8990_PLL_ENA, 0xe113);

    /* MIX VOLUNME enable */
    snd_soc_update_bits(codec_dai->codec, WM8990_POWER_MANAGEMENT_3, WM8990_DACR_ENA |
                    WM8990_DACL_ENA | WM8990_ROMIX_ENA | WM8990_LOMIX_ENA |
                    WM8990_ROPGA_ENA | WM8990_LOPGA_ENA , 0x1f3);

    /* RIN12 pga volume */
    snd_soc_update_bits(codec_dai->codec, WM8990_RIGHT_LINE_INPUT_1_2_VOLUME, WM8990_RIN12VOL_MASK |
        WM8990_RI12MUTE | WM8990_IPVU, 0x10e);

    /* volume control */
    reg = snd_soc_read(codec_dai->codec, WM8990_RIGHT_ADC_DIGITAL_VOLUME);
    reg &= WM8990_ADCR_VOL_MASK;

    for(i = reg; i<= volume; i++)
    {
        snd_soc_update_bits(codec_dai->codec, WM8990_RIGHT_ADC_DIGITAL_VOLUME, WM8990_ADC_VU, 0);
        snd_soc_update_bits(codec_dai->codec, WM8990_RIGHT_ADC_DIGITAL_VOLUME, WM8990_ADCR_VOL_MASK, i);
        snd_soc_update_bits(codec_dai->codec, WM8990_RIGHT_ADC_DIGITAL_VOLUME, WM8990_ADC_VU, 0X100);
    }

    reg = snd_soc_read(codec_dai->codec, WM8990_RIGHT_DAC_DIGITAL_VOLUME);
    reg &= WM8990_DACR_VOL_MASK;

    for(i = reg; i<= volume; i++)
    {
        snd_soc_update_bits(codec_dai->codec, WM8990_RIGHT_DAC_DIGITAL_VOLUME, WM8990_DAC_VU, 0);
        snd_soc_update_bits(codec_dai->codec, WM8990_RIGHT_DAC_DIGITAL_VOLUME, WM8990_DACR_VOL_MASK, i);
        snd_soc_update_bits(codec_dai->codec, WM8990_RIGHT_DAC_DIGITAL_VOLUME, WM8990_DAC_VU, 0X100);
    }

    /* BCLK INV */
    snd_soc_update_bits(codec_dai->codec, WM8990_AUDIO_INTERFACE_1, WM8990_AIF_BCLK_INV, blk_inv);
    snd_soc_update_bits(codec_dai->codec, WM8990_AUDIO_INTERFACE_1, WM8990_AIFADCR_SRC |
                    WM8990_AIFADCL_SRC, 0xc118);

    /* DACR slect source */
    snd_soc_update_bits(codec_dai->codec, WM8990_AUDIO_INTERFACE_2, WM8990_DACR_SRC | WM8990_DACL_SRC, dacr_src);

    ret = snd_soc_dai_set_pll(codec_dai, 0, 0, pll_in, pll_out );/* PLL 19.2MHz -> 12.288*2MHz*/
    if (ret < 0)
    {
        printk(KERN_ERR "can't set %s pll: %d\n", codec_dai->name, ret);
        return ret;
    }

    snd_soc_dai_set_clkdiv(codec_dai, WM8990_MCLK_DIV  ,  0x2<<0xb);
    snd_soc_dai_set_clkdiv(codec_dai, WM8990_DACCLK_DIV,  0x6<<0x2);
    snd_soc_dai_set_clkdiv(codec_dai, WM8990_ADCCLK_DIV,  0x6<<0x5);

    snd_soc_dai_set_clkdiv(codec_dai, WM8990_BCLK_DIV  ,  0x1<<0x1);

    snd_soc_update_bits(codec_dai->codec, WM8990_AUDIO_INTERFACE_3, WM8990_ADCLRC_RATE_MASK,
        SND_SAMPLE_RATE_8K == snd_sample_rate_get() ? 0x400 : 0x200);
    snd_soc_update_bits(codec_dai->codec, WM8990_AUDIO_INTERFACE_4, WM8990_ADCLRC_RATE_MASK,
        SND_SAMPLE_RATE_8K == snd_sample_rate_get() ? 0x400 : 0x200);

    ret = snd_soc_dai_set_fmt(codec_dai, master_slave_flag | mode_flag);
    if (ret < 0)
    {
        printk(KERN_ERR "can't set %s fmt: %d\n", codec_dai->name, ret);
        return ret;
    }

    ret = snd_soc_dai_set_sysclk(codec_dai, 0, AUDIO_SYSCLK_FREG, 0);
    if (ret < 0)
    {
        printk(KERN_ERR "can't set %s sysclk: %d\n", codec_dai->name, ret);
        return ret;
    }
    /* sample 16bit */
    params.masks[SNDRV_PCM_HW_PARAM_FORMAT - SNDRV_PCM_HW_PARAM_FIRST_MASK].bits[SNDRV_PCM_FORMAT_S16_LE] = 1;

    ret = codec_dai->driver->ops->hw_params(NULL, &params, codec_dai);
    if (ret < 0) {
        printk(KERN_ERR "can't set %s hw params: %d\n", codec_dai->name, ret);
        return -1;
    }

    /* volume unmute */
    snd_soc_update_bits(codec_dai->codec, WM8990_INPUT_MIXER2, WM8990_RMN1 | WM8990_RMP2, 0x3);

    /* RIN12 unmute */
    snd_soc_update_bits(codec_dai->codec, WM8990_INPUT_MIXER4, WM8990_R12MNBST | WM8990_R12MNB, 0x30);

    /* LDAC to LOMIX unmute */
    snd_soc_update_bits(codec_dai->codec, WM8990_OUTPUT_MIXER1, WM8990_LDLO, 0x1);

    /* RDAC to ROMIX unmute */
    snd_soc_update_bits(codec_dai->codec, WM8990_OUTPUT_MIXER2, WM8990_RDRO, 0x1);

    ret = codec_dai->driver->ops->digital_mute(codec_dai, 0);
    if (ret < 0) {
        printk(KERN_ERR "can't set %s hw params: %d\n", codec_dai->name, ret);
        return -1;
    }
    return 0;

}

static int snd_machine_probe(struct snd_soc_card *card)
{
 //   zsi_clk_cfg();
 //   zsi_enable();

    return 0;
}


static int snd_machine_remove(struct snd_soc_card *card)
{
    printk(KERN_ERR "snd machine remove ok!\n");
    return 0;
}


static struct snd_soc_card snd_balong = {
    .name = "balong audio",
    .owner = THIS_MODULE,
    .dai_link = snd_machine_dai,
    .num_links = ARRAY_SIZE(snd_machine_dai),
    .probe = snd_machine_probe,
    .remove = snd_machine_remove,

};


int snd_balong_platform_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct snd_soc_card *card = &snd_balong;

    card->dev = &pdev->dev;

    ret = snd_soc_register_card(card);
    if (ret < 0)
    {
        printk(KERN_ERR "snd_soc_register_card failed: %d\n", ret);
    }

    printk(KERN_ERR "snd machine probe ok.\n");

    return ret;
}


int snd_balong_platform_remove(struct platform_device *pdev)
{
    int ret = 0;
    struct snd_soc_card *card = &snd_balong;

    ret = snd_soc_unregister_card(card);
    if (ret < 0)
    {
        printk(KERN_ERR "snd_soc_register_card failed: %d\n", ret);
    }
    return 0;
}

MODULE_AUTHOR("Huawei Technology Co., Ltd");
MODULE_DESCRIPTION("Balong ASoC MACHINE Driver");
MODULE_LICENSE("GPL");


