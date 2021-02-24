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




/*lint -save -e322 -e7 -e537*/
#include <stdarg.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include "mdrv_rfile_common.h"
#include <bsp_hardtimer.h>
#include <product_nv_id.h>
#include <product_nv_def.h>
#include <mdrv.h>
#include "nv_comm.h"
#include "nv_file.h"
#include "nv_ctrl.h"
#include "nv_xml_dec.h"
#include "nv_debug.h"
#include "bsp_dump.h"
#include "NVIM_ResumeId.h"
#include "RfNvId.h"
#include "nv_debug.h"
/*lint -restore +e322 +e7 +e537*/

/*lint -save -e438 -e530 -e713 -e830 -e529*/
static const u32 nv_mid_droit[NV_MID_PRI_LEVEL_NUM] = {20,1,1,1,1,1, 0};

struct nv_global_ctrl_info_stru  g_nv_ctrl = {};
struct nv_global_debug_stru      g_nv_debug[NV_FUN_MAX_ID];

#include "product_nv_id.h"
#include "SysNvId.h"
#include "PsNvId.h"
#include "RfNvId.h"
#include "LNvCommon.h"

/* 
NOTICE: 
g_NvSysList����: BOOTLOADER�׶δ��ڶ�ȡnv�����󣬴˽׶ζ�ȡ��NV������sys nv�����У�
�����bootloader�׶θ�����bootloader�׶ζ�ȡ��nv���ݣ���Ҫ����nvͬ�����µ�sys nv�����У�
��g_NvSysList�б��µ�nv��Ǳ�����bootloader�׶ζ�ȡ��nv

����ӵ�g_NvSysList�б��NV��(ֻ��bootloader�׶����ȡ��NV�й�):
0xd100 - 0xd1ff֮���nv�޸Ļ�Ĭ��ˢ�µ�sys nv����,����ˢ�µ����б�
��������Ļ�Ϊ����ʹ��nv��Ҫ��ӵ����б�
*/
u16 g_NvSysList[] = { NV_ID_DRV_VER_FLAG,
                      NV_ID_DRV_NV_DRV_VERSION_REPLACE_I,
                      NV_ID_SOFT_RELIABLE_CFG,
                      NV_TEST_POWERUP_MODE_CONTROL_FLAG,
};

/*д���ٱ���nv������*/
/*ע��:������ʹ�ö��ַ�����,���nvid���밴�մ�С�����˳������
nv���ʱ�����ע��.*/
static u16 g_nvrebaklist[] =
{
    en_NV_Item_RAT_PRIO_LIST,                   /*9007 Э��*/

    NV_FOTA_PPP_APN,                            /*50400 Ӧ��*/    
    NV_FOTA_PPP_PASSWORD,                       /*50401 Ӧ��*/
    NV_FOTA_PPP_USER_ID,                        /*50402 Ӧ��*/
    NV_FOTA_PPP_AUTHTYPE,                       /*50403 Ӧ��*/
    NV_FOTA_DETECT_MODE,                        /*50406 Ӧ��*/

    NV_Item_FOTA_TIME_STAMP,                    /*50471 Э��*/

    NV_HUAWEI_OEMLOCK_I,                        /*50502 ƽ̨*/
    NV_HUAWEI_SIMLOCK_I,                        /*50503 ƽ̨*/

    en_NV_HUAWEI_SIM_SWITCH,                    /*50529 Э��*/
};
static bool g_nvrebak_flag = false; /*nv rebakȫ�ֱ�־*/

/*****************************************************************************
 �� �� ��  : nv_rebak_check_flag
 ��������  : �鿴��־�ļ��Ƿ����
 �������  : ��
 �������  : ��
 �� �� ֵ  : TRUE or FALSE
*****************************************************************************/
static bool nv_rebak_check_flag(void)
{
    bool bret = false;
    int iret = -1;

    iret = mdrv_file_access((char *)NV_REBAK_FLAG_PATH, 0);
    if(0 == iret)
    {
        bret = true;
    }

    return bret;
}

/*****************************************************************************
 �� �� ��  : nv_rebak_save_flag
 ��������  : ����rebak��־�ļ�,����nvrebak.bin�ļ�
 �������  : ��
 �������  : ��
 �� �� ֵ  : NV_OK or NV_ERROR
*****************************************************************************/
static u32 nv_rebak_save_flag(void)
{
    bool bret = false;
    FILE* fp = NULL;

    bret = nv_rebak_check_flag();
    if((true != bret) && (true == g_nvrebak_flag))
    {
        /*�ļ��������ұ�־ΪTRUE�򴴽���־�ļ�*/
        fp = mdrv_file_open((char *)NV_REBAK_FLAG_PATH, "w+");
        if(NULL == fp)
        {
            printk(KERN_ERR "[%s]: create file %s failed.\n",
                            __func__, NV_REBAK_FLAG_PATH);
            return NV_ERROR;
        }
        (void)mdrv_file_close(fp);
    }

    return NV_OK;
}

/*****************************************************************************
 �� �� ��  : nv_rebak_remove_flag
 ��������  : ɾ��rebak��־�ļ�
 �������  : ��
 �������  : ��
 �� �� ֵ  : NV_OK or NV_ERROR
*****************************************************************************/
static u32 nv_rebak_remove_flag(void)
{
    int iret = -1;
    bool bret = false;

    bret = nv_rebak_check_flag();
    if(true == bret)
    {
        iret = mdrv_file_remove((char *)NV_REBAK_FLAG_PATH);
        if(0 != iret)
        {
            printk(KERN_ERR "[%s]: remove file %s failed.\n", __func__, NV_REBAK_FLAG_PATH);
            return NV_ERROR;
        }
    }

    g_nvrebak_flag = false;
    return NV_OK;
}

/*****************************************************************************
 �� �� ��  : nv_rebak_save_data
 ��������  : �����־�ļ�����,���ݵ�ǰnv��������
 �������  : ��
 �������  : ��
 �� �� ֵ  : NV_OK or NV_ERROR
*****************************************************************************/
u32 nv_rebak_save_data(void)
{
    u32 uret = NV_ERROR;
    bool bret = false;

    bret = nv_rebak_check_flag();
    if(true == bret)
    {
        printk(KERN_INFO "[%s]: nvbackup flag is exist, then backup nv\n", __func__);
        uret = bsp_nvm_backup(NV_FLAG_NO_CRC);
        if(NV_OK != uret)
        {
            printk(KERN_ERR "[%s]: save data to [back] fail, ret = 0x%x.\n",
                           __func__, uret);
            return NV_ERROR;
        }
        /*���ݳɹ�ɾ����־�ļ�*/
        (void)nv_rebak_remove_flag();
    }

    return NV_OK;
}

/*****************************************************************************
 �� �� ��  : nv_isRebak
 ��������  : �鿴�Ƿ�Ϊ��Ҫ�ٱ��ݵ�nv,nv�϶�ʹ�ö��ַ�
 �������  : itemid -- ���鿴��nv id
 �������  : ��
 �� �� ֵ  : TRUE or FALSE
*****************************************************************************/
static bool nv_is_rebak(u16 itemid)
{
    bool bret = false;
    u32 high = 0;
    u32 low = 0;
    u32 mid = 0;

    /*���ַ�����*/
    high = (u32)(sizeof(g_nvrebaklist) / sizeof(g_nvrebaklist[0]));
    low = 1;
    while(low <= high)
    {
        mid = (low + high) / 2; /*mid��>= 1*/
        if(itemid < g_nvrebaklist[mid - 1])
        {
            high = mid - 1;
        }
        else if(itemid > g_nvrebaklist[mid - 1])
        {
            low = mid + 1;
        }
        else
        {
            /*���б����ҵ�nv��*/
            bret = true;
            break;
        }
    }

    return bret;
}

/*****************************************************************************
 �� �� ��  : nv_rebak_check
 ��������  : ��flash list�е�nv����м��,�Ƿ���Ҫ�ٴα���
 �������  : flash_info -- ��д���nv�б�
 �������  : ��
 �� �� ֵ  : NV_OK or NV_ERROR
*****************************************************************************/
static u32 nv_rebak_check(struct nv_flush_list_stru * flush_info)
{
    bool bret = false;
    int i = 0;

    if(NULL == flush_info)
    {
        printk(KERN_ERR "[%s]: Input para error.\n", __func__);
        return NV_ERROR;
    }

    /*��ǰ�Ѿ����ù���־,�����ֱ���˳�*/
    bret = nv_rebak_check_flag();
    if(true == bret)
    {
        printk(KERN_INFO  "[%s]: nvbackup flag is exist, exit.\n", __func__);
        return NV_OK;
    }
    /*ѭ������flush info�е�����*/
    for(i = 0; i < flush_info->count; i++)
    {
        bret = nv_is_rebak(flush_info->list[i].itemid);
        if(true == bret)
        {
            /*����һ���������б���ʱ���˳�*/
            printk(KERN_INFO  "[%s]: set nvbackup flag.\n", __func__);
            g_nvrebak_flag = true;
            break;
        }
    }

    return NV_OK;
}

/*nv debug func,reseverd1 used to reg branch*/
void nv_debug(u32 type,u32 reseverd1,u32 reserved2,u32 reserved3,u32 reserved4)
{
    if(0 == reseverd1)
    {
        g_nv_debug[type].callnum++;
    }
    g_nv_debug[type].reseved1 = reseverd1;
    g_nv_debug[type].reseved2 = reserved2;
    g_nv_debug[type].reseved3 = reserved3;
    g_nv_debug[type].reseved4 = reserved4;
}


/*ϵͳ����log��¼�ӿڣ����浽 NV_LOG_PATH �У���С�޶��� NV_LOG_MAX_SIZE*/
void nv_mntn_record(char* fmt,...)
{
    char   buffer[256];
    va_list arglist;
    FILE* fp = NULL;
    int ret = 0;
    int file_len;
    int buffer_size = 0;

    /*lint -save -e530*/
    va_start(arglist, fmt);
    /*lint -restore +e530*/
    vsnprintf(buffer,256, fmt, arglist);/* [false alarm]:format string */
    va_end(arglist);

    nv_printf("%s",buffer);

    if(mdrv_file_access((char*)NV_LOG_PATH,0))
    {
        fp = mdrv_file_open((char*)NV_LOG_PATH,"w+");
        if(!fp)
            return;
    }
    else
    {
        fp = mdrv_file_open((char*)NV_LOG_PATH,"r+");
        if(!fp)
            return;
        mdrv_file_seek(fp,0,SEEK_END);
        file_len = mdrv_file_tell(fp);
        /*lint -save -e737*/
        if((file_len+strlen(buffer))>= NV_LOG_MAX_SIZE)
        {
            mdrv_file_close(fp);
            fp = mdrv_file_open((char*)NV_LOG_PATH,"w+");
            if(!fp)
                return;
        }
    }
    ret = mdrv_file_write(buffer,1,(unsigned int)strlen(buffer),fp);
    if(ret != strlen(buffer))
    {
        buffer_size = strlen(buffer);
        nv_printf("mdrv_file_write   nv   log err!  ret :0x%x buffer len :0x%x\n",ret,buffer_size);
    }
    /*lint -restore +e737*/
    mdrv_file_close(fp);
}
/*����flag��־�ļ�,����д��֮ǰ����*/
void nv_create_flag_file(const s8* path)
{
    FILE* fp = NULL;

    if(!mdrv_file_access(path,0))
        return;
    fp = mdrv_file_open((char*)path, (char*)NV_FILE_WRITE);

    if(fp){
        mdrv_file_close(fp);
        return;
    }
    else
        return;
}

/*ɾ��flag��־�ļ�������д��֮�����*/
void nv_delete_flag_file(const s8* path)
{
    if(mdrv_file_access((char*)path,0))
        return;
    else
        mdrv_file_remove((char*)path);
}

/*�жϱ�־�ļ��Ƿ���� true :���ڣ� false :������*/
bool nv_flag_file_isExist(const s8* path)
{
    return (mdrv_file_access((char*)path,0) == 0)?true:false;
}

/*����֮�������������ϵͳ����flag�ļ����ڣ�����Ҫ����������д���Ӧ������*/
void nv_file_flag_check(void)
{
    if( !mdrv_file_access((char*)NV_BACK_FLAG_PATH,0)){
        nv_mntn_record("%s %s :last time [back file] write abnomal,rewrite !\n",__DATE__,__TIME__);
        (void)bsp_nvm_backup(NV_FLAG_NEED_CRC);
    }

    if( !mdrv_file_access((char*)NV_SYS_FLAG_PATH,0)){
        nv_mntn_record("%s %s :last time [sys file] write abnomal,rewrite !\n",__DATE__,__TIME__);
        (void)bsp_nvm_flushSys(NV_ERROR);
    }

    if( !mdrv_file_access((char*)NV_IMG_FLAG_PATH,0)){
        nv_mntn_record("%s %s :last time [img file] write abnomal,rewrite !\n",__DATE__,__TIME__);
        (void)bsp_nvm_flushEn(NV_FLAG_NEED_CRC);
    }
}

/*�������쳣����½�����д���������*/
u32 nv_data_writeback(void)
{
    u32 ret;

    ret = bsp_nvm_flushEn(NV_FLAG_NO_CRC);
    if(ret)
    {
        nv_error_printf("write back to [img] fail! ret :0x%x\n",ret);
        return ret;
    }

    ret = bsp_nvm_backup(NV_FLAG_NO_CRC);
    if(ret)
    {
        nv_error_printf("write back to [back] fail! ret :0x%x\n",ret);
        return ret;
    }

    ret = bsp_nvm_flushSys(NV_ERROR);
    if(ret)
    {
        nv_error_printf("write back to [system] fail! ret :0x%x\n",ret);
        return ret;
    }

    return NV_OK;
}

