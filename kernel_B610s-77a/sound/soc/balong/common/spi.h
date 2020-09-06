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


#ifndef __SPI_H__
#define __SPI_H__

//#include <balongv7r2/types.h>
#include <hi_spi.h>
#include <bsp_memmap.h>

#define DUMMY_DATA  0x00

#define ECS_SPI0_BASE HI_SPI_MST0_REGBASE_ADDR_VIRT/*SPI0控制器基址*/
#define ECS_SPI1_BASE HI_SPI_MST1_REGBASE_ADDR_VIRT /*SPI1控制器基址*/

#define SPI_BASE(x)        (spiBase[x])   /* SPI基地址 */
#define SPI_CTRL0(x)       (SPI_BASE(x) + HI_SPI_CTRLR0_OFFSET)
/*配置Ctrl0寄存器时，各成员的偏移*/
#define SPI_CTRL0_DFS_BITPOS        0   /* 数据长度 */
#define SPI_CTRL0_FRF_BITPOS        4   /* 帧类型 */
#define SPI_CTRL0_SCPH_BITPOS       6   /* 时钟相位 */
#define SPI_CTRL0_SCPOL_BITPOS      7   /* 时钟极性 */

#define SPI_CTRL0_TMOD_BITPOS       8   /* 传输模式设置 */
#define SPI_CTRL0_TMOD_BITWIDTH     2
#define SPI_CTRL0_TMOD_BITMASK     (((1<<SPI_CTRL0_TMOD_BITWIDTH)-1)<<SPI_CTRL0_TMOD_BITPOS)
#define SPI_CTRL0_TMOD_SEND_RECV    (0x00<<SPI_CTRL0_TMOD_BITPOS)   /* 收发模式 */
#define SPI_CTRL0_TMOD_SEND         (0x01<<SPI_CTRL0_TMOD_BITPOS)   /* 发送模式 */
#define SPI_CTRL0_TMOD_RECV         (0x02<<SPI_CTRL0_TMOD_BITPOS)   /* 接收模式 */
#define SPI_CTRL0_TMOD_EEPROM_READ  (0x03<<SPI_CTRL0_TMOD_BITPOS)   /* EEPROM读模式 */

#define SPI_CFS_BITPOS              12  /* 控制帧大小 */

#define SPI_CTRL1(x)      		 (SPI_BASE(x) + HI_SPI_CTRLR1_OFFSET)
#define SPI_EN(x)         		 (SPI_BASE(x) + HI_SPI_SSIENR_OFFSET)
#define SPI_MWCTRL(x)     	     (SPI_BASE(x) + HI_SPI_MWCR_OFFSET  )
#define SPI_SLAVE_EN(x)   	     (SPI_BASE(x) + HI_SPI_SER_OFFSET   )
#define SPI_BAUD(x)       		 (SPI_BASE(x) + HI_SPI_BAUDR_OFFSET )
#define SPI_TXFTL(x)      		 (SPI_BASE(x) + HI_SPI_TXFTLR_OFFSET)
#define SPI_RXFTL(x)      		 (SPI_BASE(x) + HI_SPI_RXFTLR_OFFSET)
#define SPI_TXFL(x)       		 (SPI_BASE(x) + HI_SPI_TXFLR_OFFSET )
#define SPI_RXFL(x)       		 (SPI_BASE(x) + HI_SPI_RXFLR_OFFSET )
#define SPI_STATUS(x)     		 (SPI_BASE(x) + HI_SPI_SR_OFFSET    )
#define SPI_IMR(x)        		 (SPI_BASE(x) + HI_SPI_IMR_OFFSET   )
#define SPI_INT_STATUS(x) 		 (SPI_BASE(x) + HI_SPI_ISR_OFFSET   )
#define SPI_RX_SAMPLE_DLY(x)     (SPI_BASE(x) + HI_SPI_RX_SAMPLE_DLY_OFFSET)

#define SPI_STATUS_RXFULL         (1 << 4)
#define SPI_STATUS_RXNOTEMPTY     (1 << 3)
#define SPI_STATUS_TXEMPTY        (1 << 2)
#define SPI_STATUS_TXNOTFULL      (1 << 1)
#define SPI_STATUS_BUSY           (1 << 0)

