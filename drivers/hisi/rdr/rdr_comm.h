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

#ifndef __RDR_COMM_H__
#define __RDR_COMM_H__

#include <product_config.h>
#include <osl_types.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/hisi/rdr_pub.h>


#define PATH_ROOT           "/modem_log/dump/"

#define RDR_HISTORY_LOG_PATH        "/modem_log/dump/history.log"
#define RDR_HISTORY_LOG_MAXSIZE     (4*1024)

#define RDR_EXC_DUMP_PATH           "/modem_log/dump/"
#define RDR_BIN_NAME                "rdr.bin"

#define RDR_EXC_FILE_MAX_NUM        3
#define RDR_EXC_FILE_NAME_LENGTH    64


#define RDR_WAIT_DONE(a,time)\
    do{\
        while(a)\
        {\
    		current->state = TASK_INTERRUPTIBLE;\
    		(void)schedule_timeout(HZ/10);\
    		if((time--)<= 0){\
    			break;}\
        }\
    }while(0);

#ifndef UNUSED
#define UNUSED(a)       (a=a)
#endif

s32 rdr_wait_partition(char *path, int timeout);
void rdr_remove_dir(char * dir_name);
s32  rdr_create_exc_dir(char* dir_name,char* dstpath,u32 dstsize,char* date);
s32 rdr_save_file(char *filename, void * data, u32 length, u32 max_size,bool append);
void rdr_get_date_timestamp(char* date,u32 len);


#endif /*__RDR_COMM_H__*/


