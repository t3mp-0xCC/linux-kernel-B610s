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
#include <linux/delay.h>
#include "osl_io.h"
#include "bsp_rfile.h"
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/amba/serial.h>
#include "onchiprom_uart.h"

hsuart_dev onchiprom_uart = {0};

HSUART_STATUS hsUartSend(unsigned char * pucSrcAddr, unsigned int u32TransLength);
int HSUART_RecvData(RECV_STR *pstQue, unsigned char *pBuf, unsigned int ulLong);
eHdlcState HDLC_Encap_Test(const unsigned char *pu8Src, unsigned short u16SrcLen, unsigned char *pu8Dst,
                      unsigned short u16DestBufLen, unsigned short *pu16DstLen);
eHdlcState HDLC_Decap_Test(DLOAD_HDLC_S *pstHdlc, unsigned char ucChar);

const unsigned short g_au16HdlcFcsTab[256] = 
{
   /* 00 */ 0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
   /* 08 */ 0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
   /* 10 */ 0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
   /* 18 */ 0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
   /* 20 */ 0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
   /* 28 */ 0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
   /* 30 */ 0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
   /* 38 */ 0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
   /* 40 */ 0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
   /* 48 */ 0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
   /* 50 */ 0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
   /* 58 */ 0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
   /* 60 */ 0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
   /* 68 */ 0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
   /* 70 */ 0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
   /* 78 */ 0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
   /* 80 */ 0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
   /* 88 */ 0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
   /* 90 */ 0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
   /* 98 */ 0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
   /* a0 */ 0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
   /* a8 */ 0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
   /* b0 */ 0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
   /* b8 */ 0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
   /* c0 */ 0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
   /* c8 */ 0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
   /* d0 */ 0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
   /* d8 */ 0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
   /* e0 */ 0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
   /* e8 */ 0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
   /* f0 */ 0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
   /* f8 */ 0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};


DLOAD_HDLC_S stHdlcCtrl = {0};

int IsQueNotFull(RECV_STR *pstQue)
{    
    if(NULL == pstQue)
    {
        return ERROR;
    }

    if(pstQue->ulTotalNum < HSUART_BUFFER_SIZE)
        return OK;
    else
        return ERROR;
}

int InQue(RECV_STR *pstQue, unsigned char ucData)
{
    unsigned int ulTail = 0;

    if(NULL == pstQue)
    {
        return ERROR;
    }

    if(ERROR == IsQueNotFull(pstQue))
    {
        return ERROR;
    }
    ulTail = pstQue->ulWrite;
    pstQue->ucData[ulTail] = ucData;
    ulTail = ((ulTail+1) ==  HSUART_BUFFER_SIZE)?0:(ulTail+1);    
    pstQue->ulTotalNum++;
    pstQue->ulWrite = ulTail;
    return OK;
}

int OutQue(RECV_STR *pstQue, unsigned char *pBuf,unsigned int ulLong)
{
    unsigned int ulRead = 0;

    if((NULL == pstQue) || (NULL == pBuf) || (0 == ulLong))
    {
        return ERROR;
    }

    if (pstQue->ulTotalNum < ulLong)
    {
        return  ERROR;
    }

    ulRead = pstQue->ulRead;

    while(ulLong)
    {
        *pBuf = pstQue->ucData[ulRead];        
        ulRead = ((ulRead+1) == HSUART_BUFFER_SIZE)?0:(ulRead+1);
        pstQue->ulTotalNum--;
        ulLong--;
        pBuf++;
    }
    pstQue->ulRead = ulRead;

    return OK;
}

unsigned short calcCRC16(unsigned char *Packet, unsigned int len)
{
    const static unsigned short CRC16_TABLE[256] = {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 
        0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 
        0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 
        0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 
        0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 
        0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d, 
        0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 
        0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 
        0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 
        0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 
        0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 
        0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 
        0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 
        0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49, 
        0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 
        0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 
        0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 
        0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067, 
        0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 
        0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 
        0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 
        0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 
        0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 
        0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634, 
        0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 
        0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3, 
        0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 
        0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 
        0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 
        0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 
        0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 
        0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0, 
    };
    unsigned short CRC16 = 0x0000;
    unsigned int i;

    for (i = 0; i < len; i++)
    {
        CRC16 = ((CRC16<<8)|Packet[i])^CRC16_TABLE[(CRC16>>8)&0xFF];
    }

    for (i = 0; i < 2; i++)
    {
        CRC16 = ((CRC16<<8)|0)^CRC16_TABLE[(CRC16>>8)&0xFF];
    }

    return CRC16;
}

