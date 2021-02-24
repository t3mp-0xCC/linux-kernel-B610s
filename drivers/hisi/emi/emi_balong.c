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




/*lint --e{537,958,64,734}*/
#include <osl_types.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <drv_nv_def.h>
#include <drv_nv_id.h>
#include <bsp_nvim.h>
#include <bsp_memmap.h>
//#include <hi_syscrg_interface.h>
//#include <hi_syssc.h>
#include <bsp_om.h>
#include <bsp_version.h>
#include <bsp_sysctrl.h>
#include <linux/semaphore.h>
#include <osl_sem.h>
#include <linux/emi/emi_balong.h>
#include <hi_emi.h>
#include <linux/delay.h>
#include <linux/of_address.h>
#include <linux/of.h>


#define EMI_OK 0
#define EMI_ERROR 1

#define  hiemi_error(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_EMI, "[emi]: <%s> "fmt"\n", __FUNCTION__, ##__VA_ARGS__))
#define  hiemi_trace(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_INFO, BSP_MODU_EMI, "[emi]: <%s> "fmt"\n", __FUNCTION__, ##__VA_ARGS__))
struct clk * emi_clk = NULL;
emi_reg_t * g_emi_reg_bak = NULL;
struct st_emi {
	struct semaphore    edmatx_semmux;
	u32 edmatx_ok_cnt;
	u32 edmatx_err_cnt;
};

struct emi_ctrl{
    void __iomem    *regbase;
    void __iomem    *membase;
    void __iomem    *rmembase;
};

typedef enum{
    EMI_BASE_ADDR = 0,
    EMI_OFFSET,
    EMI_START_BIT,
    EMI_END_BIT,
    EMI_ADDR_BUTT
}EMI_DTS_SYSCTRL_ADDR;

typedef enum{
    EMI_DTS_EMI_SRST_EN = 0,
    EMI_DTS_EMI_SRST_DIS,
    EMI_DTS_EBI_NORMAL_MODE,
    EMI_DTS_REG_BUTT
}EMI_DTS_SYSCTRL;

struct emi_sysctrl{
    u32 base_addr;
    u32 offset;
    u32 start_bit;
    u32 end_bit;
};

struct emi_debug{
    u32 emi_trans_way;
};

struct emi_info{
    struct emi_sysctrl emi_sysctrl[EMI_DTS_REG_BUTT];
    struct st_emi emi_st;
    struct emi_ctrl emi_ctrl;
    struct emi_debug emi_debug;
    struct edma_cb* edma_cb;
    edma_addr_t edma_addr;
    unsigned int emi_wrt_data_8bit_offset;
    unsigned int emi_wrt_data_16bit_offset;
    unsigned int emi_nv;
};

static struct emi_info g_emi_info;

static inline int hi_emi_get_reg(unsigned int *value,unsigned int base,unsigned int offset,unsigned int lowbit,unsigned int highbit)
 {
     unsigned int mask = 0;
     unsigned int temp = 0;

     temp   = readl((void*)(base + offset));
     mask   = ((1 << (highbit - lowbit + 1)) -1) << lowbit;
     *value = temp & mask;
     *value = (*value) >> lowbit;

     return EMI_OK;
 }

 static inline int hi_emi_set_reg(unsigned int value,unsigned int base,unsigned int offset,unsigned int lowbit,unsigned highbit)
 {
     unsigned int reg    = base + offset;
     unsigned int temp   = 0;
     unsigned int mask   = 0;

     temp   = readl((void*)reg);
     mask   = ((1 << (highbit - lowbit + 1)) -1) << lowbit;
     value  = (temp & (~mask)) | ((value <<lowbit) & mask);
     writel(value  ,(void*)reg);

     return EMI_OK;

 }


