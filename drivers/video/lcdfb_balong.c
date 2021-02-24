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
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/regulator/consumer.h>
#include <linux/leds.h>
#include <drv_nv_def.h>
#include <drv_nv_id.h>

#include <asm/uaccess.h>
#include <bsp_nvim.h>
#include <bsp_memmap.h>
#include <hi_base.h>
#include <hi_syssc.h>
#include <bsp_edma.h>
#include <bsp_sysctrl.h>
#include <bsp_pmu.h>
#include <bsp_dr.h>
#include <bsp_om_enum.h>
#include <bsp_dump.h>
#include <hi_emi.h>
#include <bsp_version.h>
#include <bsp_om.h>
#include <linux/video/lcdfb_balong.h>
#ifdef CONFIG_OF
#include <linux/of_address.h>
#include <linux/of.h>
#endif

#define UPDATE_LOCK_SCREEN_DELAY_TIME    50

typedef enum{
    LCD_BASE_ADDR = 0,
    LCD_OFFSET,
    LCD_START_BIT,
    LCD_END_BIT,
    LCD_ADDR_BUTT
}LCD_DTS_ADDR;

typedef enum{
    LCD_DTS_RST_N = 0,
    LCD_DTS_REG_BUTT
}LCD_DTS_REG;

struct lcd_gpio{
    u32 lcd_bl_gpio;
    u32 lcd_id0_gpio;
    u32 lcd_id1_gpio;
};

struct lcd_sysctrl{
    u32 base_addr;
    u32 offset;
    u32 start_bit;
    u32 end_bit;
};

struct stlcdfb_par {
    struct platform_device *pdev;
    struct fb_info *info;
    u32 *vmem;
    struct regulator *lcd_vcc;
    struct lcd_id id;
    u32 g_ulLcdPwrOnState;
    struct semaphore    g_screensemmux;

    #ifdef CONFIG_HAS_EARLYSUSPEND
        struct early_suspend earlysus_lcd;
    #endif
    u32 fb_imgType;
    u32 hw_refresh;
    u32 var_pixclock;
    u32 var_xres;
    u32 var_yres;
    u32 fb_page;
    struct lcd_panel_info panel_info;
    int ref_cnt;
    int lcd_backlight_registered;
    struct balong_bl bl;
    struct led_classdev led_cdev;
};

struct lcd_debug{
    u32 lcd_id;
};

struct lcd_info{
    struct lcd_gpio lcd_gpio;
    struct lcd_sysctrl lcd_sysctrl[LCD_DTS_REG_BUTT];
    struct lcd_debug lcd_debug;
    unsigned int lcd_nv;
};

static struct lcd_info g_lcd_info;

#ifdef CONFIG_FB_2_4_INCH_BALONG
static struct balong_lcd_seq initseq[] =
{
        {'c',0xCF},/**power control B*/
        {'d',0x00},
        {'d',0xD9},
        {'d',0x30},


        {'c',0xED},/*power on sequence control*/
        {'d',0x64},
        {'d',0x03},
        {'d',0x12},
        {'d',0x81},

        {'c',0xE8},/*driver timing control A*/
        {'d',0x85},
        {'d',0x10},
        {'d',0x78},

        {'c',0xCB}, /*power control A*/
        {'d',0x39},
        {'d',0x2C},
        {'d',0x00},
        {'d',0x34},
        {'d',0x02},

        {'c',0xF7},/*pump ratio control*/
        {'d',0x20},

        {'c',0xEA},/*driver timing control B*/
        {'d',0x00},
        {'d',0x00},



        {'c',0xC0},/*power control 1*/
        {'d',0x1B},/*VRH[5:0]*/

        {'c',0xC1},/*power control 2*/
        {'d',0x12},/*SAP[2:0];BT[3:0]*/

        {'c',0xC5},/*VCM control*/
        {'d',0x32},
        {'d',0x3C},


        {'c',0xC7},/*VCM control2*/
        {'d',0x99},

        {'c',0x36},/*memory access control*/
        {'d',0x08},/*RGB-BGR*/

        {'c',0x3A},
        {'d',0x55},/*65k 262k selection*/

        {'c',0xB1},
        {'d',0x00},
        {'d',0x1B},

        {'c',0xB6},/*display function control*/
        {'d',0x0A},
        {'d',0xC2},/*display order*/
        {'d',0x27},

        {'c',0xF6},
        {'d',0x01},
        {'d',0x30},
        {'d',0x20},

        {'c',0xF2},/*3gamma function disable*/
        {'d',0x00},

        {'c',0x26},/*gamma curve selected*/
        {'d',0x01},

        {'c',0xE0}, /*set gamma*/
        {'d',0x0F},
        {'d',0x1D},
        {'d',0x1A},
        {'d',0x0A},
        {'d',0x0D},
        {'d',0x07},
        {'d',0x49},
        {'d',0x66},
        {'d',0x3B},
        {'d',0x07},
        {'d',0x11},
        {'d',0x01},
        {'d',0x09},
        {'d',0x05},
        {'d',0x04},

        {'c',0xE1},
        {'d',0x00},
        {'d',0x18},
        {'d',0x1D},
        {'d',0x02},
        {'d',0x0F},
        {'d',0x04},
        {'d',0x36},
        {'d',0x13},
        {'d',0x4C},
        {'d',0x07},
        {'d',0x13},
        {'d',0x0F},
        {'d',0x2E},
        {'d',0x2F},
        {'d',0x05},

        {'c',0x11},/*SLEEP OUT*/
        {'w',120},

        {'c',0x29},/*DISPLAY ON*/

        {'c',0x2c},

};

static struct balong_lcd_seq initseqtmi[] =
{
#if (FEATURE_ON == MBB_LCD)
        {'c',0xCF},/**power control B*/
        {'d',0x00},
        {'d',0xDB},
        {'d',0x36},


        {'c',0xED},/*power on sequence control*/
        {'d',0x64},
        {'d',0x03},
        {'d',0x12},
        {'d',0x81},

        {'c',0xE8},/*driver timing control A*/
        {'d',0x85},
        {'d',0x10},
        {'d',0x78},

        {'c',0xCB}, /*power control A*/
        {'d',0x39},
        {'d',0x2C},
        {'d',0x00},
        {'d',0x34},
        {'d',0x02},

        {'c',0xF7},/*pump ratio control*/
        {'d',0x20},

        {'c',0xEA},/*driver timing control B*/
        {'d',0x00},
        {'d',0x00},



        {'c',0xC0},/*power control 1*/
        {'d',0x21},/*VRH[5:0]*/

        {'c',0xC1},/*power control 2*/
        {'d',0x12},/*SAP[2:0];BT[3:0]*/

        {'c',0xC5},/*VCM control*/
        {'d',0x44},
        {'d',0x38},


        {'c',0xC7},/*VCM control2*/
        {'d',0xAA},


        {'c',0x36},/*memory access control*/
        {'d',0x08},/*RGB-BGR*/

        {'c',0x3A},
        {'d',0x55},/*65k 262k selection*/

        {'c',0xB1},
        {'d',0x00},
        {'d',0x1B},

        {'c',0xB6},/*display function control*/
        {'d',0x0A},
        {'d',0xC2},
        {'d',0x27},

        {'c',0xF6},
        {'d',0x01},
        {'d',0x30},
        {'d',0x20},

        {'c',0xF2},/*3gamma function disable*/
        {'d',0x00},

        {'c',0x26},/*gamma curve selected*/
        {'d',0x01},

        {'c',0xE0}, /*set gamma*/
        {'d',0x0F},
        {'d',0x22},
        {'d',0x1F},
        {'d',0x0A},
        {'d',0x0E},
        {'d',0x08},
        {'d',0x4B},
        {'d',0x88},
        {'d',0x3A},
        {'d',0x09},
        {'d',0x14},
        {'d',0x07},
        {'d',0x0D},
        {'d',0x07},
        {'d',0x00},

