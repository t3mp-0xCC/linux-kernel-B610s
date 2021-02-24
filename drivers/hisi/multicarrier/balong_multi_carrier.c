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


/*lint --e{831,537,732,737,731,958,545}*/
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/dirent.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/statfs.h>
#include <linux/mtd/mtd.h>
#include <linux/kthread.h>
#include <linux/export.h>
#include <drv_nv_id.h>
#include <drv_nv_def.h>
#include <bsp_nvim.h>
#include "balong_multi_carrier.h"
#include "drv_comm.h"
#include "bsp_rfile.h"

#if (FEATURE_ON == MBB_MULTI_CARRIER)
#include <linux/mtd/flash_huawei_dload.h>
#include <bsp_icc.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include "mbb_multicarrier.h"
#include "mdrv.h"
/*oem*/
char pc_vendor_oem_info[OEM_SUPPORT_ID][OEM_NAME_LEN_MAX]=
{
    "GEN",
    "DELL",	
    "SONY",
    "HP",	
    "ACER",
    "LENOVO",
    "SAMSUNG",
    "INTEL",
    "TOSHIBA",
    "ASUS",
    "RESERVE",
    "RESERVE1",	
    "RESERVE2",	
    "RESERVE3",	
    "RESERVE4",	
    "RESERVE5",	
    "RESERVE6",	
    "RESERVE7",	
    "RESERVE8",	
    "RESERVE9"	
};
void balong_mt_carrier_restart(void);
static void balong_mt_carrier_switch_carrier_fn(void);
static mtc_op_func mt_carrier_op_func [MTC_OP_VALID+1]=
{
    huawei_mtc_auto_switch_entry,  /*MTC_SIM_SWITCH*/
    balong_mt_carrier_switch_carrier_fn,   /*MTC_MANUAL_SWITCH*/
    NULL
};
NV_DRV_PLMN_CFG mt_carrier_cur_plmninfo_from_nv;
/*�Զ����ֶ��л����*/
mtc_op_type mt_carrier_op = MTC_OP_VALID;
/*�������SIM����plmn*/
char g_sim_plmn[MTC_PLMN_MAX] = {"00000"};
/*���������л�ʱ��˯��*/
struct wake_lock multi_carrier_lock;  
#define MTC_DRIVER_NAME  "MT_CARRIER"
#endif /* MBB_MULTI_CARRIER */

unsigned int mt_carrier_trace_mask =MT_TRACE_WARNING |MT_TRACE_ALWAYS;
static bool is_mt_carrier_switch_finish = TRUE;

static struct semaphore mt_carrier_sem;
static struct task_struct *mt_carrier_tsk=NULL;

static bool balong_mt_carrier_read_xml_file(void);
/*ϵͳ֧�ֵ����о����ļ���*/
static mt_info  mt_carrier_plmn_supported = {{0,0},0,{NULL,NULL,NULL,NULL}};

#if (FEATURE_ON == MBB_MULTI_CARRIER)
#else
static NV_DRV_PLMN_CFG mt_carrier_cur_plmninfo_from_nv;
#endif /* MBB_MULTI_CARRIER */

/*��ǰϵͳ֧�����еľ����default����*/
static mtc_cur_info mt_carrier_current_info ;

/*���ϵͳ��ʱ��Ŀ�ľ������Ϣ*/
static mt_current_select_infor dest_mt_carrier_info ;

int balong_mt_carrier_set_tracemask(unsigned int trace_flag)
{
    mt_carrier_trace_mask =trace_flag;
    return 0;
}
#if 0
/**
* ����:��ָ����·��������ļ���ָ�����ڴ����ҷ��ض������ݴ�С
*
* ����:
* @d_file_path                  ---Ҫ�����ļ���
* @d_file_buf                ---Ҫ�������ڴ��ַ
* @file_size                ---��Ŷ������ݳ���
* ����ֵ:
* @���ݵõ��ɹ��ͷ���TRUE,����ͷ���FALSE
*/
static bool balong_mt_carrier_get_file_data(char* d_file_path,char* d_file_buf,u32 *file_size)
{
    int fd = -1;
    struct rfile_stat_stru fd_stat = {0};
    if ((NULL == d_file_path) || (NULL == d_file_buf) || (NULL == file_size))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier:input is NULL");
        return FALSE;
    }
     /*���һ���ļ��Ƿ����*/
    if( 0 != bsp_stat(d_file_path, &fd_stat))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier stat: is not exit %s\n",BALONG_MT_CARRIER_PLMN_CONFIG_FILE);
        return FALSE;
    }
    fd = bsp_open(d_file_path, O_RDONLY , S_IRUSR | S_IWUSR);
    if (-1 == fd)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier: open() fail");
        return FALSE;
    }
    /*���ļ��ж�����*/
    if (-1 == bsp_read(fd, d_file_buf, fd_stat.size))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier: read()  fail");
        (void)bsp_close(fd);
        return FALSE;
    }
    if (-1 == bsp_close(fd))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier: close()  fail");
    }
    mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier: get file complete\n");
    *file_size = fd_stat.size;
    return TRUE;
}
#endif
/**
* ����:�����ڴ��е����ݵ��ļ���ȥ
*
* ����:
* @d_file_path               ---Ҫ������ļ���
* @d_file_buf                ---Ҫ�����������ڴ��еĵ�ַ
* @file_size                ---Ҫ��������ݴ�С
* ����ֵ:
* ����ɹ��ͷ���TRUE,����ͷ���FALSE
*/
static bool balong_mt_carrier_save_file_data( char* d_file_path, char* d_file_buf,unsigned int d_file_size)
{
    int fd = -1;
    struct rfile_stat_stru  efs_buf;
    if ((NULL == d_file_path) || (NULL == d_file_buf))
    {
        return FALSE;
    }
    /*���һ���ļ��Ƿ����*/
    if( 0 != bsp_stat(d_file_path, &efs_buf))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"warning balong_mt_carrier_save_file_data: is not exit %s\n",d_file_path);
    }

    /*���ļ�,��trunk�ķ�ʽ���ļ�*/
    fd = bsp_open(d_file_path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (-1 == fd)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_save_file_data:open error\n");
        return FALSE;
    }
    /*д����*/
    if (d_file_size != bsp_write(fd, d_file_buf, d_file_size))
    {
#if (FEATURE_ON == MBB_MULTI_CARRIER)
        (void)bsp_close(fd);
#endif
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_save_file_data:write error\n");
        return FALSE;
    }
    if (-1 == bsp_close(fd))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_save_file_data:write close\n");
    }
    mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_save_file_data complete\n");
    return TRUE;
}

/**
* ����:�������к�Ĭ�ϵ�PLMN��Ϣ��NV�ļ���ȥ
*
* ����:
* @cur_plmninfo               ---Ҫ��������ݵ�ַ
* @plmn_nv_size               ---Ҫ��������ݴ�С
* ����ֵ:
* ���浱ǰ��PLMN��NV�ɹ��ͷ���TRUE,����ͷ���FALSE
*/
bool balong_mt_carrier_save_current_plmn_nv(char* cur_plmninfo,unsigned int plmn_nv_size)
{
#if (FEATURE_ON == MBB_MULTI_CARRIER)
#else
    u32 result ;
#endif /* MBB_MULTI_CARRIER */
    if ((NULL == cur_plmninfo) || (0 == plmn_nv_size))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"save multi_carrier nv: input param is wrong \n");
        return FALSE;
    }
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    if (!huawei_mtc_set_plmn_from_oem(cur_plmninfo, plmn_nv_size))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_init:set plmn to oeminfo fail ");
        return FALSE;
    }
#else
    /*����д���ļ���ȥ*/
    result=bsp_nvm_write(NV_ID_DRV_MTC_PLMN_CFG,(u8 *)cur_plmninfo,plmn_nv_size);

    if (result)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"save multi_carrier nv: result is wrong %d\n",result);
        return FALSE;
    }
#endif /* MBB_MULTI_CARRIER */
    mt_carrier_trace(MT_TRACE_ALWAYS,"save multi_carrier nv complete\n");
    return TRUE;
}

