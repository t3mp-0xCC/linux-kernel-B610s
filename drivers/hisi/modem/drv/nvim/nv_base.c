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




/*lint -save -e537*/
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <osl_thread.h>
#include <bsp_nvim.h>
#include <bsp_onoff.h>
#include "nv_comm.h"
#include "nv_file.h"
#include "nv_ctrl.h"
#include "nv_xml_dec.h"
#include "nv_debug.h"
#include "NVIM_ResumeId.h"
#include "bsp_dump.h"

#include "mbb_leds.h"


/*lint -restore +e537*/


#include "product_nv_id.h"
#include "bsp_sram.h"



#define FACTORY_INFO_LEN              (78)
#define MMI_TEST_FLAG_OFFSET          (24)
#define MMI_TEST_FLAG_LEN             (4)
#define NV_ID_MSP_FACTORY_INFO        (114)

#define SRAM_REBOOT_ADDR  (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_REBOOT_INFO)

#include <power_com.h>  /*fastboot引用的异常信息结构体必备头文件*/
#include "bsp_sram.h"
#include <linux/mtd/flash_huawei_dload.h>

typedef enum
{
    NV_RESTRORE_SUCCESS,
    NV_RESTRORE_FAIL,
}NV_RESTORE_STATUS;
bool nv_isSecListNv(u16 itemid)
{
    /*lint -save -e958*/
    u16 i = 0;

    /*lint -restore*/
    for(i = 0;i < bsp_nvm_getRevertNum(NV_SECURE_ITEM);i++)
    {
        if(itemid == g_ausNvResumeSecureIdList[i])
        {
            return true;
        }
    }
    return false;
}





bool nv_isAutoBakupListNv(u16 itemid)
{
    /*lint -save -e958*/
    u16 i = 0;

    /*lint -restore*/
    for(i = 0;i < bsp_nvm_getRevertNum(NV_MBB_AUTOBACKEUP_ITEM);i++)
    {
        if(itemid == g_ausNvAutoBakeupIdList[i])
        {
            return true;
        }
    }
    return false;
}



/*lint -save -e713 -e830*/
u32 nv_readEx(u32 modem_id,u32 itemid,u32 offset,u8* pdata,u32 datalen)
{
    u32 ret = NV_ERROR;
    struct nv_file_list_info_stru file_info;
    struct nv_ref_data_info_stru  ref_info;
    confidential_nv_opr_info *smem_data = NULL;
    smem_data = (confidential_nv_opr_info *)SRAM_CONFIDENTIAL_NV_OPR_ADDR;
    nv_debug(NV_FUN_READ_EX,0,itemid,modem_id,datalen);

    if((NULL == pdata)||(0 == datalen))
    {
        nv_debug(NV_FUN_READ_EX,1,itemid,0,0);
        return BSP_ERR_NV_INVALID_PARAM;
    }

    if(NV_ID_DRV_VER_FLAG == itemid)
    {
        printk(KERN_ERR "read itemid = %d\n",itemid);
        memset(pdata,0x00,datalen);
        *pdata = 1;
        return NV_OK;
    }

    /*如果是datalock密码或simlock密码NV,在未授权时禁止读取*/
    if ( (NV_HUAWEI_OEMLOCK_I == itemid) || (NV_HUAWEI_SIMLOCK_I == itemid) )
    {
        if (NULL == smem_data)
        {
            printk(KERN_ERR "smem_confidential_nv_opr_flag malloc fail!\n");
            return BSP_ERR_NV_NO_THIS_ID;
        }
        if(SMEM_CONFIDENTIAL_NV_OPR_FLAG_NUM == smem_data->smem_confidential_nv_opr_flag)
        {
            /*机要NV读取后，取消授权*/
            smem_data->smem_confidential_nv_opr_flag = SRAM_CONFIDENTIAL_NV_OPR_FLAG_CLEAR;
        }
        else
        {
            printk(KERN_ERR  "smem_confidential_nv_opr_flag invalid!\n");
            return BSP_ERR_NV_NO_THIS_ID;
        }
    }
    ret = nv_search_byid(itemid,((u8*)NV_GLOBAL_CTRL_INFO_ADDR),&ref_info,&file_info);
    if(ret)
    {
        nv_printf("\ncan not find 0x%x !\n",itemid);
        return BSP_ERR_NV_NO_THIS_ID;
    }

    if((offset + datalen) > ref_info.nv_len)
    {
        ret = BSP_ERR_NV_ITEM_LEN_ERR;
        nv_debug(NV_FUN_READ_EX,3,offset,datalen,ref_info.nv_len);
        goto nv_readEx_err;
    }
    if(modem_id <= ref_info.modem_num)
    {
        ref_info.nv_off += (modem_id - NV_USIMM_CARD_1)*ref_info.nv_len;
    }
    else
    {
        ret = BSP_ERR_NV_INVALID_PARAM;
        nv_debug(NV_FUN_READ_EX,4,ret,itemid,modem_id);
        goto nv_readEx_err;
    }

    ret = nv_read_from_mem(pdata, datalen,file_info.file_id,(ref_info.nv_off+offset));
    if(ret)
    {
        nv_debug(NV_FUN_READ_EX,5,offset,datalen,ref_info.nv_len);
        goto nv_readEx_err;
    }
    /*lint -save -e578 -e530*/
    nv_debug_trace(pdata, datalen);
    /*lint -restore +e578 +e530*/

    return NV_OK;
nv_readEx_err:
    nv_mntn_record("\n[%s]:[0x%x]:[%d]\n",__FUNCTION__,itemid,modem_id);
    nv_help(NV_FUN_READ_EX);
    return ret;
}

u32 nv_writeEx(u32 modem_id,u32 itemid,u32 offset,u8* pdata,u32 datalen)
{
    u32 ret = NV_ERROR;
    struct nv_file_list_info_stru file_info;
    struct nv_ref_data_info_stru  ref_info;
    u32 nv_offset = 0;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    u8  test_byte;
    confidential_nv_opr_info *smem_data = NULL;
    smem_data = (confidential_nv_opr_info *)SRAM_CONFIDENTIAL_NV_OPR_ADDR;

    nv_debug(NV_FUN_WRITE_EX,0,itemid,modem_id,datalen);
    nv_debug_record(NV_DEBUG_WRITEEX_START|itemid<<16);

    if((NULL == pdata)||(0 == datalen))
    {
        nv_debug(NV_FUN_WRITE_EX,1,itemid,datalen,0);
        return BSP_ERR_NV_INVALID_PARAM;
    }

    if(NV_ID_DRV_VER_FLAG == itemid)
    {
        printk(KERN_ERR "write itemid = %d\n",itemid);
        return NV_ERROR;
    }

    /* test pdata is accessable */
    test_byte = *pdata;
    UNUSED(test_byte);

    /*如果是datalock密码或simlock密码NV禁止写入*/
    if ( (NV_HUAWEI_OEMLOCK_I == itemid) || (NV_HUAWEI_SIMLOCK_I == itemid) )
    {
        if (NULL == smem_data)
        {
            printk("smem_confidential_nv_opr_flag malloc fail!\n");
            return BSP_ERR_NV_NO_THIS_ID;
        }

        if (SMEM_CONFIDENTIAL_NV_OPR_FLAG_NUM == smem_data->smem_confidential_nv_opr_flag)
        {
            /*机要NV操作后，取消授权*/
            smem_data->smem_confidential_nv_opr_flag = SRAM_CONFIDENTIAL_NV_OPR_FLAG_CLEAR;
        }
        else
        {
            printk("smem_confidential_nv_opr_flag invalid!\n");
            return BSP_ERR_NV_NO_THIS_ID;
        }
    }

    ret = nv_search_byid(itemid,((u8*)NV_GLOBAL_CTRL_INFO_ADDR),&ref_info,&file_info);
    if(ret)
    {

        nv_printf("\ncan not find 0x%x !\n",itemid);
        return BSP_ERR_NV_NO_THIS_ID;
    }

    /*lint -save -e578 -e530*/
    nv_debug_trace(pdata, datalen);
    /*lint -restore +e578 +e530*/

    if((datalen + offset) >ref_info.nv_len)
    {
        ret = BSP_ERR_NV_ITEM_LEN_ERR;
        nv_debug(NV_FUN_WRITE_EX,3,itemid,datalen,ref_info.nv_len);
        goto nv_writeEx_err;
    }
    if(modem_id <= ref_info.modem_num)
    {
        ref_info.nv_off += (modem_id - NV_USIMM_CARD_1)*ref_info.nv_len;
    }
    else
    {
        ret = BSP_ERR_NV_INVALID_PARAM;
        nv_debug(NV_FUN_WRITE_EX,4,itemid,ret,modem_id);
        goto nv_writeEx_err;
    }

    nv_offset = ddr_info->file_info[file_info.file_id -1].offset+offset+ref_info.nv_off;

    /*IPC锁保护，防止在校验CRC时写NV操作还没有完成*/
    nv_debug_record(NV_DEBUG_WRITEEX_GET_IPC_START);
    ret = nv_ipc_sem_take(IPC_SEM_NV_CRC, IPC_SME_TIME_OUT);
    if(ret)
    {
        nv_debug(NV_FUN_WRITE_EX, 10, itemid, ret,0);
        goto nv_writeEx_err;
    }
    nv_debug_record(NV_DEBUG_WRITEEX_GET_IPC_END);
    ret = nv_check_nv_data_crc(nv_offset, datalen);
    (void)nv_ipc_sem_give(IPC_SEM_NV_CRC);
    nv_debug_record(NV_DEBUG_WRITEEX_GIVE_IPC);
    if(ret)
    {
        nv_debug(NV_FUN_WRITE_EX, 5, itemid,datalen,0);
        ret = nv_resume_ddr_from_img();
        if(ret)
        {
            nv_debug(NV_FUN_WRITE_EX,6, itemid, ret, modem_id);
            goto nv_writeEx_err;
        }
    }
    nv_debug_record(NV_DEBUG_WRITEEX_MEM_START);
    ret = nv_write_to_mem(pdata,datalen,file_info.file_id,ref_info.nv_off+offset);
    if(ret)
    {
        nv_debug(NV_FUN_WRITE_EX,7,itemid,datalen,0);
        goto nv_writeEx_err;
    }

    nv_debug_record(NV_DEBUG_WRITEEX_FILE_START);

    ret = nv_write_to_file(&ref_info);
    if(ret)
    {
        nv_debug(NV_FUN_WRITE_EX,8,itemid,datalen,ret);
        goto nv_writeEx_err;
    }
    nv_debug_record(NV_DEBUG_WRITEEX_END|itemid<<16);
    return NV_OK;
nv_writeEx_err:
    nv_mntn_record("\n[%s]:[0x%x]:[%d]\n",__FUNCTION__,itemid,modem_id);
    nv_help(NV_FUN_WRITE_EX);
    return ret;
}