/*���������ݴӹ��������ָ�*/
u32 nv_load_err_proc(void)
{
    return NV_ERROR;
}

/*****************************************************************************
 �� �� ��  : nv_get_bin_file_len
 ��������  : ����nv.bin�ļ��Ĵ�С
 �������  : fp:��������ļ�
 �������  : ��
 �� �� ֵ  : �ļ���С
*****************************************************************************/
bool nv_check_file_validity(s8 * filePath, s8 *flagPath)
{
    u32 ret;

    /*�ļ�������*/
    if(nv_file_access((s8*)filePath,0))
    {
        return false;
    }
    /*��δд����ɵı�־ */
    if(true == nv_flag_file_isExist((s8*)flagPath))
    {
        nv_mntn_record("%s  last time write abornormal !\n",filePath);
        return false;
    }

    /*imei�ż��*/
    ret = nv_imei_data_comp((s8*)filePath);
    if(ret)
    {
        nv_mntn_record("%s imei compare with factory data is not same ret :0x%x!\n",filePath,ret);
        return false;
    }

    return true;
}


/*��Ҫ֧�����������ı��ݻָ���ָ����������*/
u32 nv_upgrade_revert_proc(void)
{
    u32 ret;

    /*��鹤�������ݵ���Ч��*/
    if(true == nv_check_file_validity((s8 *)NV_IMG_PATH, (s8 *)NV_IMG_FLAG_PATH))
    {
        ret = nv_revertEx((s8*)NV_IMG_PATH);
        if(ret)
        {
            nv_mntn_record("revert from %s fail,goto next err proc ret:0x%x!\n",NV_IMG_PATH,ret);
            goto revert_backup;
        }

        /*�ӹ������ָ����֮�󣬱��ݹ��������ݵ����ݷ���*/
        ret = nv_copy_img2backup();
        if(ret)/*�����쳣ֱ���˳�*/
        {
            nv_mntn_record("copy img to backup fail,ret :0x%x\n",ret);
            return ret;
        }
        return NV_OK;
    }

revert_backup:
    /*��鱸�������ݵ���Ч��*/
    if(true == nv_check_file_validity((s8 *)NV_BACK_PATH, (s8 *)NV_BACK_FLAG_PATH))
    {
        ret = nv_revertEx((s8*)NV_BACK_PATH);
        if(ret)
        {
            nv_mntn_record("revert from %s fail,goto next err proc ret:0x%x!\n",NV_BACK_PATH,ret);
            goto revert_factory;
        }

        return NV_OK;
    }

revert_factory:
    /*��������������ֱ�Ӵӳ��������ָ�*/
    if(!nv_file_access((s8*)NV_DEFAULT_PATH,0))
    {
        ret = nv_revertEx((s8*)NV_DEFAULT_PATH);
        if(ret)
        {
            nv_mntn_record("revert from %s fail,return err! ret:0x%x\n",NV_DEFAULT_PATH,ret);
            return ret;
        }

    }

    /*��Ƭ�汾�����ݻָ���ֱ�ӷ���ok*/
    return NV_OK;
}
bool nv_check_update_default_right(void)
{
    /*�����ڣ���ֱ�ӷ���true*/
    if(nv_file_access((s8*)NV_DEFAULT_PATH,0))
    {
        return true;
    }
    /*��������д����ļ�*/
    if(!mdrv_file_access((char*)NV_FACTORY_RIGHT_PATH,0))
    {
        return true;
    }

    return false;
}

void nv_delete_update_default_right(void)
{
    if(!mdrv_file_access((char*)NV_FACTORY_RIGHT_PATH,0))
    {
        (void)mdrv_file_remove((char*)NV_FACTORY_RIGHT_PATH);
    }
}

/*
 * pick up the base info from the major info,then reg in base_info
 */
u32 nv_init_file_info(u8* major_info,u8* base_info)
{
    u32 i;
    struct nv_ctrl_file_info_stru * ctrl_file = (struct nv_ctrl_file_info_stru*)major_info;
    struct nv_global_ddr_info_stru* ddr_info  = (struct nv_global_ddr_info_stru*)base_info;
    struct nv_file_list_info_stru * file_info = (struct nv_file_list_info_stru *)((u8*)ctrl_file+NV_GLOBAL_CTRL_INFO_SIZE);

    ddr_info->file_num = ctrl_file->file_num;   /*reg file num*/
    ddr_info->file_len = ctrl_file->ctrl_size;  /*reg ctrl file size,then add file size*/

    for(i = 0;i<ctrl_file->file_num;i++)
    {
        /*check file id*/
        if((i+1) != file_info->file_id)
        {
            nv_printf("file id  %d error ,i: %d\n",file_info->file_id,i);
            return BSP_ERR_NV_FILE_ERROR;
        }
        ddr_info->file_info[i].file_id = file_info->file_id;
        ddr_info->file_info[i].size    = file_info->file_size;
        ddr_info->file_info[i].offset  = ddr_info->file_len;

        ddr_info->file_len            += file_info->file_size;

        file_info++;
    }
    if(ddr_info->file_len > NV_MAX_FILE_SIZE)
    {
        nv_mntn_record("[%s]: file len error 0x%x\n",__FUNCTION__,ddr_info->file_len);
        return BSP_ERR_NV_FILE_ERROR;
    }
    return NV_OK;

}

void nv_modify_print_sw(u32 arg)
{
    g_nv_ctrl.debug_sw = arg;
}

s32 nv_modify_pm_sw(s32 arg)
{
    g_nv_ctrl.pmSw = (bool)arg;
    return 0;
}
bool nv_isSysNv(u16 itemid)
{
    /*lint -save -e958*/
    u32 i;
    /*lint -restore*/
    if(itemid >= NV_ID_SYS_MIN_ID && itemid <= NV_ID_SYS_MAX_ID)
        return true;

    for(i = 0;i<sizeof(g_NvSysList)/sizeof(g_NvSysList[0]);i++)
    {
        if(itemid == g_NvSysList[i])
            return true;
    }
    return false;

}

/*
 * get nv read right,check the nv init state or upgrade state to read nv,
 * A core may read nv data after kernel init ,C core read nv data must behine the phase of
 *       acore kernel init or acore init ok
 */
