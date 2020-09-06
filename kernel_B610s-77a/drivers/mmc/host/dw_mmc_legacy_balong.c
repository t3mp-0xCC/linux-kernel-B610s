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
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/mmc/host.h>
#include <linux/mmc/dw_mmc.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <drv_nv_def.h>
#include <drv_nv_id.h>
#include <bsp_version.h>
#include <bsp_sysctrl.h>
#include <bsp_nvim.h>

#include <dw_mmc.h>
#include <dw_mmc-pltfm.h>
#include <hi_mmc.h>
#define DRIVER_NAME "mmc_driver"
#define MMC_FEATURE_NV              NV_ID_DRV_MODULE_SUPPORT
//#define SD_GPIO_DETECT
#define SDMMC_CMD_USE_HOLD_REG        BIT(29)
#define SDMMC_TYPE_SD                 (1)
#define SDMMC_TYPE_SDIO               (0)
#define SDMMC_TYPE                    SDMMC_TYPE_SD

#define MMC_CLOCK_SOURCE_480M        (480000000)
#define MMC_CLOCK_SOURCE_600M        (600000000)
#define MMC_CLOCK_SOURCE_700M        (700000000)
#define MMC_CLOCK_SOURCE_800M        (800000000)
#define MMC_CLOCK_SOURCE_25M         (25000000)
#define MMC_CLOCK_SOURCE_20M         (20000000)
#define MMC_CLOCK_SOURCE_15M         (15000000)

#define SDMMC_CRG_MMC_CTL_PARM             (6)
#define SDMMC_CRG_MMC_CTL_SAMPLE_CTL(x)    (((x) & 1) << 15)
#define SDMMC_CRG_MMC_CTL_SAMPLE_DELAY(x)  (((x) & 3) << 13)
#define SDMMC_CRG_MMC_CTL_SAMPLE_STEP(x)   (((x) & 0x1F) << 8)

#define SDMMC_CRG_MMC_CTL_SAMPLE(x, y, z) \
        SDMMC_CRG_MMC_CTL_SAMPLE_CTL(x) | SDMMC_CRG_MMC_CTL_SAMPLE_DELAY(y) |\
        SDMMC_CRG_MMC_CTL_SAMPLE_STEP(z)

#define SDMMC_CRG_MMC_CTL_BYPASS(x)    (((x) & 1) << 7)

#define SDMMC_CRG_MMC_CTL_DRIVE_DELAY(x)    (((x) & 3) << 5)
#define SDMMC_CRG_MMC_CTL_DRIVE_STEP(x)    ((x) & 0x1F)

#define SDMMC_CRG_MMC_CTL_DRIVE(x, y) \
        SDMMC_CRG_MMC_CTL_DRIVE_DELAY(x) | SDMMC_CRG_MMC_CTL_DRIVE_STEP(y)

#define SDMMC_CRG_MMC_CTL(x, y, z, l, m, n) \
        SDMMC_CRG_MMC_CTL_SAMPLE(x, y, z) | SDMMC_CRG_MMC_CTL_BYPASS(l) | \
        SDMMC_CRG_MMC_CTL_DRIVE(m, n)



struct dw_mci_balong_state {
    u32        io_volt;
};
/* Balong implementation specific driver private data */
struct dw_mci_balong_priv_data {
    int                idx;    
    int                gpio_cd;
    u32                delay_chain_select_bits;
    u32                shift_step_select_bits;
    u32                shift_step_select_vaild_bits;
    u32             clock;    
    u32             cd_vol;
    u32             mmc_idx;
    struct clk        *mux_refclk;
    struct regulator    *ip_vmmc;
    struct regulator    *io_vmmc;
    struct dw_mci_balong_state    state;
};
/* mmc nv structure ... */
struct himci_support_nv {
    unsigned int   support;
};

void syssc_mmc0_ctrl(unsigned int value, struct device_node *np);
void syssc_mmc1_ctrl(unsigned int value, struct device_node *np);
void syssc_mmc0_sample_tuning_enable(unsigned int value, struct device_node *np);
void syssc_mmc1_sample_tuning_enable(unsigned int value, struct device_node *np);
void syssc_mmc0_sample_sel(unsigned int value, struct device_node *np);
void syssc_mmc1_sample_sel(unsigned int value, struct device_node *np);
void pinctrl_mmc_suspend(void);
void pinctrl_mmc_resume(void);

