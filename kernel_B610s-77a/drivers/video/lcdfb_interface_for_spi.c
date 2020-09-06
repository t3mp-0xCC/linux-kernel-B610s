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

#define SPIDEV_MAJOR			150	/* SPI从设备主设备号 */
#define N_SPI_MINORS			32	/* SPI从设备次设备号 */
/*定义一个数组，数组的维数通过N_SPI_MINORS计算得到*/
static DECLARE_BITMAP(minors, N_SPI_MINORS);


#define IMAGEBUFFER (LCD_WIDTH + 1)* (LCD_HIGHT + 1) *8

struct lcd_spidev_data {
	dev_t			devt;
	spinlock_t		spi_lock;
	struct spi_device	*spi;
	struct list_head	device_entry;

	/* buffer is NULL unless this device is open (users > 0) */
	struct mutex		buf_lock;
	unsigned		users;
	u8			*buffer;
};


struct spi_driver spidrv;

struct v7r2_lcd{
    struct class *lcd_class;
    struct lcd_spidev_data* g_lcd;
};

/*初始化设备链表*/
static LIST_HEAD(device_list);
/*初始化互斥信号量*/
static DEFINE_MUTEX(device_list_lock);

 u8 image[IMAGEBUFFER];
struct v7r2_lcd lcd;
s32 spi_write_data(const u8 *buf, size_t len)
{
        struct lcd_spidev_data *lcd_data = lcd.g_lcd;
        struct spi_device	*spi;
        u32 count = 0;
        u16 data;
        u32 *dst = (u32 *)&image[0];

        spin_lock_irq(&lcd_data->spi_lock);
	    spi = spi_dev_get(lcd_data->spi);
	    spin_unlock_irq(&lcd_data->spi_lock);

        if (NULL == spi)
            return LCD_ERROR;

        for(count = 0;count < len; count++){

            data = (u32)((buf[count]) | (1<<8)); /* data  */
            memcpy(dst,&data,4);
            dst++;

        }
        spi_write(spi,(void *)image,len*4);

        return LCD_OK;
}


/* spi write cmd */
 s32 spi_write_cmd(const u8 *buf, size_t len)
{
    struct lcd_spidev_data *lcd_data = lcd.g_lcd;
    struct spi_device	*spi;
    u32 count = 0;
    u32 cmd;
    u32 *dst = (u32 *)&image[0];
    spin_lock_irq(&lcd_data->spi_lock);
	spi = spi_dev_get(lcd_data->spi);
	spin_unlock_irq(&lcd_data->spi_lock);

    if (NULL == spi)
        return LCD_ERROR;

    for(count = 0;(u32)count < len; count++){

        cmd = (u32)(buf[count]) & ~(1 << 8); /* command,d/c=0 */
        memcpy(dst,&cmd,4);
        dst++;

    }
    spi_write(spi,(void *)image,len*4);
    return LCD_OK;

}

