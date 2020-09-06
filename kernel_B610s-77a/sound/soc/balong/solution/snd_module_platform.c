/*
 * sound/soc/balong/snd_module_platform.c  --  ALSA Soc Platform Layer
 *
 *  Copyright (c) 2009 Huawei Technology Co., Ltd
 *  Author: Huawei Technology Co., Ltd
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <sound/soc.h>
#include "snd_ctl.h"
#include "snd_init.h"
#include "drv_recorder_interface.h"

#define SND_EDMA_LLI_NUM      (16  )
#define SND_LLI_BLOCK_SIZE    (320 )
#define SND_DATA_SAMPLE_RATE  (8000)

static u32   g_hifi_is_ready  = 0;
static u32   g_call_is_end    = 0;
static u32   g_pcm_test_data_check_flag = 0;

typedef struct {
    u32 play_miss_num;
    u32 capt_miss_num;
}snd_debug_info;

typedef struct {
    u32   phy_addr;
    u32   period_size;
    u32   period_num;
    u32   current_period;
    u32   next_period;
    void *substream;
}snd_runtime_data_str;


static snd_runtime_data_str *pcm_prtd = NULL;
static snd_debug_info  pcm_miss_info  = {0};


void snd_pcm_debug_info_show(void)
{
    printk(KERN_ERR"play_miss_num = %d,capt_miss_num = %d.\n",
        pcm_miss_info.play_miss_num,pcm_miss_info.capt_miss_num);
}

/*控制命令接口*/
int __attribute__((weak))  HiFiAgent_SendVoiceMsgReq(u32 cmd, void *data, u32 len)
{
    return 0;
}

void snd_pcm_test_data_set_flag(u32 flag)
{
    g_pcm_test_data_check_flag = flag;
}

void snd_pcm_check_data(struct snd_pcm_substream *substream)
{
    int i = 0;
    unsigned int *addr;
    snd_runtime_data_str *prt = (snd_runtime_data_str*)substream->runtime->private_data;

    addr = substream->runtime->dma_area + (prt->next_period * prt->period_size);

    printk(KERN_ERR"[%s] next_period:%d i:%d, val: %x\n",
                        __func__, prt->next_period, i, addr[i]);
    for ( i = 0; i < prt->period_size / sizeof(unsigned int) - 1; i++)
    {
        if (addr[i] != addr[i + 1])
        {
            printk(KERN_ERR"[%s]play data error.  prt->next_period:%d i:%d, val: %x, val+1: %x\n",
                        __func__, prt->next_period, i, addr[i], addr[i + 1]);
            break;
        }
    }
}

int snd_pcm_set_buff(struct snd_pcm_substream *substream)
{
    snd_runtime_data_str *prt = (snd_runtime_data_str*)substream->runtime->private_data;
    u32  stream = substream->stream;
    u32  data_size = prt->period_size;

    struct DRV_VOICE_DATA_REQ data_req = {0};

    data_req.uwMode    = stream;
    data_req.uwBufSize = data_size;
    data_req.uwBufAddr = (u32)(substream->runtime->dma_addr + (prt->next_period * prt->period_size));

    /*just for test,to delete*/
    if (g_pcm_test_data_check_flag && SNDRV_PCM_STREAM_PLAYBACK == stream)
    {
        snd_pcm_check_data(substream);
    }

    if (HiFiAgent_SendVoiceMsgReq(ID_DRV_SET_BUFFER_REQ, &data_req, sizeof(data_req)))
    {
        printk(KERN_ERR"[%s]send data req to hifi err.\n",__func__);
        return -1;
    }
    return 0;
}