        {'c',0xE1},
        {'d',0x00},
        {'d',0x1D},
        {'d',0x20},
        {'d',0x05},
        {'d',0x11},
        {'d',0x07},
        {'d',0x34},
        {'d',0x26},
        {'d',0x45},
        {'d',0x03},
        {'d',0x09},
        {'d',0x08},
        {'d',0x32},
        {'d',0x38},
        {'d',0x0F},

        {'c',0x11},/*SLEEP OUT*/
        {'w',120},

        {'c',0x29},/*DISPLAY ON*/

        {'c',0x2c},
#endif
};
#elif defined (CONFIG_FB_1_4_5_INCH_BALONG)

struct balong_lcd_seq initseq[] =
{
    {'c',0x0011}, /*����LCD*/
        {'w', 120},

        {'c',0x003A},/*�������ظ�ʽ*/
        {'d',0x0055},  /*65K  OR 262k selectiong  12bit control*/

        {'c',0x00C0},/*GVDD VCI1*/
        {'d',0x000C},
        {'d',0x0005},
        {'w', 10},

        {'c',0x00C1},/*Vol supply*/
        {'d',0x0006},
        {'w', 10},

        {'c',0x00C2}, /*set the amount of current in operate�Ŵ�������ֵ*/
        {'d',0x0004},
        {'w', 10},

        {'c',0x00C5},/*vcoml vcomh*/
        {'d',0x0029},
        {'d',0x0035},
        {'w', 10},

        {'c',0x00C7},/*vcomƫ��*/
        {'d',0x00D5},

        {'c',0x00F2},/*e0 e1 ʹ��*/
        {'d',0x0001},

        {'c',0x00E0},/*Gamma setting */
        {'d',0x003F},
        {'d',0x001C},
        {'d',0x0018},
        {'d',0x0025},
        {'d',0x001E},
        {'d',0x000C},
        {'d',0x0042},
        {'d',0x00D8},
        {'d',0x002B},
        {'d',0x0013},
        {'d',0x0012},
        {'d',0x0008},
        {'d',0x0010},
        {'d',0x0003},
        {'d',0x0000},

        {'c',0x00E1},
        {'d',0x0000},
        {'d',0x0023},
        {'d',0x0027},
        {'d',0x000A},
        {'d',0x0011},
        {'d',0x0013},
        {'d',0x003D},
        {'d',0x0072},
        {'d',0x0054},
        {'d',0x000C},
        {'d',0x001D},
        {'d',0x0027},
        {'d',0x002F},
        {'d',0x003C},
        {'d',0x003F},

        {'c',0x0036},/*ɨ�跽��*/
        {'d',0x0008},

        {'c',0x00B1},/*֡��*/
        {'d',0x0010},
        {'d',0x000A},

        {'c',0x002A},/*ɨ�跽��BGR*/
        {'d',0x0000},
        {'d',0x0000},
        {'d',0x0000},
        {'d',0x007F},

        {'c',0x002B},
        {'d',0x0000},
        {'d',0x0000},
        {'d',0x0000},
        {'d',0x007F},

        {'c',0x0029},/*����ʾ��*/
        {'w', 5},

        {'c',0x002C},/*д*/
        {'w', 120}

};

static struct balong_lcd_seq initseqtmi[] =
{
    {'c',0x11}, /*����LCD*/
    {'w', 120},

    {'c',0xEC},/*�������ظ�ʽ*/
    {'d',0x0C},  /*65K  OR 262k selectiong  16bit control*/

   /*{'c', 0x0026}, Set Default Gamma*/
    /*{'d', 0x0004},*/

    {'c',0x26},/*GVDD VCI1*/
    {'d',0x04},


    {'c',0xB1},/*Vol supply*/
    {'d',0x0A},
    {'d', 0x14},

    {'c',0xC0}, /*set the amount of current in operate�Ŵ�������ֵ*/
    {'d',0x11},
    {'d',0x05},

    {'c',0xC1},/*vcoml vcomh*/
    {'d',0x2},

    {'c',0xC5},/*vcomƫ��*/
    {'d',0x40},
    {'d',0x45},

    {'C',0xC7},
    {'d',0xC1},

    {'c',0x2A},/*ɨ�跽��BGR*/
    {'d',0x00},
    {'d',0x00},
    {'d',0x00},
    {'d',0x7F},

    {'c',0x2B},
    {'d',0x00},
    {'d',0x00},
    {'d',0x00},
    {'d',0x7F},

    {'c',0x3A},/*ɨ�跽��*/
    {'d',0x55},


    {'c',0x36},/*ɨ�跽��*/
    {'d',0xC8},


    {'c',0xF2},/*e0 e1 ʹ��*/
    {'d',0x01},

    {'c',0xE0},/*Gamma setting*/
    {'d',0x3F},
    {'d',0x1C},
    {'d',0x18},
    {'d',0x25},
    {'d',0x1E},
    {'d',0x0C},
    {'d',0x42},
    {'d',0xD8},
    {'d',0x2B},
    {'d',0x13},
    {'d',0x12},
    {'d',0x08},
    {'d',0x10},
    {'d',0x03},
    {'d',0x00},

    {'c',0xE1},
    {'d',0x00},
    {'d',0x23},
    {'d',0x27},
    {'d',0x0A},
    {'d',0x11},
    {'d',0x13},
    {'d',0x3D},
    {'d',0x72},
    {'d',0x54},
    {'d',0x0C},
    {'d',0x1D},
    {'d',0x27},
    {'d',0x2F},
    {'d',0x3C},
    {'d',0x3F},

    {'c',0x0029},/*����ʾ��*/
    {'w', 10},

    {'c',0x002C},/*д*/
    {'w', 120}
};
#endif

struct fb_info *g_fbinfo = NULL ;

void lcd_set_trace_level(u32 level)
{
    bsp_mod_level_set(BSP_MODU_LCD, level);
    printk("bsp_mod_level_set(BSP_MODU_LCD=%d, %d)\n",BSP_MODU_LCD,level);
}
#if 0
void hi_lcd_get_reg(unsigned int *value,void* base,unsigned int offset,unsigned int lowbit,unsigned int highbit)
{
     unsigned int mask = 0;
     unsigned int temp = 0;

     temp   = readl(base + offset);
     mask   = (1 << (highbit - lowbit + 1) -1) << lowbit;
     *value = temp & mask;
     *value = (*value) >> lowbit;
}
#endif
static inline void hi_lcd_set_reg(unsigned int value,unsigned int base,unsigned int offset,unsigned int lowbit,unsigned highbit)
{
     unsigned int reg    = base + offset;
     unsigned int temp   = 0;
     unsigned int mask   = 0;

     temp   = readl((void*)reg);
     mask   = ((1 << (highbit - lowbit + 1)) -1) << lowbit;
     value  = (temp & (~mask)) | ((value <<lowbit) & mask);
     writel(value  ,(void*)reg);

}

struct fb_info g_fbinfo_get(void)
{
    return (struct fb_info )(*g_fbinfo);
}

#ifdef CONFIG_FB_2_4_INCH_BALONG
void set_gpio_updown_time(unsigned gpio,gpio_status status,u32 t_us)
{
    gpio_direction_output(gpio,status);
    //mdelay(50);
    if(t_us > 2000)
    {
        mdelay(t_us/1000);
    }
    else
    {
        udelay(t_us);/*lint !e737*/
    }
}

void balong_bl_init(struct balong_bl *lcdbl)
{

    set_gpio_updown_time(lcdbl->gpio,GPIO_UP,35);
    lcdbl->intensity_past = 16;

}

s32 intensity_to_pulse_num(struct balong_bl *lcdbl)
{
    int pulse_num = 0;
    if (lcdbl->intensity_past == lcdbl->intensity_cur)
    {
        hilcd_trace("the brightness set is not changed!\n");

    }
    else if (lcdbl->intensity_past < lcdbl->intensity_cur)
    {
        pulse_num =  MAX_BACKLIGHT_INTENSITY + lcdbl->intensity_past - lcdbl->intensity_cur ;
    }
    else
    {
        pulse_num = lcdbl->intensity_past - lcdbl->intensity_cur;
    }
    return pulse_num;

}

