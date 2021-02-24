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
/* Warning 537: (Warning -- Repeated include  */
/*lint *****--e{537,713,732,701,438,830,958,737}*/


#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/syscore_ops.h>
#include <linux/clk.h>
#include <hi_base.h>
#include <osl_types.h>


#include "osl_bio.h"
#include "osl_irq.h"
#include "osl_thread.h"
#include "osl_module.h"
#include "bsp_version.h"
#include "bsp_edma.h"
#include "bsp_ipc.h"
#include "bsp_vic.h"
#include <bsp_om.h>
#include <bsp_sysctrl.h>

/*�������󣬸���edma оƬ����*/
enum edma_peri_cfg
{
    edma_peri_spi0_rx = 0,
    edma_peri_spi0_tx,
    edma_peri_spi1_rx,
    edma_peri_spi1_tx,
    edma_peri_ltesio_rx,
    edma_peri_ltesio_tx,
    edma_peri_reserve_0,
    edma_peri_reserve_1,
    edma_peri_hsuart_rx = 8,
    edma_peri_hsuart_tx,
    edma_peri_uart0_rx,
    edma_peri_uart0_tx,
    edma_peri_uart1_rx,
    edma_peri_uart1_tx,
    edma_peri_uart2_rx,
    edma_peri_uart2_tx,
    edma_peri_sci0_rx = 0x10,
    edma_peri_sci_rx  = 0x10,
    edma_peri_sci0_tx,
    edma_peri_sci1_rx,
    edma_peri_sci1_tx,
    edma_peri_gbbp0_dbg = 0x14,
    edma_peri_bbp_dbg   = 0x14,
    edma_peri_gbbp1_grif = 0x15,
    edma_peri_bbp_grif   = 0x15,
    edma_peri_amon_soc = 0x16,
    edma_peri_amon_cpufast = 0x17,
    edma_peri_i2c0_rx,              /*711����v7r2û��*/
    edma_peri_i2c0_tx,              /*711����v7r2û��*/
    edma_peri_i2c1_rx,              /*711����v7r2û��*/
    edma_peri_i2c1_tx,              /*711����v7r2û��*/
    edma_peri_emi_trans,            /*711����v7r2û��,edma_ch16��֧��,edma_ch4��֧��*/
    edma_peri_req_max,
};

#define OSDRV_MODULE_VERSION_STRING  "ANDROID_4.2-EDMAC @HiV7R2 SFT ACPU"
#define DRIVER_NAME  "edmac_device"

#define edma_cache_sync()    cache_sync()

struct edma_id_int_isr
{
    struct chan_int_service edma_trans1_isr[ EDMA_CH16_NUM ];
    struct chan_int_service edma_trans2_isr[ EDMA_CH16_NUM ];
    struct chan_int_service edma_err1_isr[ EDMA_CH16_NUM ];
    struct chan_int_service edma_err2_isr[ EDMA_CH16_NUM ];
    struct chan_int_service edma_err3_isr[ EDMA_CH16_NUM ];
};

struct edma_drv_info
{
    void *edma_base_addr[EDMA_NUMBER];
    /*  the edmac register pointer    static */
    struct edma_reg_struct * edma_reg_str[EDMA_NUMBER];/*2 edma :16 channel & 4 channel */
    /*edma Call back Function*/
    struct edma_id_int_isr  edma_int_isr[EDMA_NUMBER];
    u32 irq_no[EDMA_NUMBER];
    struct clk *clk[EDMA_NUMBER];
};

struct edma_drv_info g_edma_drv_info;

/* edmaͨ�����䷽ʽ */
u32 g_edma_chan_distri[EDMA_BUTT] = {0};

void edma_set_trace_level(u32 level);
u32 edma_id_valid(int edma_id);
void edma_init (void);
s32 edma_channel_int_enable(u32 channel_id);
s32 edma_channel_int_disable(u32 channel_id);
s32 bsp_edma_remove(struct platform_device *pdev);
void edma_isr_install(u32 channel_id, channel_isr pFunc, u32 channel_arg, u32 int_flag);

void edma_set_trace_level(u32 level)
{
    (void)bsp_mod_level_set(BSP_MODU_EDMA, level);
    hiedmac_trace(BSP_LOG_LEVEL_ERROR,"bsp_mod_level_set(BSP_MODU_EDMA=%d, %d)\n",BSP_MODU_EDMA,level);
}

/* inline  ���� edma  ����Ƿ��Ѿ���ʼ����
    ���Ѿ���ʼ�������� 1 �����򷵻� 0 */
 u32 edma_id_valid(int edma_id)
{
    if(( EDMA_CH4_ID == edma_id)&&g_edma_drv_info.edma_reg_str[edma_id])
    {return true;}
    else
    {
        return false;
    }
}

/*******************************************************************************
  ������:     static void edma_channel_isr_init(void)
  ��������:   ��ʼ������ͨ�����жϻص�����
  �������:   ��
  �������:   ��
  ����ֵ:     ��
  ȫ�ֱ�����ʼ��Ϊ�� �ú������ܲ���Ҫ������
*******************************************************************************/
/*******************************************************************************
  ������:      static void edma_isr_install(u32 channel_id,
                       channel_isr pFunc, u32 channel_arg, u32 int_flag)
  ��������:    ����int_flag��ע��ͨ����Ӧ���жϻص�����
  �������:    channel_id : ͨ��ID������ bsp_edma_channel_init �����ķ���ֵ
               pFunc : �����ߴ�����ͨ���жϻص�����
               channel_arg : pFunc�����1
               int_flag : pFunc�����2, �ж����ͣ������
#define EDMA_INT_DONE           1          EDMA��������ж�
#define EDMA_INT_LLT_DONE       2          ��ʽEDMA�ڵ㴫������ж�
#define EDMA_INT_CONFIG_ERR     4          EDMA���ô����µ��ж�
#define EDMA_INT_TRANSFER_ERR   8          EDMA��������µ��ж�
#define EDMA_INT_READ_ERR       16         EDMA����������µ��ж�
  �������:    ��
  ����ֵ:      ��
*******************************************************************************/

/*����Ҫ��Ϊ bsp_edma_isr_install */
void edma_isr_install(u32 channel_id, channel_isr pFunc, u32 channel_arg, u32 int_flag)
{
    u32 int_judge = 0;
    u32 chan_index = 0;

    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    /*EDMAC_REG_STRU * edma_reg_str = 0;*****/
    struct edma_id_int_isr *edma_int_isr = 0;

    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_int_isr = &(g_edma_drv_info.edma_int_isr[edma_id]);

    if (channel_id >= EDMA_CHANNEL_END)
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR,"EDMA_CHANNEL_INVALID\n");
        return;
    }
    else
    {
        if ((NULL == pFunc)||(0==int_flag))
        {
            hiedmac_trace(BSP_LOG_LEVEL_WARNING,"edma_isr is NULL or int_flag is 0!\n");
            return;
        }
        else
        {
            /*0x1F,��5λȫ1����Ӧ5���ж�����*/
            int_judge = EDMA_INT_ALL & int_flag;
            chan_index = (u32)edma_id_chan_id;
            if (int_judge & EDMA_INT_DONE)
            {
                edma_int_isr->edma_trans1_isr[chan_index].chan_isr = (channel_isr)pFunc;
                edma_int_isr->edma_trans1_isr[chan_index].chan_arg = channel_arg;
                edma_int_isr->edma_trans1_isr[chan_index].int_status = int_flag;
            }
            else{}
			if (int_judge & EDMA_INT_LLT_DONE)
            {
                edma_int_isr->edma_trans2_isr[chan_index].chan_isr = (channel_isr)pFunc;
                edma_int_isr->edma_trans2_isr[chan_index].chan_arg = channel_arg;
                edma_int_isr->edma_trans2_isr[chan_index].int_status = int_flag;
            }
            else{}
			if (int_judge & EDMA_INT_CONFIG_ERR)
            {
                edma_int_isr->edma_err1_isr[chan_index].chan_isr = (channel_isr)pFunc;
                edma_int_isr->edma_err1_isr[chan_index].chan_arg = channel_arg;
                edma_int_isr->edma_err1_isr[chan_index].int_status = int_flag;
            }
            else{}
			if (int_judge & EDMA_INT_TRANSFER_ERR)
            {
                edma_int_isr->edma_err2_isr[chan_index].chan_isr = (channel_isr)pFunc;
                edma_int_isr->edma_err2_isr[chan_index].chan_arg = channel_arg;
                edma_int_isr->edma_err2_isr[chan_index].int_status = int_flag;
            }
             else{}
			 if (int_judge & (EDMA_INT_READ_ERR))
            {
                edma_int_isr->edma_err3_isr[chan_index].chan_isr = (channel_isr)pFunc;
                edma_int_isr->edma_err3_isr[chan_index].chan_arg = channel_arg;
                edma_int_isr->edma_err3_isr[chan_index].int_status = int_flag;
            }
			else{}
            hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"edma_isr_install!\n");
        }
    }

}