#ifdef HI6932_MMC
static unsigned int mmc0_timing[][SDMMC_CRG_MMC_CTL_PARM] = {
    { 0, 1, 2, 0, 0, 15 },/*400k*/
    { 0, 1, 2, 0, 0, 15 },/*hs 25M*/
    { 0, 1, 2, 0, 0, 4 },
    { 0, 1, 2, 0, 0, 4 },
    { 0, 1, 2, 0, 0, 4 },
    { 0, 1, 5, 0, 0, 4 },
    { 0, 0, 0, 0, 0, 4 },
};

static unsigned int mmc1_timing[][SDMMC_CRG_MMC_CTL_PARM] = {
    { 0, 2, 2, 0, 0, 15 },
    { 0, 2, 2, 0, 0, 15 },
    { 0, 2, 2, 0, 0, 4 },
    { 0, 2, 2, 0, 0, 4 },
    { 0, 2, 2, 0, 0, 4 },
    { 0, 2, 2, 0, 0, 4 },
    { 0, 0, 0, 0, 0, 4 },
};
#endif

#ifdef HI6950_MMC
static unsigned int mmc0_timing[][SDMMC_CRG_MMC_CTL_PARM] = {
    { 0, 1, 2, 0, 0, 15 },/*400k*/
    { 0, 1, 2, 0, 0, 15 },/*hs 25M*/
    { 0, 1, 2, 0, 0, 4 },
    { 0, 1, 2, 0, 0, 4 },
    { 0, 1, 2, 0, 0, 4 },
    { 0, 1, 3, 0, 0, 4 },
    { 0, 0, 0, 0, 0, 4 },
};

static unsigned int mmc1_timing[][SDMMC_CRG_MMC_CTL_PARM] = {
    { 0, 2, 2, 0, 0, 15 },
    { 0, 2, 2, 0, 0, 15 },
    { 0, 2, 2, 0, 0, 4 },
    { 0, 2, 2, 0, 0, 4 },
    { 0, 2, 2, 0, 0, 4 },
    { 0, 2, 2, 0, 0, 4 },
    { 0, 0, 0, 0, 0, 4 },
};
#endif

static unsigned int **mmc_timing[SDMMC_TYPE + 1] = {(unsigned int **)mmc0_timing, (unsigned int **)mmc1_timing, };
static void (*sc_mmc_ctrl[SDMMC_TYPE + 1])(unsigned int,struct device_node *) = {syssc_mmc0_ctrl, syssc_mmc1_ctrl, };
static void (*sc_mmc_tuning_enable[SDMMC_TYPE + 1])(unsigned int,struct device_node *) 
            = {syssc_mmc0_sample_tuning_enable, syssc_mmc1_sample_tuning_enable, };

static void (*sc_mmc_sample_sel[SDMMC_TYPE + 1])(unsigned int,struct device_node *) 
            = {syssc_mmc0_sample_sel, syssc_mmc1_sample_sel, };
static struct himci_support_nv g_mmc_nv = {
    .support = 0,
};