int sound_drv_alsa_ioctrl (u32 cmd, void *data, u32 len)
{
    u32 stream   = 0;
    u32 availnum = 0;
    struct snd_pcm_substream *substream;
    struct snd_pcm_runtime   *runtime;
    snd_runtime_data_str     *prtd ;

    if (NULL == pcm_prtd)
    {
        return -1;
    }

    substream = pcm_prtd->substream;
    runtime   = substream->runtime;
    prtd      = runtime->private_data;

    switch(cmd)
    {
        case ID_DRV_RECORD_START_CNF:
        {
            g_hifi_is_ready = 1;
            printk(KERN_ERR"[%s]hifi is ready for record.\n",__func__);

            break;
        }
        case ID_DRV_START_PLAY_CNF:
        {
            g_hifi_is_ready = 1;
            printk(KERN_ERR"[%s]hifi is ready for remote tts play.\n",__func__);

            break;
        }
        case ID_HIFI_DRV_BUFFER_DATA_READY:
        {
            if (!data)
            {
                printk(KERN_ERR"[%s]pass the err params.\n",__func__);
                return -1;
            }

            if (!g_hifi_is_ready || g_call_is_end)
            {
                printk(KERN_ERR"[%s]hifi is %s.\n",__func__,(g_call_is_end == 1 ? "end" : "not ready"));
                return -1;
            }

            prtd->current_period = prtd->next_period;
            snd_pcm_period_elapsed(substream);

            stream = *(u32*)data;
            if (SNDRV_PCM_STREAM_PLAYBACK == stream)
            {
                availnum = snd_pcm_playback_hw_avail(runtime);

                /*如果有效数据小于320，则发送上一次数据*/
                if ( runtime->period_size > availnum)
                {
                    printk(KERN_ERR"[%s]availnum = %d.\n",__func__,availnum);
                    pcm_miss_info.play_miss_num++;
                }
                else
                {
                    prtd->next_period = (prtd->next_period + 1) % prtd->period_num;
                }
            }
            else if (SNDRV_PCM_STREAM_CAPTURE == stream)
            {
                availnum = snd_pcm_capture_hw_avail(runtime);
                if (runtime->period_size > availnum)
                {
                    printk(KERN_ERR"[%s]buf is full,empty num = %d.\n",__func__,availnum);
                    pcm_miss_info.capt_miss_num++;
                }

                prtd->next_period = (prtd->next_period + 1) % prtd->period_num;
            }

            if (snd_pcm_set_buff(substream))
            {
                printk(KERN_ERR"[%s]send cmd set_buff to hifi err.\n",__func__);
            }
            break;
        }
        case ID_DRV_HIFI_RECORD_ERR_NOTIFY:
        case ID_DRV_HIFI_PLAY_ERR_NOTIFY:
        {
            printk(KERN_ERR"[%s]some recoverable err happened in hifi,data may be lost.\n",__func__);
            break;
        }
        /*通话结束，置硬件未连接状态，这样read、write操作会立即返回，
         *等待上层close操作*/
        case ID_HIFI_DRV_CALL_END_NOTIFY:
        {
            g_call_is_end = 1;
            snd_pcm_stop(substream, SNDRV_PCM_STATE_DISCONNECTED);

            printk(KERN_ERR"[%s]call is end,and hifi is stop.\n",__func__);
            break;
        }

        case ID_DRV_STOP_RECORD_CNF:
        case ID_DRV_STOP_PLAY_CNF:
        {
            printk(KERN_ERR"[%s]send stop cmd to hifi,and hifi is stop.\n",__func__);
            break;
        }
        default:
            break;
    }
    return 0;
}