/*******************************************************************************
  ������:      static void edma_ip_init(void)
  ��������:    ��ʼ�����жϼĴ������ж����μĴ���
  �������:    ��
  �������:    ��
  ����ֵ:      ��
*******************************************************************************/
static void edma_ip_init(void)
{
/*����ͨ����ʼ��*/

    /* ���� 2013.04.11 edmaͨ������� */
    /* ����ͨ�����ѷ��䣬ͨ��7��ldsp  */
    g_edma_chan_distri[EDMA_SPI0_TX]      = (u32)EDMA_CH_DRV_LCD;        /* Acore drv lcd ˢ��       ʵ�ʷ���ͨ�� ch16 - 14 */
    g_edma_chan_distri[EDMA_SPI1_TX]      = (u32)EDMA_CH_DRV_LCD;        /* Acore drv lcd ˢ��       ʵ�ʷ���ͨ�� ch16 - 14 */
    g_edma_chan_distri[EDMA_LTESIO_RX]    = (u32)EDMA_CH_HIFI_SIO_RX;                /* HIFI                     ʵ�ʷ���ͨ�� ch16 - 8  */
    g_edma_chan_distri[EDMA_LTESIO_TX]    = (u32)EDMA_CH_HIFI_SIO_TX;                /* HIFI                     ʵ�ʷ���ͨ�� ch16 - 9  */
    g_edma_chan_distri[EDMA_UART1_RX]     = (u32)EDMA_CH_HSUART_RX;      /* Acore drv ������������   ʵ�ʷ���ͨ�� ch16 - 12 */
    g_edma_chan_distri[EDMA_UART1_TX]     = (u32)EDMA_CH_HSUART_TX;      /* Acore drv ������������   ʵ�ʷ���ͨ�� ch16 - 13 */
    g_edma_chan_distri[EDMA_SCI0_RX]      = (u32)EDMA_CH_DRV_SIM_0;      /* Acore drv SIM��1��       ʵ�ʷ���ͨ�� ch16 - 10 */
    g_edma_chan_distri[EDMA_SCI0_TX]      = (u32)EDMA_CH_DRV_SIM_0;      /* Acore drv SIM��1��       ʵ�ʷ���ͨ�� ch16 - 10 */
    g_edma_chan_distri[EDMA_SCI1_RX]      = (u32)EDMA_CH_DRV_SIM_1;      /* Acore drv SIM��2��       ʵ�ʷ���ͨ�� ch16 - 11 */
    g_edma_chan_distri[EDMA_SCI1_TX]      = (u32)EDMA_CH_DRV_SIM_1;      /* Acore drv SIM��2��       ʵ�ʷ���ͨ�� ch16 - 11 */
    g_edma_chan_distri[EDMA_AMON_SOC]     = (u32)EDMA_CH_DRV_AXIMON;     /* Acore drv AXIMON         ʵ�ʷ���ͨ�� ch16 - 15 */
    g_edma_chan_distri[EDMA_AMON_CPUFAST] = (u32)EDMA_CH_DRV_AXIMON;     /* Acore drv AXIMON         ʵ�ʷ���ͨ�� ch16 - 15 */
    g_edma_chan_distri[EDMA_EMI_TRANS]    = (u32)EDMA_CH_DRV_LCD; 

    g_edma_chan_distri[EDMA_PCIE0_M2M_RX] = (u32)EDMA_CH_PCIE0_M2M_RX;
    g_edma_chan_distri[EDMA_PCIE0_M2M_TX] = (u32)EDMA_CH_PCIE0_M2M_TX;
    g_edma_chan_distri[EDMA_PCIE1_M2M_RX] = (u32)EDMA_CH_PCIE1_M2M_RX;
    g_edma_chan_distri[EDMA_PCIE1_M2M_TX] = (u32)EDMA_CH_PCIE1_M2M_TX;
    g_edma_chan_distri[EDMA_BUS_ANALYSE_1]= (u32)EDMA_CH_BUS_ANALYSE_1;
    g_edma_chan_distri[EDMA_BUS_ANALYSE_2]= (u32)EDMA_CH_BUS_ANALYSE_2;

}

void edma_init (void)
 {
    /*edma_channel_isr_init();*/
    edma_ip_init();
 }

/*******************************************************************************
function     :static void edma_channel_int_enable(u32 channel_id)

description  :Clear and enable int for [channel_id]. Used when a channe_id is
              applied and channel_isr is given.

input        :channel_id
output       :none
return       :none
*******************************************************************************/
s32 edma_channel_int_enable(u32 channel_id)
{
    unsigned long lock_key = 0;

    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;
    u32 chan_int_mask = 0;

    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return EDMA_FAIL;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];

    chan_int_mask = (u32)0x1<<edma_id_chan_id;
    /* Clear interrupt :write 1 clear; write 0 no change */
    edma_reg_str->ulIntTC1Raw  = chan_int_mask;
    edma_reg_str->ulIntTC2Raw  = chan_int_mask;
    edma_reg_str->ulIntERR1Raw = chan_int_mask;
    edma_reg_str->ulIntERR2Raw = chan_int_mask;
	edma_reg_str->ulIntERR3Raw = chan_int_mask;

    /* Enable interrupt: 0-mask, 1-do not mask */
    local_irq_save(lock_key);
    /*DONOT need multicore mutual exclusion*/

    edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntTC1Mask  |= chan_int_mask;
    edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntTC2Mask  |= chan_int_mask;
    edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntErr1Mask |= chan_int_mask;
    edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntErr2Mask |= chan_int_mask;
    edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntErr3Mask |= chan_int_mask;

    local_irq_restore(lock_key);
    return EDMA_SUCCESS;
}

/*******************************************************************************
function     :static void edma_int_disable(u32 channel_id)

description  :Clear and enable int for [channel_id]. Used when a channe_id is
              applied and channel_isr is given.

input        :channel_id
output       :none
return       :none
*******************************************************************************/
s32 edma_channel_int_disable(u32 channel_id)
{
    unsigned long  lock_key = 0;

    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;
    u32 chan_int_mask = 0;
    edma_id = EDMA_ID(channel_id);
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return EDMA_FAIL;
    }

    chan_int_mask = (u32)0x1<<edma_id_chan_id;
    /* Clear interrupt :write 1 clear; write 0 no change */
    edma_reg_str->ulIntTC1Raw  = chan_int_mask;
    edma_reg_str->ulIntTC2Raw  = chan_int_mask;
    edma_reg_str->ulIntERR1Raw = chan_int_mask;
    edma_reg_str->ulIntERR2Raw = chan_int_mask;
	edma_reg_str->ulIntERR3Raw = chan_int_mask;

    chan_int_mask = ~chan_int_mask;
    /* Enable interrupt: 0-mask, 1-do not mask */
    local_irq_save(lock_key);
    /*DONOT need multicore mutual exclusion*/
    edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntTC1Mask  &= chan_int_mask;
    edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntTC2Mask  &= chan_int_mask;
    edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntErr1Mask &= chan_int_mask;
    edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntErr2Mask &= chan_int_mask;
    edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntErr3Mask &= chan_int_mask;
    local_irq_restore(lock_key);
    return EDMA_SUCCESS;
}