unsigned short crcChk(unsigned char *buf, unsigned int len, unsigned short crc)
{
    unsigned short usCrc;
    
    usCrc = calcCRC16(buf, len);
    
    if(crc == usCrc)
    {
        return 1;
    }
    else
    {
        printk("\r\n in crc = %x", crc);
        printk("\r\n cul crc = %x", usCrc);
        return 0;
    }    
}

int recvFrame(RECV_STR *pstQue, DLOAD_HDLC_S *pHdlcCtrl)
{
	unsigned int iRet;
    unsigned char recvBuf;

	while (1)
	{
        if(HSUART_RecvData(pstQue, &recvBuf, 1) != OK)
        {
            return ERROR;
        }

		printk(" %02x ",recvBuf);

		iRet = HDLC_Decap_Test(pHdlcCtrl, recvBuf);

		if(HDLC_OK == iRet)
		{
			return HDLC_OK;
		}
	}
	
}

int sendFrame(unsigned char *pbuf, unsigned int ulFrameLen, RECV_STR *pstQue)
{
	unsigned int i = 0;
    unsigned int iRet;
    unsigned short hdlcPktLen;

	/* HDLC格式封装 */
	iRet = HDLC_Encap_Test(pbuf, (unsigned short)ulFrameLen, stHdlcCtrl.au8DecapBuf, (unsigned short)HDLC_BUF_SIZE, &hdlcPktLen);

	if (HDLC_OK != iRet)
	{
		printk("HDLC_Encap_Test err: %#x", iRet);
		return ERROR;
	}

	printk("\nulFrameLen = %d\n", hdlcPktLen);

	for(i=0;i < hdlcPktLen;i++)
	{
		printk(" %02x ",stHdlcCtrl.au8DecapBuf[i]);
	}

    if(hsUartSend(stHdlcCtrl.au8DecapBuf, hdlcPktLen) != HSUART_STATUS_OK)
    {
        return ERROR;
    }

	printk("\nsend ok\n");

    if (HDLC_OK == recvFrame(pstQue, &stHdlcCtrl))
    {
		if (REP_ACK != stHdlcCtrl.au8DecapBuf[REP_CODE_OS])
		{
			printk("REP_ACK not recved, recved: %#x\n", stHdlcCtrl.au8DecapBuf[REP_CODE_OS]);
			return ERROR;
		}

		printk("ok\n");
		return OK;
    }

    return OK;
}
/*****************************************************************************
* 函 数 名  : hsUartSetBaud
*
* 功能描述  : HS UART设置波特率接口函数
*
* 输入参数  : u32  u32BaudRate      需要设置的波特率
*
* 输出参数  : 无
* 返 回 值  : OK       成功
*             ERROR    失败
*
* 修改记录  :2011年7月15日   鲁婷  创建
*****************************************************************************/
HSUART_STATUS hsUartSetBaud(u32 u32BaudRate)
{
	unsigned int ibrd,fbrd;
	unsigned int regval = 0;

   	while(UART01x_FR_BUSY & readl(onchiprom_uart.vir_addr_base + UART01x_FR))
       barrier();

    ibrd = onchiprom_uart.clock / (16 * u32BaudRate);
    writel(ibrd, onchiprom_uart.vir_addr_base + UART011_IBRD);
    fbrd = onchiprom_uart.clock % (16 * u32BaudRate);
    fbrd *= (1UL << 6);
    fbrd *= 2;
    fbrd += 16 * u32BaudRate;
    fbrd /= (u32)(2 * 16 * u32BaudRate);
    writel(fbrd, onchiprom_uart.vir_addr_base + UART011_FBRD);
	regval = readl(onchiprom_uart.vir_addr_base + UART011_LCRH);
	writel(regval, onchiprom_uart.vir_addr_base + UART011_LCRH);
	
	
    return HSUART_STATUS_OK;
}

