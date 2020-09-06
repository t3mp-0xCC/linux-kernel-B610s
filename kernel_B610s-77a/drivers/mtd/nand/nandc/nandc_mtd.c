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

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/mtd/mtd.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/mtd/nand.h>
#include <bsp_shared_ddr.h>
#include <bsp_nandc.h>
#include <nandc_ctrl.h>
#include <nandc_mtd.h>
#include "../../mtdcore.h"
#if (FEATURE_ON == MBB_DLOAD)
#include <linux/mtd/partitions.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <bsp_sram.h>
#include "dload_mtd_adapter.h"
#define     DLOAD_MTDFLUSH_COUNT_MAX        (4)
#endif

#if (FEATURE_ON == MBB_HWTEST)
extern void init_hardware_flash_info(unsigned int codewore_size, unsigned int spare_size, unsigned int ecc_correct_num);
#endif
/*lint --e{705,830,752,808,718,745,830,737,702,762,701,533}*/
extern struct nandc_bit_cluster nandc6_bit_cluster;

static int nandc_mtd_remove(struct platform_device *dev);
u32 ptable_parse_mtd_partitions(struct mtd_partition** mtd_parts, u32 *nr_parts);

struct nand_ecclayout nandc6_oob32_layout =
{
    STU_SET(.eccbytes)  0 ,
    STU_SET(.eccpos )   {0} ,
    STU_SET(.oobavail)  19 ,
    STU_SET(.oobfree)
    {
        {4, 19},
    }
};

/*nand的参数在fastboot中解析完成并且通过共享内存的方式传送到A核*/
int nand_spec_info_resume(struct mtd_info *mtd, struct nand_spec_shared_mem * spec_shmem)
{
    struct nand_chip *chip = NULL;
    struct nandc_host *host = NULL;

	if(!mtd || !spec_shmem)
	{
		return -EINVAL;
	}

    if(NAND_SPEC_ALREADY_SAVED != spec_shmem->flag)
	{
	    printk("ERROR: invalid flag 0x%x\n", spec_shmem->flag);
		return -EINVAL;
	}

    chip = (struct nand_chip *)mtd->priv;
    host = (struct nandc_host *)chip->priv;

	mtd->writesize = (uint32_t)spec_shmem->page_size;
	mtd->oobsize   = (uint32_t)spec_shmem->spare_size;
	mtd->erasesize = (uint32_t)spec_shmem->block_size;
	chip->chipsize = (uint64_t)spec_shmem->chip_size;
    host->ecctype_onfi  = (uint32_t)spec_shmem->ecc_type;
    printk("ecctype_onfi %x,writesize %x,oobsize %x,erasesize %x,chipsize %x spec_shmem->ecc_type %x\n",
        (u32)host->ecctype_onfi,(u32)mtd->writesize,(u32)mtd->oobsize,(u32)mtd->erasesize,(u32)chip->chipsize,(u32)spec_shmem->ecc_type);
	return 0;
}

/*选择芯片，时钟使能*/
static void nandc_mtd_select_chip(struct mtd_info *mtd, int chipselect)
{
    struct nand_chip *chip = mtd->priv;
    struct nandc_host *host = chip->priv;
    /*不使能时钟*/
    if (chipselect == -1)
    {
        clk_disable_unprepare(host->clk);
    }
    /*使能时钟*/
    else
    {
        if (chipselect >= NANDC_MAX_CHIPS)
        {
            NANDC_TRACE(NFCDBGLVL(ERRO), ("invalid chipselect: %d\n", chipselect));
        }
        /* coverity[check_return] */
        /* coverity[unchecked_value] */
        clk_prepare_enable(host->clk);
        host->chipselect = (u32)chipselect;
    }
}

/*读一个字节*/
static uint8_t nandc_mtd_read_byte(struct mtd_info *mtd)
{
    struct nand_chip *chip = mtd->priv;
    struct nandc_host *host = chip->priv;
    unsigned char value = 0;
    if (host->command == NAND_CMD_STATUS)
    {
        value = readb(chip->IO_ADDR_R);
    }
    else
    {
        value = readb(chip->IO_ADDR_R + host->column + host->offset);
        host->offset++;
    }
    return value;
}
/*读两个字节*/
static u16 nandc_mtd_read_word(struct mtd_info *mtd)
{
    struct nand_chip  *chip = mtd->priv;
    struct nandc_host *host = chip->priv;
    u32 comoffset = host->column << 1;
    u16 value = 0;

    value = readw(chip->IO_ADDR_R + comoffset + host->offset);
    host->offset += 2;
    return value;
}