bool nv_read_right(u32 itemid)
{
    struct nv_global_ddr_info_stru* ddr_info= (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    if( NV_KERNEL_INIT_DOING > ddr_info->acore_init_state)
    {
        return false;
    }
    return true;
}

bool nv_write_right(u32 itemid)
{
    struct nv_global_ddr_info_stru* ddr_info= (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    if(NV_INIT_OK != ddr_info->acore_init_state)
    {
        return false;
    }

    /*TMODE CHECK*/

    return true;
}
/*
 * get file len
 * return : file len
 */
u32 nv_get_file_len(FILE* fp)
{
    s32 ret = -1;
    u32 seek = 0;

    ret = nv_file_seek(fp,0,SEEK_END);
    if(ret)
    {
        goto out;
    }

    seek = (u32)nv_file_ftell(fp);

    ret = nv_file_seek(fp,0,SEEK_SET);
    if(ret)
    {
        goto out;
    }

    return seek;

out:
    nv_printf("seek file fail\n");
    return NV_ERROR;
}

/*
 * check the dload file validity
 *
 */
bool nv_dload_file_check(void )
{
    u32 i;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_file_list_info_stru* file_info = (struct nv_file_list_info_stru*)(NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE);
    struct nv_ref_data_info_stru* ref_info   = (struct nv_ref_data_info_stru*)(NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE\
        +NV_GLOBAL_FILE_ELEMENT_SIZE*ctrl_info->file_num);

    struct nv_ref_data_info_stru* ref_info_next = ref_info+1;

    /*check ref list id sort */
    for(i = 0;i<ctrl_info->ref_count-1;i++)
    {
        if(ref_info->itemid >=ref_info_next->itemid)
        {
            nv_printf("i %d,itemid 0x%x,itemid_next 0x%x\n",i,ref_info->itemid,ref_info_next->itemid);
            return false;
        }
        ref_info ++;
        ref_info_next ++;
    }

    /*check file id sort*/
    for(i = 0;i<ctrl_info->file_num;i++)
    {
        if(file_info->file_id != (i+1))
        {
            nv_printf("i %d,file_id %d",i,file_info->file_id);
            return false;
        }
        file_info ++;
    }


    return true;
}
/*
 * xml decode
 * path     :xml path
 * map_path :xml map file path
 */
u32 nv_xml_decode(s8* path,s8* map_path,u32 card_type)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;

    nv_debug(NV_FUN_XML_DECODE,0,0,0,0);
    fp = nv_file_open(path,(s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        ret = BSP_ERR_NV_NO_FILE;
        nv_debug(NV_FUN_XML_DECODE,1,ret,0,0);
        return ret;
    }
    nv_mntn_record("enter to decode  %s \n",path);
    ret = xml_decode_main(fp,map_path,card_type);
    nv_file_close(fp);
    fp = NULL;
    if(ret)
    {
        nv_debug(NV_FUN_XML_DECODE,2,ret,0,0);
        goto xml_decode_err;
    }

    return NV_OK;
xml_decode_err:
    nv_mntn_record("file path :%s card type %d\n",path,card_type);
    nv_help(NV_FUN_XML_DECODE);
    return ret;
}
/*
 * read file to ddr,include download,backup,workaround,default
 */

u32 nv_read_from_file(FILE* fp,u8* ptr,u32* datalen, u32 type)
{
    u32 ret = NV_ERROR;
    struct nv_global_ddr_info_stru * ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    /* coverity[var_decl] */
    struct nv_ctrl_file_info_stru ctrl_info;
    u32 len = 0;

    *datalen = nv_get_file_len(fp);

    if((*datalen > NV_MAX_FILE_SIZE)||(*datalen == 0))
    {
        nv_mntn_record("[%s]:  datalen 0x%x\n",__func__,*datalen);
        ret = BSP_ERR_NV_FILE_ERROR;
        goto out;
    }
    /*add file head magic num check */
    ret = (u32)nv_file_read((u8*)&ctrl_info,1,sizeof(ctrl_info),fp);
    if(ret != sizeof(ctrl_info))
    {
        nv_mntn_record("[%s]:  ret 0x%x\n",__func__,ret);
        goto out;
    }
    /* coverity[uninit_use] */
    if(ctrl_info.magicnum != NV_CTRL_FILE_MAGIC_NUM)
    {
        /*add file head mntn info*/
        nv_mntn_record("[%s]:  ctrl_info.magicnum 0x%x\n",__func__,ctrl_info.magicnum);
		ret = BSP_ERR_NV_HEAD_MAGIC_ERR;
        goto out;
    }
    nv_file_seek(fp,0,SEEK_SET);

    ret = (u32)nv_file_read((u8*)ptr,1,(*datalen),fp);
    if(ret != (*datalen))
    {
        nv_mntn_record("[%s]:  ret 0x%x, datalen 0x%x\n",__func__,ret,*datalen);
        goto out;
    }

    ret = nv_init_file_info((u8*)NV_GLOBAL_CTRL_INFO_ADDR,(u8*)NV_GLOBAL_INFO_ADDR);
    if(ret)
    {
        nv_mntn_record("[%s]:  ret 0x%x\n",__func__,ret);
        goto out;
    }

    len = ddr_info->file_len;

    if((NV_FLAG_NEED_CRC == ctrl_info.crc_mark.bits.data_crc)&&(type != NV_FILE_DLOAD))
    {
        len += NV_CRC_CODE_COUNT((ddr_info->file_len - ctrl_info.ctrl_size))*sizeof(u32) + sizeof(u32);/*�����ļ�β��ʶ��*/
    }
    if((NV_FLAG_NEED_CRC == ctrl_info.crc_mark.bits.ctrl_crc)&&(type != NV_FILE_DLOAD))
    {
        len += sizeof(u32);/*�����ļ�β��ʶ����ctrl��crcУ����*/
    }

    if(*datalen != len)
    {
        nv_mntn_record("[%s]:%d,ddr len:0x%x datalen:0x%x\n",__func__,__LINE__,len, *datalen);
        ret = BSP_ERR_NV_FILE_ERROR;
        goto out;
    }
    nv_flush_cache((void*)NV_GLOBAL_CTRL_INFO_ADDR,(u32)NV_MAX_FILE_SIZE);
    return NV_OK;
out:
    nv_mntn_record("ret : 0x%x,datalen 0x%x\n",ret,*datalen);
    return NV_ERROR;
}
/*****************************************************************************
 �� �� ��  : nv_revert_default
 ��������  : ��fpָ����ļ�����ָ�NV����
 �������  : fp:    �ļ����
             offset:����ͨ�ļ������ڷ�����ƫ��
             len:   ����ͨ�ļ��ĳ���
 �������  : ��,
 �� �� ֵ  :
*****************************************************************************/
u32 nv_revert_default(FILE* fp,u32 len)
{
    u32 ret = NV_ERROR;
    u32 i = 0;
    u8* pdata = NULL;
    struct nv_ref_data_info_stru img_ref_info;
    struct nv_file_list_info_stru img_file_info;
    struct nv_comm_file_head_stru* comm_head = NULL;
    struct nv_comm_file_lookup_stru* comm_lookup = NULL;
    u32 datalen = 0;

    nv_debug(NV_FUN_REVERT_DEFAULT,20,0,0,0);

    pdata = (u8*)vmalloc(len+1);
    if(NULL == pdata)
    {
        ret = BSP_ERR_NV_MALLOC_FAIL;
        nv_debug(NV_FUN_REVERT_DEFAULT,21,ret,0,0);
        return ret;
    }

    ret = (u32)nv_file_read(pdata,1,len,fp);
    if(ret != len)
    {
        nv_debug(NV_FUN_REVERT_DEFAULT,22,0,0,0);
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto err_out ;
    }

    comm_head  = (struct nv_comm_file_head_stru*)pdata;
    comm_lookup = (struct nv_comm_file_lookup_stru*)((u8*)pdata + \
        sizeof(struct nv_comm_file_head_stru));

    for(i=0;i<comm_head->lookup_num;i++)
    {
        ret = nv_search_byid(comm_lookup->itemid, (u8*)NV_GLOBAL_CTRL_INFO_ADDR,&img_ref_info,&img_file_info);
        if(ret)
        {
            nv_mntn_record("item id 0x%x none!\n",comm_lookup->itemid);
            comm_lookup++;
            continue;
        }
        if(comm_lookup->nv_len != img_ref_info.nv_len)
        {
            nv_mntn_record("comm len :0x%x,ref info len :0x%x\n",comm_lookup->nv_len,img_ref_info.nv_len);
            comm_lookup++;
            continue;
        }
        /*ȡ��С��modem num*/
        datalen = comm_lookup->modem_num > img_ref_info.modem_num? img_ref_info.modem_num:comm_lookup->modem_num;
        datalen = comm_lookup->nv_len * datalen;

        ret = nv_write_to_mem((u8*)pdata+comm_lookup->nv_off,datalen,img_ref_info.file_id,img_ref_info.nv_off);
        if(ret)
        {
            nv_debug(NV_FUN_REVERT_DEFAULT,24,ret,0,0);
            goto err_out;
        }
        comm_lookup++;
    }
    vfree(pdata);
    return NV_OK;

err_out:
    vfree(pdata);
    nv_printf("\n");
    nv_help(NV_FUN_REVERT_DEFAULT);
    return ret;
}

/*
 * revert nv data,suport double sim card
 */
u32 nv_revert_data(s8* path,const u16* revert_data,u32 len, u32 crc_mark)
{
    FILE* fp = NULL;
    u32 ret = NV_ERROR;
    u32 i = 0;
    u32 datalen = 0;        /*read file len*/
    u32 file_offset = 0;
    u8* pdata = NULL;
    u8* nvdata = NULL;           /*single nv data ,max len 2048byte*/
    struct nv_ctrl_file_info_stru    ctrl_head_info = {0};   /*bak file ctrl file head*/
    struct nv_file_list_info_stru    bak_file_info  = {0};
    struct nv_global_ddr_info_stru   *bak_ddr_info   = NULL;
    struct nv_ref_data_info_stru     bak_ref_info   = {0};

    struct nv_ref_data_info_stru    img_ref_info  = {0};
    struct nv_file_list_info_stru   img_file_info = {0};
    u16* pidlist  = (u16*)revert_data;
    u32 modem_num = 0;

    nv_debug(NV_FUN_REVERT_DATA,0,0,0,0);
    fp = nv_file_open(path,(s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        nv_debug(NV_FUN_REVERT_DATA,1,0,0,0);
        return BSP_ERR_NV_NO_FILE;
    }

    datalen = (u32)nv_file_read((u8*)(&ctrl_head_info),1,sizeof(ctrl_head_info),fp);
    if(datalen != sizeof(ctrl_head_info))
    {
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        nv_debug(NV_FUN_REVERT_DATA,2,(u32)(unsigned long)fp,datalen,ret);
        goto close_file;
    }
    nv_file_seek(fp,0,SEEK_SET); /*jump to file head*/
    if(ctrl_head_info.magicnum != NV_CTRL_FILE_MAGIC_NUM)
    {
        ret = BSP_ERR_NV_FILE_ERROR;
        nv_debug(NV_FUN_REVERT_DATA,31,0,0,0);
        goto close_file;
    }


    if(ctrl_head_info.ctrl_size > NV_MAX_FILE_SIZE)
    {
        nv_printf("ctrl size is:0x%x too large\n", ctrl_head_info.ctrl_size);
        ret = BSP_ERR_NV_INVALID_PARAM;
        nv_debug(NV_FUN_REVERT_DATA,33,0,0,0);
        goto close_file;
    }

    pdata = (u8*)vmalloc(ctrl_head_info.ctrl_size+1);
    if(NULL == pdata)
    {
        ret = BSP_ERR_NV_MALLOC_FAIL;
        nv_debug(NV_FUN_REVERT_DATA,3,ctrl_head_info.ctrl_size,0,ret);
        goto close_file;
    }

    datalen = (u32)nv_file_read(pdata,1,ctrl_head_info.ctrl_size,fp);
    nv_file_seek(fp,0,SEEK_SET); /*jump to file head*/
    if(datalen != ctrl_head_info.ctrl_size)
    {
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        nv_debug(NV_FUN_REVERT_DATA,4,ctrl_head_info.ctrl_size,datalen,ret);
        goto free_pdata;
    }

    bak_ddr_info = (struct nv_global_ddr_info_stru *)nv_malloc(sizeof(struct nv_global_ddr_info_stru));
    if(!bak_ddr_info)
    {
        nv_debug(NV_FUN_REVERT_DATA,5,0,0,ret);
        goto free_pdata;
    }
    ret = nv_init_file_info((u8*)pdata,(u8*)bak_ddr_info);
    if(ret)
    {
        ret = BSP_ERR_NV_MEM_INIT_FAIL;
        nv_debug(NV_FUN_REVERT_DATA,6,0,0,ret);
        goto free_pdata;
    }

    nvdata = (u8*)nv_malloc(3*NV_MAX_UNIT_SIZE+1);
    if(nvdata == NULL)
    {
        ret = BSP_ERR_NV_MALLOC_FAIL;
        nv_debug(NV_FUN_REVERT_DATA,7 , NV_MAX_UNIT_SIZE,0,ret);
        goto free_pdata;
    }

    for(i = 0;i<len ;i++)
    {
        ret = nv_search_byid((u32)(*pidlist),pdata,&bak_ref_info,&bak_file_info);
        if(ret)
        {
            g_nv_ctrl.revert_search_err++;
            pidlist++;
            continue;
        }
        file_offset = bak_ddr_info->file_info[bak_file_info.file_id-1].offset +bak_ref_info.nv_off;

        /*search nv from global ddr data*/
        ret = nv_search_byid((u32)(*pidlist),(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&img_ref_info,&img_file_info);
        if(ret)
        {
            g_nv_ctrl.revert_search_err++;
            pidlist++;
            nv_debug(NV_FUN_REVERT_DATA,9,ret,(u32)*pidlist,0);
            continue;
        }

        if(img_ref_info.nv_len != bak_ref_info.nv_len)
        {
            nv_mntn_record("nv item len err,itemid :0x%x,new len:0x%x, old len :0x%x\n",\
                *pidlist,img_ref_info.nv_len,bak_ref_info.nv_len);
            g_nv_ctrl.revert_len_err++;
            pidlist++;
            continue;
        }

        modem_num = img_ref_info.modem_num > bak_ref_info.modem_num?bak_ref_info.modem_num:img_ref_info.modem_num;
        bak_ref_info.nv_len *= modem_num;
        /*jump to nv off*/
        nv_file_seek(fp,(s32)file_offset,SEEK_SET);
        datalen = (u32)nv_file_read(nvdata,1,(u32)(bak_ref_info.nv_len),fp);
        if(datalen != bak_ref_info.nv_len)
        {
            ret = BSP_ERR_NV_READ_FILE_FAIL;
            nv_debug(NV_FUN_REVERT_DATA,11,datalen,(u32)*pidlist,bak_ref_info.nv_len);
            goto free_nvdata;
        }
/*  CAT3\4���ݷ�����Ƭ���ã���������Ƭ�д�NV�ǲ���������NV������򲻻ָ�  */
        nv_file_seek(fp,0,SEEK_SET);
        if(NV_FLAG_NO_CRC == crc_mark)
        {
            ret = nv_write_to_mem_revert(nvdata,bak_ref_info.nv_len,img_file_info.file_id,img_ref_info.nv_off);
        }
        else
        {
            ret = nv_write_to_mem(nvdata,bak_ref_info.nv_len,img_file_info.file_id,img_ref_info.nv_off);
        }
        if(ret)
        {
            ret = BSP_ERR_NV_WRITE_DATA_FAIL;
            nv_debug(NV_FUN_REVERT_DATA,12,(u32)*pidlist,ret,img_file_info.file_id);
            goto free_nvdata;
        }
        pidlist++;
        g_nv_ctrl.revert_count ++;
    }
    if(pdata){vfree(pdata);}
    if(nvdata){nv_free(nvdata);}
    if(bak_ddr_info){nv_free(bak_ddr_info);}
    if(fp){nv_file_close(fp);}

    return NV_OK;

free_nvdata:
    if(nvdata){nv_free(nvdata);}
free_pdata:
    if(pdata){vfree(pdata);}

    if(bak_ddr_info){nv_free(bak_ddr_info);}
close_file:
    if(fp){nv_file_close(fp);}
    fp = NULL;
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_FUN_REVERT_DATA);
    return ret;
}



u32 nv_revertEx(const s8* path)
{
    u32 ret = NV_ERROR;

    nv_mntn_record("start to revert nv from %s!\n",path);
    ret = nv_revert_data((s8*)path,g_ausNvResumeUserIdList,\
        bsp_nvm_getRevertNum(NV_USER_ITEM), NV_FLAG_NO_CRC);
    if(ret)
    {
        nv_error_printf("ret = 0x%x\n",ret);
        return ret;
    }
    ret = nv_revert_data((s8*)path,g_ausNvResumeManufactureIdList,\
        bsp_nvm_getRevertNum(NV_MANUFACTURE_ITEM), NV_FLAG_NO_CRC);
    if(ret)
    {
        nv_error_printf("ret = 0x%x\n",ret);
        return ret;
    }
    ret = nv_revert_data((s8*)path,g_ausNvResumeSecureIdList,\
        bsp_nvm_getRevertNum(NV_SECURE_ITEM), NV_FLAG_NO_CRC);
    if(ret)
    {
        nv_error_printf("ret = 0x%x\n",ret);
        return ret;
    }
    nv_mntn_record("end of revert nv from %s!\n",path);

    return NV_OK;
}

/*
 * search nv info by nv id
 * &pdata:  data start ddr
 * output: ref_info,file_info
 */
u32 nv_search_byid(u32 itemid,u8* pdata,struct nv_ref_data_info_stru* ref_info,struct nv_file_list_info_stru* file_info)
{
    u32 low;
    u32 high;
    u32 mid;
    u32 offset;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)pdata;

    high = ctrl_info->ref_count;
    low  = 1;

    nv_debug(NV_FUN_SEARCH_NV,0,itemid,high,(u32)(unsigned long)ctrl_info);

    while(low <= high)
    {
        mid = (low+high)/2;

        offset =NV_GLOBAL_CTRL_INFO_SIZE +NV_GLOBAL_FILE_ELEMENT_SIZE*ctrl_info->file_num + (mid -1)*NV_REF_LIST_ITEM_SIZE;
        memcpy((u8*)ref_info,(u8*)ctrl_info+offset,NV_REF_LIST_ITEM_SIZE);

        if(itemid < ref_info->itemid)
        {
            high = mid-1;
        }
        else if(itemid > ref_info->itemid)
        {
            low = mid+1;
        }
        else
        {
            if(ref_info->file_id > ctrl_info->file_num)
            {
                printf("the file_id of %x is illegal\n", ref_info->itemid);
                printf("file_id = 0x%x\n",ref_info->file_id);
                printf("file_num = 0x%x\n",ctrl_info->file_num);
                return BSP_ERR_NV_NO_THIS_ID;
            }
            else
            {
                offset = NV_GLOBAL_CTRL_INFO_SIZE + NV_GLOBAL_FILE_ELEMENT_SIZE*(ref_info->file_id -1);
                memcpy((u8*)file_info,(u8*)ctrl_info+offset,NV_GLOBAL_FILE_ELEMENT_SIZE);
                return NV_OK;
            }
        }
    }
    return BSP_ERR_NV_NO_THIS_ID;

}
/*
 * copy user buff to global ddr,used to write nv data to ddr
 * &file_id :file id
 * &offset  :  offset of global file ddr
 */
u32 nv_write_to_mem_revert(u8* pdata,u32 size,u32 file_id,u32 offset)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    unsigned long nvflag;
    if(offset > ddr_info->file_info[file_id-1].size)
    {
        show_ddr_info();
        printf("[%s]:offset 0x%x\n",__FUNCTION__,offset);
        return BSP_ERR_NV_FILE_ERROR;
    }


    nv_spin_lock(nvflag, IPC_SEM_NV);
    memcpy((u8*)(NV_GLOBAL_CTRL_INFO_ADDR+ddr_info->file_info[file_id-1].offset + offset),pdata,size);/* [false alarm]:����Fortify���� */
    nv_flush_cache((u8*)(NV_GLOBAL_CTRL_INFO_ADDR+ddr_info->file_info[file_id-1].offset + offset), size);
    nv_spin_unlock(nvflag, IPC_SEM_NV);

    return NV_OK;
}

/*
 * copy user buff to global ddr,used to write nv data to ddr
 * &file_id :file id
 * &offset  :  offset of global file ddr
 */
u32 nv_write_to_mem(u8* pdata,u32 size,u32 file_id,u32 offset)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    u32 ret = 0;
    if(offset > ddr_info->file_info[file_id-1].size)
    {
        show_ddr_info();
        printf("[%s]:offset 0x%x\n",__FUNCTION__,offset);
        return BSP_ERR_NV_FILE_ERROR;
    }

    ret = nv_ipc_sem_take(IPC_SEM_NV_CRC, IPC_SME_TIME_OUT);
    if(ret)
    {
        return ret;
    }
    memcpy((u8*)(NV_GLOBAL_CTRL_INFO_ADDR+ddr_info->file_info[file_id-1].offset + offset),pdata,size);/* [false alarm]:����Fortify���� */
    ret = nv_make_nv_data_crc(ddr_info->file_info[file_id - 1].offset+ offset, size);
    if(ret)
    {
        (void)nv_ipc_sem_give(IPC_SEM_NV_CRC);
        printf("[%s]:ret = 0x%x\n",__FUNCTION__,ret);
        return BSP_ERR_NV_CRC_CODE_ERR;
    }
    nv_flush_cache((u8*)(NV_GLOBAL_CTRL_INFO_ADDR+ddr_info->file_info[file_id-1].offset + offset), size);
    (void)nv_ipc_sem_give(IPC_SEM_NV_CRC);

    return NV_OK;
}


