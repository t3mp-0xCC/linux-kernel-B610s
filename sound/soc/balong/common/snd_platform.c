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


#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <sound/soc.h>

#include "bsp_edma.h"
//#include "bsp_hardtimer.h"

#include "snd_param.h"

#include "hi_sio.h"
#if (FEATURE_ON == MBB_SLIC)
#define SND_EDMA_LLI_NUM   (3)
#else
#define SND_EDMA_LLI_NUM   (50)
#endif
#define SND_LLI_BLOCK_SIZE (160)

//unsigned int snd_dma_id[2] = {0,0};
struct runtime_data
{
    unsigned int dma_id;
};

void snd_platform_print(struct snd_pcm_runtime *runtime)
{
    printk(KERN_INFO "runtime 0x%x\n", (u32)runtime);

    printk(KERN_INFO "access 0x%x, format 0x%x, subformat 0x%x, rate 0x%x, channels 0x%x.\n",
    (u32)runtime->access, (u32)runtime->format, (u32)runtime->subformat,(u32)runtime->rate, (u32)runtime->channels);

    printk(KERN_INFO "period_size 0x%x, periods 0x%x, buffer_size 0x%x, min_align 0x%x, byte_align 0x%x.\n",
    (u32)runtime->period_size, (u32)runtime->periods, (u32)runtime->buffer_size,(u32)runtime->min_align, (u32)runtime->byte_align);

    printk(KERN_INFO "frame_bits 0x%x, sample_bits 0x%x, info 0x%x, rate_num 0x%x, rate_den 0x%x, no_period_wakeup 0x%x.\n",
    (u32)runtime->frame_bits, (u32)runtime->sample_bits, (u32)runtime->info,(u32)runtime->rate_num, (u32)runtime->rate_den, (u32)runtime->no_period_wakeup);


    printk(KERN_INFO "tstamp_mode 0x%x, period_step 0x%x, start_threshold 0x%x, stop_threshold 0x%x, silence_threshold 0x%x.\n",
    (u32)runtime->tstamp_mode, (u32)runtime->period_step, (u32)runtime->start_threshold,(u32)runtime->stop_threshold, (u32)runtime->silence_threshold);

     printk(KERN_INFO "silence_size 0x%x, boundary 0x%x, silence_start 0x%x, silence_filled 0x%x.\n",
    (u32)runtime->silence_size, (u32)runtime->boundary, (u32)runtime->silence_start,(u32)runtime->silence_filled);


}

snd_pcm_uframes_t snd_dma_pointer(struct snd_pcm_substream *substream)
{
    struct runtime_data *prtd = substream->runtime->private_data;
    struct snd_dma_buffer *dma_buf = &substream->dma_buffer;
    s32 pos = bsp_edma_current_transfer_address(prtd->dma_id) - (s32)dma_buf->addr;

    return (snd_pcm_uframes_t)bytes_to_frames(substream->runtime, pos);

}

void snd_dma_isr_handle(unsigned int channel_arg, unsigned int int_status)
{
    if(EDMA_INT_LLT_DONE == int_status || (EDMA_INT_DONE == int_status))
    {
        snd_pcm_period_elapsed((struct snd_pcm_substream *)channel_arg);
    }
    else
    {
        printk(KERN_INFO "snd dma isr status 0x%x.\n", int_status);
    }

}

