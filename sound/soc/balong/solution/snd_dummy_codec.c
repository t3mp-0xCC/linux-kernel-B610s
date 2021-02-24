/*
 * snd_dummy_codec.c  --  LE9662 ALSA Soc codec driver
 *
* Copyright (c) 1988-2015 by Huawei Technologies Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <asm/uaccess.h>
#include <asm/fcntl.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

static struct snd_soc_dai_driver dummy_codec_dai[] =
{
    [0] = {
        /* DAI PCM */
        .name    = "dummy_codec_dai",
        .id    = 0,
        .playback = {
            .stream_name  = "Playback",
            .channels_min = 1,
            .channels_max = 1,
            .rates        = SNDRV_PCM_RATE_8000,
            .formats      = SNDRV_PCM_FORMAT_S16_LE,
        },
        .capture = {
            .stream_name  = "Capture",
            .channels_min = 1,
            .channels_max = 1,
            .rates        = SNDRV_PCM_RATE_8000,
            .formats      = SNDRV_PCM_FORMAT_S16_LE,
        },
    },
};

static int snd_codec_dummy_remove(struct snd_soc_codec* codec)
{
    return 0;
}

static int  snd_codec_dummy_probe(struct snd_soc_codec* codec)
{
    return 0;
}


static struct snd_soc_codec_driver soc_codec_dev_dummy =
{
    .probe     = snd_codec_dummy_probe,
    .remove    = snd_codec_dummy_remove,
    .controls  = NULL,
    .num_controls = 0,

};

int snd_codec_probe(struct platform_device* pdev)
{
    int ret = 0;
    u32  i  = 0;
    
    ret = snd_soc_register_codec(&pdev->dev, &soc_codec_dev_dummy,
                                 dummy_codec_dai, ARRAY_SIZE(dummy_codec_dai));
    if (ret)
    {
        printk(KERN_ERR "Failed to register codec\n");
        return ret;
    }
    printk(KERN_ERR "snd codec dummy probe ok!\n");

    return ret;
}


int snd_codec_remove(struct platform_device* pdev)
{
    snd_soc_unregister_codec(&pdev->dev);
    
    printk(KERN_ERR "snd codec dummy  remove ok!\n");
    return 0;
}


MODULE_AUTHOR("Huawei Technology Co., Ltd");
MODULE_DESCRIPTION("Balong ASoC SLIC Driver");
MODULE_LICENSE("GPL");