u32 bsp_nvm_get_nv_num(void)
{
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    return ctrl_info->ref_count;
}

u32 bsp_nvm_get_nvidlist(NV_LIST_INFO_STRU*  nvlist)
{
    u32 i;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_ref_data_info_stru* ref_info   = (struct nv_ref_data_info_stru*)(NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE\
        +NV_GLOBAL_FILE_ELEMENT_SIZE*ctrl_info->file_num);

    if(NULL == nvlist)
    {
        return NV_ERROR;
    }

    for(i = 0;i<ctrl_info->ref_count;i++)
    {
        nvlist[i].usNvId       = ref_info[i].itemid;
        nvlist[i].ucNvModemNum = ref_info[i].modem_num;
    }
    return NV_OK;
}

u32 bsp_nvm_get_len(u32 itemid,u32* len)
{
    u32 ret  = NV_ERROR;
    struct nv_ref_data_info_stru ref_info = {0};
    struct nv_file_list_info_stru file_info = {0};

    nv_debug(NV_API_GETLEN,0,itemid,0,0);
    if(NULL == len)
    {
        nv_debug(NV_API_GETLEN,1,itemid,0,0);
        return BSP_ERR_NV_INVALID_PARAM;
    }

    /*check init state*/
    if(false == nv_read_right(itemid))
    {
        nv_debug(NV_API_GETLEN,3,itemid,0,0);
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }
    ret = nv_search_byid(itemid,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&ref_info, &file_info);
    if(NV_OK == ret)
    {
        *len = ref_info.nv_len;
        return NV_OK;
    }
    return ret;
}

u32 bsp_nvm_authgetlen(u32 itemid,u32* len)
{
    return bsp_nvm_get_len(itemid,len);
}



u32 bsp_nvm_dcread_direct(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    return nv_readEx(modem_id,itemid,0,pdata,datalen);
}

u32 bsp_nvm_dcread(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    /*check init state*/
    if(false == nv_read_right(itemid))
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_readEx(modem_id,itemid,0,pdata,datalen);
}

u32 bsp_nvm_auth_dcread(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    return bsp_nvm_dcread(modem_id,itemid,pdata,datalen);
}

u32 bsp_nvm_dcreadpart(u32 modem_id,u32 itemid,u32 offset,u8* pdata,u32 datalen)
{
    /*check init state*/
    if(false == nv_read_right(itemid))
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_readEx(modem_id,itemid,offset,pdata,datalen);
}

u32 bsp_nvm_dcwrite(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    if(false == nv_write_right(itemid))
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_writeEx(modem_id,itemid,0,pdata,datalen);
}

u32 bsp_nvm_auth_dcwrite(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    return bsp_nvm_dcwrite(modem_id,itemid,pdata,datalen);
}

u32 bsp_nvm_dcwritepart(u32 modem_id,u32 itemid, u32 offset,u8* pdata,u32 datalen)
{
    if(false == nv_write_right(itemid))
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_writeEx(modem_id,itemid,offset,pdata,datalen);
}

u32 bsp_nvm_dcwrite_direct(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    return nv_writeEx(modem_id,itemid,0,pdata,datalen);
}



/*lint -save -e529*/
u32 bsp_nvm_flushEx(u32 off,u32 len,u32 itemid)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    u32 crc_count = 0;
    u32 skip_crc_count = 0;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    u32 crc_value_off = 0;
    u32 crc_value_len = 0;

    nv_debug(NV_API_FLUSH,0,off,len,itemid);
    nv_debug_record(NV_DEBUG_FLUSHEX_START);

    if((off + len) > (ddr_info->file_len))
    {
        nv_debug(NV_API_FLUSH,1,off,len,ddr_info->file_len);
        goto nv_flush_err;
    }

    nv_debug_record(NV_DEBUG_FLUSHEX_OPEN_START);

    if(nv_file_access((s8*)NV_IMG_PATH,0))
    {
        fp = nv_file_open((s8*)NV_IMG_PATH,(s8*)NV_FILE_WRITE);
        if(NULL == fp)
        {
            nv_debug(NV_API_FLUSH, 6, ret,0,0);
            goto nv_flush_err;
        }
        nv_file_close(fp);
        return bsp_nvm_flushEn(NV_FLAG_NEED_CRC);
    }
    else
    {
        fp = nv_file_open((s8*)NV_IMG_PATH,(s8*)NV_FILE_RW);
        if(NULL == fp)
        {
            ret = BSP_ERR_NV_NO_FILE;
            nv_debug(NV_API_FLUSH,2,ret,0,0);
            goto nv_flush_err;
        }
    }
    nv_debug_record(NV_DEBUG_FLUSHEX_OPEN_END);

    /*如果支持CRC校验码则需要将CRC校验码写入文件*/
    if(NV_DATA_CRC_CHECK_YES)
    {
        nv_debug_record(NV_DEBUG_FLUSHEX_GET_PROTECT_SEM_START);
        osl_sem_down(&g_nv_ctrl.nv_buf_sem);
        ret = nv_ipc_sem_take(IPC_SEM_NV_CRC, IPC_SME_TIME_OUT);
        if(ret)
        {
            nv_debug(NV_API_FLUSH,8,ret,0,0);
            nv_file_close(fp);
            osl_sem_up(&g_nv_ctrl.nv_buf_sem);
            goto nv_flush_err;
        }
        nv_debug_record(NV_DEBUG_FLUSHEX_GET_PROTECT_SEM_END);

        crc_count = NV_CRC_CODE_COUNT((off + len - ctrl_info->ctrl_size)) - (off- ctrl_info->ctrl_size)/NV_CRC32_CHECK_SIZE;
        skip_crc_count = (off - ctrl_info->ctrl_size)/NV_CRC32_CHECK_SIZE;
        len = crc_count * NV_CRC32_CHECK_SIZE;
        if(off + len > ddr_info->file_len)
        {
            len = len - (off + len - ddr_info->file_len);
        }
        off = ctrl_info->ctrl_size + (skip_crc_count)*NV_CRC32_CHECK_SIZE;
        memcpy(g_nv_ctrl.p_nv_data_buf, (u8*)NV_GLOBAL_CTRL_INFO_ADDR + off, len);

        crc_value_off = ddr_info->file_len + (skip_crc_count + 1)* sizeof(u32);
        crc_value_len = crc_count * sizeof(u32);
        memcpy(g_nv_ctrl.p_nv_crc_buf, NV_GLOBAL_CTRL_INFO_ADDR + crc_value_off, crc_value_len);

        (void)nv_ipc_sem_give(IPC_SEM_NV_CRC);
        nv_debug_record(NV_DEBUG_FLUSHEX_GIVE_IPC);


        nv_debug_record(NV_DEBUG_FLUSHEX_WRITE_FILE_START);
        (void)nv_file_seek(fp, off ,SEEK_SET);/*jump to write*/
        ret = (u32)nv_file_write((u8*)g_nv_ctrl.p_nv_data_buf,1,len,fp);
        if(ret != len)
        {
            nv_file_close(fp);
            nv_debug(NV_API_FLUSH,3, off , ret,len);
            ret = BSP_ERR_NV_WRITE_FILE_FAIL;
            osl_sem_up(&g_nv_ctrl.nv_buf_sem);
            goto nv_flush_err;
        }

        (void)nv_file_seek(fp, crc_value_off ,SEEK_SET);/*jump to write*/
        ret = (u32)nv_file_write((u8*)g_nv_ctrl.p_nv_crc_buf, 1, crc_value_len, fp);
        if(ret != crc_value_len)
        {
            nv_debug(NV_API_FLUSH,4, crc_value_off, ret,crc_value_len);
            ret = BSP_ERR_NV_WRITE_FILE_FAIL;
            nv_file_close(fp);
            osl_sem_up(&g_nv_ctrl.nv_buf_sem);
            goto nv_flush_err;
        }
        nv_debug_record(NV_DEBUG_FLUSHEX_WRITE_FILE_END);

        osl_sem_up(&g_nv_ctrl.nv_buf_sem);
        nv_debug_record(NV_DEBUG_FLUSHEX_GIVE_SEM);
    }
    else
    {
        (void)nv_file_seek(fp,(s32)off,SEEK_SET);/*jump to write*/
        ret = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR+off,1,len,fp);
        if(ret != len)
        {
            nv_file_close(fp);
            nv_debug(NV_API_FLUSH,5,0,ret,len);
            ret = BSP_ERR_NV_WRITE_FILE_FAIL;
            goto nv_flush_err;
        }
    }


    nv_file_close(fp);

    return NV_OK;

nv_flush_err:
    nv_mntn_record("\n[%s] len :0x%x, off :0x%x\n",__FUNCTION__,len,off);
    nv_help(NV_API_FLUSH);
    return ret;
}
/*lint -restore +e529*/

u32 bsp_nvm_flush(void)
{
    u32 ret;

    ret = nv_flushList();
    if(ret)
    {
        return ret;
    }
    return NV_OK;
}