static void syssc_mmc_set(const struct device_node *np, const char *propname,unsigned int value)
{
    u32 para_value[3];
    u32 bitnum;
    void *base;
    u32 reg = 0;
    int ret;
    ret = of_property_read_u32_array(np, propname, para_value, 3);
    if(ret){
        printk("skip find of [%s]\n", propname);
        return ;
    }
    bitnum = para_value[2] - para_value[1] + 1;
    base = bsp_sysctrl_addr_get((void *)para_value[0]);
    if(NULL == base){
        printk("sysctrl base addr is null !\n");
        return ;
    }
    reg = readl(base);
    reg &= ~(((1<<bitnum)-1)<<para_value[1]);
    reg |= (value << para_value[1]);
    writel(reg, base);
}
void syssc_mmc0_srst_en(unsigned int value, struct device_node *np)
{
    syssc_mmc_set(np, "mmc0_srst_en", value);
}
void syssc_mmc0_srst_dis(unsigned int value, struct device_node *np)
{
    syssc_mmc_set(np, "mmc0_srst_dis", value);
}
void syssc_mmc0_sample_sel(unsigned int value, struct device_node *np)
{
    syssc_mmc_set(np, "mmc0_sample_sel", value);
}
void syssc_mmc0_clk_bypass(unsigned int value, struct device_node *np)
{
    syssc_mmc_set(np, "mmc0_clk_bypass", value);
}
void syssc_mmc0_ctrl(unsigned int value, struct device_node *np)
{
    syssc_mmc_set(np, "mmc0_ctrl", value);
}
void syssc_mmc0_sample_tuning_enable(unsigned int value, struct device_node *np)
{
    syssc_mmc_set(np, "mmc0_sample_tuning_enable", value);
}
void syssc_mmc_msc(unsigned int value, struct device_node *np)
{
    syssc_mmc_set(np, "mmc_msc", value);
}
void syssc_mmc1_srst_en(unsigned int value, struct device_node *np)
{
    syssc_mmc_set(np, "mmc1_srst_en", value);
}
void syssc_mmc1_srst_dis(unsigned int value, struct device_node *np)
{
    syssc_mmc_set(np, "mmc1_srst_dis", value);
}
void syssc_mmc1_sample_sel(unsigned int value, struct device_node *np)
{
    syssc_mmc_set(np, "mmc1_sample_sel", value);
}
void syssc_mmc1_clk_bypass(unsigned int value, struct device_node *np)
{
    syssc_mmc_set(np, "mmc1_clk_bypass", value);
}
void syssc_mmc1_ctrl(unsigned int value, struct device_node *np)
{
    syssc_mmc_set(np, "mmc1_ctrl", value);
}
void syssc_mmc1_sample_tuning_enable(unsigned int value, struct device_node *np)
{
    syssc_mmc_set(np, "mmc1_sample_tuning_enable", value);
}

static int dw_mci_balong_refclk_sel(struct device *dev,struct clk* refclk)
{
    struct clk *pclk = NULL;
    int ret;
    
    pclk = devm_clk_get(dev, "parent");
    if (IS_ERR(pclk)) {
        dev_dbg(dev, "get parent clock fail\n");
        ret = PTR_ERR(pclk);
    } else {
        ret = clk_set_parent(refclk, pclk);
    }
    

    return ret;

}

static void dw_mci_balong_pin_suspend(void)
{
   pinctrl_mmc_suspend();

}

static void dw_mci_balong_pin_resume(struct dw_mci_slot *slot)
{
    struct dw_mci        *host = slot->host;
    struct dw_mci_balong_priv_data *priv = NULL;
    
    if (host) {
        priv = host->priv;
    }
    if (priv && (SDMMC_TYPE_SD == priv->idx)) {
        pinctrl_mmc_resume();
    }

}
#ifdef SD_GPIO_DETECT
static int dw_mci_balong_get_cd(struct dw_mci *host, u32 slot_id)
{
    unsigned int status;
    struct dw_mci_balong_priv_data *priv = host->priv;

    if(priv->cd_vol)  /* cd_vol = 1 means sdcard gpio detect pin active-high */
        status = !gpio_get_value(priv->gpio_cd);
    else  /* cd_vol = 0 means sdcard gpio detect pin active-low */
        status = gpio_get_value(priv->gpio_cd);

    dev_info(host->dev," sd status = %d\n", status);
    return status;
}

