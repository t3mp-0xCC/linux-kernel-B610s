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

#ifdef __cplusplus
    extern "C" {
#endif

#include <linux/kernel.h>
#include<linux/delay.h>
#include <linux/string.h>
#include <linux/mmc/sdio.h>
#include <linux/of.h>
#include "drv_comm.h"
#include "bsp_sysctrl.h"
#include "sdio_slave_errcode.h"
#include "sdio_slave_hal.h"
#include "sdio_slave.h"

extern const BYTE CIS0_Data[17];
extern const BYTE CIS1_Data[53];


static void syssc_sdio_slave_set(const struct device_node *np, const char *propname,unsigned int value)
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
	reg = readl(base);
	reg &= ~(((1<<bitnum)-1)<<para_value[1]);
	reg |= (value << para_value[1]);
	writel(reg, base);
}

static void sc_sdio_slave_msc_set(unsigned int val)
{
	struct device_node *dev_node     = NULL;

    dev_node = of_find_compatible_node(NULL, NULL, "hisilicon,slave_balong_app");
    if(NULL == dev_node)
    {
        printk("sdio slave node not found!\n");
		return ;
	}
	syssc_sdio_slave_set(dev_node,"mmc_msc",val);
    return ;
}

/*rdy send data to host,send interrupt to host*/
BSP_S32 hal_sdio_S2MRdySend(BSP_U32 Length)
{  
    /* set AHB_TRANS_CNT */
    BSP_REG_WRITE(g_u32SlaveBase,AHB_TRANS_CNT,Length);
    /*set FUN1_RD_DATA_RDY bit to 1*/
    BSP_REG_WRITE(g_u32SlaveBase,FUN1_RD_DATA_RDY,1);
    return BSP_OK;
}  

/*msg transfer slave->host*/
BSP_VOID hal_sdio_S2MMsgSend(BSP_U32 u32Msg)
{   
    /*first set Fun1_rd_data_rdy bit*/
    (BSP_VOID)hal_sdio_S2MRdySend(sizeof(u32Msg));
    BSP_REG_WRITE(g_u32SlaveBase,ARM_GENERAL_PURPOSE,u32Msg);
}

/*UHSģʽ֧��*/
BSP_VOID  SLAVE_UHS_Support(BSP_BOOL bUhs)
{
    BSP_U32 value = 0;

    BSP_REG_READ(g_u32SlaveBase,UHS_SUPPORT,value);

    if(bUhs)
    {
        value |= 0x1;
    }
    else
    {
        value &= ~0x1;
    }

    BSP_REG_WRITE(g_u32SlaveBase,UHS_SUPPORT,value);
}

/*����ģʽ֧��*/
BSP_VOID SLAVE_SHS_Support(BSP_BOOL bSpi)
{
    BSP_U32 value = 0;
 
    BSP_REG_READ(g_u32SlaveBase,ESW_CCCR_REG,value);
    if(bSpi)
    {
        value |= PROG_ESW_CCCR_SHS;
    }
    else
    {
        value &= ~PROG_ESW_CCCR_SHS;
    }

    BSP_REG_WRITE(g_u32SlaveBase,ESW_CCCR_REG,value);
}

/*io access mode set*/
BSP_VOID SLAVE_IO_Mode(BSP_U32 value)
{
    BSP_U32 u32Reg;
    /*set IO access support register*/
    BSP_REG_READ(g_u32SlaveBase,IO_ACCESS_MODE,u32Reg);
    u32Reg |= value;
    BSP_REG_WRITE(g_u32SlaveBase,IO_ACCESS_MODE,u32Reg);

}

/*SD2.0 or SD3.0ģʽ����*/
BSP_VOID SLAVE_Version_Set(BSP_BOOL bflag)
{
    /*set SD2.0 or SD3.0ģʽ 1:SD3.0, 0:SD2.0*/
    if(bflag)
    {
        BSP_REG_WRITE(g_u32SlaveBase,ESW_CCCR_REG,PROG_REG_ESW_CCCR_30);
    }
    else
    {
        BSP_REG_WRITE(g_u32SlaveBase,ESW_CCCR_REG,PROG_REG_ESW_CCCR_20);
    }

}
/*voltage switch sequence*/
BSP_VOID SLAVE_VolSwt()
{   
    BSP_U32 u32reg;
    sc_sdio_slave_msc_set(1);
    /*������������5MS����ɵ�ѹ�л�����*/
    BSP_REG_READ(g_u32SlaveBase,UHS_SUPPORT,u32reg);
    u32reg |= 0x4; //set Card accept volt bit 
    BSP_REG_WRITE(g_u32SlaveBase,UHS_SUPPORT,u32reg); 
    u32reg |= 0x8; //set sd_clk_line_switched bit 
    BSP_REG_WRITE(g_u32SlaveBase,UHS_SUPPORT,u32reg); 
    u32reg |= 0x10; //set sd_cmd_line switch bit
    BSP_REG_WRITE(g_u32SlaveBase,UHS_SUPPORT,u32reg);
   
}