/**
* ����:���ضྵ��ļ��ط���
*
* ����:
* ��
* ����ֵ:
* �ྵ��������سɹ��ͷ���Ϊ0�����ɹ��ͷ���һ����ֵ
*/
int balong_mt_carrier_partition_mount(void)
{
    struct mtd_info *mtd;
    int rt = 0;
    char mount_name[32] ={0};

#if (FEATURE_ON == MBB_MULTI_CARRIER)
    /*online��������Ҫ���������ļ�ϵͳ��ֱ�ӷ���0*/
    return 0;
#endif /* MBB_MULTI_CARRIER */

    mtd = get_mtd_device_nm(BALONG_MT_CARRIER_MTD_PARTITION_NAME);
    if (IS_ERR(mtd))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"get_mtd_device_nm error.\n");
        return PTR_ERR(mtd);
    }

    snprintf(mount_name, sizeof(mount_name) - 1, "/dev/block/mtdblock%d", mtd->index);
    mt_carrier_trace(MT_TRACE_ALWAYS,"going to mount %s  mount point %s\n", mount_name, BALONG_MT_CARRIER_MOUNT_POINT);

    if((rt = sys_mkdir(BALONG_MT_CARRIER_MOUNT_POINT, S_IRUSR | S_IRGRP)) < 0)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"create dir failed %s ret 0x%x\n", BALONG_MT_CARRIER_MOUNT_POINT, rt);
        return rt ;
    }

    rt = sys_mount(mount_name, BALONG_MT_CARRIER_MOUNT_POINT, "yaffs2", 0, NULL);
    if(rt < 0)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"mount failed %s  %s ret 0x%x 0x%x\n", mount_name, BALONG_MT_CARRIER_MOUNT_POINT, rt, MKDEV(31,mtd->index));
        return rt ;
    }
    mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_partition_mount complete\n");
    return 0;

}
/**
* ����:��ǰ��PLMN��Ĭ�ϵ�PLMN��ͨ��NV�ж������ģ���ֻ��������Ϣ��ͨ���Ƚ�ϵͳ������֧�ֵ�PLMN��Ϣ,���õ���ǰ��Ĭ�ϵ�PLMN��ȫ����Ϣ,��ͨ���������ص�ǰplmn��Ĭ�ϵ�PLMN�Ƿ����
*
* ����:
* ��
* ����ֵ:
* ��
* ����:��������֧�ֵ�����PLMN,���ҵ�ǰ��PLMN��Ĭ�ϵ�PLMN���õ�һ���Ƚ�ȫ���PLMN��Ϣ�����mt_carrier_current_info�����ݽṹ��ȥ
*/
static void balong_mt_carrier_resolve_current_info(bool *cur_plmn_found,bool *default_plmn_found)
{
    u32 i = 0;
    u32 j= 0;
    u32 k = 0;
    if ((NULL == cur_plmn_found) || (NULL == default_plmn_found))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_resolve_current_info:input param  is wrong\n");
        return ;
    }
    *cur_plmn_found = FALSE;
    *default_plmn_found = FALSE;
    /*�������е�֧�ֵ�image*/
    for (i=0; i<mt_carrier_plmn_supported.mt_image_cnt; i++)
    {
        for(j=0; j<mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_cnt; j++)
        {
            for(k=0; k<mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_cnt; k++)
            {
                /*�������������PLMN����ͬ��*/
                if (0 == strcmp((char *)mt_carrier_cur_plmninfo_from_nv.cur_running_plmn,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_array[k]->plmn))
                {
                    /*���浱ǰ��Ŀ�������Ϣ*/
                    memset((char *)&mt_carrier_current_info.carrier_info,'\0',sizeof(mt_carrier_current_info.carrier_info));
                    mt_carrier_current_info.carrier_info.mt_image_index= mt_carrier_plmn_supported.mt_image_info_array[i]->mt_image_index;
                    mt_carrier_current_info.carrier_info.mt_carrier_id= mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_id;
                    mt_carrier_current_info.carrier_info.mt_carrier_index=mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_index;
                    mt_carrier_current_info.carrier_info.mt_carrier_xml_version= mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_xml_version;
                    memcpy((char *)mt_carrier_current_info.carrier_info.mt_image_version,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_image_version,MTC_IMAGE_VER_MAX);
                    memcpy((char *)&mt_carrier_current_info.carrier_info.mt_carrier_plmn_info,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_array[k],sizeof(mt_carrier_plmn_info));
                    *cur_plmn_found = TRUE;
                    mt_carrier_trace(MT_TRACE_ALWAYS,"cur carrier_id is %d plmn is %s",mt_carrier_current_info.carrier_info.mt_carrier_id,mt_carrier_cur_plmninfo_from_nv.cur_running_plmn);
                }
                if (0 == strcmp((char *)mt_carrier_cur_plmninfo_from_nv.default_plmn,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_array[k]->plmn))
                {
                    /*���浱ǰ��Ŀ�������Ϣ*/
                    memset((char *)&mt_carrier_current_info.defalt_info,'\0',sizeof(mt_carrier_current_info.defalt_info));
                    mt_carrier_current_info.defalt_info.mt_image_index= mt_carrier_plmn_supported.mt_image_info_array[i]->mt_image_index;
                    mt_carrier_current_info.defalt_info.mt_carrier_id= mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_id;
                    mt_carrier_current_info.defalt_info.mt_carrier_index=mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_index;
                    mt_carrier_current_info.defalt_info.mt_carrier_xml_version= mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_xml_version;
                    memcpy((char *)mt_carrier_current_info.defalt_info.mt_image_version,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_image_version,MTC_IMAGE_VER_MAX);
                    memcpy((char *)&mt_carrier_current_info.defalt_info.mt_carrier_plmn_info,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_array[k],sizeof(mt_carrier_plmn_info));
                    *default_plmn_found = TRUE;
                    mt_carrier_trace(MT_TRACE_ALWAYS,"default carrier_id is %d plmn is %s",mt_carrier_current_info.defalt_info.mt_carrier_id,mt_carrier_cur_plmninfo_from_nv.default_plmn);
                }
#if (FEATURE_ON == MBB_MULTI_CARRIER)
                if (0 == strcmp(MTC_GEN_PLMN, (char*)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_array[k]->plmn))
                {
                    /*����ͨ����Ӫ�̵������Ϣ*/
                    memset((char*)&mt_carrier_current_info.general_info, '\0', sizeof(mt_carrier_current_info.general_info));
                    mt_carrier_current_info.general_info.mt_image_index = mt_carrier_plmn_supported.mt_image_info_array[i]->mt_image_index;
                    mt_carrier_current_info.general_info.mt_carrier_id = mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_id;
                    mt_carrier_current_info.general_info.mt_carrier_index = mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_index;
                    mt_carrier_current_info.general_info.mt_carrier_xml_version = mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_xml_version;
                    memcpy((char*)mt_carrier_current_info.general_info.mt_image_version, (char*)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_image_version, MTC_IMAGE_VER_MAX);
                    memcpy((char*)&mt_carrier_current_info.general_info.mt_carrier_plmn_info, (char*)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_array[k], sizeof(mt_carrier_plmn_info));
                    *default_plmn_found = TRUE;
                    mt_carrier_trace(MT_TRACE_ALWAYS, "general carrier_id is %d plmn is %s", mt_carrier_current_info.general_info.mt_carrier_id, MTC_GEN_PLMN);
                }
#endif /* MBB_MULTI_CARRIER */
            }

        }

    }
    if(cur_plmn_found == FALSE)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"error resolve_current_info can not find cur_plmn %s\n",mt_carrier_cur_plmninfo_from_nv.cur_running_plmn);
    }
    if(default_plmn_found== FALSE)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"warning resolve_current_info can not find default_plmn %s\n",mt_carrier_cur_plmninfo_from_nv.default_plmn);
    }
    mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_resolve_current_info complete\n");

}

/**
* ����:plmn���ǰ��׼������,����ǵ�һ��ִ�д˺���ʱҪ����multicarrier���������������ļ�����ص����ݽṹ��ȥ
*
* ����:
* ��
* ����:���ȰѴ˶ྵ����ط������أ�Ȼ��������xml�������ļ����õ�һ���Ƚ�ȫ���PLMN��Ϣ�����mt_carrier_current_info�����ݽṹ��ȥ
*/
static bool balong_mt_carrier_check_plmn_ready(void)
{
    static int check_ready=0;
    bool ret = FALSE;
    if(check_ready)
    {
        return TRUE;
    }
    else
    {
        /*���ȹ���multicarrier������*/
        ret = balong_mt_carrier_partition_mount();
        if(ret != 0)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS," balong_mt_carrier_partition_mount error\n");
            return FALSE;
        }
        /*����ϵͳ֧�ֵĶྵ�������ļ�����ص����ݽṹ��ȥ*/
        ret = balong_mt_carrier_read_xml_file();
        if(ret != TRUE)
        {
#if (FEATURE_ON == MBB_MULTI_CARRIER)        
            huawei_mtc_free_mem();       
#endif
            mt_carrier_trace(MT_TRACE_ALWAYS," balong_mt_carrier_read_xml_file error\n");
            return FALSE;
        }
        check_ready=1;
        mt_carrier_trace(MT_TRACE_ALWAYS," balong_mt_carrier_check_plmn_ready complete \n");
        return ret;
    }

}

/**
* ����:��ѯ��PLMNϵͳ�Ƿ�֧�֣����֧�ָ��µ�dest_mt_carrier_info�����ݽṹ��ȥ
*
* ����:
* @plmn            ---��ѯ��PLMN
* ����ֵ:
* ���֧�ִ�PLMN�෵��TRUE,��֧���򷵻�FALSE
* ����:��������֧�ֵ�����PLMN,���ҵ�ǰ��PLMNϵͳ�Ƿ�֧��
*/
static bool balong_mt_carrier_check_plmn_support(char *plmn)
{
    u32 i = 0;
    u32 j= 0;
    u32 k = 0;
    bool is_plmn_found = FALSE;
    /*���plmn��Ϊ�յĻ�*/
    if (NULL == plmn)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier:plmn is NULL\n");
        return FALSE;
    }
    if(!balong_mt_carrier_check_plmn_ready())
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_check_plmn_ready error\n");
        return FALSE;
    }
    /*�������е�֧�ֵ�image*/
    for (i=0; i<mt_carrier_plmn_supported.mt_image_cnt; i++)
    {
        for(j=0; j<mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_cnt; j++)
        {
            for(k=0; k<mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_cnt; k++)
            {
                if (0 == strcmp((char *)plmn,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_array[k]->plmn))
                {
                    /*���浱ǰ��Ŀ�������Ϣ*/
                    memset((char *)&dest_mt_carrier_info,'\0',sizeof(dest_mt_carrier_info));
                    dest_mt_carrier_info.mt_image_index= mt_carrier_plmn_supported.mt_image_info_array[i]->mt_image_index;
                    dest_mt_carrier_info.mt_carrier_id= mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_id;
                    dest_mt_carrier_info.mt_carrier_index=mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_index;
                    dest_mt_carrier_info.mt_carrier_xml_version= mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_xml_version;
                    memcpy((char *)dest_mt_carrier_info.mt_image_version,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_image_version,MTC_IMAGE_VER_MAX);
                    memcpy((char *)&dest_mt_carrier_info.mt_carrier_plmn_info,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_array[k],sizeof(mt_carrier_plmn_info));
                    is_plmn_found = TRUE;
                    mt_carrier_trace(MT_TRACE_ALWAYS,"dest carrier_id is %d",dest_mt_carrier_info.mt_carrier_id);
                    return is_plmn_found;
                }
            }

        }

    }
    mt_carrier_trace(MT_TRACE_ALWAYS,"not find plmn %s\n",plmn);
    return is_plmn_found;
}

/**
* ����:�ж�Ҫ�л���Ŀ�ĵľ����xml�Ƿ����
*
* ����:
* @dest_mt_carrier_info            ---Ҫ�л��������xml������Ϣ
* ����ֵ:
* ���֧���򷵻�TRUE,�����֧���򷵻�FALSE
* ����:�ж�һ��Ҫ�л���Ŀ�ľ����xml�ļ��Ƿ����
*/
static bool balong_mt_carrier_check_image_xml_is_exit(mt_current_select_infor dest_mt_carrier_info)
{
    struct rfile_stat_stru  efs_buf;
    char  temp_mtc_image_path[MTC_FILE_PATH_MAX] = {0};
    char  temp_user_xml_save_path[MTC_FILE_PATH_MAX] = {0};

#if (FEATURE_ON == MBB_MULTI_CARRIER)
    (void)snprintf(temp_mtc_image_path,sizeof(temp_mtc_image_path),"/online/mtc/image%d/VxWorks", dest_mt_carrier_info.mt_image_index);
#else
    (void)snprintf(temp_mtc_image_path,sizeof(temp_mtc_image_path),"/mtc/image%d/VxWorks", dest_mt_carrier_info.mt_image_index);
#endif /* MBB_MULTI_CARRIER */
    /*���һ���ļ��Ƿ����*/
    if( 0 != bsp_stat(temp_mtc_image_path, &efs_buf))
    {
#if (FEATURE_ON == MBB_MULTI_CARRIER)
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_check_image_xml_is_exit %s is not exit\n",temp_mtc_image_path);
#else
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_check_image_xml_is_exit VxWorks is not exit\n");
#endif /* MBB_MULTI_CARRIER */
        return FALSE;
    }

#if (FEATURE_ON == MBB_MULTI_CARRIER)
    (void)snprintf(temp_mtc_image_path,sizeof(temp_mtc_image_path),"/online/mtc/image%d/DSP", dest_mt_carrier_info.mt_image_index);
    /*���һ���ļ��Ƿ����*/
    if( 0 != bsp_stat(temp_mtc_image_path, &efs_buf))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_check_image_xml_is_exit DSP is not exit\n");
        return FALSE;
    }
    (void)snprintf(temp_user_xml_save_path,sizeof(temp_user_xml_save_path),"/online/mtc/image%d/NV/Carrier/Carrier%d/cust.xml",dest_mt_carrier_info.mt_image_index, dest_mt_carrier_info.mt_carrier_index);
#else 
    (void)snprintf(temp_user_xml_save_path,sizeof(temp_user_xml_save_path),"/mtc/image%d/NV/Carrier/Carrier%d",dest_mt_carrier_info.mt_image_index, dest_mt_carrier_info.mt_carrier_index);
#endif /* MBB_MULTI_CARRIER */

    /*���һ���ļ��Ƿ����*/
    if( 0 != bsp_stat(temp_user_xml_save_path, &efs_buf))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_check_image_xml_is_exit Carrier is not exit\n");
        return FALSE;
    }
    mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_check_image_xml_is_exit complete\n");
    return TRUE;
}