static irqreturn_t dw_mci_balong_card_detect(int irq, void *data)
{
    struct dw_mci *host = (struct dw_mci *)data;

    /*dw_mci_work_routine_card*/
    queue_work(host->card_workqueue, &host->card_work);
    return IRQ_HANDLED;
};
static int dw_mci_balong_card_detect_init(struct dw_mci *host)
{
    struct device_node *np = host->dev->of_node;    
    struct dw_mci_balong_priv_data *priv = host->priv;
    int gpio;
    int err;

    if (host->pdata->quirks & DW_MCI_QUIRK_BROKEN_CARD_DETECTION)
        return 0;

    if (priv && (SDMMC_TYPE_SD != priv->idx)) {
        return 0;
    }

    err = of_property_read_u32(np, "cd-gpio", &gpio);
    if (err) {
        dev_err(host->dev, "get cd-gpio num fail,err:%d\n", err);
        return err;
    }
    if (gpio_is_valid(gpio)) {
        err = gpio_request(gpio, "dw-mci-cd");
        if (err < 0) {
            dev_err(host->dev, "gpio [%d] request failed\n", gpio);
            return err;
        }
        
        (void)gpio_direction_input(gpio);
        priv->gpio_cd = gpio;
        host->pdata->get_cd = dw_mci_balong_get_cd;
        err = devm_request_irq(host->dev, gpio_to_irq(gpio),
            dw_mci_balong_card_detect,
            IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND | IRQF_SHARED,
            DRIVER_NAME, host);
        if (err)
            dev_err(host->dev, "request gpio irq error\n");

    } else {
        dev_info(host->dev, "cd gpio not available");
    }

    return 0;
}
#endif
static int dw_mci_balong_priv_init(struct dw_mci *host)
{
    struct device_node *np = host->dev->of_node;
    struct dw_mci_balong_priv_data *priv;
    int err = 0;


    priv = devm_kzalloc(host->dev, sizeof(*priv), GFP_KERNEL);
    if (!priv) {
        dev_err(host->dev, "mem alloc failed for private data\n");
        return -ENOMEM;
    }
    
    priv->mux_refclk = devm_clk_get(host->dev, "mux");
    if (IS_ERR(priv->mux_refclk)) {
        dev_err(host->dev, "mmc1_mux_refclk not available\n");
        return -ENOMEDIUM;
    }

    err = of_property_match_string(np, "mmc-type", "sd");
    if (err < 0) {
        err = of_property_match_string(np, "mmc-type", "sdio");
        if (err < 0) {
            dev_err(host->dev, "mmc-type property not found\n");
            return -ENODEV;
        } else {
            priv->idx = SDMMC_TYPE_SDIO;
        }
    } else {
        priv->idx = SDMMC_TYPE_SD;
    }
    
    priv->ip_vmmc = devm_regulator_get(host->dev, "sd_mtcmos-vcc");
    if (IS_ERR(priv->ip_vmmc)) {
        dev_err(host->dev, "no sd_mtcmos-vcc regulator found\n");
        priv->ip_vmmc = NULL;
    } else {
        if(!regulator_is_enabled(priv->ip_vmmc)){
            err = regulator_enable(priv->ip_vmmc);
            if (err) {
                dev_err(host->dev,
                    "failed to enable regulator: %d\n", err);
                return -ENOMEDIUM;
            }
        }
    }
   
    priv->io_vmmc = devm_regulator_get(host->dev, "sd_io-vcc");
    if (IS_ERR(priv->io_vmmc)) {
        dev_err(host->dev, "no sd_io-vcc regulator found\n");
        priv->io_vmmc = NULL;
    } else {
        syssc_mmc_msc(SDMMC_TYPE_SD == priv->idx ? 0 : 1, host->dev->of_node);
        err = regulator_set_voltage(priv->io_vmmc, 
            SDMMC_TYPE_SD == priv->idx ? 2850000 : 1800000, 
            SDMMC_TYPE_SD == priv->idx ? 2850000 : 1800000);
        if (!regulator_is_enabled(priv->io_vmmc)) {
            err = regulator_enable(priv->io_vmmc);
            if (err) {
                dev_err(host->dev,
                    "failed to enable regulator: %d\n", err);
                return -ENOMEDIUM;
            }
        }
    }

    if (SDMMC_TYPE_SD == priv->idx) {
    #ifdef SD_GPIO_DETECT
        err = of_property_read_u32(np, "cd-vol",&priv->cd_vol);        
        if (err) {
            dev_err(host->dev,
                "failed to get cd-vol from dts: %d\n", err);
            return -ENOMEDIUM;
        }
    #endif
    }
    dw_mci_balong_refclk_sel(host->dev, priv->mux_refclk);

    err = of_property_read_u32(np, "mmc-idx",&priv->mmc_idx);        
    if (err) {
        dev_err(host->dev,
            "failed to get mmc-idx from dts: %d\n", err);
        return -ENOMEDIUM;
    }

    host->priv = priv;
    return 0;
    
}

static int dw_mci_balong_setup_clock(struct dw_mci *host)
{
    struct device_node *np = host->dev->of_node;

    return of_property_read_u32(np, "max-frequency", 
        &host->bus_hz);
}