u32 bsp_nvm_flushEn(u32 crc_flag)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    u32 writeLen = 0;

    nv_create_flag_file((s8*)NV_IMG_FLAG_PATH);
    nv_debug(NV_API_FLUSH,0,0,0,0);
    fp = nv_file_open((s8*)NV_IMG_PATH,(s8*)NV_FILE_WRITE);
    if(NULL == fp)
    {
        ret = BSP_ERR_NV_NO_FILE;
        nv_debug(NV_API_FLUSH,1,ret,0,0);
        goto nv_flush_err;
    }

    writeLen = ddr_info->file_len;

    if(NV_DATA_CRC_CHECK_YES)
    {
        writeLen += NV_CRC_CODE_COUNT(ddr_info->file_len - ctrl_info->ctrl_size)*sizeof(u32) \
                + sizeof(u32);/*sizeof是文件尾结束符*/
    }
    if(NV_CTRL_CRC_CHECK_YES)
    {
        writeLen += sizeof(u32);/*ctrl段CRC校验码*/
    }

    ret = nv_ipc_sem_take(IPC_SEM_NV_CRC, IPC_SME_TIME_OUT);
    if(ret)
    {
        nv_file_close(fp);
        nv_debug(NV_API_FLUSH, 2, ret, 0, 0);
        goto nv_flush_err;
    }
    /*如果需要进行CRC校验*/
    if(NV_FLAG_NEED_CRC == crc_flag)
    {
        ret = nv_check_ddr_crc();
        if(ret)
        {
            nv_file_close(fp);
            (void)nv_ipc_sem_give(IPC_SEM_NV_CRC);
            nv_debug(NV_API_FLUSH, 4, ret, 0, 0);
            (void)nv_debug_store_ddr_data();
            goto nv_flush_err;
        }
    }
    ret = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR,1, writeLen, fp);
    (void)nv_ipc_sem_give(IPC_SEM_NV_CRC);
    nv_file_close(fp);
    fp = NULL;
    if(ret != writeLen)
    {
        nv_debug(NV_API_FLUSH,5,(u32)(unsigned long)fp,ret,writeLen);
        ret = BSP_ERR_NV_WRITE_FILE_FAIL;
        goto nv_flush_err;
    }
    nv_delete_flag_file((s8*)NV_IMG_FLAG_PATH);

    return NV_OK;

nv_flush_err:
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_API_FLUSH);
    return ret;
}


u32 bsp_nvm_flushSys(u32 itemid)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    u32 ulTotalLen = 0;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    nv_create_flag_file((s8*)NV_SYS_FLAG_PATH);

    nv_debug(NV_FUN_FLUSH_SYS,0,0,0,0);
    if(nv_file_access((s8*)NV_FILE_SYS_NV_PATH,0))
    {
        fp = nv_file_open((s8*)NV_FILE_SYS_NV_PATH,(s8*)NV_FILE_WRITE);
    }
    else
    {
        fp = nv_file_open((s8*)NV_FILE_SYS_NV_PATH,(s8*)NV_FILE_RW);
    }
    if(NULL == fp)
    {
        nv_debug(NV_FUN_FLUSH_SYS,1,ret,0,0);
        ret = BSP_ERR_NV_NO_FILE;
        goto nv_flush_err;
    }
    ulTotalLen = ddr_info->file_len;
    /*在nvdload分区文件末尾置标志0xabcd8765*/
    *( unsigned int* )( (u8*)NV_GLOBAL_CTRL_INFO_ADDR + ddr_info->file_len )
        = ( unsigned int )NV_FILE_TAIL_MAGIC_NUM;
    ulTotalLen += sizeof(unsigned int);
    /*系统分区数据不做CRC校验，因此回写时不考虑CRC校验码的存放位置*/
    ret = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR,1,ulTotalLen,fp);
    nv_file_close(fp);
    if(ret != ulTotalLen)
    {
        nv_debug(NV_FUN_FLUSH_SYS,3,ret,ulTotalLen,0);
        ret = BSP_ERR_NV_WRITE_FILE_FAIL;
        goto nv_flush_err;
    }

    nv_delete_flag_file((s8*)NV_SYS_FLAG_PATH);
    return NV_OK;

nv_flush_err:
    nv_mntn_record("\n[%s]\n",__func__);
    nv_help(NV_FUN_FLUSH_SYS);
    return ret;
}



u32 bsp_nvm_backup(u32 crc_flag)
{
    u32 ret = NV_ERROR;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    FILE* fp = NULL;
    u32 writeLen = 0;
    nv_debug(NV_API_BACKUP,0,0,0,0);

    if( (ddr_info->acore_init_state != NV_INIT_OK)&&
        (ddr_info->acore_init_state != NV_KERNEL_INIT_DOING))
    {
        return NV_ERROR;
    }

    nv_create_flag_file((s8*)NV_BACK_FLAG_PATH);

    if(nv_file_access((s8*)NV_BACK_PATH,0))
    {
        fp = nv_file_open((s8*)NV_BACK_PATH,(s8*)NV_FILE_WRITE);
    }
    else
    {
        fp = nv_file_open((s8*)NV_BACK_PATH,(s8*)NV_FILE_RW);
    }
    if(NULL == fp)
    {
        ret = BSP_ERR_NV_NO_FILE;
        nv_debug(NV_API_BACKUP,1,ret,0,0);
        goto nv_backup_fail;
    }

    writeLen = ddr_info->file_len;
    if(NV_DATA_CRC_CHECK_YES)
    {
        writeLen +=  NV_CRC_CODE_COUNT(ddr_info->file_len - ctrl_info->ctrl_size)*sizeof(u32) + sizeof(u32);
    }
    if(NV_CTRL_CRC_CHECK_YES)
    {
        writeLen += sizeof(u32);/*sizeof是文件尾结束符和ctrl段CRC校验码*/
    }

    ret = nv_ipc_sem_take(IPC_SEM_NV_CRC, IPC_SME_TIME_OUT);
    if(ret)
    {
        nv_debug(NV_API_BACKUP,8,ret,0,0);
        goto nv_backup_fail;
    }
    /*如果需要进行CRC校验*/
    if(NV_FLAG_NEED_CRC == crc_flag)
    {
        ret = nv_check_ddr_crc();
        if(ret)
        {
            (void)nv_ipc_sem_give(IPC_SEM_NV_CRC);
            nv_debug(NV_API_BACKUP,2,ret,0, 0);
            (void)nv_debug_store_ddr_data();
            goto nv_backup_fail;
        }
    }
    ret = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR,1,writeLen,fp);
    (void)nv_ipc_sem_give(IPC_SEM_NV_CRC);
    nv_file_close(fp);
    fp = NULL;
    if(ret != writeLen)
    {
        nv_debug(NV_API_BACKUP,3,ret,writeLen,0);
        ret = BSP_ERR_NV_WRITE_FILE_FAIL;
        goto nv_backup_fail;
    }

    if(nv_file_update(NV_BACK_PATH))
    {
        nv_debug(NV_API_BACKUP, 4 , 0, 0, 0);
    }

    nv_delete_flag_file((s8*)NV_BACK_FLAG_PATH);

    return NV_OK;
nv_backup_fail:
    if(fp){nv_file_close(fp);}
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_API_BACKUP);
    return ret;

}

u32 bsp_nvm_revert_user(void)
{
    return nv_revert_data(NV_BACK_PATH,g_ausNvResumeUserIdList,\
        bsp_nvm_getRevertNum(NV_USER_ITEM), NV_FLAG_NEED_CRC);
}


u32 bsp_nvm_revert_manufacture(void)
{
    return nv_revert_data(NV_BACK_PATH,g_ausNvResumeManufactureIdList,\
        bsp_nvm_getRevertNum(NV_MANUFACTURE_ITEM), NV_FLAG_NEED_CRC);
}

u32 bsp_nvm_revert_secure(void)
{
    return nv_revert_data(NV_BACK_PATH,g_ausNvResumeSecureIdList,\
        bsp_nvm_getRevertNum(NV_SECURE_ITEM), NV_FLAG_NEED_CRC);
}



u32 bsp_nvm_revert(void)
{
    u32 ret  = NV_ERROR;

    nv_debug(NV_API_REVERT,0,0,0,0);

    nv_printf("enter to revert nv !\n");
    ret = bsp_nvm_revert_user();
    if(ret)
    {
        nv_debug(NV_API_REVERT,1,ret,0,0);
        goto nv_revert_fail;
    }

    ret = bsp_nvm_revert_manufacture();
    if(ret)
    {
        nv_debug(NV_API_REVERT,2,ret,0,0);
        goto nv_revert_fail;
    }
    ret = bsp_nvm_revert_secure();
    if(ret)
    {
        nv_debug(NV_API_REVERT,3,ret,0,0);
        goto nv_revert_fail;
    }
    nv_printf("revert nv end !\n");

    return NV_OK;
nv_revert_fail:
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_API_REVERT);
    return ret;
}