void balong_bl_pulse_output (struct balong_bl *lcdbl,int pulse_num,u32 t_us)
{
    int i;
    unsigned long flags;
    local_irq_save(flags);
    for(i = 1;i <= pulse_num;i++)
    {
        set_gpio_updown_time(lcdbl->gpio,GPIO_DOWN,t_us);
        set_gpio_updown_time(lcdbl->gpio,GPIO_UP,t_us);
    }
    local_irq_restore(flags);

}

#elif defined CONFIG_FB_1_4_5_INCH_BALONG
void balong_bl_init(struct balong_bl *lcdbl)
{
    int ret;

    if(PMU_DRS_MODE_FLA_LIGHT != bsp_dr_get_mode(PMIC_DR01))
    {
        bsp_dr_set_mode(PMIC_DR01, PMU_DRS_MODE_FLA_LIGHT);
    }
    ret = regulator_enable(lcdbl->bl_vcc);
    if(ret)
    {
        hilcd_error("regulator enable failed.\n");
    }
}
/*��������ֵҪ�����ò�ͬ����*/
int balong_bl_current_set(struct balong_bl *lcdbl)
{
    int cur = 0;
    unsigned selector = 0;
    int iret = LCD_OK;

    if (lcdbl->intensity_past == lcdbl->intensity_cur)
    {
        hilcd_trace("the brightness set is not changed!\n");
    }
    else
    {
        selector = (unsigned)(lcdbl->intensity_cur);
        if(selector > 7)/*selector range : 0-7*/
            selector = 7;
        cur = regulator_list_voltage(lcdbl->bl_vcc,selector);
        iret = regulator_set_voltage(lcdbl->bl_vcc,cur,cur);
    }

    return iret;
}
#endif
void balong_bl_set_intensity(struct led_classdev *led_cladev,enum led_brightness value)
{
    /*lint --e{438,533}*/
    struct stlcdfb_par *par;
    struct balong_bl *lcdbl;
#if defined CONFIG_FB_1_4_5_INCH_BALONG
    int ret;
#endif
#ifdef CONFIG_FB_2_4_INCH_BALONG
    int pulse_num = 0;
#endif

    par = container_of(led_cladev, struct stlcdfb_par, led_cdev);
    if(NULL == par)
    {
        hilcd_trace("par is null after container_of led_cdev!\n");
        return;
    }
    lcdbl = (struct balong_bl *)(&par->bl);/*lint !e413*/
    if(NULL == lcdbl)
    {
        hilcd_trace("lcdbl is null !\n");
        return;
    }

    osl_sem_down(&(lcdbl->g_backlightsemmux));

    if(0 == value)
    {

        hilcd_trace("brightness is 0,bl will be closed!\n");
#ifdef CONFIG_FB_2_4_INCH_BALONG
        set_gpio_updown_time(lcdbl->gpio,GPIO_DOWN,35000);
#elif defined CONFIG_FB_1_4_5_INCH_BALONG
        if(regulator_is_enabled(lcdbl->bl_vcc))
            regulator_disable(lcdbl->bl_vcc);
#endif
        lcdbl->intensity_cur = 0;
        goto exit;

    }
#ifdef CONFIG_FB_2_4_INCH_BALONG
    if (0 == lcdbl->intensity_past)
        balong_bl_init(lcdbl);
    hilcd_trace("intensity_past = %d\n",lcdbl->intensity_past);

        lcdbl->intensity_cur = (value - 1)/16 + 1;
        hilcd_trace("intensity_cur = %d\n",lcdbl->intensity_cur);

    pulse_num = intensity_to_pulse_num(lcdbl);
    hilcd_trace("pulse_num is %d\n",pulse_num);

    balong_bl_pulse_output(lcdbl,pulse_num,5);

#elif defined CONFIG_FB_1_4_5_INCH_BALONG

    lcdbl->intensity_cur = (int)(value / 32);/*selector range : 0-7*/
    balong_bl_current_set(lcdbl);

    if (0 == lcdbl->intensity_past)
    {
        if(!regulator_is_enabled(lcdbl->bl_vcc))
        {
             ret = regulator_enable(lcdbl->bl_vcc);
            if(ret)
            {
                hilcd_error("regulator enable failed.\n");
            }

        }

    }
#endif
exit:
    lcdbl->intensity_past = lcdbl->intensity_cur;
    osl_sem_up(&(lcdbl->g_backlightsemmux));
    return;
}



static void set_bl_flag(struct stlcdfb_par *par);
static void set_bl_flag(struct stlcdfb_par *par)
{
    par->bl.flag = 1;
}

static void clear_bl_flag(struct stlcdfb_par *par);
static void clear_bl_flag(struct stlcdfb_par *par)/*lint !e402 !e752 !e830 */
{
    par->bl.flag = 0;
}


void lcd_reset(void)
{
#ifdef CONFIG_FB_2_4_INCH_BALONG
    u8 buf[2];
    buf[0] = DISPLAY_OFF;
    buf[1] = SLEEP_IN;
    lcd_cmd_transfer(&buf[0]);
    mdelay(20);
    lcd_cmd_transfer(&buf[1]);
    mdelay(120);
#endif
#if (FEATURE_ON == MBB_LCD)
    // do nothing
#else
    hi_lcd_set_reg(1, g_lcd_info.lcd_sysctrl[LCD_DTS_RST_N].base_addr, g_lcd_info.lcd_sysctrl[LCD_DTS_RST_N].offset, g_lcd_info.lcd_sysctrl[LCD_DTS_RST_N].start_bit, g_lcd_info.lcd_sysctrl[LCD_DTS_RST_N].end_bit);
    mdelay(50);
#endif
    hi_lcd_set_reg(0, g_lcd_info.lcd_sysctrl[LCD_DTS_RST_N].base_addr, g_lcd_info.lcd_sysctrl[LCD_DTS_RST_N].offset, g_lcd_info.lcd_sysctrl[LCD_DTS_RST_N].start_bit, g_lcd_info.lcd_sysctrl[LCD_DTS_RST_N].end_bit);
#if (FEATURE_ON == MBB_LCD)
    mdelay(10);
#else
    mdelay(50);
#endif
    hi_lcd_set_reg(1, g_lcd_info.lcd_sysctrl[LCD_DTS_RST_N].base_addr, g_lcd_info.lcd_sysctrl[LCD_DTS_RST_N].offset, g_lcd_info.lcd_sysctrl[LCD_DTS_RST_N].start_bit, g_lcd_info.lcd_sysctrl[LCD_DTS_RST_N].end_bit);
#if (FEATURE_ON == MBB_LCD)
    mdelay(10);
#else
    mdelay(50);
#endif
}

static int balong_lcd_toggle(struct balong_lcd_seq *seq, int sz)
{
         int count;
    u8 buf[1];

    for (count = 0; count < sz; count++)
    {
        buf[0] = seq[count].value;
        if(seq[count].format == 'c')
        {
            /* command */
            lcd_cmd_transfer(buf);
        }
        else if(seq[count].format == 'd')
        {
            /* data */
            lcd_data_transfer(buf, 1);
        }
        else if(seq[count].format=='w')
        {
            mdelay((unsigned int)(seq[count].value)); /*lint !e62 !e737*/
        }
    }
    return LCD_OK;
}

s32 lcd_vectid_get (u16 *pu16VectId)
{
    u32 u32Data1 = 0;
    u32 u32Data2 = 0;

    if(!pu16VectId)
    {
        hilcd_error("invalid parameter\n");
        return LCD_ERROR;
    }

    /* ��ȡ����ֵ */
    u32Data1 = (u32)gpio_get_value(g_lcd_info.lcd_gpio.lcd_id0_gpio);
    u32Data2 = (u32)gpio_get_value(g_lcd_info.lcd_gpio.lcd_id1_gpio);

    hilcd_trace("LCD_ID0 %x,LCD_ID1 %x\n",u32Data1,u32Data2);
    /* ��ȡ LCD ����ID*/
    *pu16VectId = (unsigned short)((1&u32Data1<<0)\
                     |((1&u32Data2)<<1));
    g_lcd_info.lcd_debug.lcd_id = (u32)(*pu16VectId);
    return LCD_OK;

}