static void dw_mci_balong_prepare_command(struct dw_mci *host, u32 *cmdr)
{
    *cmdr |= SDMMC_CMD_USE_HOLD_REG;
}
void  dw_mci_balong_smaple_drv_init(struct dw_mci *host,unsigned int pid, unsigned int timing)
{
    unsigned int *new_timing;
    unsigned int samp = 0;

    new_timing = (unsigned int *)mmc_timing[pid & SDMMC_TYPE];
    samp = SDMMC_CRG_MMC_CTL(*(new_timing +  timing * SDMMC_CRG_MMC_CTL_PARM + 0), \
                    *(new_timing +  timing * SDMMC_CRG_MMC_CTL_PARM + 1), \
                    *(new_timing +  timing * SDMMC_CRG_MMC_CTL_PARM + 2), \
                    *(new_timing +  timing * SDMMC_CRG_MMC_CTL_PARM + 3), \
                    *(new_timing +  timing * SDMMC_CRG_MMC_CTL_PARM + 4), \
                    *(new_timing +  timing * SDMMC_CRG_MMC_CTL_PARM + 5));

    sc_mmc_ctrl[pid](samp, host->dev->of_node);
}

int  dw_mci_balong_clk_div_init(struct dw_mci *host, 
    unsigned int timing)
{
    unsigned int phase_clk_source;
    int ret = 0;

    phase_clk_source = MMC_CLOCK_SOURCE;
    if (timing != MMC_TIMING_UHS_SDR50 && timing != MMC_TIMING_UHS_SDR104) {
        phase_clk_source = phase_clk_source / 2;
    }
    
    if (host->biu_clk) {
        ret = clk_set_rate(host->biu_clk, phase_clk_source);
        udelay(500);
        if (ret) {
            dev_err(host->dev,
                "set phase clk failed\n");
        }
    }
    if (host->ciu_clk) {
        ret = clk_set_rate(host->ciu_clk, host->bus_hz);
        udelay(500);
        if (ret) {
            dev_err(host->dev,
                "set card clk failed\n");
        }
    }
    return ret;
}
static void dw_mci_balong_set_ios(struct dw_mci *host, struct mmc_ios *ios)
{
#ifdef SD_GPIO_DETECT

    int ret;
#endif
    struct dw_mci_balong_priv_data *priv = host->priv;

    switch (ios->power_mode) {
    case MMC_POWER_UP:
    #ifdef SD_GPIO_DETECT
        if(priv->idx == SDMMC_TYPE_SD){
            if (host->vmmc) {
                ret = regulator_set_voltage(host->vmmc, 2850000, 2850000);
                if (ret)
                    dev_err(host->dev, "regulator_set_voltage failed !\n");
                if (!regulator_is_enabled(host->vmmc)){
                    ret = regulator_enable(host->vmmc);
                    if (ret)
                        dev_err(host->dev, "regulator_enable failed !\n");
                }
            }
            if (priv->ip_vmmc)                    
                if (!regulator_is_enabled(priv->ip_vmmc))
                    ret = regulator_enable(priv->ip_vmmc);

            if (priv->io_vmmc)
                if (!regulator_is_enabled(priv->io_vmmc))
                    ret |= regulator_enable(priv->io_vmmc);
            if (ret){
                 dev_dbg(host->dev,"mmc io regulator enable error!ret :%d\n",ret);
                 return ret;
            }
            udelay(500);
            pinctrl_mmc_resume();
        }
    #endif
        break;
    case MMC_POWER_OFF:
        priv->clock = 0;
        
    #ifdef SD_GPIO_DETECT
        if(priv->idx == SDMMC_TYPE_SD){ 
            if (priv->ip_vmmc)
                if (regulator_is_enabled(priv->ip_vmmc))
                    regulator_disable(priv->ip_vmmc);
            if (priv->io_vmmc)
                if (regulator_is_enabled(priv->io_vmmc))
                    regulator_disable(priv->io_vmmc);
            if ((host->vmmc))
                if (regulator_is_enabled(host->vmmc))
                    regulator_disable(host->vmmc);
            
            dw_mci_balong_pin_suspend();
        }
    #endif
        break;
    case MMC_POWER_ON:
        if (ios->clock == priv->clock)
            break;
        dev_err(host->dev,
                "ios->clock: %d priv->clock: %d\n", ios->clock, priv->clock);

        priv->clock = ios->clock;
        host->bus_hz = (ios->clock < UHS_SDR12_MAX_DTR) 
                    ? MMC_BIU_ENUM_CLOCK : ios->clock;
        dw_mci_balong_smaple_drv_init(host,priv->mmc_idx, ios->timing);
        dw_mci_balong_clk_div_init(host, ios->timing);
        udelay(1000);
        break;
    default:
        break;
    }
}    