/*
 * copy global ddr to user buff,used to read nv data from ddr
 * &file_id : file id
 * &offset:  offset of the file
 */
u32 nv_read_from_mem(u8* pdata,u32 size,u32 file_id,u32 offset)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    if(offset > ddr_info->file_info[file_id-1].size)
    {
        show_ddr_info();
        printf("[%s]:offset 0x%x\n",__FUNCTION__,offset);
        return BSP_ERR_NV_FILE_ERROR;
    }

    memcpy(pdata,(u8*)(NV_GLOBAL_CTRL_INFO_ADDR +ddr_info->file_info[file_id-1].offset + offset),size);
    return NV_OK;
}


/*
 *  acore callback of icc msg.only accept req message
 */
s32 nv_icc_msg_proc(u32 chanid ,u32 len,void* pdata)
{
    if((chanid != (ICC_CHN_NV << 16 | NV_RECV_FUNC_AC)) &&
       (chanid != (ICC_CHN_MCORE_ACORE << 16 | NV_RECV_FUNC_AM))
       )
    {
        printf("[%s] icc channel error :0x%x\n",__func__,chanid);
        return -1;
    }

    g_nv_ctrl.icc_cb_count++;
    wake_lock(&g_nv_ctrl.wake_lock);
    osl_sem_up(&g_nv_ctrl.task_sem);

    nv_debug_record(NV_DEBUG_RECEIVE_ICC);
    return 0;

}

/*
 *  nv use this inter to send data through the icc channel
 */
u32 nv_icc_send(u32 chanid,u8* pdata,u32 len)
{
    s32  ret ;
    u32  fun_id = chanid & 0xffff;/*get fun id*/
    u32  core_type ;
    u32  i = 0;

    if(fun_id == NV_RECV_FUNC_AC)
    {
        core_type = ICC_CPU_MODEM;
    }
    else if(fun_id == NV_RECV_FUNC_AM)
    {
        return NV_OK;
    }
    else
    {
        return BSP_ERR_NV_INVALID_PARAM;
    }
    /*lint -save -e578 -e530*/
    nv_debug_trace(pdata, len);
    /*lint -restore +e578 +e530*/
    for(i = 0;i<NV_ICC_SEND_COUNT;i++)
    {
        ret = bsp_icc_send(core_type,chanid,pdata,len);
        if(ICC_INVALID_NO_FIFO_SPACE == ret)/*��Ϣ������,��50ms֮�����·���*/
        {
            nv_taskdelay(50);
            continue;
        }
        else if(ret != (s32)len)
        {
            printf("[%s]:ret :0x%x,len 0x%x\n",__FUNCTION__,ret,len);
            return BSP_ERR_NV_ICC_CHAN_ERR;
        }
        else
        {
            nv_debug_record(NV_DEBUG_SEND_ICC|(((struct nv_icc_stru*)pdata)->msg_type<<16));
            return NV_OK;
        }
    }
    system_error(DRV_ERRNO_NV_ICC_FIFO_FULL,core_type,chanid,(char*)pdata,len);
    return NV_ERROR;
}

/*
 *  init icc channel used by nv module
 */
u32 nv_icc_chan_init(u32 fun_id)
{
    u32 chanid;
    if(fun_id == NV_RECV_FUNC_AC)
    {
        chanid = ICC_CHN_NV << 16 | fun_id;
    }
    else if(fun_id == NV_RECV_FUNC_AM)
    {
        chanid = ICC_CHN_MCORE_ACORE << 16 | fun_id;
    }
    else
    {
        return BSP_ERR_NV_INVALID_PARAM;
    }

     /*reg icc debug proc*/
    (void)bsp_icc_debug_register(chanid,nv_modify_pm_sw,(s32)true);

    return (u32)bsp_icc_event_register(chanid,nv_icc_msg_proc,NULL,NULL,NULL);
}
u32 nv_request_flush(void)
{
    g_nv_ctrl.acore_flush_req = NV_REQ_ACORE_DELAY_FLUSH;
    g_nv_ctrl.acore_flush_count++;

    wake_lock(&g_nv_ctrl.wake_lock);
    osl_sem_up(&g_nv_ctrl.task_sem);

    return NV_OK;
}

/*
 *  write data to file/flash/rfile,base the nv priority,inner packing write to ddr
 *  &pdata:    user buff
 *  &offset:   offset of nv in ddr
 *  &len :     data length
 */
u32 nv_write_to_file(struct nv_ref_data_info_stru* ref_info)
{
    u32 ret = NV_OK;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    /* [false alarm]:off is in using */
    u32 temp_prio = 0;
    unsigned long nvflag;

    ret = nv_pushNvFlushList(ref_info);
    if(ret)
    {
        nv_printf("push nv to list fail, ret = 0x%x\n", ret);
        return ret;
    }

    if(NV_HIGH_PRIORITY == ref_info->priority)
    {
        nv_debug_record(NV_DEBUG_FLUSH_START);
        ret = nv_flushList();
        if(ret)
        {
            nv_printf("flush list error ret = 0x%x\n", ret);
            return ret;
        }
        nv_debug_record(NV_DEBUG_FLUSH_END);
    }
    else
    {
        nv_spin_lock(nvflag, IPC_SEM_NV);

        ddr_info->priority +=\
            nv_mid_droit[ref_info->priority - NV_MID_PRIORITY1];
        temp_prio = ddr_info->priority;
        nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, (u32)NV_GLOBAL_INFO_SIZE);
        nv_spin_unlock(nvflag, IPC_SEM_NV);

        /*[false alarm]:Value Never Read*/
        if((temp_prio >= g_nv_ctrl.mid_prio) || (ddr_info->flush_info.count >= NV_FLUSH_LIST_OVER_SIZE))/*���ȼ��Ѿ��㹻,���������Ѿ��㹻*/
        {
            nv_debug_record(NV_DEBUG_REQ_FLUSH_START);
            ret = nv_request_flush();
            if(ret)
            {
                nv_printf("flush List error 2222 ret = 0x%x\n", ret);
                return ret;
            }
            nv_debug_record(NV_DEBUG_REQ_FLUSH_END);
        }
    }

    if(true == nv_isSysNv(ref_info->itemid))
    {
        ret = bsp_nvm_flushSys(ref_info->itemid);
    }
    if(NV_ERROR == ret )
    {
         return ret;
    }
    else
    {
            if(true == nv_isAutoBakupListNv(ref_info->itemid))
            {
                ret = bsp_nvm_update_default();
            }
            else
            {
                if(true == nv_isSecListNv(ref_info->itemid))
                {
                    ret = bsp_nvm_backup(NV_FLAG_NO_CRC);
                }
            }
    }
    return ret;
}

/* nv_get_key_data
 * �Ӷ�Ӧ�����и���nv id��ȡ����
 * path : �ļ�·��
 * itemid: nv id
 * buffer: ���ݻ���,����/���
 * len   : buffer len
 */
u32 nv_get_key_data(const s8* path,u32 itemid,void* buffer,u32 len)
{
    FILE* fp = NULL;
    u32 ret = NV_ERROR;
    u32 file_offset = 0;
    u8* ctrl_head = NULL;
    struct nv_ctrl_file_info_stru    ctrl_head_info = {0};   /*bak file ctrl file head*/
    struct nv_global_ddr_info_stru   *file_base_info = {0};
    struct nv_file_list_info_stru file_info = {0};
    struct nv_ref_data_info_stru ref_info  = {0};


    fp = nv_file_open(path,(s8*)NV_FILE_READ);
    if(!fp)
        return BSP_ERR_NV_NO_FILE;

    ret = (u32)nv_file_read((u8*)(&ctrl_head_info),1,sizeof(ctrl_head_info),fp);
    if(ret != sizeof(ctrl_head_info))
    {
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto out;
    }

    if(ctrl_head_info.magicnum != NV_CTRL_FILE_MAGIC_NUM)
    {
        ret = BSP_ERR_NV_FILE_ERROR;
        goto out;
    }

    if(ctrl_head_info.ctrl_size > NV_MAX_FILE_SIZE)
    {
        nv_printf("ctrl size is:0x%x too large\n", ctrl_head_info.ctrl_size);
        ret = BSP_ERR_NV_INVALID_PARAM;
        goto out;
    }

    ctrl_head = (u8*)vmalloc(ctrl_head_info.ctrl_size+1);
    if(NULL == ctrl_head)
    {
        ret = BSP_ERR_NV_MALLOC_FAIL;
        goto out;
    }

    nv_file_seek(fp,0,SEEK_SET); /*jump to file head*/
    ret = (u32)nv_file_read(ctrl_head,1,ctrl_head_info.ctrl_size,fp);
    if(ret != ctrl_head_info.ctrl_size)
    {
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto out1;
    }
    file_base_info = (struct nv_global_ddr_info_stru *)nv_malloc(sizeof(struct nv_global_ddr_info_stru));
    if(!file_base_info)
    {
        nv_printf("malloc fail\n");
        ret = BSP_ERR_NV_MALLOC_FAIL;
        goto out1;
    }
    ret = nv_init_file_info((u8*)ctrl_head,(u8*)file_base_info);
    if(ret)
        goto out1;

    ret = nv_search_byid(itemid,(u8*)ctrl_head,&ref_info,&file_info);
    if(ret)
        goto out1;
    file_offset = file_base_info->file_info[ref_info.file_id-1].offset +ref_info.nv_off;

    nv_file_seek(fp,(s32)file_offset,SEEK_SET);
    ret = (u32)nv_file_read(buffer,1, len,fp);
    if(ret != len)
    {
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto out1;
    }

    if(ctrl_head){vfree(ctrl_head);}
    if(file_base_info){nv_free(file_base_info);}
    if(fp){nv_file_close(fp);}
    return NV_OK;

out1:
    if(ctrl_head){vfree(ctrl_head);}
    if(file_base_info){nv_free(file_base_info);}
out:
    if(fp){nv_file_close(fp);}
    return ret;
}

/* nv_imei_data_comp
 * ָ�������е�imei����������е����ݶԱ�
 * path : ָ�������ļ�·��
 */
u32 nv_imei_data_comp(const s8* path)
{
    u32 ret;
    char fac_imei[16];
    char path_imei[16];
    int  len = 0;

    memset(fac_imei,0,sizeof(fac_imei));
    memset(path_imei,0,sizeof(path_imei));

    /*���������е�imei�Ż�ȡʧ�ܵ����������Ƚ�*/
    ret = nv_get_key_data((s8*)NV_DEFAULT_PATH,NV_ID_DRV_IMEI,fac_imei,sizeof(fac_imei));
    if(ret)
    {
        nv_printf("get imei from %s fail ,ret :0x%x\n",NV_DEFAULT_PATH,ret);
        return NV_OK;
    }

    /*��������imei�����ȫ0Ϊ��Ч���ݣ��򲻱Ƚ�*/
    ret = (u32)memcmp(fac_imei,path_imei,sizeof(fac_imei));
    if(!ret)
    {
        nv_printf("factory imei all 0,return direct !\n");
        return NV_OK;
    }

    ret = nv_get_key_data((s8*)path,NV_ID_DRV_IMEI,path_imei,sizeof(path_imei));
    if(BSP_ERR_NV_MALLOC_FAIL == ret)/*�����ڴ�ʧ������Ƚ�*/
    {
        nv_printf("mem malloc failed ,no compare!\n");
        return NV_OK;
    }
    if(ret)
    {
        nv_printf("get imei from %s fail ,ret :0x%x\n",path,ret);
        return ret;
    }

    ret = (u32)memcmp(fac_imei,path_imei,sizeof(fac_imei));
    if(ret)
    {
        nv_modify_print_sw(1);
        len = sizeof(fac_imei);
        nv_debug_trace(fac_imei, len);
        len = sizeof(path_imei);
        nv_debug_trace(path_imei, len);
        nv_modify_print_sw(0);
        return ret;
    }

    return NV_OK;
}