void lcd_init_byid(u16 ucId)
{
    int ret = 0;
    if(LCD_ILITEK == ucId)
    {
        ret=balong_lcd_toggle(initseqtmi, ARRAY_SIZE(initseqtmi));
        if(ret)
        {
            hilcd_error("lcd init error");
        }
        return;
    }
    else if(LCD_CHIMEI == ucId)
    {

        ret=balong_lcd_toggle(initseq, ARRAY_SIZE(initseq));
        if(ret)
        {
            hilcd_error("lcd init error");
        }
        return;
    }
    else
    {
        hilcd_error("invalid lcd vector id\n");
        return;
    }

}

/*****************************************************************************
* �� �� ��  : lcdInit
*
* ��������  :  LCD ��ʼ��
*
* �������  :
*
* �������  : ��
*
* �� �� ֵ  :
*****************************************************************************/
void lcd_init(void)
{
    u16 ucId = 0;
    s32 retval = 0;
    lcd_bus_init();
    retval = lcd_vectid_get(&ucId);
    if(retval)
    {
        return;
    }

    lcd_init_byid(ucId);
    return;
}

void lcd_pwron(struct fb_info *info)
{
    struct stlcdfb_par *par = info->par;
    /*�����ϵ�״̬�����������ϵ�*/
    if (LCD_PWR_ON_STATE == par->g_ulLcdPwrOnState)
    {
        hilcd_trace("lcd has been powered on.\n");
        return ;
    }
#if (FEATURE_ON == MBB_LCD)
    // do nothing
#else
    msleep(110);/*100ms����*/
#endif

    lcd_init();
    par->g_ulLcdPwrOnState = LCD_PWR_ON_STATE;
    /*��ʼ��lcd�󱳹�ɲ���*/
    set_bl_flag(par);
    return ;
}

static s32 balong_lcd_window_set(struct fb_info *info,u16 x0,u16 y0,u16 x1,u16 y1)
{
    u8 buf[1];

    if((x0>(info->var.xres-1))||(x1>(info->var.xres-1))||(y0>(info->var.yres-1))||(y1>(info->var.yres-1)))
    {
        hilcd_error("paramter error,[x0:%d][x1:%d][y0:%d][y1:%d]",
                    x0,x1,y0,y1);
        return -EINVAL;
    }
    hilcd_trace("balong_lcd_window_set:x_start = %d,y_start = %d,x_end = %d,y_end = %d",x0,y0,x1,y1);
    buf[0] = COLUMN_ADDRESS_SET;
    lcd_cmd_transfer(buf);

    buf[0] = (u8)(x0>>8);
    /*��ʼ�������8λ */
    lcd_data_transfer(buf, 1);

    buf[0] = (u8)x0;
    /* ��ʼ�������8λ */
    lcd_data_transfer(buf, 1);

    buf[0] = (u8)(x1>>8);
    /* data */
    lcd_data_transfer(buf, 1);

    buf[0] = (u8)x1;
     /* data */
    lcd_data_transfer(buf, 1);


    buf[0] = PAGE_ADDRESS_SET;
    lcd_cmd_transfer(buf);

    buf[0] = (u8)(y0>>8);
    /* data */
    lcd_data_transfer(buf, 1);

    buf[0] = (u8)y0;
    /* data */
    lcd_data_transfer(buf, 1);

    buf[0] = (u8)(y1>>8);
    /* data */
    lcd_data_transfer(buf, 1);

    buf[0] = (u8)y1;
    /* data */
    lcd_data_transfer(buf, 1);


    buf[0] = MEMORY_WRITE;
    lcd_cmd_transfer(buf);

    return LCD_OK;
}


s32 lcd_refresh(struct fb_info *info)
{

    struct stlcdfb_par *par = info->par;
    struct fb_var_screeninfo var = info->var;
    struct fb_fix_screeninfo fix = info->fix;
    s32 ret = 0;
    #if 0
    u32 xloop,yloop;
    u8 *pucbuf = info->screen_base + var.yoffset * fix.line_length +var.xoffset ;
    u8 *buffer = pucbuf;
    #endif
    dma_addr_t src_addr =fix.smem_start + var.yoffset * fix.line_length +var.xoffset;//for emi
    u8 *buf = info->screen_base + var.yoffset * fix.line_length +var.xoffset ;//for spi
    u32 len = (fix.line_length) * (var.yres);

    osl_sem_down(&(par->g_screensemmux));

    ret = balong_lcd_window_set(info,0, 0, (u16)(var.xres - 1), (u16)(var.yres - 1));
    if(ret){
        hilcd_error("lcd window set error");
        osl_sem_up(&(par->g_screensemmux));
        return ret;
    }

    lcd_image_data_transfer(src_addr,buf,len);

    osl_sem_up(&(par->g_screensemmux));
    return LCD_OK;
}


 s32 balong_lcd_sleep_in(void)
{
    u8 buf[1];
    buf[0] = SLEEP_IN;
    lcd_cmd_transfer(buf);
#if (FEATURE_ON == MBB_LCD)
    msleep(120);  /* оƬ���ҽ�����ʱ120ms */
#else
    msleep(50);
#endif
    return LCD_OK;
}

/* lcd out of sleep */
 s32 balong_lcd_sleep_out(void)
{
    u8 buf[1];
    buf[0] = SLEEP_OUT;
    lcd_cmd_transfer(buf);
    msleep(120);
    return LCD_OK;
}
 s32 lcd_light(struct fb_info *info)
{
    struct stlcdfb_par *par = info->par;
    s32 ret = 0;
    u8 buf[1];
    buf[0] = DISPLAY_ON;

    hilcd_trace("lcd will be light!");

    osl_sem_down(&(par->g_screensemmux));
    ret = balong_lcd_sleep_out();
     if(ret)
    {
        hilcd_error("balong_lcd_sleep_out error!");
        osl_sem_up(&(par->g_screensemmux));
        return ret;
    }
    lcd_cmd_transfer(buf);
    msleep(120);/*�����sleep����Ļ����һ�°�*/
    osl_sem_up(&(par->g_screensemmux));

    balong_bl_set_intensity((struct led_classdev *)(&par->led_cdev),(enum led_brightness)par->led_cdev.brightness);
    set_bl_flag(par);

    return LCD_OK;
}


 s32 lcd_sleep(struct fb_info *info)
{
    s32 ret = 0;
    u8 buf[1];
    struct stlcdfb_par *par = NULL;

    hilcd_trace("lcd will sleep!");

    buf[0] = DISPLAY_OFF;
    par= info->par;

    clear_bl_flag(par);

    balong_bl_set_intensity((struct led_classdev *)(&par->led_cdev),(enum led_brightness)0);

    osl_sem_down(&(par->g_screensemmux));
    lcd_cmd_transfer(buf);
    mdelay(20);
    ret = balong_lcd_sleep_in();
     if(ret)
    {
        hilcd_error("balong_lcd_sleep_in error!");
        osl_sem_up(&(par->g_screensemmux));
        return ret;
    }
    mdelay(120);

    osl_sem_up(&(par->g_screensemmux));

    return LCD_OK;
}
 s32 balong_lcd_display_allblack(struct fb_info *info)
{
    struct stlcdfb_par *par = NULL;
    u32 count;
    s32 ret;
    u8 data[2];
    u32 len = 0;

    len = (info->fix.smem_len)/2; /* pixel format 565 */
    data[0] = 0x00;
    data[1] = 0x00;
    par = info->par;
    ret = balong_lcd_window_set(info,0,0,(u16)(info->var.xres-1),(u16)(info->var.yres-1));
    if(ret)
    {
        hilcd_error("lcd window set error");
        return ret;
    }
    osl_sem_down(&(par->g_screensemmux));
    for(count=0;(u32)count<len;count++)
    {
        lcd_data_transfer(data,2);
    }

    osl_sem_up(&(par->g_screensemmux));
    return LCD_OK;
}
s32 lcd_pwroff(struct fb_info *info)
{
    s32 ret = 0;
    struct stlcdfb_par *par = info->par;
    /*�����µ�״̬�����������µ�*/
    if (LCD_PWR_OFF_STATE == par->g_ulLcdPwrOnState)
    {
        hilcd_error("lcd has been powered off");
        return LCD_OK;
    }

    /*�ر���ʾ*/
    ret = lcd_sleep(info);
    if(ret)
    {
        hilcd_error("lcd sleepin error");
        return LCD_ERROR;
    }
    par->g_ulLcdPwrOnState = LCD_PWR_OFF_STATE;
    return LCD_OK;
}