/**
* ����:�ྵ���л���Ԥ�����������֧���л��Ļ��ͻ����л�����ִ�жྵ���л�
*
* ����:
* @plmn            ---Ҫ�л���plmn
* ����ֵ:
* ���֧���򷵻�TRUE,�����֧���򷵻�FALSE
*/
static bool balong_mt_carrier_switch_pre_op(char *plmn)
{
    bool cur_find,default_find;
    if (NULL == plmn)
    {
        return FALSE;
    }
    /*���ܷ����л�*/
    if (!balong_mt_carrier_check_plmn_support((char *)plmn))
    {
        return FALSE;
    }
    balong_mt_carrier_resolve_current_info(&cur_find,&default_find);
    if(cur_find != TRUE)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS," balong_mt_carrier_switch_pre_op error cur\n");
        return FALSE;
    }
    /*�ж�һ��Ҫ�л���image��xml�Ƿ���ڣ���������ھ�ֱ��ͨ��*/
    if(!balong_mt_carrier_check_image_xml_is_exit(dest_mt_carrier_info))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier:write cur_ver_file fail\n");
        return FALSE;
    }
    /*Ҫ�л��������ڵ�����ͬ�Ļ��Ͳ��л���*/
    if (dest_mt_carrier_info.mt_carrier_id == mt_carrier_current_info.carrier_info.mt_carrier_id)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier: the change plmn carrer_id is same\n");

        if (0 != strcmp((char *)dest_mt_carrier_info.mt_carrier_plmn_info.plmn,(char *)mt_carrier_current_info.carrier_info.mt_carrier_plmn_info.plmn))
        {
            memcpy((char *)mt_carrier_current_info.carrier_info.mt_carrier_plmn_info.plmn,(char *)dest_mt_carrier_info.mt_carrier_plmn_info.plmn,MTC_PLMN_MAX);
            memcpy((char *)&mt_carrier_cur_plmninfo_from_nv.cur_running_plmn,(char *)&dest_mt_carrier_info.mt_carrier_plmn_info.plmn,MTC_PLMN_MAX);
            balong_mt_carrier_save_current_plmn_nv((char *)&mt_carrier_cur_plmninfo_from_nv,sizeof (mt_carrier_cur_plmninfo_from_nv));
            return TRUE;
        }
        return TRUE;
    }
    is_mt_carrier_switch_finish = FALSE;
    /*�����л�������*/
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    mt_carrier_op = MTC_MANUAL_SWITCH;
#endif
    up(&mt_carrier_sem);
    mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_switch_pre_op complete\n");
    return TRUE;
}

/**
* ����:��mtc_info��ص���Ϣд��dest_buff_ptr��ȥ,��PLMN����ϸ��Ϣ���ظ�AT�Ĳ�ѯ����
*
* ����:
* @mtc_info            ---Ҫд��plmn���ݽṹ
* @dest_buff_ptr       ---Ҫд�����ݵ�ַ
* @length              ---����д�����ݳ���
* ����ֵ:
* ����TRUE
*/
static bool balong_mt_carrier_print(mt_current_select_infor *mtc_info,char *dest_buff_ptr,u32 *length)
{

    char buf_temp[MTC_BUF_MAX_LENGTH] = {0};
    u32 buffer_size = 0;
    if ((NULL == mtc_info)||(NULL  == dest_buff_ptr )||(NULL  == length ))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_print:input is NULL");
        return FALSE;
    }
    *(buf_temp +buffer_size)='"';
    buffer_size++;
    /*��ӡplmn����Ϣ*/
    buffer_size += snprintf( (char*)( buf_temp + buffer_size ), MTC_BUF_MAX_LENGTH - buffer_size,
                                  "%s",mtc_info->mt_carrier_plmn_info.plmn);
    *(buf_temp +buffer_size)='"';
    buffer_size++;
    *(buf_temp +buffer_size)=',';
    buffer_size++;
    /*��ӡplmn�Ķ�������Ϣ*/
    *(buf_temp +buffer_size)='"';
    buffer_size++;

    if (MTC_GEN_IPV4V6_ID == mtc_info->mt_carrier_id)
    {
        buffer_size += snprintf( (char*)( buf_temp + buffer_size ), MTC_BUF_MAX_LENGTH - buffer_size,
                                                                    "%s", "GEN(IPv4v6)");
    }
    else if (MTC_VDF_IPV4V6_ID== mtc_info->mt_carrier_id)
    {
        buffer_size += snprintf( (char*)( buf_temp + buffer_size ), MTC_BUF_MAX_LENGTH - buffer_size,
                                                                    "%s", "voda(IPv4v6)");
    }
    else
    {
        buffer_size += snprintf( (char*)( buf_temp + buffer_size ), MTC_BUF_MAX_LENGTH - buffer_size,
                                                        "%s", mtc_info->mt_carrier_plmn_info.s_name);
    }
    *(buf_temp +buffer_size)='"';
    /*��short_nameд�����*/
    buffer_size++;
    *(buf_temp +buffer_size)=',';
    buffer_size++;
    /*printf l_name*/

    *(buf_temp +buffer_size)='"';
    buffer_size++;

    if (MTC_GEN_IPV4V6_ID ==mtc_info->mt_carrier_id)
    {
        buffer_size += snprintf( (char*)( buf_temp + buffer_size ), MTC_BUF_MAX_LENGTH - buffer_size,
                                                "%s", "Generic(IPv4v6)");
    }
    else if (MTC_VDF_IPV4V6_ID == mtc_info->mt_carrier_id)
    {
        buffer_size += snprintf( (char*)( buf_temp + buffer_size ), MTC_BUF_MAX_LENGTH - buffer_size,
                                                "%s", "vodafone(IPv4v6)");
    }
    else
    {
        buffer_size += snprintf( (char*)( buf_temp + buffer_size ), MTC_BUF_MAX_LENGTH - buffer_size,
                                                "%s", mtc_info->mt_carrier_plmn_info.l_name);
    }

    *(buf_temp +buffer_size)='"';
    buffer_size++;
    *(buf_temp +buffer_size)=',';
    buffer_size++;
    /*printf fw_ver*/
    *(buf_temp +buffer_size)='"';
    buffer_size++;
    buffer_size += snprintf( (char*)( buf_temp + buffer_size ), MTC_BUF_MAX_LENGTH - buffer_size,
                                  "%s", mtc_info->mt_image_version);
    *(buf_temp +buffer_size)='"';
    buffer_size++;
    *(buf_temp +buffer_size)=',';
    buffer_size++;
    /*print xml_ver*/
    *(buf_temp +buffer_size)='"';
    buffer_size++;
    buffer_size += snprintf( (char*)( buf_temp + buffer_size ), MTC_BUF_MAX_LENGTH - buffer_size,
                                  "%02x", mtc_info->mt_carrier_xml_version);
    *(buf_temp +buffer_size)='"';
    buffer_size++;
    *(buf_temp +buffer_size)='\r';
    buffer_size++;
    *(buf_temp +buffer_size)='\n';
    buffer_size++;
    strncpy(dest_buff_ptr,buf_temp,buffer_size);
    *length= buffer_size;
    return TRUE;

}
/**
* ����:��ϵͳ����֧�ֵ�PLMN��Ϣ�����ͳ�ȥ
*
* ����:
* @dest_buff_ptr            ---���PLMN��Ϣ�ĵ�ַ
* @length                   ---�����ܹ�д���ַ���Ŀ
* ����ֵ:
* �ɹ��򷵻�TRUE,ʧ�ܷ���FALSE
*/
static bool balong_mt_carrier_support_print(char *dest_buff_ptr,u32 *length)
{
    u32 i = 0;
    u32 j = 0;
    u32 k = 0;
    u32 temp = 0;
    mt_current_select_infor  carrier_info_temp;
    if ((NULL == dest_buff_ptr)||(NULL  == length))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_support_print:input is NULL");
        return FALSE;
    }
    *length=0;
    memset((char *)&carrier_info_temp,'\0',sizeof(carrier_info_temp));
    /*����ϵͳ����֧�ֵ�PLMN*/
    for (i=0; i<mt_carrier_plmn_supported.mt_image_cnt; i++)
    {
        carrier_info_temp.mt_image_index = mt_carrier_plmn_supported.mt_image_info_array[i]->mt_image_index;
        memcpy((char *)carrier_info_temp.mt_image_version,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_image_version,MTC_IMAGE_VER_MAX);
        for(j=0; j<mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_cnt; j++)
        {
            carrier_info_temp.mt_carrier_id = mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_id;
            carrier_info_temp.mt_carrier_index =mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_index;
            carrier_info_temp.mt_carrier_xml_version =mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_xml_version;

            for(k=0; k<mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_cnt; k++)
            {
#if (FEATURE_ON == MBB_MULTI_CARRIER)
                u32 cmd_name_size = 0;
                memcpy((char *)&carrier_info_temp.mt_carrier_plmn_info,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_array[k],sizeof(mt_carrier_plmn_info));
                cmd_name_size += snprintf( (char*)dest_buff_ptr, MTC_BUF_MAX_LENGTH - *length,
                                                                    "%s", "^MTCARRIER: ");
                dest_buff_ptr +=  cmd_name_size;
                if(!balong_mt_carrier_print(&carrier_info_temp,dest_buff_ptr,&temp))
                {
                    mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_print error\n");
                    return FALSE;
                }
                dest_buff_ptr+=temp;
                *length=*length+temp+cmd_name_size;
#else
                memcpy((char *)&carrier_info_temp.mt_carrier_plmn_info,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_array[k],sizeof(mt_carrier_plmn_info));
                if(!balong_mt_carrier_print(&carrier_info_temp,dest_buff_ptr,&temp))
                {
                    mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_print error\n");
                    return FALSE;
                }
                dest_buff_ptr+=temp;
                *length=*length+temp;
#endif
            }
        }
    }
    return TRUE;
}
/**
* ����:��ѯϵͳ��PLMN��Ϣ
*
* ����:
* @dest_buff_ptr            ---���PLMN��Ϣ�ĵ�ַ
* @length                   ---�����ܹ�д���ַ���Ŀ
* @MTC_QUERY_TYPE           ---��ѯ������
* ����ֵ:
* �ɹ��򷵻�TRUE,ʧ�ܷ���FALSE
*/
bool balong_basic_exec_mtcarrier_query(char *dest_buff_ptr,u32 *length,MTC_QUERY_TYPE query)
{
    bool cur_find = FALSE;
    bool default_find = FALSE;
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    bool res = FALSE;	
#endif
    if ((NULL== dest_buff_ptr )||(NULL == length))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_basic_exec_mtcarrier_query_cur_running:input is NULL\n");
        return FALSE;
    }
    if(!balong_mt_carrier_check_plmn_ready())
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_check_plmn_ready error\n");
        return FALSE;
    }
    /*�����ٴ�������һ�ε�ԭ���ǿ���ǰһ���޸ĵ�Ĭ�ϵ�PLMN��Ϣ����Ҫ����mt_carrier_current_info�����е����ݽṹ*/
    balong_mt_carrier_resolve_current_info(&cur_find,&default_find);
    if(cur_find != TRUE)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS," balong_mt_carrier_resolve_current_info error cur\n");
        return FALSE;
    }
    if(query == MTC_QUERY_RUNNING)
    {
#if (FEATURE_ON == MBB_MULTI_CARRIER)
        res = balong_mt_carrier_print(&mt_carrier_current_info.carrier_info,dest_buff_ptr,length);
        *(dest_buff_ptr + *length-2) = 0;
        *length = *length - 2;
        return res ;
#else
        return balong_mt_carrier_print(&mt_carrier_current_info.carrier_info,dest_buff_ptr,length);
#endif
    }
    else  if(query == MTC_QUERY_DEFAULT)
    {
        if(default_find != TRUE)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS," balong_mt_carrier_resolve_current_info error default\n");
            return FALSE;
        }
#if (FEATURE_ON == MBB_MULTI_CARRIER)
        res = balong_mt_carrier_print(&mt_carrier_current_info.defalt_info,dest_buff_ptr,length);
        *(dest_buff_ptr+*length-2) = 0;
        *length = *length - 2;
        return res;