/*把nandc控制器buffer中的数据复制到内存中去*/
static void nandc_mtd_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
    struct nand_chip *chip = mtd->priv;
    struct nandc_host *host = chip->priv;
    u32 comoffset = host->column;

    /*begin lint-Info 732: (Info -- Loss of sign (arg. no.3) (int to unsigned int))*/
    memcpy(buf, (unsigned char *)(chip->IO_ADDR_R)+ comoffset + host->offset, (unsigned int)len);
    /*end*/
    host->offset += (unsigned int)len;

}

/*把内存中的数据写到nandc控制器的buffer中*/
static void nandc_mtd_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
    /*得到相关的参数*/
    struct nand_chip  *chip = mtd->priv;
    struct nandc_host *host = chip->priv;
    u32 comoffset = host->column;
    /*nandc控制器写一页的数据*/
    memset(chip->IO_ADDR_R + mtd->writesize, 0xff, NANDC_BAD_FLAG_BYTE_SIZE);
    memcpy(chip->IO_ADDR_R + comoffset + host->offset, buf, (unsigned int)len);
    host->offset += (unsigned int)len;

}

/*nand模块的命令的执行*/
static void nandc_mtd_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
    struct nand_chip *chip = mtd->priv;
    struct nandc_host *host = chip->priv;

    /*设置地址信息*/
    if (ctrl & NAND_ALE)
    {
        /*默认值的设置*/
        unsigned int addr_value = 0;
        unsigned int addr_offset = 0;

        if (ctrl & NAND_CTRL_CHANGE)
        {
            host->addr_cycle = 0x0;
            host->addr_physics[0] = 0x0;
            host->addr_physics[1] = 0x0;
        }
        addr_offset =  host->addr_cycle << 3;

        if (host->addr_cycle >= NANDC_ADDR_CYCLE_MASK)/*[false alarm]:屏蔽fortify错误 */
        {
            addr_offset = (host->addr_cycle - NANDC_ADDR_CYCLE_MASK) << 3;/*[false alarm]:屏蔽fortify错误 */
            addr_value = 1; /*[false alarm]:屏蔽fortify错误 */
        }

        host->addr_physics[addr_value] |= (((unsigned int)dat & 0xff) << addr_offset);

        host->addr_cycle ++;
    }
    /*这里是设置命令的接口*/
    if ((ctrl & NAND_CLE) && (ctrl & NAND_CTRL_CHANGE))
    {

        host->command = dat & 0xff;

        if((chip->options & NAND_BAD_MANAGEMENT) && ((dat == NAND_CMD_READSTART) || (dat == NAND_CMD_PAGEPROG)))
        {
            host->options   = NANDC_OPTION_ECC_FORBID;
            host->length    = mtd->writesize + mtd->oobsize;
            chip->options   &= ~NAND_BAD_MANAGEMENT;
        }
        else
        {
            host->options =  0;
        }

        if((NAND_CMD_ERASE2 != host->command) && (NAND_CMD_ERASE1 != host->command)
            && (!(NAND_BAD_MANAGEMENT & chip->options)))
        host->addr_physics[0] &= 0xffff0000; /*mask offset in page*/

        if(NANDC_STATUS_SUSPEND == g_nand_cmd_status)
        {
            NANDC_TRACE(NFCDBGLVL(ERRO), ("ERROR Nand controller is still suspending !!!!\n"));
            return;
        }
        /*控制器具体的命令的执行*/
        nandc_ctrl_excute_cmd(host);
    }
    /*命令执行完了之后清零操作*/
    if ((dat == NAND_CMD_NONE) && host->addr_cycle)
    {
        if (host->command == NAND_CMD_SEQIN
            || host->command == NAND_CMD_READ0
            || host->command == NAND_CMD_READID)
        {
            /*下面实现上就是把offset清零的操作*/
            host->offset = 0x0;
            host->column = host->addr_physics[0] & 0xffff;
        }
    }
}

/**
* 作用:判断nandc控制器是否ready
*
*/
static int nandc_mtd_dev_ready(struct mtd_info *mtd)
{
    return 0x1;
}

