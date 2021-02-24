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

#include <product_config.h>
#include <osl_types.h>
#include <osl_list.h>
#include <osl_sem.h>
#include <osl_malloc.h>

#include <linux/hisi/rdr_pub.h>
#include <linux/rtc.h>
#include <linux/stat.h>
#include <linux/dirent.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include "drv_nv_id.h"
#include "drv_nv_def.h"
#include "bsp_rfile.h"
#include "bsp_slice.h"
#include "bsp_nvim.h"
#include "rdr_exc.h"
#include "rdr_debug.h"
#include "rdr_area.h"
#include "rdr_comm.h"



/*
 * func name: rdr_wait_partition
 * .
 * func args:
 *  char*  path,			path of watit file.
 *  u32 timeouts,       time out.
 * return
 *	<0 fail
 *	0  success
 */
s32 rdr_wait_partition(char *path, s32 timeout)
{
    s32 time = timeout;

    path = path;

    RDR_WAIT_DONE(bsp_fs_ok(), time);

    if(time <= 0)
    {
        rdr_err("wait file system OK time out 0x%x\n",timeout);
    }
	return RDR_OK;
}
EXPORT_SYMBOL(rdr_wait_partition);
/*
 * func name: rdr_get_datetime
 * .
 * func args:
 * char * date
 * return
 */
void rdr_get_date_timestamp(char* date,u32 len)
{
    struct timex txc;
    struct rtc_time tm;
    /*fill date&tick*/
    do_gettimeofday(&(txc.time));
    rtc_time_to_tm(txc.time.tv_sec, &tm);
    snprintf(date,len,"%4d%02d%02d%02d%02d%02d-%08x",tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,bsp_get_slice_value());
    return;
}
EXPORT_SYMBOL(rdr_get_date_timestamp);
/*
 * func name: rdr_remove_dir
 * .
 * func args:
 * char * dir_name
 * return
 *	<0 fail
 *	0  success
 */
void rdr_remove_dir(char * dir_name)
{
    char *buf = NULL;
    int i;
    int read_bytes;
    struct linux_dirent64 *dir;
    struct rfile_stat_stru s_stat;
    int fd;
    char temp[128];

    buf = (char*)kmalloc(1024, GFP_KERNEL);
    if(NULL == buf)
    {
        rdr_err("alloc mem error\n");
        return;
    }

    rdr_err("%s \n",dir_name);

    fd = bsp_opendir(dir_name);
    if(fd < 0)
    {
        rdr_err("open dir %s fail\n", dir_name);
        goto out;
    }

    read_bytes = bsp_readdir(fd, (struct linux_dirent64 *)buf, 1024);
    if(read_bytes < 0)
    {
        bsp_closedir(fd);
        goto out;
    }
    bsp_closedir(fd);

    for(i=0; i<read_bytes; )
    {
        dir = (struct linux_dirent64 *)((uintptr_t)buf + i);
        i += (int)(dir->d_reclen);
        snprintf(temp, sizeof(temp), "%s/%s", dir_name, dir->d_name);
        if(0 == strncmp(dir->d_name, ".", strlen(dir->d_name)) || 0 == strncmp(dir->d_name, "..", strlen(dir->d_name)))
        {
            continue;
        }

        if(RDR_DEBUG_SW(RDR_DEBUG_SAVE_HISTORY_SW)||RDR_DEBUG_SW(RDR_DEBUG_SAVE_RDR_SW))
            continue;

        memset(&s_stat,0,sizeof(s_stat));
        if(bsp_stat(temp, &s_stat))
        {
            rdr_err("get file stat err!\n");
            continue;
        }

        if(S_ISDIR(s_stat.mode))/*如果该源文件是目录*/
        {
            rdr_remove_dir(temp);
        }
        else
        {
            (void)bsp_remove(temp);
        }
    }

    if(RDR_DEBUG_SW(RDR_DEBUG_SAVE_HISTORY_SW)||RDR_DEBUG_SW(RDR_DEBUG_SAVE_RDR_SW))
        goto out;

    if(bsp_rmdir(dir_name) < 0)
    {
        rdr_err("del dir %s fail\n", dir_name);
    }
out:
    kfree(buf);
}
EXPORT_SYMBOL(rdr_remove_dir);
/*
 * func name: rdr_create_exc_dir
 * .
 * func args:
 *  char*  dir_name, create cur dir in dir of pathname.
 *  char* dstpath, return dest path
 * return
 *	<0 fail
 *	0  success
 */