#else
        return balong_mt_carrier_print(&mt_carrier_current_info.defalt_info,dest_buff_ptr,length);
#endif
    }
    else
    {
#if (FEATURE_ON == MBB_MULTI_CARRIER)
        res = balong_mt_carrier_support_print(dest_buff_ptr,length);
        *(dest_buff_ptr+*length-2) = 0;
        *length -=2;
        return res;
#else
        return balong_mt_carrier_support_print(dest_buff_ptr,length);
#endif
    }
}
EXPORT_SYMBOL(balong_basic_exec_mtcarrier_query);

/**
* ����:�ྵ���л�������
* ����:
* @plmn                     ---Ҫ�л���PLMN
* ����ֵ:
* �ɹ��򷵻�TRUE,ʧ�ܷ���FALSE
*/
bool balong_basic_exec_mtcarrier_cmd(char *plmn)
{
    /*����������*/
    if(plmn==NULL)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"plmn is NULL\n");
        return FALSE;
    }
    /*�����һ���л���û�����*/
    if (FALSE == is_mt_carrier_switch_finish)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_basic_exec_mtcarrier_cmd already running\n");
        return FALSE;
    }
    if (!balong_mt_carrier_switch_pre_op((char *)plmn))
    {
        return FALSE;
    }
    return TRUE;
}
EXPORT_SYMBOL(balong_basic_exec_mtcarrier_cmd);

/**
* ����:���浱ǰ��Ĭ��PLMN��Ϣ��NV�ļ���ȥ
* ����:
* @plmn                     ---Ҫ�����Ĭ��PLMN
* ����ֵ:
* �ɹ��򷵻�TRUE,ʧ�ܷ���FALSE
*/
static bool balong_mt_carrier_save_mtcdft(char *plmn)
{
    bool ret=FALSE;
    if (NULL == plmn)
    {
        return FALSE;
    }
    /*�鿴��plmn�Ƿ�֧�֣������֧�ֵĻ����ܱ���*/
    if (!balong_mt_carrier_check_plmn_support((char *)plmn))
    {
        return FALSE;
    }
    /*�ж�һ��Ҫ�л���image��xml�Ƿ���ڣ���������ھ�ֱ��ͨ��*/
    if(!balong_mt_carrier_check_image_xml_is_exit(dest_mt_carrier_info))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_save_mtcdft:image_xml is not exit\n");
        return FALSE;
    }
    if (0 == strcmp((char *)plmn,(char *)mt_carrier_current_info.defalt_info.mt_carrier_plmn_info.plmn))
    {
        return TRUE;
    }
    /*��Ĭ�ϵ�plmn�������ļ���*/
    memcpy((char *)mt_carrier_cur_plmninfo_from_nv.default_plmn,(char *)plmn,MTC_PLMN_MAX);
    ret= balong_mt_carrier_save_current_plmn_nv((char *)&mt_carrier_cur_plmninfo_from_nv,sizeof (mt_carrier_cur_plmninfo_from_nv));
    return ret;

}
/**
* ����:Ĭ�ϵ�PLMN����ʵ��
* ����:
* @plmn                     ---Ҫ������Ĭ��PLMN
* @dft_option               ---������Ĭ��PLMN�Ĳ�������
* ����ֵ:
* �ɹ��򷵻�TRUE,ʧ�ܷ���FALSE
*/
bool balong_basic_exec_mtcdft_cmd(char *plmn,MTC_DFT_OPTION dft_option)
{
    bool ret=FALSE;
    /*���ֻ������default�Ļ�*/
    if (MTC_DEFAULT_SET == dft_option)
    {
        ret=balong_mt_carrier_save_mtcdft(plmn);
    }
    /*����default�Ļ�*/
    else if(MTC_DEFAULT_RUN == dft_option)
    {
        /*��������Ĭ��PLMNΪ�յĻ�*/
        if(NULL == plmn)
        {
            ret= balong_mt_carrier_switch_pre_op((char *)mt_carrier_current_info.defalt_info.mt_carrier_plmn_info.plmn);
        }
        else
        {
            /*�����ֻ�Ǳ���һ��Ĭ�ϵ�PLMN��Ϣ*/
            ret = balong_mt_carrier_save_mtcdft(plmn);
            if(FALSE == ret)
            {
                mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_save_mtcdft fail");
                return FALSE;
            }
            ret = balong_mt_carrier_switch_pre_op(plmn);
        }
    }
    return ret;
}
EXPORT_SYMBOL(balong_basic_exec_mtcdft_cmd);

bool balong_basic_exec_mtcautosel_cmd(u32 autosel)
{
    return TRUE;
}

bool balong_basic_exec_mtcautosel_query(u32 * autoselflag)
{
    return TRUE;
}
/**
* ����:�����mtc_updata_status�ļ��е�����
* ����:
* @dest_mt_carrier_info                     ---Ҫ�����Ŀ�ľ�����Ϣ
* ����ֵ:
* �ɹ��򷵻�TRUE,ʧ�ܷ���FALSE
*/
static bool balong_mt_carrier_save_updata_status_file(mt_current_select_infor dest_mt_carrier_info)
{
    bool ret=FALSE;
    mt_carrier_updata_stutus carrier_updata_stutus;
    memset((char *)&carrier_updata_stutus,'\0',sizeof(carrier_updata_stutus));

    memcpy((char *)carrier_updata_stutus.mt_cur_running_plmn,(char *)mt_carrier_current_info.carrier_info.mt_carrier_plmn_info.plmn,MTC_PLMN_MAX);
    memcpy((char *)carrier_updata_stutus.mt_dest_to_plmn,(char *)dest_mt_carrier_info.mt_carrier_plmn_info.plmn,MTC_PLMN_MAX);
    /*��������ľ�����ͬ�Ļ���Ҫ��������ı��*/
    if(mt_carrier_current_info.carrier_info.mt_image_index != dest_mt_carrier_info.mt_image_index)
    {
#if (FEATURE_ON == MBB_MULTI_CARRIER)
#else
        snprintf(carrier_updata_stutus.mt_dest_image_path,MTC_DEST_IMAGE_PATH,"/mtc/image%d",dest_mt_carrier_info.mt_image_index);
#endif /* MBB_MULTI_CARRIER */
        carrier_updata_stutus.mt_image_updata_status =1;
    }
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    /* ����û��趨��sim����������Ӫ�̾����XML��Ϣ */
    snprintf(carrier_updata_stutus.mt_dest_image_path, MTC_DEST_IMAGE_PATH, "/online/mtc/image%d", dest_mt_carrier_info.mt_image_index);
    snprintf(carrier_updata_stutus.mt_dest_carrier_xml_path, MTC_DEST_IMAGE_PATH, "/online/mtc/image%d/NV/Carrier/Carrier%d", dest_mt_carrier_info.mt_image_index, dest_mt_carrier_info.mt_carrier_index);
    /* ���Ĭ����Ӫ�̾����XML��Ϣ */
    memcpy((char*)carrier_updata_stutus.mt_default_plmn, (char*)mt_carrier_current_info.defalt_info.mt_carrier_plmn_info.plmn, MTC_PLMN_MAX);
    snprintf(carrier_updata_stutus.mt_default_image_path, MTC_DEST_IMAGE_PATH, "/online/mtc/image%d", mt_carrier_current_info.defalt_info.mt_image_index);
    snprintf(carrier_updata_stutus.mt_default_carrier_xml_path, MTC_DEST_IMAGE_PATH, "/online/mtc/image%d/NV/Carrier/Carrier%d",
             mt_carrier_current_info.defalt_info.mt_image_index, mt_carrier_current_info.defalt_info.mt_carrier_index);
    /* ���ͨ����Ӫ�̾����XML��Ϣ */
    memcpy((char*)carrier_updata_stutus.mt_general_plmn, (char*)mt_carrier_current_info.general_info.mt_carrier_plmn_info.plmn, MTC_PLMN_MAX);
    snprintf(carrier_updata_stutus.mt_general_image_path, MTC_DEST_IMAGE_PATH, "/online/mtc/image%d", mt_carrier_current_info.general_info.mt_image_index);
    snprintf(carrier_updata_stutus.mt_general_carrier_xml_path, MTC_DEST_IMAGE_PATH, "/online/mtc/image%d/NV/Carrier/Carrier%d",
             mt_carrier_current_info.general_info.mt_image_index, mt_carrier_current_info.general_info.mt_carrier_index);
    /* Ĭ���л����û��趨��sim����������Ӫ�� */
    carrier_updata_stutus.mt_dest_carrier_type = MTC_DEST_CARRIER_SET;
#else
    snprintf(carrier_updata_stutus.mt_dest_carrier_xml_path,MTC_DEST_IMAGE_PATH,"/mtc/image%d/NV/Carrier/Carrier%d",dest_mt_carrier_info.mt_image_index,dest_mt_carrier_info.mt_carrier_index);
#endif /* MBB_MULTI_CARRIER */
    carrier_updata_stutus.mt_carrier_xml_updata_status =1;
    /*����д���ļ���ȥ*/
    ret = balong_mt_carrier_save_file_data(BALONG_MT_CARRIER_UPDATA_STATUS_FILE,(char *)&carrier_updata_stutus,sizeof(carrier_updata_stutus));
    return ret;
}

/**
* ����:�л�����ľ���ִ�к���
* ����:
* @��
* ����ֵ:
* ��
*/
static void balong_mt_carrier_switch_carrier_fn(void)
{
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    u32 result = 0;
    MTCSWT_AUTO_MANUL_STATUS_STRU  mtcswt_status = {0};
#else
    /*����Ҫ���ֵ�NV�ļ���·����*/
    char temp_user_xml_save_path[MTC_FILE_PATH_MAX] = {0};
    (void)snprintf(temp_user_xml_save_path,sizeof(temp_user_xml_save_path),"/mtc/image%d/NV/Carrier/Carrier%d",mt_carrier_current_info.carrier_info.mt_image_index,
                                                                                                       mt_carrier_current_info.carrier_info.mt_carrier_index);
    /*���浱ǰ���е�NV������Ϣ*/
    if(!bsp_nvm_mtcarrier_backup(temp_user_xml_save_path))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_save_xml fail\n");
        return ;
    }
#endif /* MBB_MULTI_CARRIER */
    /*��Ҫ�л����ľ��񱣴����ļ���ȥ*/
    if(!balong_mt_carrier_save_updata_status_file(dest_mt_carrier_info))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_save_updata_status_file fail\n");
        return ;
    }
    /*����Ҫ���е�plmn��NV�������ļ���ȥ*/
    memcpy((char *)&mt_carrier_cur_plmninfo_from_nv.cur_running_plmn,(char *)&dest_mt_carrier_info.mt_carrier_plmn_info.plmn,MTC_PLMN_MAX);
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    /* �л���ɺ��ٸ���OEMINFO�����˴����ٲ��� */
#else
    if (!balong_mt_carrier_save_current_plmn_nv((char *)&mt_carrier_cur_plmninfo_from_nv,sizeof (mt_carrier_cur_plmninfo_from_nv)))
    {
        return ;
    }
#endif /* MBB_MULTI_CARRIER */
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    /*��Ϊ�ֶ��л���������SIM���Զ��л����Ϊ0*/
    if ( MTC_MANUAL_SWITCH == mt_carrier_op)
    {
        result = bsp_nvm_write( NV_HUAWEI_MULTI_CARRIER_AUTO_SWITCH_I,
                                                (uint8 *)&(mtcswt_status),
                                                sizeof(mtcswt_status));
        if (result)
        {
            return;
        }
    }

    /* ����NV���Ա��л�����/��Ӫ��֮���ܹ���NV�ָ����� */
    result = bsp_nvm_backup();
    if (NV_OK != result)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"Fail to backup Multi-carrier NV!\n");
        return;
    }
    /*��������������ݰ���*/	
    balong_mt_carrier_restart();