/*******************************************************************************
  ������:      s32 bsp_edma_channel_init (enum edma_req_id request,
                          channel_isr channel_isr, u32 channel_arg,
                          u32 int_flag)
  ��������:    ��������ŷ���ͨ����ע��ͨ���жϻص���������ʼ����������ź�����
               �������д��config�Ĵ���
  �������:    req : ���������
               channel_isr : �ϲ�ģ��ע���EDMAͨ���жϴ�������NULLʱ������ע��
               channel_arg : channel_isr�����1��
                             channel_isrΪNULL������Ҫ�����������
               int_flag : channel_isr�����2, �������ж����ͣ�ȡֵ��ΧΪ
                        EDMA_INT_DONE��EDMA_INT_LLT_DONE��
                        EDMA_INT_CONFIG_ERR��EDMA_INT_TRANSFER_ERR��
                        EDMA_INT_READ_ERR֮һ��������ϡ�
                        channel_isrΪNULL������Ҫ�����������
  �������:    ��
  ����ֵ:      �ɹ���ͨ����
               ʧ�ܣ�����
*******************************************************************************/
s32 bsp_edma_channel_init (enum edma_req_id request, channel_isr pFunc, u32 channel_arg, u32 int_flag)
{
    s32 channel_id = -1;
    u32 edma_id = 0;
    u32 edma_id_chan_id = 0;

    struct edma_reg_struct * edma_reg_str = 0;

    if((u32)request>=EDMA_BUTT)
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR,"request= 0x%x is INVALID! \n",request);
        return channel_id;
    }
    else
    {
        channel_id = (s32)g_edma_chan_distri[request];
    }
    edma_id = EDMA_ID(channel_id);

    if ((channel_id >= 0)&&(channel_id<EDMA_CHANNEL_END)&&edma_id_valid(edma_id))
    {
        edma_id_chan_id = (u32)EDMA_ID_CHAN_ID(channel_id);
        edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];
        edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info.config = EDMAC_TRANSFER_CONFIG_REQUEST(request);

        if(pFunc)
        {
            edma_isr_install((u32)channel_id, pFunc, channel_arg, int_flag);
            (void)edma_channel_int_enable( channel_id );/*lint !e732*/
        }
        else
        {}
        return channel_id;
    }
    else
    {
         hiedmac_trace(BSP_LOG_LEVEL_ERROR," request= 0x%x FAILED, no chan to give! \n",request);
         return EDMA_FAIL;
    }
}

/*******************************************************************************
  ������:       s32 bsp_edma_current_transfer_address(u32 channel_id)
  ��������:     ���ĳͨ����ǰ������ڴ��ַ
  �������:     channel_id : ͨ��ID������ bsp_edma_channel_init�����ķ���ֵ
  �������:     ��
  ����ֵ:       �ɹ���ͨ����ǰ������ڴ��ַ
                ʧ�ܣ�-1
*******************************************************************************/
s32 bsp_edma_current_transfer_address(u32 channel_id)
{
    u32 get_addr_type = 0;
    s32 ret_addr = -1;
    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;
    edma_id = EDMA_ID(channel_id);
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return EDMA_FAIL;
    }

    /*��ѯconfig�Ĵ����ĸ���λ��Դ��Ŀ�ĵ�ַΪ�ڴ�ʱΪ1*/
    if (channel_id < EDMA_CHANNEL_END)
    {
        get_addr_type = EDMAC_TRANSFER_CONFIG_BOTH_INC\
                        & edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info.config ;
        switch (get_addr_type)
        {
            case (EDMAC_TRANSFER_CONFIG_DEST_INC|EDMAC_TRANSFER_CONFIG_SOUR_INC):
            case EDMAC_TRANSFER_CONFIG_SOUR_INC:
                ret_addr = (s32)(edma_reg_str->stCurrStatusReg[edma_id_chan_id].ulCxCurrSrcAddr);
                break;
            case EDMAC_TRANSFER_CONFIG_DEST_INC:
                ret_addr = (s32)(edma_reg_str->stCurrStatusReg[edma_id_chan_id].ulCxCurrDesAddr);
                break;
            default:
                break;
        }
    }
    return ret_addr;
}


/*******************************************************************************
  ������:       u64 bsp_edma_current_cnt(u32 channel_id)
  ��������:     ���ĳͨ������ʣ�� size
                (��ά�������2G ������֧��3ά���ɲ�ʹ�� u64  )
  �������:     channel_id : ͨ��ID������ bsp_edma_channel_init�����ķ���ֵ
  �������:     ��
  ����ֵ:       �ɹ������ĳͨ������ʣ�� size
                ʧ�ܣ�-1
*******************************************************************************/
u64 bsp_edma_current_cnt(u32 channel_id)
{
    u64 size = 0;/*lint !e958*/
    u32 chan_cnt0, chan_cnt1;
    u32 a_count,b_count,c_count;

    u32 chan_curr_cnt0, chan_curr_cnt1;
    u32 curr_a_count, curr_b_count;
    u32 curr_c_count;

    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;
    volatile struct edma_cb * lli_node_tmp = 0;

    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return (u64)EDMA_FAIL;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];

    lli_node_tmp = &(edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info);

    if (channel_id >= EDMA_CHANNEL_END)
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR," ----channel ID= %d, WRONG \n",  channel_id );
        return (u64)EDMA_FAIL;
    }

    /* read the config cnt */
    chan_cnt0 = lli_node_tmp->cnt0;
    chan_cnt1 = lli_node_tmp->cnt1;
    a_count = (chan_cnt0&0xFFFF);
    b_count = (chan_cnt0&0xFFFF0000)>>16;
    c_count = (chan_cnt1&0xFFFF);
    hiedmac_trace(BSP_LOG_LEVEL_DEBUG, \
                  "a_count:0x%x, b_count:0x%x, c_count:0x%x \n",\
                   a_count, b_count, c_count);

    /* read curr cnt */
    chan_curr_cnt0 = (u32)edma_reg_str->stCurrStatusReg[edma_id_chan_id].ulCxCurrCnt0;
    chan_curr_cnt1 = (u32)edma_reg_str->stCurrStatusReg[edma_id_chan_id].ulCxCurrCnt1;
    curr_a_count = (chan_curr_cnt0&0xFFFF);
    curr_b_count = (chan_curr_cnt0&0xFFFF0000)>>16;
    curr_c_count = (chan_curr_cnt1&0xFFFF);
    hiedmac_trace(BSP_LOG_LEVEL_DEBUG, \
                  "curr_a_count:0x%x, curr_b_count:0x%x, curr_c_count:0x%x \n",\
                   curr_a_count, curr_b_count, curr_c_count);

    /*compute the size left*/
    hiedmac_trace(BSP_LOG_LEVEL_DEBUG, \
                  "curr_count_1vec:0x%x, curr_count_2vec:0x%x, curr_count_3vec:0x%llx \n",\
                   curr_a_count, (a_count*curr_b_count), (a_count*b_count*curr_c_count));

    size = (u64)(curr_a_count) /* (long): Warning 647: (Warning -- Suspicious truncation)*/
          +(u64)(a_count*curr_b_count)
          +(a_count*b_count*curr_c_count); /*lint !e647*/
    hiedmac_trace(BSP_LOG_LEVEL_DEBUG, \
                  " ------------ size:0x%llx, \n",size);
    return size;
}

/*******************************************************************************
  ������:       s32 bsp_edma_channel_stop(u32 channel_id)
  ��������:     ָֹͣ����EDMAͨ��
  �������:     channel_id : ͨ��ID������ bsp_edma_channel_init�����ķ���ֵ
  �������:     ��
  ����ֵ:       �ɹ���ͨ����ǰ������ڴ��ַ
                ʧ�ܣ�-1
*******************************************************************************/
s32 bsp_edma_channel_stop(u32 channel_id)
{
    s32 curr_addr = 0;
    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;

    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return EDMA_FAIL;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];

    if (channel_id < EDMA_CHANNEL_END)
    {
        edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info.config &= ~(EDMA_CHANNEL_ENABLE);
        curr_addr = bsp_edma_current_transfer_address(channel_id);
        return curr_addr;
    }
    else
    {
	    hiedmac_trace(BSP_LOG_LEVEL_ERROR, "Channel id is wrong!\n");
        return EDMA_FAIL;
    }
}