int snd_dma_sw_params_set(struct snd_pcm_substream *substream, struct runtime_data *prtd)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    static const struct snd_pcm_hardware dma_hardware = {
    .info                = SNDRV_PCM_INFO_INTERLEAVED |
                          SNDRV_PCM_INFO_BLOCK_TRANSFER |
                          SNDRV_PCM_INFO_PAUSE,
    .formats            = SNDRV_PCM_FORMAT_S16_LE,
    .channels_min        = 1,
    .channels_max        = 1,
    .buffer_bytes_max    = SND_EDMA_LLI_NUM*SND_LLI_BLOCK_SIZE,
    .period_bytes_min    = SND_LLI_BLOCK_SIZE,
    .period_bytes_max    = SND_LLI_BLOCK_SIZE,
    .periods_min        = SND_EDMA_LLI_NUM,
    .periods_max        = SND_EDMA_LLI_NUM,
    .fifo_size            = 8,
    };

    snd_soc_set_runtime_hwparams(substream, &dma_hardware);

    runtime->access = SNDRV_PCM_ACCESS_RW_INTERLEAVED;
    runtime->channels = SND_PCM_MODE == snd_data_mode_get() ? 0x1 : 0x2;
    runtime->sample_bits = 16;
    runtime->frame_bits = (runtime->sample_bits) * (runtime->channels);
    runtime->format = SNDRV_PCM_FORMAT_S16_LE;
    runtime->rate = SND_SAMPLE_RATE_8K == snd_sample_rate_get() ? SNDRV_PCM_RATE_8000 : SNDRV_PCM_RATE_16000;
    runtime->period_size = SND_LLI_BLOCK_SIZE*8/runtime->frame_bits;
    runtime->buffer_size = SND_EDMA_LLI_NUM * SND_LLI_BLOCK_SIZE*8/runtime->frame_bits;
    runtime->periods = SND_EDMA_LLI_NUM;

    runtime->info = dma_hardware.info;

    runtime->byte_align = runtime->frame_bits / 8;
    runtime->min_align = 1;

    /* Default sw params */
    runtime->tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
    runtime->period_step = 1;
    runtime->control->avail_min = runtime->period_size;
    runtime->start_threshold = runtime->period_size;

    runtime->boundary = runtime->buffer_size;
    while (runtime->boundary * 2 <= LONG_MAX - runtime->buffer_size)
        runtime->boundary *= 2;

    if(SND_XRUN_SILENCE == snd_xrun_mode_get())
    {
        runtime->stop_threshold = LONG_MAX;
        runtime->silence_threshold = 0;
        runtime->silence_size = runtime->boundary;
    }
    else
    {
        runtime->stop_threshold = runtime->buffer_size;
        runtime->silence_threshold = 0;
        runtime->silence_size = 0;
    }

    runtime->dma_area = substream->dma_buffer.area;
    runtime->dma_addr = substream->dma_buffer.addr;
    runtime->dma_bytes= substream->dma_buffer.bytes;

    runtime->status->state = SNDRV_PCM_STATE_PREPARED;

    snd_platform_print(runtime);

    return 0;

}


int snd_dma_hw_params_set(struct snd_pcm_substream * substream, struct runtime_data *prtd)
{
    u32 i = 0;
    s32 ret = 0;
    enum edma_req_id req_id = EDMA_BUTT;

    struct edma_cb *f_node = NULL;
    struct edma_cb *pst_temp = NULL;

    struct snd_dma_buffer *buf = &substream->dma_buffer;

    edma_addr_t edma_addr = buf->addr + buf->bytes;

    f_node = pst_temp = (struct edma_cb*)(buf->area + buf->bytes);

    for (i = 0; i < SND_EDMA_LLI_NUM ; i++)
    {
        if(i == (SND_EDMA_LLI_NUM -1))
        {
            pst_temp->lli = EDMA_SET_LLI(edma_addr, 0); /* 8bit */
        }
        else
        {
            pst_temp->lli = EDMA_SET_LLI(edma_addr + (i+1) * sizeof(struct edma_cb), 0);
        }

        if(SNDRV_PCM_STREAM_PLAYBACK == substream->stream)
        {
            pst_temp->config = EDMA_SET_CONFIG(EDMA_LTESIO_TX, EDMA_M2P,
                SND_PCM_MODE == snd_data_mode_get() ? EDMA_TRANS_WIDTH_16 : EDMA_TRANS_WIDTH_32, EDMA_BUR_LEN_4);/*just for i2s lineband and pcm */
            pst_temp->src_addr = buf->addr + i* SND_LLI_BLOCK_SIZE;
            pst_temp->des_addr = HI_LTESIO_REGBASE_ADDR + (SND_PCM_MODE == snd_data_mode_get() ? HI_SIO_PCM_XD_OFFSET : HI_SIO_I2S_DUAL_TX_CHN_OFFSET);
        }
        else
        {
            pst_temp->config = EDMA_SET_CONFIG(EDMA_LTESIO_RX, EDMA_P2M,
                SND_PCM_MODE == snd_data_mode_get() ? EDMA_TRANS_WIDTH_16 : EDMA_TRANS_WIDTH_32, EDMA_BUR_LEN_4);/*just for i2s lineband and pcm */
            pst_temp->src_addr = HI_LTESIO_REGBASE_ADDR + (SND_PCM_MODE == snd_data_mode_get() ? HI_SIO_PCM_RD_OFFSET : HI_SIO_I2S_DUAL_RX_CHN_OFFSET);
            pst_temp->des_addr = buf->addr + i * SND_LLI_BLOCK_SIZE;
        }

        pst_temp->cnt0 = SND_LLI_BLOCK_SIZE;
        pst_temp->bindx = 0;
        pst_temp->cindx = 0;
        pst_temp->cnt1  = 0;

        pst_temp++;
    }

    req_id = (SNDRV_PCM_STREAM_PLAYBACK == substream->stream) ? EDMA_LTESIO_TX : EDMA_LTESIO_RX;

    ret = bsp_edma_channel_init(req_id, (channel_isr)snd_dma_isr_handle, \
        (u32)substream, (u32)BALONG_DMA_INT_LLT_DONE | BALONG_DMA_INT_DONE \
        | BALONG_DMA_INT_CONFIG_ERR | BALONG_DMA_INT_TRANSFER_ERR | BALONG_DMA_INT_READ_ERR);
    if (ret < 0)
    {
        printk(KERN_ERR" bsp_edma_channel_init fail, req_id = 0x%x, ret %d\n", req_id, ret);
        goto chan_invalid;
    }

    prtd->dma_id = (u32)ret;

    pst_temp = bsp_edma_channel_get_lli_addr(prtd->dma_id);
    if (NULL == pst_temp)
    {
        printk(KERN_ERR"bsp_edma_channel_get_lli_addr fail, dma_id = %d!\n", prtd->dma_id);
        ret = DMA_LLIHEAD_ERROR;
        goto chan_init_error;
    }

    pst_temp->lli = f_node->lli;
    pst_temp->config = f_node->config & 0xFFFFFFFE;
    pst_temp->src_addr = f_node->src_addr;
    pst_temp->des_addr = f_node->des_addr;
    pst_temp->cnt0 = f_node->cnt0;
    pst_temp->bindx = 0;
    pst_temp->cindx = 0;
    pst_temp->cnt1  = 0;

    printk(KERN_INFO "dma%d id=0x%x virt addr=0x%x, phy addr=0x%x, config=0x%x, src addr=0x%x, phy addr=0x%x, cnto=%d, substream=0x%x.\n",
        substream->stream, prtd->dma_id, (unsigned int)buf->area, (unsigned int)buf->addr, (unsigned int)pst_temp->config,
        (unsigned int)pst_temp->src_addr, (unsigned int)pst_temp->des_addr, pst_temp->cnt0, (unsigned int)substream);

    return 0;

chan_init_error:
    bsp_edma_channel_free(prtd->dma_id);

chan_invalid:

    return ret;

}