/*****************************************************************************
* 函 数 名  : hsUartSend
*
* 功能描述  : HS UART发送数据接口函数
*
* 输入参数  : u32  u32SrcAddr       需发送的数据的首地址
*             u32  u32TransLength   需发送的数据的长度
*
* 输出参数  : 无
* 返 回 值  : OK       成功
*             ERROR    失败
*
* 修改记录  :2010年12月16日   鲁婷  创建
*****************************************************************************/
/*****************************************************************************
* 函 数 名  : HSUART_Init
*
* 功能描述  : HS UART的基本参数的初始化
*
* 输入参数  : HSUART_DEV * pHsUartDev   设备结构体指针
*
* 输出参数  : 无
* 返 回 值  : 无
*             
*
* 修改记录  :2010年12月16日   鲁婷  创建
*****************************************************************************/
HSUART_STATUS hsUartInit(unsigned int BautRate)
{
     /* disable uart */
    writel(0, (volatile void *)(onchiprom_uart.vir_addr_base) + UART011_CR);
    /* Clear all interrupts */
    writel(0, (volatile void *)(onchiprom_uart.vir_addr_base) + UART011_IMSC);
	writel(0xFF, (volatile void *)(onchiprom_uart.vir_addr_base+ UART011_ICR));
    writel(UART011_IFLS_RX4_8, (volatile void *)(onchiprom_uart.vir_addr_base) + UART011_IFLS);

	hsUartSetBaud(BautRate);
	
	while(UART01x_FR_BUSY & readl((volatile void *)(onchiprom_uart.vir_addr_base) + UART01x_FR))
		barrier();
	
	writel(UART01x_LCRH_FEN | UART01x_LCRH_WLEN_8, (volatile void *)(onchiprom_uart.vir_addr_base) + UART011_LCRH);
	writel(UART011_RTIM | UART011_RXIM, onchiprom_uart.vir_addr_base + UART011_IMSC);
	writel(UART01x_CR_UARTEN | UART011_CR_RXE | UART011_CR_TXE , (volatile void *)(onchiprom_uart.vir_addr_base) + UART011_CR);	

    return HSUART_STATUS_OK;
}


HSUART_STATUS hsUartSend(unsigned char * pucSrcAddr, unsigned int u32TransLength)
{
    unsigned char * pu8Buffer;
    unsigned int regval;
	unsigned int u32Size;

    /* 参数的有效性检查 */
    if((NULL == pucSrcAddr) || (0 == u32TransLength))
    {
    	return HSUART_STATUS_PARA_ERR;
    }
	pu8Buffer = pucSrcAddr;
	u32Size = u32TransLength;
    while(u32Size)
    {
        regval = readl(onchiprom_uart.vir_addr_base + UART01x_FR);	
        if(0 == (regval & UART01x_FR_TXFF))
        {
        	writel(*pu8Buffer, onchiprom_uart.vir_addr_base + UART01x_DR);
            pu8Buffer++;
			u32Size--;
		}
    }
        
    return HSUART_STATUS_OK;
}

HSUART_STATUS hsUartRecv(RECV_STR *pstHsUartRecvData)
{
    unsigned int status;
    int pass_counter = 32;
    char ch;  


	status = readl(onchiprom_uart.vir_addr_base + UART011_MIS);
	if (status) {
		do{
			writel(status & ~(UART011_TXIS|UART011_RTIS|
					UART011_RXIS), onchiprom_uart.vir_addr_base + UART011_ICR);
			if (status & (UART011_RTIS|UART011_RXIS)) 
			{
			    do {
			        if (0x10 != (readl(onchiprom_uart.vir_addr_base + UART01x_FR) & 0x10)) 
					{
			            ch = (char)readl(onchiprom_uart.vir_addr_base + UART01x_DR);
						if(InQue(pstHsUartRecvData, ch) != OK)
                    	{
                        	printk("\r\nbuffer full\r\n");
                        	return HsUART_STATUS_BUFFER_FULL_ERR;
                    	}
			        }
			        else
			            break;
			    }while(--pass_counter > 0);
			}
			status = readl(onchiprom_uart.vir_addr_base + UART011_MIS);
		} while (status != 0);
	}    
    return HSUART_STATUS_OK;
}

