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
/*lint --e{537,958}*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <drv_comm.h>
#include <asm/uaccess.h>
#include <osl_bio.h>
#include <osl_malloc.h>
#include <bsp_ipc.h>
#include <bsp_memmap.h>
#include <bsp_dump.h>
#include <drv_nv_id.h>
#include <drv_nv_def.h>
#include <bsp_nvim.h>
#include <bsp_icc.h>
#include <bsp_pmu.h>
#include <bsp_shared_ddr.h>
#include <osl_thread.h>
#include <osl_sem.h>
#include <osl_spinlock.h>
#include <osl_malloc.h>

#include <pmic_volt.h>
#include <pmic_irq.h>
#include "pmic_inner.h"
#include "pmic_exc.h"

struct pmic_exc *g_pmic_excinfo;/*中断全局结构体*/
extern struct pmic_irq *g_pmic_irqinfo;
extern struct pmic_volt_info g_pmic_voltinfo;


//#define pmic_log_save(fmt, ...)   print2file(PMU_OM_LOG, fmt,...);
//#define pmic_log_save(fmt, ...)   pmic_print_error(fmt,...)
#define pmic_log_save(fmt, ...)   (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_PMU, fmt,##__VA_ARGS__))
/*****************************************************************************
 函 数 名  : pmic_om_wk_handler
 功能描述  : 将非下电状态记录寄存器写入log文件，并判断此次启动是否由PMU引起的重启
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 备注说明  : 无
*****************************************************************************/
void pmic_om_wk_handler(void)
{
    int i,bit = 0;
    struct pmic_record* pmic_record = &(g_pmic_excinfo->record);
    struct record_info* record_info = NULL;
    u32 width = g_pmic_excinfo->data_width;
    struct pmic_irq_reg *irq_reg = &(g_pmic_irqinfo->irq_reg);
    u32 irq_data = 0;
    u32 rec_data = 0;
    unsigned long prec_data = 0;

    /*save and explain irq for last time*/
	pmic_print_error("irq_reg&value:");
    for(i = 0;i < irq_reg->irqarrays;i++)
    {
        irq_data = readl((void*)(SHM_MEM_PMU_NPREG_ADDR + i));
        irq_data &= 0xff;
        pmic_log_save("0x%x=0x%x ",irq_reg->irq[i],irq_data);
    }
	pmic_log_save("\n");

    /*save and explain ocp & record for last time */
	pmic_print_error("record old reg&value:");
    for(i = 0; i < pmic_record->record_num; i++)
    {
        record_info = &(pmic_record->record_info[i]);
        rec_data = readl((void*)(SHM_MEM_PMU_NPREG_ADDR + irq_reg->irqarrays + i));
        rec_data &= 0xff;
        pmic_log_save("0x%x=0x%x ",record_info->record_addr,rec_data);
        prec_data = (unsigned long)rec_data;
        for_each_set_bit(bit, &prec_data, width)
        {
            if((rec_data)&(0x1<<bit))
            {
                if((record_info->inacceptable)&(0x1<<bit))
                    pmic_log_save("event:%s(abnormal)!\n",record_info->record_name[bit]);
                else
                    pmic_log_save("event:%s(normal)!\n",record_info->record_name[bit]);
            }
        }
    }
	pmic_log_save("\n");

    /*save and explain record for this time */
	pmic_print_error("record new reg&value:");
    for(i = 0; i < pmic_record->record_num; i++)
    {
        record_info = &(pmic_record->record_info[i]);
        pmic_log_save("0x%x=0x%x ",record_info->record_addr,record_info->record_data);
        prec_data = (unsigned long)(record_info->record_data);
        for_each_set_bit(bit, &prec_data, width)
        {
            if((record_info->record_data)&(0x1<<bit))
            {
                if((record_info->inacceptable)&(0x1<<bit))
                    pmic_log_save("event:%s(abnormal)!\n",record_info->record_name[bit]);
                else
                    pmic_log_save("event:%s(normal)!\n",record_info->record_name[bit]);
            }
        }
    }
	pmic_log_save("\n");
}

