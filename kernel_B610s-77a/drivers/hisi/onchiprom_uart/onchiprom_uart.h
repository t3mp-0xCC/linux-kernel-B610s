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

#ifndef __TESTHS_H__
#define __TESTHS_H__

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/personality.h>
#include <linux/stat.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/dirent.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/statfs.h>
#include <linux/rcupdate.h>
#include <linux/hrtimer.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
/*--------------------------------------------------------------*
 * 宏定义                                                       *
 *--------------------------------------------------------------*/
 #ifndef OK
 #define OK 0
 #endif

 #ifndef ERROR
 #define ERROR (-1)
 #endif



#define HSUART_BASE_ADDR  0x90026000

/* FCR相关 */
/* 设置发送FIFO水线 */
#define UART_DEF_TET_NULL     0x00 /* FIFO 完全空*/
#define UART_DEF_TET_2CHAR    0x10 /* FIFO 中有2 个字符*/
#define UART_DEF_TET_ONEFOUR  0x20 /* FIFO 四分之一满*/
#define UART_DEF_TET_ONETWO   0x30 /* FIFO 二分之一满*/

/* 设置接收FIFO水线 */
#define UART_DEF_RT_1CHAR     0x00 /* FIFO 中有1 个字符*/
#define UART_DEF_RT_2CHAR     0xc0 /* FIFO 差2 个字符就满*/
#define UART_DEF_RT_ONEFOUR   0x40 /* FIFO 四分之一满*/
#define UART_DEF_RT_ONETWO    0x80 /* FIFO 二分之一满*/

/* DMA Mode */
#define UART_DEF_NO_DMA_MODE  0x00   /* Mode 0 */
#define UART_DEF_DMA_MODE     0x08   /* Mode 1 */

/* 复位发送接收FIFO */
#define UART_RESET_TX_FIFO    0x04
#define UART_RESET_RCV_FIFO   0x02

/* 设置FIFO使能 */
#define UART_DEF_FIFOE_ENABLE   0x1 /* FIFO 使能*/
#define UART_DEF_FIFOE_DISABLE  0x0 /* FIFO 使能*/
#define SIZE_4K 				(4096)

/* 设置LCR */
/* 停止位设置 */
#define UART_DEF_LCR_STP2   0x04  /*在LCR bit[1:0]=0 的条件下为1.5 个停止位，否则为2 个停止位*/
#define UART_DEF_LCR_STP1   0x00  /*1个停止位*/

#define UART_DEF_LCR_PEN    0x08 /* 奇偶校验使能*/
#define UART_DEF_LCR_EPS    0x10 /* 奇偶校验选择*/

#define UART_DEF_PARITY_NONE 0x00 /* 无奇偶校验 */

#define UART_DEF_LCR_DIVENABLE  0x80  /* 分频寄存器使能 */

/* 设置数据流中的每个字符的个数*/
#define UART_DEF_LCR_CS8    0x03 /* 8bit*/
#define UART_DEF_LCR_CS7    0x02 /* 7bit*/
#define UART_DEF_LCR_CS6    0x01 /* 6bit*/
#define UART_DEF_LCR_CS5    0x00 /* 5bit*/

/* 默认波特率 */
//#define HSUART_BAUT_RATE    115200//1000000

#define HSUART_MIN_BAUT_RATE  61
#define HSUART_MAX_BAUT_RATE  3000000
	
/* 时钟频率 */
#define HSUART_CLOCK_FREQ   48000000


#define HSUART_BUFFER_SIZE   1024

#define HSUART_ACK                     0xAA

#define READ_BUFFER_SIZE   448
#define AXI_MEM_BL_ADDR     0x10000000

#define FILE_BOOT_LOADER "/modem_log/usbloader.bin"


#define DL_ACK                     0xAA
#define DL_NAK                     0xA5
#define DL_SEC_NAK                 0xAC
#define DL_DUMMY                   0x00

/*
 * UART module register define
 */
#define UART2_BASE (INTEGRATOR_SC_BASE+0x16000)
#define UART_RBR   0x0
#define UART_THR   0x0
#define UART_DLL   0x0
#define UART_IER   0x4
#define UART_DLH   0x4
#define UART_IIR   0x8
#define UART_FCR   0x8
#define UART_LCR   0xC
#define UART_MCR   0x10
#define UART_LSR   0x14
#define UART_TXDEPTH 0x2C
#define UART_DEBUG 0x28
#define UART_FAR   0x70
#define UART_TFR   0x74
#define UART_RFW   0x78
#define UART_USR   0x7C
#define UART_TFL   0x80
#define UART_RFL   0x84
#define UART_HTX   0xA4
#define UART_DMASA 0xA8
#define UART_CPR   0xF4


/* register control */
#define READ_REGISTER_UCHAR(addr) (*(volatile unsigned char *)(addr))
#define READ_REGISTER_USHORT(addr) (*(volatile unsigned short *)(addr))
#define READ_REGISTER_ULONG(addr) (*(volatile unsigned int *)(addr))

