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
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <bsp_nandc.h>
#include "nandc_ctrl.h"
#include "nandc_mtd.h"
/*lint --e{746,737,702,533,830,745,746,808,701,830}*/

#if(FEATURE_ON == MBB_REB)
#define ECC_ERR_NUM_OFFSET (NANDC_REG_ERR_NUM0_BUF0_OFFSET)
#define FLASH_CTRL_OFFSET  (NANDC_REG_CON_OFFSET)

void (*set_ecc_result_func)( unsigned int, unsigned int );
void (*get_ecc_cfg_func)( unsigned int ) = NULL;
#endif
#if (FEATURE_ON == MBB_HWTEST)
extern int bit_flip_info_record(unsigned int cw_id,unsigned int corrected);
#endif
unsigned int g_nand_cmd_status;
/*控制NANDC_TRACE打印信息的级别，默认情况下面NANDC_TRACE是打印error和warning信息*/
u32 nandc_trace_mask = (u32)(NANDC_TRACE_ALL | NANDC_TRACE_ERRO | NANDC_TRACE_WARNING);

/*判断状态寄存器是否完成*/
u32 nandc_ctrl_register_wait(struct nandc_host *host, struct nandc_bit_reg status_bit, u32 value, u32 timeout)
{
    do
    {
        if(value == NANDC_GET_REG_BITS(status_bit))
        {
        return NANDC_OK;
        }
        timeout--;
    }
    while(timeout > 0);
    NANDC_TRACE(NFCDBGLVL(ERRO), ("nandc_ctrl_register_wait,timeout! \n"));
    return NANDC_E_TIMEOUT;
}
/**
* 作用:查询nandc控制器的擦除状态
*
* 参数:
* @host          		    --nandc的主机控制器指针
*
* 描述:查询擦除的状态
*
*/
u32 nandc_ctrl_erase_result(struct nandc_host *host)
{
    u32 errorcode = NANDC_ERROR;
    u32 nand_status;

    /*等待此次操作完成*/
    errorcode = nandc_ctrl_register_wait(host, host->cluster->op_stauts, NANDC_STATUS_OP_DONE, NANDC_ERASE_RESULT_TIMEOUT);
    if(NANDC_OK != errorcode)
    {
        NANDC_TRACE(NFCDBGLVL(ERRO), ("nandc_ctrl_erase_result,wait return error! \n"));/*lint !e778*/
        goto ERRO;
    }

    if(NANDC_STATUS_INT_DONE != NANDC_GET_REG_BITS(host->cluster->int_stauts))
    {
        errorcode = NANDC_E_ERASE;
        NANDC_TRACE(NFCDBGLVL(ERRO), ("nandc_ctrl_erase_result,int return error! \n"));/*lint !e778*/
        goto ERRO;
    }
    /*读nandflash的状态来判断操作是否执行成功*/
    nand_status = NANDC_GET_REG_BITS(host->cluster->nf_status);

    if(NAND_STATUS_FAIL & nand_status)
    {
        NANDC_TRACE(NFCDBGLVL(ERRO), ("nandc_ctrl_erase_result,status return FAIL!:0x%x \n",nand_status));/*lint !e778*/
        errorcode = NANDC_E_ERASE;
        goto ERRO;
    }
    return NANDC_OK;
ERRO:
    return errorcode;
}

/**
* 作用:查询nandc控制器的program后的状态
*
* 参数:
* @host          		    --nandc的主机控制器指针
*
* 描述:查询program后的状态
*
*/
 u32 nandc_ctrl_program_page_result(struct nandc_host *host)
{
    u32 errorcode = NANDC_ERROR;
    u32 nand_status_timeout = NANDC_PRG_RESULT_TIMEOUT;

    /*查看编程是否完成*/
    errorcode = nandc_ctrl_register_wait(host, host->cluster->op_stauts, NANDC_STATUS_OP_DONE, NANDC_PRG_RESULT_TIMEOUT);
    if(NANDC_OK != errorcode)
    {
        NANDC_TRACE(NFCDBGLVL(ERRO), ("nandc_ctrl_program_result,wait return ERROR! \n"));/*lint !e778*/
        return errorcode;
    }
    while ((NAND_STATUS_FAIL ==  (NAND_STATUS_FAIL & NANDC_GET_REG_BITS(host->cluster->nf_status))) && (nand_status_timeout))
    {
        nand_status_timeout--;
    }
    g_nand_cmd_status = NANDC_GET_REG_BITS(host->cluster->nf_status);

    if (0 ==  nand_status_timeout)
    {
        NANDC_TRACE(NFCDBGLVL(ERRO), ("nandc_ctrl_program_result,status return FAIL! \n"));/*lint !e778*/
        return NANDC_E_TIMEOUT;
    }
    return NANDC_OK;
}