void snd_sw_params_set(struct snd_pcm_substream *substream)
{

    struct snd_pcm_runtime *runtime = substream->runtime;
    static const struct snd_pcm_hardware dma_hardware = {
        .info            = SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_PAUSE | \
                           SNDRV_PCM_INFO_BLOCK_TRANSFER,
        .formats         = SNDRV_PCM_FORMAT_S16_LE,
        .channels_min        = 1,
        .channels_max        = 1,
        .buffer_bytes_max    = SND_EDMA_LLI_NUM*SND_LLI_BLOCK_SIZE,
        .period_bytes_min    = SND_LLI_BLOCK_SIZE,
        .period_bytes_max    = SND_LLI_BLOCK_SIZE,
        .periods_min         = SND_EDMA_LLI_NUM,
        .periods_max         = SND_EDMA_LLI_NUM,
        .fifo_size           = 8,
    };

    snd_soc_set_runtime_hwparams(substream, &dma_hardware);

    runtime->access = SNDRV_PCM_ACCESS_RW_INTERLEAVED;
    runtime->info = dma_hardware.info;
    runtime->channels = 1;
    runtime->sample_bits = 16;
    runtime->frame_bits = (runtime->sample_bits) * (runtime->channels);
    runtime->format = SNDRV_PCM_FORMAT_S16_LE;
    runtime->rate = SNDRV_PCM_RATE_8000;
    runtime->period_size = SND_LLI_BLOCK_SIZE*8/runtime->frame_bits;
    runtime->buffer_size = SND_EDMA_LLI_NUM * SND_LLI_BLOCK_SIZE*8/runtime->frame_bits;
    runtime->periods = SND_EDMA_LLI_NUM;

    runtime->byte_align = runtime->frame_bits / 8;
    runtime->min_align = 1;

    /* Default sw params */
    runtime->tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
    runtime->period_step = 1;
    runtime->control->avail_min = runtime->period_size;

    if (SNDRV_PCM_STREAM_PLAYBACK == substream->stream)
    {
        /*至少2*320个数据才开始*/
        runtime->start_threshold = runtime->period_size * 2;
    }
    else
    {
        runtime->start_threshold = runtime->period_size ;
    }

    runtime->boundary = runtime->buffer_size;
    while (runtime->boundary * 2 <= LONG_MAX - runtime->buffer_size)
        runtime->boundary *= 2;

#if (ZSI_XRUN_SILENCE == ZSI_XRUN_MODE)
    runtime->stop_threshold = LONG_MAX;
    runtime->silence_threshold = 0;
    runtime->silence_size = runtime->boundary;
#elif (ZSI_XRUN_STOP == ZSI_XRUN_MODE)
    runtime->stop_threshold = runtime->buffer_size;
    runtime->silence_threshold = 0;
    runtime->silence_size = 0;
#endif

    runtime->dma_area = substream->dma_buffer.area;
    runtime->dma_addr = substream->dma_buffer.addr;
    runtime->dma_bytes= substream->dma_buffer.bytes;

    pcm_prtd->phy_addr  = runtime->dma_addr;
    pcm_prtd->current_period = 0;
    pcm_prtd->next_period    = 0;
    pcm_prtd->substream   = substream;
    pcm_prtd->period_num  = runtime->periods;
    pcm_prtd->period_size = SND_LLI_BLOCK_SIZE;

    runtime->status->state = SNDRV_PCM_STATE_PREPARED;
    runtime->private_data  = pcm_prtd;

    return ;

}

snd_pcm_uframes_t snd_hifi_pointer(struct snd_pcm_substream *substream)
{
    snd_runtime_data_str *prtd = (snd_runtime_data_str*)substream->runtime->private_data;
    long frame                     = 0L;

    frame = bytes_to_frames(substream->runtime, prtd->current_period * prtd->period_size);
    if (frame >= substream->runtime->buffer_size)
        frame = 0;

    return (snd_pcm_uframes_t)frame;
}

static int snd_hifi_trigger(struct snd_pcm_substream *substream, int cmd)
{
    int ret = 0;
    u32 cmd_id = 0;
    u32 stream = substream->stream;
    struct DRV_VOICE_START_REQ start_quest = {0};
    snd_runtime_data_str* prt = (snd_runtime_data_str*)substream->runtime->private_data;

    if (!prt)
    {
        printk(KERN_ERR"[%s]prt is null.\n",__func__);
        return -1;
    }

    switch (cmd) {
    case SNDRV_PCM_TRIGGER_START:
    case SNDRV_PCM_TRIGGER_RESUME:
    case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:

        start_quest.uwBufAddr   = (u32)prt->phy_addr;
        start_quest.uwBufSize   = (u32)prt->period_size;
        start_quest.ulSampeRate = SND_DATA_SAMPLE_RATE;

        cmd_id = SNDRV_PCM_STREAM_PLAYBACK == stream ?  \
             ID_DRV_START_PLAY_REQ : ID_DRV_START_RECORD_REQ;
        if (HiFiAgent_SendVoiceMsgReq(cmd_id, &start_quest, sizeof(start_quest)))
        {
            printk(KERN_ERR"[%s]send start cmd to hifi err.\n",__func__);
        }

        printk(KERN_ERR"[%s]state is start.\n",__func__);
        break;

    case SNDRV_PCM_TRIGGER_STOP:
    case SNDRV_PCM_TRIGGER_SUSPEND:
    case SNDRV_PCM_TRIGGER_PAUSE_PUSH:

        if (g_call_is_end)
        {
            printk(KERN_ERR"[%s]call is end,no need to send stop cmd to hifi.\n",__func__);
            return 0;
        }

        cmd_id = SNDRV_PCM_STREAM_PLAYBACK == stream ? \
                      ID_DRV_STOP_PLAY_REQ : ID_DRV_STOP_RECORD_REQ;
        if (HiFiAgent_SendVoiceMsgReq(cmd_id, NULL , 0))
        {
            printk(KERN_ERR"[%s]send stop cmd to hifi err.\n",__func__);
        }
        printk(KERN_ERR"[%s]state is stop.\n",__func__);
        break;

    default:
        ret = -EINVAL;
        break;
    }
    return ret;
}

