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

#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <osl_spinlock.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include "drv_comm.h"
#include "bsp_memmap.h"
#include "sio.h"

/*lint -save -e43 -e959 -e958 */
struct dai_status
{
    int         pflag;
    int         cflag;
    spinlock_t	dai_lock;   
};
/*lint -restore*/

static struct dai_status soc_dai_status = {0};

static int snd_soc_dai_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
    int stream = substream->stream;
    unsigned long flag_local = 0;
    
	spin_lock_irqsave(&soc_dai_status.dai_lock, flag_local);
    
    if( 0 == soc_dai_status.pflag && 0 == soc_dai_status.cflag )
    {
        sio_init();
    }
    
    if( SNDRV_PCM_STREAM_PLAYBACK == stream )
    {
        soc_dai_status.pflag = 1;
    }
    else
    {
        soc_dai_status.cflag = 1;
    }
    
   	spin_unlock_irqrestore(&soc_dai_status.dai_lock, flag_local);
    
    return 0;
}

static void snd_soc_dai_shutdown(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
    int stream = substream->stream;
    
    if( SNDRV_PCM_STREAM_PLAYBACK == stream )
    {
        soc_dai_status.pflag = 0;
    }
    else
    {
        soc_dai_status.cflag = 0;
    }

    return;
}


static const struct snd_soc_dai_ops snd_pcm_dai_ops = {
    .startup    = snd_soc_dai_startup,
    .shutdown   = snd_soc_dai_shutdown,
};


static struct snd_soc_dai_driver snd_pcm_dai = {
	.name	= "pcm_dai",					
	.ops = &snd_pcm_dai_ops,
	.playback = {						
		.channels_min	= 1,				
		.channels_max	= 1,				
		.rates		= SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
		.formats	= SNDRV_PCM_FORMAT_S16_LE | SNDRV_PCM_FMTBIT_S16_LE,	
	},							
	.capture = {						
		.channels_min	= 1,				
		.channels_max	= 1,
		.rates		= SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
		.formats	= SNDRV_PCM_FORMAT_S16_LE | SNDRV_PCM_FMTBIT_S16_LE,
	}

};

static const struct snd_soc_component_driver snd_pcm_component = {
	.name		= "pcm_dai",
};


int snd_pcm_dev_probe(struct platform_device *pdev)
{
	int ret = 0;

    spin_lock_init(&soc_dai_status.dai_lock);
    
    ret = snd_soc_register_component(&pdev->dev, &snd_pcm_component, &snd_pcm_dai, 1);
	if (ret < 0) {
		printk(KERN_ERR "failed to get register DAI: %d\n", ret);
        return ret;
	}
    
    printk(KERN_ERR "snd pcm probe ok!\n");

	return ret;
}

int snd_pcm_dev_remove(struct platform_device *pdev)
{
    snd_soc_unregister_component(&pdev->dev);
    printk(KERN_ERR "snd pcm dev remove ok!\n");
	return 0;
}


/* Module information */
MODULE_AUTHOR("Huawei Technology Co., Ltd");
MODULE_DESCRIPTION("Balong ASoC PCM Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:balong-pcm");