static int dw_mci_balong_parse_dt(struct dw_mci *host)
{
    struct device_node *np = host->dev->of_node;
    struct dw_mci_balong_priv_data *priv = host->priv;
    int err = 0;

    err  = of_property_read_u32(np, "delay-chain-select-bits", 
        &priv->delay_chain_select_bits);
    
    err |= of_property_read_u32(np, "shift-step-select-bits", 
        &priv->shift_step_select_bits);
    
    err |= of_property_read_u32(np, "shift-step-select-vaild-bits", 
        &priv->shift_step_select_vaild_bits);

    return err;
}

static int dw_mci_balong_switch_io_to_1_8v(struct dw_mci_balong_priv_data *priv, struct dw_mci *host)
{
    int err = 0;
    if (priv->io_vmmc) {
        err = regulator_set_voltage(priv->io_vmmc, 1800000, 1800000);
        usleep_range(10000, 10500);
        syssc_mmc_msc(1, host->dev->of_node);
        udelay(1000);
    }
    return err;
}

static int dw_mci_balong_switch_io_to_2_85v(struct dw_mci_balong_priv_data *priv,struct dw_mci *host)
{
    int err = 0;
    if (priv->io_vmmc) {
         syssc_mmc_msc(0, host->dev->of_node);
        usleep_range(5000, 5500);
        err = regulator_set_voltage(priv->io_vmmc, 2850000, 2850000);
        usleep_range(5000, 5500);
    }
    return err;
}

static int dw_mci_balong_start_signal_voltage_switch(struct dw_mci *host,
    int signal_voltage)
{
    struct dw_mci_balong_priv_data *priv = host->priv;
    int err;

    /*only for sd*/
    if (SDMMC_TYPE_SDIO == priv->idx)
        return 0;

    
    if (signal_voltage == MMC_SIGNAL_VOLTAGE_180) {
        if (!priv->state.io_volt)
            return 0;
        priv->state.io_volt = 0;
        err = dw_mci_balong_switch_io_to_1_8v(priv, host);
        
    } else if (signal_voltage == MMC_SIGNAL_VOLTAGE_330) {
        if (priv->state.io_volt)
            return 0;
        priv->state.io_volt= 1;
        err = dw_mci_balong_switch_io_to_2_85v(priv, host);

    } else {
        return 0;
    }

    return err;
}

static inline void dw_mci_balong_tuning_enable(struct dw_mci *host)
{
    struct dw_mci_balong_priv_data *priv = host->priv;

    sc_mmc_tuning_enable[priv->mmc_idx & SDMMC_TYPE_SD](1, host->dev->of_node);
}

static inline void dw_mci_balong_set_clk_bypass(struct dw_mci *host)
{
}

static inline void dw_mci_balong_set_clksmpl(struct dw_mci *host, u8 sample)
{
    struct dw_mci_balong_priv_data *priv = host->priv;

    sc_mmc_sample_sel[priv->mmc_idx & SDMMC_TYPE_SD](sample, host->dev->of_node);
}

static inline void dw_mci_balong_move_next_clksmpl(struct dw_mci *host, int step, int clk)
{
    struct dw_mci_balong_priv_data *priv = host->priv;

    u8 sample;
    u8 dely;
    u8 phase;

    if (!priv->shift_step_select_vaild_bits) 
        return;

    phase = step  / (1 << priv->delay_chain_select_bits);
    dely = step % (1<<priv->delay_chain_select_bits);

    sample = clk << (priv->shift_step_select_bits + priv->delay_chain_select_bits) |(dely << priv->shift_step_select_bits) | phase;
    dw_mci_balong_set_clksmpl(host, sample);

}