u32 bsp_nvm_update_default(void)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    u32 datalen = 0;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    NV_SELF_CTRL_STRU self_ctrl = {};

    nv_debug(NV_FUN_UPDATE_DEFAULT,0,0,0,0);

    if(ddr_info->acore_init_state != NV_INIT_OK)
    {
        return NV_ERROR;
    }


    ret = bsp_nvm_read(NV_ID_DRV_SELF_CTRL, (u8*)&self_ctrl, sizeof(NV_SELF_CTRL_STRU));
    if(ret)
    {
        nv_printf("read nv 0x%x fail,ret = 0x%x\n", NV_ID_DRV_SELF_CTRL);
        return NV_ERROR;
    }
    self_ctrl.ulResumeMode = NV_MODE_USER;
    ret = bsp_nvm_write(NV_ID_DRV_SELF_CTRL, (u8*)&self_ctrl, sizeof(NV_SELF_CTRL_STRU));
    if(ret)
    {
        nv_printf("write nv 0x%x fail,ret = 0x%x\n", NV_ID_DRV_SELF_CTRL);
        return NV_ERROR;
    }
    /*在写入文件前进行CRC校验，以防数据不正确*/
    ret = nv_ipc_sem_take(IPC_SEM_NV_CRC, IPC_SME_TIME_OUT);
    if(ret)
    {
        nv_debug(NV_FUN_UPDATE_DEFAULT,2,ret,ddr_info->file_len,0);
        goto nv_update_default_err;
    }
    ret = nv_check_ddr_crc();
    if(ret)
    {
        (void)nv_ipc_sem_give(IPC_SEM_NV_CRC);
        nv_debug(NV_FUN_UPDATE_DEFAULT,3,ret,ddr_info->file_len,0);
        (void)nv_debug_store_ddr_data();
        goto nv_update_default_err;
    }
    (void)nv_ipc_sem_give(IPC_SEM_NV_CRC);

    if(nv_file_access((s8*)NV_DEFAULT_PATH,0))
    {
        fp = nv_file_open((s8*)NV_DEFAULT_PATH,(s8*)NV_FILE_WRITE);
    }
    else
    {
        fp = nv_file_open((s8*)NV_DEFAULT_PATH,(s8*)NV_FILE_RW);
    }
    if(NULL == fp)
    {
        ret = BSP_ERR_NV_NO_FILE;
        nv_debug(NV_FUN_UPDATE_DEFAULT,2,ret,0,0);
        goto nv_update_default_err;
    }
    nv_ipc_sem_take(IPC_SEM_NV_CRC, IPC_SME_TIME_OUT);
    datalen = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR,1,ddr_info->file_len,fp);
    (void)nv_ipc_sem_give(IPC_SEM_NV_CRC);

    nv_file_close(fp);
    if(datalen != ddr_info->file_len)
    {
        nv_debug(NV_FUN_UPDATE_DEFAULT,6,ret,ddr_info->file_len,0);
        ret = BSP_ERR_NV_WRITE_FILE_FAIL;
        goto nv_update_default_err;
    }

    ret = bsp_nvm_backup(NV_FLAG_NO_CRC);
    if(ret)
    {
        nv_debug(NV_FUN_UPDATE_DEFAULT,7,ret,0,0);
        goto nv_update_default_err;
    }


    return NV_OK;
nv_update_default_err:
    /* coverity[deref_arg] */
    if(fp){nv_file_close(fp);}
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_FUN_UPDATE_DEFAULT);
    return ret;
}


u32 bsp_nvm_revert_defaultEx(const s8* path)
{
    u32 ret = NV_ERROR;
    u32 i = 0;
    FILE* fp = NULL;
    struct nv_ctrl_file_info_stru  manu_ctrl_file = {0};
    u8* ctrl_file_data = NULL;
    struct nv_global_ddr_info_stru *manu_ddr_info = NULL;


    nv_debug(NV_FUN_REVERT_DEFAULT,0,0,0,0);
    fp = nv_file_open((s8*)path,(s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        nv_debug(NV_FUN_REVERT_DEFAULT,1,0,0,0);
        return BSP_ERR_NV_NO_FILE;
    }

    ret = (u32)nv_file_read((u8*)&manu_ctrl_file,1,sizeof(manu_ctrl_file),fp);
    if(ret != sizeof(manu_ctrl_file))
    {
        nv_debug(NV_FUN_REVERT_DEFAULT,2,ret,0,0);
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto file_close_err;
    }
    nv_file_seek(fp,0,SEEK_SET);

    if(manu_ctrl_file.ctrl_size > NV_MAX_FILE_SIZE)
    {
        nv_debug(NV_FUN_REVERT_DEFAULT,30,BSP_ERR_NV_INVALID_PARAM,0,0);
        nv_printf("ctrl size is:0x%x too large\n", manu_ctrl_file.ctrl_size);
        ret = BSP_ERR_NV_INVALID_PARAM;
        goto file_close_err;
    }

    ctrl_file_data = (u8*)vmalloc(manu_ctrl_file.ctrl_size+1);
    if(NULL == ctrl_file_data)
    {
        nv_debug(NV_FUN_REVERT_DEFAULT,3,BSP_ERR_NV_MALLOC_FAIL,0,0);
        ret = BSP_ERR_NV_MALLOC_FAIL;
        goto file_close_err;
    }

    ret = (u32)nv_file_read(ctrl_file_data,1,manu_ctrl_file.ctrl_size,fp);
    if(ret != manu_ctrl_file.ctrl_size)
    {
        nv_error_printf("ret 0x%x,ctrl size 0x%x\n",ret,manu_ctrl_file.ctrl_size);
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        nv_debug(NV_FUN_REVERT_DEFAULT,4,ret,manu_ctrl_file.ctrl_size,0);
        goto free_ctrl_file;
    }
    manu_ddr_info = (struct nv_global_ddr_info_stru *)nv_malloc(sizeof(struct nv_global_ddr_info_stru));
    if(!manu_ddr_info)
    {
        ret = BSP_ERR_NV_MALLOC_FAIL;
        nv_debug(NV_FUN_REVERT_DEFAULT,5,ret,manu_ctrl_file.ctrl_size,0);
        goto free_ctrl_file;
    }
    ret = nv_init_file_info((u8*)ctrl_file_data,(u8*)manu_ddr_info);
    if(ret)
    {
        nv_debug(NV_FUN_REVERT_DEFAULT,6,ret,0,0);
        ret = BSP_ERR_NV_MEM_INIT_FAIL;
        goto free_ctrl_file;
    }
    for(i = 0;i<manu_ctrl_file.file_num;i++)
    {
        ret = nv_revert_default(fp,manu_ddr_info->file_info[i].size);
        if(ret)
        {
            nv_debug(NV_FUN_REVERT_DEFAULT,7,ret,manu_ddr_info->file_info[i].size,0);
            goto free_ctrl_file;
        }
    }

    if(fp){nv_file_close(fp);}
    if(ctrl_file_data)vfree(ctrl_file_data);
    if(manu_ddr_info){nv_free((void *)manu_ddr_info);}

    return NV_OK;
free_ctrl_file:
    if(ctrl_file_data){vfree(ctrl_file_data);}
    if(manu_ddr_info){nv_free((void *)manu_ddr_info);}
file_close_err:
    nv_file_close(fp);
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_FUN_REVERT_DEFAULT);
    return ret;
}


u32 bsp_nvm_revert_default(void)
{
    u32 ret = NV_ERROR;

    printf("enter to set default nv !\r\n");
    
    ret = nv_revert_data(NV_DEFAULT_PATH,g_ausNvResumeDefualtIdList,\
             bsp_nvm_getRevertNum(NV_MBB_DEFUALT_ITEM), NV_FLAG_NEED_CRC);
    if(ret)
    {
        printf("Set default nv nv_revert_data_with_crc error!\r\n");
        goto err_out;
    }
    
    ret = bsp_nvm_flushEn(NV_FLAG_NEED_CRC);
    if(NV_OK != ret)
    {
        nv_error_printf("write back to [img] failed! ret = 0x%x.\n", ret);
        goto err_out;
    }

    ret = bsp_nvm_flushSys(NV_ERROR);
    if(NV_OK != ret)
    {
        nv_error_printf("write back to [sys] failed! ret = 0x%x.\n", ret);
    }

err_out:
    return ret;
}