int snd_dma_open(struct snd_pcm_substream *substream)
{
    int ret = 0;
    struct runtime_data *prtd = NULL;

    printk(KERN_INFO "Entered %s\n", __func__);

    prtd = kzalloc(sizeof(struct runtime_data), GFP_KERNEL);
    if (prtd == NULL)
    {
        printk(KERN_ERR "kzalloc runtime_data fail.\n");
        return -ENOMEM;
    }

    ret = snd_dma_hw_params_set(substream, prtd);
    if (ret < 0){
        kfree(prtd);
        printk(KERN_ERR"stream%d hw params set fail, ret = %d.\n", substream->stream, ret);
        return -1;
    }

    ret = snd_dma_sw_params_set(substream, prtd);
    if (ret < 0){
        kfree(prtd);
        printk(KERN_ERR"stream%d sw params set fail, ret = %d.\n", substream->stream, ret);
        return -1;
    }

    substream->runtime->private_data = prtd;

    printk(KERN_INFO "runtime%d 0x%x\n", substream->stream, (u32)substream->runtime);

    return 0;
}

int snd_dma_close(struct snd_pcm_substream *substream)
{
    struct runtime_data *prtd = NULL;

    printk(KERN_INFO "Entered %s\n", __func__);

    prtd = substream->runtime->private_data;
    bsp_edma_channel_free(prtd->dma_id);
    kfree(prtd);

    return 0;
}


static int snd_dma_trigger(struct snd_pcm_substream *substream, int cmd)
{
    int ret = 0;
    struct runtime_data *prtd = substream->runtime->private_data;

    switch (cmd) {
    case SNDRV_PCM_TRIGGER_START:
    case SNDRV_PCM_TRIGGER_RESUME:
    case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
        ret = bsp_edma_channel_lli_async_start(prtd->dma_id);
        if (ret < 0)
        {
            printk(KERN_ERR "bsp_edma_channel_lli_async_start fail, dma id = %d!\n", prtd->dma_id);
            return EDMA_TRXFER_ERROR;
        }
        printk(KERN_INFO "snd dma%d start.\n", prtd->dma_id);
        break;

    case SNDRV_PCM_TRIGGER_STOP:
    case SNDRV_PCM_TRIGGER_SUSPEND:
    case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
        ret = bsp_edma_channel_stop(prtd->dma_id);
        if(EDMA_FAIL == ret)
        {
            printk(KERN_ERR "bsp_edma_channel_stop fail, dma id = %d!\n", prtd->dma_id);
            return EDMA_TRXFER_ERROR;
        }
        printk(KERN_INFO "snd dma%d stop.\n", prtd->dma_id);
        break;

    default:
        ret = -EINVAL;
        break;
    }

    return ret;

}