int HSUART_RecvData(RECV_STR *pstQue, unsigned char *pBuf, unsigned int ulLong)
{
    if((NULL == pstQue) || (NULL == pBuf) || (0 == ulLong))
    {
        return ERROR;
    }
    
    while(pstQue->ulTotalNum < ulLong)
    {
        if(hsUartRecv(pstQue) != HSUART_STATUS_OK)
        {
            return ERROR;
        }
    }
    if(OutQue(pstQue, pBuf, ulLong) != OK)
    {
        return ERROR;
    }
    
    return OK;
}
/*****************************************************************************
* 函 数 名  : HDLC_Fcs_Test
*
* 功能描述  : 计算FCS. 见RFC 1662 Appendix C and CCITT X.25 section 2.27.
*
* 输入参数  : UINT8 *pucData    
*             u32 ulDataLen  
* 输出参数  : 
*
* 返 回 值  : 
*
* 其它说明  : 
*
*****************************************************************************/
unsigned short HDLC_Fcs_Test(unsigned char *pucData, unsigned int ulDataLen)
{
    unsigned short u16Fcs = HDLC_INIT_FCS;

    while (ulDataLen--)
    {
        u16Fcs = (u16Fcs >> 8) ^ g_au16HdlcFcsTab[(u16Fcs ^ *pucData++) & 0xff];
    }

    return u16Fcs;
}

/************************************************************************/
/* 获取文件长度，调用前保证fp已打开                                     */
/************************************************************************/
unsigned int getFileLen(u32 fp)
{
    unsigned int ulFileSize;
	int ret = 0;
    if (0 == fp)
    {
        printk("fp is NULL, in function getFileLen");
        return 0;
    }

    ret = bsp_lseek(fp, 0, SEEK_END);
	printk("ret %d",ret);
	 if(OK != ret)
    {
        printk("!!!!!!!! BSP_fseek 2 failed .\n");
    }
	ulFileSize = (unsigned int)bsp_tell(fp);
	printk("file len %d",ulFileSize);
    bsp_lseek(fp, 0, SEEK_SET);

    return ulFileSize;
}

/*****************************************************************************
* 函 数 名  : HDLC_Encap_Test
*
* 功能描述  : 将输入的原始数据封装成HDLC帧
*
* 输入参数  : const UINT8 *pu8Src   
*             UINT16 u16SrcLen       
*             UINT8 *pu8Dst        
*             UINT16 u16DestBufLen  
*             UINT16 *pu16DstLen    
* 输出参数  : 
*
* 返 回 值  : 
*
* 其它说明  : NA
*
*****************************************************************************/
eHdlcState HDLC_Encap_Test(const unsigned char *pu8Src, unsigned short u16SrcLen, unsigned char *pu8Dst,
                      unsigned short u16DestBufLen, unsigned short *pu16DstLen)
{
    unsigned short u16Fcs = HDLC_INIT_FCS;
    unsigned char *pu8DstPos = pu8Dst;
    unsigned char u8FcsChar = 0;

    if ((NULL == pu8Src) 
        ||(NULL == pu8Dst) 
        ||(NULL == pu16DstLen))
    {
        return HDLC_PARA_ERR;
    }

    if (0 == u16SrcLen)
    {
        *pu16DstLen = 0;
        return HDLC_PARA_ERR;
    }
    
    /* 信息域长度不为0的HDLC帧长度至少为5 */
    if (u16DestBufLen <= 4)
    {
        *pu16DstLen = 0;
        return HDLC_PARA_ERR;
    }

    /* 填帧头 */
    *pu8DstPos++ = HDLC_FRAME_FLAG;

    /* 遍历输入数据，计算FCS并转义 */
    while (u16SrcLen-- && ((pu8DstPos - pu8Dst) <= (u16DestBufLen - 3)))
    {
        u16Fcs = (u16Fcs >> 8) ^ g_au16HdlcFcsTab[(u16Fcs ^ *pu8Src) & 0xff];

        if ((HDLC_FRAME_FLAG == *pu8Src) || (HDLC_ESC == *pu8Src))
        {
            *pu8DstPos++ = HDLC_ESC;
            *pu8DstPos++ = (*pu8Src++) ^ HDLC_ESC_MASK;
        }
        else
        {
            *pu8DstPos++ = *pu8Src++;
        }
    }

    /* 判断目的BUFFER是否够添加FCS和帧尾 */
    if ((pu8DstPos - pu8Dst) > (u16DestBufLen - 3))
    {
        *pu16DstLen = 0;
        return HDLC_ERR_BUF_FULL;
    }

    u16Fcs = ~u16Fcs;

    /* 转义并添加FCS第一个字节 */
    u8FcsChar = u16Fcs & 0xFF;
    if ((HDLC_FRAME_FLAG == u8FcsChar) || (HDLC_ESC == u8FcsChar))
    {
        *pu8DstPos++ = HDLC_ESC;
        *pu8DstPos++ = u8FcsChar ^ HDLC_ESC_MASK;
    }
    else
    {
        *pu8DstPos++ = u8FcsChar;
    }

    /* 判断目的BUFFER是否够添加FCS和帧尾 */
    if ((pu8DstPos - pu8Dst) > (u16DestBufLen - 2))
    {
        *pu16DstLen = 0;
        return HDLC_ERR_BUF_FULL;
    }

    /* 转义并添加FCS第二个字节 */
    u8FcsChar = (u16Fcs >> 8) & 0xFF;
    if ((HDLC_FRAME_FLAG == u8FcsChar) || (HDLC_ESC == u8FcsChar))
    {
        *pu8DstPos++ = HDLC_ESC;
        *pu8DstPos++ = u8FcsChar ^ HDLC_ESC_MASK;
    }
    else
    {
        *pu8DstPos++ = u8FcsChar;
    }

    /* 判断目的BUFFER是否够添加帧尾 */
    if ((pu8DstPos - pu8Dst) > (u16DestBufLen - 1))
    {
        *pu16DstLen = 0;
        return HDLC_ERR_BUF_FULL;
    }

    *pu8DstPos++   = HDLC_FRAME_FLAG;
    *pu16DstLen     = (u16)(pu8DstPos - pu8Dst);

    return HDLC_OK;
}