/*lint -save -e438*/
u32 bsp_nvm_key_check(void)
{
    FILE* fp = NULL;
    u32 ret = NV_ERROR;
    u32 datalen = 0;        /*read file len*/
    u32 file_offset = 0;
    u8* bak_ctrl_file = NULL;
    u8* bak_data = NULL;           /*single nv data ,max len 2048byte*/
    u8* mem_data = NULL;
    struct nv_ctrl_file_info_stru    bak_ctrl_info = {0};   /*bak file ctrl file head*/
    struct nv_file_list_info_stru    bak_file_info  = {0};
    struct nv_global_ddr_info_stru   *bak_ddr_info   = NULL;
    struct nv_ref_data_info_stru     bak_ref_info   = {0};

    struct nv_ref_data_info_stru    mem_ref_info  = {0};
    struct nv_file_list_info_stru   mem_file_info = {0};

    if(nv_file_access((s8*)NV_DEFAULT_PATH,0))  /*没有文件则直接返回ok*/
    {
        return NV_OK;
    }

    nv_debug(NV_FUN_KEY_CHECK,0,0,0,0);
    fp = nv_file_open((s8*)NV_DEFAULT_PATH,(s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        nv_debug(NV_FUN_KEY_CHECK,1,0,0,0);
        return BSP_ERR_NV_NO_FILE;
    }

    /*first read ctrl file head*/
    datalen = (u32)nv_file_read((u8*)(&bak_ctrl_info),1,sizeof(bak_ctrl_info),fp);
    if(datalen != sizeof(bak_ctrl_info))
    {
        nv_debug(NV_FUN_KEY_CHECK,2,datalen,0,0);
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto close_file;
    }

    if(bak_ctrl_info.magicnum != NV_CTRL_FILE_MAGIC_NUM)
    {
        nv_debug(NV_FUN_KEY_CHECK,3,bak_ctrl_info.magicnum,0,0);
        ret = BSP_ERR_NV_FILE_ERROR;
        goto close_file;
    }

    bak_ctrl_file = (u8*)vmalloc(bak_ctrl_info.ctrl_size);
    if(NULL == bak_ctrl_file)
    {
        nv_debug(NV_FUN_KEY_CHECK,4,bak_ctrl_info.ctrl_size,0,0);
        ret = BSP_ERR_NV_MALLOC_FAIL;
        goto close_file;
    }
    /*second :read all ctrl file*/
    nv_file_seek(fp,0,SEEK_SET); /*jump to file head*/
    ret = (u32)nv_file_read(bak_ctrl_file,1,bak_ctrl_info.ctrl_size,fp);
    if(ret != bak_ctrl_info.ctrl_size)
    {
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        nv_debug(NV_FUN_KEY_CHECK,5,ret,bak_ctrl_info.ctrl_size,0);
        goto free_ctrl_data;

    }
    /*third :init ctrl file info to bak_ddr_info*/
    bak_ddr_info = (struct nv_global_ddr_info_stru *)nv_malloc(sizeof(struct nv_global_ddr_info_stru));
    if(!bak_ddr_info)
    {
        ret = BSP_ERR_NV_MALLOC_FAIL;
        nv_debug(NV_FUN_KEY_CHECK,6,ret,bak_ctrl_info.ctrl_size,0);
        goto free_ctrl_data;
    }
    ret = nv_init_file_info((u8*)bak_ctrl_file,(u8*)bak_ddr_info);
    if(ret)
    {
        nv_debug(NV_FUN_KEY_CHECK,9,ret,0,0);
        goto free_ctrl_data;
    }

    /*forth :look for imei id in bak & cur mem*/
    ret = nv_search_byid(NV_ID_DRV_IMEI,bak_ctrl_file,&bak_ref_info,&bak_file_info);
    if(ret)
    {
        nv_debug(NV_FUN_KEY_CHECK,10,ret,0,0);
        goto free_ctrl_data;
    }
    ret = nv_search_byid(NV_ID_DRV_IMEI,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&mem_ref_info,&mem_file_info);
    if(ret)
    {
        nv_debug(NV_FUN_KEY_CHECK,13,ret,0,0);
        goto free_ctrl_data;
    }

    /*compare info*/
    if( (mem_ref_info.nv_len    != bak_ref_info.nv_len) ||
        (mem_ref_info.modem_num != bak_ref_info.modem_num)
        )
    {
        nv_debug(NV_FUN_KEY_CHECK,14,bak_ref_info.nv_len,bak_ref_info.modem_num,mem_ref_info.nv_len);
        ret = BSP_ERR_NV_FILE_ERROR;
        goto free_ctrl_data;
    }

    datalen = mem_ref_info.nv_len * mem_ref_info.modem_num;

    bak_data = (u8*)nv_malloc(datalen);
    mem_data = (u8*)nv_malloc(datalen);
    if((NULL == bak_data)||(mem_data == NULL))
    {
        nv_debug(NV_FUN_KEY_CHECK,15,0,0,0);
        goto free_data;
    }

    /*count data offset in bak file*/
    file_offset = bak_ddr_info->file_info[bak_file_info.file_id-1].offset +bak_ref_info.nv_off;

    nv_file_seek(fp,(s32)file_offset,SEEK_SET);
    ret = (u32)nv_file_read(bak_data,1,datalen,fp);/*把数据从文件中指定偏移处读出*/
    if(ret != datalen)
    {
        nv_debug(NV_FUN_KEY_CHECK,16,ret,datalen,0);
        goto free_data;
    }

    ret = nv_read_from_mem(mem_data,datalen,mem_file_info.file_id,mem_ref_info.nv_off);
    if(ret)
    {
        nv_debug(NV_FUN_KEY_CHECK,17,0,0,0);
        goto free_data;
    }

    nv_file_close(fp);
    ret = (u32)memcmp(mem_data,bak_data,datalen);  /*比较数据差异*/
    if(ret)
    {
        ret = bsp_nvm_revert_defaultEx((s8*)NV_DEFAULT_PATH);/* [false alarm]:ret is in using */

        ret |= bsp_nvm_flush();/* [false alarm]:ret is in using */
        ret |= bsp_nvm_flushSys(NV_ERROR);/* [false alarm]:ret is in using */
    }
    nv_free(mem_data);
    nv_free(bak_data);
    vfree(bak_ctrl_file);
    if(bak_ddr_info){nv_free(bak_ddr_info);}

    return NV_OK;
free_data:
    if(mem_data){nv_free(mem_data);}
    if(bak_data){nv_free(bak_data);}
free_ctrl_data:
    if(bak_ctrl_file){vfree(bak_ctrl_file);}
    if(bak_ddr_info){nv_free(bak_ddr_info);}
close_file:
    nv_file_close(fp);
    nv_mntn_record("\n%s\n",__func__);
    nv_help(NV_FUN_KEY_CHECK);
    return ret;
}
/*lint -restore -e747*/

s32 bsp_nvm_icc_task(void* parm)
{
    s32 ret = -1;
    struct nv_icc_stru icc_req;
    /* coverity[var_decl] */
    struct nv_icc_stru icc_cnf;
    u32 chanid = 0;


    /* coverity[no_escape] */
    for(;;)
    {
        osl_sem_down(&g_nv_ctrl.task_sem);

        g_nv_ctrl.opState = NV_OPS_STATE;
        wake_lock(&g_nv_ctrl.wake_lock);

        /*如果当前处于睡眠状态，则等待唤醒处理*/
        if(g_nv_ctrl.pmState == NV_SLEEP_STATE)
        {
            printk("%s cur state in sleeping,wait for resume end!\n",__func__);
            continue;
        }
        if(g_nv_ctrl.acore_flush_req)
        {
            /*deal with acore msg*/
            memset(&icc_req, 0, sizeof(icc_req));
            icc_req.msg_type = g_nv_ctrl.acore_flush_req;
            g_nv_ctrl.acore_flush_req = 0;
        }
        else
        {
            /*deal with ccore msg*/
            memset(g_nv_ctrl.nv_icc_buf,0,NV_ICC_BUF_LEN);
            memset(&icc_req,0,sizeof(icc_req));

            chanid = ICC_CHN_NV << 16 | NV_RECV_FUNC_AC;
            ret = bsp_icc_read(chanid,g_nv_ctrl.nv_icc_buf,NV_ICC_BUF_LEN);
            if(((u32)ret > NV_ICC_BUF_LEN)||(ret <= 0))
            {
                nv_debug_printf("bsp icc read error, chanid :0x%x ret :0x%x\n",chanid,ret);
                g_nv_ctrl.opState = NV_IDLE_STATE;
                wake_unlock(&g_nv_ctrl.wake_lock);
                continue;
            }
            memcpy(&icc_req,g_nv_ctrl.nv_icc_buf,sizeof(icc_req));
            /*lint -save -e578 -e530*/
            nv_debug_trace(&icc_req, sizeof(icc_req));
        }
        g_nv_ctrl.task_proc_count ++;

        /*lint -restore +e578 +e530*/
        if(icc_req.msg_type == NV_ICC_REQ_SYS)
        {
            icc_cnf.ret = bsp_nvm_flushSys(icc_req.itemid);
        }
        else if(icc_req.msg_type == NV_ICC_RESUME)
        {
            icc_cnf.ret = nv_resume_ddr_from_img();
        }
        else if(icc_req.msg_type == NV_ICC_REQ_INSTANT_FLUSH)
        {
            /**flush to flush no delay*/
            icc_cnf.ret = nv_flushList();

            /*如果工具侧通过核间通信要求写的是机要nv，则启动备份*/
            if(true == nv_isSecListNv(icc_req.itemid))
            {
                ret = bsp_nvm_backup(NV_FLAG_NO_CRC);
            }
            if(NV_ERROR == ret )
            {
                printf("nvm_backup icc Err!\n");
            }
        }
        else if((icc_req.msg_type == NV_ICC_REQ_CCORE_DELAY_FLUSH)||(icc_req.msg_type == NV_REQ_ACORE_DELAY_FLUSH))
        {
            /*flush to flash delay and no cnf*/
            icc_cnf.ret = nv_flushList();
            if(icc_cnf.ret)
            {
                nv_mntn_record("flush to nv file fail, fail info:0x%x\n", icc_cnf.ret);
                nv_mntn_record("req core:%s ret:0x%x, req slice 0x%x fail slice:0x%x\n", (icc_req.msg_type == NV_ICC_REQ_CCORE_DELAY_FLUSH)?"ccore":"acore",\
                    icc_cnf.ret, icc_req.slice, bsp_get_slice_value());
            }
            wake_unlock(&g_nv_ctrl.wake_lock);
            osl_sem_up(&g_nv_ctrl.task_sem);
            continue;
        }
        else
        {
            nv_printf("invalid parameter :0x%x\n",icc_req.msg_type);
            wake_unlock(&g_nv_ctrl.wake_lock);
            osl_sem_up(&g_nv_ctrl.task_sem);
            continue;
        }
        nv_pm_trace(icc_req);

        icc_cnf.msg_type = NV_ICC_CNF;
        icc_cnf.data_off = icc_req.data_off;
        icc_cnf.data_len = icc_req.data_len;
        icc_cnf.itemid   = icc_req.itemid;
        icc_cnf.slice    = bsp_get_slice_value();

        /* coverity[uninit_use_in_call] */
        ret = (s32)nv_icc_send(chanid,(u8*)&icc_cnf,sizeof(icc_cnf));
        if(ret)
        {
            nv_printf("icc send error !\n");
        }
        wake_unlock(&g_nv_ctrl.wake_lock);
        osl_sem_up(&g_nv_ctrl.task_sem);
    }
}


u32 bsp_nvm_xml_decode(void)
{
    u32 ret = NV_ERROR;

    if(!nv_file_access(NV_XNV_CARD1_PATH,0))
    {
        ret = nv_xml_decode(NV_XNV_CARD1_PATH,NV_XNV_CARD1_MAP_PATH,NV_USIMM_CARD_1);
        if(ret)
        {
            return ret;
        }
    }

    if(!nv_file_access(NV_XNV_CARD2_PATH,0))
    {
        ret = nv_xml_decode(NV_XNV_CARD2_PATH,NV_XNV_CARD2_MAP_PATH,NV_USIMM_CARD_2);
        if(ret)
        {
            return ret;
        }
    }

    if(!nv_file_access(NV_XNV_CARD3_PATH,0))
    {
        ret = nv_xml_decode(NV_XNV_CARD3_PATH, NV_XNV_CARD3_MAP_PATH, NV_USIMM_CARD_3);
        if(ret)
        {
            return ret;
        }
    }

    /*CUST XML 无对应MAP文件，传入空值即可*/
    if(!nv_file_access(NV_CUST_CARD1_PATH,0))
    {
        ret = nv_xml_decode(NV_CUST_CARD1_PATH,NULL,NV_USIMM_CARD_1);
        if(ret)
        {
            return ret;
        }
    }

    if(!nv_file_access(NV_CUST_CARD2_PATH,0))
    {
        ret = nv_xml_decode(NV_CUST_CARD2_PATH,NULL,NV_USIMM_CARD_2);
        if(ret)
        {
            return ret;
        }
    }

    if(!nv_file_access(NV_CUST_CARD3_PATH,0))
    {
        ret = nv_xml_decode(NV_CUST_CARD3_PATH, NULL, NV_USIMM_CARD_3);
        if(ret)
        {
            return ret;
        }
    }

    return NV_OK;
}

static NV_RESTORE_STATUS g_stuType = NV_RESTRORE_SUCCESS;
static struct task_struct *show_task = NULL;
#define LED_SHOW_TASK       "show_task"
#define LED_SHOW_WAIT_TIME  (1 * HZ / 2)

/* 组播升级，nv恢复成功、失败，点灯*/
int show_threadfunc(void)
{
    printf("nv module cpe led show, %d\n", g_stuType);
    while(1)
    {
        set_current_state(TASK_INTERRUPTIBLE);
        if(0 == led_kernel_init_status())
        {
            if(NV_RESTRORE_SUCCESS == g_stuType)
            {
                led_kernel_status_set(MODE_BLUE_LED, LEDS_OFF);
                led_kernel_status_set(MODE_GREEN_LED, LEDS_ON);
                led_kernel_status_set(MODE_RED_LED, LEDS_OFF);
                led_kernel_status_set(POWER_GREEN_LED, LEDS_OFF);
                led_kernel_status_set(SIGNAL1_WHITE_LED, LEDS_ON);
                led_kernel_status_set(SIGNAL1_RED_LED, LEDS_OFF);
                led_kernel_status_set(SIGNAL2_WHITE_LED, LEDS_OFF);
                led_kernel_status_set(SIGNAL3_WHITE_LED, LEDS_ON);
                led_kernel_status_set(LAN_WHITE_LED, LEDS_OFF);
                led_kernel_status_set(STATUS_GREEN_LED, LEDS_OFF);
                led_kernel_status_set(WIFI_WHITE_LED, LEDS_OFF);
            }
            else
            {
                led_kernel_status_set(MODE_BLUE_LED, LEDS_OFF);
                led_kernel_status_set(MODE_GREEN_LED, LEDS_OFF);
                led_kernel_status_set(MODE_RED_LED, LEDS_ON);
                led_kernel_status_set(POWER_GREEN_LED, LEDS_OFF);
                led_kernel_status_set(SIGNAL1_WHITE_LED, LEDS_ON);
                led_kernel_status_set(SIGNAL1_RED_LED, LEDS_OFF);
                led_kernel_status_set(SIGNAL2_WHITE_LED, LEDS_OFF);
                led_kernel_status_set(SIGNAL3_WHITE_LED, LEDS_ON);
                led_kernel_status_set(LAN_WHITE_LED, LEDS_OFF);
                led_kernel_status_set(STATUS_GREEN_LED, LEDS_OFF);
                led_kernel_status_set(WIFI_WHITE_LED, LEDS_OFF);
            }
            break;
        }
        printf("nv module start earlier, wait for 0.5s.\n");
        schedule_timeout(LED_SHOW_WAIT_TIME);
    }
    return 0;
}

/*
* Function   : bsp_nvm_led_show
* Discription: after NV Automatic recovery, 
               multiupg upgrade led show.
* Parameter  : NA
* Output     : result
*/
s32 bsp_nvm_led_show(void)
{
    if(NULL != show_task)
    {
        return 0;
    }
    
    show_task = kthread_create((int (*)(void *))show_threadfunc, NULL, LED_SHOW_TASK);
    if (!IS_ERR(show_task))
    {
        wake_up_process(show_task);
    }

    return 0;
}

/*
* Function   : bsp_nvm_led_show_stop
* Discription: stop led show
* Parameter  : NA
* Output     : 0
*/
s32 bsp_nvm_led_show_stop(void)
{
    if(NULL != show_task)
    {
        (void)kthread_stop(show_task);
        show_task = NULL;
    }
    
    return 0;
}


s32 bsp_nvm_restore_online_handle(NV_RESTORE_STATUS stuType)
{
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    
    if (NULL == smem_data)
    {
        printf("Dload smem_data malloc fail!\n");
        return -1;
    }

    printf("smem_data->smem_online_upgrade_flag :0x%x\n", 
                smem_data->smem_online_upgrade_flag);
    printf("smem_data->smem_multiupg_flag :0x%x\n", 
                smem_data->smem_multiupg_flag);
    if(SMEM_ONNR_FLAG_NUM == smem_data->smem_online_upgrade_flag)
    {
        if(NV_RESTRORE_SUCCESS == stuType)
        {
            /*在线升级NV自动恢复阶段魔术字清零*/
            smem_data->smem_online_upgrade_flag = 0;

            /*组播升级不重启*/
            if(SMEM_MULTIUPG_FLAG_NUM == smem_data->smem_multiupg_flag)
            {
                smem_data->smem_multiupg_flag = 0;
                g_stuType = stuType;
                (void)bsp_nvm_led_show();
                printf("MULTI UPG success, do not reboot.\n");
            }
            else
            {
                smem_data->smem_switch_pcui_flag = 0;
                printf("MBB:Online Upgrade Sucessful,reboot.\n");
                /*单板重启进入正常模式*/
                bsp_drv_power_reboot();
            }
        }
        else
        {
            if(SMEM_MULTIUPG_FLAG_NUM == smem_data->smem_multiupg_flag)
            {
                smem_data->smem_multiupg_flag = 0;
                g_stuType = stuType;
                (void)bsp_nvm_led_show();
            }

            printf("MBB:Online Upgrade failed !\n");
        }
    }
    return 0;
}






u32 bsp_nvm_upgrade(void)
{
    u32 ret;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;


    nv_debug(NV_FUN_UPGRADE_PROC,0,0,0,0);


    /*判断fastboot阶段xml 解析是否异常，若出现异常，则需要重新解析xml*/
    if(ddr_info->xml_dec_state != NV_XML_DEC_SUCC_STATE)
    {
        ret = bsp_nvm_xml_decode();
        if(ret)
        {
            nv_debug(NV_FUN_UPGRADE_PROC,1,ret,0,0);
            goto upgrade_fail_out;
        }
    }
    ret = nv_set_crc_offset();
    if(ret)
    {
        nv_debug(NV_FUN_UPGRADE_PROC,2,ret,0,0);
        goto upgrade_fail_out;
    }

    /*恢复处理前先将某些nv刷新到default(定制需求)*/
    ret = nv_writepart_to_default();
    if (NV_OK != ret)
    {
        /* 保留log打印,但不返回错误 */
        printk(KERN_ERR "[nv upgrade]: write part to default error.\n");
    }

    /*升级恢复处理，烧片版本直接返回ok*/
    ret = nv_upgrade_revert_proc();
    if(ret)
    {
        nv_debug(NV_FUN_UPGRADE_PROC,4,ret,0,0);
        goto upgrade_fail_out;
    }

    ret = nv_make_ddr_crc();
    if(ret)
    {
        nv_debug(NV_FUN_UPGRADE_PROC,6,ret,0,0);
        goto upgrade_fail_out;
    }
    /*将最新数据写入各个分区*/
    ret = nv_data_writeback();
    if(ret)
    {
        nv_debug(NV_FUN_UPGRADE_PROC,7,ret,0,0);
        goto upgrade_fail_out;
    }

    /*置升级包无效*/
    ret = (u32)nv_modify_upgrade_flag(false);/*lint !e747 */
    if(ret)
    {
        nv_debug(NV_FUN_UPGRADE_PROC,8,ret,0,0);
        goto upgrade_fail_out;
    }

    ret = nv_file_update(NV_DLOAD_PATH);
    if(ret)
    {
        nv_debug(NV_FUN_UPGRADE_PROC, 9,ret,0,0);
        goto upgrade_fail_out;
    }

    return NV_OK;
upgrade_fail_out:
    nv_mntn_record("\n%s\n",__func__);
    nv_help(NV_FUN_UPGRADE_PROC);
    return NV_ERROR;
}

u32 bsp_nvm_resume_bakup(void)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    u32 datalen = 0;

    if(true == nv_check_file_validity((s8 *)NV_BACK_PATH, (s8 *)NV_BACK_FLAG_PATH))
    {
        nv_mntn_record("load from %s ...%s %s \n",NV_BACK_PATH,__DATE__,__TIME__);
        fp = nv_file_open((s8*)NV_BACK_PATH,(s8*)NV_FILE_READ);
        if(!fp)
        {
            nv_debug(NV_FUN_MEM_INIT,5,0,0,0);
            goto load_err_proc;
        }

        ret = nv_read_from_file(fp,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&datalen, NV_FILE_BACKUP);
        nv_file_close(fp);
        if(ret)
        {
            nv_debug(NV_FUN_MEM_INIT,6,0,0,0);
            goto load_err_proc;
        }
        ret = nv_set_crc_offset();
        if(ret)
        {
            nv_debug(NV_FUN_MEM_INIT,7,ret,0,0);
            goto load_err_proc;
        }

        ret = nv_check_ddr_crc();
        if(ret)
        {
            nv_debug(NV_FUN_MEM_INIT,8,ret,0,0);
            goto load_err_proc;
        }

        /*从备份区加载需要首先写入工作区*/
        ret = bsp_nvm_flushEn(NV_FLAG_NO_CRC);
        if(ret)
        {
            nv_debug(NV_FUN_MEM_INIT,9,0,0,0);
            goto load_err_proc;
        }

        return NV_OK;
    }

load_err_proc:
    ret = nv_load_err_proc();
    if(ret)
    {
        nv_mntn_record("%s %d ,err revert proc ,ret :0x%x\n",__func__,__LINE__,ret);
        nv_help(NV_FUN_MEM_INIT);
    }

    return ret;
}



