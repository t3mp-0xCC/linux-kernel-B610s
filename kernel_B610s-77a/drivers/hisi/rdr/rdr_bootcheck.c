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


 /*
  * 该文件基本功能
  * 1、记录上次系统重启信息history.log;
  * 2、上次系统异常未保存完成的检测；
  *
  */


#include <product_config.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <linux/rtc.h>
#include "bsp_slice.h"
#include "rdr_exc.h"
#include "rdr_comm.h"
#include "rdr_area.h"
#include "rdr_bootcheck.h"
#include "rdr_debug.h"

struct rdr_bootcheck_ctrl_info_s g_rdr_bootcheck_ctrl;

s32 rdr_bootcheck_task(void* parm)
{
    struct rdr_struct_s* rdr_info = (struct rdr_struct_s*)g_rdr_bootcheck_ctrl.backup_addr;
    char  history[128];
    char  dstpath[128];
    char  date[24];

    while(rdr_wait_partition("/modem_log/lost+found/",1000)!= 0);

    memset(history,0,sizeof(history));
    memset(dstpath,0,sizeof(dstpath));
    memset(date,0,sizeof(date));

    if((NULL == rdr_info)||(rdr_info->top_head.magic != RDR_TOP_HEAD_MAGIC))
    {
        rdr_get_date_timestamp(date,sizeof(date));
        snprintf(history,sizeof(history),"[%s]: [Normal] System Power On [No backup ddr or Magicnum error]!\n",date);
        goto save_history;
    }

    if(rdr_info->base_info.start_flag == RDR_START_EXCH)/*进入异常标志*/
    {
        /*如果有异常标志，则将异常的时间记录下来*/
        memcpy(date,rdr_info->base_info.datetime,sizeof(date));
        snprintf(history,sizeof(history),"[%s]:[AbNormal] System Exception Reason[%s],Core[%s],",rdr_info->base_info.datetime,\
            rdr_get_exception_type(rdr_info->base_info.e_type),\
            rdr_get_core_name(rdr_info->base_info.e_core));

        /*只有上次未完成保存以及存在备份区空间时才进行再次备份保存*/
        if(rdr_info->base_info.savefile_flag != RDR_SAVE_FILE_END)
        {
            /*创建文件目录*/
            if(rdr_create_exc_dir(RDR_EXC_DUMP_PATH,dstpath,sizeof(dstpath),date))
            {
                rdr_err("create %s failed\n",RDR_EXC_DUMP_PATH);
            }
            strcat(dstpath,"/");
            strcat(dstpath,RDR_BIN_NAME);

            /*保存rdr.bin文件*/
            if(rdr_save_file(dstpath,(void*)rdr_info,MNTN_BASE_SIZE,RDR_ERROR,false))
            {
                rdr_err("save %s err\n",dstpath);
            }
            strcat(history,"[Last Not Save Done,Resave!!!]\n");
        }
        else
        {
            strcat(history,"[Last Save Done]\n");
        }
        /*处理完成之后清除异常标志*/
        memset(rdr_info,0,sizeof(struct rdr_struct_s));
    }
    else
    {
        rdr_get_date_timestamp(date,sizeof(date));
        if(rdr_info->base_info.reboot_flag == RDR_START_REBOOT)
        {
            snprintf(history,sizeof(history),"[%s]:[Normal] System Reboot [Soft Reboot]!\n",date);
        }
        else if(rdr_info->base_info.e_type < NORMAL_POWERUP_SHUTDOWN)
        {
            snprintf(history,sizeof(history),"[%s]:[Normal] System Power On [%s]\n",date,rdr_get_exception_type(rdr_info->base_info.e_type));
        }
        else if((rdr_info->base_info.e_type >= RESTART_REBOOT_START) && (rdr_info->base_info.e_type < RESTART_REBOOT_END))
        {
            snprintf(history,sizeof(history),"[%s]:[Normal] System Reboot [%s]\n",date,rdr_get_exception_type(rdr_info->base_info.e_type));
        }
        else if(rdr_info->base_info.e_type == PMU_EXCEPTION)
        {
            snprintf(history,sizeof(history),"[%s]:[AbNormal] System Reboot [%s]\n",date,rdr_get_exception_type(rdr_info->base_info.e_type));
        }
        else
        {
            snprintf(history,sizeof(history),"[%s]:[Undefined] Reboot or Power ON Reason!\n",date);
        }

    }

save_history:
    /*save history.log*/
    printk(KERN_ERR"%s\n",history);
    (void)rdr_save_file(RDR_HISTORY_LOG_PATH,history,strlen(history),RDR_HISTORY_LOG_MAXSIZE,true);

    return RDR_OK;

}


u32 rdr_bootcheck_init(void)
{
    u32 ret;
    struct rdr_area_mntn_addr_info_s area_info;

    memset(&area_info,0,sizeof(area_info));

    rdr_get_backup_mntn_addr_info(&area_info);

    g_rdr_bootcheck_ctrl.backup_addr = (struct rdr_struct_s*)area_info.vaddr;

    ret = (u32)osl_task_init("rdr_bootcheck",15,4096,(OSL_TASK_FUNC)rdr_bootcheck_task,NULL,(OSL_TASK_ID*)&g_rdr_bootcheck_ctrl.task_id);
    if(ret)
    {
        rdr_err("create bootcheck task err!\n");
        return RDR_ERROR;
    }
    rdr_err("ok!\n");
    return RDR_OK;
}
EXPORT_SYMBOL(rdr_bootcheck_init);

static int __init rdr_exc_info_parse(char *pucChar)
{
    u32  flag;

    flag = (uintptr_t)simple_strtoul("exc_type", NULL, 0);
    g_rdr_bootcheck_ctrl.exc_type= flag;


    flag = (uintptr_t)simple_strtoul("save_flag", NULL, 0);
    g_rdr_bootcheck_ctrl.save_flag= flag;

    flag = (uintptr_t)simple_strtoul("exc_flag", NULL, 0);
    g_rdr_bootcheck_ctrl.start_flag = flag;

    return 0;
}
early_param("exc_type", rdr_exc_info_parse);