/**
* 作用:nandc_host的数据结构的创建和初始化
*
* 参数:
* platform_device:平台设备的数据结构
*
* 描述:初始化nandc控制器的数据结构并调用底层的nandc硬件控制器的初始化函数
*
*/
int nandc_mtd_chip_host_init(struct mtd_info *mtd, struct platform_device *pltdev, struct nandc_host *phost)
{
    struct nand_chip *chip = mtd->priv;
    int  i;
    uint32_t size_disable_ecc =0;
    /*首先把这些函数指针赋值*/
    phost->chip = chip ;
    phost->mtd  = mtd;
    phost->cluster  = &nandc6_bit_cluster;
    chip->priv = phost;
    /*nandc_v600的硬件buffer的ioremap操作*/
    chip->IO_ADDR_R = chip->IO_ADDR_W = ioremap_nocache(
                        pltdev->resource[1].start,
                        pltdev->resource[1].end - pltdev->resource[1].start + 1);
    if (!chip->IO_ADDR_R)
    {
        dev_err(&pltdev->dev, "ioremap IO_ADDR_R failed\n");
        return -EIO;
    }
    phost->regbase = (unsigned *)ioremap(pltdev->resource[0].start,
                    pltdev->resource[0].end - pltdev->resource[0].start + 1);

    if (!phost->regbase)
    {
        dev_err(&pltdev->dev, "ioremap failed\n");
        return -EIO;
    }
    phost->bufbase   =   (void __iomem *)chip->IO_ADDR_R;
    /*设置寄存器的长度*/
    phost->regsize   =   pltdev->resource[0].end - pltdev->resource[0].start + 1;
    phost->bufsize   =   pltdev->resource[1].end - pltdev->resource[1].start + 1;
    chip->cmd_ctrl      =   nandc_mtd_cmd_ctrl;
    chip->dev_ready     =   nandc_mtd_dev_ready;
    chip->select_chip   =   nandc_mtd_select_chip;
    chip->read_byte     =   nandc_mtd_read_byte;
    chip->read_word     =   nandc_mtd_read_word;
    chip->write_buf     =   nandc_mtd_write_buf;
    chip->read_buf      =   nandc_mtd_read_buf;

    chip->chip_delay    =   NANDC_CHIP_DELAY;
    chip->options       =   NAND_NO_AUTOINCR | NAND_SKIP_BBTSCAN;

    chip->ecc.layout    =   &nandc6_oob32_layout;
    chip->ecc.mode      =   NAND_ECC_NONE;

    chip->options &= ~NAND_BUSWIDTH_16;

    if(nand_scan(mtd, NANDC_MAX_CHIPS))
    {
        return -ENXIO;
    }
    /* 保存flash id */
    chip->select_chip(mtd, 0);
	chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);
	chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);
   	for (i = 0; i < NANDC_READID_SIZE; i++)
   	{
		phost->flash_id[i] = chip->read_byte(mtd);

   	}
    size_disable_ecc = mtd->writesize + mtd->oobsize ;
    /*配置一下在ecc0时读的数据个数*/
    writel(size_disable_ecc, (phost->regbase + NANDC_REG_DATA_NUM_OFFSET));
    return 0;
}


#if(FEATURE_ON == MBB_COMMON)

unsigned int ecc_type( unsigned int ctrl_cfg)
{
    switch ( ctrl_cfg )
    {
        case NANDC_ECC_NONE:  /*未使能flash控制器ECC*/
            break;
        case NANDC_ECC_1BIT: 
            return ECC_1BIT_CORRECT_BITS;       /* 1bit模式每512byte可纠错bit*/
        case NANDC_ECC_4BIT: 
            return ECC_4BIT_CORRECT_BITS;       /* 4bit模式每512byte可纠错bit*/
        case NANDC_ECC_8BIT:  
            return ECC_8BIT_CORRECT_BITS;       /* 8bit模式每512byte可纠错bit*/
        case NANDC_ECC_24BIT_1K:
            return ECC_24BIT_1K_CORRECT_BITS;   /* 24bit模式每512byte可纠错bit*/
        default:
            break;
    }
    return 0;
}
#endif

#if(FEATURE_ON == MBB_ECCSTATUS_OPTIMIZE)