/*****************************************************************************
* 函 数 名  : HDLC_Decap_Test
*
* 功能描述  : 从输入的HDLC帧字符流中解析出数据内容
*
* 输入参数  : DLOAD_HDLC_S *pstHdlc  
*             UINT8 ucChar          
* 输出参数  : 
*
* 返 回 值  : 
*
* 其它说明  : 
*
*****************************************************************************/
eHdlcState HDLC_Decap_Test(DLOAD_HDLC_S *pstHdlc, unsigned char ucChar)
{
    unsigned short u16Fcs;

    if(!pstHdlc)
    {
        //BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_DLOAD,"HDLC_Decap, Pointer Para is NULL !\n",1,2,3,4,5,6);
        return HDLC_PARA_ERR;
    }

    /* 未找到帧头时，丢弃非HDLC_FRAME_FLAG字符 */
    if ((pstHdlc->u32Mode & HDLC_MODE_HUNT) && (HDLC_FRAME_FLAG != ucChar))
    {
        pstHdlc->u32Length   = 0;        /* 如果用户首次调用HDLC_Decap前进行了初始化HDLC_Init，则此处pstHdlc->u32Length可以不清0 */
        return HDLC_ERR_INVALAID_FRAME;
    }

    switch (ucChar)
    {
        case HDLC_FRAME_FLAG:
            pstHdlc->u32Mode &= ~HDLC_MODE_HUNT;          /* 清标记 */

            if (pstHdlc->u32Length > HDLC_FCS_LEN)
            {                           /* 该分支表示解析到(信息域+FCS)长度大于2 BYTE的帧 */
                u16Fcs = HDLC_Fcs_Test(pstHdlc->au8DecapBuf, pstHdlc->u32Length);
                if (HDLC_GOOD_FCS != u16Fcs)
                {
                    pstHdlc->u32Length       = 0;
                    return HDLC_ERR_FCS;
                }

                pstHdlc->u32InfoLen      = pstHdlc->u32Length - HDLC_FCS_LEN;
                pstHdlc->u32Length       = 0;
                return HDLC_OK;
            }
            else
            {                           /* 该分支若pstHdlc->u32Length为0，认为是正常情况：连续的HDLC_FRAME_FLAG */
                                        /* 该分支若pstHdlc->u32Length不为0，认为有HDLC帧，但(信息域+FCS)长度小于3，故丢弃 */
                pstHdlc->u32Length   = 0;
                break;
            }
            
        case HDLC_ESC:
            if (!(pstHdlc->u32Mode & HDLC_MODE_ESC))
            {
                pstHdlc->u32Mode |= HDLC_MODE_ESC;        /* 置标记 */
            }
            else
            {                           /* 异常情况: 连续两个HDLC_ESC */
                pstHdlc->u32Mode &= ~HDLC_MODE_ESC;       /* 清标记 */
                pstHdlc->u32Mode |= HDLC_MODE_HUNT;       /* 置标记 */
                pstHdlc->u32Length = 0;
                //BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_DLOAD,"HDLC_Decap, Esc Char Error:0x%x !\n\r", ucChar,2,3,4,5,6);
                return HDLC_ERR_DISCARD;
            }
            break;
            
        default:
            /* 判断目的BUFFER是否已满 */
            if (pstHdlc->u32Length >= pstHdlc->u32DecapBufSize)
            {                           /* 异常情况: 解封装BUFFER不足 */
                pstHdlc->u32Mode |= HDLC_MODE_HUNT;        /* 置标记 */
                pstHdlc->u32Length = 0;
                //BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_DLOAD,"WARNING, HDLC_Decap, Dst Buf is full #1:BufLen:%d !\n\r",(u32)pstHdlc->u32DecapBufSize,2,3,4,5,6);
                return HDLC_ERR_BUF_FULL;
            }

            if (pstHdlc->u32Mode & HDLC_MODE_ESC)
            {
                pstHdlc->u32Mode &= ~HDLC_MODE_ESC;          /* 清标记 */
                if (((HDLC_FRAME_FLAG^HDLC_ESC_MASK) == ucChar)
                    || ((HDLC_ESC^HDLC_ESC_MASK) == ucChar))
                {
                    ucChar ^= HDLC_ESC_MASK;
                }
                else
                {                           /* 异常情况: HDLC_ESC后面的字符不正确 */
                    pstHdlc->u32Mode |= HDLC_MODE_HUNT;        /* 置标记 */
                    pstHdlc->u32Length = 0;
                    //BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_DLOAD,"HDLC_Decap, Esc Char Error:0x%x !\n\r",ucChar,2,3,4,5,6);
                    return HDLC_ERR_DISCARD;
                }
            }

            pstHdlc->au8DecapBuf[pstHdlc->u32Length] = ucChar;
            pstHdlc->u32Length++;
            break;
    }


    return HDLC_ERR_INVALAID_FRAME;
}


