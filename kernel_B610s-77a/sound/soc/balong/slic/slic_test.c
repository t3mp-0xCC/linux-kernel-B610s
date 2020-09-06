#include <linux/module.h>
#include "spi.h"
#include "snd_ctl.h"

#if (ZSI_PCLK_2M == ZSI_PCLK_MODE)
#define dtime              0x06
#elif (ZSI_PCLK_8M == ZSI_PCLK_MODE)
#define dtime              0x02
#endif
void slic_register_read(u8 reg)
{
    s32 ret    = 0;
    s32  i     = 0;
    u8  buff[3]= {0x4A,0x1,0x06}; /*write, channel, no_ops*/
    u8  data   = 0x0;
    if(0x0 == (reg & 0x1))
    {
        printk(KERN_ERR "reg is write_only, not allowed read, reg=0x%x\n", reg);
        return ;
    }

    for(i = 0;i < 2;i++){
        ret = slic_spi_send (0x0, SPI_CS_SLIC, &buff[i], 0x1);
        if(0 != ret)
        {
            printk(KERN_ERR "send data 0x%x, ret %d", buff[i], ret);
            return;
        }
        udelay(dtime);
    }

    ret = slic_spi_send (0x0, SPI_CS_SLIC, &reg, 0x1); /*register address*/
    if(0 != ret)
    {
        printk(KERN_ERR "send data 0x%x, ret %d", reg, ret);
        return;
    }

    udelay(dtime);

    ret = slic_spi_send (0x0, SPI_CS_SLIC, &buff[2], 0x1);
    if(0 != ret)
    {
        printk(KERN_ERR "recv data 0x%x, ret %d\n", buff[2], ret);
    }

    udelay(dtime);

    ret = slic_spi_recv (0x0, SPI_CS_SLIC, &data,0x1,&buff[2],0x1); /* register data*/
    if(0 != ret)
    {
        printk(KERN_ERR "recv data 0x%x, ret %d\n", buff[2],  ret);

    }

    printk(KERN_ERR "read data 0x%x\n", data);
}

void slic_register_write(u8 reg,u8 data)
{
    s32 ret    = 0;
    u8  buff[3]={0x4A,0x1,0x06}; /*write, channel, no_ops*/
    u8  i = 0;


    if(0x1 == (reg & 0x1))
    {
        printk(KERN_ERR "data is read_only, not allowed write, data=0x%x\n", reg);
        return ;
    }


    for(i = 0;i < 2;i++){
        ret = slic_spi_send (0x0,SPI_CS_SLIC, &buff[i], 0x1);
        if(0 != ret)
        {
            printk(KERN_ERR "send data 0x%x, ret %d", data, ret);
            return;
        }
        udelay(dtime);
    }

    ret = slic_spi_send (0x0, SPI_CS_SLIC, &reg, 0x1); /*register address*/
    if(0 != ret)
    {
        printk(KERN_ERR "send data 0x%x, ret %d", reg, ret);
        return;
    }
    udelay(dtime);
    ret = slic_spi_send (0x0, SPI_CS_SLIC, &data, 0x1); /* register data*/
    if(0 != ret)
    {
        printk(KERN_ERR "send data 0x%x, ret %d", data, ret);
        return;
    }
    printk(KERN_ERR "write register[0x%x] data[0x%x] ok.\n",reg,data);

}

void slic_spi_send_recv_test(void)
{
    u8 s_data[6] = {0x4a, 0x1, 0x40, 0x7/*data*/, 0x41, 0x6};
    u8 r_data[2] = {0, 0};
    int i = 0;
    int ret = slic_spi_send(0, SPI_CS_SLIC, &s_data[0], 1);
    if(0 != ret)
    {
        printk(KERN_ERR "s_data 0x%x, ret %d", s_data[0], ret);
    }

    for(i = 0;i < 6;i++)
    {
        udelay(dtime);
        ret = slic_spi_send(0, SPI_CS_SLIC, &s_data[i], 1);
        if(0 != ret)
        {
            printk(KERN_ERR "send data 0x%x, ret %d", s_data[1], ret);
            return;
        }
    }

    for(i = 0;i < 2;i++)
    {
        udelay(dtime);
        ret = slic_spi_recv(0, SPI_CS_SLIC, &r_data[i],1,&s_data[5],1);
        if(0 != ret)
        {
            printk(KERN_ERR "recv data 0x%x, ret %d\n", r_data[i],  ret);
        }
    }

    for(i = 0;i < 2;i++)
    {
        printk(KERN_ERR "slic data 0x%x.\n", r_data[i]);
    }
}

void slic_spi_version_test(void)
{
    u8 s_data[3] = {0x4, 0x73/*ver*/, 0x6};
    u8 r_data[3] = {0, 0, 0};
    int i = 0;
    int ret = 0;

    for(i = 0;i < 3;i++)
    {
        ret = slic_spi_send (0, SPI_CS_SLIC, &s_data[i], 1);
        if(0 != ret)
        {
            printk(KERN_ERR "send data 0x%x, ret %d", s_data[i], ret);
            return;
        }
        udelay(dtime);
    }

    for(i = 0;i < 3;i++)
    {
        udelay(dtime);
        ret = slic_spi_recv (0, SPI_CS_SLIC, &r_data[i],1,&s_data[2],1);
        if(0 != ret)
        {
            printk(KERN_ERR "recv data 0x%x, ret %d\n", r_data[i],  ret);
        }
    }

    for(i = 0;i < 2;i++)
    {
        printk(KERN_ERR "slic version 0x%x.\n", r_data[i]);
    }

}

#endif