void mbb_reb_info_init(struct nandc_host *host)
{
#if (FEATURE_ON == MBB_HWTEST)
    u32 spare_size = 0, codeword_size = 0;
#endif

    host->mtd->ecccorrectability = ecc_type(host->ecctype_onfi);

    NANDC_TRACE(NFCDBGLVL(ERRO), ("mbb_reb_info_init:ecccorrectability=%x!\n",host->mtd->ecccorrectability));

#if (FEATURE_ON == MBB_HWTEST)
    if(2048 == host->mtd->writesize)
    {
        codeword_size = 2;   
    }
    else
    {
        codeword_size = 4;  
    }
    spare_size = 16; /*巴龙平台默认为16*/
    init_hardware_flash_info(codeword_size, spare_size, host->mtd->ecccorrectability);
#endif
}
#endif
/**
* 作用:nandc模块提供的probe函数
*
* 参数:
* platform_device:平台设备的数据结构
*
* 描述:初始化nandc控制器的数据结构并调用底层的nandc硬件控制器的初始化函数
*
*/
static int nandc_mtd_probe(struct platform_device* dev)
{
    struct nandc_host *host = NULL;
    struct nand_chip  *chip = NULL;
    struct mtd_info   *mtd  = NULL;
    struct platform_device *pltdev = dev; /* nandc_mtd_pltdev */
    struct mtd_partition *parts = NULL;

    unsigned int nr_parts = 0;
    int result = 0;

    int size = sizeof(struct nand_chip) + sizeof(struct mtd_info);
    /*此时EBI是否要设置一下*/
    //hi_syssc_ebi_mode_normal();

    /*nand_chip和mtd的内存空间分配*/
    chip = (struct nand_chip*)kmalloc((size_t)size, GFP_KERNEL);
    if (!chip)
    {
        dev_err(&pltdev->dev, "nandc_mtd_probe failed to allocate device structure.\n");
        return -ENOMEM;
    }
    memset((void *)chip, 0, (unsigned int)size);
    mtd  = (struct mtd_info *)&chip[1];
    mtd->priv  = chip;
    mtd->owner = THIS_MODULE;
    mtd->name  = (char*)(pltdev->name);

    /*nandc_host的数据结构内存分配*/
    size = sizeof(struct nandc_host) ;
    /* coverity[alloc_fn] */
    /* coverity[var_assign] */
    host = (struct nandc_host *)kmalloc((size_t)size, GFP_KERNEL);
    if (!host)
    {
        dev_err(&pltdev->dev, "nandc_mtd_probe failed to allocate nandc_host.\n");
        kfree(chip);
        chip=NULL;
        return -ENOMEM;
    }
    /* coverity[noescape] */
    memset((void *)host, 0, (unsigned int)size);
    host->clk = clk_get(NULL,"nandc_clk");
    if(IS_ERR(host->clk)){
		dev_err(&pltdev->dev, "cannot get nandc clk!\n");
		goto err;
	}
#ifdef CONFIG_PM
    /* get memory to story register value when deepsleep */
    host->sleep_buffer = (u32 *)kmalloc(NANDC_SLEEP_BUFFER_SIZE, GFP_KERNEL);
    if(!host->sleep_buffer)
    {
        kfree(host);
        host = NULL;
        kfree(chip);
        chip = NULL;
        return -ENOMEM;
    }
#endif
    pltdev->dev.platform_data = host;
    if (nandc_mtd_chip_host_init(mtd, pltdev, host))
    {
        dev_err(&pltdev->dev, "failed to nandc_mtd_host_init.\n");
        result = -ENOMEM;
        goto err;
    }
    
#if(FEATURE_ON == MBB_ECCSTATUS_OPTIMIZE)
    mbb_reb_info_init(host);
#endif

    /* coverity[alloc_arg] */
    if(ptable_parse_mtd_partitions(&parts, &nr_parts))/*lint !e64*/
    {
        dev_err(&pltdev->dev, "failed to ptable_parse_mtd_partitions.\n");
        result = -ENOMEM;
        goto err;
    }
    mtd->name  = (char*)(pltdev->name);

    if (nr_parts > 0)
    {
#if(FEATURE_ON == MBB_DLOAD)
        dload_set_mtd_info(mtd);
#endif
        add_mtd_partitions(mtd, parts, (int)nr_parts);
        return 0;
    }
    else
    {
        printk("ERROR !No Partition !!!!\n");
        result = -ENODEV;
        goto err;
    }

err:
    nandc_mtd_remove(pltdev);
    /* coverity[leaked_storage] */
    return result;
}