static int dw_mci_balong_get_best_clksmpl(u64 candiates, int step32)
{
    unsigned int iter;
    unsigned int c0;
    u64 c1;
    unsigned int cand = candiates & 0xffffffff;/*not ~0,just 32bit*/
    int i, loc = -1;
    if (step32 == 0x20) {
        /*for mmc0 4*8 smple*/
        iter = 32;
        for (i = 0; i < iter; i++) {
            c0 = ror32(cand, i);
            if ((c0 & 0x83) == 0x83) {
                loc = i;
                goto out;
            }
        }
    } else {
        /*for mmc1 4*16 smple*/
        iter = 64;
        for (i = 0; i < iter; i++) {
            c1 = ror64(candiates, i);
            if ((c1 & 0x83) == 0x83) {
                loc = i;
                goto out;
            }
        }
    }

out:
    return loc;
}

static int dw_mci_balong_nv_init(void)
{
    DRV_MODULE_SUPPORT_STRU support;
    unsigned int ret;
    
    ret = bsp_nvm_read(MMC_FEATURE_NV, (u8*)&support, sizeof(DRV_MODULE_SUPPORT_STRU));
    if(ret){
        support.sdcard = 0;
        printk("sd card read nv fail!\n");
    }

    g_mmc_nv.support = support.sdcard;

    return (int)ret;
    
}


static int dw_mci_balong_suspend(struct dw_mci_slot *slot)
{

    struct dw_mci        *host = slot->host;
    struct dw_mci_balong_priv_data *priv = NULL;
    
    if (host) {
        priv = slot->host->priv;
    }
    if (priv) {    
        regulator_disable(priv->ip_vmmc);
        if (priv->io_vmmc)
            regulator_disable(priv->io_vmmc);

        if (SDMMC_TYPE_SD == priv->idx) {
            dw_mci_balong_pin_suspend();
        }

    }
    
    return 0;
}


static int dw_mci_balong_resume(struct dw_mci_slot *slot)
{

    struct dw_mci        *host = slot->host;
    struct dw_mci_balong_priv_data *priv = NULL;
    int ret;
    
    if (host) {
        priv = slot->host->priv;
    }
    if (priv) {    
        ret = regulator_enable(priv->ip_vmmc);
        if (priv->io_vmmc)
            ret |= regulator_enable(priv->io_vmmc);
        if(ret){
             dev_dbg(host->dev,"mmc io regulator enable error!ret :%d\n",ret);
             return ret;
        }
        
        if (SDMMC_TYPE_SD == priv->idx) {
            dw_mci_balong_pin_resume(slot);
        }
    }

    return 0;

}


static int dw_mci_balong_execute_tuning(struct dw_mci_slot *slot, u32 opcode,
                    struct dw_mci_tuning_data *tuning_data)
{
    struct dw_mci *host = slot->host;
    struct mmc_host *mmc = slot->mmc;
    const u8 *blk_pattern = tuning_data->blk_pattern;
    u8 *blk_test;
    unsigned int blksz = tuning_data->blksz;
    u64 candiates = 0;
    s8 found = -1;
    int ret = 0;
    int step = 0;
    int delay = 1;
    int shift = 1;
    int clk= 0;
    struct mmc_request mrq = {NULL};
    struct mmc_command cmd = {0};
    struct mmc_command stop = {0};
    struct mmc_data data = {0};
    struct scatterlist sg;
    struct dw_mci_balong_priv_data *priv = host->priv;


    blk_test = kmalloc(blksz, GFP_KERNEL);
    if (!blk_test)
        return -ENOMEM;

    dw_mci_balong_tuning_enable(host);


    cmd.opcode = opcode;
    cmd.arg = 0;
    cmd.flags = MMC_RSP_R1 | MMC_CMD_ADTC;

    stop.opcode = MMC_STOP_TRANSMISSION;
    stop.arg = 0;
    stop.flags = MMC_RSP_R1B | MMC_CMD_AC;

    data.blksz = blksz;
    data.blocks = 1;
    data.flags = MMC_DATA_READ;
    data.sg = &sg;
    data.sg_len = 1;

    sg_init_one(&sg, blk_test, blksz);
    mrq.cmd = &cmd;
    mrq.stop = &stop;
    mrq.data = &data;
    host->mrq = &mrq;

    mci_writel(host, TMOUT, ~0);
    if (priv->delay_chain_select_bits) 
        delay = 1 << priv->delay_chain_select_bits;
    if (priv->shift_step_select_vaild_bits) 
        shift = 1 << priv->shift_step_select_vaild_bits;