/*******************************************************************************
  ������:       s32 bsp_edma_channel_is_idle (u32 channel_id)
  ��������:     ��ѯEDMAͨ���Ƿ����
  �������:     channel_id : ͨ��ID������ bsp_edma_channel_init�����ķ���ֵ
  �������:     ��
  ����ֵ:       0 : ͨ��æµ
                1 : ͨ������
                ���� : ʧ��
*******************************************************************************/
s32 bsp_edma_channel_is_idle (u32 channel_id)
{
    u32  ChannelState = 0;
    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;

    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return EDMA_FAIL;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];

    if (channel_id < (EDMA_CHANNEL_END))
    {
        ChannelState = edma_reg_str->ulChannelState;
        if ((1UL << edma_id_chan_id) & ChannelState)
        {
            return EDMA_CHN_BUSY;    /* The channel is active    */
        }
        else
        {
            return EDMA_CHN_FREE;    /* The channel is idle      */
        }
    }
    else
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel :%d is NOT VALID \n", channel_id);
        return EDMA_CHANNEL_INVALID;
    }
}

/*******************************************************************************
  ������:       s32 bsp_edma_chanmap_is_idle (u32 channel_map)
  ��������:     ��ѯEDMA�ļ���ͨ���Ƿ����
  �������:     channel_map : ͨ��IDs bit X Ϊ1 ��ʾ��ѯ xͨ��
  �������:     ��
  ����ֵ:       0 : ��ͨ��æµ
                1 : ��ѯ������ͨ������
                ���� : ʧ��
*******************************************************************************/
s32 bsp_edma_chanmap_is_idle (u32 channel_map)
{
    hiedmac_trace(BSP_LOG_LEVEL_ERROR, "func stub\n");
    return 1;
}

/*******************************************************************************
  ������:       s32 bsp_edma_channel_free (u32 channel_id)
  ��������:     �����ͷŹ���ͨ������Ҫ����:ȥʹ��ͨ���жϣ�����isr
  �������:     channel_id : ͨ��ID������ bsp_edma_channel_init�����ķ���ֵ
  �������:     ��
  ����ֵ:       0 : �ɹ��ͷ�
                ���� : ʧ��
*******************************************************************************/
s32 bsp_edma_channel_free (u32 channel_id)
{

    u32 chan_index = 0;
    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;

    struct edma_id_int_isr *edma_int_isr = 0;

    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return EDMA_FAIL;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);

    edma_int_isr = &(g_edma_drv_info.edma_int_isr[edma_id]);

    if (channel_id >= EDMA_CHANNEL_END)
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR," ----channel ID= %d, WRONG \n",  channel_id );
        return EDMA_FAIL;
    }
    else
    {
        (void)edma_channel_int_disable(channel_id);
        chan_index = (u32)edma_id_chan_id;
        /*do not need lock when freed */
        edma_int_isr->edma_trans1_isr[chan_index].chan_isr = NULL;
        edma_int_isr->edma_trans1_isr[chan_index].chan_arg = 0;
        edma_int_isr->edma_trans1_isr[chan_index].int_status = 0;

        edma_int_isr->edma_trans2_isr[chan_index].chan_isr = NULL;
        edma_int_isr->edma_trans2_isr[chan_index].chan_arg = 0;
        edma_int_isr->edma_trans2_isr[chan_index].int_status = 0;

        edma_int_isr->edma_err1_isr[chan_index].chan_isr = NULL;
        edma_int_isr->edma_err1_isr[chan_index].chan_arg = 0;
        edma_int_isr->edma_err1_isr[chan_index].int_status = 0;

        edma_int_isr->edma_err2_isr[chan_index].chan_isr = NULL;
        edma_int_isr->edma_err2_isr[chan_index].chan_arg = 0;
        edma_int_isr->edma_err2_isr[chan_index].int_status = 0;

        edma_int_isr->edma_err3_isr[chan_index].chan_isr = NULL;
        edma_int_isr->edma_err3_isr[chan_index].chan_arg = 0;
        edma_int_isr->edma_err3_isr[chan_index].int_status = 0;

        hiedmac_trace(BSP_LOG_LEVEL_DEBUG," ----channel %d is freed \n",  channel_id );
        return EDMA_SUCCESS;
    }
}

/*  �����ӡ�������жϷ�������еĴ�ӡ �� �ͼ���Ĵ�ӡ
    Ĭ�ϲ�ִ�� bsp_trace�� �ɵ��� set_edma_irq_trace 1 �� */
u32 edma_irq_trace = 0;
void set_edma_irq_trace(u32 irq_trace_flag)
{
    edma_irq_trace = irq_trace_flag;
}
#define hiedmac_irq_trace(level, fmt, ...) do{ \
    if(edma_irq_trace)\
        (bsp_trace(level, BSP_MODU_EDMA, "[edma]:<%s> <%d> "fmt, __FUNCTION__,__LINE__, ##__VA_ARGS__)); \
}while(0)

/*******************************************************************************
  ������:       static void edma_execute_channel_isr( chan_int_service *int_serve )
  ��������:     ���ж��ж���ɺ� [�ĸ�ͨ�������ж�]��ִ�о�����жϴ�������
  �������:     int_serve : chan_int_service�ṹ��� isr, ���������ִ�к����Ͳ���
  �������:     ��
  ����ֵ:       ��
*******************************************************************************/
static void edma_execute_channel_isr(struct chan_int_service *int_serve , u32 int_flag)
{
    if(int_serve->chan_isr != NULL)
    {
        hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG,"chan_isr is executed !\n");
        (int_serve->chan_isr)(int_serve->chan_arg, int_flag);
    }
    else
    {
        hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG,"chan_isr is NULL, no irq executed\n");
    }
}


/*******************************************************************************
  ������:       static void edma_isr_channel(u32 channel_id)
  ��������:     ���ж��жϵ�ͨ���󣬸�ͨ�����жϴ���
  �������:     channel_id : �����жϵ�ͨ��
  �������:     ��
  ����ֵ:       ��
*******************************************************************************/
/*lint -save -e438*/
/* Warning 438: (Warning -- Last value assigned   not used) */
static void edma_isr_channel(u32 edma_id_chan_id, u32 edma_id, u32 int_stat)
{
    u32 int_tc1    = 0;
    u32 int_tc2    = 0;
    u32 int_err1   = 0;
    u32 int_err2   = 0;
    u32 int_err3   = 0;
    u32 current_bit = 0;
    u32 chan_index = 0;
    u32 channel_id = 0;

    struct edma_reg_struct * edma_reg_str = 0;
    struct edma_id_int_isr* edma_id_int_isr;

    UNUSED(int_stat);

    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];
    edma_id_int_isr = &(g_edma_drv_info.edma_int_isr[edma_id]);

    int_tc1    = edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntTC1;
    int_tc2    = edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntTC2;
    int_err1   = edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntErr1;
    int_err2   = edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntErr2;
    int_err3   = edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntErr3;

    current_bit = (u32)0x1<<edma_id_chan_id;
    chan_index = edma_id_chan_id;
    channel_id = ( (1==edma_id) ? (edma_id_chan_id+16) : edma_id_chan_id );

    if (int_tc1 & current_bit)
    {
        hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG, "channel_id %d channel transfer success!\n",channel_id);
        /* Clear TC / Link_TC Interrupt */
        edma_reg_str->ulIntTC1Raw = current_bit;
        edma_execute_channel_isr(&(edma_id_int_isr->edma_trans1_isr[chan_index]), EDMA_INT_DONE);
    }
    else if (int_tc2 & current_bit)
    {
		hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG, "channel_id %d LLI transfer success!\n",channel_id);
        /* Clear LLI TC / Link_TC Interrupt */
        edma_reg_str->ulIntTC2Raw = current_bit;
        edma_execute_channel_isr(&(edma_id_int_isr->edma_trans2_isr[chan_index]), EDMA_INT_LLT_DONE);
	}
    else if (int_err1 & current_bit)
    {
		hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG, "channel_id %d transfer config_error1!\n", channel_id);
		/* Clear the Error / Link_Error interrupt */
		edma_reg_str->ulIntERR1Raw = current_bit;
        edma_execute_channel_isr(&(edma_id_int_isr->edma_err1_isr[chan_index]), EDMA_INT_CONFIG_ERR);
    }
	else if (int_err2 & current_bit)
	{
		hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG, "channel_id %d transfer data_error2\n", channel_id);
		edma_reg_str->ulIntERR2Raw        = current_bit;
        edma_execute_channel_isr(&(edma_id_int_isr->edma_err2_isr[chan_index]), EDMA_INT_TRANSFER_ERR);
	}
	else if (int_err3 & current_bit)
	{
        hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG, "channel_id %d transfer read_lli_error3!\n", channel_id);
		/* Clear the Error / Link_Error interrupt */
		edma_reg_str->ulIntERR3Raw    = current_bit;
        edma_execute_channel_isr(&(edma_id_int_isr->edma_err3_isr[chan_index]), EDMA_INT_READ_ERR);
    }
    else
    {
        hiedmac_irq_trace(BSP_LOG_LEVEL_WARNING,"Error in edma_isr!\n");
		return;
    }
}
/*lint -restore +e438*/
/*******************************************************************************
  ������:       static irqreturn_t edma_isr(s32 irq, void *private)
  ��������:     EDMA���������жϷ�������жϲ���ʱ�����ã������ж����ĸ�
                ͨ���������жϣ�������Ӧͨ�����жϴ������
  �������:     irq : �жϺţ�����request_irq����ע���жϷ������ʱ��req����ֵ
                private : ����request_irq����ע���жϷ������ʱ��dev����ֵ
  �������:     ��
  ����ֵ:       IRQ_HANDLED ���ж��Ѿ�������
*******************************************************************************/
static OSL_IRQ_FUNC(irqreturn_t, edma_isr, irq, dev_id)
{
    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;
    u32 int_stat = 0;
    u32 curr_bit = 0;

    edma_id = (s32)dev_id;
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];

    int_stat = edma_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntState;

    hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG," ----shouldn't be 0, ulChannel_Int_Status_Reg = 0x%X \n", int_stat);
    /* decide which channel has trigger the interrupt*/
    for (edma_id_chan_id = EDMA_CHANNEL_START; edma_id_chan_id < EDMA_CH16_NUM; edma_id_chan_id++)
    {
        curr_bit = 1UL << edma_id_chan_id;
        if (int_stat & curr_bit)
        {
            hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG," intterupt from channel %d \n", edma_id_chan_id);
            edma_isr_channel((u32)edma_id_chan_id, (u32)edma_id , int_stat);/* Handle channel_id interrupt */
        }
    }
	return IRQ_HANDLED;
}