/*设备删除函数*/
static int nandc_mtd_remove(struct platform_device *dev)
{
    struct mtd_info *mtd = dev->dev.platform_data;
    struct nand_chip  *chip = NULL;
    struct nandc_host   *host  = NULL;
    chip = mtd->priv;
    host = chip->priv;
    nand_release(host->mtd);
    if(chip->IO_ADDR_W)
    {
        iounmap(chip->IO_ADDR_W);
        chip->IO_ADDR_W =NULL;

    }
    if(host->regbase)
    {
        iounmap(host->regbase);
        host->regbase =NULL;
    }
    kfree(host);
    host=NULL;
    kfree(chip);
    chip=NULL;
    return 0;
}


#ifdef CONFIG_PM

u32 reg_offset[NANDC_SLEEP_BUFFER_SIZE / sizeof(u32)] =
{
    NANDC_REG_CON_OFFSET,
    NANDC_REG_PWIDTH_OFFSET,
    NANDC_REG_OPIDLE_OFFSET,
    NANDC_REG_DATA_NUM_OFFSET,
    NANDC_REG_OP_PARA_OFFSET,
    NANDC_REG_OOB_SEL_OFFSET,
    END_FLAG
};
/*nand模块的低功耗suspend函数*/
static int balong_nandc_suspend(struct device *dev)
{
    struct nandc_host *host = (struct nandc_host *)dev->platform_data;
    u32 nandc_reg_base_addr = (u32)(host->regbase);
    u32 *nandc_sleep_buffer  = host->sleep_buffer;
    u32 i;
    s32 ret;
    ret = clk_prepare_enable(host->clk);
    if(ret)
    {
        return ret;
    }

    for(i = 0; reg_offset[i] != END_FLAG; i++)
    {
        *nandc_sleep_buffer = readl((void *)(nandc_reg_base_addr + reg_offset[i]));
        /*如果是8bit的ecc的话就要设置一下ecc_type寄存器*/
        if((reg_offset[i] == NANDC_REG_CON_OFFSET) && (host->ecctype_onfi == NANDC_ECC_8BIT))
        {
            *nandc_sleep_buffer = ((*nandc_sleep_buffer) | (NANDC_ECC_8BIT << 8));
        }
        nandc_sleep_buffer++;
    }
    clk_disable_unprepare(host->clk);
    g_nand_cmd_status = NANDC_STATUS_SUSPEND;
    return 0;
}

/*nand模块的低功耗恢复函数*/
static int balong_nandc_resume(struct device *dev)
{
    struct nandc_host *host = (struct nandc_host *)dev->platform_data;

    u32 nandc_reg_base_addr = (u32)(host->regbase);
    u32 *nandc_sleep_buffer  = host->sleep_buffer;
    u32 i;
    s32 ret;

    ret = clk_prepare_enable(host->clk);
    if(ret)
    {
        return ret;
    }

    host->command = NAND_CMD_RESET;
    nandc_ctrl_excute_cmd(host);
    for(i = 0; reg_offset[i] != END_FLAG; i++)
    {
        writel(*nandc_sleep_buffer, (void *)(nandc_reg_base_addr + reg_offset[i]));
        nandc_sleep_buffer++;

    }
    clk_disable_unprepare(host->clk);
    g_nand_cmd_status = NANDC_STATUS_RESUME;
    return 0;
}

static const struct dev_pm_ops balong_nandc_dev_pm_ops =
{
    .suspend_noirq    =   balong_nandc_suspend,
    .resume_noirq     =   balong_nandc_resume,
};

#define BALONG_NANDC_PM_OPS (&balong_nandc_dev_pm_ops)
#else
#define BALONG_NANDC_PM_OPS NULL
#endif


/*先把DTS模块移植成功后再来开nand模块的DTS*/
#ifdef CONFIG_OF
static const struct of_device_id hs_nandc_balong_of_match[] = {
	{ .compatible = "hisilicon,balong-nandc", },
	{},
};
MODULE_DEVICE_TABLE(of, hs_nandc_balong_of_match);
#endif

static struct platform_driver nandc_mtd_driver =
{
    .driver = {	.name       =   NANDC_NAME,
				.owner  	=	THIS_MODULE,
			    .bus        =   &platform_bus_type,
			    .pm         =   BALONG_NANDC_PM_OPS,
			    #ifdef CONFIG_OF
			    .of_match_table = of_match_ptr(hs_nandc_balong_of_match),
			    #endif
    		  },
    .probe      =   nandc_mtd_probe,
};