s32 __init lcd_spidev_probe(struct spi_device *spi)
{
	struct lcd_spidev_data	*spidev;
	s32			status;
	u32		    minor;
    u32         tmp;
    int         retval=0;
    u8          save;

	dev_info(&spi->dev, " spidev_probe begin! \n");
	/*分配lcd_spidev_data结构 */
	spidev = kzalloc(sizeof(*spidev), GFP_KERNEL);
	if (!spidev)
		return -ENOMEM;
    /*初始化lcd的spi相关的信息*/
	lcd.g_lcd = spidev;

	spidev->spi = spi;
    /*spi总线的锁和互斥信号量初始化*/
	spin_lock_init(&spidev->spi_lock);
    mutex_init(&spidev->buf_lock);
    /*初始化spi设备链表*/
	INIT_LIST_HEAD(&spidev->device_entry);
    /*创建spi从设备，并加入到设备链表中*/
	mutex_lock(&device_list_lock);/*spi设备链表锁*/
	minor = find_first_zero_bit(minors, N_SPI_MINORS);
	if (minor < N_SPI_MINORS) {
		struct device *dev;
        spidev->devt = MKDEV(SPIDEV_MAJOR, minor);
		dev = device_create(lcd.lcd_class, &spi->dev, spidev->devt,
				    spidev, "tftlcd%d.%d",
				    spi->master->bus_num, spi->chip_select);
		status = IS_ERR(dev) ? PTR_ERR(dev) : 0;
	} else {
		dev_dbg(&spi->dev, "no minor number available!\n");
		status = -ENODEV;
	}
	if (status == 0) {
		set_bit(minor, minors);
		list_add(&spidev->device_entry, &device_list);
	}
    mutex_unlock(&device_list_lock);

    if (status == 0)
		spi_set_drvdata(spi, spidev);
	else
		kfree(spidev);

    save = spi->mode;
    tmp = SPI_MODE_0;

    spi->mode = (u8)tmp;
    spi->bits_per_word = 9;
    spi->chip_select = 0;
    spi->max_speed_hz = 8000000;

    /*spi初始化设置，即使能*/
    retval = spi_setup(spi);
	if (retval < 0)
	{
		spi->mode = save;
		dev_info(&spi->dev, "set up failed,now spi mode %02x\n", spi->mode);
	}
	printk("lcd_spidev_probe is over!!!\n");
	return status;
}



static s32 lcd_spidev_remove(struct spi_device *spi)
{
	struct lcd_spidev_data	*spidev = spi_get_drvdata(spi);

	/* make sure ops on existing fds can abort cleanly */
	spin_lock_irq(&spidev->spi_lock);
	spidev->spi = NULL;
	spi_set_drvdata(spi, NULL);
	spin_unlock_irq(&spidev->spi_lock);

	/* prevent new opens */
	mutex_lock(&device_list_lock);
	list_del(&spidev->device_entry);
	device_destroy(lcd.lcd_class, spidev->devt);
	clear_bit(MINOR(spidev->devt), minors);
	if (spidev->users == 0)
		kfree(spidev);
	mutex_unlock(&device_list_lock);

	return 0;
}




s32 lcd_bus_init(void)
{
    return 0;
}


void lcd_image_data_transfer(dma_addr_t src_addr, u8 *buf, u32 len)
{
    unsigned ret = 0;
    ret = spi_write_data(buf, len); /* data */
	if(ret)
	{
	    hilcd_error("spi write data error!");
	    return;
    }
}

void lcd_data_transfer(u8 *data, u32 data_len)
{
    unsigned ret = 0;
    ret = spi_write_data(data, data_len); /* data */
	if(ret)
	{
	    hilcd_error("spi write data error!");
	    return;
    }
    
}

void lcd_cmd_transfer(u8 *data)
{
    unsigned ret = 0;
    ret = spi_write_cmd(data, 1);  /* command */
	if(ret)
	{
	    hilcd_error("spi write command error!");
	    return;
	}
}

void lcd_bus_disable_clk(void)
{
    
}

void lcd_bus_enable_clk(void)
{
    
    
}


  

void bsp_lcd_bus_init(void)
{
    u32 ret = 0;
    /*定义spi设备驱动*/
	spidrv.driver.name = "balong_lcd_spi";
	spidrv.probe    = lcd_spidev_probe;
	spidrv.remove   = lcd_spidev_remove;
    /*创建lcd类*/
    lcd.lcd_class = class_create(THIS_MODULE, "lcd");
	if (IS_ERR(lcd.lcd_class)) {
        goto exit;

	}

    /*注册spi设备驱动,匹配spi设备，构造master匹配boardlist中的boardinfo*/
	ret = spi_register_driver(&spidrv);
	if (ret < 0) {
		printk(KERN_ERR "couldn't register SPI Interface\n");
        goto spi_unregister;
	}
    return;
    
spi_unregister:
    spi_unregister_driver(&spidrv);
exit:
    class_destroy(lcd.lcd_class);
    return;
}
