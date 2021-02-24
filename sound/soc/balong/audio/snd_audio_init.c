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
#include <sound/soc.h>
#include <linux/wakelock.h>
#include <linux/i2c.h>

#include <bsp_version.h>
#include "snd_init.h"
#include "snd_audio_machine.h"
#include "snd_param.h"
#include "bsp_pmu.h"

#define SND_DEV_NUM 3

void snd_dev_release(struct device *dev)
{
    return;
}

static const struct platform_driver snd_balong_drv[SND_DEV_NUM]=
{
    [0]={
        .probe = snd_pcm_dev_probe,
        .remove = snd_pcm_dev_remove,
        .driver = {
            .name  = "pcm_dai",
            .owner = THIS_MODULE,
        }
    },
    [1]={
        .probe = snd_platform_probe,
        .remove = snd_platform_remove,
        .driver = {
            .name  = "snd_plat",
            .owner = THIS_MODULE,
        }
    },
    [2]={
        .probe = snd_balong_platform_probe,
        .remove = snd_balong_platform_remove,
        .driver = {
            .name  = "snd_audio",
            .owner = THIS_MODULE,
        }
    },

};

static const struct platform_device snd_balong_dev[SND_DEV_NUM]=
{
    [0]={
        .name    = "pcm_dai",
        .id       = -1,
        .dev    ={
            .release = snd_dev_release,
        }
    },
    [1]={
        .name    = "snd_plat",
        .id        = -1,
        .dev    ={
            .release = snd_dev_release,
        }
    },
    [2]={
        .name    = "snd_audio",
        .id        = -1,
        .dev    ={
            .release = snd_dev_release,
        }
    },
};

static struct platform_device snd_dev[SND_DEV_NUM];
static struct platform_driver snd_drv[SND_DEV_NUM];

static struct i2c_board_info audio_board_info = {
    .type = "wm8990",
    .addr = 0x1a,
    .platform_data = NULL,
};

void snd_audio_init(void)
{

    struct i2c_client * audio_client = NULL;
    struct i2c_adapter *adapter = NULL;
    unsigned int i2c_num = 0;

#ifdef CONFIG_OF
    const void *property = NULL;
    char name[20] = "";

    int len = 0x4;
    int ret = 0;

    struct device_node *device = NULL;
    const char node_name[] = "hisilicon,audio_balong_app";


    memset(&audio_board_info, 0, sizeof(struct i2c_board_info));

    device = of_find_compatible_node(NULL, NULL, node_name);
    if(!device)
    {
        printk(KERN_ERR "audio device node is null.\n");
        return ;
    }

    property = of_get_property(device, "type", NULL);

    len = strlen((char*)property);
    strncpy(audio_board_info.type, property, len + 1);

    ret = of_property_read_u32(device, "addr", (u32*)&audio_board_info.addr);
    if (ret < 0)
    {
        printk(KERN_ERR "audio addr error, ret %d.\n", ret);
        return ;
    }

    ret = of_property_read_u32(device, "i2c_num", &i2c_num);
    if (ret < 0)
    {
        printk(KERN_ERR "audio i2c_num error, ret %d.\n", ret);
        return ;
    }

    snprintf(name, 20, "%s.%u-%04x",audio_board_info.type, i2c_num, audio_board_info.addr);

    if(0 != (ret = snd_machine_codec_name_set(name)))
    {
        printk(KERN_ERR "machine codec name set fail, ret %d.\n", ret);
        return ;
    }

    /* audio config */
    printk(KERN_INFO "audio type=%s, addr=0x%x, i2c_num=0x%x, machine codec name=%s.\n",
                audio_board_info.type, (unsigned int)audio_board_info.addr, (unsigned int)i2c_num, name);
#else
    i2c_num = 0;
#endif

    adapter = i2c_get_adapter(i2c_num);
    if (!adapter)
    {
        printk(KERN_ERR "i2c_get_adapter failed.\n");
        return ;
    }

    audio_client = i2c_new_device(adapter, &audio_board_info);
    if (!audio_client) {
        printk(KERN_ERR "i2c_new_device failed, addr = 0x%x.\n", audio_board_info.addr);
        return ;
    }
    printk(KERN_ERR "snd_audio_init ok!\n");

}

/*lint -save -e21*/
#define AUDIO_STR_LEN 16
static char sample_rate_param[AUDIO_STR_LEN]  = "8000";
module_param_string(sample_rate_param, sample_rate_param, AUDIO_STR_LEN, S_IRUGO);
MODULE_PARM_DESC(sample_rate_param, "sample rate pass from app");