/**
* 作用:读nandc的状态寄存器判断读操作是否正确
*
* 参数:
* @host        --nandc的主机控制器指针
*
*/
 u32 nandc_ctrl_read_page_result(struct nandc_host *host)
{
    u32 errorcode = NANDC_ERROR;
#if(FEATURE_ON == MBB_ECCSTATUS_OPTIMIZE)
    host->mtd->exceed_threshold = 0;
#endif
    struct nandc_bit_cluster *cluster;

    cluster = host->cluster;
    /*查询读的状态，判断是否完成*/
    errorcode = nandc_ctrl_register_wait(host, cluster->op_stauts, NANDC_STATUS_OP_DONE, NANDC_RD_RESULT_TIMEOUT);
    if(NANDC_OK != errorcode)
    {
        NANDC_TRACE(NFCDBGLVL(ERRO), ("wait nandc_ctrl_read_result!ERROR \n"));
        goto ERRO;
    }
    #ifdef NANDC_READ_RESULT_DELAY
    {
        volatile u32 loopcnt = 500;
        while(loopcnt--)
        {};                  /*wait 500 before read controler status (0x20 bit1)*/
    }
    #endif
    if(NANDC_ECC_ERR_VALID == NANDC_GET_REG_BITS(cluster->ecc_result))
    {
        u32 ecc_warning_data = 0;
        ecc_warning_data = NANDC_GET_REG_BITS(cluster->errnum0buf0);
#if(FEATURE_ON == MBB_ECCSTATUS_OPTIMIZE)
        int n = 0;
        u32 ecc_errs = 0;
        int sector = 0;

        if(2048 == host->mtd->writesize)
        {
            sector = 2;   
        }
        else
        {
            sector = 4;  
        }

        for (n = 0; n < sector; n++) 
        {
            ecc_errs = ecc_warning_data & (0xff << (n * 8));
            ecc_errs = ecc_errs >> (n * 8);
            if(ecc_errs)
            {
#if (FEATURE_ON == MBB_HWTEST)
                /*record ecc flip info*/      
                bit_flip_info_record(n,ecc_errs);
#endif
                host->mtd->ecc_stats.corrected += ecc_errs;
                if(ecc_errs > (2 * host->mtd->ecccorrectability) * 3 / 4)
                {
                     host->mtd->exceed_threshold = 1;
                }
            }
        }
        /*调试的时候打开，上库时关闭
        NANDC_TRACE(NFCDBGLVL(ERRO), ("just ecc warning errnum0buf0 sector ecccorrectability exceed_threshold0x%x! %d %d %d %d\n",ecc_warning_data,host->mtd->ecc_stats.corrected,sector,host->mtd->ecccorrectability,host->mtd->exceed_threshold));
        NANDC_TRACE(NFCDBGLVL(ERRO), ("nandc_ctrl_read_result:warning address[0]:0x%x, address[1]:0x%x!\n",host->addr_physics[0],host->addr_physics[1]));
        */
#else
        //MBB,先注释掉，后期可靠性调试时再修改log打印级别
        //NANDC_TRACE(NFCDBGLVL(WARNING), ("just ecc warning errnum0buf0  0x%x!\n",ecc_warning_data));
#endif
    }
    if(NANDC_ECC_ERR_INVALID == NANDC_GET_REG_BITS(cluster->ecc_result))
    {
        u32 ecc_error_data = 0;
        ecc_error_data = NANDC_GET_REG_BITS(cluster->errnum0buf0);
        host->mtd->ecc_stats.failed++;
        NANDC_TRACE(NFCDBGLVL(ERRO), ("nandc_ctrl_read_result: ecc error invalid errnum0buf0  0x%x!\n",ecc_error_data))
        errorcode = NANDC_E_ECC;
        goto ERRO;

    }
    return NANDC_OK;
ERRO:
    NANDC_TRACE(NFCDBGLVL(ERRO), ("nandc_ctrl_read_result:error address[0]:0x%x, address[1]:0x%x!\n",
                                                host->addr_physics[0],host->addr_physics[1]));
    return errorcode;
    }

