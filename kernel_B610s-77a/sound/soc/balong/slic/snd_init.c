#include <linux/module.h>
#include <sound/soc.h>
#include "snd_init.h"
#include "snd_param.h"
#include "bsp_sram.h"
#include "mbb_config.h"
#include "product_config.h"
#include <linux/delay.h>

#define SND_DEV_NUM 4

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
        .probe = snd_codec_probe,
        .remove = snd_codec_remove,
        .driver = {
        	.name  = "snd_slic",
        	.owner = THIS_MODULE,
        }
    },
    [3]={
        .probe = snd_balong_platform_probe,
        .remove = snd_balong_platform_remove,
        .driver = {
        	.name  = "snd_machine",
        	.owner = THIS_MODULE,
        }
    }

};

static const struct platform_device snd_balong_dev[SND_DEV_NUM]=
{
    [0]={
        .name	= "pcm_dai",
        .id   	= -1,
        .dev    ={
            .release = snd_dev_release,
        }
    },
    [1]={
        .name	= "snd_plat",
        .id	    = -1,
        .dev    ={
            .release = snd_dev_release,
        }
    },
    [2]={
        .name	= "snd_slic",
        .id	    = -1,
        .dev    ={
            .release = snd_dev_release,
        }
    },
    [3]={
        .name	= "snd_machine",
        .id	    = -1,
        .dev    ={
            .release = snd_dev_release,
        }
    },
};

static struct platform_device snd_dev[SND_DEV_NUM];
static struct platform_driver snd_drv[SND_DEV_NUM];

/*lint -save -e21*/
#define AUDIO_STR_LEN 16
static char sample_rate_param[AUDIO_STR_LEN]  = "8000";
module_param_string(sample_rate_param, sample_rate_param, AUDIO_STR_LEN, S_IRUGO);
MODULE_PARM_DESC(sample_rate_param, "sample rate pass from app");
/*lint -restore*/

int snd_balong_init(void)
{
    int ret = 0;
    int i   = 0;
    unsigned int rate = SND_SAMPLE_RATE_8K;

#if (FEATURE_ON == MBB_FACTORY)
    /* 750平台C核启动慢，slic在A启动很快导致pll异常，由于此时加载时还是单进程未到用户态，此处进行延时 */
    mdelay(500);
#endif	

#if (FEATURE_ON == MBB_DLOAD)
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;

    if (NULL == smem_data)
    {
        printk(KERN_ERR "Dload smem_data malloc fail!\n");
        return -1;
    }

    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        return ret;
    }
#endif /*MBB_DLOAD*/
    memcpy(snd_dev, snd_balong_dev, sizeof(snd_balong_dev));
    memcpy(snd_drv, snd_balong_drv, sizeof(snd_balong_drv));

	if(!strcmp(sample_rate_param, "8000"))
	{
		rate = SND_SAMPLE_RATE_8K;
	}
	else if(!strcmp(sample_rate_param, "8000w"))
	{
		rate = SND_SAMPLE_RATE_WIDE_8K;
	}
	else if(!strcmp(sample_rate_param, "16000"))
	{
		rate = SND_SAMPLE_RATE_16K;
	}
	else
	{
		printk(KERN_ERR "slic sample_rate_param=%s error, please check!\n", sample_rate_param);
		return -1;
	}
    
    snd_sample_rate_set(rate);
    snd_master_slave_set(SND_SOC_MASTER);
    snd_data_mode_set(SND_PCM_MODE);
    snd_xrun_mode_set(SND_XRUN_SILENCE);

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

#if (FEATURE_ON == MBB_FACTORY)
late_initcall(snd_balong_init);
#else
module_init(snd_balong_init);
#endif
module_exit(snd_balong_exit);


MODULE_AUTHOR("Huawei Technology Co., Ltd");
MODULE_DESCRIPTION("Balong ASoC Driver");
MODULE_LICENSE("GPL");