#if (FEATURE_ON == MBB_LCD)
/*****************************************************************
Parameters    :  info
Return        :  NULL  
Description   :  LCD�������µ����̣���Ӧ�õ���ioctl����blank����
*****************************************************************/
static void lcd_fb_blank(struct fb_info *info)
{
    int ret = -1;
    u8 buf = DISPLAY_OFF;
    struct stlcdfb_par *par = NULL;

    if(NULL == info)
    {
        printk(KERN_ERR "info is null\n");
        return;
    }
    par = info->par;
    osl_sem_down(&(par->g_screensemmux));
    ret = regulator_is_enabled(par->lcd_vcc);
    if(LCD_PWR_ON_STATE == ret)
    {
        lcd_cmd_transfer(&buf); /* ����ʾ */
        msleep(20); /* оƬ���ҽ�����ʱ20ms */
        balong_lcd_sleep_in();  /* ����sleepģʽ */
        regulator_disable(par->lcd_vcc);   
        par->g_ulLcdPwrOnState = LCD_PWR_OFF_STATE;
    }
    osl_sem_up(&(par->g_screensemmux));
}

/*****************************************************************
Parameters    :  info
Return        :  NULL    
Description   :  LCD�������ϵ����̣���Ӧ�õ���ioctl����blank����
*****************************************************************/
static void lcd_fb_unblank(struct fb_info *info)
{
    int ret = -1;
    struct stlcdfb_par *par = NULL;

    if(NULL == info)
    {
        printk(KERN_ERR "info is null\n");
        return;
    }
    par = info->par;
    osl_sem_down(&(par->g_screensemmux));
    ret = regulator_is_enabled(par->lcd_vcc);
    if(LCD_PWR_ON_STATE != ret)
    {
        if(0 != regulator_enable(par->lcd_vcc))
        {
            hilcd_error("lcd_vcc regulator enable is fail. \n");
            return;
        }
        lcd_reset();
        lcd_init();
        par->g_ulLcdPwrOnState = LCD_PWR_ON_STATE;
        osl_sem_up(&(par->g_screensemmux));
        balong_lcd_display_allblack(info);
    }
    else
    {
        /* ʲô������ */
        osl_sem_up(&(par->g_screensemmux));
    }
}

/*****************************************************************
Parameters    :  void
Return        :  NULL  
Description   :  ������ػ�������ʱLCD�µ�ӿ�
*****************************************************************/
void bsp_power_lcd_fb_blank(void)
{
    struct stlcdfb_par *par = g_fbinfo->par;

    if(NULL == g_fbinfo)
    {
        printk(KERN_ERR "g_fbinfo is null\n");
        return;
    }

    /* �ȹرձ��� */
    balong_bl_set_intensity((struct led_classdev *)(&par->led_cdev),(enum led_brightness)0);

    /* �ٹر�LCD */
    lcd_fb_blank(g_fbinfo);
}

#endif

 s32 stlcdfb_blank(int blank_mode,struct fb_info *fbinfo)
{
   /* struct platform_device *pdev = to_platform_device(fbinfo);*/

    hilcd_error("blank_mode is %d",blank_mode);
    
    switch (blank_mode)
    {
#if (FEATURE_OFF == MBB_LCD)
        case VESA_NO_BLANKING:/*lcd power 1;backlight power1*/
        (void)lcd_light(fbinfo);
        break;
        case VESA_VSYNC_SUSPEND:/*lcd on;backlight off/sleepin*/
        (void)lcd_sleep(fbinfo);
        break;
        case VESA_HSYNC_SUSPEND:
        case VESA_POWERDOWN:/*lcd off;backlight:off*/
        (void)lcd_pwroff(fbinfo);
        break;
#else
        case FB_BLANK_UNBLANK:
            (void)lcd_fb_unblank(fbinfo);
            break;
        case FB_BLANK_NORMAL:
        case FB_BLANK_VSYNC_SUSPEND:
        case FB_BLANK_HSYNC_SUSPEND:
        case FB_BLANK_POWERDOWN:
            /*  MBB��blank �µ�����  */
            (void)lcd_fb_blank(fbinfo);
            break;    
#endif        
        default:
        break;

    }
    return 0;
}

#if 0
s32 lcd_refresh_bybit(struct fb_info *info,u8 x0,u8 y0,u8 x1,u8 y1,const u8 * pbuffer)/*lint !e438 !e529 !e830 */
{
    struct stlcdfb_par *par = info->par;
    s32 ret = 0;
    u32 bitNum = 0;
    u8 xloop,yloop;
    u8 *buffer =(u8 *)pbuffer;
    u8 white[2] = {0XFF,0XFF};
    u8 black[2] = {0X00,0X00};

    if(NULL == pbuffer)
    {
        return LCD_ERROR;
    }

    /* set window before write image */
    osl_sem_down (&(par->g_screensemmux));

    ret = balong_lcd_window_set(info,x0, y0, x1, y1);
    if(ret)
    {
        osl_sem_up(&(par->g_screensemmux));
        return ret;
    }
    for (yloop = y0; yloop <= y1; yloop++)
    {
        for (xloop = x0; xloop <= x1; xloop++)
        {
            if( bitNum ==8 )
            {
                 buffer++;
                 bitNum=0;/*һ���ֽ�������ʱָ����ƣ�λ��־����*/
             }

            if( (((*buffer)>>(7-bitNum)) & 0x01) == 0x01)/*�Ӹ�λ��ʼ�ж�*/
            {
                lcd_data_transfer(white,2);
            }
            else
            {
                lcd_data_transfer(black,2);
            }
            bitNum++;
        }
    }

    osl_sem_up(&(par->g_screensemmux));
    return LCD_OK;
}

#endif

void lcdfb_ocp_handler(int volt_id)
{
    hilcd_error("ocp %d error, system will be restarted!\n", volt_id);
    system_error(DRV_ERROR_LCD_VCC, volt_id, 0, NULL, 0);
}

/* lcd display white at all window */
#if 0
s32 balong_lcd_display_allwhite(struct fb_info *info)/*lint !e830 !e529 */
{
    u32 count;
    s32 ret = 0;
    u8 data[2] = {0XFF,0XFF};
    u32 len = (info->fix.smem_len)/2;
    struct stlcdfb_par *par = info->par;
    ret = balong_lcd_window_set(info,0,0,(u16)(info->var.xres-1),(u16)(info->var.yres-1));
    if(ret)
    {
        hilcd_error("lcd window set error");
        return ret;
    }
    osl_sem_down(&(par->g_screensemmux));
    for(count=0;(unsigned int)count<len;count++)
    {
        lcd_data_transfer(data, 2);
    }
    osl_sem_up(&(par->g_screensemmux));
    return LCD_OK;
}
#endif

static u32 stlcd_fb_line_length(int fb_index, u32 xres, u32 bpp)
{   /* The adreno GPU hardware requires that the pitch be aligned to
32 pixels for color buffers, so for the cases where the GPU
is writing directly to fb0, the framebuffer pitch
also needs to be 32 pixel aligned */
    /*if (fb_index == 0)
       return ALIGN(xres, 32) * bpp;
    else  */
        return (u32)(xres * bpp);
}