#else
    /*��ʾ�л���ɱ��*/
    is_mt_carrier_switch_finish = TRUE;
#endif /* MBB_MULTI_CARRIER */

}

void balong_mt_carrier_restart(void)
{
    mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_restart\n");
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    mdrv_dload_set_curmode(DLOAD_MODE_DOWNLOAD);
    /* ���ù����ڴ�ħ���֣��´ν�������ģʽ */
    mdrv_dload_set_softload(true);
    mdrv_dload_normal_reboot();
#endif /* MBB_MULTI_CARRIER */
}
/**
* ����:�ڶྵ���������ʱ�����һ��"mtc/mtc_updata_status"����ļ��Ƿ���ڣ�ȷ�����ļ���һ������,���ҰѴ��ļ��е����ݶ���updata_status_addr��ȥ
*
* ����:
* @updata_status_addr                     ---���ļ��е����ݶ����ڴ��еĵ�ַ
* ����ֵ:
* �ɹ��򷵻�TRUE,ʧ�ܷ���FALSE
*/
bool balong_mt_carrier_read_datastatus(void * updata_status_addr)
{
    int fd = -1;

    struct rfile_stat_stru fd_stat = {0};
    if(updata_status_addr == NULL)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_updata_status: updata_status_addr is null\n");
    }
    /*���һ���ļ��Ƿ����*/
    if( 0 != bsp_stat(BALONG_MT_CARRIER_UPDATA_STATUS_FILE, &fd_stat))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_updata_status is not exit %s\n",BALONG_MT_CARRIER_UPDATA_STATUS_FILE);
        return FALSE;
    }
    /*open a file*/
    fd = bsp_open(BALONG_MT_CARRIER_UPDATA_STATUS_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
    if (-1 == fd)
    {
        return FALSE;
    }
    /*���Ϊ0�Ļ�Ҳ��Ϊ�ǲ����ڵģ���ʾ�д�����*/
    if (0 == fd_stat.size)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier: mt_carrier_updata_status_check() size fail");
        (void)bsp_close(fd);
        return FALSE;
    }

    /*read the data from file*/
    if (fd_stat.size != (loff_t)bsp_read(fd, (char *)updata_status_addr, (u32)fd_stat.size))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier: mt_carrier_updata_status_check() read fail");
        (void)bsp_close(fd);
        return FALSE;
    }
    (void)bsp_close(fd);
    return TRUE;

}

/**
* ����:�ྵ�����ģ����л�����,ΪʲôҪ��һ��������ִ���л���������Ϊ���Ĺ��ضྵ����������������ļ�������NV��ʱ����һ�㳤��AT�����ִ��ʱ����һ�㳤
*
* ����:
* ��
* ����ֵ:
* ��
*/
static int balong_mt_carrier_task(void *_c)
{
    /*Ҫ�����������ȼ��Ƚϸ�,�����˾�ִ��*/
    set_user_nice(current, 10);
    for (;;)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,": multcarrier stask sleeping...\n");
        /*һֱ�ȴ�ֱ������ź���*/
        /*lint -save -e722*/
        while(down_interruptible( &mt_carrier_sem));
        /*lint -restore*/
        /*�л�ǰ�ӳ�ϵͳ ���ߣ���ֹģ�������л�ʧЧ��ʱ��2s*/
        wake_lock_timeout(&multi_carrier_lock, (long)msecs_to_jiffies(2000)); /*lint !e526 !e628 !e516*/
#if (FEATURE_ON == MBB_MULTI_CARRIER)
        if ((NULL != mt_carrier_op_func[mt_carrier_op]) && (mt_carrier_op <MTC_OP_VALID))
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"start multcarrier option %d\n",mt_carrier_op);
            mt_carrier_op_func[mt_carrier_op]();
            mt_carrier_trace(MT_TRACE_ALWAYS,"complete multcarrier\n");
        }
#else
        mt_carrier_trace(MT_TRACE_ALWAYS,"start multcarrier\n");
        balong_mt_carrier_switch_carrier_fn();
        mt_carrier_trace(MT_TRACE_ALWAYS,"complete multcarrier\n");
#endif
    }
    /*lint -save -e527*/
    return 0;
    /*lint -restore*/
}

/**
* ����:�ྵ�����ģ����л�����ĳ�ʼ����������ʼ��ͬ��������ź����ʹ����л�����
*
* ����:
* ��
* ����ֵ:
* ��
*/
static int balong_mt_carrier_task_init(void)
{
    /*�ź�����ʼ������*/
    sema_init(&mt_carrier_sem, 0);
    /*��һ�ε�ʱ�򴴽��ྵ����л�����*/
    if(mt_carrier_tsk == NULL)
    {
        mt_carrier_tsk = kthread_run(balong_mt_carrier_task, NULL, "mt_carrier_task");
        if (IS_ERR(mt_carrier_tsk))
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"fork failed for mt_carrier_task\n");
        }
        else
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"multicarrier thread is pid %d\n", mt_carrier_tsk->pid);
        }
    }
    return 0;
}
/**
* ����:��buf_start��buf_end֮ǰ���ҵ�һ������find_str�ַ����ĵ�ַ�����û�в��ҵ��Ļ�����NULL
*
* ����:
* @buf_start             ---Ҫ�������ݵ���ʼ��ַ
* @buf_end               ---Ҫ�������ݵĽ�����ַ
* @find_str              ---Ҫ���ҵ��ַ���
* ����ֵ:
* ������ҵ�ָ����ʽ���ַ����ͷ����ַ��ĵ�ַ�����û�в��ҵ��ͷ���NULL
*/
static char *balong_mt_carrier_strstr(const char *buf_start, const char *buf_end,const char *find_str)
{
    const char *str_temp = NULL;
    if((buf_start == NULL) ||(buf_end == NULL) ||(find_str == NULL))
    {
        return NULL;
    }
    str_temp= strstr(buf_start,find_str);
    /*lint -save -e613*/
    if(str_temp > buf_end)
    {
        str_temp=NULL;
    }
    /*lint -restore*/
    return (char *)str_temp;
}
/**
* ����:��buffer�а����ض��ĸ�ʽ�������ݴ����mt_carrier_plmn_info�����ݽṹ��ȥ,�ҷ����ж��ٸ�plmn����Ŀ
*
* ����:
* @buffer_start             ---buffer���ݵ���ʼ��ַ
* @buffer_end               ---buffer���ݵĽ�����ַ,����������һ���ڴ˷�Χ��
* @support_plmn_array       ---�ѽ�����ɵ�������support_plmn_array�����ݽṹ����ڴ˱�����
* @support_plmn_cnt         ---����plmn����Ŀ
* ����ֵ:
* ��xml�ļ��е����ݸ�ʽһ��Ҫ�ϸ���Ҫ������֯��������ȷ����TRUE,�������󷵻�FALSE
*/
static bool balong_mt_carrier_decode_support_plmn(char* buffer_start,char* buffer_end,mt_carrier_plmn_info **support_plmn_array,int *support_plmn_cnt)
{

    char *buf_start=NULL;
    char *buf_end=NULL;
    char *buf_endtemp=NULL;
    int support_cn=0;

    mt_carrier_plmn_info  *support_temp=NULL;

    if((NULL == buffer_start) || (NULL == buffer_end) ||(NULL == support_plmn_array) ||(NULL == support_plmn_cnt))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_support_plmn: param fail");
        return FALSE;
    }

    buf_start=buffer_start;
    while((buf_start !=NULL) &&((buf_start=balong_mt_carrier_strstr(buf_start,buffer_end,START_BOARD_SUPPORT_PLMN_LABEL))!=NULL))
    {
        support_plmn_array[support_cn] =(mt_carrier_plmn_info *)kmalloc(sizeof(mt_carrier_plmn_info),GFP_KERNEL);

        if(support_plmn_array[support_cn]==NULL)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS," decode plmn memory alloc error\n");
            return FALSE;
        }
        support_temp=support_plmn_array[support_cn];
        memset(support_temp,'\0',sizeof(mt_carrier_plmn_info));
        buf_end=balong_mt_carrier_strstr(buf_start,buffer_end,END_BOARD_SUPPORT_PLMN_LABEL);
        if(NULL == buf_end)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_support_plmn: support plmn end fail");
            return FALSE;
        }
        buf_start=balong_mt_carrier_strstr(buf_start,buf_end,START_BOARD_SUPPORT_PLMN_NAME_LABEL);
        if(NULL == buf_start)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_support_plmn: plmn name start fail");
            return FALSE;
        }
        buf_endtemp =balong_mt_carrier_strstr(buf_start,buf_end,END_BOARD_SUPPORT_PLMN_NAME_LABEL);
        if(NULL == buf_endtemp)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_support_plmn: plmn name end fail");
            return FALSE;
        }
        memcpy((char *)support_temp->plmn,(char *)buf_start+strlen(START_BOARD_SUPPORT_PLMN_NAME_LABEL),buf_endtemp-buf_start-strlen(START_BOARD_SUPPORT_PLMN_NAME_LABEL));
        buf_start=balong_mt_carrier_strstr(buf_start,buf_end,START_BOARD_SUPPORT_S_NAME_LABEL);
        if(NULL == buf_start)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_support_plmn: plmn s_name start fail");
            return FALSE;
        }
        buf_endtemp =balong_mt_carrier_strstr(buf_start,buf_end,END_BOARD_SUPPORT_S_NAME_LABEL);
        if(NULL == buf_endtemp)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_support_plmn: plmn s_name end fail");
            return FALSE;
        }
        memcpy(support_temp->s_name,buf_start+strlen(START_BOARD_SUPPORT_S_NAME_LABEL),buf_endtemp-buf_start-strlen(START_BOARD_SUPPORT_S_NAME_LABEL));
        buf_start=balong_mt_carrier_strstr(buf_start,buf_end,START_BOARD_SUPPORT_L_NAME_LABEL);
        if(NULL == buf_start)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_support_plmn: plmn l_name start fail");
            return FALSE;
        }
        buf_endtemp =balong_mt_carrier_strstr(buf_start,buf_end,END_BOARD_SUPPORT_L_NAME_LABEL);
        if(NULL == buf_endtemp)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_support_plmn: plmn l_name end fail");
            return FALSE;
        }
        memcpy(support_temp->l_name,buf_start+strlen(START_BOARD_SUPPORT_L_NAME_LABEL),buf_endtemp-buf_start-strlen(START_BOARD_SUPPORT_L_NAME_LABEL));
        support_cn++;
        buf_start = buf_end;
    }
    *support_plmn_cnt =support_cn;
    return TRUE;
}
/**
* ����:��buffer�а����ض��ĸ�ʽ�������ݴ����my_carrier_info�����ݽṹ��ȥ,�ҷ����ж��ٸ�carrier����Ŀ
*
* ����:
* @buffer_start             ---buffer���ݵ���ʼ��ַ
* @buffer_end               ---buffer���ݵĽ�����ַ,����������һ���ڴ˷�Χ��
* @my_carrier_info            ---�ѽ�����ɵ�������my_carrier_info�����ݽṹ����ڴ˱�����
* @carrier_cnt                ---����carrier����Ŀ
* ����ֵ:
* ��xml�ļ��е����ݸ�ʽһ��Ҫ�ϸ���Ҫ������֯��������ȷ����TRUE,�������󷵻�FALSE
*/
static bool balong_mt_carrier_decode_carrier(char* buffer_start,char* buffer_end,mt_carrier_info **my_carrier_info,int *carrier_cnt)
{

    char *buf_start=NULL;
    char *buf_end=NULL;
    int carrier_id=0;
    int support_plmn_cnt=0;
    bool ret=FALSE;

    if((NULL == buffer_start) || (NULL == buffer_end) ||(NULL == my_carrier_info) ||(NULL == carrier_cnt))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_carrier: param fail");
        return FALSE;
    }

    buf_start=buffer_start;

    while((buf_start !=NULL) && ((buf_start=balong_mt_carrier_strstr(buf_start,buffer_end,START_MT_CARRIER_INDEX_LABEL))!=NULL))
    {
        my_carrier_info[carrier_id] =(mt_carrier_info *)kmalloc(sizeof(mt_carrier_info),GFP_KERNEL);
        if(my_carrier_info[carrier_id]==NULL)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"decode carrier memory alloc error\n");
            return FALSE;
        }
        memset(my_carrier_info[carrier_id],'\0',sizeof(mt_carrier_info));
        buf_end=balong_mt_carrier_strstr(buf_start, buffer_end,END_MT_CARRIER_INDEX_LABEL);
        if(NULL == buf_end)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_carrier: carrier index end fail");
            return FALSE;
        }
        my_carrier_info[carrier_id]->mt_carrier_index =simple_strtol(buf_start+strlen(START_MT_CARRIER_INDEX_LABEL),NULL,0);
        buf_start=balong_mt_carrier_strstr(buf_start,buf_end,MT_CARRIER_XML_VERSION_LABEL);
        if(NULL == buf_start)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_carrier: xml version fail");
            return FALSE;
        }
        my_carrier_info[carrier_id]->mt_carrier_xml_version =simple_strtol(buf_start+strlen(MT_CARRIER_XML_VERSION_LABEL),NULL,0);
        buf_start=balong_mt_carrier_strstr(buf_start,buf_end,START_MT_CARRIER_ID_LABEL);
        if(NULL == buf_start)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_carrier: carrier id fail");
            return FALSE;
        }
        my_carrier_info[carrier_id]->mt_carrier_id =simple_strtol(buf_start+strlen(START_MT_CARRIER_ID_LABEL),NULL,0);

        ret = balong_mt_carrier_decode_support_plmn(buf_start,buf_end,my_carrier_info[carrier_id]->mt_carrier_support_plmn_array,&support_plmn_cnt);
        if(FALSE == ret)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_carrier: decode support_plmn fail");
            return FALSE;
        }
        my_carrier_info[carrier_id]->mt_carrier_support_plmn_cnt =support_plmn_cnt;
        carrier_id++;
        buf_start =buf_end;

    }
    *carrier_cnt =carrier_id;
    return TRUE;
}
/**
* ����:��buffer�а����ض��ĸ�ʽ�������ݴ����my_image_info�����ݽṹ��ȥ,�ҷ����ж��ٸ�image����Ŀ
*
* ����:
* @buffer_start             ---buffer���ݵ���ʼ��ַ
* @buffer_end               ---buffer���ݵĽ�����ַ,����������һ���ڴ˷�Χ��
* @my_image_info            ---�ѽ�����ɵ�������mt_image_info�����ݽṹ����ڴ˱�����
* @image_cnt                ---����image����Ŀ
* ����ֵ:
* ��xml�ļ��е����ݸ�ʽһ��Ҫ�ϸ���Ҫ������֯��������ȷ����TRUE,�������󷵻�FALSE
*/
static bool balong_mt_carrier_decode_image_info(char* buffer_start,char* buffer_end,mt_image_info ** my_image_info,int *image_cnt)
{
    char *buf_start=NULL;
    char *buf_end=NULL;
    char *buf_endtemp=NULL;
    int image_id=0;
    int carrier_cnt=0;
    bool ret = FALSE;
    if((NULL == buffer_start) || (NULL == buffer_end) ||(NULL == my_image_info) ||(NULL == image_cnt))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_image_info: param fail");
        return FALSE;
    }

    buf_start=buffer_start;

    while((buf_start!=NULL) && ((buf_start=balong_mt_carrier_strstr(buf_start,buffer_end,START_MT_IMAGE_ID_LABEL))!=NULL))
    {
        my_image_info[image_id] =(mt_image_info *)kmalloc(sizeof(mt_image_info),GFP_KERNEL);
        if(my_image_info[image_id]==NULL)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"memory alloc error\n");
            return FALSE;
        }
        memset(my_image_info[image_id],'\0',sizeof(mt_image_info));
        buf_end=balong_mt_carrier_strstr(buf_start,buffer_end,END_MT_IMAGE_ID_LABEL);
        if(NULL == buf_end)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_image_info: image end fail");
            return FALSE;
        }
        my_image_info[image_id]->mt_image_index =simple_strtol(buf_start+strlen(START_MT_IMAGE_ID_LABEL),NULL,0);
        buf_start=balong_mt_carrier_strstr(buf_start,buf_end,START_MT_IMAGE_VERSION_LABEL);
        if(NULL == buf_start)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_image_info: image version start fail");
            return FALSE;
        }
        buf_endtemp=balong_mt_carrier_strstr(buf_start,buf_end,END_MT_IMAGE_VERSION_LABEL);
        if(NULL == buf_endtemp)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_image_info: image version end fail");
            return FALSE;
        }
        memcpy(my_image_info[image_id]->mt_image_version,buf_start+strlen(START_MT_IMAGE_VERSION_LABEL),buf_endtemp-buf_start-strlen(START_MT_IMAGE_VERSION_LABEL));
        ret=balong_mt_carrier_decode_carrier(buf_start,buf_end,my_image_info[image_id]->mt_carrier_info_array,&carrier_cnt);
        if(FALSE == ret)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_image_info: decode carrier fail");
            return FALSE;
        }
        my_image_info[image_id]->mt_carrier_cnt=carrier_cnt;
        image_id++;
        buf_start =buf_end;
    }
    *image_cnt =image_id;
    mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_image_info complete image_cnt %d\n",image_id);
    return TRUE;
}