/*****************************************************************************
* 函 数 名  : HDLC_Init_Test
*
* 功能描述  : HDLC结构体初始化
*
* 输入参数  : DLOAD_HDLC_S *pstHdlc  
* 输出参数  : DLOAD_HDLC_S *pstHdlc 
*
* 返 回 值  : NA
*
* 其它说明  : 每个应用在第一次使用解封装功能HDLC_Decap前需要调用一次该函数
*
*****************************************************************************/
void HDLC_Init_Test(DLOAD_HDLC_S *pstHdlc)
{
    if(NULL == pstHdlc)
    {
        return;
    }

    pstHdlc->u32DecapBufSize = HDLC_BUF_SIZE;

    pstHdlc->u32InfoLen  = 0;
    pstHdlc->u32Length   = 0;
    pstHdlc->u32Mode     = HDLC_MODE_HUNT;
}

unsigned int asmConnectFrame(unsigned char* pbuf)
{
	unsigned int frameLen = 0;
    //unsigned short pkt_len = PKT_LEN_CONNECT;

	pbuf[frameLen++] = PKT_TYPE_CONNECT;

	//pbuf[frameLen++] = (char)(pkt_len>>8);
	//pbuf[frameLen++] = (char)pkt_len;

	return frameLen;
}

unsigned int asmBaudSetFrame(unsigned char* pbuf, unsigned int ulBaudRate)
{
	unsigned int frameLen = 0;
    //unsigned short pkt_len = PKT_LEN_BAUD_SET;
    
	pbuf[frameLen++] = PKT_TYPE_BAUD_SET;

	//pbuf[frameLen++] = (char)(pkt_len>>8);
	//pbuf[frameLen++] = (char)pkt_len;

    pbuf[frameLen++] = (unsigned char)(ulBaudRate>>24);
    pbuf[frameLen++] = (unsigned char)(ulBaudRate>>16);
    pbuf[frameLen++] = (unsigned char)(ulBaudRate>>8);
    pbuf[frameLen++] = (unsigned char)(ulBaudRate);

	return frameLen;
}

unsigned int asmApCmdFrame(unsigned char* pbuf, unsigned char ucCmd)
{
	unsigned int frameLen = 0;
    //unsigned short pkt_len = PKT_LEN_AP_CMD;

	pbuf[frameLen++] = PKT_TYPE_AP_CMD;

	//pbuf[frameLen++] = (char)(pkt_len>>8);
	//pbuf[frameLen++] = (char)pkt_len;

	pbuf[frameLen++] = ucCmd;

	return frameLen;
}

