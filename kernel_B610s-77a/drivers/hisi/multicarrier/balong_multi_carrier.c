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
/*自动或手动切换标记*/
mtc_op_type mt_carrier_op = MTC_OP_VALID;
/*用来存放SIM卡的plmn*/
char g_sim_plmn[MTC_PLMN_MAX] = {"00000"};
/*用来进行切换时锁睡眠*/
struct wake_lock multi_carrier_lock;  
#define MTC_DRIVER_NAME  "MT_CARRIER"
#endif /* MBB_MULTI_CARRIER */

unsigned int mt_carrier_trace_mask =MT_TRACE_WARNING |MT_TRACE_ALWAYS;
static bool is_mt_carrier_switch_finish = TRUE;

static struct semaphore mt_carrier_sem;
static struct task_struct *mt_carrier_tsk=NULL;

static bool balong_mt_carrier_read_xml_file(void);
/*系统支持的所有镜像文件表*/
static mt_info  mt_carrier_plmn_supported = {{0,0},0,{NULL,NULL,NULL,NULL}};

#if (FEATURE_ON == MBB_MULTI_CARRIER)
#else
static NV_DRV_PLMN_CFG mt_carrier_cur_plmninfo_from_nv;
#endif /* MBB_MULTI_CARRIER */

/*当前系统支持运行的镜像和default镜像*/
static mtc_cur_info mt_carrier_current_info ;

/*存放系统临时的目的镜像的信息*/
static mt_current_select_infor dest_mt_carrier_info ;