u32 bsp_nvm_reload(void)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    u32 datalen = 0;
    u32 try_times = STARTUP_TRY_TIMES;
    /*获取fastboot中异常信息共享内存地址*/
    power_info_s *power_info = (power_info_s *)SRAM_REBOOT_ADDR;

    nv_debug(NV_FUN_MEM_INIT,0,0,0,0);

    /*工作分区数据存在，且无未写入完成的标志文件*/
    if( true == nv_check_file_validity((s8 *)NV_IMG_PATH, (s8 *)NV_IMG_FLAG_PATH))
    {
        /*如果发现反复重启的次数等于STARTUP_TRY_TIMES次*/
        if(try_times == power_info->wdg_rst_cnt)
        {
            /*如果备份区没有数据不做任何操作*/
            if(!nv_file_access(NV_BACK_PATH,0))
            {
                /*记录发生异常的log信息到nvlog中*/
                nv_mntn_record("%s %s :The restart time has reached MAX:%d!\n",__DATE__,__TIME__,try_times);
                /*删除工作区域nv文件*/
                nv_file_remove((s8*)NV_IMG_PATH);
                /*启用备份区nv文件*/
                goto load_bak;
            }
        }
        nv_mntn_record("load from %s ...%s %s \n",NV_IMG_PATH,__DATE__,__TIME__);
        fp = nv_file_open((s8*)NV_IMG_PATH,(s8*)NV_FILE_READ);
        if(!fp)
        {
            nv_debug(NV_FUN_MEM_INIT,1,0,0,0);
            goto load_bak;
        }

        ret = nv_read_from_file(fp,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&datalen, NV_FILE_SYS_NV);
        nv_file_close(fp);
        if(ret)
        {
            nv_debug(NV_FUN_MEM_INIT,2,ret,0,0);
            goto load_bak;
        }

        ret = nv_set_crc_offset();
        if(ret)
        {
            nv_debug(NV_FUN_MEM_INIT,3,ret,0,0);
            goto load_bak;
        }

        ret = nv_check_ddr_crc();
        if(ret)
        {
            nv_debug(NV_FUN_MEM_INIT,4,ret,0,0);
            nv_mntn_record("nv image check crc failed %d...%s %s \n", ret, __DATE__,__TIME__);

            /* 保存错误镜像，然后从bakup分区恢复 */
            (void)nv_debug_store_file(NV_IMG_PATH);
            if(nv_debug_is_resume_bakup())
            {
                ret = bsp_nvm_resume_bakup();
                if(ret)
                {
                    nv_mntn_record("nv resume bakup failed %d...%s %s \n", ret, __DATE__,__TIME__);
                }
            }
            else
            {
                nv_mntn_record("config don't resume bakup...%s %s \n",__DATE__,__TIME__);
            }

            /* 复位系统 */
            if(nv_debug_is_reset())
            {
                system_error(DRV_ERRNO_NV_CRC_ERR, NV_FUN_MEM_INIT, 3, NULL, 0);
            }
        }

        /*nvimg分区加载成功,检查是否需要备份
                无论是否成功单板都正常运行*/
        (void)nv_rebak_save_data();

        return ret;
    }

