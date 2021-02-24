/*
 * snd_slic.c  --  LE9662 ALSA Soc codec driver
 *
* Copyright (c) 1988-2014 by Huawei Technologies Co., Ltd. All rights reserved.
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
#include <linux/gpio.h>
#include <linux/of.h>
#include <asm/uaccess.h>
#include <asm/fcntl.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include "spi.h"
#include "slic_ctl.h"


static const struct snd_soc_dai_ops slic_pcm_dai_ops = {
	.hw_params	= NULL,
	.set_fmt	= NULL,
	.set_sysclk	= NULL,
	.set_pll    = NULL,
	.set_clkdiv = NULL,
};

static struct snd_soc_dai_driver slic_dai[] = {
    [0] = {
    		/* DAI PCM */
    		.name	= "slic_dai",
    		.id	= 0,
    		.playback = {
    			.stream_name	= "Playback",
    			.channels_min	= 1,
    			.channels_max	= 1,
    			.rates		= SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
    			.formats	= SNDRV_PCM_FORMAT_S16_LE,
    		},
    		.capture = {
    			.stream_name	= "Capture",
    			.channels_min	= 1,
    			.channels_max	= 1,
    			.rates		= SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
    			.formats	= SNDRV_PCM_FORMAT_S16_LE,
    		},
    		.ops	= &slic_pcm_dai_ops,
    	},
};
/*lint -save -e123*/
int slic_func_info(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BYTES;
	uinfo->count = MAX_DATA_SIZE;
    return 0;
}

int slic_func_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    return 0;
}

int slic_func_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    u32  cmd = 0;
    s32  err = 0;
    struct slic_ioctl* slic_ctl = slic_priv->slic_ctl;
   
    if( NULL == ucontrol)
    {
        printk(KERN_ERR"Error:invalid ucontrol para\n");
        return -EINVAL;
    }

    if( NULL == slic_ctl)
    {
        printk(KERN_ERR"Error:invalid slic_ctl para\n");
        return -EINVAL;
    }

    osl_sem_down(&slic_priv->ioctl_sem); 
    
    memcpy(slic_ctl,(u8*)ucontrol->value.bytes.data,sizeof(struct slic_ioctl)); 
    cmd= ucontrol->id.numid;
    err = slic_cmd_ctrl(cmd,slic_ctl);
    
    osl_sem_up(&slic_priv->ioctl_sem);
    
    return err;
}
/*lint -restore*/


static int slic_codec_probe(struct snd_soc_codec *codec)
{
    int err = 0;
    if( NULL != codec )
    {
        err = slic_probe(codec);
    }
    
    return err;
}

static int  slic_codec_remove(struct snd_soc_codec *codec)
{
    int err = slic_remove();
    
    return err;
}


static struct snd_kcontrol_new slic_controls[SLIC_CTRL_COUNT];

static struct snd_soc_codec_driver soc_codec_dev_slic = {
	.probe =	slic_codec_probe,
	.remove =	slic_codec_remove,
	.controls = slic_controls,
	.num_controls = SLIC_CTRL_COUNT,

};

void snd_slic_unreset(void)
{
	int ret = 0;
	unsigned int gpio = 0;

	struct device_node *dev = NULL;
	const char name[] = "hisilicon,slic_app";

	dev = of_find_compatible_node(NULL, NULL, name);
	if(NULL == dev)
	{
		printk(KERN_ERR "slic device node not found\n");
		return ;
	}
	ret = of_property_read_u32(dev, "gpio_num", &gpio);
	if (ret < 0)
	{
		printk(KERN_ERR "slic gpio_num error, ret %d.\n", ret);
		return ;
	}

	if(0 != gpio)
	{
		(void)gpio_direction_output(gpio, 1);

	}

}

int snd_codec_probe(struct platform_device *pdev)
{
    int ret = 0;
    u32  i  = 0;
    static const u8 name[] = "slic_ctl";
    
    memset(slic_controls, 0x0, sizeof(slic_controls));    
    for( i = 0;i < SLIC_CTRL_COUNT; i++ )
    {
        slic_controls[i].name   = name;
        slic_controls[i].iface  = SNDRV_CTL_ELEM_TYPE_BYTES;  
        slic_controls[i].device = i;
    	slic_controls[i].access = SNDRV_CTL_ELEM_ACCESS_TLV_READ | SNDRV_CTL_ELEM_ACCESS_READWRITE;
    	slic_controls[i].info   = slic_func_info;
    	slic_controls[i].get    = slic_func_get;
    	slic_controls[i].put    = slic_func_put;        
    }
    
	ret = snd_soc_register_codec(&pdev->dev, &soc_codec_dev_slic,
				     slic_dai, ARRAY_SIZE(slic_dai));
	if (ret) 
    {
		printk(KERN_ERR "Failed to register codec\n");
		return ret;
	}

	platform_set_drvdata(pdev, NULL); /* just as you want */    
    slic_spi_init(0);
    snd_slic_unreset();
    printk(KERN_ERR "snd slic probe ok!\n");
    
	return ret;
}


int snd_codec_remove(struct platform_device *pdev)
{
    snd_soc_unregister_codec(&pdev->dev);
    printk(KERN_ERR "snd slic remove ok!\n");
	return 0;
}


MODULE_AUTHOR("Huawei Technology Co., Ltd");
MODULE_DESCRIPTION("Balong ASoC SLIC Driver");
MODULE_LICENSE("GPL");