/*******************************************************************************
  ������:      s32 bsp_edma_channel_set_config (u32 channel_id,
                       u32 direction, u32 burst_width, u32 burst_len)
  ��������:    ����ʽEDMA����ʱ�����ñ���������ͨ��������
               ��ʽEDMA����ʱ������Ҫʹ�ñ�������
  �������:    channel_id : ͨ��ID������ bsp_edma_channel_init�����ķ���ֵ
               direction : EDMA���䷽��, ȡֵΪEDMA_P2M��EDMA_M2P��
                           EDMA_M2M֮һ
               burst_width��ȡֵΪ0��1��2��3����ʾ��burstλ��Ϊ8��16��32��64bit
               burst_len��ȡֵ��Χ0~15����ʾ��burst����Ϊ1~16
  �������:    ��
  ����ֵ:      �ɹ���0
               ʧ�ܣ�����
*******************************************************************************/
s32 bsp_edma_channel_set_config (u32 channel_id, u32 direction, u32 burst_width, u32 burst_len)
{
    u32 chan_cfg = 0;
    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;

    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return EDMA_FAIL;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];

    chan_cfg = edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info.config;
    if( (channel_id <  EDMA_CHANNEL_END) \
         && (burst_width <= EDMA_TRANS_WIDTH_64) \
         && (burst_len   <= EDMA_BUR_LEN_16) )
    {
        chan_cfg &= (u32)0x1F0; /*CX_CONFIG_PERI bitλ 4-9, bit9��δ��*/
        switch (direction)
        {
            case EDMA_P2M:
                chan_cfg |= P2M_CONFIG;
                break;
            case EDMA_M2P:
                chan_cfg |= M2P_CONFIG;
                break;
            case EDMA_M2M:
                chan_cfg |= M2M_CONFIG;
                break;
            default:
                hiedmac_trace(BSP_LOG_LEVEL_ERROR,\
                    "ChannelConfing direction=%d, ERROR \n", direction);
                break;
        }
        chan_cfg |= EDMAC_BASIC_CONFIG(burst_width, burst_len);
        edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info.config = chan_cfg;/*lint !e662 !e661*/
        hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"stChannelConfing->config = 0x%X!\n", chan_cfg);
        return EDMA_SUCCESS;
    }
    else
    {
         hiedmac_trace(BSP_LOG_LEVEL_ERROR,\
                       "ChanConfing ERROR chan=%d, burstWidth=%d, Len=%d\n",\
                        channel_id, burst_width, burst_len);
         return EDMA_CONFIG_ERROR;
    }
}

/*******************************************************************************
  ������:      int balong_dma_channel_dest_set_config (UINT32 channel_id,
                       UINT32 burst_width, UINT32 burst_len)
  ��������:    BBP GRIF��Ҫ��������Ŀ������λ��ͳ��ȣ����ñ��������������β�ʹ�á�
               ʹ��ʱ���ýӿ���balong_dma_channel_set_config֮����á�
  �������:    channel_id��ͨ��ID������balong_dma_channel_init�����ķ���ֵ
               burst_width��ȡֵΪ0��1��2��3����ʾ��burstλ��Ϊ8��16��32��64bit
               burst_len��ȡֵ��Χ0~15����ʾ��burst����Ϊ1~16
  �������:    ��
  ����ֵ:      �ɹ���0
               ʧ�ܣ�����
*******************************************************************************/
s32 bsp_edma_channel_dest_set_config (u32 channel_id, u32 dest_width, u32 dest_len)
{
    u32 chan_cfg = 0;
    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;

    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return EDMA_FAIL;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];

    chan_cfg = edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info.config;

    /* ��������Ŀ������burstλ��ͳ���,����ߴ������� */
    chan_cfg &=(~(EDMAC_TRANSFER_CONFIG_DEST_WIDTH_MASK|EDMAC_TRANSFER_CONFIG_DEST_BURST_LENGTH_MASK));
    chan_cfg |= EDMAC_TRANSFER_CONFIG_DEST_WIDTH(dest_width);
    chan_cfg |= EDMAC_TRANSFER_CONFIG_DEST_BURST_LENGTH(dest_len);

    edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info.config = chan_cfg;

    return EDMA_SUCCESS;
}