BSP_VOID SLAVE_Rst()
{
    /*���ø�λ�ܽ�*/
    BSP_REG_WRITE(g_u32SlaveBase,SOFT_RESET_VALID,0x1);
}

BSP_VOID SLAVE_Sps_Support(BSP_BOOL bSpi)
{
    BSP_U32 value = 0;

    BSP_REG_READ(g_u32SlaveBase,ESW_FBR_REG,value);
    if(bSpi)
    {
        value |= PROG_FBR_SPS_SUP;
    }
    else
    {
        value &= ~PROG_FBR_SPS_SUP;
    }

    BSP_REG_WRITE(g_u32SlaveBase,ESW_FBR_REG,value);
}

BSP_VOID SLAVE_CSA_Support(BSP_BOOL bSup)
{
    BSP_U32 u32RegValue = 0;
    BSP_REG_READ(g_u32SlaveBase,ESW_FBR_REG,u32RegValue);
    if(bSup)
    {
        u32RegValue |= 0x10;
    }
    else
    {
        u32RegValue &= ~0x10;
    }
    BSP_REG_WRITE(g_u32SlaveBase,ESW_FBR_REG,u32RegValue);
}
 

/* set function enable or disable */
BSP_VOID SLAVE_fun1_switch(BSP_BOOL bMode)
{ 
    /* Enable the function, enable and clear interrupts */
	if (bMode)	
	{
        /* ����FUN1 IOR RDY bit */
        BSP_REG_WRITE(g_u32SlaveBase,ESW_IOR_REG,0x01);       
        BSP_REG_WRITE(g_u32SlaveBase,AHB_FUN1_INT_ENABLE,FUN1_INT_BITS_ALL);
        BSP_REG_WRITE(g_u32SlaveBase,AHB_FUN1_INT_STATUS,GLOBAL_INT_BITS_ALL);
	}
	else
	{   /* ��� FUN1 IOR RDY bit */
		BSP_REG_WRITE(g_u32SlaveBase,ESW_IOR_REG,0x0);  
        BSP_REG_WRITE(g_u32SlaveBase,AHB_FUN1_INT_ENABLE,0x0);
	}
}

BSP_VOID SLAVE_int_init(BSP_VOID)
{    
    /* ����жϱ�־ */
    SLAVE_IntClr_Global(GLOBAL_INT_BITS_ALL);
    SLAVE_IntClr_Fun0(FUN0_INT_BITS_ALL);
    SLAVE_IntClr_Fun1(FUN1_INT_BITS_ALL);
   
    /* ʹ���ж� */
    SLAVE_IntMask_Global(GLOBAL_INT_BITS_ALL);
    SLAVE_IntMask_Fun0(FUN0_INT_BITS_ALL);
    SLAVE_IntMask_Fun1(FUN1_INT_BITS_ALL);
}

/*����ȫ���ж�*/
BSP_VOID SLAVE_int_disable()
{
    SLAVE_IntMask_Global(0);
    SLAVE_IntMask_Fun0(0);
    SLAVE_IntMask_Fun1(0);
    SLAVE_IntMask_Mem(0);
}


/* IO�豸�ϵ�ö�ٹ��� ��ʼ������*/
BSP_S32 SLAVE_IOEnumCfg(BSP_VOID)
{
    BSP_U32 i = 0;

	/* Check if the core is ready for programming */
	
	//sdio_set_init_ios(1);
#ifndef SLAVE_DEBUG
	for(i=0;i < CARD_RDY_TIMEOUT;i++)
#else
	while(1)
#endif
	{
	    if(SLAVE_GetReady() & PROG_REG_CARD_RDY)
	    {
			break;
		}
            msleep(10);
	}
	
	if(i >= CARD_RDY_TIMEOUT)
	{
	    //BSP_TRACE(BSP_LOG_LEVEL_ERROR, BSP_MODU_SLAVE, "card not ready!\n");
	    printk("card not ready!\n");
	}

    /* config ESW_CCCR register */
    SLAVE_Version_Set(TRUE); /*Ĭ�����ó�3.0ģʽ*/
    SLAVE_UHS_Support(BSP_TRUE);
    /* config FUN1 IO OCR register */
    BSP_REG_WRITE(g_u32SlaveBase,ESW_OCR_REG,OCR_REG_VOLTAGE_RANGE);

    /* config ESW FBR1 Register */
    BSP_REG_WRITE(g_u32SlaveBase,ESW_FBR_REG,PROG_REG_ESW_FBR);

    /* program FUN1 IOR1 register,set function 1 as ready status */ 
    SLAVE_Fun1_Ready();

    /* ���ó�ʱֵ,cmd11��ѹ�л��ȴ�ʱ�� */
    BSP_REG_WRITE(g_u32SlaveBase,CLK_DELAY_TIMER,TIME_OUT_VALUE);//timeout 
        
    /*set IO access support register*/
    SLAVE_IO_Mode(IO_SPEED_MODE);
    
    //BSP_TRACE(g_u32SlaveBase, BSP_MODU_SLAVE, "device enum succuss!\n");
    
    return SLAVE_OK;
}