#if(FEATURE_ON == MBB_DLOAD)
/*
 * stub function not used now
 *
 */
static int mtdflushpart_proc_read(struct file *file, char __user *buf,size_t count, loff_t *ppos)
{
    return 0;
}

/*
 * This function flush mtd partition, when userspace write '1' to
 * /proc/mtdflush file
 *
 * file: file struct
 * buffer: buffer from user space
 * count: buffer length
 * data: private data point
 */
static int mtdflushpart_proc_write(struct file *file, const char __user *buffer, 
                        size_t count, loff_t *ppos)
{
    char *buf = NULL;
    int err;
 #if (FEATURE_ON == MBB_DLOAD_QC)
    struct msm_nand_info *nand_info = NULL;
 #endif
    if (count < 1 || count > DLOAD_MTDFLUSH_COUNT_MAX)
    {
        printk(KERN_ERR "count length error\n");
        return - EINVAL;
    }

#if (FEATURE_ON == MBB_DLOAD_QC)
    if (NULL == data)
    {
        printk(KERN_ERR "param error\n");
        return - EINVAL;
    }
    else
    {
        nand_info = (struct msm_nand_info *)data;
    }
#endif
    buf = kmalloc(count, GFP_KERNEL);
    if (NULL == buf)
    {
        printk(KERN_ERR "kmalloc error\n");
        return - ENOMEM;
    }

    if (copy_from_user(buf, buffer, count))
    {
        printk(KERN_ERR "copy_from_user error\n");
        kfree(buf);
        return - EFAULT;
    }

    if (buf[0] == '1')
    {
#if (FEATURE_ON == MBB_DLOAD_QC)
        err = mtdchar_flush_part(&(nand_info->mtd));
#endif
    }

    kfree(buf);
    return count;
}

/*******************************************************************************
 * FUNC NAME:
 * nvresumeport_proc_read() -
 *
 * PARAMETER:
 *
 * DESCRIPTION:stub function not used now
 *
 * CALL FUNC:
 * () -
********************************************************************************/
static int nvresumeport_proc_read(struct file *file, char __user *buf,size_t count, loff_t *ppos)
{
    return 0;
}

/*******************************************************************************
 * FUNC NAME:
 * nvresumeport_proc_write() -
 *
 * PARAMETER:
 * @file: file struct
 * @buffer: buffer from user space
 * @count: buffer length
 * @data: private data point
 * 
 * DESCRIPTION:write nvrestoreflag,  pcui port switch. 
 *
 * CALL FUNC:
 * () -
********************************************************************************/
static int nvresumeport_proc_write(struct file *file, const char __user *buffer, 
                        size_t count, loff_t *ppos)
{
    unsigned int flag = 0;
    huawei_smem_info *smem_data = NULL;

    if (DLOAD_MTDFLUSH_COUNT_MAX != count)
    {
	    printk(KERN_ERR "count length error\n");
        return - EINVAL;
    }

    if (copy_from_user(&flag, buffer, count))
    {
	    printk(KERN_ERR "copy_from_user error\n");
        return - EFAULT;
    }
    
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
	    printk(KERN_ERR "nvresumeport_proc_write:get smem_data error\n");
        return - EFAULT;
    }
    /* download over set nvresume flag */
    smem_data->smem_switch_pcui_flag = flag;
    /* download over clear dload flag */
    smem_data->smem_dload_flag = 0;

    smem_data->smem_sd_upgrade = 0;

    return count;
}

#if ((FEATURE_ON == MBB_DLOAD_SDUP) || (FEATURE_ON == MBB_DLOAD_BBOU))
/*******************************************************************************
 * FUNC NAME:
 * nvresumeflag_proc_read() -
 *
 * PARAMETER:
 *
 * DESCRIPTION:stub function not used now
 *
 * CALL FUNC:
 * () -
********************************************************************************/
static int nvresumeflag_proc_read(struct file *file, char __user *buf,size_t count, loff_t *ppos)
{
    return 0;
}