/*****************************************************************************
 �� �� ��  : nv_get_bin_file_len
 ��������  : ����nv.bin�ļ��Ĵ�С
 �������  : fp:��������ļ�
 �������  : ��
 �� �� ֵ  : �ļ���С
*****************************************************************************/
u32 nv_get_bin_file_len(struct nv_ctrl_file_info_stru* ctrl_info,struct nv_file_list_info_stru* file_info,u32 * file_len)
{
    u32 i;
    *file_len = ctrl_info->ctrl_size;

    for(i = 0;i<ctrl_info->file_num;i++)
    {
        *file_len += file_info->file_size;
        file_info ++;
    }
    if(*file_len >= NV_MAX_FILE_SIZE)
    {
        printf("[%s]:file len 0x%x,MAX size 0x%x\n",__func__,*file_len,NV_MAX_FILE_SIZE);
        return BSP_ERR_NV_ITEM_LEN_ERR;
    }
    return NV_OK;
}
/*****************************************************************************
 �� �� ��  : nv_get_bin_file_len
 ��������  : ����nv.bin+nv.bin��crcУ�����ļ��Ĵ�С
 �������  : fp:��������ļ�
 �������  : ��
 �� �� ֵ  : �ļ���С
*****************************************************************************/
u32 nv_get_bin_crc_file_len(struct nv_ctrl_file_info_stru* ctrl_info,struct nv_file_list_info_stru* file_info,u32 * file_len)
{
    u32 count = 0;
    u32 ret = NV_ERROR;

    ret = nv_get_bin_file_len(ctrl_info, file_info, file_len);
    if(ret)
    {
        nv_printf("get nv.bin len error");
        return BSP_ERR_NV_ITEM_LEN_ERR;
    }
    /*������Ҫ���ɵ�У�������*/
    if(ctrl_info->crc_mark.bits.data_crc)
    {
        count = NV_CRC_CODE_COUNT(*file_len - ctrl_info->ctrl_size);
        *file_len += count*sizeof(u32) + sizeof(u32);
    }
    if(ctrl_info->crc_mark.bits.ctrl_crc)
    {
        *file_len += sizeof(u32);
    }

    return NV_OK;
}




/* �����ַ�����CRC */
u32 nv_cal_crc32(u8 *Packet, u32 dwLength)
{
    static u32 CRC32_TABLE[256] = {
        0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
        0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
        0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
        0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
        0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
        0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
        0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
        0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
        0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
        0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
        0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
        0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
        0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
        0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
        0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
        0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
        0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
        0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
        0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
        0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
        0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
        0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
        0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
        0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
        0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
        0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
        0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
        0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
        0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
        0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
        0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
        0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
    };

    u32 CRC32 = 0;
    u32 i     = 0;


    for(i=0; i<dwLength; i++)
    {
        CRC32 = ((CRC32<<8)|Packet[i]) ^ (CRC32_TABLE[(CRC32>>24)&0xFF]); /* [false alarm]:fortify  */
    }

    for(i=0; i<4; i++)
    {
        CRC32 = ((CRC32<<8)|0x00) ^ (CRC32_TABLE[(CRC32>>24)&0xFF]); /* [false alarm]:fortify  */
    }

    return CRC32;
}
/*****************************************************************************
 �� �� ��  : nv_make_nv_ctrl_crc
 ��������  : ����NV CTRL�ε�CRCУ����
 �������  :

 �������  : NV_OK:�ɹ�
 �� �� ֵ  : ��
*****************************************************************************/
u32 nv_make_nv_ctrl_crc(void)
{
    u32 *p_ctrl_crc = (u32*)NV_DDR_CRC_CODE_OFFSET;
    u32 crcCount = 0;
    u32 len = NV_FILE_LEN - NV_FILE_CTRL_SIZE;

    if(NV_DATA_CRC_CHECK_YES)
    {
        NV_FILE_CTRL_CRC_MARK = NV_FLAG_NEED_CRC;

        crcCount = NV_CRC_CODE_COUNT(len);
        p_ctrl_crc += crcCount;
    }
    if(NV_CTRL_CRC_CHECK_YES)
    {
        *p_ctrl_crc = nv_cal_crc32((u8 *)NV_GLOBAL_CTRL_INFO_ADDR, NV_FILE_CTRL_SIZE);
    }
    return NV_OK;
}
/*****************************************************************************
 �� �� ��  : nv_make_nv_data_crc
 ��������  : ��NVд���NV��4KΪ��λ����CRCУ����,��д���ڴ��Ӧλ��
 �������  : offset:д��ƫ��,�����ctrl�ļ�ͷ
             datalen:����CRC���ݵĳ���
 �������  : NV_OK:�ɹ�
 �� �� ֵ  : ��
*****************************************************************************/
u32 nv_make_nv_data_crc(u32 offset, u32 datalen)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    u32 ret = 0;
    u8 *pCrcData = 0;
    u32 FileCrcOffset = 0;
    u32 index = 0;
    u32 crcSize = 0;
    u32 crcCount = 0;
    u32 skipCrcCount = 0;
    u32 *p_crc_code = NV_DDR_CRC_CODE_OFFSET;

    if(!NV_DATA_CRC_CHECK_YES)/*�Ƿ���Ҫ����CRCУ��*/
    {
        return NV_OK;
    }
    if((offset + datalen) > ddr_info->file_len)
    {
        return BSP_ERR_NV_INVALID_PARAM;
    }
    /*��4KΪ��λ������CRC������*/
    crcCount = NV_CRC_CODE_COUNT((offset + datalen - ctrl_info->ctrl_size)) - (offset - ctrl_info->ctrl_size)/NV_CRC32_CHECK_SIZE;
    /*��Ҫ������CRCУ�����������*/
    skipCrcCount = (offset - ctrl_info->ctrl_size)/NV_CRC32_CHECK_SIZE;
    /*��һ��CRC��4K���ݵ�ƫ��*/
    FileCrcOffset = offset - (offset - ctrl_info->ctrl_size)%NV_CRC32_CHECK_SIZE;
    /*��4KΪ��λ�����ݽ���CRCУ��*/
    for(index = 0; index < crcCount; index++)
    {
        crcSize = (ddr_info->file_len - FileCrcOffset > NV_CRC32_CHECK_SIZE)?NV_CRC32_CHECK_SIZE:(ddr_info->file_len - FileCrcOffset);
        pCrcData = (u8 *)(NV_GLOBAL_CTRL_INFO_ADDR + FileCrcOffset);
        ret = nv_cal_crc32(pCrcData, crcSize);
        p_crc_code[index + skipCrcCount] = ret;/* [false alarm]:����Fortify���� */
        FileCrcOffset += crcSize;
    }
    nv_flush_cache((u8*)(NV_GLOBAL_CTRL_INFO_ADDR + offset), crcCount*sizeof(u32));
    return NV_OK;
}

/*****************************************************************************
 �� �� ��  : nv_make_ddr_crc
 ��������  : ��NV����nv.bin����CRCУ����
 �������  : path �ļ�·��
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
u32 nv_make_ddr_crc(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    u32 file_len = 0;
    u32 count = 0;
    u32 ret = NV_ERROR;


    if(NV_DATA_CRC_CHECK_YES)
    {
        file_len = ddr_info->file_len;
        /*������Ҫ���ɵ�У�������*/
        count = NV_CRC_CODE_COUNT((file_len - ctrl_info->ctrl_size));
        /*ʣ���ڴ�ռ䲻���Ա���CRCУ����*/
        if(count*sizeof(u32) > NV_MAX_FILE_SIZE - file_len - sizeof(u32)-sizeof(u32))
        {
            nv_debug(NV_CRC32_MAKE_DDR, 0, BSP_ERR_NV_DDR_NOT_ENOUGH_ERR, count, file_len);
            nv_help(NV_CRC32_MAKE_DDR);
            return BSP_ERR_NV_DDR_NOT_ENOUGH_ERR;
        }

        /*��nv data�μ���CRCУ����*/
        ret = nv_make_nv_data_crc(ctrl_info->ctrl_size, ddr_info->file_len - ctrl_info->ctrl_size);
        if(ret)
        {
            nv_debug(NV_CRC32_MAKE_DDR, 1, ret, 0, 0);
            nv_help(NV_CRC32_MAKE_DDR);
            return ret;
        }
    }

    /*��ctrl�μ���CRCУ����*/
    ret = nv_make_nv_ctrl_crc();
    if(ret)
    {
        nv_debug(NV_CRC32_MAKE_DDR, 2, ret, 0, 0);
        return ret;
    }
    return NV_OK;
}




/*****************************************************************************
 �� �� ��  : nv_check_part_ddr_crc
 ��������  : ��һ�����ȵ�NV���ݽ���CRCУ��
 �������  : pData:����ָ��
             old_crc:ԭ����CRCУ����
             size:��ҪУ��Ĵ�С
 �������  : ��
 �� �� ֵ  : BSP_ERR_NV_CRC_CODE_ERR У�����
             NV_OK:У��ɹ�
*****************************************************************************/
u32 nv_check_part_ddr_crc(u8 *pData, u32 old_crc, u32 size)
{
    u32 new_crc = 0;

    if((NULL == pData)||(0 == size))
    {
        nv_printf("PARAM INVALID\n");
        return BSP_ERR_NV_INVALID_PARAM;
    }

    new_crc = nv_cal_crc32(pData, size);
    if(old_crc == new_crc)
    {
        return NV_OK;
    }
    else
    {
        nv_printf("pData = 0x%p old_crc = 0x%x new_crc_test = 0x%x\n", pData, old_crc, new_crc);
        return BSP_ERR_NV_CRC_CODE_ERR;
    }
}
/*****************************************************************************
 �� �� ��  : nv_check_nv_ctrl_crc
 ��������  : ���nv ctrl�ε�CRCУ����
 �������  :
 �������  : NV_OK:У��ͨ��
 �� �� ֵ  : ��
*****************************************************************************/
u32 nv_check_nv_ctrl_crc(void)
{
    u32  crcCount = 0;
    u32 *p_ctrl_crc = NULL;
    u32 ret = 0;

    if(NV_CTRL_CRC_CHECK_YES)
    {
        crcCount = NV_CRC_CODE_COUNT(NV_FILE_LEN - NV_FILE_CTRL_SIZE);
        p_ctrl_crc = NV_DDR_CRC_CODE_OFFSET + crcCount;
        ret = nv_check_part_ddr_crc((u8 *)NV_GLOBAL_CTRL_INFO_ADDR,  *p_ctrl_crc,  NV_FILE_CTRL_SIZE);
        if(ret)
        {
            nv_printf("check crc error\n");
            return ret;
        }
    }
    return NV_OK;
}
/*****************************************************************************
 �� �� ��  : nv_check_nv_data_crc
 ��������  : ��ƫ��Ϊoffset������Ϊdatalen�����ݰ���4kΪ��λ������CRCУ��
 �������  : offset:Ҫд������ݵ�ƫ�ƣ�����ڿ����ļ�ͷ
             datalen:��У������ݳ���
 �������  : NV_OK:У��ͨ��
 �� �� ֵ  : ��
*****************************************************************************/
u32 nv_check_nv_data_crc(u32 offset, u32 datalen)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    u32 ret = 0;
    u8 *pCrcData = 0;
    u32 FileCrcOffset = 0;
    u32 index = 0;
    u32 old_crc = 0;
    u32 crc_size = 0;
    u32 crcCount = 0;
    u32 skipCrcCount = 0;
    u32 *p_crc_code = NV_DDR_CRC_CODE_OFFSET;

    if(!NV_DATA_CRC_CHECK_YES)/*�Ƿ���Ҫ����CRCУ��*/
    {
        return NV_OK;
    }
    if((offset + datalen) > ddr_info->file_len)
    {
        return BSP_ERR_NV_INVALID_PARAM;
    }
    /*��4KΪ��λ������CRC������*/
    crcCount = NV_CRC_CODE_COUNT((offset + datalen- ctrl_info->ctrl_size)) - (offset- ctrl_info->ctrl_size)/NV_CRC32_CHECK_SIZE;
    /*��Ҫ������CRCУ�����������*/
    skipCrcCount = (offset - ctrl_info->ctrl_size)/NV_CRC32_CHECK_SIZE;
    /*��һ��CRC��4K���ݵ�ƫ��*/
    FileCrcOffset = offset - (offset - ctrl_info->ctrl_size)%NV_CRC32_CHECK_SIZE;
    /*��4KΪ��λ�����ݽ���CRCУ��*/
    for(index = 0;index < crcCount; index++)
    {
        crc_size = (ddr_info->file_len - FileCrcOffset > NV_CRC32_CHECK_SIZE)?NV_CRC32_CHECK_SIZE:(ddr_info->file_len - FileCrcOffset);
        old_crc =  p_crc_code[index + skipCrcCount];
        pCrcData = (u8 *)(NV_GLOBAL_CTRL_INFO_ADDR + FileCrcOffset);
        ret = nv_check_part_ddr_crc(pCrcData, old_crc, crc_size);
        if(ret)
        {
            nv_printf("nv_check_part_ddr_crc error 2, index = 0x%x ret = 0x%x \n", index, ret);
            return ret;
        }
        FileCrcOffset += crc_size;
    }
    return NV_OK;
}

/*****************************************************************************
 �� �� ��  : nv_crc_make_crc
 ��������  : ��NV����nv.bin����CRCУ����
 �������  : path �ļ�·��
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
u32 nv_check_ddr_crc(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    u32 ret = NV_ERROR;

    /*���data�ε�CRCУ����*/
    ret = nv_check_nv_data_crc(ctrl_info->ctrl_size , ddr_info->file_len - ctrl_info->ctrl_size);
    if(ret)
    {
        nv_printf("check crc error\n");
        nv_debug(NV_CRC32_DDR_CRC_CHECK, ret, 0, 0, 0);
        nv_help(NV_CRC32_DDR_CRC_CHECK);

        return ret;
    }

    /*���ctrl�ε�CRCУ����*/
    ret = nv_check_nv_ctrl_crc();
    if(ret)
    {
        return ret;
    }
    return NV_OK;
}