unsigned int asmFileFrameDl(unsigned char* pbuf, unsigned int fileLen, unsigned int ulAddress)
{
    unsigned int frameLen = 0;
    //unsigned short pkt_len = PKT_LEN_START_DATA;

    pbuf[frameLen++] = PKT_TYPE_STAT_DATA;
    
    //pbuf[frameLen++] = (char)(pkt_len>>8);
	//pbuf[frameLen++] = (char)pkt_len;

    pbuf[frameLen++] = (unsigned char)(fileLen>>24);
    pbuf[frameLen++] = (unsigned char)(fileLen>>16);
    pbuf[frameLen++] = (unsigned char)(fileLen>>8);
    pbuf[frameLen++] = (unsigned char)(fileLen);

    pbuf[frameLen++] = (unsigned char)(ulAddress>>24);
    pbuf[frameLen++] = (unsigned char)(ulAddress>>16);
    pbuf[frameLen++] = (unsigned char)(ulAddress>>8);
    pbuf[frameLen++] = (unsigned char)ulAddress;

    return frameLen;
}

unsigned int asmDataFrameDl(unsigned char* pbuf, unsigned char ucSEQ, unsigned char *pDataBuf, unsigned int ulDataLen)
{
    unsigned int frameLen = 0;

    pbuf[frameLen++] = PKT_TYPE_MIDST_DATA;
    pbuf[frameLen++] = (unsigned char)(ulDataLen>>8);
	pbuf[frameLen++] = (unsigned char)ulDataLen;
    
    pbuf[frameLen++] = ucSEQ;

    memcpy(pbuf+frameLen, pDataBuf, ulDataLen);
    frameLen += ulDataLen;

    return frameLen;
}

unsigned int asmEndFrameDl(unsigned char* pbuf, unsigned char ucSEQ)
{
    unsigned int frameLen = 0;
    //unsigned short pkt_len = PKT_LEN_END_DATA;

    pbuf[frameLen++] = PKT_TYPE_END_DATA;

    //pbuf[frameLen++] = (char)(pkt_len>>8);
	//pbuf[frameLen++] = (char)pkt_len;

    return frameLen;
}

unsigned int asmExeFrameDl(unsigned char* pbuf, unsigned char ucSEQ)
{
    unsigned int frameLen = 0;
    //unsigned short pkt_len = PKT_LEN_EXEC_DATA;

    pbuf[frameLen++] = PKT_TYPE_EXEC_DATA;

    //pbuf[frameLen++] = (char)(pkt_len>>8);
	//pbuf[frameLen++] = (char)pkt_len;

    return frameLen;
}
unsigned char sendBuf[512];
unsigned char readBuf[512];