/*******************************************************************************
 * FUNC NAME:
 * nvresumeflag_proc_write() -
 *
 * PARAMETER:
 * @file: file struct
 * @buffer: buffer from user space
 * @count: buffer length
 * @data: private data point
 * 
 * DESCRIPTION:write nvrestoreflag,  app not start. 
 *
 * CALL FUNC:
 * () -
********************************************************************************/
static int nvresumeflag_proc_write(struct file *file, const char __user *buffer, 
                        size_t count, loff_t *ppos)
{
    unsigned long int flag = 0;
    huawei_smem_info *smem_data = NULL;

    if (DLOAD_MTDFLUSH_COUNT_MAX != count)
    {
        return -EINVAL;
    }

    if (copy_from_user(&flag, buffer, count))
    {
        printk(KERN_ERR "copy_from_user error\n");
        return -EFAULT;
    }
    
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        printk(KERN_ERR "nvresumeflag_proc_write:get smem_data error\n");
        return -EFAULT;
    }

    smem_data->smem_online_upgrade_flag = flag;
    printk("smem_data->smem_online_upgrade_flag = 0x%x,flag = %lu\n",smem_data->smem_online_upgrade_flag,flag);
    return count;
}
#endif /*defined (MBB_DLOAD_SDUP) || defined (MBB_DLOAD_BBOU)*/

#if (FEATURE_ON == MBB_DLOAD_MCPEUP)
static const struct file_operations g_file_forcedloadflag ={
    .owner    = THIS_MODULE,
};
#endif
static const struct file_operations g_file_flushpart ={
    .owner    = THIS_MODULE,
    .read     = mtdflushpart_proc_read,
    .write    = mtdflushpart_proc_write,
};
static const struct file_operations g_file_nvresumeport ={
    .owner    = THIS_MODULE,
    .read     = nvresumeport_proc_read,
    .write    = nvresumeport_proc_write,
};
#if ((FEATURE_ON == MBB_DLOAD_SDUP) || (FEATURE_ON == MBB_DLOAD_BBOU))
static const struct file_operations g_file_nvresumeflag ={
    .owner    = THIS_MODULE,
    .read     = nvresumeflag_proc_read,
    .write    = nvresumeflag_proc_write,
};
#endif
#if (FEATURE_ON == MBB_DLOAD_SDUP)
static const struct file_operations g_file_sdupgradeflag ={
    .owner    = THIS_MODULE,
};
#endif
static const struct file_operations g_file_nvrestore_pcuiflag ={
    .owner    = THIS_MODULE,
};
static const struct file_operations g_file_factoryflag ={
    .owner    = THIS_MODULE,
};