int snd_dma_init(struct snd_pcm *pcm, int stream)
{
    struct snd_pcm_substream *substream = pcm->streams[stream].substream;
    struct snd_dma_buffer *buf = &substream->dma_buffer;

    buf->dev.type = SNDRV_DMA_TYPE_DEV;
    buf->dev.dev = pcm->card->dev;
    buf->private_data = NULL;

    buf->area = dma_alloc_coherent(pcm->card->dev,  \
            SND_EDMA_LLI_NUM * SND_LLI_BLOCK_SIZE +  \
            SND_EDMA_LLI_NUM * sizeof(struct edma_cb),\
            &buf->addr, GFP_DMA|__GFP_WAIT);
    if (!buf->area){
        printk(KERN_ERR"stream%d buf is null %s\n", stream, __func__);
        return -ENOMEM;
    }

    buf->bytes = SND_EDMA_LLI_NUM * SND_LLI_BLOCK_SIZE;

    return 0;

}



int snd_dma_new(struct snd_soc_pcm_runtime *rtd)
{
    static u64 dma_mask = DMA_BIT_MASK(32);
    struct snd_card *card = rtd->card->snd_card;
    struct snd_pcm *pcm = rtd->pcm;

    int ret = 0;

    printk(KERN_INFO "Entered %s\n", __func__);

    if (!card->dev->dma_mask)
        card->dev->dma_mask = &dma_mask;
    if (!card->dev->coherent_dma_mask)
        card->dev->coherent_dma_mask = DMA_BIT_MASK(32);

    //spin_lock_init(&snd_lock);

    if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) {
        ret = snd_dma_init(pcm,    SNDRV_PCM_STREAM_PLAYBACK);
        if (ret)
            goto out;
    }

    if (pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream) {
        ret = snd_dma_init(pcm,    SNDRV_PCM_STREAM_CAPTURE);
        if (ret)
            goto out;
    }
out:
    return ret;

}

void snd_dma_free(struct snd_pcm *pcm)
{
    struct snd_pcm_substream *substream;
    struct snd_dma_buffer *buf;
    int stream;

    printk(KERN_INFO "Entered %s\n", __func__);

    for (stream = 0; stream < 2; stream++) {
        substream = pcm->streams[stream].substream;
        if (!substream)
            continue;

        buf = &substream->dma_buffer;
        if (!buf->area)
            continue;

        dma_free_coherent(pcm->card->dev, SND_EDMA_LLI_NUM * SND_LLI_BLOCK_SIZE +  \
            SND_EDMA_LLI_NUM * sizeof(struct edma_cb), buf->area, buf->addr);

        buf->area = NULL;
    }

}

static struct snd_pcm_ops snd_dma_ops = {
    .open        = snd_dma_open,
    .close        = snd_dma_close,
    //.ioctl        = snd_pcm_lib_ioctl,
    .trigger    = snd_dma_trigger,
    .pointer    = snd_dma_pointer,
    .mmap        = NULL,
};

static struct snd_soc_platform_driver snd_platform_balong = {
    .ops        = &snd_dma_ops,
    .pcm_new    = snd_dma_new,
    .pcm_free    = snd_dma_free,
};

int snd_platform_probe(struct platform_device *pdev)
{
    int ret = 0;

    ret = snd_soc_register_platform(&pdev->dev, &snd_platform_balong);
    if (ret < 0) {
        printk(KERN_ERR "failed to get register DAI: %d\n", ret);
        return ret;
    }

    printk(KERN_ERR "snd platform probe ok!\n");

    return ret;

}

int snd_platform_remove(struct platform_device *pdev)
{
    snd_soc_unregister_platform(&pdev->dev);
    printk(KERN_ERR "snd platform remove ok!\n");
    return 0;
}


MODULE_AUTHOR("Huawei Technology Co., Ltd");
MODULE_DESCRIPTION("Balong ASoC DMA Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:balong sound platform");