s32  rdr_create_exc_dir(char* dir_name,char* dstpath,u32 dstsize,char* date)
{
    int i;
    int j=0;
    int read_bytes;
    int fd;
    char *buf = NULL;
    struct linux_dirent64 *dir;
    char file_name[RDR_EXC_FILE_MAX_NUM][RDR_EXC_FILE_NAME_LENGTH] = {{0}, {0}, {0}};
    char temp[RDR_EXC_FILE_NAME_LENGTH];
    DUMP_FILE_CFG_STRU dumpfile;

    /*读取异常文件配置*/
    if(bsp_nvm_read(NV_ID_DRV_DUMP_FILE, (u8 *)&dumpfile, sizeof(DUMP_FILE_CFG_STRU)))
    {
        dumpfile.file_cnt = RDR_EXC_FILE_MAX_NUM;
    }

    if(bsp_access(dir_name,0))
    {
        (void)bsp_mkdir(dir_name,0660);
    }

    fd = bsp_opendir(dir_name);
    if(fd < 0)
    {
        rdr_err("open  dir %s failed\n",dir_name);
        return RDR_ERROR;
    }
    buf = kmalloc(0x800, GFP_KERNEL);
    if(NULL == buf)
    {
        (void)bsp_closedir(fd);
        return RDR_ERROR;
    }
    read_bytes = bsp_readdir(fd, (struct linux_dirent64 *)buf, 0x800);
    if(read_bytes < 0)
    {
        rdr_err("read dir %s failed ,%d\n",dir_name,read_bytes);
        (void)bsp_closedir(fd);
        kfree(buf);
        return RDR_ERROR;
    }
    (void)bsp_closedir(fd);

    for(i=0; i<read_bytes; )
    {
        dir = (struct linux_dirent64 *)(buf + i);
        i += (int)dir->d_reclen;

        for(j=0; j<RDR_EXC_FILE_MAX_NUM; j++)
        {
            memset(temp, 0, sizeof(temp));
            snprintf(temp, sizeof(temp), "DUMP_%02d", j);
            if(NULL != strstr((char *)dir->d_name, temp))
            {
                snprintf(temp, sizeof(temp), "%s%s", dir_name, dir->d_name);
                /* 超过配置的最大存储份数, 删除目录  删除重复序号目录 */
                if((j >= (int)dumpfile.file_cnt - 1) || (j >= RDR_EXC_FILE_MAX_NUM - 1))
                {
                    rdr_err("delete oldest file dir %s\n",temp);
                    rdr_remove_dir(temp);
                }
                else
                {
                    strncpy(file_name[j], temp, RDR_EXC_FILE_NAME_LENGTH - 1);
                }
            }
            else if(NULL != strstr((char *)dir->d_name, "cp_log"))
            {
                memset(temp, 0, sizeof(temp));
                snprintf(temp, sizeof(temp), "%s%s", dir_name, "cp_log");
                rdr_err("delete %s \n",temp);
                rdr_remove_dir(temp);
            }
        }
    }

    /* 根据配置的最大存储份数，重命名异常目录 */
    j = (dumpfile.file_cnt > RDR_EXC_FILE_MAX_NUM) ? RDR_EXC_FILE_MAX_NUM : dumpfile.file_cnt;
    for(j=j-1; j>0; j--)
    {
        memset(temp, 0, sizeof(temp));
        if(file_name[j-1][0])
        {
            strncpy(temp, file_name[j-1], RDR_EXC_FILE_NAME_LENGTH - 1);
            temp[strlen(temp)-1] = j + '0';
            (void)bsp_rename(file_name[j-1], temp);
            rdr_err("Rename %s =>> %s\n",file_name[j-1],temp);
        }
    }
    /* 目录命名格式:日期_时间戳_DUMP_编号*/
    snprintf(dstpath, dstsize, "%s%s_DUMP_00", dir_name, date);
    kfree(buf);

    if(RDR_DEBUG_SW(RDR_DEBUG_CREATE_DIR_SW))
        return RDR_OK;

    /* 创建新目录，返回目录名 */
    (void)bsp_mkdir(dstpath,0660);
    strcat(dstpath,"/");

    return RDR_OK;
}
EXPORT_SYMBOL(rdr_create_exc_dir);
/*
 * func name: rdr_save_file
 * .
 * func args:
 *  char*  filename,			path of watit file.
 *  void * data,                data.
 *  u32 length,                 data length
 *  u32 max_size,               file max length
 *	bool append                 is append file
 *
 *  return
 *	0  success
 */
s32 rdr_save_file(char *filename, void * data, u32 length, u32 max_size,bool append)
{
    s32 ret;
    s32 len = 0;
    s32 fd;


    if(!bsp_access(filename,0))/*文件存在*/
    {
        fd = bsp_open(filename,RFILE_RDWR,0660);
        if(fd < 0)
        {
            rdr_err("open %s failed %d!\n",filename,fd);
            return RDR_ERROR;
        }
    }
    else
    {
        fd = bsp_open(filename, RFILE_RDWR|RFILE_CREAT, 0660);
        if(fd < 0)
        {
            rdr_err("create & open %s failed %d!\n",filename,fd);
            return RDR_ERROR;
        }
    }

    bsp_lseek(fd,0,SEEK_END);
    len = bsp_tell((u32)fd);
    bsp_lseek(fd,0,SEEK_SET);

    if((len+length)> max_size)
    {
        rdr_err("%s len = %d,length = %d,max len = %d\n",filename,len,length,max_size);
        bsp_close((u32)fd);
        bsp_remove(filename);


        fd = bsp_open(filename, RFILE_RDWR|RFILE_CREAT, 0660);
        if(fd < 0)
        {
            rdr_err("create & open %s failed %d!\n",filename,fd);
            return RDR_ERROR;
        }
    }
    if(append == true)/*增量保存*/
    {
        bsp_lseek(fd,0,SEEK_END);
    }

    if(RDR_DEBUG_SW(RDR_DEBUG_SAVE_HISTORY_SW)||RDR_DEBUG_SW(RDR_DEBUG_SAVE_RDR_SW))
        goto out;

    ret = bsp_write(fd,data,length);
    if(ret != length)
    {
        rdr_err("write %s failed,%d,%d\n",filename,ret,fd);
        bsp_close(fd);
        return RDR_ERROR;
    }
out:
    bsp_close(fd);
    rdr_err("save %s finished!\n",filename);
    return RDR_OK;
}
/*
 * func name: bbox_check_edition
 * .
 * func args:
 *  
 *
 *  return
 *	2  beta
 */
unsigned int bbox_check_edition(void)
{
    return EDITION_INTERNAL_BETA;
}

EXPORT_SYMBOL(rdr_save_file);