int stlcdfb_check_var(struct fb_var_screeninfo *var,
            struct fb_info *info)
{
    u32 len;
    if(var->rotate != FB_ROTATE_UR)
        return -EINVAL;
    if(var->grayscale != info->var.grayscale)
        return -EINVAL;
    switch(var->bits_per_pixel){
    case 16:
        if((var->green.offset != 5)||
            !((var->blue.offset == 11)||(var->blue.offset == 0))||
            !((var->red.offset == 11)||(var->red.offset == 0))||/*suport RGB or BGR*/
            (var->blue.length != 5)||
            (var->green.length !=6)||
            (var->red.length != 5)||/*support 565*/
            (var->blue.msb_right != 0)||
            (var->green.msb_right != 0)||
            (var->red.msb_right != 0)||
            (var->transp.length!= 0)||
            (var->transp.length != 0))
            return -EINVAL;
        break;
    case 24:
        if ((var->blue.offset != 0) ||
            (var->green.offset != 8) ||
            (var->red.offset != 16) ||
            (var->blue.length != 8) ||
            (var->green.length != 8) ||
            (var->red.length != 8) ||
            (var->blue.msb_right != 0) ||
            (var->green.msb_right != 0) ||
            (var->red.msb_right != 0) ||
            !(((var->transp.offset == 0) &&
            (var->transp.length == 0)) ||
            ((var->transp.offset == 24) &&
            (var->transp.length == 8))))
            return -EINVAL;
        break;
    case 32:
        /* Figure out if the user meant RGBA or ARGB
        and verify the position of the RGB components */
        if (var->transp.offset == 24) {
            if ((var->blue.offset != 0) ||
                (var->green.offset != 8) ||
                (var->red.offset != 16))
                return -EINVAL;
            } else if (var->transp.offset == 0) {
            if ((var->blue.offset != 8) ||
                (var->green.offset != 16) ||
                (var->red.offset != 24))
                return -EINVAL;
            } else
            return -EINVAL;
            /* Check the common values for both RGBA and ARGB */
            if ((var->blue.length != 8) ||
                (var->green.length != 8) ||
                (var->red.length != 8) ||
                (var->transp.length != 8) ||
                (var->blue.msb_right != 0) ||
                (var->green.msb_right != 0) ||
                (var->red.msb_right != 0))
                return -EINVAL;
            break;
    default:
            return -EINVAL;

    }
    if ((var->xres_virtual == 0) || (var->yres_virtual == 0))
        return -EINVAL;
    len = var->xres_virtual * var->yres_virtual * (var->bits_per_pixel / 8);
    if (len > info->fix.smem_len)
        return -EINVAL;
    if ((var->xres == 0) || (var->yres == 0))
        return -EINVAL;
    /*if ((var->xres > mfd->panel_info.xres) ||
        (var->yres > mfd->panel_info.yres))
        return -EINVAL;  */
    if (var->xoffset > (var->xres_virtual - var->xres))
        return -EINVAL;
    if (var->yoffset > (var->yres_virtual - var->yres))
        return -EINVAL;
    return 0;
}

int stlcdfb_set_par(struct fb_info *info)
{
    struct stlcdfb_par *par = (struct stlcdfb_par *)(info->par);
    struct fb_var_screeninfo *var = &info->var;
    u32 old_imgType;
    int blank = 0;
    old_imgType = par->fb_imgType;
    switch (var->bits_per_pixel) {
        case 16:
            if (var->red.offset == 0)
                par->fb_imgType = MDP_BGR_565;
            else
                par->fb_imgType = MDP_RGB_565;
            break;
            case 24:
                if ((var->transp.offset == 0) && (var->transp.length == 0))
                    par->fb_imgType = MDP_RGB_888;
                else if ((var->transp.offset == 24) &&
                    (var->transp.length == 8)) {
                    par->fb_imgType = MDP_ARGB_8888;
                    info->var.bits_per_pixel = 32;
                    }
                break;
        case 32:
            if (var->transp.offset == 24)
                par->fb_imgType = MDP_ARGB_8888;
            else
                par->fb_imgType = MDP_RGBA_8888;
            break;
            default:
                return -EINVAL;
        }
    if ((par->var_pixclock != var->pixclock) ||
        (par->hw_refresh && ((par->fb_imgType != old_imgType) ||
        (par->var_pixclock != var->pixclock) ||
        (par->var_xres != var->xres) ||
        (par->var_yres != var->yres)))) {
            par->var_xres = var->xres;
            par->var_yres = var->yres;
            par->var_pixclock = var->pixclock;
            blank = 1;
            }
    par->info->fix.line_length = stlcd_fb_line_length(info->node, var->xres,
        var->bits_per_pixel / 8);
    if (blank) {
        stlcdfb_blank(VESA_POWERDOWN, info);
#if (FEATURE_ON == MBB_LCD)
        (void)lcd_fb_unblank(info);
#else
        lcd_on(info);
#endif
        }
    return 0;
}

int stlcdfb_pan_display( struct fb_var_screeninfo *var, struct fb_info *info)
{
    static int is_first_dispaly = 1;
    struct stlcdfb_par *par = (struct stlcdfb_par *)(info->par);

    if (var->xoffset > (info->var.xres_virtual - info->var.xres)){
        hilcd_trace("var.xoffset is unnormal and the value is %x\n",var->xoffset);
        return -EINVAL;
    }
    if (var->yoffset > (info->var.yres_virtual - info->var.yres)){
        hilcd_trace("var.yoffset is unnormal and the value is %x\n",var->yoffset);
        return -EINVAL;
    }
    if (info->fix.xpanstep)
        info->var.xoffset =
        (var->xoffset / info->fix.xpanstep) * info->fix.xpanstep;

    if (info->fix.ypanstep)
        info->var.yoffset =
        (var->yoffset / info->fix.ypanstep) * info->fix.ypanstep;

    /*Ϊ�˱��ֶ��������ԣ�probe�в���ʼ��lcd������fastboot�е�ͼ��һֱ��Ӧ��ˢ��ʱ�ų�ʼ��*/
    if(is_first_dispaly)
    {
        //gpio_set_value(par->bl.gpio,0);
        balong_bl_set_intensity((struct led_classdev *)(&par->led_cdev),(enum led_brightness)0);
#if (FEATURE_ON == MBB_LCD)
        // do nothing
#else
        lcd_reset();
#endif
        lcd_pwron(info);
        balong_lcd_display_allblack(info);
        balong_bl_set_intensity((struct led_classdev *)(&par->led_cdev),(enum led_brightness)255);
        //gpio_set_value(par->bl.gpio,1);

        is_first_dispaly = 0;
        hilcd_trace("LCD:the lcd is display for the first time!!\n");
    }
    lcd_refresh(info);

    return LCD_OK;
}

int stlcdfb_open(struct fb_info *info,int user)
{
    struct stlcdfb_par *par = (struct stlcdfb_par *)info->par;
    if(!par->ref_cnt){
        hilcd_trace("first open fb %d\n",info->node);
    }
    par->ref_cnt++;
    return LCD_OK;
}


int stlcdfb_release(struct fb_info *info,int user)
{
    struct stlcdfb_par *par = (struct stlcdfb_par *)info->par;

    if(!par->ref_cnt){
        hilcd_error("try to close unopened fb %d\n",info->node);
        return -EINVAL;
        }
    par->ref_cnt--;
    if(!par->ref_cnt){
         hilcd_trace("last close fb %d\n",info->node);
        }

    return LCD_OK;
}



static struct fb_ops stlcdfb_ops = {
    .owner        = THIS_MODULE,
    .fb_open = stlcdfb_open,
    .fb_release = stlcdfb_release,
    .fb_blank   = stlcdfb_blank,
    .fb_check_var = stlcdfb_check_var,
    .fb_set_par = stlcdfb_set_par,
    .fb_pan_display= stlcdfb_pan_display,
    /*.fb_fillrect = cfb_fillrect,*/
    /*.fb_copyarea = cfb_copyarea,*/
    /*.fb_imageblit = cfb_imageblit,*/

};