void emi_trace_level_set(u32 level)
{
    bsp_mod_level_set(BSP_MODU_EMI, level);
    printk("bsp_mod_level_set(BSP_MODU_EMI=%d, %d)\n",BSP_MODU_EMI,level);
}
void emi_enable_clk(void)
{
    int ret = 0;
    ret = clk_enable(emi_clk);
    if(ret)
    {
        hiemi_error("clk enable failed !\n");
    }
    return;
}
void emi_disable_clk(void)
{

    unsigned int cnt,emi_done;
    unsigned int fifo_max_delay = 64;
    //fifo_cnt = 63;fifo_cnt_mask = 16;

    hi_emi_get_reg(&cnt,(u32)(g_emi_info.emi_ctrl.regbase),HI_EMI_SMBSR0_OFFSET,16,21);
    //cnt = hi_emi_fifo_cnt();
    do
    {
        while(0 != cnt)
        {
            udelay(1);
            fifo_max_delay --;
            //fifo_cnt = 63;fifo_cnt_mask = 16;
            hi_emi_get_reg(&cnt,(u32)(g_emi_info.emi_ctrl.regbase),HI_EMI_SMBSR0_OFFSET,16,21);
            //cnt = hi_emi_fifo_cnt();
            if(0 == fifo_max_delay)
            {
                hiemi_error("clear fifo time out! \n");
                break;
            }
        }
        if(0 == cnt)
        {
            udelay(1);
            fifo_max_delay--;
            //fifo_cnt = 63;fifo_cnt_mask = 16;
            hi_emi_get_reg(&cnt,(u32)(g_emi_info.emi_ctrl.regbase),HI_EMI_SMBSR0_OFFSET,16,21);
            //cnt = hi_emi_fifo_cnt();
            if(0 == cnt)
            {

                hi_emi_get_reg(&emi_done,(u32)(g_emi_info.emi_ctrl.regbase),HI_EMI_SMBSR0_OFFSET,31,31);
                if(emi_done == 0)
                {
                    udelay(1);
                }

                clk_disable(emi_clk);
                break;
            }
            if(0 == fifo_max_delay)
            {
                hiemi_error("clear fifo time out! \n");
                break;
            }
        }
    }while(0 != cnt);


}
void emi_is_clk_enabled(void)
{

}