/*****************************************************************************
 函 数 名  : pmic_boot_om_log_save
 功能描述  : 将非下电状态记录寄存器保存到全局变量
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 备注说明  : 将record的值在fastboot中已经读到共享内存中
*****************************************************************************/
void pmic_record_save(void)
{
    unsigned long timeout = 0;
    int i = 0;
    struct pmic_record* pmic_record = &(g_pmic_excinfo->record);
    struct record_info* record_info = NULL;

    /*read record and clear reg*/
    for(i = 0; i < pmic_record->record_num; i++)
    {
        record_info = &(pmic_record->record_info[i]);
        pmic_reg_read(record_info->record_addr,(u32*)&(record_info->record_data));
        pmic_reg_write(record_info->record_addr,record_info->record_data);
    }

    /* 记录重启后，pmu的最初状态，分析重启原因,记录日志时间长，采用工作队列完成 */
    g_pmic_excinfo->om_wq = create_singlethread_workqueue("pmic_exc_om");
    if (!g_pmic_excinfo->om_wq)
    {
        pmic_print_error("%s: create_singlethread_workqueue fail\n", __FUNCTION__);
        return;
    }

    INIT_DELAYED_WORK(&g_pmic_excinfo->om_wk, (void *)pmic_om_wk_handler);
    queue_delayed_work(g_pmic_excinfo->om_wq, &g_pmic_excinfo->om_wk, timeout);

    return;
}

/*****************************************************************************
 函 数 名  : pmic_otp_threshold_set
 功能描述  : 系统启动时设置过温警告阈值
 输入参数  : @threshold 要设置的温度(摄氏度)
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void pmic_otp_threshold_set(void)
{
    pmic_excflags_t exc_flags = 0;
    struct pmic_otp *pmic_otp = &g_pmic_excinfo->otp;

    spin_lock_irqsave(&g_pmic_excinfo->lock,exc_flags);
    pmic_reg_write_mask(pmic_otp->otp_base, pmic_otp->otp_value,pmic_otp->otp_mask);
    spin_unlock_irqrestore(&g_pmic_excinfo->lock,exc_flags);

    return;
}

/*****************************************************************************
 函 数 名  : bsp_hi6559_uvp_threshold_set
 功能描述  : 系统启动时设置欠压警告阈值
 输入参数  : @threshold 要设置的电压(mV)
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void pmic_uvp_threshold_set(void)
{
    pmic_excflags_t exc_flags = 0;
    struct pmic_uvp *pmic_uvp = &g_pmic_excinfo->uvp;

    spin_lock_irqsave(&g_pmic_excinfo->lock,exc_flags);
    pmic_reg_write_mask(pmic_uvp->uvp_base, pmic_uvp->uvp_value,pmic_uvp->uvp_mask);
    spin_unlock_irqrestore(&g_pmic_excinfo->lock,exc_flags);
}

/*ocp register*/
int pmic_ocp_register(int volt_id,PMU_OCP_FUNCPTR func)
{
    struct pmic_volt_ocp *ocp = NULL;

    ocp = (struct pmic_volt_ocp *)osl_malloc(sizeof(struct pmic_volt_ocp));
    if (NULL == ocp)
    {
        pmic_print_error("ocp malloc failed ,err %d\n", ocp);
        return -1;
    }

    INIT_LIST_HEAD(&ocp->list);
    ocp->func = func;
    list_add(&ocp->list, &g_pmic_voltinfo.volt_ctrl[volt_id].ocp_list);
	/*coverity[leaked_storage] */
    return 0;
}
void pmic_modem_ocp_handler(int volt_id)
{
    int len;
    u32 chan_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_PMU_OCP;

    len = bsp_icc_send(ICC_CPU_MODEM, chan_id, (unsigned char*)&volt_id, sizeof(int));
    if(len == 0)
    {
        pmic_print_error("send len(%x) != expected len(%x).\n", len, sizeof(int));
        return ;
    }
}

void pmic_modem_ocp_init(struct platform_device *pdev)
{
    struct device_node *root = NULL;
    struct device_node *child_node = NULL;
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    int volt_id = 0;
    int iret = 0;

    root = of_find_compatible_node(np, NULL, "hisilicon,pmic_ocp_modem");
    if(!root)
    {
        pmic_print_error("get pmic dts node failed!\n");
        return;
    }
    for_each_child_of_node(root, child_node)
    {
        iret = of_property_read_u32(child_node, "id",(u32 *)&volt_id);
        if(!iret)
            (void)pmic_ocp_register(volt_id,pmic_modem_ocp_handler);
    }
}