#define WRITE_REGISTER_UCHAR(addr,val) (*(volatile unsigned char *)(addr) = (val))
#define WRITE_REGISTER_USHORT(addr,val) (*(volatile unsigned short *)(addr) = (val))
#define WRITE_REGISTER_ULONG(addr,val) (*(volatile unsigned int *)(addr) = (val))

#define INREG8(x)           READ_REGISTER_UCHAR(x)
#define OUTREG8(x, y)       WRITE_REGISTER_UCHAR(x, (unsigned char)(y))
#define SETREG8(x, y)       OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)       OUTREG8(x, INREG8(x)&~(y))
#define SETBITVALUE8(addr, mask, value) \
                OUTREG8(addr, (INREG8(addr)&(~mask)) | (value&mask))

#define INREG16(x)          READ_REGISTER_USHORT(x)
#define OUTREG16(x, y)      WRITE_REGISTER_USHORT(x,(unsigned short)(y))
#define SETREG16(x, y)      OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)      OUTREG16(x, INREG16(x)&~(y))
#define SETBITVALUE16(addr, mask, value) \
                OUTREG16(addr, (INREG16(addr)&(~mask)) | (value&mask))

#define INREG32(x)          READ_REGISTER_ULONG(x)
#define OUTREG32(x, y)      WRITE_REGISTER_ULONG(x, (unsigned int)(y))
#define SETREG32(x, y)      OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y)      OUTREG32(x, INREG32(x)&~(y))
#define SETBITVALUE32(addr, mask, value) \
                OUTREG32(addr, (INREG32(addr)&(~mask)) | (value&mask))

/*--------------------------------------------------------------*
 * 数据结构                                                     *
 *--------------------------------------------------------------*/
typedef enum tagHSUART_STATUS
{
    HSUART_STATUS_OK = 0,
    HSUART_STATUS_PARA_ERR,
    HSUART_STATUS_SET_BAUTRATE_ERR,
    HSUART_STATUS_SEND_ERR,
    HsUART_STATUS_BUFFER_FULL_ERR
}HSUART_STATUS;

typedef struct hsuart_recv_str
{
    unsigned int ulRead;
    unsigned int ulWrite;
    unsigned int ulTotalNum;
    unsigned char ucData[HSUART_BUFFER_SIZE];
}RECV_STR;

typedef enum _eDlFileType
{
    DL_FILE_TYPE_CFG = 0,
    DL_FILE_TYPE_CODE
}eDlFileType;

typedef enum _eDlFrameType
{
    DL_FRAME_TYPE_FILE = 0x55,     /* FILE帧头字节 */
    DL_FRAME_TYPE_DATA = 0x5C,     /* DATA帧头字节 */
    DL_FRAME_TYPE_END  = 0x5A,     /* END帧头字节 */
    DL_FRAME_TYPE_EXE  = 0x53     /* END帧头字节 */
}eDlFrameType;

#define HDLC_FRAME_FLAG  0x7e        /* HDLC帧标志位，固定为0x7e，区分不同的HDLC帧 */
#define HDLC_ESC         0x7d        /* 转义字符，紧接其后的字符需要转义 */
#define HDLC_ESC_MASK    0x20        /* 转义字符掩字 */
        
#define HDLC_INIT_FCS    0xffff      /* 计算FCS的初始FCS值 */
#define HDLC_GOOD_FCS    0xf0b8      /* 计算结果为该FCS值时，说明FCS校验正确 */

#define HDLC_MODE_HUNT   0x1         /* 解封装模式中bit0为1，表明未找到帧标志，没有开始解封装 */
#define HDLC_MODE_ESC    0x2         /* 解封装模式中bit1为1，表明上个字符为转义字符，当前字符为真正数据的掩字 */
        
#define HDLC_FCS_LEN     2           /* HDLC帧中FCS未转义时的长度(单位: Byte) */

#define HDLC_BUF_SIZE    1024

typedef struct tagHSUART_DEV_S
{
    unsigned int phy_addr_base;
    volatile void __iomem * vir_addr_base;
	int irq;
    unsigned int clock;
	unsigned int reg_size;
}hsuart_dev;

typedef enum _eHdlcState
{
    HDLC_OK = 0,                /* HDLC处理成功 */
    HDLC_PARA_ERR,              /* 参数检查异常 */
    HDLC_ERR_BUF_FULL,
    HDLC_ERR_INVALAID_FRAME,    /* 没有完整的帧 */
    HDLC_ERR_FCS,               /* 发生了校验错误 */
    HDLC_ERR_DISCARD            /* 发生异常，丢弃当前HDLC帧 */
}eHdlcState;

