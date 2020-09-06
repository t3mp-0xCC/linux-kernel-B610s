/*
 *  sound/soc/balong/snd_machine.c
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
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <asm/mach-types.h>

#include "zsi.h"
#include "snd_param.h"

/* LE9662 BALONG DAI operations */
static struct snd_soc_ops balong_snd_ops = {
    .hw_params = NULL,

};


static struct snd_soc_dai_link snd_machine_dai[] = {
    [0]={ /* Sound Playback */
        .name = "balong voip",
        .stream_name = "playback_capture",
        .cpu_dai_name = "pcm_dai",
        .codec_dai_name = "slic_dai",
        .platform_name = "snd_plat",
        .codec_name = "snd_slic",
        .ops = &balong_snd_ops,
    },
};

static int snd_machine_probe(struct snd_soc_card *card)
{
    zsi_clk_cfg(8192000, SND_SAMPLE_RATE_16K == snd_sample_rate_get() ? 16000 : 8000 );
    zsi_enable();

    return 0;
}


static int snd_machine_remove(struct snd_soc_card *card)
{
	zsi_clk_disable();
    printk(KERN_ERR "snd machine remove ok!\n");
	return 0;
}


static struct snd_soc_card snd_balong = {
    .name = "balong voip",
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
		return ret;
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
		return ret;

	}
	return 0;
}

MODULE_AUTHOR("Huawei Technology Co., Ltd");
MODULE_DESCRIPTION("Balong ASoC MACHINE Driver");
MODULE_LICENSE("GPL");


