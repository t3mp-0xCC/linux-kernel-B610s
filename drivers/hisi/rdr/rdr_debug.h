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

#ifndef __RDR_DEBUG_H__
#define __RDR_DEBUG_H__

#include <product_config.h>
#include <osl_types.h>
#include <linux/hisi/rdr_pub.h>



#define RDR_DEBUG_TYPE(x)     {x,#x}
enum
{
   RDR_DEBUG_PRINT_SW,
   RDR_DEBUG_OPS_REG_SW,
   RDR_DEBUG_EXCEPTION_REG_SW,
   RDR_DEBUG_EXCEPTION_PROCESS_SW,
   RDR_DEBUG_SAVE_RDR_SW,
   RDR_DEBUG_SAVE_HISTORY_SW,
   RDR_DEBUG_CREATE_DIR_SW,
   RDR_DEBUG_RESET_SW,
   RDR_DEBUG_DUMP_SW,

   RDR_DEBUG_MAX_SW = 32
};

struct rdr_debug_sw_s
{
    u32 print_sw:1;
    u32 ops_reg_sw:1;
    u32 exc_reg_sw:1;
    u32 exc_proc_sw:1;
    u32 save_rdr_sw:1;
    u32 save_his_sw:1;
    u32 create_dir_sw:1;
    u32 reset_sw:1;
    u32 dump_sw:1;
    u32 reserved:24;
};

struct rdr_debug_ctrl_info_s
{
    u32  initState;
    union
    {
        u32 ulSw;
        struct rdr_debug_sw_s sw;
    };
    struct list_head help;
};

struct rdr_debug_help_info_s
{
    struct list_head help_list;
    char   desc[64];
    void (*help)(u32 arg);
    u32 arg;
};

struct rdr_debug_sw_info_s
{
    u32 sw_type;
	char name[64];
};


extern struct rdr_debug_ctrl_info_s g_rdr_debug_ctrl;

#define rdr_err(fmt,...)        printk(KERN_ERR"[rdr]: <%s> line = %d  "fmt, __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define rdr_fetal(fmt,...)      printk(KERN_ALERT"[rdr]: <%s> line = %d  "fmt, __FUNCTION__,__LINE__, ##__VA_ARGS__)

/*用于各个功能实现的开关，对应bit位为1时，该功能关闭，bit位为0，功能打开，默认为0*/
#define RDR_DEBUG_SW(ops)       ((g_rdr_debug_ctrl.ulSw >> ops)&0x1)

#define rdr_enter \
    do{\
        if(g_rdr_debug_ctrl.sw.print_sw == true)\
            printk(KERN_ERR ">>>>>enter %s: %.4d. <<<<<\n", __func__, __LINE__);\
    }while(0);

#define rdr_outer   \
    do{\
        if(g_rdr_debug_ctrl.sw.print_sw == true)\
            printk(KERN_ERR ">>>>>exit %s: %.4d. <<<<<\n", __func__, __LINE__);\
    }while(0);


u32 rdr_debug_init(void);
void rdr_debug_register_debug_info(char* desc,void (*help)(u32 arg),u32 arg);
void rdr_help(u32 arg);
void rdr_debug_set_sw(u32 ops, u32 flag);

#endif /*__RDR_DEBUG_H__*/