int balong_mt_carrier_set_tracemask(unsigned int trace_flag)
{
    mt_carrier_trace_mask =trace_flag;
    return 0;
}
#if 0
/**
* 作用:从指定的路径下面读文件到指定的内存中且返回读的数据大小
*
* 参数:
* @d_file_path                  ---要读的文件名
* @d_file_buf                ---要读到的内存地址
* @file_size                ---存放读的数据长度
* 返回值:
* @数据得到成功就返回TRUE,错误就返回FALSE
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
     /*检查一下文件是否存在*/
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
    /*从文件中读数据*/
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
* 作用:保存内存中的数据到文件中去
*
* 参数:
* @d_file_path               ---要保存的文件名
* @d_file_buf                ---要保存数据在内存中的地址
* @file_size                ---要保存的数据大小
* 返回值:
* 保存成功就返回TRUE,否则就返回FALSE
*/
static bool balong_mt_carrier_save_file_data( char* d_file_path, char* d_file_buf,unsigned int d_file_size)
{
    int fd = -1;
    struct rfile_stat_stru  efs_buf;
    if ((NULL == d_file_path) || (NULL == d_file_buf))
    {
        return FALSE;
    }
    /*检查一下文件是否存在*/
    if( 0 != bsp_stat(d_file_path, &efs_buf))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"warning balong_mt_carrier_save_file_data: is not exit %s\n",d_file_path);
    }

    /*打开文件,以trunk的方式打开文件*/
    fd = bsp_open(d_file_path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (-1 == fd)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_save_file_data:open error\n");
        return FALSE;
    }
    /*写数据*/
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
* 作用:保存运行和默认的PLMN信息到NV文件中去
*
* 参数:
* @cur_plmninfo               ---要保存的数据地址
* @plmn_nv_size               ---要保存的数据大小
* 返回值:
* 保存当前的PLMN的NV成功就返回TRUE,否则就返回FALSE
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
    /*内容写到文件中去*/
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
* 作用:挂载多镜像的加载分区
*
* 参数:
* 无
* 返回值:
* 多镜像分区加载成功就返回为0，不成功就返回一个负值
*/
int balong_mt_carrier_partition_mount(void)
{
    struct mtd_info *mtd;
    int rt = 0;
    char mount_name[32] ={0};

#if (FEATURE_ON == MBB_MULTI_CARRIER)
    /*online分区不需要单独挂载文件系统，直接返回0*/
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
* 作用:当前的PLMN和默认的PLMN是通过NV中读出来的，但只有名字信息，通过比较系统中所有支持的PLMN信息,来得到当前和默认的PLMN的全面信息,且通过参数返回当前plmn和默认的PLMN是否存在
*
* 参数:
* 无
* 返回值:
* 无
* 描述:遍历单板支持的所有PLMN,查找当前的PLMN和默认的PLMN，得到一个比较全面的PLMN信息存放在mt_carrier_current_info的数据结构中去
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
    /*遍历所有的支持的image*/
    for (i=0; i<mt_carrier_plmn_supported.mt_image_cnt; i++)
    {
        for(j=0; j<mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_cnt; j++)
        {
            for(k=0; k<mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_cnt; k++)
            {
                /*不会存在有两个PLMN是相同的*/
                if (0 == strcmp((char *)mt_carrier_cur_plmninfo_from_nv.cur_running_plmn,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_array[k]->plmn))
                {
                    /*保存当前的目的相关信息*/
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
                    /*保存当前的目的相关信息*/
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
                    /*保存通用运营商的相关信息*/
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
* 作用:plmn检查前的准备工作,如果是第一次执行此函数时要挂载multicarrier分区，解析配置文件到相关的数据结构中去
*
* 参数:
* 无
* 描述:首先把此多镜像加载分区挂载，然后来解析xml的配置文件，得到一个比较全面的PLMN信息存放在mt_carrier_current_info的数据结构中去
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
        /*首先挂载multicarrier分区的*/
        ret = balong_mt_carrier_partition_mount();
        if(ret != 0)
        {
            mt_carrier_trace(MT_TRACE_ALWAYS," balong_mt_carrier_partition_mount error\n");
            return FALSE;
        }
        /*解析系统支持的多镜像配置文件到相关的数据结构中去*/
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
* 作用:查询此PLMN系统是否支持，如果支持更新到dest_mt_carrier_info此数据结构中去
*
* 参数:
* @plmn            ---查询的PLMN
* 返回值:
* 如果支持此PLMN侧返回TRUE,不支持则返回FALSE
* 描述:遍历单板支持的所有PLMN,查找当前的PLMN系统是否支持
*/
static bool balong_mt_carrier_check_plmn_support(char *plmn)
{
    u32 i = 0;
    u32 j= 0;
    u32 k = 0;
    bool is_plmn_found = FALSE;
    /*如果plmn是为空的话*/
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
    /*遍历所有的支持的image*/
    for (i=0; i<mt_carrier_plmn_supported.mt_image_cnt; i++)
    {
        for(j=0; j<mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_cnt; j++)
        {
            for(k=0; k<mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_cnt; k++)
            {
                if (0 == strcmp((char *)plmn,(char *)mt_carrier_plmn_supported.mt_image_info_array[i]->mt_carrier_info_array[j]->mt_carrier_support_plmn_array[k]->plmn))
                {
                    /*保存当前的目的相关信息*/
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
* 作用:判断要切换的目的的镜像和xml是否存在
*
* 参数:
* @dest_mt_carrier_info            ---要切换到镜像和xml配置信息
* 返回值:
* 如果支持则返回TRUE,如果不支持则返回FALSE
* 描述:判断一下要切换的目的镜像和xml文件是否存在
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
    /*检查一下文件是否存在*/
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
    /*检查一下文件是否存在*/
    if( 0 != bsp_stat(temp_mtc_image_path, &efs_buf))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_check_image_xml_is_exit DSP is not exit\n");
        return FALSE;
    }
    (void)snprintf(temp_user_xml_save_path,sizeof(temp_user_xml_save_path),"/online/mtc/image%d/NV/Carrier/Carrier%d/cust.xml",dest_mt_carrier_info.mt_image_index, dest_mt_carrier_info.mt_carrier_index);
#else 
    (void)snprintf(temp_user_xml_save_path,sizeof(temp_user_xml_save_path),"/mtc/image%d/NV/Carrier/Carrier%d",dest_mt_carrier_info.mt_image_index, dest_mt_carrier_info.mt_carrier_index);
#endif /* MBB_MULTI_CARRIER */

    /*检查一下文件是否存在*/
    if( 0 != bsp_stat(temp_user_xml_save_path, &efs_buf))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_check_image_xml_is_exit Carrier is not exit\n");
        return FALSE;
    }
    mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_check_image_xml_is_exit complete\n");
    return TRUE;
}

/**
* 作用:多镜像切换的预处理函数，如果支持切换的话就唤醒切换任务执行多镜像切换
*
* 参数:
* @plmn            ---要切换到plmn
* 返回值:
* 如果支持则返回TRUE,如果不支持则返回FALSE
*/
static bool balong_mt_carrier_switch_pre_op(char *plmn)
{
    bool cur_find,default_find;
    if (NULL == plmn)
    {
        return FALSE;
    }
    /*不能发生切换*/
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
    /*判断一下要切换的image和xml是否存在，如果不存在就直接通出*/
    if(!balong_mt_carrier_check_image_xml_is_exit(dest_mt_carrier_info))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"multi_carrier:write cur_ver_file fail\n");
        return FALSE;
    }
    /*要切换的与现在的是相同的话就不切换了*/
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
    /*发送切换的命令*/
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    mt_carrier_op = MTC_MANUAL_SWITCH;
#endif
    up(&mt_carrier_sem);
    mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_switch_pre_op complete\n");
    return TRUE;
}

/**
* 作用:把mtc_info相关的信息写到dest_buff_ptr中去,把PLMN的详细信息返回给AT的查询命令
*
* 参数:
* @mtc_info            ---要写的plmn数据结构
* @dest_buff_ptr       ---要写的数据地址
* @length              ---返回写的数据长度
* 返回值:
* 返回TRUE
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
    /*打印plmn的信息*/
    buffer_size += snprintf( (char*)( buf_temp + buffer_size ), MTC_BUF_MAX_LENGTH - buffer_size,
                                  "%s",mtc_info->mt_carrier_plmn_info.plmn);
    *(buf_temp +buffer_size)='"';
    buffer_size++;
    *(buf_temp +buffer_size)=',';
    buffer_size++;
    /*打印plmn的短名字信息*/
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
    /*把short_name写完成了*/
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
* 作用:把系统所有支持的PLMN信息都发送出去
*
* 参数:
* @dest_buff_ptr            ---存放PLMN信息的地址
* @length                   ---返回总共写的字符数目
* 返回值:
* 成功则返回TRUE,失败返回FALSE
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
    /*遍历系统所有支持的PLMN*/
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
* 作用:查询系统的PLMN信息
*
* 参数:
* @dest_buff_ptr            ---存放PLMN信息的地址
* @length                   ---返回总共写的字符数目
* @MTC_QUERY_TYPE           ---查询的类型
* 返回值:
* 成功则返回TRUE,失败返回FALSE
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
    /*这里再次来解析一次的原因是可能前一次修改的默认的PLMN信息，需要更新mt_carrier_current_info变量中的数据结构*/
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
* 作用:多镜像切换的命令
* 参数:
* @plmn                     ---要切换的PLMN
* 返回值:
* 成功则返回TRUE,失败返回FALSE
*/
bool balong_basic_exec_mtcarrier_cmd(char *plmn)
{
    /*输入参数检查*/
    if(plmn==NULL)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"plmn is NULL\n");
        return FALSE;
    }
    /*如果上一个切换还没有完成*/
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
* 作用:保存当前的默认PLMN信息到NV文件中去
* 参数:
* @plmn                     ---要保存的默认PLMN
* 返回值:
* 成功则返回TRUE,失败返回FALSE
*/
static bool balong_mt_carrier_save_mtcdft(char *plmn)
{
    bool ret=FALSE;
    if (NULL == plmn)
    {
        return FALSE;
    }
    /*查看此plmn是否支持，如果不支持的话不能保存*/
    if (!balong_mt_carrier_check_plmn_support((char *)plmn))
    {
        return FALSE;
    }
    /*判断一下要切换的image和xml是否存在，如果不存在就直接通出*/
    if(!balong_mt_carrier_check_image_xml_is_exit(dest_mt_carrier_info))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_save_mtcdft:image_xml is not exit\n");
        return FALSE;
    }
    if (0 == strcmp((char *)plmn,(char *)mt_carrier_current_info.defalt_info.mt_carrier_plmn_info.plmn))
    {
        return TRUE;
    }
    /*把默认的plmn保存在文件中*/
    memcpy((char *)mt_carrier_cur_plmninfo_from_nv.default_plmn,(char *)plmn,MTC_PLMN_MAX);
    ret= balong_mt_carrier_save_current_plmn_nv((char *)&mt_carrier_cur_plmninfo_from_nv,sizeof (mt_carrier_cur_plmninfo_from_nv));
    return ret;

}
/**
* 作用:默认的PLMN操作实现
* 参数:
* @plmn                     ---要操作的默认PLMN
* @dft_option               ---操作的默认PLMN的操作类型
* 返回值:
* 成功则返回TRUE,失败返回FALSE
*/
bool balong_basic_exec_mtcdft_cmd(char *plmn,MTC_DFT_OPTION dft_option)
{
    bool ret=FALSE;
    /*如果只是设置default的话*/
    if (MTC_DEFAULT_SET == dft_option)
    {
        ret=balong_mt_carrier_save_mtcdft(plmn);
    }
    /*运行default的话*/
    else if(MTC_DEFAULT_RUN == dft_option)
    {
        /*如果传入的默认PLMN为空的话*/
        if(NULL == plmn)
        {
            ret= balong_mt_carrier_switch_pre_op((char *)mt_carrier_current_info.defalt_info.mt_carrier_plmn_info.plmn);
        }
        else
        {
            /*这里的只是保存一下默认的PLMN信息*/
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
* 作用:保存的mtc_updata_status文件中的数据
* 参数:
* @dest_mt_carrier_info                     ---要保存的目的镜像信息
* 返回值:
* 成功则返回TRUE,失败返回FALSE
*/
static bool balong_mt_carrier_save_updata_status_file(mt_current_select_infor dest_mt_carrier_info)
{
    bool ret=FALSE;
    mt_carrier_updata_stutus carrier_updata_stutus;
    memset((char *)&carrier_updata_stutus,'\0',sizeof(carrier_updata_stutus));

    memcpy((char *)carrier_updata_stutus.mt_cur_running_plmn,(char *)mt_carrier_current_info.carrier_info.mt_carrier_plmn_info.plmn,MTC_PLMN_MAX);
    memcpy((char *)carrier_updata_stutus.mt_dest_to_plmn,(char *)dest_mt_carrier_info.mt_carrier_plmn_info.plmn,MTC_PLMN_MAX);
    /*如果两个的镜像不相同的话就要设置下面的标记*/
    if(mt_carrier_current_info.carrier_info.mt_image_index != dest_mt_carrier_info.mt_image_index)
    {
#if (FEATURE_ON == MBB_MULTI_CARRIER)
#else
        snprintf(carrier_updata_stutus.mt_dest_image_path,MTC_DEST_IMAGE_PATH,"/mtc/image%d",dest_mt_carrier_info.mt_image_index);
#endif /* MBB_MULTI_CARRIER */
        carrier_updata_stutus.mt_image_updata_status =1;
    }
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    /* 填充用户设定或sim卡触发的运营商镜像和XML信息 */
    snprintf(carrier_updata_stutus.mt_dest_image_path, MTC_DEST_IMAGE_PATH, "/online/mtc/image%d", dest_mt_carrier_info.mt_image_index);
    snprintf(carrier_updata_stutus.mt_dest_carrier_xml_path, MTC_DEST_IMAGE_PATH, "/online/mtc/image%d/NV/Carrier/Carrier%d", dest_mt_carrier_info.mt_image_index, dest_mt_carrier_info.mt_carrier_index);
    /* 填充默认运营商镜像和XML信息 */
    memcpy((char*)carrier_updata_stutus.mt_default_plmn, (char*)mt_carrier_current_info.defalt_info.mt_carrier_plmn_info.plmn, MTC_PLMN_MAX);
    snprintf(carrier_updata_stutus.mt_default_image_path, MTC_DEST_IMAGE_PATH, "/online/mtc/image%d", mt_carrier_current_info.defalt_info.mt_image_index);
    snprintf(carrier_updata_stutus.mt_default_carrier_xml_path, MTC_DEST_IMAGE_PATH, "/online/mtc/image%d/NV/Carrier/Carrier%d",
             mt_carrier_current_info.defalt_info.mt_image_index, mt_carrier_current_info.defalt_info.mt_carrier_index);
    /* 填充通用运营商镜像和XML信息 */
    memcpy((char*)carrier_updata_stutus.mt_general_plmn, (char*)mt_carrier_current_info.general_info.mt_carrier_plmn_info.plmn, MTC_PLMN_MAX);
    snprintf(carrier_updata_stutus.mt_general_image_path, MTC_DEST_IMAGE_PATH, "/online/mtc/image%d", mt_carrier_current_info.general_info.mt_image_index);
    snprintf(carrier_updata_stutus.mt_general_carrier_xml_path, MTC_DEST_IMAGE_PATH, "/online/mtc/image%d/NV/Carrier/Carrier%d",
             mt_carrier_current_info.general_info.mt_image_index, mt_carrier_current_info.general_info.mt_carrier_index);
    /* 默认切换到用户设定或sim卡触发的运营商 */
    carrier_updata_stutus.mt_dest_carrier_type = MTC_DEST_CARRIER_SET;
#else
    snprintf(carrier_updata_stutus.mt_dest_carrier_xml_path,MTC_DEST_IMAGE_PATH,"/mtc/image%d/NV/Carrier/Carrier%d",dest_mt_carrier_info.mt_image_index,dest_mt_carrier_info.mt_carrier_index);
#endif /* MBB_MULTI_CARRIER */
    carrier_updata_stutus.mt_carrier_xml_updata_status =1;
    /*内容写到文件中去*/
    ret = balong_mt_carrier_save_file_data(BALONG_MT_CARRIER_UPDATA_STATUS_FILE,(char *)&carrier_updata_stutus,sizeof(carrier_updata_stutus));
    return ret;
}

/**
* 作用:切换命令的具体执行函数
* 参数:
* @无
* 返回值:
* 无
*/
static void balong_mt_carrier_switch_carrier_fn(void)
{
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    u32 result = 0;
    MTCSWT_AUTO_MANUL_STATUS_STRU  mtcswt_status = {0};
#else
    /*保存要备分的NV文件的路径名*/
    char temp_user_xml_save_path[MTC_FILE_PATH_MAX] = {0};
    (void)snprintf(temp_user_xml_save_path,sizeof(temp_user_xml_save_path),"/mtc/image%d/NV/Carrier/Carrier%d",mt_carrier_current_info.carrier_info.mt_image_index,
                                                                                                       mt_carrier_current_info.carrier_info.mt_carrier_index);
    /*保存当前运行的NV配置信息*/
    if(!bsp_nvm_mtcarrier_backup(temp_user_xml_save_path))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_save_xml fail\n");
        return ;
    }
#endif /* MBB_MULTI_CARRIER */
    /*把要切换到的镜像保存在文件中去*/
    if(!balong_mt_carrier_save_updata_status_file(dest_mt_carrier_info))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_save_updata_status_file fail\n");
        return ;
    }
    /*更新要运行的plmn到NV的配置文件中去*/
    memcpy((char *)&mt_carrier_cur_plmninfo_from_nv.cur_running_plmn,(char *)&dest_mt_carrier_info.mt_carrier_plmn_info.plmn,MTC_PLMN_MAX);
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    /* 切换完成后再更新OEMINFO分区此处不再操作 */
#else
    if (!balong_mt_carrier_save_current_plmn_nv((char *)&mt_carrier_cur_plmninfo_from_nv,sizeof (mt_carrier_cur_plmninfo_from_nv)))
    {
        return ;
    }
#endif /* MBB_MULTI_CARRIER */
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    /*若为手动切换，则设置SIM卡自动切换标记为0*/
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

    /* 备份NV，以便切换镜像/运营商之后能够将NV恢复回来 */
    result = bsp_nvm_backup();
    if (NV_OK != result)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"Fail to backup Multi-carrier NV!\n");
        return;
    }
    /*重启单板进行数据搬移*/	
    balong_mt_carrier_restart();
#else
    /*表示切换完成标记*/
    is_mt_carrier_switch_finish = TRUE;
#endif /* MBB_MULTI_CARRIER */

}

void balong_mt_carrier_restart(void)
{
    mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_restart\n");
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    mdrv_dload_set_curmode(DLOAD_MODE_DOWNLOAD);
    /* 设置共享内存魔术字，下次进入下载模式 */
    mdrv_dload_set_softload(true);
    mdrv_dload_normal_reboot();
#endif /* MBB_MULTI_CARRIER */
}
/**
* 作用:在多镜像加载升级时，检查一下"mtc/mtc_updata_status"这个文件是否存在，确保此文件是一定存在,并且把此文件中的内容读到updata_status_addr中去
*
* 参数:
* @updata_status_addr                     ---把文件中的数据读到内存中的地址
* 返回值:
* 成功则返回TRUE,失败返回FALSE
*/
bool balong_mt_carrier_read_datastatus(void * updata_status_addr)
{
    int fd = -1;

    struct rfile_stat_stru fd_stat = {0};
    if(updata_status_addr == NULL)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"balong_mt_carrier_updata_status: updata_status_addr is null\n");
    }
    /*检查一下文件是否存在*/
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
    /*如果为0的话也认为是不存在的，表示有错误了*/
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
* 作用:多镜像加载模块的切换任务,为什么要用一个任务来执行切换操作，因为担心挂载多镜像分区、解析配置文件、保存NV的时间有一点长，AT命令的执行时间有一点长
*
* 参数:
* 无
* 返回值:
* 无
*/
static int balong_mt_carrier_task(void *_c)
{
    /*要求此任务的优先级比较高,唤醒了就执行*/
    set_user_nice(current, 10);
    for (;;)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,": multcarrier stask sleeping...\n");
        /*一直等待直到获得信号量*/
        /*lint -save -e722*/
        while(down_interruptible( &mt_carrier_sem));
        /*lint -restore*/
        /*切换前延迟系统 休眠，防止模块休眠切换失效，时间2s*/
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
* 作用:多镜像加载模块的切换任务的初始化函数，初始化同步任务的信号量和创建切换任务
*
* 参数:
* 无
* 返回值:
* 无
*/
static int balong_mt_carrier_task_init(void)
{
    /*信号量初始化函数*/
    sema_init(&mt_carrier_sem, 0);
    /*第一次的时候创建多镜像的切换任务*/
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
* 作用:在buf_start和buf_end之前查找第一个出现find_str字符串的地址，如果没有查找到的话返回NULL
*
* 参数:
* @buf_start             ---要查找数据的起始地址
* @buf_end               ---要查找数据的结束地址
* @find_str              ---要查找的字符串
* 返回值:
* 如果查找到指定格式的字符串就返回字符的地址，如果没有查找到就返回NULL
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
* 作用:从buffer中按照特定的格式解析数据存放在mt_carrier_plmn_info的数据结构中去,且返回有多少个plmn的数目
*
* 参数:
* @buffer_start             ---buffer数据的起始地址
* @buffer_end               ---buffer数据的结束地址,解析的数据一定在此范围内
* @support_plmn_array       ---把解析完成的数据以support_plmn_array的数据结构存放在此变量中
* @support_plmn_cnt         ---返回plmn的数目
* 返回值:
* 此xml文件中的数据格式一定要严格按照要求来组织，解析正确返回TRUE,解析错误返回FALSE
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
* 作用:从buffer中按照特定的格式解析数据存放在my_carrier_info的数据结构中去,且返回有多少个carrier的数目
*
* 参数:
* @buffer_start             ---buffer数据的起始地址
* @buffer_end               ---buffer数据的结束地址,解析的数据一定在此范围内
* @my_carrier_info            ---把解析完成的数据以my_carrier_info的数据结构存放在此变量中
* @carrier_cnt                ---返回carrier的数目
* 返回值:
* 此xml文件中的数据格式一定要严格按照要求来组织，解析正确返回TRUE,解析错误返回FALSE
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
* 作用:从buffer中按照特定的格式解析数据存放在my_image_info的数据结构中去,且返回有多少个image的数目
*
* 参数:
* @buffer_start             ---buffer数据的起始地址
* @buffer_end               ---buffer数据的结束地址,解析的数据一定在此范围内
* @my_image_info            ---把解析完成的数据以mt_image_info的数据结构存放在此变量中
* @image_cnt                ---返回image的数目
* 返回值:
* 此xml文件中的数据格式一定要严格按照要求来组织，解析正确返回TRUE,解析错误返回FALSE
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
* 作用:打印系统所有支持的PLMN信息
*
* 参数:
* @mt_info_my             ---mt_info_my系统支持的所有的PLMN的数据结构
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
* 作用:从buffer中按照特定的格式解析数据存放在mt_info的数据结构中去
*
* 参数:
* @buffer            ---buffer数据的起始地址
* @end               ---buffer数据的结束地址
* @mt_info_my        ---把解析完成的数据存放在此数据结构中去
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
* 作用:读Image_Xml_Config.xml文件并且解析到相关的数据结构中去
*
* 参数:
* 无
*/
static bool balong_mt_carrier_read_xml_file(void)
{
    int fd = -1;
    struct rfile_stat_stru  fd_stat;
    char *data_start=NULL;
    char *data_end =NULL;

    /*检查一下文件是否存在*/
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
    /*从文件中读数据*/
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
* 作用: 多镜像模块的初始化函数，首先读当前的PLMN和默认的PLMN的NV配置文件，如果此NV不存在的话就返回失败,成功读完NV后再来
* 创建多镜像切换任务
*
* 参数:
* 无
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
    /*初始化系统 锁*/
    wake_lock_init(&multi_carrier_lock, WAKE_LOCK_SUSPEND, MTC_DRIVER_NAME);

    /*注册icc回调函数，用来sim卡切换*/	
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
 函 数 名  : huawei_mtc_get_plmn_from_oem
 功能描述  : 从 OMEINFO中获取plmn
 输入参数  : 
 输出参数  : 无
 返 回 值  : TRUE:获取成功
             FALSE:获取失败
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
 函 数 名  : huawei_mtc_set_plmn_from_oem
 功能描述  : 向 OMEINFO中写入plmn
 输入参数  : 
 输出参数  : 无
 返 回 值  : TRUE:更新成功
             FALSE:更新失败
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
 函 数 名  : huawei_mtc_get_version_entry
 功能描述  : 获取MTC相关版本号接口
 输入参数  : char *version  用来存储版本号的地址,size:输入数据区的大小
 输出参数  : 无
 返 回 值  : 无
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
 函 数 名  : huawei_mtc_get_package_version
 功能描述  : 获取整包版本号接口
 输入参数  : char *version  用来存储版本号的地址,size:输入数据区的大小
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void huawei_mtc_get_package_version(char *version, unsigned char size)
{
    bool res = FALSE;
    char *ver_ptr = NULL;	
    if (( NULL == version) || ( 0 == size ))
    {
        return;
    }
    /*解析Image_Xml_Config.xml，获得package_ver*/
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
 函 数 名  : huawei_mtc_get_oem_version
 功能描述  : 获取OEM版本号接口
 输入参数  : char *version  用来存储版本号的地址,size:数据空间大小
 输出参数  : 无
 返 回 值  : 无
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
    /* 读取产品名称信息*/
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
 函 数 名  : huawei_mtc_get_image_version
 功能描述  : 获取IMAGE版本号接口
 输入参数  : char *version  用来存储版本号的地址,size:数据空间大小
 输出参数  : 无
 返 回 值  : 无
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
 函 数 名  : huawei_mtc_get_feature_version
 功能描述  : 获取MTC的feature版本号接口
 输入参数  : char *version  用来存储版本号的地址,size:数据空间大小
 输出参数  : 无
 返 回 值  : 无
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
 函 数 名  : huawei_mtc_check_sim_plmn
 功能描述  : 查找sim_plmn
 输入参数  : char *data_start  当前carrier_plmn数据范围的开始地址
                            char * data_end  当前carrier_plmn数据范围的结束地址
                            char *sim_plmn  从sim卡中获得的plmn
 输出参数  : 无
 返 回 值  : TRUE:查找成功
                        FALSE:查找失败
 调用函数  :
 被调函数  :
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
 函 数 名  : huawei_mtc_plmn_list_parse
 功能描述  : 解析plmn_list.xml
 输入参数  : char *data_start  xml的开始地址
                            char * data_end  xml的结束地址
                            char *sim_plmn  从sim卡中获得的plmn
                            char * carrier_plmn 从plmn list中查询到的对应运营商代表plmn
 输出参数  : 无
 返 回 值  : TRUE:切换成功
                        FALSE:切换失败
 调用函数  :
 被调函数  :
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
    /*先将carrier_plmn设置成通用，再进行查找*/
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
 函 数 名  : huawei_mtc_get_carrier_plmn
 功能描述  : 检查当前插入的sim卡的plmn对应的运营商plmn
 输入参数  : char *sim_plmn  从sim卡中获得的plmn
                            char * carrier_plmn 从plmn list中查询到的对应运营商代表plmn
 输出参数  : 无
 返 回 值  : TRUE:切换成功
                        FALSE:切换失败
 调用函数  :
 被调函数  :
*****************************************************************************/
bool huawei_mtc_get_carrier_plmn(char *sim_plmn,char * carrier_plmn)
{
    int fd = -1;
    struct kstat  fd_stat;
    char *data_start=NULL;
    char *data_end=NULL;
    bool parse_res = FALSE;
    /*检查一下文件是否存在*/
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
    /*从文件中读数据*/
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
    /*解析xml文件，查找sim_plmn对应的carrier_plmn*/
    parse_res = huawei_mtc_plmn_list_parse(data_start,data_end,sim_plmn,carrier_plmn);
    kfree(data_start);
    data_start = NULL;
    return parse_res;
}
/*****************************************************************************
 函 数 名  : huawei_mtc_check_auto_switch_valid
 功能描述  : 检查当前是否允许进行SIM卡自动切换 
 输入参数  : char *plmn  从sim卡中获得的plmn
 输出参数  : 无
 返 回 值  : TRUE:切换成功
                        FALSE:切换失败
 调用函数  :
 被调函数  :
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
 函 数 名  : huawei_mtc_get_plmn
 功能描述  : 从C核的hplmn数据结构中提取plmn
 输入参数  : stHplmn  从C核得到的hplmn数据结构
                            sim_plmn 组装好的plmn
 输出参数  : 无
 返 回 值  : TRUE:切换成功
                        FALSE:切换失败
 调用函数  :
 被调函数  :
*****************************************************************************/
void huawei_mtc_get_plmn(MTC_HPLMN_WITH_MNC_LEN_STRU *stHplmn, char *sim_plmn)
{
    char plmn_temp[MTC_PLMN_MAX] = {0};
    char basic_zero = '0';
    plmn_temp[0] = (0x0f & stHplmn->stHplmn.Mcc) + basic_zero;
    plmn_temp[1] = ((0x0f00 & stHplmn->stHplmn.Mcc) >> 8) + basic_zero;
    plmn_temp[2] = ((0x0f0000 & stHplmn->stHplmn.Mcc) >> 16) + basic_zero;
    /*2位mnc*/	
    if (2 == stHplmn->ucHplmnMncLen)
    {
        plmn_temp[3] = (0x0f & stHplmn->stHplmn.Mnc) + basic_zero;
        plmn_temp[4] = ((0x0f00 & stHplmn->stHplmn.Mnc) >> 8) + basic_zero;
    }
    /*3位mnc*/	
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
 函 数 名  : huawei_mtc_auto_switch_entry
 功能描述  : SIM卡自动切换总入口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :
*****************************************************************************/
void huawei_mtc_auto_switch_entry(void)
{
    char carrier_plmn[MTC_PLMN_MAX] = {0};
    bool ulRet = FALSE;	
    int times = 10;	//等待10次
    struct kstat  fd_stat;
	
    /*检查online分区mtc文件夹是否存在*/
    while ((0 != bsp_stat("/online/mtc/",&fd_stat)) && ( 0 != times ))
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"/online/mtc is not exist wait for 1s");
        msleep(1000);
        times --;		
    }
    /*检查bodysar的plmn-list是否包含插入的sim卡plmn,如果是，同意打开bodysar*/
#if (FEATURE_ON ==  MBB_FEATURE_BODYSAR)
    {
        extern void bodysar_powr_limit_by_plmnlist_check(char *sim_plmn);
        bodysar_powr_limit_by_plmnlist_check(g_sim_plmn);
    }
#endif
    /*判断sim卡自动切换功能是否生效，是否做过手动切换*/
    ulRet = huawei_mtc_check_auto_switch_valid();
    if (!ulRet)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_auto_switch_entry auto switch is invalid");
        return ;
    }
	
    /*根据传入的sim卡plmn，查找对应运营商的标准plmn*/
    ulRet = huawei_mtc_get_carrier_plmn(g_sim_plmn,carrier_plmn);
    if (!ulRet)
    {
        mt_carrier_trace(MT_TRACE_ALWAYS,"huawei_mtc_auto_switch_entry get carrier plmn error");
        return ;
    }
    /*调用海思镜像切换函数*/
    (void)balong_basic_exec_mtcarrier_cmd(carrier_plmn);

}
/*****************************************************************************
 函 数 名  : huawei_mtc_sim_switch_cb
 功能描述  : SIM卡初始化后向A核发icc消息的回调函数
 输入参数  : NULL
 输出参数  : 无
 返 回 值  : 0:成功  -1:失败
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
    /*发送切换的命令*/
    mt_carrier_op = MTC_SIM_SWITCH;
    up(&mt_carrier_sem);
    return 0;
}
/*****************************************************************************
 函 数 名  : huawei_mtc_free_mem
 功能描述  : 用来释放申请的内存
 输入参数  : 无
 输出参数  : 无
 返 回 值  :无
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

/*此模块的使用的过程中不会动态卸载*/
module_init(balong_mt_carrier_init);