static char pcm_i2s_param[AUDIO_STR_LEN]  = "pcm";
module_param_string(pcm_i2s_param, pcm_i2s_param, AUDIO_STR_LEN, S_IRUGO);
MODULE_PARM_DESC(pcm_i2s_param, "pcm/i2s mode pass from app");

static char master_slave_param[AUDIO_STR_LEN]  = "slave";
module_param_string(master_slave_param, master_slave_param, AUDIO_STR_LEN, S_IRUGO);
MODULE_PARM_DESC(master_slave_param, "master/slave mode pass from app");
/*lint -restore*/
extern void pinctrl_audio_resume(void);
int snd_balong_init(void)
{
    int ret = 0;
    int i   = 0;
    unsigned int version = bsp_get_version_info()->chip_type;
    unsigned int rate, pcm_i2s, master_slave;
    static struct wake_lock audio_wake_lock;

    if(!strcmp(sample_rate_param, "8000"))
    {
        rate = SND_SAMPLE_RATE_8K;
    }
    else if(!strcmp(sample_rate_param, "16000"))
    {
        rate = SND_SAMPLE_RATE_16K;
    }
    else
    {
        printk(KERN_ERR "audio sample_rate_param=%s error, please check!\n", sample_rate_param);
        return -1;
    }

    if(!strcmp(pcm_i2s_param, "pcm"))
    {
        pcm_i2s = SND_PCM_MODE;
    }
    else if(!strcmp(pcm_i2s_param, "i2s"))
    {
        pcm_i2s = SND_I2S_MODE;
    }
    else
    {
        printk(KERN_ERR "audio pcm_i2s_param=%s error, please check!\n", pcm_i2s_param);
        return -1;
    }

    if(!strcmp(master_slave_param, "master"))
    {
        master_slave = SND_SOC_MASTER;
    }
    else if(!strcmp(master_slave_param, "slave"))
    {
        master_slave = SND_SOC_SLAVE;
    }
    else
    {
        printk(KERN_ERR "audio master_slave_param=%s error, please check!\n", master_slave_param);
        return -1;
    }

    pinctrl_audio_resume();

    snd_sample_rate_set(rate);
    snd_master_slave_set(master_slave);
    snd_data_mode_set(pcm_i2s);
    snd_xrun_mode_set(SND_XRUN_SILENCE);

    snd_audio_init();

    if(CHIP_V750 == version)
        bsp_pmu_xo_clk_enable(0); /* 与王霞确认此处参数0为打开AUDIO 19.2M时钟门控 */

    memcpy(snd_dev, snd_balong_dev, sizeof(snd_balong_dev));
    memcpy(snd_drv, snd_balong_drv, sizeof(snd_balong_drv));

    for(i = 0;i < SND_DEV_NUM;i++)
    {
        ret = platform_device_register(&snd_dev[i]);
        if(ret)
        {
            printk(KERN_ERR "pcm failed to register platform device!\n");
            goto dev_fail;
        }

        ret = platform_driver_register(&snd_drv[i]);
        if(ret)
        {
            printk(KERN_ERR "pcm balong failed to register platform driver!\n");
            goto drv_fail;
        }

    }

    wake_lock_init(&audio_wake_lock, WAKE_LOCK_SUSPEND, "AUDIO WM8990");
    wake_lock(&audio_wake_lock);

    printk(KERN_ERR "sound machine init ok!\n");

    return 0;

drv_fail:
    platform_device_unregister(&snd_dev[i]);

dev_fail:
    while (--i >= 0)
    {
        platform_driver_unregister(&snd_drv[i]);
        platform_device_unregister(&snd_dev[i]);
    };
    printk(KERN_ERR "sound machine init fail!\n");
    return ret;
}

void snd_balong_exit(void)
{
    int i = SND_DEV_NUM;
    while (--i >= 0)
    {
        platform_driver_unregister(&snd_drv[i]);
        platform_device_unregister(&snd_dev[i]);
    };
    printk(KERN_ERR "sound balong exit ok!\n");
}

module_init(snd_balong_init);
module_exit(snd_balong_exit);


MODULE_AUTHOR("Huawei Technology Co., Ltd");
MODULE_DESCRIPTION("Balong ASoC Driver");
MODULE_LICENSE("GPL");

