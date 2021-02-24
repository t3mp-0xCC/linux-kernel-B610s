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
/*lint --e{537}*/
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/clk.h>
#include <osl_common.h>
#include <linux/delay.h>
//#include "sysctrl_interface.h"
#include "spi.h"

#define SPI_NAME_LEN 20

char* spiBase[2] = {NULL, NULL};

void spi_clk_enable(u32 spiNo)
{
	struct clk *spi_clk = NULL;
	char name[SPI_NAME_LEN] = "";

    if ((0 != spiNo) && (1 != spiNo))
    {
        return ;
    }

	snprintf(name, SPI_NAME_LEN, "dw_ssi%d_clk", spiNo);

	spi_clk = clk_get(NULL, name);
	if (IS_ERR(spi_clk))
	{
		printk(KERN_ERR "spi_clk get fail./n");
		return;
	}

	(void)clk_prepare(spi_clk);
	(void)clk_enable(spi_clk);

}

s32 slic_spi_init(u32 spiNo)
{
	char *base_addr = NULL;
	struct device_node *device = NULL;
	char node_name[32] = "";

    if ((0 != spiNo) && (1 != spiNo))
    {
        return SPI_ERROR;
    }

    snprintf(node_name, 32,"/spi@%d", spiNo);

	device = of_find_node_by_path(node_name);
	if(!device)
	{
		printk(KERN_ERR "snd spi device node is null\n");
		return SPI_ERROR;
	}

	base_addr = (char*)of_iomap(device, 0);
	if (0 == base_addr)
	{
		printk(KERN_ERR "sio remap addr fail\n");
		return SPI_ERROR;
	}

    spiBase[spiNo] = base_addr;

    /*打开clk*/
    spi_clk_enable(spiNo);

    /* 禁止SPI Slave*/
    writel(0,SPI_SLAVE_EN(spiNo));

    /* 禁止SPI Master*/
    writel(0,SPI_EN(spiNo));

    /*spi0*/
    /* 配置ctrl0寄存器，命令字长度为1，数据宽度为8,上升沿触发,低电平有效 */
    writel((0x07<<SPI_CTRL0_DFS_BITPOS)       \
            | (0x0<<SPI_CTRL0_SCPH_BITPOS)    \
            | (0x0<<SPI_CTRL0_SCPOL_BITPOS)   \
            | (0x0<< SPI_CTRL0_FRF_BITPOS)    \
            | (0x00<<SPI_CFS_BITPOS),SPI_CTRL0(spiNo));

    /* 配置SPI波特率为SSI CLK的1/24，即48/24=2MHz */
    writel(24,SPI_BAUD(spiNo));


    /*禁止所有中断信号*/
    writel(0,SPI_IMR(spiNo));

    /*屏蔽DMA传输*/
    writel(0, SPI_DMAC(spiNo));

    return SPI_OK;
}