/*******************************************************************************
  ������:      s32 bsp_edma_channel_start (u32 channel_id, u32 src_addr,
                       u32 des_addr, u32 len)
  ��������:    ����һ��ͬ��EDMA����, EDMA������ɺ󣬲ŷ���
               ʹ�ñ�����ʱ������Ҫע���жϴ�����
               len<65536(64K):һά����
               65536(64K)<=len<16384*65536(1G)ʱ:��ά����
               len>16384*65536:��֧��
               ��ά����lenΪ16K��������������������֧��

  �������:    channel_id��ͨ��ID,���� bsp_edma_channel_init�����ķ���ֵ
               src_addr�����ݴ���Դ��ַ�������������ַ
               des_addr�����ݴ���Ŀ�ĵ�ַ�������������ַ
               len�����ݴ��䳤�ȣ���λ���ֽڣ�һ�δ������ݵ���󳤶���65535�ֽ�
  �������:    ��
  ����ֵ:      �ɹ���0
               ʧ�ܣ�����
*******************************************************************************/
s32 bsp_edma_channel_start (u32 channel_id, u32 src_addr, u32 des_addr, u32 len)
{
    u32 reg_value = 0;
    u32 loop_index = 0;
    u32 wait_loop = EDMA_DATA_TIMEOUT;

    u32 bindx = 0;
    u32 cnt0 = 0;

    u32 b_count = 0;
    u32 a_count = 0;

    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;
    volatile struct edma_cb * lli_node_tmp = 0;

    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return EDMA_FAIL;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];

    hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG,"--------------------------edma_start!\n");
    reg_value = edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info.config;
    hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG,"---------pre stChannelConfing->config = 0x%X\n",reg_value);

    if(len >= SZ_64K)  /* 2 vec  -  align_size = 16k */
    {
        b_count = (!(len%SZ_16K)) ? (len/SZ_16K - 1) \
                : (!(len%SZ_8K )) ? (len/SZ_8K  - 1) \
                : (!(len%SZ_4K )) ? (len/SZ_4K  - 1) : 0;
        if(0==b_count)
        {
            hiedmac_trace(BSP_LOG_LEVEL_ERROR,"try 2vec failed! not aligned. channel:%d size:0x%x \n",channel_id ,len);
            return EDMA_TRXFERSIZE_INVALID;
        }
        else
        {
            a_count = len / (b_count+1);
            bindx = (((reg_value & EDMAC_TRANSFER_CONFIG_SOUR_INC) ? (a_count << 16) : 0) | \
                     ((reg_value & EDMAC_TRANSFER_CONFIG_DEST_INC) ? a_count : 0));
            cnt0  =  (b_count<<16)|a_count;
            wait_loop *= 2;
        }
    }
    else    /* 1 vec  -  len < 64k */
    {
        cnt0 = len;
    }

    if (channel_id >= EDMA_CHANNEL_END)
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR,"EDMA_CHANNEL_INVALID!\n");
        return EDMA_CHANNEL_INVALID;
    }
    else
    {
        lli_node_tmp = &(edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info);
        lli_node_tmp->config  = EDMA_CHANNEL_DISABLE;

        lli_node_tmp->lli       = 0;
        lli_node_tmp->bindx     = bindx;
        lli_node_tmp->cindx     = 0;
        lli_node_tmp->cnt1      = 0;
        lli_node_tmp->cnt0      = cnt0;
        lli_node_tmp->src_addr  = src_addr;
        lli_node_tmp->des_addr  = des_addr;
        edma_cache_sync();
        lli_node_tmp->config   = reg_value | EDMA_CHANNEL_ENABLE;
        edma_cache_sync();
        reg_value = lli_node_tmp->config;
        hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG,"--------later stChannelConfing->config = 0x%X\n",reg_value);

        for( loop_index=0; loop_index<wait_loop; loop_index++ )
        {
            msleep(1);
            if ( bsp_edma_channel_is_idle(channel_id) )
            {
                break;
            }
        }
        if(wait_loop==loop_index)
        {
            hiedmac_irq_trace(BSP_LOG_LEVEL_WARNING,"edma_start trans is timeout!\n");
            return EDMA_TRXFER_ERROR;
        }
        else
        {
            hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"edma_start trans_complete!\n");
            return EDMA_SUCCESS;
        }
    }
}



/*******************************************************************************
  ������:      s32 bsp_edma_channel_2vec_start (u32 channel_id, u32 src_addr,
                       u32 des_addr, u32 len)
  ��������:    ����dsp ������ݵĴ��䡣����һ��ͬ��--2ά--EDMA����, EDMA������ɺ󣬲ŷ���
               ʹ�ñ�����ʱ������Ҫע���жϴ�����
  �������:    channel_id��ͨ��ID,���� bsp_edma_channel_init�����ķ���ֵ
               src_addr�����ݴ���Դ��ַ�������������ַ
               des_addr�����ݴ���Ŀ�ĵ�ַ�������������ַ
               len�����ݴ��䳤�ȣ���λ���ֽڣ������ڴ������65535�ֽڵ����ݡ�
               size_align: ��ά����ʱ Ҫ�� �� size_align ���룬 �� len/size_align < 0x10000��
               ***���飬ʹ�� 2ά����ʱ��set_config���� burst_width-64bit burst_len-16
  �������:    ��
  ����ֵ:      �ɹ���0
               ʧ�ܣ�����
*******************************************************************************/

s32 bsp_edma_channel_2vec_start (u32 channel_id, u32 src_addr, u32 des_addr, u32 len, u32 size_align)
{
    u32 edma_ret = 0;
    u32 reg_value = 0;
    u32 array_cnt = 0;
    u32 bindx_set = 0;
    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;
    volatile struct edma_cb * lli_node_tmp = 0;

    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return EDMA_FAIL;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];
    lli_node_tmp = &(edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info);

    if((size_align>=0x8000)||(0==size_align))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR," input size_align=0x%X is invalide. \n",size_align);
        return ERROR;
    }
    edma_ret = len%size_align;
    if( 0 != edma_ret)
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR,"input len=0x%X not aligned . \n",len);
        return ERROR;
    }

    array_cnt = (len/size_align)-1;
    if( 0x10000 <= array_cnt)
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR,"input len=0x%X too big, try increase size_align. \n",len);
        return ERROR;
    }

    hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"--------------------------edma_2vec_start!\n");
    reg_value = lli_node_tmp->config;
    hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"---------pre stChannelConfing->config = 0x%X\n",reg_value);

    /* if src_index is inc, src_bindx should be set to size_align;
       if not, src_bindx is set to 0 */
    if(reg_value&EDMAC_TRANSFER_CONFIG_SOUR_INC)
    {
        bindx_set = bindx_set|(size_align<<16);
    }
    if(reg_value&EDMAC_TRANSFER_CONFIG_DEST_INC)
    {
        bindx_set = bindx_set|(size_align);
    }

    if (channel_id >= EDMA_CHANNEL_END)
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR,"EDMA_CHANNEL_INVALID!\n");
        return EDMA_CHANNEL_INVALID;
    }
    else
    {
        lli_node_tmp->config  = EDMA_CHANNEL_DISABLE;

        lli_node_tmp->lli       = 0;
        lli_node_tmp->bindx     = bindx_set;
        lli_node_tmp->cindx     = 0;
        lli_node_tmp->cnt1      = 0;
        lli_node_tmp->cnt0      = (array_cnt<<16|size_align);
        lli_node_tmp->src_addr  = src_addr;
        lli_node_tmp->des_addr  = des_addr;
        edma_cache_sync();
        lli_node_tmp->config   = reg_value | EDMA_CHANNEL_ENABLE;
        edma_cache_sync();
        reg_value = lli_node_tmp->config;
        hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"--------later stChannelConfing->config = 0x%X\n",reg_value);

        return EDMA_SUCCESS;
    }
}

/*******************************************************************************
  ������:      s32 bsp_edma_channel_async_start (u32 channel_id,
                u32 src_addr, u32 des_addr, u32 len)
  ��������:    ����һ���첽EDMA���䡣����EDMA����󣬾ͷ��ء����ȴ�EDMA������ɡ�
               ʹ�ñ�����ʱ��ע���жϴ��������жϴ������д���EDMA��������¼�
               ���ߣ���ע���жϴ�������ʹ�� bsp_edma_channel_is_idle������ѯ
               EDMA�����Ƿ����
  �������:    channel_id��ͨ��ID,���� bsp_edma_channel_init�����ķ���ֵ
               src_addr�����ݴ���Դ��ַ�������������ַ
               des_addr�����ݴ���Ŀ�ĵ�ַ�������������ַ
               len�����ݴ��䳤�ȣ���λ���ֽڣ�һ�δ������ݵ���󳤶���65535�ֽ�
  �������:    ��
  ����ֵ:      �ɹ���0
               ʧ�ܣ�����
*******************************************************************************/
s32 bsp_edma_channel_async_start (u32 channel_id, u32 src_addr, u32 des_addr, u32 len)
{
    u32 reg_value = 0;
    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;
    volatile struct edma_cb * lli_node_tmp = 0;


    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return EDMA_FAIL;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];
    lli_node_tmp = &(edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info);

    hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"--------------------------edma_start!\n");
    reg_value = lli_node_tmp->config;
    hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"---------pre stChannelConfing->config = 0x%X\n",reg_value);

    if (channel_id >= EDMA_CHANNEL_END)
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR,"EDMA_CHANNEL_INVALID\n");
        return EDMA_CHANNEL_INVALID;
    }
    else
    {
        lli_node_tmp->config  = EDMA_CHANNEL_DISABLE;

        lli_node_tmp->lli       = 0;
        lli_node_tmp->bindx     = 0;
        lli_node_tmp->cindx     = 0;
        lli_node_tmp->cnt1      = 0;
        lli_node_tmp->cnt0      = len;
        lli_node_tmp->src_addr  = src_addr;
        lli_node_tmp->des_addr  = des_addr;
        edma_cache_sync();
        lli_node_tmp->config   = reg_value | EDMA_CHANNEL_ENABLE;
        edma_cache_sync();

        return EDMA_SUCCESS;
    }
}