/*****************************************************************************
 �� �� ��  : nv_dload_file_crc_check
 ��������  : ������������CRCУ��
 �������  : fp�ļ�ָ��
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
u32 nv_dload_file_crc_check(FILE *fp)
{
    return NV_OK;
}

u32 nv_resume_ddr_from_img(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    u32 ret = NV_ERROR;
    unsigned long nvflag = 0;

    nv_debug(NV_CRC32_DDR_RESUME_IMG,0,0,0,0);
    if(!nv_debug_is_resume_img())
    {
        nv_mntn_record("nv resume cfg not %s ...%s %s \n",NV_IMG_PATH,__DATE__,__TIME__);
        return NV_OK;
    }
    else
    {
        nv_mntn_record("nv resume %s ...%s %s \n",NV_IMG_PATH,__DATE__,__TIME__);
    }

	/*lock write right*/
    nv_spin_lock(nvflag, IPC_SEM_NV);
    ddr_info->acore_init_state = NV_KERNEL_INIT_DOING;
    nv_spin_unlock(nvflag, IPC_SEM_NV);
    nv_flush_cache((u8*)NV_GLOBAL_START_ADDR, SHM_MEM_NV_SIZE);

    (void)nv_debug_store_ddr_data();

    ret = bsp_nvm_reload();
    if(ret)
    {
        nv_mntn_record("NV resume fail ...%s %s \n",__DATE__,__TIME__);
    }
    else
    {
        /*unlock wirte right*/
        nv_spin_lock(nvflag, IPC_SEM_NV);
        ddr_info->acore_init_state = NV_INIT_OK;
        nv_spin_unlock(nvflag, IPC_SEM_NV);
        nv_flush_cache((u8*)NV_GLOBAL_START_ADDR, SHM_MEM_NV_SIZE);
        nv_mntn_record("NV resume OK ...%s %s \n",__DATE__,__TIME__);
    }

    if(nv_debug_is_reset())
    {
        system_error(DRV_ERRNO_NV_CRC_ERR, NV_CRC32_DDR_RESUME_IMG, 2, NULL, 0);
    }
    return ret;
}
/*****************************************************************************
 �� �� ��  : nv_set_crc_offset
 ��������  : ����crc��ƫ��
 �������  : void
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
u32 nv_set_crc_offset(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_file_list_info_stru* file_info = (struct nv_file_list_info_stru*)((unsigned long)NV_GLOBAL_CTRL_INFO_ADDR + NV_GLOBAL_CTRL_INFO_SIZE);
    u32 fileLen = 0;
    u32 ret = 0;
    unsigned long nvflag = 0;
    if(!(NV_DATA_CRC_CHECK_YES||NV_CTRL_CRC_CHECK_YES))
    {
        return NV_OK;
    }

    nv_spin_lock(nvflag, IPC_SEM_NV);
    ret = nv_get_bin_file_len(ctrl_info, file_info, &fileLen);
    if(ret)
    {
        nv_spin_unlock(nvflag, IPC_SEM_NV);
        nv_printf("get file len error\n");
        return ret;
    }
    ddr_info->p_crc_code = (unsigned long)SHD_DDR_V2P((void *)(NV_GLOBAL_CTRL_INFO_ADDR + fileLen + sizeof(u32)));
    nv_spin_unlock(nvflag, IPC_SEM_NV);
    return NV_OK;
}
/*****************************************************************************
 �� �� ��  : nv_flushList
 ��������  : ��ʼ���ػ�д������
 �������  : void
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void nv_flushListInit(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    memset(&(ddr_info->flush_info), 0, sizeof(struct nv_flush_list_stru));
    nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, sizeof(struct nv_global_ddr_info_stru*));
    return;
}
/*****************************************************************************
 �� �� ��  : nv_pushNvFlushList
 ��������  : ���nv��������
 �������  : void
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
u32  nv_pushNvFlushList(struct nv_ref_data_info_stru* ref_info)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    unsigned long nvflag = 0;
    u32 index = 0;
    u32 i = 0;
    u32 ret = 0;

    if((nv_pop_list_to_glb_list())&&(ref_info->priority > 0))
    {
        ddr_info->priority += nv_mid_droit[0];
    }


    osl_sem_down(&g_nv_ctrl.nv_list_sem);
    nv_spin_lock(nvflag, IPC_SEM_NV);
    index = ddr_info->flush_info.count;
    for(i = 0; i < ddr_info->flush_info.count; i++)
    {
        if(ddr_info->flush_info.list[i].itemid == ref_info->itemid)
        {
            break;
        }
    }

    ret = nv_check_local_list(ref_info->itemid);

    if(ret && (i >= ddr_info->flush_info.count))
    {
        if(ddr_info->flush_info.count >= NV_FLUSH_LIST_SIZE)
        {
            ret = nv_push_to_local_list(ref_info);
            if(ret)
            {
                nv_spin_unlock(nvflag, IPC_SEM_NV);
                osl_sem_up(&g_nv_ctrl.nv_list_sem);
                return ret;
            }
        }
        else
        {
            index = ddr_info->flush_info.count;
            /* coverity[overrun-local] */
            ddr_info->flush_info.list[index].itemid = ref_info->itemid;
            ddr_info->flush_info.list[index].nv_len= ref_info->nv_len;
            ddr_info->flush_info.list[index].nv_off= ddr_info->file_info[ref_info->file_id-1].offset + ref_info->nv_off;
            ddr_info->flush_info.count++;
            nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, (u32)NV_GLOBAL_INFO_SIZE);
        }
    }

    nv_spin_unlock(nvflag, IPC_SEM_NV);
    osl_sem_up(&g_nv_ctrl.nv_list_sem);


    return NV_OK;
}

/*****************************************************************************
 �� �� ��  : nv_flushList
 ��������  : ��ʼ���ػ�д������
 �������  : void
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void nv_flushListMgr(struct nv_flush_list_stru * flush_info, struct nv_crc_flush_info_stru *CrcOffset)
{
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    u32 i = 0;
    u32 j = 0;
    u32 crcCount = 0;
    u32 skipCrcCount = 0;
    u32 crc_index = 0;
    u32 temp_skip_crc_count = 0;

    memset((void*)CrcOffset, 0, sizeof(struct nv_crc_flush_info_stru));
    for(i = 0; i < flush_info->count; i++)
    {
        skipCrcCount = (flush_info->list[i].nv_off - ctrl_info->ctrl_size)/NV_CRC32_CHECK_SIZE;
        crcCount = NV_CRC_CODE_COUNT(flush_info->list[i].nv_off + flush_info->list[i].nv_len - ctrl_info->ctrl_size) - skipCrcCount;
        for(crc_index = 0; crc_index < crcCount; crc_index++)
        {
            temp_skip_crc_count = skipCrcCount + crc_index;

            for(j = 0; j < CrcOffset->count; j++)
            {
                if(CrcOffset->offsetArray[j] == temp_skip_crc_count)
                {
                    break;
                }
            }
            if((j == CrcOffset->count)||(0 == CrcOffset->count))
            {
                CrcOffset->offsetArray[CrcOffset->count++] = temp_skip_crc_count;
            }
        }
    }
    return;
}

/*****************************************************************************
 �� �� ��  : nv_flushList
 ��������  : ���ڴ��е�����д�뵽flash��
 �������  : void
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
u32 nv_flushList(void)
{
    struct nv_global_ddr_info_stru* shared_ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_flush_list_stru  *p_flush_info = NULL;
    struct nv_crc_flush_info_stru *pCrcOffset = NULL;/*�����Ҫд���ļ���4K���ݿ��ƫ��*/
    unsigned long nvflag = 0;
    u32 flush_info_len = 0;
    u32 ret = 0;
    u32 i = 0;
    u32 off = 0;
    u32 len = 0;

    pCrcOffset = (struct nv_crc_flush_info_stru *)nv_malloc(sizeof(struct nv_crc_flush_info_stru));
    if(NULL == pCrcOffset)
    {
        nv_printf("malloc fail\n");
        return BSP_ERR_NV_MALLOC_FAIL;
    }
    flush_info_len = (NV_FLUSH_LIST_SIZE * sizeof(struct nv_flush_info_stru) + sizeof(u32));
    p_flush_info = (struct nv_flush_list_stru *)nv_malloc(flush_info_len);
    if(NULL == p_flush_info)
    {
        nv_free(pCrcOffset);
        nv_printf("malloc fail\n");
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    nv_debug(NV_API_FLUSH_LIST,0,0,0,0);
    do{
        nv_spin_lock(nvflag, IPC_SEM_NV);
        /* coverity[overrun-buffer-arg] */
        memcpy((void *)p_flush_info, &(shared_ddr_info->flush_info), flush_info_len);
        memset(&(shared_ddr_info->flush_info), 0, sizeof(struct nv_flush_list_stru));
        memset(&(shared_ddr_info->priority),0,sizeof(shared_ddr_info->priority));/*clear*/
        nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, sizeof(struct nv_global_ddr_info_stru));
        nv_spin_unlock(nvflag, IPC_SEM_NV);

        /*д���ļ�ǰ�鿴�Ƿ���Ҫ����*/
        (void)nv_rebak_check(p_flush_info);
        nv_flushListMgr(p_flush_info, pCrcOffset);

        for(i = 0;i < pCrcOffset->count; i++)
        {
            off = (pCrcOffset->offsetArray[i])*NV_CRC32_CHECK_SIZE + ctrl_info->ctrl_size;
            len = (shared_ddr_info->file_len - off > NV_CRC32_CHECK_SIZE)? NV_CRC32_CHECK_SIZE:shared_ddr_info->file_len - off;
            ret = bsp_nvm_flushEx(off , len, NV_ERROR);
            if(ret)
            {
                nv_debug(NV_API_FLUSH_LIST,1,ret,0,0);
                nv_mntn_record("\n[%s]\n",__FUNCTION__);
                nv_help(NV_API_FLUSH_LIST);
                nv_free((void *)p_flush_info);
                nv_free((void *)pCrcOffset);
                return ret;
            }
        }
    }while(nv_pop_list_to_glb_list());

    /*д��ɹ�����ݰ�����д�뱸�ݱ��*/
    (void)nv_rebak_save_flag();

    nv_free((void *)p_flush_info);
    nv_free((void *)pCrcOffset);
    return NV_OK;
}
/*****************************************************************************
 �� �� ��  : nv_check_local_list
 ��������  : �鿴ָ����nvid��local list���Ƿ����
 �������  : void
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
u32 nv_check_local_list(u32 itemid)
{
    struct list_head * me = NULL;
    struct nv_flush_list_node_stru * cur = NULL;

    list_for_each(me, &g_nv_ctrl.nv_list)
    {
        cur = list_entry(me, struct nv_flush_list_node_stru, stList);/*������ͬ��nv id*/
        if(cur->itemid == itemid)
        {
            return NV_OK;
        }
    }
    return NV_ERROR;
}
/*****************************************************************************
 �� �� ��  : nv_push_to_local_list
 ��������  : �����˵�nvд����Ϣд�뵽������
 �������  : void
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
u32 nv_push_to_local_list(struct nv_ref_data_info_stru* ref_info)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_flush_list_node_stru * cur = NULL;
    struct list_head * me = NULL;
    u32 count = 0;

    cur = NULL;
    /* [false alarm]:test using */
    /* coverity[alloc_fn] *//* coverity[var_assign] */
    cur = (struct nv_flush_list_node_stru*)nv_malloc(sizeof(struct nv_flush_list_node_stru));/* [false alarm]:test using */
    if(!cur)
    {
        nv_printf("malloc fail\n");
        return BSP_ERR_NV_MALLOC_FAIL;
    }
    cur->itemid = ref_info->itemid;
    cur->nv_len = ref_info->nv_len;
    cur->nv_off = ddr_info->file_info[ref_info->file_id-1].offset + ref_info->nv_off;


    list_add_tail(&cur->stList, &g_nv_ctrl.nv_list);


    list_for_each(me, &g_nv_ctrl.nv_list)
    {
        count++;
    }
    if(count > 2*NV_FLUSH_LIST_SIZE)
    {
        nv_printf("nv local list is full\n");
        system_error(DRV_ERRNO_NV_LIST_FULL, count, 0, NULL, 0);
    }
    /* coverity[leaked_storage] */
    return NV_OK;
}
/*****************************************************************************
 �� �� ��  : nv_pop_list_to_glb_list
 ��������  : �����˵�nvд����Ϣ����������ڴ���
 �������  : void
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
u32 nv_pop_list_to_glb_list(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct list_head * me = NULL;
    struct nv_flush_list_node_stru * cur = NULL;
    unsigned long nvflag = 0;
    u32 list_del_mark = 0;
    u32 push_count = 0;

    osl_sem_down(&g_nv_ctrl.nv_list_sem);
    nv_spin_lock(nvflag, IPC_SEM_NV);

    list_for_each(me, &g_nv_ctrl.nv_list)
    {
        if(list_del_mark)
        {
            list_del(&(cur->stList));/* [false alarm]:fortify  */
            nv_free(cur);
            list_del_mark = 0;
        }
        if(ddr_info->flush_info.count < NV_FLUSH_LIST_OVER_SIZE)
        {
            cur = list_entry(me, struct nv_flush_list_node_stru, stList);/*������ͬ��nv id*/
            push_count++;
            list_del_mark = 1;
            if(ddr_info->flush_info.list[ddr_info->flush_info.count].itemid == cur->itemid)
            {
                continue;
            }
            ddr_info->flush_info.list[ddr_info->flush_info.count].itemid = cur->itemid;
            ddr_info->flush_info.list[ddr_info->flush_info.count].nv_len= cur->nv_len;
            ddr_info->flush_info.list[ddr_info->flush_info.count].nv_off= cur->nv_off;
            ddr_info->flush_info.count++;
        }
        else
        {
            break;
        }
    }
    nv_spin_unlock(nvflag, IPC_SEM_NV);

    if(list_del_mark)
    {
        list_del(&(cur->stList));
        nv_free(cur);
    }

    osl_sem_up(&g_nv_ctrl.nv_list_sem);
    return push_count;
}