int snd_hifi_open(struct snd_pcm_substream *substream)
{
    if (pcm_prtd)
    {
        printk(KERN_ERR "this device has been opened.\n");
        return -1;
    }

    pcm_prtd = kzalloc(sizeof(snd_runtime_data_str), GFP_KERNEL);
    if (pcm_prtd == NULL)
    {
        printk(KERN_ERR "kzalloc runtime_data fail.\n");
        return -ENOMEM;
    }

    snd_sw_params_set(substream);

    pcm_miss_info.capt_miss_num = 0;
    pcm_miss_info.play_miss_num = 0;
    g_hifi_is_ready = 0;
    g_call_is_end   = 0;

    printk(KERN_ERR "snd_hifi_open ok.\n");
    return 0;
}

int snd_hifi_close(struct snd_pcm_substream *substream)
{
    kfree(pcm_prtd);
    pcm_prtd = NULL;

    printk(KERN_ERR "snd_hifi_close ok.\n");
    return 0;
}


int snd_hifi_new(struct snd_soc_pcm_runtime *rtd)
{
    int ret = 0;
    u64 dma_mask = DMA_BIT_MASK(32);
    struct snd_card *card = rtd->card->snd_card;
    struct snd_pcm *pcm = rtd->pcm;

    if (1 != pcm->device)
    {
        printk(KERN_ERR "[%s]wrong device num.\n",__func__);
        return -1;
    }

    if (!card->dev->dma_mask)
        card->dev->dma_mask = &dma_mask;
    if (!card->dev->coherent_dma_mask)
        card->dev->coherent_dma_mask = DMA_BIT_MASK(32);

    ret = snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_DEV, pcm->card->dev,
            SND_EDMA_LLI_NUM * SND_LLI_BLOCK_SIZE, SND_EDMA_LLI_NUM * SND_LLI_BLOCK_SIZE);
    if (ret)
    {
        printk(KERN_ERR "[%s]allocate pages failed!\n",__func__);
    }
    return ret;

}

void snd_hifi_free(struct snd_pcm *pcm)
{
    snd_pcm_lib_preallocate_free_for_all(pcm);
    return;
}

static struct snd_pcm_ops snd_hifi_ops = {
    .open       = snd_hifi_open,
    .close      = snd_hifi_close,
    //.ioctl    = snd_pcm_lib_ioctl,
    .trigger    = snd_hifi_trigger,
    .pointer    = snd_hifi_pointer,
};

static struct snd_soc_platform_driver snd_hifi_platform_balong = {
    .ops        = &snd_hifi_ops,
    .pcm_new    = snd_hifi_new,
    .pcm_free   = snd_hifi_free,
};

int snd_module_platform_probe(struct platform_device *pdev)
{
    int ret = 0;

    ret = snd_soc_register_platform(&pdev->dev, &snd_hifi_platform_balong);
    if (ret < 0) {
        printk(KERN_ERR "snd_module_platform_probe register failed: %d\n", ret);
        return ret;
    }

    printk(KERN_ERR "snd_module_platform_probe ok!\n");

    return ret;

}

int snd_module_platform_remove(struct platform_device *pdev)
{
    snd_soc_unregister_platform(&pdev->dev);
    
    printk(KERN_ERR "snd_module_platform_remove  ok!\n");
    return 0;
}


MODULE_AUTHOR("Huawei Technology Co., Ltd");
MODULE_DESCRIPTION("Balong ASoC DMA Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:balong sound platform");