/*返回nand控制器操作后的状态*/
static u32 nandc_ctrl_read_excutecmd_status(struct nandc_host *host)
{
    /*把nand控制器的状态信息放在控制器的BUFFER中*/
    *(volatile unsigned int*)host->bufbase = g_nand_cmd_status | NAND_STATUS_WP | NAND_STATUS_READY;
    return NANDC_OK;
}
/**
* 作用:复位nandflash的操作
*
* 参数:
* @host         --nandc的主机控制器指针
*
* 描述:复位nandflash的操作
*
*/
u32 nandc_ctrl_reset(struct nandc_host *host)
{
    struct nandc_bit_cluster*  cluster;
    volatile int reset_timeout = 10000;
    u32 errorcode = NANDC_ERROR;

    cluster = (struct nandc_bit_cluster*)host->cluster;
    NANDC_SET_REG_BITS (cluster->cmd,     NAND_CMD_RESET|(NAND_CMD_NOP<<8)|(NAND_CMD_NOP<<16) );
    NANDC_SET_REG_BITS (cluster->int_clr,     INTMASK_ALL);
    NANDC_WRITE_REG_BITS(cluster->addr_cycle,       NANDC_ADDRCYCLE_0    );
    NANDC_SET_REG_BITS (cluster->int_enable,       INTDIS_ALL);

    NANDC_SET_REG_BITS (cluster->operate,          OP_RESET    );

    g_nand_cmd_status = 0;
    /*begin lint-Info 732: (Info -- Loss of sign (arg. no. 4) (int to unsigned int))*/
    errorcode = nandc_ctrl_register_wait(host, cluster->int_stauts, NANDC_STATUS_INT_DONE, (u32)reset_timeout);
    /*end*/
    if(NANDC_OK != errorcode)
    {
        return NANDC_ERROR;
    }
    errorcode = nandc_ctrl_register_wait(host, cluster->op_stauts, NANDC_STATUS_OP_DONE, NANDC_RESULT_TIME_OUT);
    if(NANDC_OK != errorcode)
    {
        return NANDC_ERROR;
    }
    return NANDC_OK;

}


/**
* 作用:读nandflash的ID
*
* 参数:
* @host            --nandc的主机控制器指针
*
* 描述:读nandflash的ID
*
*/
u32 nandc_ctrl_read_id(struct nandc_host *host)
{
    struct nandc_bit_cluster*  cluster;

    u32 errorcode;

    cluster = (struct nandc_bit_cluster*)host->cluster;
    NANDC_SET_REG_BITS (cluster->int_enable,     INTDIS_ALL);
    nandc_ctrl_reset(host);

    NANDC_SET_REG_BITS(cluster->cmd,     NAND_CMD_READID|(NAND_CMD_NOP<<8)|(NAND_CMD_NOP<<16) );
    NANDC_SET_REG_BITS(cluster->addr_high,         0                   );
    NANDC_SET_REG_BITS(cluster->addr_low,          0                   );
    NANDC_SET_REG_BITS(cluster->ecc_type,          0                   );

    NANDC_SET_REG_BITS (cluster->int_clr,     INTMASK_ALL);

    NANDC_WRITE_REG_BITS(cluster->addr_cycle,      NANDC_ADDRCYCLE_1   );
    NANDC_SET_REG_BITS(cluster->operate,         OP_READ_ID );

    errorcode = nandc_ctrl_register_wait(host, cluster->int_stauts, NANDC_STATUS_INT_DONE, NANDC_RESULT_TIME_OUT);
    if(NANDC_OK != errorcode)
    {
        goto ERRO;
    }
    errorcode = nandc_ctrl_register_wait(host, cluster->op_stauts, NANDC_STATUS_OP_DONE, NANDC_RESULT_TIME_OUT);
    if(NANDC_OK != errorcode)
    {
        goto ERRO;
    }
    return NANDC_OK;
ERRO:
    return NANDC_ERROR;

}