void show_local_list(void)
{
    struct list_head * me = NULL;
    struct nv_flush_list_node_stru * cur = NULL;
    list_for_each(me, &g_nv_ctrl.nv_list)
    {
        cur = list_entry(me, struct nv_flush_list_node_stru, stList);
        nv_printf("nv id :0x%x\n", cur->itemid);
        nv_printf("nv len:0x%x\n", cur->nv_len);
        nv_printf("nv off:0x%x\n", cur->nv_off);
    }
    return;
}

u32 nv_check_factory_nv(u32 list_nvid, u32 modem_num, u32 nv_crc_value)
{
    struct nv_ref_data_info_stru ref_info = {};
    struct nv_file_list_info_stru file_info = {};
    NV_CHECK_ITEM_STRU * p_check_item = NULL;
    u8 * temp_nv_data = NULL;
    u8 * pdata = NULL;
    u32 ret = 0;
    u32 len = 0;
    u32 nvid_index = 0;
    u32 crc_value = 0;
    /*read nv list*/
    len = sizeof(NV_CHECK_ITEM_STRU);

    /* coverity[alloc_fn] *//* coverity[var_assign] */
    p_check_item = (NV_CHECK_ITEM_STRU *)nv_malloc(len);
    if(!p_check_item)
    {
        nv_mntn_record("malloc p_check_item fail\n");
        return BSP_ERR_NV_MALLOC_FAIL;
    }
    /* coverity[noescape] */
    ret = bsp_nvm_read(list_nvid, (u8 *)p_check_item, len);
    if(ret)
    {
        nv_free(p_check_item);
        nv_mntn_record("read nv data 0x%x fail\n", list_nvid);
        return ret;
    }
    if(p_check_item->uhwValidCount > MAX_CHECK_NUM)
    {
        nv_mntn_record("nv list size is larger, uhwValidCount: 0x%x \n", p_check_item->uhwValidCount);
        nv_free(p_check_item);
        return BSP_ERR_NV_OVER_MEM_ERR;
    }

    pdata = (u8 *)nv_malloc(sizeof(u32) + 2*NV_MAX_UNIT_SIZE);
    if(!pdata)
    {
        nv_free(p_check_item);
        nv_mntn_record("malloc pdata fail\n");
        return BSP_ERR_NV_MALLOC_FAIL;
    }
    temp_nv_data = pdata;
    len = 0;
    for(nvid_index = 0; nvid_index < p_check_item->uhwValidCount; nvid_index++)
    {
        ret = nv_search_byid(p_check_item->auhwNeedCheckID[nvid_index], NV_GLOBAL_CTRL_INFO_ADDR, &ref_info, &file_info);
        if(ret)
        {
            nv_mntn_record("search id 0x%x fail, ret = 0x%x\n", p_check_item->auhwNeedCheckID[nvid_index], ret);
            nv_free(pdata);
            nv_free(p_check_item);
            return ret;
        }
        ret = bsp_nvm_dcread(modem_num, p_check_item->auhwNeedCheckID[nvid_index], temp_nv_data, ref_info.nv_len);
        if(ret)
        {
            nv_mntn_record("read nv 0x%x fail, ret = 0x%x\n", p_check_item->auhwNeedCheckID[nvid_index], ret);
            nv_free(pdata);
            nv_free(p_check_item);

            return ret;
        }
        len += ref_info.nv_len;
        crc_value = nv_cal_crc32(pdata, len);

        temp_nv_data = (u8*)((unsigned long)pdata + sizeof(u32));
        len = sizeof(u32);
        *(u32 *)pdata = crc_value;
    }

    nv_free(pdata);
    nv_free(p_check_item);

    if(crc_value == nv_crc_value)
    {
        return NV_OK;
    }
    else
    {
        nv_printf("new crc:0x%x, old crc:0x%x\n", crc_value, nv_crc_value);
        return BSP_ERR_NV_CRC_CODE_ERR;
    }

}
/*****************************************************************************
 �� �� ��  : nv_check_factory_nv_status
 ��������  : У��У׼NV��CRCУ����
 �������  : u32 modeȡֵ0: GU���� 1:TL���� 2:GU����
 �������  : ��
 �� �� ֵ  : 0:У��ͨ��
             0xxxxx027:CRCУ�����
             ����������ִ�й����г���
*****************************************************************************/
u32 nv_check_factory_nv_status(u32 mode)
{
    NV_CRC_CHECK_RESULT_STRU crc_result_stru = {};
    u32 list_nvid= 0;
    u32 modem_num = 0;
    u32 crc_value = 0;
    u32 ret = 0;

    /*read crc value,nv 50000 is defined by product line*/
    ret = bsp_nvm_read(NV_ID_CRC_CHECK_RESULT, (u8*)&crc_result_stru, sizeof(NV_CRC_CHECK_RESULT_STRU));
    if(ret)
    {
        nv_mntn_record("read crc result id:0x%x fail\n", NV_ID_CRC_CHECK_RESULT);
        return ret;
    }

    switch(mode)
    {
        case NV_FACTORY_CHECK_M1_GU:
            list_nvid = NV_ID_GU_CHECK_ITEM;
            modem_num = NV_USIMM_CARD_1;
            crc_value = crc_result_stru.uwGuCrcResult;
            break;
        case NV_FACTORY_CHECK_M1_TL:
            list_nvid = NV_ID_TL_CHECK_ITEM;
            modem_num = NV_USIMM_CARD_1;
            crc_value = crc_result_stru.uwTlCrcResult;
            break;
        case NV_FACTORY_CHECK_M2_GU:
            list_nvid = NV_ID_GU_M2_CHECK_ITEM;
            modem_num = NV_USIMM_CARD_2;
            crc_value = crc_result_stru.uwGuM2CrcResult;
            break;
        default:
            nv_printf("para error\n");
            return BSP_ERR_NV_INVALID_PARAM;
    }

    /*У��NVֵ��CRCУ����*/
    return nv_check_factory_nv(list_nvid, modem_num, crc_value);
}

void show_ddr_info(void)
{
    u32 i;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_file_list_info_stru* file_info = (struct nv_file_list_info_stru*)(NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE);
    struct nv_ref_data_info_stru* ref_info   = (struct nv_ref_data_info_stru*)(NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE\
        +NV_GLOBAL_FILE_ELEMENT_SIZE*ctrl_info->file_num);

    printf("global start ddr        :0x%p\n",NV_GLOBAL_INFO_ADDR);
    printf("global ctrl file ddr    :0x%p\n",NV_GLOBAL_CTRL_INFO_ADDR);
    printf("global file list ddr    :0x%p\n",file_info);
    printf("global ref info  ddr    :0x%p\n",ref_info);
    printf("icc core type           :0x%x\n",g_nv_ctrl.icc_core_type);
    printf("*******************ddr global ctrl************************\n");
    printf("acore init state    : 0x%x\n",ddr_info->acore_init_state);
    printf("ccore init state    : 0x%x\n",ddr_info->ccore_init_state);
    printf("ddr read case       : 0x%x\n",ddr_info->ddr_read);
    printf("crc status          : 0x%x\n",ctrl_info->crc_mark.value);
    printf("crc code addr       : 0x%x\n",ddr_info->p_crc_code);
    printf("mid priority 		: 0x%x\n",ddr_info->priority);
    nv_mntn_record("mem file type   : 0x%x\n",ddr_info->mem_file_type);
    nv_mntn_record("total revert count      :%d\n",g_nv_ctrl.revert_count);
    nv_mntn_record("revert search err count :%d\n",g_nv_ctrl.revert_search_err);
    nv_mntn_record("revert len err count    :%d\n",g_nv_ctrl.revert_len_err);
    nv_mntn_record("file total len  : 0x%x\n",ddr_info->file_len);
    nv_mntn_record("comm file num   : 0x%x\n",ddr_info->file_num);
    nv_mntn_record("nv resume mode  : 0x%x\n", g_nv_ctrl.nv_self_ctrl.ulResumeMode);

    if(ddr_info->acore_init_state <= NV_INIT_FAIL)
	return;
    for(i = 0;i<ddr_info->file_num;i++)
    {
        nv_mntn_record("##############################\n");
        nv_mntn_record("** file id   0x%x\n",ddr_info->file_info[i].file_id);
        nv_mntn_record("** file size 0x%x\n",ddr_info->file_info[i].size);
        nv_mntn_record("** file off  0x%x\n",ddr_info->file_info[i].offset);
    }

    nv_mntn_record("*******************global ctrl file***********************\n");
    nv_mntn_record("ctrl file size    : 0x%x\n",ctrl_info->ctrl_size);
    nv_mntn_record("file num          : 0x%x\n",ctrl_info->file_num);
    nv_mntn_record("file list off     : 0x%x\n",ctrl_info->file_offset);
    nv_mntn_record("file list size    : 0x%x\n",ctrl_info->file_size);
    nv_mntn_record("ctrl file magic   : 0x%x\n",ctrl_info->magicnum);
    nv_mntn_record("modem num         : 0x%x\n",ctrl_info->modem_num);
    nv_mntn_record("nv count          : 0x%x\n",ctrl_info->ref_count);
    nv_mntn_record("nv ref data off   : 0x%x\n",ctrl_info->ref_offset);
    nv_mntn_record("nv ref data size  : 0x%x\n",ctrl_info->ref_size);
    nv_mntn_record("*******************global file list***********************\n");
    for(i = 0;i<ctrl_info->file_num;i++)
    {
        nv_mntn_record("file_info     : 0x%x\n",file_info);
        nv_mntn_record("file id       : 0x%x\n",file_info->file_id);
        nv_mntn_record("file name     : %s\n",file_info->file_name);
        nv_mntn_record("file size     : 0x%x\n",file_info->file_size);
        file_info ++;
        nv_mntn_record("\n");
    }
}


void show_ref_info(u32 arg1,u32 arg2)
{
    u32 i;
    u32 _max;
    u32 _min;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_ref_data_info_stru* ref_info   = (struct nv_ref_data_info_stru*)(NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE\
        +NV_GLOBAL_FILE_ELEMENT_SIZE*ctrl_info->file_num);

    _max = arg2 > ctrl_info->ref_count ? ctrl_info->ref_count:arg2;
    _min = arg1 > _max ? 0: arg1;

    _max = (arg2 ==0)?ctrl_info->ref_count: _max;

    ref_info = ref_info+_min;

    for(i = _min;i<_max;i++)
    {
        printf("��%d�� :\n",i);
        printf("nvid   :0x%-8x, file id : 0x%-8x\n",ref_info->itemid,ref_info->file_id);
        printf("nvlen  :0x%-8x, nv_off  : 0x%-8x, nv_pri 0x%-8x\n",ref_info->nv_len,ref_info->nv_off,ref_info->priority);
        printf("dsda   :0x%-8x\n",ref_info->modem_num);
        ref_info++;
    }
}

void nv_show_fastboot_err(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    nv_mntn_record("**************************************\n");
    nv_mntn_record("line       :%d\n",  ddr_info->fb_debug.line);
    nv_mntn_record("result     :0x%x\n",ddr_info->fb_debug.ret);
    nv_mntn_record("reseved1   :0x%x\n",ddr_info->fb_debug.reseverd1);
    nv_mntn_record("reseved2   :0x%x\n",ddr_info->fb_debug.reseverd2);
    nv_mntn_record("**************************************\n");
}


/* [false alarm]:i is in using */
void nv_help(u32 type)
{
    /*[false alarm]:i is in using*/
    u32 i = type;
    if(type == 63/*'?'*/)
    {
        printf("1   -------  read\n");
        printf("4   -------  auth read\n");
        printf("5   -------  write\n");
        printf("6   -------  auth write\n");
        printf("8   -------  get len\n");
        printf("9   -------  auth get len\n");
        printf("10  -------  flush\n");
        printf("12  -------  backup\n");
        printf("15  -------  import\n");
        printf("16  -------  export\n");
        printf("19  -------  kernel init\n");
        printf("20  -------  remain init\n");
        printf("21  -------  nvm init\n");
        printf("22  -------  xml decode\n");
        printf("24  -------  revert\n");
        printf("25  -------  update default\n");
        return;

    }
    if(type == NV_FUN_MAX_ID)
    {
        for(i = 0;i< NV_FUN_MAX_ID;i++)
        {
            printf("************fun id %d******************\n",i);
            printf("call num             : 0x%x\n",g_nv_debug[i].callnum);
            printf("out branch (reseved1): 0x%x\n",g_nv_debug[i].reseved1);
            printf("reseved2             : 0x%x\n",g_nv_debug[i].reseved2);
            printf("reseved3             : 0x%x\n",g_nv_debug[i].reseved3);
            printf("reseved4             : 0x%x\n",g_nv_debug[i].reseved4);
            printf("***************************************\n");
        }
        return ;
    }

    i = type;
    nv_mntn_record("************fun id %d******************\n",i);
    nv_mntn_record("call num             : 0x%x\n",g_nv_debug[i].callnum);
    nv_mntn_record("out branch (reseved1): 0x%x\n",g_nv_debug[i].reseved1);
    nv_mntn_record("reseved2             : 0x%x\n",g_nv_debug[i].reseved2);
    nv_mntn_record("reseved3             : 0x%x\n",g_nv_debug[i].reseved3);
    nv_mntn_record("reseved4             : 0x%x\n",g_nv_debug[i].reseved4);
    nv_mntn_record("***************************************\n");
}


