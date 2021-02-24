/*
 *  sound/soc/balong/snd_module_machine.c
 *
 *  Copyright (c) 2009 Huawei Technology Co., Ltd
 *  Author: Huawei Technology Co., Ltd
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <linux/module.h>
#include <linux/string.h>
#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <asm/mach-types.h>
#include "snd_init.h"


static struct snd_soc_ops balong_snd_ops = {
    .hw_params = NULL,
};

static struct snd_soc_dai_link snd_machine_dai[] = {
    [0]={ /* Sound Playback */
        .name = "local_pcm",
        .stream_name = "playback_capture",
        .cpu_dai_name = "dma_pcm_dai",
        .codec_dai_name = "dummy_codec_dai",
        .platform_name = "snd_plat",
        .codec_name = "dummy_codec",
        .ops = &balong_snd_ops,
    },
    [1]={ /* Sound Playback */
        .name = "hifi_pcm",
        .stream_name = "playback_capture",
        .cpu_dai_name = "hifi_pcm_dai",
        .codec_dai_name = "dummy_codec_dai",
        .platform_name = "snd_module_plat",
        .codec_name = "dummy_codec",
        .ops = &balong_snd_ops,
    },
};

static int snd_machine_probe(struct snd_soc_card *card)
{
    return 0;
}


static int snd_machine_remove(struct snd_soc_card *card)
{
    printk(KERN_ERR "snd machine remove ok!\n");
	return 0;
}


static struct snd_soc_card snd_balong = {
    .name = "module_sound",
    .owner = THIS_MODULE,
    .dai_link = snd_machine_dai,
    .num_links = ARRAY_SIZE(snd_machine_dai),
    .probe = snd_machine_probe,
    .remove = snd_machine_remove,
};


int snd_module_machine_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct snd_soc_card *card = &snd_balong;

    card->dev = &pdev->dev;

	ret = snd_soc_register_card(card);
	if (ret < 0)
    {
		printk(KERN_ERR "snd_soc_register_card failed: %d\n", ret);
	}

    printk(KERN_ERR "snd_module_machine_probe ok.\n");

	return ret;
}


int snd_module_machine_remove(struct platform_device *pdev)
{
    int ret = 0;
    struct snd_soc_card *card = &snd_balong;

	ret = snd_soc_unregister_card(card);
	if (ret < 0)
    {
		printk(KERN_ERR "snd_m2m_machine_remove failed: %d\n", ret);
	}
	return 0;
}

MODULE_AUTHOR("Huawei Technology Co., Ltd");
MODULE_DESCRIPTION("Balong ASoC MACHINE Driver");
MODULE_LICENSE("GPL");