     clk =0;/*clk bit*/
    do {
        dw_mci_balong_move_next_clksmpl(host, step, clk);

        mmc_wait_for_req(mmc, &mrq);

        if (!cmd.error && !data.error) {
            if (!memcmp(blk_pattern, blk_test, blksz))
                candiates |= (1 << step);
		  dev_dbg(host->dev,
                "Tuning ok: step:%d\n",step);

        } else {
              dev_dbg(host->dev,
                "Tuning error: cmd.error:%d, data.error:%d,step:%d\n",
                cmd.error, data.error,step);

        }
	
        step++;
    } while (step < delay * shift );

    found = dw_mci_balong_get_best_clksmpl(candiates, delay * shift );
    if (found >= 0)
        dw_mci_balong_set_clksmpl(host, found);
    else{
	    clk =1;/*clk niv*/
	    do {
	        dw_mci_balong_move_next_clksmpl(host, step, clk);

	        mmc_wait_for_req(mmc, &mrq);

	        if (!cmd.error && !data.error) {
	            if (!memcmp(blk_pattern, blk_test, blksz))
	                candiates |= (1 << step);
			  dev_dbg(host->dev,
	                "Tuning ok: step:%d\n",step);

	        } else {
	              dev_dbg(host->dev,
	                "Tuning error: cmd.error:%d, data.error:%d,step:%d\n",
	                cmd.error, data.error,step);

	        }
		
	        step++;
	    } while (step < delay * shift );

	    found = dw_mci_balong_get_best_clksmpl(candiates, delay * shift );
        if(found >= 0)
		dw_mci_balong_set_clksmpl(host, found);
        else
            ret = -EIO;
    }
    kfree(blk_test);
    return ret;
}

/* Common capabilities of Balong v7r5/Balong5 SoC */
static unsigned long balong_dwmmc_caps[] = {
    MMC0_CAPS,
    MMC1_CAPS,
};

static const struct dw_mci_drv_data balong_drv_data = {
    .caps            = balong_dwmmc_caps,
    .init            = dw_mci_balong_priv_init,
    .setup_clock        = dw_mci_balong_setup_clock,
    .prepare_command    = dw_mci_balong_prepare_command,
    .set_ios        = dw_mci_balong_set_ios,
    .parse_dt        = dw_mci_balong_parse_dt,
    .start_signal_voltage_switch    = dw_mci_balong_start_signal_voltage_switch,
    .execute_tuning = dw_mci_balong_execute_tuning,
    .resume = dw_mci_balong_resume,
    .suspend = dw_mci_balong_suspend,
    .resume_pin         = dw_mci_balong_pin_resume,
#ifdef SD_GPIO_DETECT
    .cd_detect_init  = dw_mci_balong_card_detect_init,
#endif
};

static const struct of_device_id dw_mci_balong_match[] = {
    { .compatible = "hisilicon,hi6950-dw-mshc",
            .data = &balong_drv_data, },
    {},
};
MODULE_DEVICE_TABLE(of, dw_mci_balong_match);

static int dw_mci_balong_probe(struct platform_device *pdev)
{
    const struct dw_mci_drv_data *drv_data;
    const struct of_device_id *match;
    unsigned int support = 0;
    
    if(!pdev->dev.of_node || of_property_read_u32(pdev->dev.of_node, "is_support", &support))
    {
        printk(KERN_ERR "Hi DWC mmc get support node fail.\n");
        return 0;
    }

    if ((0 == support) || dw_mci_balong_nv_init() || !g_mmc_nv.support) {
            printk(KERN_INFO "Hi DWC some mmc is not support.\n");
            return 0;
    }
    match = of_match_node(dw_mci_balong_match, pdev->dev.of_node);
    drv_data = match->data;
    return dw_mci_pltfm_register(pdev, drv_data);
}


static struct platform_driver dw_mci_balong_pltfm_driver = {
    .probe        = dw_mci_balong_probe,
    .remove        = __exit_p(dw_mci_pltfm_remove),
    .driver        = {
        .name        = "dwmmc_balong",
        .of_match_table    = dw_mci_balong_match,
        .pm        = &dw_mci_pltfm_pmops,
    },
};

module_platform_driver(dw_mci_balong_pltfm_driver);


MODULE_DESCRIPTION("Hisilicon Specific DW-MSHC Driver Extension");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:dwmmc-balong");