/*******************************************************************************
  ������:      struct edma_cb *bsp_edma_channel_get_lli_addr(u32 channel_id)
  ��������:    ��ȡָ��EDMAͨ����������ƿ����ʼ��ַ
  �������:    channel_id��ͨ��ID,���� bsp_edma_channel_init�����ķ���ֵ
  �������:    ��
  ����ֵ:      �ɹ���������ƿ���ʼ��ַ
               ʧ�ܣ�0
*******************************************************************************/
struct edma_cb *bsp_edma_channel_get_lli_addr (u32 channel_id)
{
    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;

    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return NULL;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];

    if (channel_id >= EDMA_CHANNEL_END)
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR,"EDMA_CHANNEL_INVALID\n");
        return NULL;
    }
    else
    {
        /*warning : return discards 'volatile' qualifier*/
        return (struct edma_cb *)&(edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info);
    }
}

/*******************************************************************************
  ������:      s32 bsp_edma_channel_lli_start (u32 channel_id)
  ��������:    ������ʽEDMA���䡣����ʽEDMA�����нڵ㴫�䶼ȫ����ɺ�ŷ��ء�
               ��ʽEDMA��ÿ���ڵ����������䳤��Ϊ65535�ֽڡ�
               ע�⣺���ô˺���ǰ���������ú�������ƿ顣
  �������:    channel_id��ͨ��ID,���� bsp_edma_channel_init�����ķ���ֵ
  �������:    ��
  ����ֵ:      �ɹ���0
               ʧ�ܣ�����
*******************************************************************************/
s32 bsp_edma_channel_lli_start (u32 channel_id)
{
    u32 reg_value = 0;
    u32 loop_index = 0;
    u32 wait_loop = EDMA_DATA_TIMEOUT;

    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;
    volatile struct edma_cb * lli_node_tmp = 0;

    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return EDMA_FAIL;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];
    lli_node_tmp = &(edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info);


    hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"--------------------------edma_start!\n");
    reg_value = lli_node_tmp->config;
    hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"-----------pre_stChannelConfing->config = 0x%X\n",reg_value);
    if (channel_id >= EDMA_CHANNEL_END)
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR,"EDMA_CHANNEL_INVALID\n");
        return EDMA_CHANNEL_INVALID;
    }
    else
    {
        edma_cache_sync();
        lli_node_tmp->config = reg_value | EDMA_CHANNEL_ENABLE;
        edma_cache_sync();

        reg_value = lli_node_tmp->config;
        hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"---------later stChannelConfing->config = 0x%X\n",reg_value);

        for(loop_index=0;loop_index<wait_loop;loop_index++)
        {
            msleep(1);
            if (bsp_edma_channel_is_idle(channel_id))
            {
                break;
            }
        }
        if(wait_loop==loop_index)
        {
            hiedmac_trace(BSP_LOG_LEVEL_ERROR,"edma_lli_start trans is timeout!\n");
            return EDMA_TRXFER_ERROR;
        }
        else
        {
            hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG,"edma_lli_start trans_complete!\n");
            return EDMA_SUCCESS;
        }
    }
}

/*******************************************************************************
  ������:      s32 bsp_edma_channel_lli_start (u32 channel_id)
  ��������:    ������ʽEDMA���䣬Ȼ���������أ����ȴ�EDMA������ɡ�
               ��ʽEDMA��ÿ���ڵ����������䳤��Ϊ65535�ֽڡ�
               ע�⣺���ô˺���ǰ���������ú�������ƿ顣
  �������:    channel_id��ͨ��ID,���� bsp_edma_channel_init�����ķ���ֵ
  �������:    ��
  ����ֵ:      �ɹ���0
               ʧ�ܣ�����
*******************************************************************************/
s32 bsp_edma_channel_lli_async_start (u32 channel_id)
{
    u32 reg_value = 0;
    s32 edma_id = -1;
    s32 edma_id_chan_id = -1;
    struct edma_reg_struct * edma_reg_str = 0;
    volatile struct edma_cb * lli_node_tmp = 0;

    edma_id = EDMA_ID(channel_id);
    if(!edma_id_valid(edma_id))
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR, "channel_id %d not supported \n", channel_id);
        return EDMA_FAIL;
    }
    edma_id_chan_id = (s32)EDMA_ID_CHAN_ID(channel_id);
    edma_reg_str = g_edma_drv_info.edma_reg_str[edma_id];
    lli_node_tmp = &(edma_reg_str->stTransferConfig[edma_id_chan_id].lli_node_info);

    hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"--------------------------edma_start!\n");
    reg_value = lli_node_tmp->config;
    hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"-----------pre_stChannelConfing->config = 0x%X\n",reg_value);
    if (channel_id >= EDMA_CHANNEL_END)
    {
        hiedmac_trace(BSP_LOG_LEVEL_ERROR,"EDMA_CHANNEL_INVALID\n");
        return EDMA_CHANNEL_INVALID;
    }
    else
    {
        edma_cache_sync();
        lli_node_tmp->config = reg_value | EDMA_CHANNEL_ENABLE;
        edma_cache_sync();

        reg_value = lli_node_tmp->config;
        hiedmac_irq_trace(BSP_LOG_LEVEL_DEBUG,"--------later stChannelConfing->config = 0x%X\n",reg_value);
        return EDMA_SUCCESS;
    }
}

/*******************************************************************************
  ������:       static s32 __devinit bsp_edma_suspend (struct platform_device *dev)
  ��������:     ϵͳ�µ�ǰ�����ã����ڱ���EDMAӲ����״̬��������һЩ��Ҫ�Ĵ���
                ȷ������suspend��resume���̺�EDMA����������������
  �������:     dev����ʾ��ǰ�豸
  �������:     ��
  ����ֵ:       �ɹ���0
                ʧ�ܣ���0
*******************************************************************************/
u32 edma_ctrl_bak = 0;


u32 edma_4_chan_saved[2] = {EDMA_CH_HSUART_RX,EDMA_CH_HSUART_TX};
u32 edma_4_chan_mask = 0x6;

EDMAC_CPU_REG_STRU edma_4_int_mask_bak;

struct EDMAC_TRANSFER_CONFIG_STRU edma_4_chan_cfg[2];

static int bsp_edma_suspend (void)
{
    s32 edma4_ret = -1;
    struct edma_reg_struct * edma4_reg_str = 0;
    
	edma_ctrl_bak = g_edma_drv_info.edma_reg_str[EDMA_CH4_ID]->ulDmaCtrl;

    edma4_reg_str = g_edma_drv_info.edma_base_addr[EDMA_CH4_ID];
    edma4_ret = (s32)(edma4_reg_str->ulChannelState & edma_4_chan_mask);
    if((edma4_ret)!=0)
        {return EDMA_FAIL;}
    edma_4_int_mask_bak = edma4_reg_str->stCpuXReg[EDMAC_DT_ARM];
    edma_4_chan_cfg[0] = edma4_reg_str->stTransferConfig[1];
    edma_4_chan_cfg[1] = edma4_reg_str->stTransferConfig[2];
    
	return 0;
}

/*******************************************************************************
  ������:       static s32 __devinit bsp_edma_resume(struct platform_device *dev)
  ��������:     ϵͳ�µ�ǰ�����ã����ڱ���EDMAӲ����״̬��������һЩ��Ҫ�Ĵ���
                ȷ������suspend��resume���̺�EDMA����������������
  �������:     dev����ʾ��ǰ�豸
  �������:     ��
  ����ֵ:       �ɹ���0
                ʧ�ܣ���0
*******************************************************************************/
static void bsp_edma_resume(void)
{
    struct edma_reg_struct * edma4_reg_str = 0;


        /* edma_ch4 outstanding ��Ϊ4 */
        g_edma_drv_info.edma_reg_str[EDMA_CH4_ID]->ulDmaCtrl |= (edma_ctrl_bak&(0x1<<4));


    edma4_reg_str = g_edma_drv_info.edma_base_addr[EDMA_CH4_ID];
    edma4_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntTC1Mask |= edma_4_int_mask_bak.ulIntTC1Mask&edma_4_chan_mask;
    edma4_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntTC2Mask |= edma_4_int_mask_bak.ulIntTC2Mask&edma_4_chan_mask;
    edma4_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntErr1Mask |= edma_4_int_mask_bak.ulIntErr1Mask&edma_4_chan_mask;
    edma4_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntErr2Mask |= edma_4_int_mask_bak.ulIntErr2Mask&edma_4_chan_mask;
    edma4_reg_str->stCpuXReg[EDMAC_DT_ARM].ulIntErr3Mask |= edma_4_int_mask_bak.ulIntErr3Mask&edma_4_chan_mask;
    edma4_reg_str->stTransferConfig[1] = edma_4_chan_cfg[0];
    edma4_reg_str->stTransferConfig[2] = edma_4_chan_cfg[1];

}

