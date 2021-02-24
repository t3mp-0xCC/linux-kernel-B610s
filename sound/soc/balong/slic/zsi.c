/*lint --e{537}*/
#include <linux/clk.h>
//#include "sysctrl_interface.h"
#include "snd_param.h"
#include "zsi.h"
#include "sio.h"

void zsi_clock_enable(void)
{
    struct clk *zsi_clk = clk_get(NULL, "zsi_clk");
    if (IS_ERR(zsi_clk))
    {
        printk(KERN_ERR "zsi_clk get fail.\n");
        return;
    }

    (void)clk_prepare(zsi_clk);
    (void)clk_enable(zsi_clk);
}


void zsi_clk_cfg(unsigned int p, unsigned int fs)
{

    /* ʱ�ӷ�Ƶ 245760k/120/256=8k bit0:15=30(pclk=8M) bit16:27=1024(sync=8K) */
    sio_pcm_div(p, fs);
    /* ��ZSI/SIO����ʱ���ſ� */
    sio_clock_enable();

    /* V7R5����ZSIʱ��ʱ���ſ� */
    zsi_clock_enable();

    /* ����SIOΪ��ģʽ */
    sio_master_set(SND_SOC_MASTER);

    /* ����SIOΪPCMģʽ */
    sio_pcm_mode_set();

}

void zsi_enable(void)
{
    /* ����ZSIʹ�� */
    sio_zsi_set(ZSI_MODE);

}

void zsi_clk_disable(void)
{
    /* ����ZSIʹ�� */
   sio_clock_disable();

}