/**
* 作用:读nandc的状态寄存器判断读操作是否正确
*
* 参数:
* @host        --nandc的主机控制器指针
*
*/
u32 nandc_ctrl_erase_block(struct nandc_host *host)
{
    struct nandc_bit_cluster *cluster=NULL;
    u32 count = 3;
    u32 errocode = NANDC_ERROR;
    cluster = host->cluster;

    NANDC_SET_REG_BITS(cluster->int_enable,INTDIS_ALL);          /* disable all interrupt */
    if(0 == host->addr_physics[0])
    {
        NANDC_TRACE(NFCDBGLVL(WARNING), ("nandc_ctrl_earse_block: erase blknum 0.\n"));
    }
    NANDC_SET_REG_BITS (cluster->cmd,     NAND_CMD_ERASE1|(NAND_CMD_ERASE2<<8)|(NAND_CMD_STATUS<<16) );
    NANDC_SET_REG_BITS (cluster->int_clr,     INTMASK_ALL);

    NANDC_SET_REG_BITS(cluster->addr_low,       host->addr_physics[0]   );
    NANDC_SET_REG_BITS(cluster->ecc_type,       0                       );
    NANDC_WRITE_REG_BITS(cluster->addr_cycle,     host->addr_cycle	    );

    while((NANDC_OK != errocode)&&(count > 0))
    {
        NANDC_SET_REG_BITS(cluster->operate,        OP_ERASE   );
        errocode =  nandc_ctrl_erase_result(host);
        count--;
    }
    if(NANDC_OK != errocode)
    {
        NANDC_TRACE(NFCDBGLVL(ERRO), ("nandc_ctrl_earse_block, failed 3 times!\n"));/*lint !e778*/
        return errocode;
    }
    return NANDC_OK;
}
/**
* 作用:读写数据时配置ecc是否使能
*
* 参数:
* @host          		    --nandc的主机控制器指针
*
*
*/
u32 nandc_ctrl_read_write_eccconfig(struct nandc_host *host)
{
    struct nandc_bit_cluster*   cluster;
    u32 ecctype;

    cluster = host->cluster;
    if(NAND_CMD_READSTART == host->command)
    {
        NANDC_SET_REG_BITS(cluster->cmd, NAND_CMD_READ0|(NAND_CMD_READSTART<<8)|(NAND_CMD_NOP<<16));
    }
    else
    {
        NANDC_SET_REG_BITS(cluster->cmd, NAND_CMD_SEQIN|(NAND_CMD_PAGEPROG<<8)|(NAND_CMD_STATUS<<16));
    }
    /*如果是不使用ECC的情况时*/
    if(host->options & NANDC_OPTION_ECC_FORBID)
    {
        ecctype =0;
        host->options = host->options & (~NANDC_OPTION_ECC_FORBID);
    }
    else
    {
        ecctype= host->ecctype_onfi;
    }
    NANDC_SET_REG_BITS(cluster->ecc_type,     ecctype);
    NANDC_WRITE_REG_BITS(cluster->addr_cycle,      host->addr_cycle);
    return 0;
}
/**
* 作用:读一整页的数据
*
* 参数:
* @host          		    --nandc的主机控制器指针
*
* 描述:读整页面数据
*
*/
u32 nandc_ctrl_read_page(struct nandc_host *host)
{
    struct nandc_bit_cluster*   cluster;
    u32 errorcode;
#if(FEATURE_ON == MBB_REB)
    u32 ecc_ability = 0;
#endif
    cluster = host->cluster;
    /*如果flash的页面数大于8k的话就不支持*/
    if(host->mtd->writesize > NANDC_BUFSIZE_BASIC)
    {
        NANDC_TRACE(NFCDBGLVL(NORMAL), ("nandc_ctrl_read_page error, pagesize %x! \n",host->mtd->writesize));
        return NANDC_ERROR;
    }

    NANDC_SET_REG_BITS(cluster->int_enable,INTDIS_ALL); /* disable all interrupt */
    /*是否已经写了部分的硬件寄存器，如果没有写的话要写一次*/
    nandc_ctrl_read_write_eccconfig(host);

#if(FEATURE_ON == MBB_REB)
    if ( NULL != get_ecc_cfg_func )
    {
        ecc_ability = ecc_type(host->ecctype_onfi);
        get_ecc_cfg_func(ecc_ability);
    }
#endif

    NANDC_SET_REG_BITS (cluster->addr_low,       host->addr_physics[0]		);
    NANDC_SET_REG_BITS (cluster->addr_high,      host->addr_physics[1]		);
    NANDC_SET_REG_BITS (cluster->int_clr,        INTMASK_ALL	);
    NANDC_SET_REG_BITS (cluster->operate,         OP_READ_PAGE);

    /*轮询方式等待nandflash的操作完成*/
    errorcode = nandc_ctrl_read_page_result(host);
    if(NANDC_OK != errorcode)
    {
        goto EXIT;
    }
    
#if( FEATURE_ON == MBB_REB )
    /*软件可靠性:刷新位翻转信息*/
    if ( set_ecc_result_func )
    {
        set_ecc_result_func( host->regbase, ( unsigned int )( ECC_ERR_NUM_OFFSET ) );
    }
#endif

    return NANDC_OK;
EXIT:
    return errorcode;

}