/*******************************************************************************
  ������:       static void __devinit bsp_edma_shutdown (struct platform_device *dev)
  ��������:     �ڹػ������б����ã���һЩ��Ҫ�Ĺػ�ǰ����
  �������:     dev����ʾ��ǰ�豸
  �������:     ��
  ����ֵ:       ��
*******************************************************************************/
static void bsp_edma_shutdown (struct platform_device *dev)
{
	 return;
}

/*******************************************************************************
  ������:       static s32 bsp_edma_probe
  ��������:     ��EDMAC�ļĴ��������ַӳ��������ַ��ע���жϷ������
                EDMAȫ�ּĴ������ڴ˳�ʼ������vxWorks EDMA���������г�ʼ����
  �������:     pdev��ָ��EDMAC�豸��ָ�롣
  �������:     ��
  ����ֵ:       �ɹ���0
                ʧ�ܣ���0
*******************************************************************************/


static struct syscore_ops edma_syscore_ops = {
	.suspend = bsp_edma_suspend,
	.resume = bsp_edma_resume,
	};
static s32 bsp_edma_probe(struct platform_device *pdev)
{
    s32 ret = -1;
    const char * clk_name = NULL;
    u32 i = 0;

	struct device_node *edma_node = NULL;
	const char *edma_name[EDMA_NUMBER] = {"hisilicon,edma0","hisilicon,edma1"};
	void *edma_iomap = NULL;

    memset((void*)g_edma_chan_distri,-1,sizeof(g_edma_chan_distri));
    memset((void*)(&g_edma_drv_info),0,sizeof(struct edma_drv_info));

    for(i = 0; i<EDMA_NUMBER ; i++)
    {
        /* ��DTS */
    	edma_node = of_find_compatible_node(NULL, NULL, edma_name[i]);
    	if(!edma_node)
    	{
    		hiedmac_trace(BSP_LOG_LEVEL_ERROR, "edma %d of_find_compatible_node not found\n", i );
    	}
        else
        {   
        	/* �ڴ�ӳ�䣬��û�ַ */
        	edma_iomap = (char *)of_iomap(edma_node, 0);
        	if (NULL == edma_iomap)
        	{
        		hiedmac_trace(BSP_LOG_LEVEL_ERROR, "edma %d of_iomap failed\n", i );
        		return ERROR;
        	}
            g_edma_drv_info.irq_no[i] = irq_of_parse_and_map(edma_node, 0);


            g_edma_drv_info.edma_base_addr[i] = (void *)edma_iomap;
            g_edma_drv_info.edma_reg_str[i] =\
                (struct edma_reg_struct *)g_edma_drv_info.edma_base_addr[i];
            
            if(of_property_read_string(edma_node, "clk_name", &clk_name))
            {
                hiedmac_trace(BSP_LOG_LEVEL_ERROR,"can not get clk_name, edma_id=%d need not clk control\n",i);
            }
            else
            {
        	    g_edma_drv_info.clk[i] = clk_get(NULL, clk_name);
                if(IS_ERR(g_edma_drv_info.clk[i])){
                    hiedmac_trace(BSP_LOG_LEVEL_ERROR,"fail to get clk,clk_name:%s\n",clk_name);
                    return ERROR;
                }else{
                    clk_prepare_enable( g_edma_drv_info.clk[i] );
                }
            }

            /* Clear interrupt */
            g_edma_drv_info.edma_reg_str[i]->ulIntTC1Raw  = 0;
            g_edma_drv_info.edma_reg_str[i]->ulIntTC2Raw  = 0;
            g_edma_drv_info.edma_reg_str[i]->ulIntERR1Raw = 0;
            g_edma_drv_info.edma_reg_str[i]->ulIntERR2Raw = 0;
        	g_edma_drv_info.edma_reg_str[i]->ulIntERR3Raw = 0;
            
            hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"edmac request irq = %d\n", g_edma_drv_info.irq_no[i]);
            if(request_irq(g_edma_drv_info.irq_no[i], (irq_handler_t)edma_isr, IRQF_DISABLED, "EDMAC", (void *)i))
            {
                hiedmac_trace(BSP_LOG_LEVEL_CRIT,"edma id=%d Irq=%d request failed!\n", i,g_edma_drv_info.irq_no[i]);
                ret = EDMA_FAIL;
                return ret;
        	}
        }

    }

    edma_init();

    hiedmac_trace(BSP_LOG_LEVEL_DEBUG, "edmac probe OK!\n");
    return EDMA_SUCCESS;
}

s32 bsp_edma_remove(struct platform_device *pdev)
{
    u32 i = 0;
    for(i = 0; i<EDMA_NUMBER ; i++)
    {
        if(g_edma_drv_info.edma_base_addr[i])
            free_irq(g_edma_drv_info.irq_no[i], (void*)i );
    }
    return 0;
}

static struct platform_driver edmac_driver = {
	.probe = bsp_edma_probe,
	.remove = bsp_edma_remove,
	.shutdown = bsp_edma_shutdown,
	.driver = {
		.name = DRIVER_NAME,
		.owner  = THIS_MODULE,
		.pm     = NULL,
	},
};

static struct platform_device edmac_device =
{
    .name = DRIVER_NAME,
    .id       = 1,
    .num_resources = 0,
};

static s32 __init bsp_edma_init(void)
{
    s32 retval;

    hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"bsp_edmac_module_init start\n");
    retval = platform_device_register(&edmac_device);
    if (retval)
    {
        hiedmac_trace(BSP_LOG_LEVEL_CRIT,"hisilicon platform_device_register failed!\n");
        return retval;
    }

    retval = platform_driver_register(&edmac_driver);

    if (retval)
    {
        hiedmac_trace(BSP_LOG_LEVEL_CRIT,"hisilicon platform devicer register Failed!\n");
        platform_device_unregister(&edmac_device);
        return retval;
    }
	register_syscore_ops(&edma_syscore_ops);

    hiedmac_trace(BSP_LOG_LEVEL_DEBUG,"bsp_edmac_module_init end\n");
    return retval;
}

static void __exit bsp_edma_exit(void)
{
    platform_driver_unregister(&edmac_driver);
    platform_device_unregister(&edmac_device);
}

int  mdrv_edma_bbp_sample_reboot(void)
{return -1;}

EXPORT_SYMBOL_GPL(bsp_edma_channel_init);
EXPORT_SYMBOL_GPL(bsp_edma_channel_lli_async_start);
EXPORT_SYMBOL_GPL(bsp_edma_channel_stop);
EXPORT_SYMBOL_GPL(bsp_edma_channel_free);
EXPORT_SYMBOL_GPL(bsp_edma_current_transfer_address);
EXPORT_SYMBOL_GPL(bsp_edma_channel_get_lli_addr);
EXPORT_SYMBOL_GPL(bsp_edma_channel_start);
EXPORT_SYMBOL_GPL(bsp_edma_channel_async_start);
EXPORT_SYMBOL_GPL(bsp_edma_channel_lli_start);
EXPORT_SYMBOL_GPL(bsp_edma_channel_set_config);
EXPORT_SYMBOL_GPL(bsp_edma_channel_is_idle);
EXPORT_SYMBOL_GPL(bsp_edma_channel_2vec_start);

subsys_initcall(bsp_edma_init);
module_exit(bsp_edma_exit);/*lint !e19*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hisilicon Drive Group");
MODULE_VERSION("HI_VERSION=" OSDRV_MODULE_VERSION_STRING);