#ifdef CONFIG_PM
#if (FEATURE_ON == MBB_LCD)
/*  ����ioctl blank��ʽ����lcd���ģ���������һ�����߻��� */
#define BALONG_DEV_PM_OPS NULL
#else
int lcd_prepare(struct device *dev)
{
    hilcd_trace("lcd suspend entered!\n");
    lcd_sleep(g_fbinfo);
    return LCD_OK;
}
void lcd_complete(struct device *dev)
{
    hilcd_trace("lcd resume entered!\n");
    mdelay(UPDATE_LOCK_SCREEN_DELAY_TIME);
    lcd_light(g_fbinfo);
    return;
}

static const struct dev_pm_ops lcd_pm_ops ={
    .prepare = lcd_prepare,
    .complete = lcd_complete,
};

#define BALONG_DEV_PM_OPS (&lcd_pm_ops)
#endif
#else
#define BALONG_DEV_PM_OPS NULL
#endif

s32 stlcdfb_fill(struct fb_info *info,struct stlcdfb_par *par)
{
    struct fb_var_screeninfo *var = &info->var;


    switch (par->fb_imgType) {
        case MDP_RGB_565:
            info->fix.type = FB_TYPE_PACKED_PIXELS;
            info->fix.xpanstep = 1;
            info->fix.ypanstep = 1;
            var->vmode = FB_VMODE_NONINTERLACED;
            var->blue.offset = 0;
            var->green.offset = 5;
            var->red.offset = 11;
            var->blue.length = 5;
            var->green.length = 6;
            var->red.length = 5;
            var->blue.msb_right = 0;
            var->green.msb_right = 0;
            var->red.msb_right = 0;
            var->transp.offset = 0;
            var->transp.length = 0;
            var->bits_per_pixel = 2 * 8;
        break;
        case MDP_RGB_888:
            info->fix.type = FB_TYPE_PACKED_PIXELS;
            info->fix.xpanstep = 1;
            info->fix.ypanstep = 1;
            var->vmode = FB_VMODE_NONINTERLACED;
            var->blue.offset = 0;
            var->green.offset = 8;
            var->red.offset = 16;
            var->blue.length = 8;
            var->green.length = 8;
            var->red.length = 8;
            var->blue.msb_right = 0;
            var->green.msb_right = 0;
            var->red.msb_right = 0;
            var->transp.offset = 0;
            var->transp.length = 0;
            var->bits_per_pixel = 3 * 8;
        break;
        case MDP_ARGB_8888:
            info->fix.type = FB_TYPE_PACKED_PIXELS;
            info->fix.xpanstep = 1;
            info->fix.ypanstep = 1;
            var->vmode = FB_VMODE_NONINTERLACED;
            var->blue.offset = 0;
            var->green.offset = 8;
            var->red.offset = 16;
            var->blue.length = 8;
            var->green.length = 8;
            var->red.length = 8;
            var->blue.msb_right = 0;
            var->green.msb_right = 0;
            var->red.msb_right = 0;
            var->transp.offset = 24;
            var->transp.length = 8;
            var->bits_per_pixel = 4 * 8;
            break;
         case MDP_RGBA_8888:
            info->fix.type = FB_TYPE_PACKED_PIXELS;
            info->fix.xpanstep = 1;
            info->fix.ypanstep = 1;
            var->vmode = FB_VMODE_NONINTERLACED;
            var->blue.offset = 8;
            var->green.offset = 16;
            var->red.offset = 24;
            var->blue.length = 8;
            var->green.length = 8;
            var->red.length = 8;
            var->blue.msb_right = 0;
            var->green.msb_right = 0;
            var->red.msb_right = 0;
            var->transp.offset = 0;
            var->transp.length = 8;
            var->bits_per_pixel = 4 * 8;
        break;
        case MDP_YCRYCB_H2V1:
            info->fix.type = FB_TYPE_INTERLEAVED_PLANES;
            info->fix.xpanstep = 2;
            info->fix.ypanstep = 1;
            var->vmode = FB_VMODE_NONINTERLACED;
            /* how about R/G/B offset? */
            var->blue.offset = 0;
            var->green.offset = 5;
            var->red.offset = 11;
            var->blue.length = 5;
            var->green.length = 6;
            var->red.length = 5;
            var->blue.msb_right = 0;
            var->green.msb_right = 0;
            var->red.msb_right = 0;
            var->transp.offset = 0;
            var->transp.length = 0;
            var->bits_per_pixel = 2 * 8;
        break;
        default:
            hilcd_error("stlcd_fb_init: fb %d unkown image type!\n",
                info->node);
            return LCD_ERROR;
        }
    return LCD_OK;
}

int bsp_lcd_dts_init(struct platform_device *pdev)
{
    struct device_node *device = NULL;
    char node_name[32] = "";
    char reg_name[LCD_DTS_REG_BUTT][32] = {"lcd_rst_n"};
    u32 reg_addr[4] = {0};
    int i = 0;
    int ret = 0;

    snprintf(node_name, 32,"/lcd");
    device = of_find_node_by_path(node_name);
    if(!device)
    {
        dev_err(&pdev->dev, "audio device node is null\n");
#if (FEATURE_ON == MBB_LCD)
        return LCD_ERROR;
#else
        return -1;
#endif
    }

    ret = of_property_read_u32_array(device, "gpio", reg_addr, 3);
    if(ret)
    {
        hilcd_error("failed to get gpio from dts!\n");
        return -1;
    }
    g_lcd_info.lcd_gpio.lcd_id0_gpio = reg_addr[0];
    g_lcd_info.lcd_gpio.lcd_id1_gpio = reg_addr[1];
    g_lcd_info.lcd_gpio.lcd_bl_gpio = reg_addr[2];

    for(i = 0; i < LCD_DTS_REG_BUTT; i++)
    {
        ret = of_property_read_u32_array(device, reg_name[i], reg_addr, LCD_ADDR_BUTT);
        if(ret)
        {
            hilcd_error("failed to get lcd reset from dts!\n");
            return -1;
        }
        g_lcd_info.lcd_sysctrl[i].base_addr = (u32)bsp_sysctrl_addr_get((void *)reg_addr[LCD_BASE_ADDR]);
        g_lcd_info.lcd_sysctrl[i].offset = reg_addr[LCD_OFFSET];
        g_lcd_info.lcd_sysctrl[i].start_bit = reg_addr[LCD_START_BIT];
        g_lcd_info.lcd_sysctrl[i].end_bit = reg_addr[LCD_END_BIT];
    }
    return 0;

}