/*****************************************************************************
 函 数 名  : pmic_exc_ocp_scp_handle
 功能描述  : 过流处理函数
 输入参数  : @para 参数指针
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void pmic_ocp_handle(void* para)
{
    u64 *pmu_ocp_flag = (u64 *)SHM_PMU_OCP_INFO_ADDR;   /* 过流，需要关闭的过流源 */
    u8 i,bit = 0;
    struct pmic_ocp* pmic_ocp = &(g_pmic_excinfo->ocp);
    struct ocp_info* ocp_info = NULL;
    struct pmic_volt_ocp *ocp_handle = NULL;
    unsigned long pocp_data = 0;

    /*lint --e{690,831}*/
    para = para;
    pmic_print_error("******* pmic_current_overflow! **********\n");

    for(i = 0; i < pmic_ocp->ocp_num; i++)
    {
        ocp_info = &(pmic_ocp->ocp_info[i]);

        pmic_reg_read(ocp_info->ocp_addr, (u32*)&(ocp_info->ocp_data));
        /*disable volt*/
        pocp_data = (unsigned long)(ocp_info->ocp_data);
        for_each_set_bit(bit, &pocp_data, 8)
        {
            /*set ocp flag*/
            *pmu_ocp_flag |= ((u32)0x1 << ocp_info->ocp_id[bit]);
            pmic_print_error("volt %d:%s overflow,will be closed!\n", ocp_info->ocp_id[bit],ocp_info->ocp_name[bit]);
            pmic_volt_disable(ocp_info->ocp_id[bit]);
            /*clear irq*/
            pmic_reg_write((u32)(ocp_info->ocp_addr),((u32)0x1 << bit));
            /*handle customer register ops*/
            list_for_each_entry(ocp_handle, &g_pmic_voltinfo.volt_ctrl[ocp_info->ocp_id[bit]].ocp_list, list)
            {
                if(ocp_handle->func)
                {
                    (void) (*(ocp_handle->func))(ocp_info->ocp_id[bit]);
                }
            }
         }
    }

    return;
}

/*****************************************************************************
 函 数 名  : pmic_exc_otp_handle
 功能描述  : 过温处理函数
 输入参数  : @para 参数指针
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void pmic_otp_handle(void* para)
{
    int index = 0;
    struct pmic_pro* exc_pro = &g_pmic_excinfo->pro;

    para = para;

    pmic_print_error("pmic:temperature overflow !\n");
    /* 记录om log */
    pmic_log_save("pmic:temperature overflow!\n");

    /*根据dts配置，决定是否关闭非核心电源,后续增加产品形态相关特殊处理*/
    for(index = 0; index < exc_pro->otp_off_num; index++)
    {
        pmic_print_error("volt id %d will be closed !\n",exc_pro->otp_off_arry[index]);
        pmic_volt_disable(exc_pro->otp_off_arry[index]);
    }

    if(exc_pro->otp_rst)
    {
        pmic_print_error("system will be restart!\n");
        system_error(DRV_ERRNO_PMU_OVER_TEMP, PMU_STATE_OVER_TEMP, 0, NULL, 0);
    }

    return;
}