/**
* ����:��ӡϵͳ����֧�ֵ�PLMN��Ϣ
*
* ����:
* @mt_info_my             ---mt_info_myϵͳ֧�ֵ����е�PLMN�����ݽṹ
*/
static int balong_mt_carrier_dump_mt_info(mt_info *mt_info_my)
{
    int i,j,k;
    mt_carrier_plmn_info *temp=NULL;
    if(NULL == mt_info_my)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_dump_mt_info: param fail");
        return 0;
    }
    mt_carrier_trace(MT_TRACE_ALWAYS,"soft version is %s\n",mt_info_my->mt_software_version);

    for(i=0;i<mt_info_my->mt_image_cnt;i++)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"image index %d image version %s,carrier cnt %d\n",mt_info_my->mt_image_info_array[i]->mt_image_index,mt_info_my->mt_image_info_array[i]->mt_image_version,mt_info_my->mt_image_info_array[i]->mt_carrier_cnt);
        for(j=0;j<mt_info_my->mt_image_info_array[i]->mt_carrier_cnt;j++)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"carrier index %d carrier id %d,plmn cnt %d xml version %d\n",mt_info_my->mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_index,mt_info_my->mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_id,mt_info_my->mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_cnt,mt_info_my->mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_xml_version);
            for(k=0;k<mt_info_my->mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_cnt;k++)
            {
                temp=mt_info_my->mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_array[k];
                mt_carrier_trace(MT_TRACE_ALWAYS,"plmn_name %s ,s_name %s,l_name %s\n",temp->plmn,temp->s_name,temp->l_name);
            }
        }
    }
    return 0;
}
/**
* ����:��buffer�а����ض��ĸ�ʽ�������ݴ����mt_info�����ݽṹ��ȥ
*
* ����:
* @buffer            ---buffer���ݵ���ʼ��ַ
* @end               ---buffer���ݵĽ�����ַ
* @mt_info_my        ---�ѽ�����ɵ����ݴ���ڴ����ݽṹ��ȥ
*/
static int balong_mt_carrier_decode_config_info(char* buffer,char* end,mt_info *mt_info_my)
{
    char *buf_start=NULL;
    char *buf_end =NULL;
    char *buf_temp_end=NULL;
    int image_cnt=0;
    if((NULL == buffer) || (NULL == end) ||(NULL == mt_info_my))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_config_info: param fail");
        return -1;
    }

    buf_start=balong_mt_carrier_strstr(buffer,end,START_MT_SOFTWARE_VERSION_LABEL);
    if(NULL == buf_start)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_config_info: software start fail");
#if (FEATURE_ON == MBB_MULTI_CARRIER)
        return -1;
#endif
    }
    buf_temp_end=balong_mt_carrier_strstr(buf_start,end,END_MT_SOFTWARE_VERSION_LABEL);
    if(NULL == buf_temp_end)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_config_info: software end fail");
#if (FEATURE_ON == MBB_MULTI_CARRIER)
        return -1;
#endif
    }
    /*lint -save -e613*/
    memcpy(mt_info_my->mt_software_version,buf_start+strlen(START_MT_SOFTWARE_VERSION_LABEL),buf_temp_end-buf_start-strlen(START_MT_SOFTWARE_VERSION_LABEL));
    /*lint -restore*/
    buf_end =balong_mt_carrier_strstr(buf_start,end,END_MT_XML_CONFIG_LABEL);
    if(NULL == buf_end)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_decode_config_info: config end fail");
#if (FEATURE_ON == MBB_MULTI_CARRIER)
        return -1;
#endif
    }
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    if (FALSE == balong_mt_carrier_decode_image_info(buf_start,buf_end,mt_info_my->mt_image_info_array,&image_cnt))
    {
        return -1;
    }
#else
    balong_mt_carrier_decode_image_info(buf_start,buf_end,mt_info_my->mt_image_info_array,&image_cnt);
#endif
    mt_info_my->mt_image_cnt = image_cnt;
    balong_mt_carrier_dump_mt_info(mt_info_my);
    return 0;
}

/**
* ����:��Image_Xml_Config.xml�ļ����ҽ�������ص����ݽṹ��ȥ
*
* ����:
* ��
*/
static bool balong_mt_carrier_read_xml_file(void)
{
    int fd = -1;
    struct rfile_stat_stru  fd_stat;
    char *data_start=NULL;
    char *data_end =NULL;

    /*���һ���ļ��Ƿ����*/
    if( 0 != bsp_stat(BALONG_MT_CARRIER_PLMN_CONFIG_FILE, &fd_stat))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_read_xml_file: is not exit %s\n",BALONG_MT_CARRIER_PLMN_CONFIG_FILE);
        return FALSE;
    }
    fd = bsp_open(BALONG_MT_CARRIER_PLMN_CONFIG_FILE, O_RDONLY , S_IRUSR | S_IWUSR);
    if (-1 == fd)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_read_xml_file: bsp_open fail");
        return FALSE;
    }

    data_start =(char *)kmalloc((size_t)fd_stat.size,GFP_KERNEL);
    if(data_start == NULL)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier: get memory fail");
        (void)bsp_close(fd);
        return FALSE;
    }
    data_end   =data_start + fd_stat.size;
    /*���ļ��ж�����*/
    if (fd_stat.size != bsp_read(fd, data_start, (u32)fd_stat.size))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier: read()  fail");
        (void)bsp_close(fd);
#if (FEATURE_ON == MBB_MULTI_CARRIER)
        kfree(data_start);