static int onchiprom_download(unsigned int changebd)
{
   	u32 pFile;
	u32 ret = 0;
    unsigned int fileLen;
    unsigned int ucSeq = 0;

    unsigned int sendLen;
    unsigned int readLen;
//  unsigned char recvBuf[10] = {0};
    RECV_STR pstQue = {0};

    HDLC_Init_Test(&stHdlcCtrl);
    
    /************************************************************************/
	/* 发送Connect命令                                                       */
	/************************************************************************/
	sendLen = asmConnectFrame(sendBuf);
	if (sendFrame(sendBuf, sendLen, &pstQue) != OK)
	{
		printk("asmConnectFrame failed\n");
		return ERROR;
	}

    /************************************************************************/
	/* 发送修改波特率命令                                                       */
	/************************************************************************/
	sendLen = asmBaudSetFrame(sendBuf, changebd);
	if (sendFrame(sendBuf, sendLen, &pstQue) != OK)
	{
		printk("asmBaudSetFrame failed\n");
		return ERROR;
	}

    /* 修改波特率 */
    if(hsUartSetBaud(changebd) != HSUART_STATUS_OK)
    {
    	printk("set baudrate error\n");
		return ERROR;
    }

    msleep(500);

    /************************************************************************/
	/* 发送操作指令                                                         */
	/************************************************************************/
	sendLen = asmApCmdFrame(sendBuf, AP_CMD_DL_BL_BURN_PRE);
	if (sendFrame(sendBuf, sendLen, &pstQue) != OK)
	{
		printk("asmApCmdFrame failed\n");
		return ERROR;
	}

    /* 开始下载BootLoader */
    ret = bsp_open(FILE_BOOT_LOADER, (s32)(RFILE_RDWR),(s32)0660);
	if(ret<0)
	{
        printk("can't open file %s\n", FILE_BOOT_LOADER);
        return ERROR;
    }
	pFile = (u32)ret;
    fileLen = getFileLen(pFile);

    /************************************************************************/
    /* 发送FILE帧                                                           */
    /************************************************************************/
    sendLen = asmFileFrameDl(sendBuf, fileLen, AXI_MEM_BL_ADDR);		
    if (sendFrame(sendBuf, sendLen, &pstQue) != OK)
    {
        printk("send failed, seq %d\n", ucSeq);
		bsp_close(pFile);
        return ERROR;
    }

	printk("send file frame succ\n");

    /************************************************************************/
    /* 循环发送DATA帧                                                       */
    /************************************************************************/
    while(fileLen)
    {
        readLen = (fileLen > READ_BUFFER_SIZE) ? READ_BUFFER_SIZE : fileLen;
        bsp_read(pFile,readBuf,readLen);

		fileLen -= readLen;

        sendLen = asmDataFrameDl(sendBuf, ucSeq, readBuf, readLen);
        if (sendFrame(sendBuf, sendLen, &pstQue) != OK)
        {
            printk("send failed, seq %d\n", ucSeq);
			bsp_close(pFile);
            return ERROR;
        }

        printk("send data frame succ %d \n",ucSeq);
		
        ucSeq++;
    }

    /************************************************************************/
    /* 发送END帧                                                            */
    /************************************************************************/
    sendLen = asmEndFrameDl(sendBuf, ucSeq);
    if (sendFrame(sendBuf, sendLen, &pstQue) != OK)
    {
        printk("send failed, seq %d\n", ucSeq);
		bsp_close(pFile);
        return ERROR;
    }
    printk("send end frame succ\n");

    /************************************************************************/
    /* 发送Execute帧                                                        */
    /************************************************************************/
    ucSeq = 0;
    sendLen = asmExeFrameDl(sendBuf, ucSeq);
    if (sendFrame(sendBuf, sendLen, &pstQue) != OK)
    {
        printk("send failed, seq %d\n", ucSeq);
		bsp_close(pFile);
        return ERROR;
    }
    printk("send Execute frame succ\n");
	bsp_close(pFile);
    return OK;
}
/*****************************************************************************
* 函 数 名  : get_info_from_dts
* 功能描述  : UART UDI 初始化接口
* 输入参数  : NA
* 输出参数  : NA
* 返 回 值  : 成功/失败
*****************************************************************************/
static int get_info_from_dts(void)
{
	struct device_node *node = NULL;
	unsigned int reg_data[2]={0,0};
	
	node = of_find_compatible_node(NULL, NULL, "hisilicon,onchiprom_uart");
	if (!node)
	{
		printk("dts node hsuart app not found!\n");
		return ERROR;
	}
	
	if(of_property_read_u32(node, "clock", &onchiprom_uart.clock))
	{
		printk("read reg from dts is failed!\n");
		return ERROR;
	}

	if(of_property_read_u32_array(node, "reg", (u32 *)&reg_data,2))
	{
		printk("read reg from dts is failed!\n");
		return ERROR;
	}
	onchiprom_uart.phy_addr_base = reg_data[0];
	onchiprom_uart.reg_size = reg_data[1];
	onchiprom_uart.irq = (u32)irq_of_parse_and_map(node, 0);
	if(onchiprom_uart.irq == 0)
	{
		printk("read hsuart irq fail\n");
		return ERROR;
	}
	onchiprom_uart.vir_addr_base = of_iomap(node, 0);
	if(NULL == onchiprom_uart.vir_addr_base )
	{
		printk("vir addr get fail\n");
		return ERROR;
	}
	
	return OK;
}


int onchiprom_uart_init(unsigned int BautRate, unsigned int changebd)
{
    int ret = 0;
	struct clk *hsuart_clk = NULL;

	if(OK != get_info_from_dts())
	{
		printk("hsuart get dts err\n");
		return ERROR;
	}
	
	hsuart_clk = clk_get(NULL, "uart1_clk");
	if(IS_ERR(hsuart_clk))
	{
		printk("get clk fail\n");
		return ERROR;
	}
	clk_prepare(hsuart_clk);
	clk_enable(hsuart_clk);
	
    hsUartInit(BautRate);  
    ret = onchiprom_download(changebd);

    return ret;
}