#define SPI_RAW_INT_STATUS(x)  (SPI_BASE(x) + HI_SPI_RISR_OFFSET            )
#define SPI_TXO_INT_CLEAR(x)   (SPI_BASE(x) + HI_SPI_TXOICR_OFFSET          )
#define SPI_RXO_INT_CLEAR(x)   (SPI_BASE(x) + HI_SPI_RXOICR_OFFSET          )
#define SPI_RXU_INT_CLEAR(x)   (SPI_BASE(x) + HI_SPI_RXUICR_OFFSET          )
#define SPI_MST_INT_CLEAR(x)   (SPI_BASE(x) + HI_SPI_MSTICR_OFFSET          )
#define SPI_INT_CLEAR(x)       (SPI_BASE(x) + HI_SPI_ICR_OFFSET             )
#define SPI_DMAC(x)            (SPI_BASE(x) + HI_SPI_DMACR_OFFSET           )
#define SPI_DMATDL(x)          (SPI_BASE(x) + HI_SPI_DMATDLR_OFFSET         )
#define SPI_DMARDL(x)          (SPI_BASE(x) + HI_SPI_DMARDLR_OFFSET         )
#define SPI_ID(x)              (SPI_BASE(x) + HI_SPI_IDR_OFFSET             )
#define SPI_COMP_VERSION(x)    (SPI_BASE(x) + HI_SPI_SSI_COMP_VERSION_OFFSET)
#define SPI_DR(x)              (SPI_BASE(x) + HI_SPI_DR_OFFSET              )

/* for porting test */
/* Porting E2PROM使用SPI0 CS0 ; SFLASH使用SPI0 CS1*/
#define SPI_NO_DEV          0        /*SPI0*/
#define SPI_CS_EEPROM       (1<<0)   /* EEPROM使用SSI0 CS0 */
#define SPI_CS_SFLASH       (1<<1)   /* SFLASH使用SSI0 CS1 */
#define SPI_CS_DEV          SPI_CS_SFLASH/*硬件实际连接的设备*/
/* EEPROM和串行Flash通用命令 */
#define SPI_DEV_CMD_WRSR	0x1     /*Write Status Register*/
#define SPI_DEV_CMD_WRITE	0x2     /*Write Data to Memory Array*/
#define SPI_DEV_CMD_READ	0x3     /*Read Data from Memory Array*/
#define SPI_DEV_CMD_WRDI	0x4     /*Reset Write Enable Latch*/
#define SPI_DEV_CMD_RDSR	0x5     /*Read Status Register*/
#define SPI_DEV_CMD_WREN	0x6     /*Set Write Enable Latch*/
/* 串行Flash特有命令 */
#define SPI_DEV_CMD_RDID	0x9F    /* Read Identification */
#define SPI_DEV_CMD_PP		0x02    /* 页编程 */
#define SPI_DEV_CMD_SE		0xD8    /* 块擦除 */
#define SPI_DEV_CMD_BE		0xC7    /* Bulk Erase */
/* 状态寄存器状态位 */
#define SPI_DEV_STATUS_WIP  (1<<0)  /* Write In Progress Bit */
#define SPI_DEV_STATUS_WEL  (1<<1)  /* Write Enable Latch Bit */
#define SPI_DEV_STATUS_BP0  (1<<2)  /* Block Protect Bits */
#define SPI_DEV_STATUS_BP1  (1<<3)  /* Block Protect Bits */
#define SPI_DEV_STATUS_BP2  (1<<4)  /* Block Protect Bits */
#define SPI_DEV_STATUS_SRWD  (1<<7)  /* Status Register Write Protect */

/*for ASIC*/
/* SLIC使用SPI0 CS0 */
#define SPI_NO_SLIC          	0
#define SPI_CS_SLIC          	1     /* CS0,for slic */

/* PMU使用SPI0 CS0 */
#define SPI_NO_PMU          	0
#define SPI_CS_PMU          	(1<<0)     /* CS0,for hi6451 */

/* LCD使用SPI1 CS0 */
#define SPI_NO_LCD          	1
#define SPI_CS_LCD          	(1<<0)     /* CS0,for LCD */

#define SPI_MAX_DELAY_TIMES 0x10000	/*最大延迟时间*/

/*错误码*/
#define SPI_OK        		0
#define SPI_ERROR          -1

typedef enum eSpiDevType_t
{
    E_SPI_DEV_TYPE_EEPROM = 0,
    E_SPI_DEV_TYPE_SFLASH
}eSpiDevType;

/*函数声明*/
s32 slic_spi_init (u32 spiNo);
s32 slic_spi_recv (u32 spiNo, u32 cs, u8* prevData, u32 recvSize,u8* psendData,u32 sendSize );
s32 slic_spi_send (u32 spiNo, u32 cs, u8* pData, u32 ulLen);

#endif