#endif /* MBB_MULTI_CARRIER */
        return FALSE;
    }
    balong_mt_carrier_decode_config_info(data_start,data_end,&mt_carrier_plmn_supported);

    if (-1 == bsp_close(fd))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier: close()  fail");
    }
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    kfree(data_start);
#endif /* MBB_MULTI_CARRIER */
    return TRUE;
}

/**
* ����: �ྵ��ģ��ĳ�ʼ�����������ȶ���ǰ��PLMN��Ĭ�ϵ�PLMN��NV�����ļ��������NV�����ڵĻ��ͷ���ʧ��,�ɹ�����NV������
* �����ྵ���л�����
*
* ����:
* ��
*/
static int __init balong_mt_carrier_init(void)
{
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    int rt;
    u32 channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_MTC_SIM_INIT;
    memset((char *)&mt_carrier_cur_plmninfo_from_nv,'\0',(unsigned int)sizeof(mt_carrier_cur_plmninfo_from_nv));
    if (!huawei_mtc_get_plmn_from_oem((char *)&mt_carrier_cur_plmninfo_from_nv,sizeof(mt_carrier_cur_plmninfo_from_nv)))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_init:get plmn from oeminfo fail ");
        return -1;
    }
    /*��ʼ��ϵͳ ��*/
    wake_lock_init(&multi_carrier_lock, WAKE_LOCK_SUSPEND, MTC_DRIVER_NAME);

    /*ע��icc�ص�����������sim���л�*/	
    rt = bsp_icc_event_register(channel_id, (read_cb_func)huawei_mtc_sim_switch_cb, NULL, NULL, NULL);
    if(rt != 0)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"MTC icc event register failed.\n");
        return -1;
    }
#else
    u32 ret = 0;
    memset((char *)&mt_carrier_cur_plmninfo_from_nv,'\0',sizeof(mt_carrier_cur_plmninfo_from_nv));
    ret=bsp_nvm_read(NV_ID_DRV_MTC_PLMN_CFG, (u8*)&mt_carrier_cur_plmninfo_from_nv, sizeof(mt_carrier_cur_plmninfo_from_nv));
    if(NV_OK != ret)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_init: read nv  fail");
        return -1;
    }
    mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_init  run %s,default %s\n",(char *)mt_carrier_cur_plmninfo_from_nv.cur_running_plmn,(char *)mt_carrier_cur_plmninfo_from_nv.default_plmn);
#endif /* MBB_MULTI_CARRIER */
    balong_mt_carrier_task_init();

    return 0;
}

#if (FEATURE_ON == MBB_MULTI_CARRIER)

static mtc_ver_get_func mtc_ver_get_function[MTC_VERSION_MAX]=
{
    huawei_mtc_get_feature_version,/*MTC_FEATURE_VER*/
    huawei_mtc_get_package_version,/*MTC_PACKAGE_VER*/
    huawei_mtc_get_oem_version,/*MTC_OEM_VER*/
    huawei_mtc_get_image_version/*MTC_IMAGE_VER*/
};
extern char * bsp_version_get_firmware(void);
/*****************************************************************************
 �� �� ��  : huawei_mtc_get_plmn_from_oem
 ��������  : �� OMEINFO�л�ȡplmn
 �������  : 
 �������  : ��
 �� �� ֵ  : TRUE:��ȡ�ɹ�
             FALSE:��ȡʧ��
*****************************************************************************/
bool huawei_mtc_get_plmn_from_oem(char *buffer,unsigned int size)
{
    if ((NULL == buffer ) ||(size != sizeof(NV_DRV_PLMN_CFG)))
    {
        return FALSE;
    }
    return flash_get_share_region_info(RGN_MTCARRIER_PLMN,buffer,size);
}
/*****************************************************************************
 �� �� ��  : huawei_mtc_set_plmn_from_oem
 ��������  : �� OMEINFO��д��plmn
 �������  : 
 �������  : ��
 �� �� ֵ  : TRUE:���³ɹ�
             FALSE:����ʧ��
*****************************************************************************/
bool huawei_mtc_set_plmn_from_oem(char *buffer, unsigned int buf_len)
{
    if ((NULL == buffer ) ||(buf_len != sizeof(NV_DRV_PLMN_CFG)))
    {
        return FALSE;
    }
    return flash_update_share_region_info(RGN_MTCARRIER_PLMN, buffer, buf_len);
}

/*****************************************************************************
 �� �� ��  : huawei_mtc_get_version_entry
 ��������  : ��ȡMTC��ذ汾�Žӿ�
 �������  : char *version  �����洢�汾�ŵĵ�ַ,size:�����������Ĵ�С
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void huawei_mtc_get_version_entry(char *version, unsigned char  size,mtc_version_type ver_type)
{
    if (( NULL == version) || ( 0 == size ))
    {
        return;
    }
    (void)memset(version, 0, size);
    if ( NULL != mtc_ver_get_function[ver_type])
    {
        mtc_ver_get_function[ver_type](version,size);
        mt_carrier_trace(MT_TRACE_ALWAYS,"mtc_get_version type is %d",ver_type);
    }
    return;	
}
EXPORT_SYMBOL(huawei_mtc_get_version_entry);
/*****************************************************************************
 �� �� ��  : huawei_mtc_get_package_version
 ��������  : ��ȡ�����汾�Žӿ�
 �������  : char *version  �����洢�汾�ŵĵ�ַ,size:�����������Ĵ�С
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void huawei_mtc_get_package_version(char *version, unsigned char size)
{
    bool res = FALSE;
    char *ver_ptr = NULL;	
    if (( NULL == version) || ( 0 == size ))
    {
        return;
    }
    /*����Image_Xml_Config.xml�����package_ver*/
    res = balong_mt_carrier_check_plmn_ready();
    if (!res)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_get_package_version  fail,add the original ver");
        ver_ptr = bsp_version_get_firmware();
        (void)strncpy(version, (const char *)ver_ptr, (unsigned int)strlen(ver_ptr));
        return;
    }
    (void)snprintf(version,size,"%s",mt_carrier_plmn_supported.mt_software_version);
}
/*****************************************************************************
 �� �� ��  : huawei_mtc_get_oem_version
 ��������  : ��ȡOEM�汾�Žӿ�
 �������  : char *version  �����洢�汾�ŵĵ�ַ,size:���ݿռ��С
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void huawei_mtc_get_oem_version(char *version,unsigned char  size)
{
    u32 ret = 0;
    char product_name[MTC_SOFT_VER_MAX]={0};
    MULTI_IMAGE_TYPE_STRU  mtc_oem_ver = {0};
    if (( NULL == version) || ( 0 == size ))
    {
        return;
    }
    ret=bsp_nvm_read(NV_HUAWEI_MULTI_IMAGE_I, (u8*)&mtc_oem_ver, sizeof(MULTI_IMAGE_TYPE_STRU));
    if(NV_OK != ret)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_get_oem_version  fail");
    }
    /* ��ȡ��Ʒ������Ϣ*/
    if (0 != mdrv_dload_get_productname(product_name, MTC_SOFT_VER_MAX))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"outer product name read error.");
    }
    (void)snprintf(version,size,"%sCUST-B001SP%02d_%s",
                          product_name,
                          mtc_oem_ver.vendor_config_file_ver,
                          pc_vendor_oem_info[mtc_oem_ver.vendor_ver]);
}
/*****************************************************************************
 �� �� ��  : huawei_mtc_get_image_version
 ��������  : ��ȡIMAGE�汾�Žӿ�
 �������  : char *version  �����洢�汾�ŵĵ�ַ,size:���ݿռ��С
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void huawei_mtc_get_image_version(char *version,unsigned char  size)
{
    if (( NULL == version) || ( 0 == size ))
    {
        return;
    }
    (void)snprintf(version,size,"%s",PRODUCT_DLOAD_SOFTWARE_VER);
}
/*****************************************************************************
 �� �� ��  : huawei_mtc_get_feature_version
 ��������  : ��ȡMTC��feature�汾�Žӿ�
 �������  : char *version  �����洢�汾�ŵĵ�ַ,size:���ݿռ��С
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void huawei_mtc_get_feature_version(char *version,unsigned char  size)
{
    if (( NULL == version) || ( 0 == size ))
    {
        return;
    }
    (void)snprintf(version,size,"%s",MTC_FEATURE_VERSION);
}

/*****************************************************************************
 �� �� ��  : huawei_mtc_check_sim_plmn
 ��������  : ����sim_plmn
 �������  : char *data_start  ��ǰcarrier_plmn���ݷ�Χ�Ŀ�ʼ��ַ
                            char * data_end  ��ǰcarrier_plmn���ݷ�Χ�Ľ�����ַ
                            char *sim_plmn  ��sim���л�õ�plmn
 �������  : ��
 �� �� ֵ  : TRUE:���ҳɹ�
                        FALSE:����ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
bool huawei_mtc_check_sim_plmn(char *data_start, char * data_end,char* sim_plmn)
{
    char *buf_start=NULL;
    char *buf_end =NULL;
    char sim_plmn_temp[MTC_PLMN_MAX]={0};
    u8 plmn_size = 0;

    if((NULL == data_start) || (NULL == data_end) ||(NULL == sim_plmn))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_check_sim_plmn param is error");
        return FALSE;
    }
    buf_start = 	data_start;
    while((buf_start!=NULL) && ((buf_start=balong_mt_carrier_strstr(buf_start,data_end,START_MT_SIM_PLMN_LABEL))!=NULL))
    {
        buf_end=balong_mt_carrier_strstr(buf_start,data_end,END_MT_SIM_PLMN_LABEL);
        if(NULL == buf_end)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_check_sim_plmn find /sim_plmn is error");
            return FALSE;
        }
        plmn_size = buf_end-buf_start-strlen(START_MT_SIM_PLMN_LABEL);
        if (plmn_size > MTC_PLMN_MAX)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_check_sim_plmn plmn size is error");
            return FALSE;
        }
        memset(sim_plmn_temp,0,sizeof(sim_plmn_temp));
        memcpy(sim_plmn_temp,(buf_start+strlen(START_MT_SIM_PLMN_LABEL)),plmn_size);
        if(!strcmp((const char*)sim_plmn,(const char*)sim_plmn_temp))
        {
            return TRUE;
        }
        buf_start = buf_end;
    }
    mt_carrier_trace(MT_TRACE_ALWAYS,"did not get the sim plmn in this carrier list");
    return FALSE;	
}
/*****************************************************************************
 �� �� ��  : huawei_mtc_plmn_list_parse
 ��������  : ����plmn_list.xml
 �������  : char *data_start  xml�Ŀ�ʼ��ַ
                            char * data_end  xml�Ľ�����ַ
                            char *sim_plmn  ��sim���л�õ�plmn
                            char * carrier_plmn ��plmn list�в�ѯ���Ķ�Ӧ��Ӫ�̴���plmn
 �������  : ��
 �� �� ֵ  : TRUE:�л��ɹ�
                        FALSE:�л�ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
bool huawei_mtc_plmn_list_parse(char *xml_data_start, char * xml_data_end,char* sim_plmn, char * carrier_plmn)
{
    char *data_end=NULL;
    char *buf_start=NULL;
    char *buf_end =NULL;
    char *buf_temp_end=NULL;
    char  carrier_plmn_temp[MTC_PLMN_MAX]={0};
    u8 plmn_size = 0;
    bool check_res = FALSE;
    
    if((NULL == xml_data_start) || (NULL == xml_data_end) ||(NULL == sim_plmn)||(NULL == carrier_plmn))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_plmn_list_parse param is error");
        return FALSE;
    }
    buf_start = xml_data_start;
    data_end = balong_mt_carrier_strstr(buf_start,xml_data_end,END_MT_CARRIER_PLMN_LIST_LABEL);
    if ( NULL == data_end )
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_plmn_list_parse find /plmn_list error");
        return FALSE;
    }
    /*�Ƚ�carrier_plmn���ó�ͨ�ã��ٽ��в���*/
    strncpy(carrier_plmn,MTC_GEN_PLMN,MTC_PLMN_MAX);
    while((buf_start!=NULL) && ((buf_start=balong_mt_carrier_strstr(buf_start,data_end,START_MT_CARRIER_PLMN_LABEL))!=NULL))
    {
        buf_temp_end=balong_mt_carrier_strstr(buf_start,data_end,">");
        if(NULL == buf_temp_end)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_plmn_list_parse find > error");
            return FALSE;
        }
        plmn_size = buf_temp_end-buf_start-strlen(START_MT_CARRIER_PLMN_LABEL);
        if (plmn_size > MTC_PLMN_MAX)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_plmn_list_parse plmn size is error");
            return FALSE;
        }
        memset(carrier_plmn_temp,0,sizeof(carrier_plmn_temp));
        memcpy(carrier_plmn_temp,(buf_start+strlen(START_MT_CARRIER_PLMN_LABEL)),plmn_size);
        buf_end = balong_mt_carrier_strstr(buf_start,data_end,END_MT_CARRIER_PLMN_LABEL);
        if(NULL == buf_end)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_plmn_list_parse find /carrier_plmn error");
            return FALSE;
        }
        check_res = huawei_mtc_check_sim_plmn(buf_start,buf_end,sim_plmn);
        if (check_res)
        {
            strncpy(carrier_plmn,carrier_plmn_temp,MTC_PLMN_MAX);
            break;
        }
        buf_start = buf_end;
    }
    mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_plmn_list_parse ok, plmn is %s",carrier_plmn);
    return TRUE;
}
/*****************************************************************************
 �� �� ��  : huawei_mtc_get_carrier_plmn
 ��������  : ��鵱ǰ�����sim����plmn��Ӧ����Ӫ��plmn
 �������  : char *sim_plmn  ��sim���л�õ�plmn
                            char * carrier_plmn ��plmn list�в�ѯ���Ķ�Ӧ��Ӫ�̴���plmn
 �������  : ��
 �� �� ֵ  : TRUE:�л��ɹ�
                        FALSE:�л�ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
bool huawei_mtc_get_carrier_plmn(char *sim_plmn,char * carrier_plmn)
{
    int fd = -1;
    struct kstat  fd_stat;
    char *data_start=NULL;
    char *data_end=NULL;
    bool parse_res = FALSE;
    /*���һ���ļ��Ƿ����*/
    if( 0 != bsp_stat(MTC_PLMN_LIST_FILE, &fd_stat))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"MTC_PLMN_LIST_FILE  does not exist \n");
        return FALSE;
    }
    fd = bsp_open(MTC_PLMN_LIST_FILE, O_RDONLY , S_IRUSR | S_IWUSR);
    if (-1 == fd)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"MTC_PLMN_LIST_FILE  open fail \n");
        return FALSE;
    }

    data_start =(char *)kmalloc((size_t)fd_stat.size,GFP_KERNEL);
    if(data_start == NULL)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS," mtc malloc  fail \n");
        (void)bsp_close(fd);
        return FALSE;
    }
    /*���ļ��ж�����*/
    if (fd_stat.size != bsp_read(fd, data_start, (u32)fd_stat.size))
    {
        (void)bsp_close(fd);
        kfree(data_start);
        data_start = NULL;
        mt_carrier_trace(MT_TRACE_ALWAYS," mtc bsp_read  fail \n");
        return FALSE;
    }
    (void)bsp_close(fd);
    data_end = 	data_start+fd_stat.size;
    /*����xml�ļ�������sim_plmn��Ӧ��carrier_plmn*/
    parse_res = huawei_mtc_plmn_list_parse(data_start,data_end,sim_plmn,carrier_plmn);
    kfree(data_start);
    data_start = NULL;
    return parse_res;
}
/*****************************************************************************
 �� �� ��  : huawei_mtc_check_auto_switch_valid
 ��������  : ��鵱ǰ�Ƿ��������SIM���Զ��л� 
 �������  : char *plmn  ��sim���л�õ�plmn
 �������  : ��
 �� �� ֵ  : TRUE:�л��ɹ�
                        FALSE:�л�ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
bool  huawei_mtc_check_auto_switch_valid(void)
{
    MTCSWT_AUTO_MANUL_STATUS_STRU  mtcswt_status = {0};
    (void)bsp_nvm_read(NV_HUAWEI_MULTI_CARRIER_AUTO_SWITCH_I, (u8*)&mtcswt_status, sizeof(mtcswt_status));
    if (MTC_SWITCH_OPEN == mtcswt_status.auto_status)
    {
        return TRUE;
    }
    return FALSE;
}
/*****************************************************************************
 �� �� ��  : huawei_mtc_get_plmn
 ��������  : ��C�˵�hplmn���ݽṹ����ȡplmn
 �������  : stHplmn  ��C�˵õ���hplmn���ݽṹ
                            sim_plmn ��װ�õ�plmn
 �������  : ��
 �� �� ֵ  : TRUE:�л��ɹ�
                        FALSE:�л�ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
void huawei_mtc_get_plmn(MTC_HPLMN_WITH_MNC_LEN_STRU *stHplmn, char *sim_plmn)
{
    char plmn_temp[MTC_PLMN_MAX] = {0};
    char basic_zero = '0';
    plmn_temp[0] = (0x0f & stHplmn->stHplmn.Mcc) + basic_zero;
    plmn_temp[1] = ((0x0f00 & stHplmn->stHplmn.Mcc) >> 8) + basic_zero;
    plmn_temp[2] = ((0x0f0000 & stHplmn->stHplmn.Mcc) >> 16) + basic_zero;
    /*2λmnc*/	
    if (2 == stHplmn->ucHplmnMncLen)
    {
        plmn_temp[3] = (0x0f & stHplmn->stHplmn.Mnc) + basic_zero;
        plmn_temp[4] = ((0x0f00 & stHplmn->stHplmn.Mnc) >> 8) + basic_zero;
    }
    /*3λmnc*/	
    else if (3 == stHplmn->ucHplmnMncLen)
    {
        plmn_temp[3] = (0x0f & stHplmn->stHplmn.Mnc) + basic_zero;
        plmn_temp[4] = ((0x0f00 & stHplmn->stHplmn.Mnc) >> 8) + basic_zero;
        plmn_temp[5] = ((0x0f0000 & stHplmn->stHplmn.Mnc) >> 16) + basic_zero;
    }
    else
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"HPLMN MNC LEN INVAILID");
    }
    strncpy(sim_plmn,plmn_temp,MTC_PLMN_MAX);
    mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_get_plmn is %s",sim_plmn);
}