int pmic_otp_init(struct platform_device *pdev)
{
    struct device_node *root = NULL;
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    struct pmic_otp* pmic_otp = &g_pmic_excinfo->otp;
    pmic_excflags_t exc_flags = 0;
    int iret = 0;

    root = of_find_compatible_node(np, NULL, "hisilicon,pmic_exc_otp");
    if (!root)
    {
        dev_err(dev, "[%s]no hisilicon,pmic_exc_ap root node\n", __func__);
        return -ENODEV;
    }

    iret = of_property_read_u32_array(root, "otp_info", (u32 *)pmic_otp,sizeof(struct pmic_otp)/sizeof(u32));
    if (iret)
    {
        pmic_print_error("read otp info failed,err id %d.\n",iret);
        return iret;
    }

    /* 设置过温预警阈值 */
    spin_lock_irqsave(&g_pmic_excinfo->lock,exc_flags);
    pmic_reg_write_mask(pmic_otp->otp_base, pmic_otp->otp_value << pmic_otp->otp_offset,pmic_otp->otp_mask);
    spin_unlock_irqrestore(&g_pmic_excinfo->lock,exc_flags);

    iret = pmic_irq_callback_register(pmic_otp->otp_irq, pmic_otp_handle, NULL);
    if (iret) {
        pmic_print_error("pmic irq register otp interrupt failed,err id %d!\n",iret);
        return iret;
    }

    return iret;
}
int pmic_ocp_init(struct platform_device *pdev)
{
    struct device_node *root = NULL;
    struct device_node *child_node = NULL;
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    u32 reg_value = 0;
    int iret = 0;
    int index = 0,bit = 0;
    struct pmic_ocp *pmic_ocp = &g_pmic_excinfo->ocp;
    struct ocp_info *ocp_info = NULL;
    u32 data_width = g_pmic_excinfo->data_width;

    root = of_find_compatible_node(np, NULL, "hisilicon,pmic_exc_ocp");
    if (!root)
    {
        dev_err(dev, "[%s]no hisilicon,pmic_exc_ocp root node\n", __func__);
        return -ENODEV;
    }

    iret = of_property_read_u32_index(root, "interrupts",0,(u32 *)&pmic_ocp->ocp_irq);
    if(iret)
    {
        pmic_print_error("get interrupts from dts failed.\n");
        return -1;
    }
    if(pmic_ocp->ocp_irq != 8)
    {
        pmic_print_error("ocp irq not right,set to 8!\n");
        pmic_ocp->ocp_irq = 8;
    }
    pmic_ocp->ocp_num = of_get_child_count(root);
    pmic_ocp->ocp_info = devm_kzalloc(dev, sizeof(struct ocp_info)*(pmic_ocp->ocp_num), GFP_KERNEL);
    if (!pmic_ocp->ocp_info)
    {
        pmic_print_error("kzalloc ocp_info buffer failed.\n");
        return -ENOMEM;
    }

    for_each_child_of_node(root, child_node)
    {
        ocp_info = &(pmic_ocp->ocp_info[index]);
        index++;

        ocp_info->ocp_id = (u32*)devm_kzalloc(dev, data_width*sizeof(u32), GFP_KERNEL);
        if (NULL == ocp_info->ocp_id) {
            pmic_print_error("devm_kzalloc ocp_id error.\n");
            return -ENOMEM;
        }
        ocp_info->ocp_name = (char **)devm_kzalloc(dev, data_width*sizeof(char *), GFP_KERNEL);
        if (NULL == ocp_info->ocp_name) {
            pmic_print_error("devm_kzalloc ocp_name error.\n");
            return -ENOMEM;
        }

        iret = of_property_read_u32(child_node, "ocp_addr", (u32 *)&ocp_info->ocp_addr);

        for (bit = 0; bit < data_width; bit++)
        {
            iret |= of_property_read_u32_index(child_node, "ocp_id", bit, &ocp_info->ocp_id[bit]);
            iret |= of_property_read_string_index(child_node, "ocp_name",
            bit, (const char **)&ocp_info->ocp_name[bit]);
        }
        if (iret)
        {
            pmic_print_error("get ocp_addr or ocp_id/ocp_name attribute failed.\n");
            return -ENODEV;
        }
    }

    /*清除短路过流中断寄存器，这样过流中断位才能消除*/
    for(index = 0; index < pmic_ocp->ocp_num; index++)
    {
        pmic_reg_read((pmic_ocp->ocp_info[index].ocp_addr),&reg_value);
        pmic_reg_write((pmic_ocp->ocp_info[index].ocp_addr), reg_value);
    }

    iret = pmic_irq_callback_register(pmic_ocp->ocp_irq, pmic_ocp_handle, NULL);
    if (iret)
    {
        pmic_print_error("irq register ocp interrupt failed!\n");
        return iret;
    }

    pmic_modem_ocp_init(pdev);

    return iret;
}

