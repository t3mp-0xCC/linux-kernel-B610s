/* sound/soc/balong/snd_module_dai.c
 *
 * ALSA SoC Pcm Layer
 *
 *  Copyright (c) 2009 Huawei Technology Co., Ltd
 *  Author: Huawei Technology Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <osl_spinlock.h>
#include <osl_sem.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <bsp_icc.h>
#include "bsp_nvim.h"
#include "drv_comm.h"
#include "bsp_memmap.h"
#include "sio.h"
#include "snd_ctl.h"
#include "snd_init.h"


#define  en_NV_SIO_VOICE_MASTER  30012
#define  sio_unlock_pll   0
#define  sio_locked_pll   1

/*lint -save -e43 -e959 -e958 */
struct dai_status
{
    int         pflag;
    int         cflag;
    osl_sem_id  dai_sem;
    spinlock_t  dai_lock;
};
/*lint -restore*/

struct pll_ctrl_str{
    int is_lock;
};

static struct dai_status soc_dai_status = {0};

void sio_set_pll_lock(int lock)
{
    int ret = 0;
    u32 channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_WPLL;
    struct pll_ctrl_str ctrlmsg = {0};

    ctrlmsg.is_lock = lock;
    printk(KERN_ERR "[sio_set_pll_lock] lock= %d!\n",lock);

    osl_sem_down(&soc_dai_status.dai_sem);

    ret = bsp_icc_send(ICC_CPU_MODEM, channel_id, (u8*)&ctrlmsg, sizeof(ctrlmsg));
    if (sizeof(ctrlmsg) != ret)
    {
         printk(KERN_ERR "[sio_set_pll_lock] send icc lock pll msg fail!\n");
    }

    osl_sem_up(&soc_dai_status.dai_sem);

    return;
}

void sio_mode_set(void)
{
    unsigned short sio_mode = 1;
    unsigned int   ret = 0;

    ret = bsp_nvm_read(en_NV_SIO_VOICE_MASTER,&sio_mode,sizeof(unsigned short));
    if(ret)
    {
        printk(KERN_ERR "[snd_machine_probe] read nv %d fail,set as master!\n",
                        en_NV_SIO_VOICE_MASTER);
    }

    if (1 == sio_mode)
    {
        /* 配置SIO为主模式 */
        sio_master_set(SIO_MASTER);
#if (ZSI_PCLK_8M == ZSI_PCLK_MODE)
        /* 时钟分频 245760k/120/256=8k bit0:15=30(pclk=8M) bit16:27=1024(sync=8K) */
        sio_pcm_div(8192000, 8000);
#elif (ZSI_PCLK_2M == ZSI_PCLK_MODE)
        /* 时钟分频 245760k/120/256=8k bit0:15=120(pclk=2M) bit16:27=256(sync=8K) */
        sio_pcm_div(2048000, 8000);
#endif
        printk(KERN_ERR "[snd_machine_probe]sio set as master!\n");
    }
    else
    {
        /* 配置SIO为从模式 */
        sio_master_set(SIO_SLAVE);
        printk(KERN_ERR "[snd_machine_probe]sio set as slave!\n");
    }
    return;
}

void sio_close(void)
{
    sio_reset();
    sio_clock_disable();
    printk(KERN_ERR "Enter this:%s!\n",__func__);
    return;
}

static int snd_soc_dai_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
    int stream = substream->stream;
    unsigned long flag_local = 0;

    spin_lock_irqsave(&soc_dai_status.dai_lock, flag_local);

    if( 0 == soc_dai_status.pflag && 0 == soc_dai_status.cflag )
    {
        sio_set_pll_lock(sio_locked_pll);
        sio_mode_set();
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
    if (0 == soc_dai_status.pflag && 0 == soc_dai_status.cflag)
    {
        sio_close();
        sio_set_pll_lock(sio_unlock_pll);
    }

    return;
}
static int snd_soc_dai_startup_dummy(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
    return 0;
}

static void snd_soc_dai_shutdown_dummy(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
    return;
}

static const struct snd_soc_dai_ops snd_pcm_dai_ops = {
    .startup    = snd_soc_dai_startup,
    .shutdown   = snd_soc_dai_shutdown,
};
static const struct snd_soc_dai_ops snd_hifi_pcm_dai_ops = {
    .startup    = snd_soc_dai_startup_dummy,
    .shutdown   = snd_soc_dai_shutdown_dummy,
};

static struct snd_soc_dai_driver snd_pcm_dai[] = {
    [0]={
        .name    = "dma_pcm_dai",
        .ops = &snd_pcm_dai_ops,
        .playback = {
            .channels_min    = 1,
            .channels_max    = 1,
            .rates        = SNDRV_PCM_RATE_8000,
            .formats      = SNDRV_PCM_FORMAT_S16_LE | SNDRV_PCM_FMTBIT_S16_LE,
        },
        .capture = {
            .channels_min    = 1,
            .channels_max    = 1,
            .rates        = SNDRV_PCM_RATE_8000,
            .formats      = SNDRV_PCM_FORMAT_S16_LE | SNDRV_PCM_FMTBIT_S16_LE,
        }
    },
    [1]={
        .name    = "hifi_pcm_dai",
        .ops = &snd_hifi_pcm_dai_ops,
        .playback = {
            .channels_min    = 1,
            .channels_max    = 1,
            .rates        = SNDRV_PCM_RATE_8000,
            .formats      = SNDRV_PCM_FORMAT_S16_LE | SNDRV_PCM_FMTBIT_S16_LE,
        },
        .capture = {
            .channels_min    = 1,
            .channels_max    = 1,
            .rates        = SNDRV_PCM_RATE_8000,
            .formats      = SNDRV_PCM_FORMAT_S16_LE | SNDRV_PCM_FMTBIT_S16_LE,
        }
    },
};

static const struct snd_soc_component_driver snd_pcm_component[] = {
    [0]={
        .name = "dma_pcm_dai",
    },
    [1]={
        .name = "hifi_pcm_dai",
    },
};


int snd_pcm_dev_probe(struct platform_device *pdev)
{
    int ret = 0;

    spin_lock_init(&soc_dai_status.dai_lock);
    osl_sem_init(1,&soc_dai_status.dai_sem);

    ret = snd_soc_register_component(&pdev->dev, &snd_pcm_component, &snd_pcm_dai, 2);
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