typedef struct tagDLOAD_HDLC_S
{
    unsigned int  u32Mode;                       /* HDLC解析使用的内部参数，HDLC负责维护 */
    unsigned int  u32Length;                     /* HDLC解析时内部维护的中间变量 */
    unsigned int  u32DecapBufSize;               /* HDLC解析内容存放BUFFER的长度，调用者负责指定 */
    unsigned int  u32InfoLen;                    /* HDLC解析成功的数据长度，HDLC负责维护 */
    unsigned char au8DecapBuf[HDLC_BUF_SIZE];    /* HDLC解析内容存放地址，由调用者负责指定空间 */
}DLOAD_HDLC_S;

typedef enum tagBaudRateE
{
	BAUDRATE_115200 = 115200,
	BAUDRATE_9600   = 9600,
	BAUDRATE_1M     = 1000000,
	BAUDRATE_2M     = 2000000,
	BAUDRATE_3M     = 3000000,
	BAUDRATE_4M     = 4000000
}baudRateE;

/* 包类型定义 */
typedef enum _tagPktTypeE
{
	PKT_TYPE_CONNECT    = 0x00,
	PKT_TYPE_REP        = 0x01,

	PKT_TYPE_BAUD_SET   = 0x10,
	PKT_TYPE_AP_CMD     = 0x11,
	PKT_TYPE_STAT_DATA  = 0x12,
	PKT_TYPE_MIDST_DATA = 0x13,
	PKT_TYPE_END_DATA   = 0x14,
	PKT_TYPE_EXEC_DATA  = 0x15,
	PKT_TYPE_MAX
}pktTypeE;

/* 包长定义,包长为除去HDLC协议头/尾/CRC部分 */
#define PKT_LEN_CONNECT     3
#define PKT_LEN_BAUD_SET    7
#define PKT_LEN_AP_CMD      4
#define PKT_LEN_START_DATA  11
#define PKT_LEN_MIDST_DATA  (DATA_PKT_LEN + 4)
#define PKT_LEN_END_DATA    3
#define PKT_LEN_EXEC_DATA   3
#define PKT_LEN_REP         4

/* 指令包指令定义 */
typedef enum _tagApCmdE
{
	AP_CMD_DL_BL_BURN_PRE = 0x20,   /* 下载BootLoader或烧写引导程序 */
	AP_CMD_NAND_BOOT = 0x21        /* 直接从Nand启动 */
}apCmdE;

/* 应答包错误码定义 */
typedef enum _tagRepCodeE
{
	REP_ACK = 0x80,
	REP_BAUD_NOT_SUPPORT,
	REP_AP_CMD_NOT_SUPPORT,
	REP_NAND_READ_ERR,
	REP_SEC_CHK_ERR,
	REP_SEQ_BIG,
	REP_FILE_RECV_ERR,
	REP_VERIFY_ERROR,
	REP_ERROR_CHKSUM,
	REP_CHKSUM_DIFF
}RepCodeE;

/* 下载交互流程错误码定义 */
typedef enum _tagApDlStatusE
{
	AP_DL_ERR_OK = 0,
	AP_DL_ERR_HDLC_ENCAP,
	AP_DL_ERR_HDLC_DECAP,
	AP_DL_ERR_RECV,
	AP_DL_ERR_MAX
}apDlStatusE;

#define PKT_TYPE_OS     0
#define PKT_TYPE_LEN    1


#define BAUD_SET_OS     (PKT_TYPE_OS+PKT_TYPE_LEN)
#define AP_CMD_OS       (PKT_TYPE_OS+PKT_TYPE_LEN)

#define START_DATA_FILELEN_OS   (PKT_TYPE_OS+PKT_TYPE_LEN)
#define FILELEN_LEN     4
#define START_DATA_ADDR_OS   (START_DATA_FILELEN_OS+FILELEN_LEN)

#define PKT_LEN_OS      (PKT_TYPE_OS+PKT_TYPE_LEN)
#define PKT_LEN_LEN     2
#define MIDST_DATA_SEQ_OS   (PKT_LEN_OS+PKT_LEN_LEN)
#define MIDST_DATA_SEQ_LEN  1
#define MIDST_DATA_OS       (MIDST_DATA_SEQ_OS+MIDST_DATA_SEQ_LEN)

#define REP_CODE_OS     (PKT_TYPE_OS+PKT_TYPE_LEN)



/*--------------------------------------------------------------*
 * 函数原型声明                                                 *
 *--------------------------------------------------------------*/

HSUART_STATUS hsUartInit(unsigned int BautRate);
HSUART_STATUS hsUartRecv(RECV_STR *pstHsUartRecvData);
HSUART_STATUS hsUartSend(unsigned char* pucSrcAddr, unsigned int u32TransLength);
int OutQue(RECV_STR *pstQue, unsigned char *pBuf,unsigned int ulLong);
extern s32 bsp_open(const s8 * path, s32 flags, s32 mode);
extern s32 bsp_close(u32 fd);
extern s32 bsp_read(u32 fd, s8 * ptr, u32 size);
extern s32 bsp_write(u32 fd, const s8 * ptr, u32 size);
extern long bsp_tell(u32 fd);
extern s32 bsp_lseek(u32 fd, long offset, s32 whence);
#endif /* hsuart.h */