/*******************************************************************************
 * FUNC NAME:
 * nandc_mtd_dload_proc_deal() -
 *
 * PARAMETER:
 * @file: file struct
 * 
 * 
 * DESCRIPTION: upgrade proc node create or init. 
 *
 * CALL FUNC:
 * () -
********************************************************************************/
static int nandc_mtd_dload_proc_deal(void)
{
    int err = 0;
    struct proc_dir_entry *flushpart = NULL;
    struct proc_dir_entry *pcuiflag = NULL;
    struct proc_dir_entry *nvresumeflag = NULL;
    struct proc_dir_entry *sdupgradeflag = NULL;
    struct proc_dir_entry *nvrestore_pcuiflag = NULL; /*NV 自动恢复阶段，设置PROC 节点信息*/
    struct proc_dir_entry *factoryflag = NULL;

#if (FEATURE_ON == MBB_DLOAD_MCPEUP)
    struct proc_dir_entry *forceloadflag = NULL;
#endif
    huawei_smem_info *smem_data = NULL;
    
    printk("nandc_mtd_dload_proc_deal start \n");
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;

    if (NULL == smem_data)
    {
        printk(KERN_ERR "nandc_mtd_dload_proc_deal:get smem_data error\n");
        err = -EINVAL;
        goto out;
    }
#if (FEATURE_ON == MBB_DLOAD_MCPEUP)
    printk("smem_data->smem_dload_flag = 0x%x \
            smem_data->smem_forceload_flag = 0x%x!\n", \
            smem_data->smem_dload_flag,smem_data->smem_forceload_flag);
#else
    printk("smem_data->smem_dload_flag = 0x%x!\n", smem_data->smem_dload_flag);
#endif

    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
#if (FEATURE_ON == MBB_DLOAD_MCPEUP)
        if (SMEM_FORCELOAD_FLAG_NUM == smem_data->smem_forceload_flag)
        {
            forceloadflag = proc_create("forceloadflag", S_IFREG | S_IRUGO, NULL, &g_file_forcedloadflag);
            if (NULL == forceloadflag)
            {
                printk(KERN_ERR "create forceloadflag procfile error\n");
                err = -EINVAL;
                goto out;
            }
        }
#endif
        flushpart = proc_create("mtdflushpart", S_IFREG | S_IRUGO, NULL, &g_file_flushpart);
        if (NULL == flushpart)
        {
            printk(KERN_ERR "create mtdflushpart procfile error\n");
            err = -EINVAL;
            goto out;
        }

        pcuiflag = proc_create("nvresumeport", S_IFREG | S_IRUGO, NULL, &g_file_nvresumeport);
        if (NULL == pcuiflag)
        {
            printk(KERN_ERR "create nvresumeport procfile error\n");
            err = -EINVAL;
            goto out;
        }

    /*为触犯SD卡升级创建，设备PROC 节点*/
#if (FEATURE_ON == MBB_DLOAD_SDUP)
        if((SMEM_SDUP_FLAG_NUM == smem_data->smem_sd_upgrade))
        {
              /* SD卡升级魔术字被设置，创建sdupgradeflag 
                 节点，屏蔽APP 启动*/
            sdupgradeflag = proc_create("sdupgradeflag", S_IFREG | S_IRUGO, NULL, &g_file_sdupgradeflag);
            if (NULL == sdupgradeflag)
            {
                printk(KERN_ERR "create sdupgradeflag procfile error\n");
                err = -EINVAL;
                goto out;
            }
        }
#endif /*CONFIG_NBB_DLOAD_SDUP*/ 
    /*为SD卡升级和在线升级，升级NV恢复阶段标识*/
#if ((FEATURE_ON == MBB_DLOAD_SDUP) || (FEATURE_ON == MBB_DLOAD_BBOU))
        nvresumeflag = proc_create("nvresumeflag", S_IFREG | S_IRUGO, NULL, &g_file_nvresumeflag);
        if (NULL == nvresumeflag)
        {
            printk(KERN_ERR "create nvresumeflag procfile error\n");
            err = -EINVAL;
            goto out;
        }
#endif /*defined (CONFIG_MBB_DLOAD_SDUP) || defined (CONFIG_MBB_DLOAD_BBOU)*/
    }

    if(SMEM_SWITCH_PUCI_FLAG_NUM == smem_data->smem_switch_pcui_flag)
    {
        /*一键升级NV自动恢复阶段，设置nvrestore_pcuiflasg
         节点，屏蔽APP android 启动*/
        nvrestore_pcuiflag = proc_create("nvrestore_pcuiflag", S_IFREG | S_IRUGO, NULL, &g_file_nvrestore_pcuiflag);
        if (NULL == nvrestore_pcuiflag)
        {
            printk(KERN_ERR "create nvrestore_pcuiflag procfile error\n");
            err = - EINVAL;
            goto out;
        }
    }

    if(SMEM_BURN_UPDATE_FLAG_NUM == smem_data->smem_burn_update_flag)
    {
        /* 用此proc节点来判断单板是否为烧片版本 */
        factoryflag = proc_create("factoryflag", S_IFREG | S_IRUGO, NULL, &g_file_factoryflag); 
        if (NULL == factoryflag)
        {
            printk(KERN_ERR "create factoryflag procfile error\n");
            err = -EINVAL;
            goto out;
        }
    }
out:
    printk("nandc_mtd_dload_proc_deal end err = %d \n",err);
    return err;
    
}
#endif /*MBB_DLOAD*/
/**
* 作用:nandc模块模块初始化函数
*
* 参数:
* 无
*
*/
static int __init nandc_mtd_module_init(void)
{
    int result;
    printk("Hisilicon Nand Flash Controller Driver, Version 1.00\n");
    result = platform_driver_register(&nandc_mtd_driver);
#if(FEATURE_ON == MBB_DLOAD)
    result = nandc_mtd_dload_proc_deal();
    if(result)
    {
         printk("nandc_mtd_dload_proc_deal error \n");
    }
#endif /*MBB_DLOAD*/
    return result;
}
/*nand模块没有module_exit的函数是因为nand模块会一直不卸载*/
module_init(nandc_mtd_module_init);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hisilicon Tech.Co.,Ltd.<monan@huawei.com>");
MODULE_DESCRIPTION("Balong Hisilicon Nand Flash Controller driver");