void emi_reg_read(u32 reg, u32* value)
{
    //emi_enable_clk();
    *value = readl(g_emi_info.emi_ctrl.regbase + reg);
    //emi_disable_clk();
}
void emi_reg_write(u32 reg, u32 value)
{
    //emi_enable_clk();
    writel(value , g_emi_info.emi_ctrl.regbase + reg);
    //emi_disable_clk();
}
void emi_reg_show(u32 reg)
{
    u32 value = 0;

    //emi_enable_clk();
    emi_reg_read(reg,&value);
    //emi_disable_clk();
    hiemi_error("reg %d value is %d .\n",reg,value);
}


 s32 emi_config(emi_scdul_mode_e EMI_MODE)
{
    s32 iRet = EMI_OK;


    //emi_enable_clk();
    /*8-bit 和16-bit 区分*/

    if(EMI_TFT_9BIT == EMI_MODE)
    {
        hi_emi_set_reg(1, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBCR0_OFFSET, 3,4);
    }
    else
    {
        hi_emi_set_reg(0, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBCR0_OFFSET, 3,4);
    }
    hi_emi_set_reg(0, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBCR0_OFFSET, 5,5);

    switch(EMI_MODE)
    {
        case EMI_TFT_8BIT:
        case EMI_TFT_9BIT:
        {
            /* 设置读操作时序*/
            hi_emi_set_reg(24, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWST1R0_OFFSET, 0,9);
            hi_emi_set_reg(1, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWST2R0_OFFSET, 16,23);
            hi_emi_set_reg(2,(u32)( g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWSTOENR0_OFFSET, 0,3);
            hi_emi_set_reg(2, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWSTOENR0_OFFSET, 5,6);


            /* 设置写操作时序*/

            hi_emi_set_reg(5, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWST2R0_OFFSET, 0,9);
            hi_emi_set_reg(2, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWST2R0_OFFSET, 16,23);
            hi_emi_set_reg(1, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWSTOENR0_OFFSET, 0,3);
            hi_emi_set_reg(0, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWSTWENR0_OFFSET, 4,5);

        }
            break;
        case EMI_STN_8BIT_M6800:
        case EMI_STN_8BIT_I8080:
        {


            u32 u32UpDlyrSave   = 0;
            u32 u32DwnDlyrSave  = 0;
            u32 u32UpDlywSave   = 0;
            u32 u32DwnDlywSave  = 0;
            u32 u32UpDlyrTmp    = 0;
            u32 u32DwnDlyrTmp   = 0;
            u32 u32UpDlywTmp    = 0;
            u32 u32DwnDlywTmp   = 0;

            /* 保存参数 */
            u32UpDlyrSave   = 1;
            u32DwnDlyrSave  = 9;
            u32UpDlywSave   = 2;
            u32DwnDlywSave  = 7;

             /* 设置读操作时序*/
             hi_emi_set_reg(10, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWST1R0_OFFSET, 0,9);
            //set_hi_emi_smbwst1r0_wst1(10);/* 总线读访问等待时间*/

            u32UpDlyrTmp = (u32UpDlyrSave & 0x007f);
            hi_emi_set_reg(u32UpDlyrTmp, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBIDCYR0_OFFSET, 4,10);
            //set_hi_emi_smbidcyr0_updlyr_l(u32UpDlyrTmp);/* PREB信号拉高相对CS有效信号的延迟时间，低7位*/

            u32UpDlyrTmp = ((u32UpDlyrSave >> 7) & 0x0007);
            hi_emi_set_reg(u32UpDlyrTmp, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWST1R0_OFFSET, 10,12);
            //set_hi_emi_smbwst1r0_updlyr_h(u32UpDlyrTmp);/* PREB信号拉高相对CS有效信号的延迟时间，高3位*/

            u32DwnDlyrTmp = (u32DwnDlyrSave & 0x007f);
            hi_emi_set_reg(u32UpDlyrTmp, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBIDCYR0_OFFSET, 11,17);
            //set_hi_emi_smbidcyr0_dwndlyr_l(u32DwnDlyrTmp); /* PREB信号拉低相对CS有效信号的延迟时间，低7位*/


            u32DwnDlyrTmp = ((u32DwnDlyrSave >> 7) & 0x0007);
            hi_emi_set_reg(u32UpDlyrTmp, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWST1R0_OFFSET, 13,15);
            //set_hi_emi_smbwst1r0_dwndlyr_h(u32DwnDlyrTmp);/* PREB信号拉低相对CS有效信号的延迟时间，高3位*/


            /* 设置读操作时序*/
            hi_emi_set_reg(10, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWST2R0_OFFSET, 0,9);
            hi_emi_set_reg(3, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWST2R0_OFFSET, 16,23);

            //set_hi_emi_smbwst2r0_wst2(10);/* 总线写访问等待时间*/
            //set_hi_emi_smbwst2r0_wst3(3); /* 总线读写访问等待时间*/



            u32UpDlywTmp = (u32UpDlywSave & 0x007f);
            hi_emi_set_reg(u32UpDlywTmp, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBIDCYR0_OFFSET, 18,24);
            //set_hi_emi_smbidcyr0_updlyw_l(u32UpDlywTmp);/* PREB信号拉高相对CS有效信号的延迟时间，低7位*/

            u32UpDlywTmp = ((u32UpDlywSave >> 7) & 0x0007);
            hi_emi_set_reg(u32UpDlywTmp, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWST1R0_OFFSET, 16,18);
            //set_hi_emi_smbwst1r0_updlyw_h(u32UpDlywTmp);/* PREB信号拉高相对CS有效信号的延迟时间，高3位*/


            u32DwnDlywTmp = (u32DwnDlywSave & 0x007f);
            hi_emi_set_reg(u32DwnDlywTmp, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBIDCYR0_OFFSET, 25,31);
            //set_hi_emi_smbidcyr0_dwndlyw_l(u32DwnDlywTmp); /* PREB信号拉低相对CS有效信号的延迟时间，低7位*/

            u32DwnDlywTmp = ((u32DwnDlywSave >> 7) & 0x0007);
            hi_emi_set_reg(u32DwnDlywTmp, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWST1R0_OFFSET, 19,21);
            //set_hi_emi_smbwst1r0_dwndlyw_h(u32DwnDlywTmp);/* PREB信号拉低相对CS有效信号的延迟时间，高3位*/


            hi_emi_set_reg(0, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWSTWENR0_OFFSET, 0,3);
            hi_emi_set_reg(0, (u32)(g_emi_info.emi_ctrl.regbase), HI_EMI_SMBWSTWENR0_OFFSET, 4,5);
            //set_hi_emi_smbwstwenr0_wstwen(0); /* CS有效到写信号有效延迟*/
            //set_hi_emi_smbwstwenr0_wendly(0);/* 写信号无效到片选无效时间延迟*/


        }
            break;
        case EMI_LCD_8BIT:
        default:
            hiemi_error("EMI SCHDULING MODE IS NOT SUPPORTED. \n");
            iRet = EMI_ERROR;
    }
    //emi_disable_clk();
    return iRet;
}

void emiCmdOut8(u8 cmd)
{
    //emi_enable_clk();
    //writel((u32)cmd, (g_emi_membase + EMI_BUF_WRT_CMD));
    iowrite8(cmd, g_emi_info.emi_ctrl.membase + EMI_BUF_WRT_CMD);
    //emi_disable_clk();
}
void emiDataOut8(u8 data)
{
    //emi_enable_clk();
    //writel((u32)data, g_emi_membase + EMI_WRT_DATA_8BIT);
    iowrite8(data, g_emi_info.emi_ctrl.membase + g_emi_info.emi_wrt_data_8bit_offset);
    //emi_disable_clk();
}
u8 emiCmdIn8(void)
{
    u8 value = 0;

    //emi_enable_clk();
    //value= (u8)readl(HI_EMIMEM_REGBASE_ADDR_VIRT + EMI_BUF_WRT_CMD);
    value= ioread8(g_emi_info.emi_ctrl.membase + EMI_BUF_WRT_CMD);
    //emi_disable_clk();

    return value;
}
u8 emiDataIn8(void)
{
    u8 value = 0;

    //emi_enable_clk();
    //value= (u8)readl(HI_EMIMEM_REGBASE_ADDR_VIRT + EMI_WRT_DATA_8BIT);
    value= ioread8(g_emi_info.emi_ctrl.membase + g_emi_info.emi_wrt_data_8bit_offset);

    //emi_disable_clk();

    return value;
}
void emi_edma_isr(u32 para1,u32 int_flag)
{
    if(int_flag&EDMA_INT_DONE)
        g_emi_info.emi_st.edmatx_ok_cnt++;
    else
        g_emi_info.emi_st.edmatx_err_cnt++;

    osl_sem_up(&g_emi_info.emi_st.edmatx_semmux);
}
u32 emi_edma_node_size = (0x10000 - 1);/*链表节点大小*/
u32 set_emi_edma_node_size(u32 size)
{
    if(0!=size)
    {
        emi_edma_node_size = size;
    }
    return emi_edma_node_size;
}
s32 emi_edma_transfer(dma_addr_t src,unsigned long len)
{
    //edma_addr_t  edma_addr    = 0;
    s32         channel_id          = 0;
    u32      i            = 0;
    u32 ilinum;
    u32 ililast;
    u32 last_len;
    s32 ret = 0;
    s32 edma_ret = 0;
    struct edma_cb *pstNode   = NULL;
    struct edma_cb *psttemp   = NULL;
    struct edma_cb *FirstNode   = NULL;

    if(len == 0)
    {
        hiemi_error("balong SPI: balong_edma_transfer  failed，lens is wrong!\n");
        return EMI_ERROR;
    }

    channel_id = bsp_edma_channel_init(EDMA_EMI_TRANS,emi_edma_isr,0,EDMA_INT_DONE|EDMA_INT_TRANSFER_ERR|EDMA_INT_CONFIG_ERR|EDMA_INT_READ_ERR);
    if(channel_id<0)
    {
        hiemi_error("balong SPI: bsp_edma_channel_init failed!\n");
        return channel_id;
    }


    last_len = (len) % (emi_edma_node_size);/*最后一个节点传输长度*/

    if(last_len)/*最后一个节点非完整*/
    {
        ilinum = (len) / (emi_edma_node_size) + 1;/*链表节点数目*/
        ililast = last_len;/*最后一个节点传输长度*/
    }
    else
    {
        ilinum = (len) / (emi_edma_node_size);/*链表节点数目*/
        ililast = emi_edma_node_size;
    }

    /*创建链表*/
    pstNode = g_emi_info.edma_cb;
    FirstNode = pstNode;
    psttemp = pstNode;
    for (i = 0; i <= ilinum - 1; i++)
    {
        psttemp->lli = EDMA_SET_LLI(g_emi_info.edma_addr + (i+1) * sizeof(struct edma_cb), ((i < ilinum - 1)?0:1));
        psttemp->config = EDMA_SET_CONFIG( EDMA_EMI_TRANS,   EDMA_M2P,  EDMA_TRANS_WIDTH_8 , EDMA_BUR_LEN_1 ) & (~(EDMAC_TRANSFER_CONFIG_INT_TC_ENABLE));
        /* changer src_busrt_len to 16 */
        psttemp->config &= 0xF0FFFFFF;
		psttemp->config |= EDMA_BUR_LEN_16<<24;

        /*& ~bit30*/
        psttemp->src_addr = src + i * (emi_edma_node_size);

        psttemp->des_addr = (u32)(g_emi_info.emi_ctrl.rmembase+EMI_WRT_DATA_8BIT);

        psttemp->cnt0 = ((i < ilinum - 1)?(emi_edma_node_size) : ililast);
        psttemp->bindx = 0;
        psttemp->cindx = 0;
        psttemp->cnt1  = 0;
        psttemp++;
    }

    /*获取首节点寄存器地址*/
    psttemp = bsp_edma_channel_get_lli_addr((u32)channel_id);
    if (NULL == psttemp)
    {
        ret = EMI_ERROR;
        goto early_exit;

    }

        /*配置首节点寄存器*/
        psttemp->lli = FirstNode->lli;
        psttemp->config = FirstNode->config & 0xFFFFFFFE;
        psttemp->src_addr = FirstNode->src_addr;  /*物理地址*/
        psttemp->des_addr = FirstNode->des_addr;  /*物理地址*/
        psttemp->cnt0 = FirstNode->cnt0;
        psttemp->bindx = 0;
        psttemp->cindx = 0;
        psttemp->cnt1  = 0;

        //emi_enable_clk();

        /*启动EDMA传输后即返回，通过查询通道是否busy来确定传输是否完成*/
        if (bsp_edma_channel_lli_async_start((u32)channel_id))
        {
            ret = EMI_ERROR;
            goto early_exit;

        }
        /*改用中断方式,查询方式占用cpu太多*/
        osl_sem_down(&g_emi_info.emi_st.edmatx_semmux);

early_exit:
        //emi_disable_clk();
        edma_ret = bsp_edma_channel_free((u32)channel_id);
        if(edma_ret)
        {
            ret = EMI_ERROR;
            hiemi_error("failed to free edma channel!\n");
        }

        return ret;
}

static int emi_nv_init(void)
{
    DRV_MODULE_TEST_STRU support;
    unsigned int ret;

    ret = bsp_nvm_read(NV_ID_DRV_TEST_SUPPORT, (u8*)&support, sizeof(DRV_MODULE_TEST_STRU));
    if(ret){
        support.emi = 0;
        hiemi_error("lcd read nv fail!\n");
    }

    g_emi_info.emi_nv = support.emi;

    return (int)ret;
}

s32 emi_probe(struct platform_device *dev)
{
    struct resource *r;
    int ret = 0;
    unsigned int data = 0;
    struct device_node *device = NULL;
    char reg_name[EMI_DTS_REG_BUTT][32] = {"emi_srst_en", "emi_srst_dis", "ebi_normal_mode"};
    u32 reg_addr[4];
    int i = 0;
    const char *compatible_name = "hisilicon,emi_balong_app";

    /* 读取基地址并映射 */
	device = of_find_compatible_node(NULL, NULL, compatible_name);
	if (!device)  /* 找不到节点 */
	{
		dev_err(&dev->dev, "audio device node is null\n");
		goto err_free;
	}

    r = platform_get_resource(dev, IORESOURCE_MEM, 0);
    if (r == NULL) {
        dev_err(&dev->dev, "no memory resource defined\n");
        ret = -ENODEV;
        goto err_free;
    }

    r = request_mem_region(r->start, resource_size(r), dev->name);
    if (r == NULL) {
        dev_err(&dev->dev, "failed to request memory resource\n");
        ret = -EBUSY;
        goto err_free;
    }
    g_emi_info.emi_ctrl.regbase = ioremap(r->start, resource_size(r));
    if (g_emi_info.emi_ctrl.regbase == NULL) {
        dev_err(&dev->dev, "failed to ioremap() registers\n");
        ret = -ENODEV;
        goto err_free_mem;
    }

    ret = of_property_read_u32(device, "mem", &data);
	if (ret < 0)
	{
		dev_err(&dev->dev,"emi membase get error, ret %d\n", ret);
		goto err_free_mem;
	}
    g_emi_info.emi_ctrl.rmembase = (void*)data;
    g_emi_info.emi_ctrl.membase = ioremap(data, resource_size(r));
    if (g_emi_info.emi_ctrl.membase == NULL) {
        dev_err(&dev->dev, "failed to ioremap() registers\n");
        ret = -ENODEV;
        goto err_free_mem;
    }


    for(i = 0; i < EMI_DTS_REG_BUTT; i++)
    {
        of_property_read_u32_array(device, reg_name[i], reg_addr, EMI_ADDR_BUTT);
        g_emi_info.emi_sysctrl[i].base_addr = (u32)bsp_sysctrl_addr_get((void *)reg_addr[EMI_BASE_ADDR]);
        g_emi_info.emi_sysctrl[i].offset = reg_addr[EMI_OFFSET];
        g_emi_info.emi_sysctrl[i].start_bit = reg_addr[EMI_START_BIT];
        g_emi_info.emi_sysctrl[i].end_bit = reg_addr[EMI_END_BIT];
    }

    emi_clk = clk_get(NULL,"emi_clk");
    ret = clk_prepare(emi_clk);
    if (ret) {
		hiemi_error("Clock prepare failed\n");
		goto early_exit;
	}

    emi_enable_clk();

    hi_emi_set_reg(1, g_emi_info.emi_sysctrl[EMI_DTS_EMI_SRST_EN].base_addr, g_emi_info.emi_sysctrl[EMI_DTS_EMI_SRST_EN].offset, g_emi_info.emi_sysctrl[EMI_DTS_EMI_SRST_EN].start_bit, g_emi_info.emi_sysctrl[EMI_DTS_EMI_SRST_EN].end_bit);
    hi_emi_set_reg(1, g_emi_info.emi_sysctrl[EMI_DTS_EMI_SRST_DIS].base_addr, g_emi_info.emi_sysctrl[EMI_DTS_EMI_SRST_DIS].offset, g_emi_info.emi_sysctrl[EMI_DTS_EMI_SRST_DIS].start_bit, g_emi_info.emi_sysctrl[EMI_DTS_EMI_SRST_DIS].end_bit);
    /*设置为normal模式*/
    hi_emi_set_reg(1, g_emi_info.emi_sysctrl[EMI_DTS_EBI_NORMAL_MODE].base_addr, g_emi_info.emi_sysctrl[EMI_DTS_EBI_NORMAL_MODE].offset, g_emi_info.emi_sysctrl[EMI_DTS_EBI_NORMAL_MODE].start_bit, g_emi_info.emi_sysctrl[EMI_DTS_EBI_NORMAL_MODE].end_bit);

    g_emi_info.edma_cb = (struct edma_cb *)dma_alloc_coherent(NULL,(3 * sizeof(struct edma_cb)),
                                                             &g_emi_info.edma_addr, GFP_DMA|__GFP_WAIT);
    if(NULL == g_emi_info.edma_cb)
    {
        ret = EMI_ERROR;
        goto err_free_mem;
    }

    /* 数据信号*/
    /* 不用设置*/

    hi_emi_set_reg(0,(u32)(g_emi_info.emi_ctrl.regbase),HI_EMI_SMBCR0_OFFSET,1,1);
    //set_hi_emi_smbcr0_cspol(0);/*片选信号低有效*/

    //dma_bypass = 0;dma_bypass_mask = 22;
    //dma_en = 1;dma_en_mask =21;
    //gateclock_en = 1;gateclock_en_mask = 23;
    hi_emi_set_reg(0,(u32)(g_emi_info.emi_ctrl.regbase),HI_EMI_SMBCR0_OFFSET,22,22);
    hi_emi_set_reg(1,(u32)(g_emi_info.emi_ctrl.regbase),HI_EMI_SMBCR0_OFFSET,21,21);
    hi_emi_set_reg(1,(u32)(g_emi_info.emi_ctrl.regbase),HI_EMI_SMBCR0_OFFSET,23,23);
    g_emi_info.emi_debug.emi_trans_way = 0;
    //hi_emi_support_dma();
    //hi_emi_dma_enable();    /* dma enable, just for v711 */
    //hi_emi_gateclock_enable();

    //emi_disable_clk();

    /*初始化emiedma传输信号量*/
    osl_sem_init(SEM_EMPTY, &g_emi_info.emi_st.edmatx_semmux);
    g_emi_info.emi_st.edmatx_ok_cnt = 0;
    g_emi_info.emi_st.edmatx_err_cnt = 0;

    g_emi_info.emi_wrt_data_8bit_offset = EMI_WRT_DATA_8BIT;
    g_emi_info.emi_wrt_data_16bit_offset = EMI_WRT_DATA_16BIT;

    /*g_emi_reg_bak = (struct emi_reg_t*)malloc(sizeof(struct emi_reg_t));*/
    g_emi_reg_bak = kzalloc(sizeof(emi_reg_t), GFP_KERNEL);
    if (!g_emi_reg_bak)
        return -ENOMEM;
    else
        return 0;
early_exit:
    dma_free_coherent(NULL,(3 * sizeof(struct edma_cb)),
                              (void*)g_emi_info.edma_cb, g_emi_info.edma_addr);
err_free_mem:
    release_mem_region(r->start, resource_size(r));
err_free:
    return ret;

}
s32  emi_remove(struct platform_device *dev)
{
    clk_disable(emi_clk);
    clk_put(emi_clk);
    emi_clk = NULL;

    kfree(g_emi_reg_bak);
    g_emi_reg_bak = NULL;

    return 0;
}
void emi_reg_backup(void)
{
    emi_reg_read(HI_EMI_SMBIDCYR0_OFFSET, &(g_emi_reg_bak->smbidcyr0));
    emi_reg_read(HI_EMI_SMBWST1R0_OFFSET, &(g_emi_reg_bak->smbwst1r0));
    emi_reg_read(HI_EMI_SMBWST2R0_OFFSET, &(g_emi_reg_bak->smbwst2r0));
    emi_reg_read(HI_EMI_SMBWSTOENR0_OFFSET, &(g_emi_reg_bak->smbwstoenr0));
    emi_reg_read(HI_EMI_SMBWSTWENR0_OFFSET, &(g_emi_reg_bak->smbwstwenr0));
    emi_reg_read(HI_EMI_SMBCR0_OFFSET, &(g_emi_reg_bak->smbcr0));
    emi_reg_read(HI_EMI_SMBSR0_OFFSET, &(g_emi_reg_bak->smbsr0));
}
void emi_reg_restore(void)
{
    emi_reg_write(HI_EMI_SMBIDCYR0_OFFSET , g_emi_reg_bak->smbidcyr0);
    emi_reg_write(HI_EMI_SMBWST1R0_OFFSET , g_emi_reg_bak->smbwst1r0);
    emi_reg_write(HI_EMI_SMBWST2R0_OFFSET , g_emi_reg_bak->smbwst2r0);
    emi_reg_write(HI_EMI_SMBWSTOENR0_OFFSET , g_emi_reg_bak->smbwstoenr0);
    emi_reg_write(HI_EMI_SMBWSTWENR0_OFFSET , g_emi_reg_bak->smbwstwenr0);
    emi_reg_write(HI_EMI_SMBCR0_OFFSET , g_emi_reg_bak->smbcr0);
   /* emi_reg_write(HI_EMI_SMBSR0_OFFSET , g_emi_reg_bak->smbsr0);*/
}
void emi_reg_bak_info(void)
{
    hiemi_error("g_emi_reg_bak->smbidcyr0 is %d \n", g_emi_reg_bak->smbidcyr0 );
    hiemi_error("g_emi_reg_bak->smbwst1r0 is %d \n", g_emi_reg_bak->smbwst1r0 );
    hiemi_error("g_emi_reg_bak->smbwst2r0 is %d \n", g_emi_reg_bak->smbwst2r0 );
    hiemi_error("g_emi_reg_bak->smbwstoenr0 is %d \n", g_emi_reg_bak->smbwstoenr0);
    hiemi_error("g_emi_reg_bak->smbwstwenr0 is %d \n", g_emi_reg_bak->smbwstwenr0);
    hiemi_error("g_emi_reg_bak->smbcr0 is %d \n", g_emi_reg_bak->smbcr0);
    hiemi_error("g_emi_reg_bak->smbsr0 is %d \n", g_emi_reg_bak->smbsr0);
}
int emi_resume (struct device *dev)
{
    emi_enable_clk();
    emi_reg_restore();
    //emi_disable_clk();
    hiemi_trace("emi_resume ok!\n");

    return 0;
}

int emi_suspend (struct device *dev)
{
    //emi_enable_clk();
    emi_reg_backup();
    emi_disable_clk();
    hiemi_trace("emi_suspend ok!\n");

    return 0;
}
static const struct dev_pm_ops emi_pm_ops ={
    .suspend = emi_suspend,
    .resume = emi_resume,
};

#define BALONG_DEV_PM_OPS (&emi_pm_ops)

static const struct of_device_id balong_emi_of_match[] = {
	{ .compatible = "hisilicon,emi_balong_app", },
	{},
};
MODULE_DEVICE_TABLE(of, balong_emi_of_match);


static struct platform_driver balong_emi_driver = {
    .probe = emi_probe,
    .remove = emi_remove,
    .driver = {
        .name = "balong_emi",
        .owner  = THIS_MODULE,
        .pm     = BALONG_DEV_PM_OPS,
        .of_match_table = of_match_ptr(balong_emi_of_match),
    },
};


static int __init emi_init(void)
{
    int ret = 0;

    /* 读取nv，如果nv中对应bit为0，则不支持emi，不注册，直接返回 */
    if (emi_nv_init() || !g_emi_info.emi_nv)
    {
        hiemi_error("emi is not support.\n");
        return 0;
    }

    ret = platform_driver_register(&balong_emi_driver);
    if (ret)
    {
        hiemi_error("Platform ipc deriver register is failed!\n");
        //platform_device_unregister(&balong_emi_device);
        return ret;
    }
    return ret;
}
static void __exit emi_exit(void)
{
    platform_driver_unregister(&balong_emi_driver);
    //platform_device_unregister(&balong_emi_device);
}

arch_initcall(emi_init);
module_exit(emi_exit);