u32 nv_show_ref_info(u16 itemid)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ref_data_info_stru ref_info = {};
    struct nv_file_list_info_stru file_info = {};
    u32 nv_phy_addr = 0;
    void* nv_virt_addr = 0;
    u32 ret = 0;

    ret = nv_search_byid((u32)itemid, (u8*)NV_GLOBAL_CTRL_INFO_ADDR, &ref_info, &file_info);
    if(ret)
    {
        return NV_ERROR;
    }
    nv_printf("itemid = 0x%x\n", ref_info.itemid);
    nv_printf("nv_len = 0x%x\n", ref_info.nv_len);
    nv_printf("nv_off = 0x%x\n", ref_info.nv_off);
    nv_printf("file_id = 0x%x\n", ref_info.file_id);
    nv_printf("priority = 0x%x\n", ref_info.priority);
    nv_printf("modem_num = 0x%x\n", ref_info.modem_num);

    nv_printf("file_id = 0x%x\n", file_info.file_id);
    nv_printf("file_name = %s\n", file_info.file_name);
    nv_printf("file_size = 0x%x\n", file_info.file_size);

    nv_virt_addr = NV_GLOBAL_CTRL_INFO_ADDR + ddr_info->file_info[ref_info.file_id - 1].offset + ref_info.nv_off;

    nv_phy_addr = (u32)(unsigned long)SHD_DDR_V2P(nv_virt_addr);

    nv_printf("nv_virt_addr = 0x%x\n", nv_virt_addr);
    nv_printf("nv_phy_addr = 0x%x\n", nv_phy_addr);

    return NV_OK;
}
void nv_show_crc_status(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;

    printf("crc status          : 0x%x\n",ctrl_info->crc_mark.value);
    printf("crc code addr       : 0x%x\n",ddr_info->p_crc_code);
}

/*lint -restore*/
/*****************************************************************************
 �� �� ��  : nv_check_item_len
 ��������  : ���nv id��start��end֮��ĸ���nv��ĳ���
             ���������ֽڶ����nv id�͸�nv��ĳ��ȴ�ӡ����
 �������  : start:  nv id����ʼֵ
             end  :  nv id�Ľ���ֵ
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
u32 nv_check_item_len(u32 start, u32 end)
{
    u32 i;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_ref_data_info_stru* ref_info   = (struct nv_ref_data_info_stru*)(NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE\
        +NV_GLOBAL_FILE_ELEMENT_SIZE*ctrl_info->file_num);
    u32 item_num = 0;

    nv_printf("==================================\n");
    for(i = 0;i<ctrl_info->ref_count;i++)
    {

        if(((ref_info->nv_len)%4 != 0)&&(start <= ref_info->itemid)&&(end >= ref_info->itemid))
        {
           item_num++;
           nv_printf("nv id = 0x%x nv len = 0x%x\n ", ref_info->itemid, ref_info->nv_len);
        }
        ref_info++;
    }
    nv_printf("==========num = 0x%x ===============\n", item_num);

    return NV_OK;
}

/*****************************************************************************
 �� �� ��  : nv_writepart_to_flash
 ��������  : ��flash��д�벿��nvֵ
 �������  : path -- ��д���flash����·��
             nvlist -- ��д���nv�б�
             len -- �б���
             crc_flag -- �Ƿ����crcУ��,������չʹ��
 �������  : ��
 �� �� ֵ  : NV_OK / NV_ERROR
*****************************************************************************/
static u32 nv_writepart_to_flash(s8 *path, const u16 *nvlist, u32 len, bool crc_flag)
{
    FILE* fp = NULL;
    u32 uret = NV_ERROR;
    s32 i = 0;
    u32 file_len = 0;
    u8 *pdata = NULL; /*���ݶ������ڴ��ַ*/
    /*�ڴ��е�Դ����*/
    struct nv_global_ddr_info_stru* ddr_info_from = (struct nv_global_ddr_info_stru *)NV_GLOBAL_INFO_ADDR;
    struct nv_ref_data_info_stru ref_info_from = {0};
    struct nv_file_list_info_stru file_info_from = {0};
    u32 nv_offset_from = 0;
    /*��д�������*/
    struct nv_global_ddr_info_stru *ddr_info_to = NULL;
    struct nv_ctrl_file_info_stru *ctrl_info_to = NULL;
    struct nv_ref_data_info_stru ref_info_to = {0};
    struct nv_file_list_info_stru file_info_to = {0};
    u32 nv_offset_to = 0;
    bool wr_flag = false; /* �����Ƿ�ˢ�±�ʶ,������ݾ���ͬ������ˢ�� */
    
    if((NULL == path) || (NULL == nvlist))
    {
        printk(KERN_ERR "[nv writepart]: Input para error.\n");
        return NV_ERROR;
    }
    else if((0 == len) || (0 != nv_file_access(path, 0)))
    {
        /*�������ݳ���Ϊ0���ļ�������ֱ���˳�*/
        return NV_OK;
    }
    crc_flag = crc_flag; /*�ñ�־��ʱ��ʹ��,Ϊ�Ժ���չ*/

    /*��ֻ����ʽ���ļ�*/
    fp = nv_file_open(path, NV_FILE_READ);
    if(NULL == fp)
    {
        printk(KERN_ERR "[nv writepart]: open file %s rd mode error.\n", path);
        return NV_ERROR;
    }
    /*��ȡ�ļ�����*/
    file_len = nv_get_file_len(fp);
    if((NV_MAX_FILE_SIZE < file_len) || (sizeof(struct nv_ctrl_file_info_stru) >= file_len))
    {
        printk(KERN_ERR "[nv writepart]: file %s len %u error.\n", path, file_len);
        goto FAIL;
    }
    /*��ȡ����*/
    pdata = (u8 *)nv_malloc(file_len);
    if(NULL == pdata)
    {
        printk(KERN_ERR "[nv writepart]: malloc data fail, file is %s.\n", path);
        goto FAIL;
    }
    memset((void *)pdata, 0, file_len);

    uret = (u32)nv_file_read(pdata, 1, file_len, fp);
    if(uret != file_len)
    {
        printk(KERN_ERR "[nv writepart]: file %s read fail, ret=0x%x.\n", path, uret);
        goto FAIL;
    }
    /*У��������Ч��*/
    ctrl_info_to = (struct nv_ctrl_file_info_stru *)pdata;
    if(NV_CTRL_FILE_MAGIC_NUM != ctrl_info_to->magicnum)
    {
        printk(KERN_ERR "[nv writepart]: file %s ctrl magic number 0x%x error.\n", path, ctrl_info_to->magicnum);
        goto FAIL;
    }
    /*��ʼ�������ļ���Ϣ*/
    ddr_info_to = (struct nv_global_ddr_info_stru *)nv_malloc(sizeof(struct nv_global_ddr_info_stru));
    if(NULL == ddr_info_to)
    {
        printk(KERN_ERR "[nv writepart]: malloc ddr info fail, file is %s.\n", path);
        goto FAIL;
    }
    memset((void *)ddr_info_to, 0, sizeof(struct nv_global_ddr_info_stru));

    uret = nv_init_file_info((u8 *)ctrl_info_to, (u8 *)ddr_info_to);
    if(NV_OK != uret)
    {
        printk(KERN_ERR "[nv writepart]: file %s init file info fail.\n", path);
        goto FAIL;
    }
    /*�ȹر��ļ��������Կ�д��*/
    (void)nv_file_close(fp);
    fp = NULL;

    /*����nvlistˢ������޸�����,����һ��ʧ�ܲ�Ӱ��������*/
    for(i = 0; i < len; i++)
    {
        /*������սṹ*/
        memset((void *)(&ref_info_from), 0, sizeof(struct nv_ref_data_info_stru));
        memset((void *)(&file_info_from), 0, sizeof(struct nv_file_list_info_stru));
        memset((void *)(&ref_info_to), 0, sizeof(struct nv_ref_data_info_stru));
        memset((void *)(&file_info_to), 0, sizeof(struct nv_file_list_info_stru));
        uret = nv_search_byid((u32)nvlist[i], (u8 *)NV_GLOBAL_CTRL_INFO_ADDR, &ref_info_from, &file_info_from);
        uret |= nv_search_byid((u32)nvlist[i], pdata, &ref_info_to, &file_info_to);
        if((NV_OK != uret) || (ref_info_from.nv_len != ref_info_to.nv_len))
        {
            continue;
        }
        nv_offset_from = ddr_info_from->file_info[file_info_from.file_id - 1].offset + ref_info_from.nv_off;
        nv_offset_to = ddr_info_to->file_info[file_info_to.file_id - 1].offset + ref_info_to.nv_off;
        if((nv_offset_to + ref_info_to.nv_len) > file_len)
        {
            /*Ŀ��ƫ�Ƽ�nv�����Ȳ��ܴ���file_len�Ա���Խ��
            Դ������nvģ�鱣֤���ﲻ���ж�*/
            continue;
        }
        /* �Ƚ��Ƿ���ͬ,����ͬ�򿽱�Դ���ݸ��ǵ�Ŀ�� */
        if (0 != memcmp((void *)(pdata + nv_offset_to),
                (void *)((u8 *)NV_GLOBAL_CTRL_INFO_ADDR + nv_offset_from), ref_info_from.nv_len))
        {
            memcpy((void *)(pdata + nv_offset_to),
                    (void *)((u8 *)NV_GLOBAL_CTRL_INFO_ADDR + nv_offset_from), ref_info_from.nv_len);
            wr_flag = true; /* ��һ��nv�ı�������Ϊtrue */
        }
    }

    /* ������޸ĸ���Ŀ���������,����ֱ�ӷ���ok */
    if (true == wr_flag)
    {
        /* ���´��ļ�д������ */
        fp = nv_file_open(path, NV_FILE_RW);
        if (NULL == fp)
        {
            printk(KERN_ERR "[nv writepart]: open file %s with rw mode error.\n", path);
            goto FAIL;
        }
        uret = (u32)nv_file_write(pdata, 1, file_len, fp);
        if (uret != file_len)
        {
            printk(KERN_ERR "[nv writepart]: file %s write fail, ret=0x%x.\n", path, uret);
            goto FAIL;
        }
        /* д��ɹ���ر��ļ���� */
        (void)nv_file_close(fp);
        fp = NULL;
    }

    nv_free(ddr_info_to);
    ddr_info_to = NULL;
    nv_free(pdata);
    pdata = NULL;

    return NV_OK;

FAIL:
    if(NULL != ddr_info_to)
    {
        nv_free(ddr_info_to);
        ddr_info_to = NULL;
    }
    if(NULL != pdata)
    {
        nv_free(pdata);
        pdata = NULL;
    }
    if(NULL != fp)
    {
        (void)nv_file_close(fp);
        fp = NULL;
    }
    return NV_ERROR;
}

/*д��default������nvlist
����Ĭ��Ϊ��,����ʱ��������
����Ʒ��ͬ,�������ʱ��Ҫʹ����̬����Ʒ�����*/
static u16 g_nv_default_list[] =
{
};

/*****************************************************************************
 �� �� ��  : nv_writepart_to_default
 ��������  : ��default������д�벿��nvֵ
 �������  : void
 �������  : ��
 �� �� ֵ  : NV_OK / NV_ERROR
*****************************************************************************/
u32 nv_writepart_to_default(void)
{
    return nv_writepart_to_flash((s8*)NV_DEFAULT_PATH, g_nv_default_list,
                        (sizeof(g_nv_default_list) / sizeof(u16)), false);
}

EXPORT_SYMBOL(nv_search_byid);
EXPORT_SYMBOL(nv_resume_ddr_from_img);
EXPORT_SYMBOL(nv_make_ddr_crc);
EXPORT_SYMBOL(nv_imei_data_comp);
EXPORT_SYMBOL(nv_check_ddr_crc);
EXPORT_SYMBOL(show_ref_info);
EXPORT_SYMBOL(nv_show_fastboot_err);
EXPORT_SYMBOL(nv_help);
EXPORT_SYMBOL(show_local_list);
EXPORT_SYMBOL(nv_show_ref_info);
EXPORT_SYMBOL(nv_show_crc_status);
EXPORT_SYMBOL(nv_check_item_len);
EXPORT_SYMBOL(nv_load_err_proc);
EXPORT_SYMBOL(nv_upgrade_revert_proc);
EXPORT_SYMBOL(nv_check_update_default_right);
EXPORT_SYMBOL(nv_delete_update_default_right);
EXPORT_SYMBOL(nv_modify_print_sw);
EXPORT_SYMBOL(nv_modify_pm_sw);
EXPORT_SYMBOL(nv_dload_file_check);
EXPORT_SYMBOL(nv_xml_decode);
EXPORT_SYMBOL(nv_dload_file_crc_check);
EXPORT_SYMBOL(nv_set_crc_offset);
EXPORT_SYMBOL(nv_pushNvFlushList);
EXPORT_SYMBOL(nv_check_factory_nv_status);