load_bak:

    return bsp_nvm_resume_bakup();
}
/*****************************************************************************
 函 数 名  : bsp_nvm_write_buf_init
 功能描述  : 初始化写入NV时使用的buf和信号量
 输入参数  :
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
u32 bsp_nvm_buf_init(void)
{
    /*create sem*/
    osl_sem_init(1,&g_nv_ctrl.nv_buf_sem);

    /*malloc buf*/
    g_nv_ctrl.p_nv_data_buf = (void *)nv_malloc(NV_CRC32_CHECK_SIZE*3);

    if(NULL == g_nv_ctrl.p_nv_data_buf)
    {
        nv_printf("malloc p_nv_data_buf fail\n");
        return BSP_ERR_NV_MALLOC_FAIL;
    }
    g_nv_ctrl.p_nv_crc_buf = (void *)nv_malloc((SHM_MEM_NV_SIZE/NV_CRC32_CHECK_SIZE + 1) * sizeof(u32));
    if(NULL == g_nv_ctrl.p_nv_crc_buf)
    {
        nv_free(g_nv_ctrl.p_nv_data_buf);
        nv_printf("malloc p_nv_crc_buf fail\n");
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    /*create sem*/
    osl_sem_init(1,&g_nv_ctrl.nv_list_sem);
    INIT_LIST_HEAD(&g_nv_ctrl.nv_list);

    return NV_OK;
}

u32 clean_mmi_nv_flag(void)
{

    u32 ret = NV_OK;
    /*0xFF,表示无效值*/
    u32 real_factory_mode = 0xFF;
    u8  factory_info[FACTORY_INFO_LEN] = {0};

    memset(factory_info, 0x00, FACTORY_INFO_LEN);
    /*读取当前单板所升级的软件为烧片软件还是升级软件*/
    ret = bsp_nvm_read(NV_ID_MSP_SW_VER_FLAG,(u8*)(&real_factory_mode),sizeof(u32));
    if(ret)
    {
        nv_printf("read 0x%x fail,use default value! ret :0x%x\n",NV_ID_MSP_SW_VER_FLAG,ret);
        return ret;
    }
    /*烧片软件一键升级返工，清空MMI标记*/
    if(0 == real_factory_mode)
    {
        /*读取NV114中的值*/
        ret = bsp_nvm_read(NV_ID_MSP_FACTORY_INFO,(u8*)(&factory_info),(sizeof(u8) * FACTORY_INFO_LEN));
        if(ret)
        {
            nv_printf("read 0x%x fail,use default value! ret :0x%x\n",NV_ID_MSP_FACTORY_INFO,ret);
        }
        else
        {
            /*清空NV114中MMI比较bit位*/
            memcpy(&factory_info[MMI_TEST_FLAG_OFFSET], "0000", MMI_TEST_FLAG_LEN);
            /*写入114清空MMI结果后的值*/
            ret = bsp_nvm_write(NV_ID_MSP_FACTORY_INFO,(u8*)(&factory_info),(sizeof(u8) * FACTORY_INFO_LEN));
            if(ret)
            {
                nv_printf("write 0x%x fail,use default value! ret :0x%x\n",NV_ID_MSP_FACTORY_INFO,ret);
            }
            else
            {
                ret = NV_OK;
            }
        }
    }
    /*出错的时候直接返回非0值，正确的时候返回NV_OK*/
    return ret;
}



s32 bsp_nvm_kernel_init(void)
{
    u32 ret = NV_ERROR;
    u32 clean_mmi_flag = 0;
    huawei_smem_info *smem_data = NULL;
    u32 nvsys_boot_flag = 0;
    u32 nvback_boot_flag = 0;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    nv_debug(NV_FUN_KERNEL_INIT,0,0,0,0);

    
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        printf("nv_file_init: smem_data is NULL \n");
        return -1;  

    }

    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        /*升级模式，屏蔽nv模块的启动*/
        printf("entry update not init nvim !\n");
        return -1;  
    }

    /*sem & lock init*/
    spin_lock_init(&g_nv_ctrl.spinlock);
    osl_sem_init(0,&g_nv_ctrl.task_sem);
    osl_sem_init(1,&g_nv_ctrl.rw_sem);
    osl_sem_init(0,&g_nv_ctrl.cc_sem);
    wake_lock_init(&g_nv_ctrl.wake_lock,WAKE_LOCK_SUSPEND,"nv_wakelock");
    g_nv_ctrl.shared_addr = (struct nv_global_ddr_info_stru *)NV_GLOBAL_INFO_ADDR;

    nv_mntn_record("Balong nv init  start! %s %s\n",__DATE__,__TIME__);

    (void)nv_debug_init();

    /*file info init*/
    ret = nv_file_init();
    if(ret)
    {
        nv_debug(NV_FUN_KERNEL_INIT,1,ret,0,0);
        goto out;
    }

    /* 标识清除前先保存下 */
    nvsys_boot_flag = ddr_info->nvsys_boot_state;
    nvback_boot_flag = ddr_info->nvback_boot_state;
    if(ddr_info->acore_init_state != NV_BOOT_INIT_OK)
    {
        nv_mntn_record("fast boot nv init fail !\n");
        nv_show_fastboot_err();
        memset(ddr_info,0,sizeof(struct nv_global_ddr_info_stru));
    }

    ddr_info->acore_init_state = NV_KERNEL_INIT_DOING;
    nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, (u32)NV_GLOBAL_INFO_SIZE);

    if((ddr_info->mem_file_type == NV_MEM_DLOAD) &&
       (!nv_file_access((s8*)NV_DLOAD_PATH,0)) &&/*升级分区存在数据*/
       (true == nv_get_upgrade_flag())/*升级文件有效*/
       )
    {
        clean_mmi_flag = 1;
        ret = bsp_nvm_upgrade();
        if(ret)
        {
            nv_debug(NV_FUN_KERNEL_INIT,3,ret,0,0);
            goto out;
        }
        /*读取NV自管理配置*/
        ret = bsp_nvm_read(NV_ID_DRV_SELF_CTRL,(u8*)(&(g_nv_ctrl.nv_self_ctrl)),sizeof(NV_SELF_CTRL_STRU));
        if(ret)
        {
            g_nv_ctrl.nv_self_ctrl.ulResumeMode = NV_MODE_USER;
            nv_printf("read 0x%x fail,use default value! ret :0x%x\n",NV_ID_DRV_SELF_CTRL,ret);
        }
    }
    else
    {
        /*读取NV自管理配置*/
        ret = bsp_nvm_read(NV_ID_DRV_SELF_CTRL,(u8*)(&(g_nv_ctrl.nv_self_ctrl)),sizeof(NV_SELF_CTRL_STRU));
        if(ret)
        {
            g_nv_ctrl.nv_self_ctrl.ulResumeMode = NV_MODE_USER;
            nv_printf("read 0x%x fail,use default value! ret :0x%x\n",NV_ID_DRV_SELF_CTRL,ret);
        }

        /*重新加载最新数据*/
        ret = bsp_nvm_reload();
        if(ret)
        {
            nv_debug(NV_FUN_KERNEL_INIT,4,ret,0,0);
            goto out;
        }
        else
        {
            nv_printf("reload success!\n");
            /* reload success查看是否需要修复nvsys和nvback
                共享内存值可能已修复,因此两者都要判断 */
            if ((NV_INIT_OK != nvsys_boot_flag)
                && (NV_INIT_OK != ddr_info->nvsys_boot_state))
            {
                /* nvsys加载失败修复,无论结果均执行且清空标识 */
                (void)bsp_nvm_flushSys(NV_ERROR);
                nvsys_boot_flag = NV_INIT_OK;
                ddr_info->nvsys_boot_state = NV_INIT_OK;
                /* 在nv log文件中记录*/
                nv_mntn_record("[nv init]:repair nvsys data ok.\n");
            }
            if ((NV_INIT_OK != nvback_boot_flag)
                && (NV_INIT_OK != ddr_info->nvback_boot_state))
            {
                /* nvsys加载失败修复,无论结果均执行且清空标识 */
                (void)bsp_nvm_backup(NV_FLAG_NO_CRC);
                nvback_boot_flag = NV_INIT_OK;
                ddr_info->nvback_boot_state = NV_INIT_OK;
                /* 在nv log文件中记录*/
                nv_mntn_record("[nv init]:repair nvback data ok.\n");
            }
        }
    }
    /*初始化双核使用的链表*/
    nv_flushListInit();
    ret = bsp_ipc_sem_create(IPC_SEM_NV_CRC);
    if(ret)
    {
        nv_debug(NV_FUN_KERNEL_INIT,6,ret,0,0);
        goto out;
    }

    ret = bsp_nvm_buf_init();
    if(ret)
    {
        nv_debug(NV_FUN_KERNEL_INIT,10,ret,0,0);
        goto out;
    }

    /*置初始化状态为OK*/
    ddr_info->acore_init_state = NV_INIT_OK;
    nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, (u32)NV_GLOBAL_INFO_SIZE);

    /*保证各分区数据正常写入*/
    nv_file_flag_check();

    INIT_LIST_HEAD(&g_nv_ctrl.stList);