int pmic_record_init(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    struct device_node *dev_node,*child_node = NULL;
    struct pmic_record *pmic_record = &g_pmic_excinfo->record;
    struct record_info *record_info = NULL;
    int ret = 0;
    int index = 0,bit = 0;
    u32 data_width = g_pmic_excinfo->data_width;

    dev_node = of_find_compatible_node(np,NULL,"hisilicon,pmic_exc_record");
    if(!dev_node)
    {
        pmic_print_error("get pmic record dts node failed!\n");
        return BSP_ERROR;
    }

    pmic_record->record_num = of_get_child_count(dev_node);
    pmic_record->record_info = (struct record_info *)devm_kzalloc(dev, sizeof(struct record_info)*(pmic_record->record_num), GFP_KERNEL);
    if (pmic_record->record_info == NULL)
    {
        pmic_print_error("kzalloc record_info buffer failed.\n");
        return -ENOMEM;
    }

    for_each_child_of_node(dev_node, child_node)
    {
        record_info = &(pmic_record->record_info[index]);
        index++;
        ret = of_property_read_u32(child_node, "record_addr", (u32 *)&record_info->record_addr);
        ret |= of_property_read_u32(child_node, "inacceptable_event", (u32 *)&record_info->inacceptable);
        if (ret)
        {
            pmic_print_error("get record_addr or inacceptable_event attribute failed.\n");
            goto out;
        }
        record_info->record_name = (char **)devm_kzalloc(dev, data_width*sizeof(char *), GFP_KERNEL);
        if (NULL == record_info->record_name)
        {
            pmic_print_error("devm_kzalloc record_name error.\n");
            goto out;
        }
        for (bit = 0; bit < data_width; bit++)
        {
            ret |= of_property_read_string_index(child_node, "record_name",
                bit, (const char **)&record_info->record_name[bit]);
        }
    }
    /*save log*/
    /* 写start flag到log文件 */
    pmic_log_save("system start....\n");

    /* 保存非下电寄存器 */
    pmic_record_save();
    
    return ret;
out:
    kfree(pmic_record->record_info);
    return ret;
}
int pmic_pro_init(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *dev_node = NULL;
    struct pmic_pro *pmic_pro = &g_pmic_excinfo->pro;
    int otp_size = 0;

    dev_node = of_find_compatible_node(NULL,NULL,"hisilicon,pmic_exc_pro");
    if(!dev_node)
    {
        pmic_print_error("get pmic exc pro dts node failed!\n");
        return BSP_ERROR;
    }

    /*read dts*/
    if(of_property_read_u32(dev_node, "otp_rst", &pmic_pro->otp_rst))
        pmic_pro->otp_rst = 1;
    if(of_property_read_u32(dev_node, "otp_off_num", &pmic_pro->otp_off_num))
        goto otp_err;

    otp_size = (pmic_pro->otp_off_num)*sizeof(u32);
    pmic_pro->otp_off_arry = (u32 *)devm_kzalloc(dev, otp_size, GFP_KERNEL);
    if (NULL == pmic_pro->otp_off_arry) {
        pmic_print_error("devm_kzalloc otp_off_arry error.\n");
        goto otp_err;
    }
    if(of_property_read_u32_array(dev_node, "otp_off_arry", pmic_pro->otp_off_arry, pmic_pro->otp_off_num))
        goto otp_err;

    return 0;

otp_err:
    pmic_pro->otp_off_num = 0;
    return -1;
}

/*****************************************************************************
 函 数 名  : bsp_hi6559_exc_init
 功能描述  : 异常处理模块初始化函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
int pmic_exc_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *root = dev->of_node;
    int iret = 0;

    g_pmic_excinfo = kzalloc(sizeof(struct pmic_exc), GFP_KERNEL);
    if (g_pmic_excinfo == NULL) {
        pmic_print_error("pmic exc kzalloc is failed,please check!\n");
        return -1;
    }
    spin_lock_init(&g_pmic_excinfo->lock);
    /*read dts*/
    iret = of_property_read_u32(root, "data_width", (u32 *)&g_pmic_excinfo->data_width);
    if (iret)
    {
        pmic_print_error("read data_width from dts failed,err id %d.\n",iret);
        kfree(g_pmic_excinfo);
        return iret;
    }

    pmic_pro_init(pdev);
    pmic_otp_init(pdev);
    pmic_ocp_init(pdev);
    pmic_record_init(pdev);

    pmic_print_error("pmic exc init ok!\n");

    return iret;
}
static  int pmic_exc_remove(struct platform_device *pdev)
{
    struct pmic_irq_data *pmic_irq_data_pt;
    /*get pmic_irq_data*/
    pmic_irq_data_pt = platform_get_drvdata(pdev);
    if (NULL == pmic_irq_data_pt) {
        pr_err("%s %d platform_get_drvdata NULL\n", __func__, __LINE__);
        return -1;
    }

    /*release memory*/
    kfree(pmic_irq_data_pt);

    return 0;
}
static const struct of_device_id pmic_exc_of_match[] = {
    { .compatible = "hisilicon,pmic_exc_app"},
    {},
};
MODULE_DEVICE_TABLE(of, pmic_exc_of_match);

static struct platform_driver pmic_exc_driver = {
    .probe = pmic_exc_probe,
    .remove = pmic_exc_remove,
    .driver     = {
        .name           = "pmic_exc",
        .of_match_table = of_match_ptr(pmic_exc_of_match),
    },
};
static int __init pmic_exc_init(void)
{
    int ret = 0;

    ret = platform_driver_register(&pmic_exc_driver);

    if (ret)
        printk("Failed to register pmic_exc driver or device: %d\n", ret);

    return ret;
}

static void __exit pmic_exc_exit(void)
{
    platform_driver_unregister(&pmic_exc_driver);
}

module_init(pmic_exc_init);
module_exit(pmic_exc_exit);

EXPORT_SYMBOL_GPL(pmic_otp_threshold_set);
EXPORT_SYMBOL_GPL(pmic_uvp_threshold_set);
EXPORT_SYMBOL_GPL(pmic_otp_handle);
EXPORT_SYMBOL_GPL(pmic_ocp_handle);