static int lcd_nv_init(void)
{
    DRV_MODULE_TEST_STRU support;
    unsigned int ret;

    ret = bsp_nvm_read(NV_ID_DRV_TEST_SUPPORT, (u8*)&support, sizeof(DRV_MODULE_TEST_STRU));
    if(ret){
        support.lcd = 0;
        hilcd_error("lcd read nv fail!\n");
    }

#if (FEATURE_ON == MBB_LCD)
    g_lcd_info.lcd_nv = 1;
    return 0;
#else
    g_lcd_info.lcd_nv = support.lcd;

    return (int)ret;
#endif

}
static s32 __init stlcdfb_probe (struct platform_device *pdev)
{
    struct fb_info *info;
    struct stlcdfb_par *par;
    struct fb_var_screeninfo *var ;//= &info->var
    size_t lcd_size;
    u32 phys = 0;
    s32 retval = -ENOMEM;
    int ret = 0;

    /*����������*/
    if(NULL == pdev)
    {
        return -ENOMEM;
    }

    ret = bsp_lcd_dts_init(pdev);
    if(ret)
    {
        hilcd_error("lcd get dts failed!!! \n");
        return ret;
    }
    /* ��ʼ�����������Ϣ */
    bsp_lcd_bus_init();

    /*����һ��fb_info�ṹ��,�ṹ����ͨ��ָ��*parΪstruct stlcdfb_par�ṹ*/
    info = framebuffer_alloc(sizeof(struct stlcdfb_par), &pdev->dev);
    if (!info)
        return -ENOMEM;

    /*info ��Ϊpdev��drvdata*/
    platform_set_drvdata(pdev, info);

    /*par �ҽ���info->par��*/
    par = info->par;

    /*��ʼ����Ļ�����ź���*/
    osl_sem_init(LCD_SEM_FULL, &(par->g_screensemmux));

    /*����û���Ҫ��par����*/
    par->g_ulLcdPwrOnState = LCD_PWR_OFF_STATE;
    par->fb_page = FB_NUM;/*֧��˫buffer*/
    par->fb_imgType = MDP_RGB_565;
    par->ref_cnt = 0;
    par->lcd_backlight_registered = 0;

     /*��ʼ��fb_info�ṹ��*/
    strncpy(info->fix.id , "STLCD", sizeof(info->fix.id));

    info->fbops = &stlcdfb_ops;

    /*�����û���Ҫ����ɫ��ʽ,��䲿��info��par�ṹ��ֵ*/
    retval = stlcdfb_fill(info,par);
    if(retval)
    {
        hilcd_error("stlcdfb_fill error!\n");
        goto exit;
    }

    info->flags = FBINFO_FLAG_DEFAULT | FBINFO_VIRTFB;

    var = (struct fb_var_screeninfo *)&info->var;
    var->xres = LCD_WIDTH +1;
    var->yres = LCD_HIGHT +1;
    var->xres_virtual = LCD_WIDTH+1;
    var->yres_virtual = var->yres * par->fb_page;


    par->var_xres = var->xres;
    par->var_yres = var->yres;
    par->var_pixclock = var->pixclock;


    info->fix.line_length = stlcd_fb_line_length(info->node, var->xres,var->bits_per_pixel/8);

    lcd_size = info->fix.line_length * var->yres *par->fb_page;
    par->vmem =  dma_alloc_coherent(NULL, lcd_size,(dma_addr_t *) &phys, GFP_KERNEL);
    if (!par->vmem){
        hilcd_error("unable to alloc fbmem size = %u\n",lcd_size);
        goto exit;
    }

    info->screen_base = (char *)(par->vmem);/*(u8 __force __iomem *)*/
    info->fix.smem_start = phys;
    info->fix.smem_len = lcd_size;
    hilcd_trace("framebuffer address_virt = %x;address_phys = %x;len = %d",
        info->screen_base,phys,lcd_size);
    info->flags = FBINFO_FLAG_DEFAULT | FBINFO_VIRTFB;



    if(gpio_request(g_lcd_info.lcd_gpio.lcd_id0_gpio, "LCD_ID0"))
    {
        hilcd_error("Gpio is busy!!! \n");
    }

    if(gpio_request(g_lcd_info.lcd_gpio.lcd_id1_gpio, "LCD_ID1"))
    {
        hilcd_error("Gpio is busy!!! \n");
    }
#if defined CONFIG_FB_2_4_INCH_BALONG
    par->bl.gpio = g_lcd_info.lcd_gpio.lcd_bl_gpio;

    ret = gpio_request(par->bl.gpio, "lcd_backlight");
    if(ret < 0)
    {
        hilcd_error("gpio_request failed!\n");
    }
#elif defined CONFIG_FB_1_4_5_INCH_BALONG
    par->bl.bl_vcc = regulator_get(NULL, "BL-vcc");
    ret = IS_ERR(par->bl.bl_vcc);
    if(ret)
    {
        hilcd_error("dr BL-vcc failed!\n");
    }
#endif
    par->lcd_vcc = regulator_get(NULL,"LCD-vcc");
    if (IS_ERR(par->lcd_vcc)) {
        dev_err(&pdev->dev, "Get %s fail\n", "LCD-vcc");
        goto release_mem_exit;
    }

#if (FEATURE_ON == MBB_LCD)
    ret = regulator_enable(par->lcd_vcc);
    if (ret)
    {
        hilcd_error("lcd-vcc regulator_enable failed!\n");
    }
#endif
    bsp_pmu_ocp_register(regulator_get_id(par->lcd_vcc), lcdfb_ocp_handler);
    par->led_cdev.name = "lcd-backlight";
    par->led_cdev.brightness = MAX_BACKLIGHT_BRIGNTNESS;
    par->led_cdev.brightness_set = balong_bl_set_intensity;
    //par->led_cdev.brightness_set = balong_backlight_set_intensity;

    /*��ʼ������ɲ�����־*/
    clear_bl_flag(par);
    /*��ʼ�����⻥���ź���*/
    osl_sem_init(LCD_SEM_FULL, &(par->bl.g_backlightsemmux));

   /*par info pdev ֮�����ϵ����*/
    par->info = info;
    par->pdev = pdev;
    g_fbinfo = info;
    /*ע��framebuffer�豸*/
    retval = register_framebuffer(info);
    if (retval < 0)
        goto early_exit;

    if(!par->lcd_backlight_registered){
        if(led_classdev_register(&pdev->dev,&par->led_cdev))
            hilcd_error("led_classdev_register failed!");
        else
            par->lcd_backlight_registered = 1;
        }

    //lcd_reset();

    //lcd_pwron(info);
    balong_bl_init(&par->bl);

    hilcd_trace(
        "fb%d: %s frame buffer device,\n\tusing %d Byte of video memory\n",
        info->node, info->fix.id, info->fix.smem_len);

    return 0;

early_exit:
    osl_sema_delete(&(par->bl.g_backlightsemmux));
release_mem_exit:/*lint !e563*/
    dma_free_coherent(NULL, lcd_size,par->vmem, phys);
exit:
    osl_sema_delete(&(par->g_screensemmux));
    platform_set_drvdata(pdev,NULL);
    framebuffer_release(info);
    return retval;
}

static s32 stlcdfb_remove(struct platform_device *pdev)
{
    struct fb_info *info = platform_get_drvdata(pdev);
    struct stlcdfb_par *par;

    if(NULL == info)
    {
        pr_err("%s %d platform_get_drvdata info is NULL\n", __func__, __LINE__);
        return -1;
    }
    par = (struct stlcdfb_par *)info->par;
    platform_set_drvdata(pdev, NULL);
    if (info) {
        unregister_framebuffer(info);
        kfree(info->screen_base);
        framebuffer_release(info);
        osl_sema_delete(&(par->g_screensemmux));
    }
    if(par->lcd_backlight_registered){
        par->lcd_backlight_registered = 0;
        led_classdev_unregister(&par->led_cdev);

    }

    return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id balong_lcd_of_match[] = {
    { .compatible = "hisilicon,lcd_balong_app", },
    {},
};
MODULE_DEVICE_TABLE(of, balong_lcd_of_match);
#endif

/*����ע��*/
static struct platform_driver stlcdfb_driver = {
    .driver = {
        .name   = "stlcd_tft_fb",
        .owner  = THIS_MODULE,
        .pm     = BALONG_DEV_PM_OPS,
        .of_match_table = of_match_ptr(balong_lcd_of_match),
    },

    .probe  = stlcdfb_probe,

    .remove = stlcdfb_remove,
};


static int __init stlcdfb_init(void)
{
    int ret = 0 ;

    /* ��ȡnv�����nv�ж�ӦbitΪ0����֧��lcd����ע�ᣬֱ�ӷ��� */
    if (lcd_nv_init() || !g_lcd_info.lcd_nv)
    {
        hilcd_error("lcd is not support.\n");
        return 0;
    }

    ret = platform_driver_register(&stlcdfb_driver);
    if(ret)
    {
        hilcd_error("platform_driver_register failed");
        return ret;
    }

    return 0;
}

static void __exit stlcdfb_exit(void)
{
    platform_driver_unregister(&stlcdfb_driver);
}

module_init(stlcdfb_init);
module_exit(stlcdfb_exit);

EXPORT_SYMBOL_GPL(lcd_set_trace_level);
#ifdef CONFIG_PM
#if (FEATURE_OFF == MBB_LCD)
EXPORT_SYMBOL_GPL(lcd_prepare);
EXPORT_SYMBOL_GPL(lcd_complete);
#endif
#endif