/*lint -save -e740*/
/*  处理单板的CAT等级，如果处理失败，则不再做处理  */

    ret = (u32)osl_task_init("drv_nv",15,1024,(OSL_TASK_FUNC)bsp_nvm_icc_task,NULL,(OSL_TASK_ID*)&g_nv_ctrl.task_id);
    if(ret)
    {
        nv_mntn_record("[%s]:nv task init err! ret :0x%x\n",__func__,ret);
        goto out;
    }
/*lint -restore +e740*/

    if((ret = nv_icc_chan_init(NV_RECV_FUNC_AC))\
        ||(ret = nv_icc_chan_init(NV_RECV_FUNC_AM))\
        )
    {
        nv_debug(NV_FUN_KERNEL_INIT,5,ret,0,0);
        goto out;
    }

    /*to do:nv id use macro define*/
    ret = bsp_nvm_read(NV_ID_MSP_FLASH_LESS_MID_THRED,(u8*)(&(g_nv_ctrl.mid_prio)),sizeof(u32));
    if(ret)
    {
        g_nv_ctrl.mid_prio = 20;
        nv_printf("read 0x%x fail,use default value! ret :0x%x\n",NV_ID_MSP_FLASH_LESS_MID_THRED,ret);
    }

    nvchar_init();
    /*在一键升级备份恢复NV的时候对烧片版本的MMI标记进行清0操作*/
    if(1 == clean_mmi_flag)
    {
        ret = clean_mmi_nv_flag();
        if(ret)
        {
            /*5为debug级别*/
            nv_debug(NV_FUN_KERNEL_INIT,5,ret,0,0);
            goto out;
        }
    }


    nv_mntn_record("Balong nv init ok!\n");
    /*nv初始化成功后将软件版版本号写入oeminfo 分区
    先读出判断是否一致,不一致写入
    此处不判断返回值,是否写入成功均向下执行*/
    (void)huawei_dload_set_swver_to_oeminfo();

    ret = bsp_nvm_restore_online_handle(NV_RESTRORE_SUCCESS);
    if(ret)
    {
        return ret;
    }
    return NV_OK;

out:
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    ddr_info->acore_init_state = NV_INIT_FAIL;
    nv_help(NV_FUN_KERNEL_INIT);
    show_ddr_info();
    bsp_nvm_restore_online_handle(NV_RESTRORE_FAIL);
    return -1;
}

/*lint -save -e529*/
static void bsp_nvm_exit(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    /*关机写数据*/
    (void)bsp_nvm_flush();
    /*清除标志*/
    memset(ddr_info,0,sizeof(struct nv_global_ddr_info_stru));
    
    (void)bsp_nvm_led_show_stop();
}
/*lint -restore +e529*/


u32 nvm_read_rand(u32 nvid)
{
    u32 ret;
    u8* tempdata;
    u32 i= 0;
    struct nv_ref_data_info_stru ref_info = {0};
    struct nv_file_list_info_stru file_info = {0};

    ret = nv_search_byid(nvid, (u8*)NV_GLOBAL_CTRL_INFO_ADDR,&ref_info,&file_info);
    if(NV_OK != ret)
    {
        return ret;
    }
    bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_NV,"[0x%x]:len 0x%x,off 0x%x,file id %d\n",nvid,ref_info.nv_len,ref_info.nv_off,ref_info.file_id);

    tempdata = (u8*)nv_malloc((u32)(ref_info.nv_len) +1);
    if(NULL == tempdata)
    {
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    ret = bsp_nvm_read(nvid,tempdata,ref_info.nv_len);
    if(NV_OK != ret)
    {
        nv_free(tempdata);
        return BSP_ERR_NV_READ_DATA_FAIL;
    }

    for(i=0;i<ref_info.nv_len;i++)
    {
        if((i%32) == 0)
        {
            bsp_trace(BSP_LOG_LEVEL_ERROR,BSP_MODU_NV, "\n");
        }
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_NV, "%02x ",(u8)(*(tempdata+i)));
    }
    nv_free(tempdata);
    bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_NV,"\n\n");
    return 0;
}


u32 nvm_read_randex(u32 nvid,u32 modem_id)
{
	u32 ret;    u8* tempdata;    u32 i= 0;
	struct nv_ref_data_info_stru ref_info = {0};
	struct nv_file_list_info_stru file_info = {0};

	ret = nv_search_byid(nvid, (u8*)NV_GLOBAL_CTRL_INFO_ADDR,&ref_info,&file_info);
	if(NV_OK != ret)
	{
		return ret;
	}
	if(ref_info.nv_len == 0)
	{
		return NV_ERROR;
	}

	nv_printf("[0x%x]:len 0x%x,off 0x%x,file id %d\n",nvid,ref_info.nv_len,ref_info.nv_off,ref_info.file_id);
	nv_printf("[0x%x]:dsda 0x%x\n",nvid,ref_info.modem_num);

	tempdata = (u8*)nv_malloc((u32)(ref_info.nv_len) +1);
	if(NULL == tempdata)
	{
		return BSP_ERR_NV_MALLOC_FAIL;
	}
	ret = bsp_nvm_dcread(modem_id,nvid,tempdata,ref_info.nv_len);
	if(NV_OK != ret)
	{
		nv_free(tempdata);
		return BSP_ERR_NV_READ_DATA_FAIL;
	}

	for(i=0;i<ref_info.nv_len;i++)
	{
		if((i%32) == 0)
		{
			printf("\n");
		}
		printf("%02x ",(u8)(*(tempdata+i)));
	}

	printf("\n\n");
	nv_free(tempdata);

	return 0;

}

/*lint -save -e19*/
module_init(bsp_nvm_kernel_init);
module_exit(bsp_nvm_exit);
/*lint -restore +e19*/

EXPORT_SYMBOL(bsp_nvm_backup);
EXPORT_SYMBOL(bsp_nvm_dcread);
EXPORT_SYMBOL(bsp_nvm_kernel_init);
EXPORT_SYMBOL(bsp_nvm_revert);
EXPORT_SYMBOL(bsp_nvm_update_default);
EXPORT_SYMBOL(bsp_nvm_revert_default);
EXPORT_SYMBOL(bsp_nvm_dcreadpart);
EXPORT_SYMBOL(bsp_nvm_get_len);
EXPORT_SYMBOL(bsp_nvm_dcwrite);
EXPORT_SYMBOL(bsp_nvm_flush);
EXPORT_SYMBOL(bsp_nvm_reload);
EXPORT_SYMBOL(nvm_read_rand);
EXPORT_SYMBOL(nvm_read_randex);
EXPORT_SYMBOL(show_ddr_info);
EXPORT_SYMBOL(bsp_nvm_dcread_direct);
EXPORT_SYMBOL(bsp_nvm_dcwrite_direct);
EXPORT_SYMBOL(bsp_nvm_auth_dcread);
EXPORT_SYMBOL(bsp_nvm_auth_dcwrite);
EXPORT_SYMBOL(bsp_nvm_dcwritepart);
EXPORT_SYMBOL(bsp_nvm_get_nvidlist);
EXPORT_SYMBOL(bsp_nvm_authgetlen);
EXPORT_SYMBOL(bsp_nvm_flushEx);
EXPORT_SYMBOL(bsp_nvm_xml_decode);

/*lint -restore*/




