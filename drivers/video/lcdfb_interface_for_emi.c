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

/*lint --e{537,958,529,64,718,746}*/

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
#include <asm/uaccess.h>

#include <bsp_memmap.h>
#include <hi_base.h>
#include <hi_syssc.h>
#include <bsp_edma.h>
#include <bsp_sysctrl.h>
#include <hi_emi.h>
//#include <hi_lcd.h>
#include <linux/emi/emi_balong.h>
#include <bsp_version.h>
#include <bsp_om.h>
#include <linux/video/lcdfb_balong.h>
#ifdef CONFIG_OF
#include <linux/of_address.h>
#include <linux/of.h>
#endif




#define ILI_LCD_EMI_MODE  EMI_TFT_8BIT

typedef enum{
    LCD_BASE_ADDR = 0,
    LCD_OFFSET,
    LCD_START_BIT,
    LCD_END_BIT,
    LCD_ADDR_BUTT
}LCD_EMI_DTS_ADDR;

typedef enum{
    LCD_DTS_EMI_SEL = 0,
    LCD_EMI_DTS_REG_BUTT
}LCD_EMI_DTS_REG;



struct lcd_emi_sysctrl{
    u32 base_addr;
    u32 offset;
    u32 start_bit;
    u32 end_bit;
};



struct lcd_emi_info{
    struct lcd_emi_sysctrl lcd_emi_sysctrl[LCD_EMI_DTS_REG_BUTT];
};

static struct lcd_emi_info g_lcd_emi_info;

//struct semaphore	g_screen_light_semmux;



#if 0
static inline void hi_lcd_emi_get_reg(unsigned int *value,void* base,unsigned int offset,unsigned int lowbit,unsigned int highbit)
 {
     unsigned int mask = 0;
     unsigned int temp = 0;
 
     temp   = readl(base + offset);
     mask   = (1 << (highbit - lowbit + 1) -1) << lowbit;
     *value = temp & mask;
     *value = (*value) >> lowbit; 
 }
 #endif
 
 static inline void hi_lcd_emi_set_reg(unsigned int value,unsigned int base,unsigned int offset,unsigned int lowbit,unsigned highbit)
 {
     unsigned int reg    = base + offset;
     unsigned int temp   = 0;
     unsigned int mask   = 0;
 
     temp   = readl((void*)reg);
     mask   = ((1 << (highbit - lowbit + 1)) -1) << lowbit;
     value  = (temp & (~mask)) | ((value <<lowbit) & mask);
     writel(value  ,(void*)reg);   
 
 }

s32 lcd_bus_init(void)
{
    s32 ret = 0;
    /* 设置时序 */
    if(ILI_LCD_EMI_MODE>= EMI_SCHEDULING_BUTT)
    {
        hilcd_error("EMI SCHDULING MODE IS VALID. \n");
        return LCD_ERROR;
    }
    /* I8080 和M6800 区分.LCD时序选择*/
    if(EMI_STN_8BIT_M6800 == ILI_LCD_EMI_MODE)
    {
        
        hi_lcd_emi_set_reg(1, g_lcd_emi_info.lcd_emi_sysctrl[LCD_DTS_EMI_SEL].base_addr, \
            g_lcd_emi_info.lcd_emi_sysctrl[LCD_DTS_EMI_SEL].offset, \
            g_lcd_emi_info.lcd_emi_sysctrl[LCD_DTS_EMI_SEL].start_bit, \
            g_lcd_emi_info.lcd_emi_sysctrl[LCD_DTS_EMI_SEL].end_bit);
        
    }
    else
    {
        hi_lcd_emi_set_reg(0, g_lcd_emi_info.lcd_emi_sysctrl[LCD_DTS_EMI_SEL].base_addr, \
            g_lcd_emi_info.lcd_emi_sysctrl[LCD_DTS_EMI_SEL].offset, \
            g_lcd_emi_info.lcd_emi_sysctrl[LCD_DTS_EMI_SEL].start_bit, \
            g_lcd_emi_info.lcd_emi_sysctrl[LCD_DTS_EMI_SEL].end_bit);
    }

    ret = emi_config(ILI_LCD_EMI_MODE);
    if(ret)
    {
       hilcd_error("emi_config failed!\n");
       return LCD_ERROR;
    }
    if((EMI_TFT_8BIT == ILI_LCD_EMI_MODE)||(EMI_TFT_9BIT == ILI_LCD_EMI_MODE))
    {
        return LCD_OK;
    }
    else
    {
        hilcd_error("do not support the kind of lcd\n");
        return LCD_ERROR;
    }    

}
void lcd_image_data_transfer(dma_addr_t src_addr, u8 *buf, u32 len)
{
    /*EDMA传输图像数据*/
    emi_edma_transfer(src_addr,len);
}

void lcd_data_transfer(u8 *data, u32 data_len)
{
    u32 i = 0;
    for(i = 0; i < data_len; i++)
    {
        emiDataOut8(*(data++));
    }
    
}

void lcd_cmd_transfer(u8 *data)
{
    emiCmdOut8(*data);  
}

void lcd_bus_disable_clk(void)
{
    emi_disable_clk();
}

void lcd_bus_enable_clk(void)
{
    
    emi_enable_clk();
}
void bsp_lcd_emi_dts_init(void)
{
    struct device_node *device = NULL;
    const char *node_name = "hisilicon,lcd_balong_app";
    const char *reg_name[LCD_EMI_DTS_REG_BUTT] = {"emi_sel"};
    u32 reg_addr[4];
    int i = 0;
    
    device = of_find_compatible_node(NULL, NULL, node_name);
	if(!device)
	{
		hilcd_error("audio device node is null\n");
		return;
	}
        
    for(i = 0; i < LCD_EMI_DTS_REG_BUTT; i++)
    {
        of_property_read_u32_array(device, reg_name[i], reg_addr, LCD_ADDR_BUTT);   
        g_lcd_emi_info.lcd_emi_sysctrl[i].base_addr = (u32)bsp_sysctrl_addr_get((void *)reg_addr[LCD_BASE_ADDR]);
        g_lcd_emi_info.lcd_emi_sysctrl[i].offset = reg_addr[LCD_OFFSET];
        g_lcd_emi_info.lcd_emi_sysctrl[i].start_bit = reg_addr[LCD_START_BIT];
        g_lcd_emi_info.lcd_emi_sysctrl[i].end_bit = reg_addr[LCD_END_BIT];
    }
    
    return;
}
void bsp_lcd_bus_init(void)
{
    //dts
    bsp_lcd_emi_dts_init();
    return;  
}