/*****************************************************************************
 �� �� ��  : huawei_mtc_auto_switch_entry
 ��������  : SIM���Զ��л������
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :
*****************************************************************************/
void huawei_mtc_auto_switch_entry(void)
{
    char carrier_plmn[MTC_PLMN_MAX] = {0};
    bool ulRet = FALSE;	
    int times = 10;	//�ȴ�10��
    struct kstat  fd_stat;
	
    /*���online����mtc�ļ����Ƿ����*/
    while ((0 != bsp_stat("/online/mtc/",&fd_stat)) && ( 0 != times ))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"/online/mtc is not exist wait for 1s");
        msleep(1000);
        times --;		
    }
    /*���bodysar��plmn-list�Ƿ���������sim��plmn,����ǣ�ͬ���bodysar*/
#if (FEATURE_ON ==  MBB_FEATURE_BODYSAR)
    {
        extern void bodysar_powr_limit_by_plmnlist_check(char *sim_plmn);
        bodysar_powr_limit_by_plmnlist_check(g_sim_plmn);
    }
#endif
    /*�ж�sim���Զ��л������Ƿ���Ч���Ƿ������ֶ��л�*/
    ulRet = huawei_mtc_check_auto_switch_valid();
    if (!ulRet)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_auto_switch_entry auto switch is invalid");
        return ;
    }
	
    /*���ݴ����sim��plmn�����Ҷ�Ӧ��Ӫ�̵ı�׼plmn*/
    ulRet = huawei_mtc_get_carrier_plmn(g_sim_plmn,carrier_plmn);
    if (!ulRet)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_auto_switch_entry get carrier plmn error");
        return ;
    }
    /*���ú�˼�����л�����*/
    (void)balong_basic_exec_mtcarrier_cmd(carrier_plmn);

}
/*****************************************************************************
 �� �� ��  : huawei_mtc_sim_switch_cb
 ��������  : SIM����ʼ������A�˷�icc��Ϣ�Ļص�����
 �������  : NULL
 �������  : ��
 �� �� ֵ  : 0:�ɹ�  -1:ʧ��
*****************************************************************************/
signed int huawei_mtc_sim_switch_cb(void)
{
    int read_len = 0;
    MTC_HPLMN_WITH_MNC_LEN_STRU plmn_info = {{0},0,{0}};
    unsigned int channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_MTC_SIM_INIT;
    read_len = bsp_icc_read(channel_id, (unsigned char*)&plmn_info, sizeof(plmn_info));
    if ( sizeof(plmn_info) != read_len )
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"MTC bsp_icc_read len is %d.\n",read_len);
        return -1;		
    }
    huawei_mtc_get_plmn((MTC_HPLMN_WITH_MNC_LEN_STRU *)&plmn_info, g_sim_plmn);
    /*�����л�������*/
    mt_carrier_op = MTC_SIM_SWITCH;
    up(&mt_carrier_sem);
    return 0;
}
/*****************************************************************************
 �� �� ��  : huawei_mtc_free_mem
 ��������  : �����ͷ�������ڴ�
 �������  : ��
 �������  : ��
 �� �� ֵ  :��
*****************************************************************************/
void huawei_mtc_free_mem(void)
{
    int image_num = 0;
    int carrier_num = 0;
    int plmn_num = 0;
    for (image_num = 0; image_num < MTC_IMAGE_MAX; image_num++)
    {
        if (NULL != mt_carrier_plmn_supported.mt_image_info_array[image_num])        
        {
            for (carrier_num = 0; carrier_num < MTC_PER_IMAGE_MAX_CARRIER; carrier_num++)
            {
                if (NULL != mt_carrier_plmn_supported.mt_image_info_array[image_num]->mt_carrier_info_array[carrier_num])
                {
                    if (NULL != mt_carrier_plmn_supported.mt_image_info_array[image_num]->mt_carrier_info_array[carrier_num]->mt_carrier_support_plmn_array[plmn_num])
                    {
                        kfree(mt_carrier_plmn_supported.mt_image_info_array[image_num]->mt_carrier_info_array[carrier_num]->mt_carrier_support_plmn_array[plmn_num]);
                        mt_carrier_plmn_supported.mt_image_info_array[image_num]->mt_carrier_info_array[carrier_num]->mt_carrier_support_plmn_array[plmn_num] = NULL;			
                    }
                    kfree(mt_carrier_plmn_supported.mt_image_info_array[image_num]->mt_carrier_info_array[carrier_num]);
                    mt_carrier_plmn_supported.mt_image_info_array[image_num]->mt_carrier_info_array[carrier_num] = NULL;
                }
            }
            kfree(mt_carrier_plmn_supported.mt_image_info_array[image_num]);
            mt_carrier_plmn_supported.mt_image_info_array[image_num] = NULL;
        }
    }
}

#endif /* MBB_MULTI_CARRIER */

/*��ģ���ʹ�õĹ����в��ᶯ̬ж��*/
module_init(balong_mt_carrier_init);