/**
* 作用:nandc控制器写一页数据
*
* 参数:
* @host          		    --nandc的主机控制器指针
*
* 描述:写一整页的数据
*
*/
u32 nandc_ctrl_program_page(struct nandc_host *host)
{
	struct nandc_bit_cluster*   cluster;
	u32 errorcode;

	cluster = host->cluster;
    /*判断一下页面的大小是否超过了最大的buffer缓冲区*/
    if(host->mtd->writesize > NANDC_BUFSIZE_BASIC)
    {
        NANDC_TRACE(NFCDBGLVL(NORMAL), ("nandc_ctrl_program_page error, pagesize %x! \n",host->mtd->writesize));
        return NANDC_ERROR;
    }
	NANDC_SET_REG_BITS(cluster->int_enable,INTDIS_ALL); /* disable all interrupt */
	/*是否已经写了部分的硬件寄存器，如果没有写的话要写一次*/
	nandc_ctrl_read_write_eccconfig(host);
	NANDC_SET_REG_BITS (cluster->addr_low,      host->addr_physics[0]   );          /* nfc addrl */
	NANDC_SET_REG_BITS (cluster->addr_high,     host->addr_physics[1]   );          /* nfc addrh */
	NANDC_SET_REG_BITS (cluster->int_clr,       INTMASK_ALL	);          /* interupt clear */

	NANDC_SET_REG_BITS(cluster ->operate,    OP_WRITE_PAGE);

	errorcode   = nandc_ctrl_program_page_result(host);
	if(NANDC_OK != errorcode)
	{
		goto EXIT;
	}
	return NANDC_OK;
EXIT:
	return errorcode;
}

/*nand控制器命令的执行*/
u32 nandc_ctrl_excute_cmd(struct nandc_host *host)
{
    u32 errorcode = NANDC_ERROR ;

    /* coverity[printf_arg_mismatch] */
    NANDC_TRACE(NFCDBGLVL(NORMAL), ("nandc_ctrl_excute_cmd(%d),flash_addr(0x%x), low addr:0x%x, high addr:0x%x\n",
                                       host->command, (unsigned int)host->flash_addr, host->addr_physics[0],host->addr_physics[1]));
    switch (host->command)
    {
        case NAND_CMD_READSTART:
            errorcode = nandc_ctrl_read_page(host);
        break;

        case NAND_CMD_READID:
            errorcode = nandc_ctrl_read_id(host);
        break;

        case NAND_CMD_STATUS:
            errorcode = nandc_ctrl_read_excutecmd_status(host);
        break;

        case NAND_CMD_PAGEPROG:
            errorcode = nandc_ctrl_program_page(host);
        break;

        case NAND_CMD_ERASE2:
            errorcode = nandc_ctrl_erase_block(host);
        break;

        case NAND_CMD_RESET:
            errorcode = nandc_ctrl_reset(host);
        break;

        case NAND_CMD_SEQIN:
        case NAND_CMD_ERASE1:
        case NAND_CMD_READ0:
        break;

        default :
            errorcode = NANDC_ERROR;
            NANDC_TRACE(NFCDBGLVL(NORMAL), ("unknown nand command %x!", host->command));
        break;
    }

    return errorcode;
}