/*****************************************************************************
* 函 数 名  : slic_spi_recv
*
* 功能描述  : SPI数据轮询接收数据
*
* 输入参数  : spiNo SPI控制器号
*                  cs:   片选号
*                  prevData:   指向要读取数据地址的指针
*                  recvSize：要读取的数据的大小
*                  psendData: 指向要发送的命令和地址的指针
*                  sendSize:  要发送命令和地址的长度(3 或者4)
* 输出参数  :
*
* 返 回 值  : SPI_OK or SPI_ERROR
*
* 其它说明  :
*
*****************************************************************************/
s32 slic_spi_recv (u32 spiNo, u32 cs, u8* prevData, u32 recvSize,u8* psendData,u32 sendSize )
{
    u8 *pRh;
    u8 *pSh;
    u32  i;
    u32 ulLoop = SPI_MAX_DELAY_TIMES;

    if (((0 != spiNo) && (1 != spiNo))
        || (0 != cs && 1 != cs)
        || (NULL == psendData) || (NULL == prevData) || (0 == recvSize) || (0 == sendSize))
    {
        return SPI_ERROR;
    }

    pRh = prevData;
    pSh = psendData;

    /* 禁止SPI Slave*/
    writel(0,SPI_SLAVE_EN(spiNo));

    /* 禁止SPI Master*/
    writel(0, SPI_EN(spiNo));

    /* 设置成SLIC专用读模式 */
    writel(((readl(SPI_CTRL0(spiNo)) & ~SPI_CTRL0_TMOD_BITMASK) | SPI_CTRL0_TMOD_SEND_RECV),SPI_CTRL0(spiNo));

    /* 设置接收数据的数目*/
    writel((recvSize-1),SPI_CTRL1(spiNo));

    /*使能SPI Master*/
    writel(1, SPI_EN(spiNo));

    /*使能SPI Slave*/
    writel((1 << cs), SPI_SLAVE_EN(spiNo));

    /* 发送命令 */
    for(i = 0; i < sendSize; i++)
    {
        /* 等待发送FIFO非满 */
        while(!(readl(SPI_STATUS(spiNo)) & SPI_STATUS_TXNOTFULL)
            && (0 != --ulLoop))
        {
        }

        if(0 == ulLoop)
        {
            return -2;
        }

        writel(*pSh++, SPI_DR(spiNo));
    }

    /*将发送FIFO中的数据全部发出*/
    while(!(readl(SPI_STATUS(spiNo)) & SPI_STATUS_TXEMPTY)
        && (0 != --ulLoop))
    {
    }

    if(0 == ulLoop)
    {
        return -3;
    }

    /* 接收数据 */
    for(i = 0; i < recvSize; i++)
    {
        ulLoop = SPI_MAX_DELAY_TIMES;
        /* 等待读取到数据 */
        while(!(readl(SPI_STATUS(spiNo)) & SPI_STATUS_RXNOTEMPTY)
            && (0 != --ulLoop))
        {

        }

        if(0 == ulLoop)
        {
            return -4;
        }

        *pRh++ = (u8)readl(SPI_DR(spiNo));

    }

    return SPI_OK;
}

/*****************************************************************************
* 函 数 名  : slic_spi_send
*
* 功能描述  : SPI数据轮询发送
*
* 输入参数  : spiNo SPI控制器号
*                  cs:   片选号
*                  pData:   指向要发送数据地址的指针
*                  ulLen：要发送的数据的大小
* 输出参数  :
*
* 返 回 值  : SPI_OK or SPI_ERROR
*
* 其它说明  :
*
*****************************************************************************/
s32 slic_spi_send (u32 spiNo, u32 cs, u8* pData, u32 ulLen)
{
    u8 *pSh;
    u32  i;
    u32 ulLoop = SPI_MAX_DELAY_TIMES;
    u32 ulVal;

    if (((0 != spiNo) && (1 != spiNo))
        || (0 != cs && 1 != cs)
        || (NULL == pData) || (0 == ulLen))
    {
        return SPI_ERROR;
    }

    pSh = (u8*)pData;

    /* 禁止SPI Slave*/
    writel(0, SPI_SLAVE_EN(spiNo));

    /* 禁止SPI Master*/
    writel(0, SPI_EN(spiNo));

    /* 设置成发送模式 */
    writel(((readl(SPI_CTRL0(spiNo)) & ~SPI_CTRL0_TMOD_BITMASK) | SPI_CTRL0_TMOD_SEND),SPI_CTRL0(spiNo));

    /*使能SPI Master*/
    writel(1, SPI_EN(spiNo));

    /*使能SPI Slave*/
    writel((1 << cs), SPI_SLAVE_EN(spiNo));

    /* 发送命令 */
    for(i = 0; i < ulLen; i++)
    {
        /* 等待发送FIFO非满 */
        while(!(readl(SPI_STATUS(spiNo)) & SPI_STATUS_TXNOTFULL)
            && (0 != --ulLoop))
        {
        }

        if(0 == ulLoop)
        {
            return -2;
        }
        writel(*pSh++, SPI_DR(spiNo));
    }

    /*将发送FIFO中的数据全部发出,且不BUSY*/
    ulLoop = SPI_MAX_DELAY_TIMES;
    ulVal = readl(SPI_STATUS(spiNo));
    while(((!(ulVal & SPI_STATUS_TXEMPTY)) || (ulVal & SPI_STATUS_BUSY))
        && (0 != --ulLoop))
    {
        ulVal = readl(SPI_STATUS(spiNo));
    }

    if(0 == ulLoop)
    {
        return -3;
    }

    return SPI_OK;
}