/* MEM�豸�ϵ�ö�ٹ��� �ݲ�ʵ�� */
BSP_S32 SLAVE_MEM_EnumCfg(BSP_VOID)
{
    return BSP_OK;
}

/* IO&MEM�豸�ϵ�ö�ٹ��� �ݲ�ʵ�� */
BSP_S32 SLAVE_IOMEM_EnumCfg(BSP_VOID)
{
    return BSP_OK;
}

/*��ӡ�Ĵ�����Ϣ*/
BSP_VOID SLAVE_RegPrint()
{
    printk(" ======================= Register Value ==========================\n");
    
    printk("\t CCCR register:           0x%x = 0x%x\n", (int)(g_u32SlaveBase + ESW_CCCR_REG),
        BSP_REG(g_u32SlaveBase, ESW_CCCR_REG));
    printk("\t AHB Trans_cnt register:  0x%x = 0x%x\n", (int)(g_u32SlaveBase + AHB_TRANS_CNT),
        BSP_REG(g_u32SlaveBase, AHB_TRANS_CNT));
    printk("\t SDIO TRANS_CNT register: 0x%x = 0x%x\n", (int)(g_u32SlaveBase + SDIO_TRANS_CNT),
        BSP_REG(g_u32SlaveBase, SDIO_TRANS_CNT));
    printk("\t ESW_FBR_REG register:    0x%x = 0x%x\n", (int)(g_u32SlaveBase + ESW_FBR_REG),
        BSP_REG(g_u32SlaveBase, ESW_FBR_REG));
    printk("\t BLOCK_SIZE register:     0x%x = 0x%x\n", (int)(g_u32SlaveBase + BLOCK_SIZE_REG),
        BSP_REG(g_u32SlaveBase, BLOCK_SIZE_REG));
    printk("\t ARGUMENT register:       0x%x = 0x%x\n", (int)(g_u32SlaveBase + ARGUMENT_REG),
        BSP_REG(g_u32SlaveBase, ARGUMENT_REG));
    printk("\t SYSTEM ADDRESS register: 0x%x = 0x%x\n", (int)(g_u32SlaveBase + ADMA_SYS_ADDR),
        BSP_REG(g_u32SlaveBase, ARGUMENT_REG));
    printk("\t HOST_GENERAL_PURPOSE:    0x%x = 0x%x\n", (int)(g_u32SlaveBase + HOST_GENERAL_PURPOSE),
        BSP_REG(g_u32SlaveBase, HOST_GENERAL_PURPOSE));
    printk("\t IO_ACCESS_MODE:          0x%x = 0x%x\n", (int)(g_u32SlaveBase + IO_ACCESS_MODE),
        BSP_REG(g_u32SlaveBase, IO_ACCESS_MODE));
    
    printk(" =================================================================\n");
} 

void slave_reinit()
{
	BSP_U32 Cis0Len = sizeof(CIS0_Data)/sizeof(CIS0_Data[0]);
    BSP_U32 Cis1Len = sizeof(CIS1_Data)/sizeof(CIS1_Data[0]);

	SLAVE_Version_Set(TRUE); /*Ĭ�����ó�3.0ģʽ*/

	SLAVE_UHS_Support(BSP_TRUE);


	/* config FUN1 IO OCR register */
	BSP_REG_WRITE(g_u32SlaveBase,ESW_OCR_REG,OCR_REG_VOLTAGE_RANGE);

	/* config ESW FBR1 Register */
	BSP_REG_WRITE(g_u32SlaveBase,ESW_FBR_REG,PROG_REG_ESW_FBR);

	memcpy(g_u32SlaveBase + 0x80, CIS0_Data, Cis0Len);
    memcpy(g_u32SlaveBase + 0xa0, CIS1_Data, Cis1Len);

	/* program FUN1 IOR1 register,set function 1 as ready status */ 
	SLAVE_Fun1_Ready();

	/* ���ó�ʱֵ,cmd11��ѹ�л��ȴ�ʱ�� */
	BSP_REG_WRITE(g_u32SlaveBase,CLK_DELAY_TIMER,TIME_OUT_VALUE);//timeout 
	    
	/*set IO access support register*/
	SLAVE_IO_Mode(IO_SPEED_MODE);
	SLAVE_Fun_Ready();
	/* ʹ���ж� */
	SLAVE_IntMask_Global(GLOBAL_INT_BITS_ALL);
	SLAVE_IntMask_Fun0(FUN0_INT_BITS_ALL);
	SLAVE_IntMask_Fun1(FUN1_INT_BITS_ALL);
	
}

#ifdef __cplusplus
}
#endif
